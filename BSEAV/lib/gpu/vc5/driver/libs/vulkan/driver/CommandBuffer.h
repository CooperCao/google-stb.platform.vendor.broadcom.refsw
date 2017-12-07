/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>
#include <memory.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Dispatchable.h"
#include "Allocating.h"
#include "SysMemCmdBlock.h"
#include "DevMemCmdBlock.h"
#include "DevMemDataBlock.h"
#include "CommandPool.h"
#include "CmdBufState.h"
#include "RenderPass.h"
#include "CommandBufferBuilder.h"
#include "SysMemCmdBlock.h"
#include "ArenaAllocator.h"

namespace bvk {

struct CmdPipelineLayoutCopy;
class Command;
class CPUCommand;
class CmdBinRenderJobObj;
class CmdComputeJobObj;
class ColorAspectCommandBuilder;
class CmdControlListObj;
class CmdFlushControlListObj;
#if !V3D_VER_AT_LEAST(4,1,34,0)
class CmdSecondaryDeferredClipObj;
#endif
class CommandReference;

// The command buffer object
class CommandBuffer : public NonCopyable, public Dispatchable, public Allocating
{
public:
   CommandBuffer(
      const VkAllocationCallbacks         *pCallbacks,
      bvk::CommandPool                    *pPool,
      bvk::Device                         *pDevice,
      const VkCommandBufferAllocateInfo   *pAllocateInfo);

   ~CommandBuffer() noexcept;

   VkResult BeginCommandBuffer(
      const VkCommandBufferBeginInfo   *pBeginInfo) noexcept;

   VkResult EndCommandBuffer() noexcept;

   VkResult ResetCommandBuffer(
      VkCommandBufferResetFlags   flags) noexcept;

   void CmdBindPipeline(
      VkPipelineBindPoint   pipelineBindPoint,
      bvk::Pipeline        *pipeline) noexcept;

   void CmdSetViewport(
      uint32_t           firstViewport,
      uint32_t           viewportCount,
      const VkViewport  *pViewports) noexcept;

   void CmdSetScissor(
      uint32_t        firstScissor,
      uint32_t        scissorCount,
      const VkRect2D *pScissors) noexcept;

   void CmdSetLineWidth(
      float  lineWidth) noexcept;

   void CmdSetDepthBias(
      float  depthBiasConstantFactor,
      float  depthBiasClamp,
      float  depthBiasSlopeFactor) noexcept;

   void CmdSetBlendConstants(
      const float  blendConstants[4]) noexcept;

   void CmdSetDepthBounds(
      float  minDepthBounds,
      float  maxDepthBounds) noexcept;

   void CmdSetStencilCompareMask(
      VkStencilFaceFlags faceMask,
      uint32_t           compareMask) noexcept;

   void CmdSetStencilWriteMask(
      VkStencilFaceFlags faceMask,
      uint32_t           writeMask) noexcept;

   void CmdSetStencilReference(
      VkStencilFaceFlags faceMask,
      uint32_t           reference) noexcept;

   void CmdBindDescriptorSets(
      VkPipelineBindPoint      pipelineBindPoint,
      bvk::PipelineLayout     *layout,
      uint32_t                 firstSet,
      uint32_t                 descriptorSetCount,
      const VkDescriptorSet   *pDescriptorSets,
      uint32_t                 dynamicOffsetCount,
      const uint32_t          *pDynamicOffsets) noexcept;

   void CmdBindIndexBuffer(
      bvk::Buffer *buffer,
      VkDeviceSize offset,
      VkIndexType  indexType) noexcept;

   void CmdBindVertexBuffers(
      uint32_t              firstBinding,
      uint32_t              bindingCount,
      const VkBuffer       *pBuffers,
      const VkDeviceSize   *pOffsets) noexcept;

   void CmdDraw(
      uint32_t  vertexCount,
      uint32_t  instanceCount,
      uint32_t  firstVertex,
      uint32_t  firstInstance) noexcept;

   void CmdDrawIndexed(
      uint32_t  indexCount,
      uint32_t  instanceCount,
      uint32_t  firstIndex,
      int32_t   vertexOffset,
      uint32_t  firstInstance) noexcept;

