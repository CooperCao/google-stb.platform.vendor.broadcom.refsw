/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#include "libs/core/lfmt/lfmt_block.h"

extern GFX_LFMT_T gfx_buffer_bc_get_decompressed_fmt(
   GFX_LFMT_T compressed_fmt);

extern void gfx_buffer_bc_get_pixel_from_block(
   GFX_LFMT_BLOCK_T *px, const GFX_LFMT_BLOCK_T *block,
   /* Block-relative coords */
   uint32_t x, uint32_t y);
