/***************************************************************************
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
 *
 * [File Description:]
 *
 ***************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g3_priv.h"
#if BCHP_CHIP==4517
#include "bchp_tm.h"
#endif

BDBG_MODULE(bast_g3_priv_acq);


#define BAST_DEBUG_ACQ(x) /* x */
#define BAST_DEBUG_VCO_AVOIDANCE(x) /* x */


/******************************************************************************
 BAST_g3_P_InitHandleDefault()
******************************************************************************/
BERR_Code BAST_g3_P_InitHandleDefault(BAST_Handle h)
{
   BAST_g3_P_Handle *hDev = (BAST_g3_P_Handle *)(h->pImpl);
   uint8_t i;

#ifndef BAST_EXCLUDE_FTM
   hDev->bFtmLocalReset = false; /* must initialize this flag independent of ftm init */
   hDev->bFtmPoweredDown = false;
   hDev->hFtmDev.txChannel = BAST_FskChannel_e0;
   hDev->hFtmDev.rxChannel = BAST_FskChannel_e0;
   hDev->hFtmDev.txPower = 0xF;
   #ifdef BAST_ENABLE_GENERIC_FSK
   hDev->hFtmDev.txFreqHz = 2300000;
   hDev->hFtmDev.rxFreqHz = 2300000;
   hDev->hFtmDev.txDevHz = 40000;
   #endif
#endif
   hDev->searchRange = 5000000;
#ifndef BAST_EXCLUDE_BCM3445
   hDev->bBcm3445 = false;
   hDev->bcm3445Address = BAST_BCM3445_DEFAULT_ADDRESS;
#endif
   hDev->dftMinN = 4;
   hDev->bInit = false;

   for (i = 0; i < BAST_TUNER_KVCO_CAL_TABLE_SIZE; i++)
   {
      hDev->tuner_kvco_cal_capcntl_table[i] = 0;
      hDev->tuner_kvco_cal_kvcocntl_table[i] = 0;
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_InitChannelHandle()
******************************************************************************/
BERR_Code BAST_g3_P_InitChannelHandle(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t i;

   hChn->baudTimerIsr = NULL;
   hChn->berTimerIsr = NULL;
   hChn->gen1TimerIsr = NULL;
   hChn->gen2TimerIsr = NULL;
   hChn->gen3TimerIsr = NULL;
   hChn->acqState = BAST_AcqState_eIdle;
#ifndef BAST_EXCLUDE_MI2C
   hChn->bMi2cInProgress = false;
#endif
   hChn->berErrors = 0;
   hChn->mpegErrorCount = 0;
   hChn->mpegFrameCount = 0;
#ifndef BAST_EXCLUDE_LDPC
   hChn->ldpcBadBlocks = 0;
   hChn->ldpcCorrBlocks = 0;
   hChn->ldpcTotalBlocks = 0;
   hChn->ldpcCtl = BAST_G3_CONFIG_LDPC_CTL_DISABLE_PSL;
   hChn->ldpcScanModes = BAST_G3_CONFIG_LDPC_SCAN_MODES_ALL;
   for (i = 0; i < 16; i++)
      hChn->ldpcScramblingSeq[i] = 0x00;
   hChn->ldpcScramblingSeq[2] = 0x01;
   hChn->acmMaxMode = BAST_Mode_eLdpc_8psk_9_10;
#endif
   hChn->rsCorr = 0;
   hChn->rsUncorr = 0;
   hChn->preVitErrors = 0;
#ifndef BAST_EXCLUDE_TURBO
   hChn->turboBadBlocks = 0;
   hChn->turboCorrBlocks = 0;
   hChn->turboTotalBlocks = 0;
#endif
   hChn->reacqCtl = BAST_G3_CONFIG_REACQ_CTL_RETUNE | BAST_G3_CONFIG_REACQ_CTL_FREQ_DRIFT;
   hChn->bLastLocked = false;
   hChn->bUndersample = false;
   hChn->timeSinceStableLock = 0;
#ifndef BAST_EXCLUDE_SPUR_CANCELLER
   BAST_g3_P_ClearSpurCancellerConfig(h);
#endif
   if (hChn->bHasDiseqc)
   {
      hChn->diseqc->diseqc1TimerIsr = NULL;
      hChn->diseqc->diseqc2TimerIsr = NULL;
      hChn->diseqc->dsecSettings.bEnvelope = true;
      hChn->diseqc->dsecSettings.bToneAlign = false;
      hChn->diseqc->dsecSettings.bDisableRRTO = false;
      hChn->diseqc->dsecSettings.bEnableToneburst = false;
      hChn->diseqc->dsecSettings.bToneburstB = false;
      hChn->diseqc->dsecSettings.bOverrideFraming = false;
      hChn->diseqc->dsecSettings.bExpectReply = false;
      hChn->diseqc->dsecSettings.bEnableLNBPU = false;
      hChn->diseqc->dsecSettings.bDisableRxOnly = false;
      hChn->diseqc->dsecSettings.rrtoUsec = 210000;        /* 210000us */
      hChn->diseqc->dsecSettings.bitThreshold = 0x2D3;     /* 723us */
      hChn->diseqc->dsecSettings.toneThreshold = 0x6E;     /* 687.5mV * 0.16 = 110 */
      hChn->diseqc->dsecSettings.preTxDelay = 15;          /* default 15 ms delay */
      hChn->diseqc->dsecSettings.vsenseThresholdHi = 0x30; /* ~22 V */
      hChn->diseqc->dsecSettings.vsenseThresholdLo = 0xD1; /* ~10 V */
      hChn->diseqc->diseqcStatus.status = BAST_DiseqcSendStatus_eSuccess;
   }

   hChn->blindScanModes = BAST_G3_CONFIG_BLIND_SCAN_MODES_DVB;
#ifndef BAST_EXCLUDE_LDPC
   hChn->blindScanModes |= BAST_G3_CONFIG_BLIND_SCAN_MODES_LDPC;
#endif
#ifndef BAST_EXCLUDE_TURBO
   hChn->blindScanModes |= BAST_G3_CONFIG_BLIND_SCAN_MODES_TURBO;
#endif

#ifndef BAST_EXCLUDE_POWERDOWN
   hChn->coresPoweredDown = BAST_CORE_DISEQC;
#else
   hChn->coresPoweredDown = 0;
#endif
   hChn->dtvScanCodeRates = BAST_G3_CONFIG_DTV_SCAN_CODE_RATES_ALL;
   hChn->dvbScanCodeRates = BAST_G3_CONFIG_DVB_SCAN_CODE_RATES_ALL;
   hChn->dciiScanCodeRates = BAST_G3_CONFIG_DCII_SCAN_CODE_RATES_ALL;
#ifndef BAST_EXCLUDE_TURBO
   hChn->turboScanCurrMode = 0;
   hChn->turboScanModes = BAST_G3_CONFIG_TURBO_SCAN_MODES_ALL;
   hChn->turboCtl = 0;
#endif
   hChn->peakScanSymRateMin = 0;
   hChn->peakScanSymRateMax = 0;
   hChn->acqTime = 0;
#ifndef BAST_EXCLUDE_STATUS_EVENTS
   hChn->statusEventIndicators = 0;
   hChn->rainFadeThreshold = 3<<3; /* 3dB*2^3 */
   hChn->freqDriftThreshold = 1500000;
   hChn->bStatusInterrupts = false;
   hChn->rainFadeWindow = 3000; /* 300 secs */
#endif
   hChn->stuffBytes = 0;
   hChn->plcCtl = 0;
   hChn->bFsNotDefault = true;
   hChn->dftFreqEstimateStatus = 0;
   hChn->tunerCtl = 0;
   hChn->bReacqTimerExpired = false;
   hChn->dftRangeStart = 0x0000;
   hChn->dftRangeEnd = 0x00FF;
   hChn->signalDetectStatus = 0;
#ifndef BAST_EXCLUDE_BCM3445
   hChn->bcm3445Status = 0;
   hChn->bcm3445Mi2cChannel = BAST_Mi2cChannel_e0;
   hChn->bcm3445TunerInput = BAST_Bcm3445OutputChannel_eNone;
   hChn->bcm3445Ctl = BAST_G3_BCM3445_CTL_AGC_TOP_MID;
#endif
#ifndef BAST_HAS_WFE
   hChn->tunerFilCalUpperThresh = 65; /* upper threshold is 65% of half_tone_avg */
   hChn->tunerFilCalLowerThresh = 35; /* lower threshold is 35% of half_tone_avg */
   hChn->tunerLsRange = 2;
   hChn->bOverrideKvco = false;
   hChn->bCalibrateKvco = false;
   hChn->tunerVcoFreqMhz = 0;
   hChn->tunerKvcoCntl = 0;
   hChn->tunerVcRefLow = 19;
   hChn->tunerVcRefHigh = 25;
   hChn->tunerAgcThreshold = 0x00400040;  /* [31:16] for BB, [15:0] for LNA */
   hChn->tunerAgcWinLength = 0x13231323;  /* [31:16] for BB, [15:0] for LNA */
   hChn->tunerAgcAmpThresh = 0x0909;      /* [12:8] for BB, [4:0] for LNA */
   hChn->tunerAgcLoopCoeff = 0x1414;      /* [12:8] for BB, [4:0] for LNA */
   hChn->tunerDaisyGain = 0x2;   /* default daisy output gain */
   hChn->tempAmbient = 0;
#endif
   hChn->bMonitorLock = true;
   hChn->debug1 = 0;
   hChn->debug2 = 0;
   hChn->debug3 = 0;
   hChn->acqParams.symbolRate = 20000000;
   hChn->acqParams.carrierFreqOffset = 0;
   hChn->acqParams.mode = BAST_Mode_eDss_scan;
   hChn->acqParams.acq_ctl = BAST_ACQSETTINGS_DEFAULT;
   hChn->bEnableFineFreq = false;

   BAST_g3_P_InitConfig(h); /* chip-specific configuration */

   BAST_g3_ResetInterruptCounters(h);
#ifndef BAST_EXCLUDE_STATUS_EVENTS
   retCode = BAST_g3_P_ResetStatusIndicators(h);
#endif

   return retCode;
}


#ifndef BAST_HAS_WFE
/******************************************************************************
 BAST_g3_P_InitNextChannel_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_InitNextChannel_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_Handle *pDev = (BAST_g3_P_Handle*)(h->pDevice->pImpl);

   next:
   pDev->counter++;

   if (pDev->counter >= h->pDevice->totalChannels)
   {
      BKNI_SetEvent(pDev->hInitDoneEvent);
      return BERR_SUCCESS;
   }

   if (!(h->pDevice->pChannels[pDev->counter]))
      goto next;

   return BAST_g3_P_InitAllChannels_isr(h->pDevice->pChannels[pDev->counter]);
}
#endif


/******************************************************************************
 BAST_g3_P_InitAllChannels() - non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_InitAllChannels(BAST_Handle h)
{
   /* complete tuning and acquisition in ISR context */
   BAST_g3_P_Handle *pDev = (BAST_g3_P_Handle*)(h->pImpl);
   pDev->counter = 0;
   return BAST_g3_P_EnableTimer(h->pChannels[0], BAST_TimerSelect_eBaudUsec, 30, BAST_g3_P_InitAllChannels_isr);
}


/******************************************************************************
 BAST_g3_P_InitAllChannels_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g3_P_InitAllChannels_isr(BAST_ChannelHandle h)
{
   BERR_Code retCode;

#ifdef BAST_HAS_WFE
   BAST_g3_P_Handle *pDev = (BAST_g3_P_Handle*)(h->pDevice->pImpl);
   int i;

   for (i = 0; i < h->pDevice->totalChannels; i++)
   {
      BAST_CHK_RETCODE(BAST_g3_P_InitChannel_isr(h->pDevice->pChannels[i]));
      BAST_CHK_RETCODE(BAST_g3_P_TunerInit_isr(h->pDevice->pChannels[i], NULL));
   }
   BKNI_SetEvent(pDev->hInitDoneEvent);
#else
   BAST_CHK_RETCODE(BAST_g3_P_InitChannel_isr(h));
   retCode = BAST_g3_P_TunerInit_isr(h, BAST_g3_P_InitNextChannel_isr);
#endif
   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_InitChannel_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g3_P_InitChannel_isr(BAST_ChannelHandle h)
{
   static const uint32_t script_sds_init[] =
   {
#ifdef BAST_HAS_WFE
      /* BCM4528 */
      BAST_SCRIPT_WRITE(BCHP_SDS_CG_CGDIV01, 0x005D0119), /* 375KHz mi2c clk */
      BAST_SCRIPT_AND(BCHP_SDS_MISC_MISCTL, ~0x00000008), /* power down ring oscillators for noise reduction */
      BAST_SCRIPT_WRITE(BCHP_SDS_BL_BFOS, 0x03A0CC00), /* assume Fb=20, Fs=145.125 */
      BAST_SCRIPT_WRITE(BCHP_SDS_MISC_TPDIR, 0xFF4B008F),
      /*BAST_SCRIPT_WRITE(BCHP_SDS_MISC_TPCTL2, 0x8F000000),*/
      BAST_SCRIPT_WRITE(BCHP_SDS_FE_ADCPCTL, 0x000F0001),  /* i_nofifo=1, q_nofifo=1, insel=0, ob_tc=0 */
      BAST_SCRIPT_AND(BCHP_SDS_FE_ADCPCTL, ~0x1), /* clear reset */
      BAST_SCRIPT_WRITE(BCHP_SDS_FE_IQCTL, 0x00008284), /* bypass IQ phase/amplitude imbalance correction */
      BAST_SCRIPT_WRITE(BCHP_SDS_FE_DSTGCTL, 0x00000001), /* bypass destagger */
      BAST_SCRIPT_WRITE(BCHP_SDS_FE_DCOCTL, 0x00020311), /* bypass DCO */
#else
      BAST_SCRIPT_WRITE(BCHP_SDS_CG_CGDIV01, 0x005D0119), /* 375KHz mi2c clk */
      /* BAST_SCRIPT_OR(BCHP_SDS_CG_CGDIV00, 0x02), per Hiroshi */
      BAST_SCRIPT_WRITE(BCHP_SDS_CG_SPLL_CTRL, 0x84400000), /* per Hiroshi */
   #if (BCHP_CHIP==7358)
      BAST_SCRIPT_AND_OR(BCHP_SDS_CG_SPLL_MDIV_CTRL, ~0x000FF800, 0x0003C000), /* pll_ctrl_msb=0x78 */
      BAST_SCRIPT_AND_OR(BCHP_SDS_CG_SPLL_CTRL, ~0xF0000000, 0xA0000000), /* Kp=0xA */
   #endif
   #if (BCHP_CHIP!=4517)
      BAST_SCRIPT_AND(BCHP_SDS_MISC_TPCTL2, ~0x70000000),
   #endif
      BAST_SCRIPT_AND(BCHP_SDS_MISC_MISCTL, ~0x00000008), /* power down ring oscillators for noise reduction */
      BAST_SCRIPT_WRITE(BCHP_SDS_AGC_AGCCTL, 0x00041F1F), /* IF AGC only, freeze/reset RF/IF */
      BAST_SCRIPT_WRITE(BCHP_SDS_BL_BFOS, 0x049CCC00),
      BAST_SCRIPT_WRITE(BCHP_SDS_MISC_TPDIR, 0xFF4B008F),
      BAST_SCRIPT_OR(BCHP_SDS_FE_ADCPCTL, 0x00000001),  /* reset */
      BAST_SCRIPT_AND(BCHP_SDS_FE_ADCPCTL, 0xFFFFFF00), /* clear reset */
      BAST_SCRIPT_WRITE(BCHP_SDS_FE_IQCTL, 0x00008080), /* changed from TT40G */
#endif
      BAST_SCRIPT_EXIT
   };

#ifndef BAST_EXCLUDE_EXT_TUNER
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
#endif
   BERR_Code retCode;

   BAST_CHK_RETCODE(BAST_g3_P_SetDefaultSampleFreq_isr(h));

   /* initialize clockgen and sds */
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_sds_init));

   BAST_g3_P_IndicateNotLocked_isrsafe(h);

#ifndef BAST_EXCLUDE_EXT_TUNER
   if (hChn->bExternalTuner)
   {
      /* external tuner */
      BDBG_ERR(("BAST_g3_P_InitChannel_isr(): external tuner code not implemented"));
      BDBG_ASSERT(0);
   }
#endif

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_SetDefaultSampleFreq_isr()
******************************************************************************/
BERR_Code BAST_g3_P_SetDefaultSampleFreq_isr(BAST_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;

#ifndef BAST_HAS_WFE
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   if (hChn->bFsNotDefault)
   {
#if BCHP_CHIP==4517
      BAST_CHK_RETCODE(BAST_g3_P_SetSampleFreq_isr(h, 0, 6));
#else
      BAST_CHK_RETCODE(BAST_g3_P_SetSampleFreq_isr(h, 82, 6));
#endif

      if (hChn->sampleFreq == BAST_DEFAULT_SAMPLE_FREQ)
         hChn->bFsNotDefault = false;
      else
      {
         BDBG_ERR(("unable to set default sample freq (%d Hz)", BAST_DEFAULT_SAMPLE_FREQ));
         BERR_TRACE(retCode = BAST_ERR_AP_UNKNOWN);
      }
   }

   done:
#else
   /* do nothing */
#if 0
   hChn->sampleFreq = BAST_DEFAULT_SAMPLE_FREQ;
   hChn->bFsNotDefault = false;
#endif
#endif

   return retCode;
}


