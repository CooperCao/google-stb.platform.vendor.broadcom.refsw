/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/lfmt/lfmt_block.h"

EXTERN_C_BEGIN

extern GFX_LFMT_T gfx_buffer_get_decompressed_fmt(GFX_LFMT_T compressed_fmt);

/* Decompress a compressed block.
 * gfx_lfmt_is_compressed(block->fmt) must be true.
 * Assumes pxs has been setup already; will only write pxs->fmt and *pxs->u.
 * The dimensions of pxs should match the dimensions of the compressed block. */
extern void gfx_buffer_decompress_block(
   GFX_LFMT_BLOCK_ARR_T *pxs, const GFX_LFMT_BLOCK_T *block);

/* Compress a block of pixels.
 * block->fmt should be set by the caller to the desired compression format.
 * The dimensions of pxs should match the dimensions of a compressed block. */
extern void gfx_buffer_compress_block(
   GFX_LFMT_BLOCK_T *block, const GFX_LFMT_BLOCK_ARR_T *pxs);

extern void gfx_buffer_get_pixel_from_compr_block(
   GFX_LFMT_BLOCK_T *px, const GFX_LFMT_BLOCK_T *block,
   /* Block-relative coords */
   uint32_t x, uint32_t y, uint32_t z);

EXTERN_C_END
