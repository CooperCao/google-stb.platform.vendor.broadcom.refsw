/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/v3d/v3d_tile_size.h"
#include "libs/util/log/log.h"

#include <algorithm>

LOG_DEFAULT_CAT("bvk::RenderPass");

namespace bvk {

static bool Preserves(const VkSubpassDescription &spDesc, uint32_t rpAttachmentIndex)
{
   for (uint32_t i = 0; i != spDesc.preserveAttachmentCount; ++i)
      if (spDesc.pPreserveAttachments[i] == rpAttachmentIndex)
         return true;
   return false;
}

static bool ColourOrDepth(const VkSubpassDescription &spDesc, uint32_t rpAttachmentIndex)
{
   if (spDesc.pDepthStencilAttachment != nullptr)
      if (spDesc.pDepthStencilAttachment->attachment == rpAttachmentIndex)
         return true;

   for (uint32_t i = 0; i != spDesc.colorAttachmentCount; ++i)
      if (spDesc.pColorAttachments[i].attachment == rpAttachmentIndex)
         return true;

   for (uint32_t i = 0; i != spDesc.inputAttachmentCount; ++i)
      if (spDesc.pInputAttachments[i].attachment == rpAttachmentIndex)
         return true;

   return false;
}

static bool Uses(const VkSubpassDescription &spDesc, uint32_t rpAttachmentIndex)
{
   if (ColourOrDepth(spDesc, rpAttachmentIndex))
      return true;

   for (uint32_t i = 0; i != spDesc.inputAttachmentCount; ++i)
      if (spDesc.pInputAttachments[i].attachment == rpAttachmentIndex)
         return true;

   return false;
}

static bool SharesInputAndOutput(const VkSubpassDescription &spDesc)
{
   for (uint32_t i = 0; i != spDesc.inputAttachmentCount; ++i)
   {
      uint32_t ia = spDesc.pInputAttachments[i].attachment;
      if (ia != VK_ATTACHMENT_UNUSED)
         if (ColourOrDepth(spDesc, ia))
            return true;
   }
   return false;
}

RenderPass::RenderPass(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   const VkRenderPassCreateInfo  *pCreateInfo) :
      Allocating(pCallbacks),
      m_attachments(pCreateInfo->attachmentCount, Attachment(), GetObjScopeAllocator<Attachment>()),
      m_subpasses(pCreateInfo->subpassCount, Subpass(), GetObjScopeAllocator<Subpass>()),
      m_subpassGroups(GetObjScopeAllocator<SubpassGroup>())
{
   // Gather as much information about the attachments up-front as we can
   GatherAttachments(pCreateInfo->pAttachments, pCreateInfo->attachmentCount);
   GatherSubpasses(pCreateInfo->pSubpasses, pCreateInfo->subpassCount);

   // Calculate subpass groupings
   CalculateSubpassGroups(pCreateInfo);

   // Do we need pre or post barriers?
   for (uint32_t i = 0; i != pCreateInfo->dependencyCount; ++i)
   {
      const VkSubpassDependency& dep = pCreateInfo->pDependencies[i];

      if (dep.srcSubpass == VK_SUBPASS_EXTERNAL && dep.srcStageMask != VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)
         m_preBarrierExternal = true;

      if (dep.dstSubpass == VK_SUBPASS_EXTERNAL && dep.dstStageMask != VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
         m_postBarrierExternal = true;
   }

   log_trace("Max tile width  = %u", m_maxTileWidth);
   log_trace("Max tile height = %u", m_maxTileHeight);
   log_trace("Pre barrier external  = %u", m_preBarrierExternal ? 1 : 0);
   log_trace("Post barrier external = %u", m_postBarrierExternal ? 1 : 0);

   // Calculate and cache depth bits
   for (uint32_t a = 0; a != pCreateInfo->attachmentCount; ++a)
      m_depthBits = gfx_lfmt_depth_bits(m_attachments[a].format);

   // Print the final groups
   LogSubpassGroups();
}

void RenderPass::GatherAttachments(const VkAttachmentDescription *attachments, uint32_t attachmentCount)
{
   // Gather as much format based information about the attachments as we can
   for (uint32_t a = 0; a != attachmentCount; ++a)
   {
      const VkAttachmentDescription &desc  = attachments[a];
      Attachment &att = m_attachments[a];

      // Find the format of the attachment and convert to v3d types
      att.format = Formats::GetLFMT(desc.format);

      // Precompute mask used by CommandBufferBuilder::DrawClearRectForAttachmentIfNeeded
      VkImageAspectFlags aspectMask = Formats::GetAspects(att.format);
      if (desc.loadOp != VK_ATTACHMENT_LOAD_OP_CLEAR)
         aspectMask &= ~(VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT);
      if (desc.stencilLoadOp != VK_ATTACHMENT_LOAD_OP_CLEAR)
         aspectMask &= ~VK_IMAGE_ASPECT_STENCIL_BIT;
      att.vkClearRectAspectMask = aspectMask;

      att.v3dRtFormat = V3D_RT_FORMAT_T{V3D_RT_BPP_32, V3D_RT_TYPE_8, V3D_RT_CLAMP_NONE};

      // We may see attachments in the list that aren't valid. This is useless, but not
      // invalid usage, so we must use the maybe variants of translate functions.
      if (gfx_lfmt_has_depth(att.format) || gfx_lfmt_has_stencil(att.format))
         att.v3dDepthType = gfx_lfmt_maybe_translate_depth_type(att.format);
      else
         gfx_lfmt_maybe_translate_rt_format(&att.v3dRtFormat, att.format);
   }
}

void RenderPass::GatherSubpasses(const VkSubpassDescription *subpasses, uint32_t subpassCount)
{
   for (uint32_t i = 0; i != subpassCount; ++i)
   {
      const VkSubpassDescription &desc = subpasses[i];
      Subpass &subpass = m_subpasses[i];

      // Compute colour write mask for this subpass.
      uint32_t enableMask = 0;
      for (uint32_t rt = 0; rt != desc.colorAttachmentCount; ++rt)
      {
         uint32_t att = desc.pColorAttachments[rt].attachment;
         if (att == VK_ATTACHMENT_UNUSED)
            continue;

         // Disable alpha writes for buffers which don't have alpha channels. We need
         // the alpha in the TLB to be 1 to get correct blending in the case where
         // the buffer doesn't have alpha. The TLB will set alpha to 1 when it loads
         // an alpha-less buffer, but we need to explicitly mask alpha writes after
         // that to prevent it changing.
         GFX_LFMT_T lfmt = m_attachments[att].format;
         enableMask |= (gfx_lfmt_has_alpha(lfmt) ? 0xf : 0x7) << (rt * 4);
      }
      subpass.colorWriteMask = enableMask;
   }
}

static void SetLoadOp(RenderPass::SubpassGroup &spg, VkImageAspectFlags dsc, uint32_t rt,
                      const VkAttachmentDescription &attDesc)
{
   if (dsc & VK_IMAGE_ASPECT_DEPTH_BIT)
      spg.m_depthLoadOp = attDesc.loadOp;
   if (dsc & VK_IMAGE_ASPECT_STENCIL_BIT)
      spg.m_stencilLoadOp = attDesc.stencilLoadOp;
   if (dsc & VK_IMAGE_ASPECT_COLOR_BIT)
      spg.m_colorLoadOp[rt] = attDesc.loadOp;
}

static void SetStoreOp(RenderPass::SubpassGroup &spg, VkImageAspectFlags dsc, uint32_t rt,
                       const VkAttachmentDescription &attDesc)
{
   if (dsc & VK_IMAGE_ASPECT_DEPTH_BIT)
      spg.m_depthStoreOp = attDesc.storeOp;
   if (dsc & VK_IMAGE_ASPECT_STENCIL_BIT)
      spg.m_stencilStoreOp = attDesc.stencilStoreOp;
   if (dsc & VK_IMAGE_ASPECT_COLOR_BIT)
      spg.m_colorStoreOp[rt] = attDesc.storeOp;
}

static void SetStoreOp(RenderPass::SubpassGroup &spg, VkImageAspectFlags dsc, uint32_t rt,
                       VkAttachmentStoreOp storeOp)
{
   if (dsc & VK_IMAGE_ASPECT_DEPTH_BIT)
      spg.m_depthStoreOp = storeOp;
   if (dsc & VK_IMAGE_ASPECT_STENCIL_BIT)
      spg.m_stencilStoreOp = storeOp;
   if (dsc & VK_IMAGE_ASPECT_COLOR_BIT)
      spg.m_colorStoreOp[rt] = storeOp;
}

void RenderPass::CalculateSubpassLoadsAndStores(
   SubpassGroup &spg,
   uint32_t subpassIndex,
   uint32_t rpAttachmentIndex,
   uint32_t rt,
   const bvk::vector<uint32_t> &firstUsage,
   const bvk::vector<uint32_t> &lastUsage,
   const VkAttachmentDescription *attachments,
   const VkSubpassDescription *nextSubpass)
{
   const VkAttachmentDescription &attDesc = attachments[rpAttachmentIndex];

   VkImageAspectFlags aspects = Formats::GetAspects(m_attachments[rpAttachmentIndex].format);

   if (firstUsage[rpAttachmentIndex] == subpassIndex)  // First used in this subpass
   {
      SetLoadOp(spg, aspects, rt, attDesc);

      if (lastUsage[rpAttachmentIndex] != subpassIndex) // Not also the last use
      {
         assert(nextSubpass);

         if (Uses(*nextSubpass, rpAttachmentIndex) || Preserves(*nextSubpass, rpAttachmentIndex))
         {
            // Need to store from this subpass
            SetStoreOp(spg, aspects, rt, VK_ATTACHMENT_STORE_OP_STORE);
         }
         else
         {
            // No need to store
            SetStoreOp(spg, aspects, rt, VK_ATTACHMENT_STORE_OP_DONT_CARE);
         }
      }
   }

   if (subpassIndex > firstUsage[rpAttachmentIndex] && subpassIndex <= lastUsage[rpAttachmentIndex])
   {
      // We are being used after the first usage and before or during the last usage.
      // If the first used subpass needed to store, then we need to preserve.
      const SubpassGroup &firstSPG = m_subpassGroups[firstUsage[rpAttachmentIndex]];

      if ((aspects & VK_IMAGE_ASPECT_DEPTH_BIT) &&
           firstSPG.m_depthStoreOp == VK_ATTACHMENT_STORE_OP_STORE)
      {
         // Load and store this subpass
         spg.m_depthLoadOp  = VK_ATTACHMENT_LOAD_OP_LOAD;
         spg.m_depthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
      }

      if ((aspects & VK_IMAGE_ASPECT_STENCIL_BIT) &&
           firstSPG.m_stencilStoreOp == VK_ATTACHMENT_STORE_OP_STORE)
      {
         spg.m_stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_LOAD;
         spg.m_stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
      }

      if (aspects & VK_IMAGE_ASPECT_COLOR_BIT)
      {
         spg.m_colorLoadOp[rt]  = VK_ATTACHMENT_LOAD_OP_LOAD;
         spg.m_colorStoreOp[rt] = VK_ATTACHMENT_STORE_OP_STORE;
      }
   }

   if (lastUsage[rpAttachmentIndex] == subpassIndex)  // Last used in this subpass
      SetStoreOp(spg, aspects, rt, attDesc);
}

static void UpdateUsage(uint32_t rpIndex, uint32_t subpassIndex,
                        bvk::vector<uint32_t> &firstUsage, bvk::vector<uint32_t> &lastUsage)
{
   if (rpIndex != VK_ATTACHMENT_UNUSED)
   {
      if (firstUsage[rpIndex] == ~0U)
         firstUsage[rpIndex] = subpassIndex;

      if (lastUsage[rpIndex] == ~0U || lastUsage[rpIndex] < subpassIndex)
         lastUsage[rpIndex] = subpassIndex;
   }
}

static bool DependsOnEarlierSubpass(const VkRenderPassCreateInfo *pCreateInfo, uint32_t subpass)
{
   for (uint32_t d = 0; d != pCreateInfo->dependencyCount; ++d)
   {
      const VkSubpassDependency& dep = pCreateInfo->pDependencies[d];
      if (dep.dstSubpass != VK_SUBPASS_EXTERNAL && dep.srcSubpass != VK_SUBPASS_EXTERNAL)
         if (subpass == dep.dstSubpass && subpass > dep.srcSubpass)
            return true;
   }
   return false;
}

void RenderPass::CalculateSubpassGroups(const VkRenderPassCreateInfo *pCreateInfo)
{
   // We need to determine which sub-passes we can group together into single master
   // control lists. In order to group they:
   // 1) Must use the same render targets
   // 2) Must not do multisample resolves
   // 3) All render targets must share the same number of samples
   // 4) Must have adjacent load/store/clear values that allow merging

   // TODO : do something more complex than one subpass per group

   // We need to know the first and last usage points of each attachment, so work it out
   bvk::vector<uint32_t> firstUsage(pCreateInfo->attachmentCount, ~0U, GetObjScopeAllocator<uint32_t>());
   bvk::vector<uint32_t> lastUsage(pCreateInfo->attachmentCount, ~0U, GetObjScopeAllocator<uint32_t>());

   for (uint32_t s = 0; s != pCreateInfo->subpassCount; ++s)
   {
      const VkSubpassDescription &spDesc = pCreateInfo->pSubpasses[s];

      if (spDesc.pDepthStencilAttachment)
         UpdateUsage(spDesc.pDepthStencilAttachment->attachment, s, firstUsage, lastUsage);

      for (uint32_t i = 0; i != spDesc.colorAttachmentCount; ++i)
         UpdateUsage(spDesc.pColorAttachments[i].attachment, s, firstUsage, lastUsage);

      for (uint32_t i = 0; i != spDesc.inputAttachmentCount; ++i)
         UpdateUsage(spDesc.pInputAttachments[i].attachment, s, firstUsage, lastUsage);
   }

   // Start by putting each subpass in its own subpassGroup (we'll merge them later)
   m_subpassGroups.reserve(pCreateInfo->subpassCount);
   for (uint32_t s = 0; s != pCreateInfo->subpassCount; ++s)
   {
      const VkSubpassDescription &spDesc = pCreateInfo->pSubpasses[s];

      // We'll need the next subpass too
      const VkSubpassDescription *nextSubpass = nullptr;
      if (s + 1 < pCreateInfo->subpassCount)
         nextSubpass = &pCreateInfo->pSubpasses[s + 1];

      // Add a new group to the list and fill in the details
      m_subpassGroups.emplace_back();
      auto &spg = m_subpassGroups.back();

      spg.m_firstSubpass = s;
      spg.m_numSubpasses = 1;

      spg.m_preBarrier           = DependsOnEarlierSubpass(pCreateInfo, s);
      spg.m_sharesInputAndOutput = SharesInputAndOutput(spDesc);

      assert(spDesc.colorAttachmentCount <= V3D_MAX_RENDER_TARGETS);
      spg.m_numColorRenderTargets = spDesc.colorAttachmentCount;
      for (size_t a = 0; a < spDesc.colorAttachmentCount; a++)
      {
         uint32_t att     = spDesc.pColorAttachments[a].attachment;
         uint32_t resolve = spDesc.pResolveAttachments
                          ? spDesc.pResolveAttachments[a].attachment : VK_ATTACHMENT_UNUSED;

         spg.m_colorRTAttachments[a]   = att;
         spg.m_resolveRTAttachments[a] = resolve;

         // If this spg is the first use of the attachment, mark it for clearing
         spg.m_doInitialClear[a] = att != VK_ATTACHMENT_UNUSED && firstUsage[att] == s;
      }

      // Assign depth/ stencil attributes
      spg.m_dsAttachment = spDesc.pDepthStencilAttachment
                         ? spDesc.pDepthStencilAttachment->attachment : VK_ATTACHMENT_UNUSED;
      spg.m_hasDepth     = false;
      spg.m_hasStencil   = false;

      spg.m_depthLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      spg.m_stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      spg.m_depthStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      spg.m_stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

      if (spg.m_dsAttachment != VK_ATTACHMENT_UNUSED)
      {
         const GFX_LFMT_T lfmt = m_attachments[spg.m_dsAttachment].format;

         spg.m_hasDepth   = gfx_lfmt_has_depth(lfmt);
         spg.m_hasStencil = gfx_lfmt_has_stencil(lfmt);

         // If this spg is the first use of the attachment, mark it for clearing
         if (firstUsage[spg.m_dsAttachment] == s)
            spg.m_doDSInitialClear = true;

         CalculateSubpassLoadsAndStores(spg, s, spg.m_dsAttachment, ~0U,
                                        firstUsage, lastUsage, pCreateInfo->pAttachments,
                                        nextSubpass);

#if !V3D_HAS_GFXH1461_FIX
         if (spg.m_hasDepth && spg.m_hasStencil)
         {
            const VkAttachmentDescription &attDesc = pCreateInfo->pAttachments[spg.m_dsAttachment];
            if ((attDesc.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD  && attDesc.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) ||
                (attDesc.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR && attDesc.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD ))
            {
               spg.m_workaroundGFXH1461 = true;
            }
         }
#endif
      }

      // Assign color attributes
      for (uint32_t rt = 0; rt < spg.m_numColorRenderTargets; rt++)
      {
         uint32_t colAttIndex = spg.m_colorRTAttachments[rt];
         if (colAttIndex == VK_ATTACHMENT_UNUSED)
            continue;

         spg.m_colorLoadOp[rt]  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
         spg.m_colorStoreOp[rt] = VK_ATTACHMENT_STORE_OP_DONT_CARE;

         CalculateSubpassLoadsAndStores(spg, s, colAttIndex, rt,
                                        firstUsage, lastUsage, pCreateInfo->pAttachments,
                                        nextSubpass);
      }
   }

   // Now we iterate over each subpassGroup attempting to 'merge' with the
   // previous one. In order to merge successfully, all the rules above must hold.
   MergeSubpassGroups();

   // At this point we should have the final set of subpassGroups

   // Work out the final set of data now the groups are merged
   m_maxTileWidth = 1;
   m_maxTileHeight = 1;

   // maxBPP needs to be the same across all subpasses to get the correct tile sizes
   v3d_rt_bpp_t maxBPP = V3D_RT_BPP_32;

   for (auto &spg : m_subpassGroups)
   {
      // Tell each of the subpasses what group they belong to
      for (uint32_t sp = 0; sp < spg.m_numSubpasses; sp++)
      {
         auto &subpass = m_subpasses[sp + spg.m_firstSubpass];
         subpass.group = &spg;
      }

      // Work out what we can and store in the subpassGroup
      spg.m_maxRenderTargetBPP = V3D_RT_BPP_32;
      spg.m_multisampled       = false;
      spg.m_colorStoreMask     = 0;

      for (uint32_t rt = 0; rt < spg.m_numColorRenderTargets; rt++)
      {
         uint32_t rpIndex = spg.m_colorRTAttachments[rt];
         if (rpIndex == VK_ATTACHMENT_UNUSED)
            continue;

         // Build the final store mask
         if (spg.m_colorStoreOp[rt] == VK_ATTACHMENT_STORE_OP_STORE)
            spg.m_colorStoreMask |= (1 << rt);

         // Find maximum BPP across all active attachments & subpasses
         const V3D_RT_FORMAT_T& v3dRtFormat = m_attachments[rpIndex].v3dRtFormat;
         maxBPP = std::max(maxBPP, v3dRtFormat.bpp);
         spg.m_maxRenderTargetBPP = maxBPP;

         // From the Vulkan spec:
         // "If the subpass for which this pipeline is being created uses color and/or depth/stencil
         //  attachments, then rasterizationSamples must be the same as the sample count for those
         //  subpass attachments." - so all attachments in a group must have the same sample count.
         spg.m_multisampled = (pCreateInfo->pAttachments[rpIndex].samples != VK_SAMPLE_COUNT_1_BIT);
      }

      if (spg.m_dsAttachment != VK_ATTACHMENT_UNUSED)
         spg.m_multisampled = (pCreateInfo->pAttachments[spg.m_dsAttachment].samples != VK_SAMPLE_COUNT_1_BIT);

      if (spg.m_numColorRenderTargets > 0)
      {
         // TODO : double_buffer
         v3d_tile_size_pixels(&spg.m_tileWidth, &spg.m_tileHeight,
            spg.m_multisampled, /*double_buffer=*/false,
            spg.m_numColorRenderTargets, spg.m_maxRenderTargetBPP);
      }
      else
      {
         // Just depth, or no attachments at all
         spg.m_tileWidth  = V3D_MAX_TLB_WIDTH_PX;
         spg.m_tileHeight = V3D_MAX_TLB_HEIGHT_PX;
      }

      // We know the tile size for each subpassGroup, but it's only queried at the entire
      // renderPass level, so calculate the max of all groups
      m_maxTileWidth  = std::max(m_maxTileWidth, spg.m_tileWidth);
      m_maxTileHeight = std::max(m_maxTileHeight, spg.m_tileHeight);
   }
}

void RenderPass::MergeSubpassGroups()
{
   bool merged = false;
   do
   {
      for (uint32_t s = 0; s < m_subpassGroups.size() - 1; s++)
      {
         merged = m_subpassGroups[s].Merge(m_subpassGroups[s + 1]);
         if (merged)
            break;
      }
   }
   while (merged);
}

bool RenderPass::SubpassGroup::Merge(SubpassGroup &from)
{
   // TODO : This is where we optimize multiple subpasses into single groups.
   return false;
}

static const char *LoadOpStr(VkAttachmentLoadOp op)
{
   switch (op)
   {
   case VK_ATTACHMENT_LOAD_OP_LOAD  : return "LOAD";
   case VK_ATTACHMENT_LOAD_OP_CLEAR : return "CLEAR";
   default                          : return "DONT_CARE";
   }
}

static const char *StoreOpStr(VkAttachmentStoreOp op)
{
   switch (op)
   {
   case VK_ATTACHMENT_STORE_OP_STORE : return "STORE";
   default                           : return "DONT_CARE";
   }
}

void RenderPass::SubpassGroup::LogTrace() const
{
   if (log_trace_enabled())
   {
      log_trace("  Includes subpasses %u->%u", m_firstSubpass,
                        m_firstSubpass + m_numSubpasses - 1);
      log_trace("  preBarrier  = %u", m_preBarrier ? 1 : 0);
      log_trace("  sharedInOut = %u", m_sharesInputAndOutput ? 1 : 0);
      log_trace("  tileWidth   = %u", m_tileWidth);
      log_trace("  tileHeight  = %u", m_tileHeight);
      log_trace("  maxRenderTargetBPP = %u", 1 << (5 + m_maxRenderTargetBPP));
      log_trace("  multisampled = %u", m_multisampled ? 1 : 0);
      log_trace("  depthStencilAttachment = 0x%X", m_dsAttachment);
      log_trace("  numColorRenderTargets  = %u", m_numColorRenderTargets);
      for (uint32_t i = 0; i < m_numColorRenderTargets; i++)
      {
         if (m_colorRTAttachments[i] == VK_ATTACHMENT_UNUSED)
            continue;
         log_trace("    colorRTAttachment[%u] = 0x%X", i, m_colorRTAttachments[i]);
         log_trace("      colorLoad[%u]      = %s", i, LoadOpStr(m_colorLoadOp[i]));
         log_trace("      colorStore[%u]     = %s", i, StoreOpStr(m_colorStoreOp[i]));
         log_trace("      resolveTo[%u]      = 0x%X", i, m_resolveRTAttachments[i]);
         log_trace("      doInitialClear[%u] = %u", i, m_doInitialClear[i] ? 1 : 0);
      }
      log_trace("  doInitialDSClear = %u", m_doDSInitialClear ? 1 : 0);
      log_trace("  depthLoad        = %s", LoadOpStr(m_depthLoadOp));
      log_trace("  depthStore       = %s", StoreOpStr(m_depthStoreOp));
      log_trace("  stencilLoad      = %s", LoadOpStr(m_stencilLoadOp));
      log_trace("  stencilStore     = %s", StoreOpStr(m_stencilStoreOp));
      log_trace("  colorStoreMask   = 0x%X", m_colorStoreMask);
   }
}

void RenderPass::LogSubpassGroups() const
{
   if (log_trace_enabled())
   {
      uint32_t s = 0;
      for (auto &spg : m_subpassGroups)
      {
         log_trace("SubpassGroup[%u]", s);
         spg.LogTrace();
         s++;
      }
   }
}

void RenderPass::GetRenderAreaGranularity(
   bvk::Device *device,
   VkExtent2D  *pGranularity) noexcept
{
   pGranularity->width  = m_maxTileWidth;
   pGranularity->height = m_maxTileHeight;
}

bool RenderPass::EarlyZCompatible() const
{
   // A render pass is early z compatible if it clears the depth buffer before first use
   for (auto &spg : m_subpassGroups)
   {
      if (spg.m_hasDepth && spg.m_doDSInitialClear)
         return true;
   }
   return false;
}

}
