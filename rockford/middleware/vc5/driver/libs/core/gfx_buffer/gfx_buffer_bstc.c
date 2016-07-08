/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/

#include "gfx_buffer_bstc.h"

#include "libs/util/gfx_util/gfx_util.h"
#include "libs/core/v3d/v3d_common.h"
#include "libs/core/v3d/v3d_gen.h"

// THRESH & NUM are used to control the selection of max endpoints.
// Default values (140,9) select max endpoints when endpoint range
// exceeds 140, and less than 9 pixels are close to the actual endpoints.
// ("close" meaning within 0,+-1,+-2.)
#define MAX_THRESH 140
#define MAX_NUM 9

GFX_LFMT_T gfx_buffer_bstc_get_decompressed_fmt(GFX_LFMT_T compressed_fmt)
{
   assert((compressed_fmt == GFX_LFMT_BSTC_RGBA_UNORM) || (compressed_fmt == GFX_LFMT_BSTCYFLIP_RGBA_UNORM));
   return GFX_LFMT_R8_G8_B8_A8_UNORM;
}

static inline union gfx_lfmt_block_data *px(const GFX_LFMT_BLOCK_ARR_T *pxs, bool yflip, uint32_t i)
{
   assert(i < 16);
   uint32_t x = i % 4, y = i / 4;
   if (yflip)
      y = 3 - y;
   return gfx_lfmt_block_arr_elem(pxs, x, y, 0);
}

/** Unpacked plane data */

struct plane
{
   uint32_t min, max;
   uint32_t bits_per_endpoint;

   uint32_t weights[16];
   uint32_t bits_per_weight;
};

static void set_endpoints(struct plane *plane, uint32_t min, uint32_t max, uint32_t bits_per_endpoint)
{
   assert(min <= gfx_mask(bits_per_endpoint));
   assert(max <= gfx_mask(bits_per_endpoint));
   plane->min = min;
   plane->max = max;
   plane->bits_per_endpoint = bits_per_endpoint;
}

static void set_weights(struct plane *plane, const uint32_t weights[16], uint32_t bits_per_weight)
{
   memcpy(plane->weights, weights, sizeof(plane->weights));
   plane->bits_per_weight = bits_per_weight;
}

/** Unpacking */

static void unpack_three_plane_rg(
   struct plane *plane,
   const V3D_BSTC_BLOCK_THREE_PLANE_RG_ELEM_T *in)
{
   if (in->max_range)
   {
      set_endpoints(plane, 0, 255, 8);
      set_weights(plane, in->u.max_range.weights, 5);
   }
   else
   {
      const V3D_BSTC_BLOCK_THREE_PLANE_RG_ELEM_NORMAL_RANGE_T *n = &in->u.normal_range;
      set_endpoints(plane, n->min, n->max, 8);
      set_weights(plane, n->weights, 4);
   }
}

static void unpack_three_plane_ba(
   struct plane *b, struct plane *a,
   const V3D_BSTC_BLOCK_THREE_PLANE_BA_T *in)
{
   if (in->max_range)
   {
      const V3D_BSTC_BLOCK_THREE_PLANE_BA_MAX_RANGE_T *m = &in->u.max_range;

      set_endpoints(b, 0, 255, 8);
      set_weights(b, m->weights, 5);

      set_endpoints(a, m->min_a, m->max_a, 6);
      set_weights(a, m->weights, 5);
   }
   else
   {
      const V3D_BSTC_BLOCK_THREE_PLANE_BA_NORMAL_RANGE_T *n = &in->u.normal_range;

      set_endpoints(b, n->min_b, n->max_b, 8);
      set_weights(b, n->weights, 4);

      set_endpoints(a, n->min_a, n->max_a, 6);
      set_weights(a, n->weights, 4);
   }
}

static void unpack_four_plane_rgb(
   struct plane *plane,
   const V3D_BSTC_BLOCK_FOUR_PLANE_RGB_ELEM_T *in)
{
   if (in->max_range)
   {
      set_endpoints(plane, 0, 255, 8);
      set_weights(plane, in->u.max_range.weights, 4);
   }
   else
   {
      const V3D_BSTC_BLOCK_FOUR_PLANE_RGB_ELEM_NORMAL_RANGE_T *n = &in->u.normal_range;
      set_endpoints(plane, n->min, n->max, 8);
      set_weights(plane, n->weights, 3);
   }
}

