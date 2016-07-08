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
#include "bchp.h"       /* Chip Info */
#include "breg_mem.h"   /* Chip register access. */
#include "bkni.h"       /* Kernel Interface */
#include "bint.h"       /* Interrupt */
#include "btmr.h"       /* Timer Handle  */

#include "berr_ids.h"   /* Error codes */
#include "bdbg.h"       /* Debug Support */
#include "../common/bhdm_priv.h"
#include "bhdm_mhl.h"


BDBG_MODULE(BHDM_MHL) ;
BDBG_OBJECT_ID(HDMI_MHL);


/******************************************************************************
Summary: Install MHL Standby Callback
*******************************************************************************/
BERR_Code BHDM_MHL_InstallStandbyCallback(
	const BHDM_Handle hHDMI,                /* [in] HDMI Handle */
	const BHDM_CallbackFunc pfCallback_isr, /* [in] cb for MHL standby */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */
	int iParm2)    /* [in] the second argument(int) passed to the callback function */
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDM_MHL_InstallStandbyCallback) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* Check if this is a valid function */
	if( pfCallback_isr == NULL )
	{
		rc = BERR_TRACE(BERR_INVALID_PARAMETER);
		return rc;
	}

	BKNI_EnterCriticalSection() ;

	hHDMI->hMhl->pfStandbyCallback = pfCallback_isr;
	hHDMI->hMhl->pvStandbyParm1 = pvParm1;
	hHDMI->hMhl->iStandbyParm2 = iParm2;

	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDM_MHL_InstallStandbyCallback);

	return rc ;
}


/******************************************************************************
Summary: Uninstall MHL Standby Callback
*******************************************************************************/
BERR_Code BHDM_MHL_UninstallStandbyCallback(
	const BHDM_Handle hHDMI,                /* [in] HDMI Handle */
	const BHDM_CallbackFunc pfCallback_isr) /* [in] cb for format changes */
{
	BERR_Code rc = BERR_SUCCESS ;
	BSTD_UNUSED(pfCallback_isr) ;

	BDBG_ENTER(BHDM_MHL_UninstallStandbyCallback) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_EnterCriticalSection() ;
		hHDMI->hMhl->pfStandbyCallback = (BHDM_CallbackFunc) NULL ;
#ifdef BHDM_CONFIG_MHL_SUPPORT
	if (hHDMI->bMhlMode)
	{
		hHDMI->hMhl->pfStandbyCallback = (BHDM_CallbackFunc)NULL;
	}
#endif

	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BHDM_MHL_UninstallStandbyCallback) ;
	return rc;
}
