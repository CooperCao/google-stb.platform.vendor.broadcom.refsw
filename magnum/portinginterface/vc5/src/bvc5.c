/***************************************************************************
 *     Broadcom Proprietary and Confidential. (c)2014 Broadcom.  All rights reserved.
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
 **************************************************************************/

#include "bstd.h"
#include "bvc5.h"
#include "bvc5_priv.h"
#include "bvc5_registers_priv.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

BDBG_MODULE(BVC5);

static void BVC5_P_UnregisterAllClients(BVC5_Handle hVC5);

BVC5_BinPoolHandle BVC5_P_GetBinPool(
   BVC5_Handle hVC5
)
{
   return (hVC5->bSecure && hVC5->hSecureBinPool) ? hVC5->hSecureBinPool : hVC5->hBinPool;
}

BMEM_Heap_Handle BVC5_P_GetHeap(
   BVC5_Handle hVC5
)
{
   return (hVC5->bSecure && hVC5->hSecureHeap) ? hVC5->hSecureHeap : hVC5->hHeap;
}

BMMA_Heap_Handle BVC5_P_GetMMAHeap(
   BVC5_Handle hVC5
)
{
   return (hVC5->bSecure && hVC5->hSecureMMAHeap) ? hVC5->hSecureMMAHeap : hVC5->hMMAHeap;
}

/***************************************************************************/

void BVC5_GetDefaultOpenParameters(
   BVC5_OpenParameters *openParams
)
{
   if (openParams != NULL)
   {
      openParams->bUseClockGating = true;
      openParams->bUseNexusMMA = true;
      openParams->bUseStallDetection = true;

#ifdef V3D_HAS_BPCM
      openParams->bUsePowerGating = true;
#else
      openParams->bUsePowerGating = false;
#endif
      openParams->bGPUMonDeps   = false;
      openParams->bNoQueueAhead = false;

      openParams->bResetOnStall = true;
      openParams->bMemDumpOnStall = false;

      openParams->bNoBurstSplitting = false;
   }
}

/***************************************************************************/
BERR_Code BVC5_Open(
   BVC5_Handle          *phVC5,
   BCHP_Handle          hChp,
   BREG_Handle          hReg,
   BMEM_Heap_Handle     hHeap,
   BMMA_Heap_Handle     hMMAHeap,
   BMEM_Heap_Handle     hSecureHeap,
   BMMA_Heap_Handle     hSecureMMAHeap,
   BINT_Handle          hInt,
   BVC5_OpenParameters *sOpenParams,
   BVC5_Callbacks      *sCallbacks
)
{
   BERR_Code   err  = BERR_SUCCESS;
   BVC5_Handle hVC5 = NULL;

   BDBG_ENTER(BVC5_Open);

   /* allocate device handle */
   hVC5 = (BVC5_Handle)BKNI_Malloc(sizeof (BVC5_P_Handle));
   if (hVC5 == NULL)
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);

   /* init device handle structure to default value */
   BKNI_Memset((void *)hVC5, 0, sizeof (BVC5_P_Handle));

   /* save base modules handles for future use */
   hVC5->hChp  = hChp;
   hVC5->hReg  = hReg;
   hVC5->hInt  = hInt;

   /* Must both be set or both be unset */
   if ((hSecureHeap != NULL) ^ (hSecureMMAHeap != NULL))
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   /* Heaps */
   hVC5->hHeap          = hHeap;
   hVC5->hMMAHeap       = hMMAHeap;
   hVC5->hSecureHeap    = hSecureHeap;
   hVC5->hSecureMMAHeap = hSecureMMAHeap;

   /* Parameters */
   hVC5->sOpenParams = *sOpenParams;
   hVC5->sCallbacks  = *sCallbacks;

   /* NexusMMA API usage is always on now - so override whatever is sent in */
   hVC5->sOpenParams.bUseNexusMMA = true;

   /* Keep track of whether the security scrubbing code is being run */
   hVC5->bToggling      = false;

#ifndef V3D_HAS_BPCM
   /* Don't allow an override if it's not supported */
   hVC5->sOpenParams.bUsePowerGating = false;
