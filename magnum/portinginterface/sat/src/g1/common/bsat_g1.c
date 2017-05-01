/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bsat_g1.h"
#include "bsat_g1_priv.h"


BDBG_MODULE(bsat_g1);


static const BSAT_Settings defDevSettings =
{
   {
      BSAT_g1_P_Open,
      BSAT_g1_P_Close,
      BSAT_g1_P_GetTotalChannels,
      BSAT_g1_GetChannelDefaultSettings,
      BSAT_g1_P_OpenChannel,
      BSAT_g1_P_CloseChannel,
      BSAT_g1_P_GetDevice,
      BSAT_g1_P_GetVersionInfo,
      BSAT_g1_P_Reset,
      BSAT_g1_P_PowerDownChannel,
      BSAT_g1_P_PowerUpChannel,
      BSAT_g1_P_IsChannelOn,
      BSAT_g1_P_Acquire,
      BSAT_g1_P_GetLockStatus,
      BSAT_g1_P_GetChannelStatus,
      BSAT_g1_P_ResetChannelStatus,
      BSAT_g1_P_GetSoftDecisions,
      BSAT_g1_P_ResetChannel,
      BSAT_g1_P_SetBertSettings,
      BSAT_g1_P_GetBertSettings,
      BSAT_g1_P_GetBertStatus,
      BSAT_g1_P_SetSearchRange,
      BSAT_g1_P_GetSearchRange,
#ifndef BSAT_EXCLUDE_AFEC
      BSAT_g1_P_SetAmcScramblingSeq,
#else
      NULL,
#endif
      BSAT_g1_P_SetNetworkSpec,
      BSAT_g1_P_GetNetworkSpec_isrsafe,
      BSAT_g1_P_SetOutputTransportSettings,
      BSAT_g1_P_GetOutputTransportSettings,
      BSAT_g1_P_GetInitDoneEventHandle,
      BSAT_g1_P_GetLockStateChangeEventHandle,
      BSAT_g1_P_GetAcqDoneEventHandle,
      BSAT_g1_P_GetSignalNotificationEventHandle,
      BSAT_g1_P_GetReadyEventHandle,
      BSAT_g1_P_SetAcqDoneEventSettings,
      BSAT_g1_P_SetSignalNotificationSettings,
      BSAT_g1_P_GetSignalNotificationSettings,
      BSAT_g1_P_StartToneDetect,
      BSAT_g1_P_GetToneDetectStatus,
      BSAT_g1_P_StartSymbolRateScan,
      BSAT_g1_P_GetSymbolRateScanStatus,
      BSAT_g1_P_StartSignalDetect,
      BSAT_g1_P_GetSignalDetectStatus,
      BSAT_g1_P_SetConfig,
      BSAT_g1_P_GetConfig,
      BSAT_g1_P_SetChannelConfig,
      BSAT_g1_P_GetChannelConfig,
      BSAT_g1_P_GetLegacyQpskAcqSettings,
      BSAT_g1_P_SetLegacyQpskAcqSettings,
#ifndef BSAT_EXCLUDE_AFEC
      BSAT_g1_P_GetDvbs2AcqSettings,
      BSAT_g1_P_SetDvbs2AcqSettings,
#else
      NULL,
      NULL,
#endif
#ifndef BSAT_EXCLUDE_TFEC
      BSAT_g1_P_GetTurboAcqSettings,
      BSAT_g1_P_SetTurboAcqSettings,
#else
      NULL,
      NULL,
#endif
      BSAT_g1_P_GetExtAcqSettings,
      BSAT_g1_P_SetExtAcqSettings,
      BSAT_g1_P_SetACIBandwidth,
#ifdef BSAT_HAS_WFE
      BSAT_g1_P_SetNotchSettings,
      BSAT_g1_P_GetNotchSettings,
#else
      NULL,
      NULL,
#endif
      BSAT_g1_P_GetTraceInfo,
#ifndef BSAT_EXCLUDE_SPUR_CANCELLER
      BSAT_g1_P_SetCwc,
#else
      NULL,
#endif
      NULL, /* BSAT_SetExternalBertSettings */
      NULL, /* BSAT_GetExternalBertSettings */
#ifdef BSAT_HAS_DVBS2X
      BSAT_g1_P_GetDvbs2xAcqSettings,
      BSAT_g1_P_SetDvbs2xAcqSettings,
#else
      NULL, /* BSAT_GetDvb2xAcqSettings */
      NULL, /* BSAT_SetDvb2xAcqSettings */
#endif
      BSAT_g1_P_StartPsdScan,
      BSAT_g1_P_GetPsdScanStatus,
      BSAT_g1_P_SetAcmSettings,
      BSAT_g1_P_GetAcmSettings,
      BSAT_g1_P_GetStreamList,
      BSAT_g1_P_GetStreamStatus,
      NULL, /* BSAT_GetFastChannelStatus */
      BSAT_g1_P_ScanSpectrum,
      BSAT_g1_P_GetSpectrumStatus
   }
};


static const BSAT_ChannelSettings defChnSettings =
{
   (uint32_t)0
};


/******************************************************************************
 BSAT_g1_GetDefaultSettings()
******************************************************************************/
BERR_Code BSAT_g1_GetDefaultSettings(
   BSAT_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BSAT_g1_GetChannelDefaultSettings(
   BSAT_Handle   h,                      /* [in] BSAT handle */
   uint32_t      chnNo,                  /* [in] channel number */
   BSAT_ChannelSettings *pChnDefSettings /* [out] default channel settings */
)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(chnNo);
   *pChnDefSettings = defChnSettings;
   return BERR_SUCCESS;
}
