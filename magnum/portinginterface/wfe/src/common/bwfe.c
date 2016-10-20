/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "bwfe.h"
#include "bwfe_priv.h"

BDBG_MODULE(bwfe);


#ifndef BWFE_EXCLUDE_API_TABLE
/******************************************************************************
 BWFE_Open()
******************************************************************************/
BERR_Code BWFE_Open(BWFE_Handle *h, BCHP_Handle hChip, void *pReg, BINT_Handle hInt, const BWFE_Settings *pDefSettings)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pDefSettings);

   return (pDefSettings->api.Open(h, hChip, pReg, hInt, pDefSettings));
}


/******************************************************************************
 BWFE_Close()
******************************************************************************/
BERR_Code BWFE_Close(BWFE_Handle h)
{
   BDBG_ASSERT(h);
   return (h->settings.api.Close(h));
}


/******************************************************************************
 BWFE_GetTotalChannels()
******************************************************************************/
BERR_Code BWFE_GetTotalChannels(BWFE_Handle h, BWFE_ChannelInfo *pInfo)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetTotalChannels(h, pInfo));
}


/******************************************************************************
 BWFE_GetTotalRefChannels()
******************************************************************************/
BERR_Code BWFE_GetTotalRefChannels(BWFE_Handle h, uint8_t *totalRefChannels)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetTotalRefChannels(h, totalRefChannels));
}


/******************************************************************************
 BWFE_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BWFE_GetChannelDefaultSettings(BWFE_Handle h, uint8_t chanNum, BWFE_ChannelSettings *pChnDefSettings)
{
   BDBG_ASSERT(h);
   return (h->settings.api.GetChannelDefaultSettings(h, chanNum, pChnDefSettings));
}


/******************************************************************************
 BWFE_OpenChannel()
******************************************************************************/
BERR_Code BWFE_OpenChannel(BWFE_Handle h, BWFE_ChannelHandle *pChannelHandle, uint8_t chanNum, const BWFE_ChannelSettings *pSettings)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pChannelHandle);
   return (h->settings.api.OpenChannel(h, pChannelHandle, chanNum, pSettings));
}


/******************************************************************************
 BWFE_CloseChannel()
******************************************************************************/
BERR_Code BWFE_CloseChannel(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.CloseChannel(h));
}


/******************************************************************************
 BWFE_Reset()
******************************************************************************/
BERR_Code BWFE_Reset(BWFE_Handle h)
{
   BDBG_ASSERT(h);
   return (h->settings.api.Reset(h));
}


/******************************************************************************
 BWFE_GetVersion()
******************************************************************************/
BERR_Code BWFE_GetVersion(BWFE_Handle h, BFEC_VersionInfo *pVersion)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pVersion);
   return (h->settings.api.GetVersion(h, pVersion));
}


/******************************************************************************
 BWFE_IsInputEnabled()
******************************************************************************/
BERR_Code BWFE_IsInputEnabled(BWFE_ChannelHandle h, bool *bEnabled)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.IsInputEnabled(h, bEnabled));
}


/******************************************************************************
 BWFE_EnableInput()
******************************************************************************/
BERR_Code BWFE_EnableInput(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.EnableInput(h));
}


/******************************************************************************
 BWFE_DisableInput()
******************************************************************************/
BERR_Code BWFE_DisableInput(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.DisableInput(h));
}


/******************************************************************************
 BWFE_GetChannelStatus()
******************************************************************************/
BERR_Code BWFE_GetChannelStatus(BWFE_ChannelHandle h, BWFE_ChannelStatus *pStatus)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pStatus);
   if (h->pDevice->settings.api.GetChannelStatus)
      return (h->pDevice->settings.api.GetChannelStatus(h, pStatus));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_GetLicTaps()
******************************************************************************/
BERR_Code BWFE_GetLicTaps(BWFE_ChannelHandle h, uint32_t **pLicCoeff)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pLicCoeff);
   if (h->pDevice->settings.api.GetLicTaps)
      return (h->pDevice->settings.api.GetLicTaps(h, pLicCoeff));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_GetEqTaps()
******************************************************************************/
BERR_Code BWFE_GetEqTaps(BWFE_ChannelHandle h, uint32_t ****pEqCoeff)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pEqCoeff);
   if (h->pDevice->settings.api.GetEqTaps)
      return (h->pDevice->settings.api.GetEqTaps(h, pEqCoeff));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_FreezeRfAgc()
******************************************************************************/
BERR_Code BWFE_FreezeRfAgc(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.FreezeRfAgc)
      return (h->pDevice->settings.api.FreezeRfAgc(h));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_UnfreezeRfAgc()
******************************************************************************/
BERR_Code BWFE_UnfreezeRfAgc(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.UnfreezeRfAgc)
      return (h->pDevice->settings.api.UnfreezeRfAgc(h));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_RunLaneCorrection()
******************************************************************************/
BERR_Code BWFE_RunLaneCorrection(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.RunLaneCorrection)
      return (h->pDevice->settings.api.RunLaneCorrection(h));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_CalibrateINL()
******************************************************************************/
BERR_Code BWFE_CalibrateINL(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.CalibrateINL)
      return (h->pDevice->settings.api.CalibrateINL(h));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_EqualizePipeline()
