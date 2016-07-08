/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Implementation of client-side subset of Khronos image wrapper.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"

#include "interface/khronos/common/khrn_int_image.h"
#include "interface/khronos/common/khrn_int_util.h"

/******************************************************************************
formats
******************************************************************************/

uint32_t khrn_image_get_bpp(KHRN_IMAGE_FORMAT_T format)
{
   vcos_assert(format != IMAGE_FORMAT_INVALID);

   switch (format & IMAGE_FORMAT_COMP_MASK) {
   case IMAGE_FORMAT_UNCOMP:
      switch (format & IMAGE_FORMAT_PIXEL_SIZE_MASK) {
         case IMAGE_FORMAT_1:  return 1;
         case IMAGE_FORMAT_4:  return 4;
         case IMAGE_FORMAT_8:  return 8;
         case IMAGE_FORMAT_16: return 16;
         case IMAGE_FORMAT_24: return 24;
         case IMAGE_FORMAT_32: return 32;
         case IMAGE_FORMAT_64: return 64;
         default:              UNREACHABLE(); return 0;
      }
   case IMAGE_FORMAT_ETC1:        return 4;
   case IMAGE_FORMAT_PACKED_MASK: return 32;
   default:                       UNREACHABLE(); return 0;
   }
}

/*
   returns the number of red bits in each pixel of an image of the
   specified format, or zero if not an RGB or RGBA format
*/

uint32_t khrn_image_get_red_size(KHRN_IMAGE_FORMAT_T format)
{
   if (khrn_image_is_color(format) && (format & IMAGE_FORMAT_RGB)) {
      switch (format & (IMAGE_FORMAT_PIXEL_SIZE_MASK | IMAGE_FORMAT_PIXEL_LAYOUT_MASK)) {
      case (IMAGE_FORMAT_32 | IMAGE_FORMAT_8888): return 8;
      case (IMAGE_FORMAT_24 | IMAGE_FORMAT_888):  return 8;
      case (IMAGE_FORMAT_16 | IMAGE_FORMAT_4444): return 4;
      case (IMAGE_FORMAT_16 | IMAGE_FORMAT_5551): return 5;
      case (IMAGE_FORMAT_16 | IMAGE_FORMAT_565):  return 5;
      default:                                    UNREACHABLE(); return 0;
      }
   } else {
      return 0;
   }
}

/*
   returns the number of green bits in each pixel of an image of the
   specified format, or zero if not an RGB or RGBA format
*/

uint32_t khrn_image_get_green_size(KHRN_IMAGE_FORMAT_T format)
{
   if (khrn_image_is_color(format) && (format & IMAGE_FORMAT_RGB)) {
      switch (format & (IMAGE_FORMAT_PIXEL_SIZE_MASK | IMAGE_FORMAT_PIXEL_LAYOUT_MASK)) {
      case (IMAGE_FORMAT_32 | IMAGE_FORMAT_8888): return 8;
      case (IMAGE_FORMAT_24 | IMAGE_FORMAT_888):  return 8;
      case (IMAGE_FORMAT_16 | IMAGE_FORMAT_4444): return 4;
      case (IMAGE_FORMAT_16 | IMAGE_FORMAT_5551): return 5;
      case (IMAGE_FORMAT_16 | IMAGE_FORMAT_565):  return 6;
      default:                                    UNREACHABLE(); return 0;
      }
   } else {
      return 0;
   }
}

/*
   returns the number of blue bits in each pixel of an image of the
   specified format, or zero if not an RGB or RGBA format
*/

uint32_t khrn_image_get_blue_size(KHRN_IMAGE_FORMAT_T format)
{
   if (khrn_image_is_color(format) && (format & IMAGE_FORMAT_RGB)) {
      switch (format & (IMAGE_FORMAT_PIXEL_SIZE_MASK | IMAGE_FORMAT_PIXEL_LAYOUT_MASK)) {
      case (IMAGE_FORMAT_32 | IMAGE_FORMAT_8888): return 8;
      case (IMAGE_FORMAT_24 | IMAGE_FORMAT_888):  return 8;
      case (IMAGE_FORMAT_16 | IMAGE_FORMAT_4444): return 4;
      case (IMAGE_FORMAT_16 | IMAGE_FORMAT_5551): return 5;
      case (IMAGE_FORMAT_16 | IMAGE_FORMAT_565):  return 5;
      default:                                    UNREACHABLE(); return 0;
      }
   } else {
      return 0;
   }
}

