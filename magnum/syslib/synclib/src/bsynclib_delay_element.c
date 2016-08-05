/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 **************************************************************************/
#include "bstd.h"
#include "bsynclib_delay_element.h"

BDBG_MODULE(synclib);

#ifdef BDBG_DEBUG_BUILD
const char * const BSYNClib_DelayElement_LifecycleEventNames[] = 
	{
		"none",
		"started",
		"stopped"
	};
const char * const BSYNClib_DelayElement_SynchronizationEventNames[] = 
	{
		"none",
		"synchronized",
		"ignored"
	};
#endif

void BSYNClib_DelayElement_Init(BSYNClib_DelayElement * psElement)
{
	BDBG_ENTER(BSYNClib_DelayElement_Init);
	BDBG_ASSERT(psElement);
	BKNI_Memset(psElement, 0, sizeof(BSYNClib_DelayElement));
	BSYNClib_Delay_Init(&psElement->sDelay);
	BSYNClib_DelayNotification_Init(&psElement->sNotification);
	BDBG_LEAVE(BSYNClib_DelayElement_Init);
}

void BSYNClib_DelayElement_Reset_isr(BSYNClib_DelayElement * psElement)
{
	BDBG_ENTER(BSYNClib_DelayElement_Reset_isr);
	BDBG_ASSERT(psElement);
	psElement->sData.bStarted = false;
	psElement->sData.bNonRealTime = false;
	BSYNClib_Delay_Reset_isr(&psElement->sDelay);
	BSYNClib_DelayNotification_Reset_isr(&psElement->sNotification);
	BDBG_LEAVE(BSYNClib_DelayElement_Reset_isr);
}

void BSYNClib_DelayElement_Snapshot_isr(BSYNClib_DelayElement * psElement)
{
	BDBG_ENTER(BSYNClib_DelayElement_Snapshot_isr);
	BDBG_ASSERT(psElement);
	psElement->sSnapshot = psElement->sData;
	BSYNClib_Delay_Snapshot_isr(&psElement->sDelay);
	BSYNClib_DelayNotification_Snapshot_isr(&psElement->sNotification);
	BDBG_LEAVE(BSYNClib_DelayElement_Snapshot_isr);
}

void BSYNClib_Delay_Init(BSYNClib_Delay * psDelay)
{
	BDBG_ENTER(BSYNClib_Delay_Init);
	BDBG_ASSERT(psDelay);
	BKNI_Memset(psDelay, 0, sizeof(BSYNClib_Delay));
	BDBG_LEAVE(BSYNClib_Delay_Init);
}

void BSYNClib_Delay_Reset_isr(BSYNClib_Delay * psDelay)
{
	BDBG_ENTER(BSYNClib_Delay_Reset_isr);
	BDBG_ASSERT(psDelay);
	psDelay->sData.bValid = false;
	psDelay->sData.uiCustom = 0;
	psDelay->sData.uiMeasured = 0;
	psDelay->sSnapshot.bValid = false;
	psDelay->sSnapshot.uiCustom = 0;
	psDelay->sSnapshot.uiMeasured = 0;
	psDelay->sResults.bAccepted = false;
	psDelay->sResults.bEstimated = false;
	psDelay->sResults.uiApplied = 0;
	psDelay->sResults.uiDesired = 0;
	BDBG_LEAVE(BSYNClib_Delay_Reset_isr);
}

void BSYNClib_Delay_Snapshot_isr(BSYNClib_Delay * psDelay)
{
	BDBG_ENTER(BSYNClib_Delay_Snapshot_isr);
	BDBG_ASSERT(psDelay);
	psDelay->sSnapshot = psDelay->sData;
	BDBG_LEAVE(BSYNClib_Delay_Snapshot_isr);
}

void BSYNClib_DelayNotification_Init(BSYNClib_DelayNotification * psNotification)
{
	BDBG_ENTER(BSYNClib_DelayNotification_Init);
	BDBG_ASSERT(psNotification);
	BKNI_Memset(psNotification, 0, sizeof(BSYNClib_DelayNotification));
	psNotification->sData.bEnabled = true;
	psNotification->sSnapshot.bEnabled = true;
	psNotification->sResults.bEnabled = true;
	BDBG_LEAVE(BSYNClib_DelayNotification_Init);
}

void BSYNClib_DelayNotification_Reset_isr(BSYNClib_DelayNotification * psNotification)
{
	BDBG_ENTER(BSYNClib_DelayNotification_Reset_isr);
	BDBG_ASSERT(psNotification);
	psNotification->sData.bEnabled = true; /* TODO: should be based on callback installed */
	psNotification->sData.bReceived = false;
	psNotification->sSnapshot.bEnabled = true;
	psNotification->sSnapshot.bReceived = false;
	psNotification->sResults.bEnabled = true;
	psNotification->sResults.bGenerateCallback = false;
	BDBG_LEAVE(BSYNClib_DelayNotification_Reset_isr);
}

void BSYNClib_DelayNotification_Snapshot_isr(BSYNClib_DelayNotification * psNotification)
{
	BDBG_ENTER(BSYNClib_DelayNotification_Snapshot_isr);
	BDBG_ASSERT(psNotification);
	psNotification->sSnapshot = psNotification->sData;
	BDBG_LEAVE(BSYNClib_DelayNotification_Snapshot_isr);
}

