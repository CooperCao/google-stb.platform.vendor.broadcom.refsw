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
#include "bsynclib_priv.h"
#include "bsynclib_channel_priv.h"
#include "bsynclib_video_format.h"
#include "bsynclib_rate_mismatch_detector.h"
#include "bsynclib_video_sink.h"
#include "bsynclib_mute_control.h"

#define BSYNCLIB_P_VIDEO_SINK_DEFAULT_UNITS BSYNClib_Units_e60HzVsyncs
#define BSYNCLIB_P_VIDEO_SINK_DEFAULT_THRESHOLD_VALUE 1 /* 1 60Hz vsync */
#define BSYNCLIB_P_VIDEO_SINK_DEFAULT_CAPACITY 4 /* 4 60Hz vsyncs */

BDBG_MODULE(synclib);

BSYNClib_VideoSink * BSYNClib_VideoSink_Create(void)
{
	BSYNClib_VideoSink * psSink = NULL;

	BDBG_ENTER(BSYNClib_VideoSink_Create);

	psSink = (BSYNClib_VideoSink *)BKNI_Malloc(sizeof(BSYNClib_VideoSink));

	if (psSink)
	{
		BKNI_Memset(psSink, 0, sizeof(BSYNClib_VideoSink));
		BSYNClib_DelayElement_Init(&psSink->sElement);
		BSYNClib_VideoFormat_Init(&psSink->sFormat);
		BSYNClib_VideoSink_GetDefaultConfig(&psSink->sConfig);
		BSYNClib_VideoSink_P_GetDefaultStatus(&psSink->sStatus);
		/* TODO: remove started var */
		psSink->sElement.sData.bStarted = true;
		psSink->sElement.sDelay.sData.eOriginalUnits = BSYNCLIB_P_VIDEO_SINK_DEFAULT_UNITS;
		psSink->sElement.sDelay.sSnapshot.eOriginalUnits = BSYNCLIB_P_VIDEO_SINK_DEFAULT_UNITS;
		psSink->sElement.sNotification.sResults.sThreshold.eUnits = BSYNCLIB_P_VIDEO_SINK_DEFAULT_UNITS;
		psSink->sElement.sNotification.sResults.sThreshold.uiValue = BSYNCLIB_P_VIDEO_SINK_DEFAULT_THRESHOLD_VALUE;
	}

	BDBG_LEAVE(BSYNClib_VideoSink_Create);
	return psSink;
}

void BSYNClib_VideoSink_Destroy(BSYNClib_VideoSink * psSink)
{
	BDBG_ENTER(BSYNClib_VideoSink_Destroy);
	BDBG_ASSERT(psSink);
	BKNI_Free(psSink);
	BDBG_LEAVE(BSYNClib_VideoSink_Destroy);
}

bool BSYNClib_VideoSink_SyncCheck(BSYNClib_VideoSink * psSink)
{
	bool bPass = false;

	BDBG_ENTER(BSYNClib_VideoSink_SyncCheck);

	BDBG_ASSERT(psSink);

	bPass = !psSink->sElement.sSnapshot.bSynchronize
		|| (psSink->sFormat.sSnapshot.bValid /* must have a valid format */
			&& psSink->sElement.sDelay.sSnapshot.bValid /* have a valid measured delay */
			&& psSink->sElement.sDelay.sResults.bAccepted); /* and be accepted */

	BDBG_MSG(("[%d] Video sink %d sync check:", psSink->sElement.hParent->iIndex, psSink->sElement.uiIndex));
	BDBG_MSG(("[%d]  %s", psSink->sElement.hParent->iIndex, psSink->sElement.sSnapshot.bSynchronize ? "synchronized" : "ignored"));
	BDBG_MSG(("[%d]  format %s", psSink->sElement.hParent->iIndex, psSink->sFormat.sSnapshot.bValid ? "valid" : "invalid"));
	BDBG_MSG(("[%d]  delay %s, %s", psSink->sElement.hParent->iIndex, psSink->sElement.sDelay.sSnapshot.bValid ? "valid" : "invalid",
		psSink->sElement.sDelay.sResults.bAccepted ? "accepted" : "unaccepted"));

	/* TODO: is there a better way to get these settings? */
	if (psSink->sElement.hParent->hParent->sSettings.sVideo.bRequireFullScreen)
	{
		bPass = bPass && psSink->sSnapshot.bFullScreen;
		BDBG_MSG(("[%d]  full screen required, %s screen found", psSink->sElement.hParent->iIndex,
			psSink->sSnapshot.bFullScreen ? "full" : "partial"));
	}

	BDBG_LEAVE(BSYNClib_VideoSink_SyncCheck);
	return bPass;
}

