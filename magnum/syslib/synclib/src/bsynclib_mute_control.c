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
#include "bsyslib.h"
#include "bsyslib_list.h"
#include "bsynclib.h"
#include "bsynclib_priv.h"
#include "bsynclib_channel_priv.h"
#include "bsynclib_mute_control.h"
#include "bsynclib_mute_control_priv.h"
#include "bsynclib_audio_source.h"
#include "bsynclib_video_source.h"

BDBG_MODULE(synclib);

BERR_Code BSYNClib_MuteControl_ScheduleTask_isr(BSYNClib_Channel_Handle hChn)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_Channel_Results * psResults;

	BDBG_ENTER(BSYNClib_MuteControl_ScheduleTask_isr);

	BDBG_ASSERT(hChn);

	psResults = &hChn->sResults;

	if (!psResults->bMuteTaskScheduled)
	{
		rc = BSYNClib_Channel_P_StartTimer_isr(hChn,
			hChn->psMuteControlTaskTimer, 0,
			&BSYNClib_MuteControl_P_TaskTimerExpired, hChn, 0);

		if (!rc)
		{
			psResults->bMuteTaskScheduled = true;
			BDBG_MSG(("[%d] Mute control task scheduled", hChn->iIndex));
		}
		else
		{
			psResults->bMuteTaskScheduled = false;
		}
		if (rc) goto error;
	}
	else
	{
		BDBG_MSG(("[%d] Mute control task already scheduled", hChn->iIndex));
	}

	goto end;

error:

end:

	BDBG_LEAVE(BSYNClib_MuteControl_ScheduleTask_isr);
	return rc;
}

void BSYNClib_MuteControl_CancelUnmuteTimers_isr(BSYNClib_Channel_Handle hChn)
{
	BSYSlib_List_IteratorHandle hIterator;
	BSYNClib_VideoSource * psVideoSource;
	BSYNClib_AudioSource * psAudioSource;

	BDBG_ASSERT(hChn);

	BDBG_MSG(("[%d] Canceling all unmute timers", hChn->iIndex));

	hIterator = BSYSlib_List_AcquireIterator_isr(hChn->sVideo.hSources);
	while (BSYSlib_List_HasNext_isr(hIterator))
	{
		psVideoSource = (BSYNClib_VideoSource *)BSYSlib_List_Next_isr(hIterator);

		BSYNClib_Channel_P_CancelTimer_isr(hChn, psVideoSource->psUnmuteTimer);
	}
	BSYSlib_List_ReleaseIterator_isr(hIterator);

	hIterator = BSYSlib_List_AcquireIterator_isr(hChn->sAudio.hSources);
	while (BSYSlib_List_HasNext_isr(hIterator))
	{
		psAudioSource = (BSYNClib_AudioSource *)BSYSlib_List_Next_isr(hIterator);

		BSYNClib_Channel_P_CancelTimer_isr(hChn, psAudioSource->psUnmuteTimer);
	}
	BSYSlib_List_ReleaseIterator_isr(hIterator);

	return;
}

