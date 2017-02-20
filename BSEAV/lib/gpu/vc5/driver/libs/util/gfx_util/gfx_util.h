/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <assert.h>
#ifdef __cplusplus
#include <cmath>
#endif
#include "libs/util/common.h"

EXTERN_C_BEGIN

/** misc */

/* overflow-proof (x + y) <= z */
static inline bool gfx_usum_le(uint32_t x, uint32_t y, uint32_t z)
{
   uint32_t sum = x + y;
   return (sum >= x) && (sum <= z);
}

static inline uint32_t gfx_mask(uint32_t num_bits)
{
   assert(num_bits <= 32);
   return (num_bits == 0) ? 0 : ((uint32_t)-1 >> (32 - num_bits));
}

static inline uint64_t gfx_mask64(uint32_t num_bits)
{
   assert(num_bits <= 64);
   return (num_bits == 0) ? 0 : ((uint64_t)-1 >> (64 - num_bits));
}

static inline bool gfx_umul_overflow(uint32_t x, uint32_t y)
{
   uint64_t r = (uint64_t)x * (uint64_t)y;
   return !!(r >> 32);
}

static inline uint32_t gfx_umul_no_overflow(uint32_t x, uint32_t y)
{
   assert(!gfx_umul_overflow(x, y));
   return x * y;
}

/* lsr = logical shift right (ie shift in 0s)
 * asr = arithmetic shift right (ie replicate sign bit)
 *
 * "exact" means we assert that we don't discard bits when doing the shift */

static inline uint32_t gfx_exact_lsr(uint32_t x, uint32_t y)
{
   assert(((x >> y) << y) == x);
   return x >> y;
}

static inline int32_t gfx_exact_asr(int32_t x, uint32_t y)
{
   assert(((x >> y) << y) == x);
   return x >> y;
}

/* Rotate bits right */
static inline uint32_t gfx_ror(uint32_t x, uint32_t y, uint32_t num_bits)
{
   assert(!(x & ~gfx_mask(num_bits)));
   assert(y < num_bits);
   return (x >> y) | ((x << (num_bits - y)) & gfx_mask(num_bits));
}

/* Rotate bits left */
static inline uint32_t gfx_rol(uint32_t x, uint32_t y, uint32_t num_bits)
{
   return gfx_ror(x, (y == 0) ? 0 : (num_bits - y), num_bits);
}

static inline int32_t gfx_sext(uint32_t x, uint32_t num_bits)
{
   assert(num_bits > 0);
   assert(num_bits <= 32);
   return (int32_t)(x << (32 - num_bits)) >> (32 - num_bits);
}

/* Return the greatest common divisor of a and b */
static inline uint32_t gfx_gcd(uint32_t a, uint32_t b)
{
   /* See http://en.wikipedia.org/wiki/Euclidean_algorithm */
   while (b)
   {
      uint32_t t = b;
      b = a % b;
      a = t;
   }
   return a;
}

/* Return the least common multiple of a and b */
static inline uint32_t gfx_lcm(uint32_t a, uint32_t b)
{
   uint32_t gcd = gfx_gcd(a, b);
   assert(gcd > 0);
   uint64_t lcm = ((uint64_t)a * b) / gcd;
   assert(lcm <= UINT32_MAX);
   return (uint32_t)lcm;
}

/* eg GFX_STRINGIFY(V3D_MAX_NUM_CORES) --> "4" */
#define GFX_STRINGIFY_INTERNAL(X) #X
#define GFX_STRINGIFY(X) GFX_STRINGIFY_INTERNAL(X)

/** min/max/clamp */

/* Macros for use in expressions which need to be constant (eg array
 * dimensions). Please don't use these elsewhere... */
#define GFX_MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define GFX_MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define GFX_MAX3(X, Y, Z) (GFX_MAX(GFX_MAX(X, Y), Z))

static inline int32_t gfx_smin(int32_t x, int32_t y)
{
   return (x < y) ? x : y;
}

static inline int32_t gfx_smin3(int32_t x, int32_t y, int32_t z)
{
   return gfx_smin(gfx_smin(x, y), z);
}

static inline int64_t gfx_smin64(int64_t x, int64_t y)
{
   return (x < y) ? x : y;
}

static inline int32_t gfx_smax(int32_t x, int32_t y)
{
   return (x > y) ? x : y;
}

