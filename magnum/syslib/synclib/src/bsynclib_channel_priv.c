/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bkni.h"
#include "bsyslib.h"
#include "bsynclib.h"
#include "bsynclib_priv.h"
#include "bsynclib_channel_priv.h"
#include "bsynclib_algo.h"
#include "bsynclib_mute_control.h"

BDBG_MODULE(synclib);

/*
Summary:
Opens a SYNC lib channel
Description:
 */
BERR_Code BSYNClib_Channel_P_Create(
	const BSYNClib_Settings * psLibSettings,
	const BSYNClib_Channel_Settings * psSettings,
	BSYNClib_Channel_Handle * phChn /* [out] */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_Channel_Handle hChn = NULL;
	BSYNClib_StateMachine_Settings sMachineSettings;
	BSYNClib_RateMismatchDetector_Settings sRmdSettings;

	BDBG_ENTER(BSYNClib_Channel_P_Create);

	BDBG_ASSERT(phChn);

	hChn = (BSYNClib_Channel_Handle)BKNI_Malloc(sizeof(struct BSYNClib_Channel_Impl));
	if (!hChn)
	{
		rc = BERR_OUT_OF_SYSTEM_MEMORY;
		goto error;
	}

	BDBG_MSG(("create channel: %p", (void *)hChn));

	BKNI_Memset(hChn, 0, sizeof(struct BSYNClib_Channel_Impl));

	if (psSettings)
	{
		hChn->sSettings = *psSettings;
	}

	BSYNClib_Channel_P_GetDefaultConfig(&hChn->sConfig);

	BSYNClib_Channel_P_GetDefaultStatus(&hChn->sStatus);

	/* open state machine */
	BSYNClib_StateMachine_GetDefaultSettings(&sMachineSettings);
	rc = BSYNClib_StateMachine_Open(&sMachineSettings, &hChn->hMachine);
	if (rc) goto error;

	/* start state machine */
	BDBG_MSG(("chn %p: start", (void *)hChn));
	rc = BSYNClib_StateMachine_SendSignal(hChn->hMachine, BSYNClib_StateMachine_Signal_eStart);
	if (rc) goto error;

	/* allocate timers */
	if (hChn->sSettings.cbTimer.pfCreate)
	{
		void *pvParm1 = hChn->sSettings.cbTimer.pvParm1;
		int iParm2 = hChn->sSettings.cbTimer.iParm2;
		unsigned int i = 0;

		/* create timer resource pool */
		hChn->psTimers = BSYNClib_ResourcePool_Create();

		for (i = 0; i < BSYNCLIB_TIMER_COUNT; i++)
		{
			BSYNClib_Timer * psTimer;

			psTimer = (BSYNClib_Timer *)BKNI_Malloc(sizeof(BSYNClib_Timer));
			if (!psTimer) goto error;

			BSYNClib_Timer_Init(psTimer);

			BDBG_MSG(("chn %p: Creating timer %p(%d)", (void*)hChn, (void*)psTimer, i));
			rc = hChn->sSettings.cbTimer.pfCreate(pvParm1, iParm2, &psTimer->hTimer);
			if (rc)
			{
				BKNI_Free(psTimer);
				goto error;
			}

			BSYNClib_ResourcePool_Add(hChn->psTimers, psTimer);
		}

		/* Timer for task context switch */
		hChn->psTaskTimer = (BSYNClib_Timer *)BKNI_Malloc(sizeof(BSYNClib_Timer));
		if (!hChn->psTaskTimer) goto error;

		BSYNClib_Timer_Init(hChn->psTaskTimer);

		BDBG_MSG(("chn %p: Creating task context switch timer %p", (void*)hChn, (void*)hChn->psTaskTimer));
		rc = hChn->sSettings.cbTimer.pfCreate(pvParm1, iParm2, &hChn->psTaskTimer->hTimer);
		if (rc) goto error;

		/* Timer for mute control task context switch */
		hChn->psMuteControlTaskTimer = (BSYNClib_Timer *)BKNI_Malloc(sizeof(BSYNClib_Timer));
		if (!hChn->psMuteControlTaskTimer) goto error;

		BSYNClib_Timer_Init(hChn->psMuteControlTaskTimer);

		BDBG_MSG(("chn %p: Creating mute control task context switch timer %p", (void*)hChn, (void*)hChn->psMuteControlTaskTimer));
		rc = hChn->sSettings.cbTimer.pfCreate(pvParm1, iParm2, &hChn->psMuteControlTaskTimer->hTimer);
		if (rc) goto error;
	}

	/* open rate mismatch detector */
	BSYNClib_RateMismatchDetector_GetDefaultSettings(&sRmdSettings);
	sRmdSettings.sMismatch.uiTimeout = psLibSettings->sVideo.sRateMismatchDetection.uiTimeout;
	sRmdSettings.sMismatch.uiAcceptableMtbcLower = psLibSettings->sVideo.sRateMismatchDetection.uiAcceptableMtbcLower;
	sRmdSettings.sMismatch.uiAcceptableMtbcUpper = psLibSettings->sVideo.sRateMismatchDetection.uiAcceptableMtbcUpper;
	sRmdSettings.sMismatch.uiAcceptableTtlc = psLibSettings->sVideo.sRateMismatchDetection.uiAcceptableTtlc;
	sRmdSettings.sRematch.uiTimeout = psLibSettings->sVideo.sRateMismatchDetection.uiTimeout;
	sRmdSettings.sRematch.uiAcceptableTtlc = psLibSettings->sVideo.sRateRematchDetection.uiAcceptableTtlc;
	rc = BSYNClib_RateMismatchDetector_Open(hChn, &sRmdSettings, &hChn->hDetector);
	if (rc) goto error;

	hChn->sVideo.hSources = BSYSlib_List_Create();
	hChn->sVideo.hSinks = BSYSlib_List_Create();
	hChn->sAudio.hSources = BSYSlib_List_Create();
	hChn->sAudio.hSinks = BSYSlib_List_Create();

#if BSYNCLIB_JITTER_TOLERANCE_IMPROVEMENT_SUPPORT
	BDBG_MSG(("chn %p: jitter tolerance improvement supported", (void*)hChn));
#endif

#if BSYNCLIB_UNCONDITIONAL_VIDEO_UNMUTE_SUPPORT
	BDBG_MSG(("chn %p: unconditional video unmute supported", (void*)hChn));
#endif

#if BSYNCLIB_UNCONDITIONAL_AUDIO_UNMUTE_SUPPORT
	BDBG_MSG(("chn %p: unconditional audio unmute supported", (void*)hChn));
#endif

	*phChn = hChn;

	goto end;

	error:

	if (hChn)
	{
		BSYNClib_Channel_P_Destroy(hChn);
		*phChn = NULL;
	}

	end:

	BDBG_LEAVE(BSYNClib_Channel_P_Create);
	return rc;
}


