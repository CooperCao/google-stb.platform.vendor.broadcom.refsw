/***************************************************************************
*     Copyright (c) 2004-2012, Broadcom Corporation
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
#include "bastmlib_presentation_task.h"
#include "bastmlib_presenter_priv.h" /* TODO: remove dependency on presenter internals */

BDBG_MODULE(astmlib);

#define BASTMLIB_DEBUG_QUEUE 0
#define BASTMLIB_DEBUG_SIGNALS 0

void BASTMlib_P_Presentation_GetDefaultConfig(
	BASTMlib_Config * psConfig
)
{
	BDBG_ENTER(BASTMlib_P_Presentation_GetDefaultConfig);

	BDBG_ASSERT(psConfig);

	psConfig->sPresentation.ePreferredStcSource = BASTMlib_StcSource_eClockReference;
	psConfig->sPresentation.hPreferredPresenter = NULL;
	psConfig->sPresentation.uiInitialAcquisitionTime = BASTMLIB_PRESENTATION_DEFAULT_INITIAL_ACQUISITION_TIME;
	psConfig->sPresentation.uiProcessingFrequency = BASTMLIB_PRESENTATION_DEFAULT_PROCESSING_FREQUENCY;
	psConfig->sPresentation.uiSettlingTime = BASTMLIB_PRESENTATION_DEFAULT_SETTLING_TIME;
	psConfig->sPresentation.uiTsmDisabledWatchdogTimeout = BASTMLIB_PRESENTATION_DEFAULT_TSM_DISABLED_WATCHDOG_TIMEOUT;
	psConfig->sPresentation.ePreferredPresentationRateControl = BASTMlib_PresentationRateControl_eTimeStamp;

	BDBG_LEAVE(BASTMlib_P_Presentation_GetDefaultConfig);
}

void BASTMlib_P_Presentation_GetMaxAcquisitionTime(
	BASTMlib_Handle hAstm,
	unsigned int * puiMaxAcquisitionTime
)
{
	BDBG_ENTER(BASTMlib_P_Presentation_GetMaxAcquisitionTime);

	BDBG_ASSERT(hAstm);
	BDBG_ASSERT(puiMaxAcquisitionTime);

	*puiMaxAcquisitionTime = hAstm->sConfig.sPresentation.uiInitialAcquisitionTime 
		> hAstm->sConfig.sPresentation.uiProcessingFrequency 
		? hAstm->sConfig.sPresentation.uiInitialAcquisitionTime 
		: hAstm->sConfig.sPresentation.uiProcessingFrequency;

	BDBG_LEAVE(BASTMlib_P_Presentation_GetMaxAcquisitionTime);
}

BERR_Code BASTMlib_P_Presentation_WatchdogTimerExpired(
	void * pvParm1, /* first user context parameter [in] */ 
	int iParm2, /* second user context parameter [in] */ 
	BSYSlib_Timer_Handle hTimer /* the handle of the timer that expired [in] */ 
)
{
	BERR_Code rc = BERR_SUCCESS;
	BASTMlib_Handle hAstm = pvParm1;

	BDBG_ENTER(BASTMlib_P_Presentation_WatchdogTimerExpired);

	BDBG_ASSERT(hAstm);

	BSTD_UNUSED(iParm2);
	BSTD_UNUSED(hTimer);

	if (hAstm->sConfig.bEnabled && hAstm->bStarted)
	{
		/* watchdog only happens when we have been in output master mode for a long time. 
		we will try to go back to the preferred presentation master mode */
		BDBG_WRN(("Presentation TSM-disabled watchdog timer expired"));
		BASTMlib_P_Presentation_RateControlStateMachine_SendSignal(hAstm, 
			BASTMlib_Presentation_StateMachineSignal_ePresenterPass);
	}
	else
	{
		BDBG_MSG(("Presentation TSM-disabled watchdog timer expired, but ASTM was not started or enabled"));
	}

	BDBG_LEAVE(BASTMlib_P_Presentation_WatchdogTimerExpired);
	return rc;
}

