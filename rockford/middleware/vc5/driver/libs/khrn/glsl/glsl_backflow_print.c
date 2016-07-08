/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_common.h"

#include "glsl_backflow.h"
#include "glsl_backend.h"
#include "glsl_backflow_visitor.h"
#include "glsl_backend_uniforms.h"
#include "glsl_fastmem.h"

#ifndef NDEBUG

#include <stdlib.h>

#define GRAPHVIZ_GRAPH_NAME         "Scheduler_DAG"

static void dpostv_gather(Backflow* dataflow, void* data)
{
   BackflowChain *chain = (BackflowChain *)data;
   assert(chain != NULL);

   // Add this node to the chain.
   glsl_backflow_chain_append(chain, dataflow);
}

typedef enum
{
   EDGE_SOLID,
   EDGE_DASHED,
   EDGE_SEQUENCE,
   EDGE_STYLE_COUNT
} EdgeStyle;

static void print_edge(FILE* f, EdgeStyle style, Backflow* supplier, Backflow* consumer)
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

static void print_edges(FILE *f, Backflow *node)
{
   BackflowChainNode *n;
   unsigned int i;

   for (i=0; i<BACKFLOW_DEP_COUNT; ++i) {
      if (node->dependencies[i] != NULL)
         print_edge(f, EDGE_SOLID, node->dependencies[i], node);
   }

   LIST_FOR_EACH(n, &node->io_dependencies, l)
   {
      // Dependency: dashed line.
      print_edge(f, EDGE_DASHED, n->ptr, node);
   }
}

/* TODO: There are some unpack problems here. {MIN,MAX}ABS, for example */
static struct alu_a_op {
   uint32_t op;
   uint32_t op1;
   uint32_t op2;
   const char *s;
} aops [] = {
   { 0,   ~0u, ~0u, "+"        },
   { 53,  ~0u, ~0u, "pack"     },
   { 56,  ~0u, ~0u, "i+"       },
   { 60,  ~0u, ~0u, "i-"       },
   { 64,  ~0u, ~0u, "-"        },
   { 120, ~0u, ~0u, "imin"     },
   { 121, ~0u, ~0u, "imax"     },
   { 122, ~0u, ~0u, "umin"     },
   { 123, ~0u, ~0u, "umax"     },
   { 124, ~0u, ~0u, "<<"       },
   { 125, ~0u, ~0u, "shr"      },
   { 126, ~0u, ~0u, "ashr"     },
   { 128, ~0u, ~0u, "min/max"  },     /* TODO: Distinguish. It can be important */
   { 181, ~0u, ~0u, "&"        },
   { 182, ~0u, ~0u, "|"        },
   { 183, ~0u, ~0u, "^"        },
   { 186,  0,  ~0u, "~"        },
   { 186,  1,  ~0u, "ineg"     },
   { 186,  6,  ~0u, "msf"      },
   { 187,  0,   1, "tidx"      },
   { 187,  1,   0, "fxcd"      },
   { 187,  1,   3, "xcd"       },
   { 187,  1,   4, "fycd"      },
   { 187,  1,   7, "ycd"       },
   { 187,  2,   0, "getmsf"    },
   { 187,  2,   1, "ff"        },
   { 187,  2,   5, "tmuwt"     },
   { 187,  3,  ~0u, "vpmsetup" },
   { 191,  0,  ~0u, "neg"      },
   { 192, ~0u, ~0u, "fcmp"     },
   { 245,  0,  ~0u, "nearest"  },
   { 245,  3,  ~0u, "ftoin"    },
   { 245,  4,  ~0u, "trunc"    },
   { 245,  7,  ~0u, "ftoiz"    },
   { 246,  0,  ~0u, "floor"    },
   { 246,  3,  ~0u, "ftouz"    },
   { 246,  4,  ~0u, "ceil"     },
   { 246,  7,  ~0u, "ftoc"     },
   { 247,  0,  ~0u, "fdx"      },
   { 247,  3,  ~0u, "isinf"    },
   { 247,  4,  ~0u, "fdy"      },
   { 247,  7,  ~0u, "isnan"    },
   { 252,  0,  ~0u, "itof"     },
   { 252,  4,  ~0u, "utof"     },
   { ~0u, ~0u, ~0u, "sentinel" }
};