/*
Summary:
Closes an SYNC lib channel
Description:
 */
void BSYNClib_Channel_P_Destroy(
	BSYNClib_Channel_Handle hChn
)
{
	BSYNClib_Channel_Config sConfig;

	BDBG_ENTER(BSYNClib_Channel_P_Destroy);

	BDBG_ASSERT(hChn);

	BSYNClib_Channel_P_Stop(hChn);

	/* this will empty the lists and destroy all the elements (freeing up the timers) */
	BSYNClib_Channel_GetConfig(hChn, &sConfig);
	sConfig.bEnabled = false;
	sConfig.uiVideoSourceCount = 0;
	sConfig.uiVideoSinkCount = 0;
	sConfig.uiAudioSourceCount = 0;
	sConfig.uiAudioSinkCount = 0;
	BSYNClib_Channel_SetConfig(hChn, &sConfig);

	if (hChn->hMachine)
	{
		BSYNClib_StateMachine_Close(hChn->hMachine);
	}

	if (hChn->hDetector)
	{
		BSYNClib_RateMismatchDetector_Close(hChn->hDetector);
	}

	/* deallocate timers */
	if (hChn->sSettings.cbTimer.pfDestroy)
	{
		BSYSlib_List_IteratorHandle hIterator;
		void * pvParm1 = hChn->sSettings.cbTimer.pvParm1;
		int iParm2 = hChn->sSettings.cbTimer.iParm2;

		/* Timer for mute control task context switch */
		BDBG_MSG(("chn %p: Destroying mute control task context switch timer %p", (void*)hChn, hChn->psMuteControlTaskTimer->hTimer));
		hChn->sSettings.cbTimer.pfDestroy(pvParm1, iParm2, hChn->psMuteControlTaskTimer->hTimer);
		hChn->psMuteControlTaskTimer->hTimer = NULL;
		BKNI_Free(hChn->psMuteControlTaskTimer);

		/* Timer for task context switch */
		BDBG_MSG(("chn %p: Destroying task context switch timer %p", (void*)hChn, hChn->psTaskTimer->hTimer));
		hChn->sSettings.cbTimer.pfDestroy(pvParm1, iParm2, hChn->psTaskTimer->hTimer);
		hChn->psTaskTimer->hTimer = NULL;
		BKNI_Free(hChn->psTaskTimer);

		/* TODO: remove dependency on resource pool impl */
		hIterator = BSYSlib_List_AcquireIterator(hChn->psTimers->hResources);

		while (BSYSlib_List_HasNext(hIterator))
		{
			BSYNClib_Timer * psTimer;

			psTimer = (BSYNClib_Timer *)BSYSlib_List_Next(hIterator);

			if (psTimer)
			{
				BDBG_MSG(("chn %p: Destroying timer %p", (void*)hChn, psTimer->hTimer));

				hChn->sSettings.cbTimer.pfDestroy(pvParm1, iParm2, psTimer->hTimer);
				psTimer->hTimer = NULL;

				BSYNClib_ResourcePool_Remove(hChn->psTimers, psTimer);

				BKNI_Free(psTimer);
			}
		}

		/* destroy timer resource pool */
		BSYNClib_ResourcePool_Destroy(hChn->psTimers);
	}

	BSYSlib_List_Destroy(hChn->sVideo.hSources);
	BSYSlib_List_Destroy(hChn->sVideo.hSinks);
	BSYSlib_List_Destroy(hChn->sAudio.hSources);
	BSYSlib_List_Destroy(hChn->sAudio.hSinks);

	/* free me */
	BKNI_Free(hChn);

	BDBG_LEAVE(BSYNClib_Channel_P_Destroy);
}

/*******************************************
syslib framework stuff
 *******************************************/

void BSYNClib_Channel_P_CancelTimer_isr(
	BSYNClib_Channel_Handle hChn,
	BSYNClib_Timer * psTimer
)
{
	BDBG_ENTER(BSYNClib_Channel_P_CancelTimer_isr);

	BDBG_ASSERT(hChn);
	BDBG_ASSERT(psTimer);

	if (psTimer->bScheduled)
	{
		if (hChn->sSettings.cbTimer.pfCancel_isr)
		{
			hChn->sSettings.cbTimer.pfCancel_isr(
				hChn->sSettings.cbTimer.pvParm1,
				hChn->sSettings.cbTimer.iParm2,
				psTimer->hTimer);
		}
		psTimer->bScheduled = false;
	}

	BDBG_LEAVE(BSYNClib_Channel_P_CancelTimer_isr);
	return;
}

