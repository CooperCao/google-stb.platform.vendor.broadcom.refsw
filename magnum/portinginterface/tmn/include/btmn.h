/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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


/*= Module Overview *********************************************************
<verbatim>

Overview
The TMN PI module controls the temperature monitor block in the BCM7420.

Design
The design for BTMN PI API is broken into three parts.

o Part 1 (open/close/configuration):

  These APIs are used for opening and closing BTMN device, configure measurement interval.
    
o Part 2 (enable TMN):

  These APIs enable the TMN to start temperature monitoring.

o Part 3 (handle interrupt, get the data):

  These APIs handle the TMN interrupt and read the voltage values.

Usage
The usage of BTMN involves the following:

  * Configure/Open of BTMN

    * Configure BTMN device for the target system
    * Open BTMN device

  * Enable TMN

  * Wait for interrupt and get the data

Sample Code

BTMN_Handle				hTmn;
uint32_t				tmontest_top, tmontest_bottom, tmontest_mid1, tmontest_mid0;

void tmon_isr (void)
{
	uint32_t			lval;

	printf("TMON Diags installed ISR\n");

	//Get the voltage values
	BTMN_GetVoltageValues (hTmn, &tmontest_top, &tmontest_bottom, &tmontest_mid1, &tmontest_mid0);
}

void main( void )
{
    BTMN_Settings			defSettings;
    BREG_Handle				hReg;
    BCHP_Handle				hChip;
    BINT_Handle				hInt;
	BKNI_EventHandle		dataReadyEvent;

    // Do other initialization, i.e. for BREG, BCHP, etc.

    // Now, ready to setup the BTMN
    BTMN_GetDefaultSettings( &defSettings, hChip );
   	BTMN_Open (&hTmn, hChip, hReg, hInt, &defSettings);
	BTMN_GetDataReadyEventHandle (hTmn, &dataReadyEvent);

    // Install callback and go
	BTMN_InstallDataReadyInterruptCallback (hTmn, (BTMN_CallbackFunc)tmon_isr, (void *)NULL, 0);
	BTMN_SetMeasurementInterval (hTmn, 10);	// set measurement interval to 1 second
	BTMN_Enable (hTmn, true);

	while (1)
	{
		// Wait for data ready interrupt
		BKNI_WaitForEvent (dataReadyEvent, BKNI_INFINITE);
		
		//Now process the voltage values
		tmonapp_ProcessTrel0Data (	tmontest_top, 
									tmontest_bottom, 
									tmontest_mid1, 
									tmontest_mid0 );
	}

}

B</verbatim>
***************************************************************************/


#ifndef BTMN_H__
#define BTMN_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"
#include "bchp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* These parts support on-chip temperature that does NOT require external thermistor to work */
#ifdef BCHP_TMON_REG_START
#define BTMN_SUPPORTS_ONCHIPTEMP 1
#endif

/***************************************************************************
Summary:
	Error Codes specific to BTMN

Description:

See Also:

****************************************************************************/
#define BTMN_ERR_BAD_PARAMS				BERR_MAKE_CODE(BERR_TMN_ID, 0)

#define MAX_MEASURE_INTERVAL		100			/* maximum of 10s interval */

/***************************************************************************
Summary:
	The handle for tmn module.

Description:

See Also:
	BTMN_Open()

****************************************************************************/
typedef struct BTMN_P_Handle				*BTMN_Handle;

/***************************************************************************
Summary:
	Required default settings structure for TMN module.

Description:
	The default setting structure defines the default configure of
	TMN when the device is opened.  

See Also:
	BTMN_Open()

****************************************************************************/
typedef struct BTMN_Settings
{
	uint8_t			measureInterval;
} BTMN_Settings;


/***************************************************************************
Summary:
	Callback function for BTMN

Description:
	The callback function is registered by the caller.  It gets called
	when TMN interrupt happens.

See Also:
	BTMN_InstallInterruptCallback()

****************************************************************************/
typedef void (*BTMN_CallbackFunc)( void *pParm1, int parm2, void *pTMN_data);

