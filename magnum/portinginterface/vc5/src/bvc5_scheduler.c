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
   uint32_t             uiCoreIndex = 0; /* TODO: multi-core */
   uint32_t             uiCapturedReason;
   uint32_t             uiTFUCapturedReason;
   BVC5_P_InternalJob  *pJob = NULL;

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
            BVC5_ClientHandle hClient = NULL;

            pJob = pState->psJob[BVC5_P_HW_QUEUE_RUNNING];
            if (pJob != NULL)
            {
#if INCLUDE_LEGACY_EVENT_TRACKS
               BVC5_P_AddCoreJobEvent(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_CORE_RENDER_TRACK,
                                      BVC5_P_EVENT_MONITOR_RENDERING, BVC5_EventEnd,
                                      pJob, BVC5_P_GetEventTimestamp(hVC5, uiCoreIndex));
#endif

               BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_RENDER_JOBS_COMPLETED, 1);

               BVC5_P_HardwareJobDone(hVC5, uiCoreIndex, BVC5_P_HardwareUnit_eRenderer);

               BVC5_P_BinMemArrayDestroy(&pJob->jobData.sRender.sBinMemArray);

               /* This job is done, so add to completed queue and remove from job state map */
               hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, pJob->uiClientId);
               BVC5_P_ClientJobRunningToCompleted(hVC5, hClient, pJob);
            }
         } while (__sync_sub_and_fetch(&pState->uiCapturedRFC, 1) != 0);
      }
   }

   /* Bin done? */
   if (BCHP_GET_FIELD_DATA(uiCapturedReason, V3D_CTL_INT_STS_INT, FLDONE))
   {
      BVC5_P_BinnerState  *pState  = BVC5_P_HardwareGetBinnerState(hVC5, uiCoreIndex);
      BVC5_ClientHandle    hClient = NULL;

      pJob = pState->psJob;
      if (pJob != NULL)
      {
#if INCLUDE_LEGACY_EVENT_TRACKS
         BVC5_P_AddCoreJobEvent(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_CORE_BIN_TRACK, BVC5_P_EVENT_MONITOR_BINNING,
                                 BVC5_EventEnd, pJob, BVC5_P_GetEventTimestamp(hVC5, uiCoreIndex));
#endif

         BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_BIN_JOBS_COMPLETED, 1);

         BVC5_P_HardwareJobDone(hVC5, uiCoreIndex, BVC5_P_HardwareUnit_eBinner);

         /* This job is done, so add to completed queue and remove from job state map */
         hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, pJob->uiClientId);
         BVC5_P_ClientJobRunningToCompleted(hVC5, hClient, pJob);

         /* Render job's dependency is resolved */
         pJob->jobData.sBin.psInternalRenderJob->jobData.sRender.uiBinJobId = 0;
      }
   }

   if (BCHP_GET_FIELD_DATA(uiCapturedReason, V3D_CTL_INT_STS_INT, OUTOMEM))
   {
      BVC5_P_BinnerState  *pState     = BVC5_P_HardwareGetBinnerState(hVC5, uiCoreIndex);

      pJob = pState->psJob;

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
#if INCLUDE_LEGACY_EVENT_TRACKS
         BVC5_P_AddCoreEvent(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_CORE_BIN_TRACK, pJob->uiJobId,
                             BVC5_P_EVENT_MONITOR_BOOM, BVC5_EventOneshot,
                             BVC5_P_GetEventTimestamp(hVC5, uiCoreIndex));
#endif

         BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_BIN_OOMS, 1);

         pBlock = BVC5_P_BinMemArrayAdd(&pRenderJob->jobData.sRender.sBinMemArray, 0, &uiPhysOffset);

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

         /* Translate to fixed MMU virtual mapping of Nexus heap */
         if (pJob->pBase->uiPagetablePhysAddr != 0)
            uiPhysOffset = BVC5_P_TranslateBinAddress(hVC5, uiPhysOffset, pJob->pBase->bSecure);

         /* TODO: which core for multi-core */
         BVC5_P_HardwareSupplyBinner(hVC5, uiCoreIndex, uiPhysOffset, pBlock->uiNumBytes);

         /* Now the hardware is happy, prepare some more memory for next time */
         BVC5_P_BinMemArrayReplenishPool(&pRenderJob->jobData.sRender.sBinMemArray);
      }
   }

   /* TFU */
   if (BCHP_GET_FIELD_DATA(uiTFUCapturedReason, V3D_TFU_TFUINT_STS_INT, TFUC))
   {
      BVC5_ClientHandle    hClient = NULL;
      BVC5_P_TFUState     *pState  = &hVC5->sTFUState;

      pJob = pState->psJob;
      if (pJob != NULL)
      {
         BVC5_P_AddTFUJobEvent(hVC5, BVC5_EventEnd, pJob,
                               BVC5_P_GetEventTimestamp(hVC5, uiCoreIndex));

         BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_TFU_JOBS_COMPLETED, 1);

         BVC5_P_HardwareJobDone(hVC5, 0, BVC5_P_HardwareUnit_eTFU);

         hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, pJob->uiClientId);
         BVC5_P_ClientJobRunningToCompleted(hVC5, hClient, pJob);
      }
   }

