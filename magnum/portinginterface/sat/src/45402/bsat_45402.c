/******************************************************************************
* Copyright (C) 2018 Broadcom.
* The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to
* the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied),
* right to use, or waiver of any kind with respect to the Software, and
* Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
* THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
* IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization,
* constitutes the valuable trade secrets of Broadcom, and you shall use all
* reasonable efforts to protect the confidentiality thereof, and to use this
* information only in connection with your use of Broadcom integrated circuit
* products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
* "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
* OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
* RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
* IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
* A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
* ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
* THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
* OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
* INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
* RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
* HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
* EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
* WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
* FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* Module Description:
*
*****************************************************************************/
#include "bstd.h"
#include "bhab_45402.h"
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_45402.h"
#include "bsat_45402_priv.h"


BDBG_MODULE(bsat_45402);


static const BSAT_Settings defDevSettings =
{
   {
      BSAT_45402_P_Open,
      BSAT_45402_P_Close,
      BSAT_45402_P_GetTotalChannels,
      BSAT_45402_GetChannelDefaultSettings,
      BSAT_45402_P_OpenChannel,
      BSAT_45402_P_CloseChannel,
      BSAT_45402_P_GetDevice,
      BSAT_45402_P_GetVersionInfo,
      BSAT_45402_P_Reset,
      BSAT_45402_P_PowerDownChannel,
      BSAT_45402_P_PowerUpChannel,
      BSAT_45402_P_IsChannelOn,
      BSAT_45402_P_Acquire,
      BSAT_45402_P_GetLockStatus,
      BSAT_45402_P_GetChannelStatus,
      BSAT_45402_P_ResetChannelStatus,
      BSAT_45402_P_GetSoftDecisions,
      BSAT_45402_P_ResetChannel,
      BSAT_45402_P_SetBertSettings,
      BSAT_45402_P_GetBertSettings,
      BSAT_45402_P_GetBertStatus,
      BSAT_45402_P_SetSearchRange,
      BSAT_45402_P_GetSearchRange,
      BSAT_45402_P_SetAmcScramblingSeq,
      BSAT_45402_P_SetNetworkSpec,
      BSAT_45402_P_GetNetworkSpec,
      BSAT_45402_P_SetOutputTransportSettings,
      BSAT_45402_P_GetOutputTransportSettings,
      BSAT_45402_P_GetInitDoneEventHandle,
      BSAT_45402_P_GetLockStateChangeEventHandle,
      BSAT_45402_P_GetAcqDoneEventHandle,
      BSAT_45402_P_GetSignalNotificationEventHandle,
      BSAT_45402_P_GetReadyEventHandle,
      BSAT_45402_P_SetAcqDoneEventSettings,
      BSAT_45402_P_SetSignalNotificationSettings,
      BSAT_45402_P_GetSignalNotificationSettings,
      BSAT_45402_P_StartToneDetect,
      BSAT_45402_P_GetToneDetectStatus,
      BSAT_45402_P_StartSymbolRateScan,
      BSAT_45402_P_GetSymbolRateScanStatus,
      BSAT_45402_P_StartSignalDetect,
      BSAT_45402_P_GetSignalDetectStatus,
      BSAT_45402_P_SetConfig,
      BSAT_45402_P_GetConfig,
      BSAT_45402_P_SetChannelConfig,
      BSAT_45402_P_GetChannelConfig,
      BSAT_45402_P_GetLegacyQpskAcqSettings,
      BSAT_45402_P_SetLegacyQpskAcqSettings,
      BSAT_45402_P_GetDvbs2AcqSettings,
      BSAT_45402_P_SetDvbs2AcqSettings,
      BSAT_45402_P_GetTurboAcqSettings,
      BSAT_45402_P_SetTurboAcqSettings,
      BSAT_45402_P_GetExtAcqSettings,
      BSAT_45402_P_SetExtAcqSettings,
      BSAT_45402_P_SetACIBandwidth,
      NULL, /* BSAT_45402_P_SetNotchSettings */
      NULL, /* BSAT_45402_P_GetNotchSettings */
      BSAT_45402_P_GetTraceInfo,
      NULL, /* BSAT_SetCwc */
      BSAT_45402_P_SetExternalBertSettings,
      BSAT_45402_P_GetExternalBertSettings,
      BSAT_45402_P_GetDvbs2xAcqSettings,
      BSAT_45402_P_SetDvbs2xAcqSettings,
      BSAT_45402_P_StartPsdScan,
      BSAT_45402_P_GetPsdScanStatus,
      BSAT_45402_P_SetAcmSettings,
      BSAT_45402_P_GetAcmSettings,
      BSAT_45402_P_GetStreamList,
      BSAT_45402_P_GetStreamStatus,
      NULL, /* BSAT_GetFastChannelStatus */
      BSAT_45402_P_ScanSpectrum,
      BSAT_45402_P_GetSpectrumStatus
   }
};


