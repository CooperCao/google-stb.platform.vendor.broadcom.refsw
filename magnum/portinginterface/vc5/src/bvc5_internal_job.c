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
#include "bvc5_priv.h"
#include "bvc5_internal_job_priv.h"
#include "bvc5_bin_mem_priv.h"
#include "bvc5_client_priv.h"

#include "bvc5_fence_priv.h"

/* BVC5_P_CreateInternalJob

   Fill in all the common fields of the job.
 */
static BVC5_P_InternalJob *BVC5_P_CreateInternalJob(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   BVC5_JobBase            *toJob,
   BVC5_JobBase            *fromJob
   )
{
   BVC5_P_InternalJob   *pJob = NULL;

   BSTD_UNUSED(hVC5);

   if (toJob == NULL)
      return NULL;

   pJob = (BVC5_P_InternalJob *)BKNI_Malloc(sizeof(BVC5_P_InternalJob));
   if (pJob != NULL)
   {
      BKNI_Memset(pJob, 0, sizeof(BVC5_P_InternalJob));
      pJob->pBase      = toJob;
      pJob->uiJobId    = fromJob->uiJobId;
      pJob->uiClientId = uiClientId;
      BKNI_Memcpy(&pJob->sRunDep_NotCompleted, &fromJob->sCompletedDependencies, sizeof(BVC5_SchedDependencies));
      BKNI_Memcpy(&pJob->sRunDep_NotFinalized, &fromJob->sFinalizedDependencies, sizeof(BVC5_SchedDependencies));
      BKNI_Memcpy(&pJob->sFinDep_NotFinalized, &fromJob->sCompletedDependencies, sizeof(BVC5_SchedDependencies));
      pJob->eStatus    = BVC5_JobStatus_eSUCCESS;
   }

   return pJob;
}

static void BVC5_P_FreeInternalJob(BVC5_P_InternalJob *pJob)
{
   if (pJob != NULL)
      BKNI_Free(pJob);
}

BVC5_P_InternalJob *BVC5_P_JobCreateNull(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobNull      *psJob
)
{
   BVC5_JobNull         *pNullJob = (BVC5_JobNull *)BKNI_Malloc(sizeof(BVC5_JobNull));
   BVC5_P_InternalJob   *pJob = NULL;

   if (pNullJob != NULL)
   {
      pJob = BVC5_P_CreateInternalJob(hVC5, uiClientId, (BVC5_JobBase *)pNullJob, (BVC5_JobBase *)psJob);

      if (pJob != NULL)
         BKNI_Memcpy(pNullJob, psJob, sizeof(BVC5_JobNull));
      else
         BKNI_Free(pNullJob);
   }

   return pJob;
}

BVC5_P_InternalJob *BVC5_P_JobCreateBin(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobBin       *psJob,
   BVC5_P_InternalJob      *pRenderJob
)
{
   BVC5_JobBin          *pBinJob = (BVC5_JobBin *)BKNI_Malloc(sizeof(BVC5_JobBin));
   BVC5_P_InternalJob   *pJob = NULL;

   if (pBinJob != NULL)
   {
      pJob = BVC5_P_CreateInternalJob(hVC5, uiClientId, (BVC5_JobBase *)pBinJob, (BVC5_JobBase *)psJob);

      if (pJob != NULL)
      {
         BKNI_Memcpy(pBinJob, psJob, sizeof(BVC5_JobBin));
         pJob->jobData.sBin.psInternalRenderJob = pRenderJob;
         pJob->jobData.sBin.uiMinInitialBinBlockSize = psJob->uiMinInitialBinBlockSize;
      }
      else
         BKNI_Free(pBinJob);
   }

   return pJob;
}

