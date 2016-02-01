/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/
#pragma once

#include "gfx_buffer.h"

VCOS_EXTERN_C_BEGIN

#define GFX_BUFFER_USAGE_V3D_TEXTURE         (1u << 0)
#define GFX_BUFFER_USAGE_V3D_CUBEMAP         (1u << 1) /* Must also specify V3D_TEXTURE */
#define GFX_BUFFER_USAGE_V3D_RENDER_TARGET   (1u << 2)
#define GFX_BUFFER_USAGE_V3D_DEPTH_STENCIL   (1u << 3)
#define GFX_BUFFER_USAGE_V3D_TLB_RAW         (1u << 4) /* Must also specify V3D_RENDER_TARGET/V3D_DEPTH_STENCIL */
#define GFX_BUFFER_USAGE_YFLIP_IF_POSSIBLE   (1u << 5)

#define GFX_BUFFER_USAGE_ALL                 ((1u << 6) - 1)

#define GFX_BUFFER_USAGE_V3D_TLB (        \
   GFX_BUFFER_USAGE_V3D_RENDER_TARGET |   \
   GFX_BUFFER_USAGE_V3D_DEPTH_STENCIL)

/* A combination of the flags above */
typedef uint32_t gfx_buffer_usage_t;

extern size_t gfx_buffer_sprint_usage(char *buf, size_t buf_size, size_t offset,
   gfx_buffer_usage_t usage);

#define GFX_BUFFER_SPRINT_USAGE(BUF_NAME, USAGE) \
   VCOS_SAFE_STRFUNC_TO_LOCAL_BUF(BUF_NAME, 128, gfx_buffer_sprint_usage, USAGE)

/* Returned pointer is only valid until next call... */
extern const char *gfx_buffer_desc_usage(gfx_buffer_usage_t usage);

extern void gfx_buffer_desc_gen(
   GFX_BUFFER_DESC_T *mls, /* one for each mip level. [0] is the top mip level */
   size_t *size, size_t *align, /* total size/align */
   gfx_buffer_usage_t usage,
   uint32_t width, uint32_t height, uint32_t depth,
   uint32_t num_mip_levels,
   /* Dims, yflip & format must be specified and must be compatible with usage.
    * They will be passed through unmodified (except when
    * GFX_BUFFER_USAGE_YFLIP_IF_POSSIBLE is set -- see below).
    *
    * If swizzling is specified, it must be compatible with usage and will be
    * passed through unmodified (exception: UIF swizzlings may be adjusted to
    * their XOR equivalents). Otherwise, it will be selected based on usage.
    *
    * If GFX_BUFFER_USAGE_YFLIP_IF_POSSIBLE is set, yflip/format will be
    * adjusted to y-flip if possible. */
   uint32_t num_planes, const GFX_LFMT_T *plane_lfmts);

struct gfx_buffer_uif_cfg
{
   bool force;
   /* Only valid if force set: */
   bool ub_xor; /* UIF XOR mode? */
   bool ub_noutile;
   uint32_t ub_pads[GFX_BUFFER_MAX_PLANES]; /* Padding, in UIF-blocks */
};

struct gfx_buffer_ml_cfg
{
   struct gfx_buffer_uif_cfg uif;

   /* TODO This should probably be per-plane */
   bool force_slice_pitch;
   uint32_t slice_pitch; /* Only valid if force_slice_pitch set */
};

static inline void gfx_buffer_default_ml_cfg(
   struct gfx_buffer_ml_cfg *ml_cfg)
{
   ml_cfg->uif.force = false;
   ml_cfg->force_slice_pitch = false;
}

extern void gfx_buffer_desc_gen_with_ml0_cfg(
   GFX_BUFFER_DESC_T *mls,
   size_t *size, size_t *align,
   gfx_buffer_usage_t usage,
   uint32_t width, uint32_t height, uint32_t depth,
   uint32_t num_mip_levels,
   uint32_t num_planes, const GFX_LFMT_T *plane_lfmts,
   const struct gfx_buffer_ml_cfg *ml0_cfg); /* NULL allowed; means no special config for level 0 */

extern void gfx_buffer_get_tmu_uif_cfg(
   struct gfx_buffer_uif_cfg *uif_cfg,
   const GFX_BUFFER_DESC_T *ml0, uint32_t plane_i);

VCOS_EXTERN_C_END
