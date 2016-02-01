/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_print_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.h"
#include "middleware/khronos/glsl/glsl_common.h"
#include <stdio.h>

static const char *ResultRegName(DFlowNode* dflow)
{
   static const char *regNames[Register_NUM_REGISTERS] =
   {
      "RA0",  "RA1",  "RA2",  "RA3",  "RA4",  "RA5",  "RA6",  "RA7",
      "RA8",  "RA9",  "RA10", "RA11", "RA12", "RA13", "RA14", "RA15",
      "RA16", "RA17", "RA18", "RA19", "RA20", "RA21", "RA22", "RA23",
      "RA24", "RA25", "RA26", "RA27", "RA28", "RA29", "RA30", "RA31",
      "RB0",  "RB1",  "RB2",  "RB3",  "RB4",  "RB5",  "RB6",  "RB7",
      "RB8",  "RB9",  "RB10", "RB11", "RB12", "RB13", "RB14", "RB15",
      "RB16", "RB17", "RB18", "RB19", "RB20", "RB21", "RB22", "RB23",
      "RB24", "RB25", "RB26", "RB27", "RB28", "RB29", "RB30", "RB31",
      "ACC0", "ACC1", "ACC2", "ACC3", "ACC4", "ACC5",
      "SMALL_IMMED",
      "UNIFORM_READ",
      "VARYING_READ",
      "TMU_NOSWAP",
      "ELEMENT_NUMBER",
      "QPU_NUMBER",
      "HOST_INT",
      "NOP",
      "UNIFORMS_ADDRESS",
      "X_PIXEL_COORD",
      "Y_PIXEL_COORD",
      "QUAD_X",
      "QUAD_Y",
      "MS_FLAGS",
      "REV_FLAG",
      "TLB_STENCIL_SETUP",
      "TLB_Z",
      "TLB_COLOUR_MS",
      "TLB_COLOUR_ALL",
      "TLB_ALPHA_MASK",
      "VPM_READ",
      "VPM_WRITE",
      "VPM_LD_BUSY",
      "VPM_ST_BUSY",
      "VPMVCD_RD_SETUP",
      "VPMVCD_WR_SETUP",
      "VPM_LD_WAIT",
      "VPM_ST_WAIT",
      "VPM_LD_ADDR",
      "VPM_ST_ADDR",
      "MUTEX_ACQUIRE",
      "MUTEX_RELEASE",
      "SFU_RECIP",
      "SFU_RECIPSQRT",
      "SFU_EXP",
      "SFU_LOG",
      "TMU0_S",
      "TMU0_T",
      "TMU0_R",
      "TMU0_B",
      "TMU1_S",
      "TMU1_T",
      "TMU1_R",
      "TMU1_B",
      "VIRTUAL_FLAGS",
      "VIRTUAL_TMUWRITE",
      "UNKNOWN"
   };

   if (!QPUOperand_IsRegister(&dflow->m_result))
      return "";

   return regNames[QPUOperand_ValueRegister(&dflow->m_result)];
}

