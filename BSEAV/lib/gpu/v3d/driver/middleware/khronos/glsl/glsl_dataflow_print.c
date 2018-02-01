/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "middleware/khronos/glsl/glsl_common.h"

#include "middleware/khronos/glsl/glsl_dataflow_visitor.h"
#include "middleware/khronos/glsl/glsl_dataflow_print.h"
#include "middleware/khronos/glsl/glsl_map.h"
#include "middleware/khronos/glsl/glsl_fastmem.h"
#include "middleware/khronos/glsl/2708/glsl_allocator_4.h"

#ifndef NDEBUG

#include <stdlib.h>

#define GRAPHVIZ_GRAPH_NAME         "Dataflow"
#define GRAPHVIZ_CLUSTER_PREFIX     "line_num"

static void dpostv_gather_by_line_num(Dataflow* dataflow, void* data)
{
   Map *map = (Map *)data;

   // See whether dataflow->line_num is in the map.
   DataflowChain* chain;
   if (dataflow->line_num == 0) dataflow->line_num = -1;     /* Because map thinks line_num is a pointer, 0 isn't valid */
   chain = (DataflowChain *)glsl_map_get(map, (void *)(uintptr_t)dataflow->line_num, true);

   // If it's not in the map, insert it.
   if (!chain)
   {
      chain = (DataflowChain *)malloc_fast(sizeof(DataflowChain));

      // Init.
      glsl_dataflow_chain_init(chain);

      // Insert.
      glsl_map_put(map, (void *)(uintptr_t)dataflow->line_num, chain);
   }

   // Add this node to the chain.
   glsl_dataflow_chain_append(chain, dataflow);
}

typedef enum
{
   EDGE_SOLID,
   EDGE_DASHED,
   EDGE_SEQUENCE,

   EDGE_STYLE_COUNT
} EdgeStyle;

static void print_edge(FILE* f, EdgeStyle style, Dataflow* supplier, Dataflow* consumer)
{
   fprintf(f, "\tn%p -> n%p", consumer, supplier);

   switch (style) {
   case EDGE_SOLID:
      break;
   case EDGE_DASHED:
      fprintf(f, " [style=dashed]");
      break;
   case EDGE_SEQUENCE:
      fprintf(f, " [constraint=false,color=red]");
      break;
   default:
      UNREACHABLE();
      break;
   }

   fprintf(f, ";\n");
}

static void print_edges(FILE* f, Dataflow* dataflow, bool cross)
{
   DataflowChainNode *node;

   // cross: print only edges that do cross a cluster boundary
   // !cross: print only edges that do not cross a cluster boundary

   int i;

   switch (dataflow->flavour)
   {
   case DATAFLOW_UNIFORM_ADDRESS:
      /* don't visit indexed_uniform_sampler.uniform */
      break;

   default:
      for (i=0; i<dataflow->dependencies_count; ++i) {
         if (dataflow->d.dependencies[i]->flavour != DATAFLOW_CONST_SAMPLER) {
            if (cross == (dataflow->line_num != dataflow->d.dependencies[i]->line_num)) {
               print_edge(f, EDGE_SOLID, dataflow->d.dependencies[i], dataflow);
            }
         }
      }
   }

   for (node = dataflow->iodependencies.first; node; node = node->next)
   {
      if (cross == (dataflow->line_num != node->dataflow->line_num))
      {
         // Dependency: dashed line.
         print_edge(f, EDGE_DASHED, node->dataflow, dataflow);
      }
   }
}

