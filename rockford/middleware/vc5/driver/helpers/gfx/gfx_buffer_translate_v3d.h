/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/
#pragma once

#include "helpers/gfx/gfx_buffer.h"
#include "helpers/v3d/v3d_common.h"
#include "helpers/v3d/v3d_cl.h"

extern v3d_memory_format_t gfx_buffer_translate_memory_format(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i, unsigned slice);

typedef struct {
   v3d_memory_format_t memory_format;
   v3d_pixel_format_t pixel_format;
   v3d_rt_type_t internal_type;
   v3d_rt_bpp_t internal_bpp;

   bool flipy;

   uint32_t pad;
   /* These only need to be provided to the hardware if pad == 15 */
   uint32_t clear3_raster_padded_width_or_nonraster_height;
   uint32_t clear3_uif_height_in_ub;

   uint32_t addr_offset;
} GFX_BUFFER_RCFG_COLOR_TRANSLATION_T;

static inline GFX_BUFFER_RCFG_COLOR_TRANSLATION_T gfx_buffer_rcfg_color_translation_dummy(void)
{
   GFX_BUFFER_RCFG_COLOR_TRANSLATION_T t = {
      V3D_MEMORY_FORMAT_RASTER,
      V3D_PIXEL_FORMAT_R8,
      V3D_RT_TYPE_8,
      V3D_RT_BPP_32,
      false,
      0, 0, 0, 0};
   return t;
}

/* This assumes elements in desc correspond exactly with pixels, so eg desc
 * should not describe a multisample buffer with 4 elements (samples) per
 * pixel. This matters for the width/height calculations. */
extern void gfx_buffer_translate_rcfg_color(
   GFX_BUFFER_RCFG_COLOR_TRANSLATION_T *t,
   const GFX_BUFFER_DESC_T *desc,
   uint32_t plane_i,
   unsigned slice,
   uint32_t frame_width, uint32_t frame_height);

// Get TFU input format and stride from desc.
extern v3d_tfu_iformat_t gfx_buffer_desc_get_tfu_iformat_and_stride(
   uint32_t* o_stride,
   GFX_BUFFER_DESC_T const* desc,
   unsigned plane_index
   );

// Get TFU output format and padding from desc.
extern v3d_tfu_oformat_t gfx_buffer_desc_get_tfu_oformat_and_height_pad_in_ub(
   uint32_t* o_height_pad_in_ub,
   GFX_BUFFER_DESC_T const* desc,
   unsigned plane_index
   );
