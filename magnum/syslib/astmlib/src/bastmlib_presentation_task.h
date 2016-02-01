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
#include "bsyslib.h"
#include "bsyslib_list.h"

#ifndef BASTMLIB_PRESENTATION_TASK_H__
#define BASTMLIB_PRESENTATION_TASK_H__

#define BASTMLIB_PRESENTATION_DEFAULT_INITIAL_ACQUISITION_TIME 1000
#define BASTMLIB_PRESENTATION_DEFAULT_PROCESSING_FREQUENCY 5000
#define BASTMLIB_PRESENTATION_DEFAULT_SETTLING_TIME 3000
#define BASTMLIB_PRESENTATION_DEFAULT_TSM_DISABLED_WATCHDOG_TIMEOUT 120000

typedef enum
{
	BASTMlib_Presentation_StateMachineSignal_eReset,
	BASTMlib_Presentation_StateMachineSignal_ePresenterPass,
	BASTMlib_Presentation_StateMachineSignal_ePresenterFailure,
	BASTMlib_Presentation_StateMachineSignal_eClockReferencePass,
	BASTMlib_Presentation_StateMachineSignal_eClockReferenceFailure,
	BASTMlib_Presentation_StateMachineSignal_eMax
} BASTMlib_Presentation_StateMachineSignal;

typedef struct
{
	bool bAcquire;
	BSYSlib_Timer_Handle hTimer;
	BSYSlib_Timer_Handle hWatchdogTimer;
	uint32_t uiPresenterCount;
	BSYSlib_List_Handle hPresenters;
} BASTMlib_PresentationTaskState;

void BASTMlib_P_Presentation_GetDefaultConfig(
	BASTMlib_Config * psConfig
);

BERR_Code BASTMlib_P_Presentation_TimerExpired(
	void * pvParm1, /* first user context parameter [in] */ 
	int iParm2, /* second user context parameter [in] */ 
	BSYSlib_Timer_Handle hTimer /* the handle of the timer that expired [in] */ 
);

BERR_Code BASTMlib_P_Presentation_Process(
	BASTMlib_Handle hAstm
);

void BASTMlib_P_Presentation_RateControlStateMachine_SendSignal(
	BASTMlib_Handle hAstm,
	BASTMlib_Presentation_StateMachineSignal eSignal
);

void BASTMlib_P_Presentation_StcSourceStateMachine_SendSignal(
	BASTMlib_Handle hAstm,
	BASTMlib_Presentation_StateMachineSignal eSignal
);

BERR_Code BASTMlib_P_Presentation_StateChangeHandler(
	BASTMlib_Handle hAstm,
	bool bRateControlChanged
);

void BASTMlib_P_Presentation_GetMaxAcquisitionTime(
	BASTMlib_Handle hAstm,
	unsigned int * puiMaxAcquisitionTime
);

#endif /* BASTMLIB_PRESENTATION_TASK_H__ */

