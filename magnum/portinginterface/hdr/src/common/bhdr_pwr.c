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


#include "bhdr_pwr.h"
#include "bhdr_fe_priv.h"

BDBG_MODULE(BHDR_PWR) ;


/******************************************************************************
Summary: Get Default standby Settings 
*******************************************************************************/
void BHDR_GetDefaultStandbySettings(
	BHDR_StandbySettings *pSettings
	)
{
	BDBG_ENTER(BHDR_GetDefaultStandbySettings) ;
	
	pSettings->bEnableWakeup = false ;
	
	BDBG_LEAVE(BHDR_GetDefaultStandbySettings) ;
}


/******************************************************************************
Summary: Enter standby mode
*******************************************************************************/
BERR_Code BHDR_Standby(
	BHDR_FE_Handle hFrontEnd, /* [in] HDR FE Handle */
	const BHDR_StandbySettings *pSettings
	)
{
#if BCHP_PWR_SUPPORT	
	BERR_Code rc = BERR_SUCCESS ;
	BHDR_StandbySettings settings;

	BDBG_ENTER(BHDR_Standby);
	BDBG_OBJECT_ASSERT(hFrontEnd, BHDR_FE_P_Handle) ;

		
	if (hFrontEnd->standby) {
		BDBG_ERR(("Already in standby"));
		rc = BERR_UNKNOWN ;
		goto done ;
	}
    
	if (pSettings == NULL) 
		BHDR_GetDefaultStandbySettings(&settings);
	else 
		settings = *pSettings;
    
	if (settings.bEnableWakeup) 
	{
		hFrontEnd->enableWakeup = true;
	}
	hFrontEnd->standby = true;

        BHDR_FE_P_PowerResourceRelease_DVP_HR(hFrontEnd) ;

done:
	BDBG_LEAVE(BHDR_Standby);
	return rc ; /* this will release the HDMI_RX_TMDS resource */
#else
	BSTD_UNUSED(hFrontEnd) ;
	BSTD_UNUSED(pSettings) ;
	return BERR_SUCCESS  ; 
#endif
}


/******************************************************************************
Summary: Resume from standby mode
*******************************************************************************/
BERR_Code BHDR_Resume(
	BHDR_FE_Handle hFrontEnd /* [in] HDR FE Handle */
	)
{
#if BCHP_PWR_SUPPORT	
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDR_Resume);
	BDBG_OBJECT_ASSERT(hFrontEnd, BHDR_FE_P_Handle) ;

	if (!hFrontEnd->standby) 
	{
		BDBG_ERR(("Not in standby"));
		rc = BERR_UNKNOWN ;
		goto done ;
	}

	if (hFrontEnd->enableWakeup) 
	{
		hFrontEnd->enableWakeup = false;
	}

	hFrontEnd->standby = false;
        BHDR_FE_P_PowerResourceAcquire_DVP_HR(hFrontEnd) ;

done:
	BDBG_LEAVE(BHDR_Resume);
	return rc ;
#else
	BSTD_UNUSED(hFrontEnd) ;
	return BERR_SUCCESS  ; 
#endif
}

