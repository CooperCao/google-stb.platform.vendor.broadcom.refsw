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
#include "bads_7550.h"
#include "bads_7550_api.h"

BDBG_MODULE(bads_7550);

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
    BADS_7550_Open()

****************************************************************************/
BERR_Code BADS_7550_GetDefaultSettings(
    BADS_Settings *pDefSettings,        /* [out] Returns default setting */
    BCHP_Handle hChip                   /* [in] Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BADS_7550_GetDefaultSettings);
    BSTD_UNUSED(hChip);
    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));

    pDefSettings->funcPtr.Open						= BADS_Soc_Open;
	pDefSettings->funcPtr.Close						= BADS_Soc_Close;
	pDefSettings->funcPtr.Init						= BADS_Soc_Init;
	pDefSettings->funcPtr.GetVersion				= BADS_Soc_GetVersion;
	pDefSettings->funcPtr.GetTotalChannels			= BADS_Soc_GetTotalChannels;
	pDefSettings->funcPtr.OpenChannel				= BADS_Soc_OpenChannel;
	pDefSettings->funcPtr.CloseChannel				= BADS_Soc_CloseChannel;
	pDefSettings->funcPtr.GetDevice					= BADS_Soc_GetDevice;
	pDefSettings->funcPtr.GetChannelDefaultSettings	= BADS_Soc_GetChannelDefaultSettings;
	pDefSettings->funcPtr.GetStatus					= BADS_Soc_GetStatus;
	pDefSettings->funcPtr.GetLockStatus				= BADS_Soc_GetLockStatus;
	pDefSettings->funcPtr.GetSoftDecision			= BADS_Soc_GetSoftDecision;
	pDefSettings->funcPtr.InstallCallback			= BADS_Soc_InstallCallback;
	pDefSettings->funcPtr.Acquire					= BADS_Soc_Acquire;
	pDefSettings->funcPtr.EnablePowerSaver			= BADS_Soc_EnablePowerSaver;
	pDefSettings->funcPtr.ProcessNotification		= BADS_Soc_ProcessNotification;
	pDefSettings->funcPtr.SetDaisyChain				= BADS_Soc_SetDaisyChain;
	pDefSettings->funcPtr.GetDaisyChain				= BADS_Soc_GetDaisyChain;
	pDefSettings->funcPtr.ResetStatus				= BADS_Soc_ResetStatus;
	pDefSettings->funcPtr.GetInterruptEventHandle	= BADS_Soc_GetInterruptEventHandle;
	pDefSettings->funcPtr.ProcessInterruptEvent		= BADS_Soc_ProcessInterruptEvent;
	pDefSettings->funcPtr.Untune					= BADS_Soc_Untune;
	pDefSettings->isOpenDrain						= false;
    pDefSettings->transportConfig					= BADS_TransportData_eGpioSerial; 
 
    BDBG_LEAVE(BADS_7550_GetDefaultSettings);
    return( retCode );
}


/***************************************************************************
Summary:
    This function is called from the tuner PI.

Description:
    This funstion will allow the tuner to manage the power

Returns:
    TODO:

See Also:
    BADS_7550_ProcessTnrInterrupt_isr()

****************************************************************************/
BERR_Code BADS_7550_ProcessTnrInterrupt_isr(
	BADS_ChannelHandle hChannel,
	const BADS_7550_TnrInterruptData *pInterruptData
    )
{
	BERR_Code retCode = BERR_SUCCESS;

	if (pInterruptData->bResetDpm) 
	{
		BADS_Soc_P_ResetDPM(hChannel);
	}
	else
	{
		BADS_Soc_P_SetDPM(hChannel, pInterruptData->iOutdivMs);
	}
    return( retCode );
}


