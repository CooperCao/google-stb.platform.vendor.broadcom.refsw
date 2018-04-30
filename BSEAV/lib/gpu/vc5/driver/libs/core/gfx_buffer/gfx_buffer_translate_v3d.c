/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "gfx_buffer_translate_v3d.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/v3d/v3d_tlb_decimate.h"

v3d_memory_format_t gfx_buffer_translate_memory_format(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i)
{
   const GFX_BUFFER_DESC_PLANE_T *p = &desc->planes[plane_i];

   switch (gfx_lfmt_get_swizzling(&p->lfmt))
   {
   case GFX_LFMT_SWIZZLING_RSO:     return V3D_MEMORY_FORMAT_RASTER;
   case GFX_LFMT_SWIZZLING_UIF:     return V3D_MEMORY_FORMAT_UIF_NO_XOR;
   case GFX_LFMT_SWIZZLING_UIF_XOR: return V3D_MEMORY_FORMAT_UIF_XOR;
   case GFX_LFMT_SWIZZLING_LT:
   {
#ifndef NDEBUG
      GFX_LFMT_BASE_DETAIL_T bd;
      gfx_lfmt_base_detail(&bd, p->lfmt);
#endif
      assert(
         (desc->height <= gfx_lfmt_ut_h_2d(&bd)) ||
         (gfx_buffer_lt_width_in_ut(desc, plane_i) == 1));
      return V3D_MEMORY_FORMAT_LINEARTILE;
   }
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

void gfx_buffer_translate_tlb_ldst(struct v3d_tlb_ldst_params *ls,
   v3d_addr_t base_addr, const GFX_BUFFER_DESC_T *desc, uint32_t plane_i, uint32_t slice, bool color,
   bool tlb_ms, bool ext_ms, v3d_dither_t dither)
{
   const GFX_BUFFER_DESC_PLANE_T *p = &desc->planes[plane_i];

   assert(slice < desc->depth);
   ls->addr = base_addr + p->offset + (slice * p->slice_pitch);

   ls->memory_format = gfx_buffer_translate_memory_format(desc, plane_i);
#if V3D_VER_AT_LEAST(4,1,34,0)
   gfx_lfmt_translate_pixel_format(p->lfmt, &ls->pixel_format, &ls->chan_reverse, &ls->rb_swap);
   ls->load_alpha_to_one = !gfx_lfmt_has_alpha(p->lfmt);
#else
   if (color)
      ls->output_format.pixel = gfx_lfmt_translate_pixel_format(p->lfmt);
   else if ((p->lfmt & GFX_LFMT_FORMAT_MASK) == GFX_LFMT_S8_UINT)
      ls->output_format.depth = V3D_DEPTH_FORMAT_INVALID;
   else
      ls->output_format.depth = gfx_lfmt_translate_depth_format(p->lfmt);
#endif

   if (ext_ms)
   {
      assert(tlb_ms);
      ls->decimate = V3D_DECIMATE_ALL_SAMPLES;
   }
#if V3D_VER_AT_LEAST(4,1,34,0)
   else if (tlb_ms && color && v3d_pixel_format_supports_4x_decimate(ls->pixel_format))
#else
   else if (tlb_ms && color && v3d_pixel_format_supports_4x_decimate(ls->output_format.pixel))
#endif
      ls->decimate = V3D_DECIMATE_4X;
   else
      ls->decimate = V3D_DECIMATE_SAMPLE0;
   ls->dither = dither;

   ls->stride = gfx_lfmt_is_rso(p->lfmt) ?
#if V3D_VER_AT_LEAST(4,1,34,0)
      p->pitch
#else
      gfx_buffer_rso_padded_width(desc, plane_i)
#endif
      : gfx_buffer_maybe_uif_height_in_ub(desc, plane_i);

   assert(gfx_lfmt_yflip_consistent(p->lfmt));
   ls->flipy = !!gfx_lfmt_get_yflip(&p->lfmt);
   if (ls->flipy)
   {
      GFX_LFMT_BASE_DETAIL_T bd;
      gfx_lfmt_base_detail(&bd, p->lfmt);
      assert((desc->height % bd.block_h) == 0);
      if (gfx_lfmt_is_rso(p->lfmt))
         /* Must pass address of y=0 row to hardware */
         ls->addr += ((desc->height / bd.block_h) - 1) * p->pitch;
   }
   uint32_t w_px;
   v3d_tlb_pixel_from_decimated_coords(&w_px, &ls->flipy_height_px, desc->width, desc->height, tlb_ms ? 4 : 1, ls->decimate);
}

static v3d_tfu_iformat_t buffer_desc_to_tfu_iformat(const GFX_BUFFER_DESC_T *desc,
      unsigned plane_idx)
{
   assert(plane_idx < desc->num_planes);

   GFX_LFMT_SWIZZLING_T swizzling = gfx_lfmt_get_swizzling(&desc->planes[plane_idx].lfmt);
   if (gfx_lfmt_is_sand_family((GFX_LFMT_T)swizzling))
   {
#if !V3D_VER_AT_LEAST(3,3,0,0)
      if (gfx_lfmt_is_bigend_sand_family(desc->planes[plane_idx].lfmt) ||
          gfx_lfmt_dram_map_version(swizzling) != 2)
         return V3D_TFU_IFORMAT_INVALID;
#else
      if (!gfx_lfmt_is_bigend_sand_family(desc->planes[plane_idx].lfmt))
         return V3D_TFU_IFORMAT_INVALID;
#endif

      switch (gfx_lfmt_sandcol_w_in_bytes(swizzling))
      {
      case 128:   return V3D_TFU_IFORMAT_SAND_128;
      case 256:   return V3D_TFU_IFORMAT_SAND_256;
      default:    unreachable(); return V3D_TFU_IFORMAT_INVALID;
      }
   }

   return v3d_tfu_iformat_from_memory_format(
      gfx_buffer_translate_memory_format(desc, plane_idx));
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
   v3d_memory_format_t mem_format = gfx_buffer_translate_memory_format(desc, plane_index);
   v3d_tfu_oformat_t oformat = v3d_tfu_oformat_from_memory_format(mem_format);

   uint32_t pad = 0;
   if (oformat == V3D_TFU_OFORMAT_UIF_NO_XOR || oformat == V3D_TFU_OFORMAT_UIF_XOR)
   {
      pad = gfx_buffer_uif_height_pad_in_ub(desc, plane_index);
   }
   *o_height_pad_in_ub = pad;
   return oformat;
}
