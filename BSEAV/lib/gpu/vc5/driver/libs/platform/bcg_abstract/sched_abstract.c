/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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

static void register_sched_interface(BEGL_SchedInterface *iface)
{
   if (iface != NULL)
   {
      demand(s_context.sched_iface.QueueJobs == NULL);
      s_context.sched_iface = *iface;                                    /* Register   */

   }
   else
      memset(&s_context.sched_iface, 0, sizeof(BEGL_SchedInterface));    /* Unregister */
}

// Public shared library entrypoint in the GLES driver for use by NXPL
__attribute__((visibility("default")))
void BEGL_RegisterSchedInterface(BEGL_SchedInterface *iface)
{
   register_sched_interface(iface);
}

// Private entrypoint for Vulkan driver library platform initialization.
// The vulkan platform cannot use the BEGL entrypoint; if the GLES library
// has already been loaded then it will act like an LD_PRELOAD and the wrong
// entrypoint will be taken and the wrong static context will be set.
void BVK_RegisterSchedInterface(BEGL_SchedInterface *iface)
{
   register_sched_interface(iface);
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
   const struct bcm_sched_dependencies *completed_deps,
   const struct bcm_sched_dependencies *finalised_deps,
   struct bcm_sched_query_response *response)
{
   demand(s_context.sched_iface.Query != NULL);

   s_context.sched_iface.Query(s_context.sched_iface.context, s_context.session_id, completed_deps, finalised_deps, response);
}

uint32_t bcm_sched_get_num_counter_groups(void)
{
   uint32_t numGroups = 0;

   BEGL_SchedPerfCountInterface *pci = &s_context.sched_iface.perf_count_iface;
   if (pci->GetPerfNumCounterGroups)
      pci->GetPerfNumCounterGroups(s_context.sched_iface.context, s_context.session_id, &numGroups);

   return numGroups;
}

bool bcm_sched_enumerate_group_counters(
   uint32_t                              group,
   struct bcm_sched_counter_group_desc  *group_desc)
{
   BEGL_SchedStatus status = BEGL_SchedFail;
   BEGL_SchedPerfCountInterface *pci = &s_context.sched_iface.perf_count_iface;

   if (pci->GetPerfCounterGroupInfo)
      status = pci->GetPerfCounterGroupInfo(s_context.sched_iface.context, s_context.session_id,
                                            group, group_desc);

   return status == BEGL_SchedSuccess;
}

bool bcm_sched_select_group_counters(
   const struct bcm_sched_group_counter_selector *selector)
{
   BEGL_SchedStatus status = BEGL_SchedFail;

   BEGL_SchedPerfCountInterface *pci = &s_context.sched_iface.perf_count_iface;

   if (pci->ChoosePerfCounters)
      status = pci->ChoosePerfCounters(s_context.sched_iface.context, s_context.session_id, selector);

   return status == BEGL_SchedSuccess;
}

bool bcm_sched_set_counter_collection(
   enum bcm_sched_counter_state  state)
{
   BEGL_SchedStatus status = BEGL_SchedFail;

   BEGL_SchedPerfCountInterface *pci = &s_context.sched_iface.perf_count_iface;

   if (pci->SetPerfCounting)
      status = pci->SetPerfCounting(s_context.sched_iface.context, s_context.session_id, state);

   return status == BEGL_SchedSuccess;
}

uint32_t bcm_sched_get_counters(
   struct bcm_sched_counter  *counters,
   uint32_t                   max_counters,
   bool                       reset_counts)
{
   uint32_t ctrs = 0;

   BEGL_SchedPerfCountInterface *pci = &s_context.sched_iface.perf_count_iface;

   if (pci->GetPerfCounterData)
      ctrs = pci->GetPerfCounterData(s_context.sched_iface.context, s_context.session_id, counters,
                                     max_counters, reset_counts);

   return ctrs;
}

