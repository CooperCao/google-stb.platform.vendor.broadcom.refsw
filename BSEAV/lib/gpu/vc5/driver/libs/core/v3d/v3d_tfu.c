/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/

#include "v3d_tfu.h"
#include "libs/core/gfx_buffer/gfx_buffer.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"

void v3d_tfu_regs_collect(
   V3D_TFU_COMMAND_T *cmd,
   const V3D_TFU_REGS_T *r)
{
   cmd->crc_gen = r->su.crc;
   cmd->crc_chain = r->su.crcchain;

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

   r->su.crc = cmd->crc_gen;
   r->su.crcchain = cmd->crc_chain;

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
      v3d_addr_t src_base_addr, v3d_addr_t dst_base_addr)
{
   GFX_LFMT_T src_lfmt[3];

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

   memset(cmd, 0, sizeof(V3D_TFU_COMMAND_T));
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

   if (!gfx_lfmt_maybe_translate_to_tfu_type(&cmd->src_ttype, &cmd->srgb,
            &cmd->src_channel_order, src_lfmt, src_desc->num_planes,
            /* TODO Color space should be specified in src_desc? Currently just
             * assuming Rec. 601... */
            V3D_TFU_YUV_COL_SPACE_REC601,
            dst_desc->planes[0].lfmt))
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
   if (src_desc->num_planes > 1)
   {
      v3d_tfu_iformat_t iformat;
      iformat = gfx_buffer_desc_get_tfu_iformat_and_stride(&cmd->src_strides[1], src_desc, 1);
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
      cmd->src_base_addrs[i] = src_base_addr + src_desc->planes[i].offset;
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
