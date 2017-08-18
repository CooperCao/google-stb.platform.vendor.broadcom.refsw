/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "middleware/khronos/glsl/glsl_common.h"

#include <stdlib.h>
#include <assert.h>

#include "middleware/khronos/glsl/glsl_dataflow.h"
#include "middleware/khronos/glsl/glsl_trace.h"
#include "middleware/khronos/glsl/glsl_ast_visitor.h"
#include "middleware/khronos/glsl/glsl_errors.h"
#include "middleware/khronos/glsl/glsl_fastmem.h"
#include "middleware/khronos/glsl/glsl_stack.h"

#include "middleware/khronos/glsl/glsl_const_operators.h"

#include "interface/khronos/include/GLES2/gl2ext.h"
#include "interface/khronos/common/khrn_options.h"

#include "middleware/khronos/glsl/glsl_backend.h"

static Dataflow *boolify(Dataflow *dataflow, BOOL_REP_T *rep_out);
static bool dataflow_is_bool(Dataflow *dataflow);
static void set_bool_rep(Dataflow *dataflow, BOOL_REP_T bool_rep);

// Prototype for the function that brings it all together, so that the individual functions can recurse.
static void expr_calculate_dataflow(Dataflow** scalar_values, Expr* expr);
static Statement* sprev_calculate_dataflow(Statement* statement, void* data);

static Expr *calling_function_stack [100] = { NULL };
static unsigned calling_function_stack_top = 0;

#ifdef BUILD_FOR_DAVES_TEST
   /* Dave's test verifies that results are bit accurate, including multiplying by constant zeros and
      expecting the sign of such to be preserved.  This means that some optimisations have to
     be turned off.  They must not be turned off in normal usage as they provide
     a very valuable gain */
#   define DATAFLOW_DONT_OPTIMISE_MUL_CONST_ZERO
#endif

#ifdef DATAFLOW_LOG
static int dataflow_log_allocs[256];

void glsl_dataflow_log_init()
{
   int i;

   for (i = 0; i < 256; i++)
      dataflow_log_allocs[i] = 0;
}

void glsl_dataflow_log_dump()
{
   int i;

   for (i = 0; i < 256; i++)
      if (dataflow_log_allocs[i])
         printf("%d, %d\n", i, dataflow_log_allocs[i]);
}

#define DATAFLOW_LOG_ALLOC(x) do { dataflow_log_allocs[x] += sizeof(Dataflow); } while(0)
#else
#define DATAFLOW_LOG_ALLOC(x)
#endif

//
// Scalar value stack.
//

#define STACK_FRAME_DEFAULT_SIZE   128

typedef struct STACK_FRAME {
   Dataflow **scalar_values;

   struct STACK_FRAME *next;
} STACK_FRAME_T;

static STACK_FRAME_T *free_frames;
static STACK_FRAME_T *used_frames;

void glsl_init_dataflow_stack()
{
   free_frames = NULL;
   used_frames = NULL;
}

static Dataflow **stack_alloc_by_size(int size)
{
   STACK_FRAME_T *frame;

   if (free_frames == NULL || size > STACK_FRAME_DEFAULT_SIZE) {
      const int allocated_size = size > STACK_FRAME_DEFAULT_SIZE ? size : STACK_FRAME_DEFAULT_SIZE;
      frame                = malloc_fast(sizeof(*frame));
      frame->scalar_values = malloc_fast(sizeof(*frame->scalar_values) * allocated_size);
   } else {
      frame       = free_frames;
      free_frames = free_frames->next;
   }

   frame->next = used_frames;
   used_frames = frame;

   return frame->scalar_values;
}

static Dataflow **stack_alloc_by_type(SymbolType *type)
{
   return stack_alloc_by_size(type->scalar_count);
}

static void stack_free(void)
{
   STACK_FRAME_T *frame = used_frames;

   vcos_assert(frame);

   used_frames = frame->next;

   frame->next = free_frames;
   free_frames = frame;
}

//
// Dataflow chain functions.
//

void glsl_dataflow_chain_init(DataflowChain* chain)
{
   chain->first = NULL;
   chain->last = NULL;
   chain->count = 0;
}

DataflowChain* glsl_dataflow_chain_append(DataflowChain* chain, Dataflow* dataflow)
{
   DataflowChainNode* node = (DataflowChainNode *)malloc_fast(sizeof(DataflowChainNode));

   node->dataflow = dataflow;

   node->unlinked = false;
   node->prev = chain->last;
   node->next = NULL;

   if (!chain->first) chain->first = node;
   if (chain->last) chain->last->next = node;
   chain->last = node;

   chain->count++;

   return chain;
}

DataflowChain* glsl_dataflow_chain_remove_node(DataflowChain* chain, DataflowChainNode* node)
{
   // Update nodes.
   if (node->prev) node->prev->next = node->next;
   if (node->next) node->next->prev = node->prev;

   // Update chain.
   chain->count--;
   if (chain->first == node) chain->first = node->next;
   if (chain->last == node) chain->last = node->prev;

   if (chain->count == 0) {
      vcos_assert(!chain->first);
      vcos_assert(!chain->last);
   }

   // Update node.
   node->unlinked = true;

   return chain;
}

DataflowChain* glsl_dataflow_chain_remove(DataflowChain *chain, Dataflow *dataflow)
{
   DataflowChainNode *node;

   for (node = chain->first; node; node = node->next)
      if (node->dataflow == dataflow)
         glsl_dataflow_chain_remove_node(chain, node);

   return chain;
}

DataflowChain* glsl_dataflow_chain_replace(DataflowChain *chain, Dataflow *dataflow_old, Dataflow *dataflow_new)
{
   DataflowChainNode *node;

   for (node = chain->first; node; node = node->next)
      if (node->dataflow == dataflow_old)
         node->dataflow = dataflow_new;

   return chain;
}

DataflowChain *glsl_dataflow_chain_filter(DataflowChain *dst, DataflowChain *src, void *data, DataflowFilter filter)
{
   DataflowChainNode *node;

   for (node = src->first; node; node = node->next) {
      Dataflow *dataflow = node->dataflow;

      if (filter(dataflow, data))
         glsl_dataflow_chain_append(dst, dataflow);
   }

   return dst;
}

bool glsl_dataflow_chain_contains(DataflowChain *chain, Dataflow *dataflow)
{
   DataflowChainNode *node;

   for (node = chain->first; node; node = node->next)
      if (node->dataflow == dataflow)
         return true;

   return false;
}

//
// Utility function to initialise scheduler-specific fields
//

static void init_backend_fields(Dataflow *dataflow)
{
   dataflow->delay = 0;

   {
      int i;
      for (i=0; i<TMU_DEP_WORD_COUNT; i++) dataflow->tmu_dependencies[i] = 0;
      dataflow->copy = NULL;
   }

   dataflow->phase = -1;

   dataflow->slot = -1;

   dataflow->bcg_helper = NULL;
}

/* in general, sprev_calculate_dataflow and expr_calculate_dataflow(_*) should
 * update dataflow_line_num after recursing and before constructing any dataflow
 * nodes for itself. */
static int dataflow_line_num = LINE_NUMBER_UNDEFINED;

static Dataflow *dataflow_construct_common(DataflowFlavour flavour)
{
   Dataflow* dataflow;

   dataflow = (Dataflow *)malloc_fast(sizeof(Dataflow));
   DATAFLOW_LOG_ALLOC(flavour)
   dataflow->line_num = dataflow_line_num;
   dataflow->pass = DATAFLOW_PASS_INIT;
   dataflow->flavour = flavour;
   glsl_dataflow_clear_chains(dataflow);
   init_backend_fields(dataflow);
   set_bool_rep(dataflow, BOOL_REP_NONE); // boolean nodes should overwrite this.
   return dataflow;
}

//
// Dataflow constructors.
//

Dataflow* glsl_dataflow_construct_const_bool(const_bool value)
{
   Dataflow* dataflow;

   dataflow = dataflow_construct_common(DATAFLOW_CONST_BOOL);

   dataflow->u.const_bool.value = value;

	dataflow->dependencies_count = 0;

   set_bool_rep(dataflow, BOOL_REP_BOOL);

   return dataflow;
}

Dataflow* glsl_dataflow_construct_const_int(const_int value)
{
   Dataflow* dataflow;

   dataflow = dataflow_construct_common(DATAFLOW_CONST_INT);

   dataflow->u.const_int.value = value;
   dataflow->u.const_int.index = ~0;

	dataflow->dependencies_count = 0;

   return dataflow;
}

Dataflow* glsl_dataflow_construct_const_float(const_float value)
{
   Dataflow* dataflow;

   dataflow = dataflow_construct_common(DATAFLOW_CONST_FLOAT);

   dataflow->u.const_float.value = value;
   dataflow->u.const_float.index = ~0;

	dataflow->dependencies_count = 0;

   return dataflow;
}

Dataflow* glsl_dataflow_construct_const_sampler(void)
{
   Dataflow* dataflow;

   dataflow = dataflow_construct_common(DATAFLOW_CONST_SAMPLER);

   dataflow->u.const_sampler.location = SAMPLER_LOCATION_UNDEFINED;
   dataflow->u.const_sampler.name = SAMPLER_NAME_UNDEFINED;

	dataflow->dependencies_count = 0;

   return dataflow;
}

Dataflow* glsl_dataflow_construct_linkable_value(DataflowFlavour flavour, PrimitiveTypeIndex type_index)
{
   Dataflow* dataflow;

   dataflow = dataflow_construct_common(flavour);

   dataflow->u.linkable_value.row = LINKABLE_VALUE_ROW_UNDEFINED;
   dataflow->u.linkable_value.name = LINKABLE_VALUE_NAME_UNDEFINED;

   dataflow->dependencies_count = 0;

   if (type_index == PRIM_BOOL)
      set_bool_rep(dataflow, BOOL_REP_BOOL);

   return dataflow;
}

#define COORD_DIM    3
#define COORD_S      0
#define COORD_T      1
#define COORD_R      2

enum
{
   TEXTURE_2D,
   TEXTURE_CUBE
} texture_type;

enum
{
   LOOKUP_BIAS,
   LOOKUP_LOD
} lookup_type;

/* Next available sampler slot */
static int next_sampler_index_offsets;
/* number of samplers per shader */
static unsigned int num_samplers[SHADER_FLAVOUR_COUNT];

Dataflow* glsl_dataflow_construct_linkable_value_offset(DataflowFlavour flavour, PrimitiveTypeIndex type_index, Dataflow* linkable_value, Dataflow* offset)
{
   Dataflow *pseudo_sampler;
   Dataflow *gadget_set_t;
   Dataflow *gadget_set_s;
   Dataflow *gadget_get_rgba;

   UNUSED_NDEBUG(flavour);
   UNUSED(type_index);

   vcos_assert(DATAFLOW_UNIFORM_OFFSET == flavour); // only uniforms can be accessed dynamically
   UNUSED_NDEBUG(linkable_value);
   vcos_assert(linkable_value == NULL);
   pseudo_sampler = NULL;
   gadget_set_t = NULL;

   gadget_set_s = glsl_dataflow_construct_texture_lookup_set(
      DATAFLOW_TEX_SET_DIRECT,
      pseudo_sampler,
      offset,
      NULL,
      gadget_set_t,
      NULL);

   gadget_get_rgba = glsl_dataflow_construct_texture_lookup_get(
      DATAFLOW_TEX_GET_CMP_R,
      pseudo_sampler,
      gadget_set_s,
      NULL,
      NULL);

   return gadget_get_rgba;
}