static void print_node(FILE* f, Dataflow* dataflow)
{
   // Print node reference and open label string.
   fprintf(f, "\tn%p [label=\"", dataflow);

   // Print contents of label string.
   switch (dataflow->flavour)
   {
      case DATAFLOW_CONST_BOOL:
         fprintf(f, dataflow->u.const_bool.value ? "true" : "false");
         break;

      case DATAFLOW_CONST_INT:
         fprintf(f, "%d", dataflow->u.const_int.value);
         break;

      case DATAFLOW_CONST_FLOAT:
         {
            mixed_float t;
            t.ui = dataflow->u.const_float.value;
            fprintf(f, "%f", t.f);
         }
         break;

      case DATAFLOW_CONST_SAMPLER:
         fprintf(f, "sampler[%d] from <%s>", dataflow->u.const_sampler.location, dataflow->u.const_sampler.name);
         break;

      case DATAFLOW_UNIFORM:
         fprintf(f, "uniform[%d] from <%s>", dataflow->u.linkable_value.row, dataflow->u.linkable_value.name);
         break;
      case DATAFLOW_ATTRIBUTE:
         fprintf(f, "attribute[%d] from <%s>", dataflow->u.linkable_value.row, dataflow->u.linkable_value.name);
         break;
      case DATAFLOW_VARYING:
         fprintf(f, "varying[%d] from <%s>", dataflow->u.linkable_value.row, dataflow->u.linkable_value.name);
         break;

      case DATAFLOW_UNIFORM_OFFSET:
         fprintf(f, "uniform offset");
         break;

      case DATAFLOW_ARITH_NEGATE:
         fprintf(f, "-");
         break;
      case DATAFLOW_LOGICAL_NOT:
         fprintf(f, "!");
         break;
   case DATAFLOW_FTOI_TRUNC:
      fprintf(f, "ftoi_trunc");
      break;
   case DATAFLOW_FTOI_NEAREST:
      fprintf(f, "ftoi_nearest");
      break;
      case DATAFLOW_INTRINSIC_RSQRT:
         fprintf(f, "rsqrt");
         break;
      case DATAFLOW_INTRINSIC_CEIL:
         fprintf(f, "ceil");
         break;
      case DATAFLOW_INTRINSIC_FLOOR:
         fprintf(f, "floor");
         break;
      case DATAFLOW_INTRINSIC_SIGN:
         fprintf(f, "sign");
         break;
      case DATAFLOW_INTRINSIC_RCP:
         fprintf(f, "rcp");
         break;
      case DATAFLOW_INTRINSIC_LOG2:
         fprintf(f, "log2");
         break;
      case DATAFLOW_INTRINSIC_EXP2:
         fprintf(f, "exp2");
         break;

      case DATAFLOW_MOV:
         fprintf(f, "mov");
         break;

      case DATAFLOW_ITOF:
      fprintf(f, "itof");
         break;

      case DATAFLOW_MUL:
         fprintf(f, "*");
         break;
      case DATAFLOW_ADD:
         fprintf(f, "+");
         break;
      case DATAFLOW_SUB:
      case DATAFLOW_RSUB:
         fprintf(f, "-");
         break;
      case DATAFLOW_LESS_THAN:
         fprintf(f, "<");
         break;
      case DATAFLOW_LESS_THAN_EQUAL:
         fprintf(f, "<=");
         break;
      case DATAFLOW_GREATER_THAN:
         fprintf(f, ">");
         break;
      case DATAFLOW_GREATER_THAN_EQUAL:
         fprintf(f, ">=");
         break;
      case DATAFLOW_EQUAL:
         fprintf(f, "==");
         break;
      case DATAFLOW_NOT_EQUAL:
         fprintf(f, "!=");
         break;
      case DATAFLOW_LOGICAL_AND:
         fprintf(f, "&&");
         break;
      case DATAFLOW_LOGICAL_XOR:
         fprintf(f, "^^");
         break;
      case DATAFLOW_LOGICAL_OR:
         fprintf(f, "||");
         break;
      case DATAFLOW_INTRINSIC_MIN:
         fprintf(f, "min");
         break;
      case DATAFLOW_INTRINSIC_MAX:
         fprintf(f, "max");
         break;
      case DATAFLOW_INTRINSIC_MINABS:
         fprintf(f, "minabs");
         break;
      case DATAFLOW_INTRINSIC_MAXABS:
         fprintf(f, "maxabs");
         break;

      case DATAFLOW_CONDITIONAL:
         fprintf(f, "cond");
         break;

      case DATAFLOW_THREADSWITCH:
         fprintf(f, "thrsw");
         break;

      case DATAFLOW_TEX_SET_COORD_S:
         fprintf(f, "set coord s");
         break;
      case DATAFLOW_TEX_SET_COORD_T:
         fprintf(f, "set coord t");
         break;
      case DATAFLOW_TEX_SET_COORD_R:
         fprintf(f, "set coord r");
         break;
      case DATAFLOW_TEX_SET_BIAS:
         fprintf(f, "set bias");
         break;
      case DATAFLOW_TEX_SET_LOD:
         fprintf(f, "set lod");
         break;

      case DATAFLOW_TEX_GET_CMP_R:
         fprintf(f, "get cmp r");
         break;
      case DATAFLOW_TEX_GET_CMP_G:
         fprintf(f, "get cmp g");
         break;
      case DATAFLOW_TEX_GET_CMP_B:
         fprintf(f, "get cmp b");
         break;
      case DATAFLOW_TEX_GET_CMP_A:
         fprintf(f, "get cmp a");
         break;

      case DATAFLOW_FRAG_GET_X:
         fprintf(f, "get frag x");
         break;
      case DATAFLOW_FRAG_GET_Y:
         fprintf(f, "get frag y");
         break;
      case DATAFLOW_FRAG_GET_Z:
         fprintf(f, "get frag z");
         break;
      case DATAFLOW_FRAG_GET_W:
         fprintf(f, "get frag w");
         break;
      case DATAFLOW_FRAG_GET_PC_X:
         fprintf(f, "get frag pc x");
         break;
      case DATAFLOW_FRAG_GET_PC_Y:
         fprintf(f, "get frag pc y");
         break;
      case DATAFLOW_FRAG_GET_FF:
         fprintf(f, "get frag ff");
         break;

      case DATAFLOW_BITWISE_NOT:
         fprintf(f, "~");
         break;
      case DATAFLOW_BITWISE_AND:
         fprintf(f, "&");
         break;
      case DATAFLOW_BITWISE_OR:
         fprintf(f, "|");
         break;
      case DATAFLOW_BITWISE_XOR:
         fprintf(f, "^");
         break;
      case DATAFLOW_V8MULD:
         fprintf(f, "v8 *");
         break;
      case DATAFLOW_V8MIN:
         fprintf(f, "v8 min");
         break;
      case DATAFLOW_V8MAX:
         fprintf(f, "v8 max");
         break;
      case DATAFLOW_V8ADDS:
         fprintf(f, "v8 +");
         break;
      case DATAFLOW_V8SUBS:
         fprintf(f, "v8 -");
         break;
      case DATAFLOW_INTEGER_ADD:
         fprintf(f, "int +");
         break;
      case DATAFLOW_VARYING_C:
         fprintf(f, "vary c");
         break;
      case DATAFLOW_FRAG_GET_COL:
         fprintf(f, "get frag col");
         break;
      case DATAFLOW_FRAG_SUBMIT_STENCIL:
         fprintf(f, "set frag stencil");
         break;
      case DATAFLOW_FRAG_SUBMIT_Z:
         fprintf(f, "set frag z");
         break;
      case DATAFLOW_FRAG_SUBMIT_MS:
         fprintf(f, "set frag ms");
         break;
      case DATAFLOW_FRAG_SUBMIT_ALL:
         fprintf(f, "set frag all");
         break;
      case DATAFLOW_FRAG_SUBMIT_R0:
         fprintf(f, "set r0");
         break;
      case DATAFLOW_FRAG_SUBMIT_R1:
         fprintf(f, "set r1");
         break;
      case DATAFLOW_FRAG_SUBMIT_R2:
         fprintf(f, "set r2");
         break;
      case DATAFLOW_FRAG_SUBMIT_R3:
         fprintf(f, "set r3");
         break;
      case DATAFLOW_TMU_SWAP:
         fprintf(f, "set tmu swap");
         break;
      case DATAFLOW_TEX_SET_DIRECT:
         fprintf(f, "set direct lookup");
         break;
      case DATAFLOW_VERTEX_SET:
         fprintf(f, "set vertex data");
         break;
      case DATAFLOW_VPM_READ_SETUP:
         fprintf(f, "set vpm read setup");
         break;
      case DATAFLOW_VPM_WRITE_SETUP:
         fprintf(f, "set vpm write setup");
         break;
      case DATAFLOW_PACK_COL_R:
         fprintf(f, "pack col r");
         break;
      case DATAFLOW_PACK_COL_G:
         fprintf(f, "pack col g");
         break;
      case DATAFLOW_PACK_COL_B:
         fprintf(f, "pack col b");
         break;
      case DATAFLOW_PACK_COL_A:
         fprintf(f, "pack col a");
         break;
      case DATAFLOW_PACK_16A:
         fprintf(f, "pack 16a");
         break;
      case DATAFLOW_PACK_16B:
         fprintf(f, "pack 16b");
         break;
      case DATAFLOW_UNPACK_COL_R:
         fprintf(f, "unpack col r");
         break;
      case DATAFLOW_UNPACK_COL_G:
         fprintf(f, "unpack col g");
         break;
      case DATAFLOW_UNPACK_COL_B:
         fprintf(f, "unpack col b");
         break;
      case DATAFLOW_UNPACK_COL_A:
         fprintf(f, "unpack col a");
         break;
      case DATAFLOW_UNPACK_16A:
         fprintf(f, "unpack 16a");
         break;
      case DATAFLOW_UNPACK_16A_F:
         fprintf(f, "unpack 16a float");
         break;
      case DATAFLOW_UNPACK_16B:
         fprintf(f, "unpack 16b");
         break;
      case DATAFLOW_UNPACK_16B_F:
         fprintf(f, "unpack 16b float");
         break;
      case DATAFLOW_UNPACK_8A:
         fprintf(f, "unpack 8a");
         break;
      case DATAFLOW_UNPACK_8B:
         fprintf(f, "unpack 8b");
         break;
      case DATAFLOW_UNPACK_8C:
         fprintf(f, "unpack 8c");
         break;
      case DATAFLOW_UNPACK_8D:
         fprintf(f, "unpack 8d");
         break;
      case DATAFLOW_UNPACK_8R:
         fprintf(f, "unpack 8r");
         break;
      case DATAFLOW_SHIFT_RIGHT:
         fprintf(f, ">>");
         break;
      case DATAFLOW_LOGICAL_SHR:
         fprintf(f, "logical >>");
         break;
      case DATAFLOW_UNIFORM_ADDRESS:
         fprintf(f, "uniform address");
         break;
      case DATAFLOW_SCOREBOARD_WAIT:
         fprintf(f, "sbwait");
         break;

      default:
         UNREACHABLE();
         return;
   }

   if (dataflow->slot == -1)
      fprintf(f, " -> ?");
   else
      if (dataflow->slot & DATAFLOW_SLOT_VRF)
         fprintf(f, " -> v%d", dataflow->slot & DATAFLOW_SLOT_MASK);
      else
         fprintf(f, " -> s%d", dataflow->slot);

   fprintf(f, " (%d)\"", dataflow->delay);

   switch (dataflow->phase)
   {
   case BACKEND_PASS_AWAITING_SCHEDULE:
      fprintf(f, " style=\"filled\" fillcolor=\"#ffff80\"");
      break;
   case BACKEND_PASS_SCHEDULED:
      fprintf(f, " style=\"filled\" fillcolor=\"#ff8080\"");
      break;
   case BACKEND_PASS_DIVING:
      fprintf(f, " style=\"filled\" fillcolor=\"#8080ff\"");
      break;
   }

   // Close label string.
   fprintf(f, "];\n");
}

