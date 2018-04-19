/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"


#if !defined(BSAT_HAS_DVBS2X)
#error "invalid setting for BSAT_HAS_DVBS2X"
#endif


BDBG_MODULE(bsat_g1_priv_hp);

#if BCHP_CHIP==45308
#include "fe.h"
#endif


#define BSAT_DEBUG_HP(x) /* x */


/* local functions */
BERR_Code BSAT_g1_P_HpAcquire1_isr(BSAT_ChannelHandle h);
bool BSAT_g1_P_HpOkToEnableFineFreqEst_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_OnHpTimeOut_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_HpConfig_isr(BSAT_ChannelHandle h);


#define BSAT_DVBS2X_NUM_MODCODS 25
static const uint8_t BSAT_P_dvbs2x_plscode[BSAT_DVBS2X_NUM_MODCODS] =
{
   132, /* DVB-S2X QPSK 13/45 */
   134, /* DVB-S2X QPSK 9/20 */
   136, /* DVB-S2X QPSK 11/20 */
   138, /* DVB-S2X 8APSK 5/9 */
   140, /* DVB-S2X 8APSK 26/45-L */
   142, /* DVB-S2X 8APSK 23/36 */
   144, /* DVB-S2X 8APSK 25/36 */
   146, /* DVB-S2X 8APSK 13/18 */
   148, /* DVB-S2X 16APSK 1/2-L */
   150, /* DVB-S2X 16APSK 8/15-L */
   152, /* DVB-S2X 16APSK 5/9-L */
   154, /* DVB-S2X 16APSK 26/45 */
   156, /* DVB-S2X 16APSK 3/5 */
   158, /* DVB-S2X 16APSK 3/5-L */
   160, /* DVB-S2X 16APSK 28/45 */
   162, /* DVB-S2X 16APSK 23/36 */
   164, /* DVB-S2X 16APSK 2/3-L */
   166, /* DVB-S2X 16APSK 25/36 */
   168, /* DVB-S2X 16APSK 13/18 */
   170, /* DVB-S2X 16APSK 7/9 */
   172, /* DVB-S2X 16APSK 77/90 */
   174, /* DVB-S2X 32APSK 2/3-L */
   178, /* DVB-S2X 32APSK 32/45 */
   180, /* DVB-S2X 32APSK 11/15 */
   182  /* DVB-S2X 32APSK 7/9 */
};


/******************************************************************************
 BSAT_g1_P_HpAcquire_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_HpAcquire_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT funct)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BSAT_DEBUG_HP(BDBG_MSG(("in BSAT_g1_P_HpAcquire")));

   hChn->nextFunct = funct; /* passFunct = function to call after HP locks */
   hChn->bEnableFineFreqEst = false;

   BSAT_CHK_RETCODE(BSAT_g1_P_LogTraceBuffer_isr(h, BSAT_TraceEvent_eStartHp));
   BSAT_CHK_RETCODE(BSAT_g1_P_HpAcquire1_isr(h));

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_HpAcquire1_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_HpAcquire1_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t timeout;

   if (hChn->acqSettings.symbolRate <= 5000000)
      timeout = hChn->acqSettings.symbolRate;
   else if (hChn->acqSettings.symbolRate <= 12000000)
      timeout = hChn->acqSettings.symbolRate >> 1;
   else
      timeout = hChn->acqSettings.symbolRate >> 2;

   /* configure the HP */
   hChn->bShortFrame = false;
   BSAT_CHK_RETCODE(BSAT_g1_P_HpEnable_isr(h, false));
   BSAT_CHK_RETCODE(BSAT_g1_P_HpConfig_isr(h));

   /* interrupt on HP state match 5 (RECEIVER_LOCK) */
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, ~0xF0000000, 0x50000000);

   /* let the HP run */
   /* BSAT_g1_P_GetAcquisitionTimerValue_isr(h, &(hChn->count1)); */
   BSAT_CHK_RETCODE(BSAT_g1_P_HpEnable_isr(h, true));

   /* start reacquisition timer */
   if (hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_ENABLED)
   {
      if (hChn->acqSettings.symbolRate <= 5000000)
         timeout *= 2; /* 2 sec timeout on low baud rate scan mode */
   }

   retCode = BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaud, timeout, BSAT_g1_P_OnHpTimeOut_isr);

#ifndef BSAT_EXCLUDE_TFEC
   if (hChn->acqSettings.mode == BSAT_Mode_eTurbo_scan)
      hChn->turboScanState |= BSAT_TURBO_SCAN_STATE_HP_INIT;
#endif

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_HpEnable()
******************************************************************************/
BERR_Code BSAT_g1_P_HpEnable(BSAT_ChannelHandle h, bool bEnable)
{
   BERR_Code retCode;

   BKNI_EnterCriticalSection();
   retCode = BSAT_g1_P_HpEnable_isr(h, bEnable);
   BKNI_LeaveCriticalSection();

   return retCode;
}


