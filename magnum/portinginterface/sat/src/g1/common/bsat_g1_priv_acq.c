/******************************************************************************
* Copyright (C) 2018 Broadcom.
* The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to
* the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied),
* right to use, or waiver of any kind with respect to the Software, and
* Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
* THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
* IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization,
* constitutes the valuable trade secrets of Broadcom, and you shall use all
* reasonable efforts to protect the confidentiality thereof, and to use this
* information only in connection with your use of Broadcom integrated circuit
* products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
* "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
* OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
* RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
* IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
* A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
* ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
* THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
* OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
* INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
* RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
* HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
* EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
* WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
* FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* Module Description:
*
*****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bmth.h"
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"


BDBG_MODULE(bsat_g1_priv_acq);

#define BSAT_DEBUG_ACQ(x) /* x */
#define BSAT_DEBUG_OI(x) /* x */

#define BSAT_G1_LOCK_FILTER_INCR 1250 /* orig: 5000 */


/* local functions */
static BERR_Code BSAT_g1_P_Acquire1_isr(BSAT_ChannelHandle h);
static BERR_Code BSAT_g1_P_Acquire2_isr(BSAT_ChannelHandle h);
static BERR_Code BSAT_g1_P_Acquire3_isr(BSAT_ChannelHandle h);
static BERR_Code BSAT_g1_P_Acquire4_isr(BSAT_ChannelHandle h);
static BERR_Code BSAT_g1_P_OnStableLock_isr(BSAT_ChannelHandle h);
static BERR_Code BSAT_g1_P_OnStableLock1_isr(BSAT_ChannelHandle h);
static BERR_Code BSAT_g1_P_OnMonitorLock_isr(BSAT_ChannelHandle h);


/* extern bool bEnableDebugLog; */

/******************************************************************************
 BSAT_g1_P_GetModeFunct_isrsafe()
******************************************************************************/
const BSAT_g1_ModeFunct* BSAT_g1_P_GetModeFunct_isrsafe(BSAT_AcqType t)
{
   static const BSAT_g1_ModeFunct BSAT_ModeFunct[3] =
   {
      {
         BSAT_g1_P_QpskAcquire_isr,
         BSAT_g1_P_QpskIsSpinv_isrsafe,
         BSAT_g1_P_QpskIsValidMode_isr,
         BSAT_g1_P_QpskOnLock_isr,
         BSAT_g1_P_QpskOnLostLock_isr,
         BSAT_g1_P_QpskOnStableLock_isr,
         BSAT_g1_P_QpskEnableLockInterrupts_isr,
         NULL,
         BSAT_g1_P_QpskIsLocked_isr,
         BSAT_g1_P_Reacquire_isr
      },
      {
         BSAT_g1_P_AfecAcquire_isr,
         BSAT_g1_P_HpIsSpinv_isrsafe,
         BSAT_g1_P_AfecIsValidMode_isr,
         BSAT_g1_P_AfecOnLock_isr,
         BSAT_g1_P_AfecOnLostLock_isr,
         BSAT_g1_P_AfecOnStableLock_isr,
         BSAT_g1_P_AfecEnableLockInterrupts_isr,
         BSAT_g1_P_AfecOnMonitorLock_isr,
         BSAT_g1_P_AfecIsLocked_isr,
         BSAT_g1_P_AfecReacquire_isr,
      },
#ifndef BSAT_EXCLUDE_TFEC
      {
         BSAT_g1_P_TfecAcquire_isr,
         BSAT_g1_P_HpIsSpinv_isrsafe,
         BSAT_g1_P_TfecIsValidMode_isr,
         BSAT_g1_P_TfecOnLock_isr,
         BSAT_g1_P_TfecOnLostLock_isr,
         BSAT_g1_P_TfecOnStableLock_isr,
         BSAT_g1_P_TfecEnableLockInterrupts_isr,
         BSAT_g1_P_TfecOnMonitorLock_isr,
         NULL,
#ifdef BSAT_HAS_DUAL_TFEC
         BSAT_g1_P_TfecReacquire_isr,
#else
         BSAT_g1_P_Reacquire_isr,
#endif
      }
#else
      {
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL
      }
#endif
   };

   return &BSAT_ModeFunct[t];
}


/******************************************************************************
 BSAT_g1_P_InitHandle() - default initial settings for BSAT device handle
******************************************************************************/
BERR_Code BSAT_g1_P_InitHandle(BSAT_Handle h)
{
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle *)(h->pImpl);

   hDev->nNotch = 0;
   hDev->acqDoneThreshold = 3;
   hDev->networkSpec = BSAT_NetworkSpec_eDefault;
   hDev->pSaBuf = 0;
   hDev->afecRampSettings = 0x00011170;
   hDev->afecRampChanMask = 0;
   hDev->afecRampLowThreshChanMask = 0;
   return BSAT_g1_P_InitHandleExt(h);
}


/******************************************************************************
 BSAT_g1_P_InitChannelHandle() - default initial settings for BSAT channel
                                 device handle
******************************************************************************/
BERR_Code BSAT_g1_P_InitChannelHandle(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   hChn->bLocked = false;
   hChn->bLastLocked = false;
   hChn->bEverStableLock = false;
   hChn->bStableLock = false;
   hChn->bBlindScan = false;
   hChn->bAcqDone = false;
   hChn->bPlcTracking = false;
   hChn->bUndersample = false;
   hChn->bForceReacq = false;
   hChn->bAbortAcq = true;
   hChn->acqState = BSAT_AcqState_eIdle;
   hChn->mpegErrorCount = 0;
   hChn->mpegFrameCount = 0;
   hChn->baudTimerIsr = NULL;
   hChn->berTimerIsr = NULL;
   hChn->gen1TimerIsr = NULL;
   hChn->gen2TimerIsr = NULL;
   hChn->gen3TimerIsr = NULL;
   hChn->searchRange = 5000000;
   hChn->bertSettings.bInvert = true;
   hChn->bertSettings.mode = BSAT_BertMode_ePRBS23;
   hChn->xportSettings.bSerial = true;
   hChn->xportSettings.bClkInv = true;
   hChn->xportSettings.bClkSup = true;
   hChn->xportSettings.bVldInv = false;
   hChn->xportSettings.bSyncInv = false;
   hChn->xportSettings.bErrInv = false;
   hChn->xportSettings.bXbert = false;
   hChn->xportSettings.bTei = true;
   hChn->xportSettings.bDelay = false;
   hChn->xportSettings.bSync1 = false;
   hChn->xportSettings.bHead4 = false;
   hChn->xportSettings.bDelHeader = false;
   hChn->xportSettings.bOpllBypass = false;
   hChn->xportSettings.bDataInv = false;
   hChn->xportSettings.bchMpegErrorMode = BSAT_BchMpegErrorMode_eBchAndCrc8;
   hChn->sigNotSettings.rainFadeThreshold = 3<<3; /* 3dB*2^3 */
   hChn->sigNotSettings.rainFadeWindow = 3000; /* 300 secs */
   hChn->sigNotSettings.freqDriftThreshold = 2000000;
   hChn->qpskSettings.bRsDisable = false;
   hChn->qpskSettings.dtvScanModes = BSAT_SCAN_MODE_DTV_ALL;
   hChn->qpskSettings.dvbsScanModes = BSAT_SCAN_MODE_DVBS_ALL;
   hChn->qpskSettings.dciiScanModes = BSAT_SCAN_MODE_DCII_ALL;
   hChn->qpskSettings.stuffBytes = 0;
   hChn->dvbs2Settings.scanModes = BSAT_SCAN_MODE_DVBS2_ALL;
   hChn->dvbs2Settings.ctl = BSAT_DVBS2_CTL_DISABLE_PSL | BSAT_DVBS2_CTL_SEL_UPL;
   hChn->turboSettings.scanModes = BSAT_SCAN_MODE_TURBO_ALL;
   hChn->turboSettings.ctl = 0;
   hChn->turboSettings.tzsyOverride = 0x0420040F;
   hChn->turboSettings.flbwOverride = 25000;
   hChn->scramblingSeq.xseed = 0x100;
   hChn->scramblingSeq.plhdrscr1 = 0;
   hChn->scramblingSeq.plhdrscr2 = 0;
   hChn->scramblingSeq.plhdrscr3 = 0;
#ifndef BSAT_HAS_DVBS2X
   hChn->bAfecFlushFailed = false;
#else
   hChn->dvbs2xSettings.scanModes = BSAT_SCAN_MODE_DVBS2X_ALL;
#endif
   hChn->acqSettings.mode = BSAT_Mode_eUnknown;
   hChn->acqSettings.symbolRate = 20000000;
   hChn->acqSettings.options = BSAT_ACQ_DEFAULT;
   hChn->acqSettings.adcSelect = 0;
   hChn->acqSettings.freq = 1000000000;
   hChn->outputBitrate = 0;
   hChn->sampleFreq = BSAT_DEFAULT_SAMPLE_FREQ;
   hChn->reacqCount = 0;
   hChn->reacqCause = BSAT_ReacqCause_eOK;
   hChn->aciBandwidth = 0;
   hChn->bOverrideAciBandwidth = false;
   hChn->actualTunerFreq = 0;
   hChn->relockCount = 0;
   hChn->acqType = 0;
   hChn->fecFreq = 0;
   hChn->configParam[BSAT_g1_CONFIG_PLC_CTL] = 0;
   hChn->configParam[BSAT_g1_CONFIG_ACQ_TIME] = 0;
   hChn->configParam[BSAT_g1_CONFIG_VLC_GAIN] = 0;
   hChn->configParam[BSAT_g1_CONFIG_HP_CTL] = 0;
   hChn->configParam[BSAT_g1_CONFIG_EQ_CTL] = 0;
   hChn->configParam[BSAT_g1_CONFIG_ACQ_DAFE_CTL] = 0;
   hChn->configParam[BSAT_g1_CONFIG_TRK_DAFE_CTL] = 0;
   hChn->configParam[BSAT_g1_CONFIG_ACM_DEBUG] = 0;
   hChn->notchState = -1;
   BKNI_Memset((void*)&(hChn->miscSettings), 0, sizeof(BSAT_ExtAcqSettings));
   BKNI_Memset((void*)&(hChn->saSettings), 0, sizeof(BSAT_ScanSpectrumSettings));
   BKNI_Memset((void*)&(hChn->saStatus), 0, sizeof(BSAT_SpectrumStatus));
   hChn->saStatus.status = BERR_NOT_INITIALIZED;
#ifdef BSAT_HAS_ACM
   BKNI_Memset((void*)&(hChn->acmSettings), 0, sizeof(BSAT_AcmSettings));
   hChn->acmSettings.softDecisions.captureMode = BSAT_AcmSoftDecisionCapture_eAll;
   hChn->acmSettings.snr.filterMode = BSAT_AcmSnrFilter_eNone;
   hChn->acmSettings.bert.bEnable = false;
   BKNI_Memset((void*)&(hChn->mpegErrorCountMs), 0, 8*sizeof(uint32_t));
   BKNI_Memset((void*)&(hChn->mpegFrameCountMs), 0, 8*sizeof(uint32_t));
#endif
   hChn->miscSettings.blindScanModes = BSAT_BLIND_SCAN_MODE_DVBS | BSAT_BLIND_SCAN_MODE_DVBS2;
   hChn->miscSettings.maxReacqs = 0;
   BSAT_g1_P_ClearTraceBuffer(h);
   BSAT_g1_P_ResetSignalNotification_isrsafe(h);
   BSAT_g1_P_ResetSignalDetectStatus(h);
   BSAT_g1_P_ResetSymbolRateScanStatus(h);
   BSAT_g1_P_ResetToneDetectStatus(h);
   return BSAT_g1_P_InitChannelHandleExt(h);
}


/******************************************************************************
 BSAT_g1_P_InitChannel()
******************************************************************************/
BERR_Code BSAT_g1_P_InitChannel(BSAT_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle *)(h->pDevice->pImpl);

   /* BDBG_MSG(("BSAT_g1_P_InitChannel(%d)", h->channel)); */

   if (hDev->sdsRevId == 0)
      hDev->sdsRevId = (uint8_t)(BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_REVID) & 0xFF);

   BSAT_g1_P_InitChannelHandle(h);
   BSAT_g1_P_IndicateNotLocked_isrsafe(h);

   if (hChn->bHasAfec)
   {
      BKNI_EnterCriticalSection();
      BSAT_g1_P_AfecPowerUp_isr(h);
      BKNI_LeaveCriticalSection();
      BSAT_g1_P_AfecPowerDown_isrsafe(h);
   }

#ifndef BSAT_EXCLUDE_TFEC
   if (hChn->bHasTfec)
   {
#ifdef BSAT_HAS_DUAL_TFEC
      if (BSAT_g1_P_TfecIsOtherChannelBusy_isrsafe(h) == false)
#endif
      {
         BKNI_EnterCriticalSection();
         BSAT_g1_P_TfecPowerUp_isr(h);
         BKNI_LeaveCriticalSection();
         BSAT_g1_P_TfecPowerDown_isrsafe(h);
      }
   }
