/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g3_priv.h"

#if (BCHP_CHIP == 4538)
#error "This file should not be compiled into the BCM4538 firmware."
#endif

BDBG_MODULE(bast_g3_priv_hp);

#define BAST_DEBUG_HP(x) /* x */

#define BAST_HP_STATE_FIRST_PEAK       0
#define BAST_HP_STATE_PEAK_VERIFY_WAIT 1
#define BAST_HP_STATE_PEAK_VERIFY      2
#define BAST_HP_STATE_RCVR_LOCK_WAIT   3
#define BAST_HP_STATE_RCVR_VERIFY      4
#define BAST_HP_STATE_RCVR_LOCK        5

#define BAST_HP_TIMEOUT 2000000 /* time in baud clocks to wait for rcvr lock */
#define BAST_MODCOD_STABLE_TIMEOUT 150000 /* time in baud clocks to ensure modcod is stable */
#define hp_state_change_MASK 0x01000000 /* BCHP_SDS_INTR2_0_CPU_CLEAR_hp_state_change_MASK */

/* local functions */
BERR_Code BAST_g3_P_HpAcquire1_isr(BAST_ChannelHandle h);
void BAST_g3_P_HpSetFineFrequencyEstimate_isr(BAST_ChannelHandle h);


/******************************************************************************
 BAST_g3_P_HpDisableInterrupts_isr()
******************************************************************************/
BERR_Code BAST_g3_P_HpDisableInterrupts_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BINT_DisableCallback_isr(hChn->hHpLockCb);
   BINT_ClearCallback_isr(hChn->hHpLockCb);

   if (hChn->hHpFrameBoundaryCb)
   {
      BINT_DisableCallback_isr(hChn->hHpFrameBoundaryCb);
      BINT_ClearCallback_isr(hChn->hHpFrameBoundaryCb);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_HpEnableInterrupts_isr()
******************************************************************************/
BERR_Code BAST_g3_P_HpEnableInterrupts_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BINT_ClearCallback_isr(hChn->hHpLockCb);
   BINT_EnableCallback_isr(hChn->hHpLockCb);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_HpEnable_isr()
******************************************************************************/
BERR_Code BAST_g3_P_HpEnable_isr(BAST_ChannelHandle h, bool bEnable)
{
   BERR_Code retCode;
   uint32_t val;

   BAST_CHK_RETCODE(BAST_g3_P_HpDisableInterrupts_isr(h));

   if (bEnable)
   {
      /* enable */
      BAST_CHK_RETCODE(BAST_g3_P_HpEnableInterrupts_isr(h));
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, ~1, 8);
   }
   else
   {
      /* disable FROF1, FROF2, FROF3 */
      val = 0;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FROF1_SW, &val);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FROF2_SW, &val);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FROF3_SW, &val);

      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, ~0x1808, 1); /* acm_mode_delayed = 0, caren=0 */
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, ~0xC000);

      /* assert micro override of HP controls to receiver per Steve */
      val = 0x003FFFFF;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE, &val);
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_HpEnable()
******************************************************************************/
BERR_Code BAST_g3_P_HpEnable(BAST_ChannelHandle h, bool bEnable)
{
   BERR_Code retCode;

   BKNI_EnterCriticalSection();
   retCode = BAST_g3_P_HpEnable_isr(h, bEnable);
   BKNI_LeaveCriticalSection();

   return retCode;
}


#if defined(BCHP_SDS_HP_HP_DAFE)
/******************************************************************************
 BAST_g3_P_HpSetDafeAverage_isr()
******************************************************************************/
void BAST_g3_P_HpSetDafeAverage_isr(BAST_ChannelHandle h, uint8_t dafe_average)
{
   uint32_t val;

   val = 0x20230020 | (dafe_average << 8);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HP_DAFE, &val);
}
#endif


/******************************************************************************
 BAST_g3_P_HpConfig_isr()
******************************************************************************/
BERR_Code BAST_g3_P_HpConfig_isr(BAST_ChannelHandle h)
{
#ifndef BAST_EXCLUDE_LDPC
   static const uint32_t script_hp_config_0[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_8PSK_NORMAL, (22194 << 16) | 21690),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_8PSK_SHORT, (5598 << 16) | 5490),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_QPSK_NORMAL, (33282 << 16) | 32490),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_QPSK_SHORT, (8370 << 16) | 8190),
      BAST_SCRIPT_EXIT
   };
