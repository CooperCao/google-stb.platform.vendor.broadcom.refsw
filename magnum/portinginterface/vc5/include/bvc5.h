/******************************************************************************
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
 ******************************************************************************/
#ifndef BVC5_H__
#define BVC5_H__

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


****************************************************************************/

#define BVC5_MAX_BIN_SUBJOBS           8
#define BVC5_MAX_RENDER_SUBJOBS        16
#define BVC5_MAX_QPU_SUBJOBS           12
#define BVC5_MAX_DEPENDENCIES          8
#define BVC5_MAX_CORES                 8
#define BVC5_MAX_IDENTS                4
#define BVC5_MAX_HUB_IDENTS            4

#define BVC5_EMPTY_TILE_MODE_NONE      0u
#define BVC5_EMPTY_TILE_MODE_SKIP      1u
#define BVC5_EMPTY_TILE_MODE_FILL      2u

/* Workaround flags */
#define BVC5_NO_BIN_RENDER_OVERLAP     1
#define BVC5_GFXH_1181                 2

/* Cache operations */
#define BVC5_CACHE_CLEAR_SIC  (1 << 0)
#define BVC5_CACHE_CLEAR_SUC  (1 << 1)
#define BVC5_CACHE_CLEAR_L1TD (1 << 2)
#define BVC5_CACHE_CLEAR_L1TC (1 << 3)
#define BVC5_CACHE_CLEAR_VCD  (1 << 4)
#define BVC5_CACHE_CLEAR_L2C  (1 << 5)
#define BVC5_CACHE_FLUSH_L2T  (1 << 6)
#define BVC5_CACHE_CLEAN_L1TD (1 << 7)
#define BVC5_CACHE_CLEAN_L2T  (1 << 8)
#define BVC5_CACHE_CLEAR_GCA  (1 << 9)
#define BVC5_CACHE_FLUSH_L3C  (1 << 10)
#define BVC5_CACHE_CLEAN_L3C  (1 << 11)

/***************************************************************************
Summary:
   The handle for the VC5 graphics module.

Description:
   This is the main handle required to do any operations within the
   VC5 graphics module.

See Also:
   BVC5_Open
****************************************************************************/
typedef struct BVC5_P_Handle *BVC5_Handle;

/***************************************************************************
Summary:
   Specifies which hardware sources are signaling an interrupt.
****************************************************************************/
typedef enum BVC5_Signaller
{
   BVC5_Signaller_eBinSig    = 1 << 0, /* The bin unit is signaling         */
   BVC5_Signaller_eRenderSig = 1 << 1, /* The render unit is signaling      */
   BVC5_Signaller_eUserSig   = 1 << 2  /* The user-shader unit is signaling */
} BVC5_Signaller;

/***************************************************************************
Summary:
   Holds queried information about the 3D core.

Description:
   Filled by BVC5_GetInfo to report essential information about
   the core.

See Also:
   BVC5_GetInfo
****************************************************************************/
typedef struct BVC5_Info
{
   uint32_t    uiIdent[BVC5_MAX_CORES * BVC5_MAX_IDENTS];
   uint32_t    uiHubIdent[BVC5_MAX_HUB_IDENTS];
   uint32_t    uiDDRMapVer;
} BVC5_Info;

typedef enum BVC5_JobType
{
   BVC5_JobType_eNull,
   BVC5_JobType_eBin,
   BVC5_JobType_eRender,
   BVC5_JobType_eUser,
   BVC5_JobType_eTFU,
   BVC5_JobType_eFenceWait,
   BVC5_JobType_eTest,
   BVC5_JobType_eUsermode,
   BVC5_JobType_eBarrier,
   BVC5_JobType_eWaitOnEvent,
   BVC5_JobType_eSetEvent,
   BVC5_JobType_eResetEvent,
   BVC5_JobType_eNumJobTypes
} BVC5_JobType;

typedef struct BVC5_SchedDependencies
{
   uint64_t                uiDep[BVC5_MAX_DEPENDENCIES];
   uint32_t                uiNumDeps;
} BVC5_SchedDependencies;

typedef struct BVC5_SyncListItem
{
   int32_t                 iMemHandle;
   void                    *pMemSyncPtr;
   uint32_t                uiFlag;
   uint32_t                uiOffset;
   uint32_t                uiLength;
} BVC5_SyncListItem;

