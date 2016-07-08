/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Various cross-API hardware-specific functions
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_parallel.h"
#include "interface/khronos/common/khrn_api_interposer.h"
#include "middleware/khronos/common/2708/khrn_copy_buffer_4.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#include "middleware/khronos/common/2708/khrn_interlock_priv_4.h"
#include "middleware/khronos/common/2708/khrn_render_state_4.h"
#include "middleware/khronos/common/2708/khrn_tfconvert_4.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "middleware/khronos/ext/egl_khr_image.h"
#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/egl/egl_server.h"
#include "middleware/khronos/egl/egl_disp.h"
#include "middleware/khronos/glxx/2708/glxx_inner_4.h"  /* for GL render state */
#ifndef NO_OPENVG
#include "middleware/khronos/vg/vg_server.h"            /* for VG render state */
#include "middleware/khronos/vg/2708/vg_hw_priv_4.h"
#endif /* NO_OPENVG */

#ifndef WIN32
#include <sys/times.h>
#endif

#ifdef __linux__
#include <sys/time.h>
#endif

#ifdef ANDROID
#include <cutils/log.h>
#endif

static KHRN_DRIVER_COUNTERS_T s_driverCounters;
static uint32_t               s_jobCallbacksPending;
static VCOS_MUTEX_T           s_callbackMutex;
static uint64_t               s_jobSequenceIssued = 0;
static uint64_t               s_jobSequenceDone   = 0;
static VCOS_EVENT_T           s_jobDoneEvent;

void khrn_send_job(BEGL_HWJob *job);

#ifdef ANDROID
void selective_logd(const char *fmt, ...)
{
   char buffer[256];
   FILE *fp;

   snprintf(buffer, sizeof(buffer), "/proc/%d/cmdline", getpid());

   fp = fopen(buffer, "r");
   if (fp != NULL)
   {
      buffer[0] = '\0';
      fgets(buffer, sizeof(buffer), fp);
      fclose(fp);

      if (strcmp(buffer, "com.android.opengl.cts") == 0)
      {
         va_list args;
         va_start(args, fmt);
         vsnprintf(buffer, sizeof(buffer), fmt, args);
         LOGD("%s", buffer);
      }
   }
}
#endif

#ifdef TIMELINE_EVENT_LOGGING
static void EventLog(BEGL_HWNotification *notification, BEGL_HWCallbackRecord *cbRec, uint64_t sequence)
{
   EventData ev;

   ev.eventData = sequence & 0xFFFFFFFF;
   ev.desc = 0;

   switch (cbRec->reason)
   {
   case KHRN_BIN_RENDER_DONE :
   case KHRN_VG_DONE :
      ev.eventType = eEVENT_START;
      ev.eventCode = eEVENT_BIN;
      ev.eventRow  = eEVENT_BINNER;
      ev.eventSecs = notification->timelineData->binStart.secs;
      ev.eventNanosecs = 1000 * notification->timelineData->binStart.microsecs;
      khrn_remote_event_log(&ev);
      ev.eventType = eEVENT_START;
      ev.eventCode = eEVENT_RENDER;
      ev.eventRow  = eEVENT_RENDERER;
      ev.eventSecs = notification->timelineData->renderStart.secs;
      ev.eventNanosecs = 1000 * notification->timelineData->renderStart.microsecs;
      khrn_remote_event_log(&ev);
      ev.eventType = eEVENT_END;
      ev.eventCode = eEVENT_BIN;
      ev.eventRow  = eEVENT_BINNER;
      ev.eventSecs = notification->timelineData->binEnd.secs;
      ev.eventNanosecs = 1000 * notification->timelineData->binEnd.microsecs;
      khrn_remote_event_log(&ev);
      ev.eventType = eEVENT_END;
      ev.eventCode = eEVENT_RENDER;
      ev.eventRow  = eEVENT_RENDERER;
      ev.eventSecs = notification->timelineData->renderEnd.secs;
      ev.eventNanosecs = 1000 * notification->timelineData->renderEnd.microsecs;
      khrn_remote_event_log(&ev);
      break;
   case KHRN_COPYBUFFERS_DONE :
      ev.eventType = eEVENT_START;
      ev.eventCode = eEVENT_RENDER;
      ev.eventRow  = eEVENT_RENDERER;
      ev.eventSecs = notification->timelineData->renderStart.secs;
      ev.eventNanosecs = 1000 * notification->timelineData->renderStart.microsecs;
      khrn_remote_event_log(&ev);
      ev.eventType = eEVENT_END;
      ev.eventCode = eEVENT_RENDER;
      ev.eventRow  = eEVENT_RENDERER;
      ev.eventSecs = notification->timelineData->renderEnd.secs;
      ev.eventNanosecs = 1000 * notification->timelineData->renderEnd.microsecs;
      khrn_remote_event_log(&ev);
      break;
   case KHRN_TFCONVERT_DONE :
      khrn_tfconvert_logevents(notification, cbRec);
      break;
   default :
      break;
   }
}
#else
#define EventLog(n, c, s)
#endif

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

   job->collectTimeline = khrn_remote_event_logging();

   for (i = 0; i < BEGL_HW_JOB_MAX_INSTRUCTIONS; ++i)
   {
      if (job->program[i].operation == BEGL_HW_OP_END)
         break;

      if (job->program[i].callbackParam != 0)
         vcos_atomic_increment(&s_jobCallbacksPending);
   }

   BEGL_GetDriverInterfaces()->hwInterface->SendJob(BEGL_GetDriverInterfaces()->hwInterface->context, job);
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

