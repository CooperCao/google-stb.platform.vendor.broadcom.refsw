/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_tfu.h"
#include "libs/core/gfx_buffer/gfx_buffer.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"

void v3d_tfu_regs_collect(
   V3D_TFU_COMMAND_T *cmd,
   const V3D_TFU_REGS_T *r)
{
   cmd->interrupt_on_complete = r->i.cfg.ioc;
   cmd->src_channel_order = r->i.cfg.rgbord;
   cmd->flip_y = r->i.cfg.flipy;
   cmd->srgb = r->i.cfg.srgb;
   cmd->num_mip_levels = r->i.cfg.nummm;
   cmd->src_ttype = r->i.cfg.ttype;
   cmd->src_memory_format = r->i.cfg.iformat;
   cmd->dst_pad_in_uif_blocks = r->i.cfg.opad;

   cmd->src_base_addrs[0] = r->i.ia;
   cmd->src_base_addrs[1] = r->i.ca;
   cmd->src_base_addrs[2] = r->i.ua;

   cmd->src_strides[0] = r->i.is.stride0;
   cmd->src_strides[1] = r->i.is.stride1;

   cmd->disable_main_texture_write = r->i.oa.dimtw;
   cmd->dst_memory_format = r->i.oa.oformat;
   cmd->dst_base_addr = r->i.oa.oaddr;

   cmd->width = r->i.os.xsize;
   cmd->height = r->i.os.ysize;

   cmd->yuv_coef.yuv_to_srgb.ay  = r->coef0.ay;
   cmd->yuv_coef.yuv_to_srgb.arc = r->coef0.arc;
   cmd->use_programmable_yuv_coef = r->coef0.usecoef;
   cmd->yuv_coef.yuv_to_srgb.agc = r->coef1.agc;
   cmd->yuv_coef.yuv_to_srgb.abc = r->coef1.abc;
   cmd->yuv_coef.yuv_to_srgb.agr = r->coef2.agr;
   cmd->yuv_coef.yuv_to_srgb.arr = r->coef2.arr;
   cmd->yuv_coef.yuv_to_srgb.abb = r->coef3.abb;
   cmd->yuv_coef.yuv_to_srgb.agb = r->coef3.agb;
}

void v3d_tfu_regs_uncollect(
   V3D_TFU_REGS_T *r,
   const V3D_TFU_COMMAND_T *cmd)
{
   memset(r, 0, sizeof(*r));

   r->i.cfg.ioc = cmd->interrupt_on_complete;
   r->i.cfg.rgbord = cmd->src_channel_order;
   r->i.cfg.flipy = cmd->flip_y;
   r->i.cfg.srgb = cmd->srgb;
   r->i.cfg.nummm = cmd->num_mip_levels;
   r->i.cfg.ttype = cmd->src_ttype;
   r->i.cfg.iformat = cmd->src_memory_format;
   r->i.cfg.opad = cmd->dst_pad_in_uif_blocks;

   r->i.ia = cmd->src_base_addrs[0];
   r->i.ca = cmd->src_base_addrs[1];
   r->i.ua = cmd->src_base_addrs[2];

   r->i.is.stride0 = cmd->src_strides[0];
   r->i.is.stride1 = cmd->src_strides[1];

   r->i.oa.dimtw = cmd->disable_main_texture_write;
   r->i.oa.oformat = cmd->dst_memory_format;
   r->i.oa.oaddr = cmd->dst_base_addr;

   r->i.os.xsize = cmd->width;
   r->i.os.ysize = cmd->height;

   r->coef0.ay  = cmd->yuv_coef.yuv_to_srgb.ay;
   r->coef0.arc = cmd->yuv_coef.yuv_to_srgb.arc;
   r->coef0.usecoef = cmd->use_programmable_yuv_coef;
   r->coef1.agc = cmd->yuv_coef.yuv_to_srgb.agc;
   r->coef1.abc = cmd->yuv_coef.yuv_to_srgb.abc;
   r->coef2.agr = cmd->yuv_coef.yuv_to_srgb.agr;
   r->coef2.arr = cmd->yuv_coef.yuv_to_srgb.arr;
   r->coef3.abb = cmd->yuv_coef.yuv_to_srgb.abb;
   r->coef3.agb = cmd->yuv_coef.yuv_to_srgb.agb;
}

