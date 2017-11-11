/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"
#include "SchedDependencies.h"

#include "libs/platform/v3d_scheduler.h"
#include "libs/util/log/log.h"

LOG_DEFAULT_CAT("bvk::Semaphore");

namespace bvk {

Semaphore::Semaphore(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   const VkSemaphoreCreateInfo   *pCreateInfo) :
      Allocating(pCallbacks)
{
   log_trace("%p Created", this);
}

Semaphore::~Semaphore() noexcept
{
   log_trace("%p Destroyed", this);
}

SchedDependencies Semaphore::ScheduleWait(
   VkPipelineStageFlags stageFlags)
{
   log_trace("%p ScheduleWait", this);

   SchedDependencies deps = m_deps;
   m_deps.n = 0;

   return deps;
}

void Semaphore::ScheduleSignal(const SchedDependencies &deps)
{
   log_trace("%p ScheduleSignal with %d deps", this, deps.n);

   assert(m_deps.n == 0);

   m_deps = deps;
}

// SignalNow and WaitNow are only used by WSI platform code
// when a surface is acquired or presented

void Semaphore::SignalNow()
{
   // On all our Vulkan V3DPlatforms a surface is returned only
   // when it is off the display and available for rendering.
   // All the platform signal the semaphore straight away.
   // We keep this function to conserve a balanced API and
   // provide debugging information
   log_trace("%p SignalNow", this);

   assert(m_deps.n == 0);
}

void Semaphore::WaitNow()
{
   log_trace("%p WaitNow", this);

   v3d_scheduler_wait_jobs(&m_deps, V3D_SCHED_DEPS_COMPLETED);

   m_deps.n = 0;
}

} // namespace bvk
