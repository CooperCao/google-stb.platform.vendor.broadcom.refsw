/***************************************************************************
 *     Broadcom Proprietary and Confidential. (c)2012-13 Broadcom.  All rights reserved.
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
   uint32_t                         uiParam;          /* The notification parameter that was part of the job */
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

/**
Summary:
Setup performance monitoring using these settings.
**/
typedef struct NEXUS_Graphicsv3dPerfMonitorSettings
{
   uint32_t    uiHwBank;        /* 0 = no bank, 1 = 1st bank, 2 = 2nd bank         */
   uint32_t    uiMemBank;       /* 0 = no bank, 1 = 1st bank, 2 = 2nd bank         */
   uint32_t    uiFlags;         /* Bitwise or of flags in BEGL_HWPerfMonitorFlags  */
} NEXUS_Graphicsv3dPerfMonitorSettings;

/**
Summary:
Performance monitoring data returned by NEXUS_Graphicsv3d_GetPerformanceData.
**/
typedef struct NEXUS_Graphicsv3dPerfMonitorData
{
   uint64_t    uiHwCounters[16];      /* The hardware counters for the currently selected h/w banks */
   uint64_t    uiMemCounters[2];      /* The counters for the memory monitors                       */
} NEXUS_Graphicsv3dPerfMonitorData;

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
   uint32_t                   uiArg2;           /* Second argument (if appropriate)                  */
   uint32_t                   uiCallbackParam;  /* Callback parameter (or 0 if no callback required) */
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
Create a GraphicsV3D interface for OpenGLES and OpenVG use.

Description:
This module is only designed to be used by the OpenGLES & OpenVG driver.
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

/**
Summary:
Sets the parameters for the performance monitoring module.

Description:
Changes the performance monitoring modes of the 3D core.
**/
void NEXUS_Graphicsv3d_SetPerformanceMonitor(
   NEXUS_Graphicsv3dHandle                      hGfx,
   const NEXUS_Graphicsv3dPerfMonitorSettings   *psSettings
   );

/**
Summary:
Gets performance data.

Description:
Retrieves the current state of the performance counters in the 3D core.
**/
void NEXUS_Graphicsv3d_GetPerformanceData(
   NEXUS_Graphicsv3dHandle            hGfx,
   NEXUS_Graphicsv3dPerfMonitorData   *psSettings /* [out] */
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
   uint32_t                         *pValidClients
   );

/**
Summary:
Create a kernel fence if supported.

Returns -1 if fences are not supported or a valid file descriptor if they are
**/
NEXUS_Error NEXUS_Graphicsv3d_FenceOpen(
   NEXUS_Graphicsv3dHandle          gfx,
   int *fd                                         /* [out] file handle */
   );

/**
Summary:
Move the fence timeline
**/
NEXUS_Error NEXUS_Graphicsv3d_FenceSignal(
   int fd
   );

/**
Summary:
Close a fence.  Done automatically when it triggers, but the user
may want to kill an unsignalled fence.
**/
void NEXUS_Graphicsv3d_FenceClose(
   int fd
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
