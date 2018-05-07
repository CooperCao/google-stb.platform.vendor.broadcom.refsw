/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

#include "libs/core/v3d/v3d_addr.h"
#include "libs/core/v3d/v3d_align.h"

namespace bvk {

LOG_DEFAULT_CAT("bvk::QueryPool");

/////////////////////////////////////////////////////////////////////////////////////////////////////////

QueryMemBlock::QueryMemBlock(uint32_t numCores)
{
   uint32_t queryBlockSize      = numCores * V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_SIZE;
   uint32_t numCountersPerBlock = V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_COUNTERS;

   m_devMemHandle = gmem_alloc_and_map(queryBlockSize, V3D_OCCLUSION_QUERY_COUNTER_FIRST_CORE_CACHE_LINE_ALIGN,
                                       GMEM_USAGE_V3D_WRITE, "QueryPoolBlock");
   if (m_devMemHandle == GMEM_HANDLE_INVALID)
      throw bad_device_alloc();

   m_basePhys = gmem_get_addr(m_devMemHandle);
   m_baseAddr = static_cast<uint32_t*>(gmem_get_ptr(m_devMemHandle));

   log_trace("Made new devmem block of %u bytes for %u counters", queryBlockSize, numCountersPerBlock);
}

void QueryMemBlock::MoveFrom(QueryMemBlock &rhs)
{
   gmem_free(m_devMemHandle); // This handle will usually be GMEM_HANDLE_INVALID, but just in case

   m_devMemHandle = rhs.m_devMemHandle;
   m_basePhys     = rhs.m_basePhys;
   m_baseAddr     = rhs.m_baseAddr;

   rhs.m_devMemHandle = GMEM_HANDLE_INVALID;
}

QueryMemBlock::QueryMemBlock(QueryMemBlock &&rhs)
{
   MoveFrom(rhs);
}

QueryMemBlock &QueryMemBlock::operator=(QueryMemBlock &&rhs)
{
   if (&rhs != this)
      MoveFrom(rhs);
   return *this;
}

QueryMemBlock::~QueryMemBlock()
{
   gmem_free(m_devMemHandle);
}

void QueryMemBlock::Flush() const
{
   gmem_flush_mapped_buffer(m_devMemHandle);
}

void QueryMemBlock::Invalidate() const
{
   gmem_invalidate_mapped_buffer(m_devMemHandle);
}

v3d_addr_t QueryMemBlock::PhysAddr(uint32_t offset, uint32_t core) const
{
   const uint32_t coreStride = V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_SIZE;
   return m_basePhys + (core * coreStride) + (offset * sizeof(uint32_t));
}

uint32_t *QueryMemBlock::VirtAddr(uint32_t offset, uint32_t core) const
{
   const uint32_t coreStride = V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_SIZE / sizeof(uint32_t);
   return m_baseAddr + (core * coreStride) + offset;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

QueryPool::QueryPool(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   const VkQueryPoolCreateInfo   *pCreateInfo) :
      Allocating(pCallbacks),
      m_memoryBlocks(GetObjScopeAllocator<QueryResult>()),
      m_queryResults(GetObjScopeAllocator<QueryResult>())
{
   log_trace("Create[%p], %u queries", this, pCreateInfo->queryCount);

   m_numCores            = pDevice->GetPlatform().GetNumCores();
   m_numCountersPerBlock = V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_COUNTERS;

   if (pCreateInfo->queryType == VK_QUERY_TYPE_OCCLUSION)
   {
      // We cannot use a single large block of memory for all the counters, each block has to
      // be correctly aligned. See v3d_align.h.
      uint32_t blocksNeeded = gfx_udiv_round_up(pCreateInfo->queryCount, m_numCountersPerBlock);

      m_memoryBlocks.reserve(blocksNeeded);

      for (uint32_t b = 0; b < blocksNeeded; b++)
      {
         // Allocate a new block into the vector. This will do the gmem_alloc
         m_memoryBlocks.emplace_back(m_numCores);
      }

      m_queryResults.resize(pCreateInfo->queryCount);
   }
   else
      NOT_IMPLEMENTED_YET;
}

QueryPool::~QueryPool() noexcept
{
}

v3d_addr_t QueryPool::QueryPhysAddr(uint32_t query, uint32_t core) const
{
   uint32_t blockIndx = query / m_numCountersPerBlock;
   uint32_t offset    = query % m_numCountersPerBlock;
   assert(blockIndx < m_memoryBlocks.size());

   return m_memoryBlocks[blockIndx].PhysAddr(offset, core);
}

uint32_t *QueryPool::QueryVirtAddr(uint32_t query, uint32_t core) const
{
   uint32_t blockIndx = query / m_numCountersPerBlock;
   uint32_t offset    = query % m_numCountersPerBlock;
   assert(blockIndx < m_memoryBlocks.size());

   return m_memoryBlocks[blockIndx].VirtAddr(offset, core);
}

v3d_addr_t QueryPool::Begin(uint32_t query)
{
   std::lock_guard<std::mutex> lock{ m_lock };

   assert(!m_queryResults[query].active);
   assert(m_queryResults[query].count == 0);

   m_queryResults[query].active = true;

   return QueryPhysAddr(query);
}

void QueryPool::End(uint32_t query)
{
   std::lock_guard<std::mutex> lock{ m_lock };

   assert(m_queryResults[query].active);

   m_queryResults[query].active = false;
}

// Will be called asynchronously on a separate thread (during Command buffer execution)
void QueryPool::ResetNow(uint32_t firstQuery, uint32_t queryCount)
{
   std::lock_guard<std::mutex> lock { m_lock };
   log_trace("Resetting[p:%p q:%u->%u]", this, firstQuery, firstQuery + queryCount - 1);

   assert(firstQuery + queryCount <= m_queryResults.size());

   for (uint32_t i = firstQuery; i < firstQuery + queryCount; i++)
   {
      m_queryResults[i].count     = 0;
      m_queryResults[i].available = false;

      // Zero the hardware counters
      for (uint32_t core = 0; core < m_numCores; core++)
      {
         uint32_t *p = QueryVirtAddr(i, core);
         log_trace("Zeroing h/w counter %u at virt %p", i, p);
         *p = 0;
      }

      // NOTE: the obvious thing to do here would be to clear waitDeps.
      // That won't always work as the reset can happen after SetUpdateDependency() is called
      // and that would wipe them out again. Since all query commands are specified to execute in
      // submission order, we can just keep amalgamating the deps here and get the correct
      // behaviour.
      m_queryResults[i].waitDeps.Amalgamate();
   }

   // Flush any blocks we touched
   uint32_t startBlock = firstQuery / m_numCountersPerBlock;
   uint32_t endBlock   = (firstQuery + queryCount) / m_numCountersPerBlock;

   for (uint32_t b =-startBlock; b <= endBlock; b++)
      m_memoryBlocks[b].Flush();
}

// Will be called when the job that updates the pool counters is scheduled.
// The jobID of the update job is passed so we can later wait on it to determine
// when the result is available.
void QueryPool::SetUpdateDependency(uint32_t queryID, JobID updateJob)
{
   std::lock_guard<std::mutex> lock { m_lock };
   assert(queryID < m_queryResults.size());
   m_queryResults[queryID].waitDeps += updateJob;
}

// Will be called asynchronously on a separate thread (during Command buffer execution)
void QueryPool::UpdateQuery(uint32_t query)
{
   std::lock_guard<std::mutex> lock { m_lock };
   assert(query < m_queryResults.size());

   QueryResult &qr = m_queryResults[query];

   // Ensure the CPU can see the results
   uint32_t block = query / m_numCountersPerBlock;
   m_memoryBlocks[block].Invalidate();

   uint32_t result = 0;

   for (uint32_t core = 0; core < m_numCores; core++)
   {
      uint32_t *p = QueryVirtAddr(query, core);
      log_trace("Reading h/w counter at virt %p = %#x", p, *p);
      result += *p;
   }

   qr.count = result;

   log_trace("UpdateQuery[p:%p q:%u] = %u", this, query, qr.count);

   // Only finished queries end up in the data block, so at this point the
   // result is available.
   qr.available = true;
}

// Returns the number of results that were unavailable
uint32_t QueryPool::CopyQueryPoolResults(
   uint32_t           firstQuery,
   uint32_t           queryCount,
   size_t             dataSize,
   void              *pDest,
   VkDeviceSize       stride,
   VkQueryResultFlags flags
   )
{
   uint8_t *dst = static_cast<uint8_t*>(pDest);
   uint32_t unavailableCount = 0;
   bool     allowPartial = (flags & VK_QUERY_RESULT_PARTIAL_BIT) != 0;

   assert(dataSize >= queryCount * stride);
   assert(firstQuery + queryCount <= m_queryResults.size());

   std::lock_guard<std::mutex> lock { m_lock };

   for (uint32_t i = firstQuery; i < firstQuery + queryCount; i++)
   {
      uint32_t *d32 = reinterpret_cast<uint32_t*>(dst);
      bool      available = m_queryResults[i].available;

      if (!available)
         unavailableCount++;

      if ((flags & VK_QUERY_RESULT_64_BIT) != 0)
      {
         uint64_t *d64 = reinterpret_cast<uint64_t*>(dst);
         if (available || allowPartial)
            *d64 = m_queryResults[i].count;
         d32 += 2;
      }
      else
      {
         if (available || allowPartial)
            *d32 = m_queryResults[i].count;
         d32++;
      }

      if ((flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) != 0)
         *d32++ = available ? 1 : 0;

      dst += stride;
   }

   return unavailableCount;
}

VkResult QueryPool::GetQueryPoolResults(
   bvk::Device       *device,
   uint32_t           firstQuery,
   uint32_t           queryCount,
   size_t             dataSize,
   void              *pData,
   VkDeviceSize       stride,
   VkQueryResultFlags flags) noexcept
{
   log_trace("GetQueryPoolResults[%p], %u counts starting at %u", this, queryCount, firstQuery);

   assert(firstQuery + queryCount <= m_queryResults.size());

   if ((flags & VK_QUERY_RESULT_WAIT_BIT) != 0)
   {
      // Wait for the results
      v3d_scheduler_flush();

      for (uint32_t q = firstQuery; q < firstQuery + queryCount; q++)
      {
         std::unique_lock<std::mutex> lock { m_lock };
         SchedDependencies deps = m_queryResults[q].waitDeps;
         lock.unlock();
         // Wait for the job that will update this counter to finish
         v3d_scheduler_wait_jobs(&deps, V3D_SCHED_DEPS_COMPLETED);
      }
   }

   uint32_t unavailable = CopyQueryPoolResults(firstQuery, queryCount, dataSize, pData, stride, flags);

   if (unavailable > 0)
   {
      log_trace("%u results unavailable", unavailable);
      return VK_NOT_READY;
   }

   return VK_SUCCESS;
}

} // namespace bvk
