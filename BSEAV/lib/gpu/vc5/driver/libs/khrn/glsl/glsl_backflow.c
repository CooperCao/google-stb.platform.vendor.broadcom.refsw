/*=============================================================================
Broadcom Proprietary and Confidential. (c)2012 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file
File     :  $RCSfile: $
Revision :  $Revision: $

FILE DESCRIPTION
Translate a Dataflow graph to Backflow. This involves inserting hw-specific
constructs as well as the parts of the shader that depend on things other than
shader source.
=============================================================================*/
#include "glsl_common.h"
#include "glsl_backend_uniforms.h"
#include "glsl_backflow.h"
#include "glsl_backflow_visitor.h"
#include "glsl_backend.h"
#include "glsl_dataflow.h"
#include "glsl_dataflow_visitor.h"
#include "glsl_tex_params.h"

#include "glsl_sched_node_helpers.h"

#include "../glxx/glxx_int_config.h"
#include "../glxx/glxx_shader_cache.h"

#include "libs/util/gfx_util/gfx_util.h"

#include <assert.h>

static uint32_t dataflow_age = 0; /* Store this as static to avoid having to thread through Backflow constructors */

//
// Backflow chain functions (TODO: Possibly need a better home)
//
void glsl_backflow_chain_init(BackflowChain *chain)
{
   glsl_list_BackflowChainNode_init(chain);
}

void glsl_backflow_chain_append(BackflowChain *chain, Backflow *backflow)
{
   assert(backflow);
   glsl_node_list_Backflow_append(chain, backflow);
}

bool glsl_backflow_chain_contains(BackflowChain *chain, Backflow *backflow)
{
   return glsl_node_list_Backflow_contains(chain, backflow);
}

void glsl_backflow_chain_remove(BackflowChain *chain, Backflow *backflow)
{
   BackflowChainNode *node;
   LIST_FOR_EACH(node, chain, l) {
      if (node->ptr == backflow) {
         glsl_list_BackflowChainNode_remove(chain, node);
         return;
      }
   }
   unreachable();
}

/* Priority queue functions. TODO: Possibly need a better home */
void glsl_backflow_priority_queue_init(BackflowPriorityQueue *queue, int size)
{
   queue->size = size;
   queue->used = 0;
   queue->nodes = malloc_fast(size * sizeof(Backflow *));
}

static inline bool compare(const Backflow *d0, const Backflow *d1)
{
   return d1->age < d0->age;
}

static void siftDown(BackflowPriorityQueue *queue, int start, int end)
{
   int root = start;

   while (root * 2 + 1 <= end) {
      int child = (root << 1) + 1;

      if (child + 1 <= end && compare(queue->nodes[child], queue->nodes[child + 1]))
          child++;

      if (compare(queue->nodes[root], queue->nodes[child])) {
         Backflow *temp = queue->nodes[root];
         queue->nodes[root] = queue->nodes[child];
         queue->nodes[child] = temp;

         root = child;
      } else
         return;
   }
}

static void siftUp(BackflowPriorityQueue *queue, int root)
{
   while (root > 0) {
      int parent = (root - 1) >> 1;

      if (compare(queue->nodes[parent], queue->nodes[root])) {
         Backflow *temp = queue->nodes[root];
         queue->nodes[root] = queue->nodes[parent];
         queue->nodes[parent] = temp;

         root = parent;
      } else
         return;
   }
}

void glsl_backflow_priority_queue_heapify(BackflowPriorityQueue* queue)
{
   int start = (queue->used - 2) >> 1;

   while (start >= 0) {
      siftDown(queue, start, queue->used - 1);
      start--;
   }
}

void glsl_backflow_priority_queue_push(BackflowPriorityQueue* queue, Backflow *node)
{
   assert(queue->used < queue->size);

   queue->nodes[queue->used] = node;

   siftUp(queue, queue->used++);
}

Backflow *glsl_backflow_priority_queue_pop(BackflowPriorityQueue* queue)
{
   if (queue->used == 0) {
      return NULL;
   } else {
      Backflow *result = queue->nodes[0];

      queue->nodes[0] = queue->nodes[--queue->used];

      siftDown(queue, 0, queue->used - 1);

      return result;
   }
}


/* Helper functions */

/* TODO: We could do this *much* better by keeping the lists sorted */
static void tmu_dep_append(struct tmu_dep_s **dep_list, struct tmu_lookup_s *lookup) {
   struct tmu_dep_s *new_dep = malloc_fast(sizeof(struct tmu_dep_s));
   new_dep->l = lookup;
   new_dep->next = *dep_list;
   *dep_list = new_dep;
}

static bool tmu_dep_list_contains(const struct tmu_dep_s *list, const struct tmu_lookup_s *l) {
   while (list != NULL) {
      if (list->l == l) return true;
      list = list->next;
   }
   return false;
}

static void tmu_dep_merge(struct tmu_dep_s **dep_list, const struct tmu_dep_s *in_list) {
   while (in_list != NULL) {
      if (!tmu_dep_list_contains(*dep_list, in_list->l)) {
         tmu_dep_append(dep_list, in_list->l);
      }
      in_list = in_list->next;
   }
}

static void dep(Backflow *consumer, uint32_t i, Backflow *supplier)
{
   assert(i < BACKFLOW_DEP_COUNT);
   assert(consumer->dependencies[i] == NULL);
   consumer->dependencies[i] = supplier;
   if (supplier != NULL) {
      glsl_backflow_chain_append(&supplier->data_dependents, consumer);
      tmu_dep_merge(&consumer->tmu_deps, supplier->tmu_deps);
   }
}

void glsl_iodep(Backflow *consumer, Backflow *supplier)
{
   if (supplier != NULL) {
      supplier->any_io_dependents = true;
      glsl_backflow_chain_append(&consumer->io_dependencies, supplier);
      tmu_dep_merge(&consumer->tmu_deps, supplier->tmu_deps);
   }
}

bool glsl_sched_node_requires_regfile(BackflowFlavour f) {
#if V3D_VER_AT_LEAST(4,0,2,0)
   return f == BACKFLOW_LDVPMV_IN  || f == BACKFLOW_LDVPMD_IN  ||
          f == BACKFLOW_LDVPMV_OUT || f == BACKFLOW_LDVPMD_OUT ||
          f == BACKFLOW_LDVPMG_IN  || f == BACKFLOW_LDVPMG_OUT ||
          f == BACKFLOW_LDVPMP;
#else
   return false;
#endif
}

bool glsl_sched_node_admits_unpack(BackflowFlavour f) {
   switch(f) {
      case BACKFLOW_ADD:
      case BACKFLOW_SUB:
      case BACKFLOW_MIN:
      case BACKFLOW_MAX:
      case BACKFLOW_FCMP:
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

      case BACKFLOW_MUL:
      case BACKFLOW_FMOV:
         return true;

      default:
         return false;
   }
}

static SchedNodeType get_sched_node_type(BackflowFlavour f) {
   switch (f) {
      case BACKFLOW_MUL:
      case BACKFLOW_SMUL:
      case BACKFLOW_UMUL:
      case BACKFLOW_MOV:
      case BACKFLOW_FMOV:
         return ALU_M;

      case BACKFLOW_IMUL32:       return SPECIAL_IMUL32;
      case BACKFLOW_THREADSWITCH: return SPECIAL_THRSW;
      case BACKFLOW_DUMMY:        return SPECIAL_VOID;
      default:                    return ALU_A;
   }
}

static Backflow *create_internal() {
   Backflow *node = malloc_fast(sizeof(Backflow));
   memset(node, 0, sizeof(Backflow));

   glsl_backflow_chain_init(&node->data_dependents);
   glsl_backflow_chain_init(&node->io_dependencies);

   node->age = dataflow_age;
   node->unif_type = BACKEND_UNIFORM_UNASSIGNED;
   node->magic_write = REG_UNDECIDED;
   node->cond_setf = SETF_NONE;
   return node;
}

Backflow *create_sig(uint32_t sigbits) {
   Backflow *node = create_internal();
   node->type = SIG;
   node->u.sigbits = sigbits;
   return node;
}

Backflow *create_varying(VaryingType vary_type, uint32_t vary_row, Backflow *w)
{
   Backflow *node = create_internal();

   node->type = SPECIAL_VARYING;
   node->u.varying.type = vary_type;
   node->u.varying.row  = vary_row;

   dep(node, 1, w);
   return node;
}

Backflow *create_node(BackflowFlavour flavour, SchedNodeUnpack unpack, uint32_t cond_setf, Backflow *flag,
                      Backflow *left, Backflow *right, Backflow *output)
{
   Backflow *node = create_internal();

   assert(flavour < BACKFLOW_FLAVOUR_COUNT);
   node->type = get_sched_node_type(flavour);
   node->u.alu.op  = flavour;
   node->u.alu.unpack[0] = unpack;
   node->u.alu.unpack[1] = UNPACK_NONE;

   node->cond_setf = cond_setf;

   /* Verify that the flag field is used correctly */
   assert(flag == NULL || flavour   == BACKFLOW_FL || flavour   == BACKFLOW_FLN ||
                          cond_setf == COND_IFFLAG || cond_setf == COND_IFNFLAG ||
                          cs_is_updt(cond_setf));
   assert(output == NULL || cond_setf == COND_IFFLAG || cond_setf == COND_IFNFLAG);
   /* Could further validate that left and right are used consistently with the operation */

   dep(node, 0, flag);
   dep(node, 1, left);
   dep(node, 2, right);
   dep(node, 3, output);
   return node;
}

typedef enum {
   GLSL_TRANSLATION_UNVISITED = 0, /* This should be 0 as nodes start off cleared with zeroes */
   GLSL_TRANSLATION_VOID,
   GLSL_TRANSLATION_WORD,
   GLSL_TRANSLATION_VEC4, /* Only used for texture lookup nodes */
   GLSL_TRANSLATION_BOOL_FLAG,    /* (flag==0) = (reg!=0) = False, (flag==1) = (reg==0) = True */
   GLSL_TRANSLATION_BOOL_FLAG_N,  /* (flag==0) = (reg!=0) = True,  (flag==1) = (reg==0) = False */
} GLSL_TRANSLATION_TYPE_T;

typedef struct {
   GLSL_TRANSLATION_TYPE_T type;
   struct backflow_s *node[4];
#if !V3D_VER_AT_LEAST(4,0,2,0)
   bool per_sample;
#endif
} GLSL_TRANSLATION_T;

#if !V3D_VER_AT_LEAST(4,0,2,0)
typedef struct _GLSL_TRANSLATION_LIST_T {
   GLSL_TRANSLATION_T *value;
   struct _GLSL_TRANSLATION_LIST_T *next;
} GLSL_TRANSLATION_LIST_T;
#endif

/* TODO: How the backend handles this and the transition to per-sample is very unclear */
struct tlb_read_s {
   /* QPU does a chain of reads for each of 4 samples per RT */
   Backflow *first;
   Backflow *last;
#if !V3D_VER_AT_LEAST(4,0,2,0)
   uint8_t samples_read;
   Backflow *data[16];
#endif
};

typedef struct {
   const Dataflow *df_arr;

   /* Translated dataflow (looked up by dataflow->id) */
   GLSL_TRANSLATION_T *translations;
   int translations_count;

   bool ms;

#if !V3D_VER_AT_LEAST(4,0,2,0)
   /* These translations need to be done multiple times during multisampling */
   GLSL_TRANSLATION_LIST_T *per_sample_clear_list;
   int sample_num;      /* Which sample we're currently translating */
#endif

   bool *per_quad;

   struct tlb_read_s tlb_read[V3D_MAX_RENDER_TARGETS];

#if !V3D_VER_AT_LEAST(3,3,0,0)
   glsl_gadgettype_t gadgettype[GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS];
   glsl_gadgettype_t img_gadgettype[GLXX_CONFIG_MAX_IMAGE_UNITS];
#endif

   const LinkMap *link_map;

   SchedShaderInputs *in;
   SchedBlock *block;
} GLSL_TRANSLATE_CONTEXT_T;

static inline Backflow *tr_mov_to_reg_cond_io(uint32_t reg, Backflow *param, GLSL_TRANSLATION_T *cond, Backflow *iodep) {
   Backflow *result;

   if (cond == NULL)
      result = create_node(BACKFLOW_MOV, UNPACK_NONE, SETF_NONE, NULL, param, NULL, NULL);
   else if (cond->type == GLSL_TRANSLATION_BOOL_FLAG)
      result = create_node(BACKFLOW_MOV, UNPACK_NONE, COND_IFFLAG, cond->node[0], param, NULL, NULL);
   else
      result = create_node(BACKFLOW_MOV, UNPACK_NONE, COND_IFNFLAG, cond->node[0], param, NULL, NULL);

   assert(result->magic_write == REG_UNDECIDED);
   result->magic_write = reg;

   glsl_iodep(result, iodep);
   return result;
}

