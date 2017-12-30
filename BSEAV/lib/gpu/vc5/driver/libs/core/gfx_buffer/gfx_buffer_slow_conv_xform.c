/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "gfx_buffer_slow_conv_xform.h"
#include "gfx_buffer_slow_conv_compr.h"
#include "gfx_buffer_v3d_tfu_srgb_conversions.h"
#include "libs/core/lfmt/lfmt_fmt_detail.h"

#include "libs/sim/qpu_float/qpu_float.h"
#include "libs/util/assert_helpers.h"

#include <math.h>

typedef uint32_t (*XFORM_SLOT_FUNC_T)(
   uint32_t dst_slot_width, GFX_LFMT_TYPE_T dst_slot_type,
   uint32_t src_slot_bits, uint32_t src_slot_width, GFX_LFMT_TYPE_T src_slot_type,
   GFX_LFMT_CHANNELS_T dst_channel,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z);

/* Call xform_slot_func for each non-X slot in dst, providing it data from src
 * slot with matching channel. xform_slot_func should return dst_slot_bits. If
 * matching channel in src not found then xform_slot_func is called with
 * src_slot_type/width/bits = 0. */
static void map_over_slots(
   /* This function takes GFX_LFMT_BLOCK_ARR_T pointers rather than
    * GFX_LFMT_BLOCK_T pointers as that's what we have at all of the call
    * sites. The arrays should contain exactly one block each. */
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z,
   XFORM_SLOT_FUNC_T xform_slot_func)
{
   /* For now just support 1 plane in, 1 plane out */
   assert((num_dst_planes == 1) && gfx_lfmt_block_arr_is_single(dst));
   assert((num_src_planes == 1) && gfx_lfmt_block_arr_is_single(src));

   GFX_LFMT_FMT_DETAIL_T dst_fd, src_fd;
   gfx_lfmt_fmt_detail(&dst_fd, dst->fmt);
   gfx_lfmt_fmt_detail(&src_fd, src->fmt);

   for (uint32_t dst_slot_idx = 0; dst_slot_idx != dst_fd.num_slots; ++dst_slot_idx)
   {
      struct gfx_lfmt_slot_detail *dst_slot = &dst_fd.slts[dst_slot_idx];

      uint32_t dst_slot_bits;
      if (dst_slot->channel == GFX_LFMT_CHANNELS_X)
         dst_slot_bits = 0;
      else
      {
         uint32_t src_slot_idx = gfx_lfmt_fmt_detail_get_slot_by_channel_maybe(
            &src_fd, dst_slot->channel);
         if (src_slot_idx == GFX_LFMT_FMT_DETAIL_INVALID_SLOT)
         {
            // channel not found in source, but in some cases we can copy from
            // another channel...
            switch (dst_slot->channel)
            {
            case GFX_LFMT_CHANNELS_R:
            case GFX_LFMT_CHANNELS_G:
            case GFX_LFMT_CHANNELS_B:
               src_slot_idx = gfx_lfmt_fmt_detail_get_slot_by_channel_maybe(
                  &src_fd, GFX_LFMT_CHANNELS_L);
               break;
            case GFX_LFMT_CHANNELS_L:
               src_slot_idx = gfx_lfmt_fmt_detail_get_slot_by_channel_maybe(
                  &src_fd, GFX_LFMT_CHANNELS_R);
               break;
            default:
               break;
            }
         }

         GFX_LFMT_TYPE_T src_slot_type = GFX_LFMT_TYPE_NONE;
         uint32_t src_slot_width = 0;
         uint32_t src_slot_bits = 0;
         if (src_slot_idx != GFX_LFMT_FMT_DETAIL_INVALID_SLOT)
         {
            struct gfx_lfmt_slot_detail *src_slot = &src_fd.slts[src_slot_idx];
            src_slot_type = src_slot->type;
            src_slot_width = src_slot->bit_width;
            src_slot_bits = gfx_lfmt_block_get_slot_bits(src->u, &src_fd, src_slot_idx);
         }

         dst_slot_bits = xform_slot_func(dst_slot->bit_width, dst_slot->type,
            src_slot_bits, src_slot_width, src_slot_type, dst_slot->channel,
            opts, x, y, z);
      }

      gfx_lfmt_block_set_slot_bits(dst->u, &dst_fd, dst_slot_idx, dst_slot_bits);
   }
}

/** Individual transform functions */

void gfx_buffer_xform_reinterpret(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   assert(num_dst_planes == num_src_planes);
   for (uint32_t p = 0; p != num_dst_planes; ++p)
   {
      /* Only allow type/channels changes -- a base change might mean different
       * block dimensions, different block size in memory, a different member
       * of the GFX_LFMT_BLOCK_T union is used, etc... */
      assert(gfx_lfmt_get_base(&dst[p].fmt) == gfx_lfmt_get_base(&src[p].fmt));
      gfx_lfmt_block_arr_copy_data(&dst[p], &src[p]);
   }
}

void gfx_buffer_xform_clamp_float_depth(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   assert(num_src_planes == 1);
   switch (src->fmt)
   {
   case GFX_LFMT_D32_S8X24_FLOAT_UINT:
   case GFX_LFMT_D32_FLOAT:
      break;
   default:
      not_impl();
   }

   /* no change in fmt */
   assert(num_dst_planes == 1);
   assert(dst->fmt == src->fmt);

   /* other slots pass through */
   *dst->u = *src->u;

   /* clamp depth: GLES3 spec on TexImage3D: "For depth component groups, the
    * depth value is clamped to [0,1]" */
   float clamped = gfx_fclamp(src->u->f[0], 0.0f, 1.0f);
   dst->u->f[0] = clamped;
}

void gfx_buffer_xform_split_d32s8(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   assert(num_src_planes == 1);
   assert(src->fmt == GFX_LFMT_D32_S8X24_FLOAT_UINT);
   assert(num_dst_planes == 2);
   assert(dst[0].fmt == GFX_LFMT_D32_FLOAT);
   assert(dst[1].fmt == GFX_LFMT_S8_UINT);

   dst[0].u->f[0] = src->u->f[0];
   dst[1].u->ui8[0] = (uint8_t)src->u->ui32[1];
}

void gfx_buffer_xform_expand_subsampled(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   assert(num_dst_planes == num_src_planes);
   for (uint32_t p = 0; p != num_dst_planes; ++p)
   {
      assert(dst[p].fmt == gfx_lfmt_drop_subsample_size(src[p].fmt));

      GFX_LFMT_BASE_DETAIL_T bd;
      gfx_lfmt_base_detail(&bd, src[p].fmt);

      assert(gfx_lfmt_block_is_single_elem(dst[p].fmt));
      assert(dst[p].w == (src[p].w * bd.block_w));
      assert(dst[p].h == (src[p].h * bd.block_h));
      assert(dst[p].d == (src[p].d * bd.block_d));

      for (uint32_t sz = 0; sz != src[p].d; ++sz)
         for (uint32_t sy = 0; sy != src[p].h; ++sy)
            for (uint32_t sx = 0; sx != src[p].w; ++sx)
            {
               const union gfx_lfmt_block_data *d =
                  gfx_lfmt_block_arr_elem(&src[p], sx, sy, sz);
               for (uint32_t bz = 0; bz != bd.block_d; ++bz)
                  for (uint32_t by = 0; by != bd.block_h; ++by)
                     for (uint32_t bx = 0; bx != bd.block_w; ++bx)
                        *gfx_lfmt_block_arr_elem(&dst[p],
                           (sx * bd.block_w) + bx,
                           (sy * bd.block_h) + by,
                           (sz * bd.block_d) + bz) = *d;
            }
   }
}