#endif

   BSAT_g1_P_PowerDownOpll_isrsafe(h);

   /* initialize clockgen and sds */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CG_CGDIV01, 0x005D0119); /* 375KHz mi2c clk */
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_MISC_MISCTL, ~0x00000008); /* power down ring oscillators for noise reduction */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BFOS, 0x03A0CC00); /* assume Fb=20, Fs=145.125 */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC_TPDIR, 0xFF4B008F);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_ADCPCTL, 0x000F0001);
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_FE_ADCPCTL, ~0x1); /* clear reset */
#ifdef BCHP_SDS_FE_DSTGCTL
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_DSTGCTL, 0x00000001); /* bypass destagger */
#endif
#ifndef BSAT_HAS_DVBS2X
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_IQCTL, 0x00008284); /* bypass IQ phase/amplitude imbalance correction */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_DCOCTL, 0x00020311); /* bypass DCO */
#endif

   /* initialize tuner */
   BSAT_g1_P_TunerInit(h, NULL);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_IndicateLocked_isrsafe()
******************************************************************************/
void BSAT_g1_P_IndicateLocked_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   hChn->bLocked = true;
   if (hChn->bLastLocked == false)
   {
      BDBG_MSG(("*** chan%d locked", h->channel));
      BKNI_SetEvent(hChn->hLockChangeEvent);
   }

   hChn->bLastLocked = true;
}


/******************************************************************************
 BSAT_g1_P_IndicateNotLocked_isrsafe()
******************************************************************************/
void BSAT_g1_P_IndicateNotLocked_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   hChn->bLocked = false;
   if (hChn->bLastLocked)
   {
      BDBG_MSG(("*** chan%d not locked", h->channel));
      if (hChn->bAbortAcq == false)
         BKNI_SetEvent(hChn->hLockChangeEvent);
   }

   hChn->bLastLocked = false;
}


/******************************************************************************
 BSAT_g1_P_IndicateAcqDone_isr()
******************************************************************************/
void BSAT_g1_P_IndicateAcqDone_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (!hChn->bAcqDone)
   {
      hChn->bAcqDone = true;
      BKNI_SetEvent(hChn->hAcqDoneEvent);
   }
}


/******************************************************************************
 BSAT_g1_P_IncrementReacqCount_isr()
******************************************************************************/
void BSAT_g1_P_IncrementReacqCount_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle*)h->pDevice->pImpl;

   hChn->reacqCount++;
   if (hChn->reacqCount > hDev->acqDoneThreshold)
      BSAT_g1_P_IndicateAcqDone_isr(h);
}


/******************************************************************************
 BSAT_g1_P_IsCarrierOffsetOutOfRange_isr()
******************************************************************************/
static bool BSAT_g1_P_IsCarrierOffsetOutOfRange_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   int32_t carrierError;
   uint32_t val, range;
   BERR_Code retCode;
   bool bOutOfRange = false;

   BSAT_CHK_RETCODE(BSAT_g1_P_GetCarrierError_isrsafe(h, &carrierError));

   if (carrierError < 0)
      val = -carrierError;
   else
      val = carrierError;

   range = (hChn->searchRange * 6) / 5;
   if (val > range)
   {
      bOutOfRange = true;
      BDBG_WRN(("carrier offset out of range (%d Hz)", carrierError));
   }

   done:
   return bOutOfRange;
}


/******************************************************************************
 BSAT_g1_P_LeakPliToFli_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_LeakPliToFli_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   int32_t pli, fli, slval2, slval3, x;
   uint32_t P_hi, P_lo, val;

   if (BSAT_MODE_IS_DVBS2(hChn->actualMode) || BSAT_MODE_IS_DVBS2X(hChn->actualMode))
      goto done;

   /* turbo and legacy QPSK */
   pli = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_PLI);
   fli = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_FLI);

   if (BSAT_MODE_IS_TURBO(hChn->actualMode))
   {
      slval3 = 16384;
      slval2 = 128;
   }
   else if (BSAT_MODE_IS_DCII(hChn->actualMode))
   {
      slval3 = 272144;
      slval2 = 512;
   }
   else
   {
      slval3 = 16384;
      slval2 = 128;
   }

   /* updated by Xiaofen: */
   slval2 = slval2 >> 5;
   slval3 = slval3 >> 5;

   hChn->freqTransferInt += (pli / slval3);
   slval2 = (pli / slval2) + hChn->freqTransferInt;

   if (slval2 < 0)
      val = -slval2;
   else
      val = slval2;

   BMTH_HILO_32TO64_Mul(val, hChn->acqSettings.symbolRate, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &P_hi, &P_lo);
   if (slval2 >= 0)
      x = (int32_t)P_lo;
   else
      x = (int32_t)-P_lo;

   fli += x;
   pli -= slval2;

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLI, pli);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLI, fli);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_DisableChannelInterrupts_isr() - ISR context
******************************************************************************/
BERR_Code BSAT_g1_P_DisableChannelInterrupts_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eBaud);
   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eBer);
   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eGen1);
   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eGen2);

   /* don't disable acquisition timer if interrupt isn't enabled */
   if (!BSAT_g1_P_IsTimerRunning_isr(h, BSAT_TimerSelect_eGen3) || (hChn->gen3TimerIsr != NULL))
      BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eGen3);

   BSAT_CHK_RETCODE(BSAT_g1_P_QpskEnableLockInterrupts_isr(h, false));
   BSAT_CHK_RETCODE(BSAT_g1_P_DftDisableInterrupt_isr(h));

   if (hChn->bHasAfec || hChn->bHasTfec)
   {
      BSAT_CHK_RETCODE(BSAT_g1_P_HpDisableInterrupts_isr(h));
   }

   if (hChn->bHasAfec)
   {
      if (BSAT_g1_P_IsAfecOn_isrsafe(h))
      {
         BSAT_CHK_RETCODE(BSAT_g1_P_AfecEnableLockInterrupts_isr(h, false));
         BSAT_CHK_RETCODE(BSAT_g1_P_AfecEnableMpegInterrupts_isr(h, false));
      }
   }

#ifndef BSAT_EXCLUDE_TFEC
   if (hChn->bHasTfec)
   {
      if (BSAT_g1_P_IsTfecOn_isrsafe(h))
      {
         BSAT_CHK_RETCODE(BSAT_g1_P_TfecEnableLockInterrupts_isr(h, false));
         BSAT_CHK_RETCODE(BSAT_g1_P_TfecEnableSyncInterrupt_isr(h, false));
      }
   }
#endif

#ifndef BSAT_EXCLUDE_MI2C
   BSAT_CHK_RETCODE(BSAT_g1_P_Mi2cEnableInterrupts_isr(h, false));
#endif

   done:
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_DisableChannelInterrupts() - Non-ISR context
******************************************************************************/
BERR_Code BSAT_g1_P_DisableChannelInterrupts(BSAT_ChannelHandle h)
{
   BERR_Code retCode;

   BKNI_EnterCriticalSection();
   retCode = BSAT_g1_P_DisableChannelInterrupts_isr(h);
   BKNI_LeaveCriticalSection();

   return retCode;
}


/******************************************************************************
 BSAT_g1_P_DisableInterrupts() - Non-ISR context
******************************************************************************/
BERR_Code BSAT_g1_P_DisableInterrupts(BSAT_Handle h)
{
   int i;
   BSAT_ChannelHandle hChn;

   for (i = 0; i < h->totalChannels; i++)
   {
      hChn = h->pChannels[i];
      if (hChn != NULL)
         BSAT_g1_P_DisableChannelInterrupts(hChn);
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_PowerDownOpll_isrsafe() - power down the OPLL core
******************************************************************************/
void BSAT_g1_P_PowerDownOpll_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL00, 0x00080001);
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL00, ~0x1);
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_OI_OPLL_PWRDN_RST, 0x7);
#ifdef BCHP_SDS_OI_0_OPLL_PWRDN_RST_pwrdn_ldo_MASK
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_OI_OPLL_PWRDN_RST,
      BCHP_SDS_OI_0_OPLL_PWRDN_RST_pwrdn_ldo_MASK | BCHP_SDS_OI_0_OPLL_PWRDN_RST_pwrdn_bg_MASK);
#endif
}


/******************************************************************************
 BSAT_g1_P_PowerUpOpll_isr() - power up the OPLL core
******************************************************************************/
void BSAT_g1_P_PowerUpOpll_isr(BSAT_ChannelHandle h)
{
#if BCHP_CHIP==45402
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_OI_OPLL_PWRDN_RST, ~0x1C); /* pwrdn=0 */
#else
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OPLL_PWRDN_RST, 0);
#endif
}


/******************************************************************************
 BSAT_g1_P_GetSymbolRateError()
******************************************************************************/
BERR_Code BSAT_g1_P_GetSymbolRateError(BSAT_ChannelHandle h, int32_t *pSymbolRateError)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t bw, shift, val, P_hi, P_lo, Q_hi, Q_lo;
   int32_t bri, sval;

   /* determine actual symbol rate */
   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_BL_BLPCTL);
   bw = ((val >> 9) & 0x06);
   bri = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_BL_BRI);
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

   sval += BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_BL_BFOS);

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL);
   sval *= (32 - (val & 0x1F));
   if ((val & 0x80) == 0) /* all 6 HBs are used */
     sval *= 2;

   BMTH_HILO_32TO64_Mul(hChn->sampleFreq, 16777216, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, (uint32_t)sval, &Q_hi, &Q_lo);

   if (hChn->bUndersample)
      Q_lo = Q_lo << 1;

   *pSymbolRateError = (int32_t)Q_lo - (int32_t)hChn->acqSettings.symbolRate;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetCarrierErrorFli_isrsafe()
******************************************************************************/
static void BSAT_g1_P_GetCarrierErrorFli_isrsafe(BSAT_ChannelHandle h, int32_t *pCarrierErrorFli)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   int32_t sval;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo;

   sval = (int32_t)BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_FLI); /* front loop enabled */
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
 BSAT_g1_P_GetCarrierErrorPli_isrsafe()
******************************************************************************/
static void BSAT_g1_P_GetCarrierErrorPli_isrsafe(BSAT_ChannelHandle h, int32_t *pCarrierErrorPli)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   int32_t sval;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo;

   sval = (int32_t)BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_PLI);
   if (sval >= 0)
      val = sval;
   else
      val = (uint32_t)-sval;
   BMTH_HILO_32TO64_Mul(val, hChn->acqSettings.symbolRate, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 1073741824, &Q_hi, &Q_lo);  /* Q=abs(fval1) */
   BMTH_HILO_64TO64_Div32(Q_hi, Q_lo, 8, &Q_hi, &Q_lo);
   if (sval >= 0)
      *pCarrierErrorPli = (int32_t)Q_lo;
   else
      *pCarrierErrorPli = (int32_t)-Q_lo;
   /* BDBG_ERR(("carrier_error(PLI)=%d", *pCarrierErrorPli)); */
}


/******************************************************************************
 BSAT_g1_P_GetCarrierError_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_GetCarrierError_isrsafe(BSAT_ChannelHandle h, int32_t *pCarrierError)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   int32_t carrier_error_pli, carrier_error_fli, carrier_error, sval;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo, flif;
   bool bHpSpinv, bFrontLoopEnabled, bBackLoopEnabled;

   carrier_error = 0;
   carrier_error_pli = 0;

   bHpSpinv = false;  /* bHpSpinv=true if HP is enabled and spinv detected */
   bFrontLoopEnabled = false;
   bBackLoopEnabled = false;
   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL);
   if (val & 0x08)   /* HP enabled */
      bHpSpinv = BSAT_g1_P_HpIsSpinv_isrsafe(h);

   /* check FLI and PLI for legacy modes */
   if (BSAT_MODE_IS_LEGACY_QPSK(hChn->acqSettings.mode))
   {
      /* check if front loop is enabled */
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE);
      if (val & 0x01)
      {
         /* front loop enable not controlled by HP */
         val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1);
         bFrontLoopEnabled = (val & 0x10) ? true : false;
      }

      /* read FLI if front loop enabled */
      if (bFrontLoopEnabled)
         BSAT_g1_P_GetCarrierErrorFli_isrsafe(h, &carrier_error_fli);
      else
         carrier_error_fli = 0;
      carrier_error = carrier_error_fli;

      /* check if back loop is enabled */
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_HPOVERRIDE);
      if (val & 0x10)
      {
         /* back loop enable is not controlled by HP */
         val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2);
         bBackLoopEnabled = (val & 0x1000) ? true : false;
      }

      /* read PLI if back loop enabled */
      if (bBackLoopEnabled)
         BSAT_g1_P_GetCarrierErrorPli_isrsafe(h, &carrier_error_pli);
      else
         carrier_error_pli = 0;
      carrier_error += carrier_error_pli;
   }
   else
   {
      BSAT_CHK_RETCODE(BSAT_g1_P_HpGetFreqOffsetEstimate(h, &carrier_error));

      if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
      {
         BSAT_g1_P_GetCarrierErrorFli_isrsafe(h, &carrier_error_fli);
         BSAT_g1_P_GetCarrierErrorPli_isrsafe(h, &carrier_error_pli);
         sval = carrier_error_fli + carrier_error_pli;
         if (bHpSpinv)
            carrier_error -= sval;
         else
            carrier_error += sval;
      }
   }

   /* FLIF * Fs / 2^28 */
   flif = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_FLIF);
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
 BSAT_g1_P_ResetMpegCount_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_ResetMpegCount_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   /* reset mpeg frame and error counters */