#endif

   static const uint32_t script_hp_config_1[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_DUMMY_NORMAL, (3330 << 16) | 3330),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_16APSK_NORMAL, (16686 << 16) | 16290),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_32APSK_NORMAL, (13338 << 16) | 13050),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_RESERVED_29_NORMAL, (3330 << 16) | 3330),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_RESERVED_30_NORMAL, (3330 << 16) | 3330),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_RESERVED_31_NORMAL, (3330 << 16) | 3330),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_DUMMY_SHORT, (3330 << 16) | 3330),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_16APSK_SHORT, (4212 << 16) | 4140),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_32APSK_SHORT, (3402 << 16) | 3330),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_RESERVED_29_SHORT, (3330 << 16) | 3330),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_RESERVED_30_SHORT, (3330 << 16) | 3330),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_RESERVED_31_SHORT, (3330 << 16) | 3330),
      BAST_SCRIPT_WRITE(BCHP_SDS_HP_FRAME_LENGTH_SAMPLE, 3 * 33282),
#ifndef BAST_ENABLE_FINE_FREQ_ESTIMATION
      BAST_SCRIPT_AND(BCHP_SDS_HP_HPCONFIG, ~0x00080400), /* frof3_accum=0, dafe_loop_bypass=0 */
#endif
      BAST_SCRIPT_EXIT
   };

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
   BERR_Code retCode;
   uint32_t hpconfig, val, P_hi, P_lo, Q_hi, Q_lo, dafe_average;
#ifndef BAST_EXCLUDE_TURBO
   uint16_t turbo_frame_length = 0;
#endif
   uint8_t peak_verify_n_check, peak_verify_m_peak, rcvr_verify_n_check, rcvr_verify_m_peak, rcvr_lock_n_check, rcvr_lock_m_peak;
#ifdef BAST_ENABLE_FINE_FREQ_ESTIMATION
   uint8_t mask;
#endif

   /* set default values */ /* to make maximum DAFE freq average up to 16 PL Frames */
   peak_verify_n_check = 20;
   peak_verify_m_peak = 4;
   rcvr_verify_n_check = 9;
   rcvr_verify_m_peak = 4;
   rcvr_lock_n_check = 9;
   rcvr_lock_m_peak = 4;

   hpconfig = 0x00040000; /* orig: 0x000C0400 */

   if (hDev->sdsRevId < 0x68)
   {
      if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
      {
         dafe_average = ((uint32_t)15 << 20); /* dafe_average must be < peak_verify_n_check */
      }
      else
      {
         /* turbo: set to 1 because frof2 is disabled */
         dafe_average = ((uint32_t)1 << 20); /* dafe_average must be < peak_verify_n_check */

         val = 1;
         BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FROF2_SW, &val); /* disable FROF2 */
      }

      hpconfig |= dafe_average;
   }
   else
   {
#if defined(BCHP_SDS_HP_HP_DAFE)
      dafe_average = 0x0F;
      if (dafe_average > peak_verify_n_check)
         peak_verify_n_check = dafe_average + 1; /* make sure n_check > dafe_average */
      BAST_g3_P_HpSetDafeAverage_isr(h, dafe_average);

      val = 0x000A0401;
#ifndef BAST_EXCLUDE_LDPC
      if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
      {
         if ((hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_ENABLED) ||
             (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_LDPC_PILOT) ||
             (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM))
         {
            /* We typically set no_frequency_estimates_on_state_4 to 1 for dvb-s2 pilot modes.
               However for ACM or non-pilot modes, it is OK to leave this bit set since we'll only
               miss updating FROF3 for one single frame (during state 4) and that's not too bad. */
            val |= 0x10; /* no_frequency_estimates_on_state_4=1 */
         }
      }
#endif /* BAST_EXCLUDE_LDPC */
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_NEW_STATE, &val);
#endif /* defined(BCHP_SDS_HP_HP_DAFE) */
   }

#ifndef BAST_EXCLUDE_LDPC
   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
   {
      hpconfig |= 0x5A001000; /* trnlen = 90, dvbs2_mode */
      if ((hChn->acqParams.mode == BAST_Mode_eLdpc_ACM) ||
          (hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_ENABLED))
      {
         /* ACM */
         hpconfig |= 0x00012300; /* set acm_mode, dcorr_modcod_search, hdr_mode=3 */
      }
      else
      {
         /* CCM, non-modcod search */
         if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_LDPC_SHORT_FRAME)
            hpconfig |= 0x40;

         hpconfig |= 0x00000180; /* hdr_mode=1 */
         if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_LDPC_PILOT)
            hpconfig |= 0x00000020; /* type=1 */

         /* set modcod */
         hpconfig |= ((hChn->actualMode - BAST_Mode_eLdpc_Qpsk_1_2) + 4);
      }
   }
