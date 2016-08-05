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
#include "bsynclib_algo.h"
#include "bsynclib_priv.h"
#include "bsynclib_channel_priv.h"

BDBG_MODULE(synclib);

void BSYNClib_Algo_AudioVideo_Sync(BSYNClib_Channel_Path * psAudio, BSYNClib_Channel_Path * psVideo, BSYNClib_Channel_Results * psResults)
{
	BDBG_ENTER((BSYNClib_Algo_AudioVideo_Sync));

	BDBG_MSG(("[%d] Attempting to synchronize audio with video...", psAudio->hChn->iIndex));

	BSYNClib_Algo_VideoSink_MaxFinder(psVideo);
	BSYNClib_Algo_VideoSource_MaxFinder(psVideo);
	BSYNClib_Algo_AudioSink_MaxFinder(psAudio);
	BSYNClib_Algo_AudioSource_MaxFinder(psAudio);
	BSYNClib_Algo_AudioVideo_MaxFinder(psAudio, psVideo, psResults);
	BSYNClib_Algo_AudioVideo_Allocator(psAudio, psVideo, psResults);

	BDBG_LEAVE((BSYNClib_Algo_AudioVideo_Sync));
}

void BSYNClib_Algo_VideoVideo_Sync(BSYNClib_Channel_Path * psPath)
{
	BDBG_ENTER((BSYNClib_Algo_VideoVideo_Sync));

	BDBG_MSG(("[%d] Attempting to synchronize all isocontent video paths...", psPath->hChn->iIndex));

	BSYNClib_Algo_VideoSink_MaxFinder(psPath);
	BSYNClib_Algo_VideoSource_MaxFinder(psPath);
	BSYNClib_Algo_VideoVideo_Allocator(psPath);

	BDBG_LEAVE((BSYNClib_Algo_VideoVideo_Sync));
}


void BSYNClib_Algo_AudioAudio_Sync(BSYNClib_Channel_Path * psPath)
{
	BDBG_ENTER((BSYNClib_Algo_AudioAudio_Sync));

	BDBG_MSG(("[%d] Attempting to synchronize all isocontent audio paths...", psPath->hChn->iIndex));

	BSYNClib_Algo_AudioSink_MaxFinder(psPath);
	BSYNClib_Algo_AudioSource_MaxFinder(psPath);
	BSYNClib_Algo_AudioAudio_Allocator(psPath);

	BDBG_LEAVE((BSYNClib_Algo_AudioAudio_Sync));
}

