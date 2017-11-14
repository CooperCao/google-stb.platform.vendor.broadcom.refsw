/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"
#include "Dispatchable.h"
#include "SchedDependencies.h"

namespace bvk {

class Queue: public NonCopyable, public Allocating, public Dispatchable
{
public:
#if !V3D_USE_CSD
   static constexpr unsigned MaxInFlightComputeJobs = 2;
#endif

   Queue(
      const VkAllocationCallbacks *pCallbacks,
      Device                      *pDevice,
      float                        priority);

   // Queues are emplaced in a vector in the device.
   // Since we hid the copy constructor, we must have a move constructor
   Queue(Queue &&rhs) :
      Allocating(GetCallbacks()),
      m_device(std::move(rhs.m_device)),
      m_priority(std::move(rhs.m_priority)),
      m_allPreviousJobs(std::move(rhs.m_allPreviousJobs)),
      m_waitBarriers(std::move(rhs.m_waitBarriers))
   {
#if !V3D_USE_CSD
      for (unsigned i = 0; i != MaxInFlightComputeJobs; ++i)
         m_prevComputeJobs[i] = rhs.m_prevComputeJobs[i];
#endif
   }

   ~Queue() noexcept;

   VkResult QueueSubmit(
      uint32_t              submitCount,
      const VkSubmitInfo   *pSubmits,
      bvk::Fence           *fence) noexcept;

   VkResult QueueWaitIdle() noexcept;

   VkResult QueueBindSparse(
      uint32_t                 bindInfoCount,
      const VkBindSparseInfo  *pBindInfo,
      bvk::Fence              *fence) noexcept;

   VkResult QueuePresentKHR(
      const VkPresentInfoKHR  *pPresentInfo) noexcept;

   // Implementation specific from this point on

   const Device &GetDevice() const { return *m_device; }
   const SchedDependencies &AllPreviousJobs() const { return m_allPreviousJobs; }
   const SchedDependencies &WaitBarriers() const { return m_waitBarriers; }

   void RecordPrevJob(JobID jobID)
   {
      m_allPreviousJobs += jobID;
   }

   void RecordBarrierJob(JobID jobID)
   {
      m_waitBarriers += jobID;
   }

#if !V3D_USE_CSD
   void RecordComputeJob(JobID jobID)
   {
      for (unsigned i = 1; i != MaxInFlightComputeJobs; ++i)
         m_prevComputeJobs[i-1] = m_prevComputeJobs[i];
      m_prevComputeJobs[MaxInFlightComputeJobs-1] = jobID;
   }

   JobID PenultimateComputeJob() const { return m_prevComputeJobs[0]; }
#endif

private:
   Device            *m_device;
   float              m_priority;
   SchedDependencies  m_allPreviousJobs;  // Effectively every job we've ever scheduled
   SchedDependencies  m_waitBarriers;     // All previous execution barrier dependencies;

#if !V3D_USE_CSD
   JobID m_prevComputeJobs[MaxInFlightComputeJobs]{};
#endif
};

} // namespace bvk
