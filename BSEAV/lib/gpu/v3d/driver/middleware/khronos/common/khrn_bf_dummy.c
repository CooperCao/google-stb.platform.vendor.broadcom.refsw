/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/khrn_bf.h"

bool khrn_bf_clear_region(
   KHRN_IMAGE_WRAP_T *wrap, uint32_t x, uint32_t y,
   uint32_t width, uint32_t height,
   uint32_t rgba)
{
   UNUSED(wrap);
   UNUSED(x);
   UNUSED(y);
   UNUSED(width);
   UNUSED(height);
   UNUSED(rgba);

   return false;
}

bool khrn_bf_copy_region(
   KHRN_IMAGE_WRAP_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_WRAP_T *src, uint32_t src_x, uint32_t src_y)
{
   UNUSED(dst);
   UNUSED(dst_x);
   UNUSED(dst_y);
   UNUSED(width);
   UNUSED(height);
   UNUSED(src);
   UNUSED(src_x);
   UNUSED(src_y);

   return false;
}

bool khrn_bf_copy_scissor_regions(
   KHRN_IMAGE_WRAP_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_WRAP_T *src, uint32_t src_x, uint32_t src_y,
   const int32_t *scissor_rects, uint32_t scissor_rects_count)
{
   UNUSED(dst);
   UNUSED(dst_x);
   UNUSED(dst_y);
   UNUSED(width);
   UNUSED(height);
   UNUSED(src);
   UNUSED(src_x);
   UNUSED(src_y);
   UNUSED(scissor_rects);
   UNUSED(scissor_rects_count);

   return false;
}

bool khrn_bf_subsample(
   KHRN_IMAGE_WRAP_T *dst,
   const KHRN_IMAGE_WRAP_T *src)
{
   UNUSED(dst);
   UNUSED(src);

   return false;
}
