/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

// A command buffer consists of a list of command objects.
//
// Each command object represents either a control-list to execute,
// some synchronization to be performed, or some other CPU based action
// to take.
//
// The command pool allocates blocks of system and device memory.

#include "AllObjects.h"
#include "Common.h"
#include "Command.h"
#include "CommandBufferBuilder.h"
#include "DSAspectCommandBuilder.h"
#include "QueryManager.h"

#include "libs/core/v3d/v3d_tile_size.h"
#include "libs/util/log/log.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"

namespace bvk {

LOG_DEFAULT_CAT("bvk::CommandBuffer");

static v3d_index_type_t TranslateIndexType(VkIndexType indxType, uint32_t *numBytes)
{
   if (indxType == VK_INDEX_TYPE_UINT16)
   {
      *numBytes = sizeof(uint16_t);
      return V3D_INDEX_TYPE_16BIT;
   }
   else
   {
      assert(indxType == VK_INDEX_TYPE_UINT32);
      *numBytes = sizeof(uint32_t);
      return V3D_INDEX_TYPE_32BIT;
   }
}

CommandBuffer::CommandBuffer(
   const VkAllocationCallbacks         *pCallbacks,
   bvk::CommandPool                    *pPool,
   bvk::Device                         *pDevice,
   const VkCommandBufferAllocateInfo   *pAllocInfo) :
      Allocating(pCallbacks),
      m_device(pDevice),
      m_sysMemArena(&pPool->SysMemPool(), pCallbacks),
      m_devDataArena(&pPool->DevDataPool(), pCallbacks),
      m_devQueryArena(&pPool->DevQueryPool(), pCallbacks),
      m_deviceBlocks(GetObjScopeAllocator<DevMemCmdBlock*>()),
      m_commandList(GetObjScopeAllocator<Command*>()),
      m_secondaryPreCommandList(GetObjScopeAllocator<Command*>()),
      m_secondaryPostCommandList(GetObjScopeAllocator<Command*>()),
      m_cleanupCommandList(GetObjScopeAllocator<Command*>())
{
   m_pool = pPool;
   m_level = pAllocInfo->level;
}

CommandBuffer::~CommandBuffer() noexcept
{
   // Return all blocks to the pool
   ReleaseResources();

   // Run any cleanup commands
   for (auto c: m_cleanupCommandList)
      c->Execute();

   // Clear any leftover transient data
   destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(m_td, GetCallbacks());
   m_td = nullptr;
}

void CommandBuffer::LogMemoryUsage() const
{
   if (log_trace_enabled())
   {
      size_t   sysMemBlockCount = 0;
      size_t   sysBytesAlloced = 0;
      size_t   sysBytesUsed = 0;
      size_t   sysDeletedBytesWasted = 0;

      size_t   devDataBlockCount = 0;
      size_t   devDataBytesAlloced = 0;
      size_t   devDataBytesUsed = 0;
      size_t   devDataDeletedBytesWasted = 0;

      size_t   devQueryBlockCount = 0;
      size_t   devQueryBytesAlloced = 0;
      size_t   devQueryBytesUsed = 0;
      size_t   devQueryDeletedBytesWasted = 0;

      size_t   devCmdBytesAlloced = 0;
      size_t   devCmdBytesUsed = 0;

      m_sysMemArena.GetUsageData(&sysMemBlockCount, &sysBytesAlloced,
                                 &sysBytesUsed, &sysDeletedBytesWasted);
      m_devDataArena.GetUsageData(&devDataBlockCount, &devDataBytesAlloced,
                                  &devDataBytesUsed, &devDataDeletedBytesWasted);
      m_devQueryArena.GetUsageData(&devDataBlockCount, &devDataBytesAlloced,
                                   &devDataBytesUsed, &devDataDeletedBytesWasted);

      for (const DevMemCmdBlock *b : m_deviceBlocks)
      {
         devCmdBytesAlloced += b->Size();
         devCmdBytesUsed += b->Used();
      }

      log_trace("VkCommandBuffer %p:", toHandle<VkCommandBuffer>(this));
      log_trace("   Number of commands           : %zu", m_commandList.size());
      log_trace("   System memory blocks used    : %zu", sysMemBlockCount);
      log_trace("   System memory bytes alloced  : %zu", sysBytesAlloced);
      log_trace("   System memory bytes used     : %zu", sysBytesUsed);
      log_trace("   System wasted deleted bytes  : %zu", sysDeletedBytesWasted);
      log_trace("   Device command blocks used   : %zu", m_deviceBlocks.size());
      log_trace("   Device command bytes alloced : %zu", devCmdBytesAlloced);
      log_trace("   Device command bytes used    : %zu", devCmdBytesUsed);
      log_trace("   Device data blocks used      : %zu", devDataBlockCount);
      log_trace("   Device data bytes alloced    : %zu", devDataBytesAlloced);
      log_trace("   Device data bytes used       : %zu", devDataBytesUsed);
      log_trace("   Device data wasted del bytes : %zu", devDataDeletedBytesWasted);
      log_trace("   Device query blocks used     : %zu", devQueryBlockCount);
      log_trace("   Device query bytes alloced   : %zu", devQueryBytesAlloced);
      log_trace("   Device query bytes used      : %zu", devQueryBytesUsed);
      log_trace("   Device query wasted del bytes: %zu", devQueryDeletedBytesWasted);
   }
}

VkResult CommandBuffer::BeginCommandBuffer(
   const VkCommandBufferBeginInfo   *pBeginInfo) noexcept
{
   try
   {
      assert(m_mode != eRECORDING);

      if (m_pool->PoolCreateFlags() & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
         ResetCommandBuffer(0);
      else
         assert(m_mode != eEXECUTABLE);

      m_commandList.reserve(64);

      m_mode = eRECORDING;
      m_status = VK_SUCCESS;
      m_usageFlags = pBeginInfo->flags;

      assert(m_td == nullptr);

      m_td = createObject<CommandBufferBuilder, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
                                       GetCallbacks(), GetCallbacks(), m_device, this);

      if (m_usageFlags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)
      {
         // Indicates a secondary buffer that lives entirely inside a render pass.
         // Set state to emulate being in that particular render pass and subpass. We
         // have to do this as there will be no call to BeginRenderPass in this command buffer.
         assert(m_level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);

         m_td->BeginSecondaryCommandBuffer(pBeginInfo->pInheritanceInfo);

         // We do not support inherited occlusion queries
         assert(!pBeginInfo->pInheritanceInfo->occlusionQueryEnable);
      }
   }
   catch (const std::bad_alloc &)        { return VK_ERROR_OUT_OF_HOST_MEMORY;   }
   catch (const bvk::bad_device_alloc &) { return VK_ERROR_OUT_OF_DEVICE_MEMORY; }

   return VK_SUCCESS;
}

void CommandBuffer::SplitSecondaryControlList()
{
   assert(m_level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);
   assert(InRenderPass());

   // Close the previous control list
   m_td->FinalizeControlList();

   CmdControlListObj *clObj = NewObject<CmdControlListObj>(CurSubpass()->controlList.Start());

   // Add the control list object to the command list now
   m_commandList.push_back(clObj);

   // Reset the control list
   CurSubpass()->controlList = {};

   // Restart control list
   m_td->NeedControlList();
}

VkResult CommandBuffer::EndCommandBuffer() noexcept
{
   assert(m_mode == eRECORDING);

   if (m_status == VK_SUCCESS)
   {
      // This may be a secondary command buffer which still needs to be flushed
      if (m_level == VK_COMMAND_BUFFER_LEVEL_SECONDARY && InRenderPass())
      {
         // Close the previous control list
         m_td->FinalizeControlList();

         // Secondary command buffers don't have fully fledged bin/render jobs,
         // just control lists. They must be fully contained in one subpass if inside
         // a render pass.
         CmdControlListObj *clObj = NewObject<CmdControlListObj>(CurSubpass()->controlList.Start());

         // Add the control list object to the command list now
         m_commandList.push_back(clObj);
      }

      // Ensure any device memory we touched is flushed now
      for (auto block : m_deviceBlocks)
         block->SyncMemory();

      // Visit each devMem block in the arena and sync those too
      m_devDataArena.VisitBlocks(std::mem_fn(&DevMemDataBlock::SyncMemory));

      // And each devMemQuery block
      m_devQueryArena.VisitBlocks(std::mem_fn(&DevMemQueryBlock::SyncMemory));

      // Dump memory usage information
      LogMemoryUsage();
   }

   // Clear the transient data we only needed during building
   destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(m_td, GetCallbacks());
   m_td = nullptr;

   m_mode = eEXECUTABLE;
   return m_status;
}

void CommandBuffer::ReleaseResources()
{
   // Run command destructors
   for (Command *cmd : m_commandList)
      cmd->~Command();

   bvk::vector<Command*>().swap(m_commandList);

   m_sysMemArena.FreeAll();
   m_devDataArena.FreeAll();
   m_devQueryArena.FreeAll();

   for (DevMemCmdBlock *block : m_deviceBlocks)
      m_pool->ReleaseDeviceCmdBlock(block);
   m_deviceBlocks.clear();
}

VkResult CommandBuffer::ResetCommandBuffer(
   VkCommandBufferResetFlags   flags) noexcept
{
   try
   {
      // Note : we ignore the VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT and always
      // return blocks to the pool. There is very little overhead in having the pool
      // hand them out again when we want them back.
      ReleaseResources();

      // Clear the transient data
      destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(m_td, GetCallbacks());
      m_td = nullptr;

      m_mode = eINITIAL;
      m_status = VK_SUCCESS;
   }
   catch (const std::bad_alloc &)        { return VK_ERROR_OUT_OF_HOST_MEMORY;   }
   catch (const bvk::bad_device_alloc &) { return VK_ERROR_OUT_OF_DEVICE_MEMORY; }

   return VK_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
// STATE SETTING COMMANDS
//////////////////////////////////////////////////////////////////////////////////
void CommandBuffer::CmdBindPipeline(
   VkPipelineBindPoint   pipelineBindPoint,
   bvk::Pipeline        *pipeline) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS)
      CurState().BindGraphicsPipeline(dynamic_cast<GraphicsPipeline*>(pipeline));
   else
      CurState().BindComputePipeline(dynamic_cast<ComputePipeline*>(pipeline));
   CMD_END
}

