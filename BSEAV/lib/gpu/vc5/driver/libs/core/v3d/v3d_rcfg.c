/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "v3d_rcfg.h"
#include "v3d_util.h"
#include "v3d_tlb_decimate.h"
#include "v3d_align.h"
#include "v3d_tile_size.h"

#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"

void v3d_rcfg_collect(
   struct v3d_rcfg *rcfg,
   const V3D_CL_TILE_RENDERING_MODE_CFG_T *i)
{
   switch (i->type)
   {
   case V3D_RCFG_TYPE_COMMON:
   {
      const V3D_CL_TILE_RENDERING_MODE_CFG_COMMON_T *common = &i->u.common;

      rcfg->num_rts = common->num_rts;
      assert(rcfg->num_rts <= V3D_MAX_RENDER_TARGETS);

      rcfg->frame_width_px = common->frame_width;
      rcfg->frame_height_px = common->frame_height;

      rcfg->num_samp = common->ms_mode ? 4 : 1;
      rcfg->double_buffer = common->double_buffer;
      rcfg->cov_mode = common->cov_mode;

      v3d_tile_size_pixels(&rcfg->tile_w_px, &rcfg->tile_h_px,
         common->ms_mode, common->double_buffer,
         common->num_rts, common->max_bpp);

#if V3D_VER_AT_LEAST(4,0,2,0)
      rcfg->depth_type = common->internal_depth_type;
      rcfg->early_ds_clear = common->early_ds_clear;
#else
      rcfg->disable_rt_store_mask = common->disable_rt_store_mask;
      rcfg->depth_store = common->depth_store;
      rcfg->stencil_store = common->stencil_store;
#endif

      break;
   }
#if !V3D_VER_AT_LEAST(4,0,2,0)
   case V3D_RCFG_TYPE_Z_STENCIL:
   {
      const V3D_CL_TILE_RENDERING_MODE_CFG_Z_STENCIL_T *zs = &i->u.z_stencil;
      struct v3d_tlb_ldst_params *ls = &rcfg->depth_default_ldst_params;

      rcfg->depth_type = zs->internal_type;

      ls->addr = zs->addr;
      ls->memory_format = zs->memory_format;
      ls->output_format.depth = zs->output_format;
      ls->decimate = zs->decimate_mode;
      ls->dither = V3D_DITHER_OFF;
      ls->stride = zs->uif_height_in_ub;
      ls->flipy = false;
      ls->flipy_height_px = 0; /* Shouldn't be used */

      break;
   }
   case V3D_RCFG_TYPE_SEPARATE_STENCIL:
   {
      const V3D_CL_TILE_RENDERING_MODE_CFG_SEPARATE_STENCIL_T *ss = &i->u.separate_stencil;
      struct v3d_tlb_ldst_params *ls = &rcfg->separate_stencil_default_ldst_params;

      ls->addr = ss->addr;
      ls->memory_format = ss->memory_format;
      /* This is a bit horrible: use a depth format without stencil, which
       * implies 8bpp separate stencil */
      ls->output_format.depth = V3D_DEPTH_FORMAT_32F;
      ls->decimate = ss->decimate_mode;
      ls->dither = V3D_DITHER_OFF;
      ls->stride = ss->uif_height_in_ub;
      ls->flipy = false;
      ls->flipy_height_px = 0; /* Shouldn't be used */

      break;
   }
#endif
   case V3D_RCFG_TYPE_COLOR:
   {
      const V3D_CL_TILE_RENDERING_MODE_CFG_COLOR_T *color = &i->u.color;

#if V3D_VER_AT_LEAST(4,0,2,0)
      for (unsigned i = 0; i != countof(color->rts); ++i)
      {
         rcfg->rts[i].type = color->rts[i].internal_type;
         rcfg->rts[i].wps = gfx_udiv_exactly(v3d_translate_from_rt_bpp(color->rts[i].internal_bpp), 32);
      }
#else
      assert(color->rt < rcfg->num_rts); /* Assume common config will come first... */
      struct v3d_rt_cfg *r = &rcfg->rts[color->rt];
      struct v3d_tlb_ldst_params *ls = &r->default_ldst_params;

      r->type = color->internal_type;
      r->wps = gfx_udiv_exactly(v3d_translate_from_rt_bpp(color->internal_bpp), 32);
      r->pad = color->pad;

      ls->addr = color->addr;
      ls->memory_format = color->memory_format;
      ls->output_format.pixel = color->output_format;
      ls->decimate = color->decimate_mode;
      ls->dither = color->dither_mode;
      ls->flipy = color->flipy;
      /* stride/flipy_height_px will be filled in by v3d_rcfg_finalise() */
#endif

      break;
   }
   case V3D_RCFG_TYPE_ZS_CLEAR_VALUES:
   {
      const V3D_CL_TILE_RENDERING_MODE_CFG_ZS_CLEAR_VALUES_T *zs_clear = &i->u.zs_clear_values;

      rcfg->depth_clear = gfx_float_to_bits(zs_clear->depth_clear);
      rcfg->stencil_clear = (uint8_t)zs_clear->stencil_clear;

      break;
   }
   case V3D_RCFG_TYPE_CLEAR_COLORS_PART1:
   {
      const V3D_CL_TILE_RENDERING_MODE_CFG_CLEAR_COLORS_PART1_T *c_clear1 = &i->u.clear_colors_part1;

      assert(c_clear1->rt < rcfg->num_rts); /* Assume common config will come first... */
      struct v3d_rt_cfg *r = &rcfg->rts[c_clear1->rt];

      r->clear[0] = c_clear1->clear_col_0;
      r->clear[1] = (r->clear[1] & ~0x00ffffff) | c_clear1->clear_col_1_andm24;

      break;
   }
   case V3D_RCFG_TYPE_CLEAR_COLORS_PART2:
   {
      const V3D_CL_TILE_RENDERING_MODE_CFG_CLEAR_COLORS_PART2_T *c_clear2 = &i->u.clear_colors_part2;

      assert(c_clear2->rt < rcfg->num_rts); /* Assume common config will come first... */
      struct v3d_rt_cfg *r = &rcfg->rts[c_clear2->rt];

      r->clear[1] = (r->clear[1] & ~0xff000000) | (c_clear2->clear_col_1_shift24 << 24);
      r->clear[2] = c_clear2->clear_col_2;
      r->clear[3] = (r->clear[3] & ~0x0000ffff) | c_clear2->clear_col_3_andm16;

      break;
   }
   case V3D_RCFG_TYPE_CLEAR_COLORS_PART3:
   {
      const V3D_CL_TILE_RENDERING_MODE_CFG_CLEAR_COLORS_PART3_T *c_clear3 = &i->u.clear_colors_part3;

      assert(c_clear3->rt < rcfg->num_rts); /* Assume common config will come first... */
      struct v3d_rt_cfg *r = &rcfg->rts[c_clear3->rt];

      r->clear[3] = (r->clear[3] & ~0xffff0000) | (c_clear3->clear_col_3_shift16 << 16);

#if !V3D_VER_AT_LEAST(4,0,2,0)
      r->clear3_raster_padded_width_or_nonraster_height =
         c_clear3->raster_padded_width_or_nonraster_height;
      r->clear3_uif_height_in_ub = c_clear3->uif_height_in_ub;
#endif

      break;
   }
   default:
      unreachable();
   }
}