static void unpack_four_plane_a(
   struct plane *plane,
   const V3D_BSTC_BLOCK_FOUR_PLANE_A_T *in)
{
   set_endpoints(plane, in->min, in->max, 6);
   set_weights(plane, in->weights, 3);
}

static void unpack_planes(
   struct plane rgba[4],
   const V3D_BSTC_BLOCK_T *block)
{
   if (block->four_plane)
   {
      for (uint32_t i = 0; i != 3; ++i)
         unpack_four_plane_rgb(&rgba[i], &block->u.four_plane.rgb[i]);
      unpack_four_plane_a(&rgba[3], &block->u.four_plane.a);
   }
   else
   {
      for (uint32_t i = 0; i != 2; ++i)
         unpack_three_plane_rg(&rgba[i], &block->u.three_plane.rg[i]);
      unpack_three_plane_ba(&rgba[2], &rgba[3], &block->u.three_plane.ba);
   }
}

/** Decompression */

static uint32_t interp(uint32_t min, uint32_t max, uint32_t weight, uint32_t num_weight_bits)
{
   uint32_t max_weight = gfx_mask(num_weight_bits);
   assert(weight <= max_weight);

   uint32_t s = (min * (max_weight - weight)) + (max * weight);

   /* Approx division by max_weight */
   switch (max_weight)
   {
   case 7:
      s *= 512;
      s = s + s/8 + s/64 + s/512;
      break;
   case 15:
      s *= 256;
      s = s + s/16 + s/256 + s/4096;
      break;
   case 31:
      s *= 128;
      s = s + s/32 + s/1024;
      break;
   default:
      unreachable();
   }
   s += 2048;
   s /= 4096;
   assert(s <= 255);

   return s;
}

static uint32_t to_8(uint32_t x, uint32_t num_bits)
{
   assert(x <= gfx_mask(num_bits));
   switch (num_bits)
   {
   case 6:
      x *= 4 * 65;
      x = (x + 32) / 64;
      assert(x <= 256);
      return gfx_umin(x, 255);
   case 8:
      return x;
   default:
      unreachable();
      return 0;
   }
}

void gfx_buffer_bstc_decompress_block(GFX_LFMT_BLOCK_ARR_T *pxs, const GFX_LFMT_BLOCK_T *block_in)
{
   assert((pxs->w == 4) && (pxs->h == 4) && (pxs->d == 1));
   assert((block_in->fmt == GFX_LFMT_BSTC_RGBA_UNORM) || (block_in->fmt == GFX_LFMT_BSTCYFLIP_RGBA_UNORM));
   bool yflip = block_in->fmt == GFX_LFMT_BSTCYFLIP_RGBA_UNORM;

   V3D_BSTC_BLOCK_T block;
   v3d_unpack_bstc_block(&block, block_in->u.ui8);

   struct plane rgba[4];
   unpack_planes(rgba, &block);

   pxs->fmt = GFX_LFMT_R8_G8_B8_A8_UNORM;
   for (uint32_t i = 0; i != 16; ++i)
      for (uint32_t j = 0; j != 4; ++j)
         px(pxs, yflip, i)->ui8[j] = interp(
            to_8(rgba[j].min, rgba[j].bits_per_endpoint),
            to_8(rgba[j].max, rgba[j].bits_per_endpoint),
            rgba[j].weights[i], rgba[j].bits_per_weight);
}

/** Compression */

static uint32_t from_8(uint32_t x, uint32_t num_bits)
{
   assert(x <= 255);
   switch (num_bits)
   {
   case 6:
      x *= 63;
      x = (x + 128) / 256;
      assert(x <= 63);
      return x;
   case 8:
      return x;
   default:
      unreachable();
      return 0;
   }
}

