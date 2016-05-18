/***************************************************************************
 *     (c)2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#include "bvc5_hardware_debug.h"

BDBG_MODULE(BVC5_P);

/* BVC5_P_ProcessInterrupt

   Called by main scheduler loop when an interrupt is signalled.
   The hardware interrupt routine will set the reason and trigger the event.

 */
static void BVC5_P_ProcessInterrupt(
   BVC5_Handle hVC5
)
{
   uint32_t          uiCoreIndex = 0; /* TODO: multi-core */
   uint32_t          uiCapturedReason;
   uint32_t          uiTFUCapturedReason;

   uiCapturedReason         = __sync_fetch_and_and(&hVC5->uiInterruptReason, 0);
   uiTFUCapturedReason      = __sync_fetch_and_and(&hVC5->uiTFUInterruptReason, 0);

   /*BKNI_Printf("Process interrupt %x\n", uiCapturedReason);*/

   /* Render done ? */
   if (BCHP_GET_FIELD_DATA(uiCapturedReason, V3D_CTL_INT_STS_INT, FRDONE))
   {
      BVC5_P_RenderState  *pState  = BVC5_P_HardwareGetRenderState(hVC5, uiCoreIndex);

      if (__sync_add_and_fetch(&pState->uiCapturedRFC, 0) > 0)
      {
         do
         {
            BVC5_ClientHandle     hClient = NULL;
            BVC5_P_InternalJob   *pJob    = pState->psJob[BVC5_P_HW_QUEUE_RUNNING];

            if (pJob != NULL)
            {
               BVC5_P_AddCoreJobEvent(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_CORE_RENDER_TRACK,
                                      BVC5_P_EVENT_MONITOR_RENDERING, BVC5_EventEnd,
                                      pJob);

               BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_RENDER_JOBS_COMPLETED, 1);

               hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, pJob->uiClientId);

               /* TODO - do we need to invalidate the texture cache here? */
               /* Probably - could be writing to a texture */

               /* This job is done, so add to completed queue and remove from job state map */
               BVC5_P_BinMemArrayDestroy(&pJob->jobData.sRender.sBinMemArray, hVC5->hBinPool);
               BVC5_P_ClientJobRunningToCompleted(hClient, pJob);
               BVC5_P_HardwareJobDone(hVC5, uiCoreIndex, BVC5_P_HardwareUnit_eRenderer);
            }
         } while (__sync_sub_and_fetch(&pState->uiCapturedRFC, 1) != 0);
      }
   }

   /* Bin done? */
   if (BCHP_GET_FIELD_DATA(uiCapturedReason, V3D_CTL_INT_STS_INT, FLDONE))
   {
      BVC5_P_BinnerState  *pState  = BVC5_P_HardwareGetBinnerState(hVC5, uiCoreIndex);
      BVC5_ClientHandle     hClient = NULL;
      BVC5_P_InternalJob   *pJob    = pState->psJob;

      if (pJob != NULL)
      {
         BVC5_P_AddCoreJobEvent(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_CORE_BIN_TRACK, BVC5_P_EVENT_MONITOR_BINNING,
                                 BVC5_EventEnd, pJob);

         BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_BIN_JOBS_COMPLETED, 1);

         hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, pJob->uiClientId);

         /* This job is done, so add to completed queue and remove from job state map */
         BVC5_P_ClientJobRunningToCompleted(hClient, pJob);

         /* Render job's dependency is resolved */
         pJob->jobData.sBin.psInternalRenderJob->jobData.sRender.uiBinJobId = 0;

         BVC5_P_HardwareJobDone(hVC5, uiCoreIndex, BVC5_P_HardwareUnit_eBinner);
      }
   }

   if (BCHP_GET_FIELD_DATA(uiCapturedReason, V3D_CTL_INT_STS_INT, OUTOMEM))
   {
      BVC5_P_BinnerState  *pState     = BVC5_P_HardwareGetBinnerState(hVC5, uiCoreIndex);
      BVC5_P_InternalJob  *pJob       = pState->psJob;

      /* Note: if a bin-done and OOM condition happen simultaneously, which they can, the bin done
       * code above will run first, which will cause pJob here to become NULL. That will prevent us
       * having to do anything with the OOM condition. Since the OOM interrupt is edge-triggered,
       * and we cleared it in the real ISR handler, we should be fine. */
      if (pJob != NULL)
      {
         BVC5_P_InternalJob  *pRenderJob = pJob->jobData.sBin.psInternalRenderJob;
         BVC5_BinBlockHandle  pBlock;
         uint32_t             uiPhysOffset = 0;

         /* TODO -- bin only jobs won't currently work as they require somewhere to store memory           */
         /* One way to work around this would be to issue a dummy associated render job with no work in it */
         BVC5_P_AddCoreEvent(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_CORE_BIN_TRACK, pJob->uiJobId,
                             BVC5_P_EVENT_MONITOR_BOOM, BVC5_EventOneshot);

         BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_BIN_OOMS, 1);

         pBlock = BVC5_P_BinMemArrayAdd(&pRenderJob->jobData.sRender.sBinMemArray, hVC5->hBinPool, 0, &uiPhysOffset);

         if (pBlock == NULL)
         {
            /* We have run out of bin memory.  Re-use some memory,
               and let it run to completion on that instead.
             */
            pBlock = BVC5_P_BinMemArrayGetBlock(&pRenderJob->jobData.sRender.sBinMemArray, 0);
            uiPhysOffset = BVC5_P_BinBlockGetPhysical(pBlock);

            pJob->eStatus = BVC5_JobStatus_eOUT_OF_MEMORY;

            /* Bin result will be rubbish, so abandon the render */
            pRenderJob->bAbandon = true;
            pRenderJob->eStatus  = BVC5_JobStatus_eOUT_OF_MEMORY;
         }

         /* TODO: which core for multi-core */
         BVC5_P_HardwareSupplyBinner(hVC5, uiCoreIndex, uiPhysOffset, pBlock->uiNumBytes);

         /* Now the hardware is happy, prepare some more memory for next time */
         BVC5_P_BinPoolReplenish(hVC5->hBinPool);
      }
   }

   /* TFU */
   if (BCHP_GET_FIELD_DATA(uiTFUCapturedReason, V3D_TFU_TFUINT_STS_INT, TFUC))
   {
      BVC5_ClientHandle    hClient = NULL;
      BVC5_P_TFUState     *pState  = &hVC5->sTFUState;
      BVC5_P_InternalJob  *pJob    = pState->psJob;

      if (pJob != NULL)
      {
         BVC5_P_AddTFUJobEvent(hVC5, BVC5_EventEnd, pJob);

         BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_TFU_JOBS_COMPLETED, 1);

         hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, pJob->uiClientId);

         BVC5_P_ClientJobRunningToCompleted(hClient, pJob);

         BVC5_P_HardwareJobDone(hVC5, 0, BVC5_P_HardwareUnit_eTFU);
      }
   }
}

