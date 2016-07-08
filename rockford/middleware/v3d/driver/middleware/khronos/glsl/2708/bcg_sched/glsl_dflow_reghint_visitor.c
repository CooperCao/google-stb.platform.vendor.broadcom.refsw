/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#include "middleware/khronos/glsl/glsl_common.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_reghint_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.h"

static void Destroy(void *me)
{
   UNUSED(me);
}

static void Accept(void *me, DFlowNode *node)
{
   DFlowRegHintVisitor  *self = (DFlowRegHintVisitor *)me;
   const NodeList       *children;

   // If this node is a "given" i.e. the result must use a given register file,
   // then set this now
   switch (DFlowNode_Flavour(node))
   {
   case DATAFLOW_FRAG_GET_X:
   case DATAFLOW_PACK_COL_REPLICATE:
   case DATAFLOW_PACK_16A:
   case DATAFLOW_PACK_16B:
   case DATAFLOW_PACK_COL_R:
   case DATAFLOW_PACK_COL_G:
   case DATAFLOW_PACK_COL_B:
   case DATAFLOW_PACK_COL_A:
      DFlowNode_SetRegFile(node, DFlowRegFile_A_OR_ACCUM);
      break;

   case DATAFLOW_CONST_BOOL:
   case DATAFLOW_CONST_INT:
   case DATAFLOW_CONST_FLOAT:
   case DATAFLOW_CONST_SAMPLER:
   case DATAFLOW_FRAG_GET_Y:
      DFlowNode_SetRegFile(node, DFlowRegFile_B_OR_ACCUM);
      break;

   case DATAFLOW_UNIFORM:
   case DATAFLOW_ATTRIBUTE:
   case DATAFLOW_VARYING:
   case DATAFLOW_ARITH_NEGATE:
   case DATAFLOW_LOGICAL_NOT:
   case DATAFLOW_MUL:
   case DATAFLOW_ADD:
   case DATAFLOW_SUB:
   case DATAFLOW_RSUB:
   case DATAFLOW_LESS_THAN:
   case DATAFLOW_LESS_THAN_EQUAL:
   case DATAFLOW_GREATER_THAN:
   case DATAFLOW_GREATER_THAN_EQUAL:
   case DATAFLOW_EQUAL:
   case DATAFLOW_NOT_EQUAL:
	case DATAFLOW_LOGICAL_AND:
	case DATAFLOW_LOGICAL_XOR:
	case DATAFLOW_LOGICAL_OR:
	case DATAFLOW_CONDITIONAL:
   case DATAFLOW_MOV:
   case DATAFLOW_FTOI_TRUNC:
   case DATAFLOW_FTOI_NEAREST:
   case DATAFLOW_ITOF:
   case DATAFLOW_BITWISE_NOT:
   case DATAFLOW_BITWISE_AND:
   case DATAFLOW_BITWISE_OR:
   case DATAFLOW_BITWISE_XOR:
   case DATAFLOW_V8MULD:
   case DATAFLOW_V8MIN:
   case DATAFLOW_V8MAX:
   case DATAFLOW_V8ADDS:
   case DATAFLOW_V8SUBS:
   case DATAFLOW_INTEGER_ADD:
   case DATAFLOW_INTRINSIC_RSQRT:
	case DATAFLOW_INTRINSIC_RCP:
	case DATAFLOW_INTRINSIC_LOG2:
	case DATAFLOW_INTRINSIC_EXP2:
   case DATAFLOW_INTRINSIC_CEIL:
   case DATAFLOW_INTRINSIC_FLOOR:
   case DATAFLOW_INTRINSIC_SIGN:
	case DATAFLOW_INTRINSIC_MIN:
	case DATAFLOW_INTRINSIC_MAX:
   case DATAFLOW_INTRINSIC_MINABS:
   case DATAFLOW_INTRINSIC_MAXABS:
   case DATAFLOW_THREADSWITCH:
   case DATAFLOW_TEX_SET_COORD_S:
	case DATAFLOW_TEX_SET_COORD_T:
	case DATAFLOW_TEX_SET_COORD_R:
	case DATAFLOW_TEX_SET_BIAS:
	case DATAFLOW_TEX_SET_LOD:
	case DATAFLOW_TEX_GET_CMP_R:
	case DATAFLOW_TEX_GET_CMP_G:
	case DATAFLOW_TEX_GET_CMP_B:
	case DATAFLOW_TEX_GET_CMP_A:
   case DATAFLOW_FRAG_GET_Z:
   case DATAFLOW_FRAG_GET_W:
   case DATAFLOW_FRAG_GET_PC_X:
   case DATAFLOW_FRAG_GET_PC_Y:
   case DATAFLOW_FRAG_GET_FF:
   case DATAFLOW_VARYING_C:
   case DATAFLOW_FRAG_GET_COL:
   case DATAFLOW_FRAG_SUBMIT_STENCIL:
   case DATAFLOW_FRAG_SUBMIT_Z:
   case DATAFLOW_FRAG_SUBMIT_MS:
   case DATAFLOW_FRAG_SUBMIT_ALL:
   case DATAFLOW_FRAG_SUBMIT_R0:
   case DATAFLOW_FRAG_SUBMIT_R1:
   case DATAFLOW_FRAG_SUBMIT_R2:
   case DATAFLOW_FRAG_SUBMIT_R3:
   case DATAFLOW_TMU_SWAP:
   case DATAFLOW_TEX_SET_DIRECT:
   case DATAFLOW_VERTEX_SET:
   case DATAFLOW_VPM_READ_SETUP:
   case DATAFLOW_VPM_WRITE_SETUP:
   case DATAFLOW_SHIFT_RIGHT:
   case DATAFLOW_LOGICAL_SHR:
   case DATAFLOW_UNIFORM_ADDRESS:
   case DATAFLOW_SCOREBOARD_WAIT:
      break;

   case DATAFLOW_UNIFORM_OFFSET:
      break;

   case DATAFLOW_UNPACK_COL_R:
   case DATAFLOW_UNPACK_COL_G:
   case DATAFLOW_UNPACK_COL_B:
   case DATAFLOW_UNPACK_COL_A:
   case DATAFLOW_UNPACK_16A:
   case DATAFLOW_UNPACK_16A_F:
   case DATAFLOW_UNPACK_16B:
   case DATAFLOW_UNPACK_16B_F:
   case DATAFLOW_UNPACK_8A:
   case DATAFLOW_UNPACK_8B:
   case DATAFLOW_UNPACK_8C:
   case DATAFLOW_UNPACK_8D:
   case DATAFLOW_UNPACK_8R:
   case DATAFLOW_UNPACK_PLACEHOLDER_R:
   case DATAFLOW_UNPACK_PLACEHOLDER_B:
      {
         DFlowNode         *child = NodeList_front(DFlowNode_Children(node));
         DataflowFlavour    childFlavour = DFlowNode_Flavour(child);

         if (childFlavour != DATAFLOW_TEX_GET_CMP_R && childFlavour != DATAFLOW_INTRINSIC_RCP)
            DFlowNode_SetRegFile(child, DFlowRegFile_A_ONLY);
         return;
      }

   default:
      UNREACHABLE();
      break;
   }

   children = DFlowNode_Children(node);

   if (NodeList_size(children) == 2)
   {
      DFlowNode               *child[2] = { NULL };
      uint32_t                 ix = 0;
      NodeList_const_iterator  iter;

      for (iter = NodeList_const_begin(children); iter != NodeList_const_end(children); NodeList_const_next(&iter), ++ix)
         child[ix] = NodeList_const_star(iter);

      DFlowNode_SetRegFile(child[0], DFlowRegFile_OtherFile(DFlowNode_GetRegFile(child[1]), DFlowRegFile_A_OR_ACCUM));
      DFlowNode_SetRegFile(child[1], DFlowRegFile_OtherFile(DFlowNode_GetRegFile(child[0]), DFlowRegFile_B_OR_ACCUM));
   }
}

void DFlowRegHintVisitor_Constr(DFlowRegHintVisitor *self, DFlowRecursionOptimizer *opt)
{
   DFlowVisitor_Constr(self, Destroy, Accept, opt);
}

void DFlowRegHintVisitor_Visit(DFlowRegHintVisitor *self, DFlowNode *node)
{
   DFlowVisitor_VisitBottomUp(self, node);
}
