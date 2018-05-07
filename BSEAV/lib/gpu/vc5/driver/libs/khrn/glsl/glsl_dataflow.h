/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_const_types.h"
#include "glsl_list.h"
#include "glsl_common.h"

EXTERN_C_BEGIN

typedef struct _Dataflow Dataflow;

void glsl_dataflow_reset_count(void);
int  glsl_dataflow_get_count  (void);
int  glsl_dataflow_get_next_id(void);

// Set up age which will be applied to dataflow. This might not be a good thing.
void glsl_dataflow_reset_age(void);
void glsl_dataflow_set_age(int age);

void  glsl_dataflow_begin_construction();
void  glsl_dataflow_end_construction();
void *glsl_dataflow_malloc(size_t);

// Dataflow chain structure, for efficient appending.
DECLARE_NODE_LIST(Dataflow,glsl_dataflow_malloc)

typedef bool (*DataflowFilter)(Dataflow* dataflow, void* data);

void           glsl_dataflow_chain_init    (DataflowChain *chain);
DataflowChain *glsl_dataflow_chain_append  (DataflowChain *chain, Dataflow *dataflow);
// If node in chain, removes node from chain, else, behaviour undefined.
DataflowChain *glsl_dataflow_chain_remove_node(DataflowChain *chain, DataflowChainNode *node);

#if !V3D_VER_AT_LEAST(4,1,34,0)
typedef enum {
  IMAGE_INFO_ARR_STRIDE,
  IMAGE_INFO_SWIZZLING,
  IMAGE_INFO_XOR_ADDR,
  IMAGE_INFO_LX_ADDR,
  IMAGE_INFO_LX_PITCH,
  IMAGE_INFO_LX_SLICE_PITCH,
} ImageInfoParam;
#endif