static v3d_tfu_yuv_col_space_t get_tfu_col_space(gfx_buffer_colorimetry_t src_primaries)
{
   // NOTE : Work is planned in Nexus/Magnum to allow us to query the actual VDC matrix given
   // the colorimetry enum. For now, just handle what we can. When the matrix query is
   // available, we need to expose it through the platform interface and use that instead which
   // will involve filling in the matrix coefficients in the V3D_TFU_COMMAND_T.

   // These mapping are based on the magnum mappings in s_aAvcMatrixCoeffs_to_Colorimetry
   switch (src_primaries)
   {
   case GFX_BUFFER_COLORIMETRY_DEFAULT               :
   case GFX_BUFFER_COLORIMETRY_BT_709                :
   case GFX_BUFFER_COLORIMETRY_XVYCC_709             :
   case GFX_BUFFER_COLORIMETRY_UNKNOWN               :
   case GFX_BUFFER_COLORIMETRY_HDMI_FULL_RANGE_YCBCR :
   case GFX_BUFFER_COLORIMETRY_DVI_FULL_RANGE_RGB    : return V3D_TFU_YUV_COL_SPACE_REC709;

   case GFX_BUFFER_COLORIMETRY_SMPTE_170M            :
   case GFX_BUFFER_COLORIMETRY_XVYCC_601             :
   case GFX_BUFFER_COLORIMETRY_BT_470_2_BG           : // Same matrix as BT.601
   case GFX_BUFFER_COLORIMETRY_FCC                   : return V3D_TFU_YUV_COL_SPACE_REC601;

   // We don't currently have direct support for these, so they all assume BT.601
   case GFX_BUFFER_COLORIMETRY_BT_2020_NCL           : // Should be BT.2020
   case GFX_BUFFER_COLORIMETRY_BT_2020_CL            : // Should be BT.2020
   case GFX_BUFFER_COLORIMETRY_SMPTE_240M            : // Should be Smpte240M
   default                                           : return V3D_TFU_YUV_COL_SPACE_REC601;
   }
}

void v3d_tfu_regs_pack_command(
   V3D_TFU_REGS_PACKED_T *pr,
   const V3D_TFU_COMMAND_T *cmd)
{
   V3D_TFU_REGS_T r;
   v3d_tfu_regs_uncollect(&r, cmd);

   pr->su = v3d_pack_tfusu(&r.su);
   pr->icfg = v3d_pack_tfuicfg(&r.i.cfg);
   pr->iia = r.i.ia;
   pr->ica = r.i.ca;
   pr->iua = r.i.ua;
   pr->iis = v3d_pack_tfuiis(&r.i.is);
   pr->ioa = v3d_pack_tfuioa(&r.i.oa);
   pr->ios = v3d_pack_tfuios(&r.i.os);
   pr->coef0 = v3d_pack_tfucoef0(&r.coef0);
   pr->coef1 = v3d_pack_tfucoef1(&r.coef1);
   pr->coef2 = v3d_pack_tfucoef2(&r.coef2);
   pr->coef3 = v3d_pack_tfucoef3(&r.coef3);
}

