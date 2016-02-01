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
#include "bastmlib_priv.h"
#include "bastmlib_clock_reference.h"
#include "bastmlib_clock_reference_priv.h"

BDBG_MODULE(astmlib);

void BASTMlib_ClockReference_Create(BASTMlib_ClockReference_Handle * phReference)
{
	BASTMlib_ClockReference_Handle hReference;

	BDBG_ENTER(BASTMlib_ClockReference_Create);

	hReference = (BASTMlib_ClockReference_Handle)BKNI_Malloc(sizeof(struct BASTMlib_ClockReference_Impl));

	BDBG_ASSERT(hReference);
	BKNI_Memset(hReference, 0, sizeof(struct BASTMlib_ClockReference_Impl));
	BASTMlib_ClockReference_GetDefaultConfig(&hReference->sConfig);

	BDBG_MSG(("clock reference initial configuration:"));
	BDBG_MSG(("    min time between events: %u ms", hReference->sConfig.uiMinimumTimeBetweenEvents));
	BDBG_MSG(("    clock reference domain: %u Hz", hReference->sConfig.eClockReferenceDomain));
	BDBG_MSG(("    deviation threshold: %u ticks", hReference->sConfig.uiDeviationThreshold));
	BDBG_MSG(("    deviant count threshold: %u events", hReference->sConfig.uiDeviantCountThreshold));
	BDBG_MSG(("    ideal count threshold: %u events", hReference->sConfig.uiIdealCountThreshold));

	*phReference = hReference;

	BDBG_LEAVE(BASTMlib_ClockReference_Create);
}

void BASTMlib_ClockReference_Destroy(BASTMlib_ClockReference_Handle hReference)
{
	BDBG_ENTER(BASTMlib_ClockReference_Destroy);
	BDBG_ASSERT(hReference);

	if (hReference->sEventQueue.asEvents)
	{
		BKNI_Free(hReference->sEventQueue.asEvents);
	}
	
	BKNI_Free(hReference);
	BDBG_LEAVE(BASTMlib_ClockReference_Destroy);
}

void BASTMlib_ClockReference_GetDefaultConfig(BASTMlib_ClockReference_Config * psConfig)
{
	BDBG_ENTER(BASTMlib_ClockReference_P_GetDefaultConfig);

	BDBG_ASSERT(psConfig);

	psConfig->eClockReferenceDomain = BASTMlib_ClockRate_e45Khz;
	psConfig->uiMinimumTimeBetweenEvents = BASTMLIB_CLOCK_REFERENCE_P_DEFAULT_MIN_TIME_BETWEEN_EVENTS;
	psConfig->uiDeviationThreshold = BASTMLIB_CLOCK_REFERENCE_P_DEFAULT_DEVIATION_THRESHOLD * BASTMlib_ClockRate_e45Khz / 1000;
	psConfig->uiDeviantCountThreshold = BASTMLIB_CLOCK_REFERENCE_P_DEFAULT_DEVIANT_COUNT_THRESHOLD;
	psConfig->uiIdealCountThreshold = BASTMLIB_CLOCK_REFERENCE_P_DEFAULT_IDEAL_COUNT_THRESHOLD;

	BDBG_LEAVE(BASTMlib_ClockReference_P_GetDefaultConfig);
}

void BASTMlib_ClockReference_Flush_isr(
    BASTMlib_ClockReference_Handle hReference
)
{
    BDBG_ENTER(BASTMlib_ClockReference_Flush_isr);

    BDBG_ASSERT(hReference);

    hReference->sEventQueue.uiWrite = hReference->sEventQueue.uiCapacity / 2;
    hReference->sEventQueue.uiRead = hReference->sEventQueue.uiWrite - 1;
    hReference->sEventQueue.uiSize = 0;

    BDBG_LEAVE(BASTMlib_ClockReference_Flush_isr);
}

void BASTMlib_ClockReference_Reset_isr(
	BASTMlib_ClockReference_Handle hReference
)
{
	BDBG_ENTER(BASTMlib_ClockReference_Reset_isr);

	BDBG_ASSERT(hReference);

	hReference->lAverageDeviation = 0;
	hReference->uiDeviantCount = 0;
	hReference->uiIdealCount = 0;

	BDBG_LEAVE(BASTMlib_ClockReference_Reset_isr);
}

void BASTMlib_ClockReference_GetConfig(
	const BASTMlib_ClockReference_Handle hReference, 
	BASTMlib_ClockReference_Config * psConfig
)
{
	BDBG_ENTER(BASTMlib_ClockReference_GetConfig);

	BDBG_ASSERT(hReference);
	BDBG_ASSERT(psConfig);

	*psConfig = hReference->sConfig;
	
	BDBG_LEAVE(BASTMlib_ClockReference_GetConfig);
}

