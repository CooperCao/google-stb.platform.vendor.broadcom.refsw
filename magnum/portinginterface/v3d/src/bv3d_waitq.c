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
#include "bv3d.h"
#include "bv3d_waitq_priv.h"
#include "blst_queue.h"

typedef struct BV3D_P_WaitQJob
{
   BV3D_Job * psJob;
   BLST_Q_ENTRY(BV3D_P_WaitQJob)      sChain;
} BV3D_P_WaitQJob;

typedef struct BV3D_P_WaitQHandle
{
   uint32_t uiSize;
   BLST_Q_HEAD(sQueue, BV3D_P_WaitQJob) sQueue;
} BV3D_P_WaitQHandle;


/***************************************************************************/
BERR_Code BV3D_P_WaitQCreate(
   BV3D_WaitQHandle *phWaitQ
)
{
   BV3D_WaitQHandle hWaitQ;

   if (phWaitQ == NULL)
      return BERR_INVALID_PARAMETER;

   hWaitQ = (BV3D_WaitQHandle)BKNI_Malloc(sizeof(BV3D_P_WaitQHandle));
   if (hWaitQ == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   BLST_Q_INIT(&hWaitQ->sQueue);
   hWaitQ->uiSize = 0;

   *phWaitQ = hWaitQ;

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_WaitQDestroy(
   BV3D_WaitQHandle hWaitQ
)
{
   if (hWaitQ == NULL)
      return BERR_INVALID_PARAMETER;

   BDBG_ASSERT(BLST_Q_EMPTY(&hWaitQ->sQueue));
   BDBG_ASSERT(hWaitQ->uiSize == 0);

   BKNI_Free(hWaitQ);

   return BERR_SUCCESS;
}

/***************************************************************************/
BV3D_Job * BV3D_P_WaitQTop(
   BV3D_WaitQHandle hWaitQ
)
{
   BV3D_P_WaitQJob *psWaitQJob;

   if (hWaitQ == NULL)
      return NULL;

   psWaitQJob = BLST_Q_FIRST(&hWaitQ->sQueue);
   if (psWaitQJob == NULL)
      return NULL;

   return psWaitQJob->psJob;
}

/***************************************************************************/
BERR_Code BV3D_P_WaitQPop(
   BV3D_WaitQHandle hWaitQ
)
{
   BV3D_P_WaitQJob *psWaitQJob;

   if (hWaitQ == NULL)
      return BERR_INVALID_PARAMETER;

   BDBG_ASSERT(!BLST_Q_EMPTY(&hWaitQ->sQueue));
   BDBG_ASSERT(hWaitQ->uiSize > 0);

   psWaitQJob = BLST_Q_FIRST(&hWaitQ->sQueue);

   BLST_Q_REMOVE_HEAD(&hWaitQ->sQueue, sChain);

   BKNI_Free(psWaitQJob);
   hWaitQ->uiSize--;

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_WaitQPush(
   BV3D_WaitQHandle hWaitQ,
   BV3D_Job *psJob
)
{
   BV3D_P_WaitQJob *psWaitQJob;

   if ((hWaitQ == NULL) || (psJob == NULL))
      return BERR_INVALID_PARAMETER;

   psWaitQJob = (BV3D_P_WaitQJob *)BKNI_Malloc(sizeof(BV3D_P_WaitQJob));
   if (psWaitQJob == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   psWaitQJob->psJob = psJob;

   BLST_Q_INSERT_TAIL(&hWaitQ->sQueue, psWaitQJob, sChain);

   hWaitQ->uiSize++;

   return BERR_SUCCESS;
}

/***************************************************************************/
uint32_t BV3D_P_WaitQSize(
   BV3D_WaitQHandle hWaitQ
)
{
   return hWaitQ->uiSize;
}