typedef enum {
   DATAFLOW_CONST,

   DATAFLOW_LOAD,
   DATAFLOW_PHI,
   DATAFLOW_EXTERNAL,

   DATAFLOW_LOGICAL_NOT,
   DATAFLOW_CONST_IMAGE,
   DATAFLOW_CONST_SAMPLER,
#if V3D_VER_AT_LEAST(4,1,34,0)
   DATAFLOW_SAMPLER_UNNORMS,
#endif
   DATAFLOW_FTOI_TRUNC,
   DATAFLOW_FTOI_NEAREST,
   DATAFLOW_FTOU,
   DATAFLOW_ISNAN,

   DATAFLOW_BITWISE_NOT,
   DATAFLOW_BITWISE_AND,
   DATAFLOW_BITWISE_OR,
   DATAFLOW_BITWISE_XOR,

   DATAFLOW_SHL,
   DATAFLOW_SHR,
   DATAFLOW_ROR,

   DATAFLOW_ADDRESS_STORE,
   DATAFLOW_IN_LOAD,
   DATAFLOW_VECTOR_LOAD,
   DATAFLOW_ATOMIC_ADD,
   DATAFLOW_ATOMIC_SUB,
   DATAFLOW_ATOMIC_MIN,
   DATAFLOW_ATOMIC_MAX,
   DATAFLOW_ATOMIC_AND,
   DATAFLOW_ATOMIC_OR,
   DATAFLOW_ATOMIC_XOR,
   DATAFLOW_ATOMIC_XCHG,
   DATAFLOW_ATOMIC_CMPXCHG,
   DATAFLOW_UNIFORM,
   DATAFLOW_UNIFORM_BUFFER,
   DATAFLOW_STORAGE_BUFFER,
   DATAFLOW_ATOMIC_COUNTER,
   DATAFLOW_IN,
   DATAFLOW_MUL,
   DATAFLOW_MUL24,
   DATAFLOW_DIV,
   DATAFLOW_REM,
   DATAFLOW_ADD,
   DATAFLOW_SUB,
   DATAFLOW_ARITH_NEGATE,
   DATAFLOW_LESS_THAN,
   DATAFLOW_LESS_THAN_EQUAL,
   DATAFLOW_GREATER_THAN,
   DATAFLOW_GREATER_THAN_EQUAL,
   DATAFLOW_EQUAL,
   DATAFLOW_NOT_EQUAL,
   DATAFLOW_LOGICAL_AND,
   DATAFLOW_LOGICAL_XOR,
   DATAFLOW_LOGICAL_OR,
   DATAFLOW_CONDITIONAL,

   DATAFLOW_SQRT,
   DATAFLOW_RSQRT,
   DATAFLOW_RCP,
   DATAFLOW_LOG2,
   DATAFLOW_EXP2,
   DATAFLOW_SIN,
   DATAFLOW_COS,
   DATAFLOW_TAN,
   DATAFLOW_MIN,
   DATAFLOW_MAX,
   DATAFLOW_TRUNC,
   DATAFLOW_NEAREST,
   DATAFLOW_CEIL,
   DATAFLOW_FLOOR,
   DATAFLOW_ABS,
   DATAFLOW_FDX,
   DATAFLOW_FDY,
   DATAFLOW_REINTERP,
   DATAFLOW_FPACK,
   DATAFLOW_FUNPACKA,
   DATAFLOW_FUNPACKB,

   DATAFLOW_VFMIN,
   DATAFLOW_VFMAX,
   DATAFLOW_VFMUL,
   DATAFLOW_VITODENF,
   DATAFLOW_VFMULDENFTOI,

   DATAFLOW_ITOF,
   DATAFLOW_UTOF,
   DATAFLOW_CLZ,

   // IR texture gadget
   DATAFLOW_VEC4,
   DATAFLOW_TEXTURE,
#if V3D_VER_AT_LEAST(4,1,34,0)
   DATAFLOW_TEXTURE_ADDR,
#endif
   DATAFLOW_TEXTURE_SIZE,
   DATAFLOW_TEXTURE_LEVELS,
   DATAFLOW_GET_VEC4_COMPONENT,

   // Fragment colour get (used with GET_VEC4_COMPONENT)
   DATAFLOW_FRAG_GET_COL,
   DATAFLOW_FRAG_GET_DEPTH,
   DATAFLOW_FRAG_GET_STENCIL,

   // Fragment coordinate retrieval.
   DATAFLOW_FRAG_GET_X,
   DATAFLOW_FRAG_GET_Y,
   DATAFLOW_FRAG_GET_X_UINT,
   DATAFLOW_FRAG_GET_Y_UINT,
   DATAFLOW_FRAG_GET_Z,
   DATAFLOW_FRAG_GET_W,
   DATAFLOW_FRAG_GET_FF,

   DATAFLOW_COMP_GET_ID0,
   DATAFLOW_COMP_GET_ID1,

   DATAFLOW_GET_THREAD_INDEX,
   DATAFLOW_SHARED_PTR,

   DATAFLOW_IS_HELPER,
   DATAFLOW_SAMPLE_POS_X,
   DATAFLOW_SAMPLE_POS_Y,
   DATAFLOW_SAMPLE_MASK,
   DATAFLOW_SAMPLE_ID,
   DATAFLOW_NUM_SAMPLES,

   DATAFLOW_SG_LOCAL_IDX,
   DATAFLOW_SG_ELECT,

   DATAFLOW_GET_VERTEX_ID,
   DATAFLOW_GET_INSTANCE_ID,
   DATAFLOW_GET_BASE_INSTANCE,
   DATAFLOW_GET_POINT_COORD_X,
   DATAFLOW_GET_POINT_COORD_Y,
   DATAFLOW_GET_LINE_COORD,
   DATAFLOW_GET_DEPTHRANGE_NEAR,
   DATAFLOW_GET_DEPTHRANGE_FAR,
   DATAFLOW_GET_DEPTHRANGE_DIFF,
   DATAFLOW_GET_NUMWORKGROUPS_X,
   DATAFLOW_GET_NUMWORKGROUPS_Y,
   DATAFLOW_GET_NUMWORKGROUPS_Z,

   DATAFLOW_GET_INVOCATION_ID,
   DATAFLOW_GET_PRIMITIVE_ID,

   // Buffer loads and stores
   DATAFLOW_ADDRESS,
   DATAFLOW_BUF_SIZE,
   DATAFLOW_BUF_ARRAY_LENGTH,

#if !V3D_VER_AT_LEAST(4,1,34,0)
   /* image unit additional params */
   DATAFLOW_IMAGE_INFO_PARAM,
#endif

   DATAFLOW_GET_FB_MAX_LAYER,

   DATAFLOW_FLAVOUR_COUNT
} DataflowFlavour;

#define DATAFLOW_MAX_DEPENDENCIES 6

/* These are deliberately in a different order from PRIM_* */
typedef enum {
   DF_BOOL,
   DF_INT,
   DF_UINT,
   DF_FLOAT,
   DF_F_SAMP_IMG,    /* SAMP variants correspond to SPIR-V sampled images   */
   DF_I_SAMP_IMG,    /* (GLSL textures) and STOR variants to storage images */
   DF_U_SAMP_IMG,    /* (GLSL images).                                      */
   DF_ATOMIC,
   DF_F_STOR_IMG,
   DF_I_STOR_IMG,
   DF_U_STOR_IMG,
   DF_SAMPLER,
   DF_VOID,
   DF_TYPE_COUNT,
   DF_INVALID,
} DataflowType;

#define DF_TEXBITS_GATHER_COMP_SHIFT  8
#define DF_TEXBITS_GATHER_COMP_MASK  (3 << DF_TEXBITS_GATHER_COMP_SHIFT)

