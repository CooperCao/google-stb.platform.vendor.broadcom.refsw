/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
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
#include "bastmlib_clock_reference.h"
#include "bsyslib.h"

#ifndef BASTMLIB_CLOCK_COUPLING_TASK_H__
#define BASTMLIB_CLOCK_COUPLING_TASK_H__

#define BASTMLIB_CLOCK_COUPLING_DEFAULT_INITIAL_ACQUISITION_TIME 300
#define BASTMLIB_CLOCK_COUPLING_DEFAULT_PROCESSING_FREQUENCY 2000
#define BASTMLIB_CLOCK_COUPLING_DEFAULT_IDEAL_PROCESSING_FREQUENCY 10000
#define BASTMLIB_CLOCK_COUPLING_DEFAULT_SETTLING_TIME 500

typedef enum
{
	BASTMlib_ClockCoupling_StateMachineSignal_eReset,
	BASTMlib_ClockCoupling_StateMachineSignal_eClockReferencePass,
	BASTMlib_ClockCoupling_StateMachineSignal_eClockReferenceFailure,
	BASTMlib_ClockCoupling_StateMachineSignal_eMax
} BASTMlib_ClockCoupling_StateMachineSignal;

typedef struct
{
	bool bAcquire;
	BSYSlib_Timer_Handle hTimer;
	BASTMlib_ClockReference_Handle hReference;
} BASTMlib_ClockCouplingTaskState;

void BASTMlib_P_ClockCoupling_GetDefaultConfig(
	BASTMlib_Config * psConfig
);

BERR_Code BASTMlib_P_ClockCoupling_TimerExpired(
	void * pvParm1, /* first user context parameter [in] */ 
	int iParm2, /* second user context parameter [in] */ 
	BSYSlib_Timer_Handle hTimer /* the handle of the timer that expired [in] */ 
);

BERR_Code BASTMlib_P_ClockCoupling_Process(
	BASTMlib_Handle hAstm
);

void BASTMlib_P_ClockCoupling_StateMachine_SendSignal(
	BASTMlib_Handle hAstm,
	BASTMlib_ClockCoupling_StateMachineSignal eSignal
);

BERR_Code BASTMlib_P_ClockCoupling_StateChangeHandler(
	BASTMlib_Handle hAstm 
);

void BASTMlib_P_ClockCoupling_GetMaxAcquisitionTime(
	BASTMlib_Handle hAstm,
	unsigned int * puiMaxAcquisitionTime
);

#endif /* BASTMLIB_CLOCK_COUPLING_TASK_H__ */