BERR_Code BASTMlib_P_Presentation_TimerExpired(
	void * pvParm1, /* first user context parameter [in] */ 
	int iParm2, /* second user context parameter [in] */ 
	BSYSlib_Timer_Handle hTimer /* the handle of the timer that expired [in] */ 
)
{
	BERR_Code rc = BERR_SUCCESS;
	BASTMlib_Handle hAstm = pvParm1;

	BDBG_ENTER(BASTMlib_P_Presentation_TimerExpired);

	BDBG_ASSERT(hAstm);

	BSTD_UNUSED(iParm2);
	BSTD_UNUSED(hTimer);

	if (hAstm->sConfig.bEnabled && hAstm->bStarted)
	{
		if (hAstm->sPresentation.bAcquire)
		{
			BDBG_MSG(("Presentation processing timer expired"));
			/* if we are acquiring, then this is a processing timer expiry event */
			rc = BASTMlib_P_Presentation_Process(hAstm);
			if (rc) goto error;
		}
		else
		{
			BDBG_MSG(("Presentation settling timer expired"));
			/* if we are NOT acquiring, then this is a settling timer expiry event 
			and we need to re-enable acquisition of data */
			BKNI_EnterCriticalSection();
			hAstm->sPresentation.bAcquire = true;
			BKNI_LeaveCriticalSection();
		}

		/* reschedule for later */
		BDBG_MSG(("Scheduling presentation processing timer"));
		BKNI_EnterCriticalSection();
		rc = BASTMlib_P_StartTimer_isr(hAstm, hAstm->sPresentation.hTimer, 
			hAstm->sConfig.sPresentation.uiProcessingFrequency, 
			&BASTMlib_P_Presentation_TimerExpired, hAstm, 0);
		BKNI_LeaveCriticalSection();
		if (rc) goto error;
	}
	else
	{
		BDBG_MSG(("Presentation timer expired, but ASTM was not started or enabled"));
	}

	goto end;

error:

end:
	BDBG_LEAVE(BASTMlib_P_Presentation_TimerExpired);
	return rc;
}

