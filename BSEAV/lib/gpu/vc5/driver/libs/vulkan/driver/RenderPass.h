/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

#include "libs/core/lfmt/lfmt.h"
#include "libs/core/v3d/v3d_cl.h"
#include "libs/core/v3d/v3d_limits.h"

namespace bvk {

class RenderPass : public NonCopyable, public Allocating
{
public:
   // A SubpassGroup contains data relevant to a single group of subpasses.
   // Subpasses are grouped together when they can be executed in a single master
   // control list. i.e. sub-passes that don't need to load/store/resolve except at
   // the group level for example.
   class SubpassGroup
   {
   public:
      void LogTrace() const;
      bool Merge(SubpassGroup &from);

   public:
      uint32_t             m_firstSubpass;         // 1st subpass in this group
      uint32_t             m_numSubpasses;         // Number of subpasses in this group

      VkAttachmentLoadOp   m_colorLoadOp[V3D_MAX_RENDER_TARGETS];
      VkAttachmentStoreOp  m_colorStoreOp[V3D_MAX_RENDER_TARGETS];

      VkAttachmentLoadOp   m_depthLoadOp;
      VkAttachmentStoreOp  m_depthStoreOp;

      VkAttachmentLoadOp   m_stencilLoadOp;
      VkAttachmentStoreOp  m_stencilStoreOp;

      uint32_t             m_colorStoreMask;       // Bitmask for render target stores

      bool                 m_preBarrier;           // Execution barrier needed?
      bool                 m_sharesInputAndOutput; // Uses the same attachment as input & output

      // These contain indexes into the renderPass attachment list
      uint32_t             m_colorRTAttachments[V3D_MAX_RENDER_TARGETS];
      uint32_t             m_resolveRTAttachments[V3D_MAX_RENDER_TARGETS];
      uint32_t             m_dsAttachment;
#if !V3D_HAS_GFXH1461_FIX
      bool                 m_workaroundGFXH1461 = false;
#endif

      bool                 m_doInitialClear[V3D_MAX_RENDER_TARGETS] = {};
      bool                 m_doDSInitialClear = false;

      bool                 m_hasDepth;
      bool                 m_hasStencil;

      uint32_t             m_numColorRenderTargets;
      uint32_t             m_tileWidth;
      uint32_t             m_tileHeight;
      v3d_rt_bpp_t         m_maxRenderTargetBPP;   // Biggest BPP in this group
      bool                 m_multisampled;         // All attachments must match
   };

   struct Subpass
   {
      uint32_t       colorWriteMask;
      SubpassGroup  *group;
   };

   struct Attachment
   {
      VkImageAspectFlags vkClearRectAspectMask;
      V3D_RT_FORMAT_T    v3dRtFormat;
      v3d_depth_type_t   v3dDepthType;
      GFX_LFMT_T         format;
   };

public:
   RenderPass(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::Device                   *pDevice,
      const VkRenderPassCreateInfo  *pCreateInfo);

   void GetRenderAreaGranularity(
      bvk::Device *device,
      VkExtent2D  *pGranularity) noexcept;

   // Implementation specific from this point on
   const bvk::vector<Subpass>     &Subpasses() const   { return m_subpasses; }
   const bvk::vector<Attachment>  &Attachments() const { return m_attachments; }

   const SubpassGroup *GroupForSubpass(uint32_t subpass) const { return m_subpasses[subpass].group; }

   uint32_t DepthBits() const { return m_depthBits; }
   uint32_t ColorWriteMasks(uint32_t subpass) const { return m_subpasses[subpass].colorWriteMask; }

   bool PreBarrierExternal() const  { return m_preBarrierExternal; }
   bool PostBarrierExternal() const { return m_postBarrierExternal; }

private:
   void GatherAttachments(const VkAttachmentDescription *attachments, uint32_t attachmentCount);
   void GatherSubpasses(const VkSubpassDescription *subpasses, uint32_t subpassCount);
   void CalculateSubpassLoadsAndStores(
      SubpassGroup &spg,
      uint32_t subpassIndex,
      uint32_t rpAttachmentIndex,
      uint32_t rt,
      const bvk::vector<uint32_t> &firstUsage,
      const bvk::vector<uint32_t> &lastUsage,
      const VkAttachmentDescription *attachments,
      const VkSubpassDescription *nextSubpass);

   void CalculateSubpassGroups(const VkRenderPassCreateInfo *pCreateInfo);
   void MergeSubpassGroups();
   void LogSubpassGroups() const;

private:
   bvk::vector<Attachment>    m_attachments;
   bvk::vector<Subpass>       m_subpasses;
   bvk::vector<SubpassGroup>  m_subpassGroups;

   uint32_t m_maxTileWidth;
   uint32_t m_maxTileHeight;
   uint32_t m_depthBits;

   // Set if any subpass has VK_SUBPASS_EXTERNAL dependencies
   bool m_preBarrierExternal = false;
   bool m_postBarrierExternal = false;
};

} // namespace bvk
