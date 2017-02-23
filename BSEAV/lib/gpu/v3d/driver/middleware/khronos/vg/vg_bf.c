/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG blitting/filtering

FILE DESCRIPTION
Non-platform-specific parts of implementation.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/vg/vg_bf.h"
#include "interface/khronos/common/khrn_int_math.h"

uint32_t vg_bf_gblur_kernel_gen(int16_t *kernel, float std_dev)
{
   float tolerated_sum, oo_std_dev, exp_scale;
   uint32_t count;
   float sum, scale;
   uint32_t i;

   if (std_dev < EPS) {
      kernel[0] = 0x7fff;
      return 1;
   }

   /*
      G(x, s) = exp(-x^2 / 2s^2) / (sqrt(2pi) * s)
   */

   tolerated_sum = sqrt_(2.0f * PI) * std_dev * VG_BF_GBLUR_KERNEL_TOLERANCE;
   oo_std_dev = recip_(std_dev);
   exp_scale = -0.5f * oo_std_dev * oo_std_dev;

   count = 1;
   sum = 1.0f;
   do {
      float x;

      /*
         tolerated_sum is based on integral rather than sum of samples (which is
         what sum actually is)
         this works well enough for large std dev, but for very small std dev,
         tolerated_sum is too small
         to workaround this, we always do at least one iteration of the loop
         even if sum < tolerated_sum to begin with
      */

      count += 2;
      x = (float)(count >> 1);
      sum += 2.0f * exp_(x * x * exp_scale); /* times 2 for left and right */
   } while ((count != VG_BF_GBLUR_KERNEL_COUNT_MAX) && (sum < tolerated_sum));

   scale = recip_(sum);

   kernel[count >> 1] = (int16_t)_min(float_to_int_shift(scale, 15), 0x7fff);
   for (i = 0; i != (count >> 1); ++i) {
      float x = (float)((count >> 1) - i);
      kernel[i] = kernel[count - (i + 1)] =
         (int16_t)float_to_int_shift(exp_(x * x * exp_scale) * scale, 15);
   }

   return count;
}
