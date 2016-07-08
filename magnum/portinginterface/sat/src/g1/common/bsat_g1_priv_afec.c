/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to the terms and
* conditions of a separate, written license agreement executed between you and Broadcom
* (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
* no license (express or implied), right to use, or waiver of any kind with respect to the
* Software, and Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
* secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
* and to use this information only in connection with your use of Broadcom integrated circuit products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
* AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
* WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
* THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
* OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
* LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
* OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
* USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
* LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
* EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
* USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
* ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
* LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
* ANY LIMITED REMEDY.
*
* Module Description:
*
*****************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"

#ifndef BSAT_EXCLUDE_AFEC

BDBG_MODULE(bsat_g1_priv_afec);


#define BSAT_DEBUG_FLUSH(x) /* x */
#define BSAT_DEBUG_AFEC(x) /* x */


/* local functions */
bool BSAT_g1_P_AfecIs8psk_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecSetPldctl_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecConfigEq_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecSetVlctl_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecSetHardDecisionLevels_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecSetScramblingSeq_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecSetPilotctl_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecOnHpLock_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecGeneratePdTable_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecConfigSnr_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecConfig_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecSetOpll_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecIsFlushDone_isr(BSAT_ChannelHandle h, bool *pbReady);
BERR_Code BSAT_g1_P_AfecAcquire0_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecWaitForFlushDone_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT nextFunct);
BERR_Code BSAT_g1_P_AfecWaitForFlushDone1_isr(BSAT_ChannelHandle h);
bool BSAT_g1_P_AfecIsOtherChannelEnabled_isr(BSAT_ChannelHandle h);
bool BSAT_g1_P_AfecIsOtherChannelFlushFailed_isr(BSAT_ChannelHandle h);
void BSAT_g1_P_AfecInitEqTaps_isr(BSAT_ChannelHandle h);


/******************************************************************************
 BSAT_g1_P_AfecEnableLockInterrupts_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecEnableLockInterrupts_isr(BSAT_ChannelHandle h, bool bEnable)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (bEnable)
   {
      BINT_EnableCallback_isr(hChn->hAfecLockCb);
      BINT_EnableCallback_isr(hChn->hAfecNotLockCb);
   }
   else
   {
      BINT_DisableCallback_isr(hChn->hAfecLockCb);
      BINT_DisableCallback_isr(hChn->hAfecNotLockCb);
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecEnableMpegInterrupts_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecEnableMpegInterrupts_isr(BSAT_ChannelHandle h, bool bEnable)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (bEnable)
   {
      BINT_EnableCallback_isr(hChn->hAfecMpegLockCb);
      BINT_EnableCallback_isr(hChn->hAfecMpegNotLockCb);
   }
   else
   {
      BINT_DisableCallback_isr(hChn->hAfecMpegLockCb);
      BINT_DisableCallback_isr(hChn->hAfecMpegNotLockCb);
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecAcquire_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecAcquire_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   if (hChn->bHasAfec == false)
   {
      if (hChn->bBlindScan)
      {
         hChn->reacqCause = BSAT_ReacqCause_eNoAfec;
         return BSAT_g1_P_AfecReacquire_isr(h);
      }
      else
      {
         hChn->reacqCause = BSAT_ReacqCause_eNoAfec;
         return BERR_NOT_AVAILABLE;
      }
   }

   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_scan)
      hChn->dvbs2ScanState = BSAT_DVBS2_SCAN_STATE_ENABLED;
   else
      hChn->dvbs2ScanState = 0;

   /* software pilot scan */
   if ((hChn->reacqCount > 0) && (hChn->acqSettings.options & BSAT_ACQ_PILOT_SCAN_ENABLE))
      hChn->acqSettings.options ^= BSAT_ACQ_PILOT;

   BSAT_CHK_RETCODE(BSAT_g1_P_NonLegacyModeAcquireInit_isr(h));

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecConfigEq_isr(h));
   BSAT_g1_P_AfecInitEqTaps_isr(h);
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 1); /* reset the eq */
   BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetVlctl_isr(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetHardDecisionLevels_isr(h));
   /* move to HP: BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetScramblingSeq_isr(h)); */

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_XTAP1, 0x00000100);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_XTAP2, 0x00805000);

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetPldctl_isr(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_ConfigPlc_isr(h, true)); /* set acquisition plc */
   BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetPilotctl_isr(h));

   /* disable soft pd tables until after HP locks*/
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, ~0x00000060);

   /* configure and run the HP */
   retCode = BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaud, 400000, BSAT_g1_P_AfecAcquire0_isr);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_AfecAcquire0_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecAcquire0_isr(BSAT_ChannelHandle h)
{
   return BSAT_g1_P_HpAcquire_isr(h, BSAT_g1_P_AfecOnHpLock_isr);
}


/******************************************************************************
 BSAT_g1_P_AfecOnHpLock_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecOnHpLock_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val, val2;

   if (hChn->actualMode >= BSAT_Mode_eDvbs2_16apsk_2_3)
   {
      BDBG_MSG(("BSAT_Mode 0x%X not supported", hChn->actualMode));
      return BSAT_g1_P_AfecReacquire_isr(h);
   }

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecPowerUp_isr(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_AfecEnableChannel_isrsafe(h, false));
   BSAT_CHK_RETCODE(BSAT_g1_P_GetAfecClock_isrsafe(h, &(hChn->fecFreq)));

   if (BSAT_g1_P_AfecIsOtherChannelBusy_isrsafe(h))
   {
      BSAT_ChannelHandle hOther = BSAT_g1_P_AfecGetOtherChannelHandle_isrsafe(h);
      BSAT_g1_P_ChannelHandle *hOtherChn = (BSAT_g1_P_ChannelHandle *)hOther->pImpl;

      if (hChn->bShortFrame != hOtherChn->bShortFrame)
      {
         BDBG_WRN(("Cannot mix normal/short frames in dual AFEC"));
         hChn->reacqCause = BSAT_ReacqCause_eFrameLengthCannotMix;
         return BSAT_g1_P_AfecReacquire_isr(h);
      }
   }

   /* set AFECNX_GLOBAL_CONFIG.SHORT_CODE if necessary */
   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CONFIG);
   val2 = val;
   if ((val & BCHP_AFECNX_0_GLOBAL_CONFIG_SHORT_CODE_MASK) && !hChn->bShortFrame)
      val2 &= ~BCHP_AFECNX_0_GLOBAL_CONFIG_SHORT_CODE_MASK; /* normal frames */
   else if (((val & BCHP_AFECNX_0_GLOBAL_CONFIG_SHORT_CODE_MASK) == 0) && hChn->bShortFrame)
      val2 |= BCHP_AFECNX_0_GLOBAL_CONFIG_SHORT_CODE_MASK;
   if (val != val2)
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CONFIG, val2);

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecGeneratePdTable_isr(h));

   if (hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_ENABLED)
   {
      BSAT_CHK_RETCODE(BSAT_g1_P_ConfigPlc_isr(h, true)); /* set acquisition plc to the non-scan plc value */
      BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetVlctl_isr(h));
      BSAT_CHK_RETCODE(BSAT_g1_P_AfecConfigEq_isr(h));

      if (BSAT_MODE_IS_DVBS2_QPSK(hChn->actualMode))
      {
         /* we need to reprogram HDQPSK because we assumed 8PSK when acquiring in scan mode */
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HDQPSK, 0x01000000);
      }
   }
   else if (hChn->acqSettings.options & BSAT_ACQ_PILOT_SCAN_ENABLE)
   {
      BSAT_CHK_RETCODE(BSAT_g1_P_AfecConfigEq_isr(h));
   }

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetPilotctl_isr(h));

   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, 0xFFFFFBFF); /* disable CMA */
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 0xFFFF00FF, 0x0200); /* unfreze other taps */

   if (hChn->bEnableFineFreqEst == false)
   {
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE, 0x0000000F); /* override front carrier loop */
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, 0xFFFFFFEF);    /* disable front carrier loop */
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, 0x00000004);     /* freeze front carrier loop */
   }
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE, 1<<21); /* override snore reset */

   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
   {
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_MGAINA, 0x00000000);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_MGAIND, 0x5a383838);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_MGAIND, 0x384e5a5a);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_MGAIND, 0x5a5a5a5a);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_MGAIND, 0x40404040);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_MGAIND, 0x48480000);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_MGAINA, 0x00000000);
   }
   else
   {
      if (hChn->actualMode >= BSAT_Mode_eDvbs2_8psk_8_9)
         val = 0x24000000;
      else if (BSAT_MODE_IS_DVBS2_8PSK(hChn->actualMode))
         val = 0x28000000;
      else if (hChn->actualMode == BSAT_Mode_eDvbs2_Qpsk_1_2)
         val = 0x33000000;
      else if (hChn->actualMode == BSAT_Mode_eDvbs2_Qpsk_3_5)
         val = 0x3E000000;
      else
         val = 0x43000000;    /* rest of qpsk */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VLCI, val);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VLCQ, val);
   }

   BSAT_CHK_RETCODE(BSAT_g1_P_SetAgcTrackingBw_isr(h));

   val = 0x34;
   if ((hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM) || BSAT_g1_P_AfecIsPilot_isr(h))
      val |= 0x08;
   if ((hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM) || BSAT_g1_P_AfecIs8psk_isr(h))
      val |= 0xC0;
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLFBCTL, 0xFFFF0000, val);

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecConfigSnr_isr(h));

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecConfig_isr(h));

   if (hChn->xportSettings.bOpllBypass == false)
      BSAT_g1_P_PowerUpOpll_isr(h);

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetOpll_isr(h));

   BSAT_CHK_RETCODE(BSAT_g1_P_ConfigOif_isr(h));

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_RST, 0x00070002);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_INTR_CPU_CLEAR, 0xFFFFFFFF);
   BSAT_CHK_RETCODE(BSAT_g1_P_AfecEnableChannel_isrsafe(h, true));