BERR_Code BASTMlib_P_Presentation_Process(
	BASTMlib_Handle hAstm
)
{
	BERR_Code rc = BERR_SUCCESS;
	uint32_t uiPassEventCount;
	uint32_t uiFailEventCount;
	uint32_t uiPassEventThreshold;
	uint32_t uiFailEventThreshold;
	uint32_t uiFailingPresenterCount;
	uint32_t uiPassingPresenterCount;
	uint32_t uiSize;
	BSYSlib_List_IteratorHandle hIterator;
	BASTMlib_Presenter_Handle hPresenter;

	BDBG_ENTER(BASTMlib_P_Presentation_Process);

	BDBG_ASSERT(hAstm);

	uiFailingPresenterCount = 0;
	uiPassingPresenterCount = 0;

	hIterator = BSYSlib_List_AcquireIterator(hAstm->sPresentation.hPresenters);
	
	while (BSYSlib_List_HasNext(hIterator))
	{
		hPresenter = (BASTMlib_Presenter_Handle)BSYSlib_List_Next(hIterator);

		uiPassEventCount = 0;
		uiPassEventThreshold = hPresenter->sConfig.uiPassEventCountThreshold;

		uiFailEventCount = 0;
		uiFailEventThreshold = hPresenter->sConfig.uiFailEventCountThreshold;

		BKNI_EnterCriticalSection();
		uiSize = hPresenter->sEventQueue.uiSize;
		BKNI_LeaveCriticalSection();
		
		while (uiSize)
		{
			BASTMlib_Presenter_Event sEvent;

#if BASTMLIB_DEBUG_QUEUE
			BDBG_MSG(("Removing event from presenter %p queue at %u", hPresenter, hPresenter->sEventQueue.uiRead + 1));
#endif
			BKNI_EnterCriticalSection();
			hPresenter->sEventQueue.uiRead++;
			hPresenter->sEventQueue.uiRead %= hPresenter->sEventQueue.uiCapacity;
			sEvent = hPresenter->sEventQueue.asEvents[hPresenter->sEventQueue.uiRead];
			hPresenter->sEventQueue.uiSize--;
			BKNI_LeaveCriticalSection();

			BDBG_MSG(("Presenter %s TSM %s PTS %#x STC %#x delta %+ld", hPresenter->pcName, sEvent.bPass ? "pass" : "failure", sEvent.uiPts, sEvent.uiStc, (long)sEvent.uiPts - (long)sEvent.uiStc));
			
			if (sEvent.bPass)
			{
				uiPassEventCount++;
			}
			else
			{
				/* don't count the current STC master presenter as failing, since it will seed the STC */
				if (!(hAstm->sStatus.eStcSource == BASTMlib_StcSource_ePresenter && hAstm->sStatus.hStcMaster == hPresenter))
				{
					uiFailEventCount++;
				}
			}
			
			BKNI_EnterCriticalSection();
			uiSize = hPresenter->sEventQueue.uiSize;
			BKNI_LeaveCriticalSection();
		}

		if (uiPassEventCount > uiPassEventThreshold)
		{
			uiPassingPresenterCount++;
		}

		if (uiFailEventCount > uiFailEventThreshold)
		{
			uiFailingPresenterCount++;
		}
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	BDBG_MSG(("Passing presenters: %u", uiPassingPresenterCount));
	BDBG_MSG(("Failing presenters: %u", uiFailingPresenterCount));

	if (uiFailingPresenterCount > 0)
	{
		switch (hAstm->sStatus.eStcSource)
		{
			case BASTMlib_StcSource_eClockReference:
				/* don't signal presentation master presenter failure if not all presenters fail */
				if (uiFailingPresenterCount != hAstm->sPresentation.uiPresenterCount)
				{
					BASTMlib_P_Presentation_RateControlStateMachine_SendSignal(hAstm, 
						BASTMlib_Presentation_StateMachineSignal_ePresenterFailure);
				}
				else /* instead signal stc master presenter failure */
				{
					BASTMlib_P_Presentation_StcSourceStateMachine_SendSignal(hAstm, 
						BASTMlib_Presentation_StateMachineSignal_ePresenterFailure);
				}
				break;
			case BASTMlib_StcSource_ePresenter:
				BASTMlib_P_Presentation_RateControlStateMachine_SendSignal(hAstm, 
					BASTMlib_Presentation_StateMachineSignal_ePresenterFailure);
				break;
			default:
				break;
		}
	}
	else if (uiPassingPresenterCount == hAstm->sPresentation.uiPresenterCount)
	{
		BASTMlib_P_Presentation_RateControlStateMachine_SendSignal(hAstm,
			BASTMlib_Presentation_StateMachineSignal_ePresenterPass);
	}

	BDBG_LEAVE(BASTMlib_P_Presentation_Process);
	return rc;
}

void BASTMlib_P_Presentation_RateControlStateMachine_SendSignal(
	BASTMlib_Handle hAstm,
	BASTMlib_Presentation_StateMachineSignal eSignal
)
{
	BASTMlib_PresentationRateControl eNewState;

	BDBG_ENTER(BASTMlib_P_Presentation_RateControlStateMachine_SendSignal);
	
	BDBG_ASSERT(hAstm);

#if BASTMLIB_DEBUG_SIGNALS
	BDBG_MSG(("Presentation rate control signal received: %u", eSignal));
#endif

	/*
	Presentation rate control

	signal (s)
	0 reset
	1 presenter pass
	2 presenter fail
	3 clock reference pass
	4 clock reference fail

	preference (p), before (t-1), now (t)
	0 time stamp
	1 output clock

	p	s	t-1	t
	0	0	x	0
	0	1	x	0
	0	2	x	1
	0	3	0	0
	0	3	1	1
	0	4	0	0
	0	4	1	1
	1	x	x	1
	*/
	switch (eSignal)
	{
		case BASTMlib_Presentation_StateMachineSignal_eReset:
		case BASTMlib_Presentation_StateMachineSignal_ePresenterPass:
			eNewState = hAstm->sConfig.sPresentation.ePreferredPresentationRateControl; /* never go above preference */
			break;
		case BASTMlib_Presentation_StateMachineSignal_ePresenterFailure:
			eNewState = BASTMlib_PresentationRateControl_eOutputClock;
			break;
		default:
			eNewState = hAstm->sStatus.ePresentationRateControl; /* copy old state */
			break;
	}

	if (eNewState != hAstm->sStatus.ePresentationRateControl)
	{
		hAstm->sStatus.ePresentationRateControl = eNewState;
		BASTMlib_P_Presentation_StateChangeHandler(hAstm, true);
	}
	
	BDBG_LEAVE(BASTMlib_P_Presentation_RateControlStateMachine_SendSignal);
}

void BASTMlib_P_Presentation_StcSourceStateMachine_SendSignal(
	BASTMlib_Handle hAstm,
	BASTMlib_Presentation_StateMachineSignal eSignal
)
{
	BASTMlib_StcSource eNewState;

	BDBG_ENTER(BASTMlib_P_Presentation_StcSourceStateMachine_SendSignal);
	
	BDBG_ASSERT(hAstm);

#if BASTMLIB_DEBUG_SIGNALS
	BDBG_MSG(("STC source signal received: %u", eSignal));
#endif

	/*
	STC source

	signal (s)
	0 reset
	1 presenter pass
	2 presenter fail
	3 clock reference pass
	4 clock reference fail

	preference (p), before (t-1), now (t)
	0 clock reference
	1 presenter

	p	s	t-1	t
	0	0	x	0
	0	1	0	0
	0	1	1	1
	0	2	x	1
	0	3	x	0
	0	4	x	1
	1	x	x	1
	*/
	switch (eSignal)
	{
		case BASTMlib_Presentation_StateMachineSignal_eReset:
		case BASTMlib_Presentation_StateMachineSignal_eClockReferencePass:
			eNewState = hAstm->sConfig.sPresentation.ePreferredStcSource; /* never go above preference */
			break;
		case BASTMlib_Presentation_StateMachineSignal_ePresenterFailure: /* means all presenters are failing */
		case BASTMlib_Presentation_StateMachineSignal_eClockReferenceFailure:
			eNewState = BASTMlib_StcSource_ePresenter;
			break;
		default:
			eNewState = hAstm->sStatus.eStcSource; /* copy old state */
			break;
	}

	if (eNewState != hAstm->sStatus.eStcSource)
	{
		hAstm->sStatus.eStcSource = eNewState;
		if (eNewState == BASTMlib_StcSource_ePresenter)
		{
			hAstm->sStatus.hStcMaster = hAstm->sConfig.sPresentation.hPreferredPresenter;
		}
		else
		{
			hAstm->sStatus.hStcMaster = NULL;
		}

		BASTMlib_P_Presentation_StateChangeHandler(hAstm, false);
	}
	
	BDBG_LEAVE(BASTMlib_P_Presentation_StcSourceStateMachine_SendSignal);
}

BERR_Code BASTMlib_P_Presentation_StateChangeHandler(
	BASTMlib_Handle hAstm,
	bool bPresentationMasterChanged
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BASTMlib_P_Presentation_StateChangeHandler);

	BDBG_ASSERT(hAstm);

	/* state changed, so stop acquiring for a bit */
	BKNI_EnterCriticalSection();
	hAstm->sPresentation.bAcquire = false;
	BKNI_LeaveCriticalSection();

	if (bPresentationMasterChanged)
	{
		/* Presentation mastership changed, do watchdog processing */
		switch (hAstm->sStatus.ePresentationRateControl)
		{
			case BASTMlib_PresentationRateControl_eTimeStamp:
				/* cancel any pending watchdog timer */
				BKNI_EnterCriticalSection();
				if (hAstm->sSettings.cbTimer.pfCancel_isr)
				{
					rc = hAstm->sSettings.cbTimer.pfCancel_isr(hAstm->sSettings.cbTimer.pvParm1,
						hAstm->sSettings.cbTimer.iParm2, hAstm->sPresentation.hWatchdogTimer);
				}
				BKNI_LeaveCriticalSection();
				if (rc) goto error;
				break;
			case BASTMlib_PresentationRateControl_eOutputClock:
				/* zero-length watchdog => disabled, do not schedule */
				if (hAstm->sConfig.sPresentation.uiTsmDisabledWatchdogTimeout)
				{
					/* schedule watchdog timer */
					BDBG_MSG(("Scheduling TSM-disabled watchdog timer"));
					BKNI_EnterCriticalSection();
					rc = BASTMlib_P_StartTimer_isr(hAstm, hAstm->sPresentation.hWatchdogTimer, 
						hAstm->sConfig.sPresentation.uiTsmDisabledWatchdogTimeout, 
						&BASTMlib_P_Presentation_WatchdogTimerExpired, hAstm, 0);
					BKNI_LeaveCriticalSection();
					if (rc) goto error;
				}
				break;
			default:
				break;
		}
	}

	/* tell external sw that the state changed */
	rc = BSYSlib_Callback_Invoke(&hAstm->sSettings.sPresentation.cbStateChange);
	if (rc) goto error;

	/* cancel processing timer and schedule settling timer -- if started */
	if (hAstm->bStarted)
	{
		BDBG_MSG(("Scheduling presentation settling timer"));
		BKNI_EnterCriticalSection();
		rc = BASTMlib_P_StartTimer_isr(hAstm, hAstm->sPresentation.hTimer, 
			hAstm->sConfig.sPresentation.uiSettlingTime, 
			&BASTMlib_P_Presentation_TimerExpired, hAstm, 0);
		BKNI_LeaveCriticalSection();
		if (rc) goto error;
	}

	goto end;

error:

end:
	BDBG_LEAVE(BASTMlib_P_Presentation_StateChangeHandler);
	return rc;
}

