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
This module represents the implementation of Bcm32505b (OutOfBand) tuner.
It is designed as a subdevice of BTNR device.  It contains the 'Custom' part
of a tuner module.  The 'General' part is implemented in BTNR.  Please see
BTNR documentation for a detail discussion on a tuner and BTNR.


Design
The design for BTNR_3255Ob PI API is broken into two parts.

o Part 1 (open/close):

    These APIs are used for opening and closing a specific BTNR_3255Ob device.

o Part 2 (create):

	This section is an API to create a general BTNR device handle from
	Bcm3255Ob specific BTNR_3255Ob device handle.


Usage
The usage of BTNR_3255Ob involves the following:

   * Configure/Open of BTNR_3255b

      * Configure BTNR_3255Ob device for the target system
      * Open BTNR_3255Ob device
	  * Using Get/Set/Close functions


Interrupt Requirements:
None


Sample Code
Please see BTNR documentation for sample code.

</verbatim>
***************************************************************************/

#ifndef BTNR_3255OB_H__
#define BTNR_3255OB_H__

#include "bchp.h"

#ifdef __cplusplus
extern "C" {
#endif



/***************************************************************************
Summary:
	Required default settings structure for OutOfBand Bcm3255 Tuner module.

Description:
	The default setting structure defines the default configuration of
	OutOfBand Bcm3255 tuner when the device is opened.

See Also:
	BTNR_3255Ob_Open(), BTNR_3255Ob_GetDefaultSettings()

****************************************************************************/
#if BCHP_CHIP == 7405 || BCHP_CHIP == 7420
#define	BTNR_3255OB_SETTINGS_IFFREQ			(1750000)		/* 1.75 MHz */
#elif BCHP_CHIP == 7125
#define	BTNR_3255OB_SETTINGS_IFFREQ			(1250000)		/* 1.25 MHz */
#else
#define	BTNR_3255OB_SETTINGS_IFFREQ			(44000000)		/* 44.00 MHz */
#endif
typedef struct
{
	unsigned int devId;					/* generic device ID */
	void *hGeneric;						/* generic handle can be used for anything */
	unsigned long ifFreq;				/* in Hertz */
} BTNR_3255Ob_Settings;


/***************************************************************************
Summary:
	This function opens OutOfBand Bcm3250 Tuner module.

Description:
	This function is responsible for opening OutOfBand Bcm3255 BTNR module.
	When BTNR is opened, it will create a module handle and configure the
	module based on the default settings. Once the device is opened, it
	must be closed before it can be opened again.

	Note: This function requires BAOB_Handle, which is used to
	communicate to Bcm3255 OutOfBand core.  Access to BQOB core will
	be serialized by BTNR_32505b.

Returns:
	TODO:

See Also:
	BTNR_Close(), BTNR_3255Ob_GetDefaultSettings()

****************************************************************************/
BERR_Code BTNR_3255Ob_Open(
	BTNR_Handle *phDev,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	const BTNR_3255Ob_Settings *pDefSettings /* [in] Default settings */
	);

/***************************************************************************
Summary:
	This function returns the default settings for OutOfBand Bcm3255 Tuner module.

Description:
	This function is responsible for returns the default setting for
	BTNR module. The returning default setting should be when
	opening the device.

Returns:
	TODO:

See Also:
	BTNR_3255Ob_Open()

****************************************************************************/
BERR_Code BTNR_3255Ob_GetDefaultSettings(
	BTNR_3255Ob_Settings *pDefSettings,	/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	);



#ifdef __cplusplus
}
#endif

#endif



