/***************************************************************************
*     Copyright (c) 2004-2009, Broadcom Corporation
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
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bsyslib.h"
#include "bastmlib.h"
#include "bastmlib_priv.h"

BDBG_MODULE(astmlib);

BERR_Code BASTMlib_P_StartTimer_isr(
	BASTMlib_Handle hAstm,
	BSYSlib_Timer_Handle hTimer, 
	unsigned long ulTimeout,
	BSYSlib_Timer_ExpiryHandler pfTimerExpired,
	void * pvParm1,
	int iParm2
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYSlib_Timer_Start_isr pfStart_isr;
	BSYSlib_Timer_Cancel_isr pfCancel_isr;
	
	BDBG_ENTER(BASTMlib_P_StartTimer_isr);
	
	if (hAstm->sSettings.cbTimer.pfStart_isr)
	{
		BSYSlib_Timer_Settings sTimer;

		sTimer.ulTimeout = ulTimeout;
		sTimer.pvParm1 = pvParm1;
		sTimer.iParm2 = iParm2;
		sTimer.pfTimerExpired = pfTimerExpired;

		pfStart_isr = hAstm->sSettings.cbTimer.pfStart_isr;
		pfCancel_isr = hAstm->sSettings.cbTimer.pfCancel_isr;
		pvParm1 = hAstm->sSettings.cbTimer.pvParm1;
		iParm2 = hAstm->sSettings.cbTimer.iParm2;

		/* clean up old timer if any */
		if (pfCancel_isr)
		{
			pfCancel_isr(pvParm1, iParm2, hTimer);
		}

		/* (re)schedule timer */
		rc = pfStart_isr(pvParm1, iParm2, hTimer, &sTimer);
		if (rc) goto error;
	}

	goto end;

error:

end:

	BDBG_LEAVE(BASTMlib_P_StartTimer_isr);
	return rc;
}

void BASTMlib_P_GetDefaultConfig(
	BASTMlib_Config * psConfig
)
{
	BDBG_ENTER(BASTMlib_P_GetDefaultConfig);

	BDBG_ASSERT(psConfig);

	psConfig->bEnabled = true;
	psConfig->eStcRate = BASTMlib_ClockRate_e45Khz;

	BASTMlib_P_Presentation_GetDefaultConfig(psConfig);
	BASTMlib_P_ClockCoupling_GetDefaultConfig(psConfig);

	BDBG_LEAVE(BASTMlib_P_GetDefaultConfig);
}

bool BASTMlib_P_Enabled(
	BASTMlib_Handle hAstm
)
{
	bool bEnabled = false;
	
	BDBG_ENTER(BASTMlib_P_Enabled);

	BDBG_ASSERT(hAstm);

	BKNI_EnterCriticalSection();
	bEnabled = BASTMlib_P_Enabled_isr(hAstm);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BASTMlib_P_Enabled);
	return bEnabled;
}

bool BASTMlib_P_Enabled_isr(
	BASTMlib_Handle hAstm
)
{
	bool bEnabled = false;
	
	BDBG_ENTER(BASTMlib_P_Enabled_isr);

	BDBG_ASSERT(hAstm);

	bEnabled = hAstm->bEnabled;

	BDBG_LEAVE(BASTMlib_P_Enabled_isr);
	return bEnabled;
}

