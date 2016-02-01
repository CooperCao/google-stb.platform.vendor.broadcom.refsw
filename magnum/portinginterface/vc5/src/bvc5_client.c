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

#include "bstd.h"
#include "bkni.h"
#include "bvc5_priv.h"
#include "bvc5_jobq_priv.h"
#include "bvc5_client_priv.h"

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

BERR_Code BVC5_P_ClientCreate(
   BVC5_Handle        hVC5,
   BVC5_ClientHandle *phClient,
   uint32_t           uiClientId
)
{
   BERR_Code   err = BERR_OUT_OF_SYSTEM_MEMORY;

   BVC5_ClientHandle hClient = (BVC5_P_Client *)BKNI_Malloc(sizeof(BVC5_P_Client));

   *phClient = hClient;

   if (hClient == NULL)
      goto exit;

   BKNI_Memset(hClient, 0, sizeof(BVC5_P_Client));

   hClient->uiClientId = uiClientId;

   hClient->uiMaxJobId = 0;
   hClient->uiOldestNotFinalized = 1;

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

   if (hClient->hCompletedQ != NULL)
      BVC5_P_JobQDestroy(hVC5, hClient->hCompletedQ);

   if (hClient->hFinalizableQ != NULL)
      BVC5_P_JobQDestroy(hVC5, hClient->hFinalizableQ);

   if (hClient->hFinalizingQ != NULL)
      BVC5_P_JobQDestroy(hVC5, hClient->hFinalizingQ);

   if (hClient->hActiveJobs != NULL)
      BVC5_P_ActiveQDestroy(hClient->hActiveJobs);

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
   uint32_t                uiClientId
)
{
   BVC5_ClientHandle hClient = NULL;
   BERR_Code         err     = BERR_SUCCESS;

   if (hClientMap == NULL)
      return BERR_INVALID_PARAMETER;

   err = BVC5_P_ClientCreate(hVC5, &hClient, uiClientId);
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
   BKNI_Printf("%s : FATAL : Duplicate client id inserted to map\n", __FUNCTION__);
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

/* JOB STATE TRANSITIONS */

void BVC5_P_ClientJobToWaiting(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *pJob
)
{
   BSTD_UNUSED(hVC5);

   BVC5_P_ActiveQInsert(hClient->hActiveJobs, pJob);
   BVC5_P_JobQInsert(hClient->hWaitQ, pJob);

   /* A CPU cache-flush must precede this and any subsequent job. */
   if (pJob->pBase->uiSyncFlags & BVC5_SYNC_CPU_WRITE)
      hClient->uiFlushCpuCacheReq += 1;

   /* Since no subsequent CPU writes can affect this job, it is safe to capture
    * uiFlushCpuCacheReq here. If proxy jobs are added, then uiFlushCpuCacheReq would
    * need to to captured for dependent jobs when they become runnable. The proxy
    * job would need to signal to increment hClient->uiFlushCpuCacheReq first. */
   pJob->uiFlushCpuCacheReq = hClient->uiFlushCpuCacheReq;
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
   default                     : return hClient->hRunnableSoftQ;
   }
}

void BVC5_P_ClientJobWaitingToRunnable(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
)
{
   BVC5_JobQHandle   hRunQ = BVC5_P_RunQ(hClient, psJob->pBase->eType);

   BSTD_UNUSED(hVC5);
   BDBG_ASSERT(hRunQ != NULL);

   /* Add to client runnable list and remove from waitq */
   BVC5_P_JobQRemove(hClient->hWaitQ, psJob);
   BVC5_P_JobQInsert(hRunQ, psJob);
}

