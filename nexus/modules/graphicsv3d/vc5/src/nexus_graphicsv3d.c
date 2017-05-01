/******************************************************************************
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
 ******************************************************************************/
#include "nexus_graphicsv3d_module.h"
#include "nexus_graphicsv3d_init.h"
#include "priv/nexus_core.h"
#if NEXUS_HAS_SAGE
#include "priv/nexus_sage_priv.h"
#endif

#include "nexus_client_resources.h"

#include "blst_queue.h"

BDBG_OBJECT_ID_DECLARE(NEXUS_Graphicsv3d);

BDBG_MODULE(graphicsvc5);

/* Static checks */
#define CASSERT(EXPR, MSG) typedef int assertion_failed_##MSG[(EXPR) ? 1 : -1]

/* Static sanity checks on defines */
#define CHECK_DEFINE(NEXUS_NAME, MAGNUM_NAME) \
   CASSERT(NEXUS_NAME == MAGNUM_NAME, NEXUS_NAME##_not_equal_##MAGNUM_NAME)

CHECK_DEFINE(NEXUS_GRAPHICSV3D_MAX_BIN_SUBJOBS,    BVC5_MAX_BIN_SUBJOBS);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_MAX_RENDER_SUBJOBS, BVC5_MAX_RENDER_SUBJOBS);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_MAX_QPU_SUBJOBS,    BVC5_MAX_QPU_SUBJOBS);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_MAX_DEPENDENCIES,   BVC5_MAX_DEPENDENCIES);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_MAX_CORES,          BVC5_MAX_CORES);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_MAX_IDENTS,         BVC5_MAX_IDENTS);

CHECK_DEFINE(NEXUS_GRAPHICSV3D_MAX_GROUP_NAME_LEN,          BVC5_MAX_GROUP_NAME_LEN);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_MAX_COUNTER_NAME_LEN,        BVC5_MAX_COUNTER_NAME_LEN);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_MAX_COUNTER_UNIT_NAME_LEN,   BVC5_MAX_COUNTER_UNIT_NAME_LEN);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_MAX_COUNTERS_PER_GROUP,      BVC5_MAX_COUNTERS_PER_GROUP);

CHECK_DEFINE(NEXUS_GRAPHICSV3D_EMPTY_TILE_MODE_NONE,        BVC5_EMPTY_TILE_MODE_NONE);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_EMPTY_TILE_MODE_SKIP,        BVC5_EMPTY_TILE_MODE_SKIP);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_EMPTY_TILE_MODE_FILL,        BVC5_EMPTY_TILE_MODE_FILL);

CHECK_DEFINE(NEXUS_GRAPHICSV3D_NO_BIN_RENDER_OVERLAP,       BVC5_NO_BIN_RENDER_OVERLAP);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_GFXH_1181,                   BVC5_GFXH_1181);

/* V3D cache operations */
CHECK_DEFINE(NEXUS_GRAPHICSV3D_CACHE_CLEAR_SIC,   BVC5_CACHE_CLEAR_SIC);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_CACHE_CLEAR_SUC,   BVC5_CACHE_CLEAR_SUC);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_CACHE_CLEAR_L1TD,  BVC5_CACHE_CLEAR_L1TD);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_CACHE_CLEAR_L1TC,  BVC5_CACHE_CLEAR_L1TC);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_CACHE_CLEAR_VCD,   BVC5_CACHE_CLEAR_VCD);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_CACHE_CLEAR_L2C,   BVC5_CACHE_CLEAR_L2C);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_CACHE_FLUSH_L2T,   BVC5_CACHE_FLUSH_L2T);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_CACHE_CLEAN_L1TD,  BVC5_CACHE_CLEAN_L1TD);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_CACHE_CLEAN_L2T,   BVC5_CACHE_CLEAN_L2T);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_CACHE_CLEAR_GCA,   BVC5_CACHE_CLEAR_GCA);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_CACHE_FLUSH_L3C,   BVC5_CACHE_FLUSH_L3C);
CHECK_DEFINE(NEXUS_GRAPHICSV3D_CACHE_CLEAN_L3C,   BVC5_CACHE_CLEAN_L3C);