static inline int32_t gfx_smax3(int32_t x, int32_t y, int32_t z)
{
   return gfx_smax(gfx_smax(x, y), z);
}

static inline int64_t gfx_smax64(int64_t x, int64_t y)
{
   return (x > y) ? x : y;
}

static inline int32_t gfx_sclamp(int32_t x, int32_t min, int32_t max)
{
   assert(min <= max);
   return gfx_smin(gfx_smax(x, min), max);
}

static inline int64_t gfx_sclamp64(int64_t x, int64_t min, int64_t max)
{
   assert(min <= max);
   return gfx_smin64(gfx_smax64(x, min), max);
}

static inline int32_t gfx_sclamp_to_bits(int32_t x, uint32_t num_bits)
{
   assert((num_bits > 0) && (num_bits <= 32));
   int32_t max = gfx_mask(num_bits - 1);
   int32_t min = -max - 1;
   return gfx_sclamp(x, min, max);
}

static inline int64_t gfx_sclamp64_to_bits(int64_t x, uint32_t num_bits)
{
   assert((num_bits > 0) && (num_bits <= 64));
   int64_t max = gfx_mask64(num_bits - 1);
   int64_t min = -max - 1;
   return gfx_sclamp64(x, min, max);
}

static inline uint32_t gfx_umin(uint32_t x, uint32_t y)
{
   return (x < y) ? x : y;
}

static inline uint32_t gfx_umin3(uint32_t x, uint32_t y, uint32_t z)
{
   return gfx_umin(gfx_umin(x, y), z);
}

static inline uint32_t gfx_umax(uint32_t x, uint32_t y)
{
   return (x > y) ? x : y;
}

static inline uint32_t gfx_umax3(uint32_t x, uint32_t y, uint32_t z)
{
   return gfx_umax(gfx_umax(x, y), z);
}

static inline uint32_t gfx_uclamp(uint32_t x, uint32_t min, uint32_t max)
{
   assert(min <= max);
   return gfx_umin(gfx_umax(x, min), max);
}

static inline uint32_t gfx_uclamp_to_bits(uint32_t x, uint32_t num_bits)
{
   return gfx_uclamp(x, 0, gfx_mask(num_bits));
}

static inline uint32_t gfx_uabs_diff(uint32_t x, uint32_t y)
{
   return (gfx_umax(x, y) - gfx_umin(x, y));
}

static inline size_t gfx_zmin(size_t x, size_t y)
{
   return (x < y) ? x : y;
}

static inline size_t gfx_zmax(size_t x, size_t y)
{
   return (x > y) ? x : y;
}

static inline float gfx_fclamp(float x, float min, float max)
{
   assert(min <= max);
   return fminf(fmaxf(x, min), max);
}

static inline int32_t gfx_check_srange(int32_t x, int32_t min, int32_t max)
{
   assert(x >= min);
   assert(x <= max);
   return x;
}

static inline uint32_t gfx_check_urange(uint32_t x, uint32_t min, uint32_t max)
{
   assert(x >= min);
   assert(x <= max);
   return x;
}

static inline uint64_t gfx_check_urange64(uint64_t x, uint64_t min, uint64_t max)
{
   assert(x >= min);
   assert(x <= max);
   return x;
}

static inline float gfx_check_frange(float x, float min, float max)
{
   assert(x >= min);
   assert(x <= max);
   return x;
}

/** Safe shifts, for any value of rhs */

static inline int32_t gfx_srshift(int32_t x, uint32_t y)
{
   /* x >> y is implementation-defined if x < 0. We assume sign-extension. */
   return y >= (sizeof(int32_t) * 8) ? (x < 0 ? -1 : 0) : x >> y;
}

static inline int64_t gfx_srshift64(int64_t x, uint32_t y)
{
   return y >= (sizeof(int64_t) * 8) ? (x < 0 ? -1 : 0) : x >> y;
}

static inline bool gfx_lshift64_would_overflow(int64_t x, uint32_t y)
{
   if (x == 0)
      return false;
   if (y > 63)
      return true;
   return (((x < 0) ? ~x : x) & ~gfx_mask64(63 - y)) != 0;
}

/* x << y. If the result would overflow, it is clamped. */
static inline int64_t gfx_slshift64(int64_t x, uint32_t y)
{
   if (x == 0)
      return 0;

   if (gfx_lshift64_would_overflow(x, y))
      return (x < 0) ? INT64_MIN : INT64_MAX;

   assert(y <= 63);
   return x << y;
}