Dataflow* glsl_dataflow_construct_unary_op(DataflowFlavour flavour, Dataflow* operand)
{
   Dataflow* dataflow;

   if (operand->flavour == DATAFLOW_CONST_FLOAT) {
      // constant folding for floats

      const_int i;
      const_float f;

      switch (flavour) {
      case DATAFLOW_ARITH_NEGATE:
      {
         op_arith_negate__const_float__const_float(&f, &operand->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }
      case DATAFLOW_FTOI_TRUNC:
      {
         op_floattoint_trunc__const_int__const_float(&i, &operand->u.const_float.value);

         return glsl_dataflow_construct_const_int(i);
      }
      case DATAFLOW_FTOI_NEAREST:
      {
         op_floattoint_nearest__const_int__const_float(&i, &operand->u.const_float.value);

         return glsl_dataflow_construct_const_int(i);
      }
      case DATAFLOW_INTRINSIC_RSQRT:
      {
         op_rsqrt__const_float__const_float(&f, &operand->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }
      case DATAFLOW_INTRINSIC_RCP:
      {
         op_recip__const_float__const_float(&f, &operand->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }
      case DATAFLOW_INTRINSIC_LOG2:
      {
         op_log2__const_float__const_float(&f, &operand->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }
      case DATAFLOW_INTRINSIC_EXP2:
      {
         op_exp2__const_float__const_float(&f, &operand->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }
      case DATAFLOW_INTRINSIC_CEIL:
      {
         op_ceil__const_float__const_float(&f, &operand->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }
      case DATAFLOW_INTRINSIC_FLOOR:
      {
         op_floor__const_float__const_float(&f, &operand->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }
      case DATAFLOW_INTRINSIC_SIGN:
      {
         const_float z = CONST_FLOAT_ZERO;

         op_cmp__const_int__const_float__const_float(&i, &operand->u.const_float.value, &z);

         return glsl_dataflow_construct_const_float(const_float_from_int(i));
      }
      case DATAFLOW_MOV:         // a mov is there for a reason; don't fold it
         break;
      default:
         UNREACHABLE();
         break;
      }
   }

   if (operand->flavour == DATAFLOW_CONST_INT) {
      // constant folding for floats

      const_int i;

      switch (flavour) {
      case DATAFLOW_ARITH_NEGATE:
      {
         op_arith_negate__const_int__const_int(&i, &operand->u.const_int.value);

         return glsl_dataflow_construct_const_int(i);
      }
      case DATAFLOW_INTRINSIC_RCP:     // a recip is there because we're doing integer division
      case DATAFLOW_MOV:               // a mov is there for a reason; don't fold it
         break;
      case DATAFLOW_ITOF:
         return operand;
      case DATAFLOW_FTOI_TRUNC:
      case DATAFLOW_FTOI_NEAREST:
      {
         Dataflow *value = glsl_dataflow_construct_const_float(operand->u.const_int.value);
         return value;
      }
      case DATAFLOW_BITWISE_NOT:
         break;
      default:
         UNREACHABLE();
         break;
      }
   }

   if (operand->flavour == DATAFLOW_CONST_BOOL) {
      // constant folding for floats

      const_bool b;

      switch (flavour) {
      case DATAFLOW_LOGICAL_NOT:
      {
         op_logical_not__const_bool__const_bool(&b, &operand->u.const_bool.value);

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_MOV:         // a mov is there for a reason; don't fold it
         break;
      default:
         UNREACHABLE();
         break;
      }
   }

   if (flavour == DATAFLOW_LOGICAL_NOT) {
      /*
         negation of relational ops
      */

      switch (operand->flavour)
      {
#if 0
         case DATAFLOW_EQUAL:
            return glsl_dataflow_construct_binary_op(DATAFLOW_NOT_EQUAL, operand->u.binary_op.left, operand->u.binary_op.right);
         case DATAFLOW_NOT_EQUAL:
            return glsl_dataflow_construct_binary_op(DATAFLOW_EQUAL, operand->u.binary_op.left, operand->u.binary_op.right);
         case DATAFLOW_LESS_THAN:
            return glsl_dataflow_construct_binary_op(DATAFLOW_GREATER_THAN_EQUAL, operand->u.binary_op.left, operand->u.binary_op.right);
         case DATAFLOW_LESS_THAN_EQUAL:
            return glsl_dataflow_construct_binary_op(DATAFLOW_GREATER_THAN, operand->u.binary_op.left, operand->u.binary_op.right);
         case DATAFLOW_GREATER_THAN:
            return glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN_EQUAL, operand->u.binary_op.left, operand->u.binary_op.right);
         case DATAFLOW_GREATER_THAN_EQUAL:
            return glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN, operand->u.binary_op.left, operand->u.binary_op.right);
#endif
         case DATAFLOW_LOGICAL_NOT:
            return operand->d.unary_op.operand;
         default:
            break;
      }
   }

   /* No negate instruction */
   if (flavour == DATAFLOW_ARITH_NEGATE)
   {
      return glsl_dataflow_construct_binary_op(DATAFLOW_SUB, glsl_dataflow_construct_const_float(CONST_FLOAT_ZERO), operand);
   }
   /* No floor instruction */
   if (flavour == DATAFLOW_INTRINSIC_FLOOR)
   {
      /* TODO: incorrect for negative input */
      Dataflow *value = glsl_dataflow_construct_unary_op(DATAFLOW_ITOF, glsl_dataflow_construct_unary_op(DATAFLOW_FTOI_TRUNC, operand));
      value = glsl_dataflow_construct_cond_op(glsl_dataflow_construct_binary_op(DATAFLOW_GREATER_THAN, value, operand),
                                              glsl_dataflow_construct_binary_op(DATAFLOW_SUB, value, glsl_dataflow_construct_const_int(1)),
                                              value);
      return value;
   }

   switch (flavour)
   {
//      case DATAFLOW_ARITH_NEGATE: /* These 2 cases handled above */
//      case DATAFLOW_INTRINSIC_FLOOR:
      case DATAFLOW_LOGICAL_NOT:
      case DATAFLOW_FTOI_TRUNC:
      case DATAFLOW_FTOI_NEAREST:
      case DATAFLOW_INTRINSIC_RSQRT:
      case DATAFLOW_INTRINSIC_RCP:
      case DATAFLOW_INTRINSIC_LOG2:
      case DATAFLOW_INTRINSIC_EXP2:
      case DATAFLOW_INTRINSIC_CEIL:
      case DATAFLOW_INTRINSIC_SIGN:
      case DATAFLOW_MOV:
      case DATAFLOW_ITOF:
      case DATAFLOW_BITWISE_NOT:
         break;

      default:
         UNREACHABLE();
         return NULL;
   }

   dataflow = dataflow_construct_common(flavour);

   // Add this node to the lists of dependents of the nodes it depends on.
   glsl_dataflow_add_dependent(operand, dataflow);

   dataflow->dependencies_count = 1;

	dataflow->d.unary_op.operand = operand;

   if (flavour == DATAFLOW_LOGICAL_NOT)
   {
      BOOL_REP_T rep = BOOL_REP_NONE;

      switch (glsl_dataflow_get_bool_rep(operand))
      {
      case BOOL_REP_BOOL:   rep = BOOL_REP_BOOL_N; break;
      case BOOL_REP_BOOL_N: rep = BOOL_REP_BOOL;   break;
      case BOOL_REP_NEG:    rep = BOOL_REP_NEG_N;  break;
      case BOOL_REP_NEG_N:  rep = BOOL_REP_NEG;    break;
      case BOOL_REP_ZERO:   rep = BOOL_REP_ZERO_N; break;
      case BOOL_REP_ZERO_N: rep = BOOL_REP_ZERO;   break;
      default:
         UNREACHABLE();
      }

      set_bool_rep(dataflow, rep);
   }
   if (flavour == DATAFLOW_MOV && dataflow_is_bool(operand))
      set_bool_rep(dataflow, glsl_dataflow_get_bool_rep(operand));

   return dataflow;
}

Dataflow* glsl_dataflow_construct_binary_op(DataflowFlavour flavour, Dataflow* left, Dataflow* right)
{
   Dataflow* dataflow;
   BOOL_REP_T rep = BOOL_REP_NONE;

   vcos_assert(left);
   vcos_assert(right);

   if (left->flavour == DATAFLOW_CONST_FLOAT && right->flavour == DATAFLOW_CONST_FLOAT) {
      // constant folding for floats

      const_float f;
      const_bool b;

      switch (flavour) {
      case DATAFLOW_MUL:
      {
         op_mul__const_float__const_float__const_float(&f, &left->u.const_float.value, &right->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }
      case DATAFLOW_ADD:
      {
         op_add__const_float__const_float__const_float(&f, &left->u.const_float.value, &right->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }
      case DATAFLOW_SUB:
      {
         op_sub__const_float__const_float__const_float(&f, &left->u.const_float.value, &right->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }
      case DATAFLOW_RSUB:
      {
         op_sub__const_float__const_float__const_float(&f, &right->u.const_float.value, &left->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }
      case DATAFLOW_LESS_THAN:
      {
         op_less_than__const_bool__const_float__const_float(&b, &left->u.const_float.value, &right->u.const_float.value);

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_LESS_THAN_EQUAL:
      {
         op_less_than_equal__const_bool__const_float__const_float(&b, &left->u.const_float.value, &right->u.const_float.value);

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_GREATER_THAN:
      {
         op_greater_than__const_bool__const_float__const_float(&b, &left->u.const_float.value, &right->u.const_float.value);

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_GREATER_THAN_EQUAL:
      {
         op_greater_than_equal__const_bool__const_float__const_float(&b, &left->u.const_float.value, &right->u.const_float.value);

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_EQUAL:
      {
         b = left->u.const_float.value == right->u.const_float.value;

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_NOT_EQUAL:
      {
         b = left->u.const_float.value != right->u.const_float.value;

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_INTRINSIC_MIN:
      {
         op_min__const_float__const_float__const_float(&f, &left->u.const_float.value, &right->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }
      case DATAFLOW_INTRINSIC_MAX:
      {
         op_max__const_float__const_float__const_float(&f, &left->u.const_float.value, &right->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }
      case DATAFLOW_INTRINSIC_MINABS:
      {
         op_minabs__const_float__const_float__const_float(&f, &left->u.const_float.value, &right->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }
      case DATAFLOW_INTRINSIC_MAXABS:
      {
         op_maxabs__const_float__const_float__const_float(&f, &left->u.const_float.value, &right->u.const_float.value);

         return glsl_dataflow_construct_const_float(f);
      }

      case DATAFLOW_BITWISE_AND:
      case DATAFLOW_BITWISE_OR:
      case DATAFLOW_BITWISE_XOR:
      case DATAFLOW_V8MULD:
      case DATAFLOW_V8MIN:
      case DATAFLOW_V8MAX:
      case DATAFLOW_V8ADDS:
      case DATAFLOW_V8SUBS:
      case DATAFLOW_INTEGER_ADD:
         /* TODO: ? */
         break;
      default:
         UNREACHABLE();
         break;
      }
   }

   if (left->flavour == DATAFLOW_CONST_INT && right->flavour == DATAFLOW_CONST_INT) {
      // constant folding for ints

      const_int i;
      const_bool b;

      switch (flavour) {
      case DATAFLOW_MUL:
      {
         op_mul__const_int__const_int__const_int(&i, &left->u.const_int.value, &right->u.const_int.value);

         return glsl_dataflow_construct_const_int(i);
      }
      case DATAFLOW_ADD:
      {
         op_add__const_int__const_int__const_int(&i, &left->u.const_int.value, &right->u.const_int.value);

         return glsl_dataflow_construct_const_int(i);
      }
      case DATAFLOW_SUB:
      {
         op_sub__const_int__const_int__const_int(&i, &left->u.const_int.value, &right->u.const_int.value);

         return glsl_dataflow_construct_const_int(i);
      }
      case DATAFLOW_RSUB:
      {
         op_sub__const_int__const_int__const_int(&i, &right->u.const_int.value, &left->u.const_int.value);

         return glsl_dataflow_construct_const_int(i);
      }
      case DATAFLOW_LESS_THAN:
      {
         op_less_than__const_bool__const_int__const_int(&b, &left->u.const_int.value, &right->u.const_int.value);

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_LESS_THAN_EQUAL:
      {
         op_less_than_equal__const_bool__const_int__const_int(&b, &left->u.const_int.value, &right->u.const_int.value);

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_GREATER_THAN:
      {
         op_greater_than__const_bool__const_int__const_int(&b, &left->u.const_int.value, &right->u.const_int.value);

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_GREATER_THAN_EQUAL:
      {
         op_greater_than_equal__const_bool__const_int__const_int(&b, &left->u.const_int.value, &right->u.const_int.value);

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_EQUAL:
      {
         b = left->u.const_int.value == right->u.const_int.value;

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_NOT_EQUAL:
      {
         b = left->u.const_int.value != right->u.const_int.value;

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_INTRINSIC_MIN:
      {
         op_min__const_int__const_int__const_int(&i, &left->u.const_int.value, &right->u.const_int.value);

         return glsl_dataflow_construct_const_int(i);
      }
      case DATAFLOW_INTRINSIC_MAX:
      {
         op_max__const_int__const_int__const_int(&i, &left->u.const_int.value, &right->u.const_int.value);

         return glsl_dataflow_construct_const_int(i);
      }
      case DATAFLOW_BITWISE_AND:
      case DATAFLOW_BITWISE_OR:
      case DATAFLOW_BITWISE_XOR:
      case DATAFLOW_V8MULD:
      case DATAFLOW_V8MIN:
      case DATAFLOW_V8MAX:
      case DATAFLOW_V8ADDS:
      case DATAFLOW_V8SUBS:
      case DATAFLOW_INTEGER_ADD:
         /* TODO: ? */
         break;
      default:
         UNREACHABLE();
         break;
      }
   }

   if (left->flavour == DATAFLOW_CONST_BOOL && right->flavour == DATAFLOW_CONST_BOOL) {
      // constant folding for bools

      const_bool b;

      switch (flavour) {
      case DATAFLOW_EQUAL:
      {
         b = left->u.const_bool.value == right->u.const_bool.value;

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_NOT_EQUAL:
      {
         b = left->u.const_bool.value != right->u.const_bool.value;

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_LOGICAL_AND:
      {
         b = left->u.const_bool.value & right->u.const_bool.value;

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_LOGICAL_XOR:
      {
         b = left->u.const_bool.value ^ right->u.const_bool.value;

         return b ? g_BoolTrue : g_BoolFalse;
      }
      case DATAFLOW_LOGICAL_OR:
      {
         b = left->u.const_bool.value | right->u.const_bool.value;

         return b ? g_BoolTrue : g_BoolFalse;
      }
      default:
         UNREACHABLE();
         break;
      }
   }

   if (left->flavour == DATAFLOW_UNIFORM && right->flavour == DATAFLOW_UNIFORM) {
      // uniform memory is single ported and needs to be accessed via A port

      return glsl_dataflow_construct_binary_op(flavour, left, glsl_dataflow_construct_unary_op(DATAFLOW_MOV, right));
   }

#ifndef DATAFLOW_DONT_OPTIMISE_CONST_ZERO
   if (flavour == DATAFLOW_MUL && left->flavour == DATAFLOW_CONST_FLOAT) {
      if (left->u.const_float.value == CONST_FLOAT_ZERO)
         return left;
      if (left->u.const_float.value == CONST_FLOAT_ONE)
         return right;
   }

   if (flavour == DATAFLOW_MUL && right->flavour == DATAFLOW_CONST_FLOAT) {
      if (right->u.const_float.value == CONST_FLOAT_ZERO)
         return right;
      if (right->u.const_float.value == CONST_FLOAT_ONE)
         return left;
   }

   if (flavour == DATAFLOW_ADD && left->flavour == DATAFLOW_CONST_FLOAT) {
      if (left->u.const_float.value == CONST_FLOAT_ZERO)
         return right;
   }

   if (flavour == DATAFLOW_ADD && right->flavour == DATAFLOW_CONST_FLOAT) {
      if (right->u.const_float.value == CONST_FLOAT_ZERO)
         return left;
   }

   if (flavour == DATAFLOW_INTRINSIC_MINABS && left->flavour == DATAFLOW_CONST_FLOAT) {
      if (left->u.const_float.value == CONST_FLOAT_ZERO)
         return left;
   }

   if (flavour == DATAFLOW_INTRINSIC_MINABS && right->flavour == DATAFLOW_CONST_FLOAT) {
      if (right->u.const_float.value == CONST_FLOAT_ZERO)
         return right;
   }
#endif

   if (flavour == DATAFLOW_LOGICAL_AND && right->flavour == DATAFLOW_CONST_BOOL) {
      if (right->u.const_bool.value)
         return left;
      else
         return right;
   }

   if (flavour == DATAFLOW_LOGICAL_OR && right->flavour == DATAFLOW_CONST_BOOL) {
      if (right->u.const_bool.value)
         return right;
      else
         return left;
   }

   if (flavour == DATAFLOW_LOGICAL_AND && left->flavour == DATAFLOW_CONST_BOOL) {
      if (left->u.const_bool.value)
         return right;
      else
         return left;
   }

   if (flavour == DATAFLOW_LOGICAL_OR && left->flavour == DATAFLOW_CONST_BOOL) {
      if (left->u.const_bool.value)
         return left;
      else
         return right;
   }

   if (flavour == DATAFLOW_V8MULD && right->flavour == DATAFLOW_CONST_INT) {
      if (right->u.const_int.value == 0)
         return right;
      else if (right->u.const_int.value == (int)0xffffffff)
         return left;
   }

   /* Boolean representation stuff */

   switch (flavour)
   {
      case DATAFLOW_LOGICAL_AND:
      case DATAFLOW_LOGICAL_XOR:
      case DATAFLOW_LOGICAL_OR:
      {
         BOOL_REP_T left_rep, right_rep;
         bool l1, r1;
         bool reverse = false;

         left = boolify(left, &left_rep);
         right = boolify(right, &right_rep);

         vcos_assert((left_rep == BOOL_REP_BOOL || left_rep == BOOL_REP_BOOL_N) && (right_rep == BOOL_REP_BOOL || right_rep == BOOL_REP_BOOL_N));
         l1 = left_rep == BOOL_REP_BOOL;
         r1 = right_rep == BOOL_REP_BOOL;
         if (flavour == DATAFLOW_LOGICAL_AND &&  l1 &&  r1) {
            flavour = DATAFLOW_LOGICAL_AND; rep = BOOL_REP_BOOL;                  // and
         } else if (flavour == DATAFLOW_LOGICAL_AND &&  l1 && !r1) {
            flavour = DATAFLOW_LOGICAL_SHR; rep = BOOL_REP_BOOL;                  // shr
         } else if (flavour == DATAFLOW_LOGICAL_AND && !l1 &&  r1) {
            flavour = DATAFLOW_LOGICAL_SHR; rep = BOOL_REP_BOOL; reverse = true;// shr
         } else if (flavour == DATAFLOW_LOGICAL_AND && !l1 && !r1) {
            flavour = DATAFLOW_LOGICAL_OR;  rep = BOOL_REP_BOOL_N;                // nor
         } else if (flavour == DATAFLOW_LOGICAL_XOR &&  l1 &&  r1) {
            flavour = DATAFLOW_LOGICAL_XOR; rep = BOOL_REP_BOOL;                  // xor
         } else if (flavour == DATAFLOW_LOGICAL_XOR &&  l1 && !r1) {
            flavour = DATAFLOW_LOGICAL_XOR; rep = BOOL_REP_BOOL_N;                // nxor
         } else if (flavour == DATAFLOW_LOGICAL_XOR && !l1 &&  r1) {
            flavour = DATAFLOW_LOGICAL_XOR; rep = BOOL_REP_BOOL_N;                // nxor
         } else if (flavour == DATAFLOW_LOGICAL_XOR && !l1 && !r1) {
            flavour = DATAFLOW_LOGICAL_XOR; rep = BOOL_REP_BOOL;                  // xor
         } else if (flavour == DATAFLOW_LOGICAL_OR  &&  l1 &&  r1) {
            flavour = DATAFLOW_LOGICAL_OR;  rep = BOOL_REP_BOOL;                  // or
         } else if (flavour == DATAFLOW_LOGICAL_OR &&  l1 && !r1) {
            flavour = DATAFLOW_LOGICAL_SHR; rep = BOOL_REP_BOOL_N; reverse = true;// nshr
         } else if (flavour == DATAFLOW_LOGICAL_OR && !l1 &&  r1) {
            flavour = DATAFLOW_LOGICAL_SHR; rep = BOOL_REP_BOOL_N;                // nshr
         } else if (flavour == DATAFLOW_LOGICAL_OR && !l1 && !r1) {
            flavour = DATAFLOW_LOGICAL_AND; rep = BOOL_REP_BOOL_N;                // nand
         } else {
            UNREACHABLE();
         }

         if (reverse)
         {
            Dataflow *temp = left;
            left = right;
            right = temp;
         }

         break;
      }

      case DATAFLOW_LESS_THAN:
         rep = BOOL_REP_NEG;
         break;
      case DATAFLOW_LESS_THAN_EQUAL:
         rep = BOOL_REP_NEG_N;
         break;
      case DATAFLOW_GREATER_THAN:
         rep = BOOL_REP_NEG;
         break;
      case DATAFLOW_GREATER_THAN_EQUAL:
         rep = BOOL_REP_NEG_N;
         break;
      case DATAFLOW_EQUAL:
         if (dataflow_is_bool(left) && dataflow_is_bool(right))
            return glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT,
               glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_XOR, left, right));
         rep = BOOL_REP_ZERO;
         break;
      case DATAFLOW_NOT_EQUAL:
         if (dataflow_is_bool(left) && dataflow_is_bool(right))
            return glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_XOR, left, right);
         rep = BOOL_REP_ZERO_N;
         break;
      default:
         break;
   }

   dataflow = dataflow_construct_common(flavour);

   set_bool_rep(dataflow, rep);

   switch (flavour)
   {
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
      case DATAFLOW_INTRINSIC_MIN:
      case DATAFLOW_INTRINSIC_MAX:
      case DATAFLOW_INTRINSIC_MINABS:
      case DATAFLOW_INTRINSIC_MAXABS:
      case DATAFLOW_LOGICAL_SHR:
      case DATAFLOW_SHIFT_RIGHT:
      case DATAFLOW_BITWISE_AND:
      case DATAFLOW_BITWISE_OR:
      case DATAFLOW_BITWISE_XOR:
      case DATAFLOW_V8MULD:
      case DATAFLOW_V8MIN:
      case DATAFLOW_V8MAX:
      case DATAFLOW_V8ADDS:
      case DATAFLOW_V8SUBS:
      case DATAFLOW_INTEGER_ADD:
         break;

      default:
         UNREACHABLE();
         return NULL;
   }

   // Add this node to the lists of dependents of the nodes it depends on.
   glsl_dataflow_add_dependent(left, dataflow);
   glsl_dataflow_add_dependent(right, dataflow);

   dataflow->dependencies_count = 2;

	dataflow->d.binary_op.left = left;
	dataflow->d.binary_op.right = right;

   return dataflow;
}

#if 0
static bool is_complement(Dataflow *c1, Dataflow *c2)
{
   switch (c1->flavour) {
   case DATAFLOW_LESS_THAN:
      return c2->flavour == DATAFLOW_GREATER_THAN_EQUAL && c2->u.binary_op.left == c1->u.binary_op.left  && c2->u.binary_op.right == c1->u.binary_op.right ||
             c2->flavour == DATAFLOW_LESS_THAN_EQUAL    && c2->u.binary_op.left == c1->u.binary_op.right && c2->u.binary_op.right == c1->u.binary_op.left;
   case DATAFLOW_LESS_THAN_EQUAL:
      return c2->flavour == DATAFLOW_GREATER_THAN && c2->u.binary_op.left == c1->u.binary_op.left  && c2->u.binary_op.right == c1->u.binary_op.right ||
             c2->flavour == DATAFLOW_LESS_THAN    && c2->u.binary_op.left == c1->u.binary_op.right && c2->u.binary_op.right == c1->u.binary_op.left;
   case DATAFLOW_GREATER_THAN:
      return c2->flavour == DATAFLOW_LESS_THAN_EQUAL    && c2->u.binary_op.left == c1->u.binary_op.left  && c2->u.binary_op.right == c1->u.binary_op.right ||
             c2->flavour == DATAFLOW_GREATER_THAN_EQUAL && c2->u.binary_op.left == c1->u.binary_op.right && c2->u.binary_op.right == c1->u.binary_op.left;
   case DATAFLOW_GREATER_THAN_EQUAL:
      return c2->flavour == DATAFLOW_LESS_THAN    && c2->u.binary_op.left == c1->u.binary_op.left  && c2->u.binary_op.right == c1->u.binary_op.right ||
             c2->flavour == DATAFLOW_GREATER_THAN && c2->u.binary_op.left == c1->u.binary_op.right && c2->u.binary_op.right == c1->u.binary_op.left;
   case DATAFLOW_EQUAL:
      return c2->flavour == DATAFLOW_NOT_EQUAL && (c2->u.binary_op.left == c1->u.binary_op.left  && c2->u.binary_op.right == c1->u.binary_op.right ||
                                                   c2->u.binary_op.left == c1->u.binary_op.right && c2->u.binary_op.right == c1->u.binary_op.left);
   case DATAFLOW_NOT_EQUAL:
      return c2->flavour == DATAFLOW_EQUAL && (c2->u.binary_op.left == c1->u.binary_op.left  && c2->u.binary_op.right == c1->u.binary_op.right ||
                                               c2->u.binary_op.left == c1->u.binary_op.right && c2->u.binary_op.right == c1->u.binary_op.left);
   }

   return c1->flavour == DATAFLOW_LOGICAL_NOT && c1->u.unary_op.operand == c2 ||
          c2->flavour == DATAFLOW_LOGICAL_NOT && c2->u.unary_op.operand == c1;
}
#endif

Dataflow* glsl_dataflow_construct_cond_op(Dataflow* cond, Dataflow* true_value, Dataflow* false_value)
{
   Dataflow* dataflow;

   if (cond->flavour == DATAFLOW_CONST_BOOL) {
      if (cond->u.const_bool.value)
         return true_value;
      else
         return false_value;
   }

   if (true_value == false_value)
      return true_value;

   if (true_value->flavour == DATAFLOW_CONST_BOOL && false_value->flavour == DATAFLOW_CONST_BOOL) {
      // TODO: when CSE implemented, remove the following
      if (true_value->u.const_bool.value == false_value->u.const_bool.value)
         return true_value;

      if (true_value->u.const_bool.value) {
         // We're a mov
         vcos_assert(!false_value->u.const_bool.value);

         return cond;
      } else {
         // We're a not
         vcos_assert(false_value->u.const_bool.value);

         return glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, cond);
      }
   }

   if (dataflow_is_bool(true_value) && dataflow_is_bool(false_value) &&
      glsl_dataflow_get_bool_rep(true_value) != glsl_dataflow_get_bool_rep(false_value))
   {
      /*
         Constructing a conditional may not work as inputs have different bool reps.
         So build out of ANDs and ORs instead.
      */
      return glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_OR,
         glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND,
            cond,
            true_value),
         glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND,
            glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, cond),
            false_value));
   }

   if (true_value == cond && false_value->flavour == DATAFLOW_CONST_BOOL && !false_value->u.const_bool.value)
      return cond;
   if (false_value == cond && true_value->flavour == DATAFLOW_CONST_BOOL && true_value->u.const_bool.value)
      return cond;

#if 0
   if (true_value->flavour == DATAFLOW_CONDITIONAL && is_complement(cond, true_value->u.cond_op.cond))
      return glsl_dataflow_construct_cond_op(cond, true_value->u.cond_op.false_value, false_value);

   if (false_value->flavour == DATAFLOW_CONDITIONAL && is_complement(cond, false_value->u.cond_op.cond))
      return glsl_dataflow_construct_cond_op(cond, true_value, false_value->u.cond_op.true_value);
#endif
   if (cond->flavour == DATAFLOW_LOGICAL_NOT)
      return glsl_dataflow_construct_cond_op(cond->d.unary_op.operand, false_value, true_value);

   /*
      TODO: consider replacing this with proper tracking of guarding predicates in back end
   */

   if (true_value->flavour == DATAFLOW_CONDITIONAL && true_value->d.cond_op.cond == cond)
      return glsl_dataflow_construct_cond_op(cond, true_value->d.cond_op.true_value, false_value);
   if (false_value->flavour == DATAFLOW_CONDITIONAL && false_value->d.cond_op.cond == cond)
      return glsl_dataflow_construct_cond_op(cond, true_value, false_value->d.cond_op.false_value);


   dataflow = dataflow_construct_common(DATAFLOW_CONDITIONAL);

   vcos_assert(dataflow_is_bool(cond));

   if (dataflow_is_bool(true_value))
   {
      set_bool_rep(dataflow, glsl_dataflow_get_bool_rep(true_value));
   }

   // Add this node to the lists of dependents of the nodes it depends on.
   glsl_dataflow_add_dependent(cond, dataflow);
   glsl_dataflow_add_dependent(true_value, dataflow);
   glsl_dataflow_add_dependent(false_value, dataflow);

   dataflow->dependencies_count = 3;

	dataflow->d.cond_op.cond = cond;
	dataflow->d.cond_op.true_value = true_value;
	dataflow->d.cond_op.false_value = false_value;

   return dataflow;
}

Dataflow* glsl_dataflow_construct_texture_lookup_set(DataflowFlavour flavour, Dataflow* sampler, Dataflow* param, Dataflow* depends_on0, Dataflow* depends_on1, Dataflow* depends_on2)
{
   Dataflow* dataflow;

   dataflow = dataflow_construct_common(flavour);

   switch (flavour)
   {
      case DATAFLOW_TEX_SET_COORD_S:
      case DATAFLOW_TEX_SET_COORD_T:
      case DATAFLOW_TEX_SET_COORD_R:
      case DATAFLOW_TEX_SET_BIAS:
      case DATAFLOW_TEX_SET_LOD:
      case DATAFLOW_TEX_SET_DIRECT:
         break;

      default:
         UNREACHABLE();
         return NULL;
   }

   // Add this node to the lists of dependents of the nodes it depends on.
   dataflow->dependencies_count = 1;
   if (sampler) {
      glsl_dataflow_add_dependent(sampler, dataflow);
      ++dataflow->dependencies_count;
   }
   glsl_dataflow_add_dependent(param, dataflow);

   if (depends_on0) {
      glsl_dataflow_add_iodependent(depends_on0, dataflow);
      glsl_dataflow_add_iodependency(dataflow, depends_on0);
   }
   if (depends_on1) {
      glsl_dataflow_add_iodependent(depends_on1, dataflow);
      glsl_dataflow_add_iodependency(dataflow, depends_on1);
   }
   if (depends_on2) {
      glsl_dataflow_add_iodependent(depends_on2, dataflow);
      glsl_dataflow_add_iodependency(dataflow, depends_on2);
   }

	dataflow->d.texture_lookup_set.sampler = sampler;
	dataflow->d.texture_lookup_set.param = param;

   return dataflow;
}

Dataflow* glsl_dataflow_construct_fragment_get(DataflowFlavour flavour)
{
   Dataflow* dataflow;

   dataflow = dataflow_construct_common(flavour);

   switch (flavour)
   {
      case DATAFLOW_FRAG_GET_Z:
      case DATAFLOW_FRAG_GET_W:
      case DATAFLOW_FRAG_GET_PC_X:
      case DATAFLOW_FRAG_GET_PC_Y:
      case DATAFLOW_VARYING_C:
      case DATAFLOW_FRAG_GET_X:
      case DATAFLOW_FRAG_GET_Y:
      case DATAFLOW_FRAG_GET_COL:
         break;

      case DATAFLOW_FRAG_GET_FF:
         set_bool_rep(dataflow, BOOL_REP_BOOL_N);
         break;

      default:
         UNREACHABLE();
         return NULL;
   }

	dataflow->dependencies_count = 0;

   return dataflow;
}

Dataflow* glsl_dataflow_construct_texture_lookup_get(DataflowFlavour flavour, Dataflow* sampler, Dataflow* depends_on0, Dataflow* depends_on1, Dataflow* depends_on2)
{
   Dataflow* dataflow;

   dataflow = dataflow_construct_common(flavour);

   switch (flavour)
   {
      case DATAFLOW_TEX_GET_CMP_R:
      case DATAFLOW_TEX_GET_CMP_G:
      case DATAFLOW_TEX_GET_CMP_B:
      case DATAFLOW_TEX_GET_CMP_A:
         break;

      default:
         UNREACHABLE();
         return NULL;
   }

   // Add this node to the lists of dependents of the nodes it depends on.
   dataflow->dependencies_count = 0;
   if (sampler) {
      glsl_dataflow_add_dependent(sampler, dataflow);
      dataflow->dependencies_count = 1;
   }

   if (depends_on0) {
      glsl_dataflow_add_iodependent(depends_on0, dataflow);
      glsl_dataflow_add_iodependency(dataflow, depends_on0);
   }
   if (depends_on1) {
      glsl_dataflow_add_iodependent(depends_on1, dataflow);
      glsl_dataflow_add_iodependency(dataflow, depends_on1);
   }
   if (depends_on2) {
      glsl_dataflow_add_iodependent(depends_on2, dataflow);
      glsl_dataflow_add_iodependency(dataflow, depends_on2);
   }

	dataflow->d.texture_lookup_get.sampler = sampler;

   return dataflow;
}

Dataflow* glsl_dataflow_construct_vertex_set(DataflowFlavour flavour, Dataflow* param, Dataflow* prev)
{
   Dataflow* dataflow;

   vcos_assert(flavour == DATAFLOW_VERTEX_SET || flavour == DATAFLOW_VPM_READ_SETUP || flavour == DATAFLOW_VPM_WRITE_SETUP);

   dataflow = dataflow_construct_common(flavour);

   // Add this node to the lists of dependents of the nodes it depends on.
   glsl_dataflow_add_dependent(param, dataflow);

   dataflow->dependencies_count = 1;

   if (prev)
   {
      glsl_dataflow_add_iodependent(prev, dataflow);
      glsl_dataflow_add_iodependency(dataflow, prev);
   }

   dataflow->d.vertex_set.param = param;

   return dataflow;
}

Dataflow* glsl_dataflow_construct_fragment_submit(DataflowFlavour flavour, Dataflow* param, Dataflow *prev, Dataflow *discard)
{
   Dataflow* dataflow;

   dataflow = dataflow_construct_common(flavour);

   switch (flavour)
   {
      case DATAFLOW_FRAG_SUBMIT_STENCIL:
      case DATAFLOW_FRAG_SUBMIT_Z:
      case DATAFLOW_FRAG_SUBMIT_MS:
      case DATAFLOW_FRAG_SUBMIT_ALL:
      case DATAFLOW_FRAG_SUBMIT_R0:
      case DATAFLOW_FRAG_SUBMIT_R1:
      case DATAFLOW_FRAG_SUBMIT_R2:
      case DATAFLOW_FRAG_SUBMIT_R3:
      case DATAFLOW_TMU_SWAP:
         break;

      default:
         UNREACHABLE();
         return NULL;
   }

   // Add this node to the lists of dependents of the nodes it depends on.
   glsl_dataflow_add_dependent(param, dataflow);
   dataflow->dependencies_count = 1;

   if (prev)
   {
      glsl_dataflow_add_iodependent(prev, dataflow);
      glsl_dataflow_add_iodependency(dataflow, prev);
   }

	dataflow->d.fragment_set.param = param;
   if (discard == NULL || (discard->flavour == DATAFLOW_CONST_BOOL && discard->u.const_bool.value == CONST_BOOL_FALSE)) {
      dataflow->d.fragment_set.discard = NULL;
   } else {
      dataflow->d.fragment_set.discard = discard;
      glsl_dataflow_add_dependent(discard, dataflow);
      ++dataflow->dependencies_count;
   }

   return dataflow;
}

Dataflow *glsl_dataflow_construct_pack(DataflowFlavour flavour, Dataflow *operand, Dataflow *background)
{
   Dataflow* dataflow;

   dataflow = dataflow_construct_common(flavour);

   vcos_assert(DATAFLOW_PACK_COL_R == flavour ||
      DATAFLOW_PACK_COL_G == flavour ||
      DATAFLOW_PACK_COL_B == flavour ||
      DATAFLOW_PACK_COL_A == flavour ||
      DATAFLOW_PACK_16A == flavour ||
      DATAFLOW_PACK_16B == flavour ||
      (DATAFLOW_PACK_COL_REPLICATE == flavour && !khrn_options.old_glsl_sched));

   // Add this node to the lists of dependents of the nodes it depends on.
   glsl_dataflow_add_dependent(operand, dataflow);
   dataflow->dependencies_count = 1;
   if (NULL != background) {
      glsl_dataflow_add_dependent(background, dataflow);
      ++dataflow->dependencies_count;
   }

   dataflow->d.pack.operand = operand;
   dataflow->d.pack.background = background;

   return dataflow;
}

static bool same_for_pack_purposes(Dataflow *a, Dataflow *b)
{
   if (a == b)
      return true;

   if (a->flavour != b->flavour)
      return false;

   switch (a->flavour)
   {
   case DATAFLOW_CONST_FLOAT :
      return a->u.const_float.value == b->u.const_float.value &&
               a->u.const_float.index == b->u.const_float.index;
   case DATAFLOW_CONST_INT :
      return a->u.const_int.value == b->u.const_int.value &&
               a->u.const_int.index == b->u.const_int.index;
   case DATAFLOW_CONST_BOOL :
      return a->u.const_bool.value == b->u.const_bool.value &&
               a->u.const_bool.index == b->u.const_bool.index;

   default:
      return false;
   }
}

Dataflow* glsl_dataflow_construct_pack_col(Dataflow* red, Dataflow *green, Dataflow *blue, Dataflow *alpha)
{
   Dataflow *dataflow = NULL;
   Dataflow *unpack_operand = NULL;

   if (!khrn_options.old_glsl_sched)
   {
      // Use pack replicate where appropriate
      Dataflow *allCol[4] = { red, green, blue, alpha };
      uint32_t cnt[4] = { 0, 0, 0, 0 };
      uint32_t i, most = 0, replicate = 0;

      for (i = 0; i < 4; i++)
      {
         if (same_for_pack_purposes(allCol[i], red))   cnt[0]++;
         if (same_for_pack_purposes(allCol[i], green)) cnt[1]++;
         if (same_for_pack_purposes(allCol[i], blue))  cnt[2]++;
         if (same_for_pack_purposes(allCol[i], alpha)) cnt[3]++;
      }

      for (i = 0; i < 4; i++)
      {
         if (cnt[i] > most)
         {
            most = cnt[i];
            replicate = i;
         }
      }

      if (most > 1)
      {
         dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_REPLICATE, allCol[replicate], NULL);

         for (i = 0; i < 4; i++)
            if (!same_for_pack_purposes(allCol[replicate], allCol[i]))
               dataflow = glsl_dataflow_construct_pack((DataflowFlavour)(DATAFLOW_PACK_COL_R + i), allCol[i], dataflow);

         return dataflow;
      }
   }

#if 1
   /* if *ANY* of the pack originally came from an unpack, just take it as it was */
   if (red->flavour == DATAFLOW_UNPACK_COL_R)
   {
      dataflow = glsl_dataflow_construct_unary_op(DATAFLOW_MOV, red->d.unary_op.operand);
      unpack_operand = red->d.unary_op.operand;
   }
   else if (green->flavour == DATAFLOW_UNPACK_COL_G)
   {
      dataflow = glsl_dataflow_construct_unary_op(DATAFLOW_MOV, green->d.unary_op.operand);
      unpack_operand = green->d.unary_op.operand;
   }
   else if (blue->flavour == DATAFLOW_UNPACK_COL_B)
   {
      dataflow = glsl_dataflow_construct_unary_op(DATAFLOW_MOV, blue->d.unary_op.operand);
      unpack_operand = blue->d.unary_op.operand;
   }
   else if (alpha->flavour == DATAFLOW_UNPACK_COL_A)
   {
      dataflow = glsl_dataflow_construct_unary_op(DATAFLOW_MOV, alpha->d.unary_op.operand);
      unpack_operand = alpha->d.unary_op.operand;
   }
   /* else : NO match is valid */

   if (red->flavour != DATAFLOW_UNPACK_COL_R ||
       unpack_operand != red->d.unary_op.operand)
   {
      dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_R, red, dataflow);
   }

   if (green->flavour != DATAFLOW_UNPACK_COL_G ||
       unpack_operand != green->d.unary_op.operand)
   {
      dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_G, green, dataflow);
   }

   if (blue->flavour != DATAFLOW_UNPACK_COL_B ||
       unpack_operand != blue->d.unary_op.operand)
   {
      dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_B, blue, dataflow);
   }

   if (alpha->flavour != DATAFLOW_UNPACK_COL_A ||
       unpack_operand != alpha->d.unary_op.operand)
   {
      dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_A, alpha, dataflow);
   }
#else
   if (red->flavour == DATAFLOW_UNPACK_COL_R &&
       green->flavour == DATAFLOW_UNPACK_COL_G &&
       blue->flavour == DATAFLOW_UNPACK_COL_B &&
       alpha->flavour == DATAFLOW_UNPACK_COL_A &&
      red->d.unary_op.operand == green->d.unary_op.operand &&
      red->d.unary_op.operand == blue->d.unary_op.operand &&
      red->d.unary_op.operand == alpha->d.unary_op.operand)
   {
      //all the inputs to this pack come in the same order from a single unpack
      //so we can collapse the pack/unpack and just return the parent
      dataflow = red->d.unary_op.operand;
   }
   else
   {
      //add iodependencies to force ordering
//      glsl_dataflow_add_iodependency(green, red);
//      glsl_dataflow_add_iodependent(red, green);
//
//      glsl_dataflow_add_iodependency(blue, green);
//      glsl_dataflow_add_iodependent(green, blue);
//
//      glsl_dataflow_add_iodependency(alpha, blue);
//      glsl_dataflow_add_iodependent(blue, alpha);

      dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_R, red, dataflow);
      dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_G, green, dataflow);
      dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_B, blue, dataflow);
      dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_A, alpha, dataflow);
   }
#endif

   return dataflow;
}

Dataflow* glsl_dataflow_construct_pack_col_rgb(Dataflow* red, Dataflow *green, Dataflow *blue)
{
   Dataflow *dataflow = NULL;
   Dataflow *unpack_operand = NULL;

   if (!khrn_options.old_glsl_sched)
   {
      // Use pack replicate where appropriate
      Dataflow *allCol[3] = { red, green, blue };
      uint32_t cnt[3] = { 0, 0, 0 };
      uint32_t i, most = 0, replicate = 0;

      for (i = 0; i < 3; i++)
      {
         if (same_for_pack_purposes(allCol[i], red))   cnt[0]++;
         if (same_for_pack_purposes(allCol[i], green)) cnt[1]++;
         if (same_for_pack_purposes(allCol[i], blue))  cnt[2]++;
      }

      for (i = 0; i < 3; i++)
      {
         if (cnt[i] > most)
         {
            most = cnt[i];
            replicate = i;
         }
      }

      if (most > 1)
      {
         dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_REPLICATE, allCol[replicate], NULL);

         for (i = 0; i < 3; i++)
            if (!same_for_pack_purposes(allCol[replicate], allCol[i]))
               dataflow = glsl_dataflow_construct_pack((DataflowFlavour)(DATAFLOW_PACK_COL_R + i), allCol[i], dataflow);

         return dataflow;
      }
   }
#if 1
   /* if *ANY* of the pack originally came from an unpack, just take it as it was */
   if (red->flavour == DATAFLOW_UNPACK_COL_R)
   {
      dataflow = glsl_dataflow_construct_unary_op(DATAFLOW_MOV, red->d.unary_op.operand);
      unpack_operand = red->d.unary_op.operand;
   }
   else if (green->flavour == DATAFLOW_UNPACK_COL_G)
   {
      dataflow = glsl_dataflow_construct_unary_op(DATAFLOW_MOV, green->d.unary_op.operand);
      unpack_operand = green->d.unary_op.operand;
   }
   else if (blue->flavour == DATAFLOW_UNPACK_COL_B)
   {
      dataflow = glsl_dataflow_construct_unary_op(DATAFLOW_MOV, blue->d.unary_op.operand);
      unpack_operand = blue->d.unary_op.operand;
   }
   /* else : NO match is valid */

   if (red->flavour != DATAFLOW_UNPACK_COL_R ||
       unpack_operand != red->d.unary_op.operand)
   {
      dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_R, red, dataflow);
   }

   if (green->flavour != DATAFLOW_UNPACK_COL_G ||
       unpack_operand != green->d.unary_op.operand)
   {
      dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_G, green, dataflow);
   }

   if (blue->flavour != DATAFLOW_UNPACK_COL_B ||
       unpack_operand != blue->d.unary_op.operand)
   {
      dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_B, blue, dataflow);
   }
#else
   if (red->flavour  == DATAFLOW_UNPACK_COL_R &&
       green->flavour == DATAFLOW_UNPACK_COL_G &&
       blue->flavour == DATAFLOW_UNPACK_COL_B &&
      red->d.unary_op.operand == green->d.unary_op.operand &&
      red->d.unary_op.operand == blue->d.unary_op.operand )
   {
      //all the inputs to this pack come in the same order from a single unpack
      //so we can collapse the pack/unpack and just return the parent
      dataflow = red->d.unary_op.operand;
   }
   else
   {
      //add iodependencies to force ordering
//      glsl_dataflow_add_iodependency(green, red);
//      glsl_dataflow_add_iodependent(red, green);
//
//      glsl_dataflow_add_iodependency(blue, green);
//      glsl_dataflow_add_iodependent(green, blue);

      dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_R, red, dataflow);
      dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_G, green, dataflow);
      dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_B, blue, dataflow);
   }
#endif

   return dataflow;
}

Dataflow* glsl_dataflow_construct_pack_int16(Dataflow* a, Dataflow *b)
{
   Dataflow *dataflow = NULL;

   dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_16A, a, dataflow);
   dataflow = glsl_dataflow_construct_pack(DATAFLOW_PACK_16B, b, dataflow);

   return dataflow;
}

Dataflow* glsl_dataflow_construct_varying_tree(Dataflow* varying)
{
   Dataflow *w;
   Dataflow *c;
   Dataflow *dataflow;

   vcos_assert(varying->flavour == DATAFLOW_VARYING);

   w = glsl_dataflow_construct_fragment_get(DATAFLOW_FRAG_GET_W);
   c = glsl_dataflow_construct_fragment_get(DATAFLOW_VARYING_C);

   dataflow = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, varying, w);
   dataflow = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, dataflow, c);

   // We don't need this as an actual iodependency
   // Rather, it will let us find the VARYING_C node from the VARYING node.
   glsl_dataflow_add_iodependent(varying, c);
   glsl_dataflow_add_iodependency(c, varying);

   return dataflow;
}

Dataflow* glsl_dataflow_construct_varying_non_perspective_tree(Dataflow* varying)
{
   Dataflow *w;
   Dataflow *c;
   Dataflow *dataflow;

   vcos_assert(varying->flavour == DATAFLOW_VARYING);

   w = glsl_dataflow_construct_const_float(float_to_bits(1.0f));
   c = glsl_dataflow_construct_fragment_get(DATAFLOW_VARYING_C);

   dataflow = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, varying, w);
   dataflow = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, dataflow, c);

   // We don't need this as an actual iodependency
   // Rather, it will let us find the VARYING_C node from the VARYING node.
   glsl_dataflow_add_iodependent(varying, c);
   glsl_dataflow_add_iodependency(c, varying);

   return dataflow;
}