/******************************************************************************
 BSAT_g1_P_HpEnable_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_HpEnable_isr(BSAT_ChannelHandle h, bool bEnable)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BSAT_CHK_RETCODE(BSAT_g1_P_HpDisableInterrupts_isr(h));

   if (bEnable)
   {
      /* enable */
      BINT_ClearCallback_isr(hChn->hHpStateMatchCb);
      BINT_EnableCallback_isr(hChn->hHpStateMatchCb);
      BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, ~1, 8);
   }
   else
   {
      /* disable FROF1, FROF2, FROF3 */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FROF1_SW, 0);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FROF2_SW, 0);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FROF3_SW, 0);

      BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, ~0x18F8, 0x51); /* acm_mode_delayed = 0, caren=0, one_of_n_period=5 */
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, ~0xC000);

      /* assert micro override of HP controls to receiver */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE, 0x003FFFFF);
   }

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_HpConfig_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_HpConfig_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t hpconfig, hpcontrol, fnorm, dcorr_threshold, hpoverride, m_n_peak_verify, m_n_receiver_verify, m_n_receiver_lock, val;
   uint32_t frame_length_sample, frof3_sw, hp_dafe, new_state, ignore_phi_from_dafe;
   uint32_t P_hi, P_lo, Q_hi;
   uint8_t tfec_symbol_length, n_check, m_peak, plscode, modcod, s2_type, mask, n_check_limit, dafe_average;
   uint8_t peak_verify_n_check, peak_verify_m_peak, rcvr_verify_n_check, rcvr_verify_m_peak, rcvr_lock_n_check, rcvr_lock_m_peak;
   bool bDvbs2Pilot = false, bDvbs2Mode = false, bDvbs2Scan = false;

   if (hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_ENABLED)
      bDvbs2Scan = true;

   if (BSAT_MODE_IS_DVBS2(hChn->acqSettings.mode) || BSAT_MODE_IS_DVBS2X(hChn->acqSettings.mode))
      bDvbs2Mode = true;

   /* determine if fine freq estimation should be enabled */
   hChn->bEnableFineFreqEst = false;
   if (bDvbs2Mode)
   {
      /* DVB-S2* mode */
      if ((hChn->acqSettings.mode != BSAT_Mode_eDvbs2_ACM) &&
          !bDvbs2Scan && (hChn->acqSettings.options & BSAT_ACQ_PILOT))
      {
         dvbs2_pilot_mode:
         bDvbs2Pilot = true;
         hChn->bEnableFineFreqEst = BSAT_g1_P_HpOkToEnableFineFreqEst_isr(h);
      }
      else if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
      {
         hChn->bEnableFineFreqEst = true;
      }
      else
      {
         /* try fine freq first in scan mode */
         mask = BSAT_DVBS2_SCAN_STATE_ENABLED | BSAT_DVBS2_SCAN_STATE_FOUND;
         if ((hChn->dvbs2ScanState & mask) == BSAT_DVBS2_SCAN_STATE_ENABLED)
         {
            hChn->bEnableFineFreqEst = BSAT_g1_P_HpOkToEnableFineFreqEst_isr(h);
         }

         mask = BSAT_DVBS2_SCAN_STATE_ENABLED | BSAT_DVBS2_SCAN_STATE_FOUND | BSAT_DVBS2_SCAN_STATE_PILOT;
         if ((hChn->dvbs2ScanState & mask) == mask)
            goto dvbs2_pilot_mode; /* modcod scan mode: from previous hp acquisition, we know there is pilot, so enable fine freq est */
      }
   }

   /* calculate HP_FROF3_SW */
   if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
      frof3_sw = BCHP_SDS_HP_0_FROF3_SW_FROF3_SW_OVRD_MASK; /* disable FROF3 for turbo */
   else if (hChn->bEnableFineFreqEst)
      frof3_sw = BCHP_SDS_HP_0_FROF3_SW_FROF3_USE_FINE_FREQ_MASK;
   else
      frof3_sw = 0;

   /* calculate HPOVERRIDE */
   hpoverride = BCHP_FIELD_DATA(SDS_HP_0_HPOVERRIDE, VLCRSTOV, 1);
   BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, VLCFRZOV, 1);
   BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, VLCENOV, 1);
   BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, EQFRZQOV, 1);
   BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, EQFRZIOV, 1);
   BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, EQFRZOV, 1);
   BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, EQRSTOV, 1);
   BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, EQMODEOV, 1);

   if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
   {
      BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, SNOREOV, 1);
      BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, CLFBINTRSTOV, 1);
      BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, CLFBRSTOV, 1);
      BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, CLFBFRZOV, 1);
      BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, CLFBENOV, 1);
   }
   else
   {
      BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, CLFINTRSTOV, 1);
      BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, CLFRSTOV, 1);
      BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, CLFFRZOV, 1);
      BCHP_SET_FIELD_DATA(hpoverride, SDS_HP_0_HPOVERRIDE, CLFENOV, 1);
   }

   /* calculate HPCONFIG */
   hpconfig = BCHP_FIELD_DATA(SDS_HP_0_HPCONFIG, frof2_accum, 1);
   s2_type = 0;
   if (bDvbs2Mode)
   {
      BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, dvbs2_mode, 1);
      BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, TRNLEN, 90);

      mask = BSAT_DVBS2_SCAN_STATE_ENABLED | BSAT_DVBS2_SCAN_STATE_FOUND | BSAT_DVBS2_SCAN_STATE_PILOT;
      if ((!bDvbs2Scan && (hChn->acqSettings.options & BSAT_ACQ_PILOT)) || ((hChn->dvbs2ScanState & mask) == mask))
      {
         s2_type |= 1;
      }

      if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
      {
         modcod = 0;
         s2_type = 0;
      }
      else if (BSAT_MODE_IS_DVBS2X(hChn->acqSettings.mode))
      {
         BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, modcod_msb_for_dvbs2x, 1);
         BSAT_g1_P_GetDvbs2Plscode_isrsafe(hChn->acqSettings.mode, &plscode);
         modcod = (plscode >> 2) & 0x1F;
