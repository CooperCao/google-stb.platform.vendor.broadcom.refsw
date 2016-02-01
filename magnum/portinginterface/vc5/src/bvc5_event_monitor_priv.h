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

#ifndef __BVC5_EVENT_MONITOR_PRIV_H__
#define __BVC5_EVENT_MONITOR_PRIV_H__

#define BVC5_P_EVENT_MONITOR_SCHED_TRACK     0
#define BVC5_P_EVENT_MONITOR_TFU_TRACK       1
#define BVC5_P_EVENT_MONITOR_NON_CORE_TRACKS 2

/* These are per core, so are actually offsets */
#define BVC5_P_EVENT_MONITOR_CORE_BIN_TRACK     0
#define BVC5_P_EVENT_MONITOR_CORE_RENDER_TRACK  1
#define BVC5_P_EVENT_MONITOR_NUM_CORE_TRACKS    2

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

typedef struct BVC5_P_EventDesc
{
   BVC5_EventDesc          sDesc;
   BVC5_EventFieldDesc     sFieldDescs[BVC5_P_EVENT_MONITOR_MAX_FIELDS];
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
} BVC5_P_EventMonitor;

void BVC5_P_InitEventMonitor(
   BVC5_Handle hVC5
   );

bool BVC5_P_AddEvent(
   BVC5_Handle    hVC5,
   uint32_t       uiTrack,
   uint32_t       uiId,
   uint32_t       uiEventIndex,
   BVC5_EventType eEventType
   );

bool BVC5_P_AddFlushEvent(
   BVC5_Handle    hVC5,
   uint32_t       uiId,
   BVC5_EventType eEventType,
   bool           clearL3,
   bool           clearL2C,
   bool           clearL2T
   );

bool BVC5_P_AddTFUJobEvent(
   BVC5_Handle          hVC5,
   BVC5_EventType       eEventType,
   BVC5_P_InternalJob  *psJob
   );

bool BVC5_P_AddCoreEvent(
   BVC5_Handle    hVC5,
   uint32_t       uiCore,
   uint32_t       uiTrack,
   uint32_t       uiId,
   uint32_t       uiEventIndex,
   BVC5_EventType eEventType
   );

bool BVC5_P_AddCoreEventCJ(
   BVC5_Handle          hVC5,
   uint32_t             uiCore,
   uint32_t             uiTrack,
   uint32_t             uiEventIndex,
   BVC5_EventType       eEventType,
   BVC5_P_InternalJob  *psJob
   );

bool BVC5_P_AddCoreJobEvent(
   BVC5_Handle          hVC5,
   uint32_t             uiCore,
   uint32_t             uiTrack,
   uint32_t             uiEventIndex,
   BVC5_EventType       eEventType,
   BVC5_P_InternalJob  *psJob
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

#endif /* __BVC5_EVENT_MONITOR_PRIV_H__ */