void CommandBuffer::CmdSetViewport(
   uint32_t           firstViewport,
   uint32_t           viewportCount,
   const VkViewport  *pViewports) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   CurState().SetViewport(pViewports[0]);

   CMD_END
}

void CommandBuffer::CmdSetScissor(
   uint32_t        firstScissor,
   uint32_t        scissorCount,
   const VkRect2D *pScissors) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   CurState().SetScissorRect(pScissors[0]);

   CMD_END
}

void CommandBuffer::CmdSetLineWidth(
   float  lineWidth) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   CurState().SetLineWidth(lineWidth);
   CMD_END
}

void CommandBuffer::CmdSetDepthBias(
   float  depthBiasConstantFactor,
   float  depthBiasClamp,
   float  depthBiasSlopeFactor) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   CurState().SetDepthBias(depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
   CMD_END
}

void CommandBuffer::CmdSetBlendConstants(
   const float  blendConstants[4]) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   CurState().SetBlendConstants(blendConstants);
   CMD_END
}

void CommandBuffer::CmdSetDepthBounds(
   float  minDepthBounds,
   float  maxDepthBounds) noexcept
{
   /* Not supported */
}

void CommandBuffer::CmdSetStencilCompareMask(
   VkStencilFaceFlags faceMask,
   uint32_t           compareMask) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   CurState().SetStencilCompareMask(faceMask, compareMask);
   CMD_END
}

void CommandBuffer::CmdSetStencilWriteMask(
   VkStencilFaceFlags faceMask,
   uint32_t           writeMask) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   CurState().SetStencilWriteMask(faceMask, writeMask);
   CMD_END
}

void CommandBuffer::CmdSetStencilReference(
   VkStencilFaceFlags faceMask,
   uint32_t           reference) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   CurState().SetStencilReference(faceMask, reference);
   CMD_END
}

void CommandBuffer::CmdBindDescriptorSets(
   VkPipelineBindPoint      pipelineBindPoint,
   bvk::PipelineLayout     *layout,
   uint32_t                 firstSet,
   uint32_t                 descriptorSetCount,
   const VkDescriptorSet   *pDescriptorSets,
   uint32_t                 dynamicOffsetCount,
   const uint32_t          *pDynamicOffsets) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   uint32_t dynamicOffset = 0;
   for (uint32_t i = 0; i < descriptorSetCount; i++)
   {
      // Set the binding in our current state
      CurState().BindDescriptorSet(pipelineBindPoint, firstSet + i, pDescriptorSets[i],
                                   dynamicOffsetCount, pDynamicOffsets, dynamicOffset);

      // Accumulate the starting dynamic offset
      DescriptorSet *ds = fromHandle<DescriptorSet>(pDescriptorSets[i]);
      dynamicOffset += ds->GetNumDynamicOffsetsNeeded();
   }

   CMD_END
}

void CommandBuffer::CmdBindIndexBuffer(
   bvk::Buffer *buffer,
   VkDeviceSize offset,
   VkIndexType  indexType) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   CurState().BindIndexBuffer(buffer, offset, indexType);
   CMD_END
}

void CommandBuffer::CmdBindVertexBuffers(
   uint32_t              firstBinding,
   uint32_t              bindingCount,
   const VkBuffer       *pBuffers,
   const VkDeviceSize   *pOffsets) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   for (uint32_t i = 0; i < bindingCount; i++)
      CurState().BindVertexBuffer(firstBinding + i, fromHandle<Buffer>(pBuffers[i]), pOffsets[i]);

   CMD_END
}

void CommandBuffer::CmdPushConstants(
   PipelineLayout       *layout,
   VkShaderStageFlags    stageFlags,
   uint32_t              offset,
   uint32_t              size,
   const void           *pValues) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   m_td->SetPushConstants(layout, stageFlags, offset, size, pValues);
   CMD_END
}

void CommandBuffer::CmdDebugMarkerBeginEXT(
   const VkDebugMarkerMarkerInfoEXT *pMarkerInfo) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   CMD_END
}

void CommandBuffer::CmdDebugMarkerEndEXT() noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   CMD_END
}