******************************************************************************/
BERR_Code BWFE_EqualizePipeline(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.EqualizePipeline)
      return (h->pDevice->settings.api.EqualizePipeline(h));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_SetAdcSampleFreq()
******************************************************************************/
BERR_Code BWFE_SetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t freqKhz)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.SetAdcSampleFreq)
      return (h->pDevice->settings.api.SetAdcSampleFreq(h, freqKhz));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_GetAdcSampleFreq()
******************************************************************************/
BERR_Code BWFE_GetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t *freqKhz)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.GetAdcSampleFreq)
      return (h->pDevice->settings.api.GetAdcSampleFreq(h, freqKhz));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_ReadConfig()
******************************************************************************/
BERR_Code BWFE_ReadConfig(BWFE_ChannelHandle h, uint16_t id, uint8_t *p, uint8_t len)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.ReadConfig)
      return (h->pDevice->settings.api.ReadConfig(h, id, p, len));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_WriteConfig()
******************************************************************************/
BERR_Code BWFE_WriteConfig(BWFE_ChannelHandle h, uint16_t id, uint8_t *p, uint8_t len)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.WriteConfig)
      return (h->pDevice->settings.api.WriteConfig(h, id, p, len));
   else
      return BERR_NOT_SUPPORTED;
}

/******************************************************************************
 BWFE_ResetDgsLut()
******************************************************************************/
BERR_Code BWFE_ResetDgsLut(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.ResetDgsLut)
      return (h->pDevice->settings.api.ResetDgsLut(h));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_ResetEqTaps()
******************************************************************************/
BERR_Code BWFE_ResetEqTaps(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.ResetEqTaps)
      return (h->pDevice->settings.api.ResetEqTaps(h));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_ResetLicTaps()
******************************************************************************/
BERR_Code BWFE_ResetLicTaps(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.ResetLicTaps)
      return (h->pDevice->settings.api.ResetLicTaps(h));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_CancelDCOffset()
******************************************************************************/
BERR_Code BWFE_CancelDCOffset(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.CancelDCOffset)
      return (h->pDevice->settings.api.CancelDCOffset(h));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_GetWfeReadyEventHandle()
******************************************************************************/
BERR_Code BWFE_GetWfeReadyEventHandle(BWFE_ChannelHandle h, BKNI_EventHandle *hWfeReady)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(hWfeReady);
   if (h->pDevice->settings.api.GetWfeReadyEventHandle)
      return (h->pDevice->settings.api.GetWfeReadyEventHandle(h, hWfeReady));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_CalibrateAnalogDelay()
******************************************************************************/
BERR_Code BWFE_CalibrateAnalogDelay(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.CalibrateAnalogDelay)
      return (h->pDevice->settings.api.CalibrateAnalogDelay(h));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_GetAnalogDelay()
******************************************************************************/
BERR_Code BWFE_GetAnalogDelay(BWFE_ChannelHandle h, uint32_t *pCorrDelays)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pCorrDelays);
   if (h->pDevice->settings.api.GetAnalogDelay)
      return (h->pDevice->settings.api.GetAnalogDelay(h, pCorrDelays));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_CompensateDelay()
******************************************************************************/
BERR_Code BWFE_CompensateDelay(BWFE_ChannelHandle h, uint32_t *pSliceAdjust)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSliceAdjust);
   if (h->pDevice->settings.api.CompensateDelay)
      return (h->pDevice->settings.api.CompensateDelay(h, pSliceAdjust));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_GetSaDoneEventHandle()
******************************************************************************/
BERR_Code BWFE_GetSaDoneEventHandle(BWFE_Handle h, BKNI_EventHandle *hSaDoneEvent)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(hSaDoneEvent);
   if (h->settings.api.GetSaDoneEventHandle)
      return (h->settings.api.GetSaDoneEventHandle(h, hSaDoneEvent));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_ScanSpectrum()
******************************************************************************/
BERR_Code BWFE_ScanSpectrum(BWFE_ChannelHandle h, BWFE_SpecAnalyzerParams *pParams)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pParams);
   if (h->pDevice->settings.api.ScanSpectrum)
      return (h->pDevice->settings.api.ScanSpectrum(h, pParams));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_GetSaSamples()
******************************************************************************/
BERR_Code BWFE_GetSaSamples(BWFE_Handle h, uint32_t *pSaSamples)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSaSamples);
   if (h->settings.api.GetSaSamples)
      return (h->settings.api.GetSaSamples(h, pSaSamples));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_MirrorEquRefTaps()
******************************************************************************/
BERR_Code BWFE_MirrorEquRefTaps(BWFE_ChannelHandle h, BWFE_ChannelHandle hRef)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.MirrorEquRefTaps)
      return (h->pDevice->settings.api.MirrorEquRefTaps(h, hRef));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_CalibrateAdcPhase()
******************************************************************************/
BERR_Code BWFE_CalibrateAdcPhase(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.CalibrateAdcPhase)
      return (h->pDevice->settings.api.CalibrateAdcPhase(h));
   else
      return BERR_NOT_SUPPORTED;
}

#endif

