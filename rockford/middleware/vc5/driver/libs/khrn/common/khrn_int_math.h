/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Math

FILE DESCRIPTION
Standard math functions, possibly optimised for VC.
=============================================================================*/

#ifndef KHRN_INT_MATH_H
#define KHRN_INT_MATH_H

#include "khrn_int_util.h"
#include <math.h>
#ifndef __VIDEOCORE__  // threadsx/nucleus define LONG which clashses
#include "vcos.h"
#endif

#define PI 3.1415926535897932384626433832795f
#define SQRT_2 1.4142135623730950488016887242097f
#define EPS 1.0e-10f

static inline float floor_(float x)
{
   return floorf(x);
}

static inline float ceil_(float x)
{
   return ceilf(x);
}

/*
   Preconditions:

   -

   Postconditions:

   returns the magnitude of x. returns +infinity if x is infinite. returns a nan
   if x is nan.
*/

static inline float absf_(float x)
{
   return (x < 0.0f) ? -x : x;
}

static inline float modf_(float x, float y)
{
   return fmodf(x, y);
}

static inline void sin_cos_(float *s, float *c, float angle)
{
   *s = sinf(angle);
   *c = cosf(angle);
}

static inline float sin_(float angle)
{
   return sinf(angle);
}

static inline float cos_(float angle)
{
   return cosf(angle);
}

static inline float atan2_(float y, float x)
{
   return atan2f(y, x);
}

extern float acos_(float x);

static inline float exp_(float x)
{
   return expf(x);
}

extern float mod_one_(float x);

#ifdef _VIDEOCORE
#include <vc/intrinsics.h>

static inline float nan_recip_(float x)
{
   float est = _frecip(x);          /* initial estimate, accurate to ~12 bits */
   return est * (2.0f - (x * est)); /* refine with newton-raphson to get accuracy to ~24 bits */
}

static inline float rsqrt_(float x)
{
   assert(x > 0.0f);
   float est = _frsqrt(x);                         /* initial estimate, accurate to ~12 bits */
   return est * (1.5f - ((x * 0.5f) * est * est)); /* refine with newton-raphson to get accuracy to ~24 bits */
}
#else
static inline float nan_recip_(float x)
{
   return 1.0f / x;
}

static inline float rsqrt_(float x)
{
   assert(x > 0.0f);
   return 1.0f / sqrtf(x);
}
#endif

static inline float recip_(float x)
{
   assert(x != 0.0f);
   return nan_recip_(x);
}

static inline bool is_nan_(float x)
{
   uint32_t bits = float_to_bits(x);
   return ((bits & 0x7f800000) == 0x7f800000) && /* max exponent */
      (bits << 9); /* non-zero mantissa */
}

static inline bool nan_lt_(float x, float y)
{
   return
#ifndef KHRN_NAN_COMPARISONS_CORRECT
      !is_nan_(x) && !is_nan_(y) &&
#endif
      (x < y);
}

static inline bool nan_gt_(float x, float y)
{
   return
#ifndef KHRN_NAN_COMPARISONS_CORRECT
      !is_nan_(x) && !is_nan_(y) &&
#endif
      (x > y);
}

static inline bool nan_ne_(float x, float y)
{
   return
#ifndef KHRN_NAN_COMPARISONS_CORRECT
      !is_nan_(x) && !is_nan_(y) &&
#endif
      (x != y);
}

static inline float sqrt_(float x)
{
   assert(!nan_lt_(x, 0.0f));
   return sqrtf(x);
}

#endif
