/***************************************************************************
 *     (c)2012 Broadcom Corporation
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
#ifndef BV3D_H__
#define BV3D_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bmma.h"
#include "bint.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "berr_ids.h"
#include "bchp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=Module Overview: ********************************************************
The purpose of this module is to take 'jobs' submitted by a user-mode client 
OpenGLES/VG driver and schedule them on the underlying V3D hardware. 

Multiple Processes
------------------

Multiple user-mode processes can be running OpenGLES/VG jobs simultaneously.
This module is designed to fairly schedule the jobs from multiple clients onto
the V3D hardware. It also manages a pool of memory that is used by the
binning hardware. This single pool is shared by all running clients.

Non Real-Time Client
--------------------

This is a "non real-time client" since its operations are not guaranteed to finish
within a fixed period of time. Jobs completion is notified via a callback mechanism.

****************************************************************************/


/***************************************************************************
Summary:
   The handle for the v3d graphics module.

Description:
   This is the main handle required to do any operations within the 
   v3d graphics module.

See Also:
   BV3D_Open
****************************************************************************/
typedef struct BV3D_P_Handle *BV3D_Handle;

/***************************************************************************
Summary:
   Specifies which hardware sources are signaling an interrupt.
****************************************************************************/
typedef enum BV3D_Signaller
{
   BV3D_Signaller_eBinSig    = 1 << 0, /* The bin unit is signaling         */
   BV3D_Signaller_eRenderSig = 1 << 1, /* The render unit is signaling      */
   BV3D_Signaller_eUserSig   = 1 << 2  /* The user-shader unit is signaling */
} BV3D_Signaller;

/***************************************************************************
Summary:
   Used to control the performance monitoring hardware counters.

Description:
   The hardware monitor counters can be enabled, disabled and reset using
   these values.

See Also:
   BV3D_SetPerformanceMonitor,
   BV3D_PerfMonitorSettings,
   BV3D_PerfMonitorData
****************************************************************************/
typedef enum BV3D_PerfMonitorFlags
{
   BV3D_PerfMonitor_None   = 0,        /* No action           */
   BV3D_PerfMonitor_Start  = 1 << 0,   /* Starts the counters */
   BV3D_PerfMonitor_Reset  = 1 << 1,   /* Resets the counters */
   BV3D_PerfMonitor_Stop   = 1 << 2    /* Stops the counters  */
} BV3D_PerfMonitorFlags;

/***************************************************************************
Summary:
   Used to hold a timestamp.

Description:

See Also:
   BV3D_TimelineData,
   BV3D_GetNotification
****************************************************************************/
typedef struct BV3D_EventTime
{
   uint32_t uiSecs;        /* Seconds        */
   uint32_t uiMicrosecs;   /* Microseconds   */
} BV3D_EventTime;

/***************************************************************************
Summary:
   Contains the time-stamp data for one debug timeline entry.

Description:
   If requested, a notification can contain debug timestamp data that can be
   used for performance tuning of a 3D application. This structure contains
   the start and end times for the operation of the hardware units for a
   particular job.

See Also:
   BV3D_EventTime,
   BV3D_GetNotification
****************************************************************************/
typedef struct BV3D_TimelineData
{
   BV3D_EventTime    sBinStart;     /* Binner start time       */
   BV3D_EventTime    sBinEnd;       /* Binner end time         */
   BV3D_EventTime    sRenderStart;  /* Renderer start time     */
   BV3D_EventTime    sRenderEnd;    /* Renderer end time       */
   BV3D_EventTime    sUserStart;    /* User-shader start time  */
   BV3D_EventTime    sUserEnd;      /* User-shader end time    */
} BV3D_TimelineData;

/***************************************************************************
Summary:
   Used to hold the data for an event notification.

Description:
   Notifications are queued when a callback from a job is issued.
   Clients can pop the notification queue when they get a callback so they 
   can find out what is being notified.

See Also:
   BV3D_TimelineData,
   BV3D_GetNotification
****************************************************************************/
typedef struct BV3D_Notification
{
   uint32_t          uiParam;
   uint32_t          uiSync;
   uint32_t          uiOutOfMemory;
   uint64_t          uiJobSequence;
   BV3D_TimelineData *pTimelineData;
} BV3D_Notification;

