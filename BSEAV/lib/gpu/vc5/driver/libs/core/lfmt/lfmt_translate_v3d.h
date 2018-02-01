/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "lfmt.h"
#include "libs/core/v3d/v3d_common.h"
#include "libs/core/v3d/v3d_cl.h"
#include "libs/core/v3d/v3d_tmu.h"
#include "libs/core/v3d/v3d_tfu.h"

EXTERN_C_BEGIN

extern v3d_index_type_t gfx_lfmt_translate_index_type(GFX_LFMT_T lfmt);

/** TLB/TFU memory format */

/* These functions return dims (=2D) & swizzling */
extern GFX_LFMT_T gfx_lfmt_translate_from_memory_format(
   v3d_memory_format_t memory_format);
extern GFX_LFMT_T gfx_lfmt_translate_from_tfu_iformat(
   v3d_tfu_iformat_t tfu_iformat, unsigned dram_map_version);
extern GFX_LFMT_T gfx_lfmt_translate_from_tfu_oformat(
   v3d_tfu_oformat_t tfu_oformat);

/** TLB pixel format */

#if V3D_VER_AT_LEAST(4,1,34,0)
/* These only pay attention to format */
extern bool gfx_lfmt_maybe_translate_pixel_format(GFX_LFMT_T lfmt, v3d_pixel_format_t *pf, bool *reverse, bool *rb_swap);
extern void gfx_lfmt_translate_pixel_format(GFX_LFMT_T lfmt, v3d_pixel_format_t *pf, bool *reverse, bool *rb_swap);

/* Returns just format */
extern GFX_LFMT_T gfx_lfmt_translate_from_pixel_format(v3d_pixel_format_t pixel_format, bool reverse, bool rb_swap);

/* Returns dims, layout, and format */
static inline GFX_LFMT_T gfx_lfmt_translate_from_memory_pixel_format_and_flipy(
   v3d_memory_format_t memory_format, v3d_pixel_format_t pixel_format, bool reverse, bool rb_swap, bool flipy)
{
   assert(v3d_memory_and_pixel_formats_compatible(memory_format, pixel_format));
   GFX_LFMT_T lfmt = gfx_lfmt_set_format(
      gfx_lfmt_translate_from_memory_format(memory_format),
      gfx_lfmt_translate_from_pixel_format(pixel_format, reverse, rb_swap));
   if (flipy)
   {
      gfx_lfmt_set_base(&lfmt, gfx_lfmt_yflip_base(gfx_lfmt_get_base(&lfmt)));
      gfx_lfmt_set_yflip(&lfmt, GFX_LFMT_YFLIP_YFLIP);
   }
   return lfmt;
}

#else
/* These only pay attention to format */
extern v3d_pixel_format_t gfx_lfmt_maybe_translate_pixel_format(GFX_LFMT_T lfmt);
extern v3d_pixel_format_t gfx_lfmt_translate_pixel_format(GFX_LFMT_T lfmt);

/* Returns just format */
extern GFX_LFMT_T gfx_lfmt_translate_from_pixel_format(v3d_pixel_format_t pixel_format);

/* Returns dims, layout, and format */
static inline GFX_LFMT_T gfx_lfmt_translate_from_memory_pixel_format_and_flipy(
   v3d_memory_format_t memory_format, v3d_pixel_format_t pixel_format, bool flipy)
{
   assert(v3d_memory_and_pixel_formats_compatible(memory_format, pixel_format));
   GFX_LFMT_T lfmt = gfx_lfmt_set_format(
      gfx_lfmt_translate_from_memory_format(memory_format),
      gfx_lfmt_translate_from_pixel_format(pixel_format));
   if (flipy)
   {
      gfx_lfmt_set_base(&lfmt, gfx_lfmt_yflip_base(gfx_lfmt_get_base(&lfmt)));
      gfx_lfmt_set_yflip(&lfmt, GFX_LFMT_YFLIP_YFLIP);
   }
   return lfmt;
}

#endif

extern bool gfx_lfmt_maybe_translate_rt_format(V3D_RT_FORMAT_T *rt_format, GFX_LFMT_T lfmt);
extern void gfx_lfmt_translate_rt_format(V3D_RT_FORMAT_T *rt_format, GFX_LFMT_T lfmt);

#if !V3D_VER_AT_LEAST(4,1,34,0)

/* Returns dims & format */
extern GFX_LFMT_T gfx_lfmt_translate_internal_raw_mode(GFX_LFMT_T lfmt);

/** TLB depth format */

/* Note that when the TLB loads/stores only stencil with a depth-only depth
 * format, it assumes an 8bpp stencil format, ignoring the actual depth format.
 * This case needs to be handled by the calling code */