typedef struct BVC5_QueryResponse
{
   uint32_t uiStateAchieved;
} BVC5_QueryResponse;

typedef enum BVC5_JobStatus
{
   BVC5_JobStatus_eSUCCESS,
   BVC5_JobStatus_eOUT_OF_MEMORY,
   BVC5_JobStatus_eERROR
} BVC5_JobStatus;

typedef struct BVC5_Completion
{
   uint64_t          uiJobId;
   uint64_t          uiCallback;
   uint64_t          uiData;
   BVC5_JobStatus    eStatus;
   BVC5_JobType      eType;
} BVC5_Completion;

typedef struct BVC5_CompletionInfo
{
   uint64_t          uiOldestNotFinalized;
} BVC5_CompletionInfo;

typedef struct BVC5_JobBase
{
   uint64_t                uiJobId;
   BVC5_JobType            eType;
   BVC5_SchedDependencies  sCompletedDependencies;
   BVC5_SchedDependencies  sFinalizedDependencies;
   uint64_t                uiCompletion;
   uint64_t                uiData;
   uint32_t                uiCacheOps;
   bool                    bSecure;
   uint64_t                uiPagetablePhysAddr;
   uint32_t                uiMmuMaxVirtAddr;
} BVC5_JobBase;

typedef struct BVC5_JobNull
{
   BVC5_JobBase            sBase;
} BVC5_JobNull;

typedef struct BVC5_JobBin
{
   BVC5_JobBase            sBase;
   uint32_t                uiNumSubJobs;
   uint32_t                uiStart[BVC5_MAX_BIN_SUBJOBS];
   uint32_t                uiEnd[BVC5_MAX_BIN_SUBJOBS];
   uint32_t                uiOffset;
   uint32_t                uiFlags;
   uint32_t                uiMinInitialBinBlockSize;
   uint32_t                uiTileStateSize;
} BVC5_JobBin;

typedef struct BVC5_JobRender
{
   BVC5_JobBase            sBase;
   uint32_t                uiNumSubJobs;
   uint32_t                uiStart[BVC5_MAX_RENDER_SUBJOBS];
   uint32_t                uiEnd[BVC5_MAX_RENDER_SUBJOBS];
   uint32_t                uiFlags;
   uint32_t                uiEmptyTileMode;
} BVC5_JobRender;

typedef struct BVC5_JobBarrier
{
   BVC5_JobBase            sBase;
} BVC5_JobBarrier;

typedef struct BVC5_JobUser
{
   BVC5_JobBase            sBase;
   uint32_t                uiNumSubJobs;
   uint32_t                uiPC[BVC5_MAX_QPU_SUBJOBS];
   uint32_t                uiUnif[BVC5_MAX_QPU_SUBJOBS];
} BVC5_JobUser;

typedef struct BVC5_JobFenceWait
{
   BVC5_JobBase            sBase;
   int32_t                 iFence;
} BVC5_JobFenceWait;

typedef struct BVC5_JobTFU
{
   BVC5_JobBase            sBase;

   struct
   {
      uint16_t    uiTextureType;
      uint16_t    uiByteFormat;
      uint16_t    uiEndianness;
      uint16_t    uiComponentOrder;
      uint32_t    uiRasterStride;
      uint32_t    uiChromaStride;
      uint32_t    uiAddress;
      uint32_t    uiChromaAddress;
      uint32_t    uiUPlaneAddress;
      uint64_t    uiFlags;
   } sInput;

   struct
   {
      uint32_t    uiMipmapCount;
      uint32_t    uiVerticalPadding;
      uint32_t    uiWidth;
      uint32_t    uiHeight;
      uint16_t    uiEndianness;
      uint16_t    uiByteFormat;
      uint32_t    uiAddress;
      uint32_t    uiFlags;
   } sOutput;

   struct
   {
      uint16_t    uiY;
      uint16_t    uiRC;
      uint16_t    uiBC;
      uint16_t    uiGC;
      uint16_t    uiRR;
      uint16_t    uiGR;
      uint16_t    uiGB;
      uint16_t    uiBB;
   } sCustomCoefs;
} BVC5_JobTFU;

typedef struct BVC5_JobTest
{
   BVC5_JobBase            sBase;
   uint32_t                uiDelay;
} BVC5_JobTest;