#if 0
         s2_modcod_msb = (plscode >> 7) & 1;
         if (s2_modcod_msb)
            hpconfig |= BCHP_SDS_HP_0_HPCONFIG_modcod_msb_for_dvbs2x_MASK;
#endif
         s2_type |= (plscode & 0x02);
      }
      else
      {
         if (hChn->acqSettings.options & BSAT_ACQ_DVBS2_SHORT_FRAMES)
            s2_type |= 2;
         modcod = hChn->acqSettings.mode - BSAT_Mode_eDvbs2_Qpsk_1_4 + 1;
      }

      if ((hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM) || bDvbs2Scan)
      {
         /* modcod scan or ACM */
         BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, dcorr_modcod_search, 1);
         BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, hdr_mode, 3);
         BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, acm_mode, 1);
      }
      else
      {
         BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, use_sw_modcod_type, 1);
         BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, hdr_mode, 1);
      }

#ifdef BCHP_SDS_EQ_0_ACM_FIFO
      mask = BSAT_DVBS2_SCAN_STATE_ENABLED | BSAT_DVBS2_SCAN_STATE_FOUND | BSAT_DVBS2_SCAN_STATE_PILOT;
      if (hChn->bEnableFineFreqEst || (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM))
      {
         val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_ACM_FIFO);
         BCHP_SET_FIELD_DATA(val, SDS_EQ_0_ACM_FIFO, acm_fifo_byp, 0);
         if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
         {
            BCHP_SET_FIELD_DATA(val, SDS_EQ_0_ACM_FIFO, buf_delay, 0x122);
         }
         else
         {
            BCHP_SET_FIELD_DATA(val, SDS_EQ_0_ACM_FIFO, buf_delay, 0xC);
         }
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_ACM_FIFO, val);
      }
      else
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_ACM_FIFO, 0x0000100C); /* acm_fifo_byp=1, buf_delay=0xC */
#endif
   }
   else
   {
      /* turbo mode */
      BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, one_of_n_pn_mode, 1);
      BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, use_sw_modcod_type, 1);

      if (BSAT_MODE_IS_TURBO_QPSK(hChn->actualMode))
      {
         BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, TRNLEN, 128);
         BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, turbo_qpsk_mode, 1);
         modcod = 29;
      }
      else
      {
         BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, TRNLEN, 64);
         modcod = 30;
      }
   }
   BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, modcod, modcod);
   BCHP_SET_FIELD_DATA(hpconfig, SDS_HP_0_HPCONFIG, type, s2_type);

   /* calculate HP_DAFE */
   hp_dafe = 1 << BCHP_SDS_HP_0_HP_DAFE_dafe_n_frof2_SHIFT;
   hp_dafe |= (2 << BCHP_SDS_HP_0_HP_DAFE_dafe_n_frof3_SHIFT);
   if (hChn->configParam[BSAT_g1_CONFIG_HP_CTL] & BSAT_g1_CONFIG_HP_CTL_OVERRIDE_DAFE_AVERAGE)
      dafe_average = hChn->configParam[BSAT_g1_CONFIG_HP_CTL] & BSAT_g1_CONFIG_HP_CTL_DAFE_AVERAGE_MASK;
   else if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
      dafe_average = 15;
   else if (bDvbs2Scan)
      dafe_average = 15;
   else if (!bDvbs2Pilot && (BSAT_MODE_IS_DVBS2X(hChn->acqSettings.mode) || BSAT_MODE_IS_DVBS2_EXTENDED(hChn->acqSettings.mode)))
      dafe_average = 63; /* optimize later */
   else
      dafe_average = 15; /* should depend on modcod */

   if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
      hp_dafe &= ~BCHP_SDS_HP_0_HP_DAFE_use_hns_coefficients_MASK;
   else
      hp_dafe |= BCHP_SDS_HP_0_HP_DAFE_use_hns_coefficients_MASK;
   hp_dafe |= (dafe_average << BCHP_SDS_HP_0_HP_DAFE_dafe_average_SHIFT);
   hp_dafe |= BCHP_SDS_HP_0_HP_DAFE_read_internal_FROF_values_MASK;

   /* calculate NEW_STATE */
   n_check = 9;
   m_peak = 3;
   new_state = BCHP_SDS_HP_0_NEW_STATE_use_state_6_MASK | BCHP_SDS_HP_0_NEW_STATE_use_rm_decoder_to_decode_pilot_state_MASK;
   new_state |= BCHP_SDS_HP_0_NEW_STATE_update_frof3_every_frame_MASK;
   new_state |= (n_check << BCHP_SDS_HP_0_NEW_STATE_N_Check_SHIFT);
   new_state |= (m_peak << BCHP_SDS_HP_0_NEW_STATE_M_Peak_SHIFT);
   if (bDvbs2Pilot)
      new_state |= BCHP_SDS_HP_0_NEW_STATE_no_frequency_estimates_on_state_4_MASK;

   /* calculate IGNORE_PHI_FROM_DAFE */
   ignore_phi_from_dafe = BCHP_SDS_HP_0_IGNORE_PHI_FROM_DAFE_receiver_lock_MASK;
   ignore_phi_from_dafe |= BCHP_SDS_HP_0_IGNORE_PHI_FROM_DAFE_receiver_verify_MASK;
   ignore_phi_from_dafe |= BCHP_SDS_HP_0_IGNORE_PHI_FROM_DAFE_receiver_lock_wait_MASK;
   ignore_phi_from_dafe |= BCHP_SDS_HP_0_IGNORE_PHI_FROM_DAFE_peak_verify_MASK;

   /* calculate HPCONTROL */
   hpcontrol = (32<<BCHP_SDS_HP_0_HPCONTROL_hopeless_modcod_SHIFT); /* hopeless_modcod=32 */
   hpcontrol |= BCHP_SDS_HP_0_HPCONTROL_update_shadow_register_MASK;
   hpcontrol |= BCHP_SDS_HP_0_HPCONTROL_CAREN_MASK;
   hpcontrol |= BCHP_SDS_HP_0_HPCONTROL_PFC_RST_MASK;
   hpcontrol |= BCHP_SDS_HP_0_HPCONTROL_one_of_n_direction_MASK;
   hpcontrol |= BCHP_SDS_HP_0_HPCONTROL_trn_valid_dir_MASK;
   hpcontrol |= (9 << BCHP_SDS_HP_0_HPCONTROL_equalizer_pipeline_delay_SHIFT);
   if (bDvbs2Pilot && !hChn->bEnableFineFreqEst)
      hpcontrol |= BCHP_SDS_HP_0_HPCONTROL_pilot_dafe_start_enable_MASK; /* pilot_dafe_start_enable=1 */
   if ((hChn->actualMode == BSAT_Mode_eTurbo_8psk_8_9) || (hChn->actualMode == BSAT_Mode_eTurbo_Qpsk_5_6))
      tfec_symbol_length = 6;
   else if (hChn->actualMode == BSAT_Mode_eTurbo_Qpsk_1_2)
      tfec_symbol_length = 1;
   else if (hChn->actualMode == BSAT_Mode_eTurbo_Qpsk_2_3)
      tfec_symbol_length = 3;
   else if (hChn->actualMode == BSAT_Mode_eTurbo_Qpsk_3_4)
      tfec_symbol_length = 4;
   else if (hChn->actualMode == BSAT_Mode_eTurbo_8psk_3_4)
      tfec_symbol_length = 7;
   else
      tfec_symbol_length = 5;
   hpcontrol |= (tfec_symbol_length << BCHP_SDS_HP_0_HPCONTROL_one_of_n_period_SHIFT);

   /* calculate FNORM */
   BMTH_HILO_32TO64_Mul(1073741824, hChn->acqSettings.symbolRate, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &Q_hi, &fnorm);

   /* calculate M_N_PEAK_VERIFY, M_N_RECEIVER_VERIFY, M_N_RECEIVER_LOCK, DCORR_THRESHOLD, FRAME_LENGTH_SAMPLE */
   n_check_limit = 9;
   peak_verify_n_check = (n_check_limit >= dafe_average) ? n_check_limit : (dafe_average+1);
   peak_verify_m_peak = 4; /*bDvbs2Mode ? 2 : 3*/
   rcvr_verify_n_check = 9;
   rcvr_verify_m_peak = 4;
   rcvr_lock_n_check = 9;
   rcvr_lock_m_peak = 4;
   m_n_peak_verify = peak_verify_n_check << BCHP_SDS_HP_0_M_N_PEAK_VERIFY_N_Check_SHIFT;
   m_n_peak_verify |= (peak_verify_m_peak << BCHP_SDS_HP_0_M_N_PEAK_VERIFY_M_Peak_SHIFT);
   m_n_receiver_verify = rcvr_verify_n_check << BCHP_SDS_HP_0_M_N_RECEIVER_VERIFY_N_Check_SHIFT;
   m_n_receiver_verify |= rcvr_verify_m_peak << BCHP_SDS_HP_0_M_N_RECEIVER_VERIFY_M_Peak_SHIFT;
   m_n_receiver_lock = rcvr_lock_n_check << BCHP_SDS_HP_0_M_N_RECEIVER_LOCK_N_Check_SHIFT;
   m_n_receiver_lock |= rcvr_lock_m_peak << BCHP_SDS_HP_0_M_N_RECEIVER_LOCK_M_Peak_SHIFT;
   dcorr_threshold = 1; /* N_Check=1 */
   frame_length_sample = 100000;

   /* write the registers */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, hpcontrol);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, hpconfig);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FNORM, fnorm);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_DCORR_THRESHOLD, dcorr_threshold);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE, hpoverride);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_M_N_PEAK_VERIFY, m_n_peak_verify);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_M_N_RECEIVER_VERIFY, m_n_receiver_verify);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_M_N_RECEIVER_LOCK, m_n_receiver_lock);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_SAMPLE, frame_length_sample);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FROF3_SW, frof3_sw);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HP_DAFE, hp_dafe);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_NEW_STATE, new_state);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_IGNORE_PHI_FROM_DAFE, ignore_phi_from_dafe);
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_HP_CLOCK_GATE_D_CORR, 1);
   BSAT_g1_P_AfecSetScramblingSeq_isr(h);

   if (hChn->bEnableFineFreqEst == false)
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, ~0x00080400); /* frof3_accum=0, dafe_loop_bypass=0 */

   if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, ~BCHP_SDS_CL_0_CLCTL1_clen_MASK); /* front carrier loop disable */
   else
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, ~(BCHP_SDS_CL_0_CLCTL1_cl_en_rcvr_lf_MASK | BCHP_SDS_CL_0_CLCTL1_clen_MASK));
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, 0x00000004);   /* freeze front carrier loop */

   /* set DAFE loop BW */
   val = 0;
   if (hChn->configParam[BSAT_g1_CONFIG_ACQ_DAFE_CTL])
      val = hChn->configParam[BSAT_g1_CONFIG_ACQ_DAFE_CTL];
   else if (hDev->sdsRevId < 0x74)
   {
      /* BCM45308-A0 and earlier */
      if (hChn->bEnableFineFreqEst)
         val = 0x00000058; /* dafe_int_scale=5(2^-12), dafe_lin_scale=4(2^-6), dafe_leak=0 */
      else if (bDvbs2Mode && !bDvbs2Pilot)
      {
         /* DVB-S2/DVB-S2X pilot off */
         val = 0x5032; /* dafe_int_scale=3(2^-16), dafe_lin_scale=1(2^-12), dafe_leak=0x50 */
      }
   }
   else
   {
      /* BCM45308-B0 and later */
      /* pilot: dafe_int_scale=5(2^-12), dafe_lin_scale=4(2^-6) */
      /* non-pilot: dafe_int_scale=3(2^-16), dafe_lin_scale=1(2^-12), dafe_leak=0x50 */
      val = 0x00005000;
      if (hChn->acqSettings.symbolRate >= 30000000)
      {
         val |= (4 << BCHP_SDS_CL_0_CLDAFECTL_dafe_lin_scale1_SHIFT);
         val |= (5 << BCHP_SDS_CL_0_CLDAFECTL_dafe_int_scale1_SHIFT);
         val |= (3 << BCHP_SDS_CL_0_CLDAFECTL_dafe_lin_scale0_SHIFT);
         val |= (3 << BCHP_SDS_CL_0_CLDAFECTL_dafe_int_scale0_SHIFT);
      }
      else if (hChn->acqSettings.symbolRate >= 20000000)
      {
         val |= (5 << BCHP_SDS_CL_0_CLDAFECTL_dafe_lin_scale1_SHIFT);
         val |= (7 << BCHP_SDS_CL_0_CLDAFECTL_dafe_int_scale1_SHIFT);
         val |= (4 << BCHP_SDS_CL_0_CLDAFECTL_dafe_lin_scale0_SHIFT);
         val |= (5 << BCHP_SDS_CL_0_CLDAFECTL_dafe_int_scale0_SHIFT);
      }
      else if (hChn->acqSettings.symbolRate >= 10000000)
      {
         val |= (5 << BCHP_SDS_CL_0_CLDAFECTL_dafe_lin_scale1_SHIFT);
         val |= (8 << BCHP_SDS_CL_0_CLDAFECTL_dafe_int_scale1_SHIFT);
         val |= (4 << BCHP_SDS_CL_0_CLDAFECTL_dafe_lin_scale0_SHIFT);
         val |= (6 << BCHP_SDS_CL_0_CLDAFECTL_dafe_int_scale0_SHIFT);
      }
      else if (hChn->acqSettings.symbolRate >= 5000000)
      {
         val |= (5 << BCHP_SDS_CL_0_CLDAFECTL_dafe_lin_scale1_SHIFT);
         val |= (9 << BCHP_SDS_CL_0_CLDAFECTL_dafe_int_scale1_SHIFT);
         val |= (4 << BCHP_SDS_CL_0_CLDAFECTL_dafe_lin_scale0_SHIFT);
         val |= (7 << BCHP_SDS_CL_0_CLDAFECTL_dafe_int_scale0_SHIFT);
      }
      else
      {
         /* under 5 Mbaud */
         val |= (6 << BCHP_SDS_CL_0_CLDAFECTL_dafe_lin_scale1_SHIFT);
         val |= (10 << BCHP_SDS_CL_0_CLDAFECTL_dafe_int_scale1_SHIFT);
         val |= (5 << BCHP_SDS_CL_0_CLDAFECTL_dafe_lin_scale0_SHIFT);
         val |= (8 << BCHP_SDS_CL_0_CLDAFECTL_dafe_int_scale0_SHIFT);
      }
   }
   if (val)
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_HpStateMatch_isr() - callback routine for HP state match interrupt
******************************************************************************/
void BSAT_g1_P_HpStateMatch_isr(void *p, int int_id)
{
   BSAT_ChannelHandle h = (BSAT_ChannelHandle)p;
   BSAT_g1_P_ChannelHandle *hChn = h->pImpl;
   uint32_t t;
#ifndef BSAT_EXCLUDE_AFEC
   uint32_t acm_check, val;
   uint8_t oldScanState, modcod, plscode, i;
#endif

   /* verify that we're in RECEIVER_LOCK state */
   if (BSAT_g1_P_IsHpLocked_isr(h) == false)
   {
      if (BSAT_g1_P_IsTimerExpired_isr(h, BSAT_TimerSelect_eBaud) == false)
      {
         /* re-enable the HP state match irq */
         BINT_ClearCallback_isr(hChn->hHpStateMatchCb);
         BINT_EnableCallback_isr(hChn->hHpStateMatchCb);
      }
      else
      {
         hChn->reacqCause = BSAT_ReacqCause_eHpCouldNotLock;
         BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h); /* give up */
      }
      return;
   }

   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eBaud); /* kill the reacquisition timer */
   BSAT_g1_P_HpDisableInterrupts_isr(h);

   BSAT_g1_P_GetAcquisitionTimerValue_isr(h, &t);

   /* HP is locked */
   BSAT_g1_P_LogTraceBuffer_isr(h, BSAT_TraceEvent_eRcvrLocked);
   /* BDBG_MSG(("HP%d locked in %u usecs", h->channel, t-hChn->count1)); */