/******************************************************************************
 BAST_g3_P_GetLockChangeEventHandle()
******************************************************************************/
BERR_Code BAST_g3_P_GetLockChangeEventHandle(BAST_ChannelHandle h, BKNI_EventHandle *hLockChangeEvent)
{
   BDBG_ENTER(BAST_g3_P_GetLockChangeEventHandle);

   *hLockChangeEvent = ((BAST_g3_P_ChannelHandle *)h->pImpl)->hLockChangeEvent;

   BDBG_LEAVE(BAST_g3_P_GetLockChangeEventHandle);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_ProcessScript_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_ProcessScript_isrsafe(BAST_ChannelHandle hChn, uint32_t const *pScr)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb, val, reg, op;

   while (1)
   {
      mb = *pScr++;
      val = *pScr++;
      reg = mb & ~BAST_SCRIPT_OPCODE;
      op = mb & BAST_SCRIPT_OPCODE;
      if (op == BAST_SCRIPT_OPCODE_WRITE)
      {
         BAST_g3_P_WriteRegister_isrsafe(hChn, reg, &val);
      }
      else if (op == BAST_SCRIPT_OPCODE_AND)
      {
         BAST_g3_P_ReadRegister_isrsafe(hChn, reg, &mb);
         mb &= val;
         BAST_g3_P_WriteRegister_isrsafe(hChn, reg, &mb);
      }
      else if (op == BAST_SCRIPT_OPCODE_OR)
      {
         BAST_g3_P_ReadRegister_isrsafe(hChn, reg, &mb);
         mb |= val;
         BAST_g3_P_WriteRegister_isrsafe(hChn, reg, &mb);
      }
      else if (op == BAST_SCRIPT_OPCODE_AND_OR)
      {
         BAST_g3_P_ReadRegister_isrsafe(hChn, reg, &mb);
         mb &= val;
         mb |= *pScr++;
         BAST_g3_P_WriteRegister_isrsafe(hChn, reg, &mb);
      }
      else if (op == BAST_SCRIPT_OPCODE_DEBUG)
      {
         BDBG_MSG(("script debug: %d", val));
      }
      else if (op == BAST_SCRIPT_OPCODE_EXIT)
         break;
      else
      {
         BDBG_ERR(("invalid script operation"));
         BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
         break;
      }
   }

   return retCode;
}


/******************************************************************************
 BAST_g3_P_GCF_isr()
******************************************************************************/
uint32_t BAST_g3_P_GCF_isr(uint32_t m, uint32_t n)
{
   uint32_t L1, L2, L3, L4;

   L1 = m;
   L2 = n;

   while (L2 > 0)
   {
      L3 = L1 / L2;
      L4 = L1 - (L2 * L3);
      L1 = L2;
      L2 = L4;
   }
   return L1;
}


/******************************************************************************
 BAST_g3_P_ReadModifyWriteRegister_isrsafe()
******************************************************************************/
void BAST_g3_P_ReadModifyWriteRegister_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t and_mask, uint32_t or_mask)
{
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, reg, &val);
   val = (val & and_mask) | or_mask;
   BAST_g3_P_WriteRegister_isrsafe(h, reg, &val);
}


/******************************************************************************
 BAST_g3_P_OrRegister_isrsafe()
******************************************************************************/
void BAST_g3_P_OrRegister_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t or_mask)
{
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, reg, &val);
   val |= or_mask;
   BAST_g3_P_WriteRegister_isrsafe(h, reg, &val);
}


/******************************************************************************
 BAST_g3_P_AndRegister_isrsafe()
******************************************************************************/
void BAST_g3_P_AndRegister_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t and_mask)
{
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, reg, &val);
   val &= and_mask;
   BAST_g3_P_WriteRegister_isrsafe(h, reg, &val);
}


/******************************************************************************
 BAST_g3_P_ToggleBit_isrsafe()
******************************************************************************/
void BAST_g3_P_ToggleBit_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t mask)
{
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, reg, &val);

   val |= mask;
   BAST_g3_P_WriteRegister_isrsafe(h, reg, &val);

   val &= ~mask;
   BAST_g3_P_WriteRegister_isrsafe(h, reg, &val);
}


/******************************************************************************
 BAST_g3_P_UpdateUndersampleMode_isr()
******************************************************************************/
static void BAST_g3_P_UpdateUndersampleMode_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, Fb4;

   Fb4 = hChn->acqParams.symbolRate << 2;
   val = (Fb4 * 4) / 1000;  /* .4% margin */
   Fb4 += val;
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CG_CGDIV00, &val);

   if (hChn->sampleFreq < Fb4)
   {
      BDBG_ERR(("Undersample Mode enabled"));
      hChn->bUndersample = true;
      val |= 0x01;
   }
   else
   {
      hChn->bUndersample = false;
      val &= ~0x01;
   }
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CG_CGDIV00, &val);
}


#ifndef BAST_HAS_WFE
/******************************************************************************
 BAST_g3_P_GetSampleFreqMN_isr()
******************************************************************************/
void BAST_g3_P_GetSampleFreqMN_isr(BAST_ChannelHandle h, uint32_t *pN, uint32_t *pM)
{
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CG_SPLL_NPDIV, &val);
   *pN = (val >> 24) & 0xFF;
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CG_SPLL_MDIV_CTRL, &val);
   *pM = val & 0xFF;
}
#endif


/******************************************************************************
 BAST_g3_P_SetSampleFreq_isr() -
    for narrowband tuners, Fs = (xtal * ndiv) / (4 * mdiv)
    for wideband tuner, Fs = ndiv
******************************************************************************/
BERR_Code BAST_g3_P_SetSampleFreq_isr(BAST_ChannelHandle h, uint32_t ndiv, uint32_t mdiv)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

#ifndef BAST_HAS_WFE
#if BCHP_CHIP==4517
   uint32_t reg, val;

   if (h->channel == 0)
      reg = BCHP_TM_ANA_PLL_CLK_SDS0;
   else if (h->channel == 1)
      reg = BCHP_TM_ANA_PLL_CLK_SDS1;
   else
      reg = BCHP_TM_ANA_PLL_CLK_SDS2;

   BAST_g3_P_ReadRegister_isrsafe(h, reg, &val);
   val |= (1<<16); /* LOAD_DIS=1 */
   BAST_g3_P_WriteRegister_isrsafe(h, reg, &val);
   val &= ~0xFF;
   val |= (mdiv & 0xFF);
   BAST_g3_P_WriteRegister_isrsafe(h, reg, &val);
   val &= ~(1<<16); /* LOAD_DIS=0 */
   BAST_g3_P_WriteRegister_isrsafe(h, reg, &val);
#else
   uint32_t val;
   uint16_t count = 0;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CG_SPLL_NPDIV, &val);
   val &= ~0xFF000000;
   val |= (ndiv << 24);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CG_SPLL_NPDIV, &val);

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CG_SPLL_MDIV_CTRL, &val);
   val &= ~0x000000FF;
   val |= (mdiv & 0xFF);
   val |= 0x00000100;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CG_SPLL_MDIV_CTRL, &val);
   val &= ~0x00000100;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CG_SPLL_MDIV_CTRL, &val);

   /* check spll lock status */
   do {
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CG_SPLL_STATUS, &val);
      count++;
      if (count > 100)
         BKNI_Delay(1);
   }
   while (((val & 0x1000) == 0) && (count <= 200));

   if ((val & 0x1000) == 0)
   {
      BDBG_ERR(("BAST_g3_P_SetSampleFreq_isr(): SPLL not locked"));
      return BAST_ERR_SAMPLE_FREQ;
   }
#endif

   retCode = BAST_g3_P_GetSampleFreq_isrsafe(h, &(hChn->sampleFreq));
   if (retCode == BERR_SUCCESS)
   {
      BDBG_MSG(("sample freq = %u", hChn->sampleFreq));

      if (hChn->sampleFreq == BAST_DEFAULT_SAMPLE_FREQ)
         hChn->bFsNotDefault = false;
      else
      {
         hChn->bFsNotDefault = true;
      }
   }
#else
   /* Fs is not controlled by AST PI */
   BSTD_UNUSED(ndiv);
   BSTD_UNUSED(mdiv);
   hChn->bFsNotDefault = false;
#endif

   BAST_g3_P_UpdateUndersampleMode_isr(h);

   return retCode;
}


#ifndef BAST_HAS_WFE
/******************************************************************************
 BAST_g3_P_GetVcoRefClock_isrsafe() - updates vcoRefClock
******************************************************************************/
static BERR_Code BAST_g3_P_GetVcoRefClock_isrsafe(BAST_ChannelHandle h, uint32_t *pVcoRefClock)
{
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
   uint32_t val, ndiv, pdiv, P_hi, P_lo, Q_hi;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CG_SPLL_NPDIV, &val);
   ndiv = (val >> 24) & 0xFF;
   pdiv = val & 0x07;
   if (pdiv == 0)
      pdiv = 8;
   BMTH_HILO_32TO64_Mul(hDev->xtalFreq, ndiv, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, pdiv, &Q_hi, pVcoRefClock);
   return BERR_SUCCESS;
}
#endif


/******************************************************************************
 BAST_g3_P_GetSampleFreq_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_GetSampleFreq_isrsafe(BAST_ChannelHandle h, uint32_t *pSampleFreq)
{
#if BCHP_CHIP!=4517
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
#endif
#ifndef BAST_HAS_WFE
#if BCHP_CHIP==4517
   uint32_t reg, val, Fclk_ana_pll_vco;
   extern void BAST_g3_P_GetAnaPllVcoFreq_isrsafe(BAST_Handle h, uint32_t *Fclk_ana_pll_vco);

   BAST_g3_P_GetAnaPllVcoFreq_isrsafe(h->pDevice, &Fclk_ana_pll_vco);

   if (h->channel == 0)
      reg = BCHP_TM_ANA_PLL_CLK_SDS0;
   else if (h->channel == 1)
      reg = BCHP_TM_ANA_PLL_CLK_SDS1;
   else
      reg = BCHP_TM_ANA_PLL_CLK_SDS2;

   BAST_g3_P_ReadRegister_isrsafe(h, reg, &val);
   val &= 0xFF;
   *pSampleFreq = Fclk_ana_pll_vco / (val << 1);
   return BERR_SUCCESS;
#else
   BERR_Code retCode;
   uint32_t val;

   BAST_CHK_RETCODE(BAST_g3_P_GetVcoRefClock_isrsafe(h, &(hChn->vcoRefClock)));

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CG_SPLL_MDIV_CTRL, &val);
   val &= 0x000000FF;
   *pSampleFreq = (hChn->vcoRefClock / val) >> 1;  /* div back by 2 */

   /* BDBG_MSG(("channel %d: VCO_freq=%u, Fs=%u", h->channel, hChn->vcoRefClock, *pSampleFreq)); */

   done:
   return retCode;
#endif
#else
   *pSampleFreq = hChn->sampleFreq;
   return BERR_SUCCESS;
#endif
}


/******************************************************************************
 BAST_g3_P_SetNyquistAlpha_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_SetNyquistAlpha_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t Fs_over_Fb, nvctl, pfctl;

   Fs_over_Fb = (hChn->sampleFreq * 4) / hChn->acqParams.symbolRate;
   if (Fs_over_Fb < 9)
     nvctl = 0x80;
   else if (Fs_over_Fb < 10)
     nvctl = 0x40;
   else
     nvctl = 0;

   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_NYQUIST_20)
   {
      /* alpha = 0.20 */
      nvctl |= 0x04; /* nvctl */
      pfctl = 0x01;  /* pfctl */
   }
   else
   {
      /* alpha = 0.35 */
      pfctl = 0x00; /* pfctl */
   }

   if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
      pfctl |= 0x02;

   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_NVCTL, &nvctl);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_PFCTL, &pfctl);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_SetBaudBw_isr() - damp is scaled 2^2
******************************************************************************/
BERR_Code BAST_g3_P_SetBaudBw_isr(BAST_ChannelHandle h, uint32_t bw, uint32_t damp)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t lval1, lval2, lval3, P_hi, P_lo;

   /* BDBG_MSG(("BAST_g3_P_SetBaudBw_isr(): bw=%d, damp*4=%d", bw, damp)); */
   BMTH_HILO_32TO64_Mul(bw, 1073741824, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &lval2, &lval1);
   lval2 = ((lval1 * damp) + 1) >> 1;
   lval2 &= 0xFFFFFF00;
   BMTH_HILO_32TO64_Mul(lval1, bw, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &lval3, &lval1);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BRLC, &lval2);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BRIC, &lval1);
   return BERR_SUCCESS;
}


#if 0
/******************************************************************************
 BAST_g3_P_GetNumHb() - returns the number of halfbands
******************************************************************************/
static uint32_t BAST_g3_P_GetNumHb(uint32_t Fb, uint32_t Fs)
{
   uint32_t hb_number, rx_os_ratio, P_hi, P_lo, Q_hi;

   BMTH_HILO_32TO64_Mul(Fs, 65536, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, Fb, &Q_hi, &rx_os_ratio);

   if (rx_os_ratio < (8*65536))
      hb_number = 0;
   else if (rx_os_ratio < (16*65536))
      hb_number = 1;
   else if (rx_os_ratio < (32*65536))
      hb_number = 2;
   else if (rx_os_ratio <= (64*65536))
      hb_number = 3;
   else if (rx_os_ratio <= (128*65536))
      hb_number = 4;
   else
      hb_number = 5;
   return hb_number;
}
#endif


/******************************************************************************
 BAST_g3_P_SetBfos_isr()
******************************************************************************/
BERR_Code BAST_g3_P_SetBfos_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Q_lo, hb, two_pow_hb;

   hb = BAST_g3_P_GetNumDecimatingFilters_isr(h);
   two_pow_hb = 1 << hb;
   BMTH_HILO_32TO64_Mul(16777216, hChn->sampleFreq, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Q_lo);
   BMTH_HILO_64TO64_Div32(Q_hi, Q_lo, two_pow_hb, &Q_hi, &Q_lo);

   if (hChn->bUndersample)
      Q_lo = Q_lo << 1;
   Q_lo &= ~0xFF;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BFOS, &Q_lo);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_SetDecimationFilters_isr()
******************************************************************************/
BERR_Code BAST_g3_P_SetDecimationFilters_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t Fs_over_Fb, P_hi, P_lo, Q_hi, filtctl;
   uint32_t val, val2, val3, data0;

   /* Fs_over_Fb = 2^16 * Fs/Fb */
   BMTH_HILO_32TO64_Mul(65536, hChn->sampleFreq, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Fs_over_Fb);

   val2 = Fs_over_Fb;
   data0 = 1;
   while ((val2 > 524288) && (data0 < 64))
   {
      data0 = data0 << 1;
      val2 = val2 >> 1;
   }
   val2 = Fs_over_Fb / data0;
   if ((val2 > 262144) && (val2 < 268698))
   {
      data0 = data0 >> 1;
      if (data0 == 0)
         data0 = 1;
   }

   if (data0 > 32) /* all 6 HBs are used */
      filtctl = 0x40;
   else
      filtctl = (32 - data0) | 0xC0;

   if (hChn->miscCtl & BAST_G3_CONFIG_MISC_CTL_OVERRIDE_FNDF)
   {
      /* manual override of final non-decimating filter configuration */
      val3 = hChn->miscCtl & BAST_G3_CONFIG_MISC_CTL_FNDF_MASK;
      filtctl |= (val3 << 8);
   }
   else
   {
      if (h->pDevice->settings.networkSpec == BAST_NetworkSpec_eEuro)
         val3 = 373555; /* 5.7*2^16 */
      else
         val3 = 416154; /* 6.35*2^16 */

      if (val2 > val3)
         filtctl |= 0x0000; /* quarter band */
      else if (val2 > 279183) /* 4.26*2^16 */
         filtctl |= 0x4000; /* third band */
      else
         filtctl |= 0xC000; /* half band */ /* was 0xC000 */
   }


#if BCHP_CHIP==4517
   if (filtctl == 0x40D8) /* 3.7 to 5.4 */
   {
      data0 = 4;
      filtctl = 0xC0DC;
   }
   else if (filtctl == 0x00D8) /* 2.9 to 3.6 */
   {
      filtctl = 0x40D8; /* increase from quarterband to thirdband */
   }
   else if (((filtctl & 0x00FF) == 0xD0) || ((filtctl & 0x00FF) == 0xC0))
   {
      if (hChn->acqParams.symbolRate < 2500000)
      {
         data0 = 8;
         filtctl = 0x00D8;
      }
      else /* 2.5 to 2.8 */
      {
         filtctl = 0x40D8;
         data0 = 8;
      }
   }