void BASTMlib_ClockReference_SetConfig(
	BASTMlib_ClockReference_Handle hReference, 
	const BASTMlib_ClockReference_Config * psConfig
)
{
	BASTMlib_Handle hAstm;
	
	BDBG_ENTER(BASTMlib_ClockReference_SetConfig);

	BDBG_ASSERT(hReference);
	BDBG_ASSERT(psConfig);

	hReference->sConfig = *psConfig;

	BDBG_MSG(("clock reference reconfigured:"));
	BDBG_MSG(("  min time between events: %u ms", hReference->sConfig.uiMinimumTimeBetweenEvents));
	BDBG_MSG(("  clock reference domain: %u Hz", hReference->sConfig.eClockReferenceDomain));
	BDBG_MSG(("  deviation threshold: %u ticks", hReference->sConfig.uiDeviationThreshold));
	BDBG_MSG(("  deviant count threshold: %u events", hReference->sConfig.uiDeviantCountThreshold));
	BDBG_MSG(("  ideal count threshold: %u events", hReference->sConfig.uiIdealCountThreshold));

	hAstm = hReference->hAstm;

	/* don't resize if already started, wait till next restart */
	if (!hAstm->bStarted)
	{
		BASTMlib_ClockReference_ResizeEventQueue(hReference);
	}

	BDBG_LEAVE(BASTMlib_ClockReference_SetConfig);
}

void BASTMlib_ClockReference_EventHandler_isr(
	BASTMlib_ClockReference_Handle hReference,
	const BASTMlib_ClockReference_Event * psEvent
	
)
{
	BASTMlib_Handle hAstm;

	BDBG_ENTER(BASTMlib_ClockReference_EventHandler_isr);

	BDBG_ASSERT(hReference);
	BDBG_ASSERT(psEvent);

	hAstm = hReference->hAstm;
	BDBG_ASSERT(hAstm);

	if (hAstm->bStarted)
	{
		if (hAstm->sClockCoupling.bAcquire)
		{
			if (hReference->sEventQueue.uiSize < hReference->sEventQueue.uiCapacity)
			{
#if BASTMLIB_DEBUG_QUEUE
				BDBG_MSG(("Adding event to clock reference queue at %u", hReference->sEventQueue.uiWrite));
#endif
				hReference->sEventQueue.asEvents[hReference->sEventQueue.uiWrite] = *psEvent;
				hReference->sEventQueue.uiWrite++;
				hReference->sEventQueue.uiWrite %= hReference->sEventQueue.uiCapacity;
				hReference->sEventQueue.uiSize++;
			}
			else
			{
				BDBG_MSG(("Clock reference event queue full.  Discarding events."));
			}
		}
		else
		{
			BDBG_MSG(("Clock reference event received while not acquiring. Ignored"));
		}
	}
	else
	{
		BDBG_MSG(("Clock reference event received while stopped. Ignored"));
	}

	BDBG_LEAVE(BASTMlib_ClockReference_EventHandler_isr);
}

void BASTMlib_ClockReference_ResizeEventQueue(
	BASTMlib_ClockReference_Handle hReference
)
{
	unsigned int uiMinimumTimeBetweenEvents;
	unsigned int uiMaximumAcquisitionTime;
	unsigned int uiCapacity;

	BDBG_ASSERT(hReference);
	
	BDBG_ENTER(BASTMlib_ClockReference_ResizeEventQueue);

	uiMinimumTimeBetweenEvents = hReference->sConfig.uiMinimumTimeBetweenEvents;
	uiMaximumAcquisitionTime = hReference->uiMaximumAcquisitionTime;

	if (uiMinimumTimeBetweenEvents && uiMaximumAcquisitionTime)
	{
		uiCapacity = uiMaximumAcquisitionTime / uiMinimumTimeBetweenEvents + 1;
		uiCapacity += uiCapacity / 10; /* 10% bigger than required */
	}
	else
	{
		uiCapacity = BASTMLIB_CLOCK_REFERENCE_P_DEFAULT_EVENT_QUEUE_CAPACITY;
	}

	if (hReference->sEventQueue.uiCapacity != uiCapacity)
	{
		if (hReference->sEventQueue.asEvents)
		{
			BKNI_Free(hReference->sEventQueue.asEvents);
		}

		hReference->sEventQueue.asEvents = (BASTMlib_ClockReference_Event *)BKNI_Malloc(sizeof(BASTMlib_ClockReference_Event) * uiCapacity);
		BDBG_ASSERT(hReference->sEventQueue.asEvents);
		BKNI_Memset(hReference->sEventQueue.asEvents, 0, sizeof(BASTMlib_ClockReference_Event) * uiCapacity);

		hReference->sEventQueue.uiCapacity = uiCapacity;

		BKNI_EnterCriticalSection();
	    BASTMlib_ClockReference_Flush_isr(hReference);
		BKNI_LeaveCriticalSection();
	}

	BDBG_LEAVE(BASTMlib_ClockReference_ResizeEventQueue);
}