static void DFlowPrintVisitor_PrintNode(DFlowPrintVisitor *self, DFlowNode* dflow)
{
   FILE  *fp = self->m_fp;
   char  buf[12];

   // Print node reference and open label string.
   fprintf(fp, "\tn%x [label=\"", (size_t)((dflow->m_uniqueId << 16) | dflow->m_flavour));

   // Print contents of label string.
   switch (dflow->m_flavour)
   {
   case DATAFLOW_CONST_BOOL:
      fprintf(fp, dflow->m_uniform.m_constBool.m_cbValue ? "true" : "false");
      break;

   case DATAFLOW_CONST_INT:
      fprintf(fp, "%d", dflow->m_uniform.m_constInt.m_ciValue);
      break;

   case DATAFLOW_CONST_FLOAT:
      {
         if (NodeList_size(DFlowNode_Parents(dflow)) > 0 &&
             DFlowNode_Flavour(NodeList_front(DFlowNode_Parents(dflow))) == DATAFLOW_INTEGER_ADD)
         {
            fprintf(fp, "%d", dflow->m_uniform.m_constFloat.m_cfValue);
         }
         else
         {
            mixed_float t;
            t.ui = dflow->m_uniform.m_constFloat.m_cfValue;
            fprintf(fp, "%f", t.f);
         }
      }
      break;

   case DATAFLOW_CONST_SAMPLER:
      fprintf(fp, "sampler[%d] from <%s>", dflow->m_uniform.m_constSampler.m_csLocation, dflow->m_uniform.m_constSampler.m_csName);
      break;

   case DATAFLOW_UNIFORM:
      fprintf(fp, "uniform[%d] from <%s>", dflow->m_uniform.m_linkableValue.m_lvRow, dflow->m_uniform.m_linkableValue.m_lvName);
      break;
   case DATAFLOW_ATTRIBUTE:
      fprintf(fp, "attribute[%d] from <%s>", dflow->m_uniform.m_linkableValue.m_lvRow, dflow->m_uniform.m_linkableValue.m_lvName);
      break;
   case DATAFLOW_VARYING:
      fprintf(fp, "varying[%d] from <%s>", dflow->m_uniform.m_linkableValue.m_lvRow, dflow->m_uniform.m_linkableValue.m_lvName);
      break;

   case DATAFLOW_UNIFORM_OFFSET:
      fprintf(fp, "uniform offset");
      break;

   case DATAFLOW_ARITH_NEGATE:
      fprintf(fp, "-");
      break;
   case DATAFLOW_LOGICAL_NOT:
      fprintf(fp, "!");
      break;
   case DATAFLOW_INTRINSIC_RSQRT:
      fprintf(fp, "rsqrt");
      break;
   case DATAFLOW_INTRINSIC_CEIL:
      fprintf(fp, "ceil");
      break;
   case DATAFLOW_INTRINSIC_FLOOR:
      fprintf(fp, "floor");
      break;
   case DATAFLOW_INTRINSIC_SIGN:
      fprintf(fp, "sign");
      break;
   case DATAFLOW_INTRINSIC_RCP:
      fprintf(fp, "rcp");
      break;
   case DATAFLOW_INTRINSIC_LOG2:
      fprintf(fp, "log2");
      break;
   case DATAFLOW_INTRINSIC_EXP2:
      fprintf(fp, "exp2");
      break;

   case DATAFLOW_MOV:
      fprintf(fp, "mov");
      break;

   case DATAFLOW_FTOI_TRUNC:
   case DATAFLOW_FTOI_NEAREST:
      fprintf(fp, "toi");
      break;
   case DATAFLOW_ITOF:
      fprintf(fp, "tof");
      break;

   case DATAFLOW_MUL:
      fprintf(fp, "*");
      break;
   case DATAFLOW_ADD:
      fprintf(fp, "+");
      break;
   case DATAFLOW_SUB:
   case DATAFLOW_RSUB:
      fprintf(fp, "-");
      break;
   case DATAFLOW_LESS_THAN:
      fprintf(fp, "<");
      break;
   case DATAFLOW_LESS_THAN_EQUAL:
      fprintf(fp, "<=");
      break;
   case DATAFLOW_GREATER_THAN:
      fprintf(fp, ">");
      break;
   case DATAFLOW_GREATER_THAN_EQUAL:
      fprintf(fp, ">=");
      break;
   case DATAFLOW_EQUAL:
      fprintf(fp, "==");
      break;
   case DATAFLOW_NOT_EQUAL:
      fprintf(fp, "!=");
      break;
   case DATAFLOW_LOGICAL_AND:
      fprintf(fp, "&&");
      break;
   case DATAFLOW_LOGICAL_XOR:
      fprintf(fp, "^^");
      break;
   case DATAFLOW_LOGICAL_OR:
      fprintf(fp, "||");
      break;
   case DATAFLOW_INTRINSIC_MIN:
      fprintf(fp, "min");
      break;
   case DATAFLOW_INTRINSIC_MAX:
      fprintf(fp, "max");
      break;
   case DATAFLOW_INTRINSIC_MINABS:
      fprintf(fp, "minabs");
      break;
   case DATAFLOW_INTRINSIC_MAXABS:
      fprintf(fp, "maxabs");
      break;

   case DATAFLOW_CONDITIONAL:
      fprintf(fp, "cond");
      break;

   case DATAFLOW_THREADSWITCH:
      fprintf(fp, "thrsw");
      break;

   case DATAFLOW_TEX_SET_COORD_S:
      fprintf(fp, "set coord s");
      break;
   case DATAFLOW_TEX_SET_COORD_T:
      fprintf(fp, "set coord t");
      break;
   case DATAFLOW_TEX_SET_COORD_R:
      fprintf(fp, "set coord r");
      break;
   case DATAFLOW_TEX_SET_BIAS:
      fprintf(fp, "set bias");
      break;
   case DATAFLOW_TEX_SET_LOD:
      fprintf(fp, "set lod");
      break;

   case DATAFLOW_TEX_GET_CMP_R:
      fprintf(fp, "get cmp r");
      break;
   case DATAFLOW_TEX_GET_CMP_G:
      fprintf(fp, "get cmp g");
      break;
   case DATAFLOW_TEX_GET_CMP_B:
      fprintf(fp, "get cmp b");
      break;
   case DATAFLOW_TEX_GET_CMP_A:
      fprintf(fp, "get cmp a");
      break;

   case DATAFLOW_FRAG_GET_X:
      fprintf(fp, "get frag x");
      break;
   case DATAFLOW_FRAG_GET_Y:
      fprintf(fp, "get frag y");
      break;
   case DATAFLOW_FRAG_GET_Z:
      fprintf(fp, "get frag z");
      break;
   case DATAFLOW_FRAG_GET_W:
      fprintf(fp, "get frag w");
      break;
   case DATAFLOW_FRAG_GET_PC_X:
      fprintf(fp, "get frag pc x");
      break;
   case DATAFLOW_FRAG_GET_PC_Y:
      fprintf(fp, "get frag pc y");
      break;
   case DATAFLOW_FRAG_GET_FF:
      fprintf(fp, "get frag ff");
      break;

   case DATAFLOW_BITWISE_NOT:
      fprintf(fp, "~");
      break;
   case DATAFLOW_BITWISE_AND:
      fprintf(fp, "&");
      break;
   case DATAFLOW_BITWISE_OR:
      fprintf(fp, "|");
      break;
   case DATAFLOW_BITWISE_XOR:
      fprintf(fp, "^");
      break;
   case DATAFLOW_V8MULD:
      fprintf(fp, "v8 *");
      break;
   case DATAFLOW_V8MIN:
      fprintf(fp, "v8 min");
      break;
   case DATAFLOW_V8MAX:
      fprintf(fp, "v8 max");
      break;
   case DATAFLOW_V8ADDS:
      fprintf(fp, "v8 +");
      break;
   case DATAFLOW_V8SUBS:
      fprintf(fp, "v8 -");
      break;
   case DATAFLOW_INTEGER_ADD:
      fprintf(fp, "int +");
      break;
   case DATAFLOW_VARYING_C:
      fprintf(fp, "vary c");
      break;
   case DATAFLOW_FRAG_GET_COL:
      fprintf(fp, "get frag col");
      break;
   case DATAFLOW_FRAG_SUBMIT_STENCIL:
      fprintf(fp, "set frag stencil");
      break;
   case DATAFLOW_FRAG_SUBMIT_Z:
      fprintf(fp, "set frag z");
      break;
   case DATAFLOW_FRAG_SUBMIT_MS:
      fprintf(fp, "set frag ms");
      break;
   case DATAFLOW_FRAG_SUBMIT_ALL:
      fprintf(fp, "set frag all");
      break;
   case DATAFLOW_FRAG_SUBMIT_R0:
      fprintf(fp, "set r0");
      break;
   case DATAFLOW_FRAG_SUBMIT_R1:
      fprintf(fp, "set r1");
      break;
   case DATAFLOW_FRAG_SUBMIT_R2:
      fprintf(fp, "set r2");
      break;
   case DATAFLOW_FRAG_SUBMIT_R3:
      fprintf(fp, "set r3");
      break;
   case DATAFLOW_TMU_SWAP:
      fprintf(fp, "set tmu swap");
      break;
   case DATAFLOW_TEX_SET_DIRECT:
      fprintf(fp, "set direct lookup");
      break;
   case DATAFLOW_VERTEX_SET:
      fprintf(fp, "set vertex data");
      break;
   case DATAFLOW_VPM_READ_SETUP:
      fprintf(fp, "set vpm read setup");
      break;
   case DATAFLOW_VPM_WRITE_SETUP:
      fprintf(fp, "set vpm write setup");
      break;
   case DATAFLOW_PACK_COL_REPLICATE:
      fprintf(fp, "pack abcd");
      break;
   case DATAFLOW_PACK_COL_R:
      fprintf(fp, "pack col r");
      break;
   case DATAFLOW_PACK_COL_G:
      fprintf(fp, "pack col g");
      break;
   case DATAFLOW_PACK_COL_B:
      fprintf(fp, "pack col b");
      break;
   case DATAFLOW_PACK_COL_A:
      fprintf(fp, "pack col a");
      break;
   case DATAFLOW_PACK_16A:
      fprintf(fp, "pack 16a");
      break;
   case DATAFLOW_PACK_16B:
      fprintf(fp, "pack 16b");
      break;

   case DATAFLOW_UNPACK_COL_R:
      fprintf(fp, "unpack col r");
      break;
   case DATAFLOW_UNPACK_COL_G:
      fprintf(fp, "unpack col g");
      break;
   case DATAFLOW_UNPACK_COL_B:
      fprintf(fp, "unpack col b");
      break;
   case DATAFLOW_UNPACK_COL_A:
      fprintf(fp, "unpack col a");
      break;
   case DATAFLOW_UNPACK_16A:
      fprintf(fp, "unpack 16a");
      break;
   case DATAFLOW_UNPACK_16A_F:
      fprintf(fp, "unpack 16a float");
      break;
   case DATAFLOW_UNPACK_16B:
      fprintf(fp, "unpack 16b");
      break;
   case DATAFLOW_UNPACK_16B_F:
      fprintf(fp, "unpack 16b float");
      break;
   case DATAFLOW_UNPACK_8A:
      fprintf(fp, "unpack 8a");
      break;
   case DATAFLOW_UNPACK_8B:
      fprintf(fp, "unpack 8b");
      break;
   case DATAFLOW_UNPACK_8C:
      fprintf(fp, "unpack 8c");
      break;
   case DATAFLOW_UNPACK_8D:
      fprintf(fp, "unpack 8d");
      break;
   case DATAFLOW_UNPACK_8R:
      fprintf(fp, "unpack 8r");
      break;

   case DATAFLOW_SHIFT_RIGHT:
      fprintf(fp, ">>");
      break;
   case DATAFLOW_LOGICAL_SHR:
      fprintf(fp, "logical >>");
      break;
   case DATAFLOW_UNIFORM_ADDRESS:
      fprintf(fp, "uniform address");
      break;
   case DATAFLOW_SCOREBOARD_WAIT:
      fprintf(fp, "sbwait");
      break;
    case DATAFLOW_UNPACK_PLACEHOLDER_R :
       fprintf(fp, "unpack ph r");
       break;
    case DATAFLOW_UNPACK_PLACEHOLDER_B :
       fprintf(fp, "unpack ph b");
       break;

   default:
      UNREACHABLE();
      return;
   }

   fprintf(fp, " (%d)", DFlowNode_TreeDepth(dflow));

   // Print the preferred reg file
   fprintf(fp, "<%s>", DFlowRegFile_GetFileString(DFlowNode_GetRegFile(dflow)));
   if (QPUOperand_IsRegister(&dflow->m_result))
      fprintf(fp, "{%s}", ResultRegName(dflow));

   fprintf(fp, "\"");
   if (dflow->m_subtreeRoot)
      sprintf(buf, "#ff3333");
   else
      sprintf(buf, "#ffff80");
   fprintf(fp, " style=\"filled\" fillcolor=\"%s\"", buf);

   // Close label string.
   fprintf(fp, "];\n");
}