void gfx_buffer_xform_extract_yuv(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t unused_x, uint32_t unused_y, uint32_t unused_z)
{
   assert(num_dst_planes == 1);
   assert(dst->fmt == GFX_LFMT_Y8_U8_V8_UNORM);

   assert((dst->w <= 2) && (dst->h <= 2));
   uint8_t l[2][2] = {{0xff, 0xff}, {0xff, 0xff}}; /* [y][x] */
   uint8_t u = 0, v = 0;
   for (uint32_t p = 0; p != num_src_planes; ++p)
   {
      switch (src[p].fmt)
      {
      #define YUYV_CASE(CHANNELS, Y0, Y1, U, V)                      \
         case GFX_LFMT_C8_C8_C8_C8_2X1_UNORM | GFX_LFMT_CHANNELS_##CHANNELS:  \
            assert((dst->w == 2) && (dst->h == 1));                  \
            l[0][0] = src[p].u->ui8[Y0];                             \
            l[0][1] = src[p].u->ui8[Y1];                             \
            u = src[p].u->ui8[U];                                    \
            v = src[p].u->ui8[V];                                    \
            break;
      YUYV_CASE(YUYV, 0, 2, 1, 3)
      YUYV_CASE(VYUY, 3, 1, 2, 0)
      YUYV_CASE(YYUV, 0, 1, 2, 3)
      YUYV_CASE(VUYY, 3, 2, 1, 0)
      #undef YUYV_CASE

      case GFX_LFMT_Y8_UNORM:
         for (uint32_t y = 0; y != dst->h; ++y)
            for (uint32_t x = 0; x != dst->w; ++x)
               l[y][x] = gfx_lfmt_block_arr_elem(&src[p], x, y, 0)->ui8[0];
         break;

      default:
         assert(gfx_lfmt_block_dims_equal(src[p].fmt, dst->w, dst->h, 1));
         switch (gfx_lfmt_drop_subsample_size(src[p].fmt))
         {
         case GFX_LFMT_U8_V8_UNORM:
            u = src[p].u->ui8[0];
            v = src[p].u->ui8[1];
            break;
         case GFX_LFMT_V8_U8_UNORM:
            v = src[p].u->ui8[0];
            u = src[p].u->ui8[1];
            break;
         case GFX_LFMT_U8_UNORM:
            u = src[p].u->ui8[0];
            break;
         case GFX_LFMT_V8_UNORM:
            v = src[p].u->ui8[0];
            break;
         default:
            unreachable();
         }
      }
   }

   for (uint32_t y = 0; y != dst->h; ++y)
   {
      for (uint32_t x = 0; x != dst->w; ++x)
      {
         union gfx_lfmt_block_data *d = gfx_lfmt_block_arr_elem(dst, x, y, 0);
         d->ui8[0] = l[y][x];
         d->ui8[1] = u;
         d->ui8[2] = v;
      }
   }
}

static int32_t rgb_10_fixed_point_to_unorm8(int32_t in)
{
   in = gfx_srshift_rtne(in, 10);

   return gfx_sclamp(in, 0, 255);
}

void gfx_buffer_xform_yuv_to_srgb(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts_in,
   uint32_t x, uint32_t y_unused, uint32_t z)
{
   assert(num_src_planes == 1);
   assert(src->fmt == GFX_LFMT_Y8_U8_V8_UNORM);
   assert(num_dst_planes == 1);
   assert(dst->fmt == GFX_LFMT_R8_G8_B8_A8_SRGB_SRGB_SRGB_UNORM);

   /* TFUCOEF* fields are unsigned 2.10 fixed-point, except the *C fields which
    * are 10.2 fixed point. yuv inputs are just unorm/uints (i.e. 8.0 fixed point) */

   const GFX_BUFFER_XFORM_OPTIONS_YUV_TO_SRGB_T *opts = &opts_in->yuv_to_srgb;
   int32_t ay  = opts->ay;
   int32_t arr = opts->arr;
   int32_t arc = opts->arc << 8; /* now 10.10 fixed point */
   int32_t agb = opts->agb;
   int32_t agr = opts->agr;
   int32_t agc = opts->agc << 8;
   int32_t abb = opts->abb;
   int32_t abc = opts->abc << 8;

   uint8_t y = src->u->ui8[0];
   uint8_t u = src->u->ui8[1];
   uint8_t v = src->u->ui8[2];

   int32_t r = ay * y           + arr * v - arc;
   int32_t g = ay * y - agb * u - agr * v + agc;
   int32_t b = ay * y + abb * u           - abc;

   /* r,g,b in _signed_ .10 fixed point */

   r = rgb_10_fixed_point_to_unorm8(r);
   g = rgb_10_fixed_point_to_unorm8(g);
   b = rgb_10_fixed_point_to_unorm8(b);

   dst->u->ui8[0] = r;
   dst->u->ui8[1] = g;
   dst->u->ui8[2] = b;

   /* alpha to 1 */
   dst->u->ui8[3] = 0xff;
}

void gfx_buffer_xform_force_xs_to_0(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   assert(num_dst_planes == num_src_planes);
   for (uint32_t p = 0; p != num_dst_planes; ++p)
   {
      assert(dst[p].fmt == src[p].fmt);

      /* Could handle multiple blocks here but currently no need */
      assert(gfx_lfmt_block_arr_is_single(&dst[p]));
      assert(gfx_lfmt_block_arr_is_single(&src[p]));

      /* Copy all channels across */
      *dst[p].u = *src[p].u;

      /* Force the X channels to 0 */
      GFX_LFMT_FMT_DETAIL_T fd;
      gfx_lfmt_fmt_detail(&fd, dst[p].fmt);
      for (uint32_t slot_idx = 0; slot_idx != fd.num_slots; ++slot_idx)
         if (fd.slts[slot_idx].channel == GFX_LFMT_CHANNELS_X)
            gfx_lfmt_block_set_slot_bits(dst[p].u, &fd, slot_idx, 0);
   }
}

void gfx_buffer_xform_clamp_rgb_to_alpha(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   /* For now, only need to support RGBA5551... */
   assert(num_dst_planes == 1);
   assert(dst->fmt == GFX_LFMT_A1B5G5R5_UNORM || dst->fmt == GFX_LFMT_A1R5G5B5_UNORM);
   assert(num_src_planes == 1);
   assert(src->fmt == GFX_LFMT_A1B5G5R5_UNORM || src->fmt == GFX_LFMT_A1R5G5B5_UNORM);

   dst->u->ui16[0] = (src->u->ui16[0] & 1u) ? src->u->ui16[0] : 0;
}

