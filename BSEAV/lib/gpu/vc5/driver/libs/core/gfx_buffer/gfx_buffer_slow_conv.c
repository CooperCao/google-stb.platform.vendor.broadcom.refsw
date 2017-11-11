/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "gfx_buffer_slow_conv.h"

#include "libs/core/lfmt/lfmt_fmt_detail.h"
#include "gfx_buffer_uif_config.h"
#include "libs/util/assert_helpers.h"
#include "libs/util/log/log.h"
#include "libs/sim/qpu_float/qpu_float.h"
#include "vcos.h"

LOG_DEFAULT_CAT("gfx_buffer_slow_conv")

size_t gfx_buffer_block_offset(
   const GFX_BUFFER_DESC_PLANE_T *plane, const GFX_LFMT_BASE_DETAIL_T *bd,
   uint32_t x_in_blocks, uint32_t y_in_blocks, uint32_t z_in_blocks,
   uint32_t height)
{
   uint32_t dims = gfx_lfmt_dims_from_enum(gfx_lfmt_get_dims(&plane->lfmt));
   const GFX_LFMT_SWIZZLING_T swizzling = gfx_lfmt_get_swizzling(&plane->lfmt);

   assert((dims >= 2) || (y_in_blocks == 0));
   assert((dims >= 3) || (z_in_blocks == 0));

   if (gfx_lfmt_get_yflip(&plane->lfmt))
   {
      uint32_t h_in_blocks = gfx_udiv_round_up(height, bd->block_h);
      assert(y_in_blocks < h_in_blocks);
      y_in_blocks = h_in_blocks - y_in_blocks - 1;
   }

   if (gfx_lfmt_is_sand_family((GFX_LFMT_T)swizzling))
   {
      uint32_t col_w_in_bytes = gfx_lfmt_sandcol_w_in_bytes(swizzling);
      uint32_t col_w_in_blocks = gfx_udiv_exactly(col_w_in_bytes, bd->bytes_per_block);
      uint32_t col = x_in_blocks / col_w_in_blocks;
      size_t offset = plane->offset +
         (col * col_w_in_blocks * plane->pitch) +
         (((y_in_blocks * col_w_in_blocks) + (x_in_blocks % col_w_in_blocks)) * bd->bytes_per_block);

      switch (gfx_lfmt_dram_map_version(swizzling))
      {
      case 2:
         break;
      case 5:
         if (offset & ((col_w_in_bytes == 128) ? 0x100 : 0x200))
            offset ^= 32;
         break;
      case 8:
         assert(col_w_in_bytes == 256);
         if ((offset ^ (offset >> 1)) & 0x100)
            offset ^= 32;
         break;
      default:
         unreachable();
      }

      /* See GFXH-1344 */
      if (gfx_lfmt_is_bigend_sand_family((GFX_LFMT_T)swizzling))
      {
         assert((plane->offset % 32) == 0);
         offset ^= 31;
         offset -= bd->bytes_per_block - 1;
      }

      return offset;
   }

   switch (gfx_lfmt_collapse_uif_family(swizzling))
   {
   case GFX_LFMT_SWIZZLING_RSO:
      return plane->offset +
         (x_in_blocks * bd->bytes_per_block) +
         (y_in_blocks * plane->pitch) +
         (z_in_blocks * plane->slice_pitch);
   case GFX_LFMT_SWIZZLING_LT:
   {
      uint32_t x_in_utiles = x_in_blocks / bd->ut_w_in_blocks_2d;
      uint32_t y_in_utiles = y_in_blocks / bd->ut_h_in_blocks_2d;
      uint32_t ut_offset;

      ut_offset = (y_in_utiles * bd->ut_h_in_blocks_2d * plane->pitch) +
         (x_in_utiles * GFX_LFMT_UTILE_SIZE);

      return plane->offset + ut_offset +
         ((((y_in_blocks % bd->ut_h_in_blocks_2d) * bd->ut_w_in_blocks_2d) +
         (x_in_blocks % bd->ut_w_in_blocks_2d)) * bd->bytes_per_block) +
         z_in_blocks * plane->slice_pitch;
   }
   case GFX_LFMT_SWIZZLING_UIF:
   case GFX_LFMT_SWIZZLING_UBLINEAR:
   {
      const uint32_t ub_w_in_blocks = gfx_lfmt_ub_w_in_blocks_2d(bd, swizzling);
      const uint32_t ub_h_in_blocks = gfx_lfmt_ub_h_in_blocks_2d(bd, swizzling);
      const uint32_t x_in_ub = x_in_blocks / ub_w_in_blocks;
      const uint32_t y_in_ub = y_in_blocks / ub_h_in_blocks;
      uint32_t ub_number;

      if (swizzling == GFX_LFMT_SWIZZLING_UBLINEAR) {
         if (plane->pitch == bd->bytes_per_block * gfx_lfmt_ub_w_in_blocks_2d(bd, swizzling)) {
            assert(x_in_ub == 0);
            ub_number = y_in_ub;
         } else if (plane->pitch == 2 * bd->bytes_per_block * gfx_lfmt_ub_w_in_blocks_2d(bd, swizzling)) {
            assert(x_in_ub <= 1);
            ub_number = y_in_ub * 2 + x_in_ub;
         } else {
            unreachable(); /* ubl only ever 1 or 2 ub wide. */
         }
      } else {
         const uint32_t sdram_page_width_in_ub = GFX_UIF_COL_W_IN_UB;
         const bool xor_mode = gfx_lfmt_is_uif_xor_family((GFX_LFMT_T)swizzling);
         const uint32_t y_in_ub_xor = y_in_ub ^ GFX_UIF_XOR_ADDR;
         const uint32_t padded_image_h_in_ub =
            plane->pitch * gfx_lfmt_ucol_w_in_blocks_2d(bd, swizzling) /
            (GFX_UIF_COL_W_IN_UB * GFX_UIF_UB_SIZE);
         const bool odd_col = (x_in_ub / sdram_page_width_in_ub) % 2;

         /* from uif spreadsheet */
         ub_number = (x_in_ub / sdram_page_width_in_ub)
            * (padded_image_h_in_ub - 1) * sdram_page_width_in_ub
            + sdram_page_width_in_ub * (odd_col && xor_mode ? y_in_ub_xor : y_in_ub)
            + x_in_ub;
      }

      if (gfx_lfmt_is_noutile_family((GFX_LFMT_T)swizzling))
      {
         assert(z_in_blocks == 0);
         return plane->offset + ub_number * GFX_UIF_UB_SIZE +
            ((y_in_blocks % ub_h_in_blocks) * ub_w_in_blocks +
            (x_in_blocks % ub_w_in_blocks)) * bd->bytes_per_block;
      }
      else
      {
         /* 2x2 utiles in a uifblock, raster order */
         const uint32_t ut_x_within_ub = (x_in_blocks / bd->ut_w_in_blocks_2d) % 2;
         const uint32_t ut_y_within_ub = (y_in_blocks / bd->ut_h_in_blocks_2d) % 2;

         return plane->offset + ub_number * GFX_UIF_UB_SIZE +
            (ut_y_within_ub * 2 + ut_x_within_ub) * GFX_LFMT_UTILE_SIZE +
            ((((y_in_blocks % bd->ut_h_in_blocks_2d) * bd->ut_w_in_blocks_2d) +
            (x_in_blocks % bd->ut_w_in_blocks_2d)) * bd->bytes_per_block) +
            z_in_blocks * plane->slice_pitch;
      }
   }
   default:
      unreachable();
      return 0;
   }
}