#ifdef BCHP_AFECNX_CHANNEL_ENABLE
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_RST, 0);
#else
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_AFEC_RST, 1);
#endif

   retCode = BSAT_g1_P_StartTracking_isr(h);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_AfecReacquire_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecReacquire_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BSAT_DEBUG_FLUSH(BDBG_WRN(("BSAT_g1_P_AfecReacquire_isr(%d)", h->channel)));

   if ((hChn->acqSettings.options & BSAT_ACQ_DISABLE_REACQ) && hChn->miscSettings.bPreserveState)
      goto reacquire;

   if (hChn->miscSettings.maxReacqs > 0)
   {
      if (hChn->reacqCount >= hChn->miscSettings.maxReacqs)
      {
         if (hChn->miscSettings.bPreserveState)
            goto reacquire;
      }
   }

   if (BSAT_g1_P_IsAfecOn_isrsafe(h) == false)
   {
      reacquire:
      return BSAT_g1_P_Reacquire_isr(h);
   }

   BSAT_g1_P_AfecResetChannel_isr(h);

   if (BSAT_g1_P_AfecIsChannelEnabled_isr(h) == false)
      goto reacquire;

   /* channel is currently enabled */
   if (BSAT_g1_P_AfecIsOtherChannelBusy_isrsafe(h))
   {
      /* do a flush */
      BSAT_DEBUG_FLUSH(BDBG_WRN(("AFEC%d flush", h->channel)));
      BSAT_g1_P_AfecEnableChannel_isrsafe(h, false);
      return BSAT_g1_P_AfecWaitForFlushDone_isr(h, BSAT_g1_P_Reacquire_isr);
   }
   else
   {
      BSAT_g1_P_AfecPowerDown_isrsafe(h);
      goto reacquire;
   }
}


/******************************************************************************
 BSAT_g1_P_AfecIsValidMode_isr()
******************************************************************************/
bool BSAT_g1_P_AfecIsValidMode_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t i, mask;

   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
      return true;

   if (BSAT_MODE_IS_DVBS2(hChn->actualMode))
   {
      if (hChn->actualMode == BSAT_Mode_eDvbs2_scan)
         goto invalid_condition;

      i = hChn->actualMode - BSAT_Mode_eDvbs2_Qpsk_1_4;
      mask = (1 << i);
      if ((mask & hChn->dvbs2Settings.scanModes) == 0)
         return false;
   }
   else
   {
      invalid_condition:
      BDBG_ERR(("BSAT_g1_P_AfecIsValidMode_isr() - invalid condition"));
      return false;
   }

   return true;
}


/******************************************************************************
 BSAT_g1_P_AfecIsLocked_isr()
******************************************************************************/
bool BSAT_g1_P_AfecIsLocked_isr(BSAT_ChannelHandle h)
{
   uint32_t val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_INTR_RAW_STS0) & 0x180;
   return (val == 0x80) ? true : false;
}


/******************************************************************************
 BSAT_g1_P_AfecIsMpegLocked_isr()
******************************************************************************/
bool BSAT_g1_P_AfecIsMpegLocked_isr(BSAT_ChannelHandle h)
{
   uint32_t val;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_LDPC_STATUS);
   if ((val & 0xC0000000) != 0xC0000000)
      return false;
   else
      return true;
}


/******************************************************************************
 BSAT_g1_P_AfecIsPilot_isr()
******************************************************************************/
bool BSAT_g1_P_AfecIsPilot_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_ENABLED)
   {
      if (hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_PILOT)
         return true;
   }
   else if (hChn->acqSettings.options & BSAT_ACQ_PILOT)
      return true;
   return false;
}


/******************************************************************************
 BSAT_g1_P_AfecOnLock_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecOnLock_isr(BSAT_ChannelHandle h)
{
   BSTD_UNUSED(h);

   /* do nothing */
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecOnLostLock_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecOnLostLock_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecUpdateBlockCount_isrsafe(h));

   if (BSAT_g1_P_IsHpLocked_isr(h) == false)
   {
      hChn->reacqCause = BSAT_ReacqCause_eHpLostLock;
      hChn->bForceReacq = true;
   }

   done:
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecOnStableLock_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecOnStableLock_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val;

   if (BSAT_g1_P_AfecIsMpegLocked_isr(h) == false)
      hChn->count1 = 1;

   if (hChn->bPlcTracking == false)
   {
      BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 0x00FFFFFF, 0x66000000); /* set tracking mu */
      BSAT_g1_P_ConfigPlc_isr(h, false); /* set tracking PLC */
   }

   if (hChn->bEnableFineFreqEst == false)
   {
      /* narrow DAFE loop bw */
      if (BSAT_g1_P_AfecIsPilot_isr(h)) /* all pilot on modes, except LDPC 8PSK 3/4 */
      {
         if (hChn->actualMode != BSAT_Mode_eDvbs2_8psk_3_4)
         {
            val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL) | 0x04; /* Use Pilot for FROF3 estimation to avoid error at the edge of freq. drift */
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, val);
            val = 0x00005044;
         }
         else
            val = 0x00005056;  /* 8psk 3/4 pilot  on, do not use pilot for FROF3 est. */
      }
      else
         val = 0x00005056;
      BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, 0x60000, val);
   }

   /* set tracking baud loop bw */
   BSAT_CHK_RETCODE(BSAT_g1_P_SetBaudBw_isr(h, hChn->acqSettings.symbolRate / 400, 4));

   if (hChn->bEverStableLock == false)
   {
      BSAT_CHK_RETCODE(BSAT_g1_P_AfecResetBlockCount_isrsafe(h));
   }

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_AfecOnMonitorLock_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecOnMonitorLock_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (BSAT_g1_P_AfecIsMpegLocked_isr(h) == false)
   {
      hChn->count1++;
      if (hChn->count1 > 2)
      {
         /* MP is not locked, so force reacquire */
         BSAT_DEBUG_AFEC(BDBG_WRN(("BSAT_g1_P_AfecOnMonitorLock_isr(%d): mpeg not locked, reacquiring...", h->channel)));
         hChn->bForceReacq = true;
      }
   }
   else
      hChn->count1 = 0;

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecMpegLock_isr() - callback routine for AFEC MPEG lock interrupt
******************************************************************************/
void BSAT_g1_P_AfecMpegLock_isr(void *p, int int_id)
{
   BSAT_ChannelHandle h = (BSAT_ChannelHandle)p;
   BSAT_g1_P_IncrementInterruptCounter_isr(h, int_id);
   /* TBD... */
}