typedef struct BVC5_JobUsermode
{
   BVC5_JobBase            sBase;
   uint64_t                uiUsermode;
   uint64_t                uiData;
} BVC5_JobUsermode;

typedef struct BVC5_JobSchedJob
{
   BVC5_JobBase            sBase;
   uint64_t                uiEventId;
} BVC5_JobSchedJob;

/**
Summary:
Usermode callback jobs record returned by query function NEXUS_Graphicsv3d_GetUsermode
**/
typedef struct BVC5_Usermode
{
   uint64_t          uiJobId;
   uint64_t          uiCallback;
   uint64_t          uiData;
   bool              bHaveJob;
} BVC5_Usermode;

/**************************************************************************
Summary:
   Structure used to communicate open options
***************************************************************************/
typedef struct BVC5_OpenParameters
{
   bool  bUsePowerGating;    /* Use power gating or not       */
   bool  bUseClockGating;    /* Use clock gating or not       */
   bool  bUseNexusMMA;       /* Use Nexus MMA for allocations */
   bool  bUseStallDetection; /* Use watchdog stall detection  */
   bool  bGPUMonDeps;        /* Report dependency information to GPUMonitor  */
   bool  bNoQueueAhead;      /* Prevent queue ahead of render jobs           */
   bool  bResetOnStall;      /* Reset & recover when a h/w stall is detected */
   bool  bMemDumpOnStall;    /* Dump heap when a stall is detected            */
   bool  bNoBurstSplitting;  /* Disable burst splitting in the wrapper? */
   uint32_t uiDRMDevice;     /* DRM device to open if signalling client termination */
} BVC5_OpenParameters;

/**************************************************************************
Summary:
   Structure used to hold callbacks to Nexus or other containing layer
***************************************************************************/
typedef struct BVC5_Callbacks
{
   void (*fpUsermodeHandler)(void *pVc5);    /* Issue a usermode callback           */
   void (*fpCompletionHandler)(void *pVC5);  /* Issue a completion callback         */
   void (*fpSecureToggleHandler)(bool bEnter);  /* Transition to/from secure operation */
} BVC5_Callbacks;

/***************************************************************************
Summary:
   Fills in the default open parameters for the module
***************************************************************************/
void BVC5_GetDefaultOpenParameters(
   BVC5_OpenParameters *openParams
);

/***************************************************************************
Summary:
   Opens the vc5 graphics module.

Description:
   The module is opened and a VC5 graphics module handle is created and
   returned. This handle will be necessary to perform any tasks in the
   vc5 graphics module.

   The BVC5 module should only be opened once.

Returns:
   BERR_SUCCESS - Module was opened and valid handle was returned.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.
   BERR_OUT_OF_SYSTEM_MEMORY - Not enough system memory to open the module.

See Also:
   BVC5_Close,
   BVC5_RegisterClient,
   BVC5_UnregisterClient
****************************************************************************/
BERR_Code BVC5_Open(
   BVC5_Handle         *phVC5,        /* [out] Pointer to returned VC5 handle.  */
   BCHP_Handle          hChp,         /* [in] Chip handle.                      */
   BREG_Handle          hReg,         /* [in] Register access handle.           */
   BMMA_Heap_Handle     hMMAHeap,     /* [in] Unsecure heap handles.            */
   BMMA_Heap_Handle     hSecureMMAHeap, /* [in] Secure heap handles.              */
   uint64_t             ulDbgHeapOffset,/* [in] used for debug memory dump only   */
   unsigned             uDbgHeapSize, /* [in] used for debug memory dump only   */
   BINT_Handle          bint,         /* [in] Interrupt handle.                 */
   BVC5_OpenParameters *sOpenParams,  /* [in] Options                           */
   BVC5_Callbacks      *sCallbacks    /* [in] Callback fn pointers              */
);

/***************************************************************************
Summary:
   Closes the vc5 graphics module.

Description:
   Once this function is called no more VC5 graphics module functions can
   be used.

   Outstanding callbacks may be interrupted without callbacks being
   called.

Returns:
   BERR_SUCCESS - Module was opened and valid handle was returned.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
   BVC5_Open,
   BVC5_UnregisterClient
****************************************************************************/
BERR_Code BVC5_Close(
   BVC5_Handle hVC5              /* [in] Handle to VC5 module.             */
);

