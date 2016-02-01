/***************************************************************************
*     Copyright (c) 2004-2007, Broadcom Corporation
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
#include "bsynclib.h"
#include "bsynclib_test_stubs_priv.h"
#include "blst_queue.h"

BDBG_MODULE(synclib);

BERR_Code BSYNClib_GetDefaultSettings
(
	BSYNClib_Settings * psSettings /* [out] */
)
{
	BERR_Code rc = BERR_SUCCESS;

	BKNI_Memset(psSettings, 0, sizeof(BSYNClib_Settings));

	psSettings->bEnabled = true;
	psSettings->sVideo.bRequireFullScreen = true;
	psSettings->sVideo.uiTsmLockTimeout = BSYNCLIB_VIDEO_TSM_LOCK_TIMER_DEFAULT_TIMEOUT;
	psSettings->sVideo.uiUnmuteTimeout = BSYNCLIB_VIDEO_UNMUTE_DEFAULT_TIMEOUT;
	psSettings->sVideo.sRateMismatchDetection.uiTimeout = BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_TIMER_DEFAULT_TIMEOUT;
	psSettings->sVideo.sRateMismatchDetection.uiAcceptableMtbcLower = BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_MTBC_LOWER;
	psSettings->sVideo.sRateMismatchDetection.uiAcceptableMtbcUpper = BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_MTBC_UPPER;
	psSettings->sVideo.sRateMismatchDetection.uiAcceptableTtlc = BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_TTLC;
	psSettings->sVideo.sRateRematchDetection.uiTimeout = BSYNCLIB_VIDEO_RATE_REMATCH_DETECTION_TIMER_DEFAULT_TIMEOUT;
	psSettings->sVideo.sRateRematchDetection.uiAcceptableTtlc = BSYNCLIB_VIDEO_RATE_REMATCH_DETECTION_DEFAULT_ACCEPTABLE_TTLC;
	psSettings->sAudio.uiUnmuteTimeout = BSYNCLIB_AUDIO_UNMUTE_DEFAULT_TIMEOUT;
	psSettings->sAudio.uiReceiverDelayCompensation = BSYNCLIB_AUDIO_RECEIVER_DELAY_COMPENSATION_DEFAULT;
	psSettings->sAudio.uiUnconditionalUnmuteTimeout = BSYNCLIB_AUDIO_UNCONDITIONAL_UNMUTE_DEFAULT_TIMEOUT;

	return rc;
}

BERR_Code BSYNClib_Open
(
	const BSYNClib_Settings * psSettings,
	BSYNClib_Handle * phSync /* [out] */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_Handle hSync;

	hSync = (BSYNClib_Handle)BKNI_Malloc(sizeof(struct BSYNClib_Impl));
	BKNI_Memset(hSync, 0, sizeof(struct BSYNClib_Impl));

	BLST_Q_INIT(hSync->sChannels);

	hSync->sSettings = *psSettings;

	*phSync = hSync;

	return rc;
}

BERR_Code BSYNClib_Close
(
	BSYNClib_Handle hSync
)
{
	BERR_Code rc = BERR_SUCCESS;
	unsigned int i = 0;
	BSYNClib_Channel_Handle hChn;

	if (hSync)
	{
		for (hChn = BLST_Q_FIRST(hSync->sChannels); hChn; hChn = BLST_Q_FIRST(hSync->sChannels))
		{
			BLST_Q_REMOVE_HEAD(hSync->sChannels, sLink);
			BSYNClib_DestroyChannel(hChn);
		}
		
		BKNI_Free(hSync);
	}

	return rc;
}

void BSYNClib_GetChannelDefaultSettings
(
	const BSYNClib_Handle hSync, 
	BSYNClib_Channel_Settings * psSettings /* [out] */
)
{
	BSTD_UNUSED(hSync);

	BKNI_Memset(psSettings, 0, sizeof(BSYNClib_Channel_Settings));

	psSettings->pvUserCallbackContextParm1 = NULL; 
	psSettings->iUserCallbackContextParm2 = 0; 
#if 0
	psSettings->pfScheduleTask_isr = NULL; 
#endif
	psSettings->pfCreateTimer = NULL; 
	psSettings->pfDestroyTimer = NULL; 
	psSettings->pfStartTimer_isr = NULL; 
	psSettings->pfCancelTimer_isr = NULL; 
	psSettings->pfGetTime_isr = NULL; 
	psSettings->sVideo.sSource.pfSetMute = NULL;
	psSettings->sVideo.sSource.pfSetDelay = NULL;
	psSettings->sVideo.sSource.pfSetDelayNotification = NULL;
	psSettings->sVideo.sSink.pfSetDelay = NULL;
	psSettings->sVideo.sSink.pfSetDelayNotification = NULL;
	psSettings->sAudio.sSource.pfSetMute = NULL;
	psSettings->sAudio.sSource.pfSetDelay = NULL;
	psSettings->sAudio.sSource.pfSetDelayNotification = NULL;
	psSettings->sAudio.sSink.pfSetDelay = NULL;
	psSettings->sAudio.sSink.pfSetDelayNotification = NULL;
}

