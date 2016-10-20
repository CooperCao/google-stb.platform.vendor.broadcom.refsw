/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"


#ifndef BSAT_EXCLUDE_TFEC

BDBG_MODULE(bsat_g1_priv_tfec);

#define BSAT_DEBUG_TFEC(x) /* x */
/* #define BSAT_DEBUG_ACQ_TIME */

/* local functions */
bool BSAT_g1_P_TfecScanTryNextMode_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_TfecConfigCl_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_TfecConfigEq_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_TfecOnHpLock_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_TfecSetOpll_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_TfecConfig_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_TfecRun_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_TfecUpdateBlockCount_isrsafe(BSAT_ChannelHandle h);
#ifndef BSAT_HAS_DUAL_TFEC
BERR_Code BSAT_g1_P_TfecAcquire2_isr(BSAT_ChannelHandle h);
#endif
#ifdef BSAT_HAS_DUAL_TFEC
BSAT_ChannelHandle BSAT_g1_P_TfecGetOtherChannelHandle_isrsafe(BSAT_ChannelHandle h);
#endif

#ifdef BSAT_DEBUG_ACQ_TIME
static uint32_t t0, t1, max_t = 0;
static BSAT_Mode lastMode = 0;
#endif


/******************************************************************************
 BSAT_g1_P_TfecAcquire_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecAcquire_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val;

   if (hChn->bHasTfec == false)
   {
      turbo_acquire_failed:
      if (hChn->bBlindScan)
      {
         hChn->reacqCause = BSAT_ReacqCause_eNoTfec;
         return BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);
      }
      else
      {
         hChn->reacqCause = BSAT_ReacqCause_eNoTfec;
         return BERR_NOT_AVAILABLE;
      }
   }

   if (hChn->acqSettings.mode == BSAT_Mode_eTurbo_scan)
   {
      if (BSAT_g1_P_TfecScanTryNextMode_isr(h) == false)
         goto turbo_acquire_failed;
   }
   else
      hChn->turboScanState = 0;

#ifndef BSAT_EXCLUDE_AFEC
   hChn->dvbs2ScanState = 0;
#endif

   /* dynamically turn on/off TFEC clock as needed */
   BSAT_CHK_RETCODE(BSAT_g1_P_TfecPowerUp_isr(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_GetTfecClock_isrsafe(h, &(hChn->fecFreq)));
   BSAT_CHK_RETCODE(BSAT_g1_P_NonLegacyModeAcquireInit_isr(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_TfecConfigCl_isr(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_TfecConfigEq_isr(h));
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 1); /* reset the eq */

   val = 0x00000904;
   if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
      val |= 0x00010000;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VLCTL, val);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HD8PSK1, 0x01D901D9);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HD8PSK2, 0x00C400C4);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HDQPSK, 0x01000000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_XTAP1, 0x00000100);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_XTAP2, 0x00805000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_PILOTCTL, 0x00000004);
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, ~0x60); /* for now, disable soft pd tables */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VCOS, 0x76303000);

   BSAT_CHK_RETCODE(BSAT_g1_P_ConfigPlc_isr(h, true)); /* set acquisition plc */

   /* Chan: before HP lock */
   if (hDev->sdsRevId < 0x74)
   {
      /* BCM45308-A0 and earlier */
      if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
         val = 0x00005088; /* leak=0x50, dafe_int_scale=8(2^-6), dafe_lin_scale=4(2^-6) */
      else
         val = 0x0000406A; /* leak=0x40, dafe_int_scale=6(2^-10), dafe_lin_scale=5(2^-4) */
   }
   else
   {
      /* BCM45308-B0 and later */
      if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
         val = 0x04805088; /* leak=0x50, dafe_int_scale=8(2^-6), dafe_lin_scale=4(2^-6) */
      else
         val = 0x0560406A; /* leak=0x40, dafe_int_scale=6(2^-10), dafe_lin_scale=5(2^-4) */
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, val);

   /* configure and run the HP */
   retCode = BSAT_g1_P_HpAcquire_isr(h, BSAT_g1_P_TfecOnHpLock_isr);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_TfecEnableLockInterrupts_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecEnableLockInterrupts_isr(BSAT_ChannelHandle h, bool bEnable)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (bEnable)
   {
      BINT_EnableCallback_isr(hChn->hTfecLockCb);
      BINT_EnableCallback_isr(hChn->hTfecNotLockCb);
   }
   else
   {
      BINT_DisableCallback_isr(hChn->hTfecLockCb);
      BINT_DisableCallback_isr(hChn->hTfecNotLockCb);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TfecEnableSyncInterrupt_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecEnableSyncInterrupt_isr(BSAT_ChannelHandle h, bool bEnable)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (bEnable)
      return BINT_EnableCallback_isr(hChn->hTfecSyncCb);
   else
      return BINT_DisableCallback_isr(hChn->hTfecSyncCb);
}


