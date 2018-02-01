/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "CommandBufferBuilder.h"
#include "CommandBuffer.h"
#include "Command.h"
#include "ImageView.h"
#include "Image.h"
#include "Framebuffer.h"
#include "Options.h"
#include "LinkResult.h"
#include "Viewport.h"

#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/v3d/v3d_gen.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/khrn/glsl/glsl_tex_params.h"

#include <functional>
#include <algorithm>
#include <limits>
#include "DriverLimits.h"

namespace bvk {

void CommandBufferBuilder::CurRenderPassState::SetRenderPassBeginInfo(const VkRenderPassBeginInfo *beginInfo)
{
   framebuffer = fromHandle<Framebuffer>(beginInfo->framebuffer);
   renderPass  = fromHandle<RenderPass>(beginInfo->renderPass);
   renderArea  = beginInfo->renderArea;

   // Create a clear value entry for each possible attachment.
   // Note : the list in pClearValues may not cover the entire set of attachments, or may be larger.
   uint32_t numAtts = renderPass->Attachments().size();

   clearValues.resize(std::max(numAtts, beginInfo->clearValueCount));

   for (uint32_t i = 0; i < beginInfo->clearValueCount; i++)
      clearValues[i] = beginInfo->pClearValues[i];
   for (uint32_t i = beginInfo->clearValueCount; i < numAtts; i++)
      clearValues[i] = {};

   VkExtent3D fbDims;
   framebuffer->Dimensions(&fbDims);

   // TODO : factor in tile size when we have selective tile lists
   specialCaseRenderArea = renderArea.offset.x != 0 || renderArea.offset.y != 0 ||
                           renderArea.extent.width != fbDims.width ||
                           renderArea.extent.height != fbDims.height;

   needRectBasedClears = specialCaseRenderArea || fbDims.depth > 1;
}

CommandBufferBuilder::CommandBufferBuilder(
      const VkAllocationCallbacks *pCallbacks, Device *device, CommandBuffer *cmdBuf):
   ControlListBuilder(pCallbacks, cmdBuf),
   m_device(device),
   m_curState(device),
   m_curRenderPassState(pCallbacks),
   m_tlbAttachmentInfo(GetObjScopeAllocator<TLBAttachmentInfo>()),
   m_subpasses(GetObjScopeAllocator<SubPass>()),
   m_queryManager(cmdBuf, this, pCallbacks),
   m_delayedPostCommands(GetObjScopeAllocator<Command*>())
{
}

void CommandBufferBuilder::GetTLBDataForAttachment(
   uint32_t             attachmentIndex,
   TLBAttachmentInfo   *out
)
{
   if (attachmentIndex != VK_ATTACHMENT_UNUSED)
   {
      // Find the format of the attachment and convert to v3d types
      assert(m_curRenderPassState.framebuffer != nullptr);

      VkImageView  vkImgView = m_curRenderPassState.framebuffer->Attachments()[attachmentIndex];
      ImageView   *imgView   = fromHandle<ImageView>(vkImgView);
      imgView->GetTLBParams(&out->ldstParams);

      out->layerStride    = imgView->GetLayerStride();
      out->isMultisampled = imgView->GetSamples() != VK_SAMPLE_COUNT_1_BIT;
   }
}

void CommandBufferBuilder::GatherTLBInfo()
{
   // Run over all the attachments and calculate the information that we need for the
   // control lists
   uint32_t i = 0;
   for (auto &attInfo : m_tlbAttachmentInfo)
   {
      GetTLBDataForAttachment(i, &attInfo);
      i++;
   }
}

void CommandBufferBuilder::AllocateLazyMemory(uint32_t attIndex)
{
   VkImageView  vkImgView = m_curRenderPassState.framebuffer->Attachments()[attIndex];
   ImageView   *imgView = fromHandle<ImageView>(vkImgView);

   if (imgView->HasUnallocatedLazyMemory())
   {
      // Allocate the lazy memory
      imgView->AllocateLazyMemory();

      // Update the attachment info to include the new physAddr
      GetTLBDataForAttachment(attIndex, &m_tlbAttachmentInfo[attIndex]);
   }
}

void CommandBufferBuilder::BeginRenderPass(
   const VkRenderPassBeginInfo   *pRenderPassBegin,
   VkSubpassContents              contents) noexcept
{
   assert(!m_inRenderPass);

   m_curRenderPassState.SetRenderPassBeginInfo(pRenderPassBegin);

   auto &renderPass = *m_curRenderPassState.renderPass;
   auto &frambuffer = m_curRenderPassState.framebuffer;

   m_tlbAttachmentInfo.clear(); // Ensure we get newly created objects in the vector
   m_tlbAttachmentInfo.resize(renderPass.Attachments().size());

   // Record the fact that we are now in a render pass
   m_inRenderPass = true;
   m_curSubpassIndex = 0;

   // Which group is the subpass in?
   m_curSubpassGroup = renderPass.GroupForSubpass(m_curSubpassIndex);

   // Ensure our subpass vector is the right size
   m_subpasses.clear(); // Ensure we get newly created objects in the vector
   m_subpasses.resize(renderPass.Subpasses().size());
   m_curSubpass = &m_subpasses[m_curSubpassIndex];
   m_curSubpassGroup = renderPass.GroupForSubpass(m_curSubpassIndex);

   // Get frambuffer dimensions
   VkExtent3D  fbDims;
   frambuffer->Dimensions(&fbDims);

   m_numPixelsX = fbDims.width;
   m_numPixelsY = fbDims.height;

   // Gather some v3d specific data for each attachment
   GatherTLBInfo();

   BeginSubpass();
}

void CommandBufferBuilder::DrawClearRectForAttachmentIfNeeded(uint32_t rpIndex, uint32_t rt)
{
   if (rpIndex == VK_ATTACHMENT_UNUSED)
      return;

   const RenderPass::Attachment& attachment = m_curRenderPassState.renderPass->Attachments()[rpIndex];
   if (attachment.vkClearRectAspectMask)
   {
      ImageView *imgView = fromHandle<ImageView>(m_curRenderPassState.framebuffer->Attachments()[rpIndex]);
      auto &srr = imgView->GetSubresourceRange();

      VkClearAttachment ca;
      ca.aspectMask      = attachment.vkClearRectAspectMask;
      ca.colorAttachment = rpIndex;
      ca.clearValue      = m_curRenderPassState.clearValues[rpIndex];

      m_cmdBuf->DrawClearRect(rt, ca, m_curRenderPassState.renderArea, attachment.format, srr);
   }
}

void CommandBufferBuilder::BeginSubpass()
{
   // Work out num tiles for this subpass
   m_numTilesX = (m_numPixelsX + (m_curSubpassGroup->m_tileWidth - 1)) / m_curSubpassGroup->m_tileWidth;
   m_numTilesY = (m_numPixelsY + (m_curSubpassGroup->m_tileHeight - 1)) / m_curSubpassGroup->m_tileHeight;

   // If the renderArea doesn't cover the entire framebuffer we may have to insert
   // manual clearRect draws to replace the implicit clears since we need to load
   // the current attachment in order to preserve pixels outside the renderArea.
   if (m_curRenderPassState.needRectBasedClears)
   {
      for (uint32_t rt = 0; rt < m_curSubpassGroup->m_numColorRenderTargets; rt++)
         if (m_curSubpassGroup->m_doInitialClear[rt]) // Only clear in the group where first used
            DrawClearRectForAttachmentIfNeeded(m_curSubpassGroup->m_colorRTAttachments[rt], rt);

      if (m_curSubpassGroup->m_doDSInitialClear) // Only clear in the group where first used
         DrawClearRectForAttachmentIfNeeded(m_curSubpassGroup->m_dsAttachment, 0);
   }
#if !V3D_HAS_GFXH1461_FIX
   else if (m_curSubpassGroup->m_workaroundGFXH1461)
   {
      // Loading depth and clearing stencil, or vice-versa. Need to use
      // draw rect clear to avoid GFXH-1461...
      DrawClearRectForAttachmentIfNeeded(m_curSubpassGroup->m_dsAttachment, 0);
   }
#endif
}

void CommandBufferBuilder::NextSubpass(VkSubpassContents contents) noexcept
{
   // Get next subpass
   m_curSubpass = &m_subpasses[++m_curSubpassIndex];
   m_curSubpassGroup = m_curRenderPassState.renderPass->GroupForSubpass(m_curSubpassIndex);

   BeginSubpass();
}

void CommandBufferBuilder::EndRenderPass() noexcept
{
   m_inRenderPass = false;
}

void CommandBufferBuilder::BeginSecondaryCommandBuffer(
   const VkCommandBufferInheritanceInfo *inheritanceInfo) noexcept
{
   // Used only by inRenderPass secondaries
   m_inRenderPass = true;

   // Note: framebuffer is allowed to be NULL here
   m_curRenderPassState.framebuffer = fromHandle<Framebuffer>(inheritanceInfo->framebuffer);
   m_curRenderPassState.renderPass  = fromHandle<RenderPass>(inheritanceInfo->renderPass);
   m_curSubpassIndex                = inheritanceInfo->subpass;

   auto &renderPass = *m_curRenderPassState.renderPass;

   // Ensure our subpass vector is the right size
   m_subpasses.clear(); // Ensure we get newly created objects in the vector
   m_subpasses.resize(renderPass.Subpasses().size());

   m_curSubpass = &m_subpasses[m_curSubpassIndex];
   m_curSubpassGroup = renderPass.GroupForSubpass(m_curSubpassIndex);
}

bool CommandBufferBuilder::NeedToLoad(VkAttachmentLoadOp loadOp) const
{
   // We need to load an attachment if it was explicitly requested, or if
   // it we aren't rendering the entire area.
   return loadOp == VK_ATTACHMENT_LOAD_OP_LOAD || m_curRenderPassState.specialCaseRenderArea;
}

// Create master bin control list
void CommandBufferBuilder::CreateBinnerControlList(
   CmdBinRenderJobObj *brJob,
   v3d_barrier_flags   syncFlags)
{
   // uint32_t maxCores = Options::binSubjobs; TODO - multi-core

   // Record the start of the MCL bin list
   ControlList binList;

   StartBinJobCL(
      brJob, &binList,
      std::max(1u, CurSubpassGroup()->m_numColorRenderTargets),
      CurSubpassGroup()->m_multisampled,
      CurSubpassGroup()->m_maxRenderTargetBPP);

   // Branch to each subpass control list in the group in turn
   //
   // TODO: figure out any Z Prepasses for this sub-pass and set
   //       brJob->m_numZPrepassBins
   for (uint32_t i = 0; i < CurSubpassGroup()->m_numSubpasses; i++)
   {
      const auto &subpass = m_subpasses[CurSubpassGroup()->m_firstSubpass + i];
      if (subpass.controlList.Start() != 0)
         v3d_cl_branch_sub(CLPtr(), subpass.controlList.Start());
   }

   EndBinJobCL(brJob, &binList, syncFlags);
}

void CommandBufferBuilder::InsertDepthStencilClearValues()
{
   // Depth and stencil clear values
   float             depthClear = 0.0f;
   uint32_t          stencilClear = 0;
   v3d_depth_type_t  depthType = V3D_DEPTH_TYPE_32F;
   uint32_t          dsAttachmentIndex = CurSubpassGroup()->m_dsAttachment;

   if (dsAttachmentIndex != VK_ATTACHMENT_UNUSED)
   {
      const VkClearDepthStencilValue &ds = m_curRenderPassState.clearValues[dsAttachmentIndex].depthStencil;
      depthClear = ds.depth;
      stencilClear = ds.stencil & 0xFF;
      depthType = m_curRenderPassState.renderPass->Attachments()[dsAttachmentIndex].v3dDepthType;
   }

   // HW needs this to be last V3D_CL_TILE_RENDERING_MODE_CFG sub item
   v3d_cl_tile_rendering_mode_cfg_zs_clear_values(CLPtr(),
         stencilClear, v3d_snap_depth(depthClear, depthType));
}

// Create master render control list
void CommandBufferBuilder::CreateRenderControlList(
   CmdBinRenderJobObj *brJob,
   const ControlList  &genTileList,
   v3d_barrier_flags   syncFlags)
{
   uint32_t core = 0; // TODO - multi core

   ControlList renderList;

   constexpr bool doubleBuffer = false; // TODO - double buffer
   constexpr bool earlyDSClear = false; // TODO - early DS clear

   StartRenderJobCL(brJob, &renderList);

   // Output the rendering config
   InsertRenderTargetCfg(doubleBuffer, earlyDSClear);

   // Clear values
   InsertColorClearValues();
   InsertDepthStencilClearValues();

   bool clearRTs = CurSubpassGroup()->m_numColorRenderTargets > 0;

   // Initial clears
   InsertInitialTLBClear(doubleBuffer, clearRTs, !earlyDSClear);

   EndRenderJobCL(brJob, &renderList, genTileList, core, syncFlags);
}

// Start a new control list
void CommandBufferBuilder::NeedControlList()
{
   if (m_curSubpass->controlList.Start() == 0)
   {
      SetCurrentControlList(&m_curSubpass->controlList);
      m_curSubpass->controlList.SetStart(*m_curDeviceBlock);
   }
}

void CommandBufferBuilder::RestartControlList()
{
   SetCurrentControlList(&m_curSubpass->controlList);
   m_curSubpass->controlList.SetStart(*m_curDeviceBlock);
}

// Add a final return and update sync list
void CommandBufferBuilder::FinalizeControlList()
{
   if (m_curSubpass && m_curSubpass->controlList.Start() != 0)
   {
      // Close the previous control list
      v3d_cl_return(CLPtr());
      m_curSubpass->controlList.SetEnd(*m_curDeviceBlock);
      SetCurrentControlList(nullptr);

      // Ensure the next control list will get valid state written
      m_curState.SetAllDirty();
   }
}

// Take all the data we gathered during pipeline construction and turn it
// into real device memory based shader record, uniforms and attributes.
void CommandBufferBuilder::WriteGraphicsShaderRecord()
{
   const GraphicsPipeline *pipe = m_curState.BoundGraphicsPipeline();
   const LinkResult       &linkData = pipe->GetLinkResult();

   // Make a devMem copy of the shader record that belongs to this command buffer
   DevMemRange shadrecMem;
   m_cmdBuf->NewDevMemRange(&shadrecMem, pipe->GetShaderRecord().size() * sizeof(uint32_t),
                            V3D_SHADREC_ALIGN);
   uint32_t *shadrecPtr = static_cast<uint32_t*>(shadrecMem.Ptr());

   // Copy the shader record. We will need to patch bits of it up.
   const uint32_t *shadrecSysMemPtr = pipe->GetShaderRecord().data();
   memcpy(shadrecPtr, shadrecSysMemPtr, pipe->GetShaderRecord().size() * sizeof(uint32_t));

   // Now make and patch devMem copies of the uniform buffers
   GraphicsUniformBufferData unifBufs;
   CopyAndPatchGraphicsUniformBuffers(unifBufs, *pipe);

   // Patch TnG uniform addresses
   PatchTNGUniformAddresses(shadrecSysMemPtr, shadrecPtr, unifBufs);

   // Patch the main shader record (unpack from sysMem, patch, pack to devMem)
   PatchMainShadrecUniformAddresses(shadrecSysMemPtr, shadrecPtr, unifBufs);

   // Patch in the attribute addresses
   PatchMainShadrecAttributeAddressesAndMaxIndex(shadrecSysMemPtr, shadrecPtr);

   // Choose which control list function to use
   std::function<void(uint8_t**, uint32_t, v3d_addr_t)> clShaderRecFn;
   if (linkData.m_hasGeom && linkData.m_hasTess)
      clShaderRecFn = v3d_cl_gl_tg_shader;
   else if (linkData.m_hasGeom)
      clShaderRecFn = v3d_cl_gl_g_shader;
   else if (linkData.m_hasTess)
      clShaderRecFn = v3d_cl_gl_t_shader;
   else
      clShaderRecFn = v3d_cl_gl_shader;

   // Add the shader record to the control list
   uint32_t numAttrs = gfx_umax(1u, linkData.m_attrCount);  // Zero attrs not allowed
   clShaderRecFn(CLPtr(), numAttrs, shadrecMem.Phys());
}

void CommandBufferBuilder::DisableOcclusionQuery()
{
   if (m_curControlList != nullptr)
   {
      // Turn off counting, and ensure the next draw turns it on again if needed
      v3d_cl_occlusion_query_counter_enable(CLPtr(), 0);
      m_curState.SetDirty(CmdBufState::eOcclusionCounter);
   }
}

void CommandBufferBuilder::CopyAndPatchGraphicsUniformBuffers(
   GraphicsUniformBufferData &unifBufs,
   const GraphicsPipeline &pipe)
{
   for (unsigned m = 0; m != MODE_COUNT; ++m)
   {
      for (unsigned s = 0; s != LinkResult::SHADER_VPS_COUNT; ++s)
      {
         CopyAndPatchUniformBuffer(
            unifBufs.vpUniformMem[s][m],
            LinkResult::ConvertShaderFlavour(s),
            pipe,
            pipe.GetVPSUniforms((LinkResult::eVertexPipeStage)s, (shader_mode)m));
      }
   }
   CopyAndPatchUniformBuffer(unifBufs.fsUniformMem, SHADER_FRAGMENT, pipe, pipe.GetFSUniforms());
}

void CommandBufferBuilder::PatchMainShadrecUniformAddresses(
   const uint32_t                   *srcPtr,
   uint32_t                         *dstPtr,
   const GraphicsUniformBufferData  &unifBufs)
{
   // Patch the main shader record (unpack from sysMem, patch, pack to devMem)
   const GraphicsPipeline *pipe = m_curState.BoundGraphicsPipeline();

   V3D_SHADREC_GL_MAIN_T srec;
   v3d_unpack_shadrec_gl_main(&srec, srcPtr);

   // TODO : would be nice if we could do this without unpacking and repacking
   // the record. Any way we can just patch the required words?

   assert(srec.fs.unifs_addr == pipe->UNIFORM_PATCHTAG);
   srec.fs.unifs_addr = unifBufs.fsUniformMem.Phys();

   assert(srec.vs.unifs_addr == pipe->UNIFORM_PATCHTAG);
   srec.vs.unifs_addr = unifBufs.vpUniformMem[LinkResult::SHADER_VPS_VS][MODE_RENDER].Phys();

   assert(srec.cs.unifs_addr == pipe->UNIFORM_PATCHTAG);
   srec.cs.unifs_addr = unifBufs.vpUniformMem[LinkResult::SHADER_VPS_VS][MODE_BIN].Phys();

   v3d_pack_shadrec_gl_main(dstPtr, &srec);
}

void CommandBufferBuilder::PatchMainShadrecAttributeAddressesAndMaxIndex(
   const uint32_t          *srcPtr,
   uint32_t                *dstPtr)
{
   const GraphicsPipeline *pipe = m_curState.BoundGraphicsPipeline();
   const LinkResult       &linkData = pipe->GetLinkResult();

   for (uint32_t n = 0; n < linkData.m_attrCount; n++)
   {
      uint32_t attrIndex  = linkData.m_attr[n].idx;

      const auto &attDesc = pipe->GetVertexAttributeDesc(attrIndex);
      const auto &vBuf    = m_cmdBuf->CurState().BoundVertexBuffer(attDesc.binding);

      v3d_addr_t physAddr = vBuf.CalculateBufferOffsetAddr(attDesc.offset);

      // Unpack, patch and repack attribute record
      V3D_SHADREC_GL_ATTR_T attr;
      size_t                offset = pipe->GetShaderPatchingData().offsetGLAttr +
                                     V3D_SHADREC_GL_ATTR_PACKED_SIZE / sizeof(uint32_t) * n;

      v3d_unpack_shadrec_gl_attr(&attr, srcPtr + offset);
      assert(attr.addr == 0);
      attr.addr = physAddr;

      uint32_t maxIndex;
      const auto &bindDesc = pipe->GetVertexBindingDesc(attDesc.binding);
      if (!vBuf.CalcMaxIndex(maxIndex, attDesc, bindDesc.stride))
         throw bvk::nothing_to_do();
      assert(attr.max_index == 0);
      attr.max_index = gfx_umin(maxIndex, V3D_VCD_MAX_INDEX);

      v3d_pack_shadrec_gl_attr(dstPtr + offset, &attr);
   }
}

uint32_t CommandBufferBuilder::UniformPatcher::CalcBufferAddress(uint32_t value, bool ssbo) const
{
   uint32_t descTableIndex = value & DriverLimits::eBufferTableIndexMask;
   uint32_t byteOffset = (value >> DriverLimits::eBufferTableIndexBits) & DriverLimits::eBufferOffsetMask;

   // Get the descriptor info from the table in the linkResult
   const DescriptorInfo &descInfo = m_descTables.GetBuffer(ssbo, descTableIndex);

   // Get the physical address of the UBO from the bound descriptor set
   auto &ds = m_descSetBindings[descInfo.descriptorSet];

   uint32_t   dynamicOffset = ds.GetDynamicOffset(descInfo.bindingPoint, descInfo.element);
   uint32_t   range = ds.descriptorSet->GetBufferRange(descInfo.bindingPoint, descInfo.element);
   uint32_t   size  = ds.descriptorSet->GetBufferSize(descInfo.bindingPoint, descInfo.element);
   v3d_addr_t base  = ds.descriptorSet->GetBufferAddress(descInfo.bindingPoint, descInfo.element);
   v3d_addr_t addr;

   if (byteOffset >= range || dynamicOffset + byteOffset >= size)
      addr = base;
   else
      addr = base + dynamicOffset + byteOffset;

   if (addr < base)  // In case of overflow
      addr = base;

   return addr;
}

uint32_t CommandBufferBuilder::UniformPatcher::CalcPushConstantAddress(uint32_t value,
                                                   const DevMemRange &pcsDevMem) const
{
   uint32_t byteOffset = (value >> DriverLimits::eBufferTableIndexBits) & DriverLimits::eBufferOffsetMask;

   if (byteOffset >= pcsDevMem.Size())
      byteOffset = 0;

   return pcsDevMem.Phys() + byteOffset;
}

uint32_t CommandBufferBuilder::UniformPatcher::CalcBufferUsableSpace(uint32_t value, bool ssbo,
                                                                     const CommandBufferBuilder &builder) const
{
   uint32_t descTableIndex = value & DriverLimits::eBufferTableIndexMask;
   uint32_t byteOffset     = (value >> DriverLimits::eBufferTableIndexBits) & DriverLimits::eBufferOffsetMask;
   uint32_t dynamicOffset  = 0;
   uint32_t range          = 0;
   uint32_t size           = 0;

   if (descTableIndex == DriverLimits::ePushConstantIdentifier && !ssbo)
   {
      size  = builder.PushConstants().size();
      range = size;
   }
   else
   {
      // Get the descriptor info from the table in the linkResult
      const DescriptorInfo &descInfo = m_descTables.GetBuffer(ssbo, descTableIndex);

      auto &ds = m_descSetBindings[descInfo.descriptorSet];

      dynamicOffset = ds.GetDynamicOffset(descInfo.bindingPoint, descInfo.element);
      range = ds.descriptorSet->GetBufferRange(descInfo.bindingPoint, descInfo.element);
      size  = ds.descriptorSet->GetBufferSize(descInfo.bindingPoint, descInfo.element);
   }

   if (byteOffset >= range || dynamicOffset + byteOffset >= size)
      return 0;

   return range - byteOffset;
}

const DescriptorInfo &CommandBufferBuilder::UniformPatcher::GetTMUParamSamplerDescriptorSetInfo(uint32_t uValue) const
{
   // Sampler
   uint32_t samplerTableIndex = backend_uniform_get_sampler(uValue) & DriverLimits::eSamplerTableIndexMask;
   return m_descTables.GetSampler(samplerTableIndex);
}

const DescriptorInfo &CommandBufferBuilder::UniformPatcher::GetIMGParamImageDescriptorSetInfo(uint32_t uValue) const
{
   // Sampled or Storage image
   uint32_t imageTableIndex = backend_uniform_get_sampler(uValue) & DriverLimits::eImageTableIndexMask;
   return m_descTables.GetImage(imageTableIndex);
}

uint32_t CommandBufferBuilder::UniformPatcher::GetTextureNumLevels(uint32_t uValue) const
{
   const DescriptorInfo &descInfo = m_descTables.GetImage(uValue);
   auto &ds = m_descSetBindings[descInfo.descriptorSet];
   return ds.descriptorSet->GetNumLevels(descInfo.bindingPoint, descInfo.element);
}

VkExtent3D CommandBufferBuilder::UniformPatcher::GetTextureBaseExtent(uint32_t uValue) const
{
   const DescriptorInfo &descInfo = m_descTables.GetImage(uValue);
   auto &ds = m_descSetBindings[descInfo.descriptorSet];
   return ds.descriptorSet->GetTextureSize(descInfo.bindingPoint, descInfo.element);
}

void CommandBufferBuilder::SetPushConstants(uint32_t bytesRequired, uint32_t offset,
                                            uint32_t size, const void *pValues)
{
   // Resize pushConstant buffer if needed
   if (bytesRequired > m_pushConstants.size())
      m_pushConstants.resize(bytesRequired);

   assert(offset + size <= m_pushConstants.size());

   memcpy(m_pushConstants.data() + offset, pValues, size);

   m_pushConstantsDirty = true;
}

DevMemRange &CommandBufferBuilder::UpdateDynamicPushConstantBlock()
{
   DevMemRange &pcsDevMem = PushConstantsDevMem();

   if (m_pushConstantsDirty)
   {
      // We need a new PushConstant devMem region
      const bvk::vector<uint8_t> &pcs = PushConstants();

      // This will replace the current-push-constant-device-mem block in the curState with a new one
      // The old one will remain in device memory until the command buffer is destroyed.
      m_cmdBuf->NewDevMemRange(&pcsDevMem, pcs.size(), /*align=*/V3D_TMU_GENERAL_CONSERVATIVE_ALIGN);
      uint8_t *pcPtr = static_cast<uint8_t*>(pcsDevMem.Ptr());
      memcpy(pcPtr, pcs.data(), pcs.size());
      pcsDevMem.SyncMemory();

      m_pushConstantsDirty = false;
   }

   return pcsDevMem;
}

void CommandBufferBuilder::CopyAndPatchUniformBuffer(
   DevMemRange                  &uniformMem,
   ShaderFlavour                 shaderFlavour,
   const Pipeline               &pipe,
   const Pipeline::UniformData  &uniforms)
{
   if (uniforms.defaults.empty())
      return;

   // Include enough room for read-ahead
   // TODO : It would be nicer to have the read-ahead space just at the end of the
   // block, rather than in each allocation, but that's a more complex solution.
   v3d_size_t size = uniforms.defaults.size() * sizeof(Pipeline::Uniform);

   m_cmdBuf->NewDevMemRange(&uniformMem, size + V3D_MAX_QPU_UNIFS_READAHEAD, V3D_QPU_UNIFS_ALIGN);
   Pipeline::Uniform *uniformsPtr = static_cast<Pipeline::Uniform*>(uniformMem.Ptr());
   memcpy(uniformsPtr, uniforms.defaults.data(), size);

   UniformPatcher patcher(
      pipe.GetLinkResult().m_descriptorTables[shaderFlavour],
      m_curState.BoundDescriptorSetArray(pipe.GetBindPoint()));

   const Viewport *vp = nullptr;
   if (pipe.GetBindPoint() == VK_PIPELINE_BIND_POINT_GRAPHICS)
      vp = &m_curState.GetViewport();

   for (const auto &patch: uniforms.patches)
   {
      Pipeline::Uniform *ptr = uniformsPtr + patch.offset;

      switch (patch.uniformFlavour)
      {
      case BACKEND_UNIFORM_SPECIAL:
         switch ((BackendSpecialUniformFlavour)patch.value)
         {
         case BACKEND_SPECIAL_UNIFORM_VP_SCALE_X:
            ptr->f = vp->internalScale[0];
            break;
         case BACKEND_SPECIAL_UNIFORM_VP_SCALE_Y:
            ptr->f = vp->internalScale[1];
            break;
         case BACKEND_SPECIAL_UNIFORM_VP_OFFSET_Z:
         case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_NEAR:
            ptr->f = vp->depthNear;
            break;
         case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_FAR:
            ptr->f = vp->depthFar;
            break;
         case BACKEND_SPECIAL_UNIFORM_VP_SCALE_Z:
         case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_DIFF:
            ptr->f = vp->depthDiff;
            break;
         default:
            unreachable();
         }
         break;

      case BACKEND_UNIFORM_PLAIN:
         ptr->u = GetPushConstantValue(patch.value);
         break;
      case BACKEND_UNIFORM_UBO_ADDRESS:
         if ((patch.value & DriverLimits::eBufferTableIndexMask) == DriverLimits::ePushConstantIdentifier)
         {
            auto &pcsDevMem = UpdateDynamicPushConstantBlock();
            ptr->u = patcher.CalcPushConstantAddress(patch.value, pcsDevMem);
         }
         else
            ptr->u = patcher.CalcBufferAddress(patch.value, /*ssbo=*/false);
         break;
      case BACKEND_UNIFORM_SSBO_ADDRESS:
         ptr->u = patcher.CalcBufferAddress(patch.value, /*ssbo=*/true);
         break;
      case BACKEND_UNIFORM_SSBO_SIZE:
         ptr->u = patcher.CalcBufferUsableSpace(patch.value, /*ssbo=*/true, *this);
         break;
      case BACKEND_UNIFORM_UBO_SIZE:
         ptr->u = patcher.CalcBufferUsableSpace(patch.value, /*ssbo=*/false, *this);
         break;
      case BACKEND_UNIFORM_TEX_PARAM0:  // Sampled image
      case BACKEND_UNIFORM_IMG_PARAM0:  // Storage image
         ptr->u = patcher.AssembleIMGParam0(patch.value);
         break;
      case BACKEND_UNIFORM_TEX_PARAM1:  // Sampler
         ptr->u = patcher.AssembleTMUParam1(patch.value);
         break;
      case BACKEND_UNIFORM_TEX_SIZE_X:
      case BACKEND_UNIFORM_IMG_SIZE_X:
         ptr->u = patcher.GetTextureBaseExtent(patch.value).width;
         break;
      case BACKEND_UNIFORM_TEX_SIZE_Y:
      case BACKEND_UNIFORM_IMG_SIZE_Y:
         ptr->u = patcher.GetTextureBaseExtent(patch.value).height;
         break;
      case BACKEND_UNIFORM_TEX_SIZE_Z:
      case BACKEND_UNIFORM_IMG_SIZE_Z:
         ptr->u = patcher.GetTextureBaseExtent(patch.value).depth;
         break;
      case BACKEND_UNIFORM_TEX_LEVELS:
         ptr->u = patcher.GetTextureNumLevels(patch.value);
         break;
      case BACKEND_UNIFORM_ATOMIC_ADDRESS: // Don't exist in Vulkan
      default:
         unreachable();
      }
   }
}

void CommandBufferBuilder::InsertRenderTargetCfg(bool doubleBuffer, bool earlyDSClear)
{
   const auto &renderPass = *m_curRenderPassState.renderPass;

   // Get framebuffer size
   VkExtent3D fbDims;
   m_curRenderPassState.framebuffer->Dimensions(&fbDims);

   uint32_t          dsAttachmentIndex = CurSubpassGroup()->m_dsAttachment;
   v3d_depth_type_t  depthType = V3D_DEPTH_TYPE_32F;
   if (dsAttachmentIndex != VK_ATTACHMENT_UNUSED)
      depthType = renderPass.Attachments()[dsAttachmentIndex].v3dDepthType;

   // Output the rendering config
   v3d_cl_tile_rendering_mode_cfg_common(CLPtr(),
      std::max(1U, m_curSubpassGroup->m_numColorRenderTargets),
      fbDims.width,
      fbDims.height,
      m_curSubpassGroup->m_maxRenderTargetBPP,
      m_curSubpassGroup->m_multisampled,
      doubleBuffer,
      /*coverage=*/false,
      V3D_EZ_DIRECTION_LT_LE,    // TODO - earlyZ
      /*earlyZDisable=*/true,    // TODO - earlyZ
      depthType,
      earlyDSClear
   );

   V3D_CL_TILE_RENDERING_MODE_CFG_T i = {};
   i.type = V3D_RCFG_TYPE_COLOR;
   for (unsigned rt = 0; rt < CurSubpassGroup()->m_numColorRenderTargets; ++rt)
   {
      uint32_t att = CurSubpassGroup()->m_colorRTAttachments[rt];
      if (att != VK_ATTACHMENT_UNUSED)
         i.u.color.rt_formats[rt] = renderPass.Attachments()[att].v3dRtFormat;
   }
   v3d_cl_tile_rendering_mode_cfg_indirect(CLPtr(), &i);
}

void CommandBufferBuilder::InsertColorClearValues()
{
   auto &renderPass = *m_curRenderPassState.renderPass;

   // Color clear values
   for (unsigned rt = 0; rt < m_curSubpassGroup->m_numColorRenderTargets; ++rt)
   {
      uint32_t att = m_curSubpassGroup->m_colorRTAttachments[rt];
      if (att == VK_ATTACHMENT_UNUSED)
         continue;

      const auto &attData = renderPass.Attachments()[att];

      uint32_t clearColors[4] = {};
      memcpy(clearColors, m_curRenderPassState.clearValues[att].color.uint32, sizeof(clearColors));

      // If the render target does not have an alpha channel, we need alpha in
      // the TLB to be 1 to get correct blending.
      if (!gfx_lfmt_has_alpha(attData.format))
         clearColors[3] = gfx_float_to_bits(1.0f);

      v3d_cl_rcfg_clear_colors(CLPtr(), rt, clearColors, &attData.v3dRtFormat);
   }
}

void CommandBufferBuilder::AddStore(uint32_t rpIndex, v3d_ldst_buf_t buf, bool resolve)
{
   // Allocate lazy memory if we're going to store to it.
   // This may alter the physAddr in m_tlbAttachmentInfo[rpIndex].ldstParams
   AllocateLazyMemory(rpIndex);

   const v3d_tlb_ldst_params  &ls = m_tlbAttachmentInfo[rpIndex].ldstParams;
   v3d_decimate_t              decimate = ls.decimate;

   if (resolve && v3d_classify_ldst_buf(buf) == V3D_LDST_BUF_CLASS_COLOR &&
      v3d_pixel_format_supports_4x_decimate(ls.pixel_format))
   {
      // The pre-calculated LDST params for a non-MS target will contain
      // V3D_DECIMATE_SAMPLE0 by default. If we require a 4X resolve then we
      // must override that here.
      decimate = V3D_DECIMATE_4X;
   }

   v3d_cl_store(CLPtr(), buf, ls.memory_format, ls.flipy,
      ls.dither, decimate, ls.pixel_format, /*clear=*/false,
      ls.chan_reverse, ls.rb_swap,
      ls.stride, ls.flipy_height_px, ls.addr);
}

void CommandBufferBuilder::AddLoad(uint32_t rpIndex, v3d_ldst_buf_t buf)
{
   // Allocate lazy memory if we're going to load from it.
   // This may alter the physAddr in m_tlbAttachmentInfo[rpIndex].ldstParams
   AllocateLazyMemory(rpIndex);

   const v3d_tlb_ldst_params  &ls = m_tlbAttachmentInfo[rpIndex].ldstParams;

   v3d_cl_load(CLPtr(), buf, ls.memory_format, ls.flipy, ls.decimate, ls.pixel_format,
      ls.load_alpha_to_one, ls.chan_reverse, ls.rb_swap,
      ls.stride, ls.flipy_height_px, ls.addr);
}

void CommandBufferBuilder::AddTileListLoads()
{
   for (unsigned rt = 0; rt < m_curSubpassGroup->m_numColorRenderTargets; ++rt)
   {
      uint32_t rpIndex = m_curSubpassGroup->m_colorRTAttachments[rt];
      if (rpIndex == VK_ATTACHMENT_UNUSED)
         continue;

      if (m_forceRTLoad || NeedToLoad(m_curSubpassGroup->m_colorLoadOp[rt]))
         AddLoad(rpIndex, v3d_ldst_buf_color(rt));
   }

   bool loadDepth = NeedToLoad(m_curSubpassGroup->m_depthLoadOp);
   bool loadStencil = NeedToLoad(m_curSubpassGroup->m_stencilLoadOp);

   loadDepth = m_curSubpassGroup->m_hasDepth && (loadDepth || m_forceRTLoad);
   loadStencil = m_curSubpassGroup->m_hasStencil && (loadStencil || m_forceRTLoad);

   if (m_curSubpassGroup->m_dsAttachment != VK_ATTACHMENT_UNUSED && (loadDepth || loadStencil))
   {
      uint32_t       rpIndex = m_curSubpassGroup->m_dsAttachment;
      v3d_ldst_buf_t buf = v3d_ldst_buf_ds(loadDepth, loadStencil);

      AddLoad(rpIndex, buf);
   }
}

void CommandBufferBuilder::AddTileListStores()
{
   bool doneStore = false;
   bool colorClear = false;
   bool dsClear = false;

   // Note: we don't clear during store, since we may be storing to multiple locations
   for (unsigned rt = 0; rt < m_curSubpassGroup->m_numColorRenderTargets; ++rt)
   {
      uint32_t rpIndex = m_curSubpassGroup->m_colorRTAttachments[rt];
      if (rpIndex == VK_ATTACHMENT_UNUSED)
         continue;

      colorClear |= (m_curSubpassGroup->m_colorLoadOp[rt] == VK_ATTACHMENT_LOAD_OP_CLEAR);

      if (m_forceRTStore || m_curSubpassGroup->m_colorStoreOp[rt] == VK_ATTACHMENT_STORE_OP_STORE)
      {
         AddStore(rpIndex, v3d_ldst_buf_color(rt), /*resolve=*/false);
         doneStore = true;
      }

      // Do any multisample resolving
      uint32_t resolveTo = m_curSubpassGroup->m_resolveRTAttachments[rt];
      if (resolveTo != VK_ATTACHMENT_UNUSED)
      {
         AddStore(resolveTo, v3d_ldst_buf_color(rt), /*resolve=*/true);
      }
   }

   dsClear |= (m_curSubpassGroup->m_depthLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ||
      m_curSubpassGroup->m_stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR);

   bool storeDepth = m_curSubpassGroup->m_depthStoreOp == VK_ATTACHMENT_STORE_OP_STORE;
   bool storeStencil = m_curSubpassGroup->m_stencilStoreOp == VK_ATTACHMENT_STORE_OP_STORE;
   storeDepth = m_curSubpassGroup->m_hasDepth && (storeDepth || m_forceRTStore);
   storeStencil = m_curSubpassGroup->m_hasStencil && (storeStencil || m_forceRTStore);

   if (m_curSubpassGroup->m_dsAttachment != VK_ATTACHMENT_UNUSED && (storeDepth || storeStencil))
   {
      uint32_t rpIndex = m_curSubpassGroup->m_dsAttachment;
      v3d_ldst_buf_t buf = v3d_ldst_buf_ds(storeDepth, storeStencil);

      AddStore(rpIndex, buf, /*resolve=*/false);
      doneStore = true;
   }

   if (!doneStore)
   {
      // We must have at least one store - so add a dummy one
      v3d_cl_store(CLPtr(), V3D_LDST_BUF_NONE, V3D_MEMORY_FORMAT_RASTER, /*flipy=*/false,
         V3D_DITHER_OFF, V3D_DECIMATE_SAMPLE0, V3D_PIXEL_FORMAT_SRGB8_ALPHA8, /*clear=*/false,
         /*chan_reverse=*/false, /*rb_swap=*/false,
         /*stride=*/0, /*height=*/0, /*addr=*/0);
   }

   if (!m_forceRTStore && (colorClear || dsClear))
      v3d_cl_clear(CLPtr(), colorClear, dsClear);
}

void CommandBufferBuilder::PatchTNGUniformAddresses(
   const uint32_t                   *srcPtr,
   uint32_t                         *dstPtr,
   const GraphicsUniformBufferData  &unifBufs)
{
   const GraphicsPipeline *pipe = m_curState.BoundGraphicsPipeline();

   // Patch the uniform addresses in the T+G records
   for (unsigned s = LinkResult::SHADER_VPS_VS + 1; s <= LinkResult::SHADER_VPS_TES; ++s)
   {
      uint32_t offset = pipe->GetShaderPatchingData().offsetGLGeom[s][MODE_RENDER];
      if (offset != 0)
      {
         // Unpack from sysMem
         V3D_SHADREC_GL_GEOM_T stageShadrec;
         v3d_unpack_shadrec_gl_geom(&stageShadrec, srcPtr + offset);

         // TODO : would be nice if we could do this without unpacking and repacking
         // the record. Any way we can just patch the required words?

         assert(stageShadrec.gs_bin.unifs_addr == pipe->UNIFORM_PATCHTAG);
         stageShadrec.gs_bin.unifs_addr = unifBufs.vpUniformMem[s][MODE_BIN].Phys();

         assert(stageShadrec.gs_render.unifs_addr == pipe->UNIFORM_PATCHTAG);
         stageShadrec.gs_render.unifs_addr = unifBufs.vpUniformMem[s][MODE_RENDER].Phys();

         // Pack to devMem
         v3d_pack_shadrec_gl_geom(dstPtr + offset, &stageShadrec);
      }
   }
}

uint32_t CommandBufferBuilder::UniformPatcher::AssembleIMGParam0(uint32_t uValue) const
{
   // Sampled or Storage image
   const DescriptorInfo &descInfo = GetIMGParamImageDescriptorSetInfo(uValue);
   auto &ds = m_descSetBindings[descInfo.descriptorSet];
   uint32_t bits = ds.descriptorSet->GetImageParam(descInfo.bindingPoint, descInfo.element);
   uint32_t extraBits = backend_uniform_get_extra(uValue);
   assert(!(bits & extraBits));
   return (bits | extraBits);
}

uint32_t CommandBufferBuilder::UniformPatcher::AssembleTMUParam1(uint32_t uValue) const
{
   // Sampler
   const DescriptorInfo &descInfo = GetTMUParamSamplerDescriptorSetInfo(uValue);
   auto    &ds = m_descSetBindings[descInfo.descriptorSet];
   uint32_t bits = ds.descriptorSet->GetSamplerParam(descInfo.bindingPoint, descInfo.element);
   uint32_t extraBits = backend_uniform_get_extra(uValue);
   assert(!(bits & extraBits));
   return (bits | extraBits);
}

} // namespace bvk
