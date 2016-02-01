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
 * This module is the driver for LNA.
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
/*= Module Overview *********************************************************
<verbatim>

Overview
The Low Noise Amplifier (LNA) is responsible for providing input signal
conditioning for cable television signals in the 50MHz to 860MHz band.
Bcm3405 provides RF AGC functions, which spans 30 dB, can be used to
accommodate input signal strength variations from -15 dBmV to +15 dBmV.
Bcm3405 is optimized to interface to Bcm3418 tuner.  The Bcm3405 can drive up
to three cable tuners, as well as providing bypass and out of band outputs.
The Bcm3431 can drive up to three cable tuners.


Design
  The design for BLNA PI API is broken into two parts.

o Part 1 (open/close/configuration):

    These APIs are used for opening and closing BLNA device.
    When a device is opened, the device can be configured.

o Part 2 (get/set):

    These APIs are used for getting and setting runtime attributes of BLNA.



Usage
The usage of BLNA involves the following:

   * Configure/Open of BLNA

      * Configure BLNA device for the target system
      * Open BLNA device

   * Program device

      * Program LNA


Interrupt Requirements
None


Sample Code:
//
// NOTE: The following sample code does not do any error checking.
//
// In this example, the target H/W board is configured for an
// Out-of-Band Downstream DVS 178 application, with a Crystal Frequency of 24.00 MHz
// and IF Frequency of 43.75 MHz.
//
static BLNA_Handle lnaDev;
static BCHP_Handle hChip3250;
static BREG_I2C_Handle hBcm7038I2C;


main( void )
{
	BLNA_3405_Settings lna3405DevSettings;


	// Initialize hChip3250, hReg3250, hInt3250, and hBcm7038I2C . . .

	// *************************************************************************
	// Start of H/W board specific configuration for BLNA
	// *************************************************************************
	// Configure Bcm3405 Device
	BLNA_3405_GetDefaultSettings( &lna3405DevSettings, NULL );
	// Now initialize Bcm3405 specific data, Bcm3405 needs to know the
	//   I2C address of Bcm3405 device, addr=0x64 (7-bit address)
	lna3405DevSettings.i2cAddr = 0x64;
	lna3405DevSettings.gainMode = BLNA_GainMode_eAGC;
	// Open BLNA Device								 
	BLNA_3405_Open( &lnaDev, NULL, hBcm7038I2C, &lna3405DevSettings );
	// *************************************************************************
	// End of H/W board specific configuration for BLNA
	// *************************************************************************

	// Set to InBand port 0 to analog mode
	BLNA_SetInBandMode( lnaDev, 0, BLNA_InBandMode_eAnalog );
	
	// Enable AGC mode, with 8 dB gain, with a window of -0.6 dB to +0.6 dB
	BLNA_EnableAutomaticGainControl( lnaDev, 40, 3 ); 

	// Disable AGC mode and enable manual gain, with 10 dB gain
	BLNA_EnableManualGainControl( lnaDev, 50 ); 

}

</verbatim>
***************************************************************************/

#ifndef BLNA_H__
#define BLNA_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bkni.h"
#include "berr_ids.h"
#include "breg_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif


#define	BLNA_MX_INBAND_PORTS		(5)
#define	BLNA_MX_OUTOFBAND_PORTS		(5)
#define	BLNA_MX_EXTDRV				(5)
/***************************************************************************
Summary:
	Error Codes specific to BLNA

Description:

See Also:

****************************************************************************/
#define BLNA_ERR_OUTPUT_LEVEL_OUT_OF_RANGE		BERR_MAKE_CODE(BERR_LNA_ID, 0)
#define BLNA_ERR_INBAND_PORTNBR_OUT_OF_RANGE	BERR_MAKE_CODE(BERR_LNA_ID, 1)
#define BLNA_ERR_INBAND_MODE					BERR_MAKE_CODE(BERR_LNA_ID, 2)
#define BLNA_ERR_OUTOFBAND_PORTNBR_OUT_OF_RANGE	BERR_MAKE_CODE(BERR_LNA_ID, 3)
#define BLNA_ERR_OUTOFBAND_MODE					BERR_MAKE_CODE(BERR_LNA_ID, 4)
#define BLNA_ERR_EXTNBR_OUT_OF_RANGE			BERR_MAKE_CODE(BERR_LNA_ID, 5)


/***************************************************************************
Summary:
	The handles for Low Noise Amplifier module.

Description:
	An opaque handle for BLNA device.

See Also:
	BLNA_Open()

****************************************************************************/