   void CmdDrawIndirect(
      bvk::Buffer *buffer,
      VkDeviceSize offset,
      uint32_t     drawCount,
      uint32_t     stride) noexcept;

   void CmdDrawIndexedIndirect(
      bvk::Buffer *buffer,
      VkDeviceSize offset,
      uint32_t     drawCount,
      uint32_t     stride) noexcept;

   void CmdDispatch(
      uint32_t  x,
      uint32_t  y,
      uint32_t  z) noexcept;

   void CmdDispatchIndirect(
      bvk::Buffer *buffer,
      VkDeviceSize offset) noexcept;

   void CmdCopyBuffer(
      bvk::Buffer          *srcBuffer,
      bvk::Buffer          *dstBuffer,
      uint32_t              regionCount,
      const VkBufferCopy   *pRegions) noexcept;

   void CmdCopyImage(
      bvk::Image        *srcImage,
      VkImageLayout      srcImageLayout,
      bvk::Image        *dstImage,
      VkImageLayout      dstImageLayout,
      uint32_t           regionCount,
      const VkImageCopy *pRegions) noexcept;

   void CmdBlitImage(
      bvk::Image        *srcImage,
      VkImageLayout      srcImageLayout,
      bvk::Image        *dstImage,
      VkImageLayout      dstImageLayout,
      uint32_t           regionCount,
      const VkImageBlit *pRegions,
      VkFilter           filter) noexcept;

   void CmdCopyBufferToImage(
      bvk::Buffer             *srcBuffer,
      bvk::Image              *dstImage,
      VkImageLayout            dstImageLayout,
      uint32_t                 regionCount,
      const VkBufferImageCopy *pRegions) noexcept;

   void CmdCopyImageToBuffer(
      bvk::Image              *srcImage,
      VkImageLayout            srcImageLayout,
      bvk::Buffer             *dstBuffer,
      uint32_t                 regionCount,
      const VkBufferImageCopy *pRegions) noexcept;

   void CmdUpdateBuffer(
      bvk::Buffer    *dstBuffer,
      VkDeviceSize    dstOffset,
      VkDeviceSize    dataSize,
      const void     *pData) noexcept;

   void CmdFillBuffer(
      bvk::Buffer *dstBuffer,
      VkDeviceSize dstOffset,
      VkDeviceSize size,
      uint32_t     data) noexcept;

   void CmdClearColorImage(
      bvk::Image                    *image,
      VkImageLayout                  imageLayout,
      const VkClearColorValue       *pColor,
      uint32_t                       rangeCount,
      const VkImageSubresourceRange *pRanges) noexcept;

   void CmdClearDepthStencilImage(
      bvk::Image                       *image,
      VkImageLayout                     imageLayout,
      const VkClearDepthStencilValue   *pDepthStencil,
      uint32_t                          rangeCount,
      const VkImageSubresourceRange    *pRanges) noexcept;

   void CmdClearAttachments(
      uint32_t                 attachmentCount,
      const VkClearAttachment *pAttachments,
      uint32_t                 rectCount,
      const VkClearRect       *pRects) noexcept;

   void CmdResolveImage(
      bvk::Image           *srcImage,
      VkImageLayout         srcImageLayout,
      bvk::Image           *dstImage,
      VkImageLayout         dstImageLayout,
      uint32_t              regionCount,
      const VkImageResolve *pRegions) noexcept;

   void CmdSetEvent(
      bvk::Event           *event,
      VkPipelineStageFlags  stageMask) noexcept;

   void CmdResetEvent(
      bvk::Event           *event,
      VkPipelineStageFlags  stageMask) noexcept;

   void CmdWaitEvents(
      uint32_t                       eventCount,
      const VkEvent                 *pEvents,
      VkPipelineStageFlags           srcStageMask,
      VkPipelineStageFlags           dstStageMask,
      uint32_t                       memoryBarrierCount,
      const VkMemoryBarrier         *pMemoryBarriers,
      uint32_t                       bufferMemoryBarrierCount,
      const VkBufferMemoryBarrier   *pBufferMemoryBarriers,
      uint32_t                       imageMemoryBarrierCount,
      const VkImageMemoryBarrier    *pImageMemoryBarriers) noexcept;