bool v3d_build_tfu_cmd(V3D_TFU_COMMAND_T *cmd,
      const GFX_BUFFER_DESC_T *src_desc,
      const GFX_BUFFER_DESC_T *dst_desc,
      unsigned num_dst_levels, bool skip_dst_level_0,
      const v3d_addr_t src_base_addr[GFX_BUFFER_MAX_PLANES],
      v3d_addr_t dst_base_addr)
{
   assert(src_desc->num_planes <= 3);
   assert(!skip_dst_level_0 || num_dst_levels > 1);

   /* TFU on HW < 3.3 can't converts more that 2 planes */
   /* There is another code path for YV12               */
#if !V3D_VER_AT_LEAST(3,3,0,0)
   if (src_desc->num_planes > 2)
      return false;
#endif

   if (dst_desc->num_planes != 1)
      return false;

   if (dst_desc->width > V3D_TFU_MAX_WIDTH || dst_desc->height > V3D_TFU_MAX_HEIGHT)
      return false;

   GFX_LFMT_T src_lfmt[3];
   for (unsigned i = 0; i < src_desc->num_planes; i++)
   {
      src_lfmt[i] = src_desc->planes[i].lfmt;
      if (!gfx_lfmt_is_2d(src_lfmt[i]))
         return false;
   }

   if (!gfx_lfmt_is_2d(dst_desc->planes[0].lfmt))
      return false;

   if (gfx_lfmt_is_rso(dst_desc->planes[0].lfmt))
      return false;

   memset(cmd, 0, sizeof(V3D_TFU_COMMAND_T));

   // Get a color space conversion enum
   v3d_tfu_yuv_col_space_t col_space = get_tfu_col_space(src_desc->colorimetry);

   // We only convert to RGB today
   assert(dst_desc->colorimetry == GFX_BUFFER_COLORIMETRY_DEFAULT);

   if (!gfx_lfmt_maybe_translate_to_tfu_type(&cmd->src_ttype, &cmd->srgb,
            &cmd->src_channel_order, src_lfmt, src_desc->num_planes,
            col_space, dst_desc->planes[0].lfmt))
      return false;

   assert(dst_desc->width <= src_desc->width &&  dst_desc->height <= src_desc->height);
   assert(src_desc->num_planes == 1 ||
          (gfx_lfmt_get_yflip(&src_lfmt[0]) == gfx_lfmt_get_yflip(&src_lfmt[1])));

   cmd->flip_y = (gfx_lfmt_get_yflip(&src_lfmt[0]) != gfx_lfmt_get_yflip(&dst_desc->planes[0].lfmt));

   /* tfu flipy supported only for raster input */
   if (cmd->flip_y && !gfx_lfmt_is_rso(src_lfmt[0]))
      return false;

   bool src_adjust_ptr = false;
   if (gfx_lfmt_get_yflip(&src_lfmt[0]) && dst_desc->height != src_desc->height)
   {
      /* we need to adjust the source pointer, and we can do it easily only for
       * raster , one plane, one elem block height images; */
       if (!gfx_lfmt_is_rso(src_lfmt[0]) || src_desc->num_planes != 1)
          return false;;

      GFX_LFMT_BASE_DETAIL_T bd;
      gfx_lfmt_base_detail(&bd, src_lfmt[0]);
      if (bd.block_h != 1)
         return false;

      src_adjust_ptr = true;
   }

   cmd->src_memory_format = gfx_buffer_desc_get_tfu_iformat_and_stride(&cmd->src_strides[0],
                                                                       src_desc, 0);
   if (cmd->src_memory_format == V3D_TFU_IFORMAT_INVALID)
      return false;
   if (cmd->src_strides[0] > V3D_TFU_MAX_STRIDE)
      return false;

   if (src_desc->num_planes > 1)
   {
      v3d_tfu_iformat_t iformat;
      iformat = gfx_buffer_desc_get_tfu_iformat_and_stride(&cmd->src_strides[1], src_desc, 1);
      if (iformat == V3D_TFU_IFORMAT_INVALID)
         return false;
      if (cmd->src_strides[1] > V3D_TFU_MAX_STRIDE)
         return false;

      assert(iformat == cmd->src_memory_format);
   }

   cmd->dst_memory_format = gfx_buffer_desc_get_tfu_oformat_and_height_pad_in_ub(&cmd->dst_pad_in_uif_blocks,
                                                                                 dst_desc, 0);

   cmd->num_mip_levels = num_dst_levels;
   cmd->disable_main_texture_write = skip_dst_level_0;

   cmd->width = dst_desc->width;
   cmd->height = dst_desc->height;

   for (unsigned i = 0; i < src_desc->num_planes; i++)
   {
      // Note: src_base_addr can be NULL when this function is used to check if a conversion
      // is possible, rather than actually doing one.
      cmd->src_base_addrs[i]  = src_base_addr ? src_base_addr[src_desc->planes[i].region] : 0;
      cmd->src_base_addrs[i] += src_desc->planes[i].offset;

      if (src_adjust_ptr)
      {
         assert(gfx_lfmt_is_rso(src_lfmt[i]));
         cmd->src_base_addrs[i] += (src_desc->height - dst_desc->height) *
                                    src_desc->planes[i].pitch;
      }
   }

   cmd->dst_base_addr = dst_base_addr + dst_desc->planes[0].offset;

   return true;
}

