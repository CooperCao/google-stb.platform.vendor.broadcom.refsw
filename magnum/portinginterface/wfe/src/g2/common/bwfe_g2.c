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
#include "bwfe_g2.h"
#include "bwfe_g2_priv.h"


BDBG_MODULE(bwfe_g2);


static const BWFE_Settings defDevSettings =
{
   /* API function table */
   {
      BWFE_g2_P_Open,
      BWFE_g2_P_Close,
      BWFE_g2_P_GetTotalChannels,
      BWFE_g2_P_GetTotalRefChannels,
      BWFE_g2_GetChannelDefaultSettings,
      BWFE_g2_P_OpenChannel,
      BWFE_g2_P_CloseChannel,
      BWFE_g2_P_Reset,
      BWFE_g2_P_GetVersion,
      BWFE_g2_P_IsInputEnabled,
      BWFE_g2_P_EnableInput,
      BWFE_g2_P_DisableInput,
      BWFE_g2_P_GetChannelStatus,
      BWFE_g2_P_GetLicTaps,  /* debug */
      BWFE_g2_P_GetEqTaps,   /* debug */
      BWFE_g2_P_FreezeRfAgc,
      BWFE_g2_P_UnfreezeRfAgc,
      BWFE_g2_P_RunLaneCorrection,
      BWFE_g2_P_CalibrateINL,
      BWFE_g2_P_EqualizePipeline,
      BWFE_g2_P_SetAdcSampleFreq,  /* debug */
      BWFE_g2_P_GetAdcSampleFreq,  /* debug */
      BWFE_g2_P_ReadConfig,
      BWFE_g2_P_WriteConfig,
      BWFE_g2_P_ResetDgsLut,
      BWFE_g2_P_ResetEqTaps,
      BWFE_g2_P_ResetLicTaps,
      BWFE_g2_P_CancelDCOffset,
      BWFE_g2_P_GetWfeReadyEventHandle,
      BWFE_g2_P_CalibrateAnalogDelay,
      BWFE_g2_P_GetAnalogDelay,
      BWFE_g2_P_GetSaDoneEventHandle,
      BWFE_g2_P_ScanSpectrum,
      BWFE_g2_P_GetSaSamples,
      BWFE_g2_P_MirrorEquRefTaps,
      BWFE_g2_P_CalibrateAdcPhase,
      NULL
   },
   BWFE_CHIP_SVT_SIGMA_NOM_TO_ONE_FAST
};


static const BWFE_ChannelSettings defChanSettings =
{
   (bool)0
};


/******************************************************************************
 BWFE_g2_GetDefaultSettings()
******************************************************************************/
BERR_Code BWFE_g2_GetDefaultSettings(
   BWFE_Settings *pDefSettings   /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g2_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BWFE_g2_GetChannelDefaultSettings(
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

