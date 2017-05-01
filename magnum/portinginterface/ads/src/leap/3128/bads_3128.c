/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bads_3128.h"
#include "../bads_leap_priv.h"

/* TODO: Add support for mutiple chip revisions */
#if (BADS_312X_VER == BCHP_VER_C0)
#include "../../../../3128/rdb/c0/bchp_leap_ctrl.h"
#endif

BDBG_MODULE(bads_3128);

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
        BADS_Leap_Open,
        BADS_Leap_Close,
        BADS_Leap_Init,
        BADS_Leap_GetVersion,
        BADS_Leap_GetVersionInfo,
        NULL,
        BADS_Leap_GetTotalChannels,
        BADS_Leap_OpenChannel,
        BADS_Leap_CloseChannel,
        BADS_Leap_GetDevice,
        BADS_Leap_GetChannelDefaultSettings,
        BADS_Leap_GetStatus,
        BADS_3128_GetLockStatus,
        BADS_Leap_GetSoftDecision,
        BADS_Leap_InstallCallback,
        BADS_Leap_GetDefaultAcquireParams,
        BADS_Leap_SetAcquireParams,
        BADS_Leap_GetAcquireParams,
        BADS_Leap_Acquire,
        BADS_Leap_EnablePowerSaver,
        BADS_Leap_DisablePowerSaver,
        BADS_Leap_ProcessNotification,
        BADS_Leap_SetDaisyChain,
        BADS_Leap_GetDaisyChain,
        BADS_Leap_ResetStatus,
        NULL,
        NULL,
        NULL,
        BADS_Leap_RequestAsyncStatus,
        BADS_Leap_GetAsyncStatus,
        BADS_Leap_GetScanStatus,
        BADS_Leap_ReadSlave,
        BADS_Leap_WriteSlave,
        BADS_Leap_SetScanParam,
        BADS_Leap_GetScanParam,
        NULL, /* BADS_RequestSpectrumAnalyzerData */
        NULL /* BADS_GetSpectrumAnalyzerData  */
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
    BADS_Leap_Open()

****************************************************************************/
BERR_Code BADS_3128_GetDefaultSettings(
    BADS_Settings *pDefSettings,        /* [out] Returns default setting */
    BCHP_Handle hChip                   /* [in] Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BADS_3128_GetDefaultSettings);
    BSTD_UNUSED(hChip);

    *pDefSettings = defDevSettings;

    BDBG_LEAVE(BADS_3128_GetDefaultSettings);
    return( retCode );
}

BERR_Code BADS_3128_GetLockStatus(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_LockStatus *pLockStatus         /* [out] Returns lock status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint32_t sb;
    uint8_t status;

    BDBG_ENTER(BADS_3128_GetLockStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    CHK_RETCODE(retCode, BHAB_ReadRegister(hImplChnDev->hHab, BCHP_LEAP_CTRL_SW_SPARE2, &sb));
    status = (sb >> hImplChnDev->chnNo*4) & 0xF;

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
    BDBG_LEAVE(BADS_3128_GetLockStatus);
    return retCode;
}