#endif

   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL, &filtctl);
/*BDBG_ERR(("Fb=%u, FILTCTL=0x%X", hChn->acqParams.symbolRate, filtctl));*/
   filtctl &= 0xFF;
   if (filtctl == 0x10)
      val = 0x10;
   else if (filtctl == 0x00)
      val = 0x30;
   else
      val = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_DFCTL, &val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_ConfigFe_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_ConfigFe_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val;

   val = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLIF, &val);

   /* AGF setup */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_FE_AGFCTL, 0x00000001); /* reset AGF */
   if (BAST_MODE_IS_LEGACY_QPSK(hChn->acqParams.mode))
      val = 0x04300000;
   else
      val = 0x0A0A0000;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_AGF, &val);
   val = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_AGFCTL, &val);

   val = 1;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_MIXCTL, &val);

   /* reset DCO */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_FE_DCOCTL, 0x00000300);

   /* reset IQ */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_FE_IQCTL, 0x00000001);

   BAST_CHK_RETCODE(BAST_g3_P_SetNyquistAlpha_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_SetBaudBw_isr(h, hChn->acqParams.symbolRate / 100, 4));

   val = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BRI, &val);

   /* reset BL */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_BL_BLPCTL, 0x00000100);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_InitBert_isr()
******************************************************************************/
BERR_Code BAST_g3_P_InitBert_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = 0xA5;
#if (BCHP_CHIP==4538) && (BCHP_VER>=BCHP_VER_B0)
   val |= 0x1000; /* disable clear-on-read */
#endif
   if ((hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_PN_INVERT) == 0)
      val |= 0x0200;
   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_PRBS15)
      val |= 0x0100;
   val |= 1; /* hold BERT in reset state */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BERCTL, &val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_StartBert_isr()
******************************************************************************/
BERR_Code BAST_g3_P_StartBert_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_BERT_ENABLE)
   {
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_BERT_BERCTL, &val);
      val &= 0xFFFFFF00;

      /* enable BERT */
      if (hChn->xportSettings.bSerial)
         val |= 0xD0; /* serial */
      else
         val |= 0xC0; /* parallel */

      if ((hChn->xportSettings.bXbert) && (BAST_MODE_IS_DTV(hChn->acqParams.mode)))
      {
         val |= 0x08; /* XBERT */
      }

      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BERCTL, &val);
      val |= 0x01;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BERCTL, &val);
      val |= 0x02;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BERCTL, &val);
      val &= ~1;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BERCTL, &val);
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_SetCarrierBw_isr() - damp is scaled 2^2
******************************************************************************/
BERR_Code BAST_g3_P_SetCarrierBw_isr(BAST_ChannelHandle h, uint32_t bw, uint32_t damp)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t lval1, lval2, P_hi, P_lo, x, mb, stfllc;
   static const uint32_t int_scale[7] = {1, 4, 16, 64, 256, 1024, 4096};
   static const uint32_t lin_scale[5] = {1, 4, 16, 64, 256};

   for (x = 0; x < 5; x++)
   {
      lval1 = ((damp * bw) >> 1) * lin_scale[x];
      if (lval1 > hChn->sampleFreq)
         break;
   }
   if (x)
      x--;

   BMTH_HILO_32TO64_Mul(damp * bw, 8192 * 256 * lin_scale[x], &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &lval2, &lval1);
   stfllc = (lval1 << 7) + 0x8000;
   stfllc &= 0x3FFF0000;
   stfllc |= ((x & 0x07) << 8);

   /* compute 16-bit integrator coefficient */
   for (x = 0; x < 7; x++)
   {
      BMTH_HILO_32TO64_Mul(bw * bw, int_scale[x], &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &lval2, &lval1);
      if (lval1 > hChn->sampleFreq)
         break;
   }
   if (x)
      x--;

   lval2 = ((bw + 5) / 10);
   BMTH_HILO_32TO64_Mul(lval2 * int_scale[x] * 8, lval2 * 8192, &P_hi, &P_lo);
   lval1 = ((hChn->acqParams.symbolRate + 5) / 10);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, lval1, &P_hi, &P_lo);
   lval1 = ((hChn->sampleFreq + 5) / 10);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, lval1, &P_hi, &P_lo);
   mb = ((P_lo << 15) + 0x8000);
   mb &= 0xFFFF0000;
   mb |= ((x & 0x0F) << 8);

   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLLC, &stfllc);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLIC, &mb);
   /* BDBG_MSG(("BAST_g3_P_SetCarrierBw_isr(): bw=%d, damp*4=%d, fllc=0x%X, flic=0x%X", bw, damp, stfllc, mb)); */
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_ConfigOif_isr()
 input: opll_N, opll_D, outputBitRate
******************************************************************************/
BERR_Code BAST_g3_P_ConfigOif_isr(BAST_ChannelHandle h)
{
   /* set OI_OPLL, OI_OPLL2, OI_OIFCTL01, OI_OSUBD, OI_OPLL_NPDIV OI_OPLL_MDIV_CTRL, OI_OIFCTL00 */
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
   uint32_t d_factor, compensated_d, compensated_d_n, byt_clk_sel, bit_clk_opll, val, ndiv_int,
            vco_ref_clock, vco_base, vco_residue, m1div, ndiv_frac, P_hi, P_lo, Q_hi, Q_lo,
            vco_freq;

   for (d_factor = 1; d_factor < 8; d_factor = d_factor << 1)
   {
      if ((hChn->opll_N << 1) < (hChn->opll_D * d_factor))
         break;
   }

   compensated_d = (uint32_t)(hChn->opll_D * (uint32_t)d_factor);
   compensated_d_n = compensated_d - hChn->opll_N;

   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OPLL, &(hChn->opll_N));
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OPLL2, &compensated_d_n);

   switch (d_factor)
   {
      case 1:
         byt_clk_sel = 0x000;
         break;

      case 2:
         byt_clk_sel = 0x100;
         break;

      case 4:
         byt_clk_sel = 0x200;
         break;

      default: /* d_factor==8 */
         byt_clk_sel = 0x300;
         break;
   }
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01, &val);
   val &= ~0x00001B00; /* clear bits 8, 9, 11, 12 */
   val |= byt_clk_sel;
   val |= (byt_clk_sel << 3);

#if 1
   val &= ~0x7800; /* clear bits 14:11 */
#else
   /* Hiroshi: Please program OIFCTL01[14:13]=01 when you program OIFCTL01[9:8]=01 (3/4 8PSK case).
           Likewise, please program OIFCTL01[14:13]=10 when you program OIFCTL01[9:8]=10 (very high XPT speed cases).
           There is a difference in the output IF operation mode between 2/3 8PSK 20Mbaud and 3/4 8PSK 20Mbaud.
           In 2/3 8PSK 20 Mbaud, we use byt_clk=bit_clk/8 whereas in 3/4 8PSK case, we use byt_clk=bit_clk/4.
           Since we got rid of dual-edge NCO, we cannot generate fast bit_clk from NCO.
           We need to generate either bit_clk=4*byt_clk or bit_clk=8*byt_clk. */
   val2 = (val >> 8) & 0x3;
   if ((val2 == 1) || (val2 == 2))
   {
      val &= ~0x6000; /* clear bits 14:13 */
      val |= (val2 << 13);
   }
#endif
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01, &val);

   /* val = ceil(4000000 / hChn->outputBitRate); */
   val = ((4000000 / hChn->outputBitRate) + 1) & 0x000000FF;
   bit_clk_opll = val * hChn->outputBitRate;

   if ((val & 0x1F) != 1)
      val |= 0x40;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OSUBD, &val);

   if (hChn->xportSettings.bOpllBypass)
   {
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01, 0x14);
   }
   else
   {
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01, ~0x14);

      /* set OPLL_NPDIV, OPLL_MDIV_CTRL */
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_OPLL_NPDIV, &val);
      val &= 0x07;
      if (val == 0)
         val = 8;
      vco_ref_clock = hDev->xtalFreq / val;
      for (ndiv_int = 30; ndiv_int <= 59; ndiv_int++)
      {
         vco_base = ndiv_int * vco_ref_clock;

         /* m1div = ceil(vco_base/bit_clk_opll) */
         m1div = vco_base / bit_clk_opll;
         if ((vco_base % bit_clk_opll) > 0)
            m1div++;

         if (m1div >= 255)
            continue;

         vco_freq = m1div * bit_clk_opll;
         if ((vco_freq < 1000000000UL) || (vco_freq > 1600000000UL))
            continue;

         vco_residue = (m1div * bit_clk_opll) - vco_base;
         if (vco_residue >= vco_ref_clock)
            continue;

         BMTH_HILO_32TO64_Mul(vco_residue, 1048576 << 1, &P_hi, &P_lo);
         BMTH_HILO_64TO64_Div32(P_hi, P_lo, vco_ref_clock, &Q_hi, &Q_lo);
         ndiv_frac = (Q_lo + 1) >> 1;

         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_OPLL_NPDIV, &val);
         val &= 0x00000007;
         val |= ((ndiv_int << 24) & 0xFF000000);
         val |= ((ndiv_frac << 4) & 0x00FFFFF0);
         BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OPLL_NPDIV, &val);

         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_OPLL_MDIV_CTRL, &val);
         val &= 0x000FF800; /* save pll_ctrl_msb */

         m1div |= BCHP_SDS_OI_OPLL_MDIV_CTRL_channel_load_en_MASK;
         m1div |= val;
         BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OPLL_MDIV_CTRL, &m1div);
         m1div &= ~BCHP_SDS_OI_OPLL_MDIV_CTRL_channel_load_en_MASK;
         BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OPLL_MDIV_CTRL, &m1div);
         break;
      }

      /* enable the loop */
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01, &val);
      val |= BCHP_SDS_OI_OIFCTL01_loop_en_MASK;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01, &val);
   }

   val = 0x01; /* reset oif */
   if (hChn->xportSettings.bSerial)
      val |= 0x02;
   if (hChn->xportSettings.bClkInv)
      val |= 0x08;
   if (hChn->xportSettings.bClkSup)
      val |= 0x10;
   if (hChn->xportSettings.bVldInv)
      val |= 0x20;
   if (hChn->xportSettings.bSyncInv)
      val |= 0x40;
   if (hChn->xportSettings.bErrInv)
      val |= 0x80;
   if (BAST_MODE_IS_TURBO(hChn->actualMode))
      val |= 0x8100;
   else if (BAST_MODE_IS_LDPC(hChn->actualMode))
      val |= 0x8000;
   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_PN_INVERT)
      val |= 0x0800;
   if (hChn->xportSettings.bXbert)
      val |= 0x0400;
   if (hChn->xportSettings.bDelHeader)
      val |= 0x800000;
   if (hChn->xportSettings.bHead4)
      val |= 0x400000;
   if (hChn->xportSettings.bSync1)
      val |= 0x200000;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL00, &val);
   val &= ~1; /* toggle OIF reset */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL00, &val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_SetFfeMainTap_isr()
******************************************************************************/
BERR_Code BAST_g3_P_SetFfeMainTap_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)(h->pImpl);
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, &val);
   val &= 0xFF00FFFF;
   if ((BAST_MODE_IS_DTV(hChn->acqParams.mode)) || hChn->bUndersample)
      val |= 0x00060000;
   else
      val |= 0x000C0000;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, &val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_ConfigAgc_isr()
******************************************************************************/
BERR_Code BAST_g3_P_ConfigAgc_isr(BAST_ChannelHandle h)
{
#ifdef BAST_HAS_WFE
   /* don't have to do anything here since AGCs were already configured in BAST_g3_P_TunerSetFreq_isr() */
   BSTD_UNUSED(h);
   return BERR_SUCCESS;
#else
   static const uint32_t script_agc_init[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_AGC_ABW, 0x0A0A0000),
      BAST_SCRIPT_WRITE(BCHP_SDS_AGC_AGCCTL, 0x00041F19),   /* enable IF AGC only, freeze/reset RF/IF */
      BAST_SCRIPT_AND(BCHP_SDS_AGC_AGCCTL, ~0x00000101),    /* clear IF and RF AGC reset */
      BAST_SCRIPT_EXIT
   };

   BERR_Code retCode;
   uint32_t val;

   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_agc_init));

   if (h->pDevice->settings.networkSpec == BAST_NetworkSpec_eDefault)
      val = 0x18000000;
   else
      val = 0x10000000;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_AGC_ATHR, &val);

   done:
   return retCode;
#endif
}


/******************************************************************************
 BAST_g3_P_SetAgcTrackingBw_isr()
******************************************************************************/
BERR_Code BAST_g3_P_SetAgcTrackingBw_isr(BAST_ChannelHandle h)
{
#ifdef BAST_HAS_WFE
      BAST_g3_P_ConfigChanAgc_isr(h, true);
      return BERR_SUCCESS;
#else
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   if (hChn->agcCtl & BAST_G3_CONFIG_AGC_CTL_METER_MODE)
      val = 0x0A0A0000;
   else
      val = 0x03030000;

   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_AGC_ABW, &val);
   return BERR_SUCCESS;
#endif
}


/******************************************************************************
 BAST_g3_P_CheckTimingLoopStateMachine_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_CheckTimingLoopStateMachine_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val, i;
   int32_t  bfos_step, sum, bri;

   while (hChn->funct_state != 0xFFFFFFFF)
   {
      /* BDBG_MSG(("BAST_g3_P_CheckTimingLoop(): state=%d", hChn->funct_state)); */
      switch (hChn->funct_state)
      {
         case 0:
            /* open up timing loop bw */
            BAST_CHK_RETCODE(BAST_g3_P_SetBaudBw_isr(h, hChn->acqParams.symbolRate / 400, 4 * 4));

            bfos_step = hChn->bfos >> 9;
            if (hChn->count1 == 0)
               bfos_step = -bfos_step;

            val = (uint32_t)((int32_t)hChn->bfos + bfos_step);
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BFOS, &val);

            /* reset baud loop integrator */
            BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_BL_BLPCTL, 0x00000100);
            val = 0;
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BRI, &val);

            /* wait some time */
            hChn->funct_state = 1;
            return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, 300000, BAST_g3_P_CheckTimingLoopStateMachine_isr);

         case 1:
            /* Narrow baud loop bandwidth */
            BAST_CHK_RETCODE(BAST_g3_P_SetBaudBw_isr(h, hChn->acqParams.symbolRate / 1600, 11 * 4));

            sum = 0;
            for (i = 32; i; i--)
            {
               BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_BL_BRI, (uint32_t*)&bri);
               sum += (bri / 32);
            }

            if (hChn->count1)
            {
               if (sum > 0)
                  sum = 0;
               else
                  sum = -sum;
            }
            else if (sum < 0)
               sum = 0;

            /* restore original BFOS */
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BFOS, &(hChn->bfos));

            val = 0;
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BRI, &val);

            if ((hChn->tunerCtl & BAST_G3_CONFIG_TUNER_CTL_PRESERVE_COMMANDED_FREQ) == 0)
               BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLIF, &val);

            if (sum > 0x20000)
            {
               if (hChn->count1)
               {
                  /* indicate timing loop locked */
                  hChn->bTimingLock = true;
                  hChn->funct_state = 0xFFFFFFFF;
               }
               else
               {
                  hChn->count1 = 1;
                  hChn->funct_state = 0;
               }
            }
            else
               hChn->funct_state = 0xFFFFFFFF;
            break;

         default:
            break;
      }
   }

   retCode = hChn->passFunct(h);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_CheckTimingLoop_isr() - sets bTimingLock if timing loop is locked
******************************************************************************/
static BERR_Code BAST_g3_P_CheckTimingLoop_isr(BAST_ChannelHandle h, BAST_g3_FUNCT nextFunct)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   hChn->passFunct = nextFunct;
   hChn->bTimingLock = false;
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_BL_BFOS, &(hChn->bfos));
   hChn->funct_state = 0;
   hChn->count1 = 0;
   return BAST_g3_P_CheckTimingLoopStateMachine_isr(h);
}


/******************************************************************************
 BAST_g3_P_SignalDetectModeExit()
******************************************************************************/
static BERR_Code BAST_g3_P_SignalDetectModeExit(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   if (hChn->bTimingLock)
      hChn->signalDetectStatus |= BAST_SIGNAL_DETECT_TIMING_LOCK;
   hChn->signalDetectStatus &= ~BAST_SIGNAL_DETECT_IN_PROGRESS;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_Acquire2_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_Acquire2_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   if (hChn->bTimingLock)
   {
      hChn->count1 = hChn->count2 = 0;
      return hChn->acqFunct(h);
   }

   BDBG_WRN(("timing loop not locked"));
   hChn->reacqCause = BAST_ReacqCause_eTimingLoopNotLocked;
   return BAST_g3_P_Reacquire_isr(h);
}


