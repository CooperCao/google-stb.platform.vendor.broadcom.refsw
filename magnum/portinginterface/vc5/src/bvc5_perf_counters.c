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
#include "bvc5.h"
#include "bvc5_priv.h"
#include "bvc5_perf_counters_priv.h"
#include "bvc5_registers_priv.h"
#include "bstd.h"
#include "bmma_system.h"

BDBG_MODULE(BVC5);

static void _strncpy(char *d, const char *s, uint32_t maxLen)
{
   uint32_t i;
   for (i = 0; i < maxLen - 1 && *s != 0; i++)
      *d++ = *s++;

   *d = '\0';
}

static void BVC5_P_SetCounterNamesEx(
   BVC5_CounterGroupDesc   *grp,
   uint64_t                index,
   const char              *name,
   const char              *unit,
   uint64_t                minVal,
   uint64_t                maxVal,
   uint64_t                denom
   )
{
   _strncpy(grp->saCounters[index].caName, name, BVC5_MAX_COUNTER_NAME_LEN);
   _strncpy(grp->saCounters[index].caUnitName, unit, BVC5_MAX_COUNTER_NAME_LEN);

   if (index >= grp->uiTotalCounters)
   {
      grp->uiTotalCounters = index + 1;
      grp->uiMaxActiveCounters = index + 1;
   }

   grp->saCounters[index].uiMinValue = minVal;
   grp->saCounters[index].uiMaxValue = maxVal;
   grp->saCounters[index].uiDenominator = denom;
}

static void BVC5_P_SetCounterNames(
   BVC5_CounterGroupDesc   *grp,
   uint64_t                index,
   const char              *name,
   const char              *unit
   )
{
   BVC5_P_SetCounterNamesEx(grp, index, name, unit, 0, ~0, 1);
}

