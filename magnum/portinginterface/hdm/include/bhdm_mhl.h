/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
The MHL (Mobile High-Definition Link) API is a library used to
provide MHL functionality between Broadcom cores and the connected MHL receivers.

The API includes support for installing callback associated with standby requests
from the receiver and updating the MHL FW that operates in the MPM micrprocessor
when the MHL-capable Broadcom SOC is in S3 standby mode.

</verbatim>
****************************************************************************/

#ifndef BHDM_MHL_H__
#define BHDM_MHL_H__


#include "bhdm.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
Summary:
MHL Basic Return Codes
*******************************************************************************/
#define BHDM_FORCED_MHL_MODE 0 /* Applies to non-sink powered MHL mode */
#define BHDM_MHL_ERRS  400     /* MHL error codes */


/******************************************************************************
Summary:
	install MHL standby Callback to notify of sink's message to go on standby

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pfCallback_isr - callback for MHL standby

*******************************************************************************/
BERR_Code BHDM_MHL_InstallStandbyCallback(
	BHDM_Handle hHDMI,			/* [in] HDMI Handle */
	const BHDM_CallbackFunc pfCallback_isr, /* [in] cb for informing MHL standby message */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */
	int iParm2) ;	/* [in] the second argument(int) passed to the callback function */


/******************************************************************************
Summary:
	Uninstall MHL Standby Callback

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pfCallback_isr - callback for  MHL standby

*******************************************************************************/
BERR_Code BHDM_MHL_UninstallStandbyCallback(
	BHDM_Handle hHDMI,						 /* [in] HDMI Handle */
	const BHDM_CallbackFunc pfCallback_isr) ; /* [in] cb for informing MHL standby message */

#ifdef __cplusplus
}
#endif
#endif /* BHDM_MHL_H__ */