static inline int64_t gfx_slshift64s(int64_t x, int32_t y)
{
   return (y < 0) ? gfx_srshift64(x, -y) : gfx_slshift64(x, y);
}

static inline int32_t gfx_srshift_rtne(int32_t x, uint32_t y)
{
   if (y >= 32)
      return 0; /* At most a half, which rounds to 0 */
   if (y == 0)
      return x;

   uint32_t frac = x & gfx_mask(y);
   x >>= y;

   if (x & 0x1)
      frac += 1;
   if (frac > (1u << (y-1)))
      x += 1;

   return x;
}

static inline int64_t gfx_srshift64_rtne(int64_t x, uint32_t y)
{
   if (y >= 64)
      return 0; /* At most a half, which rounds to 0 */
   if (y == 0)
      return x;

   uint64_t frac = x & gfx_mask64(y);
   x >>= y;

   if (x & 0x1)
      frac += 1;
   if (frac > (1ull << (y-1)))
      x += 1;

   return x;
}

static inline int64_t gfx_slshift64s_rtne(int64_t x, int32_t y)
{
   return (y < 0) ? gfx_srshift64_rtne(x, -y) : gfx_slshift64(x, y);
}

/** Bit twiddling stuff */

static inline bool gfx_is_power_of_2(uint32_t x)
{
   return (x != 0) && ((x & (x - 1)) == 0);
}

static inline bool gfx_u64_is_power_of_2(uint64_t x)
{
   return (x != 0) && ((x & (x - 1)) == 0);
}

static inline bool gfx_size_is_power_of_2(size_t x)
{
   return (x != 0) && ((x & (x - 1)) == 0);
}

static inline uint64_t gfx_lowest_bit64(uint64_t x)
{
   return x & ~(x - 1);
}

static inline uint32_t gfx_lowest_bit(uint32_t x)
{
   return x & ~(x - 1);
}

#if defined(__GNUC__)

static inline unsigned gfx_msb64(uint64_t x)
{
   assert(x != 0);
   return 63u - __builtin_clzll(x);
}

static inline unsigned gfx_msb(uint32_t x)
{
   assert(x != 0);
   return 31u - __builtin_clz(x);
}

#else

static inline unsigned gfx_msb64(uint64_t x)
{
   assert(x != 0);
   unsigned msb = ~0u;
   do
   {
      ++msb;
      x >>= 1;
   }
   while (x != 0);
   return msb;
}

static inline unsigned gfx_msb(uint32_t x)
{
   return gfx_msb64(x);
}

#endif

static inline unsigned gfx_lsb64(uint64_t x)
{
   return gfx_msb64(gfx_lowest_bit64(x));
}

static inline unsigned gfx_lsb(uint32_t x)
{
   return gfx_msb(gfx_lowest_bit(x));
}

static inline uint32_t gfx_log2(uint32_t x)
{
   assert(gfx_is_power_of_2(x));
   return gfx_msb(x);
}

static inline uint32_t gfx_next_power_of_2(uint32_t x)
{
   assert(x <= (1u << 31)); /* Or the next power of 2 is 2^32, which we can't represent */
   if (gfx_is_power_of_2(x))
      return x;
   if (x)
      return 1u << (gfx_msb(x) + 1);
   return 1;
}

static inline uint32_t gfx_num_set_bits(uint32_t x)
{
   uint32_t num = 0;
   while (x) {
      num += x & 1;
      x >>= 1;
   }
   return num;
}

/** integer round-to-multiple */

static inline uint32_t gfx_uround_down(uint32_t x, uint32_t y)
{
   return (x / y) * y;
}

static inline uint32_t gfx_uround_down_p2(uint32_t x, uint32_t y)
{
   assert(gfx_is_power_of_2(y));
   return x & ~(y - 1);
}

static inline uint32_t gfx_uround_up_p2(uint32_t x, uint32_t y)
{
   assert(gfx_is_power_of_2(y));
   return (x + (y - 1)) & ~(y - 1);
}

static inline uint32_t gfx_udiv_round_up(uint32_t x, uint32_t y)
{
   if (x == 0)
      return 0;
   return ((x - 1) / y) + 1;
}