Dataflow* glsl_dataflow_construct_unpack(DataflowFlavour flavour, Dataflow* param)
{
   Dataflow* dataflow;

   dataflow = dataflow_construct_common(flavour);

   vcos_assert(
      flavour == DATAFLOW_UNPACK_COL_R ||
      flavour == DATAFLOW_UNPACK_COL_G ||
      flavour == DATAFLOW_UNPACK_COL_B ||
      flavour == DATAFLOW_UNPACK_COL_A ||
      flavour == DATAFLOW_UNPACK_16A ||
      flavour == DATAFLOW_UNPACK_16A_F ||
      flavour == DATAFLOW_UNPACK_16B ||
      flavour == DATAFLOW_UNPACK_16B_F ||
      flavour == DATAFLOW_UNPACK_8A ||
      flavour == DATAFLOW_UNPACK_8B ||
      flavour == DATAFLOW_UNPACK_8C ||
      flavour == DATAFLOW_UNPACK_8D ||
      flavour == DATAFLOW_UNPACK_8R);

   // Add this node to the lists of dependents of the nodes it depends on.
   glsl_dataflow_add_dependent(param, dataflow);

   dataflow->dependencies_count = 1;

   dataflow->d.unary_op.operand = param;

   return dataflow;
}

Dataflow* glsl_dataflow_construct_unpack_placeholder(DataflowFlavour flavour, Dataflow* param, Dataflow *sampler)
{
   Dataflow* dataflow;

   dataflow = dataflow_construct_common(flavour);

   vcos_assert(
      flavour == DATAFLOW_UNPACK_PLACEHOLDER_R ||
      flavour == DATAFLOW_UNPACK_PLACEHOLDER_B);

   // Add this node to the lists of dependents of the nodes it depends on.
   glsl_dataflow_add_dependent(param, dataflow);
   glsl_dataflow_add_dependent(sampler, dataflow);

   dataflow->dependencies_count = 2;

   dataflow->d.binary_op.left = param;
   dataflow->d.binary_op.right = sampler;

   return dataflow;
}

Dataflow* glsl_dataflow_construct_uniform_address(Dataflow* uniform, uint32_t size)
{
   Dataflow* dataflow;

   dataflow = dataflow_construct_common(DATAFLOW_UNIFORM_ADDRESS);

   vcos_assert(DATAFLOW_UNIFORM == uniform->flavour);

   // Add this node to the lists of dependents of the nodes it depends on.
   glsl_dataflow_add_dependent(uniform, dataflow);

   dataflow->d.indexed_uniform_sampler.uniform = uniform;

   dataflow->dependencies_count = 1;

   dataflow->u.indexed_uniform_sampler.size = size;

   return dataflow;
}

Dataflow* glsl_dataflow_construct_scoreboard_wait(Dataflow* operand)
{
   Dataflow* dataflow;

   dataflow = dataflow_construct_common(DATAFLOW_SCOREBOARD_WAIT);

   // Add this node to the lists of dependents of the nodes it depends on.

   if (operand != NULL)
   {
      glsl_dataflow_add_iodependent(operand, dataflow);
      glsl_dataflow_add_iodependency(dataflow, operand);
   }

   dataflow->dependencies_count = 0;

   return dataflow;
}

/*
   Convert boolean value into a form which can be handled by AND/OR/XOR.

   First strip off any "not" node (this doesn't correspond to any actual instruction, it
   just flips our sense of which value is true).

   If it is in BOOL_REP_BOOL form then leave it as it is.

   If it is in BOOL_REP_NEG form then shift right by 31 (to be left with msb)

   If it is in BOOL_REP_ZERO form then construct a conditional node, so that 1 gets moved
   over the top of the value if it is nonzero.
*/

static Dataflow *boolify(Dataflow *dataflow, BOOL_REP_T *rep_out)
{
   BOOL_REP_T rep;

   vcos_assert(dataflow_is_bool(dataflow));

   rep = glsl_dataflow_get_bool_rep(dataflow);

   if (DATAFLOW_LOGICAL_NOT == dataflow->flavour)
   {
      dataflow = dataflow->d.unary_op.operand;
   }

   switch (rep)
   {
   case BOOL_REP_BOOL:
   case BOOL_REP_BOOL_N:
      *rep_out = rep;
      return dataflow;

   case BOOL_REP_NEG:
   case BOOL_REP_NEG_N:
   {
      Dataflow *const31;

      /* TODO: it's annoying that we have to do it this way (to stop ourselves accidentally converting 31 back to float) */
      const31 = glsl_dataflow_construct_const_float(31);

      dataflow = glsl_dataflow_construct_binary_op(
         DATAFLOW_SHIFT_RIGHT,
         dataflow,
         const31);
      rep = (rep == BOOL_REP_NEG) ? BOOL_REP_BOOL : BOOL_REP_BOOL_N;
      set_bool_rep(dataflow, rep);
      *rep_out = rep;
      return dataflow;
   }
   case BOOL_REP_ZERO:
   case BOOL_REP_ZERO_N:
      if (glsl_dataflow_get_bool_rep(dataflow) == BOOL_REP_ZERO)
      {
         /* TODO: the hackiness is to prevent glsl_dataflow_construct_cond_op from flattening */
         Dataflow *cbool = glsl_dataflow_construct_const_float(1);
         set_bool_rep(cbool, BOOL_REP_ZERO);
         dataflow = glsl_dataflow_construct_cond_op(
            dataflow,
            dataflow,
            cbool);
      }
      else
      {
         Dataflow *cbool = glsl_dataflow_construct_const_float(1);
         set_bool_rep(cbool, BOOL_REP_ZERO_N);
         dataflow = glsl_dataflow_construct_cond_op(
            dataflow,
            cbool,
            dataflow);
      }

      rep = (rep == BOOL_REP_ZERO) ? BOOL_REP_BOOL_N : BOOL_REP_BOOL;
      set_bool_rep(dataflow, rep);
      *rep_out = rep;
      return dataflow;

   default:
      UNREACHABLE();
      return NULL;
   }
}