/***************************************************************************
Summary:
   Holds queried information about the 3D core.

Description:
   Filled by NEXUS_Graphicsv3d_GetInfo to report essential information about
   the core.

See Also:
   BV3D_GetInfo
****************************************************************************/
typedef struct BV3D_Info
{
   char        chName[10];              /* The core variant as a printable string */
   uint32_t    uiNumSlices;             /* Number of slices                       */
   uint32_t    uiTextureUnitsPerSlice;  /* Number of texture units per slice      */
   uint32_t    uiTechRev;               /* The technology revision number         */
   uint32_t    uiRevision;              /* The core revision number               */
   char        chRevStr[3];             /* The revision as a string               */
} BV3D_Info;

/***************************************************************************
Summary:
   Bin memory is requested using these settings.

Description:

See Also:
   BV3D_BinMemory,
   BV3D_GetBinMemory
****************************************************************************/
typedef struct BV3D_BinMemorySettings
{
   uint32_t    uiSize;     /* Size in bytes */
} BV3D_BinMemorySettings;

/***************************************************************************
Summary:
   Bin memory details filled in by BV3D_GetBinMemory.

Description:

See Also:
   BV3D_BinMemory,
   BV3D_GetBinMemory
****************************************************************************/
typedef struct BV3D_BinMemory
{
   uint32_t    uiAddress;  /* Device memory offset to the memory allocated  */
   uint32_t    uiSize;     /* The size in bytes of the memory block         */
} BV3D_BinMemory;

/***************************************************************************
Summary:
   Overflow code

Description:
   Defined when there is no more space in the job record

See Also:
****************************************************************************/
#define BV3D_ERR_JOB_OVERFLOW    BERR_MAKE_CODE(BERR_V3D_ID, 0)

/***************************************************************************
Summary:
   Opens the v3d graphics module.

Description:
   The module is opened and a v3d graphics module handle is created and 
   returned. This handle will be necessary to perform any tasks in the 
   v3d graphics module.

   The BV3D module should only be opened once.

Returns:
   BERR_SUCCESS - Module was opened and valid handle was returned.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.
   BERR_OUT_OF_SYSTEM_MEMORY - Not enough system memory to open the module.

See Also:
   BV3D_Close,
   BV3D_RegisterClient,
   BV3D_UnregisterClient
****************************************************************************/
BERR_Code BV3D_Open(
   BV3D_Handle *pv3d,            /* [out] Pointer to returned V3D handle.  */
   BCHP_Handle hChp,             /* [in] Chip handle.                      */
   BREG_Handle hReg,             /* [in] Register access handle.           */
   BMMA_Heap_Handle hMma,        /* [in] Memory allocation handle.         */
   uint64_t    uiHeapOffset,     /* [in] Used to patch the top two bits    */
   BINT_Handle bint,             /* [in] Interrupt handle.                 */
   uint32_t    uiBinMemMegs,     /* [in] Amount of memory for binning      */
   uint32_t    uiBinMemChunkPow, /* [in] (1 << pow) * 256k                 */
   bool        bDisableAQA,      /* [in] Disable adaptive QPU assignment   */
   uint32_t    uiClockFreq       /* [in] Clock freq <= requested,0=default */
);

/***************************************************************************
Summary:
   Closes the v3d graphics module.

Description:
   Once this function is called no more v3d graphics module functions can 
   be used. 

   Outstanding callbacks may be interrupted without callbacks being
   called.

Returns:
   BERR_SUCCESS - Module was opened and valid handle was returned.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
   BV3D_Open,
   BV3D_UnregisterClient
****************************************************************************/
BERR_Code BV3D_Close(
   BV3D_Handle v3d               /* [in] Handle to V3D module.             */
);

/***************************************************************************
Summary:
	Maximum number of instructions that a job can contain.
****************************************************************************/
#define V3D_JOB_MAX_INSTRUCTIONS 8