void gfx_buffer_xform_rgba_to_la(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   unused(opts);

   assert(num_src_planes == 1);
   assert(src->fmt == GFX_LFMT_R32_G32_B32_A32_FLOAT);

   const float lum =
      src->u->f[0] * 0.25f + src->u->f[1] * 0.5f + src->u->f[2] * 0.25f; // close enough :)
   const float alpha = src->u->f[3];

   assert(num_dst_planes == 1);
   switch (dst->fmt)
   {
   case GFX_LFMT_L8_UNORM:
      dst->u->ui8[0] = gfx_float_to_unorm(lum, 8);
      break;
   case GFX_LFMT_A8_UNORM:
      dst->u->ui8[0] = gfx_float_to_unorm(alpha, 8);
      break;
   case GFX_LFMT_L8_A8_UNORM:
      dst->u->ui8[0] = gfx_float_to_unorm(lum, 8);
      dst->u->ui8[1] = gfx_float_to_unorm(alpha, 8);
      break;
   default:
      unreachable();
   }
}

void gfx_buffer_xform_repack_c1_c4(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   assert((num_src_planes == 1) && (num_dst_planes == 1));
   assert((src->fmt & ~GFX_LFMT_BASE_MASK) == (dst->fmt & ~GFX_LFMT_BASE_MASK));
   switch (gfx_lfmt_get_base(&src->fmt))
   {
   case GFX_LFMT_BASE_C1:
      assert(gfx_lfmt_get_base(&dst->fmt) == GFX_LFMT_BASE_C1X7);
      for (uint32_t i = 0; i != 8; ++i)
         gfx_lfmt_block_arr_elem(dst, i, 0, 0)->ui8[0] = (src->u->ui8[0] >> i) & 0x1;
      break;
   case GFX_LFMT_BASE_C4:
      assert(gfx_lfmt_get_base(&dst->fmt) == GFX_LFMT_BASE_C4X4);
      for (uint32_t i = 0; i != 2; ++i)
         gfx_lfmt_block_arr_elem(dst, i, 0, 0)->ui8[0] = (src->u->ui8[0] >> (i * 4)) & 0xf;
      break;
   case GFX_LFMT_BASE_C1X7:
      assert(gfx_lfmt_get_base(&dst->fmt) == GFX_LFMT_BASE_C1);
      dst->u->ui8[0] = 0;
      for (uint32_t i = 0; i != 8; ++i)
         dst->u->ui8[0] |= (gfx_lfmt_block_arr_elem(src, i, 0, 0)->ui8[0] & 0x1) << i;
      break;
   case GFX_LFMT_BASE_C4X4:
      assert(gfx_lfmt_get_base(&dst->fmt) == GFX_LFMT_BASE_C4);
      dst->u->ui8[0] = 0;
      for (uint32_t i = 0; i != 2; ++i)
         dst->u->ui8[0] |= (gfx_lfmt_block_arr_elem(src, i, 0, 0)->ui8[0] & 0xf) << (i * 4);
      break;
   default:
      unreachable();
   }
}

static bool transmute_dither_channel(
   GFX_LFMT_CHANNELS_T channel,
   const GFX_BUFFER_XFORM_OPTIONS_TRANSMUTE_T *opts)
{
   switch (channel)
   {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
      return opts->dither_rgb;
   case GFX_LFMT_CHANNELS_A:
      return opts->dither_a;
   default:
      return false;
   }
}

/* 8 bits down to num_bits with a simple ordered dither. This matches the V3D
 * TLB's dithering algorithm */
static uint32_t ord_dither_8(
   uint32_t value, uint32_t num_bits,
   uint32_t x, uint32_t y)
{
   static const uint32_t dither_matrix[16] = {
      0, 8, 2,10,
     12, 4,14, 6,
      3,11, 1, 9,
     15, 7,13, 5
   };
   uint32_t dither = dither_matrix[(x & 3) | ((y & 3) << 2)];
   switch (num_bits) {
   case 1:
      value = (value + (dither << 4) + 8) >> 8;
      break;
   case 4:
   case 5:
   case 6:
      value -= value >> num_bits;
      value += dither >> (num_bits - 4);
      value >>= 8 - num_bits;
      break;
   default:
      unreachable();
   }
   assert(value < (1u << num_bits));
   return value;
}

