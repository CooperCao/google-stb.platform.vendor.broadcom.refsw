/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/

/***************************************************************************
 * PRIVATE API
 * -----------
 *
 * The only supported mechanism to operate the 3d hardware is via OpenGL.
 *
 **************************************************************************/

/***************************************************************************
 * PRIVATE API
 * -----------
 *
 * The only supported mechanism to operate the 3d hardware is via OpenGL.
 *
 **************************************************************************/
#ifndef NEXUS_GRAPHICSV3D_H__
#define NEXUS_GRAPHICSV3D_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Handle for the GraphicsV3D interface.
**/
typedef struct NEXUS_Graphicsv3d *NEXUS_Graphicsv3dHandle;

/**
Summary:
Contains a debugging timestamp used when event logging.
**/
typedef struct NEXUS_Graphicsv3dEventTime
{
   uint32_t uiSecs;        /* Number of seconds      */
   uint32_t uiMicrosecs;   /* Number of microseconds */
} NEXUS_Graphicsv3dEventTime;

/**
Summary:
Contains the time-stamp data for one debug timeline entry.
**/
typedef struct NEXUS_Graphicsv3dTimelineData
{
   NEXUS_Graphicsv3dEventTime    sBinStart;     /* Time that the binner started      */
   NEXUS_Graphicsv3dEventTime    sBinEnd;       /* Time that the binner ended        */
   NEXUS_Graphicsv3dEventTime    sRenderStart;  /* Time that the renderer started    */
   NEXUS_Graphicsv3dEventTime    sRenderEnd;    /* Time that the renderer ended      */
   NEXUS_Graphicsv3dEventTime    sUserStart;    /* Time that the user-shader started */
   NEXUS_Graphicsv3dEventTime    sUserEnd;      /* Time that the user-shader ended   */
} NEXUS_Graphicsv3dTimelineData;

/**
Summary:
Structure containing the data for an event notification.

Description:
Notifications are queued when a callback from a job is issued.
Clients can pop the notification queue when they get a callback so they can find out
what is being notified.
**/
typedef struct NEXUS_Graphicsv3dNotification
{
   uint64_t                         uiParam;          /* The notification parameter that was part of the job */
   uint32_t                         uiSync;           /* 1 if this a synchronous notification, 0 otherwise   */
   uint32_t                         uiOutOfMemory;    /* Did the core run out of memory during the job?      */
   uint64_t                         uiJobSequence;    /* The sequence number of the job                      */
} NEXUS_Graphicsv3dNotification;

/**
Summary:
Structure holds information about the 3D core.

Description:
Filled by NEXUS_Graphicsv3d_GetInfo to report essential information about the core.
**/
typedef struct NEXUS_Graphicsv3dInfo
{
   char        chName[10];                /* The core variant as a printable string */
   uint32_t    uiNumSlices;               /* Number of slices                       */
   uint32_t    uiTextureUnitsPerSlice;    /* Number of texture units per slice      */
   uint32_t    uiTechRev;                 /* The technology revision number         */
   uint32_t    uiRevision;                /* The core revision number               */
   char        chRevStr[3];               /* The revision as a string               */
} NEXUS_Graphicsv3dInfo;

/**
Summary:
Bin memory is requested using these settings.
**/
typedef struct NEXUS_Graphicsv3dBinMemorySettings
{
   bool    bSecure;
} NEXUS_Graphicsv3dBinMemorySettings;

/**
Summary:
Bin memory filled in by NEXUS_Graphicsv3d_GetBinMemory.
**/
typedef struct NEXUS_Graphicsv3dBinMemory
{
   uint32_t    uiAddress;  /* Device memory offset to the memory allocated  */
   uint32_t    uiSize;     /* The size in bytes of the memory block         */
} NEXUS_Graphicsv3dBinMemory;

#define NEXUS_GRAPHICSV3D_JOB_MAX_INSTRUCTIONS  (8)

/**
Summary:
Client load data returned by BV3D_GetLoadData.
**/
typedef struct NEXUS_Graphicsv3dClientLoadData
{
   uint32_t uiClientId;
   uint32_t uiClientPID;
   uint32_t uiNumRenders;
   uint8_t  sRenderPercent;
} NEXUS_Graphicsv3dClientLoadData;

