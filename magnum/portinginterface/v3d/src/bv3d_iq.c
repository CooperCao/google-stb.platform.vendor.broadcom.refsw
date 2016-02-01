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
#include "bv3d_iq_priv.h"
#include "bkni.h"
#include "blst_queue.h"

typedef struct BV3D_P_IQInstruction
{
   BV3D_Instruction * psInstruction;
   BLST_Q_ENTRY(BV3D_P_IQInstruction)      sChain;
} BV3D_P_IQInstruction;

typedef struct BV3D_P_IQHandle
{
   BLST_Q_HEAD(sQueue, BV3D_P_IQInstruction) sQueue;
   bool              bWaiting;
   uint32_t          uiSize;              /* queue size */
} BV3D_P_IQHandle;

/***************************************************************************/
BERR_Code BV3D_P_IQCreate(
   BV3D_IQHandle *phIQ
)
{
   BV3D_IQHandle hIQ;

   if (phIQ == NULL)
      return BERR_INVALID_PARAMETER;

   hIQ = (BV3D_IQHandle)BKNI_Malloc(sizeof(BV3D_P_IQHandle));
   if (hIQ == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   BLST_Q_INIT(&hIQ->sQueue);

   hIQ->uiSize = 0;
   hIQ->bWaiting = false;

   *phIQ = hIQ;

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_IQDestroy(
   BV3D_IQHandle hIQ
)
{
   if (hIQ == NULL)
      return BERR_INVALID_PARAMETER;

   BDBG_ASSERT(BLST_Q_EMPTY(&hIQ->sQueue));

   BKNI_Free(hIQ);

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_IQPush(
   BV3D_IQHandle hIQ,
   BV3D_Instruction *psInstruction
)
{
   BV3D_P_IQInstruction * pIQInstruction;

   if ((hIQ == NULL) || (psInstruction == NULL))
      return BERR_INVALID_PARAMETER;

   pIQInstruction = (BV3D_P_IQInstruction *)BKNI_Malloc(sizeof(BV3D_P_IQInstruction));
   if (pIQInstruction == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   pIQInstruction->psInstruction = psInstruction;
   hIQ->uiSize += 1;

   BLST_Q_INSERT_TAIL(&hIQ->sQueue, pIQInstruction, sChain);

   return BERR_SUCCESS;
}

/***************************************************************************/
BV3D_Instruction * BV3D_P_IQPop(
   BV3D_IQHandle hIQ
)
{
   BV3D_P_IQInstruction * pIQInstruction;
   BV3D_Instruction *psInstruction = NULL;

   if (hIQ == NULL)
      return NULL;

   pIQInstruction = BLST_Q_FIRST(&hIQ->sQueue);
   if (pIQInstruction)
   {
      BLST_Q_REMOVE_HEAD(&hIQ->sQueue, sChain);

      hIQ->uiSize -= 1;

      psInstruction = pIQInstruction->psInstruction;

      BKNI_Free(pIQInstruction);
   }

   return psInstruction;
}

/***************************************************************************/
BV3D_Instruction * BV3D_P_IQTop(
   BV3D_IQHandle hIQ
)
{
   BV3D_P_IQInstruction * pIQInstruction;
   BV3D_Instruction *psInstruction;

   if (hIQ == NULL)
      return NULL;

   pIQInstruction = BLST_Q_FIRST(&hIQ->sQueue);

   psInstruction = pIQInstruction->psInstruction;

   return psInstruction;
}

/***************************************************************************/
bool BV3D_P_IQGetWaiting(
   BV3D_IQHandle hIQ
)
{
   if (hIQ == NULL)
      return false;

   return hIQ->bWaiting;
}

/***************************************************************************/
BERR_Code BV3D_P_IQSetWaiting(
   BV3D_IQHandle hIQ,
   bool bWaiting
)
{
   if (hIQ == NULL)
      return BERR_INVALID_PARAMETER;

   hIQ->bWaiting = bWaiting;

   return BERR_SUCCESS;
}

/***************************************************************************/
uint32_t BV3D_P_IQGetSize(
   BV3D_IQHandle hIQ
)
{
   if (hIQ == NULL)
      return -1;

   return hIQ->uiSize;
}

/***************************************************************************/
bool BV3D_P_JobQFindBinOrWaitBinAndMoveToTop(BV3D_IQHandle hIQ)
{
   BV3D_P_IQInstruction * pIQInstruction;
   BV3D_Instruction *psInstruction = NULL;

   for (pIQInstruction = BLST_Q_FIRST(&hIQ->sQueue);
        pIQInstruction != NULL && psInstruction == NULL;
        pIQInstruction = BLST_Q_NEXT(pIQInstruction, sChain))
   {
      /* if its a bin, move it to the top */
      if ((pIQInstruction->psInstruction->eOperation == BV3D_Operation_eBinInstr) ||
          ((pIQInstruction->psInstruction->eOperation == BV3D_Operation_eWaitInstr) &&
           (pIQInstruction->psInstruction->uiArg1 & BV3D_Signaller_eBinSig)))
      {
         psInstruction = pIQInstruction->psInstruction;

         BLST_Q_REMOVE(&hIQ->sQueue, pIQInstruction, sChain);
         BLST_Q_INSERT_HEAD(&hIQ->sQueue, pIQInstruction, sChain);
      }
      else if ((pIQInstruction->psInstruction->eOperation != BV3D_Operation_eRenderInstr) &&
               (pIQInstruction->psInstruction->eOperation != BV3D_Operation_eNotifyInstr) &&
               (pIQInstruction->psInstruction->eOperation != BV3D_Operation_eFenceInstr))
      {
         break;
      }
   }

   return psInstruction != NULL;
}
