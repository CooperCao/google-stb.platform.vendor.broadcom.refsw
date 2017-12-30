/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_imgconv_internal.h"

#ifdef USE_IMAGECONV

#include <imageconv.h>
#include <imageconv_consts.h>

static bool can_accelerate(GFX_LFMT_T lfmt, unsigned int swizzle)
{
   /* Only support 2D images */
   if (!gfx_lfmt_is_2d(lfmt))
      return false;

   /* With the correct swizzling */
   if (gfx_lfmt_get_swizzling(&lfmt) != swizzle)
      return false;

   /* X's in the format need special handling which the accelerated path is not doing */
   if (gfx_lfmt_has_x(lfmt))
      return false;

   /* And is RGBish */
   switch(gfx_lfmt_get_channels(&lfmt))
   {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
      break;
   default:
      return false;
   }

   return true;
}

static unsigned int bit_width(GFX_LFMT_T lfmt)
{
   switch(gfx_lfmt_get_base(&lfmt))
   {
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8C8C8C8:
      return 32;
   case GFX_LFMT_BASE_C8_C8_C8:
      return 24;
   case GFX_LFMT_BASE_C4C4C4C4:
   case GFX_LFMT_BASE_C5C6C5:
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C8C8:
      return 16;
   case GFX_LFMT_BASE_C8:
      return 8;
   default:
      return 0;
   }
}

static bool is_sub_image_copying(const struct v3d_imgconv_base_tgt *tgt,
                               unsigned int width, unsigned int height)
{
   return tgt->x || tgt->y ||
          tgt->desc->width != width || tgt->desc->height != height;
}

static bool can_use_uif_raster_convert(const struct v3d_imgconv_base_tgt *src,
   const struct v3d_imgconv_base_tgt *dst,
   unsigned int src_bpp, unsigned int dst_bpp,
   unsigned int width, unsigned int height, unsigned int depth)
{
   if (src->z != 0 || dst->z != 0)
      return false;

   if (!src_bpp || src_bpp != dst_bpp || depth != 1)
      return false;

   if (src_bpp != 8 && src_bpp != 16 && src_bpp != 32)
      return false;

   if (is_sub_image_copying(src, width, height) ||
       is_sub_image_copying(dst, width, height))
      return false;

   if (gfx_lfmt_get_type(&src->desc->planes[0].lfmt) !=
         gfx_lfmt_get_type(&dst->desc->planes[0].lfmt))
      return false;

   if (!can_accelerate(src->desc->planes[0].lfmt, GFX_LFMT_SWIZZLING_UIF) &&
       !can_accelerate(src->desc->planes[0].lfmt, GFX_LFMT_SWIZZLING_UIF_XOR))
      return false;

   if (!can_accelerate(dst->desc->planes[0].lfmt, GFX_LFMT_SWIZZLING_RSO))
      return false;

   return true;
}

static bool can_use_raster_uif_convert(const struct v3d_imgconv_base_tgt *src,
   const struct v3d_imgconv_base_tgt *dst,
   unsigned int src_bpp, unsigned int dst_bpp,
   unsigned int width, unsigned int height, unsigned int depth)
{
   /* z is used to look at the index for planes, we only send a single plane
      to each conversion back end now so leave the check in for now. */
   if (src->z != 0 || dst->z != 0)
      return false;

   if (!src_bpp || src_bpp != dst_bpp || depth != 1)
      return false;

   if (src_bpp != 8 && src_bpp != 16 && src_bpp != 32)
      return false;

   if (gfx_lfmt_get_type(&src->desc->planes[0].lfmt) !=
         gfx_lfmt_get_type(&dst->desc->planes[0].lfmt))
      return false;

   if (!can_accelerate(src->desc->planes[0].lfmt, GFX_LFMT_SWIZZLING_RSO))
      return false;

   if (!can_accelerate(dst->desc->planes[0].lfmt, GFX_LFMT_SWIZZLING_UIF) &&
       !can_accelerate(dst->desc->planes[0].lfmt, GFX_LFMT_SWIZZLING_UIF_XOR))
      return false;

   return true;
}

