/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "AllObjects.h"
#include "Common.h"
#include "SchedTask.h"

namespace bvk {

// Base class for all commands held in a command buffer
class Command
{
public:
   virtual ~Command() {};

   // Schedule this command to be executed
   virtual JobID Schedule(
      // Queue this command is being executed on.
      Queue &queue,
      // Must wait on all these dependencies before continuing
      const SchedDependencies &waitDeps) const { return 0; }

   // Insert from a secondary buffer into a primary.
   virtual void RecordInPrimary(CommandBuffer &cmdBuf) const { assert(0); };

   // Returns the RTTI typename. Used only by tracing.
   virtual const char *CommandName() const { return typeid(*this).name(); }
};

// Base class for commands held only in a secondary command buffer
class SecondaryOnlyCommand : public Command
{
public:
   // Insert from a secondary buffer into a primary
   virtual void RecordInPrimary(CommandBuffer &cmdBuf) const = 0;
};

// This class simply references another command and forwards scheduling
// requests to it. Used in primary command buffers to reference commands
// in secondary ones.
class CommandReference : public Command
{
public:
   CommandReference(const Command &referTo) : m_reference(referTo) {}

   JobID Schedule(
      // Queue this command is being executed on.
      Queue &queue,
      // Must wait on all these dependencies before continuing
      const SchedDependencies &waitDeps) const override
   {
      return m_reference.Schedule(queue, waitDeps);
   }

   virtual const char *CommandName() const { return m_reference.CommandName(); }

private:
   const Command &m_reference;
};

// Intermediate helper class for CPU only commands to derive from
class CPUCommand : public Command
{
public:
   class CPUTask : public SchedTask
   {
   public:
      CPUTask(const CPUCommand *cmd) : m_cmd(cmd) {}
      JobID ScheduleTask(const SchedDependencies &deps) { return ScheduleCPUTask(deps); }
      void CPUExecute() { m_cmd->Execute(); }

   private:
      const CPUCommand *m_cmd;
   };

   JobID Schedule(Queue &queue, const SchedDependencies &deps) const override
   {
      CPUTask *task = new CPUTask(this);
      return task->ScheduleTask(deps);
   }

   // Will be called on a separate thread
   virtual void Execute() const = 0;
};

class CmdBinRenderJobObj : public Command
{
public:
   class Task : public SchedTask
   {
   public:
      Task(const CmdBinRenderJobObj *cmd) : m_cmd(cmd) {}
      ~Task();

      JobID ScheduleTask(const SchedDependencies &deps);

      static uint32_t BinRenderJobCounter() { return m_brJobCounter; }

   private:
      const CmdBinRenderJobObj *m_cmd;
      static uint32_t           m_brJobCounter;    // Used only for debug functionality
   };

   // Main constructor
   CmdBinRenderJobObj(PhysicalDevice *physicalDevice)
     : m_layers(1)
   {}

   // Copy constructor
   CmdBinRenderJobObj(const CmdBinRenderJobObj &rhs);

   void SetBinStart(v3d_addr_t addr)
   {
      v3d_subjob subjob;
      subjob.start = addr;
      m_binSubjobs.push_back(subjob);
   }
   void SetBinEnd(v3d_addr_t addr)   { m_binSubjobs.back().end = addr; }

   void SetRenderStart(v3d_addr_t addr)
   {
      v3d_subjob subjob;
      subjob.start = addr;
      m_renderSubjobs.push_back(subjob);
   }
   void SetRenderEnd(v3d_addr_t addr)   { m_renderSubjobs.back().end = addr; }

   JobID Schedule(Queue &queue, const SchedDependencies &deps) const override
   {
      Task *task = new Task(this);
      return task->ScheduleTask(deps);
   }

public:
   std::vector<v3d_subjob>          m_binSubjobs;
   unsigned                         m_layers;
   std::vector<v3d_subjob>          m_renderSubjobs;
   v3d_bin_render_details           m_brDetails = {};

   uint32_t                         m_numZPrepassBins = 0;
};

class CmdComputeJobObj : public Command, bvk::NonCopyable
{
public:
#if !V3D_USE_CSD
   JobID Schedule(Queue &queue, const SchedDependencies &deps) const override;
#endif

protected:
   struct Last // for VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
   {
      std::mutex mutex{}; // protects jobID and allows atomic submit of preprocess and compute job pair.
      JobID jobID{};
   };

#if !V3D_USE_CSD
   virtual const uint32_t* PatchAndGetNumWorkGroups() const noexcept = 0;
#endif

