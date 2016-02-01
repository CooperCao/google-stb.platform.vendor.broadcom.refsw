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
#include "bstd.h"
#include "bsrf.h"
#include "bsrf_priv.h"

BDBG_MODULE(bsrf);


#ifndef BSRF_EXCLUDE_API_TABLE
/******************************************************************************
 BSRF_Open()
******************************************************************************/
BERR_Code BSRF_Open(BSRF_Handle *h, BCHP_Handle hChip, void *pReg, BINT_Handle hInt, const BSRF_Settings *pDefSettings)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pDefSettings);

   return (pDefSettings->api.Open(h, hChip, pReg, hInt, pDefSettings));
}


/******************************************************************************
 BSRF_Close()
******************************************************************************/
BERR_Code BSRF_Close(BSRF_Handle h)
{
   BDBG_ASSERT(h);
   return (h->settings.api.Close(h));
}


/******************************************************************************
 BSRF_GetTotalChannels()
******************************************************************************/
BERR_Code BSRF_GetTotalChannels(BSRF_Handle h, uint8_t *totalChannels)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(totalChannels);
   return (h->settings.api.GetTotalChannels(h, totalChannels));
}


/******************************************************************************
 BSRF_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BSRF_GetChannelDefaultSettings(BSRF_Handle h, uint8_t chanNum, BSRF_ChannelSettings *pChnDefSettings)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetChannelDefaultSettings(h, chanNum, pChnDefSettings));
}


/******************************************************************************
 BSRF_OpenChannel()
******************************************************************************/
BERR_Code BSRF_OpenChannel(BSRF_Handle h, BSRF_ChannelHandle *pChannelHandle, uint8_t chanNum, const BSRF_ChannelSettings *pSettings)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pChannelHandle);
   return (h->settings.api.OpenChannel(h, pChannelHandle, chanNum, pSettings));
}


/******************************************************************************
 BSRF_CloseChannel()
******************************************************************************/
BERR_Code BSRF_CloseChannel(BSRF_ChannelHandle h)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.CloseChannel(h));
}


/******************************************************************************
 BSRF_Reset()
******************************************************************************/
BERR_Code BSRF_Reset(BSRF_Handle h)
{
   BDBG_ASSERT(h);
   return (h->settings.api.Reset(h));
}


/******************************************************************************
 BSRF_GetVersion()
******************************************************************************/
BERR_Code BSRF_GetVersion(BSRF_Handle h, BFEC_VersionInfo *pVersion)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pVersion);
   return (h->settings.api.GetVersion(h, pVersion));
}


/******************************************************************************
 BSRF_PowerUp
******************************************************************************/
BERR_Code BSRF_PowerUp(BSRF_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_PowerUp);
   return (h->pDevice->settings.api.PowerUp(h));
}


/******************************************************************************
 BSRF_PowerDown
******************************************************************************/
BERR_Code BSRF_PowerDown(BSRF_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_PowerDown);
   return (h->pDevice->settings.api.PowerDown(h));
}


/******************************************************************************
 BSRF_FreezeRfAgc
******************************************************************************/
BERR_Code BSRF_FreezeRfAgc(BSRF_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_FreezeRfAgc);
   return (h->pDevice->settings.api.FreezeRfAgc(h));
}


/******************************************************************************
 BSRF_UnfreezeRfAgc
******************************************************************************/
BERR_Code BSRF_UnfreezeRfAgc(BSRF_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_UnfreezeRfAgc);
   return (h->pDevice->settings.api.UnfreezeRfAgc(h));
}


/******************************************************************************
 BSRF_WriteRfAgc
******************************************************************************/
BERR_Code BSRF_WriteRfAgc(BSRF_ChannelHandle h, uint32_t val)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_WriteRfAgc);
   return (h->pDevice->settings.api.WriteRfAgc(h, val));
}


/******************************************************************************
 BSRF_ReadRfAgc
******************************************************************************/
BERR_Code BSRF_ReadRfAgc(BSRF_ChannelHandle h, uint32_t *pVal)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_ReadRfAgc);
   return (h->pDevice->settings.api.ReadRfAgc(h, pVal));
}


/******************************************************************************
 BSRF_GetInputPower
******************************************************************************/
BERR_Code BSRF_GetInputPower(BSRF_ChannelHandle h, uint32_t *pPower)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_GetInputPower);
   return (h->pDevice->settings.api.GetInputPower(h, pPower));
}


/******************************************************************************
 BSRF_SetRfAgcSettings
******************************************************************************/
BERR_Code BSRF_SetRfAgcSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings settings)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_SetRfAgcSettings);
   return (h->pDevice->settings.api.SetRfAgcSettings(h, settings));
}


/******************************************************************************
 BSRF_GetRfAgcSettings
******************************************************************************/
BERR_Code BSRF_GetRfAgcSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings *pSettings)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_GetRfAgcSettings);
   return (h->pDevice->settings.api.GetRfAgcSettings(h, pSettings));
}


/******************************************************************************
 BSRF_SetFastDecayGainThreshold
******************************************************************************/
BERR_Code BSRF_SetFastDecayGainThreshold(BSRF_ChannelHandle h, uint32_t threshold)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_SetFastDecayGainThreshold);
   return (h->pDevice->settings.api.SetFastDecayGainThreshold(h, threshold));
}