/* Issues a swapbuffers sync job */
void khrn_issue_swapbuffers_job(MEM_HANDLE_T image)
{
   BEGL_HWCallbackRecord      *callback_data = (BEGL_HWCallbackRecord*)calloc(sizeof(BEGL_HWCallbackRecord), 1);
   BEGL_HWJob                 job;
   bool                       abandon = false;

   memset(&job, 0, sizeof(BEGL_HWJob));

   callback_data->reason     = KHRN_SWAPBUFFERS_DONE;
   callback_data->payload[0] = (uint32_t)image;

   /* This is essentially an empty job which keeps everything in order.
      We will be notified when this job executes and the renderer is done.
      It differs from a WaitRender in that it doesn't stop later bin instructions
      from executing - thus allowing bin/render overlap. */
   job.program[0].operation     = BEGL_HW_OP_NOTIFY;
   job.program[0].arg1          = BEGL_HW_SIG_RENDER;
   job.program[0].callbackParam = (uint32_t)callback_data;

   khrn_send_job(&job);
}

/* Called when a swapbuffers job is complete */
void khrn_swapbuffers_job_done(BEGL_HWCallbackRecord *cbRecord)
{
   MEM_HANDLE_T   image       = (MEM_HANDLE_T)cbRecord->payload[0];

   /* Push the completed image to the display */
   egl_disp_push(image);

   free(cbRecord);
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
   callback_data->payload[0] = (uint32_t)fmem;
   callback_data->payload[1] = (uint32_t)rs->fence_active;

   settings.secure = secure;
   if (!BEGL_GetDriverInterfaces()->hwInterface->GetBinMemory(BEGL_GetDriverInterfaces()->hwInterface->context, &settings, &binMemory))
   {
      /* Flush out any pending jobs and try again */
      khrn_issue_finish_job();

      /* If this doesn't work, the job will have to be abandoned */
      abandon = !BEGL_GetDriverInterfaces()->hwInterface->GetBinMemory(BEGL_GetDriverInterfaces()->hwInterface->context, &settings, &binMemory);
   }

   job.binMemory = binMemory.address;
   job.binMemorySecure = secure;

   khrn_fmem_prep_for_job(fmem, binMemory.address, binMemory.size);

   /* rs->drawn is false if no drawing or clearing has occurred since the last swapBuffers.
      Treat that like an abandoned job too. */
   /* should read (!abandon && rs->drawn), but unfortunatly some demos call swapbuffers without drawing
      anything.  Causes corruption on startup which doesnt look good */
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
      job.program[i].arg1          = khrn_hw_addr(fmem->bin_begin, &fmem->bin_begin_lbh);
      job.program[i].arg2          = khrn_hw_addr(fmem->bin_end, &fmem->render_begin_lbh);
      i++;

      job.program[i].operation     = BEGL_HW_OP_WAIT;
      job.program[i].arg1          = BEGL_HW_SIG_BIN;
      i++;

      if (rs->fence != -1)
      {
         job.program[i].operation  = BEGL_HW_OP_FENCE;
         job.program[i].arg1       = rs->fence;
         rs->fence                 = -1;
         i++;
      }

      job.program[i].operation     = BEGL_HW_OP_RENDER;
      job.program[i].arg1          = khrn_hw_addr(fmem->render_begin, &fmem->render_begin_lbh);
      job.program[i].arg2          = khrn_hw_addr(fmem->cle_pos, &fmem->lbh);
      job.program[i].callbackParam = (uint32_t)callback_data;
   }
   else
   {
      /* This is an empty job which keeps everything in order */
      job.program[i].operation     = BEGL_HW_OP_WAIT;
      job.program[i].arg1          = BEGL_HW_SIG_BIN | BEGL_HW_SIG_RENDER;
      job.program[i].callbackParam = (uint32_t)callback_data;
   }

   /* Flush the dcache prior to the bin/render starting */
   khrn_hw_flush_dcache();

   khrn_send_job(&job);
}