/* Static sanity checks on enums */
#define CHECK_ENUM(NEXUS_NAME, MAGNUM_NAME) \
   CASSERT((int)NEXUS_NAME == (int)MAGNUM_NAME, NEXUS_NAME##_not_equal_##MAGNUM_NAME)

CHECK_ENUM(NEXUS_Graphicsv3dJobType_eNull,         BVC5_JobType_eNull);
CHECK_ENUM(NEXUS_Graphicsv3dJobType_eBin,          BVC5_JobType_eBin);
CHECK_ENUM(NEXUS_Graphicsv3dJobType_eRender,       BVC5_JobType_eRender);
CHECK_ENUM(NEXUS_Graphicsv3dJobType_eUser,         BVC5_JobType_eUser);
CHECK_ENUM(NEXUS_Graphicsv3dJobType_eTFU,          BVC5_JobType_eTFU);
CHECK_ENUM(NEXUS_Graphicsv3dJobType_eFenceWait,    BVC5_JobType_eFenceWait);
CHECK_ENUM(NEXUS_Graphicsv3dJobType_eTest,         BVC5_JobType_eTest);
CHECK_ENUM(NEXUS_Graphicsv3dJobType_eUsermode,     BVC5_JobType_eUsermode);
CHECK_ENUM(NEXUS_Graphicsv3dJobType_eBarrier,      BVC5_JobType_eBarrier);
CHECK_ENUM(NEXUS_Graphicsv3dJobType_eNumJobTypes,  BVC5_JobType_eNumJobTypes);

CHECK_ENUM(NEXUS_Graphicsv3dCtrAcquire,            BVC5_CtrAcquire);
CHECK_ENUM(NEXUS_Graphicsv3dCtrRelease,            BVC5_CtrRelease);
CHECK_ENUM(NEXUS_Graphicsv3dCtrStart,              BVC5_CtrStart);
CHECK_ENUM(NEXUS_Graphicsv3dCtrStop,               BVC5_CtrStop);

CHECK_ENUM(NEXUS_Graphicsv3dEventAcquire,          BVC5_EventAcquire);
CHECK_ENUM(NEXUS_Graphicsv3dEventRelease,          BVC5_EventRelease);
CHECK_ENUM(NEXUS_Graphicsv3dEventStart,            BVC5_EventStart);
CHECK_ENUM(NEXUS_Graphicsv3dEventStop,             BVC5_EventStop);

CHECK_ENUM(NEXUS_Graphicsv3dEventBegin,            BVC5_EventBegin);
CHECK_ENUM(NEXUS_Graphicsv3dEventEnd,              BVC5_EventEnd);
CHECK_ENUM(NEXUS_Graphicsv3dEventOneshot,          BVC5_EventOneshot);

CHECK_ENUM(NEXUS_Graphicsv3dEventInt32,            BVC5_EventInt32);
CHECK_ENUM(NEXUS_Graphicsv3dEventUInt32,           BVC5_EventUInt32);
CHECK_ENUM(NEXUS_Graphicsv3dEventInt64,            BVC5_EventInt64);
CHECK_ENUM(NEXUS_Graphicsv3dEventUInt64,           BVC5_EventUInt64);

CHECK_ENUM(NEXUS_Graphicsv3dJobStatus_eSUCCESS,       BVC5_JobStatus_eSUCCESS);
CHECK_ENUM(NEXUS_Graphicsv3dJobStatus_eOUT_OF_MEMORY, BVC5_JobStatus_eOUT_OF_MEMORY);
CHECK_ENUM(NEXUS_Graphicsv3dJobStatus_eERROR,         BVC5_JobStatus_eERROR);

/* Static sanity checks on structures */
#define CHECK_STRUCT(NEXUS_NAME, MAGNUM_NAME) \
   CASSERT(sizeof(NEXUS_NAME) == sizeof(MAGNUM_NAME), mismatched_sizes_##NEXUS_NAME##_and_##MAGNUM_NAME)

CHECK_STRUCT(NEXUS_Graphicsv3dInfo,                BVC5_Info);
CHECK_STRUCT(NEXUS_Graphicsv3dSchedDependencies,   BVC5_SchedDependencies);
CHECK_STRUCT(NEXUS_Graphicsv3dQueryResponse,       BVC5_QueryResponse);
CHECK_STRUCT(NEXUS_Graphicsv3dJobBase,             BVC5_JobBase);
CHECK_STRUCT(NEXUS_Graphicsv3dJobNull,             BVC5_JobNull);
CHECK_STRUCT(NEXUS_Graphicsv3dJobBin,              BVC5_JobBin);
CHECK_STRUCT(NEXUS_Graphicsv3dJobRender,           BVC5_JobRender);
CHECK_STRUCT(NEXUS_Graphicsv3dJobUser,             BVC5_JobUser);
CHECK_STRUCT(NEXUS_Graphicsv3dJobFenceWait,        BVC5_JobFenceWait);
CHECK_STRUCT(NEXUS_Graphicsv3dJobTFU,              BVC5_JobTFU);
CHECK_STRUCT(NEXUS_Graphicsv3dJobTest,             BVC5_JobTest);
CHECK_STRUCT(NEXUS_Graphicsv3dJobUsermode,         BVC5_JobUsermode);

CHECK_STRUCT(NEXUS_Graphicsv3dUsermode,            BVC5_Usermode);
CHECK_STRUCT(NEXUS_Graphicsv3dCompletion,          BVC5_Completion);
CHECK_STRUCT(NEXUS_Graphicsv3dCompletionInfo,      BVC5_CompletionInfo);

CHECK_STRUCT(NEXUS_Graphicsv3dCounterDesc,         BVC5_CounterDesc);
CHECK_STRUCT(NEXUS_Graphicsv3dCounterGroupDesc,    BVC5_CounterGroupDesc);
CHECK_STRUCT(NEXUS_Graphicsv3dCounterSelector,     BVC5_CounterSelector);
CHECK_STRUCT(NEXUS_Graphicsv3dCounter,             BVC5_Counter);

CHECK_STRUCT(NEXUS_Graphicsv3dEventDesc,           BVC5_EventDesc);
CHECK_STRUCT(NEXUS_Graphicsv3dEventFieldDesc,      BVC5_EventFieldDesc);
CHECK_STRUCT(NEXUS_Graphicsv3dEventTrackDesc,      BVC5_EventTrackDesc);

static void NEXUS_Graphicsv3d_P_FenceHandler(void *pVc5, uint64_t uiEvent);
static void NEXUS_Graphicsv3d_P_UsermodeHandler(void *pVc5);
static void NEXUS_Graphicsv3d_P_CompletionHandler(void *pVc5);
static void NEXUS_Graphicsv3d_P_SecureToggleHandler(bool bEnter);

/* Event Queue for Fence implementation */
typedef struct NEXUS_P_EventQueue
{
   uint64_t                            uiEvent;
   BLST_Q_ENTRY(NEXUS_P_EventQueue)    sChain;
} NEXUS_P_EventQueue;

typedef struct NEXUS_P_TSEventQueue
{
   BKNI_MutexHandle                        hMutex;
   BLST_Q_HEAD(sQueue, NEXUS_P_EventQueue) sQueue;
} NEXUS_P_TSEventQueue;

/* NEXUS_Graphicsv3d_P_TSEventQueueConstruct
 *
 * Initialize a thread safe queue of events
 */
static bool NEXUS_Graphicsv3d_P_TSEventQueueConstruct(
   NEXUS_P_TSEventQueue  *tsQ)
{
   if (tsQ == NULL)
      return false;

   tsQ->hMutex = NULL;

   /* Must test return code to appease coverity */
   if (BKNI_CreateMutex(&tsQ->hMutex) != BERR_SUCCESS || tsQ->hMutex == NULL)
      return false;

   BLST_Q_INIT(&tsQ->sQueue);
   return true;
}

/* NEXUS_Graphicsv3d_P_TSEventQueuePush
 *
 * Push an event into a thread-safe queue of events
 */
static void NEXUS_Graphicsv3d_P_TSEventQueuePush(
   NEXUS_P_TSEventQueue                *tsQ,
   uint64_t                             uiEvent)
{
   NEXUS_P_EventQueue   *entry;

   if (tsQ == NULL)
      return;

   BKNI_AcquireMutex(tsQ->hMutex);

   entry = (NEXUS_P_EventQueue *)BKNI_Malloc(sizeof(NEXUS_P_EventQueue));

   if (entry != NULL)
   {
      entry->uiEvent = uiEvent;
      BLST_Q_INSERT_TAIL(&tsQ->sQueue, entry, sChain);
   }

   BKNI_ReleaseMutex(tsQ->hMutex);
}

/* NEXUS_Graphicsv3d_P_TSEventQueuePop
 *
 * Pop an event from a thread-safe queue of events
 */
static uint64_t NEXUS_Graphicsv3d_P_TSEventQueuePop(
   NEXUS_P_TSEventQueue *tsQ)
{
   uint64_t   uiEvent = 0;

   BKNI_AcquireMutex(tsQ->hMutex);

   if (!BLST_Q_EMPTY(&tsQ->sQueue))
   {
      NEXUS_P_EventQueue *entry = BLST_Q_FIRST(&tsQ->sQueue);

      if (entry != NULL)
      {
         uiEvent = entry->uiEvent;
         BLST_Q_REMOVE_HEAD(&tsQ->sQueue, sChain);
         BKNI_Free(entry);
      }
   }

   BKNI_ReleaseMutex(tsQ->hMutex);

   return uiEvent;
}

/* NEXUS_Graphicsv3d_P_TSEventQueueDestruct
 *
 * Destroy a thread-safe queue of events
 */
static void NEXUS_Graphicsv3d_P_TSEventQueueDestruct(
   NEXUS_P_TSEventQueue *tsQ)
{
   if (tsQ == NULL)
      return;

   if (tsQ->hMutex != NULL)
   {
      NEXUS_P_EventQueue *entry;

      BKNI_DestroyMutex(tsQ->hMutex);
      tsQ->hMutex = NULL;

      entry = BLST_Q_FIRST(&tsQ->sQueue);

      while (entry != NULL)
      {
         BLST_Q_REMOVE_HEAD(&tsQ->sQueue, sChain);
         BKNI_Free(entry);
         entry = BLST_Q_FIRST(&tsQ->sQueue);
      }
   }
}

/* MODULE IMPLEMENTATION */

struct NEXUS_Graphicsv3d {
   NEXUS_OBJECT(NEXUS_Graphicsv3d);

   uint32_t                   uiClientId;
   NEXUS_TaskCallbackHandle   hUsermodeCallback;
   NEXUS_TaskCallbackHandle   hFenceDoneCallback;
   NEXUS_TaskCallbackHandle   hCompletionCallback;

   NEXUS_P_TSEventQueue       sFenceEventQueue;
};

NEXUS_Graphicsv3d_P_ModuleState g_NEXUS_Graphicsv3d_P_ModuleState;

/* Only called from NEXUS_Graphicsv3dModule_Init
   The magnum module must be open.
 */
static void NEXUS_Graphicsv3d_P_SchedulerStart(void)
{
   BVC5_SchedulerSettings  schedulerSettings;
   NEXUS_ThreadSettings    threadSettings;
   BERR_Code               rc;

   NEXUS_Thread_GetDefaultSettings(&threadSettings);

   /* Our worker thread needs a high priority to service interrupts */
   threadSettings.priority = 0;

   /* must live for the lifetime of the worker (until BVC5_Close) */
   BKNI_CreateEvent(&g_NEXUS_Graphicsv3d_P_ModuleState.hSchedulerSyncEvent);

   schedulerSettings.hVC5       = g_NEXUS_Graphicsv3d_P_ModuleState.hVc5;
   schedulerSettings.hSyncEvent = g_NEXUS_Graphicsv3d_P_ModuleState.hSchedulerSyncEvent;

   g_NEXUS_Graphicsv3d_P_ModuleState.hSchedulerThread =
      NEXUS_Thread_Create("graphicsv3d_worker", BVC5_Scheduler, (void *)&schedulerSettings, &threadSettings);

   /* Wait for the thread to start up */
   rc = BKNI_WaitForEvent(g_NEXUS_Graphicsv3d_P_ModuleState.hSchedulerSyncEvent, BKNI_INFINITE);
   if (rc) BERR_TRACE(rc);

   return;
}

/* Only called from NEXUS_GraphicsVC5Module_Uninit.
   Must only be called after the magnum module has gone
 */
static void NEXUS_Graphicsv3d_P_SchedulerStop(
   void)
{
   if (g_NEXUS_Graphicsv3d_P_ModuleState.hSchedulerThread != NULL)
      NEXUS_Thread_Destroy(g_NEXUS_Graphicsv3d_P_ModuleState.hSchedulerThread);

   if (g_NEXUS_Graphicsv3d_P_ModuleState.hSchedulerSyncEvent != NULL)
      BKNI_DestroyEvent(g_NEXUS_Graphicsv3d_P_ModuleState.hSchedulerSyncEvent);
}

void NEXUS_Graphicsv3dModule_GetDefaultSettings(
   NEXUS_Graphicsv3dModuleSettings *pSettings)
{
   BDBG_ENTER(NEXUS_Graphicsv3dModule_GetDefaultSettings);

   BDBG_ASSERT(pSettings);
   BKNI_Memset(pSettings, 0, sizeof(*pSettings));

   BDBG_LEAVE(NEXUS_Graphicsv3dModule_GetDefaultSettings);
}

NEXUS_ModuleHandle NEXUS_Graphicsv3dModule_Init(
   const NEXUS_Graphicsv3dModuleSettings *pSettings)
{
   BERR_Code            err = BERR_SUCCESS;
   NEXUS_Error          rc = NEXUS_SUCCESS;
   NEXUS_ModuleSettings moduleSettings;
   const char           *pcEnv;
   BVC5_OpenParameters  openParams;
   BVC5_Callbacks       callbacks;
   BMMA_Heap_Handle     heapSecureMma = NULL;
   NEXUS_MemoryStatus   memoryStatus;

   BSTD_UNUSED(pSettings);

   BDBG_ENTER(NEXUS_Graphicsv3dModule_Init);

   NEXUS_Module_GetDefaultSettings(&moduleSettings);
   moduleSettings.priority = NEXUS_ModulePriority_eHigh;
   g_NEXUS_Graphicsv3d_P_ModuleState.hModule = NEXUS_Module_Create("graphicsvc5", &moduleSettings);

   if (g_NEXUS_Graphicsv3d_P_ModuleState.hModule == NULL)
      goto error1;

   BVC5_GetDefaultOpenParameters(&openParams);

   pcEnv = NEXUS_GetEnv("V3D_USE_POWER_GATING");
   openParams.bUsePowerGating = pcEnv != NULL ? (NEXUS_atoi(pcEnv) == 0 ? false : true) : openParams.bUsePowerGating;

   pcEnv = NEXUS_GetEnv("V3D_USE_CLOCK_GATING");
   openParams.bUseClockGating = pcEnv != NULL ? (NEXUS_atoi(pcEnv) == 0 ? false : true) : openParams.bUseClockGating;

   pcEnv = NEXUS_GetEnv("V3D_USE_STALL_DETECTION");
   openParams.bUseStallDetection = pcEnv != NULL ? (NEXUS_atoi(pcEnv) == 0 ? false : true) : openParams.bUseStallDetection;

   pcEnv = NEXUS_GetEnv("V3D_GPUMON_DEPS");
   openParams.bGPUMonDeps = pcEnv != NULL ? (NEXUS_atoi(pcEnv) == 0 ? false : true) : openParams.bGPUMonDeps;

   pcEnv = NEXUS_GetEnv("V3D_DRIVER_NO_QUEUEING");
   openParams.bNoQueueAhead = pcEnv != NULL ? (NEXUS_atoi(pcEnv) == 0 ? false : true) : openParams.bNoQueueAhead;

   pcEnv = NEXUS_GetEnv("V3D_RESET_ON_STALL");
   openParams.bResetOnStall = pcEnv != NULL ? (NEXUS_atoi(pcEnv) == 0 ? false : true) : openParams.bResetOnStall;

   pcEnv = NEXUS_GetEnv("V3D_MEM_DUMP_ON_STALL");
   openParams.bMemDumpOnStall = pcEnv != NULL ? (NEXUS_atoi(pcEnv) == 0 ? false : true) : openParams.bMemDumpOnStall;

   pcEnv = NEXUS_GetEnv("V3D_NO_BURST_SPLITTING");
   openParams.bNoBurstSplitting = pcEnv != NULL ? (NEXUS_atoi(pcEnv) == 0 ? false : true) : openParams.bNoBurstSplitting;

   /*
    * These options must be the same EV names as those used by memory_drm.c
    * in the nxpl library to initialize the client's DRM usage.
    */

   pcEnv = NEXUS_GetEnv("V3D_DRM_DEVICE_NUM");
   openParams.uiDRMDevice = pcEnv != NULL ? (uint32_t)NEXUS_atoi(pcEnv) : openParams.uiDRMDevice;

   callbacks.fpUsermodeHandler     = NEXUS_Graphicsv3d_P_UsermodeHandler;
   callbacks.fpCompletionHandler   = NEXUS_Graphicsv3d_P_CompletionHandler;
   callbacks.fpSecureToggleHandler = NEXUS_Graphicsv3d_P_SecureToggleHandler;

   g_NEXUS_Graphicsv3d_P_ModuleState.hHeap       = pSettings->hHeap;
   g_NEXUS_Graphicsv3d_P_ModuleState.hHeapSecure = pSettings->hHeapSecure;

   if (pSettings->hHeapSecure)
      heapSecureMma = NEXUS_Heap_GetMmaHandle(g_NEXUS_Graphicsv3d_P_ModuleState.hHeapSecure);

   rc = NEXUS_Heap_GetStatus_driver(pSettings->hHeap, &memoryStatus);
   if (rc != NEXUS_SUCCESS)
      goto error1;

   err = BVC5_Open(&g_NEXUS_Graphicsv3d_P_ModuleState.hVc5,
                   g_pCoreHandles->chp,
                   g_pCoreHandles->reg,
                   NEXUS_Heap_GetMmaHandle(g_NEXUS_Graphicsv3d_P_ModuleState.hHeap),
                   heapSecureMma,
                   memoryStatus.offset, /* debug only */
                   memoryStatus.size,   /* debug only */
                   g_pCoreHandles->bint,
                   &openParams,
                   &callbacks);

   if (err != BERR_SUCCESS)
      goto error1;

   NEXUS_Graphicsv3d_P_SchedulerStart();

   BDBG_LEAVE(NEXUS_Graphicsv3dModule_Init);

   return g_NEXUS_Graphicsv3d_P_ModuleState.hModule;

error1:
   if (g_NEXUS_Graphicsv3d_P_ModuleState.hModule != NULL)
      NEXUS_Graphicsv3dModule_Uninit();

   BDBG_LEAVE(NEXUS_Graphicsv3dModule_Init);

   return NULL;
}

void NEXUS_Graphicsv3dModule_Uninit(void)
{
   BDBG_ENTER(NEXUS_Graphicsv3dModule_Uninit);

   BDBG_ASSERT(g_NEXUS_Graphicsv3d_P_ModuleState.hModule);

   if (g_NEXUS_Graphicsv3d_P_ModuleState.hVc5 != NULL)
      (void)BVC5_Close(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5);

   NEXUS_Graphicsv3d_P_SchedulerStop();

   NEXUS_Module_Destroy(g_NEXUS_Graphicsv3d_P_ModuleState.hModule);

   g_NEXUS_Graphicsv3d_P_ModuleState.hModule = NULL;

   BDBG_LEAVE(NEXUS_Graphicsv3dModule_Uninit);
}

void NEXUS_Graphicsv3d_GetDefaultCreateSettings(
   NEXUS_Graphicsv3dCreateSettings *pSettings)
{
   BDBG_ENTER(NEXUS_Graphicsv3d_GetDefaultCreateSettings);

   BDBG_ASSERT(pSettings);
   BKNI_Memset(pSettings, 0, sizeof(*pSettings));

   NEXUS_CallbackDesc_Init(&pSettings->sUsermode);
   NEXUS_CallbackDesc_Init(&pSettings->sFenceDone);
   NEXUS_CallbackDesc_Init(&pSettings->sCompletion);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetDefaultCreateSettings);
}

/* Automatic implementation using finalizer */
NEXUS_OBJECT_CLASS_MAKE(NEXUS_Graphicsv3d, NEXUS_Graphicsv3d_Destroy);

NEXUS_Graphicsv3dHandle NEXUS_Graphicsv3d_Create(
   const NEXUS_Graphicsv3dCreateSettings *pSettings)
{
   BERR_Code                        berr = NEXUS_OUT_OF_SYSTEM_MEMORY;
   int                              rc;
   NEXUS_Graphicsv3dHandle          gfx = NULL;
   NEXUS_Graphicsv3dCreateSettings  default_settings;

   BDBG_ENTER(NEXUS_Graphicsv3d_Create);

   BDBG_ASSERT(pSettings);

   rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(graphicsv3d, Count, NEXUS_ANY_ID);

   if (rc != 0)
   {
      rc = BERR_TRACE(rc);
      return NULL;
   }

   if (pSettings == NULL)
   {
      NEXUS_Graphicsv3d_GetDefaultCreateSettings(&default_settings);
      pSettings = &default_settings;
   }

   gfx = BKNI_Malloc(sizeof(*gfx));

   if (gfx == NULL)
      goto exit;

   NEXUS_OBJECT_INIT(NEXUS_Graphicsv3d, gfx);

   /* Create queue for fence callbacks */
   if (!NEXUS_Graphicsv3d_P_TSEventQueueConstruct(&gfx->sFenceEventQueue))
      goto exit;

   /**********************************************/
   /* Create callback for usermode callback jobs */
   gfx->hUsermodeCallback = NEXUS_TaskCallback_Create(gfx, NULL);

   if (gfx->hUsermodeCallback == NULL)
   {
      BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
      goto exit;
   }

   /* Connect callback if supplied */
   if (pSettings->sUsermode.callback)
      NEXUS_TaskCallback_Set(gfx->hUsermodeCallback, &pSettings->sUsermode);

   /**********************************************/
   /* Create callback for fences */
   gfx->hFenceDoneCallback = NEXUS_TaskCallback_Create(gfx, NULL);

   if (gfx->hFenceDoneCallback == NULL)
   {
      BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
      goto exit;
   }

   /* Connect callback if supplied */
   if (pSettings->sFenceDone.callback)
      NEXUS_TaskCallback_Set(gfx->hFenceDoneCallback, &pSettings->sFenceDone);

   /***********************************************/
   /* Create callback for completion notification */
   gfx->hCompletionCallback = NEXUS_TaskCallback_Create(gfx, NULL);

   if (gfx->hCompletionCallback == NULL)
   {
      BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
      goto exit;
   }

   /* Connect callback if supplied */
   if (pSettings->sCompletion.callback)
      NEXUS_TaskCallback_Set(gfx->hCompletionCallback, &pSettings->sCompletion);


   /* Register this client with Magnum */
   berr = BVC5_RegisterClient(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, gfx, &gfx->uiClientId, pSettings->iUnsecureBinTranslation, pSettings->iSecureBinTranslation, pSettings->uiPlatformToken);

   if (berr != BERR_SUCCESS)
      goto exit;

   berr = BERR_SUCCESS;

exit:
   if (berr != BERR_SUCCESS)
   {
      BERR_TRACE(berr);
      NEXUS_Graphicsv3d_Destroy(gfx);
      gfx = NULL;
   }

   BDBG_LEAVE(NEXUS_Graphicsv3d_Create);

   return gfx;
}

static void NEXUS_Graphicsv3d_P_Finalizer(
   NEXUS_Graphicsv3dHandle gfx)
{
   if (gfx == NULL)
      return;

   if (gfx->uiClientId != 0)
      BVC5_UnregisterClient(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, gfx->uiClientId);

   if (gfx->hUsermodeCallback != NULL)
      NEXUS_TaskCallback_Destroy(gfx->hUsermodeCallback);

   if (gfx->hFenceDoneCallback != NULL)
      NEXUS_TaskCallback_Destroy(gfx->hFenceDoneCallback);

   NEXUS_Graphicsv3d_P_TSEventQueueDestruct(&gfx->sFenceEventQueue);

   if (gfx->hCompletionCallback != NULL)
      NEXUS_TaskCallback_Destroy(gfx->hCompletionCallback);

   NEXUS_CLIENT_RESOURCES_RELEASE(graphicsv3d, Count, NEXUS_ANY_ID);

   BKNI_Free(gfx);
}

void NEXUS_Graphicsv3d_GetInfo(
   NEXUS_Graphicsv3dInfo *info)
{
   BDBG_ENTER(NEXUS_Graphicsv3d_GetInfo);

   if (info != NULL)
      BVC5_GetInfo(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, (BVC5_Info *)info);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetInfo);
}

