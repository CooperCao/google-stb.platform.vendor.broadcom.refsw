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

BDBG_MODULE(bast_g3_priv_qpsk);

#define BAST_DEBUG_QPSK(x) /* x */

/* bit definitions for dtvScanState */
#define BAST_G3_DTV_SCAN_ON  0x80
#define BAST_G3_DTV_SCAN_1_2 0x01
#define BAST_G3_DTV_SCAN_OFF 0x00

/* local function prototypes */
BERR_Code BAST_g3_P_QpskInitializeLoopParameters_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskSetMode_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskLockViterbi_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskSetActualMode_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskSetVcos_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskSpInvScan_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskSpinvScan1_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskNarrowBw_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskNarrowBw1_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskAcquire1_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskSetFinalFlBw_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskSetFinalBlBw_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskSetOqpsk_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskSetOpll_isr(BAST_ChannelHandle h);


/* private functions */
/******************************************************************************
 BAST_g3_P_QpskAcquire_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g3_P_QpskAcquire_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   if (hChn->acqParams.mode == BAST_Mode_eDss_scan)
      hChn->dtvScanState = BAST_G3_DTV_SCAN_ON;
   else
      hChn->dtvScanState = BAST_G3_DTV_SCAN_OFF;

   /* set qpsk mode and init bert */
   BAST_g3_P_QpskSetMode_isr(h);
   BAST_g3_P_InitBert_isr(h);

   BAST_g3_P_QpskInitializeLoopParameters_isr(h);
   BAST_CHK_RETCODE(BAST_g3_P_LogTraceBuffer_isr(h, BAST_TraceEvent_eStartViterbi));

   hChn->funct_state = 0;
   return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, 20000, BAST_g3_P_QpskLockViterbi_isr);

   done:
   return retCode;
}

/******************************************************************************
 BAST_g3_P_QpskUpdateErrorCounters_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_QpskUpdateErrorCounters_isrsafe(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_FEC_FERR, &val);
   hChn->rsCorr += ((val >> 16) & 0xFFFF);
   hChn->rsUncorr += (val & 0xFFFF);

   /* get pre-Viterbi error count */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_VIT_VTCTL, 0x00008000); /* take snapshot of VRCV+VREC */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VREC, &val);
   hChn->preVitErrors += val;

   BAST_g3_P_UpdateErrorCounters_isrsafe(h);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_QpskCheckMode_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g3_P_QpskCheckMode_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BAST_Mode acq_mode = hChn->acqParams.mode;
   uint8_t base, idx, enable_mask, mode_mask;

   if ((acq_mode == BAST_Mode_eDss_scan) || (acq_mode == BAST_Mode_eDvb_scan) ||
       (acq_mode == BAST_Mode_eDcii_scan))
   {
      if (BAST_MODE_IS_DVB(hChn->actualMode))
      {
         base = BAST_Mode_eDvb_1_2;
         enable_mask = hChn->dvbScanCodeRates;
      }
      else if (BAST_MODE_IS_DTV(hChn->actualMode))
      {
         base = BAST_Mode_eDss_1_2;
         enable_mask = hChn->dtvScanCodeRates;
      }
      else
      {
         base = BAST_Mode_eDcii_1_2;
         enable_mask = hChn->dciiScanCodeRates;
      }
      idx = hChn->actualMode - base;
      mode_mask = 1 << idx;
      if ((mode_mask & enable_mask) == 0)
      {
         hChn->reacqCause = BAST_ReacqCause_eInvalidMode;
         hChn->reacqCtl |= BAST_G3_CONFIG_REACQ_CTL_FORCE; /* invalid mode */
      }
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_DisableFreqDetector_isr()
******************************************************************************/
BERR_Code BAST_g3_P_DisableFreqDetector_isr(BAST_ChannelHandle h)
{
#if 0
   uint32_t val;
#endif

   BAST_DEBUG_QPSK(BDBG_MSG(("disabling freq detector")));
#if 0
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, &val);
   val &= 0xFF00FFFF;
   val |= 0x000A0000;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, &val);
#else
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, ~0x00400000);
#endif
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_QpskGetRTLockStatus_isr
******************************************************************************/
void BAST_g3_P_QpskGetRTLockStatus_isr(BAST_ChannelHandle h, bool *pbLocked)
{
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_INTR_RAW_STS0, &val);
   if ((val & 0x1B) == 0x09)
      *pbLocked = true;
   else
      *pbLocked = false;
}


/******************************************************************************
 BAST_g3_P_QpskOnLock_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g3_P_QpskOnLock_isr(BAST_ChannelHandle h)
{
#if (BCHP_CHIP!=4528)
   /* disable freq detector 30 msecs after lock */
   BAST_DEBUG_QPSK(BDBG_MSG(("starting 30 msec timer for freq detector disable")));
   return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 30000, BAST_g3_P_DisableFreqDetector_isr);
#else
   return BERR_SUCCESS;
#endif
}


/******************************************************************************
 BAST_g3_P_QpskOnLostLock_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g3_P_QpskOnLostLock_isr(BAST_ChannelHandle h)
{
#if 0
   uint32_t val;
#endif

   BAST_g3_P_DisableTimer_isr(h, BAST_TimerSelect_eBaudUsec);

   /* re-enable frequency detector */
#if 0
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, &val);
   val &= 0xFF00FFFF;
   val |= 0x004A0000;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, &val);