BERR_Code BSYNClib_Channel_P_StartTimer_isr(
	BSYNClib_Channel_Handle hChn,
	BSYNClib_Timer * psTimer,
	unsigned long ulTimeout,
	BSYSlib_Timer_ExpiryHandler pfTimerExpired,
	void * pvParm1,
	int iParm2
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BSYNClib_Channel_P_StartTimer_isr);

	BDBG_ASSERT(hChn);
	BDBG_ASSERT(psTimer);
	BDBG_ASSERT(pfTimerExpired);

	if (hChn->sSettings.cbTimer.pfStart_isr)
	{
		BSYSlib_Timer_Settings sTimer;

		sTimer.ulTimeout = ulTimeout;
		sTimer.pvParm1 = pvParm1;
		sTimer.iParm2 = iParm2;
		sTimer.pfTimerExpired = pfTimerExpired;

		/* just reusing stack space here to reduce typing */
		pvParm1 = hChn->sSettings.cbTimer.pvParm1;
		iParm2 = hChn->sSettings.cbTimer.iParm2;

		/* clean up old timer if any */
		BSYNClib_Channel_P_CancelTimer_isr(hChn, psTimer);

		/* reschedule timer */
		rc = hChn->sSettings.cbTimer.pfStart_isr(pvParm1, iParm2, psTimer->hTimer, &sTimer);
		if (rc) goto error;

		psTimer->bScheduled = true;
	}

	goto end;

	error:

	end:

	BDBG_LEAVE(BSYNClib_Channel_P_StartTimer_isr);
	return rc;
}


/*
Summary:
Handles expiry of any channel timer except for the zero length context switching timer
 */
BERR_Code BSYNClib_Channel_P_TimerExpired(
	BSYNClib_Channel_Handle hChn,
	BSYSlib_Timer_Handle hTimer
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYSlib_List_IteratorHandle hIterator;
	BDBG_ENTER(BSYNClib_Channel_P_TimerExpired);

	BDBG_ASSERT(hChn);
	BDBG_ASSERT(hTimer);

	/* clean up timer that expired */
	/* TODO: remove dependency on resource pool internals */
	hIterator = BSYSlib_List_AcquireIterator(hChn->psTimers->hResources);
	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_Timer * psTimer;

		psTimer = (BSYNClib_Timer *)BSYSlib_List_Next(hIterator);
		if (psTimer->hTimer == hTimer)
		{
			BKNI_EnterCriticalSection();
			BSYNClib_Timer_Reset_isr(psTimer);
			BKNI_LeaveCriticalSection();
			break;
		}
	}
	BSYSlib_List_ReleaseIterator(hIterator);

	BDBG_LEAVE(BSYNClib_Channel_P_TimerExpired);
	return rc;
}

BERR_Code BSYNClib_Channel_P_ScheduleTask_isr(BSYNClib_Channel_Handle hChn)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_Channel_Results * psResults;

	BDBG_ENTER(BSYNClib_Channel_P_ScheduleTask_isr);

	BDBG_ASSERT(hChn);

	psResults = &hChn->sResults;

	if (!psResults->bSyncTaskScheduled)
	{
		rc = BSYNClib_Channel_P_StartTimer_isr(hChn,
			hChn->psTaskTimer, 0,
			&BSYNClib_Channel_P_TaskTimerExpired, hChn, 0);

		if (!rc)
		{
			psResults->bSyncTaskScheduled = true;
			BDBG_MSG(("[%d] Sync task scheduled", hChn->iIndex));
		}
		else
		{
			psResults->bSyncTaskScheduled = false;
		}
		if (rc) goto error;
	}
	else
	{
		BDBG_MSG(("[%d] Sync task already scheduled", hChn->iIndex));
	}

	goto end;

	error:

	end:
	BDBG_LEAVE(BSYNClib_Channel_P_ScheduleTask_isr);
	return rc;
}

BERR_Code BSYNClib_Channel_P_EnqueueTaskRequest_isr(BSYNClib_Channel_Handle hChn)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_Channel_Results * psResults;

	BDBG_ENTER(BSYNClib_Channel_P_EnqueueTaskRequest_isr);

	BDBG_ASSERT(hChn);

	psResults = &hChn->sResults;

	if (!psResults->bSyncTaskRequested)
	{
		psResults->bSyncTaskRequested = true;
		BDBG_MSG(("[%d] Sync task request enqueued", hChn->iIndex));
	}

	BDBG_LEAVE(BSYNClib_Channel_P_EnqueueTaskRequest_isr);
	return rc;
}

BERR_Code BSYNClib_Channel_P_DequeueTaskRequest_isr(BSYNClib_Channel_Handle hChn)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_Channel_Results * psResults;

	BDBG_ENTER(BSYNClib_Channel_P_DequeueTaskRequest_isr);

	BDBG_ASSERT(hChn);

	psResults = &hChn->sResults;

	if (psResults->bSyncTaskRequested)
	{
		psResults->bSyncTaskRequested = false;
		BDBG_MSG(("[%d] Sync task request dequeued", hChn->iIndex));
		rc = BSYNClib_Channel_P_ScheduleTask_isr(hChn);
		if (rc) BERR_TRACE(rc);
	}

	BDBG_LEAVE(BSYNClib_Channel_P_DequeueTaskRequest_isr);
	return rc;
}