#endif

   BDBG_MSG((
      "VC5 options:\n"
      " Power gating = %s\n"
      " Clock gating = %s\n"
      " Stall Detection = %s\n"
      " GPUMon dependencies = %s\n"
      " Reset on stall = %s\n"
      " Dump on stall = %s\n"
      " No burst splitting = %s\n",
      hVC5->sOpenParams.bUsePowerGating ? "on" : "off",
      hVC5->sOpenParams.bUseClockGating ? "on" : "off",
      hVC5->sOpenParams.bUseStallDetection ? "on" : "off",
      hVC5->sOpenParams.bGPUMonDeps ? "on" : "off",
      hVC5->sOpenParams.bResetOnStall ? "on" : "off",
      hVC5->sOpenParams.bMemDumpOnStall ? "on" : "off",
      hVC5->sOpenParams.bNoBurstSplitting ? "on" : "off"
      ));

   err = BKNI_CreateMutex(&hVC5->hModuleMutex);
   if (err != BERR_SUCCESS)
      goto exit;

   err = BKNI_CreateMutex(&hVC5->hEventMutex);
   if (err != BERR_SUCCESS)
      goto exit;

   err = BVC5_P_FenceArrayCreate(&hVC5->hFences);
   if (err != BERR_SUCCESS)
      goto exit;

   err = BVC5_P_ClientMapCreate(hVC5, &hVC5->hClientMap);
   if (err != BERR_SUCCESS)
      goto exit;

   err = BVC5_P_SchedulerStateConstruct(&hVC5->sSchedulerState);
   if (err != BERR_SUCCESS)
      goto exit;

   err = BVC5_P_BinPoolCreate(hMMAHeap, &hVC5->hBinPool);
   if (err != BERR_SUCCESS)
      goto exit;

   if (hSecureMMAHeap)
   {
      err = BVC5_P_BinPoolCreate(hSecureMMAHeap, &hVC5->hSecureBinPool);
      if (err != BERR_SUCCESS)
         goto exit;
   }

   /* "Create" the hardware */
#if defined(BVC5_HARDWARE_SIMPENROSE)
   err = BVC5_P_SimpenroseInit(hVC5, &hVC5->hSimpenrose);
   if (err != BERR_SUCCESS)
      goto exit;
#elif defined(BVC5_HARDWARE_NONE)
   BKNI_Printf("=== NULL BVC5 Driver - don't expect any output ===\n")
#endif

   BVC5_P_HardwareInitializeCoreStates(hVC5);

   /* Power up the core now if we don't want to use dynamic power gating */
   /* Note: must do this before calling BVC5_P_HardwarePowerAcquire */
   if (!hVC5->sOpenParams.bUsePowerGating)
      BVC5_P_HardwareBPCMPowerUp(hVC5);

   /* Turn on the clocks now if we don't want to use clock gating */
   if (!hVC5->sOpenParams.bUseClockGating)
      BVC5_P_HardwarePowerAcquire(hVC5, ~0u);

   BVC5_P_InitPerfCounters(hVC5);
   BVC5_P_InitEventMonitor(hVC5);

   /* Putting core into standby will eventually result in all cores being powered down */
   hVC5->bInStandby = false;

#ifdef BVC5_HARDWARE_REAL
   /* TODO: connect up interrupts for the TFU however it is configured */
   /* install an IRQ handler for the module */
   BINT_CreateCallback(&hVC5->callback_intctl, hInt,
                       BCHP_INT_ID_V3D_INTR, BVC5_P_InterruptHandler_isr, hVC5, 0);

   /* Leave inplace for TFU, this is a HUB interrupt */
   BINT_CreateCallback(&hVC5->callback_hub_intctl, hInt,
                       BCHP_INT_ID_V3D_HUB_INTR, BVC5_P_InterruptHandlerHub_isr, hVC5, 0);
#endif

   *phVC5 = hVC5;

   err = BERR_SUCCESS;

exit:
   if (err != BERR_SUCCESS)
      BVC5_Close(hVC5);

   BDBG_LEAVE(BVC5_Open);

   return err;
}