#else
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, 0x00400000);
#endif

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_QpskOnStableLock_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g3_P_QpskOnStableLock_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BAST_Mode prev_mode = hChn->actualMode;
   BAST_Mode curr_mode;
   BERR_Code retCode;

   if (hChn->bPlcTracking == false)
      BAST_g3_P_ConfigPlc_isr(h, false); /* set tracking PLC */

#if (BCHP_CHIP==4528) || (BCHP_CHIP==4538)
   BAST_g3_P_DisableFreqDetector_isr(h);
#endif

   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 0x00FFFFFF, 0x69000000); /* change mu for tracking */

   BAST_CHK_RETCODE(BAST_g3_P_GetActualMode_isr(h, &curr_mode));
   if (prev_mode != curr_mode)
   {
      /* different code rate now detected */
      BDBG_MSG(("diff code rate detected"));
      hChn->reacqCtl |= BAST_G3_CONFIG_REACQ_CTL_FORCE;
      hChn->reacqCause = BAST_ReacqCause_eCodeRateChanged;
   }
   else if (hChn->bEverStableLock == false)
   {
      BAST_g3_P_QpskUpdateErrorCounters_isrsafe(h);
      hChn->rsCorr = hChn->rsUncorr = 0;
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_QpskEnableLockInterrupts_isr()
******************************************************************************/
BERR_Code BAST_g3_P_QpskEnableLockInterrupts_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BINT_ClearCallback_isr(hChn->hQpskLockCb);
   BINT_ClearCallback_isr(hChn->hQpskNotLockCb);

   BINT_EnableCallback_isr(hChn->hQpskLockCb);
   BINT_EnableCallback_isr(hChn->hQpskNotLockCb);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_QpskDisableLockInterrupts_isr()
******************************************************************************/
BERR_Code BAST_g3_P_QpskDisableLockInterrupts_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BINT_DisableCallback_isr(hChn->hQpskLockCb);
   BINT_DisableCallback_isr(hChn->hQpskNotLockCb);

   BINT_ClearCallback_isr(hChn->hQpskLockCb);
   BINT_ClearCallback_isr(hChn->hQpskNotLockCb);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_QpskSetFunctTable_isr()
******************************************************************************/
BERR_Code BAST_g3_P_QpskSetFunctTable_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   hChn->acqFunct = BAST_g3_P_QpskAcquire_isr;
   hChn->checkModeFunct = BAST_g3_P_QpskCheckMode_isr;
   hChn->onLockFunct = BAST_g3_P_QpskOnLock_isr;
   hChn->onLostLockFunct = BAST_g3_P_QpskOnLostLock_isr;
   hChn->onStableLockFunct = BAST_g3_P_QpskOnStableLock_isr;
   hChn->onMonitorLockFunct = NULL;
   hChn->enableLockInterrupts = BAST_g3_P_QpskEnableLockInterrupts_isr;
   hChn->disableLockInterrupts = BAST_g3_P_QpskDisableLockInterrupts_isr;
   hChn->getLockStatusFunct = BAST_g3_P_QpskGetRTLockStatus_isr;

   return BERR_SUCCESS;
}


