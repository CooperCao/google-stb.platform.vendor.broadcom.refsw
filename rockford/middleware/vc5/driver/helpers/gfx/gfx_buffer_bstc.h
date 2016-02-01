/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/
#pragma once

#include "gfx_lfmt_block.h"

extern GFX_LFMT_T gfx_buffer_bstc_get_decompressed_fmt(GFX_LFMT_T compressed_fmt);

/* This function expects pxs to be setup already; it will only write pxs->fmt
 * and *pxs->u. The dimensions of pxs should match the dimensions of the
 * compressed block. */
extern void gfx_buffer_bstc_decompress_block(GFX_LFMT_BLOCK_ARR_T *pxs, const GFX_LFMT_BLOCK_T *block);

/* block->fmt should be set by the caller to the desired compression format.
 * The dimensions of pxs should match the dimensions of a compressed block. */
extern void gfx_buffer_bstc_compress_block(GFX_LFMT_BLOCK_T *block, const GFX_LFMT_BLOCK_ARR_T *pxs);