static inline size_t gfx_zdiv_round_up(size_t x, size_t y)
{
   if (x == 0)
      return 0;
   return ((x - 1) / y) + 1;
}

static inline uint32_t gfx_udiv_exactly(uint32_t x, uint32_t y)
{
   assert(x % y == 0);
   return x / y;
}

static inline uint32_t gfx_uround_up(uint32_t x, uint32_t y)
{
   return gfx_udiv_round_up(x, y) * y;
}

static inline size_t gfx_zround_down(size_t x, size_t y)
{
   return (x / y) * y;
}

static inline size_t gfx_zround_up(size_t x, size_t y)
{
   return gfx_zdiv_round_up(x, y) * y;
}

static inline size_t gfx_zround_down_p2(size_t x, size_t y)
{
   assert(gfx_size_is_power_of_2(y));
   return x & ~(y - 1);
}

static inline size_t gfx_zround_up_p2(size_t x, size_t y)
{
   assert(gfx_size_is_power_of_2(y));
   return (x + (y - 1)) & ~(y - 1);
}

static inline uint64_t gfx_u64round_down(uint64_t x, uint64_t y)
{
   return (x / y) * y;
}

static inline uint64_t gfx_u64round_down_p2(uint64_t x, uint64_t y)
{
   assert(gfx_u64_is_power_of_2(y));
   return x & ~(y - 1);
}

static inline uint64_t gfx_u64round_up_p2(uint64_t x, uint64_t y)
{
   assert(gfx_u64_is_power_of_2(y));
   return (x + (y - 1)) & ~(y - 1);
}

static inline uint64_t gfx_u64div_round_up(uint64_t x, uint64_t y)
{
   if (x == 0)
      return 0;
   return ((x - 1) / y) + 1;
}

static inline uint64_t gfx_u64div_exactly(uint64_t x, uint64_t y)
{
   assert(x % y == 0);
   return x / y;
}

static inline uint64_t gfx_u64round_up(uint64_t x, uint64_t y)
{
   return gfx_u64div_round_up(x, y) * y;
}

static inline int32_t gfx_sdiv_rtni(int32_t x, int32_t y)
{
   assert(y > 0);
   if (x >= 0)
      return x / y;
   return ((x + 1) / y) - 1;
}

/** alignment stuff */

static inline void *gfx_align_down(void *p, uint32_t align)
{
   assert(gfx_is_power_of_2(align));
   return (void *)((uintptr_t)p & ~(uintptr_t)(align - 1));
}

static inline void *gfx_align_up(void *p, uint32_t align)
{
   assert(gfx_is_power_of_2(align));
   return (void *)(((uintptr_t)p + (align - 1)) & ~(uintptr_t)(align - 1));
}

static inline bool gfx_aligned(void *p, uint32_t align)
{
   assert(gfx_is_power_of_2(align));
   return !((uintptr_t)p & (align - 1));
}

/** Bit-packing stuff */

static inline uint32_t gfx_bits(uint32_t x, uint32_t num_bits)
{
   assert(!(x & ~gfx_mask(num_bits)));
   return x;
}

static inline uint64_t gfx_bits64(uint64_t x, uint32_t num_bits)
{
   assert(!(x & ~gfx_mask64(num_bits)));
   return x;
}

static inline uint32_t gfx_pack_uint_0_is_max(
   uint32_t x, uint32_t num_bits)
{
   assert(x != 0);
   assert(num_bits > 0);
   assert(num_bits < 32);
   return (x == (1u << num_bits)) ? 0 : gfx_bits(x, num_bits);
}

static inline uint32_t gfx_unpack_uint_0_is_max(
   uint32_t x, uint32_t num_bits)
{
   assert(num_bits > 0);
   assert(num_bits < 32);
   return (x == 0) ? (1u << num_bits) : x;
}

static inline uint32_t gfx_pack_uint_minus_1(
   uint32_t x, uint32_t num_bits)
{
   assert(x != 0);
   assert(num_bits > 0);
   assert(num_bits < 32);
   return gfx_bits(x - 1, num_bits);
}

static inline uint32_t gfx_pack_sint(
   int32_t x, uint32_t num_bits)
{
   assert(num_bits > 0);
   assert(num_bits <= 32);
   assert(
      ((x >> (num_bits - 1)) == 0) ||
      ((x >> (num_bits - 1)) == -1));
   return x & gfx_mask(num_bits);
}