/* local functions */
/******************************************************************************
 BAST_g3_P_QpskInitializeLoopParameters_isr()
******************************************************************************/
BERR_Code BAST_g3_P_QpskInitializeLoopParameters_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t carrier_acq1_bw, carrier_acq2_bw, carrier_acq3_bw, carrier_trk_bw, baud_acq_bw, baud_trk_bw;
   uint32_t Fb, P_hi, P_lo, Q_hi;
   uint8_t idx = 0;

   /* generated 10-18-2010 10:29:22 */
   /* BW scale = 2097152 (BW is 2 bytes), damp scale = 4 (damp is 1 byte) */
   static const uint16_t qpsk_bw[7][6] =
   {
      /* carrierAcq1, carrierAcq2, carrierAcq3, carrierTrk, baudAcq, baudTrk */
      {0x51EC, 0x051F, 0x03B0, 0x03B0, 0x49BA, 0x0EBF},  /* 0 <= Fb < 2000000 */
      {0x71EC, 0x0C3D, 0x0819, 0x0619, 0x99BA, 0x1A31},  /* 2000000 <= Fb < 4000000 */
      {0x71EC, 0x083D, 0x0219, 0x0119, 0x49BA, 0x0831},  /* 4000000 <= Fb < 12000000 */
      {0x71EC, 0x083D, 0x080C, 0x0619, 0x49BA, 0x0831},  /* 12000000 <= Fb < 17000000 */
      {0x71EC, 0x0C3D, 0x0C3D, 0x0C3D, 0x49BA, 0x0831},  /* 17000000 <= Fb < 23000000 */
      {0x71EC, 0x0C3D, 0x0C3D, 0x0C3D, 0x49BA, 0x0831},  /* 23000000 <= Fb < 27000000 */
      {0x71EC, 0x0C3D, 0x0C3D, 0x0C3D, 0x49BA, 0x0831}   /* 27000000 <= Fb < 45000000 */
   };

   static const uint8_t qpsk_damp[7][6] =
   {
      /* carrierAcq1, carrierAcq2, carrierAcq3, carrierTrk, baudAcq, baudTrk */
      {0x0C, 0x08, 0x18, 0x18, 0x0C, 0x0C},  /* 0 <= Fb < 2000000 */
      {0x0C, 0x08, 0x18, 0x18, 0x08, 0x08},  /* 2000000 <= Fb < 4000000 */
      {0x0C, 0x08, 0x18, 0x18, 0x04, 0x04},  /* 4000000 <= Fb < 12000000 */
      {0x0C, 0x08, 0x18, 0x18, 0x04, 0x04},  /* 12000000 <= Fb < 17000000 */
      {0x0C, 0x08, 0x08, 0x08, 0x0C, 0x0C},  /* 17000000 <= Fb < 23000000 */
      {0x0C, 0x08, 0x18, 0x18, 0x08, 0x04},  /* 23000000 <= Fb < 27000000 */
      {0x0C, 0x08, 0x18, 0x18, 0x04, 0x04}   /* 27000000 <= Fb < 45000000 */
   };

   /* determine symbol rate range */
   Fb = hChn->acqParams.symbolRate;
   if (Fb >= 27000000)
      idx = 6;
   else if (Fb >= 23000000)
      idx = 5;
   else if (Fb >= 17000000)
      idx = 4;
   else if (Fb >= 12000000)
      idx = 3;
   else if (Fb >= 4000000)
      idx = 2;
   else if (Fb >= 2000000)
      idx = 1;
   else
      idx = 0;

   /* initialize baud/carrier loop parameters based on symbol rate */
   carrier_acq1_bw = qpsk_bw[idx][0];
   carrier_acq2_bw = qpsk_bw[idx][1];
   carrier_acq3_bw = qpsk_bw[idx][2];
   carrier_trk_bw = qpsk_bw[idx][3];
   baud_acq_bw = qpsk_bw[idx][4];
   baud_trk_bw = qpsk_bw[idx][5];

   /* bw scaled 2^21 = 2097152 */
   BMTH_HILO_32TO64_Mul(carrier_acq1_bw, Fb, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 2097152, &Q_hi, &carrier_acq1_bw);
   BMTH_HILO_32TO64_Mul(carrier_acq2_bw, Fb, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 2097152, &Q_hi, &carrier_acq2_bw);
   BMTH_HILO_32TO64_Mul(carrier_acq3_bw, Fb, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 2097152, &Q_hi, &carrier_acq3_bw);
   BMTH_HILO_32TO64_Mul(carrier_trk_bw, Fb, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 2097152, &Q_hi, &carrier_trk_bw);
   BMTH_HILO_32TO64_Mul(baud_acq_bw, Fb, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 2097152, &Q_hi, &baud_acq_bw);
   BMTH_HILO_32TO64_Mul(baud_trk_bw, Fb, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 2097152, &Q_hi, &baud_trk_bw);

   hChn->carrierAcq1Bw = carrier_acq1_bw;
   hChn->carrierAcq2Bw = carrier_acq2_bw;
   hChn->carrierAcq3Bw = carrier_acq3_bw;
   hChn->carrierTrkBw = carrier_trk_bw;
   hChn->baudAcqBw = baud_acq_bw;
   hChn->baudTrkBw = baud_trk_bw;

   hChn->carrierAcq1Damp = qpsk_damp[idx][0];
   hChn->carrierAcq2Damp = qpsk_damp[idx][1];
   hChn->carrierAcq3Damp = qpsk_damp[idx][2];
   hChn->carrierTrkDamp = qpsk_damp[idx][3];
   hChn->baudAcqDamp = qpsk_damp[idx][4];
   hChn->baudTrkDamp = qpsk_damp[idx][5];

#if 0
   BDBG_MSG(("carrierAcq1Bw=%d, carrierAcq1Damp=%d", hChn->carrierAcq1Bw, hChn->carrierAcq1Damp));
   BDBG_MSG(("carrierAcq2Bw=%d, carrierAcq2Damp=%d", hChn->carrierAcq2Bw, hChn->carrierAcq2Damp));
   BDBG_MSG(("carrierAcq3Bw=%d, carrierAcq3Damp=%d", hChn->carrierAcq3Bw, hChn->carrierAcq3Damp));
   BDBG_MSG(("carrierTrkBw=%d, carrierTrkDamp=%d", hChn->carrierTrkBw, hChn->carrierTrkDamp));
   BDBG_MSG(("baudAcqBw=%d, baudAcqDamp=%d", hChn->baudAcqBw, hChn->baudAcqDamp));
   BDBG_MSG(("baudTrkBw=%d, baudTrkDamp=%d", hChn->baudTrkBw, hChn->baudTrkDamp));