/**
Summary:
The op-code for individual instructions used within jobs.
**/
typedef enum NEXUS_Graphicsv3dOperation
{
   NEXUS_Graphicsv3dOperation_eEndInstr = 0,   /* Args none                        */
   NEXUS_Graphicsv3dOperation_eBinInstr,       /* Args bin_start    and bin_end    */
   NEXUS_Graphicsv3dOperation_eRenderInstr,    /* Args render_start and render_end */
   NEXUS_Graphicsv3dOperation_eUserInstr,      /*                                  */
   NEXUS_Graphicsv3dOperation_eWaitInstr,      /* Args waitFlags                   */
   NEXUS_Graphicsv3dOperation_eSyncInstr,      /* Args none                        */
   NEXUS_Graphicsv3dOperation_eNotifyInstr,    /* Args notifyFlags                 */
   NEXUS_Graphicsv3dOperation_eFenceInstr,     /* Args none         and fence      */
   NEXUS_Graphicsv3dOperation_eSecureInstr     /* Args is the job secure           */
} NEXUS_Graphicsv3dOperation;

/**
Summary:
Job instructions.

Description:
A job consists of multiple instructions. Each instruction is one of these.
**/
typedef struct NEXUS_Graphicsv3dInstruction
{
   NEXUS_Graphicsv3dOperation eOperation;       /* The opcode for this instruction                   */
   uint32_t                   uiArg1;           /* First argument (if appropriate)                   */
   uint64_t                   uiArg2;           /* Second argument (if appropriate)                  */
   uint64_t                   uiCallbackParam;  /* Callback parameter (or 0 if no callback required) */
} NEXUS_Graphicsv3dInstruction;

/**
Summary:
The job structure used to submit jobs in NEXUS_Graphicsv3d_SendJob

Description:
Contains a list of instructions and the necessary data to complete the job.
**/
typedef struct NEXUS_Graphicsv3dJob
{
   NEXUS_Graphicsv3dInstruction  sProgram[NEXUS_GRAPHICSV3D_JOB_MAX_INSTRUCTIONS];  /* List of instructions       */
   uint32_t                      uiBinMemory;         /* Pointer to initial bin memory for tasks in this job      */
   bool                          bBinMemorySecure;    /* Bin memory came from secure pool, so it can be attached  */
   uint32_t                      uiUserVPM;           /* Settings for the V3D VPM for this job                    */
   bool                          bCollectTimeline;    /* True if time-line data is wanted                         */
   uint64_t                      uiJobSequence;       /* Unique sequence id                                       */
} NEXUS_Graphicsv3dJob;

/**
Summary:
Settings used when creating the V3D module
**/
typedef struct NEXUS_Graphicsv3dCreateSettings
{
   NEXUS_CallbackDesc   sJobCallback;   /* The callback handler that will receive all callbacks from the module */
   uint32_t             uiClientPID;    /* The process id of the client. Used for debug purposes when querying client loads */
} NEXUS_Graphicsv3dCreateSettings;

/**
Summary:
Get default settings for NEXUS_Graphicsv3d_Create
**/
void NEXUS_Graphicsv3d_GetDefaultCreateSettings(
   NEXUS_Graphicsv3dCreateSettings *psSettings        /* [out] */
   );

/**
Summary:
Set frequency scaling percentage for Graphics 3D
**/
NEXUS_Error NEXUS_Graphicsv3d_SetFrequencyScaling(
   uint32_t percent                                   /* [in] */
   );

/**
Summary:
Create a GraphicsV3D interface for OpenGLES use.

Description:
This module is only designed to be used by the OpenGLES driver.
It should not be used directly.
**/
NEXUS_Graphicsv3dHandle NEXUS_Graphicsv3d_Create(  /* attr{destructor=NEXUS_Graphicsv3d_Destroy}  */
   const NEXUS_Graphicsv3dCreateSettings *psSettings
   );

/**
Summary:
Close the GraphicsV3D interface.
**/
void NEXUS_Graphicsv3d_Destroy(
   NEXUS_Graphicsv3dHandle hGfx
   );

/**
Summary:
Get information about the type of 3D core in this chip.

Description:
Fills in a NEXUS_Graphicsv3dInfo structure with relevant data.
**/
NEXUS_Error NEXUS_Graphicsv3d_GetInfo(
   NEXUS_Graphicsv3dInfo   *psInfo                /* [out] */
   );

/**
Summary:
Queue a job for the V3D hardware to execute at the earliest opportunity.

Description:
Sends a job to the 3D job queuing system. The job will be executed as
soon as the hardware is available (subject to fair scheduling with other
processes).

Any instructions in the job with are flagged to callback will do so as they
complete. Likewise, any explicit notify instructions will callback as they
complete.
**/
NEXUS_Error NEXUS_Graphicsv3d_SendJob(
   NEXUS_Graphicsv3dHandle       hGfx,
   const NEXUS_Graphicsv3dJob    *psJob
   );