/***************************************************************************
Summary:
   Instruction codes for all valid job instructions.

Description:
   An instruction in a job must be one of these types.

See Also:
   BV3D_Instruction,
   BV3D_Job,
   BV3D_SendJob
****************************************************************************/
typedef enum BV3D_Operation
{
   BV3D_Operation_eEndInstr = 0,   /* Args none                            */
   BV3D_Operation_eBinInstr,       /* Args bin_start    and bin_end        */
   BV3D_Operation_eRenderInstr,    /* Args render_start and render_end     */
   BV3D_Operation_eUserInstr,      /*                                      */
   BV3D_Operation_eWaitInstr,      /* Args waitFlags                       */
   BV3D_Operation_eSyncInstr,      /* Args none                            */
   BV3D_Operation_eNotifyInstr,    /* Args notify flags                    */
   BV3D_Operation_eFenceInstr      /* Args fence                           */
} BV3D_Operation;

/***************************************************************************
Summary:
   A job instruction.

Description:
   A job consists of multiple instructions. Each instruction is one of these.
   Instructions are defined by an operation type, two optional arguments
   (which depend on the type of operation), and a callback parameter.

   If the callback parameter is non-zero, a notification will be triggered
   when the instruction completes.

See Also:
   BV3D_Operation,
   BV3D_Job,
   BV3D_SendJob
****************************************************************************/
typedef struct BV3D_Instruction
{
   BV3D_Operation       eOperation;       /* The opcode for this instruction                   */
   uint32_t             uiArg1;           /* First argument (if appropriate)                   */
   uint32_t             uiArg2;           /* Second argument (if appropriate)                  */
   uint32_t             uiCallbackParam;  /* Callback parameter (or 0 if no callback required) */
   struct BV3D_Job      *psJob;           /* A pointer to the owning job                       */
} BV3D_Instruction;

/***************************************************************************
Summary:
   The job structure used to submit jobs to the queue.

Description:
   Contains a list of instructions and the necessary data to complete the job.

See Also:
   BV3D_Instruction,
   BV3D_SendJob,
   BV3D_TimelineData
****************************************************************************/
typedef struct BV3D_Job
{
   BV3D_Instruction  sProgram[V3D_JOB_MAX_INSTRUCTIONS]; /* The list of instructions                 */
   uint32_t          uiCurrentInstr;         /* Index of current running instruction                 */
   uint32_t          uiClientId;             /* Unique id of client that submitted the job           */
   uint32_t          uiBinMemory;            /* Initial bin memory offset for tasks in this program  */
   uint32_t          uiUserVPM;              /* Settings for the V3D VPM for this job                */
   uint32_t          uiAbandon;              /* Set when job must be abandoned                       */
   uint32_t          uiOutOfMemory;          /* Set when job is abandoned due to out of memory       */
   uint32_t          uiInstrCount;           /* Count of currently unexecuted instructions           */
   uint32_t          uiOverspill;            /* Last overspill block used (recycled if out of mem)   */
   uint64_t          uiSequence;             /* Unique sequence number for this job                  */
   bool              bCollectTimeline;       /* Set when timeline data is wanted                     */
   uint32_t          uiNotifyCallbackParam;  /* Set internally for notify instruction implementation */
   uint64_t          uiNotifySequenceNum;    /* Set internally for notify instruction implementation */
   BV3D_TimelineData sTimelineData;          /* The timeline data that is being collected            */
   void              *psClientJob;           /* Opaque pointer to a client's view of the job         */
} BV3D_Job;

/***************************************************************************
Summary:
   Setup performance monitoring using these settings.

Description:
   Hardware counters are notionally split into 2 banks, only one of which
   can be active at a time. The same applies to memory counters.

   This structure selects which (if any) of the counter banks is active.

   uiFlags is a combination of values from BV3D_PerfMonitorFlags.

See Also:
   BV3D_PerfMonitorFlags,
   BV3D_SetPerformanceMonitor,
   BV3D_GetPerformanceData
****************************************************************************/
typedef struct BV3D_PerfMonitorSettings
{
   uint32_t    uiHWBank;        /* 0 = no bank, 1 = 1st bank, 2 = 2nd bank          */
   uint32_t    uiMemBank;       /* 0 = no bank, 1 = 1st bank, 2 = 2nd bank          */
   uint32_t    uiFlags;         /* Combination of values from BV3D_PerfMonitorFlags */
} BV3D_PerfMonitorSettings;

