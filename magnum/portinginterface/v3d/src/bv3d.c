/***************************************************************************
 *     Copyright (C) 2012-16 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bv3d_worker_priv.h"
#include "bv3d_fence_priv.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#if (BCHP_CHIP == 7445)
#include "bchp_clkgen.h"
#endif

#include "bchp_v3d_dbg.h"
#include "bchp_v3d_ctl.h"
#include "bchp_v3d_qps.h"
#include "bchp_v3d_vpm.h"
#include "bchp_v3d_gca.h"
#include "bchp_v3d_pctr.h"

#define BV3D_P_MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define BV3D_P_MAX(X,Y) ((X) > (Y) ? (X) : (Y))

BDBG_MODULE(BV3D);

static void InterruptCallback_intctl_isr(void * pParm, int value)
{
   uint32_t flags;
#ifdef V3D_HANDLED_VIA_L2
   uint32_t intena;
#endif

   BV3D_Handle v3d = (BV3D_Handle)pParm;

   BSTD_UNUSED(value);

   if (!v3d->bScrubbing)
   {
      /* What Bin/Render interrupts have happened? */
      flags  = BREG_Read32_isr(v3d->hReg, BCHP_V3D_CTL_INTCTL);
      flags &= BREG_Read32_isr(v3d->hReg, BCHP_V3D_CTL_INTENA);
      flags &= BREG_Read32_isr(v3d->hReg, BCHP_V3D_CTL_INTCTL);

      /* Clear bin/render interrupts that we captured */
      if (flags)
      {
         BREG_Write32_isr(v3d->hReg, BCHP_V3D_CTL_INTCTL, flags);

         /* If it's OOM, then disable the interrupt until we have done something about it */
         if ((flags & BCHP_V3D_CTL_INTCTL_OFB_MASK) != 0)
         {
            BREG_Write32_isr(v3d->hReg, BCHP_V3D_CTL_INTDIS, BCHP_V3D_CTL_INTCTL_OFB_MASK);
         }

         BV3D_P_OnInterrupt_isr(v3d, flags);
      }

#ifdef V3D_HANDLED_VIA_L2
      /* L2 controller accepts a 'v3d_int_pulse'.  This means it need to go low->high to re-assert the IRQ.
         The v3d block contains lots of async blocks so this can't be guaranteed.

         To force the v3d IRQ low->high mask off IRQs and then put them back */
      intena = BREG_Read32_isr(v3d->hReg, BCHP_V3D_CTL_INTENA);
      BREG_Write32_isr(v3d->hReg, BCHP_V3D_CTL_INTDIS, -1);

      /* re-enable state */
      BREG_Write32_isr(v3d->hReg, BCHP_V3D_CTL_INTENA, intena);
#endif
   }
}

static void InterruptCallback_dbqitc_isr(void * pParm, int value)
{
   uint32_t dbqitc;

   BV3D_Handle v3d = (BV3D_Handle)pParm;

   BSTD_UNUSED(value);

   if (!v3d->bScrubbing && !v3d->bSecure)
   {
      /* What USR interrupts happened? */
      dbqitc = BREG_Read32_isr(v3d->hReg, BCHP_V3D_DBG_DBQITC);

      /* Clear the USR bits */
      if (dbqitc)
         BREG_Write32_isr(v3d->hReg, BCHP_V3D_DBG_DBQITC, dbqitc);

      /* Signal the interrupt to the worker thread */
      dbqitc = (dbqitc << 16);

      if (dbqitc != 0)
         BV3D_P_OnInterrupt_isr(v3d, dbqitc);
   }
}

#ifdef V3D_HANDLED_VIA_L2
static void InterruptCallback_isr(void * pParm, int value)
{
   InterruptCallback_intctl_isr(pParm, value);
   InterruptCallback_dbqitc_isr(pParm, value);
}
#endif