BERR_Code BSYNClib_MuteControl_StartUnmuteTimers(BSYNClib_Channel_Handle hChn)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYSlib_List_IteratorHandle hIterator;
	BSYNClib_VideoSource * psVideoSource;
	BSYNClib_AudioSource * psAudioSource;
	unsigned int count;

	BDBG_ASSERT(hChn);

	count = 0;
	hIterator = BSYSlib_List_AcquireIterator(hChn->sVideo.hSources);
	while (BSYSlib_List_HasNext(hIterator))
	{
		psVideoSource = (BSYNClib_VideoSource *)BSYSlib_List_Next(hIterator);

		/* if we've already applied delays, we shouldn't be muting */
		psVideoSource->sResults.bMutePending = false;

		if (psVideoSource->sStatus.bMuted)
		{
			count++;
			BKNI_EnterCriticalSection();
			rc = BSYNClib_Channel_P_StartTimer_isr(hChn, psVideoSource->psUnmuteTimer,
				hChn->hParent->sSettings.sVideo.uiUnmuteTimeout,
				&BSYNClib_MuteControl_P_VideoSourceUnmuteTimerExpired, psVideoSource, 0);
			BKNI_LeaveCriticalSection();
			if (rc) goto error;
		}
	}
	BSYSlib_List_ReleaseIterator(hIterator);

	hIterator = BSYSlib_List_AcquireIterator(hChn->sAudio.hSources);
	while (BSYSlib_List_HasNext(hIterator))
	{
		psAudioSource = (BSYNClib_AudioSource *)BSYSlib_List_Next(hIterator);

		/* if we've already applied delays, we shouldn't be muting */
		psAudioSource->sResults.bMutePending = false;

		if (psAudioSource->sStatus.bMuted)
		{
			count++;
			BKNI_EnterCriticalSection();
			rc = BSYNClib_Channel_P_StartTimer_isr(hChn, psAudioSource->psUnmuteTimer,
				hChn->hParent->sSettings.sAudio.uiUnmuteTimeout,
				&BSYNClib_MuteControl_P_AudioSourceUnmuteTimerExpired, psAudioSource, 0);
			BKNI_LeaveCriticalSection();
			if (rc) goto error;
		}
	}
	BSYSlib_List_ReleaseIterator(hIterator);

	if (count)
	{
		BDBG_MSG(("[%d] Scheduled %u unmute timers", hChn->iIndex, count));
	}

	goto end;

	error:
	if (hIterator)
	{
		BSYSlib_List_ReleaseIterator(hIterator);
	}

end:

	return rc;
}

BERR_Code BSYNClib_MuteControl_P_TaskTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_Channel_Handle hChn = pvParm1;

	BSTD_UNUSED(iParm2);
	BDBG_ASSERT(hChn);
	BDBG_ASSERT(hTimer);

	BDBG_MSG(("[%d] Mute task timer expired", hChn->iIndex));

	/* clean up this timer */
	BSYNClib_Channel_P_TimerExpired(hChn, hTimer);

	rc = BSYNClib_MuteControl_P_Process(hChn, 0);

	return rc;
}

