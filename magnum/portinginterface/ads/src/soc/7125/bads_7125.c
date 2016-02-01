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
#include "bads.h"
#include "bads_priv.h"
#include "bads_7125.h"
#include "bads_7125_priv.h"

BDBG_MODULE(bads_7125);

/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BADS_Settings defDevSettings =
{
    0,
    NULL,                   /* BHAB handle, must be provided by application*/
    {
        BADS_7125_Open,
        BADS_7125_Close,
        BADS_7125_Init,
        BADS_7125_GetVersion,
        NULL, /* BADS_GetVersionInfo */
        NULL, /* GetBondingCapability */
        BADS_7125_GetTotalChannels,
        BADS_7125_OpenChannel,
        BADS_7125_CloseChannel,
        BADS_7125_GetDevice,
        BADS_7125_GetChannelDefaultSettings,
        BADS_7125_GetStatus,
        BADS_7125_GetLockStatus,
        BADS_7125_GetSoftDecision,
        BADS_7125_InstallCallback,
        NULL, /* BADS_GetDefaultAcquireParams */
        NULL, /* BADS_SetAcquireParams */
        NULL, /* BADS_GetAcquireParams */
        BADS_7125_Acquire,
        BADS_7125_EnablePowerSaver,
        BADS_7125_DisablePowerSaver,
        BADS_7125_ProcessNotification,
        BADS_7125_SetDaisyChain,
        BADS_7125_GetDaisyChain,
        BADS_7125_ResetStatus,
        BADS_7125_GetInterruptEventHandle,
        BADS_7125_ProcessInterruptEvent,
        NULL, /* Untune */
        NULL, /* RequestAsyncStatus */
        NULL,  /* GetAsyncStatus */
        NULL, /* GetScanStatus */
        NULL, /* ReadSlave */
        NULL,  /* WriteSlave */
        NULL, /*SetScanParam*/
        NULL, /* GetScanParam*/
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
    BADS_7125_Open()

****************************************************************************/
BERR_Code BADS_7125_GetDefaultSettings(
    BADS_Settings *pDefSettings,        /* [out] Returns default setting */
    BCHP_Handle hChip                   /* [in] Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BADS_7125_GetDefaultSettings);
    BSTD_UNUSED(hChip);

    *pDefSettings = defDevSettings;

    BDBG_LEAVE(BADS_7125_GetDefaultSettings);
    return( retCode );
}

/***************************************************************************
Summary:
    This function is called from the tuner PI.

Description:
    This function will allow the tuner to manage the power

Returns:
    TODO:

See Also:
    BADS_7125_ProcessTnrInterrupt_isr()

****************************************************************************/
BERR_Code BADS_7125_ProcessTnrInterrupt_isr(
    BADS_ChannelHandle hChannel,
    const BADS_7125_TnrInterruptData *pInterruptData
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Handle hDev = (BADS_Handle)(hChannel->hAds);
    BADS_7125_Handle hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BADS_7125_ChannelHandle h = (BADS_7125_ChannelHandle)(hChannel->pImpl);
    unsigned int chn = h->chnNo;

    switch (pInterruptData->action) {
        case BADS_7125_ResetDpm:
            BADS_7125_P_ResetDPM_isr(hChannel);
            break;
        case BADS_7125_SetDpm:
            BADS_7125_P_SetDPM_isr(hChannel, pInterruptData->iOutdivMs);
            break;
        case BADS_7125_RequestLnaGain:
            if ( hImplDev->pCallback[chn][BADS_Callback_eUpdateGain] != NULL )
            {
                    BDBG_MSG(("%s(): Calling BADS_Callback_eUpdateGain callback"));
                    (hImplDev->pCallback[chn][BADS_Callback_eUpdateGain])(hImplDev->pCallbackParam[chn][BADS_Callback_eUpdateGain] );
            }
            break;
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }
    return( retCode );
}
