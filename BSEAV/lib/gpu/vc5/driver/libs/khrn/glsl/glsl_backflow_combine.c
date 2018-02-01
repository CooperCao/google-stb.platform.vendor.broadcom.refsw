/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_backflow.h"
#include "glsl_backflow_visitor.h"
#include "glsl_backend.h"
#include "glsl_backend_uniforms.h"
#include "glsl_map.h"

#include "libs/util/gfx_util/gfx_util.h"

typedef struct combine_data {
   Map *node_num_dependents;
} combine_data;

static void inc_num_dependents(combine_data *data, Backflow *node) {
   // Only accumulate this information for these node types.
   if (node->type != SPECIAL_VARYING && node->type != SPECIAL_IMUL32)
      return;

   uintptr_t num = (uintptr_t)glsl_map_get(data->node_num_dependents, node);
   glsl_map_put(data->node_num_dependents, node, (void*)(num + 1));
}

static unsigned get_num_dependents(combine_data *data, Backflow *node) {
   assert(node->type == SPECIAL_VARYING || node->type == SPECIAL_IMUL32);
   return (unsigned)(uintptr_t)glsl_map_get(data->node_num_dependents, node);
}

static bool unpack_abs_invalid(v3d_qpu_opcode_t op) {
   switch(op) {
      case V3D_QPU_OP_VFPACK:
      case V3D_QPU_OP_FROUND:
      case V3D_QPU_OP_FTRUNC:
      case V3D_QPU_OP_FFLOOR:
      case V3D_QPU_OP_FCEIL:
      case V3D_QPU_OP_FDX:
      case V3D_QPU_OP_FDY:
      case V3D_QPU_OP_FTOIN:
      case V3D_QPU_OP_FTOIZ:
      case V3D_QPU_OP_FTOUZ:
      case V3D_QPU_OP_FTOC:
         return true;

      default:
         return false;
   }
}

static void dpostv_node_combine_init(Backflow *node, void *data) {
   for (unsigned i = 0; i < BACKFLOW_DEP_COUNT; i++)
      if (node->dependencies[i] != NULL)
         inc_num_dependents(data, node->dependencies[i]);
}

static void dpostv_node_combine(Backflow *node, void *data) {
   if (node->type == ALU && (node->u.alu.op == V3D_QPU_OP_MOV || node->u.alu.op == V3D_QPU_OP_FMOV) &&
        ((node->magic_write != REG_UNDECIDED && node->cond_setf == SETF_NONE) ||
         (node->magic_write == REG_UNDECIDED && (node->cond_setf == COND_IFFLAG || node->cond_setf == COND_IFNFLAG))))
   {
      /* Try to combine the move target with the previous node */
      Backflow *operand = node->dependencies[1];

      if (operand->type != ALU && operand->type != SIG && operand->type != SPECIAL_VARYING && operand->type != SPECIAL_IMUL32) return;
#if V3D_VER_AT_LEAST(4,1,34,0)
      const static v3d_qpu_sigbits_t valid_sigbits = V3D_QPU_SIG_LDUNIF;
#else
      const static v3d_qpu_sigbits_t valid_sigbits = 0;
#endif
      if (operand->type == SIG && (operand->u.sigbits & valid_sigbits) == 0) return;
      if (operand->type == SIG && node->cond_setf != SETF_NONE) return;

      /* Only combine varyings or IMUL32 if there is a single consumer. */
      if ((operand->type == SPECIAL_VARYING || operand->type == SPECIAL_IMUL32) &&
          (get_num_dependents(data, operand) > 1 || node->cond_setf != SETF_NONE))
         return;

      if (operand->magic_write != REG_UNDECIDED || operand->dependencies[0] != NULL) return;
      if (operand->any_io_dependents) return;
      /* XXX: We don't need to bail in this case, if we remember to transfer the iodependency */
      if (operand->io_dependencies.head != NULL) return;

      /* Can't combine regfile-requiring nodes with moves to register targets */
      if (glsl_sched_node_requires_regfile(operand)) return;

      /* Can't combine uniform loads with magic targets that also load uniforms */
      if (node->unif_type != BACKEND_UNIFORM_UNASSIGNED && operand->unif_type != BACKEND_UNIFORM_UNASSIGNED) return;

      /* If this node uses flags then it can't combine with a node that already does */
      if (node->cond_setf != SETF_NONE && operand->dependencies[3]) return;

      node->type = operand->type;
      for (int i=1; i<BACKFLOW_DEP_COUNT; i++)
         if (i != 3 || node->cond_setf == SETF_NONE)
            node->dependencies[i] = operand->dependencies[i];

      if (operand->type == ALU) {
         node->u.alu.op  = operand->u.alu.op;
         node->u.alu.unpack[0] = operand->u.alu.unpack[0];
         node->u.alu.unpack[1] = operand->u.alu.unpack[1];
      } else if (operand->type == SPECIAL_VARYING) {
         node->u.varying = operand->u.varying;
      } else if (operand->type == SIG) {
         node->u.sigbits = operand->u.sigbits;
      }
      node->age = operand->age;

      if (operand->unif_type != BACKEND_UNIFORM_UNASSIGNED) {
         node->unif_type = operand->unif_type;
         node->unif = operand->unif;
      }

      for (BackflowIODepChainNode *n=node->io_dependencies.head; n; n=n->next) {
         node->age = gfx_umax(node->age, n->val.dep->age);

         // Offset the IO deps to align the last instruction correctly.
         switch (operand->type) {
         case SPECIAL_VARYING:   n->val.io_timestamp_offset -= 2; break;
         case SPECIAL_IMUL32:    n->val.io_timestamp_offset -= 1; break;
         default: break;
         }
      }
   }

   if (node->dependencies[0]) {
      /* If this is a flag operation, see if we can reduce trips through data */
      Backflow *flag = node->dependencies[0];
      if (flag->type == ALU && flag->u.alu.op == V3D_QPU_OP_VFLNA) node->dependencies[0] = flag->dependencies[0];
      if (flag->type == ALU && flag->u.alu.op == V3D_QPU_OP_VFLA) {
         if (node->cond_setf == COND_IFFLAG || node->cond_setf == COND_IFNFLAG) {
            node->cond_setf = (node->cond_setf == COND_IFFLAG) ? COND_IFNFLAG : COND_IFFLAG;
            node->dependencies[0] = flag->dependencies[0];
         }
      }
   }

   if (node->type == ALU) {
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

            if (operand->type == ALU && operand->u.alu.op == V3D_QPU_OP_FMOV) {
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

static void visit_sched_block_nodes(SchedBlock *b, BackflowPostVisitor dpostv, combine_data* data) {
   BackflowVisitor *comb_visit = glsl_backflow_visitor_begin(data, NULL, dpostv);

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

void glsl_combine_sched_nodes(SchedBlock *b, BackflowPostVisitor dpostv)
{
   combine_data data = {0, };
   data.node_num_dependents = glsl_map_new();

   // Init pass, setup num_dependents for each node.
   visit_sched_block_nodes(b, dpostv_node_combine_init, &data);

   // The block has an implicit dependency on output nodes and the block condition,
   // as these nodes are copied/moved to the right place after scheduling.
   for (int i=0; i<b->num_outputs; i++) {
      if (b->outputs[i])
         inc_num_dependents(&data, b->outputs[i]);
   }
   if (b->branch_cond)
      inc_num_dependents(&data, b->branch_cond);

   // Combine pass.
   visit_sched_block_nodes(b, dpostv_node_combine, &data);

   glsl_map_delete(data.node_num_dependents);
}