/*
   returns the number of alpha bits in each pixel of an image of the
   specified format, or zero if not an alpha or RGBA format
*/

uint32_t khrn_image_get_alpha_size(KHRN_IMAGE_FORMAT_T format)
{
   if (khrn_image_is_color(format) && (format & IMAGE_FORMAT_A)) {
      switch (format & (IMAGE_FORMAT_PIXEL_SIZE_MASK | IMAGE_FORMAT_PIXEL_LAYOUT_MASK)) {
      case (IMAGE_FORMAT_32 | IMAGE_FORMAT_8888): return 8;
      case (IMAGE_FORMAT_16 | IMAGE_FORMAT_4444): return 4;
      case (IMAGE_FORMAT_16 | IMAGE_FORMAT_5551): return 1;
      case (IMAGE_FORMAT_16 | IMAGE_FORMAT_88):   return 8;
      case IMAGE_FORMAT_8:                        return 8;
      case IMAGE_FORMAT_4:                        return 4;
      case IMAGE_FORMAT_1:                        return 1;
      default:                                    UNREACHABLE(); return 0;
      }
   } else {
      return 0;
   }
}

/*
   returns the number of depth bits in each pixel of an image of the
   specified format, or zero if not a depth or depth+stencil format
*/

uint32_t khrn_image_get_z_size(KHRN_IMAGE_FORMAT_T format)
{
   if (khrn_image_is_depth(format) && (format & IMAGE_FORMAT_Z)) {
      if (format == DEPTH_32_TLBD || format == DEPTH_COL_64_TLBD)
         return 24;
      switch (format & IMAGE_FORMAT_PIXEL_SIZE_MASK) {
      case IMAGE_FORMAT_32: return 24;
      case IMAGE_FORMAT_16: return 16;
      default:              UNREACHABLE(); return 0;
      }
   } else {
      return 0;
   }
}

/*
   returns the number of stencil bits in each pixel of an image of the
   specified format, or zero if not a depth+stencil format
*/

uint32_t khrn_image_get_stencil_size(KHRN_IMAGE_FORMAT_T format)
{
   if (khrn_image_is_depth(format) && (format & IMAGE_FORMAT_STENCIL)) {
      if (format == DEPTH_32_TLBD || format == DEPTH_COL_64_TLBD)
         return 8;
      vcos_assert((format & IMAGE_FORMAT_PIXEL_SIZE_MASK) == IMAGE_FORMAT_32);
      return 8;
   } else {
      return 0;
   }
}

uint32_t khrn_image_get_log2_utile_width(KHRN_IMAGE_FORMAT_T format)
{
   vcos_assert(khrn_image_is_tformat(format) || khrn_image_is_lineartile(format));

   switch (format & IMAGE_FORMAT_COMP_MASK) {
   case IMAGE_FORMAT_UNCOMP:
   {
      switch (format & IMAGE_FORMAT_PIXEL_SIZE_MASK) {
      case IMAGE_FORMAT_1:  return 6; /* 64 * 8 */
      case IMAGE_FORMAT_4:  return 4; /* 16 * 8 */
      case IMAGE_FORMAT_8:  return 3; /* 8 * 8 */
      case IMAGE_FORMAT_16: return 3; /* 8 * 4 */
      case IMAGE_FORMAT_32: return 2; /* 4 * 4 */
      default:              UNREACHABLE(); return 0;
      }
   }
   case IMAGE_FORMAT_ETC1:        return 3; /* 8 * 16 */
   case IMAGE_FORMAT_PACKED_MASK: return 2; /* 4 * 4 */
   default:                       UNREACHABLE(); return 0;
   }
}