#ifdef BCHP_SDS_OI_MPEG_CTL0
   int i;

   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_OI_MPEG_CTL0, BCHP_SDS_OI_0_MPEG_CTL0_mpeg_counter_reset_MASK);
   for (i = 0; i < 8; i++)
   {
      hChn->mpegFrameCountMs[i] = 0;
      hChn->mpegErrorCountMs[i] = 0;
   }
#else
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_OI_OIFCTL01, 0x00200000);
#endif

   hChn->mpegFrameCount = 0;
   hChn->mpegErrorCount = 0;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_UpdateMpegCount()
******************************************************************************/
BERR_Code BSAT_g1_P_UpdateMpegCount(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t frc, ferc;

   /* take snapshot of mpeg frame and error counter */
#ifdef BCHP_SDS_OI_MPEG_CTL0
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_OI_MPEG_CTL0, BCHP_SDS_OI_0_MPEG_CTL0_mpeg_counter_snap_MASK);
#else
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_OI_OIFCTL01, 0x00400000);
#endif
   BKNI_Delay(1);

   frc = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_FRC);
   hChn->mpegFrameCount += frc;
   ferc = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_FERC);
   hChn->mpegErrorCount += ferc;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_ResetSignalNotification_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_ResetSignalNotification_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   hChn->notification = 0;
   hChn->rainFadeSnrAve = 0;
   hChn->rainFadeSnrMax = 0;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_CheckSignalNotification_isr() - use BSAT_g1_P_CheckStatusIndicators as reference
******************************************************************************/
static BERR_Code BSAT_g1_P_CheckSignalNotification_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   bool bSetEvent = false;
   int32_t offset;
   uint32_t diff, snr, snr8, val, snrThreshold1, snrThreshold2;
   int idx;

   static const uint16_t BSAT_SNR_THRESHOLD1[] = /* in units of 1/256 dB */
   {
      822,      /* 0: DVB 1/2 = 3.21 */
      1308,     /* 1: DVB 2/3 = 5.109375 */
      1567,     /* 2: DVB 3/4 = 6.1211 */
      1817,     /* 3: DVB 5/6 = 7.09766 */
      2017,     /* 4: DVB 7/8 = 7.8789 */
      1331,     /* 5: DTV 1/2 = 5.2 */
      1690,     /* 6: DTV 2/3 = 6.6 */
      2253,     /* 7: DTV 6/7 = 8.8 */
      15 * 256, /* 8: DCII 1/2 */
      15 * 256, /* 9: DCII 2/3 */
      15 * 256, /* 10: DCII 3/4 */
      15 * 256, /* 11: DCII 5/6 */
      15 * 256, /* 12: DCII 7/8 */
      15 * 256, /* 13: DCII 5/11 */
      15 * 256, /* 14: DCII 3/5 */
      15 * 256, /* 15: DCII 4/5 */
      538,  /* 16: DVB-S2 QPSK 1/4 = 2.1 */
      538,  /* 17: DVB-S2 QPSK 1/3 = 2.1 */
      538,  /* 18: DVB-S2 QPSK 2/5 = 2.1 */
      947,  /* 19: DVB-S2 QPSK 1/2 = 3.7 */
      1357,  /* 20: DVB-S2 QPSK 3/5 = 5.3 */
      1587, /* 21: DVB-S2 QPSK 2/3 = 6.2 */
      1562, /* 22: DVB-S2 QPSK 3/4 = 6.1 */
      1741, /* 23: DVB-S2 QPSK 4/5 = 6.8 */
      1869, /* 24: DVB-S2 QPSK 5/6 = 7.3 */
      2150, /* 25: DVB-S2 QPSK 8/9 = 8.4 */
      2227, /* 26: DVB-S2 QPSK 9/10 = 8.7 */
      1997, /* 27: DVB-S2 8PSK 3/5 = 7.8 */
      2278, /* 28: DVB-S2 8PSK 2/3 = 8.9 */
      2611, /* 29: DVB-S2 8PSK 3/4 = 10.2 */
      2995, /* 30: DVB-S2 8PSK 5/6 = 11.7 */
      3328, /* 31: DVB-S2 8PSK 8/9 = 13 */
      3379, /* 32: DVB-S2 8PSK 9/10 = 13.2 */
      2893, /* 33: DVB-S2 16APSK 2/3 = 11.3 */
      3174, /* 34: DVB-S2 16APSK 3/4 = 12.4 */
      3405, /* 35: DVB-S2 16APSK 4/5 = 13.3 */
      3558, /* 36: DVB-S2 16APSK 5/6 = 13.9 */
      3866, /* 37: DVB-S2 16APSK 8/9 = 15.1 */
      3917, /* 38: DVB-S2 16APSK 9/10 = 15.3 */
      3891, /* 39: DVB-S2 32APSK 3/4 = 15.2 */
      4070, /* 40: DVB-S2 32APSK 4/5 = 15.9 */
      4250, /* 41: DVB-S2 32APSK 5/6 = 16.6 */
      4582, /* 42: DVB-S2 32APSK 8/9 = 17.9 */
      4659, /* 43: DVB-S2 32APSK 9/10 = 18.2 */
      400,  /* 44: Turbo QPSK 1/2 = 1.5625 */
      903,  /* 45: Turbo QPSK 2/3 = 3.5273 */
      1173, /* 46: Turbo QPSK 3/4 = 4.582 */
      1466, /* 47: Turbo QPSK 5/6 = 5.7266 */
      1642, /* 48: Turbo QPSK 7/8 = 6.4140625 */
      1842, /* 49: Turbo 8PSK 2/3 = 7.1953 */
      2042, /* 50: Turbo 8PSK 3/4 = 7.97656 */
      2172, /* 51: Turbo 8PSK 4/5 = 8.484375 */
      2549, /* 52: Turbo 8PSK 5/6 = 9.957 */
      2674, /* 53: Turbo 8PSK 8/9 = 10.4453 */
      128,  /* 54: DVB-S2X QPSK 13/45 = 0.5 */
      614,  /* 55: DVB-S2X QPSK 9/20 = 2.4 */
      922,  /* 56: DVB-S2X QPSK 11/20 = 3.6 */
      1818, /* 57: DVB-S2X 8APSK 5/9-L = 7.1 */
      1920, /* 58: DVB-S2X 8APSK 26/45-L = 7.5 */
      2227, /* 59: DVB-S2X 8APSK 23/36 = 8.7 */
      2355, /* 60: DVB-S2X 8APSK 25/36 = 9.2 */
      2483, /* 61: DVB-S2X 8APSK 13/18 = 9.7 */
      2176, /* 62: DVB-S2X 16APSK 1/2-L = 8.5 */
      2330, /* 63: DVB-S2X 16APSK 8/15-L = 9.1 */
      2406, /* 64: DVB-S2X 16APSK 5/9-L = 9.4 */
      2970, /* 65: DVB-S2X 16APSK 26/45 = 11.6 */
      2918, /* 66: DVB-S2X 16APSK 3/5 = 11.4 */
      2560, /* 67: DVB-S2X 16APSK 3/5-L = 10.0 */
      2816, /* 68: DVB-S2X 16APSK 28/45 = 11.0 */
      2842, /* 69: DVB-S2X 16APSK 23/36 = 11.1 */
      2714, /* 70: DVB-S2X 16APSK 2/3-L = 10.6 */
      2995, /* 71: DVB-S2X 16APSK 25/36 = 11.7 */
      3354, /* 72: DVB-S2X 16APSK 13/18 = 13.1 */
      3456, /* 73: DVB-S2X 16APSK 7/9 = 13.5 */
      3635, /* 74: DVB-S2X 16APSK 77/90 = 14.2 */
      3942, /* 75: DVB-S2X 32APSK 2/3-L = 15.4 */
      3942, /* 76: DVB-S2X 32APSK 32/45 = 15.4 */
      3942, /* 77: DVB-S2X 32APSK 11/15 = 15.4 */
      3968  /* 78: DVB-S2X 32APSK 7/9 = 15.5 */
   };

   static const uint16_t BSAT_SNR_THRESHOLD2[] = /* in units of 1/256 dB */
   {
      552,  /* 0: DVB 1/2 = 2.15625 */
      1038, /* 1: DVB 2/3 = 4.0547 */
      1308, /* 2: DVB 3/4 = 5.109375 */
      1567, /* 3: DVB 5/6 = 6.1211 */
      1767, /* 4: DVB 7/8 = 6.90234 */
      1075, /* 5: DTV 1/2 = 4.2 */
      1434, /* 6: DTV 2/3 = 5.6 */
      1997, /* 7: DTV 6/7 = 7.8 */
      10 * 256, /* 8: DCII 1/2 */
      10 * 256, /* 9: DCII 2/3 */
      10 * 256, /* 10: DCII 3/4 */
      10 * 256, /* 11: DCII 5/6 */
      10 * 256, /* 12: DCII 7/8 */
      10 * 256, /* 13: DCII 5/11 */
      10 * 256, /* 14: DCII 3/5 */
      10 * 256, /* 15: DCII 4/5 */
      26,  /* 16: DVB-S2 QPSK 1/4 = 0.1 */
      26,  /* 17: DVB-S2 QPSK 1/3 = 0.1 */
      26,  /* 18: DVB-S2 QPSK 2/5 = 0.1 */
      435,  /* 19: DVB-S2 QPSK 1/2 = 1.7 */
      589,  /* 20: DVB-S2 QPSK 3/5 = 2.3 */
      819, /* 21: DVB-S2 QPSK 2/3 = 3.2 */
      1050, /* 22: DVB-S2 QPSK 3/4 = 4.1 */
      1229, /* 23: DVB-S2 QPSK 4/5 = 4.8 */
      1357, /* 24: DVB-S2 QPSK 5/6 = 5.3 */
      1638, /* 25: DVB-S2 QPSK 8/9 = 6.4 */
      1715, /* 26: DVB-S2 QPSK 9/10 = 6.7 */
      1485, /* 27: DVB-S2 8PSK 3/5 = 5.8 */
      1766, /* 28: DVB-S2 8PSK 2/3 = 6.9 */
      2099, /* 29: DVB-S2 8PSK 3/4 = 8.2 */
      2483, /* 30: DVB-S2 8PSK 5/6 = 9.7 */
      2816, /* 31: DVB-S2 8PSK 8/9 = 11 */
      2867, /* 32: DVB-S2 8PSK 9/10 = 11.2 */
      2381, /* 33: DVB-S2 16APSK 2/3 = 9.3 */
      2662, /* 34: DVB-S2 16APSK 3/4 = 10.4 */
      2893, /* 35: DVB-S2 16APSK 4/5 = 11.3 */
      3046, /* 36: DVB-S2 16APSK 5/6 = 11.9 */
      3354, /* 37: DVB-S2 16APSK 8/9 = 13.1 */
      3405, /* 38: DVB-S2 16APSK 9/10 = 13.3 */
      3379, /* 39: DVB-S2 32APSK 3/4 = 13.2 */
      3558, /* 40: DVB-S2 32APSK 4/5 = 13.9 */
      3738, /* 41: DVB-S2 32APSK 5/6 = 14.6 */
      4070, /* 42: DVB-S2 32APSK 8/9 = 15.9 */
      4147, /* 43: DVB-S2 32APSK 9/10 = 16.2 */
      400,  /* 44: Turbo QPSK 1/2 = 1.5625 */
      903,  /* 45: Turbo QPSK 2/3 = 3.5273 */
      1173, /* 46: Turbo QPSK 3/4 = 4.582 */
      1466, /* 47: Turbo QPSK 5/6 = 5.7266 */
      1642, /* 48: Turbo QPSK 7/8 = 6.4140625 */
      1842, /* 49: Turbo 8PSK 2/3 = 7.1953 */
      2042, /* 50: Turbo 8PSK 3/4 = 7.97656 */
      2172, /* 51: Turbo 8PSK 4/5 = 8.484375 */
      2549, /* 52: Turbo 8PSK 5/6 = 9.957 */
      2674, /* 53: Turbo 8PSK 8/9 = 10.4453 */
      26,  /* 54: DVB-S2X QPSK 13/45 = 0.1 */
      102,  /* 55: DVB-S2X QPSK 9/20 = 0.4 */
      410,  /* 56: DVB-S2X QPSK 11/20 = 1.6 */
      1306, /* 57: DVB-S2X 8APSK 5/9-L = 5.1 */
      1408, /* 58: DVB-S2X 8APSK 26/45-L = 5.5 */
      1715, /* 59: DVB-S2X 8APSK 23/36 = 6.7 */
      1843, /* 60: DVB-S2X 8APSK 25/36 = 7.2 */
      1971, /* 61: DVB-S2X 8APSK 13/18 = 7.7 */
      1664, /* 62: DVB-S2X 16APSK 1/2-L = 6.5 */
      1818, /* 63: DVB-S2X 16APSK 8/15-L = 7.1 */
      1894, /* 64: DVB-S2X 16APSK 5/9-L = 7.4 */
      2458, /* 65: DVB-S2X 16APSK 26/45 = 9.6 */
      2406, /* 66: DVB-S2X 16APSK 3/5 = 9.4 */
      2048, /* 67: DVB-S2X 16APSK 3/5-L = 8.0 */
      2304, /* 68: DVB-S2X 16APSK 28/45 = 9.0 */
      2330, /* 69: DVB-S2X 16APSK 23/36 = 9.1 */
      2202, /* 70: DVB-S2X 16APSK 2/3-L = 8.6 */
      2483, /* 71: DVB-S2X 16APSK 25/36 = 9.7 */
      2842, /* 72: DVB-S2X 16APSK 13/18 = 11.1 */
      2944, /* 73: DVB-S2X 16APSK 7/9 = 11.5 */
      3123, /* 74: DVB-S2X 16APSK 77/90 = 12.2 */
      3430, /* 75: DVB-S2X 32APSK 2/3-L = 13.4 */
      3430, /* 76: DVB-S2X 32APSK 32/45 = 13.4 */
      3430, /* 77: DVB-S2X 32APSK 11/15 = 13.4 */
      3456  /* 78: DVB-S2X 32APSK 7/9 = 13.5 */
   };

   /* check freq drift threshold */
   BSAT_CHK_RETCODE(BSAT_g1_P_GetCarrierError_isrsafe(h, &offset));

   if (offset > hChn->initFreqOffset)
      diff = (uint32_t)(offset - hChn->initFreqOffset);
   else
      diff = (uint32_t)(hChn->initFreqOffset - offset);
   if (diff > hChn->sigNotSettings.freqDriftThreshold)
   {
      if ((hChn->notification & BSAT_STATUS_NOTIFICATION_FREQ_DRIFT) == 0)
      {
         /* BDBG_MSG(("ch %d freq drift detected", h->channel)); */
         hChn->notification |= BSAT_STATUS_NOTIFICATION_FREQ_DRIFT;
         bSetEvent = true;
      }
   }
   else
      hChn->notification &= ~BSAT_STATUS_NOTIFICATION_FREQ_DRIFT;

   BSAT_CHK_RETCODE(BSAT_g1_P_GetSnr_isr(h, &snr)); /* snr is in 1/256 dB */

   /* determine threshold 1 and 2 values from mode */
   if (BSAT_MODE_IS_DVBS(hChn->actualMode))
      idx = hChn->actualMode - BSAT_Mode_eDvbs_1_2;
   else if (BSAT_MODE_IS_DTV(hChn->actualMode))
      idx = 5 + hChn->actualMode - BSAT_Mode_eDss_1_2;
   else if (BSAT_MODE_IS_DCII(hChn->actualMode))
      idx = 8 + hChn->actualMode - BSAT_Mode_eDcii_1_2;
   else if (BSAT_MODE_IS_DVBS2(hChn->actualMode))
      idx = 16 + (hChn->actualMode - BSAT_Mode_eDvbs2_Qpsk_1_4);
   else if (BSAT_MODE_IS_TURBO(hChn->actualMode))
      idx = 44 + (hChn->actualMode - BSAT_Mode_eTurbo_Qpsk_1_2);
   else if (BSAT_MODE_IS_DVBS2X(hChn->actualMode))
      idx = 54 + (hChn->actualMode - BSAT_Mode_eDvbs2x_Qpsk_13_45);
   else
   {
      snrThreshold1 = snrThreshold2 = 0;
      goto check_snr_threshold;
   }

   /* BDBG_MSG(("snrThreshold_idx=%d (actualMode=%d)", idx, hChn->actualMode)); */
   snrThreshold1 = BSAT_SNR_THRESHOLD1[idx];
   snrThreshold2 = BSAT_SNR_THRESHOLD2[idx];

   /* check SNR threshold 1 */
   check_snr_threshold:
   if (snr < snrThreshold1)
   {
      if ((hChn->notification & BSAT_STATUS_NOTIFICATION_THRESHOLD1) == 0)
      {
         /* BDBG_MSG(("ch %d CNR threshold1 detected", h->channel)); */
         hChn->notification |= BSAT_STATUS_NOTIFICATION_THRESHOLD1;
         bSetEvent = true;
      }
   }
   else
      hChn->notification &= ~BSAT_STATUS_NOTIFICATION_THRESHOLD1;

   /* check SNR threshold 2 */
   if (snr < snrThreshold2)
   {
      if ((hChn->notification & BSAT_STATUS_NOTIFICATION_THRESHOLD2) == 0)
      {
         /* BDBG_MSG(("ch %d CNR threshold2 detected", h->channel)); */
         hChn->notification |= BSAT_STATUS_NOTIFICATION_THRESHOLD2;
         bSetEvent = true;
      }
   }
   else
      hChn->notification &= ~BSAT_STATUS_NOTIFICATION_THRESHOLD2;

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
      val = val / hChn->sigNotSettings.rainFadeWindow;
      if (snr8 > hChn->rainFadeSnrAve)
         hChn->rainFadeSnrAve += val;
      else
         hChn->rainFadeSnrAve -= val;

      if (snr8 > hChn->rainFadeSnrMax)
         hChn->rainFadeSnrMax = snr8; /* set new max */
      else
         hChn->rainFadeSnrMax -= ((hChn->rainFadeSnrMax - hChn->rainFadeSnrAve) / hChn->sigNotSettings.rainFadeWindow); /* decay max snr */

      if (snr8 < (hChn->rainFadeSnrMax - (uint32_t)(hChn->sigNotSettings.rainFadeThreshold)))
      {
         if ((hChn->notification & BSAT_STATUS_NOTIFICATION_RAIN_FADE) == 0)
         {
            hChn->notification |= BSAT_STATUS_NOTIFICATION_RAIN_FADE;
            bSetEvent = true;
         }
      }
      else
         hChn->notification &= ~BSAT_STATUS_NOTIFICATION_RAIN_FADE;
   }
   /* BDBG_MSG(("rain_fade: ave=0x%08X, max=0x%08X, curr=0x%08X", hChn->rainFadeSnrAve, hChn->rainFadeSnrMax, snr8)); */

   if (bSetEvent)
   {
      /* BDBG_MSG(("set status event")); */
      BKNI_SetEvent(hChn->hSignalNotificationEvent);
   }

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_ResetSignalDetectStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_ResetSignalDetectStatus(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BKNI_Memset((void*)&(hChn->signalDetectStatus), 0, sizeof(BSAT_SignalDetectStatus));
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_ResetSymbolRateScanStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_ResetSymbolRateScanStatus(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BKNI_Memset((void*)&(hChn->symbolRateScanStatus), 0, sizeof(BSAT_SymbolRateScanStatus));
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_ResetToneDetectStatus()
******************************************************************************/
BERR_Code BSAT_g1_P_ResetToneDetectStatus(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BKNI_Memset((void*)&(hChn->toneDetectStatus), 0, sizeof(BSAT_ToneDetectStatus));
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_ResetLockFilter_isr()
******************************************************************************/
static void BSAT_g1_P_ResetLockFilter_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi;

   hChn->stableLockTimeout = BSAT_MODE_IS_LEGACY_QPSK(hChn->actualMode) ? 3000 : 5000;
   if (hChn->acqType == BSAT_AcqType_eDvbs2)
      hChn->maxStableLockTimeout = 20000;
   else
      hChn->maxStableLockTimeout = 150000;
   hChn->lockFilterRamp = -2;

   if (hChn->acqSettings.symbolRate < 20000000)
   {
      /* scale to symbol rate */
      BMTH_HILO_32TO64_Mul(20000000, hChn->stableLockTimeout, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqSettings.symbolRate, &Q_hi, &(hChn->stableLockTimeout));
      BMTH_HILO_32TO64_Mul(20000000, hChn->maxStableLockTimeout, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqSettings.symbolRate, &Q_hi, &(hChn->maxStableLockTimeout));
   }

   /* BSAT_DEBUG_ACQ(BDBG_ERR(("BSAT_g1_P_ResetLockFilter_isr(%d), stableLockTimeout=%d, maxStableLockTimeout=%d", h->channel, hChn->stableLockTimeout, hChn->maxStableLockTimeout))); */
}


/******************************************************************************
 BSAT_g1_P_DetermineUndersampleMode_isr()
******************************************************************************/
static void BSAT_g1_P_DetermineUndersampleMode_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, Fb4;

   Fb4 = hChn->acqSettings.symbolRate << 2;
   val = (Fb4 * 5) / 1000;  /* .5% margin */
   Fb4 += val;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CG_CGDIV00);

   if ((hChn->operation == BSAT_Operation_eToneDetect) || (hChn->operation == BSAT_Operation_eSymbolRateScan))
      goto no_undersample;

   if (hChn->sampleFreq  < Fb4)
   {
      BDBG_MSG(("Undersample Mode enabled"));
      hChn->bUndersample = true;
      val |= 0x01;
   }
   else
   {
      no_undersample:
      hChn->bUndersample = false;
      val &= ~0x01;
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CG_CGDIV00, val);
}


