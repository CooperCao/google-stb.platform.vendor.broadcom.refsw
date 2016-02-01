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
#ifndef BTNR_7584OB_H__
#define BTNR_7584OB_H__

#include "bchp.h"
#include "breg_i2c.h"
#include "bhab.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BTNR_OOB_RF_INPUT_MODE 0x24
/***************************************************************************
Summary:
    Enumeration for RF Input mode

Description:
    This enumeration defines the AOB RF Input mode

See Also:

****************************************************************************/
typedef enum BTNR_7584Ob_OutOfBandInputMode
{
    BTNR_7584Ob_OutOfBandInputMode_eOutOfBandAdc,   /* OOB ADC connected to OOB Demod */
    BTNR_7584Ob_OutOfBandInputMode_eWideBandAdc,    /* WB ADC connected to OOB Demod */
    BTNR_7584Ob_OutOfBandInputMode_eBandPassFilteredOutOfBandAdc, /* OOB ADC connected to OOB Demod.  The signal is band pass filtered to filter out frequencies below 70Mhz,
                                                                     as normal out of band frequency range is from 70-130 Mhz. It also helps in saving power */
    BTNR_7584Ob_OutOfBandInputMode_eLast    
} BTNR_7584Ob_OutOfBandInputMode;

typedef struct
{
	unsigned short i2cAddr;			/* 7bit I2C address of Bcm7584 */	
	unsigned ifFreq;
    BTNR_7584Ob_OutOfBandInputMode inputMode;   /* RF input mode for AOB */
} BTNR_7584Ob_Settings;

/***************************************************************************
Summary:
	This function returns the default settings for Bcm7584 Tuner module.

Description:
	This function is responsible for returns the default setting for 
	BTNR module. The returning default setting should be when
	opening the device.

Returns:
	TODO:

See Also:
	BTNR_7584_Open()

****************************************************************************/
BERR_Code BTNR_7584Ob_GetDefaultSettings(
	BTNR_7584Ob_Settings *pDefSettings	/* [out] Returns default setting */
	);


/***************************************************************************
Summary:
	This function opens Bcm7584 Tuner module.

Description:
	This function is responsible for opening Bcm7584 BTNR module. When BTNR is
	opened, it will create a module handle and configure the module based
	on the default settings. Once the device is opened, it must be closed
	before it can be opened again.

Returns:
	TODO:

See Also:
	BTNR_Close(), BTNR_7584_GetDefaultSettings()

****************************************************************************/
BERR_Code BTNR_7584Ob_Open(
	BTNR_Handle *phDev,					/* [out] Returns handle */
	BTNR_7584Ob_Settings *pSettings,
	BHAB_Handle hHab
	);

#ifdef __cplusplus
}
#endif
 
#endif