/**
Summary:
Pop the earliest pending notification from the queue of notifications.

Description:
As job instructions complete their notifications are added to a queue and the
callback that was registered in NEXUS_Graphicsv3d_Create is called. When a callback
is received by the client application, it should keep calling NEXUS_Graphicsv3d_GetNotification
repeatedly until it stops returning NEXUS_SUCCESS. This will ensure that all notifications
are retrieved since there may be less actual callbacks than notifications in the queue.
**/
NEXUS_Error NEXUS_Graphicsv3d_GetNotification(
   NEXUS_Graphicsv3dHandle       hGfx,
   NEXUS_Graphicsv3dNotification *psNotification /* [out] */
   );

/**
Summary:
Pops the timeline data from a queue.

Description:
If the job was flagged to collect timeline data (via bCollectTimeline), each notification will
have a corresponding timeline data placed in a queue. When the notification is popped, the
timeline data should also be popped using this function.
**/
NEXUS_Error NEXUS_Graphicsv3d_GetTimelineForLastNotification(
   NEXUS_Graphicsv3dHandle       hGfx,
   NEXUS_Graphicsv3dTimelineData *psTimelineData /* [out] */
   );

/**
Summary:
Sends a synchronous acknowledgment to the V3D core.

Description:
There are very few occasions that the V3D core has to wait for the client.
When this is required, the client will call this function tell the V3D core to
continue. When blocked in this way other processes will still continue to run
on the 3D core, so deadlock cannot occur.

If the client decides that the job should be abandoned instead, it can set the abandon
flag.
**/
void NEXUS_Graphicsv3d_SendSync(
   NEXUS_Graphicsv3dHandle hGfx,
   bool                    bAbandon
   );

/**
Summary:
Request initial bin memory for a job to use.

Description:
Requests the initial block of bin memory that a job will use.
Any further bin memory that is needed during job execution will be
managed by the core software layers, not the client code.
**/
NEXUS_Error NEXUS_Graphicsv3d_GetBinMemory(
   NEXUS_Graphicsv3dHandle                   hGfx,
   const NEXUS_Graphicsv3dBinMemorySettings  *psSettings,
   NEXUS_Graphicsv3dBinMemory                *psMemory     /* [out] */
   );

/************************************************************************/
/* Performance counters                                                 */
/************************************************************************/

#define NEXUS_GRAPHICSV3D_MAX_GROUP_NAME_LEN         64
#define NEXUS_GRAPHICSV3D_MAX_COUNTER_NAME_LEN       64
#define NEXUS_GRAPHICSV3D_MAX_COUNTER_UNIT_NAME_LEN  32
#define NEXUS_GRAPHICSV3D_MAX_COUNTERS_PER_GROUP     96

typedef enum NEXUS_Graphicsv3dCounterState
{
   NEXUS_Graphicsv3dCtrAcquire = 0,
   NEXUS_Graphicsv3dCtrRelease = 1,
   NEXUS_Graphicsv3dCtrStart   = 2,
   NEXUS_Graphicsv3dCtrStop    = 3
} NEXUS_Graphicsv3dCounterState;

/* Descriptor structure for a counter.
 * Examples of unit_name : cycles, %, bytes, frames, hits, misses, etc.
*/
typedef struct NEXUS_Graphicsv3dCounterDesc
{
   char        caName[NEXUS_GRAPHICSV3D_MAX_COUNTER_NAME_LEN];
   char        caUnitName[NEXUS_GRAPHICSV3D_MAX_COUNTER_UNIT_NAME_LEN];
   uint64_t    uiMinValue;
   uint64_t    uiMaxValue;
   uint64_t    uiDenominator;
} NEXUS_Graphicsv3dCounterDesc;

/* Descriptor for a counter group */
typedef struct NEXUS_Graphicsv3dCounterGroupDesc
{
   char                          caName[NEXUS_GRAPHICSV3D_MAX_GROUP_NAME_LEN];
   uint32_t                      uiTotalCounters;
   uint32_t                      uiMaxActiveCounters;
   NEXUS_Graphicsv3dCounterDesc  saCounters[NEXUS_GRAPHICSV3D_MAX_COUNTERS_PER_GROUP];
} NEXUS_Graphicsv3dCounterGroupDesc;

/* Holds a list of counter values to be enabled/disabled for a given group */
typedef struct NEXUS_Graphicsv3dCounterSelector
{
   uint32_t    uiGroupIndex;
   uint32_t    uiEnable;
   uint32_t    uiaCounters[NEXUS_GRAPHICSV3D_MAX_COUNTERS_PER_GROUP];
   uint32_t    uiNumCounters;
} NEXUS_Graphicsv3dCounterSelector;