/***************************************************************************/
BERR_Code BVC5_Close(
   BVC5_Handle hVC5
)
{
   BERR_Code err = BERR_SUCCESS;

   BDBG_ENTER(BVC5_Close);

   if (hVC5 == NULL)
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   if (hVC5->bSchedulerRunning)
   {
      /* Tell the scheduler to quit ...
       * Signaling events to stop the scheduler:
       * Two events need to be signaled consecutively
       * One to signal the termination and
       * one to make the scheduler progress
       * The scheduler will destroy the events once
       * terminated so the critical section makes sure
       * that the two events occurs without interruption
       * in between. */
      BKNI_EnterCriticalSection();
      BKNI_SetEvent(hVC5->hSchedulerTerminateEvent);
      BKNI_SetEvent(hVC5->hSchedulerWakeEvent);
      BKNI_LeaveCriticalSection();

      /* ... and wait until it has */
      BKNI_WaitForEvent(hVC5->hSchedulerSyncEvent, BKNI_INFINITE);
   }

   /* Make sure that we leave SAGE with secure mode off */
   if (hVC5->bSecure && hVC5->sCallbacks.fpSecureToggleHandler)
      hVC5->sCallbacks.fpSecureToggleHandler(false);

#if defined(BVC5_HARDWARE_SIMPENROSE)
   BVC5_P_SimpenroseTerm(hVC5, hVC5->hSimpenrose);
#endif

   BKNI_AcquireMutex(hVC5->hModuleMutex);
   BVC5_P_UnregisterAllClients(hVC5);     /* Requires that the mutex is locked */
   BKNI_ReleaseMutex(hVC5->hModuleMutex);

   BVC5_P_FenceArrayDestroy(hVC5->hFences);
   BVC5_P_ClientMapDestroy(hVC5, hVC5->hClientMap);
   BVC5_P_SchedulerStateDestruct(&hVC5->sSchedulerState);

   BVC5_P_BinPoolDestroy(hVC5->hBinPool);
   BVC5_P_BinPoolDestroy(hVC5->hSecureBinPool);

   if (hVC5->hGMPTable)
      BMMA_Free(hVC5->hGMPTable);

#ifdef BVC5_HARDWARE_REAL
   /* remove IRQ handler */
   if (hVC5->callback_intctl != NULL)
      BINT_DestroyCallback(hVC5->callback_intctl);

   /* remove V3D HUB (TFU) */
   if (hVC5->callback_hub_intctl != NULL)
      BINT_DestroyCallback(hVC5->callback_hub_intctl);
#endif

   /* Shut everything down */
   if (!hVC5->sOpenParams.bUseClockGating)
      BVC5_P_HardwarePowerRelease(hVC5, ~0u);

   if (!hVC5->sOpenParams.bUsePowerGating)
      BVC5_P_HardwareBPCMPowerDown(hVC5);

   if (hVC5->hEventMutex != NULL)
      BKNI_DestroyMutex(hVC5->hEventMutex);

   if (hVC5->hModuleMutex != NULL)
      BKNI_DestroyMutex(hVC5->hModuleMutex);

   BKNI_Free(hVC5);

exit:
   BDBG_LEAVE(BVC5_Close);
   return err;
}

/***************************************************************************/
void BVC5_GetInfo(
   BVC5_Handle hVC5,
   BVC5_Info   *pInfo
)
{
   BDBG_ENTER(BVC5_GetInfo);

   if ((pInfo == NULL) || (hVC5 == NULL))
   {
      BDBG_LEAVE(BVC5_GetInfo);
      return;
   }

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   BVC5_P_HardwareGetInfo(hVC5, pInfo);

   BKNI_ReleaseMutex(hVC5->hModuleMutex);
   BDBG_LEAVE(BVC5_GetInfo);
}

/* BVC5_P_GetFreeClientId
 *
 * Generate unique client id.
 */
uint32_t BVC5_P_GetFreeClientId(BVC5_Handle hVC5)
{
   hVC5->uiNextClientId++;

   return hVC5->uiNextClientId;
}


/***************************************************************************/
BERR_Code BVC5_RegisterClient(
   BVC5_Handle  hVC5,
   void        *pContext,
   uint32_t    *puiClientId
)
{
   BERR_Code         err = BERR_SUCCESS;

   BDBG_ENTER(BVC5_RegisterClient);

   if (hVC5 == NULL)
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   *puiClientId = BVC5_P_GetFreeClientId(hVC5);

   err = BVC5_P_ClientMapCreateAndInsert(hVC5, hVC5->hClientMap, pContext, *puiClientId);
   if (err != BERR_SUCCESS)
      goto exit1;

   err = BVC5_P_SchedulerStateRegisterClient(&hVC5->sSchedulerState, BVC5_P_ClientMapSize(hVC5->hClientMap));
   if (err != BERR_SUCCESS)
      BVC5_P_ClientMapRemoveAndDestroy(hVC5, hVC5->hClientMap, *puiClientId);

exit1:
   BKNI_ReleaseMutex(hVC5->hModuleMutex);

exit:
   BDBG_LEAVE(BVC5_RegisterClient);

   return err;
}

/***************************************************************************/
BERR_Code BVC5_P_UnregisterClient(
   BVC5_Handle hVC5,
   uint32_t    uiClientId
)
{
   BERR_Code         err = BERR_SUCCESS;
   BVC5_ClientHandle hClient;

   if (uiClientId == 0)
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, uiClientId);

   if (hClient == NULL)
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   /* Wait for client to be out of hardware units
      Remove all (local non-shared) fences from this client
      Remove jobs from the state map
      Remove client's jobs from waitq
      Clean up client resources (runnable and completed lists)
      Tell scheduler there is one less client
    */
   BVC5_P_WaitForJobCompletion(hVC5, uiClientId);
   BVC5_P_ClientMapRemoveAndDestroy(hVC5, hVC5->hClientMap, uiClientId);
   BVC5_P_FenceClientDestroy(hVC5->hFences, uiClientId);
   BVC5_P_FenceClientCheckDestroy(hVC5->hFences, uiClientId);
   BVC5_P_SchedulerStateResetFirst(&hVC5->sSchedulerState);

   /* Remove any acquire locks current held be event or perf counters */
   BVC5_P_PerfCountersRemoveClient(hVC5, uiClientId);
   BVC5_P_EventsRemoveClient(hVC5, uiClientId);

