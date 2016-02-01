/*=============================================================================
Copyright (c) 2012 Broadcom Europe Limited.
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
#include "glsl_tex_params.h"

#include "interface/khronos/glxx/glxx_int_config.h"
#include "middleware/khronos/glxx/glxx_shader_cache.h"

#if defined (_WIN32)
#define _USE_MATH_DEFINES
#endif

#include <assert.h>
#include <math.h>

static uint32_t dataflow_age = 0; /* Store this as static to avoid having to thread through Backflow constructors */

//
// Backflow chain functions (TODO: Possibly need a better home)
//
void glsl_backflow_chain_init(BackflowChain* chain)
{
   glsl_list_BackflowChainNode_init(chain);
}

void glsl_backflow_chain_append(BackflowChain* chain, Backflow* backflow)
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
   UNREACHABLE();
}

/* Priority queue functions. TODO: Possibly need a better home */
void glsl_backflow_priority_queue_init(BackflowPriorityQueue* queue, int size)
{
   queue->size = size;
   queue->used = 0;
   queue->nodes = malloc_fast(size * sizeof(Backflow *));
}

static inline bool compare(const Backflow *d0, const Backflow *d1)
{
   return d1->delay > d0->delay;
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
      consumer->any_io_dependents = true;
      glsl_backflow_chain_append(&consumer->io_dependencies, supplier);
      tmu_dep_merge(&consumer->tmu_deps, supplier->tmu_deps);
   }
}

struct translate_s {
   BackflowFlavour flavour;
   ALU_TYPE_T type;
   uint32_t op;
   uint32_t op1;
   uint32_t op2;
   uint32_t magic_write;
   uint32_t sigbits;
};

static const struct translate_s translations[] = {
   { BACKFLOW_ADD,               ALU_A_SWAP0, 0|1<<2|1, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_PACK_FLOAT16,      ALU_A, 53, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_IADD,              ALU_A, 56, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_ISUB,              ALU_A, 60, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_SUB,               ALU_A, 64|1<<2|1, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_IMIN,              ALU_A, 120, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_IMAX,              ALU_A, 121, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_UMIN,              ALU_A, 122, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_UMAX,              ALU_A, 123, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_SHL,               ALU_A, 124, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_SHR,               ALU_A, 125, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_ASHR,              ALU_A, 126, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_MIN,               ALU_A_SWAP0, 128|1<<2|1, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_MAX,               ALU_A_SWAP1, 128|1<<2|1, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_BITWISE_AND,       ALU_A, 181, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_BITWISE_OR,        ALU_A, 182, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_BITWISE_XOR,       ALU_A, 183, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_BITWISE_NOT,       ALU_A, 186, 0, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_IARITH_NEGATE,     ALU_A, 186, 1, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_FRAG_SUBMIT_MSF,   ALU_A, 186, 6, ~0u, REG_MAGIC_NOP, 0 },
   { BACKFLOW_TIDX,              ALU_A, 187, 0,  1,  REG_UNDECIDED, 0 },
   { BACKFLOW_FXCD,              ALU_A, 187, 1,  0,  REG_UNDECIDED, 0 },
   { BACKFLOW_XCD,               ALU_A, 187, 1,  3,  REG_UNDECIDED, 0 },
   { BACKFLOW_FYCD,              ALU_A, 187, 1,  4,  REG_UNDECIDED, 0 },
   { BACKFLOW_YCD,               ALU_A, 187, 1,  7,  REG_UNDECIDED, 0 },
   { BACKFLOW_MSF,               ALU_A, 187, 2,  0,  REG_UNDECIDED, 0 },
   { BACKFLOW_REVF,              ALU_A, 187, 2,  1,  REG_UNDECIDED, 0 },
   { BACKFLOW_TMUWT,             ALU_A, 187, 2,  5,  REG_MAGIC_NOP, 0 },
   { BACKFLOW_VPM_SETUP,         ALU_A, 187, 3, ~0u, REG_MAGIC_NOP, 0 },
   { BACKFLOW_ARITH_NEGATE,      ALU_A, 191, 0, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_FCMP,              ALU_A, 192|1<<2|1, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_ROUND,             ALU_A, 245, 0, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_FTOI_NEAREST,      ALU_A, 245, 3, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_TRUNC,             ALU_A, 245, 4, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_FTOI_TRUNC,        ALU_A, 245, 7, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_FLOOR,             ALU_A, 246, 0, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_FTOUZ,             ALU_A, 246, 3, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_CEIL,              ALU_A, 246, 4, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_FTOC,              ALU_A, 246, 7, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_FDX,               ALU_A, 247, 0, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_FDY,               ALU_A, 247, 4, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_ITOF,              ALU_A, 252, 0, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_CLZ,               ALU_A, 252, 3, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_UTOF,              ALU_A, 252, 4, ~0u, REG_UNDECIDED, 0 },

   { BACKFLOW_UMUL,              ALU_M, 3, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_SMUL,              ALU_M, 9, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_MULTOP,            ALU_M, 10, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_MUL,               ALU_M, 16|1<<2|1, ~0u, ~0u, REG_UNDECIDED, 0 },

   { BACKFLOW_MOV,               ALU_MOV, ~0u, ~0u, ~0u, REG_UNDECIDED,   0 },

   { BACKFLOW_ABS,               ALU_FMOV, ~0u, 0, 0, REG_UNDECIDED, 0 },
   { BACKFLOW_FMOV,              ALU_FMOV, ~0u, 0, 1, REG_UNDECIDED, 0 },
   { BACKFLOW_UNPACK_16A_F,      ALU_FMOV, ~0u, 0, 2, REG_UNDECIDED, 0 },
   { BACKFLOW_UNPACK_16B_F,      ALU_FMOV, ~0u, 0, 3, REG_UNDECIDED, 0 },

   { BACKFLOW_UNIFORM,           SIG, ~0u, ~0u, ~0u, REG_UNDECIDED, SIGBIT_LDUNIF },
   { BACKFLOW_TEX_GET,           SIG, ~0u, ~0u, ~0u, REG_UNDECIDED, SIGBIT_LDTMU  },
   { BACKFLOW_FRAG_GET_TLB,      SIG, ~0u, ~0u, ~0u, REG_UNDECIDED, SIGBIT_LDTLB  },
   { BACKFLOW_FRAG_GET_TLBU,     SIG, ~0u, ~0u, ~0u, REG_UNDECIDED, SIGBIT_LDTLBU },
   { BACKFLOW_ATTRIBUTE,         SIG, ~0u, ~0u, ~0u, REG_UNDECIDED, SIGBIT_LDVPM  },
   { BACKFLOW_WRTMUC,            SIG, ~0u, ~0u, ~0u, REG_UNDECIDED, SIGBIT_WRTMUC },

   /* TODO: Awful abuses of the 'op' field: */
   { BACKFLOW_VARYING,            SPECIAL_VARYING, VARYING_DEFAULT,    ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_VARYING_LINE_COORD, SPECIAL_VARYING, VARYING_LINE_COORD, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_VARYING_FLAT,       SPECIAL_VARYING, VARYING_FLAT,       ~0u, ~0u, REG_UNDECIDED, 0 },

   { BACKFLOW_IMUL32,       SPECIAL_IMUL32, ~0u, ~0u, ~0u, REG_UNDECIDED, 0 },
   { BACKFLOW_THREADSWITCH, SPECIAL_THRSW,  ~0u, ~0u, ~0u, REG_MAGIC_NOP, 0 },
   { BACKFLOW_DUMMY,        SPECIAL_VOID,   ~0u, ~0u, ~0u, REG_MAGIC_NOP, 0 },

   { BACKFLOW_LDVPM,    ALU_A, 188,  0,  ~0u, REG_UNDECIDED, 0  },
   { BACKFLOW_STVPM,    ALU_A, 248, ~0u, ~0u, REG_MAGIC_NOP, 0  },
};

static Backflow *create_node(BackflowFlavour flavour, uint32_t cond_setf, Backflow *flag,
                             Backflow *left, Backflow *right, Backflow *output)
{
   Backflow *backend = malloc_fast(sizeof(Backflow));
   memset(backend, 0, sizeof(Backflow));
   backend->pass = 0;

   glsl_backflow_chain_init(&backend->data_dependents);
   glsl_backflow_chain_init(&backend->io_dependencies);

   backend->age = dataflow_age;

   backend->unif_type = BACKEND_UNIFORM_UNASSIGNED;

   assert(flavour < BACKFLOW_FLAVOUR_COUNT);
   assert(translations[flavour].flavour == flavour);
   backend->type = translations[flavour].type;
   backend->op  = translations[flavour].op;
   backend->op1 = translations[flavour].op1;
   backend->op2 = translations[flavour].op2;
   backend->magic_write = translations[flavour].magic_write;
   backend->sigbits = translations[flavour].sigbits;

   backend->cond_setf = cond_setf;

   /* Verify that the flag field is used correctly */
   assert(flag == NULL || cond_setf == COND_IFFLAG || cond_setf == COND_IFNFLAG || cs_is_updt(cond_setf));
   assert(output == NULL || cond_setf == COND_IFFLAG || cond_setf == COND_IFNFLAG);
   /* Could further validate that left and right are used consistently with the operation */

   dep(backend, 0, flag);
   dep(backend, 1, left);
   dep(backend, 2, right);
   dep(backend, 3, output);
   return backend;
}

/* Low-level constructors */
static Backflow *tr_nullary(BackflowFlavour flavour)
{
   return create_node(flavour, SETF_NONE, NULL, NULL, NULL, NULL);
}

static Backflow *tr_unary_op(BackflowFlavour flavour, Backflow *operand)
{
   return create_node(flavour, SETF_NONE, NULL, operand, NULL, NULL);
}

static Backflow *tr_mov_to_reg(uint32_t reg, Backflow *operand) {
   Backflow *ret = tr_unary_op(BACKFLOW_MOV, operand);
   assert(ret->magic_write == REG_UNDECIDED);
   ret->magic_write = reg;
   return ret;
}

static Backflow *tr_conditional_uop(BackflowFlavour flavour, uint32_t cond_setf,
                                    Backflow *flag, Backflow *operand)
{
   return create_node(flavour, cond_setf, flag, operand, NULL, NULL);
}

static Backflow *tr_conditional_binop(BackflowFlavour flavour, uint32_t cond_setf,
                                      Backflow *left, Backflow *right)
{
   return create_node(flavour, cond_setf, NULL, left, right, NULL);
}

static Backflow *tr_binop(BackflowFlavour flavour, Backflow *left, Backflow *right)
{
   return tr_conditional_binop(flavour, SETF_NONE, left, right);
}

static Backflow *tr_nullary_io(BackflowFlavour flavour, Backflow *prev)
{
   Backflow *result = tr_nullary(flavour);
   glsl_iodep(result, prev);
   return result;
}

static Backflow *tr_uop_io(BackflowFlavour flavour, Backflow *param, Backflow *prev)
{
   Backflow *result = tr_unary_op(flavour, param);
   glsl_iodep(result, prev);
   return result;
}

static Backflow *tr_mov_to_reg_io(uint32_t reg, Backflow *param, Backflow *iodep)
{
   Backflow *result = tr_mov_to_reg(reg, param);
   glsl_iodep(result, iodep);
   return result;
}

static Backflow *tr_cond(Backflow *cond, Backflow *true_value, Backflow *false_value, GLSL_TRANSLATION_TYPE_T rep)
{
   uint32_t cond_setf = (rep == GLSL_TRANSLATION_BOOL_FLAG) ? COND_IFFLAG : COND_IFNFLAG;
   assert(rep == GLSL_TRANSLATION_BOOL_FLAG || rep == GLSL_TRANSLATION_BOOL_FLAG_N);

   return create_node(BACKFLOW_MOV, cond_setf, cond, true_value, NULL, false_value);
}

