/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
 ***************************************************************************/
 
#ifndef BHDR_FE_H__
#define BHDR_FE_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "bhdr.h"


#include "bchp.h"
#include "bhdr_datatypes.h"


typedef struct _BHDR_FE_ChannelPowerSettings_
{
	bool bCecPowered ;
	bool bHdmiRxPowered ;	

} BHDR_FE_ChannelPowerSettings ;


typedef struct _BHDR_FE_Settings_
{
	uint8_t NoSettings ;
} BHDR_FE_Settings ;


typedef struct _BHDR_FE_ChannelSettings_
{
	uint8_t uiChannel ;
	uint8_t uiCecAddr ;
	bool bEnableEqualizer ;
	bool bExternalSwitch ;
	bool bHpdDisconnected ;
} BHDR_FE_ChannelSettings ;



/***************************************************************************
Summary:
	Prototype for external modules to install callback with HDMI Rx Front End to notify of 
	events/changes

Description:

Input:
	pvParam1 - User defined data structure casted to void.
	iParam2 - Additional user defined value.
	pvData - Data to pass from HDMI to application thru callback. 

Output:
	None

See Also:
	BHDR_FE_InstallCecCallback
	BHDR_FE_UnInstallCecCallback
	BHDR_FE_InstallHotPlugCallback
	BHDR_FE_UnInstallHotPlugCallback
**************************************************************************/
typedef void (*BHDR_FE_CallbackFunc)
	( void                            *pvParam1,
	  int                              iParam2,
	  void                            *pvData );



/***************************************************************************
Summary:
	Get the Default Settings for opening the Front End Channel

Input:
	hChip - handle for the Chip; different Chips may require different settings
	
    
Output:
	pDefaultSettings - default values in the struct BHDR_FE_Settings 

Returns:
	BERR_SUCCESS 

See Also:
	BHDR_FE_Open	

***************************************************************************/
BERR_Code BHDR_FE_GetDefaultSettings(
	BCHP_Handle hChip,
	BHDR_FE_Settings *pDefSettings) ;



/***************************************************************************
Summary:
	Open the Front End Channel Device

Input:
	hChip - handle for the Chip; different Chips may require different settings
	hRegister - Register Handle
	hInterrupt - Interrupt Handle 
	pDefaultSettings - default values in the struct BHDR_FE_Settings 
	
	    
Output:
	phFrontEnd - pointer to newly allocated Front End Handle

Returns:
	BERR_SUCCESS 

See Also:
	BHDR_FE_Open	
	BHDR_GetDefaultSettings

***************************************************************************/
BERR_Code BHDR_FE_Open(BHDR_FE_Handle *phFrontEnd,
 	BCHP_Handle hChip,                  /* Chip handle */
	BREG_Handle hRegister,              /* Register handle */
	BINT_Handle hInterrupt,             /* Interrupt handle */   
	const BHDR_FE_Settings  *pSettings /* [in] default HDMI settings */
)  ;
	
 

/***************************************************************************
Summary:
	Close the Front End Channel Device

Input:
	hFrontEnd - Front End Handle to close
	
	    
Output:

Returns:
	BERR_SUCCESS 

See Also:
	BHDR_FE_Open	

***************************************************************************/
BERR_Code BHDR_FE_Close(BHDR_FE_Handle hFrontEnd) ;




/***************************************************************************
Summary:
	Get the total number of channels available for the Front End Channel Device

Input:
	hFrontEnd - Front End Handle to use to determine total number of channels
	
	    
Output:
	uiTotalChannels - number of available channels 
	
Returns:
	BERR_SUCCESS 

See Also:

***************************************************************************/
BERR_Code BHDR_FE_GetTotalChannels(
    BHDR_FE_Handle hFrontEnd, uint8_t *uiTotalChannels) ;



/***************************************************************************
Summary:
	Get the Default Channel settings for a Front End Channel

Input:
	hFrontEnd - Front End Handle for the associtate channel
	eChannel  - channel for which to get the default settings
	
    
Output:
	pChSettings - default values in the struct BHDR_FE_ChannelSettings to 
	used for BHDR_FE_OpenChannel

Returns:
	BERR_SUCCESS 

See Also:
	BHDR_FE_Open	

***************************************************************************/
BERR_Code BHDR_FE_GetDefaultChannelSettings(
    BHDR_FE_Handle hFrontEnd, BHDR_FE_ChannelSettings *pChSettings) ;



