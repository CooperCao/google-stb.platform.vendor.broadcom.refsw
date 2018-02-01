/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "ControlListBuilder.h"
#include "RenderPass.h"
#include "LinkResult.h"
#include "QueryManager.h"

namespace bvk {

class Command;

// Transient command buffer state -
//   - used during construction of renderpass and subpass control lists
//     while recording command buffers
//   - used to construct the master control lists for a renderpass bin/render
//     job
class CommandBufferBuilder : public ControlListBuilder
{
public:
   struct CurRenderPassState : public Allocating
   {
      CurRenderPassState(const VkAllocationCallbacks *pCallbacks) :
         Allocating(pCallbacks),
         clearValues(GetObjScopeAllocator<VkClearValue>())
      {
      }

      void SetRenderPassBeginInfo(const VkRenderPassBeginInfo *beginInfo);

      RenderPass                *renderPass  = nullptr;
      Framebuffer               *framebuffer = nullptr;
      VkRect2D                   renderArea;
      bvk::vector<VkClearValue>  clearValues;
      bool                       specialCaseRenderArea = false;
      bool                       needRectBasedClears   = false;
   };

   struct SubPass
   {
      ControlList controlList;
   };

   struct TLBAttachmentInfo
   {
      v3d_tlb_ldst_params  ldstParams;
      uint32_t             layerStride;
      bool                 isMultisampled;
   };

private:
   struct GraphicsUniformBufferData
   {
      DevMemRange   vpUniformMem[LinkResult::SHADER_VPS_COUNT][MODE_COUNT];
      DevMemRange   fsUniformMem;
   };

public:
   CommandBufferBuilder(const VkAllocationCallbacks *pCallbacks, Device *device, CommandBuffer *cmdBuf);

   void GetTLBDataForAttachment(uint32_t attachmentIndex, TLBAttachmentInfo *out);
   void GatherTLBInfo();

   void BeginRenderPass(const VkRenderPassBeginInfo *pRenderPassBegin,
                        VkSubpassContents contents) noexcept;

   void NextSubpass(VkSubpassContents contents) noexcept;

   void EndRenderPass() noexcept;

   void BeginSecondaryCommandBuffer(const VkCommandBufferInheritanceInfo *inheritanceInfo) noexcept;

   void NeedControlList();
   void RestartControlList();
   void FinalizeControlList();

   void WriteGraphicsShaderRecord();

   void DisableOcclusionQuery();

   QueryManager *GetQueryManager() { return &m_queryManager; }

   const bvk::vector<uint8_t>   &PushConstants() const { return m_pushConstants; }
   DevMemRange &PushConstantsDevMem() { return m_pushConstantsDevMem; }

   void SetPushConstants(uint32_t bytesRequired, uint32_t offset, uint32_t size, const void *pValues);

   uint32_t GetPushConstantValue(uint32_t offset)
   {
      assert(offset + sizeof(uint32_t) <= m_pushConstants.size());
      return *reinterpret_cast<uint32_t*>(m_pushConstants.data() + offset);
   }

   void CopyAndPatchUniformBuffer(
      DevMemRange &uniformMem,
      ShaderFlavour shaderFlavour,
      const Pipeline &pipe,
      const Pipeline::UniformData &uniforms);

   const TLBAttachmentInfo &GetTLBAttachmentInfo(uint32_t attIndx) const
   {
      return m_tlbAttachmentInfo[attIndx];
   }

private:
   class UniformPatcher
   {
   public:
      UniformPatcher(const DescriptorTables &descTables, const CmdBufState::DescriptorSetBindings &descSetBindings)
         :  m_descTables(descTables), m_descSetBindings(descSetBindings) {}

      uint32_t CalcBufferAddress(uint32_t value, bool ssbo) const;
      uint32_t CalcPushConstantAddress(uint32_t value, const DevMemRange &pcsDevMem) const;
      uint32_t CalcBufferUsableSpace(uint32_t value, bool ssbo, const CommandBufferBuilder &builder) const;
      uint32_t GetTextureNumLevels(uint32_t uValue) const;
      VkExtent3D GetTextureBaseExtent(uint32_t uValue) const;

      uint32_t AssembleIMGParam0(uint32_t uValue) const;
      uint32_t AssembleTMUParam1(uint32_t uValue) const;

      const DescriptorInfo &GetIMGParamImageDescriptorSetInfo(uint32_t uValue) const;
      const DescriptorInfo &GetTMUParamSamplerDescriptorSetInfo(uint32_t uValue) const;

   private:
      const DescriptorTables                   &m_descTables;
      const CmdBufState::DescriptorSetBindings &m_descSetBindings;
   };


   SubPass                        *CurSubpass() { return m_curSubpass; }
   const RenderPass::SubpassGroup *CurSubpassGroup() { return m_curSubpassGroup; }

   void BeginSubpass();
   void DrawClearRectForAttachmentIfNeeded(uint32_t rpIndex, uint32_t rt);
   bool NeedToLoad(VkAttachmentLoadOp loadOp) const;

   void CreateBinnerControlList(CmdBinRenderJobObj *brJob, v3d_barrier_flags syncFlags) override;
   void InsertRenderTargetCfg(bool doubleBuffer, bool earlyDSClear);
   void InsertColorClearValues();
   void InsertDepthStencilClearValues();
   void CreateRenderControlList(CmdBinRenderJobObj *brJob, const ControlList  &genTileList,
                                v3d_barrier_flags syncFlags) override;
   void AddStore(uint32_t rpIndex, v3d_ldst_buf_t buf, bool resolve);
   void AddLoad(uint32_t rpIndex, v3d_ldst_buf_t buf);
   void AddTileListLoads() override;
   void AddTileListStores() override;
   void CopyAndPatchGraphicsUniformBuffers(GraphicsUniformBufferData& unifBufs,
                                           const GraphicsPipeline &pipe);
   void PatchTNGUniformAddresses(const uint32_t *srcPtr, uint32_t *dstPtr,
                                 const GraphicsUniformBufferData &unifBufs);
   void PatchMainShadrecUniformAddresses(const uint32_t *srcPtr, uint32_t *dstPtr,
                                         const GraphicsUniformBufferData &unifBufs);
   void PatchMainShadrecAttributeAddressesAndMaxIndex(const uint32_t *srcPtr, uint32_t *dstPtr);
   void AllocateLazyMemory(uint32_t attIndex);

   DevMemRange &UpdateDynamicPushConstantBlock();

public:
   Device                          *m_device = nullptr;

   // Current command buffer dynamic state
   CmdBufState                      m_curState;

   // Current push constant state and backing memory (when needed)
   bvk::vector<uint8_t>             m_pushConstants;
   DevMemRange                      m_pushConstantsDevMem;
   bool                             m_pushConstantsDirty = true;

   // Render pass current state
   bool                             m_inRenderPass = false;
   CurRenderPassState               m_curRenderPassState;
   uint32_t                         m_curSubpassIndex = 0;
   bvk::vector<TLBAttachmentInfo>   m_tlbAttachmentInfo;

   // Subpasses of the current render pass
   bvk::vector<SubPass>             m_subpasses;
   SubPass                         *m_curSubpass = nullptr;
   const RenderPass::SubpassGroup  *m_curSubpassGroup = nullptr;

   // Query state management
   QueryManager                     m_queryManager;

   // List of commands from a secondary buffer which must execute after the primary
   // binRender job that branches into the secondary has completed.
   bvk::vector<Command*>            m_delayedPostCommands;
};

} // namespace bvk