void BSYNClib_DelayElement_CheckLifecycle_isr(
	bool bStarted,
	const BSYNClib_DelayElement * psCurrent,
	BSYNClib_DelayElement_DiffResults * psResults
)
{
	BDBG_ENTER(BSYNClib_DelayElement_CheckLifecycle_isr);

	BDBG_ASSERT(psCurrent);
	BDBG_ASSERT(psResults);

	if (bStarted && !psCurrent->sData.bStarted)
	{
		psResults->bChanged = true;
		psResults->eLifecycleEvent = BSYNClib_DelayElement_LifecycleEvent_eStarted;
	}
	else if (!bStarted && psCurrent->sData.bStarted)
	{
		psResults->bChanged = true;
		psResults->eLifecycleEvent = BSYNClib_DelayElement_LifecycleEvent_eStopped;
	}
	else
	{
		psResults->eLifecycleEvent = BSYNClib_DelayElement_LifecycleEvent_eNone;
	}

	BDBG_LEAVE(BSYNClib_DelayElement_CheckLifecycle_isr);
}


void BSYNClib_DelayElement_Diff_isr(
	const BSYNClib_DelayElement * psDesired,
	const BSYNClib_DelayElement * psCurrent,
	BSYNClib_DelayElement_DiffResults * psResults
)
{
	BDBG_ENTER(BSYNClib_DelayElement_Diff_isr);

	BDBG_ASSERT(psDesired);
	BDBG_ASSERT(psCurrent);
	BDBG_ASSERT(psResults);

	/* check if synchronization for this element was just enabled */
	if (psDesired->sData.bSynchronize && !psCurrent->sData.bSynchronize)
	{
		psResults->bChanged = true;
		psResults->bGenerateDelayCallback = true;
		psResults->bGenerateNotificationCallback = true;
		psResults->eSynchronizationEvent = BSYNClib_DelayElement_SynchronizationEvent_eSynchronized;
	}
	else if (!psDesired->sData.bSynchronize && psCurrent->sData.bSynchronize)
	{
		/* or disabled */
		psResults->bChanged = true;
		psResults->bGenerateDelayCallback = false;
		psResults->bGenerateNotificationCallback = false;
		psResults->eSynchronizationEvent = BSYNClib_DelayElement_SynchronizationEvent_eIgnored;
	}
	else
	{
		psResults->eSynchronizationEvent = BSYNClib_DelayElement_SynchronizationEvent_eNone;
	}

	/* copy user custom delay */
	if (psCurrent->sDelay.sData.uiCustom != psDesired->sDelay.sData.uiCustom)
	{
		psResults->bChanged = true;
	}

	/* copy NRT flag */
	if (psCurrent->sData.bNonRealTime != psDesired->sData.bNonRealTime)
	{
		psResults->bChanged = true;
	}

	/* if delay notification is being enabled (or we are starting with 
	delay notification enabled), generate callback for threshold value */
	if ((!psCurrent->sNotification.sData.bEnabled && psDesired->sNotification.sData.bEnabled)
		|| (psDesired->sNotification.sData.bEnabled && psResults->eLifecycleEvent == BSYNClib_DelayElement_LifecycleEvent_eStarted))
	{
		psResults->bGenerateNotificationCallback = true;
		psResults->bChanged = true;
	}

	/* if delay notification is enabled, check that we received one, otherwise, copy delay level and validate */
	if (psCurrent->sNotification.sData.bEnabled)
	{
		/* received a delay notification? */
		if (psDesired->sNotification.sData.bReceived)
		{
			psResults->bDelayReceived = true;
			psResults->bChanged = true;
		}
	}
	else
	{
		if (psCurrent->sDelay.sData.uiMeasured != psDesired->sDelay.sData.uiMeasured)
		{
			psResults->bChanged = true;
		}
	}

	if (psCurrent->sDelay.sData.uiCapacity != psDesired->sDelay.sData.uiCapacity)
	{
		psResults->bChanged = true;
	}

	BDBG_LEAVE(BSYNClib_DelayElement_Diff_isr);
}

void BSYNClib_DelayElement_Patch_isr(
	const BSYNClib_DelayElement * psDesired,
	BSYNClib_DelayElement * psCurrent,
	BSYNClib_DelayElement_DiffResults * psResults
)
{
	BDBG_ENTER(BSYNClib_DelayElement_Patch_isr);

	BDBG_ASSERT(psCurrent);
	BDBG_ASSERT(psDesired);
	BDBG_ASSERT(psResults);

	if (psResults->bChanged)
	{
		psCurrent->sData = psDesired->sData;
		psCurrent->sDelay.sData = psDesired->sDelay.sData;
		psCurrent->sNotification.sData = psDesired->sNotification.sData;

		if (psResults->bDelayReceived)
		{
			/* validate */
			psCurrent->sDelay.sData.bValid = true;
			/* accept only after timer expires */
			psCurrent->sDelay.sResults.bAccepted = false;
		}

		/* Accept and validate the delay if notification was disabled */
		if (!psCurrent->sNotification.sData.bEnabled)
		{
			psCurrent->sDelay.sData.bValid = true;
			/* TODO: do elsewhere? */
			/* TODO: assume avg delay? */
		}

		if (psResults->bGenerateDelayCallback)
		{
			psCurrent->sDelay.sResults.bGenerateCallback = true;
		}

		if (psResults->bGenerateNotificationCallback)
		{
			psCurrent->sNotification.sResults.bGenerateCallback = true;
		}
	}

	BDBG_LEAVE(BSYNClib_DelayElement_Patch_isr);
}
