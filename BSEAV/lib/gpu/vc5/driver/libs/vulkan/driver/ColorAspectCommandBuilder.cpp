/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "ColorAspectCommandBuilder.h"
#include "CommandBuffer.h"
#include "Command.h"
#include "Options.h"
#include "Image.h"

#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/v3d/v3d_gen.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/core/v3d/v3d_tile_size.h"
#include "libs/core/gfx_buffer/gfx_buffer_uif_config.h"

namespace bvk {

LOG_DEFAULT_CAT("bvk::ColorAspectCommandBuilder");

VkOffset2D ColorAspectCommandBuilder::GetImageOffsetAlignments(
      const GFX_BUFFER_DESC_T *desc)
{
   VkOffset2D align {0, 0};
   GFX_LFMT_T lfmt = desc->planes[0].lfmt;

   // This query is only valid for uncompressed images
   if (gfx_lfmt_is_compressed(lfmt))
      return align;

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, lfmt);

   if (gfx_lfmt_is_rso(lfmt))
   {
      align.y = 1;

      // Keep a minimum 4byte address alignment.
      switch (bd.bytes_per_block)
      {
         case 1:  align.x = 4; break;
         case 2:  align.x = 2; break;
         default: align.x = 1; break;
      }
   }
   else if (gfx_lfmt_is_uif_family(lfmt))
   {
      // With no XOR we can offset horizontally by the column width and
      // vertically by the UIF block height.
      align.x = gfx_lfmt_ucol_w_2d(&bd, gfx_lfmt_get_swizzling(&lfmt));
      align.y = gfx_lfmt_ub_h_2d(&bd, gfx_lfmt_get_swizzling(&lfmt));

      if (gfx_lfmt_is_uif_xor(lfmt))
      {
         // XOR effects the odd columns so only allow offsets starting on
         // the even column otherwise the wrong column will be XOR'd.
         align.x *= 2;

         // We can offset vertically by the size of the repeating XOR
         // address pattern for this image format. We work this out using the
         // same basic logic used to validate the pitch padding when creating
         // the LD/ST config for the TLB, found in:
         //    gfx_buffer.c: gfx_buffer_uif_height_in_ub()
         const uint32_t b = gfx_msb(GFX_UIF_XOR_ADDR) + 1;

         // XORing will swap things around within regions of height 2^b UIF blocks
         const uint32_t xorUIFBlockAlign = 1 << b;

         // Now turn that into raster lines, align.y already contains the
         // UIF block height in lines.
         align.y *= xorUIFBlockAlign;
      }
   }
   // TODO: LT and UBLINEAR for long thin images
   return align;
}


uint32_t ColorAspectCommandBuilder::OffsetToBytes(
      GFX_BUFFER_DESC_T *desc,
      VkOffset2D         offset,
      bool               ms)
{
   if (offset.x == 0 && offset.y == 0)
      return 0; // Nothing to do.

   if (ms)
   {
      // The understanding is the MS images are organised by doubling each
      // dimension. So this should translate to the correct position and if
      // the original offset is supported then doubling the x and y should also
      // be supported as the alignments are powers of two.
      offset.x *= 2;
      offset.y *= 2;
   }

#ifndef NDEBUG
   VkOffset2D offsetAlignments = GetImageOffsetAlignments(desc);
   assert(offsetAlignments.x != 0 && offsetAlignments.y != 0);
   assert((offset.x % offsetAlignments.x) == 0 && (offset.y % offsetAlignments.y) == 0);
#endif

   GFX_LFMT_T lfmt = desc->planes[0].lfmt;

   assert (!gfx_lfmt_is_compressed(lfmt));

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, lfmt);

   uint32_t byteOffset;
   if (gfx_lfmt_is_rso(lfmt))
   {
      byteOffset  = offset.y * desc->planes[0].pitch;
      byteOffset += offset.x * bd.bytes_per_block;
   }
   else if (gfx_lfmt_is_uif_family(lfmt))
   {
      // Note: This is deliberately over commented and split into individual
      //       steps to aid readers who are not that familiar with UIF memory
      //       layout.

      // offset.x must be a multiple of the UIF column width in pixels.
      //
      // This looks odd, but the pitch in this case is based on the _height_ of
      // the image (padded to UIF block multiples). So pitch * columnwidth in
      // pixels moves you in the x axis by one column.
      byteOffset = offset.x * desc->planes[0].pitch;

      // offset.y must be a multiple of the block size and XOR padding
      // size (if an XOR format).
      GFX_LFMT_SWIZZLING_T swizzling = gfx_lfmt_get_swizzling(&lfmt);
      const uint32_t yInUIFBlocks = offset.y / gfx_lfmt_ub_h_2d(&bd, swizzling);

      // The total number of UIF blocks we want to shift the address offset by
      // is the number of blocks in the y direction multiplied by the number
      // of blocks in a column.
      const uint32_t totalBlocks = yInUIFBlocks * GFX_UIF_COL_W_IN_UB;

      // Finally adjust the offset by the number of bytes in each block
      byteOffset += totalBlocks * GFX_UIF_UB_SIZE;
   }
   else
   {
      // TODO: LT and UBLINEAR
      unreachable();
   }
   return byteOffset;
}

