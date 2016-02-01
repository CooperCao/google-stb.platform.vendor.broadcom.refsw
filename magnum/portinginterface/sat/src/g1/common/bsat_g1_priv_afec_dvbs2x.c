/******************************************************************************
*    (c)2011-2013 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*
* Revision History:
*
* $brcm_Log: $
*
*****************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"


/* #define BSAT_DEBUG_ACM */


#if (defined(BSAT_EXCLUDE_AFEC) || !defined(BSAT_HAS_DVBS2X))
#error "invalid setting for BSAT_EXCLUDE_AFEC/BSAT_HAS_DVBS2X"
#endif

BDBG_MODULE(bsat_g1_priv_afec_dvbs2x);


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
BERR_Code BSAT_g1_P_AfecAcquire0_isr(BSAT_ChannelHandle h);
void BSAT_g1_P_AfecSetCmaModulus_isr(BSAT_ChannelHandle h);
void BSAT_g1_P_AfecInitEqTaps_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecConfigPdLut_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecConfigPdLut0_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecConfigPdLut1_isr(BSAT_ChannelHandle h, uint8_t lutMode);
BERR_Code BSAT_g1_P_AfecSetVlcGain_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecSetDecoderParams_isr(BSAT_ChannelHandle h);


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

   /* disable soft pd tables until after HP locks */
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

   if (hChn->acqSettings.options & BSAT_ACQ_CHAN_BOND)
   {
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_CG_PMCG_CTL, BCHP_SDS_CG_0_PMCG_CTL_xpt_bclk_en_MASK);
   }

   if (hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_ENABLED)
      BSAT_g1_P_AfecInitEqTaps_isr(h);

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecPowerUp_isr(h));
   BSAT_g1_P_AfecResetChannel_isr(h);
   BSAT_CHK_RETCODE(BSAT_g1_P_GetAfecClock_isrsafe(h, &(hChn->fecFreq)));

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_RAMP_DELTA_UP, 0x80000003);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_RAMP_DELTA_DOWN, 0x80000003);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_RAMP_STEP_UP, 0x00000100);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_RAMP_STEP_DOWN, 0x00000010);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_RAMP_N, 0x0000000a);

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecConfigPdLut_isr(h)); /* this eventually calls BSAT_g1_P_AfecAcquire1_isr */

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_AfecAcquire1_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecAcquire1_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val;

   /* set ACM_MODCOD_OVERIDE */
   if (hChn->acqSettings.mode != BSAT_Mode_eDvbs2_ACM)
   {
      uint32_t acm_check = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK);

      if (BSAT_MODE_IS_DVBS2X(hChn->actualMode))
      {
         /* DVB-S2X */
         uint8_t plscode;
         BSAT_CHK_RETCODE(BSAT_g1_P_GetDvbs2Plscode_isrsafe(hChn->actualMode, &plscode));
         val = (uint32_t)plscode | (acm_check & 1);
      }
      else
      {
         /* DVB-S2 */
         val = ((hChn->actualMode - BSAT_Mode_eDvbs2_Qpsk_1_4) + 1) << 2;
         val |= (acm_check & 0x3);
      }
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_ACM_MODCOD_OVERIDE, val | BCHP_AFEC_0_ACM_MODCOD_OVERIDE_MODCOD_OVERIDE_MASK);
   }
   else
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_AFEC_ACM_MODCOD_OVERIDE, ~BCHP_AFEC_0_ACM_MODCOD_OVERIDE_MODCOD_OVERIDE_MASK);

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetEqsftctl_isrsafe(h));

   if (hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_ENABLED)
   {
      BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetPldctl_isr(h));
      BSAT_CHK_RETCODE(BSAT_g1_P_ConfigPlc_isr(h, true)); /* set acquisition plc to the non-scan plc value */
      BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetVlctl_isr(h));
      BSAT_CHK_RETCODE(BSAT_g1_P_AfecConfigEq_isr(h));

      if (BSAT_MODE_IS_DVBS2_QPSK(hChn->actualMode))
      {
         /* we need to reprogram HDQPSK because we assumed 8PSK when acquiring in scan mode */
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HDQPSK, 0x01000000);
      }
   }
   else
   {
      if (hChn->acqSettings.options & BSAT_ACQ_PILOT_SCAN_ENABLE)
      {
         BSAT_CHK_RETCODE(BSAT_g1_P_AfecConfigEq_isr(h));
      }
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
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE, BCHP_SDS_HP_0_HPOVERRIDE_SNOREOV_MASK); /* override SNORE reset */

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetVlcGain_isr(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_SetAgcTrackingBw_isr(h));

   val = 0x34;
   if ((hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM) || BSAT_g1_P_AfecIsPilot_isr(h))
      val |= BCHP_SDS_CL_0_CLFBCTL_fb_mode_MASK;
   if (BSAT_g1_P_AfecIs8psk_isr(h))
      val |= BCHP_SDS_CL_0_CLFBCTL_fb_err_mode_MASK;
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLFBCTL, 0xFFFF0000, val);

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecConfigSnr_isr(h));

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecConfig_isr(h));

   if (hChn->xportSettings.bOpllBypass == false)
      BSAT_g1_P_PowerUpOpll_isr(h);

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetOpll_isr(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_ConfigOif_isr(h));

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01);
   if (hChn->bUndersample && ((val & 0x300) == 0x300))
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_AFEC_ACM_MISC_0, ~BCHP_AFEC_0_ACM_MISC_0_INTERFACE_OUT_MASK);
   else
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_AFEC_ACM_MISC_0, BCHP_AFEC_0_ACM_MISC_0_INTERFACE_OUT_MASK);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_RST, 0x00070100);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_INTR_CTRL2_CPU_CLEAR, 0xFFFFFFFF);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_RST, 0);

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
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);

   /* BDBG_MSG(("BSAT_g1_P_AfecReacquire_isr(%d)", h->channel)); */
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
      goto reacquire;

#ifdef BSAT_DEBUG_ACM
   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
   {
      uint32_t ldpc_status, lock_status;
      ldpc_status = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_LDPC_STATUS);
      lock_status = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_LOCK_STATUS);
      BDBG_ERR(("BSAT_g1_P_AfecReacquire_isr: AFEC_LDPC_STATUS=0x%X, AFEC_LOCK_STATUS=0x%X", ldpc_status, lock_status));
   }
#endif

   BSAT_g1_P_AfecResetChannel_isr(h);
   BSAT_g1_P_AfecPowerDown_isrsafe(h);

   reacquire:
   return BSAT_g1_P_Reacquire_isr(h);
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
   else if (BSAT_MODE_IS_DVBS2X(hChn->actualMode))
   {
      i = hChn->actualMode - BSAT_Mode_eDvbs2x_Qpsk_13_45;
      mask = (1 << i);
      if ((mask & hChn->dvbs2xSettings.scanModes) == 0)
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
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_INTR_RAW_STS0) & 0x180;
   return (val == 0x80) ? true : false;
}


/******************************************************************************
 BSAT_g1_P_AfecIsMpegLocked_isr()
******************************************************************************/
bool BSAT_g1_P_AfecIsMpegLocked_isr(BSAT_ChannelHandle h)
{
   uint32_t val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_LDPC_STATUS);
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

#ifdef BSAT_DEBUG_ACM
   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
   {
      uint32_t ldpc_status, lock_status;
      ldpc_status = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_LDPC_STATUS);
      lock_status = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_LOCK_STATUS);
      BDBG_ERR(("BSAT_g1_P_AfecOnLock_isr: AFEC_LDPC_STATUS=0x%X, AFEC_LOCK_STATUS=0x%X", ldpc_status, lock_status));
   }
#endif

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

#ifdef BSAT_DEBUG_ACM
   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
   {
      uint32_t ldpc_status, lock_status;
      ldpc_status = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_LDPC_STATUS);
      lock_status = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_LOCK_STATUS);
      BDBG_ERR(("BSAT_g1_P_AfecOnLostLock_isr: AFEC_LDPC_STATUS=0x%X, AFEC_LOCK_STATUS=0x%X", ldpc_status, lock_status));
   }
#endif

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
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val;