/* Called when a bin/render job is complete */
void khrn_bin_render_done(BEGL_HWCallbackRecord *cbRecord, uint64_t job_sequence)
{
   KHRN_FMEM_T    *fmem   = (KHRN_FMEM_T*)cbRecord->payload[0];
   bool           fence   = (bool)cbRecord->payload[1];

   khrn_job_done_fmem(fmem);

   /* If this render state has waiting fences, notify them now */
   if (fence)
      egl_khr_fence_update(job_sequence);

   free(cbRecord);
}

#ifndef NO_OPENVG
void khrn_issue_vg_job(VG_BE_RENDER_STATE_T *rs,
                       bool loadFrameUsed,
                       bool storeFrameUsed,
                       bool maskUsed)
{
   BEGL_HWBinMemorySettings   settings = { 0 };
   BEGL_HWBinMemory           binMemory;
   BEGL_HWCallbackRecord      *callback_data = (BEGL_HWCallbackRecord*)calloc(sizeof(BEGL_HWCallbackRecord), 1);
   BEGL_HWJob                 job;
   KHRN_FMEM_T                *fmem = rs->fmem;
   bool                       abandon = false;

   memset(&job, 0, sizeof(BEGL_HWJob));

   callback_data->reason = KHRN_VG_DONE;
   callback_data->payload[0] = (uint32_t)fmem;
   callback_data->payload[1] = (uint32_t)rs->tess;
   callback_data->payload[2] = (uint32_t)rs->state;
   callback_data->payload[3] = storeFrameUsed ? (uint32_t)rs->frame : MEM_INVALID_HANDLE;
   callback_data->payload[4] = loadFrameUsed ? (uint32_t)rs->frame_src : MEM_INVALID_HANDLE;
   callback_data->payload[5] = maskUsed ? (uint32_t)rs->mask : MEM_INVALID_HANDLE;
   callback_data->payload[6] = (uint32_t)rs->fence_active;

   if (!BEGL_GetDriverInterfaces()->hwInterface->GetBinMemory(BEGL_GetDriverInterfaces()->hwInterface->context, &settings, &binMemory))
   {
      /* Flush out any pending jobs and try again */
      khrn_issue_finish_job();

      /* If this doesn't work, the job will have to be abandoned */
      abandon = !BEGL_GetDriverInterfaces()->hwInterface->GetBinMemory(BEGL_GetDriverInterfaces()->hwInterface->context, &settings, &binMemory);
   }

   job.binMemory = binMemory.address;
   job.binMemorySecure = false;

   khrn_fmem_prep_for_job(fmem, binMemory.address, binMemory.size);

   if (!abandon)
   {
      uint32_t i = 0;
      job.program[i].operation     = BEGL_HW_OP_BIN;
      job.program[i].arg1          = khrn_hw_addr(fmem->bin_begin, &fmem->bin_begin_lbh);
      job.program[i].arg2          = khrn_hw_addr(fmem->bin_end, &fmem->render_begin_lbh);
      i++;

      job.program[i].operation     = BEGL_HW_OP_WAIT;
      job.program[i].arg1          = BEGL_HW_SIG_BIN;
      i++;

      if (rs->fence != -1)
      {
         job.program[i].operation  = BEGL_HW_OP_FENCE;
         job.program[i].arg1       = rs->fence;
         rs->fence                 = -1;
         i++;
      }

      job.program[i].operation     = BEGL_HW_OP_RENDER;
      job.program[i].arg1          = khrn_hw_addr(fmem->render_begin, &fmem->render_begin_lbh);
      job.program[i].arg2          = khrn_hw_addr(fmem->cle_pos, &fmem->lbh);
      job.program[i].callbackParam = (uint32_t)callback_data;
      i++;
   }
   else
   {
      /* This is an empty job which keeps everything in order */
      job.program[0].operation     = BEGL_HW_OP_WAIT;
      job.program[0].arg1          = BEGL_HW_SIG_BIN | BEGL_HW_SIG_RENDER;
      job.program[0].callbackParam = (uint32_t)callback_data;
   }

   /* Flush the dcache prior to the bin/render starting */
   khrn_hw_flush_dcache();

   khrn_send_job(&job);
}