void CommandBuffer::CmdDebugMarkerInsertEXT(
   const VkDebugMarkerMarkerInfoEXT *pMarkerInfo) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   CMD_END
}

//////////////////////////////////////////////////////////////////////////////////
// ACTION COMMANDS
//////////////////////////////////////////////////////////////////////////////////

void CommandBuffer::DrawCallPreamble()
{
   // Ensure we have a control list
   m_td->NeedControlList();

   // Set the latest occlusion address
   CurState().SetOcclusionCounter(m_td->GetQueryManager()->GetCurHWCounterAddr());

   // Update any changed state into the control list
   CurState().BuildStateUpdateCL(this);

   m_td->WriteGraphicsShaderRecord();
}

void CommandBuffer::CmdDraw(
   uint32_t  vertexCount,
   uint32_t  instanceCount,
   uint32_t  firstVertex,
   uint32_t  firstInstance) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   if (vertexCount == 0 || instanceCount == 0)
      return;

   const GraphicsPipeline *pipe = m_td->m_curState.BoundGraphicsPipeline();
   v3d_prim_mode_t         primMode = pipe->GetDrawPrimMode();

   // Output draw call preamble control list items
   DrawCallPreamble();

   if (firstInstance != 0)
      v3d_cl_base_vertex_base_instance(CLPtr(), 0, firstInstance);

   if (instanceCount > 1)
      v3d_cl_vertex_array_instanced_prims(CLPtr(), primMode, vertexCount, instanceCount, firstVertex);
   else
      v3d_cl_vertex_array_prims(CLPtr(), primMode, vertexCount, firstVertex);

   CMD_END
}

void CommandBuffer::CmdDrawIndexed(
   uint32_t  indexCount,
   uint32_t  instanceCount,
   uint32_t  firstIndex,
   int32_t   vertexOffset,
   uint32_t  firstInstance) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   if (indexCount == 0 || instanceCount == 0)
      return;

   const GraphicsPipeline *pipe = m_td->m_curState.BoundGraphicsPipeline();
   v3d_prim_mode_t         primMode = pipe->GetDrawPrimMode();

   // Output draw call preamble control list items
   DrawCallPreamble();

   const auto &indexBuffer = CurState().BoundIndexBuffer();

   size_t   idxBufferSize = static_cast<size_t>(indexBuffer.GetBufferSize());
   uint32_t indexSize;

   v3d_index_type_t indexType = TranslateIndexType(indexBuffer.GetIndexType(), &indexSize);

   if (vertexOffset || firstInstance)
      v3d_cl_base_vertex_base_instance(CLPtr(), vertexOffset, firstInstance);

   v3d_addr_t indicesAddr  = indexBuffer.CalculateBufferOffsetAddr(0);

#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_cl_index_buffer_setup(CLPtr(), indicesAddr, idxBufferSize);
#else
   assert(firstIndex + indexCount <= idxBufferSize / indexSize);
   uint32_t maxAllowedIndex, maxAllowedInstance;
   bool valid = m_td->CalcMaxAllowedIndexAndInstance(pipe->GetLinkResult(), &maxAllowedIndex, &maxAllowedInstance);
   if (!valid)
      return;
#endif

   if (instanceCount > 1)
   {
      V3D_CL_INDEXED_INSTANCED_PRIM_LIST_T primList;
      primList.prim_mode      = primMode;
      primList.index_type     = indexType;
      primList.num_indices    = indexCount;
      primList.prim_restart   = pipe->PrimitiveRestartEnable();
      primList.num_instances  = instanceCount;
#if V3D_VER_AT_LEAST(4,1,34,0)
      primList.indices_offset = firstIndex * indexSize;
#else
      primList.indices_addr   = indicesAddr + static_cast<v3d_addr_t>(firstIndex * indexSize);
      primList.max_index      = maxAllowedIndex;
#endif

      v3d_cl_indexed_instanced_prim_list_indirect(CLPtr(), &primList);
   }
   else
   {
      V3D_CL_INDEXED_PRIM_LIST_T primList;
      primList.prim_mode      = primMode;
      primList.index_type     = indexType;
      primList.num_indices    = indexCount;
      primList.prim_restart   = pipe->PrimitiveRestartEnable();
#if V3D_VER_AT_LEAST(4,1,34,0)
      primList.indices_offset = firstIndex * indexSize;
#else
      primList.indices_addr   = indicesAddr + static_cast<v3d_addr_t>(firstIndex * indexSize);
      primList.min_index      = 0;
      primList.max_index      = maxAllowedIndex;
#endif
      v3d_cl_indexed_prim_list_indirect(CLPtr(), &primList);
   }

   CMD_END
}

void CommandBuffer::CmdDrawIndirect(
   bvk::Buffer *buffer,
   VkDeviceSize offset,
   uint32_t     drawCount,
   uint32_t     stride) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   if (drawCount == 0)
      return;

   const GraphicsPipeline *pipe = m_td->m_curState.BoundGraphicsPipeline();
   v3d_prim_mode_t         primMode = pipe->GetDrawPrimMode();

   // Output draw call preamble control list items
   DrawCallPreamble();

   // From the spec:
   // If drawCount is equal to 1, (offset + sizeof(VkDrawIndirectCommand))
   //     must be less than or equal to the size of buffer
   // If drawCount is greater than 1, (stride * (drawCount - 1) + offset + sizeof(VkDrawIndirectCommand))
   //     must be less than or equal to the size of buffer
   if (drawCount == 1)
      assert(offset + sizeof(VkDrawIndirectCommand) <= buffer->Size());
   else
      assert((stride * (drawCount - 1)) + offset + sizeof(VkDrawIndirectCommand) <= buffer->Size());

#if !V3D_VER_AT_LEAST(4,1,34,0)
   uint32_t maxAllowedIndex, maxAllowedInstance;
   bool valid = m_td->CalcMaxAllowedIndexAndInstance(pipe->GetLinkResult(), &maxAllowedIndex, &maxAllowedInstance);

   if (!valid)
      return;

   // Set the allowable ranges
   v3d_cl_indirect_primitive_limits(CLPtr(), maxAllowedIndex, maxAllowedInstance, 0);
#endif

   v3d_addr_t indirectAddr = buffer->CalculateBufferOffsetAddr(offset);

   // Do the indirect draw
   v3d_cl_indirect_vertex_array_prims(CLPtr(), primMode, drawCount, indirectAddr, stride);

   CMD_END
}

void CommandBuffer::CmdDrawIndexedIndirect(
   bvk::Buffer *buffer,
   VkDeviceSize offset,
   uint32_t     drawCount,
   uint32_t     stride) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   if (drawCount == 0)
      return;

   const GraphicsPipeline *pipe     = m_td->m_curState.BoundGraphicsPipeline();
   v3d_prim_mode_t         primMode = pipe->GetDrawPrimMode();

   // Output draw call preamble control list items
   DrawCallPreamble();

   // From the spec:
   // If drawCount is equal to 1, (offset + sizeof(VkDrawIndexedIndirectCommand ))
   //     must be less than or equal to the size of buffer
   // If drawCount is greater than 1, (stride * (drawCount - 1) + offset + sizeof(VkDrawIndexedIndirectCommand ))
   //     must be less than or equal to the size of buffer
   if (drawCount == 1)
      assert(offset + sizeof(VkDrawIndexedIndirectCommand) <= buffer->Size());
   else
      assert((stride * (drawCount - 1)) + offset + sizeof(VkDrawIndexedIndirectCommand) <= buffer->Size());

   const auto &indexBuffer  = CurState().BoundIndexBuffer();

   size_t     idxBufferSize = static_cast<size_t>(indexBuffer.GetBufferSize());
   v3d_addr_t indicesAddr   = indexBuffer.CalculateBufferOffsetAddr(0);

   uint32_t indexSize;
   v3d_index_type_t indexType = TranslateIndexType(indexBuffer.GetIndexType(), &indexSize);