/******************************************************************************
 BSAT_g1_P_TfecOnHpLock_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecOnHpLock_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t val;
   BERR_Code retCode;

   if (hChn->acqSettings.mode == BSAT_Mode_eTurbo_scan)
   {
      hChn->turboScanState |= BSAT_TURBO_SCAN_STATE_HP_LOCKED;
      if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
         hChn->turboScanState |= BSAT_TURBO_SCAN_STATE_8PSK_HP_LOCKED;
      else
         hChn->turboScanState &= ~BSAT_TURBO_SCAN_STATE_8PSK_HP_LOCKED;
   }

   if (hDev->sdsRevId < 0x74)
   {
      /* BCM45308-A0 and earlier */
      if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
         val = 0x0000408A; /* dafe_int_scale=8(2^-6) */
      else
         val = 0x0000406A; /* dafe_int_scale=6(2^-10) */
   }
   else
   {
      /* BCM45308-B0 and later */
      if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
         val = 0x0580408A; /* dafe_int_scale=8(2^-6), dafe_lin_scale=5 */
      else
         val = 0x0560406A; /* dafe_int_scale=6(2^-10), dafe_lin_scale=5 */
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, val);

   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, ~BCHP_SDS_EQ_0_EQMISCCTL_cma_en_MASK);           /* disable CMA */

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_main_mu, 6);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_mu, 6);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_mu_delta_en, 0);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_frz, 0);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_mainq_frz, 1);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_maini_frz, 0);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, val);

   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, ~BCHP_SDS_CL_0_CLCTL1_clen_MASK); /* disable front carrier loop */
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, BCHP_SDS_CL_0_CLCTL2_fclfrz_MASK); /* freeze front carrier loop */

   if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
      val = 0x99;
   else
      val = 0x8A;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNORECTL, val);

   BSAT_g1_P_InitBert_isr(h);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BEM1, 0x04576231);

   if (hChn->xportSettings.bOpllBypass == false)
      BSAT_g1_P_PowerUpOpll_isr(h);
   BSAT_CHK_RETCODE(BSAT_g1_P_TfecSetOpll_isr(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_ConfigOif_isr(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_SetAgcTrackingBw_isr(h));

   /* clear the TFEC interrupts */
   BSAT_CHK_RETCODE(BSAT_g1_P_TfecEnableSyncInterrupt_isr(h, false));
   BINT_ClearCallback_isr(hChn->hTfecSyncCb);
   BINT_ClearCallback_isr(hChn->hTfecLockCb);
   BINT_ClearCallback_isr(hChn->hTfecNotLockCb);

   /* set up the TFEC */
   BSAT_CHK_RETCODE(BSAT_g1_P_TfecConfig_isr(h));

   /* set tracking baud loop bw */
   BSAT_CHK_RETCODE(BSAT_g1_P_SetBaudBw_isr(h, hChn->acqSettings.symbolRate / 400, 4));

   if (hDev->sdsRevId < 0x74)
   {
      /* BCM45308-A0 and earlier */
      if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
         val = 0x00004066; /* dafe_int_scale=6(2^-10) */
      else
         val = 0x00003063; /* dafe_int_scale=6(2^-10), dafe_lin_scale=1 */
   }
   else
   {
      /* BCM45308-B0 and later */
      if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
         val = 0x03604066; /* dafe_int_scale=6(2^-10), dafe_lin_scale=3 */
      else
         val = 0x01603063; /* dafe_int_scale=6(2^-10), dafe_lin_scale=1 */
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, val);

   /* set tracking PLC lock */
   if (hChn->bPlcTracking == false)
      BSAT_g1_P_ConfigPlc_isr(h, false); /* set tracking PLC */

#ifdef BSAT_HAS_DUAL_TFEC
   BSAT_CHK_RETCODE(BSAT_g1_P_TfecEnableSyncInterrupt_isr(h, true));
   retCode = BSAT_g1_P_TfecRun_isr(h);
#else
   if (hChn->actualMode == BSAT_Mode_eTurbo_Qpsk_1_2)
      val = 400000;
   else
      val = 300000;
   retCode = BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaud, val, BSAT_g1_P_TfecAcquire2_isr);
#endif

   done:
   return retCode;
}


#ifndef BSAT_HAS_DUAL_TFEC
/******************************************************************************
 BSAT_g1_P_TfecAcquire2_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecAcquire2_isr(BSAT_ChannelHandle h)
{
   BERR_Code retCode;

   /* wait for sync with timeout */
   BSAT_CHK_RETCODE(BSAT_g1_P_TfecEnableSyncInterrupt_isr(h, true));

   retCode = BSAT_g1_P_TfecRun_isr(h);

   done:
   return retCode;
}
#endif


/******************************************************************************
 BSAT_g1_P_TfecIsValidMode_isr()
******************************************************************************/
bool BSAT_g1_P_TfecIsValidMode_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t i;
   uint16_t mask;

   if (BSAT_MODE_IS_TURBO(hChn->actualMode))
   {
      if (hChn->actualMode == BSAT_Mode_eTurbo_scan)
         goto invalid_condition;

      i = hChn->actualMode - BSAT_Mode_eTurbo_Qpsk_1_2;
      mask = (uint16_t)(1 << i);
      if ((mask & hChn->turboSettings.scanModes) == 0)
         return false;
   }
   else
   {
      invalid_condition:
      BDBG_ERR(("BSAT_g1_P_TurboCheckMode() - invalid condition"));
      return false;
   }

   return true;
}


