/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_parallel.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#include "middleware/khronos/common/2708/khrn_interlock_priv_4.h"
#include "middleware/khronos/common/2708/khrn_render_state_4.h"
#include "middleware/khronos/common/2708/khrn_tfconvert_4.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "middleware/khronos/common/khrn_debug_helper.h"
#include "middleware/khronos/ext/egl_khr_image.h"
#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/egl/egl_server.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/2708/glxx_inner_4.h"  /* for GL render state */
#include "bcm_perf_api.h"

#ifndef WIN32
#include <sys/times.h>
#endif

#ifdef __linux__
#include <sys/time.h>
#endif

#ifdef KHRN_AUTOCLIF
#include "tools/v3d/autoclif/autoclif.h"
#endif

#include <stdbool.h>

static uint32_t               s_jobCallbacksPending;
static VCOS_MUTEX_T           s_callbackMutex;
static uint64_t               s_jobSequenceIssued = 0;
static uint64_t               s_jobSequenceDone   = 0;
static VCOS_EVENT_T           s_jobDoneEvent;
static void                  *s_sessionId         = NULL;

void khrn_send_job(BEGL_HWJob *job);

/* order of init/term calls is important */

bool khrn_hw_common_init(void)
{
   s_jobCallbacksPending = 0;

   if (!khrn_hw_init()) {
      return false;
   }

   if (!khrn_nmem_init()) {
      khrn_hw_term();
      return false;
   }

   khrn_render_state_init();

   vcos_demand(vcos_mutex_create(&s_callbackMutex, "Job Callback Mutex") == VCOS_SUCCESS);
   vcos_demand(vcos_event_create(&s_jobDoneEvent, "Job Done Event") == VCOS_SUCCESS);

   return true;
}

void khrn_hw_common_term(void)
{
   khrn_render_state_term();
   khrn_nmem_term();
   khrn_hw_term();

   vcos_event_delete(&s_jobDoneEvent);
   vcos_mutex_delete(&s_callbackMutex);
}

void khrn_hw_common_flush(void)
{
   khrn_render_state_flush_all(KHRN_RENDER_STATE_TYPE_NONE);
}

void khrn_hw_common_finish(void)
{
   glxx_context_gl_lock();
   khrn_hw_common_flush();
   khrn_hw_common_wait();
   glxx_context_gl_unlock();
}

void khrn_hw_common_wait(void)
{
   khrn_hw_wait();
}

void lockCallback(void)
{
   vcos_mutex_lock(&s_callbackMutex);
}

void unlockCallback(void)
{
   vcos_mutex_unlock(&s_callbackMutex);
}

/* Schedule the given job */
void khrn_send_job(BEGL_HWJob *job)
{
   uint32_t i;

   lockCallback();
   s_jobSequenceIssued++;
   job->jobSequence = s_jobSequenceIssued;
   unlockCallback();

   job->collectTimeline = false;

   for (i = 0; i < BEGL_HW_JOB_MAX_INSTRUCTIONS; ++i)
   {
      if (job->program[i].operation == BEGL_HW_OP_END)
         break;

      if (job->program[i].callbackParam != 0)
         vcos_atomic_increment(&s_jobCallbacksPending);
   }

   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   driverInterfaces->hwInterface->SendJob(driverInterfaces->hwInterface->context, job);
}

void khrn_decrement_job_callbacks_pending(void)
{
   vcos_atomic_decrement(&s_jobCallbacksPending);
}

/* Returns true if there are still some scheduled jobs in flight. i.e. We have some callbacks
 * which haven't been hit yet.
 */
bool khrn_has_job_callbacks_pending(void)
{
   return s_jobCallbacksPending != 0;
}

void khrn_issue_fence_wait_job(uint64_t p)
{
   BEGL_HWCallbackRecord      *callback_data = (BEGL_HWCallbackRecord*)calloc(sizeof(BEGL_HWCallbackRecord), 1);
   BEGL_HWJob                 job;

   memset(&job, 0, sizeof(BEGL_HWJob));

   callback_data->reason = KHRN_FENCE_WAIT_DONE;

   job.program[0].operation      = BEGL_HW_OP_FENCE;
   job.program[0].arg2           = p;
   job.program[0].callbackParam  = (uint64_t)(uintptr_t)callback_data;

   platform_dbg_message_add("%s -> fence %p", __FUNCTION__, p);

   khrn_send_job(&job);
}

