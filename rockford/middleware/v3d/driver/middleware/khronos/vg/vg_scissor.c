/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  VG scissor

FILE DESCRIPTION
VG scissor allocation and generation implementation.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/vg/vg_scissor.h"
#include "middleware/khronos/common/khrn_image.h"
#include "interface/khronos/common/khrn_int_util.h"
#include <string.h>

MEM_HANDLE_T vg_scissor_alloc(void)
{
   return mem_alloc_ex(
      0, /* don't allocate yet, just get a handle */
      VG_SCISSOR_ALIGN,
      (MEM_FLAG_T)(MEM_FLAG_DISCARDABLE | MEM_FLAG_DIRECT | MEM_FLAG_RESIZEABLE | MEM_FLAG_ZERO | MEM_FLAG_INIT),
      "vg_scissor",
      MEM_COMPACT_DISCARD);
}

static void vg_scissor_gen(
   void *data, uint32_t stride, int32_t *bounds,
   KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height,
   const int32_t *rects, uint32_t rects_count)
{
   KHRN_IMAGE_WRAP_T wrap;
   uint32_t i;

   /* data already initialised to 0 by virtue of MEM_FLAG_ZERO and MEM_FLAG_INIT */
   bounds[0] = width;
   bounds[1] = height;
   bounds[2] = 0;
   bounds[3] = 0;

   khrn_image_interlock_wrap(&wrap, format, width, height, stride, 0, data, NULL);
   for (i = 0; i != rects_count; i += 4) {
      int32_t rect_x      = rects[i];
      int32_t rect_y      = rects[i + 1];
      int32_t rect_width  = rects[i + 2];
      int32_t rect_height = rects[i + 3];
      khrn_clip_rect(
         &rect_x, &rect_y, &rect_width, &rect_height,
         0, 0, width, height);
      khrn_image_wrap_clear_region(&wrap, rect_x, rect_y, rect_width, rect_height, 0xffffffff, IMAGE_CONV_GL);
      bounds[0] = _min(bounds[0], rect_x);
      bounds[1] = _min(bounds[1], rect_y);
      bounds[2] = _max(bounds[2], rect_x + rect_width);
      bounds[3] = _max(bounds[3], rect_y + rect_height);
   }
}

bool vg_scissor_retain(
   MEM_HANDLE_T handle, int32_t *bounds,
   uint32_t width, uint32_t height,
   const int32_t *rects, uint32_t rects_count)
{
   if (!mem_retain(handle)) {
      KHRN_IMAGE_FORMAT_T format = vg_scissor_get_format(width, height);
      uint32_t stride = khrn_image_get_stride(format, width);
      if (!mem_resize_ex(handle, khrn_image_pad_height(format, height) * stride, MEM_COMPACT_DISCARD)) {
         mem_unretain(handle);
         return false;
      }
      vg_scissor_gen(mem_lock(handle, NULL), stride, bounds, format, width, height, rects, rects_count);
      mem_unlock(handle);
   }
   return true;
}