static void read_block(void *block,
   const GFX_BUFFER_BLIT_TGT_FUNC_T *src, size_t offset,
   const GFX_LFMT_BASE_DETAIL_T *src_bd)
{
   if (src->read)
      src->read(block, offset, src_bd->bytes_per_block, src->p);
   else
      memcpy(block, (uint8_t *)src->p + offset, src_bd->bytes_per_block);
}

static void write_block(
   const GFX_BUFFER_BLIT_TGT_FUNC_T *dst, size_t offset,
   const GFX_LFMT_BASE_DETAIL_T *dst_bd,
   const void *block)
{
   if (dst->write)
      dst->write(offset, block, dst_bd->bytes_per_block, dst->p);
   else
      memcpy((uint8_t *)dst->p + offset, block, dst_bd->bytes_per_block);
}

static void get_px_and_apply_xform_seq(GFX_LFMT_BLOCK_T *dst_px,
   uint32_t x, uint32_t y, uint32_t z,
   const GFX_BUFFER_XFORM_SEQ_T *xform_seq,
   const GFX_BUFFER_BLIT_TGT_FUNC_T *src)
{
   GFX_LFMT_BASE_DETAIL_T src_bd;
   GFX_LFMT_BLOCK_T src_px;
   const GFX_BUFFER_DESC_PLANE_T *src_plane;

   assert(src->desc.num_planes == 1);

   src_plane = src->desc.planes + 0;

   gfx_lfmt_base_detail(&src_bd, src_plane->lfmt);

   size_t src_block_offset = gfx_buffer_block_offset(src_plane, &src_bd, x, y, z, src->desc.height);

   src_px.fmt = gfx_buffer_xform_seq_i(xform_seq, 0)->dst_fmts[0];
   assert(src_px.fmt == gfx_lfmt_fmt(src_plane->lfmt));
   read_block(&src_px.u, src, src_block_offset, &src_bd);

   gfx_buffer_apply_xform_seq_single_block(dst_px, &src_px, xform_seq, x, y, z);
}

static void blend_ui8(uint8_t out[], uint8_t in0[], uint8_t in1[], int slots_n)
{
   int i;

   for (i=0; i<slots_n; ++i) {
      out[i] = (in0[i] + in1[i]) >> 1;
   }
}

