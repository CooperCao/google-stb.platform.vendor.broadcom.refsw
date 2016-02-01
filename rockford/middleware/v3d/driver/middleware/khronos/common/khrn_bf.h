/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Blitting/filtering

FILE DESCRIPTION
All functions declared in this file return false (and do nothing else) if the
requested operation cannot be accelerated on this platform. For this reason, you
should call the corresponding khrn_image_* functions (which will fallback to
pure c implementations) rather than calling these functions directly.
=============================================================================*/

#ifndef KHRN_BF_H
#define KHRN_BF_H

#include "middleware/khronos/common/khrn_image.h"

extern bool khrn_bf_clear_region(
   KHRN_IMAGE_WRAP_T *wrap, uint32_t x, uint32_t y,
   uint32_t width, uint32_t height,
   uint32_t rgba, /* rgba non-lin, unpre */
   KHRN_IMAGE_CONV_T conv);

extern bool khrn_bf_copy_region(
   /* dst/src regions may overlap */
   KHRN_IMAGE_WRAP_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_WRAP_T *src, uint32_t src_x, uint32_t src_y,
   KHRN_IMAGE_CONV_T conv);

extern bool khrn_bf_copy_scissor_regions(
   /* dst/src regions may overlap */
   KHRN_IMAGE_WRAP_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_WRAP_T *src, uint32_t src_x, uint32_t src_y,
   KHRN_IMAGE_CONV_T conv,
   const int32_t *scissor_rects, uint32_t scissor_rects_count);

extern bool khrn_bf_subsample(
   KHRN_IMAGE_WRAP_T *dst,
   const KHRN_IMAGE_WRAP_T *src);

#endif
