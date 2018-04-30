/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bstd.h"
#include "bv3d.h"
#include "bv3d_priv.h"
#include "bv3d_worker_priv.h"
#include "bchp_v3d_pctr.h"
#include "bstd.h"
#include "bmma_system.h"
#include "bv3d_perf_counters_priv.h"

BDBG_MODULE(BV3D);

static void _strncpy(char *d, const char *s, uint32_t maxLen)
{
   uint32_t i;
   for (i = 0; i < maxLen - 1 && *s != 0; i++)
      *d++ = *s++;

   *d = '\0';
}

static void BV3D_P_SetCounterNamesEx(
   BV3D_CounterGroupDesc   *grp,
   uint64_t                index,
   const char              *name,
   const char              *unit,
   uint64_t                minVal,
   uint64_t                maxVal,
   uint64_t                denom
   )
{
   _strncpy(grp->saCounters[index].caName, name, BV3D_MAX_COUNTER_NAME_LEN);
   _strncpy(grp->saCounters[index].caUnitName, unit, BV3D_MAX_COUNTER_NAME_LEN);

   if (index >= grp->uiTotalCounters)
      grp->uiTotalCounters = index + 1;

   grp->saCounters[index].uiMinValue = minVal;
   grp->saCounters[index].uiMaxValue = maxVal;
   grp->saCounters[index].uiDenominator = denom;
}

static void BV3D_P_SetCounterNames(
   BV3D_CounterGroupDesc   *grp,
   uint64_t                index,
   const char              *name,
   const char              *unit
   )
{
   BV3D_P_SetCounterNamesEx(grp, index, name, unit, 0, ~0, 1);
}

/***************************************************************************/
void BV3D_P_InitPerfCounters(
   BV3D_Handle  hV3d
   )
{
   BV3D_CounterGroupDesc *d = &hV3d->sPerfCounters.sGroupDescs[BV3D_P_PERF_COUNTER_HW_GROUP];
   uint64_t              now;

   BKNI_Memset(&hV3d->sPerfCounters, 0, sizeof(hV3d->sPerfCounters));

   _strncpy(d->caName, "Hardware Counters", BV3D_MAX_GROUP_NAME_LEN);

   d->uiMaxActiveCounters = BV3D_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE;

   #include "bv3d_perf_counters_hw_2_x.inc"

   d = &hV3d->sPerfCounters.sGroupDescs[BV3D_P_PERF_COUNTER_SCHED_GROUP];

   _strncpy(d->caName, "Scheduler Counters", BV3D_MAX_GROUP_NAME_LEN);

   BV3D_P_SetCounterNames(d, BV3D_P_PERF_BIN_JOBS_SUBMITTED,      "bin_jobs_submitted", "jobs");
   BV3D_P_SetCounterNames(d, BV3D_P_PERF_BIN_JOBS_COMPLETED,      "bin_jobs_completed", "jobs");
   BV3D_P_SetCounterNames(d, BV3D_P_PERF_RENDER_JOBS_SUBMITTED,   "render_jobs_submitted", "jobs");
   BV3D_P_SetCounterNames(d, BV3D_P_PERF_RENDER_JOBS_COMPLETED,   "render_jobs_completed", "jobs");
   BV3D_P_SetCounterNames(d, BV3D_P_PERF_USER_JOBS_SUBMITTED,     "user_jobs_submitted", "jobs");
   BV3D_P_SetCounterNames(d, BV3D_P_PERF_USER_JOBS_COMPLETED,     "user_jobs_completed", "jobs");
   BV3D_P_SetCounterNames(d, BV3D_P_PERF_INTERRUPTS,              "interrupts", "interrupts");
   BV3D_P_SetCounterNames(d, BV3D_P_PERF_BIN_OOMS,                "bin_ooms", "booms");
   BV3D_P_SetCounterNamesEx(d, BV3D_P_PERF_BIN_MEMORY_RESERVED,   "bin_memory_reserved", "MB", 0, 1024, 1024 * 1024);
   BV3D_P_SetCounterNamesEx(d, BV3D_P_PERF_BIN_MEMORY_IN_USE,     "bin_memory_in_use", "MB", 0, 1024, 1024 * 1024);
   BV3D_P_SetCounterNames(d, BV3D_P_PERF_LOCKUP_DETECTION,        "lockup_detection", "lockups");
   BV3D_P_SetCounterNamesEx(d, BV3D_P_PERF_BINNER_LOAD_AVG,       "binner_load_avg", "%", 0, 100, 1);
   BV3D_P_SetCounterNamesEx(d, BV3D_P_PERF_RENDERER_LOAD_AVG,     "renderer_load_avg", "%", 0, 100, 1);
   BV3D_P_SetCounterNamesEx(d, BV3D_P_PERF_USER_LOAD_AVG,         "user_load_avg", "%", 0, 100, 1);
   BV3D_P_SetCounterNamesEx(d, BV3D_P_PERF_POWERED_ON_AVG,        "powered_on_avg", "%", 0, 100, 1);
   BV3D_P_SetCounterNamesEx(d, BV3D_P_PERF_EVENT_MEMORY_RESERVED, "knl_event_memory_reserved", "KB", 0, 1024 * 32, 1024);
   BV3D_P_SetCounterNamesEx(d, BV3D_P_PERF_EVENT_MEMORY_IN_USE,   "knl_event_memory_in_use", "KB", 0, 1024 * 32, 1024);
   BV3D_P_SetCounterNamesEx(d, BV3D_P_PERF_HEAP_MEMORY_IN_USE,    "device_heap_memory_in_use", "MB", 0, 1024, 1024 * 1024);
   BV3D_P_SetCounterNamesEx(d, BV3D_P_PERF_HEAP_MEMORY_FREE,      "device_heap_memory_free", "MB", 0, 1024, 1024 * 1024);

   d->uiMaxActiveCounters = BV3D_P_PERF_COUNTER_MAX_SCHED_CTRS_ACTIVE;

   BV3D_P_GetTime_isrsafe(&now);
   hV3d->sPerfCounters.uiLastCollectionTime = now;
   hV3d->sPerfCounters.uiBinnerIdleStartTime = now;
   hV3d->sPerfCounters.uiRendererIdleStartTime = now;
   hV3d->sPerfCounters.uiUserIdleStartTime = now;
}