#endif

#ifndef BAST_EXCLUDE_TURBO
   if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
   {
      /* turbo */
      peak_verify_n_check = 20;
      peak_verify_m_peak = 12;
      rcvr_verify_n_check = 20;
      rcvr_verify_m_peak = 14;
      rcvr_lock_n_check = 20;
      rcvr_lock_m_peak = 17;

      hpconfig |= 0x00020081; /* use_sw_modcod_type, set modcod=1, one_of_n_pn_mode */
      if ((hChn->bTurboSpinv) && (hDev->sdsRevId < 0x68))
      {
         hpconfig |= 0x0000C000; /* spinv */
         BAST_DEBUG_HP(BDBG_MSG(("setting hp spinv")));
      }

      if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
         hpconfig |= 0x40000000; /* trnlen = 64 */
      else /* turbo qpsk */
         hpconfig |= 0x80000800; /* trnlen = 128, turbo_qpsk_mode */
   }
#endif

   if (hDev->sdsRevId < 0x68)
   {
      if (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM)
         val = 2<<8; /* ignore_phi_from_dafe=2 per Michael */
      else
         val = 4<<8; /* ignore_phi_from_dafe=4 */
   }
   else
   {
#if defined(BCHP_SDS_HP_IGNORE_PHI_FROM_DAFE)
      if (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM)
         val = 0x00111100; /* states peak_verify and up */
      else
         val = 0x00110000; /* states receiver_verify and up */
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_IGNORE_PHI_FROM_DAFE, &val);
#endif

      val = 0; /* hpcontrol.ignore_phi_from_dafe was deleted in SDS versions 0x68 and later */
   }
   val |= 0x5812E853;
#ifndef BAST_EXCLUDE_TURBO
   if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
   {
      /* set period of 1/N */
      val &= 0xFFFFFF0F;
      if (hChn->actualMode == BAST_Mode_eTurbo_8psk_3_4)
         val |= 0x70;
      else if (hChn->actualMode == BAST_Mode_eTurbo_8psk_8_9)
         val |= 0x60;
      else
         val |= 0x50;
   }
#endif

#ifndef BAST_ENABLE_FINE_FREQ_ESTIMATION
#ifndef BAST_EXCLUDE_LDPC
   if (hDev->sdsRevId >= 0x68)
   {
      if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
      {
         if (BAST_g3_P_IsLdpcPilotOn_isrsafe(h))
            val |= 0x02;
      }
   }
#endif
#endif

   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, &val);

   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, &hpconfig);

#ifndef BAST_EXCLUDE_TURBO
   if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
      val = 1; /* disable FROF3 for turbo */
   else
#endif
   {
      /* LDPC mode */
      val = 0;
#ifdef BAST_ENABLE_FINE_FREQ_ESTIMATION
      if (hDev->sdsRevId >= 0x68)
      {
         if ((hChn->acqParams.mode != BAST_Mode_eLdpc_ACM) &&
             ((hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_ENABLED) == 0) &&
             (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_LDPC_PILOT))
         {
            /* in newer hp design, fine freq estimation can be enabled from the start for ldpc pilot modes */
            enable_fine_freq_est:
            BAST_g3_P_HpSetFineFrequencyEstimate_isr(h);
#if defined(BCHP_SDS_HP_HP_DAFE)
            dafe_average = 0x1F;
            if (dafe_average > peak_verify_n_check)
               peak_verify_n_check = dafe_average + 1; /* make sure n_check > dafe_average */
            BAST_g3_P_HpSetDafeAverage_isr(h, dafe_average);
#endif
            goto after_fine_freq_est_enable;
         }

         mask = BAST_LDPC_SCAN_STATE_ENABLED | BAST_LDPC_SCAN_STATE_FOUND | BAST_LDPC_SCAN_STATE_PILOT;
         if ((hChn->ldpcScanState & mask) == mask)
            goto enable_fine_freq_est; /* modcod scan mode: from previous hp acquisition, we know there is pilot, so enable fine freq est */
      }
#endif
   }
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FROF3_SW, &val);

