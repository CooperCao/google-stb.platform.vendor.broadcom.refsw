/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "gfx_buffer_translate_v3d.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"

static v3d_memory_format_t translate_memory_format_with_bd(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i,
   unsigned slice,
   const GFX_LFMT_BASE_DETAIL_T *bd)
{
   const GFX_BUFFER_DESC_PLANE_T *p = &desc->planes[plane_i];
   assert((gfx_lfmt_is_2d(p->lfmt) && slice == 0)||
               (gfx_lfmt_is_3d(p->lfmt)));
   switch (gfx_lfmt_get_swizzling(&p->lfmt))
   {
   case GFX_LFMT_SWIZZLING_RSO:      return V3D_MEMORY_FORMAT_RASTER;
   case GFX_LFMT_SWIZZLING_UIF:      return V3D_MEMORY_FORMAT_UIF_NO_XOR;
   case GFX_LFMT_SWIZZLING_UIF_XOR:  return V3D_MEMORY_FORMAT_UIF_XOR;
   case GFX_LFMT_SWIZZLING_LT:
      assert(
         (desc->height <= gfx_lfmt_ut_h_2d(bd)) ||
         (gfx_buffer_lt_width_in_ut(desc, plane_i) == 1));
      return V3D_MEMORY_FORMAT_LINEARTILE;
   case GFX_LFMT_SWIZZLING_UBLINEAR:
      switch (gfx_buffer_ublinear_width_in_ub(desc, plane_i))
      {
      case 1:  return V3D_MEMORY_FORMAT_UBLINEAR_1;
      case 2:  return V3D_MEMORY_FORMAT_UBLINEAR_2;
      default: unreachable(); return V3D_MEMORY_FORMAT_INVALID;
      }
   default:
      unreachable();
      return V3D_MEMORY_FORMAT_INVALID;
   }
}

v3d_memory_format_t gfx_buffer_translate_memory_format(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i, unsigned slice)
{
   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, desc->planes[plane_i].lfmt);
   return translate_memory_format_with_bd(desc, plane_i, slice, &bd);
}

void gfx_buffer_translate_rcfg_color(
   GFX_BUFFER_RCFG_COLOR_TRANSLATION_T *t,
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i,
   unsigned slice,
   uint32_t frame_width, uint32_t frame_height)
{
   const GFX_BUFFER_DESC_PLANE_T *p = &desc->planes[plane_i];
   GFX_LFMT_BASE_DETAIL_T bd;
   bool raster;
   uint32_t raster_padded_width;
   uint32_t uif_height_in_ub;

   assert((gfx_lfmt_is_2d(p->lfmt) && slice==0)||
               (gfx_lfmt_is_3d(p->lfmt)));

   gfx_lfmt_base_detail(&bd, p->lfmt);
   raster = gfx_lfmt_is_rso(p->lfmt);
   raster_padded_width = gfx_buffer_maybe_rso_padded_width(desc, plane_i);
   uif_height_in_ub = gfx_buffer_maybe_uif_height_in_ub(desc, plane_i);

   t->memory_format = translate_memory_format_with_bd(desc, plane_i, slice, &bd);
   t->pixel_format = gfx_lfmt_translate_pixel_format(p->lfmt);
   t->internal_type = v3d_pixel_format_internal_type(t->pixel_format);
   t->internal_bpp = v3d_pixel_format_internal_bpp(t->pixel_format);
   assert(gfx_lfmt_yflip_consistent(p->lfmt));
   t->flipy = !!gfx_lfmt_get_yflip(&p->lfmt);

   t->pad = 0;
   t->addr_offset = 0;

   assert(desc->height >= frame_height);
   if (gfx_lfmt_is_3d(p->lfmt))
      assert(!t->flipy);

   if (t->flipy)
   {
      assert((desc->height % bd.block_h) == 0);
      if (raster)
         /* Must pass address of y=0 row to hardware */
         t->addr_offset = ((desc->height / bd.block_h) - 1) * p->pitch;
      else if (desc->height != frame_height)
         /* Provide desc->height to hardware explicitly (see below) */
         t->pad = 15;
   }

   /* Try to encode pitch in pad field... */
   if (t->pad != 15)
   {
      if (raster)
      {
         assert(raster_padded_width != 0);
         assert(raster_padded_width >= frame_width);
         t->pad = gfx_umin(gfx_msb(gfx_lowest_bit(raster_padded_width)), 14);
         if ((1u << t->pad) <= (raster_padded_width - frame_width))
            t->pad = 15;
         else
            assert(gfx_uround_up_p2(frame_width, 1u << t->pad) ==
               raster_padded_width);
      }
      else if (gfx_lfmt_is_uif_family(p->lfmt))
         t->pad = gfx_umin(15,
            uif_height_in_ub - gfx_udiv_round_up(frame_height, gfx_lfmt_ub_h_2d(
                  &bd, gfx_lfmt_get_swizzling(&p->lfmt))));
   }

   if (t->pad == 15)
   {
      t->clear3_raster_padded_width_or_nonraster_height =
         raster ? raster_padded_width : desc->height;
      t->clear3_uif_height_in_ub = uif_height_in_ub;
   }
   else
   {
      /* These fields should be ignored by hardware -- just stick 0s in */
      t->clear3_raster_padded_width_or_nonraster_height = 0;
      t->clear3_uif_height_in_ub = 0;
   }
}