/******************************************************************************
 BSRF_GetFastDecayGainThreshold
******************************************************************************/
BERR_Code BSRF_GetFastDecayGainThreshold(BSRF_ChannelHandle h, uint32_t *pThreshold)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_GetFastDecayGainThreshold);
   return (h->pDevice->settings.api.GetFastDecayGainThreshold(h, pThreshold));
}


/******************************************************************************
 BSRF_SetAntennaOverThreshold
******************************************************************************/
BERR_Code BSRF_SetAntennaOverThreshold(BSRF_ChannelHandle h, uint8_t mode)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_SetAntennaOverThreshold);
   return (h->pDevice->settings.api.SetAntennaOverThreshold(h, mode));
}


/******************************************************************************
 BSRF_GetAntennaOverThreshold
******************************************************************************/
BERR_Code BSRF_GetAntennaOverThreshold(BSRF_ChannelHandle h, uint8_t *pMode)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_GetAntennaOverThreshold);
   return (h->pDevice->settings.api.GetAntennaOverThreshold(h, pMode));
}


/******************************************************************************
 BSRF_SetAntennaDetectThreshold
******************************************************************************/
BERR_Code BSRF_SetAntennaDetectThreshold(BSRF_ChannelHandle h, uint8_t mode)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_SetAntennaDetectThreshold);
   return (h->pDevice->settings.api.SetAntennaDetectThreshold(h, mode));
}


/******************************************************************************
 BSRF_GetAntennaDetectThreshold
******************************************************************************/
BERR_Code BSRF_GetAntennaDetectThreshold(BSRF_ChannelHandle h, uint8_t *pMode)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_GetAntennaDetectThreshold);
   return (h->pDevice->settings.api.GetAntennaDetectThreshold(h, pMode));
}


/******************************************************************************
 BSRF_GetAntennaStatus
******************************************************************************/
BERR_Code BSRF_GetAntennaStatus(BSRF_ChannelHandle h, BSRF_AntennaStatus *pStatus)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_GetAntennaStatus);
   return (h->pDevice->settings.api.GetAntennaStatus(h, pStatus));
}


/******************************************************************************
 BSRF_Tune
******************************************************************************/
BERR_Code BSRF_Tune(BSRF_ChannelHandle h, int32_t freq_hz)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_Tune);
   return (h->pDevice->settings.api.Tune(h, freq_hz));
}


/******************************************************************************
 BSRF_GetTunerStatus
******************************************************************************/
BERR_Code BSRF_GetTunerStatus(BSRF_ChannelHandle h, BSRF_TunerStatus *pStatus)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_GetTunerStatus);
   return (h->pDevice->settings.api.GetTunerStatus(h, pStatus));
}


/******************************************************************************
 BSRF_ResetClipCount
******************************************************************************/
BERR_Code BSRF_ResetClipCount(BSRF_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_ResetClipCount);
   return (h->pDevice->settings.api.ResetClipCount(h));
}


/******************************************************************************
 BSRF_GetClipCount
******************************************************************************/
BERR_Code BSRF_GetClipCount(BSRF_ChannelHandle h, uint32_t *pClipCount)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_GetClipCount);
   return (h->pDevice->settings.api.GetClipCount(h, pClipCount));
}


/******************************************************************************
 BSRF_ConfigTestMode
******************************************************************************/
BERR_Code BSRF_ConfigTestMode(BSRF_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_ConfigTestMode);
   return (h->pDevice->settings.api.ConfigTestMode(h));
}


/******************************************************************************
 BSRF_ConfigOutput
******************************************************************************/
BERR_Code BSRF_ConfigOutput(BSRF_Handle h, BSRF_OutputSelect output, bool bSlewEdges, uint8_t driveStrength_ma)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_ConfigOutput);
   return (h->settings.api.ConfigOutput(h, output, bSlewEdges, driveStrength_ma));
}


/******************************************************************************
 BSRF_ConfigTestDac
******************************************************************************/
BERR_Code BSRF_ConfigTestDac(BSRF_Handle h, int32_t freq_hz, int16_t *pCoeff)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_ConfigTestDac);
   return (h->settings.api.ConfigTestDac(h, freq_hz, pCoeff));
}


/******************************************************************************
 BSRF_EnableTestDac
******************************************************************************/
BERR_Code BSRF_EnableTestDac(BSRF_Handle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_EnableTestDac);
   return (h->settings.api.EnableTestDac(h));
}


/******************************************************************************
 BSRF_DisableTestDac
******************************************************************************/
BERR_Code BSRF_DisableTestDac(BSRF_Handle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_DisableTestDac);
   return (h->settings.api.DisableTestDac(h));
}


/******************************************************************************
 BSRF_EnableTestDacTone
******************************************************************************/
BERR_Code BSRF_EnableTestDacTone(BSRF_Handle h, bool bToneOn, uint16_t toneAmpl)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_EnableTestDacTone);
   return (h->settings.api.EnableTestDacTone(h, bToneOn, toneAmpl));
}

#endif