/******************************************************************************
 BSAT_g1_P_TfecOnLock_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecOnLock_isr(BSAT_ChannelHandle h)
{
   uint32_t hpoverride;
   BSTD_UNUSED(h);

   BSAT_DEBUG_TFEC(BDBG_WRN(("TFEC locked")));

   hpoverride = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE);
   BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, CLFFRZOV, 1);
   BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, CLFENOV, 1);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE, hpoverride);

   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, BCHP_SDS_CL_0_CLCTL1_clen_MASK); /* enable front carrier loop */
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, ~BCHP_SDS_CL_0_CLCTL1_flfsel_MASK); /* use front phase detector */
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, BCHP_SDS_CL_0_CLCTL2_fclfrz_MASK); /* freeze front carrier loop */

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TfecOnLostLock_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecOnLostLock_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BSAT_CHK_RETCODE(BSAT_g1_P_TfecUpdateBlockCount_isrsafe(h));

   if (BSAT_g1_P_IsHpLocked_isr(h) == false)
   {
      hChn->reacqCause = BSAT_ReacqCause_eHpLostLock;
      hChn->bForceReacq = true;
   }
   else
   {
      BSAT_DEBUG_TFEC(BDBG_WRN(("TFEC lost lock")));
   }

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_TfecOnStableLock_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecOnStableLock_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   if (hChn->bEverStableLock == false)
   {
      BSAT_CHK_RETCODE(BSAT_g1_P_AfecResetBlockCount_isrsafe(h));
   }

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_TfecOnMonitorLock_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecOnMonitorLock_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (BSAT_g1_P_IsHpLocked_isr(h) == false)
   {
      BDBG_MSG(("HP fell out of lock"));
      hChn->bForceReacq = true;
      hChn->reacqCause = BSAT_ReacqCause_eHpLostLock;
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TfecSync_isr() - callback routine for TFEC sync interrupt
******************************************************************************/
void BSAT_g1_P_TfecSync_isr(void *p, int int_id)
{
   BSAT_ChannelHandle h = (BSAT_ChannelHandle)p;
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BSTD_UNUSED(int_id);

   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eBaud); /* disable sync timeout */
   BSAT_g1_P_TfecEnableSyncInterrupt_isr(h, false);

#ifdef BSAT_DEBUG_ACQ_TIME
BSAT_g1_P_GetAcquisitionTimerValue_isr(h, &t1);
if (lastMode == hChn->actualMode)
{
   if ((t1-t0)>max_t)
      max_t = t1-t0;
}
else
   max_t = t1-t0;
lastMode = hChn->actualMode;
BDBG_ERR(("TfecSync: t=%d, max_t=%d, actualMode=0x%X", t1-t0, max_t, hChn->actualMode));
#endif

   hChn->turboScanLockedMode = hChn->turboScanCurrMode;
   hChn->turboScanLockedModeFailures = 0;
   hChn->turboScanState |= BSAT_TURBO_SCAN_STATE_SYNC_ACQUIRED;
   BSAT_g1_P_StartTracking_isr(h);
}


/******************************************************************************
 BSAT_g1_P_TfecGetStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecGetStatus(BSAT_ChannelHandle h, BSAT_TurboStatus *pStatus)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BKNI_EnterCriticalSection();
   retCode = BSAT_g1_P_TfecUpdateBlockCount_isrsafe(h);
   BKNI_LeaveCriticalSection();

   if (retCode == BERR_SUCCESS)
   {
      pStatus->bValid = true;
      pStatus->totalBlocks = hChn->totalBlocks;
      pStatus->corrBlocks = hChn->corrBlocks;
      pStatus->badBlocks = hChn->badBlocks;
      pStatus->fecFreq = hChn->fecFreq;
   }
   else
      pStatus->bValid = false;
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_TfecResetBlockCount_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecResetBlockCount_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (BSAT_g1_P_IsTfecOn_isrsafe(h))
   {
      BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_TFEC_TFECTL, 0x06);
      hChn->totalBlocks = 0;
      hChn->corrBlocks = 0;
      hChn->badBlocks = 0;
      return BERR_SUCCESS;
   }
   return BSAT_ERR_POWERED_DOWN;
}


/******************************************************************************
 BSAT_g1_P_TfecUpdateBlockCount_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecUpdateBlockCount_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   if (BSAT_g1_P_IsTfecOn_isrsafe(h))
   {
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_TFEC_TNBLK);
      hChn->totalBlocks += val;

      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_TFEC_TCBLK);
      hChn->corrBlocks += val;

      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_TFEC_TBBLK);
      hChn->badBlocks += val;

      /* reset the FEC error counters */
      BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_TFEC_TFECTL, 0x06);
      return BERR_SUCCESS;
   }
   return BSAT_ERR_POWERED_DOWN;
}