/* BVC5_P_WatchdogTimeout

   Called from the main scheduler loop when the interrupt wait event times out.
   To be used for detecting hardware lock-up and for power down.

 */
static void BVC5_P_WatchdogTimeout(
   BVC5_Handle hVC5
)
{
   /* TODO: multi-core */
   uint32_t    uiCoreIndex = 0;

   BVC5_P_CoreState  *psCoreState = &hVC5->psCoreStates[uiCoreIndex];

   /* We only care if the core is notionally powered on */
   if (psCoreState->uiPowerOnCount > 0)
   {
      /* If it is the first timeout: initialise bin and render addresses */
      if (psCoreState->uiTimeoutCount == 0)
         BVC5_P_HardwareIsUnitStalled(hVC5, uiCoreIndex);

      /* After a number of timeouts check if any unit is stalled */
      if (psCoreState->uiTimeoutCount >= BVC5_TIMEOUTS_FOR_STALL_CHECK)
      {
         /* TODO: multiple cores */
         BVC5_P_HardwareUnitType uiStalled = BVC5_P_HardwareIsUnitStalled(hVC5, uiCoreIndex);

         /* If any unit has stalled, we will have to reset and abandon */
         /* TODO: multi core -- render jobs could be in multiple cores so
            we will probably need to apply a nuclear option and reset all such cores
            and abandon all jobs therein
          */
         if (uiStalled != 0)
         {
            BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_LOCKUP_DETECTION, 1);

            if (uiStalled & BVC5_P_HardwareUnit_eBinner)
            {
               BVC5_P_AddCoreEventCJ(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_SCHED_TRACK,
                                     BVC5_P_EVENT_MONITOR_BIN_LOCKUP, BVC5_EventOneshot,
                                     psCoreState->sBinnerState.psJob);
            }
            if (uiStalled & BVC5_P_HardwareUnit_eRenderer)
            {
               BVC5_P_AddCoreEventCJ(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_SCHED_TRACK,
                                     BVC5_P_EVENT_MONITOR_RENDER_LOCKUP, BVC5_EventOneshot,
                                     psCoreState->sRenderState.psJob[0/* SAH TODO */]);
            }
            if (uiStalled & BVC5_P_HardwareUnit_eTFU)
            {
               /*
               BVC5_P_AddEvent(hVC5, BVC5_P_EVENT_MONITOR_SCHED_TRACK, 0,
                                 BVC5_P_EVENT_MONITOR_TFU_LOCKUP, BVC5_EventOneshot);
               */
            }

            if (!hVC5->bLockupReported)
            {
               BVC5_P_DebugDump(hVC5, uiCoreIndex);

               if (hVC5->sOpenParams.bResetOnStall)
               {
                  BVC5_P_HardwareAbandonJobs(hVC5, uiCoreIndex);

                  /* Reset the core - sledgehammer time */
                  BVC5_P_HardwareResetCoreAndState(hVC5, uiCoreIndex);

                  /* Make sure we report subsequent lockups */
                  hVC5->bLockupReported = false;
               }
               else
               {
                  /* The user has told us not to reset on lockup, so let's not continually
                   * report the same lockup */
                  hVC5->bLockupReported = true;
               }
            }
         }

         BVC5_P_HardwareResetWatchdog(hVC5, uiCoreIndex);
      }
      else
      {
         /* Count number of timeouts until BVC5_TIMEOUTS_FOR_STALL_CHECK */
         psCoreState->uiTimeoutCount++;
      }
   }
}

