/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#include "nexus_graphicsv3d_module.h"
#include "nexus_graphicsv3d_init.h"
#include "priv/nexus_core.h"
#if NEXUS_HAS_SAGE
#include "priv/nexus_sage_priv.h"
#endif

#include "nexus_client_resources.h"
#include "bchp_pwr.h"

BDBG_OBJECT_ID_DECLARE(NEXUS_Graphicsv3d);

BDBG_MODULE(graphicsv3d);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */

struct NEXUS_Graphicsv3d {
   NEXUS_OBJECT(NEXUS_Graphicsv3d);
   NEXUS_TaskCallbackHandle      jobHandler;
   NEXUS_Graphicsv3dTimelineData timelineForLastNotify;
   uint32_t                      clientId;
};

void
NEXUS_Graphicsv3dModule_GetDefaultSettings(
   NEXUS_Graphicsv3dModuleSettings *pSettings)
{
   BDBG_ENTER(NEXUS_Graphicsv3dModule_GetDefaultSettings);

   BDBG_ASSERT(pSettings);
   BKNI_Memset(pSettings, 0, sizeof(*pSettings));

   BDBG_LEAVE(NEXUS_Graphicsv3dModule_GetDefaultSettings);
}

NEXUS_Graphicsv3d_P_ModuleState g_NEXUS_Graphicsv3d_P_ModuleState;

/* only called from NEXUS_Graphicsv3dModule_Init */
static void graphics3d_worker_start(void)
{
   BERR_Code rc;
   NEXUS_ThreadSettings settings;
   BV3D_WorkerSettings workerSettings;
   NEXUS_Thread_GetDefaultSettings(&settings);

   /* Our worker thread needs a high priority to service interrupts */
   settings.priority = 0;

   /* must live for the lifetime of the worker (until BV3D_Close) */
   BKNI_CreateEvent(&g_NEXUS_Graphicsv3d_P_ModuleState.sync);

   workerSettings.hV3d = g_NEXUS_Graphicsv3d_P_ModuleState.v3d;
   workerSettings.hSync = g_NEXUS_Graphicsv3d_P_ModuleState.sync;

   g_NEXUS_Graphicsv3d_P_ModuleState.worker = NEXUS_Thread_Create("graphics3d_worker",
      BV3D_Worker, (void *)&workerSettings, &settings);

   rc = BKNI_WaitForEvent(g_NEXUS_Graphicsv3d_P_ModuleState.sync, BKNI_INFINITE);
   if (rc) BERR_TRACE(rc);

   return;
}

/* only called from NEXUS_Graphicsv3dModule_Uninit.  Must be called after the magnum module has gone */
static void graphics3d_worker_stop(void)
{
   NEXUS_Thread_Destroy(g_NEXUS_Graphicsv3d_P_ModuleState.worker);

   BKNI_DestroyEvent(g_NEXUS_Graphicsv3d_P_ModuleState.sync);
}

static void graphics3d_secure_toggle(bool secure)
{
#if NEXUS_HAS_SAGE
   /* if nothing more to advance, check for mode switch */
   BAVC_CoreList coreList;
   int rc = BERR_SUCCESS;

   BKNI_Memset(&coreList, 0, sizeof(coreList));
   coreList.aeCores[BAVC_CoreId_eV3D_0] = true;
   coreList.aeCores[BAVC_CoreId_eV3D_1] = true;
   if (secure)
      rc = NEXUS_Sage_AddSecureCores(&coreList, NEXUS_SageUrrType_eDisplay);
   else
      NEXUS_Sage_RemoveSecureCores(&coreList, NEXUS_SageUrrType_eDisplay);

   if (rc) BERR_TRACE(rc);
#else
   BSTD_UNUSED(secure);
#endif
}