/***************************************************************************
Summary:
	Callback info structure

Description:
	This is the callback information structure.  Caller registers this
	information with module to be called at interrupt time.

See Also:
	BTMN_InstallInterruptCallback()

****************************************************************************/
typedef struct BTMN_CallbackInfo
{
	BTMN_CallbackFunc cbFunc ;
	void              *pParm1 ;
	int               parm2 ;
} BTMN_CallbackInfo;


/***************************************************************************
Summary:
	This function opens TMN module.

Description:
	This function is responsible for opening BTMN module. When BTMN is
	opened, it will create a module handle and configure the module based
	on the default settings. Once the device is opened, it must be closed
	before it can be opened again.

Returns:
	TODO:

See Also:
	BTMN_Close()
	BTMN_GetDefaultSettings()

****************************************************************************/
BERR_Code BTMN_Open(
	BTMN_Handle *pTMN,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_Handle hRegister,				/* [in] Register handle */
	BINT_Handle hInterrupt,				/* [in] Interrupt handle */
	const BTMN_Settings *pDefSettings	/* [in] Default settings */
	);

/***************************************************************************
Summary:
	This function closes TMN module.

Description:
	This function is responsible for closing BTMN module. Closing BTMN 
	will free main BTMN handle. 

Returns:
	TODO:

See Also:
	BTMN_Open()

****************************************************************************/
BERR_Code BTMN_Close(
	BTMN_Handle hDev					/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function returns the default settings for TMN module.

Description:
	This function is responsible for returns the default setting for 
	BTMN module. The returning default setting should be when
	opening the device.

Returns:
	TODO:

See Also:
	BTMN_Open()

****************************************************************************/
BERR_Code BTMN_GetDefaultSettings(
	BTMN_Settings *pDefSettings,		/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	);

/***************************************************************************
Summary:
	This function resets the TMON core

Description:
	This function is used to reset the TMON core.

Returns:
	TODO:

See Also:
	BTMN_Open()

****************************************************************************/
BERR_Code BTMN_Reset(
	BTMN_Handle hDev					/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function gets the data ready event handle for BTMN module.

Description:
	This function is responsible for getting the data ready event handle. The
	application code should use this function get BTMN's event handle, 
	which the application should use to pend on.  The TMN ISR will 
	set the event.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BTMN_GetDataReadyEventHandle(
	BTMN_Handle 		hDev,			/* [in] Device handle */
	BKNI_EventHandle 	*phEvent		/* [out] Returns event handle */
	);

/***************************************************************************
Summary:
	This function gets the over temperature event handle for BTMN module.

Description:
	This function is responsible for getting the over temp event handle. The
	application code should use this function get BTMN's event handle, 
	which the application should use to pend on.  The TMN ISR will 
	set the event.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BTMN_GetOverTempEventHandle(
	BTMN_Handle 		hDev,			/* [in] Device handle */
	BKNI_EventHandle 	*phEvent		/* [out] Returns event handle */
	);


/***************************************************************************
Summary:
	This function enables/disables TMON hardware

Description:
	This function enables or disables the TMON hardware depending on the enable flag.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BTMN_Enable (
	BTMN_Handle 		hDev, 			/* [in] Device handle */
	bool				enable			/* enable flag */
);

/***************************************************************************
Summary:
	This function gets the state of TMON enable.

Description:
	This function returns whether TMON is enabled or not.
	
Returns:
	TODO:

See Also:

****************************************************************************/
bool BTMN_GetEnable (
	BTMN_Handle 		hDev 			/* [in] Device handle */
);


/***************************************************************************
Summary:
	This function asserts the Over Temperature interrupt

Description:
	This function is used to signal the condition where the average temperature
	is greater than the maximum average temperature.  It will trigger an
	interrupt to the CPU.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BTMN_AssertOverTempInterrupt (
	BTMN_Handle 		hDev 			/* [in] Device handle */
);

