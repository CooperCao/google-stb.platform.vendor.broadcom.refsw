/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/lfmt/lfmt_block.h"

EXTERN_C_BEGIN

extern GFX_LFMT_T gfx_buffer_etc_get_decompressed_fmt(
   GFX_LFMT_T compressed_fmt);

/* pxs should be setup by the caller; this function will only write pxs->fmt
 * and *pxs->u. The dimensions of pxs should match the dimensions of the
 * compressed block. */
extern void gfx_buffer_etc_decompress_block(
   GFX_LFMT_BLOCK_ARR_T *pxs, const GFX_LFMT_BLOCK_T *block);

EXTERN_C_END