/***************************************************************************/
void BVC5_P_InitPerfCounters(
   BVC5_Handle  hVC5
   )
{
   BVC5_CounterGroupDesc *d = &hVC5->sPerfCounters.sGroupDescs[BVC5_P_PERF_COUNTER_HW_GROUP];
   uint64_t              now;

   BKNI_Memset(&hVC5->sPerfCounters, 0, sizeof(hVC5->sPerfCounters));

   _strncpy(d->caName, "Hardware Counters", BVC5_MAX_GROUP_NAME_LEN);

   d->uiMaxActiveCounters = BVC5_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE;

#if V3D_VER_AT_LEAST(4,0,2,0)
   #include "bvc5_perf_counters_hw_4_0.inc"
#else
   #include "bvc5_perf_counters_hw_3_x.inc"

   d = &hVC5->sPerfCounters.sGroupDescs[BVC5_P_PERF_COUNTER_MEM_BW_GROUP];

   _strncpy(d->caName, "V3D Memory B/W", BVC5_MAX_GROUP_NAME_LEN);

   BVC5_P_SetCounterNamesEx(d, 0, "memory_read_bandwidth", "MB", 0, ~0, 1024 * 1024 / 32);
   BVC5_P_SetCounterNamesEx(d, 1, "memory_write_bandwidth", "MB", 0, ~0, 1024 * 1024 / 32);

   d->uiMaxActiveCounters = BVC5_P_PERF_COUNTER_MAX_BW_CTRS_ACTIVE;

#endif

   d = &hVC5->sPerfCounters.sGroupDescs[BVC5_P_PERF_COUNTER_SCHED_GROUP];

   _strncpy(d->caName, "Scheduler Counters", BVC5_MAX_GROUP_NAME_LEN);

   BVC5_P_SetCounterNames(d, BVC5_P_PERF_BIN_JOBS_SUBMITTED,      "bin_jobs_submitted", "jobs");
   BVC5_P_SetCounterNames(d, BVC5_P_PERF_BIN_JOBS_COMPLETED,      "bin_jobs_completed", "jobs");
   BVC5_P_SetCounterNames(d, BVC5_P_PERF_RENDER_JOBS_SUBMITTED,   "render_jobs_submitted", "jobs");
   BVC5_P_SetCounterNames(d, BVC5_P_PERF_RENDER_JOBS_COMPLETED,   "render_jobs_completed", "jobs");
   BVC5_P_SetCounterNames(d, BVC5_P_PERF_TFU_JOBS_SUBMITTED,      "tfu_jobs_submitted", "jobs");
   BVC5_P_SetCounterNames(d, BVC5_P_PERF_TFU_JOBS_COMPLETED,      "tfu_jobs_completed", "jobs");
   BVC5_P_SetCounterNames(d, BVC5_P_PERF_INTERRUPTS,              "interrupts", "interrupts");
   BVC5_P_SetCounterNames(d, BVC5_P_PERF_BIN_OOMS,                "bin_ooms", "booms");
   BVC5_P_SetCounterNamesEx(d, BVC5_P_PERF_BIN_MEMORY_RESERVED,   "bin_memory_reserved", "MB", 0, 1024, 1024 * 1024);
   BVC5_P_SetCounterNamesEx(d, BVC5_P_PERF_BIN_MEMORY_IN_USE,     "bin_memory_in_use", "MB", 0, 1024, 1024 * 1024);
   BVC5_P_SetCounterNames(d, BVC5_P_PERF_LOCKUP_DETECTION,        "lockup_detection", "lockups");
   BVC5_P_SetCounterNamesEx(d, BVC5_P_PERF_BINNER_LOAD_AVG,       "binner_load_avg", "%", 0, 100, 1);
   BVC5_P_SetCounterNamesEx(d, BVC5_P_PERF_RENDERER_LOAD_AVG,     "renderer_load_avg", "%", 0, 100, 1);
   BVC5_P_SetCounterNamesEx(d, BVC5_P_PERF_TFU_LOAD_AVG,          "tfu_load_avg", "%", 0, 100, 1);
   BVC5_P_SetCounterNamesEx(d, BVC5_P_PERF_POWERED_ON_AVG,        "powered_on_avg", "%", 0, 100, 1);
   BVC5_P_SetCounterNamesEx(d, BVC5_P_PERF_EVENT_MEMORY_RESERVED, "knl_event_memory_reserved", "KB", 0, 1024 * 32, 1024);
   BVC5_P_SetCounterNamesEx(d, BVC5_P_PERF_EVENT_MEMORY_IN_USE,   "knl_event_memory_in_use", "KB", 0, 1024 * 32, 1024);
   BVC5_P_SetCounterNamesEx(d, BVC5_P_PERF_HEAP_MEMORY_IN_USE,    "device_heap_memory_in_use", "MB", 0, 1024, 1024 * 1024);
   BVC5_P_SetCounterNamesEx(d, BVC5_P_PERF_HEAP_MEMORY_FREE,      "device_heap_memory_free", "MB", 0, 1024, 1024 * 1024);

   BDBG_ASSERT(d->uiMaxActiveCounters == BVC5_P_PERF_COUNTER_MAX_SCHED_CTRS_ACTIVE);

   BVC5_P_GetTime_isrsafe(&now);
   hVC5->sPerfCounters.uiLastCollectionTime = now;
   hVC5->sPerfCounters.uiBinnerIdleStartTime = now;
   hVC5->sPerfCounters.uiRendererIdleStartTime = now;
   hVC5->sPerfCounters.uiTFUIdleStartTime = now;
}

void BVC5_GetPerfNumCounterGroups(
   BVC5_Handle  hVC5,
   uint32_t     *puiNumGroups
   )
{
   BDBG_ENTER(BVC5_GetPerfNumCounterGroups);
   BKNI_AcquireMutex(hVC5->hModuleMutex);

   BSTD_UNUSED(hVC5);

   if (puiNumGroups != NULL)
      *puiNumGroups = BVC5_P_PERF_COUNTER_NUM_GROUPS;

   BKNI_ReleaseMutex(hVC5->hModuleMutex);
   BDBG_LEAVE(BVC5_GetPerfNumCounterGroups);
}

