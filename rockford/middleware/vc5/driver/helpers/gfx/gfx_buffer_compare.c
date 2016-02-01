/*==============================================================================
 Copyright (c) 2012 Broadcom Europe Limited.
 All rights reserved.

 Module   :  gfx_buffer

 $Id: $

 FILE DESCRIPTION
 Image comparision functions
==============================================================================*/

#include "helpers/gfx/gfx_buffer_compare.h"

#include "helpers/gfx/gfx_util.h"
#include "helpers/gfx/gfx_lfmt.h"
#include "helpers/gfx/gfx_buffer.h"
#include "helpers/gfx/gfx_buffer_raw.h"
#include "helpers/gfx/gfx_lfmt_fmt_detail.h"

/* Clamp threshold to avoid allowing differences that are very large relative
 * to the max slot value (eg a threshold of 1 for a 1 bit slot would mean
 * anything goes) */
static uint32_t clamp_threshold(uint32_t threshold, uint32_t slot_width)
{
   uint32_t max_threshold;
   switch (slot_width)
   {
   case 1:  max_threshold = 0; break;
   case 2:  max_threshold = 1; break;
   default: max_threshold = 1u << (slot_width - 3); break; /* Limit threshold to ~13% of max value */
   }
   return gfx_umin(threshold, max_threshold);
}

/*
Performs comparsion of a pair of slots as specified by delta_desc.

@param *result          Return parameter - pixel representing comparision result
@param *cmp_result      Return parameter - set if slots not equal
@return continue slot comparision - whether slot comparision should continue
 */
static bool compare_slots(GFX_LFMT_BLOCK_T *result,
      gfx_buffer_cmp_result_t *cmp_result, const GFX_LFMT_FMT_DETAIL_T *result_fd, int slot_idx,
      uint32_t slot_width, uint32_t lhs_slot_bits, uint32_t rhs_slot_bits,
      const GFX_BUFFER_DELTA_DESC_T *delta_desc)
{
   switch(delta_desc->delta_type & GFX_BUFFER_DELTA_TYPE_MASK)
   {
      case GFX_BUFFER_DELTA_EQUAL:
         if (lhs_slot_bits != rhs_slot_bits)
         {
            *result = delta_desc->diff_colour;
            *cmp_result = GFX_BUFFER_CMP_RESULT_BEYOND_THRESHOLD;
            return false;
         }
         break;
      case GFX_BUFFER_DELTA_THRESHOLD:
      {
         uint32_t delta_slot_bits = gfx_uabs_diff(lhs_slot_bits, rhs_slot_bits);
         if (delta_slot_bits > 0)
         {
            // TODO percentage threshold or something?
            // What about float types and such?
            uint32_t threshold = clamp_threshold(
               delta_desc->threshold, slot_width);
            if (delta_slot_bits <= threshold)
            {
               *result = delta_desc->threshold_colour;
               *cmp_result = GFX_BUFFER_CMP_RESULT_WITHIN_THRESHOLD;
               return true;
            }
            else
            {
               *result = delta_desc->diff_colour;
               *cmp_result = GFX_BUFFER_CMP_RESULT_BEYOND_THRESHOLD;
               return false;
            }
         }
         break;
      }
      case GFX_BUFFER_DELTA_SUBTRACT:
      {
         uint32_t result_slot_bits = gfx_uabs_diff(lhs_slot_bits, rhs_slot_bits);
         gfx_lfmt_block_set_slot_bits(&result->u, result_fd, slot_idx, result_slot_bits);
         if (result_slot_bits != 0)
            *cmp_result = GFX_BUFFER_CMP_RESULT_BEYOND_THRESHOLD;
         return true;
         break;
      }
      default:
         unreachable();
   }
   return true;
}