static const BSAT_ChannelSettings defChnSettings =
{
   0
};


/******************************************************************************
 BSAT_45402_GetDefaultSettings()
******************************************************************************/
BERR_Code BSAT_45402_GetDefaultSettings(
   BSAT_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_45402_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BSAT_45402_GetChannelDefaultSettings(
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


/******************************************************************************
 BSAT_45402_GetBp3Session()
******************************************************************************/
BERR_Code BSAT_45402_GetBp3SessionInfo(BSAT_Handle h, BSAT_45402_Bp3SessionInfo *pSessionInfo)
{
   BERR_Code retCode;
   BSAT_45402_P_Handle *pImpl = (BSAT_45402_P_Handle *)(h->pImpl);
   uint32_t hab[6];

   BDBG_ENTER(BSAT_45402_GetBp3SessionInfo);

   BKNI_Memset(hab, 0, 6*sizeof(uint32_t));
   hab[0] = BHAB_45402_InitHeader(0x3D, 0, 0, 0);
   BSAT_45402_CHK_RETCODE(BSAT_45402_P_SendCommand(pImpl->hHab, hab, 6));

   pSessionInfo->sessionToken[0] = (uint8_t)((hab[1] >> 24) & 0xFF);
   pSessionInfo->sessionToken[1] = (uint8_t)((hab[1] >> 16) & 0xFF);
   pSessionInfo->sessionToken[2] = (uint8_t)((hab[1] >> 8) & 0xFF);
   pSessionInfo->sessionToken[3] = (uint8_t)(hab[1] & 0xFF);
   pSessionInfo->sessionToken[4] = (uint8_t)((hab[2] >> 24) & 0xFF);
   pSessionInfo->sessionToken[5] = (uint8_t)((hab[2] >> 16) & 0xFF);
   pSessionInfo->sessionToken[6] = (uint8_t)((hab[2] >> 8) & 0xFF);
   pSessionInfo->sessionToken[7] = (uint8_t)(hab[2] & 0xFF);
   pSessionInfo->otpId[0] = (uint8_t)((hab[3] >> 24) & 0xFF);
   pSessionInfo->otpId[1] = (uint8_t)((hab[3] >> 16) & 0xFF);
   pSessionInfo->otpId[2] = (uint8_t)((hab[3] >> 8) & 0xFF);
   pSessionInfo->otpId[3] = (uint8_t)(hab[3] & 0xFF);
   pSessionInfo->rootKeyDerivation = (uint8_t)(hab[4] & 0xFF);
   pSessionInfo->klDerivation = (uint8_t)((hab[4] >> 8) & 0xFF);

   done:
   BDBG_LEAVE(BSAT_45402_GetBp3SessionInfo);
   return retCode;
}


/******************************************************************************
 BSAT_45402_ConfigBp3()
******************************************************************************/
BERR_Code BSAT_45402_ConfigBp3(BSAT_Handle h, uint8_t *pEncryptedData)
{
   BERR_Code retCode;
   BSAT_45402_P_Handle *pImpl = (BSAT_45402_P_Handle *)(h->pImpl);
   uint32_t hab[10];
   int i;
   uint8_t *pData;

   BDBG_ENTER(BSAT_45402_ConfigBp3);
   BKNI_Memset(hab, 0, 10*sizeof(uint32_t));
   hab[0] = BHAB_45402_InitHeader(0x48, 0, 0, 0);
   for (pData = pEncryptedData, i = 0; i < 8; pData += 4, i++)
      hab[1+i] = (pData[0] << 24) | (pData[1] << 16) | (pData[2] << 8) | pData[3];

   retCode = BSAT_45402_P_SendCommand(pImpl->hHab, hab, 10);
   BDBG_LEAVE(BSAT_45402_ConfigBp3);
   return retCode;
}