NEXUS_ModuleHandle
NEXUS_Graphicsv3dModule_Init(
   const NEXUS_Graphicsv3dModuleSettings *pSettings)
{
   BERR_Code            err = BERR_SUCCESS;
   NEXUS_ModuleSettings moduleSettings;
   BSTD_UNUSED(pSettings);

   BDBG_ENTER(NEXUS_Graphicsv3dModule_Init);

   NEXUS_Module_GetDefaultSettings(&moduleSettings);
   moduleSettings.priority = NEXUS_ModulePriority_eHigh;
   g_NEXUS_Graphicsv3d_P_ModuleState.module = NEXUS_Module_Create("graphicsv3d", &moduleSettings);

   if (g_NEXUS_Graphicsv3d_P_ModuleState.module == NULL)
      goto error1;

#if 0
   /* this must be used in kernel mode */
   if (!NEXUS_P_CpuAccessibleHeap(pSettings->hHeap))
   {
      BDBG_LEAVE(NEXUS_Graphicsv3dModule_Init);
      return BERR_TRACE(NEXUS_INVALID_PARAMETER);
   }

   /* any addresses passed to the kernel must be checked as acessable
      otherwise the kernel silently fails */
   if (!NEXUS_P_CpuAccessibleAddress(userSettings->address))
   {
      BDBG_LEAVE(NEXUS_Graphicsv3dModule_Init);
      return BERR_TRACE(NEXUS_INVALID_PARAMETER);
   }
#endif

   g_NEXUS_Graphicsv3d_P_ModuleState.heapHandle = pSettings->hHeap;
   g_NEXUS_Graphicsv3d_P_ModuleState.heapHandleSecure = pSettings->hHeapSecure;

   {
      const char  *pcBinMemMegs        = NEXUS_GetEnv("V3D_BIN_MEM_MEGS");
      const char  *pcBinMemMegsSecure  = NEXUS_GetEnv("V3D_BIN_MEM_MEGS_SECURE");
      uint32_t    uiBinMemMegs         = pcBinMemMegs != NULL ? NEXUS_atoi(pcBinMemMegs) : 0;
      uint32_t    uiBinMemMegsSecure   = pcBinMemMegsSecure != NULL ? NEXUS_atoi(pcBinMemMegsSecure) : 0;
      const char  *pcBinMemChunkPow    = NEXUS_GetEnv("V3D_BIN_MEM_CHUNK_POW");
      uint32_t    uiBinMemChunkPow     = pcBinMemChunkPow != NULL ? NEXUS_atoi(pcBinMemChunkPow) : 0;
      const char  *pcDisableAQA        = NEXUS_GetEnv("V3D_DISABLE_AQA");  /* AQA = Adaptive QPU assignment */
      const char  *pcClockFreq         = NEXUS_GetEnv("V3D_CLOCK_FREQ");
      uint32_t    uiClockFreq          = pcClockFreq != NULL ? NEXUS_atoi(pcClockFreq) : 0; /* 0 = default for device, do not change */
      bool        bDisableAQA          = false;
      NEXUS_MemoryStatus heapStatus;
      NEXUS_MemoryStatus heapStatusSecure;
      BMMA_Heap_Handle hSecureHeapHandle = NULL;
      NEXUS_Addr hSecureOffset = 0;

      if (uiBinMemMegs <= 1)
      {
#ifdef B_REFSW_ANDROID
         BCHP_FeatureData featureData;
         BCHP_GetFeature(g_pCoreHandles->chp, BCHP_Feature_eProductId, &featureData);
         if (featureData.data.productId == 0x7445 || featureData.data.productId == 0x7252)
            uiBinMemMegs = 28;
         else
            uiBinMemMegs = 16;
#else
         uiBinMemMegs = 16;
#endif
      }

      /* Secure is only really intended for video as graphics, so go for minimum amount.
         This can be increased via the V3D_BIN_MEM_MEGS_SECURE environment variable */
      if (uiBinMemMegsSecure <= 1)
         uiBinMemMegsSecure = 2;

      BDBG_MSG(("Bin memory allocation        %dMbytes; in chunks of %dKbytes", uiBinMemMegs, (1 << uiBinMemChunkPow) * 256));
      BDBG_MSG(("Bin memory allocation secure %dMbytes; in chunks of %dKbytes", uiBinMemMegsSecure, (1 << uiBinMemChunkPow) * 256));

      if (pcDisableAQA != NULL)
      {
         if (*pcDisableAQA == '1' || *pcDisableAQA == 't' || *pcDisableAQA == 'T' || *pcDisableAQA == 'y' || *pcDisableAQA == 'Y')
         {
            bDisableAQA = true;
            BDBG_MSG(("Adapative QPU assignment disabled"));
         }
      }

      NEXUS_Heap_GetStatus(g_NEXUS_Graphicsv3d_P_ModuleState.heapHandle, &heapStatus);

      if (g_NEXUS_Graphicsv3d_P_ModuleState.heapHandleSecure)
      {
         NEXUS_Heap_GetStatus(g_NEXUS_Graphicsv3d_P_ModuleState.heapHandleSecure, &heapStatusSecure);
         hSecureOffset = heapStatusSecure.offset;
         hSecureHeapHandle = NEXUS_Heap_GetMmaHandle(g_NEXUS_Graphicsv3d_P_ModuleState.heapHandleSecure);
      }

      err = BV3D_Open(&g_NEXUS_Graphicsv3d_P_ModuleState.v3d,
                      g_pCoreHandles->chp,
                      g_pCoreHandles->reg,
                      NEXUS_Heap_GetMmaHandle(g_NEXUS_Graphicsv3d_P_ModuleState.heapHandle),
                      heapStatus.offset,
                      hSecureHeapHandle,
                      hSecureOffset,
                      g_pCoreHandles->bint,
                      uiBinMemMegs,
                      uiBinMemChunkPow,
                      uiBinMemMegsSecure,
                      graphics3d_secure_toggle,
                      bDisableAQA,
                      uiClockFreq);
   }

   if (err != BERR_SUCCESS)
      goto error1;

   graphics3d_worker_start();

   BDBG_LEAVE(NEXUS_Graphicsv3dModule_Init);

   return g_NEXUS_Graphicsv3d_P_ModuleState.module;

error1:
   NEXUS_Module_Destroy(g_NEXUS_Graphicsv3d_P_ModuleState.module);

   BDBG_LEAVE(NEXUS_Graphicsv3dModule_Init);

   return NULL;
}

