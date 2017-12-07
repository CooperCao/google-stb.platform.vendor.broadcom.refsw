/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
   glsl_backflow_chain_push_back(chain, dataflow);
}

typedef enum
{
   EDGE_SOLID,
   EDGE_DASHED,
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
   default:
      unreachable();
      break;
   }

   fprintf(f, ";\n");
}

static void print_edges(FILE *f, Backflow *node)
{
   for (unsigned i=0; i<BACKFLOW_DEP_COUNT; ++i) {
      if (node->dependencies[i] != NULL)
         print_edge(f, EDGE_SOLID, node->dependencies[i], node);
   }

   for (BackflowIODepChainNode *n=node->io_dependencies.head; n; n=n->next) {
      // Dependency: dashed line.
      print_edge(f, EDGE_DASHED, n->val.dep, node);
   }
}

static void print_node(FILE *f, Backflow *backflow)
{
   // Print node reference and open label string.
   fprintf(f, "\tn%p [label=\"", backflow);

   // Print contents of label string.
   switch (backflow->type) {
      case SIG:
         switch (backflow->u.sigbits) {
            case V3D_QPU_SIG_LDUNIF: fprintf(f, "uniform"); break;
            case V3D_QPU_SIG_LDTMU:  fprintf(f, "tmu");     break;
            case V3D_QPU_SIG_LDTLB:  fprintf(f, "ldtlb");   break;
            case V3D_QPU_SIG_LDTLBU: fprintf(f, "ldtlbu");  break;

            case V3D_QPU_SIG_SMALL_IMM:  /* These are either not generated at all or */
            case V3D_QPU_SIG_LDVARY:     /* will use a different node type (not SIG) */
            case V3D_QPU_SIG_THRSW:
            case V3D_QPU_SIG_UCB:
            case V3D_QPU_SIG_ROTATE:  assert(0);  break;
#if V3D_VER_AT_LEAST(4,0,2,0)
            case V3D_QPU_SIG_WRTMUC: fprintf(f, "wrtmuc");  break;
#else
            case V3D_QPU_SIG_LDVPM:  fprintf(f, "vpm");     break;
#endif
#if V3D_VER_AT_LEAST(4,1,34,0)
            case V3D_QPU_SIG_LDUNIFRF:  fprintf(f, "ldunifrf");  break;
            case V3D_QPU_SIG_LDUNIFA:   fprintf(f, "ldunifa");   break;
            case V3D_QPU_SIG_LDUNIFARF: fprintf(f, "ldunifarf"); break;
#endif
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
      case ALU_M:
         switch(backflow->u.alu.op) {
            case BACKFLOW_UMUL:   fprintf(f, "umul");   break;
            case BACKFLOW_SMUL:   fprintf(f, "smul");   break;
            case BACKFLOW_MULTOP: fprintf(f, "multop"); break;
            case BACKFLOW_MUL:    fprintf(f, "mul");    break;
            case BACKFLOW_MOV:    fprintf(f, "mov");    break;
            case BACKFLOW_FMOV:   fprintf(f, "fmov");   break;
            default: unreachable();
         }
         break;
      case ALU_A:
         switch(backflow->u.alu.op) {
            case BACKFLOW_ADD:        fprintf(f, "+");        break;
            case BACKFLOW_VFPACK:     fprintf(f, "vfpack");   break;
            case BACKFLOW_IADD:       fprintf(f, "i+");       break;
            case BACKFLOW_ISUB:       fprintf(f, "i-");       break;
            case BACKFLOW_SUB:        fprintf(f, "-");        break;
            case BACKFLOW_IMIN:       fprintf(f, "imin");     break;
            case BACKFLOW_IMAX:       fprintf(f, "imax");     break;
            case BACKFLOW_UMIN:       fprintf(f, "umin");     break;
            case BACKFLOW_UMAX:       fprintf(f, "umax");     break;
            case BACKFLOW_SHL:        fprintf(f, "<<");       break;
            case BACKFLOW_SHR:        fprintf(f, ">>");       break;
            case BACKFLOW_ASHR:       fprintf(f, "a>>");      break;
            case BACKFLOW_ROR:        fprintf(f, "r>>");      break;
            case BACKFLOW_MIN:        fprintf(f, "min");      break;
            case BACKFLOW_MAX:        fprintf(f, "max");      break;
            case BACKFLOW_AND:        fprintf(f, "&");        break;
            case BACKFLOW_OR:         fprintf(f, "|");        break;
            case BACKFLOW_XOR:        fprintf(f, "^");        break;
            case BACKFLOW_NOT:        fprintf(f, "~");        break;
            case BACKFLOW_INEG:       fprintf(f, "ineg");     break;
            case BACKFLOW_SETMSF:     fprintf(f, "setmsf");   break;
            case BACKFLOW_SETREVF:    fprintf(f, "setrevf");  break;
            case BACKFLOW_TIDX:       fprintf(f, "tidx");     break;
            case BACKFLOW_EIDX:       fprintf(f, "eidx");     break;
            case BACKFLOW_FL:         fprintf(f, "fl");       break;
            case BACKFLOW_FLN:        fprintf(f, "fln");      break;
            case BACKFLOW_FXCD:       fprintf(f, "fxcd");     break;
            case BACKFLOW_XCD:        fprintf(f, "xcd");      break;
            case BACKFLOW_FYCD:       fprintf(f, "fycd");     break;
            case BACKFLOW_YCD:        fprintf(f, "ycd");      break;
            case BACKFLOW_MSF:        fprintf(f, "msf");      break;
            case BACKFLOW_REVF:       fprintf(f, "revf");     break;
#if V3D_VER_AT_LEAST(4,0,2,0)
            case BACKFLOW_IID:        fprintf(f, "iid");      break;
            case BACKFLOW_SAMPID:     fprintf(f, "sampid");   break;
            case BACKFLOW_BARRIERID:  fprintf(f, "barrierid"); break;
#endif
            case BACKFLOW_TMUWT:      fprintf(f, "tmuwt");    break;
#if V3D_VER_AT_LEAST(4,0,2,0)
            case BACKFLOW_VPMWT:      fprintf(f, "vpmwt");    break;
            case BACKFLOW_LDVPMV_IN:  fprintf(f, "ldvpmv_in");  break;
            case BACKFLOW_LDVPMD_IN:  fprintf(f, "ldvpmd_in");  break;
            case BACKFLOW_LDVPMV_OUT: fprintf(f, "ldvpmv_out"); break;
            case BACKFLOW_LDVPMD_OUT: fprintf(f, "ldvpmd_out"); break;
            case BACKFLOW_LDVPMP:     fprintf(f, "ldvpmp");     break;
            case BACKFLOW_LDVPMG_IN:  fprintf(f, "ldvpmg_in");  break;
            case BACKFLOW_LDVPMG_OUT: fprintf(f, "ldvpmg_out"); break;
            case BACKFLOW_STVPMV:     fprintf(f, "stvpmv");     break;
            case BACKFLOW_STVPMD:     fprintf(f, "stvpmd");     break;
            case BACKFLOW_STVPMP:     fprintf(f, "stvpmp");     break;
#else
            case BACKFLOW_VPMSETUP:   fprintf(f, "vpmsetup"); break;
#endif
            case BACKFLOW_NEG:        fprintf(f, "neg");      break;
            case BACKFLOW_FCMP:       fprintf(f, "fcmp");     break;
            case BACKFLOW_ROUND:      fprintf(f, "round");    break;
            case BACKFLOW_FTOIN:      fprintf(f, "ftoin");    break;
            case BACKFLOW_TRUNC:      fprintf(f, "trunc");    break;
            case BACKFLOW_FTOIZ:      fprintf(f, "ftoiz");    break;
            case BACKFLOW_FLOOR:      fprintf(f, "floor");    break;
            case BACKFLOW_FTOUZ:      fprintf(f, "ftouz");    break;
            case BACKFLOW_CEIL:       fprintf(f, "ceil");     break;
            case BACKFLOW_FTOC:       fprintf(f, "ftoc");     break;
            case BACKFLOW_FDX:        fprintf(f, "fdx");      break;
            case BACKFLOW_FDY:        fprintf(f, "fdy");      break;
            case BACKFLOW_ITOF:       fprintf(f, "itof");     break;
            case BACKFLOW_CLZ:        fprintf(f, "clz");      break;
            case BACKFLOW_UTOF:       fprintf(f, "utof");     break;
            default: unreachable();
         }
         break;
      default:
         unreachable();
   }

   /* TODO: Print out the unpacks here */

   switch (backflow->type) {
      case ALU_M:
      case ALU_A:
         switch (backflow->magic_write) {
            case REG_UNDECIDED:   break;
            case REG_MAGIC_NOP:     fprintf(f, " -> [nop]");   break;
            case REG_MAGIC_TLB:     fprintf(f, " -> tlb");     break;
            case REG_MAGIC_TLBU:    fprintf(f, " -> tlbu");    break;
            case REG_MAGIC_RECIP:   fprintf(f, " -> rcp");     break;
            case REG_MAGIC_RSQRT:   fprintf(f, " -> rsqrt");   break;
            case REG_MAGIC_EXP:     fprintf(f, " -> exp2");    break;
            case REG_MAGIC_LOG:     fprintf(f, " -> log2");    break;
            case REG_MAGIC_SIN:     fprintf(f, " -> sin");     break;
            case REG_MAGIC_RSQRT2:  fprintf(f, " -> rsqrt2");  break;
            case REG_MAGIC_TMUD:    fprintf(f, " -> tmud");    break;
            case REG_MAGIC_TMUA:    fprintf(f, " -> tmua");    break;
            case REG_MAGIC_TMUAU:   fprintf(f, " -> tmuau");   break;
#if V3D_VER_AT_LEAST(4,0,2,0)
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
#else
            case REG_MAGIC_TMU:     fprintf(f, " -> tmu");     break;
            case REG_MAGIC_TMUL:    fprintf(f, " -> tmul");    break;
            case REG_MAGIC_VPM:     fprintf(f, " -> vpm");     break;
            case REG_MAGIC_VPMU:    fprintf(f, " -> vpmu");    break;
#endif
            default: unreachable(); break;
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
      case BACKEND_UNIFORM_PLAIN:            fprintf(f, "\\nu%d", backflow->unif);   break;
      case BACKEND_UNIFORM_LITERAL:          fprintf(f, "\\n0x%x", backflow->unif);  break;
      case BACKEND_UNIFORM_ADDRESS:          fprintf(f, "\\na[%d]", 4 * (backflow->unif & 0xffff) + (backflow->unif >> 16));      break;
      case BACKEND_UNIFORM_UBO_ADDRESS:      fprintf(f, "\\nubo: %d, o: %d", backflow->unif & 0x1f, backflow->unif >> 5);       break;
      case BACKEND_UNIFORM_SSBO_ADDRESS:     fprintf(f, "\\nssbo: %d, o: %d", backflow->unif & 0x1f, backflow->unif >> 5);      break;
      case BACKEND_UNIFORM_ATOMIC_ADDRESS:   fprintf(f, "\\natomic: %d, o: %d", backflow->unif >> 16, backflow->unif & 0xffff); break;
      case BACKEND_UNIFORM_SSBO_SIZE:        fprintf(f, "\\nssbo size: %d", backflow->unif); break;
      case BACKEND_UNIFORM_SSBO_ARRAY_LENGTH:fprintf(f, "\\nssbo array length: %d", backflow->unif); break;
      case BACKEND_UNIFORM_UBO_SIZE:         fprintf(f, "\\nubo size: %d", backflow->unif); break;
      case BACKEND_UNIFORM_UBO_ARRAY_LENGTH: fprintf(f, "\\nubo array length: %d", backflow->unif); break;
      case BACKEND_UNIFORM_TEX_PARAM0:       fprintf(f, "\\ntex_parm 0: %d", backflow->unif); break;
      case BACKEND_UNIFORM_TEX_PARAM1:       fprintf(f, "\\ntex_parm 1: %d", backflow->unif); break;
#if V3D_VER_AT_LEAST(4,0,2,0)
      case BACKEND_UNIFORM_TEX_PARAM1_UNNORMS:
                                             fprintf(f, "\\ntex_parm 1 unnorm array: %d", backflow->unif); break;
#endif
      case BACKEND_UNIFORM_TEX_SIZE_X:       fprintf(f, "\\ntex_size z: %d", backflow->unif); break;
      case BACKEND_UNIFORM_TEX_SIZE_Y:       fprintf(f, "\\ntex_size y: %d", backflow->unif); break;
      case BACKEND_UNIFORM_TEX_SIZE_Z:       fprintf(f, "\\ntex_size z: %d", backflow->unif); break;
      case BACKEND_UNIFORM_TEX_LEVELS:       fprintf(f, "\\ntex_levels: %d", backflow->unif); break;
      case BACKEND_UNIFORM_IMG_PARAM0:       fprintf(f, "\\nimg_parm 0: %d", backflow->unif); break;
#if !V3D_VER_AT_LEAST(4,0,2,0)
      case BACKEND_UNIFORM_IMG_PARAM1:       fprintf(f, "\\nimg_parm 1: %d", backflow->unif); break;
      case BACKEND_UNIFORM_TEX_BASE_LEVEL:
      case BACKEND_UNIFORM_TEX_BASE_LEVEL_FLOAT:
      case BACKEND_UNIFORM_IMAGE_ARR_STRIDE:
      case BACKEND_UNIFORM_IMAGE_SWIZZLING:
      case BACKEND_UNIFORM_IMAGE_XOR_ADDR:
      case BACKEND_UNIFORM_IMAGE_LX_ADDR:
      case BACKEND_UNIFORM_IMAGE_LX_PITCH:
      case BACKEND_UNIFORM_IMAGE_LX_SLICE_PITCH:
#endif
      case BACKEND_UNIFORM_IMG_SIZE_X:
      case BACKEND_UNIFORM_IMG_SIZE_Y:
      case BACKEND_UNIFORM_IMG_SIZE_Z:       fprintf(f, "\\nu??? %d", backflow->unif_type); break;
      case BACKEND_UNIFORM_SPECIAL:          fprintf(f, "\\ns %d", backflow->unif);  break;

      default: unreachable();
   }

   // Close label string.
   fprintf(f, "\"];\n");
}

// Outputs graphviz dot representation of these nodes,
static void print_backflow_from_chain(FILE *f, BackflowChain *chain)
{
   assert(chain != NULL);
   assert(chain->head != NULL);

   // Print opening.
   fprintf(f, "digraph " GRAPHVIZ_GRAPH_NAME "\n");
   fprintf(f, "{\n");

   // Declare all the nodes
   for (BackflowChainNode *n=chain->head; n; n=n->next)
      print_node(f, n->val);

   // Print all the edges
   for (BackflowChainNode *n=chain->head; n; n=n->next)
      print_edges(f, n->val);

   // Print closing.
   fprintf(f, "}\n");
}

void glsl_print_backflow_from_roots(FILE *f, Backflow **roots, int num_roots, const BackflowChain *iodeps)
{
   BackflowChain chain;
   glsl_backflow_chain_init(&chain);

   BackflowVisitor *v = glsl_backflow_visitor_begin(&chain, NULL, dpostv_gather);

   /* Gather all the nodes into one big chain */
   for (int i=0; i<num_roots; i++) {
      glsl_backflow_visit(roots[i], v);
   }
   for (BackflowChainNode *n=iodeps->head; n; n=n->next)
      glsl_backflow_visit(n->val, v);

   glsl_backflow_visitor_end(v);

   print_backflow_from_chain(f, &chain);
}
#else
/* keep Metaware happy by providing an exported symbol */
void glsl_print_backflow_from_roots(FILE *f, BackflowChain *roots)
{
   (void)(f);
   (void)(roots);
}
#endif // _DEBUG
