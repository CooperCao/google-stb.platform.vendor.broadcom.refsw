/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bhab.h"
#include "btc2.h"
#include "btc2_priv.h"
#include "btc2_3461.h"
#include "btc2_3461_priv.h"

BDBG_MODULE(btc2_3461);

/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BTC2_Settings defDevSettings =
{
    BHAB_DevId_eTC20,       
    NULL,                   /* BHAB handle, must be provided by application*/
    {
        BTC2_3461_Open,
        BTC2_3461_Close,
        BTC2_3461_Init,
        BTC2_3461_GetVersion,
        BTC2_3461_GetVersionInfo,
        BTC2_3461_GetTotalChannels,
        BTC2_3461_OpenChannel,
        BTC2_3461_CloseChannel,
        BTC2_3461_GetDevice,
        BTC2_3461_GetChannelDefaultSettings,
        BTC2_3461_GetStatus,
        BTC2_3461_GetLockStatus,        
        BTC2_3461_GetSoftDecision,
        BTC2_3461_InstallCallback,
        BTC2_3461_GetDefaultAcquireParams,
        BTC2_3461_SetAcquireParams,
        BTC2_3461_GetAcquireParams,       
        BTC2_3461_Acquire,
        BTC2_3461_EnablePowerSaver,
        BTC2_3461_DisablePowerSaver,
        BTC2_3461_ProcessNotification,
        BTC2_3461_ResetStatus,
        BTC2_3461_RequestAsyncStatus,
        BTC2_3461_GetAsyncStatus,
        BTC2_3461_ReadSlave,
        BTC2_3461_WriteSlave,
        BTC2_3461_RequestSelectiveAsyncStatus,
        BTC2_3461_GetSelectiveAsyncStatusReadyType,
        BTC2_3461_GetSelectiveAsyncStatus
    },
    false,
    NULL,
    NULL
};

static const BTC2_InbandParam defInbandParams = 
{
    BTC2_ModulationType_eDvbt2,
    BTC2_AcquireMode_eAuto,
    BTC2_Bandwidth_e8MHz,
    true,
    0,
    false,
    false,
    BTC2_SpectrumMode_eManual,
    BTC2_InvertSpectrum_eNormal
};

/***************************************************************************
Summary:
    This function returns the default settings for TC2 module.

Description:
    This function is responsible for returns the default setting for 
    BTC2 module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BTC2_3461_Open()

****************************************************************************/
BERR_Code BTC2_3461_GetDefaultSettings(
    BTC2_Settings *pDefSettings,        /* [out] Returns default setting */
    BCHP_Handle hChip                   /* [in] Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTC2_3461_GetDefaultSettings);
    BSTD_UNUSED(hChip);

    *pDefSettings = defDevSettings;

    BDBG_LEAVE(BTC2_3461_GetDefaultSettings);
    return( retCode );
}

/***************************************************************************
Summary:
    This function returns the default settings for TC2 module.

Description:
    This function is responsible for returns the default setting for 
    BTC2 module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BTC2_3461_Open()

****************************************************************************/
BERR_Code BTC2_3461_GetDefaultAcquireParams(
    BTC2_InbandParam *ibParams          /* [out] default Inband Parameters */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTC2_3461_GetDefaultAcquireParams);

    *ibParams = defInbandParams;

    BDBG_LEAVE(BTC2_3461_GetDefaultAcquireParams);
    return( retCode );
}