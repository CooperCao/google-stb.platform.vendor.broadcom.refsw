/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "QueryManager.h"
#include "CommandBuffer.h"
#include "CommandBufferBuilder.h"
#include "Device.h"
#include "Command.h"

#include "libs/core/v3d/v3d_align.h"
#include "libs/util/log/log.h"

namespace bvk {

LOG_DEFAULT_CAT("bvk::QueryManager");

void Query::AddHWCounter(const DevMemRange &counter)
{
   m_includedCounters.emplace_back(counter);
}

void Query::SetQueryDataEntryPtr(QueryDataEntry *qdePtr)
{
   m_queryDataEntryPtr = qdePtr;

   // Set the pointer that we made during Begin to point at the list of counters. The Begin
   // cmd then knows which h/w counters to zero.
   assert(m_resetDataEntryPtr != nullptr);
   *m_resetDataEntryPtr = qdePtr;
}

void Query::Begin(CmdZeroQueryHWCountersObj *cmd)
{
   assert(!m_active);

   m_active = true;
   m_queryDataEntryPtr = nullptr;

   // Remember the address that we need to write a real QueryDataEntry pointer into during EndQuery.
   // This will point at a data block with all the hardware counters for this query so they
   // can be zeroed.
   m_resetDataEntryPtr = cmd->GetQueryDataEntryPtrPtr();
}

void Query::End()
{
   assert(m_active);
   m_active = false;
}

void QueryDataEntry::ZeroHWCounters(uint32_t numCores) const
{
   const uint32_t coreStride = V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_SIZE;

   for (uint32_t c = 0; c < numHWCounters; c++)
   {
      for (uint32_t core = 0; core < numCores; core++)
      {
         auto &block = mappedHWCounters[c].block;

         uint32_t *p = static_cast<uint32_t*>(block.Ptr());

         // Each core's counter is a fixed stride away
         p += core * coreStride / sizeof(uint32_t);

         log_trace("Zeroing h/w counter at phys %08x, virt %p", block.Phys() + core * coreStride, p);

         // Zero and flush
         assert(p != nullptr);
         *p = 0;
         block.SyncMemory();
      }
   }
}

uint32_t QueryDataEntry::GetFinalCounterValue(uint32_t numCores) const
{
   const uint32_t coreStride = V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_SIZE;
   uint32_t       result = 0;

   for (uint32_t c = 0; c < numHWCounters; c++)
   {
      for (uint32_t core = 0; core < numCores; core++)
      {
         auto &block = mappedHWCounters[c].block;

         // Ensure the CPU can see the results
         block.InvalidateMemory();

         uint32_t *p = static_cast<uint32_t*>(block.Ptr());

         // Each core's counter is a fixed stride away
         p += core * coreStride / sizeof(uint32_t);

         result += *p;
      }
   }

   return result;
}

QueryManager::QueryManager(CommandBuffer *cmdBuf, CommandBufferBuilder *builder,
                           const VkAllocationCallbacks *pCallbacks) :
   Allocating(pCallbacks),
   m_cmdBuf(cmdBuf)
{
   uint32_t numCores = builder->m_device->GetPlatform().GetNumCores();

   m_queryBlockSize = numCores * V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_SIZE;

   m_numCountersPerBlock = V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_COUNTERS;
}

// Returns an array of QueryDataEntry pointers
QueryDataEntryPtrList QueryManager::MakeQueryDataForJob(ArenaAllocator<SysMemCmdBlock, void*> &arena)
{
   log_trace("MakeQueryDataForJob : m_queries.size() = %zu", m_queries.size());

   // Make a new block of query data in CommandBuffer system memory for any queries that have
   // now finished.
   if (m_queries.size() == 0)
   {
      QueryDataEntryPtrList qdeList(0, arena);
      return qdeList;
   }

   // Work out how much room we need to store the data
   uint32_t numFinished = 0;
   for (auto &q : m_queries)
      if (!q.second.IsActive())
         numFinished++;

   QueryDataEntryPtrList qdeList(numFinished, arena);

   // Copy the finished query data into the sysMem block
   uint32_t n = 0;
   for (auto &q : m_queries)
   {
      log_trace("  MakeQueryDataForJob : IsActive=%u", q.second.IsActive() ? 1 : 0);

      if (!q.second.IsActive())
      {
         qdeList.SetPtrAt(n, q.second.GetQueryDataEntryPtr());
         n++;
      }
   }

   log_trace("MakeQueryDataForJob, finishedQueries=%u", numFinished);

   return qdeList;
}

QueryDataEntry *QueryManager::MakeQueryData(QueryPool *queryPool, uint32_t qID,
   ArenaAllocator<SysMemCmdBlock, void*> &arena)
{
   log_trace("MakeQueryData for p:%p q:%u", queryPool, qID);

   QueryID  id(queryPool, qID);
   Query   &query = m_queries[id];

   assert(!query.IsActive());

   if (query.NumHWCounters() == 0)
      return nullptr;

   // Work out how much room we need to store the data
   size_t bytes = sizeof(QueryDataEntry) + query.NumHWCounters() * sizeof(QueryCounter);

   void *dataPtr;
   arena.Allocate(&dataPtr, bytes, /*align=*/sizeof(void*));

   // Copy the finished query data into the sysMem block
   uint8_t        *ptr  = static_cast<uint8_t*>(dataPtr);
   QueryDataEntry *data = reinterpret_cast<QueryDataEntry*>(ptr);

   data->pool          = queryPool;
   data->queryID       = qID;
   data->numHWCounters = query.NumHWCounters();

   for (uint32_t c = 0; c < data->numHWCounters; c++)
   {
      data->mappedHWCounters[c] = query.GetHWCounter(c);
      log_trace("    %u %p %08x", c, data->mappedHWCounters[c].block.Ptr(),
                                     data->mappedHWCounters[c].block.Phys());
   }

   return data;
}

void QueryManager::AcquireNewHWCounter()
{
   if (m_nextCounterInBlock % m_numCountersPerBlock == 0)
   {
      // Need a new block
      m_cmdBuf->NewDevMemQueryRange(&m_curMemBlock, m_queryBlockSize,
                                    V3D_OCCLUSION_QUERY_COUNTER_FIRST_CORE_CACHE_LINE_ALIGN);
      m_nextCounterInBlock = 0;

      log_trace("Made new devmem block of %zu bytes for %u counters",
                 m_queryBlockSize, m_numCountersPerBlock);
   }

   m_curMappedHWCounter = static_cast<uint32_t*>(m_curMemBlock.Ptr()) + m_nextCounterInBlock;
   m_curHWCounterAddr   = m_curMemBlock.Phys() + (m_nextCounterInBlock * sizeof(uint32_t));

   // Make a tighter devMemRange that just covers this single counter, not the whole block
   m_curSingleCounterRange.Set(m_curMemBlock.Handle(), m_curHWCounterAddr, m_curMappedHWCounter,
                               sizeof(uint32_t));

   m_nextCounterInBlock++;

   log_trace("New counter %08x (ptr=%p), nextIndx = %d", m_curHWCounterAddr, m_curMappedHWCounter, m_nextCounterInBlock);
}

void QueryManager::AddNewCounterToActiveQueries()
{
   // Get a counter
   if (m_activeQueries > 0)
   {
      AcquireNewHWCounter();

      // Tell every active Query to include the h/w counter
      for (auto &q : m_queries)
         if (q.second.IsActive())
            q.second.AddHWCounter(m_curSingleCounterRange);
   }
   else
      m_curHWCounterAddr = 0;
}

void QueryManager::Begin(QueryPool *queryPool, uint32_t qID, CmdZeroQueryHWCountersObj *cmd)
{
   // Begin can only happen after a reset, but reset can be in a different commandBuffer,
   // so do the actual h/w counter resets in Begin.
   // "A query must begin and end in the same command buffer" -
   //  but not necessarily be reset in the same one.

   QueryID id(queryPool, qID);
   Query &query = m_queries[id];
   assert(!query.IsActive());

   query.Begin(cmd);

   m_activeQueries++;

   log_trace("Begin[p:%p,q:%u] (%d active queries)", queryPool, qID, m_activeQueries);

   AddNewCounterToActiveQueries();
}

void QueryManager::End(QueryPool *queryPool, uint32_t qID, ArenaAllocator<SysMemCmdBlock, void*> &arena)
{
   QueryID id(queryPool, qID);
   Query &query = m_queries[id];
   assert(query.IsActive());

   query.End();

   m_activeQueries--;

   log_trace("End[%p,%u] (%d active queries)", queryPool, qID, m_activeQueries);

   // Make a sysMem block containing the list of h/w counters for this query. This is the earliest point
   // at which we know the full set of h/w counters.
   query.SetQueryDataEntryPtr(MakeQueryData(queryPool, qID, arena));

   AddNewCounterToActiveQueries();
}

void QueryManager::RemoveFinishedQueries()
{
   log_trace("RemoveFinishedQueries (m_activeQueries = %u)", m_activeQueries);

   // Remove queries that have ended
   if (m_activeQueries == 0)
      m_queries.clear();
   else
   {
      for (auto it = m_queries.begin(); it != m_queries.end(); )
      {
         if (!it->second.IsActive())
            it = m_queries.erase(it);
         else
            ++it;
      }
   }

   // Start new h/w counters for any still running.
   // This means we are fixing the values of all the counters that were flushed on the
   // last bin/render job. They can no longer change.
   AddNewCounterToActiveQueries();
}

} // namespace bvk