void BSYNClib_VideoSink_Reset_isr(BSYNClib_VideoSink * psSink)
{
	BDBG_ENTER(BSYNClib_VideoSink_Reset_isr);
	BDBG_ASSERT(psSink);
	psSink->sData.bForcedCaptureEnabled = false;
	psSink->sData.bMasterFrameRateEnabled = false;
	psSink->sData.bVisible = false;
	psSink->sData.bFullScreen = false;
	BDBG_LEAVE(BSYNClib_VideoSink_Reset_isr);
}

void BSYNClib_VideoSink_GetDefaultConfig(BSYNClib_VideoSink_Config * psConfig)
{
	BDBG_ENTER(BSYNClib_VideoSink_GetDefaultConfig);
	BDBG_ASSERT(psConfig);
	BKNI_Memset(psConfig, 0, sizeof(BSYNClib_VideoSink_Config));
	psConfig->bForcedCaptureEnabled = true;
	psConfig->bMasterFrameRateEnabled = true;
	psConfig->sFormat.bInterlaced = true;
	psConfig->sFormat.eFrameRate = BAVC_FrameRateCode_eUnknown;
	psConfig->sDelay.sMeasured.eUnits = BSYNCLIB_P_VIDEO_SINK_DEFAULT_UNITS;
	psConfig->sDelay.sMax.eUnits = BSYNCLIB_P_VIDEO_SINK_DEFAULT_UNITS;
	psConfig->sDelay.sMax.uiValue = BSYNCLIB_P_VIDEO_SINK_DEFAULT_CAPACITY;
	BDBG_LEAVE(BSYNClib_VideoSink_GetDefaultConfig);
}

void BSYNClib_VideoSink_P_SelfClearConfig_isr(BSYNClib_VideoSink * psSink)
{
	BDBG_ENTER(BSYNClib_VideoSink_P_SelfClearConfig_isr);
	BDBG_ASSERT(psSink);
	psSink->sConfig.sDelay.bReceived = false;
	psSink->sConfig.sFormat.bReceived = false;
	BDBG_LEAVE(BSYNClib_VideoSink_P_SelfClearConfig_isr);
}

