/***************************************************************************
 *     Copyright (c) 2006-2012, Broadcom Corporation
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
#ifndef BCHP_AVS_H__
#define BCHP_AVS_H__

#include "bstd.h"
#include "bchp.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    The handle for AVS features.

Description:
    An opaque handle for AVS features.
    This is returned on a successful Open call and is passed into the
    Monitor and Close calls as input parameter.

See Also:
    BCHP_P_AvsOpen(), BCHP_P_AvsClose(), BCHP_P_AvsMonitorPvt.

****************************************************************************/
typedef struct BCHP_P_AvsContext *BCHP_P_AvsHandle;


/***************************************************************************
Summary:
    Open a chip handle for AVS processing.

Description:
    This function opens a chip handle for AVS processing.

Input:
    hRegister - A valid chip register handle previous open using BREG_Open.

Output:
    phHandle - Upon successful completion of open this pointer is non NULL
    and contains valid information about this chip.  This handle is used
    on subsequences BCHP_??? API.  *phHandle is NULL if failure.

See Also:
    BCHP_P_AvsClose, BCHP_P_AvsMonitorPvt

**************************************************************************/
BERR_Code BCHP_P_AvsOpen (
    BCHP_P_AvsHandle *phHandle,  /* [out] returns new handle on success */
    BCHP_Handle       hChip);    /* [in] handle for chip data */

/***************************************************************************
Summary:
    Close a AVS chip handle.

Description:
    This function closes a chip handle for AVS processing.

Input:
    hHandle - The handle supplied by a successful BCHP_AvsOpen call.

See Also:
    BCHP_P_AvsOpen

**************************************************************************/
BERR_Code BCHP_P_AvsClose (
    BCHP_P_AvsHandle hHandle ); /* [in] handle supplied from open */

/***************************************************************************
Summary:
    Process AVS data.

Description:
    This function is the periodic processing function for the AVS.
    This should be called on a timely basis (every second) to ensure processing gets done.

Input:
    hHandle - The handle supplied by a successful BCHP_AvsOpen call.

See Also:
    BCHP_P_AvsOpen

**************************************************************************/
BERR_Code BCHP_P_AvsMonitorPvt ( 
    BCHP_P_AvsHandle hHandle); /* [in] handle supplied from open */

/***************************************************************************
Summary:
    Return status data for AVS.

Description:
    This can be used to get the current voltage and temperature of the part.

Input:
    hHandle - The handle supplied by a successful BCHP_P_AvsOpen call.
	pData - is pointer to location to return the data.

See Also:
    BCHP_P_AvsOpen, BCHP_P_AvsMonitorPvt

**************************************************************************/
BERR_Code BCHP_P_AvsGetData(
	BCHP_P_AvsHandle hHandle, /* [in] handle supplied from open */
    BCHP_AvsData *pData);     /* [out] location to put data */

/***************************************************************************
Summary:
    Enter/exit stand-by mode.

Description:
    This can be used to have AVS hardware enter stand-by (low power) mode.
	Once entered, calls to BCHP_P_AvsMonitorPvt can be used but will be non-functional.

Input:
    hHandle - The handle supplied by a successful BCHP_P_AvsOpen call.
	activate - is set to true to enter stand-by mode, false returns from stand-by.

See Also:
    BCHP_P_AvsOpen, BCHP_P_AvsMonitorPvt

**************************************************************************/
BERR_Code BCHP_P_AvsStandbyMode(
	BCHP_P_AvsHandle hHandle, /* [in] handle supplied from open */
	bool activate);           /* [in] true to enter low power mode */

#ifdef __cplusplus
}
#endif

#endif /* BCHP_AVS_H__ */

