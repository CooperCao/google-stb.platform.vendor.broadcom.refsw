/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"
#include "SchedDependencies.h"
#include "DevMemRange.h"

#include <future>
#include <atomic>
#include <mutex>

namespace bvk {

struct QueryDataEntry;
class QueryDataEntryPtrList;

// Pool/QueryID pair
class QueryID
{
public:
   QueryID(QueryPool *p, uint32_t q) : m_pool(p), m_query(q) {}

   bool operator<(const QueryID &rhs) const
   {
      return m_pool == rhs.m_pool ? m_query < rhs.m_query : m_pool < rhs.m_pool;
   }

   QueryPool *GetPool()  const { return m_pool; }
   uint32_t   GetQuery() const { return m_query; }

private:
   QueryPool *m_pool;
   uint32_t   m_query;
};

// A pool result - the aggregated counter and available state
struct QueryResult
{
   QueryResult() = default;

   QueryResult(const QueryResult &rhs)
   {
      available = rhs.available;
   }

   uint32_t                count = 0;
   bool                    available = false;
   bool                    active = false;
   SchedDependencies       waitDeps;
};

// Counters can only be allocated in certain sized blocks, this represents one.
class QueryMemBlock
{
public:
   QueryMemBlock() {}
   QueryMemBlock(uint32_t numCores);
   QueryMemBlock(const QueryMemBlock &rhs) = delete;
   QueryMemBlock(QueryMemBlock &&rhs);
   ~QueryMemBlock();

   QueryMemBlock &operator=(QueryMemBlock &&rhs);
   QueryMemBlock &operator=(const QueryMemBlock &rhs) = delete;

   void Flush() const;
   void Invalidate() const;
   v3d_addr_t PhysAddr(uint32_t offset, uint32_t core) const;
   uint32_t  *VirtAddr(uint32_t offset, uint32_t core) const;

private:
   void MoveFrom(QueryMemBlock &rhs);

private:
   gmem_handle_t  m_devMemHandle = GMEM_HANDLE_INVALID;
   v3d_addr_t     m_basePhys;
   uint32_t      *m_baseAddr;
};

class QueryPool: public NonCopyable, public Allocating
{
public:
   QueryPool(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::Device                   *pDevice,
      const VkQueryPoolCreateInfo   *pCreateInfo);

   ~QueryPool() noexcept;

   VkResult GetQueryPoolResults(
      bvk::Device       *device,
      uint32_t           firstQuery,
      uint32_t           queryCount,
      size_t             dataSize,
      void              *pData,
      VkDeviceSize       stride,
      VkQueryResultFlags flags) noexcept;

   // Implementation specific from this point on
public:
   v3d_addr_t Begin(uint32_t query); // Returns the h/w counter address
   void       End(uint32_t query);
   void       ResetNow(uint32_t firstQuery, uint32_t queryCount);
   void       UpdateQuery(uint32_t query);
   uint32_t   CopyQueryPoolResults(uint32_t firstQuery, uint32_t queryCount, size_t dataSize,
                                   void *pDest, VkDeviceSize stride, VkQueryResultFlags flags);
   void       SetUpdateDependency(uint32_t queryID, JobID updateJob);

private:
   v3d_addr_t QueryPhysAddr(uint32_t query, uint32_t core = 0) const;
   uint32_t  *QueryVirtAddr(uint32_t query, uint32_t core = 0) const;

private:
   std::mutex                 m_lock;
   bvk::vector<QueryMemBlock> m_memoryBlocks;
   bvk::vector<QueryResult>   m_queryResults;
   uint32_t                   m_numCores;
   uint32_t                   m_numCountersPerBlock;
};

} // namespace bvk