static inline uint32_t gfx_pack_sint_0_is_max(
   int32_t x, uint32_t num_bits)
{
   assert(x != 0);
   assert(num_bits > 0);
   assert(num_bits < 32);
   return (x == (1 << (num_bits - 1))) ? 0 : gfx_pack_sint(x, num_bits);
}

static inline int32_t gfx_unpack_sint_0_is_max(
   uint32_t x, uint32_t num_bits)
{
   assert(num_bits > 0);
   assert(num_bits < 32);
   return (x == 0) ? (1 << (num_bits - 1)) : gfx_sext(x, num_bits);
}

static inline uint32_t gfx_pack_8888(
   uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
   return (gfx_bits(a, 8) << 0) |
          (gfx_bits(b, 8) << 8) |
          (gfx_bits(c, 8) << 16) |
          (gfx_bits(d, 8) << 24);
}

static inline uint32_t gfx_pick_8(
   uint32_t x, uint32_t i)
{
   return (x >> (i * 8)) & 0xff;
}

static inline uint32_t gfx_pack_1616(
   uint32_t a, uint32_t b)
{
   return (gfx_bits(a, 16) << 0) |
          (gfx_bits(b, 16) << 16);
}

static inline uint64_t gfx_pack_16161616(
   uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
   return ((uint64_t)gfx_bits(a, 16) << 0) |
          ((uint64_t)gfx_bits(b, 16) << 16) |
          ((uint64_t)gfx_bits(c, 16) << 32) |
          ((uint64_t)gfx_bits(d, 16) << 48);
}

static inline uint32_t gfx_pick_16(
   uint64_t x, uint32_t i)
{
   return (uint32_t)((x >> (i * 16)) & 0xffff);
}

static inline uint32_t gfx_swap_1616(uint32_t x)
{
   return (x << 16) | (x >> 16);
}

/** misc float stuff */

typedef union {
   float f;
   uint32_t bits;
} GFX_FLOAT_BITS_T;

static inline uint32_t gfx_float_to_bits(float f)
{
   GFX_FLOAT_BITS_T t;
   t.f = f;
   return t.bits;
}

static inline float gfx_float_from_bits(uint32_t bits)
{
   GFX_FLOAT_BITS_T t;
   t.bits = bits;
   return t.f;
}

static inline bool gfx_sign_bit_set(float f)
{
   return (gfx_float_to_bits(f) >> 31) != 0;
}

/* -NaN to -inf, +NaN to +inf. This is useful for modelling bits of hardware
 * that don't give NaNs special treatment */
static inline float gfx_nan_to_inf(float f)
{
#ifdef __cplusplus
   if (!std::isnan(f))
      return f;
#else
   if (!isnan(f))
      return f;
#endif

   uint32_t bits = gfx_float_to_bits(f);
   return gfx_float_from_bits(bits & 0xff800000);
}

/* Also -0 --> +0 */
static inline float gfx_denormal_to_zero(float f)
{
#ifdef __cplusplus
   switch (std::fpclassify(f))
#else
   switch (fpclassify(f))
#endif
   {
   case FP_SUBNORMAL:
   case FP_ZERO:
      return 0.0f;
   default:
      return f;
   }
}

/* Positive NaNs come out larger than everything else, negative NaNs come out
 * smaller than everything else */
static inline uint32_t gfx_bitwise_orderable_float(float x)
{
   uint32_t bits = gfx_float_to_bits(x);
   if (bits & (1u << 31))
      return -1 - bits;
   return (1u << 31) + bits;
}

static inline bool gfx_float_lt_bitwise(float a, float b)
{
   return gfx_bitwise_orderable_float(a) < gfx_bitwise_orderable_float(b);
}

static inline bool gfx_float_eq_bitwise(float a, float b)
{
   return gfx_float_to_bits(a) == gfx_float_to_bits(b);
}

/* Positive NaNs come out larger than everything else, negative NaNs come out
 * smaller than everything else */
static inline uint32_t gfx_bitwise_orderable_float16(uint32_t x)
{
   if (x & (1u << 15))
      return 0xffff - x;
   return (1u << 15) + x;
}

static inline bool gfx_float16_lt_bitwise(uint32_t a, uint32_t b)
{
   return gfx_bitwise_orderable_float16(a) < gfx_bitwise_orderable_float16(b);
}

EXTERN_C_END