static void blend_f16(uint16_t out[], uint16_t in0[], uint16_t in1[], int slots_n)
{
   int i;

   for (i=0; i<slots_n; ++i)
   {
      /* should match TFU */
      uint32_t in0_f32 = qpu_float_unpack16(in0[i]);
      uint32_t in1_f32 = qpu_float_unpack16(in1[i]);
      uint32_t in0o2_f32 = qpu_fmul(in0_f32, gfx_float_to_bits(0.5f), false);
      uint32_t in1o2_f32 = qpu_fmul(in1_f32, gfx_float_to_bits(0.5f), false);
      uint32_t avg_f32 = qpu_fadd(
         qpu_float_unpack16(qpu_float_pack16(in0o2_f32)),
         qpu_float_unpack16(qpu_float_pack16(in1o2_f32)),
         false);
      out[i] = qpu_float_pack16(avg_f32);
   }
}

static void blend_f32(float out[], float in0[], float in1[], int slots_n)
{
   int i;

   for (i=0; i<slots_n; ++i) {
      out[i] = in0[i]/2.0f + in1[i]/2.0f;
   }
}

static void blend(GFX_LFMT_BLOCK_T *dst_px, GFX_LFMT_BLOCK_T *src_px0, GFX_LFMT_BLOCK_T *src_px1)
{
   dst_px->fmt = src_px0->fmt;
   switch(gfx_lfmt_get_type(&dst_px->fmt)) {
   case GFX_LFMT_TYPE_UNORM:
      switch(gfx_lfmt_get_base(&dst_px->fmt)) {
      case GFX_LFMT_BASE_C8:
      case GFX_LFMT_BASE_C8_C8:
      case GFX_LFMT_BASE_C8_C8_C8:
         /* blending the extra slots does no harm */
      case GFX_LFMT_BASE_C8_C8_C8_C8:
         blend_ui8(dst_px->u.ui8, src_px0->u.ui8, src_px1->u.ui8, 4);
         break;
      default: unreachable();
      }
      break;
   case GFX_LFMT_TYPE_FLOAT:
      switch(gfx_lfmt_get_base(&dst_px->fmt)) {
      case GFX_LFMT_BASE_C16:
      case GFX_LFMT_BASE_C16_C16:
      case GFX_LFMT_BASE_C16_C16_C16:
      case GFX_LFMT_BASE_C16_C16_C16_C16:
         blend_f16(dst_px->u.ui16, src_px0->u.ui16, src_px1->u.ui16, 4);
         break;
      case GFX_LFMT_BASE_C32:
      case GFX_LFMT_BASE_C32_C32:
      case GFX_LFMT_BASE_C32_C32_C32:
      case GFX_LFMT_BASE_C32_C32_C32_C32:
         blend_f32(dst_px->u.f, src_px0->u.f, src_px1->u.f, 4);
         break;
      default: unreachable();
      }
      break;
   default: unreachable();
   }
}

/* order of src_px matters: TFU averages horizontally first, then vertically.
*/
static void decimate_cascade(GFX_LFMT_BLOCK_T *dst_px, GFX_LFMT_BLOCK_T *src_px, uint32_t count)
{
   GFX_LFMT_BLOCK_T temp0, temp1, temp2, temp3;

   assert(count == 4 || count == 8);
   blend(&temp0, src_px+0, src_px+1);
   blend(&temp1, src_px+2, src_px+3);
   blend(&temp2, &temp0, &temp1);
   if (count == 4)
   {
      *dst_px = temp2;
   }
   else
   {
      blend(&temp0, src_px+4, src_px+5);
      blend(&temp1, src_px+6, src_px+7);
      blend(&temp3, &temp0, &temp1);
      blend(dst_px, &temp2, &temp3);
   }
}

/* with round to nearest even */
static void decimate_accumulate_and_divide(GFX_LFMT_BLOCK_T *dst_px, GFX_LFMT_BLOCK_T *src_px, uint32_t count)
{
   assert(count == 2 || count == 4 || count == 8); /* sanity check dimensions */

   dst_px->fmt = src_px[0].fmt;

   GFX_LFMT_FMT_DETAIL_T fd;
   gfx_lfmt_fmt_detail(&fd, dst_px->fmt);

   for (uint32_t slot_idx = 0; slot_idx != fd.num_slots; ++slot_idx)
   {
      struct gfx_lfmt_slot_detail *slot = &fd.slts[slot_idx];

      assert(
         slot->type == GFX_LFMT_TYPE_UNORM ||
         slot->type == GFX_LFMT_TYPE_SNORM ||
         slot->type == GFX_LFMT_TYPE_UINT);
      assert(slot->bit_width <= 16); /* 32 bit inputs would overflow acc. */

      int32_t acc = 0;
      for (uint32_t p = 0; p != count; ++p)
      {
         uint32_t src_slot_bits = gfx_lfmt_block_get_slot_bits(&src_px[p].u, &fd, slot_idx);
         if (slot->type == GFX_LFMT_TYPE_SNORM)
         {
            src_slot_bits = gfx_sext(src_slot_bits, slot->bit_width);
         }
         acc += src_slot_bits;
      }

      /* divide by 2/4/8, round to nearest even */
      acc = gfx_srshift_rtne(acc, gfx_log2(count));

      gfx_lfmt_block_set_slot_bits(&dst_px->u, &fd, slot_idx, acc);
   }
}