/***************************************************************************
Summary:
   Register a new client with the VC5 scheduler.

Description:
   The VC5 scheduler is able to deal with multiple clients (user-processes)
   simultaneously. When a new client process wants to use the 3D core it
   must call this function to register itself.

   The clientId returned by register client is system unique.

   The client must call BVC5_UnregisterClient when it is closing in order
   to release any resources held.

Returns:
   BERR_SUCCESS - Client successfully registered.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.
   BERR_OUT_OF_SYSTEM_MEMORY - Not enough system memory available.

See Also:
   BVC5_UnregisterClient
****************************************************************************/
BERR_Code BVC5_RegisterClient(
   BVC5_Handle  hVC5,
   void        *pContext,
   uint32_t    *puiClientId,
   int64_t      iUnsecureBinTranslation,
   int64_t      iSecureBinTranslation,
   uint64_t     uiPlatformToken,
   uint32_t     uiClientPID
);

/***************************************************************************
Summary:
   Unregister a client with the VC5 scheduler.

Description:
   Tells the scheduler that this client has stopped using VC5 and is most
   likely about to exit.

   Frees any resources associated with the given client.

Returns:
   BERR_SUCCESS - Client successfully unregistered.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
   BVC5_RegisterClient
****************************************************************************/
BERR_Code BVC5_UnregisterClient(
   BVC5_Handle hVC5,
   uint32_t    uiClientId
);

/***************************************************************************/

/***************************************************************************
Summary:
   Worker thread function.

Description:
   Since Magnum cannot create threads itself, and a worker thread is
   required by the BVC5 module, the calling code must create a thread
   which will call this worker thread function.

   The void* parameter should point to a BVC5_SchedulerSettings structure
   which contains the BVC5 module handle and a sync event.

   Start the thread AFTER calling BVC5_Open.

See Also:
   BVC5_SchedulerSettings,
   BVC5_Open
****************************************************************************/
void BVC5_Scheduler(
   void * p                      /* [in] BVC5_SchedulerSettings pointer.  Called as a worker thread function, never in the main thread */
);

/***************************************************************************
Summary:
   Settings used by the worker thread function.

See Also:
   BVC5_Worker
****************************************************************************/
typedef struct BVC_SchedulerSettings
{
   BVC5_Handle      hVC5;         /* [in] Handle to VC5 module. */
   BKNI_EventHandle hSyncEvent;   /* [in] sync object to make sure the starts/terminates correctly */
} BVC5_SchedulerSettings;

/***************************************************************************
Summary:
   Power down core.

Description:
   The VC5 is a frame renderer.  Wait until the pipeline is drained before
   shutting the core down.

Returns:
   BERR_SUCCESS - Client successfully unregistered.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
****************************************************************************/
BERR_Code BVC5_Standby(
   BVC5_Handle hVC5              /* [in] Handle to VC5 module. */
);

/***************************************************************************
Summary:
   Power up core.

Description:
   This is an immediate restoration of the VC5.

Returns:
   BERR_SUCCESS - Client successfully unregistered.
   BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
****************************************************************************/
BERR_Code BVC5_Resume(
   BVC5_Handle hVC5                       /* [in] Handle to VC5 module. */
   );

BERR_Code BVC5_BinRenderJob(
   BVC5_Handle                 hVC5,            /* [in] Handle to VC5 module. */
   uint32_t                    uiClientId,      /* [in]                       */
   const BVC5_JobBin          *pBin,            /* [in]                       */
   const BVC5_JobRender       *pRender          /* [in]                       */
   );

BERR_Code BVC5_FenceWaitJob(
   BVC5_Handle                 hVC5,       /* [in] Handle to VC5 module. */
   uint32_t                    uiClientId, /* [in]                       */
   const BVC5_JobFenceWait    *pWait       /* [in]                       */
   );

BERR_Code BVC5_NullJob(
   BVC5_Handle                 hVC5,       /* [in] Handle to VC5 module. */
   uint32_t                    uiClientId, /* [in]                       */
   const BVC5_JobNull         *pNull       /* [in]                       */
   );

BERR_Code BVC5_BarrierJob(
   BVC5_Handle                 hVC5,       /* [in] Handle to VC5 module  */
   uint32_t                    uiClientId, /* [in]                       */
   const BVC5_JobBarrier      *pBarrierJobs/* [in]                       */
   );