BERR_Code BSYNClib_Channel_P_TaskTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_Channel_Handle hChn = pvParm1;

	BDBG_ENTER(BSYNClib_Channel_P_TaskTimerExpired);

	BSTD_UNUSED(iParm2);

	BDBG_ASSERT(hChn);
	BDBG_ASSERT(hTimer);

	BDBG_MSG(("[%d] Sync task timer expired", hChn->iIndex));

	rc = BSYNClib_Channel_P_TimerExpired(hChn, hTimer);
	if (rc) goto end;

	rc = BSYNClib_Channel_P_Process(hChn, 0);

	end:
	BDBG_LEAVE(BSYNClib_Channel_P_TaskTimerExpired);
	return rc;
}

bool BSYNClib_Channel_P_SyncCheck(
	BSYNClib_Channel_Handle hChn
)
{
	BSYSlib_List_IteratorHandle hIterator;
	bool bSync = true;

	BDBG_ENTER(BSYNClib_Channel_P_SyncCheck);

	BDBG_ASSERT(hChn);

	BDBG_MSG(("[%d] Performing sync check...", hChn->iIndex));

	/* TODO: at least one video and audio source should pass and be synchronized in order to sync av */
	/* TODO: all video sinks should pass before doing v/v sync */
	/* TODO: all audio sinks should pass before doing a/a sync */
	hIterator = BSYSlib_List_AcquireIterator(hChn->sVideo.hSources);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_VideoSource * psSource;

		psSource = (BSYNClib_VideoSource *)BSYSlib_List_Next(hIterator);

		bSync = bSync && BSYNClib_VideoSource_SyncCheck(psSource);
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	hIterator = BSYSlib_List_AcquireIterator(hChn->sVideo.hSinks);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_VideoSink * psSink;

		psSink = (BSYNClib_VideoSink *)BSYSlib_List_Next(hIterator);

		/* invisible windows are ignored */
		if (psSink->sSnapshot.bVisible)
		{
			bSync = bSync && BSYNClib_VideoSink_SyncCheck(psSink);
		}
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	hIterator = BSYSlib_List_AcquireIterator(hChn->sAudio.hSources);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_AudioSource * psSource;

		psSource = (BSYNClib_AudioSource *)BSYSlib_List_Next(hIterator);

		bSync = bSync && BSYNClib_AudioSource_SyncCheck(psSource);
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	hIterator = BSYSlib_List_AcquireIterator(hChn->sAudio.hSinks);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_AudioSink * psSink;

		psSink = (BSYNClib_AudioSink *)BSYSlib_List_Next(hIterator);

		bSync = bSync && BSYNClib_AudioSink_SyncCheck(psSink);
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	BDBG_LEAVE(BSYNClib_Channel_P_SyncCheck);
	return bSync;
}

void BSYNClib_Channel_P_Snapshot_isr(
	BSYNClib_Channel_Handle hChn
)
{
	BSYSlib_List_IteratorHandle hIterator;

	BDBG_ENTER(BSYNClib_Channel_P_Snapshot_isr);

	BDBG_ASSERT(hChn);

	hIterator = BSYSlib_List_AcquireIterator_isr(hChn->sVideo.hSources);

	while (BSYSlib_List_HasNext_isr(hIterator))
	{
		BSYNClib_VideoSource * psSource;

		psSource = (BSYNClib_VideoSource *)BSYSlib_List_Next_isr(hIterator);

		BSYNClib_VideoSource_Snapshot_isr(psSource);
	}

	BSYSlib_List_ReleaseIterator_isr(hIterator);

	hIterator = BSYSlib_List_AcquireIterator_isr(hChn->sVideo.hSinks);

	while (BSYSlib_List_HasNext_isr(hIterator))
	{
		BSYNClib_VideoSink * psSink;

		psSink = (BSYNClib_VideoSink *)BSYSlib_List_Next_isr(hIterator);

		BSYNClib_VideoSink_Snapshot_isr(psSink);
	}

	BSYSlib_List_ReleaseIterator_isr(hIterator);

	hIterator = BSYSlib_List_AcquireIterator_isr(hChn->sAudio.hSources);

	while (BSYSlib_List_HasNext_isr(hIterator))
	{
		BSYNClib_AudioSource * psSource;

		psSource = (BSYNClib_AudioSource *)BSYSlib_List_Next_isr(hIterator);

		BSYNClib_AudioSource_Snapshot_isr(psSource);
	}

	BSYSlib_List_ReleaseIterator_isr(hIterator);

	hIterator = BSYSlib_List_AcquireIterator_isr(hChn->sAudio.hSinks);

	while (BSYSlib_List_HasNext_isr(hIterator))
	{
		BSYNClib_AudioSink * psSink;

		psSink = (BSYNClib_AudioSink *)BSYSlib_List_Next_isr(hIterator);

		BSYNClib_AudioSink_Snapshot_isr(psSink);
	}

	BSYSlib_List_ReleaseIterator_isr(hIterator);

	BDBG_LEAVE(BSYNClib_Channel_P_Snapshot_isr);
}

#if !BDBG_NO_MSG
static const char * const gpcSyncStateNames[] =
	{
		"stopped",
		"acquire",
		"sync",
		"track",
		NULL
	};
#endif

