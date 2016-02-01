/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdlib.h>

#include "glsl_dataflow.h"
#include "glsl_fastmem.h"

#include "glsl_dataflow_simplify.h"

static uint32_t dataflow_count = 0;
static uint32_t dataflow_age = 0;

// Enum which specifies the retrun type of dataflow node in the type table
typedef enum {
   DF_RET_BOOL    = DF_BOOL,
   DF_RET_INT     = DF_INT,
   DF_RET_UINT    = DF_UINT,
   DF_RET_FLOAT   = DF_FLOAT,
   DF_RET_MATCH_ARG0,   // the type must match the type of argument 0
   DF_RET_MATCH_ARG1,   // the type must match the type of argument 1
   DF_RET_UNDEFINED     // type information is not available for this flavour
} DataflowReturnType;

// Bit-field which specifies the set of permited types for given dataflow argument
typedef enum {
   DF_ARG_BOOL       = (1 << DF_BOOL),
   DF_ARG_INT        = (1 << DF_INT),
   DF_ARG_UINT       = (1 << DF_UINT),
   DF_ARG_FLOAT      = (1 << DF_FLOAT),
   DF_ARG_BITWISE    = DF_ARG_INT | DF_ARG_UINT,
   DF_ARG_ARITH      = DF_ARG_FLOAT | DF_ARG_INT | DF_ARG_UINT,
   DF_ARG_ANY        = DF_ARG_FLOAT | DF_ARG_INT | DF_ARG_UINT | DF_ARG_BOOL,
   DF_ARG_MATCH_ARG0 = (1 << 30), // the type must match the type of argument 0
   DF_ARG_MATCH_ARG1 = (1 << 31)  // the type must match the type of argument 1
} DataflowArgumentTypes;

struct dataflow_op_info_s {
   DataflowFlavour         flavour; // for checking the table
   const char             *name;
   DataflowReturnType      return_type;
   int                     num_args;
   DataflowArgumentTypes   arg_types[3];
};

