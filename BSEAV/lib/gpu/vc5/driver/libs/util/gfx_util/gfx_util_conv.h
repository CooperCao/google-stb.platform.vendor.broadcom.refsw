/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "gfx_util.h"

#include <stdint.h>
#include "vcos_types.h"

VCOS_EXTERN_C_BEGIN

/** float <-> float16 */

/* Round to nearest, half to even. Overflows to infinity. NaNs preserved.
 *
 * This should match the behaviour of the QPU, except for NaNs. This function
 * preserves the low mantissa bits of NaN inputs (as long as they aren't all
 * zero) so that gfx_float_to_float16(gfx_float16_to_float(f16)) always returns
 * f16. The QPU on the other hand returns the same mantissa bits for all input
 * NaNs. Both this function and the QPU preserve the sign bit of NaNs. */
extern uint32_t gfx_floatbits_to_float16(uint32_t in);
static inline uint32_t gfx_float_to_float16(float f) { return gfx_floatbits_to_float16(gfx_float_to_bits(f)); }

/* Exact.
 *
 * This should match the behaviour of the QPU, except for NaNs. This function
 * preserves the mantissa and sign bits of NaN inputs, whereas the QPU always
 * returns the same NaN for all input NaNs. We preserve the mantissa/sign bits
 * so that gfx_float_to_float16(gfx_float16_to_float(f16)) always returns f16.
 * Note that merely preserving NaN bits in both functions is not sufficient to
 * guarantee this: they have to be preserved in compatible ways! */
extern uint32_t gfx_float16_to_floatbits(uint32_t f16);
static inline float gfx_float16_to_float(uint32_t f16) { return gfx_float_from_bits(gfx_float16_to_floatbits(f16)); }

/** float16 <-> ufloat10/11 */

/* 1.5.10 to/from 0.5.6/0.5.5. Matches v3d TLB.
 * See also GLES 3 spec "2.1.3 Unsigned 11-Bit Floating-Point Numbers" */

/* Round towards zero. Negatives to 0. NaNs preserved but exact mantissa bits not. */
extern uint32_t gfx_float16_to_ufloat10_rtz(uint32_t f16);
extern uint32_t gfx_float16_to_ufloat11_rtz(uint32_t f16);

/* Round to nearest, half to even. Negatives to 0. NaNs preserved but exact mantissa bits not. */
extern uint32_t gfx_float16_to_ufloat10(uint32_t f16);
extern uint32_t gfx_float16_to_ufloat11(uint32_t f16);

/* NaNs preserved but exact mantissa bits not. Otherwise exact. */
extern uint32_t gfx_ufloat10_to_float16(uint32_t uf10);
extern uint32_t gfx_ufloat11_to_float16(uint32_t uf11);

/** ufloat10/11 -> float */

/* NaNs preserved but exact mantissa bits not. Otherwise exact. */
static inline float gfx_ufloat10_to_float(uint32_t uf10)
{
   return gfx_float16_to_float(gfx_ufloat10_to_float16(uf10));
}

/* NaNs preserved but exact mantissa bits not. Otherwise exact. */
static inline float gfx_ufloat11_to_float(uint32_t uf11)
{
   return gfx_float16_to_float(gfx_ufloat11_to_float16(uf11));
}

/** float <-> rgb9e5 */

/* See GLES 3 spec "Encoding of Special Internal Formats" */

/* Round to nearest, half away from 0. Saturating. NaNs to 0. */
extern uint32_t gfx_floats_to_rgb9e5(const float f[3]);

/* Exact */
extern void gfx_rgb9e5_to_floats(float f[3], uint32_t rgb9e5);

/** float -> [u]int */

/* Round towards negative infinity. Saturating. NaNs to 0. */
extern uint32_t gfx_float_to_uint32_rtni(float f);

/* Round towards positive infinity. Saturating. NaNs to 0. */
extern uint32_t gfx_float_to_uint32_rtpi(float f);

/* Round to nearest, half to even. Saturating. NaNs to 0. */
extern uint32_t gfx_double_to_uint32(double d);
extern uint32_t gfx_float_to_uint32(float f);
extern int32_t gfx_float_to_int32(float f);
extern int64_t gfx_float_to_int64(float f);

/** [u]int -> float */

/* Round towards zero */
extern float gfx_uint64_to_float_rtz(uint64_t u);

/* Round towards zero */
extern float gfx_int64_to_float_rtz(int64_t i);

/* Round towards zero */
static inline float gfx_uint32_to_float_rtz(uint32_t u)
{
   return gfx_uint64_to_float_rtz(u);
}

/* Round towards zero */
static inline float gfx_int32_to_float_rtz(int32_t i)
{
   return gfx_int64_to_float_rtz(i);
}

/** float -> u/snorm */

extern uint32_t gfx_float_to_small_unorm(float f, uint32_t num_bits);
extern uint32_t gfx_float_to_big_unorm(float f, uint32_t num_bits);

/* Round to nearest, half to even. Saturating. NaNs to 0. */
static inline uint32_t gfx_float_to_unorm(float f, uint32_t num_bits)
{
   return (num_bits <= 29) ?
      gfx_float_to_small_unorm(f, num_bits) :
      gfx_float_to_big_unorm(f, num_bits);
}

static inline uint32_t gfx_float_to_unorm8(float f)
{
   return gfx_float_to_unorm(f, 8);
}