#ifdef BAST_ENABLE_FINE_FREQ_ESTIMATION
   after_fine_freq_est_enable:
#endif
   BMTH_HILO_32TO64_Mul(1073741824, hChn->acqParams.symbolRate, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &Q_hi, &Q_lo);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FNORM, &Q_lo);

   /* TBD: determine threshold from 100000 symbols and set to about 1/2 peak */
   if (hDev->sdsRevId < 0x68)
      val = 1; /* do this for now... */
   else
      val = 0x100;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_DCORR_THRESHOLD, &val);

   val = peak_verify_m_peak | (peak_verify_n_check << 8);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_M_N_PEAK_VERIFY, &val);

   val = rcvr_verify_m_peak | (rcvr_verify_n_check << 8);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_M_N_RECEIVER_VERIFY, &val);

   val = rcvr_lock_m_peak | (rcvr_lock_n_check << 8);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_M_N_RECEIVER_LOCK, &val);

#ifndef BAST_EXCLUDE_TURBO
   if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
   {
      val = (2564 * 4) + ((hpconfig >> 24) & 0xFF); /* val=turbo_frame_length */
      turbo_frame_length = (uint16_t)val;
   }
#endif
#ifndef BAST_EXCLUDE_LDPC
   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
   {
      val = 33282;  /* use longest frame lengths for initial search (qpsk pilot) */
   }
#endif
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_INITIAL, &val);

#ifndef BAST_EXCLUDE_TURBO
   if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
   {
      val = (uint32_t)turbo_frame_length | (turbo_frame_length << 16);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_8PSK_NORMAL, &val);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_8PSK_SHORT, &val);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_QPSK_NORMAL, &val);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FRAME_LENGTH_QPSK_SHORT, &val);
   }
#endif
#ifndef BAST_EXCLUDE_LDPC
   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
   {
      BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_hp_config_0));
   }
#endif

   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_hp_config_1));

#ifndef BAST_ENABLE_FINE_FREQ_ESTIMATION
   val = 0x0022700F; /* orig: 0x0002700F; */
#ifndef BAST_EXCLUDE_TURBO
   if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
   {
      /* set clfbintrstov(11), clfbrstov(10), clfbfrzov(9), clfbenov(8) */
      val |= 0x00000F00;
      val &= ~0x0002000F; /* enable front carrier loop */
   }
#endif
#else
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, 0xFFFFFFEF);  /* disable front carrier loop */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, 0x00000004);   /* freeze front carrier loop */

   val = 0x0000600F; /* dafe loop filter should be reset by state machine */ /* orig: 0x0000E000; */
#ifndef BAST_EXCLUDE_TURBO
   if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
   {
      /* val |= 0x0F00; */
      val = 0x207F00;
   }
#endif
#endif
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE, &val);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_HpAcquire_isr()
******************************************************************************/
BERR_Code BAST_g3_P_HpAcquire_isr(BAST_ChannelHandle h, BAST_g3_FUNCT funct)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BAST_DEBUG_HP(BDBG_MSG(("in BAST_g3_P_HpAcquire")));

   hChn->passFunct = funct; /* passFunct = function to call after HP locks */
   hChn->count1 = 0;

   BAST_CHK_RETCODE(BAST_g3_P_LogTraceBuffer_isr(h, BAST_TraceEvent_eStartHp));
   BAST_CHK_RETCODE(BAST_g3_P_HpAcquire1_isr(h));

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_HpAcquire1_isr()
******************************************************************************/
BERR_Code BAST_g3_P_HpAcquire1_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   /* configure the HP */
   BAST_CHK_RETCODE(BAST_g3_P_HpEnable_isr(h, false));
   BAST_CHK_RETCODE(BAST_g3_P_HpConfig_isr(h));

   /* interrupt on RECEIVER_LOCK state */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, ~0xF0000000, 0x50000000);

   /* let the HP run */
   BAST_CHK_RETCODE(BAST_g3_P_HpEnable_isr(h, true));

   /* start reacquisition timer */
   retCode = BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, BAST_HP_TIMEOUT, BAST_g3_P_OnHpTimeOut_isr);

