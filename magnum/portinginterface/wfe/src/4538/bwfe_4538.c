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
#include "bwfe_4538.h"
#include "bwfe_4538_priv.h"


BDBG_MODULE(bwfe_4538);


static const BWFE_Settings defDevSettings =
{
   /* API function table */
   {
      BWFE_4538_P_Open,
      BWFE_4538_P_Close,
      BWFE_4538_P_GetTotalChannels,
      NULL,    /* BWFE_GetTotalRefChannels */
      BWFE_4538_GetChannelDefaultSettings,
      BWFE_4538_P_OpenChannel,
      BWFE_4538_P_CloseChannel,
      BWFE_4538_P_Reset,
      BWFE_4538_P_GetVersion,
      BWFE_4538_P_IsInputEnabled,
      BWFE_4538_P_EnableInput,
      BWFE_4538_P_DisableInput,
      NULL,    /* BWFE_GetChannelStatus */
      NULL,    /* DEBUG: BWFE_GetLicTaps */
      NULL,    /* DEBUG: BWFE_GetEqTaps */
      NULL,    /* BWFE_FreezeRfAgc */
      NULL,    /* BWFE_UnfreezeRfAgc */
      NULL,    /* BWFE_RunLaneCorrection */
      NULL,    /* BWFE_CalibrateINL */
      NULL,    /* BWFE_EqualizePipeline */
      NULL,    /* DEBUG: BWFE_SetAdcSampleFreq */
      NULL,    /* DEBUG: BWFE_GetAdcSampleFreq */
      NULL,    /* BWFE_ReadConfig */
      NULL,    /* BWFE_WriteConfig */
      NULL,    /* BWFE_ResetDgsLut */
      NULL,    /* BWFE_ResetEqTaps */
      NULL,    /* BWFE_ResetLicTaps */
      NULL,    /* BWFE_CancelDCOffset */
      BWFE_4538_P_GetWfeReadyEventHandle,
      NULL,    /* BWFE_CalibrateAnalogDelay */
      NULL,    /* BWFE_GetAnalogDelay */
      BWFE_4538_P_GetSaDoneEventHandle,
      BWFE_4538_P_ScanSpectrum,
      BWFE_4538_P_GetSaSamples,
      NULL,    /* BWFE_MirrorEquRefTaps */
      NULL     /* BWFE_CalibrateAdcPhase */
   },
   BWFE_CHIP_SVT_SIGMA_NOM_TO_ONE_FAST,
   54000 /* default xtal freq */
};


static const BWFE_ChannelSettings defChanSettings =
{
   (bool)0
};


/******************************************************************************
 BWFE_4538_GetDefaultSettings()
******************************************************************************/
BERR_Code BWFE_4538_GetDefaultSettings(
   BWFE_Settings *pDefSettings   /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_4538_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BWFE_4538_GetChannelDefaultSettings(
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