void
NEXUS_Graphicsv3dModule_Uninit(void)
{
   BDBG_ENTER(NEXUS_Graphicsv3dModule_Uninit);

   BDBG_ASSERT(g_NEXUS_Graphicsv3d_P_ModuleState.module);

   (void)BV3D_Close(g_NEXUS_Graphicsv3d_P_ModuleState.v3d);

   graphics3d_worker_stop();

   NEXUS_Module_Destroy(g_NEXUS_Graphicsv3d_P_ModuleState.module);
   g_NEXUS_Graphicsv3d_P_ModuleState.module = NULL;

   BDBG_LEAVE(NEXUS_Graphicsv3dModule_Uninit);
}

void
NEXUS_Graphicsv3d_GetDefaultCreateSettings(
   NEXUS_Graphicsv3dCreateSettings *pSettings)
{
   BDBG_ENTER(NEXUS_Graphicsv3d_GetDefaultCreateSettings);

   BDBG_ASSERT(pSettings);
   BKNI_Memset(pSettings, 0, sizeof(*pSettings));

   NEXUS_CallbackDesc_Init(&pSettings->sJobCallback);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetDefaultCreateSettings);
}

static void
JobHandler(
   uint32_t clientId,
   void     *context)
{
   NEXUS_Graphicsv3dHandle gfx = (NEXUS_Graphicsv3dHandle)context;

   BSTD_UNUSED(clientId);
   BDBG_MSG(("Job callback handler, issuing task callback"));

   NEXUS_TaskCallback_Fire(gfx->jobHandler);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Graphicsv3d, NEXUS_Graphicsv3d_Destroy);

