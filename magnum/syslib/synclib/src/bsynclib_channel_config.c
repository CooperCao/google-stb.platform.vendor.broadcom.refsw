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
#include "bsyslib.h"
#include "bsynclib.h"
#include "bsynclib_priv.h"
#include "bsynclib_channel_priv.h"
#include "bsynclib_algo.h"
#include "bkni.h"

BDBG_MODULE(synclib);

void BSYNClib_Channel_GetDefaultConfig(
	BSYNClib_Channel_Config * psConfig /* out */
)
{
	BSYNClib_Channel_P_GetDefaultConfig(psConfig);
}

/*
Summary:
Returns the default configuration of a sync channel
 */
void BSYNClib_Channel_P_GetDefaultConfig(BSYNClib_Channel_Config * psConfig)
{
	BDBG_ENTER(BSYNClib_Channel_P_GetDefaultConfig);
	BDBG_ASSERT(psConfig);
	BKNI_Memset(psConfig, 0, sizeof(BSYNClib_Channel_Config));
	psConfig->bEnabled = true;
	psConfig->sMuteControl.bEnabled = true;
	psConfig->bPrecisionLipsyncEnabled = true;
	BDBG_LEAVE(BSYNClib_Channel_P_GetDefaultConfig);
}

/*
Summary:
Returns the current configuration of an SYNC lib channel
Description:
 */
void BSYNClib_Channel_GetConfig(
	BSYNClib_Channel_Handle hChn,
	BSYNClib_Channel_Config * psConfig
)
{
	BDBG_ENTER(BSYNClib_Channel_GetConfig);
	BDBG_ASSERT(hChn);
	BDBG_ASSERT(psConfig);
	*psConfig = hChn->sConfig;
	BDBG_LEAVE(BSYNClib_Channel_GetConfig);
}

BERR_Code BSYNClib_Channel_SetConfig(
	BSYNClib_Channel_Handle hChn,
	const BSYNClib_Channel_Config * psConfig
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ENTER(BSYNClib_Channel_SetConfig);
	BDBG_ASSERT(hChn);
	BDBG_ASSERT(psConfig);
	/* copy config */
	hChn->sConfig = *psConfig;
	rc = BSYNClib_Channel_P_ProcessConfig(hChn);
	BDBG_LEAVE(BSYNClib_Channel_SetConfig);
	return rc;
}