void ColorAspectCommandBuilder::SetImageSubresources(
      Image *srcImage, Image *dstImage, GFX_LFMT_T altTLBFmt,
      uint32_t srcMipLevel, uint32_t dstMipLevel,
      uint32_t srcBaseLayer, uint32_t dstBaseLayer,
      uint32_t layerCount, uint32_t fbWidth, uint32_t fbHeight)
{
   assert(layerCount <= V3D_MAX_RENDER_TARGETS);
   m_numRenderTargets = layerCount;

   m_srcMS = srcImage && srcImage->Samples() != VK_SAMPLE_COUNT_1_BIT;
   m_dstMS = dstImage->Samples() != VK_SAMPLE_COUNT_1_BIT;

   assert(dstImage != nullptr);
   if (srcImage != nullptr)
   {
      ImageParams(m_loadTLBParams, m_loadOffset, srcImage, altTLBFmt, srcMipLevel, srcBaseLayer);
      m_needsLoads = true;
   }

   ImageParams(m_storeTLBParams, m_storeOffset, dstImage, altTLBFmt, dstMipLevel, dstBaseLayer);

   SetupFrameConfig(dstImage, altTLBFmt, dstMipLevel, fbWidth, fbHeight);
}

void ColorAspectCommandBuilder::SetDstTLBParamsDirect(
      const v3d_tlb_ldst_params &params, bool isMultisampled, uint32_t width, uint32_t height)
{
   m_storeTLBParams[0] = params;
   m_numRenderTargets  = 1;
   m_dstMS             = isMultisampled;

   v3d_pixel_format_to_rt_format(&m_v3dRtFormat, params.pixel_format);

   SetupFrameConfig(m_dstMS ? (width  / 2) : width,
                    m_dstMS ? (height / 2) : height);
}

void ColorAspectCommandBuilder::ImageParams(
      v3d_tlb_ldst_params *ldstParams, VkOffset2D imgOffset,
      Image *img, GFX_LFMT_T altTLBFmt,
      uint32_t mipLevel, uint32_t baseLayer)
{
   log_trace("ImageParams:\n\timage = %p mipLevel = %u baseLayer = %u",
         img, (unsigned)mipLevel, (unsigned)baseLayer);

   const v3d_addr_t basePhys = img->PhysAddr();
   bool ms = (img->Samples() != VK_SAMPLE_COUNT_1_BIT);

   log_trace("\tbasePhys= %#x", (unsigned)basePhys);

   GFX_BUFFER_DESC_T desc = img->GetDescriptor(mipLevel);
   Image::AdjustDescForTLBLfmt(&desc, altTLBFmt, ms);
   uint32_t byteOffset = OffsetToBytes(&desc, imgOffset, ms);

   bool is3D = gfx_lfmt_is_3d(desc.planes[0].lfmt);

   for(uint32_t i = 0; i < m_numRenderTargets; i++)
   {
      uint32_t slice = is3D ? baseLayer + i : 0;
      uint32_t layer = is3D ? 0 : baseLayer + i;

      uint32_t offset = img->LayerOffset(layer) + byteOffset;

      log_trace("\tTarget%u: %s = %u offset = %u",
            i, is3D ? "slice" : "layer", (unsigned)(baseLayer + i), (unsigned)offset + desc.planes[0].offset);

      bool msTLB = m_srcMS || m_dstMS;
      gfx_buffer_translate_tlb_ldst(
            &ldstParams[i], basePhys + offset, &desc, /*plane_i=*/0, slice,
            /* color= */ true , msTLB, /*ext_ms=*/ms, V3D_DITHER_OFF);
   }
}