#endif
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_QpskSetMode_isr()
******************************************************************************/
BERR_Code BAST_g3_P_QpskSetMode_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t vtctl, fmod, val;
   BERR_Code retCode;

   static const uint32_t script_qpsk_dcii[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_VIT_V76, 0x0B000A00),   /* 7/8 and 5/6 */
      BAST_SCRIPT_WRITE(BCHP_SDS_VIT_V54, 0x08000700),   /* 4/5 and 3/4 */
      BAST_SCRIPT_WRITE(BCHP_SDS_VIT_V32, 0x04000500),   /* 2/3 and 3/5 */
      BAST_SCRIPT_WRITE(BCHP_SDS_VIT_V10, 0x04500400),   /* 1/2 and 5/11 */
      BAST_SCRIPT_WRITE(BCHP_SDS_VIT_VINT, 0x25800000),  /* integration period */
      BAST_SCRIPT_WRITE(BCHP_SDS_FEC_FRS, 0x0a0BFFFE),
      BAST_SCRIPT_WRITE(BCHP_SDS_FEC_FSYN, 0x02000805),
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_qpsk_dvb_dtv[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_VIT_V76, 0x00E600E6),
      BAST_SCRIPT_WRITE(BCHP_SDS_VIT_V54, 0x09000880),
      BAST_SCRIPT_WRITE(BCHP_SDS_VIT_V32, 0x08660780),
      BAST_SCRIPT_WRITE(BCHP_SDS_VIT_V10, 0x06000480),
      BAST_SCRIPT_WRITE(BCHP_SDS_VIT_VINT, 0x27E70000),
      BAST_SCRIPT_WRITE(BCHP_SDS_FEC_FRS, 0x020B2523),
      BAST_SCRIPT_WRITE(BCHP_SDS_FEC_FSYN, 0x02000805),
      BAST_SCRIPT_EXIT
   };

   static const uint8_t vtctl_init[0x13] =
   {
      0xF0, /* BAST_Mode_eDvb_scan */
      0xE0, /* BAST_Mode_eDvb_1_2 */
      0xE1, /* BAST_Mode_eDvb_2_3 */
      0xE2, /* BAST_Mode_eDvb_3_4 */
      0xE3, /* BAST_Mode_eDvb_5_6 */
      0xE5, /* BAST_Mode_eDvb_7_8 */
      0xF1, /* BAST_Mode_eDss_scan */
      0xE0, /* BAST_Mode_eDss_1_2 */
      0xE1, /* BAST_Mode_eDss_2_3 */
      0xE4, /* BAST_Mode_eDss_6_7 */
      0x12, /* BAST_Mode_eDcii_scan */
      0x09, /* BAST_Mode_eDcii_1_2 */
      0x0B, /* BAST_Mode_eDcii_2_3 */
      0x0C, /* BAST_Mode_eDcii_3_4 */
      0x0E, /* BAST_Mode_eDcii_5_6 */
      0x0F, /* BAST_Mode_eDcii_7_8 */
      0x08, /* BAST_Mode_eDcii_5_11 */
      0x0A, /* BAST_Mode_eDcii_3_5 */
      0x0D, /* BAST_Mode_eDcii_4_5 */
   };

   if (BAST_MODE_IS_DCII(hChn->acqParams.mode))
   {
      /* DCII */
      BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_qpsk_dcii));
   }
   else if (BAST_MODE_IS_LEGACY_QPSK(hChn->acqParams.mode))
   {
      /* DVB or DTV */
      BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_qpsk_dvb_dtv));
   }
   else
   {
      /* non-legacy-qpsk mode */
      return BERR_INVALID_PARAMETER;
   }

   vtctl = (uint32_t)vtctl_init[hChn->acqParams.mode];
   if (BAST_MODE_IS_DCII(hChn->acqParams.mode))
   {
      /* DCII */
      if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_DCII_SPLIT)
      {
         vtctl |= 0x0400;
         if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_DCII_SPLIT_Q)
         {
            vtctl |= 0xA0; /* split Q */
            vtctl &= ~0x40;
         }
         else
         {
            vtctl |= 0x80; /* split I */
            vtctl &= ~0x60;
         }
      }
      else
         vtctl |= 0xE0; /*combine */
   }

   if ((hChn->acqParams.mode < BAST_Mode_eDcii_scan) ||
       ((hChn->acqParams.mode >= BAST_Mode_eDcii_scan) && ((hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_DCII_SPLIT) == 0)))
   {
      vtctl &= ~0x60;
      vtctl |= ((hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_SPINV_IQ_SCAN) >> 8);
   }

   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_OQPSK)
   {
      vtctl |= 0x0200;
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, 0x00020000);
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, 0x00000010);
   }

   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, &vtctl);

   /* reset viterbi decoder */
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, 0x00004000);

   val = 0x19;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_CLEAR, &val);

   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, ~0x00004000);

   /* set FMOD */
   fmod = hChn->stuffBytes & 0x1F;
   if (BAST_MODE_IS_DVB(hChn->acqParams.mode))
   {
      /* DVB */
      fmod |= 0x5200;
   }
   else if (BAST_MODE_IS_DTV(hChn->acqParams.mode))
   {
      /* DTV */
      fmod |= 0x5600;
   }
   else
   {
      /* DCII */
      if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_DCII_SPLIT)
      {
         if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_DCII_SPLIT_Q)
            fmod |= 0x0A00;   /* split Q */
         else
            fmod |= 0x4A00;   /* split I */
      }
      else
         fmod |= 0x5A00; /* combine */
   }
   fmod |= ((hChn->xportSettings.bTei) ? 0x0200 : 0x0000);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FMOD, &fmod);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_QpskLockViterbi_isr() - this routine is formerly verify_lock