BERR_Code BSYNClib_Channel_P_ProcessConfig(
	BSYNClib_Channel_Handle hChn
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_Channel_Config * psConfig;
	BSYNClib_RateMismatchDetector_Config sRmdConfig;
	BSYSlib_List_IteratorHandle hIterator;
	unsigned int i = 0;
	bool bGenerateInitialCallbacks = false;
	bool bEnabledEdge = false;

	BDBG_ENTER(BSYNClib_Channel_P_ProcessConfig);

	BDBG_ASSERT(hChn);

	psConfig = &hChn->sConfig;

	/* copy enable flag first */
	bEnabledEdge = !hChn->bEnabled && psConfig->bEnabled;
	hChn->bEnabled = psConfig->bEnabled;

	if ((psConfig->uiVideoSourceCount && psConfig->uiVideoSourceCount != hChn->sVideo.uiSourceCount)
		|| (psConfig->uiVideoSinkCount && psConfig->uiVideoSinkCount != hChn->sVideo.uiSinkCount)
		|| (psConfig->uiAudioSourceCount && psConfig->uiAudioSourceCount != hChn->sAudio.uiSourceCount)
		|| (psConfig->uiAudioSinkCount && psConfig->uiAudioSinkCount != hChn->sAudio.uiSinkCount))
	{
		bGenerateInitialCallbacks = true;
	}

	/* video sources */
	if (psConfig->uiVideoSourceCount != hChn->sVideo.uiSourceCount)
	{
		/* take down previous data */
		hIterator = BSYSlib_List_AcquireIterator(hChn->sVideo.hSources);

		while (BSYSlib_List_HasNext(hIterator))
		{
			BSYNClib_VideoSource * psSource;

			psSource = (BSYNClib_VideoSource *)BSYSlib_List_Next(hIterator);
			BSYSlib_List_RemoveElement(hChn->sVideo.hSources, psSource);

			if (psSource->psTsmLockTimer)
			{
				BDBG_MSG(("[%d] Releasing video source %d tsm lock timer", hChn->iIndex, psSource->sElement.uiIndex));
				BSYNClib_ResourcePool_Release(hChn->psTimers, psSource->psTsmLockTimer);
				psSource->psTsmLockTimer = NULL;
			}
			if (psSource->psUnmuteTimer)
			{
				BDBG_MSG(("[%d] Releasing video source %d unmute timer", hChn->iIndex, psSource->sElement.uiIndex));
				BSYNClib_ResourcePool_Release(hChn->psTimers, psSource->psUnmuteTimer);
				psSource->psUnmuteTimer = NULL;
			}

#if BSYNCLIB_UNCONDITIONAL_VIDEO_UNMUTE_SUPPORT
			if (psSource->psUnconditionalUnmuteTimer)
			{
				BDBG_MSG(("[%d] Releasing video source %d unconditional unmute timer", hChn->iIndex, psSource->sElement.uiIndex));
				BSYNClib_ResourcePool_Release(hChn->psTimers, psSource->psUnconditionalUnmuteTimer);
				psSource->psUnconditionalUnmuteTimer = NULL;
			}
#endif

			BSYNClib_VideoSource_Destroy(psSource);
		}

		BSYSlib_List_ReleaseIterator(hIterator);

		/* bring up new data */
		for (i = 0; i < psConfig->uiVideoSourceCount; i++)
		{
			BSYNClib_VideoSource * psSource;
			psSource = BSYNClib_VideoSource_Create();
			if (!psSource) goto error;
			psSource->sElement.hParent = hChn;
			BSYSlib_List_AddElement(hChn->sVideo.hSources, psSource);
			psSource->sElement.uiIndex = i;

			BDBG_MSG(("[%d] Acquiring video source %d tsm lock timer", hChn->iIndex, i));
			psSource->psTsmLockTimer = BSYNClib_ResourcePool_Acquire(hChn->psTimers);

			BDBG_MSG(("[%d] Acquiring video source %d unmute timer", hChn->iIndex, i));
			psSource->psUnmuteTimer = BSYNClib_ResourcePool_Acquire(hChn->psTimers);

#if BSYNCLIB_UNCONDITIONAL_VIDEO_UNMUTE_SUPPORT
			BDBG_MSG(("[%d] Acquiring video source %d unconditional unmute timer", hChn->iIndex, i));
			psSource->psUnconditionalUnmuteTimer = BSYNClib_ResourcePool_Acquire(hChn->psTimers);
#endif

			/* 20151207 bandrews - not everyone uses mute control */
			psSource->sElement.sDelay.sResults.uiDesired =
				BSYNClib_P_Convert_isrsafe(BSYNCLIB_VIDEO_INITIAL_DELAY,
					BSYNClib_Units_eMilliseconds, BSYNClib_Units_e27MhzTicks);
			psSource->sElement.sDelay.sResults.uiApplied = psSource->sElement.sDelay.sResults.uiDesired;

			psSource->sElement.sDelay.sData.ePreferredUnits = hChn->sSettings.sVideo.sSource.cbDelay.preferredDelayUnits;
			psSource->sElement.sNotification.sData.ePreferredUnits = hChn->sSettings.sVideo.sSource.cbDelay.preferredDelayUnits;

			psSource->sElement.sDelay.sResults.bGenerateCallback = true;
			psSource->sElement.sNotification.sResults.bGenerateCallback = true;
		}

		hChn->sVideo.uiSourceCount = psConfig->uiVideoSourceCount;
	}

	/* PR49294 20081125 bandrews - add precision lipsync flag */
	if (psConfig->bPrecisionLipsyncEnabled != hChn->bPrecisionLipsyncEnabled)
	{
		/* copy flag */
		hChn->bPrecisionLipsyncEnabled = psConfig->bPrecisionLipsyncEnabled;

		/* generate callbacks at end of process config */
		bGenerateInitialCallbacks = true;

		hIterator = BSYSlib_List_AcquireIterator(hChn->sVideo.hSources);

		/* enable notification callback on all video sources */
		while (BSYSlib_List_HasNext(hIterator))
		{
			BSYNClib_VideoSource * psSource;

			psSource = (BSYNClib_VideoSource *)BSYSlib_List_Next(hIterator);
			psSource->sElement.sNotification.sResults.bGenerateCallback = true;
			psSource->sElement.sNotification.sResults.bEnabled &= hChn->bPrecisionLipsyncEnabled;
			BKNI_EnterCriticalSection();
			BSYNClib_VideoSource_EstimateDelay_isr(psSource);
			BKNI_LeaveCriticalSection();
		}

		BSYSlib_List_ReleaseIterator(hIterator);
	}

	/* video sinks */
	if (psConfig->uiVideoSinkCount != hChn->sVideo.uiSinkCount)
	{
		/* take down previous */
		hIterator = BSYSlib_List_AcquireIterator(hChn->sVideo.hSinks);

		while (BSYSlib_List_HasNext(hIterator))
		{
			BSYNClib_VideoSink * psSink;

			psSink = (BSYNClib_VideoSink *)BSYSlib_List_Next(hIterator);
			BSYSlib_List_RemoveElement(hChn->sVideo.hSinks, psSink);

			BSYNClib_VideoSink_Destroy(psSink);
		}

		BSYSlib_List_ReleaseIterator(hIterator);

		/* bring up new */
		for (i = 0; i < psConfig->uiVideoSinkCount; i++)
		{
			BSYNClib_VideoSink * psSink;
			psSink = BSYNClib_VideoSink_Create();
			if (!psSink) goto error;
			psSink->sElement.hParent = hChn;
			BSYSlib_List_AddElement(hChn->sVideo.hSinks, psSink);
			psSink->sElement.uiIndex = i;

			psSink->sElement.sDelay.sData.ePreferredUnits = hChn->sSettings.sVideo.sSink.cbDelay.preferredDelayUnits;
			psSink->sElement.sNotification.sData.ePreferredUnits = hChn->sSettings.sVideo.sSink.cbDelay.preferredDelayUnits;

			psSink->sElement.sDelay.sResults.bGenerateCallback = true;
			psSink->sElement.sNotification.sResults.bGenerateCallback = true;
		}

		hChn->sVideo.uiSinkCount = psConfig->uiVideoSinkCount;
	}

	/* RMD */
	BSYNClib_RateMismatchDetector_GetConfig(hChn->hDetector, &sRmdConfig);
	sRmdConfig.uiVideoSourceCount = psConfig->uiVideoSourceCount;
	sRmdConfig.uiVideoSinkCount = psConfig->uiVideoSinkCount;
	BSYNClib_RateMismatchDetector_SetConfig(hChn->hDetector, &sRmdConfig);

	/* audio sources */
	if (psConfig->uiAudioSourceCount != hChn->sAudio.uiSourceCount)
	{
		/* take down previous */
		hIterator = BSYSlib_List_AcquireIterator(hChn->sAudio.hSources);

		while (BSYSlib_List_HasNext(hIterator))
		{
			BSYNClib_AudioSource * psSource;

			psSource = (BSYNClib_AudioSource *)BSYSlib_List_Next(hIterator);
			BSYSlib_List_RemoveElement(hChn->sAudio.hSources, psSource);

			if (psSource->psUnmuteTimer)
			{
				BDBG_MSG(("[%d] Releasing audio source %d unmute timer", hChn->iIndex, psSource->sElement.uiIndex));
				BSYNClib_ResourcePool_Release(hChn->psTimers, psSource->psUnmuteTimer);
				psSource->psUnmuteTimer = NULL;
			}

#if BSYNCLIB_UNCONDITIONAL_AUDIO_UNMUTE_SUPPORT
			if (psSource->psUnconditionalUnmuteTimer)
			{
				BDBG_MSG(("[%d] Releasing audio source %d unconditional unmute timer", hChn->iIndex, psSource->sElement.uiIndex));
				BSYNClib_ResourcePool_Release(hChn->psTimers, psSource->psUnconditionalUnmuteTimer);
				psSource->psUnconditionalUnmuteTimer = NULL;
			}
#endif

			BSYNClib_AudioSource_Destroy(psSource);
		}

		BSYSlib_List_ReleaseIterator(hIterator);

		/* bring up new */
		for (i = 0; i < psConfig->uiAudioSourceCount; i++)
		{
			BSYNClib_AudioSource * psSource;
			psSource = BSYNClib_AudioSource_Create();
			if (!psSource) goto error;
			psSource->sElement.hParent = hChn;
			BSYSlib_List_AddElement(hChn->sAudio.hSources, psSource);
			psSource->sElement.uiIndex = i;

			BDBG_MSG(("[%d] Acquiring audio source %d unmute timer", hChn->iIndex, i));
			psSource->psUnmuteTimer = BSYNClib_ResourcePool_Acquire(hChn->psTimers);

#if BSYNCLIB_UNCONDITIONAL_AUDIO_UNMUTE_SUPPORT
			BDBG_MSG(("[%d] Acquiring audio source %d unconditional unmute timer", hChn->iIndex, i));
			psSource->psUnconditionalUnmuteTimer = BSYNClib_ResourcePool_Acquire(hChn->psTimers);
#endif

			psSource->sElement.sDelay.sData.ePreferredUnits = hChn->sSettings.sAudio.sSource.cbDelay.preferredDelayUnits;
			psSource->sElement.sNotification.sData.ePreferredUnits = hChn->sSettings.sAudio.sSource.cbDelay.preferredDelayUnits;

			psSource->sElement.sDelay.sResults.bGenerateCallback = true;
			psSource->sElement.sNotification.sResults.bGenerateCallback = true;
		}

		hChn->sAudio.uiSourceCount = psConfig->uiAudioSourceCount;
	}

	/* audio sinks */
	if (psConfig->uiAudioSinkCount != hChn->sAudio.uiSinkCount)
	{
		/* take down previous */
		hIterator = BSYSlib_List_AcquireIterator(hChn->sAudio.hSinks);

		while (BSYSlib_List_HasNext(hIterator))
		{
			BSYNClib_AudioSink * psSink;

			psSink = (BSYNClib_AudioSink *)BSYSlib_List_Next(hIterator);
			BSYSlib_List_RemoveElement(hChn->sAudio.hSinks, psSink);

			BSYNClib_AudioSink_Destroy(psSink);
		}

		BSYSlib_List_ReleaseIterator(hIterator);

		/* bring up new */
		for (i = 0; i < psConfig->uiAudioSinkCount; i++)
		{
			BSYNClib_AudioSink * psSink;
			psSink = BSYNClib_AudioSink_Create();
			if (!psSink) goto error;
			psSink->sElement.hParent = hChn;
			BSYSlib_List_AddElement(hChn->sAudio.hSinks, psSink);
			psSink->sElement.uiIndex = i;

			psSink->sElement.sDelay.sData.ePreferredUnits = hChn->sSettings.sAudio.sSink.cbDelay.preferredDelayUnits;
			psSink->sElement.sNotification.sData.ePreferredUnits = hChn->sSettings.sAudio.sSink.cbDelay.preferredDelayUnits;

			psSink->sElement.sDelay.sResults.bGenerateCallback = true;
			psSink->sElement.sNotification.sResults.bGenerateCallback = true;
		}

		hChn->sAudio.uiSinkCount = psConfig->uiAudioSinkCount;
	}

	if (bGenerateInitialCallbacks)
	{
		BDBG_MSG(("[%d] Generating initial state callbacks", hChn->iIndex));
		BSYNClib_Channel_P_GenerateCallbacks(hChn);
	}

	if (bEnabledEdge)
	{
		BKNI_EnterCriticalSection();
		rc = BSYNClib_Channel_P_DequeueTaskRequest_isr(hChn);
		BKNI_LeaveCriticalSection();
	}

	goto end;

	error:

	end:

	BDBG_LEAVE(BSYNClib_Channel_P_ProcessConfig);
	return rc;
}


