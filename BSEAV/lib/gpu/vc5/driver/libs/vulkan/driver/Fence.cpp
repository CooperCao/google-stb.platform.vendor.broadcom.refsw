/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"
#include "libs/platform/bcm_sched_api.h"

#include <chrono>
#include <thread>

LOG_DEFAULT_CAT("bvk::Fence");

namespace bvk {

Fence::Fence(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   const VkFenceCreateInfo       *pCreateInfo) :
      Allocating(pCallbacks)
{
   m_isSignalled = pCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT;

   log_trace("Create Fence: this = %p", this);
}

Fence::~Fence() noexcept
{
}

inline bool Fence::IsSignalled() noexcept
{
   if (m_isSignalled)
      return true;

   if (m_isQueued)
      m_isSignalled = v3d_scheduler_jobs_reached_state(&m_deps, V3D_SCHED_DEPS_FINALISED, true);

   return m_isSignalled;
}

VkResult Fence::GetFenceStatus(bvk::Device *device) noexcept
{
   return IsSignalled() ? VK_SUCCESS : VK_NOT_READY;
}

bool Fence::ResetFence(bvk::Device *device) noexcept
{
   m_isSignalled = false;
   m_isQueued    = false;
   return true;
}

void Fence::ScheduleSignal(const SchedDependencies &deps) noexcept
{
   assert(!m_isQueued);
   m_isQueued = true;
   m_deps     = deps;
   log_trace("ScheduleSignal: this = %p", this);
}

void Fence::Signal() noexcept
{
   log_trace("Signal: this = %p", this);
   m_isSignalled = true;
}

VkResult Fence::Wait(uint64_t timeoutNs) noexcept
{
   uint64_t timeoutMs = timeoutNs / 1000000;
   if (timeoutMs > INT32_MAX)
      timeoutMs = INT32_MAX;

   log_trace("Wait: this = %p timeoutMs = %#" PRIx64, this, timeoutMs);

   if (m_isSignalled)
      return VK_SUCCESS;

   assert(m_isQueued);
   m_isSignalled = v3d_scheduler_wait_jobs_timeout(&m_deps, V3D_SCHED_DEPS_FINALISED, static_cast<int>(timeoutMs));

   if (!m_isSignalled)
   {
      log_trace("Wait: this = %p timed out", this);
      return VK_TIMEOUT;
   }

   log_trace("Wait: this = %p completed", this);

   return VK_SUCCESS;
}

// Note: this is a static function called from Device.
// The code is here to keep it local to Fence and avoid polluting Device.cpp with
// Fence specific stuff.
VkResult Fence::WaitForFences(
   uint32_t       fenceCount,
   const VkFence  *pFences,
   VkBool32       waitAll,
   uint64_t       timeout) noexcept
{
   bool allFencesSignalled   = true;
   bool oneFenceNotScheduled = false;

   log_trace(
      "WaitForFences: fenceCount = %u waitAll = %s timeout = %#" PRIx64,
      fenceCount, waitAll?"true":"false", timeout);

   SchedDependencies deps;
   for (uint32_t i = 0; i != fenceCount; ++i)
   {
      Fence *fence = fromHandle<Fence>(pFences[i]);

      // Check with this function as the list of deps might be empty
      // and this call will update the fence status
      bool isSignalled = fence->IsSignalled();
      allFencesSignalled = allFencesSignalled && isSignalled;

      if (isSignalled)
      {
         if (!waitAll)
         {
            log_trace("WaitForFences: Not waiting for all and one fence is already signalled");
            return VK_SUCCESS;
         }
      }
      else
      {
         if (fence->m_isQueued)
         {
            log_trace("WaitForFences: Should wait for fence %p", fence);
            deps += fence->m_deps;
         }
         else
            oneFenceNotScheduled = true;
      }
   }

   if (allFencesSignalled)
   {
      log_trace("WaitForFences: All fences already Signalled");
      return VK_SUCCESS;
   }

   uint64_t timeoutMs = timeout / 1000000;
   if (timeoutMs > INT32_MAX)
      timeoutMs = INT32_MAX;

   log_trace("WaitForFences: timeoutMs = %#" PRIx64, timeoutMs);
   bool complete = false;

   // At least one fence is not signaled and has not been scheduled therefore
   // the fence is not going to be signaled now (can't be signaled from the host)
   // so wait to timeout if waitAll == true
   if (oneFenceNotScheduled && waitAll)
   {
      log_warn("WaitForFences: wait for the timeout as a fence has not been scheduled");
      std::this_thread::sleep_for(std::chrono::nanoseconds(timeout));
   }
   else
   {
      if (waitAll)
         complete = v3d_scheduler_wait_jobs_timeout(&deps, V3D_SCHED_DEPS_FINALISED, static_cast<int>(timeoutMs));
      else
         complete = v3d_scheduler_wait_any_job_timeout(&deps, V3D_SCHED_DEPS_FINALISED, static_cast<int>(timeoutMs));
   }

   if (!complete)
   {
      log_trace("WaitForFences: timed out");
      return VK_TIMEOUT;
   }

   log_trace("WaitForFences: completed");

   return VK_SUCCESS;
}

} // namespace bvk