exit:
   return err;
}

/***************************************************************************/
static void BVC5_P_UnregisterAllClients(
   BVC5_Handle hVC5
   )
{
   void              *pIter;
   BVC5_ClientHandle h;

   BDBG_ENTER(BVC5_P_UnregisterAllClients);

   h = BVC5_P_ClientMapFirst(hVC5->hClientMap, &pIter);

   while (h != NULL)
   {
      BVC5_P_UnregisterClient(hVC5, h->uiClientId);
      /* Always removing head of list - hence MapFirst not MapNext */
      h = BVC5_P_ClientMapFirst(hVC5->hClientMap, &pIter);
   }

   BDBG_LEAVE(BVC5_P_UnregisterAllClients);
}

/***************************************************************************/
BERR_Code BVC5_UnregisterClient(
   BVC5_Handle hVC5,
   uint32_t    uiClientId
)
{
   BERR_Code      err = BERR_SUCCESS;

   BDBG_ENTER(BVC5_UnregisterClient);

   if (hVC5 == NULL)
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   err = BVC5_P_UnregisterClient(hVC5, uiClientId);

exit:
   if (hVC5 != NULL)
   {
      BKNI_ReleaseMutex(hVC5->hModuleMutex);
   }

   BDBG_LEAVE(BVC5_UnregisterClient);

   return err;
}

/* BVC5_P_AddJob

   Add a new job into the system
   Jobs go into the waiting jobs queue, then move to the runnable queue when
   their dependencies have completed.  They then proceed to the appropriate hardware
   unit in order.  When the hardware completes, the job goes into the completed queue.
   When the completed queue is emptied the jobs are retired.

*/
static void BVC5_P_AddJob(
   BVC5_Handle           hVC5,
   BVC5_ClientHandle     hClient,
   BVC5_P_InternalJob   *psJob
)
{
   BVC5_P_ClientJobToWaiting(hVC5, hClient, psJob);
}

BERR_Code BVC5_NullJob(
   BVC5_Handle                 hVC5,
   uint32_t                    uiClientId,
   const BVC5_JobNull         *pNull
)
{
   BERR_Code             err;
   BVC5_P_InternalJob   *pNullJob = NULL;
   BVC5_ClientHandle     hClient;

   BDBG_ENTER(BVC5_NullJob);

   if (pNull == NULL)
      return BERR_INVALID_PARAMETER;

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, uiClientId);
   if (hClient == NULL || !BVC5_P_ClientSetMaxJobId(hClient, pNull->sBase.uiJobId))
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   /* Take a copy of the jobs */
   pNullJob = BVC5_P_JobCreateNull(hVC5, uiClientId, pNull);
   if (pNullJob == NULL)
   {
      err = BERR_OUT_OF_SYSTEM_MEMORY;
      goto exit;
   }

   BVC5_P_AddJob(hVC5, hClient, pNullJob);
   BVC5_P_PumpClient(hVC5, hClient);

   err = BERR_SUCCESS;

exit:
   BKNI_ReleaseMutex(hVC5->hModuleMutex);

   BDBG_LEAVE(BVC5_NullJob);

   return err;
}

BERR_Code BVC5_UsermodeJob(
   BVC5_Handle                 hVC5,
   uint32_t                    uiClientId,
   const BVC5_JobUsermode     *pUsermode
)
{
   BERR_Code            err;
   BVC5_P_InternalJob  *pUsermodeJob = NULL;
   BVC5_ClientHandle    hClient;

   BDBG_ENTER(BVC5_UsermodeJob);

   if (pUsermode == NULL)
      return BERR_INVALID_PARAMETER;

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, uiClientId);
   if (hClient == NULL || !BVC5_P_ClientSetMaxJobId(hClient, pUsermode->sBase.uiJobId))
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   /* Take a copy of the jobs */
   pUsermodeJob = BVC5_P_JobCreateUsermode(hVC5, uiClientId, pUsermode);
   if (pUsermodeJob == NULL)
   {
      err = BERR_OUT_OF_SYSTEM_MEMORY;
      goto exit;
   }

   BVC5_P_AddJob(hVC5, hClient, pUsermodeJob);
   BVC5_P_PumpClient(hVC5, hClient);

   err = BERR_SUCCESS;

exit:
   BKNI_ReleaseMutex(hVC5->hModuleMutex);

   BDBG_LEAVE(BVC5_UsermodeJob);

   return err;
}