static void DFlowPrintVisitor_PrintEdge(DFlowPrintVisitor *self, DFlowPrintVisitor_EdgeStyle style, DFlowNode* supplier, DFlowNode* consumer)
{
   FILE  *fp = self->m_fp;

   if (consumer && supplier)
   {
      fprintf(fp, "\tn%x -> n%x", consumer ? (size_t)((consumer->m_uniqueId << 16) | consumer->m_flavour) : 0, supplier ? (size_t)((supplier->m_uniqueId << 16) | supplier->m_flavour) : 0);

      switch (style) {
      case DFlowPrintVisitor_EDGE_SOLID:
         break;
      case DFlowPrintVisitor_EDGE_DASHED:
         fprintf(fp, " [style=dashed]");
         break;
      case DFlowPrintVisitor_EDGE_PARENT:
         fprintf(fp, " [color=blue]");
         break;
      case DFlowPrintVisitor_EDGE_SEQUENCE:
         fprintf(fp, " [constraint=false,color=red]");
         break;
      default:
         UNREACHABLE();
         break;
      }

      fprintf(fp, ";\n");
   }
}

static void DFlowPrintVisitor_PrintEdges(DFlowPrintVisitor *self, DFlowNode* dataflowNode)
{
   NodeList_const_iterator  iter;
   const NodeList          *kids = DFlowNode_Children(dataflowNode);
   const NodeList          *parents = DFlowNode_Parents(dataflowNode);
   const NodeList          *iokids = DFlowNode_IoChildren(dataflowNode);

   for (iter = NodeList_const_begin(kids); iter != NodeList_const_end(kids); NodeList_const_next(&iter))
   {
      // Dependency: dashed line.
      DFlowPrintVisitor_PrintEdge(self, DFlowPrintVisitor_EDGE_SOLID, NodeList_const_star(iter), dataflowNode);
   }

   /* Uncomment to show parent links in blue*/
   for (iter = NodeList_const_begin(parents); iter != NodeList_const_end(parents); NodeList_const_next(&iter))
   {
      // Parent: blue line
      DFlowPrintVisitor_PrintEdge(self, DFlowPrintVisitor_EDGE_PARENT, dataflowNode, NodeList_const_star(iter));
   }


   for (iter = NodeList_const_begin(iokids); iter != NodeList_const_end(iokids); NodeList_const_next(&iter))
   {
      // Dependency: dashed line.
      DFlowPrintVisitor_PrintEdge(self, DFlowPrintVisitor_EDGE_DASHED, NodeList_const_star(iter), dataflowNode);
   }
}