/******************************************************************************
 BSAT_g1_P_SetAcqType_isr()
******************************************************************************/
static void BSAT_g1_P_SetAcqType_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_Mode mode = hChn->acqSettings.mode;

   if (BSAT_MODE_IS_DVBS2(mode) || BSAT_MODE_IS_DVBS2X(mode))
   {
      hChn->acqType = BSAT_AcqType_eDvbs2;
      return;
   }
#ifndef BSAT_EXCLUDE_TFEC
   if (BSAT_MODE_IS_TURBO(mode))
   {
      hChn->acqType = BSAT_AcqType_eTurbo;
      return;
   }
#endif
   if (BSAT_MODE_IS_LEGACY_QPSK(mode))
      hChn->acqType = BSAT_AcqType_eQpsk;
   else
   {
      /* should never get here */
      BDBG_ERR(("BSAT_g1_P_SetFunctTable(): invalid mode (%d)", hChn->acqSettings.mode));
      BDBG_ASSERT(0);
   }
}


/******************************************************************************
 BSAT_g1_P_BlindScanSetNextMode_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_BlindScanSetNextMode_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   /* determine next mode to try */
   next_mode:
   while (hChn->miscSettings.blindScanModes)
   {
      if (hChn->blindScanCurrMode == 0)
         hChn->blindScanCurrMode = 1;
      else
         hChn->blindScanCurrMode = (hChn->blindScanCurrMode << 1) & BSAT_BLIND_SCAN_MODE_MASK;

      if (hChn->blindScanCurrMode == 0)
         BSAT_g1_P_IncrementReacqCount_isr(h);
      else if (hChn->blindScanCurrMode & hChn->miscSettings.blindScanModes)
         break;
   }

   if (hChn->blindScanCurrMode == 0)
   {
      BDBG_ERR(("BSAT_g1_P_BlindScanSetNextMode_isr(): blindScanCurrMode should not be 0"));
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      return retCode;
   }

   if (hChn->blindScanCurrMode & BSAT_BLIND_SCAN_MODE_DVBS)
   {
      BDBG_MSG(("Blind scan: trying DVB scan"));
      hChn->acqSettings.mode = BSAT_Mode_eDvbs_scan;
   }
   else if (hChn->blindScanCurrMode & BSAT_BLIND_SCAN_MODE_DTV)
   {
      BDBG_MSG(("Blind scan: trying DTV scan"));
      hChn->acqSettings.mode = BSAT_Mode_eDss_scan;
   }
   else if (hChn->blindScanCurrMode & BSAT_BLIND_SCAN_MODE_DCII)
   {
      BDBG_MSG(("Blind scan: trying DCII scan"));
      hChn->acqSettings.mode = BSAT_Mode_eDcii_scan;
   }
   else if (hChn->blindScanCurrMode & BSAT_BLIND_SCAN_MODE_DVBS2)
   {
      if (hChn->bHasAfec)
      {
         BDBG_MSG(("Blind scan: trying DVB-S2 scan"));
         hChn->acqSettings.mode = BSAT_Mode_eDvbs2_scan;
      }
      else
      {
         /* remove LDPC from blind scan because this channel does not have an AFEC */
         hChn->miscSettings.blindScanModes &= ~BSAT_BLIND_SCAN_MODE_DVBS2;
         goto next_mode;
      }
   }
   else if (hChn->blindScanCurrMode & BSAT_BLIND_SCAN_MODE_TURBO)
   {
#ifndef BSAT_EXCLUDE_TFEC
      if (hChn->bHasTfec)
      {
         BDBG_MSG(("Blind scan: trying Turbo scan"));
         hChn->acqSettings.mode = BSAT_Mode_eTurbo_scan;
      }
      else
#endif
      {
         /* remove Turbo from blind scan because this channel does not have a TFEC */
         hChn->miscSettings.blindScanModes &= ~BSAT_BLIND_SCAN_MODE_TURBO;
         goto next_mode;
      }
   }
   else
   {
      BDBG_ERR(("BSAT_g1_P_BlindScanSetNextMode_isr(): invalid value for blindScanCurrMode (0x%X)", hChn->blindScanCurrMode));
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      return retCode;
   }

   BSAT_g1_P_SetAcqType_isr(h);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_BlindScanInit_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_BlindScanInit_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)(h->pImpl);

   hChn->bBlindScan = true;
   hChn->blindScanCurrMode = 0;
   return BSAT_g1_P_BlindScanSetNextMode_isr(h);
}