#if !V3D_VER_AT_LEAST(4,1,34,0)
   uint32_t maxAllowedIndex, maxAllowedInstance;
   bool valid = m_td->CalcMaxAllowedIndexAndInstance(pipe->GetLinkResult(),
         &maxAllowedIndex, &maxAllowedInstance);

   if (!valid)
      return;

   // Set the allowable ranges
   uint32_t maxIndices = idxBufferSize / indexSize;
   v3d_cl_indirect_primitive_limits(CLPtr(), maxAllowedIndex, maxAllowedInstance, maxIndices);
#else
   v3d_cl_index_buffer_setup(CLPtr(), indicesAddr, idxBufferSize);
#endif

   v3d_addr_t indirectAddr = buffer->CalculateBufferOffsetAddr(offset);

   // Do the indirect draw
   v3d_cl_indirect_indexed_prim_list(CLPtr(), primMode, indexType, drawCount,
                                     pipe->PrimitiveRestartEnable(), indirectAddr,
#if !V3D_VER_AT_LEAST(4,1,34,0)
                                     indicesAddr,
#endif
                                     stride);

   CMD_END
}

void CommandBuffer::CmdDispatchCommon(
   const ComputePipeline& pipe,
   CmdComputeJobObj& cmd,
   DevMemRange& uniformMem) noexcept
{
   m_td->CopyAndPatchUniformBuffer(uniformMem, SHADER_COMPUTE, pipe, pipe.GetUniforms());

#if V3D_USE_CSD
   cmd.m_subjob = pipe.GetLinkResult().m_cs.subjob;
   cmd.m_subjob.unifs_addr = uniformMem.Phys();
#else
   if (m_usageFlags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)
      cmd.m_last.reset(NewObject<CmdComputeJobObj::Last>());

   cmd.m_computeShader = &pipe.GetLinkResult().m_cs;
   cmd.m_unifsAddr = uniformMem.Phys();

   static const compute_cl_mem_if memInterface =
   {
      //! Obtain a pointer to append size bytes to the control-list. Returns NULL for failure.
      /*.write_cl = */ [](void* ctx, unsigned size) -> uint8_t*
      {
         // Don't throw exceptions into C code.
         CommandBuffer *cmdBuf = (CommandBuffer*)ctx;
         try
         {
            uint8_t** cl = cmdBuf->m_td->CLPtr(size);
            uint8_t* ret = *cl;
            *cl += size;
            return ret;
         }
         catch (std::bad_alloc&) { cmdBuf->m_status = VK_ERROR_OUT_OF_HOST_MEMORY; }
         catch (bad_device_alloc&) { cmdBuf->m_status = VK_ERROR_OUT_OF_DEVICE_MEMORY; }
         return nullptr;
      },

      //! Obtain a pointer to append size bytes to the control-list for the last time.
      //! Size will be no more than V3D_CL_BRANCH_SIZE and must not fail.
      /*.write_cl_final = */ [](void* ctx, unsigned size) -> uint8_t*
      {
         uint8_t** cl = ((CommandBuffer*)ctx)->m_td->CLPtrFinal(size);
         uint8_t* ret = *cl;
         *cl += size;
         return ret;
      }
   };

   CommandBufferBuilder::ControlList controlList;
   m_td->SetCurrentControlList(&controlList);
   controlList.SetStart(*m_td->m_curDeviceBlock);

   // We should be able to add multiple dispatch commands to the same job...
   compute_cl_begin(&memInterface, this);
   uint8_t* dispatch = compute_cl_add_dispatch(&memInterface, this);
   if (!dispatch)
      return;
   v3d_size_t dispatchClOffset = m_td->m_curDeviceBlock->OffsetOfPtr(dispatch);
   gmem_handle_t dispatchClMem = m_td->m_curDeviceBlock->Handle();
   compute_cl_end(&memInterface, this);

   controlList.SetEnd(*m_td->m_curDeviceBlock);
   m_td->SetCurrentControlList(nullptr);

   cmd.m_subjob.start = controlList.Start();
   cmd.m_subjob.end = controlList.End();
   cmd.m_dispatchClOffset = dispatchClOffset;
   cmd.m_dispatchClMem = dispatchClMem;
#endif
}

void CommandBuffer::CmdDispatch(
   uint32_t x,
   uint32_t y,
   uint32_t z) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   UniquePtr<CmdComputeImmediateJobObj> cmdPtr(NewCmd(CmdComputeImmediateJob));
   CmdComputeImmediateJobObj& cmd = *cmdPtr.get();

   const ComputePipeline &pipe = *m_td->m_curState.BoundComputePipeline();
   DevMemRange uniformMem;
   CmdDispatchCommon(pipe, cmd, uniformMem);

#if V3D_USE_CSD
   uint32_t* numWorkGroups = cmd.m_subjob.num_wgs;
#else
   uint32_t* numWorkGroups = cmd.m_numWorkGroups;
#endif
   numWorkGroups[0] = x;
   numWorkGroups[1] = y;
   numWorkGroups[2] = z;

   // Patch num-work-group uniforms now.
   const ComputePipeline::PreprocessPatchInfo &patchInfo = pipe.GetPreprocessPatchInfo();
   for (const auto& u: patchInfo.numWorkGroups)
   {
      ((uint32_t*)uniformMem.Ptr())[u.offset] = numWorkGroups[u.index];
   }

   m_commandList.push_back(&cmd);
   cmdPtr.release();
   CMD_END;
}

void CommandBuffer::CmdDispatchIndirect(
   bvk::Buffer *buffer,
   VkDeviceSize offset) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   UniquePtr<CmdComputeIndirectJobObj> cmdPtr(NewCmd(CmdComputeIndirectJob));
   CmdComputeIndirectJobObj& cmd = *cmdPtr.get();
   cmd.m_bufferOffset = (v3d_size_t)buffer->GetBoundMemoryOffset() + (v3d_size_t)offset;
   cmd.m_bufferHandle = buffer->GetBoundMemory()->Handle();

   // Ensure mapped. VK_ERROR_OUT_OF_DEVICE_MEMORY doesn't seem like quite the right error
   // here, but that is what the existing Vulkan code reports if it can't map a new device block.
   if (!gmem_map_and_get_ptr(cmd.m_bufferHandle))
      throw bvk::bad_device_alloc();

   const ComputePipeline &pipe = *m_td->m_curState.BoundComputePipeline();
   DevMemRange uniformMem;
   CmdDispatchCommon(pipe, cmd, uniformMem);

   // Num-work-group uniforms will be patched in preprocess.
   const ComputePipeline::PreprocessPatchInfo &patchInfo = pipe.GetPreprocessPatchInfo();
   if (!patchInfo.numWorkGroups.empty())
   {
    #if V3D_USE_CSD
      if (m_usageFlags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)
         cmd.m_last.reset(NewObject<CmdComputeJobObj::Last>());
    #endif

      cmd.m_patchInfo = &patchInfo;
      cmd.m_uniformsHandle = uniformMem.Handle();
      cmd.m_uniformsOffset = uniformMem.Phys() - gmem_get_addr(cmd.m_uniformsHandle);
   }

   m_commandList.push_back(&cmd);
   cmdPtr.release();
   CMD_END;
}