   friend class CommandBuffer;

#if V3D_USE_CSD
   v3d_compute_subjob m_subjob;
#else
   const LinkResult::CS* m_computeShader{};
   v3d_addr_t m_unifsAddr{};
   gmem_handle_t m_dispatchClMem{};
   uint32_t m_dispatchClOffset{};
   v3d_subjob m_subjob{};
   mutable CommandBuffer::UniquePtr<Last> m_last{};
#endif
};

class CmdComputeImmediateJobObj : CmdComputeJobObj
{
public:
#if V3D_USE_CSD
   JobID Schedule(Queue &queue, const SchedDependencies &deps) const override;
#endif

protected:
#if !V3D_USE_CSD
   const uint32_t* PatchAndGetNumWorkGroups() const noexcept { return m_numWorkGroups; }
#endif

   friend class CommandBuffer;

#if !V3D_USE_CSD
   uint32_t m_numWorkGroups[3];
#endif
};

class CmdComputeIndirectJobObj : CmdComputeJobObj
{
public:
#if V3D_USE_CSD
   JobID Schedule(Queue &queue, const SchedDependencies &deps) const override;
#endif

protected:
   const uint32_t* PatchAndGetNumWorkGroups() const noexcept;

   friend class CommandBuffer;

   const ComputePipeline::PreprocessPatchInfo* m_patchInfo{};
   gmem_handle_t m_bufferHandle{};
   gmem_handle_t m_uniformsHandle{};
   v3d_size_t m_bufferOffset{};
   uint32_t m_uniformsOffset{};
#if V3D_USE_CSD
   mutable CommandBuffer::UniquePtr<Last> m_last{};
#endif
};

// Represents a control list in a commandList.
// Never scheduled directly, as they only exist in secondary command buffers.
class CmdControlListObj : public SecondaryOnlyCommand
{
public:
   // Main constructor
   CmdControlListObj(v3d_addr_t startAddr) :
      m_firstInnerControlListAddr(startAddr)
   {}

   void RecordInPrimary(CommandBuffer &cmdBuf) const
   {
      if (m_firstInnerControlListAddr != 0)
         cmdBuf.RecordSecondaryControlList(*this);
   }

public:
   v3d_addr_t  m_firstInnerControlListAddr = 0;
};

// Causes a hard pipeline barrier in a secondary control list.
// Never scheduled directly, as they only exist in secondary command buffers.
class CmdFlushControlListObj : public SecondaryOnlyCommand
{
public:
   // Main constructor
   CmdFlushControlListObj(v3d_addr_t startAddr) :
      m_firstInnerControlListAddr(startAddr)
   {}

   void RecordInPrimary(CommandBuffer &cmdBuf) const
   {
      if (m_firstInnerControlListAddr != 0)
         cmdBuf.RecordSecondaryControlListFlush(*this);
   }

public:
   v3d_addr_t  m_firstInnerControlListAddr = 0;
};

// Command that schedules a secondary command buffer (outside render pass).
// These only exist in primary command buffers.
class CmdSecondaryExecuteObj : public Command
{
public:
   CmdSecondaryExecuteObj(const CommandBuffer *cmdBuf) :
      m_secondaryBuffer(cmdBuf) {}

   JobID Schedule(Queue &queue, const SchedDependencies &deps) const override;

public:
   const CommandBuffer *m_secondaryBuffer = nullptr;
};

// Command that manages a deferred ClearAttachment record when the secondary
// command buffer is constructed without a known framebuffer.
// These only exist in secondary command buffers.
class CmdSecondaryDeferredClearObj : public SecondaryOnlyCommand
{
public:
   CmdSecondaryDeferredClearObj() {}

   void RecordInPrimary(CommandBuffer &cmdBuf) const
   {
      cmdBuf.RecordSecondaryDeferredClear(m_attachmentCount, m_attachments, m_rectCount, m_rects);
   }

public:
   uint32_t           m_attachmentCount;
   VkClearAttachment *m_attachments;
   uint32_t           m_rectCount;
   VkClearRect       *m_rects;
};

class CmdPipelineBarrierObj : public Command
{
public:
   JobID Schedule(Queue &queue, const SchedDependencies &deps) const override;

   void RecordInPrimary(CommandBuffer &cmdBuf) const
   {
      cmdBuf.AppendCommandReference(*this);
   }
};

class CmdSetEventObj : public Command
{
public:
   JobID Schedule(Queue &queue, const SchedDependencies &deps) const override
   {
      return m_event->ScheduleSignal(deps);
   }

