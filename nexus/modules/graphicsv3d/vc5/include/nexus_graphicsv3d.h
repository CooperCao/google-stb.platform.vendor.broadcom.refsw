/***************************************************************************
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
#ifndef NEXUS_GRAPHICSV3D_H__
#define NEXUS_GRAPHICSV3D_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Handles used by the GraphicsVC5 interface.
**/
typedef struct NEXUS_Graphicsv3d *NEXUS_Graphicsv3dHandle;

#define NEXUS_GRAPHICSV3D_MAX_BIN_SUBJOBS       8
#define NEXUS_GRAPHICSV3D_MAX_RENDER_SUBJOBS    16
#define NEXUS_GRAPHICSV3D_MAX_QPU_SUBJOBS       12
#define NEXUS_GRAPHICSV3D_MAX_DEPENDENCIES      8
#define NEXUS_GRAPHICSV3D_MAX_COMPLETIONS       8
#define NEXUS_GRAPHICSV3D_MAX_CORES             8
#define NEXUS_GRAPHICSV3D_MAX_IDENTS            4
#define NEXUS_GRAPHICSV3D_MAX_HUB_IDENTS        4

#define NEXUS_GRAPHICSV3D_EMPTY_TILE_MODE_NONE 0
#define NEXUS_GRAPHICSV3D_EMPTY_TILE_MODE_SKIP 1
#define NEXUS_GRAPHICSV3D_EMPTY_TILE_MODE_FILL 2


/* Workaround flags */
#define NEXUS_GRAPHICSV3D_NO_BIN_RENDER_OVERLAP 1
#define NEXUS_GRAPHICSV3D_GFXH_1181             2


/* Cache operations */
#define NEXUS_GRAPHICSV3D_CACHE_CLEAR_SIC  (1 << 0)
#define NEXUS_GRAPHICSV3D_CACHE_CLEAR_SUC  (1 << 1)
#define NEXUS_GRAPHICSV3D_CACHE_CLEAR_L1TD (1 << 2)
#define NEXUS_GRAPHICSV3D_CACHE_CLEAR_L1TC (1 << 3)
#define NEXUS_GRAPHICSV3D_CACHE_CLEAR_VCD  (1 << 4)
#define NEXUS_GRAPHICSV3D_CACHE_CLEAR_L2C  (1 << 5)
#define NEXUS_GRAPHICSV3D_CACHE_FLUSH_L2T  (1 << 6)
#define NEXUS_GRAPHICSV3D_CACHE_CLEAN_L1TD (1 << 7)
#define NEXUS_GRAPHICSV3D_CACHE_CLEAN_L2T  (1 << 8)
#define NEXUS_GRAPHICSV3D_CACHE_CLEAR_GCA  (1 << 9)
#define NEXUS_GRAPHICSV3D_CACHE_FLUSH_L3C  (1 << 10)
#define NEXUS_GRAPHICSV3D_CACHE_CLEAN_L3C  (1 << 11)


/**
Summary:
Structure holds information about the 3D cores.

Description:
Filled by NEXUS_GraphicsVC5_GetInfo to report essential information about the cores.
**/
typedef struct NEXUS_Graphicsv3dInfo
{
   uint32_t    uiIdent[NEXUS_GRAPHICSV3D_MAX_CORES * NEXUS_GRAPHICSV3D_MAX_IDENTS];
   uint32_t    uiHubIdent[NEXUS_GRAPHICSV3D_MAX_HUB_IDENTS];
} NEXUS_Graphicsv3dInfo;

typedef enum NEXUS_Graphicsv3dJobType
{
   NEXUS_Graphicsv3dJobType_eNull,
   NEXUS_Graphicsv3dJobType_eBin,
   NEXUS_Graphicsv3dJobType_eRender,
   NEXUS_Graphicsv3dJobType_eUser,
   NEXUS_Graphicsv3dJobType_eTFU,
   NEXUS_Graphicsv3dJobType_eFenceWait,
   NEXUS_Graphicsv3dJobType_eTest,
   NEXUS_Graphicsv3dJobType_eUsermode,
   NEXUS_Graphicsv3dJobType_eBarrier,
   NEXUS_Graphicsv3dJobType_eNumJobTypes
} NEXUS_Graphicsv3dJobType;