static void BSYNClib_VideoSource_P_GetDefaultConfig(BSYNClib_VideoSource_Config * psConfig) 
{
	BDBG_ASSERT(psConfig);

	BKNI_Memset(psConfig, 0, sizeof(BSYNClib_VideoSource_Config));

	psConfig->bStarted = false;
	psConfig->bDigital = true;
	
	psConfig->sDelay.bReceived = false;
	psConfig->sDelay.uiMeasured = 0;
	psConfig->sDelay.uiCustom = 0;
	
	psConfig->sFormat.bReceived = false;
	psConfig->sFormat.bInterlaced = true;
	psConfig->sFormat.uiHeight = 0;
	psConfig->sFormat.eFrameRate = BAVC_FrameRateCode_eUnknown;

	psConfig->bLastPictureHeld = true;
}

static void BSYNClib_VideoSink_P_GetDefaultConfig(BSYNClib_VideoSink_Config * psConfig) 
{
	BDBG_ASSERT(psConfig);

	BKNI_Memset(psConfig, 0, sizeof(BSYNClib_VideoSink_Config));

	psConfig->bStarted = false;
	
	psConfig->sDelay.bReceived = false;
	psConfig->sDelay.uiMeasured = 0;
	psConfig->sDelay.uiCustom = 0;
	
	psConfig->sFormat.bReceived = false;
	psConfig->sFormat.bInterlaced = true;
	psConfig->sFormat.uiHeight = 0;
	psConfig->sFormat.eFrameRate = BAVC_FrameRateCode_eUnknown;

	psConfig->bForcedCaptureEnabled = true;
	psConfig->bFullScreen = false;
	psConfig->bMasterFrameRateEnabled = true;
	psConfig->bVisible = false;
}

static void BSYNClib_AudioSource_P_GetDefaultConfig(BSYNClib_AudioSource_Config * psConfig) 
{
	BDBG_ASSERT(psConfig);

	BKNI_Memset(psConfig, 0, sizeof(BSYNClib_AudioSource_Config));

	psConfig->bStarted = false;
	psConfig->bDigital = true;
	
	psConfig->sDelay.bReceived = false;
	psConfig->sDelay.uiMeasured = 0;
	psConfig->sDelay.uiCustom = 0;
	
	psConfig->bSamplingRateReceived = false;
}

static void BSYNClib_AudioSink_P_GetDefaultConfig(BSYNClib_AudioSink_Config * psConfig) 
{
	BDBG_ASSERT(psConfig);

	BKNI_Memset(psConfig, 0, sizeof(BSYNClib_AudioSink_Config));

	psConfig->bStarted = false;
	psConfig->bCompressed = false;
	
	psConfig->sDelay.bReceived = false;
	psConfig->sDelay.uiMeasured = 0;
	psConfig->sDelay.uiCustom = 0;
	
	psConfig->uiSamplingRate = 48000;
}

static BERR_Code BSYNClib_Channel_P_GetDefaultConfig(BSYNClib_Channel_Config * psConfig) 
{
	BDBG_ASSERT(psConfig);

	BKNI_Memset(psConfig, 0, sizeof(BSYNClib_Channel_Config));

	psConfig->bEnabled = true;
	psConfig->uiVideoSourceCount = 0;
	psConfig->uiVideoSinkCount = 0;
	psConfig->uiAudioSourceCount = 0;
	psConfig->uiAudioSinkCount = 0;
}


