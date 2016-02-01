/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
#ifndef BHDR_PWR_H__
#define BHDR_PWR_H__

#include "bchp_pwr.h"	 
#include "bhdr_fe.h"       /* Chip Info */



/***************************************************************************
Summary:
	HDMI Rx  standby settings
	
****************************************************************************/
typedef struct BHDR_StandbySettings
{
	bool bEnableWakeup; /* If true, then allows wakeup from standby using HDMI Rx. 
	                       If false, the device is powered down during standby */
} BHDR_StandbySettings;

/******************************************************************************
Summary:
	Get default HDMI Rx standby settings
	
*******************************************************************************/
void BHDR_GetDefaultStandbySettings(
	BHDR_StandbySettings *pSettings
	);


/******************************************************************************
Summary:
	Enter standby mode
	
*******************************************************************************/
BERR_Code BHDR_Standby(
        BHDR_FE_Handle hFrontEnd, /* [in] HDR FE Handle */
	const BHDR_StandbySettings *pSettings /* optional */
	);

/******************************************************************************
Summary:
	Resume from standby mode
	
*******************************************************************************/
BERR_Code BHDR_Resume(
	BHDR_FE_Handle hFrontEnd /* [in] HDR FE Handle */
	);

#ifdef __cplusplus
}
#endif

#endif /* BHDR_PWR_H__ */