BOOL_REP_T glsl_dataflow_get_bool_rep(Dataflow *dataflow)
{
   vcos_assert(dataflow->bool_rep != BOOL_REP_NONE);
   return dataflow->bool_rep;
}

static bool dataflow_is_bool(Dataflow *dataflow)
{
   return dataflow->bool_rep != BOOL_REP_NONE;
}

static void set_bool_rep(Dataflow *dataflow, BOOL_REP_T bool_rep)
{
   dataflow->bool_rep = bool_rep;
}


//
// Dataflow type conversion.
//

Dataflow* glsl_dataflow_convert_type(Dataflow* input, PrimitiveTypeIndex in_type_index, PrimitiveTypeIndex out_type_index)
{
   switch (in_type_index)
   {
      case PRIM_BOOL:
         switch (out_type_index)
         {
            case PRIM_BOOL:
               return input;

            case PRIM_INT:
               return glsl_dataflow_construct_cond_op(
                  input,
                  glsl_dataflow_construct_const_int(1),
                  glsl_dataflow_construct_const_int(0));

            case PRIM_FLOAT:
               return glsl_dataflow_construct_cond_op(
                  input,
                  glsl_dataflow_construct_const_float(CONST_FLOAT_ONE),
                  glsl_dataflow_construct_const_float(CONST_FLOAT_ZERO));

            default:
               UNREACHABLE();
               return NULL;
         }

      case PRIM_INT:
         switch (out_type_index)
         {
            case PRIM_BOOL:
               return glsl_dataflow_construct_binary_op(DATAFLOW_NOT_EQUAL, input, glsl_dataflow_construct_const_int(0));

            case PRIM_INT:
               return input;

            case PRIM_FLOAT:
               // constant folding
               if (input->flavour == DATAFLOW_CONST_INT) {
                  const_float f;
                  op_inttofloat__const_float__const_int(&f, &input->u.const_int.value);
                  return glsl_dataflow_construct_const_float(f);
               }

               return input;

            default:
               UNREACHABLE();
               return NULL;
         }

      case PRIM_FLOAT:
         switch (out_type_index)
         {
            case PRIM_BOOL:
               return glsl_dataflow_construct_binary_op(DATAFLOW_NOT_EQUAL, input, glsl_dataflow_construct_const_float(CONST_FLOAT_ZERO));

            case PRIM_INT:
               // "When constructors are used to convert a float to an int,
               // the fractional part of the floating-point value is dropped."
               // -- I take this to mean truncation.
               return glsl_dataflow_construct_unary_op(DATAFLOW_ITOF, glsl_dataflow_construct_unary_op(DATAFLOW_FTOI_TRUNC, input));

            case PRIM_FLOAT:
               return input;

            default:
               UNREACHABLE();
               return NULL;
         }

      default:
         UNREACHABLE();
         return NULL;
   }
}


//
// Dataflow calculation helper functions.
//

// Populates and array of Dataflow** with count members starting at offset.
// This represents the lvalue of the expr and should be updated if this expr is used as an lvalue.

static void expr_evaluate_lvalue(Expr* expr, Dataflow ***result, int offset, int count)
{
   STACK_CHECK();

   switch (expr->flavour)
   {
      case EXPR_VALUE:
      case EXPR_FUNCTION_CALL:
      case EXPR_PRIM_CONSTRUCTOR_CALL:
      case EXPR_TYPE_CONSTRUCTOR_CALL:
      case EXPR_POST_INC:
      case EXPR_POST_DEC:
      case EXPR_PRE_INC:
      case EXPR_PRE_DEC:
      case EXPR_ARITH_NEGATE:
      case EXPR_LOGICAL_NOT:
      case EXPR_MUL:
      case EXPR_DIV:
      case EXPR_ADD:
      case EXPR_SUB:
      case EXPR_LESS_THAN:
      case EXPR_LESS_THAN_EQUAL:
      case EXPR_GREATER_THAN:
      case EXPR_GREATER_THAN_EQUAL:
      case EXPR_EQUAL:
      case EXPR_NOT_EQUAL:
      case EXPR_LOGICAL_AND:
      case EXPR_LOGICAL_XOR:
      case EXPR_LOGICAL_OR:
      case EXPR_CONDITIONAL:
      case EXPR_ASSIGN:
      case EXPR_SEQUENCE:
      case EXPR_INTRINSIC_TEXTURE_2D_BIAS:
      case EXPR_INTRINSIC_TEXTURE_2D_LOD:
      case EXPR_INTRINSIC_TEXTURE_CUBE_BIAS:
      case EXPR_INTRINSIC_TEXTURE_CUBE_LOD:
      case EXPR_INTRINSIC_RSQRT:
      case EXPR_INTRINSIC_RCP:
      case EXPR_INTRINSIC_LOG2:
      case EXPR_INTRINSIC_EXP2:
      case EXPR_INTRINSIC_CEIL:
      case EXPR_INTRINSIC_FLOOR:
      case EXPR_INTRINSIC_SIGN:
      case EXPR_INTRINSIC_TRUNC:
      case EXPR_INTRINSIC_NEAREST:
      case EXPR_INTRINSIC_MIN:
      case EXPR_INTRINSIC_MAX:
      case EXPR_INTRINSIC_MINABS:
      case EXPR_INTRINSIC_MAXABS:
         // These are not lvalues.
         UNREACHABLE();
         return;

      case EXPR_INSTANCE:
         {
            int i;

            switch (expr->u.instance.symbol->flavour) {
            case SYMBOL_VAR_INSTANCE:
            case SYMBOL_PARAM_INSTANCE:
               if (!strcmp(expr->u.instance.symbol->name, "gl_FragColor"))
                  g_AssignedFragColor = true;
               if (!strcmp(expr->u.instance.symbol->name, "gl_FragData"))
                  g_AssignedFragData = true;

               for (i = 0; i < count; i++)
                  result[i] = expr->u.instance.symbol->u.var_instance.scalar_values + offset + i;
               break;

            default:
               UNREACHABLE();
               break;
            }

            break;
         }

      case EXPR_SUBSCRIPT:
         {
            const_int index;
            int member_scalar_count;

            // All subscripts used as lvalues must be indexed by constants.
            vcos_assert(expr->u.subscript.subscript->constant_index_expression);
            if (expr->u.subscript.subscript->compile_time_value)
            {
               index = *(const_int*)expr->u.subscript.subscript->compile_time_value;
            }
            else
            {
               Dataflow *dataflow;
               vcos_assert(expr->u.subscript.subscript->type->scalar_count == 1);
               expr_calculate_dataflow(&dataflow, expr->u.subscript.subscript);
               vcos_assert(dataflow->flavour == DATAFLOW_CONST_INT);
               index = dataflow->u.const_int.value;
            }

            switch (expr->u.subscript.aggregate->type->flavour)
            {
               case SYMBOL_PRIMITIVE_TYPE:
                  member_scalar_count = primitiveTypeSubscriptTypes[expr->u.subscript.aggregate->type->u.primitive_type.index]->scalar_count;
                  break;

               case SYMBOL_ARRAY_TYPE:
                  member_scalar_count = expr->u.subscript.aggregate->type->u.array_type.member_type->scalar_count;
                  break;

               default:
                  member_scalar_count = 0;
                  UNREACHABLE();
                  break;
            }

            // Perform bounds check.

            if (index * member_scalar_count >= expr->u.subscript.aggregate->type->scalar_count)
            {
               // Indexing with an integral constant expression greater than declared size.
               glsl_compile_error(ERROR_SEMANTIC, 20, expr->line_num, NULL);
               return;
            }

            expr_evaluate_lvalue(expr->u.subscript.aggregate, result, offset + index * member_scalar_count, count);

            break;
         }

      case EXPR_FIELD_SELECTOR_STRUCT:
         {
            int i;
            int field_no = expr->u.field_selector_struct.field_no;
            int scalar_count_offset;

            vcos_assert(SYMBOL_STRUCT_TYPE == expr->u.field_selector_struct.aggregate->type->flavour);

            for (i = 0, scalar_count_offset = 0; i < field_no; i++)
            {
               scalar_count_offset += expr->u.field_selector_struct.aggregate->type->u.struct_type.member_types[i]->scalar_count;
            }

            expr_evaluate_lvalue(expr->u.subscript.aggregate, result, offset + scalar_count_offset, count);

            break;
         }

      case EXPR_FIELD_SELECTOR_SWIZZLE:
         {
            Dataflow **aggregate_scalar_values[4];
            int i;

            vcos_assert(expr->u.field_selector_swizzle.aggregate->type->scalar_count <= 4);

            expr_evaluate_lvalue(expr->u.field_selector_swizzle.aggregate, aggregate_scalar_values, 0, expr->u.field_selector_swizzle.aggregate->type->scalar_count);

            for (i = 0; i < count; i++) {
               vcos_assert(expr->u.field_selector_swizzle.swizzle_slots[i + offset] != SWIZZLE_SLOT_UNUSED);

               result[i] = aggregate_scalar_values[expr->u.field_selector_swizzle.swizzle_slots[i + offset]];
            }

            break;
         }

      default:
         // Nothing else.
         UNREACHABLE();
         break;
   }
}

// Writes dataflow graph pointers for compile_time_value to scalar_values.
// scalar_values is array of Dataflow* with expr->type->scalar_count members.
static void expr_calculate_dataflow_compile_time_value(Dataflow** scalar_values, void* compile_time_value, SymbolType* type)
{
   STACK_CHECK();

   switch (type->flavour)
   {
      case SYMBOL_PRIMITIVE_TYPE:
         {
            int i;
            switch (primitiveTypeFlags[type->u.primitive_type.index] & (PRIM_BOOL_TYPE | PRIM_INT_TYPE | PRIM_FLOAT_TYPE))
            {
               case PRIM_BOOL_TYPE:
                  for (i = 0; i < type->scalar_count; i++)
                  {
                     scalar_values[i] = ((const_bool*)compile_time_value)[i] ? g_BoolTrue : g_BoolFalse;
                  }
                  return;

               case PRIM_INT_TYPE:
                  for (i = 0; i < type->scalar_count; i++)
                  {
                     scalar_values[i] = glsl_dataflow_construct_const_int(((const_int*)compile_time_value)[i]);
                  }
                  return;

               case PRIM_FLOAT_TYPE:
                  for (i = 0; i < type->scalar_count; i++)
                  {
                     scalar_values[i] = glsl_dataflow_construct_const_float(((const_float*)compile_time_value)[i]);
                  }
                  return;

               default:
                  UNREACHABLE();
                  return;
            }
         }

      case SYMBOL_STRUCT_TYPE:
         {
            int i;

            for (i = 0; i < type->u.struct_type.member_count; i++)
            {
               SymbolType* member_type = type->u.struct_type.member_types[i];

               expr_calculate_dataflow_compile_time_value(scalar_values, compile_time_value, member_type);
               compile_time_value = (char *)compile_time_value + member_type->size_as_const;
               scalar_values += member_type->scalar_count;
            }
         }
         return;

      case SYMBOL_ARRAY_TYPE:
         // These can't have compile time values.
         UNREACHABLE();
         return;

      default:
         UNREACHABLE();
         return;
   }
}



//
// Dataflow calculation for different expression flavours.
//

typedef enum {
   GUARD_LOOP,
   GUARD_FUNCTION
} GuardFlavour;

typedef struct _Guard {
   GuardFlavour flavour;

   union {
      struct {
         Dataflow *break_guard;
         Dataflow *continue_guard;
      } loop;

      struct {
         Dataflow **scalar_values;
         Dataflow *return_guard;
      } function;
   } u;

   Dataflow *if_guard;

   struct _Guard *next;
} Guard;

static Guard *guard_construct_loop(Guard *next)
{
   Guard *guard = (Guard *)malloc_fast(sizeof(Guard));

   guard->flavour = GUARD_LOOP;
   guard->u.loop.break_guard = g_BoolTrue;
   guard->u.loop.continue_guard = g_BoolTrue;
   guard->if_guard = g_BoolTrue;
   guard->next = next;

   return guard;
}

static Guard *guard_construct_function(Dataflow **scalar_values, Guard *next)
{
   Guard *guard = (Guard *)malloc_fast(sizeof(Guard));

   guard->flavour = GUARD_FUNCTION;
   guard->u.function.scalar_values = scalar_values;
   guard->u.function.return_guard = g_BoolTrue;
   guard->if_guard = g_BoolTrue;
   guard->next = next;

   return guard;
}

static Guard *g_CurrentGuard;

static Dataflow *guard_scalar_assign(Dataflow *new_value, Dataflow *old_value, bool is_local_variable)
{
   Dataflow *cond = g_BoolTrue;
   Guard *guard;

   for (guard = g_CurrentGuard; guard; guard = guard->next) {
      switch (guard->flavour) {
      case GUARD_LOOP:
         cond = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, cond, guard->u.loop.break_guard);
         cond = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, cond, guard->u.loop.continue_guard);
         break;
      case GUARD_FUNCTION:
         cond = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, cond, guard->u.function.return_guard);
         break;
      default:
         UNREACHABLE();
         break;
      }

      cond = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, cond, guard->if_guard);

      if (guard->flavour == GUARD_FUNCTION && is_local_variable)
         break;
   }

   return glsl_dataflow_construct_cond_op(cond, new_value, old_value);
}

static void update_return_guard(void)
{
   /*
      we've not returned if we hadn't returned and either the current statement is inactive or we've broken or continued

      r = r . (i + c + b)
        = r . (i . c . b)

      we must consider the if, continue and break guards out to the level of the function we are returning from
   */

   Dataflow *cond = g_BoolTrue;
   Guard *guard;

   for (guard = g_CurrentGuard; guard->flavour == GUARD_LOOP; guard = guard->next) {
      cond = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, cond, guard->u.loop.break_guard);
      cond = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, cond, guard->u.loop.continue_guard);
      cond = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, cond, guard->if_guard);
   }

   cond = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, cond, guard->if_guard);
   cond = glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, cond);

   guard->u.function.return_guard = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, cond, guard->u.function.return_guard);
}

static void update_scalar_values(Dataflow **scalar_values, int count)
{
   Guard *guard = g_CurrentGuard;
   int i;

   while (guard->flavour == GUARD_LOOP)
      guard = guard->next;

   for (i = 0; i < count; i++)
      guard->u.function.scalar_values[i] = glsl_dataflow_construct_cond_op(guard->u.function.return_guard, scalar_values[i], guard->u.function.scalar_values[i]);
}

// Increment the loop variable *without* applying guards.  The loop expression must be: ++, --, += or -=.
// This should have the same effect as re-declaring the variable at the start of the loop and initialising it with a literal.
// Other argument is that if the loop variable is affected by the guard, then all subsequent code that uses it is guarded anyway.
// (i.e. once we break from a loop, no extra code should be executed in the loop so it does not matter that the loop variable is wrong)
// 'loop_index' is the variable declared in the for statement and it *must not* be accessed after the end of the loop.
// We are trying to avoid the guard because it prevents us from determining the number of loop iterations at compile time.
static int unguarded_loop_iteration(Symbol *loop_index, Expr *loop)
{
   bool is_int;
   const_int loop_stride_int;
   const_float loop_stride_float;

   if (loop_index == NULL || loop == NULL)
      return 0;

   if (loop_index->type->u.primitive_type.index == PRIM_INT)
      is_int = 1;
   else if (loop_index->type->u.primitive_type.index == PRIM_FLOAT)
      is_int = 0;
   else
      return 0; // unsupported loop index type

   // Find the loop's stride from the iterator expression
   switch (loop->flavour) {
      case EXPR_POST_INC:
      case EXPR_PRE_INC:
         if (loop->u.unary_op.operand->flavour != EXPR_INSTANCE || loop->u.unary_op.operand->u.instance.symbol != loop_index)
            return 0; // loop expression must act directly on loop index

         if (is_int)
            loop_stride_int = 1;
         else
            loop_stride_float = CONST_FLOAT_ONE;
         break;
      case EXPR_POST_DEC:
      case EXPR_PRE_DEC:
         if (loop->u.unary_op.operand->flavour != EXPR_INSTANCE || loop->u.unary_op.operand->u.instance.symbol != loop_index)
            return 0; // loop expression must act directly on loop index

         if (is_int)
            loop_stride_int = -1;
         else
            loop_stride_float = CONST_FLOAT_MINUS_ONE;
         break;
      case EXPR_ASSIGN:
      {
         Expr *lhs = loop->u.assign_op.lvalue;
         Expr *rhs = loop->u.assign_op.rvalue;

         if (EXPR_ADD != rhs->flavour && EXPR_SUB != rhs->flavour)
            return 0; // loop expression must be: ++, --, += or -=

         if (EXPR_INSTANCE != lhs->flavour || lhs->u.instance.symbol != loop_index)
            return 0; // loop expression must act directly on loop index

         if (EXPR_INSTANCE != rhs->u.binary_op.left->flavour || rhs->u.binary_op.left->u.instance.symbol != loop_index)
            return 0; // loop expression must act directly on loop index

         if (!rhs->u.binary_op.right->compile_time_value)
            return 0; // right side of loop expression must be constant

         if (is_int) {
            loop_stride_int = *(const_int*)rhs->u.binary_op.right->compile_time_value;
            if (EXPR_SUB == rhs->flavour)
               op_arith_negate__const_int__const_int(&loop_stride_int, &loop_stride_int);
         } else {
            loop_stride_float = *(const_float*)rhs->u.binary_op.right->compile_time_value;
            if (EXPR_SUB == rhs->flavour)
               op_arith_negate__const_float__const_float(&loop_stride_float, &loop_stride_float);
         }

         break;
      }
      default:
         return 0; // loop expression must be: ++, --, += or -=
   }

   // Increment the value of the loop index by stride

   if (is_int) {
      if (loop_index->u.var_instance.scalar_values[0]->flavour != DATAFLOW_CONST_INT)
         return 0; // loop index must be a constant expression
      {
         const_int loop_value = loop_index->u.var_instance.scalar_values[0]->u.const_int.value;
         op_add__const_int__const_int__const_int(&loop_value, &loop_value, &loop_stride_int);
         loop_index->u.var_instance.scalar_values[0] = glsl_dataflow_construct_const_int(loop_value);
      }
   } else {
      if (loop_index->u.var_instance.scalar_values[0]->flavour != DATAFLOW_CONST_FLOAT)
         return 0; // loop index must be a constant expression
      {
         const_float loop_value = loop_index->u.var_instance.scalar_values[0]->u.const_float.value;
         op_add__const_float__const_float__const_float(&loop_value, &loop_value, &loop_stride_float);
         loop_index->u.var_instance.scalar_values[0] = glsl_dataflow_construct_const_float(loop_value);
      }
   }

   return 1; // success
}

#define MAX_ITERATIONS 1024

// Calculates dataflow for a loop (for, while or do-while)
// This conceptually handles loops that may have both pre-condition as well as post-contition
static void statement_calculate_dataflow_iterator(Symbol* loop_index, Statement *precond, Expr *loop, Expr *postcond, Statement *block)
{
   Dataflow *scalar_value;
   int i;
   bool f = false;
   Guard *function_guard;

   g_CurrentGuard = guard_construct_loop(g_CurrentGuard);

   for (i = 0; ; i++) {
      if (i >= MAX_ITERATIONS)
         glsl_compile_error(ERROR_OPTIMIZER, 3, block->line_num, "too many iterations or infinite loop");

      // Evaluate the condition of for-loop or while-loop
      if (precond != NULL) {
         if (precond->flavour == STATEMENT_EXPR) {
            expr_calculate_dataflow(&scalar_value, precond->u.expr.expr);
         } else {
            Symbol* var;
            vcos_assert(precond->flavour == STATEMENT_VAR_DECL);
            // Evaluate initializer and assign it to the symbol
            glsl_statement_accept_prefix(precond, &f, sprev_calculate_dataflow, NULL);
            var = precond->u.var_decl.var;
            if (var->u.var_instance.type_qual == TYPE_QUAL_CONST)
               scalar_value = glsl_dataflow_construct_const_bool(*((const_bool*)var->u.var_instance.compile_time_value));
            else
               scalar_value = var->u.var_instance.scalar_values[0];
         }

         if (scalar_value->flavour != DATAFLOW_CONST_BOOL)
            glsl_compile_error(ERROR_OPTIMIZER, 3, precond->line_num, "unable to determine the number of iterations at compile time");

         if (scalar_value->u.const_bool.value == 0)
            break; // the loop condition was false
      }

      // Excecute the body of the loop
      glsl_statement_accept_prefix(block, &f, sprev_calculate_dataflow, NULL);
      g_CurrentGuard->u.loop.continue_guard = glsl_dataflow_construct_const_bool(true);

      // Increment the loop counter
      if (loop != NULL) {
         // Try to increment the loop counter statically, so that we know the compile time result...
         if (!unguarded_loop_iteration(loop_index, loop)) {
            // ...but if we can not, calculate the proper guarded value
            expr_calculate_dataflow(&scalar_value, loop);
         }
      }

      // Evaluate the condition of do-while-loop
      if (postcond != NULL) {
         expr_calculate_dataflow(&scalar_value, postcond);

         if (scalar_value->flavour != DATAFLOW_CONST_BOOL)
            glsl_compile_error(ERROR_OPTIMIZER, 3, postcond->line_num, "unable to determine the number of iterations at compile time");

         if (scalar_value->u.const_bool.value == 0)
            break; // the loop condition was false
      }

      // Allow infinite loop to be terminated by deterministic break statement
      if (g_CurrentGuard->u.loop.break_guard->flavour == DATAFLOW_CONST_BOOL && g_CurrentGuard->u.loop.break_guard->u.const_bool.value == 0)
         break;

      // Allow loop to be terminated by deterministic return statement
      for (function_guard = g_CurrentGuard; function_guard->flavour != GUARD_FUNCTION; function_guard = function_guard->next) { }
      if (function_guard->u.function.return_guard->flavour == DATAFLOW_CONST_BOOL && function_guard->u.function.return_guard->u.const_bool.value == 0)
         break;
   }

   g_CurrentGuard = g_CurrentGuard->next;
}

