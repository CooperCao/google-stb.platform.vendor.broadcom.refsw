/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

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
      case ALU_MOV:
      case ALU_FMOV:
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
   if (node->type == ALU_MOV && node->magic_write != REG_UNDECIDED && node->cond_setf == SETF_NONE) {
      /* Try to combine the move target with the previous node */
      Backflow *operand = node->dependencies[1];

      if (!is_alu_type(operand->type)) return;

      if (operand->magic_write != REG_UNDECIDED) return;
      if (operand->any_io_dependents) return;
      /* XXX: We don't need to bail in this case, if we remember to transfer the iodependency */
      if (operand->io_dependencies.count != 0) return;

      /* Can't combine ldvpm with moves to register targets */
      if (operand->type == ALU_A && glsl_sched_node_requires_regfile(operand->u.alu.op)) return;

      node->type = operand->type;
      for (int i=0; i<BACKFLOW_DEP_COUNT; i++) {
         node->dependencies[i] = operand->dependencies[i];
      }
      node->u.alu.op  = operand->u.alu.op;
      node->u.alu.unpack[0] = operand->u.alu.unpack[0];
      node->u.alu.unpack[1] = operand->u.alu.unpack[1];
      node->age = operand->age;

      BackflowChainNode *n;
      LIST_FOR_EACH(n, &node->io_dependencies, l) node->age = gfx_umax(node->age, n->ptr->age);

      /* Validity checking: These would be invalid nodes, and would break this */
      assert(operand->unif == BACKEND_UNIFORM_UNASSIGNED);

      /* Fix the data_dependents chains in the modified nodes */
      glsl_backflow_chain_remove(&operand->data_dependents, node);
      for (int i=0; i<BACKFLOW_DEP_COUNT; i++) {
         if (operand->dependencies[i] != NULL)
            glsl_backflow_chain_append(&operand->dependencies[i]->data_dependents, node);
      }
   }

   if (is_alu_type(node->type)) {
      if (glsl_sched_node_admits_unpack(node->u.alu.op)) {
         /* Loop over left and right dependencies. 1 = Left, 2 = Right */
         for (int i=1; i<=2; i++) {
            Backflow *operand = node->dependencies[i];
            if (!operand) continue;

            if (operand->any_io_dependents) continue;
            /* XXX: We don't need to bail in this case, if we remember to transfer the iodendency */
            if (operand->io_dependencies.count != 0) continue;

            /* Bail if the unpack slot is already in use */
            if (node->u.alu.unpack[i-1] != UNPACK_NONE) continue;

            if (operand->type == ALU_FMOV) {
               /* Merge up with node */
               SchedNodeUnpack pack_code = operand->u.alu.unpack[0];

               /* Some instructions don't support the '.abs' unpack */
               if (pack_code == UNPACK_ABS && unpack_abs_invalid(node->u.alu.op)) continue;

               node->u.alu.unpack[i-1] = pack_code;
               node->dependencies[i] = operand->dependencies[1];

               glsl_backflow_chain_remove(&operand->data_dependents, node);
               glsl_backflow_chain_append(&operand->dependencies[1]->data_dependents, node);
            }
         }
      }
   }
}

void glsl_combine_sched_nodes(SchedBlock *b) {
   for (int i=0; i<b->num_outputs; i++) {
      glsl_backflow_accept_towards_leaves(b->outputs[i], NULL, NULL, dpostv_node_combine, BACKEND_PASS_COMBINE);
   }

   BackflowChainNode *chain_node;
   LIST_FOR_EACH(chain_node, &b->iodeps, l) {
      glsl_backflow_accept_towards_leaves(chain_node->ptr, NULL, NULL, dpostv_node_combine, BACKEND_PASS_COMBINE);
   }

   /* Texture dependencies have not been resolved yet, so data written will not be combined */
   for (struct tmu_lookup_s *l = b->tmu_lookups; l; l=l->next) {
      glsl_backflow_accept_towards_leaves(l->last_write, NULL, NULL, dpostv_node_combine, BACKEND_PASS_COMBINE);
   }
}