static uint32_t transmute_slot_func(
   uint32_t dst_slot_width, GFX_LFMT_TYPE_T dst_slot_type,
   uint32_t src_slot_bits, uint32_t src_slot_width, GFX_LFMT_TYPE_T src_slot_type,
   GFX_LFMT_CHANNELS_T channel,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts_in,
   uint32_t x, uint32_t y, uint32_t z)
{
   const GFX_BUFFER_XFORM_OPTIONS_TRANSMUTE_T *opts = &opts_in->transmute;

   if (src_slot_width == 0)
   {
      // channel not present in source
      switch (channel)
      {
      case GFX_LFMT_CHANNELS_R:
      case GFX_LFMT_CHANNELS_G:
      case GFX_LFMT_CHANNELS_B:
         return 0;
      case GFX_LFMT_CHANNELS_A:
         /* Put 1.0 in dest alpha */
         switch (dst_slot_type)
         {
         case GFX_LFMT_TYPE_UNORM:
            return gfx_mask(dst_slot_width);
         case GFX_LFMT_TYPE_SNORM:
            return gfx_mask(dst_slot_width - 1);
         case GFX_LFMT_TYPE_FLOAT:
            switch (dst_slot_width)
            {
            case 16:
               return gfx_float_to_float16(1.0f);
            case 32:
               return gfx_float_to_bits(1.0f);
            default:
               unreachable();
               return 0;
            }
         case GFX_LFMT_TYPE_INT:
         case GFX_LFMT_TYPE_UINT:
            return 1;
         default:
            unreachable();
            return 0;
         }
      case GFX_LFMT_CHANNELS_D:
      case GFX_LFMT_CHANNELS_S:
         /* Just fill with some arbitrary value. TODO This probably
          * shouldn't be the default behaviour... */
         return 0x55555555 & gfx_mask(dst_slot_width);
      default:
         unreachable();
         return 0;
      }
   }

   if (dst_slot_width == src_slot_width && dst_slot_type == src_slot_type)
      return src_slot_bits;

   switch (src_slot_type)
   {
   case GFX_LFMT_TYPE_UNORM:
   case GFX_LFMT_TYPE_SNORM:
   case GFX_LFMT_TYPE_SRGB:
      switch (dst_slot_type)
      {
      case GFX_LFMT_TYPE_UNORM:
      case GFX_LFMT_TYPE_SNORM:
      case GFX_LFMT_TYPE_SRGB:
         assert(src_slot_type == dst_slot_type);

         if (dst_slot_width > src_slot_width)
         {
            /* Replicating conversion. Note this is imperfect when
             * (dst_slot_width % src_slot_width) != 0. This happens in the TLB
             * when loading eg 565. */
            switch (src_slot_type)
            {
            case GFX_LFMT_TYPE_UNORM:
            case GFX_LFMT_TYPE_SRGB:
               return gfx_unorm_to_unorm_rep(src_slot_bits, src_slot_width, dst_slot_width);
            case GFX_LFMT_TYPE_SNORM:
               return gfx_snorm_to_snorm_rep(src_slot_bits, src_slot_width, dst_slot_width);
            default:
               unreachable();
               return 0;
            }
         }

         if (transmute_dither_channel(channel, opts))
         {
            assert((src_slot_width == 8) && (src_slot_type == GFX_LFMT_TYPE_UNORM));
            return ord_dither_8(src_slot_bits, dst_slot_width, x, y);
         }

         switch (src_slot_type)
         {
         case GFX_LFMT_TYPE_UNORM:
         case GFX_LFMT_TYPE_SRGB:
            return gfx_unorm_to_unorm(src_slot_bits, src_slot_width, dst_slot_width);
         case GFX_LFMT_TYPE_SNORM:
            return gfx_snorm_to_snorm(src_slot_bits, src_slot_width, dst_slot_width);
         default:
            unreachable();
            return 0;
         }

      case GFX_LFMT_TYPE_FLOAT:
         switch (dst_slot_width)
         {
         case 16: // float16
            switch (src_slot_type)
            {
            case GFX_LFMT_TYPE_UNORM:
               if (src_slot_width < 8)
                  /* Both TMU & TLB convert via unorm8 */
                  return gfx_unorm_to_float16(gfx_unorm_to_unorm_rep(src_slot_bits, src_slot_width, 8), 8);
               return gfx_unorm_to_float16(src_slot_bits, src_slot_width);
            case GFX_LFMT_TYPE_SNORM:
               return gfx_snorm_to_float16(src_slot_bits, src_slot_width);
            case GFX_LFMT_TYPE_SRGB:
               return gfx_float_to_float16(gfx_srgb_to_lin_float(
                     gfx_unorm_to_float(src_slot_bits, src_slot_width)));
            default:
               unreachable();
               return 0;
            }
         case 32: // float32
         {
            float f32;
            switch (src_slot_type)
            {
            case GFX_LFMT_TYPE_UNORM:
               if (opts->depth_unorm_float)
                  f32 = gfx_unorm_to_float_depth(src_slot_bits, src_slot_width);
               else if (src_slot_width < 8)
                  /* Both TMU & TLB convert via unorm8 */
                  f32 = gfx_unorm8_to_float(gfx_unorm_to_unorm_rep(src_slot_bits, src_slot_width, 8));
               else
                  f32 = gfx_unorm_to_float(src_slot_bits, src_slot_width);
               break;
            case GFX_LFMT_TYPE_SNORM:
               f32 = gfx_snorm_to_float(src_slot_bits, src_slot_width);
               break;
            case GFX_LFMT_TYPE_SRGB:
               f32 = gfx_srgb_to_lin_float(
                  gfx_unorm_to_float(src_slot_bits, src_slot_width));
               break;
            default:
               unreachable();
            }
            return gfx_float_to_bits(f32);
         }
         default:
            unreachable();
            return 0;
         }

      default:
         unreachable();
         return 0;
      }

   case GFX_LFMT_TYPE_FLOAT:
      switch (dst_slot_type)
      {
      case GFX_LFMT_TYPE_FLOAT:
         if (src_slot_width == 32 && dst_slot_width == 16)
            // downconvert for test app generating test data.
            return qpu_float_pack16(src_slot_bits);
         if (src_slot_width == 16 && dst_slot_width == 32)
            return qpu_float_unpack16(src_slot_bits);
         unreachable();
         return 0;

      case GFX_LFMT_TYPE_UFLOAT:
      {
         uint32_t src_f16;
         switch (src_slot_width)
         {
         case 16: src_f16 = src_slot_bits; break;
         case 32: src_f16 = qpu_float_pack16(src_slot_bits); break;
         default: unreachable();
         }

         switch (dst_slot_width)
         {
         case 10: return opts->f16_to_uf_rtz ? gfx_float16_to_ufloat10_rtz(src_f16) : gfx_float16_to_ufloat10(src_f16);
         case 11: return opts->f16_to_uf_rtz ? gfx_float16_to_ufloat11_rtz(src_f16) : gfx_float16_to_ufloat11(src_f16);
         default: unreachable(); return 0;
         }
      }

      case GFX_LFMT_TYPE_UNORM:
      case GFX_LFMT_TYPE_SNORM:
      {
         /* e.g. VC5 TLB 16f internal to rgb10_a2 framebuffer */

         float src_f32;
         switch (src_slot_width)
         {
         case 16: src_f32 = gfx_float_from_bits(qpu_float_unpack16(src_slot_bits)); break;
         case 32: src_f32 = gfx_float_from_bits(src_slot_bits); break;
         default: unreachable();
         }

         switch (dst_slot_type)
         {
         case GFX_LFMT_TYPE_UNORM:
            if (opts->depth_unorm_float)
               return gfx_float_to_unorm_depth(src_f32, dst_slot_width);
            return gfx_float_to_unorm(src_f32, dst_slot_width);
         case GFX_LFMT_TYPE_SNORM:
            return gfx_float_to_snorm(src_f32, dst_slot_width);
         default:
            unreachable();
            return 0;
         }
      }

      case GFX_LFMT_TYPE_SRGB:
      {
         float f32;
         switch (src_slot_width)
         {
         case 16: /* e.g. VC5 TLB 16f internal to srgb8_alpha8 framebuffer */
            f32 = gfx_float_from_bits(qpu_float_unpack16(src_slot_bits));
            break;
         default:
            unreachable();
         }

         f32 = gfx_lin_to_srgb_float(f32);

         assert(dst_slot_width == 8);
         return gfx_float_to_unorm(f32, dst_slot_width);
      }

      default:
         unreachable();
         return 0;
      }

   case GFX_LFMT_TYPE_UFLOAT:
      switch (dst_slot_type)
      {
      case GFX_LFMT_TYPE_FLOAT:
         if (src_slot_width == 11 && dst_slot_width == 32)
            return qpu_float_unpack16(gfx_ufloat11_to_float16(src_slot_bits));
         if (src_slot_width == 10 && dst_slot_width == 32)
            return qpu_float_unpack16(gfx_ufloat10_to_float16(src_slot_bits));
         if (src_slot_width == 11 && dst_slot_width == 16)
            return gfx_ufloat11_to_float16(src_slot_bits);
         if (src_slot_width == 10 && dst_slot_width == 16)
            return gfx_ufloat10_to_float16(src_slot_bits);
         unreachable();
         return 0;

      case GFX_LFMT_TYPE_UNORM:
      {
         float f32;
         switch(src_slot_width)
         {
         case 10:
            f32 = gfx_float_from_bits(
               qpu_float_unpack16(gfx_ufloat10_to_float16(src_slot_bits)));
            break;
         case 11:
            f32 = gfx_float_from_bits(
               qpu_float_unpack16(gfx_ufloat11_to_float16(src_slot_bits)));
            break;
         default:
            unreachable();
         }
         return gfx_float_to_unorm(f32, dst_slot_width);
      }

      default:
         unreachable();
         return 0;
      }

   /* when reducing uint/int bit width via glCopyTexImage, we must clamp.
    * Discarding high bits won't do. CTS packed_pixels needs it.
    *
    * TLB store will just discard high bits (e.g. rgba16ui internal storing to
    * rgb10_a2ui external) */

   case GFX_LFMT_TYPE_UINT:
      assert(src_slot_type == dst_slot_type);
      if (opts->clamp_integer)
         return gfx_umin(src_slot_bits, gfx_mask(dst_slot_width));
      return src_slot_bits & gfx_mask(dst_slot_width);

   case GFX_LFMT_TYPE_INT:
   {
      assert(src_slot_type == dst_slot_type);
      int32_t sext = gfx_sext(src_slot_bits, src_slot_width);
      if (opts->clamp_integer)
      {
         int32_t max = gfx_mask(dst_slot_width - 1);
         int32_t min = -max - 1;
         sext = gfx_sclamp(sext, min, max);
      }
      return sext & gfx_mask(dst_slot_width);
   }

   default:
      unreachable();
      return 0;
   }
}