/******************************************************************************
 BSAT_g1_P_TfecScanTryNextMode_isr()
******************************************************************************/
bool BSAT_g1_P_TfecScanTryNextMode_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   bool bIsTurbo8psk;

   hChn->actualMode = BSAT_Mode_eUnknown;

   if (hChn->turboScanState & BSAT_TURBO_SCAN_STATE_SYNC_ACQUIRED)
   {
      if (hChn->turboScanLockedModeFailures > 3)
      {
         hChn->turboScanState &= ~BSAT_TURBO_SCAN_STATE_SYNC_ACQUIRED;
         hChn->turboScanLockedModeFailures = 0;
      }
      else
      {
         if (!hChn->bBlindScan && ((hChn->turboScanState & BSAT_TURBO_SCAN_STATE_FIRST_PASS) == 0))
            BSAT_g1_P_IncrementReacqCount_isr(h);

         hChn->turboScanCurrMode = hChn->turboScanLockedMode;
         goto set_actual_mode;
      }
   }

   next_mode:
   if (hChn->turboSettings.scanModes == 0)
      return false;

   if (hChn->turboScanCurrMode == 0)
   {
      hChn->turboScanCurrMode = 1;
      hChn->turboScanState &= BSAT_TURBO_SCAN_STATE_FIRST_PASS;
      if (!hChn->bBlindScan && ((hChn->turboScanState & BSAT_TURBO_SCAN_STATE_FIRST_PASS) == 0))
         BSAT_g1_P_IncrementReacqCount_isr(h);
   }
   else
   {
      if ((hChn->turboScanState & BSAT_TURBO_SCAN_STATE_FIRST_ACQ) == 0)
         hChn->turboScanCurrMode = (hChn->turboScanCurrMode << 1) & BSAT_SCAN_MODE_TURBO_ALL;

      if (hChn->turboScanCurrMode == 0)
      {
         hChn->turboScanState &= ~BSAT_TURBO_SCAN_STATE_FIRST_PASS;
         if (hChn->bBlindScan)
            return false;
      }
   }

   hChn->turboScanState &= ~BSAT_TURBO_SCAN_STATE_FIRST_ACQ;

   set_actual_mode:
   if (hChn->turboScanCurrMode & hChn->turboSettings.scanModes)
   {
      if (hChn->turboScanCurrMode & BSAT_SCAN_MODE_TURBO_QPSK_1_2)
         hChn->actualMode = BSAT_Mode_eTurbo_Qpsk_1_2;
      else if (hChn->turboScanCurrMode & BSAT_SCAN_MODE_TURBO_QPSK_2_3)
         hChn->actualMode = BSAT_Mode_eTurbo_Qpsk_2_3;
      else if (hChn->turboScanCurrMode & BSAT_SCAN_MODE_TURBO_QPSK_3_4)
         hChn->actualMode = BSAT_Mode_eTurbo_Qpsk_3_4;
      else if (hChn->turboScanCurrMode & BSAT_SCAN_MODE_TURBO_QPSK_5_6)
         hChn->actualMode = BSAT_Mode_eTurbo_Qpsk_5_6;
      else if (hChn->turboScanCurrMode & BSAT_SCAN_MODE_TURBO_QPSK_7_8)
         hChn->actualMode = BSAT_Mode_eTurbo_Qpsk_7_8;
      else if (hChn->turboScanCurrMode & BSAT_SCAN_MODE_TURBO_8PSK_2_3)
         hChn->actualMode = BSAT_Mode_eTurbo_8psk_2_3;
      else if (hChn->turboScanCurrMode & BSAT_SCAN_MODE_TURBO_8PSK_3_4)
         hChn->actualMode = BSAT_Mode_eTurbo_8psk_3_4;
      else if (hChn->turboScanCurrMode & BSAT_SCAN_MODE_TURBO_8PSK_4_5)
         hChn->actualMode = BSAT_Mode_eTurbo_8psk_4_5;
      else if (hChn->turboScanCurrMode & BSAT_SCAN_MODE_TURBO_8PSK_5_6)
         hChn->actualMode = BSAT_Mode_eTurbo_8psk_5_6;
      else /* (hChn->turboScanCurrMode & BSAT_SCAN_MODE_TURBO_8PSK_8_9) */
         hChn->actualMode = BSAT_Mode_eTurbo_8psk_8_9;

      if (hChn->turboScanState & BSAT_TURBO_SCAN_STATE_HP_INIT)
      {
         bIsTurbo8psk = BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode) ? true : false;
         if (hChn->turboScanState & BSAT_TURBO_SCAN_STATE_HP_LOCKED)
         {
            if (bIsTurbo8psk)
            {
               if ((hChn->turboScanState & BSAT_TURBO_SCAN_STATE_8PSK_HP_LOCKED) == 0)
                  goto next_mode;
            }
            else if (hChn->turboScanState & BSAT_TURBO_SCAN_STATE_8PSK_HP_LOCKED)
               goto next_mode;
         }
         else if (hChn->turboScanState & BSAT_TURBO_SCAN_STATE_8PSK_FAILED)
         {
            /* only consider qpsk */
            if (bIsTurbo8psk)
               goto next_mode;
         }
         else
         {
            /* only consider 8psk */
            if (!bIsTurbo8psk)
               goto next_mode;
         }
      }

      switch (hChn->actualMode)
      {
         case BSAT_Mode_eTurbo_Qpsk_1_2:
            BDBG_MSG(("trying BSAT_Mode_eTurbo_Qpsk_1_2"));
            break;
         case BSAT_Mode_eTurbo_Qpsk_2_3:
            BDBG_MSG(("trying BSAT_Mode_eTurbo_Qpsk_2_3"));
            break;
         case BSAT_Mode_eTurbo_Qpsk_3_4:
            BDBG_MSG(("trying BSAT_Mode_eTurbo_Qpsk_3_4"));
            break;
         case BSAT_Mode_eTurbo_Qpsk_5_6:
            BDBG_MSG(("trying BSAT_Mode_eTurbo_Qpsk_5_6"));
            break;
         case BSAT_Mode_eTurbo_Qpsk_7_8:
            BDBG_MSG(("trying BSAT_Mode_eTurbo_Qpsk_7_8"));
            break;
         case BSAT_Mode_eTurbo_8psk_2_3:
            BDBG_MSG(("trying BSAT_Mode_eTurbo_8psk_2_3"));
            break;
         case BSAT_Mode_eTurbo_8psk_3_4:
            BDBG_MSG(("trying BSAT_Mode_eTurbo_8psk_3_4"));
            break;
         case BSAT_Mode_eTurbo_8psk_4_5:
            BDBG_MSG(("trying BSAT_Mode_eTurbo_8psk_4_5"));
            break;
         case BSAT_Mode_eTurbo_8psk_5_6:
            BDBG_MSG(("trying BSAT_Mode_eTurbo_8psk_5_6"));
            break;
         case BSAT_Mode_eTurbo_8psk_8_9:
            BDBG_MSG(("trying BSAT_Mode_eTurbo_8psk_8_9"));
            break;
         default:
            BDBG_ERR(("BSAT_g1_P_TurboScanTryNextMode(): should not get here!"));
            return false;
      }
   }
   else
      goto next_mode;

   return true;
}