/* A single counter entry */
typedef struct NEXUS_Graphicsv3dCounter
{
   uint32_t   uiGroupIndex;
   uint32_t   uiCounterIndex;  /* Within group */
   uint64_t   uiValue;
} NEXUS_Graphicsv3dCounter;


void NEXUS_Graphicsv3d_GetPerfNumCounterGroups(
   NEXUS_Graphicsv3dHandle  hGfx,                              /* [in]  */
   uint32_t                *puiNumGroups                       /* [out] */
   );

void NEXUS_Graphicsv3d_GetPerfCounterDesc(
   NEXUS_Graphicsv3dHandle             hGfx,                   /* [in]  */
   uint32_t                            uiGroup,                /* [in]  */
   uint32_t                            uiCounter,              /* [in]  */
   NEXUS_Graphicsv3dCounterDesc        *psDesc                 /* [out] */
   );

void NEXUS_Graphicsv3d_GetPerfCounterGroupInfo(
   NEXUS_Graphicsv3dHandle             hGfx,                   /* [in]  */
   uint32_t                            uiGroup,                /* [in]  */
   uint32_t                            uiGrpNameSize,          /* [in]  */
   char                                *chGrpName,             /* [out] attr{nelem=uiGrpNameSize}*/
   uint32_t                            *uiMaxActiveCounter,    /* [out] */
   uint32_t                            *uiTotalCounter         /* [out] */
   );

NEXUS_Error NEXUS_Graphicsv3d_SetPerfCounting(
   NEXUS_Graphicsv3dHandle             hGfx,                   /* [in]  */
   NEXUS_Graphicsv3dCounterState       eState                  /* [in]  */
   );

void NEXUS_Graphicsv3d_ChoosePerfCounters(
   NEXUS_Graphicsv3dHandle                hGfx,                /* [in]  */
   const NEXUS_Graphicsv3dCounterSelector *psSelector          /* [in]  */
   );

/*
   NOTE:
   debug API, so no reserved property for nelem
*/
void NEXUS_Graphicsv3d_GetPerfCounterData(
   NEXUS_Graphicsv3dHandle    hGfx,                            /* [in]  */
   uint32_t                   uiMaxCounters,                   /* [in]  */
   uint32_t                   uiResetCounts,                   /* [in]  */
   uint32_t                   *puiCountersOut,                 /* [out] */
   NEXUS_Graphicsv3dCounter   *psCounters                      /* [out] attr{nelem=uiMaxCounters;nelem_out=puiCountersOut;null_allowed=y} */
   );

/**
Summary:
Turn load data collection on or off (defaults off).
**/
void NEXUS_Graphicsv3d_SetGatherLoadData(
   bool bCollect
   );

/**
Summary:
Get information about the loading of individual V3D clients.

Description:
Fills in a BV3D_LoadData structure with relevant data about the 3D core.

Calling with pData==NULL & uiNumClients==0 will fill in pValidClients with the number of
clients that have data. You can then allocate enough space for the pData.

Call again with a real pData array to retrieve the data.

The load data statistics will be reset when the client data has been retrieved.
**/
NEXUS_Error NEXUS_Graphicsv3d_GetLoadData(
   NEXUS_Graphicsv3dClientLoadData *pLoadData,     /* [out] attr{nelem=uiNumClients;nelem_out=pValidClients;null_allowed=y} */
   uint32_t                         uiNumClients,
   uint32_t                        *pValidClients
   );

/************************************************************************/
/* Event timeline                                                       */
/************************************************************************/
#define NEXUS_GRAPHICSV3D_MAX_EVENT_STRING_LEN   64

typedef enum NEXUS_Graphicsv3dEventState
{
   NEXUS_Graphicsv3dEventAcquire = 0,
   NEXUS_Graphicsv3dEventRelease = 1,
   NEXUS_Graphicsv3dEventStart   = 2,
   NEXUS_Graphicsv3dEventStop    = 3
} NEXUS_Graphicsv3dEventState;

typedef enum NEXUS_Graphicsv3dEventType
{
   NEXUS_Graphicsv3dEventBegin   = 0,
   NEXUS_Graphicsv3dEventEnd     = 1,
   NEXUS_Graphicsv3dEventOneshot = 2
} NEXUS_Graphicsv3dEventType;

typedef enum NEXUS_Graphicsv3dFieldType
{
   NEXUS_Graphicsv3dEventInt32   = 0,
   NEXUS_Graphicsv3dEventUInt32  = 1,
   NEXUS_Graphicsv3dEventInt64   = 2,
   NEXUS_Graphicsv3dEventUInt64  = 3
} NEXUS_Graphicsv3dFieldType;

