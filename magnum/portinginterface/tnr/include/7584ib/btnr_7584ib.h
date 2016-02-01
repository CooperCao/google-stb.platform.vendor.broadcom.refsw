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
#ifndef BTNR_7584IB_H__
#define BTNR_7584IB_H__

#include "bchp.h"
#include "breg_i2c.h"
#include "bhab.h"
#include "btnr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned short i2cAddr; /* 7bit I2C address of Bcm7584 */
    unsigned int channelNo; /* Channel number to tune to */
} BTNR_7584Ib_Settings;

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
BERR_Code BTNR_7584Ib_GetDefaultSettings(
    BTNR_7584Ib_Settings *pDefSettings  /* [out] Returns default setting */
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
BERR_Code BTNR_7584Ib_Open(
    BTNR_Handle *phDev,                 /* [out] Returns handle */
    BTNR_7584Ib_Settings *pSettings,
    BHAB_Handle hHab
    );

/***************************************************************************
Summary:
    This function returns the default IF DAC settings.

Description:
    This function is responsible for returing the default settings for 
    IF DAC.

Returns:
    TODO:

See Also:
    BTNR_TuneIfDac()

****************************************************************************/    
BERR_Code BTNR_7584Ib_GetDefaultIfDacSettings(
    BTNR_IfDacSettings *pDefIfdacSettings  /* [out] Returns default setting */
    );
    
#ifdef __cplusplus
}
#endif
 
#endif