void BV3D_GetPerfNumCounterGroups(
   BV3D_Handle  hV3d,
   uint32_t     *puiNumGroups
   )
{
   BDBG_ENTER(BV3D_GetPerfNumCounterGroups);
   BKNI_AcquireMutex(hV3d->hModuleMutex);

   BSTD_UNUSED(hV3d);

   if (puiNumGroups != NULL)
      *puiNumGroups = BV3D_P_PERF_COUNTER_NUM_GROUPS;

   BKNI_ReleaseMutex(hV3d->hModuleMutex);
   BDBG_LEAVE(BV3D_GetPerfNumCounterGroups);
}

void BV3D_GetPerfCounterDesc(
   BV3D_Handle             hV3d,
   uint32_t                uiGroup,
   uint32_t                uiCounter,
   BV3D_CounterDesc        *psDesc
   )
{
   BDBG_ENTER(BV3D_GetPerfCounterDesc);
   BKNI_AcquireMutex(hV3d->hModuleMutex);

   if (psDesc != NULL &&
         uiGroup < BV3D_P_PERF_COUNTER_NUM_GROUPS &&
         uiCounter < BV3D_MAX_COUNTERS_PER_GROUP)
      *psDesc = hV3d->sPerfCounters.sGroupDescs[uiGroup].saCounters[uiCounter];

   BKNI_ReleaseMutex(hV3d->hModuleMutex);
   BDBG_LEAVE(BV3D_GetPerfCounterDesc);
}

