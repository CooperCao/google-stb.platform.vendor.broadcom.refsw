/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


BDBG_MODULE(bsat_g1_priv_hp);

#define BSAT_DEBUG_HP(x) /* x */

#define BSAT_HP_TIMEOUT 2000000 /* time in baud clocks to wait for rcvr lock */

/* local functions */
static BERR_Code BSAT_g1_P_HpAcquire1_isr(BSAT_ChannelHandle h);
static bool BSAT_g1_P_HpOkToEnableFineFreqEst_isr(BSAT_ChannelHandle h);


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
 BSAT_g1_P_IsHpLocked_isr()
******************************************************************************/
bool BSAT_g1_P_IsHpLocked_isr(BSAT_ChannelHandle h)
{
   uint32_t acm_check, hpstate;

   acm_check = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK);
   hpstate = (acm_check >> 8) & 0x07;
   return (hpstate == 5) ? true : false;
}


/******************************************************************************
 BSAT_g1_P_HpStateMatch_isr() - callback routine for HP state match interrupt
******************************************************************************/
void BSAT_g1_P_HpStateMatch_isr(void *p, int int_id)
{
   BSAT_ChannelHandle h = (BSAT_ChannelHandle)p;
   BSAT_g1_P_ChannelHandle *hChn = h->pImpl;
#ifndef BSAT_EXCLUDE_AFEC
   uint32_t acm_check, modcod, val;
   uint8_t oldScanState;
#endif

   BSTD_UNUSED(int_id);

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

   /* HP is locked */
   BSAT_g1_P_LogTraceBuffer_isr(h, BSAT_TraceEvent_eRcvrLocked);
   BDBG_MSG(("HP%d locked", h->channel));

#ifndef BSAT_EXCLUDE_AFEC
   if (BSAT_MODE_IS_DVBS2(hChn->acqSettings.mode))
   {
      acm_check = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK);
      if (acm_check & 0x02)
         hChn->bShortFrame = true;
      else
         hChn->bShortFrame = false;

      oldScanState = hChn->dvbs2ScanState;

      if (hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_ENABLED)
      {
         modcod = ((acm_check >> 2) & 0x1F);
         if (modcod == 0)
         {
            /* invalid modcod */
            hChn->bEnableFineFreqEst = false;
            goto reacquire_hp;
         }
         hChn->actualMode = BSAT_Mode_eDvbs2_Qpsk_1_4 + modcod - 1;
         hChn->dvbs2ScanState |= BSAT_DVBS2_SCAN_STATE_FOUND;
         if (acm_check & 0x01)
            hChn->dvbs2ScanState |= BSAT_DVBS2_SCAN_STATE_PILOT;
         else
            hChn->dvbs2ScanState &= ~BSAT_DVBS2_SCAN_STATE_PILOT;
         if (BSAT_MODE_IS_DVBS2_QPSK(hChn->actualMode))
            hChn->dvbs2ScanState |= BSAT_DVBS2_SCAN_STATE_QPSK;
         else
            hChn->dvbs2ScanState &= ~BSAT_DVBS2_SCAN_STATE_QPSK;

         BSAT_DEBUG_HP(BDBG_MSG(("Read MODCOD: actualMode=0x%X, pilot=%d", hChn->actualMode, acm_check & 0x01)));

         if ((oldScanState & BSAT_DVBS2_SCAN_STATE_FOUND) == 0)
         {
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
         }

         /* program new modcod and type */
         val = 0x80 | modcod | ((acm_check & 0x03) << 5);
         BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, ~0x7F, val);

         BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, 0x1000); /* set acq_mode_delayed */
         BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, ~0x2000); /* acm_mode=0 */
      }
   }
#endif

   hChn->nextFunct(h);
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

      BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, ~0x1808, 1); /* acm_mode_delayed = 0, caren=0 */
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, ~0xC000);

      /* assert micro override of HP controls to receiver per Steve */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE, 0x003FFFFF);
   }

   done:
   return retCode;
}



/******************************************************************************
 BSAT_g1_P_HpSetDafeAverage_isr()
******************************************************************************/
static void BSAT_g1_P_HpSetDafeAverage_isr(BSAT_ChannelHandle h, uint8_t dafe_average)
{
   uint32_t val;

   val = 0x20230022 | (dafe_average << 8); /* use HNS coeffs for DAFE */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HP_DAFE, val);
}


