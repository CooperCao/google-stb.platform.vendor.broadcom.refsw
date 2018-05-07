/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BV3D_EVENT_MONITOR_PRIV_H__
#define __BV3D_EVENT_MONITOR_PRIV_H__

#include "bv3d.h"

#define BV3D_P_EVENT_MONITOR_SCHED_TRACK        0
#define BV3D_P_EVENT_MONITOR_USER_TRACK         1
#define BV3D_P_EVENT_MONITOR_BIN_TRACK          2
#define BV3D_P_EVENT_MONITOR_RENDER_TRACK       3
#define BV3D_P_EVENT_MONITOR_NUM_TRACKS         4

#define BV3D_P_EVENT_MONITOR_BINNING         0
#define BV3D_P_EVENT_MONITOR_RENDERING       1
#define BV3D_P_EVENT_MONITOR_USER            2
#define BV3D_P_EVENT_MONITOR_BOOM            3
#define BV3D_P_EVENT_MONITOR_BIN_LOCKUP      4
#define BV3D_P_EVENT_MONITOR_RENDER_LOCKUP   5
#define BV3D_P_EVENT_MONITOR_USER_LOCKUP     6 /* unused */
#define BV3D_P_EVENT_MONITOR_PROXY           7 /* unused */
#define BV3D_P_EVENT_MONITOR_POWER_UP        8 /* unused */
#define BV3D_P_EVENT_MONITOR_POWER_DOWN      9 /* unused */
#define BV3D_P_EVENT_MONITOR_FLUSH_V3D       10
#define BV3D_P_EVENT_MONITOR_FLUSH_CPU       11 /* unused */
#define BV3D_P_EVENT_MONITOR_PART_FLUSH_CPU  12 /* unused */
#define BV3D_P_EVENT_MONITOR_NUM_EVENTS      13

#define BV3D_P_EVENT_MONITOR_MAX_FIELDS      (7)

#define BV3D_P_EVENT_BUFFER_BYTES            (1 * 1024 * 1024)

#define BV3D_P_EVENT_TIMESTAMP_BYTES         8
#define BV3D_P_EVENT_TRACK_BYTES             4
#define BV3D_P_EVENT_ID_BYTES                4
#define BV3D_P_EVENT_INDEX_BYTES             4
#define BV3D_P_EVENT_TYPE_BYTES              4
#define BV3D_P_EVENT_FIXED_BYTES             (BV3D_P_EVENT_TIMESTAMP_BYTES + BV3D_P_EVENT_TRACK_BYTES + BV3D_P_EVENT_ID_BYTES + BV3D_P_EVENT_INDEX_BYTES + BV3D_P_EVENT_TYPE_BYTES)

typedef struct BV3D_P_EventDesc
{
   BV3D_EventDesc          sDesc;
   BV3D_EventFieldDesc     sFieldDescs[BV3D_P_EVENT_MONITOR_MAX_FIELDS];
   uint32_t                uiSize;
} BV3D_P_EventDesc;

typedef struct BV3D_P_EventBuffer
{
   /* TODO - multiple simultaneous clients */
   void        *pBuffer;
   size_t      uiCapacityBytes;
   size_t      uiBytesUsed;
   void        *pWrite;
   void        *pRead;
   bool         bOverflow;
} BV3D_P_EventBuffer;

/* only keep a limited set of state required for event tracking */
typedef struct BV3D_P_EventInfo
{
   uint64_t       uiTimeStamp; /* from monotonic clock */

   uint32_t       uiClientId;
   uint64_t       uiJobId;
} BV3D_P_EventInfo;

typedef struct BV3D_P_EventMonitor
{
   uint32_t                uiNumTracks;
   BV3D_EventTrackDesc     sTrackDescs[BV3D_P_EVENT_MONITOR_NUM_TRACKS];
   BV3D_P_EventDesc        sEventDescs[BV3D_P_EVENT_MONITOR_NUM_EVENTS];

   bool                    bAcquired;
   uint32_t                uiClientId;

   BV3D_P_EventBuffer      sBuffer;

   bool                    bActive;
   uint32_t                uiSchedTrackNextId;
} BV3D_P_EventMonitor;

void BV3D_P_InitEventMonitor(
   BV3D_Handle hV3d
   );

bool BV3D_P_AddEvent(
   BV3D_Handle    hV3d,
   uint32_t       uiTrack,
   uint32_t       uiId,
   uint32_t       uiEventIndex,
   BV3D_EventType eEventType,
   uint64_t       uiTimestamp
   );

bool BV3D_P_AddFlushEvent(
   BV3D_Handle    hV3d,
   uint32_t       uiId,
   BV3D_EventType eEventType,
   bool           clearL3,
   bool           clearL2C,
   bool           clearL2T,
   uint64_t       uiTimestamp
   );

bool BV3D_P_AddEventCJ(
   BV3D_Handle             hV3d,
   uint32_t                uiTrack,
   uint32_t                uiEventIndex,
   BV3D_EventType          eEventType,
   uint32_t                uiClientId,
   uint64_t                uiJobId,
   uint64_t                uiTimestamp
   );

bool BV3D_P_AddJobEvent(
   BV3D_Handle          hV3d,
   uint32_t             uiTrack,
   uint32_t             uiEventIndex,
   BV3D_EventType       eEventType,
   BV3D_P_EventInfo    *psEventInfo,
   uint64_t             uiTimestamp
   );

void BV3D_P_EventMemStats(
   BV3D_Handle hV3d,
   size_t      *puiCapacityBytes,
   size_t      *puiUsedBytes
   );

void BV3D_P_EventsRemoveClient(
   BV3D_Handle    hV3d,
   uint32_t       uiClientId
   );

void BV3D_P_PopulateEventInfo(
   BV3D_Handle hV3d,
   const BV3D_Job *psJob,
   BV3D_P_EventInfo *psEventInfo
   );

#endif /* __BV3D_EVENT_MONITOR_PRIV_H__ */