BERR_Code BVC5_TFUJobs(
   BVC5_Handle                 hVC5,       /* [in] Handle to VC5 module  */
   uint32_t                    uiClientId, /* [in]                       */
   uint32_t                    uiNumJobs,  /* [in]                       */
   const BVC5_JobTFU          *pTFUJobs    /* [in]                       */
   );

BERR_Code BVC5_TestJob(
   BVC5_Handle                 hVC5,       /* [in] Handle to VC5 module  */
   uint32_t                    uiClientId, /* [in]                       */
   const BVC5_JobTest         *pTest       /* [in]                       */
   );

BERR_Code BVC5_UsermodeJob(
   BVC5_Handle                 hVC5,       /* [in] Handle to VC5 module  */
   uint32_t                    uiClientId, /* [in]                       */
   const BVC5_JobUsermode     *pUsermode   /* [in]                       */
   );

/* Will call pfnCallback(pContext, pParam) when signalled */
BERR_Code BVC5_FenceRegisterWaitCallback(
   BVC5_Handle                 hVC5,                          /* [in]          */
   int                         iFence,                        /* [in]          */
   uint32_t                    uiClientId,                    /* [in]          */
   void                      (*pfnCallback)(void *, uint64_t),/* [in]          */
   void                       *pContext,                      /* [in]          */
   uint64_t                    uiParam                        /* [in]          */
   );

BERR_Code BVC5_FenceUnregisterWaitCallback(
   BVC5_Handle                 hVC5,                          /* [in]          */
   int                         iFence,                        /* [in]          */
   uint32_t                    uiClientId,                    /* [in]          */
   void                      (*pfnCallback)(void *, uint64_t),/* [in]          */
   void                       *pContext,                      /* [in]          */
   uint64_t                    uiParam,                       /* [in]          */
   bool                       *bSignalled                     /* [out]         */
   );

BERR_Code BVC5_Query(
   BVC5_Handle                 hVC5,                   /* [in]          */
   uint32_t                    uiClientId,             /* [in]          */
   BVC5_SchedDependencies     *pCompletedDeps,         /* [in]          */
   BVC5_SchedDependencies     *pFinalizedDeps,         /* [in]          */
   BVC5_QueryResponse         *pResponse               /* [out]         */
   );

BERR_Code BVC5_MakeFenceForJobs(
   BVC5_Handle                   hVC5,             /* [in] */
   uint32_t                      uiClientId,       /* [in] */
   const BVC5_SchedDependencies *pCompletedDeps,   /* [in] */
   const BVC5_SchedDependencies *pFinalizedDeps,   /* [in] */
   bool                          bForceCreate,     /* [in] */
   int                          *piFence           /* [out] */
   );

BERR_Code BVC5_MakeFenceForAnyNonFinalizedJob(
   BVC5_Handle hVC5,       /* [in] */
   uint32_t    uiClientId, /* [in] */
   int        *piFence     /* [out] */
   );

BERR_Code BVC5_MakeFenceForAnyJob(
   BVC5_Handle                   hVC5,             /* [in] */
   uint32_t                      uiClientId,       /* [in] */
   const BVC5_SchedDependencies *pCompletedDeps,   /* [in] */
   const BVC5_SchedDependencies *pFinalizedDeps,   /* [in] */
   int                          *piFence           /* [out] */
   );

BERR_Code BVC5_FenceKeep(
   BVC5_Handle                 hVC5,                   /* [in]           */
   int                         iFence                  /* [in]           */
   );

BERR_Code BVC5_FenceSignal(
   BVC5_Handle                 hVC5,                   /* [in]           */
   int                         iFence                  /* [in]           */
   );

BERR_Code BVC5_FenceClose(
   BVC5_Handle                 hVC5,                   /* [in]           */
   int                         iFence                  /* [in]           */
   );

/* BVC5_FenceMakeLocal

   Create a fence belonging to a particular client.  This fence will be
   destroyed with the client, so is not suitable for sharing between
   processes.

 */
BERR_Code BVC5_FenceMakeLocal(
   BVC5_Handle                 hVC5,                   /* [in]           */
   uint32_t                    uiClientId,             /* [in]           */
   int                        *piFence                 /* [out]          */
   );


/* BVC5_GetUsermode

 Fetch a usermode callback packet

 */