#ifndef BAST_EXCLUDE_TURBO
   if (hChn->acqParams.mode == BAST_Mode_eTurbo_scan)
      hChn->turboScanState |= BAST_TURBO_SCAN_STATE_HP_INIT;
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_HpIsSpinv_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_HpIsSpinv_isrsafe(BAST_ChannelHandle h, bool *pSpinv)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
#ifndef BAST_EXCLUDE_TURBO
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
#endif
   uint32_t val;

   *pSpinv = false;
#ifndef BAST_EXCLUDE_LDPC
   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
   {
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK, &val);
      if (val & 0x80)
         *pSpinv = true;
   }
#endif
#ifndef BAST_EXCLUDE_TURBO
   if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
   {
      if (hDev->sdsRevId < 0x68)
      {
         /* software spinv scan */
         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, &val);
         if ((val & 0xC000) == 0xC000)
            *pSpinv = true;
      }
      else
      {
         /* hardware spinv scan */
         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK, &val);
         if (val & 0x80)
            *pSpinv = true;
      }
   }
#endif
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_HpGetFreqOffsetEstimate_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_HpGetFreqOffsetEstimate_isrsafe(BAST_ChannelHandle h, int32_t *pFreq)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   int32_t frof1, frof2, frof3, dafeint, carrier_error;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo;
   bool bSpinv;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_FROF1, (uint32_t*)&frof1);
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_FROF2, (uint32_t*)&frof2);
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_FROF3, (uint32_t*)&frof3);
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_DAFEINT, (uint32_t*)&dafeint);
   /* BDBG_P_PrintWithNewLine("frof1=%d, frof2=%d, dafeint=%d, frof3=%d", frof1, frof2, dafeint, frof3); */

   carrier_error = frof1 + frof2;

   /* check DAFE loop bypass and FROF3 accummulation */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_FROF3_SW, &val);
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

   BAST_CHK_RETCODE(BAST_g3_P_HpIsSpinv_isrsafe(h, &bSpinv));
   if (bSpinv)
      carrier_error = -carrier_error;  /* invert if HP is enabled and spinv detect */

   *pFreq = carrier_error;

   done:
   return BERR_SUCCESS;
}



/******************************************************************************
 BAST_g3_P_OnHpStableLock_isr()
******************************************************************************/
BERR_Code BAST_g3_P_OnHpStableLock_isr(BAST_ChannelHandle h)
{
   BERR_Code retCode;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
#ifndef BAST_EXCLUDE_LDPC
   uint32_t acm_check, modcod, val;
   BAST_Mode actualMode;
   uint8_t oldScanState;
#endif

   /* HP is in RECEIVER_LOCK state */
   BAST_g3_P_DisableTimer_isr(h, BAST_TimerSelect_eBaud); /* kill the reacquisition timer */
   BAST_g3_P_HpDisableInterrupts_isr(h);
   retCode = BAST_g3_P_LogTraceBuffer_isr(h, BAST_TraceEvent_eRcvrLocked);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_WRN(("BAST_g3_P_OnHpStableLock_isr(): BAST_g3_P_LogTraceBuffer_isr() error 0x%X", retCode));
   }
   BDBG_MSG(("HP(chan %d) locked", h->channel));

#ifndef BAST_EXCLUDE_TURBO
   if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
   {
      BAST_g3_P_HpIsSpinv_isrsafe(h, &(hChn->bTurboSpinv));
      BAST_DEBUG_HP(BDBG_MSG(("Turbo SPINV = %d", hChn->bTurboSpinv)));
   }
#endif