BERR_Code BVC5_BinRenderJob(
   BVC5_Handle                 hVC5,
   uint32_t                    uiClientId,
   const BVC5_JobBin          *pBin,
   const BVC5_JobRender       *pRender
)
{
   BERR_Code             err;
   BVC5_P_InternalJob   *pBinJob    = NULL;
   BVC5_P_InternalJob   *pRenderJob = NULL;
   BVC5_ClientHandle     hClient;

   BDBG_ENTER(BVC5_BinRenderJob);

   /* We will allow for no bin job */
   if (pRender == NULL)
      return BERR_INVALID_PARAMETER;

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, uiClientId);
   if (hClient == NULL || !BVC5_P_ClientSetMaxJobId(hClient, pBin->sBase.uiJobId)
                       || !BVC5_P_ClientSetMaxJobId(hClient, pRender->sBase.uiJobId))
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   /* Take a copy of the jobs */
   pRenderJob = BVC5_P_JobCreateRender(hVC5, uiClientId, pRender);
   if (pRenderJob == NULL)
   {
      err = BERR_OUT_OF_SYSTEM_MEMORY;
      goto exit;
   }

   if (pBin != NULL)
   {
      pBinJob = BVC5_P_JobCreateBin(hVC5, uiClientId, pBin, pRenderJob);
      if (pBinJob == NULL)
      {
         BVC5_P_JobDestroy(hVC5, pRenderJob);
         err = BERR_OUT_OF_SYSTEM_MEMORY;
         goto exit;
      }
   }

   if (pBinJob != NULL)
   {
      BVC5_P_AddJob(hVC5, hClient, pBinJob);
      /* Render job implicitly depends on bin job */
      pRenderJob->jobData.sRender.uiBinJobId = pBinJob->uiJobId;
   }
   else
      pRenderJob->jobData.sRender.bRenderOnlyJob = true;

   BVC5_P_AddJob(hVC5, hClient, pRenderJob);
   BVC5_P_PumpClient(hVC5, hClient);

   err = BERR_SUCCESS;

exit:
   BKNI_ReleaseMutex(hVC5->hModuleMutex);

   BDBG_LEAVE(BVC5_BinRenderJob);

   return err;
}

BERR_Code BVC5_TestJob(
   BVC5_Handle                 hVC5,
   uint32_t                    uiClientId,
   const BVC5_JobTest         *pJob
)
{
   BERR_Code             err = BERR_SUCCESS;
   BVC5_P_InternalJob   *pTestJob = NULL;
   BVC5_ClientHandle     hClient;

   BDBG_ENTER(BVC5_TestJob);

   if (pJob == NULL)
      return BERR_INVALID_PARAMETER;

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, uiClientId);
   if (hClient == NULL || !BVC5_P_ClientSetMaxJobId(hClient, pJob->sBase.uiJobId))
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   /* Take a copy of the jobs */
   pTestJob = BVC5_P_JobCreateTest(hVC5, uiClientId, pJob);
   if (pTestJob == NULL)
   {
      err = BERR_OUT_OF_SYSTEM_MEMORY;
      goto exit;
   }

   BVC5_P_AddJob(hVC5, hClient, pTestJob);
   BVC5_P_PumpClient(hVC5, hClient);

   err = BERR_SUCCESS;

exit:
   BKNI_ReleaseMutex(hVC5->hModuleMutex);

   BDBG_LEAVE(BVC5_TestJob);

   return err;
}


BERR_Code BVC5_FenceWaitJob(
   BVC5_Handle                 hVC5,
   uint32_t                    uiClientId,
   const BVC5_JobFenceWait    *pJob
)
{
   BERR_Code             err;
   BVC5_P_InternalJob   *pWaitJob;
   BVC5_ClientHandle     hClient;

   BDBG_ENTER(BVC5_FenceWaitJob);

   if (pJob == NULL)
      return BERR_INVALID_PARAMETER;

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, uiClientId);
   if (hClient == NULL || !BVC5_P_ClientSetMaxJobId(hClient, pJob->sBase.uiJobId))
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   /* Take a copy of the job */
   pWaitJob = BVC5_P_JobCreateFenceWait(hVC5, uiClientId, pJob);
   if (pWaitJob == NULL)
   {
      err = BERR_OUT_OF_SYSTEM_MEMORY;
      goto exit;
   }

   BVC5_P_AddJob(hVC5, hClient, pWaitJob);
   BVC5_P_PumpClient(hVC5, hClient);

   err = BERR_SUCCESS;

exit:
   BKNI_ReleaseMutex(hVC5->hModuleMutex);

   BDBG_LEAVE(BVC5_FenceWaitJob);

   return err;
}