BVC5_P_InternalJob *BVC5_P_JobCreateRender(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobRender    *psJob
)
{
   BVC5_JobRender       *pRenderJob = (BVC5_JobRender *)BKNI_Malloc(sizeof(BVC5_JobRender));
   BVC5_P_InternalJob   *pJob = NULL;

   if (pRenderJob != NULL)
   {
      pJob = BVC5_P_CreateInternalJob(hVC5, uiClientId, (BVC5_JobBase *)pRenderJob, (BVC5_JobBase *)psJob);

      if (pJob != NULL)
      {
         BKNI_Memcpy(pRenderJob, psJob, sizeof(BVC5_JobRender));

         if (BVC5_P_BinMemArrayCreate(&pJob->jobData.sRender.sBinMemArray) != BERR_SUCCESS)
         {
            BVC5_P_BinMemArrayDestroy(&pJob->jobData.sRender.sBinMemArray, NULL);
            BVC5_P_FreeInternalJob(pJob);
            BKNI_Free(pRenderJob);
            pJob = NULL;
         }
      }
      else
         BKNI_Free(pRenderJob);
   }

   return pJob;
}

BVC5_P_InternalJob *BVC5_P_JobCreateFenceSignal(
   BVC5_Handle                hVC5,
   uint32_t                   uiClientId,
   const BVC5_JobFenceSignal *psJob,
   int32_t                    *pFence
)
{
   BVC5_JobFenceSignal  *pFenceJob = (BVC5_JobFenceSignal *)BKNI_Malloc(sizeof(BVC5_JobFenceSignal));
   BVC5_P_InternalJob   *pJob = NULL;
   *pFence = -1;

   if (pFenceJob != NULL)
   {
      pJob = BVC5_P_CreateInternalJob(hVC5, uiClientId, (BVC5_JobBase *)pFenceJob, (BVC5_JobBase *)psJob);

      if (pJob != NULL)
      {
         BKNI_Memcpy(pFenceJob, psJob, sizeof(BVC5_JobFenceSignal));
         *pFence = BVC5_P_FenceCreate(hVC5->hFences, uiClientId, &pJob->jobData.sSignal.signalData);
         if (*pFence == -1)
         {
            BVC5_P_FreeInternalJob(pJob);
            BKNI_Free(pFenceJob);
            pJob = NULL;
         }
      }
      else
         BKNI_Free(pFenceJob);
   }

   return pJob;
}

static void BVC5_P_JobWaitCallback(void *context, void *param)
{
   BVC5_Handle        hVC5  = (BVC5_Handle)context;
   BSTD_UNUSED(param);

   /* Prod the scheduler */
   BKNI_SetEvent(hVC5->hSchedulerWakeEvent);
}

BVC5_P_InternalJob *BVC5_P_JobCreateFenceWait(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobFenceWait *psJob
)
{
   BVC5_JobFenceWait    *pWaitJob = (BVC5_JobFenceWait *)BKNI_Malloc(sizeof(BVC5_JobFenceWait));
   BVC5_P_InternalJob   *pJob = NULL;

   if (pWaitJob != NULL)
   {
      pJob = BVC5_P_CreateInternalJob(hVC5, uiClientId, (BVC5_JobBase *)pWaitJob, (BVC5_JobBase *)psJob);

      if (pJob != NULL)
      {
         int res;
         BKNI_Memcpy(pWaitJob, psJob, sizeof(BVC5_JobFenceWait));

         res = BVC5_P_FenceWaitAsync(hVC5->hFences, uiClientId, psJob->iFence, BVC5_P_JobWaitCallback, hVC5, pJob,
               &pJob->jobData.sWait.waitData);
         if (res < 0)
         {
            BVC5_P_FreeInternalJob(pJob);
            BKNI_Free(pWaitJob);
            pJob = NULL;
         }
         else if (res > 0)
         {
            /* the fence was already signalled;*/
            pJob->jobData.sWait.signaled = true;
            pJob->jobData.sWait.waitData = NULL;
         }
         if (res != 0)
         {
            /* the wait async didn't succedd or the fence was already signalled,
             * we must close the fd */
            BVC5_P_FenceClose(hVC5->hFences, psJob->iFence);
         }
      }
      else
         BKNI_Free(pWaitJob);
   }

   return pJob;
}