/* matches TFU mipmap generation */
void gfx_buffer_subsample_func(
   const GFX_BUFFER_BLIT_TGT_FUNC_T *dst,
   const GFX_BUFFER_BLIT_TGT_FUNC_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_TRANSMUTE_T *transmute_options)
{
   assert(dst->desc.num_planes == 1);
   assert(src->desc.num_planes == 1);
   const GFX_BUFFER_DESC_PLANE_T *dst_plane = dst->desc.planes + 0;
   const GFX_BUFFER_DESC_PLANE_T *src_plane = src->desc.planes + 0;

   GFX_LFMT_T fmt = gfx_lfmt_fmt(dst_plane->lfmt);
   assert(fmt == gfx_lfmt_fmt(src_plane->lfmt));
   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, fmt);

   assert(dst->x == 0);
   assert(dst->y == 0);
   assert(dst->z == 0);
   assert(src->x == 0);
   assert(src->y == 0);
   assert(src->z == 0);

   assert(bd.block_w == 1);
   assert(bd.block_h == 1);
   assert(bd.block_d == 1);

   assert(dst->write || gfx_aligned((uint8_t *)dst->p + dst_plane->offset, bd.bytes_per_word));
   assert(src->read || gfx_aligned((uint8_t *)src->p + src_plane->offset, bd.bytes_per_word));

   assert(dst->desc.width == gfx_umax(src->desc.width >> 1, 1));
   assert(dst->desc.height == gfx_umax(src->desc.height >> 1, 1));
   assert(dst->desc.depth == gfx_umax(src->desc.depth >> 1, 1));

   /* calculate upconvert, decimate, and downconvert */
   GFX_BUFFER_XFORM_SEQ_T xform_seq_up;
   void (*decimate)(GFX_LFMT_BLOCK_T*, GFX_LFMT_BLOCK_T*, uint32_t);
   GFX_BUFFER_XFORM_SEQ_T xform_seq_down;
   {
      bool has_srgb = gfx_lfmt_contains_srgb(fmt);
      bool has_norm = gfx_lfmt_contains_unorm(fmt) || gfx_lfmt_contains_snorm(fmt) || has_srgb;
      bool has_float = gfx_lfmt_contains_float(fmt);

      assert(has_norm ^ has_float);
      assert(!(has_float && has_srgb));

      gfx_buffer_xform_seq_init(&xform_seq_up, &src->desc);
      if (has_srgb)
      {
         /* r/g/b16 uint are actually tfu13, and a16 uint is actually a8 unorm.
          * I didn't feel it was worth creating some new enums for these
          * intermediary formats. That's fine for our purposes: we're just
          * doing decimate_accumulate_and_divide on them. */
         GFX_LFMT_T interm_fmt = GFX_LFMT_R16_G16_B16_A16_UINT;
         gfx_buffer_xform_seq_add(&xform_seq_up, gfx_buffer_xform_srgb_to_tfu13, interm_fmt);
         decimate = decimate_accumulate_and_divide;
         gfx_buffer_xform_seq_init_from_fmt(&xform_seq_down, interm_fmt);
         gfx_buffer_xform_seq_add(&xform_seq_down, gfx_buffer_xform_tfu13_to_srgb, fmt);
      }
      else if (has_float)
      {
         GFX_LFMT_T interm_fmt = GFX_LFMT_R16_G16_B16_A16_FLOAT;
         gfx_buffer_xform_seq_construct_continue(&xform_seq_up, 1, &interm_fmt,
            GFX_BUFFER_XFORM_CONVS_REGULAR, transmute_options);
         decimate = decimate_cascade;
         gfx_buffer_xform_seq_construct(&xform_seq_down, 1, &fmt, 1, &interm_fmt,
            GFX_BUFFER_XFORM_CONVS_REGULAR, transmute_options);
      }
      else
      {
         decimate = decimate_accumulate_and_divide;
         gfx_buffer_xform_seq_init_from_fmt(&xform_seq_down, fmt);
      }
   }

   for (uint32_t z = 0; z != dst->desc.depth; ++z) {
      for (uint32_t y = 0; y != dst->desc.height; ++y) {
         for (uint32_t x = 0; x != dst->desc.width; ++x) {
            GFX_LFMT_BLOCK_T in_px[8], blend_px, out_px;
            uint32_t sx0 = x << 1;
            uint32_t sy0 = y << 1;
            uint32_t sz0 = z << 1;
            uint32_t sx1 = gfx_umin(sx0 + 1, src->desc.width - 1);
            uint32_t sy1 = gfx_umin(sy0 + 1, src->desc.height - 1);
            uint32_t sz1 = gfx_umin(sz0 + 1, src->desc.depth - 1);

            size_t dst_block_offset = gfx_buffer_block_offset(dst_plane, &bd,
               x, y, z, dst->desc.height);

            get_px_and_apply_xform_seq(in_px+0, sx0, sy0, sz0, &xform_seq_up, src);
            get_px_and_apply_xform_seq(in_px+1, sx1, sy0, sz0, &xform_seq_up, src);
            get_px_and_apply_xform_seq(in_px+2, sx0, sy1, sz0, &xform_seq_up, src);
            get_px_and_apply_xform_seq(in_px+3, sx1, sy1, sz0, &xform_seq_up, src);
            if (sz0 == sz1)
            {
               decimate(&blend_px, in_px, 4);
            }
            else
            {
               get_px_and_apply_xform_seq(in_px+4, sx0, sy0, sz1, &xform_seq_up, src);
               get_px_and_apply_xform_seq(in_px+5, sx1, sy0, sz1, &xform_seq_up, src);
               get_px_and_apply_xform_seq(in_px+6, sx0, sy1, sz1, &xform_seq_up, src);
               get_px_and_apply_xform_seq(in_px+7, sx1, sy1, sz1, &xform_seq_up, src);
               decimate(&blend_px, in_px, 8);
            }

            gfx_buffer_apply_xform_seq_single_block(&out_px, &blend_px, &xform_seq_down, x, y, z);

            write_block(dst, dst_block_offset, &bd, &out_px.u);
         }
      }
   }
}

