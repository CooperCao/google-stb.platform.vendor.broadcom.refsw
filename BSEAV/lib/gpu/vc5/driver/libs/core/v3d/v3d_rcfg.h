/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/gfx_buffer/gfx_buffer.h"
#include "v3d_cl.h"
#include "v3d_limits.h"

VCOS_EXTERN_C_BEGIN

struct v3d_rt_cfg
{
   V3D_RT_FORMAT_T format;

   uint32_t clear[V3D_MAX_RT_WORDS_PER_SAMPLE];

#if !V3D_VER_AT_LEAST(4,0,2,0)
   /* stride/flipy_height_px valid after v3d_rcfg_finalise() */
   struct v3d_tlb_ldst_params default_ldst_params;

   /* These fields are only read by v3d_rcfg_finalise(). They're used to fill
    * in default_ldst_params.stride/flipy_height_px */
   uint32_t pad;
   uint32_t clear3_raster_padded_width_or_nonraster_height;
   uint32_t clear3_uif_height_in_ub;
#endif
};

struct v3d_rcfg
{
   /* Frame size in pixels, as specified in the common rendering mode config
    * instruction. Note that for y-flipping the (possibly larger)
    * per-render-target height is used */
   uint32_t frame_width_px, frame_height_px;

   uint32_t num_samp; /* Number of samples per pixel */
   bool double_buffer;
   bool cov_mode;

   uint32_t tile_w_px, tile_h_px;

#if !V3D_VER_AT_LEAST(4,0,2,0)
   uint32_t disable_rt_store_mask;
   bool depth_store;
   bool stencil_store;
#endif

   uint32_t num_rts;
   struct v3d_rt_cfg rts[V3D_MAX_RENDER_TARGETS];

   v3d_depth_type_t depth_type;
   uint32_t depth_clear;
#if !V3D_VER_AT_LEAST(4,0,2,0)
   struct v3d_tlb_ldst_params depth_default_ldst_params;
#endif

   bool ez_disable;
   v3d_ez_direction_t ez_direction;

   uint8_t stencil_clear;
#if !V3D_VER_AT_LEAST(4,0,2,0)
   struct v3d_tlb_ldst_params separate_stencil_default_ldst_params;
#endif

#if V3D_VER_AT_LEAST(4,0,2,0)
   bool early_ds_clear;
#endif
};

extern void v3d_rcfg_collect(
   struct v3d_rcfg *rcfg,
   const V3D_CL_TILE_RENDERING_MODE_CFG_T *i);
#if !V3D_VER_AT_LEAST(4,0,2,0)
extern void v3d_rcfg_finalise(struct v3d_rcfg *rcfg);

static inline bool v3d_rcfg_use_separate_stencil(const struct v3d_rcfg *rcfg)
{
   return !v3d_depth_format_has_stencil(
      rcfg->depth_default_ldst_params.output_format.depth);
}

extern void v3d_rcfg_get_default_ldst_params(
   struct v3d_tlb_ldst_params *ls,
   const struct v3d_rcfg *rcfg, v3d_ldst_buf_t buf);
extern void v3d_tlb_ldst_params_to_raw(
   struct v3d_tlb_ldst_params *ls,
   const struct v3d_rcfg *rcfg, v3d_ldst_buf_t buf);
#endif

extern void v3d_calc_tlb_ldst_buffer_desc(
   v3d_addr_t *base_addr, GFX_BUFFER_DESC_T *desc,
   const struct v3d_rcfg *rcfg, v3d_ldst_buf_t buf,
   const struct v3d_tlb_ldst_params *ls);

VCOS_EXTERN_C_END