   friend class CommandBuffer;

private:
   Event                *m_event;
   VkPipelineStageFlags  m_stageMask;
};

class CmdResetEventObj : public Command
{
public:
   friend class CommandBuffer;

   JobID Schedule(Queue &queue, const SchedDependencies &deps) const override
   {
      return m_event->ScheduleReset(deps);
   }

private:
   Event                *m_event;
   VkPipelineStageFlags  m_stageMask;
};

class CmdWaitEventsObj : public Command
{
public:
   friend class CommandBuffer;

   JobID Schedule(Queue &queue, const SchedDependencies &deps) const override;

private:
   uint32_t                 m_eventCount;
   VkEvent                 *m_events;
   VkPipelineStageFlags     m_srcStageMask;
   VkPipelineStageFlags     m_dstStageMask;
   uint32_t                 m_memoryBarrierCount;
   VkMemoryBarrier         *m_memoryBarriers;
   uint32_t                 m_bufferMemoryBarrierCount;
   VkBufferMemoryBarrier   *m_bufferMemoryBarriers;
   uint32_t                 m_imageMemoryBarrierCount;
   VkImageMemoryBarrier    *m_imageMemoryBarriers;
};

class CmdCopyBufferObj : public CPUCommand
{
public:
   friend class CommandBuffer;

   // Will be called on a separate thread
   void Execute() const
   {
      m_dstBuffer->CopyFromBuffer(m_srcBuffer, m_regionCount, m_regions);
   }

private:
   Buffer         *m_srcBuffer;
   Buffer         *m_dstBuffer;
   uint32_t        m_regionCount;
   VkBufferCopy   *m_regions;
};

class CmdCopyImageObj : public Command
{
public:
   friend class CommandBuffer;

   JobID Schedule(Queue &queue, const SchedDependencies &deps) const override
   {
      // We do not care about the layouts as the memory organisation is
      // determined by the tiling state of the images themselves
      //
      // We just call the helper on the destination to schedule the imageconv
      // job(s) required for the specified regions.
      return m_dstImage->CopyFromImage(m_srcImage, m_regionCount, m_regions, deps);
   }

private:
   Image             *m_srcImage;
   VkImageLayout      m_srcImageLayout;
   Image             *m_dstImage;
   VkImageLayout      m_dstImageLayout;
   uint32_t           m_regionCount;
   VkImageCopy       *m_regions;
};

class CmdCopyBufferToImageObj : public Command
{
public:
   friend class CommandBuffer;

   JobID Schedule(Queue &queue, const SchedDependencies &deps) const override
   {
      return m_dstImage->CopyFromBuffer(m_srcBuffer, m_regionCount, m_regions, deps);
   }

private:
   Buffer               *m_srcBuffer;
   Image                *m_dstImage;
   VkImageLayout         m_dstImageLayout;
   uint32_t              m_regionCount;
   VkBufferImageCopy    *m_regions;
};

class CmdCopyImageToBufferObj : public Command
{
public:
   friend class CommandBuffer;

   JobID Schedule(Queue &queue, const SchedDependencies &deps) const override
   {
      return m_srcImage->CopyToBuffer(m_dstBuffer, m_regionCount, m_regions, deps);
   }

private:
   Image             *m_srcImage;
   VkImageLayout      m_srcImageLayout;
   Buffer            *m_dstBuffer;
   uint32_t           m_regionCount;
   VkBufferImageCopy *m_regions;
};

class CmdUpdateBufferObj : public CPUCommand
{
public:
   friend class CommandBuffer;

   // Will be called on a separate thread
   void Execute() const
   {
      m_dstBuffer->UpdateBuffer(m_dstOffset, m_dataSize, m_data);
   }

private:
   Buffer         *m_dstBuffer;
   VkDeviceSize    m_dstOffset;
   VkDeviceSize    m_dataSize;
   void           *m_data;
};

class CmdFillBufferObj : public CPUCommand
{
public:
   friend class CommandBuffer;

   // Will be called on a separate thread
   void Execute() const
   {
      m_dstBuffer->FillBuffer(m_dstOffset, m_size, m_data);
   }

private:
   Buffer      *m_dstBuffer;
   VkDeviceSize m_dstOffset;
   VkDeviceSize m_size;
   uint32_t     m_data;
};

// Zeros all the h/w counters associated with a single query
class CmdZeroQueryHWCountersObj : public CPUCommand
{
public:
   friend class CommandBuffer;

   // Will be called on a separate thread
   void Execute() const
   {
      m_qde->pool->ZeroHWCounters(m_qde);
   }

   QueryDataEntry **GetQueryDataEntryPtrPtr()
   {
      return &m_qde;
   }