/***************************************************************************/
BERR_Code BV3D_Open(
   BV3D_Handle *pV3d,
   BCHP_Handle hChp,
   BREG_Handle hReg,
   BMMA_Heap_Handle hMma,
   uint64_t    uiHeapOffset,
   BMMA_Heap_Handle hMmaSecure,
   uint64_t    uiHeapOffsetSecure,
   BINT_Handle hInt,
   uint32_t    uiBinMemMegs,
   uint32_t    uiBinMemChunkPow,
   uint32_t    uiBinMemMegsSecure,
   void (*pfnSecureToggle)(bool),
   bool        bDisableAQA,
   uint32_t    uiClockFreq
)
{
   BERR_Code   err  = BERR_SUCCESS;
   BV3D_Handle hV3d = NULL;

   BDBG_ENTER(BV3D_Open);

   /* allocate device handle */
   hV3d = (BV3D_Handle)BKNI_Malloc(sizeof (BV3D_P_Handle));
   if (hV3d == NULL)
   {
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   /* init device handle structure to default value */
   BKNI_Memset((void *)hV3d, 0, sizeof (BV3D_P_Handle));

   /* save base modules handles for future use */
   hV3d->hChp              = hChp;
   hV3d->hReg              = hReg;
   hV3d->hInt              = hInt;
   hV3d->hMma              = hMma;
   hV3d->hMmaSecure        = hMmaSecure;
   hV3d->pfnSecureToggle   = pfnSecureToggle;

   hV3d->hFd   = -1;

   /* v3d only handles 32bit */
   hV3d->uiHeapOffset = (uint32_t)uiHeapOffset;
   hV3d->uiHeapOffsetSecure = (uint32_t)uiHeapOffsetSecure;

   hV3d->reallyPoweredOn = false;
   hV3d->isStandby = false;
   hV3d->bPerfMonitorActive = false;
   hV3d->bCollectLoadStats = false;

   hV3d->bDisableAQA = bDisableAQA;

#if (BCHP_CHIP == 7445)
   if (uiClockFreq == 0)
#if (BCHP_VER>=BCHP_VER_B0)
      /* E0 powers up with a lower default clock speed.  Restore to correct frequency */
      hV3d->uiMdiv = 0x8;
#else
      hV3d->uiMdiv = BCHP_CLKGEN_PLL_MOCA_PLL_CHANNEL_CTRL_CH_3_MDIV_CH3_DEFAULT;
#endif
   else
   {
      uint32_t div = 5;
      /* clamp to some sensible values */
      uiClockFreq = BV3D_P_MIN(BV3D_P_MAX(uiClockFreq, 257), 720);

      while (uiClockFreq < (3600 / div))
         div++;

      hV3d->uiMdiv = div;
      BKNI_Printf("V3D Clock frequency %d, MDIV %d\n", 3600 / hV3d->uiMdiv, hV3d->uiMdiv);
   }
#else
   BSTD_UNUSED(uiClockFreq);
   hV3d->uiMdiv = 0;
#endif

   err = BKNI_CreateMutex(&hV3d->hModuleMutex);
   if (err != BERR_SUCCESS)
      goto error;

   err = BV3D_P_IQMapCreate(&hV3d->hIQMap);
   if (err != BERR_SUCCESS)
      goto error;

   err = BV3D_P_WaitQCreate(&hV3d->hWaitQ);
   if (err != BERR_SUCCESS)
      goto error;

   err = BV3D_P_NotifyQCreate(&hV3d->hNotifyQ);
   if (err != BERR_SUCCESS)
      goto error;

   err = BV3D_P_CallbackMapCreate(&hV3d->hCallbackQ);
   if (err != BERR_SUCCESS)
      goto error;

   err = BV3D_P_BinMemCreate(&hV3d->hBinMemManager, hV3d->hMma, uiBinMemMegs, uiBinMemChunkPow);
   if (err != BERR_SUCCESS)
      goto error;

   if (hV3d->hMmaSecure)
   {
      err = BV3D_P_BinMemCreate(&hV3d->hBinMemManagerSecure, hV3d->hMmaSecure, uiBinMemMegsSecure, uiBinMemChunkPow);
      if (err != BERR_SUCCESS)
         goto error;
   }
   else
      hV3d->hBinMemManagerSecure = NULL;

   err = BV3D_P_FenceArrayCreate(&hV3d->hFences);
   if (err != BERR_SUCCESS)
      goto error;
   err = BV3D_P_OsInit(hV3d);
   if (err != BERR_SUCCESS)
      goto error;

   BV3D_P_ResetCore(hV3d, 0);

#ifdef V3D_HANDLED_VIA_L2
   /* In the case of L2, the IRQ handler calls both InterruptCallback_intctl & InterruptCallback_dbqitc
      as they are triggered from the same bit */
   BINT_CreateCallback(&hV3d->callback_intctl, hInt,
                       BCHP_INT_ID_V3D_INTR, InterruptCallback_isr, hV3d, 0);
   BINT_EnableCallback(hV3d->callback_intctl);
#else
   /* install an IRQ handler for the module */
   BINT_CreateCallback(&hV3d->callback_intctl, hInt,
                       BCHP_INT_ID_V3D_INTCTL_INTR, InterruptCallback_intctl_isr, hV3d, 0);
   BINT_CreateCallback(&hV3d->callback_dbqitc, hInt,
                       BCHP_INT_ID_V3D_DBQITC_INTR, InterruptCallback_dbqitc_isr, hV3d, 0);
#endif

   *pV3d = hV3d;

   BDBG_LEAVE(BV3D_Open);

   return BERR_SUCCESS;

error:
   BV3D_Close(hV3d);

   BDBG_LEAVE(BV3D_Close);

   return err;
}

/***************************************************************************/
BERR_Code BV3D_Close(
   BV3D_Handle hV3d)
{
   BDBG_ENTER(BV3D_Close);

   if (hV3d == NULL)
   {
      BDBG_LEAVE(BV3D_Close);
      return BERR_INVALID_PARAMETER;
   }

   if (hV3d->hIQMap != NULL)
   {
      /* check if clients are still registered */
      if (BV3D_P_IQMapSize(hV3d->hIQMap) > 0)
      {
         /* if so, remove all clients prior to termination */
         void *pNext;
         uint32_t uiPrev;
         uiPrev = BV3D_P_IQMapFirstKey(hV3d->hIQMap, &pNext);

         while (uiPrev != ~0u)
         {
            BV3D_UnregisterClient(hV3d, uiPrev);
            /* The unregister call will have removed the key, so you cannot call the iterator 'BV3D_P_IQMapNextKey()'.
               Instead, just re-get the first key */
            uiPrev = BV3D_P_IQMapFirstKey(hV3d->hIQMap, &pNext);
         }
      }
   }

   /* return to unsecure mode */
   if (hV3d->bSecure)
      BV3D_P_SwitchMode(hV3d, false);

   BV3D_P_ReallyPowerOff(hV3d);

   /* remove IRQ handler */
#ifdef V3D_HANDLED_VIA_L2
   if (hV3d->callback_intctl != NULL)
   {
      BINT_DisableCallback(hV3d->callback_intctl);
      BINT_DestroyCallback(hV3d->callback_intctl);
   }
#else
   if (hV3d->callback_intctl != NULL)
      BINT_DestroyCallback(hV3d->callback_intctl);

   if (hV3d->callback_dbqitc != NULL)
      BINT_DestroyCallback(hV3d->callback_dbqitc);
#endif

   if (hV3d->workerPresent)
   {
      BERR_Code rc;
      /* trigger inner first, then outer to mitigate any race condition */
      BKNI_SetEvent(hV3d->workerCanTerminate);
      /* wait until it has terminated */
      rc = BKNI_WaitForEvent(hV3d->workerSync, BKNI_INFINITE);
      if (rc) BERR_TRACE(rc);
   }

   if (hV3d->hIQMap != NULL)
      BV3D_P_IQMapDestroy(hV3d->hIQMap);

   if (hV3d->hWaitQ)
      BV3D_P_WaitQDestroy(hV3d->hWaitQ);

   if (hV3d->hNotifyQ)
      BV3D_P_NotifyQDestroy(hV3d->hNotifyQ);

   if (hV3d->hCallbackQ)
      BV3D_P_CallbackMapDestroy(hV3d->hCallbackQ);

   if (hV3d->hBinMemManager != NULL)
      BV3D_P_BinMemDestroy(hV3d->hBinMemManager);

   if (hV3d->hBinMemManagerSecure != NULL)
      BV3D_P_BinMemDestroy(hV3d->hBinMemManagerSecure);

   if (hV3d->hFences != NULL)
      BV3D_P_FenceArrayDestroy(hV3d->hFences);

   BDBG_ASSERT(hV3d->powerOnCount == 0);

   if (hV3d->hModuleMutex != NULL)
      BKNI_DestroyMutex(hV3d->hModuleMutex);
   BV3D_P_OsUninit(hV3d);
   BKNI_Free(hV3d);

   BDBG_LEAVE(BV3D_Close);

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_GetInfo(
   BV3D_Handle hV3d,
   BV3D_Info *pInfo
)
{
   uint32_t uiIdent0;
   uint32_t uiIdent1;
   uint32_t uiNumSlices;
   uint32_t uiTmusPerSlice;
   uint32_t uiTechRev;
   uint32_t uiRevision;
   BCHP_FeatureData featureData;

   BDBG_ENTER(BV3D_GetInfo);

   if ((pInfo == NULL) || (hV3d == NULL))
   {
      BDBG_LEAVE(BV3D_GetInfo);
      return BERR_INVALID_PARAMETER;
   }

   BKNI_AcquireMutex(hV3d->hModuleMutex);

   BV3D_P_PowerOn(hV3d);

   BKNI_Memset((void *)pInfo, 0, sizeof (BV3D_Info));

   uiIdent0         = BREG_Read32(hV3d->hReg, BCHP_V3D_CTL_IDENT0);
   uiIdent1         = BREG_Read32(hV3d->hReg, BCHP_V3D_CTL_IDENT1);
   uiNumSlices      = (uiIdent1 >> 4) & 0xF;
   uiTmusPerSlice   = (uiIdent1 >> 12) & 0xF;
   uiTechRev        = (uiIdent0 >> 24);
   uiRevision       = (uiIdent1 & 0xF);

   BCHP_GetFeature(hV3d->hChp, BCHP_Feature_eProductId, &featureData);
   if (featureData.data.productId == 0x7250)
      uiNumSlices--;

   BDBG_ASSERT(uiNumSlices <= 9);
   BDBG_ASSERT(uiTmusPerSlice <= 2);
   BDBG_ASSERT(uiTechRev >= 2);
   BDBG_ASSERT(uiRevision <= 9);

   pInfo->chName[0] = '4';
   pInfo->chName[1] = '0' + uiNumSlices;
   pInfo->chName[2] = '0' + 5 * (uiTmusPerSlice - 1);
   pInfo->chName[3] = '\0';

   pInfo->uiNumSlices            = uiNumSlices;
   pInfo->uiTextureUnitsPerSlice = uiTmusPerSlice;
   pInfo->uiTechRev              = uiTechRev;
   pInfo->uiRevision             = uiRevision;

   /* techRev  : 2=A, 3=B */
   /* revision : 0=0, 1=1, 2=2 */
   pInfo->chRevStr[0] = 'A' + uiTechRev - 2;
   pInfo->chRevStr[1] = '0' + uiRevision;
   pInfo->chRevStr[2] = '\0';

   BV3D_P_PowerOff(hV3d);

   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_GetInfo);

   return BERR_SUCCESS;
}

/* BV3D_P_GetFreeClientId
 *
 * Placeholder for generating client ids.
 */
static uint32_t BV3D_P_GetFreeClientId(BV3D_Handle hV3d)
{
   hV3d->uiNextClientId++;

   return hV3d->uiNextClientId;
}

/***************************************************************************/
BERR_Code BV3D_RegisterClient(
   BV3D_Handle hV3d,
   uint32_t    *puiClientId,
   void        *pContext,
   void (*cbFunc)(uint32_t, void *),
   uint32_t    uiClientPID
)
{
   BERR_Code      err;
   BV3D_IQHandle  instrQueue;
   uint32_t       uiClientId;

   BDBG_ENTER(BV3D_RegisterClient);

   if (hV3d == NULL)
   {
      BDBG_LEAVE(BV3D_RegisterClient);
      return BERR_INVALID_PARAMETER;
   }

   BKNI_AcquireMutex(hV3d->hModuleMutex);

   /* Create a new instruction queue for this client */
   err = BV3D_P_IQCreate(&instrQueue);
   if (err != BERR_SUCCESS)
      goto error0;

   uiClientId   = BV3D_P_GetFreeClientId(hV3d);
   *puiClientId = uiClientId;

   err = BV3D_P_IQMapInsert(hV3d->hIQMap, instrQueue, uiClientId);
   if (err != BERR_SUCCESS)
      goto error1;

   err = BV3D_P_CallbackMapInsert(hV3d->hCallbackQ, uiClientId, uiClientPID, pContext, cbFunc);
   if (err != BERR_SUCCESS)
      goto error2;

   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_RegisterClient);

   return BERR_SUCCESS;

error2:
   BV3D_P_IQMapRemove(hV3d->hIQMap, uiClientId);

error1:
   BV3D_P_IQDestroy(instrQueue);

error0:
   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_RegisterClient);

   return err;
}

