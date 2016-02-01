/***************************************************************************
*     Copyright (c) 2004-2013, Broadcom Corporation
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
#include "bastmlib_clock_coupling_task.h"
#include "bastmlib_clock_reference_priv.h"
#include "bastmlib_presentation_task.h"

BDBG_MODULE(astmlib);

#define BASTMLIB_DEBUG_QUEUE 0
#define BASTMLIB_DEBUG_SIGNALS 0

void BASTMlib_P_ClockCoupling_GetDefaultConfig(
	BASTMlib_Config * psConfig
)
{
	BASTMlib_ClockReference_Config sReferenceConfig;

	BDBG_ENTER(BASTMlib_P_ClockCoupling_GetDefaultConfig);

	BDBG_ASSERT(psConfig);

	BASTMlib_ClockReference_GetDefaultConfig(&sReferenceConfig);

	psConfig->sClockCoupling.uiMinimumTimeBetweenEvents = sReferenceConfig.uiMinimumTimeBetweenEvents;
	psConfig->sClockCoupling.eClockReferenceDomain = sReferenceConfig.eClockReferenceDomain;
	psConfig->sClockCoupling.uiDeviationThreshold = sReferenceConfig.uiDeviationThreshold;
	psConfig->sClockCoupling.uiDeviantCountThreshold = sReferenceConfig.uiDeviantCountThreshold;
	psConfig->sClockCoupling.uiIdealCountThreshold = sReferenceConfig.uiIdealCountThreshold;
	psConfig->sClockCoupling.uiInitialAcquisitionTime = BASTMLIB_CLOCK_COUPLING_DEFAULT_INITIAL_ACQUISITION_TIME;
	psConfig->sClockCoupling.uiProcessingFrequency = BASTMLIB_CLOCK_COUPLING_DEFAULT_PROCESSING_FREQUENCY;
	psConfig->sClockCoupling.uiIdealProcessingFrequency = BASTMLIB_CLOCK_COUPLING_DEFAULT_IDEAL_PROCESSING_FREQUENCY;
	psConfig->sClockCoupling.uiSettlingTime = BASTMLIB_CLOCK_COUPLING_DEFAULT_SETTLING_TIME;
	psConfig->sClockCoupling.ePreferredClockCoupling = BASTMlib_ClockCoupling_eInputClock;

	BDBG_LEAVE(BASTMlib_P_ClockCoupling_GetDefaultConfig);
}

void BASTMlib_P_ClockCoupling_GetMaxAcquisitionTime(
	BASTMlib_Handle hAstm,
	unsigned int * puiMaxAcquisitionTime
)
{
	BDBG_ENTER(BASTMlib_P_ClockCoupling_GetMaxAcquisitionTime);

	BDBG_ASSERT(hAstm);
	BDBG_ASSERT(puiMaxAcquisitionTime);

	*puiMaxAcquisitionTime = hAstm->sConfig.sClockCoupling.uiInitialAcquisitionTime 
		> hAstm->sConfig.sClockCoupling.uiProcessingFrequency 
		? hAstm->sConfig.sClockCoupling.uiInitialAcquisitionTime 
		: hAstm->sConfig.sClockCoupling.uiProcessingFrequency;

	*puiMaxAcquisitionTime = *puiMaxAcquisitionTime 
		> hAstm->sConfig.sClockCoupling.uiIdealProcessingFrequency
		? *puiMaxAcquisitionTime
		: hAstm->sConfig.sClockCoupling.uiIdealProcessingFrequency;

	BDBG_LEAVE(BASTMlib_P_ClockCoupling_GetMaxAcquisitionTime);
}

BERR_Code BASTMlib_P_ClockCoupling_TimerExpired(
	void * pvParm1, /* first user context parameter [in] */ 
	int iParm2, /* second user context parameter [in] */ 
	BSYSlib_Timer_Handle hTimer /* the handle of the timer that expired [in] */ 
)
{
	BERR_Code rc = BERR_SUCCESS;
	BASTMlib_Handle hAstm = pvParm1;

	BDBG_ENTER(BASTMlib_P_ClockCoupling_TimerExpired);

	BDBG_ASSERT(hAstm);

	BSTD_UNUSED(iParm2);
	BSTD_UNUSED(hTimer);

	if (hAstm->sConfig.bEnabled && hAstm->bStarted)
	{
		if (hAstm->sClockCoupling.bAcquire)
		{
			BDBG_MSG(("Clock coupling processing timer expired"));
			/* if we are acquiring, then this is a processing timer expiry event */
			rc = BASTMlib_P_ClockCoupling_Process(hAstm);
			if (rc) goto error;
		}
		else
		{
			BDBG_MSG(("Clock coupling settling timer expired"));
			/* if we are NOT acquiring, then this is a settling timer expiry event 
			and we need to re-enable acquisition of data */
			BKNI_EnterCriticalSection();
			hAstm->sClockCoupling.bAcquire = true;
			BKNI_LeaveCriticalSection();
		}

		/* reschedule for later */
		BDBG_MSG(("Scheduling clock coupling processing timer"));
		BKNI_EnterCriticalSection();
		rc = BASTMlib_P_StartTimer_isr(hAstm, hAstm->sClockCoupling.hTimer, 
			hAstm->sStatus.eClockCoupling == BASTMlib_ClockCoupling_eInputClock 
			? hAstm->sConfig.sClockCoupling.uiProcessingFrequency 
			: hAstm->sConfig.sClockCoupling.uiIdealProcessingFrequency, 
			&BASTMlib_P_ClockCoupling_TimerExpired, hAstm, 0);
		BKNI_LeaveCriticalSection();
		if (rc) goto error;
	}
	else
	{
		BDBG_MSG(("Clock coupling timer expired, but ASTM was not started or enabled"));
	}

	goto end;

error:

end:

	BDBG_LEAVE(BASTMlib_P_ClockCoupling_TimerExpired);
	return rc;
}