/* Issues a swapbuffers sync job */
void khrn_issue_swapbuffers_job(int fd, uint64_t p, char type)
{
   BEGL_HWCallbackRecord      *callback_data = (BEGL_HWCallbackRecord*)calloc(sizeof(BEGL_HWCallbackRecord), 1);
   BEGL_HWJob                 job;

   memset(&job, 0, sizeof(BEGL_HWJob));

   callback_data->reason      = KHRN_SWAPBUFFERS_DONE;
   callback_data->payload[0]  = fd;

   /* This is essentially an empty job which keeps everything in order.
      We will be notified when this job executes and the renderer is done.
      It differs from a WaitRender in that it doesn't stop later bin instructions
      from executing - thus allowing bin/render overlap. */
   job.program[0].operation      = BEGL_HW_OP_NOTIFY;
   job.program[0].arg1           = BEGL_HW_SIG_RENDER;
   job.program[0].arg2           = p;
   job.program[0].callbackParam  = (uint64_t)(uintptr_t)callback_data;

   platform_dbg_message_add("%s %c -> fd = %d, fence %p", __FUNCTION__, type, fd, p);

   khrn_send_job(&job);
}

/* Called when a swapbuffers job is complete */
void khrn_swapbuffers_job_done(BEGL_HWCallbackRecord *cbRecord)
{
   int fd = (int)cbRecord->payload[0];
   if (fd != -1)
   {
      BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
      /* optional, for instance Android has already done this in the kernel */
      if (driverInterfaces->hwInterface->FenceSignal)
         driverInterfaces->hwInterface->FenceSignal(driverInterfaces->hwInterface->context, fd);

      platform_dbg_message_add("%s -> fd = %d", __FUNCTION__, fd);
   }

   free(cbRecord);
}

void khrn_create_fence(int *fd, uint64_t *p, char type)
{
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   driverInterfaces->hwInterface->FenceOpen(driverInterfaces->hwInterface->context, fd, p, type);
}

uint64_t khrn_fence_wait_async(int fd)
{
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   uint64_t v3dfence = 0;
   if (fd != -1)
      driverInterfaces->hwInterface->FenceWaitAsync(driverInterfaces->hwInterface->context, fd, &v3dfence);
   return v3dfence;
}

