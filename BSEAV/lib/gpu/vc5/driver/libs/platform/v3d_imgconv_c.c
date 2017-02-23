/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/

#include "v3d_imgconv_internal.h"

static bool c_path_claim(
   const struct v3d_imgconv_base_tgt *dst, const struct v3d_imgconv_base_tgt *src,
   unsigned int width, unsigned int height, unsigned int depth, const security_info_t *sec_info)
{
   if (!v3d_imgconv_valid_cpu_sec_info(sec_info))
      return false;

   if (!(dst->desc.num_planes == 1 && src->desc.num_planes == 1))
      return false;

   GFX_LFMT_T dst_fmt = dst->desc.planes[0].lfmt;
   gfx_lfmt_set_yflip(&dst_fmt, GFX_LFMT_YFLIP_NOYFLIP);
   GFX_LFMT_T src_fmt = src->desc.planes[0].lfmt;
   gfx_lfmt_set_yflip(&src_fmt, GFX_LFMT_YFLIP_NOYFLIP);

   if (dst_fmt == GFX_LFMT_B8G8R8X8_UNORM_2D_RSO)
      return src_fmt == GFX_LFMT_R8_G8_B8_X8_UNORM_2D_RSO || src_fmt == GFX_LFMT_R8_G8_B8_A8_UNORM_2D_RSO;

   if (dst_fmt == GFX_LFMT_B8G8R8A8_UNORM_2D_RSO)
      return src_fmt == GFX_LFMT_R8_G8_B8_A8_UNORM_2D_RSO;

   return false;
}

static void c_path_convert(
   const struct v3d_imgconv_base_tgt *dst, void *dst_data,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height, unsigned int depth)
{
   GFX_LFMT_BASE_DETAIL_T dst_bd, src_bd;
   gfx_lfmt_base_detail(&dst_bd, dst->desc.planes[0].lfmt);
   gfx_lfmt_base_detail(&src_bd, src->desc.planes[0].lfmt);

   for (unsigned y = 0; y != height; ++y)
   {
      uint32_t *d = gfx_buffer_block_p(&dst->desc.planes[0], &dst_bd, dst_data,
         dst->x, dst->y + y, dst->z, dst->desc.height);
      const uint8_t *s = gfx_buffer_block_p(&src->desc.planes[0], &src_bd, src_data,
         src->x, src->y + y, src->z, src->desc.height);
      for (unsigned x = 0; x != width; ++x)
      {
         *d = gfx_pack_8888(s[2], s[1], s[0], s[3]);
         ++d;
         s += 4;
      }
   }
}

static const v3d_imgconv_methods c_path =
{
   .claim         = c_path_claim,
   .convert_async = NULL,
   .convert_sync  = c_path_convert,
   .convert_prep  = NULL
};

const v3d_imgconv_methods *get_c_path(void)
{
   return &c_path;
}
