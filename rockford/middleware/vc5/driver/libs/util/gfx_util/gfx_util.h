/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GFX_UTIL_H
#define GFX_UTIL_H

#include "libs/util/assert_helpers.h"
#include <stdbool.h>
#include <stdint.h>
#include "vcos_types.h"
#include <math.h>

VCOS_EXTERN_C_BEGIN

/** misc */

/* overflow-proof (x + y) <= z */
static inline bool gfx_usum_le(uint32_t x, uint32_t y, uint32_t z)
{
   uint32_t sum = x + y;
   return (sum >= x) && (sum <= z);
}

static inline uint32_t gfx_mod(int32_t x, int32_t y)
{
   int32_t m = x % y;
   return (m < 0) ? (m + y) : m;
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

/* exit() is to avoid unused value warning */
#define GFX_CHECK_PTR_TYPES_MATCH(A, B) \
   (0 ? exit(&*(A) == &*(B)) : 0)

/* Just like container_of() in the Linux kernel */
#define GFX_CONTAINER_OF(PTR, TYPE, MEMBER) \
   (GFX_CHECK_PTR_TYPES_MATCH(&((TYPE *)0)->MEMBER, PTR), \
   ((TYPE *)((char *)(PTR) - offsetof(TYPE, MEMBER))))

/* GFX_CONTAINER_OF_TYPEEXPR() is just like GFX_CONTAINER_OF() except that
 * instead of taking a type T it takes an (lvalue) expression TYPE_EXPR of type
 * T. The expression is not evaluated; only its type matters.
 *
 * The point of this is to get around typeof not being available on MSVC. */
#ifdef __GNUC__
#define GFX_CONTAINER_OF_TYPEEXPR(PTR, TYPE_EXPR, MEMBER) \
   container_of(PTR, __typeof__(TYPE_EXPR), MEMBER)
#else
/* Take advantage of ternary operator's type unification... */
#define GFX_OFFSETOF_TYPEEXPR(TYPE_EXPR, MEMBER) \
   ((char *)&(0 ? &(TYPE_EXPR) : 0)->MEMBER - (char *)0)
#define GFX_CONTAINER_OF_TYPEEXPR(PTR, TYPE_EXPR, MEMBER) \
   (GFX_CHECK_PTR_TYPES_MATCH(&(TYPE_EXPR).MEMBER, PTR), \
   (0 ? &(TYPE_EXPR) : (void *)((char *)(PTR) - GFX_OFFSETOF_TYPEEXPR(TYPE_EXPR, MEMBER))))
#endif

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
   return gfx_smin(gfx_smax(x, min), max);
}