/***************************************************************************/
BERR_Code BV3D_UnregisterClient(
   BV3D_Handle hV3d,
   uint32_t    uiClientId
   )
{
   BERR_Code      err = BERR_SUCCESS;
   BV3D_IQHandle  instrQueue;
   uint32_t       uiRemovedCount;

   BDBG_ENTER(BV3D_UnregisterClient);

   if (hV3d == NULL)
   {
      BDBG_LEAVE(BV3D_UnregisterClient);
      return BERR_INVALID_PARAMETER;
   }

   BKNI_AcquireMutex(hV3d->hModuleMutex);

   /* Get the queue from the clientId */
   instrQueue = BV3D_P_IQMapGet(hV3d->hIQMap, uiClientId);
   if (instrQueue != NULL)
   {
      BV3D_Instruction *psInstruction = NULL;
      uiRemovedCount = 0;

      /* remove all the elements from the instruction queue */
      while ((psInstruction = BV3D_P_IQPop(instrQueue)) != NULL)
      {
         /* any pending fences swept away require signalled */
         if (psInstruction->eOperation == BV3D_Operation_eNotifyInstr)
         {
            /* signal the fence if provided (probably Android only) */
            void *p = (void *)((uintptr_t)psInstruction->uiArg2);
            BV3D_P_FenceSignalAndCleanup(hV3d->hFences, p, BSTD_FUNCTION);
         }
         else if (psInstruction->eOperation == BV3D_Operation_eFenceInstr)
         {
            /* delete any waiter callbacks */
            void *p = (void *)((uintptr_t)psInstruction->uiArg1);
            BV3D_P_FenceFree(hV3d->hFences, p);
         }

         BV3D_P_InstructionDone(hV3d, psInstruction->psJob);
         uiRemovedCount += 1;
      }

      if (uiRemovedCount > 0)
         BDBG_MSG(("Stripped %d records from instruction queue", uiRemovedCount));
   }

   while ((hV3d->sBin.eOperation != BV3D_Operation_eEndInstr && hV3d->sBin.psJob->uiClientId == uiClientId) ||
          (hV3d->sRender.eOperation != BV3D_Operation_eEndInstr && hV3d->sRender.psJob->uiClientId == uiClientId))
   {
      /* We still have instructions active in hardware. Wait for them to complete. */
      BDBG_MSG(("DIED WITH JOB RUNNING"));
      BKNI_ReleaseMutex(hV3d->hModuleMutex);
      BKNI_Sleep(1);
      BKNI_AcquireMutex(hV3d->hModuleMutex);
   }

   if (instrQueue != NULL)
   {
      /* Remove it from the client list and free the resource */
      err = BV3D_P_IQMapRemove(hV3d->hIQMap, uiClientId);
      if (err == BERR_SUCCESS)
         err = BV3D_P_IQDestroy(instrQueue);
   }

   err = BV3D_P_CallbackMapRemove(hV3d->hCallbackQ, uiClientId);

   uiRemovedCount = 0;

   /* remove all the elements from the notify queue */
   while (BV3D_P_NotifyQPop(hV3d->hNotifyQ, uiClientId, NULL, NULL, NULL, NULL, NULL) != BERR_INVALID_PARAMETER)
      uiRemovedCount += 1;

   if (uiRemovedCount > 0)
      BDBG_MSG(("Stripped %d records from notify queue", uiRemovedCount));

   /* Release all remaining bin memory associated with this client */
   BV3D_P_BinMemReleaseByClient(hV3d->hBinMemManager, uiClientId);
   if (hV3d->hBinMemManagerSecure != NULL)
      BV3D_P_BinMemReleaseByClient(hV3d->hBinMemManagerSecure, uiClientId);

   /* Remove any pending fences */
   BV3D_P_FenceClientDestroy(hV3d->hFences, uiClientId);

   /* if this was the last removed client, make the device mode unsecure */
   if ((hV3d->bSecure) && (BV3D_P_IQMapSize(hV3d->hIQMap) == 0))
      BV3D_P_SwitchMode(hV3d, false);

   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_UnregisterClient);

   return err;
}