#ifndef BSAT_EXCLUDE_AFEC
   if ((BSAT_MODE_IS_DVBS2(hChn->acqSettings.mode)) || (BSAT_MODE_IS_DVBS2X(hChn->acqSettings.mode)))
   {
      acm_check = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK);
      plscode = acm_check & 0x7F;
      if (acm_check & BCHP_SDS_HP_0_ACM_CHECK_modcod_b0_MASK)
         plscode |= 0x80;

      if (hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_ENABLED)
      {
         if (plscode & 0x80)
         {
            /* DVB-S2X */
            modcod = (plscode & 0xFE);
            for (i = 0; i < BSAT_DVBS2X_NUM_MODCODS; i++)
            {
               if (modcod == BSAT_P_dvbs2x_plscode[i])
                  break;
            }
            if (i >= BSAT_DVBS2X_NUM_MODCODS)
               goto invalid_modcod;
            hChn->actualMode = i + BSAT_Mode_eDvbs2x_Qpsk_13_45;

#if BCHP_CHIP==45308
            if (otp_disable_feature & OTP_DISABLE_FEATURE_DVBS2X)
            {
               if (BSAT_MODE_IS_DVBS2X(hChn->actualMode))
               {
                  /* JIRA SWSATFE-756 */
                  not_supported:
                  hChn->reacqCause = BSAT_ReacqCause_eDvbs2xNotSupported;
                  BSAT_g1_P_Reacquire_isr(h);
                  return;
               }
            }
#endif
         }
         else
         {
            /* DVB-S2 */
            hChn->bShortFrame = (plscode & 0x02) ? true : false;
            modcod = (plscode >> 2) & 0x1F;
            if ((modcod == 0) || (modcod >= 29))
            {
               invalid_modcod:
               hChn->bEnableFineFreqEst = false;
               goto reacquire_hp;
            }
            hChn->actualMode = BSAT_Mode_eDvbs2_Qpsk_1_4 + modcod - 1;

#if BCHP_CHIP==45308
            if (otp_disable_feature & OTP_DISABLE_FEATURE_DVBS2X)
            {
               /* SWSATFE-822: do not allow DVB-S2 16/32APSK or QPSK 1/4,1/3,2/5 modes on S2-only variants */
               if (((hChn->actualMode >= BSAT_Mode_eDvbs2_16apsk_2_3) && (hChn->actualMode <= BSAT_Mode_eDvbs2_32apsk_9_10)) ||
                    ((hChn->actualMode >= BSAT_Mode_eDvbs2_Qpsk_1_4) && (hChn->actualMode <= BSAT_Mode_eDvbs2_Qpsk_2_5)))
                  goto not_supported;
            }
#endif
         }

         oldScanState = hChn->dvbs2ScanState;

         if (plscode & 0x01)
            hChn->dvbs2ScanState |= BSAT_DVBS2_SCAN_STATE_PILOT;
         else
            hChn->dvbs2ScanState &= ~BSAT_DVBS2_SCAN_STATE_PILOT;

         hChn->dvbs2ScanState |= BSAT_DVBS2_SCAN_STATE_FOUND;

         BSAT_DEBUG_HP(BDBG_MSG(("Read MODCOD: acm_check=%X, actualMode=0x%X, pilot=%d", acm_check, hChn->actualMode, plscode & 0x01)));

         if ((oldScanState & BSAT_DVBS2_SCAN_STATE_FOUND) == 0)
         {
#if 0
            if (hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_PILOT)
            {
               /* fine freq est was off when we initially locked the HP in modcod scan mode */
               /* reacquire the HP again with fine freq enabled */
               if (BSAT_g1_P_HpOkToEnableFineFreqEst_isr(h))
               {
                  reacquire_hp:
                  BSAT_g1_P_HpAcquire1_isr(h);
                  return;
               }
            }
#else
            if (((hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_PILOT) == 0) && hChn->bEnableFineFreqEst)
            {
               /* fine freq est was enabled when we initially locked the HP in modcod scan mode */
               /* reacquire the HP again with fine freq disabled */
               reacquire_hp:
               BSAT_g1_P_HpAcquire1_isr(h);
               return;
            }
#endif
         }

         /* program new modcod and type */
         val = BCHP_SDS_HP_0_HPCONFIG_use_sw_modcod_type_MASK;
         if (acm_check & BCHP_SDS_HP_0_ACM_CHECK_modcod_b0_MASK)
            val |= BCHP_SDS_HP_0_HPCONFIG_modcod_msb_for_dvbs2x_MASK;
         val |= (((acm_check >> BCHP_SDS_HP_0_ACM_CHECK_modcod_SHIFT) & 0x1F) << BCHP_SDS_HP_0_HPCONFIG_modcod_SHIFT);
         val |= ((acm_check & BCHP_SDS_HP_0_ACM_CHECK_type_MASK) << BCHP_SDS_HP_0_HPCONFIG_type_SHIFT);
         BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, ~0x002000FF, val);

         BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, BCHP_SDS_HP_0_HPCONTROL_acm_mode_delayed_MASK);
         val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG);
         if (plscode & 0x80)
            val |= BCHP_SDS_HP_0_HPCONFIG_modcod_msb_for_dvbs2x_MASK;
         else
            val &= ~BCHP_SDS_HP_0_HPCONFIG_modcod_msb_for_dvbs2x_MASK;
         val &= ~BCHP_SDS_HP_0_HPCONFIG_acm_mode_MASK;
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, val);

      }
      else if ((BSAT_MODE_IS_DVBS2(hChn->acqSettings.mode)) && (hChn->acqSettings.mode != BSAT_Mode_eDvbs2_ACM))
      {
         hChn->bShortFrame = (plscode & 0x02) ? true : false;
      }
   }
