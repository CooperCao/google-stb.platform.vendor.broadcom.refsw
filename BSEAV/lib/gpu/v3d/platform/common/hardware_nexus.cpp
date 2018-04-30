/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "nexus_platform.h"
#include "nexus_graphicsv3d.h"
#include "bcm_perf_api.h"
#include "perf_event.h"

#include "bkni_multi.h"

#include <EGL/begl_platform.h>

#include <cassert>
#include <cstdio>

#include <memory>
#include <map>
#include <mutex>

/* #include <cutils/log.h> */

#include "../helpers/semaphore.h"

/* Static checks */
#define CASSERT(EXPR, MSG) typedef int assertion_failed_##MSG[(EXPR) ? 1 : -1]

/* Static checks on structures */
#define CHECK_STRUCT(BCM_NAME, NEXUS_NAME) \
   CASSERT(sizeof(struct BCM_NAME) == sizeof(NEXUS_NAME), mismatched_sizes_##BCM_NAME##_and_##NEXUS_NAME)

CHECK_STRUCT(bcm_sched_counter_desc,            NEXUS_Graphicsv3dCounterDesc);
CHECK_STRUCT(bcm_sched_counter_group_desc,      NEXUS_Graphicsv3dCounterGroupDesc);
CHECK_STRUCT(bcm_sched_group_counter_selector,  NEXUS_Graphicsv3dCounterSelector);
CHECK_STRUCT(bcm_sched_counter,                 NEXUS_Graphicsv3dCounter);

CHECK_STRUCT(bcm_sched_event_desc,              NEXUS_Graphicsv3dEventDesc);
CHECK_STRUCT(bcm_sched_event_field_desc,        NEXUS_Graphicsv3dEventFieldDesc);
CHECK_STRUCT(bcm_sched_event_track_desc,        NEXUS_Graphicsv3dEventTrackDesc);

typedef void (*CallbackFunc)(void);

typedef struct
{
   NEXUS_Graphicsv3dHandle nexusHandle;
   CallbackFunc            callback;
   std::mutex              fenceLock;
   std::map<int, void *>   fenceMap;
   int                     fence;
   EventContext           *eventContext;
   bool                    eventContextIsExternal;
} NXPL_HWData;

/*****************************************************************************
 * H/W driver interface
 *****************************************************************************/

static void JobCallbackHandler(void *context, int param __attribute__((unused)))
{
   NXPL_HWData *data = static_cast<NXPL_HWData*>(context);

   if (data && data->callback)
      data->callback();
}

/* Get hardware info */
static bool GetInfo(void *context __attribute__((unused)), BEGL_HWInfo *info)
{
   NEXUS_Graphicsv3dInfo nInfo;
   NEXUS_Error err = NEXUS_Graphicsv3d_GetInfo(&nInfo);

   if (err == NEXUS_SUCCESS)
   {
      info->numSlices   = nInfo.uiNumSlices;
      info->revision    = nInfo.uiRevision;
      info->techRev     = nInfo.uiTechRev;
      info->textureUnitsPerSlice = nInfo.uiTextureUnitsPerSlice;
      snprintf(info->name, sizeof(info->name), "%s", nInfo.chName);
      snprintf(info->revStr, sizeof(info->revStr), "%s", nInfo.chRevStr);
      NEXUS_Graphicsv3d_GetTime(&info->time);
      return true;
   }

   return false;
}

/* Send a job to the V3D job queue */
bool SendJob(void *context, BEGL_HWJob *job)
{
   NXPL_HWData *data = static_cast<NXPL_HWData*>(context);

   if (data != NULL)
   {
      NEXUS_Graphicsv3dJob nJob;
      for (int i = 0; i < NEXUS_GRAPHICSV3D_JOB_MAX_INSTRUCTIONS; i++)
      {
         nJob.sProgram[i].eOperation      = static_cast<NEXUS_Graphicsv3dOperation>(job->program[i].operation);    /* We are assuming the enum values are the same */
         nJob.sProgram[i].uiArg1          = job->program[i].arg1;
         nJob.sProgram[i].uiArg2          = job->program[i].arg2;
         nJob.sProgram[i].uiCallbackParam = job->program[i].callbackParam;
      }

      nJob.uiBinMemory      = job->binMemory;
      nJob.bBinMemorySecure = job->binMemorySecure;
      nJob.uiUserVPM        = job->userVPM;
      nJob.bCollectTimeline = job->collectTimeline;
      nJob.uiJobSequence    = job->jobSequence;

#if 0
      if (nJob.sProgram[0].eOperation == NEXUS_Graphicsv3dOperation_eNotifyInstr)
         ALOGE("job->program[0].arg2 = %p\n", (void *)((uintptr_t)job->program[0].arg2));
#endif

      NEXUS_Error err = NEXUS_Graphicsv3d_SendJob(data->nexusHandle, &nJob);

      return err == NEXUS_SUCCESS;
   }
#if 0
   else
      ALOGE("unable to send job, dead client\n");
#endif

   return false;
}

/* Retrieves and removes latest notification for this client */
bool GetNotification(void *context, BEGL_HWNotification *notification)
{
   NXPL_HWData *data = static_cast<NXPL_HWData*>(context);

   if (data != NULL)
   {
      NEXUS_Graphicsv3dNotification nNot;
      NEXUS_Error err = NEXUS_Graphicsv3d_GetNotification(data->nexusHandle, &nNot);

      if (err == NEXUS_SUCCESS)
      {
         notification->needsSync   = nNot.uiSync;
         notification->param       = reinterpret_cast<void *>(nNot.uiParam);
         notification->outOfMemory = nNot.uiOutOfMemory;
         notification->jobSequence = nNot.uiJobSequence;

         if (notification->timelineData != NULL)
         {
            NEXUS_Graphicsv3dTimelineData nTimeline;
            err = NEXUS_Graphicsv3d_GetTimelineForLastNotification(data->nexusHandle, &nTimeline);
            if (err == NEXUS_SUCCESS)
            {
               notification->timelineData->binStart.secs         = nTimeline.sBinStart.uiSecs;
               notification->timelineData->binStart.microsecs    = nTimeline.sBinStart.uiMicrosecs;
               notification->timelineData->binEnd.secs           = nTimeline.sBinEnd.uiSecs;
               notification->timelineData->binEnd.microsecs      = nTimeline.sBinEnd.uiMicrosecs;
               notification->timelineData->renderStart.secs      = nTimeline.sRenderStart.uiSecs;
               notification->timelineData->renderStart.microsecs = nTimeline.sRenderStart.uiMicrosecs;
               notification->timelineData->renderEnd.secs        = nTimeline.sRenderEnd.uiSecs;
               notification->timelineData->renderEnd.microsecs   = nTimeline.sRenderEnd.uiMicrosecs;
               notification->timelineData->userStart.secs        = nTimeline.sUserStart.uiSecs;
               notification->timelineData->userStart.microsecs   = nTimeline.sUserStart.uiMicrosecs;
               notification->timelineData->userEnd.secs          = nTimeline.sUserEnd.uiSecs;
               notification->timelineData->userEnd.microsecs     = nTimeline.sUserEnd.uiMicrosecs;
            }
         }
      }

      return err == NEXUS_SUCCESS;
   }

   return false;
}

/* Acknowledge the last synchronous notification */
static void SendSync(void *context, bool abandon)
{
   NXPL_HWData *data = static_cast<NXPL_HWData*>(context);

   if (data != NULL)
      NEXUS_Graphicsv3d_SendSync(data->nexusHandle, abandon);
}

/* Request bin memory */
static bool GetBinMemory(void *context, const BEGL_HWBinMemorySettings *settings, BEGL_HWBinMemory *memory)
{
   if (settings == NULL || memory == NULL)
      return false;

   NEXUS_Graphicsv3dBinMemory nexusMemory;
   NEXUS_Graphicsv3dBinMemorySettings nexusMemorySettings = { /*.bSecure = */ settings->secure };

   NXPL_HWData *data = static_cast<NXPL_HWData*>(context);
   NEXUS_Error rc = NEXUS_Graphicsv3d_GetBinMemory(data->nexusHandle, &nexusMemorySettings, &nexusMemory);

   if (rc == NEXUS_SUCCESS)
   {
      memory->address = nexusMemory.uiAddress;
      memory->size    = nexusMemory.uiSize;
   }
   else
   {
      memory->address = 0;
      memory->size    = 0;
   }

   return rc == NEXUS_SUCCESS;
}

#ifdef ANDROID
static void FenceOpen(void *context, int *fd, uint64_t *p, char type)
{
   NXPL_HWData *data = static_cast<NXPL_HWData*>(context);

   if (data != NULL)
      NEXUS_Graphicsv3d_FenceOpen(data->nexusHandle, fd, p, type, getpid());
}

static void FenceWaitAsync(void *context, int fd, uint64_t *v3dfence)
{
   NXPL_HWData *data = static_cast<NXPL_HWData*>(context);

   if (data != NULL)
      NEXUS_Graphicsv3d_FenceWaitAsync(data->nexusHandle, fd, v3dfence);
}
#else
static void FenceOpen(void *context, int *fd, uint64_t *p, char type __attribute__((unused)))
{
   NXPL_HWData *data = static_cast<NXPL_HWData*>(context);

   std::unique_lock<std::mutex> lock(data->fenceLock);

   auto m = new helper::Semaphore();
   int fence = data->fence++;
   data->fenceMap.insert(std::pair<int, void *>(fence, m));
   *fd = fence;
   *p = 0;  // TODO: probably remove this
}

// only required on platforms with a consumer thread/managing own display.
// Android is handled in the kernel
static void FenceSignal(void *context, int fd)
{
   NXPL_HWData *data = static_cast<NXPL_HWData*>(context);

   std::unique_lock<std::mutex> lock(data->fenceLock);

   auto it = data->fenceMap.find(fd);
   if (it != data->fenceMap.end())
   {
      auto m = static_cast<helper::Semaphore *>(it->second);
      m->notify();
      data->fenceMap.erase(it);
   }
}

// only required on platforms with a consumer thread/managing own display.
// Android is handled in the kernel
static void *FenceGet(void *context, int fd)
{
   NXPL_HWData *data = static_cast<NXPL_HWData*>(context);

   std::unique_lock<std::mutex> lock(data->fenceLock);

   auto it = data->fenceMap.find(fd);
   if (it != data->fenceMap.end())
      return static_cast<helper::Semaphore *>(it->second);
   else
      return NULL;
}
#endif

// Performance counters
static void GetPerfNumCounterGroups(void *context,
      void *session __attribute__((unused)), uint32_t *numGroups)
{
   NXPL_HWData *ctx = static_cast<NXPL_HWData*>(context);

   if (numGroups != NULL)
      NEXUS_Graphicsv3d_GetPerfNumCounterGroups(ctx->nexusHandle, numGroups);
}

static bool GetPerfCounterGroupInfo(void *context,
      void *session __attribute__((unused)), uint32_t group,
      struct bcm_sched_counter_group_desc *desc)
{
   NXPL_HWData *ctx = static_cast<NXPL_HWData*>(context);
   uint32_t numGroups;

   NEXUS_Graphicsv3d_GetPerfNumCounterGroups(ctx->nexusHandle, &numGroups);
   if (group >= numGroups)
      return false;

   /* Nexus API limits the amount of data passing through to 4k  */
   /* so group description needs to be retrieved one by one      */
   /* We rely on the fact that Nexus and our counter structures are identical */
   NEXUS_Graphicsv3d_GetPerfCounterGroupInfo(ctx->nexusHandle, group,
                                             sizeof(desc->name), desc->name,
                                             &desc->max_active_counters,
                                             &desc->total_counters);

   for (uint32_t counter_index = 0; counter_index < desc->total_counters; counter_index++)
   {
      NEXUS_Graphicsv3d_GetPerfCounterDesc(ctx->nexusHandle, group, counter_index, (NEXUS_Graphicsv3dCounterDesc *)&desc->counters[counter_index]);
   }

   return true;
}

static bool SetPerfCounting(void *context,
      void *session __attribute__((unused)), BEGL_SchedCounterState state)
{
   NXPL_HWData *ctx = static_cast<NXPL_HWData*>(context);
   NEXUS_Error err;

   err = NEXUS_Graphicsv3d_SetPerfCounting(ctx->nexusHandle, (NEXUS_Graphicsv3dCounterState)state);

   if (err == NEXUS_SUCCESS)
      return true;
   else
      return false;
}

static bool ChoosePerfCounters(void *context,
      void *session __attribute__((unused)),
      const struct bcm_sched_group_counter_selector *selector)
{
   NXPL_HWData *ctx = static_cast<NXPL_HWData*>(context);
   uint32_t numGroups;

   NEXUS_Graphicsv3d_GetPerfNumCounterGroups(ctx->nexusHandle, &numGroups);
   if (selector->group_index >= numGroups)
      return false;

   NEXUS_Graphicsv3d_ChoosePerfCounters(ctx->nexusHandle, (NEXUS_Graphicsv3dCounterSelector *)selector);

   return true;
}

static uint32_t GetPerfCounterData(void *context,
      void *session __attribute__((unused)), struct bcm_sched_counter *counters,
      uint32_t max_counters, uint32_t reset_counts)
{
   NXPL_HWData *ctx = static_cast<NXPL_HWData*>(context);
   uint32_t cnt = 0;

   /* We rely on the fact that Nexus and our counter structures are identical */
   NEXUS_Graphicsv3d_GetPerfCounterData(ctx->nexusHandle, max_counters, reset_counts, &cnt, (NEXUS_Graphicsv3dCounter*)counters);

   return cnt;
}

// Event timeline
static void GetEventCounts(void *context, void *session __attribute__((unused)),
      uint32_t *numTracks, uint32_t *numEvents)
{
   NXPL_HWData *ctx = static_cast<NXPL_HWData*>(context);

   if (numTracks != NULL && numEvents != NULL)
   {
      *numTracks = *numEvents = 0;

      NEXUS_Graphicsv3d_GetEventCounts(ctx->nexusHandle, numTracks, numEvents);

      // We will need these at various points
      ctx->eventContext->nexusTracks = *numTracks;
      ctx->eventContext->nexusEvents = *numEvents;

      // Append any tracks and events from this platform layer
      PerfAdjustEventCounts(ctx->eventContext, numTracks, numEvents);
   }
}

static bool GetEventTrackInfo(void *context,
      void *session __attribute__((unused)), uint32_t track,
      struct bcm_sched_event_track_desc *track_desc)
{
   NXPL_HWData *ctx = static_cast<NXPL_HWData*>(context);
   NEXUS_Error  err;

   BDBG_ASSERT(track_desc != NULL);

   if (track < ctx->eventContext->nexusTracks)
   {
      err = NEXUS_Graphicsv3d_GetEventTrackInfo(ctx->nexusHandle, track,
                                                (NEXUS_Graphicsv3dEventTrackDesc *)track_desc);
      return err == NEXUS_SUCCESS;
   }
   else
   {
      return PerfGetEventTrackInfo(ctx->eventContext, track, track_desc);
   }

   return false;
}

static bool GetEventInfo(void *context,
      void *session __attribute__((unused)), uint32_t event,
      struct bcm_sched_event_desc *event_desc)
{
   NXPL_HWData *ctx = static_cast<NXPL_HWData*>(context);

   if (event < ctx->eventContext->nexusEvents)
   {
       NEXUS_Error err = NEXUS_Graphicsv3d_GetEventInfo(ctx->nexusHandle, event,
                                                        (NEXUS_Graphicsv3dEventDesc *)event_desc);
      return err == NEXUS_SUCCESS;
   }
   else
   {
      return PerfGetEventInfo(ctx->eventContext, event, event_desc);
   }

   return false;
}

static bool GetEventDataFieldInfo(void *context,
      void *session __attribute__((unused)), uint32_t event, uint32_t field,
      struct bcm_sched_event_field_desc *field_desc)
{
   NXPL_HWData *ctx = static_cast<NXPL_HWData*>(context);

   if (event < ctx->eventContext->nexusEvents)
   {
      NEXUS_Error err = NEXUS_Graphicsv3d_GetEventDataFieldInfo(ctx->nexusHandle, event, field,
                                                    (NEXUS_Graphicsv3dEventFieldDesc *)field_desc);
      return err == NEXUS_SUCCESS;
   }
   else
   {
      return PerfGetEventDataFieldInfo(ctx->eventContext, event, field, field_desc);
   }

   return false;
}

static bool SetEventCollection(void *context,
      void *session __attribute__((unused)), BEGL_SchedEventState state)
{
   NXPL_HWData *ctx = static_cast<NXPL_HWData*>(context);

   bool status;

   NEXUS_Error err = NEXUS_Graphicsv3d_SetEventCollection(ctx->nexusHandle, (NEXUS_Graphicsv3dEventState)state);
   status = err == NEXUS_SUCCESS;

   if (status)
   {
      if (state == BEGL_EventStart)
      {
         // Synchronize timestamps before we start
         uint64_t timeNow;
         uint32_t bytesCopied = 0, lostData = 0;
         NEXUS_Graphicsv3d_GetEventData(ctx->nexusHandle, 0, 0, &lostData, &timeNow, &bytesCopied);
         ctx->eventContext->timeSync = timeNow;  // This will be used in PerfSetEventCollection below
      }

      status = PerfSetEventCollection(ctx->eventContext, state);
   }

   return status;
}

static uint32_t GetEventData(void *context,
      void *session __attribute__((unused)), uint32_t event_buffer_bytes,
      void *event_buffer, uint32_t *lost_data, uint64_t *timestamp)
{
   NXPL_HWData *ctx = static_cast<NXPL_HWData*>(context);
   uint32_t       bytesCopied = 0;

   NEXUS_Graphicsv3d_GetEventData(ctx->nexusHandle, event_buffer_bytes, event_buffer, lost_data, timestamp,
                                  &bytesCopied);

   void *buffer = event_buffer;
   if (event_buffer != NULL)
      buffer = (void*)((uint8_t*)event_buffer + bytesCopied);

   uint32_t display_lost_data = 0;
   uint32_t displayBytes = PerfGetEventData(ctx->eventContext, event_buffer_bytes - bytesCopied,
                                             buffer, &display_lost_data, timestamp);
   if (display_lost_data)
      *lost_data = 1;

   bytesCopied += displayBytes;

   return bytesCopied;
}

typedef struct NXPL_HWInterface : BEGL_HWInterface
{
   NXPL_HWData data;
} NXPL_HWInterface;

extern "C" BEGL_HWInterface *NXPL_CreateHWInterface(BEGL_HardwareCallbacks *callbacks, EventContext *eventContext)
{
   static_assert(NEXUS_GRAPHICSV3D_JOB_MAX_INSTRUCTIONS == BEGL_HW_JOB_MAX_INSTRUCTIONS, "missmatch between BEGL enums and NEXUS");

   auto hw = std::unique_ptr<NXPL_HWInterface>(new NXPL_HWInterface());
   if (!hw)
      return NULL;

   hw->context                = &hw->data;

   hw->GetInfo                = GetInfo;
   hw->SendJob                = SendJob;
   hw->GetNotification        = GetNotification;
   hw->SendSync               = SendSync;
   hw->GetBinMemory           = GetBinMemory;
   hw->FenceOpen              = FenceOpen;
#ifdef ANDROID
   hw->FenceWaitAsync         = FenceWaitAsync;
#else
   hw->FenceSignal            = FenceSignal;
   hw->FenceGet               = FenceGet;
#endif

   // Performance counters
   hw->perf_count_iface.GetPerfNumCounterGroups = GetPerfNumCounterGroups;
   hw->perf_count_iface.GetPerfCounterGroupInfo = GetPerfCounterGroupInfo;
   hw->perf_count_iface.SetPerfCounting         = SetPerfCounting;
   hw->perf_count_iface.ChoosePerfCounters      = ChoosePerfCounters;
   hw->perf_count_iface.GetPerfCounterData      = GetPerfCounterData;

   // Event timeline
   hw->event_track_iface.GetEventCounts         = GetEventCounts;
   hw->event_track_iface.GetEventTrackInfo      = GetEventTrackInfo;
   hw->event_track_iface.GetEventInfo           = GetEventInfo;
   hw->event_track_iface.GetEventDataFieldInfo  = GetEventDataFieldInfo;
   hw->event_track_iface.SetEventCollection     = SetEventCollection;
   hw->event_track_iface.GetEventData           = GetEventData;

   NEXUS_Graphicsv3dCreateSettings  settings;
   NEXUS_Graphicsv3d_GetDefaultCreateSettings(&settings);

   settings.sJobCallback.callback = JobCallbackHandler;
   settings.sJobCallback.context  = &hw->data;
   settings.sJobCallback.param    = 0;
   settings.uiClientPID           = getpid();

   hw->data.nexusHandle = NEXUS_Graphicsv3d_Create(&settings);

   hw->data.fence = 0;

   if (eventContext != NULL)
   {
      hw->data.eventContext = eventContext;
      hw->data.eventContextIsExternal = true;
   }
   else
   {
      hw->data.eventContext = (EventContext*)calloc(1, sizeof(EventContext));
      hw->data.eventContextIsExternal = false;
   }

   PerfInitialize(hw->data.eventContext); /* Initialize any performance event data */

   /* Drain the callback queue */
   BEGL_HWNotification notification = {};
   while (GetNotification(&hw->data, &notification))
      assert(notification.param == (void*)0xDEADBEEF);

   /* Now set the real driver callback */
   hw->data.callback = callbacks->JobCallback;

   return hw.release();
}

extern "C" void NXPL_DestroyHWInterface(BEGL_HWInterface *p)
{
   auto hw = std::unique_ptr<NXPL_HWInterface>(static_cast<NXPL_HWInterface *>(p));
   if (hw)
   {
      PerfTerminate(hw->data.eventContext);
      if (!hw->data.eventContextIsExternal)
         free(hw->data.eventContext);

      NEXUS_Graphicsv3d_Destroy(hw->data.nexusHandle);
   }
}
