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
#ifndef __BV3D_PERF_COUNTERS_PRIV_H__
#define __BV3D_PERF_COUNTERS_PRIV_H__

/*#include "bv3d_registers_priv.h"*/

#define BV3D_P_PERF_COUNTER_HW_GROUP      0
#define BV3D_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE 16
#define BV3D_P_PERF_COUNTER_SCHED_GROUP   1
#define BV3D_P_PERF_COUNTER_NUM_GROUPS    2

#define BV3D_P_PERF_BIN_JOBS_SUBMITTED             0
#define BV3D_P_PERF_BIN_JOBS_COMPLETED             1
#define BV3D_P_PERF_RENDER_JOBS_SUBMITTED          2
#define BV3D_P_PERF_RENDER_JOBS_COMPLETED          3
#define BV3D_P_PERF_USER_JOBS_SUBMITTED            4
#define BV3D_P_PERF_USER_JOBS_COMPLETED            5
#define BV3D_P_PERF_INTERRUPTS                     6
#define BV3D_P_PERF_BIN_OOMS                       7
#define BV3D_P_PERF_BIN_MEMORY_RESERVED            8
#define BV3D_P_PERF_BIN_MEMORY_IN_USE              9
#define BV3D_P_PERF_LOCKUP_DETECTION               10
#define BV3D_P_PERF_BINNER_LOAD_AVG                11
#define BV3D_P_PERF_RENDERER_LOAD_AVG              12
#define BV3D_P_PERF_USER_LOAD_AVG                  13
#define BV3D_P_PERF_POWERED_ON_AVG                 14
#define BV3D_P_PERF_EVENT_MEMORY_RESERVED          15
#define BV3D_P_PERF_EVENT_MEMORY_IN_USE            16
#define BV3D_P_PERF_HEAP_MEMORY_IN_USE             17
#define BV3D_P_PERF_HEAP_MEMORY_FREE               18
#define BV3D_P_PERF_COUNTER_MAX_SCHED_CTRS_ACTIVE  19 /* Must come last */

typedef struct BV3D_P_CounterValue
{
   uint64_t uiValue;
   bool     bEnabled;
} BV3D_P_CounterValue;

typedef struct BV3D_P_HwCounter
{
   bool     bEnabled;
} BV3D_P_HwCounter;

typedef struct BV3D_P_PerfCounters
{
   BV3D_CounterGroupDesc    sGroupDescs[BV3D_P_PERF_COUNTER_NUM_GROUPS];

   BV3D_P_HwCounter         sHwCounters[BV3D_MAX_COUNTERS_PER_GROUP];
   BV3D_P_CounterValue      sSchedValues[BV3D_MAX_COUNTERS_PER_GROUP];

   bool                     bAcquired;
   uint32_t                 uiClientId;
   bool                     bCountersActive;
   uint32_t                 uiActiveHwCounters;

   /* Used for load avg calculations */
   uint64_t                 uiBinnerCumIdleTime;
   uint64_t                 uiRendererCumIdleTime;
   uint64_t                 uiUserCumIdleTime;
   uint64_t                 uiPoweredCumOffTime;
   uint64_t                 uiBinnerIdleStartTime;
   uint64_t                 uiRendererIdleStartTime;
   uint64_t                 uiUserIdleStartTime;
   uint64_t                 uiPoweredOffStartTime;
   uint64_t                 uiLastCollectionTime;

   /* Shadow register writes, which need to be configured when the core powers */
   uint32_t                 uiPCTRSShadow[BV3D_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE];  /* mappings */
   uint32_t                 uiPCTRShadows[BV3D_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE];  /* counts */
   uint32_t                 uiPCTREShadow;

} BV3D_P_PerfCounters;

void BV3D_P_InitPerfCounters(
   BV3D_Handle hVC5
   );

void BV3D_P_SchedPerfCounterAdd(
   BV3D_Handle    hVC5,
   uint32_t       uiCtr,
   uint64_t       uiValue
   );

void BV3D_P_PerfCountersRemoveClient(
   BV3D_Handle    hVC5,
   uint32_t       uiClientId
   );

#endif /* __BV3D_PERF_COUNTERS_PRIV_H__ */