BERR_Code BSYNClib_MuteControl_P_Process(void * pvParm1, int iParm2)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_Channel_Handle hChn = pvParm1;
	BSYSlib_List_IteratorHandle hIterator;
	BSYNClib_VideoSource * psVideoSource;
	BSYNClib_AudioSource * psAudioSource;
	bool bMutePending;
	bool bStarted;
	bool bStartUnconditionalUnmute;
	unsigned count;

	BSTD_UNUSED(iParm2);
	BDBG_ASSERT(hChn);

	BKNI_EnterCriticalSection();
	hChn->sResults.bMuteTaskScheduled = false;
	BKNI_LeaveCriticalSection();

	BDBG_MSG(("[%d] Mute control process invoked", hChn->iIndex));

	if (hChn->sConfig.sMuteControl.bEnabled)
	{
		/* check fullscreen-ed-ness */
		if (!BSYNClib_MuteControl_P_FullScreenCheck(hChn))
		{
			rc = BSYNClib_MuteControl_P_UnmuteAll(hChn);
		}
		else
		{
			BDBG_MSG(("[%d]  Checking video sources for pending mutes or starts", hChn->iIndex));
			count = 0;
			/* check for pending video source mutes */
			hIterator = BSYSlib_List_AcquireIterator(hChn->sVideo.hSources);
			while (BSYSlib_List_HasNext(hIterator))
			{
				psVideoSource = (BSYNClib_VideoSource *)BSYSlib_List_Next(hIterator);
				BKNI_EnterCriticalSection();
				bMutePending = psVideoSource->sResults.bMutePending;
				bStarted = psVideoSource->sElement.sData.bStarted;
				bStartUnconditionalUnmute = psVideoSource->sElement.sData.bStarted && !psVideoSource->sResults.bMuteLastStarted;
				psVideoSource->sResults.bMuteLastStarted = psVideoSource->sElement.sData.bStarted;
				BKNI_LeaveCriticalSection();

#if BSYNCLIB_UNCONDITIONAL_VIDEO_UNMUTE_SUPPORT
				/*
				 * if mute control is enabled (as above), then we will start muted
				 * and need to start the unconditional unmute timer on start event.
				 * However, we may immediately unmute, so the unconditional timer
				 * needs to be started before we handle the mute.
				 */
				if (bStartUnconditionalUnmute)
				{
					count++;
					/* need to start this when video is started, not when it is muted */
					if (!psVideoSource->psUnconditionalUnmuteTimer->bScheduled)
					{
						BDBG_MSG(("[%d]  Scheduling video unconditional unmute timer", hChn->iIndex));
						/* schedule unconditional unmute timer */
						BKNI_EnterCriticalSection();
						rc = BSYNClib_Channel_P_StartTimer_isr(hChn, psVideoSource->psUnconditionalUnmuteTimer,
							hChn->hParent->sSettings.sVideo.uiUnconditionalUnmuteTimeout,
							&BSYNClib_MuteControl_P_VideoSourceUnconditionalUnmuteTimerExpired, psVideoSource, 0);
						BKNI_LeaveCriticalSection();
						if (rc) goto end;
					}
				}
#endif

				if (bMutePending)
				{
					count++;
					rc = BSYNClib_MuteControl_P_HandleVideoSourceMutePending(hChn, psVideoSource, bStarted);
					if (rc) goto end;
				}
			}
			BSYSlib_List_ReleaseIterator(hIterator);
			if (!count)
			{
				BDBG_MSG(("[%d]    No pending video source mutes or starts found", hChn->iIndex));
			}

			BDBG_MSG(("[%d]  Checking audio sources for pending mutes or starts", hChn->iIndex));
			count = 0;
			/* check for pending audio source mutes and start events */
			hIterator = BSYSlib_List_AcquireIterator(hChn->sAudio.hSources);
			while (BSYSlib_List_HasNext(hIterator))
			{
				psAudioSource = (BSYNClib_AudioSource *)BSYSlib_List_Next(hIterator);
				BKNI_EnterCriticalSection();
				bMutePending = psAudioSource->sResults.bMutePending;
				bStarted = psAudioSource->sElement.sData.bStarted;
				bStartUnconditionalUnmute = psAudioSource->sElement.sData.bStarted && !psAudioSource->sResults.bMuteLastStarted;
				psAudioSource->sResults.bMuteLastStarted = psAudioSource->sElement.sData.bStarted;
				BKNI_LeaveCriticalSection();

#if BSYNCLIB_UNCONDITIONAL_AUDIO_UNMUTE_SUPPORT
				/*
				 * if mute control is enabled (as above), then we will start muted
				 * and need to start the unconditional unmute timer on start event.
				 * However, we may immediately unmute, so the unconditional timer
				 * needs to be started before we handle the mute.
				 */
				if (bStartUnconditionalUnmute)
				{
					count++;
					/* need to start this when audio is started, not when it is muted */
					if (!psAudioSource->psUnconditionalUnmuteTimer->bScheduled)
					{
						BDBG_MSG(("[%d]  Scheduling audio unconditional unmute timer", hChn->iIndex));
						/* schedule unconditional unmute timer */
						BKNI_EnterCriticalSection();
						rc = BSYNClib_Channel_P_StartTimer_isr(hChn, psAudioSource->psUnconditionalUnmuteTimer,
							hChn->hParent->sSettings.sAudio.uiUnconditionalUnmuteTimeout,
							&BSYNClib_MuteControl_P_AudioSourceUnconditionalUnmuteTimerExpired, psAudioSource, 0);
						BKNI_LeaveCriticalSection();
						if (rc) goto end;
					}
				}
#endif

				if (bMutePending)
				{
					count++;
					rc = BSYNClib_MuteControl_P_HandleAudioSourceMutePending(hChn, psAudioSource, bStarted);
					if (rc) goto end;
				}
			}
			BSYSlib_List_ReleaseIterator(hIterator);
			if (!count)
			{
				BDBG_MSG(("[%d]    No pending audio source mutes or starts found", hChn->iIndex));
			}
		}
	}
	else
	{
		BDBG_MSG(("[%d] Mute control process called while mute control disabled", hChn->iIndex));
	}

end:

	return rc;
}