#if V3D_VER_AT_LEAST(3,3,0,0)
   /* Read the event counter fifos and log the events */
   BVC5_P_HardwareReadEventFifos(hVC5, uiCoreIndex);
#endif
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
                                     psCoreState->sBinnerState.psJob,
                                     BVC5_P_GetEventTimestamp(hVC5, uiCoreIndex));
            }
            if (uiStalled & BVC5_P_HardwareUnit_eRenderer)
            {
               BVC5_P_AddCoreEventCJ(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_SCHED_TRACK,
                                     BVC5_P_EVENT_MONITOR_RENDER_LOCKUP, BVC5_EventOneshot,
                                     psCoreState->sRenderState.psJob[0/* SAH TODO */],
                                     BVC5_P_GetEventTimestamp(hVC5, uiCoreIndex));
            }
            if (uiStalled & BVC5_P_HardwareUnit_eTFU)
            {
               /*
               BVC5_P_AddEvent(hVC5, BVC5_P_EVENT_MONITOR_SCHED_TRACK, 0,
                                 BVC5_P_EVENT_MONITOR_TFU_LOCKUP, BVC5_EventOneshot,
                                 BVC5_P_GetEventTimestamp(hVC5, uiCoreIndex));
               */
            }

            if (!hVC5->bLockupReported)
            {
               BVC5_P_DebugDump(hVC5, uiCoreIndex);

               if (hVC5->sOpenParams.bResetOnStall)
               {
                  /* Reset the core - sledgehammer time */
                  BVC5_P_HardwareResetCoreAndState(hVC5, uiCoreIndex);

                  /* Mark jobs as abandoned -- MUST be done after reset as the reset clears the interrupt reasons */
                  BVC5_P_HardwareAbandonJobs(hVC5, uiCoreIndex);

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

/* BVC5_P_ScheduleSoftJobsForClient

   Schedule i.e. run and complete, all jobs with only computational components.

 */
static bool BVC5_P_ScheduleSoftJobsForClient(
   BVC5_Handle       hVC5,
   BVC5_ClientHandle hClient
)
{
   bool                  bIssued = false;
   BVC5_JobQHandle       hSoftJobQ = hClient->hRunnableSoftQ;

   BVC5_P_InternalJob   *pJob = BVC5_P_JobQPop(hSoftJobQ);

   while (pJob != NULL)
   {
      /* Add to completed queue (for this client) and remove from map */
      BVC5_P_ClientJobRunningToCompleted(hVC5, hClient, pJob);

      bIssued = true;

      pJob = BVC5_P_JobQPop(hSoftJobQ);
   }

   return bIssued;
}

/* BVC5_P_CoreMatchPageTable

  Check the page table here and change if possible.
  Can only change if the cores are idle.
  The scheduler will ensure that each job gets a chance to set
  its page table and run its jobs by making sure that work
  that it wants to do (at least one bin, render and tfu job)
  is done before moving to the next client.

 */
static bool BVC5_P_CoreMatchPageTable(
   BVC5_Handle           hVC5,
   BVC5_P_InternalJob   *pJob
)
{
   BVC5_JobBase *pBase = pJob->pBase;

   if (hVC5->psCoreStates[0].uiBRCurrentPagetable != pBase->uiPagetablePhysAddr)
   {
      if (!BVC5_P_HardwareIsCoreIdle(hVC5, 0))
         return false;

      hVC5->psCoreStates[0].uiBRCurrentPagetable = pBase->uiPagetablePhysAddr;
   }

   BVC5_P_HardwareSetupCoreMmu(hVC5, 0, pBase->uiPagetablePhysAddr, pBase->uiMmuMaxVirtAddr);

   return true;
}

/* BVC5_P_CoreJobIsRunnable

   Check that the system is in the right state to run a bin/render job
   If possible, change state to accommodate new job

   POWER MUST BE ON
 */
static bool BVC5_P_CoreJobIsRunnable(
   BVC5_Handle         hVC5,
   BVC5_P_InternalJob *pJob
)
{
   /* Workaround for GFXH-1181 */
   if (!pJob->bFlushedV3D && BVC5_P_HardwareCacheClearBlocked(hVC5, 0))
      return false;

   if (!BVC5_P_SwitchSecurityMode(hVC5, pJob->pBase->bSecure))
      return false;

   if (!BVC5_P_CoreMatchPageTable(hVC5, pJob))
      return false;

   return true;
}

static bool BVC5_P_RenderJobIsRunnable(
   BVC5_Handle         hVC5,
   BVC5_P_InternalJob *pJob
)
{
   return BVC5_P_CoreJobIsRunnable(hVC5, pJob);
}

static bool BVC5_P_BinnerJobIsRunnable(
   BVC5_Handle         hVC5,
   BVC5_P_InternalJob *pJob
)
{
   return BVC5_P_CoreJobIsRunnable(hVC5, pJob) &&
          BVC5_P_BinPoolReplenish(BVC5_P_GetBinPool(hVC5));
}

/* BVC5_P_TFUMatchPageTable

  Change the TFU page table here -- will always be possible at
  present as we only have one TFU job running at a time and it
  has its own MMU.
 */
static bool BVC5_P_TFUMatchPageTable(
   BVC5_Handle           hVC5,
   BVC5_P_InternalJob   *pJob
)
{
   BVC5_JobBase *pBase = pJob->pBase;

   /* Currently the TFU has its own page table, so we can always swap */
   BVC5_P_HardwareSetupTfuMmu(hVC5, pBase->uiPagetablePhysAddr, pBase->uiMmuMaxVirtAddr);

   return true;
}

/* BVC5_P_TFUJobIsRunnable

   Check that the system is in the right state to run a TFU job
   If possible, change state to accommodate new job

   POWER MUST BE ON
 */
static bool BVC5_P_TFUJobIsRunnable(
   BVC5_Handle         hVC5,
   BVC5_P_InternalJob *pJob
)
{
   return BVC5_P_SwitchSecurityMode(hVC5, pJob->pBase->bSecure) &&
          BVC5_P_TFUMatchPageTable(hVC5, pJob);
}

/* BVC5_P_ScheduleRenderJobs

   Issue render job if available and runnable in the current mode
   Prefer the current client

   POWER MUST BE ON

 */
static void BVC5_P_ScheduleRenderJobs(
   BVC5_Handle        hVC5,
   uint32_t           uiNumClients
)
{
   BVC5_P_SchedulerState *psState = &hVC5->sSchedulerState;

   uint32_t uiClient;
   bool     bRendererAvailable = BVC5_P_HardwareIsRendererAvailable(hVC5, 0);

   /* Schedule render jobs */
   for (uiClient = 0; bRendererAvailable && uiClient < uiNumClients; ++uiClient)
   {
      BVC5_ClientHandle     hClient   = BVC5_P_SchedulerStateGetClient(psState, uiClient);
      BVC5_P_InternalJob   *renderJob = BVC5_P_JobQTop(hClient->hRunnableRenderQ);

      if (renderJob)
      {
         if (BVC5_P_RenderJobIsRunnable(hVC5, renderJob))
         {
            BVC5_P_HardwareIssueRenderJob(hVC5, 0, renderJob);
            BVC5_P_JobQPop(hClient->hRunnableRenderQ);
            BVC5_P_ClientSetGiven(hClient, BVC5_CLIENT_RENDER);
            bRendererAvailable = BVC5_P_HardwareIsRendererAvailable(hVC5, 0);
         }
         else
         {
            /* The current client wanted to render but was denied */
            if (uiClient == 0)
               break;
         }
      }
   }
}

/* BVC5_P_ScheduleBinnerJobs

   Issue binner job if available and runnable in the current mode
   Prefer the current client

   POWER MUST BE ON

 */
static void BVC5_P_ScheduleBinnerJobs(
   BVC5_Handle        hVC5,
   uint32_t           uiNumClients
)
{
   BVC5_P_SchedulerState *psState = &hVC5->sSchedulerState;

   uint32_t uiClient;
   bool     bBinnerAvailable = BVC5_P_HardwareIsBinnerAvailable(hVC5, 0);

   for (uiClient = 0; bBinnerAvailable && uiClient < uiNumClients; ++uiClient)
   {
      BVC5_ClientHandle     hClient = BVC5_P_SchedulerStateGetClient(psState, uiClient);
      BVC5_P_InternalJob   *binJob  = BVC5_P_JobQTop(hClient->hRunnableBinnerQ);

      if (binJob)
      {
         bool  bOutOfMemory = false;

         if (BVC5_P_BinnerJobIsRunnable(hVC5, binJob))
         {
            /* Binner job could fail due to out of binner memory.  We are on a sticky wicket in this case.
               We could a) wait for enough memory to become available for this client (potentially never) or
                        b) let other jobs have a chance to run and hope that this job gets memory in the future.
               For now we will do (b) it seems preferable to stall one client rather than many.
            */
            if (BVC5_P_HardwareIssueBinnerJob(hVC5, 0, binJob))
            {
               BVC5_P_JobQPop(hClient->hRunnableBinnerQ);
               BVC5_P_ClientSetGiven(hClient, BVC5_CLIENT_BIN);
               bBinnerAvailable = false;
            }
            else
            {
               bOutOfMemory = true;
            }
         }

         /* If the binner is still available, we didn't manage to issue a bin */
         if (bBinnerAvailable)
         {
            /* If we failed due to out of memory, then let the others have a go */
            if (uiClient == 0 && !bOutOfMemory)
               break;
         }
      }
   }
}

/* BVC5_P_ScheduleTFUJobs

   Issue TFU job if available and runnable in the current mode
   Prefer the current client

   POWER MUST BE ON

 */
static void BVC5_P_ScheduleTFUJobs(
   BVC5_Handle        hVC5,
   uint32_t           uiNumClients
)
{
   BVC5_P_SchedulerState *psState = &hVC5->sSchedulerState;

   uint32_t uiClient;
   bool     bTFUAvailable = BVC5_P_HardwareIsTFUAvailable(hVC5);

   for (uiClient = 0; bTFUAvailable && uiClient < uiNumClients; ++uiClient)
   {
      BVC5_ClientHandle     hClient = BVC5_P_SchedulerStateGetClient(psState, uiClient);
      BVC5_P_InternalJob   *tfuJob  = BVC5_P_JobQTop(hClient->hRunnableTFUQ);

      if (tfuJob)
      {
         if (BVC5_P_TFUJobIsRunnable(hVC5, tfuJob))
         {
            BVC5_P_HardwareIssueTFUJob(hVC5, tfuJob);
            BVC5_P_JobQPop(hClient->hRunnableTFUQ);
            BVC5_P_ClientSetGiven(hClient, BVC5_CLIENT_TFU);
            bTFUAvailable = false;
         }
         else
         {
            /* The current client wanted to do a TFU job but was denied */
            if (uiClient == 0)
               break;
         }
      }
   }
}

static void BVC5_P_ScheduleUsermodeJobs(
   BVC5_Handle        hVC5,
   uint32_t           uiNumClients,
   BVC5_ClientHandle *phClients
)
{
   uint32_t uiClient;
   bool     bUsermodeAvailable = BVC5_P_UsermodeIsAvailable(hVC5);

   /* Handle usermode jobs */
   for (uiClient = 0; bUsermodeAvailable && uiClient < uiNumClients; ++uiClient)
   {
      BVC5_ClientHandle     hClient     = phClients[uiClient];
      BVC5_P_InternalJob   *usermodeJob = BVC5_P_JobQTop(hClient->hRunnableUsermodeQ);

      if (usermodeJob)
      {
         BVC5_P_IssueUsermodeJob(hVC5, hClient, usermodeJob);
         BVC5_P_JobQPop(hClient->hRunnableUsermodeQ);
         bUsermodeAvailable = false;
      }
   }
}

static void BVC5_P_ScheduleSoftJobs(
   BVC5_Handle        hVC5,
   uint32_t           uiNumClients,
   BVC5_ClientHandle *phClients
)
{
   bool     issued = false;
   uint32_t uiClient;

   for (uiClient = 0; uiClient < uiNumClients; ++uiClient)
      issued = BVC5_P_ScheduleSoftJobsForClient(hVC5, phClients[uiClient]) || issued;

   /* If soft jobs were issued then it is possible we have more work to do */
   if (issued)
      BKNI_SetEvent(hVC5->hSchedulerWakeEvent);
}

/* BVC5_P_HaveHardJobs
 */
static bool BVC5_P_HaveHardJobs(
   uint32_t           uiNumClients,
   BVC5_ClientHandle *phClients
)
{
   uint32_t             uiClient;
   bool                 bHaveHardJobs = false;

   for (uiClient = 0; !bHaveHardJobs && uiClient < uiNumClients; ++uiClient)
   {
      BVC5_ClientHandle hClient = phClients[uiClient];

      if (BVC5_P_ClientHasHardJobs(hClient))
         bHaveHardJobs = true;
   }

   return bHaveHardJobs;
}

/* BVC5_P_ScheduleRunnableJobs

   Schedules as many jobs as possible from the client's runnable queues.

 */
static void BVC5_P_ScheduleRunnableJobs(
   BVC5_Handle        hVC5,
   uint32_t           uiNumClients,
   BVC5_ClientHandle *phClients
)
{
   if (!hVC5->bEnterStandby && BVC5_P_HaveHardJobs(uiNumClients, phClients))
   {
      /* Power is needed to change modes for security etc. and to issue work */
      if (!hVC5->sSchedulerState.bHoldingPower)
      {
         BVC5_P_HardwarePowerAcquire(hVC5, 1 << 0);
         hVC5->sSchedulerState.bHoldingPower = true;
      }

      BVC5_P_SchedulerStateSetClientWanted(&hVC5->sSchedulerState);

      BVC5_P_ScheduleRenderJobs(hVC5, uiNumClients);
      BVC5_P_ScheduleBinnerJobs(hVC5, uiNumClients);
      BVC5_P_ScheduleTFUJobs   (hVC5, uiNumClients);

      /* If we did the work for the favoured client, give someone else a go */
      BVC5_P_SchedulerStateNextClient(&hVC5->sSchedulerState);

      /* If nothing was run then turn off again */
      if (!BVC5_P_HaveHardJobs(uiNumClients, phClients))
      {
         BVC5_P_HardwarePowerRelease(hVC5, 1 << 0);
         hVC5->sSchedulerState.bHoldingPower = false;
      }
   }

   BVC5_P_ScheduleUsermodeJobs(hVC5, uiNumClients, phClients);

   BVC5_P_ScheduleSoftJobs(hVC5, uiNumClients, phClients);
}

static bool BVC5_P_AreDepsCompleted(
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *pJob
)
{
   bool           bHasNoDependencies = true;   /* Assume we have none until we find one */
   unsigned int   i;

   /* Early exit if all deps satisfied */
   if (pJob->sRunDep_NotCompleted.uiNumDeps == 0)
      return true;

   for (i = 0; bHasNoDependencies && i < pJob->sRunDep_NotCompleted.uiNumDeps; ++i)
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
            bHasNoDependencies = false;
         }
      }
   }

   if (bHasNoDependencies)
      pJob->sRunDep_NotCompleted.uiNumDeps = 0;

   return bHasNoDependencies;
}

