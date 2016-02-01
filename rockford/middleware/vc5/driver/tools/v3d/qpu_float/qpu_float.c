/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  3D Tools
Module   :  2760 3D ("Penrose") Simulator

FILE DESCRIPTION
Emulates QPU floating point operations and 16-bit float packing/unpacking.
=============================================================================*/

#include "tools/v3d/qpu_float/qpu_float.h"
#include "helpers/gfx/gfx_util.h"
#include "helpers/v3d/v3d_common.h"
#include <assert.h>
#include <float.h>
#include <math.h>

/* There are 2 ways we can get NaN output from an operation:
 * - If one of the inputs is NaN. In this case we return PROPAGATED_NAN
 * - If the operation is invalid, eg -inf + inf or 0 * inf. In this case we
 *   return GENERATED_NAN */
#define PROPAGATED_NAN 0x7fc00000
#define GENERATED_NAN  0xffc00000

static uint32_t choose_nan(uint32_t a, uint32_t b)
{
   if (gfx_is_nan_bits(a) || gfx_is_nan_bits(b)) {
      return PROPAGATED_NAN;
   }
   return GENERATED_NAN;
}

uint32_t qpu_float_unpack16(uint32_t f16)
{
   uint32_t f32 = gfx_float_to_bits(gfx_float16_to_float(f16));
   if (gfx_is_nan_bits(f32))
      return PROPAGATED_NAN;
   return f32;
}

uint32_t qpu_float_pack16(uint32_t f32)
{
   if (gfx_is_nan_bits(f32))
      /* Preserve the sign bit */
      return (f32 & (1u << 31)) ? 0xfe00 : 0x7e00;
   return gfx_float_to_float16(gfx_float_from_bits(f32));
}

uint32_t qpu_fadd(uint32_t a, uint32_t b, bool prop_nan)
{
   float f = gfx_float_from_bits(a);
   float g = gfx_float_from_bits(b);

   float x = f + g;

   if (gfx_is_nan(x)) {
      return prop_nan ? choose_nan(a, b) : 0;
   }
   return gfx_float_to_bits(x);
}

uint32_t qpu_fsub(uint32_t a, uint32_t b, bool prop_nan)
{
   return qpu_fadd(a, b ^ 0x80000000, prop_nan);
}

/* See GFXH-1080 */
uint32_t qpu_fcmp(uint32_t a, uint32_t b, bool prop_nan)
{
   if (((a == GFX_INF_BITS) && (b == GFX_INF_BITS)) ||
      ((a == GFX_NEGINF_BITS) && (b == GFX_NEGINF_BITS)))
      return 0;

   return qpu_fsub(a, b, prop_nan);
}

uint32_t qpu_fmul(uint32_t a, uint32_t b, bool prop_nan)
{
   float f = gfx_float_from_bits(a);
   float g = gfx_float_from_bits(b);

   float x = f * g;
   if (gfx_is_nan(x)) {
      return prop_nan ? choose_nan(a, b) : 0;
   }
   return gfx_float_to_bits(x);
}

static bool float_less_than(uint32_t a, uint32_t b)
{
   int nega = (a & 0x80000000) != 0;
   int negb = (b & 0x80000000) != 0;

   if (nega && !negb)
      return true;
   else if (!nega && negb)
      return false;
   else if (!nega && !negb)
      return (a & 0x7fffffff) < (b & 0x7fffffff);
   else
      return (a & 0x7fffffff) > (b & 0x7fffffff);
}

uint32_t qpu_fmin(uint32_t a, uint32_t b, bool prop_nan)
{
   if (gfx_is_nan_bits(a) && !gfx_is_nan_bits(b))
      return b;
   else if (gfx_is_nan_bits(b) && !gfx_is_nan_bits(a))
      return a;
   else if (gfx_is_nan_bits(a) && gfx_is_nan_bits(b))
      return prop_nan ? PROPAGATED_NAN : 0;
   else if (float_less_than(a, b))
      return a;
   else
      return b;
}

uint32_t qpu_fmax(uint32_t a, uint32_t b, bool prop_nan)
{
   if (gfx_is_nan_bits(a) && !gfx_is_nan_bits(b))
      return b;
   else if (gfx_is_nan_bits(b) && !gfx_is_nan_bits(a))
      return a;
   else if (gfx_is_nan_bits(a) && gfx_is_nan_bits(b))
      return prop_nan ? PROPAGATED_NAN : 0;
   else if (float_less_than(a, b))
      return b;
   else
      return a;
}

uint32_t qpu_fabs(uint32_t a)
{
   return a & 0x7fffffff;
}

uint32_t qpu_fround(uint32_t a, bool prop_nan)
{
   /*TODO: this may be nonsense*/
   float x, f, c;
   x = gfx_float_from_bits(a);
   if (gfx_is_nan(x))
      return prop_nan ? PROPAGATED_NAN : 0;
   else
   {
      f = floorf(x);
      c = ceilf(x);
      assert(f <= x && x <= c);
      if (x - f < c - x)
         return gfx_float_to_bits(f);
      else if (x - f > c - x)
         return gfx_float_to_bits(c);
      else if (f * 0.5f == floorf(f * 0.5f))
         return gfx_float_to_bits(f);
      else
         return gfx_float_to_bits(c);
   }
}

