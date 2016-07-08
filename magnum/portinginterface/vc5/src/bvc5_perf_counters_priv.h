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

#ifndef __BVC5_PERF_COUNTERS_PRIV_H__
#define __BVC5_PERF_COUNTERS_PRIV_H__

#define BVC5_P_PERF_COUNTER_HW_GROUP      0
#define BVC5_P_PERF_COUNTER_MEM_BW_GROUP  1
#define BVC5_P_PERF_COUNTER_SCHED_GROUP   2
#define BVC5_P_PERF_COUNTER_NUM_GROUPS    3

#define BVC5_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE 16
#define BVC5_P_PERF_COUNTER_MAX_BW_CTRS_ACTIVE 1

#define BVC5_P_PERF_BIN_JOBS_SUBMITTED             0
#define BVC5_P_PERF_BIN_JOBS_COMPLETED             1
#define BVC5_P_PERF_RENDER_JOBS_SUBMITTED          2
#define BVC5_P_PERF_RENDER_JOBS_COMPLETED          3
#define BVC5_P_PERF_TFU_JOBS_SUBMITTED             4
#define BVC5_P_PERF_TFU_JOBS_COMPLETED             5
#define BVC5_P_PERF_INTERRUPTS                     6
#define BVC5_P_PERF_BIN_OOMS                       7
#define BVC5_P_PERF_BIN_MEMORY_RESERVED            8
#define BVC5_P_PERF_BIN_MEMORY_IN_USE              9
#define BVC5_P_PERF_LOCKUP_DETECTION               10
#define BVC5_P_PERF_BINNER_LOAD_AVG                11
#define BVC5_P_PERF_RENDERER_LOAD_AVG              12
#define BVC5_P_PERF_TFU_LOAD_AVG                   13
#define BVC5_P_PERF_POWERED_ON_AVG                 14
#define BVC5_P_PERF_EVENT_MEMORY_RESERVED          15
#define BVC5_P_PERF_EVENT_MEMORY_IN_USE            16
#define BVC5_P_PERF_HEAP_MEMORY_IN_USE             17
#define BVC5_P_PERF_HEAP_MEMORY_FREE               18
#define BVC5_P_PERF_COUNTER_MAX_SCHED_CTRS_ACTIVE  19 /* Must come last */

typedef struct BVC5_P_CounterValue
{
   uint64_t uiValue;
   bool     bEnabled;
} BVC5_P_CounterValue;

typedef struct BVC5_P_HwCounter
{
   bool     bEnabled;
} BVC5_P_HwCounter;

typedef struct BVC5_P_PerfCounters
{
   BVC5_CounterGroupDesc    sGroupDescs[BVC5_P_PERF_COUNTER_NUM_GROUPS];

   BVC5_P_HwCounter         sHwCounters[BVC5_MAX_COUNTERS_PER_GROUP];
   BVC5_P_HwCounter         sBwCounters[BVC5_MAX_COUNTERS_PER_GROUP];
   BVC5_P_CounterValue      sSchedValues[BVC5_MAX_COUNTERS_PER_GROUP];

   bool                     bAcquired;
   uint32_t                 uiClientId;
   bool                     bCountersActive;
   uint32_t                 uiActiveHwCounters;
   uint32_t                 uiActiveBwCounters;

   /* Used for load avg calculations */
   uint64_t                 uiBinnerCumIdleTime;
   uint64_t                 uiRendererCumIdleTime;
   uint64_t                 uiTFUCumIdleTime;
   uint64_t                 uiPoweredCumOffTime;
   uint64_t                 uiBinnerIdleStartTime;
   uint64_t                 uiRendererIdleStartTime;
   uint64_t                 uiTFUIdleStartTime;
   uint64_t                 uiPoweredOffStartTime;
   uint64_t                 uiLastCollectionTime;

   /* Shadow register writes, which need to be configured when the core powers */
   uint32_t                 uiPCTRSShadow[BVC5_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE];  /* mappings */
   uint32_t                 uiPCTRShadows[BVC5_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE];  /* counts */
   uint32_t                 uiPCTREShadow;

   uint32_t                 uiMemBwCntShadow;
   uint32_t                 uiGCAPMSelShadow;

} BVC5_P_PerfCounters;

void BVC5_P_InitPerfCounters(
   BVC5_Handle hVC5
   );

void BVC5_P_SchedPerfCounterAdd_isr(
   BVC5_Handle    hVC5,
   uint32_t       uiCtr,
   uint64_t       uiValue
   );

void BVC5_P_PerfCountersRemoveClient(
   BVC5_Handle    hVC5,
   uint32_t       uiClientId
   );

#endif /* __BVC5_PERF_COUNTERS_PRIV_H__ */
