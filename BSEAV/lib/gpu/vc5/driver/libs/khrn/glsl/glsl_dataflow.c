/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "glsl_dataflow.h"
#include "glsl_fastmem.h"
#include "glsl_dataflow_simplify.h"

#include "libs/util/gfx_util/gfx_util.h"

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
   { DATAFLOW_ROR,                 "ror",                  DF_RET_MATCH_ARG0, 2, { DF_ARG_BITWISE, DF_ARG_BITWISE } },

   { DATAFLOW_ADDRESS_STORE,       "address_store",        DF_RET_UNDEFINED   },
   { DATAFLOW_VECTOR_LOAD,         "vector_load",          DF_RET_UNDEFINED   },
   { DATAFLOW_ATOMIC_ADD,          "atomic_add",           DF_RET_UNDEFINED   },
   { DATAFLOW_ATOMIC_SUB,          "atomic_sub",           DF_RET_UNDEFINED   },
   { DATAFLOW_ATOMIC_MIN,          "atomic_min",           DF_RET_UNDEFINED   },
   { DATAFLOW_ATOMIC_MAX,          "atomic_max",           DF_RET_UNDEFINED   },
   { DATAFLOW_ATOMIC_AND,          "atomic_and",           DF_RET_UNDEFINED   },
   { DATAFLOW_ATOMIC_OR,           "atomic_or",            DF_RET_UNDEFINED   },
   { DATAFLOW_ATOMIC_XOR,          "atomic_xor",           DF_RET_UNDEFINED   },
   { DATAFLOW_ATOMIC_XCHG,         "atomic_xchg",          DF_RET_UNDEFINED   },
   { DATAFLOW_ATOMIC_CMPXCHG,      "atomic_cmpxchg",       DF_RET_UNDEFINED   },
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
   { DATAFLOW_SQRT,                "sqrt",                 DF_RET_FLOAT,      1, { DF_ARG_FLOAT } },
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
#if V3D_VER_AT_LEAST(4,0,2,0)
   { DATAFLOW_TEXTURE_ADDR,        "texture_addr",         DF_RET_UNDEFINED   },
#endif
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
   { DATAFLOW_SHARED_PTR,          "shared_ptr",           DF_RET_UINT,       0 },

   { DATAFLOW_IS_HELPER,           "is_helper",            DF_RET_BOOL,       0 },
   { DATAFLOW_SAMPLE_POS_X,        "samp_pos_x",           DF_RET_FLOAT,      0 },
   { DATAFLOW_SAMPLE_POS_Y,        "samp_pos_y",           DF_RET_FLOAT,      0 },
   { DATAFLOW_SAMPLE_MASK,         "sample_mask",          DF_RET_INT,        0 },
   { DATAFLOW_SAMPLE_ID,           "sample_id",            DF_RET_INT,        0 },
   { DATAFLOW_NUM_SAMPLES,         "num_samples",          DF_RET_INT,        0 },

   { DATAFLOW_GET_VERTEX_ID,       "get_vertex_id",        DF_RET_INT,        0 },
   { DATAFLOW_GET_INSTANCE_ID,     "get_instance_id",      DF_RET_INT,        0 },
   { DATAFLOW_GET_BASE_INSTANCE,   "get_base_instance",    DF_RET_INT,        0 },
   { DATAFLOW_GET_POINT_COORD_X,   "get_point_coord_x",    DF_RET_FLOAT,      0 },
   { DATAFLOW_GET_POINT_COORD_Y,   "get_point_coord_y",    DF_RET_FLOAT,      0 },
   { DATAFLOW_GET_LINE_COORD,      "get_line_coord",       DF_RET_FLOAT,      0 },
   { DATAFLOW_GET_DEPTHRANGE_NEAR, "get_depthrange_near",  DF_RET_FLOAT,      0 },
   { DATAFLOW_GET_DEPTHRANGE_FAR,  "get_depthrange_far",   DF_RET_FLOAT,      0 },
   { DATAFLOW_GET_DEPTHRANGE_DIFF, "get_depthrange_diff",  DF_RET_FLOAT,      0 },
   { DATAFLOW_GET_NUMWORKGROUPS_X, "get_numworkgroups_x",  DF_RET_UINT,       0 },
   { DATAFLOW_GET_NUMWORKGROUPS_Y, "get_numworkgroups_y",  DF_RET_UINT,       0 },
   { DATAFLOW_GET_NUMWORKGROUPS_Z, "get_numworkgroups_z",  DF_RET_UINT,       0 },

   { DATAFLOW_GET_INVOCATION_ID,   "get_invocation_id",    DF_RET_INT,        0 },

   { DATAFLOW_ADDRESS,             "address",              DF_RET_UNDEFINED   },
   { DATAFLOW_BUF_SIZE,            "buf_size",             DF_RET_UNDEFINED   },
   { DATAFLOW_IMAGE_INFO_PARAM,    "image_info_param",     DF_RET_UNDEFINED   },
   { DATAFLOW_TEXBUFFER_INFO_PARAM, "texbuffer_info_param", DF_RET_UNDEFINED  },
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