void BV3D_GetPerfCounterGroupInfo(
   BV3D_Handle             hV3d,
   uint32_t                uiGroup,
   uint32_t                uiGrpNameSize,
   char                    *chGrpName,
   uint32_t                *uiMaxActiveCounter,
   uint32_t                *uiTotalCounter
   )
{
   BDBG_ENTER(BV3D_GetPerfCounterGroupInfo);
   BKNI_AcquireMutex(hV3d->hModuleMutex);

   if (chGrpName != NULL &&
         uiMaxActiveCounter != NULL &&
         uiTotalCounter != NULL &&
         uiGroup < BV3D_P_PERF_COUNTER_NUM_GROUPS)
   {
      _strncpy(chGrpName, hV3d->sPerfCounters.sGroupDescs[uiGroup].caName, uiGrpNameSize);
      *uiMaxActiveCounter = hV3d->sPerfCounters.sGroupDescs[uiGroup].uiMaxActiveCounters;
      *uiTotalCounter = hV3d->sPerfCounters.sGroupDescs[uiGroup].uiTotalCounters;
   }

   BKNI_ReleaseMutex(hV3d->hModuleMutex);
   BDBG_LEAVE(BV3D_GetPerfCounterGroupInfo);
}

static BERR_Code BV3D_P_SetPerfCounting(
   BV3D_Handle             hV3d,
   uint32_t                uiClientId,
   BV3D_CounterState       eState
   )
{
   BV3D_P_PerfCounters  *psPerf;
   uint32_t             i;
   BERR_Code            err = BERR_SUCCESS;

   psPerf = &hV3d->sPerfCounters;

   switch (eState)
   {
   case BV3D_CtrAcquire:
      if (psPerf->bAcquired)
      {
         if (psPerf->uiClientId != uiClientId)
         {
            /* TODO - check that the current client is still alive */

            err = BERR_NOT_AVAILABLE;
            goto error;
         }
      }
      else
      {
         psPerf->bAcquired = true;
         psPerf->uiClientId = uiClientId;
      }
      break;
   case BV3D_CtrRelease:
      if (!psPerf->bAcquired || psPerf->uiClientId != uiClientId)
      {
         err = BERR_INVALID_PARAMETER;
         goto error;
      }
      else
      {
         psPerf->bAcquired = false;
         psPerf->uiClientId = 0;
      }
      break;
   case BV3D_CtrStart:
   {
      if (psPerf->bAcquired && psPerf->uiClientId == uiClientId)
      {
         uint32_t c = 0;
         psPerf->bCountersActive = true;
         for (i = 0; i < psPerf->sGroupDescs[BV3D_P_PERF_COUNTER_HW_GROUP].uiTotalCounters; i++)
         {
            /* TODO - multi-core */
            if (psPerf->sHwCounters[i].bEnabled)
            {
               psPerf->uiPCTRSShadow[c] = i;
               c++;
            }
         }

         BDBG_ASSERT(c == psPerf->uiActiveHwCounters);
         psPerf->uiPCTREShadow = BCHP_FIELD_DATA(V3D_PCTR_PCTRE, CTEN, 1) | ((1 << psPerf->uiActiveHwCounters) - 1);

         /* update the counters, but only if the core is on - otherwise they'll get set at next power on */
         BV3D_P_RestorePerfCounters(hV3d, /*bWriteSelectorsAndEnables=*/true);
      }
      else
         err = BERR_NOT_AVAILABLE;
      break;
   }
   case BV3D_CtrStop:
      if (psPerf->bAcquired && psPerf->uiClientId == uiClientId)
      {
         psPerf->bCountersActive = false;
         psPerf->uiPCTREShadow = 0;
      }
      else
         err = BERR_NOT_AVAILABLE;
      break;
   }

error:
   return err;
}

BERR_Code BV3D_SetPerfCounting(
   BV3D_Handle             hV3d,
   uint32_t                uiClientId,
   BV3D_CounterState       eState
   )
{
   BERR_Code err;

   BDBG_ENTER(BV3D_SetPerfCounting);
   BKNI_AcquireMutex(hV3d->hModuleMutex);

   err = BV3D_P_SetPerfCounting(hV3d, uiClientId, eState);

   BKNI_ReleaseMutex(hV3d->hModuleMutex);
   BDBG_LEAVE(BV3D_SetPerfCounting);

   return err;
}