typedef struct BLNA_P_Handle				*BLNA_Handle;

/***************************************************************************
Summary:
	Enumeration for LNA Gain mode

Description:
	This enumeration defines the valid LNA Gain mode.

See Also:
	BLNA_EnableAutomaticGainControl(), BLNA_EnableManualGainControl(),
	BLNA_GetStatus()

****************************************************************************/
typedef enum BLNA_GainMode
{
	BLNA_GainMode_eAGC,					/* Automatic Gain Control mode */
	BLNA_GainMode_eMGC,					/* Manual Gain Control mode */
	BLNA_GainMode_eNotInitialized		/* Not Initialized */
} BLNA_GainMode;

/***************************************************************************
Summary:
	Enumeration for LNA Boost mode

Description:
	This enumeration defines the valid LNA Boost modes.

See Also:

****************************************************************************/
typedef enum BLNA_BoostMode
{
	BLNA_BoostMode_eNormal,				/* Normal mode */
	BLNA_BoostMode_eBoost,				/* Boost mode */
	BLNA_BoostMode_eSuperBoost			/* Superboost mode */
} BLNA_BoostMode;

/***************************************************************************
Summary:
	Enumeration for LNA InBand port mode

Description:
	This enumeration defines the valid LNA InBand Port mode.

See Also:
	BLNA_SetInBandMode(), BLNA_GetStatus()

****************************************************************************/
typedef enum BLNA_InBandMode
{
	BLNA_InBandMode_eOpenCable,
	BLNA_InBandMode_eAnalog,
	BLNA_InBandMode_eDigital,
	BLNA_InBandMode_ePowerDownRf,
	BQOB_InBandMode_eLast
} BLNA_InBandMode;

/***************************************************************************
Summary:
	Enumeration for LNA OutOfBand port mode

Description:
	This enumeration defines the valid LNA OutOfBand Port mode.

See Also:
	BLNA_SetOutOfBandMode(), BLNA_GetStatus()

****************************************************************************/
typedef enum BLNA_OutOfBandMode
{
	BLNA_OutOfBandMode_eMaxGain,
	BLNA_OutOfBandMode_eBypass,
	BLNA_OutOfBandMode_eNormal,
	BLNA_OutOfBandMode_ePowerDownRf,
	BQOB_OutOfBandMode_eLast
} BLNA_OutOfBandMode;

/***************************************************************************
Summary:
	This structure represents the LNA Status.

Description:
	This structure is returned when BLNA_GetStatus() is called.  This
	structure contains the complete status of LNA module.

See Also:
	BLNA_GetStatus()

****************************************************************************/
typedef struct BLNA_Status
{
	BLNA_GainMode gainMode;				/* See BLNA_GainMode for description */
	BLNA_InBandMode inBandPorts[BLNA_MX_INBAND_PORTS];		/* InBand port mode */
	BLNA_OutOfBandMode outOfBandPorts[BLNA_MX_OUTOFBAND_PORTS];	/* OutOfBand port mode */
	bool isExternalDriverEnabled[BLNA_MX_EXTDRV]; /* Ext. Driver Enabled=1, otherwise disabled */
	unsigned int agcOutputLevel;		/* AGC Output Level, in 2/10 dB */
	int agcDeltaVal;					/* AGC Delta Value, in 2/10 dB */
	unsigned int manualGainVal;			/* If Manual Gain, manual gain value, in 2/10 dB */
	bool gainBoostEnabled;				/* True if Stage 1 gain boost is enabled */
	bool superBoostEnabled;				/* True if SuperBoost is enabled */
	bool tiltEnabled;					/* True if Stage 1 tilt enabled  */
} BLNA_Status;

