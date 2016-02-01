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
#include "bdsq.h"
#include "bdsq_45216.h"
#include "bdsq_45216_priv.h"


BDBG_MODULE(bdsq_45216);


static const BDSQ_Settings defDevSettings =
{
   {
      BDSQ_45216_P_Open,
      BDSQ_45216_P_Close,
      BDSQ_45216_P_GetTotalChannels,
      BDSQ_45216_GetChannelDefaultSettings,
      BDSQ_45216_P_OpenChannel,
      BDSQ_45216_P_CloseChannel,
      BDSQ_45216_P_GetDevice,
      BDSQ_45216_P_GetVersionInfo,
      BDSQ_45216_P_Reset,
      BDSQ_45216_P_ResetChannel,
      BDSQ_45216_P_PowerDownChannel,
      BDSQ_45216_P_PowerUpChannel,
      BDSQ_45216_P_IsChannelOn,
      BDSQ_45216_P_PowerDownVsense,
      BDSQ_45216_P_PowerUpVsense,
      BDSQ_45216_P_IsVsenseOn,
      BDSQ_45216_P_SetTone,
      BDSQ_45216_P_GetTone,
      BDSQ_45216_P_SetVoltage,
      BDSQ_45216_P_GetVoltage,
      BDSQ_45216_P_EnableVsenseInterrupt,
      NULL, /* BDSQ_45216_P_EnableActivityInterrupt */
      NULL, /* BDSQ_45216_P_EnableIdleInterrupt */
      BDSQ_45216_P_Write,
      BDSQ_45216_P_EnableRx,
      NULL, /* BDSQ_45216_P_EnableLnb */
      BDSQ_45216_P_Read,
      BDSQ_45216_P_SendAcw,
      BDSQ_45216_P_GetStatus,
      NULL, /* BDSQ_45216_P_GetActivityStatus */
      NULL, /* BDSQ_45216_P_ResetActivityStatus */
      BDSQ_45216_P_SetChannelSettings,
      BDSQ_45216_P_GetChannelSettings,
      BDSQ_45216_P_SetConfig,
      BDSQ_45216_P_GetConfig,
      BDSQ_45216_P_SetChannelConfig,
      BDSQ_45216_P_GetChannelConfig,
      BDSQ_45216_P_GetTxEventHandle,
      BDSQ_45216_P_GetRxEventHandle,
      BDSQ_45216_P_GetVsenseEventHandle,
      NULL, /* BDSQ_45216_P_GetActivityEventHandle */
      NULL  /* BDSQ_45216_P_GetIdleEventHandle */
   }
};


static const BDSQ_ChannelSettings defChnSettings =
{
   true,    /* bEnvelope */
   true,    /* bToneAlign */
   false,   /* bDisableRRTO */
   false,   /* bEnableToneburst */
   true,    /* bToneburstB */
   false,   /* bOverrideFraming */
   false,   /* bExpectReply */
   false,   /* TBD unused */
   false    /* bAdaptiveRxSlice */
};


/******************************************************************************
 BDSQ_45216_GetDefaultSettings()
******************************************************************************/
BERR_Code BDSQ_45216_GetDefaultSettings(
   BDSQ_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


/******************************************************************************
 BDSQ_45216_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BDSQ_45216_GetChannelDefaultSettings(
   BDSQ_Handle   h,                      /* [in] BDSQ handle */
   uint32_t      chnNo,                  /* [in] channel number */
   BDSQ_ChannelSettings *pChnDefSettings /* [out] default channel settings */
)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(chnNo);
   *pChnDefSettings = defChnSettings;
   return BERR_SUCCESS;
}