static gfx_buffer_cmp_result_t compare_pixels(GFX_LFMT_BLOCK_T *result,
      const GFX_LFMT_BLOCK_T *lhs,
      const GFX_LFMT_BLOCK_T *rhs,
      const GFX_BUFFER_DELTA_DESC_T *delta_desc)
{
   bool ignore_alpha = (GFX_BUFFER_DELTA_NO_ALPHA ==
                       (delta_desc->delta_type & GFX_BUFFER_DELTA_COMPARE_ALPHA_MASK));
   bool strict_channel_order = (GFX_BUFFER_DELTA_CHANNELS_STRICT ==
                       (delta_desc->delta_type & GFX_BUFFER_DELTA_CHANNELS_MASK));

   // Start off with equality, and modify each channel if differences found
   gfx_buffer_cmp_result_t cmp_result = GFX_BUFFER_CMP_RESULT_EQUAL;
   *result = delta_desc->equality_colour;

   // Result format should be the same as lhs when doing delta_subtract
   assert((delta_desc->delta_type != GFX_BUFFER_DELTA_SUBTRACT) || lhs->fmt == result->fmt);

   GFX_LFMT_FMT_DETAIL_T lhs_fd, rhs_fd, result_fd;
   gfx_lfmt_fmt_detail(&lhs_fd, lhs->fmt);
   gfx_lfmt_fmt_detail(&rhs_fd, rhs->fmt);
   gfx_lfmt_fmt_detail(&result_fd, result->fmt);

   for (uint32_t lhs_slot_idx = 0; lhs_slot_idx != lhs_fd.num_slots; ++lhs_slot_idx)
   {
      struct gfx_lfmt_slot_detail *lhs_slot = &lhs_fd.slts[lhs_slot_idx];

      GFX_LFMT_CHANNELS_T ch = lhs_slot->channel;
      if ((ignore_alpha && (ch == GFX_LFMT_CHANNELS_A)) || (ch == GFX_LFMT_CHANNELS_X))
         continue;

      uint32_t rhs_slot_idx = gfx_lfmt_fmt_detail_get_slot_by_channel_maybe(&rhs_fd, ch);
      if ((rhs_slot_idx == GFX_LFMT_FMT_DETAIL_INVALID_SLOT) ||
          (strict_channel_order && (lhs_slot_idx != rhs_slot_idx)))
      {
         *result = delta_desc->diff_colour;
         return GFX_BUFFER_CMP_RESULT_BEYOND_THRESHOLD;
      }
      struct gfx_lfmt_slot_detail *rhs_slot = &rhs_fd.slts[rhs_slot_idx];

      if ((lhs_slot->type != rhs_slot->type) || (lhs_slot->bit_width != rhs_slot->bit_width))
      {
         *result = delta_desc->diff_colour;
         return GFX_BUFFER_CMP_RESULT_BEYOND_THRESHOLD;
      }

      assert(lhs_slot->bit_width <= 32);
      assert(rhs_slot->bit_width <= 32);
      uint32_t lhs_slot_bits = gfx_lfmt_block_get_slot_bits(&lhs->u, &lhs_fd, lhs_slot_idx);
      uint32_t rhs_slot_bits = gfx_lfmt_block_get_slot_bits(&rhs->u, &rhs_fd, rhs_slot_idx);

      if (!compare_slots(result, &cmp_result, &result_fd, lhs_slot_idx,
            lhs_slot->bit_width, lhs_slot_bits, rhs_slot_bits, delta_desc))
      {
         break;
      }
   }
   return cmp_result;
}

static gfx_buffer_cmp_result_t combine_cmp_results(
   gfx_buffer_cmp_result_t a,
   gfx_buffer_cmp_result_t b)
{
   if ((a == GFX_BUFFER_CMP_RESULT_BEYOND_THRESHOLD) ||
      (b == GFX_BUFFER_CMP_RESULT_BEYOND_THRESHOLD))
      return GFX_BUFFER_CMP_RESULT_BEYOND_THRESHOLD;

   if ((a == GFX_BUFFER_CMP_RESULT_WITHIN_THRESHOLD) ||
      (b == GFX_BUFFER_CMP_RESULT_WITHIN_THRESHOLD))
      return GFX_BUFFER_CMP_RESULT_WITHIN_THRESHOLD;

   assert(a == GFX_BUFFER_CMP_RESULT_EQUAL);
   assert(b == GFX_BUFFER_CMP_RESULT_EQUAL);
   return GFX_BUFFER_CMP_RESULT_EQUAL;
}