static Backflow *tr_typed_uniform(BackendUniformFlavour flavour, uint32_t value) {
   Backflow *result  = tr_nullary(BACKFLOW_UNIFORM);
   assert(flavour >= 0 && flavour <= BACKEND_UNIFORM_LAST_ELEMENT);
   result->unif_type = flavour;
   result->unif      = value;
   return result;
}

/* Convenience Constructors */

static Backflow *tr_discard(Backflow *prev, Backflow *cond) {
   Backflow *zero   = tr_typed_uniform(BACKEND_UNIFORM_LITERAL, 0);
   Backflow *result = create_node(BACKFLOW_FRAG_SUBMIT_MSF, COND_IFFLAG, cond, zero, NULL, NULL);

   glsl_iodep(result, prev);

   return result;
}

static Backflow *tr_special_uniform(BackendSpecialUniformFlavour flavour) {
   assert(flavour >= 0 || flavour <= BACKEND_SPECIAL_UNIFORM_LAST_ELEMENT);
   return tr_typed_uniform(BACKEND_UNIFORM_SPECIAL, flavour);
}

static Backflow *tr_unif(uint32_t i)
{
   return tr_typed_uniform(BACKEND_UNIFORM_PLAIN, i);
}

static Backflow *tr_uniform_address(uint32_t row)
{
   return tr_typed_uniform(BACKEND_UNIFORM_ADDRESS, row);
}

static Backflow *tr_uniform_block_address(uint32_t index)
{
   return tr_typed_uniform(BACKEND_UNIFORM_BLOCK_ADDRESS, index);
}

static Backflow *tr_const(uint32_t c) {
   return tr_typed_uniform(BACKEND_UNIFORM_LITERAL, c);
}

static Backflow *tr_cfloat(float f) { return tr_const(float_to_bits(f)); }

/* Define these static inline, so that unused functions for versioned
 * builds won't cause compiler warnings.                             */
static inline Backflow *mul(Backflow *a, Backflow *b)
{
   return tr_binop(BACKFLOW_MUL, a, b);
}
static inline Backflow *add(Backflow *a, Backflow *b)
{
   return tr_binop(BACKFLOW_ADD, a, b);
}
static inline Backflow *bitand(Backflow *a, Backflow *b)
{
   return tr_binop(BACKFLOW_BITWISE_AND, a, b);
}
static inline Backflow *bitor(Backflow *a, Backflow *b)
{
   return tr_binop(BACKFLOW_BITWISE_OR, a, b);
}
static inline Backflow *shl(Backflow *a, uint32_t b)
{
   return tr_binop(BACKFLOW_SHL, a, tr_const(b));
}
static inline Backflow *shr(Backflow *a, uint32_t b)
{
   return tr_binop(BACKFLOW_SHR, a, tr_const(b));
}
static inline Backflow *asr(Backflow *a, uint32_t b)
{
   return tr_binop(BACKFLOW_ASHR, a, tr_const(b));
}
/* Used by glsl_scheduler_4.c */
Backflow *glsl_backflow_thrsw(void) {
   return tr_nullary(BACKFLOW_THREADSWITCH);
}
Backflow *glsl_backflow_tmuwt(void) {
   return tr_nullary(BACKFLOW_TMUWT);
}
Backflow *glsl_backflow_dummy(void) {
   return tr_nullary(BACKFLOW_DUMMY);
}

static Backflow *tr_sin(Backflow *angle)
{
   Backflow *x = tr_binop(BACKFLOW_MUL, angle, tr_cfloat((float)M_1_PI));
   Backflow *y = tr_unary_op(BACKFLOW_ROUND, x);
   Backflow *sfu_sin = tr_mov_to_reg(REG_MAGIC_SIN, tr_binop(BACKFLOW_SUB, x, y));
   Backflow *i = tr_unary_op(BACKFLOW_FTOI_NEAREST, y);
   Backflow *il31 = tr_binop(BACKFLOW_SHL, i, tr_const(31));

   return tr_binop(BACKFLOW_BITWISE_XOR, sfu_sin, il31);
}

static Backflow *tr_cos(Backflow *angle)
{
   return tr_sin( tr_binop(BACKFLOW_ADD, angle, tr_cfloat((float)M_PI / 2.0) ) );
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
      Rs = tr_binop(BACKFLOW_BITWISE_OR, Rs, c1);

      cond1 = tr_conditional_binop(BACKFLOW_BITWISE_XOR, SETF_PUSHZ, c2, CONST1);
      ncond = create_node(BACKFLOW_ISUB, SETF_NORNC, cond1, Rs, Rd, NULL);

      Rs = tr_cond(ncond, tr_binop(BACKFLOW_ISUB, Rs, Rd), Rs, GLSL_TRANSLATION_BOOL_FLAG_N);
      Rq = tr_cond(ncond, tr_binop(BACKFLOW_BITWISE_OR, Rq, CONST1), Rq, GLSL_TRANSLATION_BOOL_FLAG_N);
   }

   *div = Rq;
   *rem = Rs;
}

static void signed_div_rem(Backflow **div, Backflow **rem, Backflow *l, Backflow *r) {
   Backflow *cond_left  = tr_conditional_uop(BACKFLOW_MOV, SETF_PUSHN, NULL, l);
   Backflow *cond_right = tr_conditional_uop(BACKFLOW_MOV, SETF_PUSHN, NULL, r);
   Backflow *uleft  = tr_cond(cond_left,  tr_unary_op(BACKFLOW_IARITH_NEGATE, l), l, GLSL_TRANSLATION_BOOL_FLAG);
   Backflow *uright = tr_cond(cond_right, tr_unary_op(BACKFLOW_IARITH_NEGATE, r), r, GLSL_TRANSLATION_BOOL_FLAG);

   Backflow *udiv, *urem;
   unsigned_div_rem(&udiv, &urem, uleft, uright);

   udiv = tr_cond(cond_left,  tr_unary_op(BACKFLOW_IARITH_NEGATE, udiv), udiv, GLSL_TRANSLATION_BOOL_FLAG);
   *div = tr_cond(cond_right, tr_unary_op(BACKFLOW_IARITH_NEGATE, udiv), udiv, GLSL_TRANSLATION_BOOL_FLAG);
   *rem = tr_cond(cond_left,  tr_unary_op(BACKFLOW_IARITH_NEGATE, urem), urem, GLSL_TRANSLATION_BOOL_FLAG);
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
         UNREACHABLE();
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
         UNREACHABLE();
         return NULL;
   }
}

static inline bool is_const(const Backflow *b) {
   return (b->type == SIG && b->sigbits == SIGBIT_LDUNIF &&
           b->unif_type == BACKEND_UNIFORM_LITERAL       );
}

static inline bool is_const_zero(const Backflow *b) {
   return (is_const(b) && b->unif == 0);
}

/* Mid-level constructors */

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
      { BACKFLOW_ISUB, SETF_PUSHC, false,  false },             /* <  */
      { BACKFLOW_ISUB, SETF_PUSHC, false,  true  },             /* >  */
      { BACKFLOW_ISUB, SETF_PUSHC, true,   true  },             /* <= */
      { BACKFLOW_ISUB, SETF_PUSHC, true,   false },             /* >= */
      { BACKFLOW_BITWISE_XOR,  SETF_PUSHZ, false,  false },     /* == */
      { BACKFLOW_BITWISE_XOR,  SETF_PUSHZ, true,   false },     /* != */
   }, {           /* int */
      { BACKFLOW_IMIN, SETF_PUSHC, false,  true  },             /* <  */
      { BACKFLOW_IMIN, SETF_PUSHC, false,  false },             /* >  */
      { BACKFLOW_IMIN, SETF_PUSHC, true,   false },             /* <= */
      { BACKFLOW_IMIN, SETF_PUSHC, true,   true  },             /* >= */
      { BACKFLOW_BITWISE_XOR,  SETF_PUSHZ, false,  false },     /* == */
      { BACKFLOW_BITWISE_XOR,  SETF_PUSHZ, true,   false },     /* != */
   }};
   const struct cmp_s *op;
   int type_idx, flavour_idx;

   switch (type) {
      case DF_FLOAT: type_idx = 0;  break;
      case DF_UINT:  type_idx = 1;  break;
      case DF_INT:   type_idx = 2; break;
      default: UNREACHABLE();
   }
   switch (flavour) {
      case DATAFLOW_LESS_THAN:          flavour_idx = 0; break;
      case DATAFLOW_GREATER_THAN:       flavour_idx = 1; break;
      case DATAFLOW_LESS_THAN_EQUAL:    flavour_idx = 2; break;
      case DATAFLOW_GREATER_THAN_EQUAL: flavour_idx = 3; break;
      case DATAFLOW_EQUAL:              flavour_idx = 4; break;
      case DATAFLOW_NOT_EQUAL:          flavour_idx = 5; break;
      default: UNREACHABLE();
   }

   op = &ops[type_idx][flavour_idx];
   Backflow *op_l = op->reverse ? right : left;
   Backflow *op_r = op->reverse ? left  : right;
   result->type = op->negate ? GLSL_TRANSLATION_BOOL_FLAG_N : GLSL_TRANSLATION_BOOL_FLAG;
   if (is_const_zero(op_r) && type == DF_FLOAT) {
      result->node[0] = tr_conditional_uop(BACKFLOW_FMOV, op->cond_setf, NULL, op_l);
   } else {
      result->node[0] = tr_conditional_binop(op->flavour, op->cond_setf, op_l, op_r);
   }
}

static void tr_logical_not(GLSL_TRANSLATION_T *result, GLSL_TRANSLATION_T *param)
{
   switch(param->type)
   {
   case GLSL_TRANSLATION_BOOL_FLAG:   result->type = GLSL_TRANSLATION_BOOL_FLAG_N; break;
   case GLSL_TRANSLATION_BOOL_FLAG_N: result->type = GLSL_TRANSLATION_BOOL_FLAG; break;
   default:
      UNREACHABLE();
   }
   result->node[0] = param->node[0];
}

static void tr_logical_and_or(GLSL_TRANSLATION_T *result, DataflowFlavour flavour, GLSL_TRANSLATION_T *left, GLSL_TRANSLATION_T *right)
{
   GLSL_TRANSLATION_TYPE_T rep = GLSL_TRANSLATION_VOID;
   uint32_t cond_setf;

   assert(flavour == DATAFLOW_LOGICAL_AND || flavour == DATAFLOW_LOGICAL_OR);
   assert(left->type  == GLSL_TRANSLATION_BOOL_FLAG || left->type  == GLSL_TRANSLATION_BOOL_FLAG_N);
   assert(right->type == GLSL_TRANSLATION_BOOL_FLAG || right->type == GLSL_TRANSLATION_BOOL_FLAG_N);

   bool l1 = left->type == GLSL_TRANSLATION_BOOL_FLAG;
   bool r1 = right->type == GLSL_TRANSLATION_BOOL_FLAG;

   if (flavour == DATAFLOW_LOGICAL_AND) {
      rep = GLSL_TRANSLATION_BOOL_FLAG;
   } else { /* DATAFLOW_LOGICAL_OR */
      /* A || B == !( !A && !B) */
      rep = GLSL_TRANSLATION_BOOL_FLAG_N;
      r1 = !r1; l1 = !l1;
   }

   if      ( l1 &&  r1) cond_setf = SETF_ANDZ;
   /* TODO: The next two look the wrong way round. ANDN <-> NORNZ because
    *       arguments are flipped later. dep1 is target. */
   else if (!l1 &&  r1) cond_setf = SETF_NORNZ;
   else if ( l1 && !r1) cond_setf = SETF_ANDNZ;
   else if (!l1 && !r1) cond_setf = SETF_NORZ;
   else UNREACHABLE();

   result->type = rep;
   result->node[0] = tr_conditional_uop(BACKFLOW_MOV, cond_setf, left->node[0], right->node[0]);
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
   /* Backend does not support cond ops on boolean values */
   /* Forced to build out of ANDs and ORs */
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
   Backflow *result = tr_nullary_io(BACKFLOW_TEX_GET, iodep0);

   /* Drop all the existing tmu deps. This stops the write's deps leaking
    * through to the reads (and, hence, everything subsequent).
    */
   result->tmu_deps = NULL;
   tmu_dep_append(&result->tmu_deps, lookup);

   return result;
}