#endif

   hChn->nextFunct(h);
}


/******************************************************************************
 BSAT_g1_P_OnHpTimeOut_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_OnHpTimeOut_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, hpstate;
   BERR_Code retCode = BERR_SUCCESS;

   BSAT_DEBUG_HP(BDBG_WRN(("in BSAT_g1_P_OnHpTimeOut_isr")));

   /* check if HP is locked (state 5) */
   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK);
   hpstate = (val >> 8) & 0x07;
   if (hpstate == 5)
   {
      /* for some reason, we didn't get the HP state match irq */
      BSAT_DEBUG_HP(BDBG_WRN(("in BSAT_g1_P_OnHpTimeOut_isr(): locked (HP_ACM_CHECK=0x%X)", val)));
      BSAT_g1_P_HpStateMatch_isr((void*)h, 0);
      return BERR_SUCCESS;
   }

#ifndef BSAT_EXCLUDE_TFEC
   if (hChn->acqSettings.mode == BSAT_Mode_eTurbo_scan)
   {
      if (hChn->turboScanState & BSAT_TURBO_SCAN_STATE_SYNC_ACQUIRED)
      {
         hChn->turboScanLockedModeFailures++;
         if (hChn->turboScanLockedModeFailures > 3)
            hChn->turboScanState &= ~BSAT_TURBO_SCAN_STATE_HP_LOCKED;
      }
      if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
         hChn->turboScanState |= BSAT_TURBO_SCAN_STATE_8PSK_FAILED;
   }
