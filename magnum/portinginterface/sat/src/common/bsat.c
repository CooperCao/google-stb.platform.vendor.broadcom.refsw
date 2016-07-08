/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
* Module Description:
*
*****************************************************************************/
#include "bsat.h"
#include "bsat_priv.h"


BDBG_MODULE(bsat);

#ifndef BSAT_EXCLUDE_API_TABLE


/******************************************************************************
 BSAT_Open()
******************************************************************************/
BERR_Code BSAT_Open(
   BSAT_Handle *h,                /* [out] BSAT handle */
   BCHP_Handle hChip,             /* [in] chip handle */
   void *pReg,                    /* [in] BREG or BHAB handle */
   BINT_Handle hInterrupt,        /* [in] BINT handle */
   const BSAT_Settings *pSettings /* [in] default settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   return (pSettings->api.Open(h, hChip, pReg, hInterrupt, pSettings));
}


/******************************************************************************
 BSAT_Close()
******************************************************************************/
uint32_t BSAT_Close(
   BSAT_Handle h   /* [in] BSAT handle */
)
{
   BDBG_ASSERT(h);
   return (h->settings.api.Close(h));
}


/******************************************************************************
 BSAT_GetTotalChannels()
******************************************************************************/
BERR_Code BSAT_GetTotalChannels(
   BSAT_Handle h,             /* [in] BSAT handle */
   uint32_t    *totalChannels /* [out] number of satellite downstream channels supported by this chip */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(totalChannels);
   return (h->settings.api.GetTotalChannels(h, totalChannels));
}


/******************************************************************************
 BSAT_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BSAT_GetChannelDefaultSettings(
   BSAT_Handle h,                    /* [in] BSAT handle */
   uint32_t    chan,                 /* [in] channel index (0 based) */
   BSAT_ChannelSettings *pSettings   /* [out] default channel settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   return (h->settings.api.GetChannelDefaultSettings(h, chan, pSettings));
}


/******************************************************************************
 BSAT_OpenChannel()
******************************************************************************/
BERR_Code BSAT_OpenChannel(
   BSAT_Handle                h,               /* [in] BSAT handle */
   BSAT_ChannelHandle         *pChannelHandle, /* [out] BSAT channel handle */
   uint32_t                   chan,      /* [in] channel index (0 based) */
   const BSAT_ChannelSettings *pSettings /* [in] channel settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pChannelHandle);
   BDBG_ASSERT(pSettings);
   return (h->settings.api.OpenChannel(h, pChannelHandle, chan, pSettings));
}


/******************************************************************************
 BSAT_CloseChannel()
******************************************************************************/
BERR_Code BSAT_CloseChannel(
   BSAT_ChannelHandle h  /* [in] BSAT channel handle */
)
{
   BDBG_ASSERT(h);
   return (h->pDevice->settings.api.CloseChannel(h));
}


/******************************************************************************
 BSAT_GetDevice()
******************************************************************************/
BERR_Code BSAT_GetDevice(
   BSAT_ChannelHandle h,     /* [in] BSAT channel handle */
   BSAT_Handle        *pDev  /* [out] BSAT handle */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pDev);
   return (h->pDevice->settings.api.GetDevice(h, pDev));
}


/******************************************************************************
 BSAT_GetVersionInfo()
******************************************************************************/
BERR_Code BSAT_GetVersionInfo(
   BSAT_Handle h,             /* [in] BSAT handle */
   BFEC_VersionInfo *pVersion /* [out] version information */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pVersion);
   return (h->settings.api.GetVersionInfo(h, pVersion));
}


/******************************************************************************
 BSAT_Reset()
******************************************************************************/
BERR_Code BSAT_Reset(
   BSAT_Handle h    /* [in] BSAT handle */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.Reset)
      return h->settings.api.Reset(h);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_PowerDownChannel()
******************************************************************************/
BERR_Code BSAT_PowerDownChannel(
   BSAT_ChannelHandle h  /* [in] BSAT channel handle */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.PowerDownChannel)
      return h->pDevice->settings.api.PowerDownChannel(h);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_PowerUpChannel()