/* Called when a vg job is complete */
void khrn_vg_job_done(BEGL_HWCallbackRecord *cbRecord, uint64_t job_sequence)
{
   KHRN_FMEM_T       *fmem  = (KHRN_FMEM_T*)cbRecord->payload[0];
   VG_TESS_HANDLE_T  tess   = (VG_TESS_HANDLE_T)cbRecord->payload[1];
   MEM_HANDLE_T      state  = (VG_TESS_HANDLE_T)cbRecord->payload[2];
   MEM_HANDLE_T      frame  = (MEM_HANDLE_T)cbRecord->payload[3];
   MEM_HANDLE_T      frame_src = (MEM_HANDLE_T)cbRecord->payload[4];
   MEM_HANDLE_T      mask   = (MEM_HANDLE_T)cbRecord->payload[5];
   bool              fence  = (bool)cbRecord->payload[6];

   vg_tess_finish_llat(tess);

   if (mask != MEM_INVALID_HANDLE)
      mem_release(mask);

   if (frame != MEM_INVALID_HANDLE)
   {
      if ((frame_src != MEM_INVALID_HANDLE) && (frame_src != frame))
         mem_release(frame_src);
      mem_release(frame);
   }
   mem_release(state); /* this might call vg_be_term! */

   vg_tess_finish(tess);

   khrn_job_done_fmem(fmem);

   /* If this render state has waiting fences, notify them now */
   if (fence)
      egl_khr_fence_update(job_sequence);

   free(cbRecord);
}
#endif /* NO_OPENVG */

/* Submit a tf convert job */
void khrn_issue_tfconvert_job(KHRN_FMEM_T *fmem, MEM_HANDLE_T heglimage,
   uint8_t *ctrlListPtr, uint32_t numBytes, bool secure)
{
   BEGL_HWCallbackRecord   *callback_data = (BEGL_HWCallbackRecord*)calloc(sizeof(BEGL_HWCallbackRecord), 1);
   BEGL_HWJob              job;
   CONVERT_CALLBACK_DATA_T *convData = (CONVERT_CALLBACK_DATA_T*)&(callback_data->payload[0]);

   memset(&job, 0, sizeof(BEGL_HWJob));

   callback_data->reason = KHRN_TFCONVERT_LOCK;
   callback_data->payload[0] = (uint32_t)fmem;

   khrn_fmem_prep_for_render_only_job(fmem);

   convData->fmem = fmem;
   MEM_ASSIGN(convData->heglimage, heglimage);
   convData->controlList = ctrlListPtr;
   convData->conversionKilled = false;
   convData->semaphoreTaken = false;
   convData->numCLBytes = numBytes;

   job.program[0].operation = BEGL_HW_OP_SECURE;
   job.program[0].arg1 = secure;

   job.program[1].operation     = BEGL_HW_OP_SYNC;
   job.program[1].callbackParam = (uint32_t)callback_data;

   job.program[2].operation     = BEGL_HW_OP_RENDER;
   job.program[2].arg1          = khrn_hw_addr(fmem->render_begin, &fmem->render_begin_lbh);
   job.program[2].arg2          = khrn_hw_addr(fmem->cle_pos, &fmem->lbh);
   job.program[2].callbackParam = (uint32_t)callback_data;

   /* The dcache will be flushed during the Sync callback for lock, so no need to do it here */

   khrn_send_job(&job);
}

/* Submit a job to copy a buffer */
void khrn_issue_copy_buffer_job(KHRN_FMEM_T *fmem, MEM_HANDLE_T dst, MEM_HANDLE_T src, uint32_t numCLBytes, bool secure)
{
   BEGL_HWCallbackRecord   *callback_data = (BEGL_HWCallbackRecord*)calloc(sizeof(BEGL_HWCallbackRecord), 1);
   BEGL_HWJob              job;

   memset(&job, 0, sizeof(BEGL_HWJob));

   callback_data->reason = KHRN_COPYBUFFERS_DONE;
   callback_data->payload[0] = (uint32_t)fmem;
   callback_data->payload[1] = (uint32_t)dst;
   callback_data->payload[2] = (uint32_t)src;

   khrn_fmem_prep_for_render_only_job(fmem);

   job.program[0].operation = BEGL_HW_OP_SECURE;
   job.program[0].arg1 = secure;

   job.program[1].operation     = BEGL_HW_OP_RENDER;
   job.program[1].arg1          = khrn_hw_addr(fmem->render_begin, &fmem->render_begin_lbh);
   job.program[1].arg2          = khrn_hw_addr(fmem->cle_pos, &fmem->lbh);
   job.program[1].callbackParam = (uint32_t)callback_data;

   khrn_hw_flush_dcache_range(fmem->render_begin, numCLBytes);

   khrn_send_job(&job);
}

