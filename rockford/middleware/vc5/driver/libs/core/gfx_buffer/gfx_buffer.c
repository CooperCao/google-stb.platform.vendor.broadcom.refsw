/*==============================================================================
 Broadcom Proprietary and Confidential. (c)2012 Broadcom.
 All rights reserved.

 Module   :  gfx_buffer

 $Id: $

 FILE DESCRIPTION
 Implementation of Image Format Library APIs
==============================================================================*/

#include "gfx_buffer.h"

#include <string.h>
#include <stdio.h>
#include "vcos.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "gfx_buffer_uif_config.h"

void gfx_buffer_mip_dims(
   uint32_t *mip_width, uint32_t *mip_height, uint32_t *mip_depth,
   uint32_t width, uint32_t height, uint32_t depth,
   uint32_t mip_level)
{
   *mip_width = gfx_umax(width >> mip_level, 1);
   *mip_height = gfx_umax(height >> mip_level, 1);
   *mip_depth = gfx_umax(depth >> mip_level, 1);
}

void gfx_buffer_desc_rebase(
   uintptr_t *base_addr, GFX_BUFFER_DESC_T *desc)
{
   uint32_t min_offset, i;

   assert(desc->num_planes >= 1);
   min_offset = desc->planes[0].offset;
   for (i = 1; i != desc->num_planes; ++i)
      min_offset = gfx_umin(min_offset, desc->planes[i].offset);

   *base_addr += min_offset;
   for (i = 0; i != desc->num_planes; ++i)
      desc->planes[i].offset -= min_offset;
}

static bool fmts_same_block_size_and_dims(GFX_LFMT_T a, GFX_LFMT_T b)
{
   GFX_LFMT_BASE_DETAIL_T bd_a, bd_b;
   gfx_lfmt_base_detail(&bd_a, a);
   gfx_lfmt_base_detail(&bd_b, b);
   return (bd_a.bytes_per_block == bd_b.bytes_per_block) &&
      (bd_a.block_w == bd_b.block_w) &&
      (bd_a.block_h == bd_b.block_h) &&
      (bd_a.block_d == bd_b.block_d);
}

static bool gfx_buffer_equal_internal(const GFX_BUFFER_DESC_T *lhs,
   const GFX_BUFFER_DESC_T *rhs, bool permit_diff_fmt)
{
   if ((lhs->width != rhs->width) ||
      (lhs->height != rhs->height) ||
      (lhs->depth != rhs->depth) ||
      (lhs->num_planes != rhs->num_planes))
      return false;

   for (uint32_t i = 0; i != lhs->num_planes; ++i)
   {
      GFX_LFMT_T lhs_lfmt = lhs->planes[i].lfmt;
      GFX_LFMT_T rhs_lfmt = rhs->planes[i].lfmt;

      if (permit_diff_fmt)
      {
         if ((lhs_lfmt & ~GFX_LFMT_FORMAT_MASK) != (rhs_lfmt & ~GFX_LFMT_FORMAT_MASK))
            return false;
         if (!fmts_same_block_size_and_dims(lhs_lfmt, rhs_lfmt))
            return false;
      }
      else if (lhs_lfmt != rhs_lfmt)
         return false;

      if (lhs->planes[i].offset != rhs->planes[i].offset)
         return false;

      if ((gfx_lfmt_dims_from_enum(gfx_lfmt_get_dims(&lhs->planes[i].lfmt)) >= 2) &&
         (lhs->planes[i].pitch != rhs->planes[i].pitch))
         return false;

      if ((gfx_lfmt_dims_from_enum(gfx_lfmt_get_dims(&lhs->planes[i].lfmt)) >= 3) &&
         (lhs->planes[i].slice_pitch != rhs->planes[i].slice_pitch))
         return false;
   }

   return true;
}

bool gfx_buffer_equal(const GFX_BUFFER_DESC_T *lhs,
   const GFX_BUFFER_DESC_T *rhs)
{
   return gfx_buffer_equal_internal(lhs, rhs, false);
}