static inline Backflow *tr_cond(Backflow *cond, Backflow *true_value, Backflow *false_value, GLSL_TRANSLATION_TYPE_T rep)
{
   uint32_t cond_setf = (rep == GLSL_TRANSLATION_BOOL_FLAG) ? COND_IFFLAG : COND_IFNFLAG;
   assert(rep == GLSL_TRANSLATION_BOOL_FLAG || rep == GLSL_TRANSLATION_BOOL_FLAG_N);

   return create_node(BACKFLOW_MOV, UNPACK_NONE, cond_setf, cond, true_value, NULL, false_value);
}

static Backflow *tr_sin(Backflow *angle)
{
   Backflow *x = tr_binop(BACKFLOW_MUL, angle, tr_const(0x3ea2f983 /* 1 / pi */));
   Backflow *y = tr_uop(BACKFLOW_ROUND, x);
   Backflow *sfu_sin = tr_mov_to_reg(REG_MAGIC_SIN, tr_binop(BACKFLOW_SUB, x, y));
   Backflow *i = tr_uop(BACKFLOW_FTOIN, y);
   Backflow *il31 = tr_binop(BACKFLOW_SHL, i, tr_const(31));

   return tr_binop(BACKFLOW_XOR, sfu_sin, il31);
}

static Backflow *tr_cos(Backflow *angle)
{
   return tr_sin( tr_binop(BACKFLOW_ADD, angle, tr_const(0x3fc90fdb /* pi/2 */) ) );
}

static Backflow *tr_tan(Backflow *angle)
{
   Backflow *sin = tr_sin(angle);
   Backflow *one_on_cos = tr_mov_to_reg(REG_MAGIC_RECIP, tr_cos(angle));
   return tr_binop( BACKFLOW_MUL, sin, one_on_cos );
}

/*
 * void udiv (uint32_t a, uint32_t b, uint32_t * q, uint32_t *r)
 * {
 *    uint32_t Rd = b;
 *    uint32_t Rs = 0;
 *    uint32_t Rq = a;
 *
 *    for (int i=0; i<32; i++) {
 *       uint32_t carry1 = Rq >> 31; Rq <<= 1;               // shift with carry
 *       uint32_t carry2 = Rs >> 31; Rs <<= 1; Rs |= carry1; // shift with carry
 *       if (carry2 == 1 || Rs >= Rd) {
 *          Rs -= Rd;
 *          Rq |= 1;
 *       }
 *    }
 *
 *    *q = Rq;
 *    *r = Rs;
 * }
 */
static void unsigned_div_rem(Backflow **div, Backflow **rem, Backflow *l, Backflow *r) {
   Backflow *Rd = r;
   Backflow *Rs = tr_const(0);
   Backflow *Rq = l;
   Backflow *CONST1 = tr_const(1);
   Backflow *CONST31 = tr_const(31);

   for (int i=0; i<32; i++) {
      Backflow *c1, *c2, *ncond, *cond1;
      c1 = tr_binop(BACKFLOW_SHR, Rq, CONST31);
      Rq = tr_binop(BACKFLOW_SHL, Rq, CONST1);
      c2 = tr_binop(BACKFLOW_SHR, Rs, CONST31);
      Rs = tr_binop(BACKFLOW_SHL, Rs, CONST1);
      Rs = tr_binop(BACKFLOW_OR, Rs, c1);

      cond1 = tr_binop_push(BACKFLOW_XOR, SETF_PUSHZ, c2, CONST1);
      ncond = create_node(BACKFLOW_ISUB, UNPACK_NONE, SETF_NORNC, cond1, Rs, Rd, NULL);

      Rs = tr_cond(ncond, tr_binop(BACKFLOW_ISUB, Rs, Rd), Rs, GLSL_TRANSLATION_BOOL_FLAG_N);
      Rq = tr_cond(ncond, tr_binop(BACKFLOW_OR, Rq, CONST1), Rq, GLSL_TRANSLATION_BOOL_FLAG_N);
   }

   *div = Rq;
   *rem = Rs;
}

static void signed_div_rem(Backflow **div, Backflow **rem, Backflow *l, Backflow *r) {
   Backflow *cond_left  = tr_uop_cond(BACKFLOW_MOV, SETF_PUSHN, NULL, l);
   Backflow *cond_right = tr_uop_cond(BACKFLOW_MOV, SETF_PUSHN, NULL, r);
   Backflow *uleft  = tr_cond(cond_left,  tr_uop(BACKFLOW_INEG, l), l, GLSL_TRANSLATION_BOOL_FLAG);
   Backflow *uright = tr_cond(cond_right, tr_uop(BACKFLOW_INEG, r), r, GLSL_TRANSLATION_BOOL_FLAG);

   Backflow *udiv, *urem;
   unsigned_div_rem(&udiv, &urem, uleft, uright);

   udiv = tr_cond(cond_left,  tr_uop(BACKFLOW_INEG, udiv), udiv, GLSL_TRANSLATION_BOOL_FLAG);
   *div = tr_cond(cond_right, tr_uop(BACKFLOW_INEG, udiv), udiv, GLSL_TRANSLATION_BOOL_FLAG);
   *rem = tr_cond(cond_left,  tr_uop(BACKFLOW_INEG, urem), urem, GLSL_TRANSLATION_BOOL_FLAG);
}

static Backflow *tr_div(DataflowType type, Backflow *l, Backflow *r) {
   Backflow *div, *rem;
   switch (type) {
      case DF_INT:
         signed_div_rem(&div, &rem, l, r);
         return div;
      case DF_UINT:
         unsigned_div_rem(&div, &rem, l, r);
         return div;
      case DF_FLOAT:
         return tr_binop(BACKFLOW_MUL, l, tr_mov_to_reg(REG_MAGIC_RECIP, r));
      default:
         unreachable();
         return NULL;
   }
}

static Backflow *tr_rem(DataflowType type, Backflow *l, Backflow *r) {
   Backflow *div, *rem;
   switch (type) {
      case DF_INT:
         signed_div_rem(&div, &rem, l, r);
         return rem;
      case DF_UINT:
         unsigned_div_rem(&div, &rem, l, r);
         return rem;
      case DF_FLOAT:
      default:
         unreachable();
         return NULL;
   }
}

static void tr_comparison(GLSL_TRANSLATION_T *result, DataflowFlavour flavour, DataflowType type, Backflow *left, Backflow *right)
{
   struct cmp_s {
      BackflowFlavour flavour;
      uint32_t cond_setf;
      bool negate;
      bool reverse;
   };

   static const struct cmp_s ops[3][6] = {
   {              /* Float */
      /* Flavour        setf       negate  reverse */
      { BACKFLOW_FCMP, SETF_PUSHN, false,  false },     /* <  */
      { BACKFLOW_FCMP, SETF_PUSHN, false,  true  },     /* >  */
      { BACKFLOW_FCMP, SETF_PUSHC, false,  false },     /* <= */
      { BACKFLOW_FCMP, SETF_PUSHC, false,  true  },     /* >= */
      { BACKFLOW_FCMP, SETF_PUSHZ, false,  false },     /* == */
      { BACKFLOW_FCMP, SETF_PUSHZ, true,   false },     /* != */
   }, {           /* uint */
      { BACKFLOW_ISUB, SETF_PUSHC, false,  false },     /* <  */
      { BACKFLOW_ISUB, SETF_PUSHC, false,  true  },     /* >  */
      { BACKFLOW_ISUB, SETF_PUSHC, true,   true  },     /* <= */
      { BACKFLOW_ISUB, SETF_PUSHC, true,   false },     /* >= */
      { BACKFLOW_ISUB, SETF_PUSHZ, false,  false },     /* == */
      { BACKFLOW_ISUB, SETF_PUSHZ, true,   false },     /* != */
   }, {           /* int */
      { BACKFLOW_ISUB, SETF_PUSHN, false,  false },     /* <  */
      { BACKFLOW_ISUB, SETF_PUSHN, false,  true  },     /* >  */
      { BACKFLOW_ISUB, SETF_PUSHN, true,   true  },     /* <= */
      { BACKFLOW_ISUB, SETF_PUSHN, true,   false },     /* >= */
      { BACKFLOW_ISUB, SETF_PUSHZ, false,  false },     /* == */
      { BACKFLOW_ISUB, SETF_PUSHZ, true,   false },     /* != */
   }};
   const struct cmp_s *op;
   int type_idx, flavour_idx;

   switch (type) {
      case DF_FLOAT: type_idx = 0;  break;
      case DF_UINT:  type_idx = 1;  break;
      case DF_INT:   type_idx = 2; break;
      default: unreachable();
   }
   switch (flavour) {
      case DATAFLOW_LESS_THAN:          flavour_idx = 0; break;
      case DATAFLOW_GREATER_THAN:       flavour_idx = 1; break;
      case DATAFLOW_LESS_THAN_EQUAL:    flavour_idx = 2; break;
      case DATAFLOW_GREATER_THAN_EQUAL: flavour_idx = 3; break;
      case DATAFLOW_EQUAL:              flavour_idx = 4; break;
      case DATAFLOW_NOT_EQUAL:          flavour_idx = 5; break;
      default: unreachable();
   }

   op = &ops[type_idx][flavour_idx];
   Backflow *op_l = op->reverse ? right : left;
   Backflow *op_r = op->reverse ? left  : right;
   result->type = op->negate ? GLSL_TRANSLATION_BOOL_FLAG_N : GLSL_TRANSLATION_BOOL_FLAG;
   if (is_const_zero(op_r)) {
      BackflowFlavour f = (type == DF_FLOAT) ? BACKFLOW_FMOV : BACKFLOW_MOV;
      result->node[0] = tr_uop_cond(f, op->cond_setf, NULL, op_l);
   } else if (is_const_zero(op_l) && op->flavour == BACKFLOW_ISUB) {
      result->node[0] = tr_uop_cond(BACKFLOW_INEG, op->cond_setf, NULL, op_r);
   } else {
      result->node[0] = tr_binop_push(op->flavour, op->cond_setf, op_l, op_r);
   }
}

static void tr_logical_not(GLSL_TRANSLATION_T *result, GLSL_TRANSLATION_T *param)
{
   switch(param->type)
   {
   case GLSL_TRANSLATION_BOOL_FLAG:   result->type = GLSL_TRANSLATION_BOOL_FLAG_N; break;
   case GLSL_TRANSLATION_BOOL_FLAG_N: result->type = GLSL_TRANSLATION_BOOL_FLAG; break;
   default:
      unreachable();
   }
   result->node[0] = param->node[0];
}

static void tr_logical_and_or(GLSL_TRANSLATION_T *result, DataflowFlavour flavour, GLSL_TRANSLATION_T *left, GLSL_TRANSLATION_T *right)
{
   assert(flavour == DATAFLOW_LOGICAL_AND || flavour == DATAFLOW_LOGICAL_OR);
   assert(left->type  == GLSL_TRANSLATION_BOOL_FLAG || left->type  == GLSL_TRANSLATION_BOOL_FLAG_N);
   assert(right->type == GLSL_TRANSLATION_BOOL_FLAG || right->type == GLSL_TRANSLATION_BOOL_FLAG_N);

   bool l1 = left->type == GLSL_TRANSLATION_BOOL_FLAG;
   bool r1 = right->type == GLSL_TRANSLATION_BOOL_FLAG;

   GLSL_TRANSLATION_TYPE_T rep;
   if (flavour == DATAFLOW_LOGICAL_AND) {
      rep = GLSL_TRANSLATION_BOOL_FLAG;
   } else { /* DATAFLOW_LOGICAL_OR */
      /* A || B == !( !A && !B) */
      rep = GLSL_TRANSLATION_BOOL_FLAG_N;
      r1 = !r1; l1 = !l1;
   }

   uint32_t cond_setf;
   if      ( l1 &&  r1) cond_setf = SETF_ANDZ;
   else if ( l1 && !r1) cond_setf = SETF_ANDNZ;
   else if (!l1 &&  r1) cond_setf = SETF_NORNZ;
   else if (!l1 && !r1) cond_setf = SETF_NORZ;
   else unreachable();

   result->type = rep;
   result->node[0] = tr_uop_cond(BACKFLOW_MOV, cond_setf, left->node[0], right->node[0]);
}

static void tr_logical_binop(GLSL_TRANSLATION_T *result, DataflowFlavour flavour, GLSL_TRANSLATION_T *left, GLSL_TRANSLATION_T *right)
{
   GLSL_TRANSLATION_T leftn, rightn, a, b;

   assert(flavour == DATAFLOW_LOGICAL_XOR || flavour == DATAFLOW_NOT_EQUAL || flavour == DATAFLOW_EQUAL);

   tr_logical_not(&leftn, left);
   tr_logical_not(&rightn, right);

   tr_logical_and_or(&a, DATAFLOW_LOGICAL_AND, left, &rightn);
   tr_logical_and_or(&b, DATAFLOW_LOGICAL_AND, right, &leftn);
   tr_logical_and_or(result, DATAFLOW_LOGICAL_OR, &a, &b);

   if (flavour == DATAFLOW_EQUAL) tr_logical_not(result, result);
}