static void check_blit_rect_within_buffer(
   const GFX_BUFFER_BLIT_TGT_FUNC_T *bt,
   uint32_t width, uint32_t height, uint32_t depth)
{
   assert(gfx_usum_le(bt->x, width, bt->desc.width));
   assert(gfx_usum_le(bt->y, height, bt->desc.height));
   assert(gfx_usum_le(bt->z, depth, bt->desc.depth));
}

static void check_blit_offset_block_aligned(
   const GFX_BUFFER_BLIT_TGT_FUNC_T *bt,
   const GFX_LFMT_BASE_DETAIL_T *bds)
{
   for (uint32_t p = 0; p != bt->desc.num_planes; ++p)
   {
      assert((bt->x % bds[p].block_w) == 0);
      assert((bt->y % bds[p].block_h) == 0);
      assert((bt->z % bds[p].block_d) == 0);
   }
}

struct blit_range
{
   int32_t begin, end;
   int32_t step;
};

static void init_blit_range(struct blit_range *r, uint32_t size, uint32_t quant, bool reverse)
{
   assert(quant > 0);
   size = gfx_uround_up(size, quant);

   if (reverse)
   {
      r->begin = size - quant;
      r->end = -(int32_t)quant;
      r->step = -(int32_t)quant;
   }
   else
   {
      r->begin = 0;
      r->end = size;
      r->step = quant;
   }
}

/* For p in 0..src->desc.num_planes-1, load an array of blocks starting from
 * (x,y,z) in plane p in src into arrs[p].
 * src block load coordinates will be clamped to (src_w-1,src_h-1,src_d-1)
 * (rounded down to the nearest block).
 * arrs[p].fmt must match gfx_lfmt_fmt(src->desc.planes[p].lfmt). */
static void load_arrs(
   const GFX_LFMT_BLOCK_ARR_T *arrs,
   const GFX_BUFFER_BLIT_TGT_FUNC_T *src,
   uint32_t src_w, uint32_t src_h, uint32_t src_d, /* In elements, must be <= the actual dimensions of src */
   const GFX_LFMT_BASE_DETAIL_T *bds, /* bds[p] should be base detail for arrs[p].fmt */
   uint32_t x, uint32_t y, uint32_t z) /* In elements, must be < (src_w,src_h,src_d) */
{
   src_w += src->x;
   src_h += src->y;
   src_d += src->z;
   assert(src_w <= src->desc.width);
   assert(src_h <= src->desc.height);
   assert(src_d <= src->desc.depth);

   x += src->x;
   y += src->y;
   z += src->z;
   assert(x < src_w);
   assert(y < src_h);
   assert(z < src_d);

   for (uint32_t p = 0; p != src->desc.num_planes; ++p)
   {
      assert(arrs[p].fmt == gfx_lfmt_fmt(src->desc.planes[p].lfmt));

      uint32_t bt_begin_x_in_blocks = gfx_udiv_exactly(x, bds[p].block_w);
      uint32_t bt_begin_y_in_blocks = gfx_udiv_exactly(y, bds[p].block_h);
      uint32_t bt_begin_z_in_blocks = gfx_udiv_exactly(z, bds[p].block_d);
      for (uint32_t z_in_blocks = 0; z_in_blocks != arrs[p].d; ++z_in_blocks)
      {
         uint32_t bt_z_in_blocks = bt_begin_z_in_blocks + z_in_blocks;
         for (uint32_t y_in_blocks = 0; y_in_blocks != arrs[p].h; ++y_in_blocks)
         {
            uint32_t bt_y_in_blocks = bt_begin_y_in_blocks + y_in_blocks;
            for (uint32_t x_in_blocks = 0; x_in_blocks != arrs[p].w; ++x_in_blocks)
            {
               uint32_t bt_x_in_blocks = bt_begin_x_in_blocks + x_in_blocks;

               union gfx_lfmt_block_data *d = gfx_lfmt_block_arr_elem(&arrs[p],
                  x_in_blocks, y_in_blocks, z_in_blocks);
               if ((bt_x_in_blocks * bds[p].block_w) >= src_w)
                  *d = *gfx_lfmt_block_arr_elem(&arrs[p], x_in_blocks - 1, y_in_blocks, z_in_blocks);
               else if ((bt_y_in_blocks * bds[p].block_h) >= src_h)
                  *d = *gfx_lfmt_block_arr_elem(&arrs[p], x_in_blocks, y_in_blocks - 1, z_in_blocks);
               else if ((bt_z_in_blocks * bds[p].block_d) >= src_d)
                  *d = *gfx_lfmt_block_arr_elem(&arrs[p], x_in_blocks, y_in_blocks, z_in_blocks - 1);
               else
               {
                  size_t block_offset = gfx_buffer_block_offset(
                     &src->desc.planes[p], &bds[p],
                     bt_x_in_blocks, bt_y_in_blocks, bt_z_in_blocks,
                     src->desc.height);
                  read_block(d, src, block_offset, &bds[p]);
               }
            }
         }
      }
   }
}