static bool can_use_sand128_uif_convert(const struct v3d_imgconv_base_tgt *src,
   const struct v3d_imgconv_base_tgt *dst,
   unsigned int src_bpp, unsigned int dst_bpp,
   unsigned int width, unsigned int height, unsigned int depth)
{
   if (src->z != 0 || dst->z != 0)
      return false;

   if (depth != 1)
      return false;

   if (is_sub_image_copying(src, width, height) ||
       is_sub_image_copying(dst, width, height))
      return false;

   if (src->desc->planes[0].lfmt != GFX_LFMT_Y8_UNORM_2D_SAND_128)
      return false;

   GFX_LFMT_T tmp = dst->desc->planes[0].lfmt;
   gfx_lfmt_set_swizzling(&tmp, GFX_LFMT_SWIZZLING_NONE);
   if ((tmp != GFX_LFMT_R8_G8_B8_X8_UNORM_2D) &&
       (tmp != GFX_LFMT_R8_G8_B8_A8_UNORM_2D))
      return false;

   if (!can_accelerate(dst->desc->planes[0].lfmt, GFX_LFMT_SWIZZLING_UIF) &&
       !can_accelerate(dst->desc->planes[0].lfmt, GFX_LFMT_SWIZZLING_UIF_XOR))
      return false;

   return true;
}

static bool can_use_yv12_uif_convert(const struct v3d_imgconv_base_tgt *src,
   const struct v3d_imgconv_base_tgt *dst,
   unsigned int src_bpp, unsigned int dst_bpp,
   unsigned int width, unsigned int height, unsigned int depth)
{
   if (src->z != 0 || dst->z != 0)
      return false;

   if (depth != 1)
      return false;

   if (is_sub_image_copying(src, width, height) ||
       is_sub_image_copying(dst, width, height))
      return false;

   /* check input is YVU420 planer */
   if ((src->desc->planes[0].lfmt != GFX_LFMT_Y8_UNORM_2D_RSO) ||
       (src->desc->planes[1].lfmt != GFX_LFMT_V8_2X2_UNORM_2D_RSO) ||
       (src->desc->planes[2].lfmt != GFX_LFMT_U8_2X2_UNORM_2D_RSO))
      return false;

   /* check output is RGBX or RGBA */
   if ((gfx_lfmt_get_channels(&dst->desc->planes[0].lfmt) != GFX_LFMT_CHANNELS_RGBX) &&
       (gfx_lfmt_get_channels(&dst->desc->planes[0].lfmt) != GFX_LFMT_CHANNELS_RGBA))
      return false;

   /* check output is 2D */
   if (gfx_lfmt_get_dimms(&dst->desc->planes[0].lfmt) != GFX_LFMT_DIMS_2D)
      return false;

   /* check output is UIF */
   if (!can_accelerate(dst->desc->planes[0].lfmt, GFX_LFMT_SWIZZLING_UIF) &&
       !can_accelerate(dst->desc->planes[0].lfmt, GFX_LFMT_SWIZZLING_UIF_XOR))
      return false;

   return true;
}

static bool can_use_yuv420_sp_uif_convert(const struct v3d_imgconv_base_tgt *src,
   const struct v3d_imgconv_base_tgt *dst,
   unsigned int src_bpp, unsigned int dst_bpp,
   unsigned int width, unsigned int height, unsigned int depth)
{
   if (src->z != 0 || dst->z != 0)
      return false;

   if (depth != 1)
      return false;

   if (is_sub_image_copying(src, width, height) ||
       is_sub_image_copying(dst, width, height))
      return false;

   /* check input is YVU420 semi-planer */
   if ((src->desc->planes[0].lfmt != GFX_LFMT_Y8_UNORM_2D_RSO) ||
       ((src->desc->planes[1].lfmt != GFX_LFMT_V8_U8_2X2_UNORM_2D_RSO) &&
       (src->desc->planes[1].lfmt != GFX_LFMT_U8_V8_2X2_UNORM_2D_RSO)))
      return false;

   /* check output is RGBX or RGBA */
   if ((gfx_lfmt_get_channels(&dst->desc->planes[0].lfmt) != GFX_LFMT_CHANNELS_RGBX) &&
       (gfx_lfmt_get_channels(&dst->desc->planes[0].lfmt) != GFX_LFMT_CHANNELS_RGBA))
      return false;

   /* check output is 2D */
   if (gfx_lfmt_get_dimms(&dst->desc->planes[0].lfmt) != GFX_LFMT_DIMS_2D)
      return false;

   /* check output is UIF */
   if (!can_accelerate(dst->desc->planes[0].lfmt, GFX_LFMT_SWIZZLING_UIF) &&
       !can_accelerate(dst->desc->planes[0].lfmt, GFX_LFMT_SWIZZLING_UIF_XOR))
      return false;

   return true;
}

