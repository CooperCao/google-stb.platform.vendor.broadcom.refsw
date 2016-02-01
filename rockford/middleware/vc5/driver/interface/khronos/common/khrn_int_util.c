/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Util

FILE DESCRIPTION
Misc utility functions.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "interface/khronos/glxx/gl_public_api.h"

void khrn_clip_range(
   int32_t *min_x0_io, int32_t *l0_io,
   int32_t min_x1, int32_t l1)
{
   int32_t min_x0;
   int32_t l0;
   int32_t max_x0;
   int32_t max_x1;

   assert((*l0_io >= 0) && (l1 >= 0));

   min_x0 = *min_x0_io;
   l0 = *l0_io;

   max_x0 = _adds(min_x0, l0);
   max_x1 = min_x1 + l1;

   if (min_x0 < min_x1) {
      min_x0 = min_x1;
   }
   if (max_x0 > max_x1) {
      max_x0 = max_x1;
   }

   l0 = MAX(_subs(max_x0, min_x0), 0);

   *min_x0_io = (l0 == 0) ? min_x1 : min_x0;
   *l0_io = l0;
}

void khrn_clip_range2(
   int32_t *min_ax0_io, int32_t *min_bx0_io, int32_t *l0_io,
   int32_t min_ax1, int32_t al1,
   int32_t min_bx1, int32_t bl1)
{
   int32_t min_ax0;
   int32_t min_bx0;
   int32_t l0;
   int32_t max_ax0;
   int32_t max_bx0;
   int32_t max_ax1;
   int32_t max_bx1;

   assert((*l0_io >= 0) && (al1 >= 0) && (bl1 >= 0));

   min_ax0 = *min_ax0_io;
   min_bx0 = *min_bx0_io;
   l0 = *l0_io;

   l0 = _adds(MAX(min_ax0, min_bx0), l0) - MAX(min_ax0, min_bx0); /* clamp l0 to avoid overflow */
   max_ax0 = min_ax0 + l0;
   max_bx0 = min_bx0 + l0;
   max_ax1 = min_ax1 + al1;
   max_bx1 = min_bx1 + bl1;

   /*
      if any of the following _adds/_subs overflow, we will get either
      min_ax0 >= max_ax0 or min_bx0 >= max_bx0, so l0 will be 0
   */

   if (min_ax0 < min_ax1) {
      min_bx0 = _adds(min_bx0, _subs(min_ax1, min_ax0));
      min_ax0 = min_ax1;
   }
   if (max_ax0 > max_ax1) {
      max_bx0 = _subs(max_bx0, _subs(max_ax0, max_ax1));
      max_ax0 = max_ax1;
   }

   if (min_bx0 < min_bx1) {
      min_ax0 = _adds(min_ax0, _subs(min_bx1, min_bx0));
      min_bx0 = min_bx1;
   }
   if (max_bx0 > max_bx1) {
      max_ax0 = _subs(max_ax0, _subs(max_bx0, max_bx1));
      max_bx0 = max_bx1;
   }

   l0 = MAX(_subs(max_ax0, min_ax0), 0);
   assert(l0 == MAX(_subs(max_bx0, min_bx0), 0));

   *min_ax0_io = (l0 == 0) ? min_ax1 : min_ax0;
   *min_bx0_io = (l0 == 0) ? min_bx1 : min_bx0;
   *l0_io = l0;
}

void khrn_clip_rect(
   int32_t *x0, int32_t *y0, int32_t *w0, int32_t *h0,
   int32_t x1, int32_t y1, int32_t w1, int32_t h1)
{
   khrn_clip_range(x0, w0, x1, w1);
   khrn_clip_range(y0, h0, y1, h1);
}

void khrn_clip_rect2(
   int32_t *ax0, int32_t *ay0, int32_t *bx0, int32_t *by0, int32_t *w0, int32_t *h0,
   int32_t ax1, int32_t ay1, int32_t aw1, int32_t ah1,
   int32_t bx1, int32_t by1, int32_t bw1, int32_t bh1)
{
   khrn_clip_range2(ax0, bx0, w0, ax1, aw1, bx1, bw1);
   khrn_clip_range2(ay0, by0, h0, ay1, ah1, by1, bh1);
}

unsigned int khrn_get_type_size(int type, int count)
{
   switch ((GLenum)type) {
   case GL_BYTE:
   case GL_UNSIGNED_BYTE:
      return 1 * count;
   case GL_SHORT:
   case GL_UNSIGNED_SHORT:
   case GL_HALF_FLOAT_OES:
   case GL_HALF_FLOAT:
      return 2 * count;
   case GL_FIXED:
   case GL_FLOAT:
   case GL_INT:
   case GL_UNSIGNED_INT:
      return 4 * count;
   case GL_INT_2_10_10_10_REV:
   case GL_UNSIGNED_INT_2_10_10_10_REV:
      return 4;
   default:
      UNREACHABLE();
      return 0;
   }
}

void khrn_dma_memset(void *dest, uint32_t val, uint32_t size)
{
   memset(dest,val,size);
}