/***************************************************************************
Summary:
	This function sets measurement interval

Description:
	Call this function to change measurement interval from 1 to 10s.  Interval
	is in 100ms increments.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BTMN_SetMeasurementInterval (
	BTMN_Handle 		hDev, 			/* [in] Device handle */
	uint8_t				interval		/* [in] new measurement interval */
);


/***************************************************************************
Summary:
	This function returns measurement interval

Description:
	This function gets the current measurement interval from the hardware.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BTMN_GetMeasurementInterval (
	BTMN_Handle 		hDev, 			/* [in] Device handle */
	uint8_t				*interval		/* [out] pointer to measurement interval */
);

/***************************************************************************
Summary:
	This function installs a callback for data ready interrupt

Description:
	This function allows the user to install a callback for the data ready
	interrupt.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BTMN_InstallDataReadyInterruptCallback (
	BTMN_Handle 		hDev,			/* [in] Device handle */
	BTMN_CallbackFunc	callback,		/* [in] callback function */
	void				*pParm1, 		/* [in] application specified parameter */
	int					parm2 			/* [in] application specified parameter */
);

/***************************************************************************
Summary:
	This function uninstalls the data ready callback

Description:
	This function uninstalls the data ready callback that was installed when
	the user calls BTMN_InstallDataReadyInterruptCallback.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BTMN_UnInstallDataReadyInterruptCallback (
	BTMN_Handle 		hDev			/* [in] Device handle */
);

/***************************************************************************
Summary:
	This function installs a callback for over temperature interrupt

Description:
	This function allows the user to install a callback for the over temperature
	interrupt.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BTMN_InstallOverTempInterruptCallback (
	BTMN_Handle 		hDev,			/* [in] Device handle */
	BTMN_CallbackFunc	callback,		/* [in] callback function */
	void				*pParm1, 		/* [in] application specified parameter */
	int					parm2 			/* [in] application specified parameter */
);

/***************************************************************************
Summary:
	This function uninstalls the over temperature callback

Description:
	This function uninstalls the over temperature callback that was installed when
	the user calls BTMN_InstallOverTempInterruptCallback.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BTMN_UnInstallOverTempInterruptCallback (
	BTMN_Handle 		hDev			/* [in] Device handle */
);

/***************************************************************************
Summary:
	This function gets the voltage values read during the data ready interrupt

Description:
	This function outputs the 4 voltage values read during the data ready 
	interrupt.  These values are used to calculate the average relative
	temperature.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BTMN_GetVoltageValues (
	BTMN_Handle 		hDev,			/* [in] Device handle */
	uint32_t			*top,			/* [out] pointer to top */
	uint32_t			*bottom,		/* [out] pointer to bottom */
	uint32_t			*mid1,			/* [out] pointer to mid1 */
	uint32_t			*mid0			/* [out]pointer to mid0 */
);

/***************************************************************************
Summary:
	This function gets the voltage values read during the data ready interrupt

Description:
	This function outputs the 4 voltage values read during the data ready 
	interrupt.  These values are used to calculate the average relative
	temperature.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BTMN_GetVoltageValues_isr (
	BTMN_Handle 		hDev,			/* [in] Device handle */
	uint32_t			*top,			/* [out] pointer to top */
	uint32_t			*bottom,		/* [out] pointer to bottom */
	uint32_t			*mid1,			/* [out] pointer to mid1 */
	uint32_t			*mid0			/* [out]pointer to mid0 */
);

#if BTMN_SUPPORTS_ONCHIPTEMP
/***************************************************************************
Summary:
	This function gets the on-chip temperature

Description:
	This function calculates the on-chip temperature.  This is only
	available on chips with this feature (7422, ... ).
	The temperature is in degrees C truncated to the nearest integer value.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BTMN_GetOnChipTemp (
	BTMN_Handle 		hDev,			/* [in] Device handle */
	signed			*tempC			/* [out] temperature in degrees C */
);
#endif


#ifdef __cplusplus
}
#endif
 
#endif