typedef enum {
   DF_TEXBITS_NONE          = 0,
   DF_TEXBITS_CUBE          = (1<<0),
   DF_TEXBITS_FETCH         = (1<<1),
   DF_TEXBITS_GATHER        = (1<<2),
   DF_TEXBITS_BSLOD         = (1<<3),
   DF_TEXBITS_I_OFF         = (1<<4),
   DF_TEXBITS_SAMPLER_FETCH = (1<<5),
#if V3D_VER_AT_LEAST(4,2,13,0)
   DF_TEXBITS_LOD_QUERY     = (1<<6),
#endif

   DF_TEXBITS_GATHER_COMP_0 = (0 << DF_TEXBITS_GATHER_COMP_SHIFT),
   DF_TEXBITS_GATHER_COMP_1 = (1 << DF_TEXBITS_GATHER_COMP_SHIFT),
   DF_TEXBITS_GATHER_COMP_2 = (2 << DF_TEXBITS_GATHER_COMP_SHIFT),
   DF_TEXBITS_GATHER_COMP_3 = (3 << DF_TEXBITS_GATHER_COMP_SHIFT),
} DFTexbits;

struct _Dataflow
{
   int id;
   DataflowFlavour flavour;
   DataflowType type;
   int dependencies_count;

   union {
      Dataflow *dependencies[DATAFLOW_MAX_DEPENDENCIES];
      int reloc_deps[DATAFLOW_MAX_DEPENDENCIES];

      struct {
         Dataflow *operand;
      } unary_op;

      struct {
         Dataflow *left;
         Dataflow *right;
      } binary_op;

      struct {
         Dataflow *cond;
         Dataflow *true_value;
         Dataflow *false_value;
      } cond_op;

      struct {
         Dataflow *addr; /* May be a DATAFLOW_TEXTURE_ADDR (V3D_VER_AT_LEAST(4,1,34,0) only) */
         Dataflow *val;
         Dataflow *cond;
         Dataflow *prev;
      } addr_store;

      struct {
         Dataflow *coords;
         Dataflow *d;       /* may be NULL */
         Dataflow *b;       /* may be NULL */
         Dataflow *off;     /* may be NULL */
         Dataflow *image;
         Dataflow *sampler; /* may be NULL */
      } texture;

#if V3D_VER_AT_LEAST(4,1,34,0)
      struct {
         Dataflow *x;
         Dataflow *y;
         Dataflow *z;
         Dataflow *i;
         Dataflow *image;
      } texture_addr;
#endif

      struct {
         Dataflow *image;
      } texture_size;

      struct {
         Dataflow *image;
      } image_info_param;

      struct {
         Dataflow *image;
      } texbuffer_info_param;
   } d;

   union {
      uint32_t raw[4];     /* XXX: Must be the same size as the other members */

      // DATAFLOW_CONST, DATAFLOW_BUF_SIZE, DATAFLOW_BUF_ARRAY_LENGTH
      struct {
         const_value value;
      } constant;

      // DATAFLOW_CONST_IMAGE
      struct {
         const_value location;
         bool is_32bit;
         bool format_valid;  /* true for images*/
         FormatQualifier format;
      } const_image;

      // DATAFLOW_IN
      struct {
         const_value row;
      } linkable_value;

      // DATAFLOW_UNIFORM, DATAFLOW_UNIFORM_BUFFER, DATAFLOW_STORAGE_BUFFER, DATAFLOW_ATOMIC_COUNTER
      struct {
         const_value index;
         const_value offset;
      } buffer;

      struct {
         DFTexbits bits;
      } texture;

      struct {
         int render_target;
      } get_col;

      struct {
         uint32_t component_index;
      } get_vec4_component;

      struct {
         int block;
         int output;
      } external;

      struct {
         int in_a;
         int in_b;
      } phi;

#if !V3D_VER_AT_LEAST(4,1,34,0)
      struct {
         ImageInfoParam param;
      } image_info_param;
#endif

      //DATAFLOW_LOAD
      struct{
        bool fmt_valid;
        FormatQualifier fmt;
      } load;
   } u;

   uint32_t age; /* determines scheduling order */
};