/***************************************************************************
Summary:
   Performance monitoring data returned by BV3D_GetPerformanceData.

Description:
   The current set of active performance monitor counters are returned in this
   structure by BV3D_GetPerformanceData.

See Also:
   BV3D_PerfMonitorFlags,
   BV3D_PerfMonitorSettings,
   BV3D_GetPerformanceData
****************************************************************************/
typedef struct BV3D_PerfMonitorData
{
   uint64_t    uiHwCounters[16];  /* The hardware counters for the currently selected h/w banks */
   uint64_t    uiMemCounters[2]; /* The counters for the selected memory monitors              */
} BV3D_PerfMonitorData;

/***************************************************************************
Summary:
   Client load data returned by BV3D_GetLoadData.

Description:
   Information on how much each client is loading the 3D core.

See Also:
   BV3D_GetLoadData
   BV3D_SetGatherLoadData
****************************************************************************/
typedef struct BV3D_ClientLoadData
{
   uint32_t uiClientId;
   uint32_t uiClientPID;
   uint32_t uiNumRenders;
   uint8_t  sRenderPercent;
} BV3D_ClientLoadData;

/***************************************************************************
Summary:
   Queue a job for the V3D hardware to execute at the earliest opportunity.

Description:
   Sends a job to the 3D job queuing system. The job will be executed as
   soon as the hardware is available (subject to fair scheduling with other
   processes).

   Any instructions in the job with are flagged to callback will do so as they
   complete. Likewise, any explicit notify instructions will callback as they
   complete.

Returns:
   BERR_SUCCESS - The job was successfully queued.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.
   BERR_OUT_OF_SYSTEM_MEMORY - Not enough system memory to schedule the job.

See Also:
   BV3D_Job,
   BV3D_GetNotification,
   BV3D_SendSync
****************************************************************************/
BERR_Code BV3D_SendJob(
   BV3D_Handle hV3d,                      /* [in] Handle to V3D module  */
   BV3D_Job    *psJob                     /* [in] Job record            */
);

/***************************************************************************
Summary:
   Get information about the type of 3D core in this chip.

Description:
   Fills in a BV3D_Info structure with relevant data about the 3D core.

Returns:
   BERR_SUCCESS - The information was retrieved.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
   BV3D_Info
****************************************************************************/
BERR_Code BV3D_GetInfo(
   BV3D_Handle hV3d,                      /* [in] Handle to V3D module. */
   BV3D_Info *pInfo                       /* [out] Pointer to completed dataset about v3d core revision */
);

/***************************************************************************
Summary:
   Turn load data collection on or off (defaults off).

Description:
   Turn load data collection on or off (defaults off).

Returns:
   BERR_SUCCESS - Collection mode adjusted.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
   BV3D_ClientLoadData
   BV3D_GetLoadData
****************************************************************************/
BERR_Code BV3D_SetGatherLoadData(
   BV3D_Handle          hV3d,            /* [in] Handle to V3D module. */
   bool                 bCollect         /* [in] True to turn on load data collection, false to turn off */
);

/***************************************************************************
Summary:
   Get information about the loading of individual V3D clients.

Description:
   Fills in a BV3D_LoadData structure with relevant data about the 3D core.

   Calling with pLoadData == NULL will simply return with pValidClients filled
   with the number of clients. You can then allocate enough space for the pLoadData
   and call again with valid buffers.

   The load data statistics will be reset when the client data has been retrieved.

Returns:
   BERR_SUCCESS - The information was retrieved.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
   BV3D_ClientLoadData
   BV3D_SetGatherLoadData
****************************************************************************/
BERR_Code BV3D_GetLoadData(
   BV3D_Handle         hV3d,             /* [in] Handle to V3D module. */
   BV3D_ClientLoadData *pLoadData,       /* [out] Array of client loading data */
   uint32_t            uiNumClients,     /* [in] How many client slots are available in the pData array */
   uint32_t            *pValidClients    /* [out] The actual number of valid clients */
);