static bool can_use_rgb_rso_convert(const struct v3d_imgconv_base_tgt *src,
   const struct v3d_imgconv_base_tgt *dst,
   unsigned int width, unsigned int height, unsigned int depth)
{
   /* z is used to look at the index for planes, we only send a single plane
      to each conversion back end now so leave the check in for now. */
   if (src->z != 0 || dst->z != 0)
      return false;

   if (is_sub_image_copying(src, width, height) ||
       is_sub_image_copying(dst, width, height))
      return false;

   if (src->desc->planes[0].lfmt != GFX_LFMT_R8_G8_B8_UNORM_2D_RSO)
      return false;

   if (dst->desc->planes[0].lfmt != GFX_LFMT_R8_G8_B8_X8_UNORM_2D_RSO)
      return false;

   return true;
}

bool claim_conversion(const struct v3d_imgconv_base_tgt *dst,
      const struct v3d_imgconv_base_tgt *src,
      unsigned int width, unsigned int height, unsigned int depth,
      const conversion_info_t *info)
{
   if (!v3d_imgconv_valid_cpu_conv_info(info))
      return false;

   unsigned int src_bpp, dst_bpp; /* bits per pixel */

   src_bpp = bit_width(src->desc->planes[0].lfmt);
   dst_bpp = bit_width(dst->desc->planes[0].lfmt);

   if (can_use_raster_uif_convert(src, dst, src_bpp, dst_bpp, width, height, depth))
      return true;

   if (can_use_uif_raster_convert(src, dst, src_bpp, dst_bpp, width, height, depth))
      return true;

   if (can_use_rgb_rso_convert(src, dst, width, height, depth))
      return true;

   return false;
}