BERR_Code BVC5_TFUJobs(
   BVC5_Handle                 hVC5,
   uint32_t                    uiClientId,
   uint32_t                    uiNumJobs,
   const BVC5_JobTFU          *pTFUJobs
)
{
   BERR_Code             err = BERR_SUCCESS;
   BVC5_P_InternalJob   *pInternalJobs[32];
   BVC5_ClientHandle     hClient;
   uint32_t              i;

   BDBG_ENTER(BVC5_TFUJob);

   if (pTFUJobs == NULL || uiNumJobs > 32)
      return BERR_INVALID_PARAMETER;

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, uiClientId);
   if (hClient == NULL)
      goto exit;

   for (i = 0; i != uiNumJobs; ++i)
   {
      if (!BVC5_P_ClientSetMaxJobId(hClient, pTFUJobs[i].sBase.uiJobId))
      {
         err = BERR_INVALID_PARAMETER;
         break;
      }

      /* Take a copy of the job */
      pInternalJobs[i] = BVC5_P_JobCreateTFU(hVC5, uiClientId, &pTFUJobs[i]);
      if (pInternalJobs[i] == NULL)
      {
         err = BERR_OUT_OF_SYSTEM_MEMORY;
         break;
      }
   }

   if (err != BERR_SUCCESS)
   {
      while (i-- != 0)
      {
         BVC5_P_JobDestroy(hVC5, pInternalJobs[i]);
      }
   }
   else
   {
      for (i = 0; i != uiNumJobs; ++i)
      {
         BVC5_P_AddJob(hVC5, hClient, pInternalJobs[i]);
      }
      BVC5_P_PumpClient(hVC5, hClient);
   }

exit:
   BKNI_ReleaseMutex(hVC5->hModuleMutex);

   BDBG_LEAVE(BVC5_TFUJob);

   return err;
}

static bool BVC5_P_HaveDepsCompleted(
   BVC5_ClientHandle        hClient,
   BVC5_SchedDependencies  *pDeps
   )
{
   bool     hasNoDependencies = true;
   uint32_t i;

   if (pDeps != NULL)
      for (i = 0; hasNoDependencies && i < pDeps->uiNumDeps; ++i)
         hasNoDependencies = BVC5_P_ClientIsJobComplete(hClient, pDeps->uiDep[i]);

   return hasNoDependencies;
}

static bool BVC5_P_HaveDepsFinalized(
   BVC5_ClientHandle        hClient,
   BVC5_SchedDependencies  *pDeps
   )
{
   bool     hasNoDependencies = true;
   uint32_t i;

   if (pDeps != NULL)
      for (i = 0; hasNoDependencies && i < pDeps->uiNumDeps; ++i)
         hasNoDependencies = BVC5_P_ClientIsJobFinalized(hClient, pDeps->uiDep[i]);

   return hasNoDependencies;
}

BERR_Code BVC5_Query(
   BVC5_Handle             hVC5,
   uint32_t                uiClientId,
   BVC5_SchedDependencies *pCompletedDeps,
   BVC5_SchedDependencies *pFinalizedDeps,
   BVC5_QueryResponse     *pResponse
)
{
   BERR_Code         err;
   BVC5_ClientHandle hClient;

   if (pResponse == NULL)
      return BERR_INVALID_PARAMETER;

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, uiClientId);

   if (hClient == NULL)
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   BVC5_P_PumpClient(hVC5, hClient);

   pResponse->uiStateAchieved = BVC5_P_HaveDepsCompleted(hClient, pCompletedDeps) &&
                                BVC5_P_HaveDepsFinalized(hClient, pFinalizedDeps);

   err = BERR_SUCCESS;

exit:
   BKNI_ReleaseMutex(hVC5->hModuleMutex);

   return err;
}

BERR_Code BVC5_MakeFenceForJobs(
   BVC5_Handle                   hVC5,
   uint32_t                      uiClientId,
   const BVC5_SchedDependencies *pCompletedDeps,
   const BVC5_SchedDependencies *pFinalizedDeps,
   bool                          bForceCreate,
   int                          *piFence
)
{
   BVC5_ClientHandle hClient;
   BERR_Code berr;

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, uiClientId);

   if (hClient == NULL)
      berr = BERR_INVALID_PARAMETER;
   else
      berr = BVC5_P_ClientMakeFenceForJobs(hVC5, hClient,
         pCompletedDeps, pFinalizedDeps, bForceCreate, piFence);

   BKNI_ReleaseMutex(hVC5->hModuleMutex);

   return berr;
}

BERR_Code BVC5_MakeFenceForAnyNonFinalizedJob(
   BVC5_Handle hVC5,
   uint32_t    uiClientId,
   int        *piFence
)
{
   BVC5_ClientHandle hClient;
   BERR_Code berr;

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, uiClientId);

   if (hClient == NULL)
      berr = BERR_INVALID_PARAMETER;
   else
      berr = BVC5_P_ClientMakeFenceForAnyNonFinalizedJob(hVC5, hClient, piFence);

   BKNI_ReleaseMutex(hVC5->hModuleMutex);

   return berr;
}