BERR_Code BSYNClib_CreateChannel
(
	BSYNClib_Handle hSync, 
	const BSYNClib_Channel_Settings * psSettings, 
	BSYNClib_Channel_Handle * phChn /* [out] */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_Channel_Handle hChn;
	unsigned int i = 0;

	hChn = (BSYNClib_Channel_Handle)BKNI_Malloc(sizeof(struct BSYNClib_Channel_Impl));
	BKNI_Memset(hChn, 0, sizeof(struct BSYNClib_Channel_Impl));

	hChn->sSettings = *psSettings;

	BLST_Q_INSERT_TAIL(hSync->sChannels, hChn, sLink);
	hChn->hParent = hSync;

	BSYNClib_Channel_P_GetDefaultConfig(&hChn->sConfig);
	BSYNClib_VideoSource_P_GetDefaultConfig(&hChn->sVideoSourceConfig);
	BSYNClib_VideoSink_P_GetDefaultConfig(&hChn->sVideoSinkConfig);
	BSYNClib_AudioSource_P_GetDefaultConfig(&hChn->sAudioSourceConfig);
	BSYNClib_AudioSink_P_GetDefaultConfig(&hChn->sAudioSinkConfig);

	if (hChn->sSettings.pfCreateTimer)
	{
		rc = hChn->sSettings.pfCreateTimer(hChn->sSettings.pvUserCallbackContextParm1, hChn->sSettings.iUserCallbackContextParm2, &hChn->hTimer1);
		if (rc) goto error;

		rc = hChn->sSettings.pfCreateTimer(hChn->sSettings.pvUserCallbackContextParm1, hChn->sSettings.iUserCallbackContextParm2, &hChn->hTimer2);
		if (rc) goto error;
	}

	*phChn = hChn;

	goto end;

error:

end:

	return rc;
}

void BSYNClib_DestroyChannel
(
	BSYNClib_Channel_Handle hChn
)
{
	BSYNClib_Handle hSync;
	unsigned int i = 0;

	hSync = hChn->hParent;
	BLST_Q_REMOVE(hSync->sChannels, hChn, sLink);

	if (hChn->sSettings.pfDestroyTimer)
	{
		hChn->sSettings.pfDestroyTimer(hChn->sSettings.pvUserCallbackContextParm1, hChn->sSettings.iUserCallbackContextParm2, hChn->hTimer1);
		hChn->hTimer1 = NULL;

		hChn->sSettings.pfDestroyTimer(hChn->sSettings.pvUserCallbackContextParm1, hChn->sSettings.iUserCallbackContextParm2, hChn->hTimer2);
		hChn->hTimer2 = NULL;
	}

	BKNI_Free(hChn);
}

static BERR_Code BSYNClib_Channel_P_TimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer)
{
	BERR_Code rc = BERR_SUCCESS;

	BSTD_UNUSED(pvParm1);
	BSTD_UNUSED(iParm2);

	BDBG_MSG(("Timer %p expired", (void *)hTimer));

	return rc;
}

static void BSYNClib_Channel_P_SyncTask(void * pvParm1, int iParm2)
{
	BSYNClib_Channel_Handle hChn = pvParm1;
	unsigned int i = 0;
	unsigned long ulDelay = 10; /* 10 ms */
	unsigned long ulThreshold = 1; /* 1 ms */

	BSTD_UNUSED(iParm2);
	
	/* calling all callbacks for testing purposes */

	pvParm1 = hChn->sSettings.pvUserCallbackContextParm1;
	iParm2 = hChn->sSettings.iUserCallbackContextParm2;

	if (hChn->sSettings.sVideo.pfSetMute)
	{
		BDBG_MSG(("Muting video source %d", i));
		hChn->sSettings.sVideo.pfSetMute(pvParm1, iParm2, 0, true);
		BDBG_MSG(("Unmuting video source %d", i));
		hChn->sSettings.sVideo.pfSetMute(pvParm1, iParm2, 0, false);
	}

	if (hChn->sSettings.sVideo.sSource.pfSetDelay)
	{
		BDBG_MSG(("Setting video source %d delay to %lu", 0, ulDelay));
		hChn->sSettings.sVideo.sSource.pfSetDelay(pvParm1, iParm2, 0, ulDelay);
	}

	if (hChn->sSettings.sVideo.sSource.pfSetDelayNotification)
	{
		BDBG_MSG(("Setting video source %d delay notification to enabled with threshold %lu", i, ulThreshold));
		hChn->sSettings.sVideo.sSource.pfSetDelayNotification(pvParm1, iParm2, 0, true, ulThreshold);
	}

	if (hChn->sSettings.sVideo.sSink.pfSetDelay)
	{
		BDBG_MSG(("Setting video sink %d delay to %lu", i, ulDelay));
		hChn->sSettings.sVideo.sSink.pfSetDelay(pvParm1, iParm2, 0, ulDelay);
	}

	if (hChn->sSettings.sVideo.sSource.pfSetDelayNotification)
	{
		BDBG_MSG(("Setting video sink %d delay notification to enabled with threshold %lu", i, ulThreshold));
		hChn->sSettings.sVideo.sSink.pfSetDelayNotification(pvParm1, iParm2, 0, true, ulThreshold);
	}

	if (hChn->sSettings.sAudio.pfSetMute)
	{
		BDBG_MSG(("Muting audio source %d", i));
		hChn->sSettings.sAudio.pfSetMute(pvParm1, iParm2, 0, true);
		BDBG_MSG(("Unmuting audio source %d", i));
		hChn->sSettings.sAudio.pfSetMute(pvParm1, iParm2, 0, false);
	}

	if (hChn->sSettings.sAudio.sSource.pfSetDelay)
	{
		BDBG_MSG(("Setting audio source %d delay to %lu", i, ulDelay));
		hChn->sSettings.sAudio.sSource.pfSetDelay(pvParm1, iParm2, 0, ulDelay);
	}

	if (hChn->sSettings.sAudio.sSource.pfSetDelayNotification)
	{
		BDBG_MSG(("Setting audio source %d delay notification to enabled with threshold %lu", i, ulThreshold));
		hChn->sSettings.sAudio.sSource.pfSetDelayNotification(pvParm1, iParm2, 0, true, ulThreshold);
	}

	if (hChn->sSettings.sAudio.sSink.pfSetDelay)
	{
		BDBG_MSG(("Setting audio sink %d delay to %lu", i, ulDelay));
		hChn->sSettings.sAudio.sSink.pfSetDelay(pvParm1, iParm2, 0, ulDelay);
	}

	if (hChn->sSettings.sAudio.sSource.pfSetDelayNotification)
	{
		BDBG_MSG(("Setting audio sink %d delay notification to enabled with threshold %lu", i, ulThreshold));
		hChn->sSettings.sAudio.sSink.pfSetDelayNotification(pvParm1, iParm2, 0, true, ulThreshold);
	}
}