   void RecordInPrimary(CommandBuffer &cmdBuf) const
   {
      cmdBuf.AppendCommandReference(*this);
   }

private:
   QueryDataEntry *m_qde = nullptr;
};

// Zeros the query pool result values (not the h/w counters)
class CmdResetQueryPoolObj : public CPUCommand
{
public:
   friend class CommandBuffer;

   CmdResetQueryPoolObj(QueryPool *qp, uint32_t first, uint32_t count) :
      m_queryPool(qp),
      m_firstQuery(first),
      m_queryCount(count)
   {
   }

   // Will be called on a separate thread
   void Execute() const
   {
      m_queryPool->ResetNow(m_firstQuery, m_queryCount);
   }

   void RecordInPrimary(CommandBuffer &cmdBuf) const
   {
      cmdBuf.AppendCommandReference(*this);
   }

private:
   QueryPool   *m_queryPool;
   uint32_t     m_firstQuery;
   uint32_t     m_queryCount;
};

// Updates the query pool result counters from the h/w counters
class CmdUpdateQueryPoolObj : public CPUCommand
{
public:
   friend class CommandBuffer;

   CmdUpdateQueryPoolObj(const QueryDataEntryPtrList &qdePtrList) :
      m_qdePtrList(qdePtrList)
   {}

   // Will be called on a separate thread
   void Execute() const
   {
      for (uint32_t n = 0; n < m_qdePtrList.NumEntries(); n++)
      {
         QueryDataEntry *entry = m_qdePtrList.PtrAt(n);
         entry->pool->UpdateQuery(*entry);
      }
   }

   JobID Schedule(Queue &queue, const SchedDependencies &deps) const override
   {
      CPUTask *task = new CPUTask(this);
      JobID    updateJob = task->ScheduleTask(deps);

      for (uint32_t n = 0; n < m_qdePtrList.NumEntries(); n++)
      {
         QueryDataEntry *entry = m_qdePtrList.PtrAt(n);
         entry->pool->SetUpdateDependency(entry->queryID, updateJob);
      }

      return updateJob;
   }

   void RecordInPrimary(CommandBuffer &cmdBuf) const
   {
      cmdBuf.AppendCommandReference(*this);
   }

private:
   QueryDataEntryPtrList   m_qdePtrList;
};

// Copies the query pool results into a device memory buffer
class CmdCopyQueryPoolResultsObj : public CPUCommand
{
public:
   friend class CommandBuffer;

   // Will be called on a separate thread
   void Execute() const
   {
      DeviceMemory *mem = m_dstBuffer->GetBoundMemory();
      void         *dstPtr;
      size_t        bufBytes = static_cast<size_t>(m_dstBuffer->Size() -
                                                   m_dstBuffer->GetBoundMemoryOffset());

      assert(m_stride * m_queryCount <= bufBytes);

      mem->MapMemory(m_device, m_dstBuffer->GetBoundMemoryOffset(), m_stride * m_queryCount, 0, &dstPtr);

      m_queryPool->CopyQueryPoolResults(m_firstQuery, m_queryCount, bufBytes, dstPtr, m_stride, m_flags);

      // Flush the writes
      VkMappedMemoryRange  range;
      range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
      range.pNext = nullptr;
      range.memory = toHandle<VkDeviceMemory>(mem);
      range.offset = m_dstBuffer->GetBoundMemoryOffset();
      range.size   = m_stride * m_queryCount;
      mem->FlushMappedRange(range);

      mem->UnmapMemory(m_device);
   }

private:
   Device            *m_device;
   QueryPool         *m_queryPool;
   uint32_t           m_firstQuery;
   uint32_t           m_queryCount;
   Buffer            *m_dstBuffer;
   VkDeviceSize       m_dstOffset;
   VkDeviceSize       m_stride;
   VkQueryResultFlags m_flags;
};


class CmdWriteTimestampObj : public Command
{
public:
   friend class CommandBuffer;

private:
   VkPipelineStageFlagBits  m_pipelineStage;
   QueryPool               *m_queryPool;
   uint32_t                 m_query;
};

class CmdDebugMarkerBeginEXTObj : public Command
{
public:
   friend class CommandBuffer;

private:
   char  *m_markerName;
   float  m_color[4];
};

class CmdDebugMarkerEndEXTObj : public Command
{
public:
   friend class CommandBuffer;

private:
};

class CmdDebugMarkerInsertEXTObj : public Command
{
public:
   friend class CommandBuffer;

private:
   char  *m_markerName;
   float  m_color[4];
};

} //namespace bvk
