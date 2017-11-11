/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "nexus_platform.h"
#include "nexus_graphicsv3d.h"

#include "bkni_multi.h"

#include <EGL/egl.h>

#include <cassert>
#include <cstdio>

#include <memory>

/* #include <cutils/log.h> */

#include "../helpers/semaphore.h"

typedef void (*CallbackFunc)(void);

typedef struct
{
   NEXUS_Graphicsv3dHandle nexusHandle;
   CallbackFunc            callback;
} NXPL_HWData;

/*****************************************************************************
 * H/W driver interface
 *****************************************************************************/

static void JobCallbackHandler(void *context, int param __attribute__((unused)))
{
   NXPL_HWData *data = reinterpret_cast<NXPL_HWData*>(context);

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
   NXPL_HWData *data = reinterpret_cast<NXPL_HWData*>(context);

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
   NXPL_HWData *data = reinterpret_cast<NXPL_HWData*>(context);

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
   NXPL_HWData *data = reinterpret_cast<NXPL_HWData*>(context);

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

   NXPL_HWData *data = reinterpret_cast<NXPL_HWData*>(context);
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

/* Setup or change performance monitoring */
static void SetPerformanceMonitor(void *context, const BEGL_HWPerfMonitorSettings *settings)
{
   if (settings == NULL)
      return;

   NEXUS_Graphicsv3dPerfMonitorSettings nSettings;
   nSettings.uiHwBank  = settings->hwBank;
   nSettings.uiMemBank = settings->memBank;
   nSettings.uiFlags   = settings->flags;

   NXPL_HWData *data = reinterpret_cast<NXPL_HWData*>(context);
   NEXUS_Graphicsv3d_SetPerformanceMonitor(data->nexusHandle, &nSettings);
}

/* Get performance data */
static void GetPerformanceData(void *context, BEGL_HWPerfMonitorData *bData)
{
   if (bData == NULL)
      return;

   NEXUS_Graphicsv3dPerfMonitorData nData;
   NXPL_HWData *data = reinterpret_cast<NXPL_HWData*>(context);
   NEXUS_Graphicsv3d_GetPerformanceData(data->nexusHandle, &nData);

   for (int i = 0; i < 16; i++)
      bData->hwCounters[i] = nData.uiHwCounters[i];

   for (int i = 0; i < 2; i++)
      bData->memCounters[i] = nData.uiMemCounters[i];
}

#ifdef ANDROID
static void FenceOpen(void *context, int *fd, void **p, char type)
{
   NXPL_HWData *data = (NXPL_HWData*)context;

   if (data != NULL)
      NEXUS_Graphicsv3d_FenceOpen(data->nexusHandle, fd, p, type, getpid());
}

static void FenceWaitAsync(void *context, int fd, void **v3dfence)
{
   NXPL_HWData *data = (NXPL_HWData*)context;

   if (data != NULL)
      NEXUS_Graphicsv3d_FenceWaitAsync(data->nexusHandle, fd, v3dfence);
}
#else
static void FenceOpen(void *context __attribute__((unused)), int *fd, void **p, char type __attribute__((unused)))
{
   auto m = new helper::Semaphore();
   *fd = reinterpret_cast<int>(m);
   *p = m;
}

// only required on platforms with a consumer thread/managing own display.
// Android is handled in the kernel
static void FenceSignal(void *context __attribute__((unused)), int fd)
{
   auto m = reinterpret_cast<helper::Semaphore *>(fd);
   m->notify();
}
#endif

typedef struct NXPL_HWInterface : BEGL_HWInterface
{
   NXPL_HWData data;
} NXPL_HWInterface;

extern "C" BEGL_HWInterface *NXPL_CreateHWInterface(BEGL_HardwareCallbacks *callbacks)
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
   hw->SetPerformanceMonitor  = SetPerformanceMonitor;
   hw->GetPerformanceData     = GetPerformanceData;
   hw->FenceOpen              = FenceOpen;
#ifdef ANDROID
   hw->FenceWaitAsync         = FenceWaitAsync;
#else
   hw->FenceSignal            = FenceSignal;
#endif

   NEXUS_Graphicsv3dCreateSettings  settings;
   NEXUS_Graphicsv3d_GetDefaultCreateSettings(&settings);

   settings.sJobCallback.callback = JobCallbackHandler;
   settings.sJobCallback.context  = &hw->data;
   settings.sJobCallback.param    = 0;
   settings.uiClientPID           = getpid();

   hw->data.nexusHandle = NEXUS_Graphicsv3d_Create(&settings);

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
   auto hw = std::unique_ptr<NXPL_HWInterface>(reinterpret_cast<NXPL_HWInterface *>(p));
   if (hw)
      NEXUS_Graphicsv3d_Destroy(hw->data.nexusHandle);
}