static inline int64_t gfx_sclamp64(int64_t x, int64_t min, int64_t max)
{
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

/* TODO NaN handling in gfx_fmin/gfx_fmax/gfx_fclamp/etc. At the moment, no
 * special handling. Might be nice to have eg gfx_fmin(NaN, a) == a,
 * gfx_fclamp(NaN, min, max) in [min, max], etc */

static inline float gfx_fmin(float x, float y)
{
   return (x < y) ? x : y;
}

static inline float gfx_fmin3(float x, float y, float z)
{
   return gfx_fmin(gfx_fmin(x, y), z);
}

static inline float gfx_fmax(float x, float y)
{
   return (x > y) ? x : y;
}

static inline float gfx_fmax3(float x, float y, float z)
{
   return gfx_fmax(gfx_fmax(x, y), z);
}

static inline float gfx_fclamp(float x, float min, float max)
{
   return gfx_fmin(gfx_fmax(x, min), max);
}

static inline int32_t gfx_check_srange(int32_t x, int32_t min, int32_t max)
{
   vcos_unused_in_release(min);
   vcos_unused_in_release(max);
   assert(x >= min);
   assert(x <= max);
   return x;
}

static inline uint32_t gfx_check_urange(uint32_t x, uint32_t min, uint32_t max)
{
   vcos_unused_in_release(min);
   vcos_unused_in_release(max);
   assert(x >= min);
   assert(x <= max);
   return x;
}

static inline uint64_t gfx_check_urange64(uint64_t x, uint64_t min, uint64_t max)
{
   vcos_unused_in_release(min);
   vcos_unused_in_release(max);
   assert(x >= min);
   assert(x <= max);
   return x;
}

static inline float gfx_check_frange(float x, float min, float max)
{
   vcos_unused_in_release(min);
   vcos_unused_in_release(max);
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

static inline uint32_t gfx_lowest_bit(uint32_t x)
{
   return x & ~(x - 1);
}

static inline int32_t gfx_msb64(uint64_t x)
{
   int32_t msb = -1;
   while (x != 0)
   {
      ++msb;
      x >>= 1;
   }
   return msb;
}

static inline int32_t gfx_msb(uint32_t x)
{
   return gfx_msb64(x);
}

static inline uint32_t gfx_log2(uint32_t x)
{
   assert(gfx_is_power_of_2(x));
   return (uint32_t)gfx_msb(x);
}

static inline uint32_t gfx_next_power_of_2(uint32_t x)
{
   assert(x <= (1u << 31)); /* Or the next power of 2 is 2^32, which we can't represent */
   return gfx_is_power_of_2(x) ? x : (1u << (gfx_msb(x) + 1));
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

static inline int32_t gfx_srshift_rtne(int32_t x, uint32_t y)
{
   assert(y < 32);

   if (y == 0)
   {
      return x;
   }

   uint32_t frac = x & gfx_mask(y);
   x >>= y;

   if (x & 0x1)
      frac += 1;
   if (frac > (1u << (y-1)))
      x += 1;

   return x;
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
   vcos_unused_in_release(num_bits);
   assert(!(x & ~gfx_mask(num_bits)));
   return x;
}

static inline uint64_t gfx_bits64(uint64_t x, uint32_t num_bits)
{
   vcos_unused_in_release(num_bits);
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

static inline uint16_t gfx_pack_88(
   uint32_t a, uint32_t b)
{
   return (uint16_t)((gfx_bits(a, 8) << 0) |
                     (gfx_bits(b, 8) << 8));
}

static inline uint32_t gfx_pack_8888(
   uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
   return (gfx_bits(a, 8) << 0) |
          (gfx_bits(b, 8) << 8) |
          (gfx_bits(c, 8) << 16) |
          (gfx_bits(d, 8) << 24);
}

static inline uint32_t gfx_pack_s8888(
   int32_t a, int32_t b, int32_t c, int32_t d)
{
   return (gfx_pack_sint(a, 8) << 0) |
          (gfx_pack_sint(b, 8) << 8) |
          (gfx_pack_sint(c, 8) << 16) |
          (gfx_pack_sint(d, 8) << 24);
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

static inline uint32_t gfx_pack_s1616(
   int32_t a, int32_t b)
{
   return (gfx_pack_sint(a, 16) << 0) |
          (gfx_pack_sint(b, 16) << 16);
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

#define GFX_INF_BITS 0x7f800000
#define GFX_INF (gfx_float_from_bits(GFX_INF_BITS))

#define GFX_NEGINF_BITS 0xff800000
#define GFX_NEGINF (gfx_float_from_bits(GFX_NEGINF_BITS))

static inline bool gfx_is_nan_bits(uint32_t bits)
{
   /* NaN is max exponent with non-zero mantissa */
   return (((bits >> 23) & 0xff) == 0xff) && ((bits & gfx_mask(23)) != 0);
}

static inline bool gfx_is_inf_bits(uint32_t bits)
{
   /* Infinity is max exponent with zero mantissa */
   return (((bits >> 23) & 0xff) == 0xff) && ((bits & gfx_mask(23)) == 0);
}

static inline bool gfx_is_denormal_or_zero_bits(uint32_t bits)
{
   return ((bits >> 23) & 0xff) == 0;
}

static inline bool gfx_is_nan(float f)
{
   return gfx_is_nan_bits(gfx_float_to_bits(f));
}

static inline bool gfx_is_inf(float f)
{
   return gfx_is_inf_bits(gfx_float_to_bits(f));
}

static inline bool gfx_is_denormal_or_zero(float f)
{
   return gfx_is_denormal_or_zero_bits(gfx_float_to_bits(f));
}

static inline bool gfx_sign_bit_set(float f)
{
   return (gfx_float_to_bits(f) >> 31) != 0;
}

/* -NaN to -inf, +NaN to +inf. This is useful for modelling bits of hardware
 * that don't give NaNs special treatment */
static inline float gfx_nan_to_inf(float f)
{
   uint32_t bits = gfx_float_to_bits(f);

   if (!gfx_is_nan_bits(bits))
      return f;

   return gfx_float_from_bits(bits & 0xff800000);
}

/* Also -0 --> +0 */
static inline float gfx_denormal_to_zero(float f)
{
   return gfx_is_denormal_or_zero(f) ? 0.0f : f;
}

/* This should match the behaviour of the QPU, except for NaNs. This function
 * preserves the low mantissa bits of NaN inputs (as long as they aren't all
 * zero) so that gfx_float_to_float16(gfx_float16_to_float(f16)) always returns
 * f16. The QPU on the other hand returns the same mantissa bits for all input
 * NaNs. Both this function and the QPU preserve the sign bit of NaNs. */
static inline uint32_t gfx_bits_to_float16(uint32_t in)
{
   uint32_t in_exp = (in >> 23) & 0xff;
   uint32_t in_mant = in & 0x7fffff;
   uint32_t out;

   if (in_exp >= 0x8f) // positive overflow or NaN
   {
      out = 0x7c00; // INF

      if (gfx_is_nan_bits(in))
      {
         /* Preserve low mantissa bits of NaN inputs, unless they're all zero
          * (as that would give infinity) */
         uint32_t m = in_mant & 0x3ff;
         out |= m ? m : 0x200;
      }
   }
   else if (in_exp <= 0x65)
   {
      out = 0x0; // result is zero
   }
   else
   {
      uint32_t out_exp;
      bool round_to_nearest_even = true;
      uint32_t unshifted_out_mant;

      if (in_exp <= 0x70)
      {
         uint32_t subn_shift = 0x71 - in_exp; // result is subnormal, need to shift mantissa

         out_exp = 0x0; // fp16 result is subnormal

         // when result is subnormal and any bit shifted out is 1 prevent
         // rounding to nearest even
         if (in_mant & gfx_mask(subn_shift))
            round_to_nearest_even = false;

         unshifted_out_mant = (0x800000 | in_mant) >> subn_shift;
      }
      else
      {
         out_exp = in_exp - 0x70;

         unshifted_out_mant = in_mant;
      }

      // NOTE: exp and mantissa are added (not ORed!), because rounding of
      // mantissa might overflow, which means, that exponent has to be
      // incremented
      out = (out_exp << 10) +
         ((unshifted_out_mant + 0x1000) >> 13); // add 0.5 and shift
      if (round_to_nearest_even && ((unshifted_out_mant & 0x1fff) == 0x1000))
         out = out & 0x7ffe; // that is the even part of round to nearest even, so clear the even bit
   }

   // fix sign bit
   if (in >> 31)
      out |= 0x8000;

   return out;
}

static inline uint32_t gfx_float_to_float16(float f)
{
   uint32_t in = gfx_float_to_bits(f);
   return gfx_bits_to_float16(in);
}

static inline uint64_t gfx_float4_to_float16161616(const float *f)
{
   return gfx_pack_16161616(
      gfx_float_to_float16(f[0]),
      gfx_float_to_float16(f[1]),
      gfx_float_to_float16(f[2]),
      gfx_float_to_float16(f[3]));
}

/* This should match the behaviour of the QPU, except for NaNs. This function
 * preserves the mantissa and sign bits of NaN inputs, whereas the QPU always
 * returns the same NaN for all input NaNs. We preserve the mantissa/sign bits
 * so that gfx_float_to_float16(gfx_float16_to_float(f16)) always returns f16.
 * Note that merely preserving NaN bits in both functions is not sufficient to
 * guarantee this: they have to be preserved in compatible ways! */
static inline float gfx_float16_to_float(uint32_t f16)
{
   uint32_t sgn, exp, mnt;
   assert(f16 <= 0xffff);

   sgn = (f16 >> 15) & 0x1;
   exp = (f16 >> 10) & 0x1f;
   mnt = f16 & 0x3ff;

   if (exp == 0)
   {
      int32_t i = gfx_msb(mnt);
      if (i != -1)       //denormal
      {
         exp = 127 - 14 - (10 - i);
         mnt <<= 23 - i;
         assert((mnt >> 23) == 1); // hidden bit
         mnt &= gfx_mask(23);
      }                  //else zero
   }
   else
   {
      if (exp == 0x1f)   //infinity or NaN
      {
         exp = 0xff;
         /* Always return quiet NaNs. Signaling NaNs can cause weird things to
          * happen */
         if (mnt)
            mnt |= 1u << 22; /* This bit indicates a quiet NaN */
      }
      else
      {
         exp += 112;     // 15 --> 127
         mnt <<= 13;
      }
   }

   assert(sgn <= 0x1);
   assert(exp <= 0xff);
   assert(mnt <= 0x7fffff);

   return gfx_float_from_bits(sgn << 31 | exp << 23 | mnt);
}

/* 1.5.10 to 0.5.6/0.5.5. Matches v3d TLB. See also GLES 3 spec "2.1.3 Unsigned
 * 11-Bit Floating-Point Numbers"
 */

static inline uint32_t gfx_float16_to_ufloat11(uint32_t f16)
{
   uint32_t in_sign = f16 >> 15;
   uint32_t in_exp_bits = (f16 >> 10) & 0x1f;
   uint32_t in_mant_bits = f16 & 0x3ff;

   assert(f16 <= 0xffff);

   if (in_exp_bits == 0x1f && in_mant_bits != 0)
      return 0x7ff; // nan

   if (in_sign)
      return 0; // -ve numbers and -ve infinity to 0.
   return (in_exp_bits << 6) | (in_mant_bits >> 4); // This works for +ve infinity too
}

static inline uint32_t gfx_ufloat11_to_float16(uint32_t uf11)
{
   uint32_t exp_bits = (uf11 >> 6) & 0x1f;
   uint32_t mant_bits = uf11 & 0x3f;

   assert(uf11 <= 0x7ff);

   if (exp_bits == 0x1f && mant_bits != 0)
      return 0x7fff; // nan

   return uf11 << 4;
}

static inline uint32_t gfx_float16_to_ufloat10(uint32_t f16)
{
   uint32_t in_sign = f16 >> 15;
   uint32_t in_exp_bits = (f16 >> 10) & 0x1f;
   uint32_t in_mant_bits = f16 & 0x3ff;

   assert(f16 <= 0xffff);

   if (in_exp_bits == 0x1f && in_mant_bits != 0)
      return 0x3ff; // nan

   if (in_sign)
      return 0; // -ve numbers and -ve infinity to 0.
   return (in_exp_bits << 5) | (in_mant_bits >> 5); // This works for +ve infinity too
}

static inline uint32_t gfx_ufloat10_to_float16(uint32_t uf10)
{
   uint32_t exp_bits = (uf10 >> 5) & 0x1f;
   uint32_t mant_bits = uf10 & 0x1f;

   assert(uf10 <= 0x3ff);

   if (exp_bits == 0x1f && mant_bits != 0)
      return 0x7fff; // nan

   return uf10 << 5;
}

static inline uint32_t gfx_float_to_ufloat11(float x)
{
   return gfx_float16_to_ufloat11(gfx_float_to_float16(x));
}

static inline float gfx_ufloat11_to_float(uint32_t uf11)
{
   return gfx_float16_to_float(gfx_ufloat11_to_float16(uf11));
}

static inline uint32_t gfx_float_to_ufloat10(float x)
{
   return gfx_float16_to_ufloat10(gfx_float_to_float16(x));
}

static inline float gfx_ufloat10_to_float(uint32_t uf10)
{
   return gfx_float16_to_float(gfx_ufloat10_to_float16(uf10));
}

/* Positive NaNs come out larger than everything else, negative NaNs come out
 * smaller than everything else */
static inline uint32_t gfx_bitwise_orderable_float(float x)
{
   uint32_t bits = gfx_float_to_bits(x);
   if (bits & (1u << 31)) {
      return -1 - bits;
   }
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
   if (x & (1u << 15)) {
      return 0xffff - x;
   }
   return (1u << 15) + x;
}

static inline bool gfx_float16_lt_bitwise(uint32_t a, uint32_t b)
{
   return gfx_bitwise_orderable_float16(a) < gfx_bitwise_orderable_float16(b);
}

/** float <-> int conversions */

/* TODO would be more consistent with the naming of other functions in this
 * file to have eg gfx_float_to_uint instead of gfx_float_to_uint32 */

/* round to negative infinity */
static inline uint32_t gfx_float_to_uint32_rtni(float f)
{
   if (gfx_is_nan(f) || (f < 0.0f)) {
      return 0;
   }
   if (f > gfx_float_from_bits(0x4f7fffff)) {
      return -1;
   }
   return (uint32_t)f;
}

/* round to positive infinity */
static inline uint32_t gfx_float_to_uint32_rtpi(float f)
{
   return gfx_float_to_uint32_rtni((float)ceil(f));
}

/* round to nearest. TODO round half to even? */
static inline uint32_t gfx_float_to_uint32(float f)
{
   return gfx_float_to_uint32_rtni(f + 0.5f);
}

/* round to zero */
static inline int32_t gfx_float_to_int32_rtz(float f)
{
   if (gfx_is_nan(f)) {
      return 0;
   }
   if (f < gfx_float_from_bits(0xcf000000)) {
      return 1u << 31;
   }
   if (f > gfx_float_from_bits(0x4effffff)) {
      return (1u << 31) - 1;
   }
   return (int32_t)f;
}

/* round to nearest. TODO round half to even? */
static inline int32_t gfx_float_to_int32(float f)
{
   return gfx_float_to_int32_rtz(f + ((f < 0.0f) ? -0.5f : 0.5f));
}

static inline float gfx_uint64_to_float_rtz(uint64_t u)
{
   int32_t exp = gfx_msb64(u);
   if (exp == -1)
   {
      return 0.0f;
   }

   uint32_t biased_exp = exp + 127;
   assert((biased_exp > 0) && (biased_exp < 255));

   int32_t shift = exp - 23;
   uint32_t mantissa;
   if (shift < 0)
      mantissa = (uint32_t)(u << -shift);
   else
      mantissa = (uint32_t)(u >> shift); /* Round to zero, ie just drop bits */

   /* Hidden bit */
   assert(mantissa & (1u << 23));
   mantissa &= ~(1u << 23);
   assert(mantissa < (1u << 23));

   uint32_t bits = (biased_exp << 23) | mantissa;
   return gfx_float_from_bits(bits);
}

static inline float gfx_int64_to_float_rtz(int64_t i)
{
   if (i == INT64_MIN)
      return (float)INT64_MIN; /* Can be represented exactly */

   float f = gfx_uint64_to_float_rtz((i < 0) ? -i : i);
   if (i < 0)
      f = -f;
   return f;
}

static inline float gfx_uint32_to_float_rtz(uint32_t u)
{
   return gfx_uint64_to_float_rtz(u);
}

static inline float gfx_int32_to_float_rtz(int32_t i)
{
   return gfx_int64_to_float_rtz(i);
}

/* ref: GLES 3 spec "Conversion from Floating-Point to Normalized Fixed-Point" */
static inline uint32_t gfx_float_to_unorm(float f, uint32_t num_bits)
{
   assert(num_bits > 0);
   uint32_t max = gfx_mask(num_bits);
   return gfx_umin(gfx_float_to_uint32(f * max), max);
}

/* ref: GLES 3 spec "Conversion from Normalized Fixed-Point to Floating-Point" */
static inline float gfx_unorm_to_float(uint32_t u, uint32_t num_bits)
{
   assert(num_bits > 0);
   assert(u < (1ull << num_bits));
   return gfx_fmin(u / (float)gfx_mask(num_bits), 1.0f); /* fmin just to be on the safe side */
}

static inline uint32_t gfx_unorm_to_float_generic(
   uint32_t u, uint32_t num_bits,
   uint32_t num_exp_bits, uint32_t num_mantissa_bits,
   bool round_to_nearest) /* Else round-to-zero */
{
   assert(num_bits > 0);
   assert(u <= gfx_mask(num_bits));

   uint32_t exp_bias = (1u << (num_exp_bits - 1)) - 1;
   int32_t min_exp = 1 - exp_bias; /* Exp 0 means 0/denormal */

   if (u == 0)
      return 0;
   if (u == gfx_mask(num_bits))
   {
      return exp_bias << num_mantissa_bits; /* One */
   }

   int32_t u_msb = gfx_msb(u);
   assert(u_msb >= 0);
   assert((uint32_t)u_msb < num_bits);

   /* Figure out exponent (we may adjust this up 1 later for rounding) */
   int32_t unclamped_exp = u_msb - num_bits;
   bool denormal = unclamped_exp < min_exp;
   uint32_t biased_exp = denormal ? 0 : (unclamped_exp + exp_bias); /* Exp field in float */
   assert(biased_exp < exp_bias); /* Float should be less than one */
   int32_t actual_exp = denormal ? min_exp : unclamped_exp;

   /* Build mantissa by repeating u. We have:
    * Value we want to represent = 0.uuuuuu...
    * Value represented by the float = mantissa * 2**-num_mantissa_bits * 2**actual_exp
    * So we want:
    * mantissa = 0.uuuuuu... * 2**(num_mantissa_bits - actual_exp)
    * Note that we actually include an extra bit at the end of the mantissa for
    * rounding (which we will use and then throw away later). Note also that we
    * will generate the hidden bit here; it will be stripped away later. */
   int32_t rep_bits = num_mantissa_bits - actual_exp + 1; /* 1 extra for rounding bit */
   assert(rep_bits > 0);
   assert((rep_bits + u_msb + 1) <= 64); /* Or shift below would overflow... */
   uint32_t mantissa = 0;
   while (rep_bits > 0)
   {
      uint64_t bits = ((uint64_t)u << rep_bits) >> num_bits;
      assert(bits <= UINT32_MAX);
      mantissa |= (uint32_t)bits;
      rep_bits -= num_bits;
   }

   /* Form float from mantissa & exponent. Note that we still have the extra
    * rounding bit at this point. */
   if (!denormal)
   {
      /* Clear hidden bit */
      assert(mantissa & (1u << (num_mantissa_bits + 1)));
      mantissa &= ~(1u << (num_mantissa_bits + 1));
   }
   assert(mantissa < (1u << (num_mantissa_bits + 1)));
   assert((num_mantissa_bits + 1 + num_exp_bits) <= 32);
   uint32_t f = (biased_exp << (num_mantissa_bits + 1)) | mantissa;

   /* Round, then discard extra rounding bit. Note that rounding may overflow
    * the mantissa and increment the exponent (which is correct, if subtle) */
   if (round_to_nearest)
      f += 1; /* Round */
   f >>= 1; /* Discard extra rounding bit */

   return f;
}

/* Like gfx_unorm_to_float(), but round towards zero (gfx_unorm_to_float() does
 * round-to-nearest) */
static inline float gfx_unorm_to_float_rtz(uint32_t u, uint32_t num_bits)
{
   float f = gfx_float_from_bits(gfx_unorm_to_float_generic(u, num_bits,
      /*num_exp_bits=*/8, /*num_mantissa_bits=*/23, /*round_to_nearest=*/false));
   assert(f >= 0.0f);
   assert(f <= 1.0f);
   return f;
}

/* Like gfx_float_to_float16(gfx_unorm_to_float(u, num_bits)), but with exact
 * rounding in all cases. Note that there is always a closest float16 to round
 * to -- there is never a case where we need to decide between two float16s
 * that are equally close to the input unorm. */
static inline uint32_t gfx_unorm_to_float16(uint32_t u, uint32_t num_bits)
{
   uint32_t f16 = gfx_unorm_to_float_generic(u, num_bits,
      /*num_exp_bits=*/5, /*num_mantissa_bits=*/10, /*round_to_nearest=*/true);
   assert(f16 < (1u << 15)); /* Sign bit should not be set */
   assert(gfx_float16_to_float(f16) <= 1.0f);
   return f16;
}

static inline uint32_t gfx_float_to_snorm(float f, uint32_t num_bits)
{
   assert(num_bits > 1);
   assert(num_bits <= 32);
   int32_t max = gfx_mask(num_bits - 1);
   int32_t min = -max;
   int32_t r_sext = gfx_sclamp(gfx_float_to_int32(f * max), min, max);
   return r_sext & gfx_mask(num_bits);
}

static inline float gfx_snorm_to_float(uint32_t s, uint32_t num_bits)
{
   assert(num_bits > 1);
   int32_t s_sext = gfx_sext(s, num_bits);
   return gfx_fclamp(s_sext / (float)gfx_mask(num_bits - 1), -1.0f, 1.0f);
}

static inline uint32_t gfx_snorm_to_float_generic(
   uint32_t s, uint32_t num_bits,
   uint32_t num_exp_bits, uint32_t num_mantissa_bits,
   bool round_to_nearest) /* Else round-to-zero */
{
   assert(num_bits > 1);
   assert(s <= gfx_mask(num_bits));

   uint32_t sign_bit = 1u << (num_exp_bits + num_mantissa_bits);

   if (s == (1u << (num_bits - 1)))
   {
      /* Minus one */
      uint32_t exp = (1u << (num_exp_bits - 1)) - 1;
      return sign_bit | (exp << num_mantissa_bits);
   }

   int32_t s_sext = gfx_sext(s, num_bits);
   uint32_t u = (s_sext < 0) ? -s_sext : s_sext;
   uint32_t f = gfx_unorm_to_float_generic(u, num_bits - 1,
      num_exp_bits, num_mantissa_bits, round_to_nearest);
   if (s_sext < 0)
      f |= sign_bit;
   return f;
}

/* Like gfx_snorm_to_float(), but round towards zero (gfx_snorm_to_float() does
 * round-to-nearest) */
static inline float gfx_snorm_to_float_rtz(uint32_t s, uint32_t num_bits)
{
   float f = gfx_float_from_bits(gfx_snorm_to_float_generic(s, num_bits,
      /*num_exp_bits=*/8, /*num_mantissa_bits=*/23, /*round_to_nearest=*/false));
   assert(f >= -1.0f);
   assert(f <= 1.0f);
   return f;
}

/* Like gfx_float_to_float16(gfx_snorm_to_float(s, num_bits)), but with exact
 * rounding in all cases. Note that there is always a closest float16 to round
 * to -- there is never a case where we need to decide between two float16s
 * that are equally close to the input snorm. */
static inline uint32_t gfx_snorm_to_float16(uint32_t s, uint32_t num_bits)
{
   uint32_t f16 = gfx_snorm_to_float_generic(s, num_bits,
      /*num_exp_bits=*/5, /*num_mantissa_bits=*/10, /*round_to_nearest=*/true);
   assert(f16 < (1u << 16));
   assert(gfx_float16_to_float(f16) >= -1.0f);
   assert(gfx_float16_to_float(f16) <= 1.0f);
   return f16;
}

/* This matches the behaviour of the TLB when converting 32-bit float depth to
 * 24-bit/16-bit unorm. Note that this is less accurate than gfx_float_to_unorm
 * in two ways:
 * - It scales by 2^width rather than (2^width)-1
 * - It truncates rather than rounding */
static inline uint32_t gfx_float_to_unorm_depth(float f, uint32_t num_bits)
{
   assert(num_bits > 0);
   uint32_t max = gfx_mask(num_bits);
   return gfx_umin(
      gfx_float_to_uint32_rtni(gfx_nan_to_inf(f) * (float)(1ull << num_bits)),
      max);
}

/* This matches the behaviour of the TLB/TMU when converting 24-bit/16-bit
 * unorm depth to 32-bit float (for 16-bit unorm color, the TMU does the more
 * expensive gfx_unorm_to_float conversion). This is less accurate than
 * gfx_unorm_to_float as it divides by 2^width rather than (2^width)-1. This is
 * the inverse of gfx_float_to_unorm_depth. */
static inline float gfx_unorm_to_float_depth(uint32_t u, uint32_t num_bits)
{
   assert(num_bits > 0);
   assert(u < (1ull << num_bits));

   if (u == gfx_mask(num_bits))
      return 1.0f; /* Handled specially so we get exactly 1 */

   float f = u / (float)(1ull << num_bits);
   assert(f <= 1.0f);

   return f;
}

static inline uint32_t gfx_float_to_unorm8(float f)
{
   return gfx_float_to_unorm(f, 8);
}

/* This matches the conversion in the QPU->TLB FIFO. */
static inline uint32_t gfx_float_to_unorm8_approx(float f)
{
   uint32_t fl = gfx_float_to_bits(f);
   int exp, mmt, rsh, ans, ans256;

   exp = (fl >> 23) & 0xff;
   mmt = (fl & 0x7fffff) | 0x800000; // bit 24 = hidden bit

   if ((fl >> 31) || (exp <= 117)) {
      return 0;
   }
   if (exp >= 127) {
      return 255;
   }

   // between 1/512 and 1

   rsh = (127 - exp) + 12;
   ans256 = mmt >> rsh;
   if (mmt << (32 - rsh)) {
      ans256 |= 1;
   }

   ans = ans256 >> 8;
   if (ans256 << 24) {
      ans |= 1;
   }

   // subtract 1/256th of ans from itself (so multiplying by 255 not 256)
   ans = ans256 - ans;

   // work out rounding
   return (ans + 4) >> 3;
}

static inline float gfx_unorm8_to_float(uint32_t u)
{
   return gfx_unorm_to_float(u, 8);
}

/* Convert unormN to unormM (N=num_bits, M=num_result_bits, N<=M) by
 * replicating bits like abc --> abcabcab */
static inline uint32_t gfx_unorm_replicate(uint32_t u_in, uint32_t num_bits, uint32_t num_result_bits)
{
   assert(num_bits > 0);
   assert(num_bits <= num_result_bits);
   assert(num_result_bits <= 32);
   assert(u_in <= gfx_mask(num_bits));

   uint64_t u = u_in;
   while (num_bits < num_result_bits)
   {
      u = (u << num_bits) | u;
      num_bits *= 2;
   }
   u >>= num_bits - num_result_bits;

   return (uint32_t)u;
}

/* Convert snormN to snormM (N=num_bits, M=num_result_bits, N<=M) by
 * replicating bits */
static inline uint32_t gfx_snorm_replicate(uint32_t s, uint32_t num_bits, uint32_t num_result_bits)
{
   assert(num_bits > 1);
   assert(num_bits <= num_result_bits);
   assert(num_result_bits <= 32);
   assert(s <= gfx_mask(num_bits));

   if (s == (1u << (num_bits - 1)))
   {
      return 1u << (num_result_bits - 1);
   }

   int32_t s_sext = gfx_sext(s, num_bits);
   uint32_t u = (s_sext < 0) ? -s_sext : s_sext;
   int32_t res = gfx_unorm_replicate(u, num_bits - 1, num_result_bits - 1);
   if (s_sext < 0)
      res = -res;
   return res & gfx_mask(num_result_bits);
}

static inline float gfx_unorm_to_float_via8(uint32_t u, uint32_t num_bits)
{
   return gfx_unorm8_to_float(gfx_unorm_replicate(u, num_bits, 8));
}

static inline uint32_t gfx_unorm_to_float16_via8(uint32_t u, uint32_t num_bits)
{
   return gfx_unorm_to_float16(gfx_unorm_replicate(u, num_bits, 8), 8);
}

static inline uint32_t gfx_float4_to_unorm8888(const float *f)
{
   return gfx_pack_8888(
      gfx_float_to_unorm8(f[0]),
      gfx_float_to_unorm8(f[1]),
      gfx_float_to_unorm8(f[2]),
      gfx_float_to_unorm8(f[3]));
}

static inline void gfx_unorm8888_to_float4(float *f, uint32_t u)
{
   uint32_t i;
   for (i = 0; i != 4; ++i) {
      f[i] = gfx_unorm8_to_float(gfx_pick_8(u, i));
   }
}

static inline void gfx_float4_to_unorm8_8_8_8(uint8_t *unorm, const float *f)
{
   uint32_t i;
   for (i = 0; i != 4; ++i) {
      unorm[i] = gfx_float_to_unorm8(f[i]);
   }
}

/* TODO this function is a bit misleadingly named */
/* floors. return INT32_MIN for <= 0 and denorms, INT32_MAX for infinity */
static inline int32_t gfx_flog2(float f)
{
   uint32_t bits = gfx_float_to_bits(f);
   uint32_t exp = bits >> 23;
   int32_t r;

   if (f <= 0.0f) { r = INT32_MIN; }
   else if (exp == 0) { r = INT32_MIN; }
   else if (exp == 0xff) { r = INT32_MAX; }
   else { r = (int32_t)exp - 127; }

   return r;
}

/** sRGB conversions */

/* Both conversions clamp to [0, 1], but preserve NaNs */

/* ES3 spec "4.1.8 sRGB Conversion" */
static inline float gfx_lin_to_srgb_float(float f)
{
   if (gfx_is_nan(f)) {
      return f;
   }
   if (f <= 0.0f) {
      return 0.0f;
   }
   if (f < 0.0031308f) {
      return 12.92f * f;
   }
   if (f < 1.0f) {
      return (1.055f * powf(f, 1.0f / 2.4f)) - 0.055f;
   }
   return 1.0f;
}

/* ES3 spec "3.8.15 sRGB Texture Color Conversion" */
static inline float gfx_srgb_to_lin_float(float f)
{
   if (gfx_is_nan(f)) {
      return f;
   }
   if (f <= 0.0f) {
      return 0.0f;
   }
   if (f <= 0.04045f) {
      return f / 12.92f;
   }
   if (f < 1.0f) {
      return powf((f + 0.055f) / 1.055f, 2.4f);
   }
   return 1.0f;
}

VCOS_EXTERN_C_END

#endif
