/***************************************************************************
 *     Copyright (c) 2005, Broadcom Corporation
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
This module represents the implementation of Thompson DTT760x tuner.  It is 
designed as a subdevice of BTNR device.  It contains the 'Custom' part
of a tuner module.  The 'General' part is implemented in BTNR.  Please see
BTNR documentation for a detail discussion on a tuner and BTNR.


Design
The design for BTNR_DTT760X PI API is broken into two parts.

o Part 1 (open/close):

    These APIs are used for opening and closing a specific BTNR_DTT760X device.

o Part 2 (create):

	This section is an API to create a general BTNR device handle from
	DTT760x specific BTNR_DTT760X device handle.


Usage
The usage of BTNR_DTT760X involves the following:

   * Configure/Open of BTNR_DTT760X

      * Configure BTNR_DTT760X device for the target system
      * Open BTNR_DTT760X device
	  * Using Get/Set/Close functions


Interrupt Requirements:
None


Sample Code
Please see BTNR documentation for sample code.

</verbatim>
***************************************************************************/


#ifndef BTNR_DTT760X_H__
#define BTNR_DTT760X_H__

#include "bchp.h"
#include "breg_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
	Required default settings structure for DTT760x Tuner module.

Description:
	The default setting structure defines the default configuration of
	DTT760x tuner when the device is opened.

See Also:
	BTNR_DTT760X_Open(), BTNR_DTT760X_GetDefaultSettings()

****************************************************************************/
#define	BTNR_DTT760X_SETTINGS_IFFREQ			(44000000)		/* 44.00 MHz */
#define	BTNR_DTT760X_SETTINGS_I2CADDR			(0x61)			/* 7bit addr */
typedef struct
{
	unsigned long ifFreq;				/* in Hertz */
	unsigned short i2cAddr;				/* 7bit I2C address of DTT760x */
} BTNR_DTT760X_Settings;


/***************************************************************************
Summary:
	This function opens DTT760x Tuner module.

Description:
	This function is responsible for opening DTT760x BTNR module. When BTNR is
	opened, it will create a module handle and configure the module based
	on the default settings. Once the device is opened, it must be closed
	before it can be opened again.

Returns:
	TODO:

See Also:
	BTNR_Close(), BTNR_DTT760X_GetDefaultSettings()

****************************************************************************/
BERR_Code BTNR_DTT760X_Open(
	BTNR_Handle *phDev,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_I2C_Handle hI2CReg,			/* [in] I2C Register handle */
	const BTNR_DTT760X_Settings *pDefSettings /* [in] Default settings */
	);

/***************************************************************************
Summary:
	This function returns the default settings for DTT760x Tuner module.

Description:
	This function is responsible for returns the default setting for 
	BTNR module. The returning default setting should be when
	opening the device.

Returns:
	TODO:

See Also:
	BTNR_DTT760X_Open()

****************************************************************************/
BERR_Code BTNR_DTT760X_GetDefaultSettings(
	BTNR_DTT760X_Settings *pDefSettings,	/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	);


#ifdef __cplusplus
}
#endif
 
#endif