BERR_Code BSYNClib_Channel_GetConfig
(
	const BSYNClib_Channel_Handle hChn,
	BSYNClib_Channel_Config * psConfig /* [out] */
)
{
	BERR_Code rc = BERR_SUCCESS;

	*psConfig = hChn->sConfig;

	return rc;
}

BERR_Code BSYNClib_Channel_SetConfig
(
	BSYNClib_Channel_Handle hChn,
	const BSYNClib_Channel_Config * psConfig
)
{
	BERR_Code rc = BERR_SUCCESS;
	void * pvParm1;
	int iParm2;

	pvParm1 = hChn->sSettings.pvUserCallbackContextParm1;
	iParm2 = hChn->sSettings.iUserCallbackContextParm2;

	hChn->sConfig = *psConfig;

	/* just for testing */
	if (hChn->sSettings.pfStartTimer_isr && hChn->hTimer1 && hChn->hTimer2)
	{
		BSYSlib_Timer_Settings sTimer;

		sTimer.ulTimeout = 5000;
		sTimer.pfTimerExpired = &BSYNClib_Channel_P_TimerExpired;
		sTimer.pvParm1 = hChn;
		sTimer.iParm2 = 0;

		BDBG_MSG(("Scheduling timer 1 for %lu ms", sTimer.ulTimeout));
		hChn->sSettings.pfStartTimer_isr(pvParm1, iParm2, hChn->hTimer1, &sTimer);
		BDBG_MSG(("Timer 1 scheduled: %p", (void *)hChn->hTimer1));
		
		if (hChn->sSettings.pfCancelTimer_isr)
		{
			BDBG_MSG(("Canceling timer 1: %p", (void *)hChn->hTimer1));
			hChn->sSettings.pfCancelTimer_isr(pvParm1, iParm2, hChn->hTimer1);
		}

		BDBG_MSG(("Scheduling timer 2 for %lu ms", sTimer.ulTimeout));
		hChn->sSettings.pfStartTimer_isr(pvParm1, iParm2, hChn->hTimer2, &sTimer);
		BDBG_MSG(("Timer 2 scheduled: %p", (void *)hChn->hTimer2));

		sTimer.ulTimeout = 0;
		sTimer.pfTimerExpired = &BSYNClib_Channel_P_SyncTask;
		BDBG_MSG(("Scheduling timer 1 for %lu ms for task time call of function %p", sTimer.ulTimeout, sTimer.pfTimerExpired));
		hChn->sSettings.pfStartTimer_isr(pvParm1, iParm2, hChn->hTimer1, &sTimer);
		BDBG_MSG(("Timer 1 scheduled: %p", (void *)hChn->hTimer1));
	}

	return rc;
}

/*
Summary:
Returns the current status of a sync lib channel.
*/
BERR_Code BSYNClib_Channel_GetStatus
(
	const BSYNClib_Channel_Handle hChn, 
	BSYNClib_Channel_Status * psStatus /* [out] */
)
{
	BERR_Code rc = BERR_SUCCESS;

	*psStatus = hChn->sStatus;

	return rc;
}