#if !V3D_VER_AT_LEAST(4,0,2,0)
void v3d_rcfg_finalise(struct v3d_rcfg *rcfg)
{
   for (uint32_t rt = 0; rt != rcfg->num_rts; ++rt)
   {
      struct v3d_rt_cfg *r = &rcfg->rts[rt];
      struct v3d_tlb_ldst_params *ls = &r->default_ldst_params;

      GFX_LFMT_T lfmt = gfx_lfmt_translate_from_memory_pixel_format_and_flipy(
         ls->memory_format, ls->output_format.pixel, ls->flipy);
      bool raster = gfx_lfmt_is_rso(lfmt);
      GFX_LFMT_BASE_DETAIL_T bd;
      gfx_lfmt_base_detail(&bd, lfmt);

      if ((r->pad == 15) && !raster)
         ls->flipy_height_px = r->clear3_raster_padded_width_or_nonraster_height;
      else
         ls->flipy_height_px = rcfg->frame_height_px;

      uint32_t frame_width_dec, frame_height_dec;
      v3d_tlb_pixel_to_decimated_coords(
         &frame_width_dec, &frame_height_dec,
         rcfg->frame_width_px, rcfg->frame_height_px,
         rcfg->num_samp, ls->decimate);

      if (raster)
         ls->stride = (r->pad == 15) ?
            r->clear3_raster_padded_width_or_nonraster_height :
            gfx_uround_up_p2(frame_width_dec, 1 << r->pad);
      else if (gfx_lfmt_is_uif_family(lfmt))
         ls->stride = (r->pad == 15) ?
            r->clear3_uif_height_in_ub :
            (gfx_udiv_round_up(frame_height_dec, gfx_lfmt_ub_h_2d(&bd, gfx_lfmt_get_swizzling(&lfmt))) + r->pad);
      else
         ls->stride = 0;
   }
}