/******************************************************************************
 BSAT_g1_P_HpConfig_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_HpConfig_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t hpconfig, val, P_hi, P_lo, Q_hi, Q_lo, dafe_average;
   uint16_t turbo_frame_length = 0;
   uint8_t peak_verify_n_check, peak_verify_m_peak, rcvr_verify_n_check, rcvr_verify_m_peak, rcvr_lock_n_check, rcvr_lock_m_peak;
#ifndef BSAT_EXCLUDE_AFEC
   uint8_t mask;
#endif

   /* set default values */ /* to make maximum DAFE freq average up to 16 PL Frames */
   peak_verify_n_check = 20;
   peak_verify_m_peak = 4;
   rcvr_verify_n_check = 9;
   rcvr_verify_m_peak = 4;
   rcvr_lock_n_check = 9;
   rcvr_lock_m_peak = 4;
   dafe_average = 0x0F;

   hpconfig = 0x00040000; /* orig: 0x000C0400 */

   /* set HP_NEW_STATE */
   val = 0x000A0401;
#ifndef BSAT_EXCLUDE_AFEC
   if (BSAT_MODE_IS_DVBS2(hChn->acqSettings.mode))
   {
      if ((hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_ENABLED) ||
          (hChn->acqSettings.options & BSAT_ACQ_PILOT) ||
          (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM))
      {
         /* We typically set no_frequency_estimates_on_state_4 to 1 for dvb-s2 pilot modes.
            However for ACM or non-pilot modes, it is OK to leave this bit set since we'll only
            miss updating FROF3 for one single frame (during state 4) and that's not too bad. */
         val |= 0x10; /* no_frequency_estimates_on_state_4=1 */
      }
   }
#endif
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_NEW_STATE, val);

#ifndef BSAT_EXCLUDE_AFEC
   if (BSAT_MODE_IS_DVBS2(hChn->acqSettings.mode))
   {
      hpconfig |= 0x5A001000; /* trnlen = 90, dvbs2_mode */
      if ((hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM) ||
          (hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_ENABLED))
      {
         /* modcod scan or ACM */
         hpconfig |= 0x00012300; /* set acm_mode, dcorr_modcod_search, hdr_mode=3 */

		 /* QPSK 1/2 scan mode has problems, so ease the lock criteria */
         rcvr_lock_n_check = 10;
         rcvr_lock_m_peak = 2;
      }
      else
      {
         if (hChn->acqSettings.options & BSAT_ACQ_DVBS2_SHORT_FRAMES)
            hpconfig |= 0x40;

         /* CCM, non-modcod search */
         hpconfig |= 0x00000180; /* hdr_mode=1 */
         if (hChn->acqSettings.options & BSAT_ACQ_PILOT)
            hpconfig |= 0x00000020; /* type=1 */

         /* set modcod */
         hpconfig |= ((hChn->actualMode - BSAT_Mode_eDvbs2_Qpsk_1_4) + 1);
      }
   }
#endif
#ifndef BSAT_EXCLUDE_TFEC
   if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
   {
      /* turbo */
      peak_verify_n_check = 20;
      peak_verify_m_peak = 12;
      rcvr_verify_n_check = 20;
      rcvr_verify_m_peak = 14;
      rcvr_lock_n_check = 20;
      rcvr_lock_m_peak = 17;

      hpconfig |= 0x00020081; /* use_sw_modcod_type, set modcod=1, one_of_n_pn_mode */
      if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
         hpconfig |= 0x40000000; /* trnlen = 64 */
      else /* turbo qpsk */
         hpconfig |= 0x80000800; /* trnlen = 128, turbo_qpsk_mode */
   }
#endif

   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
      val = 0x00111100; /* states peak_verify and up */
   else
      val = 0x00110000; /* states receiver_verify and up */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_IGNORE_PHI_FROM_DAFE, val);

   val = 0x5812E853;
#ifndef BSAT_EXCLUDE_TFEC
   if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
   {
      /* set period of 1/N */
      val &= 0xFFFFFF0F;
      if (hChn->actualMode == BSAT_Mode_eTurbo_8psk_3_4)
         val |= 0x70;
      else if (hChn->actualMode == BSAT_Mode_eTurbo_8psk_8_9)
         val |= 0x60;
      else
         val |= 0x50;
   }