static v3d_tfu_iformat_t buffer_desc_to_tfu_iformat(const GFX_BUFFER_DESC_T *desc,
      unsigned plane_idx)
{
   v3d_tfu_iformat_t tfu_iformat;

   assert(plane_idx < desc->num_planes);

   switch (gfx_lfmt_get_swizzling(&desc->planes[plane_idx].lfmt))
   {
   case GFX_LFMT_SWIZZLING_UIF_NOUTILE:
      tfu_iformat = V3D_TFU_IFORMAT_UIF_NO_XOR;
      break;
   case GFX_LFMT_SWIZZLING_UIF_NOUTILE_XOR:
      tfu_iformat = V3D_TFU_IFORMAT_UIF_XOR;
      break;
   case GFX_LFMT_SWIZZLING_SAND_128:
      tfu_iformat = V3D_TFU_IFORMAT_SAND_128;
      break;
   case GFX_LFMT_SWIZZLING_SAND_256:
      tfu_iformat = V3D_TFU_IFORMAT_SAND_256;
      break;
   default:
      tfu_iformat = v3d_tfu_iformat_from_memory_format(
         gfx_buffer_translate_memory_format(desc, plane_idx, 0));
      break;
   }

   return tfu_iformat;
}

v3d_tfu_iformat_t gfx_buffer_desc_get_tfu_iformat_and_stride(
   uint32_t *stride,
   GFX_BUFFER_DESC_T const *desc,
   unsigned plane_index
   )
{
   v3d_tfu_iformat_t iformat = buffer_desc_to_tfu_iformat(desc, plane_index);
   switch (iformat)
   {
   case V3D_TFU_IFORMAT_UIF_NO_XOR:
   case V3D_TFU_IFORMAT_UIF_XOR:
      // APB register expects stride (image height) in uif blocks.
      *stride = gfx_buffer_uif_height_in_ub(desc, plane_index);
      break;

   case V3D_TFU_IFORMAT_RASTER:
   case V3D_TFU_IFORMAT_SAND_128:
   case V3D_TFU_IFORMAT_SAND_256:
      {
         GFX_BUFFER_DESC_PLANE_T const* plane = &desc->planes[plane_index];
         GFX_LFMT_BASE_DETAIL_T bd;
         gfx_lfmt_base_detail(&bd, plane->lfmt);

         uint32_t pitch_in_blocks = gfx_udiv_exactly(plane->pitch, bd.bytes_per_block);
         bool vertical_pitch = v3d_tfu_iformat_vertical_pitch(iformat);

         // APB register expects stride in blocks for compressed formats, pixels otherwise.
         // TFU doesn't consider some formats as compressed formats the same way gfx_lfmt does.
         switch (gfx_lfmt_get_base(&plane->lfmt))
         {
         case GFX_LFMT_BASE_C1:
         case GFX_LFMT_BASE_C4:
         case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
         case GFX_LFMT_BASE_C8_C8_2X1:
         case GFX_LFMT_BASE_C8_C8_2X2:
            *stride = pitch_in_blocks * (vertical_pitch ? bd.block_h : bd.block_w);
            break;
         default:
            *stride = pitch_in_blocks;
            break;
         }
      }
      break;
   default:
      *stride = 0;
      break;
   }
   return iformat;
}

v3d_tfu_oformat_t gfx_buffer_desc_get_tfu_oformat_and_height_pad_in_ub(
   uint32_t* o_height_pad_in_ub,
   GFX_BUFFER_DESC_T const* desc,
   unsigned plane_index
   )
{
   v3d_memory_format_t mem_format = gfx_buffer_translate_memory_format(desc, plane_index, 0);
   v3d_tfu_oformat_t oformat = v3d_tfu_oformat_from_memory_format(mem_format);

   uint32_t pad = 0;
   if (oformat == V3D_TFU_OFORMAT_UIF_NO_XOR || oformat == V3D_TFU_OFORMAT_UIF_XOR)
   {
      pad = gfx_buffer_uif_height_pad_in_ub(desc, plane_index);
   }
   *o_height_pad_in_ub = pad;
   return oformat;
}