void BSYNClib_Channel_GetVideoSourceConfig_isr
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	BSYNClib_VideoSource_Config * psConfig /* [out] */
)
{
	BDBG_ENTER(BSYNClib_Channel_GetVideoSourceConfig_isr);

	BDBG_ASSERT(hChn);

	if (BSYNClib_P_Enabled_isrsafe(hChn->hParent))
	{
		BSYNClib_VideoSource * psSource;

		psSource = (BSYNClib_VideoSource *)BSYSlib_List_GetByIndex_isr(hChn->sVideo.hSources, uiSource);

		if (psSource)
		{
			/* copy config */
			BDBG_ASSERT(psConfig);
			*psConfig = psSource->sConfig;
		}
	}

	BDBG_LEAVE(BSYNClib_Channel_GetVideoSourceConfig_isr);
}

BERR_Code BSYNClib_Channel_SetVideoSourceConfig_isr
(
	BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	const BSYNClib_VideoSource_Config * psConfig
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BSYNClib_Channel_SetVideoSourceConfig_isr);

	BDBG_ASSERT(hChn);

	if (BSYNClib_P_Enabled_isrsafe(hChn->hParent))
	{
		BSYNClib_VideoSource * psSource;

		psSource = (BSYNClib_VideoSource *)BSYSlib_List_GetByIndex_isr(hChn->sVideo.hSources, uiSource);

		if (psSource)
		{
			/* copy config */
			BDBG_ASSERT(psConfig);
			psSource->sConfig = *psConfig;
			rc = BSYNClib_VideoSource_P_ProcessConfig_isr(psSource);
			BSYNClib_VideoSource_P_SelfClearConfig_isr(psSource);
		}
	}

	BDBG_LEAVE(BSYNClib_Channel_SetVideoSourceConfig_isr);
	return rc;
}