/***************************************************************************
Summary:
	This function closes LNA module.

Description:
	This function is responsible for closing BLNA module. Closing BLNA 
	will free main BLNA handle.

Returns:
	TODO:

See Also:
	BLNA_Open()

****************************************************************************/
BERR_Code BLNA_Close(
	BLNA_Handle hLna					/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function enable the automatic gain control feature of LNA.

Description:
	This function is responsible for enabling the automatic gain control
	feature of LNA.  When enabling this feature, the manual gain control
	feature will be disabled.
	
Returns:
	TODO:

See Also:
	BLNA_EnableManualGainControl(), BLNA_GetStatus()

****************************************************************************/
BERR_Code BLNA_EnableAutomaticGainControl(
	BLNA_Handle hDev,					/* [in] Device handle */
	unsigned int agcOutputLevel,		/* [in] AGC Output Level, in 2/10 dB */
	int agcDeltaVal						/* [in] Delta value, specifics window, in 2/10 dB */
	);

/***************************************************************************
Summary:
	This function enable the manual gain control feature of LNA.

Description:
	This function is responsible for enabling the manual gain control
	feature of LNA.  When enabling this feature, the automatic gain control
	feature will be disabled.
	
Returns:
	TODO:

See Also:
	BLNA_EnableAutomaticGainControl(), BLNA_GetStatus()

****************************************************************************/
BERR_Code BLNA_EnableManualGainControl(
	BLNA_Handle hDev,					/* [in] Device handle */
	unsigned int gain					/* [in] Gain, in 2/10 dB */
	);

/***************************************************************************
Summary:
	This function sets the InBand port mode.

Description:
	This function is responsible for setting the InBand port's mode
	of operation.
	
Returns:
	TODO:

See Also:
	BLNA_GetStatus()

****************************************************************************/
BERR_Code BLNA_SetInBandMode(
	BLNA_Handle hDev,					/* [in] Device handle */
	unsigned int portNbr,				/* [in] Port number, 0-2 */
	BLNA_InBandMode mode				/* [in] InBand mode */
	);

/***************************************************************************
Summary:
	This function sets the OutOfBand port mode.

Description:
	This function is responsible for setting the OutOfBand port's mode
	of operation.
	
Returns:
	TODO:

See Also:
	BLNA_GetStatus()

****************************************************************************/
BERR_Code BLNA_SetOutOfBandMode(
	BLNA_Handle hDev,					/* [in] Device handle */
	unsigned int portNbr,				/* [in] Port number, 0-1 */
	BLNA_OutOfBandMode mode				/* [in] OutOfBand mode */
	);

/***************************************************************************
Summary:
	This function enables the external driver.

Description:
	This function is responsible for enabling the external driver on LNA.
	
Returns:
	TODO:

See Also:
	BLNA_GetStatus()

****************************************************************************/
BERR_Code BLNA_EnableExternalDriver(
	BLNA_Handle hDev,					/* [in] Device handle */
	unsigned int extNbr					/* [in] Extneral driver */
	);

/***************************************************************************
Summary:
	This function disables the external driver.

Description:
	This function is responsible for disabling the external driver on LNA.
	
Returns:
	TODO:

See Also:
	BLNA_GetStatus()

****************************************************************************/
BERR_Code BLNA_DisableExternalDriver(
	BLNA_Handle hDev,					/* [in] Device handle */
	unsigned int extNbr					/* [in] Extneral driver */
	);

/***************************************************************************
Summary:
	This function gets the status of LNA.

Description:
	This function is responsible for getting the complete status for
	LNA module.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BLNA_GetStatus(
	BLNA_Handle hDev,					/* [in] Device handle */
	BLNA_Status *pStatus				/* [out] Returns status */
	);

/***************************************************************************
Summary:
	This function reads the LNA's AGC value.

Description:
	This semi-private function is responsible for reading the
	LNA's AGC register value.  This function is specially written for
	Docsis use.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BLNA_GetLnaAgcRegVal(
	BLNA_Handle hDev,					/* [in] Device handle */
	uint32_t regOffset,					/* [in] AGC register offset */
	uint32_t *agcVal					/* [out] output value */
	);

/***************************************************************************
Summary:
	This function powers down the LNA.

Description:
	This function is responsible for putting the LNA into lowest-power mode.
		
Returns:
	TODO:

See Also:
    BLNA_SetInBandMode()
    BLNA_SetOutOfBandMode()

****************************************************************************/
BERR_Code BLNA_EnablePowerSaver(
	BLNA_Handle hDev					/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function powers up the LNA.

Description:
	This function is responsible for taking the LNA out of lowest-power mode.
	
Returns:
	TODO:

See Also:
    BLNA_SetInBandMode()
    BLNA_SetOutOfBandMode()

****************************************************************************/
BERR_Code BLNA_DisablePowerSaver(
	BLNA_Handle hDev					/* [in] Device handle */
	);


/***************************************************************************
Summary:
	This function enables loop through in the LNA.

Description:
		
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BLNA_EnableLoopThrough(
	BLNA_Handle hDev					/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function disables loop through in the LNA.

Description:
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BLNA_DisableLoopThrough(
	BLNA_Handle hDev					/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function allows the setting of gain and superboost.

Description:
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BLNA_SetBoostMode(
	BLNA_Handle hDev,					/* [in] Device handle */
	BLNA_BoostMode boostMode
	);

#ifdef __cplusplus
}
#endif
 
#endif



