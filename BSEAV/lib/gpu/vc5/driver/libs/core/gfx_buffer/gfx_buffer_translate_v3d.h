/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "gfx_buffer.h"
#include "libs/core/v3d/v3d_common.h"
#include "libs/core/v3d/v3d_cl.h"

EXTERN_C_BEGIN

extern v3d_memory_format_t gfx_buffer_translate_memory_format(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i);

#if !V3D_VER_AT_LEAST(4,1,34,0)
/* ls->output_format.depth set to V3D_DEPTH_FORMAT_INVALID for S8 */
#endif
extern void gfx_buffer_translate_tlb_ldst(struct v3d_tlb_ldst_params *ls,
   v3d_addr_t base_addr, const GFX_BUFFER_DESC_T *desc, uint32_t plane_i, uint32_t slice, bool color,
   bool tlb_ms, bool ext_ms, v3d_dither_t dither);

// Get TFU input format and stride from desc.
extern v3d_tfu_iformat_t gfx_buffer_desc_get_tfu_iformat_and_stride(
   uint32_t* stride,
   GFX_BUFFER_DESC_T const* desc,
   unsigned plane_index
   );

// Get TFU output format and padding from desc.
extern v3d_tfu_oformat_t gfx_buffer_desc_get_tfu_oformat_and_height_pad_in_ub(
   uint32_t* o_height_pad_in_ub,
   GFX_BUFFER_DESC_T const* desc,
   unsigned plane_index
   );

EXTERN_C_END