******************************************************************************/
BERR_Code BAST_g3_P_QpskLockViterbi_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, bw, irq, raw;
   BERR_Code retCode;

   while (1)
   {
      switch (hChn->funct_state)
      {
         case 0:
            /* reset the equalizer */
            BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 1);

            hChn->carrierDelay = (hChn->acqParams.mode == BAST_Mode_eDcii_scan) ? 900000 : 700000;
            hChn->checkLockInterval = 20000;

            /* extend delay for small symbol rates, shorten delay for higher symbol rates */
            if (hChn->acqParams.symbolRate < 5000000)
            {
               hChn->carrierDelay = (hChn->carrierDelay * 3) >> 1;
            }

            /* set baud loop bw */
#if 0       /* this bw is way too high, --> do NOT turn on */
            BAST_g3_P_SetBaudBw_isr(h, hChn->baudAcqBw, hChn->baudAcqDamp);
#endif

            hChn->funct_state = 1;
            break;

         case 1:
            /* set carrier loop bw */
            BAST_g3_P_SetCarrierBw_isr(h, hChn->carrierAcq1Bw, hChn->carrierAcq1Damp);

            /* initialize counters */
            hChn->count1 = 0;
            hChn->maxCount1 = hChn->carrierDelay / hChn->checkLockInterval;
            if (hChn->maxCount1 == 0)
               hChn->maxCount1 = 1;

            hChn->funct_state = 2;
            break;

         case 2:
            /* gradually narrow the carrier bw from carrierAcq1Bw to carrierAcq2Bw */
            bw = hChn->carrierAcq1Bw - ((hChn->count1 * (hChn->carrierAcq1Bw - hChn->carrierAcq2Bw)) / hChn->maxCount1);
            BAST_g3_P_SetCarrierBw_isr(h, bw, hChn->carrierAcq1Damp);
            hChn->count1++;

            if (hChn->count1 < hChn->maxCount1)
               return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, hChn->checkLockInterval, BAST_g3_P_QpskLockViterbi_isr);

            /* narrow to tracking bw */
            BAST_g3_P_SetCarrierBw_isr(h, hChn->carrierAcq2Bw, hChn->carrierAcq2Damp);

            hChn->funct_state = 3;
            break;

         case 3:
            /* calculate time to wait for viterbi */
            hChn->count1 = hChn->carrierDelay / hChn->checkLockInterval;
            if (hChn->count1 < 1)
               hChn->count1 = 1;

            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_INTR_RAW_STS0, &val);
            hChn->prev_state = (val & BCHP_SDS_INTR2_0_CPU_STATUS_vit_in_sync_MASK);

            /* clear sticky vit lock status bit */
            val = 0x18;
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_CLEAR, &val);

            hChn->funct_state = 4;
            break;

         case 4:
            /* wait for viterbi lock */
            if ((hChn->count1 > 0) || (hChn->prev_state == BCHP_SDS_INTR2_0_CPU_STATUS_vit_in_sync_MASK))
            {
               hChn->funct_state = 5; /* vit NOT locked */
               return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, hChn->checkLockInterval, BAST_g3_P_QpskLockViterbi_isr);
            }
            hChn->funct_state = 6; /* vit NOT locked after carrierDelay */
            break;

         case 5:
            /* make sure real time status is locked before checking sticky bits */
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_INTR_RAW_STS0, &raw);
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_STATUS, &irq);
            /* BDBG_MSG(("raw=0x%X, irq=0x%X", raw, irq)); */
            raw &= 0x18;
            irq &= 0x18;
            if (raw == BCHP_SDS_INTR2_0_CPU_STATUS_vit_in_sync_MASK)
            {
               BAST_DEBUG_QPSK(BDBG_MSG(("vit raw sts is locked")));

               /* check sticky bit status for lock stability */
               if ((irq == 0) && (hChn->prev_state == BCHP_SDS_INTR2_0_CPU_STATUS_vit_in_sync_MASK))
               {
                  hChn->bVitLocked = true;
                  hChn->funct_state = 7;  /* vit locked */
                  break;
               }
               hChn->prev_state = BCHP_SDS_INTR2_0_CPU_STATUS_vit_in_sync_MASK;
            }
            else
               hChn->prev_state = 0;
            if (irq != 0)
               BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_CLEAR, &irq);

            if (hChn->count1 > 0)
               hChn->count1--;
            hChn->funct_state = 4;  /* wait again */
            break;

         case 6:
            /* viterbi NOT locked */
            hChn->bVitLocked = false;

            /* manually handle dtv scan */
            if (hChn->dtvScanState & BAST_G3_DTV_SCAN_ON)
            {
               hChn->dtvScanState ^= BAST_G3_DTV_SCAN_1_2;

               if (hChn->dtvScanState & BAST_G3_DTV_SCAN_1_2)
               {
                  /* trying dtv 1/2 */
                  BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, ~0x0000001F, 0x00004000);
                  BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, ~0x00004000);  /* deassert vit soft rst */

                  hChn->funct_state = 1;
                  break;
               }
            }
            hChn->funct_state = 7;
            break;

         case 7:
            /* reacquire if unable to lock viterbi */
            if (hChn->bVitLocked == false)
            {
               BDBG_MSG(("Vit could not lock"));
               hChn->reacqCause = BAST_ReacqCause_eVitNotLock;
               return BAST_g3_P_Reacquire_isr(h);
            }

            BDBG_MSG(("vit locked"));
            BAST_CHK_RETCODE(BAST_g3_P_LogTraceBuffer_isr(h, BAST_TraceEvent_eViterbiLocked));

            /* determine stabilized code rate */
            if (BAST_g3_P_QpskSetActualMode_isr(h) != BERR_SUCCESS)
            {
               BDBG_MSG(("unable to set actual mode"));
               hChn->reacqCause = BAST_ReacqCause_eUnableToDetermineActualMode;
               return BAST_g3_P_Reacquire_isr(h);
            }
            BAST_CHK_RETCODE(BAST_g3_P_QpskSetVcos_isr(h));

            return BAST_g3_P_QpskSpInvScan_isr(h);

         default:
            BDBG_ERR(("invalid state"));
            BERR_TRACE(retCode = BAST_ERR_AP_IRQ);
            break;
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_QpskSetActualMode_isr()
******************************************************************************/
BERR_Code BAST_g3_P_QpskSetActualMode_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t i, j, val, vst_save;
   BERR_Code retCode;

   /* check initial code rate */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VST, &val);
   vst_save = (val >> 24) & 0x0F;

   if ((hChn->acqParams.mode == BAST_Mode_eDss_scan) ||
       (hChn->acqParams.mode == BAST_Mode_eDvb_scan) ||
       (hChn->acqParams.mode == BAST_Mode_eDcii_scan))
   {
      /* check for stable code rate in scan mode */
      for (i = 10; i > 0; i--)
      {
         for (j = 50; j > 0; j--)
         {
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VST, &val);
            val = (val >> 24) & 0x0F;
            if (val != vst_save)
               break;
         }
         vst_save = val;
         if (j == 0)
         {
            /* VST is stable */
            break;
         }
      }
   }

   BAST_CHK_RETCODE(BAST_g3_P_GetActualMode_isr(h, &(hChn->actualMode)));
   if (hChn->actualMode == BAST_Mode_eUnknown)
      BERR_TRACE(retCode = BERR_UNKNOWN);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_QpskSetVcos_isr()
