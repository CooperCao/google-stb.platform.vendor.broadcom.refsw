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
#ifndef BVC5_INTERNAL_JOB_H__
#define BVC5_INTERNAL_JOB_H__

#include "bvc5.h"
#include "bvc5_bin_mem_priv.h"
#include "blst_queue.h"
#include "bvc5_registers_priv.h"

struct BVC5_P_JobDependentFence;

#define BVC5_P_BIN_JOB_COMPLETED   ((uint64_t)0)
#define BVC5_P_BIN_JOB_FINALIZED (~((uint64_t)0))

typedef struct BVC5_P_SharedFenceInfo
{
   bool           bSignalled;
   uint32_t       uNumberRequiredSignaled;        /* Number of times the fence need to be signalled before deleting the struct */
   void           *pFenceSignalData;
   int            iFence;
   uint32_t       uFenceUid;                      /* Unique id for the fence used for debug and GPUMonitor events */
} BVC5_P_SharedFenceInfo;

typedef struct BVC5_P_JobDependentFence
{
   struct BVC5_P_JobDependentFence  *psNext;             /* Next pointer for psOnCompleted/FinalizedFenceList in job */

   /* Note that these will *not* include the job this fence is currently listed
    * on -- that is implicit */
   BVC5_SchedDependencies           sNotCompleted;       /* Completion dependencies not yet done */
   BVC5_SchedDependencies           sNotFinalized;       /* Finalize dependencies not yet finalized */

   struct BVC5_P_SharedFenceInfo    *psSharedFenceInfo;  /* Fence info shared between jobs that have a common fence */
} BVC5_P_JobDependentFence;

typedef struct BVC5_P_InternalJob
{
   BVC5_JobBase            *pBase;                    /* The original job data                        */
   uint64_t                 uiJobId;
   uint32_t                 uiClientId;
   uint32_t                 uiNeedsCacheFlush;        /* One bit per core */
   bool                     bAbandon;

   /* These deps must be satisfied for a job to run */
   BVC5_SchedDependencies   sRunDep_NotCompleted;     /* Completion dependencies not yet done         */
   BVC5_SchedDependencies   sRunDep_NotFinalized;     /* Finalize dependencies not yet finalized      */

   /* These deps must be satisfied for a job to finalize */
   BVC5_SchedDependencies   sFinDep_NotFinalized;     /* Completion dependencies not yet finalized    */
                                                      /* Finalized dependencies already finalized!    */

   BVC5_P_JobDependentFence *psOnCompletedFenceList;
   BVC5_P_JobDependentFence *psOnFinalizedFenceList;

   BVC5_JobStatus           eStatus;                  /* Returned in completion handler               */

#if !V3D_VER_AT_LEAST(3,3,0,0)
   uint64_t                 uiRenderStart;            /* handled via queue registers, so no state needed >= 3.3 */
#endif

   union BVC5_P_InternalJobData
   {
      struct BVC5_P_InternalRender
      {
         BVC5_P_BinMemArray    sBinMemArray;         /* Memory allocated for associated bin           */
         uint64_t              uiBinJobId;           /* Implicit dependency                           */
         bool                  bRenderOnlyJob;       /* No corresponding bin job - render only        */
      } sRender;

      struct BVC5_P_InternalBin
      {
         struct BVC5_P_InternalJob  *psInternalRenderJob;
         uint32_t                   uiMinInitialBinBlockSize;
         uint32_t                   uiTileStateSize;
      } sBin;

#if V3D_VER_AT_LEAST(4,1,34,0)
      struct BVC5_P_InternalCompute
      {
         struct BVC5_P_ComputeSubjobs *pSubjobs;
         uint32_t uiNumIssued;
         uint32_t uiNumDone;
      } sCompute;
#endif

      struct BVC5_P_InternalWait
      {
         bool signaled;
         void *waitData;
      } sWait;

      struct BVC5_P_InternalEvent
      {
         uint64_t eventId;
      } sEvent;
   } jobData;

   /* Job list link data
         -- each list (stored per client) represents a state of the job e.g. running, completed etc.
         -- jobs can only be in one list at a time by design
         -- allocation of a job brings the list link along with it which simplifies oom handling
    */
   BLST_Q_ENTRY(BVC5_P_InternalJob) sJobqChain;

   /* Job active queue
         -- jobs that have not completed are in this queue
    */
   BLST_Q_ENTRY(BVC5_P_InternalJob) sActiveqChain;

} BVC5_P_InternalJob;

#if V3D_VER_AT_LEAST(4,1,34,0)
typedef struct BVC5_P_ComputeSubjobs
{
   uint32_t uiSize;
   uint32_t uiCapacity;
   BVC5_JobComputeSubjob pData[1];
} BVC5_P_ComputeSubjobs;
#endif

/* CONSTRUCTORS AND DESTRUCTOR */

BVC5_P_InternalJob *BVC5_P_JobCreateNull(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobNull      *psJob
);

BVC5_P_InternalJob *BVC5_P_JobCreateBarrier(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobBarrier   *psJob
);

BVC5_P_InternalJob *BVC5_P_JobCreateBin(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobBin       *psJob,
   BVC5_P_InternalJob      *pRenderJob
);

BVC5_P_InternalJob *BVC5_P_JobCreateRender(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobRender    *psJob
);

#if V3D_VER_AT_LEAST(4,1,34,0)
BVC5_P_InternalJob *BVC5_P_JobCreateCompute(
   BVC5_Handle             hVC5,
   uint32_t                uiClientId,
   const BVC5_JobCompute   *pComputeJob,
   BVC5_P_ComputeSubjobs   *pSubjobs);
#endif

BVC5_P_InternalJob *BVC5_P_JobCreateFenceWait(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobFenceWait *psJob
);

BVC5_P_InternalJob *BVC5_P_JobCreateTFU(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobTFU       *psJob
);

BVC5_P_InternalJob *BVC5_P_JobCreateTest(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobTest      *psJob
);

BVC5_P_InternalJob *BVC5_P_JobCreateUsermode(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobUsermode  *psJob
);

BVC5_P_InternalJob *BVC5_P_JobCreateSchedEvent(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobSchedJob  *psJob
);

void BVC5_P_JobDestroy(
   BVC5_Handle           hVC5,
   BVC5_P_InternalJob   *psJob
);

#if V3D_VER_AT_LEAST(4,1,34,0)
BVC5_P_ComputeSubjobs *BVC5_P_NewComputeSubjobs(uint32_t uiCapacity);
void BVC5_P_UpdateComputeSubjobs(BVC5_P_ComputeSubjobs *pSubjobs, uint32_t uiSize, const BVC5_JobComputeSubjob *pNewData);
void BVC5_P_DeleteComputeSubjobs(BVC5_P_ComputeSubjobs *pSubjobs);
#endif

#endif /* BVC5_INTERNAL_JOB_H__ */