/* BVC5_P_GatherClients

   Copy client handles into schedulerState client array for round robin processing
   of clients.

 */
static void BVC5_P_GatherClients(
   BVC5_Handle hVC5
)
{
   BVC5_P_SchedulerState  *psSchedulerState = &hVC5->sSchedulerState;
   BVC5_ClientMapHandle    hClientMap       = hVC5->hClientMap;

   void                *iter = NULL;
   BVC5_ClientHandle    hClient;
   uint32_t             uiNumClients = BVC5_P_ClientMapSize(hClientMap);

   uint32_t i = psSchedulerState->uiClientOffset;

   for (hClient = BVC5_P_ClientMapFirst(hClientMap, &iter);
        hClient != NULL;
        hClient = BVC5_P_ClientMapNext(hClientMap, &iter))
   {
      psSchedulerState->phClientHandles[i] = hClient;

      i = (i + 1) % uiNumClients;
   }
}


/* BVC5_P_ScheduleSoftJobs

   Schedule i.e. run and complete, all jobs with only computational components.

 */
static bool BVC5_P_ScheduleSoftJobs(
   BVC5_Handle       hVC5,
   BVC5_ClientHandle hClient
)
{
   bool                  bIssued = false;
   BVC5_JobQHandle       hSoftJobQ = hClient->hRunnableSoftQ;

   BVC5_P_InternalJob   *pJob = BVC5_P_JobQPop(hSoftJobQ);

   while (pJob != NULL)
   {
      /* Signal jobs need to do something */
      if (pJob->pBase->eType == BVC5_JobType_eFenceSignal)
      {
         BVC5_P_FenceSignalAndCleanup(hVC5->hFences, pJob->jobData.sSignal.signalData);
         pJob->jobData.sSignal.signalData = NULL;
      }

      /* Add to completed queue (for this client) and remove from map */
      BVC5_P_ClientJobRunningToCompleted(hClient, pJob);

      bIssued = true;

      pJob = BVC5_P_JobQPop(hSoftJobQ);
   }

   return bIssued;
}

/* BVC5_P_ScheduleBinnerJob

   Issue a binner job if there is one.  Binner must be available.

 */