uint32_t qpu_ftrunc(uint32_t a, bool prop_nan)
{
   float f;
   f = gfx_float_from_bits(a);
   if (gfx_is_nan(f))
      return prop_nan ? PROPAGATED_NAN : 0;
   else
   {
      if (f >= 0)
         f = floorf(f);
      else
         f = ceilf(f);
      return gfx_float_to_bits(f);
   }
}

uint32_t qpu_ffloor(uint32_t a, bool prop_nan)
{
   if (gfx_is_nan_bits(a)) {
      return prop_nan ? PROPAGATED_NAN : 0;
   }
   return gfx_float_to_bits(floorf(gfx_float_from_bits(a)));
}

uint32_t qpu_fceil(uint32_t a, bool prop_nan)
{
   if (gfx_is_nan_bits(a)) {
      return prop_nan ? PROPAGATED_NAN : 0;
   }
   return gfx_float_to_bits(ceilf(gfx_float_from_bits(a)));
}

uint32_t qpu_ftoin(uint32_t a, bool *carry)
{
   *carry = false;
   if (gfx_is_nan_bits(a))
      return 0;
   else if (a >= 0xcf000000)
   {
      *carry = (a >= 0xcf000001);
      return 0x80000000; /* negative overflow */
   }
   else if (a >= 0x4f000000 && a < 0x80000000)
   {
      *carry = true;
      return 0x7fffffff; /* positive overflow */
   }
   else
   {
      /* Floating point to integer conversions generate 0 if the input is a
       * NaN. Get this behaviour by passing propagate_nans=false to qpu_fround */
      return (int)gfx_float_from_bits(qpu_fround(a, false));
   }
}

uint32_t qpu_ftoiz(uint32_t a, bool *carry)
{
   *carry = false;
   if (gfx_is_nan_bits(a))
      return 0;
   else if (a >= 0xcf000000)
   {
      *carry = (a >= 0xcf000001);
      return 0x80000000; /* negative overflow */
   }
   else if (a >= 0x4f000000 && a < 0x80000000)
   {
      *carry = true;
      return 0x7fffffff; /* positive overflow */
   }
   else
      return (int32_t)gfx_float_from_bits(a);
}

uint32_t qpu_ftouz(uint32_t a, bool *carry)
{
   *carry = false;
   if (gfx_is_nan_bits(a))
      return 0;
   else if (a >= 0x80000001)
   {
      *carry = true;
      return 0x00000000; /* saturate to zero for negative input */
   }
   else if (a >= 0x4f800000 && a < 0x80000000)
   {
      *carry = true;
      return 0xffffffff; /* positive overflow */
   }
   else
      return (uint32_t)gfx_float_from_bits(a);
}

uint32_t qpu_ftoc(uint32_t a, uint32_t quad_elnum)
{
   uint32_t x, y, i;
   uint32_t result;
   float f = gfx_is_nan_bits(a)? 0.0f : gfx_float_from_bits(a);  // map naN @ input to zero
   /* dither values are generated by casting integers below to floats, and then divide these by 17.0. The bits are stored, to make sure, we use the same table in hardware implementation.
                                     {          1,         9,         3,        11,        13,         5,        15,         7,         4,        12,         2,        10,        16,         8,        14,         6 } */
   const uint32_t dither_pattern[] = { 0x3d70f0f1,0x3f078788,0x3e34b4b5,0x3f25a5a6,0x3f43c3c4,0x3e969697,0x3f61e1e2,0x3ed2d2d3,0x3e70f0f1,0x3f34b4b5,0x3df0f0f1,0x3f169697,0x3f70f0f1,0x3ef0f0f1,0x3f52d2d3,0x3eb4b4b5 } ;

   result = 0;
   for (i = 0; i < 4; i++)
   {
      x = (quad_elnum & 1)    << 1 | (i & 1);
      y = (quad_elnum>>1 & 1) << 1 | (i>>1 & 1);
      if (f >= gfx_float_from_bits(dither_pattern[4 * y + x]))
      {
         result |= 1<<i;
      }
   }
   return result;
}

uint32_t qpu_itof(uint32_t a)
{
   /* Round-to-nearest with "even" tiebreak */
   float f = (float)(int)a;

   return gfx_float_to_bits(f);
}

uint32_t qpu_utof(uint32_t a)
{
   return gfx_float_to_bits((float)a);
}

#define L(x) qpu_float_unpack16((x) & 0xffff)
#define H(x) qpu_float_unpack16((x) >> 16)
#define V(f,a,b,p) qpu_vfpack(f(L(a), L(b), p), f(H(a), H(b), p))

uint32_t qpu_vfpack(uint32_t a, uint32_t b)
{
   return qpu_float_pack16(b) << 16 | qpu_float_pack16(a);
}

uint32_t qpu_vfmin(uint32_t a, uint32_t b, bool prop_nan)
{
   return V(qpu_fmin,a,b,prop_nan);
}

uint32_t qpu_vfmax(uint32_t a, uint32_t b, bool prop_nan)
{
   return V(qpu_fmax,a,b,prop_nan);
}

uint32_t qpu_vfmul(uint32_t a, uint32_t b, bool prop_nan)
{
   return V(qpu_fmul,a,b,prop_nan);
}
