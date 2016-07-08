/***************************************************************************
*     Copyright (c) 2004-2011, Broadcom Corporation
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
#include "bsyslib_list.h"
#include "bsynclib.h"
#include "bsynclib_priv.h"
#include "bsynclib_channel_priv.h"

BDBG_MODULE(synclib);

/*
Summary:
Returns the default global SYNC lib module settings
Description:
*/
void BSYNClib_GetDefaultSettings(
	BSYNClib_Settings * psSettings /* [out] */
)
{
	BDBG_ENTER(BSYNClib_GetDefaultSettings);

	BDBG_ASSERT(psSettings);

	psSettings->bEnabled = true;
	psSettings->sVideo.bRequireFullScreen = false;
	psSettings->sVideo.uiTsmLockTimeout = BSYNCLIB_VIDEO_TSM_LOCK_TIMER_DEFAULT_TIMEOUT;
	psSettings->sVideo.uiUnmuteTimeout = BSYNCLIB_VIDEO_UNMUTE_DEFAULT_TIMEOUT;
	psSettings->sVideo.uiUnconditionalUnmuteTimeout = BSYNCLIB_VIDEO_UNCONDITIONAL_UNMUTE_DEFAULT_TIMEOUT;
	psSettings->sVideo.sRateMismatchDetection.uiTimeout = BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_TIMER_DEFAULT_TIMEOUT;
	psSettings->sVideo.sRateMismatchDetection.uiAcceptableMtbcLower = BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_MTBC_LOWER;
	psSettings->sVideo.sRateMismatchDetection.uiAcceptableMtbcUpper = BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_MTBC_UPPER;
	psSettings->sVideo.sRateMismatchDetection.uiAcceptableTtlc = BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_TTLC;
	psSettings->sVideo.sRateRematchDetection.uiTimeout = BSYNCLIB_VIDEO_RATE_REMATCH_DETECTION_TIMER_DEFAULT_TIMEOUT;
	psSettings->sVideo.sRateRematchDetection.uiAcceptableTtlc = BSYNCLIB_VIDEO_RATE_REMATCH_DETECTION_DEFAULT_ACCEPTABLE_TTLC;
	psSettings->sAudio.uiUnmuteTimeout = BSYNCLIB_AUDIO_UNMUTE_DEFAULT_TIMEOUT;
	psSettings->sAudio.uiReceiverDelayCompensation = BSYNCLIB_AUDIO_RECEIVER_DELAY_COMPENSATION_DEFAULT;
	psSettings->sAudio.uiUnconditionalUnmuteTimeout = BSYNCLIB_AUDIO_UNCONDITIONAL_UNMUTE_DEFAULT_TIMEOUT;

	BDBG_LEAVE(BSYNClib_GetDefaultSettings);
}