bool gfx_buffer_equal_permit_diff_fmt(const GFX_BUFFER_DESC_T *lhs,
   const GFX_BUFFER_DESC_T *rhs)
{
   return gfx_buffer_equal_internal(lhs, rhs, true);
}

static bool gfx_buffer_equal_with_bases_internal(
   uintptr_t base_addr_lhs, const GFX_BUFFER_DESC_T *lhs_in,
   uintptr_t base_addr_rhs, const GFX_BUFFER_DESC_T *rhs_in,
   bool permit_diff_fmt)
{
   GFX_BUFFER_DESC_T lhs = *lhs_in, rhs = *rhs_in;

   /* Rebase so base addresses and descs can be compared independently */
   gfx_buffer_desc_rebase(&base_addr_lhs, &lhs);
   gfx_buffer_desc_rebase(&base_addr_rhs, &rhs);

   return (base_addr_lhs == base_addr_rhs) &&
      gfx_buffer_equal_internal(&lhs, &rhs, permit_diff_fmt);
}

bool gfx_buffer_equal_with_bases(
   uintptr_t base_addr_lhs, const GFX_BUFFER_DESC_T *lhs,
   uintptr_t base_addr_rhs, const GFX_BUFFER_DESC_T *rhs)
{
   return gfx_buffer_equal_with_bases_internal(
      base_addr_lhs, lhs,
      base_addr_rhs, rhs,
      false);
}

bool gfx_buffer_equal_with_bases_permit_diff_fmt(
   uintptr_t base_addr_lhs, const GFX_BUFFER_DESC_T *lhs,
   uintptr_t base_addr_rhs, const GFX_BUFFER_DESC_T *rhs)
{
   return gfx_buffer_equal_with_bases_internal(
      base_addr_lhs, lhs,
      base_addr_rhs, rhs,
      true);
}

bool gfx_buffer_equal_slice_with_bases_permit_diff_fmt(
   uintptr_t base_addr_2d, const GFX_BUFFER_DESC_T *desc_2d,
   uintptr_t base_addr_3d, const GFX_BUFFER_DESC_T *desc_3d)
{
   assert(gfx_buffer_dims(desc_2d) == 2);
   assert(gfx_buffer_dims(desc_3d) == 3);
   assert(desc_2d->depth == 1);
   assert(desc_3d->depth >= 1);

   if ((desc_2d->width != desc_3d->width) ||
      (desc_2d->height != desc_3d->height) ||
      (desc_2d->num_planes != desc_3d->num_planes))
      return false;

   for (uint32_t i = 0; i != desc_2d->num_planes; ++i)
   {
      const GFX_BUFFER_DESC_PLANE_T *plane_2d = &desc_2d->planes[i];
      const GFX_BUFFER_DESC_PLANE_T *plane_3d = &desc_3d->planes[i];

      if ((plane_2d->lfmt & GFX_LFMT_LAYOUT_MASK) != (plane_3d->lfmt & GFX_LFMT_LAYOUT_MASK))
         return false;
      if (!fmts_same_block_size_and_dims(plane_2d->lfmt, plane_3d->lfmt))
         return false;

      if (plane_2d->pitch != plane_3d->pitch)
         return false;

      uintptr_t addr_2d = base_addr_2d + plane_2d->offset;
      uintptr_t addr_3d = base_addr_3d + plane_3d->offset;
      if (addr_2d < addr_3d)
         return false;
      if (((addr_2d - addr_3d) % plane_3d->slice_pitch) != 0)
         return false;
      uintptr_t z_in_blocks = (addr_2d - addr_3d) / plane_3d->slice_pitch;

      GFX_LFMT_BASE_DETAIL_T bd;
      gfx_lfmt_base_detail(&bd, plane_3d->lfmt);
      if ((z_in_blocks * bd.block_d) >= desc_3d->depth)
         return false;
   }

   return true;
}

uint32_t gfx_buffer_dims(const GFX_BUFFER_DESC_T *desc)
{
   assert(desc->num_planes >= 1);
   uint32_t dims = gfx_lfmt_dims_from_enum(gfx_lfmt_get_dims(&desc->planes[0].lfmt));
   for (uint32_t i = 1; i != desc->num_planes; ++i)
      assert(gfx_lfmt_dims_from_enum(gfx_lfmt_get_dims(&desc->planes[i].lfmt)) == dims);
   return dims;
}