void CommandBuffer::CmdCopyBuffer(
   bvk::Buffer          *srcBuffer,
   bvk::Buffer          *dstBuffer,
   uint32_t              regionCount,
   const VkBufferCopy   *pRegions) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   assert(!InRenderPass());

   auto cmd = NewCmd(CmdCopyBuffer);
   cmd->m_srcBuffer   = srcBuffer;
   cmd->m_dstBuffer   = dstBuffer;
   cmd->m_regionCount = regionCount;

   CloneObjArray(&cmd->m_regions, pRegions, regionCount);

   m_commandList.push_back(cmd);
   CMD_END
}

void CommandBuffer::CmdCopyImage(
   bvk::Image        *srcImage,
   VkImageLayout      srcImageLayout,
   bvk::Image        *dstImage,
   VkImageLayout      dstImageLayout,
   uint32_t           regionCount,
   const VkImageCopy *pRegions) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   assert(!InRenderPass());

   auto cmd = NewCmd(CmdCopyImage);
   cmd->m_srcImage       = srcImage;
   cmd->m_srcImageLayout = srcImageLayout;
   cmd->m_dstImage       = dstImage;
   cmd->m_dstImageLayout = dstImageLayout;
   cmd->m_regionCount    = regionCount;

   CloneObjArray(&cmd->m_regions, pRegions, regionCount);

   m_commandList.push_back(cmd);
   CMD_END
}

void CommandBuffer::CmdCopyBufferToImage(
   bvk::Buffer             *srcBuffer,
   bvk::Image              *dstImage,
   VkImageLayout            dstImageLayout,
   uint32_t                 regionCount,
   const VkBufferImageCopy *pRegions) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   assert(!InRenderPass());

   auto cmd = NewCmd(CmdCopyBufferToImage);
   cmd->m_srcBuffer      = srcBuffer;
   cmd->m_dstImage       = dstImage;
   cmd->m_dstImageLayout = dstImageLayout;
   cmd->m_regionCount    = regionCount;

   CloneObjArray(&cmd->m_regions, pRegions, regionCount);

   m_commandList.push_back(cmd);
   CMD_END
}

void CommandBuffer::CmdCopyImageToBuffer(
   bvk::Image              *srcImage,
   VkImageLayout            srcImageLayout,
   bvk::Buffer             *dstBuffer,
   uint32_t                 regionCount,
   const VkBufferImageCopy *pRegions) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   assert(!InRenderPass());

   auto cmd = NewCmd(CmdCopyImageToBuffer);
   cmd->m_srcImage       = srcImage;
   cmd->m_srcImageLayout = srcImageLayout;
   cmd->m_dstBuffer      = dstBuffer;
   cmd->m_regionCount    = regionCount;

   CloneObjArray(&cmd->m_regions, pRegions, regionCount);

   m_commandList.push_back(cmd);
   CMD_END
}

void CommandBuffer::CmdUpdateBuffer(
   bvk::Buffer    *dstBuffer,
   VkDeviceSize    dstOffset,
   VkDeviceSize    dataSize,
   const void     *pData) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   assert(!InRenderPass());

   auto cmd = NewCmd(CmdUpdateBuffer);
   cmd->m_dstBuffer = dstBuffer;
   cmd->m_dstOffset = dstOffset;
   cmd->m_dataSize  = dataSize;

   // The size is in bytes, but must be a multiple of 4 (uint32_t chunks)
   size_t size = static_cast<size_t>(dataSize);

   CloneObjArray(reinterpret_cast<uint8_t**>(&cmd->m_data),
                 static_cast<const uint8_t*>(pData), size);

   m_commandList.push_back(cmd);
   CMD_END
}

void CommandBuffer::CmdFillBuffer(
   bvk::Buffer *dstBuffer,
   VkDeviceSize dstOffset,
   VkDeviceSize size,
   uint32_t     data) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   assert(!InRenderPass());

   auto cmd = NewCmd(CmdFillBuffer);
   cmd->m_dstBuffer = dstBuffer;
   cmd->m_dstOffset = dstOffset;
   cmd->m_size      = size;
   cmd->m_data      = data;

   m_commandList.push_back(cmd);
   CMD_END
}