static void tr_logical_cond(GLSL_TRANSLATION_T *result, GLSL_TRANSLATION_T *cond, GLSL_TRANSLATION_T *true_value, GLSL_TRANSLATION_T *false_value)
{
   /* QPU does not support cond ops on boolean values. Build out of ANDs and ORs */
   GLSL_TRANSLATION_T a, b, condn;
   tr_logical_not(&condn, cond);
   tr_logical_and_or(&a, DATAFLOW_LOGICAL_AND, cond, true_value);
   tr_logical_and_or(&b, DATAFLOW_LOGICAL_AND, &condn, false_value);
   tr_logical_and_or(result, DATAFLOW_LOGICAL_OR, &a, &b);
}

Backflow* tr_pack_int16(Backflow* a, Backflow *b)
{
   Backflow *i65535 = tr_const(65535);
   Backflow *i16 = tr_const(16);

   a = bitand(a, i65535);
   b = bitand(b, i65535);

   return bitor(a, tr_binop(BACKFLOW_SHL, b, i16));
}

static Backflow *tr_texture_get(Backflow *iodep0, struct tmu_lookup_s *lookup)
{
   Backflow *result = tr_sig_io(SIGBIT_LDTMU, iodep0);

   /* Drop all the existing tmu deps. This stops the write's deps leaking
    * through to the reads (and, hence, everything subsequent).  */
   result->tmu_deps = NULL;
   tmu_dep_append(&result->tmu_deps, lookup);

   return result;
}

static struct tmu_lookup_s *new_tmu_lookup(SchedBlock *block) {
   struct tmu_lookup_s *new_lookup = malloc_fast(sizeof(struct tmu_lookup_s));
   new_lookup->is_modify = false;
   new_lookup->next = NULL;
   new_lookup->done = false;

   /* It would be easiest to add these at the head of the list, but we want
    * them to stay in order (I think) for sorting purposes                  */
   if (block->tmu_lookups == NULL) {
      block->tmu_lookups = new_lookup;
   } else {
      struct tmu_lookup_s *last = block->tmu_lookups;
      while(last->next != NULL) last = last->next;
      last->next = new_lookup;
   }

   return new_lookup;
}

static int get_word_reads(uint32_t components, bool is_32bit) {
   int word_reads = 0;

   if (is_32bit) {
      word_reads = components;
   } else {
      if (components & 0x3) word_reads |= 1;
      if (components & 0xC) word_reads |= 2;
   }
   assert(!(word_reads & ~0xf));
   return word_reads;
}

static Backflow *unpack_float(Backflow *in, bool high) {
   Backflow *res = tr_uop(BACKFLOW_FMOV, in);
   if (!high) res->u.alu.unpack[0] = UNPACK_F16_A;
   else       res->u.alu.unpack[0] = UNPACK_F16_B;
   return res;
}

static Backflow *unpack_uint(Backflow *in, bool high) {
   if (!high) return bitand(in, tr_const(0xffff));
   else       return shr(in, 16);
}

static Backflow *unpack_int(Backflow *in, bool high) {
   if (!high) return asr(shl(in, 16), 16);
   else       return asr(    in, 16);
}

static void unpack_tmu_return(Backflow *output[4], Backflow *words[4], DataflowType type, uint32_t components, bool is_32bit) {
   if (is_32bit) {
      for (int i=0; i<4; i++) output[i] = words[i];
   } else {
      if (type == DF_FSAMPLER || type == DF_FIMAGE) {
         for (int i=0; i<4; i++) if (components & (1<<i)) output[i] = unpack_float(words[i/2], i&1);
      } else if (type == DF_USAMPLER || type == DF_UIMAGE) {
         for (int i=0; i<4; i++) if (components & (1<<i)) output[i] = unpack_uint(words[i/2], i&1);
      } else {
         for (int i=0; i<4; i++) if (components & (1<<i)) output[i] = unpack_int (words[i/2], i&1);
      }
   }
}

/* The tr_begin/mid_tmu_access functions maintain these invariants:
 * - lookup->first_write is a dummy node that all other write nodes (possibly
 *   transitively) depend on.
 * - lookup->last_write is either a dummy node (if the retiring write node has
 *   not been created yet) or the retiring write node and depends (possibly
 *   transitively) on all other write nodes. */

static struct tmu_lookup_s *tr_begin_tmu_access(SchedBlock *block)
{
   struct tmu_lookup_s *lookup = new_tmu_lookup(block);
   lookup->first_write = tr_nullary(BACKFLOW_DUMMY);
   lookup->last_write = tr_nullary(BACKFLOW_DUMMY);
   glsl_iodep(lookup->last_write, lookup->first_write);
   lookup->write_count = 0;
   return lookup;
}

static void tr_mid_tmu_access_data(struct tmu_lookup_s *lookup, GLSL_TRANSLATION_T *data)
{
   assert(data->type == GLSL_TRANSLATION_VEC4);
   Backflow *node = lookup->first_write;
   for (int i=0; i<4; i++) {
      if (data->node[i] != NULL) {
         node = tr_mov_to_reg_io(REG_MAGIC_TMUD, data->node[i], node);
         lookup->write_count++;
      }
   }
   assert(node != lookup->first_write);
   glsl_iodep(lookup->last_write, node);
}

#if V3D_VER_AT_LEAST(4,0,2,0)
static Backflow *tr_texture_cfg(int i, uint32_t cfg, bool is_image, Backflow *dep) {
   static const BackendUniformFlavour t[2][3] = { { BACKEND_UNIFORM_TEX_PARAM0, BACKEND_UNIFORM_TEX_PARAM1, BACKEND_UNIFORM_LITERAL },
                                                  { BACKEND_UNIFORM_IMAGE_PARAM0, BACKEND_UNIFORM_IMAGE_PARAM1, BACKEND_UNIFORM_LITERAL } };
   Backflow *ret = tr_sig_io(SIGBIT_WRTMUC, dep);
   ret->unif_type = t[is_image ? 1 : 0][i];
   ret->unif = cfg;
   return ret;
}

static void tr_mid_tmu_access_texture(struct tmu_lookup_s *lookup,
   uint32_t texture_index, DataflowType type, bool is_32bit, bool per_quad,
   int word_reads, DFTexbits extra, v3d_tmu_op_t op, GLSL_TRANSLATION_T *cond,
   Backflow *s, Backflow *t, Backflow *r, Backflow *idx, Backflow *dref, Backflow *b, Backflow *off)
{
   bool fetch = (extra & (DF_TEXBITS_FETCH | DF_TEXBITS_SAMPLER_FETCH));
   bool sampler_fetch = (extra & DF_TEXBITS_SAMPLER_FETCH);
   uint32_t sampler_index = (!fetch || sampler_fetch) ? texture_index : GLSL_SAMPLER_NONE;

   bool is_image = (type == DF_FIMAGE || type == DF_IIMAGE || type == DF_UIMAGE);
   bool cubemap = (extra & DF_TEXBITS_CUBE);

   /* All images must be fetch mode and cubemaps appear as 2D arrays */
   assert(!is_image || (fetch && !cubemap));

   int n_cfg_words;
   uint32_t cfg_word[3];
   cfg_word[0] = (texture_index << 4) | word_reads;
   cfg_word[1] = (sampler_index << 4) | ((!per_quad) << 2) | (0 << 1) | is_32bit;

   bool gather  = (extra & DF_TEXBITS_GATHER);
   bool bslod   = (extra & DF_TEXBITS_BSLOD) || gather;
   uint32_t offset = 0;
   bool ind_off = (extra & DF_TEXBITS_I_OFF);
   if (!ind_off && off && is_const(off)) {
      offset = off->unif & 0xFFF;
      off = NULL;
   }

   if (gather || offset || ind_off || (bslod && cubemap) || (op != V3D_TMU_OP_REGULAR)) {
      int gather_comp = (extra & DF_TEXBITS_GATHER_COMP_MASK) >> DF_TEXBITS_GATHER_COMP_SHIFT;
      cfg_word[2] = (offset << 8) | (gather << 7) | (gather_comp << 5) | (bslod << 1) | (ind_off);
      cfg_word[2] |= (uint32_t)op << 20;
      n_cfg_words = 3;
   } else if (!fetch || sampler_fetch || is_32bit || per_quad)
      n_cfg_words = 2;
   else
      n_cfg_words = 1;

   Backflow *last_cfg = lookup->first_write;
   for (int i=0; i<n_cfg_words; i++) last_cfg = tr_texture_cfg(i, cfg_word[i], is_image, last_cfg);
   glsl_iodep(lookup->last_write, last_cfg);

   if (b && is_const_zero(b)) b = NULL;

   static const uint32_t reg[6] = { REG_MAGIC_TMUT, REG_MAGIC_TMUR, REG_MAGIC_TMUI, REG_MAGIC_TMUDREF, REG_MAGIC_TMUB, REG_MAGIC_TMUOFF };
   Backflow *coords[6] = { t, r, idx, dref, b, off };
   for (int i=0; i<6; i++) {
      if (coords[i] != NULL) {
         Backflow *node = tr_mov_to_reg_io(reg[i], coords[i], lookup->first_write);
         lookup->write_count++;
         glsl_iodep(lookup->last_write, node);
      }
   }

   uint32_t s_reg;
   if    (cubemap) s_reg = REG_MAGIC_TMUSCM;
   else if (fetch) s_reg = REG_MAGIC_TMUSF;
   else if (bslod) s_reg = REG_MAGIC_TMUSLOD;
   else            s_reg = REG_MAGIC_TMUS;
   lookup->last_write = tr_mov_to_reg_cond_io(s_reg, s, cond, lookup->last_write);
   lookup->write_count++;
}
#endif

/* Size 0 is valid for read-atomics, for which we set the size as 32-bit */
static const uint8_t tmu_sizes[] = { 7, 7, 2, 3, 4 };

static void tr_mid_tmu_access_general(struct tmu_lookup_s *lookup,
   v3d_tmu_op_t op, GLSL_TRANSLATION_T *cond, Backflow *addr)
{
   uint32_t cfg = 0xFFFFFF80 | ((uint32_t)op << 3) | tmu_sizes[lookup->write_count];
   if (cfg != 0xFFFFFFFF) {
      lookup->last_write = tr_mov_to_reg_cond_io(REG_MAGIC_TMUAU, addr, cond, lookup->last_write);
      lookup->last_write->unif_type = BACKEND_UNIFORM_LITERAL;
      lookup->last_write->unif = cfg;
   } else {
      lookup->last_write = tr_mov_to_reg_cond_io(REG_MAGIC_TMUA, addr, cond, lookup->last_write);
   }
   ++lookup->write_count;
}

static void tr_end_tmu_access(Backflow **words, struct tmu_lookup_s *lookup,
   int word_reads, uint32_t age)
{
   lookup->first_read = NULL;
   lookup->read_count = 0;
   Backflow *node = NULL;
   for (int i = 0; i < 4; i++) {
      if (word_reads & (1<<i)) {
         node = tr_texture_get(node, lookup);
         if (lookup->first_read == NULL) lookup->first_read = node;
         lookup->read_count++;
         words[i] = node;
      }
   }
   lookup->last_read = node;

   lookup->age = age;
}

#if V3D_VER_AT_LEAST(4,0,2,0)

static void tr_texture_lookup(GLSL_TRANSLATE_CONTEXT_T *ctx, GLSL_TRANSLATION_T *result,
                              uint32_t texture_index, DataflowType type, bool is_32bit, bool per_quad,
                              uint32_t age, uint32_t required_components, DFTexbits extra,
                              Backflow *s, Backflow *t, Backflow *r, Backflow *idx, Backflow *dref, Backflow *b, Backflow *off)
{
   struct tmu_lookup_s *lookup = tr_begin_tmu_access(ctx->block);
   int word_reads = get_word_reads(required_components, is_32bit);
   tr_mid_tmu_access_texture(lookup,
      texture_index, type, is_32bit, per_quad,
      word_reads, extra, V3D_TMU_OP_REGULAR, /*cond=*/NULL,
      s, t, r, idx, dref, b, off);
   Backflow *words[4] = {NULL};
   tr_end_tmu_access(words, lookup, word_reads, age);

   result->type = GLSL_TRANSLATION_VEC4;
   unpack_tmu_return(result->node, words, type, required_components, is_32bit);
}

#else

static uint32_t infer_ltype(Backflow *s, Backflow *t, Backflow *r, Backflow *idx, uint32_t cfg_bits) {
   static const uint32_t ltype[2][3] = { { GLSL_TEXPARAM0_1D, GLSL_TEXPARAM0_2D, GLSL_TEXPARAM0_3D },
                                         { GLSL_TEXPARAM0_1D_ARRAY, GLSL_TEXPARAM0_2D_ARRAY, -1} };
   if (cfg_bits & DF_TEXBITS_CUBE) return GLSL_TEXPARAM0_CUBE;
   else {
      int dim = (s != NULL) + (t != NULL) + (r != NULL);
      bool array = (idx != NULL);
      return ltype[array ? 1 : 0][dim-1];
   }
}