static bool BVC5_P_ScheduleBinnerJob(
   BVC5_Handle       hVC5,
   BVC5_ClientHandle hClient
)
{
   BVC5_JobQHandle       hBinnerQ = hClient->hRunnableBinnerQ;
   bool                  issued   = false;
   BVC5_P_InternalJob   *pJob     = BVC5_P_JobQTop(hBinnerQ);

   if (pJob != NULL)
   {
      /* GFXH-1181 workaround for uniform cache issue (TODO -- multicore although probably irrelevant) */
      if (!pJob->bFlushedV3D && BVC5_P_HardwareCacheClearBlocked(hVC5, 0))
         return false;

      issued = BVC5_P_HardwareIssueBinnerJob(hVC5, 0, pJob);
      if (issued)
         BVC5_P_JobQPop(hBinnerQ);
   }

   return issued;
}

/* BVC5_P_ScheduleRenderJob

   Issue a render job if there is one.  Renderer must be available.

 */
static bool BVC5_P_ScheduleRenderJob(
   BVC5_Handle       hVC5,
   BVC5_ClientHandle hClient
)
{
   BVC5_JobQHandle      hRenderQ = hClient->hRunnableRenderQ;
   bool                 issued   = false;
   BVC5_P_InternalJob  *pJob     = BVC5_P_JobQTop(hRenderQ);

   if (pJob != NULL)
   {
      /* GFXH-1181 workaround for uniform cache issue (TODO -- multicore although probably irrelevant) */
      if (!pJob->bFlushedV3D && BVC5_P_HardwareCacheClearBlocked(hVC5, 0))
         return false;

      BVC5_P_JobQPop(hRenderQ);

      BVC5_P_HardwareIssueRenderJob(hVC5, 0, pJob);
      issued = true;
   }

   return issued;
}

/* BVC5_P_ScheduleTFUJob

   Issue a TFU job if there is one.  TFU must be available.

 */
static bool BVC5_P_ScheduleTFUJob(
   BVC5_Handle       hVC5,
   BVC5_ClientHandle hClient
)
{
   BVC5_JobQHandle   hTFUQ  = hClient->hRunnableTFUQ;
   bool              issued = false;

   if (BVC5_P_JobQSize(hTFUQ) != 0)
   {
      BVC5_P_InternalJob   *pJob = BVC5_P_JobQPop(hTFUQ);
      BVC5_P_HardwareIssueTFUJob(hVC5, pJob);
      issued = true;
   }

   return issued;
}

/* BVC5_P_ScheduleUsermodeJob

   Schedule a usermode callback job.  Usermode callback must be free.

 */
static bool BVC5_P_ScheduleUsermodeJob(
   BVC5_Handle       hVC5,
   BVC5_ClientHandle hClient
)
{
   BVC5_JobQHandle   hUsermodeQ = hClient->hRunnableUsermodeQ;
   bool              issued = false;

   if (BVC5_P_JobQSize(hUsermodeQ) != 0)
   {
      BVC5_P_InternalJob  *pJob = BVC5_P_JobQPop(hUsermodeQ);
      BVC5_P_IssueUsermodeJob(hVC5, hClient, pJob);
      issued = true;
   }

   return issued;
}

/* BVC5_P_ScheduleRunnableJobs

   Schedules as many jobs as possible from the client's runnable queues.
   Typically all null, wait and signal jobs will run since these do not require hardware.
   Only one client will likely get to render.

 */