uint32_t khrn_image_get_log2_utile_height(KHRN_IMAGE_FORMAT_T format)
{
   vcos_assert(khrn_image_is_tformat(format) || khrn_image_is_lineartile(format));

   switch (format & IMAGE_FORMAT_COMP_MASK) {
   case IMAGE_FORMAT_UNCOMP:
   {
      switch (format & IMAGE_FORMAT_PIXEL_SIZE_MASK) {
      case IMAGE_FORMAT_1:  return 3; /* 64 * 8 */
      case IMAGE_FORMAT_4:  return 3; /* 16 * 8 */
      case IMAGE_FORMAT_8:  return 3; /* 8 * 8 */
      case IMAGE_FORMAT_16: return 2; /* 8 * 4 */
      case IMAGE_FORMAT_32: return 2; /* 4 * 4 */
      default:              UNREACHABLE(); return 0;
      }
   }
   case IMAGE_FORMAT_ETC1:        return 4; /* 8 * 16 */
   case IMAGE_FORMAT_PACKED_MASK: return 2; /* 4 * 4 */
   default:                       UNREACHABLE(); return 0;
   }
}

uint32_t khrn_image_get_log2_tile_width(KHRN_IMAGE_FORMAT_T format)
{
   return khrn_image_get_log2_utile_width(format) + 3;
}

uint32_t khrn_image_get_log2_tile_height(KHRN_IMAGE_FORMAT_T format)
{
   return khrn_image_get_log2_utile_height(format) + 3;
}

/******************************************************************************
image handling
******************************************************************************/

uint32_t khrn_image_pad_width(KHRN_IMAGE_FORMAT_T format, uint32_t width)
{
   vcos_assert(format != IMAGE_FORMAT_INVALID);

   switch (format & IMAGE_FORMAT_MEM_LAYOUT_MASK) {
   case IMAGE_FORMAT_RSO: return width;
   case IMAGE_FORMAT_TF:  return round_up(width, (uint32_t) (1 << khrn_image_get_log2_tile_width(format)));  /* pad to multiple of tiles */
   case IMAGE_FORMAT_LT:  return round_up(width, (uint32_t) (1 << khrn_image_get_log2_utile_width(format))); /* pad to multiple of utiles */
   case IMAGE_FORMAT_TLB_DUMP: return round_up(width, 64);
   default:               UNREACHABLE(); return 0;
   }
}

uint32_t khrn_image_pad_height(KHRN_IMAGE_FORMAT_T format, uint32_t height)
{
   vcos_assert(format != IMAGE_FORMAT_INVALID);

   if (khrn_image_is_packed_mask(format)) {
      height = khrn_image_get_packed_mask_height(height);
   }

   switch (format & IMAGE_FORMAT_MEM_LAYOUT_MASK) {
   case IMAGE_FORMAT_RSO: return height;
   case IMAGE_FORMAT_TF:  return round_up(height, (uint32_t) (1 << khrn_image_get_log2_tile_height(format)));  /* pad to multiple of tiles */
   case IMAGE_FORMAT_LT:  return round_up(height, (uint32_t) (1 << khrn_image_get_log2_utile_height(format))); /* pad to multiple of utiles */
   case IMAGE_FORMAT_TLB_DUMP: return round_up(height, 64);
   default:               UNREACHABLE(); return 0;
   }
}

uint32_t khrn_image_get_stride(KHRN_IMAGE_FORMAT_T format, uint32_t width)
{
   return (khrn_image_pad_width(format, width) * khrn_image_get_bpp(format)) >> 3;
}

uint32_t khrn_image_get_size(KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height)
{
   uint32_t size = khrn_image_get_stride(format, width) * khrn_image_pad_height(format, height);
   return size;
}

void khrn_image_interlock_wrap(KHRN_IMAGE_WRAP_T *wrap, KHRN_IMAGE_FORMAT_T format,
   uint32_t width, uint32_t height, int32_t stride,
   uint32_t flags, bool secure, void *storage, KHRN_INTERLOCK_T *interlock)
{
   wrap->format = format;
   wrap->width = (uint16_t)width;
   wrap->height = (uint16_t)height;
   wrap->stride = stride;
   wrap->flags = flags;
   wrap->secure = secure;
   wrap->palette = NULL;
   wrap->storage = storage;
   wrap->interlock = interlock;
}