static void calc_endpoints(
   uint32_t *min_i_out, uint32_t *max_i_out, /* These may be NULL */
   /* plane->bits_per_endpoint should be set by caller.
    * plane->min/max will be written by this function.
    * plane->bits_per_weight will be incremented in the max range case. */
   struct plane *plane,
   const GFX_LFMT_BLOCK_ARR_T *pxs, bool yflip, uint32_t ch)
{
   #define CH(I) (px(pxs, yflip, I)->ui8[ch])

   /* Find min/max */
   uint32_t min_i = 0, max_i = 0; /* These are pixel indices */
   for (uint32_t i = 1; i != 16; ++i)
   {
      if (CH(i) < CH(min_i)) min_i = i;
      if (CH(i) > CH(max_i)) max_i = i;
   }

   /* Count number of pixels close to min/max */
   uint32_t num_near_min = 0;
   uint32_t num_near_max = 0;
   for (uint32_t i = 0; i != 16; ++i)
   {
      if ((int)CH(i) < ((int)CH(min_i) + 3)) ++num_near_min;
      if ((int)CH(i) > ((int)CH(max_i) - 3)) ++num_near_max;
   }

   if ((plane->bits_per_endpoint == 8) &&
      ((CH(max_i) - CH(min_i)) > MAX_THRESH) &&
      ((num_near_min + num_near_max) < MAX_NUM))
   {
      plane->min = 0;
      plane->max = 255;
      ++plane->bits_per_weight;
   }
   else
   {
      plane->min = from_8(CH(min_i), plane->bits_per_endpoint);
      plane->max = from_8(CH(max_i), plane->bits_per_endpoint);
   }

   if (min_i_out) *min_i_out = min_i;
   if (max_i_out) *max_i_out = max_i;

   #undef CH
}

/* b->bits_per_endpoint should be set by caller.
 * b->bits_per_weight will be incremented in the max range case.
 * If *alpha_is_smaller is true, *a_min may be larger than *a_max.
 * Similarly, if *alpha_is_smaller is false, b->min may be larger than b->max. */
static void calc_endpoints_ba(
   struct plane *b, uint32_t *a_min, uint32_t *a_max, bool *alpha_is_smaller,
   const GFX_LFMT_BLOCK_ARR_T *pxs, bool yflip)
{
   #define B(I) (px(pxs, yflip, I)->ui8[2])
   #define A(I) (px(pxs, yflip, I)->ui8[3])

   uint32_t min_b_i, max_b_i;
   calc_endpoints(&min_b_i, &max_b_i, b, pxs, yflip, /*ch=*/2);

   uint32_t min_a_i = 0, max_a_i = 0;
   for (uint32_t i = 1; i != 16; ++i)
   {
      if (A(i) < A(min_a_i)) min_a_i = i;
      if (A(i) > A(max_a_i)) max_a_i = i;
   }

   assert(b->bits_per_endpoint == 8);
   uint32_t b_range = b->max - b->min;
   uint32_t a_range = to_8(from_8(A(max_a_i), 6), 6) - to_8(from_8(A(min_a_i), 6), 6);

   *alpha_is_smaller = a_range <= b_range;
   if (*alpha_is_smaller)
   {
      min_a_i = min_b_i;
      max_a_i = max_b_i;
   }
   else
   {
      /* We will never hit this case in max range mode */
      assert(b->bits_per_endpoint == 8);
      b->min = B(min_a_i);
      b->max = B(max_a_i);
   }

   *a_min = from_8(A(min_a_i), 6);
   *a_max = from_8(A(max_a_i), 6);

   #undef A
   #undef B
}

static uint32_t sq_approx(uint32_t x)
{
   assert(x < (1u << 16)); /* Or we would overflow */
   uint32_t sq = 0;
   for (uint32_t bit = 1; x; bit <<= 1)
   {
      sq += (x & bit) * bit;
      x &= ~bit;
   }
   return sq;
}

static uint32_t calc_weight(uint32_t *ssq_error, uint32_t min, uint32_t max, uint32_t value, uint32_t num_weight_bits)
{
   assert(min <= max);
   /* Note that value might be outside the range [min, max] */

   /* weight = max_weight * (value - min) / (max - min) */
   uint32_t max_weight = gfx_mask(num_weight_bits);
   int32_t num = (value < min) ? 0 : (max_weight * (value - min));
   int32_t den = max - min;

   /* Simple shift-and-subtract division */
   uint32_t weight = 0;
   for (int32_t m = 1 << (num_weight_bits - 1); m > 0; m >>= 1)
   {
      if (num >= (den * m))
      {
         weight += m;
         num -= den * m;
      }
   }
   /* Rounding step */
   if (num >= (den / 2))
   {
      weight += 1;
      num -= den;
   }

   int32_t error;
   switch (max_weight)
   {
   case 31: error = num/32; break;
   case 15: error = num/16; break;
   case 7:  error = (num*9)/64; break;
   default: unreachable();
   }
   *ssq_error += sq_approx(abs(error));

   /* value may have been > max! */
   weight = gfx_umin(weight, max_weight);

   return weight;
}