/**
Summary:
An array of job dependencies.

Description:
Lists the jobs that must complete before the owning job can start.
**/
typedef struct NEXUS_Graphicsv3dSchedDependencies
{
   uint64_t uiDep[NEXUS_GRAPHICSV3D_MAX_DEPENDENCIES];
   uint32_t uiNumDeps;
} NEXUS_Graphicsv3dSchedDependencies;

typedef enum NEXUS_Graphicsv3dJobStatus
{
   NEXUS_Graphicsv3dJobStatus_eSUCCESS,
   NEXUS_Graphicsv3dJobStatus_eOUT_OF_MEMORY,
   NEXUS_Graphicsv3dJobStatus_eERROR
} NEXUS_Graphicsv3dJobStatus;

/**
Summary:
Used to return results of query function

Description:
Pass to NEXUS_Graphicsv3d_Query to receive status of specific job.
**/
typedef struct NEXUS_Graphicsv3dQueryResponse
{
   uint32_t uiStateAchieved;
} NEXUS_Graphicsv3dQueryResponse;

/**
Summary:
Structure returned from a completion callback query NEXUS_Graphicsv3d_GetCompletions
**/
typedef struct NEXUS_Graphicsv3dCompletion
{
   uint64_t                      uiJobId;
   uint64_t                      uiCallback;
   uint64_t                      uiData;
   NEXUS_Graphicsv3dJobStatus    eStatus;
   NEXUS_Graphicsv3dJobType      eType;
} NEXUS_Graphicsv3dCompletion;

/**
Summary:
Info structure returned from a completion callback query NEXUS_Graphicsv3d_GetCompletions
**/
typedef struct NEXUS_Graphicsv3dCompletionInfo
{
   uint64_t uiOldestNotFinalized;
} NEXUS_Graphicsv3dCompletionInfo;


/**
Summary:
The base data for all job types.

Description:
Contained at the start of all concrete jobs.
**/
typedef struct NEXUS_Graphicsv3dJobBase
{
   uint64_t                            uiJobId;
   NEXUS_Graphicsv3dJobType            eType;
   NEXUS_Graphicsv3dSchedDependencies  sCompletedDependencies;
   NEXUS_Graphicsv3dSchedDependencies  sFinalizedDependencies;
   uint64_t                            uiCompletion;
   uint64_t                            uiCompletionData;
   uint32_t                            uiCacheOps;
   bool                                bSecure;
   uint64_t                            uiPagetablePhysAddr;
   uint32_t                            uiMmuMaxVirtAddr;
} NEXUS_Graphicsv3dJobBase;

/**
Summary:
A null job (does nothing but can be used to chain dependencies)
**/
typedef struct NEXUS_Graphicsv3dJobNull
{
   NEXUS_Graphicsv3dJobBase            sBase;
} NEXUS_Graphicsv3dJobNull;

/**
Summary:
A binning job.
**/
typedef struct NEXUS_Graphicsv3dJobBin
{
   NEXUS_Graphicsv3dJobBase            sBase;
   uint32_t                            uiNumSubJobs;
   uint32_t                            uiStart[NEXUS_GRAPHICSV3D_MAX_BIN_SUBJOBS];
   uint32_t                            uiEnd[NEXUS_GRAPHICSV3D_MAX_BIN_SUBJOBS];
   uint32_t                            uiOffset;
   uint32_t                            uiFlags;
   uint32_t                            uiMinInitialBinBlockSize;
} NEXUS_Graphicsv3dJobBin;

/**
Summary:
A render job.
**/
typedef struct NEXUS_Graphicsv3dJobRender
{
   NEXUS_Graphicsv3dJobBase            sBase;
   uint32_t                            uiNumSubJobs;
   uint32_t                            uiStart[NEXUS_GRAPHICSV3D_MAX_RENDER_SUBJOBS];
   uint32_t                            uiEnd[NEXUS_GRAPHICSV3D_MAX_RENDER_SUBJOBS];
   uint32_t                            uiFlags;
   uint32_t                            uiEmptyTileMode;
} NEXUS_Graphicsv3dJobRender;