   void CmdPipelineBarrier(
      VkPipelineStageFlags           srcStageMask,
      VkPipelineStageFlags           dstStageMask,
      VkDependencyFlags              dependencyFlags,
      uint32_t                       memoryBarrierCount,
      const VkMemoryBarrier         *pMemoryBarriers,
      uint32_t                       bufferMemoryBarrierCount,
      const VkBufferMemoryBarrier   *pBufferMemoryBarriers,
      uint32_t                       imageMemoryBarrierCount,
      const VkImageMemoryBarrier    *pImageMemoryBarriers) noexcept;

   void CmdBeginQuery(
      bvk::QueryPool       *queryPool,
      uint32_t              query,
      VkQueryControlFlags   flags) noexcept;

   void CmdEndQuery(
      bvk::QueryPool *queryPool,
      uint32_t        query) noexcept;

   void CmdResetQueryPool(
      bvk::QueryPool *queryPool,
      uint32_t        firstQuery,
      uint32_t        queryCount) noexcept;

   void CmdWriteTimestamp(
      VkPipelineStageFlagBits  pipelineStage,
      bvk::QueryPool          *queryPool,
      uint32_t                 query) noexcept;

   void CmdCopyQueryPoolResults(
      bvk::QueryPool    *queryPool,
      uint32_t           firstQuery,
      uint32_t           queryCount,
      bvk::Buffer       *dstBuffer,
      VkDeviceSize       dstOffset,
      VkDeviceSize       stride,
      VkQueryResultFlags flags) noexcept;

   void CmdPushConstants(
      bvk::PipelineLayout  *layout,
      VkShaderStageFlags    stageFlags,
      uint32_t              offset,
      uint32_t              size,
      const void           *pValues) noexcept;

   void CmdBeginRenderPass(
      const VkRenderPassBeginInfo   *pRenderPassBegin,
      VkSubpassContents              contents) noexcept;

   void CmdNextSubpass(
      VkSubpassContents  contents) noexcept;

   void CmdEndRenderPass() noexcept;

   void CmdExecuteCommands(
      uint32_t                 commandBufferCount,
      const VkCommandBuffer   *pCommandBuffers) noexcept;

   void CmdDebugMarkerBeginEXT(
      const VkDebugMarkerMarkerInfoEXT *pMarkerInfo) noexcept;

   void CmdDebugMarkerEndEXT() noexcept;

   void CmdDebugMarkerInsertEXT(
      const VkDebugMarkerMarkerInfoEXT *pMarkerInfo) noexcept;

   // Implementation specific from this point on
   const bvk::vector<Command*> &CommandList() const { return m_commandList; }

   DevMemCmdBlock *AddDeviceCmdBlock()
   {
      auto block = m_pool->AcquireDeviceCmdBlock();
      m_deviceBlocks.push_back(block);
      return block;
   }

   void DrawClearRect(uint32_t rtIndex, const VkClearAttachment &ca, const VkRect2D &rect,
                      GFX_LFMT_T format, const VkImageSubresourceRange &srr);
   void DrawClearRects(uint32_t rtIndex, const VkClearAttachment &ca,
                       uint32_t rectCount, const VkClearRect *rects, GFX_LFMT_T format,
                       const VkImageSubresourceRange &srr);

   // Called when a device memory data region is needed (for things like
   // uniforms and attributes) - not for control list commands.
   void NewDevMemRange(DevMemRange *range, size_t size, size_t align)
   {
      m_devDataArena.Allocate(range, size, align);
   }

   // Called when a device memory data region is needed for queries
   // - not for control list commands.
   void NewDevMemQueryRange(DevMemRange *range, size_t size, size_t align)
   {
      m_devQueryArena.Allocate(range, size, align);
   }

   // Dispatched via CmdControlListObj to record a secondary command buffer
   void RecordSecondaryControlList(const CmdControlListObj &cl);
   void RecordSecondaryControlListFlush(const CmdFlushControlListObj &cl);

#if !V3D_VER_AT_LEAST(4,1,34,0)
   // Dispatched via CmdSecondaryDeferredClipObj to record a secondary command buffer clip
   void RecordSecondaryDeferredClip(const CmdSecondaryDeferredClipObj &clip);
#endif