/******************************************************************************
 BAST_g3_P_VcoAvoidance_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_VcoAvoidance_isr(BAST_ChannelHandle h)
{
#if (BCHP_CHIP==4517) || (BCHP_CHIP==7346) || (BCHP_CHIP==73465)
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t vco_avoidance_spacing_khz = 1000; /* default 1MHz */
   uint32_t lo_vco_khz, lo_div, fddfs_khz, i, i_lo, i_hi, harmonic = 0, range_lo = 0, range_hi = 0, range, Fs_khz;
   int32_t lo_vco_shift;
   bool bShiftVco = false;

#if BCHP_CHIP==4517
   #define num_special_case_freqs 3
   static const uint32_t special_case_freqs[num_special_case_freqs] = {2530125000UL, 2683500000UL, 2952400000UL};
   BERR_Code BAST_g3_P_TunerUpdateActualTunerFreq_isr(BAST_ChannelHandle h);
#endif

   if ((hChn->tunerCtl & (BAST_G3_CONFIG_TUNER_CTL_DISABLE_VCO_AVOIDANCE | BAST_G3_CONFIG_TUNER_CTL_PRESERVE_COMMANDED_FREQ))
       || (hChn->acqParams.symbolRate < 19000000))
      return BAST_g3_P_Acquire1_isr(h);

   /* LO VCO avoidance from Fddfs/4 harmonics (i.e. 1188/4=297MHz) */
   BAST_CHK_RETCODE(BAST_g3_P_TunerGetLoVco_isr(h, &lo_vco_khz, &lo_div));
   fddfs_khz = hChn->tunerFddfs / 4000;
   range = vco_avoidance_spacing_khz * lo_div;
   if (lo_div < 3)
      range = (range * 3); /* 3 MHz spacing for lower lo_div */
   BAST_DEBUG_VCO_AVOIDANCE(BDBG_ERR(("Fddfs/4 avoidance: LO_VCO=%u KHz, Fddfs/4=%u KHz, range=%u, lo_div=%d", lo_vco_khz, fddfs_khz, range, lo_div)));
   i = lo_vco_khz / fddfs_khz;
   if (i > 1)
      i_lo = i - 1;
   else
      i_lo = 1;
   i_hi = i + 1;
   for (i = i_lo; i <= i_hi; i++)
   {
      harmonic = fddfs_khz * i;
      range_lo = harmonic - range;
      range_hi = harmonic + range;
      if ((lo_vco_khz >= range_lo) && (lo_vco_khz <= range_hi))
      {
         BAST_DEBUG_VCO_AVOIDANCE(BDBG_ERR(("detected ddfs/4 harmonic (%u KHz) too close to LO VCO!", harmonic)));
         bShiftVco = true;
         goto search_done;
      }
   }

   /* LO VCO avoidance from Fs harmonics */
   Fs_khz = hChn->sampleFreq / 1000;
   i = lo_vco_khz / Fs_khz;
   if (i > 1)
      i_lo = i - 1;
   else
      i_lo = 1;
   i_hi = i + 1;
   BAST_DEBUG_VCO_AVOIDANCE(BDBG_ERR(("Fs Avoidance: Fs_khz=%u", Fs_khz)));
   for (i = i_lo; i <= i_hi; i++)
   {
      harmonic = Fs_khz * i;
      range_lo = harmonic - range;
      range_hi = harmonic + range;
      if ((lo_vco_khz >= range_lo) && (lo_vco_khz <= range_hi))
      {
         BAST_DEBUG_VCO_AVOIDANCE(BDBG_ERR(("detected Fs harmonic (%u KHz) too close to LO VCO!, lo_vco_khz=%u", harmonic, lo_vco_khz)));
         BAST_DEBUG_VCO_AVOIDANCE(BDBG_ERR(("range=%u, range_hi=%u", range, range_hi)));

         bShiftVco = true;
         goto search_done;
      }
   }

#if BCHP_CHIP==4517
   BAST_g3_P_TunerUpdateActualTunerFreq_isr(h);
   range = 1000000; /*(range / lo_div) * 1000 */
   for (i = 0; !bShiftVco && (i < num_special_case_freqs); i++)
   {
      harmonic = special_case_freqs[i];
      range_lo = harmonic - range;
      range_hi = harmonic + range;
      if ((hChn->actualTunerFreq >= range_lo) && (hChn->actualTunerFreq <= range_hi))
      {
         BAST_DEBUG_VCO_AVOIDANCE(BDBG_ERR(("VCO Avoidance special case freq %u", harmonic)));
         if (hChn->actualTunerFreq > harmonic)
            lo_vco_shift = harmonic - hChn->actualTunerFreq + range;
         else
            lo_vco_shift = harmonic - hChn->actualTunerFreq - range;
         goto retune;
      }
   }
#endif

   search_done:
   if (!bShiftVco)
      return BAST_g3_P_Acquire1_isr(h);

   /* shift the tuning freq 1MHz away from the harmonic */
   if (harmonic <= lo_vco_khz)
   {
      if (((int32_t)lo_vco_khz - (int32_t)harmonic) < ((int32_t)range_hi - (int32_t)lo_vco_khz))
         lo_vco_shift = (int32_t)harmonic - (int32_t)lo_vco_khz; /* align LO VCO with harmonic since current LO VCO is closer to harmonic */
      else
         lo_vco_shift = (int32_t)range_hi - (int32_t)lo_vco_khz; /* move LO VCO to range_hi */
   }
   else
   {
      if ((int32_t)(harmonic - lo_vco_khz) < (int32_t)(lo_vco_khz - range_lo))
         lo_vco_shift = (int32_t)harmonic - (int32_t)lo_vco_khz; /* align LO VCO with harmonic since current LO VCO is closer to harmonic */
      else
         lo_vco_shift = (int32_t)range_lo - (int32_t)lo_vco_khz;
   }
   lo_vco_shift = (lo_vco_shift * 1000) / (int32_t)lo_div;

#if BCHP_CHIP==4517
   retune:
#endif
   hChn->tunerIfStep += lo_vco_shift;
   BAST_DEBUG_VCO_AVOIDANCE(BDBG_ERR(("--> Shift LO VCO by %d Hz, tunerIfStep=%d", lo_vco_shift, hChn->tunerIfStep)));
   hChn->bVcoAvoidance = true;
   if (lo_vco_shift < 0)
      hChn->tunerVcoAvoidanceLpfChange = -lo_vco_shift;
   else
      hChn->tunerVcoAvoidanceLpfChange = lo_vco_shift;
   hChn->tunerVcoAvoidanceLpfChange += 500000;
   hChn->tunerVcoAvoidanceLpfChange /= 1000000; /* round to nearest MHz */
   BAST_DEBUG_VCO_AVOIDANCE(BDBG_ERR(("tuner LPF change is %u MHz", hChn->tunerVcoAvoidanceLpfChange)));
   BAST_g3_P_SetFlifOffset_isr(h, -lo_vco_shift);
   retCode = BAST_g3_P_TunerQuickTune_isr(h, BAST_g3_P_Acquire1_isr);

   done:
   return retCode;
#else
   return BAST_g3_P_Acquire1_isr(h);
#endif
}


/******************************************************************************
 BAST_g3_P_CheckTimingLoop0_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_CheckTimingLoop0_isr(BAST_ChannelHandle h)
{
   return BAST_g3_P_CheckTimingLoop_isr(h, BAST_g3_P_SignalDetectModeExit);
}


/******************************************************************************
 BAST_g3_P_Acquire1_isr()
******************************************************************************/
BERR_Code BAST_g3_P_Acquire1_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t delay;
#if (BCHP_CHIP==4517)
   uint32_t lo_vco_khz, lo_div;
#endif

   static const uint32_t script_qpsk_0[] =
   {
      BAST_SCRIPT_AND_OR(BCHP_SDS_CL_CLCTL1, ~0x000000FF, 0x00000013),
      BAST_SCRIPT_AND_OR(BCHP_SDS_CL_CLCTL1, ~0x000000FF, 0x00000010),
      BAST_SCRIPT_AND_OR(BCHP_SDS_CL_CLCTL2, ~0x0000FF00, 0x00007000),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_EQMISCCTL, 0x00000400),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_EQCFAD, 0x00000046),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_F0B, 0x16000000),
      BAST_SCRIPT_AND_OR(BCHP_SDS_BL_BLPCTL, ~0x000000FF, 0x00000007),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_VLCTL, 0x0000F185),
      BAST_SCRIPT_WRITE(BCHP_SDS_FEC_FERR, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_PILOTCTL, 0x0),
      BAST_SCRIPT_EXIT
   };

#if (BCHP_CHIP==4517)
   BAST_CHK_RETCODE(BAST_g3_P_TunerGetLoVco_isr(h, &lo_vco_khz, &lo_div));
   BAST_DEBUG_VCO_AVOIDANCE(BDBG_ERR(("final LO VCO=%u KHz", lo_vco_khz)));
#endif

#ifndef BAST_HAS_WFE
   BAST_CHK_RETCODE(BAST_g3_P_TunerSetFilter_isr(h, true)); /* set tracking LPF */
#endif

   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_qpsk_0));
   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_SIGNAL_DETECT_MODE)
   {
      /* signal detect mode */
      if (hChn->acqParams.symbolRate <= 2000000)
         delay = 15000;
      else if (hChn->acqParams.symbolRate <= 5000000)
         delay = 10000;
      else
         delay = 1000;
      return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, delay, BAST_g3_P_CheckTimingLoop0_isr);
   }
   else if (hChn->miscCtl & BAST_G3_CONFIG_MISC_CTL_CHECK_TIMING_LOOP)
      retCode = BAST_g3_P_CheckTimingLoop_isr(h, BAST_g3_P_Acquire2_isr);
   else
   {
      hChn->bTimingLock = true;
      retCode = BAST_g3_P_Acquire2_isr(h);
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_Acquire0_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_Acquire0_isr(BAST_ChannelHandle h)
{
   static const uint32_t script_acq_1[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_CLFBCTL, 0x00000002), /* turn off fwd-bckwd loop */
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_CLDAFECTL, 0x00010001),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_VCOS, 0x00004000),
      BAST_SCRIPT_WRITE(BCHP_SDS_SNR_SNRCTL, 0x01),
      BAST_SCRIPT_AND(BCHP_SDS_CL_CLCTL2, 0x00FFFFFF), /* CLOON=0x00 */
      BAST_SCRIPT_OR(BCHP_SDS_VIT_VTCTL, 0x00004000), /* reset viterbi block */
      BAST_SCRIPT_AND(BCHP_SDS_VIT_VTCTL, 0xFFFF00FF), /* VTCTL2=0x00 */
      BAST_SCRIPT_WRITE(BCHP_SDS_FEC_FECTL, 0x01),
      BAST_SCRIPT_WRITE(BCHP_SDS_FEC_FECTL, 0x00),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_EQFFECTL, 0x22000250), /* need to unfreeze EQ for Legacy, was 0x00000740 */
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_VLCI, 0x08000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_VLCQ, 0x08000000),
#if 1
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_CLCTL1, 0x0C8A1073), /* orig:0x008A1073 */
#else
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_CLCTL1, 0x0C881073), /* close loop in the receiver */
#endif
      BAST_SCRIPT_AND(BCHP_SDS_CL_CLCTL1, ~0x00000003), /* clear carrier loop reset bit */
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_CLCTL2, 0x00000003),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_CLFFCTL, 0x02),  /* bypass fine mixer */
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_PLDCTL, 0x0000800a),   /* bypass rescrambler/descrambler */
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_PLC, 0x06080518),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLTD, 0x20000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLI, 0x00000000),
      /* read only: BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLPA, 0x00000000), */
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_PLTD, 0x20000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_PLI, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_PLPA, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_HDQPSK, 0x01000000),
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_acq_2[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLLC, 0x01000100),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLIC, 0x01000000),
      BAST_SCRIPT_EXIT
   };

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t ffe_main_tap, i, val, eqcfad, f0b, dco_ctl;
   BERR_Code retCode;

   BAST_CHK_RETCODE(BAST_g3_P_SetDecimationFilters_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_SetBfos_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_ConfigFe_isr(h));

   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_TUNER_TEST_MODE)
   {
#ifndef BAST_HAS_WFE
      BAST_CHK_RETCODE(BAST_g3_P_TunerSetFilter_isr(h, true)); /* set tracking LPF */
#endif
      hChn->acqState = BAST_AcqState_eIdle;
      return BERR_SUCCESS; /* exit the acquisition */
   }

   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_acq_1));

   if (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM)
   {
      val = 0;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLFFCTL, &val);
   }
   BAST_CHK_RETCODE(BAST_g3_P_SetFfeMainTap_isr(h));

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, &val);
   ffe_main_tap = (val >> 16) & 0x1F;
   for (i = 0; i < 24; i++)
   {
      val = 0x00;
      if (i == ffe_main_tap)
      {
#ifdef BAST_HAS_WFE
         val = 0x11;
#else
         val = 0x16; /*0x18*/
#endif
      }

      eqcfad = 0x40 | i;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQCFAD, &eqcfad);

      f0b = (val & 0xFF) << 24;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_F0B, &f0b);
   }
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 1); /* reset the eq */

   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_acq_2));
   BAST_CHK_RETCODE(BAST_g3_P_ConfigPlc_isr(h, true)); /* set acquisition plc */

   if (hChn->peakScanStatus & BAST_PEAK_SCAN_STATUS_ENABLED)
   {
#if BCHP_CHIP!=4538
      if (hChn->peakScanStatus & BAST_PEAK_SCAN_STATUS_DUMP_BINS)
         return BAST_g3_P_DftDumpBins_isr(h);
      else
#endif
      {
         if ((hChn->peakScanSymRateMin == 0) || (hChn->peakScanSymRateMax == 0))
         {
            if ((hChn->miscCtl & BSAT_G3_CONFIG_MISC_CTL_PEAKSCAN_PSD) == 0)
            {
               /* change DCO gain control */
               BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_FE_DCOCTL, &val);
               dco_ctl = 8;
               val = 0x30000 | (dco_ctl << 10);
               BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_DCOCTL, &val);
               return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 2000, BAST_g3_P_DftPeakScan_isr);
            }
         }
         return BAST_g3_P_DftPeakScan_isr(h);
      }
   }

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, &val);
   val &= 0xFFFFFF00;
   val |= 0x0000001F;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, &val);
   val &= 0xFFFFFF00;
   val |= 0x0000001C;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, &val);

   /* enable I/Q phase detector, enable baud recovery loop */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_BL_BLPCTL, ~0x000000FF, 0x00000007);

   retCode = BAST_g3_P_DftSearchCarrier_isr(h, BAST_g3_P_VcoAvoidance_isr);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_Acquire_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_Acquire_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BAST_DEBUG_ACQ(BDBG_MSG(("BAST_g3_P_Acquire_isr()")));

   /* check for bypass acquire option */
   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_BYPASS_ACQUIRE)
   {
      hChn->acqState = BAST_AcqState_eIdle;
      return BERR_SUCCESS;
   }

   hChn->acqState = BAST_AcqState_eAcquiring;
   hChn->reacqCtl &= (BAST_G3_CONFIG_REACQ_CTL_FREQ_DRIFT | BAST_G3_CONFIG_REACQ_CTL_RETUNE);
   hChn->timeSinceStableLock = 0;
   hChn->relockCount = 0;

   BAST_CHK_RETCODE(BAST_g3_P_GetActualMode_isr(h, &(hChn->actualMode)));
   BAST_CHK_RETCODE(BAST_g3_P_ConfigAgc_isr(h));
   retCode = BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 7000, BAST_g3_P_Acquire0_isr);  /* AGC delay to get the stable DFT output power at the beginning of DFT, esp. under ACI  */

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_GetActualMode_isr()
******************************************************************************/
BERR_Code BAST_g3_P_GetActualMode_isr(BAST_ChannelHandle h, BAST_Mode *pActualMode)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t fmod, vst, i;

   *pActualMode = BAST_Mode_eUnknown;

   if ((hChn->acqState == BAST_AcqState_eIdle) && (hChn->acqParams.mode == BAST_Mode_eUnknown))
      goto done;

   if ((hChn->acqParams.mode == BAST_Mode_eDvb_scan) ||
       (hChn->acqParams.mode == BAST_Mode_eDss_scan) ||
       (hChn->acqParams.mode == BAST_Mode_eDcii_scan))
      goto legacy_mode;

   if ((hChn->acqState == BAST_AcqState_eTuning) || (hChn->acqState == BAST_AcqState_eAcquiring))
   {
      if ((hChn->acqParams.mode == BAST_Mode_eLdpc_scan) ||
          (hChn->acqParams.mode == BAST_Mode_eTurbo_scan) ||
          (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM) ||
          (hChn->acqParams.mode == BAST_Mode_eBlindScan))
      {
         goto done;
      }
      else
      {
         *pActualMode = hChn->acqParams.mode;
      }
   }
   else if (BAST_MODE_IS_LEGACY_QPSK(hChn->acqParams.mode))
   {
      /* Legacy QPSK */
      legacy_mode:
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_FEC_FMOD, &fmod);
      fmod = (fmod >> 8) & 0x0C;
      if (fmod == 0x04)
         *pActualMode = BAST_Mode_eDss_1_2; /* DTV */
      else if (fmod == 0x00)
         *pActualMode = BAST_Mode_eDvb_1_2; /* DVB */
      else
         *pActualMode = BAST_Mode_eDcii_1_2; /* DCII */

      /* determine code rate */
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VST, &vst);
      vst = (vst >> 24) & 0x0F;
      switch (vst)
      {
         case 0x00: /* DVB/DTV 1/2 */
         case 0x09: /* DCII 1/2 */
            i = 0;
            break;
         case 0x01: /* DVB/DTV 2/3 */
         case 0x0B: /* DCII 2/3 */
            i = 1;
            break;
         case 0x02: /* DVB 3/4 */
         case 0x0C: /* DCII 3/4 */
         case 0x04: /* DTV 6/7 */
            i = 2;
            break;
         case 0x03: /* DVB 5/6 */
         case 0x0E: /* DCII 5/6 */
            i = 3;
            break;
         case 0x05: /* DVB 7/8 */
         case 0x0F: /* DCII 7/8 */
            i = 4;
            break;
         case 0x08: /* DCII 5/11 */
            i = 5;
            break;
         case 0x0A: /* DCII 3/5 */
            i = 6;
            break;
         case 0x0D: /* DCII 4/5 */
            i = 7;
            break;
         default:
            i = 0xFF;
            break;
      }
      if (i == 0xFF)
         *pActualMode = BAST_Mode_eUnknown;
      else
         *pActualMode += i;
   }
   else if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
   {
      /* actual mode is set after modcod in ldpc scan */
      if (hChn->acqParams.mode != BAST_Mode_eLdpc_scan)
         *pActualMode = hChn->acqParams.mode;
      else
         *pActualMode = hChn->actualMode;
   }
   else if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
   {
      /* actual mode is set at start of each acquisition in turbo scan */
      if (hChn->acqParams.mode != BAST_Mode_eTurbo_scan)
         *pActualMode = hChn->acqParams.mode;
      else
         *pActualMode = hChn->actualMode;
   }

   done:
   /* BAST_DEBUG_ACQ(BDBG_MSG(("BAST_g3_P_GetActualMode_isr(): actualMode=0x%X", *pActualMode))); */
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_GetActualMode()
******************************************************************************/
BERR_Code BAST_g3_P_GetActualMode(BAST_ChannelHandle h, BAST_Mode *pActualMode)
{
   BERR_Code retCode;

   BKNI_EnterCriticalSection();
   retCode = BAST_g3_P_GetActualMode_isr(h, pActualMode);
   BKNI_LeaveCriticalSection();
   return retCode;
}