/******************************************************************************
 BSAT_g1_P_AfecMpegNotLock_isr() - callback routine for AFEC MPEG not lock interrupt
******************************************************************************/
void BSAT_g1_P_AfecMpegNotLock_isr(void *p, int int_id)
{
   BSAT_ChannelHandle h = (BSAT_ChannelHandle)p;
   BSAT_g1_P_IncrementInterruptCounter_isr(h, int_id);
   /* TBD... */
}


/******************************************************************************
 BSAT_g1_P_AfecGetStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecGetStatus(BSAT_ChannelHandle h, BSAT_Dvbs2Status *pStatus)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BKNI_EnterCriticalSection();
   retCode = BSAT_g1_P_AfecUpdateBlockCount_isrsafe(h);
   BKNI_LeaveCriticalSection();

   if (retCode == BERR_SUCCESS)
   {
      pStatus->bValid = true;
      pStatus->corrBlocks = hChn->corrBlocks;
      pStatus->badBlocks = hChn->badBlocks;
      pStatus->totalBlocks = hChn->totalBlocks;
      pStatus->fecFreq = hChn->fecFreq;
      pStatus->bPilot = BSAT_g1_P_AfecIsPilot_isr(h);
      pStatus->bShortFrame = hChn->bShortFrame;
   }
   else
      pStatus->bValid = false;
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_AfecIs8psk_isr()
******************************************************************************/
bool BSAT_g1_P_AfecIs8psk_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if ((hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_ENABLED) == BSAT_DVBS2_SCAN_STATE_ENABLED)
   {
      /* ldpc scan in progress */
      if ((hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_QPSK) == 0)
         return true;
   }
   else if (BSAT_MODE_IS_DVBS2(hChn->actualMode))
   {
      if (BSAT_MODE_IS_DVBS2_8PSK(hChn->actualMode))
         return true;
   }
   return false;
}


/******************************************************************************
 BSAT_g1_P_AfecSetPldctl_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecSetPldctl_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = 0x14;
   if ((BSAT_g1_P_AfecIs8psk_isr(h) == 0) && (hChn->acqSettings.mode != BSAT_Mode_eDvbs2_ACM))
   {
      /* DVB-S2 QPSK */
#ifdef BSAT_HAS_WFE
      val |= 0x7800; /* 70% backoff */
#else
      val |= 0x8000;
#endif
   }
   else
   {
      /* DVB-S2 8PSK */
#ifdef BSAT_HAS_WFE
      val |= 0x5600; /* 70% backoff */
#else
      val |= 0x4000;
#endif
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_PLDCTL, val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecInitEqTaps_isr()
******************************************************************************/
void BSAT_g1_P_AfecInitEqTaps_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, ffe_main_tap, i;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL);
   ffe_main_tap = (val >> 16) & 0x1F;

   for (i = 0; i < 24; i++)
   {
      val = 0x00;
      if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
      {
         if (i == ffe_main_tap)
            val = 0x20;
      }
      else if (BSAT_g1_P_AfecIs8psk_isr(h))
      {
         /* 8PSK */
         if (i == ffe_main_tap)
         {
#ifdef BSAT_HAS_WFE
            val = 0x36;
#else
            val = 0x20;
#endif
         }
      }
      else
      {
         /* QPSK */
         if (i == ffe_main_tap)
         {
#ifdef BSAT_HAS_WFE
            val = 0x25;
#else
            val = 0x18;
#endif
         }
      }

      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQCFAD, 0x40 | i);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_F0B, (val & 0xFF) << 24);
   }
}