static void tr_texture_lookup(GLSL_TRANSLATE_CONTEXT_T *ctx, GLSL_TRANSLATION_T *result,
                              uint32_t sampler_index, DataflowType type, bool is_32bit, bool per_quad,
                              uint32_t age, uint32_t required_components, DFTexbits extra,
                              Backflow *s, Backflow *t, Backflow *r, Backflow *idx, Backflow *d, Backflow *b, Backflow *off)
{
   struct tmu_lookup_s *lookup = new_tmu_lookup(ctx->block);
   int word_reads = get_word_reads(required_components, is_32bit);

   uint32_t cfg0 = infer_ltype(s, t, r, idx, extra);
   uint32_t cfg1 = word_reads;
   if (extra & (DF_TEXBITS_FETCH | DF_TEXBITS_SAMPLER_FETCH)) {
      cfg0 |= GLSL_TEXPARAM0_FETCH;
      cfg1 |= GLSL_TEXPARAM1_FETCH;
   }
   if (extra & (DF_TEXBITS_GATHER)) {
      cfg0 |= GLSL_TEXPARAM0_GATHER;
      cfg1 |= GLSL_TEXPARAM1_GATHER;
      cfg1 |= ((extra & DF_TEXBITS_GATHER_COMP_MASK) >> DF_TEXBITS_GATHER_COMP_SHIFT) << GLSL_TEXPARAM1_GATHER_COMP_SHIFT;
   }
   if (extra & (DF_TEXBITS_BSLOD | DF_TEXBITS_GATHER)) cfg0 |= GLSL_TEXPARAM0_BSLOD;
   /* Copy the offsets to the parameter */
   if (off) {
      assert(is_const(off)); /* Non-const only supported with new TMU config */
      cfg0 |= (off->unif & 0xFFF) << 19;
   }

   if (!per_quad)
      cfg0 |= GLSL_TEXPARAM0_PIX_MASK;

   if (d != NULL) cfg0 |= GLSL_TEXPARAM0_SHADOW;
   if (b != NULL && !glsl_dataflow_tex_cfg_implies_bslod(extra)) cfg0 |= GLSL_TEXPARAM0_BIAS;

   // GFXH-1363: explicit LoD should be relative to base level but HW does not add base level...
   if (glsl_dataflow_tex_cfg_implies_bslod(extra)) {
      bool bias_int = extra & (DF_TEXBITS_FETCH | DF_TEXBITS_SAMPLER_FETCH); // Bias is an int for fetch, otherwise a float
      Backflow *base_level = tr_typed_uniform(
         bias_int ? BACKEND_UNIFORM_TEX_BASE_LEVEL : BACKEND_UNIFORM_TEX_BASE_LEVEL_FLOAT, sampler_index);
      if (is_const_zero(b))
         b = base_level;
      else
         b = tr_binop(bias_int ? BACKFLOW_IADD : BACKFLOW_ADD, base_level, b);
   }

   assert(s != NULL);
   if (t == NULL) {
      if (idx != NULL) { t = idx; idx = NULL; } /* 1D array: Compact array index down */
      else t = s;                               /* 1D: Write junk. s will do          */
   }

   assert(r == NULL || idx == NULL);     /* 3D or cubemap arrays not supported on this version */
   Backflow *c[3] = { NULL, };
   int n_extra_coords = 0;
   if (r   != NULL) c[n_extra_coords++] = r;
   if (idx != NULL) c[n_extra_coords++] = idx;
   if (d   != NULL) c[n_extra_coords++] = d;
   if (b   != NULL) c[n_extra_coords++] = b;

   bool is_image = (type == DF_FIMAGE || type == DF_IIMAGE || type == DF_UIMAGE);

   /* GFXH-1332: On v3.3 the bug is fixed and for images the LOD is always
    * clamped to the correct value, so 'b' is irrelevant */
   if (V3D_VER_AT_LEAST(3,3,0,0) || is_image || b == NULL) {
      while (n_extra_coords > 0 && is_const_zero(c[n_extra_coords-1])) {
         c[n_extra_coords-1] = NULL;
         n_extra_coords--;
      }
   }

   if (is_image) {
      assert(extra & DF_TEXBITS_FETCH);
      int ltype = (cfg0 & 7);
      assert(ltype != GLSL_TEXPARAM0_CUBE); /* Cubemap images should appear as 2D arrays here */
   }

   Backflow *node = tr_mov_to_reg(REG_MAGIC_TMU, s);
   node->unif_type = is_image ? BACKEND_UNIFORM_IMAGE_PARAM0 : BACKEND_UNIFORM_TEX_PARAM0;
   node->unif = BACKEND_UNIFORM_MAKE_PARAM(sampler_index, cfg0);
   lookup->first_write = node;

   node = tr_mov_to_reg_io( (n_extra_coords == 0) ? REG_MAGIC_TMUL : REG_MAGIC_TMU, t, node);
   node->unif_type = is_image ? BACKEND_UNIFORM_IMAGE_PARAM1 : BACKEND_UNIFORM_TEX_PARAM1;
   node->unif = BACKEND_UNIFORM_MAKE_PARAM(sampler_index, cfg1);

   lookup->write_count = 2 + n_extra_coords;

   for (int i=0; i<n_extra_coords; i++) {
      node = tr_mov_to_reg_io( (i == n_extra_coords-1) ? REG_MAGIC_TMUL : REG_MAGIC_TMU, c[i], node);
   }

   assert(lookup->write_count <= 5);
   assert(node != NULL);
   lookup->last_write = node;

   Backflow *words[4] = { NULL, };
   tr_end_tmu_access(words, lookup, word_reads, age);

   result->type = GLSL_TRANSLATION_VEC4;
   unpack_tmu_return(result->node, words, type, required_components, is_32bit);
}

#if !V3D_VER_AT_LEAST(3,3,0,0)
static void swizzle_words(Backflow *words[4], int swizzle[4], bool int_tex)
{
   Backflow *nodes[6];
   nodes[0] = tr_const(0);
   nodes[1] = int_tex ? tr_const(1) : tr_cfloat(1.0f);
   for (int i=0; i<4; i++) nodes[i+2] = words[i];

   for (int i=0; i<4; i++) words[i] = nodes[swizzle[i]];
}

static void tr_texture_gadget(GLSL_TRANSLATE_CONTEXT_T *ctx, GLSL_TRANSLATION_T *result, uint32_t sampler_index, DataflowType type,
                              bool is_32bit, bool per_quad, uint32_t age, uint32_t required_components, DFTexbits extra,
                              Backflow *s, Backflow *t, Backflow *r, Backflow *i, Backflow *d, Backflow *b, Backflow *off)
{
   int swizzle[4] = { 2, 3, 4, 5};
   uint32_t gadgettype;

   if (type == DF_FIMAGE || type == DF_IIMAGE || type == DF_UIMAGE) {
      assert(sampler_index < vcos_countof(ctx->img_gadgettype));
      gadgettype = ctx->img_gadgettype[sampler_index];
   } else {
      assert(sampler_index < vcos_countof(ctx->gadgettype));
      gadgettype = ctx->gadgettype[sampler_index];
   }

   if (GLSL_GADGETTYPE_NEEDS_SWIZZLE(gadgettype)) {
      /* For shader swizzling the required components are post swizzle. Work
       * out what we need pre-swizzling to request from the TMU             */
      uint32_t swizzled_components = required_components;

      for (int i=0; i<4; i++) swizzle[i] = (gadgettype >> (3*i)) & 7;
      gadgettype &= ~0xfff;

      required_components = 0;
      for (int i=0; i<4; i++)
        if (swizzle[i] >= 2 && swizzled_components & (1 << i)) required_components |= (1 << (swizzle[i]-2));
   }

   if (required_components != 0) {
      uint32_t lookup_required_components = required_components;
      if (d != NULL)
         assert(!is_32bit); /* Shadow lookups are always 16 bit */
      else switch (gadgettype)
      {
      case GLSL_GADGETTYPE_AUTO:
         break;
      case GLSL_GADGETTYPE_SWAP1632:
         is_32bit = !is_32bit;
         break;
      case GLSL_GADGETTYPE_DEPTH_FIXED:
      case GLSL_GADGETTYPE_INT32:
         is_32bit = true;
         break;
      case GLSL_GADGETTYPE_INT16:
         is_32bit = false;
         break;
      case GLSL_GADGETTYPE_INT8:
      case GLSL_GADGETTYPE_INT10_10_10_2:
         is_32bit = true;
         lookup_required_components = 1;
         break;
      default:
         unreachable();
      }

      /* For shadow clamp D_ref to [0,1] for fixed point textures. */
      if (gadgettype == GLSL_GADGETTYPE_DEPTH_FIXED && d != NULL) {
         d = tr_binop(BACKFLOW_MAX, tr_cfloat(0.0f), d);
         d = tr_binop(BACKFLOW_MIN, tr_cfloat(1.0f), d);
      }

      tr_texture_lookup(ctx, result, sampler_index, type, is_32bit, per_quad, age, lookup_required_components, extra, s, t, r, i, d, b, off);

      /* The dataflow type tells us how the texture was declared in the shader,
       * the gadgettype what type the actual texture is. If they don't match then
       * the values returned are undefined per the spec. We only pay attention to
       * the gadget to the extent required to get the defined behaviour right. */
      Backflow *word = result->node[0];
      switch (gadgettype)
      {
      case GLSL_GADGETTYPE_INT8:
         if (type == DF_USAMPLER || type == DF_UIMAGE) {
            if (required_components & 1) result->node[0] = bitand(    word,      tr_const(0xff));
            if (required_components & 2) result->node[1] = bitand(shr(word, 8),  tr_const(0xff));
            if (required_components & 4) result->node[2] = bitand(shr(word, 16), tr_const(0xff));
            if (required_components & 8) result->node[3] =        shr(word, 24);
         } else {
            if (required_components & 1) result->node[0] = asr(shl(word, 24), 24);
            if (required_components & 2) result->node[1] = asr(shl(word, 16), 24);
            if (required_components & 4) result->node[2] = asr(shl(word, 8),  24);
            if (required_components & 8) result->node[3] = asr(    word, 24);
         }
         break;
      case GLSL_GADGETTYPE_INT10_10_10_2:
         if (required_components & 1) result->node[0] = bitand(word,          tr_const(0x3ff));
         if (required_components & 2) result->node[1] = bitand(shr(word, 10), tr_const(0x3ff));
         if (required_components & 4) result->node[2] = bitand(shr(word, 20), tr_const(0x3ff));
         if (required_components & 8) result->node[3] =        shr(word, 30);
         break;

      default:
         /* Do nothing */
         break;
      }
   }

   /* Set type here in case all the results are coming from swizzles */
   result->type = GLSL_TRANSLATION_VEC4;
   swizzle_words(result->node, swizzle, (type != DF_FSAMPLER && type != DF_FIMAGE));
}
#endif
#endif

void tr_read_tlb(bool ms, uint32_t rt_num, uint32_t rt_type,
                 uint32_t required_components,
                 Backflow **result,             /* Array of 4 or 16, depending on cfg */
                 Backflow **first_read, Backflow **last_read)
{
   uint8_t config_byte;
   int len;
   if (rt_type == GLXX_FB_F16) {
      if (required_components & 0xC) len = 2;
      else                           len = 1;
      // .all_samples_same_data should not affect reads, but it does
      V3D_TLB_CONFIG_COLOR_F16_T cfg = { .num_words = len, .no_swap = true,
                                         .all_samples_same_data = false, .rt = rt_num };
      config_byte = v3d_pack_tlb_config_color_f16(&cfg);
   } else {
      len = gfx_msb(required_components) + 1;
      V3D_TLB_CONFIG_COLOR_32_T cfg = { .num_words = len, .all_samples_same_data = false,
                                        .rt = rt_num, .as_int = (rt_type == GLXX_FB_I32) };
      config_byte = v3d_pack_tlb_config_color_32(&cfg);
   }

   result[0] = create_sig(SIGBIT_LDTLBU);
   result[0]->unif_type = BACKEND_UNIFORM_LITERAL;
   result[0]->unif = config_byte | 0xffffff00; /* Unused config entries must be all 1s */

   unsigned load_len = (ms ? 4 : 1) * len;
   for (unsigned i=1; i<load_len; i++)
      result[i] = tr_sig_io(SIGBIT_LDTLB, result[i-1]);

   *first_read = result[0];
   *last_read  = result[load_len-1];

   /* Unpack the data. Loop backwards so that we can expand in place */
   if (rt_type == GLXX_FB_F16)
      for (unsigned i=2*load_len-1; i<2*load_len; i--) result[i] = unpack_float(result[i/2], i&1);
}

