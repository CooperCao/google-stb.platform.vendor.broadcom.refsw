/***************************************************************************
 *     Copyright (C) 2012 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#include "bv3d.h"
#include "bv3d_priv.h"
#include "bv3d_iq_priv.h"
#include "bv3d_qmap_priv.h"
#include "bv3d_worker_priv.h"

BDBG_MODULE(BV3D);

/* 7255: ignore until 64bit issues addressed properly */
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

/***************************************************************************/
bool BV3D_P_InstructionIsClear(
   const BV3D_Instruction *psInstruction
)
{
   if (psInstruction == NULL)
      return true;

   return psInstruction->eOperation == BV3D_Operation_eEndInstr;
}

/***************************************************************************/
void BV3D_P_InstructionClear(
   BV3D_Instruction *psInstruction
)
{
   if (psInstruction != NULL)
   {
      psInstruction->eOperation = BV3D_Operation_eEndInstr;
      psInstruction->psJob = NULL;
      psInstruction->uiArg1 = 0;
      psInstruction->uiArg2 = 0;
      psInstruction->uiCallbackParam = 0;
   }
}

/***************************************************************************/
static void BV3D_P_JobAdvanceInstruction(
   BV3D_Job *psJob)
{
   if (psJob->uiCurrentInstr < V3D_JOB_MAX_INSTRUCTIONS)
      psJob->uiCurrentInstr += 1;
}

/***************************************************************************/
static BV3D_Instruction * BV3D_P_JobGetCurrentInstruction(
   BV3D_Job *psJob
)
{
   BV3D_Instruction *psInstruction = NULL;

   if (psJob->uiCurrentInstr < V3D_JOB_MAX_INSTRUCTIONS)
      psInstruction = &psJob->sProgram[psJob->uiCurrentInstr];

   return psInstruction;
}

/***************************************************************************/
/* Copy all the instructions up to end into client queue */
BERR_Code BV3D_P_UnrollJob(
   BV3D_Handle hV3d,
   BV3D_Job *psJob
)
{
   BV3D_Instruction *psInstruction;
   BV3D_IQHandle hIQ;

   hIQ = BV3D_P_IQMapGet(hV3d->hIQMap, psJob->uiClientId);

   /* Check client still exists */
   if (hIQ != NULL)
   {
      /* Own the bin memory allocated for this job so it can be cleaned up when the job ends */
      if (psJob->uiBinMemory != 0)
      {
         BV3D_BinMemHandle   hMem = (BV3D_BinMemHandle)psJob->uiBinMemory;
         /* Attached prior to the job running, and pulled via Nexus API and passed to the driver.
            This may be inconstent to hV3d->bSecure, but it will have transitioned to the correct state
            by the time the job runs. */
         bool bBinMemorySecure = psJob->bBinMemorySecure;

         /* The job owns this memory chunk */
         BV3D_P_BinMemAttachToJob(bBinMemorySecure ? hV3d->hBinMemManagerSecure : hV3d->hBinMemManager, hMem, psJob);
      }

      while ((psInstruction = BV3D_P_JobGetCurrentInstruction(psJob)) &&
             !BV3D_P_InstructionIsClear(psInstruction) &&
             psJob->uiInstrCount < V3D_JOB_MAX_INSTRUCTIONS)
      {
         psInstruction->psJob = psJob;
         BV3D_P_IQPush(hIQ, psInstruction);
         BV3D_P_JobAdvanceInstruction(psJob);
         psJob->uiInstrCount += 1;
      }
   }
   else
   {
      /* TODO */
   }

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_SendJob(
   BV3D_Handle hV3d,
   BV3D_Job *psJob
)
{
   BV3D_Job *pOurJob;
   BDBG_ENTER(BV3D_SendJob);

   if (psJob == NULL)
   {
      BDBG_LEAVE(BV3D_SendJob);
      return BERR_INVALID_PARAMETER;
   }

   /* freed at the backend of the queue */
   pOurJob = (BV3D_Job *)BKNI_Malloc(sizeof (BV3D_Job));

   if (pOurJob == NULL)
   {
      BDBG_LEAVE(BV3D_SendJob);
      return BERR_OUT_OF_SYSTEM_MEMORY;
   }

   BKNI_AcquireMutex(hV3d->hModuleMutex);

   /* careful, inline structure copy */
   /* TODO: probably should clear out fields that we want zero -- shouldn't rely on caller */
   *pOurJob = *psJob;

   if (pOurJob->bCollectTimeline)
      BKNI_Memset(&pOurJob->sTimelineData, 0, sizeof(BV3D_TimelineData));

   if (pOurJob->uiUserVPM != hV3d->uiUserVPM || BV3D_P_WaitQSize(hV3d->hWaitQ) != 0)
      BV3D_P_WaitQPush(hV3d->hWaitQ, pOurJob);
   else
      BV3D_P_UnrollJob(hV3d, pOurJob);

   BKNI_SetEvent(hV3d->wakeWorkerThread);

   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_SendJob);

   return BERR_SUCCESS;
}