#endif

   hChn->reacqCause = BSAT_ReacqCause_eHpCouldNotLock;
   BSAT_CHK_RETCODE(BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h));

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_HpStateChange_isr() - callback routine for HP state change irq
******************************************************************************/
void BSAT_g1_P_HpStateChange_isr(void *p, int int_id)
{
   /* BSAT_ChannelHandle h = (BSAT_ChannelHandle)p; */
   BSTD_UNUSED(p);
   BSTD_UNUSED(int_id);
}


/******************************************************************************
 BSAT_g1_P_HpFrameBoundary_isr() - callback routine for HP frame boundary
                                   interrupt
******************************************************************************/
void BSAT_g1_P_HpFrameBoundary_isr(void *p, int int_id)
{
   /* BSAT_ChannelHandle h = (BSAT_ChannelHandle)p; */
   BSTD_UNUSED(p);
   BSTD_UNUSED(int_id);
}


/******************************************************************************
 BSAT_g1_P_HpDisableInterrupts_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_HpDisableInterrupts_isr(BSAT_ChannelHandle h)
{
   BERR_Code retCode;
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BSAT_CHK_RETCODE(BINT_DisableCallback_isr(hChn->hHpStateMatchCb));
   BSAT_CHK_RETCODE(BINT_ClearCallback_isr(hChn->hHpStateMatchCb));
   BSAT_CHK_RETCODE(BINT_DisableCallback_isr(hChn->hHpStateChangeCb));
   BSAT_CHK_RETCODE(BINT_ClearCallback_isr(hChn->hHpStateChangeCb));
   BSAT_CHK_RETCODE(BINT_DisableCallback_isr(hChn->hHpFrameBoundaryCb));
   BSAT_CHK_RETCODE(BINT_ClearCallback_isr(hChn->hHpFrameBoundaryCb));

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_IsHpLocked_isr()
******************************************************************************/
bool BSAT_g1_P_IsHpLocked_isr(BSAT_ChannelHandle h)
{
   uint32_t acm_check, hpstate;

   acm_check = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK);
   hpstate = (acm_check & BCHP_SDS_HP_0_ACM_CHECK_hp_state_MASK) >> BCHP_SDS_HP_0_ACM_CHECK_hp_state_SHIFT;
   return (hpstate == 5) ? true : false;
}


