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
 *
 * Revision History: $
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BTNR_3462_H__
#define BTNR_3462_H__

#include "bchp.h"
#include "breg_i2c.h"
#include "bhab.h"
#include "btnr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned short i2cAddr; /* 7bit I2C address of Bcm3462 */
    unsigned int channelNo; /* Channel number to tune to */
    unsigned int mxChnNo;   /* Max tuner channels */    
} BTNR_3462_Settings;

/***************************************************************************
Summary:
    This function returns the default settings for Bcm34xx Tuner module.

Description:
    This function is responsible for returns the default setting for 
    BTNR module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BTNR_3462_Open()

****************************************************************************/
BERR_Code BTNR_3462_GetDefaultSettings(
    BTNR_3462_Settings *pDefSettings  /* [out] Returns default setting */
    );


/***************************************************************************
Summary:
    This function opens Bcm34xx Tuner module.

Description:
    This function is responsible for opening Bcm34xx BTNR module. When BTNR 
    is opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

Returns:
    TODO:

See Also:
    BTNR_Close(), BTNR_34xx_GetDefaultSettings()

****************************************************************************/
BERR_Code BTNR_3462_Open(
    BTNR_Handle *phDev,                 /* [out] Returns handle */
    BTNR_3462_Settings *pSettings,
    BHAB_Handle hHab
    );

#ifdef __cplusplus
}
#endif
 
#endif