static void print_node(FILE *f, Backflow *backflow)
{
   // Print node reference and open label string.
   fprintf(f, "\tn%p [label=\"", backflow);

   // Print contents of label string.
   switch (backflow->type) {
      case SIG:
         switch (backflow->sigbits) {
            case SIGBIT_LDUNIF: fprintf(f, "uniform"); break;
            case SIGBIT_LDTMU:  fprintf(f, "tmu");     break;
            case SIGBIT_LDTLB:  fprintf(f, "ldtlb");   break;
            case SIGBIT_LDTLBU: fprintf(f, "ldtlbu");  break;
            case SIGBIT_LDVPM:  fprintf(f, "vpm");     break;
            case SIGBIT_WRTMUC: fprintf(f, "wrtmuc");  break;
         }
         break;
      case SPECIAL_THRSW:
         fprintf(f, "thrsw");
         break;
      case SPECIAL_VOID:
         fprintf(f, "???");
         break;
      case SPECIAL_IMUL32:
         fprintf(f, "imul32");
         break;
      case SPECIAL_VARYING:
         fprintf(f, "varying");
         break;
      case ALU_MOV:
      case ALU_FMOV:
         /* TODO: Unpacks */
         fprintf(f, "mov");
         break;
      case ALU_M:
         switch(backflow->op) {
            /* TODO: Do the opcodes properly */
            case 3:  fprintf(f, "umul");   break;
            case 9:  fprintf(f, "smul");   break;
            case 10: fprintf(f, "multop"); break;
            case 21: fprintf(f, "mul");    break;
         }
         break;
      case ALU_A:
      case ALU_A_SWAP0:
      case ALU_A_SWAP1:
      {
         int i = 0;
         while (aops[i].op != backflow->op && aops[i+1].op <= backflow->op) i++;
         /* TODO: Some kind of bounds checking? */
         while (aops[i].op1 < backflow->op1 || aops[i].op2 < backflow->op2) i++;
         fprintf(f, "%s", aops[i].s);
         break;
      }
      default:
         UNREACHABLE();
   }

   switch (backflow->type) {
      case ALU_MOV:
      case ALU_FMOV:
      case ALU_M:
      case ALU_A:
      case ALU_A_SWAP0:
      case ALU_A_SWAP1:
         switch (backflow->magic_write) {
            case REG_UNDECIDED:   break;
            case REG_MAGIC_NOP:     fprintf(f, " -> [nop]");   break;
            case REG_MAGIC_TLB:     fprintf(f, " -> tlb");     break;
            case REG_MAGIC_TLBU:    fprintf(f, " -> tlbu");    break;
            case REG_MAGIC_TMU:     fprintf(f, " -> tmu");     break;
            case REG_MAGIC_TMUL:    fprintf(f, " -> tmul");    break;
            case REG_MAGIC_TMUD:    fprintf(f, " -> tmud");    break;
            case REG_MAGIC_TMUA:    fprintf(f, " -> tmua");    break;
            case REG_MAGIC_TMUAU:   fprintf(f, " -> tmuau");   break;
            case REG_MAGIC_VPM:     fprintf(f, " -> vpm");     break;
            case REG_MAGIC_VPMU:    fprintf(f, " -> vpmu");    break;
            case REG_MAGIC_RECIP:   fprintf(f, " -> rcp");     break;
            case REG_MAGIC_RSQRT:   fprintf(f, " -> rsqrt");   break;
            case REG_MAGIC_EXP:     fprintf(f, " -> exp2");    break;
            case REG_MAGIC_LOG:     fprintf(f, " -> log2");    break;
            case REG_MAGIC_SIN:     fprintf(f, " -> sin");     break;
            case REG_MAGIC_RSQRT2:  fprintf(f, " -> rsqrt2");  break;
#if V3D_HAS_NEW_TMU_CFG
            case REG_MAGIC_TMUC:    fprintf(f, " -> tmuc");    break;
            case REG_MAGIC_TMUS:    fprintf(f, " -> tmus");    break;
            case REG_MAGIC_TMUT:    fprintf(f, " -> tmut");    break;
            case REG_MAGIC_TMUR:    fprintf(f, " -> tmur");    break;
            case REG_MAGIC_TMUI:    fprintf(f, " -> tmui");    break;
            case REG_MAGIC_TMUB:    fprintf(f, " -> tmub");    break;
            case REG_MAGIC_TMUDREF: fprintf(f, " -> tmudref"); break;
            case REG_MAGIC_TMUOFF:  fprintf(f, " -> tmuoff");  break;
            case REG_MAGIC_TMUSCM:  fprintf(f, " -> tmuscm");  break;
            case REG_MAGIC_TMUSF:   fprintf(f, " -> tmusf");   break;
            case REG_MAGIC_TMUSLOD: fprintf(f, " -> tmuslod"); break;
#endif
            default: UNREACHABLE(); break;
         }

         switch(backflow->cond_setf) {
            case SETF_NONE: break;
            case SETF_PUSHZ: fprintf(f, " [pushz]"); break;
            case SETF_PUSHN: fprintf(f, " [pushn]"); break;
            case SETF_PUSHC: fprintf(f, " [pushc]"); break;
            case SETF_ANDZ:  fprintf(f, " [andz]");  break;
            case SETF_ANDNZ: fprintf(f, " [andnz]"); break;
            case SETF_NORNZ: fprintf(f, " [nornz]"); break;
            case SETF_NORZ:  fprintf(f, " [norz]");  break;
            case SETF_ANDN:  fprintf(f, " [andn]");  break;
            case SETF_ANDNN: fprintf(f, " [andnn]"); break;
            case SETF_NORNN: fprintf(f, " [nornn]"); break;
            case SETF_NORN:  fprintf(f, " [norn]");  break;
            case SETF_ANDC:  fprintf(f, " [andc]");  break;
            case SETF_ANDNC: fprintf(f, " [andnc]"); break;
            case SETF_NORNC: fprintf(f, " [nornc]"); break;
            case SETF_NORC:  fprintf(f, " [norc]");  break;
            case COND_IFA:
            case COND_IFB:
            case COND_IFFLAG:  fprintf(f, " [if]"); break;
            case COND_IFNA:
            case COND_IFNB:
            case COND_IFNFLAG: fprintf(f, " [ifn]"); break;
         }
         break;
      default:
         /* Nothing */
         break;
   }

   switch (backflow->unif_type) {
      case BACKEND_UNIFORM_UNASSIGNED: break;
      case BACKEND_UNIFORM_PLAIN:      fprintf(f, "\\nu%d", backflow->unif);   break;
      case BACKEND_UNIFORM_LITERAL:    fprintf(f, "\\n0x%x", backflow->unif);  break;
      case BACKEND_UNIFORM_ADDRESS:    fprintf(f, "\\na[%d]", backflow->unif); break;
      case BACKEND_UNIFORM_SPECIAL:    fprintf(f, "\\ns %d", backflow->unif);  break;
      default: fprintf(f, "\\nu???");
   }

   // Close label string.
   fprintf(f, "\"];\n");
}