uint32_t bcm_sched_get_num_event_tracks(void)
{
   uint32_t numTracks = 0;
   uint32_t numEvents = 0;

   BEGL_SchedEventTrackInterface *eti = &s_context.sched_iface.event_track_iface;

   if (eti->GetEventCounts)
      eti->GetEventCounts(s_context.sched_iface.context, s_context.session_id, &numTracks, &numEvents);

   return numTracks;
}

bool bcm_sched_describe_event_track(
   uint32_t                            track_index,
   struct bcm_sched_event_track_desc   *track_desc)
{
   BEGL_SchedStatus status = BEGL_SchedFail;

   BEGL_SchedEventTrackInterface *eti = &s_context.sched_iface.event_track_iface;

   if (eti->GetEventTrackInfo)
      status = eti->GetEventTrackInfo(s_context.sched_iface.context, s_context.session_id, track_index,
                                      track_desc);

   return status == BEGL_SchedSuccess;
}

uint32_t bcm_sched_get_num_events(void)
{
   uint32_t numTracks = 0;
   uint32_t numEvents = 0;

   BEGL_SchedEventTrackInterface *eti = &s_context.sched_iface.event_track_iface;

   if (eti->GetEventCounts)
      eti->GetEventCounts(s_context.sched_iface.context, s_context.session_id, &numTracks, &numEvents);

   return numEvents;
}

bool bcm_sched_describe_event(
   uint32_t                      event_index,
   struct bcm_sched_event_desc   *event_desc)
{
   BEGL_SchedStatus status = BEGL_SchedFail;

   BEGL_SchedEventTrackInterface *eti = &s_context.sched_iface.event_track_iface;

   if (eti->GetEventInfo)
      status = eti->GetEventInfo(s_context.sched_iface.context, s_context.session_id, event_index,
                                 event_desc);

   return status == BEGL_SchedSuccess;
}

bool bcm_sched_describe_event_data(
   uint32_t                            event_index,
   uint32_t                            field_index,
   struct bcm_sched_event_field_desc   *field_desc)
{
   BEGL_SchedStatus status = BEGL_SchedFail;

   BEGL_SchedEventTrackInterface *eti = &s_context.sched_iface.event_track_iface;

   if (eti->GetEventDataFieldInfo)
      status = eti->GetEventDataFieldInfo(s_context.sched_iface.context, s_context.session_id,
                                          event_index, field_index, field_desc);

   return status == BEGL_SchedSuccess;
}

bool bcm_sched_set_event_collection(
   enum bcm_sched_event_state    state)
{
   BEGL_SchedStatus status = BEGL_SchedFail;

   BEGL_SchedEventTrackInterface *eti = &s_context.sched_iface.event_track_iface;

   if (eti->SetEventCollection)
      status = eti->SetEventCollection(s_context.sched_iface.context, s_context.session_id, state);

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

   BEGL_SchedEventTrackInterface *eti = &s_context.sched_iface.event_track_iface;

   if (eti->GetEventData)
      bytesCopied = eti->GetEventData(s_context.sched_iface.context, s_context.session_id,
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
      s_context.sched_iface.context, completed_deps, finalised_deps, force_create);
}

bool bcm_sched_wait_for_any_non_finalised(void)
{
   demand(s_context.sched_iface.WaitForAnyNonFinalisedJob);
   return s_context.sched_iface.WaitForAnyNonFinalisedJob(
      s_context.sched_iface.context);
}

bcm_wait_status bcm_sched_wait_any_job_timeout(
   const struct bcm_sched_dependencies *completed_deps,
   const struct bcm_sched_dependencies *finalised_deps,
   int timeout)
{
   demand(s_context.sched_iface.WaitForAnyJobTimeout);
   return s_context.sched_iface.WaitForAnyJobTimeout(
      s_context.sched_iface.context, completed_deps, finalised_deps, timeout);
}

void bcm_sched_wait_jobs(
   const struct bcm_sched_dependencies *completed_deps,
   const struct bcm_sched_dependencies *finalised_deps)
{
   demand(s_context.sched_iface.WaitJobs);
   s_context.sched_iface.WaitJobs(
      s_context.sched_iface.context, completed_deps, finalised_deps);
}