static void tr_get_col_gadget(GLSL_TRANSLATE_CONTEXT_T *ctx,
                              GLSL_TRANSLATION_T *result,
                              uint32_t rt_type,
                              uint32_t required_components,
                              uint32_t render_target)
{
#if V3D_VER_AT_LEAST(4,0,2,0)
   Backflow *data[16];
   tr_read_tlb(ctx->ms, render_target, rt_type, required_components, data, &ctx->tlb_read[render_target].first, &ctx->tlb_read[render_target].last);

   for (int i=0; i<4; i++) result->node[i] = data[i];

   if (ctx->ms) {
      Backflow *sampid = tr_nullary(BACKFLOW_SAMPID);
      for (int s = 1; s<4; s++) {
         Backflow *cond = tr_binop_push(BACKFLOW_ISUB, SETF_PUSHZ, sampid, tr_const(s));
         for (int i=0; i<4; i++)
            result->node[i] = create_node(BACKFLOW_MOV, UNPACK_NONE, COND_IFFLAG, cond, data[s*4 + i], NULL, result->node[i]);
      }
   }
#else
   if (ctx->tlb_read[render_target].samples_read == 0)
      tr_read_tlb(ctx->ms, render_target, rt_type, required_components, ctx->tlb_read[render_target].data, &ctx->tlb_read[render_target].first, &ctx->tlb_read[render_target].last);

   ctx->tlb_read[render_target].samples_read |= (1 << ctx->sample_num);
   for (int i=0; i<4; i++) result->node[i] = ctx->tlb_read[render_target].data[4*ctx->sample_num + i];
#endif

   result->type = GLSL_TRANSLATION_VEC4;
}

Backflow *glsl_backflow_fake_tmu(SchedBlock *block) {
   struct tmu_lookup_s *lookup = new_tmu_lookup(block);
   Backflow *zero = tr_const(0);
   Backflow *node = tr_uop_cond(BACKFLOW_MOV, COND_IFNFLAG, zero, zero);
   assert(node->magic_write == REG_UNDECIDED);
   node->magic_write = REG_MAGIC_TMUA;

   lookup->first_write = lookup->last_write = node;
   lookup->write_count = 1;
   lookup->read_count = 1;

   node = tr_texture_get(NULL, lookup);
   lookup->first_read = lookup->last_read = node;

   lookup->age = 0;  /* XXX Is this bad? */

   return node;
}

#define TMU_CFG_PIX_MASK (1<<7)

static void tr_indexed_read_vector_gadget(SchedBlock *block,
                                          GLSL_TRANSLATION_T *result,
                                          int age, Backflow *addr, uint32_t components, bool per_quad)
{
   int num_reads = 0;

   while (components != 0) {
      components >>= 1;
      num_reads++;
   }
   assert(num_reads > 0 && num_reads <= 4);

   struct tmu_lookup_s *lookup = new_tmu_lookup(block);

   Backflow *node = tr_mov_to_reg(REG_MAGIC_TMUAU, addr);
   node->unif_type = BACKEND_UNIFORM_LITERAL;
   node->unif = 0xFFFFFF78 | (per_quad ? 0 : TMU_CFG_PIX_MASK) | tmu_sizes[num_reads];

   if (node->unif == ~0u) {
      node->magic_write = REG_MAGIC_TMUA;
      node->unif_type = 0;
      node->unif = 0;
   }

   lookup->first_write = lookup->last_write = node;
   lookup->write_count = 1;

   result->type = GLSL_TRANSLATION_VEC4;
   result->node[0] = tr_texture_get(NULL, lookup);
   for (int i=1; i<num_reads; i++) result->node[i] = tr_texture_get(result->node[i-1], lookup);

   lookup->read_count = num_reads;
   lookup->first_read = result->node[0];
   lookup->last_read = result->node[num_reads-1];

   lookup->age = age;
}

static inline Backflow *varying_io(VaryingType vary_type, uint32_t vary_row, Backflow *w, Backflow *dep) {
   Backflow *res = create_varying(vary_type, vary_row, w);
   glsl_iodep(res, dep);
   return res;
}

static Backflow *init_frag_vary( SchedShaderInputs *in,
                                 uint32_t primitive_type,
                                 const VARYING_INFO_T *varying)
{
   Backflow *dep = NULL;

   if (primitive_type == GLXX_PRIM_POINT) {
      dep = in->point_x = varying_io(VARYING_DEFAULT, VARYING_ID_HW_0, in->w, dep);
      dep = in->point_y = varying_io(VARYING_DEFAULT, VARYING_ID_HW_1, in->w, dep);
   } else {
      in->point_x = tr_cfloat(0.0f);
      in->point_y = tr_cfloat(0.0f);
   }

   if (primitive_type == GLXX_PRIM_LINE)
      dep = in->line = varying_io(VARYING_LINE_COORD, VARYING_ID_HW_0, NULL, dep);
   else
      in->line = tr_cfloat(0.0f);

   memset(in->inputs, 0, sizeof(Backflow *)*(V3D_MAX_VARYING_COMPONENTS));
   for (int i = 0; i < V3D_MAX_VARYING_COMPONENTS; i++)
   {
      VaryingType t = (varying[i].flat ? VARYING_FLAT : VARYING_DEFAULT);
      Backflow *w;
      if      (varying[i].flat)     w = NULL;
      else if (varying[i].centroid) w = in->w_c;
      else                          w = in->w;

      in->inputs[i] = varying_io(t, i, w, dep);
   }

   return dep;
}

#if !V3D_VER_AT_LEAST(4,0,2,0)
/* We must load all samples for any RT that we read */
static void validate_tlb_loads(struct tlb_read_s *reads, bool ms) {
   for (int rt=0; rt<V3D_MAX_RENDER_TARGETS; rt++)
      assert(reads[rt].samples_read == 0 || reads[rt].samples_read == (ms ? 0xF : 0x1));
}
#endif

static void resolve_tlb_loads(SchedBlock *block, struct tlb_read_s *reads) {
   /* Sort out TLB loading dependencies */
   Backflow *first_read = NULL;
   Backflow *last_read = NULL;

   for (int rt=0; rt<V3D_MAX_RENDER_TARGETS; rt++) {
      struct tlb_read_s *r = &reads[rt];
      /* If this RT isn't read then skip it */
      if (r->first == NULL) continue;

      /* Record the first_read so we can force reads after the "sbwait" */
      if (first_read == NULL) {
         first_read = r->first;
         last_read  = r->last;
      } else {
         glsl_iodep(r->first, last_read);
      }
   }
   block->first_tlb_read = first_read;
   block->last_tlb_read  = last_read;
}

#if V3D_VER_AT_LEAST(4,0,2,0)

static Backflow *ldvpmv(SchedShaderInputs *ins, int addr) {
   Backflow *r = tr_uop(BACKFLOW_LDVPMV_IN, tr_const(addr));
   ins->vpm_dep[addr] = r;
   return r;
}

static void fetch_all_attribs(SchedShaderInputs *ins, const ATTRIBS_USED_T *attr, uint32_t reads_total)
{
   int addr = 0;
   if (attr->instanceid_used)   ins->instanceid   = ldvpmv(ins, addr++);
   if (attr->baseinstance_used) ins->baseinstance = ldvpmv(ins, addr++);
   if (attr->vertexid_used)     ins->vertexid     = ldvpmv(ins, addr++);

   for (int i = 0; i < V3D_MAX_ATTR_ARRAYS; i++) {
      for (unsigned j=0; j<attr->scalars_used[i]; j++) {
         ins->inputs[4*i+j] = ldvpmv(ins, addr++);
      }
   }
}

#else

static Backflow *tr_vpm_read_setup(uint32_t count, uint32_t addr)
{
   assert(count >= 1 && count <= 32);
   assert(addr  >= 0 && addr  <= 0x1fff);
   uint32_t value = 1<<30 | 1<<27 | 1<<15 | 2<<13 | 1<<29 | (count & 31) << 22 | (addr & 0x1fff);
   Backflow *result = tr_uop(BACKFLOW_VPMSETUP, tr_const(value));
   result->age = 0;     /* XXX Hack this to make it come out early */
   return result;
}

static void fetch_all_attribs(SchedShaderInputs *ins, const ATTRIBS_USED_T *attr, uint32_t reads_total)
{
   Backflow *attrs[V3D_MAX_ATTR_ARRAYS * 4 + 2];

   for (unsigned i=0; i<reads_total; i++) attrs[i] = create_sig(SIGBIT_LDVPM);

   assert(reads_total < MAX_VPM_DEPENDENCY);
   for (unsigned i=0; i<reads_total; i++) ins->vpm_dep[i] = attrs[i];

   unsigned attr_count = 0;
   if (attr->instanceid_used) ins->instanceid = attrs[attr_count++];
   if (attr->vertexid_used)   ins->vertexid   = attrs[attr_count++];
   assert(!attr->baseinstance_used);

   for (unsigned i = 0; i < V3D_MAX_ATTR_ARRAYS; i++) {
      for (unsigned j=0; j<attr->scalars_used[i]; j++) {
         ins->inputs[4*i+j] = attrs[attr_count++];
      }
   }

   Backflow *config[4];
   unsigned config_count = (reads_total + 31) / 32;
   assert(config_count < 5);
   for (unsigned i=0; i<config_count; i++)
      config[i] = tr_vpm_read_setup(gfx_umin(reads_total - 32*i, 32), 32*i);

   for (unsigned i=0; i<attr_count; i++) {
      if (i % 32 == 0) glsl_iodep(attrs[i], config[i/32]);
      else             glsl_iodep(attrs[i], attrs[i-1]);
   }
   for (unsigned i=1; i<config_count; i++) glsl_iodep(config[i], attrs[32*i-1]);

   ins->read_dep = attr_count > 0 ? attrs[attr_count-1] : NULL;
}

#endif

static inline GLSL_TRANSLATION_TYPE_T get_translation_type(DataflowType type)
{
   assert(type == DF_BOOL || type == DF_INT ||
          type == DF_UINT || type == DF_FLOAT);

   if (type == DF_BOOL) return GLSL_TRANSLATION_BOOL_FLAG_N;
   else return GLSL_TRANSLATION_WORD;
}

static BackflowFlavour translate_flavour(DataflowFlavour flavour, DataflowType type, int *params) {
   switch(flavour) {
#if V3D_VER_AT_LEAST(4,0,2,0)
      case DATAFLOW_SAMPLE_ID:         *params = 0; return BACKFLOW_SAMPID;
      case DATAFLOW_GET_INVOCATION_ID: *params = 0; return BACKFLOW_IID;
#endif
      case DATAFLOW_FRAG_GET_X:        *params = 0; return BACKFLOW_FXCD;
      case DATAFLOW_FRAG_GET_Y:        *params = 0; return BACKFLOW_FYCD;
      case DATAFLOW_FRAG_GET_X_UINT:   *params = 0; return BACKFLOW_XCD;
      case DATAFLOW_FRAG_GET_Y_UINT:   *params = 0; return BACKFLOW_YCD;
      case DATAFLOW_GET_THREAD_INDEX:  *params = 0; return BACKFLOW_TIDX;
      case DATAFLOW_FTOI_TRUNC:        *params = 1; return BACKFLOW_FTOIZ;
      case DATAFLOW_FTOI_NEAREST:      *params = 1; return BACKFLOW_FTOIN;
      case DATAFLOW_FTOU:              *params = 1; return BACKFLOW_FTOUZ;
      case DATAFLOW_ITOF:              *params = 1; return BACKFLOW_ITOF;
      case DATAFLOW_CLZ:               *params = 1; return BACKFLOW_CLZ;
      case DATAFLOW_UTOF:              *params = 1; return BACKFLOW_UTOF;
      case DATAFLOW_BITWISE_NOT:       *params = 1; return BACKFLOW_NOT;
      case DATAFLOW_ARITH_NEGATE:      *params = 1; return type == DF_FLOAT ? BACKFLOW_NEG: BACKFLOW_INEG;
      case DATAFLOW_FDX:               *params = 1; return BACKFLOW_FDX;
      case DATAFLOW_FDY:               *params = 1; return BACKFLOW_FDY;
      case DATAFLOW_TRUNC:             *params = 1; return BACKFLOW_TRUNC;
      case DATAFLOW_NEAREST:           *params = 1; return BACKFLOW_ROUND;
      case DATAFLOW_CEIL:              *params = 1; return BACKFLOW_CEIL;
      case DATAFLOW_FLOOR:             *params = 1; return BACKFLOW_FLOOR;
      case DATAFLOW_MUL:               *params = 2; return type == DF_FLOAT ? BACKFLOW_MUL : BACKFLOW_IMUL32;
      case DATAFLOW_ADD:               *params = 2; return type == DF_FLOAT ? BACKFLOW_ADD : BACKFLOW_IADD;
      case DATAFLOW_SUB:               *params = 2; return type == DF_FLOAT ? BACKFLOW_SUB : BACKFLOW_ISUB;
      case DATAFLOW_MIN:               *params = 2; return type == DF_FLOAT ? BACKFLOW_MIN :
                                                         ( type == DF_INT ? BACKFLOW_IMIN : BACKFLOW_UMIN );
      case DATAFLOW_MAX:               *params = 2; return type == DF_FLOAT ? BACKFLOW_MAX :
                                                         ( type == DF_INT ? BACKFLOW_IMAX : BACKFLOW_UMAX );
      case DATAFLOW_FPACK:             *params = 2; return BACKFLOW_VFPACK;
      case DATAFLOW_SHL:               *params = 2; return BACKFLOW_SHL;
      case DATAFLOW_SHR:               *params = 2; return type == DF_UINT ? BACKFLOW_SHR : BACKFLOW_ASHR;
      case DATAFLOW_ROR:               *params = 2; return BACKFLOW_ROR;
      case DATAFLOW_BITWISE_AND:       *params = 2; return BACKFLOW_AND;
      case DATAFLOW_BITWISE_OR:        *params = 2; return BACKFLOW_OR;
      case DATAFLOW_BITWISE_XOR:       *params = 2; return BACKFLOW_XOR;
      default: unreachable(); return 0;
   }
}