/******************************************************************************
 BSAT_g1_P_HpIsSpinv_isrsafe()
******************************************************************************/
bool BSAT_g1_P_HpIsSpinv_isrsafe(BSAT_ChannelHandle h)
{
   uint32_t val;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK);
   if (val & 0x80)
      return true;
   else
      return false;
}


/******************************************************************************
 BSAT_g1_P_HpGetFreqOffsetEstimate()
******************************************************************************/
BERR_Code BSAT_g1_P_HpGetFreqOffsetEstimate(BSAT_ChannelHandle h, int32_t *pFreq)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   int32_t frof1, frof2, frof3, dafeint, carrier_error;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo;

   frof1 = (int32_t)(BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_FROF1));
   frof2 = (int32_t)(BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_FROF2));
   frof3 = (int32_t)(BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_FROF3));
   dafeint = (int32_t)(BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_DAFEINT));
   /* BDBG_MSG(("frof1=%d, frof2=%d, dafeint=%d, frof3=%d", frof1, frof2, dafeint, frof3)); */

   carrier_error = frof1 + frof2;

   /* check DAFE loop bypass and FROF3 accummulation */
   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_FROF3_SW);
   if (val & 0x02) /* use frof3 if fine freq est is enabled */
   {
      carrier_error += frof3;
   }

   carrier_error += dafeint;

   /* multiply by (Fs / 2^32) */
   if (carrier_error >= 0)
      val = (uint32_t)carrier_error;
   else
      val = (uint32_t)-carrier_error;
   BMTH_HILO_32TO64_Mul(val, hChn->sampleFreq / 2, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 2147483648UL, &Q_hi, &Q_lo);
   if (carrier_error >= 0)
      carrier_error = (int32_t)Q_lo;
   else
      carrier_error = (int32_t)-Q_lo;

   if (BSAT_g1_P_HpIsSpinv_isrsafe(h))
      carrier_error = -carrier_error;  /* invert if HP is enabled and spinv detect */

   *pFreq = carrier_error;

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_HpOkToEnableFineFreqEst_isr()
******************************************************************************/
bool BSAT_g1_P_HpOkToEnableFineFreqEst_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if ((hChn->dvbs2Settings.ctl & BSAT_DVBS2_CTL_DISABLE_FINE_FREQ_EST) == 0)
      return true;
   else
      return false;
}