BERR_Code BSYNClib_MuteControl_P_HandleVideoSourceMutePending(BSYNClib_Channel_Handle hChn, BSYNClib_VideoSource * psSource, bool bStarted)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_AudioSource * psAudioSource;
	bool bAudioSourceSynchronized;
	BSYSlib_List_IteratorHandle hIterator;

	BDBG_ASSERT(hChn);
	BDBG_ASSERT(psSource);

	BDBG_MSG(("[%d]  Found pending video source mute", hChn->iIndex));

	/* check if any audio sources are synchronized */
	bAudioSourceSynchronized = false;
	hIterator = BSYSlib_List_AcquireIterator(hChn->sAudio.hSources);
	while (BSYSlib_List_HasNext(hIterator))
	{
		psAudioSource = (BSYNClib_AudioSource *)BSYSlib_List_Next(hIterator);

		if (psAudioSource->sConfig.bSynchronize)/* at mute time, no snapshot has been made */
		{
			bAudioSourceSynchronized = true;
			break;
		}
	}
	BSYSlib_List_ReleaseIterator(hIterator);

	if (bAudioSourceSynchronized)
	{
		/* only mute synchronized video sources */
		if (psSource->sConfig.bSynchronize) /* at mute time, no snapshot has been made */
		{
			/* only mute if full screen check passes */
			if (BSYNClib_MuteControl_P_FullScreenCheck(hChn))
			{
				/* user intended this to be an av session, mute video until done with sync */
				BDBG_MSG(("[%d]    Video source synchronized in av session, muting", hChn->iIndex));
				rc = BSYNClib_VideoSource_SetMute(psSource, true);
				if (rc) goto end;
			}
			else
			{
				BSYNClib_MuteControl_P_UnmuteAll(hChn);
			}
		}
		else
		{
			BDBG_MSG(("[%d]    Video source not synchronized, pending mute postponed", hChn->iIndex));
		}
	}
	else
	{
		if (bStarted && !hChn->sConfig.sMuteControl.bAllowIncrementalStart)
		{
			BDBG_MSG(("[%d]    No synchronized audio sources, video started, unmuting", hChn->iIndex));
			rc = BSYNClib_VideoSource_SetMute(psSource, false);
			if (rc) goto end;
		}
		else
		{
			BDBG_MSG(("[%d]    No synchronized audio sources, pending video mute postponed", hChn->iIndex));
		}
	}

end:

	return rc;
}

BERR_Code BSYNClib_MuteControl_P_HandleAudioSourceMutePending(BSYNClib_Channel_Handle hChn, BSYNClib_AudioSource * psSource, bool bStarted)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_VideoSource * psVideoSource;
	bool bVideoSourceSynchronized;
	BSYSlib_List_IteratorHandle hIterator;

	BDBG_ASSERT(hChn);
	BDBG_ASSERT(psSource);

	BDBG_MSG(("[%d]  Found pending audio source mute", hChn->iIndex));

	/* check if any video sources are synchronized */
	bVideoSourceSynchronized = false;
	hIterator = BSYSlib_List_AcquireIterator(hChn->sVideo.hSources);
	while (BSYSlib_List_HasNext(hIterator))
	{
		psVideoSource = (BSYNClib_VideoSource *)BSYSlib_List_Next(hIterator);

		if (psVideoSource->sConfig.bSynchronize)/* at mute time, no snapshot has been made */
		{
			bVideoSourceSynchronized = true;
			break;
		}
	}
	BSYSlib_List_ReleaseIterator(hIterator);

	if (bVideoSourceSynchronized)
	{
		/* only mute synchronized audio sources */
		if (psSource->sConfig.bSynchronize)/* at mute time, no snapshot has been made */
		{
			/* only mute if full screen check passes */
			if (BSYNClib_MuteControl_P_FullScreenCheck(hChn))
			{
				/* user intended this to be an av session, mute audio until done with sync */
				BDBG_MSG(("[%d]    Audio source synchronized in av session, muting", hChn->iIndex));
				rc = BSYNClib_AudioSource_SetMute(psSource, true);
				if (rc) goto end;
			}
			else
			{
				BDBG_MSG(("[%d] Ignoring audio mute request", hChn->iIndex));
			}
		}
		else
		{
			BDBG_MSG(("[%d]    Audio source not synchronized, pending mute postponed", hChn->iIndex));
		}
	}
	else
	{
		if (bStarted && !hChn->sConfig.sMuteControl.bAllowIncrementalStart)
		{
			BDBG_MSG(("[%d]    No synchronized video sources, audio started, unmuting", hChn->iIndex));
			rc = BSYNClib_AudioSource_SetMute(psSource, false);
			if (rc) goto end;
		}
		else
		{
			BDBG_MSG(("[%d]    No synchronized video sources, pending audio mute postponed", hChn->iIndex));
		}
	}

