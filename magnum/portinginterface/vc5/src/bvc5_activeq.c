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
#include "bvc5.h"
#include "bvc5_internal_job_priv.h"
#include "bvc5_activeq_priv.h"

BDBG_MODULE(BVC5_P);

typedef struct BVC5_P_ActiveQ
{
   uint32_t                                  uiSize;
   BLST_Q_HEAD(sQueue, BVC5_P_InternalJob)   sQueue;
} BVC5_P_ActiveQ;


/***************************************************************************/

BERR_Code BVC5_P_ActiveQCreate(
   BVC5_ActiveQHandle *phActiveQ
)
{
   BVC5_ActiveQHandle hActiveQ;

   if (phActiveQ == NULL)
      return BERR_INVALID_PARAMETER;

   hActiveQ = (BVC5_ActiveQHandle)BKNI_Malloc(sizeof(struct BVC5_P_ActiveQ));
   if (hActiveQ == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   BLST_Q_INIT(&hActiveQ->sQueue);
   hActiveQ->uiSize = 0;

   *phActiveQ = hActiveQ;

   return BERR_SUCCESS;
}

/***************************************************************************/

BERR_Code BVC5_P_ActiveQDestroy(
   BVC5_ActiveQHandle hActiveQ
)
{
   if (hActiveQ == NULL)
      return BERR_INVALID_PARAMETER;

   BKNI_Free(hActiveQ);

   return BERR_SUCCESS;
}


/***************************************************************************/

void BVC5_P_ActiveQInsertTail(
   BVC5_ActiveQHandle   hActiveQ,
   BVC5_P_InternalJob  *psJob
)
{
   BLST_Q_INSERT_TAIL(&hActiveQ->sQueue, psJob, sActiveqChain);

   hActiveQ->uiSize++;
}

/***************************************************************************/

void BVC5_P_ActiveQInsertHead(
   BVC5_ActiveQHandle   hActiveQ,
   BVC5_P_InternalJob  *psJob
)
{
   BLST_Q_INSERT_HEAD(&hActiveQ->sQueue, psJob, sActiveqChain);

   hActiveQ->uiSize++;
}

/***************************************************************************/

void BVC5_P_ActiveQRemove(
   BVC5_ActiveQHandle   hActiveQ,
   BVC5_P_InternalJob  *psJob
)
{
   if (hActiveQ == NULL || psJob == NULL)
      return;

   BLST_Q_REMOVE(&hActiveQ->sQueue, psJob, sActiveqChain);

   hActiveQ->uiSize--;
}

/***************************************************************************/

/* BVC5_P_ActiveQFindById
 * Removes a job given its id.  Returns the job.
 */
BVC5_P_InternalJob *BVC5_P_ActiveQFindById(
   BVC5_ActiveQHandle   hActiveQ,
   uint64_t             uiJobId
)
{
   BVC5_P_InternalJob  *psJob = NULL;

   if (hActiveQ == NULL)
      return NULL;

   for (psJob = BLST_Q_FIRST(&hActiveQ->sQueue); psJob != NULL; psJob = BLST_Q_NEXT(psJob, sActiveqChain))
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

/* BVC5_P_ActiveQRemoveById
 * Removes a job given its id.  Returns the job.
 */
BVC5_P_InternalJob *BVC5_P_ActiveQRemoveById(
   BVC5_ActiveQHandle   hActiveQ,
   uint64_t             uiJobId
)
{
   BVC5_P_InternalJob  *psJob = BVC5_P_ActiveQFindById(hActiveQ, uiJobId);

   if (psJob != NULL)
      BVC5_P_ActiveQRemove(hActiveQ, psJob);

   return psJob;
}

/***************************************************************************/

bool BVC5_P_ActiveQContainsId(
   BVC5_ActiveQHandle   hActiveQ,
   uint64_t             uiJobId
)
{
   BDBG_MSG(("BVC5_P_ActiveQContainsId jobID="BDBG_UINT64_FMT, BDBG_UINT64_ARG(uiJobId)));
   return BVC5_P_ActiveQFindById(hActiveQ, uiJobId) != NULL;
}

/***************************************************************************/

uint32_t BVC5_P_ActiveQSize(
   BVC5_ActiveQHandle hActiveQ
)
{
   return hActiveQ->uiSize;
}

/***************************************************************************/

BVC5_P_InternalJob *BVC5_P_ActiveQFirst(
   BVC5_ActiveQHandle hActiveQ
)
{
   return BLST_Q_FIRST(&hActiveQ->sQueue);
}

/***************************************************************************/

BVC5_P_InternalJob *BVC5_P_ActiveQNext(
   BVC5_P_InternalJob  *psIter
)
{
   return BLST_Q_NEXT(psIter, sActiveqChain);
}

/***************************************************************************/

BVC5_P_InternalJob *BVC5_P_ActiveQLast(
   BVC5_ActiveQHandle hActiveQ
)
{
   return BLST_Q_LAST(&hActiveQ->sQueue);
}

/***************************************************************************/

BVC5_P_InternalJob *BVC5_P_ActiveQPrev(
   BVC5_P_InternalJob  *psIter
)
{
   return BLST_Q_PREV(psIter, sActiveqChain);
}

/***************************************************************************/

static void BVC5_P_ActiveQInsertAfter(
   BVC5_ActiveQHandle   hActiveQ,
   BVC5_P_InternalJob  *pAt,
   BVC5_P_InternalJob  *pNew
)
{
   BLST_Q_INSERT_AFTER(&hActiveQ->sQueue, pAt, pNew, sActiveqChain);

   hActiveQ->uiSize++;
}

/* BVC5_P_ActiveQInsert

   Insert a job into a sorted queue.  Start at end of queue as higher numbers come later
   and will be towards the back of the queue.

 */
void BVC5_P_ActiveQInsert(
   BVC5_ActiveQHandle   hActiveQ,
   BVC5_P_InternalJob  *pJob
)
{
   BVC5_P_InternalJob   *pIter;

   /* Find insert point */
   for (pIter = BVC5_P_ActiveQLast(hActiveQ); pIter != NULL; pIter = BVC5_P_ActiveQPrev(pIter))
   {
      /* Found insert point? */
      if (pIter->uiJobId < pJob->uiJobId)
         break;
   }

   if (pIter == NULL)
      BVC5_P_ActiveQInsertHead(hActiveQ, pJob);
   else
      BVC5_P_ActiveQInsertAfter(hActiveQ, pIter, pJob);
}

/***************************************************************************/
