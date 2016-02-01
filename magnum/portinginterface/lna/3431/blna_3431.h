/***************************************************************************
 *     Copyright (c) 2003-2006, Broadcom Corporation
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
 * This module is the driver for BCM3431 LNA.
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
/*= Module Overview *********************************************************
<verbatim>

Overview
The Low Noise Amplifier (LNA) is responsible for providing input signal.
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
	BLNA_3431_Settings lna3431DevSettings;


	// Initialize hChip3250, hReg3250, hInt3250, and hBcm7038I2C . . .

	// *************************************************************************
	// Start of H/W board specific configuration for BLNA
	// *************************************************************************
	// Configure Bcm3431 Device
	BLNA_3431_GetDefaultSettings( &lna3431DevSettings, NULL );
	// Now initialize Bcm3431 specific data, Bcm3431 needs to know the
	//   I2C address of Bcm3431 device, addr=0x65 (7-bit address)
	lna3431sDevSettings.i2cAddr = 0x65;
	// Open BLNA Device								 
	BLNA_3431_Open( &lnaDev, NULL, hBcm7038I2C, &lna3431DevSettings );
	// *************************************************************************
	// End of H/W board specific configuration for BLNA
	// *************************************************************************
	
}

</verbatim>
***************************************************************************/


#ifndef BLNA_3431_H__
#define BLNA_3431_H__

#ifdef __cplusplus
extern "C" {
#endif

#define	BLNA_3431_MX_INBAND_PORTS		(3)
#define	BLNA_3431_MX_EXTDRV				(2)

/***************************************************************************
Summary:
	Required default settings structure for LNA module.

Description:
	The default setting structure defines the default configuration of
	LNA when the device is opened.

See Also:
	BLNA_Open()

****************************************************************************/
#define BLNA_3431_SETTINGS_I2CADDR					(0x65)
#define BLNA_3431_SETTINGS_INBANDPORTS				(BLNA_InBandMode_eDigital)

typedef struct BLNA_3431_Settings
{
	unsigned short i2cAddr;				/* 7bit I2C address of Bcm3431 */
	BLNA_InBandMode inBandPorts[BLNA_MX_INBAND_PORTS];		/* InBand port mode */

} BLNA_3431_Settings;

/***************************************************************************
Summary:
	This function opens LNA module.

Description:
	This function is responsible for opening BLNA module. When BLNA is
	opened, it will create a module handle and configure the module based
	on the default settings. Once the device is opened, it must be closed
	before it can be opened again.

Returns:
	TODO:

See Also:
	BLNA_Close(), BLNA_GetDefaultSettings()

****************************************************************************/
BERR_Code BLNA_3431_Open(
	BLNA_Handle *phLna,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_I2C_Handle hI2cReg,			/* [in] I2C Register handle */
	const BLNA_3431_Settings *pDefSettings	/* [in] Default settings */
	);


/***************************************************************************
Summary:
	This function returns the default settings for LNA module.

Description:
	This function is responsible for returns the default setting for 
	BLNA module. The returning default setting should be when
	opening the device.

Returns:
	TODO:

See Also:
	BLNA_Open()

****************************************************************************/
BERR_Code BLNA_3431_GetDefaultSettings(
	BLNA_3431_Settings *pDefSettings,		/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	);

#ifdef __cplusplus
}
#endif
 
#endif