void CommandBuffer::CmdClearDepthStencilImage(
   bvk::Image                       *image,
   VkImageLayout                     imageLayout,
   const VkClearDepthStencilValue   *pDepthStencil,
   uint32_t                          rangeCount,
   const VkImageSubresourceRange    *pRanges) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   assert(!InRenderPass());

   // Stencil only format S8_UINT is optional and not currently supported
   assert(gfx_lfmt_has_depth(image->NaturalLFMT()));
   assert(pDepthStencil->depth >= 0.0f && pDepthStencil->depth <= 1.0f);

   log_trace("CmdClearDepthStencilImage: image = %p", image);

   // The Vulkan spec does not appear to disallow depth/stencil images created
   // as 1-D or 3-D, with mipmaps or as an array.
   const bool is3D = gfx_lfmt_is_3d(image->NaturalLFMT());

   for (uint32_t i = 0; i < rangeCount; i++)
   {
      assert((pRanges[i].aspectMask &  (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0);
      assert((pRanges[i].aspectMask & ~(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) == 0);

      for (uint32_t m = 0; m < pRanges[i].levelCount; m++)
      {
         uint32_t layerCount;
         if (is3D)
            layerCount = image->Extent().depth;
         else if (pRanges[i].layerCount == VK_REMAINING_ARRAY_LAYERS)
            layerCount = image->ArrayLayers() - pRanges[i].baseArrayLayer;
         else
            layerCount = pRanges[i].layerCount;

         const uint32_t mipLevel  = pRanges[i].baseMipLevel + m;
         const uint32_t baseLayer = is3D ? 0 : pRanges[i].baseArrayLayer;

         for (uint32_t l = baseLayer; l < (baseLayer + layerCount); l++)
         {
            DSAspectCommandBuilder dsb {GetCallbacks(), this};

            // The aspect mask is used to determine which components to store
            // out from the TLB. The TLB is able to mask out writes to either
            // depth or stencil even for the packed D24S8 format. This means we
            // do not need to load the destination and modify the depth or
            // stencil with a shader for this operation.
            //
            // Unlike the colour aspect builder which can use multiple render
            // targets, the TLB only has a single depth+stencil target so only
            // one layer/slice can be cleared at a time.
            dsb.SetImageSubresources(image, mipLevel, l, pRanges[i].aspectMask);
            dsb.SetClearParams(pDepthStencil->depth, pDepthStencil->stencil);

            auto cmd = NewObject<CmdBinRenderJobObj>(m_device->GetPhysicalDevice());

            log_trace("\tadding job for range = %u, mipLevel = %u layer/slice = %u",
                  i, (unsigned)mipLevel, (unsigned)l);

            // Default control list builder sync flags are OK for TLB clears
            dsb.CreateMasterControlLists(cmd);

            // We need to add our own execution barriers between regions as each
            // job may be writing to portions of the same memory. On the multi-core
            // simulator this will cause rendering errors as tile writes fight
            // against each other.
            if (i != 0)
               InsertExecutionBarrier();

            m_commandList.push_back(cmd);
         }
      }
   }
   CMD_END
}

void CommandBuffer::ClearAttachments(
   uint32_t                 attachmentCount,
   const VkClearAttachment *pAttachments,
   uint32_t                 rectCount,
   const VkClearRect       *pRects) noexcept
{
   assert(m_td->m_curRenderPassState.framebuffer != nullptr);

   m_td->NeedControlList();

   // Do not record occlusion counters during clear
   m_td->DisableOcclusionQuery();

   for (uint32_t a = 0; a < attachmentCount; a++)
   {
      uint32_t rpAttachmentIndex = pAttachments[a].colorAttachment;
      uint32_t colAttIndex       = 0;

      if (pAttachments[a].aspectMask == VK_IMAGE_ASPECT_COLOR_BIT)
      {
         colAttIndex       = pAttachments[a].colorAttachment;
         rpAttachmentIndex = m_td->m_curSubpassGroup->m_colorRTAttachments[colAttIndex];
      }
      else
         rpAttachmentIndex = m_td->m_curSubpassGroup->m_dsAttachment;

      if (rpAttachmentIndex != VK_ATTACHMENT_UNUSED)
      {
         GFX_LFMT_T fmt = m_td->m_curRenderPassState.renderPass->Attachments()[rpAttachmentIndex].format;

         ImageView *imgView = fromHandle<ImageView>(m_td->m_curRenderPassState.framebuffer->
                                                    Attachments()[rpAttachmentIndex]);

         DrawClearRects(colAttIndex, pAttachments[a], rectCount, pRects, fmt,
                        imgView->GetSubresourceRange());
      }
   }
}

void CommandBuffer::CmdClearAttachments(
   uint32_t                 attachmentCount,
   const VkClearAttachment *pAttachments,
   uint32_t                 rectCount,
   const VkClearRect       *pRects) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   assert(InRenderPass());

   if (m_level == VK_COMMAND_BUFFER_LEVEL_SECONDARY &&
      (PrimaryClearRequired(rectCount, pRects) || m_td->m_curRenderPassState.framebuffer == nullptr))
   {
      // We need to defer the clear until we get recorded in a primary buffer.
      SplitSecondaryControlList();

      auto cmd = NewCmd(CmdSecondaryDeferredClear);

      cmd->m_attachmentCount = attachmentCount;
      cmd->m_rectCount       = rectCount;

      CloneObjArray(&cmd->m_attachments, pAttachments, attachmentCount);
      CloneObjArray(&cmd->m_rects, pRects, rectCount);

      m_commandList.push_back(cmd);
   }
   else
   {
      ClearAttachments(attachmentCount, pAttachments, rectCount, pRects);
   }
   CMD_END
}

void CommandBuffer::CmdBeginQuery(
   bvk::QueryPool       *queryPool,
   uint32_t              query,
   VkQueryControlFlags   flags) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   auto cmd = NewCmd(CmdZeroQueryHWCounters);

   m_td->GetQueryManager()->Begin(queryPool, query, cmd);

   m_commandList.push_back(cmd);

   // Ensure we wait for the h/w counter zeroing to complete before continuing
   InsertExecutionBarrier();

   CMD_END
}

// Add commands to the given command list to update the query pool counters after
// an execution barrier
void CommandBuffer::AddQueryPoolUpdateCommands(bvk::vector<Command*> &cmdList)
{
   QueryDataEntryPtrList qdePtrList = m_td->GetQueryManager()->MakeQueryDataForJob(m_sysMemArena);
   if (qdePtrList.NumEntries() > 0)
   {
      // Ensure we wait for the previous jobs to complete before getting counter values
      InsertExecutionBarrier(cmdList);

      CmdUpdateQueryPoolObj *updateJob = NewObject<CmdUpdateQueryPoolObj>(qdePtrList);
      cmdList.push_back(updateJob);

      // Ensure we don't use the same h/w counters again
      m_td->GetQueryManager()->RemoveFinishedQueries();
   }
}

void CommandBuffer::CmdEndQuery(
   bvk::QueryPool *queryPool,
   uint32_t        query) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   m_td->GetQueryManager()->End(queryPool, query, m_sysMemArena);

   if (m_td->GetQueryManager()->HasQueries())
   {
      if (InRenderPass())
      {
         if (m_level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
         {
            // Record any query counter updates now into the 2ndary post command list
            AddQueryPoolUpdateCommands(m_secondaryPostCommandList);
         }
      }
      else
      {
         // The queries outside of the render pass will not have finished when the bin/render
         // job was made, and therefore won't have any query data blocks and won't trigger
         // update jobs. We need to manually trigger an update.
         AddQueryPoolUpdateCommands();
      }
   }

   CMD_END
}

void CommandBuffer::CmdResetQueryPool(
   bvk::QueryPool *queryPool,
   uint32_t        firstQuery,
   uint32_t        queryCount) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   // Make sure we don't execute too early and dump on pending results
   InsertExecutionBarrier();

   // Insert a user mode job that will zero the s/w pool counters as requested
   auto cmd = NewObject<CmdResetQueryPoolObj>(queryPool, firstQuery, queryCount);
   m_commandList.push_back(cmd);

   // Wait for the zeroing to complete before continuing
   InsertExecutionBarrier();

   CMD_END
}

void CommandBuffer::CmdWriteTimestamp(
   VkPipelineStageFlagBits  pipelineStage,
   bvk::QueryPool          *queryPool,
   uint32_t                 query) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   auto cmd = NewCmd(CmdWriteTimestamp);
   cmd->m_pipelineStage = pipelineStage;
   cmd->m_queryPool     = queryPool;
   cmd->m_query         = query;

   m_commandList.push_back(cmd);
   CMD_END
}

void CommandBuffer::CmdCopyQueryPoolResults(
   bvk::QueryPool    *queryPool,
   uint32_t           firstQuery,
   uint32_t           queryCount,
   bvk::Buffer       *dstBuffer,
   VkDeviceSize       dstOffset,
   VkDeviceSize       stride,
   VkQueryResultFlags flags) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   if ((flags & VK_QUERY_RESULT_WAIT_BIT) != 0)
      InsertExecutionBarrier();

   auto cmd = NewCmd(CmdCopyQueryPoolResults);
   cmd->m_device     = m_device;
   cmd->m_queryPool  = queryPool;
   cmd->m_firstQuery = firstQuery;
   cmd->m_queryCount = queryCount;
   cmd->m_dstBuffer  = dstBuffer;
   cmd->m_dstOffset  = dstOffset;
   cmd->m_stride     = stride;
   cmd->m_flags      = flags;

   m_commandList.push_back(cmd);
   CMD_END
}

