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
#include "bsrf.h"
#include "bsrf_g1.h"
#include "bsrf_g1_priv.h"

BDBG_MODULE(bsrf_g1);


static const BSRF_Settings defDevSettings =
{
   /* API function table */
   {
      BSRF_g1_P_Open,
      BSRF_g1_P_Close,
      BSRF_g1_P_GetTotalChannels,
      BSRF_g1_GetChannelDefaultSettings,
      BSRF_g1_P_OpenChannel,
      BSRF_g1_P_CloseChannel,
      BSRF_g1_P_Reset,
      BSRF_g1_P_GetVersion,
      BSRF_g1_P_PowerUp,
      BSRF_g1_P_PowerDown,
      BSRF_g1_P_FreezeRfAgc,
      BSRF_g1_P_UnfreezeRfAgc,
      BSRF_g1_P_WriteRfAgc,
      BSRF_g1_P_ReadRfAgc,
      BSRF_g1_P_GetInputPower,
      BSRF_g1_P_SetRfAgcSettings,
      BSRF_g1_P_GetRfAgcSettings,
      BSRF_g1_P_SetFastDecayGainThreshold,
      BSRF_g1_P_GetFastDecayGainThreshold,
      BSRF_g1_P_SetAntennaOverThreshold,
      BSRF_g1_P_GetAntennaOverThreshold,
      BSRF_g1_P_SetAntennaDetectThreshold,
      BSRF_g1_P_GetAntennaDetectThreshold,
      BSRF_g1_P_GetAntennaStatus,
      BSRF_g1_P_Tune,
      BSRF_g1_P_GetTunerStatus,
      BSRF_g1_P_ResetClipCount,
      BSRF_g1_P_GetClipCount,
      BSRF_g1_P_ConfigTestMode,
      BSRF_g1_P_ConfigOutput,
      BSRF_g1_P_ConfigTestDac,
      BSRF_g1_P_EnableTestDac,
      BSRF_g1_P_DisableTestDac,
      BSRF_g1_P_EnableTestDacTone
   }
};

static const BSRF_ChannelSettings defChanSettings =
{
   (uint8_t)0
};


/******************************************************************************
 BSRF_g1_GetDefaultSettings()
******************************************************************************/
BERR_Code BSRF_g1_GetDefaultSettings(BSRF_Settings *pDefSettings)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BSRF_g1_GetChannelDefaultSettings(
   BSRF_Handle   h,                       /* [in] BSRF handle */
   uint8_t       chanNum,                 /* [in] channel number */
   BSRF_ChannelSettings *pChnDefSettings  /* [out] default channel settings */
)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(chanNum);

   *pChnDefSettings = defChanSettings;
   return BERR_SUCCESS;
}