/* Size of plane i, in bytes, excluding most padding at the end of the plane.
 * The size returned by this is guaranteed to be <= the size returned by
 * gfx_buffer_desc_gen. */
uint32_t gfx_buffer_size_plane(const GFX_BUFFER_DESC_T *desc, uint32_t plane_i)
{
   const GFX_BUFFER_DESC_PLANE_T *plane = &desc->planes[plane_i];

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, plane->lfmt);

   GFX_LFMT_SWIZZLING_T swizzling = gfx_lfmt_get_swizzling(&plane->lfmt);

   uint32_t size;
   if (gfx_lfmt_is_uif_family((GFX_LFMT_T)swizzling))
   {
      if (gfx_lfmt_is_uif_xor_family(plane->lfmt))
      {
         // TODO Padding is included here...
         uint32_t padded_width, padded_height;
         gfx_buffer_padded_width_height(&padded_width, &padded_height, desc, plane_i);
         uint32_t num_blocks = gfx_udiv_exactly(padded_height * padded_width, bd.block_h * bd.block_w);
         size = num_blocks * bd.bytes_per_block;
      }
      else
      {
         uint32_t w_in_col = gfx_udiv_round_up(desc->width, gfx_lfmt_ucol_w_2d(&bd, swizzling));
         uint32_t w_in_ub = gfx_udiv_round_up(desc->width, gfx_lfmt_ub_w_2d(&bd, swizzling));
         uint32_t last_col_w_in_ub = w_in_ub - ((w_in_col - 1) * GFX_UIF_COL_W_IN_UB);
         uint32_t h_in_ub = gfx_udiv_round_up(desc->height, gfx_lfmt_ub_h_2d(&bd, swizzling));
         size = (plane->pitch * (w_in_col - 1) * gfx_lfmt_ucol_w_in_blocks_2d(&bd, swizzling)) +
            ((h_in_ub - 1) * GFX_UIF_COL_W_IN_UB * GFX_UIF_UB_SIZE) +
            (last_col_w_in_ub * GFX_UIF_UB_SIZE);
      }
   }
   else if (gfx_lfmt_is_sand_family((GFX_LFMT_T)swizzling))
   {
      assert(gfx_lfmt_is_2d(plane->lfmt));

      /* padded according to sandcol stride */
      uint32_t padded_width, padded_height;
      gfx_buffer_padded_width_height(
         &padded_width, &padded_height,
         desc, plane_i);

      uint32_t last_col_height_padded_to_block_h = gfx_uround_up(desc->height, bd.block_h);

      uint32_t pixels =
         padded_height * padded_width -
         (gfx_lfmt_sandcol_w_2d(&bd, swizzling) *
            (padded_height - last_col_height_padded_to_block_h));

      size = gfx_udiv_exactly(pixels,
         bd.block_h * bd.block_w) * bd.bytes_per_block;

      /* See GFXH-1344 */
      if (gfx_lfmt_is_bigend_sand_family((GFX_LFMT_T)swizzling))
         size = gfx_zround_up(size, 32);
   }
   else switch (swizzling)
   {
   case GFX_LFMT_SWIZZLING_RSO:
      size = gfx_udiv_round_up(desc->width, bd.block_w) * bd.bytes_per_block;
      if (gfx_lfmt_dims_from_enum(gfx_lfmt_get_dims(&plane->lfmt)) >= 2)
         size += plane->pitch * (gfx_udiv_round_up(desc->height, bd.block_h) - 1);
      break;
   case GFX_LFMT_SWIZZLING_LT:
   {
      uint32_t w_in_ut = gfx_udiv_round_up(desc->width, gfx_lfmt_ut_w_2d(&bd));
      uint32_t h_in_ut = gfx_udiv_round_up(desc->height, gfx_lfmt_ut_h_2d(&bd));
      size = (plane->pitch * (h_in_ut - 1) * bd.ut_h_in_blocks_2d) + (w_in_ut * GFX_LFMT_UTILE_SIZE);
      break;
   }
   case GFX_LFMT_SWIZZLING_UBLINEAR:
   {
      uint32_t w_in_ub = gfx_udiv_round_up(desc->width, gfx_lfmt_ub_w_2d(&bd, swizzling));
      uint32_t h_in_ub = gfx_udiv_round_up(desc->height, gfx_lfmt_ub_h_2d(&bd, swizzling));
      size = (plane->pitch * (h_in_ub - 1) * gfx_lfmt_ub_h_in_blocks_2d(&bd, swizzling)) + (w_in_ub * GFX_UIF_UB_SIZE);
      break;
   }
   default:
      unreachable();
   }

   if (gfx_lfmt_is_3d(plane->lfmt))
      size += plane->slice_pitch * (gfx_udiv_round_up(desc->depth, bd.block_d) - 1);

   return size;
}