NEXUS_Graphicsv3dHandle
NEXUS_Graphicsv3d_Create(
   const NEXUS_Graphicsv3dCreateSettings *pSettings)
{
   BERR_Code               berr;
   int                     rc;
   NEXUS_Graphicsv3dHandle gfx;

   BDBG_ENTER(NEXUS_Graphicsv3d_Create);

   BDBG_ASSERT(pSettings);

   rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(graphicsv3d,Count,NEXUS_ANY_ID);
   if (rc) { rc = BERR_TRACE(rc); return NULL; }

   gfx = BKNI_Malloc(sizeof(*gfx));

   if (!gfx)
   {
      NEXUS_CLIENT_RESOURCES_RELEASE(graphicsv3d,Count,NEXUS_ANY_ID);
      BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
      BDBG_LEAVE(NEXUS_Graphicsv3d_Create);
      return NULL;
   }

   NEXUS_OBJECT_INIT(NEXUS_Graphicsv3d, gfx);

   gfx->jobHandler = NEXUS_TaskCallback_Create(gfx, NULL);

   if (gfx->jobHandler == NULL)
   {
      BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
      goto error;
   }

   /* Connect up callback with NEXUS */
   if (pSettings->sJobCallback.callback)
      NEXUS_TaskCallback_Set(gfx->jobHandler, &pSettings->sJobCallback);

   /* Register client and callbacks for this client */
   berr = BV3D_RegisterClient(g_NEXUS_Graphicsv3d_P_ModuleState.v3d, &gfx->clientId, gfx, JobHandler, pSettings->uiClientPID);

   if (berr != BERR_SUCCESS)
   {
      BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
      goto error;
   }

   BDBG_LEAVE(NEXUS_Graphicsv3d_Create);
   return gfx;

error:
   NEXUS_Graphicsv3d_Destroy(gfx);
   BDBG_LEAVE(NEXUS_Graphicsv3d_Create);
   return NULL;
}

static void
NEXUS_Graphicsv3d_P_Finalizer(
   NEXUS_Graphicsv3dHandle gfx)
{
   if (gfx->clientId != 0)
      BV3D_UnregisterClient(g_NEXUS_Graphicsv3d_P_ModuleState.v3d, gfx->clientId);

   NEXUS_CLIENT_RESOURCES_RELEASE(graphicsv3d,Count,NEXUS_ANY_ID);
   NEXUS_TaskCallback_Destroy(gfx->jobHandler);

   BKNI_Free(gfx);
}

/**
External API
**/
NEXUS_Error
NEXUS_Graphicsv3d_GetInfo(
   NEXUS_Graphicsv3dInfo *info)
{
   BV3D_Info   bv3dInfo;
   NEXUS_Error err = NEXUS_UNKNOWN;

   BDBG_ENTER(NEXUS_Graphicsv3d_GetInfo);

   if (info != NULL && BV3D_GetInfo(g_NEXUS_Graphicsv3d_P_ModuleState.v3d, &bv3dInfo) == BERR_SUCCESS)
   {
      BKNI_Memcpy(info->chName,   bv3dInfo.chName,   10);
      BKNI_Memcpy(info->chRevStr, bv3dInfo.chRevStr, 3 );

      info->uiNumSlices            = bv3dInfo.uiNumSlices;
      info->uiTextureUnitsPerSlice = bv3dInfo.uiTextureUnitsPerSlice;
      info->uiTechRev              = bv3dInfo.uiTechRev;
      info->uiRevision             = bv3dInfo.uiRevision;

      err = NEXUS_SUCCESS;
   }

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetInfo);
   return err;
}

NEXUS_Error
NEXUS_Graphicsv3d_SendJob(
   NEXUS_Graphicsv3dHandle    gfx,
   const NEXUS_Graphicsv3dJob *job)
{
   BV3D_Job    bv3dJob;
   BERR_Code   err;
   uint32_t    i;

   BDBG_ENTER(NEXUS_Graphicsv3d_SendJob);

   BDBG_ASSERT(job != 0);

   BKNI_Memset(&bv3dJob, 0, sizeof(BV3D_Job));

   for (i = 0; i < NEXUS_GRAPHICSV3D_JOB_MAX_INSTRUCTIONS; ++i)
   {
      bv3dJob.sProgram[i].eOperation      = job->sProgram[i].eOperation;
      bv3dJob.sProgram[i].uiArg1          = job->sProgram[i].uiArg1;
      bv3dJob.sProgram[i].uiArg2          = job->sProgram[i].uiArg2;
      bv3dJob.sProgram[i].uiCallbackParam = job->sProgram[i].uiCallbackParam;
   }

   bv3dJob.uiBinMemory      = job->uiBinMemory;
   bv3dJob.bBinMemorySecure = job->bBinMemorySecure;
   bv3dJob.uiUserVPM        = job->uiUserVPM;
   bv3dJob.bCollectTimeline = job->bCollectTimeline;
   bv3dJob.uiClientId       = gfx->clientId;
   bv3dJob.uiSequence       = job->uiJobSequence;

   err = BV3D_SendJob(g_NEXUS_Graphicsv3d_P_ModuleState.v3d, &bv3dJob);

   BDBG_LEAVE(NEXUS_Graphicsv3d_SendJob);

   /* TODO proper error code for failure */
   return err == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}


