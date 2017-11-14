/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "middleware/khronos/common/2708/khrn_prod_4.h"
#include "middleware/khronos/glxx/2708/glxx_inner_4.h"

#include <assert.h>

#define TU_OFFSET_CS    0
#define TU_OFFSET_CFG   1
#define TU_OFFSET_DIM   2
#define TU_OFFSET_CPX   5

#define TU_MODE_TWOD    0
#define TU_MODE_CUBE    1


/*!
 * \brief Converts the image format supplied to the indicator format stored in the TU config.
 *
 * According to the specification, image formats are represented by integers between 0 and 12
 * in the TU configuration.
 *
 * \param format is the image format to convert.
 * \return Converted image format.
 */
static INLINE uint32_t tu_image_format_to_type(KHRN_IMAGE_FORMAT_T format)
{
   switch (format) {
   case ABGR_8888_TF: return 0;
   case RGBA_8888_TF: return 0;
   case BGRA_8888_TF: return 0;
   case XBGR_8888_TF: return 1;
   case RGBX_8888_TF: return 1;
   case RGBA_4444_TF: return 2;
   case RGBA_5551_TF: return 3;
   case RGB_565_TF: return 4;
   case L_8_TF: return 5;
   case A_8_TF: return 6;
   case AL_88_TF: return 7;
   case LA_88_TF: return 7;
   case ETC1_TF: return 8;
   case PALETTE_4_TF: return 9;
   case SAMPLE_8_TF: return 10;
   case SAMPLE_16_TF: return 11;
   case L_1_TF: return 12;
   case ABGR_8888_RSO:
   case ARGB_8888_RSO:
   case XBGR_8888_RSO:
   case XRGB_8888_RSO:
      return 16;
   case YUV_422_RSO: return 17;

   case ARGB_8888_TF: return 0;
   case XRGB_8888_TF: return 1;
   default:
      //Unreachable
      UNREACHABLE();
      return 0;
   }
}

static INLINE bool tu_image_format_rb_swap(KHRN_IMAGE_FORMAT_T format)
{
   switch (format) {
   case ABGR_8888_TF:
   case XBGR_8888_TF:
   case RGBA_8888_TF:
   case RGBX_8888_TF:
   case RGBA_4444_TF:
   case RGBA_5551_TF:
   case RGB_565_TF:
   case L_8_TF:
   case A_8_TF:
   case AL_88_TF:
   case LA_88_TF:
   case ETC1_TF:
   case PALETTE_4_TF:
   case SAMPLE_8_TF:
   case SAMPLE_16_TF:
   case L_1_TF:
   case ABGR_8888_RSO:
   case XBGR_8888_RSO:
   case RGB_565_RSO:
   case YUV_422_RSO:
   case YV12_RSO:
      return false;
   case ARGB_8888_TF:
   case XRGB_8888_TF:
   case BGRA_8888_TF:
   case ARGB_8888_RSO:
   case BGRA_8888_RSO:
   case BGR_565_RSO:
      return true;
   default:
      //Unreachable
      UNREACHABLE();
      return false;
   }
}


/*!
 * \brief Sets the texture unit configuration found at \a um_ptr.
 *
 * The texture unit configuration is stored in uniform memory and contains a number of flags,
 * stuffed into 4 (2 actually used) words. This function sets these flags.
 *
 * \param um_ptr is the pointer to the first of 4 words in uniform memory the config is to be
 * stored at.
 * \param wrapt is the T wrap setting.
 * \param wraps is the S wrap setting.
 * \param minfilt is the minification filter setting.
 * \param magfilt is the magnification filter setting.
 * \param miplvls is the number of mipmap levels.
 * \param format is the image format (unconverted, see tu_image_format_to_type).
 * \param img_type is the image type.
 * \param mode is the texture mode.
 * \param width is the texture width.
 * \param height is the texture height.
 * \param base_addr is the address to be set (texture base address).
 */

//unused

//static INLINE void glxx_hw_tu_set_config(GLXX_FIXABLE_BUF_T *um_ptr, uint32_t wrapt, uint32_t wraps, uint32_t minfilt, uint32_t magfilt, uint32_t miplvls, KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height, GLXX_FIXABLE_ADDR_T *base_addr)
//{
//   uint32_t type = tu_image_format_to_type(format);
//   uint32_t nonp2 = (width & (width-1)) != 0 || (height & (height-1)) != 0;
//   uint32_t etcflipy = 1;
//
//   assert(wrapt <= 3);
//   assert(wraps <= 3);
//   assert(miplvls <= 11);
//   assert(type <= 14);
////   assert((base_addr & 0xfff) == 0);
//
//  assert(minfilt <= 5);
//   assert(magfilt <= 1);
//
//   glxx_add_addr_plus(um_ptr, base_addr, nonp2 << 9 | type << 4 | miplvls);   /* Param0 (c_swiz and flipy set to 0) */
//   glxx_add_word(um_ptr, height << 20 | etcflipy << 19 | width << 8 | magfilt << 7 | minfilt << 4 | wrapt << 2 | wraps);   /* Param1 */
//}