NEXUS_Error NEXUS_Graphicsv3d_Standby_priv(
   bool enabled,
   const NEXUS_StandbySettings *pSettings
)
{
#if NEXUS_POWER_MANAGEMENT
   NEXUS_Error rc = NEXUS_SUCCESS;

   BSTD_UNUSED(pSettings);

   if (enabled)
      rc = BVC5_Standby(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5);
   else
      rc = BVC5_Resume(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5);

   return rc;
#else
   BSTD_UNUSED(enabled);
   BSTD_UNUSED(pSettings);

   return NEXUS_SUCCESS;
#endif
}

NEXUS_Error NEXUS_Graphicsv3d_QueueBinRender(
   NEXUS_Graphicsv3dHandle              hGfx,
   const NEXUS_Graphicsv3dJobBin       *bin,
   const NEXUS_Graphicsv3dJobRender    *render
   )
{
   BERR_Code          berr = 0;

   BDBG_ENTER(NEXUS_Graphicsv3d_QueueBinRender);

   berr = BVC5_BinRenderJob(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId,
                            (const BVC5_JobBin *)bin,
                            (const BVC5_JobRender *)render);

   BDBG_LEAVE(NEXUS_Graphicsv3d_QueueBinRender);

   return berr == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_QueueBin(
   NEXUS_Graphicsv3dHandle              hGfx,
   const NEXUS_Graphicsv3dJobBin       *bin
   )
{
   BDBG_ENTER(NEXUS_Graphicsv3d_QueueBin);
   BSTD_UNUSED(hGfx);
   BSTD_UNUSED(bin);

   /* Not implemented yet */

   BDBG_LEAVE(NEXUS_Graphicsv3d_QueueBin);

   return NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_QueueRender(
   NEXUS_Graphicsv3dHandle              hGfx,
   const NEXUS_Graphicsv3dJobRender    *render
   )
{
   BERR_Code          berr = 0;

   BDBG_ENTER(NEXUS_Graphicsv3d_QueueRender);

   berr = BVC5_BinRenderJob(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5,hGfx->uiClientId,
                            NULL,
                            (const BVC5_JobRender *)render);

   BDBG_LEAVE(NEXUS_Graphicsv3d_QueueRender);

   return berr == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_QueueTFU(
   NEXUS_Graphicsv3dHandle              hGfx,
   uint32_t                             uiNumJobs,
   const NEXUS_Graphicsv3dJobTFU       *tfuJobs
)
{
   BERR_Code            berr = 0;

   BDBG_ENTER(NEXUS_Graphicsv3d_QueueTFU);

   berr = BVC5_TFUJobs(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId, uiNumJobs,
                      (const BVC5_JobTFU *)tfuJobs);

   BDBG_LEAVE(NEXUS_Graphicsv3d_QueueTFU);

   return berr ==  BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_QueueBarrier(
   NEXUS_Graphicsv3dHandle              hGfx,
   const NEXUS_Graphicsv3dJobBarrier   *barrier
)
{
   BERR_Code            berr = 0;

   BDBG_ENTER(NEXUS_Graphicsv3d_QueueBarrier);

   berr = BVC5_BarrierJob(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId,
                      (const BVC5_JobBarrier *)barrier);

   BDBG_LEAVE(NEXUS_Graphicsv3d_QueueBarrier);

   return berr ==  BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_QueueNull(
   NEXUS_Graphicsv3dHandle             hGfx,
   const NEXUS_Graphicsv3dJobNull      *null
)
{
   BERR_Code            berr = 0;

   BDBG_ENTER(NEXUS_Graphicsv3d_QueueNull);

   berr = BVC5_NullJob(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId,
                       (const BVC5_JobNull *)null);

   BDBG_LEAVE(NEXUS_Graphicsv3d_QueueNull);

   return berr ==  BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_QueueFenceWait(
   NEXUS_Graphicsv3dHandle                   hGfx,
   const NEXUS_Graphicsv3dJobFenceWait      *wait
   )
{
   BERR_Code            berr;

   BDBG_ENTER(NEXUS_Graphicsv3d_QueueFenceWait);

   berr = BVC5_FenceWaitJob(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId,
                            (const BVC5_JobFenceWait *)wait);

   BDBG_LEAVE(NEXUS_Graphicsv3d_QueueFenceWait);

   return berr ==  BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_QueueTest(
   NEXUS_Graphicsv3dHandle             hGfx,
   const NEXUS_Graphicsv3dJobTest      *test
)
{
   BERR_Code            berr = 0;

   BDBG_ENTER(NEXUS_Graphicsv3d_QueueTest);

   berr = BVC5_TestJob(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId,
                       (const BVC5_JobTest *)test);

   BDBG_LEAVE(NEXUS_Graphicsv3d_QueueTest);

   return berr ==  BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_QueueUsermode(
   NEXUS_Graphicsv3dHandle             hGfx,
   const NEXUS_Graphicsv3dJobUsermode  *usermode
)
{
   BERR_Code            berr = 0;

   BDBG_ENTER(NEXUS_Graphicsv3d_QueueUsermode);

   berr = BVC5_UsermodeJob(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId,
                           (const BVC5_JobUsermode *)usermode);

   BDBG_LEAVE(NEXUS_Graphicsv3d_QueueUsermode);

   return berr ==  BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_Query(
   NEXUS_Graphicsv3dHandle                   hGfx,
   const NEXUS_Graphicsv3dSchedDependencies *pCompletedDeps,
   const NEXUS_Graphicsv3dSchedDependencies *pFinalizedDeps,
   NEXUS_Graphicsv3dQueryResponse           *pResponse
   )
{
   BERR_Code          berr;

   BDBG_ENTER(NEXUS_Graphicsv3d_Query);

   BKNI_Memset(pResponse, 0, sizeof(NEXUS_Graphicsv3dQueryResponse));

   berr = BVC5_Query(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId,
                     (BVC5_SchedDependencies *)pCompletedDeps,
                     (BVC5_SchedDependencies *)pFinalizedDeps,
                     (BVC5_QueryResponse *)pResponse);

   BDBG_LEAVE(NEXUS_Graphicsv3d_Query);

   return berr ==  BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_MakeFenceForJobs(
   NEXUS_Graphicsv3dHandle                   hGfx,
   const NEXUS_Graphicsv3dSchedDependencies *pCompletedDeps,
   const NEXUS_Graphicsv3dSchedDependencies *pFinalizedDeps,
   bool                                      bForceCreate,
   int                                      *piFence
   )
{
   BERR_Code berr;

   BDBG_ENTER(NEXUS_Graphicsv3d_MakeFenceForJobs);

   berr = BVC5_MakeFenceForJobs(
      g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId,
      (const BVC5_SchedDependencies *)pCompletedDeps,
      (const BVC5_SchedDependencies *)pFinalizedDeps,
      bForceCreate,
      piFence);

   BDBG_LEAVE(NEXUS_Graphicsv3d_MakeFenceForJobs);

   return berr == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_MakeFenceForAnyNonFinalizedJob(
   NEXUS_Graphicsv3dHandle hGfx,
   int                    *piFence
   )
{
   BERR_Code berr;

   BDBG_ENTER(NEXUS_Graphicsv3d_MakeFenceForAnyNonFinalizedJob);

   berr = BVC5_MakeFenceForAnyNonFinalizedJob(
      g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId,
      piFence);

   BDBG_LEAVE(NEXUS_Graphicsv3d_MakeFenceForAnyNonFinalizedJob);

   return berr == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_MakeFenceForAnyJob(
   NEXUS_Graphicsv3dHandle                   hGfx,
   const NEXUS_Graphicsv3dSchedDependencies *pCompletedDeps,
   const NEXUS_Graphicsv3dSchedDependencies *pFinalizedDeps,
   int                                      *piFence
   )
{
   BERR_Code berr;

   BDBG_ENTER(NEXUS_Graphicsv3d_MakeFenceForAnyJob);

   berr = BVC5_MakeFenceForAnyJob(
      g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId,
      (const BVC5_SchedDependencies *)pCompletedDeps,
      (const BVC5_SchedDependencies *)pFinalizedDeps,
      piFence);

   BDBG_LEAVE(NEXUS_Graphicsv3d_MakeFenceForAnyJob);

   return berr == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

static void NEXUS_Graphicsv3d_P_FenceHandler(
   void  *pVc5,
   uint64_t uiEvent
   )
{
   NEXUS_Graphicsv3dHandle  hGfx    = (NEXUS_Graphicsv3dHandle)pVc5;
   NEXUS_TaskCallbackHandle handler = hGfx->hFenceDoneCallback;

   BDBG_MSG(("%s: V3d handle %p, Event " BDBG_UINT64_FMT, __FUNCTION__, (void*)hGfx, BDBG_UINT64_ARG(uiEvent)));

   NEXUS_Graphicsv3d_P_TSEventQueuePush(&hGfx->sFenceEventQueue, uiEvent);

   NEXUS_TaskCallback_Fire(handler);
}

NEXUS_Error NEXUS_Graphicsv3d_GetPendingFenceEvent(
   NEXUS_Graphicsv3dHandle              hGfx,
   uint64_t                            *puiEvent)
{
   BDBG_ASSERT(puiEvent != NULL);

   BDBG_ENTER(NEXUS_Graphicsv3d_GetPendingFenceEvent);

   *puiEvent = NEXUS_Graphicsv3d_P_TSEventQueuePop(&hGfx->sFenceEventQueue);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetPendingFenceEvent);

   return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Graphicsv3d_FenceMake(
   NEXUS_Graphicsv3dHandle  hGfx,
   int                     *fence)
{
   BERR_Code   berr;

   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_FenceMake);

   berr = BVC5_FenceMakeLocal(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId, fence);

   BDBG_MSG(("Made fence %d", *fence));

   BDBG_LEAVE(NEXUS_Graphicsv3d_FenceMake);

   return berr ==  BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_FenceKeep(
   NEXUS_Graphicsv3dHandle  hGfx,
   int                      fence)
{
   BERR_Code   berr;

   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_FenceKeep);
   BDBG_MSG(("Keeping fence %d reference", fence));

   berr = BVC5_FenceKeep(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, fence);

   BDBG_LEAVE(NEXUS_Graphicsv3d_FenceKeep);

   return berr ==  BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_RegisterFenceWait(
   NEXUS_Graphicsv3dHandle             hGfx,
   int                                 fence,
   uint64_t                            uiEvent)
{
   BERR_Code   err;

   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_RegisterFenceWait);
   BDBG_MSG(("Register wait for %d", fence));

   /* Magnum will callback FenceHandler(handler) */
   err = BVC5_FenceRegisterWaitCallback(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, fence,
                                        hGfx->uiClientId,
                                        NEXUS_Graphicsv3d_P_FenceHandler, hGfx, uiEvent);

   BDBG_LEAVE(NEXUS_Graphicsv3d_RegisterFenceWait);

   return err ==  BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_UnregisterFenceWait(
   NEXUS_Graphicsv3dHandle           hGfx,
   int                               fence,
   uint64_t                          uiEvent,
   bool                              *signalled
   )
{
   BDBG_ENTER(NEXUS_Graphicsv3d_UnregisterFenceWait);

   BVC5_FenceUnregisterWaitCallback(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, fence,
                                    hGfx->uiClientId,
                                    NEXUS_Graphicsv3d_P_FenceHandler, hGfx, uiEvent,
                                    signalled);

   BDBG_LEAVE(NEXUS_Graphicsv3d_UnregisterFenceWait);

   return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Graphicsv3d_FenceSignal(
   NEXUS_Graphicsv3dHandle  hGfx,
   int                      fence)
{
   BERR_Code   berr;

   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_FenceSignal);
   BDBG_MSG(("Signalling fence %d", fence));

   berr = BVC5_FenceSignal(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, fence);

   BDBG_LEAVE(NEXUS_Graphicsv3d_FenceSignal);

   return berr ==  BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_Graphicsv3d_FenceClose(
   NEXUS_Graphicsv3dHandle  hGfx,
   int                      fence)
{
   BERR_Code   berr;

   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_FenceClose);
   BDBG_MSG(("Closing fence %d", fence));

   berr = BVC5_FenceClose(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, fence);

   BDBG_LEAVE(NEXUS_Graphicsv3d_FenceClose);

   return berr == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

/*************************************************************************************************/
/* USER MODE CALLBACK HANDLING                                                                   */
/*************************************************************************************************/
NEXUS_Error NEXUS_Graphicsv3d_GetUsermode(
   NEXUS_Graphicsv3dHandle       hGfx,
   uint64_t                      jobId,
   NEXUS_Graphicsv3dUsermode    *usermode)
{
   BERR_Code berr;

   BDBG_ENTER(NEXUS_Graphicsv3d_GetUsermode);

   berr =  BVC5_GetUsermode(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId, jobId, (BVC5_Usermode *)usermode);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetUsermode);

   return berr == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

static void NEXUS_Graphicsv3d_P_UsermodeHandler(void *pVc5)
{
   NEXUS_Graphicsv3dHandle  hGfx    = (NEXUS_Graphicsv3dHandle)pVc5;
   NEXUS_TaskCallbackHandle handler = hGfx->hUsermodeCallback;

   NEXUS_TaskCallback_Fire(handler);
}

/*************************************************************************************************/
/* COMPLETION CALLBACK HANDLING                                                                   */
/*************************************************************************************************/
static void NEXUS_Graphicsv3d_P_CompletionHandler(void *pVc5)
{
   NEXUS_Graphicsv3dHandle  hGfx    = (NEXUS_Graphicsv3dHandle)pVc5;
   NEXUS_TaskCallbackHandle handler = hGfx->hCompletionCallback;

   NEXUS_TaskCallback_Fire(handler);
}

NEXUS_Error NEXUS_Graphicsv3d_GetCompletions(
   NEXUS_Graphicsv3dHandle          hGfx,
   uint32_t                         uiNumFinalizedJobs,
   const uint64_t                   *puiFinalizedJobs,
   uint32_t                         uiMaxCompletionsOut,
   NEXUS_Graphicsv3dCompletionInfo  *psCompletionInfo,
   uint32_t                         *puiCompletionsOut,
   NEXUS_Graphicsv3dCompletion      *psCompletions
   )
{
   BERR_Code berr;

   BDBG_ENTER(NEXUS_Graphicsv3d_GetCompletions);

   berr = BVC5_GetCompletions(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId,
                              uiNumFinalizedJobs, puiFinalizedJobs,
                              uiMaxCompletionsOut, (BVC5_CompletionInfo*)psCompletionInfo,
                              puiCompletionsOut, (BVC5_Completion*)psCompletions);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetCompletions);

   return berr == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_UNKNOWN;
}

/*************************************************************************************************/

static void NEXUS_Graphicsv3d_P_SecureToggleHandler(bool bEnter)
{
#if NEXUS_HAS_SAGE
   /* if nothing more to advance, check for mode switch */
   BAVC_CoreList coreList;
   int rc = 0;

   BKNI_Memset(&coreList, 0, sizeof(coreList));
   coreList.aeCores[BAVC_CoreId_eV3D_0] = true;
   coreList.aeCores[BAVC_CoreId_eV3D_1] = true;
   if (bEnter)
      rc = NEXUS_Sage_AddSecureCores(&coreList);
   else
      NEXUS_Sage_RemoveSecureCores(&coreList);

   if (rc) BERR_TRACE(rc);
#else
   BSTD_UNUSED(bEnter);
#endif
}

/*************************************************************************************************/
void NEXUS_Graphicsv3d_GetPerfNumCounterGroups(
   NEXUS_Graphicsv3dHandle  hGfx,
   uint32_t                *puiNumGroups
   )
{
   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_GetPerfNumCounterGroups);

   BVC5_GetPerfNumCounterGroups(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, puiNumGroups);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetPerfNumCounterGroups);
}

void NEXUS_Graphicsv3d_GetPerfCounterDesc(
   NEXUS_Graphicsv3dHandle             hGfx,                   /* [in]  */
   uint32_t                            uiGroup,                /* [in]  */
   uint32_t                            uiCounter,              /* [in]  */
   NEXUS_Graphicsv3dCounterDesc        *psDesc                 /* [out] */
   )
{
   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_GetPerfCounterDesc);

   BVC5_GetPerfCounterDesc(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, uiGroup, uiCounter, (BVC5_CounterDesc *)psDesc);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetPerfCounterDesc);
}

void NEXUS_Graphicsv3d_GetPerfCounterGroupInfo(
   NEXUS_Graphicsv3dHandle             hGfx,
   uint32_t                            uiGroup,
   uint32_t                            uiGrpNameSize,
   char                                *chGrpName,
   uint32_t                            *uiMaxActiveCounter,
   uint32_t                            *uiTotalCounter
   )
{
   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_GetPerfCounterGroupInfo);

   BVC5_GetPerfCounterGroupInfo(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, uiGroup, uiGrpNameSize, chGrpName, uiMaxActiveCounter, uiTotalCounter);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetPerfCounterGroupInfo);
}

NEXUS_Error NEXUS_Graphicsv3d_SetPerfCounting(
   NEXUS_Graphicsv3dHandle             hGfx,
   NEXUS_Graphicsv3dCounterState       eState
   )
{
   BERR_Code            err;

   BDBG_ENTER(NEXUS_Graphicsv3d_SetPerfCounting);

   err = BVC5_SetPerfCounting(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId, (BVC5_CounterState)eState);

   BDBG_LEAVE(NEXUS_Graphicsv3d_SetPerfCounting);

   switch (err)
   {
   case BERR_NOT_AVAILABLE:      return NEXUS_NOT_AVAILABLE;
   case BERR_INVALID_PARAMETER:  return NEXUS_INVALID_PARAMETER;
   case BERR_SUCCESS:            return NEXUS_SUCCESS;
   default:                      return NEXUS_UNKNOWN;
   }
}

void NEXUS_Graphicsv3d_ChoosePerfCounters(
   NEXUS_Graphicsv3dHandle                hGfx,
   const NEXUS_Graphicsv3dCounterSelector *psSelector
   )
{
   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_ChoosePerfCounters);

   BVC5_ChoosePerfCounters(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId, (BVC5_CounterSelector *)psSelector);

   BDBG_LEAVE(NEXUS_Graphicsv3d_ChoosePerfCounters);
}

void NEXUS_Graphicsv3d_GetPerfCounterData(
   NEXUS_Graphicsv3dHandle    hGfx,
   uint32_t                   uiMaxCounters,
   uint32_t                   uiResetCounts,
   uint32_t                   *puiCountersOut,
   NEXUS_Graphicsv3dCounter   *psCounters
   )
{
   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_GetPerfCounterData);

   *puiCountersOut = BVC5_GetPerfCounterData(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, uiMaxCounters,
                                             uiResetCounts, (BVC5_Counter *)psCounters);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetPerfCounterData);
}

void NEXUS_Graphicsv3d_GetEventCounts(
   NEXUS_Graphicsv3dHandle  hGfx,
   uint32_t                 *uiNumTracks,
   uint32_t                 *uiNumEvents
   )
{
   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_GetEventCounts);

   BVC5_GetEventCounts(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, uiNumTracks, uiNumEvents);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetEventCounts);
}

NEXUS_Error NEXUS_Graphicsv3d_GetEventTrackInfo(
   NEXUS_Graphicsv3dHandle           hGfx,
   uint32_t                          uiTrack,
   NEXUS_Graphicsv3dEventTrackDesc   *psTrackDesc
   )
{
   BERR_Code            berr;

   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_GetEventTrackInfo);

   berr = BVC5_GetEventTrackInfo(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, uiTrack, (BVC5_EventTrackDesc *)psTrackDesc);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetEventTrackInfo);

   return berr == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_INVALID_PARAMETER;
}

