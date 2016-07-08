/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION
DESC
=============================================================================*/

#include "nexus_platform.h"
#include "nexus_graphicsv3d.h"

#include "bkni_multi.h"

#include <EGL/egl.h>

#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

#if NEXUS_GRAPHICSV3D_JOB_MAX_INSTRUCTIONS != BEGL_HW_JOB_MAX_INSTRUCTIONS
#error "V3D & Nexus mismatch : NEXUS_GRAPHICSV3D_JOB_MAX_INSTRUCTIONS != BEGL_HW_JOB_MAX_INSTRUCTIONS"
#endif

#define UNUSED(X) X

typedef void (*CallbackFunc)(void);

typedef struct
{
   NEXUS_Graphicsv3dHandle nexusHandle;
   CallbackFunc            callback;
} NXPL_HWData;

/*****************************************************************************
 * H/W driver interface
 *****************************************************************************/

static void JobCallbackHandler(void *context, int param)
{
   NXPL_HWData *data = (NXPL_HWData*)context;
   UNUSED(param);

   if (data && data->callback)
      data->callback();
}

/* Get hardware info */
static bool GetInfo(void *context, BEGL_HWInfo *info)
{
   NEXUS_Graphicsv3dInfo  nInfo;
   NEXUS_Error            err;
   UNUSED(context);

   err = NEXUS_Graphicsv3d_GetInfo(&nInfo);

   if (err == NEXUS_SUCCESS)
   {
      info->numSlices   = nInfo.uiNumSlices;
      info->revision    = nInfo.uiRevision;
      info->techRev     = nInfo.uiTechRev;
      info->textureUnitsPerSlice = nInfo.uiTextureUnitsPerSlice;
      strncpy(info->name, nInfo.chName, (sizeof(info->name) - 1));
      strncpy(info->revStr, nInfo.chRevStr, (sizeof(info->revStr) - 1));
      NEXUS_Graphicsv3d_GetTime(&info->time);
      return true;
   }

   return false;
}

/* Send a job to the V3D job queue */
bool SendJob(void *context, BEGL_HWJob *job)
{
   NXPL_HWData             *data = (NXPL_HWData*)context;
   NEXUS_Error             err;
   NEXUS_Graphicsv3dJob    nJob;
   uint32_t                i;

   if (data != NULL)
   {
      for (i = 0; i < NEXUS_GRAPHICSV3D_JOB_MAX_INSTRUCTIONS; i++)
      {
         nJob.sProgram[i].eOperation      = job->program[i].operation;    /* We are assuming the enum values are the same */
         nJob.sProgram[i].uiArg1          = job->program[i].arg1;
         nJob.sProgram[i].uiArg2          = job->program[i].arg2;
         nJob.sProgram[i].uiCallbackParam = job->program[i].callbackParam;
      }

      nJob.uiBinMemory      = job->binMemory;
      nJob.bBinMemorySecure = job->binMemorySecure;
      nJob.uiUserVPM        = job->userVPM;
      nJob.bCollectTimeline = job->collectTimeline;
      nJob.uiJobSequence    = job->jobSequence;

      err = NEXUS_Graphicsv3d_SendJob(data->nexusHandle, &nJob);

      return err == NEXUS_SUCCESS;
   }

   return false;
}