BERR_Code BSYNClib_Channel_P_Process(void * pvParm1, int iParm2)
{
	BERR_Code rc = BERR_SUCCESS;
	bool bSync = true;
	BSYNClib_Channel_Handle hChn = pvParm1;
	BSYNClib_StateMachine_Status sStatus;

	BDBG_ENTER(BSYNClib_Channel_P_Process);

	BSTD_UNUSED(iParm2);

	BDBG_ASSERT(hChn);

	BKNI_EnterCriticalSection();
	hChn->sResults.bSyncTaskScheduled = false;
	BKNI_LeaveCriticalSection();

	BDBG_MSG(("[%d] Sync process invoked", hChn->iIndex));

	BDBG_MSG(("[%d] Snapshotting data...", hChn->iIndex));

	/* snapshot all the data we will use for our calcs
	this is so we don't hold the cs for a long time while doing those calcs */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_P_Snapshot_isr(hChn);
	BKNI_LeaveCriticalSection();

	if (!BSYNClib_Channel_P_Enabled_isrsafe(hChn))
	{
		goto end;
	}

	/* do a sync check */
	bSync = BSYNClib_Channel_P_SyncCheck(hChn);

	BDBG_MSG(("[%d] Sync check %s", hChn->iIndex, bSync ? "passed" : "failed"));

	/* send sync check signal */
	if (bSync)
	{
		rc = BSYNClib_StateMachine_SendSignal(hChn->hMachine, BSYNClib_StateMachine_Signal_eCheckPassed);
		if (rc) goto error;

		/* also send data changed signal, because we got here */
		rc = BSYNClib_StateMachine_SendSignal(hChn->hMachine, BSYNClib_StateMachine_Signal_eDataChanged);
		if (rc) goto error;
	}
	else
	{
		rc = BSYNClib_StateMachine_SendSignal(hChn->hMachine, BSYNClib_StateMachine_Signal_eCheckFailed);
		if (rc) goto error;
	}

	rc = BSYNClib_StateMachine_GetStatus(hChn->hMachine, &sStatus);
	if (rc) goto error;

	/* if after all we have reached the sync state, apply delays */
	if (sStatus.eState == BSYNClib_StateMachine_State_eSync)
	{
		rc = BSYNClib_Channel_P_Synchronize(hChn);
		if (rc) goto error;

		rc = BSYNClib_StateMachine_SendSignal(hChn->hMachine, BSYNClib_StateMachine_Signal_eDelayApplied);
		if (rc) goto error;
	}

	rc = BSYNClib_StateMachine_GetStatus(hChn->hMachine, &sStatus);
	if (rc) goto error;

	BDBG_MSG(("[%d] End process sync state: %s", hChn->iIndex, gpcSyncStateNames[sStatus.eState]));

	/* call callbacks that don't result from sync */
	rc = BSYNClib_Channel_P_GenerateCallbacks(hChn);
	if (rc) goto error;

	goto end;

	error:

	end:

	BDBG_LEAVE(BSYNClib_Channel_P_Process);
	return rc;
}

BERR_Code BSYNClib_Channel_P_ApplyDelays(BSYNClib_Channel_Handle hChn)
{
	BERR_Code rc = BERR_SUCCESS;
	void * pvParm1;
	int iParm2;

	BDBG_ENTER(BSYNClib_Channel_P_ApplyDelays);

	BDBG_ASSERT(hChn);

	pvParm1 = hChn->sSettings.sVideo.sSource.cbDelay.pvParm1;
	iParm2 = hChn->sSettings.sVideo.sSource.cbDelay.iParm2;

	rc = BSYNClib_Algo_VideoSource_Applicator(&hChn->sVideo, hChn->sSettings.sVideo.sSource.cbDelay.pfSetDelay, pvParm1, iParm2);
	if (rc) goto error;

	pvParm1 = hChn->sSettings.sVideo.sSink.cbDelay.pvParm1;
	iParm2 = hChn->sSettings.sVideo.sSink.cbDelay.iParm2;

	rc = BSYNClib_Algo_VideoSink_Applicator(&hChn->sVideo, hChn->sSettings.sVideo.sSink.cbDelay.pfSetDelay, pvParm1, iParm2);
	if (rc) goto error;

	pvParm1 = hChn->sSettings.sAudio.sSource.cbDelay.pvParm1;
	iParm2 = hChn->sSettings.sAudio.sSource.cbDelay.iParm2;

	rc = BSYNClib_Algo_AudioSource_Applicator(&hChn->sAudio, hChn->sSettings.sAudio.sSource.cbDelay.pfSetDelay, pvParm1, iParm2);
	if (rc) goto error;

	pvParm1 = hChn->sSettings.sAudio.sSink.cbDelay.pvParm1;
	iParm2 = hChn->sSettings.sAudio.sSink.cbDelay.iParm2;

	rc = BSYNClib_Algo_AudioSink_Applicator(&hChn->sAudio, hChn->sSettings.sAudio.sSink.cbDelay.pfSetDelay, pvParm1, iParm2);
	if (rc) goto error;

	/* TODO: copy desired and applied delays to status */

	/* start unmute timers after applying delays */
	rc = BSYNClib_MuteControl_StartUnmuteTimers(hChn);
	if (rc) goto error;

	goto end;

	error:

	end:

	BDBG_LEAVE((BSYNClib_Channel_P_ApplyDelays));
	return rc;
}