/*
For each of these tmu dependencies, unset the per_pixel_mask bit
TODO: it would be better if we had the information about which textures depend on which earlier, so that we wouldn't have to do this fixing up.
*/
#define TMU_CFG_PIX_MASK (1<<7)
/* TODO: Make this work with the new TMU config */
#if !V3D_HAS_NEW_TMU_CFG
static void set_tmu_dependencies_to_per_quad(struct tmu_dep_s *dep_list)
{
   while (dep_list != NULL)
   {
      Backflow *node = dep_list->l->first_write;

      assert(node->magic_write == REG_MAGIC_TMU  ||
             node->magic_write == REG_MAGIC_TMUL ||
             node->magic_write == REG_MAGIC_TMUA ||
             node->magic_write == REG_MAGIC_TMUAU );

      if (node->unif_type == BACKEND_UNIFORM_TEX_PARAM0) node->unif &= ~GLSL_TEXBITS_PIX_MASK;

      if (node->magic_write == REG_MAGIC_TMUA) {
         /* First add an explicit load of the default config so we can update it */
         assert(node->unif_type == BACKEND_UNIFORM_UNASSIGNED);
         node->magic_write = REG_MAGIC_TMUAU;
         node->unif_type = BACKEND_UNIFORM_LITERAL;
         node->unif = ~0u;
      }

      if (node->magic_write == REG_MAGIC_TMUAU) {
         assert(node->unif_type == BACKEND_UNIFORM_LITERAL);
         node->unif &= ~TMU_CFG_PIX_MASK;
      }

      dep_list = dep_list->next;
   }
}
#endif

static struct tmu_lookup_s *new_tmu_lookup(SchedBlock *block) {
   struct tmu_lookup_s *new_lookup = malloc_fast(sizeof(struct tmu_lookup_s));
   new_lookup->tmu_deps = NULL;
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
   if (!high) return tr_unary_op(BACKFLOW_UNPACK_16A_F, in);
   else       return tr_unary_op(BACKFLOW_UNPACK_16B_F, in);
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
      if (type == DF_FSAMPLER) {
         for (int i=0; i<4; i++) if (components & (1<<i)) output[i] = unpack_float(words[i/2], i&1);
      } else if (type == DF_USAMPLER) {
         for (int i=0; i<4; i++) if (components & (1<<i)) output[i] = unpack_uint(words[i/2], i&1);
      } else {
         for (int i=0; i<4; i++) if (components & (1<<i)) output[i] = unpack_int (words[i/2], i&1);
      }
   }
}

#if V3D_HAS_NEW_TMU_CFG
static Backflow *tr_texture_cfg(int i, uint32_t cfg, Backflow *dep) {
   static const BackendUniformFlavour t[3] = { BACKEND_UNIFORM_TEX_PARAM0, BACKEND_UNIFORM_TEX_PARAM1, BACKEND_UNIFORM_LITERAL };
   Backflow *ret = tr_nullary_io(BACKFLOW_WRTMUC, dep);
   ret->unif_type = t[i];
   ret->unif = cfg;
   return ret;
}

static void tr_texture_lookup(GLSL_TRANSLATE_CONTEXT_T *ctx, GLSL_TRANSLATION_T *result, uint32_t sampler_index, DataflowType type, bool is_32bit, uint32_t age, uint32_t required_components, uint32_t extra, Backflow *s, Backflow *t, Backflow *r, Backflow *idx, Backflow *d, Backflow *b)
{
   int word_reads = get_word_reads(required_components, is_32bit);
   bool fetch = (extra & GLSL_TEXBITS_FETCH);

   /* TODO: We want to use px_enable here, but we can't remove the config properly */
   int n_cfg_words;
   uint32_t cfg_word[3];
   cfg_word[0] = (sampler_index << 4) | word_reads;
   cfg_word[1] = (sampler_index << 4) | (0 << 2) | (0 << 1) | is_32bit;

   bool gather = (extra & GLSL_TEXBITS_GATHER);
   bool bslod  = (extra & GLSL_TEXBITS_BSLOD);
   uint32_t offset = (extra >> 19) & 0xFFF;
   int ltype = (extra & 7);
   bool is_cubemap = (ltype == GLSL_TEXBITS_CUBE || ltype == GLSL_TEXBITS_CUBE_ARRAY);

   if (gather || offset || (bslod && is_cubemap)) {
      int gather_comp = (extra & GLSL_TEXBITS_GATHER_COMP_MASK) >> GLSL_TEXBITS_GATHER_COMP_SHIFT;
      cfg_word[2] = (offset << 8) | (gather << 7) | (gather_comp << 5) | (bslod << 1);
      n_cfg_words = 3;
   } else
      n_cfg_words = 2;

   Backflow *rq_start = tr_nullary(BACKFLOW_DUMMY);
   Backflow *last_cfg = rq_start;
   for (int i=0; i<n_cfg_words; i++) last_cfg = tr_texture_cfg(i, cfg_word[i], last_cfg);

   static const uint32_t reg[5] = { REG_MAGIC_TMUT, REG_MAGIC_TMUR, REG_MAGIC_TMUI, REG_MAGIC_TMUDREF, REG_MAGIC_TMUB };
   Backflow *coords[5] = { t, r, idx, d, b };
   Backflow *coord_writes[5] = { NULL, };
   int n_coords = 0;
   for (int i=0; i<5; i++) {
      if (coords[i] != NULL) {
         coord_writes[n_coords++] = tr_mov_to_reg_io(reg[i], coords[i], rq_start);
      }
   }

   uint32_t s_reg;
   if (is_cubemap) s_reg = REG_MAGIC_TMUSCM;
   else if (fetch) s_reg = REG_MAGIC_TMUSF;
   else if (bslod) s_reg = REG_MAGIC_TMUSLOD;
   else            s_reg = REG_MAGIC_TMUS;

   Backflow *s_write = tr_mov_to_reg(s_reg, s);
   for (int i=0; i<n_coords; i++) glsl_iodep(s_write, coord_writes[i]);
   glsl_iodep(s_write, last_cfg);

   struct tmu_lookup_s *lookup = new_tmu_lookup(ctx->block);
   lookup->config_count = n_cfg_words;
   lookup->write_count  = n_coords + 1;
   lookup->first_write = rq_start;
   lookup->last_write = s_write;
   lookup->tmu_deps = s_write->tmu_deps;

   lookup->first_read = NULL;
   lookup->read_count = 0;
   Backflow *node = lookup->last_write;
   Backflow *words[4] = { NULL, };
   for (int i = 0; i < 4; i++) {
      if (word_reads & (1<<i)) {
         node = tr_texture_get(node, lookup);
         if (lookup->first_read == NULL) lookup->first_read = node;
         lookup->last_read = node;
         lookup->read_count++;
         words[i] = node;
      }
   }

   /* TODO: This bit won't work and we need to do something. */
   /* If we're using implicit LOD (ie. not bslod) then our inputs must be per-quad */
   //if ((extra & GLSL_TEXBITS_BSLOD) == 0)
   //   set_tmu_dependencies_to_per_quad(lookup->tmu_deps);

   /* TODO: The below is copied directly from the v3.3 version. Share the code properly */
   lookup->age = age;

   result->type = GLSL_TRANSLATION_VEC4;

   unpack_tmu_return(result->node, words, type, required_components, is_32bit);
}

#else

static void tr_texture_lookup(GLSL_TRANSLATE_CONTEXT_T *stuff, GLSL_TRANSLATION_T *result, uint32_t sampler_index, DataflowType type, bool is_32bit, uint32_t age, uint32_t required_components, uint32_t extra, Backflow *s, Backflow *t, Backflow *r, Backflow *idx, Backflow *d, Backflow *b)
{
   Backflow *node;
   Backflow *words[4] = { NULL, };
   struct tmu_lookup_s *lookup = new_tmu_lookup(stuff->block);
   int word_reads = get_word_reads(required_components, is_32bit);

   extra |= word_reads << GLSL_TEXBITS_WORD_READS_SHIFT;

   /* Set per-pixel mask enable bit. This will be removed again if any textures
    * or dFdx/dFdy depend on this (we don't have this information yet). */
   extra |= GLSL_TEXBITS_PIX_MASK;

   if (d != NULL) extra |= GLSL_TEXBITS_SHADOW;
   if (b != NULL && !(extra & (GLSL_TEXBITS_BSLOD | GLSL_TEXBITS_FETCH))) extra |= GLSL_TEXBITS_BIAS;

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

   if (stuff->v3d_version >= V3D_MAKE_VER(3, 3)) {
      /* GFXH-1332 prevents us using default coordinates on v3dv3.2 */
      while (n_extra_coords > 0 && is_const_zero(c[n_extra_coords-1])) {
         c[n_extra_coords-1] = NULL;
         n_extra_coords--;
      }
   }

   node = tr_mov_to_reg(REG_MAGIC_TMU, s);
   node->unif_type = BACKEND_UNIFORM_TEX_PARAM0;
   node->unif = BACKEND_UNIFORM_MAKE_PARAM(sampler_index, extra);
   lookup->first_write = node;

   node = tr_mov_to_reg_io( (n_extra_coords == 0) ? REG_MAGIC_TMUL : REG_MAGIC_TMU, t, node);
   node->unif_type = BACKEND_UNIFORM_TEX_PARAM1;
   node->unif = BACKEND_UNIFORM_MAKE_PARAM(sampler_index, extra);

   lookup->write_count = 2 + n_extra_coords;
   lookup->config_count = 2;

   for (int i=0; i<n_extra_coords; i++) {
      node = tr_mov_to_reg_io( (i == n_extra_coords-1) ? REG_MAGIC_TMUL : REG_MAGIC_TMU, c[i], node);
   }

   assert(lookup->write_count <= 5);
   assert(node != NULL);
   lookup->last_write = node;

   lookup->first_read = NULL;
   lookup->read_count = 0;
   for (int i = 0; i < 4; i++) {
      if (word_reads & (1<<i)) {
         node = tr_texture_get(node, lookup);
         if (lookup->first_read == NULL) lookup->first_read = node;
         lookup->last_read = node;
         lookup->read_count++;
         words[i] = node;
      }
   }

   lookup->tmu_deps = lookup->last_write->tmu_deps;

   /* If we're using implicit LOD (ie. not bslod) then our inputs must be per-quad */
   if ((extra & GLSL_TEXBITS_BSLOD) == 0)
      set_tmu_dependencies_to_per_quad(lookup->tmu_deps);

   lookup->age = age;

   result->type = GLSL_TRANSLATION_VEC4;

   unpack_tmu_return(result->node, words, type, required_components, is_32bit);
}