******************************************************************************/
BERR_Code BSAT_PowerUpChannel(
   BSAT_ChannelHandle h  /* [in] BSAT channel handle */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.PowerUpChannel)
      return h->pDevice->settings.api.PowerUpChannel(h);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_IsChannelOn()
******************************************************************************/
BERR_Code BSAT_IsChannelOn(
   BSAT_ChannelHandle h, /* [in] BSAT channel handle */
   bool *pOn             /* [out] true=channel is powered */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.IsChannelOn)
      return h->pDevice->settings.api.IsChannelOn(h, pOn);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_Acquire()
******************************************************************************/
BERR_Code BSAT_Acquire(
   BSAT_ChannelHandle h,         /* [in] BSAT channel handle */
   BSAT_AcqSettings   *pParams   /* [in] acquisition parameters */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pParams);
   if (h->pDevice->settings.api.Acquire)
      return h->pDevice->settings.api.Acquire(h, pParams);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetLockStatus()
******************************************************************************/
BERR_Code BSAT_GetLockStatus(
   BSAT_ChannelHandle h,        /* [in] BSAT channel handle */
   bool               *pbLocked, /* [out] true if demod, FEC, and BERT (if enabled) are all locked */
   bool               *pbAcqDone /* [out] true if Acquisition Done has been asserted */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pbLocked);
   BDBG_ASSERT(pbAcqDone);
   if (h->pDevice->settings.api.GetLockStatus)
      return h->pDevice->settings.api.GetLockStatus(h, pbLocked, pbAcqDone);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetChannelStatus()
******************************************************************************/
BERR_Code BSAT_GetChannelStatus(
   BSAT_ChannelHandle h,       /* [in] BSAT channel handle */
   BSAT_ChannelStatus *pStatus /* [out] channel status */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pStatus);
   if (h->pDevice->settings.api.GetChannelStatus)
      return h->pDevice->settings.api.GetChannelStatus(h, pStatus);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_ResetChannelStatus()
******************************************************************************/
BERR_Code BSAT_ResetChannelStatus(
   BSAT_ChannelHandle h  /* [in] BSAT channel handle */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.ResetChannelStatus)
      return h->pDevice->settings.api.ResetChannelStatus(h);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetSoftDecisions()