/*
Summary:
Opens the global SYNC lib module
Description:
*/
BERR_Code BSYNClib_Open(
	const BSYNClib_Settings * psSettings,
	BSYNClib_Handle * phSync /* [out] */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_Handle hSync = NULL;

	BDBG_ENTER(BSYNClib_Open);

	BDBG_ASSERT(phSync);

	hSync = (BSYNClib_Handle)BKNI_Malloc(sizeof(struct BSYNClib_Impl));
	if (!hSync)
	{
		rc = BERR_OUT_OF_SYSTEM_MEMORY;
		*phSync = NULL;
		goto end;
	}

	BKNI_Memset(hSync, 0, sizeof(struct BSYNClib_Impl));

	if (psSettings)
	{
		hSync->sSettings = *psSettings;
	}
	else
	{
		BSYNClib_GetDefaultSettings(&hSync->sSettings);
	}

	if (BSYNClib_P_Enabled(hSync))
	{
		hSync->hChannels = BSYSlib_List_Create();

	    BDBG_MSG(("Settings Summary:"));
	    BDBG_MSG(("  sync correction: %s", hSync->sSettings.bEnabled ? "enabled" : "disabled"));
	    BDBG_MSG(("  video"));
	    BDBG_MSG(("    full screen: %s", hSync->sSettings.sVideo.bRequireFullScreen ? "required" : "not required"));
	    BDBG_MSG(("    TSM lock timeout: %d milliseconds", hSync->sSettings.sVideo.uiTsmLockTimeout));
	    BDBG_MSG(("    unmute timeout: %d milliseconds", hSync->sSettings.sVideo.uiUnmuteTimeout));
	    BDBG_MSG(("    unconditional unmute timeout: %d milliseconds", hSync->sSettings.sVideo.uiUnconditionalUnmuteTimeout));
	    BDBG_MSG(("    rate mismatch detection"));
	    BDBG_MSG(("      timeout: %d milliseconds", hSync->sSettings.sVideo.sRateMismatchDetection.uiTimeout));
	    BDBG_MSG(("      acceptable mtbc lower bound: %d milliseconds", hSync->sSettings.sVideo.sRateMismatchDetection.uiAcceptableMtbcLower));
	    BDBG_MSG(("      acceptable mtbc upper bound: %d milliseconds", hSync->sSettings.sVideo.sRateMismatchDetection.uiAcceptableMtbcUpper));
	    BDBG_MSG(("      acceptable ttlc: %d milliseconds", hSync->sSettings.sVideo.sRateMismatchDetection.uiAcceptableTtlc));
	    BDBG_MSG(("    rate rematch detection"));
	    BDBG_MSG(("      timeout: %d milliseconds", hSync->sSettings.sVideo.sRateRematchDetection.uiTimeout));
	    BDBG_MSG(("      acceptable ttlc: %d milliseconds", hSync->sSettings.sVideo.sRateRematchDetection.uiAcceptableTtlc));
	    BDBG_MSG(("  audio"));
	    BDBG_MSG(("    unmute timeout: %d milliseconds", hSync->sSettings.sAudio.uiUnmuteTimeout));
	    BDBG_MSG(("    unconditional unmute timeout: %d milliseconds", hSync->sSettings.sAudio.uiUnconditionalUnmuteTimeout));
	    BDBG_MSG(("    receiver delay compensation: %d milliseconds", hSync->sSettings.sAudio.uiReceiverDelayCompensation));
	}
	else
	{
	    BDBG_MSG(("Settings Summary:"));
	    BDBG_MSG(("  sync correction: %s", BSYNClib_P_Enabled(hSync) ? "enabled" : "disabled"));
	}

	*phSync = hSync;

end:

	BDBG_LEAVE(BSYNClib_Open);
	return rc;
}


/*
Summary:
Closes the global SYNC lib module
Description:
*/
void BSYNClib_Close(
	BSYNClib_Handle hSync
)
{
	BSYNClib_Channel_Handle hChn;
	BSYSlib_List_IteratorHandle hIterator;

	BDBG_ENTER(BSYNClib_Close);

	BDBG_ASSERT(hSync);

    hIterator = BSYSlib_List_AcquireIterator(hSync->hChannels);

    while (BSYSlib_List_HasNext(hIterator))
    {
        hChn = (BSYNClib_Channel_Handle)BSYSlib_List_Next(hIterator);
        BSYNClib_DestroyChannel(hSync, hChn);
    }

    BSYSlib_List_ReleaseIterator(hIterator);

    BSYSlib_List_Destroy(hSync->hChannels);

	/* free me */
	BKNI_Free(hSync);

	BDBG_LEAVE(BSYNClib_Close);
}

#define BSYNCLIB_P_DEFAULT_SOURCE_PREFERRED_UNITS BSYNClib_Units_e45KhzTicks
#define BSYNCLIB_P_DEFAULT_VIDEO_SINK_PREFERRED_UNITS BSYNClib_Units_e60HzVsyncs
#define BSYNCLIB_P_DEFAULT_AUDIO_SINK_PREFERRED_UNITS BSYNClib_Units_eMilliseconds

