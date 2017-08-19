/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_backflow.h"
#include "glsl_backflow_visitor.h"
#include "glsl_backend.h"
#include "glsl_backend_uniforms.h"

#include "libs/util/gfx_util/gfx_util.h"

static bool is_alu_type(SchedNodeType type) {
   switch (type) {
      case ALU_A:
      case ALU_M:
         return true;
      case SPECIAL_THRSW:
      case SPECIAL_IMUL32:
      case SPECIAL_VARYING:
      case SPECIAL_VOID:
      case SIG:
         return false;
   }
   unreachable();
}

static bool unpack_abs_invalid(BackflowFlavour f) {
   switch(f) {
      case BACKFLOW_VFPACK:
      case BACKFLOW_ROUND:
      case BACKFLOW_TRUNC:
      case BACKFLOW_FLOOR:
      case BACKFLOW_CEIL:
      case BACKFLOW_FDX:
      case BACKFLOW_FDY:
      case BACKFLOW_FTOIN:
      case BACKFLOW_FTOIZ:
      case BACKFLOW_FTOUZ:
      case BACKFLOW_FTOC:
         return true;

      default:
         return false;
   }
}

void dpostv_node_combine(Backflow *node, void *data) {
   if (node->type == ALU_M && (node->u.alu.op == BACKFLOW_MOV || node->u.alu.op == BACKFLOW_FMOV) && node->magic_write != REG_UNDECIDED && node->cond_setf == SETF_NONE) {
      /* Try to combine the move target with the previous node */
      Backflow *operand = node->dependencies[1];

      if (!is_alu_type(operand->type) && operand->type != SIG) return;
#if V3D_HAS_SIG_TO_MAGIC2
      const static v3d_qpu_sigbits_t valid_sigbits = V3D_QPU_SIG_LDUNIF;
#else
      const static v3d_qpu_sigbits_t valid_sigbits = 0;
#endif
      if (operand->type == SIG && (operand->u.sigbits & valid_sigbits) == 0) return;

      if (operand->magic_write != REG_UNDECIDED || operand->dependencies[0] != NULL) return;
      if (operand->any_io_dependents) return;
      /* XXX: We don't need to bail in this case, if we remember to transfer the iodependency */
      if (operand->io_dependencies.head != NULL) return;

      /* Can't combine regfile-requiring nodes with moves to register targets */
      if (glsl_sched_node_requires_regfile(operand)) return;

      /* Can't combine uniform loads with magic targets that also load uniforms */
      if (node->unif_type != BACKEND_UNIFORM_UNASSIGNED && operand->unif_type != BACKEND_UNIFORM_UNASSIGNED) return;

      node->type = operand->type;
      for (int i=0; i<BACKFLOW_DEP_COUNT; i++) {
         node->dependencies[i] = operand->dependencies[i];
      }
      if (is_alu_type(operand->type)) {
         node->u.alu.op  = operand->u.alu.op;
         node->u.alu.unpack[0] = operand->u.alu.unpack[0];
         node->u.alu.unpack[1] = operand->u.alu.unpack[1];
      } else {
         node->u.sigbits = operand->u.sigbits;
      }
      node->age = operand->age;

      if (operand->unif_type != BACKEND_UNIFORM_UNASSIGNED) {
         node->unif_type = operand->unif_type;
         node->unif = operand->unif;
      }

      for (BackflowIODepChainNode *n=node->io_dependencies.head; n; n=n->next)
         node->age = gfx_umax(node->age, n->val.dep->age);
   }

   if (node->dependencies[0]) {
      /* If this is a flag operation, see if we can reduce trips through data */
      Backflow *flag = node->dependencies[0];
      if (flag->type == ALU_A && flag->u.alu.op == BACKFLOW_FLN) node->dependencies[0] = flag->dependencies[0];
      if (flag->type == ALU_A && flag->u.alu.op == BACKFLOW_FL) {
         if (node->cond_setf == COND_IFFLAG || node->cond_setf == COND_IFNFLAG) {
            node->cond_setf = (node->cond_setf == COND_IFFLAG) ? COND_IFNFLAG : COND_IFFLAG;
            node->dependencies[0] = flag->dependencies[0];
         }
      }
   }

   if (is_alu_type(node->type)) {
      if (glsl_sched_node_admits_unpack(node->u.alu.op)) {
         /* Loop over left and right dependencies. 1 = Left, 2 = Right */
         for (int i=1; i<=2; i++) {
            Backflow *operand = node->dependencies[i];
            if (!operand) continue;

            if (operand->magic_write != REG_UNDECIDED || operand->dependencies[0] != NULL) continue;
            if (operand->any_io_dependents) continue;
            /* XXX: We don't need to bail in this case, if we remember to transfer the iodendency */
            if (operand->io_dependencies.head != NULL) continue;

            /* Bail if the unpack slot is already in use */
            if (node->u.alu.unpack[i-1] != UNPACK_NONE) continue;

            if (operand->type == ALU_M && operand->u.alu.op == BACKFLOW_FMOV) {
               /* Merge up with node */
               SchedNodeUnpack unpack_code = operand->u.alu.unpack[0];

               /* Some instructions don't support the '.abs' unpack */
               if (unpack_code == UNPACK_ABS && unpack_abs_invalid(node->u.alu.op)) continue;

               node->u.alu.unpack[i-1] = unpack_code;
               node->dependencies[i] = operand->dependencies[1];
            }
         }
      }
   }
}

void glsl_combine_sched_nodes(SchedBlock *b) {
   BackflowVisitor *comb_visit = glsl_backflow_visitor_begin(NULL, NULL, dpostv_node_combine);

   for (int i=0; i<b->num_outputs; i++)
      glsl_backflow_visit(b->outputs[i], comb_visit);

   for (BackflowChainNode *n=b->iodeps.head; n; n=n->next)
      glsl_backflow_visit(n->val, comb_visit);

   /* Texture dependencies have not been resolved yet, so data written will not be combined */
   for (struct tmu_lookup_s *l = b->tmu_lookups; l; l=l->next) {
      glsl_backflow_visit(l->last_write, comb_visit);
   }

   if(b->branch_cond)
      glsl_backflow_visit(b->branch_cond, comb_visit);

   glsl_backflow_visitor_end(comb_visit);
}