void BVC5_P_ClientJobRunningToCompleted(
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
)
{
   BVC5_P_JobQInsert(hClient->hCompletedQ, psJob);
   BVC5_P_ActiveQRemove(hClient->hActiveJobs, psJob);
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

static uint64_t BVC5_P_OldestInCache(
   BVC5_ActiveQHandle  hActive,
   uint64_t            uiLowestSoFar
   )
{
   BVC5_P_InternalJob  *psJob = BVC5_P_ActiveQFirst(hActive);

   if (psJob == NULL || psJob->uiJobId > uiLowestSoFar)
      return uiLowestSoFar;

   return psJob->uiJobId;
}

static uint64_t BVC5_P_OldestInList(
   BVC5_JobQHandle   hJobQ,
   uint64_t          uiLowestSoFar
)
{
   BVC5_P_InternalJob  *psJob = BVC5_P_JobQFirst(hJobQ);

   if (psJob == NULL || psJob->uiJobId > uiLowestSoFar)
      return uiLowestSoFar;

   return psJob->uiJobId;
}

static void BVC5_P_UpdateOldestId(
   BVC5_ClientHandle hClient,
   uint64_t          uiFinalizedJobId
)
{
   /* If its not the oldest id, so we can keep the current oldest id, otherwise recalculate it */
   if (uiFinalizedJobId == hClient->uiOldestNotFinalized || hClient->uiOldestNotFinalized == 0)
   {
      uint64_t uiLowest = ~((uint64_t)0);

      uiLowest = BVC5_P_OldestInCache(hClient->hActiveJobs,  uiLowest);
      uiLowest = BVC5_P_OldestInList(hClient->hCompletedQ,   uiLowest);
      uiLowest = BVC5_P_OldestInList(hClient->hFinalizableQ, uiLowest);
      uiLowest = BVC5_P_OldestInList(hClient->hFinalizingQ,  uiLowest);

      hClient->uiOldestNotFinalized = (uiLowest != ~((uint64_t)0) ? uiLowest : (hClient->uiMaxJobId + 1));
   }
}

/* Move a job to finalized state - should only be used if the job has no callback fn */
void BVC5_P_ClientJobCompletedToFinalized(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
   )
{
   uint64_t jobId = psJob->uiJobId;

   BVC5_P_JobQRemove(hClient->hCompletedQ, psJob);

   /* And destroy it -- its finished */
   if (psJob != NULL)
      BVC5_P_JobDestroy(hVC5, psJob);

   BVC5_P_UpdateOldestId(hClient, jobId);
}

void BVC5_P_ClientJobFinalizingToFinalized(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   uint64_t             uiJobId
)
{
   BVC5_P_InternalJob *psJob = BVC5_P_JobQRemoveById(hClient->hFinalizingQ, uiJobId);

   /* And destroy it -- its finished */
   if (psJob != NULL)
      BVC5_P_JobDestroy(hVC5, psJob);

   BVC5_P_UpdateOldestId(hClient, uiJobId);
}

bool BVC5_P_ClientIsJobComplete(
   BVC5_ClientHandle hClient,
   uint64_t          uiJobId
)
{
   /* If this job id is lower than the oldest not finalized then it must have left the system,
      otherwise we can check if the job is still not completed */
   return uiJobId < hClient->uiOldestNotFinalized ||
          !BVC5_P_ActiveQContainsId(hClient->hActiveJobs, uiJobId);
}

bool BVC5_P_ClientIsJobFinishing(
   BVC5_ClientHandle hClient,
   uint64_t          uiJobId
)
{
   return BVC5_P_JobQContainsId(hClient->hCompletedQ,   uiJobId)  ||
          BVC5_P_JobQContainsId(hClient->hFinalizableQ, uiJobId)  ||
          BVC5_P_JobQContainsId(hClient->hFinalizingQ,  uiJobId);
}

bool BVC5_P_ClientIsJobFinalized(
   BVC5_ClientHandle hClient,
   uint64_t          uiJobId
)
{
   return BVC5_P_ClientIsJobComplete(hClient, uiJobId) &&
         !BVC5_P_ClientIsJobFinishing(hClient, uiJobId);
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

uint64_t BVC5_P_ClientGetOldestNotFinalized(
   BVC5_ClientHandle hClient
)
{
   return hClient->uiOldestNotFinalized;
}

void BVC5_P_ClientMarkJobsFlushedV3D(
   BVC5_ClientHandle hClient
)
{
   BVC5_P_InternalJob   *pJob = NULL;
   BVC5_JobQHandle queues[5];
   int i;

   queues[0] = hClient->hRunnableSoftQ;
   queues[1] = hClient->hRunnableBinnerQ;
   queues[2] = hClient->hRunnableRenderQ;
   queues[3] = hClient->hRunnableTFUQ;
   queues[4] = hClient->hRunnableUsermodeQ;

   for (i = 0; i < 5; i++)
   {
      for (pJob = BVC5_P_JobQFirst(queues[i]);
         pJob != NULL;
         pJob = BVC5_P_JobQNext(pJob))
      {
         /* V3D flush is not valid unless CPU flush was done. */
         /* Handle case where uiFlushCpuCacheDone wraps eg 0x4 - 0xFFFFFFFA = 0xA, so is still OK. */
         if ((int32_t)(hClient->uiFlushCpuCacheDone - pJob->uiFlushCpuCacheReq) >= 0)
            pJob->bFlushedV3D = true;
      }
   }
}