/******************************************************************************
 BSAT_g1_P_AfecConfigEq_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecConfigEq_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL);
   val &= 0x0000FF06;
   val |= 0x22060720;
   val &= ~0x500;  /* unfreeze ffe and ffe main tap */
   if ((hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM) ||
       (BSAT_g1_P_AfecIsPilot_isr(h)))
      val |= 0x18;
   else
      val |= 0x10;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, val);

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL);
   val &= ~0x0038401F;
   val |= 0x00140000;
   if ((hChn->acqSettings.mode != BSAT_Mode_eDvbs2_ACM) && BSAT_g1_P_AfecIs8psk_isr(h))
      val |= 0x1A; /* err_mode, sym_mode, dvbs2_8psk_mapping */
   if ((hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM) || BSAT_g1_P_AfecIs8psk_isr(h))
      val |= 0x4000;  /* ext_en */
   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
   {
     val &= ~0x1F;
     val |= 0x1A;
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecSetVlctl_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecSetVlctl_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = 0x00040914;
   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
      val |= 0x00020008; /* set bit 17 (vlc_soft_insel), set bit 3 (vgain_sel) */
   else if (BSAT_g1_P_AfecIs8psk_isr(h))
      val |= 0x020000; /* set bit 17 (vlc_soft_insel) */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VLCTL, val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecSetHardDecisionLevels_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecSetHardDecisionLevels_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HD8PSK1, 0x01D901D9);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HD8PSK2, 0x00C400C4);

   if (hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_ENABLED)
   {
      if (hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_FOUND)
      {
         if (hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_QPSK)
            goto ldpc_set_hd_1;
         else
            goto ldpc_set_hd_0;
      }
      else
      {
         /* assume 8PSK in scan mode */
         goto ldpc_set_hd_0;
      }
   }
   else if ((hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM) || BSAT_g1_P_AfecIs8psk_isr(h))
   {
      ldpc_set_hd_0:
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HDQPSK, 0x016A0000);
   }
   else
   {
      /* LDPC QPSK */
      ldpc_set_hd_1:
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HDQPSK, 0x01000000);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecSetScramblingSeq_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecSetScramblingSeq_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_XSEED, hChn->scramblingSeq.xseed);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_PLHDRSCR1, hChn->scramblingSeq.plhdrscr1);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_PLHDRSCR2, hChn->scramblingSeq.plhdrscr2);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_PLHDRSCR3, hChn->scramblingSeq.plhdrscr3);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecSetPilotctl_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecSetPilotctl_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = 0;
   if ((hChn->acqSettings.mode != BSAT_Mode_eDvbs2_ACM) && BSAT_g1_P_AfecIsPilot_isr(h))
      val |= 0x01; /* pilot_mode */
   if (BSAT_g1_P_AfecIsPilot_isr(h) || (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM))
      val |= 0x02; /* pilot_update_mode */
   if ((hChn->acqSettings.mode != BSAT_Mode_eDvbs2_ACM) && (BSAT_g1_P_AfecIsPilot_isr(h) == false))
      val |= 0x04; /* phase_adj_en */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_PILOTCTL, val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecGetPdTableIdx_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecGetPdTableIdx_isr(uint32_t i, uint32_t j, uint32_t *pIdx)
{
   uint32_t sval1;

   /* idx = 30*i - i*(i-1)/2 + j - 1; */
   sval1 = (i * (i - 1)) >> 1;
   *pIdx = 30 * i - sval1 + j - 1;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecGeneratePdTable_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecGeneratePdTable_isr(BSAT_ChannelHandle h)
{
#define BSAT_g1_PD_DATA_QPSK_EXTENSION 16

   static const uint8_t pd_data_qpsk[] =
   {
      0x00,
      0x2E,
      0x54,
      0x70,
      0x81,
      0x8C,
      0x91,
      0x95,
      0x97,
      0x98,
      0x98,
      0x98,
      0x99,
      0x99,
      0x99,
      0x99,
      0x99,
   };

   static const uint8_t pd_data_8psk[] =
   {
      0x00, 0x01, 0x06,
      0x00, 0x01, 0x6F,
      0x00, 0x01, 0x93,
      0x00, 0x01, 0xA1,
      0x00, 0x01, 0xA8,
      0x00, 0x01, 0xAB,
      0x00, 0x01, 0xAD,
      0x00, 0x01, 0xAD,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x01, 0x55, 0x54,
      0x01, 0x05, 0x83,
      0x00, 0xD9, 0x99,
      0x00, 0xC3, 0xA4,
      0x00, 0xB7, 0xA9,
      0x00, 0xB3, 0xAC,
      0x00, 0xAF, 0xAD,
      0x00, 0xAF, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x01, 0xC7, 0x5C,
      0x01, 0x79, 0x82,
      0x01, 0x4B, 0x98,
      0x01, 0x31, 0xA3,
      0x01, 0x23, 0xA9,
      0x01, 0x1D, 0xAB,
      0x01, 0x1B, 0xAD,
      0x01, 0x19, 0xAD,
      0x01, 0x19, 0xAE,
      0x01, 0x19, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0xF1, 0x5D,
      0x01, 0xA7, 0x81,
      0x01, 0x79, 0x96,
      0x01, 0x61, 0xA2,
      0x01, 0x55, 0xA8,
      0x01, 0x4D, 0xAB,
      0x01, 0x4B, 0xAD,
      0x01, 0x49, 0xAD,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0xFF, 0x5D,
      0x01, 0xB9, 0x80,
      0x01, 0x8B, 0x96,
      0x01, 0x73, 0xA2,
      0x01, 0x67, 0xA8,
      0x01, 0x61, 0xAB,
      0x01, 0x5D, 0xAD,
      0x01, 0x5D, 0xAD,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x02, 0x05, 0x5D,
      0x01, 0xBF, 0x80,
      0x01, 0x93, 0x96,
      0x01, 0x7B, 0xA2,
      0x01, 0x6D, 0xA8,
      0x01, 0x67, 0xAB,
      0x01, 0x65, 0xAD,
      0x01, 0x63, 0xAD,
      0x01, 0x63, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC1, 0x80,
      0x01, 0x95, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x6F, 0xA8,
      0x01, 0x69, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC1, 0x80,
      0x01, 0x95, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC1, 0x80,
      0x01, 0x95, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC1, 0x80,
      0x01, 0x95, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x02, 0x07, 0x5D,
   };

   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val, i, j, idx;
   uint8_t isb_byte1, isb_byte2, isb_byte3;

   /* enable PD LUT memory micro access */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_LUPA, 0x80000000);

   for (i = 0; i < 32; i++)
   {
      for (j = 0; j < 32; j++)
      {
         if (BSAT_g1_P_AfecIs8psk_isr(h))
         {
            /* 8psk case */
            if (i < j)
            {
               /* i < j case */
               BSAT_CHK_RETCODE(BSAT_g1_P_AfecGetPdTableIdx_isr(i, j, &idx));
               idx *= 3;
               isb_byte3 = pd_data_8psk[idx];
               isb_byte2 = pd_data_8psk[idx + 1];
               isb_byte1 = pd_data_8psk[idx + 2];
            }
            else if (i > j)
            {
               /* i > j case, must transpose value also */
               BSAT_CHK_RETCODE(BSAT_g1_P_AfecGetPdTableIdx_isr(j, i, &idx));
               idx *= 3;
               isb_byte3 = ((pd_data_8psk[idx + 1] << 1) | (pd_data_8psk[idx + 2] >> 7)) & 0x3;
               isb_byte2 = (pd_data_8psk[idx + 2] << 1) | (pd_data_8psk[idx] >> 1);
               isb_byte1 = (pd_data_8psk[idx] << 7) | (pd_data_8psk[idx + 1] >> 1);
            }
            else
            {
               isb_byte1 = isb_byte2 = isb_byte3 = 0;
            }
         }
         else
         {
            /* qpsk case */
            isb_byte1 = pd_data_qpsk[(j < BSAT_g1_PD_DATA_QPSK_EXTENSION) ? j : BSAT_g1_PD_DATA_QPSK_EXTENSION];
            val = pd_data_qpsk[(i < BSAT_g1_PD_DATA_QPSK_EXTENSION) ? i : BSAT_g1_PD_DATA_QPSK_EXTENSION];
            isb_byte2 = (val << 1);
            isb_byte3 = (val >> 7);
         }
         val = (isb_byte3 << 24) | (isb_byte2 << 16) | (isb_byte1 << 8);
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_LUPD, val);
      }
   }

   /* disable PD LUT memory micro access */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_LUPA, 0);

   /* deselect CCI phase detect, select soft slicer for carrier loop */
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, ~0x0000040, 0x00000020);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_AfecConfigSnr_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecConfigSnr_isr(BSAT_ChannelHandle h)
{
   static const uint32_t DVBS2_SNRHT[] =
   {
      0x00000000, /* DVB-S2 QPSK 1/4 */
      0x00000000, /* DVB-S2 QPSK 1/3 */
      0x00000000, /* DVB-S2 QPSK 2/5 */
      0x03c03235, /* DVB-S2 QPSK 1/2 */
      0x02e95977, /* DVB-S2 QPSK 3/5 */
      0x026bf466, /* DVB-S2 QPSK 2/3 */
      0x01ec7290, /* DVB-S2 QPSK 3/4 */
      0x01a3240c, /* DVB-S2 QPSK 4/5 */
      0x01758f45, /* DVB-S2 QPSK 5/6 */
      0x0128ba9e, /* DVB-S2 QPSK 8/9 */
      0x011b5fbc, /* DVB-S2 QPSK 9/10 */
      0x0299deea, /* DVB-S2 8PSK 3/5 */
      0x0210eb81, /* DVB-S2 8PSK 2/3 */
      0x01881801, /* DVB-S2 8PSK 3/4 */
      0x011594c5, /* DVB-S2 8PSK 5/6 */
      0x00c916fa, /* DVB-S2 8PSK 8/9 */
      0x00bbaafa, /* DVB-S2 8PSK 9/10 */
      0x00000000, /* DVB-S2 16APSK 2/3 */
      0x00000000, /* DVB-S2 16APSK 3/4 */
      0x00000000, /* DVB-S2 16APSK 4/5 */
      0x00000000, /* DVB-S2 16APSK 5/6 */
      0x00000000, /* DVB-S2 16APSK 8/9 */
      0x00000000, /* DVB-S2 16APSK 9/10 */
      0x00000000, /* DVB-S2 32APSK 3/4 */
      0x00000000, /* DVB-S2 32APSK 4/5 */
      0x00000000, /* DVB-S2 32APSK 5/6 */
      0x00000000, /* DVB-S2 32APSK 8/9 */
      0x00000000, /* DVB-S2 32APSK 9/10 */
   };

   static const uint32_t DVBS2_SNRLT[] =
   {
      0x00000000, /* DVB-S2 QPSK 1/4 */
      0x00000000, /* DVB-S2 QPSK 1/3 */
      0x00000000, /* DVB-S2 QPSK 2/5 */
      0x2581f613, /* DVB-S2 QPSK 1/2 */
      0x2581f613, /* DVB-S2 QPSK 3/5 */
      0x18378c00, /* DVB-S2 QPSK 2/3 */
      0x133c79a2, /* DVB-S2 QPSK 3/4 */
      0x105f6879, /* DVB-S2 QPSK 4/5 */
      0x0e9798ae, /* DVB-S2 QPSK 5/6 */
      0x0b974a29, /* DVB-S2 QPSK 8/9 */
      0x0b11bd5a, /* DVB-S2 QPSK 9/10 */
      0x1a02b525, /* DVB-S2 8PSK 3/5 */
      0x14a9330f, /* DVB-S2 8PSK 2/3 */
      0x0f50f00e, /* DVB-S2 8PSK 3/4 */
      0x0ad7cfb3, /* DVB-S2 8PSK 5/6 */
      0x07dae5c5, /* DVB-S2 8PSK 8/9 */
      0x0754adc5, /* DVB-S2 8PSK 9/10 */
      0x00000000, /* DVB-S2 16APSK 2/3 */
      0x00000000, /* DVB-S2 16APSK 3/4 */
      0x00000000, /* DVB-S2 16APSK 4/5 */
      0x00000000, /* DVB-S2 16APSK 5/6 */
      0x00000000, /* DVB-S2 16APSK 8/9 */
      0x00000000, /* DVB-S2 16APSK 9/10 */
      0x00000000, /* DVB-S2 32APSK 3/4 */
      0x00000000, /* DVB-S2 32APSK 4/5 */
      0x00000000, /* DVB-S2 32APSK 5/6 */
      0x00000000, /* DVB-S2 32APSK 8/9 */
      0x00000000, /* DVB-S2 32APSK 9/10 */
   };

   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val, i;

   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
   {
      /* TBD */
   }
   else
   {
      i = hChn->actualMode - BSAT_Mode_eDvbs2_Qpsk_1_4;
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRHT, DVBS2_SNRHT[i]); /* TBD: complete the tables! */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRLT, DVBS2_SNRLT[i]);
   }

   BSAT_CHK_RETCODE(BSAT_g1_P_InitBert_isr(h));

   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
      val = 0x01546732;
   else if (BSAT_g1_P_AfecIs8psk_isr(h))
   {
      if (hChn->acqSettings.options & BSAT_ACQ_ENABLE_BERT)
         val = 0x04623751;
      else
         val = 0x01326754;
   }
   else
      val = 0x04576231;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BEM1, val);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BEM2, 0x01546732);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BEIT, 0x0FFF05FF);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRCTL, 0x88);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRCTL, 0x0B);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNORECTL, 0xA3); /* alpha=2^-8 */

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_AfecSetMpcfg_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecSetMpcfg_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_BCH_MPCFG);
   val &= ~0x30;
   if ((hChn->xportSettings.bchMpegErrorMode == BSAT_BchMpegErrorMode_eBch) ||
       (hChn->xportSettings.bchMpegErrorMode == BSAT_BchMpegErrorMode_eBchAndCrc8))
      val |= 0x20;
   if ((hChn->xportSettings.bchMpegErrorMode == BSAT_BchMpegErrorMode_eCrc8) ||
       (hChn->xportSettings.bchMpegErrorMode == BSAT_BchMpegErrorMode_eBchAndCrc8))
      val |= 0x10;

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_MPCFG, val);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_MPLCK, 0x03040E0F);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecComputeOutPfill_isr() - compute the output smoother data FIFO
                                   pre-fill level as follows:
    out_pfill = (int)((((bch_k - 80) - 1504) * 3) / 32)
