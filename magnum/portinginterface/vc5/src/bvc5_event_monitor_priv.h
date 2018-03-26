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
#ifndef __BVC5_EVENT_MONITOR_PRIV_H__
#define __BVC5_EVENT_MONITOR_PRIV_H__

#include "bvc5_registers_priv.h"

#define BVC5_P_EVENT_MONITOR_SCHED_TRACK     0
#define BVC5_P_EVENT_MONITOR_TFU_TRACK       1
#define BVC5_P_EVENT_MONITOR_NON_CORE_TRACKS 2

/* These are per core, so are actually offsets */
#if V3D_VER_AT_LEAST(3,3,0,0)
#define BVC5_P_EVENT_MONITOR_CORE_CLE_BIN_TRACK 0
#define BVC5_P_EVENT_MONITOR_CORE_PTB_BIN_TRACK 1
#define BVC5_P_EVENT_MONITOR_CORE_CLE_RDR_TRACK 2
#define BVC5_P_EVENT_MONITOR_CORE_TLB_RDR_TRACK 3
#define BVC5_P_EVENT_MONITOR_NUM_CORE_TRACKS    4
#else
#define BVC5_P_EVENT_MONITOR_CORE_BIN_TRACK     0
#define BVC5_P_EVENT_MONITOR_CORE_RENDER_TRACK  1
#define BVC5_P_EVENT_MONITOR_NUM_CORE_TRACKS    2
#endif

#define BVC5_P_EVENT_MONITOR_NUM_TRACKS (BVC5_P_EVENT_MONITOR_NON_CORE_TRACKS + BVC5_P_EVENT_MONITOR_NUM_CORE_TRACKS * BVC5_MAX_CORES)

#define BVC5_P_EVENT_MONITOR_BINNING         0
#define BVC5_P_EVENT_MONITOR_RENDERING       1
#define BVC5_P_EVENT_MONITOR_TFU             2
#define BVC5_P_EVENT_MONITOR_BOOM            3
#define BVC5_P_EVENT_MONITOR_BIN_LOCKUP      4
#define BVC5_P_EVENT_MONITOR_RENDER_LOCKUP   5
#define BVC5_P_EVENT_MONITOR_TFU_LOCKUP      6
#define BVC5_P_EVENT_MONITOR_PROXY           7
#define BVC5_P_EVENT_MONITOR_POWER_UP        8
#define BVC5_P_EVENT_MONITOR_POWER_DOWN      9
#define BVC5_P_EVENT_MONITOR_FLUSH_VC5       10
#define BVC5_P_EVENT_MONITOR_FLUSH_CPU       11
#define BVC5_P_EVENT_MONITOR_PART_FLUSH_CPU  12
#define BVC5_P_EVENT_MONITOR_NUM_EVENTS      13

#define BVC5_P_EVENT_MONITOR_MAX_FIELDS      (7 + BVC5_MAX_DEPENDENCIES)

#define BVC5_P_EVENT_BUFFER_BYTES            (1 * 1024 * 1024)

#define BVC5_P_EVENT_TIMESTAMP_BYTES         8
#define BVC5_P_EVENT_TRACK_BYTES             4
#define BVC5_P_EVENT_ID_BYTES                4
#define BVC5_P_EVENT_INDEX_BYTES             4
#define BVC5_P_EVENT_TYPE_BYTES              4
#define BVC5_P_EVENT_FIXED_BYTES             (BVC5_P_EVENT_TIMESTAMP_BYTES + BVC5_P_EVENT_TRACK_BYTES + BVC5_P_EVENT_ID_BYTES + BVC5_P_EVENT_INDEX_BYTES + BVC5_P_EVENT_TYPE_BYTES)

typedef struct BVC5_P_EventDesc
{
   BVC5_EventDesc          sDesc;
   BVC5_EventFieldDesc     sFieldDescs[BVC5_P_EVENT_MONITOR_MAX_FIELDS];
   uint32_t                uiSize;
} BVC5_P_EventDesc;

typedef struct BVC5_P_EventBuffer
{
   /* TODO - multiple simultaneous clients */
   void        *pBuffer;
   size_t      uiCapacityBytes;
   size_t      uiBytesUsed;
   void        *pWrite;
   void        *pRead;
   bool         bOverflow;
} BVC5_P_EventBuffer;

/* only keep a limited set of state required for event tracking */
typedef struct BVC5_P_EventInfo
{
   uint64_t       uiTimeStamp; /* from monotonic clock */

   uint32_t       uiClientId;
   uint64_t       uiJobId;

   uint16_t       uiTextureType;
   uint32_t       uiRasterStride;

   uint32_t       uiMipmapCount;
   uint32_t       uiWidth;
   uint32_t       uiHeight;

   bool           bHasDeps;
   BVC5_SchedDependencies sCompletedDependencies;
} BVC5_P_EventInfo;

typedef struct BVC5_P_Fifo
{
   BVC5_P_EventInfo       *psEventInfo[BVC5_P_HW_QUEUE_STAGES];
   uint8_t                uiWriteIndx;
   uint8_t                uiReadIndx;
   uint8_t                uiCount;
} BVC5_P_Fifo;

typedef struct BVC5_P_JobQueue
{
   /* main storage */
   BVC5_P_EventInfo       sEventInfo[BVC5_P_HW_QUEUE_STAGES];

   BVC5_P_Fifo            sFreeFifo;
   BVC5_P_Fifo            sSendFifo;
} BVC5_P_JobQueue;