void BVC5_GetPerfCounterDesc(
   BVC5_Handle             hVC5,
   uint32_t                uiGroup,
   uint32_t                uiCounter,
   BVC5_CounterDesc        *psDesc
   )
{
   BDBG_ENTER(BVC5_GetPerfCounterDesc);
   BKNI_AcquireMutex(hVC5->hModuleMutex);

   if (psDesc != NULL &&
         uiGroup < BVC5_P_PERF_COUNTER_NUM_GROUPS &&
         uiCounter < BVC5_MAX_COUNTERS_PER_GROUP)
      *psDesc = hVC5->sPerfCounters.sGroupDescs[uiGroup].saCounters[uiCounter];

   BKNI_ReleaseMutex(hVC5->hModuleMutex);
   BDBG_LEAVE(BVC5_GetPerfCounterDesc);
}

void BVC5_GetPerfCounterGroupInfo(
   BVC5_Handle             hVC5,
   uint32_t                uiGroup,
   uint32_t                uiGrpNameSize,
   char                    *chGrpName,
   uint32_t                *uiMaxActiveCounter,
   uint32_t                *uiTotalCounter
   )
{
   BDBG_ENTER(BVC5_GetPerfCounterGroupInfo);
   BKNI_AcquireMutex(hVC5->hModuleMutex);

   if (chGrpName != NULL &&
         uiMaxActiveCounter != NULL &&
         uiTotalCounter != NULL &&
         uiGroup < BVC5_P_PERF_COUNTER_NUM_GROUPS)
   {
      _strncpy(chGrpName, hVC5->sPerfCounters.sGroupDescs[uiGroup].caName, uiGrpNameSize);
      *uiMaxActiveCounter = hVC5->sPerfCounters.sGroupDescs[uiGroup].uiMaxActiveCounters;
      *uiTotalCounter = hVC5->sPerfCounters.sGroupDescs[uiGroup].uiTotalCounters;
   }

   BKNI_ReleaseMutex(hVC5->hModuleMutex);
   BDBG_LEAVE(BVC5_GetPerfCounterGroupInfo);
}

static BERR_Code BVC5_P_SetPerfCounting(
   BVC5_Handle             hVC5,
   uint32_t                uiClientId,
   BVC5_CounterState       eState
   )
{
   BVC5_P_PerfCounters  *psPerf;
   uint32_t             i;
   BERR_Code            err = BERR_SUCCESS;

   psPerf = &hVC5->sPerfCounters;

   switch (eState)
   {
   case BVC5_CtrAcquire:
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
   case BVC5_CtrRelease:
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
   case BVC5_CtrStart:
   {
      if (psPerf->bAcquired && psPerf->uiClientId == uiClientId)
      {
         uint32_t c = 0;
         psPerf->bCountersActive = true;
         for (i = 0; i < psPerf->sGroupDescs[BVC5_P_PERF_COUNTER_HW_GROUP].uiTotalCounters; i++)
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

#if !V3D_VER_AT_LEAST(4,0,2,0)
         c = 0;
         for (i = 0; i < psPerf->sGroupDescs[BVC5_P_PERF_COUNTER_MEM_BW_GROUP].uiTotalCounters; i++)
         {
            if (psPerf->sBwCounters[i].bEnabled)
            {
               psPerf->uiGCAPMSelShadow = BCHP_FIELD_DATA(V3D_GCA_PM_CTRL, PM_SEL, i);
               c++;
            }
         }
#endif

         BDBG_ASSERT(c == psPerf->uiActiveBwCounters);

         /* update the counters, but only if the core is on - otherwise they'll get set at next power on */
         BVC5_P_RestorePerfCounters(hVC5, 0, /*bWriteSelectorsAndEnables=*/true);
      }
      else
         err = BERR_NOT_AVAILABLE;
      break;
   }
   case BVC5_CtrStop:
      if (psPerf->bAcquired && psPerf->uiClientId == uiClientId)
      {
         psPerf->bCountersActive = false;
         psPerf->uiPCTREShadow = 0;
#if !V3D_VER_AT_LEAST(4,0,2,0)
         psPerf->uiGCAPMSelShadow |= BCHP_FIELD_DATA(V3D_GCA_PM_CTRL, PM_CNT_FREEZE, 1);
#endif
      }
      else
         err = BERR_NOT_AVAILABLE;
      break;
   }

error:
   return err;
}

