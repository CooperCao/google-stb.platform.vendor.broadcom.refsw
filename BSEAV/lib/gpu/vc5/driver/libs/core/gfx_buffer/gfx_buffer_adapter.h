/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/lfmt/lfmt_block.h"

EXTERN_C_BEGIN

#if WANT_THIRDPARTY
/* Just declare functions here; they will be defined by gfx_buffer_adapter_*.cpp */
#define GFX_BUFFER_ADAPTER_LINKAGE extern
#define GFX_BUFFER_ADAPTER_BODY(DUMMY_RET) ;
#else
/* Define dummy functions here */
#define GFX_BUFFER_ADAPTER_LINKAGE static inline
#define GFX_BUFFER_ADAPTER_BODY(DUMMY_RET) { not_impl(); DUMMY_RET; }
#endif

GFX_BUFFER_ADAPTER_LINKAGE GFX_LFMT_T gfx_buffer_adapter_etc_get_decompressed_fmt(
   GFX_LFMT_T compressed_fmt) GFX_BUFFER_ADAPTER_BODY(return GFX_LFMT_NONE)
GFX_BUFFER_ADAPTER_LINKAGE GFX_LFMT_T gfx_buffer_adapter_astc_get_decompressed_fmt(
   GFX_LFMT_T compressed_fmt) GFX_BUFFER_ADAPTER_BODY(return GFX_LFMT_NONE)

/* Decompress an ETC/ASTC compressed block.
 * These functions expect pxs to be setup already; they will only write
 * pxs->fmt and *pxs->u. The dimensions of pxs should match the dimensions of
 * the compressed block. */
GFX_BUFFER_ADAPTER_LINKAGE void gfx_buffer_adapter_etc_decompress_block(
   GFX_LFMT_BLOCK_ARR_T *pxs, const GFX_LFMT_BLOCK_T *block) GFX_BUFFER_ADAPTER_BODY(return)
GFX_BUFFER_ADAPTER_LINKAGE void gfx_buffer_adapter_astc_decompress_block(
   GFX_LFMT_BLOCK_ARR_T *pxs, const GFX_LFMT_BLOCK_T *block) GFX_BUFFER_ADAPTER_BODY(return)

EXTERN_C_END
