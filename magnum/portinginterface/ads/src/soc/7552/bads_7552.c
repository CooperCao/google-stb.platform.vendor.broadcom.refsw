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
#include "bads_7552.h"
#include "bads_3x7x_priv.h"


BDBG_MODULE(bads_7552);

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
    BADS_7552_Open()

****************************************************************************/
BERR_Code BADS_7552_GetDefaultSettings(
    BADS_Settings *pDefSettings,        /* [out] Returns default setting */
    BCHP_Handle hChip                   /* [in] Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BADS_7552_GetDefaultSettings);
    BSTD_UNUSED(hChip);

	BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));

    pDefSettings->funcPtr.Open						= BADS_3x7x_Open;
	pDefSettings->funcPtr.Close						= BADS_3x7x_Close;
	pDefSettings->funcPtr.Init						= BADS_3x7x_Init;
	pDefSettings->funcPtr.GetVersion				= BADS_3x7x_GetVersion;
	pDefSettings->funcPtr.GetTotalChannels			= BADS_3x7x_GetTotalChannels;
	pDefSettings->funcPtr.OpenChannel				= BADS_3x7x_OpenChannel;
	pDefSettings->funcPtr.CloseChannel				= BADS_3x7x_CloseChannel;
	pDefSettings->funcPtr.GetDevice					= BADS_3x7x_GetDevice;
	pDefSettings->funcPtr.GetChannelDefaultSettings	= BADS_3x7x_GetChannelDefaultSettings;
	pDefSettings->funcPtr.GetStatus					= BADS_3x7x_GetStatus;
	pDefSettings->funcPtr.GetLockStatus				= BADS_3x7x_GetLockStatus;
	pDefSettings->funcPtr.GetSoftDecision			= BADS_3x7x_GetSoftDecision;
	pDefSettings->funcPtr.InstallCallback			= BADS_3x7x_InstallCallback;
	pDefSettings->funcPtr.Acquire					= BADS_3x7x_Acquire;
	pDefSettings->funcPtr.EnablePowerSaver			= BADS_3x7x_EnablePowerSaver;
	pDefSettings->funcPtr.DisablePowerSaver			= BADS_3x7x_DisablePowerSaver;
	pDefSettings->funcPtr.ProcessNotification		= BADS_3x7x_ProcessNotification;
	pDefSettings->funcPtr.SetDaisyChain				= BADS_3x7x_SetDaisyChain;
	pDefSettings->funcPtr.GetDaisyChain				= BADS_3x7x_GetDaisyChain;
	pDefSettings->funcPtr.ResetStatus				= BADS_3x7x_ResetStatus;
	pDefSettings->funcPtr.GetInterruptEventHandle	= BADS_3x7x_GetInterruptEventHandle;
	pDefSettings->funcPtr.ProcessInterruptEvent		= BADS_3x7x_ProcessInterruptEvent;
	pDefSettings->funcPtr.Untune					= BADS_3x7x_Untune;
	pDefSettings->funcPtr.GetScanStatus				= BADS_3x7x_GetScanStatus,	
    pDefSettings->funcPtr.SetScanParam				= BADS_3x7x_SetScanParam,
    pDefSettings->funcPtr.GetScanParam				= BADS_3x7x_GetScanParam,
	pDefSettings->funcPtr.SetAcquireParams			= BADS_3x7x_SetAcquireParams,
	pDefSettings->isOpenDrain						= false;
    pDefSettings->transportConfig					= BADS_TransportData_eGpioSerial; 


    BDBG_LEAVE(BADS_7552_GetDefaultSettings);
    return( retCode );
}

