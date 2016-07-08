/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  v3d_platform
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "../bcm_sched_api.h"
#include "../bcm_perf_api.h"
#include "sched_abstract.h"
#include "platform_common.h"
#include "../v3d_platform.h"
#include "../v3d_scheduler_priv.h"
#include "gmem_abstract.h"

#include "libs/util/demand.h"

typedef struct sched_context
{
   BEGL_SchedInterface  sched_iface;
   void* session_id;
} sched_context;

static sched_context s_context;

__attribute__((visibility("default")))
void BEGL_RegisterSchedInterface(BEGL_SchedInterface *iface)
{
   if (iface != NULL)
      s_context.sched_iface = *iface;                                    /* Register   */
   else
      memset(&s_context.sched_iface, 0, sizeof(BEGL_SchedInterface));    /* Unregister */
}

// ===================================================================

void bcm_sched_queue_jobs(
   const struct bcm_sched_job *jobs,
   unsigned num_jobs)
{
   demand(s_context.sched_iface.QueueJobs != NULL);

   s_context.sched_iface.QueueJobs(s_context.sched_iface.context, s_context.session_id, jobs, num_jobs);
}

void bcm_sched_queue_bin_render(
   const struct bcm_sched_job *bin,
   const struct bcm_sched_job *render)
{
   demand(s_context.sched_iface.QueueBinRender != NULL);

   s_context.sched_iface.QueueBinRender(s_context.sched_iface.context, s_context.session_id, bin, render);
}

void bcm_sched_query(
   struct bcm_sched_dependencies *completed_deps,
   struct bcm_sched_dependencies *finalised_deps,
   struct bcm_sched_query_response *response)
{
   demand(s_context.sched_iface.Query != NULL);

   s_context.sched_iface.Query(s_context.sched_iface.context, s_context.session_id, completed_deps, finalised_deps, response);
}

uint32_t bcm_sched_get_num_counter_groups(void)
{
   uint32_t numGroups = 0;

   if (s_context.sched_iface.GetPerfNumCounterGroups)
      s_context.sched_iface.GetPerfNumCounterGroups(s_context.sched_iface.context, s_context.session_id, &numGroups);

   return numGroups;
}

bool bcm_sched_enumerate_group_counters(
   uint32_t                              group,
   struct bcm_sched_counter_group_desc  *group_desc)
{
   BEGL_SchedStatus status = BEGL_SchedFail;

   if (s_context.sched_iface.GetPerfCounterGroupInfo)
      status = s_context.sched_iface.GetPerfCounterGroupInfo(s_context.sched_iface.context, s_context.session_id, group, group_desc);

   return status == BEGL_SchedSuccess;
}

bool bcm_sched_select_group_counters(
   const struct bcm_sched_group_counter_selector *selector)
{
   BEGL_SchedStatus status = BEGL_SchedFail;

   if (s_context.sched_iface.ChoosePerfCounters)
      status = s_context.sched_iface.ChoosePerfCounters(s_context.sched_iface.context, s_context.session_id, selector);

   return status == BEGL_SchedSuccess;
}

bool bcm_sched_set_counter_collection(
   enum bcm_sched_counter_state  state)
{
   BEGL_SchedStatus status = BEGL_SchedFail;

   if (s_context.sched_iface.SetPerfCounting)
      status = s_context.sched_iface.SetPerfCounting(s_context.sched_iface.context, s_context.session_id, state);

   return status == BEGL_SchedSuccess;
}

uint32_t bcm_sched_get_counters(
   struct bcm_sched_counter  *counters,
   uint32_t                   max_counters,
   bool                       reset_counts)
{
   uint32_t ctrs = 0;
   if (s_context.sched_iface.GetPerfCounterData)
      ctrs = s_context.sched_iface.GetPerfCounterData(s_context.sched_iface.context, s_context.session_id, counters, max_counters, reset_counts);

   return ctrs;
}

uint32_t bcm_sched_get_num_event_tracks(void)
{
   uint32_t numTracks = 0;
   uint32_t numEvents = 0;

   if (s_context.sched_iface.GetEventCounts)
      s_context.sched_iface.GetEventCounts(s_context.sched_iface.context, s_context.session_id, &numTracks, &numEvents);

   return numTracks;
}

bool bcm_sched_describe_event_track(
   uint32_t                            track_index,
   struct bcm_sched_event_track_desc   *track_desc)
{
   BEGL_SchedStatus status = BEGL_SchedFail;

   if (s_context.sched_iface.GetEventTrackInfo)
      status = s_context.sched_iface.GetEventTrackInfo(s_context.sched_iface.context, s_context.session_id, track_index, track_desc);

   return status == BEGL_SchedSuccess;
}

uint32_t bcm_sched_get_num_events(void)
{
   uint32_t numTracks = 0;
   uint32_t numEvents = 0;

   if (s_context.sched_iface.GetEventCounts)
      s_context.sched_iface.GetEventCounts(s_context.sched_iface.context, s_context.session_id, &numTracks, &numEvents);

   return numEvents;
}

bool bcm_sched_describe_event(
   uint32_t                      event_index,
   struct bcm_sched_event_desc   *event_desc)
{
   BEGL_SchedStatus status = BEGL_SchedFail;