BERR_Code BVC5_SetPerfCounting(
   BVC5_Handle             hVC5,
   uint32_t                uiClientId,
   BVC5_CounterState       eState
   )
{
   BERR_Code err;

   BDBG_ENTER(BVC5_SetPerfCounting);
   BKNI_AcquireMutex(hVC5->hModuleMutex);

   err = BVC5_P_SetPerfCounting(hVC5, uiClientId, eState);

   BKNI_ReleaseMutex(hVC5->hModuleMutex);
   BDBG_LEAVE(BVC5_SetPerfCounting);

   return err;
}

void BVC5_ChoosePerfCounters(
   BVC5_Handle                hVC5,
   uint32_t                   uiClientId,
   const BVC5_CounterSelector *psSelector
   )
{
   uint32_t i;
   BDBG_ENTER(BVC5_ChoosePerfCounters);

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   if (!hVC5->sPerfCounters.bAcquired || hVC5->sPerfCounters.uiClientId != uiClientId)
      goto error;

   if (psSelector->uiGroupIndex == BVC5_P_PERF_COUNTER_HW_GROUP)
   {
      for (i = 0; i < psSelector->uiNumCounters; i++)
      {
         BDBG_ASSERT(psSelector->uiaCounters[i] < hVC5->sPerfCounters.sGroupDescs[BVC5_P_PERF_COUNTER_HW_GROUP].uiTotalCounters);

         if (psSelector->uiEnable && hVC5->sPerfCounters.uiActiveHwCounters < BVC5_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE)
         {
            if (!hVC5->sPerfCounters.sHwCounters[psSelector->uiaCounters[i]].bEnabled)
               hVC5->sPerfCounters.uiActiveHwCounters++;

            hVC5->sPerfCounters.sHwCounters[psSelector->uiaCounters[i]].bEnabled = true;
         }
         else if (!psSelector->uiEnable)
         {
            if (hVC5->sPerfCounters.sHwCounters[psSelector->uiaCounters[i]].bEnabled)
               hVC5->sPerfCounters.uiActiveHwCounters--;
            hVC5->sPerfCounters.sHwCounters[psSelector->uiaCounters[i]].bEnabled = false;
         }
      }

      BDBG_ASSERT(hVC5->sPerfCounters.uiActiveHwCounters <= BVC5_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE);

      /* Clear the counters */
      BKNI_Memset(hVC5->sPerfCounters.uiPCTRShadows, 0, sizeof(hVC5->sPerfCounters.uiPCTRShadows));
   }
#if !V3D_VER_AT_LEAST(4,0,2,0)
   if (psSelector->uiGroupIndex == BVC5_P_PERF_COUNTER_MEM_BW_GROUP)
   {
      for (i = 0; i < psSelector->uiNumCounters; i++)
      {
         BDBG_ASSERT(psSelector->uiaCounters[i] < hVC5->sPerfCounters.sGroupDescs[BVC5_P_PERF_COUNTER_MEM_BW_GROUP].uiTotalCounters);

         if (psSelector->uiEnable && hVC5->sPerfCounters.uiActiveBwCounters < BVC5_P_PERF_COUNTER_MAX_BW_CTRS_ACTIVE)
         {
            if (!hVC5->sPerfCounters.sBwCounters[psSelector->uiaCounters[i]].bEnabled)
               hVC5->sPerfCounters.uiActiveBwCounters++;

            hVC5->sPerfCounters.sBwCounters[psSelector->uiaCounters[i]].bEnabled = true;
         }
         else if (!psSelector->uiEnable)
         {
            if (hVC5->sPerfCounters.sBwCounters[psSelector->uiaCounters[i]].bEnabled)
               hVC5->sPerfCounters.uiActiveBwCounters--;
            hVC5->sPerfCounters.sBwCounters[psSelector->uiaCounters[i]].bEnabled = false;
         }
      }

      BDBG_ASSERT(hVC5->sPerfCounters.uiActiveBwCounters <= BVC5_P_PERF_COUNTER_MAX_BW_CTRS_ACTIVE);

      /* Clear the counters */
      hVC5->sPerfCounters.uiMemBwCntShadow = 0;
   }
#endif
   else if (psSelector->uiGroupIndex == BVC5_P_PERF_COUNTER_SCHED_GROUP)
   {
      BDBG_ASSERT(psSelector->uiNumCounters <= BVC5_P_PERF_COUNTER_MAX_SCHED_CTRS_ACTIVE);

      for (i = 0; i < psSelector->uiNumCounters; i++)
      {
         BDBG_ASSERT(psSelector->uiaCounters[i] < hVC5->sPerfCounters.sGroupDescs[BVC5_P_PERF_COUNTER_SCHED_GROUP].uiTotalCounters);
         hVC5->sPerfCounters.sSchedValues[psSelector->uiaCounters[i]].bEnabled = psSelector->uiEnable;
         hVC5->sPerfCounters.sSchedValues[psSelector->uiaCounters[i]].uiValue  = 0;
      }
   }

error:
   BKNI_ReleaseMutex(hVC5->hModuleMutex);
   BDBG_LEAVE(BVC5_ChoosePerfCounters);
}