/******************************************************************************
 BSAT_g1_P_TfecConfigCl_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecConfigCl_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1);
   BCHP_SET_FIELD_DATA(val, SDS_CL_0_CLCTL1, front_in_sel, 0);
   BCHP_SET_FIELD_DATA(val, SDS_CL_0_CLCTL1, updqamff, 0);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, val);

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2);
   val &= ~0xFF000000;
   if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
      val |= 0xEE000000;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, val);

   if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLFFCTL, ~BCHP_SDS_CL_0_CLFFCTL_fine_mix_byp_MASK);
   else
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLFFCTL, BCHP_SDS_CL_0_CLFFCTL_fine_mix_byp_MASK);

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLFBCTL);
   BCHP_SET_FIELD_DATA(val, SDS_CL_0_CLFBCTL, fb_err_mode, 0);
   BCHP_SET_FIELD_DATA(val, SDS_CL_0_CLFBCTL, fw_rst_mode, 0);
   BCHP_SET_FIELD_DATA(val, SDS_CL_0_CLFBCTL, bw_rst_mode, 0);
   BCHP_SET_FIELD_DATA(val, SDS_CL_0_CLFBCTL, fb_mode, 0);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLFBCTL, val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TfecConfigEq_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecConfigEq_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, i, main_tap_idx = 12;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQMISCCTL, ffe_mu_delta, 2);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQMISCCTL, dvbs2_8psk_mapping, 0);

   if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
   {
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQMISCCTL, err_mode, 2);
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQMISCCTL, sym_mode, 2);
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQMISCCTL, ext_en, 1);
   }
   else
   {
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQMISCCTL, err_mode, 0);
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQMISCCTL, sym_mode, 0);
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQMISCCTL, ext_en, 0);
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, val);

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_main_tap, main_tap_idx);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_update_rate, 1);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_reset, 0);
   if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
   {
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_main_mu, 6);
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_mu, 6);
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_update_mode, 1);
   }
   else
   {
      /* Turbo QPSK */
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_main_mu, 3);
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_mu, 3);
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_update_mode, 2);
   }
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_frz, 0);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_maini_frz, 0);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_mainq_frz, 1);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, val);

   for (i = 0; i < 24; i++)
   {
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQCFAD);
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQCFAD, coeff_addr, i);
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQCFAD, coeff_rw_en, 1);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQCFAD, val);

      val = 0;
      if (i == main_tap_idx)
      {
         if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
         {
#ifdef BSAT_HAS_WFE
            BCHP_SET_FIELD_DATA(val, SDS_EQ_0_F0B, coeff_i, 0x3800); /* val = 0x38000000 */
#else
            BCHP_SET_FIELD_DATA(val, SDS_EQ_0_F0B, coeff_i, 0x3900); /* val = 0x39000000 */
#endif
         }
         else
         {
#ifdef BSAT_HAS_WFE
            BCHP_SET_FIELD_DATA(val, SDS_EQ_0_F0B, coeff_i, 0x2500); /* val = 0x25000000 */
#else
            BCHP_SET_FIELD_DATA(val, SDS_EQ_0_F0B, coeff_i, 0x2860); /* val = 0x28600000 */
#endif
         }
      }
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_F0B, val);
   }

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQCFAD, 0);

   val = BCHP_FIELD_DATA(SDS_EQ_0_PLDCTL, pl_des_byp, 1);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_PLDCTL, pl_res_byp, 1);
   if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
   {
#ifdef BSAT_HAS_WFE
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_PLDCTL, hp_in_scale, 0x35); /* threshold=0x4B */
#else
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_PLDCTL, hp_in_scale, 0x67); /* val = 0x670A */
#endif
   }
   else
   {
      /* Turbo QPSK */
#ifdef BSAT_HAS_WFE
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_PLDCTL, hp_in_scale, 0x3E); /* threshold=0x58 */
#else
      BCHP_SET_FIELD_DATA(val, SDS_EQ_0_PLDCTL, hp_in_scale, 0x75); /* val = 0x750A */