static bool BVC5_P_AreDepsFinalized(
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *pJob
)
{
   bool           bHasNoDependencies = true;   /* Assume we have none until we find one */
   unsigned int   i;

   /* Early exit if all deps satisfied */
   if (pJob->sRunDep_NotFinalized.uiNumDeps == 0)
      return true;

   for (i = 0; bHasNoDependencies && i < pJob->sRunDep_NotFinalized.uiNumDeps; ++i)
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
            bHasNoDependencies = false;
         }
      }
   }

   if (bHasNoDependencies)
      pJob->sRunDep_NotFinalized.uiNumDeps = 0;

   return bHasNoDependencies;
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
   bool           bHasNoDependencies = true;
   unsigned int   i;

   if (pJob->sFinDep_NotFinalized.uiNumDeps == 0)
      return true;

   for (i = 0; bHasNoDependencies && i < pJob->sFinDep_NotFinalized.uiNumDeps; ++i)
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
            bHasNoDependencies = false;
         }
      }
   }

   if (bHasNoDependencies)
      pJob->sFinDep_NotFinalized.uiNumDeps = 0;

   return bHasNoDependencies;
}

/* Test if a job's other prerequisites have been satisified */
static bool BVC5_P_IsRunnable(
   BVC5_Handle          hVC5,
   BVC5_P_InternalJob  *pInternalJob
)
{
   BVC5_JobBase  *pBaseJob    = pInternalJob->pBase;
   bool           bIsRunnable = true;

   switch (pBaseJob->eType)
   {
   case BVC5_JobType_eRender:
      /* Has the bin finished? */
      bIsRunnable = (pInternalJob->jobData.sRender.uiBinJobId == 0);
      break;

   case BVC5_JobType_eFenceWait:
      /* Has the fence been signalled? */
      if (!pInternalJob->jobData.sWait.signaled)
      {
         int res;
         res = BVC5_P_FenceWaitAsyncIsSignaled(hVC5->hFences, pInternalJob->jobData.sWait.waitData);
         pInternalJob->jobData.sWait.signaled = (res == 1);
      }

      bIsRunnable = pInternalJob->jobData.sWait.signaled;
      break;

   default:
      /* Nothing else to check */
      break;
   }

   return bIsRunnable;
}