******************************************************************************/
BERR_Code BSAT_g1_P_AfecComputeOutPfill_isr(BSAT_ChannelHandle h, uint32_t modcod, uint32_t *pOutPfill)
{
   static const uint16_t BSAT_bch_k[] =
   {
      16008, /* DVB-S2 QPSK 1/4 */
      21408, /* DVB-S2 QPSK 1/3 */
      25728, /* DVB-S2 QPSK 2/5 */
      32208, /* DVB-S2 QPSK 1/2 */
      38688, /* DVB-S2 QPSK 3/5 */
      43040, /* DVB-S2 QPSK 2/3 */
      48408, /* DVB-S2 QPSK 3/4 */
      51648, /* DVB-S2 QPSK 4/5 */
      53840, /* DVB-S2 QPSK 5/6 */
      57472, /* DVB-S2 QPSK 8/9 */
      58192, /* DVB-S2 QPSK 9/10 */
      38688, /* DVB-S2 8PSK 3/5 */
      43040, /* DVB-S2 8PSK 2/3 */
      48408, /* DVB-S2 8PSK 3/4 */
      53840, /* DVB-S2 8PSK 5/6 */
      57472, /* DVB-S2 8PSK 8/9 */
      58192, /* DVB-S2 8PSK 9/10 */
      0, /* DVB-S2 16APSK 2/3 */
      0, /* DVB-S2 16APSK 3/4 */
      0, /* DVB-S2 16APSK 4/5 */
      0, /* DVB-S2 16APSK 5/6 */
      0, /* DVB-S2 16APSK 8/9 */
      0, /* DVB-S2 16APSK 9/10 */
      0, /* DVB-S2 32APSK 3/4 */
      0, /* DVB-S2 32APSK 4/5 */
      0, /* DVB-S2 32APSK 5/6 */
      0, /* DVB-S2 32APSK 8/9 */
      0  /* DVB-S2 32APSK 9/10 */
   };

   BSTD_UNUSED(h);
   *pOutPfill = (uint32_t)((3 * ((BSAT_bch_k[modcod-1] - 80) - 1504)) / 32);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecGetDecoderParams_isr() -
 compute number of ldpc clock cycles to decode the channel frame.
******************************************************************************/
BERR_Code BSAT_g1_P_AfecGetDecoderParams_isr(BSAT_ChannelHandle h, uint32_t *pModcod, uint32_t *pDecoderCycleCount, uint32_t *pMaxIter)
{
   #define MARGIN 50 /* cycle count adjustment */
   #define SYMBOLS_BTN_FRAMES 90 /* number of symbols between frames */
   #define OVERHEAD 50

   static const uint16_t BSAT_cycles_one_time[] =
   {
      1120, /* DVB-S2 QPSK 1/4 */
      1241, /* DVB-S2 QPSK 1/3 */
      1338, /* DVB-S2 QPSK 2/5 */
      1299, /* DVB-S2 QPSK 1/2 */
      1631, /* DVB-S2 QPSK 3/5 */
      1247, /* DVB-S2 QPSK 2/3 */
      1310, /* DVB-S2 QPSK 3/4 */
      1349, /* DVB-S2 QPSK 4/5 */
      1379, /* DVB-S2 QPSK 5/6 */
      1135, /* DVB-S2 QPSK 8/9 */
      1138, /* DVB-S2 QPSK 9/10 */
      1631, /* DVB-S2 8PSK 3/5 */
      1247, /* DVB-S2 8PSK 2/3 */
      1310, /* DVB-S2 8PSK 3/4 */
      1379, /* DVB-S2 8PSK 5/6 */
      1135, /* DVB-S2 8PSK 8/9 */
      1138, /* DVB-S2 8PSK 9/10 */
      0, /* DVB-S2 16APSK 2/3 */
      0, /* DVB-S2 16APSK 3/4 */
      0, /* DVB-S2 16APSK 4/5 */
      0, /* DVB-S2 16APSK 5/6 */
      0, /* DVB-S2 16APSK 8/9 */
      0, /* DVB-S2 16APSK 9/10 */
      0, /* DVB-S2 32APSK 3/4 */
      0, /* DVB-S2 32APSK 4/5 */
      0, /* DVB-S2 32APSK 5/6 */
      0, /* DVB-S2 32APSK 8/9 */
      0  /* DVB-S2 32APSK 9/10 */
   };

   static const uint16_t BSAT_cycles_per_iter[] =
   {
      1118, /* DVB-S2 QPSK 1/4 */
      1239, /* DVB-S2 QPSK 1/3 */
      1336, /* DVB-S2 QPSK 2/5 */
      1297, /* DVB-S2 QPSK 1/2 */
      1629, /* DVB-S2 QPSK 3/5 */
      1245, /* DVB-S2 QPSK 2/3 */
      1308, /* DVB-S2 QPSK 3/4 */
      1347, /* DVB-S2 QPSK 4/5 */
      1377, /* DVB-S2 QPSK 5/6 */
      1133, /* DVB-S2 QPSK 8/9 */
      1136, /* DVB-S2 QPSK 9/10 */
      1629, /* DVB-S2 8PSK 3/5 */
      1245, /* DVB-S2 8PSK 2/3 */
      1308, /* DVB-S2 8PSK 3/4 */
      1377, /* DVB-S2 8PSK 5/6 */
      1133, /* DVB-S2 8PSK 8/9 */
      1136, /* DVB-S2 8PSK 9/10 */
      0, /* DVB-S2 16APSK 2/3 */
      0, /* DVB-S2 16APSK 3/4 */
      0, /* DVB-S2 16APSK 4/5 */
      0, /* DVB-S2 16APSK 5/6 */
      0, /* DVB-S2 16APSK 8/9 */
      0, /* DVB-S2 16APSK 9/10 */
      0, /* DVB-S2 32APSK 3/4 */
      0, /* DVB-S2 32APSK 4/5 */
      0, /* DVB-S2 32APSK 5/6 */
      0, /* DVB-S2 32APSK 8/9 */
      0  /* DVB-S2 32APSK 9/10 */
   };

   static const uint16_t BSAT_cycles_one_time_short_frame[] =
   {
      305, /* DVB-S2 QPSK 1/4 */
      342, /* DVB-S2 QPSK 1/3 */
      370, /* DVB-S2 QPSK 2/5 */
      315, /* DVB-S2 QPSK 1/2 */
      456, /* DVB-S2 QPSK 3/5 */
      363, /* DVB-S2 QPSK 2/3 */
      332, /* DVB-S2 QPSK 3/4 */
      312, /* DVB-S2 QPSK 4/5 */
      353, /* DVB-S2 QPSK 5/6 */
      351, /* DVB-S2 QPSK 8/9 */
      0,   /* DVB-S2 QPSK 9/10 */
      456, /* DVB-S2 8PSK 3/5 */
      363, /* DVB-S2 8PSK 2/3 */
      332, /* DVB-S2 8PSK 3/4 */
      353, /* DVB-S2 8PSK 5/6 */
      351, /* DVB-S2 8PSK 8/9 */
      0, /* DVB-S2 8PSK 9/10 */
      0, /* DVB-S2 16APSK 2/3 */
      0, /* DVB-S2 16APSK 3/4 */
      0, /* DVB-S2 16APSK 4/5 */
      0, /* DVB-S2 16APSK 5/6 */
      0, /* DVB-S2 16APSK 8/9 */
      0, /* DVB-S2 16APSK 9/10 */
      0, /* DVB-S2 32APSK 3/4 */
      0, /* DVB-S2 32APSK 4/5 */
      0, /* DVB-S2 32APSK 5/6 */
      0, /* DVB-S2 32APSK 8/9 */
      0  /* DVB-S2 32APSK 9/10 */
   };

   static const uint16_t BSAT_cycles_per_iter_short_frame[] =
   {
      298, /* DVB-S2 QPSK 1/4 */
      329, /* DVB-S2 QPSK 1/3 */
      354, /* DVB-S2 QPSK 2/5 */
      297, /* DVB-S2 QPSK 1/2 */
      431, /* DVB-S2 QPSK 3/5 */
      335, /* DVB-S2 QPSK 2/3 */
      301, /* DVB-S2 QPSK 3/4 */
      278, /* DVB-S2 QPSK 4/5 */
      318, /* DVB-S2 QPSK 5/6 */
      313, /* DVB-S2 QPSK 8/9 */
      0,   /* DVB-S2 QPSK 9/10 */
      431, /* DVB-S2 8PSK 3/5 */
      335, /* DVB-S2 8PSK 2/3 */
      301, /* DVB-S2 8PSK 3/4 */
      318, /* DVB-S2 8PSK 5/6 */
      313, /* DVB-S2 8PSK 8/9 */
      0, /* DVB-S2 8PSK 9/10 */
      0, /* DVB-S2 16APSK 2/3 */
      0, /* DVB-S2 16APSK 3/4 */
      0, /* DVB-S2 16APSK 4/5 */
      0, /* DVB-S2 16APSK 5/6 */
      0, /* DVB-S2 16APSK 8/9 */
      0, /* DVB-S2 16APSK 9/10 */
      0, /* DVB-S2 32APSK 3/4 */
      0, /* DVB-S2 32APSK 4/5 */
      0, /* DVB-S2 32APSK 5/6 */
      0, /* DVB-S2 32APSK 8/9 */
      0  /* DVB-S2 32APSK 9/10 */
   };

   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Q_lo, B, Fb, mult_factor;
   const uint16_t *pCyclesOneTime, *pCyclesPerIter;

#define BSAT_AFEC_MIN_FB 20000000
   *pModcod = hChn->actualMode - BSAT_Mode_eDvbs2_Qpsk_1_4 + 1;
   if (hChn->acqSettings.symbolRate < BSAT_AFEC_MIN_FB)
      Fb = BSAT_AFEC_MIN_FB;
   else
      Fb = hChn->acqSettings.symbolRate;

   /* decoder_cycle_count =  INT(0.98*((fecFreq * S)/(B * N * Fb) - MARGIN))
                           + INT((fecFreq * X) / Fb) - V
      where
         fecFreq = LDPC decoder clock rate
         S = frame size (short_frame=16200, long_frame=64800)
         B = 2 (QPSK), or 3 (8PSK)
         N = number of channels per LDPC decoder = 2
         X = symbols between frames
         V = overhead
         Fb = symbol rate
         MARGIN = cycle count adjustment = 50
   */
   if (BSAT_MODE_IS_DVBS2_8PSK(hChn->actualMode))
      B = 3;
   else
      B = 2;

   mult_factor = 98; /* 0.98 */

   BMTH_HILO_32TO64_Mul(hChn->fecFreq >> 1, hChn->bShortFrame ? 16200 : 64800, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, Fb * B * BSAT_G1_CHANNELS_PER_LDPC_DECODER, &Q_hi, &Q_lo);
   *pDecoderCycleCount = (mult_factor * (Q_lo - MARGIN)) / 100;
   BMTH_HILO_32TO64_Mul(hChn->fecFreq >> 1, SYMBOLS_BTN_FRAMES, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, Fb, &Q_hi, &Q_lo);
   *pDecoderCycleCount += (Q_lo - OVERHEAD);

   if (hChn->bShortFrame)
   {
      pCyclesOneTime = BSAT_cycles_one_time_short_frame;
      pCyclesPerIter = BSAT_cycles_per_iter_short_frame;
   }
   else
   {
      pCyclesOneTime = BSAT_cycles_one_time;
      pCyclesPerIter = BSAT_cycles_per_iter;
   }

   /* max_iterations = INT((dec_cyc_count - cycles_one_time)/cycles_per_iter) */
   *pMaxIter = (uint32_t)((*pDecoderCycleCount - (uint32_t)pCyclesOneTime[*pModcod-1]) / (uint32_t)pCyclesPerIter[*pModcod-1]);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecConfig_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecConfig_isr(BSAT_ChannelHandle h)
{
   /* bits 16:0 of AFEC_MODCOD_PARAM_1 */
   static const uint32_t BSAT_sigma_scale[] =
   {
      0x00005574, /* DVB-S2 QPSK 1/4 */
      0x00006f84, /* DVB-S2 QPSK 1/3 */
      0x00007cf4, /* DVB-S2 QPSK 2/5 */
      0x000068e5, /* DVB-S2 QPSK 1/2 */
      0x00005f07, /* DVB-S2 QPSK 3/5 */
      0x00005a88, /* DVB-S2 QPSK 2/3 */
      0x00006348, /* DVB-S2 QPSK 3/4 */
      0x00006cd8, /* DVB-S2 QPSK 4/5 */
      0x000071b8, /* DVB-S2 QPSK 5/6 */
      0x00008748, /* DVB-S2 QPSK 8/9 */
      0x00008378, /* DVB-S2 QPSK 9/10 */
      0x000079d8, /* DVB-S2 8PSK 3/5 */
      0x00008748, /* DVB-S2 8PSK 2/3 */
      0x00009b58, /* DVB-S2 8PSK 3/4 */
      0x0000ba18, /* DVB-S2 8PSK 5/6 */
      0x0000be99, /* DVB-S2 8PSK 8/9 */
      0x0000c549, /* DVB-S2 8PSK 9/10 */
      0x00009fd9, /* DVB-S2 16APSK 2/3 */
      0x0000b859, /* DVB-S2 16APSK 3/4 */
      0x0000ca99, /* DVB-S2 16APSK 4/5 */
      0x0000d899, /* DVB-S2 16APSK 5/6 */
      0x0000faf9, /* DVB-S2 16APSK 8/9 */
      0x000101f9, /* DVB-S2 16APSK 9/10 */
      0x0000f659, /* DVB-S2 32APSK 3/4 */
      0x00011199, /* DVB-S2 32APSK 4/5 */
      0x00012679, /* DVB-S2 32APSK 5/6 */
      0x00015a69, /* DVB-S2 32APSK 8/9 */
      0x00016909  /* DVB-S2 32APSK 9/10 */
   };

   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t modcod, val, dec_cycle_cnt, max_iter, psl_ctl;
   uint32_t pct, thresh, gain, beta, out_pfill;

   BSAT_g1_P_AfecEnableChannel_isrsafe(h, true);

   retCode = BSAT_g1_P_AfecUpdateBlockCount_isrsafe(h); /* need to do this because counters are about to be reset */
   if (retCode != BERR_SUCCESS)
   {
      BDBG_WRN(("BSAT_g1_P_AfecConfig_isr(): BSAT_g1_P_AfecUpdateBlockCount_isrsafe() error 0x%X", retCode));
   }

   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_AFECNX_GLOBAL_RESET, h->channel & 1 ? 0x100 : 0x80); /* afec cfg reset */

   if (hChn->acqSettings.mode != BSAT_Mode_eDvbs2_ACM)
   {
      if (hChn->actualMode <= BSAT_Mode_eDvbs2_Qpsk_2_3)
         val = 0x0000406A;
      else if (hChn->actualMode <= BSAT_Mode_eDvbs2_Qpsk_9_10)
         val = 0x0000506A;
      else if (hChn->actualMode <= BSAT_Mode_eDvbs2_8psk_3_4)
         val = 0x0000606A;
      else
         val = 0x0000606A; /* was 0x0000708C;  too wide, fell out of lock */
      BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, 0x60000, val);
   }
   else
   {
      /* TBD: adjust CLDAFECTL for ACM */
   }

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecGetDecoderParams_isr(h, &modcod, &dec_cycle_cnt, &max_iter));
   BSAT_CHK_RETCODE(BSAT_g1_P_AfecComputeOutPfill_isr(h, modcod, &out_pfill));

   /* BDBG_MSG(("modcod=%d, dec_cycle_cnt=%d, max_iter=%d, out_pfill=%d", modcod, dec_cycle_cnt, max_iter, out_pfill)); */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_MODCOD_PARAM_0, (dec_cycle_cnt << 8) | modcod);

   val = BSAT_sigma_scale[modcod-1] | (max_iter << 20);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_MODCOD_PARAM_1, val);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_CONFIG, 0x01000B01);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_CTRL, 0x00000001);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_BBHDR4, 0x000005E0);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_BBHDR3, 0x0043000C);

   val = (out_pfill & 0x3FFF) | 0xE0000000;
   if (hChn->xportSettings.bTei)
   {
      /* software workaround for DVB-S2 sync byte corruption: disable TEI */
#if ((BCHP_CHIP==7364) || (BCHP_CHIP==7374))
      val |= 0x8000;
#endif
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_SMCFG, val);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_MODCOD_STATS_CONFIG, 0x00000000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_CNTR_CTRL, 0x00000003); /* clear counters */

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetMpcfg_isr(h));

   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_AFEC_CNTR_CTRL, 0x00000006); /* clear counters */
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_AFEC_RST, 2); /* reset data path */

   if (hChn->dvbs2Settings.ctl & BSAT_DVBS2_CTL_DISABLE_PSL)
   {
      disable_psl:
      psl_ctl = 0x50;
   }
   else
   {
      if (hChn->acqSettings.symbolRate > 30000000)
         goto disable_psl;
      if (hDevImpl->networkSpec == BSAT_NetworkSpec_eDefault)
         pct = 372;
      else
         pct = 298;

      thresh = ((max_iter * pct) + 500) / 1000;
      thresh = thresh << 2;

      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_LDPC_PSL_FILTER);
      val &= 0xFFFF001F;
      val |= (thresh << 5);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_PSL_FILTER, val);

      /* set gain and beta */
      if (hChn->acqSettings.symbolRate < 5000000)
      {
         gain = 0x0B;
         beta = 0x0A;
      }
      else
      {
         gain = 0x09;
         beta = 0x07;
      }

      psl_ctl = ((beta << 4) & 0xF0) | 0x05;
      psl_ctl |= ((gain & 0x0F) | (((thresh & 0x0003) << 6) & 0xC0)) << 8;
      psl_ctl |= ((thresh >> 2) & 0xFF) << 16;
      psl_ctl |= ((thresh >> 10) & 0x07) << 24;
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_PSL_CTL, psl_ctl);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_AfecSetOpll_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecSetOpll_isr(BSAT_ChannelHandle h)
{
   static const uint16_t BSAT_NUMBER_OF_BITS[] =
   {
      15928, /* DVB-S2 QPSK 1/4 */
      21328, /* DVB-S2 QPSK 1/3 */
      25648, /* DVB-S2 QPSK 2/5 */
      32128, /* DVB-S2 QPSK 1/2 */
      38608, /* DVB-S2 QPSK 3/5 */
      42960, /* DVB-S2 QPSK 2/3 */
      48328, /* DVB-S2 QPSK 3/4 */
      51568, /* DVB-S2 QPSK 4/5 */
      53760, /* DVB-S2 QPSK 5/6 */
      57392, /* DVB-S2 QPSK 8/9 */
      58112, /* DVB-S2 QPSK 9/10 */
      38608, /* DVB-S2 8PSK 3/5 */
      42960, /* DVB-S2 8PSK 2/3 */
      48328, /* DVB-S2 8PSK 3/4 */
      53760, /* DVB-S2 8PSK 5/6 */
      57392, /* DVB-S2 8PSK 8/9 */
      58112, /* DVB-S2 8PSK 9/10 */
      42960, /* DVB-S2 16APSK 2/3 */
      48328, /* DVB-S2 16APSK 3/4 */
      51568, /* DVB-S2 16APSK 4/5 */
      53760, /* DVB-S2 16APSK 5/6 */
      57392, /* DVB-S2 16APSK 8/9 */
      58112, /* DVB-S2 16APSK 9/10 */
      48328, /* DVB-S2 32APSK 3/4 */
      51568, /* DVB-S2 32APSK 4/5 */
      53760, /* DVB-S2 32APSK 5/6 */
      57392, /* DVB-S2 32APSK 8/9 */
      58112, /* DVB-S2 32APSK 9/10 */
   };

   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t i, lval1, lval2, number_of_bits, number_of_symbols;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   BSAT_Mode mode;

   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
      return BERR_NOT_SUPPORTED;
   else
      mode = hChn->actualMode;

   lval2 = 504; /* 14*36 */
   if (BSAT_MODE_IS_DVBS2_8PSK(mode))
   {
      /* 8PSK */
      lval1 = 21600;
   }
   else if (BSAT_MODE_IS_DVBS2_QPSK(mode))
   {
      /* QPSK */
      lval1 = 32400;
      lval2 = 792; /* 22*36 */
   }
   else if (BSAT_MODE_IS_DVBS2_16APSK(mode))
   {
      /* 16APSK */
      lval1 = 16200;
   }
   else if (BSAT_MODE_IS_DVBS2_32APSK(mode))
   {
      /* 32APSK */
      lval1 = 12960;
   }
   else
   {
      BDBG_WRN(("Invalid mode: %08X\n", mode));
      return BERR_INVALID_PARAMETER;
   }

   i = mode - BSAT_Mode_eDvbs2_Qpsk_1_4;
   number_of_bits = (uint32_t)BSAT_NUMBER_OF_BITS[i];

   if (BSAT_g1_P_AfecIsPilot_isr(h) || (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM))
      lval1 += lval2;
   number_of_symbols = lval1 + 90;

   /* opll_N = final N */
   hChn->opll_N = (number_of_bits >> 1);

   /* opll_D = final D */
   if (hChn->bUndersample)
      hChn->opll_D = number_of_symbols;
   else
      hChn->opll_D = (number_of_symbols << 1);

   BMTH_HILO_32TO64_Mul(hChn->acqSettings.symbolRate, number_of_bits * 2, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, number_of_symbols, &Q_hi, &Q_lo);
   hChn->outputBitrate = (Q_lo + 1) >> 1;

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecResetBlockCount_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecResetBlockCount_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (BSAT_g1_P_IsAfecOn_isrsafe(h))
   {
      BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_AFEC_CNTR_CTRL, 0x04); /* clear BCH counters */
      hChn->badBlocks = 0;
      hChn->corrBlocks = 0;
      hChn->totalBlocks = 0;
      return BERR_SUCCESS;
   }
   return BSAT_ERR_POWERED_DOWN;
}


