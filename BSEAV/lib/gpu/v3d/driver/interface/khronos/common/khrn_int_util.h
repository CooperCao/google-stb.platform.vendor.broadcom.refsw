/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <ctype.h>
#include <float.h>
#include <math.h>
#include <assert.h>
#include <stdint.h>

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/vcos/vcos.h"

/******************************************************************************
replacements for videocore intrinsics
******************************************************************************/

static inline int32_t _bmask(int32_t x, int32_t y)
{
   return x & ((1 << (y & 0x1f)) - 1);
}

static inline int32_t _min(int32_t x, int32_t y)
{
   return x < y ? x : y;
}

static inline int32_t _max(int32_t x, int32_t y)
{
   return x > y ? x : y;
}

#if defined(_MSC_VER)
static inline int32_t _msb(uint32_t x)
{
#if defined(__clang__)
   return x ? (31 - __builtin_clz(x)) : -1;
#else
   int32_t l = -1;

   if (x)
      __asm {
         bsr eax, x
         mov l, eax
      }

   return l;
#endif
}
#elif defined(__GNUC__)
static inline int32_t _msb(uint32_t x)
{
   return x ? (31 - __builtin_clz(x)) : -1;
}
#else
static inline int32_t _msb(uint32_t x) /* unsigned to get lsr */
{
   int32_t msb = -1;
   while (x != 0) {
      ++msb;
      x >>= 1;
   }
   return msb;
}
#endif

static inline uint32_t _count(uint32_t x)
{
   uint32_t count = 0;
   while (x != 0) {
      if (x & 0x1) { ++count; }
      x >>= 1;
   }
   return count;
}

static inline uint32_t _bitrev(uint32_t x, uint32_t y)
{
   uint32_t bitrev = 0;
   uint32_t i;
   for (i = 0; i != y; ++i) {
      bitrev |= ((x >> i) & 1) << (y - i - 1);
   }
   return bitrev;
}

static inline int32_t _adds(int32_t x, int32_t y)
{
   int32_t z = x + y;
   return (y > 0) ? ((z < x) ? (int32_t)0x7fffffff : z) : ((z > x) ? (int32_t)0x80000000 : z);
}

static inline int32_t _subs(int32_t x, int32_t y)
{
   int32_t z = x - y;
   return (y > 0) ? ((z > x) ? (int32_t)0x80000000 : z) : ((z < x) ? (int32_t)0x7fffffff : z);
}

static inline uint32_t _ror(uint32_t x, uint32_t y)
{
   return (x << (32 - y)) | (x >> y);
}

static inline float _minf(float x, float y)
{
   return x < y ? x : y;
}

static inline float _maxf(float x, float y)
{
   return x > y ? x : y;
}

/******************************************************************************
misc stuff
******************************************************************************/

#define ARR_COUNT(ARR) (sizeof(ARR) / sizeof(*(ARR)))

/* sign-extend 16-bit value with range [-0x4000, 0xbfff] */
static inline int32_t s_ext_off16(int32_t x)
{
   return ((int32_t)(int16_t)(x - 0x4000)) + 0x4000;
}

static inline bool is_power_of_2(uint32_t x)
{
   return (x != 0) && ((x & (x - 1)) == 0);
}

static inline uint32_t next_power_of_2(uint32_t x)
{
   return is_power_of_2(x) ? x : (uint32_t)(1 << (_msb(x) + 1));
}

static inline uint32_t round_up(uint32_t x, uint32_t y)
{
   assert(is_power_of_2(y));
   return (x + (y - 1)) & ~(y - 1);
}

static inline void *round_up_ptr(void *x, uint32_t y)
{
   assert(is_power_of_2(y));
   return (void *)(((uintptr_t)x + (uintptr_t)(y - 1)) & ~(uintptr_t)(y - 1));
}

static inline uint32_t mod(int32_t x, int32_t y)
{
   int32_t m = x % y;
   return (m < 0) ? (m + y) : m;
}

extern int khrn_get_type_size(int type /* GLenum*/);

static inline int find_max(int count, int size, const void *indices)
{
   int i;
   int32_t max = -1;

   switch (size) {
   case 1:
   {
      uint8_t *u = (uint8_t *)indices;

      for (i = 0; i < count; i++)
         max = _max( max, (int32_t) u[i]);

      break;
   }
   case 2:
   {
      uint16_t *u = (uint16_t *)indices;

      for (i = 0; i < count; i++)
         max = _max( max, (int32_t) u[i]);

      break;
   }
   default:
      UNREACHABLE();
      break;
   }

   return (int) max;
}

/******************************************************************************
for poking around inside floats (we assume ieee-754)
******************************************************************************/

typedef union {
   float f;
   uint32_t bits;
} KHRN_FLOAT_BITS_T;

static inline uint32_t float_to_bits(float f)
{
   KHRN_FLOAT_BITS_T t;
   t.f = f;
   return t.bits;
}

static inline float float_from_bits(uint32_t bits)
{
   KHRN_FLOAT_BITS_T t;
   t.bits = bits;
   return t.f;
}

/******************************************************************************
input cleaning stuff
******************************************************************************/

#include "interface/khronos/common/khrn_int_util_cr.h"

static inline void clean_floats(float *dst, const float *src, uint32_t count)
{
   uint32_t i;
   for (i = 0; i != count; ++i) {
      dst[i] = clean_float(src[i]);
   }
}

/******************************************************************************
float to int conversions
******************************************************************************/