#ifdef BSAT_DEBUG_ACM
   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
   {
      uint32_t ldpc_status, lock_status;
      ldpc_status = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_LDPC_STATUS);
      lock_status = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_LOCK_STATUS);
      BDBG_ERR(("BSAT_g1_P_AfecOnStableLock_isr: AFEC_LDPC_STATUS=0x%X, AFEC_LOCK_STATUS=0x%X", ldpc_status, lock_status));
   }
#endif

   if (BSAT_g1_P_AfecIsMpegLocked_isr(h) == false)
      hChn->count1 = 1;

   if (hChn->bPlcTracking == false)
   {
      if (hChn->configParam[BSAT_g1_CONFIG_EQ_CTL] & BSAT_g1_CONFIG_EQ_CTL_OVERRIDE_MU)
      {
         /* override the default settings for ffe_main_mu and ffe_mu */
         val = (hChn->configParam[BSAT_g1_CONFIG_EQ_CTL] & (BSAT_g1_CONFIG_EQ_CTL_TRK_FFE_MU_MASK | BSAT_g1_CONFIG_EQ_CTL_TRK_FFE_MAIN_MU_MASK));
      }
      else if (BSAT_g1_P_AfecIsPilot_isr(h))
         val = 0x66000000;
      else
         val = 0x99000000;
      BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 0x00FFFFFF, val); /* set tracking mu */
      BSAT_g1_P_ConfigPlc_isr(h, false); /* set tracking PLC */
   }

   if (hChn->configParam[BSAT_g1_CONFIG_TRK_DAFE_CTL])
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, hChn->configParam[BSAT_g1_CONFIG_TRK_DAFE_CTL] & BSAT_g1_CONFIG_DAFE_CTL_CLDAFECTL);
   else if (hChn->bEnableFineFreqEst == false)
   {
      /* narrow DAFE loop bw for pilot off */
      if (hDev->sdsRevId < 0x74)
      {
         /* BCM45308-A0 and earlier */
         if (BSAT_MODE_IS_DVBS2_EXTENDED(hChn->actualMode) || BSAT_MODE_IS_DVBS2X(hChn->actualMode))
            val = 0x00005032; /* Xiaofen: dafe_int_scale=3 (2^-16), dafe_lin_scale=1 */
         else
            val = 0x00005056; /* dafe_int_scale=5, dafe_lin_scale=3 (2^-12) */
         BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, 0x60000, val);
      }
      else
      {
         /* BCM45308-B0 and later */
         if (BSAT_MODE_IS_DVBS2_EXTENDED(hChn->actualMode) || BSAT_MODE_IS_DVBS2X(hChn->actualMode))
            val = 0x01305032; /* dafe_int_scale=3 (2^-16), dafe_lin_scale=1 */
         else
            val = 0x03505056; /* dafe_int_scale=5 (2^-12), dafe_lin_scale=3 */
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, val);
      }
   }

   /* set tracking baud loop bw */
   BSAT_CHK_RETCODE(BSAT_g1_P_SetBaudBw_isr(h, hChn->acqSettings.symbolRate / 400, 4));

   if (hChn->bEverStableLock == false)
   {
      BSAT_CHK_RETCODE(BSAT_g1_P_AfecResetBlockCount_isrsafe(h));

      if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
      {
         int i;
         uint32_t stat, shift;

         /* program the stream IDs in MPEG_CTL1/MPEG_CTL2 */
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_MPEG_CTL1, 0);
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_MPEG_CTL2, 0);
         val = 0;

         for (i = 0; i < 8; i++)
         {
            BSAT_g1_P_GetStreamRegStat_isrsafe(h, i, &stat);
            if ((stat & BCHP_AFEC_0_BCH_STREAM_ID0_STAT_avail_MASK) == 0)
            {
               uint8_t streamId = (uint8_t)((stat & BCHP_AFEC_0_BCH_STREAM_ID0_STAT_stream_ID_MASK) >> BCHP_AFEC_0_BCH_STREAM_ID0_STAT_stream_ID_SHIFT);
               shift = (i & 0x3) * 8;
               val |= (streamId << shift);
            }
            if ((i == 3) && (val != 0))
            {
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_MPEG_CTL1, val);
               val = 0;
            }
            else if ((i == 7) && (val != 0))
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_MPEG_CTL2, val);
         }
      }
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

#ifdef BSAT_HAS_ACM
   if (hChn->acqSettings.mode != BSAT_Mode_eDvbs2_ACM)
#endif
   {
      if (BSAT_g1_P_AfecIsMpegLocked_isr(h) == false)
      {
         hChn->count1++;
         if (hChn->count1 > 2)
         {
            /* MP is not locked, so force reacquire */
            BDBG_MSG(("BSAT_g1_P_AfecOnMonitorLock_isr(%d): mpeg not locked, reacquiring...", h->channel));
            hChn->bForceReacq = true;
         }
      }
      else
         hChn->count1 = 0;
   }
#ifdef BSAT_HAS_ACM
   else
   {
      uint32_t val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_LOCK_STATUS);
      if ((val & BCHP_AFEC_0_LOCK_STATUS_LDPC_LOCK_PLS_MASK) == 0)
      {
         /* all plscodes are not locked */
         BDBG_MSG(("BSAT_g1_P_AfecOnMonitorLock_isr(%d): AFEC_LOCK_STATUS=0x%X, reacquiring...", h->channel, val));
         hChn->bForceReacq = true;
      }
   }
#endif

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

   if ((hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_MASK) == BSAT_DVBS2_SCAN_STATE_ENABLED)
   {
      /* scan in progress */
      return false;
   }
   else if ((BSAT_MODE_IS_DVBS2_8PSK(hChn->actualMode)) || (BSAT_MODE_IS_DVBS2X_8PSK(hChn->actualMode)))
      return true;
   return false;
}


/******************************************************************************
 BSAT_g1_P_AfecSetPldctl_isr() - hp input scaler values are 70% backed off from threshold
******************************************************************************/
BERR_Code BSAT_g1_P_AfecSetPldctl_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, max_pilot;
   BSAT_Mode mode = hChn->actualMode;

   if (BSAT_MODE_IS_DVBS2_QPSK(mode) || BSAT_MODE_IS_DVBS2X_QPSK(mode))
      val = 0x7800;
   else if (BSAT_MODE_IS_DVBS2_8PSK(mode) || BSAT_MODE_IS_DVBS2X_8PSK(mode))
      val = 0x5600;
   else if (BSAT_MODE_IS_DVBS2_16APSK(mode))
      val = 0x4C00;
   else if (BSAT_MODE_IS_DVBS2_32APSK(mode))
      val = 0x4500;
   else if (BSAT_MODE_IS_DVBS2X_8APSK(mode) || BSAT_MODE_IS_DVBS2X_16APSK(mode))
      val = 0x4800;
   else if (BSAT_MODE_IS_DVBS2X_32APSK(mode))
      val = 0x4200;
   else /* scan mode */
      val = 0x5000;

   val |= 0x14;

   if (hChn->dvbs2Settings.ctl & BSAT_DVBS2_CTL_GOLD_CODE_SEEK)
   {
      val |= BCHP_SDS_EQ_0_PLDCTL_enable_seek_MASK;
      if (BSAT_MODE_IS_DVBS2_QPSK(mode) || BSAT_MODE_IS_DVBS2X_QPSK(mode))
         max_pilot = 15;
      else if (BSAT_MODE_IS_DVBS2_8PSK(mode) || BSAT_MODE_IS_DVBS2X_8APSK(mode) || BSAT_MODE_IS_DVBS2X_8PSK(mode))
         max_pilot = 14;
      else if (BSAT_MODE_IS_DVBS2_16APSK(mode) || BSAT_MODE_IS_DVBS2X_16APSK(mode))
         max_pilot = 11;
      else /* 32APSK */
         max_pilot = 8;
      val |= (max_pilot << BCHP_SDS_EQ_0_PLDCTL_max_pilot_SHIFT);
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
   BSAT_Mode mode = hChn->actualMode;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL);
   ffe_main_tap = (val >> 16) & 0x1F;

   for (i = 0; i < 24; i++)
   {
      if (i == ffe_main_tap)
      {
         if (hChn->configParam[BSAT_g1_CONFIG_EQ_CTL] & BSAT_g1_CONFIG_EQ_CTL_OVERRIDE_F0B)
         {
            val = (hChn->configParam[BSAT_g1_CONFIG_EQ_CTL] & BSAT_g1_CONFIG_EQ_CTL_MAIN_TAP_F0B_MASK) >> 8;
            BDBG_WRN(("override eq main tap to 0x%X\n", val));
         }
         else if (BSAT_MODE_IS_DVBS2_QPSK(mode) || BSAT_MODE_IS_DVBS2X_QPSK(mode))
            val = 0x25;
         else if (BSAT_MODE_IS_DVBS2_8PSK(mode) || BSAT_MODE_IS_DVBS2X_8APSK(mode))
            val = 0x36;
         else if (BSAT_MODE_IS_DVBS2X_16APSK(mode) || BSAT_MODE_IS_DVBS2X_32APSK(mode) || BSAT_MODE_IS_DVBS2_32APSK(mode))
            val = 0x78;
         else
            val = 0x38;
      }
      else
         val = 0x00;

      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQCFAD, 0x40 | i);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_F0B, (val & 0xFF) << 24);
   }
}