void BV3D_ChoosePerfCounters(
   BV3D_Handle                hV3d,
   uint32_t                   uiClientId,
   const BV3D_CounterSelector *psSelector
   )
{
   uint32_t i;
   BDBG_ENTER(BV3D_ChoosePerfCounters);

   BKNI_AcquireMutex(hV3d->hModuleMutex);

   if (!hV3d->sPerfCounters.bAcquired || hV3d->sPerfCounters.uiClientId != uiClientId)
      goto error;

   if (psSelector->uiGroupIndex == BV3D_P_PERF_COUNTER_HW_GROUP)
   {
      for (i = 0; i < psSelector->uiNumCounters; i++)
      {
         BDBG_ASSERT(psSelector->uiaCounters[i] < hV3d->sPerfCounters.sGroupDescs[BV3D_P_PERF_COUNTER_HW_GROUP].uiTotalCounters);

         if (psSelector->uiEnable && hV3d->sPerfCounters.uiActiveHwCounters < BV3D_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE)
         {
            if (!hV3d->sPerfCounters.sHwCounters[psSelector->uiaCounters[i]].bEnabled)
               hV3d->sPerfCounters.uiActiveHwCounters++;

            hV3d->sPerfCounters.sHwCounters[psSelector->uiaCounters[i]].bEnabled = true;
         }
         else if (!psSelector->uiEnable)
         {
            if (hV3d->sPerfCounters.sHwCounters[psSelector->uiaCounters[i]].bEnabled)
               hV3d->sPerfCounters.uiActiveHwCounters--;
            hV3d->sPerfCounters.sHwCounters[psSelector->uiaCounters[i]].bEnabled = false;
         }
      }

      BDBG_ASSERT(hV3d->sPerfCounters.uiActiveHwCounters <= BV3D_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE);

      /* Clear the counters */
      BKNI_Memset(hV3d->sPerfCounters.uiPCTRShadows, 0, sizeof(hV3d->sPerfCounters.uiPCTRShadows));
   }
   else if (psSelector->uiGroupIndex == BV3D_P_PERF_COUNTER_SCHED_GROUP)
   {
      BDBG_ASSERT(psSelector->uiNumCounters <= BV3D_P_PERF_COUNTER_MAX_SCHED_CTRS_ACTIVE);

      for (i = 0; i < psSelector->uiNumCounters; i++)
      {
         BDBG_ASSERT(psSelector->uiaCounters[i] < hV3d->sPerfCounters.sGroupDescs[BV3D_P_PERF_COUNTER_SCHED_GROUP].uiTotalCounters);
         hV3d->sPerfCounters.sSchedValues[psSelector->uiaCounters[i]].bEnabled = psSelector->uiEnable;
         hV3d->sPerfCounters.sSchedValues[psSelector->uiaCounters[i]].uiValue  = 0;
      }
   }

error:
   BKNI_ReleaseMutex(hV3d->hModuleMutex);
   BDBG_LEAVE(BV3D_ChoosePerfCounters);
}