/***************************************************************************/
BERR_Code BV3D_GetNotification(
   BV3D_Handle          hV3d,
   uint32_t             clientId,
   BV3D_Notification    *notification
)
{
   BERR_Code err;

   BDBG_ENTER(BV3D_GetNotification);

   if ((notification == NULL) || (hV3d == NULL))
   {
      BDBG_LEAVE(BV3D_GetNotification);
      return BERR_INVALID_PARAMETER;
   }

   BKNI_AcquireMutex(hV3d->hModuleMutex);

   err = BV3D_P_NotifyQPop(hV3d->hNotifyQ,
                           clientId,
                           &notification->uiParam,
                           &notification->uiSync,
                           &notification->uiOutOfMemory,
                           &notification->uiJobSequence,
                           notification->pTimelineData);

   BDBG_MSG(("GetNotification(p=%d), client %d", notification->uiParam, clientId));

   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_GetNotification);

   return err;
}

/*************************************************************************
 * Called from the client to acknowledge a synchronous callback.
 * Synchronous callbacks will timeout after a short time to avoid deadlock.
 */
BERR_Code BV3D_SendSync(
   BV3D_Handle hV3d,
   uint32_t    clientId,
   bool        abandon
)
{
   BV3D_IQHandle     instrQueue;

   BDBG_ENTER(BV3D_SendSync);

   if (hV3d == NULL)
   {
      BDBG_LEAVE(BV3D_SendSync);
      return BERR_INVALID_PARAMETER;
   }

   BKNI_AcquireMutex(hV3d->hModuleMutex);

   /* Get the queue from the clientId */
   instrQueue = BV3D_P_IQMapGet(hV3d->hIQMap, clientId);
   if (instrQueue != NULL)
   {
      /* Handle synchronous callback acknowledgment */
      BV3D_Instruction *instr = BV3D_P_IQTop(instrQueue);

      BDBG_ASSERT(instr != NULL);
      BDBG_ASSERT(instr->eOperation == BV3D_Operation_eSyncInstr);

      /* Check that the instruction at the top of the queue is waiting for a sync callback */
      if ((instr != NULL) && (instr->eOperation == BV3D_Operation_eSyncInstr))
      {
         if (abandon)
            BV3D_P_AbandonJob(hV3d, instr);

         BV3D_P_IQSetWaiting(instrQueue, false);

         BV3D_P_Issue(hV3d);
      }
   }

   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_SendSync);

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_GetBinMemory(
   BV3D_Handle                   hV3d,
   const BV3D_BinMemorySettings  *settings,
   uint32_t                      uiClientId,
   BV3D_BinMemory                *pMemInfo
)
{
   BERR_Code         rc = BERR_SUCCESS;

   BSTD_UNUSED(settings);

   BDBG_ENTER(BV3D_GetBinMemory);

   if ((pMemInfo == NULL) || (hV3d == NULL))
   {
      BDBG_LEAVE(BV3D_GetBinMemory);
      return BERR_INVALID_PARAMETER;
   }

   BKNI_AcquireMutex(hV3d->hModuleMutex);
   {
      uint32_t                   uiSize;
      BV3D_BinMemHandle          hMem = NULL;
      BV3D_BinMemManagerHandle   hBinMemManager = settings->bSecure ? hV3d->hBinMemManagerSecure : hV3d->hBinMemManager;

      BDBG_ASSERT(hBinMemManager != NULL);

      /* Mustn't give away a block that could be used to replenish the overspill. */
      if (BV3D_P_BinMemEnoughFreeBlocks(hBinMemManager))
         hMem = BV3D_P_BinMemAlloc(hBinMemManager, uiClientId, &uiSize);

      if (hMem != NULL)
      {
         pMemInfo->uiAddress = (uint32_t)hMem;
         pMemInfo->uiSize    = uiSize;
      }
      else
      {
         pMemInfo->uiAddress = 0;
         pMemInfo->uiSize    = 0;
         rc                  = BERR_OUT_OF_SYSTEM_MEMORY;
      }
   }
   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_GetBinMemory);

   return rc;
}