/* TODO: merge these into the rework for separate state machines for a/a, v/v, a/v sync */
bool BSYNClib_Channel_P_AudioVideoSyncCheck(BSYNClib_Channel_Handle hChn)
{
	BSYSlib_List_IteratorHandle hIterator;
	bool bVideoSync = false;
	bool bAudioSync = false;

	BDBG_ENTER(BSYNClib_Channel_P_AudioVideoSyncCheck);

	BDBG_ASSERT(hChn);

	BDBG_MSG(("[%d] Performing a/v sync check...", hChn->iIndex));

	hIterator = BSYSlib_List_AcquireIterator(hChn->sVideo.hSources);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_VideoSource * psSource;

		psSource = (BSYNClib_VideoSource *)BSYSlib_List_Next(hIterator);

		bVideoSync = bVideoSync || psSource->sElement.sSnapshot.bSynchronize;
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	hIterator = BSYSlib_List_AcquireIterator(hChn->sAudio.hSources);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_AudioSource * psSource;

		psSource = (BSYNClib_AudioSource *)BSYSlib_List_Next(hIterator);

		bAudioSync = bAudioSync || psSource->sElement.sSnapshot.bSynchronize;
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	BDBG_LEAVE(BSYNClib_Channel_P_AudioVideoSyncCheck);
	/* at least one video and audio source should be synchronized in order to sync av */
	return bVideoSync && bAudioSync;
}

/* TODO: merge these into the rework for separate state machines for a/a, v/v, a/v sync */
bool BSYNClib_Channel_P_AudioAudioSyncCheck(BSYNClib_Channel_Handle hChn)
{
	BSYSlib_List_IteratorHandle hIterator;
	unsigned int uiAudioSyncCount = 0;

	BDBG_ENTER(BSYNClib_Channel_P_AudioAudioSyncCheck);

	BDBG_ASSERT(hChn);

	BDBG_MSG(("[%d] Performing a/a sync check...", hChn->iIndex));

	hIterator = BSYSlib_List_AcquireIterator(hChn->sAudio.hSinks);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_AudioSink * psSink;

		psSink = (BSYNClib_AudioSink *)BSYSlib_List_Next(hIterator);

		if (psSink->sElement.sSnapshot.bSynchronize)
		{
			uiAudioSyncCount++;
		}
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	BDBG_LEAVE(BSYNClib_Channel_P_AudioAudioSyncCheck);
	/* TODO: at least 2 audio sinks should be synchronized for a/a sync */
	return (uiAudioSyncCount > 1);
}

/* TODO: merge these into the rework for separate state machines for a/a, v/v, a/v sync */
bool BSYNClib_Channel_P_VideoVideoSyncCheck(BSYNClib_Channel_Handle hChn)
{
	BSYSlib_List_IteratorHandle hIterator;
	unsigned int uiVideoSyncCount = 0;
	bool bResetDelay = false;

	BDBG_ENTER(BSYNClib_Channel_P_VideoVideoSyncCheck);

	BDBG_ASSERT(hChn);

	BDBG_MSG(("[%d] Performing v/v sync check...", hChn->iIndex));

	hIterator = BSYSlib_List_AcquireIterator(hChn->sVideo.hSinks);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_VideoSink * psSink;

		psSink = (BSYNClib_VideoSink *)BSYSlib_List_Next(hIterator);

		/* invisible windows are ignored */
		if ((psSink->sSnapshot.bVisible && psSink->sElement.sSnapshot.bSynchronize) || bResetDelay)
		{
			uiVideoSyncCount++;
		}
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	BDBG_LEAVE(BSYNClib_Channel_P_VideoVideoSyncCheck);
	return (uiVideoSyncCount > 1);
}

BERR_Code BSYNClib_Channel_P_Synchronize(
	BSYNClib_Channel_Handle hChn
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BSYNClib_Channel_P_Synchronize);

	BDBG_ASSERT(hChn);

	/* TODO: change to 3 separate state machines */
	if (BSYNClib_Channel_P_VideoVideoSyncCheck(hChn))
	{
		BSYNClib_RateMismatchDetector_Status sRmdStatus;
		BSYNClib_RateMismatchDetector_GetStatus(hChn->hDetector, &sRmdStatus);
		/* TODO: move this to a better location; here is better than sync check, but still not ideal */
		if (!sRmdStatus.bSinkSinkMatched)
		{
			BDBG_MSG(("[%d] Inconsistent sink refresh rate domains, v/v sync reset", hChn->iIndex));
			hChn->sVideo.sResults.bInconsistentSinkDomains = true;
		}
		else
		{
			hChn->sVideo.sResults.bInconsistentSinkDomains = false;
		}
		BSYNClib_Algo_VideoVideo_Sync(&hChn->sVideo);
	}
	if (BSYNClib_Channel_P_AudioAudioSyncCheck(hChn))
	{
		BSYNClib_Algo_AudioAudio_Sync(&hChn->sAudio);
	}
	if (BSYNClib_Channel_P_AudioVideoSyncCheck(hChn))
	{
		BSYNClib_Algo_AudioVideo_Sync(&hChn->sAudio, &hChn->sVideo, &hChn->sResults);
	}

	if (!hChn->sResults.bSyncTaskScheduled)
	{
		rc = BSYNClib_Channel_P_ApplyDelays(hChn);
		if (rc) goto error;
	}
	else
	{
		BDBG_MSG(("[%d] Impending sync readjustment -- deferring delay application", hChn->iIndex));
	}

	goto end;

	error:

	end:

	BDBG_LEAVE(BSYNClib_Channel_P_Synchronize);
	return rc;
}