#endif
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_PLDCTL, val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TfecSetOpll_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecSetOpll_isr(BSAT_ChannelHandle h)
{
   static const uint32_t turbo_opll_N[10] =
   {
      38352000, /* turbo qpsk 1/2 */
      51286400, /* turbo qpsk 2/3 */
      57603200, /* turbo qpsk 3/4 */
      64070400, /* turbo qpsk 5/6 */
      67228800, /* turbo qpsk 7/8 */
      77004800, /* turbo 8psk 2/3 */
      82419200, /* turbo 8psk 3/4 */
      84675200, /* turbo 8psk 4/5 */
      92345600, /* turbo 8psk 5/6 */
      96256000  /* turbo 8psk 8/9 */
   };


   static const uint32_t turbo_opll_D[10] =
   {
      161699648, /* turbo qpsk 1/2 */
      161699648, /* turbo qpsk 2/3 */
      161699648, /* turbo qpsk 3/4 */
      161699648, /* turbo qpsk 5/6 */
      161699648, /* turbo qpsk 7/8 */
      160703040, /* turbo 8psk 2/3 */
      160703040, /* turbo 8psk 3/4 */
      160703040, /* turbo 8psk 4/5 */
      160703040, /* turbo 8psk 5/6 */
      160703040  /* turbo 8psk 8/9 */
   };

   static const uint16_t turbo_number_of_bits_in_block[10] =
   {
      2550, /* turbo qpsk 1/2 */
      3410, /* turbo qpsk 2/3 */
      3830, /* turbo qpsk 3/4 */
      4260, /* turbo qpsk 5/6 */
      4470, /* turbo qpsk 7/8 */
      5120, /* turbo 8psk 2/3 */
      5480, /* turbo 8psk 3/4 */
      5630, /* turbo 8psk 4/5 */
      6140, /* turbo 8psk 5/6 */
      6400  /* turbo 8psk 8/9 */
   };

   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, i, P_hi, P_lo, Q_hi, Q_lo;

   i = hChn->actualMode - BSAT_Mode_eTurbo_Qpsk_1_2;
   hChn->opll_N = turbo_opll_N[i];
   hChn->opll_D = turbo_opll_D[i];

   /* val = HP header symbol */
   if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
      val = 64;
   else
      val = 128;

   BMTH_HILO_32TO64_Mul(hChn->acqSettings.symbolRate, turbo_number_of_bits_in_block[i] * 3760 * 4 * 2, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, (val + 10256) * 3893, &Q_hi, &Q_lo);
   hChn->outputBitrate = (Q_lo + 1) >> 1;

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TfecSetTitr_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecSetTitr_isr(BSAT_ChannelHandle h)
{
   static const uint32_t turbo_titr[] =
   {
      0x007F2824,  /* turbo qpsk 1/2 */
      0x00D4C828,  /* turbo qpsk 2/3 */
      0x00BF4830,  /* turbo qpsk 3/4 */
      0x00D4C836,  /* turbo qpsk 5/6 */
      0x00DF483A,  /* turbo qpsk 7/8 */
      0x00FF8024,  /* turbo 8psk 2/3 */
      0x01118028,  /* turbo 8psk 3/4 */
      0x01190030,  /* turbo 8psk 4/5 */
      0x01328036,  /* turbo 8psk 5/6 */
      0x013F803A   /* turbo 8psk 8/9 */
   };

   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t i;

   i = hChn->actualMode - BSAT_Mode_eTurbo_Qpsk_1_2;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TITR, turbo_titr[i]);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TfecSetTtur_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecSetTtur_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, iter, P_hi, P_lo, Q_hi, Q_lo;

   /* 0.5 + 16/256 = 0.5625 = 9/16 */
   /* iter = (uint32_t)((float)fec_freq/(0.5625 * (float)acq_symbol_rate) - 1.0); */
#ifdef BSAT_HAS_DUAL_TFEC
   BMTH_HILO_32TO64_Mul(8, hChn->fecFreq, &P_hi, &P_lo); /* divide TFEC clock by 2 for dual TFEC cores */
#else
   BMTH_HILO_32TO64_Mul(16, hChn->fecFreq, &P_hi, &P_lo);
#endif
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 9 * hChn->acqSettings.symbolRate, &Q_hi, &Q_lo);
   iter = Q_lo - 1;
   if (iter > 19)
      iter = 19;

   val = 0x00000A03 | (iter << 16);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TTUR, val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TfecSetTssq_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecSetTssq_isr(BSAT_ChannelHandle h)
{
   static const uint8_t turbo_tssq_qpsk_1_2[2] =
   {
      0x01,
      0xDD
   };

   static const uint8_t turbo_tssq_qpsk_2_3[4] =
   {
      0x03,
      0xDD,
      0xDD,
      0xEE
   };

   static const uint8_t turbo_tssq_qpsk_3_4[5] =
   {
      0x04,
      0xED,
      0xED,
      0xDE,
      0xDE
   };

   static const uint8_t turbo_tssq_qpsk_5_6[7] =
   {
      0x06,
      0xED,
      0xDE,
      0xDE,
      0xEE,
      0xEE,
      0xED
   };

   static const uint8_t turbo_tssq_qpsk_7_8[9] =
   {
      0x08,
      0xED,
      0xED,
      0xDE,
      0xDE,
      0xEE,
      0xEE,
      0xEE,
      0xEE
   };

   static const uint8_t turbo_tssq_8psk_2_3[6] =
   {
      0x05,
      0x00,
      0x00,
      0x00,
      0x00,
      0x22
   };

   static const uint8_t turbo_tssq_8psk_3_4[8] =
   {
      0x07,
      0x00,
      0x00,
      0x20,
      0x10,
      0x00,
      0x01,
      0x02
   };

   static const uint8_t turbo_tssq_8psk_4_5[6] =
   {
      0x05,
      0x10,
      0x00,
      0x00,
      0x01,
      0x22
   };

   static const uint8_t turbo_tssq_8psk_5_6[6] =
   {
      0x05,
      0x10,
      0x01,
      0x10,
      0x01,
      0x22
   };

   static const uint8_t turbo_tssq_8psk_8_9[7] =
   {
      0x06,
      0x10,
      0x01,
      0x21,
      0x10,
      0x01,
      0x12
   };

   const uint8_t* turbo_tssq_table[10] =
   {
      (const uint8_t *)turbo_tssq_qpsk_1_2,
      (const uint8_t *)turbo_tssq_qpsk_2_3,
      (const uint8_t *)turbo_tssq_qpsk_3_4,
      (const uint8_t *)turbo_tssq_qpsk_5_6,
      (const uint8_t *)turbo_tssq_qpsk_7_8,
      (const uint8_t *)turbo_tssq_8psk_2_3,
      (const uint8_t *)turbo_tssq_8psk_3_4,
      (const uint8_t *)turbo_tssq_8psk_4_5,
      (const uint8_t *)turbo_tssq_8psk_5_6,
      (const uint8_t *)turbo_tssq_8psk_8_9
   };

   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, i;
   const uint8_t *pTable;
   uint8_t len;

   i = hChn->actualMode - BSAT_Mode_eTurbo_Qpsk_1_2;
   pTable = turbo_tssq_table[i];

   len = *pTable++;
   for (i = (uint32_t)len; i; i--)
   {
      val = (uint32_t)(*pTable++);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TSSQ, val);
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TfecConfig_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecConfig_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val;

#ifdef BSAT_HAS_DUAL_TFEC
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, 0x80); /* tfec_rst=1, tfec_go=0 */
#else
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_GR_BRIDGE_SW_INIT_0, 1);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_GR_BRIDGE_SW_INIT_0, 0);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, 0x80); /* tfec_rst=1 */
#endif

   BSAT_CHK_RETCODE(BSAT_g1_P_TfecSetTitr_isr(h));

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TCIL, 0x00009FCC);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TRSD, 0x00004FCC);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TZPK, 0x03B58F34);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFMT, 0x00028008);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TSYN, 0x0103FEFE);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TPAK, 0x0009BB47);

   if (hChn->xportSettings.bTei)
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_TFEC_TPAK, 1<<17);

   BSAT_CHK_RETCODE(BSAT_g1_P_TfecSetTtur_isr(h));

   if (hChn->turboSettings.ctl & BSAT_TURBO_CTL_OVERRIDE_TZSY)
      val = hChn->turboSettings.tzsyOverride;
   else
   {
      if (hChn->actualMode == BSAT_Mode_eTurbo_Qpsk_1_2)
         val = 0x00080810;
      else
         val = 0x000C0810;
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TZSY, val); /* orig: 0x00020E0F */