/* For p in 0..dst->desc.num_planes-1, store the array of blocks arrs[p] to
 * plane p in dst starting at (x,y,z).
 * Blocks with coordinates >= (dst_w,dst_h,dst_d) in dst will be skipped.
 * arrs[p].fmt must match gfx_lfmt_fmt(dst->desc.planes[p].lfmt). */
static void store_arrs(
   const GFX_BUFFER_BLIT_TGT_FUNC_T *dst,
   uint32_t dst_w, uint32_t dst_h, uint32_t dst_d, /* In elements, must be <= the actual dimensions of dst */
   const GFX_LFMT_BLOCK_ARR_T *arrs,
   const GFX_LFMT_BASE_DETAIL_T *bds, /* bds[p] should be base detail for arrs[p].fmt */
   uint32_t x, uint32_t y, uint32_t z) /* In elements, must be < (dst_w,dst_h,dst_d) */
{
   dst_w += dst->x;
   dst_h += dst->y;
   dst_d += dst->z;
   assert(dst_w <= dst->desc.width);
   assert(dst_h <= dst->desc.height);
   assert(dst_d <= dst->desc.depth);

   x += dst->x;
   y += dst->y;
   z += dst->z;
   assert(x < dst_w);
   assert(y < dst_h);
   assert(z < dst_d);

   for (uint32_t p = 0; p != dst->desc.num_planes; ++p)
   {
      assert(arrs[p].fmt == gfx_lfmt_fmt(dst->desc.planes[p].lfmt));

      uint32_t bt_begin_x_in_blocks = gfx_udiv_exactly(x, bds[p].block_w);
      uint32_t bt_begin_y_in_blocks = gfx_udiv_exactly(y, bds[p].block_h);
      uint32_t bt_begin_z_in_blocks = gfx_udiv_exactly(z, bds[p].block_d);

      uint32_t bt_end_x_in_blocks = gfx_umin(
         bt_begin_x_in_blocks + arrs[p].w,
         gfx_udiv_round_up(dst_w, bds[p].block_w));
      uint32_t bt_end_y_in_blocks = gfx_umin(
         bt_begin_y_in_blocks + arrs[p].h,
         gfx_udiv_round_up(dst_h, bds[p].block_h));
      uint32_t bt_end_z_in_blocks = gfx_umin(
         bt_begin_z_in_blocks + arrs[p].d,
         gfx_udiv_round_up(dst_d, bds[p].block_d));

      for (uint32_t bt_z_in_blocks = bt_begin_z_in_blocks;
         bt_z_in_blocks != bt_end_z_in_blocks;
         ++bt_z_in_blocks)
      {
         uint32_t z_in_blocks = bt_z_in_blocks - bt_begin_z_in_blocks;
         for (uint32_t bt_y_in_blocks = bt_begin_y_in_blocks;
            bt_y_in_blocks != bt_end_y_in_blocks;
            ++bt_y_in_blocks)
         {
            uint32_t y_in_blocks = bt_y_in_blocks - bt_begin_y_in_blocks;
            for (uint32_t bt_x_in_blocks = bt_begin_x_in_blocks;
               bt_x_in_blocks != bt_end_x_in_blocks;
               ++bt_x_in_blocks)
            {
               uint32_t x_in_blocks = bt_x_in_blocks - bt_begin_x_in_blocks;

               size_t block_offset = gfx_buffer_block_offset(
                  &dst->desc.planes[p], &bds[p],
                  bt_x_in_blocks, bt_y_in_blocks, bt_z_in_blocks,
                  dst->desc.height);
               write_block(dst, block_offset, &bds[p],
                  gfx_lfmt_block_arr_elem(&arrs[p], x_in_blocks, y_in_blocks, z_in_blocks));
            }
         }
      }
   }
}