static inline float r2ni_to_r2n_bias(float f, int32_t shift)
{
   assert((shift >= -129) && (shift <= 124));
   return f + float_from_bits(((127 - (shift + 2)) << 23) | 0x7fffff);
}

/*
   convert float to integer value with shift
   saturating, round to nearest

   on videocore, we support shifts in [-32, 31]. we only need to support shifts
   of 0 and 16 for client-side code
*/

static inline int32_t float_to_int_shift(float f, int32_t shift)
{
   assert((shift >= 0) && (shift <= 31));
   f *= (float)(uint32_t)(1 << shift);
   f += (f < 0.0f) ? -0.49999997f : 0.49999997f; /* assume float -> int conversion is round to zero */
   if (f < -2.14748365e9f) { return 0x80000000; }
   if (f > 2.14748352e9f)  { return 0x7fffffff; }
   return (int32_t)f;
}

/*
   convert float to integer value
   saturating, round to nearest
*/

static inline int32_t float_to_int(float f)
{
   return float_to_int_shift(f, 0);
}

/*
   convert float to integer value
   saturating, round to negative inf
*/

static inline int32_t float_to_int_floor(float f)
{
   /*
      special-case handling of small negative floats
      this is so we return -1 for negative denormals (which the vg cts requires)
      (we shouldn't need this if the fp library/hw properly handle denormals)
   */

   uint32_t u = float_to_bits(f);
   if (((u & (1 << 31)) && (u + u)) && (f > -1.0f)) {
      return -1;
   }

   f = floorf(f); /* assume float -> int conversion is round to zero */
   if (f < -2.14748365e9f) { return 0x80000000; }
   if (f > 2.14748352e9f) { return 0x7fffffff; }
   return (int32_t)f;
}

/*
   convert float to integer value
   saturating, round to zero
*/

static inline int32_t float_to_int_zero(float f)
{
   /* assume float -> int conversion is round to zero */
   if (f < -2.14748365e9f) { return 0x80000000; }
   if (f > 2.14748352e9f) { return 0x7fffffff; }
   return (int32_t)f;
}

/*
   convert float to 16.16 fixed point value
   saturating, round to nearest

   Khronos documentation:

   If a value is so large in magnitude that it cannot be represented with the
   requested type, then the nearest value representable using the requested type
   is returned.
*/

static inline int32_t float_to_fixed(float f)
{
   return float_to_int_shift(f, 16);
}

/******************************************************************************
exact float tests (in case fp library/hw don't handle denormals correctly)
******************************************************************************/

static inline bool floats_identical(float x, float y)
{
   return float_to_bits(x) == float_to_bits(y);
}

static inline bool is_zero(float f)
{
   uint32_t u = float_to_bits(f);
   return !(u + u);
}

static inline bool is_le_zero(float f)
{
   uint32_t u = float_to_bits(f);
   return (u & (1 << 31)) || !u;
}

/******************************************************************************
alignment stuff
******************************************************************************/

#ifdef _MSC_VER
   #define alignof(T) __alignof(T)
#elif defined(__CC_ARM)
   #define alignof(T) __alignof__(T)
#else
   #define alignof(T) (sizeof(struct { T t; char ch; }) - sizeof(T))
#endif

/*
   must use both ALIGNED and ALIGN_TO...
   ALIGNED(16) int align_me[10];
   ALIGN_TO(align_me, 16);
*/

#ifdef _MSC_VER
   #define ALIGNED(ALIGNMENT) __declspec(align(ALIGNMENT))
   #define ALIGN_TO(X, ALIGNMENT)
#elif defined(__GNUC__)
   #define ALIGNED(ALIGNMENT) __attribute__ ((aligned(ALIGNMENT)))
   #define ALIGN_TO(X, ALIGNMENT)
#elif defined(__HIGHC__)
   #define ALIGNED(ALIGMENT)
   #define ALIGN_TO(X, ALIGNMENT) pragma Align_to(ALIGNMENT, X)
#else
   /* leave undefined (will get error on use) */
#endif

/******************************************************************************
range/rect intersect stuff
******************************************************************************/

extern void khrn_clip_range(
   int32_t *x0, int32_t *l0,
   int32_t x1, int32_t l1);

extern void khrn_clip_range2(
   int32_t *ax0, int32_t *bx0, int32_t *l0,
   int32_t ax1, int32_t al1,
   int32_t bx1, int32_t bl1);

extern void khrn_clip_rect(
   int32_t *x0, int32_t *y0, int32_t *w0, int32_t *h0,
   int32_t x1, int32_t y1, int32_t w1, int32_t h1);

extern void khrn_clip_rect2(
   int32_t *ax0, int32_t *ay0, int32_t *bx0, int32_t *by0, int32_t *w0, int32_t *h0,
   int32_t ax1, int32_t ay1, int32_t aw1, int32_t ah1,
   int32_t bx1, int32_t by1, int32_t bw1, int32_t bh1);

static inline bool khrn_ranges_intersect(
   int32_t x0, int32_t l0,
   int32_t x1, int32_t l1)
{
   return (x0 < (x1 + l1)) && (x1 < (x0 + l0));
}

static inline bool khrn_rects_intersect(
   int32_t x0, int32_t y0, int32_t w0, int32_t h0,
   int32_t x1, int32_t y1, int32_t w1, int32_t h1)
{
   return khrn_ranges_intersect(x0, w0, x1, w1) && khrn_ranges_intersect(y0, h0, y1, h1);
}

/******************************************************************************
memory barrier
******************************************************************************/

static inline void khrn_barrier(void) {
#ifdef __GNUC__
   __sync_synchronize();
#endif
}
