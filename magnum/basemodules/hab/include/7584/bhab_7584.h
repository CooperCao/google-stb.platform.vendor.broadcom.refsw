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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BHAB_7584_H
#define BHAB_7584_H

#ifdef __cplusplus
extern "C" {
#endif


#include "bhab.h"

/***************************************************************************
Summary:
	This function returns the default settings for 7584 module.

Description:
	This function is responsible for returning the default settings for 
	7584 module. The returning default setting should be used when
	opening the device.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BHAB_7584_GetDefaultSettings(
	BHAB_Settings * pDefSetting     /* [in] Default settings */
);

/***************************************************************************
Summary:
	This function returns the default recalibrate settings for a 7584 module.

Description:
	This function is responsible for returning the default recalibrate settings
    for a 7584 module.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BHAB_7584_GetDefaultRecalibrateSettings(
    BHAB_RecalibrateSettings *pRecalibrateSettings /* [out] default recalibrate settings */
);

#ifdef __cplusplus
}
#endif

#endif
