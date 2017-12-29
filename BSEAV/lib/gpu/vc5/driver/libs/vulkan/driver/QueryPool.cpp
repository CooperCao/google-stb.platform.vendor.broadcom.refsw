/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"
#include "QueryManager.h"

namespace bvk {

LOG_DEFAULT_CAT("bvk::QueryPool");

QueryPool::QueryPool(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   const VkQueryPoolCreateInfo   *pCreateInfo) :
      Allocating(pCallbacks),
      m_queryResults(GetObjScopeAllocator<QueryResult>())
{
   log_trace("Create[%p], %u queries", this, pCreateInfo->queryCount);

   m_numCores = pDevice->GetPlatform().GetNumCores();

   if (pCreateInfo->queryType == VK_QUERY_TYPE_OCCLUSION)
   {
      m_queryResults.resize(pCreateInfo->queryCount);
   }
   else
      NOT_IMPLEMENTED_YET;
}

QueryPool::~QueryPool() noexcept
{
}

// Will be called asynchronously on a separate thread (during Command buffer execution)
void QueryPool::ZeroHWCounters(QueryDataEntry *qde)
{
   log_trace("ZeroHWCounters[p:%p q:%u]", qde->pool, qde->queryID);

   qde->ZeroHWCounters(m_numCores);
}

// Will be called asynchronously on a separate thread (during Command buffer execution)
void QueryPool::ResetNow(uint32_t firstQuery, uint32_t queryCount)
{
   log_trace("Resetting[p:%p q:%u->%u]", this, firstQuery, firstQuery + queryCount - 1);

   assert(firstQuery + queryCount <= m_queryResults.size());

   for (uint32_t i = firstQuery; i < firstQuery + queryCount; i++)
   {
      m_queryResults[i].count     = 0;
      m_queryResults[i].available = false;
      m_queryResults[i].waitDeps  = SchedDependencies();
   }
}

// Will be called when the job that updates the pool counters is scheduled.
// The jobID of the update job is passed so we can later wait on it to determine
// when the result is available.
void QueryPool::SetUpdateDependency(uint32_t queryID, JobID updateJob)
{
   assert(queryID < m_queryResults.size());
   m_queryResults[queryID].waitDeps += updateJob;
}

// Will be called asynchronously on a separate thread (during Command buffer execution)
void QueryPool::UpdateQuery(const QueryDataEntry &data)
{
   assert(data.queryID < m_queryResults.size());

   QueryResult &qr = m_queryResults[data.queryID];

   qr.count = data.GetFinalCounterValue(m_numCores);

   log_trace("UpdateQuery[p:%p q:%u] = %u", this, data.queryID, qr.count);

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
         QueryResult &qr = m_queryResults[q];

         // Wait for the job that will update this counter to finish
         v3d_scheduler_wait_jobs(&qr.waitDeps, V3D_SCHED_DEPS_COMPLETED);
      }
   }

   uint32_t unavailable = CopyQueryPoolResults(firstQuery, queryCount, dataSize, pData, stride, flags);

   if (unavailable > 0)
      return VK_NOT_READY;

   return VK_SUCCESS;
}

} // namespace bvk
