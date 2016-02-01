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
#include "bwfe.h"
#include "bwfe_g3.h"
#include "bwfe_g3_priv.h"


BDBG_MODULE(bwfe_g3);


static const BWFE_Settings defDevSettings =
{
   /* API function table */
   {
      BWFE_g3_P_Open,
      BWFE_g3_P_Close,
      BWFE_g3_P_GetTotalChannels,
      BWFE_g3_P_GetTotalRefChannels,
      BWFE_g3_GetChannelDefaultSettings,
      BWFE_g3_P_OpenChannel,
      BWFE_g3_P_CloseChannel,
      BWFE_g3_P_Reset,
      BWFE_g3_P_GetVersion,
      BWFE_g3_P_IsInputEnabled,
      BWFE_g3_P_EnableInput,
      BWFE_g3_P_DisableInput,
      BWFE_g3_P_GetChannelStatus,
      BWFE_g3_P_GetLicTaps,  /* debug */
      BWFE_g3_P_GetEqTaps,   /* debug */
      BWFE_g3_P_FreezeRfAgc,
      BWFE_g3_P_UnfreezeRfAgc,
      BWFE_g3_P_RunLaneCorrection,
      BWFE_g3_P_CalibrateINL,
      BWFE_g3_P_EqualizePipeline,
      BWFE_g3_P_SetAdcSampleFreq,  /* debug */
      BWFE_g3_P_GetAdcSampleFreq,  /* debug */
      BWFE_g3_P_GetChannelConfig,
      BWFE_g3_P_SetChannelConfig,
      BWFE_g3_P_ResetDgsLut,
      BWFE_g3_P_ResetEqTaps,
      BWFE_g3_P_ResetLicTaps,
      BWFE_g3_P_CancelDCOffset,
      BWFE_g3_P_GetWfeReadyEventHandle,
      BWFE_g3_P_CalibrateAnalogDelay,
      BWFE_g3_P_GetAnalogDelay,
      BWFE_g3_P_GetSaDoneEventHandle,
      BWFE_g3_P_ScanSpectrum,
      BWFE_g3_P_GetSaSamples,
      BWFE_g3_P_MirrorEquRefTaps,
      BWFE_g3_P_CalibrateAdcPhase
   },
   BWFE_CHIP_SVT_SIGMA_NOM_TO_ONE_FAST,
   54000 /* default xtal freq */
};


static const BWFE_ChannelSettings defChanSettings =
{
   false
};


/******************************************************************************
 BWFE_g3_GetDefaultSettings()
******************************************************************************/
BERR_Code BWFE_g3_GetDefaultSettings(
   BWFE_Settings *pDefSettings   /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BWFE_g3_GetChannelDefaultSettings(
   BWFE_Handle   h,                      /* [in] BWFE handle */
   uint8_t       chanNum,                /* [in] channel number */
   BWFE_ChannelSettings *pChnDefSettings /* [out] default channel settings */
)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(chanNum);
   
   *pChnDefSettings = defChanSettings;
   return BERR_SUCCESS;
}