static const struct dataflow_op_info_s dataflow_info[DATAFLOW_FLAVOUR_COUNT] = {
   { DATAFLOW_CONST,               "const",                DF_RET_UNDEFINED   },
   { DATAFLOW_LOAD,                "load",                 DF_RET_UNDEFINED   },
   { DATAFLOW_PHI,                 "phi",                  DF_RET_UNDEFINED   },
   { DATAFLOW_EXTERNAL,            "external",             DF_RET_UNDEFINED   },
   { DATAFLOW_LOGICAL_NOT,         "logical_not",          DF_RET_BOOL,       1, { DF_ARG_BOOL } },
   { DATAFLOW_CONST_SAMPLER,       "const_sampler",        DF_RET_UNDEFINED   },
   { DATAFLOW_FTOI_TRUNC,          "ftoi_trunc",           DF_RET_INT,        1, { DF_ARG_FLOAT } },
   { DATAFLOW_FTOI_NEAREST,        "ftoi_nearest",         DF_RET_INT,        1, { DF_ARG_FLOAT } },
   { DATAFLOW_FTOU,                "ftou",                 DF_RET_UINT,       1, { DF_ARG_FLOAT } },

   { DATAFLOW_BITWISE_NOT,         "bitwise_not",          DF_RET_MATCH_ARG0, 1, { DF_ARG_BITWISE } },
   { DATAFLOW_BITWISE_AND,         "bitwise_and",          DF_RET_MATCH_ARG0, 2, { DF_ARG_BITWISE, DF_ARG_BITWISE } },
   { DATAFLOW_BITWISE_OR,          "bitwise_or",           DF_RET_MATCH_ARG0, 2, { DF_ARG_BITWISE, DF_ARG_BITWISE } },
   { DATAFLOW_BITWISE_XOR,         "bitwise_xor",          DF_RET_MATCH_ARG0, 2, { DF_ARG_BITWISE, DF_ARG_BITWISE } },

   { DATAFLOW_SHL,                 "shl",                  DF_RET_MATCH_ARG0, 2, { DF_ARG_BITWISE, DF_ARG_BITWISE } },
   { DATAFLOW_SHR,                 "shr",                  DF_RET_MATCH_ARG0, 2, { DF_ARG_BITWISE, DF_ARG_BITWISE } },

   { DATAFLOW_ADDRESS_LOAD,        "address_load",         DF_RET_UNDEFINED   },
   { DATAFLOW_VECTOR_LOAD,         "vector_load",          DF_RET_UNDEFINED   },
   { DATAFLOW_UNIFORM,             "uniform",              DF_RET_UNDEFINED   },
   { DATAFLOW_UNIFORM_BUFFER,      "uniform_buffer",       DF_RET_UNDEFINED   },
   { DATAFLOW_STORAGE_BUFFER,      "storage_buffer",       DF_RET_UNDEFINED   },
   { DATAFLOW_ATOMIC_COUNTER,      "atomic_uint",          DF_RET_UNDEFINED   },
   { DATAFLOW_IN,                  "in",                   DF_RET_UNDEFINED   },
   { DATAFLOW_MUL,                 "mul",                  DF_RET_MATCH_ARG0, 2, { DF_ARG_ARITH, DF_ARG_MATCH_ARG0 } },
   { DATAFLOW_DIV,                 "div",                  DF_RET_MATCH_ARG0, 2, { DF_ARG_ARITH, DF_ARG_MATCH_ARG0 } },
   { DATAFLOW_REM,                 "rem",                  DF_RET_MATCH_ARG0, 2, { (DF_ARG_INT | DF_ARG_UINT), DF_ARG_MATCH_ARG0 } },
   { DATAFLOW_ADD,                 "add",                  DF_RET_MATCH_ARG0, 2, { DF_ARG_ARITH, DF_ARG_MATCH_ARG0 } },
   { DATAFLOW_SUB,                 "sub",                  DF_RET_MATCH_ARG0, 2, { DF_ARG_ARITH, DF_ARG_MATCH_ARG0 } },
   { DATAFLOW_ARITH_NEGATE,        "arith_negate",         DF_RET_MATCH_ARG0, 1, { DF_ARG_ARITH } },
   { DATAFLOW_LESS_THAN,           "less_than",            DF_RET_BOOL,       2, { DF_ARG_ARITH, DF_ARG_MATCH_ARG0 } },
   { DATAFLOW_LESS_THAN_EQUAL,     "less_than_equal",      DF_RET_BOOL,       2, { DF_ARG_ARITH, DF_ARG_MATCH_ARG0 } },
   { DATAFLOW_GREATER_THAN,        "greater_than",         DF_RET_BOOL,       2, { DF_ARG_ARITH, DF_ARG_MATCH_ARG0 } },
   { DATAFLOW_GREATER_THAN_EQUAL,  "greater_than_equal",   DF_RET_BOOL,       2, { DF_ARG_ARITH, DF_ARG_MATCH_ARG0 } },
   { DATAFLOW_EQUAL,               "equal",                DF_RET_BOOL,       2, { DF_ARG_ANY, DF_ARG_MATCH_ARG0 }   },
   { DATAFLOW_NOT_EQUAL,           "not_equal",            DF_RET_BOOL,       2, { DF_ARG_ANY, DF_ARG_MATCH_ARG0 }   },
   { DATAFLOW_LOGICAL_AND,         "logical_and",          DF_RET_BOOL,       2, { DF_ARG_BOOL, DF_ARG_BOOL } },
   { DATAFLOW_LOGICAL_XOR,         "logical_xor",          DF_RET_BOOL,       2, { DF_ARG_BOOL, DF_ARG_BOOL } },
   { DATAFLOW_LOGICAL_OR,          "logical_or",           DF_RET_BOOL,       2, { DF_ARG_BOOL, DF_ARG_BOOL } },
   { DATAFLOW_CONDITIONAL,         "conditional",          DF_RET_MATCH_ARG1, 3, { DF_ARG_BOOL, DF_ARG_ANY, DF_ARG_MATCH_ARG1 } },
   { DATAFLOW_RSQRT,               "rsqrt",                DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
   { DATAFLOW_RCP,                 "rcp",                  DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
   { DATAFLOW_LOG2,                "log2",                 DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
   { DATAFLOW_EXP2,                "exp2",                 DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
   { DATAFLOW_SIN,                 "sin",                  DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
   { DATAFLOW_COS,                 "cos",                  DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
   { DATAFLOW_TAN,                 "tan",                  DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
   { DATAFLOW_MIN,                 "min",                  DF_RET_MATCH_ARG0, 2, { DF_ARG_ANY, DF_ARG_MATCH_ARG0 } },
   { DATAFLOW_MAX,                 "max",                  DF_RET_MATCH_ARG0, 2, { DF_ARG_ANY, DF_ARG_MATCH_ARG0 } },
   { DATAFLOW_TRUNC,               "trunc",                DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
   { DATAFLOW_NEAREST,             "nearest",              DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
   { DATAFLOW_CEIL,                "ceil",                 DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
   { DATAFLOW_FLOOR,               "floor",                DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
   { DATAFLOW_ABS,                 "abs",                  DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
   { DATAFLOW_FDX,                 "fdx",                  DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
   { DATAFLOW_FDY,                 "fdy",                  DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
   { DATAFLOW_REINTERP,            "reinterp",             DF_RET_UNDEFINED   },
   { DATAFLOW_FPACK,               "fpack",                DF_RET_UINT,       2, { DF_ARG_FLOAT, DF_ARG_FLOAT } },
   { DATAFLOW_FUNPACKA,            "funpacka",             DF_RET_FLOAT,      1, { DF_ARG_UINT }  },
   { DATAFLOW_FUNPACKB,            "funpackb",             DF_RET_FLOAT,      1, { DF_ARG_UINT }  },

   { DATAFLOW_ITOF,                "itof",                 DF_RET_FLOAT,      1, { DF_ARG_INT }   },
   { DATAFLOW_UTOF,                "utof",                 DF_RET_FLOAT,      1, { DF_ARG_UINT }  },
   { DATAFLOW_CLZ,                 "clz",                  DF_RET_UINT,       1, { DF_ARG_UINT }  },

   { DATAFLOW_VEC4,                "vec4",                 DF_RET_UNDEFINED   },
   { DATAFLOW_TEXTURE,             "texture",              DF_RET_UNDEFINED   },
   { DATAFLOW_TEXTURE_SIZE,        "texture_size",         DF_RET_UNDEFINED   },
   { DATAFLOW_GET_VEC4_COMPONENT,  "get_vec4_component",   DF_RET_UNDEFINED   },

   { DATAFLOW_FRAG_GET_COL,        "frag_get_col",         DF_RET_UNDEFINED   },

   { DATAFLOW_FRAG_GET_X,          "frag_get_x",           DF_RET_FLOAT,      0 },
   { DATAFLOW_FRAG_GET_Y,          "frag_get_y",           DF_RET_FLOAT,      0 },
   { DATAFLOW_FRAG_GET_X_UINT,     "frag_get_x_uint",      DF_RET_UINT,       0 },
   { DATAFLOW_FRAG_GET_Y_UINT,     "frag_get_y_uint",      DF_RET_UINT,       0 },
   { DATAFLOW_FRAG_GET_Z,          "frag_get_z",           DF_RET_FLOAT,      0 },
   { DATAFLOW_FRAG_GET_W,          "frag_get_w",           DF_RET_FLOAT,      0 },
   { DATAFLOW_FRAG_GET_FF,         "frag_get_ff",          DF_RET_BOOL,       0 },

   { DATAFLOW_GET_THREAD_INDEX,    "get_thread_index",     DF_RET_UINT,       0 },

   { DATAFLOW_IS_HELPER,           "is_helper",            DF_RET_BOOL,       0 },

   { DATAFLOW_GET_VERTEX_ID,       "get_vertex_id",        DF_RET_INT,        0 },
   { DATAFLOW_GET_INSTANCE_ID,     "get_instance_id",      DF_RET_INT,        0 },
   { DATAFLOW_GET_POINT_COORD_X,   "get_point_coord_x",    DF_RET_FLOAT,      0 },
   { DATAFLOW_GET_POINT_COORD_Y,   "get_point_coord_y",    DF_RET_FLOAT,      0 },
   { DATAFLOW_GET_LINE_COORD,      "get_line_coord",       DF_RET_FLOAT,      0 },
   { DATAFLOW_GET_DEPTHRANGE_NEAR, "get_depthrange_near",  DF_RET_FLOAT,      0 },
   { DATAFLOW_GET_DEPTHRANGE_FAR,  "get_depthrange_far",   DF_RET_FLOAT,      0 },
   { DATAFLOW_GET_DEPTHRANGE_DIFF, "get_depthrange_diff",  DF_RET_FLOAT,      0 },

   { DATAFLOW_ADDRESS,             "address",              DF_RET_UNDEFINED   },
};

void glsl_dataflow_reset_count() {
   dataflow_count = 0;
}

int glsl_dataflow_get_count(void) {
   return dataflow_count;
}

int glsl_dataflow_get_next_id(void) {
   return dataflow_count++;
}

void glsl_dataflow_reset_age() {
   dataflow_age = 0;
}

static ArenaAlloc *s_DataflowArena = NULL;

void glsl_dataflow_begin_construction() {
   s_DataflowArena = glsl_arena_create(64*1024);
}

void glsl_dataflow_end_construction() {
   glsl_arena_free(s_DataflowArena);
   s_DataflowArena = NULL;
}

void *glsl_dataflow_malloc(size_t size) {
   return glsl_arena_calloc(s_DataflowArena, 1, size);
}

void glsl_dataflow_set_age(int age) {
   dataflow_age = age;
}

//
// Dataflow chain functions.
//

void glsl_dataflow_chain_init(DataflowChain *chain)
{
   glsl_list_DataflowChainNode_init(chain);
}

DataflowChain *glsl_dataflow_chain_append(DataflowChain *chain, Dataflow *dataflow)
{
   glsl_node_list_Dataflow_append(chain, dataflow);
   return chain;
}

DataflowChain *glsl_dataflow_chain_remove_node(DataflowChain *chain, DataflowChainNode *node)
{
   glsl_list_DataflowChainNode_remove(chain, node);
   return chain; /* Why? */
}

static Dataflow *dataflow_construct_common(DataflowFlavour flavour, DataflowType type)
{
   Dataflow *dataflow = glsl_arena_malloc(s_DataflowArena, sizeof(*dataflow));
   dataflow->id = glsl_dataflow_get_next_id();
   dataflow->flavour = flavour;
   dataflow->type = type;
   dataflow->age = dataflow_age++;
   memset(&dataflow->d, 0, sizeof(dataflow->d));
   memset(&dataflow->u, 0, sizeof(dataflow->u));
   return dataflow;
}

//
// Dataflow constructors.
//

Dataflow *glsl_dataflow_construct_const_value(DataflowType type, const_value value)
{
   assert(type==DF_BOOL || type==DF_INT || type==DF_UINT || type==DF_FLOAT);

   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_CONST, type);
   dataflow->u.constant.value = value;
   dataflow->dependencies_count = 0;
   return dataflow;
}

Dataflow *glsl_dataflow_construct_const_bool(bool value)
{
   return glsl_dataflow_construct_const_value(DF_BOOL, value ? 1 : 0);
}

Dataflow *glsl_dataflow_construct_const_int(int value)
{
   return glsl_dataflow_construct_const_value(DF_INT, value);
}

Dataflow *glsl_dataflow_construct_const_uint(unsigned value)
{
   return glsl_dataflow_construct_const_value(DF_UINT, value);
}

Dataflow *glsl_dataflow_construct_const_float(float value)
{
   return glsl_dataflow_construct_const_value(DF_FLOAT, float_to_bits(value));
}

Dataflow *glsl_dataflow_construct_load(DataflowType type)
{
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_LOAD, type);
   dataflow->dependencies_count = 0;
   return dataflow;
}

Dataflow *glsl_dataflow_construct_const_sampler(DataflowType type, bool is_32bit)
{
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_CONST_SAMPLER, type);
   assert(type == DF_FSAMPLER || type == DF_ISAMPLER || type == DF_USAMPLER ||
          type == DF_FIMAGE   || type == DF_IIMAGE   || type == DF_UIMAGE);
   dataflow->u.const_sampler.location = SAMPLER_LOCATION_UNDEFINED;
   dataflow->u.const_sampler.is_32bit = is_32bit;
   dataflow->dependencies_count = 0;
   return dataflow;
}

Dataflow *glsl_dataflow_construct_linkable_value(DataflowFlavour flavour, DataflowType type)
{
   Dataflow *dataflow = dataflow_construct_common(flavour, type);
   dataflow->u.linkable_value.row = LINKABLE_VALUE_ROW_UNDEFINED;
   dataflow->dependencies_count = 0;
   return dataflow;
}

Dataflow *glsl_dataflow_construct_vector_load(DataflowType type, Dataflow *address) {
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_VECTOR_LOAD, type);
   dataflow->d.unary_op.operand = address;
   dataflow->dependencies_count = 1;
   dataflow->u.vector_load.required_components = 0;
   return dataflow;
}

Dataflow *glsl_dataflow_construct_address_load(DataflowType type, Dataflow *address)
{
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_ADDRESS_LOAD, type);
   dataflow->d.unary_op.operand = address;
   dataflow->dependencies_count = 1;
   return dataflow;
}

Dataflow *glsl_dataflow_construct_reinterp(Dataflow *operand, DataflowType new_type)
{
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_REINTERP, new_type);
   dataflow->dependencies_count = 1;
   dataflow->d.unary_op.operand = operand;

   Dataflow *simplified = glsl_dataflow_simplify(dataflow);
   if(simplified) {
      return simplified;
   }

   return dataflow;
}

const char *glsl_dataflow_info_get_name(DataflowFlavour flavour)
{
   assert(dataflow_info[flavour].flavour == flavour);
   return dataflow_info[flavour].name;
}

#ifndef NDEBUG
static void dataflow_op_verify(DataflowFlavour flavour, int num_args, Dataflow **arguments) {
   assert(dataflow_info[flavour].flavour == flavour);
   assert(dataflow_info[flavour].return_type != DF_RET_UNDEFINED);
   assert(dataflow_info[flavour].num_args == num_args);
   /* Check all the args have appropriate types */
   for(int i=0; i<num_args; i++) {
      const DataflowArgumentTypes seen_type = (1 << arguments[i]->type);
      const DataflowArgumentTypes valid_types = dataflow_info[flavour].arg_types[i];
      if (valid_types == DF_ARG_MATCH_ARG0) {
         assert(arguments[i]->type == arguments[0]->type && i != 0);
      } else if (valid_types == DF_ARG_MATCH_ARG1) {
         assert(arguments[i]->type == arguments[1]->type);
      } else {
         assert(seen_type & valid_types);
      }
   }
}
#endif

static DataflowType dataflow_op_type(DataflowFlavour flavour, int num_args, Dataflow **arguments) {
   DataflowReturnType ret_flags = dataflow_info[flavour].return_type;
   switch(ret_flags) {
   case DF_RET_FLOAT:      return DF_FLOAT;
   case DF_RET_INT:        return DF_INT;
   case DF_RET_UINT:       return DF_UINT;
   case DF_RET_BOOL:       return DF_BOOL;
   case DF_RET_MATCH_ARG0: return arguments[0]->type;
   case DF_RET_MATCH_ARG1: return arguments[1]->type;
   default:
      UNREACHABLE();
      return DF_INVALID;
   }
}

static Dataflow *dataflow_op_build(DataflowFlavour flavour, int num_args, Dataflow **arguments) {
   const DataflowType type = dataflow_op_type(flavour, num_args, arguments);

   Dataflow *dataflow = dataflow_construct_common(flavour, type);

   dataflow->dependencies_count = num_args;
   for(int i=0; i<num_args; i++) {
      dataflow->d.dependencies[i] = arguments[i];
   }
   return dataflow;
}

Dataflow *glsl_dataflow_construct_op(DataflowFlavour flavour, int num_args, Dataflow **arguments) {
#ifndef NDEBUG
   dataflow_op_verify(flavour, num_args, arguments);
#endif
   Dataflow *dataflow   = dataflow_op_build(flavour, num_args, arguments);
   Dataflow *simplified = glsl_dataflow_simplify(dataflow);
   if(simplified) {
      return simplified;
   }
   return dataflow;
}

Dataflow *glsl_dataflow_construct_nullary_op(DataflowFlavour flavour) {
   return glsl_dataflow_construct_op(flavour, 0, NULL);
}

Dataflow *glsl_dataflow_construct_unary_op(DataflowFlavour flavour, Dataflow *operand) {
   return glsl_dataflow_construct_op(flavour, 1, &operand);
}

Dataflow *glsl_dataflow_construct_binary_op(DataflowFlavour flavour, Dataflow *left, Dataflow *right)
{
   Dataflow *dependents[2];
   dependents[0] = left;
   dependents[1] = right;
   return glsl_dataflow_construct_op(flavour, 2, dependents);
}

Dataflow *glsl_dataflow_construct_ternary_op(DataflowFlavour flavour, Dataflow *first, Dataflow *second, Dataflow *third)
{
   Dataflow *dependents[3];
   dependents[0] = first;
   dependents[1] = second;
   dependents[2] = third;
   return glsl_dataflow_construct_op(flavour, 3, dependents);
}

Dataflow *glsl_dataflow_construct_address(Dataflow *operand)
{
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_ADDRESS, DF_INT);

   dataflow->d.unary_op.operand = operand;
   dataflow->dependencies_count = 1;

   return dataflow;
}

Dataflow *glsl_dataflow_convert_type(Dataflow *input, DataflowType out_type)
{
   if (out_type == input->type)
      return input;

   switch (input->type)
   {
      case DF_BOOL:
         switch (out_type)
         {
            case DF_INT:
            case DF_UINT:
               return glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL,
                                                         input,
                                                         glsl_dataflow_construct_const_value(out_type, 1),
                                                         glsl_dataflow_construct_const_value(out_type, 0));
            case DF_FLOAT:
               return glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL,
                                                         input,
                                                         glsl_dataflow_construct_const_float(1.0),
                                                         glsl_dataflow_construct_const_float(0.0));
            default:
               UNREACHABLE();
               return NULL;
         }

      case DF_INT:
         switch (out_type)
         {
            case DF_BOOL:
               return glsl_dataflow_construct_binary_op(DATAFLOW_NOT_EQUAL, input, glsl_dataflow_construct_const_int(0));
            case DF_UINT:
               return glsl_dataflow_construct_reinterp(input, out_type);
            case DF_FLOAT:
               return glsl_dataflow_construct_unary_op(DATAFLOW_ITOF, input);
            default:
               UNREACHABLE();
               return NULL;
         }

      case DF_UINT:
         switch (out_type)
         {
            case DF_BOOL:
               return glsl_dataflow_construct_binary_op(DATAFLOW_NOT_EQUAL, input, glsl_dataflow_construct_const_uint(0));
            case DF_INT:
               return glsl_dataflow_construct_reinterp(input, out_type);
            case DF_FLOAT:
               return glsl_dataflow_construct_unary_op(DATAFLOW_UTOF, input);
            default:
               UNREACHABLE();
               return NULL;
         }

      case DF_FLOAT:
         switch (out_type)
         {
            case DF_BOOL:
               return glsl_dataflow_construct_binary_op(DATAFLOW_NOT_EQUAL,
                                                      input, glsl_dataflow_construct_const_float(0.0));
            case DF_INT:
               return glsl_dataflow_construct_unary_op(DATAFLOW_FTOI_TRUNC, input);
            case DF_UINT:
               return glsl_dataflow_construct_unary_op(DATAFLOW_FTOU, input);
            default:
               UNREACHABLE();
               return NULL;
         }

      default:
         UNREACHABLE();
         return NULL;
   }
}

Dataflow *glsl_dataflow_construct_get_vec4_component(uint32_t component_index,
                                                     Dataflow *param,
                                                     DataflowType type)
{
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_GET_VEC4_COMPONENT, type);

   dataflow->dependencies_count = 1;
   dataflow->d.unary_op.operand = param;
   dataflow->u.get_vec4_component.component_index = component_index;

   return dataflow;
}

void glsl_dataflow_construct_frag_get_col(Dataflow **out, DataflowType type,
                                          uint32_t required_components,
                                          int render_target)
{
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_FRAG_GET_COL, type);

   dataflow->dependencies_count = 0;
   dataflow->u.get_col.required_components = required_components;
   dataflow->u.get_col.render_target = render_target;

   for (int i=0; i<4; i++)
      out[i] = glsl_dataflow_construct_get_vec4_component(i, dataflow, type);
}


Dataflow *glsl_dataflow_construct_vec4(Dataflow *r, Dataflow *g, Dataflow *b, Dataflow *a) {
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_VEC4, DF_FLOAT);
   dataflow->dependencies_count = 4;
   dataflow->d.dependencies[0] = r;
   dataflow->d.dependencies[1] = g;
   dataflow->d.dependencies[2] = b;
   dataflow->d.dependencies[3] = a;
   return dataflow;
}

void glsl_dataflow_construct_texture_gadget(Dataflow **r_out, Dataflow **g_out,
                                            Dataflow **b_out, Dataflow **a_out,
                                            uint32_t bits, Dataflow *sampler, Dataflow *coords,
                                            Dataflow *d, Dataflow *b,
                                            uint32_t required_components, DataflowType component_type_index)
{
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_TEXTURE, DF_VOID);

   dataflow->dependencies_count = 4;

   dataflow->d.texture.coords = coords;
   dataflow->d.texture.d = d;
   dataflow->d.texture.b = b;
   dataflow->d.texture.sampler = sampler;

   dataflow->u.texture.required_components = required_components;
   dataflow->u.texture.bits = bits;

   if (r_out) *r_out = glsl_dataflow_construct_get_vec4_component(0, dataflow, component_type_index);
   if (g_out) *g_out = glsl_dataflow_construct_get_vec4_component(1, dataflow, component_type_index);
   if (b_out) *b_out = glsl_dataflow_construct_get_vec4_component(2, dataflow, component_type_index);
   if (a_out) *a_out = glsl_dataflow_construct_get_vec4_component(3, dataflow, component_type_index);
}

Dataflow *glsl_dataflow_construct_texture_size(Dataflow *sampler)
{
   Dataflow *ret = dataflow_construct_common(DATAFLOW_TEXTURE_SIZE, DF_VOID);
   ret->dependencies_count = 1;
   ret->d.texture_size.sampler = sampler;

   return ret;
}

Dataflow *glsl_dataflow_construct_external(DataflowType t) {
   Dataflow *d = dataflow_construct_common(DATAFLOW_EXTERNAL, t);
   d->dependencies_count = 0;
   return d;
}

/* TODO: This is just a binary op, but we erroneously PHI samplers together
 *       and the op code doesn't deal with opaque types. FIXME             */
Dataflow *glsl_dataflow_construct_phi(Dataflow *a, Dataflow *b) {
   assert(a->type == b->type);
   Dataflow *d = dataflow_construct_common(DATAFLOW_PHI, a->type);
   d->dependencies_count = 2;
   d->d.binary_op.left = a;
   d->d.binary_op.right = b;
   return d;
}