/**
Summary:
A barrier job.
**/
typedef struct NEXUS_Graphicsv3dJobBarrier
{
   NEXUS_Graphicsv3dJobBase            sBase;
} NEXUS_Graphicsv3dJobBarrier;

/**
Summary:
A user job.
**/
typedef struct NEXUS_Graphicsv3dJobUser
{
   NEXUS_Graphicsv3dJobBase            sBase;
   uint32_t                            uiNumSubJobs;
   uint32_t                            uiPC[NEXUS_GRAPHICSV3D_MAX_QPU_SUBJOBS];
   uint32_t                            uiUnif[NEXUS_GRAPHICSV3D_MAX_QPU_SUBJOBS];
} NEXUS_Graphicsv3dJobUser;

/**
Summary:
A fence wait job.

Description:
This job will only complete when the fence it contains has been signaled.
Any jobs which depend on this job will therefore block until the signal fires.
**/
typedef struct NEXUS_Graphicsv3dJobFenceWait
{
   NEXUS_Graphicsv3dJobBase            sBase;
   int32_t                             iFence;
} NEXUS_Graphicsv3dJobFenceWait;

/**
Summary:
A TFU job.
**/
typedef struct NEXUS_Graphicsv3dJobTFU
{
   NEXUS_Graphicsv3dJobBase            sBase;

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
} NEXUS_Graphicsv3dJobTFU;

/**
Summary:
A test job.
**/
typedef struct NEXUS_Graphicsv3dJobTest
{
   NEXUS_Graphicsv3dJobBase            sBase;
   uint32_t                            uiDelay;
} NEXUS_Graphicsv3dJobTest;

/**
Summary:
Usermode callback jobs record returned by query function NEXUS_Graphicsv3d_GetUsermode
**/
typedef struct NEXUS_Graphicsv3dUsermode
{
   uint64_t                      uiJobId;
   uint64_t                      uiCallback;
   uint64_t                      uiData;
   bool                          bHaveJob;
} NEXUS_Graphicsv3dUsermode;

/**
Summary:
Usermode callback jobs hold a closure to invoke back in user space
**/
typedef struct NEXUS_Graphicsv3dJobUsermode
{
   NEXUS_Graphicsv3dJobBase      sBase;
   uint64_t                      uiFunction;
   uint64_t                      uiData;
} NEXUS_Graphicsv3dJobUsermode;

/**
Summary:
Settings used when creating the V3D module
**/
typedef struct NEXUS_Graphicsv3dCreateSettings
{
   uint32_t             uiClientPID;         /* The process id of the client. Used for debug purposes when querying client loads */
   NEXUS_CallbackDesc   sUsermode;           /* Callbacks for user jobs            */
   NEXUS_CallbackDesc   sFenceDone;          /* Callbacks for fences               */
   NEXUS_CallbackDesc   sCompletion;         /* Callbacks for completion callbacks */
   int64_t              iUnsecureBinTranslation;
   int64_t              iSecureBinTranslation;
   uint64_t             uiPlatformToken;
} NEXUS_Graphicsv3dCreateSettings;

/**
Summary:
Get default settings for NEXUS_Graphicsv3d_Create
**/
void NEXUS_Graphicsv3d_GetDefaultCreateSettings(
   NEXUS_Graphicsv3dCreateSettings     *psSettings        /* [out] */
   );

/**
Summary:
Create a Graphicsv3d interface for OpenGLES and OpenVG use.

Description:
This module is only designed to be used by the OpenGLES & OpenVG driver.
It should not be used directly.
**/
NEXUS_Graphicsv3dHandle NEXUS_Graphicsv3d_Create(  /* attr{destructor=NEXUS_Graphicsv3d_Destroy}  */
   const NEXUS_Graphicsv3dCreateSettings *psSettings
   );

/**
Summary:
Close the Graphicsv3d interface.
**/
void NEXUS_Graphicsv3d_Destroy(
   NEXUS_Graphicsv3dHandle             hGfx           /* [in]  */
   );

/**
Summary:
Queue a bin render job.
**/
NEXUS_Error NEXUS_Graphicsv3d_QueueBinRender(
   NEXUS_Graphicsv3dHandle              hGfx,            /* [in]  */
   const NEXUS_Graphicsv3dJobBin       *pBin,            /* [in]  */
   const NEXUS_Graphicsv3dJobRender    *pRender          /* [in]  */
   );