BERR_Code BSYNClib_Channel_P_GenerateDelayCallback(BSYNClib_Channel_SetDelay pfSetDelay, void * pvParm1, int iParm2, unsigned int uiDeviceIndex, BSYNClib_DelayElement * psElement)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BSYNClib_Channel_P_GenerateDelayCallback);

	BDBG_ASSERT(psElement);

	if (psElement->sDelay.sResults.bGenerateCallback && psElement->sSnapshot.bSynchronize)
	{
		if (pfSetDelay)
		{
			BSYNClib_UnsignedValue sApplied;
			sApplied.uiValue = BSYNClib_P_Convert_isrsafe(
				psElement->sDelay.sResults.uiApplied,
				BSYNClib_Units_e27MhzTicks,
				psElement->sDelay.sSnapshot.ePreferredUnits);
			sApplied.eUnits = psElement->sDelay.sSnapshot.ePreferredUnits;
			rc = pfSetDelay(pvParm1, iParm2, uiDeviceIndex, &sApplied);
			if (rc) goto error;
		}

		psElement->sDelay.sResults.bGenerateCallback = false;
	}

	goto end;

	error:

	end:

	BDBG_LEAVE(BSYNClib_Channel_P_GenerateDelayCallback);
	return rc;
}

BERR_Code BSYNClib_Channel_P_GenerateDelayNotificationCallback(BSYNClib_Channel_SetDelayNotification pfSetDelayNotification, void * pvParm1, int iParm2, unsigned int uiDeviceIndex, BSYNClib_DelayElement * psElement)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BSYNClib_Channel_P_GenerateDelayNotificationCallback);

	BDBG_ASSERT(psElement);

	if (psElement->sNotification.sResults.bGenerateCallback && psElement->sSnapshot.bSynchronize)
	{
		if (pfSetDelayNotification)
		{
			BSYNClib_UnsignedValue sThreshold;
			sThreshold = psElement->sNotification.sResults.sThreshold;
			/* convert to preferred units */
			if (sThreshold.eUnits != psElement->sDelay.sSnapshot.ePreferredUnits)
			{
				sThreshold.uiValue = BSYNClib_P_Convert_isrsafe(
					sThreshold.uiValue, sThreshold.eUnits,
					BSYNClib_Units_e27MhzTicks);
				sThreshold.uiValue = BSYNClib_P_Convert_isrsafe(
					sThreshold.uiValue, BSYNClib_Units_e27MhzTicks,
					psElement->sNotification.sSnapshot.ePreferredUnits);
				sThreshold.eUnits = psElement->sNotification.sSnapshot.ePreferredUnits;
			}
			rc = pfSetDelayNotification(pvParm1, iParm2, uiDeviceIndex, psElement->sNotification.sResults.bEnabled, &sThreshold);
			if (rc) goto error;
		}

		psElement->sNotification.sResults.bGenerateCallback = false;
	}

	goto end;

	error:

	end:

	BDBG_LEAVE(BSYNClib_Channel_P_GenerateDelayNotificationCallback);
	return rc;
}

BERR_Code BSYNClib_Channel_P_GenerateCallbacks(BSYNClib_Channel_Handle hChn)
{
	BERR_Code rc = BERR_SUCCESS;
	void * pvParm1;
	int iParm2;
	BSYSlib_List_IteratorHandle hIterator;

	BDBG_ENTER(BSYNClib_Channel_P_GenerateCallbacks);

	BDBG_ASSERT(hChn);

	pvParm1 = hChn->sSettings.sVideo.sSource.cbDelay.pvParm1;
	iParm2 = hChn->sSettings.sVideo.sSource.cbDelay.iParm2;

	hIterator = BSYSlib_List_AcquireIterator(hChn->sVideo.hSources);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_VideoSource * psSource;

		psSource = (BSYNClib_VideoSource *)BSYSlib_List_Next(hIterator);

		rc = BSYNClib_Channel_P_GenerateDelayCallback(hChn->sSettings.sVideo.sSource.cbDelay.pfSetDelay, pvParm1, iParm2, psSource->sElement.uiIndex, &psSource->sElement);
		if (rc) goto error;

		rc = BSYNClib_Channel_P_GenerateDelayNotificationCallback(hChn->sSettings.sVideo.sSource.cbDelay.pfSetDelayNotification, pvParm1, iParm2, psSource->sElement.uiIndex, &psSource->sElement);
		if (rc) goto error;
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	pvParm1 = hChn->sSettings.sVideo.sSink.cbDelay.pvParm1;
	iParm2 = hChn->sSettings.sVideo.sSink.cbDelay.iParm2;

	hIterator = BSYSlib_List_AcquireIterator(hChn->sVideo.hSinks);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_VideoSink * psSink;

		psSink = (BSYNClib_VideoSink *)BSYSlib_List_Next(hIterator);

		rc = BSYNClib_Channel_P_GenerateDelayCallback(hChn->sSettings.sVideo.sSink.cbDelay.pfSetDelay, pvParm1, iParm2, psSink->sElement.uiIndex, &psSink->sElement);
		if (rc) goto error;

		rc = BSYNClib_Channel_P_GenerateDelayNotificationCallback(hChn->sSettings.sVideo.sSink.cbDelay.pfSetDelayNotification, pvParm1, iParm2, psSink->sElement.uiIndex, &psSink->sElement);
		if (rc) goto error;
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	pvParm1 = hChn->sSettings.sAudio.sSource.cbDelay.pvParm1;
	iParm2 = hChn->sSettings.sAudio.sSource.cbDelay.iParm2;

	hIterator = BSYSlib_List_AcquireIterator(hChn->sAudio.hSources);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_AudioSource * psSource;

		psSource = (BSYNClib_AudioSource *)BSYSlib_List_Next(hIterator);

		rc = BSYNClib_Channel_P_GenerateDelayCallback(hChn->sSettings.sAudio.sSource.cbDelay.pfSetDelay, pvParm1, iParm2, psSource->sElement.uiIndex, &psSource->sElement);
		if (rc) goto error;

		rc = BSYNClib_Channel_P_GenerateDelayNotificationCallback(hChn->sSettings.sAudio.sSource.cbDelay.pfSetDelayNotification, pvParm1, iParm2, psSource->sElement.uiIndex, &psSource->sElement);
		if (rc) goto error;
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	pvParm1 = hChn->sSettings.sAudio.sSink.cbDelay.pvParm1;
	iParm2 = hChn->sSettings.sAudio.sSink.cbDelay.iParm2;

	hIterator = BSYSlib_List_AcquireIterator(hChn->sAudio.hSinks);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_AudioSink * psSink;

		psSink = (BSYNClib_AudioSink *)BSYSlib_List_Next(hIterator);

		rc = BSYNClib_Channel_P_GenerateDelayCallback(hChn->sSettings.sAudio.sSink.cbDelay.pfSetDelay, pvParm1, iParm2, psSink->sElement.uiIndex, &psSink->sElement);
		if (rc) goto error;

		rc = BSYNClib_Channel_P_GenerateDelayNotificationCallback(hChn->sSettings.sAudio.sSink.cbDelay.pfSetDelayNotification, pvParm1, iParm2, psSink->sElement.uiIndex, &psSink->sElement);
		if (rc) goto error;
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	goto end;

	error:

	end:

	BDBG_LEAVE(BSYNClib_Channel_P_GenerateCallbacks);
	return rc;
}

