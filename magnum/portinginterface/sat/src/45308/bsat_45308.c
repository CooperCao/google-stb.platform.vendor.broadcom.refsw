/******************************************************************************
*    (c)2011-2013 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*
* Revision History:
*
* $brcm_Log: $
*
*****************************************************************************/
#include "bstd.h"
#include "bsat.h"
#include "bsat_45308.h"
#include "bsat_45308_priv.h"


BDBG_MODULE(bsat_45308);


static const BSAT_Settings defDevSettings =
{
   {
      BSAT_45308_P_Open,
      BSAT_45308_P_Close,
      BSAT_45308_P_GetTotalChannels,
      BSAT_45308_GetChannelDefaultSettings,
      BSAT_45308_P_OpenChannel,
      BSAT_45308_P_CloseChannel,
      BSAT_45308_P_GetDevice,
      BSAT_45308_P_GetVersionInfo,
      BSAT_45308_P_Reset,
      BSAT_45308_P_PowerDownChannel,
      BSAT_45308_P_PowerUpChannel,
      BSAT_45308_P_IsChannelOn,
      BSAT_45308_P_Acquire,
      BSAT_45308_P_GetLockStatus,
      BSAT_45308_P_GetChannelStatus,
      BSAT_45308_P_ResetChannelStatus,
      BSAT_45308_P_GetSoftDecisions,
      BSAT_45308_P_ResetChannel,
      BSAT_45308_P_SetBertSettings,
      BSAT_45308_P_GetBertSettings,
      BSAT_45308_P_GetBertStatus,
      BSAT_45308_P_SetSearchRange,
      BSAT_45308_P_GetSearchRange,
      BSAT_45308_P_SetAmcScramblingSeq,
      BSAT_45308_P_SetNetworkSpec,
      BSAT_45308_P_GetNetworkSpec,
      BSAT_45308_P_SetOutputTransportSettings,
      BSAT_45308_P_GetOutputTransportSettings,
      BSAT_45308_P_GetInitDoneEventHandle,
      BSAT_45308_P_GetLockStateChangeEventHandle,
      BSAT_45308_P_GetAcqDoneEventHandle,
      BSAT_45308_P_GetSignalNotificationEventHandle,
      BSAT_45308_P_GetReadyEventHandle,
      BSAT_45308_P_SetAcqDoneEventSettings,
      BSAT_45308_P_SetSignalNotificationSettings,
      BSAT_45308_P_GetSignalNotificationSettings,
      BSAT_45308_P_StartToneDetect,
      BSAT_45308_P_GetToneDetectStatus,
      BSAT_45308_P_StartSymbolRateScan,
      BSAT_45308_P_GetSymbolRateScanStatus,
      BSAT_45308_P_StartSignalDetect,
      BSAT_45308_P_GetSignalDetectStatus,
      BSAT_45308_P_SetConfig,
      BSAT_45308_P_GetConfig,
      BSAT_45308_P_SetChannelConfig,
      BSAT_45308_P_GetChannelConfig,
      BSAT_45308_P_GetLegacyQpskAcqSettings,
      BSAT_45308_P_SetLegacyQpskAcqSettings,
      BSAT_45308_P_GetDvbs2AcqSettings,
      BSAT_45308_P_SetDvbs2AcqSettings,
      BSAT_45308_P_GetTurboAcqSettings,
      BSAT_45308_P_SetTurboAcqSettings,
      BSAT_45308_P_GetExtAcqSettings,
      BSAT_45308_P_SetExtAcqSettings,
      BSAT_45308_P_SetACIBandwidth,
      BSAT_45308_P_SetNotchSettings,
      BSAT_45308_P_GetNotchSettings,
      BSAT_45308_P_GetTraceInfo,
      NULL,
      BSAT_45308_P_SetExternalBertSettings,
      BSAT_45308_P_GetExternalBertSettings,
      BSAT_45308_P_GetDvbs2xAcqSettings,
      BSAT_45308_P_SetDvbs2xAcqSettings,
      BSAT_45308_P_StartPsdScan,
      BSAT_45308_P_GetPsdScanStatus,
      BSAT_45308_P_SetAcmSettings,
      BSAT_45308_P_GetAcmSettings,
      BSAT_45308_P_GetStreamList,
      BSAT_45308_P_GetStreamStatus
   }
};


static const BSAT_ChannelSettings defChnSettings =
{
   0
};


/******************************************************************************
 BSAT_45308_GetDefaultSettings()
******************************************************************************/
BERR_Code BSAT_45308_GetDefaultSettings(
   BSAT_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_45308_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BSAT_45308_GetChannelDefaultSettings(
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