// Outputs graphviz dot representation of these nodes,
static void print_backflow_from_chain(FILE *f, BackflowChain *chain)
{
   BackflowChainNode *node;

   assert(chain != NULL);
   assert(chain->head != NULL);

   // Print opening.
   fprintf(f, "digraph " GRAPHVIZ_GRAPH_NAME "\n");
   fprintf(f, "{\n");

   // Declare all the nodes
   LIST_FOR_EACH(node, chain, l) print_node(f, node->ptr);

   // Print all the edges
   LIST_FOR_EACH(node, chain, l) print_edges(f, node->ptr);

   // Print closing.
   fprintf(f, "}\n");
}

void glsl_print_backflow_from_root(FILE *f, Backflow *root, int pass)
{
   BackflowChain chain;
   glsl_backflow_chain_init(&chain);
   glsl_backflow_accept_towards_leaves(root, &chain, NULL, dpostv_gather, pass);
   print_backflow_from_chain(f, &chain);
}

void glsl_print_backflow_from_roots(FILE *f, Backflow **roots, int num_roots, const BackflowChain *iodeps, int pass)
{
   BackflowChain chain;
   glsl_backflow_chain_init(&chain);

   /* Gather all the nodes into one big chain */
   for (int i=0; i<num_roots; i++) {
      glsl_backflow_accept_towards_leaves(roots[i], &chain, NULL, dpostv_gather, pass);
   }
   for (BackflowChainNode *n=iodeps->head; n; n=n->l.next)
      glsl_backflow_accept_towards_leaves(n->ptr, &chain, NULL, dpostv_gather, pass);

   print_backflow_from_chain(f, &chain);
}
#else
/* keep Metaware happy by providing an exported symbol */
void glsl_print_backflow_from_roots(FILE *f, BackflowChain *roots, int pass)
{
   UNUSED(f);
   UNUSED(roots);
   UNUSED(pass);
}
#endif // _DEBUG