/******************************************************************************
 BSAT_g1_P_HpGetAcmStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_HpGetAcmStatus(BSAT_ChannelHandle h, BSAT_g1_P_AcmStatus *pStatus)
{
   uint32_t val, plscode, modcod, i;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK);
   pStatus->bPilot = (val & 1) ? true : false;
   pStatus->modcod = (val & BCHP_SDS_HP_0_ACM_CHECK_modcod_MASK) >> BCHP_SDS_HP_0_ACM_CHECK_modcod_SHIFT;
   if (val & BCHP_SDS_HP_0_ACM_CHECK_modcod_b0_MASK)
      pStatus->modcod |= 0x20;
   pStatus->type = (uint8_t)(val & BCHP_SDS_HP_0_ACM_CHECK_type_MASK);
   pStatus->bSpinv = (val & BCHP_SDS_HP_0_ACM_CHECK_spinv_MASK) ? true : false;

   plscode = val & 0x7F;
   if (val & BCHP_SDS_HP_0_ACM_CHECK_modcod_b0_MASK)
      plscode |= 0x80;

   if (plscode & 0x80)
   {
      /* DVB-S2X */
      modcod = (plscode & 0xFE);
      for (i = 0; i < BSAT_DVBS2X_NUM_MODCODS; i++)
      {
         if (modcod == BSAT_P_dvbs2x_plscode[i])
            break;
      }
      if (i >= BSAT_DVBS2X_NUM_MODCODS)
         pStatus->mode = BSAT_Mode_eUnknown;
      else
         pStatus->mode = i + BSAT_Mode_eDvbs2x_Qpsk_13_45;
   }
   else
   {
      /* DVB-S2 */
      modcod = (plscode >> 2) & 0x1F;
      if ((modcod == 0) || (modcod >= 29))
         pStatus->mode = BSAT_Mode_eUnknown;
      else
         pStatus->mode = BSAT_Mode_eDvbs2_Qpsk_1_4 + modcod - 1;
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetDvbs2Plscode_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_GetDvbs2Plscode_isrsafe(BSAT_Mode mode, uint8_t *pPlscode)
{
   if ((mode == BSAT_Mode_eDvbs2_ACM) || (mode == BSAT_Mode_eDvbs2_scan))
      return BERR_INVALID_PARAMETER;

   if (BSAT_MODE_IS_DVBS2X(mode))
   {
      int i = mode - BSAT_Mode_eDvbs2x_Qpsk_13_45;
      *pPlscode = BSAT_P_dvbs2x_plscode[i];
   }
   else if (BSAT_MODE_IS_DVBS2(mode))
      *pPlscode = 4 * ((mode - BSAT_Mode_eDvbs2_Qpsk_1_4) + 1);
   else
      return BERR_INVALID_PARAMETER;

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetDvbs2ModeFromPls_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_GetDvbs2ModeFromPls_isrsafe(uint8_t pls, BSAT_Mode *pMode)
{
   uint8_t modcod;

   if (pls & 0x80)
   {
      /* DVB-S2X */
      if (pls <= 175)
         *pMode = (BSAT_Mode_eDvbs2x_Qpsk_13_45 + (pls>>1) - 66);
      else
         *pMode = (BSAT_Mode_eDvbs2x_32apsk_32_45 + (pls>>1) - 89);
   }
   else
   {
      /* classical DVB-S2 */
      modcod = (pls & 0xFE) >> 2;
      *pMode = BSAT_Mode_eDvbs2_Qpsk_1_4 + (modcod - 1);
   }

   return BERR_SUCCESS;
}