// map must map a line number to a DataflowChain, containing all Dataflow nodes for that line.
// Outputs graphviz dot representation of these nodes,
// clustering by line number.
static void print_dataflow_from_map(FILE* f, Map* map)
{
   DataflowChain* chain;
   DataflowChainNode* node;

   // For all line numbers...
   while ((chain = (DataflowChain *)glsl_map_pop(map)) != NULL)
   {
      int line_num;

      assert(chain->first);
      line_num = chain->first->dataflow->line_num;

      // Open cluster if we have a valid line number to group by.
      if (LINE_NUMBER_UNDEFINED != line_num)
      {
//         fprintf(f, "\tsubgraph cluster_" GRAPHVIZ_CLUSTER_PREFIX "_%d\n", line_num);
         fprintf(f, "\t{\n");
         fprintf(f, "\tlabel=\"line %d\";\n", line_num);
         fprintf(f, "\tstyle=filled;\n");
         fprintf(f, "\tfillcolor=\"0 0 .9\";\n");
      }

      // For all nodes...
      for (node = chain->first; node; node = node->next)
      {
         // Declare node.
         print_node(f, node->dataflow);

         // Add all the edges that don't cross a cluster boundary.
         print_edges(f, node->dataflow, false);
      }

      // Close cluster.
      if (LINE_NUMBER_UNDEFINED != line_num)
      {
         fprintf(f, "\t}\n");
      }

      // For all nodes...
      for (node = chain->first; node; node = node->next)
      {
         // Add remaining edges.
         print_edges(f, node->dataflow, true);
      }
   }
}