/******************************************************************************
 BAST_g3_P_GetSymbolRateError()
******************************************************************************/
BERR_Code BAST_g3_P_GetSymbolRateError(BAST_ChannelHandle h, int32_t *pSymRateError)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t bw, shift, val, P_hi, P_lo, Q_hi, Q_lo;
   int32_t bri, sval;

   /* determine actual symbol rate */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_BL_BLPCTL, &val);
   bw = ((val >> 9) & 0x06);
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_BL_BRI, (uint32_t*)&bri);
   switch (bw)
   {
      case 0:
         shift = 6;
         break;
      case 0x02:
         shift = 8;
         break;
      case 0x04:
         shift = 10;
         break;
      default: /* 0x06 */
         shift = 12;
         break;
   }

   sval = bri / (int32_t)(1 << shift);

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_BL_BFOS, &val);
   sval += val;
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL, &val);

   sval *= (32 - (val & 0x1F));
   if ((val & 0x80) == 0) /* all 6 HBs are used */
     sval *= 2;

   BMTH_HILO_32TO64_Mul(hChn->sampleFreq, 16777216, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, (uint32_t)sval, &Q_hi, &Q_lo);

   if (hChn->bUndersample)
      Q_lo = Q_lo << 1;

   *pSymRateError = (int32_t)Q_lo - (int32_t)hChn->acqParams.symbolRate;

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_GetCarrierErrorFli_isrsafe()
******************************************************************************/
void BAST_g3_P_GetCarrierErrorFli_isrsafe(BAST_ChannelHandle h, int32_t *pCarrierErrorFli)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   int32_t sval;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_FLI, (uint32_t*)&sval); /* front loop enabled */
   if (sval >= 0)
      val = sval;
   else
      val = (uint32_t)-sval;
   BMTH_HILO_32TO64_Mul(val, hChn->sampleFreq, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 1073741824, &Q_hi, &Q_lo);  /* Q=abs(fval1) */
   BMTH_HILO_64TO64_Div32(Q_hi, Q_lo, 8, &Q_hi, &Q_lo);
   if (sval >= 0)
      *pCarrierErrorFli = (int32_t)Q_lo;
   else
      *pCarrierErrorFli = (int32_t)-Q_lo;
   /* BDBG_MSG(("carrier_error(FLI)=%d", *pCarrierErrorFli)); */
}


/******************************************************************************
 BAST_g3_P_GetCarrierErrorPli_isrsafe()
******************************************************************************/
void BAST_g3_P_GetCarrierErrorPli_isrsafe(BAST_ChannelHandle h, int32_t *pCarrierErrorPli)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   int32_t sval;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_PLI, (uint32_t*)&sval);
   if (sval >= 0)
      val = sval;
   else
      val = (uint32_t)-sval;
   BMTH_HILO_32TO64_Mul(val, hChn->acqParams.symbolRate, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 1073741824, &Q_hi, &Q_lo);  /* Q=abs(fval1) */
   BMTH_HILO_64TO64_Div32(Q_hi, Q_lo, 8, &Q_hi, &Q_lo);
   if (sval >= 0)
      *pCarrierErrorPli = (int32_t)Q_lo;
   else
      *pCarrierErrorPli = (int32_t)-Q_lo;
   /* BDBG_ERR(("carrier_error(PLI)=%d", *pCarrierErrorPli)); */
}


/******************************************************************************
 BAST_g3_P_GetCarrierError_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_GetCarrierError_isrsafe(BAST_ChannelHandle h, int32_t *pCarrierError)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   int32_t carrier_error_pli, carrier_error_fli, carrier_error, sval;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo, flif;
   bool bHpSpinv, bFrontLoopEnabled, bBackLoopEnabled;

   carrier_error = 0;
   carrier_error_pli = 0;

   bHpSpinv = false;  /* bHpSpinv=true if HP is enabled and spinv detected */
   bFrontLoopEnabled = false;
   bBackLoopEnabled = false;
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, &val);
   if (val & 0x08)   /* HP enabled */
   {
      BAST_g3_P_HpIsSpinv_isrsafe(h, &bHpSpinv);
   }

   /* check FLI and PLI for legacy modes */
   if (BAST_MODE_IS_LEGACY_QPSK(hChn->acqParams.mode))
   {
      /* check if front loop is enabled */
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE, &val);
      if (val & 0x01)
      {
         /* front loop enable not controlled by HP */
         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, &val);
         bFrontLoopEnabled = (val & 0x10) ? true : false;
      }

      /* read FLI if front loop enabled */
      if (bFrontLoopEnabled)
         BAST_g3_P_GetCarrierErrorFli_isrsafe(h, &carrier_error_fli);
      else
         carrier_error_fli = 0;
      carrier_error = carrier_error_fli;

      /* check if back loop is enabled */
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE, &val);
      if (val & 0x10)
      {
         /* back loop enable is not controlled by HP */
         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, &val);
         bBackLoopEnabled = (val & 0x1000) ? true : false;
      }

      /* read PLI if back loop enabled */
      if (bBackLoopEnabled)
         BAST_g3_P_GetCarrierErrorPli_isrsafe(h, &carrier_error_pli);
      else
         carrier_error_pli = 0;
      carrier_error += carrier_error_pli;
   }
   else
   {
      BAST_CHK_RETCODE(BAST_g3_P_HpGetFreqOffsetEstimate_isrsafe(h, &carrier_error));

      if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
      {
         BAST_g3_P_GetCarrierErrorFli_isrsafe(h, &carrier_error_fli);
         BAST_g3_P_GetCarrierErrorPli_isrsafe(h, &carrier_error_pli);
         sval = carrier_error_fli + carrier_error_pli;
         if (bHpSpinv)
            carrier_error -= sval;
         else
            carrier_error += sval;
      }
   }

   /* FLIF * Fs / 2^28 */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_FLIF, &flif);
   if (flif & 0x08000000)
      flif |= 0xF0000000; /* sign extend */
   sval = (int32_t)flif;
   if (sval >= 0)
      val = (uint32_t)sval;
   else
      val = (uint32_t)-sval;
   BMTH_HILO_32TO64_Mul(val, hChn->sampleFreq, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 268435456, &Q_hi, &Q_lo);
   if (sval >= 0)
      sval = (int32_t)Q_lo;
   else
      sval = (int32_t)-Q_lo;
   if (bHpSpinv)
      sval = -sval; /* invert if HP is enabled and spinv detect */
   carrier_error += sval;
/* BDBG_ERR(("FLIF=%d, carrier_error(FLIF)=%d", flif, sval)); */
/* BDBG_ERR(("tunerIfStep=%d", hChn->tunerIfStep)); */
   carrier_error -= hChn->tunerIfStep;
   carrier_error = -carrier_error;
   *pCarrierError = carrier_error;

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_SetFlifOffset_isr()
******************************************************************************/
BERR_Code BAST_g3_P_SetFlifOffset_isr(BAST_ChannelHandle h, int32_t offset)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, P_hi, P_lo;
   int32_t flif;

   if (offset >= 0)
      val = offset;
   else
      val = -offset;

   BMTH_HILO_32TO64_Mul(val, 268435456, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &P_hi, &P_lo);
   if (offset >= 0)
      flif = -P_lo;
   else
      flif = P_lo;

   val = (uint32_t)flif;
   val &= 0x0FFFFF00;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLIF, (uint32_t *)&val);
   /* BDBG_MSG(("BAST_g3_P_SetFlifOffset_isr(): offset=%d, FLIF=0x%08X", offset, val)); */
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_IsCarrierOffsetOutOfRange_isr()
******************************************************************************/
static bool BAST_g3_P_IsCarrierOffsetOutOfRange_isr(BAST_ChannelHandle h)
{
#ifndef BAST_EXCLUDE_EXT_TUNER
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
#endif
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
   int32_t carrierError;
   uint32_t val, range;
   BERR_Code retCode;
   bool bOutOfRange = false;

#ifndef BAST_EXCLUDE_EXT_TUNER
   if (hChn->bExternalTuner == false)
#endif
   {
      BAST_CHK_RETCODE(BAST_g3_P_GetCarrierError_isrsafe(h, &carrierError));

      if (carrierError < 0)
         val = -carrierError;
      else
         val = carrierError;

      range = (hDev->searchRange * 6) / 5;
      if (val > range)
      {
         bOutOfRange = true;
         BDBG_WRN(("carrier offset out of range (%d Hz)", carrierError));
      }
   }

   done:
   return bOutOfRange;
}


/******************************************************************************
 BAST_g3_P_IndicateLocked_isrsafe()
******************************************************************************/
void BAST_g3_P_IndicateLocked_isrsafe(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   hChn->bLocked = true;
   if (hChn->bLastLocked == false)
   {
      BDBG_MSG(("SDS%d locked", h->channel));
      BKNI_SetEvent(hChn->hLockChangeEvent);
   }

   hChn->bLastLocked = true;
}


/******************************************************************************
 BAST_g3_P_IndicateNotLocked_isrsafe()
******************************************************************************/
void BAST_g3_P_IndicateNotLocked_isrsafe(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   hChn->bLocked = false;
   if (hChn->bLastLocked)
   {
      BDBG_MSG(("SDS%d not locked", h->channel));
      BKNI_SetEvent(hChn->hLockChangeEvent);
   }

   hChn->bLastLocked = false;
}


#if 0
/******************************************************************************
 BAST_g3_P_TunerTestMode()
******************************************************************************/
static BERR_Code BAST_g3_P_TunerTestMode(BAST_ChannelHandle h)
{
   BSTD_UNUSED(h);
   /* any register settings for tuner test mode goes here... */
   return BERR_SUCCESS;
}
#endif


/******************************************************************************
 BAST_g3_P_BlindScanSetNextMode_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_BlindScanSetNextMode_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   /* determine next mode to try */
   next_mode:
   while (hChn->blindScanModes)
   {
      if (hChn->blindScanCurrMode == 0)
         hChn->blindScanCurrMode = 1;
      else
         hChn->blindScanCurrMode = (hChn->blindScanCurrMode << 1) & BAST_G3_CONFIG_BLIND_SCAN_MODES_MASK;

      if (hChn->blindScanCurrMode == 0)
         hChn->acqCount++;
      else if (hChn->blindScanCurrMode & hChn->blindScanModes)
         break;
   }

   if (hChn->blindScanCurrMode == 0)
   {
      BDBG_ERR(("BAST_g3_P_BlindScanSetNextMode_isr(): blindScanCurrMode should not be 0"));
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      return retCode;
   }

   if (hChn->blindScanCurrMode & BAST_G3_CONFIG_BLIND_SCAN_MODES_DVB)
   {
      BDBG_MSG(("Blind scan: trying DVB scan"));
      hChn->acqParams.mode = BAST_Mode_eDvb_scan;
   }
   else if (hChn->blindScanCurrMode & BAST_G3_CONFIG_BLIND_SCAN_MODES_DTV)
   {
      BDBG_MSG(("Blind scan: trying DTV scan"));
      hChn->acqParams.mode = BAST_Mode_eDss_scan;
   }
   else if (hChn->blindScanCurrMode & BAST_G3_CONFIG_BLIND_SCAN_MODES_DCII)
   {
      BDBG_MSG(("Blind scan: trying DCII scan"));
      hChn->acqParams.mode = BAST_Mode_eDcii_scan;
   }
   else if (hChn->blindScanCurrMode & BAST_G3_CONFIG_BLIND_SCAN_MODES_LDPC)
   {
#ifndef BAST_EXCLUDE_LDPC
      if (hChn->bHasAfec)
      {
         BDBG_MSG(("Blind scan: trying LDPC scan"));
         hChn->acqParams.mode = BAST_Mode_eLdpc_scan;
      }
      else
#endif
      {
         /* remove LDPC from blind scan because this channel does not have an AFEC */
         hChn->blindScanModes &= ~BAST_G3_CONFIG_BLIND_SCAN_MODES_LDPC;
         goto next_mode;
      }
   }
   else if (hChn->blindScanCurrMode & BAST_G3_CONFIG_BLIND_SCAN_MODES_TURBO)
   {
      if (hChn->bHasTfec)
      {
         BDBG_MSG(("Blind scan: trying Turbo scan"));
         hChn->acqParams.mode = BAST_Mode_eTurbo_scan;
      }
      else
      {
         /* remove Turbo from blind scan because this channel does not have a TFEC */
         hChn->blindScanModes &= ~BAST_G3_CONFIG_BLIND_SCAN_MODES_TURBO;
         goto next_mode;
      }
   }
   else
   {
      BDBG_ERR(("BAST_g3_P_BlindScanSetNextMode_isr(): invalid value for blindScanCurrMode (0x%X)", hChn->blindScanCurrMode));
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      return retCode;
   }

   retCode = BAST_g3_P_SetFunctTable_isr(h);
   return retCode;
}


/******************************************************************************
 BAST_g3_P_BlindScanInit_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_BlindScanInit_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)(h->pImpl);

   hChn->bBlindScan = true;
   hChn->blindScanCurrMode = 0;
   return BAST_g3_P_BlindScanSetNextMode_isr(h);
}


/******************************************************************************
 BAST_g3_P_ResetAcquisitionTimer_isr() - ISR context
******************************************************************************/
static BERR_Code BAST_g3_P_ResetAcquisitionTimer_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   retCode = BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eGen3, 0xFFFFFFFF, NULL);
   hChn->acqTime = 0;
   return retCode;
}


/******************************************************************************
 BAST_g3_P_GetAcquisitionTimerValue_isr()
******************************************************************************/
static void BAST_g3_P_GetAcquisitionTimerValue_isr(BAST_ChannelHandle h, uint32_t *pVal)
{
   uint32_t lval1;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_GENTMR3, &lval1);
   *pVal = (0xFFFFFFFF - lval1);
}


/******************************************************************************
 BAST_g3_P_ClearTraceBuffer()
******************************************************************************/
static BERR_Code BAST_g3_P_ClearTraceBuffer(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t i;

   for (i = 0; i < BAST_TraceEvent_eMax; i++)
      hChn->trace[i] = 0;

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LogTraceBuffer_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LogTraceBuffer_isr(BAST_ChannelHandle h, BAST_TraceEvent event)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   if (event < BAST_TraceEvent_eMax)
   {
      BAST_g3_P_GetAcquisitionTimerValue_isr(h, &(hChn->trace[event]));
      return BERR_SUCCESS;
   }
   else
      return BERR_INVALID_PARAMETER;
}