/**
Summary:
Queue a bin job.
**/
NEXUS_Error NEXUS_Graphicsv3d_QueueBin(
   NEXUS_Graphicsv3dHandle              hGfx,         /* [in]  */
   const NEXUS_Graphicsv3dJobBin       *pBin          /* [in]  */
   );

/**
Summary:
Queue a render job.
**/
NEXUS_Error NEXUS_Graphicsv3d_QueueRender(
   NEXUS_Graphicsv3dHandle              hGfx,         /* [in]  */
   const NEXUS_Graphicsv3dJobRender    *pRender       /* [in]  */
   );

/**
Summary:
Queue a TFU job.
**/
/*
   NOTE:
   reserved hint is from maxBatchSize in sched_nexus.c
*/
NEXUS_Error NEXUS_Graphicsv3d_QueueTFU(
   NEXUS_Graphicsv3dHandle              hGfx,         /* [in]  */
   uint32_t                             uiNumJobs,    /* [in]  */
   const NEXUS_Graphicsv3dJobTFU       *pTFUJobs      /* [in]  attr{nelem=uiNumJobs;nelem_out=0;reserved=16} */
   );

/**
Summary:
Queue a barrier job.
**/
NEXUS_Error NEXUS_Graphicsv3d_QueueBarrier(
   NEXUS_Graphicsv3dHandle              hGfx,         /* [in]  */
   const NEXUS_Graphicsv3dJobBarrier   *pBarrier      /* [in]  */
   );

/**
Summary:
Queue a null job.
**/
NEXUS_Error NEXUS_Graphicsv3d_QueueNull(
   NEXUS_Graphicsv3dHandle              hGfx,         /* [in]  */
   const NEXUS_Graphicsv3dJobNull      *pNull         /* [in]  */
   );

/**
Summary:
Create a test job.

Description:
**/
NEXUS_Error NEXUS_Graphicsv3d_QueueTest(
   NEXUS_Graphicsv3dHandle                   hGfx,       /* [in]  */
   const NEXUS_Graphicsv3dJobTest           *pTest       /* [in]  */
   );

/**
Summary:
Create a usermode job.

Description:
**/
NEXUS_Error NEXUS_Graphicsv3d_QueueUsermode(
   NEXUS_Graphicsv3dHandle                   hGfx,       /* [in]  */
   const NEXUS_Graphicsv3dJobUsermode       *pUsermode   /* [in]  */
   );

/**
Summary:
Query for particular job completion
**/
NEXUS_Error NEXUS_Graphicsv3d_Query(
   NEXUS_Graphicsv3dHandle                   hGfx,             /* [in]  */
   const NEXUS_Graphicsv3dSchedDependencies *pCompletedDeps,   /* [in]  attr{null_allowed=y} */
   const NEXUS_Graphicsv3dSchedDependencies *pFinalizedDeps,   /* [in]  attr{null_allowed=y} */
   NEXUS_Graphicsv3dQueryResponse           *pResponse         /* [out] */
   );

/**
Summary:
Create a fence to wait for the specified jobs
**/
NEXUS_Error NEXUS_Graphicsv3d_MakeFenceForJobs(
   NEXUS_Graphicsv3dHandle                   hGfx,             /* [in]  */
   const NEXUS_Graphicsv3dSchedDependencies *pCompletedDeps,   /* [in]  */
   const NEXUS_Graphicsv3dSchedDependencies *pFinalizedDeps,   /* [in]  */
   bool                                      bForceCreate,     /* [in]  */
   int                                      *piFence           /* [out] */
   );

/**
Summary:
Create a fence to wait for any non-finalized job
**/
NEXUS_Error NEXUS_Graphicsv3d_MakeFenceForAnyNonFinalizedJob(
   NEXUS_Graphicsv3dHandle hGfx,    /* [in]  */
   int                     *piFence /* [out] */
   );

/**
Summary:
Queue a fence wait job.

Description:
This job will not complete until the fence has been signaled.
**/
NEXUS_Error NEXUS_Graphicsv3d_QueueFenceWait(
   NEXUS_Graphicsv3dHandle                   hGfx,       /* [in]  */
   const NEXUS_Graphicsv3dJobFenceWait      *pJob        /* [in]  */
   );