******************************************************************************/
BERR_Code BSAT_GetSoftDecisions(
   BSAT_ChannelHandle h,   /* [in] BSAT channel handle */
   uint32_t           n,   /* [in] number of I/Q pairs to get */
   int16_t            *pI, /* [out] n I-values */
   int16_t            *pQ  /* [out] n Q-values */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(n > 0);
   BDBG_ASSERT(pI);
   BDBG_ASSERT(pQ);
   if (h->pDevice->settings.api.GetSoftDecisions)
      return h->pDevice->settings.api.GetSoftDecisions(h, n, pI, pQ);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_ResetChannel()
******************************************************************************/
BERR_Code BSAT_ResetChannel(
   BSAT_ChannelHandle h,            /* [in] BSAT channel handle */
   bool               bDisableDemod /* [in] true=disable demod/oif after stopping acquisition */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.ResetChannel)
      return h->pDevice->settings.api.ResetChannel(h, bDisableDemod);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetBertSettings()
******************************************************************************/
BERR_Code BSAT_SetBertSettings(
   BSAT_ChannelHandle h,         /* [in] BSAT channel handle */
   BSAT_BertSettings  *pSettings /* [in] BERT settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.SetBertSettings)
      return h->pDevice->settings.api.SetBertSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetBertSettings()
******************************************************************************/
BERR_Code BSAT_GetBertSettings(
   BSAT_ChannelHandle h,         /* [in] BSAT channel handle */
   BSAT_BertSettings  *pSettings /* [out] BERT settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.GetBertSettings)
      return h->pDevice->settings.api.GetBertSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetBertStatus()
******************************************************************************/
BERR_Code BSAT_GetBertStatus(
   BSAT_ChannelHandle h,       /* [in] BSAT channel handle */
   BSAT_BertStatus    *pStatus /* [out] BERT status */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pStatus);
   if (h->pDevice->settings.api.GetBertStatus)
      return h->pDevice->settings.api.GetBertStatus(h, pStatus);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetSearchRange()
******************************************************************************/
BERR_Code BSAT_SetSearchRange(
   BSAT_ChannelHandle h,          /* [in] BSAT channel handle */
   uint32_t           searchRange /* [in] search range in Hz */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.SetSearchRange)
      return h->pDevice->settings.api.SetSearchRange(h, searchRange);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetSearchRange()
******************************************************************************/
BERR_Code BSAT_GetSearchRange(
   BSAT_ChannelHandle h,            /* [in] BSAT channel handle */
   uint32_t           *pSearchRange /* [out] search range in Hz */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSearchRange);
   if (h->pDevice->settings.api.GetSearchRange)
      return h->pDevice->settings.api.GetSearchRange(h, pSearchRange);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetAmcScramblingSeq()
******************************************************************************/
BERR_Code BSAT_SetAmcScramblingSeq(
   BSAT_ChannelHandle h,       /* [in] BSAT channel handle */
   BSAT_AmcScramblingSeq *pSeq /* [in] AMC scrambling sequence */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSeq);
   if (h->pDevice->settings.api.SetAmcScramblingSeq)
      return h->pDevice->settings.api.SetAmcScramblingSeq(h, pSeq);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetNetworkSpec()
******************************************************************************/
BERR_Code BSAT_SetNetworkSpec(
   BSAT_Handle      h,          /* [in] BSAT handle */
   BSAT_NetworkSpec networkSpec /* [in] network spec */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.SetNetworkSpec)
      return h->settings.api.SetNetworkSpec(h, networkSpec);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetNetworkSpec()
******************************************************************************/
BERR_Code BSAT_GetNetworkSpec(
   BSAT_Handle      h,            /* [in] BSAT handle */
   BSAT_NetworkSpec *pNetworkSpec /* [out] network spec */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pNetworkSpec);
   if (h->settings.api.GetNetworkSpec)
      return h->settings.api.GetNetworkSpec(h, pNetworkSpec);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetOutputTransportSettings()
******************************************************************************/
BERR_Code BSAT_SetOutputTransportSettings(
   BSAT_ChannelHandle h,                   /* [in] BSAT channel handle */
   BSAT_OutputTransportSettings *pSettings /* [in] transport settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.SetOutputTransportSettings)
      return h->pDevice->settings.api.SetOutputTransportSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetOutputTransportSettings()
******************************************************************************/
BERR_Code BSAT_GetOutputTransportSettings(
   BSAT_ChannelHandle h,                   /* [in] BSAT channel handle */
   BSAT_OutputTransportSettings *pSettings /* [in] transport settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.GetOutputTransportSettings)
      return h->pDevice->settings.api.GetOutputTransportSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetInitDoneEventHandle()
******************************************************************************/
BERR_Code BSAT_GetInitDoneEventHandle(
   BSAT_Handle      h,      /* [in] BSAT handle */
   BKNI_EventHandle *hEvent /* [out] lock event handle */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(hEvent);
   if (h->settings.api.GetInitDoneEventHandle)
      return h->settings.api.GetInitDoneEventHandle(h, hEvent);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BSAT_GetLockStateChangeEventHandle(
   BSAT_ChannelHandle h,      /* [in] BSAT channel handle */
   BKNI_EventHandle   *hEvent /* [out] lock event handle */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(hEvent);
   if (h->pDevice->settings.api.GetLockStateChangeEventHandle)
      return h->pDevice->settings.api.GetLockStateChangeEventHandle(h, hEvent);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetAcqDoneEventHandle()
******************************************************************************/
BERR_Code BSAT_GetAcqDoneEventHandle(
   BSAT_ChannelHandle h,      /* [in] BSAT channel handle */
   BKNI_EventHandle   *hEvent /* [out] event handle */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(hEvent);
   if (h->pDevice->settings.api.GetAcqDoneEventHandle)
      return h->pDevice->settings.api.GetAcqDoneEventHandle(h, hEvent);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetSignalNotificationEventHandle()
******************************************************************************/
BERR_Code BSAT_GetSignalNotificationEventHandle(
   BSAT_ChannelHandle h,      /* [in] BSAT channel handle */
   BKNI_EventHandle   *hEvent /* [out] event handle */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(hEvent);
   if (h->pDevice->settings.api.GetSignalNotificationEventHandle)
      return h->pDevice->settings.api.GetSignalNotificationEventHandle(h, hEvent);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetReadyEventHandle()
******************************************************************************/
BERR_Code BSAT_GetReadyEventHandle(
   BSAT_ChannelHandle h,      /* [in] BSAT channel handle */
   BKNI_EventHandle   *hEvent /* [out] event handle */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(hEvent);
   if (h->pDevice->settings.api.GetReadyEventHandle)
      return h->pDevice->settings.api.GetReadyEventHandle(h, hEvent);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_StartToneDetect()
******************************************************************************/
BERR_Code BSAT_StartToneDetect(
   BSAT_ChannelHandle h,        /* [in] BSAT channel handle */
   uint32_t           freq,     /* [in] center frequency in Hz */
   uint32_t           adcSelect /* [in] ADC selection (0-based index) */

)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.StartToneDetect)
      return h->pDevice->settings.api.StartToneDetect(h, freq, adcSelect);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetToneDetectStatus()
******************************************************************************/
BERR_Code BSAT_GetToneDetectStatus(
   BSAT_ChannelHandle    h,       /* [in] BSAT channel handle */
   BSAT_ToneDetectStatus *pStatus /* [out] tone detect status */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pStatus);
   if (h->pDevice->settings.api.GetToneDetectStatus)
      return h->pDevice->settings.api.GetToneDetectStatus(h, pStatus);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_StartSymbolRateScan()
******************************************************************************/
BERR_Code BSAT_StartSymbolRateScan(
   BSAT_ChannelHandle h,      /* [in] BSAT channel handle */
   uint32_t           freq,   /* [in] center frequency in Hz */
   uint32_t           minSymbolRate, /* [in] minimum symbol rate in sym/sec */
   uint32_t           maxSymbolRate, /* [in] maximum symbol rate in sym/sec */
   uint32_t           adcSelect      /* [in] ADC selection (0-based index) */

)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.StartSymbolRateScan)
      return h->pDevice->settings.api.StartSymbolRateScan(h, freq, minSymbolRate, maxSymbolRate, adcSelect);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetSymbolRateScanStatus()
******************************************************************************/
BERR_Code BSAT_GetSymbolRateScanStatus(
   BSAT_ChannelHandle h,              /* [in] BSAT channel handle */
   BSAT_SymbolRateScanStatus *pStatus /* [out] status from most recent call to BSAT_StartSymbolRateScan() */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pStatus);
   if (h->pDevice->settings.api.GetSymbolRateScanStatus)
      return h->pDevice->settings.api.GetSymbolRateScanStatus(h, pStatus);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_StartPsdScan()
******************************************************************************/
BERR_Code BSAT_StartPsdScan(
   BSAT_ChannelHandle h,        /* [in] BSAT channel handle */
   uint32_t           freq,     /* [in] center frequency in Hz */
   uint32_t           adcSelect /* [in] ADC selection (0-based index) */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.StartPsdScan)
      return h->pDevice->settings.api.StartPsdScan(h, freq, adcSelect);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetPsdScanStatus()
******************************************************************************/
BERR_Code BSAT_GetPsdScanStatus(
   BSAT_ChannelHandle h,       /* [in] BSAT channel handle */
   BSAT_PsdScanStatus *pStatus /* [out] PSD scan status */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pStatus);
   if (h->pDevice->settings.api.GetPsdScanStatus)
      return h->pDevice->settings.api.GetPsdScanStatus(h, pStatus);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetConfig()
******************************************************************************/
BERR_Code BSAT_SetConfig(
   BSAT_Handle h,    /* [in] BSAT device handle */
   uint32_t    addr, /* [in] configuration parameter address */
   uint32_t    val   /* [in] configuration parameter value */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.SetConfig)
      return h->settings.api.SetConfig(h, addr, val);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetConfig()
******************************************************************************/
BERR_Code BSAT_GetConfig(
   BSAT_Handle h,    /* [in] BSAT device handle */
   uint32_t    addr, /* [in] configuration parameter address */
   uint32_t    *pVal /* [out] value of the configuration parameter */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pVal);
   if (h->settings.api.GetConfig)
      return h->settings.api.GetConfig(h, addr, pVal);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetChannelConfig()
******************************************************************************/
BERR_Code BSAT_SetChannelConfig(
   BSAT_ChannelHandle h,    /* [in] BSAT channel handle */
   uint32_t           addr, /* [in] configuration parameter address */
   uint32_t           val   /* [in] configuration parameter value */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.SetChannelConfig)
      return h->pDevice->settings.api.SetChannelConfig(h, addr, val);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetChannelConfig()
******************************************************************************/
BERR_Code BSAT_GetChannelConfig(
   BSAT_ChannelHandle h,    /* [in] BSAT channel handle */
   uint32_t           addr, /* [in] configuration parameter address */
   uint32_t           *pVal /* [out] value of the configuration parameter */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pVal);
   if (h->pDevice->settings.api.GetChannelConfig)
      return h->pDevice->settings.api.GetChannelConfig(h, addr, pVal);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetAcqDoneEventSettings()
******************************************************************************/
BERR_Code BSAT_SetAcqDoneEventSettings(
   BSAT_Handle h,      /* [in] BSAT handle */
   uint8_t     reacqs  /* [in] number of failed reacquisitions before setting Acq Done event (applies to all channels) */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.SetAcqDoneEventSettings)
      return h->settings.api.SetAcqDoneEventSettings(h, reacqs);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetSignalNotificationSettings()
******************************************************************************/
BERR_Code BSAT_SetSignalNotificationSettings(
   BSAT_ChannelHandle              h,         /* [in] BSAT channel handle */
   BSAT_SignalNotificationSettings *pSettings /* [in] settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.SetSignalNotificationSettings)
      return h->pDevice->settings.api.SetSignalNotificationSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetSignalNotificationSettings()
******************************************************************************/
BERR_Code BSAT_GetSignalNotificationSettings(
   BSAT_ChannelHandle              h,         /* [in] BSAT channel handle */
   BSAT_SignalNotificationSettings *pSettings /* [out] settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.GetSignalNotificationSettings)
      return h->pDevice->settings.api.GetSignalNotificationSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetACIBandwidth()
******************************************************************************/
BERR_Code BSAT_SetACIBandwidth(
   BSAT_ChannelHandle h,  /* [in] BSAT channel handle */
   uint32_t           bw  /* [in] filter bandwidth in Hz */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.SetACIBandwidth)
      return h->pDevice->settings.api.SetACIBandwidth(h, bw);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_StartSignalDetect()
******************************************************************************/
BERR_Code BSAT_StartSignalDetect(
   BSAT_ChannelHandle h,          /* [in] BSAT channel handle */
   uint32_t           symbolRate, /* [in] symbol rate in sym/sec */
   uint32_t           freq,       /* [in] center frequency in Hz */
   uint32_t           adcSelect   /* [in] ADC selection (0-based index) */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.StartSignalDetect)
      return h->pDevice->settings.api.StartSignalDetect(h, symbolRate, freq, adcSelect);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetSignalDetectStatus()
******************************************************************************/
BERR_Code BSAT_GetSignalDetectStatus(
   BSAT_ChannelHandle      h,       /* [in] BSAT channel handle */
   BSAT_SignalDetectStatus *pStatus /* [out] returned status */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pStatus);
   if (h->pDevice->settings.api.GetSignalDetectStatus)
      return h->pDevice->settings.api.GetSignalDetectStatus(h, pStatus);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetLegacyQpskAcqSettings()
******************************************************************************/
BERR_Code BSAT_GetLegacyQpskAcqSettings(
   BSAT_ChannelHandle         h,         /* [in] BSAT channel handle */
   BSAT_LegacyQpskAcqSettings *pSettings /* [out] settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.GetLegacyQpskAcqSettings)
      return h->pDevice->settings.api.GetLegacyQpskAcqSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetLegacyQpskAcqSettings()
******************************************************************************/
BERR_Code BSAT_SetLegacyQpskAcqSettings(
   BSAT_ChannelHandle         h,         /* [in] BSAT channel handle */
   BSAT_LegacyQpskAcqSettings *pSettings /* [in] settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.SetLegacyQpskAcqSettings)
      return h->pDevice->settings.api.SetLegacyQpskAcqSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetDvbs2AcqSettings()
******************************************************************************/
BERR_Code BSAT_GetDvbs2AcqSettings(
   BSAT_ChannelHandle    h,         /* [in] BSAT channel handle */
   BSAT_Dvbs2AcqSettings *pSettings /* [out] settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.GetDvbs2AcqSettings)
      return h->pDevice->settings.api.GetDvbs2AcqSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetDvbs2AcqSettings()
******************************************************************************/
BERR_Code BSAT_SetDvbs2AcqSettings(
   BSAT_ChannelHandle    h,         /* [in] BSAT channel handle */
   BSAT_Dvbs2AcqSettings *pSettings /* [in] settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.SetDvbs2AcqSettings)
      return h->pDevice->settings.api.SetDvbs2AcqSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetTurboAcqSettings()
******************************************************************************/
BERR_Code BSAT_GetTurboAcqSettings(
   BSAT_ChannelHandle    h,         /* [in] BSAT channel handle */
   BSAT_TurboAcqSettings *pSettings /* [out] settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.GetTurboAcqSettings)
      return h->pDevice->settings.api.GetTurboAcqSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetTurboAcqSettings()
******************************************************************************/
BERR_Code BSAT_SetTurboAcqSettings(
   BSAT_ChannelHandle    h,         /* [in] BSAT channel handle */
   BSAT_TurboAcqSettings *pSettings /* [in] settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.SetTurboAcqSettings)
      return h->pDevice->settings.api.SetTurboAcqSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetExtAcqSettings()
******************************************************************************/
BERR_Code BSAT_GetExtAcqSettings(
   BSAT_ChannelHandle    h,       /* [in] BSAT channel handle */
   BSAT_ExtAcqSettings *pSettings /* [out] settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.GetExtAcqSettings)
      return h->pDevice->settings.api.GetExtAcqSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetExtAcqSettings()
******************************************************************************/
BERR_Code BSAT_SetExtAcqSettings(
   BSAT_ChannelHandle  h,         /* [in] BSAT channel handle */
   BSAT_ExtAcqSettings *pSettings /* [in] settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.SetExtAcqSettings)
      return h->pDevice->settings.api.SetExtAcqSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetNotchSettings()
******************************************************************************/
BERR_Code BSAT_SetNotchSettings(
   BSAT_Handle        h,         /* [in] BSAT handle */
   uint32_t           n,         /* number of notches */
   BSAT_NotchSettings *pSettings /* [in] notch filter settings */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.SetNotchSettings)
      return h->settings.api.SetNotchSettings(h, n, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetNotchSettings()
******************************************************************************/
BERR_Code BSAT_GetNotchSettings(
   BSAT_Handle        h,         /* [in] BSAT handle */
   uint32_t           *pNum,     /* number of notches */
   BSAT_NotchSettings *pSettings /* [in] notch filter settings */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.GetNotchSettings)
      return h->settings.api.GetNotchSettings(h, pNum, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetTraceBuffer()
******************************************************************************/
BERR_Code BSAT_GetTraceInfo(
   BSAT_ChannelHandle  h,     /* [in] BSAT channel handle */
   BSAT_TraceInfo      *pInfo /* [out] trace information */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.GetTraceInfo)
      return h->pDevice->settings.api.GetTraceInfo(h, pInfo);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetCwc()
******************************************************************************/
BERR_Code BSAT_SetCwc(
   BSAT_Handle   h,         /* [in] BSAT handle */
   uint32_t      n,         /* [in] number of CWCs */
   uint32_t      *pFreq     /* [in] frequencies in Hz */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.SetCwc)
      return h->settings.api.SetCwc(h, n, pFreq);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetExternalBertSettings()
******************************************************************************/
BERR_Code BSAT_SetExternalBertSettings(
   BSAT_Handle               h,         /* [in] BSAT device handle */
   BSAT_ExternalBertSettings *pSettings /* [in] external BERT configuration  */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.SetExternalBertSettings)
      return h->settings.api.SetExternalBertSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}

/******************************************************************************
 BSAT_GetExternalBertSettings()
******************************************************************************/
BERR_Code BSAT_GetExternalBertSettings(
   BSAT_Handle               h,         /* [in] BSAT device handle */
   BSAT_ExternalBertSettings *pSettings /* [in] external BERT configuration  */
)
{
   BDBG_ASSERT(h);
   if (h->settings.api.GetExternalBertSettings)
      return h->settings.api.GetExternalBertSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetDvbs2xAcqSettings()
******************************************************************************/
BERR_Code BSAT_GetDvbs2xAcqSettings(
   BSAT_ChannelHandle     h,         /* [in] BSAT channel handle */
   BSAT_Dvbs2xAcqSettings *pSettings /* [out] settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.GetDvbs2xAcqSettings)
      return h->pDevice->settings.api.GetDvbs2xAcqSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetDvbs2xAcqSettings()
******************************************************************************/
BERR_Code BSAT_SetDvbs2xAcqSettings(
   BSAT_ChannelHandle     h,         /* [in] BSAT channel handle */
   BSAT_Dvbs2xAcqSettings *pSettings /* [in] settings */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.SetDvbs2xAcqSettings)
      return h->pDevice->settings.api.SetDvbs2xAcqSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_SetAcmSettings()
******************************************************************************/
BERR_Code BSAT_SetAcmSettings(
   BSAT_ChannelHandle h,          /* [in] BSAT channel handle */
   BSAT_AcmSettings   *pSettings  /* [in] ACM settings  */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.SetAcmSettings)
      return h->pDevice->settings.api.SetAcmSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetAcmSettings()
******************************************************************************/
BERR_Code BSAT_GetAcmSettings(
   BSAT_ChannelHandle h,          /* [in] BSAT channel handle */
   BSAT_AcmSettings   *pSettings  /* [out] ACM settings  */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pSettings);
   if (h->pDevice->settings.api.GetAcmSettings)
      return h->pDevice->settings.api.GetAcmSettings(h, pSettings);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetStreamList()
******************************************************************************/
BERR_Code BSAT_GetStreamList(
   BSAT_ChannelHandle h,  /* [in] BSAT channel handle */
   int bufsize,           /* [in] size of the allocated buffer */
   int *pNumStreams,      /* [out] number of stream IDs returned in pStreamIdList */
   uint8_t *pStreamIdList /* [out] array of 8-bit stream IDs */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pStreamIdList);
   BDBG_ASSERT(pNumStreams);
   if (h->pDevice->settings.api.GetStreamList)
      return h->pDevice->settings.api.GetStreamList(h, bufsize, pNumStreams, pStreamIdList);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetStreamStatus()
******************************************************************************/
BERR_Code BSAT_GetStreamStatus(
   BSAT_ChannelHandle h,        /* [in] BSAT channel handle */
   uint8_t            streamId, /* [in] stream ID */
   BSAT_StreamStatus  *pStatus  /* [out] stream status information */
)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pStatus);
   if (h->pDevice->settings.api.GetStreamStatus)
      return h->pDevice->settings.api.GetStreamStatus(h, streamId, pStatus);
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSAT_GetFastChannelStatus()
******************************************************************************/
BERR_Code BSAT_GetFastChannelStatus(
   BSAT_ChannelHandle h,            /* [in] BSAT channel handle */
   BSAT_FastStatusId *pStatusIds,   /* [in] ID of the status items */
   uint8_t numItems,                /* [in] number of status items (0 to 7) */
   BSAT_FastChannelStatus *pStatus  /* [out] returned status */
)
{
   BDBG_ASSERT(h);
   if (h->pDevice->settings.api.GetFastChannelStatus)
      return h->pDevice->settings.api.GetFastChannelStatus(h, pStatusIds, numItems, pStatus);
   else
      return BERR_NOT_SUPPORTED;
}

#endif /* BSAT_EXCLUDE_API_TABLE */