void glsl_dataflow_chain_init(DataflowChain *chain) {
   glsl_list_DataflowChainNode_init(chain);
}

DataflowChain *glsl_dataflow_chain_append(DataflowChain *chain, Dataflow *dataflow) {
   glsl_node_list_Dataflow_append(chain, dataflow);
   return chain;
}

DataflowChain *glsl_dataflow_chain_remove_node(DataflowChain *chain, DataflowChainNode *node) {
   glsl_list_DataflowChainNode_remove(chain, node);
   return chain; /* Why? */
}

static Dataflow *dataflow_construct_common(DataflowFlavour flavour, DataflowType type) {
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

Dataflow *glsl_dataflow_construct_const_value(DataflowType type, const_value value) {
   assert(type==DF_BOOL || type==DF_INT || type==DF_UINT || type==DF_FLOAT);

   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_CONST, type);
   dataflow->u.constant.value = value;
   dataflow->dependencies_count = 0;
   return dataflow;
}

Dataflow *glsl_dataflow_construct_const_bool(bool value) {
   return glsl_dataflow_construct_const_value(DF_BOOL, value ? 1 : 0);
}

Dataflow *glsl_dataflow_construct_const_int(int value) {
   return glsl_dataflow_construct_const_value(DF_INT, value);
}

Dataflow *glsl_dataflow_construct_const_uint(unsigned value) {
   return glsl_dataflow_construct_const_value(DF_UINT, value);
}

Dataflow *glsl_dataflow_construct_const_float(float value) {
   return glsl_dataflow_construct_const_value(DF_FLOAT, gfx_float_to_bits(value));
}

Dataflow *glsl_dataflow_construct_load(DataflowType type) {
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_LOAD, type);
   dataflow->dependencies_count = 0;
   return dataflow;
}

Dataflow *glsl_dataflow_construct_const_sampler(DataflowType type, const_value location, bool is_32bit) {
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_CONST_SAMPLER, type);
   assert(type == DF_FSAMPLER || type == DF_ISAMPLER || type == DF_USAMPLER ||
          type == DF_FIMAGE   || type == DF_IIMAGE   || type == DF_UIMAGE);
   dataflow->u.const_sampler.location = location;
   dataflow->u.const_sampler.is_32bit = is_32bit;
   dataflow->u.const_sampler.format_valid = false;
   dataflow->dependencies_count = 0;
   return dataflow;
}

Dataflow *glsl_dataflow_construct_linkable_value(DataflowFlavour flavour, DataflowType type, const_value row) {
   Dataflow *dataflow = dataflow_construct_common(flavour, type);
   dataflow->u.linkable_value.row = row;
   dataflow->dependencies_count = 0;
   return dataflow;
}

Dataflow *glsl_dataflow_construct_buffer(DataflowFlavour flavour, DataflowType type, const_value index, const_value offset) {
   Dataflow *dataflow = dataflow_construct_common(flavour, type);
   dataflow->u.buffer.index  = index;
   dataflow->u.buffer.offset = offset;
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

Dataflow *glsl_dataflow_construct_atomic(DataflowFlavour flavour, DataflowType type, Dataflow *address, Dataflow *arg,
      Dataflow *cond, Dataflow *prev)
{
   Dataflow *dataflow = dataflow_construct_common(flavour, type);
   dataflow->d.addr_store.addr  = address;
   dataflow->d.addr_store.val   = arg;
   dataflow->d.addr_store.cond  = cond;
   dataflow->d.addr_store.prev  = prev;
   dataflow->dependencies_count = 4;
   return dataflow;
}

Dataflow *glsl_dataflow_construct_reinterp(Dataflow *operand, DataflowType new_type) {
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_REINTERP, new_type);
   dataflow->dependencies_count = 1;
   dataflow->d.unary_op.operand = operand;

   Dataflow *simplified = glsl_dataflow_simplify(dataflow);
   if(simplified) {
      return simplified;
   }

   return dataflow;
}

const char *glsl_dataflow_info_get_name(DataflowFlavour flavour) {
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

static DataflowType dataflow_op_type(DataflowFlavour flavour, Dataflow **arguments) {
   DataflowReturnType ret_flags = dataflow_info[flavour].return_type;
   switch(ret_flags) {
   case DF_RET_FLOAT:      return DF_FLOAT;
   case DF_RET_INT:        return DF_INT;
   case DF_RET_UINT:       return DF_UINT;
   case DF_RET_BOOL:       return DF_BOOL;
   case DF_RET_MATCH_ARG0: return arguments[0]->type;
   case DF_RET_MATCH_ARG1: return arguments[1]->type;
   default:
      unreachable();
      return DF_INVALID;
   }
}

static Dataflow *dataflow_op_build(DataflowFlavour flavour, int num_args, Dataflow **arguments) {
   const DataflowType type = dataflow_op_type(flavour, arguments);

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

Dataflow *glsl_dataflow_construct_binary_op(DataflowFlavour flavour, Dataflow *left, Dataflow *right) {
   Dataflow *dependents[2];
   dependents[0] = left;
   dependents[1] = right;
   return glsl_dataflow_construct_op(flavour, 2, dependents);
}

Dataflow *glsl_dataflow_construct_ternary_op(DataflowFlavour flavour, Dataflow *first, Dataflow *second, Dataflow *third) {
   Dataflow *dependents[3];
   dependents[0] = first;
   dependents[1] = second;
   dependents[2] = third;
   return glsl_dataflow_construct_op(flavour, 3, dependents);
}

Dataflow *glsl_dataflow_construct_address(Dataflow *operand) {
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_ADDRESS, DF_UINT);
   dataflow->d.unary_op.operand = operand;
   dataflow->dependencies_count = 1;
   return dataflow;
}

Dataflow *glsl_dataflow_construct_buf_size(Dataflow *operand) {
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_BUF_SIZE, DF_UINT);
   dataflow->d.unary_op.operand = operand;
   dataflow->dependencies_count = 1;
   return dataflow;
}