static gfx_buffer_cmp_result_t gfx_buffer_compare_raw_images_impl(GFX_BUFFER_RAW_T *result,
     const GFX_BUFFER_RAW_T *lhs, const GFX_BUFFER_RAW_T *rhs,
     const GFX_BUFFER_DELTA_DESC_T *delta_desc)
{
   GFX_BUFFER_DESC_PLANE_T *result_plane = &result->descs[0].planes[0];
   GFX_LFMT_BASE_DETAIL_T result_bd;
   gfx_lfmt_base_detail(&result_bd, result_plane->lfmt);
   assert(result_bd.block_w == 1 && result_bd.block_h == 1 && result_bd.block_d == 1);

   gfx_buffer_cmp_result_t cmp_result = GFX_BUFFER_CMP_RESULT_EQUAL;
   for (uint32_t z = 0; z < result->descs[0].depth; z++)
   {
      for (uint32_t y = 0; y < result->descs[0].height; y++)
      {
         for (uint32_t x = 0; x < result->descs[0].width; x++)
         {
            GFX_LFMT_BLOCK_T lhs_px = {0};
            GFX_LFMT_BLOCK_T rhs_px = {0};
            GFX_LFMT_BLOCK_T result_px = {0};
            void *result_block = gfx_buffer_block_p(result_plane, &result_bd, result->p, x, y, z,
               result->descs[0].height);
            gfx_buffer_raw_read_pixel(&lhs_px, lhs, x, y, z, /*mip_level=*/0, /*plane=*/0);
            gfx_buffer_raw_read_pixel(&rhs_px, rhs, x, y, z, /*mip_level=*/0, /*plane=*/0);
            result_px.fmt = result_plane->lfmt;
            cmp_result = combine_cmp_results(cmp_result,
               compare_pixels(&result_px, &lhs_px, &rhs_px, delta_desc));
            memcpy(result_block, &result_px.u, result_bd.bytes_per_block);
         }
      }
   }

   return cmp_result;
}

gfx_buffer_cmp_result_t gfx_buffer_compare_raw_images(GFX_BUFFER_RAW_T *result,
     const GFX_BUFFER_RAW_T *lhs, const GFX_BUFFER_RAW_T *rhs,
     const GFX_BUFFER_DELTA_DESC_T *delta_desc)
{
   assert(result);
   assert(lhs);
   assert(rhs);
   assert(delta_desc);
   assert(delta_desc->equality_colour.fmt == delta_desc->diff_colour.fmt);

   assert(lhs->num_mip_levels == 1); // TODO: support more than one miplevel
   assert(lhs->descs[0].num_planes == 1);

   assert(gfx_lfmt_fmt(result->descs[0].planes[0].lfmt) == delta_desc->equality_colour.fmt);
   assert(result->descs[0].width == lhs->descs[0].width);
   assert(result->descs[0].height == lhs->descs[0].height);
   assert(result->descs[0].depth == lhs->descs[0].depth);


   if ((lhs->descs[0].width != rhs->descs[0].width) ||
       (lhs->descs[0].height != rhs->descs[0].height) ||
       (lhs->descs[0].depth != rhs->descs[0].depth))
   {
      return GFX_BUFFER_CMP_RESULT_BEYOND_THRESHOLD;
   }
   else
   {
      return gfx_buffer_compare_raw_images_impl(result, lhs, rhs, delta_desc);
   }
}
/* End of file */
/*-----------------------------------------------------------------------------*/