BERR_Code BVC5_GetUsermode(
   BVC5_Handle                 hVC5,                   /* [in]           */
   uint32_t                    uiClientId,             /* [in]           */
   uint64_t                    uiPrevJobId,            /* [in]           */
   BVC5_Usermode              *psUsermode              /* [out]          */
   );


/* BVC5_GetCompletions

 Fetch a batch of completion callback packets.
 This function must be called once to retrieve a set of completions,
 and then again once the finalizers for those have been run (with the
 finalized job list).

 */
BERR_Code BVC5_GetCompletions(
   BVC5_Handle          hGfx,
   uint32_t             uiClientId,
   uint32_t             uiNumFinalizedJobs,
   const uint64_t       *puiFinalizedJobs,
   uint32_t             uiMaxCompletionsOut,
   BVC5_CompletionInfo  *psCompletionInfo,
   uint32_t             *puiCompletionsOut,
   BVC5_Completion      *psCompletions
   );

/* BVC5_GetInfo
 */
void BVC5_GetInfo(
   BVC5_Handle                 hVC5,                   /* [in]           */
   BVC5_Info                  *psInfo                  /* [out]          */
   );

/* BVC5_HasBrcmv3dko
 */
bool BVC5_HasBrcmv3dko(
   void
   );

/************************************************************************/
/* Performance counters                                                 */
/************************************************************************/

#define BVC5_MAX_GROUP_NAME_LEN         64
#define BVC5_MAX_COUNTER_NAME_LEN       64
#define BVC5_MAX_COUNTER_UNIT_NAME_LEN  32
#define BVC5_MAX_COUNTERS_PER_GROUP     96

typedef enum BVC5_CounterState
{
   BVC5_CtrAcquire = 0,
   BVC5_CtrRelease = 1,
   BVC5_CtrStart   = 2,
   BVC5_CtrStop    = 3
} BVC5_CounterState;

typedef struct BVC5_CounterDesc
{
   char        caName[BVC5_MAX_COUNTER_NAME_LEN];
   char        caUnitName[BVC5_MAX_COUNTER_UNIT_NAME_LEN];
   uint64_t    uiMinValue;
   uint64_t    uiMaxValue;
   uint64_t    uiDenominator;
} BVC5_CounterDesc;

typedef struct BVC5_CounterGroupDesc
{
   char              caName[BVC5_MAX_GROUP_NAME_LEN];
   uint32_t          uiTotalCounters;
   uint32_t          uiMaxActiveCounters;
   BVC5_CounterDesc  saCounters[BVC5_MAX_COUNTERS_PER_GROUP];
} BVC5_CounterGroupDesc;

typedef struct BVC5_CounterSelector
{
   uint32_t    uiGroupIndex;
   uint32_t    uiEnable;
   uint32_t    uiaCounters[BVC5_MAX_COUNTERS_PER_GROUP];
   uint32_t    uiNumCounters;
} BVC5_CounterSelector;

typedef struct BVC5_Counter
{
   uint32_t   uiGroupIndex;
   uint32_t   uiCounterIndex;
   uint64_t   uiValue;
} BVC5_Counter;

void BVC5_GetPerfNumCounterGroups(
   BVC5_Handle  hVC5,
   uint32_t     *puiNumGroups
   );

void BVC5_GetPerfCounterDesc(
   BVC5_Handle             hVC5,
   uint32_t                uiGroup,
   uint32_t                uiCounter,
   BVC5_CounterDesc        *psDesc
   );

void BVC5_GetPerfCounterGroupInfo(
   BVC5_Handle             hVC5,
   uint32_t                uiGroup,
   uint32_t                uiGrpNameSize,
   char                    *chGrpName,
   uint32_t                *uiMaxActiveCounter,
   uint32_t                *uiTotalCounter
   );

BERR_Code BVC5_SetPerfCounting(
   BVC5_Handle             hVC5,
   uint32_t                uiClientId,
   BVC5_CounterState       eState
   );

void BVC5_ChoosePerfCounters(
   BVC5_Handle                hVC5,
   uint32_t                   uiClientId,
   const BVC5_CounterSelector *psSelector
   );

uint32_t BVC5_GetPerfCounterData(
   BVC5_Handle    hVC5,
   uint32_t       uiMaxCounters,
   uint32_t       uiResetCounts,
   BVC5_Counter   *psCounters
   );

typedef struct BVC5_ClientLoadData
{
   uint32_t uiClientId;
   uint32_t uiClientPID;
   uint32_t uiNumRenders;
   uint8_t  sRenderPercent;
} BVC5_ClientLoadData;

