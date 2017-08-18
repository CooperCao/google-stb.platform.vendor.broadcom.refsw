/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/lfmt/lfmt.h"
#include "libs/core/gfx_buffer/gfx_buffer_slow_conv.h"
#include "v3d_gen.h"
#include "v3d_limits.h"

EXTERN_C_BEGIN

// V3D_TMU_F_BITS sets # blend bits for bi/tri linear
// e.g. 4 bits give 16 levels bend,
// 5 = 32, .. etc.
#define V3D_TMU_F_BITS 8
#define V3D_TMU_F_BITS_MASK ((1 << V3D_TMU_F_BITS) - 1)
#define V3D_TMU_F_HALF (1 << (V3D_TMU_F_BITS - 1))
#define V3D_TMU_F_ONE (1 << V3D_TMU_F_BITS)

static inline unsigned v3d_tmu_nearest_mip_level(uint32_t lod)
{
   // ceil(lod + 0.5) - 1
   lod += V3D_TMU_F_HALF;
   unsigned level = lod >> V3D_TMU_F_BITS;
   if (!(lod & V3D_TMU_F_BITS_MASK))
      --level;
   return level;
}

#if V3D_HAS_SEP_ANISO_CFG
static inline v3d_max_aniso_t v3d_tmu_translate_max_aniso(float max_aniso)
{
   if (max_aniso > 8.0f)
      return V3D_MAX_ANISO_16;
   if (max_aniso > 4.0f)
      return V3D_MAX_ANISO_8;
   if (max_aniso > 2.0f)
      return V3D_MAX_ANISO_4;
   return V3D_MAX_ANISO_2;
}
#else
static inline v3d_tmu_filters_t v3d_tmu_filters_anisotropic(float max_aniso)
{
   if (max_aniso > 8.0f)
      return V3D_TMU_FILTERS_ANISOTROPIC16;
   if (max_aniso > 4.0f)
      return V3D_TMU_FILTERS_ANISOTROPIC8;
   if (max_aniso > 2.0f)
      return V3D_TMU_FILTERS_ANISOTROPIC4;
   return V3D_TMU_FILTERS_ANISOTROPIC2;
}
#endif

static inline v3d_tmu_wrap_t v3d_tmu_wrap_from_wrap_i(v3d_tmu_wrap_i_t wrap_i)
{
   switch (wrap_i)
   {
   case V3D_TMU_WRAP_I_CLAMP:    return V3D_TMU_WRAP_CLAMP;
   case V3D_TMU_WRAP_I_BORDER:   return V3D_TMU_WRAP_BORDER;
   default:                      unreachable(); return V3D_TMU_WRAP_INVALID;
   }
}

static inline bool v3d_tmu_is_depth_type(v3d_tmu_type_t type)
{
   switch (type)
   {
#if !V3D_HAS_TMU_R32F_R16_SHAD
   case V3D_TMU_TYPE_DEPTH_COMP16:
   case V3D_TMU_TYPE_DEPTH_COMP32F:
#endif
   case V3D_TMU_TYPE_DEPTH_COMP24:
   case V3D_TMU_TYPE_DEPTH24_X8:
      return true;
   default:
      return false;
   }
}

static inline bool v3d_tmu_type_supports_shadow(v3d_tmu_type_t type)
{
#if V3D_HAS_TMU_R32F_R16_SHAD
   if (type == V3D_TMU_TYPE_R32F || type == V3D_TMU_TYPE_R16)
      return true;
#endif
   return v3d_tmu_is_depth_type(type);
}

typedef enum
{
   V3D_TMU_BLEND_TYPE_FLOAT16,
   V3D_TMU_BLEND_TYPE_UNORM16,
   V3D_TMU_BLEND_TYPE_SNORM16,
   V3D_TMU_BLEND_TYPE_SNORM15, /* For SNORM8 with output_32 */
   V3D_TMU_BLEND_TYPE_INVALID
} v3d_tmu_blend_type_t;

/* V3D_TMU_BLEND_TYPE_INVALID is returned in cases where blending is not
 * possible */
extern v3d_tmu_blend_type_t v3d_maybe_get_tmu_blend_type(
   v3d_tmu_type_t type, bool srgb, bool shadow, bool output_32);