/* Retrieves and removes latest notification for this client */
bool GetNotification(void *context, BEGL_HWNotification *notification)
{
   NXPL_HWData                   *data = (NXPL_HWData*)context;
   NEXUS_Graphicsv3dNotification nNot;
   NEXUS_Graphicsv3dTimelineData nTimeline;
   NEXUS_Error                   err;

   if (data != NULL)
   {
      err = NEXUS_Graphicsv3d_GetNotification(data->nexusHandle, &nNot);

      if (err == NEXUS_SUCCESS)
      {
         notification->needsSync   = nNot.uiSync;
         notification->param       = (void *)nNot.uiParam;
         notification->outOfMemory = nNot.uiOutOfMemory;
         notification->jobSequence = nNot.uiJobSequence;

         if (notification->timelineData != NULL)
         {
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
   NXPL_HWData *data = (NXPL_HWData*)context;

   if (data != NULL)
      NEXUS_Graphicsv3d_SendSync(data->nexusHandle, abandon);
}

/* Request bin memory */
static bool GetBinMemory(void *context, const BEGL_HWBinMemorySettings *settings, BEGL_HWBinMemory *memory)
{
   NXPL_HWData                         *data = (NXPL_HWData*)context;
   NEXUS_Graphicsv3dBinMemory          nexusMemory;
   NEXUS_Graphicsv3dBinMemorySettings  nexusMemorySettings;
   NEXUS_Error                         rc;

   if (settings == NULL || memory == NULL)
      return false;

   nexusMemorySettings.bSecure = settings->secure;

   rc = NEXUS_Graphicsv3d_GetBinMemory(data->nexusHandle, &nexusMemorySettings, &nexusMemory);

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
   NXPL_HWData                            *data = (NXPL_HWData*)context;
   NEXUS_Graphicsv3dPerfMonitorSettings   nSettings;

   if (settings == NULL)
      return;

   nSettings.uiHwBank  = settings->hwBank;
   nSettings.uiMemBank = settings->memBank;
   nSettings.uiFlags   = settings->flags;

   NEXUS_Graphicsv3d_SetPerformanceMonitor(data->nexusHandle, &nSettings);
}

/* Get performance data */
static void GetPerformanceData(void *context, BEGL_HWPerfMonitorData *bData)
{
   NXPL_HWData                      *data = (NXPL_HWData*)context;
   NEXUS_Graphicsv3dPerfMonitorData nData;
   uint32_t                         i;

   if (bData == NULL)
      return;

   NEXUS_Graphicsv3d_GetPerformanceData(data->nexusHandle, &nData);

   for (i = 0; i < 16; i++)
      bData->hwCounters[i] = nData.uiHwCounters[i];

   for (i = 0; i < 2; i++)
      bData->memCounters[i] = nData.uiMemCounters[i];
}

static void FenceOpen(void *context, int *fd)
{
   NXPL_HWData                            *data = (NXPL_HWData*)context;

   if (data != NULL)
      NEXUS_Graphicsv3d_FenceOpen(data->nexusHandle, fd);
}

static void FenceSignal(int fd)
{
   NEXUS_Graphicsv3d_FenceSignal(fd);
}

static void FenceClose(int fd)
{
   NEXUS_Graphicsv3d_FenceClose(fd);
}

__attribute__((visibility("default")))
BEGL_HWInterface *NXPL_CreateHWInterface(BEGL_HardwareCallbacks *callbacks)
{
   BEGL_HWInterface *hw = (BEGL_HWInterface*)malloc(sizeof(BEGL_HWInterface));
   if (hw != NULL)
   {
      NXPL_HWData *data = (NXPL_HWData*)malloc(sizeof(NXPL_HWData));
      memset(hw, 0, sizeof(BEGL_HWInterface));

      if (data != NULL)
      {
         NEXUS_Graphicsv3dCreateSettings  settings;
         BEGL_HWNotification            notification;

         memset(data, 0, sizeof(NXPL_HWData));

         hw->context = (void*)data;

         hw->GetInfo                = GetInfo;
         hw->SendJob                = SendJob;
         hw->GetNotification        = GetNotification;
         hw->SendSync               = SendSync;
         hw->GetBinMemory           = GetBinMemory;
         hw->SetPerformanceMonitor  = SetPerformanceMonitor;
         hw->GetPerformanceData     = GetPerformanceData;
         hw->FenceOpen              = FenceOpen;
         hw->FenceSignal            = FenceSignal;
         hw->FenceClose             = FenceClose;

         NEXUS_Graphicsv3d_GetDefaultCreateSettings(&settings);

         settings.sJobCallback.callback = JobCallbackHandler;
         settings.sJobCallback.context  = data;
         settings.sJobCallback.param    = 0;
         settings.uiClientPID           = getpid();

         data->nexusHandle = NEXUS_Graphicsv3d_Create(&settings);

         /* Drain the callback queue */
         memset(&notification, 0, sizeof(BEGL_HWNotification));

         while (GetNotification(data, &notification))
            assert(notification.param == (void*)0xDEADBEEF);

         /* Now set the real driver callback */
         data->callback = callbacks->JobCallback;
      }
   }
   return hw;
}

__attribute__((visibility("default")))
void NXPL_DestroyHWInterface(BEGL_HWInterface *hw)
{
   if (hw != NULL)
   {
      NXPL_HWData *data = (NXPL_HWData*)hw->context;

      if (data != NULL)
      {
         NEXUS_Graphicsv3d_Destroy(data->nexusHandle);
         free(hw->context);
      }

      memset(hw, 0, sizeof(BEGL_HWInterface));
      free(hw);
   }
}
