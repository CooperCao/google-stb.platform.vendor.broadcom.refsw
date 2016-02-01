/*==============================================================================
 Copyright (c) 2015 Broadcom Europe Limited.
 All rights reserved.
==============================================================================*/

#include "gfx_buffer_slow_conv_compr.h"
#include "gfx_buffer_adapter.h"
#include "gfx_buffer_bc.h"
#include "gfx_buffer_bstc.h"

#include "helpers/assert.h"

GFX_LFMT_T gfx_buffer_get_decompressed_fmt(GFX_LFMT_T compressed_fmt)
{
   if (gfx_lfmt_is_etc_family(compressed_fmt))
      return gfx_buffer_adapter_etc_get_decompressed_fmt(compressed_fmt);
   if (gfx_lfmt_is_astc_family(compressed_fmt))
      return gfx_buffer_adapter_astc_get_decompressed_fmt(compressed_fmt);
   if (gfx_lfmt_is_bc_family(compressed_fmt))
      return gfx_buffer_bc_get_decompressed_fmt(compressed_fmt);
   if (gfx_lfmt_is_bstc_family(compressed_fmt))
      return gfx_buffer_bstc_get_decompressed_fmt(compressed_fmt);
   unreachable();
   return GFX_LFMT_NONE;
}

/* Each compressed format must be handled by at least one of
 * decompress_block_direct()/get_pixel_from_compr_block_direct() */

static bool decompress_block_direct(
   GFX_LFMT_BLOCK_ARR_T *pxs, const GFX_LFMT_BLOCK_T *block)
{
   assert(gfx_lfmt_is_compressed(block->fmt));

   if (gfx_lfmt_is_etc_family(block->fmt))
      gfx_buffer_adapter_etc_decompress_block(pxs, block);
   else if (gfx_lfmt_is_astc_family(block->fmt))
      gfx_buffer_adapter_astc_decompress_block(pxs, block);
   else if (gfx_lfmt_is_bstc_family(block->fmt))
      gfx_buffer_bstc_decompress_block(pxs, block);
   else
      return false;

   return true;
}

static bool get_pixel_from_compr_block_direct(
   GFX_LFMT_BLOCK_T *px, const GFX_LFMT_BLOCK_T *block,
   /* Block-relative coords */
   uint32_t x, uint32_t y, uint32_t z)
{
   assert(gfx_lfmt_is_compressed(block->fmt));

   if (gfx_lfmt_is_bc_family(block->fmt))
   {
      assert(z == 0);
      gfx_buffer_bc_get_pixel_from_block(px, block, x, y);
   }
   else
      return false;

   return true;
}

void gfx_buffer_decompress_block(
   GFX_LFMT_BLOCK_ARR_T *pxs, const GFX_LFMT_BLOCK_T *block)
{
   if (decompress_block_direct(pxs, block))
   {
      return;
   }

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, block->fmt);

   assert((pxs->w == bd.block_w) && (pxs->h == bd.block_h) && (pxs->d == bd.block_d));

   pxs->fmt = gfx_buffer_get_decompressed_fmt(block->fmt);
   for (uint32_t z = 0; z != bd.block_d; ++z)
      for (uint32_t y = 0; y != bd.block_h; ++y)
         for (uint32_t x = 0; x != bd.block_w; ++x)
         {
            GFX_LFMT_BLOCK_T px;
            verif(get_pixel_from_compr_block_direct(&px, block, x, y, z)); /* At least one _direct() func must succeed! */
            assert(px.fmt == pxs->fmt);
            *gfx_lfmt_block_arr_elem(pxs, x, y, z) = px.u;
         }
}

void gfx_buffer_compress_block(
   GFX_LFMT_BLOCK_T *block, const GFX_LFMT_BLOCK_ARR_T *pxs)
{
   if (gfx_lfmt_is_bstc_family(block->fmt))
      gfx_buffer_bstc_compress_block(block, pxs);
   else
      unreachable();
}

void gfx_buffer_get_pixel_from_compr_block(
   GFX_LFMT_BLOCK_T *px, const GFX_LFMT_BLOCK_T *block,
   /* Block-relative coords */
   uint32_t x, uint32_t y, uint32_t z)
{
   if (get_pixel_from_compr_block_direct(px, block, x, y, z))
   {
      return;
   }

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, block->fmt);

   GFX_LFMT_BLOCK_ARR_T pxs;
   GFX_LFMT_BLOCK_ARR_ALLOCA(&pxs, bd.block_w, bd.block_h, bd.block_d);
   verif(decompress_block_direct(&pxs, block)); /* At least one _direct() func must succeed! */

   px->fmt = pxs.fmt;
   px->u = *gfx_lfmt_block_arr_elem(&pxs, x, y, z);
}