void v3d_rcfg_get_default_ldst_params(
   struct v3d_tlb_ldst_params *ls,
   const struct v3d_rcfg *rcfg, v3d_ldst_buf_t buf)
{
   switch (v3d_classify_ldst_buf(buf))
   {
   case V3D_LDST_BUF_CLASS_COLOR:
      *ls = rcfg->rts[v3d_ldst_buf_rt(buf)].default_ldst_params;
      break;
   case V3D_LDST_BUF_CLASS_DEPTH_STENCIL:
      switch (buf)
      {
      case V3D_LDST_BUF_DEPTH:
      case V3D_LDST_BUF_PACKED_DEPTH_STENCIL:
         *ls = rcfg->depth_default_ldst_params;
         break;
      case V3D_LDST_BUF_STENCIL:
         if (v3d_rcfg_use_separate_stencil(rcfg))
            *ls = rcfg->separate_stencil_default_ldst_params;
         else
            *ls = rcfg->depth_default_ldst_params;
         break;
      default:
         unreachable();
      }
      break;
   default:
      unreachable();
   }
}

void v3d_tlb_ldst_params_to_raw(
   struct v3d_tlb_ldst_params *ls,
   const struct v3d_rcfg *rcfg, v3d_ldst_buf_t buf)
{
   switch (v3d_classify_ldst_buf(buf))
   {
   case V3D_LDST_BUF_CLASS_COLOR:
   {
      const struct v3d_rt_cfg *r = &rcfg->rts[v3d_ldst_buf_rt(buf)];
      ls->output_format.pixel = v3d_raw_mode_pixel_format(
         r->type, v3d_translate_rt_bpp(r->wps * 32));
      break;
   }
   case V3D_LDST_BUF_CLASS_DEPTH_STENCIL:
      /* Can't do combined depth/stencil load/store in raw mode because depth
       * is loaded/stored as 32F. You need to do 2 separate loads/stores
       * instead, one with buf=DEPTH and one with buf=STENCIL */
      assert(buf != V3D_LDST_BUF_PACKED_DEPTH_STENCIL);
      ls->output_format.depth = V3D_DEPTH_FORMAT_32F;
      break;
   default:
      unreachable();
   }

   ls->decimate = V3D_DECIMATE_ALL_SAMPLES;

   ls->dither = V3D_DITHER_OFF;
}

static GFX_LFMT_T calc_tlb_ldst_lfmt(
   v3d_ldst_buf_t buf, const struct v3d_tlb_ldst_params *ls)
{
   switch (v3d_classify_ldst_buf(buf))
   {
   case V3D_LDST_BUF_CLASS_COLOR:
      return gfx_lfmt_translate_from_memory_pixel_format_and_flipy(
         ls->memory_format, ls->output_format.pixel, ls->flipy);
   case V3D_LDST_BUF_CLASS_DEPTH_STENCIL:
   {
      GFX_LFMT_T lfmt = gfx_lfmt_translate_from_memory_and_depth_format(
         ls->memory_format, ls->output_format.depth);
      if ((buf == V3D_LDST_BUF_STENCIL) && !gfx_lfmt_has_stencil(lfmt))
         /* Depth/stencil format not containing stencil implies separate
          * stencil, which is always S8 */
         lfmt = gfx_lfmt_set_format(lfmt, GFX_LFMT_S8_UINT);
      assert(!v3d_ldst_do_depth(buf) || gfx_lfmt_has_depth(lfmt));
      assert(!v3d_ldst_do_stencil(buf) || gfx_lfmt_has_stencil(lfmt));
      assert(!ls->flipy);
      return lfmt;
   }
   default:
      unreachable();
      return GFX_LFMT_NONE;
   }
}
#endif