// On failure, these functions return NULL.
Dataflow *glsl_dataflow_construct_const_value(DataflowType type, const_value value);
Dataflow *glsl_dataflow_construct_const_bool(bool value);
Dataflow *glsl_dataflow_construct_const_int(int value);
Dataflow *glsl_dataflow_construct_const_uint(unsigned value);
Dataflow *glsl_dataflow_construct_const_float(float value);
Dataflow *glsl_dataflow_construct_const_image(DataflowType type, const_value location, bool is_32bit);
Dataflow *glsl_dataflow_construct_linkable_value(DataflowFlavour flavour, DataflowType type, const_value row);
Dataflow *glsl_dataflow_construct_buffer(DataflowFlavour flavour, DataflowType type, const_value index, const_value offset);
Dataflow *glsl_dataflow_construct_address_load(DataflowFlavour f, DataflowType type, Dataflow *address);
Dataflow *glsl_dataflow_construct_vector_load(Dataflow *address);
// cond can be NULL
Dataflow *glsl_dataflow_construct_atomic(DataflowFlavour flavour, DataflowType type, Dataflow *address, Dataflow *arg,
      Dataflow *cond, Dataflow *prev);
Dataflow *glsl_dataflow_construct_load(DataflowType type);
Dataflow *glsl_dataflow_construct_nullary_op(DataflowFlavour flavour);
Dataflow *glsl_dataflow_construct_unary_op(DataflowFlavour flavour, Dataflow* operand);
Dataflow *glsl_dataflow_construct_binary_op(DataflowFlavour flavour, Dataflow* left, Dataflow* right);
Dataflow *glsl_dataflow_construct_ternary_op(DataflowFlavour flavour, Dataflow* first, Dataflow* second, Dataflow* third);
Dataflow *glsl_dataflow_construct_op(DataflowFlavour flavour, int num_args, Dataflow **arguments);
Dataflow *glsl_dataflow_construct_reinterp(Dataflow *operand, DataflowType new_type);
Dataflow *glsl_dataflow_construct_address(Dataflow *operand);
Dataflow *glsl_dataflow_construct_buf_size(Dataflow *operand, const_value subtract_offset);
Dataflow *glsl_dataflow_construct_buf_array_length(Dataflow *operand, const_value subtract);

#if V3D_VER_AT_LEAST(4,1,34,0)
Dataflow *glsl_dataflow_construct_sampler_unnorms(Dataflow *operand);
#endif

Dataflow *glsl_dataflow_construct_vec4(Dataflow *r, Dataflow *g, Dataflow *b, Dataflow *a);
Dataflow *glsl_dataflow_construct_get_vec4_component(uint32_t component_index, Dataflow *param, DataflowType type);

void glsl_dataflow_construct_frag_get_col(Dataflow **out, DataflowType type, int render_target);
void glsl_dataflow_construct_texture_lookup(Dataflow **out, unsigned n_out,
                                            uint32_t bits, Dataflow *image,
                                            Dataflow *coords,
                                            Dataflow *d, Dataflow *b, Dataflow *off, Dataflow *sampler);
#if V3D_VER_AT_LEAST(4,1,34,0)
Dataflow *glsl_dataflow_construct_texture_addr(Dataflow *sampler,
                                               Dataflow *x, Dataflow *y, Dataflow *z,
                                               Dataflow *i);
#else
Dataflow *glsl_dataflow_construct_image_info_param(Dataflow *sampler, ImageInfoParam param);
#endif
Dataflow *glsl_dataflow_construct_texture_size(Dataflow *sampler);
Dataflow *glsl_dataflow_construct_texture_num_levels(Dataflow *sampler);
Dataflow *glsl_dataflow_construct_external(DataflowType t, int block, int output);
Dataflow *glsl_dataflow_construct_phi(Dataflow *a, int in_a, Dataflow *b, int in_b);

const char *glsl_dataflow_info_get_name(DataflowFlavour flavour);

// Returns a dataflow node that converts input to the given type.
Dataflow *glsl_dataflow_convert_type(Dataflow *input, DataflowType out_type);

static inline bool glsl_dataflow_is_integral_type(const Dataflow *df) {
   return df->type == DF_INT || df->type == DF_UINT;
}

static inline bool glsl_dataflow_type_is_sampled_image(DataflowType t) {
   return t == DF_F_SAMP_IMG || t == DF_I_SAMP_IMG || t == DF_U_SAMP_IMG;
}
static inline bool glsl_dataflow_type_is_storage_image(DataflowType t) {
   return t == DF_F_STOR_IMG || t == DF_I_STOR_IMG || t == DF_U_STOR_IMG;
}

static inline bool glsl_dataflow_requires_per_sample(DataflowFlavour f) {
   return (f == DATAFLOW_FRAG_GET_COL     || f == DATAFLOW_FRAG_GET_DEPTH ||
           f == DATAFLOW_FRAG_GET_STENCIL || f == DATAFLOW_SAMPLE_ID      ||
           f == DATAFLOW_SAMPLE_POS_X     || f == DATAFLOW_SAMPLE_POS_Y);
}

bool glsl_dataflow_affects_memory(DataflowFlavour f);
bool glsl_dataflow_tex_cfg_implies_bslod(uint32_t tex_cfg_bits);

EXTERN_C_END