/* Contains an event description. */
typedef struct NEXUS_Graphicsv3dEventDesc
{
   char     caName[NEXUS_GRAPHICSV3D_MAX_EVENT_STRING_LEN];
   uint32_t uiNumDataFields;  /* How many optional extra fields there are */
} NEXUS_Graphicsv3dEventDesc;

/* Contains an event data field description. */
typedef struct NEXUS_Graphicsv3dEventFieldDesc
{
   char                       caName[NEXUS_GRAPHICSV3D_MAX_EVENT_STRING_LEN];
   NEXUS_Graphicsv3dFieldType eDataType;
} NEXUS_Graphicsv3dEventFieldDesc;

/* Contains an event track description.
* The track is used to group related events. You can think of it as the row
* in an event table to which the event belongs. All binner related events
* should have a common track for example. This means they will all get displayed
* in the binner row in an event graphing tool.
*/
typedef struct NEXUS_Graphicsv3dEventTrackDesc
{
   char     caName[NEXUS_GRAPHICSV3D_MAX_EVENT_STRING_LEN];
} NEXUS_Graphicsv3dEventTrackDesc;

void NEXUS_Graphicsv3d_GetEventCounts(
   NEXUS_Graphicsv3dHandle  hGfx,                        /* [in]  */
   uint32_t                 *uiNumTracks,                /* [out] */
   uint32_t                 *uiNumEvents                 /* [out] */
   );

NEXUS_Error NEXUS_Graphicsv3d_GetEventTrackInfo(
   NEXUS_Graphicsv3dHandle           hGfx,               /* [in]  */
   uint32_t                          uiTrack,            /* [in]  */
   NEXUS_Graphicsv3dEventTrackDesc   *psTrackDesc        /* [out] */
   );


NEXUS_Error NEXUS_Graphicsv3d_GetEventInfo(
   NEXUS_Graphicsv3dHandle       hGfx,                   /* [in]  */
   uint32_t                      uiEvent,                /* [in]  */
   NEXUS_Graphicsv3dEventDesc   *psEventDesc             /* [out] */
   );


NEXUS_Error NEXUS_Graphicsv3d_GetEventDataFieldInfo(
   NEXUS_Graphicsv3dHandle          hGfx,                /* [in]  */
   uint32_t                         uiEvent,             /* [in]  */
   uint32_t                         uiField,             /* [in]  */
   NEXUS_Graphicsv3dEventFieldDesc  *psFieldDesc         /* [out] */
   );

NEXUS_Error NEXUS_Graphicsv3d_SetEventCollection(
   NEXUS_Graphicsv3dHandle       hGfx,                   /* [in]  */
   NEXUS_Graphicsv3dEventState   eState                  /* [in]  */
   );

/*
   NOTE:
   debug API, so no reserved property for nelem
*/
void NEXUS_Graphicsv3d_GetEventData(
   NEXUS_Graphicsv3dHandle    hGfx,                      /* [in]  */
   uint32_t                   uiEventBufferBytes,        /* [in]  */
   void                       *pvEventBuffer,            /* [out] attr{nelem=uiEventBufferBytes;nelem_out=puiBytesCopiedOut;null_allowed=y} */
   uint32_t                   *puiLostData,              /* [out] */
   uint64_t                   *puiTimeStamp,             /* [out] */
   uint32_t                   *puiBytesCopiedOut         /* [out] */
   );

/**
Summary:
Create a kernel fence if supported.

Returns -1 if fences are not supported or a valid file descriptor if they are
**/
NEXUS_Error NEXUS_Graphicsv3d_FenceOpen(
   NEXUS_Graphicsv3dHandle          gfx,
   int *fd,                                        /* [out] file handle */
   uint64_t *p,                                    /* [out] internal fence */
   char cType,                                     /* [in] marker to distinguish where the fence was made */
   int iPid                                        /* [in] creator process - as it might not be this one */
   );

/**
Summary:
Async wait on a fence.
**/
NEXUS_Error NEXUS_Graphicsv3d_FenceWaitAsync(
   NEXUS_Graphicsv3dHandle          gfx,
   int fd,                                         /* [in] file handle */
   uint64_t *pV3dFence                             /* [out] internal async fence */
   );

/**
Summary:
Get milliseconds from when the module first started.  All time data
exported from the module is in this base.

Description:
returns number of microseconds since module boot.
**/
void NEXUS_Graphicsv3d_GetTime(
   uint64_t *pMicroseconds                         /* [out] time in microseconds */
   );
#ifdef __cplusplus
}
#endif

#endif /* NEXUS_GRAPHICSV3D_H__ */
