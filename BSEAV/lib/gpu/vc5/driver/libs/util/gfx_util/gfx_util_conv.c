/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "gfx_util_conv.h"

#include <math.h>

/** float <-> float16 */

uint32_t gfx_floatbits_to_float16(uint32_t in)
{
   uint32_t in_exp = (in >> 23) & 0xff;
   uint32_t in_mant = in & 0x7fffff;
   uint32_t out;

   if (in_exp >= 0x8f) // positive overflow or NaN
   {
      out = 0x7c00; // INF

      if ((in_exp == 0xff) && in_mant) // NaN?
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

uint32_t gfx_float16_to_floatbits(uint32_t f16)
{
   uint32_t sgn, exp, mnt;
   assert(f16 <= 0xffff);

   sgn = (f16 >> 15) & 0x1;
   exp = (f16 >> 10) & 0x1f;
   mnt = f16 & 0x3ff;

   if (exp == 0)
   {
      if (mnt) //denormal
      {
         uint32_t i = gfx_msb(mnt);
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

   return sgn << 31 | exp << 23 | mnt;
}

/** float16 <-> ufloat10/11 */

static uint32_t float16_to_ufloat_generic(uint32_t f16, uint32_t num_bits, bool rtne)
{
   assert(f16 <= 0xffff);
   uint32_t sign = f16 >> 15;
   uint32_t exp_bits = (f16 >> 10) & 0x1f;
   uint32_t in_mant_bits = f16 & 0x3ff;

   if (exp_bits == 0x1f && in_mant_bits != 0)
      return gfx_mask(num_bits); // NaN

   if (sign)
      return 0; // Negatives to 0

   uint32_t num_mant_bits = num_bits - 5;

   uint32_t mant_bits;
   if (rtne)
   {
      mant_bits = gfx_srshift_rtne(in_mant_bits, 10 - num_mant_bits);
      if (mant_bits >= (1u << num_mant_bits))
      {
         assert(mant_bits == (1u << num_mant_bits));
         assert(exp_bits != 0x1f); /* mant_bits always 0 for infinity... */
         if (exp_bits == 0x1e)
            /* Don't round to infinity! */
            --mant_bits;
         else
         {
            /* Carry into exponent */
            ++exp_bits;
            mant_bits = 0;
         }
      }
   }
   else
      mant_bits = in_mant_bits >> (10 - num_mant_bits);

   return (exp_bits << num_mant_bits) | mant_bits;
}

uint32_t gfx_float16_to_ufloat10_rtz(uint32_t f16)
{
   return float16_to_ufloat_generic(f16, 10, /*rtne=*/false);
}

uint32_t gfx_float16_to_ufloat11_rtz(uint32_t f16)
{
   return float16_to_ufloat_generic(f16, 11, /*rtne=*/false);
}

uint32_t gfx_float16_to_ufloat10(uint32_t f16)
{
   return float16_to_ufloat_generic(f16, 10, /*rtne=*/true);
}

uint32_t gfx_float16_to_ufloat11(uint32_t f16)
{
   return float16_to_ufloat_generic(f16, 11, /*rtne=*/true);
}

uint32_t gfx_ufloat10_to_float16(uint32_t uf10)
{
   uint32_t exp_bits = (uf10 >> 5) & 0x1f;
   uint32_t mant_bits = uf10 & 0x1f;

   assert(uf10 <= 0x3ff);

   if (exp_bits == 0x1f && mant_bits != 0)
      return 0x7fff; // nan

   return uf10 << 5;
}

uint32_t gfx_ufloat11_to_float16(uint32_t uf11)
{
   uint32_t exp_bits = (uf11 >> 6) & 0x1f;
   uint32_t mant_bits = uf11 & 0x3f;

   assert(uf11 <= 0x7ff);

   if (exp_bits == 0x1f && mant_bits != 0)
      return 0x7fff; // nan

   return uf11 << 4;
}

/** float <-> rgb9e5 */

uint32_t gfx_floats_to_rgb9e5(const float f[3])
{
   const uint32_t N = 9, B = 15, E_max = 31;

   const float max_encodable = (float)gfx_mask(N) * (1u << (E_max - B - N));
   float clamped[3];
   for (int c=0; c!=3; ++c)
      clamped[c] = gfx_fclamp(f[c], 0.0f, max_encodable);

   float max_c = fmaxf(fmaxf(clamped[0], clamped[1]), clamped[2]);
   int32_t max_c_exp = (int32_t)(gfx_float_to_bits(max_c) >> 23) - 127;
   uint32_t shared_exp = gfx_smax(max_c_exp + 1 + B, 0); /* +1 because rgb9e5 has no hidden bit */
   float scale = (float)(1u << (B + N)) / (1u << shared_exp);

   uint32_t max_s = (uint32_t)((max_c * scale) + 0.5f);
   assert(max_s <= (1u << N));
   if (max_s == (1u << N))
   {
      /* Rounding caused overflow... */
      ++shared_exp;
      scale *= 0.5f;
   }

   uint32_t final_components[3];
   for (int c=0; c!=3; ++c)
   {
      final_components[c] = (uint32_t)((clamped[c] * scale) + 0.5f);
      assert(final_components[c] <= gfx_mask(N));
   }

   assert(shared_exp <= E_max);
   return
      final_components[0] |
      (final_components[1] << 9) |
      (final_components[2] << 18) |
      (shared_exp << 27);
}

void gfx_rgb9e5_to_floats(float f[3], uint32_t rgb9e5)
{
   const uint32_t N = 9, B = 15;

   uint32_t shared_exp = rgb9e5 >> 27;
   float scale = (float)(1u << shared_exp) / (1u << (B + N));

   for (int c=0; c!=3; ++c)
   {
      uint32_t component = (rgb9e5 >> (c*9)) & 0x1ff;
      f[c] = component * scale;
   }
}

/** float -> [u]int */

uint32_t gfx_float_to_uint32_rtni(float f)
{
   if (!(f > 0.0f)) /* <=0 or NaN? */
      return 0;
   if (f >= gfx_float_from_bits(0x4f800000))
      return UINT32_MAX;
   return (uint32_t)f;
}

uint32_t gfx_float_to_uint32_rtpi(float f)
{
   return gfx_float_to_uint32_rtni(ceilf(f));
}

uint32_t gfx_double_to_uint32(double d)
{
   if (!(d > 0.0)) /* <=0 or NaN? */
      return 0;
   if (d >= (double)UINT32_MAX)
      return UINT32_MAX;
   return (uint32_t)nearbyint(d);
}

uint32_t gfx_float_to_uint32(float f)
{
   return gfx_float_to_uint32_rtni(nearbyintf(f));
}

int32_t gfx_float_to_int32(float f)
{
   if (isnan(f))
      return 0;
   if (f <= gfx_float_from_bits(0xcf000000))
      return INT32_MIN;
   if (f >= gfx_float_from_bits(0x4f000000))
      return INT32_MAX;
   return (int32_t)nearbyintf(f);
}

int64_t gfx_float_to_int64(float f)
{
   if (isnan(f))
      return 0;
   if (f <= gfx_float_from_bits(0xdf000000))
      return INT64_MIN;
   if (f >= gfx_float_from_bits(0x5f000000))
      return INT64_MAX;
   return (int64_t)nearbyintf(f);
}

/** [u]int -> float */

float gfx_uint64_to_float_rtz(uint64_t u)
{
   if (!u)
      return 0.0f;

   uint32_t exp = gfx_msb64(u);
   uint32_t biased_exp = exp + 127;
   assert((biased_exp > 0) && (biased_exp < 255));

   int32_t shift = exp - 23;
   uint32_t mantissa;
   if (shift < 0)
      mantissa = (uint32_t)(u << -shift);
   else
      mantissa = (uint32_t)(u >> shift); /* Round towards zero, ie just drop bits */

   /* Hidden bit */
   assert(mantissa & (1u << 23));
   mantissa &= ~(1u << 23);
   assert(mantissa < (1u << 23));

   uint32_t bits = (biased_exp << 23) | mantissa;
   return gfx_float_from_bits(bits);
}

float gfx_int64_to_float_rtz(int64_t i)
{
   if (i == INT64_MIN)
      return (float)INT64_MIN; /* Can be represented exactly */

   float f = gfx_uint64_to_float_rtz((i < 0) ? -i : i);
   if (i < 0)
      f = -f;
   return f;
}

/** float -> u/snorm */

uint32_t gfx_float_to_small_unorm(float f, uint32_t num_bits)
{
   assert(num_bits > 0);
   assert(num_bits <= 29); /* Use gfx_float_to_big_unorm for big unorms! */
   uint32_t max = gfx_mask(num_bits);

   /* ((double)f * max) can be calculated without any rounding... */
   return gfx_umin(gfx_double_to_uint32((double)f * max), max);
}

uint32_t gfx_float_to_big_unorm(float f, uint32_t num_bits)
{
   assert(num_bits > 0);
   uint32_t max = gfx_mask(num_bits);

   if (!(f > 0.0f)) /* <=0 or NaN? */
      return 0;
   if (f >= 1.0f)
      return max;

   uint32_t bits = gfx_float_to_bits(f);
   /* Add back hidden bit. This isn't really correct for denormals but it
    * doesn't matter -- they'll all get converted to 0 anyway. */
   uint32_t mant = (bits & 0x7fffff) | 0x800000;
   int32_t exp = (int32_t)((bits >> 23) & 0xff) - 127;

   assert(exp < 0); /* Handled f>=1 case above */
   uint32_t res = (uint32_t)gfx_srshift64_rtne((int64_t)mant * (int64_t)max, 23 - exp);
   assert(res <= max);
   return res;
}

uint32_t gfx_float_to_unorm8_approx(float f)
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
   if (mmt & gfx_mask(rsh)) {
      ans256 |= 1;
   }

   ans = ans256 >> 8;
   if (ans256 & gfx_mask(8)) {
      ans |= 1;
   }

   // subtract 1/256th of ans from itself (so multiplying by 255 not 256)
   ans = ans256 - ans;

   // work out rounding
   return (ans + 4) >> 3;
}

uint32_t gfx_float_to_unorm_depth(float f, uint32_t num_bits)
{
   assert(num_bits > 0);
   uint32_t max = gfx_mask(num_bits);
   return gfx_umin(
      gfx_float_to_uint32_rtni(gfx_nan_to_inf(f) * (float)(1ull << num_bits)),
      max);
}

/** u/snorm -> float */

static uint32_t unorm_to_float_generic(
   uint32_t u, uint32_t num_bits,
   uint32_t num_exp_bits, uint32_t num_mantissa_bits,
   /* Else round towards zero. Note there is never a half case as the exact
    * answer has an infinitely repeating mantissa. */
   bool round_to_nearest)
{
   assert(num_bits > 0);
   assert(u <= gfx_mask(num_bits));

   uint32_t exp_bias = (1u << (num_exp_bits - 1)) - 1;
   int32_t min_exp = 1 - exp_bias; /* Exp 0 means 0/denormal */

   if (u == gfx_mask(num_bits))
      /* Even if round_to_nearest=false we want to return one in this case... */
      return exp_bias << num_mantissa_bits;

   if (!u)
      return 0;
   uint32_t u_msb = gfx_msb(u);
   assert(u_msb < num_bits);

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
   uint32_t want_rep_bits = num_mantissa_bits - actual_exp + 1; /* 1 extra for rounding bit */
   assert(want_rep_bits > 0);
   assert(want_rep_bits <= 64);
   uint64_t mantissa64 = (uint64_t)u << (64 - num_bits);
   uint32_t rep_bits = num_bits;
   while (rep_bits < want_rep_bits)
   {
      mantissa64 |= mantissa64 >> rep_bits;
      rep_bits *= 2;
   }
   mantissa64 >>= 64 - want_rep_bits;
   assert(mantissa64 <= UINT32_MAX);
   uint32_t mantissa = (uint32_t)mantissa64;

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

static uint32_t snorm_to_float_generic(
   uint32_t s, uint32_t num_bits,
   uint32_t num_exp_bits, uint32_t num_mantissa_bits,
   /* Else round towards zero. Note there is never a half case as the exact
    * answer has an infinitely repeating mantissa. */
   bool round_to_nearest)
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
   uint32_t f = unorm_to_float_generic(u, num_bits - 1,
      num_exp_bits, num_mantissa_bits, round_to_nearest);
   if (s_sext < 0)
      f |= sign_bit;
   return f;
}

float gfx_small_unorm_to_float(uint32_t u, uint32_t num_bits)
{
   assert(num_bits > 0);
   assert(num_bits <= 24); /* Use gfx_big_unorm_to_float for big unorms! */
   assert(u <= gfx_mask(num_bits));
   return u / (float)gfx_mask(num_bits);
}

float gfx_big_unorm_to_float(uint32_t u, uint32_t num_bits)
{
   float f = gfx_float_from_bits(unorm_to_float_generic(u, num_bits,
      /*num_exp_bits=*/8, /*num_mantissa_bits=*/23, /*round_to_nearest=*/true));
   assert(f >= 0.0f);
   assert(f <= 1.0f);
   return f;
}

float gfx_small_snorm_to_float(uint32_t s, uint32_t num_bits)
{
   assert(num_bits > 1);
   assert(num_bits <= 25); /* Use gfx_big_snorm_to_float for big snorms! */
   assert(s <= gfx_mask(num_bits));
   int32_t s_sext = gfx_sext(s, num_bits);
   return fmaxf(s_sext / (float)gfx_mask(num_bits - 1), -1.0f);
}

float gfx_big_snorm_to_float(uint32_t s, uint32_t num_bits)
{
   float f = gfx_float_from_bits(snorm_to_float_generic(s, num_bits,
      /*num_exp_bits=*/8, /*num_mantissa_bits=*/23, /*round_to_nearest=*/true));
   assert(f >= -1.0f);
   assert(f <= 1.0f);
   return f;
}

float gfx_unorm_to_float_rtz(uint32_t u, uint32_t num_bits)
{
   float f = gfx_float_from_bits(unorm_to_float_generic(u, num_bits,
      /*num_exp_bits=*/8, /*num_mantissa_bits=*/23, /*round_to_nearest=*/false));
   assert(f >= 0.0f);
   assert(f <= 1.0f);
   return f;
}

float gfx_unorm_to_float_gfxh1287(uint32_t u, uint32_t num_bits)
{
   if(num_bits == 32)
   {
      if (u >= 0xffffff00) u = 0xffffffff;
   }
   return gfx_unorm_to_float_rtz(u, num_bits);
}

uint32_t gfx_unorm_to_float16(uint32_t u, uint32_t num_bits)
{
   uint32_t f16 = unorm_to_float_generic(u, num_bits,
      /*num_exp_bits=*/5, /*num_mantissa_bits=*/10, /*round_to_nearest=*/true);
   assert(f16 < (1u << 15)); /* Sign bit should not be set */
   assert(gfx_float16_to_float(f16) <= 1.0f);
   return f16;
}

float gfx_snorm_to_float_rtz(uint32_t s, uint32_t num_bits)
{
   float f = gfx_float_from_bits(snorm_to_float_generic(s, num_bits,
      /*num_exp_bits=*/8, /*num_mantissa_bits=*/23, /*round_to_nearest=*/false));
   assert(f >= -1.0f);
   assert(f <= 1.0f);
   return f;
}

float gfx_snorm_to_float_gfxh1287(uint32_t s, uint32_t num_bits)
{
   if(num_bits == 32)
   {
      if (s & 0x80000000) {
         if ((s & 0x7fffffff) < 0xff) s = 0x80000000;
      } else {
         if (s >= 0x7fffff80) s = 0x7fffffff;
      }
   }
   return gfx_snorm_to_float_rtz(s, num_bits);
}

uint32_t gfx_snorm_to_float16(uint32_t s, uint32_t num_bits)
{
   uint32_t f16 = snorm_to_float_generic(s, num_bits,
      /*num_exp_bits=*/5, /*num_mantissa_bits=*/10, /*round_to_nearest=*/true);
   assert(f16 < (1u << 16));
   assert(gfx_float16_to_float(f16) >= -1.0f);
   assert(gfx_float16_to_float(f16) <= 1.0f);
   return f16;
}

float gfx_unorm_to_float_depth(uint32_t u, uint32_t num_bits)
{
   assert(num_bits > 0);
   assert(u <= gfx_mask(num_bits));

   if (u == gfx_mask(num_bits))
      return 1.0f; /* Handled specially so we get exactly 1 */

   float f = u / (float)(1ull << num_bits);
   assert(f <= 1.0f);

   return f;
}

/** unorm -> unorm */

uint32_t gfx_unorm_to_unorm(uint32_t u, uint32_t num_bits, uint32_t num_result_bits)
{
   assert(num_bits > 0);
   uint32_t in_max = gfx_mask(num_bits);
   assert(u <= in_max);

   assert(num_result_bits > 0);
   uint32_t result_max = gfx_mask(num_result_bits);

   uint64_t res = (uint64_t)u * result_max;
   res = (res + (in_max >> 1)) / in_max; /* Note in_max is always odd so there is never a half case */
   assert(res <= result_max);
   return (uint32_t)res;
}

uint32_t gfx_unorm_to_unorm_rep(uint32_t u, uint32_t num_bits, uint32_t num_result_bits)
{
   assert(num_bits > 0);
   assert(u <= gfx_mask(num_bits));
   assert(num_result_bits > 0);
   assert(num_result_bits <= 32);

   uint32_t res = u << (32 - num_bits);
   while (num_bits < num_result_bits)
   {
      res |= res >> num_bits;
      num_bits *= 2;
   }
   res >>= 32 - num_result_bits;

   return res;
}

/** snorm -> snorm */

static uint32_t snorm_to_snorm_from_unorm_to_unorm(
   uint32_t s, uint32_t num_bits, uint32_t num_result_bits,
   uint32_t (*unorm_to_unorm)(uint32_t u, uint32_t num_bits, uint32_t num_result_bits))
{
   assert(num_bits > 1);
   assert(s <= gfx_mask(num_bits));
   assert(num_result_bits > 1);
   assert(num_result_bits <= 32);

   if (s == (1u << (num_bits - 1)))
      return 1u << (num_result_bits - 1);

   int32_t s_sext = gfx_sext(s, num_bits);
   uint32_t u = (s_sext < 0) ? -s_sext : s_sext;
   uint32_t res = unorm_to_unorm(u, num_bits - 1, num_result_bits - 1);
   if (s_sext < 0)
      res = -(int32_t)res & gfx_mask(num_result_bits);
   return res;
}

uint32_t gfx_snorm_to_snorm(uint32_t s, uint32_t num_bits, uint32_t num_result_bits)
{
   return snorm_to_snorm_from_unorm_to_unorm(s, num_bits, num_result_bits, gfx_unorm_to_unorm);
}

uint32_t gfx_snorm_to_snorm_rep(uint32_t s, uint32_t num_bits, uint32_t num_result_bits)
{
   return snorm_to_snorm_from_unorm_to_unorm(s, num_bits, num_result_bits, gfx_unorm_to_unorm_rep);
}

/** sRGB conversions */

float gfx_lin_to_srgb_float(float f)
{
   if (isnan(f))
      return f;
   if (f <= 0.0f)
      return 0.0f;
   if (f < 0.0031308f)
      return 12.92f * f;
   if (f < 1.0f)
      return (1.055f * powf(f, 1.0f / 2.4f)) - 0.055f;
   return 1.0f;
}

float gfx_srgb_to_lin_float(float f)
{
   if (isnan(f))
      return f;
   if (f <= 0.0f)
      return 0.0f;
   if (f <= 0.04045f)
      return f / 12.92f;
   if (f < 1.0f)
      return powf((f + 0.055f) / 1.055f, 2.4f);
   return 1.0f;
}