BERR_Code BSYNClib_VideoSink_P_ProcessConfig_isr(BSYNClib_VideoSink * psSink)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_VideoSink_Config * psConfig;
	BSYNClib_Channel_Handle hChn;
	bool bChanged = false;
	bool bInvokeRmd = false;
	bool bPrediction = false;
	BSYNClib_DelayElement sDesired;
	BSYNClib_DelayElement * psCurrent;
	BSYNClib_DelayElement_DiffResults sElementDiffResults;
	BSYNClib_VideoFormat sFormat;
	bool bFormatReceived = false;
	BSYNClib_VideoFormat_DiffResults sFormatDiffResults;

	BDBG_ENTER(BSYNClib_VideoSink_P_ProcessConfig_isr);

	BDBG_ASSERT(psSink);

	hChn = psSink->sElement.hParent;
	psConfig = &psSink->sConfig;
	psCurrent = &psSink->sElement;

	BKNI_Memset(&sElementDiffResults, 0, sizeof(BSYNClib_DelayElement_DiffResults));

	/* check lifecycle first, not yet available for sinks */
	BSYNClib_DelayElement_CheckLifecycle_isr(true, psCurrent, &sElementDiffResults);
	if (sElementDiffResults.eLifecycleEvent == BSYNClib_DelayElement_LifecycleEvent_eStarted)
	{
		BSYNClib_DelayElement_Reset_isr(psCurrent);
	}

	/* create "desired" delay element config */
	sDesired = *psCurrent;
	sDesired.sData.bStarted = true;
	sDesired.sData.bSynchronize = psConfig->bSynchronize;
	sDesired.sDelay.sData.eOriginalUnits = psConfig->sDelay.sMeasured.eUnits;
	sDesired.sDelay.sData.uiMeasured = BSYNClib_P_Convert_isrsafe(
		psConfig->sDelay.sMeasured.uiValue, psConfig->sDelay.sMeasured.eUnits,
		BSYNClib_Units_e27MhzTicks);
	sDesired.sDelay.sData.uiCustom = BSYNClib_P_Convert_isrsafe(
		psConfig->sDelay.sCustom.uiValue, psConfig->sDelay.sCustom.eUnits,
		BSYNClib_Units_e27MhzTicks);
	sDesired.sDelay.sData.uiCapacity = BSYNClib_P_Convert_isrsafe(
		psConfig->sDelay.sMax.uiValue, psConfig->sDelay.sMax.eUnits,
		BSYNClib_Units_e27MhzTicks);
	sDesired.sNotification.sData.bReceived = psConfig->sDelay.bReceived;
	if (psConfig->sDelay.bReceived)
	{
		/* TODO: this only gets cleared if we destroy/recreate the sink */
		psSink->sStatus.bDelayReceived = true;
	}
	/* TODO: reexamine assumption of 1 vsync'edness here */
	/* if the units are vsyncs, we want a threshold of 1 vsync, don't convert 1 vsync @ 60Hz to x.y vsyncs @ 50 Hz */
	if (psConfig->sDelay.sMeasured.eUnits == BSYNClib_Units_e24HzVsyncs
		|| psConfig->sDelay.sMeasured.eUnits == BSYNClib_Units_e50HzVsyncs
		|| psConfig->sDelay.sMeasured.eUnits == BSYNClib_Units_e60HzVsyncs)
	{
		sDesired.sNotification.sResults.sThreshold.eUnits = psConfig->sDelay.sMeasured.eUnits;
	}
	else
	{
		if (psSink->sStatus.sDelayNotification.sThreshold.eUnits != psConfig->sDelay.sMeasured.eUnits)
		{
			/* convert status units to user's measurement units */
			psSink->sStatus.sDelayNotification.sThreshold.uiValue =
				BSYNClib_P_Convert_isrsafe(
					psSink->sStatus.sDelayNotification.sThreshold.uiValue,
					psSink->sStatus.sDelayNotification.sThreshold.eUnits,
					BSYNClib_Units_e27MhzTicks);
			psSink->sStatus.sDelayNotification.sThreshold.uiValue =
				BSYNClib_P_Convert_isrsafe(
					psSink->sStatus.sDelayNotification.sThreshold.uiValue,
					BSYNClib_Units_e27MhzTicks,
					psConfig->sDelay.sMeasured.eUnits);
		}
	}

	psSink->sStatus.sDelayNotification.sThreshold.eUnits = psConfig->sDelay.sMeasured.eUnits;
	/* TODO: copy max delay for 7038 */

	BSYNClib_DelayElement_Diff_isr(&sDesired, &psSink->sElement, &sElementDiffResults);

	if (sElementDiffResults.bChanged)
	{
		bChanged = true;
		BSYNClib_DelayElement_Patch_isr(&sDesired, &psSink->sElement, &sElementDiffResults);
	}

	/* TODO: need to remove this because we don't have started lifecycle event anymore */
	if (sElementDiffResults.eLifecycleEvent == BSYNClib_DelayElement_LifecycleEvent_eStarted)
	{
		/* reset sink data */
		BSYNClib_VideoSink_Reset_isr(psSink);
	}

	bFormatReceived = psConfig->sFormat.bReceived;

	/* received sink format info */
	if (bFormatReceived)
	{
		/* create "desired" video format config */
		sFormat.sData.bInterlaced = psConfig->sFormat.bInterlaced;
		sFormat.sData.uiHeight = psConfig->sFormat.uiHeight;
		sFormat.sData.eFrameRate = psConfig->sFormat.eFrameRate;

		BSYNClib_VideoFormat_Diff_isr(&sFormat, &psSink->sFormat, &sFormatDiffResults);

		if (sFormatDiffResults.bChanged)
		{
			bChanged = true;
			BSYNClib_Channel_P_ResetVideoSourceJtiFactor_isr(hChn);
			BSYNClib_VideoFormat_Patch_isr(&sFormat, &psSink->sFormat, &sFormatDiffResults);
		}

		bInvokeRmd = true;
	}

	/* do this after format processing, in case we received both at the same time */
	if (sElementDiffResults.bDelayReceived)
	{
		BDBG_MSG(("[%d] Video sink %u delay received: %u ms", hChn->iIndex, psSink->sElement.uiIndex,
			BSYNClib_P_Convert_isrsafe(
				psSink->sElement.sDelay.sData.uiMeasured,
				BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));

		/* all window delays are immediately accepted, no timeout necessary */
		psSink->sElement.sDelay.sResults.bAccepted = true;
	}

	/* copy window-specific data */
	if
	(
		(psSink->sData.bForcedCaptureEnabled != psConfig->bForcedCaptureEnabled)
		|| (psSink->sData.bMasterFrameRateEnabled != psConfig->bMasterFrameRateEnabled)
		|| (psSink->sData.bVisible != psConfig->bVisible)
		|| (psSink->sData.bSyncLocked != psConfig->bSyncLocked)
	)
	{
		psSink->sData.bForcedCaptureEnabled = psConfig->bForcedCaptureEnabled;
		psSink->sData.bMasterFrameRateEnabled = psConfig->bMasterFrameRateEnabled;
		psSink->sData.bSyncLocked = psConfig->bSyncLocked;
		psSink->sData.bVisible = psConfig->bVisible;

		bChanged = true;

		if (psSink->sFormat.sData.bValid)
		{
			bInvokeRmd = true;
		}

		/* TODO: find min delay */
		/*psSink->sData.uiMinDelay = */
	}

	if (bInvokeRmd)
	{
		/* update rmd sink params */
		rc = BSYNClib_RateMismatchDetector_SetVideoSinkFormat_isr(hChn->hDetector,
			psSink->sElement.uiIndex,
			&psSink->sFormat,
			psSink->sData.bMasterFrameRateEnabled,
			psSink->sData.bSyncLocked);
		if (rc) goto error;
	}

	/* copy window-specific data */
	if (psSink->sData.bFullScreen != psConfig->bFullScreen)
	{
		psSink->sData.bFullScreen = psConfig->bFullScreen;
		bChanged = true;

		/* TODO: old code resets rmd when window resized, still necessary? */
		/* tell mute control that something happened that needs processing */
		BSYNClib_MuteControl_ScheduleTask_isr(hChn);
	}

	if (bChanged)
	{
		BDBG_MSG(("[%d] Video sink %u properties changed:", hChn->iIndex, psSink->sElement.uiIndex));
		BDBG_MSG(("[%d]  %s", hChn->iIndex, psSink->sElement.sData.bSynchronize ? "synchronized" : "ignored"));
		if (sElementDiffResults.eLifecycleEvent != BSYNClib_DelayElement_LifecycleEvent_eNone)
		{
			BDBG_MSG(("  %s", BSYNClib_DelayElement_LifecycleEventNames[sElementDiffResults.eLifecycleEvent]));
		}
		BDBG_MSG(("[%d]  measured delay %u ms", hChn->iIndex,
			BSYNClib_P_Convert_isrsafe(
				psSink->sElement.sDelay.sData.uiMeasured,
				BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
		BDBG_MSG(("[%d]  custom delay %u ms", hChn->iIndex,
			BSYNClib_P_Convert_isrsafe(
				psSink->sElement.sDelay.sData.uiCustom,
				BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
		BDBG_MSG(("[%d]  delay capacity %u ms", hChn->iIndex,
			BSYNClib_P_Convert_isrsafe(
				psSink->sElement.sDelay.sData.uiCapacity,
				BSYNClib_Units_e27MhzTicks, BSYNClib_Units_eMilliseconds)));
		BDBG_MSG(("[%d]  forced capture %s", hChn->iIndex, psSink->sData.bForcedCaptureEnabled ? "enabled" : "disabled"));
		BDBG_MSG(("[%d]  master frame rate %s", hChn->iIndex, psSink->sData.bMasterFrameRateEnabled ? "enabled" : "disabled"));
		BDBG_MSG(("[%d]  sync-%s", hChn->iIndex, psSink->sData.bSyncLocked ? "locked" : "slipped"));
		BDBG_MSG(("[%d]  %s screen", hChn->iIndex, psSink->sData.bFullScreen ? "full" : "partial"));
		BDBG_MSG(("[%d]  %s", hChn->iIndex, psSink->sData.bVisible ? "visible" : "hidden"));
		BDBG_MSG(("[%d]  format:", hChn->iIndex));
		BDBG_MSG(("[%d]    %s", hChn->iIndex, psSink->sFormat.sData.bInterlaced ? "interlaced" : "progressive"));
		BDBG_MSG(("[%d]    height %u", hChn->iIndex, psSink->sFormat.sData.uiHeight));
		BDBG_MSG(("[%d]    frame rate %s", hChn->iIndex, BSYNClib_VideoFormat_P_GetFrameRateName_isrsafe(psSink->sFormat.sData.eFrameRate)));
	}

	if (bChanged && !bPrediction)
	{
		if (BSYNClib_Channel_P_Enabled_isrsafe(hChn))
		{
			/* immediate reprocessing based on current state */
			BSYNClib_Channel_P_ScheduleTask_isr(hChn);
		}
		else
		{
			/* reprocessing based on current state deferred until re-enabled */
			BSYNClib_Channel_P_EnqueueTaskRequest_isr(hChn);
		}
	}

	error:
	BDBG_LEAVE(BSYNClib_VideoSink_P_ProcessConfig_isr);
	return rc;
}

void BSYNClib_VideoSink_Snapshot_isr(BSYNClib_VideoSink * psSink)
{
	BDBG_ENTER(BSYNClib_VideoSink_Snapshot_isr);
	BDBG_ASSERT(psSink);
	psSink->sSnapshot = psSink->sData;
	BSYNClib_DelayElement_Snapshot_isr(&psSink->sElement);
	BSYNClib_VideoFormat_Snapshot_isr(&psSink->sFormat);
	BDBG_LEAVE(BSYNClib_VideoSink_Snapshot_isr);
}

void BSYNClib_VideoSink_P_GetDefaultStatus(
	BSYNClib_Sink_Status * psStatus
)
{
	BDBG_ENTER(BSYNClib_VideoSink_P_GetDefaultStatus);
	BDBG_ASSERT(psStatus);
	psStatus->bDelayReceived = false;
	psStatus->sDelayNotification.bEnabled = true;
	psStatus->sDelayNotification.sThreshold.uiValue = BSYNCLIB_P_VIDEO_SINK_DEFAULT_THRESHOLD_VALUE;
	psStatus->sDelayNotification.sThreshold.eUnits = BSYNCLIB_P_VIDEO_SINK_DEFAULT_UNITS;
	psStatus->sAppliedDelay.uiValue = 0;
	BDBG_LEAVE(BSYNClib_VideoSink_P_GetDefaultStatus);
}