/* Must be called with counter mutex held */
static void BVC5_P_UpdateCalculatedCounters(
   BVC5_Handle hVC5,
   uint32_t    uiResetCounts
   )
{
   size_t   capacity;
   size_t   used;
   uint64_t timeSinceLast;
   uint64_t now;

   BVC5_P_GetTime_isrsafe(&now);

   BVC5_P_BinPoolStats(BVC5_P_GetBinPool(hVC5), &capacity, &used);

   if (hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_BIN_MEMORY_RESERVED].bEnabled)
      hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_BIN_MEMORY_RESERVED].uiValue = capacity;

   if (hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_BIN_MEMORY_IN_USE].bEnabled)
      hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_BIN_MEMORY_IN_USE].uiValue   = used;

   BVC5_P_EventMemStats(hVC5, &capacity, &used);

   if (hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_EVENT_MEMORY_RESERVED].bEnabled)
      hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_EVENT_MEMORY_RESERVED].uiValue = capacity;

   if (hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_EVENT_MEMORY_IN_USE].bEnabled)
      hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_EVENT_MEMORY_IN_USE].uiValue = used;

   timeSinceLast = now - hVC5->sPerfCounters.uiLastCollectionTime;

   /* Binner load avg */
   if (hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_BINNER_LOAD_AVG].bEnabled)
   {
      uint64_t idlePct = 100;
      uint64_t idleTime = hVC5->sPerfCounters.uiBinnerCumIdleTime;

      if (hVC5->sPerfCounters.uiBinnerIdleStartTime != 0)
         idleTime += (now - hVC5->sPerfCounters.uiBinnerIdleStartTime);

      if (timeSinceLast > 0)
         idlePct = (100 * idleTime) / timeSinceLast;

      hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_BINNER_LOAD_AVG].uiValue = 100 - idlePct;

      if (uiResetCounts)
      {
         hVC5->sPerfCounters.uiBinnerCumIdleTime = 0;
         if (hVC5->sPerfCounters.uiBinnerIdleStartTime != 0)
            hVC5->sPerfCounters.uiBinnerIdleStartTime = now;
      }
   }

   /* Render load avg */
   if (hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_RENDERER_LOAD_AVG].bEnabled)
   {
      uint64_t idlePct = 100;
      uint64_t idleTime = hVC5->sPerfCounters.uiRendererCumIdleTime;

      if (hVC5->sPerfCounters.uiRendererIdleStartTime != 0)
         idleTime += (now - hVC5->sPerfCounters.uiRendererIdleStartTime);

      if (timeSinceLast > 0)
         idlePct = (100 * idleTime) / timeSinceLast;

      hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_RENDERER_LOAD_AVG].uiValue = 100 - idlePct;

      if (uiResetCounts)
      {
         hVC5->sPerfCounters.uiRendererCumIdleTime = 0;
         if (hVC5->sPerfCounters.uiRendererIdleStartTime != 0)
            hVC5->sPerfCounters.uiRendererIdleStartTime = now;
      }
   }

   /* TFU load avg */
   if (hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_TFU_LOAD_AVG].bEnabled)
   {
      uint64_t idlePct = 100;
      uint64_t idleTime = hVC5->sPerfCounters.uiTFUCumIdleTime;

      if (hVC5->sPerfCounters.uiTFUIdleStartTime != 0)
         idleTime += (now - hVC5->sPerfCounters.uiTFUIdleStartTime);

      if (timeSinceLast > 0)
         idlePct = (100 * idleTime) / timeSinceLast;

      hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_TFU_LOAD_AVG].uiValue = 100 - idlePct;

      if (uiResetCounts)
      {
         hVC5->sPerfCounters.uiTFUCumIdleTime = 0;
         if (hVC5->sPerfCounters.uiTFUIdleStartTime != 0)
            hVC5->sPerfCounters.uiTFUIdleStartTime = now;
      }
   }

   /* Powered-on avg */
   if (hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_POWERED_ON_AVG].bEnabled)
   {
      uint64_t offPct = 100;
      uint64_t offTime = hVC5->sPerfCounters.uiPoweredCumOffTime;

      if (hVC5->sPerfCounters.uiPoweredOffStartTime != 0)
         offTime += (now - hVC5->sPerfCounters.uiPoweredOffStartTime);

      if (timeSinceLast > 0)
         offPct = (100 * offTime) / timeSinceLast;

      hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_POWERED_ON_AVG].uiValue = 100 - offPct;

      if (uiResetCounts)
      {
         hVC5->sPerfCounters.uiPoweredCumOffTime = 0;
         if (hVC5->sPerfCounters.uiPoweredOffStartTime != 0)
            hVC5->sPerfCounters.uiPoweredOffStartTime = now;
      }
   }