/* BVC5_P_ProcessWaitQJob

   A job can be moved from the wait q to the runnable queue unless:
      there are still unresolved dependencies
      a render job is waiting for its bin job
      wait job's fence is not signalled

 */
static bool BVC5_P_ProcessWaitQJob(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
)
{
   bool  bRunnable = false;

   BDBG_ASSERT(hClient != NULL);

   if (BVC5_P_AreDepsResolved(hClient, psJob) &&
       BVC5_P_IsRunnable(hVC5, psJob))
   {
      BDBG_MSG(("BVC5_P_ProcessWaitQJob jobID="BDBG_UINT64_FMT" clientID=%d", BDBG_UINT64_ARG(psJob->uiJobId), psJob->uiClientId));

      bRunnable = true;

      BVC5_P_ClientJobWaitingToRunnable(hClient, psJob);
   }

   return bRunnable;
}

/* BVC5_P_ProcessWaitingJobs

   Check all the jobs in the wait q to see if they have become runnable

 */
static void BVC5_P_ProcessWaitingJobs(
   BVC5_Handle        hVC5,
   uint32_t           uiNumClients,
   BVC5_ClientHandle *phClients
)
{
   uint32_t uiClient;

   for (uiClient = 0; uiClient < uiNumClients; ++uiClient)
   {
      /* Take jobs from waitq to runnable if they are ready
       */
      BVC5_ClientHandle   hClient = phClients[uiClient];

      BVC5_P_InternalJob *pJob;
      BVC5_P_InternalJob *pNextJob;
      bool                bABinWasNotRunnable = false;

      for (pJob = BVC5_P_JobQFirst(hClient->hWaitQ); pJob != NULL; pJob = pNextJob)
      {
         bool  bIsBin   = pJob->pBase->eType == BVC5_JobType_eBin;
         bool  bProcess = true;

         if (bIsBin && bABinWasNotRunnable)
            bProcess = false;

         pNextJob = BVC5_P_JobQNext(pJob);

         if (bProcess)
         {
            bool  bWasRunnable = BVC5_P_ProcessWaitQJob(hVC5, hClient, pJob);

            if (bIsBin && !bWasRunnable)
               bABinWasNotRunnable = true;
         }
      }
   }
}