void khrn_issue_bin_render_job(GLXX_HW_RENDER_STATE_T *rs, bool secure)
{
   BEGL_HWBinMemorySettings   settings = { 0 };
   BEGL_HWBinMemory           binMemory;
   BEGL_HWCallbackRecord      *callback_data = (BEGL_HWCallbackRecord*)calloc(sizeof(BEGL_HWCallbackRecord), 1);
   BEGL_HWJob                 job;
   KHRN_FMEM_T                *fmem = rs->fmem;
   uint32_t                   i = 0;
   bool                       abandon = false;

   memset(&job, 0, sizeof(BEGL_HWJob));

   callback_data->reason     = KHRN_BIN_RENDER_DONE;
   callback_data->payload[0] = (uint64_t)(uintptr_t)fmem;

   settings.secure = secure;
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   if (!driverInterfaces->hwInterface->GetBinMemory(driverInterfaces->hwInterface->context, &settings, &binMemory))
   {
      /* Flush out any pending jobs and try again */
      khrn_issue_finish_job();

      /* If this doesn't work, the job will have to be abandoned */
      abandon = !driverInterfaces->hwInterface->GetBinMemory(driverInterfaces->hwInterface->context, &settings, &binMemory);
   }

   job.binMemory = binMemory.address;
   job.binMemorySecure = secure;

   khrn_fmem_prep_for_job(fmem, binMemory.address, binMemory.size);

   /* rs->drawn is false if no drawing or clearing has occurred since the last swapBuffers.
      Treat that like an abandoned job too. */
   /* should read (!abandon && rs->drawn), but unfortunatly some demos call swapbuffers without drawing
      anything.  Causes corruption on startup which doesnt look good */
   uint32_t bin_start, bin_end, render_start, render_end;
   if (!abandon)
   {
      job.program[i].operation         = BEGL_HW_OP_SECURE;
      job.program[i].arg1              = secure;
      i++;

      if (rs->vshader_has_texture)
      {
         /* The vertex shader for this job uses a texture which could
          * have come from an FBO or EGLImage, so wait for any preceeding renders to complete
          */
         job.program[i].operation     = BEGL_HW_OP_WAIT;
         job.program[i].arg1          = BEGL_HW_SIG_RENDER;
         i++;
      }

      job.program[i].operation     = BEGL_HW_OP_BIN;
      bin_start                    = khrn_hw_addr(fmem->bin_begin, &fmem->bin_begin_lbh);
      bin_end                      = khrn_hw_addr(fmem->bin_end, &fmem->render_begin_lbh);
      job.program[i].arg1          = bin_start;
      job.program[i].arg2          = bin_end;
      i++;

      job.program[i].operation     = BEGL_HW_OP_WAIT;
      job.program[i].arg1          = BEGL_HW_SIG_BIN;
      i++;

      job.program[i].operation     = BEGL_HW_OP_RENDER;
      render_start                 = khrn_hw_addr(fmem->render_begin, &fmem->render_begin_lbh);
      render_end                   = khrn_hw_addr(fmem->cle_pos, &fmem->lbh);
      job.program[i].arg1          = render_start;
      job.program[i].arg2          = render_end;
      job.program[i].callbackParam = (uint64_t)(uintptr_t)callback_data;
   }
   else
   {
      /* This is an empty job which keeps everything in order */
      job.program[i].operation     = BEGL_HW_OP_WAIT;
      job.program[i].arg1          = BEGL_HW_SIG_BIN | BEGL_HW_SIG_RENDER;
      job.program[i].callbackParam = (uint64_t)(uintptr_t)callback_data;
   }

   /* Flush the dcache prior to the bin/render starting */
   khrn_fmem_flush(fmem);

#ifdef KHRN_AUTOCLIF
   V3D_IDENT_T ident = {
      .tlb_w = 64, /*KHRN_HW_TILE_WIDTH;*/
      .tlb_h = 64 /*KHRN_HW_TILE_HEIGHT;*/
   };
   /* platform interface uses standard types, so make sure they match prior to calling */
   vcos_static_assert(sizeof(V3D_ADDR_T) == sizeof(uint32_t));
   autoclif_begin(autoclif_addr_to_ptr, autoclif_ptr_to_addr, &ident);
   autoclif_bin(bin_start, bin_end);
   autoclif_render(render_start, render_end);
   autoclif_end(autoclif_get_clif_filename());
   /* not part of the autoclif library, but a call to the platform layer to reset */
   autoclif_reset();
#endif

   platform_dbg_message_add("%s -> abandon = %s", __FUNCTION__, abandon ? "true" : "false");

   khrn_send_job(&job);
}

/* Called when a bin/render job is complete */
void khrn_bin_render_done(BEGL_HWCallbackRecord *cbRecord)
{
   KHRN_FMEM_T    *fmem   = (KHRN_FMEM_T*)(uintptr_t)cbRecord->payload[0];

   khrn_job_done_fmem(fmem);

   free(cbRecord);
}

/* Submit a tf convert job */
void khrn_issue_tfconvert_job(KHRN_FMEM_T *fmem, bool secure)
{
   BEGL_HWCallbackRecord   *callback_data = (BEGL_HWCallbackRecord*)calloc(sizeof(BEGL_HWCallbackRecord), 1);
   BEGL_HWJob              job;

   memset(&job, 0, sizeof(BEGL_HWJob));

   callback_data->reason = KHRN_TFCONVERT_DONE;
   callback_data->payload[0] = (uint64_t)(uintptr_t)fmem;

   khrn_fmem_prep_for_render_only_job(fmem);

   job.program[0].operation = BEGL_HW_OP_SECURE;
   job.program[0].arg1 = secure;

   job.program[1].operation     = BEGL_HW_OP_RENDER;
   job.program[1].arg1          = khrn_hw_addr(fmem->render_begin, &fmem->render_begin_lbh);
   job.program[1].arg2          = khrn_hw_addr(fmem->cle_pos, &fmem->lbh);
   job.program[1].callbackParam = (uint64_t)((uintptr_t)callback_data);

   /* Flush the dcache prior to the conversion starting */
   khrn_fmem_flush(fmem);

   platform_dbg_message_add("%s", __FUNCTION__);

   khrn_send_job(&job);
}

/* Waits for a specific job to complete */
void khrn_wait_for_job_done(uint64_t jobSequenceNumber)
{
   bool needToWait = true;

   if (jobSequenceNumber == 0)   /* Job number can't be less than this, and it's a common use-case */
      return;

   while (needToWait)
   {
      lockCallback();

      needToWait = (s_jobSequenceDone < jobSequenceNumber);
      unlockCallback();

      if (needToWait)
         vcos_event_wait(&s_jobDoneEvent);
   }
}

