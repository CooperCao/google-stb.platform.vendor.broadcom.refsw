/***************************************************************************
 *     Copyright (c) 2005-2009, Broadcom Corporation
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
This module represents the implementation of 3255 In-band tuner.  It is
designed as a subdevice of BTNR device.  It contains the 'Custom' part
of a tuner module.  The 'General' part is implemented in BTNR.  Please see
BTNR documentation for a detail discussion on a tuner and BTNR.


Design
The design for BTNR_3255Ib PI API is broken into two parts.

o Part 1 (open/close):

    These APIs are used for opening and closing a specific BTNR_3255Ib device.

o Part 2 (create):

	This section is an API to create a general BTNR device handle from
	3255 In-band specific BTNR_3255Ib device handle.


Usage
The usage of BTNR_3255Ib involves the following:

   * Configure/Open of BTNR_3255Ib

      * Configure BTNR_3255Ib device for the target system
      * Open BTNR_3255Ib device
	  * Using Get/Set/Close functions


Interrupt Requirements:
None


Sample Code
Please see BTNR documentation for sample code.

</verbatim>
***************************************************************************/


#ifndef BTNR_3255IB_H__
#define BTNR_3255IB_H__

#include "bchp.h"
#include "bint.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
	Required default settings structure for DTT760x Tuner module.

Description:
	The default setting structure defines the default configuration of
	3255Ib tuner when the device is opened.

See Also:
	BTNR_3255Ib_Open(), BTNR_3255Ib_GetDefaultSettings()

****************************************************************************/
#define	BTNR_3255Ib_SETTINGS_IFFREQ			(44000000)		/* 44.00 MHz */
typedef struct
{
	unsigned int devId;					/* generic device ID */
	void *hGeneric;						/* generic handle can be used for anything */
	unsigned long ifFreq;				/* in Hertz */
} BTNR_3255Ib_Settings;


/***************************************************************************
Summary:
	This function opens 3255Ib Tuner module.

Description:
	This function is responsible for opening 3255Ib BTNR module. When BTNR is
	opened, it will create a module handle and configure the module based
	on the default settings. Once the device is opened, it must be closed
	before it can be opened again.

Returns:
	TODO:

See Also:
	BTNR_Close(), BTNR_3255Ib_GetDefaultSettings()

****************************************************************************/
BERR_Code BTNR_3255Ib_Open(
	BTNR_Handle *phDev,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_Handle hRegister,				/* [in] Register handle */
	BINT_Handle hInterrupt,				/* [in] Interrupt handle, Bcm3250 */
	const BTNR_3255Ib_Settings *pDefSettings /* [in] Default settings */
	);

/***************************************************************************
Summary:
	This function returns the default settings for 3255Ib Tuner module.

Description:
	This function is responsible for returns the default setting for
	BTNR module. The returning default setting should be when
	opening the device.

Returns:
	TODO:

See Also:
	BTNR_3255Ib_Open()

****************************************************************************/
BERR_Code BTNR_3255Ib_GetDefaultSettings(
	BTNR_3255Ib_Settings *pDefSettings,	/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	);

#ifdef __cplusplus
}
#endif

#endif