BERR_Code BV3D_SetPerformanceMonitor(
   BV3D_Handle                    hV3d,
   const BV3D_PerfMonitorSettings *settings
)
{
   BDBG_ENTER(BV3D_SetPerformanceMonitor);

   if ((settings == NULL) || (hV3d == NULL))
   {
      BDBG_LEAVE(BV3D_SetPerformanceMonitor);
      return BERR_INVALID_PARAMETER;
   }

   BKNI_AcquireMutex(hV3d->hModuleMutex);

   if ((settings->uiFlags & BV3D_PerfMonitor_Reset) != 0)
   {
      /* Reset h/w and cumulative counters */
      BDBG_MSG(("Resetting performance counters"));
      BV3D_P_ResetPerfMonitorHWCounters(hV3d);
      BKNI_Memset(&hV3d->sPerfData, 0, sizeof(BV3D_PerfMonitorData));
   }

   if ((settings->uiFlags & BV3D_PerfMonitor_Stop) != 0)
   {
      /* Disable performance counters */
      BDBG_MSG(("Stopping performance counters"));
      hV3d->bPerfMonitorActive = false;
      BV3D_P_PowerOn(hV3d);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRE, 0);
      BV3D_P_PowerOff(hV3d);
   }

   if ((settings->uiFlags & BV3D_PerfMonitor_Start) != 0)
   {
      BDBG_MSG(("Starting performance counters"));

      hV3d->bPerfMonitorActive = true;
      hV3d->uiPerfMonitorHwBank = settings->uiHWBank;
      hV3d->uiPerfMonitorMemBank = settings->uiMemBank;

      BV3D_P_InitPerfMonitor(hV3d);
   }

   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_SetPerformanceMonitor);

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_GetPerformanceData(
   BV3D_Handle            hV3d,
   BV3D_PerfMonitorData   *data
)
{
   BDBG_ENTER(BV3D_GetPerformanceData);

   if (hV3d == NULL)
   {
      BDBG_LEAVE(BV3D_GetPerformanceData);
      return BERR_INVALID_PARAMETER;
   }

   BKNI_AcquireMutex(hV3d->hModuleMutex);

   BV3D_P_GatherPerfMonitor(hV3d);

   BDBG_ASSERT(data != NULL);
   *data = hV3d->sPerfData;

   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_GetPerformanceData);

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_Standby(
   BV3D_Handle             hV3d
)
{
   BDBG_ENTER(BV3D_Standby);

   if (hV3d == NULL)
   {
      BDBG_LEAVE(BV3D_Standby);
      return BERR_INVALID_PARAMETER;
   }

   /* wait until the core goes idle */
   while(1)
   {
      BKNI_AcquireMutex(hV3d->hModuleMutex);
      if (BV3D_P_AllUnitsIdle(hV3d))
         break;
      BKNI_ReleaseMutex(hV3d->hModuleMutex);
   }

   /* standby in unsecure mode */
   if (hV3d->bSecure)
      BV3D_P_SwitchMode(hV3d, false);

#ifdef BCHP_PWR_SUPPORT
   /* power off if the driver is really powered up but dont change the reallyPoweredOn state */
   if (hV3d->reallyPoweredOn)
   {
#ifdef V3D_HANDLED_VIA_L2
      BINT_DisableCallback(hV3d->callback_intctl);
#endif

      BDBG_MSG(("Power Off"));
      BCHP_PWR_ReleaseResource(hV3d->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D);
      BCHP_PWR_ReleaseResource(hV3d->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM);
   }
#endif

   hV3d->isStandby = true;

   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_Standby);

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_Resume(
   BV3D_Handle             hV3d
)
{
   BDBG_ENTER(BV3D_Resume);

   if (hV3d == NULL)
   {
      BDBG_LEAVE(BV3D_Resume);
      return BERR_INVALID_PARAMETER;
   }

   BKNI_AcquireMutex(hV3d->hModuleMutex);

#ifdef BCHP_PWR_SUPPORT
   /* power on if it was powered on when entering S3 state. */
   if (hV3d->reallyPoweredOn)
   {
      BDBG_MSG(("Power On"));
      BCHP_PWR_AcquireResource(hV3d->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D);
      BCHP_PWR_AcquireResource(hV3d->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM);

#ifdef V3D_HANDLED_VIA_L2
      BINT_EnableCallback(hV3d->callback_intctl);
#endif
   }
#endif

   hV3d->isStandby = false;

   BV3D_P_ResetCore(hV3d, hV3d->uiUserVPM);

   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_Resume);

   return BERR_SUCCESS;
}

