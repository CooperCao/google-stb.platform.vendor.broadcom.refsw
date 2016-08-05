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

	if (BSYNClib_P_Enabled_isrsafe(hSync))
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
		BDBG_MSG(("  sync correction: %s", BSYNClib_P_Enabled_isrsafe(hSync) ? "enabled" : "disabled"));
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