void gfx_buffer_xform_transmute(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   map_over_slots(
      num_dst_planes, dst,
      num_src_planes, src,
      opts, x, y, z, transmute_slot_func);
}

void gfx_buffer_xform_float32_to_rgb9e5(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   assert(num_dst_planes == 1);
   assert(dst->fmt == GFX_LFMT_R9G9B9SHAREDEXP5_UFLOAT);
   assert(num_src_planes == 1);
   assert(src->fmt == GFX_LFMT_R32_G32_B32_FLOAT);

   dst->u->ui32[0] = gfx_floats_to_rgb9e5(src->u->f);
}

void gfx_buffer_xform_rgb9e5_to_float32(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   assert(num_dst_planes == 1);
   assert(dst->fmt == GFX_LFMT_R32_G32_B32_FLOAT);
   assert(num_src_planes == 1);
   assert(src->fmt == GFX_LFMT_R9G9B9SHAREDEXP5_UFLOAT);

   gfx_rgb9e5_to_floats(dst->u->f, src->u->ui32[0]);
}

static uint32_t xform_srgb_to_tfu13_slot_func(
   uint32_t dst_slot_width, GFX_LFMT_TYPE_T dst_slot_type,
   uint32_t src_slot_bits, uint32_t src_slot_width, GFX_LFMT_TYPE_T src_slot_type,
   GFX_LFMT_CHANNELS_T dst_channel,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   assert(dst_slot_type == GFX_LFMT_TYPE_UINT);

   switch (src_slot_type)
   {
   case GFX_LFMT_TYPE_UNORM:
      return src_slot_bits; /* reinterpret */
   case GFX_LFMT_TYPE_SRGB:
      assert(src_slot_width == 8);
      return gfx_buffer_srgb8_to_tfu13(src_slot_bits);
   default:
      unreachable();
   }
}

static uint32_t xform_tfu13_to_srgb_slot_func(
   uint32_t dst_slot_width, GFX_LFMT_TYPE_T dst_slot_type,
   uint32_t src_slot_bits, uint32_t src_slot_width, GFX_LFMT_TYPE_T src_slot_type,
   GFX_LFMT_CHANNELS_T dst_channel,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   assert(src_slot_type == GFX_LFMT_TYPE_UINT);

   switch (dst_slot_type)
   {
   case GFX_LFMT_TYPE_UNORM:
      return src_slot_bits; /* reinterpret */
   case GFX_LFMT_TYPE_SRGB:
      assert(dst_slot_width == 8);
      return gfx_buffer_tfu13_to_srgb8(src_slot_bits);
   default:
      unreachable();
   }
}

void gfx_buffer_xform_srgb_to_tfu13(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   map_over_slots(
      num_dst_planes, dst,
      num_src_planes, src,
      opts, x, y, z, xform_srgb_to_tfu13_slot_func);
}

void gfx_buffer_xform_tfu13_to_srgb(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   map_over_slots(
      num_dst_planes, dst,
      num_src_planes, src,
      opts, x, y, z, xform_tfu13_to_srgb_slot_func);
}

void gfx_buffer_xform_compress(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   assert(num_dst_planes == 1);
   assert(num_src_planes == 1);

   GFX_LFMT_BLOCK_T dst_block = {.fmt = dst->fmt};
   gfx_buffer_compress_block(&dst_block, src);
   assert(dst->fmt == dst_block.fmt); /* gfx_buffer_compress_block() should not change dst_block.fmt... */
   *dst->u = dst_block.u;
}

void gfx_buffer_xform_decompress(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t x, uint32_t y, uint32_t z)
{
   assert(num_dst_planes == 1);
   assert(num_src_planes == 1);

   GFX_LFMT_BLOCK_T src_block;
   gfx_lfmt_block_arr_single_get(&src_block, src);

   /* gfx_buffer_decompress_block() will write dst->fmt.. */
   GFX_LFMT_T dst_fmt = dst->fmt;
   gfx_buffer_decompress_block((GFX_LFMT_BLOCK_ARR_T *)dst, &src_block);
   /* ...check that it isn't actually changed... */
   assert(dst->fmt == dst_fmt);
}

typedef struct
{
   GFX_BUFFER_XFORM_FUNC_T func;
   const char *name;
} XFORM_FUNC_NAME_T;

#define ENTRY(FUNC) {gfx_buffer_xform_##FUNC, #FUNC}
static XFORM_FUNC_NAME_T xform_func_to_name_map[] = {
   {NULL, "NULL"},
   ENTRY(reinterpret),
   ENTRY(clamp_float_depth),
   ENTRY(split_d32s8),
   ENTRY(expand_subsampled),
   ENTRY(extract_yuv),
   ENTRY(yuv_to_srgb),
   ENTRY(force_xs_to_0),
   ENTRY(clamp_rgb_to_alpha),
   ENTRY(rgba_to_la),
   ENTRY(repack_c1_c4),
   ENTRY(transmute),
   ENTRY(float32_to_rgb9e5),
   ENTRY(rgb9e5_to_float32),
   ENTRY(srgb_to_tfu13),
   ENTRY(tfu13_to_srgb),
   ENTRY(compress),
   ENTRY(decompress)
};
#undef ENTRY

const char *gfx_buffer_xform_func_desc(GFX_BUFFER_XFORM_FUNC_T xform_func)
{
   for (size_t i = 0; i != countof(xform_func_to_name_map); ++i)
   {
      if (xform_func_to_name_map[i].func == xform_func)
         return xform_func_to_name_map[i].name;
   }

   unreachable();
   return NULL;
}

size_t gfx_buffer_sprint_xform_options(char *buf, size_t buf_size, size_t offset,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts, GFX_BUFFER_XFORM_FUNC_T xform_func)
{
   if (!opts)
      return vcos_safe_sprintf(buf, buf_size, offset, "NULL");

   offset = vcos_safe_sprintf(buf, buf_size, offset, "Xform_options(%s: ",
      gfx_buffer_xform_func_desc(xform_func));

   if (xform_func == gfx_buffer_xform_transmute)
   {
      offset = vcos_safe_sprintf(buf, buf_size, offset, "%u,%u,%u,%u",
         !!opts->transmute.dither_rgb,
         !!opts->transmute.dither_a,
         !!opts->transmute.clamp_integer,
         !!opts->transmute.depth_unorm_float);
   }
   else if (xform_func == gfx_buffer_xform_yuv_to_srgb)
   {
      offset = vcos_safe_sprintf(buf, buf_size, offset,
         "%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32
         ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32,
         opts->yuv_to_srgb.ay,
         opts->yuv_to_srgb.arr,
         opts->yuv_to_srgb.arc,
         opts->yuv_to_srgb.agb,
         opts->yuv_to_srgb.agr,
         opts->yuv_to_srgb.agc,
         opts->yuv_to_srgb.abb,
         opts->yuv_to_srgb.abc);
   }
   else
      unreachable();

   offset = vcos_safe_sprintf(buf, buf_size, offset, ")");

   return offset;
}