#ifndef BSAT_HAS_DUAL_TFEC
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, 0x00); /* tfec_go=0, tfec_rst=0 */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, 0x40);
   BSAT_CHK_RETCODE(BSAT_g1_P_TfecSetTssq_isr(h));
#else
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, 0x00); /* tfec_rst=0, tfec_go=0 */
   BSAT_CHK_RETCODE(BSAT_g1_P_TfecSetTssq_isr(h));
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, 0x20); /* tfec_go=1 */
#endif

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_TfecOnSyncTimeout_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecOnSyncTimeout_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
#ifdef BSAT_DEBUG_ACQ_TIME
   uint32_t status;
   status = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_TFEC_INTR2_CPU_STATUS);
#endif

   BSAT_CHK_RETCODE(BSAT_g1_P_TfecEnableSyncInterrupt_isr(h, false));

   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eBaud); /* disable sync timeout */

   if (hChn->turboScanState & BSAT_TURBO_SCAN_STATE_SYNC_ACQUIRED)
      hChn->turboScanLockedModeFailures++;

#ifdef BSAT_DEBUG_ACQ_TIME
BSAT_g1_P_GetAcquisitionTimerValue_isr(h, &t1);
BDBG_ERR(("TfecOnSyncTimeout: t=%d, actualMode=0x%X, status=0x%X", t1-t0, hChn->actualMode, status));
#endif

   if (BSAT_g1_P_IsHpLocked_isr(h))
   {
      hChn->count2++;
      if ((hChn->acqSettings.mode == BSAT_Mode_eTurbo_scan) && (hChn->count2 > 1))
      {
         hChn->reacqCause = BSAT_ReacqCause_eTurboSyncTimeout;
         goto reacquire;
      }
      BSAT_DEBUG_TFEC(BDBG_WRN(("reacquiring HP since TFEC can't lock")));
      BSAT_CHK_RETCODE(BSAT_g1_P_HpEnable_isr(h, false));
      retCode = BSAT_g1_P_TfecAcquire_isr(h); /* reacquire HP */
   }
   else
   {
      hChn->reacqCause = BSAT_ReacqCause_eHpLostLock;

      reacquire:
      retCode = BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);
   }

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_TfecRun_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecRun_isr(BSAT_ChannelHandle h)
{
   static const uint16_t rs_timeout[10] =
   {
      /* 1359, 1226, 1097, 989, 989, 946, 946, 817, 710, 710 */
      859,  912,  870,  786, 834, 765, 892, 901, 646, 655
   };

   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t timeout, mode_idx;

#ifndef BSAT_HAS_DUAL_TFEC
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, 0x63); /* reset error counters */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, 0x61); /* clear error counter reset */
#endif