end:

	return rc;
}

bool BSYNClib_MuteControl_P_FullScreenCheck(BSYNClib_Channel_Handle hChn)
{
	BSYSlib_List_IteratorHandle hIterator;
	BSYNClib_VideoSink * psVideoSink;
	bool bFullScreenPass = true;

	BDBG_ASSERT(hChn);

	if (hChn->hParent->sSettings.sVideo.bRequireFullScreen)
	{
		hIterator = BSYSlib_List_AcquireIterator(hChn->sVideo.hSinks);
		while (BSYSlib_List_HasNext(hIterator))
		{
			psVideoSink = (BSYNClib_VideoSink *)BSYSlib_List_Next(hIterator);

			if (!psVideoSink->sConfig.bFullScreen)/* at mute time, no snapshot has been made */
			{
				BDBG_MSG(("[%d] Full screen check fail", hChn->iIndex));
				bFullScreenPass = false;
				break;
			}
		}
		BSYSlib_List_ReleaseIterator(hIterator);
	}

	return bFullScreenPass;
}

BERR_Code BSYNClib_MuteControl_P_UnmuteAll(BSYNClib_Channel_Handle hChn)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYSlib_List_IteratorHandle hIterator;
	BSYNClib_VideoSource * psVideoSource;
	BSYNClib_AudioSource * psAudioSource;

	BDBG_ASSERT(hChn);

	BDBG_MSG(("[%d] Unmuting all sources", hChn->iIndex));

	hIterator = BSYSlib_List_AcquireIterator(hChn->sVideo.hSources);
	while (BSYSlib_List_HasNext(hIterator))
	{
		psVideoSource = (BSYNClib_VideoSource *)BSYSlib_List_Next(hIterator);

		/* unmute */
		rc = BSYNClib_VideoSource_SetMute(psVideoSource, false);
		if (rc) goto error;
	}
	BSYSlib_List_ReleaseIterator(hIterator);

	hIterator = BSYSlib_List_AcquireIterator(hChn->sAudio.hSources);
	while (BSYSlib_List_HasNext(hIterator))
	{
		psAudioSource = (BSYNClib_AudioSource *)BSYSlib_List_Next(hIterator);

		/* unmute */
		rc = BSYNClib_AudioSource_SetMute(psAudioSource, false);
		if (rc) goto error;
	}
	BSYSlib_List_ReleaseIterator(hIterator);

	goto end;

	error:
	if (hIterator)
	{
		BSYSlib_List_ReleaseIterator(hIterator);
	}

end:

	return rc;
}

BERR_Code BSYNClib_MuteControl_P_VideoSourceUnmuteTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_VideoSource * psSource = pvParm1;
	BSYNClib_Channel_Handle hChn;

	BDBG_ENTER(BSYNClib_MuteControl_P_VideoSourceUnmuteTimerExpired);

	BSTD_UNUSED(iParm2);

	BDBG_ASSERT(psSource);
	BDBG_ASSERT(hTimer);

	hChn = psSource->sElement.hParent;

	/* clean up timer */
	BSYNClib_Channel_P_TimerExpired(hChn, hTimer);

	BDBG_MSG(("[%d] Video source %u unmute timer expired", hChn->iIndex, psSource->sElement.uiIndex));

	if (hChn->sConfig.sMuteControl.bSimultaneousUnmute)
	{
		/* TODO: may want to make this fire on longest timeout, instead of shortest */
		rc = BSYNClib_MuteControl_P_UnmuteAll(hChn);
	}
	else
	{
		rc = BSYNClib_VideoSource_SetMute(psSource, false);
		if (rc) goto end;
	}

end:

	BDBG_LEAVE(BSYNClib_MuteControl_P_VideoSourceUnmuteTimerExpired);
	return rc;
}