static void PrintVisitor_Destroy(void *me)
{
   DFlowPrintVisitor *self = (DFlowPrintVisitor *)me;

   if (self->m_fp != NULL)
   {
      // Print closing.
      fprintf(self->m_fp, "}\n");

      fclose(self->m_fp);
   }
}

void PrintVisitor_Accept(void *me, DFlowNode *node)
{
   DFlowPrintVisitor *self = (DFlowPrintVisitor *)me;

   if (self->m_fp != NULL)
   {
      DFlowPrintVisitor_PrintNode(self, node);
      DFlowPrintVisitor_PrintEdges(self, node);
   }
}

void DFlowPrintVisitor_Constr(DFlowPrintVisitor *self, DFlowRecursionOptimizer *opt, const char *filename)
{
   DFlowVisitor_Constr(self, PrintVisitor_Destroy, PrintVisitor_Accept, opt);

   self->m_fp = fopen(filename, "w");

   if (self->m_fp)
   {
      // Print opening.
      fprintf(self->m_fp, "digraph DFlow\n");
      fprintf(self->m_fp, "{\n");
   }
}

void DFlowPrintVisitor_Destr(DFlowPrintVisitor *self)
{
   DFlowVisitor_Destr(self);
}

void DFlowPrintVisitor_Visit(DFlowPrintVisitor *self, DFlowNode *node)
{
   DFlowVisitor_VisitTopDown(self, node);
}
