/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bhab.h"
#include "bads.h"
#include "bads_priv.h"
#include "bads_3466.h"
#include "bads_3466_priv.h"
#include "bchp_3466_leap_ctrl.h"

BDBG_MODULE(bads_3466);

/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BADS_Settings defDevSettings =
{
    BHAB_DevId_eADS0,
    NULL,                   /* BHAB handle, must be provided by application*/
    {
        BADS_3466_Open,
        BADS_3466_Close,
        BADS_3466_Init,
        BADS_3466_GetVersion,
        BADS_3466_GetVersionInfo,
        NULL,
        BADS_3466_GetTotalChannels,
        BADS_3466_OpenChannel,
        BADS_3466_CloseChannel,
        BADS_3466_GetDevice,
        BADS_3466_GetChannelDefaultSettings,
        BADS_3466_GetStatus,
        BADS_3466_GetLockStatus,
        BADS_3466_GetSoftDecision,
        BADS_3466_InstallCallback,
        BADS_3466_GetDefaultAcquireParams,
        BADS_3466_SetAcquireParams,
        BADS_3466_GetAcquireParams,
        BADS_3466_Acquire,
        BADS_3466_EnablePowerSaver,
        BADS_3466_DisablePowerSaver,
        BADS_3466_ProcessNotification,
        BADS_3466_SetDaisyChain,
        BADS_3466_GetDaisyChain,
        BADS_3466_ResetStatus,
        NULL,
        NULL,
        NULL,
        BADS_3466_RequestAsyncStatus,
        BADS_3466_GetAsyncStatus,
        BADS_3466_GetScanStatus,
        BADS_3466_ReadSlave,
        BADS_3466_WriteSlave,
        BADS_3466_SetScanParam,
        BADS_3466_GetScanParam,
        NULL,
        NULL
    },
    false,
    BADS_TransportData_eGpioSerial,
    NULL,
    NULL
};

/***************************************************************************
Summary:
    This function returns the default settings for Qam In-Band Downstream module.

Description:
    This function is responsible for returns the default setting for
    BADS module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BADS_3466_Open()

****************************************************************************/
BERR_Code BADS_3466_GetDefaultSettings(
    BADS_Settings *pDefSettings,        /* [out] Returns default setting */
    BCHP_Handle hChip                   /* [in] Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BADS_3466_GetDefaultSettings);
    BSTD_UNUSED(hChip);

    *pDefSettings = defDevSettings;

    BDBG_LEAVE(BADS_3466_GetDefaultSettings);
    return( retCode );
}

BERR_Code BADS_3466_GetLockStatus(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_LockStatus *pLockStatus         /* [out] Returns lock status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3466_ChannelHandle hImplChnDev;
    uint32_t sb;
    uint8_t status;
    uint32_t addr = BCHP_LEAP_CTRL_SW_SPARE2;

    BDBG_ENTER(BADS_3466_GetLockStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_3466_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    if (hImplChnDev->chnNo > 7)
        addr = BCHP_LEAP_CTRL_GP4;

    CHK_RETCODE(retCode, BHAB_ReadRegister(hImplChnDev->hHab, addr, &sb));
    status = (sb >> (hImplChnDev->chnNo%8)*4) & 0xF;

    switch (status)
    {
        case 1:
            *pLockStatus = BADS_LockStatus_eLocked;
            break;

        case 0: /* work-around for FW bug */
        case 2:
            *pLockStatus = BADS_LockStatus_eUnlocked;
            break;

        case 3:
            *pLockStatus = BADS_LockStatus_eNoSignal;
            break;

        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

done:
    BDBG_LEAVE(BADS_3466_GetLockStatus);
    return retCode;
}