/******************************************************************************
 BSAT_g1_P_AfecSetEqsftctl_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecSetEqsftctl_isrsafe(BSAT_ChannelHandle h)
{
#if defined(BCHP_SDS_EQ_0_EQSFT_CTL)
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val, ctl, acm_check, step = 1;
   uint8_t pls;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQSFT_CTL);
   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
   {
      switch (hChn->acmSettings.softDecisions.captureMode)
      {
         case BSAT_AcmSoftDecisionCapture_eMode:
            retCode = BSAT_g1_P_GetDvbs2Plscode_isrsafe(hChn->acmSettings.softDecisions.param.mode, &pls);
            if (retCode != BERR_SUCCESS)
               goto no_filtering;
            else
            {
               acm_check = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK);
               pls |= (acm_check & 1);
               ctl = (val & 0xFFFF0000) | 0x00001000;
               ctl |= (pls & 0xFF);
            }
            break;

         case BSAT_AcmSoftDecisionCapture_eModulation:
            ctl = (val & 0xFFFF0000) | 0x00002000;
            ctl |= (((uint32_t)hChn->acmSettings.softDecisions.param.modulation & 0x7) << BCHP_SDS_EQ_0_EQSFT_CTL_eqsft_filter_modulation_SHIFT);
            break;

         default:
            goto no_filtering;
      }
      goto write_ctl;
   }
   else
   {
      no_filtering:
      /* force no filtering if not in ACM mode */
      ctl = 0;

      write_ctl:
      ctl |= (step << BCHP_SDS_EQ_0_EQSFT_CTL_eqsft_filter_step_SHIFT);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQSFT_CTL, ctl);
   }

   return retCode;
#else
   BSTD_UNUSED(h);
   return BERR_SUCCESS;
#endif
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
   if (hChn->configParam[BSAT_g1_CONFIG_EQ_CTL] & BSAT_g1_CONFIG_EQ_CTL_OVERRIDE_MU)
   {
      /* override the default settings for ffe_main_mu and ffe_mu */
      val |= (hChn->configParam[BSAT_g1_CONFIG_EQ_CTL] & (BSAT_g1_CONFIG_EQ_CTL_ACQ_FFE_MU_MASK | BSAT_g1_CONFIG_EQ_CTL_ACQ_FFE_MAIN_MU_MASK)) << 8;
      BDBG_WRN(("override eq mu 0x%08X\n", val & 0xFF000000));
   }
   else if (BSAT_g1_P_AfecIsPilot_isr(h) == false)
      val |= 0x77000000; /* pilot off */
   else
      val |= 0x22000000; /* pilot on */
   val |= 0x00060720;
   val &= ~0x500;  /* unfreeze ffe and ffe main tap */
   if ((hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM) || (BSAT_g1_P_AfecIsPilot_isr(h)))
      val |= 0x18; /* ffe_update_mode=3 */
   else
      val |= 0x10;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, val);

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL);
   val &= ~0x0038401F;
   val |= 0x00144000;
   if (BSAT_g1_P_AfecIs8psk_isr(h))
      val |= 0x401A; /* err_mode, sym_mode, dvbs2_8psk_mapping */
   else if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
   {
     val &= ~0x2F; /* cl_soft_slice_sel=0, err_mode=0, sym_mode=0 */
     val |= 0x4010;
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, val);
   BSAT_g1_P_AfecSetCmaModulus_isr(h);
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
      val |= 0x00000008; /* clear bit 17 (vlc_soft_insel), set bit 3 (vgain_sel) */
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
         if (BSAT_MODE_IS_DVBS2_QPSK(hChn->actualMode) || BSAT_MODE_IS_DVBS2X_QPSK(hChn->actualMode))
            goto ldpc_set_hd_1;
      }
      goto ldpc_set_hd_0;
   }
   else if (BSAT_MODE_IS_DVBS2_QPSK(hChn->actualMode) || BSAT_MODE_IS_DVBS2X_QPSK(hChn->actualMode))
   {
      ldpc_set_hd_1: /* QPSK */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HDQPSK, 0x01000000);
   }
   else
   {
      ldpc_set_hd_0:
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HDQPSK, 0x016A0000);
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
   if ((hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM) || BSAT_g1_P_AfecIsPilot_isr(h))
   {
      val |= BCHP_SDS_EQ_0_PILOTCTL_pilot_mode_MASK;
      val |= BCHP_SDS_EQ_0_PILOTCTL_pilot_update_mode_MASK;
   }
   if ((hChn->acqSettings.mode != BSAT_Mode_eDvbs2_ACM) && (BSAT_g1_P_AfecIsPilot_isr(h) == false))
      val |= BCHP_SDS_EQ_0_PILOTCTL_phase_adj_en_MASK;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_PILOTCTL, val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecConfigPdLut_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecConfigPdLut_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_Mode mode = hChn->actualMode;
   uint8_t lutMode;

   /* determine which lut to use */
   if (BSAT_MODE_IS_DVBS2_QPSK(mode) || BSAT_MODE_IS_DVBS2X_QPSK(mode))
      lutMode = 4;
   else if (BSAT_MODE_IS_DVBS2_8PSK(mode) || BSAT_MODE_IS_DVBS2X_8PSK(mode))
      lutMode = 8;
   else
   {
      /* 8/16/32 APSK */
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, ~0x0000020); /* cl_soft_slice_sel=0 (select hard slicer) */
      return BSAT_g1_P_AfecAcquire1_isr(h);
   }

   return BSAT_g1_P_AfecConfigPdLut1_isr(h, lutMode);
}


/******************************************************************************
 BSAT_g1_P_AfecConfigPdLut1_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecConfigPdLut1_isr(BSAT_ChannelHandle h, uint8_t lutMode)
{
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t i, lupd;
   const uint8_t *pLut;

   pLut = BSAT_g1_P_AfecGetPdLut_isr(lutMode);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_LUPA, 0x80000000) /* enable PD LUT memory micro access */;
   BSAT_g1_P_EnableTimer(h, BSAT_TimerSelect_eBaud, 2, NULL);  /* wait 2 bclks to ensure write occurs in HW */

   for (i = 0; i < 1024; i++)
   {
      lupd = *pLut++ << 16;
      lupd |= (*pLut++ << 8);
      lupd |= *pLut++;
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_LUPD, lupd << 8);
      BSAT_g1_P_EnableTimer(h, BSAT_TimerSelect_eBaud, 2, NULL);  /* wait 2 bclks to ensure write occurs in HW */
   }

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_LUPA, 0) /* disable PD LUT memory micro access */;
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, 0x00000020); /* select soft slicer */

   return BSAT_g1_P_AfecAcquire1_isr(h);
}