static uint32_t get_magic_reg(DataflowFlavour flavour) {
   switch (flavour) {
      case DATAFLOW_RSQRT:  return REG_MAGIC_RSQRT;
      case DATAFLOW_RCP:    return REG_MAGIC_RECIP;
      case DATAFLOW_LOG2:   return REG_MAGIC_LOG;
      case DATAFLOW_EXP2:   return REG_MAGIC_EXP;
      default: unreachable(); return 0;
   }
}

static void push_phi(const Dataflow *d, PhiList **list) {
   PhiList *new = malloc_fast(sizeof(PhiList));
   new->phi = d;
   new->next = *list;
   *list = new;
}

Backflow *tr_external(int block, int output, ExternalList **list) {
   /* If this already exists in the list we must return it, otherwise we would
    * have more than one node that would start the block in a single register */
   for (ExternalList *old = *list; old; old=old->next) {
      if (old->block == block && old->output == output) return old->node;
   }

   ExternalList *new = malloc_fast(sizeof(ExternalList));
   new->node = tr_nullary(BACKFLOW_DUMMY);
   new->block = block;
   new->output = output;
   new->next = *list;

   *list = new;

   return new->node;
}

static inline const Dataflow *relocate_dataflow(GLSL_TRANSLATE_CONTEXT_T *ctx, int dataflow)
{
   return &ctx->df_arr[dataflow];
}

static bool translate_per_quad(const Dataflow *d) {
   if (d->flavour == DATAFLOW_FDX || d->flavour == DATAFLOW_FDY)
      return true;
   else if (d->flavour == DATAFLOW_TEXTURE)
      return !glsl_dataflow_tex_cfg_implies_bslod(d->u.texture.bits);
   else
      return false;
}

static void mark_deps_as_per_quad(uint32_t d_id, GLSL_TRANSLATE_CONTEXT_T *ctx) {
   if (d_id == -1) return;

   const Dataflow *d = &ctx->df_arr[d_id];

   if (ctx->per_quad[d->id]) return;
   ctx->per_quad[d->id] = true;

   for (int i=0; i<d->dependencies_count; i++) {
      if (d->d.reloc_deps[i] != -1)
         mark_deps_as_per_quad(d->d.reloc_deps[i], ctx);
   }
}

static void resolve_per_quadness(Dataflow *d, void *data) {
   if (translate_per_quad(d)) {
      for (int i=0; i<d->dependencies_count; i++) {
         mark_deps_as_per_quad(d->d.reloc_deps[i], data);
      }
   }
}

static void tr_atomic(GLSL_TRANSLATION_T *r, GLSL_TRANSLATE_CONTEXT_T *ctx,
   const Dataflow *dataflow, GLSL_TRANSLATION_T *const d[4])
{
   struct tmu_lookup_s *lookup = tr_begin_tmu_access(ctx->block);
   lookup->is_modify = true;

   v3d_tmu_op_t op;
   /* Optimise addition/subtraction of +/- 1 down to inc/dec and xor with ~0 to bitnot */
   if ( (dataflow->flavour == DATAFLOW_ATOMIC_ADD || dataflow->flavour == DATAFLOW_ATOMIC_SUB) &&
        is_const(d[1]->node[0]) && (d[1]->node[0]->unif == 1 || d[1]->node[0]->unif == 0xffffffff))
   {
      bool is_dec = (dataflow->flavour == DATAFLOW_ATOMIC_ADD) ^ (d[1]->node[0]->unif == 1);
      op = is_dec ? V3D_TMU_OP_WR_OR_RD_DEC : V3D_TMU_OP_WR_AND_RD_INC;
   } else if (dataflow->flavour == DATAFLOW_ATOMIC_XOR && is_const(d[1]->node[0]) && d[1]->node[0]->unif == 0xffffffff) {
      op = V3D_TMU_OP_WR_XOR_RD_NOT;
   } else {
      tr_mid_tmu_access_data(lookup, d[1]);

      switch (dataflow->flavour)
      {
      case DATAFLOW_ATOMIC_ADD:     op = V3D_TMU_OP_WR_ADD_RD_PREFETCH; break;
      case DATAFLOW_ATOMIC_SUB:     op = V3D_TMU_OP_WR_SUB_RD_CLEAR; break;
      case DATAFLOW_ATOMIC_MIN:     op = dataflow->type == DF_UINT ? V3D_TMU_OP_WR_UMIN_RD_FULL_L1_CLEAR : V3D_TMU_OP_WR_SMIN; break;
      case DATAFLOW_ATOMIC_MAX:     op = dataflow->type == DF_UINT ? V3D_TMU_OP_WR_UMAX : V3D_TMU_OP_WR_SMAX; break;
      case DATAFLOW_ATOMIC_AND:     op = V3D_TMU_OP_WR_AND_RD_INC; break;
      case DATAFLOW_ATOMIC_OR:      op = V3D_TMU_OP_WR_OR_RD_DEC; break;
      case DATAFLOW_ATOMIC_XOR:     op = V3D_TMU_OP_WR_XOR_RD_NOT; break;
      case DATAFLOW_ATOMIC_XCHG:    op = V3D_TMU_OP_WR_XCHG_RD_FLUSH; break;
      case DATAFLOW_ATOMIC_CMPXCHG: op = V3D_TMU_OP_WR_CMPXCHG_RD_CLEAN; break;
      case DATAFLOW_ADDRESS_STORE:  op = V3D_TMU_OP_REGULAR; break;
      default:                      unreachable();
      }
   }

   int word_reads = (dataflow->flavour == DATAFLOW_ADDRESS_STORE) ? 0 : 1;

#if V3D_VER_AT_LEAST(4,0,2,0)
   if (d[0]->type == GLSL_TRANSLATION_VOID) {
      const Dataflow *addr = relocate_dataflow(ctx, dataflow->d.reloc_deps[0]);
      assert(addr->flavour == DATAFLOW_TEXTURE_ADDR);

      Backflow *c[4] = { NULL, };
      for (int i=0; i<4; i++) {
         if (addr->d.reloc_deps[i] == -1) continue;
         const Dataflow *df = relocate_dataflow(ctx, addr->d.reloc_deps[i]);
         assert(df->id < ctx->translations_count);
         GLSL_TRANSLATION_T *t = &ctx->translations[df->id];
         assert(t->type == GLSL_TRANSLATION_WORD);
         c[i] = t->node[0];
      }

      const Dataflow *sampler = relocate_dataflow(ctx, addr->d.reloc_deps[4]);
      uint32_t sampler_index = ctx->link_map->uniforms[sampler->u.const_sampler.location];

      tr_mid_tmu_access_texture(lookup,
         sampler_index, sampler->type, sampler->u.const_sampler.is_32bit, /*per_quad=*/false,
         word_reads, DF_TEXBITS_FETCH, op, d[2], c[0], c[1], c[2], c[3],
         /*dref=*/NULL, /*b=*/NULL, /*off=*/NULL);
   }
   else
#endif
   {
      assert(d[0]->type == GLSL_TRANSLATION_WORD);
      tr_mid_tmu_access_general(lookup, op, d[2], d[0]->node[0]);
   }

   r->type = word_reads ? get_translation_type(dataflow->type) : GLSL_TRANSLATION_VOID;
   tr_end_tmu_access(&r->node[0], lookup, word_reads, dataflow->age);
}