#ifndef BAST_EXCLUDE_LDPC
   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
   {
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK, &acm_check);
      if (acm_check & 0x02)
         hChn->ldpcCtl |= BAST_G3_CONFIG_LDPC_CTL_SHORT_FRAME_DETECTED;
      else
         hChn->ldpcCtl &= ~BAST_G3_CONFIG_LDPC_CTL_SHORT_FRAME_DETECTED;

      oldScanState = hChn->ldpcScanState;

      if (hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_ENABLED)
      {
         modcod = ((acm_check >> 2) & 0x1F);
         actualMode = BAST_Mode_eLdpc_Qpsk_1_2 + modcod - 4;
         if ((BAST_MODE_IS_LDPC(actualMode) == 0) || (actualMode == BAST_Mode_eLdpc_scan) || (actualMode == BAST_Mode_eLdpc_ACM))
         {
            BDBG_ERR(("invalid modcod=0x%X, actualMode=0x%X", modcod, actualMode));
            hChn->reacqCause = BAST_ReacqCause_eInvalidModcod;
            return BAST_g3_P_Reacquire_isr(h);
         }
         hChn->actualMode = actualMode;

         hChn->ldpcScanState |= BAST_LDPC_SCAN_STATE_FOUND;
         if (acm_check & 0x01)
            hChn->ldpcScanState |= BAST_LDPC_SCAN_STATE_PILOT;
         else
            hChn->ldpcScanState &= ~BAST_LDPC_SCAN_STATE_PILOT;
         if (BAST_MODE_IS_LDPC_QPSK(hChn->actualMode))
            hChn->ldpcScanState |= BAST_LDPC_SCAN_STATE_QPSK;
         else
            hChn->ldpcScanState &= ~BAST_LDPC_SCAN_STATE_QPSK;

#ifdef BAST_ENABLE_FINE_FREQ_ESTIMATION
         if ((oldScanState & BAST_LDPC_SCAN_STATE_FOUND) == 0)
         {
            if (hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_PILOT)
            {
               /* fine freq est was off when we initially locked the HP in modcod scan mode */
               /* reacquire the HP again with fine freq enabled */
               return BAST_g3_P_HpAcquire1_isr(h);
            }
         }
#endif


         BAST_DEBUG_HP(BDBG_MSG(("Read MODCOD: actualMode=0x%X, pilot=%d", hChn->actualMode, acm_check & 0x01)));

         /* program new modcod and type */
         val = 0x80 | modcod | ((acm_check & 0x03) << 5);
         BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, ~0x7F, val);

         BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, 0x1000); /* set acq_mode_delayed */
         BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, ~0x2000); /* acm_mode=0 */
      }
   }
#endif

   return hChn->passFunct(h);
}


#ifndef BAST_EXCLUDE_LDPC
/******************************************************************************
 BAST_g3_P_HpCheckStableLock_isr()
******************************************************************************/
BERR_Code BAST_g3_P_HpCheckStableLock_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val;
   uint8_t modcod;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_STATUS, &val);
   if (val & hp_state_change_MASK)
   {
      check_again:
      if (hChn->count1 < 1)
      {
         hChn->count1++;
         BAST_DEBUG_HP(BDBG_MSG(("modcod not stable after %d bclks", BAST_MODCOD_STABLE_TIMEOUT)));
         if ((retCode = BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, BAST_MODCOD_STABLE_TIMEOUT, BAST_g3_P_HpCheckStableLock_isr)) != BERR_SUCCESS)
         {
            hChn->reacqCause = BAST_ReacqCause_eTimerError1;
            goto reacquire;
         }
      }
      else
      {
         hChn->reacqCause = BAST_ReacqCause_eModcodNotStable;

         reacquire:
         return BAST_g3_P_Reacquire_isr(h);
      }
   }
   else
   {
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK, &val);
      modcod = (uint8_t)(val & 0xFF);
      if (modcod != hChn->modcod)
      {
         hChn->modcod = modcod;
         BAST_DEBUG_HP(BDBG_MSG(("modcod changed")));
         goto check_again;
      }
      retCode = BAST_g3_P_OnHpStableLock_isr(h);
   }

   return retCode;
}
#endif


/******************************************************************************
 BAST_g3_P_HpSetFineFrequencyEstimate_isr()
******************************************************************************/
void BAST_g3_P_HpSetFineFrequencyEstimate_isr(BAST_ChannelHandle h)
{
   uint32_t val;

   val = 0x2;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_FROF3_SW, &val); /* use fine freq */
   val = 0x6509A;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, &val);
   BAST_DEBUG_HP(BDBG_MSG(("enable fine frequency estimate")));
}


/******************************************************************************
 BAST_g3_P_HpFrameBoundary_isr()
******************************************************************************/
void BAST_g3_P_HpFrameBoundary_isr(void *p, int param)
{
   BSTD_UNUSED(p);
   BSTD_UNUSED(param);
}


/******************************************************************************
 BAST_g3_P_HpLock_isr()
******************************************************************************/
void BAST_g3_P_HpLock_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g3_P_ChannelHandle *hChn = h->pImpl;
#ifndef BAST_EXCLUDE_LDPC
   uint32_t val;
#endif

   BSTD_UNUSED(param);

#ifdef BAST_LOG_REG_ACCESS
   BKNI_Printf("hp lock isr\n");