/******************************************************************************
 BSAT_g1_P_AfecSetCmaModulus_isr()
******************************************************************************/
void BSAT_g1_P_AfecSetCmaModulus_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   int i;
   uint32_t cma_modulus, val;

   /* cma modulus values are scaled 2^14 */
   static const uint32_t cma_modulus_dvbs2_16apsk[] =
   {
      20518, /* 1.2523 (BSAT_Mode_eDvbs2_16apsk_2_3) */
      20259, /* 1.2365 (BSAT_Mode_eDvbs2_16apsk_3_4) */
      20157, /* 1.2303 (BSAT_Mode_eDvbs2_16apsk_4_5) */
      20102, /* 1.2269 (BSAT_Mode_eDvbs2_16apsk_5_6) */
      19985, /* 1.2198 (BSAT_Mode_eDvbs2_16apsk_8_9) */
      19948  /* 1.2175 (BSAT_Mode_eDvbs2_16apsk_9_10) */
   };

   static const uint32_t cma_modulus_dvbs2_32apsk[] =
   {
      23156, /* 1.4133 (BSAT_Mode_eDvbs2_32apsk_3_4) */
      22712, /* 1.3862 (BSAT_Mode_eDvbs2_32apsk_4_5) */
      22472, /* 1.3716 (BSAT_Mode_eDvbs2_32apsk_5_6) */
      22086, /* 1.3480 (BSAT_Mode_eDvbs2_32apsk_8_9) */
      22048, /* 1.3457 (BSAT_Mode_eDvbs2_32apsk_9_10) */
   };

   static const uint32_t cma_modulus_dvbs2x_8apsk[] =
   {
      22736, /* 1.3877 (BSAT_Mode_eDvbs2x_8apsk_5_9_L) */
      22641, /* 1.3819 (BSAT_Mode_eDvbs2x_8apsk_26_45_L) */
   };

   static const uint32_t cma_modulus_dvbs2x_16apsk[] =
   {
      23411, /* 1.4289 (BSAT_Mode_eDvbs2x_16apsk_1_2_L) */
      23411, /* 1.4289 (BSAT_Mode_eDvbs2x_16apsk_8_15_L) */
      23411, /* 1.4289 (BSAT_Mode_eDvbs2x_16apsk_5_9_L) */
      20857, /* 1.2730 (BSAT_Mode_eDvbs2x_16apsk_26_45) */
      20857, /* 1.2730 (BSAT_Mode_eDvbs2x_16apsk_3_5) */
      23411, /* 1.4289 (BSAT_Mode_eDvbs2x_16apsk_3_5_L) */
      20749, /* 1.2664 (BSAT_Mode_eDvbs2x_16apsk_28_45) */
      20478, /* 1.2499 (BSAT_Mode_eDvbs2x_16apsk_23_36) */
      23411, /* 1.4289 (BSAT_Mode_eDvbs2x_16apsk_2_3_L) */
      20478, /* 1.2499 (BSAT_Mode_eDvbs2x_16apsk_25_36) */
      20259, /* 1.2365 (BSAT_Mode_eDvbs2x_16apsk_13_18) */
      20804, /* 1.2698 (BSAT_Mode_eDvbs2x_16apsk_7_9) */
      20555  /* 1.2546 (BSAT_Mode_eDvbs2x_16apsk_77_90) */
   };

   static const uint32_t cma_modulus_dvbs2x_32apsk[] =
   {
      23714, /* 1.4474 (BSAT_Mode_eDvbs2x_32apsk_2_3_L) */
      24275, /* 1.4816 (BSAT_Mode_eDvbs2x_32apsk_32_45) */
      24447, /* 1.4921 (BSAT_Mode_eDvbs2x_32apsk_11_15) */
      23654  /* 1.4437 (BSAT_Mode_eDvbs2x_32apsk_7_9) */
   };

   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
   {
      cma_modulus = 0x4000;
   }
   else if (BSAT_MODE_IS_DVBS2_16APSK(hChn->actualMode))
   {
      i = hChn->actualMode - BSAT_Mode_eDvbs2_16apsk_2_3;
      cma_modulus = cma_modulus_dvbs2_16apsk[i];
   }
   else if (BSAT_MODE_IS_DVBS2_32APSK(hChn->actualMode))
   {
      i = hChn->actualMode - BSAT_Mode_eDvbs2_32apsk_3_4;
      cma_modulus = cma_modulus_dvbs2_32apsk[i];
   }
   else if (BSAT_MODE_IS_DVBS2X_8APSK(hChn->actualMode))
   {
      i = hChn->actualMode - BSAT_Mode_eDvbs2x_8apsk_5_9_L;
      cma_modulus = cma_modulus_dvbs2x_8apsk[i];
   }
   else if (BSAT_MODE_IS_DVBS2X_16APSK(hChn->actualMode))
   {
      i = hChn->actualMode - BSAT_Mode_eDvbs2x_16apsk_1_2_L;
      cma_modulus = cma_modulus_dvbs2x_16apsk[i];
   }
   else if (BSAT_MODE_IS_DVBS2X_32APSK(hChn->actualMode))
   {
      i = hChn->actualMode - BSAT_Mode_eDvbs2x_32apsk_2_3_L;
      cma_modulus = cma_modulus_dvbs2x_32apsk[i];
   }
   else
   {
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, ~BCHP_SDS_EQ_0_EQMISCCTL_cma_do_MASK); /* cma_do=0 for non 8/16/32APSK */
      return;
   }

   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, BCHP_SDS_EQ_0_EQMISCCTL_cma_do_MASK); /* override default cma modulus */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_CMATH, 0);
   val = (cma_modulus << 16) | cma_modulus;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_CMA, val);
}