/* TODO These functions are almost identical... */

uint32_t gfx_buffer_lt_width_in_ut(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i)
{
   const GFX_BUFFER_DESC_PLANE_T *p = &desc->planes[plane_i];
   GFX_LFMT_BASE_DETAIL_T bd;
   uint32_t w_in_ut_min, w_in_ut_from_pitch;

   assert(gfx_lfmt_is_lt(p->lfmt));

   gfx_lfmt_base_detail(&bd, p->lfmt);

   w_in_ut_min = gfx_udiv_round_up(desc->width, gfx_lfmt_ut_w_2d(&bd));
   if (desc->height <= gfx_lfmt_ut_h_2d(&bd)) {
      return w_in_ut_min;
   }

   w_in_ut_from_pitch = gfx_udiv_exactly(p->pitch,
      bd.ut_w_in_blocks_2d * bd.bytes_per_block);
   /* If this fires, it means that p->pitch is too small for the size of the
    * image */
   assert(w_in_ut_from_pitch >= w_in_ut_min);
   return w_in_ut_from_pitch;
}

uint32_t gfx_buffer_ublinear_width_in_ub(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i)
{
   const GFX_BUFFER_DESC_PLANE_T *p = &desc->planes[plane_i];

   assert(gfx_lfmt_is_ublinear(p->lfmt));

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, p->lfmt);

   GFX_LFMT_SWIZZLING_T swizzling = gfx_lfmt_get_swizzling(&p->lfmt);

   uint32_t w_in_ub_min = gfx_udiv_round_up(desc->width, gfx_lfmt_ub_w_2d(&bd, swizzling));
   if (desc->height <= gfx_lfmt_ub_h_2d(&bd, swizzling)) {
      return w_in_ub_min;
   }

   uint32_t w_in_ub_from_pitch = gfx_udiv_exactly(p->pitch,
      gfx_lfmt_ub_w_in_blocks_2d(&bd, swizzling) * bd.bytes_per_block);
   /* If this fires, it means that p->pitch is too small for the size of the
    * image */
   assert(w_in_ub_from_pitch >= w_in_ub_min);
   return w_in_ub_from_pitch;
}

uint32_t gfx_buffer_uif_height_in_ub(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i)
{
   const GFX_BUFFER_DESC_PLANE_T *p = &desc->planes[plane_i];

   assert(gfx_lfmt_is_uif_family(p->lfmt));

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, p->lfmt);

   GFX_LFMT_SWIZZLING_T swizzling = gfx_lfmt_get_swizzling(&p->lfmt);

   uint32_t h_in_ub_min = gfx_udiv_round_up(desc->height, gfx_lfmt_ub_h_2d(&bd, swizzling));
   if (desc->width <= gfx_lfmt_ucol_w_2d(&bd, swizzling))
      return h_in_ub_min;

   uint32_t h_in_ub_from_pitch = gfx_udiv_exactly(p->pitch,
      gfx_lfmt_ub_h_in_blocks_2d(&bd, swizzling) * bd.bytes_per_block);
   /* If this fires, it means that p->pitch is too small for the size of the
    * image */
   assert(h_in_ub_from_pitch >= h_in_ub_min);

   if (gfx_lfmt_is_uif_xor_family(p->lfmt))
   {
      uint32_t b = gfx_msb(GFX_UIF_XOR_ADDR) + 1;
      /* XORing will swap things around within blocks of height 2^b. We require
       * that the buffer height is a multiple of this, so that XORing cannot
       * give addresses outside of the buffer. */
      assert(!(h_in_ub_from_pitch & gfx_mask(b)));
      vcos_unused_in_release(b);
   }

   return h_in_ub_from_pitch;
}