// Locates all expressions top-down, and calculates dataflow graphs for them.
static Statement* sprev_calculate_dataflow(Statement* statement, void* data)
{
   bool traverse_function_def = *(bool*)data;

   switch (statement->flavour)
   {
      case STATEMENT_AST:
      case STATEMENT_COMPOUND:
      case STATEMENT_DECL_LIST:
         return statement; // no expressions at this level, but recurse down

      case STATEMENT_FUNCTION_DEF:
         if (traverse_function_def || !strcmp(statement->u.function_def.header->name, "main"))
            return statement;
         else
            return NULL;

      case STATEMENT_VAR_DECL:
         {
            vcos_assert(SYMBOL_VAR_INSTANCE == statement->u.var_decl.var->flavour);

            // If there's an initializer, and the variable is non-const,
            // save the dataflow graphs (for each scalar value in the initializer) in the variable.
            if (statement->u.var_decl.initializer && TYPE_QUAL_CONST != statement->u.var_decl.var->u.var_instance.type_qual)
            {
               expr_calculate_dataflow(statement->u.var_decl.var->u.var_instance.scalar_values, statement->u.var_decl.initializer);
            }
         }
         return NULL; // no need to recurse

      case STATEMENT_EXPR:
         {
            // Throw away the dataflow graphs at the top level.
            Dataflow** scalar_values = stack_alloc_by_type(statement->u.expr.expr->type);

            expr_calculate_dataflow(scalar_values, statement->u.expr.expr);

            stack_free();
         }
         return NULL; // no need to recurse

      case STATEMENT_SELECTION:
         {
            Dataflow *cond_scalar_value;
            expr_calculate_dataflow(&cond_scalar_value, statement->u.selection.cond);
            dataflow_line_num = statement->line_num;

            {
               Dataflow *if_guard = g_CurrentGuard->if_guard;
               bool f = false;

               g_CurrentGuard->if_guard = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, cond_scalar_value, if_guard);

               glsl_statement_accept_prefix(statement->u.selection.if_true, &f, sprev_calculate_dataflow, NULL);
               dataflow_line_num = statement->line_num;

               if (statement->u.selection.if_false) {
                  g_CurrentGuard->if_guard = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, cond_scalar_value), if_guard);

                  glsl_statement_accept_prefix(statement->u.selection.if_false, &f, sprev_calculate_dataflow, NULL);
               }

               g_CurrentGuard->if_guard = if_guard;
            }
         }
         return NULL; // already handled recursion manually

      case STATEMENT_ITERATOR_FOR:
      {
         bool f = false;
         Symbol* loop_index = NULL;

         if (statement->u.iterator_for.init != NULL && statement->u.iterator_for.init->flavour == STATEMENT_VAR_DECL)
            loop_index = statement->u.iterator_for.init->u.var_decl.var;

         glsl_statement_accept_prefix(statement->u.iterator_for.init, &f, sprev_calculate_dataflow, NULL);

         statement_calculate_dataflow_iterator(loop_index, statement->u.iterator_for.cond_or_decl, statement->u.iterator_for.loop, NULL, statement->u.iterator_for.block);
         return NULL;
      }

      case STATEMENT_ITERATOR_WHILE:
         statement_calculate_dataflow_iterator(NULL, statement->u.iterator_while.cond_or_decl, NULL, NULL, statement->u.iterator_while.block);
         return NULL;

      case STATEMENT_ITERATOR_DO_WHILE:
         statement_calculate_dataflow_iterator(NULL, NULL, NULL, statement->u.iterator_do_while.cond, statement->u.iterator_do_while.block);
         return NULL;

      case STATEMENT_CONTINUE:
         if (g_CurrentGuard->flavour != GUARD_LOOP)
            glsl_compile_error(ERROR_CUSTOM, 29, statement->line_num, NULL);

         /*
            we've not continued if we hadn't continued and the current statement is inactive

            c = c . i
         */

         dataflow_line_num = statement->line_num;
         g_CurrentGuard->u.loop.continue_guard = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, g_CurrentGuard->u.loop.continue_guard, glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, g_CurrentGuard->if_guard));

         return NULL;

      case STATEMENT_BREAK:
         if (g_CurrentGuard->flavour != GUARD_LOOP)
            glsl_compile_error(ERROR_CUSTOM, 29, statement->line_num, NULL);

         /*
            we've not broken if we hadn't broken and either the current statement is inactive or we've continued

            b = b . (i + c)
              = b . (i . c)
         */

         dataflow_line_num = statement->line_num;
         g_CurrentGuard->u.loop.break_guard = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, g_CurrentGuard->u.loop.break_guard, glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, g_CurrentGuard->if_guard, g_CurrentGuard->u.loop.continue_guard)));

         return NULL;

      case STATEMENT_DISCARD:
         dataflow_line_num = statement->line_num;
         g_BuiltInVar__discard->u.var_instance.scalar_values[0] = guard_scalar_assign(g_BoolTrue, g_BuiltInVar__discard->u.var_instance.scalar_values[0], false);
         return NULL;

      case STATEMENT_RETURN:
         dataflow_line_num = statement->line_num;
         update_return_guard();
         return NULL;

      case STATEMENT_RETURN_EXPR:
      {
         Dataflow** scalar_values = stack_alloc_by_type(statement->u.return_expr.expr->type);

         expr_calculate_dataflow(scalar_values, statement->u.return_expr.expr);

         dataflow_line_num = statement->line_num;
         update_scalar_values(scalar_values, statement->u.return_expr.expr->type->scalar_count);
         update_return_guard();

         stack_free();
         return NULL;
      }

      case STATEMENT_NULL:
         return NULL; // no need to recurse

      default:
         UNREACHABLE();
         return NULL;
   }
}

void glsl_calculate_dataflow(Statement* ast)
{
   bool f = false;

   TRACE_PHASE(("calculating dataflow graphs"));

   g_CurrentGuard = guard_construct_function(NULL, NULL);

   glsl_statement_accept_prefix(ast, &f, sprev_calculate_dataflow, NULL);

   dataflow_line_num = LINE_NUMBER_UNDEFINED;
}

static INLINE void expr_calculate_dataflow_instance(Dataflow** scalar_values, Expr* expr)
{
   // Copy all dataflow pointers from the variable.
   switch (expr->u.instance.symbol->flavour) {
   case SYMBOL_VAR_INSTANCE:
   case SYMBOL_PARAM_INSTANCE:
      khrn_memcpy(scalar_values, expr->u.instance.symbol->u.var_instance.scalar_values, expr->type->scalar_count * sizeof(Dataflow*));
      break;
   default:
      UNREACHABLE();
      break;
   }
}

static Dataflow *build_uniform_offset(Expr *expr, Dataflow **linkable_value, bool *is_array, int n)
{
   STACK_CHECK();

   switch (expr->flavour) {
   case EXPR_INSTANCE:
   {
      int i;

      // Assert that only uniforms can have non-constant indexing.
      vcos_assert(SYMBOL_VAR_INSTANCE == expr->u.instance.symbol->flavour &&
             TYPE_QUAL_UNIFORM == expr->u.instance.symbol->u.var_instance.type_qual);

      for (i = 0; i < n; i++)
         linkable_value[i] = expr->u.instance.symbol->u.var_instance.scalar_values[i];

      return g_IntZero;
   }
   case EXPR_SUBSCRIPT:
   {
      Dataflow* aggregate_scalar_value;
      Dataflow* subscript_scalar_value;

      // Recurse on aggregate.
      aggregate_scalar_value = build_uniform_offset(expr->u.subscript.aggregate, linkable_value, is_array, n);
      // Record we've processed an array node, any future EXPR_FIELD_SELECTOR_STRUCT need to have
      // address calc applied for TMU
      *is_array = true;

      // Recurse on subscript.
      expr_calculate_dataflow(&subscript_scalar_value, expr->u.subscript.subscript);

      subscript_scalar_value = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, subscript_scalar_value, glsl_dataflow_construct_const_int((const_int)expr->type->scalar_count));

      return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, subscript_scalar_value, aggregate_scalar_value);
   }
   case EXPR_FIELD_SELECTOR_STRUCT:
   {
      // Recurse on aggregate.
      Dataflow* aggregate_scalar_value = build_uniform_offset(expr->u.field_selector_struct.aggregate, linkable_value, is_array, n);
      int field_offset = 0;
      for (int i = 0; i < expr->u.field_selector_struct.field_no; i++)
         field_offset += expr->u.field_selector_struct.aggregate->type->u.struct_type.member_types[i]->scalar_count;

      // are we inside an array?
      if (!*is_array)
      {
         // Update start address for TMU base
         *linkable_value = &(*linkable_value)[field_offset];
         return aggregate_scalar_value;
      }
      else
         return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, glsl_dataflow_construct_const_int(field_offset), aggregate_scalar_value);
   }

   default:
      UNREACHABLE();
      return NULL;
   }
}

static int num_tmu_lookups;     /* Number of tmu lookups used for uniform arrays */

static INLINE void expr_calculate_dataflow_subscript(Dataflow** scalar_values, Expr* expr)
{
    Dataflow** aggregate_scalar_values;
    bool       cond = false;

   if (expr->u.subscript.subscript->flavour == EXPR_INSTANCE) {
      Symbol *symbol = expr->u.subscript.subscript->u.instance.symbol;
      if (symbol->flavour == SYMBOL_PARAM_INSTANCE)
      {
         ExprChain *args;
         vcos_assert(calling_function_stack[calling_function_stack_top] != NULL);
         args = calling_function_stack[calling_function_stack_top]->u.function_call.args;
         if (args != NULL) {
            unsigned int i;
            ExprChainNode *node = args->first;
            vcos_assert(symbol->u.var_instance.param_index < (unsigned int)args->count);
            for (i=0; i<symbol->u.var_instance.param_index; i++)
               node=node->next;
            vcos_assert(node && node->expr);
            expr->u.subscript.subscript->constant_index_expression = node->expr->constant_index_expression;
         }
      }
   }

   if (expr->u.subscript.subscript->constant_index_expression)
   {
      // Check if the subscript is actually conditional, as it's not really a constant_index_expression
      // if that's the case. This will happen for a for loop inside an if for example.
      if (!expr->u.subscript.subscript->compile_time_value)
      {
         Dataflow *dataflow;
         vcos_assert(expr->u.subscript.subscript->type->scalar_count == 1);
         expr_calculate_dataflow(&dataflow, expr->u.subscript.subscript);

         if (dataflow->flavour == DATAFLOW_CONDITIONAL)
            cond = true;
      }
   }

   if (expr->u.subscript.subscript->constant_index_expression && !cond)
   {
      const_int expr_index;
      int scalar_index;

      if (expr->u.subscript.subscript->compile_time_value)
      {
         expr_index = *(const_int*)expr->u.subscript.subscript->compile_time_value;
      }
      else
      {
         Dataflow *dataflow;
         vcos_assert(expr->u.subscript.subscript->type->scalar_count == 1);
         expr_calculate_dataflow(&dataflow, expr->u.subscript.subscript);
         vcos_assert(dataflow->flavour == DATAFLOW_CONST_INT);
         expr_index = dataflow->u.const_int.value;
      }

      scalar_index = expr_index * expr->type->scalar_count;

      // Recurse on aggregate.
      if (EXPR_INSTANCE == expr->u.subscript.aggregate->flavour)
      {
         // It's a variable instance, so we can optimize out the recursion (which would just be a memcpy).
         // However, we do this less for speed, and more because it allows us to deal with much larger arrays.
         // (Instead of fitting the whole array in the stack, we need fit just a single member.)
         Symbol* symbol = expr->u.subscript.aggregate->u.instance.symbol;
         aggregate_scalar_values = symbol->u.var_instance.scalar_values;
      }
      else
      {
         aggregate_scalar_values = stack_alloc_by_type(expr->u.subscript.aggregate->type);

         expr_calculate_dataflow(aggregate_scalar_values, expr->u.subscript.aggregate);
      }

      if (scalar_index >= expr->u.subscript.aggregate->type->scalar_count)
      {
         // Indexing with an integral constant expression greater than declared size.
         glsl_compile_error(ERROR_SEMANTIC, 20, expr->line_num, NULL);
         return;
      }

      khrn_memcpy(scalar_values, aggregate_scalar_values + scalar_index, expr->type->scalar_count * sizeof(Dataflow*));

      if (EXPR_INSTANCE != expr->u.subscript.aggregate->flavour)
         stack_free();
   }
   else
   {
      uint32_t size;
      int i;
      Dataflow* subscript_scalar_value;

      aggregate_scalar_values = stack_alloc_by_type(expr->type);

      dataflow_line_num = expr->line_num;

      /* only generate FIELD_SELECTOR gpu code when the struct is inside a texture lookup.
         Array in structure, just keep as normal, whereas a structure in an array needs to have the
         respective TMU calculation applied */
      bool is_array = false;
      subscript_scalar_value = build_uniform_offset(expr, aggregate_scalar_values, &is_array, expr->type->scalar_count);

      {
         Expr *thing = expr;
         while (thing->flavour == EXPR_SUBSCRIPT)
         {
            thing = thing->u.subscript.aggregate;
         }
         size = thing->type->scalar_count;
      }

      {
         Dataflow *base = glsl_dataflow_construct_uniform_address(aggregate_scalar_values[0], size);

         subscript_scalar_value = glsl_dataflow_construct_binary_op(DATAFLOW_INTRINSIC_MAX, subscript_scalar_value, glsl_dataflow_construct_const_int(0));
         subscript_scalar_value = glsl_dataflow_construct_binary_op(DATAFLOW_INTRINSIC_MIN, subscript_scalar_value, glsl_dataflow_construct_const_int(size - expr->type->scalar_count));
         subscript_scalar_value = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, subscript_scalar_value, glsl_dataflow_construct_const_int(4));
         subscript_scalar_value = glsl_dataflow_construct_unary_op(DATAFLOW_FTOI_TRUNC, subscript_scalar_value);
         subscript_scalar_value = glsl_dataflow_construct_binary_op(DATAFLOW_INTEGER_ADD, base, subscript_scalar_value);

         num_tmu_lookups += expr->type->scalar_count;
         if (num_tmu_lookups >= MAX_TMU_LOOKUPS) {
            glsl_compile_error(ERROR_CUSTOM, 8, expr->line_num, "Caused by uniform array access");
         }

         for (i = 0; i < expr->type->scalar_count; i++)
         {
            Dataflow *value;

            /* TODO: it's annoying that we have to do it this way (to stop ourselves accidentally converting 31 back to float) */
            value = glsl_dataflow_construct_const_float(4 * i);

            value = glsl_dataflow_construct_binary_op(DATAFLOW_INTEGER_ADD, subscript_scalar_value, value);
            scalar_values[i] = glsl_dataflow_construct_linkable_value_offset(
               DATAFLOW_UNIFORM_OFFSET,
               glsl_get_scalar_value_type_index(expr->type, i),
               NULL,
               value);
         }
      }

      stack_free();
   }
}

static INLINE void expr_calculate_dataflow_sequence(Dataflow** scalar_values, Expr* expr)
{
   Dataflow** all_these_scalar_values = stack_alloc_by_type(expr->u.sequence.all_these->type);

   expr_calculate_dataflow(all_these_scalar_values, expr->u.sequence.all_these);
   expr_calculate_dataflow(scalar_values, expr->u.sequence.then_this);

   stack_free();
}

static INLINE void expr_calculate_dataflow_prim_constructor_call(Dataflow** scalar_values, Expr* expr)
{
   int d;
   ExprChain* args = expr->u.prim_constructor_call.args;
   PrimitiveTypeIndex out_scalar_type_index;
   PrimitiveTypeIndex in_scalar_type_index;

   vcos_assert(SYMBOL_PRIMITIVE_TYPE == expr->type->flavour);
   out_scalar_type_index = primitiveScalarTypeIndices[expr->type->u.primitive_type.index];

   d = primitiveTypeSubscriptDimensions[expr->type->u.primitive_type.index];

   switch(expr->u.prim_constructor_call.flavour)
   {
      case PRIM_CONS_SCALAR_FROM_FIRST_COMPONENT:
         {
            Dataflow** arg_scalar_values = stack_alloc_by_type(args->first->expr->type);

            // Recurse on first arg.
            expr_calculate_dataflow(arg_scalar_values, args->first->expr);
            dataflow_line_num = expr->line_num;

            vcos_assert(SYMBOL_PRIMITIVE_TYPE == args->first->expr->type->flavour);
            in_scalar_type_index = primitiveScalarTypeIndices[args->first->expr->type->u.primitive_type.index];

            // Convert first component of first arg.
            scalar_values[0] = glsl_dataflow_convert_type(arg_scalar_values[0], in_scalar_type_index, out_scalar_type_index);

            stack_free();
         }
         return;

      case PRIM_CONS_VECTOR_FROM_SCALAR:
         {
            int i;
            Dataflow** arg_scalar_values = stack_alloc_by_type(args->first->expr->type);
            Dataflow* converted_scalar_value;

            // Recurse on first arg.
            expr_calculate_dataflow(arg_scalar_values, args->first->expr);
            dataflow_line_num = expr->line_num;

            vcos_assert(SYMBOL_PRIMITIVE_TYPE == args->first->expr->type->flavour);
            in_scalar_type_index = primitiveScalarTypeIndices[args->first->expr->type->u.primitive_type.index];

            // Convert first component of first arg.
            converted_scalar_value = glsl_dataflow_convert_type(arg_scalar_values[0], in_scalar_type_index, out_scalar_type_index);

            // Fill.
            for (i = 0; i < d; i++)
            {
               scalar_values[i] = converted_scalar_value;
            }

            stack_free();
         }
         return;

      case PRIM_CONS_MATRIX_FROM_SCALAR:
         {
            int i, j, offset;
            Dataflow** arg_scalar_values = stack_alloc_by_type(args->first->expr->type);
            Dataflow* converted_scalar_value;

            // Recurse on first arg.
            expr_calculate_dataflow(arg_scalar_values, args->first->expr);
            dataflow_line_num = expr->line_num;

            vcos_assert(SYMBOL_PRIMITIVE_TYPE == args->first->expr->type->flavour);
            in_scalar_type_index = primitiveScalarTypeIndices[args->first->expr->type->u.primitive_type.index];

            // Convert first component of first arg.
            converted_scalar_value = glsl_dataflow_convert_type(arg_scalar_values[0], in_scalar_type_index, out_scalar_type_index);
            // Create zero value.

            // Fill.
            offset = 0;
            for (i = 0; i < d; i++)
            {
               for (j = 0; j < d; j++)
               {
                  scalar_values[offset++] = (i == j) ? converted_scalar_value : g_FloatZero;
               }
            }

            stack_free();
         }
         return;

      case PRIM_CONS_MATRIX_FROM_MATRIX:
         {
            int i, j, arg_d, offset;
            Dataflow** arg_scalar_values = stack_alloc_by_type(args->first->expr->type);

            // Recurse on first arg.
            expr_calculate_dataflow(arg_scalar_values, args->first->expr);

            // Find first arg dimension.
            vcos_assert(SYMBOL_PRIMITIVE_TYPE == args->first->expr->type->flavour);
            arg_d = primitiveTypeSubscriptDimensions[args->first->expr->type->u.primitive_type.index];

            // Fill.
            offset = 0;
            for (i = 0; i < d; i++)
            {
               for (j = 0; j < d; j++)
               {
                  scalar_values[offset++] = (i < arg_d && j < arg_d) ? arg_scalar_values[arg_d * i + j]
                     : ((i == j) ? g_FloatOne : g_FloatZero);
               }
            }

            stack_free();
         }
         return;

      case PRIM_CONS_VECTOR_OR_MATRIX_FROM_COMPONENT_FLOW:
         {
            int offset, arg_offset;
            ExprChainNode* arg;

            offset = 0;
            for (arg = args->first; arg; arg = arg->next)
            {
               Dataflow** arg_scalar_values = stack_alloc_by_type(arg->expr->type);

               arg_offset = 0;

               // Recurse on arg.
               expr_calculate_dataflow(arg_scalar_values, arg->expr);
               dataflow_line_num = expr->line_num;

               vcos_assert(SYMBOL_PRIMITIVE_TYPE == args->first->expr->type->flavour);
               in_scalar_type_index = primitiveScalarTypeIndices[arg->expr->type->u.primitive_type.index];

               while (arg_offset < arg->expr->type->scalar_count && offset < expr->type->scalar_count)
               {
                  scalar_values[offset++] = glsl_dataflow_convert_type(arg_scalar_values[arg_offset++], in_scalar_type_index, out_scalar_type_index);
               }

               stack_free();
            }
         }
         return;

      case PRIM_CONS_INVALID:
      default:
         UNREACHABLE();
         return;
   }
}

