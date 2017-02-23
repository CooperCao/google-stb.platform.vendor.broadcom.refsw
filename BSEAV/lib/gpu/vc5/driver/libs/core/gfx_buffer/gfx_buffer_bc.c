/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.
=============================================================================*/

#include "gfx_buffer_bc.h"

static uint8_t decomp3bit_inner(
   const uint8_t x, const uint8_t y,
   const int16_t p0, const int16_t p1,
   const uint8_t *const addr,
   const int16_t wt1[8][2],
   /* const removed to workaround bogus GCC warning */
   int16_t wt2[8][3])
{
   const uint8_t pid = y *4 +x;
   const uint8_t aio = pid < 8 ? 0 : 3;
   const uint32_t aih = addr[2 +aio +0];
   const uint32_t aim = addr[2 +aio +1];
   const uint32_t ail = addr[2 +aio +2];
   const uint8_t ai = ((ail << 16 | aim << 8 | aih) >> (3 *(pid %8))) & 0x7;

   if (p0 > p1)
      return (p0 *wt1[ai][0] +p1 *wt1[ai][1] +3) /7;

   return (p0 *wt2[ai][0] +p1 *wt2[ai][1] +2) /5 +wt2[ai][2];
}

static uint8_t decomp3bit(
   const uint8_t x, const uint8_t y,
   const uint8_t *const addr,
   uint8_t snorm)
{
   const int16_t wt1[8][2] = {{7, 0}, {0, 7}, {6, 1}, {5, 2}, {4, 3}, {3, 4}, {2, 5}, {1, 6}};
   int16_t wt2[8][3] = {{5, 0, 0}, {0, 5, 0}, {4, 1, 0}, {3, 2, 0}, {2, 3, 0}, {1, 4, 0}, {0, 0, 0}, {0, 0, 255}};

   int16_t c0, c1;

   if (snorm)
   {
      c0 = ((int8_t*)addr)[0];
      c1 = ((int8_t*)addr)[1];
      wt2[6][2] = -128;
      wt2[7][2] = 127;
   }
   else
   {
      c0 = addr[0];
      c1 = addr[1];
   }

   return decomp3bit_inner(x, y, c0, c1, addr, wt1, wt2);
}

static uint16_t expand_5to8(uint16_t x) { return (x << 3) | (x >> 2); }
static uint16_t expand_6to8(uint16_t x) { return (x << 2) | (x >> 4); }

GFX_LFMT_T gfx_buffer_bc_get_decompressed_fmt(
   GFX_LFMT_T compressed_fmt)
{
   switch (compressed_fmt)
   {
   case GFX_LFMT_BC1_RGBA_UNORM:
   case GFX_LFMT_BC2_RGBA_UNORM:
   case GFX_LFMT_BC3_RGBA_UNORM:
      return GFX_LFMT_R8_G8_B8_A8_UNORM;
   case GFX_LFMT_BC1_RGBA_SRGB_SRGB_SRGB_UNORM:
   case GFX_LFMT_BC2_RGBA_SRGB_SRGB_SRGB_UNORM:
   case GFX_LFMT_BC3_RGBA_SRGB_SRGB_SRGB_UNORM:
      return GFX_LFMT_R8_G8_B8_A8_SRGB_SRGB_SRGB_UNORM;

   case GFX_LFMT_BC4_R_UNORM: return GFX_LFMT_R8_UNORM;
   case GFX_LFMT_BC4_R_SNORM: return GFX_LFMT_R8_SNORM;

   case GFX_LFMT_BC5_RG_UNORM: return GFX_LFMT_R8_G8_UNORM;
   case GFX_LFMT_BC5_RG_SNORM: return GFX_LFMT_R8_G8_SNORM;

   default:
      unreachable();
      return GFX_LFMT_NONE;
   }
}