static void swizzle_words(Backflow *words[4], int swizzle[4], bool int_tex)
{
   Backflow *nodes[6];
   nodes[0] = tr_const(0);
   nodes[1] = int_tex ? tr_const(1) : tr_cfloat(1.0f);
   for (int i=0; i<4; i++) nodes[i+2] = words[i];

   for (int i=0; i<4; i++) words[i] = nodes[swizzle[i]];
}

static void tr_texture_gadget(GLSL_TRANSLATE_CONTEXT_T *stuff, GLSL_TRANSLATION_T *result, uint32_t sampler_index, DataflowType type, bool is_32bit, uint32_t age, uint32_t required_components, uint32_t extra, Backflow *s, Backflow *t, Backflow *r, Backflow *i, Backflow *d, Backflow *b)
{
   int swizzle[4] = { 2, 3, 4, 5};
   uint32_t gadgettype;

   assert(sampler_index < vcos_countof(stuff->gadgettype));
   gadgettype = stuff->gadgettype[sampler_index];
   if (GLSL_GADGETTYPE_NEEDS_SWIZZLE(gadgettype))
   {
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
         UNREACHABLE();
      }

      /* For shadow clamp D_ref to [0,1] for fixed point textures. */
      if (gadgettype == GLSL_GADGETTYPE_DEPTH_FIXED && d != NULL) {
         d = tr_binop(BACKFLOW_MAX, tr_cfloat(0.0f), d);
         d = tr_binop(BACKFLOW_MIN, tr_cfloat(1.0f), d);
      }

      tr_texture_lookup(stuff, result, sampler_index, type, is_32bit, age, lookup_required_components, extra, s, t, r, i, d, b);

      /* The dataflow type tells us how the texture was declared in the shader,
       * the gadgettype what type the actual texture is. If they don't match then
       * the values returned are undefined per the spec. We only pay attention to
       * the gadget to the extent required to get the defined behaviour right. */
      Backflow *word = result->node[0];
      switch (gadgettype)
      {
      case GLSL_GADGETTYPE_INT8:
         if (type == DF_USAMPLER) {
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
      {
         if (required_components & 1) result->node[0] = bitand(word,          tr_const(0x3ff));
         if (required_components & 2) result->node[1] = bitand(shr(word, 10), tr_const(0x3ff));
         if (required_components & 4) result->node[2] = bitand(shr(word, 20), tr_const(0x3ff));
         if (required_components & 8) result->node[3] =        shr(word, 30);
         break;
      }
      default:
         /* Do nothing */
         break;
      }
   }

   /* Set type here in case all the results are coming from swizzles */
   result->type = GLSL_TRANSLATION_VEC4;
   swizzle_words(result->node, swizzle, (type != DF_FSAMPLER));
}
#endif

static void tr_get_col_gadget(GLSL_TRANSLATE_CONTEXT_T *stuff,
                              GLSL_TRANSLATION_T *result,
                              DataflowType type,
                              uint32_t required_components,
                              uint32_t render_target)
{
   uint32_t config_byte;
   int len;
   if (type == DF_FLOAT) {
      if (required_components & 0xC) len = 2;
      else                           len = 1;
      V3D_TLB_CONFIG_COLOR_F16_T cfg = { .num_words = len, .no_swap = true,
                                         .all_samples_same_data = true, .rt = render_target };
      config_byte = v3d_pack_tlb_config_color_f16(&cfg);
   } else {
      int trc = required_components;
      len = 0;
      while (trc != 0) {
         trc >>= 1;
         len++;
      }
      V3D_TLB_CONFIG_COLOR_32_T cfg = { .num_words = len, .all_samples_same_data = true,
                                        .rt = render_target, .as_int = true };
      config_byte = v3d_pack_tlb_config_color_32(&cfg);
   }

   Backflow *load[4];
   /* Create a chain of reads for this sample */
   if (stuff->sample_num == 0) {
      load[0] = tr_nullary(BACKFLOW_FRAG_GET_TLBU);
      load[0]->unif_type = BACKEND_UNIFORM_LITERAL;
      load[0]->unif = config_byte | 0xffffff00; /* Unused config entries must be all 1s */
   } else
      load[0] = tr_nullary(BACKFLOW_FRAG_GET_TLB);

   for (int i=1; i<len; i++)
      load[i] = tr_nullary_io(BACKFLOW_FRAG_GET_TLB, load[i-1]);

   /* Append this chain to the existing one */
   struct tlb_read_s *read = &stuff->block->tlb_read[render_target];
   if (stuff->sample_num == 0) {
      assert(read->first == NULL && read->last == NULL );

      read->first = load[0];
      read->last = load[len-1];
   } else {
      assert(read->first != NULL && read->last != NULL);

      glsl_iodep(load[0], read->last);
      read->last = load[len-1];
   }

   read->samples_read |= 1<<stuff->sample_num;

   if (type == DF_FLOAT) {
      for (int i=0; i<2*len; i++) result->node[i] = unpack_float(load[i/2], i&1);
   } else {
      for (int i=0; i<len; i++) result->node[i] = load[i];
   }
   result->type = GLSL_TRANSLATION_VEC4;
}

static Backflow *tr_indexed_read_gadget(SchedBlock *block, int age, Backflow *addr)
{
   struct tmu_lookup_s *lookup = new_tmu_lookup(block);
   Backflow *node = tr_mov_to_reg(REG_MAGIC_TMUA, addr);

   lookup->first_write = lookup->last_write = node;
   lookup->write_count = 1;
   lookup->config_count = 2;     /* The hardware requires 2 config slots even for DML */
   lookup->read_count = 1;

   node = tr_texture_get(node, lookup);
   lookup->first_read = lookup->last_read = node;

   lookup->tmu_deps = lookup->last_write->tmu_deps;
   lookup->age = age;

   return node;
}

Backflow *glsl_backflow_fake_tmu(SchedBlock *block) {
   struct tmu_lookup_s *lookup = new_tmu_lookup(block);
   Backflow *zero = tr_const(0);
   Backflow *node = tr_conditional_uop(BACKFLOW_MOV, COND_IFNFLAG, zero, zero);
   assert(node->magic_write == REG_UNDECIDED);
   node->magic_write = REG_MAGIC_TMUA;

   lookup->first_write = lookup->last_write = node;
   lookup->write_count = 1;
   lookup->config_count = 2;     /* The hardware requires 2 config slots even for DML */
   lookup->read_count = 1;

   node = tr_texture_get(node, lookup);
   lookup->first_read = lookup->last_read = node;

   lookup->tmu_deps = lookup->last_write->tmu_deps;
   lookup->age = 0;  /* XXX Is this bad? */

   return node;
}

static void tr_indexed_read_vector_gadget(SchedBlock *block,
                                          GLSL_TRANSLATION_T *result,
                                          int age, Backflow *addr, uint32_t components)
{
   static const uint32_t read_unifs[5] = { 0 /* Not valid */, 0xffffffff, 0xfffffffa, 0xfffffffb, 0xfffffffc };

   int num_reads = 0;

   while (components != 0) {
      components >>= 1;
      num_reads++;
   }
   assert(num_reads > 0 && num_reads <= 4);

   struct tmu_lookup_s *lookup = new_tmu_lookup(block);

   Backflow *node = tr_mov_to_reg(REG_MAGIC_TMUAU, addr);
   node->unif_type = BACKEND_UNIFORM_LITERAL;
   node->unif = read_unifs[num_reads];

   lookup->first_write = lookup->last_write = node;
   lookup->write_count = 1;
   lookup->config_count = 2;

   result->type = GLSL_TRANSLATION_VEC4;
   result->node[0] = tr_texture_get(node, lookup);
   for (int i=1; i<num_reads; i++) result->node[i] = tr_texture_get(result->node[i-1], lookup);

   lookup->read_count = num_reads;
   lookup->first_read = result->node[0];
   lookup->last_read = result->node[num_reads-1];

   lookup->tmu_deps = lookup->last_write->tmu_deps;
   lookup->age = age;
}

/*
High-level functions
*/

static Backflow *glsl_tr_vary(uint32_t row, Backflow *w, Backflow *dep) {
   Backflow *result = tr_uop_io(BACKFLOW_VARYING, w, dep);
   result->varying = row;
   return result;
}

static Backflow *glsl_tr_vary_flat(uint32_t row, Backflow *dep) {
   Backflow *result = tr_nullary_io(BACKFLOW_VARYING_FLAT, dep);
   result->varying = row;
   return result;
}

static Backflow *glsl_tr_vary_non_perspective(uint32_t row, Backflow *dep) {
   /* GNL TODO: We could just pass w = cfloat(1.0f) to tr_vary */
   /* The resulting code wouldn't even be any worse */
   Backflow *result = tr_nullary_io(BACKFLOW_VARYING_LINE_COORD, dep);
   result->varying = row;
   return result;
}

static Backflow *init_frag_vary( SchedShaderInputs *in,
                                 uint32_t primitive_type,
                                 const VARYING_INFO_T *varying)
{
   Backflow *dep = NULL;

   if (primitive_type == GLXX_PRIM_POINT) {
      dep = in->point_x = glsl_tr_vary(VARYING_ID_HW_0, in->w, dep);
      dep = in->point_y = glsl_tr_vary(VARYING_ID_HW_1, in->w, dep);
   } else {
      in->point_x = tr_cfloat(0.0f);
      in->point_y = tr_cfloat(0.0f);
   }

   if (primitive_type == GLXX_PRIM_LINE)
      dep = in->line = glsl_tr_vary_non_perspective(VARYING_ID_HW_0, dep);
   else
      in->line = tr_cfloat(0.0f);

   memset(in->inputs, 0, sizeof(Backflow *)*(V3D_MAX_VARYING_COMPONENTS));
   for (int i = 0; i < V3D_MAX_VARYING_COMPONENTS; i++)
   {
      Backflow *v;
      if (varying[i].flat)
         v = glsl_tr_vary_flat(i, dep);
      else if (varying[i].centroid)
         v = glsl_tr_vary(i, in->w_c, dep);
      else
         v = glsl_tr_vary(i, in->w, dep);

      in->inputs[i] = v;
   }

   return dep;
}

static_assrt(GLXX_MAX_RENDER_TARGETS == V3D_MAX_RENDER_TARGETS);

static void fragment_backend_state_init_gl(
   GLSL_FRAGMENT_BACKEND_STATE_T *s,
   uint32_t backend)
{
   s->sample_alpha_to_coverage = !!(backend & GLXX_SAMPLE_ALPHA);
   s->sample_mask = !!(backend & GLXX_SAMPLE_MASK);
   s->ms = !!(backend & GLXX_SAMPLE_MS);
   s->skip_discard = !!(backend & GLXX_SKIP_DISCARD);
   for (int i=0; i<GLXX_MAX_RENDER_TARGETS; i++) {
      int shift = GLXX_FB_GADGET_S + 2*i;
      uint32_t mask = GLXX_FB_GADGET_M << shift;
      s->render_target_type[i] = (backend & mask) >> shift;
   }
}

static Backflow *coverage_and(Backflow *mask, Backflow *dep) {
   return tr_uop_io( BACKFLOW_FRAG_SUBMIT_MSF,
                     bitand(tr_nullary(BACKFLOW_MSF), mask),
                     dep);
}