static INLINE void expr_calculate_dataflow_type_constructor_call(Dataflow** scalar_values, Expr* expr)
{
   ExprChainNode* arg;

   for (arg = expr->u.type_constructor_call.args->first; arg; arg = arg->next)
   {
      Dataflow** arg_scalar_values = stack_alloc_by_type(arg->expr->type);

      // Recurse on arg.
      expr_calculate_dataflow(arg_scalar_values, arg->expr);

      // Copy out result.
      khrn_memcpy(scalar_values, arg_scalar_values, arg->expr->type->scalar_count * sizeof(Dataflow*));
      scalar_values += arg->expr->type->scalar_count;

      stack_free();
   }
}

static INLINE void expr_calculate_dataflow_field_selector_swizzle(Dataflow** scalar_values, Expr* expr)
{
   int i;
   Dataflow** aggregate_scalar_values = stack_alloc_by_type(expr->u.field_selector_swizzle.aggregate->type);

   // Recurse on aggregate.
   expr_calculate_dataflow(aggregate_scalar_values, expr->u.field_selector_swizzle.aggregate);

   for (i = 0; i < MAX_SWIZZLE_FIELD_COUNT && expr->u.field_selector_swizzle.swizzle_slots[i] != SWIZZLE_SLOT_UNUSED; i++)
   {
      vcos_assert(expr->u.field_selector_swizzle.swizzle_slots[i] < expr->u.field_selector_swizzle.aggregate->type->scalar_count);

      scalar_values[i] = aggregate_scalar_values[expr->u.field_selector_swizzle.swizzle_slots[i]];
   }

   stack_free();
}

static INLINE void expr_calculate_dataflow_field_selector_struct(Dataflow** scalar_values, Expr* expr)
{
   int i;
   Dataflow** aggregate_scalar_values = stack_alloc_by_type(expr->u.field_selector_struct.aggregate->type);
   Dataflow** field = aggregate_scalar_values;

   // Recurse on aggregate.
   expr_calculate_dataflow(aggregate_scalar_values, expr->u.field_selector_struct.aggregate);

   for (i = 0; i < expr->u.field_selector_struct.field_no; i++)
   {
      field += expr->u.field_selector_struct.aggregate->type->u.struct_type.member_types[i]->scalar_count;
   }

   // Copy out field.
   khrn_memcpy(scalar_values, field, expr->type->scalar_count * sizeof(Dataflow*));

   stack_free();
}

// Not inlined as it's common.
static void expr_calculate_dataflow_unary_op_common(Dataflow** scalar_values, Expr* expr)
{
   Expr* operand;
   int i;
   Dataflow** operand_scalar_values;

   DataflowFlavour dataflow_flavour;

   // Work out operand.
   switch (expr->flavour)
   {
      // Logical.
      // Arithemetic.
      case EXPR_LOGICAL_NOT:
      case EXPR_ARITH_NEGATE:
         operand = expr->u.unary_op.operand;
         break;

      // Intrinsic.
      case EXPR_INTRINSIC_RSQRT:
      case EXPR_INTRINSIC_RCP:
      case EXPR_INTRINSIC_LOG2:
      case EXPR_INTRINSIC_EXP2:
      case EXPR_INTRINSIC_CEIL:
      case EXPR_INTRINSIC_FLOOR:
      case EXPR_INTRINSIC_SIGN:
      case EXPR_INTRINSIC_TRUNC:
      case EXPR_INTRINSIC_NEAREST:
         operand = expr->u.intrinsic.args->first->expr;
         break;

      default:
         UNREACHABLE();
         return;
   }

   operand_scalar_values = stack_alloc_by_type(operand->type);

   // Recurse.
   expr_calculate_dataflow(operand_scalar_values, operand);
   dataflow_line_num = expr->line_num;

   // Work out dataflow flavour.
   switch (expr->flavour)
   {
      // Logical.
      case EXPR_LOGICAL_NOT:
         dataflow_flavour = DATAFLOW_LOGICAL_NOT;
         break;

      // Arithemetic.
      case EXPR_ARITH_NEGATE:
         dataflow_flavour = DATAFLOW_ARITH_NEGATE;
         break;

      // Intrinsic.
      case EXPR_INTRINSIC_RSQRT:
         dataflow_flavour = DATAFLOW_INTRINSIC_RSQRT;
         break;
      case EXPR_INTRINSIC_RCP:
         dataflow_flavour = DATAFLOW_INTRINSIC_RCP;
         break;
      case EXPR_INTRINSIC_LOG2:
         dataflow_flavour = DATAFLOW_INTRINSIC_LOG2;
         break;
      case EXPR_INTRINSIC_EXP2:
         dataflow_flavour = DATAFLOW_INTRINSIC_EXP2;
         break;
      case EXPR_INTRINSIC_CEIL:
         dataflow_flavour = DATAFLOW_INTRINSIC_CEIL;
         break;
      case EXPR_INTRINSIC_FLOOR:
         dataflow_flavour = DATAFLOW_INTRINSIC_FLOOR;
         break;
      case EXPR_INTRINSIC_SIGN:
         dataflow_flavour = DATAFLOW_INTRINSIC_SIGN;
         break;
      case EXPR_INTRINSIC_TRUNC:
         dataflow_flavour = DATAFLOW_FTOI_TRUNC;
         break;
      case EXPR_INTRINSIC_NEAREST:
         dataflow_flavour = DATAFLOW_FTOI_NEAREST;
         break;

      default:
         UNREACHABLE();
         return;
   }

   // Create nodes.
   for (i = 0; i < expr->type->scalar_count; i++)
   {
      scalar_values[i] = glsl_dataflow_construct_unary_op(
         dataflow_flavour,
         operand_scalar_values[i]);

      if (dataflow_flavour == DATAFLOW_FTOI_TRUNC || dataflow_flavour == DATAFLOW_FTOI_NEAREST)
         scalar_values[i] = glsl_dataflow_construct_unary_op(
            DATAFLOW_ITOF,
            scalar_values[i]);
   }

   stack_free();
}

static INLINE void expr_calculate_dataflow_binary_op_arithmetic(Dataflow** scalar_values, Expr* expr)
{
   Expr* left = expr->u.binary_op.left;
   Expr* right = expr->u.binary_op.right;

   ExprFlavour flavour = expr->flavour;
   DataflowFlavour dataflow_flavour;

   Dataflow** left_scalar_values = stack_alloc_by_type(left->type);
   Dataflow** right_scalar_values = stack_alloc_by_type(right->type);

   // Recurse.
   expr_calculate_dataflow(left_scalar_values, left);
   expr_calculate_dataflow(right_scalar_values, right);
   dataflow_line_num = expr->line_num;

   // Work out dataflow flavour for the component-wise operations.
   switch (flavour)
   {
      case EXPR_MUL:
         dataflow_flavour = DATAFLOW_MUL;
         break;
      case EXPR_ADD:
         dataflow_flavour = DATAFLOW_ADD;
         break;
      case EXPR_SUB:
         dataflow_flavour = DATAFLOW_SUB;
         break;
      default:
         UNREACHABLE();
         return;
   }

   // The following control flow is based on that in ast.c and should be kept in sync with it.
   {
      int left_index, right_index;
      PRIMITIVE_TYPE_FLAGS_T left_flags, right_flags;
      left_index = left->type->u.primitive_type.index;
      left_flags = primitiveTypeFlags[left_index];
      right_index = right->type->u.primitive_type.index;
      right_flags = primitiveTypeFlags[right_index];

      // Infer type, and value if possible.
      // From spec (1.00 rev 14 p46), the two operands must be:
      // 1 - the same type (the type being integer scalar/vector, float scalar/vector/matrix),
      // 2 - or one can be a scalar float and the other a float vector or matrix,
      // 3 - or one can be a scalar integer and the other an integer vector,
      // 4 - or, for multiply, one can be a float vector and the other a float matrix with the same dimensional size.
      // All operations are component-wise except EXPR_MUL involving at least one matrix (cases 1 and 4).

      /* There are only floating point matrix types */
      vcos_assert( !(left_flags & PRIM_MATRIX_TYPE) || (left_flags & PRIM_FLOAT_TYPE) );
      vcos_assert( !(right_flags & PRIM_MATRIX_TYPE) || (right_flags & PRIM_FLOAT_TYPE) );

      // Case 1.
      if (left->type == right->type)
      {
         if (EXPR_MUL == flavour && (left_flags & PRIM_MATRIX_TYPE))
         {
            // Linear algebraic mat * mat.
            int i, j, k, d = primitiveTypeSubscriptDimensions[expr->type->u.primitive_type.index];

            for (i = 0; i < d; i++)
            {
               for (j = 0; j < d; j++)
               {
                  Dataflow* acc;

                  k = 0;

                  // Multiply the first pair.
                  acc = glsl_dataflow_construct_binary_op(
                     DATAFLOW_MUL,
                     left_scalar_values[k * d + j],
                     right_scalar_values[i * d + k]);

                  // Fold in the rest.
                  for (k = 1; k < d; k++)
                  {
                     Dataflow* mul;

                     mul = glsl_dataflow_construct_binary_op(
                        DATAFLOW_MUL,
                        left_scalar_values[k * d + j],
                        right_scalar_values[i * d + k]);

                     acc = glsl_dataflow_construct_binary_op(
                        DATAFLOW_ADD,
                        acc,
                        mul);
                  }

                  scalar_values[i * d + j] = acc;
               }
            }

            stack_free();
            stack_free();
            return;
         }
         else
         {
            // Component-wise on same scalar type, same scalar count.
            int i;

            for (i = 0; i < expr->type->scalar_count; i++)
            {
               vcos_assert(left_scalar_values[i]);
               vcos_assert(right_scalar_values[i]);

               scalar_values[i] = glsl_dataflow_construct_binary_op(
                  dataflow_flavour,
                  left_scalar_values[i],
                  right_scalar_values[i]);
            }

            stack_free();
            stack_free();
            return;
         }
      }

      vcos_assert( ((left_flags & PRIM_FLOAT_TYPE) && (right_flags & PRIM_FLOAT_TYPE)) ||
                   ((left_flags & PRIM_INT_TYPE)   && (right_flags & PRIM_INT_TYPE))     );

      // Cases 2 and 3.
      if ( (left_flags & PRIM_SCALAR_TYPE) &&
	        (right_flags & (PRIM_VECTOR_TYPE | PRIM_MATRIX_TYPE)))
      {
         // Component-wise on same scalar type, different scalar counts.
         int i;

         for (i = 0; i < expr->type->scalar_count; i++)
         {
            scalar_values[i] = glsl_dataflow_construct_binary_op(
               dataflow_flavour,
               left_scalar_values[0],
               right_scalar_values[i]);
         }

         stack_free();
         stack_free();
         return;
      }
      else if ( (right_flags & PRIM_SCALAR_TYPE) &&
                (left_flags & (PRIM_VECTOR_TYPE | PRIM_MATRIX_TYPE)))
      {
         // Component-wise on same scalar type, different scalar counts.
         int i;

         for (i = 0; i < expr->type->scalar_count; i++)
         {
            scalar_values[i] = glsl_dataflow_construct_binary_op(
               dataflow_flavour,
               left_scalar_values[i],
               right_scalar_values[0]);
         }

         stack_free();
         stack_free();
         return;
      }

      // Case 4.
      if (EXPR_MUL == flavour)
      {
         if ((left_flags & PRIM_FLOAT_TYPE) && (left_flags & PRIM_MATRIX_TYPE)
            && (right_flags & PRIM_FLOAT_TYPE) && (right_flags & PRIM_VECTOR_TYPE)
            && primitiveTypeSubscriptDimensions[left_index] == primitiveTypeSubscriptDimensions[left_index])
         {
            /* Linear algebraic mat * vec. */
            int j, k, d = primitiveTypeSubscriptDimensions[expr->type->u.primitive_type.index];
            Dataflow** vec_scalar_values = right_scalar_values;
            Dataflow** mat_scalar_values = left_scalar_values;

            for (j = 0; j < d; j++)
            {
               Dataflow* acc;

               k = 0;

               // Multiply the first pair.
               acc = glsl_dataflow_construct_binary_op(
                  DATAFLOW_MUL,
                  mat_scalar_values[k * d + j],
                  vec_scalar_values[k]); // [i, k] i.e. [0, k]

               // Fold in the rest.
               for (k = 1; k < d; k++)
               {
                  Dataflow* mul;

                  mul = glsl_dataflow_construct_binary_op(
                     DATAFLOW_MUL,
                     mat_scalar_values[k * d + j],
                     vec_scalar_values[k]); // [i, k] i.e. [0, k]

                  acc = glsl_dataflow_construct_binary_op(
                     DATAFLOW_ADD,
                     acc,
                     mul);
               }

               scalar_values[j] = acc; // [i, j] i.e. [0, j]
            }

            stack_free();
            stack_free();
            return;
         }
         else if ((right_flags & PRIM_FLOAT_TYPE) && (right_flags & PRIM_MATRIX_TYPE)
            && (left_flags & PRIM_FLOAT_TYPE) && (left_flags & PRIM_VECTOR_TYPE)
            && primitiveTypeSubscriptDimensions[left_index] == primitiveTypeSubscriptDimensions[left_index])
         {
            /* Linear algebraic vec * mat. */
            int i, k, d = primitiveTypeSubscriptDimensions[expr->type->u.primitive_type.index];
            Dataflow** vec_scalar_values = left_scalar_values;
            Dataflow** mat_scalar_values = right_scalar_values;

            for (i = 0; i < d; i++)
            {
               Dataflow* acc;

               k = 0;

               // Multiply the first pair.
               acc = glsl_dataflow_construct_binary_op(
                  DATAFLOW_MUL,
                  vec_scalar_values[k], // [k, j] i.e. [k, 0]
                  mat_scalar_values[i * d + k]);

               // Fold in the rest.
               for (k = 1; k < d; k++)
               {
                  Dataflow* mul;

                  mul = glsl_dataflow_construct_binary_op(
                     DATAFLOW_MUL,
                     vec_scalar_values[k], // [k, j] i.e. [k, 0]
                     mat_scalar_values[i * d + k]);

                  acc = glsl_dataflow_construct_binary_op(
                     DATAFLOW_ADD,
                     acc,
                     mul);
               }

               scalar_values[i] = acc; // [i, j] i.e. [i, 0]
            }

            stack_free();
            stack_free();
            return;
         }
      }
   }

   // All valid instances of expr should have been handled above.
   UNREACHABLE();
}

static Dataflow* construct_integer_rounder(Dataflow* operand)
{
   Dataflow *guess0, *guess1;
   guess0 = glsl_dataflow_construct_unary_op(DATAFLOW_INTRINSIC_FLOOR, operand);
   guess1 = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, guess0, glsl_dataflow_construct_const_int(1));
   return glsl_dataflow_construct_cond_op(
      glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN,
         glsl_dataflow_construct_binary_op(DATAFLOW_SUB, operand, guess0),
         glsl_dataflow_construct_binary_op(DATAFLOW_SUB, guess1, operand)
      ),
      guess0,
      guess1);
}

static INLINE void expr_calculate_dataflow_binary_op_divide(Dataflow** scalar_values, Expr* expr)
{
   Expr* left = expr->u.binary_op.left;
   Expr* right = expr->u.binary_op.right;

   Dataflow** left_scalar_values = stack_alloc_by_type(left->type);
   Dataflow** right_scalar_values = stack_alloc_by_type(right->type);

   // Recurse.
   expr_calculate_dataflow(left_scalar_values, left);
   expr_calculate_dataflow(right_scalar_values, right);
   dataflow_line_num = expr->line_num;

   // The inputs to this function come from ast.c. Anything that this function
   // won't process should be rejected there as a compile error.
   {
      int i;
      int left_index, right_index;
      PRIMITIVE_TYPE_FLAGS_T left_flags, right_flags;
      left_index = left->type->u.primitive_type.index;
      left_flags = primitiveTypeFlags[left_index];
      right_index = right->type->u.primitive_type.index;
      right_flags = primitiveTypeFlags[right_index];

      // Infer type, and value if possible.
      // From spec (1.00 rev 14 p46), the two operands must be:
      // 1 - the same type (the type being integer scalar/vector, float scalar/vector/matrix),
      // 2 - or one can be a scalar float and the other a float vector or matrix,
      // 3 - or one can be a scalar integer and the other an integer vector,
      // 4 - or, for multiply, one can be a float vector and the other a float matrix with the same dimensional size.
      // Case 4 never occurs here because it doesn't apply to division.
      // All division operations are component-wise.

      /* There are only floating point matrix types */
      vcos_assert( !(left_flags & PRIM_MATRIX_TYPE) || (left_flags & PRIM_FLOAT_TYPE) );
      vcos_assert( !(right_flags & PRIM_MATRIX_TYPE) || (right_flags & PRIM_FLOAT_TYPE) );

      // Case 1.
      if (left->type == right->type)
      {
         // Same scalar type, same scalar count.
         for (i = 0; i < expr->type->scalar_count; i++)
         {
            scalar_values[i] = glsl_dataflow_construct_binary_op(
               DATAFLOW_MUL,
               left_scalar_values[i],
               glsl_dataflow_construct_unary_op(
                                             DATAFLOW_INTRINSIC_RCP,
                                             right_scalar_values[i]));

            if (left_flags & PRIM_INT_TYPE)
               scalar_values[i] = construct_integer_rounder(scalar_values[i]);
         }

         stack_free();
         stack_free();
         return;
      }

      /* Left and right types should match float/int */
      vcos_assert( ( (left_flags & PRIM_FLOAT_TYPE) && (right_flags & PRIM_FLOAT_TYPE) ) ||
                   ( (left_flags & PRIM_INT_TYPE)   && (right_flags & PRIM_INT_TYPE)   )    );

      // Cases 2 and 3.
      // Same scalar type, different scalar counts.
      if ( (left_flags & PRIM_SCALAR_TYPE) &&
           (right_flags & (PRIM_VECTOR_TYPE | PRIM_MATRIX_TYPE)))
      {
         for (i = 0; i < expr->type->scalar_count; i++)
         {
            scalar_values[i] = glsl_dataflow_construct_binary_op(
               DATAFLOW_MUL,
               left_scalar_values[0],
               glsl_dataflow_construct_unary_op(
                                           DATAFLOW_INTRINSIC_RCP,
                                           right_scalar_values[i]));

            if (left_flags & PRIM_INT_TYPE)
               scalar_values[i] = construct_integer_rounder(scalar_values[i]);
         }
      }
      else
      {
         Dataflow *rcp = glsl_dataflow_construct_unary_op(
                                           DATAFLOW_INTRINSIC_RCP,
                                           right_scalar_values[0]);

         vcos_assert ( (right_flags & PRIM_SCALAR_TYPE) &&
                       (left_flags & (PRIM_VECTOR_TYPE | PRIM_MATRIX_TYPE)) );

         for (i = 0; i < expr->type->scalar_count; i++)
         {
            scalar_values[i] = glsl_dataflow_construct_binary_op(
               DATAFLOW_MUL,
               left_scalar_values[i],
               rcp);

            if (left_flags & PRIM_INT_TYPE)
               scalar_values[i] = construct_integer_rounder(scalar_values[i]);
         }
      }

      stack_free();
      stack_free();
      return;
   }
}

// Not inlined as it's common.
static void expr_calculate_dataflow_binary_op_common(Dataflow** scalar_values, Expr* expr)
{
   Expr* left;
   Expr* right;

   Dataflow* left_scalar_value;
   Dataflow* right_scalar_value;

   DataflowFlavour dataflow_flavour;

   // Work out left and right operands.
   switch (expr->flavour)
   {
      // Logical.
      // Relational.
      case EXPR_LOGICAL_XOR:
      case EXPR_LESS_THAN:
      case EXPR_LESS_THAN_EQUAL:
      case EXPR_GREATER_THAN:
      case EXPR_GREATER_THAN_EQUAL:
         left = expr->u.binary_op.left;
         right = expr->u.binary_op.right;
         break;

      // Intrinsic.
      case EXPR_INTRINSIC_MIN:
      case EXPR_INTRINSIC_MAX:
      case EXPR_INTRINSIC_MINABS:
      case EXPR_INTRINSIC_MAXABS:
         left = expr->u.intrinsic.args->first->expr;
         right = expr->u.intrinsic.args->first->next->expr;
         break;

      default:
         UNREACHABLE();
         return;
   }

   // This code assumes we do not act on vectors.
   vcos_assert(1 == left->type->scalar_count);
   vcos_assert(1 == right->type->scalar_count);

   // Recurse.
   expr_calculate_dataflow(&left_scalar_value, left);
   expr_calculate_dataflow(&right_scalar_value, right);
   dataflow_line_num = expr->line_num;

   // Work out dataflow flavour.
   switch (expr->flavour)
   {
      // Logical.
      case EXPR_LOGICAL_XOR:
         dataflow_flavour = DATAFLOW_LOGICAL_XOR;
         break;

      // Relational.
      case EXPR_LESS_THAN:
         dataflow_flavour = DATAFLOW_LESS_THAN;
         break;
      case EXPR_LESS_THAN_EQUAL:
         dataflow_flavour = DATAFLOW_LESS_THAN_EQUAL;
         break;
      case EXPR_GREATER_THAN:
         dataflow_flavour = DATAFLOW_GREATER_THAN;
         break;
      case EXPR_GREATER_THAN_EQUAL:
         dataflow_flavour = DATAFLOW_GREATER_THAN_EQUAL;
         break;

      // Intrinsic.
      case EXPR_INTRINSIC_MIN:
         dataflow_flavour = DATAFLOW_INTRINSIC_MIN;
         break;
      case EXPR_INTRINSIC_MAX:
         dataflow_flavour = DATAFLOW_INTRINSIC_MAX;
         break;
      case EXPR_INTRINSIC_MINABS:
         dataflow_flavour = DATAFLOW_INTRINSIC_MINABS;
         break;
      case EXPR_INTRINSIC_MAXABS:
         dataflow_flavour = DATAFLOW_INTRINSIC_MAXABS;
         break;

      default:
         UNREACHABLE();
         return;
   }

   // Create node.
   scalar_values[0] = glsl_dataflow_construct_binary_op(
      dataflow_flavour,
      left_scalar_value,
      right_scalar_value);
}