void BVC5_P_ProcessCompletedJobs(
   BVC5_Handle        hVC5,
   uint32_t           uiNumClients,
   BVC5_ClientHandle *phClients
)
{
   uint32_t uiClient;

   for (uiClient = 0; uiClient < uiNumClients; ++uiClient)
   {
      BVC5_ClientHandle    hClient = phClients[uiClient];
      BVC5_P_InternalJob  *psIter;
      BVC5_P_InternalJob  *psNext;
      bool                 bIssueCallback = false;

      /* Move jobs to finalizable state if possible */
      for (psIter = BVC5_P_JobQFirst(hClient->hCompletedQ); psIter != NULL; psIter = psNext)
      {
         /* Remember this as entry may be removed */
         psNext = BVC5_P_JobQNext(psIter);

         if (BVC5_P_AreDepsFinalizersDone(hClient, psIter))
         {
            if (psIter->pBase->uiCompletion == 0)
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
}

void BVC5_P_PumpAllClients(
   BVC5_Handle       hVC5
)
{
   uint32_t uiNumClients = BVC5_P_ClientMapSize(hVC5->hClientMap);

   if (uiNumClients > 0)
   {
      BVC5_ClientHandle *phClients = BVC5_P_SchedulerStateGatherClients(&hVC5->sSchedulerState);

      BVC5_P_ProcessWaitingJobs  (hVC5, uiNumClients, phClients);
      BVC5_P_ScheduleRunnableJobs(hVC5, uiNumClients, phClients);
      BVC5_P_ProcessCompletedJobs(hVC5, uiNumClients, phClients);
   }
}

void BVC5_P_SchedulerPump(
   BVC5_Handle hVC5
)
{
   BVC5_P_PumpAllClients(hVC5);
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
         /* We (maybe) got an interrupt */
         BVC5_P_ProcessInterrupt(hVC5);
      }

      BVC5_P_PumpAllClients(hVC5);

      /* When enough events have happened try to reduce the bin memory footprint */
      hVC5->uiPurgeCounter++;
      if (hVC5->uiPurgeCounter >= BVC5_PURGE_LIMIT)
      {
         hVC5->uiPurgeCounter = 0;
         BVC5_P_BinPoolPurge(hVC5->hBinPool);
         BVC5_P_BinPoolPurge(hVC5->hSecureBinPool);
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
