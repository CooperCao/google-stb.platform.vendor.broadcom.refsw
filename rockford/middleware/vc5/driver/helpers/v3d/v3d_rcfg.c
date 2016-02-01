/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "helpers/v3d/v3d_rcfg.h"
#include "helpers/v3d/v3d_util.h"
#include "helpers/v3d/v3d_tlb_decimate.h"
#include "helpers/v3d/v3d_align.h"
#include "helpers/gfx/gfx_lfmt_translate_v3d.h"
#include "helpers/gfx/gfx_buffer_translate_v3d.h"

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

      rcfg->max_bpp = v3d_translate_from_rt_bpp(common->max_bpp);
      rcfg->num_samp = common->ms_mode ? 4 : 1;
      rcfg->double_buffer = common->double_buffer;
      rcfg->cov_mode = common->cov_mode;

      v3d_tile_size_pixels(
         &rcfg->tile_w_px, &rcfg->tile_h_px,
         rcfg->num_rts, rcfg->double_buffer,
         rcfg->max_bpp, common->ms_mode);

      rcfg->disable_rt_store_mask = common->disable_rt_store_mask;
      rcfg->depth_store = common->depth_store;
      rcfg->stencil_store = common->stencil_store;

      break;
   }
   case V3D_RCFG_TYPE_Z_STENCIL:
   {
      const V3D_CL_TILE_RENDERING_MODE_CFG_Z_STENCIL_T *zs = &i->u.z_stencil;
      struct v3d_ldst_params *ls = &rcfg->depth_default_ldst_params;

      rcfg->depth_type = zs->internal_type;

      ls->addr = zs->addr;
      ls->memory_format = zs->memory_format;
      ls->output_format.depth = zs->output_format;
      ls->decimate = zs->decimate_mode;
      ls->dither = V3D_DITHER_OFF;
      ls->uif_height_in_ub = zs->uif_height_in_ub;
      ls->flipy = false;

      break;
   }
   case V3D_RCFG_TYPE_SEPARATE_STENCIL:
   {
      const V3D_CL_TILE_RENDERING_MODE_CFG_SEPARATE_STENCIL_T *ss = &i->u.separate_stencil;
      struct v3d_ldst_params *ls = &rcfg->separate_stencil_default_ldst_params;

      ls->addr = ss->addr;
      ls->memory_format = ss->memory_format;
      /* This is a bit horrible: use a depth format without stencil, which
       * implies 8bpp separate stencil */
      ls->output_format.depth = V3D_DEPTH_FORMAT_32F;
      ls->decimate = ss->decimate_mode;
      ls->dither = V3D_DITHER_OFF;
      ls->uif_height_in_ub = ss->uif_height_in_ub;
      ls->flipy = false;

      break;
   }
   case V3D_RCFG_TYPE_COLOR:
   {
      const V3D_CL_TILE_RENDERING_MODE_CFG_COLOR_T *color = &i->u.color;
      struct v3d_rt_cfg *r;
      struct v3d_ldst_params *ls;

      /* There is only one valid internal type for each output format.
       * See http://confluence.broadcom.com/x/qwLKB */
      assert(color->internal_type == v3d_pixel_format_internal_type(color->output_format));

      assert(v3d_memory_and_pixel_formats_compatible(
         color->memory_format, color->output_format));

      assert(color->rt < rcfg->num_rts); /* Assume common config will come first... */
      r = &rcfg->rts[color->rt];
      ls = &r->default_ldst_params;

      r->type = color->internal_type;
      r->wps = gfx_udiv_exactly(v3d_translate_from_rt_bpp(color->internal_bpp), 32);
      r->pad = color->pad;

      ls->addr = color->addr;
      ls->memory_format = color->memory_format;
      ls->output_format.pixel = color->output_format;
      ls->decimate = color->decimate_mode;
      ls->dither = color->dither_mode;
      ls->flipy = color->flipy;

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
      struct v3d_rt_cfg *r;

      assert(c_clear1->rt < rcfg->num_rts); /* Assume common config will come first... */
      r = &rcfg->rts[c_clear1->rt];

      r->clear[0] = c_clear1->clear_col_0;
      r->clear[1] = (r->clear[1] & ~0x00ffffff) | c_clear1->clear_col_1_andm24;

      break;
   }
   case V3D_RCFG_TYPE_CLEAR_COLORS_PART2:
   {
      const V3D_CL_TILE_RENDERING_MODE_CFG_CLEAR_COLORS_PART2_T *c_clear2 = &i->u.clear_colors_part2;
      struct v3d_rt_cfg *r;

      assert(c_clear2->rt < rcfg->num_rts); /* Assume common config will come first... */
      r = &rcfg->rts[c_clear2->rt];

      r->clear[1] = (r->clear[1] & ~0xff000000) | (c_clear2->clear_col_1_shift24 << 24);
      r->clear[2] = c_clear2->clear_col_2;
      r->clear[3] = (r->clear[3] & ~0x0000ffff) | c_clear2->clear_col_3_andm16;

      break;
   }
   case V3D_RCFG_TYPE_CLEAR_COLORS_PART3:
   {
      const V3D_CL_TILE_RENDERING_MODE_CFG_CLEAR_COLORS_PART3_T *c_clear3 = &i->u.clear_colors_part3;
      struct v3d_rt_cfg *r;

      assert(c_clear3->rt < rcfg->num_rts); /* Assume common config will come first... */
      r = &rcfg->rts[c_clear3->rt];

      r->clear[3] = (r->clear[3] & ~0xffff0000) | (c_clear3->clear_col_3_shift16 << 16);

      r->clear3_raster_padded_width_or_nonraster_height =
         c_clear3->raster_padded_width_or_nonraster_height;
      r->clear3_uif_height_in_ub = c_clear3->uif_height_in_ub;

      break;
   }
   default:
      unreachable();
   }
}