/* Waits for all issued jobs to complete */
void khrn_issue_finish_job(void)
{
   bool needToWait = true;

   while (needToWait)
   {
      lockCallback();
      needToWait = (s_jobSequenceDone < s_jobSequenceIssued);
      unlockCallback();

      if (needToWait)
         vcos_event_wait(&s_jobDoneEvent);
   }
}

void khrn_signal_job_done(uint64_t jobSequence)
{
   s_jobSequenceDone = jobSequence;

   /* Signal anyone waiting on this job */
   vcos_event_signal(&s_jobDoneEvent);
}

void khrn_job_callback(void)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_HWNotification     notification;
   BEGL_HWCallbackRecord   *cbRecord;
   bool                    ok = true;

   lockCallback();

   notification.timelineData = NULL;

   /* Nexus does not queue callbacks, so we have to drain the notification queue here each time we get
      a callback. Not ideal, but it's all we can do. */
   while (driverInterfaces->hwInterface->GetNotification(driverInterfaces->hwInterface->context, &notification))
   {
      khrn_decrement_job_callbacks_pending();

      cbRecord = (BEGL_HWCallbackRecord*)notification.param;

      /* If the record is finished with, then these routines should free it up */
      switch (cbRecord->reason)
      {
      case KHRN_BIN_RENDER_DONE :
         khrn_signal_job_done(notification.jobSequence);
         khrn_bin_render_done(cbRecord);
         break;
      case KHRN_TFCONVERT_DONE :
         khrn_signal_job_done(notification.jobSequence);
         khrn_tfconvert_done(cbRecord);
         break;
      case KHRN_SWAPBUFFERS_DONE :
         khrn_signal_job_done(notification.jobSequence);
         khrn_swapbuffers_job_done(cbRecord);
         break;
      case KHRN_FENCE_WAIT_DONE :
         khrn_signal_job_done(notification.jobSequence);
         break;
      }

      if (notification.needsSync)
      {
         /* Acknowledge the sync callback */
         driverInterfaces->hwInterface->SendSync(driverInterfaces->hwInterface->context, !ok);
      }
   }

   unlockCallback();
}

uint64_t khrn_get_last_issued_seq(void)
{
   return s_jobSequenceIssued;
}

uint64_t khrn_get_last_done_seq(void)
{
   return s_jobSequenceDone;
}

uint32_t bcm_sched_get_num_counter_groups(void)
{
   uint32_t numGroups = 0;
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_SchedPerfCountInterface *pci = &driverInterfaces->hwInterface->perf_count_iface;
   if (pci->GetPerfNumCounterGroups)
      pci->GetPerfNumCounterGroups(driverInterfaces->hwInterface->context, s_sessionId, &numGroups);

   return numGroups;
}

bool bcm_sched_enumerate_group_counters(
   uint32_t                              group,
   struct bcm_sched_counter_group_desc  *group_desc)
{
   bool status = false;
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_SchedPerfCountInterface *pci = &driverInterfaces->hwInterface->perf_count_iface;

   if (pci->GetPerfCounterGroupInfo)
      status = pci->GetPerfCounterGroupInfo(driverInterfaces->hwInterface->context, s_sessionId,
                                            group, group_desc);

   return status;
}

bool bcm_sched_select_group_counters(
   const struct bcm_sched_group_counter_selector *selector)
{
   bool status = false;

   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_SchedPerfCountInterface *pci = &driverInterfaces->hwInterface->perf_count_iface;

   if (pci->ChoosePerfCounters)
      status = pci->ChoosePerfCounters(driverInterfaces->hwInterface->context, s_sessionId, selector);

   return status;
}

bool bcm_sched_set_counter_collection(
   enum bcm_sched_counter_state  state)
{
   bool status = true;

   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_SchedPerfCountInterface *pci = &driverInterfaces->hwInterface->perf_count_iface;

   if (pci->SetPerfCounting)
      status = pci->SetPerfCounting(driverInterfaces->hwInterface->context,
                                    s_sessionId, (BEGL_SchedCounterState)state);

   return status;
}