void ColorAspectCommandBuilder::SetupFrameConfig(uint32_t pixX, uint32_t pixY)
{
   bool msTLB = m_srcMS || m_dstMS;
   uint32_t tileWidth, tileHeight;
   v3d_tile_size_pixels(&tileWidth, &tileHeight, msTLB,
      /*double_buffer=*/false, m_numRenderTargets, m_v3dRtFormat.bpp);

   m_numPixelsX = pixX;
   m_numPixelsY = pixY;

   // Currently the physical device max image dimensions are set to these
   // limits, but that will probably change to support larger textures at some
   // point.
   //
   // If you have hit this assert it probably means you have a lot more work to
   // do getting all of the transfer operations that need the TLB to work with
   // big (particularly non-RSO) images.
   assert(m_numPixelsX <= V3D_MAX_CLIP_WIDTH && m_numPixelsY <= V3D_MAX_CLIP_HEIGHT);

   m_numTilesX = gfx_udiv_round_up(m_numPixelsX, tileWidth);
   m_numTilesY = gfx_udiv_round_up(m_numPixelsY, tileHeight);

   log_trace("\tfbsize (%ux%u) tileSize (%ux%u) tiles (%ux%u)",
      (unsigned)m_numPixelsX, (unsigned)m_numPixelsY,
      (unsigned)tileWidth, (unsigned)tileHeight,
      (unsigned)m_numTilesX, (unsigned)m_numTilesY);
}

void ColorAspectCommandBuilder::SetupFrameConfig(Image *img, GFX_LFMT_T altTLBFmt, uint32_t mipLevel,
                                                 uint32_t fbWidth, uint32_t fbHeight)
{
   GFX_BUFFER_DESC_T desc = img->GetDescriptor(mipLevel);
   Image::AdjustDescForTLBLfmt(&desc, altTLBFmt, m_dstMS);

   // Construct some generic information needed by the control list generation
   // based on the first layer descriptor.
   gfx_lfmt_translate_rt_format(&m_v3dRtFormat, desc.planes[0].lfmt);

   uint32_t pixX = std::min(m_dstMS ? (desc.width  / 2) : desc.width,  fbWidth);
   uint32_t pixY = std::min(m_dstMS ? (desc.height / 2) : desc.height, fbHeight);

   SetupFrameConfig(pixX, pixY);
}

void ColorAspectCommandBuilder::CreateBinnerControlList(
      CmdBinRenderJobObj *brJob,
      v3d_barrier_flags   syncFlags)
{
   assert(m_numTilesX > 0 && m_numTilesY > 0);

   ControlList binList;
   const bool multisampled = m_srcMS || m_dstMS;

   StartBinJobCL(brJob, &binList, m_numRenderTargets, multisampled, m_v3dRtFormat.bpp);

   if (m_drawList != nullptr && m_drawList->Start() != 0)
   {
      InsertNVShaderRenderState(VkRect2D {0, 0, m_numPixelsX, m_numPixelsY}, false, false, 0);
      v3d_cl_branch_sub(CLPtr(), m_drawList->Start());
   }

   EndBinJobCL(brJob, &binList, syncFlags);
}