void v3d_rcfg_finalise(struct v3d_rcfg *rcfg)
{
   uint32_t rt;
   for (rt = 0; rt != rcfg->num_rts; ++rt)
   {
      struct v3d_rt_cfg *r = &rcfg->rts[rt];
      struct v3d_ldst_params *ls = &r->default_ldst_params;

      GFX_LFMT_T lfmt = gfx_lfmt_translate_from_memory_pixel_format_and_flipy(
         ls->memory_format, ls->output_format.pixel, ls->flipy);
      bool raster = gfx_lfmt_is_rso(lfmt);
      GFX_LFMT_BASE_DETAIL_T bd;
      gfx_lfmt_base_detail(&bd, lfmt);

      if ((r->pad == 15) && !raster)
         r->flipy_height_px = r->clear3_raster_padded_width_or_nonraster_height;
      else
         r->flipy_height_px = rcfg->frame_height_px;

      uint32_t frame_width_dec, frame_height_dec;
      v3d_tlb_pixel_to_decimated_coords(
         &frame_width_dec, &frame_height_dec,
         rcfg->frame_width_px, rcfg->frame_height_px,
         rcfg->num_samp, ls->decimate);

      if (raster)
         ls->raster_padded_width_dec = (r->pad == 15) ?
            r->clear3_raster_padded_width_or_nonraster_height :
            gfx_uround_up_p2(frame_width_dec, 1 << r->pad);
      else if (gfx_lfmt_is_uif_family(lfmt))
         ls->uif_height_in_ub = (r->pad == 15) ?
            r->clear3_uif_height_in_ub :
            (gfx_udiv_round_up(frame_height_dec, gfx_lfmt_ub_h_2d(&bd, gfx_lfmt_get_swizzling(&lfmt))) + r->pad);
   }
}

void v3d_rcfg_get_default_ldst_params(
   struct v3d_ldst_params *ls,
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

void v3d_ldst_params_to_raw(
   struct v3d_ldst_params *ls,
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

GFX_LFMT_T v3d_calc_ldst_ext_lfmt(
   v3d_ldst_buf_t buf, const struct v3d_ldst_params *ls)
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

static uint32_t calc_ldst_ext_height_px(
   const struct v3d_rcfg *rcfg, v3d_ldst_buf_t buf,
   const struct v3d_ldst_params *ls)
{
   if (ls->flipy)
   {
      assert(v3d_classify_ldst_buf(buf) == V3D_LDST_BUF_CLASS_COLOR);
      return rcfg->rts[v3d_ldst_buf_rt(buf)].flipy_height_px;
   }
   return rcfg->frame_height_px;
}

static uint32_t calc_ldst_ext_pitch(
   const struct v3d_ldst_params *ls,
   GFX_LFMT_T lfmt, const GFX_LFMT_BASE_DETAIL_T *bd,
   uint32_t width_dec, uint32_t height_dec)
{
   switch (ls->memory_format)
   {
   case V3D_MEMORY_FORMAT_RASTER:
      return gfx_udiv_exactly(ls->raster_padded_width_dec, bd->block_w) * bd->bytes_per_block;
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
      return ls->uif_height_in_ub * gfx_lfmt_ub_h_in_blocks_2d(bd, gfx_lfmt_get_swizzling(&lfmt)) * bd->bytes_per_block;
   default:
      unreachable();
      return 0;
   }
}

void v3d_calc_ldst_buffer_desc(
   v3d_addr_t *base_addr, GFX_BUFFER_DESC_T *desc,
   const struct v3d_rcfg *rcfg, v3d_ldst_buf_t buf,
   const struct v3d_ldst_params *ls)
{
   GFX_LFMT_T lfmt = v3d_calc_ldst_ext_lfmt(buf, ls);
   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, lfmt);
   uint32_t height_px = calc_ldst_ext_height_px(rcfg, buf, ls);

   v3d_tlb_pixel_to_decimated_coords(
      &desc->width, &desc->height,
      rcfg->frame_width_px, height_px,
      rcfg->num_samp, ls->decimate);
   desc->depth = 1;
   desc->num_planes = 1;
   desc->planes[0].lfmt = lfmt;
   desc->planes[0].offset = 0;
   uint32_t pitch = calc_ldst_ext_pitch(ls, lfmt, &bd, desc->width, desc->height);
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
