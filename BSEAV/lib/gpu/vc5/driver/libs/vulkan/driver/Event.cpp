/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

LOG_DEFAULT_CAT("bvk::Event");

namespace bvk {

Event::Event(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   const VkEventCreateInfo       *pCreateInfo) :
      Allocating(pCallbacks)
{
   m_id = v3d_scheduler_new_event();
   log_trace("%p Created, event id: %#" PRIx64, this, m_id);

   if (m_id == V3D_INVALID_SCHED_EVENT_ID)
      throw VK_ERROR_OUT_OF_HOST_MEMORY;
}

Event::~Event() noexcept
{
   log_trace("%p Destroyed", this);
   v3d_scheduler_delete_event(m_id);
}

VkResult Event::GetEventStatus(
   bvk::Device *device) noexcept
{
   bool sig = v3d_scheduler_query_event(m_id);
   log_trace("%p GetEventStatus = %s: event id: %#" PRIx64, this, sig ? "SET" : "UNSET", m_id);
   return sig ? VK_EVENT_SET : VK_EVENT_RESET;
}

VkResult Event::SetEvent(
   bvk::Device *device) noexcept
{
   log_trace("%p SetEvent: event id: %#" PRIx64, this, m_id);

   v3d_scheduler_set_event(m_id);

   return VK_SUCCESS;
}

VkResult Event::ResetEvent(
   bvk::Device *device) noexcept
{
   // There cannot be any waiters on the event when this is called
   log_trace("%p ResetEvent: event id: %#" PRIx64, this, m_id);

   v3d_scheduler_reset_event(m_id);

   return VK_SUCCESS;
}

JobID Event::ScheduleSignal(const SchedDependencies &deps)
{
   log_trace("%p ScheduleSignal: event id: %#" PRIx64, this, m_id);

   return v3d_scheduler_submit_set_event_job(&deps, m_id);
}

// Called on a separate thread
JobID Event::ScheduleReset(const SchedDependencies &deps)
{
   log_trace("%p ScheduleReset: event id: %#" PRIx64, this, m_id);

   return v3d_scheduler_submit_reset_event_job(&deps, m_id);
}

JobID Event::ScheduleWait(
   const SchedDependencies &deps,
   VkPipelineStageFlags stageFlags)
{
   log_trace("%p ScheduleWait: event id: %#" PRIx64, this, m_id);

   return v3d_scheduler_submit_wait_on_event_job(&deps, m_id);
}

} // namespace bvk