   if (s_context.sched_iface.GetEventInfo)
      status = s_context.sched_iface.GetEventInfo(s_context.sched_iface.context, s_context.session_id, event_index, event_desc);

   return status == BEGL_SchedSuccess;
}

bool bcm_sched_describe_event_data(
   uint32_t                            event_index,
   uint32_t                            field_index,
   struct bcm_sched_event_field_desc   *field_desc)
{
   BEGL_SchedStatus status = BEGL_SchedFail;

   if (s_context.sched_iface.GetEventDataFieldInfo)
      status = s_context.sched_iface.GetEventDataFieldInfo(s_context.sched_iface.context, s_context.session_id,
                                                           event_index, field_index, field_desc);

   return status == BEGL_SchedSuccess;
}

bool bcm_sched_set_event_collection(
   enum bcm_sched_event_state    state)
{
   BEGL_SchedStatus status = BEGL_SchedFail;

   if (s_context.sched_iface.SetEventCollection)
      status = s_context.sched_iface.SetEventCollection(s_context.sched_iface.context, s_context.session_id, state);

   return status == BEGL_SchedSuccess;
}

uint32_t bcm_sched_poll_event_timeline(
   size_t                   event_buffer_bytes,
   void                     *event_buffer,
   bool                     *lost_data,
   uint64_t                 *timestamp_us)
{
   uint32_t         oflow = 0;
   uint32_t         bytesCopied = 0;

   if (s_context.sched_iface.GetEventData)
      bytesCopied = s_context.sched_iface.GetEventData(s_context.sched_iface.context, s_context.session_id,
                                                       event_buffer_bytes, event_buffer, &oflow, timestamp_us);

   *lost_data = oflow != 0;

   return bytesCopied;
}

void bcm_sched_register_update_oldest_nfid(
   bcm_update_oldest_nfid_fn  update_fn
)
{
   if (s_context.sched_iface.RegisterUpdateOldestNFID)
      s_context.sched_iface.RegisterUpdateOldestNFID(s_context.sched_iface.context, s_context.session_id, update_fn);
}

int bcm_sched_create_fence(
   const struct bcm_sched_dependencies *completed_deps,
   const struct bcm_sched_dependencies *finalised_deps,
   bool force_create)
{
   demand(s_context.sched_iface.MakeFenceForJobs);
   return s_context.sched_iface.MakeFenceForJobs(
      s_context.session_id, completed_deps, finalised_deps, force_create);
}

int bcm_sched_create_fence_for_any_non_finalised(void)
{
   demand(s_context.sched_iface.MakeFenceForAnyNonFinalizedJob);
   return s_context.sched_iface.MakeFenceForAnyNonFinalizedJob(s_context.session_id);
}

bool v3d_platform_init(void)
{
   gmem_init();

   demand(s_context.sched_iface.Open != NULL);
   s_context.session_id = s_context.sched_iface.Open(s_context.sched_iface.context);
   if (s_context.session_id == NULL)
   {
      gmem_destroy();
      return false;
   }

   v3d_scheduler_init();
   return true;
}

void v3d_platform_shutdown(void)
{
   assert(s_context.session_id != NULL);

   v3d_scheduler_shutdown();

   demand(s_context.sched_iface.Close != NULL);
   s_context.sched_iface.Close(s_context.sched_iface.context, s_context.session_id);

   gmem_destroy();
}

void v3d_platform_fence_wait(int fence)
{
   if (fence < 0)
      return;

   demand(s_context.sched_iface.WaitFence != NULL);

   s_context.sched_iface.WaitFence(s_context.sched_iface.context, fence);
}

enum v3d_fence_status v3d_platform_fence_wait_timeout(int fence, int timeout)
{
   BEGL_FenceStatus  status;

   if (fence < 0)
      return V3D_FENCE_SIGNALED;

   demand(s_context.sched_iface.WaitFenceTimeout != NULL);

   status = s_context.sched_iface.WaitFenceTimeout(s_context.sched_iface.context, fence, timeout);

   return status == BEGL_FenceSignaled ? V3D_FENCE_SIGNALED : V3D_FENCE_TIMEOUT;
}

void v3d_platform_fence_close(int fence)
{
   if (fence < 0)
      return;

   demand(s_context.sched_iface.CloseFence != NULL);

   s_context.sched_iface.CloseFence(s_context.sched_iface.context, fence);
}

void v3d_get_info(struct v3d_idents *info)
{
   demand(s_context.sched_iface.GetInfo != NULL);
   demand(info != NULL);

   memset(info, 0, sizeof(*info));

   s_context.sched_iface.GetInfo(s_context.sched_iface.context, s_context.session_id, info);
}

/* Debug functions. These don't do anything on real HW. */
void v3d_platform_set_debug_callback(v3d_debug_callback_t callback, void *p) {}
void v3d_platform_set_fragment_shader_debug(bool enabled) {}
bool v3d_platform_get_fragment_shader_debug(void) { return false; }
void v3d_platform_set_vertex_shader_debug(bool enabled) {}
bool v3d_platform_get_vertex_shader_debug(void) { return false; }