/*
Summary:
Returns the default settings for the specified channel index
Description:
*/
void BSYNClib_GetChannelDefaultSettings(
	BSYNClib_Channel_Settings * psSettings /* default channel settings [out] */
)
{
	BDBG_ENTER(BSYNClib_GetChannelDefaultSettings);

	BDBG_ASSERT(psSettings);

	BKNI_Memset(psSettings, 0, sizeof(BSYNClib_Channel_Settings));

	psSettings->sVideo.sSource.cbDelay.preferredDelayUnits = BSYNCLIB_P_DEFAULT_SOURCE_PREFERRED_UNITS;
	psSettings->sVideo.sSource.cbDelay.preferredNotificationThresholdUnits = BSYNCLIB_P_DEFAULT_SOURCE_PREFERRED_UNITS;
	psSettings->sVideo.sSink.cbDelay.preferredDelayUnits = BSYNCLIB_P_DEFAULT_VIDEO_SINK_PREFERRED_UNITS;
	psSettings->sVideo.sSink.cbDelay.preferredNotificationThresholdUnits = BSYNCLIB_P_DEFAULT_VIDEO_SINK_PREFERRED_UNITS;
	psSettings->sAudio.sSource.cbDelay.preferredDelayUnits = BSYNCLIB_P_DEFAULT_SOURCE_PREFERRED_UNITS;
	psSettings->sAudio.sSource.cbDelay.preferredNotificationThresholdUnits = BSYNCLIB_P_DEFAULT_SOURCE_PREFERRED_UNITS;
	psSettings->sAudio.sSink.cbDelay.preferredDelayUnits = BSYNCLIB_P_DEFAULT_AUDIO_SINK_PREFERRED_UNITS;
	psSettings->sAudio.sSink.cbDelay.preferredNotificationThresholdUnits = BSYNCLIB_P_DEFAULT_AUDIO_SINK_PREFERRED_UNITS;

	BDBG_LEAVE(BSYNClib_GetChannelDefaultSettings);
}

/*
Summary:
Creates a SYNC lib channel
Description:
*/
BERR_Code BSYNClib_CreateChannel(
	BSYNClib_Handle hSync,
	const BSYNClib_Channel_Settings * psSettings,
	BSYNClib_Channel_Handle * phChn /* [out] */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_Channel_Handle hChn = NULL;
	BSYNClib_Channel_Settings sSettings;

	BDBG_ENTER(BSYNClib_CreateChannel);

	BDBG_ASSERT(hSync);
	BDBG_ASSERT(phChn);

    if (!psSettings)
    {
        BSYNClib_GetChannelDefaultSettings(&sSettings);
        psSettings = &sSettings;
    }

    rc = BSYNClib_Channel_P_Create(&hSync->sSettings, psSettings, &hChn);
    if (rc) goto end;

    hChn->hParent = hSync;

    BSYSlib_List_AddElement(hSync->hChannels, hChn);
    hChn->iIndex = BSYSlib_List_IndexOf(hSync->hChannels, hChn);
    BDBG_MSG(("chn %p is index %d", (void*)hChn, hChn->iIndex));
    hChn->sVideo.hChn = hChn;
    hChn->sAudio.hChn = hChn;

	*phChn = hChn;

end:

	BDBG_LEAVE(BSYNClib_CreateChannel);
	return rc;
}

/*
Summary:
Closes a SYNC lib channel
Description:
*/
void BSYNClib_DestroyChannel(
	BSYNClib_Handle hSync,
	BSYNClib_Channel_Handle hChn
)
{
	BDBG_ENTER(BSYNClib_DestroyChannel);

    BDBG_ASSERT(hSync);
    BDBG_ASSERT(hChn);

    BSYSlib_List_RemoveElement(hSync->hChannels, hChn);

    BSYNClib_Channel_P_Destroy(hChn);

	BDBG_LEAVE(BSYNClib_DestroyChannel);
}