static void BVC5_P_ScheduleRunnableJobs(
   BVC5_Handle hVC5
)
{
   BVC5_P_SchedulerState  *psState       = &hVC5->sSchedulerState;
   unsigned int            uiNumClients  = BVC5_P_ClientMapSize(hVC5->hClientMap);
   bool                    issuedHardJob = false;
   bool                    issuedSoftJob = false;
   uint32_t                uiClient;

   /* No clients, nothing to do */
   if (uiNumClients == 0)
      return;

   /* Sets the psState->phClientHandles which is a list of all the clients */
   BVC5_P_GatherClients(hVC5);

   /* If we are going in to standby then don't issue any more non-local (h/w or usermode) tasks */
   if (!hVC5->bEnterStandby)
   {
      /* Hardware availability */
      uint32_t i;

      bool  renderFree   = BVC5_P_HardwareIsUnitAvailable(hVC5, 0, BVC5_P_HardwareUnit_eRenderer);
      bool  binnerFree   = BVC5_P_HardwareIsUnitAvailable(hVC5, 0, BVC5_P_HardwareUnit_eBinner);
      bool  tfuFree      = BVC5_P_HardwareIsUnitAvailable(hVC5, 0, BVC5_P_HardwareUnit_eTFU);
      bool  userModeFree = BVC5_P_UsermodeIsAvailable(hVC5);

      /* Handle render jobs */
      for (i = 0; i < 2; ++i)
      {
         for (uiClient = 0; renderFree && uiClient < uiNumClients; ++uiClient)
         {
            BVC5_ClientHandle hClient = psState->phClientHandles[uiClient];

            /* Schedule a render job */
            if (BVC5_P_ScheduleRenderJob(hVC5, hClient))
            {
               renderFree    = BVC5_P_HardwareIsUnitAvailable(hVC5, 0, BVC5_P_HardwareUnit_eRenderer);
               issuedHardJob = true;
            }
         }
      }

      /* Handle binner jobs */
      for (uiClient = 0; binnerFree && uiClient < uiNumClients; ++uiClient)
      {
         BVC5_ClientHandle hClient = psState->phClientHandles[uiClient];

         /* Schedule a bin job if there is memory available */
         if (BVC5_P_BinPoolReplenish(hVC5->hBinPool) &&
             BVC5_P_ScheduleBinnerJob(hVC5, hClient))
         {
            binnerFree    = false;  /* Only use one binner slot */
            issuedHardJob = true;
         }
      }

      /* Handle TFU jobs */
      for (uiClient = 0; tfuFree && uiClient < uiNumClients; ++uiClient)
      {
         BVC5_ClientHandle hClient = psState->phClientHandles[uiClient];

         /* Schedule a TFU job */
         if (BVC5_P_ScheduleTFUJob(hVC5, hClient))
         {
            tfuFree       = false;  /* We only use one TFU slot */
            issuedHardJob = true;
         }
      }

      /* Handle usermode jobs */
      for (uiClient = 0; userModeFree && uiClient < uiNumClients; ++uiClient)
      {
         BVC5_ClientHandle hClient = psState->phClientHandles[uiClient];

         if (userModeFree && BVC5_P_ScheduleUsermodeJob(hVC5, hClient))
         {
            userModeFree  = false;  /* Only do one usermode callback at a time */
            issuedHardJob = true;   /* Is this appropriate? It will cause the round-robin to tick on */
         }
      }
   }

   /* Handle soft jobs */
   for (uiClient = 0; uiClient < uiNumClients; ++uiClient)
   {
      BVC5_ClientHandle hClient    = psState->phClientHandles[uiClient];

      /* We can always schedule these */
      issuedSoftJob = issuedSoftJob || BVC5_P_ScheduleSoftJobs(hVC5, hClient);
   }

   /* If soft jobs were issued then it is possible we have more work to do */
   if (issuedSoftJob)
      BKNI_SetEvent(hVC5->hSchedulerWakeEvent);

   /* If any hardware job has been issued to any client, then move the round-robin on one */
   if (issuedHardJob)
      psState->uiClientOffset = (psState->uiClientOffset + 1) % uiNumClients;
}

static bool BVC5_P_AreDepsCompleted(
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *pJob
)
{
   bool           hasNoDependencies = true;   /* Assume we have none until we find one */
   unsigned int   i;

   /* Early exit if all deps satisfied */
   if (pJob->sRunDep_NotCompleted.uiNumDeps == 0)
      return true;

   for (i = 0; hasNoDependencies && i < pJob->sRunDep_NotCompleted.uiNumDeps; ++i)
   {
      uint64_t uiDep = pJob->sRunDep_NotCompleted.uiDep[i];

      /* Still has at least one unresolved dep? */
      if (uiDep != 0)
      {
         if (BVC5_P_ClientIsJobComplete(hClient, uiDep))
         {
            /* If job has gone -- remove from dependency list */
            pJob->sRunDep_NotCompleted.uiDep[i] = 0;
         }
         else
         {
            /* There is still a dependency */
            hasNoDependencies = false;
         }
      }
   }

   if (hasNoDependencies)
      pJob->sRunDep_NotCompleted.uiNumDeps = 0;

   return hasNoDependencies;
}