/******************************************************************************
 BAST_g3_P_IncrementInterruptCounter_isr()
******************************************************************************/
void BAST_g3_P_IncrementInterruptCounter_isr(BAST_ChannelHandle h, BAST_g3_IntID idx)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   hChn->irqCount[idx]++;
}


/******************************************************************************
 BAST_g3_P_UpdateErrorCounters_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_UpdateErrorCounters_isrsafe(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t frc, ferc, berc;

   /* take snapshot of mpeg frame and error counter */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_OI_OIFCTL01, 0x00400000);

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_FRC, &frc);
   hChn->mpegFrameCount += frc;
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_FERC, &ferc);
   hChn->mpegErrorCount += ferc;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_BERT_BERC, &berc);
#if (BCHP_CHIP==4538) && (BCHP_VER>=BCHP_VER_B0)
   /* clear on read was eliminated in BCM4538-B0 */
   hChn->berErrors = berc;
#else
   hChn->berErrors += berc;
#endif

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_ResetErrorCounters()
******************************************************************************/
BERR_Code BAST_g3_P_ResetErrorCounters(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
#if (BCHP_CHIP==4538) && (BCHP_VER>=BCHP_VER_B0)
   uint32_t val;
#endif

   BAST_CHK_RETCODE(BAST_g3_P_UpdateErrorCounters_isrsafe(h));
   BAST_CHK_RETCODE(BAST_g3_P_QpskUpdateErrorCounters_isrsafe(h));

#if (BCHP_CHIP==4538) && (BCHP_VER>=BCHP_VER_B0)
   /* for BCM4538-B0 we have to manually clear the BER error count since clear-on-read was eliminated */
   val = 0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BERC, &val);
#endif

#ifndef BAST_EXCLUDE_LDPC
   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
   {
      BAST_CHK_RETCODE(BAST_g3_P_LdpcUpdateBlockCounters_isrsafe(h));
   }
#endif
#ifndef BAST_EXCLUDE_TURBO
   if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
   {
      BAST_CHK_RETCODE(BAST_g3_P_TurboUpdateErrorCounters_isrsafe(h));
   }
#endif

   hChn->berErrors = 0;
   hChn->mpegErrorCount = 0;
   hChn->mpegFrameCount = 0;
#ifndef BAST_EXCLUDE_LDPC
   hChn->ldpcBadBlocks = 0;
   hChn->ldpcCorrBlocks = 0;
   hChn->ldpcTotalBlocks = 0;
#endif
   hChn->rsCorr = 0;
   hChn->rsUncorr = 0;
   hChn->preVitErrors = 0;
#ifndef BAST_EXCLUDE_TURBO
   hChn->turboBadBlocks = 0;
   hChn->turboCorrBlocks = 0;
   hChn->turboTotalBlocks = 0;
#endif

   done:
   return retCode;
}


#ifndef BAST_EXCLUDE_STATUS_EVENTS
/******************************************************************************
 BAST_g3_P_CheckStatusIndicators_isr()
******************************************************************************/
BERR_Code BAST_g3_P_CheckStatusIndicators_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   bool bSetEvent = false;
   int32_t offset;
   uint32_t diff, snr, snr8, val, snrThreshold1, snrThreshold2;
   int idx;

   static const uint16_t BAST_SNR_THRESHOLD1[] = /* in units of 1/256 dB */
   {
      822,  /* 0: DVB 1/2 = 3.21 */
      1308, /* 1: DVB 2/3 = 5.109375 */
      1567, /* 2: DVB 3/4 = 6.1211 */
      1817, /* 3: DVB 5/6 = 7.09766 */
      2017, /* 4: DVB 7/8 = 7.8789 */
      15 * 256, /* 5: DTV 1/2 */
      15 * 256, /* 6: DTV 2/3 */
      15 * 256, /* 7: DTV 6/7 */
      15 * 256, /* 8: DCII 1/2 */
      15 * 256, /* 9: DCII 2/3 */
      15 * 256, /* 10: DCII 3/4 */
      15 * 256, /* 11: DCII 5/6 */
      15 * 256, /* 12: DCII 7/8 */
      15 * 256, /* 13: DCII 5/11 */
      15 * 256, /* 14: DCII 3/5 */
      15 * 256, /* 15: DCII 4/5 */
      15 * 256, /* 16: LDPC QPSK 1/2 */
      15 * 256, /* 17: LDPC QPSK 3/5 */
      15 * 256, /* 18: LDPC QPSK 2/3 */
      15 * 256, /* 19: LDPC QPSK 3/4 */
      15 * 256, /* 20: LDPC QPSK 4/5 */
      15 * 256, /* 21: LDPC QPSK 5/6 */
      15 * 256, /* 22: LDPC QPSK 8/9 */
      15 * 256, /* 23: LDPC QPSK 9/10 */
      15 * 256, /* 24: LDPC 8PSK 3/5 */
      15 * 256, /* 25: LDPC 8PSK 2/3 */
      15 * 256, /* 26: LDPC 8PSK 3/4 */
      15 * 256, /* 27: LDPC 8PSK 5/6 */
      15 * 256, /* 28: LDPC 8PSK 8/9 */
      15 * 256, /* 29: LDPC 8PSK 9/10 */
      660,  /* 30: Turbo QPSK 1/2 = 2.578125 */
      1173, /* 31: Turbo QPSK 2/3 = 4.582 */
      1440, /* 32: Turbo QPSK 3/4 = 5.625 */
      1717, /* 33: Turbo QPSK 5/6 = 6.707 */
      1892, /* 34: Turbo QPSK 7/8 = 7.390625 */
      2094, /* 35: Turbo 8PSK 2/3 = 8.1797 */
      2299, /* 36: Turbo 8PSK 3/4 = 8.98047 */
      2424, /* 37: Turbo 8PSK 4/5 = 9.46875 */
      2799, /* 38: Turbo 8PSK 5/6 = 10.9336 */
      2924  /* 39: Turbo 8PSK 8/9 = 11.4219 */
   };

   static const uint16_t BAST_SNR_THRESHOLD2[] = /* in units of 1/256 dB */
   {
      552,  /* 0: DVB 1/2 = 2.15625 */
      1038, /* 1: DVB 2/3 = 4.0547 */
      1308, /* 2: DVB 3/4 = 5.109375 */
      1567, /* 3: DVB 5/6 = 6.1211 */
      1767, /* 4: DVB 7/8 = 6.90234 */
      10 * 256, /* 5: DTV 1/2 */
      10 * 256, /* 6: DTV 2/3 */
      10 * 256, /* 7: DTV 6/7 */
      10 * 256, /* 8: DCII 1/2 */
      10 * 256, /* 9: DCII 2/3 */
      10 * 256, /* 10: DCII 3/4 */
      10 * 256, /* 11: DCII 5/6 */
      10 * 256, /* 12: DCII 7/8 */
      10 * 256, /* 13: DCII 5/11 */
      10 * 256, /* 14: DCII 3/5 */
      10 * 256, /* 15: DCII 4/5 */
      10 * 256, /* 16: LDPC QPSK 1/2 */
      10 * 256, /* 17: LDPC QPSK 3/5 */
      10 * 256, /* 18: LDPC QPSK 2/3 */
      10 * 256, /* 19: LDPC QPSK 3/4 */
      10 * 256, /* 20: LDPC QPSK 4/5 */
      10 * 256, /* 21: LDPC QPSK 5/6 */
      10 * 256, /* 22: LDPC QPSK 8/9 */
      10 * 256, /* 23: LDPC QPSK 9/10 */
      10 * 256, /* 24: LDPC 8PSK 3/5 */
      10 * 256, /* 25: LDPC 8PSK 2/3 */
      10 * 256, /* 26: LDPC 8PSK 3/4 */
      10 * 256, /* 27: LDPC 8PSK 5/6 */
      10 * 256, /* 28: LDPC 8PSK 8/9 */
      10 * 256, /* 29: LDPC 8PSK 9/10 */
      400,  /* 30: Turbo QPSK 1/2 = 1.5625 */
      903,  /* 31: Turbo QPSK 2/3 = 3.5273 */
      1173, /* 32: Turbo QPSK 3/4 = 4.582 */
      1466, /* 33: Turbo QPSK 5/6 = 5.7266 */
      1642, /* 34: Turbo QPSK 7/8 = 6.4140625 */
      1842, /* 35: Turbo 8PSK 2/3 = 7.1953 */
      2042, /* 36: Turbo 8PSK 3/4 = 7.97656 */
      2172, /* 37: Turbo 8PSK 4/5 = 8.484375 */
      2549, /* 38: Turbo 8PSK 5/6 = 9.957 */
      2674  /* 39: Turbo 8PSK 8/9 = 10.4453 */
   };

   if (hChn->bStatusInterrupts == false)
      return BERR_SUCCESS;

   /* check freq drift threshold */
   BAST_CHK_RETCODE(BAST_g3_P_GetCarrierError_isrsafe(h, &offset));

   if (offset > hChn->initFreqOffset)
      diff = (uint32_t)(offset - hChn->initFreqOffset);
   else
      diff = (uint32_t)(hChn->initFreqOffset - offset);
   if (diff > hChn->freqDriftThreshold)
   {
      if ((hChn->statusEventIndicators & BAST_G3_CONFIG_STATUS_INDICATOR_FREQ_DRIFT) == 0)
      {
         /* BDBG_MSG(("ch %d freq drift detected", h->channel)); */
         hChn->statusEventIndicators |= BAST_G3_CONFIG_STATUS_INDICATOR_FREQ_DRIFT;
         bSetEvent = true;
      }
   }
   else
      hChn->statusEventIndicators &= ~BAST_G3_CONFIG_STATUS_INDICATOR_FREQ_DRIFT;

   BAST_CHK_RETCODE(BAST_g3_P_GetSnr_isr(h, &snr)); /* snr is in 1/256 dB */

   /* determine threshold 1 and 2 values from mode */
   if (BAST_MODE_IS_DVB(hChn->actualMode))
      idx = hChn->actualMode - BAST_Mode_eDvb_1_2;
   else if (BAST_MODE_IS_DTV(hChn->actualMode))
      idx = 5 + hChn->actualMode - BAST_Mode_eDss_1_2;
   else if (BAST_MODE_IS_DCII(hChn->actualMode))
      idx = 8 + hChn->actualMode - BAST_Mode_eDcii_1_2;
   else if (BAST_MODE_IS_LDPC(hChn->actualMode))
      idx = 16 + (hChn->actualMode - BAST_Mode_eLdpc_Qpsk_1_2);
   else if (BAST_MODE_IS_TURBO(hChn->actualMode))
      idx = 30 + (hChn->actualMode - BAST_Mode_eTurbo_Qpsk_1_2);
   else
   {
      snrThreshold1 = snrThreshold2 = 0;
      goto check_snr_threshold;
   }

   /* BDBG_MSG(("snrThreshold_idx=%d (actualMode=%d)", idx, hChn->actualMode)); */
   snrThreshold1 = BAST_SNR_THRESHOLD1[idx];
   snrThreshold2 = BAST_SNR_THRESHOLD2[idx];

   /* check SNR threshold 1 */
   check_snr_threshold:
   if (snr < snrThreshold1)
   {
      if ((hChn->statusEventIndicators & BAST_G3_CONFIG_STATUS_INDICATOR_THRESHOLD1) == 0)
      {
         /* BDBG_MSG(("ch %d CNR threshold1 detected", h->channel)); */
         hChn->statusEventIndicators |= BAST_G3_CONFIG_STATUS_INDICATOR_THRESHOLD1;
         bSetEvent = true;
      }
   }
   else
      hChn->statusEventIndicators &= ~BAST_G3_CONFIG_STATUS_INDICATOR_THRESHOLD1;

   /* check SNR threshold 2 */
   if (snr < snrThreshold2)
   {
      if ((hChn->statusEventIndicators & BAST_G3_CONFIG_STATUS_INDICATOR_THRESHOLD2) == 0)
      {
         /* BDBG_MSG(("ch %d CNR threshold2 detected", h->channel)); */
         hChn->statusEventIndicators |= BAST_G3_CONFIG_STATUS_INDICATOR_THRESHOLD2;
         bSetEvent = true;
      }
   }
   else
      hChn->statusEventIndicators &= ~BAST_G3_CONFIG_STATUS_INDICATOR_THRESHOLD2;

   /* check for rain fade */
   snr8 = (snr + 128) >> 5; /* snr8=snr in 1/8 dB */
   if (hChn->rainFadeSnrAve == 0)
   {
      hChn->rainFadeSnrAve = snr8;
      hChn->rainFadeSnrMax = snr8;
   }
   else
   {
      /* compute ave snr in window */
      if (snr8 > hChn->rainFadeSnrAve)
         val = snr8 - hChn->rainFadeSnrAve;
      else
         val = hChn->rainFadeSnrAve - snr8;
      val = val / hChn->rainFadeWindow;
      if (snr8 > hChn->rainFadeSnrAve)
         hChn->rainFadeSnrAve += val;
      else
         hChn->rainFadeSnrAve -= val;

      if (snr8 > hChn->rainFadeSnrMax)
         hChn->rainFadeSnrMax = snr8; /* set new max */
      else
         hChn->rainFadeSnrMax -= ((hChn->rainFadeSnrMax - hChn->rainFadeSnrAve) / hChn->rainFadeWindow); /* decay max snr */

      if (snr8 < (hChn->rainFadeSnrMax - (uint32_t)(hChn->rainFadeThreshold)))
      {
         if ((hChn->statusEventIndicators & BAST_G3_CONFIG_STATUS_INDICATOR_RAIN_FADE) == 0)
         {
            hChn->statusEventIndicators |= BAST_G3_CONFIG_STATUS_INDICATOR_RAIN_FADE;
            bSetEvent = true;
         }
      }
      else
         hChn->statusEventIndicators &= ~BAST_G3_CONFIG_STATUS_INDICATOR_RAIN_FADE;
   }
   /* BDBG_MSG(("rain_fade: ave=0x%08X, max=0x%08X, curr=0x%08X", hChn->rainFadeSnrAve, hChn->rainFadeSnrMax, snr8)); */

   if (bSetEvent)
   {
      /* BDBG_MSG(("set status event")); */
      BKNI_SetEvent(hChn->hStatusEvent);
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_ResetStatusIndicators_isr()
******************************************************************************/
BERR_Code BAST_g3_P_ResetStatusIndicators_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   hChn->statusEventIndicators = 0;
   hChn->rainFadeSnrAve = 0;
   hChn->rainFadeSnrMax = 0;
   return BERR_SUCCESS;
}

/******************************************************************************
 BAST_g3_P_ResetStatusIndicators()
******************************************************************************/
BERR_Code BAST_g3_P_ResetStatusIndicators(BAST_ChannelHandle h)
{
   BERR_Code retCode;

   BKNI_EnterCriticalSection();
   retCode = BAST_g3_P_ResetStatusIndicators_isr(h);
   BKNI_LeaveCriticalSection();
   return retCode;
}
#endif


/******************************************************************************
 BAST_g3_P_SdsPowerDownOpll_isrsafe() - power down the OPLL core
******************************************************************************/
void BAST_g3_P_SdsPowerDownOpll_isrsafe(BAST_ChannelHandle h)
{
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_OI_OPLL_PWRDN_RST, 0x7);
}


/******************************************************************************
 BAST_g3_P_SdsPowerUpOpll_isr() - power up the OPLL core
******************************************************************************/
void BAST_g3_P_SdsPowerUpOpll_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_OI_OPLL_PWRDN_RST, ~0x7);
}


/******************************************************************************
 BAST_g3_P_SdsDisableOif_isrsafe() - turn off oif
******************************************************************************/
BERR_Code BAST_g3_P_SdsDisableOif_isrsafe(BAST_ChannelHandle h)
{
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL00, 0x00080001);
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL00, ~0x1);
   BAST_g3_P_SdsPowerDownOpll_isrsafe(h);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_PrepareNewAcquisition() - non-ISR context
******************************************************************************/
BERR_Code BAST_g3_P_PrepareNewAcquisition(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   static const uint32_t script_reset_channel[] =
   {
#ifndef BAST_EXCLUDE_TURBO
#if 1 /* dynamically turn on/off TFEC clock as needed */
      BAST_SCRIPT_OR(BCHP_TFEC_MISC_POST_DIV_CTL, 0x00000004), /* disable tfec clock */
#endif
#endif
#ifndef BAST_EXCLUDE_LDPC
      BAST_SCRIPT_OR(BCHP_AFEC_GLOBAL_CLK_CNTRL, 0x00800000), /* disable afec clock */
#endif
      BAST_SCRIPT_WRITE(BCHP_SDS_FEC_FECTL, 0x01),
      BAST_SCRIPT_WRITE(BCHP_SDS_FEC_FECTL, 0x00),
      BAST_SCRIPT_EXIT
   };

   /* reset last locked status */
   hChn->bLastLocked = false;

   BAST_CHK_RETCODE(BAST_g3_P_DisableChannelInterrupts(h, false, false));
   retCode = BAST_g3_P_HpEnable(h, false);
   if (retCode != BERR_SUCCESS) goto done;

   BAST_CHK_RETCODE(BAST_g3_P_AbortAcq(h));
   BAST_CHK_RETCODE(BAST_g3_P_ResetStatus(h));
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_reset_channel));