#endif

#ifndef BSAT_EXCLUDE_AFEC
   if (BSAT_MODE_IS_DVBS2(hChn->acqSettings.mode))
   {
      if (BSAT_g1_P_AfecIsPilot_isr(h))
         val |= 0x02;
   }
#endif

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, val);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, hpconfig);

#ifndef BSAT_EXCLUDE_TFEC
   if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
      val = 1; /* disable FROF3 for turbo */
#endif
#ifndef BSAT_EXCLUDE_AFEC
   if (BSAT_MODE_IS_DVBS2(hChn->acqSettings.mode))
   {
      /* DVB-S2 mode */
      val = 0;
      if ((hChn->acqSettings.mode != BSAT_Mode_eDvbs2_ACM) &&
          ((hChn->dvbs2ScanState & BSAT_DVBS2_SCAN_STATE_ENABLED) == 0) &&
          (hChn->acqSettings.options & BSAT_ACQ_PILOT))
      {
         dvbs2_pilot:
         hChn->bEnableFineFreqEst = BSAT_g1_P_HpOkToEnableFineFreqEst_isr(h);
         if (hChn->bEnableFineFreqEst == false)
            goto write_frof3_sw;

         /* enable fine freq estimation for dvbs2 pilot modes */
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FROF3_SW, 2); /* use fine freq */
         val = 0x6500E; /* dafe_int_scale=0, dafe_lin_scale=7 */
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, 0x6500E);
         dafe_average = 0x0F; /* to reduce ACQ time for LDPC 8PSK 3/4 and 5/6 */
         BSAT_DEBUG_HP(BDBG_MSG(("enable fine frequency estimate")));

         goto after_fine_freq_est_enable;
      }

      mask = BSAT_DVBS2_SCAN_STATE_ENABLED | BSAT_DVBS2_SCAN_STATE_FOUND | BSAT_DVBS2_SCAN_STATE_PILOT;
      if ((hChn->dvbs2ScanState & mask) == mask)
         goto dvbs2_pilot; /* modcod scan mode: from previous hp acquisition, we know there is pilot, so enable fine freq est */
   }
#endif
   write_frof3_sw:
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FROF3_SW, val);

#ifndef BSAT_EXCLUDE_AFEC
   after_fine_freq_est_enable:
#endif
   if (dafe_average > peak_verify_n_check)
      peak_verify_n_check = dafe_average + 1; /* make sure n_check > dafe_average */
   BSAT_g1_P_HpSetDafeAverage_isr(h, dafe_average);

   BMTH_HILO_32TO64_Mul(1073741824, hChn->acqSettings.symbolRate, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &Q_hi, &Q_lo);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FNORM, Q_lo);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_DCORR_THRESHOLD, 1);

   val = peak_verify_m_peak | (peak_verify_n_check << 8);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_M_N_PEAK_VERIFY, val);

   val = rcvr_verify_m_peak | (rcvr_verify_n_check << 8);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_M_N_RECEIVER_VERIFY, val);

   val = rcvr_lock_m_peak | (rcvr_lock_n_check << 8);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_M_N_RECEIVER_LOCK, val);

#ifndef BSAT_EXCLUDE_TFEC
   if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
   {
      val = (2564 * 4) + ((hpconfig >> 24) & 0xFF); /* val=turbo_frame_length */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_INITIAL, val);

      turbo_frame_length = (uint16_t)val;
      val = (uint32_t)turbo_frame_length | (turbo_frame_length << 16);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_8PSK_NORMAL, val);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_8PSK_SHORT, val);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_QPSK_NORMAL, val);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_QPSK_SHORT, val);
   }
#endif
#ifndef BSAT_EXCLUDE_AFEC
   if (BSAT_MODE_IS_DVBS2(hChn->acqSettings.mode))
   {
      /* use longest frame lengths for initial search (qpsk pilot) */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_INITIAL, 33282);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_8PSK_NORMAL, (22194 << 16) | 21690);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_8PSK_SHORT, (5598 << 16) | 5490);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_QPSK_NORMAL, (33282 << 16) | 32490);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_QPSK_SHORT, (8370 << 16) | 8190);
   }