BERR_Code BASTMlib_P_ClockCoupling_Process(
	BASTMlib_Handle hAstm
)
{
	BERR_Code rc = BERR_SUCCESS;
	long lActual;
	long lExpected;
	long lDeviation;
	unsigned int uiDeviationThreshold;
	unsigned int uiDeviantCount;
	unsigned int uiDeviantCountThreshold;
	unsigned int uiIdealCount;
	unsigned int uiIdealCountThreshold;
	unsigned int uiSize;
	BASTMlib_ClockReference_Handle hReference;
	BASTMlib_ClockReference_Event sLastEvent;
	BASTMlib_ClockReference_Event * psLastEvent;

	BDBG_ENTER(BASTMlib_P_ClockCoupling_Process);

	BDBG_ASSERT(hAstm);

	hReference = hAstm->sClockCoupling.hReference;
	psLastEvent = NULL;
	uiDeviationThreshold = hReference->sConfig.uiDeviationThreshold;
	uiDeviantCountThreshold = hReference->sConfig.uiDeviantCountThreshold;
	uiIdealCountThreshold = hReference->sConfig.uiIdealCountThreshold;
	uiDeviantCount = 0;
	uiIdealCount = 0;

	BKNI_EnterCriticalSection();
	uiSize = hReference->sEventQueue.uiSize;
	BKNI_LeaveCriticalSection();
	
	while (uiSize)
	{
		BASTMlib_ClockReference_Event sEvent;

#if BASTMLIB_DEBUG_QUEUE
		BDBG_MSG(("Removing event from clock coupling queue at %u", hReference->sEventQueue.uiRead + 1));
#endif
		BKNI_EnterCriticalSection();
		hReference->sEventQueue.uiRead++;
		hReference->sEventQueue.uiRead %= hReference->sEventQueue.uiCapacity;
		sEvent = hReference->sEventQueue.asEvents[hReference->sEventQueue.uiRead];
		hReference->sEventQueue.uiSize--;
		BKNI_LeaveCriticalSection();

		if (psLastEvent)
		{
			lActual = (long)sEvent.uiClockReference - (long)psLastEvent->uiClockReference;
			lExpected = (long)sEvent.uiStc - (long)psLastEvent->uiStc;
			lDeviation = lActual - lExpected;

			/* if deviant */
			if (lDeviation > (long)uiDeviationThreshold || lDeviation < -(long)uiDeviationThreshold)
			{
				uiDeviantCount++;
				BDBG_MSG(("Clock coupling deviation failure %+ld", lDeviation));
			}
			else
			{
				uiIdealCount++;
				BDBG_MSG(("Clock coupling deviation pass %+ld", lDeviation));
			}
		}
		else
		{
			psLastEvent = &sLastEvent;
		}

		sLastEvent = sEvent;

		BKNI_EnterCriticalSection();
		uiSize = hReference->sEventQueue.uiSize;
		BKNI_LeaveCriticalSection();
	}

	/* no pcrs received during measurement period */
	if (uiDeviantCount + uiIdealCount == 0)
	{
        bool bDataPresent = true;

	    if (hAstm->sConfig.sClockCoupling.pfDetectDataPresent)
	    {
	        hAstm->sConfig.sClockCoupling.pfDetectDataPresent(
	            hAstm->sConfig.sClockCoupling.pvDataPresentContext,
	            &bDataPresent);

	    }

	    if (bDataPresent)
	    {
            BDBG_MSG(("No clock references received during measurement period"));
            BASTMlib_P_ClockCoupling_StateMachine_SendSignal(hAstm,
                BASTMlib_ClockCoupling_StateMachineSignal_eClockReferenceFailure);
            BASTMlib_P_Presentation_StcSourceStateMachine_SendSignal(hAstm,
                BASTMlib_Presentation_StateMachineSignal_eClockReferenceFailure);
	    }
	    else
	    {
            BDBG_MSG(("No clock reference data present during measurement period; waiting"));
	    }
	}
	else
	{
		/* if we are attached to input, check for deviants */
		if (hAstm->sStatus.eClockCoupling == BASTMlib_ClockCoupling_eInputClock)
		{
		    /* ignore up to threshold deviants */
		    if (uiDeviantCount > uiDeviantCountThreshold)
		    {
			    BASTMlib_P_ClockCoupling_StateMachine_SendSignal(hAstm, 
				    BASTMlib_ClockCoupling_StateMachineSignal_eClockReferenceFailure);
			    BASTMlib_P_Presentation_StcSourceStateMachine_SendSignal(hAstm, 
				    BASTMlib_Presentation_StateMachineSignal_eClockReferenceFailure);
		    }
			/* else, no change in state */
		}
		else  /* we are attached to xtal, check for ideals */
		{
			/* the following code generates an affinity for staying in the clock
			decoupled state, as any deviants detected while in the decoupled
			state will prevent us from returning to	a coupled state */
			/* ideals only get processed if there weren't any deviants */
			if (!uiDeviantCount && uiIdealCount > uiIdealCountThreshold) /* ignore up to threshold ideals */
			{
				BASTMlib_P_ClockCoupling_StateMachine_SendSignal(hAstm, 
					BASTMlib_ClockCoupling_StateMachineSignal_eClockReferencePass);
				BASTMlib_P_Presentation_StcSourceStateMachine_SendSignal(hAstm, 
					BASTMlib_Presentation_StateMachineSignal_eClockReferencePass);
			}
			/* else, no change in state */
		}
	}

	BDBG_LEAVE(BASTMlib_P_ClockCoupling_Process);
	return rc;
}

