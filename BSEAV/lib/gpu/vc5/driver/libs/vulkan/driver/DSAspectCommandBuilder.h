/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "ControlListBuilder.h"
#include "Image.h"

#include <cstring>

namespace bvk {

class DSAspectCommandBuilder: public ControlListBuilder
{
public:
   DSAspectCommandBuilder(const VkAllocationCallbacks *pCallbacks, CommandBuffer *cmdBuf):
      ControlListBuilder(pCallbacks, cmdBuf)
   {
      m_needsClears      = false;
      m_loadDestination  = false;
      m_dstMS            = false;
      m_depthType        = V3D_DEPTH_TYPE_INVALID;
      m_depthClear       = 0.0f;
      m_stencilClear     = 0;
      m_aspect           = 0;
      m_drawList         = nullptr;
   }

   void SetImageSubresources(Image *img, uint32_t mipLevel, uint32_t layerOrSlice, VkImageAspectFlags aspect);

   void SetDstTLBParamsDirect(const v3d_tlb_ldst_params &params, bool isMultisampled,
                              VkImageAspectFlags aspect, uint32_t width, uint32_t height);

   void SetClearParams(float depth, uint8_t stencil)
   {
      // load destination and clear are mutually exclusive
      assert(!m_loadDestination);
      m_needsClears  = true;
      m_depthClear   = depth;
      m_stencilClear = stencil;
   }

   void SetLoadDestination()
   {
      // load destination and clear are mutually exclusive
      assert(!m_needsClears);
      m_loadDestination = true;
   }

   void SetDrawList(const ControlList *cl) { m_drawList = cl; }

private:
   void CreateBinnerControlList(CmdBinRenderJobObj *brJob, v3d_barrier_flags syncFlags) override;
   void CreateRenderControlList(CmdBinRenderJobObj *brJob, const ControlList &gtl,
                                v3d_barrier_flags syncFlags) override;

   // These helpers have conditional implementations based on the HW version
   // They insert commands directly into the current "open" control list
   void AddTileListLoads() override;
   void AddTileListStores() override;

   void InsertRenderTargetCfg();
   void SetupFrameConfig(uint32_t width, uint32_t height);

private:
   bool                m_needsClears;
   bool                m_loadDestination;
   bool                m_dstMS;

   v3d_tlb_ldst_params m_TLBParams;
   v3d_depth_type_t    m_depthType;

   float               m_depthClear;
   uint8_t             m_stencilClear;
   bool                m_hasDepth;
   bool                m_hasStencil;
   VkImageAspectFlags  m_aspect;

   // Optional draw call control list, called by the bin control list, if not null
   const ControlList  *m_drawList;
};

} // namespace bvk
