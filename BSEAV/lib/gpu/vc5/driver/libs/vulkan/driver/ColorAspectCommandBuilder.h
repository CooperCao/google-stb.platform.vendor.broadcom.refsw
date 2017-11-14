/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "ControlListBuilder.h"
#include "Image.h"

#include "libs/core/v3d/v3d_tlb.h"

#include <cstring>

namespace bvk {

class ColorAspectCommandBuilder: public ControlListBuilder
{
public:
   ColorAspectCommandBuilder(const VkAllocationCallbacks *pCallbacks, CommandBuffer *cmdBuf):
      ControlListBuilder(pCallbacks, cmdBuf)
   {
      m_needsLoads       = false;
      m_needsClears      = false;
      m_loadDestination  = false;
      m_srcMS            = false;
      m_dstMS            = false;
      m_numRenderTargets = 0;
      m_v3dRtFormat.type = V3D_RT_TYPE_INVALID;
      m_v3dRtFormat.bpp  = V3D_RT_BPP_INVALID;
#if V3D_VER_AT_LEAST(4,1,34,0)
      m_v3dRtFormat.clamp = V3D_RT_CLAMP_INVALID;
#endif
      m_loadOffset       = VkOffset2D {0, 0};
      m_storeOffset      = VkOffset2D {0, 0};
      m_drawList         = nullptr;
   }

   // Return the multiples of x and y offsets which can be supported by the
   // hardware for the given image descriptor image layout. If no offset can
   // be supported 0 is returned in both values.
   static VkOffset2D GetImageOffsetAlignments(const GFX_BUFFER_DESC_T *desc);

   static VkOffset2D GetImageOffsetAlignments(bvk::Image *img, uint32_t mipLevel)
   {
      const GFX_BUFFER_DESC_T &desc = img->GetDescriptor(mipLevel);
      return GetImageOffsetAlignments(&desc);
   }

   bool TLBUseWRCfg16 () const { return v3d_tlb_rt_type_use_rw_cfg_16(m_v3dRtFormat.type); }
   bool TLBIsInt()       const { return v3d_tlb_rt_type_is_int(m_v3dRtFormat.type); }

   // Setup the control list parameters for (optional) src and dst images
   // - srcImage may be nullptr
   // - altTLBFmt provides an optional alternative color format that should be
   //   used instead of the (natural) one in the image descriptors. This does
   //   not have to be the Formats::TLB format, it can be anything that has the
   //   same size as the natural format and is supported by the TLB. To just
   //   use the natural format in the descriptor set this to GFX_LFMT_NONE.
   // - for 3D images the baseLayer and layerCount refer to slices otherwise
   //   they are treated as array layers.
   // - optional framebuffer width and height can restrict rendering to a
   //   portion of the image starting at the top left; Note the origin can _not_
   //   be shifted.
   void SetImageSubresources(
         Image *srcImage, Image *dstImage, GFX_LFMT_T altTLBFmt,
         uint32_t srcMipLevel, uint32_t dstMipLevel,
         uint32_t srcBaseLayer, uint32_t dstBaseLayer, uint32_t layerCount,
         uint32_t fbWidth = V3D_MAX_CLIP_WIDTH, uint32_t fbHeight = V3D_MAX_CLIP_HEIGHT);

   // Overload for rendering over an entire destination image with no loads
   void SetImageSubresources(
         Image *dstImage, GFX_LFMT_T altTLBFmt,
         uint32_t mipLevel, uint32_t baseLayer, uint32_t layerCount)
   {
      SetImageSubresources(nullptr, dstImage, altTLBFmt, 0, mipLevel, 0, baseLayer, layerCount);
   }

   void SetDstTLBParamsDirect(const v3d_tlb_ldst_params &params, bool isMultisamped,
                              uint32_t width, uint32_t height);

   void SetStoreOffset(uint32_t x, uint32_t y)
   {
      m_storeOffset.x = x;
      m_storeOffset.y = y;
   }

   void SetLoadOffset(uint32_t x, uint32_t y)
   {
      m_loadOffset.x = x;
      m_loadOffset.y = y;
   }

   void SetClearParams(const uint32_t colors[4])
   {
      // explicit load, preserve destination and clear are mutually exclusive
      assert(!m_loadDestination && !m_needsLoads);

      m_needsClears = true;
      std::memcpy(m_clearValues, colors, sizeof(m_clearValues));
   }

   void SetLoadDestination()
   {
      // explicit load, preserve destination and clear are mutually exclusive
      assert(!m_needsClears && !m_needsLoads);
      m_loadDestination = true;
   }

   void SetDrawList(const ControlList *cl) { m_drawList = cl; }

private:
   static uint32_t OffsetToBytes(GFX_BUFFER_DESC_T *desc, VkOffset2D offset, bool ms);

   void ImageParams(v3d_tlb_ldst_params *ldstParams, VkOffset2D offset,
                    Image *img, GFX_LFMT_T altTLBFmt,
                    uint32_t mipLevel, uint32_t baseLayer);

   void SetupFrameConfig(uint32_t pixX, uint32_t pixY);
   void SetupFrameConfig(Image *img, GFX_LFMT_T altTLBFmt, uint32_t mipLevel,
                         uint32_t fbWidth, uint32_t fbHeight);

   void CreateBinnerControlList(CmdBinRenderJobObj *brJob, v3d_barrier_flags syncFlags) override;
   void CreateRenderControlList(CmdBinRenderJobObj *brJob, const ControlList &gtl,
                                v3d_barrier_flags syncFlags) override;

   // These helpers have conditional implementations based on the HW version
   // They insert commands directly into the current "open" control list
   void AddTileListLoads() override;
   void AddTileListStores() override;

   void InsertRenderTargetCfg();

private:
   bool                m_needsLoads;
   bool                m_needsClears;
   bool                m_loadDestination;
   bool                m_srcMS;
   bool                m_dstMS;
   uint32_t            m_numRenderTargets;
   V3D_RT_FORMAT_T     m_v3dRtFormat;
   v3d_tlb_ldst_params m_loadTLBParams[V3D_MAX_RENDER_TARGETS];
   v3d_tlb_ldst_params m_storeTLBParams[V3D_MAX_RENDER_TARGETS];
   uint32_t            m_clearValues[4];
   VkOffset2D          m_loadOffset;
   VkOffset2D          m_storeOffset;

   // Optional draw call control list, called by the bin control list, if not null
   const ControlList  *m_drawList;
};

} // namespace bvk