/* Must be called with counter mutex held */
static void BV3D_P_UpdateCalculatedCounters(
   BV3D_Handle hV3d,
   uint32_t    uiResetCounts
   )
{
   size_t   capacity;
   size_t   used;
   uint64_t timeSinceLast;
   uint64_t now;

   BV3D_P_GetTime_isrsafe(&now);

   BV3D_P_BinPoolStats(hV3d->bSecure ? hV3d->hBinMemManagerSecure : hV3d->hBinMemManager, &capacity, &used);

   if (hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_BIN_MEMORY_RESERVED].bEnabled)
      hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_BIN_MEMORY_RESERVED].uiValue = capacity;

   if (hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_BIN_MEMORY_IN_USE].bEnabled)
      hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_BIN_MEMORY_IN_USE].uiValue   = used;

   BV3D_P_EventMemStats(hV3d, &capacity, &used);

   if (hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_EVENT_MEMORY_RESERVED].bEnabled)
      hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_EVENT_MEMORY_RESERVED].uiValue = capacity;

   if (hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_EVENT_MEMORY_IN_USE].bEnabled)
      hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_EVENT_MEMORY_IN_USE].uiValue = used;

   timeSinceLast = now - hV3d->sPerfCounters.uiLastCollectionTime;

   /* Binner load avg */
   if (hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_BINNER_LOAD_AVG].bEnabled)
   {
      uint64_t idlePct = 100;
      uint64_t idleTime = hV3d->sPerfCounters.uiBinnerCumIdleTime;

      if (hV3d->sPerfCounters.uiBinnerIdleStartTime != 0)
         idleTime += (now - hV3d->sPerfCounters.uiBinnerIdleStartTime);

      if (timeSinceLast > 0)
         idlePct = (100 * idleTime) / timeSinceLast;

      hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_BINNER_LOAD_AVG].uiValue = 100 - idlePct;

      if (uiResetCounts)
      {
         hV3d->sPerfCounters.uiBinnerCumIdleTime = 0;
         if (hV3d->sPerfCounters.uiBinnerIdleStartTime != 0)
            hV3d->sPerfCounters.uiBinnerIdleStartTime = now;
      }
   }

   /* Render load avg */
   if (hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_RENDERER_LOAD_AVG].bEnabled)
   {
      uint64_t idlePct = 100;
      uint64_t idleTime = hV3d->sPerfCounters.uiRendererCumIdleTime;

      if (hV3d->sPerfCounters.uiRendererIdleStartTime != 0)
         idleTime += (now - hV3d->sPerfCounters.uiRendererIdleStartTime);

      if (timeSinceLast > 0)
         idlePct = (100 * idleTime) / timeSinceLast;

      hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_RENDERER_LOAD_AVG].uiValue = 100 - idlePct;

      if (uiResetCounts)
      {
         hV3d->sPerfCounters.uiRendererCumIdleTime = 0;
         if (hV3d->sPerfCounters.uiRendererIdleStartTime != 0)
            hV3d->sPerfCounters.uiRendererIdleStartTime = now;
      }
   }

   /* User load avg */
   if (hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_USER_LOAD_AVG].bEnabled)
   {
      uint64_t idlePct = 100;
      uint64_t idleTime = hV3d->sPerfCounters.uiUserCumIdleTime;

      if (hV3d->sPerfCounters.uiUserIdleStartTime != 0)
         idleTime += (now - hV3d->sPerfCounters.uiUserIdleStartTime);

      if (timeSinceLast > 0)
         idlePct = (100 * idleTime) / timeSinceLast;

      hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_USER_LOAD_AVG].uiValue = 100 - idlePct;

      if (uiResetCounts)
      {
         hV3d->sPerfCounters.uiUserCumIdleTime = 0;
         if (hV3d->sPerfCounters.uiUserIdleStartTime != 0)
            hV3d->sPerfCounters.uiUserIdleStartTime = now;
      }
   }

   /* Powered-on avg */
   if (hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_POWERED_ON_AVG].bEnabled)
   {
      uint64_t offPct = 100;
      uint64_t offTime = hV3d->sPerfCounters.uiPoweredCumOffTime;

      if (hV3d->sPerfCounters.uiPoweredOffStartTime != 0)
         offTime += (now - hV3d->sPerfCounters.uiPoweredOffStartTime);

      if (timeSinceLast > 0)
         offPct = (100 * offTime) / timeSinceLast;

      hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_POWERED_ON_AVG].uiValue = 100 - offPct;

      if (uiResetCounts)
      {
         hV3d->sPerfCounters.uiPoweredCumOffTime = 0;
         if (hV3d->sPerfCounters.uiPoweredOffStartTime != 0)
            hV3d->sPerfCounters.uiPoweredOffStartTime = now;
      }
   }

#if !BMMA_USE_STUB
   /* Device memory usage */
   {
      BMMA_Heap_FastStatus memStats;

      BMMA_Heap_GetStatus(hV3d->bSecure ? hV3d->hMmaSecure : hV3d->hMma, &memStats, NULL);

      if (hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_HEAP_MEMORY_IN_USE].bEnabled)
         hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_HEAP_MEMORY_IN_USE].uiValue = memStats.totalAllocated;

      if (hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_HEAP_MEMORY_FREE].bEnabled)
         hV3d->sPerfCounters.sSchedValues[BV3D_P_PERF_HEAP_MEMORY_FREE].uiValue = memStats.totalFree;
   }
#endif

   if (uiResetCounts)
      hV3d->sPerfCounters.uiLastCollectionTime = now;
}