BERR_Code BVC5_FenceRegisterWaitCallback(
   BVC5_Handle    hVC5,
   int            iFence,
   void         (*pfnCallback)(void *, void *),
   void          *pContext,
   void          *pParam
)
{
   BVC5_FenceArrayHandle hFenceArr = hVC5->hFences;

   BDBG_ENTER(BVC5_FenceRegisterWaitCallback);

   BDBG_MSG(("BVC5_FenceRegisterWaitCallback fence %d\n", iFence));

   BDBG_ASSERT(pfnCallback != NULL);

   BVC5_P_FenceAddCallback(hFenceArr, iFence, pfnCallback, pContext, pParam);

   BDBG_LEAVE(BVC5_FenceRegisterWaitCallback);

   return BERR_SUCCESS;
}

BERR_Code BVC5_FenceUnregisterWaitCallback(
   BVC5_Handle    hVC5,
   int            iFence
)
{
   BVC5_FenceArrayHandle hFenceArr = hVC5->hFences;

   BDBG_ENTER(BVC5_FenceUnregisterWaitCallback);

   BDBG_MSG(("BVC5_FenceUnregisterWaitCallback fence %d\n", iFence));

   BVC5_P_FenceRemoveCallback(hFenceArr, iFence);

   BDBG_LEAVE(BVC5_FenceUnregisterWaitCallback);

   return BERR_SUCCESS;
}

BERR_Code BVC5_FenceClose(
   BVC5_Handle hVC5,
   int         iFence
   )
{
   BVC5_FenceArrayHandle  hFenceArr = hVC5->hFences;

   BDBG_ENTER(BVC5_FenceClose);

   BVC5_P_FenceClose(hFenceArr, iFence);

   BDBG_LEAVE(BVC5_FenceClose);

   return BERR_SUCCESS;
}

BERR_Code BVC5_FenceSignal(
   BVC5_Handle hVC5,
   int         iFence
)
{
   BDBG_ENTER(BVC5_FenceSignal);

   BVC5_P_FenceSignalFromUser(hVC5->hFences, iFence);

   BDBG_LEAVE(BVC5_FenceSignal);

   return BERR_SUCCESS;
}

BERR_Code BVC5_FenceMakeLocal(
   BVC5_Handle  hVC5,
   uint32_t     uiClientId,
   int         *pFence
)
{

   BVC5_FenceArrayHandle hFenceArr = hVC5->hFences;

   BDBG_ENTER(BVC5_FenceMakeLocal);

   BDBG_MSG(("BVC5_FenceMakeLocal fence %d\n", *pFence));

   *pFence = BVC5_P_FenceCreateToSignalFromUser(hFenceArr, uiClientId);

   BDBG_LEAVE(BVC5_FenceMakeLocal);

   return *pFence != -1 ? BERR_SUCCESS : BERR_OUT_OF_SYSTEM_MEMORY;
}

/***************************************************************************/
BERR_Code BVC5_GetUsermode(
   BVC5_Handle                 hVC5,
   uint32_t                    uiClientId,
   uint64_t                    uiPrevJobId,
   BVC5_Usermode              *psUsermode
   )
{
   bool                  haveJob       = false;
   BVC5_P_UsermodeState *usermodeState = &hVC5->sUsermodeState;

   if (psUsermode == NULL)
      return false;

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   BSTD_UNUSED(uiClientId); /* Would be needed if we allowed more than one usermode job to be active at a time */

   if (uiPrevJobId != 0)
   {
      BVC5_P_InternalJob  *psJob = usermodeState->psRunningJob;

      if (psJob != NULL) /* It really ought not to be NULL */
      {
         BVC5_ClientHandle    hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, psJob->uiClientId);

         BVC5_P_ClientJobRunningToCompleted(hVC5, hClient, psJob);

         usermodeState->psRunningJob = NULL;

         BVC5_P_PumpClient(hVC5, hClient);
      }
   }

   if (usermodeState->psPendingJob != NULL && usermodeState->psRunningJob == NULL)
   {
      BVC5_P_InternalJob   *psJob        = usermodeState->psPendingJob;
      BVC5_JobUsermode     *usermodeJob = (BVC5_JobUsermode *)psJob->pBase;

      psUsermode->uiJobId     = psJob->uiJobId;
      psUsermode->pfnCallback = usermodeJob->pfnUsermode;
      psUsermode->pData       = usermodeJob->pData;

      usermodeState->psPendingJob = NULL;
      usermodeState->psRunningJob = psJob;

      haveJob = true;
   }

   psUsermode->bHaveJob = haveJob;

   BKNI_ReleaseMutex(hVC5->hModuleMutex);

   return BERR_SUCCESS;
}