static bool BVC5_P_AreDepsFinalized(
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *pJob
)
{
   bool           hasNoDependencies = true;   /* Assume we have none until we find one */
   unsigned int   i;

   /* Early exit if all deps satisfied */
   if (pJob->sRunDep_NotFinalized.uiNumDeps == 0)
      return true;

   for (i = 0; hasNoDependencies && i < pJob->sRunDep_NotFinalized.uiNumDeps; ++i)
   {
      uint64_t uiDep = pJob->sRunDep_NotFinalized.uiDep[i];

      /* Still has at least one unresolved dep? */
      if (uiDep != 0)
      {
         if (BVC5_P_ClientIsJobFinalized(hClient, uiDep))
         {
            /* If job has gone -- remove from dependency list */
            pJob->sRunDep_NotFinalized.uiDep[i] = 0;
         }
         else
         {
            /* There is still a dependency */
            hasNoDependencies = false;
         }
      }
   }

   if (hasNoDependencies)
      pJob->sRunDep_NotFinalized.uiNumDeps = 0;

   return hasNoDependencies;
}

/* Test if a job's dependencies have been resolved */
static bool BVC5_P_AreDepsResolved(
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *pJob
)
{
   return BVC5_P_AreDepsCompleted(hClient, pJob) &&
          BVC5_P_AreDepsFinalized(hClient, pJob);
}

/* Test if a job's finalizers have been run (this assumes the jobs have completed) */
static bool BVC5_P_AreDepsFinalizersDone(
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *pJob
)
{
   bool           hasNoDependencies = true;
   unsigned int   i;

   if (pJob->sFinDep_NotFinalized.uiNumDeps == 0)
      return true;

   for (i = 0; hasNoDependencies && i < pJob->sFinDep_NotFinalized.uiNumDeps; ++i)
   {
      uint64_t uiDep = pJob->sFinDep_NotFinalized.uiDep[i];

      /* Still has at least one unresolved finalizer? */
      if (uiDep != 0)
      {
         if (!BVC5_P_ClientIsJobFinishing(hClient, uiDep))
         {
            /* If job has gone -- remove from dependency list */
            pJob->sFinDep_NotFinalized.uiDep[i] = 0;
         }
         else
         {
            /* There is still a dependency */
            hasNoDependencies = false;
         }
      }
   }

   if (hasNoDependencies)
      pJob->sFinDep_NotFinalized.uiNumDeps = 0;

   return hasNoDependencies;
}

/* Test if a job's other prerequisites have been satisified */
static bool BVC5_P_IsRunnable(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *pInternalJob
)
{
   BVC5_JobBase       *pBaseJob     = pInternalJob->pBase;
   bool                isRunnable   = true;

   BSTD_UNUSED(hClient); /* May be needed for a "flush" job */

   switch (pBaseJob->eType)
   {
   case BVC5_JobType_eRender:
      /* Has the bin finished? */
      isRunnable = (pInternalJob->jobData.sRender.uiBinJobId == 0);
      break;

   case BVC5_JobType_eFenceWait:
      {
         /* Has the fence been signalled? */
         if (!pInternalJob->jobData.sWait.signaled)
         {
            int res;
            res = BVC5_P_FenceWaitAsyncIsSignaled(hVC5->hFences, pInternalJob->jobData.sWait.waitData);
            pInternalJob->jobData.sWait.signaled = (res == 1);
         }
         isRunnable = pInternalJob->jobData.sWait.signaled;
      }
      break;

   default:
      /* Nothing else to check */
      break;
   }

   return isRunnable;
}

/* BVC5_P_ProcessWaitQJob

   A job can be moved from the wait q to the runnable queue unless:
      there are still unresolved dependencies
      a render job is waiting for its bin job
      wait job's fence is not signalled

 */