/** Transform sequences */

static void lcm_block_dims(uint32_t *width, uint32_t *height, uint32_t *depth,
   const GFX_LFMT_T fmts[GFX_BUFFER_MAX_PLANES])
{
   for (uint32_t p = 0; p != GFX_BUFFER_MAX_PLANES; ++p)
   {
      if (fmts[p] == GFX_LFMT_NONE)
         break;

      GFX_LFMT_BASE_DETAIL_T bd;
      gfx_lfmt_base_detail(&bd, fmts[p]);

      *width = gfx_lcm(*width, bd.block_w);
      *height = gfx_lcm(*height, bd.block_h);
      *depth = gfx_lcm(*depth, bd.block_d);
   }
}

void gfx_buffer_xform_seq_add_with_opts_planar(
   GFX_BUFFER_XFORM_SEQ_T *seq,
   GFX_BUFFER_XFORM_FUNC_T xform_func,
   uint32_t dst_num_planes,
   const GFX_LFMT_T *dst_fmts,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts)
{
   assert(seq->count < countof(seq->steps));

   const GFX_BUFFER_XFORM_T *prev_xform = (seq->count > 0) ?
      &seq->steps[seq->count - 1] : NULL;
   if (prev_xform &&
      gfx_lfmts_equal(
         dst_num_planes, dst_fmts,
         gfx_buffer_lfmts_num_planes(prev_xform->dst_fmts), prev_xform->dst_fmts) &&
      (xform_func != gfx_buffer_xform_clamp_float_depth) &&
      (xform_func != gfx_buffer_xform_force_xs_to_0) &&
      (xform_func != gfx_buffer_xform_clamp_rgb_to_alpha))
   {
      return; // it would be no-op: don't add.
   }

   GFX_BUFFER_XFORM_T *xform = &seq->steps[seq->count++];

   xform->xform_func = xform_func;

   gfx_buffer_lfmts_none(xform->dst_fmts);
   memcpy(xform->dst_fmts, dst_fmts, dst_num_planes * sizeof(dst_fmts[0]));

   xform->width = 1;
   xform->height = 1;
   xform->depth = 1;
   if (prev_xform)
      lcm_block_dims(&xform->width, &xform->height, &xform->depth, prev_xform->dst_fmts);
   lcm_block_dims(&xform->width, &xform->height, &xform->depth, xform->dst_fmts);

   xform->has_opts = true;
   if (opts)
      xform->opts = *opts;
   if (xform_func == gfx_buffer_xform_yuv_to_srgb)
   {
      if (!opts)
         gfx_buffer_default_yuv_to_srgb_options(&xform->opts.yuv_to_srgb);
   }
   else if (xform_func == gfx_buffer_xform_transmute)
   {
      if (!opts)
         gfx_buffer_default_transmute_options(&xform->opts.transmute);
   }
   else
   {
      assert(!opts);
      xform->has_opts = false;
   }
}

size_t gfx_buffer_sprint_xform(char *buf, size_t buf_size, size_t offset,
   const GFX_BUFFER_XFORM_T *xform)
{
   offset = vcos_safe_sprintf(buf, buf_size, offset, "Xform(func=%s, ",
      gfx_buffer_xform_func_desc(xform->xform_func));

   offset = vcos_safe_sprintf(buf, buf_size, offset, "dst_fmts=(");
   for (int i = 0; i != GFX_BUFFER_MAX_PLANES; ++i)
   {
      offset = gfx_lfmt_sprint(buf, buf_size, offset, xform->dst_fmts[i]);
      if (i != GFX_BUFFER_MAX_PLANES - 1)
         offset = vcos_safe_sprintf(buf, buf_size, offset, ",");
   }
   offset = vcos_safe_sprintf(buf, buf_size, offset, "), ");

   offset = vcos_safe_sprintf(buf, buf_size, offset, "opts=");
   offset = gfx_buffer_sprint_xform_options(buf, buf_size, offset,
      xform->has_opts ? &xform->opts : NULL, xform->xform_func);

   offset = vcos_safe_sprintf(buf, buf_size, offset, ")");

   return offset;
}

size_t gfx_buffer_sprint_xform_seq(char *buf, size_t buf_size, size_t offset,
   const GFX_BUFFER_XFORM_SEQ_T *seq)
{
   assert(seq->count != 0);

   offset = vcos_safe_sprintf(buf, buf_size, offset, "Xform_seq(");

   for (uint32_t i = 0; i < seq->count; ++i)
   {
      offset = gfx_buffer_sprint_xform(buf, buf_size, offset, &seq->steps[i]);
      if (i != seq->count - 1)
         offset = vcos_safe_sprintf(buf, buf_size, offset, ", ");
   }

   offset = vcos_safe_sprintf(buf, buf_size, offset, ")");

   return offset;
}

const char* gfx_buffer_desc_xform(const GFX_BUFFER_XFORM_T *xform)
{
   static GFX_BUFFER_SPRINT_XFORM(desc, xform);
   return desc;
}

const char* gfx_buffer_desc_xform_seq(const GFX_BUFFER_XFORM_SEQ_T *seq)
{
   static GFX_BUFFER_SPRINT_XFORM_SEQ(desc, seq);
   return desc;
}

/** Transform sequence construction helpers */

static GFX_LFMT_BASE_T expand_c1_c4(GFX_LFMT_BASE_T base)
{
   switch (base)
   {
   case GFX_LFMT_BASE_C1: return GFX_LFMT_BASE_C1X7;
   case GFX_LFMT_BASE_C4: return GFX_LFMT_BASE_C4X4;
   default:          return base;
   }
}