void BSYNClib_Algo_VideoVideo_Allocator(BSYNClib_Channel_Path * psPath)
{
	BSYSlib_List_IteratorHandle hIterator;

	BDBG_ENTER((BSYNClib_Algo_VideoVideo_Allocator));

	BDBG_ASSERT(psPath);

	/* TODO: this only allocates for sinks, need to handle multiple sources */
	/* allocate difference between max and current to all windows */

	hIterator = BSYSlib_List_AcquireIterator(psPath->hSinks);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_VideoSink * psSink;
		unsigned int delay_diff;

		psSink = (BSYNClib_VideoSink *)BSYSlib_List_Next(hIterator);

		if (psSink->sElement.sSnapshot.bSynchronize)
		{
			if (psPath->sResults.bInconsistentSinkDomains)
			{
				delay_diff = 0;
				BDBG_MSG(("[%d]  Allocating video sink %u delay: %u ms",
					psPath->hChn->iIndex, psSink->sElement.uiIndex,
					BSYNClib_P_Convert_isrsafe(delay_diff, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
				psSink->sElement.sDelay.sResults.uiDesired = delay_diff;
			}
			else
			{
				/* PR21616 20061128 bandrews - don't bother applying delays to hidden windows, like PiP */
				if (psSink->sSnapshot.bVisible)
				{
					delay_diff = psPath->sResults.uiMaxSinkDelay - psSink->sElement.sDelay.sSnapshot.uiMeasured;
				}
				else /* hidden windows get no delay */
				{
					delay_diff = 0;
				}

				/* 20081006 PR44510 bandrews */
				if (delay_diff <= psSink->sElement.sDelay.sData.uiCapacity)
				{
					BDBG_MSG(("[%d]  Allocating video sink %u delay: %u ms",
						psPath->hChn->iIndex, psSink->sElement.uiIndex,
						BSYNClib_P_Convert_isrsafe(delay_diff, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
					psSink->sElement.sDelay.sResults.uiDesired = delay_diff;
				}
				else
				{
					BDBG_WRN(("[%d]  Delay capacity reached for video sink %u; requested delay: %u ms, capacity: %u ms",
						psPath->hChn->iIndex,
						psSink->sElement.uiIndex,
						BSYNClib_P_Convert_isrsafe(delay_diff, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds),
						BSYNClib_P_Convert_isrsafe(psSink->sElement.sDelay.sData.uiCapacity, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
					delay_diff = psSink->sElement.sDelay.sData.uiCapacity;
					BDBG_MSG(("[%d]  Allocating video sink %u delay: %u ms",
						psPath->hChn->iIndex, psSink->sElement.uiIndex,
						BSYNClib_P_Convert_isrsafe(delay_diff, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
					psSink->sElement.sDelay.sResults.uiDesired = delay_diff;
				}
			}
		}
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	BDBG_LEAVE((BSYNClib_Algo_VideoVideo_Allocator));
}

void BSYNClib_Algo_AudioAudio_Allocator(BSYNClib_Channel_Path * psPath)
{
	BSYSlib_List_IteratorHandle hIterator;

	BDBG_ENTER((BSYNClib_Algo_AudioAudio_Allocator));

	BDBG_ASSERT(psPath);

	/* TODO: sources? */

	hIterator = BSYSlib_List_AcquireIterator(psPath->hSinks);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_AudioSink * psSink;
		unsigned int uiDelayDiff;

		psSink = (BSYNClib_AudioSink *)BSYSlib_List_Next(hIterator);

		if (psSink->sElement.sSnapshot.bSynchronize)
		{
			uiDelayDiff = psSink->sElement.sDelay.sSnapshot.uiMeasured - psPath->sResults.uiMaxSinkDelay;

			BDBG_MSG(("[%d]  Allocating audio sink %u delay: %u ms",
				psPath->hChn->iIndex, psSink->sElement.uiIndex,
				BSYNClib_P_Convert_isrsafe(uiDelayDiff, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
			psSink->sElement.sDelay.sResults.uiDesired = uiDelayDiff;
		}
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	BDBG_LEAVE((BSYNClib_Algo_AudioAudio_Allocator));
}

void BSYNClib_Algo_AudioVideo_Allocator(BSYNClib_Channel_Path * psAudio, BSYNClib_Channel_Path * psVideo, BSYNClib_Channel_Results * psResults)
{
	BSYSlib_List_IteratorHandle hIterator;
	unsigned int uiDelayDiff;
	unsigned int uiAdditionalAudioDelay;
#if BSYNCLIB_JITTER_TOLERANCE_IMPROVEMENT_SUPPORT
	unsigned int uiAdditionalVideoDelay;
	int iJtiFactor;
#endif

	BDBG_ENTER((BSYNClib_Algo_AudioVideo_Allocator));

	BDBG_ASSERT(psAudio);
	BDBG_ASSERT(psVideo);
	BDBG_ASSERT(psResults);

	uiAdditionalAudioDelay = 0;

	/* video first because of requantization */
	/* TODO: multiple video sources? */
	hIterator = BSYSlib_List_AcquireIterator(psVideo->hSources);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_VideoSource * psSource;

		psSource = (BSYNClib_VideoSource *)BSYSlib_List_Next(hIterator);

		if (psSource->sElement.sSnapshot.bSynchronize)
		{
			unsigned int uiSourceFramePeriod;
			unsigned int uiRequantizedVideoDelay;

			uiDelayDiff = psResults->uiMaxPathDelay - psVideo->sResults.uiMaxPathDelay;

			if (BSYNClib_P_Convert_isrsafe(uiDelayDiff, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)
				< BSYNCLIB_ALGO_MAX_SUPPORTED_DELAY_MS)
			{
				/* Requantize */
				uiSourceFramePeriod = BSYNClib_VideoFormat_P_GetFramePeriod_isrsafe(&psSource->sFormat);
				BSYNClib_Algo_RequantizeDelay(psVideo->hChn->iIndex, uiDelayDiff, uiSourceFramePeriod, &uiRequantizedVideoDelay);
				uiAdditionalAudioDelay = uiRequantizedVideoDelay - uiDelayDiff;
				psSource->sElement.sDelay.sResults.uiDesired = uiRequantizedVideoDelay;

#if BSYNCLIB_JITTER_TOLERANCE_IMPROVEMENT_SUPPORT
				/* JTI */
				BSYNClib_Algo_CalculateJitterToleranceImprovementFactor(psVideo, psSource, uiRequantizedVideoDelay, &iJtiFactor, &uiAdditionalVideoDelay);
				psSource->sElement.sDelay.sResults.uiDesired += uiAdditionalVideoDelay;
				psSource->sElement.sDelay.sResults.uiDesired += iJtiFactor;
#endif

				BDBG_MSG(("[%d]  Allocating video source %u delay: %u ms",
					psVideo->hChn->iIndex, psSource->sElement.uiIndex,
					BSYNClib_P_Convert_isrsafe(psSource->sElement.sDelay.sResults.uiDesired, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
			}
			else
			{
				BDBG_MSG(("[%d]  Refusing to compensate video source %u delay: %u ms",
					psVideo->hChn->iIndex, psSource->sElement.uiIndex,
					BSYNClib_P_Convert_isrsafe(uiDelayDiff, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
				BDBG_MSG(("[%d]  Using previous video source %u delay: %u ms",
					psVideo->hChn->iIndex, psSource->sElement.uiIndex,
					BSYNClib_P_Convert_isrsafe(psSource->sElement.sDelay.sResults.uiDesired, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
			}
		}
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	hIterator = BSYSlib_List_AcquireIterator(psAudio->hSources);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_AudioSource * psSource;

		psSource = (BSYNClib_AudioSource *)BSYSlib_List_Next(hIterator);

		if (psSource->sElement.sSnapshot.bSynchronize)
		{
			uiDelayDiff = psResults->uiMaxPathDelay - psAudio->sResults.uiMaxPathDelay + uiAdditionalAudioDelay;
			if (BSYNClib_P_Convert_isrsafe(uiDelayDiff, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)
				< BSYNCLIB_ALGO_MAX_SUPPORTED_DELAY_MS)
			{
				BDBG_MSG(("[%d]  Allocating audio source %u delay: %u ms",
					psAudio->hChn->iIndex, psSource->sElement.uiIndex,
					BSYNClib_P_Convert_isrsafe(uiDelayDiff, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
				psSource->sElement.sDelay.sResults.uiDesired = uiDelayDiff;
			}
			else
			{
				BDBG_MSG(("[%d]  Refusing to compensate audio source %u delay: %u ms",
					psAudio->hChn->iIndex, psSource->sElement.uiIndex,
					BSYNClib_P_Convert_isrsafe(uiDelayDiff, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
				BDBG_MSG(("[%d]  Using previous audio source %u delay: %u ms",
					psAudio->hChn->iIndex, psSource->sElement.uiIndex,
					BSYNClib_P_Convert_isrsafe(psSource->sElement.sDelay.sResults.uiDesired, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
			}
		}
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	BDBG_LEAVE((BSYNClib_Algo_AudioVideo_Allocator));
}

void BSYNClib_Algo_AudioVideo_MaxFinder(BSYNClib_Channel_Path * psAudio, BSYNClib_Channel_Path * psVideo, BSYNClib_Channel_Results * psResults)
{
	unsigned int uiMaxPathDelay = 0;
	BSYNClib_Channel_Path * psMaxDelayPath;

	BDBG_ENTER((BSYNClib_Algo_AudioVideo_MaxFinder));

	BDBG_ASSERT(psAudio);
	BDBG_ASSERT(psVideo);
	BDBG_ASSERT(psResults);

	psAudio->sResults.uiMaxPathDelay = psAudio->sResults.uiMaxSinkDelay + psAudio->sResults.uiMaxSourceDelay;
	psVideo->sResults.uiMaxPathDelay = psVideo->sResults.uiMaxSinkDelay + psVideo->sResults.uiMaxSourceDelay;

	BDBG_MSG(("[%d]  Max audio path delay is %u ms", psAudio->hChn->iIndex,
		BSYNClib_P_Convert_isrsafe(psAudio->sResults.uiMaxPathDelay, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
	BDBG_MSG(("[%d]  Max video path delay is %u ms", psVideo->hChn->iIndex,
		BSYNClib_P_Convert_isrsafe(psVideo->sResults.uiMaxPathDelay, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));

	if (psVideo->sResults.uiMaxPathDelay > psAudio->sResults.uiMaxPathDelay)
	{
		uiMaxPathDelay = psVideo->sResults.uiMaxPathDelay;
		psMaxDelayPath = psVideo;
	}
	else
	{
		uiMaxPathDelay = psAudio->sResults.uiMaxPathDelay;
		psMaxDelayPath = psAudio;
	}

	/* keep max and window index for later */
	psResults->uiMaxPathDelay = uiMaxPathDelay;
	psResults->psMaxDelayPath = psMaxDelayPath;
	BDBG_MSG(("[%d]  Max path delay is %u ms from %s", psAudio->hChn->iIndex,
		BSYNClib_P_Convert_isrsafe(psResults->uiMaxPathDelay, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds),
		psResults->psMaxDelayPath == psAudio ? "audio" : "video"));

	BDBG_LEAVE((BSYNClib_Algo_AudioVideo_MaxFinder));
}

void BSYNClib_Algo_VideoSink_MaxFinder(BSYNClib_Channel_Path * psPath)
{
	BSYSlib_List_IteratorHandle hIterator;
	int iMaxSinkDelay = -1;
	BSYNClib_VideoSink * psMaxDelaySink = NULL;

	BDBG_ENTER((BSYNClib_Algo_VideoSink_MaxFinder));

	BDBG_ASSERT(psPath);

	/* find max sink delay and the index of the sink that has it */
	hIterator = BSYSlib_List_AcquireIterator(psPath->hSinks);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_VideoSink * psSink;
		unsigned int uiSinkDelay;

		psSink = (BSYNClib_VideoSink *)BSYSlib_List_Next(hIterator);

		/* if not visible or not synchronized, ignore */
		if (!psSink->sSnapshot.bVisible || !psSink->sElement.sSnapshot.bSynchronize)
		{
			BDBG_MSG(("[%d]  Window max delay finder -- Ignoring invisible or unsynchronized window %u", psPath->hChn->iIndex, psSink->sElement.uiIndex));
			continue;
		}

		uiSinkDelay = psSink->sElement.sDelay.sSnapshot.uiMeasured;

		BDBG_MSG(("[%d]  Current video sink delay, sink %u: %u ms",
			psPath->hChn->iIndex, psSink->sElement.uiIndex,
			BSYNClib_P_Convert_isrsafe(uiSinkDelay, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
		if ((signed)uiSinkDelay > iMaxSinkDelay)
		{
			iMaxSinkDelay = uiSinkDelay;
			psMaxDelaySink = psSink;
		}

		/* TODO: this only does measured delay
		need to find max of max delays between all sinks for 7038 */
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	if (psMaxDelaySink)
	{
		/* keep max and window index for later */
		psPath->sResults.uiMaxSinkDelay = iMaxSinkDelay;
		psPath->sResults.pvMaxDelaySink = psMaxDelaySink;
		BDBG_MSG(("[%d]  Max video postprocessing delay is %u ms from video sink %u",
			psPath->hChn->iIndex,
			BSYNClib_P_Convert_isrsafe(psPath->sResults.uiMaxSinkDelay, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds),
			((BSYNClib_VideoSink *)(psPath->sResults.pvMaxDelaySink))->sElement.uiIndex));
	}
	else
	{
		BDBG_MSG(("[%d]  No synchronized video windows.  Setting max window delay to 0.", psPath->hChn->iIndex));
		psPath->sResults.uiMaxSinkDelay = 0;
		psPath->sResults.pvMaxDelaySink = NULL;
	}

	BDBG_LEAVE((BSYNClib_Algo_VideoSink_MaxFinder));
}


void BSYNClib_Algo_VideoSource_MaxFinder(BSYNClib_Channel_Path * psPath)
{
	BSYSlib_List_IteratorHandle hIterator;
	int iMaxSourceDelay = -1;
	BSYNClib_VideoSource * psMaxDelaySource = NULL;

	BDBG_ENTER((BSYNClib_Algo_VideoSource_MaxFinder));

	BDBG_ASSERT(psPath);

	/* find max source delay and the index of the source that has it */
	hIterator = BSYSlib_List_AcquireIterator(psPath->hSources);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_VideoSource * psSource;
		unsigned int uiSourceDelay;

		psSource = (BSYNClib_VideoSource *)BSYSlib_List_Next(hIterator);

		/* if not synchronized, ignore */
		if (!psSource->sElement.sSnapshot.bSynchronize)
		{
			BDBG_MSG(("[%d]  Video source max delay finder -- Ignoring unsynchronized source %u", psPath->hChn->iIndex, psSource->sElement.uiIndex));
			continue;
		}

		/* PR42265 20080616 bandrews - need to include custom delay in measured, 
		otherwise, sync won't compensate audio for this delay */
		uiSourceDelay = psSource->sElement.sDelay.sSnapshot.uiMeasured + psSource->sElement.sDelay.sSnapshot.uiCustom;

		BDBG_MSG(("[%d]  Current video source delay, source %u: %u ms %s",
			psPath->hChn->iIndex, psSource->sElement.uiIndex,
			BSYNClib_P_Convert_isrsafe(uiSourceDelay, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds),
			psSource->sElement.sDelay.sResults.bEstimated ? "estimated" : "measured"));
		if ((signed)uiSourceDelay > iMaxSourceDelay)
		{
			iMaxSourceDelay = uiSourceDelay;
			psMaxDelaySource = psSource;
		}
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	if (psMaxDelaySource)
	{
		/* keep max and decoder index for later */
		psPath->sResults.uiMaxSourceDelay = iMaxSourceDelay;
		psPath->sResults.pvMaxDelaySource = psMaxDelaySource;
		BDBG_MSG(("[%d]  Max video decoder delay is %u ms from video source %u", 
			psPath->hChn->iIndex, 
			BSYNClib_P_Convert_isrsafe(psPath->sResults.uiMaxSourceDelay, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds),
			((BSYNClib_VideoSource *)(psPath->sResults.pvMaxDelaySource))->sElement.uiIndex));
	}
	else
	{
		BDBG_MSG(("[%d]  No synchronized video decoders.  Setting max decoder delay to 0.", psPath->hChn->iIndex));
		psPath->sResults.uiMaxSourceDelay = 0;
		psPath->sResults.pvMaxDelaySource = NULL;
	}

	BDBG_LEAVE((BSYNClib_Algo_VideoSource_MaxFinder));
}

void BSYNClib_Algo_AudioSink_MaxFinder(BSYNClib_Channel_Path * psPath)
{
	BSYSlib_List_IteratorHandle hIterator;
	int iMaxSinkDelay = -1;
	BSYNClib_AudioSink * psMaxDelaySink = NULL;

	BDBG_ENTER((BSYNClib_Algo_AudioSink_MaxFinder));

	BDBG_ASSERT(psPath);

	/* find max sink delay and the index of the sink that has it */
	hIterator = BSYSlib_List_AcquireIterator(psPath->hSinks);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_AudioSink * psSink;
		unsigned int uiSinkDelay;

		psSink = (BSYNClib_AudioSink *)BSYSlib_List_Next(hIterator);

		/* if not synchronized, ignore */
		if (!psSink->sElement.sSnapshot.bSynchronize)
		{
			BDBG_MSG(("[%d]  Audio output max delay finder -- Ignoring unsynchronized output %u", psPath->hChn->iIndex, psSink->sElement.uiIndex));
			continue;
		}

		uiSinkDelay = psSink->sElement.sDelay.sSnapshot.uiMeasured;

		BDBG_MSG(("[%d]  Current audio sink delay, sink %u: %u ms", psPath->hChn->iIndex, psSink->sElement.uiIndex, 
			BSYNClib_P_Convert_isrsafe(uiSinkDelay, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
		if ((signed)uiSinkDelay > iMaxSinkDelay)
		{
			iMaxSinkDelay = uiSinkDelay;
			psMaxDelaySink = psSink;
		}
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	if (psMaxDelaySink)
	{
		/* keep max and output index for later */
		psPath->sResults.uiMaxSinkDelay = iMaxSinkDelay;
		psPath->sResults.pvMaxDelaySink = psMaxDelaySink;
		BDBG_MSG(("[%d]  Max audio output delay is %u ms from audio sink %u", 
			psPath->hChn->iIndex, 
			BSYNClib_P_Convert_isrsafe(psPath->sResults.uiMaxSinkDelay, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds),
			((BSYNClib_AudioSink *)(psPath->sResults.pvMaxDelaySink))->sElement.uiIndex));
	}
	else
	{
		BDBG_MSG(("[%d]  No synchronized audio outputs.  Setting max output delay to 0.", psPath->hChn->iIndex));
		psPath->sResults.uiMaxSinkDelay = 0;
		psPath->sResults.pvMaxDelaySink = NULL;
	}

	BDBG_LEAVE((BSYNClib_Algo_AudioSink_MaxFinder));
}

void BSYNClib_Algo_AudioSource_MaxFinder(BSYNClib_Channel_Path * psPath)
{
	BSYSlib_List_IteratorHandle hIterator;
	int iMaxSourceDelay = -1;
	BSYNClib_AudioSource * psMaxDelaySource = NULL;

	BDBG_ENTER((BSYNClib_Algo_AudioSource_MaxFinder));

	BDBG_ASSERT(psPath);

	/* find max source delay and the index of the source that has it */
	hIterator = BSYSlib_List_AcquireIterator(psPath->hSources);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_AudioSource * psSource;
		unsigned int uiSourceDelay;

		psSource = (BSYNClib_AudioSource *)BSYSlib_List_Next(hIterator);

		/* if not synchronized, ignore */
		if (!psSource->sElement.sSnapshot.bSynchronize)
		{
			BDBG_MSG(("[%d]  Audio source max delay finder -- Ignoring unsynchronized source %u", psPath->hChn->iIndex, psSource->sElement.uiIndex));
			continue;
		}

		uiSourceDelay = psSource->sElement.sDelay.sSnapshot.uiMeasured + psSource->sElement.sDelay.sSnapshot.uiCustom;

		BDBG_MSG(("[%d]  Current audio source delay, source %u: %u ms",
			psPath->hChn->iIndex, psSource->sElement.uiIndex,
			BSYNClib_P_Convert_isrsafe(uiSourceDelay, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
		if ((signed)uiSourceDelay > iMaxSourceDelay)
		{
			iMaxSourceDelay = uiSourceDelay;
			psMaxDelaySource = psSource;
		}
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	if (psMaxDelaySource)
	{
		/* keep max and decoder index for later */
		psPath->sResults.uiMaxSourceDelay = iMaxSourceDelay;
		psPath->sResults.pvMaxDelaySource = psMaxDelaySource;
		BDBG_MSG(("[%d]  Max audio decoder delay is %u ms from audio source %u", 
			psPath->hChn->iIndex, 
			BSYNClib_P_Convert_isrsafe(psPath->sResults.uiMaxSourceDelay, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds),
			((BSYNClib_AudioSource *)(psPath->sResults.pvMaxDelaySource))->sElement.uiIndex));
	}
	else
	{
		BDBG_MSG(("[%d]  No synchronized audio decoders.  Setting max decoder delay to 0.", psPath->hChn->iIndex));
		psPath->sResults.uiMaxSourceDelay = 0;
		psPath->sResults.pvMaxDelaySource = NULL;
	}

	BDBG_LEAVE((BSYNClib_Algo_AudioSource_MaxFinder));
}

BERR_Code BSYNClib_Algo_VideoSink_Applicator(BSYNClib_Channel_Path * psPath, BSYNClib_Channel_SetDelay pfSetDelay, void * pvParm1, int iParm2)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER((BSYNClib_Algo_VideoSink_Applicator));

	BDBG_ASSERT(psPath);

	if (pfSetDelay)
	{
		BSYSlib_List_IteratorHandle hIterator;

		hIterator = BSYSlib_List_AcquireIterator(psPath->hSinks);

		/* release all extra delay buffers first, to reduce memory required during transitions */
		while (BSYSlib_List_HasNext(hIterator))
		{
			BSYNClib_VideoSink * psSink;

			psSink = (BSYNClib_VideoSink *)BSYSlib_List_Next(hIterator);

			if (psSink->sElement.sSnapshot.bSynchronize)
			{
				/* PR48637 bandrews - do not zero out delay if same as last time */
				if (psSink->sElement.sDelay.sResults.uiApplied != psSink->sElement.sDelay.sResults.uiDesired)
				{
					BSYNClib_UnsignedValue sDelay;
					BDBG_MSG(("[%d]  Releasing %u ms of delay on video sink %u", 
						psPath->hChn->iIndex, 
						BSYNClib_P_Convert_isrsafe(psSink->sElement.sDelay.sResults.uiApplied, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds),
						psSink->sElement.uiIndex));

					sDelay.uiValue = 0;
					sDelay.eUnits = psSink->sElement.sDelay.sSnapshot.ePreferredUnits;

					rc = pfSetDelay(pvParm1, iParm2, psSink->sElement.uiIndex, &sDelay);
					if (rc) goto error;

					psSink->sStatus.sAppliedDelay.uiValue = 0;
				}
			}
		}

		/* start at beginning of list again */
		BSYSlib_List_ResetIterator(hIterator);

		/* apply desired */
		while (BSYSlib_List_HasNext(hIterator))
		{
			BSYNClib_VideoSink * psSink;

			psSink = (BSYNClib_VideoSink *)BSYSlib_List_Next(hIterator);

			if (psSink->sElement.sSnapshot.bSynchronize)
			{
				/* PR48637 bandrews - do not re-apply delay if same as last time */
				if (psSink->sElement.sDelay.sResults.uiApplied != psSink->sElement.sDelay.sResults.uiDesired)
				{
					BSYNClib_UnsignedValue sDesired;

					BDBG_MSG(("[%d]  Applying video sink %u delay: %u ms", psPath->hChn->iIndex, psSink->sElement.uiIndex, 
						BSYNClib_P_Convert_isrsafe(psSink->sElement.sDelay.sResults.uiDesired, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));

					sDesired.uiValue = BSYNClib_P_Convert_isrsafe(
						psSink->sElement.sDelay.sResults.uiDesired,
						BSYNClib_Units_e27MhzTicks,
						psSink->sElement.sDelay.sSnapshot.ePreferredUnits);
					sDesired.eUnits = psSink->sElement.sDelay.sSnapshot.ePreferredUnits;

					rc = pfSetDelay(pvParm1, iParm2, psSink->sElement.uiIndex, &sDesired);
					if (rc) goto error;

					psSink->sElement.sDelay.sResults.uiApplied = psSink->sElement.sDelay.sResults.uiDesired;
					psSink->sStatus.sAppliedDelay.uiValue =
						BSYNClib_P_Convert_isrsafe(
							psSink->sElement.sDelay.sResults.uiApplied,
							BSYNClib_Units_e27MhzTicks,
							psSink->sElement.sDelay.sSnapshot.ePreferredUnits);
					psSink->sStatus.sAppliedDelay.eUnits = psSink->sElement.sDelay.sSnapshot.ePreferredUnits;
				}
			}
		}

		BSYSlib_List_ReleaseIterator(hIterator);
	}

	goto end;

	error:

	end:

	BDBG_LEAVE((BSYNClib_Algo_VideoSink_Applicator));
	return rc;
}

BERR_Code BSYNClib_Algo_VideoSource_Applicator(BSYNClib_Channel_Path * psPath, BSYNClib_Channel_SetDelay pfSetDelay, void * pvParm1, int iParm2)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER((BSYNClib_Algo_VideoSource_Applicator));

	BDBG_ASSERT(psPath);

	/* apply desired */
	if (pfSetDelay)
	{
		BSYSlib_List_IteratorHandle hIterator;

		hIterator = BSYSlib_List_AcquireIterator(psPath->hSources);

		while (BSYSlib_List_HasNext(hIterator))
		{
			BSYNClib_VideoSource * psSource;

			psSource = (BSYNClib_VideoSource *)BSYSlib_List_Next(hIterator);

			if (psSource->sElement.sSnapshot.bSynchronize)
			{
				BSYNClib_UnsignedValue sDesired;

				BDBG_MSG(("[%d]  Applying video source %u delay: %u ms",
					psPath->hChn->iIndex, psSource->sElement.uiIndex,
					BSYNClib_P_Convert_isrsafe(
						psSource->sElement.sDelay.sResults.uiDesired,
						BSYNClib_Units_e27MhzTicks,
						BSYNClib_Units_eMilliseconds)));

				sDesired.uiValue = BSYNClib_P_Convert_isrsafe(
					psSource->sElement.sDelay.sResults.uiDesired,
					BSYNClib_Units_e27MhzTicks,
					psSource->sElement.sDelay.sSnapshot.ePreferredUnits);
				sDesired.eUnits = psSource->sElement.sDelay.sSnapshot.ePreferredUnits;

				rc = pfSetDelay(pvParm1, iParm2, psSource->sElement.uiIndex, &sDesired);
				if (rc) goto error;

				psSource->sElement.sDelay.sResults.uiApplied = psSource->sElement.sDelay.sResults.uiDesired;
				psSource->sStatus.sAppliedDelay.uiValue =
					BSYNClib_P_Convert_isrsafe(
						psSource->sElement.sDelay.sResults.uiApplied,
						BSYNClib_Units_e27MhzTicks,
						psSource->sElement.sDelay.sSnapshot.ePreferredUnits);
				psSource->sStatus.sAppliedDelay.eUnits = psSource->sElement.sDelay.sSnapshot.ePreferredUnits;
			}
		}

		BSYSlib_List_ReleaseIterator(hIterator);
	}

	goto end;

	error:

	end:

	BDBG_LEAVE((BSYNClib_Algo_VideoSource_Applicator));
	return rc;
}

BERR_Code BSYNClib_Algo_AudioSink_Applicator(BSYNClib_Channel_Path * psPath, BSYNClib_Channel_SetDelay pfSetDelay, void * pvParm1, int iParm2)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER((BSYNClib_Algo_AudioSink_Applicator));

	BDBG_ASSERT(psPath);

	/* TODO: independent delay only to one port */
	/* apply desired */
	if (pfSetDelay)
	{
		BSYSlib_List_IteratorHandle hIterator;

		hIterator = BSYSlib_List_AcquireIterator(psPath->hSinks);

		while (BSYSlib_List_HasNext(hIterator))
		{
			BSYNClib_AudioSink * psSink;

			psSink = (BSYNClib_AudioSink *)BSYSlib_List_Next(hIterator);

			if (psSink->sElement.sSnapshot.bSynchronize)
			{
				BSYNClib_UnsignedValue sDesired;

				BDBG_MSG(("[%d]  Applying audio sink %u delay: %u ms",
					psPath->hChn->iIndex, psSink->sElement.uiIndex,
					BSYNClib_P_Convert_isrsafe(
						psSink->sElement.sDelay.sResults.uiDesired,
						BSYNClib_Units_e27MhzTicks,
						BSYNClib_Units_eMilliseconds)));

				sDesired.uiValue = BSYNClib_P_Convert_isrsafe(
					psSink->sElement.sDelay.sResults.uiDesired,
					BSYNClib_Units_e27MhzTicks,
					psSink->sElement.sDelay.sSnapshot.ePreferredUnits);
				sDesired.eUnits = psSink->sElement.sDelay.sSnapshot.ePreferredUnits;

				rc = pfSetDelay(pvParm1, iParm2, psSink->sElement.uiIndex, &sDesired);
				if (rc) goto error;

				psSink->sElement.sDelay.sResults.uiApplied = psSink->sElement.sDelay.sResults.uiDesired;
				psSink->sStatus.sAppliedDelay.uiValue =
					BSYNClib_P_Convert_isrsafe(
						psSink->sElement.sDelay.sResults.uiApplied,
						BSYNClib_Units_e27MhzTicks,
						psSink->sElement.sDelay.sSnapshot.ePreferredUnits);
				psSink->sStatus.sAppliedDelay.eUnits = psSink->sElement.sDelay.sSnapshot.ePreferredUnits;
			}
		}

		BSYSlib_List_ReleaseIterator(hIterator);
	}

	goto end;

	error:

	end:

	BDBG_LEAVE((BSYNClib_Algo_AudioSink_Applicator));
	return rc;
}

BERR_Code BSYNClib_Algo_AudioSource_Applicator(BSYNClib_Channel_Path * psPath, BSYNClib_Channel_SetDelay pfSetDelay, void * pvParm1, int iParm2)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER((BSYNClib_Algo_AudioSource_Applicator));

	BDBG_ASSERT(psPath);

	/* TODO: apply slowly over time using GA > d > ST method */
	/* apply desired */
	if (pfSetDelay)
	{
		BSYSlib_List_IteratorHandle hIterator;

		hIterator = BSYSlib_List_AcquireIterator(psPath->hSources);

		while (BSYSlib_List_HasNext(hIterator))
		{
			BSYNClib_AudioSource * psSource;

			psSource = (BSYNClib_AudioSource *)BSYSlib_List_Next(hIterator);

			if (psSource->sElement.sSnapshot.bSynchronize)
			{
				BSYNClib_UnsignedValue sDesired;

				BDBG_MSG(("[%d]  Applying audio source %u delay: %u ms",
					psPath->hChn->iIndex, psSource->sElement.uiIndex,
					BSYNClib_P_Convert_isrsafe(
						psSource->sElement.sDelay.sResults.uiDesired,
						BSYNClib_Units_e27MhzTicks,
						BSYNClib_Units_eMilliseconds)));

				sDesired.uiValue = BSYNClib_P_Convert_isrsafe(
					psSource->sElement.sDelay.sResults.uiDesired,
					BSYNClib_Units_e27MhzTicks,
					psSource->sElement.sDelay.sSnapshot.ePreferredUnits);
				sDesired.eUnits = psSource->sElement.sDelay.sSnapshot.ePreferredUnits;

				rc = pfSetDelay(pvParm1, iParm2, psSource->sElement.uiIndex, &sDesired);
				if (rc) goto error;

				psSource->sElement.sDelay.sResults.uiApplied = psSource->sElement.sDelay.sResults.uiDesired;
				psSource->sStatus.sAppliedDelay.uiValue =
					BSYNClib_P_Convert_isrsafe(
						psSource->sElement.sDelay.sResults.uiApplied,
						BSYNClib_Units_e27MhzTicks,
						psSource->sElement.sDelay.sSnapshot.ePreferredUnits);
				psSource->sStatus.sAppliedDelay.eUnits = psSource->sElement.sDelay.sSnapshot.ePreferredUnits;
			}
		}

		BSYSlib_List_ReleaseIterator(hIterator);
	}

	goto end;

	error:

	end:

	BDBG_LEAVE((BSYNClib_Algo_AudioSource_Applicator));
	return rc;
}

void BSYNClib_Algo_RequantizeDelay(int iChannelIndex, unsigned int uiDelay, unsigned int uiQuantizationLevel, unsigned int * puiRequantizedDelay)
{
	unsigned int uiQuantizedDelay;

	BDBG_ASSERT(puiRequantizedDelay);

#if BDBG_NO_MSG
	BSTD_UNUSED(iChannelIndex);
#endif

	if (uiQuantizationLevel)
	{
		BDBG_MSG(("[%d]  Performing delay requantization", iChannelIndex));
		BDBG_MSG(("[%d]    delay: %u ms", iChannelIndex,
			BSYNClib_P_Convert_isrsafe(uiDelay, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
		BDBG_MSG(("[%d]    quantization level: %u ms", iChannelIndex,
			BSYNClib_P_Convert_isrsafe(uiQuantizationLevel, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));

		uiQuantizedDelay = uiDelay / uiQuantizationLevel;
		if (uiDelay % uiQuantizationLevel)
		{
			uiQuantizedDelay++;
		}

		uiQuantizedDelay *= uiQuantizationLevel;

		BDBG_MSG(("[%d]    requantized delay: %u ms", iChannelIndex,
			BSYNClib_P_Convert_isrsafe(uiQuantizedDelay, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
	}
	else
	{
		BDBG_MSG(("[%d]  Cannot perform requantization with zero quantization level.", iChannelIndex));
		uiQuantizedDelay = uiDelay;
	}

	*puiRequantizedDelay = uiQuantizedDelay;
}

#if BSYNCLIB_JITTER_TOLERANCE_IMPROVEMENT_SUPPORT
#define BSYNCLIB_IABS(x) (((x) < 0) ? ((unsigned)-(x)) : ((unsigned)(x)))
void BSYNClib_Algo_CalculateJitterToleranceImprovementFactor(BSYNClib_Channel_Path * psVideo, BSYNClib_VideoSource * psSource, unsigned int uiCurrentDelay, int * piJtiFactor, unsigned int * puiAdditionalDelay)
{
	BSYSlib_List_IteratorHandle hIterator;
	BSYNClib_VideoSource_JitterToleranceImprovementFactor * psLastFactor;
	unsigned int uiDisplayVsyncPeriod = 0;
	unsigned int uiTarget;
	unsigned int uiThreshold;
	unsigned int uiMeasurement;
	unsigned int uiAdditionalDelay;
	int iFactor;
	int iError;
	bool bAllowBottomFieldTarget;
	bool bMeasurementValid = false;

	BDBG_ASSERT(psVideo);
	BDBG_ASSERT(psSource);
	BDBG_ASSERT(piJtiFactor);
	BDBG_ASSERT(puiAdditionalDelay);

	BDBG_ENTER((BSYNClib_Algo_CalculateJitterToleranceImprovementFactor));

	BDBG_MSG(("[%d]  Jitter Tolerance Improvement", psVideo->hChn->iIndex));

	hIterator = BSYSlib_List_AcquireIterator(psVideo->hSinks);

	while (BSYSlib_List_HasNext(hIterator))
	{
		BSYNClib_VideoSink * psSink;

		psSink = (BSYNClib_VideoSink *)BSYSlib_List_Next(hIterator);

		if (psSink->sElement.sSnapshot.bSynchronize && psSink->sSnapshot.bSyncLocked)
		{
			uiDisplayVsyncPeriod = BSYNClib_VideoFormat_P_GetVsyncPeriod(&psSink->sFormat);
			BDBG_MSG(("[%d]    Display %u vsync period: %u ms", psVideo->hChn->iIndex, psSink->sElement.uiIndex,
				BSYNClib_P_Convert_isrsafe(uiDisplayVsyncPeriod, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
			break;
		}
	}

	BSYSlib_List_ReleaseIterator(hIterator);

	bAllowBottomFieldTarget = psSource->sFormat.sSnapshot.bInterlaced;
	BDBG_MSG(("[%d]    Allow bottom field target: %s", psVideo->hChn->iIndex, bAllowBottomFieldTarget ? "true" : "false"));

	/* We want to avoid the following PTS/STC phase boundaries:
       0, 1 field, and 1 frame
       Therefore, 1/2 the display vsync period gives the most amount of jitter tolerance */
	uiTarget = uiDisplayVsyncPeriod / 2;

	BDBG_MSG(("[%d]    Jitter tolerance target: %u ms", psVideo->hChn->iIndex,
		BSYNClib_P_Convert_isrsafe(uiTarget, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));

	if (uiDisplayVsyncPeriod && psSource->sElement.sDelay.sSnapshot.bValid)
	{
		bMeasurementValid = true;
		uiMeasurement = psSource->sElement.sDelay.sSnapshot.uiMeasured;
		uiThreshold = psSource->sSnapshot.uiJitterToleranceImprovementThreshold;
		psLastFactor = &psSource->sResults.sJtiFactor;
		BDBG_MSG(("[%d]    Video source %u measurement: %u ms", psVideo->hChn->iIndex, psSource->sElement.uiIndex, 
			BSYNClib_P_Convert_isrsafe(uiMeasurement, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));

		/*
	        error = reference - output
		 */
		iError = (signed)uiTarget - (signed)uiMeasurement;

		BDBG_MSG(("[%d]    Error: %d ms", psVideo->hChn->iIndex, 
			BSYNClib_P_ConvertSigned_isrsafe(iError, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));

		if (bAllowBottomFieldTarget)
		{
			int iBottomFieldError;

			iBottomFieldError = (signed)(uiTarget + uiDisplayVsyncPeriod) - (signed)uiMeasurement;

			BDBG_MSG(("[%d]    Bottom field error: %d ms", psVideo->hChn->iIndex,
				BSYNClib_P_ConvertSigned_isrsafe(iBottomFieldError, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));

			if (BSYNCLIB_IABS(iBottomFieldError) < BSYNCLIB_IABS(iError))
			{
				iError = iBottomFieldError;
			}
		}

		/* if the error is outside the threshold, might as well go ahead and correct for it */
		if (BSYNCLIB_IABS(iError) > uiThreshold)
		{
			iFactor = psLastFactor->iValue - iError;

			if (iFactor > (signed)(uiDisplayVsyncPeriod + uiThreshold))
			{
				iFactor -= 2 * (signed)uiDisplayVsyncPeriod;
			}
			else if (iFactor < -(signed)(uiDisplayVsyncPeriod + uiThreshold))
			{
				iFactor += 2 * (signed)uiDisplayVsyncPeriod;
			}

			/* save to last factor and mark as adjusted */
			psLastFactor->bAdjusted = true;
			psLastFactor->iValue = iFactor;
		}
		else
		{
			iFactor = psLastFactor->iValue;
			psLastFactor->bAdjusted = false;
			psLastFactor->iValue = 0;
		}
	}
	else
	{
		iFactor = 0;
		uiAdditionalDelay = 0;
	}

	BDBG_MSG(("[%d]    JTI factor: %d ms", psVideo->hChn->iIndex,
		BSYNClib_P_ConvertSigned_isrsafe(iFactor, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));

	/* negative correction factor means that we need to add one display period delay */
	if ((iFactor < 0 && !uiCurrentDelay) ||
		((signed)uiCurrentDelay + iFactor < 0))
	{
		uiAdditionalDelay = uiDisplayVsyncPeriod;
	}
	else /* any other delay */
	{
		uiAdditionalDelay = 0;
	}

	BDBG_MSG(("[%d]    Additional delay: %d ms", psVideo->hChn->iIndex,
		BSYNClib_P_Convert_isrsafe(uiAdditionalDelay, BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));

	*piJtiFactor = iFactor;
	*puiAdditionalDelay = uiAdditionalDelay;

	BDBG_LEAVE((BSYNClib_Algo_CalculateJitterToleranceImprovementFactor));
}
#endif /* BSYNCLIB_JITTER_TOLERANCE_IMPROVEMENT_SUPPORT */