uint32_t gfx_buffer_uif_height_pad_in_ub(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i)
{
   const GFX_BUFFER_DESC_PLANE_T *plane = &desc->planes[plane_i];

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, plane->lfmt);

   GFX_LFMT_SWIZZLING_T swizzling = gfx_lfmt_get_swizzling(&plane->lfmt);

   uint32_t h_in_ub_min = gfx_udiv_round_up(desc->height, gfx_lfmt_ub_h_2d(&bd, swizzling));

   uint32_t h_in_ub = gfx_buffer_uif_height_in_ub(desc, plane_i);

   return h_in_ub - h_in_ub_min;
}

uint32_t gfx_buffer_rso_padded_width(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i)
{
   const GFX_BUFFER_DESC_PLANE_T *p = &desc->planes[plane_i];
   GFX_LFMT_BASE_DETAIL_T bd;
   uint32_t w_block_pad, w_from_pitch;

   assert(gfx_lfmt_is_rso(p->lfmt));

   gfx_lfmt_base_detail(&bd, p->lfmt);

   w_block_pad = gfx_uround_up(desc->width, bd.block_w);
   if (desc->height <= bd.block_h) {
      return w_block_pad;
   }

   w_from_pitch = gfx_udiv_exactly(p->pitch, bd.bytes_per_block) * bd.block_w;
   /* Note that we allow w_from_pitch < w_block_pad. This is possible with VG
    * texture upload data. */
   return gfx_umax(w_block_pad, w_from_pitch);
}

uint32_t gfx_buffer_sand_padded_height(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i)
{
   const GFX_BUFFER_DESC_PLANE_T *p = &desc->planes[plane_i];
   GFX_LFMT_BASE_DETAIL_T bd;
   uint32_t h_from_pitch;

   assert(gfx_lfmt_is_sand_family(p->lfmt));

   gfx_lfmt_base_detail(&bd, p->lfmt);

   assert(p->pitch > 0);
   h_from_pitch = gfx_udiv_exactly(p->pitch, bd.bytes_per_block) * bd.block_h;
   assert(h_from_pitch >= desc->height);
   return h_from_pitch;
}

/* TODO Should derive padding from slice_pitch for 3D? */
void gfx_buffer_padded_width_height(
   uint32_t *width, uint32_t *height,
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i)
{
   const GFX_BUFFER_DESC_PLANE_T *plane = &desc->planes[plane_i];

   assert(gfx_lfmt_dims_from_enum(gfx_lfmt_get_dims(&plane->lfmt)) >= 2);

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, plane->lfmt);

   GFX_LFMT_SWIZZLING_T swizzling = gfx_lfmt_get_swizzling(&plane->lfmt);

   if (gfx_lfmt_is_uif_family((GFX_LFMT_T)swizzling))
   {
      *width = gfx_uround_up(desc->width, gfx_lfmt_ucol_w_2d(&bd, swizzling));
      *height = gfx_buffer_uif_height_in_ub(desc, plane_i) * gfx_lfmt_ub_h_2d(&bd, swizzling);
   }
   else if (gfx_lfmt_is_sand_family((GFX_LFMT_T)swizzling))
   {
      *width = gfx_uround_up(desc->width, gfx_lfmt_sandcol_w_2d(&bd, swizzling));
      *height = gfx_buffer_sand_padded_height(desc, plane_i);
   }
   else switch (swizzling)
   {
   case GFX_LFMT_SWIZZLING_RSO:
      *width = gfx_buffer_rso_padded_width(desc, plane_i);
      *height = gfx_uround_up(desc->height, bd.block_h);
      break;
   case GFX_LFMT_SWIZZLING_LT:
      *width = gfx_buffer_lt_width_in_ut(desc, plane_i) * gfx_lfmt_ut_w_2d(&bd);
      *height = gfx_uround_up(desc->height, gfx_lfmt_ut_h_2d(&bd));
      break;
   case GFX_LFMT_SWIZZLING_UBLINEAR:
      *width = gfx_buffer_ublinear_width_in_ub(desc, plane_i) * gfx_lfmt_ub_w_2d(&bd, swizzling);
      *height = gfx_uround_up(desc->height, gfx_lfmt_ub_h_2d(&bd, swizzling));
      break;
   default:
      unreachable();
   }
}