Dataflow *glsl_dataflow_convert_type(Dataflow *input, DataflowType out_type) {
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
               unreachable();
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
               unreachable();
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
               unreachable();
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
               unreachable();
               return NULL;
         }

      default:
         unreachable();
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
                                            Dataflow *d, Dataflow *b, Dataflow *off,
                                            uint32_t required_components, DataflowType component_type_index)
{
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_TEXTURE, DF_VOID);

   dataflow->dependencies_count = 5;

   dataflow->d.texture.coords = coords;
   dataflow->d.texture.d = d;
   dataflow->d.texture.b = b;
   dataflow->d.texture.off = off;
   dataflow->d.texture.sampler = sampler;

   dataflow->u.texture.required_components = required_components;
   dataflow->u.texture.bits = bits;

   if (r_out) *r_out = glsl_dataflow_construct_get_vec4_component(0, dataflow, component_type_index);
   if (g_out) *g_out = glsl_dataflow_construct_get_vec4_component(1, dataflow, component_type_index);
   if (b_out) *b_out = glsl_dataflow_construct_get_vec4_component(2, dataflow, component_type_index);
   if (a_out) *a_out = glsl_dataflow_construct_get_vec4_component(3, dataflow, component_type_index);
}

#if V3D_VER_AT_LEAST(4,0,2,0)
Dataflow *glsl_dataflow_construct_texture_addr(Dataflow *sampler,
                                               Dataflow *x, Dataflow *y, Dataflow *z,
                                               Dataflow *i)
{
   Dataflow *dataflow = dataflow_construct_common(DATAFLOW_TEXTURE_ADDR, DF_VOID);

   dataflow->dependencies_count = 5;

   dataflow->d.texture_addr.x = x;
   dataflow->d.texture_addr.y = y;
   dataflow->d.texture_addr.z = z;
   dataflow->d.texture_addr.i = i;
   dataflow->d.texture_addr.sampler = sampler;

   return dataflow;
}
#endif

Dataflow *glsl_dataflow_construct_texture_size(Dataflow *sampler) {
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

Dataflow *glsl_dataflow_construct_phi(Dataflow *a, int in_a, Dataflow *b, int in_b) {
   assert(a->type == b->type);
   Dataflow *d = dataflow_construct_common(DATAFLOW_PHI, a->type);
   d->dependencies_count = 2;
   d->d.binary_op.left  = a;
   d->d.binary_op.right = b;
   d->u.phi.in_a = in_a;
   d->u.phi.in_b = in_b;
   return d;
}

Dataflow *glsl_dataflow_construct_image_info_param(Dataflow *sampler, ImageInfoParam param) {
   Dataflow *ret = dataflow_construct_common(DATAFLOW_IMAGE_INFO_PARAM, DF_UINT);
   ret->dependencies_count = 1;
   ret->d.image_info_param.sampler = sampler;
   ret->u.image_info_param.param = param;
   return ret;
}

bool glsl_dataflow_affects_memory(DataflowFlavour f) {
   return f == DATAFLOW_ADDRESS_STORE || f == DATAFLOW_ATOMIC_ADD ||
          f == DATAFLOW_ATOMIC_SUB    || f == DATAFLOW_ATOMIC_MIN ||
          f == DATAFLOW_ATOMIC_MAX    || f == DATAFLOW_ATOMIC_AND ||
          f == DATAFLOW_ATOMIC_OR     || f == DATAFLOW_ATOMIC_XOR ||
          f == DATAFLOW_ATOMIC_XCHG   || f == DATAFLOW_ATOMIC_CMPXCHG;
}

Dataflow *glsl_construct_texbuffer_info_param(Dataflow *sampler, TexBufferInfoParam param) {
   Dataflow *ret = dataflow_construct_common(DATAFLOW_TEXBUFFER_INFO_PARAM, DF_UINT);
   ret->dependencies_count = 1;
   ret->d.texbuffer_info_param.sampler = sampler;
   ret->u.texbuffer_info_param.param = param;
   return ret;
}

bool glsl_dataflow_tex_cfg_implies_bslod(uint32_t tex_cfg_bits) {
   return tex_cfg_bits & ( DF_TEXBITS_FETCH  | DF_TEXBITS_SAMPLER_FETCH |
                           DF_TEXBITS_GATHER | DF_TEXBITS_BSLOD         );
}
