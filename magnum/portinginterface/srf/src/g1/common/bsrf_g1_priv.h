/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
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
#ifndef BSRF_G1_PRIV_H__
#define BSRF_G1_PRIV_H__

#include "bsrf.h"
#include "bsrf_g1.h"

#if (BCHP_CHIP==lonestar)
   #include "bsrf_lonestar_priv.h"
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
   BSRF_RfAgcSettings   settings;   /* rfagc settings */
   int32_t              tunerFreq;  /* tuner freq */
   uint8_t              modeAoc;    /* antenna over-current mode */
   uint8_t              modeAd;     /* antenna detect mode */
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
BERR_Code BSRF_g1_P_GetInputPower(BSRF_ChannelHandle h, uint32_t *pPower);
BERR_Code BSRF_g1_P_SetRfAgcSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings settings);
BERR_Code BSRF_g1_P_GetRfAgcSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings *pSettings);
BERR_Code BSRF_g1_P_SetFastDecayGainThreshold(BSRF_ChannelHandle h, uint32_t threshold);
BERR_Code BSRF_g1_P_GetFastDecayGainThreshold(BSRF_ChannelHandle h, uint32_t *pThreshold);
BERR_Code BSRF_g1_P_SetAntennaOverThreshold(BSRF_ChannelHandle h, uint8_t mode);
BERR_Code BSRF_g1_P_GetAntennaOverThreshold(BSRF_ChannelHandle h, uint8_t *pMode);
BERR_Code BSRF_g1_P_SetAntennaDetectThreshold(BSRF_ChannelHandle h, uint8_t mode);
BERR_Code BSRF_g1_P_GetAntennaDetectThreshold(BSRF_ChannelHandle h, uint8_t *pMode);
BERR_Code BSRF_g1_P_GetAntennaStatus(BSRF_ChannelHandle h, BSRF_AntennaStatus *pStatus);
BERR_Code BSRF_g1_P_Tune(BSRF_ChannelHandle h, int32_t freqHz);
BERR_Code BSRF_g1_P_GetTunerStatus(BSRF_ChannelHandle h, BSRF_TunerStatus *pStatus);
BERR_Code BSRF_g1_P_ResetClipCount(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_P_GetClipCount(BSRF_ChannelHandle h, uint32_t *pClipCount);
BERR_Code BSRF_g1_P_ConfigTestMode(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_P_ConfigOutput(BSRF_Handle h, BSRF_OutputSelect output, bool bSlewEdges, uint8_t driveStrength_ma);
BERR_Code BSRF_g1_P_ConfigTestDac(BSRF_Handle h, int32_t freqHz, int16_t *pCoeff);
BERR_Code BSRF_g1_P_EnableTestDac(BSRF_Handle h);
BERR_Code BSRF_g1_P_DisableTestDac(BSRF_Handle h);
BERR_Code BSRF_g1_P_EnableTestDacTone(BSRF_Handle h, bool bToneOn, uint16_t toneAmpl);

/* bsrf_g1_priv_rfagc */
BERR_Code BSRF_g1_Rfagc_P_Init(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_Rfagc_P_SetSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings settings);

/* bsrf_g1_priv_tuner */
BERR_Code BSRF_g1_Tuner_P_Init(BSRF_ChannelHandle h);
BERR_Code BSRF_g1_Tuner_P_SetFcw(BSRF_ChannelHandle h, int32_t freqHz);

#endif /* BSRF_G1_PRIV_H__ */