void BSYNClib_Channel_GetDefaultVideoSourceConfig
(
	BSYNClib_VideoSource_Config * psConfig /* [out] */
)
{
	BSYNClib_VideoSource_GetDefaultConfig(psConfig);
}

void BSYNClib_Channel_GetVideoSinkConfig_isr
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	BSYNClib_VideoSink_Config * psConfig /* [out] */
)
{
	BDBG_ENTER(BSYNClib_Channel_GetVideoSinkConfig_isr);

	BDBG_ASSERT(hChn);

	if (BSYNClib_P_Enabled_isrsafe(hChn->hParent))
	{
		BSYNClib_VideoSink * psSink;

		psSink = (BSYNClib_VideoSink *)BSYSlib_List_GetByIndex_isr(hChn->sVideo.hSinks, uiSink);

		if (psSink)
		{
			/* copy config */
			BDBG_ASSERT(psConfig);
			*psConfig = psSink->sConfig;
		}
	}

	BDBG_LEAVE(BSYNClib_Channel_GetVideoSinkConfig_isr);
}

BERR_Code BSYNClib_Channel_SetVideoSinkConfig_isr
(
	BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	const BSYNClib_VideoSink_Config * psConfig
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BSYNClib_Channel_SetVideoSinkConfig_isr);

	BDBG_ASSERT(hChn);

	if (BSYNClib_P_Enabled_isrsafe(hChn->hParent))
	{
		BSYNClib_VideoSink * psSink;

		psSink = (BSYNClib_VideoSink *)BSYSlib_List_GetByIndex_isr(hChn->sVideo.hSinks, uiSink);

		if (psSink)
		{
			/* copy config */
			BDBG_ASSERT(psConfig);
			psSink->sConfig = *psConfig;
			rc = BSYNClib_VideoSink_P_ProcessConfig_isr(psSink);
			BSYNClib_VideoSink_P_SelfClearConfig_isr(psSink);
		}
	}

	BDBG_LEAVE(BSYNClib_Channel_SetVideoSinkConfig_isr);
	return rc;
}