uint32_t bcm_sched_get_counters(
   struct bcm_sched_counter  *counters,
   uint32_t                   max_counters,
   bool                       reset_counts)
{
   uint32_t ctrs = 0;

   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_SchedPerfCountInterface *pci = &driverInterfaces->hwInterface->perf_count_iface;

   if (pci->GetPerfCounterData)
      ctrs = pci->GetPerfCounterData(driverInterfaces->hwInterface->context, s_sessionId, counters,
                                     max_counters, reset_counts);

   return ctrs;
}

uint32_t bcm_sched_get_num_event_tracks(void)
{
   uint32_t numTracks = 0;
   uint32_t numEvents = 0;

   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_SchedEventTrackInterface *eti = &driverInterfaces->hwInterface->event_track_iface;

   if (eti->GetEventCounts)
      eti->GetEventCounts(driverInterfaces->hwInterface->context, s_sessionId, &numTracks, &numEvents);

   return numTracks;
}

bool bcm_sched_describe_event_track(
   uint32_t                            track_index,
   struct bcm_sched_event_track_desc   *track_desc)
{
   bool status = false;

   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_SchedEventTrackInterface *eti = &driverInterfaces->hwInterface->event_track_iface;

   if (eti->GetEventTrackInfo)
      status = eti->GetEventTrackInfo(driverInterfaces->hwInterface->context, s_sessionId, track_index,
                                      track_desc);

   return status;
}

uint32_t bcm_sched_get_num_events(void)
{
   uint32_t numTracks = 0;
   uint32_t numEvents = 0;

   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_SchedEventTrackInterface *eti = &driverInterfaces->hwInterface->event_track_iface;

   if (eti->GetEventCounts)
      eti->GetEventCounts(driverInterfaces->hwInterface->context, s_sessionId, &numTracks, &numEvents);

   return numEvents;
}

bool bcm_sched_describe_event(
   uint32_t                      event_index,
   struct bcm_sched_event_desc   *event_desc)
{
   bool status = false;

   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_SchedEventTrackInterface *eti = &driverInterfaces->hwInterface->event_track_iface;

   if (eti->GetEventInfo)
      status = eti->GetEventInfo(driverInterfaces->hwInterface->context, s_sessionId, event_index,
                                 event_desc);

   return status;
}

bool bcm_sched_describe_event_data(
   uint32_t                            event_index,
   uint32_t                            field_index,
   struct bcm_sched_event_field_desc   *field_desc)
{
   bool status = false;

   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_SchedEventTrackInterface *eti = &driverInterfaces->hwInterface->event_track_iface;

   if (eti->GetEventDataFieldInfo)
      status = eti->GetEventDataFieldInfo(driverInterfaces->hwInterface->context, s_sessionId,
                                          event_index, field_index, field_desc);

   return status;
}

bool bcm_sched_set_event_collection(
   enum bcm_sched_event_state    state)
{
   bool status = false;

   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_SchedEventTrackInterface *eti = &driverInterfaces->hwInterface->event_track_iface;

   if (eti->SetEventCollection)
      status = eti->SetEventCollection(driverInterfaces->hwInterface->context,
                                       s_sessionId, (BEGL_SchedEventState)state);

   return status;
}

uint32_t bcm_sched_poll_event_timeline(
   size_t                   event_buffer_bytes,
   void                     *event_buffer,
   bool                     *lost_data,
   uint64_t                 *timestamp_us)
{
   uint32_t         oflow = 0;
   uint32_t         bytesCopied = 0;

   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_SchedEventTrackInterface *eti = &driverInterfaces->hwInterface->event_track_iface;

   if (eti->GetEventData)
      bytesCopied = eti->GetEventData(driverInterfaces->hwInterface->context, s_sessionId,
                                      event_buffer_bytes, event_buffer, &oflow, timestamp_us);

   *lost_data = oflow != 0;

   return bytesCopied;
}

void khrn_handlecpy(MEM_HANDLE_T hDst, size_t dstOffset, const void *src, size_t size)
{
#ifndef NDEBUG
   MEM_HEADER_T *h = (MEM_HEADER_T *)hDst;
   assert(hDst != MEM_HANDLE_INVALID);
   assert(h->magic == MAGIC);
   assert(dstOffset + size <= h->size);
#endif

   void *dst = (uint8_t *)mem_lock(hDst, NULL) + dstOffset;
   khrn_memcpy(dst, src, size);
   khrn_hw_flush_dcache_range(dst, size);
   mem_unlock(hDst);
}
