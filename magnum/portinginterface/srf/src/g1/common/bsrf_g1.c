/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
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
      BSRF_g1_P_WriteRfGain,
      BSRF_g1_P_ReadRfGain,
      BSRF_g1_P_GetInputPower,
      BSRF_g1_P_SetRfAgcSettings,
      BSRF_g1_P_GetRfAgcSettings,
      BSRF_g1_P_EnableFastDecayMode,
      BSRF_g1_P_SetFastDecayGainThreshold,
      BSRF_g1_P_GetFastDecayGainThreshold,
      BSRF_g1_P_SetAntennaOverThreshold,
      BSRF_g1_P_GetAntennaOverThreshold,
      BSRF_g1_P_SetAntennaDetectThreshold,
      BSRF_g1_P_GetAntennaDetectThreshold,
      BSRF_g1_P_GetAntennaStatus,
      BSRF_g1_Ana_P_PowerUpAntennaSense,
      BSRF_g1_Ana_P_PowerDownAntennaSense,
      BSRF_g1_P_Tune,
      BSRF_g1_P_Notch,
      BSRF_g1_P_GetTunerStatus,
      BSRF_g1_P_ResetClipCount,
      BSRF_g1_P_GetClipCount,
      BSRF_g1_P_ConfigTestMode,
      BSRF_g1_P_ConfigOutput,
      BSRF_g1_P_ConfigTestDac,
      BSRF_g1_P_EnableTestDac,
      BSRF_g1_P_DisableTestDac,
      BSRF_g1_P_EnableTestDacTone,
      BSRF_g1_P_RunDataCapture,
      BSRF_g1_P_DeleteAgcLutCodes,
      BSRF_g1_P_ConfigOutputClockPhase,
      BSRF_g1_P_SetIqEqCoeff,
      BSRF_g1_P_SetIqEqSettings,
      BSRF_g1_P_BoostMixPllCurrent
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