   void RecordSecondaryDeferredClear(uint32_t attachmentCount, const VkClearAttachment *pAttachments,
                                     uint32_t rectCount, const VkClearRect *pRects);

   void AppendCommandReference(const Command &cmd);

   // A deleter for objects allocated with NewObject.
   template<typename T>
   struct DeleteObject
   {
      void operator()(T* ptr) { ptr->~T(); }
   };

   // A unique-ptr for objects allocated with NewObject.
   template<typename T>
   using UniquePtr = std::unique_ptr<T, DeleteObject<T>>;

   friend class GraphicsPipeline;
   friend class ComputePipeline;
   friend class CmdBufState;
   friend class CommandBufferBuilder;

public:
   enum CmdBufferMode
   {
      eINITIAL,
      eRECORDING,
      eEXECUTABLE
   };

private:
   // These methods work on the system memory blocks

   // Allocate and construct an array of objects in pooled system memory
   template<typename T>
   T *NewObjectArray(uint32_t numObjs)
   {
      if (numObjs > 0)
      {
         size_t size = sizeof(T) * numObjs;
         void  *ptr  = nullptr;

         m_sysMemArena.Allocate(&ptr, size, alignof(T));

         return static_cast<T*>(ptr);
      }
      else
         return nullptr;
   }

   // Allocate and construct a new object in pooled system memory
   template<typename T, class... Types>
   T *NewObject(Types&&... args)
   {
      T *ptr = NewObjectArray<T>(1);
      return new (ptr) T(std::forward<Types>(args)...);
   }

   template<typename T>
   void CloneObjArray(T **dst, const T *src, uint32_t numObjs)
   {
      if (numObjs > 0)
      {
         T *ar = NewObjectArray<T>(numObjs);
         memcpy(ar, src, numObjs * sizeof(T));
         *dst = ar;
      }
      else
         *dst = nullptr;
   }

   void CmdDispatchCommon(
      const ComputePipeline& pipe,
      CmdComputeJobObj& cmd,
      DevMemRange& uniformMem) noexcept;
   void DrawCallPreamble();
   void BuildHardwareJob() noexcept;
   void DrawRectVertexData(DevMemRange *devMem, uint32_t *dataMaxIndex,
                           uint32_t rectCount, const VkRect2D *rects, uint32_t z);

   void AddQueryPoolUpdateCommands(bvk::vector<Command*> &cmdList);
   void AddQueryPoolUpdateCommands() { AddQueryPoolUpdateCommands(m_commandList); }

   void ReleaseResources();
   void LogMemoryUsage() const;

   void InsertExecutionBarrier(bvk::vector<Command*> &cmdList);
   void InsertExecutionBarrier() { InsertExecutionBarrier(m_commandList); }

   void FlushPrimaryControlList();
   void FlushSecondaryControlList();
   void SplitSecondaryControlList();

#if !V3D_VER_AT_LEAST(4,1,34,0)
   void InsertClampedClipRecord(int x, int y, int xMax, int yMax);
#endif

   // vkCmdResolveImage helpers
   void ResolveImageRegionTLB(
         bvk::Image *srcImage, bvk::Image *dstImage,
         const VkImageResolve &region);

   void ResolveImageRegionTMU(
         bvk::Image *srcImage, bvk::Image *dstImage,
         const VkImageResolve &region);

   void BlitImageRegion(
         bvk::Image        *srcImage,
         bvk::Image        *dstImage,
         const VkImageBlit &region,
         VkFilter           filter);

   void DrawSingleLayerClearRects(ControlListBuilder &cb, uint32_t rtIndex,
                                  const VkClearAttachment &ca,
                                  uint32_t rectCount, const VkRect2D *rects, GFX_LFMT_T format);

   void DrawMultiLayerClearRects(uint32_t rtIndex, const VkClearAttachment &ca,
                                 uint32_t rectCount, const VkClearRect *rects,
                                 GFX_LFMT_T format, const VkImageSubresourceRange &srr);