void BASTMlib_P_ClockCoupling_StateMachine_SendSignal(
	BASTMlib_Handle hAstm,
	BASTMlib_ClockCoupling_StateMachineSignal eSignal
)
{
	BASTMlib_ClockCoupling eNewState;
	
	BDBG_ENTER(BASTMlib_P_ClockCoupling_StateMachine_SendSignal);
	
	BDBG_ASSERT(hAstm);

#if BASTMLIB_DEBUG_SIGNALS
	BDBG_MSG(("Clock coupling signal received: %u", eSignal));
#endif

	/*
	Clock coupling

	signal (s)
	0 reset
	1 clock reference pass
	2 clock reference fail

	preference (p), output (y)
	0 input clock
	1 internal clock

	p	s	y
	0	0	0
	0	1	0
	0	2	1
	1	x	1
	*/
	switch (eSignal)
	{
		case BASTMlib_ClockCoupling_StateMachineSignal_eReset:
		case BASTMlib_ClockCoupling_StateMachineSignal_eClockReferencePass:
			eNewState = hAstm->sConfig.sClockCoupling.ePreferredClockCoupling; /* don't go above preference */
			break;
		case BASTMlib_ClockCoupling_StateMachineSignal_eClockReferenceFailure:
			eNewState = BASTMlib_ClockCoupling_eInternalClock;
			break;
		default:
			eNewState = hAstm->sStatus.eClockCoupling; /* copy old state */
			break;
	}

	if (hAstm->sStatus.eClockCoupling != eNewState)
	{
		hAstm->sStatus.eClockCoupling = eNewState;
		BASTMlib_P_ClockCoupling_StateChangeHandler(hAstm);
	}

	BDBG_LEAVE(BASTMlib_P_ClockCoupling_StateMachine_SendSignal);
}

BERR_Code BASTMlib_P_ClockCoupling_StateChangeHandler(
	BASTMlib_Handle hAstm
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BASTMlib_P_ClockCoupling_StateChangeHandler);

	BDBG_ASSERT(hAstm);

	/* state changed, so stop acquiring for a bit */
	BKNI_EnterCriticalSection();
	hAstm->sClockCoupling.bAcquire = false;
	BKNI_LeaveCriticalSection();

	/* tell external sw that the state changed */
	rc = BSYSlib_Callback_Invoke(&hAstm->sSettings.sClockCoupling.cbStateChange);
	if (rc) goto error;

	/* cancel processing timer and schedule settling timer -- if started */
	if (hAstm->bStarted)
	{
		BDBG_MSG(("Scheduling clock coupling settling timer"));
		BKNI_EnterCriticalSection();
		rc = BASTMlib_P_StartTimer_isr(hAstm, hAstm->sClockCoupling.hTimer, 
			hAstm->sConfig.sClockCoupling.uiSettlingTime, 
			&BASTMlib_P_ClockCoupling_TimerExpired, hAstm, 0);
		BKNI_LeaveCriticalSection();
		if (rc) goto error;
	}

	goto end;

error:

end:
	BDBG_LEAVE(BASTMlib_P_ClockCoupling_StateChangeHandler);
	return rc;
}

