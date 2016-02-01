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
#include "bastmlib.h"
#include "bastmlib_presentation_task.h"
#include "bastmlib_clock_coupling_task.h"
#include "bsyslib.h"
#include "bsyslib_list.h"

#ifndef BASTMLIB_PRIV_H__
#define BASTMLIB_PRIV_H__

struct BASTMlib_Impl
{
	bool bEnabled;
	bool bStarted;

	BASTMlib_Handle hAstm;
	BASTMlib_Settings sSettings;
	BASTMlib_Config sConfig;
	BASTMlib_Status sStatus;

	unsigned int uiPresentersCreated; /* TODO: needs to be reworked with create/destroy add/remove presenter stuff */
	BASTMlib_PresentationTaskState sPresentation;
	BASTMlib_ClockCouplingTaskState sClockCoupling;
};

void BASTMlib_P_GetDefaultConfig(
	BASTMlib_Config * psConfig
);

BERR_Code BASTMlib_P_StartTimer_isr(
	BASTMlib_Handle hAstm,
	BSYSlib_Timer_Handle hTimer, 
	unsigned long ulTimeout,
	BSYSlib_Timer_ExpiryHandler pfTimerExpired,
	void * pvParm1,
	int iParm2
);

bool BASTMlib_P_Enabled(
	BASTMlib_Handle hAstm
);

bool BASTMlib_P_Enabled_isr(
	BASTMlib_Handle hAstm
);

#endif /* BASTMLIB_PRIV_H__ */

