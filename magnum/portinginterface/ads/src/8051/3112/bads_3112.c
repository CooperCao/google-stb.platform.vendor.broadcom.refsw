/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bhab.h"
#include "bads.h"
#include "bads_priv.h"
#include "bads_3112.h"
#include "../bads_31xx_priv.h"

BDBG_MODULE(bads_3112);

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
        BADS_31xx_Open,
        BADS_31xx_Close,
        BADS_31xx_Init,
        BADS_31xx_GetVersion,
        NULL, /* BADS_GetVersionInfo */        
        NULL,
        BADS_31xx_GetTotalChannels,
        BADS_31xx_OpenChannel,
        BADS_31xx_CloseChannel,
        BADS_31xx_GetDevice,
        BADS_31xx_GetChannelDefaultSettings,
        BADS_31xx_GetStatus,
        BADS_31xx_GetLockStatus,
        BADS_31xx_GetSoftDecision,
        BADS_31xx_InstallCallback,
        NULL, /* BADS_GetDefaultAcquireParams */
        NULL, /* BADS_SetAcquireParams */
        NULL, /* BADS_GetAcquireParams */        
        BADS_31xx_Acquire,
        BADS_31xx_EnablePowerSaver,
        BADS_31xx_DisablePowerSaver,
        BADS_31xx_ProcessNotification,
        BADS_31xx_SetDaisyChain,
        BADS_31xx_GetDaisyChain,
        BADS_31xx_ResetStatus,
        NULL,
        NULL,
        NULL,
        BADS_31xx_RequestAsyncStatus,
        BADS_31xx_GetAsyncStatus,
        NULL, /* GetScanStatus */        
        BADS_31xx_ReadSlave,
        BADS_31xx_WriteSlave,
        NULL, /*SetScanParam*/
        NULL, /* GetScanParam*/
        NULL, /* BADS_RequestSpectrumAnalyzerData */
        NULL /* BADS_GetSpectrumAnalyzerData  */
    },
    false,
    BADS_TransportData_eSerial,
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
    BADS_3112_Open()

****************************************************************************/
BERR_Code BADS_3112_GetDefaultSettings(
    BADS_Settings *pDefSettings,        /* [out] Returns default setting */
    BCHP_Handle hChip                   /* [in] Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BADS_3112_GetDefaultSettings);
    BSTD_UNUSED(hChip);

    *pDefSettings = defDevSettings;

    BDBG_LEAVE(BADS_3112_GetDefaultSettings);
    return( retCode );
}