#endif

   /* verify that we're in RECEIVER_LOCK state */
   if (BAST_g3_P_IsHpLocked_isrsafe(h) == false)
   {
#ifndef BAST_EXCLUDE_LDPC
      hp_not_locked:
#endif
      if (BAST_g3_P_IsTimerExpired_isrsafe(h, BAST_TimerSelect_eBaud) == false)
         BAST_g3_P_HpEnableInterrupts_isr(h);
      else
      {
         hChn->reacqCause = BAST_ReacqCause_eHpCouldNotLock;
         BAST_g3_P_Reacquire_isr(h); /* give up */
      }
      return;
   }

#ifndef BAST_EXCLUDE_LDPC
   /* make sure modcod is stable for a few frames (~150000 baud clocks) */
   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
   {
      val = hp_state_change_MASK;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_CLEAR, &val);
      if (BAST_g3_P_IsHpLocked_isrsafe(h) == false)
         goto hp_not_locked;

      BAST_g3_P_DisableTimer_isr(h, BAST_TimerSelect_eBaud); /* kill the reacquisition timer */
      BAST_g3_P_HpDisableInterrupts_isr(h);

      if (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM)
         BAST_g3_P_OnHpStableLock_isr(h);
      else
      {
         /* save the modcod */
         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK, &val);
         hChn->modcod = (uint8_t)(val & 0xFF);

         hChn->count1 = 0;
         if (BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, BAST_MODCOD_STABLE_TIMEOUT, BAST_g3_P_HpCheckStableLock_isr) != BERR_SUCCESS)
         {
            hChn->reacqCause = BAST_ReacqCause_eTimerError2;
            BAST_g3_P_Reacquire_isr(h);
         }
      }
      return;
   }
#endif

#ifndef BAST_EXCLUDE_TURBO
   /* this is assumed to be turbo */
   BAST_g3_P_DisableTimer_isr(h, BAST_TimerSelect_eBaud); /* kill the reacquisition timer */
   BAST_g3_P_HpDisableInterrupts_isr(h);
   BAST_g3_P_OnHpStableLock_isr(h);
#endif
}


/******************************************************************************
 BAST_g3_P_OnHpTimeOut_isr()
******************************************************************************/
BERR_Code BAST_g3_P_OnHpTimeOut_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
#ifndef BAST_EXCLUDE_TURBO
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
#endif
   uint32_t val, hpstate;
   BERR_Code retCode = BERR_SUCCESS;

   BAST_DEBUG_HP(BDBG_WRN(("in BAST_g3_P_OnHpTimeOut_isr")));

   /* check if HP is locked */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK, &val);
   hpstate = (val >> 8) & 0x07;
   if (hpstate == BAST_HP_STATE_RCVR_LOCK)
   {
      BAST_DEBUG_HP(BDBG_WRN(("in BAST_g3_P_OnHpTimeOut_isr(): locked (HP_ACM_CHECK=0x%X)", val)));
      BAST_g3_P_HpLock_isr((void*)h, 0);
      return BERR_SUCCESS;
   }

#ifndef BAST_EXCLUDE_TURBO
   if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
   {
      if (hDev->sdsRevId < 0x68)
      {
         /* versions older than 6.8 dont have turbo spinv scan h/w */
         if ((hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_SPINV_IQ_SCAN) == BAST_ACQSETTINGS_SPINV_IQ_SCAN)
         {
            if (hChn->bTurboSpinv)
               hChn->bTurboSpinv = false;
            else
            {
               hChn->bTurboSpinv = true;
               BAST_CHK_RETCODE(BAST_g3_P_HpEnable_isr(h, false));
               return BAST_g3_P_Acquire1_isr(h);
            }
         }
      }
   }

   if (hChn->acqParams.mode == BAST_Mode_eTurbo_scan)
   {
      if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
         hChn->turboScanState |= BAST_TURBO_SCAN_STATE_8PSK_FAILED;
   }
#endif

   hChn->reacqCause = BAST_ReacqCause_eHpCouldNotLock;
   BAST_CHK_RETCODE(BAST_g3_P_Reacquire_isr(h));

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_IsHpLocked_isrsafe()
******************************************************************************/
bool BAST_g3_P_IsHpLocked_isrsafe(BAST_ChannelHandle h)
{
   uint32_t acm_check, hpstate;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_ACM_CHECK, &acm_check);
   hpstate = (acm_check >> 8) & 0x07;
   return (hpstate == BAST_HP_STATE_RCVR_LOCK) ? true : false;
}
