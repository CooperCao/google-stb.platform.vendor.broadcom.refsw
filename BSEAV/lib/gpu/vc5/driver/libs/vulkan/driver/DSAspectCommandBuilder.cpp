/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "DSAspectCommandBuilder.h"
#include "CommandBuffer.h"
#include "Command.h"
#include "Options.h"
#include "Image.h"

#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/v3d/v3d_gen.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/core/v3d/v3d_tile_size.h"

namespace bvk {

LOG_DEFAULT_CAT("bvk::DSAspectCommandBuilder");

// Fixed dummy RT TLB config for depth/stencil only operations
#if V3D_VER_AT_LEAST(4,1,34,0)
static const V3D_RT_FORMAT_T s_v3dRtFormat = {V3D_RT_BPP_32, V3D_RT_TYPE_8, V3D_RT_CLAMP_NONE};
#else
static const V3D_RT_FORMAT_T s_v3dRtFormat = {V3D_RT_BPP_32, V3D_RT_TYPE_8};
#endif

void DSAspectCommandBuilder::SetupFrameConfig(uint32_t width, uint32_t height)
{
   uint32_t tileWidth, tileHeight;
   v3d_tile_size_pixels(
      &tileWidth, &tileHeight, m_dstMS,
      /*double_buffer=*/false, /*num_rts=*/1, s_v3dRtFormat.bpp);

   m_numPixelsX = m_dstMS ? (width  / 2) : width;
   m_numPixelsY = m_dstMS ? (height / 2) : height;

   // Currently the physical device max image dimensions are set to these
   // limits, but that will probably change to support larger textures at some
   // point, if that is actually relevant to depth/stencil images.
   assert(m_numPixelsX <= V3D_MAX_CLIP_WIDTH && m_numPixelsY <= V3D_MAX_CLIP_HEIGHT);

   m_numTilesX = gfx_udiv_round_up(m_numPixelsX, tileWidth);
   m_numTilesY = gfx_udiv_round_up(m_numPixelsY, tileHeight);

   log_trace("\tfbsize (%ux%u) tileSize (%ux%u) tiles (%ux%u)",
      (unsigned)m_numPixelsX, (unsigned)m_numPixelsY,
      (unsigned)tileWidth, (unsigned)tileHeight,
      (unsigned)m_numTilesX, (unsigned)m_numTilesY);
}

void DSAspectCommandBuilder::SetImageSubresources(
      Image             *img,
      uint32_t           mipLevel,
      uint32_t           layerOrSlice,
      VkImageAspectFlags aspect)
{
   const v3d_addr_t basePhys = img->PhysAddr();
   const GFX_LFMT_T lfmt     = img->NaturalLFMT();

   const bool     is3D  = gfx_lfmt_is_3d(lfmt);
   const uint32_t layer = is3D ? 0 : layerOrSlice;
   const uint32_t slice = is3D ? layerOrSlice : 0;

   m_hasDepth   = gfx_lfmt_has_depth(lfmt);
   m_hasStencil = gfx_lfmt_has_stencil(lfmt);
   m_dstMS      = img->Samples() != VK_SAMPLE_COUNT_1_BIT;
   m_aspect     = aspect;

   log_trace("SetImageSubresources:");
   log_trace("\timage = %p layer/slice = %u", img, (unsigned)layerOrSlice);
   log_trace("\tbasePhys= %#x m_dstMS = %s is3D = %s", (unsigned)basePhys, TF(m_dstMS), TF(is3D));

   const GFX_BUFFER_DESC_T &desc = img->GetDescriptor(mipLevel);

   SetupFrameConfig(desc.width, desc.height);

   gfx_buffer_translate_tlb_ldst(
         &m_TLBParams, basePhys + img->LayerOffset(layer), &desc,
         /*plane_i=*/0, slice, /*color=*/ false, m_dstMS, m_dstMS, V3D_DITHER_OFF);

   // NOTE: S8 and D32S8 are not supported in the physical device properties
   m_depthType = gfx_lfmt_translate_depth_type(desc.planes[0].lfmt);
}

void DSAspectCommandBuilder::SetDstTLBParamsDirect(
      const v3d_tlb_ldst_params &params, bool isMultisampled, VkImageAspectFlags aspect,
      uint32_t width, uint32_t height)
{
   m_TLBParams  = params;
   m_hasDepth   = (aspect & VK_IMAGE_ASPECT_DEPTH_BIT)   != 0;
   m_hasStencil = (aspect & VK_IMAGE_ASPECT_STENCIL_BIT) != 0;
   m_dstMS      = isMultisampled;
   m_aspect     = aspect;

   SetupFrameConfig(width, height);

# if V3D_VER_AT_LEAST(4,1,34,0)
   GFX_LFMT_T lfmt = gfx_lfmt_translate_from_pixel_format(params.pixel_format, params.chan_reverse,
                                                          params.rb_swap);
# else
   GFX_LFMT_T lfmt = gfx_lfmt_translate_from_pixel_format(params.pixel_format);
# endif // V3D_VER_AT_LEAST(4,1,34,0)

   // NOTE: S8 and D32S8 are not supported in the physical device properties
   m_depthType = gfx_lfmt_translate_depth_type(lfmt);
}

void DSAspectCommandBuilder::CreateBinnerControlList(
      CmdBinRenderJobObj *brJob,
      v3d_barrier_flags   syncFlags)
{
   assert(m_numTilesX > 0 && m_numTilesY > 0);
   ControlList binList;

   StartBinJobCL(brJob, &binList, 1, m_dstMS, s_v3dRtFormat.bpp);

   if (m_drawList != nullptr && m_drawList->Start() != 0)
   {
      const bool stencilWrite = m_hasStencil && !!(m_aspect & VK_IMAGE_ASPECT_STENCIL_BIT);
      const bool depthUpdate  = m_hasDepth   && !!(m_aspect & VK_IMAGE_ASPECT_DEPTH_BIT);

      InsertNVShaderRenderState(VkRect2D {0, 0, m_numPixelsX, m_numPixelsY}, depthUpdate, stencilWrite,
         gfx_mask(V3D_MAX_RENDER_TARGETS * 4));
      v3d_cl_branch_sub(CLPtr(), m_drawList->Start());
   }

   EndBinJobCL(brJob, &binList, syncFlags);
}

void DSAspectCommandBuilder::AddTileListLoads()
{
   if (m_loadDestination)
   {
      v3d_cl_load(CLPtr(),
            v3d_ldst_buf_ds(m_hasDepth, m_hasStencil),
            m_TLBParams.memory_format,
            m_TLBParams.flipy,
            m_TLBParams.decimate,
            m_TLBParams.pixel_format,
#if V3D_VER_AT_LEAST(4,1,34,0)
            false, false, false,    /* alpha_to_one, chan_reverse, rb_swap */
#endif
            m_TLBParams.stride,
            m_TLBParams.flipy_height_px,
            m_TLBParams.addr);
   }
}

void DSAspectCommandBuilder::AddTileListStores()
{
   const bool stencilStore = m_hasStencil && !!(m_aspect & VK_IMAGE_ASPECT_STENCIL_BIT);
   const bool depthStore   = m_hasDepth && !!(m_aspect & VK_IMAGE_ASPECT_DEPTH_BIT);

   v3d_cl_store(CLPtr(),
         v3d_ldst_buf_ds(depthStore, stencilStore),
         m_TLBParams.memory_format,
         m_TLBParams.flipy,
         m_TLBParams.dither,
         m_TLBParams.decimate,
         m_TLBParams.pixel_format,
         /*clear=*/false,
#if V3D_VER_AT_LEAST(4,1,34,0)
         false, false,    /* chan_reverse, rb_swap */
#endif
         m_TLBParams.stride,
         m_TLBParams.flipy_height_px,
         m_TLBParams.addr);

   if (m_needsClears)
      v3d_cl_clear(CLPtr(), false, true);
}

void DSAspectCommandBuilder::InsertRenderTargetCfg()
{
   v3d_cl_tile_rendering_mode_cfg_common(CLPtr(),
         /*numRenderTargets=*/1, // there must be at least one, even if we aren't using it
         m_numPixelsX,
         m_numPixelsY,
         s_v3dRtFormat.bpp,
         m_dstMS,
         /*doubleBuffer=*/false,
         /*coverage=*/false,
         V3D_EZ_DIRECTION_LT_LE,
         /*earlyZDisable=*/true,
         /*depthType=*/m_depthType,
         /*earlyDSClear=*/false);

#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_cl_tile_rendering_mode_cfg_color(CLPtr(),
         s_v3dRtFormat.bpp, s_v3dRtFormat.type, s_v3dRtFormat.clamp,
         s_v3dRtFormat.bpp, s_v3dRtFormat.type, s_v3dRtFormat.clamp,
         s_v3dRtFormat.bpp, s_v3dRtFormat.type, s_v3dRtFormat.clamp,
         s_v3dRtFormat.bpp, s_v3dRtFormat.type, s_v3dRtFormat.clamp);
#else
   v3d_cl_tile_rendering_mode_cfg_color(CLPtr(),
         s_v3dRtFormat.bpp, s_v3dRtFormat.type,
         s_v3dRtFormat.bpp, s_v3dRtFormat.type,
         s_v3dRtFormat.bpp, s_v3dRtFormat.type,
         s_v3dRtFormat.bpp, s_v3dRtFormat.type);
#endif
}


void DSAspectCommandBuilder::CreateRenderControlList(
      CmdBinRenderJobObj *brJob,
      const ControlList  &gtl,
      v3d_barrier_flags   syncFlags)
{
   ControlList renderList;

   StartRenderJobCL(brJob, &renderList);

   InsertRenderTargetCfg();

   // HW needs this to be last V3D_CL_TILE_RENDERING_MODE_CFG sub item
   v3d_cl_tile_rendering_mode_cfg_zs_clear_values(CLPtr(),
         m_stencilClear,
         v3d_snap_depth(m_depthClear, m_depthType));

   if (m_needsClears)
      InsertInitialTLBClear(/*doubleBuffer=*/false, /*renderTargets=*/false, /*depthStencil=*/true);

   if (m_loadDestination)
      syncFlags |= V3D_BARRIER_TLB_IMAGE_READ;

   syncFlags |= V3D_BARRIER_TLB_IMAGE_WRITE;

   EndRenderJobCL(brJob, &renderList, gtl, /*core=*/0, syncFlags);
}

} // namespace bvk