BERR_Code BV3D_SetGatherLoadData(
   BV3D_Handle          hV3d,            /* [in] Handle to V3D module. */
   bool                 bCollect         /* [in] True to turn on load data collection, false to turn off */
)
{
   BDBG_ENTER(BV3D_GatherLoadData);

   if (hV3d == NULL)
   {
      BDBG_LEAVE(BV3D_GatherLoadData);
      return BERR_INVALID_PARAMETER;
   }

   BKNI_AcquireMutex(hV3d->hModuleMutex);

   hV3d->bCollectLoadStats = bCollect;

   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_GatherLoadData);

   return BERR_SUCCESS;
}

BERR_Code BV3D_GetLoadData(
   BV3D_Handle          hV3d,              /* [in] Handle to V3D module. */
   BV3D_ClientLoadData  *pLoadData,        /* [out] Array of client loading data */
   uint32_t             uiNumClients,      /* [in] How many client slots are available in the pData array */
   uint32_t             *pValidClients     /* [out] The actual number of valid clients written to pData */
)
{
   BERR_Code ret = BERR_SUCCESS;

   BDBG_ENTER(BV3D_GetLoadData);

   if (hV3d == NULL || pValidClients == NULL)
   {
      BDBG_LEAVE(BV3D_GetLoadData);
      return BERR_INVALID_PARAMETER;
   }

   BKNI_AcquireMutex(hV3d->hModuleMutex);

   if (hV3d->bCollectLoadStats)
   {
      if (pLoadData != NULL && uiNumClients > 0)
      {
         BV3D_P_CallbackMapGetStats(hV3d->hCallbackQ, &hV3d->sRender, pLoadData, uiNumClients, pValidClients, true);
      }
      else
      {
         BV3D_P_CallbackMapGetSize(hV3d->hCallbackQ, pValidClients);
      }
   }
   else
   {
      *pValidClients = 0;
      ret = BERR_NOT_SUPPORTED;
   }

   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_GetLoadData);

   return ret;
}