/******************************************************************************
 BSAT_g1_P_AfecUpdateBlockCount_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecUpdateBlockCount_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t corr, bad, total;

   if (BSAT_g1_P_IsAfecOn_isrsafe(h))
   {
      corr = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_BCH_DECCBLK);
      bad = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_BCH_DECBBLK);
      total = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_BCH_DECNBLK);
      BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_AFEC_CNTR_CTRL, 0x04); /* clear BCH counters */
      hChn->badBlocks += bad;
      hChn->corrBlocks += corr;
      hChn->totalBlocks += total;
      return BERR_SUCCESS;
   }
   return BSAT_ERR_POWERED_DOWN;
}


/******************************************************************************
 BSAT_g1_P_AfecIsFlushDone_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecIsFlushDone_isr(BSAT_ChannelHandle h, bool *pbReady)
{
   uint32_t val, mask;

   if (BSAT_g1_P_IsAfecOn_isrsafe(h))
   {
      mask = (h->channel & 1) ? 0x2 : 0x1;
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFECNX_CHANNEL_EMPTY);
      *pbReady = (val & mask) ? true : false;
      return BERR_SUCCESS;
   }
   else
   {
      *pbReady = true;
      return BERR_SUCCESS;
   }
}


/******************************************************************************
 BSAT_g1_P_AfecIsChannelEnabled_isr()
******************************************************************************/
bool BSAT_g1_P_AfecIsChannelEnabled_isr(BSAT_ChannelHandle h)
{
   uint32_t mask, val;

   if (BSAT_g1_P_IsAfecOn_isrsafe(h))
   {
#ifdef BCHP_AFECNX_0_CHANNEL_ENABLE
      mask = h->channel & 1 ? 2 : 1;
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFECNX_CHANNEL_ENABLE);
#else
      mask = BCHP_AFEC0_0_RST_channel_enable_MASK;
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_RST);
#endif
      return (val & mask) ? true : false;
   }
   else
      return false;
}