#endif

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_DUMMY_NORMAL, (3330 << 16) | 3330);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_16APSK_NORMAL, (16686 << 16) | 16290);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_32APSK_NORMAL, (13338 << 16) | 13050);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_RESERVED_29_NORMAL, (3330 << 16) | 3330);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_RESERVED_30_NORMAL, (3330 << 16) | 3330);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_RESERVED_31_NORMAL, (3330 << 16) | 3330);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_DUMMY_SHORT, (3330 << 16) | 3330);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_16APSK_SHORT, (4212 << 16) | 4140);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_32APSK_SHORT, (3402 << 16) | 3330);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_RESERVED_29_SHORT, (3330 << 16) | 3330);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_RESERVED_30_SHORT, (3330 << 16) | 3330);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_RESERVED_31_SHORT, (3330 << 16) | 3330);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_SAMPLE, 3 * 33282);

   if (hChn->bEnableFineFreqEst == false)
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, ~0x00080400); /* frof3_accum=0, dafe_loop_bypass=0 */

   if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, 0xFFFFFFEF);
   else
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, 0xFFF7FFEF);
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, 0x00000004);   /* freeze front carrier loop */

   val = BCHP_FIELD_DATA(SDS_HP_0_HPOVERRIDE, VLCFRZOV, 1);
   BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, VLCRSTOV, 1);
   BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, EQFRZQOV, 1);
   BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, EQFRZIOV, 1);
   BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, EQFRZOV, 1);
   BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, EQRSTOV, 1);
   BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, EQMODEOV, 1);
   if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
   {
      BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, SNOREOV, 1);
      BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, CLFBENOV, 1);
      BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, CLFBFRZOV, 1);
      BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, CLFBRSTOV, 1);
      BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, CLFBINTRSTOV, 1);
      BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, VLCENOV, 1);
   }
   else
   {
      BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, CLFINTRSTOV, 1);
      BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, CLFENOV, 1);
      BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, CLFFRZOV, 1);
      BCHP_SET_FIELD_DATA(val, SDS_HP_0_HPOVERRIDE, CLFRSTOV, 1);
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE, val);

   return retCode;
}


/******************************************************************************
 BSAT_g1_P_OnHpTimeOut_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_OnHpTimeOut_isr(BSAT_ChannelHandle h)
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
static BERR_Code BSAT_g1_P_HpAcquire1_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   /* configure the HP */
   BSAT_CHK_RETCODE(BSAT_g1_P_HpEnable_isr(h, false));
   BSAT_CHK_RETCODE(BSAT_g1_P_HpConfig_isr(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_AfecSetScramblingSeq_isr(h));

   /* interrupt on HP state match 5 (RECEIVER_LOCK) */
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, ~0xF0000000, 0x50000000);

   /* let the HP run */
   BSAT_CHK_RETCODE(BSAT_g1_P_HpEnable_isr(h, true));

   /* start reacquisition timer */
   retCode = BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaud, BSAT_HP_TIMEOUT, BSAT_g1_P_OnHpTimeOut_isr);

#ifndef BSAT_EXCLUDE_TFEC
   if (hChn->acqSettings.mode == BSAT_Mode_eTurbo_scan)
      hChn->turboScanState |= BSAT_TURBO_SCAN_STATE_HP_INIT;
#endif

   done:
   return retCode;
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
static bool BSAT_g1_P_HpOkToEnableFineFreqEst_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   bool bEnable = false;

   if ((hChn->dvbs2Settings.ctl & BSAT_DVBS2_CTL_DISABLE_FINE_FREQ_EST) == 0)
   {
#if 0
      /* fine freq. est. is turned on for drift tracking */
      if ((hChn->actualMode == BSAT_Mode_eDvbs2_8psk_3_4) || (hChn->actualMode == BSAT_Mode_eDvbs2_8psk_5_6))
#endif
         bEnable = true;
   }

   return bEnable;
}


/******************************************************************************
 BSAT_g1_P_HpGetAcmStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_HpGetAcmStatus(BSAT_ChannelHandle h, BSAT_g1_P_AcmStatus *pStatus)
{
   uint32_t val;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK);
   pStatus->modcod = (uint8_t)((val >> 2) & 0x1F);
   pStatus->type = (uint8_t)(val & 0x3);
   pStatus->bSpinv = val & 0x80 ? true : false;
   pStatus->mode = BSAT_Mode_eDvbs2_Qpsk_1_4 + pStatus->modcod - 1;
   return BERR_SUCCESS;
}
