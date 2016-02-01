/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Math

FILE DESCRIPTION
Standard math functions.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"

#include "interface/khronos/common/khrn_int_math.h"
#include "interface/khronos/common/khrn_int_util.h"

#define P1 1.570796e0f
#define P2 2.663139e0f
#define P3 1.349177e0f
#define P4 1.976203e-1f
#define P5 2.105075e-3f

#define Q1 1.000000e0f
#define Q2 2.332027e0f
#define Q3 1.843527e0f
#define Q4 5.578347e-1f
#define Q5 4.944801e-2f

float acos_(float x)
{
   float abs_x = absf_(x);
   float p = P1 + ((P2 + ((P3 + ((P4 + (P5 * abs_x)) * abs_x)) * abs_x)) * abs_x);
   float q = Q1 + ((Q2 + ((Q3 + ((Q4 + (Q5 * abs_x)) * abs_x)) * abs_x)) * abs_x);
   float acos_abs_x = sqrt_(_maxf(1.0f - (x * x), 0.0f)) * p * recip_(q);
   return (x < 0.0f) ? (PI - acos_abs_x) : acos_abs_x;
}

float mod_one_(float x)
{
   uint32_t m = float_to_bits(x) & 0x7fffff;
   int32_t e = ((float_to_bits(x) >> 23) & 0xff) - 127;
   uint32_t sign = float_to_bits(x) & 0x80000000, n;

   if (e < 0) { return x; }                      /* abs less then 1 (includes zeros/denormals) */
   if (e > 22) { return float_from_bits(sign); } /* no precision less than 1 */

   /*
      clear bits of mantissa corresponding to values >= 1
   */

   m &= ((1 << 23) >> e) - 1;

   /*
      normalise
   */

   if (m == 0) { return float_from_bits(sign); }

   n = 23 - _msb(m);
   e -= n;
   m <<= n;
   m &= ~(1 << 23);

   return float_from_bits(sign | ((e + 127) << 23) | m);
}