void gfx_buffer_bc_get_pixel_from_block(
   GFX_LFMT_BLOCK_T *px, const GFX_LFMT_BLOCK_T *block,
   /* Block-relative coords */
   uint32_t x, uint32_t y)
{
   assert(x < 4);
   assert(y < 4);

   px->fmt = gfx_buffer_bc_get_decompressed_fmt(block->fmt);

   const GFX_LFMT_BASE_T bfmt = gfx_lfmt_get_base(&block->fmt);
   switch (bfmt)
   {
   case GFX_LFMT_BASE_BC1:
   case GFX_LFMT_BASE_BC2:
   case GFX_LFMT_BASE_BC3:
   {
      const uint8_t *const cbp = block->u.ui8 +(bfmt == GFX_LFMT_BASE_BC1 ? 0 : 8);

      const uint16_t c0l = cbp[0];
      const uint16_t c0h = cbp[1];
      const uint16_t c1l = cbp[2];
      const uint16_t c1h = cbp[3];

      const uint16_t c0 = c0h << 8 | c0l;
      const uint16_t bcomp0 = expand_5to8((c0l & 0x1F) >> 0);
      const uint16_t rcomp0 = expand_5to8((c0h & 0xF8) >> 3);
      const uint16_t gcomp0 = expand_6to8((c0 & 0x07E0) >> 5);

      const uint16_t c1 = c1h << 8 | c1l;
      const uint16_t bcomp1 = expand_5to8((c1l & 0x1F) >> 0);
      const uint16_t rcomp1 = expand_5to8((c1h & 0xF8) >> 3);
      const uint16_t gcomp1 = expand_6to8((c1 & 0x07E0) >> 5);

      const uint8_t ci =
         ((cbp[4 +y] >> (2 *x)) & 0x03) +
         /* (c0 <= c1) only has special behaviour for BC1. This is *not* what
          * MSDN says, but this *does* match the D3D reference implementation.
          * See also issue 6 here:
          * http://www.opengl.org/registry/specs/EXT/texture_compression_s3tc.txt */
         (((bfmt == GFX_LFMT_BASE_BC1) && (c0 <= c1)) ? 4 : 0);

      const uint16_t w[8][2] = {{6, 0}, {0, 6}, {4, 2}, {2, 4},
                                {6, 0}, {0, 6}, {3, 3}, {0, 0}};

      const uint16_t rcomp = ((rcomp0 * w[ci][0]) + (rcomp1 * w[ci][1])) / 6;
      const uint16_t gcomp = ((gcomp0 * w[ci][0]) + (gcomp1 * w[ci][1])) / 6;
      const uint16_t bcomp = ((bcomp0 * w[ci][0]) + (bcomp1 * w[ci][1])) / 6;

      assert(gfx_lfmt_get_base(&px->fmt) == GFX_LFMT_BASE_C8_C8_C8_C8);

      px->u.ui8[0] = (uint8_t)rcomp;
      px->u.ui8[1] = (uint8_t)gcomp;
      px->u.ui8[2] = (uint8_t)bcomp;

      // Alpha
      switch (bfmt)
      {
      case GFX_LFMT_BASE_BC1:
         px->u.ui8[3] = (uint8_t)((c0 <= c1 && ci == 7) ? 0x00 : 0xFF);
         break;
      case GFX_LFMT_BASE_BC2:
      {
         const uint8_t a = ((block->u.ui8[y *2 +x /2] >> ((x %2) *4))) &0x0F;
         px->u.ui8[3] = a << 4 | a;
         break;
      }
      case GFX_LFMT_BASE_BC3:
         px->u.ui8[3] = decomp3bit(x, y, block->u.ui8, false);
         break;
      default:
         unreachable();
      }

      break;
   }
   case GFX_LFMT_BASE_BC4:
   case GFX_LFMT_BASE_BC5:
   {
      bool snorm = gfx_lfmt_contains_snorm(block->fmt);

      assert(gfx_lfmt_get_base(&px->fmt) ==
         ((bfmt == GFX_LFMT_BASE_BC5) ? GFX_LFMT_BASE_C8_C8 : GFX_LFMT_BASE_C8));

      px->u.ui8[0] = decomp3bit(x, y, block->u.ui8, snorm);
      if (bfmt == GFX_LFMT_BASE_BC5)
         px->u.ui8[1] = decomp3bit(x, y, block->u.ui8 +8, snorm);

      break;
   }
   default:
      unreachable();
   }
}