/* Called when a copy buffer job is complete */
void khrn_copy_buffer_job_done(BEGL_HWCallbackRecord *cbRecord)
{
   KHRN_FMEM_T    *fmem   = (KHRN_FMEM_T*)cbRecord->payload[0];

   khrn_job_done_fmem(fmem);

   free(cbRecord);
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
   BEGL_HWTimelineData     timeline;
   BEGL_HWCallbackRecord   *cbRecord;
   bool                    ok = true;

   lockCallback();

   if (khrn_remote_event_logging())
      notification.timelineData = &timeline;
   else
      notification.timelineData = NULL;

   /* Nexus does not queue callbacks, so we have to drain the notification queue here each time we get
      a callback. Not ideal, but it's all we can do. */
   while (driverInterfaces->hwInterface->GetNotification(driverInterfaces->hwInterface->context, &notification))
   {
      khrn_decrement_job_callbacks_pending();

      cbRecord = (BEGL_HWCallbackRecord*)notification.param;

      /* Log any timeline data */
      if (notification.timelineData)
         EventLog(&notification, cbRecord, notification.jobSequence);

      /* If the record is finished with, then these routines should free it up */
      switch (cbRecord->reason)
      {
      case KHRN_BIN_RENDER_DONE :
         khrn_signal_job_done(notification.jobSequence);
         khrn_bin_render_done(cbRecord, notification.jobSequence);
         break;
      case KHRN_TFCONVERT_LOCK :
         ok = khrn_tfconvert_lock(cbRecord);
         break;
      case KHRN_TFCONVERT_DONE :
         khrn_signal_job_done(notification.jobSequence);
         khrn_tfconvert_done(cbRecord);
         break;
#ifndef NO_OPENVG
      case KHRN_VG_DONE :
         khrn_signal_job_done(notification.jobSequence);
         khrn_vg_job_done(cbRecord, notification.jobSequence);
         break;
#endif /* NO_OPENVG */
      case KHRN_SWAPBUFFERS_DONE :
         khrn_signal_job_done(notification.jobSequence);
         khrn_swapbuffers_job_done(cbRecord);
         break;
      case KHRN_COPYBUFFERS_DONE :
         khrn_signal_job_done(notification.jobSequence);
         khrn_copy_buffer_job_done(cbRecord);
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

void khrn_init_driver_counters(int32_t hw_bank, int32_t l3c_bank)
{
   BEGL_HWPerfMonitorSettings settings;

#ifdef __linux__
   struct timeval curTime;
   unsigned int nowMs;
   struct tms cput;
   uint32_t cputime;
   clock_t ticks;
#endif
   memset(&s_driverCounters, 0, sizeof(KHRN_DRIVER_COUNTERS_T));

#ifdef __linux__
   /* reset the time */
   gettimeofday(&curTime, NULL);
   nowMs = curTime.tv_usec / 1000;
   nowMs += curTime.tv_sec * 1000;

   s_driverCounters.reset_time = nowMs;

   ticks = times(&cput);
   cputime = cput.tms_utime + cput.tms_stime + cput.tms_cutime + cput.tms_cstime;

   s_driverCounters.last_cpu_time = cputime;
   s_driverCounters.last_cpu_ticks = (uint32_t)ticks;
#endif

   s_driverCounters.hw_group_active = hw_bank;
   s_driverCounters.l3c_group_active = l3c_bank;

   settings.hwBank  = hw_bank + 1;
   settings.memBank = l3c_bank + 1;

   if (hw_bank < 0 && l3c_bank < 0)
      settings.flags = BEGL_HW_PERF_STOP;
   else
      settings.flags = BEGL_HW_PERF_START | BEGL_HW_PERF_RESET;

   if (BEGL_GetDriverInterfaces()->hwInterface->SetPerformanceMonitor)
      BEGL_GetDriverInterfaces()->hwInterface->SetPerformanceMonitor(BEGL_GetDriverInterfaces()->hwInterface->context,
                                                                     &settings);
}

KHRN_DRIVER_COUNTERS_T *khrn_driver_counters(void)
{
   return &s_driverCounters;
}

void khrn_update_perf_counters(void)
{
   BEGL_HWPerfMonitorData data;
   if (BEGL_GetDriverInterfaces()->hwInterface->GetPerformanceData)
      BEGL_GetDriverInterfaces()->hwInterface->GetPerformanceData(BEGL_GetDriverInterfaces()->hwInterface->context, &data);
   else
      memset(&data, 0, sizeof(BEGL_HWPerfMonitorData));

   if (s_driverCounters.hw_group_active == 0)
   {
      s_driverCounters.hw.hw_0.qpu_cycles_idle                    += data.hwCounters[0];
      s_driverCounters.hw.hw_0.qpu_cycles_vert_shade              += data.hwCounters[1];
      s_driverCounters.hw.hw_0.qpu_cycles_frag_shade              += data.hwCounters[2];
      s_driverCounters.hw.hw_0.qpu_cycles_exe_valid               += data.hwCounters[3];
      s_driverCounters.hw.hw_0.qpu_cycles_wait_tmu                += data.hwCounters[4];
      s_driverCounters.hw.hw_0.qpu_cycles_wait_scb                += data.hwCounters[5];
      s_driverCounters.hw.hw_0.qpu_cycles_wait_vary               += data.hwCounters[6];
      s_driverCounters.hw.hw_0.qpu_icache_hits                    += data.hwCounters[7];
      s_driverCounters.hw.hw_0.qpu_icache_miss                    += data.hwCounters[8];
      s_driverCounters.hw.hw_0.qpu_ucache_hits                    += data.hwCounters[9];
      s_driverCounters.hw.hw_0.qpu_ucache_miss                    += data.hwCounters[10];
      s_driverCounters.hw.hw_0.tmu_total_quads                    += data.hwCounters[11];
      s_driverCounters.hw.hw_0.tmu_cache_miss                     += data.hwCounters[12];
      s_driverCounters.hw.hw_0.l2c_hits                           += data.hwCounters[13];
      s_driverCounters.hw.hw_0.l2c_miss                           += data.hwCounters[14];
   }
   else if (s_driverCounters.hw_group_active == 1)
   {
      s_driverCounters.hw.hw_1.fep_valid_prims                    += data.hwCounters[0];
      s_driverCounters.hw.hw_1.fep_valid_prims_no_pixels          += data.hwCounters[1];
      s_driverCounters.hw.hw_1.fep_earlyz_clipped_quads           += data.hwCounters[2];
      s_driverCounters.hw.hw_1.fep_valid_quads                    += data.hwCounters[3];
      s_driverCounters.hw.hw_1.tlb_quads_no_stencil_pass_pixels   += data.hwCounters[4];
      s_driverCounters.hw.hw_1.tlb_quads_no_z_stencil_pass_pixels += data.hwCounters[5];
      s_driverCounters.hw.hw_1.tlb_quads_z_stencil_pass_pixels    += data.hwCounters[6];
      s_driverCounters.hw.hw_1.tlb_quads_all_pixels_zero_cvg      += data.hwCounters[7];
      s_driverCounters.hw.hw_1.tlb_quads_all_pixels_nonzero_cvg   += data.hwCounters[8];
      s_driverCounters.hw.hw_1.tlb_quads_valid_pixels_written     += data.hwCounters[9];
      s_driverCounters.hw.hw_1.ptb_prims_viewport_discarded       += data.hwCounters[10];
      s_driverCounters.hw.hw_1.ptb_prims_needing_clip             += data.hwCounters[11];
      s_driverCounters.hw.hw_1.pse_prims_reverse_discarded        += data.hwCounters[12];
      s_driverCounters.hw.hw_1.vpm_cycles_vdw_stalled             += data.hwCounters[13];
      s_driverCounters.hw.hw_1.vpm_cycles_vcd_stalled             += data.hwCounters[14];
   }

#ifndef CARBON
   if (s_driverCounters.l3c_group_active == 0)
   {
      s_driverCounters.l3c.l3c_read_bw_0          += data.memCounters[0];
      s_driverCounters.l3c_mem.l3c_mem_read_bw_0  += data.memCounters[1];
   }
   else if (s_driverCounters.l3c_group_active == 1)
   {
      s_driverCounters.l3c.l3c_write_bw_1         += data.memCounters[0];
      s_driverCounters.l3c_mem.l3c_mem_write_bw_1 += data.memCounters[1];
   }
#endif
}
