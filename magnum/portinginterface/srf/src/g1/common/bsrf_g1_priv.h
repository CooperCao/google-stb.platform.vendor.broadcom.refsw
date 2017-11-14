/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

 ******************************************************************************/
#ifndef BSRF_G1_PRIV_H__
#define BSRF_G1_PRIV_H__

#include "bsrf.h"
#include "bsrf_g1.h"

#if (BCHP_CHIP==lonestar)
   #include "bsrf_lonestar_priv.h"
   #define BSRF_SXM_OVERRIDE  /* sxm request to override default settings */
#elif (BCHP_CHIP==89730)
   #include "bsrf_89730_priv.h"
   #define BSRF_SXM_OVERRIDE  /* sxm request to override default settings */
#else
   #error "unsupported BCHP_CHIP"
#endif


#define BSRF_G1_RELEASE_VERSION 1


/******************************************************************************
Summary:
   This is the structure for the chip-specific component of the BSRF_Handle.
******************************************************************************/
typedef struct BSRF_g1_P_Handle
{
   BREG_Handle          hRegister;  /* register handle */
   BINT_Handle          hInterrupt; /* interrupt handle */
   int32_t              dacFreqHz;  /* test dac freq in Hz */
   int16_t              xsinxCoeff[BSRF_NUM_XSINX_COEFF];   /* output filter coeff */
} BSRF_g1_P_Handle;


/******************************************************************************
Summary:
   This is the structure for the chip-specific component of the BSRF_ChannelHandle.
******************************************************************************/
typedef struct BSRF_g1_P_ChannelHandle
{
   BINT_CallbackHandle  hAttackCountOvfCb;   /* callback handle for attack count overflow interrupt */
   BINT_CallbackHandle  hDecayCountOvfCb;    /* callback handle for decay count overflow interrupt */
   BINT_CallbackHandle  hFsCountOvfCb;       /* callback handle for fs count overflow interrupt */
   BINT_CallbackHandle  hWinDetectCb;        /* callback handle for window detect interrupt */
   BINT_CallbackHandle  hRampActiveCb;       /* callback handle for ramp active interrupt */
   BINT_CallbackHandle  hRampInactiveCb;     /* callback handle for ramp inactive interrupt */
   BSRF_RfAgcSettings   rfagcSettings;       /* rfagc settings */
   bool                 bOmitRfagcLut[BSRF_RFAGC_LUT_COUNT];   /* rfagc lut omissions */
   bool                 bEnableFastDecay;    /* enable fast decay mode */
   bool                 bAntennaSenseEnabled;   /* antenna sense enabled */
   uint32_t             tunerFreq;           /* tuner freq */
   int32_t              notchFreq;           /* notch freq */
   uint8_t              modeAoc;             /* antenna over-current mode */
   uint8_t              modeAd;              /* antenna detect mode */
   uint8_t              numRfagcLutOmitted;  /* number of rfagc lut elements omitted */
   int8_t               fastDecayGainThr;    /* fast decay gain threshold */
} BSRF_g1_P_ChannelHandle;