/******************************************************************************
 BSAT_g1_P_AfecEnableChannel_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecEnableChannel_isrsafe(BSAT_ChannelHandle h, bool bEnable)
{
#ifdef BCHP_AFECNX_0_CHANNEL_ENABLE
   uint32_t mask;
#endif

   BSAT_DEBUG_FLUSH(BDBG_MSG(("BSAT_g1_P_AfecEnableChannel_isrsafe(%d): bEnable=%d", h->channel, bEnable)));

   if (BSAT_g1_P_IsAfecOn_isrsafe(h) == false)
      return BSAT_ERR_POWERED_DOWN;

   if (!bEnable)
   {
      /* per Sean Lee: set LDPC_LCK_RST=MP_LCK_RST=1 before the flush */
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_AFEC_RST, (BCHP_AFEC0_0_RST_MP_LCK_RST_MASK | BCHP_AFEC0_0_RST_LDPC_LCK_RST_MASK));
   }

#ifdef BCHP_AFECNX_0_CHANNEL_ENABLE
   mask = (h->channel & 1) ? 2 : 1;
   if (bEnable)
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_AFECNX_CHANNEL_ENABLE, mask);
   else
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_AFECNX_CHANNEL_ENABLE, ~mask);
#else
   if (bEnable)
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_AFEC_RST, BCHP_AFEC0_0_RST_channel_enable_MASK);
   else
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_AFEC_RST, ~BCHP_AFEC0_0_RST_channel_enable_MASK);
#endif
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecGetOtherChannelHandle_isrsafe()
******************************************************************************/
BSAT_ChannelHandle BSAT_g1_P_AfecGetOtherChannelHandle_isrsafe(BSAT_ChannelHandle h)
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
 BSAT_g1_P_AfecIsOtherChannelEnabled_isr()