/******************************************************************************
 BSAT_g1_P_GetActualMode_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_GetActualMode_isr(BSAT_ChannelHandle h, BSAT_Mode *pActualMode)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t fmod, vst, i;

   if ((hChn->operation != BSAT_Operation_eAcq) || (hChn->acqSettings.options & BSAT_ACQ_TUNER_TEST_MODE))
      goto unknown_mode;

   if (BSAT_MODE_IS_LEGACY_QPSK(hChn->acqSettings.mode))
   {
      /* Legacy QPSK */
      fmod = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_FEC_FMOD);
      fmod = (fmod >> 8) & 0x0C;
      if (fmod == 0x04)
         *pActualMode = BSAT_Mode_eDss_1_2; /* DTV */
      else if (fmod == 0x00)
         *pActualMode = BSAT_Mode_eDvbs_1_2; /* DVB */
      else
         *pActualMode = BSAT_Mode_eDcii_1_2; /* DCII */

      /* determine code rate */
      vst = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VST);
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
         *pActualMode = BSAT_Mode_eUnknown;
      else
         *pActualMode += i;
   }
   else if (BSAT_MODE_IS_DVBS2(hChn->acqSettings.mode))
   {
      /* actual mode is set after modcod in ldpc scan */
      if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
         *pActualMode = BSAT_Mode_eDvbs2_ACM;
      else if (hChn->acqSettings.mode != BSAT_Mode_eDvbs2_scan)
         *pActualMode = hChn->acqSettings.mode;
      else
         *pActualMode = hChn->actualMode;
   }
#ifdef BSAT_HAS_DVBS2X
   else if (BSAT_MODE_IS_DVBS2X(hChn->acqSettings.mode))
   {
      *pActualMode = hChn->acqSettings.mode;
   }
#endif
#ifndef BSAT_EXCLUDE_TFEC
   else if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
   {
      /* actual mode is set at start of each acquisition in turbo scan */
      if (hChn->acqSettings.mode != BSAT_Mode_eTurbo_scan)
         *pActualMode = hChn->acqSettings.mode;
      else
         *pActualMode = hChn->actualMode;
   }
#endif
   else
   {
      unknown_mode:
      *pActualMode = BSAT_Mode_eUnknown;
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetNyquistAlpha_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_SetNyquistAlpha_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t Fs_over_Fb, nvctl, pfctl = 0x01;
   uint32_t alpha;

   Fs_over_Fb = (hChn->sampleFreq * 4) / hChn->acqSettings.symbolRate;
   if (Fs_over_Fb < 9)
     nvctl = 0x80;
   else if (Fs_over_Fb < 10)
     nvctl = 0x40;
   else
     nvctl = 0;

   alpha = hChn->acqSettings.options & BSAT_ACQ_NYQUIST_MASK;

   switch (alpha)
   {
      case BSAT_ACQ_NYQUIST_35:
         pfctl = 0;
         break;

      case BSAT_ACQ_NYQUIST_10:
         nvctl |= 0x200;
         break;

      case BSAT_ACQ_NYQUIST_20:
         nvctl |= 0x100;
         break;

      case BSAT_ACQ_NYQUIST_5:
         nvctl |= 0x300;
         break;
   }

   if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
      pfctl |= (1 << BCHP_SDS_BL_0_PFCTL_pd_sel_SHIFT);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_NVCTL, nvctl);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_PFCTL, pfctl);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetBaudBw_isr() - damp is scaled 2^2
******************************************************************************/
BERR_Code BSAT_g1_P_SetBaudBw_isr(BSAT_ChannelHandle h, uint32_t bw, uint32_t damp)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t lval1, lval2, lval3, P_hi, P_lo;

   /* BDBG_MSG(("BSAT_g1_P_SetBaudBw_isr(): bw=%d, damp*4=%d", bw, damp)); */
   BMTH_HILO_32TO64_Mul(bw, 1073741824, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqSettings.symbolRate, &lval2, &lval1);
   lval2 = ((lval1 * damp) + 1) >> 1;
   lval2 &= 0xFFFFFF00;
   BMTH_HILO_32TO64_Mul(lval1, bw, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqSettings.symbolRate, &lval3, &lval1);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BRLC, lval2);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BRIC, lval1);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetCarrierBw_isr() - damp is scaled 2^2
******************************************************************************/
BERR_Code BSAT_g1_P_SetCarrierBw_isr(BSAT_ChannelHandle h, uint32_t bw, uint32_t damp)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t lval1, lval2, P_hi, P_lo, x, mb, stfllc;
   static const uint32_t int_scale[7] = {1, 4, 16, 64, 256, 1024, 4096};
   static const uint32_t lin_scale[5] = {1, 4, 16, 64, 256};

#ifdef BSAT_HAS_CONTEXT_SWITCH
    if (BSAT_MODE_IS_LEGACY_QPSK(hChn->acqSettings.mode))
    {
       retCode = BSAT_g1_P_SetPlc_isr(h, bw, damp*2);
    }
#endif /* BSAT_HAS_CONTEXT_SWITCH */

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
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqSettings.symbolRate, &lval2, &lval1);
      if (lval1 > hChn->sampleFreq)
         break;
   }
   if (x)
      x--;

   lval2 = ((bw + 5) / 10);
   BMTH_HILO_32TO64_Mul(lval2 * int_scale[x] * 8, lval2 * 8192, &P_hi, &P_lo);
   lval1 = ((hChn->acqSettings.symbolRate + 5) / 10);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, lval1, &P_hi, &P_lo);
   lval1 = ((hChn->sampleFreq + 5) / 10);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, lval1, &P_hi, &P_lo);
   mb = ((P_lo << 15) + 0x8000);
   mb &= 0xFFFF0000;
   mb |= ((x & 0x0F) << 8);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLLC, stfllc);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLIC, mb);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLLC1, stfllc);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLIC1, mb);
   /* BDBG_MSG(("BSAT_g1_P_SetCarrierBw_isr(): bw=%d, damp*4=%d, fllc=0x%X, flic=0x%X", bw, damp, stfllc, mb)); */
   return retCode;
}


#if 0
/******************************************************************************
 BSAT_g1_P_GetNumHb() - returns the number of halfbands
******************************************************************************/
static uint32_t BSAT_g1_P_GetNumHb(uint32_t Fb, uint32_t Fs)
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
 BSAT_g1_P_SetBfos_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_SetBfos_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Q_lo, hb, two_pow_hb;

   hb = BSAT_g1_P_GetNumDecimatingFilters_isr(h);
   two_pow_hb = 1 << hb;

   BMTH_HILO_32TO64_Mul(16777216, hChn->sampleFreq, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqSettings.symbolRate, &Q_hi, &Q_lo);
   BMTH_HILO_64TO64_Div32(Q_hi, Q_lo, two_pow_hb, &Q_hi, &Q_lo);

   if (hChn->bUndersample)
      Q_lo = Q_lo << 1;
   Q_lo &= ~0xFF;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BFOS, Q_lo);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetDecimationFilters_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_SetDecimationFilters_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle*)h->pDevice->pImpl;
   uint32_t Fs_over_Fb, P_hi, P_lo, Q_hi, filtctl;
   uint32_t val2, val3, data0, dfctl;

   if (hChn->miscSettings.bOverrideFiltctl)
   {
      filtctl = hChn->miscSettings.filtctlOverride;
      goto write_filtctl;
   }

   /* ckpark: optimize filtctl in range Fb=(15e6,19e6) */
   if ((hChn->acqSettings.symbolRate > 15000000) && (hChn->acqSettings.symbolRate < 19000000))
   {
      filtctl = 0xC0DF;
      goto write_filtctl;
   }

   /* Fs_over_Fb = 2^16 * Fs/Fb */
   BMTH_HILO_32TO64_Mul(65536, hChn->sampleFreq, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqSettings.symbolRate, &Q_hi, &Fs_over_Fb);

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

   if (hDev->networkSpec == BSAT_NetworkSpec_eEuro)
      val3 = 373555; /* 5.7*2^16 */
   else
      val3 = 416154; /* 6.35*2^16 */

   if (val2 > val3)
      filtctl |= 0x0000; /* quarter band */
   else if (val2 > 279183) /* 4.26*2^16 */
      filtctl |= 0x4000; /* third band */
   else
      filtctl |= 0xC000; /* half band */ /* was 0xC000 */

   write_filtctl:
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL, filtctl);

   filtctl &= 0xFF;
   if (filtctl == 0x10)
      dfctl = 0x10;
   else if (filtctl == 0x00)
      dfctl = 0x30;
   else
      dfctl = 0;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_DFCTL, dfctl);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_ConfigFe_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_ConfigFe_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val;

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLIF, 0);

   /* AGF setup */
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_FE_AGFCTL, 0x00000001); /* reset AGF */
   if (BSAT_MODE_IS_LEGACY_QPSK(hChn->acqSettings.mode))
      val = 0x04300000;
   else
      val = 0x0A0A0000;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_AGF, val);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_AGFCTL, 0);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_MIXCTL, 1);

#ifndef BSAT_HAS_DVBS2X
   /* reset DCO */
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_FE_DCOCTL, 0x00000300);

   /* reset IQ */
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_FE_IQCTL, 0x00000001);
#endif

   BSAT_CHK_RETCODE(BSAT_g1_P_SetNyquistAlpha_isr(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_SetBaudBw_isr(h, hChn->acqSettings.symbolRate / 100, 4));

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BRI, 0);
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_BL_BLPCTL, 0x00000100); /* reset BL */;

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_SetFfeMainTap_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_SetFfeMainTap_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)(h->pImpl);
   uint32_t val;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL);
   val &= 0xFF00FFFF;
   /* Xiaofen: main tap 6 applies to all legacy modes (instead of just dtv) */
   if ((BSAT_MODE_IS_LEGACY_QPSK(hChn->acqSettings.mode)) || hChn->bUndersample)
      val |= 0x00060000;
   else
      val |= 0x000C0000;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetFlifOffset_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_SetFlifOffset_isr(BSAT_ChannelHandle h, int32_t offset)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
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
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLIF, val);
   /* BDBG_MSG(("BSAT_g1_P_SetFlifOffset_isr(): offset=%d, FLIF=0x%08X", offset, val)); */
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_CheckTimingLoopStateMachine_isr(): count2=original BFOS
******************************************************************************/
static BERR_Code BSAT_g1_P_CheckTimingLoopStateMachine_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val, i;
   int32_t  bfos_step, sum, bri;

   while (hChn->functState != 0xFFFFFFFF)
   {
      /* BDBG_MSG(("BSAT_g1_P_CheckTimingLoop(): state=%d", hChn->funct_state)); */
      switch (hChn->functState)
      {
         case 0:
            /* open up timing loop bw */
            BSAT_CHK_RETCODE(BSAT_g1_P_SetBaudBw_isr(h, hChn->acqSettings.symbolRate / 400, 4 * 4));

            bfos_step = hChn->count2 >> 9;
            if (hChn->count1 == 0)
               bfos_step = -bfos_step;

            val = (uint32_t)((int32_t)hChn->count2 + bfos_step);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BFOS, val);

            /* reset baud loop integrator */
            BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_BL_BLPCTL, 0x00000100);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BRI, 0);

            /* wait some time */
            hChn->functState = 1;
            return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaud, 300000, BSAT_g1_P_CheckTimingLoopStateMachine_isr);

         case 1:
            /* Narrow baud loop bandwidth */
            BSAT_CHK_RETCODE(BSAT_g1_P_SetBaudBw_isr(h, hChn->acqSettings.symbolRate / 1600, 11 * 4));

            sum = 0;
            for (i = 32; i; i--)
            {
               bri = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_BL_BRI);
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
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BFOS, hChn->count2);
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BL_BRI, 0);

            if (hChn->miscSettings.bPreserveCommandedTunerFreq == false)
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLIF, 0);

            if (sum > 0x20000)
            {
               if (hChn->count1)
               {
                  /* indicate timing loop locked */
                  hChn->bTimingLoopLock = true;
                  hChn->functState = 0xFFFFFFFF;
               }
               else
               {
                  hChn->count1 = 1;
                  hChn->functState = 0;
               }
            }
            else
               hChn->functState = 0xFFFFFFFF;
            break;

         default:
            break;
      }
   }

   retCode = hChn->nextFunct(h);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_CheckTimingLoop_isr() - sets bTimingLock if timing loop is locked
******************************************************************************/
static BERR_Code BSAT_g1_P_CheckTimingLoop_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT nextFunct)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   hChn->nextFunct = nextFunct;
   hChn->bTimingLoopLock = false;
   hChn->count2 = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_BL_BFOS);
   hChn->functState = 0;
   hChn->count1 = 0;
   return BSAT_g1_P_CheckTimingLoopStateMachine_isr(h);
}


/******************************************************************************
 BSAT_g1_P_SignalDetectModeExit_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_SignalDetectModeExit_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   hChn->signalDetectStatus.bDone = true;
   hChn->signalDetectStatus.bTimingLoopLocked = hChn->bTimingLoopLock;
   BSAT_g1_P_IndicateAcqDone_isr(h);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_SetAgcTrackingBw_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_SetAgcTrackingBw_isr(BSAT_ChannelHandle h)
{
   return BSAT_g1_P_ConfigChanAgc_isr(h, true);
}


/******************************************************************************
 BSAT_g1_P_InitBert_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_InitBert_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = 0x10A5;  /* disable clear-on-read */
   if (hChn->bertSettings.bInvert == false)
      val |= 0x0200;
   if (hChn->bertSettings.mode == BSAT_BertMode_ePRBS15)
      val |= 0x10000;
   else if (hChn->bertSettings.mode == BSAT_BertMode_ePRBS31)
      val |= 0x20000;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BERCTL, val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_StartBert_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_StartBert_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   if (hChn->acqSettings.options & BSAT_ACQ_ENABLE_BERT)
   {
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_BERT_BERCTL);
      val &= 0xFFFFFE00;

      /* enable BERT */
      if (hChn->xportSettings.bSerial)
         val |= 0xD0; /* serial */
      else
         val |= 0xC0; /* parallel */

      if ((hChn->xportSettings.bXbert) && (BSAT_MODE_IS_DTV(hChn->acqSettings.mode)))
      {
         val |= 0x08; /* XBERT */
      }

      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BERCTL, val);
      val |= 0x01;
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BERCTL, val);
      val |= 0x02;
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BERCTL, val);
      val &= ~1;
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BERCTL, val);
   }
    return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_ConfigOif_isr()
 input: opll_N, opll_D, outputBitRate
