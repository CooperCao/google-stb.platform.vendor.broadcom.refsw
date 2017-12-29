/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

namespace bvk {

class SchedDependencies;

class Semaphore: public NonCopyable, public Allocating
{
public:
   Semaphore(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::Device                   *pDevice,
      const VkSemaphoreCreateInfo   *pCreateInfo);

   ~Semaphore() noexcept;

   // Implementation specific from this point on
public:
   SchedDependencies ScheduleWait(VkPipelineStageFlags stageFlags);
   void ScheduleSignal(const SchedDependencies &deps);

   // For WSI platforms
   void SignalNow();
   void WaitNow();

private:
   SchedDependencies m_deps;
};

} // namespace bvk
