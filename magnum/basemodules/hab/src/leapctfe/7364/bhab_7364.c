/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
#include "bhab.h"
#include "bhab_priv.h"
#include "bhab_7364.h"
#include "bhab_leap_priv.h"

BDBG_MODULE(bhab_leap);


static const BHAB_Settings defDevSettings =
{
    0x6c, /* chipAddr */
    NULL, /* interruptEnableFunc */
    NULL, /* interruptEnableFuncParam */
    /* API function table */
    {
        BHAB_Leap_Open,
        BHAB_Leap_Close,
        BHAB_Leap_InitAp,
        BHAB_Leap_GetApStatus,
        BHAB_Leap_GetApVersion,
        BHAB_Leap_GetVersionInfo,
        BHAB_Leap_ReadRegister,
        BHAB_Leap_WriteRegister,
        BHAB_Leap_ReadMemory,
        BHAB_Leap_WriteMemory,
        NULL,
        NULL,
        BHAB_Leap_HandleInterrupt_isr,
        BHAB_Leap_ProcessInterruptEvent,
        BHAB_Leap_EnableLockInterrupt,
        BHAB_Leap_InstallInterruptCallback,
        BHAB_Leap_UnInstallInterruptCallback,
        BHAB_Leap_SendHabCommand,
        BHAB_Leap_GetInterruptEventHandle,
        NULL,
        NULL,
        NULL,
        NULL,
        BHAB_Leap_GetConfigSettings,
        BHAB_Leap_SetConfigSettings,
        NULL,
        NULL,
        BHAB_Leap_GetInternalGain,
        BHAB_Leap_GetExternalGain,
        BHAB_Leap_SetExternalGain,
        NULL, /* BHAB_GetAvsData */
        BHAB_Leap_GetTunerChannels,
        BHAB_Leap_GetCapabilities,
        BHAB_Leap_Reset,
        NULL, /* BHAB_GetRecalibrateSettings */
        NULL, /* BHAB_SetRecalibrateSettings */
        NULL /* BHAB_GetLnaStatus */
    },
    0x6c, /* slaveChipAddr */
    false, /* isSpi */
    false, /* isMtsif */
    NULL, /* pImgInterface */
    NULL, /* pImgContext */
    NULL  /* pChp */
};


/******************************************************************************
 BHAB_7364_GetDefaultSettings()
******************************************************************************/
BERR_Code BHAB_7364_GetDefaultSettings(
    BHAB_Settings *pDefSettings /* [out] default settings */
)
{
    *pDefSettings = defDevSettings;
    return BERR_SUCCESS;
}

/******************************************************************************
 BHAB_7364_Configure()
******************************************************************************/
BERR_Code BHAB_7364_Configure(BHAB_Handle h, BHAB_7364_Settings *pSettings)
{
    BHAB_Leap_P_Handle *pImpl = (BHAB_Leap_P_Handle *)(h->pImpl);

    if (pSettings->bUseInternalMemory == false)
    {
      if ((pSettings->physAddr & 0xFFFFF) || (pSettings->pRam == NULL))
         return BERR_INVALID_PARAMETER;
    }

    BKNI_Memcpy((void*)&(pImpl->settings), (void*)pSettings, sizeof(BHAB_7364_Settings));

    BDBG_MSG(("LEAP RAM: physAddr=0x%08X, virtAddr=0x%08X", pImpl->settings.physAddr, pImpl->settings.pRam));
    BREG_Write32(pImpl->hRegHandle, BCHP_LEAP_CTRL_GP44, pImpl->settings.physAddr);
    BREG_Write32(pImpl->hRegHandle, BCHP_LEAP_CTRL_ADDR_TRANS, (pImpl->settings.physAddr & 0xFFF00000) | 0x1);

    return BERR_SUCCESS;
}
