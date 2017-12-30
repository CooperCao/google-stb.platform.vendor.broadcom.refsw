/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Common.h"
#include "QueryPool.h"
#include "DevMemDataBlock.h"
#include "ArenaAllocator.h"
#include "SysMemCmdBlock.h"

#include "libs/core/v3d/v3d_addr.h"
#include "libs/core/v3d/v3d_align.h"

#include <vector>

#ifdef WIN32
#pragma warning(disable : 4200) // Zero sized array in structure
#endif

// The QueryManager manages the state associated with queries within a single CommandBuffer.
// It is only alive during CommandBuffer construction, not during execution.
// It determines when new h/w counters are needed and manages overlapping queries.
//
// Occlusion queries are handled in the following way:
//
// CmdResetQueryPool adds a command to the buffer which will just zero the s/w counts in the
//                   pool itself. It will not change h/w counters since it may be called from
//                   a different CommandBuffer to Begin & End.
//
// CmdBeginQuery     adds a command to the buffer which will zero all the hardware counters
//                   associated with the query being started. The set of h/w counters isn't
//                   known when Begin is called, only during End. When processing the End, a
//                   pointer in the CmdBeginQueryObj command is lazily set to point at the
//                   actual list of h/w counters.
//
// CmdEndQuery       builds the list of h/w counters (part of QueryDataEntry) now that they
//                   are all known. Sets the pointer in the BeginQuery cmd to point at that
//                   list. Also handles the special case where Begin/End is outside a render
//                   pass. In this case, the EndQuery must force a counter update as the
//                   normal mechanism (at the end of bin/render job) won't trigger.
//
// BuildHardwareJob  at the end of adding each bin/render job, an update counter cmd is added
//                   to the commandList. We do this here, since we know the VC5 counter cache
//                   will have been flushed. We don't have to worry about the counters changing
//                   after this point as we start new ones. This is the safest way to manage
//                   h/w counter lifetimes and overlap. Typically BuildHardwareJob will be
//                   called at the end of each render pass.
//
// SecondaryCommandBuffers
//                   we do not support inherited queries (i.e Begin & End in the primary counting
//                   draws in the secondary. This is an optional feature and doesn't really work
//                   with the way we implement secondaries. Making it work would involve unrolling
//                   a secondary in a primary, which isn't as efficient. We build CL fragments in
//                   our secondaries, and branch to them from the primary.
//                   We do still have to support Begin & End within secondaries though.
//                   This involves storing some pre & post commands in the secondary and
//                   scheduling them before and after the binRender job in the primary where the
//                   branches happen. (Note: begin & end must be in the same command buffer, and
//                   secondary buffers cannot do a reset, so cannot re-use queries).

struct VkAllocationCallbacks;

namespace bvk {

class CommandBufferBuilder;
class CmdZeroQueryHWCountersObj;

// Data for a single hardware counter location.
// We need the mapped and physical address, so we use a DevMemRange
struct QueryCounter
{
   QueryCounter(const DevMemRange &range) : block(range) {}

   DevMemRange  block;
};

// Class representing the data for each query held by the QueryManager
class Query
{
public:
   void     AddHWCounter(const DevMemRange &counter);
   uint32_t NumHWCounters() const                  { return m_includedCounters.size(); }
   const QueryCounter &GetHWCounter(uint32_t indx) { return m_includedCounters[indx];  }

   bool IsActive() const { return m_active; }

   QueryDataEntry *GetQueryDataEntryPtr() const { return m_queryDataEntryPtr; }
   void SetQueryDataEntryPtr(QueryDataEntry *qdePtr);

   void Begin(CmdZeroQueryHWCountersObj *cmd);
   void End();

private:
   bool m_active = false;

   // List of mapped h/w counter addresses that contribute to this query
   // Can't be a bvk vector as nested.
   std::vector<QueryCounter>  m_includedCounters;

   // The h/w counter list - filled in during EndQuery (when all h/w counters are known)
   QueryDataEntry            *m_queryDataEntryPtr = nullptr;

