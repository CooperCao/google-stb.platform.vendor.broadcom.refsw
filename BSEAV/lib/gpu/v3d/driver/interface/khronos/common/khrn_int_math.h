/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/common/khrn_int_util.h"
#include <math.h>
#include "interface/vcos/vcos.h"

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

static inline float nan_recip_(float x)
{
   return 1.0f / x;
}

static inline float rsqrt_(float x)
{
#ifndef NDEBUG
   vcos_verify(x > 0.0f);
#endif
   return 1.0f / sqrtf(x);
}

static inline float recip_(float x)
{
#ifndef NDEBUG
   vcos_verify(x != 0.0f);
#endif
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