/******************************************************************************
 BSAT_g1_P_AfecSetAcmVlcGain_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecSetAcmVlcGain_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   int i;

   static const uint8_t vcm_vlc_gain[96] =
   {
      64, 53, 49, 47, 54, 71, 79, 76, 75, 74, 72, 71,
      73, 71, 69, 68, 76, 75, 77, 76, 67, 67, 66, 74,
      74, 74, 66, 65, 65, 64, 64, 64, 64, 64, 52, 45,
      42, 47, 74, 81, 79, 79, 72, 71, 71, 70, 70, 79,
      69, 69, 77, 77, 76, 76, 66, 67, 91, 67, 66, 66,
      66, 66, 66, 65, 66, 65, 66, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 54, 53, 50, 55, 84, 77, 77, 75,
      73, 70, 72, 71, 70, 69, 68, 67, 66, 69, 66, 65
   };

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_MGAINA, BCHP_SDS_EQ_0_MGAINA_mgain_acc_en_MASK);
   for (i = 0; i < 96; i++)
   {
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_MGAIND, (uint32_t)vcm_vlc_gain[i]);
   }
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_EQ_MGAINA, ~BCHP_SDS_EQ_0_MGAINA_mgain_acc_en_MASK);

   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_EQ_VLCTL, ~BCHP_SDS_EQ_0_VLCTL_vlc_soft_insel_MASK);
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_EQ_VLCTL, BCHP_SDS_EQ_0_VLCTL_vgain_sel_MASK);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecSetVlcGain_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecSetVlcGain_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t vlc_gain, val;

   if (hChn->configParam[BSAT_g1_CONFIG_VLC_GAIN] != 0)
   {
      val = hChn->configParam[BSAT_g1_CONFIG_VLC_GAIN];
      goto write_vlc_gain;
   }

   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
      return BSAT_g1_P_AfecSetAcmVlcGain_isr(h);

   if (BSAT_g1_P_AfecIsPilot_isr(h) == false)
   {
      /* pilot off modes */
      switch (hChn->actualMode)
      {
         /* DVB S2X 16APSK 8+8 constellation, pilot off mode */
         case BSAT_Mode_eDvbs2x_16apsk_1_2_L:  /* pls=148 */
         case BSAT_Mode_eDvbs2x_16apsk_8_15_L: /* pls=150 */
            vlc_gain = 15872; /*0x3e00*/
            break;

         case BSAT_Mode_eDvbs2x_16apsk_5_9_L:  /* pls=152 */
            vlc_gain = 15104; /*0x3b00*/
            break;

         case BSAT_Mode_eDvbs2x_16apsk_3_5_L:  /* pls=158 */
            vlc_gain = 12544; /*0x3100*/
            break;

         case BSAT_Mode_eDvbs2x_16apsk_2_3_L:  /* pls=164 */
            vlc_gain = 11264; /*0x2c00*/
            break;

         default:
            goto continue1;

      }

      goto continue2;
   }

   /* vlc_gain scaled 2^13 */
   continue1:
   switch (hChn->actualMode)
   {
      /* DVB S2 16APSK */
      case BSAT_Mode_eDvbs2_16apsk_2_3:
         vlc_gain = 9783; /* 1.19418 */
         break;
      case BSAT_Mode_eDvbs2_16apsk_3_4:
         vlc_gain = 9646; /* 1.17738 */
         break;

      case BSAT_Mode_eDvbs2_16apsk_4_5:
         vlc_gain = 8509; /* 1.03869 */
         break;

      case BSAT_Mode_eDvbs2_16apsk_5_6:
         vlc_gain = 8471; /* 1.03394 */
         break;

      case BSAT_Mode_eDvbs2_16apsk_8_9:
         vlc_gain = 8400; /* 1.02538 */
         break;

      case BSAT_Mode_eDvbs2_16apsk_9_10:
         vlc_gain = 9438; /* 1.15204 */
         break;

      /* DVB S2 32APSK */
      case BSAT_Mode_eDvbs2_32apsk_3_4:
         vlc_gain = 9459; /* 1.15461 */
         break;

      case BSAT_Mode_eDvbs2_32apsk_4_5:
         vlc_gain = 9414; /* 1.14907 */
         break;

      case BSAT_Mode_eDvbs2_32apsk_5_6:
         vlc_gain = 8344; /* 1.01849 */
         break;

      case BSAT_Mode_eDvbs2_32apsk_8_9:
         vlc_gain = 8302; /* 1.01340 */
         break;

      case BSAT_Mode_eDvbs2_32apsk_9_10:
         vlc_gain = 8294; /* 1.01234 */
         break;

      /* DVB S2X QPSK */
      case BSAT_Mode_eDvbs2x_Qpsk_13_45:    /* pls=132 */
         vlc_gain = 17152; /* 1.25 */
         break;

      case BSAT_Mode_eDvbs2x_Qpsk_9_20:     /* pls=134 */
      case BSAT_Mode_eDvbs2x_Qpsk_11_20:    /* pls=136 */
         vlc_gain = 8960;
         break;

      /* DVB S2X 8APSK, 8PSK */
      case BSAT_Mode_eDvbs2x_8apsk_5_9_L:   /* pls=138 */
         vlc_gain = 6656;
         break;

      case BSAT_Mode_eDvbs2x_8apsk_26_45_L: /* pls=140 */
         vlc_gain = 10240;
         break;

      case BSAT_Mode_eDvbs2x_8psk_23_36:    /* pls=142 */
      case BSAT_Mode_eDvbs2x_8psk_25_36:    /* pls=144 */
      case BSAT_Mode_eDvbs2x_8psk_13_18:    /* pls=146 */
         vlc_gain = 10752;
         break;

      /* DVB S2X 16APSK 8+8 constellation */
      case BSAT_Mode_eDvbs2x_16apsk_1_2_L:  /* pls=148 */
      case BSAT_Mode_eDvbs2x_16apsk_8_15_L: /* pls=150 */
      case BSAT_Mode_eDvbs2x_16apsk_5_9_L:  /* pls=152 */
         vlc_gain = 10240;
         break;

      case BSAT_Mode_eDvbs2x_16apsk_3_5_L:  /* pls=158 */
      case BSAT_Mode_eDvbs2x_16apsk_2_3_L:  /* pls=164 */
         vlc_gain = 10752;
         break;

      /* DVB S2X 16APSK 12+4 constellation */
      case BSAT_Mode_eDvbs2x_16apsk_26_45:  /* pls=154 */
      case BSAT_Mode_eDvbs2x_16apsk_3_5:    /* pls=156 */
      case BSAT_Mode_eDvbs2x_16apsk_28_45:  /* pls=160 */
      case BSAT_Mode_eDvbs2x_16apsk_23_36:  /* pls=162 */
         vlc_gain = 9216;
         break;

      case BSAT_Mode_eDvbs2x_16apsk_25_36:  /* pls=166 */
      case BSAT_Mode_eDvbs2x_16apsk_13_18:  /* pls=168 */
      case BSAT_Mode_eDvbs2x_16apsk_7_9:    /* pls=170 */
         vlc_gain = 10496;
         break;

      case BSAT_Mode_eDvbs2x_16apsk_77_90:  /* pls=172 */
         vlc_gain = 8960;
         break;

      /* DVB S2X 32APSK */
      case BSAT_Mode_eDvbs2x_32apsk_2_3_L:  /* pls=174 */
      case BSAT_Mode_eDvbs2x_32apsk_32_45:  /* pls=178 */
      case BSAT_Mode_eDvbs2x_32apsk_11_15:  /* pls=180 */
      case BSAT_Mode_eDvbs2x_32apsk_7_9:    /* pls=182 */
         vlc_gain = 8704;
         break;

      default:
         if ((BSAT_MODE_IS_DVBS2_QPSK(hChn->actualMode)) || (BSAT_MODE_IS_DVBS2X_QPSK(hChn->actualMode)))
            vlc_gain = 17152; /* 2.09375 */
         else if ((BSAT_MODE_IS_DVBS2_8PSK(hChn->actualMode)) || (BSAT_MODE_IS_DVBS2X_8PSK(hChn->actualMode)))
            vlc_gain = 10240; /* 1.250 */
         else /* S2/S2X 16/32 APSK default */
            vlc_gain = 9667; /* 1.180 */
         break;
   }

   continue2:
   val = BCHP_SDS_EQ_0_VLCI_vlci_wait_MASK;
   val |= (vlc_gain << 16);

   write_vlc_gain:
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_EQ_VLCTL, ~BCHP_SDS_EQ_0_VLCTL_vgain_sel_MASK);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VLCI, val);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VLCQ, val);
   return BERR_SUCCESS;
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

   static const uint32_t DVBS2X_SNRHT[] =
   {
      0x00000000, /* DVB-S2X QPSK 13/45 */
      0x00000000, /* DVB-S2X QPSK 9/20 */
      0x00000000, /* DVB-S2X QPSK 11/20 */
      0x00000000, /* DVB-S2X 8APSK 5/9-L */
      0x00000000, /* DVB-S2X 8APSK 26/45-L */
      0x00000000, /* DVB-S2X 8APSK 23/36 */
      0x00000000, /* DVB-S2X 8APSK 25/36 */
      0x00000000, /* DVB-S2X 8APSK 13/18 */
      0x00000000, /* DVB-S2X 16APSK 1/2-L */
      0x00000000, /* DVB-S2X 16APSK 8/15-L */
      0x00000000, /* DVB-S2X 16APSK 5/9-L */
      0x00000000, /* DVB-S2X 16APSK 26/45 */
      0x00000000, /* DVB-S2X 16APSK 3/5 */
      0x00000000, /* DVB-S2X 16APSK 3/5-L */
      0x00000000, /* DVB-S2X 16APSK 28/45 */
      0x00000000, /* DVB-S2X 16APSK 23/36 */
      0x00000000, /* DVB-S2X 16APSK 2/3-L */
      0x00000000, /* DVB-S2X 16APSK 25/36 */
      0x00000000, /* DVB-S2X 16APSK 13/18 */
      0x00000000, /* DVB-S2X 16APSK 7/9 */
      0x00000000, /* DVB-S2X 16APSK 77/90 */
      0x00000000, /* DVB-S2X 32APSK 2/3-L */
      0x00000000, /* DVB-S2X 32APSK 32/45 */
      0x00000000, /* DVB-S2X 32APSK 11/15 */
      0x00000000, /* DVB-S2X 32APSK 7/9 */
   };

   static const uint32_t DVBS2X_SNRLT[] =
   {
      0x00000000, /* DVB-S2X QPSK 13/45 */
      0x00000000, /* DVB-S2X QPSK 9/20 */
      0x00000000, /* DVB-S2X QPSK 11/20 */
      0x00000000, /* DVB-S2X 8APSK 5/9-L */
      0x00000000, /* DVB-S2X 8APSK 26/45-L */
      0x00000000, /* DVB-S2X 8APSK 23/36 */
      0x00000000, /* DVB-S2X 8APSK 25/36 */
      0x00000000, /* DVB-S2X 8APSK 13/18 */
      0x00000000, /* DVB-S2X 16APSK 1/2-L */
      0x00000000, /* DVB-S2X 16APSK 8/15-L */
      0x00000000, /* DVB-S2X 16APSK 5/9-L */
      0x00000000, /* DVB-S2X 16APSK 26/45 */
      0x00000000, /* DVB-S2X 16APSK 3/5 */
      0x00000000, /* DVB-S2X 16APSK 3/5-L */
      0x00000000, /* DVB-S2X 16APSK 28/45 */
      0x00000000, /* DVB-S2X 16APSK 23/36 */
      0x00000000, /* DVB-S2X 16APSK 2/3-L */
      0x00000000, /* DVB-S2X 16APSK 25/36 */
      0x00000000, /* DVB-S2X 16APSK 13/18 */
      0x00000000, /* DVB-S2X 16APSK 7/9 */
      0x00000000, /* DVB-S2X 16APSK 77/90 */
      0x00000000, /* DVB-S2X 32APSK 2/3-L */
      0x00000000, /* DVB-S2X 32APSK 32/45 */
      0x00000000, /* DVB-S2X 32APSK 11/15 */
      0x00000000, /* DVB-S2X 32APSK 7/9 */
   };
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val, i;

   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
   {
      /* TBD */
   }
   else if (BSAT_MODE_IS_DVBS2(hChn->actualMode))
   {
      i = hChn->actualMode - BSAT_Mode_eDvbs2_Qpsk_1_4;
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRHT, DVBS2_SNRHT[i]);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRLT, DVBS2_SNRLT[i]);
   }
   else
   {
      i = hChn->actualMode - BSAT_Mode_eDvbs2x_Qpsk_13_45;
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRHT, DVBS2X_SNRHT[i]);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRLT, DVBS2X_SNRLT[i]);
   }

   BSAT_CHK_RETCODE(BSAT_g1_P_InitBert_isr(h));

   if (hChn->acqSettings.options & BSAT_ACQ_ENABLE_BERT)
      val = 0x04623751;
   else
      val = 0x01326754;