void gfx_buffer_xform_seq_construct_continue(
   GFX_BUFFER_XFORM_SEQ_T *xform_seq,
   uint32_t dst_num_planes, const GFX_LFMT_T *dst_fmts,
   gfx_buffer_xform_convs_t convs,
   const GFX_BUFFER_XFORM_OPTIONS_TRANSMUTE_T *transmute_options)
{
   for (uint32_t i = 0; i != dst_num_planes; ++i)
      assert(dst_fmts[i] == gfx_lfmt_fmt(dst_fmts[i]));

   const GFX_LFMT_T *src_fmts = gfx_buffer_xform_seq_peek(xform_seq)->dst_fmts;
   const uint32_t src_num_planes = gfx_buffer_lfmts_num_planes(src_fmts);

   if (gfx_lfmts_equal(
         dst_num_planes, dst_fmts,
         src_num_planes, src_fmts))
   {
      // nothing to do.
      return;
   }

   /* special case: D32 S8 split */
   {
      GFX_LFMT_T fmts_d32_s8_packed[] = { GFX_LFMT_D32_S8X24_FLOAT_UINT };
      GFX_LFMT_T fmts_d32_s8_split[] = { GFX_LFMT_D32_FLOAT, GFX_LFMT_S8_UINT };

      if (gfx_lfmts_equal(
            countof(fmts_d32_s8_packed), fmts_d32_s8_packed,
            src_num_planes, src_fmts) &&
         gfx_lfmts_equal(
            countof(fmts_d32_s8_split), fmts_d32_s8_split,
            dst_num_planes, dst_fmts))
      {
         gfx_buffer_xform_seq_add_planar(xform_seq, gfx_buffer_xform_split_d32s8,
            dst_num_planes, dst_fmts);
         return;
      }
   }

   /* special case: rgb9e5 */
   {
      GFX_LFMT_T fmt_rgb9e5 = GFX_LFMT_R9G9B9SHAREDEXP5_UFLOAT;
      GFX_LFMT_T fmt_f32 = GFX_LFMT_R32_G32_B32_FLOAT;

      /* foo -> rgb9e5 implemented as foo -> float32 -> rgb9e5 */
      if (gfx_lfmts_equal(
            dst_num_planes, dst_fmts,
            1, &fmt_rgb9e5))
      {
         gfx_buffer_xform_seq_construct_continue(xform_seq,
            1, &fmt_f32, convs, transmute_options);
         gfx_buffer_xform_seq_add_planar(xform_seq,
            gfx_buffer_xform_float32_to_rgb9e5, dst_num_planes, dst_fmts);
         return;
      }

      /* rgb9e5 -> foo implemented as rgb9e5 -> float32 -> foo */
      if (gfx_lfmts_equal(
            src_num_planes, src_fmts,
            1, &fmt_rgb9e5))
      {
         gfx_buffer_xform_seq_add(xform_seq,
            gfx_buffer_xform_rgb9e5_to_float32, fmt_f32);
         gfx_buffer_xform_seq_construct_continue(xform_seq,
            dst_num_planes, dst_fmts, convs, transmute_options);
         return;
      }
   }

   /* special case: c1/c4 */
   {
      GFX_LFMT_T expanded_src_fmt = src_fmts[0];
      gfx_lfmt_set_base(&expanded_src_fmt, expand_c1_c4(gfx_lfmt_get_base(&src_fmts[0])));
      if (expanded_src_fmt != src_fmts[0])
      {
         assert(src_num_planes == 1);
         gfx_buffer_xform_seq_add(xform_seq, gfx_buffer_xform_repack_c1_c4, expanded_src_fmt);
         gfx_buffer_xform_seq_construct_continue(xform_seq,
            dst_num_planes, dst_fmts, convs, transmute_options);
         return;
      }

      GFX_LFMT_T expanded_dst_fmt = dst_fmts[0];
      gfx_lfmt_set_base(&expanded_dst_fmt, expand_c1_c4(gfx_lfmt_get_base(&expanded_dst_fmt)));
      if (expanded_dst_fmt != dst_fmts[0])
      {
         assert(dst_num_planes == 1);
         gfx_buffer_xform_seq_construct_continue(xform_seq,
            1, &expanded_dst_fmt, convs, transmute_options);
         gfx_buffer_xform_seq_add(xform_seq, gfx_buffer_xform_repack_c1_c4, dst_fmts[0]);
         return;
      }
   }

   if (convs >= GFX_BUFFER_XFORM_CONVS_BROAD)
   {
      if (gfx_lfmt_is_compressed(src_fmts[0]))
      {
         assert(src_num_planes == 1);
         gfx_buffer_xform_seq_add(xform_seq, gfx_buffer_xform_decompress, gfx_buffer_get_decompressed_fmt(src_fmts[0]));
         gfx_buffer_xform_seq_construct_continue(xform_seq,
            dst_num_planes, dst_fmts, convs, transmute_options);
         return;
      }

      if (gfx_lfmt_is_compressed(dst_fmts[0]))
      {
         assert(dst_num_planes == 1);
         GFX_LFMT_T decomp_fmt = gfx_buffer_get_decompressed_fmt(dst_fmts[0]);
         gfx_buffer_xform_seq_construct_continue(xform_seq,
            1, &decomp_fmt, convs, transmute_options);
         gfx_buffer_xform_seq_add(xform_seq, gfx_buffer_xform_compress, dst_fmts[0]);
         return;
      }

      if (gfx_lfmt_any_plane(gfx_lfmt_has_u, src_num_planes, src_fmts) &&
         gfx_lfmt_any_plane(gfx_lfmt_has_v, src_num_planes, src_fmts) &&
         !gfx_lfmt_any_plane(gfx_lfmt_has_y, dst_num_planes, dst_fmts) &&
         !gfx_lfmt_any_plane(gfx_lfmt_has_u, dst_num_planes, dst_fmts) &&
         !gfx_lfmt_any_plane(gfx_lfmt_has_v, dst_num_planes, dst_fmts))
      {
         gfx_buffer_xform_seq_add(xform_seq, gfx_buffer_xform_extract_yuv, GFX_LFMT_Y8_U8_V8_UNORM);
         gfx_buffer_xform_seq_add(xform_seq, gfx_buffer_xform_yuv_to_srgb, GFX_LFMT_R8_G8_B8_A8_SRGB_SRGB_SRGB_UNORM);
         gfx_buffer_xform_seq_construct_continue(xform_seq,
            dst_num_planes, dst_fmts, convs, transmute_options);
         return;
      }

      {
         GFX_LFMT_T expanded_src_fmts[GFX_BUFFER_MAX_PLANES];
         for (uint32_t p = 0; p != src_num_planes; ++p)
            expanded_src_fmts[p] = gfx_lfmt_drop_subsample_size(src_fmts[p]);
         if (!gfx_lfmts_equal(src_num_planes, expanded_src_fmts, src_num_planes, src_fmts))
         {
            gfx_buffer_xform_seq_add_planar(xform_seq, gfx_buffer_xform_expand_subsampled,
               src_num_planes, expanded_src_fmts);
            gfx_buffer_xform_seq_construct_continue(xform_seq,
               dst_num_planes, dst_fmts, convs, transmute_options);
            return;
         }
      }

      if (convs >= GFX_BUFFER_XFORM_CONVS_DEBUG_VIEW)
      {
         assert((dst_num_planes == 1) && (src_num_planes == 1)); /* TODO */
         GFX_LFMT_T dst_fmt = dst_fmts[0];
         GFX_LFMT_T reinterp_fmt = src_fmts[0];

         if (!gfx_lfmt_has_y(dst_fmt) && !gfx_lfmt_has_u(dst_fmt) && !gfx_lfmt_has_v(dst_fmt) &&
               ((gfx_lfmt_get_channels(&reinterp_fmt) == GFX_LFMT_CHANNELS_Y) ||
               (gfx_lfmt_get_channels(&reinterp_fmt) == GFX_LFMT_CHANNELS_U) ||
               (gfx_lfmt_get_channels(&reinterp_fmt) == GFX_LFMT_CHANNELS_V)))
            gfx_lfmt_set_channels(&reinterp_fmt, GFX_LFMT_CHANNELS_L);

         if (!gfx_lfmt_has_depth(dst_fmt) && !gfx_lfmt_has_stencil(dst_fmt))
         {
            if (gfx_lfmt_has_depth(reinterp_fmt) && gfx_lfmt_has_stencil(reinterp_fmt))
               reinterp_fmt = gfx_lfmt_ds_to_rg(reinterp_fmt);
            else
               reinterp_fmt = gfx_lfmt_ds_to_l(reinterp_fmt);
         }

         if (!gfx_lfmt_contains_int(dst_fmt))
            reinterp_fmt = gfx_lfmt_int_to_norm(reinterp_fmt);

         if (!gfx_lfmt_contains_snorm(dst_fmt))
            reinterp_fmt = gfx_lfmt_snorm_to_unorm(reinterp_fmt); /* TODO Actually map -1 to 0 and 1 to 1? */

         if (!gfx_lfmt_contains_srgb(dst_fmt))
            reinterp_fmt = gfx_lfmt_srgb_to_unorm(reinterp_fmt);

         if (src_fmts[0] != reinterp_fmt)
         {
            gfx_buffer_xform_seq_add(xform_seq, gfx_buffer_xform_reinterpret, reinterp_fmt);
            gfx_buffer_xform_seq_construct_continue(xform_seq,
               dst_num_planes, dst_fmts, convs, transmute_options);
            return;
         }
      }
   }

   GFX_BUFFER_XFORM_OPTIONS_T xform_opts;
   if (transmute_options)
      xform_opts.transmute = *transmute_options;
   gfx_buffer_xform_seq_add_with_opts_planar(xform_seq,
      gfx_buffer_xform_transmute, dst_num_planes, dst_fmts,
      transmute_options ? &xform_opts : NULL);
}