void ColorAspectCommandBuilder::AddTileListLoads()
{
   if (m_needsLoads || m_loadDestination)
   {
      for (uint32_t rt = 0; rt < m_numRenderTargets; rt++)
      {
         auto &ls = m_loadDestination ? m_storeTLBParams[rt] : m_loadTLBParams[rt];

         v3d_cl_load(CLPtr(),
               v3d_ldst_buf_color(rt),
               ls.memory_format,
               ls.flipy,
               ls.decimate,
               ls.pixel_format,
               ls.load_alpha_to_one,
               ls.chan_reverse,
               ls.rb_swap,
               ls.stride,
               ls.flipy_height_px,
               ls.addr);
      }
   }
}

void ColorAspectCommandBuilder::AddTileListStores()
{
   for (uint32_t rt = 0; rt < m_numRenderTargets; rt++)
   {
      auto &ls = m_storeTLBParams[rt];

      v3d_cl_store(CLPtr(),
            v3d_ldst_buf_color(rt),
            ls.memory_format,
            ls.flipy,
            ls.dither,
            ls.decimate,
            ls.pixel_format,
            /*clear=*/false,
            ls.chan_reverse,
            ls.rb_swap,
            ls.stride,
            ls.flipy_height_px,
            ls.addr);
   }

   if (m_needsClears)
      v3d_cl_clear(CLPtr(), true, false);
}

void ColorAspectCommandBuilder::InsertRenderTargetCfg()
{
   const bool multisampled = m_srcMS || m_dstMS;

   v3d_cl_tile_rendering_mode_cfg_common(CLPtr(),
         m_numRenderTargets,
         m_numPixelsX,
         m_numPixelsY,
         m_v3dRtFormat.bpp,
         multisampled,
         /*doubleBuffer=*/false,
         /*coverage=*/false,
         V3D_EZ_DIRECTION_LT_LE,
         /*earlyZDisable=*/true,
         /*depthType=*/V3D_DEPTH_TYPE_32F,
         /*earlyDSClear=*/false);

   v3d_cl_tile_rendering_mode_cfg_color(CLPtr(),
         m_v3dRtFormat.bpp, m_v3dRtFormat.type, m_v3dRtFormat.clamp,
         m_v3dRtFormat.bpp, m_v3dRtFormat.type, m_v3dRtFormat.clamp,
         m_v3dRtFormat.bpp, m_v3dRtFormat.type, m_v3dRtFormat.clamp,
         m_v3dRtFormat.bpp, m_v3dRtFormat.type, m_v3dRtFormat.clamp);

   if (m_needsClears)
   {
      for (unsigned rt = 0; rt < m_numRenderTargets; rt++)
      {
         v3d_cl_rcfg_clear_colors(CLPtr(),
               rt, m_clearValues, &m_v3dRtFormat);
      }
   }
}


void ColorAspectCommandBuilder::CreateRenderControlList(
      CmdBinRenderJobObj *brJob,
      const ControlList  &gtl,
      v3d_barrier_flags   syncFlags)
{
   assert(m_numRenderTargets > 0);
   ControlList renderList;

   StartRenderJobCL(brJob, &renderList);

   InsertRenderTargetCfg();

   // HW needs this to be last V3D_CL_TILE_RENDERING_MODE_CFG sub item
   v3d_cl_tile_rendering_mode_cfg_zs_clear_values(CLPtr(),
      0, v3d_snap_depth(1.0f, V3D_DEPTH_TYPE_32F));

   if (m_needsClears)
      InsertInitialTLBClear(/* doubleBuffer */ false, /* renderTargets */ true, /* depthStencil */ false);

   if (m_needsLoads || m_loadDestination)
      syncFlags |= V3D_BARRIER_TLB_IMAGE_READ;

   syncFlags |= V3D_BARRIER_TLB_IMAGE_WRITE;

   EndRenderJobCL(brJob, &renderList, gtl, /* core */ 0, syncFlags);
}

} // namespace bvk