#if 0
   val = 0x04576231; /* orig QPSK setting */
#endif

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BEM1, val);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BEM2, 0x01546732);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BEIT, 0x0FFF05FF);

   val = 0;
#if defined(BSAT_HAS_ACM) && defined(BCHP_SDS_BERT_0_ACMCTL_bert_acm_filter_mode_MASK)
   if ((hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM) && (hChn->acmSettings.bert.bEnable))
   {
      val = BCHP_SDS_BERT_0_ACMCTL_bert_acm_filter_en_MASK;
      if (hChn->acmSettings.bert.bFilterByStreamId)
      {
         val |= (1 << BCHP_SDS_BERT_0_ACMCTL_bert_acm_filter_mode_SHIFT);
         val |= ((hChn->acmSettings.bert.param.streamId & 0xFF) << BCHP_SDS_BERT_0_ACMCTL_bert_target_streamid_SHIFT);
      }
      else
      {
         uint32_t acm_check;
         uint8_t plscode;

         retCode = BSAT_g1_P_GetDvbs2Plscode_isrsafe(hChn->acmSettings.bert.param.mode, &plscode);
         if (retCode == BERR_SUCCESS)
         {
            acm_check = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK);
            if (acm_check & 1)
               plscode |= 1;
            val |= (plscode << BCHP_SDS_BERT_0_ACMCTL_bert_target_plscode_SHIFT);
         }
         else
            val &= ~BCHP_SDS_BERT_0_ACMCTL_bert_acm_filter_en_MASK;
      }
   }
#endif
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_ACMCTL, val);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRCTL, 0x88);
   val = 0x0B;
#if defined(BSAT_HAS_ACM) && defined(BCHP_SDS_SNR_0_SNRCTL_snr_filter_mode_MASK)
   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
   {
      if (hChn->acmSettings.snr.filterMode == BSAT_AcmSnrFilter_eMode)
      {
         uint8_t plscode;
         retCode = BSAT_g1_P_GetDvbs2Plscode_isrsafe(hChn->acmSettings.softDecisions.param.mode, &plscode);
         if (retCode == BERR_SUCCESS)
         {
            val |= (1 << BCHP_SDS_SNR_0_SNRCTL_snr_filter_mode_SHIFT);
            val |= (plscode << BCHP_SDS_SNR_0_SNRCTL_snr_filter_plscode_SHIFT);
         }
      }
      else if ((hChn->acmSettings.snr.filterMode == BSAT_AcmSnrFilter_eModulation) || (hChn->acmSettings.snr.filterMode == BSAT_AcmSnrFilter_eModulationLE))
      {
         val |= ((uint32_t)hChn->acmSettings.snr.filterMode << BCHP_SDS_SNR_0_SNRCTL_snr_filter_mode_SHIFT);
         val |= (((uint32_t)hChn->acmSettings.snr.param.modulation & 0x07) << BCHP_SDS_SNR_0_SNRCTL_snr_filter_modulation_SHIFT);
      }
   }
#endif
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRCTL, val);
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
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_MPLCK, 0x030F0E0F);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecConfig_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecConfig_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   BERR_Code retCode;
   uint32_t val;

   retCode = BSAT_g1_P_AfecUpdateBlockCount_isrsafe(h); /* need to do this because counters are about to be reset */
   if (retCode != BERR_SUCCESS)
   {
      BDBG_WRN(("BSAT_g1_P_AfecConfig_isr(): BSAT_g1_P_AfecUpdateBlockCount_isrsafe() error 0x%X", retCode));
   }

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetDecoderParams_isr(h));

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_CTRL, 0x00000001);

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_BCH_SMCFG);
   if (hChn->xportSettings.bTei)
      val |= 0x8000;
   else
      val &= ~0x8000;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_SMCFG, val);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_CNTR_CTRL, 0x00000003); /* clear counters */

   BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetMpcfg_isr(h));

   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_AFEC_BCH_BBHDR3, ~(BCHP_AFEC_0_BCH_BBHDR3_sel_stream_id_MASK | BCHP_AFEC_0_BCH_BBHDR3_use_prev_on_illegal_MASK));
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_AFEC_CNTR_CTRL, 0x00000006); /* clear counters */
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_AFEC_RST, 0x100); /* reset data path */

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_CONFIG_0, 0x05000B02);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_AfecSetOpll_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecSetOpll_isr(BSAT_ChannelHandle h)
{
   static const uint16_t BSAT_NUMBER_OF_BITS_DVBS2[] =
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

   static const uint16_t BSAT_NUMBER_OF_BITS_DVBS2X[] =
   {
      18448, /* DVB-S2X QPSK 13/45 */
      28888, /* DVB-S2X QPSK 9/20 */
      35368, /* DVB-S2X QPSK 11/20 */
      35728, /* DVB-S2X 8APSK 5/9-L */
      37168, /* DVB-S2X 8APSK 26/45-L */
      41128, /* DVB-S2X 8APSK 23/36 */
      44728, /* DVB-S2X 8APSK 25/36 */
      46528, /* DVB-S2X 8APSK 13/18 */
      32128, /* DVB-S2X 16APSK 1/2-L */
      34288, /* DVB-S2X 16APSK 8/15-L */
      35728, /* DVB-S2X 16APSK 5/9-L */
      37168, /* DVB-S2X 16APSK 26/45 */
      38608, /* DVB-S2X 16APSK 3/5 */
      38688, /* DVB-S2X 16APSK 3/5-L */
      40048, /* DVB-S2X 16APSK 28/45 */
      41128, /* DVB-S2X 16APSK 23/36 */
      42960, /* DVB-S2X 16APSK 2/3-L */
      44728, /* DVB-S2X 16APSK 25/36 */
      46528, /* DVB-S2X 16APSK 13/18 */
      50128, /* DVB-S2X 16APSK 7/9 */
      55168, /* DVB-S2X 16APSK 77/90 */
      42960, /* DVB-S2X 32APSK 2/3-L */
      45808, /* DVB-S2X 32APSK 32/45 */
      47248, /* DVB-S2X 32APSK 11/15 */
      50128, /* DVB-S2X 32APSK 7/9 */
   };

   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t i, symbols, pilot_blocks, number_of_bits, number_of_symbols;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   BSAT_Mode mode;

   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
   {
      hChn->outputBitrate = 0; /* TBD */
      hChn->opll_N = 0; /* TBD */
      hChn->opll_D = 0; /* TBD */
      return BERR_SUCCESS;
   }
   else
      mode = hChn->actualMode;

   if (BSAT_MODE_IS_DVBS2_8PSK(mode) || BSAT_MODE_IS_DVBS2X_8PSK(mode) || BSAT_MODE_IS_DVBS2X_8APSK(mode))
   {
      /* 8PSK */
      symbols = 21600;
      pilot_blocks = 14*36;
   }
   else if (BSAT_MODE_IS_DVBS2_QPSK(mode) || BSAT_MODE_IS_DVBS2X_QPSK(mode))
   {
      /* QPSK */
      symbols = 32400;
      pilot_blocks = 22*36;
   }
   else if (BSAT_MODE_IS_DVBS2_16APSK(mode) || BSAT_MODE_IS_DVBS2X_16APSK(mode))
   {
      /* 16APSK */
      symbols = 16200;
      pilot_blocks = 11*36;
   }
   else if (BSAT_MODE_IS_DVBS2_32APSK(mode) || BSAT_MODE_IS_DVBS2X_32APSK(mode))
   {
      /* 32APSK */
      symbols = 12960;
      pilot_blocks = 8*36;
   }
   else
   {
      BDBG_WRN(("Invalid mode: %08X\n", mode));
      return BERR_INVALID_PARAMETER;
   }

   if (BSAT_MODE_IS_DVBS2(mode))
   {
      i = mode - BSAT_Mode_eDvbs2_Qpsk_1_4;
      number_of_bits = (uint32_t)BSAT_NUMBER_OF_BITS_DVBS2[i];
   }
   else
   {
      i = mode - BSAT_Mode_eDvbs2x_Qpsk_13_45;
      number_of_bits = (uint32_t)BSAT_NUMBER_OF_BITS_DVBS2X[i];
   }

   if (BSAT_g1_P_AfecIsPilot_isr(h) || (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM))
      symbols += pilot_blocks;
   number_of_symbols = symbols + 90;

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
#ifdef BSAT_HAS_ACM
      BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_AFEC_CNTR_CTRL, 0x07); /* clear BCH/LDPC/ACM counters */
      BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_AFEC_PLS_LDPC_CNTR_CTRL, 0x000000FF);
      BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_AFEC_PLS_BCH_CNTR_CTRL, 0x000000FF);