/** Transform sequence application */

void gfx_buffer_get_xform_seq_quantum(
   uint32_t *width, uint32_t *height, uint32_t *depth,
   const GFX_BUFFER_XFORM_SEQ_T *xform_seq)
{
   *width = 1;
   *height = 1;
   *depth = 1;

   for (uint32_t i = 0; i != xform_seq->count; ++i)
   {
      const GFX_BUFFER_XFORM_T *xform = gfx_buffer_xform_seq_i(xform_seq, i);
      *width = gfx_lcm(*width, xform->width);
      *height = gfx_lcm(*height, xform->height);
      *depth = gfx_lcm(*depth, xform->depth);
   }
}

static void window_arrs(
   GFX_LFMT_BLOCK_ARR_T *sub_arrs,
   uint32_t num_planes, const GFX_LFMT_BLOCK_ARR_T *arrs,
   uint32_t x, uint32_t y, uint32_t z,
   uint32_t w, uint32_t h, uint32_t d)
{
   for (uint32_t p = 0; p != num_planes; ++p)
   {
      GFX_LFMT_BASE_DETAIL_T bd;
      gfx_lfmt_base_detail(&bd, arrs[p].fmt);

      sub_arrs[p] = arrs[p];
      gfx_lfmt_block_arr_window(&sub_arrs[p],
         gfx_udiv_exactly(x, bd.block_w),
         gfx_udiv_exactly(y, bd.block_h),
         gfx_udiv_exactly(z, bd.block_d),
         gfx_udiv_exactly(w, bd.block_w),
         gfx_udiv_exactly(h, bd.block_h),
         gfx_udiv_exactly(d, bd.block_d));
   }
}

void gfx_buffer_apply_xform_seq(
   const GFX_LFMT_BLOCK_ARR_T *out,
   const GFX_LFMT_BLOCK_ARR_T *in,
   const GFX_BUFFER_XFORM_SEQ_T *xform_seq,
   uint32_t x, uint32_t y, uint32_t z)
{
   uint32_t width, height, depth;
   gfx_lfmt_block_arr_dims_in_elems(&width, &height, &depth, &in[0]);

   assert(xform_seq->count >= 1);

   uint32_t num_src_planes = gfx_buffer_lfmts_num_planes(xform_seq->steps[0].dst_fmts);
   const GFX_LFMT_BLOCK_ARR_T *src = in;
   for (uint32_t p = 0; p != num_src_planes; ++p)
   {
      assert(src[p].fmt == xform_seq->steps[0].dst_fmts[p]);
      assert(gfx_lfmt_block_arr_dims_in_elems_equals(&src[p], width, height, depth));
   }

   GFX_LFMT_BLOCK_ARR_T tmp[2][GFX_BUFFER_MAX_PLANES];
   for (uint32_t i = 0; i != 2; ++i)
      for (uint32_t p = 0; p != GFX_BUFFER_MAX_PLANES; ++p)
         GFX_LFMT_BLOCK_ARR_ALLOCA(&tmp[i][p], width, height, depth);

   for (uint32_t i = 1; i != xform_seq->count; ++i)
   {
      const GFX_BUFFER_XFORM_T *xform = gfx_buffer_xform_seq_i(xform_seq, i);

      uint32_t num_dst_planes = gfx_buffer_lfmts_num_planes(xform->dst_fmts);
      const GFX_LFMT_BLOCK_ARR_T *dst;
      if (i == (xform_seq->count - 1))
      {
         dst = out;
         for (uint32_t p = 0; p != num_dst_planes; ++p)
         {
            assert(dst[p].fmt == xform->dst_fmts[p]);
            assert(gfx_lfmt_block_arr_dims_in_elems_equals(&dst[p], width, height, depth));
         }
      }
      else
      {
         GFX_LFMT_BLOCK_ARR_T *free_tmp = (src == tmp[0]) ? tmp[1] : tmp[0];
         for (uint32_t p = 0; p != num_dst_planes; ++p)
            free_tmp[p].fmt = xform->dst_fmts[p];
         dst = free_tmp;
      }

      uint32_t sub_z;
      for (sub_z = 0; sub_z < depth; sub_z += xform->depth)
      {
         uint32_t sub_y;
         for (sub_y = 0; sub_y < height; sub_y += xform->height)
         {
            uint32_t sub_x;
            for (sub_x = 0; sub_x < width; sub_x += xform->width)
            {
               GFX_LFMT_BLOCK_ARR_T sub_dst[GFX_BUFFER_MAX_PLANES], sub_src[GFX_BUFFER_MAX_PLANES];
               window_arrs(sub_dst, num_dst_planes, dst,
                  sub_x, sub_y, sub_z,
                  xform->width, xform->height, xform->depth);
               window_arrs(sub_src, num_src_planes, src,
                  sub_x, sub_y, sub_z,
                  xform->width, xform->height, xform->depth);

               xform->xform_func(
                  num_dst_planes, sub_dst,
                  num_src_planes, sub_src,
                  xform->has_opts ? &xform->opts : NULL,
                  x + sub_x, y + sub_y, z + sub_z);
            }
            assert(sub_x == width);
         }
         assert(sub_y == height);
      }
      assert(sub_z == depth);

      num_src_planes = num_dst_planes;
      src = dst;
   }

   if (src != out)
   {
      assert(xform_seq->count == 1);
      for (uint32_t p = 0; p != num_src_planes; ++p)
      {
         assert(out[p].fmt == src[p].fmt);
         gfx_lfmt_block_arr_copy_data(&out[p], &src[p]);
      }
   }
}