/* Main translation function */
static void translate_to_backend(Dataflow *dataflow, void *data)
{
   GLSL_TRANSLATE_CONTEXT_T *ctx = data;

   assert(dataflow->id < ctx->translations_count);
   GLSL_TRANSLATION_T *r = &ctx->translations[dataflow->id];
#if V3D_VER_AT_LEAST(4,0,2,0)
   assert(r->type == GLSL_TRANSLATION_UNVISITED);
#else
   if (r->type != GLSL_TRANSLATION_UNVISITED) return;
#endif

   DataflowFlavour flavour = dataflow->flavour;
   DataflowType    type    = dataflow->type;

   GLSL_TRANSLATION_T *d[DATAFLOW_MAX_DEPENDENCIES] = { NULL, };
   for (int i=0; i<dataflow->dependencies_count; i++) {
      if (dataflow->d.reloc_deps[i] == -1) continue;
      const Dataflow *df = relocate_dataflow(ctx, dataflow->d.reloc_deps[i]);
      assert(df->id < ctx->translations_count);
      d[i] = &ctx->translations[df->id];
      assert(d[i]->type != GLSL_TRANSLATION_UNVISITED);
   }

   dataflow_age = dataflow->age;
   switch (flavour) {
      case DATAFLOW_TEXTURE:
         {
            const Dataflow *sampler = relocate_dataflow(ctx, dataflow->d.reloc_deps[4]);
            uint32_t sampler_index = ctx->link_map->uniforms[sampler->u.const_sampler.location];
            uint32_t required_components = dataflow->u.texture.required_components;

            assert(d[0]->type == GLSL_TRANSLATION_VEC4);
            for (int i=1; i<4; i++) assert(d[i] == NULL || d[i]->type == GLSL_TRANSLATION_WORD);
            /* d[4] has type void because it contained the sampler */

#if !V3D_VER_AT_LEAST(3,3,0,0)
            tr_texture_gadget(ctx, r, sampler_index, sampler->type, sampler->u.const_sampler.is_32bit,
                              ctx->per_quad[dataflow->id],
                              dataflow->age, required_components, dataflow->u.texture.bits,
                              d[0]->node[0], d[0]->node[1], d[0]->node[2], d[0]->node[3],
                              d[1] ? d[1]->node[0] : NULL, d[2] ? d[2]->node[0] : NULL, d[3] ? d[3]->node[0] : NULL);
#else
            tr_texture_lookup(ctx, r, sampler_index, sampler->type, sampler->u.const_sampler.is_32bit,
                              ctx->per_quad[dataflow->id],
                              dataflow->age, required_components, dataflow->u.texture.bits,
                              d[0]->node[0], d[0]->node[1], d[0]->node[2], d[0]->node[3],
                              d[1] ? d[1]->node[0] : NULL, d[2] ? d[2]->node[0] : NULL, d[3] ? d[3]->node[0] : NULL);
#endif
            break;
         }
      case DATAFLOW_TEXTURE_SIZE:
         {
            const Dataflow *sampler = relocate_dataflow(ctx, dataflow->d.reloc_deps[0]);
            uint32_t sampler_index = ctx->link_map->uniforms[sampler->u.const_sampler.location];
            /* Maybe this should change to GLSL_TRANSLATION_VEC. Maybe store size for bounds checking */
            r->type = GLSL_TRANSLATION_VEC4;
            r->node[0] = tr_typed_uniform(BACKEND_UNIFORM_TEX_SIZE_X, sampler_index);
            r->node[1] = tr_typed_uniform(BACKEND_UNIFORM_TEX_SIZE_Y, sampler_index);
            /* The z-size may not exist. If not then the node won't be referenced, but this isn't pretty */
            r->node[2] = tr_typed_uniform(BACKEND_UNIFORM_TEX_SIZE_Z, sampler_index);
            r->node[3] = NULL;
            break;
         }
      case DATAFLOW_VEC4:
         r->type = GLSL_TRANSLATION_VEC4;
         for (int i=0; i<4; i++) r->node[i] = d[i] ? d[i]->node[0] : NULL;
         break;
      case DATAFLOW_FRAG_GET_COL:
         {
            uint32_t type = dataflow->type == DF_FLOAT ? GLXX_FB_F16 : GLXX_FB_I32;
            tr_get_col_gadget(ctx, r, type, dataflow->u.get_col.required_components,
                                            dataflow->u.get_col.render_target);
            break;
         }
      case DATAFLOW_ADDRESS_STORE:
      case DATAFLOW_ATOMIC_ADD:
      case DATAFLOW_ATOMIC_SUB:
      case DATAFLOW_ATOMIC_MIN:
      case DATAFLOW_ATOMIC_MAX:
      case DATAFLOW_ATOMIC_AND:
      case DATAFLOW_ATOMIC_OR:
      case DATAFLOW_ATOMIC_XOR:
      case DATAFLOW_ATOMIC_XCHG:
      case DATAFLOW_ATOMIC_CMPXCHG:
         tr_atomic(r, ctx, dataflow, d);
         break;
      case DATAFLOW_VECTOR_LOAD:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         tr_indexed_read_vector_gadget(ctx->block, r, dataflow->age, d[0]->node[0], dataflow->u.vector_load.required_components, ctx->per_quad[dataflow->id]);
         break;
      case DATAFLOW_GET_VEC4_COMPONENT:
         assert(d[0]->type == GLSL_TRANSLATION_VEC4);
         assert(dataflow->u.get_vec4_component.component_index < 4);
         r->type = get_translation_type(dataflow->type);
         r->node[0] = d[0]->node[dataflow->u.get_vec4_component.component_index];
         break;

      case DATAFLOW_CONST:
         r->type = get_translation_type(dataflow->type);
         r->node[0] = tr_const(dataflow->u.constant.value);
         break;

      case DATAFLOW_UNIFORM:
      {
         r->type = get_translation_type(dataflow->type);
         int row = dataflow->u.buffer.index;
         assert(row < ctx->link_map->num_uniforms);
         r->node[0] = tr_unif(ctx->link_map->uniforms[row] + dataflow->u.buffer.offset / 4);
         break;
      }
#if V3D_VER_AT_LEAST(4,0,2,0)
      case DATAFLOW_TEXTURE_ADDR:
#endif
      case DATAFLOW_CONST_SAMPLER:
      case DATAFLOW_UNIFORM_BUFFER:
      case DATAFLOW_STORAGE_BUFFER:
      case DATAFLOW_ATOMIC_COUNTER:
         r->type = GLSL_TRANSLATION_VOID;
         break;
      case DATAFLOW_ADDRESS:
         {
            const Dataflow *o = relocate_dataflow(ctx, dataflow->d.reloc_deps[0]);
            r->type = GLSL_TRANSLATION_WORD;

            int      id     = o->u.buffer.index;
            uint32_t offset = o->u.buffer.offset;
            if(o->flavour==DATAFLOW_UNIFORM) {
               assert(id >= 0 && id < ctx->link_map->num_uniforms);
               r->node[0] = tr_uniform_address(ctx->link_map->uniforms[id], offset);
            } else if(o->flavour==DATAFLOW_UNIFORM_BUFFER) {
               assert(id >= 0 && id < ctx->link_map->num_uniforms);
               r->node[0] = tr_block_address(BACKEND_UNIFORM_UBO_ADDRESS, ctx->link_map->uniforms[id], offset);
            } else if (o->flavour == DATAFLOW_STORAGE_BUFFER) {
               assert(id >= 0 && id < ctx->link_map->num_buffers);
               r->node[0] = tr_block_address(BACKEND_UNIFORM_SSBO_ADDRESS, ctx->link_map->buffers[id], offset);
            } else if (o->flavour == DATAFLOW_ATOMIC_COUNTER) {
               r->node[0] = tr_typed_uniform(BACKEND_UNIFORM_ATOMIC_ADDRESS, (id << 16) | offset);
            } else
               unreachable();
            break;
         }

      case DATAFLOW_BUF_SIZE:
      {
         const Dataflow *o = relocate_dataflow(ctx, dataflow->d.reloc_deps[0]);
         int id = o->u.buffer.index;
         assert(o->flavour == DATAFLOW_STORAGE_BUFFER);
         assert(id < ctx->link_map->num_buffers);
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = tr_typed_uniform(BACKEND_UNIFORM_SSBO_SIZE, ctx->link_map->buffers[id]);
         break;
      }

      case DATAFLOW_IN:
      {
         r->type = GLSL_TRANSLATION_WORD;
         int id = dataflow->u.linkable_value.row;
         assert(id < ctx->link_map->num_ins);
         r->node[0] = ctx->in->inputs[ctx->link_map->ins[id]];
         assert(r->node[0] != NULL);
         break;
      }
      case DATAFLOW_SHARED_PTR:
      case DATAFLOW_GET_DEPTHRANGE_NEAR:
      case DATAFLOW_GET_DEPTHRANGE_FAR:
      case DATAFLOW_GET_DEPTHRANGE_DIFF:
      case DATAFLOW_GET_NUMWORKGROUPS_X:
      case DATAFLOW_GET_NUMWORKGROUPS_Y:
      case DATAFLOW_GET_NUMWORKGROUPS_Z:
         {
            BackendSpecialUniformFlavour row;
            switch (flavour) {
               case DATAFLOW_SHARED_PTR:          row = BACKEND_SPECIAL_UNIFORM_SHARED_PTR;      break;
               case DATAFLOW_GET_DEPTHRANGE_NEAR: row = BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_NEAR; break;
               case DATAFLOW_GET_DEPTHRANGE_FAR:  row = BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_FAR;  break;
               case DATAFLOW_GET_DEPTHRANGE_DIFF: row = BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_DIFF; break;
               case DATAFLOW_GET_NUMWORKGROUPS_X: row = BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_X; break;
               case DATAFLOW_GET_NUMWORKGROUPS_Y: row = BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_Y; break;
               case DATAFLOW_GET_NUMWORKGROUPS_Z: row = BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_Z; break;
               default: unreachable(); row = 0; break;
            }
            r->type = GLSL_TRANSLATION_WORD;
            r->node[0] = tr_special_uniform(row);
         }
         break;
      case DATAFLOW_GET_POINT_COORD_X:
         r->type    = GLSL_TRANSLATION_WORD;
         r->node[0] = ctx->in->point_x;
         break;
      case DATAFLOW_GET_POINT_COORD_Y:
         r->type    = GLSL_TRANSLATION_WORD;
         r->node[0] = ctx->in->point_y;
         break;
      case DATAFLOW_GET_LINE_COORD:
         r->type    = GLSL_TRANSLATION_WORD;
         r->node[0] = ctx->in->line;
         break;
      case DATAFLOW_FRAG_GET_FF:
         r->type = GLSL_TRANSLATION_BOOL_FLAG;
         r->node[0] = tr_nullary(BACKFLOW_REVF);
         break;
      case DATAFLOW_IS_HELPER:
         r->type = GLSL_TRANSLATION_BOOL_FLAG;
         /* For BOOL_FLAG non-zero means false, so MSF is good enough */
         r->node[0] = tr_nullary(BACKFLOW_MSF);
         break;
      case DATAFLOW_SAMPLE_POS_X:
         r->type = GLSL_TRANSLATION_WORD;
         Backflow *fxcd = tr_nullary(BACKFLOW_FXCD);
         r->node[0] = tr_binop(BACKFLOW_SUB, fxcd, tr_uop(BACKFLOW_FLOOR, fxcd));
         break;
      case DATAFLOW_SAMPLE_POS_Y:
         r->type = GLSL_TRANSLATION_WORD;
         Backflow *fycd = tr_nullary(BACKFLOW_FYCD);
         r->node[0] = tr_binop(BACKFLOW_SUB, fycd, tr_uop(BACKFLOW_FLOOR, fycd));
         break;
      case DATAFLOW_NUM_SAMPLES:
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = tr_const(ctx->ms ? 4 : 1);
         break;
      case DATAFLOW_GET_VERTEX_ID:
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = ctx->in->vertexid;
         break;
      case DATAFLOW_GET_INSTANCE_ID:
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = ctx->in->instanceid;
         break;
#if V3D_VER_AT_LEAST(4,0,2,0)
      case DATAFLOW_GET_BASE_INSTANCE:
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = ctx->in->baseinstance;
         break;
#endif

      case DATAFLOW_FRAG_GET_Z:
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = ctx->in->z;
         break;
      case DATAFLOW_FRAG_GET_W:
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = tr_mov_to_reg(REG_MAGIC_RECIP, ctx->in->w);
         break;
      case DATAFLOW_CONDITIONAL:
         if (d[1]->type == GLSL_TRANSLATION_WORD) {
            assert(d[2]->type == GLSL_TRANSLATION_WORD);
            r->type = GLSL_TRANSLATION_WORD;
            r->node[0] = tr_cond(d[0]->node[0], d[1]->node[0], d[2]->node[0], d[0]->type);
         } else {
            tr_logical_cond(r, d[0], d[1], d[2]);
         }
         break;
      case DATAFLOW_DIV:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         assert(d[1]->type == GLSL_TRANSLATION_WORD);
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = tr_div(type, d[0]->node[0], d[1]->node[0]);
         break;
      case DATAFLOW_REM:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         assert(d[1]->type == GLSL_TRANSLATION_WORD);
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = tr_rem(type, d[0]->node[0], d[1]->node[0]);
         break;

      case DATAFLOW_LESS_THAN:
      case DATAFLOW_LESS_THAN_EQUAL:
      case DATAFLOW_GREATER_THAN:
      case DATAFLOW_GREATER_THAN_EQUAL:
      case DATAFLOW_EQUAL:
      case DATAFLOW_NOT_EQUAL:
         type = relocate_dataflow(ctx, dataflow->d.reloc_deps[0])->type;
         if (d[0]->type == GLSL_TRANSLATION_WORD) {
            assert(d[1]->type == GLSL_TRANSLATION_WORD);
            tr_comparison(r, flavour, type, d[0]->node[0], d[1]->node[0]);
         } else {
            tr_logical_binop(r, flavour, d[0], d[1]);
         }
         break;

      case DATAFLOW_LOGICAL_XOR:
         tr_logical_binop(r, flavour, d[0], d[1]);
         break;
      case DATAFLOW_LOGICAL_AND:
      case DATAFLOW_LOGICAL_OR:
         tr_logical_and_or(r, flavour, d[0], d[1]);
         break;
      case DATAFLOW_SQRT:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = tr_sqrt(d[0]->node[0]);
         break;
      case DATAFLOW_RSQRT:
      case DATAFLOW_RCP:
      case DATAFLOW_LOG2:
      case DATAFLOW_EXP2:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = tr_mov_to_reg(get_magic_reg(dataflow->flavour), d[0]->node[0]);
         break;
      case DATAFLOW_ABS:
      case DATAFLOW_FUNPACKA:
      case DATAFLOW_FUNPACKB:
         r->type = d[0]->type;
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         r->node[0] = tr_uop(BACKFLOW_FMOV, d[0]->node[0]);
         r->node[0]->u.alu.unpack[0] = flavour == DATAFLOW_ABS ? UNPACK_ABS : flavour == DATAFLOW_FUNPACKA ? UNPACK_F16_A : UNPACK_F16_B;
         break;

      case DATAFLOW_SIN:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         r->type = d[0]->type;
         r->node[0] = tr_sin(d[0]->node[0]);
         break;
      case DATAFLOW_COS:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         r->type = d[0]->type;
         r->node[0] = tr_cos(d[0]->node[0]);
         break;
      case DATAFLOW_TAN:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         r->type = d[0]->type;
         r->node[0] = tr_tan(d[0]->node[0]);
         break;
      case DATAFLOW_REINTERP:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         r->type = d[0]->type;
         r->node[0] = d[0]->node[0];
         break;
      case DATAFLOW_LOGICAL_NOT:
         tr_logical_not(r, d[0]);
         break;
      case DATAFLOW_PHI:
         r->type = d[0]->type;
         r->node[0] = d[0]->node[0];
         push_phi(dataflow, &ctx->block->phi_list);
         break;
      case DATAFLOW_EXTERNAL:
         r->type = (type == DF_BOOL) ? GLSL_TRANSLATION_BOOL_FLAG : GLSL_TRANSLATION_WORD;
         r->node[0] = tr_external(dataflow->u.external.block, dataflow->u.external.output, &ctx->block->external_list);
         break;
      case DATAFLOW_IMAGE_INFO_PARAM:
         {
            const Dataflow *sampler = relocate_dataflow(ctx, dataflow->d.reloc_deps[0]);
            uint32_t sampler_index = ctx->link_map->uniforms[sampler->u.const_sampler.location];
            r->type = GLSL_TRANSLATION_WORD;
            ImageInfoParam param = dataflow->u.image_info_param.param;
            switch(param)
            {
#if !V3D_VER_AT_LEAST(4,0,2,0)
            case IMAGE_INFO_ARR_STRIDE:
               r->node[0] = tr_typed_uniform(BACKEND_UNIFORM_IMAGE_ARR_STRIDE, sampler_index);
               break;
            case IMAGE_INFO_SWIZZLING:
               r->node[0] = tr_typed_uniform(BACKEND_UNIFORM_IMAGE_SWIZZLING, sampler_index);
               break;
            case IMAGE_INFO_LX_ADDR:
               r->node[0] = tr_typed_uniform(BACKEND_UNIFORM_IMAGE_LX_ADDR, sampler_index);
               break;
            case IMAGE_INFO_LX_PITCH:
               r->node[0] = tr_typed_uniform(BACKEND_UNIFORM_IMAGE_LX_PITCH, sampler_index);
               break;
            case IMAGE_INFO_LX_SLICE_PITCH:
               r->node[0] = tr_typed_uniform(BACKEND_UNIFORM_IMAGE_LX_SLICE_PITCH, sampler_index);
               break;
#endif
            case IMAGE_INFO_LX_WIDTH:
               r->node[0] = tr_typed_uniform(BACKEND_UNIFORM_IMAGE_LX_WIDTH, sampler_index);
               break;
            case IMAGE_INFO_LX_HEIGHT:
               r->node[0] = tr_typed_uniform(BACKEND_UNIFORM_IMAGE_LX_HEIGHT, sampler_index);
               break;
            case IMAGE_INFO_LX_DEPTH:
               r->node[0] = tr_typed_uniform(BACKEND_UNIFORM_IMAGE_LX_DEPTH, sampler_index);
               break;
            default:
               unreachable();
               break;
            }
         }
         break;
      case DATAFLOW_TEXBUFFER_INFO_PARAM:
         {
            const Dataflow *sampler = relocate_dataflow(ctx, dataflow->d.reloc_deps[0]);
            uint32_t sampler_index = ctx->link_map->uniforms[sampler->u.const_sampler.location];
            r->type = GLSL_TRANSLATION_WORD;
            TexBufferInfoParam param = dataflow->u.texbuffer_info_param.param;
            switch (param)
            {
            case TEXBUFFER_INFO_LOG2_ARR_ELEM_W:
               r->node[0] = tr_typed_uniform(BACKEND_UNIFORM_TEXBUFFER_LOG2_ARR_ELEM_W, sampler_index);
               break;
            case TEXBUFFER_INFO_ARR_ELEM_W_MINUS_1:
               r->node[0] = tr_typed_uniform(BACKEND_UNIFORM_TEXBUFFER_ARR_ELEM_W_MINUS_1 , sampler_index);
               break;
            default:
               unreachable();
               break;
            }
         }
         break;
      case DATAFLOW_SAMPLE_MASK:
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = tr_nullary(BACKFLOW_MSF);
         if (!ctx->ms)
            r->node[0] = bitand(r->node[0], tr_const(1));
         break;

      default:       /* Default case: Flavour should map directly to an instruction */
      {
         int params;
         BackflowFlavour f = translate_flavour(flavour, type, &params);
         for (int i=0; i<params; i++) assert(d[i]->type == GLSL_TRANSLATION_WORD);

         r->type = GLSL_TRANSLATION_WORD;
         if      (params == 0) r->node[0] = tr_nullary(f);
         else if (params == 1) r->node[0] = tr_uop    (f, d[0]->node[0]);
         else                  r->node[0] = tr_binop  (f, d[0]->node[0], d[1]->node[0]);

#if !V3D_VER_AT_LEAST(4,0,2,0)
         if (flavour == DATAFLOW_FRAG_GET_X || flavour == DATAFLOW_FRAG_GET_Y) {
            /* In older hardware this value matches the integer coordinate so
             * add 0.5 to get the pixel centre. */
            r->node[0] = tr_binop(BACKFLOW_ADD,
                                  tr_const(gfx_float_to_bits(0.5)),
                                  r->node[0]);
         }
#endif
         break;
      }
   }

   bool df_per_sample = (flavour == DATAFLOW_FRAG_GET_COL || flavour == DATAFLOW_SAMPLE_ID ||
                         flavour == DATAFLOW_SAMPLE_POS_X || flavour == DATAFLOW_SAMPLE_POS_Y);
#if !V3D_VER_AT_LEAST(4,0,2,0)
   r->per_sample = df_per_sample;
   for (int i = 0; i < dataflow->dependencies_count; i++) {
      if (dataflow->d.reloc_deps[i] != -1) {
         const Dataflow *dependency = relocate_dataflow(ctx, dataflow->d.reloc_deps[i]);
         GLSL_TRANSLATION_T* d = &ctx->translations[dependency->id];
         r->per_sample |= d->per_sample;
      }
   }
   if (r->per_sample) {
      GLSL_TRANSLATION_LIST_T *node = malloc_fast(sizeof(GLSL_TRANSLATION_LIST_T));
      node->value = r;
      node->next = ctx->per_sample_clear_list;
      ctx->per_sample_clear_list = node;
   }
#else
   ctx->block->per_sample = ctx->block->per_sample || df_per_sample;
#endif
}