/***************************************************************************/
BERR_Code BV3D_FenceOpen(
   BV3D_Handle          hV3d,              /* [in] Handle to V3D module. */
   uint32_t             uiClientId,
   int                 *pFence,
   void               **dataToSignal,
   char                 cType,
   int                  iPid
)
{
   BDBG_ENTER(BV3D_FenceOpen);

   if (hV3d == NULL)
      return BERR_INVALID_PARAMETER;

   *pFence = BV3D_P_FenceOpen(hV3d->hFences, uiClientId, dataToSignal, cType, iPid);

   BDBG_MSG(("BV3D_FenceOpen fence %d", *pFence));

   BDBG_LEAVE(BV3D_FenceOpen);

   return *pFence != -1 ? BERR_SUCCESS : BERR_OUT_OF_SYSTEM_MEMORY;
}

/***************************************************************************/
BERR_Code BV3D_FenceWaitAsync(
   BV3D_Handle          hV3d,
   uint32_t             uiClientId,
   int                  fd,
   void               **pV3dFFence
)
{
   BERR_Code err;
   BDBG_ENTER(BV3D_FenceWaitAsync);

   err = BV3D_P_FenceWaitAsync(hV3d, uiClientId, fd, pV3dFFence);

   BDBG_LEAVE(BV3D_FenceWaitAsync);

   return err;
}

/***************************************************************************/
/* BV3D_GetTime
 */
BERR_Code BV3D_GetTime(uint64_t *pMicroseconds)
{
   BERR_Code err;

   BDBG_ENTER(BV3D_GetTime);

   if (pMicroseconds == NULL)
      err = BERR_INVALID_PARAMETER;
   else
      err = BV3D_P_GetTime_isrsafe(pMicroseconds);

   BDBG_LEAVE(BV3D_GetTime);

   return err;
}

/* End of File */