/***************************************************************************
Summary:
	Pop the earliest pending notification from the queue of notifications.

Description:
   As job instructions complete, their notifications are added to a queue and 
   the callback that was registered in BV3D_RegisterClient is called. When
   a callback is received by the client, it should keep calling 
   BV3D_GetNotification repeatedly until it stops returning BERR_SUCCESS. 
   This will ensure that all notifications are retrieved since there may be 
   less actual callbacks than notifications in the queue.

Returns:
   BERR_SUCCESS - The notification was successfully popped.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
   BV3D_Notification,
   BV3D_RegisterClient,
   BV3D_SendJob
****************************************************************************/
BERR_Code BV3D_GetNotification(
   BV3D_Handle          hV3d,             /* [in] Handle to V3D module     */
   uint32_t             clientId,         /* [in] Client id                */
   BV3D_Notification    *notification     /* [out] Notification data       */
);

/***************************************************************************
Summary:
   Sends a synchronous acknowledgment scheduler.

Description:
   There are very few occasions that the V3D core has to wait for the client.
   When this is required, the client will call this function to tell the V3D 
   core to continue. When blocked in this way other processes will still 
   continue to run on the 3D core, so deadlock cannot occur.

   If the client decides that the job should be abandoned instead, it can 
   set the abandon flag.

Returns:
   BERR_SUCCESS - The sync was accepted.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
   BV3D_Notification,
   BV3D_GetNotification,
   BV3D_SendJob
****************************************************************************/
BERR_Code BV3D_SendSync(
   BV3D_Handle hV3d,                      /* [in] Handle to V3D module     */
   uint32_t    clientId,                  /* [in] client id                */
   bool        abandon                    /* [in] abandon job?             */
);

/***************************************************************************
Summary:
   Request initial bin memory for a job to use.

Description:
   Requests the initial block of bin memory that a job will use.
   Any further bin memory that is needed during job execution will be
   managed internally, not by client code.

Returns:
   BERR_SUCCESS - Bin memory was assigned.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.
   BERR_OUT_OF_SYSTEM_MEMORY - Not enough pool memory available.

See Also:
   BV3D_SendJob,
   BV3D_BinMemorySettings,
   BV3D_BinMemory
****************************************************************************/
BERR_Code BV3D_GetBinMemory(
   BV3D_Handle                   hV3d,       /* [in]  */
   const BV3D_BinMemorySettings  *settings,  /* [in]  */
   uint32_t                      clientId,   /* [in]  */
   BV3D_BinMemory                *mem        /* [out] */
);

/***************************************************************************
Summary:
   Sets the parameters for the performance monitoring module.

Description:
   Changes the performance monitoring mode of the 3D core.

Returns:
   BERR_SUCCESS - Bin memory was assigned.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
   BV3D_PerfMonitorSettings,
   BV3D_GetPerformanceData,
   BV3D_PerfMonitorData
****************************************************************************/
BERR_Code BV3D_SetPerformanceMonitor(
   BV3D_Handle                    hV3d,       /* [in]  */
   const BV3D_PerfMonitorSettings *settings   /* [out] */
);

/***************************************************************************
Summary:
   Gets performance data.

Description:
   Retrieves the current state of the performance counters in the 3D core.

Returns:
   BERR_SUCCESS - Bin memory was assigned.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
   BV3D_PerfMonitorSettings,
   BV3D_SetPerformanceMonitor,
   BV3D_PerfMonitorData
****************************************************************************/
BERR_Code BV3D_GetPerformanceData(
   BV3D_Handle            hV3d,  /* [in]  */
   BV3D_PerfMonitorData   *data  /* [out] */
);

/***************************************************************************
Summary:
   Register a new client with the V3D scheduler.

Description:
   The V3D scheduler is able to deal with multiple clients (user-processes)
   simultaneously. When a new client process wants to use the 3D core it
   must call this function to register itself. 

   The clientId returned by register client is system unique.

   The context and client values is passed back to the client cbFunc
   on a callback.

   It must also provide a callback function pointer which will be called
   whenever a notification is sent.

   The uiClientPID argument is used only for debug purposes when accessing
   general client load data.

   The client must call BV3D_UnregisterClient when it is closing in order
   to release any held resources.

Returns:
   BERR_SUCCESS - Client successfully registered.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.
   BERR_OUT_OF_SYSTEM_MEMORY - Not enough system memory available.

See Also:
   BV3D_UnregisterClient
****************************************************************************/
BERR_Code BV3D_RegisterClient(
   BV3D_Handle hV3d,                         /* [in]  */
   uint32_t    *puiClientId,                 /* [out] */
   void        *pContext,                    /* [in]  */
   void        (*cbFunc)(uint32_t, void *),  /* [in]  */
   uint32_t    uiClientPID                   /* [in]  */
);