void BSYNClib_Channel_GetAudioSourceConfig_isr
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	BSYNClib_AudioSource_Config * psConfig /* [out] */
)
{
	BDBG_ENTER(BSYNClib_Channel_GetAudioSourceConfig_isr);

	BDBG_ASSERT(hChn);

	if (BSYNClib_P_Enabled_isrsafe(hChn->hParent))
	{
		BSYNClib_AudioSource * psSource;

		psSource = (BSYNClib_AudioSource *)BSYSlib_List_GetByIndex_isr(hChn->sAudio.hSources, uiSource);

		if (psSource)
		{
			/* copy config */
			BDBG_ASSERT(psConfig);
			*psConfig = psSource->sConfig;
		}
	}

	BDBG_LEAVE(BSYNClib_Channel_GetAudioSourceConfig_isr);
}

BERR_Code BSYNClib_Channel_SetAudioSourceConfig_isr
(
	BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	const BSYNClib_AudioSource_Config * psConfig
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BSYNClib_Channel_SetAudioSourceConfig_isr);

	BDBG_ASSERT(hChn);

	if (BSYNClib_P_Enabled_isrsafe(hChn->hParent))
	{
		BSYNClib_AudioSource * psSource;

		psSource = (BSYNClib_AudioSource *)BSYSlib_List_GetByIndex_isr(hChn->sAudio.hSources, uiSource);

		if (psSource)
		{
			/* copy config */
			BDBG_ASSERT(psConfig);
			psSource->sConfig = *psConfig;
			rc = BSYNClib_AudioSource_P_ProcessConfig_isr(psSource);
			BSYNClib_AudioSource_P_SelfClearConfig_isr(psSource);
		}
	}

	BDBG_LEAVE(BSYNClib_Channel_SetAudioSourceConfig_isr);
	return rc;
}