static void resolve_tlb_loads(SchedBlock *block, bool ms) {
   /* Sort out TLB loading dependencies */
   Backflow *first_read = NULL;
   Backflow *last_read = NULL;

   for (int rt=0; rt<V3D_MAX_RENDER_TARGETS; rt++) {
      struct tlb_read_s *r = &block->tlb_read[rt];
      /* If this RT isn't read then skip it */
      if (r->samples_read == 0) continue;

      /* We must load all samples for any RT that we read */
      assert(r->samples_read == (ms ? 0xF : 0x1));

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

static Backflow *fragment_backend(
   const GLSL_FRAGMENT_BACKEND_STATE_T *s,
   Backflow *const *rgbas,
   Backflow *discard,
   Backflow *depth,
   SchedBlock *block,
   bool *does_discard_out, bool *does_z_change_out)
{
#define R(RT, SAMPLE) (rgbas[(((RT) * 4) + 0)*4 + (SAMPLE)])
#define G(RT, SAMPLE) (rgbas[(((RT) * 4) + 1)*4 + (SAMPLE)])
#define B(RT, SAMPLE) (rgbas[(((RT) * 4) + 2)*4 + (SAMPLE)])
#define A(RT, SAMPLE) (rgbas[(((RT) * 4) + 3)*4 + (SAMPLE)])

   Backflow *coverage = NULL;

   if (s->sample_alpha_to_coverage)
      coverage = coverage_and( tr_unary_op(BACKFLOW_FTOC, A(0,0)), coverage );

   if (s->sample_mask)
      coverage = coverage_and( tr_special_uniform(BACKEND_SPECIAL_UNIFORM_SAMPLE_MASK), coverage );

   /* The conditions appear inverted here because there's an extra invert
    * when going from uniform values to flags for conditions              */
   if (discard && is_const(discard) && discard->unif == CONST_BOOL_TRUE)
      discard = NULL;

   if (discard != NULL)
      coverage = tr_discard(coverage, discard);

   /* Sort out TLB storing dependencies */
   Backflow *first_write = NULL;
   Backflow *last_write  = NULL;
   bool does_discard  = (discard != NULL || s->sample_alpha_to_coverage || s->sample_mask);
   bool does_z_change = (depth != NULL);
   if (s->skip_discard) does_discard = false;

   {
      Backflow *tlb_nodes[4*4*V3D_MAX_RENDER_TARGETS+1]; /* or more? */
      uint32_t unif_byte[4*4*V3D_MAX_RENDER_TARGETS+1];

      uint32_t tlb_depth_age  = 0;
      unsigned tlb_node_count = 0;

      if (does_z_change)
      {
         unif_byte[tlb_node_count] = 0x84;  /* per-pixel Z write */
         tlb_nodes[tlb_node_count++] = depth;
         tlb_depth_age = depth->age;
      }
      else if (does_discard)
      {
         unif_byte[tlb_node_count] = 0x80;  /* invariant Z write */
         tlb_nodes[tlb_node_count++] = tr_const(0);   /* H/W doesn't use this */
      }

      for (unsigned i = 0; i < V3D_MAX_RENDER_TARGETS; i++)
      {
         if (s->render_target_type[i] == GLXX_FB_NOT_PRESENT) continue;

         assert(s->render_target_type[i] == GLXX_FB_F16 ||
                s->render_target_type[i] == GLXX_FB_F32 ||
                s->render_target_type[i] == GLXX_FB_I32);

         if (R(i, 0) != NULL)    /* Don't write anything if output is uninitialised */
         {
            int rt_channels = 1;    /* We know R is present */
            if (G(i,0) != NULL) rt_channels++;
            if (B(i,0) != NULL) rt_channels++;
            if (A(i,0) != NULL) rt_channels++;

            /* Work around GFXH-1212 by writing 4 channels for F16 or I32 */
            if (s->render_target_type[i] == GLXX_FB_F16 || s->render_target_type[i] == GLXX_FB_I32)
               rt_channels = 4;

            /* We set this up once per render target. The config byte is
             * emitted with the first sample that's written
             */
            int vec_sz = rt_channels;
            int swap = 0;
            if (s->render_target_type[i] == GLXX_FB_F16) {
               vec_sz = (rt_channels+1)/2;   /* F16 targets have half the outputs */
               swap = 1;                     /* and support the 'swap' field (bit 2) */
            }
            assert(vec_sz <= 4);

            bool per_sample = block->per_sample;
            uint8_t config_byte = (s->render_target_type[i] << 6) | ((7-i) << 3) |
                                  ((!per_sample)<<2) | (swap << 1) | (vec_sz-1);

            int samples = per_sample ? 4 : 1;
            for (int sm=0; sm<samples; sm++) {
               Backflow *out[4] = {R(i, sm), G(i, sm), B(i, sm), A(i, sm)};

               /* Pad out the output array for working around GFXH-1212 */
               for (int j=0; j<4; j++) if (out[j] == NULL) out[j] = tr_cfloat(0.0f);

               if (s->render_target_type[i] == GLXX_FB_F16) {
                  /* Pack the values in pairs for output */
                  for (int j=0; j<vec_sz; j++) {
                     assert(out[2*j] != NULL && out[2*j+1] != NULL);
                     dataflow_age = MAX(out[2*j]->age, out[2*j+1]->age);
                     dataflow_age = MAX(dataflow_age, tlb_depth_age);
                     out[j] = tr_binop(BACKFLOW_PACK_FLOAT16, out[2*j], out[2*j+1]);
                  }
               }

               /* Write nodes for our output, config_byte with the first */
               for (int j=0; j<vec_sz; j++) {
                  unif_byte[tlb_node_count] = (j == 0 && sm == 0) ? config_byte : ~0;
                  assert(out[j] != NULL);
                  tlb_nodes[tlb_node_count++] = out[j];
               }
            }
         }
      }

      /* QPU restrictions prevent us writing nothing to the TLB. Write some fake data */
      if (block->first_tlb_read == NULL && tlb_node_count == 0) {
         unif_byte[tlb_node_count] = 0x3F;
         tlb_nodes[tlb_node_count++] = tr_const(0);
         for (int i=0; i<3; i++) {
            unif_byte[tlb_node_count] = ~0u;
            tlb_nodes[tlb_node_count++] = tr_const(0);
         }
      }

      Backflow *dep = NULL;
      assert(tlb_node_count <= vcos_countof(tlb_nodes));
      for (unsigned i = 0; i < tlb_node_count; i++)
      {
         dataflow_age = MAX(dataflow_age, tlb_nodes[i]->age);
         if (dep)
            dataflow_age = MAX(dataflow_age, dep->age);
         if (unif_byte[i] != ~0u)
         {
            uint32_t unif = 0;
            uint32_t unif_shift = 0;
            /* Read ahead to find the next (up to) 4 unif bytes */
            for (unsigned j = i; j < tlb_node_count && unif_shift < 32; j++)
            {
               if (unif_byte[j] != ~0u)
               {
                  assert(unif_byte[j] <= 0xff);
                  unif |= unif_byte[j] << unif_shift;
                  unif_shift += 8;
                  unif_byte[j] = ~0u;  /* stops us adding it next time */
               }
            }
            /* Unused config entries must be all 1s */
            if (unif_shift < 32)
               unif |= 0xffffffff << unif_shift;

            if (unif != ~0u) {
               dep = tr_mov_to_reg_io(REG_MAGIC_TLBU, tlb_nodes[i], dep);
               dep->unif_type = BACKEND_UNIFORM_LITERAL;
               dep->unif = unif;
            } else {
               /* If the config is the default, don't write it at all */
               dep = tr_mov_to_reg_io(REG_MAGIC_TLB, tlb_nodes[i], dep);
            }
         }
         else
         {
            dep = tr_mov_to_reg_io(REG_MAGIC_TLB, tlb_nodes[i], dep);
         }
         if (first_write == NULL) first_write = dep;
      }
      last_write = dep;
   }

   block->first_tlb_write = first_write;

   if (first_write != NULL)
      glsl_iodep(first_write, coverage);

   *does_discard_out  = does_discard;
   *does_z_change_out = does_z_change;
   return last_write;

#undef A
#undef B
#undef G
#undef R
}

/* transform from clip-space coordinates to *almost* window coordinates. the
 * x/y coordinates are translated by the viewport offset in hardware to get
 * actual window coordinates */
static void vertex_clip_to_almostwin(
   Backflow **recip_w_out,
   Backflow **almostwin_x, Backflow **almostwin_y, /* fixed point 12.4 */
   Backflow **win_z_out, /* float */
   Backflow *clip_x, Backflow *clip_y, Backflow *clip_z, Backflow *clip_w /* float */ )
{
   Backflow *recip_w;
   Backflow *win_x, *win_y, *win_z, *u;
   bool w_is_one = false;

   if (clip_w->type == SIG && clip_w->sigbits == SIGBIT_LDUNIF && clip_w->unif_type == BACKEND_UNIFORM_LITERAL) {
      float w_val = float_from_bits(clip_w->unif);
      w_is_one = (w_val == 1.0f);
      recip_w = tr_cfloat(1.0f/w_val);
   } else {
      recip_w = tr_mov_to_reg(REG_MAGIC_RECIP, clip_w);
   }
   *recip_w_out = recip_w;

   u = tr_special_uniform(BACKEND_SPECIAL_UNIFORM_VP_SCALE_X);
   win_x = mul(clip_x, u);
   u = tr_special_uniform(BACKEND_SPECIAL_UNIFORM_VP_SCALE_Y);
   win_y = mul(clip_y, u);
   u = tr_special_uniform(BACKEND_SPECIAL_UNIFORM_VP_SCALE_Z);
   win_z = mul(clip_z, u);
   if (!w_is_one) {
      win_x = mul(win_x, recip_w);
      win_y = mul(win_y, recip_w);
      win_z = mul(win_z, recip_w);
   }

   *almostwin_x = tr_unary_op(BACKFLOW_FTOI_NEAREST, win_x);
   *almostwin_y = tr_unary_op(BACKFLOW_FTOI_NEAREST, win_y);

   u = tr_special_uniform(BACKEND_SPECIAL_UNIFORM_VP_SCALE_W);
   *win_z_out = add(win_z, u);
}

#if V3D_HAS_LDVPM

static void vpmw(SchedBlock *b, uint32_t addr, Backflow *param, Backflow *dep)
{
   Backflow *result = tr_binop(BACKFLOW_STVPM, tr_const(addr), param);
   glsl_iodep(result, dep);
   result->age = param->age;

   glsl_backflow_chain_append(&b->iodeps, result);
}

static void vertex_backend(SchedBlock *block, SchedShaderInputs *ins,
                           Backflow *clip_x, Backflow *clip_y,
                           Backflow *clip_z, Backflow *clip_w,
                           Backflow *point_size,
                           bool write_clip_header,
                           bool z_only_write,
                           Backflow **vertex_vary,
                           const GLSL_VARY_MAP_T *vary_map)
{
   Backflow *fake_vpm_deps[MAX_VPM_DEPENDENCY] = { NULL, };
   Backflow **vpm_dep = ins ? ins->vpm_dep : fake_vpm_deps;

   block->first_vpm_write = NULL;   /* Not used on v3.4 */

   Backflow *almostwin_x = NULL, *almostwin_y = NULL;
   Backflow *win_z, *recip_w;
   if (clip_x != NULL && clip_y != NULL && clip_z != NULL && clip_w != NULL)
      vertex_clip_to_almostwin(&recip_w, &almostwin_x, &almostwin_y, &win_z,
                               clip_x, clip_y, clip_z, clip_w);
   else {
      win_z = tr_const(0);
      recip_w = tr_const(1);
   }

   if (write_clip_header) {
      if (clip_x != NULL) vpmw(block, 0, clip_x, vpm_dep[0]);
      if (clip_y != NULL) vpmw(block, 1, clip_y, vpm_dep[1]);
      if (clip_z != NULL) vpmw(block, 2, clip_z, vpm_dep[2]);
      if (clip_w != NULL) vpmw(block, 3, clip_w, vpm_dep[3]);
      if (almostwin_x != NULL) vpmw(block, 4, almostwin_x, vpm_dep[4]);
      if (almostwin_y != NULL) vpmw(block, 5, almostwin_y, vpm_dep[5]);
   } else {
      if (almostwin_x != NULL) vpmw(block, 0, almostwin_x, vpm_dep[0]);
      if (almostwin_y != NULL) vpmw(block, 1, almostwin_y, vpm_dep[1]);
      if (win_z       != NULL) vpmw(block, 2, win_z,       vpm_dep[2]);
      if (recip_w     != NULL) vpmw(block, 3, recip_w,     vpm_dep[3]);
   }

   unsigned addr = write_clip_header ? 6 : 4;

   if (point_size != NULL) {
      vpmw(block, addr, point_size, vpm_dep[addr]);
      addr++;
   }

   if (z_only_write) {
      /* For z-only mode we output z,w normally, just z for points */
      /* TODO: This screws up if points are switched on for TF */
      vpmw(block, addr, win_z, vpm_dep[addr]);
      addr++;
      if (point_size == NULL) {
         vpmw(block, addr, recip_w, vpm_dep[addr]);
         addr++;
      }
   }

   for (int i = 0; i < vary_map->n; i++) {
      if (vertex_vary[vary_map->entries[i]] != NULL) {
         Backflow *read_dep = (addr+i) < MAX_VPM_DEPENDENCY ? vpm_dep[addr+i] : NULL;
         vpmw(block, addr + i, vertex_vary[vary_map->entries[i]], read_dep);
      }
   }
}

static Backflow *vpm_read(SchedShaderInputs *ins, int *addr) {
   Backflow *r = tr_unary_op(BACKFLOW_LDVPM, tr_const(*addr));
   ins->vpm_dep[*addr] = r;
   (*addr) += 1;
   return r;
}

static Backflow *fetch_all_attribs(SchedShaderInputs *ins, const ATTRIBS_USED_T *attr, uint32_t reads_total)
{
   int addr = 0;
   if (attr->instanceid_used) ins->instanceid = vpm_read(ins, &addr);
   if (attr->vertexid_used)   ins->vertexid   = vpm_read(ins, &addr);

   for (int i = 0; i < V3D_MAX_ATTR_ARRAYS; i++) {
      for (unsigned j=0; j<attr->scalars_used[i]; j++) {
         ins->inputs[4*i+j] = vpm_read(ins, &addr);
      }
   }

   /* Last read irrelevant, so return NULL on v3.4 */
   return NULL;
}

#else

static Backflow *vpm_write_setup(uint32_t addr)
{
   assert(addr <= 0x1fff);
   uint32_t value = 1<<24 | 1<<22 | 1<<15 | 2<<13 | (addr & 0x1fff);
   return tr_unary_op(BACKFLOW_VPM_SETUP, tr_const(value));
}

/* Write depends on both the previous write and the read from this location */
static Backflow *vpmw(Backflow *param, Backflow *write_dep, Backflow *read_dep)
{
   Backflow *result = tr_mov_to_reg(REG_MAGIC_VPM, param);
   glsl_iodep(result, write_dep);
   glsl_iodep(result, read_dep);
   result->age = param->age;
   if (result->age == 0) result->age = write_dep->age;
   return result;
}

static void vertex_backend(SchedBlock *block, SchedShaderInputs *ins,
                           Backflow *clip_x, Backflow *clip_y,
                           Backflow *clip_z, Backflow *clip_w,
                           Backflow *point_size,
                           bool write_clip_header,
                           bool z_only_write,
                           Backflow **vertex_vary,
                           const GLSL_VARY_MAP_T *vary_map)
{
   Backflow *dep;
   Backflow *win_z, *recip_w;
   Backflow *fake_vpm_deps[MAX_VPM_DEPENDENCY] = { NULL, };
   Backflow **vpm_dep = ins ? ins->vpm_dep : fake_vpm_deps;
   unsigned write_index = 0;

   /* Unless the coordinates are valid then we skip writing them at all. The *
    * clip header, however, can be used for transform feedback, so if any    *
    * value is valid it must be output. Here we fill in the values to be     *
    * output, which has the side effect of making the coordinates valid, so  *
    * they get output below.                                                 */
   if (write_clip_header && (clip_x != NULL || clip_y != NULL ||
                             clip_z != NULL || clip_w != NULL   ) )
   {
      if (clip_x == NULL) clip_x = tr_const(0);
      if (clip_y == NULL) clip_y = tr_const(0);
      if (clip_z == NULL) clip_z = tr_const(0);
      if (clip_w == NULL) clip_w = tr_const(1);
   }

   if (clip_x != NULL && clip_y != NULL && clip_z != NULL && clip_w != NULL) {
      dep = vpm_write_setup(0);
      block->first_vpm_write = dep;

      if (write_clip_header)
      {
         dep = vpmw(clip_x, dep, vpm_dep[write_index++]);
         dep = vpmw(clip_y, dep, vpm_dep[write_index++]);
         dep = vpmw(clip_z, dep, vpm_dep[write_index++]);
         dep = vpmw(clip_w, dep, vpm_dep[write_index++]);
      }

      Backflow *almostwin_x, *almostwin_y;

      vertex_clip_to_almostwin(&recip_w, &almostwin_x, &almostwin_y, &win_z,
                               clip_x, clip_y, clip_z, clip_w);

      dep = vpmw(almostwin_x, dep, vpm_dep[write_index++]);
      dep = vpmw(almostwin_y, dep, vpm_dep[write_index++]);
      if (!write_clip_header)
      {
         dep = vpmw(win_z,   dep, vpm_dep[write_index++]);
         dep = vpmw(recip_w, dep, vpm_dep[write_index++]);
      }
   } else {
      write_index = write_clip_header ? 6 : 4;
      dep = vpm_write_setup(write_index);
      block->first_vpm_write = dep;
      win_z = tr_const(0);
      recip_w = tr_const(1);
   }

   if (point_size != NULL) dep = vpmw(point_size, dep, vpm_dep[write_index++]);

   if (z_only_write) {
      /* For z-only mode we output z,w normally, just z for points */
      /* TODO: This screws up if points are switched on for TF */
      dep = vpmw(win_z, dep, vpm_dep[write_index++]);
      if (point_size == NULL) dep = vpmw(recip_w, dep, vpm_dep[write_index++]);
   }

   assert(vary_map != NULL);
   for (int i = 0; i < vary_map->n; i++) {
      Backflow *read_dep = write_index < MAX_VPM_DEPENDENCY ? vpm_dep[write_index] : NULL;
      write_index++;
      /* TODO: What if other things are uninitialised? */
      if (vertex_vary[vary_map->entries[i]] != NULL)
         dep = vpmw(vertex_vary[vary_map->entries[i]], dep, read_dep);
      else
         dep = vpmw(tr_const(0), dep, read_dep);
   }

   glsl_backflow_chain_append(&block->iodeps, dep);
}

static Backflow *tr_vpm_read_setup(bool horiz, uint32_t count, uint32_t addr, Backflow *dep)
{
   Backflow *result;
   uint32_t value;
   assert(count >= 1 && count <= 32);
   assert(addr >= 0 && addr <= 0x1fff);
   value = 1<<30 | 1<<27 | 1<<15 | 2<<13 | (horiz ? 1<<29 : 0) | (count & 31) << 22 | (addr & 0x1fff);
   result = tr_uop_io(BACKFLOW_VPM_SETUP, tr_const(value), dep);
   /* XXX Hack this to make it come out early */
   result->age = 0;
   return result;
}

struct attrib_fetch_state_s {
   int reads_total;
   int reads_done;
   int remaining_batch;
   Backflow *last_vpm_read;
};

static Backflow *fetch_attrib(SchedShaderInputs *ins,
                              struct attrib_fetch_state_s *state)
{
   /* If there's no current VPM setup, create a new one */
   if (state->remaining_batch == 0)
   {
      assert(state->reads_done < state->reads_total);
      state->remaining_batch = MIN(state->reads_total - state->reads_done, 32);
      state->last_vpm_read = tr_vpm_read_setup(true, state->remaining_batch,
                                                     state->reads_done,
                                                     state->last_vpm_read);
   }

   /* Do the actual read */
   state->last_vpm_read = tr_nullary_io(BACKFLOW_ATTRIBUTE, state->last_vpm_read);
   assert(state->reads_done < MAX_VPM_DEPENDENCY);
   ins->vpm_dep[state->reads_done] = state->last_vpm_read;
   state->reads_done++;
   state->remaining_batch--;
   return state->last_vpm_read;
}

static Backflow *fetch_all_attribs(SchedShaderInputs *ins, const ATTRIBS_USED_T *attr, uint32_t reads_total)
{
   struct attrib_fetch_state_s state;
   state.last_vpm_read = NULL;
   state.remaining_batch = 0;
   state.reads_done = 0;
   state.reads_total = reads_total;

   if (attr->instanceid_used) ins->instanceid = fetch_attrib(ins, &state);
   if (attr->vertexid_used)   ins->vertexid   = fetch_attrib(ins, &state);

   for (int i = 0; i < V3D_MAX_ATTR_ARRAYS; i++) {
      for (unsigned j=0; j<attr->scalars_used[i]; j++) {
         ins->inputs[4*i+j] = fetch_attrib(ins, &state);
      }
   }
   assert(state.reads_done == state.reads_total);
   return state.last_vpm_read;
}

#endif

static inline GLSL_TRANSLATION_TYPE_T get_translation_type(DataflowType type)
{
   assert(type == DF_BOOL || type == DF_INT ||
          type == DF_UINT || type == DF_FLOAT);

   if (type == DF_BOOL) return GLSL_TRANSLATION_BOOL_FLAG_N;
   else return GLSL_TRANSLATION_WORD;
}

static BackflowFlavour translate_nullary_flavour(DataflowFlavour flavour) {
   switch(flavour) {
      case DATAFLOW_FRAG_GET_X:        return BACKFLOW_FXCD;
      case DATAFLOW_FRAG_GET_Y:        return BACKFLOW_FYCD;
      case DATAFLOW_FRAG_GET_X_UINT:   return BACKFLOW_XCD;
      case DATAFLOW_FRAG_GET_Y_UINT:   return BACKFLOW_YCD;
      case DATAFLOW_GET_THREAD_INDEX:  return BACKFLOW_TIDX;
      default: assert(0); return 0;
   }
}
static BackflowFlavour translate_binop_flavour(DataflowFlavour flavour, DataflowType type)
{
   switch(flavour)
   {
      case DATAFLOW_ADD: return type == DF_FLOAT ? BACKFLOW_ADD : BACKFLOW_IADD;
      case DATAFLOW_SUB: return type == DF_FLOAT ? BACKFLOW_SUB : BACKFLOW_ISUB;
      case DATAFLOW_MIN: return type == DF_FLOAT ? BACKFLOW_MIN : ( type == DF_INT ? BACKFLOW_IMIN : BACKFLOW_UMIN );
      case DATAFLOW_MAX: return type == DF_FLOAT ? BACKFLOW_MAX : ( type == DF_INT ? BACKFLOW_IMAX : BACKFLOW_UMAX );
      case DATAFLOW_FPACK: return BACKFLOW_PACK_FLOAT16;
      case DATAFLOW_SHL: return BACKFLOW_SHL;
      case DATAFLOW_SHR: return type == DF_UINT ? BACKFLOW_SHR : BACKFLOW_ASHR;
      case DATAFLOW_BITWISE_AND: return BACKFLOW_BITWISE_AND;
      case DATAFLOW_BITWISE_OR: return BACKFLOW_BITWISE_OR;
      case DATAFLOW_BITWISE_XOR: return BACKFLOW_BITWISE_XOR;
      default: assert(0); return 0;
   }
}

static BackflowFlavour translate_unary_flavour(DataflowFlavour flavour, DataflowType type) {
   switch(flavour) {
      case DATAFLOW_FTOI_TRUNC: return BACKFLOW_FTOI_TRUNC;
      case DATAFLOW_FTOI_NEAREST: return BACKFLOW_FTOI_NEAREST;
      case DATAFLOW_FTOU: return BACKFLOW_FTOUZ;
      case DATAFLOW_ITOF: return BACKFLOW_ITOF;
      case DATAFLOW_CLZ:  return BACKFLOW_CLZ;
      case DATAFLOW_UTOF: return BACKFLOW_UTOF;
      case DATAFLOW_BITWISE_NOT: return BACKFLOW_BITWISE_NOT;
      case DATAFLOW_ARITH_NEGATE: return type == DF_FLOAT ? BACKFLOW_ARITH_NEGATE : BACKFLOW_IARITH_NEGATE;
      case DATAFLOW_FDX:      return BACKFLOW_FDX;
      case DATAFLOW_FDY:      return BACKFLOW_FDY;
      case DATAFLOW_FUNPACKA: return BACKFLOW_UNPACK_16A_F;
      case DATAFLOW_FUNPACKB: return BACKFLOW_UNPACK_16B_F;
      case DATAFLOW_TRUNC:    return BACKFLOW_TRUNC;
      case DATAFLOW_NEAREST:  return BACKFLOW_ROUND;
      case DATAFLOW_CEIL:     return BACKFLOW_CEIL;
      case DATAFLOW_FLOOR:    return BACKFLOW_FLOOR;
      case DATAFLOW_ABS:      return BACKFLOW_ABS;
      default: assert(0); return 0;
   }
}

static uint32_t get_magic_reg(DataflowFlavour flavour) {
   switch (flavour) {
      case DATAFLOW_RSQRT:  return REG_MAGIC_RSQRT;
      case DATAFLOW_RCP:    return REG_MAGIC_RECIP;
      case DATAFLOW_LOG2:   return REG_MAGIC_LOG;
      case DATAFLOW_EXP2:   return REG_MAGIC_EXP;
      default: assert(0); return 0;
   }
}

static void push_phi(const Dataflow *d, PhiList **list) {
   PhiList *new = malloc_fast(sizeof(PhiList));
   new->phi = d;
   new->next = *list;
   *list = new;
}

static Backflow *tr_external(int block, int output, ExternalList **list) {
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

static inline const Dataflow *relocate_dataflow(GLSL_TRANSLATE_CONTEXT_T *stuff, int dataflow)
{
   return &stuff->df_arr[dataflow];
}

/* Main translation function */
GLSL_TRANSLATION_T *glsl_translate_to_backend(GLSL_TRANSLATE_CONTEXT_T *stuff, int df_idx)
{
   GLSL_TRANSLATION_T *d[6], *r;

   if (df_idx == -1) return NULL;
   const Dataflow *dataflow = relocate_dataflow(stuff, df_idx);
   assert(dataflow->id < stuff->translations_count);
   r = &stuff->translations[dataflow->id];
   if (r->type != GLSL_TRANSLATION_UNVISITED)
      return r;

   assert(dataflow->dependencies_count <= 6);

   DataflowFlavour flavour = dataflow->flavour;
   DataflowType    type    = dataflow->type;

   for (int i=0; i<dataflow->dependencies_count; i++) {
      d[i] = glsl_translate_to_backend(stuff, dataflow->d.reloc_deps[i]);
   }

   dataflow_age = dataflow->age;
   switch (flavour) {
      case DATAFLOW_TEXTURE:
         {
            const Dataflow *sampler = relocate_dataflow(stuff, dataflow->d.reloc_deps[3]);
            uint32_t sampler_index = stuff->link_map->uniforms[sampler->u.const_sampler.location];
            uint32_t required_components = dataflow->u.texture.required_components;

            assert(d[0]->type == GLSL_TRANSLATION_VEC4);
            for (int i=1; i<3; i++) assert(d[i] == NULL || d[i]->type == GLSL_TRANSLATION_WORD);
            /* d[3] has type void because it contained the sampler */

#if !V3D_HAS_NEW_TMU_CFG
            if (stuff->v3d_version == V3D_MAKE_VER(3,2))
               tr_texture_gadget(stuff, r, sampler_index, sampler->type, sampler->u.const_sampler.is_32bit,
                                 dataflow->age, required_components, dataflow->u.texture.bits,
                                 d[0]->node[0], d[0]->node[1], d[0]->node[2], d[0]->node[3],
                                 d[1] ? d[1]->node[0] : NULL, d[2] ? d[2]->node[0] : NULL);
            else
#endif
               tr_texture_lookup(stuff, r, sampler_index, sampler->type, sampler->u.const_sampler.is_32bit,
                                 dataflow->age, required_components, dataflow->u.texture.bits,
                                 d[0]->node[0], d[0]->node[1], d[0]->node[2], d[0]->node[3],
                                 d[1] ? d[1]->node[0] : NULL, d[2] ? d[2]->node[0] : NULL);
            break;
         }
      case DATAFLOW_TEXTURE_SIZE:
         {
            const Dataflow *sampler = relocate_dataflow(stuff, dataflow->d.reloc_deps[0]);
            uint32_t sampler_index = stuff->link_map->uniforms[sampler->u.const_sampler.location];
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
         tr_get_col_gadget(stuff, r, dataflow->type, dataflow->u.get_col.required_components,
                                                     dataflow->u.get_col.render_target);
         break;
      case DATAFLOW_ADDRESS_LOAD:
         r->type = get_translation_type(dataflow->type);
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         r->node[0] = tr_indexed_read_gadget(stuff->block, dataflow->age, d[0]->node[0]);
         break;
      case DATAFLOW_VECTOR_LOAD:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         tr_indexed_read_vector_gadget(stuff->block, r, dataflow->age, d[0]->node[0], dataflow->u.vector_load.required_components);
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
         r->type = get_translation_type(dataflow->type);
         int row = dataflow->u.linkable_value.row;
         assert(row < stuff->link_map->num_uniforms);
         r->node[0] = tr_unif(stuff->link_map->uniforms[row]);
         break;
      case DATAFLOW_UNIFORM_BUFFER:
      case DATAFLOW_STORAGE_BUFFER:
         r->type = GLSL_TRANSLATION_VOID;
         break;
      case DATAFLOW_ADDRESS:
         {
            const Dataflow *o = relocate_dataflow(stuff, dataflow->d.reloc_deps[0]);
            r->type = GLSL_TRANSLATION_WORD;

            int id = o->u.linkable_value.row;
            if(o->flavour==DATAFLOW_UNIFORM) {
               assert(id < stuff->link_map->num_uniforms);
               r->node[0] = tr_uniform_address(stuff->link_map->uniforms[id]);
            } else if(o->flavour==DATAFLOW_UNIFORM_BUFFER) {
               assert(id < stuff->link_map->num_uniforms);
               r->node[0] = tr_uniform_block_address(stuff->link_map->uniforms[id]);
            } else if (o->flavour == DATAFLOW_STORAGE_BUFFER) {
               assert(id < stuff->link_map->num_buffers);
               r->node[0] = tr_typed_uniform(BACKEND_UNIFORM_SSBO_ADDRESS, stuff->link_map->buffers[id]);
            } else
               UNREACHABLE();
         }
         break;

      case DATAFLOW_IN:
         r->type = GLSL_TRANSLATION_WORD;
         int id = dataflow->u.linkable_value.row;
         assert(id < stuff->link_map->num_ins);
         r->node[0] = stuff->in->inputs[stuff->link_map->ins[id]];
         assert(r->node[0] != NULL);
         break;
      case DATAFLOW_GET_DEPTHRANGE_NEAR:
      case DATAFLOW_GET_DEPTHRANGE_FAR:
      case DATAFLOW_GET_DEPTHRANGE_DIFF:
         {
            const BackendSpecialUniformFlavour row =
               flavour == DATAFLOW_GET_DEPTHRANGE_NEAR ? BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_NEAR :
               flavour == DATAFLOW_GET_DEPTHRANGE_FAR  ? BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_FAR  :
                                                         BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_DIFF;

            r->type = GLSL_TRANSLATION_WORD;
            r->node[0] = tr_special_uniform(row);
         }
         break;
      case DATAFLOW_GET_POINT_COORD_X:
         r->type    = GLSL_TRANSLATION_WORD;
         r->node[0] = stuff->in->point_x;
         break;
      case DATAFLOW_GET_POINT_COORD_Y:
         r->type    = GLSL_TRANSLATION_WORD;
         r->node[0] = stuff->in->point_y;
         break;
      case DATAFLOW_GET_LINE_COORD:
         r->type    = GLSL_TRANSLATION_WORD;
         r->node[0] = stuff->in->line;
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
      case DATAFLOW_GET_VERTEX_ID:
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = stuff->in->vertexid;
         break;
      case DATAFLOW_GET_INSTANCE_ID:
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = stuff->in->instanceid;
         break;
      case DATAFLOW_FRAG_GET_X:
      case DATAFLOW_FRAG_GET_Y:
         /* This is a workaround for a hardware issue in vc5:
            the floating point pixel coordinates returned match the integer
            pixel coordinates, i.e. the bottom left of the pixel. However,
            the GL spec ended up requiring the centre of the pixel be
            returned. Hence the offsetting is necessary. */
         r->type    = GLSL_TRANSLATION_WORD;
         r->node[0] = tr_binop(BACKFLOW_ADD,
                               tr_const(float_to_bits(0.5)),
                               tr_nullary(translate_nullary_flavour(flavour)));
         break;
      case DATAFLOW_FRAG_GET_X_UINT:
      case DATAFLOW_FRAG_GET_Y_UINT:
      case DATAFLOW_GET_THREAD_INDEX:
         r->type    = GLSL_TRANSLATION_WORD;
         r->node[0] = tr_nullary(translate_nullary_flavour(flavour));
         break;
      case DATAFLOW_FRAG_GET_Z:
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = stuff->in->z;
         break;
      case DATAFLOW_FRAG_GET_W:
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = tr_mov_to_reg(REG_MAGIC_RECIP, stuff->in->w);
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
      case DATAFLOW_MUL:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         assert(d[1]->type == GLSL_TRANSLATION_WORD);
         r->type = GLSL_TRANSLATION_WORD;
         BackflowFlavour f = (type == DF_FLOAT) ? BACKFLOW_MUL : BACKFLOW_IMUL32;
         r->node[0] = tr_binop(f, d[0]->node[0], d[1]->node[0]);
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
      case DATAFLOW_ADD:
      case DATAFLOW_SUB:
      case DATAFLOW_SHL:
      case DATAFLOW_SHR:
      case DATAFLOW_MIN:
      case DATAFLOW_MAX:
      case DATAFLOW_FPACK:
      case DATAFLOW_BITWISE_AND:
      case DATAFLOW_BITWISE_OR:
      case DATAFLOW_BITWISE_XOR:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         assert(d[1]->type == GLSL_TRANSLATION_WORD);
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = tr_binop(translate_binop_flavour(flavour, type), d[0]->node[0], d[1]->node[0]);
         break;
      case DATAFLOW_LESS_THAN:
      case DATAFLOW_LESS_THAN_EQUAL:
      case DATAFLOW_GREATER_THAN:
      case DATAFLOW_GREATER_THAN_EQUAL:
      case DATAFLOW_EQUAL:
      case DATAFLOW_NOT_EQUAL:
         type = relocate_dataflow(stuff, dataflow->d.reloc_deps[0])->type;
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
      case DATAFLOW_RSQRT:
      case DATAFLOW_RCP:
      case DATAFLOW_LOG2:
      case DATAFLOW_EXP2:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         r->type = GLSL_TRANSLATION_WORD;
         r->node[0] = tr_mov_to_reg(get_magic_reg(dataflow->flavour), d[0]->node[0]);
         break;
      case DATAFLOW_ABS:
      case DATAFLOW_TRUNC:
      case DATAFLOW_NEAREST:
      case DATAFLOW_CEIL:
      case DATAFLOW_FLOOR:
      case DATAFLOW_FTOI_TRUNC:
      case DATAFLOW_FTOI_NEAREST:
      case DATAFLOW_FTOU:
      case DATAFLOW_ITOF:
      case DATAFLOW_CLZ:
      case DATAFLOW_UTOF:
      case DATAFLOW_BITWISE_NOT:
      case DATAFLOW_ARITH_NEGATE:
      case DATAFLOW_FUNPACKA:
      case DATAFLOW_FUNPACKB:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         r->type = d[0]->type;
         r->node[0] = tr_unary_op(translate_unary_flavour(flavour, type), d[0]->node[0]);
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
      case DATAFLOW_FDX:
      case DATAFLOW_FDY:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         r->type = d[0]->type;

         /* TODO: Make this work for the new TMU */
#if !V3D_HAS_NEW_TMU_CFG
         set_tmu_dependencies_to_per_quad(d[0]->node[0]->tmu_deps);
#endif

         r->node[0] = tr_unary_op(translate_unary_flavour(flavour, type), d[0]->node[0]);
         break;
      case DATAFLOW_REINTERP:
         assert(d[0]->type == GLSL_TRANSLATION_WORD);
         r->type = d[0]->type;
         r->node[0] = d[0]->node[0];
         break;
      case DATAFLOW_LOGICAL_NOT:
         tr_logical_not(r, d[0]);
         break;
      case DATAFLOW_CONST_SAMPLER:
         r->type = GLSL_TRANSLATION_VOID;
         r->node[0] = NULL;
         break;
      case DATAFLOW_PHI:
         r->type = d[0]->type;
         r->node[0] = d[0]->node[0];
         push_phi(dataflow, &stuff->block->phi_list);
         break;
      case DATAFLOW_EXTERNAL:
         r->type = (type == DF_BOOL) ? GLSL_TRANSLATION_BOOL_FLAG : GLSL_TRANSLATION_WORD;
         r->node[0] = tr_external(dataflow->u.external.block, dataflow->u.external.output, &stuff->block->external_list);
         break;
      default:
         UNREACHABLE();
         break;
   }

   r->per_sample = (flavour == DATAFLOW_FRAG_GET_COL);
   for (int i = 0; i < dataflow->dependencies_count; i++) {
      if (dataflow->d.reloc_deps[i] != -1) {
         const Dataflow *dependency = relocate_dataflow(stuff, dataflow->d.reloc_deps[i]);
         GLSL_TRANSLATION_T* d = &stuff->translations[dependency->id];
         r->per_sample |= d->per_sample;
      }
   }
   if (r->per_sample) {
      GLSL_TRANSLATION_LIST_T *node = malloc_fast(sizeof(GLSL_TRANSLATION_LIST_T));
      node->value = r;
      node->next = stuff->per_sample_clear_list;
      stuff->per_sample_clear_list = node;
   }

   return r;
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
                                     const GLXX_LINK_RESULT_KEY_T *key,
                                     int v3d_version)
{
   ctx->v3d_version = v3d_version;

   ctx->df_arr = arr;

   ctx->translations = malloc_fast(sizeof(GLSL_TRANSLATION_T) * dataflow_count);
   ctx->translations_count = dataflow_count;
   memset(ctx->translations, 0, sizeof(GLSL_TRANSLATION_T) * dataflow_count);
   ctx->per_sample_clear_list = NULL;

   ctx->link_map = link_map;

   ctx->in = ins;
   ctx->block = block;

   for (int i = 0; i < GLXX_CONFIG_MAX_TEXTURE_UNITS; i++)
      ctx->gadgettype[i] = key->gadgettype[i];

   /* Stuff used to do framebuffer fetch on multisample targets */
   ctx->sample_num = 0;
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

SchedBlock *translate_block(const CFGBlock *b_in, const LinkMap *link_map,
                            const bool *output_active, SchedShaderInputs *ins,
                            const GLXX_LINK_RESULT_KEY_T *key, int v3d_version)
{
   SchedBlock *ret = malloc_fast(sizeof(SchedBlock));
   init_sched_block(ret);

   GLSL_TRANSLATE_CONTEXT_T ctx;
   int max_samples = (key->backend & GLXX_SAMPLE_MS) ? 4 : 1;
   /* TODO: Writing nodes_out in this order is odd and unnecessary */
   Backflow **nodes_out = glsl_safemem_calloc(4 * b_in->num_outputs, sizeof(Backflow *));

   ret->branch_cond = b_in->successor_condition;
   init_translation_context(&ctx, b_in->dataflow, b_in->num_dataflow, link_map, ins, ret, key, v3d_version);

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

      for (int i = 0; i < b_in->num_outputs; i++)
      {
         if (!output_active[i]) continue;

         GLSL_TRANSLATION_T *tr = glsl_translate_to_backend(&ctx, b_in->outputs[i]);
         if (tr != NULL)
         {
            assert(tr->type == GLSL_TRANSLATION_WORD ||
                   tr->type == GLSL_TRANSLATION_BOOL_FLAG_N ||
                   tr->type == GLSL_TRANSLATION_BOOL_FLAG     );

            if (tr->type == GLSL_TRANSLATION_BOOL_FLAG_N) {
               /* Convert the rep to FLAG for output from the block */
               if (is_const(tr->node[0]))
                  nodes_out[4*i + ctx.sample_num] = tr->node[0]->unif ? tr_const(0) : tr_const(1);
               else
                  nodes_out[4*i + ctx.sample_num] = tr_cond(tr->node[0], tr_const(0), tr_const(1), tr->type);
            } else
               nodes_out[4*i + ctx.sample_num] = tr->node[0];
         }
      }
      ctx.sample_num++;
   } while (ctx.per_sample_clear_list && ctx.sample_num < max_samples);

   if (ctx.per_sample_clear_list != NULL && max_samples > 1) ret->per_sample = true;

   int samples_out = ret->per_sample ? max_samples : 1;
   ret->num_outputs = b_in->num_outputs;
   ret->outputs = malloc_fast(samples_out * b_in->num_outputs * sizeof(Backflow *));
   for (int i=0; i<ret->num_outputs; i++) {
      for (int j=0; j<samples_out; j++) {
         ret->outputs[samples_out*i + j] = nodes_out[4*i + j];
      }
   }

   resolve_tlb_loads(ret, max_samples > 1);

   if (ins != NULL && ins->read_dep != NULL)
      glsl_backflow_chain_append(&ret->iodeps, ins->read_dep);

   glsl_safemem_free(nodes_out);

   return ret;
}

void collect_shader_outputs(SchedBlock *block, int block_id, const IRShader *sh,
                            const LinkMap *link_map, Backflow **nodes, bool out_per_sample)
{
   for (int i=0; i<link_map->num_outs; i++) {
      if (link_map->outs[i] == -1) continue;

      int samples = block->per_sample ? 4 : 1;
      int out_samples = out_per_sample ? 4 : 1;
      IROutput *o = &sh->outputs[link_map->outs[i]];
      if (o->block == block_id) {
         for (int j=0; j<samples; j++) {
            nodes[out_samples*i+j] = block->outputs[samples*o->output + j];
         }
      } else {
         nodes[out_samples*i] = tr_external(o->block, o->output, &block->external_list);
      }
   }
}

void glsl_fragment_translate(
   SchedBlock *block,
   int block_id,
   const IRShader *sh,
   const LinkMap *link_map,
   const GLXX_LINK_RESULT_KEY_T *key,
   bool *does_discard_out,
   bool *does_z_change_out)
{
   /* If block->per_sample the outputs are at 4*id + sample_num */
   Backflow *bnodes[4*DF_BLOB_FRAGMENT_COUNT] = { 0, };
   collect_shader_outputs(block, block_id, sh, link_map, bnodes, true);

   block->num_outputs = 0;
   block->outputs = NULL;

   GLSL_FRAGMENT_BACKEND_STATE_T s;
   fragment_backend_state_init_gl(&s, key->backend);
   Backflow *res = fragment_backend(&s,
                                    bnodes + DF_FNODE_R(0),
                                    bnodes[4*DF_FNODE_DISCARD],
                                    bnodes[4*DF_FNODE_DEPTH],
                                    block,
                                    does_discard_out, does_z_change_out);
   if (res != NULL)
      glsl_backflow_chain_append(&block->iodeps, res);
}

static uint32_t get_reads_total(const ATTRIBS_USED_T *attr)
{
   uint32_t reads_total = 0;

   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      assert(attr->scalars_used[i] <= 4);
      reads_total += attr->scalars_used[i];
   }
   if (attr->vertexid_used) reads_total++;
   if (attr->instanceid_used) reads_total++;

   return reads_total;
}

void vertex_shader_inputs(SchedShaderInputs *ins, const ATTRIBS_USED_T *attr_info, uint32_t *reads_total) {
   /* Ensure all unused inputs are NULL */
   memset(ins, 0, sizeof(SchedShaderInputs));

   *reads_total = get_reads_total(attr_info);
   ins->read_dep = fetch_all_attribs(ins, attr_info, *reads_total);
}

void glsl_vertex_translate(
   SchedBlock *block,
   int block_id,
   const IRShader *sh,
   const LinkMap *link_map,
   SchedShaderInputs *ins,
   const GLXX_LINK_RESULT_KEY_T *key,
   glsl_binary_shader_flavour_t shader_flavour,
   const GLSL_VARY_MAP_T *vary_map)
{
   const bool points = (key->backend & GLXX_PRIM_M) == GLXX_PRIM_POINT;

   assert(shader_flavour == BINARY_SHADER_VERTEX || shader_flavour == BINARY_SHADER_COORDINATE);
   assert(vary_map != NULL);
   assert(!block->per_sample);

   Backflow *bnodes[DF_BLOB_VERTEX_COUNT] = { 0, };
   collect_shader_outputs(block, block_id, sh, link_map, bnodes, false);

   if (ins != NULL)
      block->last_vpm_read = ins->read_dep;

   if (points && bnodes[DF_VNODE_POINT_SIZE] == NULL)
      bnodes[DF_VNODE_POINT_SIZE] = tr_const(0);
   if (!points && bnodes[DF_VNODE_POINT_SIZE] != NULL)
      bnodes[DF_VNODE_POINT_SIZE] = NULL;

   block->num_outputs = 0;
   block->outputs = NULL;

   bool is_cshader   = shader_flavour == BINARY_SHADER_COORDINATE;
   bool z_only_write = !!(key->backend & GLXX_Z_ONLY_WRITE) && is_cshader;
   bool write_clip_header = is_cshader;

   vertex_backend(block, ins,
                  bnodes[DF_VNODE_X], bnodes[DF_VNODE_Y],
                  bnodes[DF_VNODE_Z], bnodes[DF_VNODE_W],
                  bnodes[DF_VNODE_POINT_SIZE],
                  write_clip_header, z_only_write,
                  bnodes+DF_VNODE_VARY(0), vary_map);
}