******************************************************************************/
BERR_Code BAST_g3_P_QpskSetVcos_isr(BAST_ChannelHandle h)
{
   uint32_t vst, val = 0;

   static const uint8_t vcos_byte1[16] =
   {
      0x40, 0x48, 0x50, 0x60,
      0x68, 0x78, 0x40, 0x40,
      0x40, 0x40, 0x48, 0x48,
      0x50, 0x50, 0x60, 0x78
   };

   /* read code rate */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VST, &vst);
   vst = (vst >> 24) & 0x0F;

   /* look up qpsk hard decision level for VLC slicer */
   val = (vcos_byte1[vst] << 8);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VCOS, &val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_QpskSpInvScan_isr()
******************************************************************************/
BERR_Code BAST_g3_P_QpskSpInvScan_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   if ((hChn->actualMode != BAST_Mode_eDvb_5_6) && (hChn->actualMode != BAST_Mode_eDss_6_7))
      return BAST_g3_P_QpskNarrowBw_isr(h);

   /* do spinv scan */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_FEC_FSYN, ~0xFFFF, 0x0802);

   hChn->funct_state = 0;
   hChn->count1 = 0;
   return BAST_g3_P_QpskSpinvScan1_isr(h);
}


/******************************************************************************
 BAST_g3_P_QpskSpinvScan1_isr()
******************************************************************************/
BERR_Code BAST_g3_P_QpskSpinvScan1_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, vtctl, sts;
   BERR_Code retCode;

   while (1)
   {
      switch (hChn->funct_state)
      {
         case 0:
            BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, ~0x60, (hChn->count1 % 3) << 5);
            hChn->funct_state = 1;
            return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, 30000, BAST_g3_P_QpskSpinvScan1_isr);

         case 1:
            sts = 0x7F;
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_CLEAR, &sts);
            hChn->funct_state = 2;
            return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, 30000, BAST_g3_P_QpskSpinvScan1_isr);

         case 2:
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_INTR_RAW_STS0, &val);
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, &vtctl);
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_STATUS, &sts);
            /* BDBG_ERR(("spinv scan: vtctl=0x%X, raw=0x%X, sticky=0x%X", vtctl, val, sts)); */
            sts &= 0x1B;
            val &= 0x1B;
            if (val == 0x09)
            {
               if ((sts == 0) || (sts == 0x09))
               {
                  /* BDBG_ERR(("spinv scan success")); */
                  goto done;  /* IQ scan successful */
               }
               else
                  goto not_locked;
            }
            else
            {
               not_locked:
               /* BDBG_ERR(("spinv scan: RS/Vit not locked")); */
               hChn->count1++;
               if (hChn->count1 < 9)
                  hChn->funct_state = 0;
               else
               {
                  BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, ~0x60);
                  goto done;
               }
            }
            break;

         default:
            BDBG_ERR(("invalid state"));
            BERR_TRACE(retCode = BAST_ERR_AP_IRQ);
            break;
      }
   }

   done:
   return BAST_g3_P_QpskNarrowBw_isr(h);
}


/******************************************************************************
 BAST_g3_P_QpskNarrowBw_isr()
******************************************************************************/
BERR_Code BAST_g3_P_QpskNarrowBw_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   /* set thresholds for bad headers */
   val = 0x04003020;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FSYN, &val);

   hChn->count1 = 1;
   return BAST_g3_P_QpskNarrowBw1_isr(h);
}


