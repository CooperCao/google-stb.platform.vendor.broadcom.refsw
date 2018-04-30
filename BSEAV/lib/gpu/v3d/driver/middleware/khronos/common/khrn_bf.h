/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "middleware/khronos/common/khrn_image.h"

extern bool khrn_bf_clear_region(
   KHRN_IMAGE_WRAP_T *wrap, uint32_t x, uint32_t y,
   uint32_t width, uint32_t height,
   uint32_t rgba /* rgba non-lin, unpre */);

extern bool khrn_bf_copy_region(
   /* dst/src regions may overlap */
   KHRN_IMAGE_WRAP_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_WRAP_T *src, uint32_t src_x, uint32_t src_y);

extern bool khrn_bf_copy_scissor_regions(
   /* dst/src regions may overlap */
   KHRN_IMAGE_WRAP_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_WRAP_T *src, uint32_t src_x, uint32_t src_y,
   const int32_t *scissor_rects, uint32_t scissor_rects_count);

extern bool khrn_bf_subsample(
   KHRN_IMAGE_WRAP_T *dst,
   const KHRN_IMAGE_WRAP_T *src);