static uint32_t calc_weight_ba(uint32_t *ssq_error,
   uint32_t min_b, uint32_t max_b, uint32_t b,
   uint32_t min_a, uint32_t max_a, uint32_t a,
   uint32_t num_weight_bits,
   bool alpha_is_smaller)
{
   if (!alpha_is_smaller)
   {
      uint32_t tmp;
      tmp = min_b; min_b = min_a; min_a = tmp;
      tmp = max_b; max_b = max_a; max_a = tmp;
      tmp = b;     b     = a;     a     = tmp;
   }

   uint32_t weight = calc_weight(ssq_error, min_b, max_b, b, num_weight_bits);
   int32_t error = (int32_t)a - (int32_t)interp(min_a, max_a, weight, num_weight_bits);
   *ssq_error += sq_approx(abs(error));
   return weight;
}

/* Returns sum(error^2 for px in pxs) */
static uint32_t calc_endpoints_and_weights(
   /* plane->bits_per_endpoint/bits_per_weight should be set by caller.
    * plane->bits_per_weight will be incremented in the max range case. */
   struct plane *plane,
   const GFX_LFMT_BLOCK_ARR_T *pxs, bool yflip, uint32_t ch)
{
   calc_endpoints(NULL, NULL, plane, pxs, yflip, ch);
   uint32_t ssq_error = 0;
   for (uint32_t i = 0; i != 16; ++i)
      plane->weights[i] = calc_weight(&ssq_error,
         to_8(plane->min, plane->bits_per_endpoint),
         to_8(plane->max, plane->bits_per_endpoint),
         px(pxs, yflip, i)->ui8[ch], plane->bits_per_weight);
   return ssq_error;
}

/* Returns sum(a_error^2 + b_error^2 for px in pxs) */
static uint32_t calc_endpoints_and_weights_ba(
   /* b->bits_per_endpoint/bits_per_weight should be set by caller.
    * b->bits_per_weight will be incremented in the max range case. */
   struct plane *b, uint32_t *a_min, uint32_t *a_max,
   const GFX_LFMT_BLOCK_ARR_T *pxs, bool yflip)
{
   bool alpha_is_smaller;
   calc_endpoints_ba(b, a_min, a_max, &alpha_is_smaller, pxs, yflip);
   uint32_t ssq_error = 0;
   assert(b->bits_per_endpoint == 8);
   for (uint32_t i = 0; i != 16; ++i)
      b->weights[i] = calc_weight_ba(&ssq_error,
         b->min, b->max, px(pxs, yflip, i)->ui8[2],
         to_8(*a_min, 6), to_8(*a_max, 6), px(pxs, yflip, i)->ui8[3],
         b->bits_per_weight, alpha_is_smaller);
   return ssq_error;
}

/* Returns sum(error^2 for channel in pxs) */
static uint32_t compress_four_plane(V3D_BSTC_BLOCK_FOUR_PLANE_T *out,
   const GFX_LFMT_BLOCK_ARR_T *pxs, bool yflip)
{
   uint32_t ssq_error = 0;

   for (uint32_t i = 0; i != 3; ++i)
   {
      struct plane plane;
      plane.bits_per_endpoint = 8;
      plane.bits_per_weight = 3;
      ssq_error += calc_endpoints_and_weights(&plane, pxs, yflip, /*ch=*/i);
      out->rgb[i].max_range = plane.bits_per_weight == 4;
      if (out->rgb[i].max_range)
      {
         assert((plane.min == 0) && (plane.max == 255));
         memcpy(out->rgb[i].u.max_range.weights, plane.weights, sizeof(plane.weights));
      }
      else
      {
         V3D_BSTC_BLOCK_FOUR_PLANE_RGB_ELEM_NORMAL_RANGE_T *n = &out->rgb[i].u.normal_range;
         n->min = plane.min;
         n->max = plane.max;
         assert(plane.bits_per_weight == 3);
         memcpy(n->weights, plane.weights, sizeof(plane.weights));
      }
   }

   struct plane a;
   a.bits_per_endpoint = 6;
   a.bits_per_weight = 3;
   ssq_error += calc_endpoints_and_weights(&a, pxs, yflip, /*ch=*/3);
   out->a.min = a.min;
   out->a.max = a.max;
   assert(a.bits_per_weight == 3);
   memcpy(out->a.weights, a.weights, sizeof(a.weights));

   return ssq_error;
}

