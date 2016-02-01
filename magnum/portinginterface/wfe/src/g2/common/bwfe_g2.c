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
#include "bwfe_g2.h"
#include "bwfe_g2_priv.h"


BDBG_MODULE(bwfe_g2);


static const BWFE_Settings defDevSettings =
{
   /* API function table */
   {
      BWFE_g2_P_Open,
      BWFE_g2_P_Close,
      BWFE_g2_P_GetTotalChannels,
      BWFE_g2_P_GetTotalRefChannels,
      BWFE_g2_GetChannelDefaultSettings,
      BWFE_g2_P_OpenChannel,
      BWFE_g2_P_CloseChannel,
      BWFE_g2_P_Reset,
      BWFE_g2_P_GetVersion,
      BWFE_g2_P_IsInputEnabled,
      BWFE_g2_P_EnableInput,
      BWFE_g2_P_DisableInput,
      BWFE_g2_P_GetChannelStatus,
      BWFE_g2_P_GetLicTaps,  /* debug */
      BWFE_g2_P_GetEqTaps,   /* debug */
      BWFE_g2_P_FreezeRfAgc,
      BWFE_g2_P_UnfreezeRfAgc,
      BWFE_g2_P_RunLaneCorrection,
      BWFE_g2_P_CalibrateINL,
      BWFE_g2_P_EqualizePipeline,
      BWFE_g2_P_SetAdcSampleFreq,  /* debug */
      BWFE_g2_P_GetAdcSampleFreq,  /* debug */
      BWFE_g2_P_ReadConfig,
      BWFE_g2_P_WriteConfig,
      BWFE_g2_P_ResetDgsLut,
      BWFE_g2_P_ResetEqTaps,
      BWFE_g2_P_ResetLicTaps,
      BWFE_g2_P_CancelDCOffset,
      BWFE_g2_P_GetWfeReadyEventHandle,
      BWFE_g2_P_CalibrateAnalogDelay,
      BWFE_g2_P_GetAnalogDelay,
      BWFE_g2_P_GetSaDoneEventHandle,
      BWFE_g2_P_ScanSpectrum,
      BWFE_g2_P_GetSaSamples,
      BWFE_g2_P_MirrorEquRefTaps,
      BWFE_g2_P_CalibrateAdcPhase
   },
   BWFE_CHIP_SVT_SIGMA_NOM_TO_ONE_FAST
};


static const BWFE_ChannelSettings defChanSettings =
{
   (bool)0
};


/******************************************************************************
 BWFE_g2_GetDefaultSettings()
******************************************************************************/
BERR_Code BWFE_g2_GetDefaultSettings(
   BWFE_Settings *pDefSettings   /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g2_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BWFE_g2_GetChannelDefaultSettings(
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