BERR_Code BVC5_GetCompletions(
   BVC5_Handle          hVC5,
   uint32_t             uiClientId,
   uint32_t             uiNumFinalizedJobs,
   const uint64_t       *puiFinalizedJobs,
   uint32_t             uiMaxCompletionsOut,
   BVC5_CompletionInfo  *psCompletionInfo,
   uint32_t             *puiCompletionsOut,
   BVC5_Completion      *psCompletions
   )
{
   BVC5_ClientHandle hClient;
   BERR_Code         ret = BERR_SUCCESS;
   uint32_t          u, uiNumPending;

   BDBG_ENTER(BVC5_GetCompletion);

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, uiClientId);

   if (hClient == NULL)
   {
      ret = BERR_INVALID_PARAMETER;
      goto exit;
   }

   /* puiFinalizedJobs contains a list of jobs that have been finalized.
    * Remove from finalizing q now. */
   for (u = 0; u < uiNumFinalizedJobs && puiFinalizedJobs != NULL; u++)
   {
      /* Remove from finalizing queue and destroy */
      BVC5_P_ClientJobFinalizingToFinalized(hVC5, hClient, puiFinalizedJobs[u]);
   }

   BVC5_P_PumpClient(hVC5, hClient);

   /* How many completions are pending */
   uiNumPending = BVC5_P_JobQSize(hClient->hFinalizableQ);

   *puiCompletionsOut = uiNumPending < uiMaxCompletionsOut ? uiNumPending : uiMaxCompletionsOut;

   for (u = 0; u < *puiCompletionsOut; u++)
   {
      BVC5_P_InternalJob  *pJob = BVC5_P_JobQPop(hClient->hFinalizableQ);
      BVC5_JobBase        *pBaseJob = pJob->pBase;

      psCompletions[u].uiJobId      = pJob->uiJobId;
      psCompletions[u].pfnCallback  = pBaseJob->pfnCompletion;
      psCompletions[u].pData        = pBaseJob->pData;
      psCompletions[u].eStatus      = pJob->eStatus;
      psCompletions[u].eType        = pBaseJob->eType;

      BVC5_P_JobQInsert(hClient->hFinalizingQ, pJob);
   }

   /* If there are no more completed jobs, we still need to know the onfid */
   psCompletionInfo->uiOldestNotFinalized = BVC5_P_ClientGetOldestNotFinalizedId(hClient);

exit:
   BKNI_ReleaseMutex(hVC5->hModuleMutex);

   BDBG_LEAVE(BVC5_GetCompletion);

   return BERR_SUCCESS;
}

/***************************************************************************/

BERR_Code BVC5_Standby(
   BVC5_Handle hVC5
)
{
   BERR_Code   err     = BERR_SUCCESS;

   BDBG_ENTER(BV3D_Standby);

   if (hVC5 == NULL)
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

#ifdef BCHP_PWR_SUPPORT
   {
      bool        bIsIdle = false;

      hVC5->bEnterStandby = true;

      /* wait until the core goes idle */
      while (!bIsIdle)
      {
         BKNI_AcquireMutex(hVC5->hModuleMutex);
         bIsIdle = BVC5_P_HardwareIsIdle(hVC5);
         BKNI_ReleaseMutex(hVC5->hModuleMutex);

         if (!bIsIdle)
            BKNI_Sleep(500);
      }

      /* power off if the cores are really powered up but dont change the bPoweredDown states */
      BVC5_P_HardwareStandby(hVC5);
   }
#endif

exit:
   BDBG_LEAVE(BV3D_Standby);

   return err;
}

/***************************************************************************/

BERR_Code BVC5_Resume(
   BVC5_Handle hVC5
)
{
   BERR_Code   err = BERR_SUCCESS;

   BDBG_ENTER(BVC5_Resume);

   if (hVC5 == NULL)
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   BKNI_AcquireMutex(hVC5->hModuleMutex);

#ifdef BCHP_PWR_SUPPORT
   /* Power on cores that were powered on before bPoweredDown states */
   BVC5_P_HardwareResume(hVC5);

   hVC5->bEnterStandby = false;
#endif

   /* Get the pump going again */
   BVC5_P_PumpAllClients(hVC5);

   BKNI_ReleaseMutex(hVC5->hModuleMutex);

exit:
   BDBG_LEAVE(BV3D_Standby);

   return err;
}

/***************************************************************************/

/* BVC5_GetTime
 */
BERR_Code BVC5_GetTime(uint64_t *pMicroseconds)
{
   BERR_Code err;
   BDBG_ENTER(BVC5_GetTime);

   if (pMicroseconds == NULL)
      err = BERR_INVALID_PARAMETER;
   else
      err = BVC5_P_GetTime_isrsafe(pMicroseconds);

   BDBG_LEAVE(BVC5_GetTime);

   return err;
}

/* End of File */