/******************************************************************************
 BAST_g3_P_QpskNarrowBw1_isr()
******************************************************************************/
BERR_Code BAST_g3_P_QpskNarrowBw1_isr(BAST_ChannelHandle h)
{
   BERR_Code retCode;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t cdelta, bw, damp;
#if 0
   uint32_t bdelta;
#endif

   if (hChn->count1 <= 4)
   {
      cdelta = (hChn->carrierAcq2Bw - hChn->carrierAcq3Bw) >> 2;
      bw = hChn->carrierAcq2Bw - hChn->count1 * cdelta;
      cdelta = (hChn->carrierAcq3Damp - hChn->carrierAcq2Damp) >> 2;
      damp = hChn->carrierAcq2Damp + hChn->count1 * cdelta;
      BAST_g3_P_SetCarrierBw_isr(h, bw, damp);

#if 0
      bdelta = (hChn->baudAcqBw - hChn->baudTrkBw) >> 2;
      bw = hChn->baudAcqBw - hChn->count1 * bdelta;
      bdelta = (hChn->baudTrkDamp - hChn->baudAcqDamp) >> 2;
      damp = hChn->baudAcqDamp + hChn->count1 * bdelta;
      BAST_g3_P_SetBaudBw_isr(h, bw, damp);
#endif

      hChn->count1++;
      return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, 1000, BAST_g3_P_QpskNarrowBw1_isr);
   }

   BAST_g3_P_SetCarrierBw_isr(h, hChn->carrierAcq3Bw, hChn->carrierAcq3Damp);
   retCode = BAST_g3_P_SetBaudBw_isr(h, hChn->baudTrkBw, hChn->baudTrkDamp);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_WRN(("BAST_g3_P_SetBaudBw_isr() error 0x%X", retCode));
      return retCode;
   }

   return BAST_g3_P_QpskAcquire1_isr(h);
}


/******************************************************************************
 BAST_g3_P_QpskAcquire1_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g3_P_QpskAcquire1_isr(BAST_ChannelHandle h)
{
   BERR_Code retCode;
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;

   static const uint32_t script_qpsk_1[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_FEC_FSYN, 0x04003020),
#ifndef BAST_HAS_WFE
      BAST_SCRIPT_WRITE(BCHP_SDS_AGC_ABW, 0x03030000),      /* TBD set AGC and DCO tracking bandwidths per Steve */
#endif
      BAST_SCRIPT_AND(BCHP_SDS_EQ_EQMISCCTL, ~0x0000FF00),  /* EQBLND=0 */
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLTD, 0x16000000),
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_qpsk_2[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_VLCTL, 0x00040704),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_PLTD, 0x28000000),
      BAST_SCRIPT_AND_OR(BCHP_SDS_CL_CLCTL2, ~0x000000EF, 0x000000A1),  /* CLMISC=0xA1 but retain bit 4 */
#if 0
      BAST_SCRIPT_AND_OR(BCHP_SDS_CL_CLCTL1, ~0x00FFFF00, 0x004A1000),  /* CLPDCTL=0x10, CLQCFD=0x4A */
#else
      BAST_SCRIPT_AND_OR(BCHP_SDS_CL_CLCTL1, ~0x0CFFFF00, 0x0C481000),  /* CLPDCTL=0x10, CLQCFD=0x40 */
#endif
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_qpsk_3[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_PLI, 0x00000000),
      BAST_SCRIPT_AND_OR(BCHP_SDS_CL_CLCTL2, ~0x0000FF00, 0x00007100),  /*CLMISC2=0x71 */
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_qpsk_4[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_EQFFECTL, 0x22060271),
      BAST_SCRIPT_AND(BCHP_SDS_EQ_EQFFECTL, ~0x00000001),   /* clear eq reset */
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_qpsk_5_new[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_SNR_SNRCTL, 0x03),
      BAST_SCRIPT_WRITE(BCHP_SDS_SNR_SNORECTL, 0x8A),
      BAST_SCRIPT_WRITE(BCHP_SDS_SNR_SNORECTL, 0x0A),
      BAST_SCRIPT_WRITE(BCHP_SDS_SNR_SNORECTL, 0x8A),
      BAST_SCRIPT_WRITE(BCHP_SDS_BERT_BEIT, 0x00FF005F),
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_qpsk_5_old[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_SNR_SNRCTL, 0x03),
      BAST_SCRIPT_WRITE(BCHP_SDS_SNR_SNORECTL, 0x80),
      BAST_SCRIPT_WRITE(BCHP_SDS_SNR_SNORECTL, 0x00),
      BAST_SCRIPT_WRITE(BCHP_SDS_SNR_SNORECTL, 0x80),
      BAST_SCRIPT_WRITE(BCHP_SDS_BERT_BEIT, 0x00FF005F),
      BAST_SCRIPT_EXIT
   };

   /* set agc tracking bw */
   BAST_g3_P_SetAgcTrackingBw_isr(h);
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_qpsk_1));
   BAST_g3_P_QpskSetFinalBlBw_isr(h);
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_qpsk_2));
   BAST_g3_P_QpskSetOqpsk_isr(h);
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_qpsk_3));
   BAST_g3_P_QpskSetFinalFlBw_isr(h);
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_qpsk_4));

   /* not needed because main tap has already been set in script_qpsk_4: BAST_g3_P_SetFfeMainTap_isr(h); */

   if (hDev->sdsRevId >= 0x63)
   {
      BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_qpsk_5_new));
   }
   else
   {
      BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_qpsk_5_old));
   }

   /* configure output pll */
   return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, 1000, BAST_g3_P_QpskSetOpll_isr);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_QpskSetFinalFlBw_isr()