void init_sched_block(SchedBlock *block) {
   memset(block, 0, sizeof(SchedBlock));
   glsl_backflow_chain_init(&block->iodeps);
}

static void init_translation_context(GLSL_TRANSLATE_CONTEXT_T *ctx,
                                     const Dataflow *arr,
                                     int dataflow_count,
                                     const LinkMap *link_map,
                                     SchedShaderInputs *ins,
                                     SchedBlock *block,
                                     const GLXX_LINK_RESULT_KEY_T *key)
{
   ctx->df_arr = arr;

   ctx->translations = malloc_fast(sizeof(GLSL_TRANSLATION_T) * dataflow_count);
   ctx->translations_count = dataflow_count;
   memset(ctx->translations, 0, sizeof(GLSL_TRANSLATION_T) * dataflow_count);

   memset(ctx->tlb_read, 0, sizeof(struct tlb_read_s) * V3D_MAX_RENDER_TARGETS);

   ctx->per_quad = malloc_fast(sizeof(bool) * dataflow_count);
   for (int i=0; i<dataflow_count; i++) ctx->per_quad[i] = false;

   ctx->link_map = link_map;

   ctx->in = ins;
   ctx->block = block;

#if !V3D_VER_AT_LEAST(3,3,0,0)
   for (int i = 0; i < GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS; i++)
      ctx->gadgettype[i] = key->gadgettype[i];
   for (int i = 0; i < GLXX_CONFIG_MAX_IMAGE_UNITS; i++)
      ctx->img_gadgettype[i] = key->img_gadgettype[i];
#endif

   ctx->ms = (key->backend & GLXX_SAMPLE_MS);
#if !V3D_VER_AT_LEAST(4,0,2,0)
   /* Stuff used to do framebuffer fetch on multisample targets */
   ctx->sample_num = 0;
   ctx->per_sample_clear_list = NULL;
#endif
}

void fragment_shader_inputs(SchedShaderInputs *ins, uint32_t primitive_type, const VARYING_INFO_T *varying) {
   /* NULL out the whole structure to kill things we don't use */
   memset(ins, 0, sizeof(SchedShaderInputs));

   dataflow_age = 0;

   /* The QPU gets these and can't fetch them again, so create one copy */
   ins->w   = tr_nullary(BACKFLOW_DUMMY);
   ins->w->remaining_dependents = 0;
   ins->w_c = tr_nullary(BACKFLOW_DUMMY);
   ins->w_c->remaining_dependents = 0;
   ins->z   = tr_nullary(BACKFLOW_DUMMY);
   ins->z->remaining_dependents = 0;

   ins->read_dep = init_frag_vary(ins, primitive_type, varying);
}

static void translate_node_array(GLSL_TRANSLATE_CONTEXT_T *ctx, const CFGBlock *b_in,
                                 const bool *output_active, Backflow **nodes_out)
{
   DataflowRelocVisitor pass;
   bool *temp = glsl_safemem_malloc(sizeof(bool) * b_in->num_dataflow);
   glsl_dataflow_reloc_visitor_begin(&pass, b_in->dataflow, b_in->num_dataflow, temp);

   for (int i=0; i<b_in->num_outputs; i++) {
      if (!output_active[i]) continue;

      glsl_dataflow_reloc_post_visitor(&pass, b_in->outputs[i], ctx, translate_to_backend);

      const Dataflow *df = relocate_dataflow(ctx, b_in->outputs[i]);
      GLSL_TRANSLATION_T *tr = &ctx->translations[df->id];
      if (tr->type == GLSL_TRANSLATION_BOOL_FLAG_N) {
         /* Convert the rep to FLAG for output from the block */
         if (is_const(tr->node[0]))
            nodes_out[i] = tr->node[0]->unif ? tr_const(0) : tr_const(1);
         else
            nodes_out[i] = create_node(BACKFLOW_FL, UNPACK_NONE, SETF_NONE, tr->node[0], NULL, NULL, NULL);
      } else
         nodes_out[i] = tr->node[0];
   }

   glsl_dataflow_reloc_visitor_end(&pass);
   glsl_safemem_free(temp);
}

SchedBlock *translate_block(const CFGBlock *b_in, const LinkMap *link_map,
                            const bool *output_active, SchedShaderInputs *ins,
                            const GLXX_LINK_RESULT_KEY_T *key)
{
   SchedBlock *ret = malloc_fast(sizeof(SchedBlock));
   init_sched_block(ret);

   GLSL_TRANSLATE_CONTEXT_T ctx;
   init_translation_context(&ctx, b_in->dataflow, b_in->num_dataflow, link_map, ins, ret, key);

   DataflowRelocVisitor pass;
   bool *temp = glsl_safemem_malloc(sizeof(bool) * b_in->num_dataflow);
   glsl_dataflow_reloc_visitor_begin(&pass, b_in->dataflow, b_in->num_dataflow, temp);
   for (int i=0; i<b_in->num_outputs; i++) glsl_dataflow_reloc_post_visitor(&pass, b_in->outputs[i], &ctx, resolve_per_quadness);
   glsl_dataflow_reloc_visitor_end(&pass);
   glsl_safemem_free(temp);

   ret->branch_cond = b_in->successor_condition;
   ret->num_outputs = b_in->num_outputs;

#if !V3D_VER_AT_LEAST(4,0,2,0)
   int max_samples = (key->backend & GLXX_SAMPLE_MS) ? 4 : 1;
   Backflow **nodes_out[4] = { NULL, };
   for (int i=0; i<max_samples; i++) nodes_out[i] = glsl_safemem_calloc(b_in->num_outputs, sizeof(Backflow *));
   /* Translate the dataflow. If we have a get_col then clear the current
    * translation and retranslate everything that depends on the read for each
    * sample. This can only happen during fragment shaders.                    */
   do {
      GLSL_TRANSLATION_LIST_T *clear_node;
      // Clear translations which need to be done multiple times during multisampling
      for (clear_node = ctx.per_sample_clear_list; clear_node; clear_node = clear_node->next) {
         memset(clear_node->value, 0, sizeof(GLSL_TRANSLATION_T));
      }
      ctx.per_sample_clear_list = NULL;

      translate_node_array(&ctx, b_in, output_active, nodes_out[ctx.sample_num]);

      ctx.sample_num++;
   } while (ctx.per_sample_clear_list && ctx.sample_num < max_samples);

   if (ctx.per_sample_clear_list != NULL && max_samples > 1) ret->per_sample = true;

   int samples_out = ret->per_sample ? max_samples : 1;
   ret->outputs = malloc_fast(samples_out * b_in->num_outputs * sizeof(Backflow *));
   for (int i=0; i<ret->num_outputs; i++) {
      for (int j=0; j<samples_out; j++) {
         ret->outputs[samples_out*i + j] = nodes_out[j][i];
      }
   }

   for (int i=0; i<max_samples; i++) glsl_safemem_free(nodes_out[i]);

   validate_tlb_loads(ctx.tlb_read, max_samples > 1);
#else
   ret->outputs = malloc_fast(b_in->num_outputs * sizeof(Backflow *));
   memset(ret->outputs, 0, b_in->num_outputs * sizeof(Backflow *));
   translate_node_array(&ctx, b_in, output_active, ret->outputs);
#endif

   resolve_tlb_loads(ret, ctx.tlb_read);

   if (ins != NULL && ins->read_dep != NULL)
      glsl_backflow_chain_append(&ret->iodeps, ins->read_dep);

   return ret;
}

static uint32_t get_reads_total(const ATTRIBS_USED_T *attr) {
   uint32_t reads_total = 0;

   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++) {
      assert(attr->scalars_used[i] <= 4);
      reads_total += attr->scalars_used[i];
   }
   if (attr->vertexid_used) reads_total++;
   if (attr->instanceid_used) reads_total++;

   return reads_total;
}

unsigned vertex_shader_inputs(SchedShaderInputs *ins, const ATTRIBS_USED_T *attr_info) {
   /* Ensure all unused inputs are NULL */
   memset(ins, 0, sizeof(SchedShaderInputs));

   unsigned reads_total = get_reads_total(attr_info);
   fetch_all_attribs(ins, attr_info, reads_total);
   return reads_total;
}