void gfx_buffer_blit_func(
   const GFX_BUFFER_BLIT_TGT_FUNC_T *dst,
   const GFX_BUFFER_BLIT_TGT_FUNC_T *src,
   const GFX_BUFFER_XFORM_SEQ_T *xform_seq,
   uint32_t width, uint32_t height, uint32_t depth)
{
   if (log_trace_enabled())
   {
      GFX_BUFFER_SPRINT_XFORM_SEQ(seq_desc, xform_seq);
      log_trace("%s: %s", __FUNCTION__, seq_desc);
   }

   check_blit_rect_within_buffer(dst, width, height, depth);
   check_blit_rect_within_buffer(src, width, height, depth);

   GFX_LFMT_BASE_DETAIL_T dst_bds[GFX_BUFFER_MAX_PLANES];
   gfx_buffer_desc_base_details(dst_bds, &dst->desc);
   GFX_LFMT_BASE_DETAIL_T src_bds[GFX_BUFFER_MAX_PLANES];
   gfx_buffer_desc_base_details(src_bds, &src->desc);

   /* Currently cannot handle block-misaligned blits */
   check_blit_offset_block_aligned(dst, dst_bds);
   check_blit_offset_block_aligned(src, src_bds);

   /* Nor blits which only partially cover blocks in dst (note that we do allow
    * partial covering of blocks if the uncovered elements lie outside of the
    * image proper -- in that case the uncovered elements will be overwritten
    * but this doesn't matter as they're undefined anyway) */
   for (uint32_t p = 0; p != dst->desc.num_planes; ++p)
   {
      assert(((width % dst_bds[p].block_w) == 0) || ((dst->x + width) == dst->desc.width));
      assert(((height % dst_bds[p].block_h) == 0) || ((dst->y + height) == dst->desc.height));
      assert(((depth % dst_bds[p].block_d) == 0) || ((dst->z + depth) == dst->desc.depth));
   }

   uint32_t quant_w, quant_h, quant_d;
   gfx_buffer_get_xform_seq_quantum(&quant_w, &quant_h, &quant_d, xform_seq);

   struct blit_range xr, yr, zr;
   init_blit_range(&xr, width, quant_w, dst->x > src->x);
   init_blit_range(&yr, height, quant_h, dst->y > src->y);
   init_blit_range(&zr, depth, quant_d, dst->z > src->z);

   GFX_LFMT_BLOCK_ARR_T dst_arrs[GFX_BUFFER_MAX_PLANES];
   for (uint32_t p = 0; p != dst->desc.num_planes; ++p)
   {
      GFX_LFMT_BLOCK_ARR_ALLOCA(&dst_arrs[p],
         gfx_udiv_exactly(quant_w, dst_bds[p].block_w),
         gfx_udiv_exactly(quant_h, dst_bds[p].block_h),
         gfx_udiv_exactly(quant_d, dst_bds[p].block_d));
      dst_arrs[p].fmt = gfx_lfmt_fmt(dst->desc.planes[p].lfmt);
   }
   GFX_LFMT_BLOCK_ARR_T src_arrs[GFX_BUFFER_MAX_PLANES];
   for (uint32_t p = 0; p != src->desc.num_planes; ++p)
   {
      GFX_LFMT_BLOCK_ARR_ALLOCA(&src_arrs[p],
         gfx_udiv_exactly(quant_w, src_bds[p].block_w),
         gfx_udiv_exactly(quant_h, src_bds[p].block_h),
         gfx_udiv_exactly(quant_d, src_bds[p].block_d));
      src_arrs[p].fmt = gfx_lfmt_fmt(src->desc.planes[p].lfmt);
   }

   for (int32_t z = zr.begin; z != zr.end; z += zr.step)
   {
      for (int32_t y = yr.begin; y != yr.end; y += yr.step)
      {
         for (int32_t x = xr.begin; x != xr.end; x += xr.step)
         {
            load_arrs(src_arrs, src, width, height, depth, src_bds, x, y, z);
            gfx_buffer_apply_xform_seq(dst_arrs, src_arrs, xform_seq, x, y, z);
            store_arrs(dst, width, height, depth, dst_arrs, dst_bds, x, y, z);
         }
      }
   }
}

void gfx_buffer_blit_func_full(
   const GFX_BUFFER_BLIT_TGT_FUNC_T *dst,
   const GFX_BUFFER_BLIT_TGT_FUNC_T *src,
   const GFX_BUFFER_XFORM_SEQ_T *xform_seq)
{
   assert(dst->x == 0);
   assert(dst->y == 0);
   assert(dst->z == 0);
   assert(src->x == 0);
   assert(src->y == 0);
   assert(src->z == 0);
   assert(dst->desc.width == src->desc.width);
   assert(dst->desc.height == src->desc.height);
   assert(dst->desc.depth == src->desc.depth);
   gfx_buffer_blit_func(dst, src, xform_seq,
      dst->desc.width, dst->desc.height, dst->desc.depth);
}

void *gfx_buffer_plane_get_block(
      const GFX_BUFFER_DESC_PLANE_T *src_plane,
      void *src_ptr,
      uint32_t x, uint32_t y, uint32_t z,
      uint32_t height)
{
   GFX_LFMT_BASE_DETAIL_T src_bd;
   uint32_t x_in_blocks, y_in_blocks, z_in_blocks = 0;
   void *src_block = 0;

   gfx_lfmt_base_detail(&src_bd, src_plane->lfmt);

   // Extract block
   gfx_buffer_convert_to_block_coords(&x_in_blocks, &y_in_blocks, &z_in_blocks,
      &src_bd, x, y, z);
   src_block = gfx_buffer_block_p(src_plane, &src_bd, src_ptr, x_in_blocks, y_in_blocks, z_in_blocks, height);

   return src_block;
}

