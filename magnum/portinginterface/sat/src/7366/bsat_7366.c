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
#include "bstd.h"
#include "bsat.h"
#include "bsat_7366.h"
#include "bsat_7366_priv.h"


BDBG_MODULE(bsat_7366);


static const BSAT_Settings defDevSettings =
{
   {
      BSAT_7366_P_Open,
      BSAT_7366_P_Close,
      BSAT_7366_P_GetTotalChannels,
      BSAT_7366_GetChannelDefaultSettings,
      BSAT_7366_P_OpenChannel,
      BSAT_7366_P_CloseChannel,
      BSAT_7366_P_GetDevice,
      BSAT_7366_P_GetVersionInfo,
      BSAT_7366_P_Reset,
      BSAT_7366_P_PowerDownChannel,
      BSAT_7366_P_PowerUpChannel,
      BSAT_7366_P_IsChannelOn,
      BSAT_7366_P_Acquire,
      BSAT_7366_P_GetLockStatus,
      BSAT_7366_P_GetChannelStatus,
      BSAT_7366_P_ResetChannelStatus,
      BSAT_7366_P_GetSoftDecisions,
      BSAT_7366_P_ResetChannel,
      BSAT_7366_P_SetBertSettings,
      BSAT_7366_P_GetBertSettings,
      BSAT_7366_P_GetBertStatus,
      BSAT_7366_P_SetSearchRange,
      BSAT_7366_P_GetSearchRange,
      BSAT_7366_P_SetAmcScramblingSeq,
      BSAT_7366_P_SetNetworkSpec,
      BSAT_7366_P_GetNetworkSpec,
      BSAT_7366_P_SetOutputTransportSettings,
      BSAT_7366_P_GetOutputTransportSettings,
      BSAT_7366_P_GetInitDoneEventHandle,
      BSAT_7366_P_GetLockStateChangeEventHandle,
      BSAT_7366_P_GetAcqDoneEventHandle,
      BSAT_7366_P_GetSignalNotificationEventHandle,
      BSAT_7366_P_GetReadyEventHandle,
      BSAT_7366_P_SetAcqDoneEventSettings,
      BSAT_7366_P_SetSignalNotificationSettings,
      BSAT_7366_P_GetSignalNotificationSettings,
      BSAT_7366_P_StartToneDetect,
      BSAT_7366_P_GetToneDetectStatus,
      BSAT_7366_P_StartSymbolRateScan,
      BSAT_7366_P_GetSymbolRateScanStatus,
      BSAT_7366_P_StartSignalDetect,
      BSAT_7366_P_GetSignalDetectStatus,
      BSAT_7366_P_SetConfig,
      BSAT_7366_P_GetConfig,
      BSAT_7366_P_SetChannelConfig,
      BSAT_7366_P_GetChannelConfig,
      BSAT_7366_P_GetLegacyQpskAcqSettings,
      BSAT_7366_P_SetLegacyQpskAcqSettings,
      BSAT_7366_P_GetDvbs2AcqSettings,
      BSAT_7366_P_SetDvbs2AcqSettings,
      BSAT_7366_P_GetTurboAcqSettings,
      BSAT_7366_P_SetTurboAcqSettings,
      BSAT_7366_P_GetExtAcqSettings,
      BSAT_7366_P_SetExtAcqSettings,
      BSAT_7366_P_SetACIBandwidth,
      BSAT_7366_P_SetNotchSettings,
      BSAT_7366_P_GetNotchSettings,
      BSAT_7366_P_GetTraceInfo,
      BSAT_7366_P_SetCwc,
      NULL, /* BSAT_SetExternalBertSettings */
      NULL, /* BSAT_GetExternalBertSettings */
      NULL, /* BSAT_GetDvb2xAcqSettings */
      NULL, /* BSAT_SetDvb2xAcqSettings */
      BSAT_7366_P_StartPsdScan,
      BSAT_7366_P_GetPsdScanStatus,
      NULL, /* BSAT_SetAcmSettings */
      NULL, /* BSAT_GetAcmSettings */
      NULL, /* BSAT_GetStreamList */
      NULL, /* BSAT_GetStreamStatus */
      NULL, /* BSAT_GetFastChannelStatus */
      BSAT_7366_P_ScanSpectrum,
      BSAT_7366_P_GetSpectrumStatus
   }
};


static const BSAT_ChannelSettings defChnSettings =
{
   0
};


/******************************************************************************
 BSAT_7366_GetDefaultSettings()
******************************************************************************/
BERR_Code BSAT_7366_GetDefaultSettings(
   BSAT_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_7366_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BSAT_7366_GetChannelDefaultSettings(
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