   // Pointer to a pointer to fill in when the queryDataEntry is fully known
   QueryDataEntry            **m_resetDataEntryPtr = nullptr;
};

// Pool/QueryID pair used as the key in manager's map
class QueryID
{
public:
   QueryID(QueryPool *p, uint32_t q) : m_pool(p), m_query(q) {}

   bool operator<(const QueryID &rhs) const
   {
      return m_pool == rhs.m_pool ? m_query < rhs.m_query : m_pool < rhs.m_pool;
   }

private:
   QueryPool *m_pool;
   uint32_t   m_query;
};

// Data representing a query. These are held in CommandBuffer sysMem.
// They contain the query ID and a list of all h/w counters that contribute
// to that query.
struct QueryDataEntry
{
   QueryPool     *pool;
   uint32_t       queryID;
   uint32_t       numHWCounters;
   QueryCounter   mappedHWCounters[0];

   // Zero all hardware counters associated with this query
   void ZeroHWCounters(uint32_t numCores) const;

   // Add together all the hardware counters associated with this query
   uint32_t GetFinalCounterValue(uint32_t numCores) const;
};

// Helper class to make handling arrays of arena allocated lists of pointers easier
class QueryDataEntryPtrList
{
public:
   QueryDataEntryPtrList() = delete;

   QueryDataEntryPtrList(uint32_t numEntries, ArenaAllocator<SysMemCmdBlock, void*> &arena) :
      m_numEntries(numEntries)
   {
      if (numEntries > 0)
      {
         void *p;
         arena.Allocate(&p, numEntries * sizeof(QueryDataEntry*), /*align=*/sizeof(QueryDataEntry*));
         m_ptrList = static_cast<QueryDataEntry **>(p);
      }
   }

   uint32_t         NumEntries() const           { return m_numEntries;  }
   QueryDataEntry **PtrAddress(uint32_t n) const { return &m_ptrList[n]; }
   QueryDataEntry  *PtrAt(uint32_t n) const      { return m_ptrList[n];  }
   void             SetPtrAt(uint32_t n, QueryDataEntry *ptr) { m_ptrList[n] = ptr; }

private:
   QueryDataEntry **m_ptrList = nullptr;      // Memory from the arena
   uint32_t         m_numEntries = 0;
};

// Class to manage overlapping query counter blocks.
// Tracks the status of each query as the command buffer is being built.
class QueryManager : public Allocating
{
public:
   QueryManager(CommandBuffer *cmdBuf, CommandBufferBuilder *builder, const VkAllocationCallbacks *pCallbacks);

   void Begin(QueryPool *queryPool, uint32_t qID, CmdZeroQueryHWCountersObj *cmd);
   void End(QueryPool *queryPool, uint32_t qID, ArenaAllocator<SysMemCmdBlock, void*> &arena);

   void RemoveFinishedQueries();

   bool HasQueries() const                { return m_queries.size() > 0; }
   bool HasActiveQueries() const          { return m_activeQueries > 0;  }
   v3d_addr_t GetCurHWCounterAddr() const { return m_curHWCounterAddr;   }

   QueryDataEntryPtrList MakeQueryDataForJob(ArenaAllocator<SysMemCmdBlock, void*> &arena);

private:
   void AcquireNewHWCounter();
   void AddNewCounterToActiveQueries();

   QueryDataEntry *MakeQueryData(QueryPool *queryPool, uint32_t qID,
                                 ArenaAllocator<SysMemCmdBlock, void*> &arena);

private:
   bvk::map<QueryID, Query>      m_queries;
   CommandBuffer                *m_cmdBuf = nullptr;
   v3d_addr_t                    m_curHWCounterAddr = 0;
   uint32_t                     *m_curMappedHWCounter = nullptr;
   DevMemRange                   m_curSingleCounterRange;
   uint32_t                      m_nextCounterInBlock = 0;
   DevMemRange                   m_curMemBlock;
   size_t                        m_queryBlockSize = 0;
   uint32_t                      m_numCountersPerBlock = 0;
   uint32_t                      m_activeQueries = 0;
};

} // namespace bvk