NEXUS_Error
NEXUS_Graphicsv3d_GetNotification(
   NEXUS_Graphicsv3dHandle gfx,
   NEXUS_Graphicsv3dNotification *notification)
{
   BV3D_Notification bv3dNotification;
   BV3D_TimelineData bv3dTimelineData;
   BERR_Code         err;

   BDBG_ENTER(NEXUS_Graphicsv3d_GetNotification);

   BDBG_ASSERT(notification != 0);

   bv3dNotification.pTimelineData = &bv3dTimelineData;

   err = BV3D_GetNotification(g_NEXUS_Graphicsv3d_P_ModuleState.v3d, gfx->clientId, &bv3dNotification);

   if (err == BERR_SUCCESS)
   {
      notification->uiParam       = bv3dNotification.uiParam;
      notification->uiSync        = bv3dNotification.uiSync;
      notification->uiOutOfMemory = bv3dNotification.uiOutOfMemory;
      notification->uiJobSequence = bv3dNotification.uiJobSequence;

      gfx->timelineForLastNotify.sBinStart.uiSecs         = bv3dTimelineData.sBinStart.uiSecs;
      gfx->timelineForLastNotify.sBinStart.uiMicrosecs    = bv3dTimelineData.sBinStart.uiMicrosecs;
      gfx->timelineForLastNotify.sBinEnd.uiSecs           = bv3dTimelineData.sBinEnd.uiSecs;
      gfx->timelineForLastNotify.sBinEnd.uiMicrosecs      = bv3dTimelineData.sBinEnd.uiMicrosecs;
      gfx->timelineForLastNotify.sRenderStart.uiSecs      = bv3dTimelineData.sRenderStart.uiSecs;
      gfx->timelineForLastNotify.sRenderStart.uiMicrosecs = bv3dTimelineData.sRenderStart.uiMicrosecs;
      gfx->timelineForLastNotify.sRenderEnd.uiSecs        = bv3dTimelineData.sRenderEnd.uiSecs;
      gfx->timelineForLastNotify.sRenderEnd.uiMicrosecs   = bv3dTimelineData.sRenderEnd.uiMicrosecs;
      gfx->timelineForLastNotify.sUserStart.uiSecs        = bv3dTimelineData.sUserStart.uiSecs;
      gfx->timelineForLastNotify.sUserStart.uiMicrosecs   = bv3dTimelineData.sUserStart.uiMicrosecs;
      gfx->timelineForLastNotify.sUserEnd.uiSecs          = bv3dTimelineData.sUserEnd.uiSecs;
      gfx->timelineForLastNotify.sUserEnd.uiMicrosecs     = bv3dTimelineData.sUserEnd.uiMicrosecs;
   }

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetNotification);

   return err == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error
NEXUS_Graphicsv3d_GetTimelineForLastNotification(
   NEXUS_Graphicsv3dHandle       gfx,
   NEXUS_Graphicsv3dTimelineData *timelineData)
{
   BDBG_ENTER(NEXUS_Graphicsv3d_GetTimelineForLastNotification);

   BDBG_ASSERT(timelineData != NULL);

   timelineData->sBinStart.uiSecs         = gfx->timelineForLastNotify.sBinStart.uiSecs;
   timelineData->sBinStart.uiMicrosecs    = gfx->timelineForLastNotify.sBinStart.uiMicrosecs;
   timelineData->sBinEnd.uiSecs           = gfx->timelineForLastNotify.sBinEnd.uiSecs;
   timelineData->sBinEnd.uiMicrosecs      = gfx->timelineForLastNotify.sBinEnd.uiMicrosecs;
   timelineData->sRenderStart.uiSecs      = gfx->timelineForLastNotify.sRenderStart.uiSecs;
   timelineData->sRenderStart.uiMicrosecs = gfx->timelineForLastNotify.sRenderStart.uiMicrosecs;
   timelineData->sRenderEnd.uiSecs        = gfx->timelineForLastNotify.sRenderEnd.uiSecs;
   timelineData->sRenderEnd.uiMicrosecs   = gfx->timelineForLastNotify.sRenderEnd.uiMicrosecs;
   timelineData->sUserStart.uiSecs        = gfx->timelineForLastNotify.sUserStart.uiSecs;
   timelineData->sUserStart.uiMicrosecs   = gfx->timelineForLastNotify.sUserStart.uiMicrosecs;
   timelineData->sUserEnd.uiSecs          = gfx->timelineForLastNotify.sUserEnd.uiSecs;
   timelineData->sUserEnd.uiMicrosecs     = gfx->timelineForLastNotify.sUserEnd.uiMicrosecs;

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetTimelineForLastNotification);

   return NEXUS_SUCCESS;
}