******************************************************************************/
BERR_Code BSAT_g1_P_ConfigOif_isr(BSAT_ChannelHandle h)
{
   /* set OI_OPLL, OI_OPLL2, OI_OIFCTL01, OI_OSUBD, OI_OPLL_NPDIV OI_OPLL_MDIV_CTRL, OI_OIFCTL00 */
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *hDev = h->pDevice->pImpl;
   uint32_t d_factor, compensated_d, compensated_d_n, byt_clk_sel, bit_clk_opll, val, ndiv_int,
            vco_ref_clock, vco_base, vco_residue, m1div, ndiv_frac, P_hi, P_lo, Q_hi, Q_lo,
            vco_freq, ki;

   if ((hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM) && ((hChn->acqSettings.options & BSAT_ACQ_CHAN_BOND) == 0))
   {
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OPLL, 0x80000000);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OPLL2, 0x80000000);

      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01);
      val &= ~BCHP_SDS_OI_0_OIFCTL01_byt_clk_sel_MASK;
      val &= ~BCHP_SDS_OI_0_OIFCTL01_out_data_dec_MASK;
      if (hChn->bUndersample)
      {
         val |= (3 << BCHP_SDS_OI_0_OIFCTL01_byt_clk_sel_SHIFT);
         val |= (3 << BCHP_SDS_OI_0_OIFCTL01_out_data_dec_SHIFT);
      }
      else
      {
         val |= (2 << BCHP_SDS_OI_0_OIFCTL01_byt_clk_sel_SHIFT);
         val |= (2 << BCHP_SDS_OI_0_OIFCTL01_out_data_dec_SHIFT);
      }
      val |= BCHP_SDS_OI_0_OIFCTL01_opll_byp_MASK; /* bypass opll */
      val |= BCHP_SDS_OI_0_OIFCTL01_oifbyp_MASK;
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01, val);
      goto continue_oifctl;
   }

   for (d_factor = 1; d_factor < 8; d_factor = d_factor << 1)
   {
      if ((hChn->opll_N << 1) < (hChn->opll_D * d_factor))
         break;
   }

   compensated_d = (uint32_t)(hChn->opll_D * (uint32_t)d_factor);
   compensated_d_n = compensated_d - hChn->opll_N;

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OPLL, hChn->opll_N);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OPLL2, compensated_d_n);

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

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01);
   val &= ~0x00001B00; /* clear bits 8, 9, 11, 12 */
   val |= byt_clk_sel;
   val |= (byt_clk_sel << 3); /* out_data_dec should be same as byt_clk_sel */
   val &= ~0x6000;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01, val);

   /* val = ceil(2000000 / hChn->outputBitRate); */
   val = ((2000000 / hChn->outputBitrate) + 1) & 0x0000001F;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OSUBD, val);
   bit_clk_opll = val * hChn->outputBitrate;
   BSAT_DEBUG_OI(BDBG_MSG(("bit_clk_opll=%u", bit_clk_opll)));

   if (hChn->xportSettings.bOpllBypass)
   {
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01, 0x14);
   }
   else
   {
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01, ~0x14);

      /* set OPLL_NPDIV, OPLL_MDIV_CTRL */
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_OPLL_NPDIV);
      val &= 0x0F;
      vco_ref_clock = hDev->xtalFreq / val;
      BSAT_DEBUG_OI(BDBG_MSG(("vco_ref_clock=%d", vco_ref_clock)));
#if BCHP_CHIP==45402
      for (ndiv_int = 14; ndiv_int <= 83; ndiv_int++)
#else
      for (ndiv_int = 14; ndiv_int <= 36; ndiv_int++)
#endif
      {
         BSAT_DEBUG_OI(BDBG_MSG(("ndiv_int=%d", ndiv_int)));
         vco_base = ndiv_int * vco_ref_clock;
         BSAT_DEBUG_OI(BDBG_MSG(("vco_base=%d", vco_base)));

         /* m1div = ceil(vco_base/bit_clk_opll) */
         m1div = vco_base / bit_clk_opll;
         if ((vco_base % bit_clk_opll) > 0)
            m1div++;

         if (m1div > 255)
            continue;

         vco_freq = m1div * bit_clk_opll;
         BSAT_DEBUG_OI(BDBG_MSG(("vco_freq=%d", vco_freq)));
#if BCHP_CHIP==45402
         if ((vco_freq < 1200000000UL) || (vco_freq > 4000000000UL))
#else
         if ((vco_freq < 500000000UL) || (vco_freq > 1600000000UL))
#endif
            continue;

         vco_residue = vco_freq - vco_base;
         BSAT_DEBUG_OI(BDBG_MSG(("vco_residue=%d", vco_residue)));

         if (vco_residue >= vco_ref_clock)
            continue;

         BMTH_HILO_32TO64_Mul(vco_residue, 1048576 << 1, &P_hi, &P_lo);
         BMTH_HILO_64TO64_Div32(P_hi, P_lo, vco_ref_clock, &Q_hi, &Q_lo);
         ndiv_frac = (Q_lo + 1) >> 1;
         BSAT_DEBUG_OI(BDBG_MSG(("ndiv_frac=%u", ndiv_frac)));

         val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_OPLL_NPDIV);
         val &= 0x0000000F; /* retain pdiv */
         val |= ((ndiv_int << 24) & 0xFF000000);
         val |= ((ndiv_frac << 4) & 0x00FFFFF0);
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OPLL_NPDIV, val);
         BSAT_DEBUG_OI(BDBG_MSG(("SDS_OI_OPLL_NPDIV=0x%X", val)));
         BSAT_DEBUG_OI(BDBG_MSG(("read SDS_OI_OPLL_NPDIV=0x%X", BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_OPLL_NPDIV))));

#if BCHP_CHIP==45402
         if ((vco_freq >= 1200000000UL) && (vco_freq < 2200000000UL))
            ki = 4;
         else if ((vco_freq >= 2200000000UL) && (vco_freq < 4000000000UL))
            ki = 3;
         else if (vco_freq >= 4000000000UL)
            ki = 2;
         else
            continue;
#else
         if (vco_freq < 1000000000UL)
            ki = 3;
         else
            ki = 2;
#endif
         val = ((ki+3)<<17) | (ki<<14) | m1div;
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OPLL_MDIV_CTRL, val);
         BSAT_DEBUG_OI(BDBG_MSG(("SDS_OI_OPLL_MDIV_CTRL=0x%X", val)));
         break;
      }

#if BCHP_CHIP==45402
      BKNI_Delay(800000000 / hChn->acqSettings.symbolRate + 1); /* wait 800 baud clks */
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_OI_OPLL_PWRDN_RST, ~1); /* areset=0 */
      BKNI_Delay(800000000 / hChn->acqSettings.symbolRate + 1); /* wait 800 baud clks */
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_OI_OPLL_PWRDN_RST, ~2); /* dreset=0 */
      BKNI_Delay(100000000 / hChn->acqSettings.symbolRate + 1); /* wait 100 baud clks */
#endif

      /* enable the loop */
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01);
      val |= 0x400; /* BCHP_SDS_OI_OIFCTL01_loop_en_MASK */
#if BCHP_CHIP==45402
      val &= ~BCHP_SDS_OI_0_OIFCTL01_out_data_dec_MASK; /* dont decimate */
#endif
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL01, val);
   }

   continue_oifctl:
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
   if (BSAT_MODE_IS_TURBO(hChn->actualMode))
      val |= 0x8100;
   else if (BSAT_MODE_IS_DVBS2(hChn->actualMode) || BSAT_MODE_IS_DVBS2X(hChn->actualMode))
      val |= 0x8000;
   if (hChn->xportSettings.bDataInv)
      val |= 0x0800;
   if ((hChn->xportSettings.bXbert) && (hDev->sdsRevId < 0x74))
      val |= 0x0400;
   if (hChn->xportSettings.bDelHeader)
      val |= 0x800000;
   if (hChn->xportSettings.bHead4)
      val |= 0x400000;
   if (hChn->xportSettings.bSync1)
      val |= 0x200000;
#if ((BCHP_CHIP==45308) && (BSAT_CHIP_FAMILY==45316)) || (BCHP_CHIP==45402)
   if (BSAT_MODE_IS_LEGACY_QPSK(hChn->actualMode))
   {
      if ((h->channel & 1) == 0)
      {
         /* Legacy QPSK on even channel */
         val |= 0x100; /* tfec_afec_sel=1 */
      }
   }
#endif
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL00, val);
   val &= ~1; /* toggle OIF reset */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL00, val);

#ifdef BCHP_SDS_OI_0_OIFCTL02_delh_bert_MASK
   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL02);
   val &= ~(BCHP_SDS_OI_0_OIFCTL02_delh_bert_MASK | BCHP_SDS_OI_0_OIFCTL02_oifxb_xbert_MASK | BCHP_SDS_OI_0_OIFCTL02_delh_xbert_MASK);
   val |= BCHP_SDS_OI_0_OIFCTL02_clksup_xbert_MASK; /* per Bainan */

   if (hChn->acqSettings.options & BSAT_ACQ_ENABLE_BERT)
   {
      val |= BCHP_SDS_OI_0_OIFCTL02_delh_bert_MASK;
   }
   if (hChn->xportSettings.bXbert)
   {
      val |= BCHP_SDS_OI_0_OIFCTL02_delh_xbert_MASK;
      if (!(BSAT_MODE_IS_DTV(hChn->actualMode)))
         val |= BCHP_SDS_OI_0_OIFCTL02_oifxb_xbert_MASK;
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_OI_OIFCTL02, val);
#endif

   return BERR_SUCCESS;
}



/******************************************************************************
 BSAT_g1_P_IsValidFreq()
******************************************************************************/
bool BSAT_g1_P_IsValidFreq(uint32_t freq)
{
   if ((freq < 250000000UL) || (freq > 2450000000UL))
      return false;
   return true;
}


/******************************************************************************
 BSAT_g1_P_IsValidSymbolRate()
******************************************************************************/
bool BSAT_g1_P_IsValidSymbolRate(uint32_t Fb)
{
   if ((Fb < 1000000UL) ||
#if (BCHP_CHIP != 45308)
       (Fb > 45000000UL)
#else
       (Fb > 52000000UL)
#endif
       )
      return false;
   return true;
}


/******************************************************************************
 BSAT_g1_P_GetNumDecimatingFilters_isr()
******************************************************************************/
uint32_t BSAT_g1_P_GetNumDecimatingFilters_isr(BSAT_ChannelHandle h)
{
   uint32_t filtctl, i, n = 0;

   n = 0;
   filtctl = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_FE_FILTCTL);
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


/******************************************************************************
 BSAT_g1_P_NonLegacyModeAcquireInit_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_NonLegacyModeAcquireInit_isr(BSAT_ChannelHandle h)
{
   BERR_Code retCode;
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t bw;

   BSAT_CHK_RETCODE(BSAT_g1_P_SetFfeMainTap_isr(h));
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FE_MIXCTL, 3);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRCTL, 0x03);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLTD, 0x28000000);
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, 0x0000000C);

   if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, 0x00000010);

   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 0x00FF00FF, 0x03000600); /* EQMU=0x03,EQFFE2=0x06 */
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, 0x000000FF, 0x00140400); /* EQMISC=0x14, EQBLND=0x04 */
#ifdef BCHP_SDS_FEC_FERR
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FERR, 0x00000000);
#else
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_CSYM, 0x00000000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_BBLK, 0x00000000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_TSYM, 0x00000000);
#endif
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLFBCTL, 0xFFFF0000, 0x02); /* fwd loop frz, backward loop frz */
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, 0xFFFF00FF, 0x0000DB00);

#if 0
   BSAT_g1_P_SetCarrierBw_isr(h, 25000, 2*4); /* updated by Xiaofen */
#else
   if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode) && (hChn->turboSettings.ctl & BSAT_TURBO_CTL_OVERRIDE_FLBW))
      bw = hChn->turboSettings.flbwOverride;
   else
      bw = ((hChn->acqSettings.symbolRate / 1000) * 104) / 100;

   BSAT_g1_P_SetCarrierBw_isr(h, bw, 2*4);
#endif

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_OnReacqTimerExpired_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_OnReacqTimerExpired_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BSAT_DEBUG_ACQ(BDBG_WRN(("BSAT_g1_P_OnReacqTimerExpired_isr(%d)", h->channel)));

#ifdef BSAT_HAS_ACM
   if (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM)
   {
      uint32_t lock_status = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFEC_LOCK_STATUS);
      if (lock_status & BCHP_AFEC_0_LOCK_STATUS_LDPC_LOCK_PLS_MASK)
      {
         /* at least 1 stream is locked, so just declare lock */
         BSAT_DEBUG_ACQ(BDBG_ERR(("detected at least 1 stream is locked (AFEC_LOCK_STATUS=0x%X)", lock_status)));
         BSAT_g1_P_GetAcquisitionTimerValue_isr(h, &(hChn->configParam[BSAT_g1_CONFIG_ACQ_TIME]));
         return BSAT_g1_P_OnStableLock_isr(h);
      }
   }