/* TODO share with calc_tlb_ldst_pitch? */
static uint32_t calc_pitch(
   GFX_LFMT_T lfmt, v3d_memory_format_t memory_format, uint32_t width, uint32_t height)
{
   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, lfmt);
   switch (memory_format)
   {
   case V3D_MEMORY_FORMAT_RASTER:
      assert(bd.block_w == 1);
      return width * bd.bytes_per_block;
   case V3D_MEMORY_FORMAT_LINEARTILE:
      assert(
         /* In this case, pitch should be 1 utile */
         (width <= (bd.ut_w_in_blocks_2d * bd.block_w)) ||
         /* In this case, pitch is essentially ignored, so 1 utile is fine */
         (height <= (bd.ut_h_in_blocks_2d * bd.block_h)));
      return bd.ut_w_in_blocks_2d * bd.bytes_per_block;
   case V3D_MEMORY_FORMAT_UBLINEAR_1:
      return gfx_lfmt_ub_w_in_blocks_2d(&bd, gfx_lfmt_get_swizzling(&lfmt)) * bd.bytes_per_block;
   case V3D_MEMORY_FORMAT_UBLINEAR_2:
      return 2 * gfx_lfmt_ub_w_in_blocks_2d(&bd, gfx_lfmt_get_swizzling(&lfmt)) * bd.bytes_per_block;
   case V3D_MEMORY_FORMAT_UIF_NO_XOR:
   case V3D_MEMORY_FORMAT_UIF_XOR:
   {
      uint32_t height_in_blocks = gfx_udiv_round_up(height, bd.block_h);
      height_in_blocks = gfx_uround_up_p2(height_in_blocks,
         gfx_lfmt_ub_h_in_blocks_2d(&bd, gfx_lfmt_get_swizzling(&lfmt)));
      return height_in_blocks * bd.bytes_per_block;
   }
   default:
      unreachable();
      return 0;
   }
}