static bool BVC5_P_ProcessWaitQJob(
   BVC5_Handle          hVC5,
   BVC5_P_InternalJob  *psJob
)
{
   bool                runnable = false;
   BVC5_ClientHandle   hClient  = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, psJob->uiClientId);

   BDBG_ASSERT(hClient != NULL);

   if (BVC5_P_AreDepsResolved(hClient, psJob) &&
       BVC5_P_IsRunnable(hVC5, hClient, psJob))
   {
      BDBG_MSG(("BVC5_P_ProcessWaitQJob jobID=%lld clientID=%d", psJob->uiJobId, psJob->uiClientId));

      runnable = true;
      BVC5_P_ClientJobWaitingToRunnable(hVC5, hClient, psJob);
   }

   return runnable;
}

/* BVC5_P_PumpClient

   Check all the jobs in the wait q to see if they have become runnable

 */
void BVC5_P_PumpClient(
   BVC5_Handle       hVC5,
   BVC5_ClientHandle hClient
)
{
   /* Take jobs from waitq to runnable if they are ready
      Then run all runnable jobs if they can be
    */
   BVC5_P_InternalJob *pJob;
   BVC5_P_InternalJob *pNextJob;
   bool                aBinWasNotRunnable = false;

   for (pJob = BVC5_P_JobQFirst(hClient->hWaitQ); pJob != NULL; pJob = pNextJob)
   {
      bool  isBin   = pJob->pBase->eType == BVC5_JobType_eBin;
      bool  process = true;

      if (isBin && aBinWasNotRunnable)
         process = false;

      pNextJob = BVC5_P_JobQNext(pJob);

      if (process)
      {
         bool  wasRunnable;

         wasRunnable = BVC5_P_ProcessWaitQJob(hVC5, pJob);

         if (isBin && !wasRunnable)
            aBinWasNotRunnable = true;
      }
   }

   BVC5_P_ScheduleRunnableJobs(hVC5);
   BVC5_P_ProcessCompletedJobs(hVC5, hClient);
}

void BVC5_P_PumpAllClients(
   BVC5_Handle       hVC5
)
{
   void                *pIter;
   BVC5_ClientHandle    hClient = NULL;

   for (hClient = BVC5_P_ClientMapFirst(hVC5->hClientMap, &pIter);
        hClient != NULL;
        hClient = BVC5_P_ClientMapNext(hVC5->hClientMap, &pIter))
   {
      BVC5_P_PumpClient(hVC5, hClient);
   }
}

/*****************************************************************************/

/* BVC5_Scheduler

   This is the main scheduler entry point.

 */
void BVC5_Scheduler(
   void *pSettings
)
{
   bool           terminate = false;
   BVC5_Handle    hVC5      = NULL;

   BVC5_SchedulerSettings *schedulerSettings = (BVC5_SchedulerSettings *)pSettings;

   BDBG_ENTER(BVC5_Scheduler);

   if (schedulerSettings             == NULL ||
       schedulerSettings->hVC5       == NULL ||
       schedulerSettings->hSyncEvent == NULL)
      goto exit;

   /* if the parameters were created on the stack frame, then they are only
      guaranteed to exist until the SetEvent on sync */
   hVC5 = schedulerSettings->hVC5;
   hVC5->hSchedulerSyncEvent = schedulerSettings->hSyncEvent;

   /* Set this event when an interrupt arrives or you need to kick the
      scheduler pump.
      */
   BKNI_CreateEvent(&hVC5->hSchedulerWakeEvent);

   /* Set this event to terminate the scheduler pump
      */
   BKNI_CreateEvent(&hVC5->hSchedulerTerminateEvent);

   hVC5->bSchedulerRunning = true;

   /* signal caller that it can now continue */
   BKNI_SetEvent(hVC5->hSchedulerSyncEvent);

   while (!terminate)
   {
      BERR_Code err;

      BDBG_MSG(("Scheduler waiting for wakeup"));
      err = BKNI_WaitForEvent(hVC5->hSchedulerWakeEvent, BVC5_WATCHDOG_TIMEOUT);
      BDBG_MSG(("Scheduler awoken"));

      /* Are we terminating? */
      terminate = BKNI_WaitForEvent(hVC5->hSchedulerTerminateEvent, 0) == BERR_SUCCESS;
      if (terminate) {
         /* This critical section works with another one in BVC5_Close()
          * After breaking from this loop two events are destroyed
          * so this critical sections make sure that both events are
          * signaled before they are destroyed */
         BKNI_EnterCriticalSection();
         BKNI_LeaveCriticalSection();
         break;
      }

      BKNI_AcquireMutex(hVC5->hModuleMutex);

      /* Process the event or timeout */
      if (err == BERR_TIMEOUT)
      {
         /* Timeout fired */
         BVC5_P_WatchdogTimeout(hVC5);
      }
      else if (err == BERR_SUCCESS)
      {
         /* We got an interrupt */
         BVC5_P_ProcessInterrupt(hVC5);
      }

      BVC5_P_PumpAllClients(hVC5);

      /* When enough events have happened try to reduce the bin memory footprint */
      hVC5->uiPurgeCounter++;
      if (hVC5->uiPurgeCounter >= BVC5_PURGE_LIMIT)
      {
         hVC5->uiPurgeCounter = 0;
         BVC5_P_BinPoolPurge(hVC5->hBinPool);
      }

      BKNI_ReleaseMutex(hVC5->hModuleMutex);
   }

   BKNI_DestroyEvent(hVC5->hSchedulerWakeEvent);
   BKNI_DestroyEvent(hVC5->hSchedulerTerminateEvent);

   /* signal caller that it can now continue */
   BKNI_SetEvent(hVC5->hSchedulerSyncEvent);

exit:
   BDBG_LEAVE(BVC5_Scheduler);
}