uint32_t BV3D_GetPerfCounterData(
   BV3D_Handle    hV3d,
   uint32_t       uiMaxCounters,
   uint32_t       uiResetCounts,
   BV3D_Counter   *psCounters
   )
{
   BV3D_P_PerfCounters  *psPerf;
   uint32_t             ret = 0;
   uint32_t             i;

   BDBG_ENTER(BV3D_GetPerfCounterData);

   BKNI_AcquireMutex(hV3d->hModuleMutex);

   if (!hV3d->sPerfCounters.bAcquired)
      goto error;

   psPerf = &hV3d->sPerfCounters;

   if (uiMaxCounters == 0 || psCounters == NULL)
   {
      /* Get number of counters */
      uint32_t counters = psPerf->uiActiveHwCounters;

      for (i = 0; i < BV3D_P_PERF_COUNTER_MAX_SCHED_CTRS_ACTIVE; i++)
      {
         if (psPerf->sSchedValues[i].bEnabled)
            counters++;
      }

      ret = counters;
   }
   else
   {
      /* Get the counter values */
      uint32_t c = 0;

      BV3D_P_UpdateShadowCounters(hV3d);

      for (i = 0; i < BV3D_MAX_COUNTERS_PER_GROUP; i++)
      {
         if (psPerf->sHwCounters[i].bEnabled)
         {
            psCounters[c].uiGroupIndex = BV3D_P_PERF_COUNTER_HW_GROUP;
            psCounters[c].uiCounterIndex = i;
            /* TODO - multi core */
            psCounters[c].uiValue = psPerf->uiPCTRShadows[c];

            c++;
         }

         if (c >= psPerf->uiActiveHwCounters || c >= uiMaxCounters)
            break;
      }

      /* Scheduler counters */
      BV3D_P_UpdateCalculatedCounters(hV3d, uiResetCounts);

      for (i = 0; i < BV3D_P_PERF_COUNTER_MAX_SCHED_CTRS_ACTIVE; i++)
      {
         if (c < uiMaxCounters)
         {
            if (psPerf->sSchedValues[i].bEnabled)
            {
               psCounters[c].uiGroupIndex = BV3D_P_PERF_COUNTER_SCHED_GROUP;
               psCounters[c].uiCounterIndex = i;
               psCounters[c].uiValue = psPerf->sSchedValues[i].uiValue;
               c++;
            }
         }
         else
            break;
      }

      ret = c;
   }

   if (uiResetCounts)
   {
      /* Reset the PCTR registers to 0 */
      BV3D_P_RestorePerfCounters(hV3d, /*bWriteSelectorsAndEnables=*/false);

      /* TODO multi core */
      BKNI_Memset(psPerf->uiPCTRShadows, 0, sizeof(psPerf->uiPCTRShadows));

      for (i = 0; i < BV3D_P_PERF_COUNTER_MAX_SCHED_CTRS_ACTIVE; i++)
         psPerf->sSchedValues[i].uiValue = 0;
   }

error:
   BKNI_ReleaseMutex(hV3d->hModuleMutex);
   BDBG_LEAVE(BV3D_GetPerfCounterData);

   return ret;
}

void BV3D_P_SchedPerfCounterAdd(
   BV3D_Handle    hV3d,
   uint32_t       uiCtr,
   uint64_t       uiValue)
{
   BDBG_ENTER(BV3D_P_SchedPerfCounterAdd);

   BDBG_ASSERT(uiCtr < BV3D_P_PERF_COUNTER_MAX_SCHED_CTRS_ACTIVE);

   if (hV3d->sPerfCounters.bCountersActive && hV3d->sPerfCounters.sSchedValues[uiCtr].bEnabled)
      hV3d->sPerfCounters.sSchedValues[uiCtr].uiValue += uiValue;

   BDBG_LEAVE(BV3D_P_SchedPerfCounterAdd);
}

void BV3D_P_PerfCountersRemoveClient(
   BV3D_Handle    hV3d,
   uint32_t       uiClientId
   )
{
   if (hV3d->sPerfCounters.bAcquired && hV3d->sPerfCounters.uiClientId == uiClientId)
   {
      BV3D_P_SetPerfCounting(hV3d, uiClientId, BV3D_CtrStop);
      BV3D_P_SetPerfCounting(hV3d, uiClientId, BV3D_CtrRelease);
   }
}

/* End of File */