#ifndef BAST_EXCLUDE_STATUS_EVENTS
   BAST_CHK_RETCODE(BAST_g3_P_ResetStatusIndicators(h));
#endif

   BAST_g3_P_ClearTraceBuffer(h);
   hChn->bBlindScan = false;
   hChn->initFreqOffset = 0;
   hChn->dftFreqEstimateStatus = 0;
   hChn->bEverStableLock = false;
   hChn->bStableLock = false;
   hChn->signalDetectStatus = 0;
   hChn->peakScanStatus = 0;
   hChn->lockIsrFlag = 0;
   hChn->bVcoAvoidance = false;
   hChn->tunerVcoAvoidanceLpfChange = 0;
#ifndef BAST_EXCLUDE_TURBO
   hChn->turboScanCurrMode = 0;
#endif

   done:
   return retCode;
}


#ifndef BAST_HAS_LEAP
/******************************************************************************
 BAST_g3_P_SdsPowerUp() - power up the sds core
******************************************************************************/
BERR_Code BAST_g3_P_SdsPowerUp(BAST_ChannelHandle h)
{
   BSTD_UNUSED(h);
   /*BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_CG_SPLL_MDIV_CTRL, ~0x00000400);*/  /* TBD power up demod */
   /*BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_CG_SPLL_PWRDN_RST, ~0x00000004);*/   /* TBD power up spll */
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_SdsPowerDown() - power down the sds core
******************************************************************************/
BERR_Code BAST_g3_P_SdsPowerDown(BAST_ChannelHandle h)
{
   BERR_Code retCode;

   /* turn off oif */
   retCode = BAST_g3_P_SdsDisableOif_isrsafe(h);

   /*BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_CG_SPLL_MDIV_CTRL, 0x00000400);*/  /* TBD power down demod - affects refPLL */
   /*BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_CG_SPLL_PWRDN_RST, 0x00000004);*/  /* TBD power down spll - affects refPLL */

   return retCode;
}
#endif


/******************************************************************************
 BAST_g3_P_SetFunctTable_isr()
******************************************************************************/
BERR_Code BAST_g3_P_SetFunctTable_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

#ifndef BAST_EXCLUDE_LDPC
   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
   {
      retCode = BAST_g3_P_LdpcSetFunctTable_isr(h);
      goto done;
   }
#endif
#ifndef BAST_EXCLUDE_TURBO
   if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
   {
      retCode = BAST_g3_P_TurboSetFunctTable_isr(h);
      goto done;
   }
#endif
   if (BAST_MODE_IS_LEGACY_QPSK(hChn->acqParams.mode))
      retCode = BAST_g3_P_QpskSetFunctTable_isr(h);
   else
   {
      BDBG_ERR(("BAST_g3_P_SetFunctTable_isr(): invalid mode (%d)", hChn->acqParams.mode));
      BDBG_ASSERT(0);
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_ResetLockFilter_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_ResetLockFilter_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi;

   hChn->stableLockTimeout = BAST_MODE_IS_LEGACY_QPSK(hChn->actualMode) ? 3000 : 5000;
   hChn->maxStableLockTimeout = 200000;
   hChn->lockFilterRamp = -2;
   hChn->lockFilterIncr = 5000;

   if (hChn->acqParams.symbolRate < 20000000)
   {
      /* scale to symbol rate */
      BMTH_HILO_32TO64_Mul(20000000, hChn->stableLockTimeout, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &(hChn->stableLockTimeout));
      BMTH_HILO_32TO64_Mul(20000000, hChn->maxStableLockTimeout, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &(hChn->maxStableLockTimeout));
      BMTH_HILO_32TO64_Mul(20000000, hChn->lockFilterIncr, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &(hChn->lockFilterIncr));
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_SmartTune_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_SmartTune_isr(BAST_ChannelHandle h, BAST_g3_FUNCT nextFunct)
{
#if BCHP_CHIP==4517
   #define num_smart_tune_special_case_freqs 1
   static const uint32_t special_case_freqs[num_smart_tune_special_case_freqs] = {2829000000UL};
#endif

#ifndef BAST_HAS_WFE
   BERR_Code retCode = BERR_SUCCESS;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)(h->pImpl);
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
   bool bUseDefaultFs = true;
   uint32_t range_low, range_high, x, harmonic, Fs_expected, diff, i, i_lo, i_hi;
   uint32_t lo_vco_khz, lo_div, Fs_khz, x_vco_khz;

   if ((hChn->miscCtl & BAST_G3_CONFIG_MISC_CTL_DISABLE_SMART_TUNE) || (hChn->peakScanStatus & BAST_PEAK_SCAN_STATUS_ENABLED))
   {
      if (hChn->bFsNotDefault)
         goto set_default_fs;
      return nextFunct(h);
   }

   BAST_CHK_RETCODE(BAST_g3_P_TunerGetLoVco_isr(h, &lo_vco_khz, &lo_div));

   x = hDev->searchRange + ((((hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_NYQUIST_20) ? 60 : 68) * hChn->acqParams.symbolRate) / 100);
   x_vco_khz = (x * lo_div) / 1000; /* x_vco_khz = required separation from VCO in KHz */

   Fs_khz = BAST_DEFAULT_SAMPLE_FREQ / 1000;
   i = lo_vco_khz / Fs_khz;
   if (i > 1)
      i_lo = i - 1;
   else
      i_lo = 1;
   i_hi = i + 1;

   BAST_DEBUG_VCO_AVOIDANCE(BDBG_ERR(("SmartTune: VCO=%u KHz, lo_div=%d, Fs_khz=%d", lo_vco_khz, lo_div, Fs_khz)));
   BAST_DEBUG_VCO_AVOIDANCE(BDBG_ERR(("SmartTune: x=%d, x_vco_khz=%d", x, x_vco_khz)));

   for (i = i_lo; i <= i_hi; i++)
   {
      harmonic = Fs_khz * i;
      range_low = harmonic - x_vco_khz;
      range_high = harmonic + x_vco_khz;

      if ((lo_vco_khz >= range_low) && (lo_vco_khz <= range_high))
      {
         BAST_DEBUG_VCO_AVOIDANCE(BDBG_ERR(("Fs smart tune triggered: harmonic=%d, range=(%d,%d)", harmonic, range_low, range_high)));
         bUseDefaultFs  = false;
         break;
      }
   }

#if BCHP_CHIP==4517
   for (i = 0; bUseDefaultFs && (i < num_smart_tune_special_case_freqs); i++)
   {
      /* check for other special case frequencies */
      harmonic = special_case_freqs[i];
      range_low = harmonic - x;
      range_high = harmonic + x;

      if ((hChn->tunerFreq >= range_low) && (hChn->tunerFreq <= range_high))
      {
         BAST_DEBUG_VCO_AVOIDANCE(BDBG_ERR(("Fs smart tune triggered on %u!", harmonic)));
         bUseDefaultFs  = false;
      }
   }
#endif

   BAST_DEBUG_VCO_AVOIDANCE(BDBG_ERR(("bUseDefaultFs=%d, hChn->bFsNotDefault=%d", bUseDefaultFs, hChn->bFsNotDefault)));
   if (bUseDefaultFs)
   {
      set_default_fs:
      Fs_expected = BAST_DEFAULT_SAMPLE_FREQ;
      retCode = BAST_g3_P_SetDefaultSampleFreq_isr(h);
   }
   else
   {
#if BCHP_CHIP==4517
      Fs_expected = 158142857UL;
      retCode = BAST_g3_P_SetSampleFreq_isr(h, 0, 7); /* sds_div=7, Fs=2214/(7*2)=158.1428571 MHz */
#else
      Fs_expected = 182250000UL;
      retCode = BAST_g3_P_SetSampleFreq_isr(h, 81, 6); /* ndiv=81, mdiv=6, Fs=182.25MHz */
#endif
   }

   if (Fs_expected <= hChn->sampleFreq)
      diff = hChn->sampleFreq - Fs_expected;
   else
      diff = Fs_expected - hChn->sampleFreq;
   if (diff > 1000)
   {
      BDBG_ERR(("BAST_g3_P_SmartTune_isr: bad Fs (expected=%u, read=%u)", Fs_expected, hChn->sampleFreq));
      retCode = BAST_ERR_SAMPLE_FREQ;
   }

   done:
   return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 1000, nextFunct);
#else
   /* Fs smart tune doesn't apply to chips with WFE */
   return nextFunct(h);
#endif

}


/******************************************************************************
 BAST_g3_P_TuneAcquire1_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_TuneAcquire1_isr(BAST_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)(h->pImpl);

   BAST_g3_P_UpdateUndersampleMode_isr(h);
#ifndef BAST_EXCLUDE_SPUR_CANCELLER
   BAST_CHK_RETCODE(BAST_g3_P_DisableSpurCanceller_isr(h));
#endif

   if ((hChn->peakScanStatus & BAST_PEAK_SCAN_STATUS_ENABLED) == 0)
   {
      if (BAST_MODE_IS_BLIND_SCAN(hChn->acqParams.mode))
         retCode = BAST_g3_P_BlindScanInit_isr(h);
      else
         retCode = BAST_g3_P_SetFunctTable_isr(h);
      if (retCode != BERR_SUCCESS)
         goto done;
   }

#ifndef BAST_EXCLUDE_EXT_TUNER
   if (hChn->bExternalTuner)
      retCode = BAST_g3_P_Acquire_isr(h);
   else
#endif
   {
      hChn->tunerIfStep = 0;

      /* check for bypass tune option */
      if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_BYPASS_TUNE)
         retCode = BAST_g3_P_Acquire_isr(h);
      else
         retCode = BAST_g3_P_TunerSetFreq_isr(h, BAST_g3_P_Acquire_isr);
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_TuneAcquire0_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g3_P_TuneAcquire0_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)(h->pImpl);
#ifndef BAST_EXCLUDE_TURBO
   uint32_t spinv;
#endif

   BAST_g3_P_ResetLockFilter_isr(h);
   BAST_g3_P_ResetAcquisitionTimer_isr(h);
   hChn->reacqCause = BAST_ReacqCause_eOK;

#ifndef BAST_EXCLUDE_TURBO
   hChn->turboScanState = BAST_TURBO_SCAN_STATE_FIRST_TIME;
   spinv = hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_SPINV_IQ_SCAN;
   if ((spinv == BAST_ACQSETTINGS_SPINV_Q_INV) || (spinv == BAST_ACQSETTINGS_SPINV_I_INV))
      hChn->bTurboSpinv = true;
   else
      hChn->bTurboSpinv = false;
#endif

   return BAST_g3_P_SmartTune_isr(h, BAST_g3_P_TuneAcquire1_isr);
}



/******************************************************************************
 BAST_g3_P_Reacquire_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g3_P_Reacquire_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
#ifndef BAST_HAS_WFE
   bool bRefPllLocked, bMixPllLocked;
#endif

   BAST_CHK_RETCODE(BAST_g3_P_LogTraceBuffer_isr(h, BAST_TraceEvent_eReacquire));
   BAST_DEBUG_ACQ(BDBG_MSG(("BAST_g3_P_Reacquire_isr()")));

   hChn->bStableLock = false;
   BAST_g3_P_IndicateNotLocked_isrsafe(h);

   /* disable lock/lost_lock interrupts */
   BAST_CHK_RETCODE(hChn->disableLockInterrupts(h));
   BAST_CHK_RETCODE(BAST_g3_P_DisableChannelInterrupts_isr(h, false, false));

   if ((hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_REACQ_DISABLE) &&
       (hChn->reacqCtl & BAST_G3_CONFIG_REACQ_CTL_PSOF))
      goto label1;

   BAST_CHK_RETCODE(BAST_g3_P_SdsDisableOif_isrsafe(h));
   BAST_CHK_RETCODE(BAST_g3_P_HpEnable_isr(h, false));
#ifndef BAST_EXCLUDE_STATUS_EVENTS
   BAST_CHK_RETCODE(BAST_g3_P_ResetStatusIndicators_isr(h));
#endif
#ifndef BAST_EXCLUDE_SPUR_CANCELLER
   BAST_CHK_RETCODE(BAST_g3_P_DisableSpurCanceller_isr(h));
#endif

   label1:
   if (hChn->bBlindScan)
   {
#ifndef BAST_EXCLUDE_TURBO
      if ((hChn->acqParams.mode != BAST_Mode_eTurbo_scan) ||
          ((hChn->acqParams.mode == BAST_Mode_eTurbo_scan) && (hChn->turboScanCurrMode == 0)))
#endif
      {
         BAST_CHK_RETCODE(BAST_g3_P_BlindScanSetNextMode_isr(h));
      }
   }
   else
   {
      if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_REACQ_DISABLE)
      {
         hChn->acqState = BAST_AcqState_eIdle;
         BDBG_MSG(("failed to acquire"));
         return retCode;
      }

      if (hChn->acqParams.mode != BAST_Mode_eTurbo_scan)
         hChn->acqCount++;

      if ((hChn->reacqCtl & BAST_G3_CONFIG_REACQ_CTL_RETUNE) && ((hChn->acqCount & 1) == 0))
         hChn->reacqCtl |= BAST_G3_CONFIG_REACQ_CTL_FORCE_RETUNE;
   }

#ifndef BAST_HAS_WFE
   if ((hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_BYPASS_TUNE) == 0)
   {
#ifndef BAST_EXCLUDE_EXT_TUNER
      if (hChn->bExternalTuner == false)
#endif
      {
         BAST_CHK_RETCODE(BAST_g3_P_TunerSetFilter_isr(h, false)); /* set acquisition LPF */
         BAST_CHK_RETCODE(BAST_g3_P_TunerGetLockStatus_isrsafe(h, &bRefPllLocked, &bMixPllLocked));
         if (!bRefPllLocked || !bMixPllLocked)
            hChn->reacqCtl |= BAST_G3_CONFIG_REACQ_CTL_FORCE_RETUNE;
      }
   }
#endif

   if (hChn->reacqCtl & (BAST_G3_CONFIG_REACQ_CTL_RETUNE | BAST_G3_CONFIG_REACQ_CTL_FORCE_RETUNE))
   {
      hChn->tunerIfStep = 0;
      retCode = BAST_g3_P_TunerSetFreq_isr(h, BAST_g3_P_Acquire_isr);
   }
   else
      retCode = BAST_g3_P_Acquire_isr(h);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_OnReacqTimerExpired_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_OnReacqTimerExpired_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BAST_DEBUG_ACQ(BDBG_MSG(("BAST_g3_P_OnReacqTimerExpired_isr()")));

   hChn->bReacqTimerExpired = true;

   /* dont reacquire if demod is currently locked */
   if (hChn->acqState == BAST_AcqState_eWaitForStableLock)
      return BERR_SUCCESS;

   hChn->reacqCause = BAST_ReacqCause_eFecNotStableLock;
   return BAST_g3_P_Reacquire_isr(h);
}


/******************************************************************************
 BAST_g3_P_StartReacqTimer_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_StartReacqTimer_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)(h->pImpl);
   uint32_t reacquisition_timeout, P_hi, P_lo, Q_hi, min_timeout;

   /* compute the reacquisition timeout */
   if (BAST_MODE_IS_TURBO(hChn->actualMode))
      min_timeout = 500000;
   else
      min_timeout = 300000;

   if (hChn->acqParams.symbolRate >= 20000000)
      reacquisition_timeout = min_timeout;
   else
   {
      BMTH_HILO_32TO64_Mul(20000000, min_timeout, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &reacquisition_timeout);
   }

   hChn->bReacqTimerExpired = false;
   BAST_DEBUG_ACQ(BDBG_MSG(("setting reacq timer to %d usecs", reacquisition_timeout)));
   return BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eReacqTimer, reacquisition_timeout, BAST_g3_P_OnReacqTimerExpired_isr);
}