static void expr_calculate_dataflow_binary_op_short_circuit(Dataflow** scalar_values, Expr* expr)
{
   Expr* left;
   Expr* right;

   Dataflow* left_scalar_value;
   Dataflow* right_scalar_value;

   DataflowFlavour dataflow_flavour;

   left = expr->u.binary_op.left;
   right = expr->u.binary_op.right;

   // This code assumes we do not act on vectors.
   vcos_assert(1 == left->type->scalar_count);
   vcos_assert(1 == right->type->scalar_count);

   // Recurse.
   expr_calculate_dataflow(&left_scalar_value, left);
   dataflow_line_num = expr->line_num;

   // Work out dataflow flavour.
   switch (expr->flavour)
   {
      // Logical.
      case EXPR_LOGICAL_AND:
      {
         Dataflow *if_guard = g_CurrentGuard->if_guard;

         g_CurrentGuard->if_guard = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, left_scalar_value, if_guard);

         expr_calculate_dataflow(&right_scalar_value, right);
         dataflow_line_num = expr->line_num;

         g_CurrentGuard->if_guard = if_guard;

         dataflow_flavour = DATAFLOW_LOGICAL_AND;
         break;
      }
      case EXPR_LOGICAL_OR:
      {
         Dataflow *if_guard = g_CurrentGuard->if_guard;

         g_CurrentGuard->if_guard = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, left_scalar_value), if_guard);

         expr_calculate_dataflow(&right_scalar_value, right);
         dataflow_line_num = expr->line_num;

         g_CurrentGuard->if_guard = if_guard;

         dataflow_flavour = DATAFLOW_LOGICAL_OR;
         break;
      }
      default:
         UNREACHABLE();
         return;
   }

   // Create node.
   scalar_values[0] = glsl_dataflow_construct_binary_op(
      dataflow_flavour,
      left_scalar_value,
      right_scalar_value);
}

static INLINE void expr_calculate_dataflow_binary_op_equality(Dataflow** scalar_values, Expr* expr)
{
   Expr* left = expr->u.binary_op.left;
   Expr* right = expr->u.binary_op.right;

   Dataflow** left_scalar_values = stack_alloc_by_type(left->type);
   Dataflow** right_scalar_values = stack_alloc_by_type(right->type);

   DataflowFlavour dataflow_flavour, comp_flavour;

   Dataflow* acc;
   int i;

   // Recurse.
   expr_calculate_dataflow(left_scalar_values, left);
   expr_calculate_dataflow(right_scalar_values, right);
   dataflow_line_num = expr->line_num;

   // Work out dataflow flavour.
   switch (expr->flavour)
   {
      case EXPR_EQUAL:
         dataflow_flavour = DATAFLOW_EQUAL;
         comp_flavour = DATAFLOW_LOGICAL_AND;
         break;
      case EXPR_NOT_EQUAL:
         dataflow_flavour = DATAFLOW_NOT_EQUAL;
         comp_flavour = DATAFLOW_LOGICAL_OR;
         break;
      default:
         UNREACHABLE();
         return;
   }

   // Compare first two scalars.
   acc = glsl_dataflow_construct_binary_op(
      dataflow_flavour,
      left_scalar_values[0],
      right_scalar_values[0]);

   // Fold in the rest.
   vcos_assert(left->type->scalar_count == right->type->scalar_count);
   for (i = 1; i < left->type->scalar_count; i++)
   {
      Dataflow* cmp = glsl_dataflow_construct_binary_op(
         dataflow_flavour,
         left_scalar_values[i],
         right_scalar_values[i]);

      acc = glsl_dataflow_construct_binary_op(
         comp_flavour,
         acc,
         cmp);
   }

   scalar_values[0] = acc;

   stack_free();
   stack_free();
}

static INLINE void expr_calculate_dataflow_cond_op(Dataflow** scalar_values, Expr* expr)
{
   int i;
   Dataflow* cond_scalar_value;
   Dataflow** true_scalar_values = stack_alloc_by_type(expr->u.cond_op.if_true->type);
   Dataflow** false_scalar_values = stack_alloc_by_type(expr->u.cond_op.if_false->type);

   // Recurse on operands.
   expr_calculate_dataflow(&cond_scalar_value, expr->u.cond_op.cond);
   dataflow_line_num = expr->line_num;

   {
      Dataflow *if_guard = g_CurrentGuard->if_guard;

      g_CurrentGuard->if_guard = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, cond_scalar_value, if_guard);

      expr_calculate_dataflow(true_scalar_values, expr->u.cond_op.if_true);
      dataflow_line_num = expr->line_num;

      g_CurrentGuard->if_guard = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, cond_scalar_value), if_guard);

      expr_calculate_dataflow(false_scalar_values, expr->u.cond_op.if_false);
      dataflow_line_num = expr->line_num;

      g_CurrentGuard->if_guard = if_guard;
   }

   for (i = 0; i < expr->type->scalar_count; i++)
   {
      scalar_values[i] = glsl_dataflow_construct_cond_op(cond_scalar_value, true_scalar_values[i], false_scalar_values[i]);
   }

   stack_free();
   stack_free();
}

static bool is_lvalue_local(Expr *lvalue)
{
   if (lvalue->flavour == EXPR_INSTANCE)
   {
      Symbol *symbol = lvalue->u.instance.symbol;
      if (symbol->flavour == SYMBOL_VAR_INSTANCE)
         return symbol->u.var_instance.is_local;
   }
   return false;
}

static INLINE void expr_calculate_dataflow_assign_op(Dataflow** scalar_values, Expr* expr)
{
   Dataflow*** lvalue_scalar_values = (Dataflow ***)malloc_fast(expr->type->scalar_count * sizeof(Dataflow**));
   int i;

   // Recurse on rvalue and write value of this expression to scalar_values.
   expr_calculate_dataflow(scalar_values, expr->u.assign_op.rvalue);

   // Evaluate lvalue.
   expr_evaluate_lvalue(expr->u.assign_op.lvalue, lvalue_scalar_values, 0, expr->type->scalar_count);

   dataflow_line_num = expr->line_num;

   // Copy to rvalue to lvalue.
   for (i = 0; i < expr->type->scalar_count; i++)
      *(lvalue_scalar_values[i]) = guard_scalar_assign(scalar_values[i], *(lvalue_scalar_values[i]), is_lvalue_local(expr->u.assign_op.lvalue));
}

static INLINE void expr_calculate_dataflow_affix(Dataflow** scalar_values, Expr* expr)
{
   int i;

   Dataflow** operand_scalar_values = stack_alloc_by_type(expr->u.unary_op.operand->type);
   Dataflow*** lvalue_scalar_values = (Dataflow ***)malloc_fast(expr->type->scalar_count * sizeof(Dataflow**));  // TODO: stack allocate this

   bool is_pre = false;
   bool is_inc = false;

   // Manufacture a suitable representation of the constant 1.

   Dataflow* one = NULL;

   vcos_assert(expr->type->flavour == SYMBOL_PRIMITIVE_TYPE);

   switch (primitiveTypeFlags[expr->type->u.primitive_type.index] & (PRIM_INT_TYPE | PRIM_FLOAT_TYPE))
   {
      case PRIM_INT_TYPE:
         one = g_IntOne;
         break;
      case PRIM_FLOAT_TYPE:
         one = g_FloatOne;
         break;
      default:
         UNREACHABLE();
         break;
   }

   // detect whether we're pre or post, increment or decrement

   switch (expr->flavour) {
   case EXPR_POST_INC:
      is_pre = false;
      is_inc = true;
      break;
   case EXPR_POST_DEC:
      is_pre = false;
      is_inc = false;
      break;
   case EXPR_PRE_INC:
      is_pre = true;
      is_inc = true;
      break;
   case EXPR_PRE_DEC:
      is_pre = true;
      is_inc = false;
      break;
   default:
      UNREACHABLE();
      break;
   }

   // Evaluate lvalue.
   expr_evaluate_lvalue(expr->u.unary_op.operand, lvalue_scalar_values, 0, expr->type->scalar_count);

   // Evaluate rvalue.
   expr_calculate_dataflow(operand_scalar_values, expr->u.unary_op.operand);

   dataflow_line_num = expr->line_num;

   for (i = 0; i < expr->type->scalar_count; i++)
   {
      Dataflow *new_value = glsl_dataflow_construct_binary_op(
         is_inc ? DATAFLOW_ADD : DATAFLOW_SUB,
         operand_scalar_values[i],
         one);

      *(lvalue_scalar_values[i]) = guard_scalar_assign(new_value, *(lvalue_scalar_values[i]), is_lvalue_local(expr->u.unary_op.operand));

      scalar_values[i] = is_pre ? new_value : operand_scalar_values[i];
   }

   stack_free();
}

static INLINE void expr_calculate_dataflow_function_call(Dataflow** scalar_values, Expr* expr)
{
   int size;
   int i;

   Symbol *function = expr->u.function_call.function;
   Statement* function_def = function->u.function_instance.function_def;

   ExprChainNode *node;

   Dataflow **temps;

   if (function_def == NULL)
   {
      // Function call without function definition.
      glsl_compile_error(ERROR_CUSTOM, 21, expr->line_num, "maybe <%s> is a built-in function, and needs implementing?", function->name);
      return;
   }

   if (function_def->u.function_def.active)
   {
      // Function call without function definition.
      glsl_compile_error(ERROR_CUSTOM, 20, expr->line_num, "recursive call to function <%s>", function->name);
      return;
   }

   // Marshall input arguments.
   //
   // Note we do this first into a temporary buffer and then copy the temporaries into the formal parameter storage
   // so that we work correctly for nested invocations like min(a, min(b, c))

   node = expr->u.function_call.args->first;
   size = 0;
   for (i = 0; i < function->type->u.function_type.param_count; i++, node = node->next) {
      Symbol *formal = function->type->u.function_type.params[i];

      if (formal->u.var_instance.param_qual == PARAM_QUAL_IN || formal->u.var_instance.param_qual == PARAM_QUAL_INOUT) {
         size += formal->type->scalar_count;
      }
   }

   temps = stack_alloc_by_size(size);

   node = expr->u.function_call.args->first;
   size = 0;
   for (i = 0; i < function->type->u.function_type.param_count; i++, node = node->next) {
      Symbol *formal = function->type->u.function_type.params[i];

      if (formal->u.var_instance.param_qual == PARAM_QUAL_IN || formal->u.var_instance.param_qual == PARAM_QUAL_INOUT) {
         expr_calculate_dataflow(temps + size, node->expr);
         size += formal->type->scalar_count;
      }
   }

   node = expr->u.function_call.args->first;
   size = 0;
   for (i = 0; i < function->type->u.function_type.param_count; i++, node = node->next) {
      Symbol *formal = function->type->u.function_type.params[i];

      if (formal->u.var_instance.param_qual == PARAM_QUAL_IN || formal->u.var_instance.param_qual == PARAM_QUAL_INOUT) {
         khrn_memcpy(formal->u.var_instance.scalar_values, temps + size, formal->type->scalar_count * sizeof(Dataflow*));
         size += formal->type->scalar_count;
      }
   }

   stack_free();

   // Set return scalar values to be uninitialized.

   for (i = 0; i < expr->type->scalar_count; i++)
   {
      PrimitiveTypeIndex type_index = glsl_get_scalar_value_type_index(expr->type, i);

      switch (type_index)
      {
         case PRIM_BOOL:
            scalar_values[i] = g_BoolFalse;
            break;

         case PRIM_INT:
            scalar_values[i] = g_IntZero;
            break;

         case PRIM_FLOAT:
            scalar_values[i] = g_FloatZero;
            break;

         default:
            UNREACHABLE();
            return;
      }
   }

   // Evaluate function body.

   g_CurrentGuard = guard_construct_function(scalar_values, g_CurrentGuard);

   if (calling_function_stack_top + 1 >= sizeof(calling_function_stack)/sizeof(calling_function_stack[0])) {
      calling_function_stack_top = 0;
      glsl_compile_error(ERROR_CUSTOM, 38, expr->line_num, NULL);
   }

   calling_function_stack[++calling_function_stack_top] = expr;
   function_def->u.function_def.active = true;
   {
      bool t = true;
      glsl_statement_accept_prefix(function_def, &t, sprev_calculate_dataflow, NULL);
   }
   function_def->u.function_def.active = false;
   calling_function_stack[calling_function_stack_top--] = NULL;

   g_CurrentGuard = g_CurrentGuard->next;

   // Marshall output arguments.

   node = expr->u.function_call.args->first;

   for (i = 0; i < function->type->u.function_type.param_count; i++, node = node->next) {
      Symbol *formal = function->type->u.function_type.params[i];
      Expr *actual = node->expr;

      if (formal->u.var_instance.param_qual == PARAM_QUAL_OUT || formal->u.var_instance.param_qual == PARAM_QUAL_INOUT) {
         Dataflow*** lvalue_scalar_values = (Dataflow ***)malloc_fast(formal->type->scalar_count * sizeof(Dataflow**));
         int i;

         // Evaluate lvalue.

         expr_evaluate_lvalue(actual, lvalue_scalar_values, 0, formal->type->scalar_count);
         dataflow_line_num = expr->line_num;

         // Copy to rvalue to lvalue.

         for (i = 0; i < formal->type->scalar_count; i++)
            *(lvalue_scalar_values[i]) = guard_scalar_assign(formal->u.var_instance.scalar_values[i], *(lvalue_scalar_values[i]), is_lvalue_local(actual));
      }
   }

   vcos_assert(!node);
}

void glsl_init_samplers(void)
{
   int i;
   next_sampler_index_offsets = 0;

   for (i = 0; i < SHADER_FLAVOUR_COUNT; i++)
      num_samplers[i] = 0;
}

void glsl_init_texture_lookups(void)
{
   num_tmu_lookups = 0;
}

Dataflow* glsl_dataflow_construct_threadswitch(Dataflow* operand)
{
   Dataflow* dataflow;

	dataflow = dataflow_construct_common(DATAFLOW_THREADSWITCH);

   // Add this node to the lists of dependents of the nodes it depends on.

   if (operand != NULL)
   {
      glsl_dataflow_add_iodependent(operand, dataflow);
      glsl_dataflow_add_iodependency(dataflow, operand);
   }

   return dataflow;
}

static INLINE void expr_calculate_dataflow_texture_lookup(Dataflow** scalar_values, Expr* expr)
{
   switch (g_ShaderFlavour)
   {
      case SHADER_VERTEX:
      case SHADER_FRAGMENT:
         {
            Expr* sampler_expr;
            Dataflow* sampler_scalar_value;
            Expr* coord_expr;
            Dataflow* coord_scalar_values[COORD_DIM];
            Expr* bias_or_lod_expr;
            Dataflow* bias_or_lod_scalar_value;

            Dataflow* gadget_set_s;
            Dataflow* gadget_set_t;
            Dataflow* gadget_set_r;
            Dataflow* gadget_set_bias_or_lod;
            Dataflow* gadget_get_rgba;
            Dataflow* gadget_get_r;
            Dataflow* gadget_get_g;
            Dataflow* gadget_get_b;
            Dataflow* gadget_get_a;


            // Assert that we're returning a vec4 (i.e. rgba).
            vcos_assert(&primitiveTypes[PRIM_VEC4] == expr->type);

            // Gather args.
            sampler_expr = expr->u.intrinsic.args->first->expr;
            coord_expr = expr->u.intrinsic.args->first->next->expr;
            bias_or_lod_expr = expr->u.intrinsic.args->first->next->next->expr;
            switch (expr->flavour)
            {
               case EXPR_INTRINSIC_TEXTURE_2D_BIAS:
                  vcos_assert(&primitiveTypes[PRIM_SAMPLER2D] == sampler_expr->type ||
                          &primitiveTypes[PRIM_SAMPLEREXTERNAL] == sampler_expr->type);
                  vcos_assert(&primitiveTypes[PRIM_VEC2] == coord_expr->type);
                  vcos_assert(&primitiveTypes[PRIM_FLOAT] == bias_or_lod_expr->type);
                  texture_type = TEXTURE_2D;
                  lookup_type = LOOKUP_BIAS;
                  break;

               case EXPR_INTRINSIC_TEXTURE_2D_LOD:
                  vcos_assert(&primitiveTypes[PRIM_SAMPLER2D] == sampler_expr->type ||
                          &primitiveTypes[PRIM_SAMPLEREXTERNAL] == sampler_expr->type);
                  vcos_assert(&primitiveTypes[PRIM_VEC2] == coord_expr->type);
                  vcos_assert(&primitiveTypes[PRIM_FLOAT] == bias_or_lod_expr->type);
                  texture_type = TEXTURE_2D;
                  lookup_type = LOOKUP_LOD;
                  break;

               case EXPR_INTRINSIC_TEXTURE_CUBE_BIAS:
                  vcos_assert(&primitiveTypes[PRIM_SAMPLERCUBE] == sampler_expr->type);
                  vcos_assert(&primitiveTypes[PRIM_VEC3] == coord_expr->type);
                  vcos_assert(&primitiveTypes[PRIM_FLOAT] == bias_or_lod_expr->type);
                  texture_type = TEXTURE_CUBE;
                  lookup_type = LOOKUP_BIAS;
                  break;

               case EXPR_INTRINSIC_TEXTURE_CUBE_LOD:
                  vcos_assert(&primitiveTypes[PRIM_SAMPLERCUBE] == sampler_expr->type);
                  vcos_assert(&primitiveTypes[PRIM_VEC3] == coord_expr->type);
                  vcos_assert(&primitiveTypes[PRIM_FLOAT] == bias_or_lod_expr->type);
                  texture_type = TEXTURE_CUBE;
                  lookup_type = LOOKUP_LOD;
                  break;

               default:
                  UNREACHABLE();
                  return;
            }

            if (lookup_type == LOOKUP_LOD && g_ShaderFlavour != SHADER_VERTEX) {
                  /* Lod functions allowed only in vertex shader */
                  /* use sampler_expr as sampler_scalar_value is not valid yet */
                  glsl_compile_error(ERROR_CUSTOM, 37, sampler_expr->line_num, NULL);
                  return;
            }

            // Recurse on args.
            expr_calculate_dataflow(&sampler_scalar_value, sampler_expr);
            expr_calculate_dataflow(coord_scalar_values, coord_expr);
            expr_calculate_dataflow(&bias_or_lod_scalar_value, bias_or_lod_expr);
            dataflow_line_num = expr->line_num;

            if (g_ShaderFlavour == SHADER_VERTEX && lookup_type == LOOKUP_BIAS) {
                  if (DATAFLOW_CONST_FLOAT != bias_or_lod_scalar_value->flavour
                     || CONST_FLOAT_ZERO != bias_or_lod_scalar_value->u.const_float.value) {
                     /* Bias parameter not accepted in vertex shader */
                     glsl_compile_error(ERROR_CUSTOM, 36, sampler_expr->line_num, NULL);
                     return;
                  }
                  if (texture_type == TEXTURE_2D)
                     lookup_type = LOOKUP_LOD;
            }

            // Allocate this sampler to a texture unit, if it hasn't been done already.
            if (SAMPLER_LOCATION_UNDEFINED == sampler_scalar_value->u.const_sampler.location)
            {
               unsigned int max_flavour_samplers;

               if (next_sampler_index_offsets >= GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS)
               {
                  // Too many samplers.
                  glsl_compile_error(ERROR_CUSTOM, 35, sampler_scalar_value->line_num, NULL);
                  return;
               }

               if (g_ShaderFlavour == SHADER_VERTEX)
                  max_flavour_samplers = GL20_CONFIG_MAX_VERTEX_TEXTURE_UNITS;
               else
                  max_flavour_samplers = GL20_CONFIG_MAX_FRAGMENT_TEXTURE_UNITS;

               if (num_samplers[g_ShaderFlavour] > max_flavour_samplers)
               {
                  glsl_compile_error(ERROR_CUSTOM, 26 + g_ShaderFlavour,
                        sampler_scalar_value->line_num, NULL);
                  return;
               }

               num_samplers[g_ShaderFlavour]++;
               // Allocate.
               sampler_scalar_value->u.const_sampler.location = next_sampler_index_offsets;

               // Mark this sampler index offset as taken.
               next_sampler_index_offsets++;
            }

            num_tmu_lookups++;
            if (num_tmu_lookups >= MAX_TMU_LOOKUPS) {
               /* TODO: This doesn't seem to generate a useful line number */
               glsl_compile_error(ERROR_CUSTOM, 8, sampler_scalar_value->line_num, NULL);
            }

            // Create gadget.
            gadget_set_t = glsl_dataflow_construct_texture_lookup_set(
               DATAFLOW_TEX_SET_COORD_T,
               sampler_scalar_value,
               coord_scalar_values[COORD_T],
               NULL,
               NULL,
               NULL);
            switch (texture_type)
            {
               case TEXTURE_2D:
                  gadget_set_r = NULL;
                  break;

               case TEXTURE_CUBE:
                  gadget_set_r = glsl_dataflow_construct_texture_lookup_set(
                     DATAFLOW_TEX_SET_COORD_R,
                     sampler_scalar_value,
                     coord_scalar_values[COORD_R],
                     NULL,
                     NULL,
                     NULL);
                  break;

               default:
                  UNREACHABLE();
                  return;
            }
            switch (lookup_type)
            {
               case LOOKUP_BIAS:
                  if (DATAFLOW_CONST_FLOAT == bias_or_lod_scalar_value->flavour
                     && CONST_FLOAT_ZERO == bias_or_lod_scalar_value->u.const_float.value)
                  {
                     gadget_set_bias_or_lod = NULL;
                  }
                  else
                  {
                     gadget_set_bias_or_lod = glsl_dataflow_construct_texture_lookup_set(
                        DATAFLOW_TEX_SET_BIAS,
                        sampler_scalar_value,
                        bias_or_lod_scalar_value,
                        NULL,
                        NULL,
                        NULL);
                  }
                  break;

               case LOOKUP_LOD:
                  gadget_set_bias_or_lod = glsl_dataflow_construct_texture_lookup_set(
                     DATAFLOW_TEX_SET_LOD,
                     sampler_scalar_value,
                     bias_or_lod_scalar_value,
                     NULL,
                     NULL,
                     NULL);
                  break;

               default:
                  UNREACHABLE();
                  return;
            }
            gadget_set_s = glsl_dataflow_construct_texture_lookup_set(
               DATAFLOW_TEX_SET_COORD_S,
               sampler_scalar_value,
               coord_scalar_values[COORD_S],
               gadget_set_bias_or_lod,
               gadget_set_t,
               gadget_set_r);
            //TODO: thread switch
            gadget_get_rgba = glsl_dataflow_construct_texture_lookup_get(
               DATAFLOW_TEX_GET_CMP_R,
               sampler_scalar_value,
               gadget_set_s,
               NULL,
               NULL);

            gadget_get_r = glsl_dataflow_construct_unpack_placeholder(
               DATAFLOW_UNPACK_PLACEHOLDER_R,
               gadget_get_rgba,
               sampler_scalar_value);
            gadget_get_g = glsl_dataflow_construct_unpack(
               DATAFLOW_UNPACK_COL_G,
               gadget_get_rgba);
            gadget_get_b = glsl_dataflow_construct_unpack_placeholder(
               DATAFLOW_UNPACK_PLACEHOLDER_B,
               gadget_get_rgba,
               sampler_scalar_value);
            gadget_get_a = glsl_dataflow_construct_unpack(
               DATAFLOW_UNPACK_COL_A,
               gadget_get_rgba);

            // Set return values.
            scalar_values[0] = gadget_get_r;
            scalar_values[1] = gadget_get_g;
            scalar_values[2] = gadget_get_b;
            scalar_values[3] = gadget_get_a;

            return;
         }

      default:
         UNREACHABLE();
         return;
   }
}