void glsl_print_dataflow_from_root(FILE* f, Dataflow* root, int pass)
{
   Map* map;

   // Initialize map with key comparator.
   map = glsl_map_new();

   glsl_allocator_mark_failed_nodes_on_stack();

   // Gather nodes into map.
   glsl_dataflow_accept_towards_leaves(root, map, NULL, dpostv_gather_by_line_num, pass);

   // Print opening.
   fprintf(f, "digraph " GRAPHVIZ_GRAPH_NAME "\n");
   fprintf(f, "{\n");

   // Print map.
   print_dataflow_from_map(f, map);

   // Print closing.
   fprintf(f, "}\n");
}

void glsl_print_dataflow_from_roots(FILE* f, DataflowChain* roots, DataflowChain *order, int pass)
{
   Map* map;
   DataflowChainNode *node;

   // Initialize map with key comparator.
   map = glsl_map_new();

   // Gather nodes into map.
   for (node = roots->first; node; node = node->next)
   {
      Dataflow* root = node->dataflow;
      glsl_dataflow_accept_towards_leaves(root, map, NULL, dpostv_gather_by_line_num, pass);
   }

   // Print opening.
   fprintf(f, "digraph " GRAPHVIZ_GRAPH_NAME "\n");
   fprintf(f, "{\n");

   // Print map.
   print_dataflow_from_map(f, map);

   // Print ordering edges (typically from scheduler)
   if (order)
      for (node = order->first; node; node = node->next)
         if (node->next)
            print_edge(f, EDGE_SEQUENCE, node->dataflow, node->next->dataflow);

   // Print closing.
   fprintf(f, "}\n");
}
#else
/*
   keep Metaware happy by providing an exported symbol
*/

void glsl_print_dataflow_from_roots(FILE* f, DataflowChain* roots, DataflowChain *order, int pass)
{
   UNUSED(f);
   UNUSED(roots);
   UNUSED(order);
   UNUSED(pass);
}
#endif // _DEBUG