   void ClearAttachments(uint32_t attachmentCount, const VkClearAttachment *pAttachments,
                         uint32_t rectCount, const VkClearRect *pRects) noexcept;

   static bool PrimaryClearRequired(uint32_t rectCount, const VkClearRect *rects);

   // Easy transient-data accessors
   CommandBufferBuilder::SubPass *CurSubpass()       { return m_td->m_curSubpass; }
   bool InRenderPass() const                         { return m_td->m_inRenderPass; }
   CmdBufState &CurState()                           { return m_td->m_curState; }

   // Temporaries to keep control list generation going until we can refactor it
   // to a bin/render job subclass
   uint8_t **CLPtr()                                 { return m_td->CLPtr(); }
   uint8_t **CLPtr(size_t addSize)                   { return m_td->CLPtr(addSize); }

private:
   Device                          *m_device;         // Our device
   CommandPool                     *m_pool;           // The pool our memory comes from
   CmdBufferMode                    m_mode = eINITIAL;// Current mode
   VkCommandBufferUsageFlags        m_usageFlags = 0; // Usage flags
   VkCommandBufferLevel             m_level;          // Primary or secondary

   // An arena allocator for system memory that will use the CommandPool
   ArenaAllocator<SysMemCmdBlock, void*>  m_sysMemArena;

   // An arena allocator for device memory that will use the CommandPool.
   // Note : this memory is read-only from the V3D.
   // Note : this is for device data blocks, not device control list data.
   ArenaAllocator<DevMemDataBlock, DevMemRange>  m_devDataArena;

   // An arena allocator for device memory that will use the CommandPool.
   // Note : this memory is read-write from the V3D.
   // Note : this is for device query memory, not device control list data, or read-only data.
   ArenaAllocator<DevMemQueryBlock, DevMemRange>  m_devQueryArena;

   // Control list memory needs special handling, so doesn't use an arena allocator
   bvk::vector<DevMemCmdBlock*>     m_deviceBlocks;   // Chunks of control list memory

   // The result code that will be returned during vkEndCommandBuffer
   VkResult                         m_status = VK_SUCCESS;

   // The list of abstract commands that we will schedule when this
   // command buffer is queued. All memory comes from the sysMemArena which
   // uses the CommandPool.
   bvk::vector<Command*>            m_commandList;

   // Commands in an 'inside-renderpass' secondary command buffer that will execute before the
   // primary control-list which references it has completed. This is used for occlusion query
   // zeroing which originated in the secondary command buffer.
   bvk::vector<Command*>            m_secondaryPreCommandList;

   // Commands in an 'inside-renderpass' secondary command buffer that will execute after the
   // primary control-list which references it has completed. This is used for occlusion query
   // updates which originated in the secondary command buffer.
   bvk::vector<Command*>            m_secondaryPostCommandList;

   // CPU Commands to "execute" immediately when the command buffer is destroyed
   // to clean up for example internally created images.
   bvk::vector<CPUCommand*>         m_cleanupCommandList;

   // Transient data only needed during command buffer building.
   // This is deliberately kept in a separate struct to avoid polluting
   // the actual command buffer data from the stuff only needed when building.
   // It also keeps command buffers smaller since we delete it when we're done
   // building.
   CommandBufferBuilder            *m_td = nullptr;
};


//////////////////////////////////////////////////////////////////////////////
// Private command buffer implementation helpers as the class is split across
// multiple CB*.cpp files as well as CommandBuffer.cpp. We all hate macros,
// but these make life a lot easier.

// Create a new command from the system memory pool.
// These don't need to be explicitly freed.
#define NewCmd(name) NewObject<name##Obj>()

// Used at the start of every CommandBuffer command
#define CMD_BEGIN \
   if (m_status != VK_SUCCESS)\
      return;\
   try\
   {

// Used at the end of every CommandBuffer command
#define CMD_END \
   }\
   catch (const std::bad_alloc &)        { m_status = VK_ERROR_OUT_OF_HOST_MEMORY;   }\
   catch (const bvk::bad_device_alloc &) { m_status = VK_ERROR_OUT_OF_DEVICE_MEMORY; }\
   catch (const bvk::nothing_to_do &)    {}

} // namespace bvk