static uint32_t calc_tlb_ldst_pitch(
   const struct v3d_tlb_ldst_params *ls,
   GFX_LFMT_T lfmt, const GFX_LFMT_BASE_DETAIL_T *bd,
   uint32_t width_dec, uint32_t height_dec)
{
   switch (ls->memory_format)
   {
   case V3D_MEMORY_FORMAT_RASTER:
#if V3D_VER_AT_LEAST(4,0,2,0)
      return ls->stride;
#else
      return gfx_udiv_exactly(ls->stride, bd->block_w) * bd->bytes_per_block;
#endif
   case V3D_MEMORY_FORMAT_LINEARTILE:
      assert(
         /* In this case, pitch should be 1 utile */
         (width_dec <= (bd->ut_w_in_blocks_2d * bd->block_w)) ||
         /* In this case, pitch is essentially ignored, so 1 utile is fine */
         (height_dec <= (bd->ut_h_in_blocks_2d * bd->block_h)));
      return bd->ut_w_in_blocks_2d * bd->bytes_per_block;
   case V3D_MEMORY_FORMAT_UBLINEAR_1:
      return gfx_lfmt_ub_w_in_blocks_2d(bd, gfx_lfmt_get_swizzling(&lfmt)) * bd->bytes_per_block;
   case V3D_MEMORY_FORMAT_UBLINEAR_2:
      return 2 * gfx_lfmt_ub_w_in_blocks_2d(bd, gfx_lfmt_get_swizzling(&lfmt)) * bd->bytes_per_block;
   case V3D_MEMORY_FORMAT_UIF_NO_XOR:
   case V3D_MEMORY_FORMAT_UIF_XOR:
      return ls->stride * gfx_lfmt_ub_h_in_blocks_2d(bd, gfx_lfmt_get_swizzling(&lfmt)) * bd->bytes_per_block;
   default:
      unreachable();
      return 0;
   }
}

void v3d_calc_tlb_ldst_buffer_desc(
   v3d_addr_t *base_addr, GFX_BUFFER_DESC_T *desc,
   const struct v3d_rcfg *rcfg, v3d_ldst_buf_t buf,
   const struct v3d_tlb_ldst_params *ls)
{
#if V3D_VER_AT_LEAST(4,0,2,0)
   GFX_LFMT_T lfmt = gfx_lfmt_translate_from_memory_pixel_format_and_flipy(
      ls->memory_format, ls->pixel_format, ls->flipy);

# if V3D_HAS_TLB_SWIZZLE
   if (ls->chan_reverse) lfmt = gfx_lfmt_reverse_channels(lfmt);
   if (ls->rb_swap) {
      switch (gfx_lfmt_get_channels(&lfmt)) {
         case GFX_LFMT_CHANNELS_RGBA: gfx_lfmt_set_channels(&lfmt, GFX_LFMT_CHANNELS_BGRA); break;
         case GFX_LFMT_CHANNELS_RGBX: gfx_lfmt_set_channels(&lfmt, GFX_LFMT_CHANNELS_BGRX); break;
         case GFX_LFMT_CHANNELS_BGRA: gfx_lfmt_set_channels(&lfmt, GFX_LFMT_CHANNELS_RGBA); break;
         case GFX_LFMT_CHANNELS_BGRX: gfx_lfmt_set_channels(&lfmt, GFX_LFMT_CHANNELS_RGBX); break;
         case GFX_LFMT_CHANNELS_ARGB: gfx_lfmt_set_channels(&lfmt, GFX_LFMT_CHANNELS_ABGR); break;
         case GFX_LFMT_CHANNELS_XRGB: gfx_lfmt_set_channels(&lfmt, GFX_LFMT_CHANNELS_XBGR); break;
         case GFX_LFMT_CHANNELS_ABGR: gfx_lfmt_set_channels(&lfmt, GFX_LFMT_CHANNELS_ARGB); break;
         case GFX_LFMT_CHANNELS_XBGR: gfx_lfmt_set_channels(&lfmt, GFX_LFMT_CHANNELS_XRGB); break;
         default: assert(0);     /* Only valid for RGBA/X images */
      }
   }
# endif
#else
   GFX_LFMT_T lfmt = calc_tlb_ldst_lfmt(buf, ls);
#endif
   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, lfmt);

   v3d_tlb_pixel_to_decimated_coords(
      &desc->width, &desc->height,
      rcfg->frame_width_px, ls->flipy ? ls->flipy_height_px : rcfg->frame_height_px,
      rcfg->num_samp, ls->decimate);
   desc->depth = 1;
   desc->num_planes = 1;
   desc->planes[0].lfmt = lfmt;
   desc->planes[0].offset = 0;
   uint32_t pitch = calc_tlb_ldst_pitch(ls, lfmt, &bd, desc->width, desc->height);
   desc->planes[0].pitch = pitch;
   desc->planes[0].slice_pitch = 0;

   assert(ls->addr);
   assert(v3d_addr_aligned(ls->addr, gfx_buffer_get_align(lfmt, GFX_BUFFER_ALIGN_MIN)));
   if (ls->flipy && (ls->memory_format == V3D_MEMORY_FORMAT_RASTER))
      /* Hardware gets passed address of y=0 row */
      *base_addr = ls->addr - ((gfx_udiv_exactly(desc->height, bd.block_h) - 1) * pitch);
   else
      *base_addr = ls->addr;
}