void CommandBuffer::CmdBeginRenderPass(
   const VkRenderPassBeginInfo   *pRenderPassBegin,
   VkSubpassContents              contents) noexcept
{
   CMD_BEGIN

   assert(m_mode == eRECORDING);
   assert(m_level == VK_COMMAND_BUFFER_LEVEL_PRIMARY);

   auto *rp = fromHandle<RenderPass>(pRenderPassBegin->renderPass);
   if (rp->PreBarrierExternal())
      InsertExecutionBarrier();

   m_td->BeginRenderPass(pRenderPassBegin, contents);

   CMD_END
}

void CommandBuffer::BuildHardwareJob() noexcept
{
   // Wrap in a master control list now. The MCL will branch to each subpass in the group
   CmdBinRenderJobObj *brJob = NewObject<CmdBinRenderJobObj>(m_device->GetPhysicalDevice());

   // TODO: make some of these based on the pipeline/renderpass configuration?
   //       e.g. TLB read (although does this matter, is it ever cached?),
   //       TMU config/read for bin jobs, TMU write and occlusion query
   //       read/write. You can partly base this on barrier commands for
   //       barriers already seen in the command buffer, but not fully because
   //       you do not know what barriers are coming next (if any).
   //
   //       It also isn't clear you can reasonably separate this from a specific
   //       bin/render configuration, i.e. by inserting special barrier jobs for
   //       for Vulkan pipeline barrier commands. Consider a multicore
   //       configuration like the current simulator/FPGAs, where jobs are
   //       scheduled on whatever core is available. The barrier jobs
   //       would have to flush/clean the caches in every core, as you cannot
   //       know in advance which cores will run the following jobs or have
   //       previously run jobs that the barrier is supposed to effect.

   // Note: no transform feedback related bin flags as it is not part of the
   //       Vulkan spec (it may get added later but it seems unlikely). It is
   //       instead expected that a geometry shader (if the feature is
   //       available) or compute shader will be used to write out to an SSBO,
   //       instead of using a fixed hardware block.

   v3d_barrier_flags binSyncFlags =
         V3D_BARRIER_TMU_CONFIG_READ
       | V3D_BARRIER_TMU_DATA_READ;

   v3d_barrier_flags renderSyncFlags =
         V3D_BARRIER_TMU_CONFIG_READ
       | V3D_BARRIER_TMU_DATA_READ
       | V3D_BARRIER_TMU_DATA_WRITE
       | V3D_BARRIER_TLB_IMAGE_READ
       | V3D_BARRIER_TLB_IMAGE_WRITE
       | V3D_BARRIER_TLB_OQ_READ
       | V3D_BARRIER_TLB_OQ_WRITE;

   m_td->CreateMasterControlLists(brJob, binSyncFlags, renderSyncFlags);

   // Add the bin/render job to the command list now
   m_commandList.push_back(brJob);

   if (m_td->GetQueryManager()->HasQueries())
      AddQueryPoolUpdateCommands();

   // m_td->m_delayedPostCommands contains a list of commands from a secondary command
   // buffer that must run after the binRender job that branches to the secondary runs.
   // This is used for occlusion query updates for queries inside a render pass in a
   // secondary buffer.
   for (const Command *cmd : m_td->m_delayedPostCommands)
      cmd->RecordInPrimary(*this);

   // Empty the post commands so they don't get reused by accident
   m_td->m_delayedPostCommands.clear();
}

void CommandBuffer::InsertExecutionBarrier(bvk::vector<Command*> &cmdList)
{
   auto cmd = NewCmd(CmdPipelineBarrier);
   cmdList.push_back(cmd);
}

void CommandBuffer::CmdNextSubpass(
   VkSubpassContents  contents) noexcept
{
   CMD_BEGIN

   assert(m_mode == eRECORDING);
   assert(m_level == VK_COMMAND_BUFFER_LEVEL_PRIMARY);

   // Close the previous subpass control list
   m_td->FinalizeControlList();

   RenderPass *rp = m_td->m_curRenderPassState.renderPass;

   const auto *group = rp->GroupForSubpass(m_td->m_curSubpassIndex);
   const auto *nextGroup = rp->GroupForSubpass(m_td->m_curSubpassIndex + 1);

   if (group != nextGroup)
   {
      // The subpassGroup for the next subpass is different to the current one, so we need
      // to add a bin/render job with a new MCL
      BuildHardwareJob();

      // If there is a dependency from the next subpass to this, add a barrier
      if (nextGroup && nextGroup->m_preBarrier)
         InsertExecutionBarrier();
   }

   m_td->NextSubpass(contents);

   CMD_END
}

void CommandBuffer::CmdEndRenderPass() noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   assert(m_level == VK_COMMAND_BUFFER_LEVEL_PRIMARY);
   assert(InRenderPass());

   // Close the last control list
   m_td->FinalizeControlList();

   // Final subpass always needs a bin/render job and MCL
#if V3D_VER_AT_LEAST(4,1,34,0)
   BuildHardwareJob();
#else
   if (!m_td->m_curState.CullEverything())
      BuildHardwareJob();
#endif

   m_td->EndRenderPass();

   if (m_td->m_curRenderPassState.renderPass->PostBarrierExternal())
      InsertExecutionBarrier();

   CMD_END
}

void CommandBuffer::CmdExecuteCommands(
   uint32_t                 commandBufferCount,
   const VkCommandBuffer   *pCommandBuffers) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   assert(m_level == VK_COMMAND_BUFFER_LEVEL_PRIMARY);

   // We do not support inheritedQueries
   assert(!m_td->GetQueryManager()->HasActiveQueries());

   // Run over all the secondary command buffers referencing all of their
   // commands into this primary one. Commands which have control-lists attached
   // will get their master control lists created now.
   for (uint32_t i = 0; i < commandBufferCount; i++)
   {
      const CommandBuffer *cb = fromHandle<CommandBuffer>(pCommandBuffers[i]);
      assert(cb->m_level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);

      if (InRenderPass())
      {
         if (cb->CommandList().size() > 0)
            m_td->NeedControlList();

         // Add the pre-execute commands to the primary list first.
         // This will be query counter zeroing and pipeline barriers.
         for (const Command *cmd : cb->m_secondaryPreCommandList)
            cmd->RecordInPrimary(*this);

         // Dispatch from each command to record in the current control list.
         for (const Command *cmd : cb->CommandList())
            cmd->RecordInPrimary(*this);

         // Copy the commands from the secondaries postCommandList into our delayedPost list.
         // These will get executed after the binRender job that calls the secondary command
         // buffer has run. Used for occlusion query updates that originated in the secondary
         // command buffer.
         m_td->m_delayedPostCommands = cb->m_secondaryPostCommandList;
      }
      else
      {
         // We are outside a render pass. Make a command that simply schedules all
         // the commands in the secondary list.
         auto secondaryExecCmd = NewObject<CmdSecondaryExecuteObj>(cb);
         m_commandList.push_back(secondaryExecCmd);
      }
   }
   CMD_END
}

void CommandBuffer::RecordSecondaryControlList(const CmdControlListObj &cl)
{
   // Branch to the secondary control lists
   v3d_cl_branch_sub(CLPtr(), cl.m_firstInnerControlListAddr);
}

void CommandBuffer::RecordSecondaryControlListFlush(const CmdFlushControlListObj &cl)
{
   // Branch to the secondary control lists
   v3d_cl_branch_sub(CLPtr(), cl.m_firstInnerControlListAddr);

   FlushPrimaryControlList();
}