static inline v3d_tmu_blend_type_t v3d_get_tmu_blend_type(
   v3d_tmu_type_t type, bool srgb, bool shadow, bool output_32)
{
   v3d_tmu_blend_type_t blend_type = v3d_maybe_get_tmu_blend_type(
      type, srgb, shadow, output_32);
   assert(blend_type != V3D_TMU_BLEND_TYPE_INVALID);
   return blend_type;
}

extern GFX_LFMT_T v3d_tmu_blend_fmt_from_type(v3d_tmu_blend_type_t blend_type);

/* Returns the format that we should have through the blend pipeline stage.
 * Note that we will return a format for all types, even those that cannot be
 * blended (blend must be disabled for such types, and the blend stage will
 * just pass values through unmodified). */
extern GFX_LFMT_T v3d_get_tmu_blend_fmt(
   v3d_tmu_type_t type, bool srgb, bool shadow, bool output_32);

extern uint32_t v3d_tmu_get_num_channels(v3d_tmu_type_t type);

#if !V3D_VER_AT_LEAST(4,0,2,0)

/* Return the output type (true=32, false=16) that is used when
 * !V3D_MISCCFG.ovrtmuout or when (cfg=1 & ltype!=CHILD_IMAGE &
 * output_type=V3D_TMU_OUTPUT_TYPE_AUTO) */
extern bool v3d_tmu_auto_output_32(v3d_tmu_type_t type, bool shadow);

static inline bool v3d_tmu_output_type_32(v3d_tmu_output_type_t output_type,
   v3d_tmu_type_t type, bool shadow)
{
   switch (output_type)
   {
   case V3D_TMU_OUTPUT_TYPE_16:     return false;
   case V3D_TMU_OUTPUT_TYPE_32:     return true;
   case V3D_TMU_OUTPUT_TYPE_AUTO:   return v3d_tmu_auto_output_32(type, shadow);
   default:                         unreachable();
   }
}

/* Get the number of words returned by the TMU for the specified type in cfg=0
 * mode */
extern uint32_t v3d_tmu_get_word_read_default(
   v3d_tmu_type_t type, bool misccfg_ovrtmuout);

#endif

/* Get the maximum number of words that can be returned by the TMU with the
 * specified configuration */
extern uint32_t v3d_tmu_get_word_read_max(v3d_tmu_type_t type, bool gather, bool output_32);

extern bool v3d_tmu_type_supports_srgb(v3d_tmu_type_t type);

extern void v3d_tmu_calc_mip_levels(GFX_BUFFER_DESC_T *mip_levels,
   uint32_t dims, bool srgb, v3d_tmu_type_t type,
   const struct gfx_buffer_uif_cfg *uif_cfg, uint32_t arr_str,
   uint32_t width, uint32_t height, uint32_t depth,
   uint32_t num_mip_levels);

typedef enum
{
   V3D_TMU_OP_CLASS_REGULAR,
   V3D_TMU_OP_CLASS_ATOMIC,
   V3D_TMU_OP_CLASS_CACHE
} v3d_tmu_op_class_t;

struct v3d_tmu_cfg
{
   bool texture; /* Else general */

   /* Most bits of config are only used for texture accesses. The bits of
    * config also used for general accesses are marked "general too". */

   /* Derived from written registers */
   bool is_write; /* General too */
   uint32_t dims; // 2 for cubemaps, does not include array index
   bool array;
   bool cube;
   bool shadow;
   bool fetch;

   /* From params */
   bool word_en[4]; /* General too */
   bool output_32;
   bool unnorm;
   bool pix_mask; /* General too */
   bool tmuoff_4x; /* false if tmuoff was not written */
   bool bslod;
   uint32_t sample_num;
   bool gather;
   int32_t tex_off_s;
   int32_t tex_off_t;
   int32_t tex_off_r;
   v3d_tmu_op_t op; /* General too */
#if V3D_HAS_TMU_LOD_QUERY
   bool lod_query;
#endif