void BVC5_P_ProcessCompletedJobs(
   BVC5_Handle       hVC5,
   BVC5_ClientHandle hClient
)
{
   BVC5_P_InternalJob   *psIter;
   BVC5_P_InternalJob   *psNext;
   bool                 bIssueCallback = false;

   /* Move jobs to finalizable state if possible */
   for (psIter = BVC5_P_JobQFirst(hClient->hCompletedQ); psIter != NULL; psIter = psNext)
   {
      /* Remember this as entry may be removed */
      psNext = BVC5_P_JobQNext(psIter);

      if (BVC5_P_AreDepsFinalizersDone(hClient, psIter))
      {
         if (psIter->pBase->pfnCompletion == NULL)
         {
            /* If the job has no completion callback we can just move it into
             * the finalized state right away. */
            BVC5_P_ClientJobCompletedToFinalized(hVC5, hClient, psIter);
         }
         else
         {
            BVC5_P_ClientJobCompletedToFinalizable(hClient, psIter);
         }

         /* We need to issue the callback if there were any jobs transitioned to
          * finalizable or finalized. The callback tells the driver about the oldest
          * non-finalized job. We have to do this even if there are no jobs that
          * have actual pfnCompletion functions. */
         bIssueCallback = true;
      }
   }

   /* Callback to let layer above know there are new callbacks waiting (or to update
    * the oldest non-finalized job id) */
   if (bIssueCallback)
      hVC5->sCallbacks.fpCompletionHandler(hClient->pContext);
}

/* BVC5_P_WaitForClientJobComplete

   Wait for all jobs belonging to this client to be out of the hardware.

   MUST be called with hVC5->hModuleMutex already locked.

 */
void BVC5_P_WaitForJobCompletion(
   BVC5_Handle hVC5,
   uint32_t    uiClientId
)
{
   while (BVC5_P_HardwareCoreStateHasClient(hVC5, uiClientId))
   {
      /* We still have instructions active in hardware. Wait for them to complete. */
      BDBG_MSG(("Client died with active hardware jobs"));
      BKNI_ReleaseMutex(hVC5->hModuleMutex);
      BKNI_Sleep(1);
      BKNI_AcquireMutex(hVC5->hModuleMutex);
   }
}

/* BVC5_P_MarkJobsFlushedV3D

   Mark all jobs as not requiring V3D flushes

 */
void BVC5_P_MarkJobsFlushedV3D(
   BVC5_Handle hVC5
)
{
   void                *pIter;
   BVC5_ClientHandle    hClient = NULL;

   for (hClient = BVC5_P_ClientMapFirst(hVC5->hClientMap, &pIter);
        hClient != NULL;
        hClient = BVC5_P_ClientMapNext(hVC5->hClientMap, &pIter))
   {
      BVC5_P_ClientMarkJobsFlushedV3D(hClient);
   }
}