#ifdef BSAT_DEBUG_ACQ_TIME
BSAT_g1_P_GetAcquisitionTimerValue_isr(h, &t0);
#endif

   mode_idx = hChn->actualMode - BSAT_Mode_eTurbo_Qpsk_1_2;
   timeout = ((uint32_t)rs_timeout[mode_idx]) * 1000;
   return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaud, timeout, BSAT_g1_P_TfecOnSyncTimeout_isr);
}


#ifdef BSAT_HAS_DUAL_TFEC
/******************************************************************************
 BSAT_g1_P_TfecGetOtherChannelHandle_isrsafe()
******************************************************************************/
BSAT_ChannelHandle BSAT_g1_P_TfecGetOtherChannelHandle_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_P_Handle *hDev = h->pDevice;
   BSAT_ChannelHandle hOtherChan;

   if (h->channel & 1)
      hOtherChan = hDev->pChannels[h->channel-1];
   else
      hOtherChan = hDev->pChannels[h->channel+1];
   return hOtherChan;
}


/******************************************************************************
 BSAT_g1_P_TfecIsOtherChannelBusy_isrsafe()
******************************************************************************/
bool BSAT_g1_P_TfecIsOtherChannelBusy_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_ChannelHandle hOtherChan = BSAT_g1_P_TfecGetOtherChannelHandle_isrsafe(h);
   BSAT_g1_P_ChannelHandle *hOtherChanImpl;
   bool bOtherTfecBusy = false;

   if (hOtherChan == NULL)
      return false;

   hOtherChanImpl = (BSAT_g1_P_ChannelHandle *)(hOtherChan->pImpl);
   if (hOtherChanImpl->bAbortAcq == false)
   {
      if (hOtherChanImpl->acqType == BSAT_AcqType_eTurbo)
         bOtherTfecBusy = true;
   }

   return bOtherTfecBusy;
}


/******************************************************************************
 BSAT_g1_P_TfecReacquire_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_TfecReacquire_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);

   BDBG_ERR(("BSAT_g1_P_TfecReacquire_isr(%d)", h->channel));

   if ((hChn->acqSettings.options & BSAT_ACQ_DISABLE_REACQ) && hChn->miscSettings.bPreserveState)
      goto reacquire;

   if (BSAT_g1_P_IsTfecOn_isrsafe(h) == false)
      goto reacquire;

   if (BSAT_g1_P_IsTfecOn_isrsafe(h))
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, 0x80); /* tfec_rst=1 */
   if (BSAT_g1_P_TfecIsOtherChannelBusy_isrsafe(h) == false)
      BSAT_g1_P_TfecPowerDown_isrsafe(h);

   reacquire:
   return BSAT_g1_P_Reacquire_isr(h);
}
#endif /* BSAT_HAS_DUAL_TFEC */

#endif /* BSAT_EXCLUDE_TFEC */