   /* From texture state */
   bool flipx;
   bool flipy;
   bool srgb; /* Will be false if sampler->srgb_override set */
   bool ahdr;
   v3d_addr_t l0_addr;
   uint32_t arr_str;
   uint32_t width;
   uint32_t height; /* Always 1 if 1D */
   uint32_t depth; /* Always 1 if 1D or 2D */
   uint32_t num_array_elems; /* Always 1 if not array. Multiple of 6 if cubemap. */
   v3d_tmu_type_t type;
   v3d_tmu_swizzle_t swizzles[4];
   uint32_t base_level;
   struct gfx_buffer_uif_cfg uif_cfg;

   /* From sampler */
   uint32_t aniso_level; /* Number of samples = 2^aniso_level. 0 means no anisotropic filtering. */
   v3d_tmu_filter_t magfilt;
   v3d_tmu_filter_t minfilt;
   v3d_tmu_mipfilt_t mipfilt;
   v3d_compare_func_t compare_func;
   uint32_t min_lod; // .V3D_TMU_F_BITS
   uint32_t max_lod; // .V3D_TMU_F_BITS
   int32_t fixed_bias; // .V3D_TMU_F_BITS
   v3d_tmu_wrap_t wrap_s;
   v3d_tmu_wrap_t wrap_t;
   v3d_tmu_wrap_t wrap_r;
   v3d_tmu_wrap_i_t wrap_i;
   GFX_LFMT_BLOCK_T bcolour; /* Border colour. For non-depth types will be in blend format. */

#if !V3D_VER_AT_LEAST(4,0,2,0)
   bool child_image;
   /* If !child_image, cx/yoff should be 0 and cwidth/height should match
    * width/height. */
   uint32_t cwidth;
   uint32_t cheight;
   uint32_t cxoff;
   uint32_t cyoff;

   /* Just indicates whether or not we expect a bias value to be written */
   bool bias;
#endif

   /* For texture accesses, derived from rest of config */
   v3d_addr_t base_addr; /* Offsets in mip_levels are relative to this */
   GFX_BUFFER_DESC_T mip_levels[V3D_MAX_MIP_COUNT];
   /* The range of mip levels the hardware might access. */
   uint32_t minlvl, miplvls;
   /* The format we should have through the blend pipeline stage */
   GFX_LFMT_T blend_fmt;
   v3d_tmu_op_class_t op_class; /* General too */
   /* All these general too.
    * Not used if op != V3D_TMU_OP_REGULAR or (texture && !is_write). */
   bool data_signed; /* Not used if is_write */
   uint32_t num_data_words;
   uint32_t bytes_per_data_word;
};

struct v3d_tmu_reg_flags
{
   bool d;
#if V3D_VER_AT_LEAST(4,0,2,0)
   bool s, t, r, i, b, dref, off, scm, sfetch, slod;
#endif
};

#if V3D_VER_AT_LEAST(4,0,2,0)
extern void v3d_tmu_cfg_collect_texture(struct v3d_tmu_cfg *cfg,
   const struct v3d_tmu_reg_flags *written,
   const V3D_TMU_PARAM0_T *p0,
   const V3D_TMU_PARAM1_T *p1, /* May be NULL */
   const V3D_TMU_PARAM2_T *p2, /* May be NULL */
   const V3D_TMU_TEX_STATE_T *tex_state,
   const V3D_TMU_TEX_EXTENSION_T *tex_extension, /* May be NULL */
   const V3D_TMU_SAMPLER_T *sampler, /* May be NULL */
   const uint32_t bcolour[4]); /* May be NULL */
#else
extern void v3d_tmu_cfg_collect_texture(struct v3d_tmu_cfg *cfg,
   const V3D_MISCCFG_T *misccfg,
   const V3D_TMU_PARAM0_T *p0,
   const V3D_TMU_PARAM1_CFG0_T *p1_cfg0, const V3D_TMU_PARAM1_CFG1_T *p1_cfg1,
   const V3D_TMU_INDIRECT_T *ind);
#endif

extern void v3d_tmu_cfg_collect_general(struct v3d_tmu_cfg *cfg,
   const struct v3d_tmu_reg_flags *written,
   const V3D_TMU_GENERAL_CONFIG_T *general_cfg);

#if V3D_HAS_LARGE_1D_TEXTURE
extern void v3d_tmu_get_wh_for_1d_tex_state(uint32_t *w, uint32_t *h,
      uint32_t width_in);
#endif

EXTERN_C_END