#endif

   BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->EnableLockInterrupts(h, false);
   hChn->bReacqTimerExpired = true;
   hChn->reacqCause = BSAT_ReacqCause_eFecNotStableLock;
   return BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);
}


/******************************************************************************
 BSAT_g1_P_StartReacqTimer_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_StartReacqTimer_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)(h->pImpl);
   uint32_t reacquisition_timeout, P_hi, P_lo, Q_hi, min_timeout;

   /* compute the reacquisition timeout */
   if (BSAT_MODE_IS_TURBO(hChn->actualMode))
      min_timeout = 500000;
   else
      min_timeout = 400000;

   if (hChn->acqSettings.symbolRate >= 20000000)
      reacquisition_timeout = min_timeout;
   else
   {
      BMTH_HILO_32TO64_Mul(20000000, min_timeout, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqSettings.symbolRate, &Q_hi, &reacquisition_timeout);
   }

   hChn->bReacqTimerExpired = false;
   /* BSAT_DEBUG_ACQ(BDBG_ERR(("setting reacq timer to %d usecs", reacquisition_timeout))); */
   return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eReacqTimer, reacquisition_timeout, BSAT_g1_P_OnReacqTimerExpired_isr);
}


/******************************************************************************
 BSAT_g1_P_PrepareNewAcquisition() - non-ISR context
******************************************************************************/
BERR_Code BSAT_g1_P_PrepareNewAcquisition(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   /* reset last locked status */
   hChn->bLastLocked = false;

   BSAT_CHK_RETCODE(BSAT_g1_P_ResetChannel(h, true));

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FECTL, 1);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FECTL, 0);

   BSAT_CHK_RETCODE(BSAT_g1_P_ResetChannelStatus(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_ResetSignalNotification_isrsafe(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_ResetSignalDetectStatus(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_ResetSymbolRateScanStatus(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_ResetToneDetectStatus(h));
   BSAT_g1_P_ClearTraceBuffer(h);
   hChn->bAcqDone = false;
   hChn->bEverStableLock = false;
   hChn->bStableLock = false;
   hChn->bBlindScan = false;
   hChn->reacqCause = BSAT_ReacqCause_eOK;
   hChn->actualMode = BSAT_Mode_eUnknown;
   hChn->initFreqOffset = 0;
   hChn->bEnableFineFreqEst = false;
   hChn->dvbs2ScanState = 0;
#ifndef BSAT_EXCLUDE_TFEC
   hChn->turboScanCurrMode = 0;
   hChn->turboScanLockedModeFailures = 0;
#endif

   hDev->bResetDone = false;

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_StartBackgroundAcq() - Non ISR
******************************************************************************/
BERR_Code BSAT_g1_P_StartBackgroundAcq(BSAT_ChannelHandle h)
{
   BERR_Code retCode;
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)(h->pImpl);

   BSAT_CHK_RETCODE(BSAT_g1_P_SetAdcSelect(h, hChn->acqSettings.adcSelect));

   hChn->bAbortAcq = false;
   retCode = BSAT_g1_P_EnableTimer(h, BSAT_TimerSelect_eBaudUsec, 30, BSAT_g1_P_Acquire0_isr);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_Acquire0_isr() - ISR context
******************************************************************************/
BERR_Code BSAT_g1_P_Acquire0_isr(BSAT_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)(h->pImpl);

   /* set some registers to their power-up default values */
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_CG_RSTCTL, 1);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONFIG, 0x3F8F0F80);

   BSAT_g1_P_ResetLockFilter_isr(h);
   BSAT_g1_P_ResetAcquisitionTimer_isr(h);

#ifndef BSAT_EXCLUDE_TFEC
   hChn->turboScanState = BSAT_TURBO_SCAN_STATE_FIRST_PASS | BSAT_TURBO_SCAN_STATE_FIRST_ACQ;
   hChn->turboScanCurrMode = hChn->turboScanLockedMode;
#endif

   BSAT_g1_P_DetermineUndersampleMode_isr(h);
#ifndef BSAT_EXCLUDE_SPUR_CANCELLER
   BSAT_CHK_RETCODE(BSAT_g1_P_CwcDisable_isr(h));
#endif

   if (hChn->operation == BSAT_Operation_eAcq)
   {
      if (hChn->acqSettings.mode == BSAT_Mode_eBlindScan)
         retCode = BSAT_g1_P_BlindScanInit_isr(h);
      else
         BSAT_g1_P_SetAcqType_isr(h);
      if (retCode != BERR_SUCCESS)
         goto done;
   }

   hChn->tunerIfStep = 0;
   retCode = BSAT_g1_P_TunerSetFreq_isr(h, BSAT_g1_P_Acquire1_isr);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_Acquire1_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_Acquire1_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

/* bEnableDebugLog = true; */
   hChn->acqState = BSAT_AcqState_eAcquiring;
   hChn->timeSinceStableLock = 0;
   hChn->relockCount = 0;
   hChn->bForceReacq = false;

   BSAT_CHK_RETCODE(BSAT_g1_P_GetActualMode_isr(h, &(hChn->actualMode)));
   retCode = BSAT_g1_P_Acquire2_isr(h);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_InitEqTaps_isr()
******************************************************************************/
static void BSAT_g1_P_InitEqTaps_isr(BSAT_ChannelHandle h)
{
   uint32_t ffe_main_tap, i, val, eqcfad, f0b;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL);
   ffe_main_tap = (val >> 16) & 0x1F;
   for (i = 0; i < 24; i++)
   {
      val = 0x00;
      if (i == ffe_main_tap)
         val = 0x25; /* changed by Xiaofen to be same as dvbs2 (orig is 0x12) */

      eqcfad = 0x40 | i;
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQCFAD, eqcfad);

      f0b = (val & 0xFF) << 24;
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_F0B, f0b);
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQCFAD, 0);
}


/******************************************************************************
 BSAT_g1_P_Acquire2_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_Acquire2_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, val2;
   BERR_Code retCode;

   BSAT_CHK_RETCODE(BSAT_g1_P_SetDecimationFilters_isr(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_SetBfos_isr(h));
   BSAT_CHK_RETCODE(BSAT_g1_P_ConfigFe_isr(h));

   if (hChn->acqSettings.options & BSAT_ACQ_TUNER_TEST_MODE)
   {
      hChn->acqSettings.mode = BSAT_Mode_eUnknown;
      hChn->acqState = BSAT_AcqState_eIdle;
      return BERR_SUCCESS; /* exit the acquisition */
   }

   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLFBCTL, 0xFFFF0000, 0x00000002); /* turn off fwd-bckwd loop */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, 0x00010001);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VCOS, 0x00004000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRCTL, 0x01);
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, 0x00FFFFFF); /* CLOON=0x00 */
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, 0x00004000); /* reset viterbi block */
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, 0xFFFF00FF); /* VTCTL2=0x00 */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FECTL, 0x01);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FECTL, 0x00);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 0x22000250); /* need to unfreeze EQ for Legacy, was 0x00000740 */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VLCI, 0x08000000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VLCQ, 0x08000000);
#ifdef BCHP_SDS_EQ_0_ACM_FIFO
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_ACM_FIFO, 0x0000100C); /* acm_fifo_byp=1, buf_delay=0xC */
#endif

   val2 = 0x00000003;
   if (BSAT_MODE_IS_DVBS2(hChn->acqSettings.mode) || (BSAT_MODE_IS_DVBS2X(hChn->acqSettings.mode)))
      val = 0x0C801063; /* per Xiaofen */
   else if (BSAT_MODE_IS_TURBO(hChn->acqSettings.mode))
      val = 0x0C881073;
   else
      val = 0x08881073;

#ifdef BSAT_HAS_CONTEXT_SWITCH
   if (BSAT_MODE_IS_LEGACY_QPSK(hChn->acqSettings.mode))
   {
      val = 0x08801063;   /* bit4 clen=0, bit19 cl_en_rcvr_lf=0 */
      val2 = 0x00001001;
   }
#endif /* BSAT_HAS_CONTEXT_SWITCH */

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, val);
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, ~0x00000003); /* clear carrier loop reset bit */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, val2);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLFFCTL, 0x02);  /* bypass fine mixer */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_PLDCTL, 0x0000800a);   /* bypass rescrambler/descrambler */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLC, 0x06080518);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLTD, 0x20000000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLI, 0x00000000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLTD, 0x20000000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLI, 0x00000000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLPA, 0x00000000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HDQPSK, 0x01000000);

   BSAT_CHK_RETCODE(BSAT_g1_P_SetFfeMainTap_isr(h));

   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 1); /* reset the eq */
   BSAT_g1_P_InitEqTaps_isr(h);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLLC, 0x01000100);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLIC, 0x01000000);

   if (hChn->operation == BSAT_Operation_ePsd)
      return BSAT_g1_P_DftPsdScan_isr(h);
   else if (hChn->operation == BSAT_Operation_eToneDetect)
      return BSAT_g1_P_DftToneDetect_isr(h);
   else if (hChn->operation == BSAT_Operation_eSymbolRateScan)
      return BSAT_g1_P_DftSymbolRateScan_isr(h);

   BSAT_CHK_RETCODE(BSAT_g1_P_ConfigPlc_isr(h, true)); /* set acquisition plc */

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1);
   val &= 0xFFFFFF10;
   val |= 0x0000000F;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, val);
   val &= 0xFFFFFF10;
   val |= 0x0000000C;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, val);

   /* enable I/Q phase detector, enable baud recovery loop */
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_BL_BLPCTL, ~0x000000FF, 0x00000007);

   retCode = BSAT_g1_P_DftSearchCarrier_isr(h, BSAT_g1_P_Acquire3_isr);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_Acquire3_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_Acquire3_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, ~0x000000EF, 0x00000003);
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, ~0x000000EF, 0x00000000);
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, ~0x0000FF00, 0x00007000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, 0x00000400);
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_BL_BLPCTL, ~0x000000F7, 0x00000007);

   if (hChn->acqSettings.options & BSAT_ACQ_OQPSK)
   {
      BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_BL_BLPCTL, 0xFFFFFFF0, 0x0B);
   }

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VLCTL, 0x0000F185);
#ifdef BCHP_SDS_FEC_FERR
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FERR, 0x00000000);
#else
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_CSYM, 0x00000000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_BBLK, 0x00000000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_TSYM, 0x00000000);
#endif
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_PILOTCTL, 0x0);

   if (hChn->operation == BSAT_Operation_eSignalDetect)
   {
      /* signal detect mode */
      retCode = BSAT_g1_P_CheckTimingLoop_isr(h, BSAT_g1_P_SignalDetectModeExit_isr);
   }
   else if (hChn->miscSettings.bCheckTimingLoop)
      retCode = BSAT_g1_P_CheckTimingLoop_isr(h, BSAT_g1_P_Acquire4_isr);
   else
   {
      hChn->bTimingLoopLock = true;
      if (BSAT_MODE_IS_LEGACY_QPSK(hChn->acqSettings.mode) && (hChn->acqSettings.symbolRate <= 2370000))
      {
         /* need to delay for low baud rates */
         retCode = BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaud, 300000, BSAT_g1_P_Acquire4_isr);
      }
      else
         retCode = BSAT_g1_P_Acquire4_isr(h);
   }

   return retCode;
}