******************************************************************************/
bool BSAT_g1_P_AfecIsOtherChannelEnabled_isr(BSAT_ChannelHandle h)
{
   BSAT_ChannelHandle hOtherChan = BSAT_g1_P_AfecGetOtherChannelHandle_isrsafe(h);
   if (hOtherChan == NULL)
      return false;

   return BSAT_g1_P_AfecIsChannelEnabled_isr(hOtherChan);
}


/******************************************************************************
 BSAT_g1_P_AfecIsOtherChannelBusy_isrsafe()
******************************************************************************/
bool BSAT_g1_P_AfecIsOtherChannelBusy_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_ChannelHandle hOtherChan = BSAT_g1_P_AfecGetOtherChannelHandle_isrsafe(h);
   BSAT_g1_P_ChannelHandle *hOtherChanImpl;
   bool bOtherAfecBusy = false;

   if (hOtherChan == NULL)
      return false;

   hOtherChanImpl = (BSAT_g1_P_ChannelHandle *)(hOtherChan->pImpl);
   if (hOtherChanImpl->bAbortAcq == false)
   {
      if (hOtherChanImpl->acqType == BSAT_AcqType_eDvbs2)
         bOtherAfecBusy = true;
   }

   return bOtherAfecBusy;
}


/******************************************************************************
 BSAT_g1_P_AfecIsOtherChannelFlushFailed_isr()
******************************************************************************/
bool BSAT_g1_P_AfecIsOtherChannelFlushFailed_isr(BSAT_ChannelHandle h)
{
   BSAT_ChannelHandle hOtherChan = BSAT_g1_P_AfecGetOtherChannelHandle_isrsafe(h);
   BSAT_g1_P_ChannelHandle *hOtherChanImpl;

   if (hOtherChan == NULL)
      return false;

   hOtherChanImpl = (BSAT_g1_P_ChannelHandle *)(hOtherChan->pImpl);
   return hOtherChanImpl->bAfecFlushFailed;
}


/******************************************************************************
 BSAT_g1_P_AfecResetChannel_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecResetChannel_isr(BSAT_ChannelHandle h)
{
   if (BSAT_g1_P_IsAfecOn_isrsafe(h))
   {
#ifdef BCHP_AFECNX_CHANNEL_ENABLE
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_RST, 2);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_RST, 0);
#else
      BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_AFEC_RST, 0x2);
#endif
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecWaitForFlushDone_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecWaitForFlushDone_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT nextFunct)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   /* set timeout for flush that was initiated in BSAT_g1_P_AfecAcquire_isr() */
   hChn->count1 = 0;
   hChn->nextFunct = nextFunct;
   return BSAT_g1_P_AfecWaitForFlushDone1_isr(h);
}


/******************************************************************************
 BSAT_g1_P_AfecWaitForFlushDone1_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecWaitForFlushDone1_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   bool bReady;
   BERR_Code retCode;

   retCode = BSAT_g1_P_AfecIsFlushDone_isr(h, &bReady);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_WRN(("BSAT_g1_P_AfecIsFlushDone_isr() error 0x%X", retCode));
      goto next_funct;
   }

   if (bReady)
   {
      hChn->bAfecFlushFailed = false;
      BSAT_DEBUG_FLUSH(BDBG_WRN(("AFEC%d flush done, count1=%d", h->channel, hChn->count1)));

      next_funct:
      return hChn->nextFunct(h);
   }

   hChn->count1++;
   if (hChn->count1 < 100)
   {
      retCode = BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaudUsec, 500, BSAT_g1_P_AfecWaitForFlushDone1_isr);
      if (retCode != BERR_SUCCESS)
      {
         BDBG_WRN(("BSAT_g1_P_AfecWaitForFlushDone1_isr(): BSAT_g1_P_EnableTimer_isr() error 0x%X", retCode));
         goto next_funct;
      }
   }
   else
   {
      BSAT_DEBUG_FLUSH(BDBG_WRN(("AFEC%d flush timeout", h->channel)));
      hChn->bAfecFlushFailed = true;
      if (BSAT_g1_P_AfecIsOtherChannelEnabled_isr(h) == false)
         BSAT_g1_P_AfecPowerDown_isrsafe(h);
      else if (BSAT_g1_P_AfecIsOtherChannelFlushFailed_isr(h))
      {
         BSAT_DEBUG_FLUSH(BDBG_WRN(("AFEC%d power down due to other channel flush failed", h->channel)));
         BSAT_g1_P_AfecPowerDown_isrsafe(h);
      }
      hChn->reacqCause = BSAT_ReacqCause_eAfecFlushFailed;
      goto next_funct;
   }

   return retCode;
}

#endif /* BSAT_EXCLUDE_AFEC */
