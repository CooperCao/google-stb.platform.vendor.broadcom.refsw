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

static bool is_alu_type(ALU_TYPE_T type) {
   switch (type) {
      case ALU_A:
      case ALU_A_SWAP0:
      case ALU_A_SWAP1:
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
   UNREACHABLE();
}

static bool alu_admits_unpack(Backflow *node) {
   if (node->type == ALU_A || node->type == ALU_A_SWAP0 || node->type == ALU_A_SWAP1) {
      /* XXX: Could trivially add more things that unpack here */
      /* META XXX: Implement the actual table from the spec so this is all automatic */
      if (node->op == ( 0 | 1<<2 | 1) ||              /* fadd */
          node->op == (64 | 1<<2 | 1) ||              /* fsub */
          node->op == (128| 1<<2 | 1) ||              /* fmin/fmax */
          node->op == (192| 1<<2 | 1) ||              /* fcmp */
          node->op == 53               ) return true; /* vfpack */   /* TODO: abs not allowed */
      return false;
   }

   if (node->type == ALU_M) {
      if (node->op == (16 | 1<<2 | 1)) return true;   /* fmul */
      return false;
   }
   UNREACHABLE();
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
      if (operand->type == ALU_A && operand->op == 188) return;

      node->type = operand->type;
      for (int i=0; i<BACKFLOW_DEP_COUNT; i++) {
         node->dependencies[i] = operand->dependencies[i];
      }
      node->op = operand->op;
      node->op1 = operand->op1;
      node->op2 = operand->op2;
      node->age = operand->age;

      BackflowChainNode *n;
      LIST_FOR_EACH(n, &node->io_dependencies, l) node->age = MAX(node->age, n->ptr->age);

      /* Validity checking: These would be invalid nodes, and would break this */
      assert(node->sigbits == 0 && operand->sigbits == 0);
      assert(operand->unif == BACKEND_UNIFORM_UNASSIGNED);

      /* Fix the data_dependents chains in the modified nodes */
      glsl_backflow_chain_remove(&operand->data_dependents, node);
      for (int i=0; i<BACKFLOW_DEP_COUNT; i++) {
         if (operand->dependencies[i] != NULL)
            glsl_backflow_chain_append(&operand->dependencies[i]->data_dependents, node);
      }
   }

   if (is_alu_type(node->type)) {
      if (alu_admits_unpack(node)) {
         /* XXX: We cheat here by assuming that because unpack is admitted the node's a binop */
         /* Loop over left and right dependencies. 1 = Left, 2 = Right */
         for (int i=1; i<=2; i++) {
            Backflow *operand = node->dependencies[i];
            int pack_shift[3] = { /* Not valid: */-1, 2, 0 };

            if (operand->any_io_dependents) continue;
            /* XXX: We don't need to bail in this case, if we remember to transfer the iodendency */
            if (operand->io_dependencies.count != 0) continue;

            /* Bail if the unpack slot is already in use */
            /* XXX This is already dealt with by admits_unpack, but will be needed when that becomes more general */
            if ((node->op & (3 << pack_shift[i])) != (1 << pack_shift[i])) continue;

            /* XXX: Cheat here as well by assuming that FMOV means unpack */
            if (operand->type == ALU_FMOV) {
               /* Merge up with node */
               uint32_t pack_code = operand->op2;
               if (pack_code == 0 && node->op == 53) continue; /* no vfpack.abs */
               node->op &= ~(3 << pack_shift[i]);
               node->op |= (pack_code << pack_shift[i]);
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