/******************************************************************************
 BAST_g3_P_StartTracking_isr()
******************************************************************************/
BERR_Code BAST_g3_P_StartTracking_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)(h->pImpl);
   BERR_Code retCode;

   BAST_CHK_RETCODE(BAST_g3_P_LogTraceBuffer_isr(h, BAST_TraceEvent_eStartTracking));
   hChn->trace[BAST_TraceEvent_eInitialLock] = 0;
   hChn->trace[BAST_TraceEvent_eStableLock] = 0;
   BAST_DEBUG_ACQ(BDBG_MSG(("BAST_g3_P_StartTracking_isr()")));

   BAST_CHK_RETCODE(hChn->checkModeFunct(h));

   if (hChn->reacqCtl & BAST_G3_CONFIG_REACQ_CTL_FORCE)
   {
      BDBG_MSG(("BAST_g3_P_StartTracking_isr(): invalid mode"));
      hChn->reacqCause = BAST_ReacqCause_eInvalidMode;
      return BAST_g3_P_Reacquire_isr(h);
   }

   BAST_CHK_RETCODE(BAST_g3_P_StartBert_isr(h));

   hChn->acqState = BAST_AcqState_eWaitForInitialLock;

   /* enable lock/lost_lock interrupts */
   hChn->lockIsrFlag = 0;
   BAST_CHK_RETCODE(hChn->enableLockInterrupts(h));

   BAST_CHK_RETCODE(BAST_g3_P_StartReacqTimer_isr(h));

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_LeakPliToFli_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_LeakPliToFli_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   int32_t pli, fli, slval2, slval3, x;
   uint32_t P_hi, P_lo, val;

   if (BAST_MODE_IS_LDPC(hChn->actualMode))
      goto done;

   /* turbo and legacy QPSK */
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_PLI, (uint32_t*)&pli);
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_FLI, (uint32_t*)&fli);

   if (BAST_MODE_IS_TURBO(hChn->actualMode))
   {
      slval3 = 16384;
      slval2 = 128;
   }
   else if (BAST_MODE_IS_DCII(hChn->actualMode))
   {
      slval3 = 272144;
      slval2 = 512;
   }
   else
   {
      slval3 = 16384;
      slval2 = 128;
   }

   hChn->freqTransferInt += (pli / slval3);
   slval2 = (pli / slval2) + hChn->freqTransferInt;

   if (slval2 < 0)
      val = -slval2;
   else
      val = slval2;

   BMTH_HILO_32TO64_Mul(val, hChn->acqParams.symbolRate, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &P_hi, &P_lo);
   if (slval2 >= 0)
      x = (int32_t)P_lo;
   else
      x = (int32_t)-P_lo;

   fli += x;

   pli -= slval2;

   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLI, (uint32_t*)&pli);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLI, (uint32_t*)&fli);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_OnMonitorLock_isr() - scheduled every 100 msecs
******************************************************************************/
static BERR_Code BAST_g3_P_OnMonitorLock_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)(h->pImpl);
   BERR_Code retCode;

   if (hChn->timeSinceStableLock == 4)
   {
#ifndef BAST_EXCLUDE_SPUR_CANCELLER
      BAST_CHK_RETCODE(BAST_g3_P_ResetCWC_isr(h));
#endif

#if 0 /* no longer needed for new SNORE */
      /* reset SNORE after 300 msecs */
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_SNR_SNORECTL, ~0x80);
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_SNR_SNORECTL, 0x80);
#endif
   }

   if (hChn->onMonitorLockFunct)
      hChn->onMonitorLockFunct(h);
   if (hChn->reacqCtl & BAST_G3_CONFIG_REACQ_CTL_FORCE)
      goto force_reacquire;

   if (hChn->reacqCtl & BAST_G3_CONFIG_REACQ_CTL_FREQ_DRIFT)
   {
      if (BAST_g3_P_IsCarrierOffsetOutOfRange_isr(h))
      {
         hChn->reacqCtl |= BAST_G3_CONFIG_REACQ_CTL_FORCE_RETUNE;

         force_reacquire:
         hChn->reacqCause = BAST_ReacqCause_eCarrierOffsetOutOfRange;
         return BAST_g3_P_Reacquire_isr(h);
      }
   }

#ifdef BAST_HAS_WFE
   if ((hChn->timeSinceStableLock % 4) == 0)
   {
      BAST_CHK_RETCODE(BAST_g3_P_UpdateNotch_isr(h));
   }
#endif

   BAST_CHK_RETCODE(BAST_g3_P_LeakPliToFli_isr(h));

   /* DCO ramping fix */
   /* TBD */

#ifndef BAST_EXCLUDE_STATUS_EVENTS
   BAST_g3_P_CheckStatusIndicators_isr(h);
#endif

   hChn->timeSinceStableLock++;
   if (hChn->bMonitorLock)
      retCode = BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eMonitorLockTimer, 100000, BAST_g3_P_OnMonitorLock_isr);

   done:
   return retCode;
}



/******************************************************************************
 BAST_g3_P_OnStableLock1_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_OnStableLock1_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;

   if (hChn->bMonitorLock)
   {
      BAST_CHK_RETCODE(BAST_g3_P_OnMonitorLock_isr(h));
      if (hChn->reacqCtl & BAST_G3_CONFIG_REACQ_CTL_FORCE)
         retCode = BAST_g3_P_Reacquire_isr(h);
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_OnStableLock_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_OnStableLock_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)(h->pImpl);
   BERR_Code retCode;

   /* reset SNORE */
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_SNR_SNORECTL, ~0x80);
   BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_SNR_SNORECTL, 0x80);

   BAST_CHK_RETCODE(BAST_g3_P_DisableTimer_isr(h, BAST_TimerSelect_eReacqTimer));
   BAST_CHK_RETCODE(BAST_g3_P_LogTraceBuffer_isr(h, BAST_TraceEvent_eStableLock));
   BAST_DEBUG_ACQ(BDBG_MSG(("BAST_g3_P_OnStableLock_isr(): t=%d", hChn->acqTime)));
   BAST_CHK_RETCODE(BAST_g3_P_ResetLockFilter_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_GetCarrierError_isrsafe(h, &(hChn->initFreqOffset)));
   hChn->relockCount++;

#if 0
   if (hChn->bPlcTracking == false)
      BAST_g3_P_ConfigPlc_isr(h, false); /* set tracking PLC */
#endif

   if (hChn->onStableLockFunct)
   {
      BAST_CHK_RETCODE(hChn->onStableLockFunct(h));
      if (hChn->reacqCtl & BAST_G3_CONFIG_REACQ_CTL_FORCE)
         return BAST_g3_P_Reacquire_isr(h);
   }

   hChn->acqState = BAST_AcqState_eMonitorLock;
   BAST_g3_P_IndicateLocked_isrsafe(h);
   hChn->bEverStableLock = true;
   hChn->bStableLock = true;
   hChn->freqTransferInt = 0;
   hChn->timeSinceStableLock = 0;
   hChn->count1 = 0;

#ifndef BAST_EXCLUDE_SPUR_CANCELLER
   retCode = BAST_g3_P_InitCWC_isr(h, BAST_g3_P_OnStableLock1_isr);
#else
   retCode = BAST_g3_P_OnStableLock1_isr(h);
#endif
   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_Lock_isr() - callback routine for not_locked->locked transition
******************************************************************************/
void BAST_g3_P_Lock_isr(void *p, int int_id)
{
   BERR_Code retCode;
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   bool bLocked;

   BAST_g3_P_IncrementInterruptCounter_isr(h, int_id);
   BAST_DEBUG_ACQ(BDBG_MSG(("BAST_g3_P_Lock_isr(%d), lockIsrFlag=%d", int_id, hChn->lockIsrFlag)));

   if (hChn->getLockStatusFunct)
   {
      hChn->getLockStatusFunct(h, &bLocked);
      if (!bLocked)
      {
         BDBG_MSG(("false lock detected"));
         BAST_g3_P_NotLock_isr(p, BAST_g3_IntID_eNotLock);
         return;
      }
   }

   if (hChn->lockIsrFlag == 1)
   {
      BDBG_MSG(("ignoring lock isr"));
      return;
   }
   hChn->lockIsrFlag = 1;

   BAST_g3_P_GetAcquisitionTimerValue_isr(h, &(hChn->acqTime));

   if (hChn->trace[BAST_TraceEvent_eInitialLock] == 0)
   {
      retCode = BAST_g3_P_LogTraceBuffer_isr(h, BAST_TraceEvent_eInitialLock);
      if (retCode != BERR_SUCCESS)
      {
         BDBG_WRN(("BAST_g3_P_Lock_isr(): BAST_g3_P_LogTraceBuffer_isr() error 0x%X", retCode));
      }
   }

   if (hChn->reacqCtl & BAST_G3_CONFIG_REACQ_CTL_FREQ_DRIFT)
   {
      if (BAST_g3_P_IsCarrierOffsetOutOfRange_isr(h))
      {
         hChn->reacqCtl |= (BAST_G3_CONFIG_REACQ_CTL_FORCE_RETUNE | BAST_G3_CONFIG_REACQ_CTL_FORCE);
         hChn->reacqCause = BAST_ReacqCause_eCarrierOffsetOutOfRange;
         goto reacquire;
      }
   }

   if (hChn->onLockFunct)
      hChn->onLockFunct(h);

   reacquire:
   if (hChn->reacqCtl & BAST_G3_CONFIG_REACQ_CTL_FORCE)
      BAST_g3_P_Reacquire_isr(h);
   else
   {
      hChn->acqState = BAST_AcqState_eWaitForStableLock;
      BAST_DEBUG_ACQ(BDBG_MSG(("starting stable lock timer (%d usecs)", hChn->stableLockTimeout)));
      BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eStableLockTimer, hChn->stableLockTimeout, BAST_g3_P_OnStableLock_isr);
   }
}


/******************************************************************************
 BAST_g3_P_NotLock_isr() - callback routine for locked->not_locked transition
******************************************************************************/
void BAST_g3_P_NotLock_isr(void *p, int int_id)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   bool bStartReacqTimer;

   BAST_g3_P_IncrementInterruptCounter_isr(h, int_id);
   BAST_DEBUG_ACQ(BDBG_MSG(("BAST_g3_P_NotLock_isr(%d), lockIsrFlag=%d", int_id, hChn->lockIsrFlag)));

   hChn->acqTime = 0;
   if (hChn->lockIsrFlag == 0)
   {
      BDBG_MSG(("ignoring not lock isr"));
      return;
   }
   hChn->lockIsrFlag = 0;

   BAST_g3_P_DisableTimer_isr(h, BAST_TimerSelect_eStableLockTimer);

   BAST_g3_P_IndicateNotLocked_isrsafe(h);

   if ((hChn->acqState == BAST_AcqState_eMonitorLock) ||
       (BAST_g3_P_IsTimerRunning_isrsafe(h, BAST_TimerSelect_eReacqTimer) == false))
      bStartReacqTimer = true;
   else
      bStartReacqTimer = false;

   hChn->acqState = BAST_AcqState_eWaitForInitialLock;

   /* extend stable lock timeout */
   if ((hChn->acqCount > 0) || hChn->bEverStableLock)
   {
      hChn->stableLockTimeout += (((hChn->lockFilterRamp++ > 0) ? hChn->lockFilterRamp : 1) * hChn->lockFilterIncr);
      if (hChn->stableLockTimeout > hChn->maxStableLockTimeout)
         hChn->stableLockTimeout = hChn->maxStableLockTimeout;
   }

   if (hChn->bReacqTimerExpired)
   {
      hChn->reacqCtl |= BAST_G3_CONFIG_REACQ_CTL_FORCE;
      hChn->reacqCause = BAST_ReacqCause_eFecNotStableLock;
   }

   if (hChn->onLostLockFunct)
      hChn->onLostLockFunct(h);

   if ((hChn->reacqCtl & BAST_G3_CONFIG_REACQ_CTL_FORCE) || (hChn->relockCount > 8))
   {
      BAST_g3_P_Reacquire_isr(h);
      return;
   }

   if (hChn->bStableLock && (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_BERT_ENABLE))
   {
      /* reset the bert if bert lost lock */
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_BERT_BERCTL, 1);
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_BERT_BERCTL, ~1);  /* clear reset */
   }

   hChn->bStableLock = false;

   if (bStartReacqTimer)
      BAST_g3_P_StartReacqTimer_isr(h);
}


/******************************************************************************
 BAST_g3_P_WriteVerifyRegister() - callback routine for locked->not_locked transition
******************************************************************************/
BERR_Code BAST_g3_P_WriteVerifyRegister(BAST_ChannelHandle h, uint32_t reg, uint32_t *val)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t retry, val2;

   for (retry = 0; retry < 100; retry++)
   {
      BAST_CHK_RETCODE(BAST_g3_P_WriteRegister_isrsafe(h, reg, val));
      BAST_CHK_RETCODE(BAST_g3_P_ReadRegister_isrsafe(h, reg, &val2));
      if (*val == val2)
         break;
   }

   if (retry >= 100)
      retCode = BAST_ERR_IOMB_XFER;

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_SetMixctl_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_SetMixctl_isr(BAST_ChannelHandle h)
{
#if 0
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
#endif
   uint32_t val;

#if 0
   if (hChn->tunerCtl & BAST_G3_CONFIG_TUNER_CTL_PRESERVE_COMMANDED_FREQ)
     val = 0x01;
   else
#endif
     val = 0x03;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_MIXCTL, &val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_NonLegacyModeAcquireInit_isr()
******************************************************************************/
BERR_Code BAST_g3_P_NonLegacyModeAcquireInit_isr(BAST_ChannelHandle h)
{
   static const uint32_t script_nonlegacy_0[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_SNR_SNRCTL, 0x03),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_PLTD, 0x28000000),
      BAST_SCRIPT_OR(BCHP_SDS_CL_CLCTL1, 0x0000001C),
#ifndef BAST_HAS_WFE
      BAST_SCRIPT_WRITE(BCHP_SDS_AGC_ABW, 0x0B0B0000),
#endif
      BAST_SCRIPT_AND_OR(BCHP_SDS_EQ_EQFFECTL, 0x00FF00FF, 0x03000600), /* EQMU=0x03,EQFFE2=0x06 */
      BAST_SCRIPT_AND_OR(BCHP_SDS_EQ_EQMISCCTL, 0x000000FF, 0x00140400), /* EQMISC=0x14, EQBLND=0x04 */
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_EQCFAD, 0x4C),   /* initialization of EQ tap */
      BAST_SCRIPT_WRITE(BCHP_SDS_FEC_FERR, 0x00000000),
      BAST_SCRIPT_AND_OR(BCHP_SDS_CL_CLFBCTL, 0xFFFFFF00, 0x02), /* fwd loop frz, backward loop frz */
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_nonlegacy_2[] =
   {
      BAST_SCRIPT_AND_OR(BCHP_SDS_CL_CLCTL1, 0xFFFF00FF, 0x0000DB00),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLLC, 0x20000100),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLIC, 0x40000200),
#if 1
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLLC1, 0x20000100),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLIC1, 0x40000200),
#else
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLLC1, 0x01000100), /* orig */
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLIC1, 0x01000000), /* orig */
#endif
      BAST_SCRIPT_EXIT
   };

#if 0
   static const uint32_t script_nonlegacy_3[] =
   {
      BAST_SCRIPT_AND_OR(BCHP_SDS_CL_CLCTL1, 0xFFFF00FF, 0x0000DE00),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLLC, 0x1F000100),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLIC, 0x40000200),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLLC1, 0x20000100),
      BAST_SCRIPT_WRITE(BCHP_SDS_CL_FLIC1, 0xC0000200),
      BAST_SCRIPT_EXIT
   };
#endif

#if 0
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
#endif
   BERR_Code retCode;

   BAST_CHK_RETCODE(BAST_g3_P_SetFfeMainTap_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_SetMixctl_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_nonlegacy_0));

#if 0
   if (hChn->tunerCtl & BAST_G3_CONFIG_TUNER_CTL_PRESERVE_COMMANDED_FREQ)
   {
      BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_nonlegacy_3));
   }
   else
#endif
   {
      BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_nonlegacy_2));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_GetRegisterWriteWaitTime_isrsafe()
******************************************************************************/
void BAST_g3_P_GetRegisterWriteWaitTime_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t *wait_time)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   *wait_time = 0;
   if (hChn->acqParams.symbolRate < 8000000)
   {
#if 1
      /* VIT, FEC, CL, and EQ are in the baud clock domain */
      if (((reg >= BCHP_SDS_VIT_VTCTL) && (reg <= BCHP_SDS_FEC_FRSV)) ||
         ((reg >= BCHP_SDS_CL_CLCTL1) && (reg <= BCHP_SDS_EQ_MGAINA)))
#else
      if (((reg >= BCHP_SDS_CL_CLCTL1) && (reg <= BCHP_SDS_FEC_FRSV)) ||
          ((reg >= BCHP_SDS_SNR_SNRCTL) && (reg <= BCHP_SDS_SNR_SNORECTL)))
#endif
      {
        if (hChn->acqParams.symbolRate > 2000000)
          *wait_time = 1;
        else
          *wait_time = 2;
      }
   }
}



/******************************************************************************
 BAST_g3_P_GetNumDecimatingFilters_isr()
******************************************************************************/
uint32_t BAST_g3_P_GetNumDecimatingFilters_isr(BAST_ChannelHandle h)
{
   uint32_t filtctl, i, n = 0;

   n = 0;
   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL, &filtctl);
   for (i = 0; i < 5; i++)
   {
      if ((filtctl & (1 << i)) == 0)
         n++;
      else
         break;
   }
   if ((n == 5) && ((filtctl & 0x80) == 0))
      n++;
   return n;
}