void BSYNClib_Channel_GetAudioSinkConfig_isr
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	BSYNClib_AudioSink_Config * psConfig /* [out] */
)
{
	BDBG_ENTER(BSYNClib_Channel_GetAudioSinkConfig_isr);

	BDBG_ASSERT(hChn);

	if (BSYNClib_P_Enabled_isrsafe(hChn->hParent))
	{
		BSYNClib_AudioSink * psSink;

		psSink = (BSYNClib_AudioSink *)BSYSlib_List_GetByIndex_isr(hChn->sAudio.hSinks, uiSink);

		if (psSink)
		{
			/* copy config */
			BDBG_ASSERT(psConfig);
			*psConfig = psSink->sConfig;
		}
	}

	BDBG_LEAVE(BSYNClib_Channel_GetAudioSinkConfig_isr);
}

BERR_Code BSYNClib_Channel_SetAudioSinkConfig_isr
(
	BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	const BSYNClib_AudioSink_Config * psConfig
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BSYNClib_Channel_SetAudioSinkConfig_isr);

	BDBG_ASSERT(hChn);

	if (BSYNClib_P_Enabled_isrsafe(hChn->hParent))
	{
		BSYNClib_AudioSink * psSink;

		psSink = (BSYNClib_AudioSink *)BSYSlib_List_GetByIndex_isr(hChn->sAudio.hSinks, uiSink);

		if (psSink)
		{
			/* copy config */
			BDBG_ASSERT(psConfig);
			psSink->sConfig = *psConfig;
			rc = BSYNClib_AudioSink_P_ProcessConfig_isr(psSink);
			BSYNClib_AudioSink_P_SelfClearConfig_isr(psSink);
		}
	}

	BDBG_LEAVE(BSYNClib_Channel_SetAudioSinkConfig_isr);
	return rc;
}