//
// Dataflow calculation.
//

// Writes dataflow graph pointers for expr to scalar_values.
// scalar_values is array of Dataflow* with expr->type->scalar_count members.
static void expr_calculate_dataflow(Dataflow** scalar_values, Expr* expr)
{
   STACK_CHECK();

   vcos_assert(scalar_values);

   if (expr->compile_time_value)
   {
      // Constant expression.
      dataflow_line_num = expr->line_num;
      expr_calculate_dataflow_compile_time_value(scalar_values, expr->compile_time_value, expr->type);
      return;
   }
   else
   {
      // Non-constant expression.
      switch (expr->flavour)
      {
         case EXPR_VALUE:
            // There should not be any of these for which expr->compile_time_value is NULL.
            UNREACHABLE();
            return;

         case EXPR_FUNCTION_CALL:
            expr_calculate_dataflow_function_call(scalar_values, expr);
            return;

         case EXPR_FIELD_SELECTOR_SWIZZLE:
            expr_calculate_dataflow_field_selector_swizzle(scalar_values, expr);
            return;

         case EXPR_POST_INC:
         case EXPR_POST_DEC:
         case EXPR_PRE_INC:
         case EXPR_PRE_DEC:
            expr_calculate_dataflow_affix(scalar_values, expr);
            return;

         case EXPR_ASSIGN:
            expr_calculate_dataflow_assign_op(scalar_values, expr);
            return;

         case EXPR_SEQUENCE:
            expr_calculate_dataflow_sequence(scalar_values, expr);
            return;

         case EXPR_INSTANCE:
            expr_calculate_dataflow_instance(scalar_values, expr);
            return;

         case EXPR_SUBSCRIPT:
            expr_calculate_dataflow_subscript(scalar_values, expr);
            return;

         case EXPR_PRIM_CONSTRUCTOR_CALL:
            expr_calculate_dataflow_prim_constructor_call(scalar_values, expr);
            return;

         case EXPR_TYPE_CONSTRUCTOR_CALL:
            expr_calculate_dataflow_type_constructor_call(scalar_values, expr);
            return;

         case EXPR_FIELD_SELECTOR_STRUCT:
            expr_calculate_dataflow_field_selector_struct(scalar_values, expr);
            return;

         case EXPR_ARITH_NEGATE:
         case EXPR_LOGICAL_NOT:
            expr_calculate_dataflow_unary_op_common(scalar_values, expr);
            return;

         case EXPR_MUL:
         case EXPR_ADD:
         case EXPR_SUB:
            expr_calculate_dataflow_binary_op_arithmetic(scalar_values, expr);
            return;

         case EXPR_DIV:
            expr_calculate_dataflow_binary_op_divide(scalar_values, expr);
            return;

         case EXPR_LESS_THAN:
         case EXPR_LESS_THAN_EQUAL:
         case EXPR_GREATER_THAN:
         case EXPR_GREATER_THAN_EQUAL:
            expr_calculate_dataflow_binary_op_common(scalar_values, expr);
            return;

         case EXPR_EQUAL:
         case EXPR_NOT_EQUAL:
            expr_calculate_dataflow_binary_op_equality(scalar_values, expr);
            return;

         case EXPR_LOGICAL_AND:
         case EXPR_LOGICAL_OR:
            expr_calculate_dataflow_binary_op_short_circuit(scalar_values, expr);
            return;

         case EXPR_LOGICAL_XOR:
            expr_calculate_dataflow_binary_op_common(scalar_values, expr);
            return;

         case EXPR_CONDITIONAL:
            expr_calculate_dataflow_cond_op(scalar_values, expr);
            return;

         case EXPR_INTRINSIC_TEXTURE_2D_BIAS:
         case EXPR_INTRINSIC_TEXTURE_2D_LOD:
         case EXPR_INTRINSIC_TEXTURE_CUBE_BIAS:
         case EXPR_INTRINSIC_TEXTURE_CUBE_LOD:
            expr_calculate_dataflow_texture_lookup(scalar_values, expr);
            return;

         case EXPR_INTRINSIC_RSQRT:
         case EXPR_INTRINSIC_RCP:
         case EXPR_INTRINSIC_LOG2:
         case EXPR_INTRINSIC_EXP2:
         case EXPR_INTRINSIC_CEIL:
         case EXPR_INTRINSIC_FLOOR:
         case EXPR_INTRINSIC_SIGN:
         case EXPR_INTRINSIC_TRUNC:
         case EXPR_INTRINSIC_NEAREST:
            expr_calculate_dataflow_unary_op_common(scalar_values, expr);
            return;

         case EXPR_INTRINSIC_MIN:
         case EXPR_INTRINSIC_MAX:
         case EXPR_INTRINSIC_MINABS:
         case EXPR_INTRINSIC_MAXABS:
            expr_calculate_dataflow_binary_op_common(scalar_values, expr);
            return;

         default:
            UNREACHABLE();
            return;
      }
   }
}

void glsl_dataflow_priority_queue_init(DataflowPriorityQueue* queue, int size)
{
   queue->size = size;
   queue->used = 0;

   queue->nodes = (Dataflow **)malloc_fast(size * sizeof(Dataflow *));
}

static INLINE bool compare(const Dataflow *d0, const Dataflow *d1)
{
   return d1->delay > d0->delay;
}

#define COMPARE(d0, d1) ((d1)->delay > (d0)->delay)

static void siftDown(DataflowPriorityQueue *queue, int start, int end)
{
   int root = start;

   while (root * 2 + 1 <= end) {
      int child = (root << 1) + 1;

      if (child + 1 <= end && COMPARE(queue->nodes[child], queue->nodes[child + 1]))
          child++;

      if (COMPARE(queue->nodes[root], queue->nodes[child])) {
         Dataflow *temp = queue->nodes[root];
         queue->nodes[root] = queue->nodes[child];
         queue->nodes[child] = temp;

         root = child;
      } else
         return;
   }
}

static void siftUp(DataflowPriorityQueue *queue, int root)
{
   while (root > 0) {
      int parent = (root - 1) >> 1;

      if (compare(queue->nodes[parent], queue->nodes[root])) {
         Dataflow *temp = queue->nodes[root];
         queue->nodes[root] = queue->nodes[parent];
         queue->nodes[parent] = temp;

         root = parent;
      } else
         return;
   }
}

void glsl_dataflow_priority_queue_heapify(DataflowPriorityQueue* queue)
{
   int start = (queue->used - 2) >> 1;

   while (start >= 0) {
      siftDown(queue, start, queue->used - 1);
      start--;
   }
}

void glsl_dataflow_priority_queue_push(DataflowPriorityQueue* queue, Dataflow *node)
{
   vcos_assert(queue->used < queue->size);

   queue->nodes[queue->used] = node;

   siftUp(queue, queue->used++);
}

Dataflow *glsl_dataflow_priority_queue_pop(DataflowPriorityQueue* queue)
{
   if (queue->used == 0)
      return NULL;
   else {
      Dataflow *result = queue->nodes[0];

      queue->nodes[0] = queue->nodes[--queue->used];

      siftDown(queue, 0, queue->used - 1);

      return result;
   }
}

#include "middleware/khronos/common/khrn_mem.h"

typedef struct
{
   Dataflow *dependent;
   Dataflow **inputs;
   uint32_t num_inputs;
   MEM_HANDLE_T mh_out_blob;
   uint32_t alloc_amount;
   char *in_offset;
   bool revert;
   DataflowFlavour input_flavour;
#if GL_EXT_texture_format_BGRA8888
   bool *texture_rb_swap;
#endif
} GLSL_COPY_CONTEXT_T;

static void *stuff_alloc(GLSL_COPY_CONTEXT_T *stuff, uint32_t size)
{
   if (stuff->mh_out_blob == MEM_INVALID_HANDLE)
   {
      return malloc_fast(size);
   }
   else
   {
      void *result;
      if (stuff->alloc_amount + size > mem_get_size(stuff->mh_out_blob))
      {
         verify(mem_resize(stuff->mh_out_blob, mem_get_size(stuff->mh_out_blob) + 1024));   /* TODO: out of memory */
      }
      vcos_assert(stuff->alloc_amount + size <= mem_get_size(stuff->mh_out_blob));
      result = (void *)stuff->alloc_amount;
      stuff->alloc_amount += size;
      return result;
   }
}

static void *stuff_out_lock(GLSL_COPY_CONTEXT_T *stuff, void *handle)
{
   if (stuff->mh_out_blob == MEM_INVALID_HANDLE)
      return handle;
   else
      return ((char *)mem_lock(stuff->mh_out_blob, NULL)) + (size_t)handle;
}

static void stuff_out_unlock(GLSL_COPY_CONTEXT_T *stuff, void *handle)
{
   UNUSED(handle);

   if (stuff->mh_out_blob != MEM_INVALID_HANDLE)
      mem_unlock(stuff->mh_out_blob);
}

static void *stuff_in_translate(GLSL_COPY_CONTEXT_T *stuff, void *ptr)
{
   return ptr ? (stuff->in_offset + (size_t)ptr) : 0;
}

static void stuff_chain_append(GLSL_COPY_CONTEXT_T *stuff, DataflowChain *chain, Dataflow *dataflow)
{
   void* handle = stuff_alloc(stuff, sizeof(DataflowChainNode));
   DataflowChainNode *node = (DataflowChainNode *)stuff_out_lock(stuff, handle);

   node->dataflow = dataflow;
   node->unlinked = false;
   node->prev = chain->last;
   node->next = NULL;

   stuff_out_unlock(stuff, handle);

   if (!chain->first) chain->first = handle;
   if (chain->last)
   {
      ((DataflowChainNode *)stuff_out_lock(stuff, chain->last))->next = handle;
      stuff_out_unlock(stuff, chain->last);
   }
   chain->last = handle;
   chain->count++;
}

static Dataflow *copy(GLSL_COPY_CONTEXT_T *stuff, Dataflow *dataflow_in)
{
   Dataflow *result;
   Dataflow *dataflow;

   STACK_CHECK();

   dataflow = (Dataflow *)stuff_in_translate(stuff, dataflow_in);

   vcos_assert(dataflow);

   if (dataflow->flavour == stuff->input_flavour)
   {
      vcos_assert(dataflow->flavour == DATAFLOW_ATTRIBUTE || dataflow->flavour == DATAFLOW_VARYING);
      vcos_assert((uint32_t)dataflow->u.linkable_value.row < stuff->num_inputs);
      result = stuff->inputs[dataflow->u.linkable_value.row];
      vcos_assert(result);
   }
   else if (dataflow->copy && !stuff->revert)
   {
      vcos_assert(dataflow->copy != (Dataflow *)~0);
      result = dataflow->copy;
   }
   else if (!dataflow->copy && stuff->revert)
      return NULL;
   else
   {
      Dataflow df;
      DataflowChainNode *node;
      Dataflow *olddep;

      if (!stuff->revert) dataflow->copy = (Dataflow *)~0;

      df.flavour = dataflow->flavour;
      df.dependencies_count = dataflow->dependencies_count;
      df.line_num = dataflow->line_num;
      df.pass = DATAFLOW_PASS_INIT;
      df.bool_rep = dataflow->bool_rep;
      glsl_dataflow_clear_chains(&df);
      {
         int i;
         for (i=0; i<DATAFLOW_MAX_DEPENDENCIES; ++i) {
            df.d.dependencies[i] = NULL;
         }
      }

      result = (Dataflow *)stuff_alloc(stuff, sizeof(Dataflow));

      olddep = stuff->dependent;
      stuff->dependent = result;

      switch (dataflow->flavour)
      {
      case DATAFLOW_CONST_BOOL:
         df.u.const_bool.value = dataflow->u.const_bool.value;
         df.u.const_bool.index = dataflow->u.const_bool.index;
         break;
      case DATAFLOW_CONST_FLOAT:
         df.u.const_float.value = dataflow->u.const_float.value;
         df.u.const_float.index = dataflow->u.const_float.index;
         break;
      case DATAFLOW_CONST_SAMPLER:
         df.u.const_sampler.location = dataflow->u.const_sampler.location;
         df.u.const_sampler.name = NULL;
         break;
      case DATAFLOW_UNIFORM:
      case DATAFLOW_ATTRIBUTE:
      case DATAFLOW_VARYING:
         df.u.linkable_value.row = dataflow->u.linkable_value.row;
         df.u.linkable_value.name = NULL;
         break;
      case DATAFLOW_CONST_INT:
         df.u.const_int.value = dataflow->u.const_int.value;
         df.u.const_int.index = dataflow->u.const_int.index;
         break;
      case DATAFLOW_UNPACK_PLACEHOLDER_R:
      case DATAFLOW_UNPACK_PLACEHOLDER_B:
         if (stuff->texture_rb_swap == NULL || stuff->revert)
         {
            df.d.binary_op.left = copy(stuff, dataflow->d.binary_op.left);
            df.d.binary_op.right = copy(stuff, dataflow->d.binary_op.right);
         }
         else
         {
            bool swap;
            Dataflow *sampler = copy(stuff, dataflow->d.binary_op.right);
            int loc = sampler->u.const_sampler.location;

            vcos_assert(loc >= 0 && loc < GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS);

            swap = stuff->texture_rb_swap[loc];
            if ((dataflow->flavour == DATAFLOW_UNPACK_PLACEHOLDER_R) ^ swap)
               df.flavour = DATAFLOW_UNPACK_COL_R;
            else
               df.flavour = DATAFLOW_UNPACK_COL_B;
            df.d.unary_op.operand = copy(stuff, dataflow->d.binary_op.left);
            df.dependencies_count = 1;
         }
         break;
      case DATAFLOW_UNIFORM_ADDRESS:
         df.d.indexed_uniform_sampler.uniform = copy(stuff, dataflow->d.indexed_uniform_sampler.uniform);
         df.u.indexed_uniform_sampler.size = dataflow->u.indexed_uniform_sampler.size;
         break;
      default:
         /* general case: copy dependencies */
         {
            int i;
            for (i=0; i<dataflow->dependencies_count; ++i) {
               df.d.dependencies[i] = copy(stuff, dataflow->d.dependencies[i]);
            }
         }
      }

      stuff->dependent = NULL;   /* Do this before visiting iodependencies. Don't want to add anything as a dependent of iodependencies. */

      node = (DataflowChainNode *)stuff_in_translate(stuff, dataflow->iodependencies.first);
      while (node)
      {
         Dataflow *out_iodep = copy(stuff, node->dataflow);

         if (!stuff->revert)
         {
            stuff_chain_append(stuff, &df.iodependencies, out_iodep);

            if (stuff->mh_out_blob == MEM_INVALID_HANDLE)
            {
               glsl_dataflow_chain_append(&out_iodep->iodependents, result);
            }
         }

         node = (DataflowChainNode *)stuff_in_translate(stuff, node->next);
      }

      if (stuff->revert)
      {
         vcos_assert(dataflow->copy != NULL);
         dataflow->copy = NULL;
         return NULL;
      }

      vcos_assert(dataflow->copy == (Dataflow *)~0);
      dataflow->copy = result;

      init_backend_fields(&df);
      khrn_memcpy(stuff_out_lock(stuff, result), &df, sizeof(Dataflow));
      stuff_out_unlock(stuff, result);

      stuff->dependent = olddep;
   }

   if (stuff->mh_out_blob == MEM_INVALID_HANDLE && stuff->dependent)
   {
      glsl_dataflow_chain_append(&result->dependents, stuff->dependent);
   }

   return result;
}

MEM_HANDLE_T glsl_dataflow_copy_to_relocatable(uint32_t count, Dataflow **dataflow_out, Dataflow **dataflow_in, void *in_offset)
{
   GLSL_COPY_CONTEXT_T stuff;
   uint32_t i;

   stuff.dependent = NULL;
   stuff.inputs = NULL;
   stuff.num_inputs = 0;
   stuff.input_flavour = DATAFLOW_FLAVOUR_COUNT;
   stuff.mh_out_blob = mem_alloc_ex(1024, 4, MEM_FLAG_RESIZEABLE|MEM_FLAG_HINT_GROW, "GLSL_COPY_CONTEXT_T.mh_blob", MEM_COMPACT_DISCARD);

   if (stuff.mh_out_blob == MEM_INVALID_HANDLE)
      return MEM_INVALID_HANDLE;

   stuff.alloc_amount = 4;    /* waste some memory to avoid NULL being returned */
   stuff.in_offset = in_offset;
   stuff.revert = false;
   stuff.texture_rb_swap = NULL;

   for (i = 0; i < count; i++)
      dataflow_out[i] = copy(&stuff, dataflow_in[i]);

   stuff.revert = true;
   for (i = 0; i < count; i++)
      copy(&stuff, dataflow_in[i]);

   return stuff.mh_out_blob;
}

void glsl_dataflow_copy(uint32_t count, Dataflow **dataflow_out, Dataflow **dataflow_in, void *in_offset, Dataflow **inputs, uint32_t num_inputs, DataflowFlavour input_flavour, bool *texture_rb_swap)
{
   GLSL_COPY_CONTEXT_T stuff;
   uint32_t i;

   stuff.dependent = NULL;
   stuff.inputs = inputs;
   stuff.num_inputs = num_inputs;
   stuff.input_flavour = input_flavour;
   stuff.mh_out_blob = MEM_INVALID_HANDLE;
   stuff.alloc_amount = 0;
   stuff.in_offset = in_offset;
   stuff.revert = false;
   stuff.texture_rb_swap = texture_rb_swap;

   for (i = 0; i < count; i++)
   {
      if (dataflow_in[i] != NULL)
         dataflow_out[i] = copy(&stuff, dataflow_in[i]);
      else
         dataflow_out[i] = NULL;
   }

   stuff.revert = true;
   for (i = 0; i < count; i++)
      copy(&stuff, dataflow_in[i]);
}