bool BSYNClib_Channel_P_Enabled_isrsafe(
	BSYNClib_Channel_Handle hChn
)
{
	BDBG_ENTER(BSYNClib_Channel_P_Enabled_isrsafe);
	BDBG_ASSERT(hChn);
	BDBG_LEAVE(BSYNClib_Channel_P_Enabled_isrsafe);
	return hChn->bEnabled;
}

void BSYNClib_Channel_P_ResetVideoSourceJtiFactor_isr(
	BSYNClib_Channel_Handle hChn
)
{
	BSYNClib_VideoSource * psSource = NULL;
	BSYSlib_List_IteratorHandle hIterator = NULL;

	BDBG_ENTER(BSYNClib_Channel_P_ResetVideoSourceJtiFactor_isr);

	BDBG_ASSERT(hChn);

	/* reset 1st source PWC factor */
	hIterator = BSYSlib_List_AcquireIterator_isr(hChn->sVideo.hSources);

	if (BSYSlib_List_HasNext_isr(hIterator))
	{
		psSource = (BSYNClib_VideoSource *)BSYSlib_List_Next_isr(hIterator);
	}

	BSYSlib_List_ReleaseIterator_isr(hIterator);

	if (psSource)
	{
		/* TODO: this is a problem since we don't enter CS around DCF in task context */
		psSource->sResults.sJtiFactor.iValue = 0;
	}

	BDBG_LEAVE(BSYNClib_Channel_P_ResetVideoSourceJtiFactor_isr);
}

void BSYNClib_Channel_P_GetDefaultStatus(
	BSYNClib_Channel_Status * psStatus
)
{
	BDBG_ENTER(BSYNClib_Channel_P_GetDefaultStatus);
	BDBG_ASSERT(psStatus);
	BKNI_Memset(psStatus, 0, sizeof(BSYNClib_Channel_Status));
	BDBG_LEAVE(BSYNClib_Channel_P_GetDefaultStatus);
}

void BSYNClib_Channel_P_Stop(
	BSYNClib_Channel_Handle hChn
)
{
	BDBG_ENTER(BSYNClib_Channel_P_Stop);

	BDBG_ASSERT(hChn);

	/* stop any running timers */
	if (hChn->sSettings.cbTimer.pfCancel_isr)
	{
		BSYSlib_List_IteratorHandle hIterator;
		void * pvParm1 = hChn->sSettings.cbTimer.pvParm1;
		int iParm2 = hChn->sSettings.cbTimer.iParm2;

		/* Timer for task context switch */
		BKNI_EnterCriticalSection();
		if (hChn->psTaskTimer->bScheduled)
		{
			BDBG_MSG(("[%d] Cancelling task context switch timer %p", hChn->iIndex, hChn->psTaskTimer->hTimer));
			hChn->sSettings.cbTimer.pfCancel_isr(pvParm1, iParm2, hChn->psTaskTimer->hTimer);
		}
		BKNI_LeaveCriticalSection();

		/* TODO: remove dependency on resource pool impl */
		hIterator = BSYSlib_List_AcquireIterator(hChn->psTimers->hResources);

		while (BSYSlib_List_HasNext(hIterator))
		{
			BSYNClib_Timer * psTimer;

			psTimer = (BSYNClib_Timer *)BSYSlib_List_Next(hIterator);

			if (psTimer)
			{
				BKNI_EnterCriticalSection();
				if (psTimer->bScheduled)
				{
					BDBG_MSG(("[%d] Cancelling timer %p", hChn->iIndex, psTimer->hTimer));
					hChn->sSettings.cbTimer.pfCancel_isr(pvParm1, iParm2, psTimer->hTimer);
				}
				BKNI_LeaveCriticalSection();
			}
		}
	}

	BDBG_LEAVE(BSYNClib_Channel_P_Stop);
}