void
NEXUS_Graphicsv3d_SendSync(
   NEXUS_Graphicsv3dHandle gfx,
   bool abandon)
{
   BDBG_ENTER(NEXUS_Graphicsv3d_SendSync);

   BV3D_SendSync(g_NEXUS_Graphicsv3d_P_ModuleState.v3d, gfx->clientId, abandon);

   BDBG_LEAVE(NEXUS_Graphicsv3d_SendSync);
}

NEXUS_Error
NEXUS_Graphicsv3d_GetBinMemory(
   NEXUS_Graphicsv3dHandle                   gfx,
   const NEXUS_Graphicsv3dBinMemorySettings  *settings,
   NEXUS_Graphicsv3dBinMemory                *memory)
{
   BERR_Code               err;
   BV3D_BinMemorySettings  bv3dSettings;
   BV3D_BinMemory          bv3dMemory;

   BSTD_UNUSED(gfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_GetBinMemory);

   BDBG_ASSERT(settings != NULL);
   BDBG_ASSERT(memory   != NULL);

   bv3dSettings.bSecure = settings->bSecure;

   err = BV3D_GetBinMemory(g_NEXUS_Graphicsv3d_P_ModuleState.v3d, &bv3dSettings, gfx->clientId, &bv3dMemory);

   memory->uiAddress = bv3dMemory.uiAddress;
   memory->uiSize    = bv3dMemory.uiSize;

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetBinMemory);

   return err == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

void
NEXUS_Graphicsv3d_SetPerformanceMonitor(
   NEXUS_Graphicsv3dHandle                    gfx,
   const NEXUS_Graphicsv3dPerfMonitorSettings *settings)
{
   BV3D_PerfMonitorSettings   bSettings;

   BSTD_UNUSED(gfx);

   if (settings == NULL)
      return;

   bSettings.uiHWBank  = settings->uiHwBank;
   bSettings.uiMemBank = settings->uiMemBank;
   bSettings.uiFlags   = settings->uiFlags;

   BV3D_SetPerformanceMonitor(g_NEXUS_Graphicsv3d_P_ModuleState.v3d, &bSettings);
}

void
NEXUS_Graphicsv3d_GetPerformanceData(
   NEXUS_Graphicsv3dHandle          gfx,
   NEXUS_Graphicsv3dPerfMonitorData *data)
{
   BV3D_PerfMonitorData bData;
   uint32_t             i;

   BSTD_UNUSED(gfx);

   if (data == NULL)
      return;

   BV3D_GetPerformanceData(g_NEXUS_Graphicsv3d_P_ModuleState.v3d, &bData);

   for (i = 0; i < 16; i++)
      data->uiHwCounters[i] = bData.uiHwCounters[i];

   for (i = 0; i < 2; i++)
      data->uiMemCounters[i] = bData.uiMemCounters[i];
}

NEXUS_Error NEXUS_Graphicsv3d_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
   NEXUS_Error rc = NEXUS_SUCCESS;

   BSTD_UNUSED(pSettings);

   if (enabled)
      rc = BV3D_Standby(g_NEXUS_Graphicsv3d_P_ModuleState.v3d);
   else
      rc = BV3D_Resume(g_NEXUS_Graphicsv3d_P_ModuleState.v3d);

   return rc;
#else
   BSTD_UNUSED(enabled);
   BSTD_UNUSED(pSettings);

   return NEXUS_SUCCESS;
#endif
}

void NEXUS_Graphicsv3d_SetGatherLoadData(
   bool bCollect
   )
{
   BV3D_SetGatherLoadData(g_NEXUS_Graphicsv3d_P_ModuleState.v3d, bCollect);
}