/**
Summary:
Create a new fence object.
**/
NEXUS_Error NEXUS_Graphicsv3d_FenceMake(
   NEXUS_Graphicsv3dHandle  hGfx,                      /* [in]  */
   int                     *piFence                    /* [out] */
   );

/**
Summary:
Create another reference to a fence object.
**/
NEXUS_Error NEXUS_Graphicsv3d_FenceKeep(
   NEXUS_Graphicsv3dHandle  hGfx,                      /* [in] */
   int                      iFence                     /* [in] */
   );

/**
Summary:
Set up a callback for fence wait.

Description:
The callback will be triggered when the fence is signaled, or immediately
if the fence is already in a signaled state.
**/
NEXUS_Error NEXUS_Graphicsv3d_RegisterFenceWait(
   NEXUS_Graphicsv3dHandle             hGfx,           /* [in]  */
   int                                 iFence,         /* [in]  */
   uint64_t                            uiEvent         /* [in]  */
   );

/**
Summary:
Remove a previously registered fence wait callback.
**/
NEXUS_Error NEXUS_Graphicsv3d_UnregisterFenceWait(
   NEXUS_Graphicsv3dHandle             hGfx,           /* [in]  */
   int                                 iFence,         /* [in]  */
   uint64_t                            uiEvent,        /* [in]  */
   bool                               *signalled       /* [out] attr{null_allowed=y} */
   );

/**
Summary:
Signal a fence immediately.

Description:
This does not queue a job. The fence is signaled right away.
**/
NEXUS_Error NEXUS_Graphicsv3d_FenceSignal(
   NEXUS_Graphicsv3dHandle  hGfx,                      /* [in] */
   int                      iFence                     /* [in] */
   );

NEXUS_Error NEXUS_Graphicsv3d_FenceClose(
   NEXUS_Graphicsv3dHandle  hGfx,                      /* [in] */
   int                      fence                      /* [in] */
   );

NEXUS_Error NEXUS_Graphicsv3d_GetPendingFenceEvent(
   NEXUS_Graphicsv3dHandle              hGfx,          /* [in] */
   uint64_t                            *puiEvent       /* [out] */
   );

/************************************************************************/
/* Usermode handling                                                    */
/************************************************************************/

/**
Summary:
Get next usermode record (if any)
Also acknowledges that previous jobId has finished its usermode job
**/
NEXUS_Error NEXUS_Graphicsv3d_GetUsermode(
   NEXUS_Graphicsv3dHandle       hGfx,                 /* [in]  */
   uint64_t                      uiPrevJobId,          /* [in]  */
   NEXUS_Graphicsv3dUsermode    *psUsermode            /* [out] */
   );

/************************************************************************/
/* Completion handlers                                                  */
/************************************************************************/

/**
Summary:
Get next completion records (if any)
Also acknowledges that previous jobId has finished its completion
**/
/*
   NOTE:
   reserved hint is from MAX_COMPLETIONS in sched_nexus.c
*/
NEXUS_Error NEXUS_Graphicsv3d_GetCompletions(
   NEXUS_Graphicsv3dHandle          hGfx,                 /* [in]  */
   uint32_t                         uiNumFinalizedJobs,   /* [in]  */
   const uint64_t                   *puiFinalizedJobs,    /* [in] attr{nelem=uiNumFinalizedJobs;null_allowed=y;reserved=40} */
   uint32_t                         uiMaxCompletionsOut,  /* [in]  */
   NEXUS_Graphicsv3dCompletionInfo  *psCompletionInfo,    /* [out] */
   uint32_t                         *puiCompletionsOut,   /* [out] */
   NEXUS_Graphicsv3dCompletion      *psCompletions        /* [out] attr{nelem=uiMaxCompletionsOut;nelem_out=puiCompletionsOut;null_allowed=y;reserved=40} */
   );

/************************************************************************/
/* Information                                                          */
/************************************************************************/

/**
Summary:
Get information about the vc5 cores in the system.
**/
void NEXUS_Graphicsv3d_GetInfo(
   NEXUS_Graphicsv3dInfo   *pInfo                      /* [out] */
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