/***************************************************************************
Summary:
   Unregister a client with the V3D scheduler.

Description:
   Tells the scheduler that this client has stopped using V3D and is most
   likely about to exit.

   Frees any resources associated with the given client.

Returns:
   BERR_SUCCESS - Client successfully unregistered.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
   BV3D_RegisterClient
****************************************************************************/
BERR_Code BV3D_UnregisterClient(
   BV3D_Handle hV3d,                         /* [in]  */
   uint32_t    uiClientId                    /* [in]  */
);

/***************************************************************************/

/***************************************************************************
Summary:
   Worker thread function.

Description:
   Since Magnum cannot create threads itself, and a worker thread is
   required by the BV3D module, the calling code must create a thread
   which will call this worker thread function.

   The void* parameter should point to a BV3D_WorkerSettings structure
   which contains the BV3D module handle and a sync event.

   Start the thread after calling BV3D_Open.

See Also:
   BV3D_WorkerSettings,
   BV3D_Open
****************************************************************************/
void BV3D_Worker(
   void * p                      /* [in] BV3D_WorkerSettings pointer.  Called as a worker thread function, never in the main thread */
);

/***************************************************************************
Summary:
   Settings used by the worker thread function.

See Also:
   BV3D_Worker
****************************************************************************/
typedef struct BV3D_WorkerSettings
{
   BV3D_Handle hV3d;              /* [in] Handle to V3D module. */
   BKNI_EventHandle hSync;        /* [in] sync object to make sure the starts/terminates correctly */
} BV3D_WorkerSettings;

/***************************************************************************
Summary:
   Power the core down.

Description:
   The v3d is a frame renderer.  Wait until the pipeline is drained before
   shutting the core down.

Returns:
   BERR_SUCCESS - Client successfully unregistered.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
****************************************************************************/
BERR_Code BV3D_Standby(
   BV3D_Handle hV3d              /* [in] Handle to V3D module. */
);

/***************************************************************************
Summary:
   Power up core down.

Description:
   This is an immediate restoration of the v3d.

Returns:
   BERR_SUCCESS - Client successfully unregistered.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
****************************************************************************/
BERR_Code BV3D_Resume(
   BV3D_Handle hV3d              /* [in] Handle to V3D module. */
);

/***************************************************************************
Summary:
Make a fence object

Description:
Will return -1 if the kernel doesnt support fences.  Returns a valid fd
otherwise.

Returns:
BERR_SUCCESS - Client successfully unregistered.
BERR_OUT_OF_SYSTEM_MEMORY - No more resource.
BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
****************************************************************************/
BERR_Code BV3D_FenceOpen(
   BV3D_Handle          hV3d,              /* [in] Handle to V3D module. */
   uint32_t             uiClientId,
   int                  *pFence
);

/***************************************************************************
Summary:
Step master timeline

Description:
Will clock the timeline inside the BV3D module.  Any fences waiting on this
should progress

Returns:
BERR_SUCCESS.
BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
****************************************************************************/
BERR_Code BV3D_FenceSignal(
   BV3D_Handle          hV3d,
   int                  fd
);

/***************************************************************************
Summary:
Close a fence

Description:
This removes an untriggered fence from the system

Returns:

See Also:
****************************************************************************/
void BV3D_FenceClose(
   BV3D_Handle          hV3d,
   int                  fd
);

/***************************************************************************
Summary:
Get the monotonic time value

Description:

Returns:

See Also:
****************************************************************************/
BERR_Code BV3D_GetTime(
   uint64_t *pMicroseconds    /* [in] Result                           */
   );

extern BERR_Code BV3D_P_OsInit(BV3D_Handle hV3d);
extern void BV3D_P_OsUninit(BV3D_Handle hV3d);
#if BCHP_CHIP == 11360
extern void BV3D_P_OsSoftReset(BV3D_Handle hV3d);
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BV3D_H__ */

/* end of file */