void v3d_tfu_calc_src_desc(
   v3d_addr_t *base_addr, GFX_BUFFER_DESC_T *desc,
   v3d_tfu_yuv_col_space_t *yuv_col_space, // May be NULL
   const V3D_TFU_COMMAND_T *cmd, unsigned dram_map_version)
{
   desc->width = cmd->width;
   desc->height = cmd->height;
   desc->depth = 1;

   switch (v3d_yuv_col_space_from_tfu_type(cmd->src_ttype))
   {
   case V3D_TFU_YUV_COL_SPACE_REC709:
      desc->colorimetry = GFX_BUFFER_COLORIMETRY_BT_709;
      break;
   case V3D_TFU_YUV_COL_SPACE_REC601:
      desc->colorimetry = GFX_BUFFER_COLORIMETRY_XVYCC_601;
      break;
   default:
      desc->colorimetry = GFX_BUFFER_COLORIMETRY_DEFAULT;
      break;
   }

   GFX_LFMT_T layout = gfx_lfmt_translate_from_tfu_iformat(
      cmd->src_memory_format, dram_map_version);

   GFX_LFMT_T fmts[GFX_BUFFER_MAX_PLANES];
   gfx_lfmt_translate_from_tfu_type(fmts, &desc->num_planes, yuv_col_space,
      cmd->src_ttype, cmd->src_channel_order, cmd->srgb, gfx_lfmt_is_bigend_sand_family(layout));

   /* ref: arch spec "Texture Formatting Unit (TFU)": "input formats: [...]
    * Packed YUYV 4:2:2 images in raster format"
    * 3-plane YUV 4:2:0 also always raster (YV12) */
   if (v3d_is_tfu_ttype_yuv(cmd->src_ttype) && ((desc->num_planes == 1) || (desc->num_planes == 3)))
      assert(cmd->src_memory_format == V3D_TFU_IFORMAT_RASTER);

   /* src addrs from APB registers are always to start of the plane's memory
    * (i.e. lowest address).
    *
    * Technically, our choice of base_addr could be arbitrary, as long as
    * offsets are corrected accordingly and all calculations involving offset
    * overflow correctly. I choose base_addr to be the start of the lowest
    * plane to avoid overflow and keep the numbers more sane.
    */
   *base_addr = cmd->src_base_addrs[0];
   for (uint32_t i = 1; i != desc->num_planes; ++i)
      *base_addr = v3d_addr_min(*base_addr, cmd->src_base_addrs[i]);

   for (uint32_t p = 0; p != desc->num_planes; ++p)
   {
      GFX_BUFFER_DESC_PLANE_T *plane = &desc->planes[p];

      GFX_LFMT_BASE_DETAIL_T bd;
      gfx_lfmt_base_detail(&bd, fmts[p]);

      plane->lfmt = gfx_lfmt_set_format(layout, fmts[p]);

      /* src_memory_format from APB reg isn't enough to determine lfmt layout:
         2-plane YUV is NOUTILE */
      if (desc->num_planes == 2)
      {
         switch (cmd->src_memory_format)
         {
         case V3D_TFU_IFORMAT_LINEARTILE:
         case V3D_TFU_IFORMAT_UBLINEAR_1:
         case V3D_TFU_IFORMAT_UBLINEAR_2:
            /* can't see a case where this would happen */
            not_impl();
            break;
         case V3D_TFU_IFORMAT_UIF_NO_XOR:
            plane->lfmt = gfx_lfmt_set_format(GFX_LFMT_2D_UIF_NOUTILE, plane->lfmt);
            break;
         case V3D_TFU_IFORMAT_UIF_XOR:
            plane->lfmt = gfx_lfmt_set_format(GFX_LFMT_2D_UIF_NOUTILE_XOR, plane->lfmt);
            break;
         case V3D_TFU_IFORMAT_RASTER:
         case V3D_TFU_IFORMAT_SAND_128:
         case V3D_TFU_IFORMAT_SAND_256:
            break;
         default: unreachable();
         }
      }

      if (cmd->src_memory_format == V3D_TFU_IFORMAT_RASTER && cmd->flip_y)
      {
         assert(gfx_lfmt_yflip_base_is_nop(gfx_lfmt_get_base(&plane->lfmt)));
         gfx_lfmt_set_yflip(&plane->lfmt, GFX_LFMT_YFLIP_YFLIP);
      }

      assert(v3d_addr_aligned(cmd->src_base_addrs[p],
         gfx_buffer_get_align(plane->lfmt, GFX_BUFFER_ALIGN_MIN)));

      plane->region = 0;
      plane->offset = cmd->src_base_addrs[p] - *base_addr;

      /* 2nd & 3rd planes use same stride field... */
      uint32_t cmd_stride = cmd->src_strides[gfx_umin(p, 1)];
      switch (cmd->src_memory_format)
      {
      case V3D_TFU_IFORMAT_UIF_NO_XOR:
      case V3D_TFU_IFORMAT_UIF_XOR:
         /* APB register gives stride (image height) in uif blocks */
         plane->pitch = cmd_stride * gfx_lfmt_ub_h_in_blocks_2d(&bd,
               gfx_lfmt_get_swizzling(&plane->lfmt)) * bd.bytes_per_block;
         break;
      case V3D_TFU_IFORMAT_RASTER:
      case V3D_TFU_IFORMAT_SAND_128:
      case V3D_TFU_IFORMAT_SAND_256:
      {
         uint32_t pitch_in_blocks;
         bool vertical_pitch = v3d_tfu_iformat_vertical_pitch(cmd->src_memory_format);

         /* APB register gives stride in blocks for compressed formats, pixels otherwise. */
         /* TFU doesn't consider some formats as compressed formats the same
          * way gfx_lfmt does */

         switch (gfx_lfmt_get_base(&plane->lfmt))
         {
         case GFX_LFMT_BASE_C1:
         case GFX_LFMT_BASE_C4:
         case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
         case GFX_LFMT_BASE_C8_C8_2X1:
         case GFX_LFMT_BASE_C8_C8_2X2:
            pitch_in_blocks = gfx_udiv_exactly(cmd_stride,
               vertical_pitch ? bd.block_h : bd.block_w);
            break;
         default:
            pitch_in_blocks = cmd_stride;
         }

         plane->pitch = pitch_in_blocks * bd.bytes_per_block;
         break;
      }
      default:
         /* APB register unnecessary */
         plane->pitch = calc_pitch(plane->lfmt,
            v3d_tfu_iformat_to_memory_format(cmd->src_memory_format),
            cmd->width, cmd->height);
      }

      plane->slice_pitch = 0;
   }
}