#if !BMMA_USE_STUB
   /* Device memory usage */
   {
      BMMA_Heap_FastStatus memStats;

      BMMA_Heap_GetStatus(BVC5_P_GetMMAHeap(hVC5), &memStats, NULL);

      if (hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_HEAP_MEMORY_IN_USE].bEnabled)
         hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_HEAP_MEMORY_IN_USE].uiValue = memStats.totalAllocated;

      if (hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_HEAP_MEMORY_FREE].bEnabled)
         hVC5->sPerfCounters.sSchedValues[BVC5_P_PERF_HEAP_MEMORY_FREE].uiValue = memStats.totalFree;
   }
#endif

   if (uiResetCounts)
      hVC5->sPerfCounters.uiLastCollectionTime = now;
}

uint32_t BVC5_GetPerfCounterData(
   BVC5_Handle    hVC5,
   uint32_t       uiMaxCounters,
   uint32_t       uiResetCounts,
   BVC5_Counter   *psCounters
   )
{
   BVC5_P_PerfCounters  *psPerf;
   uint32_t             ret = 0;
   uint32_t             i;
   uint32_t             uiCoreIndex = 0;

   BDBG_ENTER(BVC5_GetPerfCounterData);

   BKNI_AcquireMutex(hVC5->hModuleMutex);

   if (!hVC5->sPerfCounters.bAcquired)
      goto error;

   psPerf = &hVC5->sPerfCounters;

   if (uiMaxCounters == 0 || psCounters == NULL)
   {
      /* Get number of counters */
      uint32_t counters = psPerf->uiActiveHwCounters + psPerf->uiActiveBwCounters;

      for (i = 0; i < BVC5_P_PERF_COUNTER_MAX_SCHED_CTRS_ACTIVE; i++)
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

      BVC5_P_UpdateShadowCounters(hVC5, 0);

      for (i = 0; i < BVC5_MAX_COUNTERS_PER_GROUP; i++)
      {
         if (psPerf->sHwCounters[i].bEnabled)
         {
            psCounters[c].uiGroupIndex = BVC5_P_PERF_COUNTER_HW_GROUP;
            psCounters[c].uiCounterIndex = i;
            /* TODO - multi core */
            psCounters[c].uiValue = psPerf->uiPCTRShadows[c];

            c++;
         }

         if (c >= psPerf->uiActiveHwCounters || c >= uiMaxCounters)
            break;
      }

#if !V3D_VER_AT_LEAST(4,0,2,0)
      for (i = 0; i < BVC5_MAX_COUNTERS_PER_GROUP; i++)
      {
         if (psPerf->sBwCounters[i].bEnabled)
         {
            psCounters[c].uiGroupIndex = BVC5_P_PERF_COUNTER_MEM_BW_GROUP;
            psCounters[c].uiCounterIndex = i;
            psCounters[c].uiValue = psPerf->uiMemBwCntShadow;

            c++;
         }

         if (c >= (psPerf->uiActiveHwCounters + psPerf->uiActiveBwCounters) ||
             c >= uiMaxCounters)
            break;
      }
#endif

      /* Scheduler counters */
      BVC5_P_UpdateCalculatedCounters(hVC5, uiResetCounts);

      for (i = 0; i < BVC5_P_PERF_COUNTER_MAX_SCHED_CTRS_ACTIVE; i++)
      {
         if (c < uiMaxCounters)
         {
            if (psPerf->sSchedValues[i].bEnabled)
            {
               psCounters[c].uiGroupIndex = BVC5_P_PERF_COUNTER_SCHED_GROUP;
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
      BVC5_P_RestorePerfCounters(hVC5, uiCoreIndex, /*bWriteSelectorsAndEnables=*/false);

      /* TODO multi core */
      BKNI_Memset(psPerf->uiPCTRShadows, 0, sizeof(psPerf->uiPCTRShadows));
      psPerf->uiMemBwCntShadow = 0;

      for (i = 0; i < BVC5_P_PERF_COUNTER_MAX_SCHED_CTRS_ACTIVE; i++)
         psPerf->sSchedValues[i].uiValue = 0;
   }

error:
   BKNI_ReleaseMutex(hVC5->hModuleMutex);
   BDBG_LEAVE(BVC5_GetPerfCounterData);

   return ret;
}

void BVC5_P_SchedPerfCounterAdd_isr(
   BVC5_Handle    hVC5,
   uint32_t       uiCtr,
   uint64_t       uiValue)
{
   BDBG_ENTER(BVC5_P_SchedPerfCounterAdd);

   BDBG_ASSERT(uiCtr < BVC5_P_PERF_COUNTER_MAX_SCHED_CTRS_ACTIVE);

   if (hVC5->sPerfCounters.bCountersActive && hVC5->sPerfCounters.sSchedValues[uiCtr].bEnabled)
      __sync_fetch_and_add(&hVC5->sPerfCounters.sSchedValues[uiCtr].uiValue, uiValue);

   BDBG_LEAVE(BVC5_P_SchedPerfCounterAdd);
}

void BVC5_P_PerfCountersRemoveClient(
   BVC5_Handle    hVC5,
   uint32_t       uiClientId
   )
{
   if (hVC5->sPerfCounters.bAcquired && hVC5->sPerfCounters.uiClientId == uiClientId)
   {
      BVC5_P_SetPerfCounting(hVC5, uiClientId, BVC5_CtrStop);
      BVC5_P_SetPerfCounting(hVC5, uiClientId, BVC5_CtrRelease);
   }
}

/* End of File */