BERR_Code BSYNClib_MuteControl_P_AudioSourceUnmuteTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_AudioSource * psSource = pvParm1;
	BSYNClib_Channel_Handle hChn;

	BDBG_ENTER(BSYNClib_MuteControl_P_AudioSourceUnmuteTimerExpired);

	BSTD_UNUSED(iParm2);

	BDBG_ASSERT(psSource);
	BDBG_ASSERT(hTimer);

	hChn = psSource->sElement.hParent;

	BSYNClib_Channel_P_TimerExpired(hChn, hTimer);

	BDBG_MSG(("[%d] Audio source %u unmute timer expired", hChn->iIndex, psSource->sElement.uiIndex));

	if (hChn->sConfig.sMuteControl.bSimultaneousUnmute)
	{
		/* TODO: may want to make this fire on longest timeout, instead of shortest */
		rc = BSYNClib_MuteControl_P_UnmuteAll(hChn);
	}
	else
	{
		/* unmute the audio */
		rc = BSYNClib_AudioSource_SetMute(psSource, false);
	}

	BDBG_LEAVE(BSYNClib_MuteControl_P_AudioSourceUnmuteTimerExpired);
	return rc;
}

#if BSYNCLIB_UNCONDITIONAL_AUDIO_UNMUTE_SUPPORT
BERR_Code BSYNClib_MuteControl_P_AudioSourceUnconditionalUnmuteTimerExpired(void *pvParm1,int iParm2,BSYSlib_Timer_Handle hTimer)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_AudioSource * psSource = pvParm1;
	BSYNClib_Channel_Handle hChn;

	BDBG_ENTER(BSYNClib_MuteControl_P_AudioSourceUnconditionalUnmuteTimerExpired);

	BSTD_UNUSED(iParm2);

	BDBG_ASSERT(psSource);
	BDBG_ASSERT(hTimer);

	hChn = psSource->sElement.hParent;

	BSYNClib_Channel_P_TimerExpired(hChn, hTimer);

	BDBG_WRN(("[%d] Audio source %u unconditional unmute timer expired", hChn->iIndex, psSource->sElement.uiIndex));

	if (hChn->sConfig.sMuteControl.bSimultaneousUnmute)
	{
		/* TODO: may want to make this fire on longest timeout, instead of shortest */
		rc = BSYNClib_MuteControl_P_UnmuteAll(hChn);
	}
	else
	{
		/* unmute the audio */
		rc = BSYNClib_AudioSource_SetMute(psSource, false);
	}

	BDBG_LEAVE(BSYNClib_MuteControl_P_AudioSourceUnconditionalUnmuteTimerExpired);
	return rc;
}
#endif

#if BSYNCLIB_UNCONDITIONAL_VIDEO_UNMUTE_SUPPORT
BERR_Code BSYNClib_MuteControl_P_VideoSourceUnconditionalUnmuteTimerExpired(void *pvParm1,int iParm2,BSYSlib_Timer_Handle hTimer)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_VideoSource * psSource = pvParm1;
	BSYNClib_Channel_Handle hChn;

	BDBG_ENTER(BSYNClib_MuteControl_P_VideoSourceUnconditionalUnmuteTimerExpired);

	BSTD_UNUSED(iParm2);

	BDBG_ASSERT(psSource);
	BDBG_ASSERT(hTimer);

	hChn = psSource->sElement.hParent;

	BSYNClib_Channel_P_TimerExpired(hChn, hTimer);

	BDBG_WRN(("[%d] Video source %u unconditional unmute timer expired", hChn->iIndex, psSource->sElement.uiIndex));

	if (hChn->sConfig.sMuteControl.bSimultaneousUnmute)
	{
		/* TODO: may want to make this fire on longest timeout, instead of shortest */
		rc = BSYNClib_MuteControl_P_UnmuteAll(hChn);
	}
	else
	{
		/* unmute the video */
		rc = BSYNClib_VideoSource_SetMute(psSource, false);
	}

	BDBG_LEAVE(BSYNClib_MuteControl_P_VideoSourceUnconditionalUnmuteTimerExpired);
	return rc;
}
#endif