/***************************************************************************
Summary:
	Open a Front End Channel Handle 

Input:
	hFrontEnd - Front End device handle
	pChSettings - default values for FE Channel Settings 
	
	    
Output:
	phChannel - pointer to newly allocated/populated Front End Channel Handle

Returns:
	BERR_SUCCESS 

See Also:
	BHDR_FE_Open	
	BHDR_GetChannelDefaultSettings

***************************************************************************/
BERR_Code BHDR_FE_OpenChannel(
	BHDR_FE_Handle hFrontEnd, BHDR_FE_ChannelHandle *phChannel, 
	const BHDR_FE_ChannelSettings * pChSettings) ;



/***************************************************************************
Summary:
	Close a Front End Channel Handle

Input:
	hChannel  - pointer to Front End Channel to be closed 
 		    
Output:

Returns:
	BERR_SUCCESS 

See Also:
	BHDR_FE_OpenChannel

***************************************************************************/
BERR_Code BHDR_FE_CloseChannel(BHDR_FE_ChannelHandle hChannel) ;



/******************************************************************************
Summary: Attach the HDMI Rx Device to a Front End Channel 

Input:
	hHDR - HDMI Rx handle that was previously opened by BHDR_Open.
	hFeChannel - HDMI Front End Handle

Output:
	None

Returns:
	BERR_SUCCESS - Successfully returned the HDMI handle
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDR_Open
	BHDR_FE_OpenChannel
	
*******************************************************************************/
BERR_Code BHDR_FE_AttachHdmiRxCore(
	BHDR_FE_ChannelHandle hFeChannel,
	BHDR_Handle hHDR) ;



/******************************************************************************
Summary: Detach a HDMI Rx Device from a Front End Channel 

Input:
	hHDR - HDMI Rx handle that was previously opened by BHDR_Open.	

Output:
	None

Returns:
	BERR_SUCCESS - Successfully returned the HDMI handle
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDR_Open

Note:
	This Detach function may not be needed since Attaching a different FE will inevitably result
	in a device being detached

*******************************************************************************/
BERR_Code BHDR_FE_DetachHdmiRxCore(
	BHDR_FE_ChannelHandle hFeChannel,
	BHDR_Handle hHDR) ;



/******************************************************************************
Summary: Get the ID  of the HDMI Rx Device atached to the to the Front End Channel 

Input:
	hFeChannel - HDMI Frontend handle that was previously opened by BHDR_FE_OpenChannel.

Output:
	None

Returns:
	BERR_SUCCESS - Successfully returned the HDMI handle
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDR_FE_OpenChannel

*******************************************************************************/
BERR_Code BHDR_FE_GetAttachedHdmiRxCore(
	BHDR_FE_ChannelHandle hFeChannel,
	uint8_t *uiHdrId)  ;



/***************************************************************************
Summary:
	Enable/Disable Equaliztation for the Front End Channel 

Input:
	hFeChannel  - pointer to Front End Channel to get the attached core for.
	bEnable - boolean to indicate disable/enable
 		    
Output:

Returns:
	BERR_SUCCESS 

See Also:
	BHDR_FE_OpenChannel

***************************************************************************/
BERR_Code BHDR_FE_EnableEqualization(BHDR_FE_ChannelHandle hFeChannel, bool bEnable) ;



/**************************************************************************
Summary:
	Register a callback function to be called when a CEC event occurs

Description:
	This function is used to enable a callback function that will
	be called any time a CEC interrupt occurs 

Input:
	hFeChannel - HDMI Front End Handle
	pfCallback_isr - pointer to callback function to be called at CEC event 
	pvParam1 - User defined data structure casted to void.
	iParam2 - Additional user defined value.
	
Returns:
	BERR_SUCCESS - Callback Installation Successful

See Also:
		BHDR_FE_UnInstallCecCallback

**************************************************************************/
BERR_Code BHDR_FE_InstallCecCallback( 
	BHDR_FE_ChannelHandle hFeChannel,	/* [in] HDMI Front End Channel Handle */
	const BHDR_FE_CallbackFunc pfCallback_isr, /* [in] cb for notification */
	void *pvParm1,  /* [in] the first argument (void *) passed to the callback function */ 
	int iParm2      /* [in] the second argument(int) passed to the callback function */ 
) ;