static bool extra_neon_convert(
   const struct v3d_imgconv_base_tgt *dst, void *dst_data,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height, unsigned int depth)
{
   bool accel = false;
   unsigned int src_bpp, dst_bpp; /* bits per pixel */

   src_bpp = bit_width(src->desc->planes[0].lfmt);
   dst_bpp = bit_width(dst->desc->planes[0].lfmt);

   /*
    * Currently we only pass a few conversions to libimageconv.
    * TODO: This is conservative, we could pass more to the lib.
    */
   if (can_use_raster_uif_convert(src, dst, src_bpp, dst_bpp, width, height, depth))
   {
      unsigned int xor_offset = 0;
      unsigned int pad_rows = gfx_buffer_uif_height_pad_in_ub(dst->desc, 0);
      RECT_T subrect = { dst->x, dst->y, width, height };

      if (gfx_lfmt_get_swizzling(&dst->desc->planes[0].lfmt) == GFX_LFMT_SWIZZLING_UIF_XOR)
      {
         xor_offset = GFX_UIF_XOR_ADDR;
      }

      accel = raster_sub_uif_convert((uint8_t *)src_data + src->desc->planes[0].offset,
            (uint8_t *)dst_data + dst->desc->planes[0].offset,
            dst->desc->width, dst->desc->height,
            src_bpp, 0, pad_rows, xor_offset,
            src->desc->planes[0].pitch / (src_bpp / 8), &subrect) != 0;
   }
   else if (can_use_uif_raster_convert(src, dst, src_bpp, dst_bpp, width, height, depth))
   {
      unsigned int xor_offset = 0;
      unsigned int pad_rows = gfx_buffer_uif_height_pad_in_ub(src->desc, 0);

      if (gfx_lfmt_get_swizzling(&src->desc->planes[0].lfmt) == GFX_LFMT_SWIZZLING_UIF_XOR)
      {
         xor_offset = GFX_UIF_XOR_ADDR;
      }

      accel = uif_raster_convert((uint8_t *)src_data + src->desc->planes[0].offset,
            (uint8_t *)dst_data + dst->desc->planes[0].offset, width, height,
            src_bpp, 0, pad_rows, xor_offset,
            dst->desc->planes[0].pitch / (src_bpp / 8)) != 0;
   }
   else if (can_use_sand128_uif_convert(src, dst, src_bpp, dst_bpp, width, height, depth))
   {
      unsigned int xor_offset = 0;
      uint8_t *middle = malloc(width*height*4);

      if (!middle)
         return false;

      if (gfx_lfmt_get_swizzling(&dst->desc->planes[0].lfmt) == GFX_LFMT_SWIZZLING_UIF_XOR)
      {
         xor_offset = GFX_UIF_XOR_ADDR;
      }

      size_t success = sand128_rgbx_convert(&known_yuv_rgb_coefs[COLOURSPACE_BT601],
            &known_yuv_ranges[RANGE_FULL],
            (uint8_t *)src_data + src->desc->planes[0].offset,
            (uint8_t *)src_data + src->desc->planes[1].offset,
            (uint8_t *)middle,
            height,
            width,
            src->desc->planes[0].pitch / 128);

      if (success <= 0)
      {
         free(middle);
         return false;
      }

      accel = raster_uif_convert((uint8_t *)middle,
            (uint8_t *)dst_data + dst->desc->planes[0].offset, width, height,
            32, 0,
            (dst->desc->planes[0].pitch / (dst_bpp / 8) - height) / 8, xor_offset, width) != 0;

      free(middle);
   }
   else if (can_use_yv12_uif_convert(src, dst, src_bpp, dst_bpp, width, height, depth))
   {
      unsigned int xor_offset = 0;
      uint8_t *middle = malloc(width*height*4);

      if (!middle)
         return false;

      if (gfx_lfmt_get_swizzling(&dst->desc->planes[0].lfmt) == GFX_LFMT_SWIZZLING_UIF_XOR)
      {
         xor_offset = GFX_UIF_XOR_ADDR;
      }

      size_t success = yv12_rgbx_convert(&known_yuv_rgb_coefs[COLOURSPACE_BT601],
            &known_yuv_ranges[RANGE_FULL],
            (uint8_t *)src_data + src->desc->planes[0].offset,
            (uint8_t *)middle,
            height,
            width);

      if (success <= 0)
      {
         free(middle);
         return false;
      }

      accel = raster_uif_convert((uint8_t *)middle,
            (uint8_t *)dst_data + dst->desc->planes[0].offset, width, height,
            32, 0,
            (dst->desc->planes[0].pitch / (dst_bpp / 8) - height) / 8, xor_offset, width) != 0;

      free(middle);
   }
   else if (can_use_yuv420_sp_uif_convert(src, dst, src_bpp, dst_bpp, width, height, depth))
   {
      unsigned int xor_offset = 0;
      uint8_t *middle = malloc(width*height*4);

      if (!middle)
         return false;

      if (gfx_lfmt_get_swizzling(&dst->desc->planes[0].lfmt) == GFX_LFMT_SWIZZLING_UIF_XOR)
      {
         xor_offset = GFX_UIF_XOR_ADDR;
      }

      size_t success = yuv420_sp_rgbx_convert(&known_yuv_rgb_coefs[COLOURSPACE_BT601],
            &known_yuv_ranges[RANGE_FULL],
            (uint8_t *)src_data + src->desc->planes[0].offset,
            (uint8_t *)src_data + src->desc->planes[1].offset,
            (uint8_t *)middle,
            height,
            width);

      if (success <= 0)
      {
         free(middle);
         return false;
      }

      accel = raster_uif_convert((uint8_t *)middle,
            (uint8_t *)dst_data + dst->desc->planes[0].offset, width, height,
            32, 0,
            (dst->desc->planes[0].pitch / (dst_bpp / 8) - height) / 8, xor_offset, width) != 0;

      free(middle);
   }
   else if (can_use_rgb_rso_convert(src, dst, width, height, depth))
   {
      assert(src_bpp != 0);
      assert(dst_bpp != 0);

      accel = rgb_rso_convert((uint8_t *)src_data + src->desc->planes[0].offset,
            (uint8_t *)dst_data + dst->desc->planes[0].offset, width, height,
            src->desc->planes[0].pitch / (src_bpp / 8), RGB_RSO_TYPE_R8G8B8,
            dst->desc->planes[0].pitch / (dst_bpp / 8), RGB_RSO_TYPE_R8G8B8X8) != 0;
   }
   return accel;
}

static v3d_imgconv_methods extra_neon_path =
{
   .claim         = claim_conversion,
   .convert_async = NULL,
   .convert_sync  = extra_neon_convert,
   .convert_prep  = NULL
};
#endif

const v3d_imgconv_methods* get_extra_neon_path(void)
{
#ifdef USE_IMAGECONV
   return &extra_neon_path;
#else
   return NULL;
#endif
}
