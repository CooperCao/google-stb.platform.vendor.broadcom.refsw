/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

namespace bvk {

class Event: public NonCopyable, public Allocating
{
public:
   Event(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::Device                   *pDevice,
      const VkEventCreateInfo       *pCreateInfo);

   ~Event() noexcept;

   VkResult GetEventStatus(
      bvk::Device *device) noexcept;

   VkResult SetEvent(
      bvk::Device *device) noexcept;

   VkResult ResetEvent(
      bvk::Device *device) noexcept;

   // Implementation specific from this point on
   JobID ScheduleSignal(const SchedDependencies &deps);
   JobID ScheduleReset(const SchedDependencies &deps);
   JobID ScheduleWait(const SchedDependencies &deps,
                      VkPipelineStageFlags stageFlags);

private:
   bcm_sched_event_id   m_id;
};

} // namespace bvk