size_t gfx_buffer_sprint_desc(char *buf, size_t buf_size, size_t offset,
   const GFX_BUFFER_DESC_T *desc)
{
   offset = vcos_safe_sprintf(buf, buf_size, offset, "%" PRIu32, desc->width);
   if (gfx_lfmt_dims_from_enum(gfx_lfmt_get_dims(&desc->planes[0].lfmt)) >= 2)
      offset = vcos_safe_sprintf(buf, buf_size, offset, " x %" PRIu32, desc->height);
   else
      assert(desc->height == 1);
   if (gfx_lfmt_dims_from_enum(gfx_lfmt_get_dims(&desc->planes[0].lfmt)) >= 3)
      offset = vcos_safe_sprintf(buf, buf_size, offset, " x %" PRIu32, desc->depth);
   else
      assert(desc->depth == 1);

   if (desc->num_planes != 1)
      offset = vcos_safe_sprintf(buf, buf_size, offset, ", %" PRIu32 " planes:", desc->num_planes);

   for (uint32_t i = 0; i != desc->num_planes; ++i)
   {
      offset = vcos_safe_sprintf(buf, buf_size, offset, " ");
      offset = gfx_lfmt_sprint(buf, buf_size, offset, desc->planes[i].lfmt);
   }

   return offset;
}

const char *gfx_buffer_desc(const GFX_BUFFER_DESC_T *desc)
{
   static GFX_BUFFER_SPRINT_DESC(str, desc);
   return str;
}

size_t gfx_buffer_get_align(GFX_LFMT_T lfmt, gfx_buffer_align_t a)
{
   assert((a == GFX_BUFFER_ALIGN_MIN) || (a == GFX_BUFFER_ALIGN_RECOMMENDED));

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, lfmt);

   size_t align = bd.bytes_per_word;

   GFX_LFMT_SWIZZLING_T swizzling = gfx_lfmt_get_swizzling(&lfmt);
   if (gfx_lfmt_is_uif_family((GFX_LFMT_T)swizzling))
   {
      align = gfx_zmax(align,
         ((a == GFX_BUFFER_ALIGN_RECOMMENDED) && gfx_lfmt_is_uif_xor_family(lfmt)) ?
         GFX_UIF_PAGE_SIZE : GFX_UIF_UB_SIZE);
   }
   else if (gfx_lfmt_is_sand_family((GFX_LFMT_T)swizzling))
   {
      switch (gfx_lfmt_dram_map_version(swizzling))
      {
      case 2:
         break;
      case 5:
      case 8:
         align = gfx_zmax(align, 4 * gfx_lfmt_sandcol_w_in_bytes(swizzling));
         break;
      default:
         unreachable();
      }

      /* See GFXH-1344 */
      if (gfx_lfmt_is_bigend_sand_family((GFX_LFMT_T)swizzling))
         align = gfx_zmax(align, 32);
   }
   else switch (swizzling)
   {
   case GFX_LFMT_SWIZZLING_LT:
      align = gfx_zmax(align, GFX_LFMT_UTILE_SIZE);
      break;
   case GFX_LFMT_SWIZZLING_UBLINEAR:
      align = gfx_zmax(align, GFX_UIF_UB_SIZE);
      break;
   default:
      break;
   }

   if (gfx_lfmt_is_bstc_family(lfmt))
      align = gfx_zmax(align, 32);

   return align;
}