#else
      BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_AFEC_CNTR_CTRL, 0x04); /* clear BCH counters */
#endif
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
 BSAT_g1_P_AfecResetChannel_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecResetChannel_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_AFEC_RST, 0x3);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecSetDecoderParams_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_AfecSetDecoderParams_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t code_size = hChn->bShortFrame ? 16200 : 64800;
   uint32_t ldpc_clock, P_hi, P_lo, Q_hi, Q_lo, cycle_count = 0, cycles_adj, cycles_per_iter, max_iter;
   static const uint32_t symbols_btn_frames = 90;
   static const uint32_t margin_cycles = 50;
   static uint32_t bits[5] = {2, 3, 4, 5, 1};
   static uint32_t reg[5] = {BCHP_AFEC_ACM_CYCLE_CNT_0, BCHP_AFEC_ACM_CYCLE_CNT_1, BCHP_AFEC_ACM_CYCLE_CNT_2, BCHP_AFEC_ACM_CYCLE_CNT_3, BCHP_AFEC_ACM_CYCLE_CNT_4};
   code_size = hChn->bShortFrame ? 16200 : 64800;
   int i, idx;

   static const uint16_t BSAT_dvbs2_cycles_adj[] = /* cycles_for_init + output_cycles */
   {
      2201 + 94, /* DVB-S2 QPSK 1/4 */
      2442 + 124, /* DVB-S2 QPSK 1/3 */
      2635 + 148, /* DVB-S2 QPSK 2/5 */
      2560 + 184, /* DVB-S2 QPSK 1/2 */
      3216 + 220, /* DVB-S2 QPSK 3/5 */
      2448 + 244, /* DVB-S2 QPSK 2/3 */
      2571 + 274, /* DVB-S2 QPSK 3/4 */
      2646 + 292, /* DVB-S2 QPSK 4/5 */
      2700 + 304, /* DVB-S2 QPSK 5/6 */
      2216 + 324, /* DVB-S2 QPSK 8/9 */
      2219 + 328, /* DVB-S2 QPSK 9/10 */
      3216 + 220, /* DVB-S2 8PSK 3/5 */
      2448 + 244, /* DVB-S2 8PSK 2/3 */
      2571 + 274, /* DVB-S2 8PSK 3/4 */
      2700 + 304, /* DVB-S2 8PSK 5/6 */
      2216 + 324, /* DVB-S2 8PSK 8/9 */
      2219 + 328, /* DVB-S2 8PSK 9/10 */
      2448 + 244, /* DVB-S2 16APSK 2/3 */
      2571 + 274, /* DVB-S2 16APSK 3/4 */
      2646 + 292, /* DVB-S2 16APSK 4/5 */
      2700 + 304, /* DVB-S2 16APSK 5/6 */
      2216 + 324, /* DVB-S2 16APSK 8/9 */
      2219 + 328, /* DVB-S2 16APSK 9/10 */
      2571 + 274, /* DVB-S2 32APSK 3/4 */
      2646 + 292, /* DVB-S2 32APSK 4/5 */
      2700 + 304, /* DVB-S2 32APSK 5/6 */
      2216 + 324, /* DVB-S2 32APSK 8/9 */
      2219 + 328, /* DVB-S2 32APSK 9/10 */
   };

   static const uint16_t BSAT_dvbs2_cycles_adj_short[] = /* cycles_for_init + output_cycles */
   {
      581 + 22, /* DVB-S2 QPSK 1/4 */
      642 + 34, /* DVB-S2 QPSK 1/3 */
      691 + 40, /* DVB-S2 QPSK 2/5 */
      580 + 44, /* DVB-S2 QPSK 1/2 */
      840 + 58, /* DVB-S2 QPSK 3/5 */
      648 + 64, /* DVB-S2 QPSK 2/3 */
      578 + 70, /* DVB-S2 QPSK 3/4 */
      541 + 74, /* DVB-S2 QPSK 4/5 */
      605 + 78, /* DVB-S2 QPSK 5/6 */
      596 + 84, /* DVB-S2 QPSK 8/9 */
      0 + 0, /* DVB-S2 QPSK 9/10 */
      840 + 58, /* DVB-S2 8PSK 3/5 */
      648 + 64, /* DVB-S2 8PSK 2/3 */
      578 + 70, /* DVB-S2 8PSK 3/4 */
      605 + 78, /* DVB-S2 8PSK 5/6 */
      596 + 84, /* DVB-S2 8PSK 8/9 */
      0 + 0, /* DVB-S2 8PSK 9/10 */
      648 + 64, /* DVB-S2 16APSK 2/3 */
      578 + 70, /* DVB-S2 16APSK 3/4 */
      541 + 74, /* DVB-S2 16APSK 4/5 */
      605 + 78, /* DVB-S2 16APSK 5/6 */
      596 + 84, /* DVB-S2 16APSK 8/9 */
      0 + 0, /* DVB-S2 16APSK 9/10 */
      578 + 70, /* DVB-S2 32APSK 3/4 */
      541 + 74, /* DVB-S2 32APSK 4/5 */
      605 + 78, /* DVB-S2 32APSK 5/6 */
      596 + 84, /* DVB-S2 32APSK 8/9 */
      0 + 0, /* DVB-S2 32APSK 9/10 */
   };

   static const uint16_t BSAT_dvbs2x_cycles_adj[] = /* cycles_for_init + output_cycles */
   {
      2554 + 108, /* DVB-S2X QPSK 13/45 */
      2816 + 166, /* DVB-S2X QPSK 9/20 */
      2963 + 202, /* DVB-S2X QPSK 11/20 */
      2902 + 204, /* DVB-S2X 8APSK 5/9-L */
      3073 + 212, /* DVB-S2X 8APSK 26/45-L */
      2646 + 234, /* DVB-S2X 8APSK 23/36 */
      2909 + 254, /* DVB-S2X 8APSK 25/36 */
      2849 + 264, /* DVB-S2X 8APSK 13/18 */
      2904 + 184, /* DVB-S2X 16APSK 1/2-L */
      3019 + 196, /* DVB-S2X 16APSK 8/15-L */
      2902 + 204, /* DVB-S2X 16APSK 5/9-L */
      3088 + 212, /* DVB-S2X 16APSK 26/45 */
      3216 + 220, /* DVB-S2X 16APSK 3/5 */
      3219 + 220, /* DVB-S2X 16APSK 3/5-L */
      2766 + 228, /* DVB-S2X 16APSK 28/45 */
      2646 + 234, /* DVB-S2X 16APSK 23/36 */
      3179 + 244, /* DVB-S2X 16APSK 2/3-L */
      2909 + 254, /* DVB-S2X 16APSK 25/36 */
      2849 + 264, /* DVB-S2X 16APSK 13/18 */
      2935 + 284, /* DVB-S2X 16APSK 7/9 */
      3113 + 312, /* DVB-S2X 16APSK 77/90 */
      3179 + 244, /* DVB-S2X 32APSK 2/3-L */
      3076 + 260, /* DVB-S2X 32APSK 32/45 */
      3093 + 268, /* DVB-S2X 32APSK 11/15 */
      2935 + 284, /* DVB-S2X 32APSK 7/9 */
   };

   static const uint16_t BSAT_dvbs2_cycles_per_iter[] =
   {
      2200, /* DVB-S2 QPSK 1/4 */
      2441, /* DVB-S2 QPSK 1/3 */
      2634, /* DVB-S2 QPSK 2/5 */
      2559, /* DVB-S2 QPSK 1/2 */
      3215, /* DVB-S2 QPSK 3/5 */
      2447, /* DVB-S2 QPSK 2/3 */
      2570, /* DVB-S2 QPSK 3/4 */
      2645, /* DVB-S2 QPSK 4/5 */
      2699, /* DVB-S2 QPSK 5/6 */
      2215, /* DVB-S2 QPSK 8/9 */
      2218, /* DVB-S2 QPSK 9/10 */
      3215, /* DVB-S2 8PSK 3/5 */
      2447, /* DVB-S2 8PSK 2/3 */
      2570, /* DVB-S2 8PSK 3/4 */
      2699, /* DVB-S2 8PSK 5/6 */
      2215, /* DVB-S2 8PSK 8/9 */
      2218, /* DVB-S2 8PSK 9/10 */
      2447, /* DVB-S2 16APSK 2/3 */
      2570, /* DVB-S2 16APSK 3/4 */
      2645, /* DVB-S2 16APSK 4/5 */
      2699, /* DVB-S2 16APSK 5/6 */
      2215, /* DVB-S2 16APSK 8/9 */
      2218, /* DVB-S2 16APSK 9/10 */
      2570, /* DVB-S2 32APSK 3/4 */
      2645, /* DVB-S2 32APSK 4/5 */
      2699, /* DVB-S2 32APSK 5/6 */
      2215, /* DVB-S2 32APSK 8/9 */
      2218, /* DVB-S2 32APSK 9/10 */
   };

   static const uint16_t BSAT_dvbs2x_cycles_per_iter[] =
   {
      2553, /* DVB-S2X QPSK 13/45 */
      2815, /* DVB-S2X QPSK 9/20 */
      2962, /* DVB-S2X QPSK 11/20 */
      2901, /* DVB-S2X 8APSK 5/9-L */
      3072, /* DVB-S2X 8APSK 26/45-L */
      2645, /* DVB-S2X 8APSK 23/36 */
      2908, /* DVB-S2X 8APSK 25/36 */
      2848, /* DVB-S2X 8APSK 13/18 */
      2903, /* DVB-S2X 16APSK 1/2-L */
      3018, /* DVB-S2X 16APSK 8/15-L */
      2901, /* DVB-S2X 16APSK 5/9-L */
      3087, /* DVB-S2X 16APSK 26/45 */
      3215, /* DVB-S2X 16APSK 3/5 */
      3218, /* DVB-S2X 16APSK 3/5-L */
      2765, /* DVB-S2X 16APSK 28/45 */
      2645, /* DVB-S2X 16APSK 23/36 */
      3178, /* DVB-S2X 16APSK 2/3-L */
      2908, /* DVB-S2X 16APSK 25/36 */
      2848, /* DVB-S2X 16APSK 13/18 */
      2934, /* DVB-S2X 16APSK 7/9 */
      3112, /* DVB-S2X 16APSK 77/90 */
      3178, /* DVB-S2X 32APSK 2/3-L */
      3075, /* DVB-S2X 32APSK 32/45 */
      3092, /* DVB-S2X 32APSK 11/15 */
      2934, /* DVB-S2X 32APSK 7/9 */
   };

   if (BSAT_MODE_IS_DVBS2(hChn->actualMode))
   {
      idx = hChn->actualMode - BSAT_Mode_eDvbs2_Qpsk_1_4;
      cycles_per_iter = (uint32_t)BSAT_dvbs2_cycles_per_iter[idx];
      if (hChn->bShortFrame)
         cycles_adj = (uint32_t)BSAT_dvbs2_cycles_adj_short[idx];
      else
         cycles_adj = (uint32_t)BSAT_dvbs2_cycles_adj[idx];
   }
   else
   {
      idx = hChn->actualMode - BSAT_Mode_eDvbs2x_Qpsk_13_45;
      cycles_adj = (uint32_t)BSAT_dvbs2x_cycles_adj[idx];
      cycles_per_iter = (uint32_t)BSAT_dvbs2x_cycles_per_iter[idx];
   }

   if (hChn->acqSettings.mode != BSAT_Mode_eDvbs2_ACM)
   {
      if (BSAT_MODE_IS_DVBS2_QPSK(hChn->actualMode) || BSAT_MODE_IS_DVBS2X_QPSK(hChn->actualMode))
         idx = 0; /* use ACM_CYCLE_CNT_0 */
      else if (BSAT_MODE_IS_DVBS2_8PSK(hChn->actualMode) || BSAT_MODE_IS_DVBS2X_8PSK(hChn->actualMode) || BSAT_MODE_IS_DVBS2X_8APSK(hChn->actualMode))
         idx = 1; /* use ACM_CYCLE_CNT_1 */
      else if (BSAT_MODE_IS_DVBS2_16APSK(hChn->actualMode) || BSAT_MODE_IS_DVBS2X_16APSK(hChn->actualMode))
         idx = 2; /* use ACM_CYCLE_CNT_2 */
      else if (BSAT_MODE_IS_DVBS2_32APSK(hChn->actualMode) || BSAT_MODE_IS_DVBS2X_32APSK(hChn->actualMode))
         idx = 3; /* use ACM_CYCLE_CNT_3 */
      else
      {
         /* BPSK */
         BDBG_ERR(("unsupported mode (0x%X)\n", hChn->actualMode));
         return BSAT_ERR_UNSUPPORTED_HW;
      }
   }

   ldpc_clock = hChn->fecFreq; /* 270000000 */
   for (i = 0; i < 5; i++)
   {
      BMTH_HILO_32TO64_Mul(ldpc_clock, ((code_size / bits[i]) + symbols_btn_frames), &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqSettings.symbolRate, &Q_hi, &Q_lo);
      Q_lo -= margin_cycles;

#if 1
      /* per Sam, reduce by 10% for now (optimize later) */
      Q_lo = (Q_lo * 9) / 10;
#endif

      BSAT_g1_P_WriteRegister_isrsafe(h, reg[i], Q_lo);
      if (i == 0)
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_ACM_MISC_1, Q_lo);
      if (i == idx)
         cycle_count = Q_lo;
   }

   /* a few modcods will have better optimized SIGMA_SCALE and SCALE_XY values than default */
   /* set AFEC_ACM_MAX_ITER_OVERIDE/AFEC_S2X_MAX_ITER_OVERIDE and AFEC_ACM_MODCODE_* */
   max_iter = (cycle_count - cycles_adj) / cycles_per_iter;
   /* BKNI_Printf("max_iter=%d\n", max_iter); */

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetStreamRegStat_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_GetStreamRegStat_isrsafe(BSAT_ChannelHandle h, int idx, uint32_t *pStat)
{
   static const uint32_t BSAT_afec_stream_stat_reg[8] = {
      BCHP_AFEC_BCH_STREAM_ID0_STAT, BCHP_AFEC_BCH_STREAM_ID1_STAT,
      BCHP_AFEC_BCH_STREAM_ID2_STAT, BCHP_AFEC_BCH_STREAM_ID3_STAT,
      BCHP_AFEC_BCH_STREAM_ID4_STAT, BCHP_AFEC_BCH_STREAM_ID5_STAT,
      BCHP_AFEC_BCH_STREAM_ID6_STAT, BCHP_AFEC_BCH_STREAM_ID7_STAT
   };

   if ((idx >= 0) && (idx < 8))
   {
      *pStat = BSAT_g1_P_ReadRegister_isrsafe(h, BSAT_afec_stream_stat_reg[idx]);
      return BERR_SUCCESS;
   }
   else
      return BERR_INVALID_PARAMETER;
}