void gfx_buffer_get_pixel_from_block(
   GFX_LFMT_BLOCK_T *px, const GFX_LFMT_BLOCK_T *block,
   /* Block-relative coords */
   uint32_t x, uint32_t y, uint32_t z)
{
   if (gfx_lfmt_is_compressed(block->fmt))
   {
      gfx_buffer_get_pixel_from_compr_block(px, block, x, y, z);
      return;
   }

   if (gfx_lfmt_is_std_with_subsample(block->fmt))
   {
      if (!gfx_lfmt_has_repeated_channel(block->fmt))
      {
         px->fmt = gfx_lfmt_drop_subsample_size(block->fmt);
         assert(gfx_lfmt_block_is_single_elem(px->fmt));
         px->u = block->u;
         return;
      }

      assert((x < 2) && (y == 0) && (z == 0));
      px->fmt = GFX_LFMT_Y8_U8_V8_UNORM;
      switch (block->fmt)
      {
      #define CASE(CHANNELS, Y0, Y1, U, V)                           \
         case GFX_LFMT_C8_C8_C8_C8_2X1_UNORM | GFX_LFMT_CHANNELS_##CHANNELS: \
            px->u.ui8[0] = block->u.ui8[(x == 0) ? (Y0) : (Y1)];     \
            px->u.ui8[1] = block->u.ui8[U];                          \
            px->u.ui8[2] = block->u.ui8[V];                          \
            break;
      CASE(YUYV, 0, 2, 1, 3)
      CASE(VYUY, 3, 1, 2, 0)
      CASE(YYUV, 0, 1, 2, 3)
      CASE(VUYY, 3, 2, 1, 0)
      #undef CASE
      default:
         unreachable();
      }
      return;
   }

   switch (gfx_lfmt_get_base(&block->fmt))
   {
   case GFX_LFMT_BASE_C1:
      assert((x < 8) && (y == 0) && (z == 0));
      px->fmt = block->fmt;
      gfx_lfmt_set_base(&px->fmt, GFX_LFMT_BASE_C1X7);
      px->u.ui8[0] = (block->u.ui8[0] >> x) & 0x1;
      break;
   case GFX_LFMT_BASE_C4:
      assert((x < 2) && (y == 0) && (z == 0));
      px->fmt = block->fmt;
      gfx_lfmt_set_base(&px->fmt, GFX_LFMT_BASE_C4X4);
      px->u.ui8[0] = (block->u.ui8[0] >> (x * 4)) & 0xf;
      break;
   default:
      assert(gfx_lfmt_block_is_single_elem(block->fmt));
      assert((x == 0) && (y == 0) && (z == 0));
      *px = *block;
   }
}

/** Describe offset & contents of an element in a buffer */

size_t gfx_buffer_sprint_elem(char *buf, size_t buf_size, size_t offset,
   const GFX_BUFFER_DESC_T *desc, const void *p, uint32_t x, uint32_t y, uint32_t z,
   bool print_pointer)
{
   assert(desc->num_planes == 1); /* TODO Multi-plane support */
   const GFX_BUFFER_DESC_PLANE_T *plane = &desc->planes[0];

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, plane->lfmt);

   uint32_t x_in_blocks, y_in_blocks, z_in_blocks;
   gfx_buffer_convert_to_block_coords(&x_in_blocks, &y_in_blocks, &z_in_blocks,
      &bd, x, y, z);

   size_t block_offset = gfx_buffer_block_offset(plane, &bd,
      x_in_blocks, y_in_blocks, z_in_blocks, desc->height);
   const void *block_p = (const uint8_t *)p + block_offset;

   GFX_LFMT_BLOCK_T block;
   block.fmt = gfx_lfmt_fmt(plane->lfmt);
   memcpy(&block.u, block_p, bd.bytes_per_block);

   if ((bd.block_w != 1) || (bd.block_h != 1) || (bd.block_d != 1))
   {
      GFX_LFMT_BLOCK_T px;
      gfx_buffer_get_pixel_from_block(&px, &block,
         x % bd.block_w, y % bd.block_h, z % bd.block_d);

      offset = gfx_lfmt_sprint_block(buf, buf_size, offset, &px);
      offset = vcos_safe_sprintf(buf, buf_size, offset,
         "\n(%" PRIu32 ",%" PRIu32 ",%" PRIu32 ") in block (%" PRIu32 ",%" PRIu32 ",%" PRIu32 "):\n",
         x % bd.block_w, y % bd.block_h, z % bd.block_d,
         x_in_blocks, y_in_blocks, z_in_blocks);
   }

   offset = gfx_lfmt_sprint_block(buf, buf_size, offset, &block);
   if (print_pointer)
      offset = vcos_safe_sprintf(buf, buf_size, offset, "\n@%p = %p + %08" PRIxSIZE ":\n",
         block_p, p, block_offset);
   else
      offset = vcos_safe_sprintf(buf, buf_size, offset, "\n@%08" PRIxSIZE ":\n", block_offset);
   offset = gfx_lfmt_sprint_block_raw(buf, buf_size, offset, &block, /*compact=*/false);

   return offset;
}
