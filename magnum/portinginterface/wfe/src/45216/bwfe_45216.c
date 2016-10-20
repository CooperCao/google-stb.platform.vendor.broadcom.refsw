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
#include "bwfe_45216.h"
#include "bwfe_45216_priv.h"


BDBG_MODULE(bwfe_45216);


static const BWFE_Settings defDevSettings =
{
   /* API function table */
   {
      BWFE_45216_P_Open,
      BWFE_45216_P_Close,
      BWFE_45216_P_GetTotalChannels,
      NULL,    /* BWFE_GetTotalRefChannels */
      BWFE_45216_GetChannelDefaultSettings,
      BWFE_45216_P_OpenChannel,
      BWFE_45216_P_CloseChannel,
      BWFE_45216_P_Reset,
      BWFE_45216_P_GetVersion,
      BWFE_45216_P_IsInputEnabled,
      BWFE_45216_P_EnableInput,
      BWFE_45216_P_DisableInput,
      NULL,    /* BWFE_GetChannelStatus */
      NULL,    /* DEBUG: BWFE_GetLicTaps */
      NULL,    /* DEBUG: BWFE_GetEqTaps */
      NULL,    /* BWFE_FreezeRfAgc */
      NULL,    /* BWFE_UnfreezeRfAgc */
      NULL,    /* BWFE_RunLaneCorrection */
      NULL,    /* BWFE_CalibrateINL */
      NULL,    /* BWFE_EqualizePipeline */
      NULL,    /* DEBUG: BWFE_SetAdcSampleFreq */
      NULL,    /* DEBUG: BWFE_GetAdcSampleFreq */
      NULL,    /* BWFE_ReadConfig */
      NULL,    /* BWFE_WriteConfig */
      NULL,    /* BWFE_ResetDgsLut */
      NULL,    /* BWFE_ResetEqTaps */
      NULL,    /* BWFE_ResetLicTaps */
      NULL,    /* BWFE_CancelDCOffset */
      BWFE_45216_P_GetWfeReadyEventHandle,
      NULL,    /* BWFE_CalibrateAnalogDelay */
      NULL,    /* BWFE_GetAnalogDelay */
      BWFE_45216_P_GetSaDoneEventHandle,
      BWFE_45216_P_ScanSpectrum,
      BWFE_45216_P_GetSaSamples,
      NULL,    /* BWFE_MirrorEquRefTaps */
      NULL,    /* BWFE_CalibrateAdcPhase */
      NULL     /* BWFE_CompensateDelay */
   },
   BWFE_CHIP_SVT_SIGMA_NOM_TO_ONE_FAST,
   54000 /* default xtal freq */
};


static const BWFE_ChannelSettings defChanSettings =
{
   false
};


/******************************************************************************
 BWFE_45216_GetDefaultSettings()
******************************************************************************/
BERR_Code BWFE_45216_GetDefaultSettings(
   BWFE_Settings *pDefSettings   /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_45216_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BWFE_45216_GetChannelDefaultSettings(
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
