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
#include "bstd.h"
#include "bkni.h"
#include "bvc5_priv.h"
#include "bvc5_jobq_priv.h"
#include "bvc5_client_priv.h"
#include "bvc5_hardware_priv.h"

/***************************************************************************/

/* BVC5_P_ClientMap
 *
 * The client map uses the client id as the key.
 * Each entry is a BVC5_P_Client
 */
typedef struct BVC5_P_ClientMap
{
   uint32_t                          uiSize;
   BLST_S_HEAD(sList, BVC5_P_Client) sList;
} BVC5_P_ClientMap;

/***************************************************************************/

BERR_Code BVC5_P_ClientMapCreate(
   BVC5_Handle           hVC5,
   BVC5_ClientMapHandle *phClientMap
)
{
   BVC5_ClientMapHandle hClientMap;

   BSTD_UNUSED(hVC5);

   if (phClientMap == NULL)
      return BERR_INVALID_PARAMETER;

   hClientMap = (BVC5_ClientMapHandle)BKNI_Malloc(sizeof(BVC5_P_ClientMap));
   if (hClientMap == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   BLST_S_INIT(&hClientMap->sList);

   hClientMap->uiSize = 0;

   *phClientMap = hClientMap;

   return BERR_SUCCESS;
}

/***************************************************************************/

BERR_Code BVC5_P_ClientMapDestroy(
   BVC5_Handle          hVC5,
   BVC5_ClientMapHandle hClientMap
)
{
   BSTD_UNUSED(hVC5);

   if (hClientMap == NULL)
      return BERR_INVALID_PARAMETER;

   BDBG_ASSERT(BLST_S_EMPTY(&hClientMap->sList));

   BKNI_Free(hClientMap);

   return BERR_SUCCESS;
}

/***************************************************************************/

static BERR_Code BVC5_P_ClientCreate(
   BVC5_Handle        hVC5,
   BVC5_ClientHandle *phClient,
   uint32_t           uiClientId,
   uint64_t           uiPlatformToken,
   uint32_t           uiClientPID
)
{
   BERR_Code   err = BERR_OUT_OF_SYSTEM_MEMORY;

   BVC5_ClientHandle hClient = (BVC5_P_Client *)BKNI_Malloc(sizeof(BVC5_P_Client));

   *phClient = hClient;

   if (hClient == NULL)
      goto exit;

   BKNI_Memset(hClient, 0, sizeof(BVC5_P_Client));

   hClient->uiClientId = uiClientId;
   hClient->uiPlatformToken = uiPlatformToken;
   hClient->uiMaxJobId = 0;
   hClient->hVC5 = hVC5;
   hClient->uiClientPID = uiClientPID;

   BVC5_P_JobQCreate(&hClient->hWaitQ);
   if (hClient->hWaitQ == NULL)
      goto exit;

   BVC5_P_JobQCreate(&hClient->hRunnableSoftQ);
   if (hClient->hRunnableSoftQ == NULL)
      goto exit;

   BVC5_P_JobQCreate(&hClient->hRunnableBinnerQ);
   if (hClient->hRunnableBinnerQ == NULL)
      goto exit;

   BVC5_P_JobQCreate(&hClient->hRunnableRenderQ);
   if (hClient->hRunnableRenderQ == NULL)
      goto exit;

   BVC5_P_JobQCreate(&hClient->hRunnableTFUQ);
   if (hClient->hRunnableTFUQ == NULL)
      goto exit;

   BVC5_P_JobQCreate(&hClient->hRunnableUsermodeQ);
   if (hClient->hRunnableUsermodeQ == NULL)
      goto exit;

   BVC5_P_JobQCreate(&hClient->hRunnableBarrierQ);
   if (hClient->hRunnableBarrierQ == NULL)
      goto exit;

   BVC5_P_JobQCreate(&hClient->hCompletedQ);
   if (hClient->hCompletedQ == NULL)
      goto exit;

   BVC5_P_JobQCreate(&hClient->hFinalizableQ);
   if (hClient->hFinalizableQ == NULL)
      goto exit;

   BVC5_P_JobQCreate(&hClient->hFinalizingQ);
   if (hClient->hFinalizingQ == NULL)
      goto exit;

   BVC5_P_ActiveQCreate(&hClient->hActiveJobs);
   if (hClient->hActiveJobs == NULL)
      goto exit;

   err = BVC5_P_EventArrayCreate(&hClient->hEvents);
   if (err != BERR_SUCCESS)
      goto exit;

   err = BERR_SUCCESS;

exit:
   if (err != BERR_SUCCESS)
   {
      if (hClient != NULL)
         BVC5_P_ClientDestroy(hVC5, hClient);
   }

   return err;
}

/***************************************************************************/

BERR_Code BVC5_P_ClientDestroy(
   BVC5_Handle       hVC5,
   BVC5_ClientHandle hClient
)
{
   BERR_Code            err  = BERR_SUCCESS;

   if (hClient == NULL)
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   if (hClient->hWaitQ != NULL)
      BVC5_P_JobQDestroy(hVC5, hClient->hWaitQ);

   if (hClient->hRunnableSoftQ != NULL)
      BVC5_P_JobQDestroy(hVC5, hClient->hRunnableSoftQ);

   if (hClient->hRunnableBinnerQ != NULL)
      BVC5_P_JobQDestroy(hVC5, hClient->hRunnableBinnerQ);

   if (hClient->hRunnableRenderQ != NULL)
      BVC5_P_JobQDestroy(hVC5, hClient->hRunnableRenderQ);

   if (hClient->hRunnableTFUQ != NULL)
      BVC5_P_JobQDestroy(hVC5, hClient->hRunnableTFUQ);

   if (hClient->hRunnableUsermodeQ != NULL)
      BVC5_P_JobQDestroy(hVC5, hClient->hRunnableUsermodeQ);

   if (hClient->hRunnableBarrierQ != NULL)
      BVC5_P_JobQDestroy(hVC5, hClient->hRunnableBarrierQ);

   if (hClient->hCompletedQ != NULL)
      BVC5_P_JobQDestroy(hVC5, hClient->hCompletedQ);

   if (hClient->hFinalizableQ != NULL)
      BVC5_P_JobQDestroy(hVC5, hClient->hFinalizableQ);

   if (hClient->hFinalizingQ != NULL)
      BVC5_P_JobQDestroy(hVC5, hClient->hFinalizingQ);

   if (hClient->hActiveJobs != NULL)
      BVC5_P_ActiveQDestroy(hClient->hActiveJobs);

   if (hClient->uiPlatformToken != 0)
      BVC5_P_DRMTerminateClient(hClient->uiPlatformToken);

   /* Remove all the events */
   BVC5_P_DeleteAllSchedEvent(hClient->hEvents);
   /* Delete the event array */
   BVC5_P_EventArrayDestroy(hClient->hEvents);

   BKNI_Free(hClient);

exit:
   return err;
}

/***************************************************************************/

BVC5_ClientHandle BVC5_P_ClientMapGet(
   BVC5_Handle          hVC5,
   BVC5_ClientMapHandle hClientMap,
   uint32_t             uiClientId
)
{
   BVC5_P_Client *psClient;

   BSTD_UNUSED(hVC5);

   if (hClientMap == NULL)
      return NULL;

   BDBG_ASSERT(!BLST_S_EMPTY(&hClientMap->sList));

   BLST_S_DICT_FIND(&hClientMap->sList, psClient, uiClientId, uiClientId, sChain);

   BDBG_ASSERT(psClient != NULL);

   return psClient;
}

/***************************************************************************/

BERR_Code BVC5_P_ClientMapCreateAndInsert(
   BVC5_Handle             hVC5,
   BVC5_ClientMapHandle    hClientMap,
   void                   *pContext,
   uint32_t                uiClientId,
   uint64_t                uiPlatformToken,
   uint32_t                uiClientPID
)
{
   BVC5_ClientHandle hClient = NULL;
   BERR_Code         err     = BERR_SUCCESS;

   if (hClientMap == NULL)
      return BERR_INVALID_PARAMETER;

   err = BVC5_P_ClientCreate(hVC5, &hClient, uiClientId, uiPlatformToken, uiClientPID);
   if (err != BERR_SUCCESS)
      goto exit;

   /* This is a context from the surrounding API (e.g. Nexus) */
   hClient->pContext = pContext;

   hClientMap->uiSize += 1;

   BLST_S_DICT_ADD(&hClientMap->sList, hClient, BVC5_P_Client, uiClientId, sChain, duplicate);

   return BERR_SUCCESS;

exit:
   if (hClient != NULL)
      BVC5_P_ClientDestroy(hVC5, hClient);

   return err;

duplicate:
   BKNI_Printf("%s : FATAL : Duplicate client id inserted to map\n", BSTD_FUNCTION);
   BVC5_P_ClientDestroy(hVC5, hClient);
   return BERR_INVALID_PARAMETER;
}

/***************************************************************************/
BERR_Code BVC5_P_ClientMapRemoveAndDestroy(
   BVC5_Handle          hVC5,
   BVC5_ClientMapHandle hClientMap,
   uint32_t             uiClientId
)
{
   BVC5_ClientHandle hClient = NULL;
   BERR_Code         err = BERR_SUCCESS;

   if (hClientMap == NULL)
      return BERR_INVALID_PARAMETER;

   hClient = BVC5_P_ClientMapGet(hVC5, hClientMap, uiClientId);

   if (hClient == NULL)
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   hClientMap->uiSize -= 1;

   BLST_S_DICT_REMOVE(&hClientMap->sList, hClient, uiClientId, BVC5_P_Client, uiClientId, sChain);

   if (hClient != NULL)
      BVC5_P_ClientDestroy(hVC5, hClient);

exit:
   return err;
}

/***************************************************************************/

BVC5_ClientHandle BVC5_P_ClientMapFirst(
   BVC5_ClientMapHandle   hClientMap,
   void                 **ppNext
)
{
   BVC5_P_Client *psClient;

   if ((hClientMap == NULL) || (ppNext == NULL))
      return NULL;

   psClient = BLST_S_FIRST(&hClientMap->sList);

   *ppNext = (void *)psClient;

   return psClient;
}

/***************************************************************************/

BVC5_ClientHandle BVC5_P_ClientMapNext(
   BVC5_ClientMapHandle   hClientMap,
   void                 **ppNext
)
{
   BVC5_P_Client *psClient;

   if ((hClientMap == NULL) || (ppNext == NULL))
      return NULL;

   psClient = *(BVC5_P_Client **)ppNext;
   if (psClient == NULL)
      return NULL;

   psClient = BLST_S_NEXT(psClient, sChain);

   *ppNext = (void *)psClient;

   return psClient;
}

/***************************************************************************/

uint32_t BVC5_P_ClientMapSize(
   BVC5_ClientMapHandle hClientMap
)
{
   return hClientMap->uiSize;
}

/***************************************************************************/
BERR_Code BVC5_P_ClientMapGetStats(
   BVC5_ClientMapHandle    hClientMap,
   BVC5_ClientLoadData    *pLoadData,
   uint32_t                uiNumClients,
   uint32_t               *pValidClients,
   bool                    bReset
)
{
   BVC5_P_Client *psClient;
   uint64_t       uiTimeNow;
   uint32_t       i = 0;

   if (pLoadData == NULL || pValidClients == NULL)
      return BERR_INVALID_PARAMETER;

   BVC5_P_GetTime_isrsafe(&uiTimeNow);

   for (psClient = BLST_S_FIRST(&hClientMap->sList); psClient != NULL; psClient = BLST_S_NEXT(psClient, sChain))
   {
      uint64_t uiElapsedUs;

      if (i >= uiNumClients)
         break;

      uiElapsedUs = uiTimeNow - psClient->sLoadStats.uiLastCollectedTime;

      pLoadData[i].uiClientId = psClient->uiClientId;
      pLoadData[i].uiClientPID = psClient->uiClientPID;
      pLoadData[i].uiNumRenders = psClient->sLoadStats.uiRenderCount;
      pLoadData[i].sRenderPercent = (uint8_t)((psClient->sLoadStats.iRenderTimeUs) * 100 / uiElapsedUs);

      if (bReset)
      {
         psClient->sLoadStats.uiLastCollectedTime = uiTimeNow;
         psClient->sLoadStats.iRenderTimeUs = 0;
         psClient->sLoadStats.uiRenderCount = 0;
      }

      i++;
   }

   *pValidClients = i;

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BVC5_P_ClientMapUpdateStats_isrsafe(
   BVC5_ClientMapHandle   hClientMap,
   uint32_t               uiClientId,
   uint64_t               uiTimeInRendererUs
   )
{
   BVC5_P_Client *psClient;

   if (hClientMap == NULL)
      return BERR_INVALID_PARAMETER;

   BDBG_ASSERT(!BLST_S_EMPTY(&hClientMap->sList));

   BLST_S_DICT_FIND(&hClientMap->sList, psClient, uiClientId, uiClientId, sChain);
   if (psClient == NULL)
      return BERR_INVALID_PARAMETER;

   psClient->sLoadStats.uiRenderCount += 1;
   psClient->sLoadStats.iRenderTimeUs += uiTimeInRendererUs;

   return BERR_SUCCESS;
}

/* JOB STATE TRANSITIONS */

void BVC5_P_ClientJobToWaiting(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
)
{
   BSTD_UNUSED(hVC5);

   if (hClient->psOldestNotFinalized == NULL)
      hClient->psOldestNotFinalized = psJob;

   BVC5_P_ActiveQInsert(hClient->hActiveJobs, psJob);
   BVC5_P_JobQInsert(hClient->hWaitQ, psJob);
}

static BVC5_JobQHandle BVC5_P_RunQ(
   BVC5_ClientHandle hClient,
   BVC5_JobType      type
)
{
   switch (type)
   {
   case BVC5_JobType_eBin      : return hClient->hRunnableBinnerQ;
   case BVC5_JobType_eRender   : return hClient->hRunnableRenderQ;
   case BVC5_JobType_eTFU      : return hClient->hRunnableTFUQ;
   case BVC5_JobType_eUsermode : return hClient->hRunnableUsermodeQ;
   case BVC5_JobType_eBarrier  : return hClient->hRunnableBarrierQ;
   default                     : return hClient->hRunnableSoftQ;
   }
}

void BVC5_P_ClientJobWaitingToRunnable(
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
)
{
   BVC5_JobQHandle   hRunQ = BVC5_P_RunQ(hClient, psJob->pBase->eType);

   BDBG_ASSERT(hRunQ != NULL);

   switch (psJob->pBase->eType)
   {
   case BVC5_JobType_eBin:
   case BVC5_JobType_eRender:
      psJob->uiNeedsCacheFlush = BVC5_P_HardwareDeferCacheFlush(
         hClient->hVC5,
         psJob->pBase->uiCacheOps & BVC5_CACHE_FLUSH_ALL,
         BVC5_CACHE_FLUSH_ALL);
      break;
   default:
      /* Shouldn't get cache op requests for other job types. */
      BDBG_ASSERT(psJob->pBase->uiCacheOps == 0);
      break;
   }

   /* Add to client runnable list and remove from waitq */
   BVC5_P_JobQRemove(hClient->hWaitQ, psJob);
   BVC5_P_JobQInsert(hRunQ, psJob);
}

static void BVC5_P_SignalAndFreeOrAddToJob(
   BVC5_Handle                hVC5,
   BVC5_ClientHandle          hClient,
   BVC5_P_JobDependentFence  *psFence
);

void BVC5_P_ClientJobRunningToCompleted(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
)
{
   BVC5_P_JobQInsert(hClient->hCompletedQ, psJob);
   BVC5_P_ActiveQRemove(hClient->hActiveJobs, psJob);

   while (psJob->psOnCompletedFenceList)
   {
      BVC5_P_JobDependentFence *psFence = psJob->psOnCompletedFenceList;
      psJob->psOnCompletedFenceList = psFence->psNext;
      BVC5_P_SignalAndFreeOrAddToJob(hVC5, hClient, psFence);
   }
}

void BVC5_P_ClientJobCompletedToFinalizable(
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
)
{
   BVC5_P_JobQRemove(hClient->hCompletedQ, psJob);
   BVC5_P_JobQInsert(hClient->hFinalizableQ, psJob);
}

void BVC5_P_ClientJobFinalizableToFinalizing(
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
)
{
   BVC5_P_JobQRemove(hClient->hFinalizableQ, psJob);
   BVC5_P_JobQInsert(hClient->hFinalizingQ, psJob);
}

static BVC5_P_InternalJob *BVC5_P_Oldest(
   BVC5_P_InternalJob   *psJobA,
   BVC5_P_InternalJob   *psJobB
)
{
   if (psJobA == NULL)
      return psJobB;
   if (psJobB == NULL)
      return psJobA;
   return (psJobA->uiJobId < psJobB->uiJobId) ? psJobA : psJobB;
}

static void BVC5_P_UpdateOldestNotFinalized(
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psFinalizedJob
)
{
   /* If its not the oldest, we can keep the current oldest, otherwise recalculate it */
   if (hClient->psOldestNotFinalized == psFinalizedJob)
   {
      hClient->psOldestNotFinalized = BVC5_P_ActiveQFirst(hClient->hActiveJobs);
      hClient->psOldestNotFinalized = BVC5_P_Oldest(hClient->psOldestNotFinalized,
         BVC5_P_JobQFirst(hClient->hCompletedQ));
      hClient->psOldestNotFinalized = BVC5_P_Oldest(hClient->psOldestNotFinalized,
         BVC5_P_JobQFirst(hClient->hFinalizableQ));
      hClient->psOldestNotFinalized = BVC5_P_Oldest(hClient->psOldestNotFinalized,
         BVC5_P_JobQFirst(hClient->hFinalizingQ));
   }
}

static void BVC5_P_ClientJobFinalized(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
)
{
   BVC5_P_UpdateOldestNotFinalized(hClient, psJob);

   while (psJob->psOnFinalizedFenceList)
   {
      BVC5_P_JobDependentFence *psFence = psJob->psOnFinalizedFenceList;
      psJob->psOnFinalizedFenceList = psFence->psNext;
      BVC5_P_SignalAndFreeOrAddToJob(hVC5, hClient, psFence);
   }

   /* Destroy it -- it's finished */
   BVC5_P_JobDestroy(hVC5, psJob);
}

static void BVC5_P_FinalizeImplicitDependency(
   BVC5_P_InternalJob  *psJob
)
{
   /* Special case when bin jobs are finalized due to the implicit dependency between
    * render and bin. We need to tell the render job that the bin job is finalized. */
   if (psJob->pBase->eType == BVC5_JobType_eBin)
   {
      BVC5_P_InternalJob *psIntRdrJob = psJob->jobData.sBin.psInternalRenderJob;
      if (psIntRdrJob != NULL)
         psIntRdrJob->jobData.sRender.uiBinJobId = BVC5_P_BIN_JOB_FINALIZED;
   }
}

/* Move a job to finalized state - should only be used if the job has no callback fn */
void BVC5_P_ClientJobCompletedToFinalized(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
   )
{
   BVC5_P_JobQRemove(hClient->hCompletedQ, psJob);
   BVC5_P_FinalizeImplicitDependency(psJob);
   BVC5_P_ClientJobFinalized(hVC5, hClient, psJob);
}

void BVC5_P_ClientJobFinalizingToFinalized(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   uint64_t             uiJobId
)
{
   BVC5_P_InternalJob *psJob = BVC5_P_JobQRemoveById(hClient->hFinalizingQ, uiJobId);
   BVC5_P_FinalizeImplicitDependency(psJob);
   BVC5_P_ClientJobFinalized(hVC5, hClient, psJob);
}

static BVC5_P_InternalJob *BVC5_P_ClientGetJobIfNotComplete(
   BVC5_ClientHandle hClient,
   uint64_t          uiJobId
)
{
   /* If this job id is lower than the oldest not finalized then it must have
    * left the system */
   if ((hClient->psOldestNotFinalized == NULL) || (uiJobId < hClient->psOldestNotFinalized->uiJobId))
      return NULL;

   /* Otherwise we can check if the job is still not completed */
   return BVC5_P_ActiveQFindById(hClient->hActiveJobs, uiJobId);
}

bool BVC5_P_ClientIsJobComplete(
   BVC5_ClientHandle hClient,
   uint64_t          uiJobId
)
{
   return BVC5_P_ClientGetJobIfNotComplete(hClient, uiJobId) == NULL;
}

static BVC5_P_InternalJob *BVC5_P_ClientGetJobIfFinishing(
   BVC5_ClientHandle hClient,
   uint64_t          uiJobId
)
{
   BVC5_P_InternalJob *psJob = BVC5_P_JobQFindById(hClient->hCompletedQ, uiJobId);
   if (psJob)
      return psJob;

   psJob = BVC5_P_JobQFindById(hClient->hFinalizableQ, uiJobId);
   if (psJob)
      return psJob;

   return BVC5_P_JobQFindById(hClient->hFinalizingQ, uiJobId);
}

bool BVC5_P_ClientIsJobFinishing(
   BVC5_ClientHandle hClient,
   uint64_t          uiJobId
)
{
   return BVC5_P_ClientGetJobIfFinishing(hClient, uiJobId) != NULL;
}

static BVC5_P_InternalJob *BVC5_P_ClientGetJobIfNotFinalized(
   BVC5_ClientHandle hClient,
   uint64_t          uiJobId
)
{
   BVC5_P_InternalJob *psJob = BVC5_P_ClientGetJobIfNotComplete(hClient, uiJobId);
   if (psJob)
      return psJob;

   return BVC5_P_ClientGetJobIfFinishing(hClient, uiJobId);
}

bool BVC5_P_ClientIsJobFinalized(
   BVC5_ClientHandle hClient,
   uint64_t          uiJobId
)
{
   return BVC5_P_ClientGetJobIfNotFinalized(hClient, uiJobId) == NULL;
}

bool BVC5_P_ClientSetMaxJobId(
   BVC5_ClientHandle hClient,
   uint64_t uiMaxJobId
)
{
   if (uiMaxJobId <= hClient->uiMaxJobId)
      return false;
   hClient->uiMaxJobId = uiMaxJobId;
   return true;
}

uint64_t BVC5_P_ClientGetOldestNotFinalizedId(
   BVC5_ClientHandle hClient
)
{
   return hClient->psOldestNotFinalized ? hClient->psOldestNotFinalized->uiJobId :
      hClient->uiMaxJobId + 1;
}

static BVC5_P_InternalJob *BVC5_P_PopUntilNotCompletedFinalizedJob(
   BVC5_ClientHandle       hClient,
   BVC5_SchedDependencies *psDeps,
   bool                    bCompleted /* else finalised */
)
{
   BDBG_ASSERT(psDeps->uiNumDeps <= (sizeof(psDeps->uiDep) / sizeof(*psDeps->uiDep)));
   while (psDeps->uiNumDeps)
   {
      uint64_t uiJobId = psDeps->uiDep[--psDeps->uiNumDeps];
      BVC5_P_InternalJob *psJob = bCompleted ?
         BVC5_P_ClientGetJobIfNotComplete(hClient, uiJobId) :
         BVC5_P_ClientGetJobIfNotFinalized(hClient, uiJobId);
      if (psJob)
         return psJob;
   }
   return NULL;
}

static BVC5_P_InternalJob *BVC5_P_PopUntilNotCompletedJob(
   BVC5_ClientHandle       hClient,
   BVC5_SchedDependencies *psDeps
)
{
   return BVC5_P_PopUntilNotCompletedFinalizedJob(hClient, psDeps, true);
}

static BVC5_P_InternalJob *BVC5_P_PopUntilNotFinalizedJob(
   BVC5_ClientHandle       hClient,
   BVC5_SchedDependencies *psDeps
)
{
   return BVC5_P_PopUntilNotCompletedFinalizedJob(hClient, psDeps, false);
}

static BVC5_P_JobDependentFence *BVC5_P_AllocJobDependentFence(
   BVC5_Handle                   hVC5,
   BVC5_ClientHandle             hClient,
   const BVC5_SchedDependencies  *psNotCompleted,
   const BVC5_SchedDependencies  *psNotFinalized,
   const BVC5_P_SharedFenceInfo  *psSharedFenceInfo
)
{
   BVC5_P_JobDependentFence *psFence = BKNI_Malloc(sizeof(*psFence));
   if (psFence == NULL)
      return NULL;

   if (psSharedFenceInfo == NULL)
   {
      /* Create a new fence */
      psFence->psSharedFenceInfo = BKNI_Malloc(sizeof(BVC5_P_SharedFenceInfo));
      if (psFence->psSharedFenceInfo == NULL)
      {
         BKNI_Free(psFence);
         return NULL;
      }

      BKNI_Memset(psFence->psSharedFenceInfo, 0, sizeof(BVC5_P_SharedFenceInfo));

      psFence->psSharedFenceInfo->iFence = BVC5_P_FenceCreate(hVC5->hFences, hClient->uiClientId, &psFence->psSharedFenceInfo->pFenceSignalData);
      if (psFence->psSharedFenceInfo->iFence == -1)
      {
         BKNI_Free(psFence->psSharedFenceInfo);
         BKNI_Free(psFence);
         return NULL;
      }

      psFence->psSharedFenceInfo->bSignalled = false;
   }
   else
   {
      /* Use an existing fence */
      psFence->psSharedFenceInfo->bSignalled = psSharedFenceInfo->bSignalled;
      psFence->psSharedFenceInfo->pFenceSignalData = psSharedFenceInfo->pFenceSignalData;
      psFence->psSharedFenceInfo->iFence = psSharedFenceInfo->iFence;
   }

   psFence->psSharedFenceInfo->uNumberRequiredSignaled++;
   psFence->sNotCompleted = *psNotCompleted;
   psFence->sNotFinalized = *psNotFinalized;

   return psFence;
}

static void BVC5_P_SignalAndFreeOrAddToJob(
   BVC5_Handle                hVC5,
   BVC5_ClientHandle          hClient,
   BVC5_P_JobDependentFence  *psFence
)
{
   BVC5_P_InternalJob *psJob = BVC5_P_PopUntilNotCompletedJob(hClient, &psFence->sNotCompleted);
   if (psJob)
   {
      psFence->psNext = psJob->psOnCompletedFenceList;
      psJob->psOnCompletedFenceList = psFence;
      return;
   }

   psJob = BVC5_P_PopUntilNotFinalizedJob(hClient, &psFence->sNotFinalized);
   if (psJob)
   {
      psFence->psNext = psJob->psOnFinalizedFenceList;
      psJob->psOnFinalizedFenceList = psFence;
      return;
   }

   /* All jobs have completed/finalised: signal the fence */

   BDBG_ASSERT(psFence->psSharedFenceInfo != NULL);
   if (!psFence->psSharedFenceInfo->bSignalled)
   {
      BVC5_P_FenceSignalAndCleanup(hVC5->hFences, psFence->psSharedFenceInfo->pFenceSignalData);
      psFence->psSharedFenceInfo->pFenceSignalData = NULL;
      psFence->psSharedFenceInfo->bSignalled = true;
   }
   psFence->psSharedFenceInfo->uNumberRequiredSignaled--;
   if (psFence->psSharedFenceInfo->uNumberRequiredSignaled == 0)
   {
      BKNI_Free(psFence->psSharedFenceInfo);
      psFence->psSharedFenceInfo = NULL;
   }

   BKNI_Free(psFence);
}

BERR_Code BVC5_P_ClientMakeFenceForJobs(
   BVC5_Handle                   hVC5,
   BVC5_ClientHandle             hClient,
   const BVC5_SchedDependencies *pCompletedDeps,
   const BVC5_SchedDependencies *pFinalizedDeps,
   bool                          bForceCreate,
   int                          *piFence
)
{
   BVC5_SchedDependencies  sNotCompleted = *pCompletedDeps,
                           sNotFinalized = *pFinalizedDeps;
   BVC5_P_InternalJob      *psJob = BVC5_P_PopUntilNotCompletedJob(hClient, &sNotCompleted);

   *piFence = -1;

   if (psJob)
   {
      BVC5_P_JobDependentFence *psFence = BVC5_P_AllocJobDependentFence(
         hVC5, hClient, &sNotCompleted, &sNotFinalized, NULL);
      if (psFence == NULL)
         return BERR_OUT_OF_SYSTEM_MEMORY;
      *piFence = psFence->psSharedFenceInfo->iFence;
      psFence->psNext = psJob->psOnCompletedFenceList;
      psJob->psOnCompletedFenceList = psFence;
      return BERR_SUCCESS;
   }

   psJob = BVC5_P_PopUntilNotFinalizedJob(hClient, &sNotFinalized);
   if (psJob)
   {
      BVC5_P_JobDependentFence *psFence = BVC5_P_AllocJobDependentFence(
         hVC5, hClient, &sNotCompleted, &sNotFinalized, NULL);
      if (psFence == NULL)
         return BERR_OUT_OF_SYSTEM_MEMORY;
      *piFence = psFence->psSharedFenceInfo->iFence;
      psFence->psNext = psJob->psOnFinalizedFenceList;
      psJob->psOnFinalizedFenceList = psFence;
      return BERR_SUCCESS;
   }

   /* All deps satisfied. Unless forced to create a fence, just return "null"
    * fence which behaves like a fence that has been signalled already. */
   if (!bForceCreate)
   {
      *piFence = -1;
      return BERR_SUCCESS;
   }

   {
      void *pFenceSignalData;
      *piFence = BVC5_P_FenceCreate(hVC5->hFences, hClient->uiClientId, &pFenceSignalData);
      if (*piFence == -1)
         return BERR_OUT_OF_SYSTEM_MEMORY;
      BVC5_P_FenceSignalAndCleanup(hVC5->hFences, pFenceSignalData);
      return BERR_SUCCESS;
   }
}

BERR_Code BVC5_P_ClientMakeFenceForAnyNonFinalizedJob(
   BVC5_Handle       hVC5,
   BVC5_ClientHandle hClient,
   int              *piFence
)
{
   BVC5_SchedDependencies sNoDeps;
   BVC5_P_JobDependentFence *psFence;

   if (hClient->psOldestNotFinalized == NULL)
   {
      /* All jobs have been finalized. Just return "null" fence which behaves
       * like a fence that has been signalled already. */
      *piFence = -1;
      return BERR_SUCCESS;
   }

   BKNI_Memset(&sNoDeps, 0, sizeof(BVC5_SchedDependencies));
   psFence = BVC5_P_AllocJobDependentFence(
      hVC5, hClient, &sNoDeps, &sNoDeps, NULL);
   if (psFence == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;
   *piFence = psFence->psSharedFenceInfo->iFence;
   psFence->psNext = hClient->psOldestNotFinalized->psOnFinalizedFenceList;
   hClient->psOldestNotFinalized->psOnFinalizedFenceList = psFence;
   return BERR_SUCCESS;
}

static bool BVC5_P_AnyDepReachedState(BVC5_ClientHandle hClient, const BVC5_SchedDependencies  *pDeps, bool bCheckFinalisedState)
{
   uint32_t index;
   for (index = 0; index < pDeps->uiNumDeps; index++)
   {
      if (!bCheckFinalisedState)
      {
         if (BVC5_P_ClientIsJobComplete(hClient, pDeps->uiDep[index]))
            return true;
      }
      else
      {
         if (BVC5_P_ClientIsJobFinalized(hClient, pDeps->uiDep[index]))
            return true;
      }
   }

   return false;
}

BERR_Code BVC5_P_ClientMakeFenceForAnyJob(
   BVC5_Handle                   hVC5,
   BVC5_ClientHandle             hClient,
   const BVC5_SchedDependencies  *pCompletedDeps,
   const BVC5_SchedDependencies  *pFinalizedDeps,
   int                           *piFence
)
{
   uint32_t index;
   bool                       bCheckFinalisedState = true;
   BVC5_SchedDependencies     sNoDeps;
   BVC5_P_JobDependentFence   *psFence;
   BVC5_P_SharedFenceInfo     *psSharedFenceInfo = NULL;

   *piFence = -1;

   if (hClient->psOldestNotFinalized == NULL)
      /* All jobs have been finalized. Just return "null" fence which behaves
       * like a fence that has been signaled already. */
      return BERR_SUCCESS;

   if (BVC5_P_AnyDepReachedState(hClient, pCompletedDeps, !bCheckFinalisedState) ||
       BVC5_P_AnyDepReachedState(hClient, pFinalizedDeps, bCheckFinalisedState))
      /* At least one of the deps has reach its state.
       * Just return "null" fence which behaves like a fence that has been signaled already. */
      return BERR_SUCCESS;

   BKNI_Memset(&sNoDeps, 0, sizeof(BVC5_SchedDependencies));

   for (index = 0; index < pCompletedDeps->uiNumDeps; index++)
   {
      /* Get not completed job
       * (they all are not completed as it has been check above with BVC5_P_AnyDepReachedState) */
      BVC5_P_InternalJob *psJob = BVC5_P_ClientGetJobIfNotComplete(hClient, pCompletedDeps->uiDep[index]);
      if (psJob)
      {
         psFence = BVC5_P_AllocJobDependentFence(
            hVC5, hClient, &sNoDeps, &sNoDeps, psSharedFenceInfo);
         if (psFence == NULL)
         {
            *piFence = -1;
            return BERR_OUT_OF_SYSTEM_MEMORY;
         }

         *piFence = psFence->psSharedFenceInfo->iFence;
         psSharedFenceInfo = psFence->psSharedFenceInfo;

         psFence->psNext = psJob->psOnCompletedFenceList;
         psJob->psOnCompletedFenceList = psFence;
      }
   }

   for (index = 0; index < pFinalizedDeps->uiNumDeps; index++)
   {
      /* Get not finalised job
       * (they all are not finalised as it has been check above with BVC5_P_AnyDepReachedState) */
      BVC5_P_InternalJob *psJob = BVC5_P_ClientGetJobIfNotFinalized(hClient, pFinalizedDeps->uiDep[index]);
      if (psJob)
      {

         psFence = BVC5_P_AllocJobDependentFence(
            hVC5, hClient, &sNoDeps, &sNoDeps, psSharedFenceInfo);
         if (psFence == NULL)
         {
            *piFence = -1;
            return BERR_OUT_OF_SYSTEM_MEMORY;
         }

         *piFence = psFence->psSharedFenceInfo->iFence;
         psSharedFenceInfo = psFence->psSharedFenceInfo;

         psFence->psNext = psJob->psOnFinalizedFenceList;
         psJob->psOnFinalizedFenceList = psFence;
      }
   }

   return BERR_SUCCESS;
}


void BVC5_P_ClientMarkJobsFlushedV3D(
   BVC5_ClientHandle hClient,
   uint32_t uiCoreIndex
)
{
   unsigned i;
   uint32_t mask = ~(1 << uiCoreIndex);

   BVC5_JobQHandle queues[2];
   queues[0] = hClient->hRunnableBinnerQ;
   queues[1] = hClient->hRunnableRenderQ;

   for (i = 0; i < 2; i++)
   {
      BVC5_P_InternalJob *pJob;
      for (pJob = BVC5_P_JobQFirst(queues[i]);
           pJob != NULL;
           pJob = BVC5_P_JobQNext(pJob))
      {
         pJob->uiNeedsCacheFlush &= mask;
      }
   }
}


/* BVC5_P_ClientSatisifed

  Has this client been given its slice of the work it wanted?

 */
bool BVC5_P_ClientSatisifed(
   BVC5_ClientHandle hClient
)
{
   return (hClient->uiWorkWanted & hClient->uiWorkGiven) == hClient->uiWorkWanted;
}

/* BVC5_P_SetWanted

  Set the work the client wants to do

 */
void BVC5_P_ClientSetWanted(
   BVC5_ClientHandle hClient
)
{
   hClient->uiWorkWanted = (BVC5_P_JobQSize(hClient->hRunnableBarrierQ) > 0 ? BVC5_CLIENT_BARRIER: 0) |
                           (BVC5_P_JobQSize(hClient->hRunnableBinnerQ)  > 0 ? BVC5_CLIENT_BIN    : 0) |
                           (BVC5_P_JobQSize(hClient->hRunnableRenderQ)  > 0 ? BVC5_CLIENT_RENDER : 0) |
                           (BVC5_P_JobQSize(hClient->hRunnableTFUQ)     > 0 ? BVC5_CLIENT_TFU    : 0);
}

/* BVC5_P_SetGiven

  Mark the work the client has been given

 */
void BVC5_P_ClientSetGiven(
   BVC5_ClientHandle hClient,
   uint32_t          uiGiven
)
{
   hClient->uiWorkGiven |= uiGiven;
}

/* BVC5_P_ClientHasHardJobs

  Mark the work the client has been given

 */
bool BVC5_P_ClientHasHardJobs(
   BVC5_ClientHandle hClient
)
{
   return BVC5_P_JobQSize(hClient->hRunnableBarrierQ) > 0 ||
          BVC5_P_JobQSize(hClient->hRunnableBinnerQ)  > 0 ||
          BVC5_P_JobQSize(hClient->hRunnableRenderQ)  > 0 ||
          BVC5_P_JobQSize(hClient->hRunnableTFUQ)     > 0;
}

/***************************************************************************/
/* Scheduler Event Sync Object                                             */

BERR_Code BVC5_P_ClientNewSchedEvent(BVC5_ClientHandle hClient, uint64_t *pSchedEventId)
{
   return BVC5_P_NewSchedEvent(hClient->hEvents, pSchedEventId);
}

void BVC5_P_ClientDeleteSchedEvent(BVC5_ClientHandle hClient, uint64_t uiSchedEventId)
{
   BVC5_P_DeleteSchedEvent(hClient->hEvents, uiSchedEventId);
}

void BVC5_P_ClientSetSchedEvent(BVC5_ClientHandle hClient, uint64_t uiSchedEventId)
{
   BVC5_P_SetSchedEvent(hClient->hEvents, uiSchedEventId);
}

void BVC5_P_ClientResetSchedEvent(BVC5_ClientHandle hClient, uint64_t uiSchedEventId)
{
   BVC5_P_ResetSchedEvent(hClient->hEvents, uiSchedEventId);
}

bool BVC5_P_ClientQuerySchedEvent(BVC5_ClientHandle hClient, uint64_t uiSchedEventId)
{
   return BVC5_P_QuerySchedEvent(hClient->hEvents, uiSchedEventId);
}

bool BVC5_P_ClientWaitOnSchedEventDone(BVC5_ClientHandle hClient, uint64_t uiSchedEventId)
{
   return BVC5_P_WaitOnSchedEventDone(hClient->hEvents, uiSchedEventId);
}