BERR_Code BVC5_SetGatherLoadData(
   BVC5_Handle    hVC5,
   bool           bCollect
);

BERR_Code BVC5_GetLoadData(
   BVC5_Handle          hVC5,
   BVC5_ClientLoadData *pLoadData,
   uint32_t             uiNumClients,
   uint32_t            *pValidClients
);

/************************************************************************/
/* Event timeline                                                       */
/************************************************************************/
#define BVC5_MAX_EVENT_STRING_LEN   64

typedef enum BVC5_EventState
{
   BVC5_EventAcquire = 0,
   BVC5_EventRelease = 1,
   BVC5_EventStart   = 2,
   BVC5_EventStop    = 3
} BVC5_EventState;

typedef enum BVC5_EventType
{
   BVC5_EventBegin   = 0,
   BVC5_EventEnd     = 1,
   BVC5_EventOneshot = 2
} BVC5_EventType;

typedef enum BVC5_FieldType
{
   BVC5_EventInt32   = 0,
   BVC5_EventUInt32  = 1,
   BVC5_EventInt64   = 2,
   BVC5_EventUInt64  = 3
} BVC5_FieldType;

typedef struct BVC5_EventDesc
{
   char     caName[BVC5_MAX_EVENT_STRING_LEN];
   uint32_t uiNumDataFields;
} BVC5_EventDesc;

typedef struct BVC5_EventFieldDesc
{
   char              caName[BVC5_MAX_EVENT_STRING_LEN];
   BVC5_FieldType    eDataType;
} BVC5_EventFieldDesc;

typedef struct BVC5_EventTrackDesc
{
   char     caName[BVC5_MAX_EVENT_STRING_LEN];
} BVC5_EventTrackDesc;

void BVC5_GetEventCounts(
   BVC5_Handle  hVC5,
   uint32_t     *uiNumTracks,
   uint32_t     *uiNumEvents
   );

BERR_Code BVC5_GetEventTrackInfo(
   BVC5_Handle           hVC5,
   uint32_t              uiTrack,
   BVC5_EventTrackDesc   *psTrackDesc
   );


BERR_Code BVC5_GetEventInfo(
   BVC5_Handle       hVC5,
   uint32_t          uiEvent,
   BVC5_EventDesc   *psEventDesc
   );


BERR_Code BVC5_GetEventDataFieldInfo(
   BVC5_Handle          hVC5,
   uint32_t             uiEvent,
   uint32_t             uiField,
   BVC5_EventFieldDesc  *psFieldDesc
   );

BERR_Code BVC5_SetEventCollection(
   BVC5_Handle       hVC5,
   uint32_t          uiClientId,
   BVC5_EventState   eState
   );

uint32_t BVC5_GetEventData(
   BVC5_Handle    hVC5,
   uint32_t       uiClientId,
   uint32_t       uiEventBufferBytes,
   void           *pvEventBuffer,
   uint32_t       *puiLostData,
   uint64_t       *puiTimeStamp
   );

/************************************************************************/
/* Scheduler Event Sync Object                                          */
/************************************************************************/

BERR_Code BVC5_SchedEventJob(
      BVC5_Handle                 hVC5,
      uint32_t                    uiClientId,
      const BVC5_JobSchedJob     *pSchedEvent
      );

BERR_Code BVC5_NewSchedEvent(
      BVC5_Handle          hVC5,
      uint32_t             uiClientId,
      uint64_t             *pSchedEventId
      );

BERR_Code BVC5_DeleteSchedEvent(
      BVC5_Handle          hVC5,
      uint32_t             uiClientId,
      uint64_t             uiSchedEventId
      );

BERR_Code BVC5_SetSchedEvent(
      BVC5_Handle          hVC5,
      uint32_t             uiClientId,
      uint64_t             uiSchedEventId
      );

BERR_Code BVC5_ResetSchedEvent(
      BVC5_Handle          hVC5,
      uint32_t             uiClientId,
      uint64_t             uiSchedEventId
      );

BERR_Code BVC5_QuerySchedEvent(
      BVC5_Handle          hVC5,
      uint32_t             uiClientId,
      uint64_t             uiSchedEventId,
      bool                 *bEventSet
      );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVC5_H__ */

/* end of file */