typedef struct BVC5_P_EventMonitor
{
   uint32_t                uiNumTracks;
   BVC5_EventTrackDesc     sTrackDescs[BVC5_P_EVENT_MONITOR_NUM_TRACKS];
   BVC5_P_EventDesc        sEventDescs[BVC5_P_EVENT_MONITOR_NUM_EVENTS];

   bool                    bAcquired;
   uint32_t                uiClientId;

   BVC5_P_EventBuffer      sBuffer;

   bool                    bActive;
   uint32_t                uiSchedTrackNextId;

#if V3D_VER_AT_LEAST(3,3,0,0)
   BVC5_P_JobQueue         sRenderJobQueueCLE;
   BVC5_P_JobQueue         sRenderJobQueueTLB;
   BVC5_P_JobQueue         sBinJobQueueCLE;
   BVC5_P_JobQueue         sBinJobQueuePTB;
   BVC5_P_JobQueue         sQueueTFU;
   BVC5_P_JobQueue         sRenderJobQueueCLELoadStats;
#endif

} BVC5_P_EventMonitor;

void BVC5_P_InitEventMonitor(
   BVC5_Handle hVC5
   );

bool BVC5_P_AddEvent(
   BVC5_Handle    hVC5,
   uint32_t       uiTrack,
   uint32_t       uiId,
   uint32_t       uiEventIndex,
   BVC5_EventType eEventType,
   uint64_t       uiTimestamp
   );

bool BVC5_P_AddFlushEvent(
   BVC5_Handle    hVC5,
   uint32_t       uiId,
   BVC5_EventType eEventType,
   bool           clearL3,
   bool           clearL2C,
   bool           clearL2T,
   uint64_t       uiTimestamp
   );

bool BVC5_P_AddTFUJobEvent_isr(
   BVC5_Handle          hVC5,
   BVC5_EventType       eEventType,
   BVC5_P_EventInfo    *psEventInfo,
   uint64_t             uiTimestamp
   );

bool BVC5_P_AddTFUJobEvent(
   BVC5_Handle          hVC5,
   BVC5_EventType       eEventType,
   BVC5_P_EventInfo    *psEventInfo,
   uint64_t             uiTimestamp
   );

bool BVC5_P_AddCoreEvent(
   BVC5_Handle    hVC5,
   uint32_t       uiCore,
   uint32_t       uiTrack,
   uint32_t       uiId,
   uint32_t       uiEventIndex,
   BVC5_EventType eEventType,
   uint64_t       uiTimestamp
   );

bool BVC5_P_AddCoreEventCJD_isr(
   BVC5_Handle             hVC5,
   uint32_t                uiCore,
   uint32_t                uiTrack,
   uint32_t                uiEventIndex,
   BVC5_EventType          eEventType,
   uint32_t                uiClientId,
   uint64_t                uiJobId,
   BVC5_SchedDependencies *psDeps,
   uint64_t                uiTimestamp
   );

bool BVC5_P_AddCoreEventCJD(
   BVC5_Handle             hVC5,
   uint32_t                uiCore,
   uint32_t                uiTrack,
   uint32_t                uiEventIndex,
   BVC5_EventType          eEventType,
   uint32_t                uiClientId,
   uint64_t                uiJobId,
   BVC5_SchedDependencies *psDeps,
   uint64_t                uiTimestamp
   );

bool BVC5_P_AddCoreJobEvent_isr(
   BVC5_Handle          hVC5,
   uint32_t             uiCore,
   uint32_t             uiTrack,
   uint32_t             uiEventIndex,
   BVC5_EventType       eEventType,
   BVC5_P_EventInfo    *psEventInfo,
   uint64_t             uiTimestamp
   );

bool BVC5_P_AddCoreJobEvent(
   BVC5_Handle          hVC5,
   uint32_t             uiCore,
   uint32_t             uiTrack,
   uint32_t             uiEventIndex,
   BVC5_EventType       eEventType,
   BVC5_P_EventInfo    *psEventInfo,
   uint64_t             uiTimestamp
   );

void BVC5_P_EventMemStats(
   BVC5_Handle hVC5,
   size_t      *puiCapacityBytes,
   size_t      *puiUsedBytes
   );

void BVC5_P_EventsRemoveClient(
   BVC5_Handle    hVC5,
   uint32_t       uiClientId
   );

void BVC5_P_InitializeQueue(
   BVC5_P_JobQueue    *psQueue
   );

BVC5_P_EventInfo *BVC5_P_GetMessage(
   BVC5_P_JobQueue    *psQueue
   );

void BVC5_P_SendMessage(
   BVC5_P_JobQueue    *psQueue,
   BVC5_P_EventInfo   *psEventInfo
   );

BVC5_P_EventInfo *BVC5_P_ReceiveMessage_isrsafe(
   BVC5_P_JobQueue    *psQueue
   );

void BVC5_P_ReleaseMessage_isrsafe(
   BVC5_P_JobQueue    *psQueue,
   BVC5_P_EventInfo   *psEventInfo
   );

void BVC5_P_PopulateEventInfo(
   BVC5_Handle hVC5,
   bool bCopyDeps,
   bool bCopyTFU,
   BVC5_P_InternalJob *pJob,
   BVC5_P_EventInfo *psEventInfo
   );

#endif /* __BVC5_EVENT_MONITOR_PRIV_H__ */