/* These only pay attention to format */
extern v3d_depth_format_t gfx_lfmt_maybe_translate_depth_format(GFX_LFMT_T lfmt);
extern v3d_depth_format_t gfx_lfmt_translate_depth_format(GFX_LFMT_T lfmt);

/* Returns just format */
extern GFX_LFMT_T gfx_lfmt_translate_from_depth_format(v3d_depth_format_t depth_format);

/* Returns dims, layout, and format */
static inline GFX_LFMT_T gfx_lfmt_translate_from_memory_and_depth_format(
   v3d_memory_format_t memory_format, v3d_depth_format_t depth_format)
{
   return gfx_lfmt_set_format(
      gfx_lfmt_translate_from_memory_format(memory_format),
      gfx_lfmt_translate_from_depth_format(depth_format));
}

#endif

/** TLB internal depth type */

extern v3d_depth_type_t gfx_lfmt_maybe_translate_depth_type(GFX_LFMT_T depth_lfmt);
extern v3d_depth_type_t gfx_lfmt_translate_depth_type(GFX_LFMT_T lfmt);

/** TMU */

#if !V3D_VER_AT_LEAST(3,3,0,0)
/* format of data returned from tmu. the tmu word enables can limit the number
 * of words returned */
typedef enum
{
   /* tmu returns packed data */
   GFX_LFMT_TMU_RET_8,        /* 4x8-bit per word  */
   GFX_LFMT_TMU_RET_16,       /* 2x16-bit per word */
   GFX_LFMT_TMU_RET_32,       /* 1x32-bit per word */
   GFX_LFMT_TMU_RET_1010102,  /* 1010102 as one word */

   /* tmu returns data as 2x16 or 1x32, switchable */
   GFX_LFMT_TMU_RET_AUTO,

   GFX_LFMT_TMU_RET_INVALID
} gfx_lfmt_tmu_ret_t;
#endif

typedef struct
{
   v3d_tmu_type_t type;
   bool srgb;

#if !V3D_VER_AT_LEAST(3,3,0,0)
   gfx_lfmt_tmu_ret_t ret;

   /* If shader_swizzle is true, swizzling needs to be done in the shader --
    * the TMU swizzles should just be set to R,G,B,A. Otherwise, ie if
    * shader_swizzle is false, swizzles should be passed to the TMU and the
    * shader doesn't need to do any swizzling. */
   bool shader_swizzle;
#endif
   v3d_tmu_swizzle_t swizzles[4];
} GFX_LFMT_TMU_TRANSLATION_T;

/* These only pay attention to format.
 * false is returned on failure. */
extern bool gfx_lfmt_maybe_translate_tmu(GFX_LFMT_TMU_TRANSLATION_T *t,
   GFX_LFMT_T lfmt
#if !V3D_HAS_TMU_R32F_R16_SHAD
   , bool need_depth_type
#endif
   );
extern void gfx_lfmt_translate_tmu(GFX_LFMT_TMU_TRANSLATION_T *t,
   GFX_LFMT_T lfmt
#if !V3D_HAS_TMU_R32F_R16_SHAD
   , bool need_depth_type
#endif
   );

/* Returns just format */
extern GFX_LFMT_T gfx_lfmt_translate_from_tmu_type(v3d_tmu_type_t tmu_type, bool srgb);

/** TFU */

/* Returns just formats */
extern void gfx_lfmt_translate_from_tfu_type(
   GFX_LFMT_T *fmts, uint32_t *num_planes, v3d_tfu_yuv_col_space_t *yuv_col_space, // yuv_col_space may be NULL
   v3d_tfu_type_t tfu_type, v3d_tfu_rgbord_t rgbord, bool srgb, bool is_bigend_sand);

/* Pays attention to format *and layout*
 * Returns false on failure to translate */
extern bool gfx_lfmt_maybe_translate_to_tfu_type(
   v3d_tfu_type_t *tfu_type, bool *srgb, v3d_tfu_rgbord_t *rgbord,
   const GFX_LFMT_T *src_lfmts, uint32_t src_num_planes,
   v3d_tfu_yuv_col_space_t src_yuv_col_space, /* Ignored if src not YUV. TODO Should this be in lfmt? */
   GFX_LFMT_T dst_lfmt);

extern void gfx_lfmt_translate_to_tfu_type(
   v3d_tfu_type_t *tfu_type, bool *srgb, v3d_tfu_rgbord_t *rgbord,
   const GFX_LFMT_T *src_lfmts, uint32_t src_num_planes,
   v3d_tfu_yuv_col_space_t src_yuv_col_space, /* Ignored if src not YUV */
   GFX_LFMT_T dst_lfmt);

EXTERN_C_END
