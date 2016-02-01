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
#include "bvc5.h"
#include "bvc5_internal_job_priv.h"
#include "bvc5_jobq_priv.h"

BDBG_MODULE(BVC5_P);

typedef struct BVC5_P_JobQ
{
   uint32_t                                  uiSize;
   BLST_Q_HEAD(sQueue, BVC5_P_InternalJob)   sQueue;
} BVC5_P_JobQ;


/***************************************************************************/

BERR_Code BVC5_P_JobQCreate(
   BVC5_JobQHandle *phJobQ
)
{
   BVC5_JobQHandle hJobQ;

   if (phJobQ == NULL)
      return BERR_INVALID_PARAMETER;

   hJobQ = (BVC5_JobQHandle)BKNI_Malloc(sizeof(struct BVC5_P_JobQ));
   if (hJobQ == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   BLST_Q_INIT(&hJobQ->sQueue);
   hJobQ->uiSize = 0;

   *phJobQ = hJobQ;

   return BERR_SUCCESS;
}

/***************************************************************************/

BERR_Code BVC5_P_JobQDestroy(
   BVC5_Handle     hVC5,
   BVC5_JobQHandle hJobQ
)
{
   BVC5_P_InternalJob   *pJob;

   if (hJobQ == NULL)
      return BERR_INVALID_PARAMETER;

   while ((pJob = BVC5_P_JobQPop(hJobQ)) != NULL)
   {
      /* Destroy resources owned by this job */
      BVC5_P_JobDestroy(hVC5, pJob);
   }

   BKNI_Free(hJobQ);

   return BERR_SUCCESS;
}

/***************************************************************************/

BVC5_P_InternalJob *BVC5_P_JobQPop(
   BVC5_JobQHandle hJobQ
)
{
   BVC5_P_InternalJob   *psJob;

   if (hJobQ == NULL || hJobQ->uiSize == 0)
      return NULL;

   psJob = BLST_Q_FIRST(&hJobQ->sQueue);

   BLST_Q_REMOVE_HEAD(&hJobQ->sQueue, sJobqChain);

   hJobQ->uiSize--;

   return psJob;
}

BVC5_P_InternalJob *BVC5_P_JobQTop(
   BVC5_JobQHandle hJobQ
)
{
   BVC5_P_InternalJob   *psJob;

   if (hJobQ == NULL || hJobQ->uiSize == 0)
      return NULL;

   psJob = BLST_Q_FIRST(&hJobQ->sQueue);

   return psJob;
}

/***************************************************************************/

void BVC5_P_JobQInsertTail(
   BVC5_JobQHandle      hJobQ,
   BVC5_P_InternalJob  *psJob
)
{
   BLST_Q_INSERT_TAIL(&hJobQ->sQueue, psJob, sJobqChain);

   hJobQ->uiSize++;
}

/***************************************************************************/

void BVC5_P_JobQInsertHead(
   BVC5_JobQHandle      hJobQ,
   BVC5_P_InternalJob  *psJob
)
{
   BLST_Q_INSERT_HEAD(&hJobQ->sQueue, psJob, sJobqChain);

   hJobQ->uiSize++;
}

/***************************************************************************/

void BVC5_P_JobQRemove(
   BVC5_JobQHandle      hJobQ,
   BVC5_P_InternalJob  *psJob
)
{
   if (hJobQ == NULL || psJob == NULL)
      return;

   BLST_Q_REMOVE(&hJobQ->sQueue, psJob, sJobqChain);

   hJobQ->uiSize--;
}

/***************************************************************************/

/* BVC5_P_JobQFindById
 * Removes a job given its id.  Returns the job.
 */
BVC5_P_InternalJob *BVC5_P_JobQFindById(
   BVC5_JobQHandle   hJobQ,
   uint64_t          uiJobId
)
{
   BVC5_P_InternalJob  *psJob = NULL;

   if (hJobQ == NULL)
      return NULL;

   for (psJob = BLST_Q_FIRST(&hJobQ->sQueue); psJob != NULL; psJob = BLST_Q_NEXT(psJob, sJobqChain))
   {
      /* found? */
      if (psJob->uiJobId == uiJobId)
         return psJob;

      /* not going to find it now */
      if (psJob->uiJobId > uiJobId)
         return NULL;
   }

   return NULL;
}

/***************************************************************************/

/* BVC5_P_JobQRemoveById
 * Removes a job given its id.  Returns the job.
 */
BVC5_P_InternalJob *BVC5_P_JobQRemoveById(
   BVC5_JobQHandle   hJobQ,
   uint64_t          uiJobId
)
{
   BVC5_P_InternalJob  *psJob = BVC5_P_JobQFindById(hJobQ, uiJobId);

   if (psJob != NULL)
      BVC5_P_JobQRemove(hJobQ, psJob);

   return psJob;
}

/***************************************************************************/

bool BVC5_P_JobQContainsId(
   BVC5_JobQHandle   hJobQ,
   uint64_t          uiJobId
)
{
   BVC5_P_InternalJob  *psJob;

   for (psJob = BLST_Q_FIRST(&hJobQ->sQueue); psJob != NULL; psJob = BLST_Q_NEXT(psJob, sJobqChain))
   {
      if (psJob->uiJobId == uiJobId)
         return true;

      if (psJob->uiJobId > uiJobId)
         return false;
   }

   return false;
}

/***************************************************************************/

uint32_t BVC5_P_JobQSize(
   BVC5_JobQHandle hJobQ
)
{
   return hJobQ->uiSize;
}

/***************************************************************************/

BVC5_P_InternalJob *BVC5_P_JobQFirst(
   BVC5_JobQHandle hJobQ
)
{
   return BLST_Q_FIRST(&hJobQ->sQueue);
}

/***************************************************************************/

BVC5_P_InternalJob *BVC5_P_JobQNext(
   BVC5_P_InternalJob  *psIter
)
{
   return BLST_Q_NEXT(psIter, sJobqChain);
}

/***************************************************************************/

BVC5_P_InternalJob *BVC5_P_JobQLast(
   BVC5_JobQHandle hJobQ
)
{
   return BLST_Q_LAST(&hJobQ->sQueue);
}

/***************************************************************************/

BVC5_P_InternalJob *BVC5_P_JobQPrev(
   BVC5_P_InternalJob  *psIter
)
{
   return BLST_Q_PREV(psIter, sJobqChain);
}

/***************************************************************************/

static void BVC5_P_JobQInsertAfter(
   BVC5_JobQHandle      hJobQ,
   BVC5_P_InternalJob  *pAt,
   BVC5_P_InternalJob  *pNew
)
{
   BLST_Q_INSERT_AFTER(&hJobQ->sQueue, pAt, pNew, sJobqChain);

   hJobQ->uiSize++;
}

/* BVC5_P_JobQInsert

   Insert a job into a sorted queue.  Start at end of queue as higher numbers come later
   and will be towards the back of the queue.

 */
void BVC5_P_JobQInsert(
   BVC5_JobQHandle      hJobQ,
   BVC5_P_InternalJob  *pJob
)
{
   BVC5_P_InternalJob   *pIter;

   /* Find insert point */
   for (pIter = BVC5_P_JobQLast(hJobQ); pIter != NULL; pIter = BVC5_P_JobQPrev(pIter))
   {
      /* Found insert point? */
      if (pIter->uiJobId < pJob->uiJobId)
         break;
   }

   if (pIter == NULL)
      BVC5_P_JobQInsertHead(hJobQ, pJob);
   else
      BVC5_P_JobQInsertAfter(hJobQ, pIter, pJob);
}

/***************************************************************************/

/*
void BVC5_P_JobQDump(
   BVC5_JobQHandle hJobQ
)
{
   BVC5_P_InternalJob   *pJob;

   for (pJob = BVC5_P_JobQFirst(hJobQ); pJob != NULL; pJob = BVC5_P_JobQNext(pJob))
   {
      BVC5_P_JobDump(pJob);
   }
}
*/