void CommandBuffer::RecordSecondaryDeferredClear(
   uint32_t attachmentCount, const VkClearAttachment *pAttachments,
   uint32_t rectCount, const VkClearRect *pRects)
{
   ClearAttachments(attachmentCount, pAttachments, rectCount, pRects);
}

void CommandBuffer::AppendCommandReference(const Command &cmd)
{
   auto clone = NewObject<CommandReference>(cmd);
   m_commandList.push_back(clone);
}

#if !V3D_VER_AT_LEAST(4,1,34,0)

void CommandBuffer::RecordSecondaryDeferredClip(const CmdSecondaryDeferredClipObj &clip)
{
   InsertClampedClipRecord(clip.m_x, clip.m_y, clip.m_maxX, clip.m_maxY);
}

void CommandBuffer::InsertClampedClipRecord(int x, int y, int xMax, int yMax)
{
   // Incorporate framebuffer size and insert clip record
   assert(m_td->m_curRenderPassState.framebuffer != NULL);

   VkExtent3D fbDims;
   m_td->m_curRenderPassState.framebuffer->Dimensions(&fbDims);
   int xmax = std::min(xMax, static_cast<int>(fbDims.width));
   int ymax = std::min(yMax, static_cast<int>(fbDims.height));

   if (x >= xmax || y >= ymax)
   {
      m_td->m_curState.SetCullEverything(true);
      v3d_cl_clip(CLPtr(), x, y, 1, 1);
   }
   else
   {
      m_td->m_curState.SetCullEverything(false);
      v3d_cl_clip(CLPtr(), x, y, xmax - x, ymax - y);
   }
}

#endif

//////////////////////////////////////////////////////////////////////////////////
// SYNCHRONIZATION COMMANDS
//////////////////////////////////////////////////////////////////////////////////
void CommandBuffer::CmdSetEvent(
   bvk::Event           *event,
   VkPipelineStageFlags  stageMask) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   auto cmd = NewCmd(CmdSetEvent);
   cmd->m_event     = event;
   cmd->m_stageMask = stageMask;

   InsertExecutionBarrier();
   m_commandList.push_back(cmd);
   CMD_END
}

void CommandBuffer::CmdResetEvent(
   bvk::Event           *event,
   VkPipelineStageFlags  stageMask) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   auto cmd = NewCmd(CmdResetEvent);
   cmd->m_event     = event;
   cmd->m_stageMask = stageMask;

   InsertExecutionBarrier();
   m_commandList.push_back(cmd);
   CMD_END
}

void CommandBuffer::FlushPrimaryControlList()
{
   assert(m_level == VK_COMMAND_BUFFER_LEVEL_PRIMARY);
   assert(InRenderPass());

   // Close the previous control list
   m_td->FinalizeControlList();

   // Ensure the render targets are saved when we flush
   m_td->SetForceRTStores(true);

   BuildHardwareJob();
   InsertExecutionBarrier();

   // Ensure the render targets are reloaded after
   m_td->SetForceRTLoads(true);

   m_td->RestartControlList();
}

void CommandBuffer::FlushSecondaryControlList()
{
   assert(m_level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);
   assert(InRenderPass());

   v3d_addr_t start = CurSubpass()->controlList.Start();
   if (start != 0)
   {
      // Close the previous control list
      m_td->FinalizeControlList();

      CmdFlushControlListObj *clObj = NewObject<CmdFlushControlListObj>(start);

      // Add the control list object to the command list now
      m_commandList.push_back(clObj);

      // Reset the control list
      CurSubpass()->controlList = {};

      // Restart control list
      m_td->NeedControlList();
   }
}

void CommandBuffer::CmdWaitEvents(
   uint32_t                       eventCount,
   const VkEvent                 *pEvents,
   VkPipelineStageFlags           srcStageMask,
   VkPipelineStageFlags           dstStageMask,
   uint32_t                       memoryBarrierCount,
   const VkMemoryBarrier         *pMemoryBarriers,
   uint32_t                       bufferMemoryBarrierCount,
   const VkBufferMemoryBarrier   *pBufferMemoryBarriers,
   uint32_t                       imageMemoryBarrierCount,
   const VkImageMemoryBarrier    *pImageMemoryBarriers) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   auto cmd = NewCmd(CmdWaitEvents);

   cmd->m_eventCount               = eventCount;
   cmd->m_srcStageMask             = srcStageMask;
   cmd->m_dstStageMask             = dstStageMask;
   cmd->m_memoryBarrierCount       = memoryBarrierCount;
   cmd->m_bufferMemoryBarrierCount = bufferMemoryBarrierCount;
   cmd->m_imageMemoryBarrierCount  = imageMemoryBarrierCount;

   CloneObjArray(&cmd->m_events, pEvents, eventCount);
   CloneObjArray(&cmd->m_memoryBarriers, pMemoryBarriers, memoryBarrierCount);
   CloneObjArray(&cmd->m_bufferMemoryBarriers, pBufferMemoryBarriers, bufferMemoryBarrierCount);
   CloneObjArray(&cmd->m_imageMemoryBarriers, pImageMemoryBarriers, imageMemoryBarrierCount);

   m_commandList.push_back(cmd);
   CMD_END
}

void CommandBuffer::CmdPipelineBarrier(
   VkPipelineStageFlags           srcStageMask,
   VkPipelineStageFlags           dstStageMask,
   VkDependencyFlags              dependencyFlags,
   uint32_t                       memoryBarrierCount,
   const VkMemoryBarrier         *pMemoryBarriers,
   uint32_t                       bufferMemoryBarrierCount,
   const VkBufferMemoryBarrier   *pBufferMemoryBarriers,
   uint32_t                       imageMemoryBarrierCount,
   const VkImageMemoryBarrier    *pImageMemoryBarriers) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);

   // If vkCmdPipelineBarrier is called outside a render pass instance, then the first set
   // of commands is all prior commands submitted to the queue and recorded in the command
   // buffer and the second set of commands is all subsequent commands recorded in the command
   // buffer and submitted to the queue.

   // If vkCmdPipelineBarrier is called inside a render pass instance, then the first set
   // of commands is all prior commands in the same subpass and the second set of commands
   // is all subsequent commands in the same subpass.

   // We mostly only care when outside of a render pass. When inside, all control list items
   // are executed in order, so we automatically follow the rule - except when an attachment
   // is used as both input and output in the same subpass. In that case, we have to insert
   // an execution barrier which involves flushing the pipeline.
   // The dEQP-VK.renderpass.formats.*.input.*.self_dep* test these cases (for primary buffers).
   if (!InRenderPass())
   {
      // The spec says that waiting for top of pipe, or at bottom of pipe
      // doesn't delay command execution.
      if ((srcStageMask & ~VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)    != 0 &&
          (dstStageMask & ~VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) != 0)
      {
         auto cmd = NewCmd(CmdPipelineBarrier);
         m_commandList.push_back(cmd);
      }
   }
   else if (m_td->m_curSubpassGroup->m_sharesInputAndOutput)
   {
      // Shared attachment in this subpass, so we must flush to memory
      if (m_level == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
         FlushPrimaryControlList();
      else
         FlushSecondaryControlList();
   }

   CMD_END
}

} // namespace bvk