/* bsrf_g2_priv */
BERR_Code BSRF_g1_P_Open(BSRF_Handle *h, BCHP_Handle hChip, void *pReg, BINT_Handle hInt, const BSRF_Settings *pDefSettings);
BERR_Code BSRF_g1_P_Close(BSRF_Handle h);
BERR_Code BSRF_g1_P_GetTotalChannels(BSRF_Handle h, uint8_t *totalChannels);
BERR_Code BSRF_g1_P_OpenChannel(BSRF_Handle h, BSRF_ChannelHandle *pChannelHandle, uint8_t chanNum, const BSRF_ChannelSettings *pSettings);
BERR_Code BSRF_g1_P_CloseChannel(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_P_Reset(BSRF_Handle h);
BERR_Code BSRF_g1_P_GetVersion(BSRF_Handle h, BFEC_VersionInfo *pVersion);
BERR_Code BSRF_g1_P_PowerUp(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_P_PowerDown(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_P_FreezeRfAgc(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_P_UnfreezeRfAgc(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_P_WriteRfAgc(BSRF_ChannelHandle h, uint32_t val);
BERR_Code BSRF_g1_P_ReadRfAgc(BSRF_ChannelHandle h, uint32_t *pVal);
BERR_Code BSRF_g1_P_WriteRfGain(BSRF_ChannelHandle h, uint8_t gain);
BERR_Code BSRF_g1_P_ReadRfGain(BSRF_ChannelHandle h, uint8_t *pGain);
BERR_Code BSRF_g1_P_GetInputPower(BSRF_ChannelHandle h, int32_t *pPower);
BERR_Code BSRF_g1_P_SetRfAgcSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings settings);
BERR_Code BSRF_g1_P_GetRfAgcSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings *pSettings);
BERR_Code BSRF_g1_P_EnableFastDecayMode(BSRF_ChannelHandle h, bool bEnable);
BERR_Code BSRF_g1_P_SetFastDecayGainThreshold(BSRF_ChannelHandle h, int8_t threshold);
BERR_Code BSRF_g1_P_GetFastDecayGainThreshold(BSRF_ChannelHandle h, int8_t *pThreshold);
BERR_Code BSRF_g1_P_SetAntennaOverThreshold(BSRF_ChannelHandle h, uint8_t mode);
BERR_Code BSRF_g1_P_GetAntennaOverThreshold(BSRF_ChannelHandle h, uint8_t *pMode);
BERR_Code BSRF_g1_P_SetAntennaDetectThreshold(BSRF_ChannelHandle h, uint8_t mode);
BERR_Code BSRF_g1_P_GetAntennaDetectThreshold(BSRF_ChannelHandle h, uint8_t *pMode);
BERR_Code BSRF_g1_P_GetAntennaStatus(BSRF_ChannelHandle h, BSRF_AntennaStatus *pStatus);
BERR_Code BSRF_g1_P_Tune(BSRF_ChannelHandle h, uint32_t freqHz);
BERR_Code BSRF_g1_P_GetTunerStatus(BSRF_ChannelHandle h, BSRF_TunerStatus *pStatus);
BERR_Code BSRF_g1_P_ResetClipCount(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_P_GetClipCount(BSRF_ChannelHandle h, uint32_t *pClipCount);
BERR_Code BSRF_g1_P_ConfigTestMode(BSRF_ChannelHandle h, BSRF_TestportSelect tp, bool bEnable);
BERR_Code BSRF_g1_P_ConfigOutput(BSRF_Handle h, BSRF_OutputSelect output, bool bSlewEdges, uint8_t driveStrength_ma);
BERR_Code BSRF_g1_P_ConfigTestDac(BSRF_Handle h, int32_t freqHz, int16_t *pCoeff);
BERR_Code BSRF_g1_P_EnableTestDac(BSRF_Handle h);
BERR_Code BSRF_g1_P_DisableTestDac(BSRF_Handle h);
BERR_Code BSRF_g1_P_EnableTestDacTone(BSRF_Handle h, bool bToneOn, uint16_t toneAmpl);
BERR_Code BSRF_g1_P_RunDataCapture(BSRF_Handle h);
BERR_Code BSRF_g1_P_DeleteAgcLutCodes(BSRF_Handle h, uint32_t *pIdx, uint32_t n);
BERR_Code BSRF_g1_P_ConfigOutputClockPhase(BSRF_Handle h, uint8_t phase, bool bDisableOutput);
BERR_Code BSRF_g1_P_SetIqEqCoeff(BSRF_ChannelHandle h, int16_t *iTaps, int16_t *qTaps);
BERR_Code BSRF_g1_P_SetIqEqSettings(BSRF_ChannelHandle h, BSRF_IqEqSettings settings);
void BSRF_g1_P_AttackCountOvf_isr(void *p, int param);
void BSRF_g1_P_DecayCountOvf_isr(void *p, int param);
void BSRF_g1_P_FsCountOvf_isr(void *p, int param);
void BSRF_g1_P_WinDetect_isr(void *p, int param);
void BSRF_g1_P_RampActive_isr(void *p, int param);
void BSRF_g1_P_RampInactive_isr(void *p, int param);

/* bsrf_g1_priv_ana */
BERR_Code BSRF_g1_Ana_P_PowerUp(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_Ana_P_PowerDown(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_Ana_P_PowerUpAntennaSense(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_Ana_P_PowerDownAntennaSense(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_Ana_P_CalibrateCaps(BSRF_ChannelHandle h);

/* bsrf_g1_priv_rfagc */
BERR_Code BSRF_g1_Rfagc_P_Init(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_Rfagc_P_SetSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings settings);

/* bsrf_g1_priv_tuner */
BERR_Code BSRF_g1_Tuner_P_Init(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_Tuner_P_SetFcw(BSRF_ChannelHandle h, int32_t freqHz);
BERR_Code BSRF_g1_Tuner_P_SetNotchFcw(BSRF_ChannelHandle h, int32_t freqHz);
BERR_Code BSRF_g1_Tuner_P_SetIqEqCoeff(BSRF_ChannelHandle h, int16_t *iTaps, int16_t *qTaps);
BERR_Code BSRF_g1_Tuner_P_SetIqEqSettings(BSRF_ChannelHandle h, BSRF_IqEqSettings settings);

#endif /* BSRF_G1_PRIV_H__ */