NEXUS_Error NEXUS_Graphicsv3d_GetEventInfo(
   NEXUS_Graphicsv3dHandle       hGfx,
   uint32_t                      uiEvent,
   NEXUS_Graphicsv3dEventDesc   *psEventDesc
   )
{
   BERR_Code        berr;

   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_GetEventInfo);

   berr = BVC5_GetEventInfo(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, uiEvent, (BVC5_EventDesc *)psEventDesc);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetEventInfo);

   return berr == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_INVALID_PARAMETER;
}

NEXUS_Error NEXUS_Graphicsv3d_GetEventDataFieldInfo(
   NEXUS_Graphicsv3dHandle          hGfx,
   uint32_t                         uiEvent,
   uint32_t                         uiField,
   NEXUS_Graphicsv3dEventFieldDesc  *psFieldDesc
   )
{
   BERR_Code            berr;

   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_GetEventDataFieldInfo);

   berr = BVC5_GetEventDataFieldInfo(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, uiEvent, uiField, (BVC5_EventFieldDesc *)psFieldDesc);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetEventDataFieldInfo);

   return berr == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_INVALID_PARAMETER;
}

NEXUS_Error NEXUS_Graphicsv3d_SetEventCollection(
   NEXUS_Graphicsv3dHandle       hGfx,
   NEXUS_Graphicsv3dEventState   eState
   )
{
   BERR_Code        berr;

   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_SetEventCollection);

   berr = BVC5_SetEventCollection(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId, (BVC5_EventState)eState);

   BDBG_LEAVE(NEXUS_Graphicsv3d_SetEventCollection);

   return berr == BERR_SUCCESS ? NEXUS_SUCCESS : NEXUS_INVALID_PARAMETER;
}

void NEXUS_Graphicsv3d_GetEventData(
   NEXUS_Graphicsv3dHandle    hGfx,
   uint32_t                   uiEventBufferBytes,
   void                       *pvEventBuffer,
   uint32_t                   *puiLostData,
   uint64_t                   *puiTimeStamp,
   uint32_t                   *puiBytesCopiedOut
   )
{
   BSTD_UNUSED(hGfx);

   BDBG_ENTER(NEXUS_Graphicsv3d_GetEventData);

   *puiBytesCopiedOut = BVC5_GetEventData(g_NEXUS_Graphicsv3d_P_ModuleState.hVc5, hGfx->uiClientId,
                                          uiEventBufferBytes, pvEventBuffer, puiLostData, puiTimeStamp);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetEventData);
}

void NEXUS_Graphicsv3d_GetTime(uint64_t *pMicroseconds)
{
   BDBG_ENTER(NEXUS_Graphicsv3d_GetTime);

   BVC5_GetTime(pMicroseconds);

   BDBG_LEAVE(NEXUS_Graphicsv3d_GetTime);
}
