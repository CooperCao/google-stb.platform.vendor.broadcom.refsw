/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bhab.h"
#include "bods.h"
#include "bods_priv.h"
#include "bods_7563.h"
#include "../bods_leap_priv.h"

BDBG_MODULE(bods_7563);

/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BODS_Settings defDevSettings =
{  
    NULL, /* BHAB handle, must be provided by application*/
    {
        BODS_Leap_Open,
        BODS_Leap_Close,
        BODS_Leap_Init,
        BODS_Leap_GetVersion,
        BODS_Leap_GetVersionInfo,        
        BODS_Leap_GetTotalChannels,
        BODS_Leap_GetChannelDefaultSettings,
        BODS_Leap_OpenChannel,
        BODS_Leap_CloseChannel,
        BODS_Leap_GetDevice,
        BODS_Leap_GetCapabilities,
        BODS_Leap_GetLockStatus,
        BODS_Leap_GetSoftDecision,
        BODS_Leap_InstallCallback,
        BODS_Leap_GetDefaultAcquireParams,
        BODS_Leap_SetAcquireParams,
        BODS_Leap_GetAcquireParams,
        BODS_Leap_Acquire,
        BODS_Leap_EnablePowerSaver,
        BODS_Leap_DisablePowerSaver,
        BODS_Leap_ResetStatus,
        BODS_Leap_RequestSelectiveAsyncStatus,
        BODS_Leap_GetSelectiveAsyncStatusReadyType,
        BODS_Leap_GetSelectiveAsyncStatus
    }
};

/***************************************************************************
Summary:
    This function returns the default settings for ODS module.

Description:
    This function is responsible for returns the default setting for 
    BODS module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BODS_Leap_Open()

****************************************************************************/
BERR_Code BODS_7563_GetDefaultSettings(
    BODS_Settings *pDefSettings,        /* [out] Returns default setting */
    BCHP_Handle hChip                   /* [in] Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BODS_7563_GetDefaultSettings);
    BSTD_UNUSED(hChip);

    *pDefSettings = defDevSettings;

    BDBG_LEAVE(BODS_7563_GetDefaultSettings);
    return( retCode );
}