/* This matches the conversion in the QPU->TLB FIFO */
extern uint32_t gfx_float_to_unorm8_approx(float f);

/* Round to nearest, half to even. Saturating. NaNs to 0. */
static inline uint32_t gfx_float_to_snorm(float f, uint32_t num_bits)
{
   assert(num_bits > 1);
   assert(num_bits <= 32);
   uint32_t s = gfx_float_to_unorm(fabsf(f), num_bits - 1);
   if (f < 0.0f)
      s = -(int32_t)s & gfx_mask(num_bits);
   return s;
}

/* This matches the behaviour of the TLB when converting 32-bit float depth to
 * 24-bit/16-bit unorm. Note that this is less accurate than gfx_float_to_unorm
 * in two ways:
 * - It scales by 2^width rather than (2^width)-1
 * - It truncates rather than rounding */
extern uint32_t gfx_float_to_unorm_depth(float f, uint32_t num_bits);

/** u/snorm -> float */

extern float gfx_small_unorm_to_float(uint32_t u, uint32_t num_bits);
extern float gfx_big_unorm_to_float(uint32_t u, uint32_t num_bits);

/* Round to nearest. Note there is never a half case as the exact answer always
 * has an infinitely repeating mantissa. */
static inline float gfx_unorm_to_float(uint32_t u, uint32_t num_bits)
{
   return (num_bits <= 24) ?
      gfx_small_unorm_to_float(u, num_bits) :
      gfx_big_unorm_to_float(u, num_bits);
}

static inline float gfx_unorm8_to_float(uint32_t u)
{
   return gfx_unorm_to_float(u, 8);
}

extern float gfx_small_snorm_to_float(uint32_t s, uint32_t num_bits);
extern float gfx_big_snorm_to_float(uint32_t s, uint32_t num_bits);

/* Round to nearest. Note there is never a half case as the exact answer always
 * has an infinitely repeating mantissa. */
static inline float gfx_snorm_to_float(uint32_t s, uint32_t num_bits)
{
   return (num_bits <= 25) ?
      gfx_small_snorm_to_float(s, num_bits) :
      gfx_big_snorm_to_float(s, num_bits);
}

/* Like gfx_unorm_to_float(), but round towards zero (gfx_unorm_to_float() does
 * round to nearest) */
extern float gfx_unorm_to_float_rtz(uint32_t u, uint32_t num_bits);

/* Like gfx_float_to_float16(gfx_unorm_to_float(u, num_bits)), but with exact
 * rounding in all cases. Note that there is always a closest float16 to round
 * to -- there is never a case where we need to decide between two float16s
 * that are equally close to the input unorm. */
extern uint32_t gfx_unorm_to_float16(uint32_t u, uint32_t num_bits);

/* Like gfx_snorm_to_float(), but round towards zero (gfx_snorm_to_float() does
 * round to nearest) */
extern float gfx_snorm_to_float_rtz(uint32_t s, uint32_t num_bits);

/* Like gfx_float_to_float16(gfx_snorm_to_float(s, num_bits)), but with exact
 * rounding in all cases. Note that there is always a closest float16 to round
 * to -- there is never a case where we need to decide between two float16s
 * that are equally close to the input snorm. */
extern uint32_t gfx_snorm_to_float16(uint32_t s, uint32_t num_bits);

/* This matches the behaviour of the TLB/TMU when converting 24-bit/16-bit
 * unorm depth to 32-bit float (for 16-bit unorm color, the TMU does the more
 * expensive gfx_unorm_to_float conversion). This is less accurate than
 * gfx_unorm_to_float as it divides by 2^width rather than (2^width)-1. This is
 * the inverse of gfx_float_to_unorm_depth. */
extern float gfx_unorm_to_float_depth(uint32_t u, uint32_t num_bits);

/** unorm -> unorm */

/* Convert unormN to unormM (N=num_bits, M=num_result_bits). Round to nearest.
 * Note there is never a half case. */
extern uint32_t gfx_unorm_to_unorm(uint32_t u, uint32_t num_bits, uint32_t num_result_bits);

/* Convert unormN to unormM (N=num_bits, M=num_result_bits) by
 * replicating/truncating, like:
 * abc --> abcabcab
 * abcdef --> abc
 * If (M % N) == 0 the result will be exactly correct. */
extern uint32_t gfx_unorm_to_unorm_rep(uint32_t u, uint32_t num_bits, uint32_t num_result_bits);

/** snorm -> snorm */

/* Convert snormN to snormM (N=num_bits, M=num_result_bits). Round to nearest.
 * Note there is never a half case. */
extern uint32_t gfx_snorm_to_snorm(uint32_t s, uint32_t num_bits, uint32_t num_result_bits);

/* Convert snormN to snormM (N=num_bits, M=num_result_bits, N<=M) by
 * replicating/truncating, like gfx_unorm_to_unorm_rep */
extern uint32_t gfx_snorm_to_snorm_rep(uint32_t s, uint32_t num_bits, uint32_t num_result_bits);

/** sRGB conversions */

/* Both conversions clamp to [0, 1], but preserve NaNs */

/* ES3 spec "4.1.8 sRGB Conversion" */
extern float gfx_lin_to_srgb_float(float f);

/* ES3 spec "3.8.15 sRGB Texture Color Conversion" */
extern float gfx_srgb_to_lin_float(float f);

VCOS_EXTERN_C_END