NEXUS_Error NEXUS_Graphicsv3d_GetLoadData(
   NEXUS_Graphicsv3dClientLoadData *pLoadData,     /* [out] attr{nelem=uiNumClients;nelem_out=pValidClients} */
   uint32_t                         uiNumClients,
   uint32_t                         *pValidClients
   )
{
   BERR_Code            err = BERR_SUCCESS;
   BV3D_ClientLoadData  *data = NULL;

   BDBG_ASSERT(pValidClients != NULL);

   if (pLoadData == NULL)
   {
      /* How many clients are there? */
      err = BV3D_GetLoadData(g_NEXUS_Graphicsv3d_P_ModuleState.v3d, NULL, 0, pValidClients);
   }
   else if (uiNumClients > 0)
   {
      data = BKNI_Malloc(uiNumClients * sizeof(BV3D_ClientLoadData));
      if (data == NULL)
         return NEXUS_OUT_OF_SYSTEM_MEMORY;

      err = BV3D_GetLoadData(g_NEXUS_Graphicsv3d_P_ModuleState.v3d, data, uiNumClients, pValidClients);

      if (err == BERR_SUCCESS)
      {
         uint32_t i;
         for (i = 0; i < *pValidClients; i++)
         {
            pLoadData[i].uiClientId = data[i].uiClientId;
            pLoadData[i].uiClientPID = data[i].uiClientPID;
            pLoadData[i].uiNumRenders = data[i].uiNumRenders;
            pLoadData[i].sRenderPercent = data[i].sRenderPercent;
         }
      }

      BKNI_Free(data);
   }

   return err == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_FenceOpen(
   NEXUS_Graphicsv3dHandle          gfx,
   int *fd,
   uint64_t *p,
   char cType,
   int iPid
   )
{
   NEXUS_Error ret;

   BDBG_ENTER(NEXUS_Graphicsv3d_FenceOpen);

   if (fd != NULL)
   {
      BERR_Code err = BV3D_FenceOpen(g_NEXUS_Graphicsv3d_P_ModuleState.v3d, gfx->clientId, fd, (void**)p, cType, iPid);
      ret = err == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
   }
   else
      ret = NEXUS_INVALID_PARAMETER;

   BDBG_LEAVE(NEXUS_Graphicsv3d_FenceOpen);

   return ret;
}

NEXUS_Error NEXUS_Graphicsv3d_FenceWaitAsync(
   NEXUS_Graphicsv3dHandle          gfx,
   int fd,
   uint64_t *pV3dFence
   )
{
   NEXUS_Error ret;
   BERR_Code err;

   BDBG_ENTER(NEXUS_Graphicsv3d_FenceWaitAsync);

   if (pV3dFence == NULL)
   {
      ret = NEXUS_INVALID_PARAMETER;
      goto error;
   }

   err = BV3D_FenceWaitAsync(g_NEXUS_Graphicsv3d_P_ModuleState.v3d, gfx->clientId, fd, (void **)pV3dFence);
   if (err == BERR_SUCCESS)
      ret = NEXUS_SUCCESS;
   else
      ret = NEXUS_UNKNOWN;

error:
   BDBG_LEAVE(NEXUS_Graphicsv3d_FenceWaitAsync);

   return ret;
}

void NEXUS_Graphicsv3d_GetTime(uint64_t *pMicroseconds)
{
   BV3D_GetTime(pMicroseconds);
}

NEXUS_Error NEXUS_Graphicsv3d_SetFrequencyScaling(unsigned percent)
{
#if NEXUS_POWER_MANAGEMENT && BCHP_PWR_RESOURCE_GRAPHICS3D
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned clkRate;

    if(percent > 100) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    rc = BCHP_PWR_GetMaxClockRate(g_NEXUS_pCoreHandles->chp, BCHP_PWR_RESOURCE_GRAPHICS3D, &clkRate);
    if(rc) {return BERR_TRACE(NEXUS_INVALID_PARAMETER);}
    clkRate = percent*(clkRate/100);
    rc = BCHP_PWR_SetClockRate(g_NEXUS_pCoreHandles->chp, BCHP_PWR_RESOURCE_GRAPHICS3D, clkRate);
    if(rc) {return BERR_TRACE(NEXUS_INVALID_PARAMETER);}

    return rc;
#else
    BSTD_UNUSED(percent);
    return NEXUS_NOT_SUPPORTED;
#endif
}