/**************************************************************************
Summary:
	Remove a previously registered callback function for CEC events

Description:
	This function is used to remove a callback function from being called
	any time a CEC event occurs

Input:
	hFeChannel - HDMI Front End Handle
	pfCallback_isr - pointer to callback function to be removed
	
Returns:
	BERR_SUCCESS - Callback Removal Successful

See Also:
		BHDR_FE_InstallCecCallback

**************************************************************************/
BERR_Code BHDR_FE_UnInstallCecCallback( 
	BHDR_FE_ChannelHandle hFeChannel,	/* [in] HDMI Front End Channel Handle */
	const BHDR_FE_CallbackFunc pfCallback_isr /* [in] cb for CEC notification  */
) ;



/**************************************************************************
Summary:
	Register a callback function to be called when a Hot Plug event occurs

Description:
	This function is used to enable a callback function that will
	be called any time a Hot Plug interrupt occurs 

Input:
	hFeChannel - HDMI Front End Handle
	pfCallback_isr - pointer to callback function to be called at Hot Plug event 
	pvParam1 - User defined data structure casted to void.
	iParam2 - Additional user defined value.
	
Returns:
	BERR_SUCCESS - Callback Installation Successful

See Also:
		BHDR_FE_UnInstallHotPlugCallback

**************************************************************************/
BERR_Code BHDR_FE_InstallHotPlugCallback( 
	BHDR_FE_ChannelHandle hFeChannel,	/* [in] HDMI Front End Channel Handle */
	const BHDR_FE_CallbackFunc pfCallback_isr, /* [in] cb for notification */
	void *pvParm1,  /* [in] the first argument (void *) passed to the callback function */ 
	int iParm2      /* [in] the second argument(int) passed to the callback function */ 
) ;


/**************************************************************************
Summary:
	Remove a previously registered callback function for Hot Plug events

Description:
	This function is used to remove a callback function from being called
	any time a Hot Plug event occurs

Input:
	hFeChannel - HDMI Front End Handle
	pfCallback_isr - pointer to callback function to be removed
	
Returns:
	BERR_SUCCESS - Callback Removal Successful

See Also:
		BHDR_FE_InstallHotPlugCallback

**************************************************************************/
BERR_Code BHDR_FE_UnInstallHotPlugCallback( 
	BHDR_FE_ChannelHandle hFeChannel,	/* [in] HDMI Front End Channel Handle */
	const BHDR_FE_CallbackFunc pfCallback_isr /* [in] cb for Hot Plug notification  */
) ;


/***************************************************************************
Summary:
	Set the Power Management Settings for a HDMI Rx Front End Channel

Input:
	hFeChannel - HDMI Front End Channel Handle that was previously opened by BHDR_FE_OpenChannel.
	stPowerSettings - power settings for components of the HDMI Rx 

Output:
	None

Returns:
	BERR_SUCCESS - Successfully set the Power Management Settings
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDR_FE_GetPowerSettings

****************************************************************************/
BERR_Code BHDR_FE_SetPowerState(
	BHDR_FE_ChannelHandle hFeChannel, BHDR_FE_ChannelPowerSettings * stPowerSettings) ;


/***************************************************************************
Summary:
	Get  the current Power Management Settings for a HDMI Rx Front End Channel

Input:
	hFeChannel - HDMI Front End Channel Handle that was previously opened by BHDR_FE_OpenChannel.
	stPowerSettings - pointer to power settings structure

Output:
	None

Returns:
	BERR_SUCCESS - Successfully returned the Power Management Settings 
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDR_FE_SetPowerState

****************************************************************************/
BERR_Code BHDR_FE_GetPowerState(
	BHDR_FE_ChannelHandle hFeChannel, BHDR_FE_ChannelPowerSettings * stPowerSettings) ;

#if BHDR_CONFIG_DEBUG_EYE_DIAGRAM
/***************************************************************************
Summary:
	Debug function to get HDMI eye diagram data 

Input:
	hFeChannel - HDMI Front End Channel Handle that was previously opened by BHDR_FE_OpenChannel.

Output:
	pointer to AD data; memory must be allocated by caller 

Returns:
	BERR_SUCCESS - Successfully returned the eye data

See Also:

****************************************************************************/
BERR_Code BHDR_FE_DEBUG_GetEyeDiagramData(BHDR_Handle hHDR, uint8_t *padc_data) ;
#endif

#ifdef __cplusplus
}
#endif
 
#endif