void v3d_tfu_calc_dst_descs(
   v3d_addr_t *base_addr, GFX_BUFFER_DESC_T *descs,
   const V3D_TFU_COMMAND_T *cmd)
{
   GFX_LFMT_T fmt = v3d_is_tfu_ttype_yuv(cmd->src_ttype) ?
      GFX_LFMT_R8_G8_B8_A8_SRGB_SRGB_SRGB_UNORM :
      /* This will be the same as the source format but without rgbord applied */
      gfx_lfmt_translate_from_tmu_type((v3d_tmu_type_t)cmd->src_ttype, cmd->srgb);

   {
      /* desc gen just wants dimensionality, not swizzling */
      GFX_LFMT_T fmt_dims = gfx_lfmt_to_2d(fmt);

      size_t buffer_size;
      size_t buffer_align;
      gfx_buffer_desc_gen(descs, &buffer_size, &buffer_align,
         GFX_BUFFER_USAGE_V3D_TEXTURE,
         cmd->width, cmd->height, 1,
         cmd->num_mip_levels,
         1, &fmt_dims);
   }

   if (!cmd->disable_main_texture_write)
   {
      /* Level 0 layout explicitly specified in the APB regs, so override its
       * lfmt and pitch. This means the size and align from desc_gen are now
       * invalid, but we aren't using them anyway. */

      descs[0].planes[0].lfmt =
         gfx_lfmt_set_format(
            gfx_lfmt_translate_from_tfu_oformat(cmd->dst_memory_format),
            descs[0].planes[0].lfmt);

      descs[0].planes[0].pitch = calc_pitch(
         descs[0].planes[0].lfmt,
         v3d_tfu_oformat_to_memory_format(cmd->dst_memory_format),
         cmd->width, cmd->height);

      if (cmd->dst_memory_format == V3D_TFU_OFORMAT_UIF_NO_XOR
         || cmd->dst_memory_format == V3D_TFU_OFORMAT_UIF_XOR)
      {
         GFX_LFMT_BASE_DETAIL_T bd;
         gfx_lfmt_base_detail(&bd, fmt);

         descs[0].planes[0].pitch +=
            cmd->dst_pad_in_uif_blocks *
            gfx_lfmt_ub_h_in_blocks_2d(&bd, gfx_lfmt_get_swizzling(
                  &descs[0].planes[0].lfmt)) *
            bd.bytes_per_block;
      }
   }

   /* Base addr provided to TFU is address of level 0... */
   *base_addr = cmd->dst_base_addr - descs[0].planes[0].offset;
}
