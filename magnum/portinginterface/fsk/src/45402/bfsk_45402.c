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
#include "bstd.h"
#include "bfsk.h"
#include "bfsk_45402.h"
#include "bfsk_45402_priv.h"


BDBG_MODULE(bfsk_45402);


static const BFSK_Settings defDevSettings =
{
   {
      BFSK_45402_P_Open,
      BFSK_45402_P_Close,
      BFSK_45402_P_GetTotalChannels,
      BFSK_45402_GetChannelDefaultSettings,
      BFSK_45402_P_OpenChannel,
      BFSK_45402_P_CloseChannel,
      BFSK_45402_P_GetDevice,
      BFSK_45402_P_GetVersionInfo,
      BFSK_45402_P_Reset,
      BFSK_45402_P_ResetChannel,
      BFSK_45402_P_PowerDownChannel,
      BFSK_45402_P_PowerUpChannel,
      BFSK_45402_P_IsChannelOn,
      BFSK_45402_P_EnableCarrier,
      BFSK_45402_P_DisableCarrier,
      BFSK_45402_P_Write,
      BFSK_45402_P_SetRxCount,
      BFSK_45402_P_Read,
      BFSK_45402_P_GetRssiLevel,
      NULL, /* BFSK_45402_P_SetRssiThreshold */
      NULL, /* BFSK_45402_P_GetRssiThreshold */
      BFSK_45402_P_GetRxEventHandle,
      BFSK_45402_P_GetTxEventHandle,
      BFSK_45402_P_SetConfig,
      BFSK_45402_P_GetConfig,
      BFSK_45402_P_SetChannelConfig,
      BFSK_45402_P_GetChannelConfig,
      NULL, /* BFSK_45402_P_GetActivityEventHandle */
      NULL  /* BFSK_45402_P_EnableActivityInterrupt */
   }
};


static const BFSK_ChannelSettings defChnSettings =
{
   0
};


/******************************************************************************
 BFSK_45402_GetDefaultSettings()
******************************************************************************/
BERR_Code BFSK_45402_GetDefaultSettings(
   BFSK_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BFSK_45402_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BFSK_45402_GetChannelDefaultSettings(
   BFSK_Handle   h,                      /* [in] BFSK handle */
   uint32_t      chnNo,                  /* [in] channel number */
   BFSK_ChannelSettings *pChnDefSettings /* [out] default channel settings */
)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(chnNo);
   *pChnDefSettings = defChnSettings;
   return BERR_SUCCESS;
}
