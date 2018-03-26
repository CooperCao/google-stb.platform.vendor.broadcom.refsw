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
 *
 * Module Description:
 *
 *****************************************************************************/
#include "bhab.h"
#include "bhab_priv.h"
#include "bhab_7255_priv.h"

BDBG_MODULE(bhab_7255);


static const BHAB_Settings defDevSettings =
{
    0x24, /* SPI chipAddr */
    NULL, /* interruptEnableFunc */
    NULL, /* interruptEnableFuncParam */
    /* API function table */
    {
        BHAB_7255_Open,
        BHAB_7255_Close,
        BHAB_7255_InitAp,
        BHAB_7255_GetApStatus,
        BHAB_7255_GetApVersion,
        BHAB_7255_GetVersionInfo,
        BHAB_7255_ReadRegister,
        BHAB_7255_WriteRegister,
        BHAB_7255_ReadMemory,
        BHAB_7255_WriteMemory,
        NULL,
        NULL,
        BHAB_7255_HandleInterrupt_isr,
        BHAB_7255_ProcessInterruptEvent,
        BHAB_7255_EnableLockInterrupt,
        BHAB_7255_InstallInterruptCallback,
        BHAB_7255_UnInstallInterruptCallback,
        BHAB_7255_SendHabCommand,
        BHAB_7255_GetInterruptEventHandle,
        NULL,
        NULL,
        NULL,
        NULL,
        BHAB_7255_GetConfigSettings,
        BHAB_7255_SetConfigSettings,
        NULL, /* BHAB_ReadSlave */
        NULL, /* BHAB_WriteSlave */
        BHAB_7255_GetInternalGain,
        BHAB_7255_GetExternalGain,
        BHAB_7255_SetExternalGain,
        BHAB_7255_GetAvsData,
        BHAB_7255_GetTunerChannels,
        BHAB_7255_GetCapabilities,
        BHAB_7255_Reset,
        NULL, /* BHAB_GetRecalibrateSettings */
        NULL, /* BHAB_SetRecalibrateSettings */
        NULL /* BHAB_GetLnaStatus */
    },
    0x60, /* slaveChipAddr */
    false, /* isSpi */
    false, /* isMtsif */
    NULL, /* pImgInterface */
    NULL, /* pImgContext */
    NULL  /* pChp */
};


/******************************************************************************
 BHAB_7255_GetDefaultSettings()
******************************************************************************/
BERR_Code BHAB_7255_GetDefaultSettings(
    BHAB_Settings *pDefSettings /* [out] default settings */
)
{
    *pDefSettings = defDevSettings;
    return BERR_SUCCESS;
}

/******************************************************************************
 BHAB_7255_Configure()
******************************************************************************/
BERR_Code BHAB_7255_Configure(BHAB_Handle h, BHAB_7255_Settings *pSettings)
{
    BHAB_7255_P_Handle *pImpl = (BHAB_7255_P_Handle *)(h->pImpl);

    if (pSettings->bUseInternalMemory == false)
    {
      if ((pSettings->physAddr & 0xFFFFF) || (pSettings->pRam == NULL)) {
         BDBG_ERR(("BHAB_7255_Configure: BERR_INVALID_PARAMETER"));
         return BERR_INVALID_PARAMETER;
      }
    }

    BKNI_Memcpy((void*)&(pImpl->settings), (void*)pSettings, sizeof(BHAB_7255_Settings));

   /* BREG_Write32(pImpl->hRegHandle, BCHP_LEAP_CTRL_GP44, pImpl->settings.physAddr); TODO is this used??*/
    BREG_Write32(pImpl->hRegHandle, BCHP_LEAP_CTRL_ADDR_TRANS, (pImpl->settings.physAddr & 0xFFF00000) | 0x1);

    return BERR_SUCCESS;
}