bcm_wait_status bcm_sched_wait_jobs_timeout(
   const struct bcm_sched_dependencies *completed_deps,
   const struct bcm_sched_dependencies *finalised_deps,
   int timeout)
{
   demand(s_context.sched_iface.WaitJobsTimeout);
   return s_context.sched_iface.WaitJobsTimeout(
      s_context.sched_iface.context, completed_deps, finalised_deps, timeout);
}

// ===================================================================
// Functions to manipulate scheduler event state

bcm_sched_event_id bcm_sched_new_event(void)
{
   demand(s_context.sched_iface.NewSchedEvent);
   return s_context.sched_iface.NewSchedEvent(
      s_context.sched_iface.context);
}

void bcm_sched_delete_event(bcm_sched_event_id event_id)
{
   demand(s_context.sched_iface.DeleteSchedEvent);
   s_context.sched_iface.DeleteSchedEvent(
      s_context.sched_iface.context, event_id);
}

void bcm_sched_set_event(bcm_sched_event_id event_id)
{
   demand(s_context.sched_iface.SetSchedEvent);
   s_context.sched_iface.SetSchedEvent(
      s_context.sched_iface.context, event_id);
}

void bcm_sched_reset_event(bcm_sched_event_id event_id)
{
   demand(s_context.sched_iface.ResetSchedEvent);
   return s_context.sched_iface.ResetSchedEvent(
      s_context.sched_iface.context, event_id);
}

bool bcm_sched_query_event(bcm_sched_event_id event_id)
{
   demand(s_context.sched_iface.QuerySchedEvent);
   return s_context.sched_iface.QuerySchedEvent(
      s_context.sched_iface.context, event_id);
}

#if V3D_USE_CSD

v3d_compute_subjobs_id v3d_scheduler_new_compute_subjobs(unsigned max_subjobs)
{
   demand(s_context.sched_iface.NewComputeSubjobs);
   return s_context.sched_iface.NewComputeSubjobs(
      s_context.sched_iface.context, max_subjobs);
}

void v3d_scheduler_update_compute_subjobs(
   v3d_compute_subjobs_id subjobs_id,
   const v3d_compute_subjob* subjobs,
   unsigned num_subjobs)
{
   assert(num_subjobs != 0);
   for (unsigned s = 0; s != num_subjobs; ++s)
   {
      for (unsigned i = 0; i != 3; ++i)
         assert((subjobs[s].num_wgs[i] - 1) <= 0xffff);
   }

   demand(s_context.sched_iface.UpdateComputeSubjobs);
   return s_context.sched_iface.UpdateComputeSubjobs(
      s_context.sched_iface.context, subjobs_id, subjobs, num_subjobs);
}

#endif

// ===================================================================

bool v3d_platform_init(void)
{
   gmem_init();

   /*
    * Setup the MMU parts of the scheduler interface context
    * before we open it, if the interface exists.
    */
   if (s_context.sched_iface.SetMMUContext != NULL)
   {
      uint64_t phys = gmem_get_pagetable_physical_addr();
      v3d_addr_t maxVirt = gmem_get_mmu_max_virtual_addr();
      int64_t unsecureBinTrans = gmem_get_mmu_unsecure_bin_translation();
      int64_t secureBinTrans = gmem_get_mmu_secure_bin_translation();
      uint64_t platformToken = gmem_get_platform_token();

      s_context.sched_iface.SetMMUContext(s_context.sched_iface.context, phys,
           maxVirt, unsecureBinTrans, secureBinTrans, platformToken);
   }

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

void v3d_get_info(struct v3d_idents *info)
{
   demand(s_context.sched_iface.GetInfo != NULL);
   demand(info != NULL);

   memset(info, 0, sizeof(*info));

   s_context.sched_iface.GetInfo(s_context.sched_iface.context, s_context.session_id, info);
}

bool v3d_platform_explicit_sync(void)
{
   return s_context.sched_iface.ExplicitSync(s_context.sched_iface.context);
}

/* Debug functions. These don't do anything on real HW. */
void v3d_platform_set_debug_callback(v3d_debug_callback_t callback, void *p) {}