/* Returns sum(error^2 for channel in pxs) */
static uint32_t compress_three_plane(V3D_BSTC_BLOCK_THREE_PLANE_T *out,
   const GFX_LFMT_BLOCK_ARR_T *pxs, bool yflip)
{
   uint32_t ssq_error = 0;

   for (uint32_t i = 0; i != 2; ++i)
   {
      struct plane plane;
      plane.bits_per_endpoint = 8;
      plane.bits_per_weight = 4;
      ssq_error += calc_endpoints_and_weights(&plane, pxs, yflip, /*ch=*/i);
      out->rg[i].max_range = plane.bits_per_weight == 5;
      if (out->rg[i].max_range)
      {
         assert((plane.min == 0) && (plane.max == 255));
         memcpy(out->rg[i].u.max_range.weights, plane.weights, sizeof(plane.weights));
      }
      else
      {
         V3D_BSTC_BLOCK_THREE_PLANE_RG_ELEM_NORMAL_RANGE_T *n = &out->rg[i].u.normal_range;
         n->min = plane.min;
         n->max = plane.max;
         assert(plane.bits_per_weight == 4);
         memcpy(n->weights, plane.weights, sizeof(plane.weights));
      }
   }

   struct plane b;
   uint32_t a_min, a_max;
   b.bits_per_endpoint = 8;
   b.bits_per_weight = 4;
   ssq_error += calc_endpoints_and_weights_ba(&b, &a_min, &a_max, pxs, yflip);
   out->ba.max_range = b.bits_per_weight == 5;
   if (out->ba.max_range)
   {
      V3D_BSTC_BLOCK_THREE_PLANE_BA_MAX_RANGE_T *m = &out->ba.u.max_range;
      assert((b.min == 0) && (b.max == 255));
      m->min_a = a_min;
      m->max_a = a_max;
      memcpy(m->weights, b.weights, sizeof(b.weights));
   }
   else
   {
      V3D_BSTC_BLOCK_THREE_PLANE_BA_NORMAL_RANGE_T *n = &out->ba.u.normal_range;
      n->min_b = b.min;
      n->max_b = b.max;
      n->min_a = a_min;
      n->max_a = a_max;
      assert(b.bits_per_weight == 4);
      memcpy(n->weights, b.weights, sizeof(b.weights));
   }

   return ssq_error;
}

void gfx_buffer_bstc_compress_block(GFX_LFMT_BLOCK_T *block_out, const GFX_LFMT_BLOCK_ARR_T *pxs)
{
   assert(pxs->fmt == GFX_LFMT_R8_G8_B8_A8_UNORM);
   assert((pxs->w == 4) && (pxs->h == 4) && (pxs->d == 1));
   assert((block_out->fmt == GFX_LFMT_BSTC_RGBA_UNORM) || (block_out->fmt == GFX_LFMT_BSTCYFLIP_RGBA_UNORM));
   bool yflip = block_out->fmt == GFX_LFMT_BSTCYFLIP_RGBA_UNORM;

   V3D_BSTC_BLOCK_FOUR_PLANE_T four_plane;
   uint32_t fp_ssq_error = compress_four_plane(&four_plane, pxs, yflip);

   V3D_BSTC_BLOCK_THREE_PLANE_T three_plane;
   uint32_t tp_ssq_error = compress_three_plane(&three_plane, pxs, yflip);

   V3D_BSTC_BLOCK_T block;
   block.four_plane = fp_ssq_error <= tp_ssq_error;
   if (block.four_plane)
      block.u.four_plane = four_plane;
   else
      block.u.three_plane = three_plane;

   v3d_pack_bstc_block(block_out->u.ui8, &block);
}