/******************************************************************************
 BSAT_g1_P_Acquire4_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_Acquire4_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (hChn->bTimingLoopLock)
   {
      hChn->count1 = hChn->count2 = 0;
      return BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Acquire(h);
   }

   BDBG_WRN(("timing loop not locked"));
   hChn->reacqCause = BSAT_ReacqCause_eTimingLoopNotLocked;
   return BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);
}


/******************************************************************************
 BSAT_g1_P_StartTracking_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_StartTracking_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)(h->pImpl);
   BERR_Code retCode;
   bool bValid;

   BSAT_CHK_RETCODE(BSAT_g1_P_LogTraceBuffer_isr(h, BSAT_TraceEvent_eStartTracking));
   hChn->trace[BSAT_TraceEvent_eInitialLock] = 0;
   hChn->trace[BSAT_TraceEvent_eStableLock] = 0;
   /* BSAT_DEBUG_ACQ(BDBG_ERR(("BSAT_g1_P_StartTracking_isr(%d)", h->channel))); */

   /* stop the timer that was started before DFT */
   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eGen2);

   bValid = BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->IsValidMode(h);
   if (!bValid)
   {
      BDBG_WRN(("BSAT_g1_P_StartTracking_isr(): invalid mode"));
      hChn->reacqCause = BSAT_ReacqCause_eInvalidMode;
      return BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);
   }

   BSAT_CHK_RETCODE(BSAT_g1_P_StartBert_isr(h));

   hChn->acqState = BSAT_AcqState_eWaitForInitialLock;

   if (hChn->acqSettings.options & BSAT_ACQ_DISABLE_TRACKING)
   {
      retCode = BSAT_g1_P_DisableChannelInterrupts_isr(h);
      BDBG_WRN(("aborting acquisition"));
      goto done;
   }

   /* enable lock/lost_lock interrupts */
   hChn->lockIsrFlag = 0;
   BSAT_CHK_RETCODE(BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->EnableLockInterrupts(h, true));
   BSAT_CHK_RETCODE(BSAT_g1_P_StartReacqTimer_isr(h));

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_Lock_isr() - callback routine for not_locked->locked transition
******************************************************************************/
void BSAT_g1_P_Lock_isr(void *p, int int_id)
{
   BERR_Code retCode;
   BSAT_ChannelHandle h = (BSAT_ChannelHandle)p;
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   BSAT_DEBUG_ACQ(BDBG_ERR(("BSAT_g1_P_Lock_isr(%d), lockIsrFlag=%d", h->channel, hChn->lockIsrFlag)));

   if (BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->IsLocked)
   {
      if (BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->IsLocked(h) == false)
      {
         BSAT_DEBUG_ACQ(BDBG_ERR(("BSAT_g1_P_Lock_isr(%d): false lock detected", h->channel)));
         if (hChn->lockIsrFlag == 1)
            BSAT_g1_P_NotLock_isr(p, int_id+1);
         return;
      }
   }

   if (hChn->lockIsrFlag == 1)
   {
      /* BDBG_MSG(("chan%d ignoring lock isr", h->channel)); */
      return;
   }
   hChn->lockIsrFlag = 1;

   BSAT_g1_P_GetAcquisitionTimerValue_isr(h, &(hChn->configParam[BSAT_g1_CONFIG_ACQ_TIME]));

   if (hChn->trace[BSAT_TraceEvent_eInitialLock] == 0)
   {
      retCode = BSAT_g1_P_LogTraceBuffer_isr(h, BSAT_TraceEvent_eInitialLock);
      if (retCode != BERR_SUCCESS)
      {
         BDBG_WRN(("BSAT_g1_P_Lock_isr(): BSAT_g1_P_LogTraceBuffer_isr() error 0x%X", retCode));
      }
   }

   if (hChn->miscSettings.bForceUnlockOutsideSearchRange)
   {
      if (BSAT_g1_P_IsCarrierOffsetOutOfRange_isr(h))
      {
         hChn->bForceReacq = true;
         hChn->reacqCause = BSAT_ReacqCause_eCarrierOffsetOutOfRange;
         goto done;
      }
   }

   BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->OnLock(h);

   done:
   if (hChn->bForceReacq)
      BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);
   else
   {
      hChn->acqState = BSAT_AcqState_eWaitForStableLock;
      /* BSAT_DEBUG_ACQ(BDBG_ERR(("starting stable lock timer (%d usecs)", hChn->stableLockTimeout))); */
      BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eStableLockTimer, hChn->stableLockTimeout, BSAT_g1_P_OnStableLock_isr);
   }
}


/******************************************************************************
 BSAT_g1_P_NotLock_isr() - callback routine for locked->not_locked transition
******************************************************************************/
void BSAT_g1_P_NotLock_isr(void *p, int int_id)
{
   BSAT_ChannelHandle h = (BSAT_ChannelHandle)p;
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   bool bStartReacqTimer;

   BSAT_DEBUG_ACQ(BDBG_ERR(("BSAT_g1_P_NotLock_isr(%d), lockIsrFlag=%d", h->channel, hChn->lockIsrFlag)));

   if (BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->IsLocked)
   {
      if (BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->IsLocked(h) == true)
      {
         if (hChn->lockIsrFlag == 0)
            BSAT_g1_P_Lock_isr(p, int_id-1);
         return;
      }
   }

   hChn->configParam[BSAT_g1_CONFIG_ACQ_TIME] = 0;
   if (hChn->lockIsrFlag == 0)
   {
      /* BDBG_MSG(("chan%d ignoring not_lock isr", h->channel)); */
      return;
   }
   hChn->lockIsrFlag = 0;

   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eStableLockTimer);

   BSAT_g1_P_IndicateNotLocked_isrsafe(h);

   if (hChn->bReacqTimerExpired)
      goto reacquire;

   if ((hChn->acqState == BSAT_AcqState_eMonitorLock) ||
       (BSAT_g1_P_IsTimerRunning_isr(h, BSAT_TimerSelect_eReacqTimer) == false))
      bStartReacqTimer = true;
   else
      bStartReacqTimer = false;

   hChn->acqState = BSAT_AcqState_eWaitForInitialLock;

   /* extend stable lock timeout */
   if ((hChn->reacqCount> 0) || hChn->bEverStableLock)
   {
      hChn->stableLockTimeout += (((hChn->lockFilterRamp > 0) ? hChn->lockFilterRamp : 1) * BSAT_G1_LOCK_FILTER_INCR);
      if (hChn->lockFilterRamp < 32)
         hChn->lockFilterRamp++;
      if (hChn->stableLockTimeout > hChn->maxStableLockTimeout)
         hChn->stableLockTimeout = hChn->maxStableLockTimeout;

      /* BDBG_ERR(("NotLock_isr: stableLockTimeout=%d, maxStableLockTimeout=%d, lockFilterRamp=%d", hChn->stableLockTimeout, hChn->maxStableLockTimeout, hChn->lockFilterRamp)); */
   }

   if (BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->OnLostLock)
      BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->OnLostLock(h);

   if (hChn->bForceReacq || (hChn->relockCount > 8))
   {
      reacquire:
      BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);
      return;
   }

   hChn->bStableLock = false;

   if (bStartReacqTimer)
   {
      BSAT_g1_P_StartReacqTimer_isr(h);
   }
}


/******************************************************************************
 BSAT_g1_P_OnStableLock_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_OnStableLock_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)(h->pImpl);
   BERR_Code retCode;

   if (hChn->acqSettings.mode != BSAT_Mode_eDvbs2_ACM)
   {
      if (BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->IsLocked)
      {
         if (BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->IsLocked(h) == false)
         {
            BSAT_DEBUG_ACQ(BDBG_ERR(("BSAT_g1_P_OnStableLock_isr(%d): false lock detected", h->channel)));
            hChn->reacqCause = BSAT_ReacqCause_eFecNotStableLock;
            return BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);
         }
      }
   }

   if ((hChn->acqSettings.mode != BSAT_Mode_eDvbs2_ACM) && (hChn->acqType == BSAT_AcqType_eDvbs2))
   {
      /* wait until mpeg is locked */
      if (BSAT_g1_P_AfecIsMpegLocked_isr(h) == false)
         return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eStableLockTimer, 1000, BSAT_g1_P_OnStableLock_isr);
   }

   /* reset SNORE */
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_SNR_SNORECTL, ~0x80);
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_SNR_SNORECTL, 0x80);

   if (hChn->bEverStableLock == false)
   {
      /* reset the mpeg counters on first lock */
      BSAT_g1_P_ResetMpegCount_isrsafe(h);
   }

   BSAT_CHK_RETCODE(BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eReacqTimer));
   BSAT_CHK_RETCODE(BSAT_g1_P_LogTraceBuffer_isr(h, BSAT_TraceEvent_eStableLock));
   BSAT_DEBUG_ACQ(BDBG_ERR(("BSAT_g1_P_OnStableLock_isr(%u): t=%d", h->channel, hChn->configParam[BSAT_g1_CONFIG_ACQ_TIME])));
   BSAT_CHK_RETCODE(BSAT_g1_P_GetCarrierError_isrsafe(h, &(hChn->initFreqOffset)));
   hChn->relockCount++;

   BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->OnStableLock(h);
   if (hChn->bForceReacq)
      return BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);

   hChn->acqState = BSAT_AcqState_eMonitorLock;
   BSAT_g1_P_IndicateLocked_isrsafe(h);
   BSAT_g1_P_IndicateAcqDone_isr(h);
   hChn->bEverStableLock = true;
   hChn->bStableLock = true;
   hChn->freqTransferInt = 0;
   hChn->timeSinceStableLock = 0;
   hChn->count1 = 0;

#ifndef BSAT_EXCLUDE_SPUR_CANCELLER
   retCode = BSAT_g1_P_CwcInit_isr(h, BSAT_g1_P_OnStableLock1_isr);
#else
   retCode = BSAT_g1_P_OnStableLock1_isr(h);
#endif
   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_OnStableLock1_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_OnStableLock1_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;

   BSAT_CHK_RETCODE(BSAT_g1_P_OnMonitorLock_isr(h));
   if (hChn->bForceReacq)
      retCode = BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_OnMonitorLock_isr() - scheduled every 100 msecs
******************************************************************************/
static BERR_Code BSAT_g1_P_OnMonitorLock_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)(h->pImpl);
   BERR_Code retCode;

   if (hChn->timeSinceStableLock == 4)
   {
#ifndef BSAT_EXCLUDE_SPUR_CANCELLER
      BSAT_CHK_RETCODE(BSAT_g1_P_CwcReset_isr(h));
#endif
   }

   if (BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->OnMonitorLock)
      BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->OnMonitorLock(h);
   if (hChn->bForceReacq)
      goto force_reacquire;

   if (hChn->miscSettings.bForceUnlockOutsideSearchRange)
   {
      if (BSAT_g1_P_IsCarrierOffsetOutOfRange_isr(h))
      {
         hChn->bForceReacq = true;
         hChn->reacqCause = BSAT_ReacqCause_eCarrierOffsetOutOfRange;

         force_reacquire:
         return BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);
      }
   }

   if ((hChn->timeSinceStableLock % 4) == 0)
   {
      retCode = BSAT_g1_P_UpdateNotch_isr(h);
      if (retCode != BERR_SUCCESS)
      {
         BDBG_WRN(("BSAT_g1_P_OnMonitorLock_isr(): BSAT_g1_P_UpdateNotch_isr() error 0x%X", retCode));
      }
   }

   retCode = BSAT_g1_P_LeakPliToFli_isr(h);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_WRN(("BSAT_g1_P_OnMonitorLock_isr(): BSAT_g1_P_LeakPliToFli_isr() error 0x%X", retCode));
   }

   BSAT_g1_P_CheckSignalNotification_isr(h);

   hChn->timeSinceStableLock++;
   if (hChn->timeSinceStableLock > 100)
      BSAT_g1_P_ResetLockFilter_isr(h);

   retCode = BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eMonitorLockTimer, 100000, BSAT_g1_P_OnMonitorLock_isr);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_Reacquire_isr() - ISR context
******************************************************************************/
BERR_Code BSAT_g1_P_Reacquire_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   bool bStopAcq = false;

   BSAT_CHK_RETCODE(BSAT_g1_P_LogTraceBuffer_isr(h, BSAT_TraceEvent_eReacquire));
   BSAT_DEBUG_ACQ(BDBG_ERR(("BSAT_g1_P_Reacquire_isr(%d)", h->channel)));
   hChn->bStableLock = false;
   BSAT_g1_P_IndicateNotLocked_isrsafe(h);

   /* disable lock/lost_lock interrupts */
   BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->EnableLockInterrupts(h, false);
   BSAT_g1_P_DisableChannelInterrupts_isr(h);

   if ((hChn->acqSettings.options & BSAT_ACQ_DISABLE_REACQ) && hChn->miscSettings.bPreserveState)
      goto go_idle;

   if (hChn->miscSettings.maxReacqs > 0)
   {
      if (hChn->reacqCount >= hChn->miscSettings.maxReacqs)
      {
         bStopAcq = true;
         if (hChn->miscSettings.bPreserveState)
            goto go_idle;
      }
   }

#ifndef BSAT_EXCLUDE_TFEC
   if (hChn->acqType == BSAT_AcqType_eTurbo)
   {
#ifdef BSAT_HAS_DUAL_TFEC
      if (BSAT_g1_P_TfecIsOtherChannelBusy_isrsafe(h) == false)
#endif
         BSAT_g1_P_TfecPowerDown_isrsafe(h);
   }
#endif

   BSAT_g1_P_PowerDownOpll_isrsafe(h);
   BSAT_CHK_RETCODE(BSAT_g1_P_HpEnable_isr(h, false));
   BSAT_CHK_RETCODE(BSAT_g1_P_ResetSignalNotification_isrsafe(h));
#ifndef BSAT_EXCLUDE_SPUR_CANCELLER
   BSAT_CHK_RETCODE(BSAT_g1_P_CwcDisable_isr(h));
#endif

   if (bStopAcq)
      goto go_idle;

   if (hChn->bBlindScan)
   {
#ifndef BSAT_EXCLUDE_TFEC
      if ((hChn->acqSettings.mode != BSAT_Mode_eTurbo_scan) ||
          ((hChn->acqSettings.mode == BSAT_Mode_eTurbo_scan) && (hChn->turboScanCurrMode == 0)))
#endif
      {
         BSAT_CHK_RETCODE(BSAT_g1_P_BlindScanSetNextMode_isr(h));
      }
   }
   else
   {
      if (hChn->acqSettings.options & BSAT_ACQ_DISABLE_REACQ)
      {
         go_idle:
         hChn->acqState = BSAT_AcqState_eIdle;
         BDBG_MSG(("failed to acquire"));
         return retCode;
      }

      if (hChn->acqSettings.mode != BSAT_Mode_eTurbo_scan)
         BSAT_g1_P_IncrementReacqCount_isr(h);
   }

   if (hChn->miscSettings.bDontRetuneOnReacquire)
      retCode = BSAT_g1_P_Acquire1_isr(h);
   else
   {
      hChn->tunerIfStep = 0;
      retCode = BSAT_g1_P_TunerSetFreq_isr(h, BSAT_g1_P_Acquire1_isr);
   }

   done:
   return retCode;
}