******************************************************************************/
BERR_Code BAST_g3_P_QpskSetFinalFlBw_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t vst, fllc, flic;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VST, &vst);
   vst = (vst >> 24) & 0x0F;

   if (BAST_MODE_IS_DVB(hChn->acqParams.mode))
   {
      /* DVB */
      if (vst <= 1)
      {
         /* code rate 1/2 or 2/3 */
         fllc = 0x01000100;
         flic = 0x01000000;
      }
      else
      {
         fllc = 0x01000100;
         flic = 0x01000000;
      }
   }
   else if (BAST_MODE_IS_DTV(hChn->acqParams.mode))
   {
      /* DTV */
      if (vst <= 1)
      {
         /* code rate 1/2 or 2/3 */
         fllc = 0x02880300;
         flic = 0x029B0200;
      }
      else
      {
         fllc = 0x04F20200;
         flic = 0x01B20200;
      }
   }
   else
   {
      /* DCII */
      fllc = 0x02000200;
      flic = 0x01000300;
   }
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLLC, &fllc);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLIC, &flic);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_QpskSetFinalBlBw_isr()
******************************************************************************/
BERR_Code BAST_g3_P_QpskSetFinalBlBw_isr(BAST_ChannelHandle h)
{
   /* sets tracking PLC */
   return BAST_g3_P_ConfigPlc_isr(h, false);
}


/******************************************************************************
 BAST_g3_P_QpskSetOqpsk_isr() - from qpsk_set_clctl_3() in 4506
******************************************************************************/
BERR_Code BAST_g3_P_QpskSetOqpsk_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, &val);
   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_OQPSK)
      val = (val & 0x70) | 0x0C;
   else
      val = (val & 0x30) | 0x0C;
   val |= 0x0C080000;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, &val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_QpskSetOpll_isr()
******************************************************************************/
BERR_Code BAST_g3_P_QpskSetOpll_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
   uint32_t val, fmod, stuff, rs_mode, vst, data0, data2, data3, data6, data7, data4;
   uint32_t lval1, lval2, gcf, P_hi, P_lo, Q_hi, Q_lo;
   BAST_Mode mode;

   static const uint8_t qpsk_opll_data6[16] =
   {
      0x01, 0x02, 0x03, 0x05,
      0x06, 0x07, 0x01, 0x01,
      0x05, 0x01, 0x03, 0x02,
      0x03, 0x04, 0x05, 0x07
   };


   static const uint8_t qpsk_opll_data7[16] =
   {
      0x02, 0x03, 0x04, 0x06,
      0x07, 0x08, 0x01, 0x01,
      0x0B, 0x02, 0x05, 0x03,
      0x04, 0x05, 0x06, 0x08
   };

   BAST_g3_P_SdsPowerUpOpll_isr(h);

   /* reset output interface */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_OI_OIFCTL00, 0x00000001);

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_FEC_FMOD, &fmod);
   rs_mode = (fmod & 0x0C00);
   stuff = fmod & 0x1F;

   if ((rs_mode == 0x0800) || (rs_mode == 0x0000))
   {
      /* DVB and DCII */
      data2 = 188;
      data3 = 204;
   }
   else
   {
      /* DTV */
      data2 = 130;
      data3 = 147;
   }

   if (fmod & 0x00008000)
      data4 = 16; /* RS decoder disabled */
   else
      data4 = 0;

   lval1 = data2 * (data2 + data4 + stuff);
   lval2 = data3 * data2;

   if (data3 != 147)
      data3 = 188;

   /* verify puncture mode */
   BAST_g3_P_GetActualMode_isr(h, &mode);
   if (mode != hChn->actualMode)
   {
      BDBG_WRN(("code rate changed - reacquiring"));
      hChn->reacqCause = BAST_ReacqCause_eUnableToDetermineActualMode;
      return BAST_g3_P_Reacquire_isr(h);
   }

   /* read code rate */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VST, &vst);
   vst = (vst >> 24) & 0x0F;
   data6 = qpsk_opll_data6[vst];
   data7 = qpsk_opll_data7[vst];

   lval1 *= (data6 << 1);  /* lval1 = m */

   if (hChn->bUndersample)
      data0 = 2;
   else
      data0 = 4;
   lval2 *= (data7 * data0);   /* lval2 = n */

   /* are we in split mode? */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, &val);
   if (val & 0x0400)
      lval2 = lval2 << 1;

   BMTH_HILO_32TO64_Mul(hChn->acqParams.symbolRate, data0 * lval1, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, lval2, &Q_hi, &Q_lo);

   if (hChn->xportSettings.bDelHeader)
   {
      if (hChn->xportSettings.bHead4)
         data0 = 4;
      else
         data0 = 1;
      BMTH_HILO_32TO64_Mul(Q_lo, data3 - data0, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, data3, &Q_hi, &Q_lo);
   }
   hChn->outputBitRate = Q_lo;

   gcf = BAST_g3_P_GCF_isr(lval1, lval2);
   lval1 /= gcf;  /* opll_N */
   lval2 /= gcf;  /* opll_D */

   /* config output interface */
   hChn->opll_N = lval1;
   hChn->opll_D = lval2;
   BAST_g3_P_ConfigOif_isr(h);

   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_RS_DISABLE)
   {
      if (hDev->sdsRevId < 0x69)
         BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_FEC_FECTL, 0x00000008);
      else
         BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_FEC_FMOD, 0x00000100);
   }
   else
   {
      if (hDev->sdsRevId < 0x69)
         BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_FEC_FECTL, ~0x00000008);
      else
         BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_FEC_FMOD, ~0x00000100);
   }

   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_FEC_FECTL, 0x00000001);
   val = 0x7F;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_CLEAR, &val);
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_FEC_FECTL, ~0x00000001);

   /* start tracking */
   return BAST_g3_P_StartTracking_isr(h);
}