BVC5_P_InternalJob *BVC5_P_JobCreateTFU(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobTFU       *psJob
)
{
   BVC5_JobTFU          *psTFUJob = (BVC5_JobTFU *)BKNI_Malloc(sizeof(BVC5_JobTFU));
   BVC5_P_InternalJob   *pJob = NULL;

   if (psTFUJob != NULL)
   {
      pJob = BVC5_P_CreateInternalJob(hVC5, uiClientId, (BVC5_JobBase *)psTFUJob, (BVC5_JobBase *)psJob);

      if (pJob != NULL)
         BKNI_Memcpy(psTFUJob, psJob, sizeof(BVC5_JobTFU));
      else
         BKNI_Free(psTFUJob);

      /* Ensure sync flags are sensible for TFU jobs. */
      psTFUJob->sBase.uiSyncFlags &= BVC5_SYNC_TFU_READ
                                   | BVC5_SYNC_TFU_WRITE
                                   | BVC5_SYNC_CPU_READ
                                   | BVC5_SYNC_CPU_WRITE;
   }

   return pJob;
}

BVC5_P_InternalJob *BVC5_P_JobCreateTest(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobTest      *psJob
)
{
   BVC5_JobTest         *psTestJob = (BVC5_JobTest *)BKNI_Malloc(sizeof(BVC5_JobTest));
   BVC5_P_InternalJob   *pJob = NULL;

   if (psTestJob != NULL)
   {
      pJob = BVC5_P_CreateInternalJob(hVC5, uiClientId, (BVC5_JobBase *)psTestJob, (BVC5_JobBase *)psJob);

      if (pJob != NULL)
         BKNI_Memcpy(psTestJob, psJob, sizeof(BVC5_JobTest));
      else
         BKNI_Free(psTestJob);
   }

   return pJob;
}

BVC5_P_InternalJob *BVC5_P_JobCreateUsermode(
   BVC5_Handle              hVC5,
   uint32_t                 uiClientId,
   const BVC5_JobUsermode  *psJob
)
{
   BVC5_JobUsermode     *psUsermodeJob = (BVC5_JobUsermode *)BKNI_Malloc(sizeof(BVC5_JobUsermode));
   BVC5_P_InternalJob   *pJob = NULL;

   if (psUsermodeJob != NULL)
   {
      pJob = BVC5_P_CreateInternalJob(hVC5, uiClientId, (BVC5_JobBase *)psUsermodeJob, (BVC5_JobBase *)psJob);

      if (psUsermodeJob != NULL)
         BKNI_Memcpy(psUsermodeJob, psJob, sizeof(BVC5_JobUsermode));
      else
         BKNI_Free(psUsermodeJob);
   }

   return pJob;
}

void BVC5_P_JobDestroy(
   BVC5_Handle           hVC5,
   BVC5_P_InternalJob   *pJob
)
{
   BVC5_JobBase   *pJobBase;

   if (pJob == NULL)
      return;

   pJobBase = pJob->pBase;

   switch(pJobBase->eType)
   {
      case BVC5_JobType_eRender:
      /* Need to free bin memory if still allocated */
         BVC5_P_BinMemArrayDestroy(&pJob->jobData.sRender.sBinMemArray, hVC5->hBinPool);
         break;
      case BVC5_JobType_eFenceSignal:
         if (pJob->jobData.sSignal.signalData)
            BVC5_P_FenceSignalAndCleanup(hVC5->hFences, pJob->jobData.sSignal.signalData);
         break;
      case BVC5_JobType_eFenceWait:
         if (pJob->jobData.sWait.waitData)
            BVC5_P_FenceWaitAsyncCleanup(hVC5->hFences, pJob->jobData.sWait.waitData);
         break;
      default:
         break;
   }

   BKNI_Free(pJobBase);
   BVC5_P_FreeInternalJob(pJob);
}
