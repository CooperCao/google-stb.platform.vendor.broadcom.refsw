/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"
#include "SchedDependencies.h"

#include <future>
#include <atomic>

namespace bvk {

struct QueryDataEntry;
class QueryDataEntryPtrList;

struct QueryResult
{
   QueryResult() = default;

   QueryResult(const QueryResult &rhs)
   {
      available = rhs.available;
   }

   uint32_t                count = 0;
   bool                    available = false;
   SchedDependencies       waitDeps;
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
   void ZeroHWCounters(QueryDataEntry *qde);
   void ResetNow(uint32_t firstQuery, uint32_t queryCount);
   void UpdateQuery(const QueryDataEntry &data);
   uint32_t CopyQueryPoolResults(uint32_t firstQuery, uint32_t queryCount, size_t dataSize,
                                 void *pDest, VkDeviceSize stride, VkQueryResultFlags flags);

   void SetUpdateDependency(uint32_t queryID, JobID updateJob);

private:
   bvk::vector<QueryResult>   m_queryResults;
   uint32_t                   m_numCores;
};

} // namespace bvk
