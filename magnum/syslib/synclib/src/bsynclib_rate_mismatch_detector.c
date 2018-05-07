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
#include "bsynclib_rate_mismatch_detector.h"
#include "bsynclib_rate_mismatch_detector_priv.h"
#include "bsynclib_video_source.h"

BDBG_MODULE(synclib);

/*
Summary:
 */
void BSYNClib_RateMismatchDetector_GetDefaultSettings(
	BSYNClib_RateMismatchDetector_Settings * psSettings /* [out] */
)
{
	BDBG_ENTER(BSYNClib_RateMismatchDetector_GetDefaultSettings);
	BDBG_ASSERT(psSettings);
	BKNI_Memset(psSettings, 0, sizeof(BSYNClib_RateMismatchDetector_Settings));
	psSettings->bEnabled = true;
	psSettings->sMismatch.uiTimeout = BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_TIMER_DEFAULT_TIMEOUT;
	psSettings->sMismatch.uiAcceptableMtbcLower = BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_MTBC_LOWER;
	psSettings->sMismatch.uiAcceptableMtbcUpper = BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_MTBC_UPPER;
	psSettings->sMismatch.uiAcceptableTtlc = BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_TTLC;
	psSettings->sRematch.uiTimeout = BSYNCLIB_VIDEO_RATE_REMATCH_DETECTION_TIMER_DEFAULT_TIMEOUT;
	psSettings->sRematch.uiAcceptableTtlc = BSYNCLIB_VIDEO_RATE_REMATCH_DETECTION_DEFAULT_ACCEPTABLE_TTLC;
	BDBG_LEAVE(BSYNClib_RateMismatchDetector_GetDefaultSettings);
}

static void BSYNClib_RateMismatchDetector_P_GetDefaultConfig(BSYNClib_RateMismatchDetector_Config * psConfig)
{
	BDBG_ENTER(BSYNClib_RateMismatchDetector_P_GetDefaultConfig);
	BDBG_ASSERT(psConfig);
	psConfig->uiVideoSourceCount = 0;
	psConfig->uiVideoSinkCount = 0;
	BDBG_LEAVE(BSYNClib_RateMismatchDetector_P_GetDefaultConfig);
}

static void BSYNClib_RateMismatchDetector_P_GetDefaultStatus(BSYNClib_RateMismatchDetector_Status * psStatus)
{
	BDBG_ENTER(BSYNClib_RateMismatchDetector_P_GetDefaultStatus);
	BDBG_ASSERT(psStatus);
	psStatus->bSourceSinkMatched = true;
	psStatus->bSinkSinkMatched = true;
	BDBG_LEAVE(BSYNClib_RateMismatchDetector_P_GetDefaultStatus);
}

/*
Summary:
 */
BERR_Code BSYNClib_RateMismatchDetector_Open(
	BSYNClib_Channel_Handle hParent,
	const BSYNClib_RateMismatchDetector_Settings * psSettings,
	BSYNClib_RateMismatchDetector_Handle * phDetector
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_RateMismatchDetector_Handle hDetector;

	BDBG_ENTER(BSYNClib_RateMismatchDetector_Open);

	BDBG_ASSERT(hParent);
	BDBG_ASSERT(phDetector);

	hDetector = (BSYNClib_RateMismatchDetector_Handle)BKNI_Malloc(sizeof(struct BSYNClib_RateMismatchDetector_Impl));
	if (!hDetector)
	{
		rc = BERR_OUT_OF_SYSTEM_MEMORY;
		goto error;
	}

	BKNI_Memset(hDetector, 0, sizeof(struct BSYNClib_RateMismatchDetector_Impl));

	if (psSettings)
	{
		hDetector->sSettings = *psSettings;
	}

	hDetector->hParent = hParent;

	BDBG_MSG(("chn %p: Acquiring static rate match detection task timer", (void*)hParent));
	hDetector->psStaticRateMatchTimer = BSYNClib_ResourcePool_Acquire(hParent->psTimers);
	BDBG_MSG(("chn %p: Acquiring static rate mismatch detection task timer", (void*)hParent));
	hDetector->psStaticRateMismatchTimer = BSYNClib_ResourcePool_Acquire(hParent->psTimers);

	hDetector->sData.hSources = BSYSlib_List_Create();
	hDetector->sData.uiSourceCount = 0;
	hDetector->sData.hSinks = BSYSlib_List_Create();
	hDetector->sData.uiSinkCount = 0;

	BSYNClib_RateMismatchDetector_P_GetDefaultConfig(&hDetector->sConfig);

	BSYNClib_RateMismatchDetector_P_GetDefaultStatus(&hDetector->sStatus);

	*phDetector = hDetector;

	goto end;

error:

	*phDetector = NULL;

end:

	BDBG_LEAVE(BSYNClib_RateMismatchDetector_Open);
	return rc;
}

/*
Summary:
 */
void BSYNClib_RateMismatchDetector_Close(
	BSYNClib_RateMismatchDetector_Handle hDetector
)
{
	BSYNClib_RateMismatchDetector_Config sConfig;

	BDBG_ENTER(BSYNClib_RateMismatchDetector_Close);

	BDBG_ASSERT(hDetector);

	BSYNClib_RateMismatchDetector_GetConfig(hDetector, &sConfig);
	sConfig.uiVideoSourceCount = 0;
	sConfig.uiVideoSinkCount = 0;
	BSYNClib_RateMismatchDetector_SetConfig(hDetector, &sConfig);

	if (hDetector->psStaticRateMatchTimer)
	{
		BDBG_MSG(("[%d] Releasing static rate match detection task timer", hDetector->hParent->iIndex));
		BSYNClib_ResourcePool_Release(hDetector->hParent->psTimers, hDetector->psStaticRateMatchTimer);
		hDetector->psStaticRateMatchTimer = NULL;
	}

	if (hDetector->psStaticRateMismatchTimer)
	{
		BDBG_MSG(("[%d] Releasing static rate mismatch detection task timer", hDetector->hParent->iIndex));
		BSYNClib_ResourcePool_Release(hDetector->hParent->psTimers, hDetector->psStaticRateMismatchTimer);
		hDetector->psStaticRateMismatchTimer = NULL;
	}

	BSYSlib_List_Destroy(hDetector->sData.hSources);
	BSYSlib_List_Destroy(hDetector->sData.hSinks);

	BKNI_Free(hDetector);

	BDBG_LEAVE(BSYNClib_RateMismatchDetector_Close);
}

void BSYNClib_RateMismatchDetector_GetConfig(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	BSYNClib_RateMismatchDetector_Config * psConfig
)
{
	BDBG_ENTER(BSYNClib_RateMismatchDetector_GetConfig);
	BDBG_ASSERT(hDetector);
	BDBG_ASSERT(psConfig);
	*psConfig = hDetector->sConfig;
	BDBG_LEAVE(BSYNClib_RateMismatchDetector_GetConfig);
}

BERR_Code BSYNClib_RateMismatchDetector_SetConfig(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	const BSYNClib_RateMismatchDetector_Config * psConfig
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ENTER(BSYNClib_RateMismatchDetector_SetConfig);
	BDBG_ASSERT(hDetector);
	BDBG_ASSERT(psConfig);
	hDetector->sConfig = *psConfig;
	rc = BSYNClib_RateMismatchDetector_P_ProcessConfig(hDetector);
	BDBG_LEAVE(BSYNClib_RateMismatchDetector_SetConfig);
	return rc;
}

BERR_Code BSYNClib_RateMismatchDetector_P_ProcessConfig(
	BSYNClib_RateMismatchDetector_Handle hDetector
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_RateMismatchDetector_Config * psConfig;
	BSYNClib_RateMismatchDetector_Data * psData;
	BSYSlib_List_IteratorHandle hIterator;
	BSYNClib_Channel_Handle hChn;
	unsigned int i = 0;

	BDBG_ENTER(BSYNClib_RateMismatchDetector_P_ProcessConfig);

	BDBG_ASSERT(hDetector);

	hChn = hDetector->hParent;
	psConfig = &hDetector->sConfig;
	psData = &hDetector->sData;

	/* video sources */
	if (psConfig->uiVideoSourceCount != psData->uiSourceCount)
	{
		/* take down previous */
		hIterator = BSYSlib_List_AcquireIterator(hDetector->sData.hSources);

		while (BSYSlib_List_HasNext(hIterator))
		{
			BSYNClib_RateMismatchDetector_Source * psSource;

			psSource = (BSYNClib_RateMismatchDetector_Source *)BSYSlib_List_Next(hIterator);
			BSYSlib_List_RemoveElement(hDetector->sData.hSources, psSource);

			if (psSource->psTimer)
			{
				BDBG_MSG(("[%d] Releasing video source %d rate mismatch detection timer", hChn->iIndex, psSource->uiIndex));
				BSYNClib_ResourcePool_Release(hChn->psTimers, psSource->psTimer);
				psSource->psTimer = NULL;
			}

			BKNI_Free(psSource);
		}

		BSYSlib_List_ReleaseIterator(hIterator);

		/* bring up new */
		for (i = 0; i < psConfig->uiVideoSourceCount; i++)
		{
			BSYNClib_RateMismatchDetector_Source * psSource;
			psSource = (BSYNClib_RateMismatchDetector_Source * )BKNI_Malloc(sizeof(BSYNClib_RateMismatchDetector_Source));
			if (!psSource) goto error;
			BKNI_Memset(psSource, 0, sizeof(BSYNClib_RateMismatchDetector_Source));
			BSYSlib_List_AddElement(hDetector->sData.hSources, psSource);
			psSource->uiIndex = i;

			BDBG_MSG(("[%d] Acquiring video source %d rate mismatch detection timer", hChn->iIndex, i));
			psSource->psTimer = BSYNClib_ResourcePool_Acquire(hChn->psTimers);
		}

		psData->uiSourceCount = psConfig->uiVideoSourceCount;
	}

	/* video sinks */
	if (psConfig->uiVideoSinkCount != psData->uiSinkCount)
	{
		/* take down previous */
		hIterator = BSYSlib_List_AcquireIterator(hDetector->sData.hSinks);

		while (BSYSlib_List_HasNext(hIterator))
		{
			BSYNClib_RateMismatchDetector_Sink * psSink;

			psSink = (BSYNClib_RateMismatchDetector_Sink *)BSYSlib_List_Next(hIterator);
			BSYSlib_List_RemoveElement(hDetector->sData.hSinks, psSink);

			BKNI_Free(psSink);
		}

		BSYSlib_List_ReleaseIterator(hIterator);

		/* bring up new */
		for (i = 0; i < psConfig->uiVideoSinkCount; i++)
		{
			BSYNClib_RateMismatchDetector_Sink * psSink;
			psSink = (BSYNClib_RateMismatchDetector_Sink *)BKNI_Malloc(sizeof(BSYNClib_RateMismatchDetector_Sink));
			if (!psSink) goto error;
			BKNI_Memset(psSink, 0, sizeof(BSYNClib_RateMismatchDetector_Sink));
			BSYSlib_List_AddElement(hDetector->sData.hSinks, psSink);
			psSink->uiIndex = i;
		}

		psData->uiSinkCount = psConfig->uiVideoSinkCount;
	}

	goto end;

error:

end:

	BDBG_LEAVE(BSYNClib_Channel_P_ProcessConfig);
	return rc;
}


BERR_Code BSYNClib_RateMismatchDetector_ResetSourceMeasurements_isr(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	unsigned int uiSource
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_RateMismatchDetector_Source * psSource;

	BDBG_ENTER(BSYNClib_RateMismatchDetector_ResetSourceMeasurements_isr);

	BDBG_ASSERT(hDetector);

	psSource = (BSYNClib_RateMismatchDetector_Source *)BSYSlib_List_GetByIndex_isr(hDetector->sData.hSources, uiSource);

	if (psSource)
	{
		/* if we are starting, clear other vars */
		psSource->lMeanTimeBetweenCallbacks = 0;
		psSource->uiDelayCallbackCount = 0;
		psSource->ulLastCallbackTime = 0;
		/* TODO: restart any active rmd timers */
	}

	BDBG_LEAVE(BSYNClib_RateMismatchDetector_ResetSourceMeasurements_isr);
	return rc;
}

/*
Summary:
 */
BERR_Code BSYNClib_RateMismatchDetector_ResetSourceData_isr(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	unsigned int uiSource
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_RateMismatchDetector_Source * psSource;

	BDBG_ENTER(BSYNClib_RateMismatchDetector_ResetSourceData_isr);

	BDBG_ASSERT(hDetector);

	psSource = (BSYNClib_RateMismatchDetector_Source *)BSYSlib_List_GetByIndex_isr(hDetector->sData.hSources, uiSource);

	if (psSource)
	{
		/* if we are starting, clear other vars */
		psSource->lMeanTimeBetweenCallbacks = 0;
		psSource->uiDelayCallbackCount = 0;
		psSource->ulLastCallbackTime = 0;
		psSource->sFormat.sData.bValid = false;
		hDetector->sStatus.bSourceSinkMatched = true;
		/* TODO: restart any active rmd timers */
	}

	BDBG_LEAVE(BSYNClib_RateMismatchDetector_ResetSourceData_isr);
	return rc;
}

/*
Summary:
 */
BERR_Code BSYNClib_RateMismatchDetector_SetVideoSourceFormat_isr(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	unsigned int uiSource,
	const BSYNClib_VideoFormat * psFormat
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_RateMismatchDetector_Source * psSource;
	BSYSlib_List_IteratorHandle hIterator;

	BDBG_ENTER(BSYNClib_RateMismatchDetector_SetVideoSourceFormat_isr);

	BDBG_ASSERT(hDetector);
	BDBG_ASSERT(psFormat);

	psSource = (BSYNClib_RateMismatchDetector_Source *)BSYSlib_List_GetByIndex_isr(hDetector->sData.hSources, uiSource);

	if (psSource)
	{
		BSYNClib_RateMismatchDetector_Sink * psSink;
		BSYNClib_RateMismatchDetector_Sink * psSyncLockedSink = NULL;

		rc = BSYNClib_RateMismatchDetector_ResetSourceMeasurements_isr(hDetector, uiSource);
		psSource->sFormat.sData = psFormat->sData;
		psSource->sFormat.sData.bValid = true;

		hIterator = BSYSlib_List_AcquireIterator_isr(hDetector->sData.hSinks);
		while (BSYSlib_List_HasNext_isr(hIterator))
		{
			psSink = (BSYNClib_RateMismatchDetector_Sink *)BSYSlib_List_Next_isr(hIterator);
			if (psSink->bSyncLocked)
			{
				psSyncLockedSink = psSink;
				break;
			}
		}
		BSYSlib_List_ReleaseIterator_isr(hIterator);

		if (psSyncLockedSink)
		{
			BSYNClib_RateMismatchDetector_P_TestSourceAndSinkFormats_isr(hDetector, psSource, psSyncLockedSink);
		}
		else
		{
			BDBG_MSG(("No sync-locked sink found."));
		}
	}

	BDBG_LEAVE(BSYNClib_RateMismatchDetector_SetVideoSourceFormat_isr);
	return rc;
}

/*
Summary:
 */
BERR_Code BSYNClib_RateMismatchDetector_SetVideoSinkFormat_isr(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	unsigned int uiSink,
	const BSYNClib_VideoFormat * psFormat,
	bool bMasterFrameRateEnabled,
	bool bSyncLocked
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_RateMismatchDetector_Sink * psSink;

	BDBG_ENTER(BSYNClib_RateMismatchDetector_SetVideoSinkFormat_isr);

	BDBG_ASSERT(hDetector);
	BDBG_ASSERT(psFormat);

	psSink = (BSYNClib_RateMismatchDetector_Sink *)BSYSlib_List_GetByIndex_isr(hDetector->sData.hSinks, uiSink);

	if (psSink)
	{
		psSink->sFormat.sData = psFormat->sData;
		psSink->sFormat.sData.bValid = true;
		psSink->bMasterFrameRateEnabled = bMasterFrameRateEnabled;
		psSink->bSyncLocked = bSyncLocked;

		BSYNClib_RateMismatchDetector_P_CompareSinkFormats_isr(hDetector);

		if (psSink->bSyncLocked)
		{
			BSYNClib_RateMismatchDetector_Source * psSource;

			/* TODO: assumes only 1 source, which is index 0 */
			psSource = (BSYNClib_RateMismatchDetector_Source *)BSYSlib_List_GetByIndex_isr(hDetector->sData.hSources, 0);

			if (psSource)
			{
				rc = BSYNClib_RateMismatchDetector_ResetSourceMeasurements_isr(hDetector, 0);
				BSYNClib_RateMismatchDetector_P_TestSourceAndSinkFormats_isr(hDetector, psSource, psSink);
			}
		}
	}

	BDBG_LEAVE(BSYNClib_RateMismatchDetector_SetVideoSinkFormat_isr);
	return rc;
}

/*
Summary:
 */
BERR_Code BSYNClib_RateMismatchDetector_DelayNotificationHandler_isr(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	unsigned int uiSource,
	const BSYNClib_RateMismatchDetector_DelayNotificationInfo * psInfo
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_RateMismatchDetector_Source * psSource;
	long lMeanTimeBetweenCallbacks;
	BSYNClib_Channel_Handle hChn;

	BDBG_ENTER(BSYNClib_RateMismatchDetector_DelayNotificationHandler_isr);

	BDBG_ASSERT(hDetector);
	BDBG_ASSERT(psInfo);

	psSource = (BSYNClib_RateMismatchDetector_Source *)BSYSlib_List_GetByIndex_isr(hDetector->sData.hSources, uiSource);
	hChn = hDetector->hParent;

	if (psSource)
	{
		lMeanTimeBetweenCallbacks = psSource->lMeanTimeBetweenCallbacks;
		lMeanTimeBetweenCallbacks *= psSource->uiDelayCallbackCount;
		lMeanTimeBetweenCallbacks += (long)psInfo->ulDelayNotificationTime -
			(long)psSource->ulLastCallbackTime;
		psSource->uiDelayCallbackCount++;
		psSource->ulLastCallbackTime = psInfo->ulDelayNotificationTime;
		lMeanTimeBetweenCallbacks /= psSource->uiDelayCallbackCount;
		psSource->lMeanTimeBetweenCallbacks = lMeanTimeBetweenCallbacks;

		if (!psSource->psTimer->bScheduled)
		{
			/* start video rate mismatch timer */
			if (hDetector->sStatus.bSourceSinkMatched)
			{
				if (hDetector->sSettings.sMismatch.uiTimeout)
				{
					BDBG_MSG(("[%d]  rate mismatch detection timer started", hChn->iIndex));
					rc = BSYNClib_Channel_P_StartTimer_isr(
						hChn,
						psSource->psTimer,
						hDetector->sSettings.sMismatch.uiTimeout,
						&BSYNClib_RateMismatchDetector_MismatchTimerExpired,
						hDetector,
						uiSource
					);
				}
				else
				{
					BDBG_MSG(("[%d]  rate mismatch detection timer disabled", hChn->iIndex));
				}
			}
			else
			{
				/* TODO: this never happens because we wouldn't get a callback if we didn't think we were matched */
				if (hDetector->sSettings.sRematch.uiTimeout)
				{
					BDBG_MSG(("[%d]  rate rematch detection timer started", hChn->iIndex));
					rc = BSYNClib_Channel_P_StartTimer_isr(
						hChn,
						psSource->psTimer,
						hDetector->sSettings.sRematch.uiTimeout,
						&BSYNClib_RateMismatchDetector_RematchTimerExpired,
						hDetector,
						uiSource
					);
				}
				else
				{
					BDBG_MSG(("[%d]  rate rematch detection timer disabled", hChn->iIndex));
				}
			}
		}
	}

	BDBG_LEAVE(BSYNClib_RateMismatchDetector_DelayNotificationHandler_isr);
	return rc;
}

/*
Summary:
 */
BERR_Code BSYNClib_RateMismatchDetector_GetStatus(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	BSYNClib_RateMismatchDetector_Status * psStatus
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ENTER(BSYNClib_RateMismatchDetector_GetStatus);
	BDBG_ASSERT(hDetector);
	BDBG_ASSERT(psStatus);
	BKNI_EnterCriticalSection();
	*psStatus = hDetector->sStatus;
	BKNI_LeaveCriticalSection();
	BDBG_LEAVE(BSYNClib_RateMismatchDetector_GetStatus);
	return rc;
}

BERR_Code BSYNClib_RateMismatchDetector_MismatchTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_RateMismatchDetector_Handle hDetector = pvParm1;
	unsigned int uiSource = iParm2;
	unsigned long ulLast;
	unsigned long ulNow;
	unsigned int uiCount;
	long lMtbc;
	long lAcceptableMtbcLower;
	long lAcceptableMtbcUpper;
	long lTtlc;
	long lAcceptableTtlc;
	bool bIsMatched = false;
	bool bWasMatched = false;
	BSYNClib_RateMismatchDetector_Source * psSource;
	BSYNClib_VideoSource * psVideoSource;
	BSYNClib_Channel_Handle hChn;

	BDBG_ENTER((BSYNClib_RateMismatchDetector_MismatchTimerExpired));

	BDBG_ASSERT(hDetector);
	BDBG_ASSERT(hTimer);

	hChn = hDetector->hParent;

	if (!hDetector->sSettings.bEnabled)
	{
		goto end;
	}

	psSource = (BSYNClib_RateMismatchDetector_Source *)BSYSlib_List_GetByIndex(hDetector->sData.hSources, uiSource);

	if (hChn->sSettings.cbTime.pfGetTime_isr)
	{
		BKNI_EnterCriticalSection();
		hChn->sSettings.cbTime.pfGetTime_isr(hChn->sSettings.cbTime.pvParm1, hChn->sSettings.cbTime.iParm2, &ulNow);
		BKNI_LeaveCriticalSection();
	}
	else
	{
		goto error;
	}

	/* TODO: move this to callback at channel level */
	psVideoSource = (BSYNClib_VideoSource *)BSYSlib_List_GetByIndex(hDetector->hParent->sVideo.hSources, uiSource);
	if (!psVideoSource)
	{
		BDBG_ERR(("[%d] Could not find video source %u", hChn->iIndex, uiSource));
		rc = BERR_INVALID_PARAMETER;
		goto error;
	}

	bWasMatched = hDetector->sStatus.bSourceSinkMatched;

	BDBG_MSG(("[%d] Video source rate %smatch detection timer expired.", hChn->iIndex, bWasMatched ? "mis" : "re"));

	BKNI_EnterCriticalSection();
	uiCount = psSource->uiDelayCallbackCount;
	ulLast = psSource->ulLastCallbackTime;
	lMtbc = psSource->lMeanTimeBetweenCallbacks;
	/* clear these for the next detection cycle */
	psSource->uiDelayCallbackCount = 0;
	psSource->lMeanTimeBetweenCallbacks = 0;
	BKNI_LeaveCriticalSection();

	lTtlc = ulNow - ulLast;

	lAcceptableMtbcLower = hDetector->sSettings.sMismatch.uiAcceptableMtbcLower;
	lAcceptableMtbcUpper = hDetector->sSettings.sMismatch.uiAcceptableMtbcUpper;
	lAcceptableTtlc = hDetector->sSettings.sMismatch.uiAcceptableTtlc;

	BDBG_MSG(("[%d]  mtbc: %ld; ttlc: %ld", hChn->iIndex, lMtbc, lTtlc));
	BDBG_MSG(("[%d]  rate match?: %u <= 1 or ((%ld < %ld and %ld > %ld) or %ld > %ld)", hChn->iIndex, uiCount, lMtbc, lAcceptableMtbcLower, lTtlc, lAcceptableTtlc, lMtbc, lAcceptableMtbcUpper));

	bIsMatched = (uiCount <= 1) || ((((lMtbc < lAcceptableMtbcLower) && (lTtlc > lAcceptableTtlc)) || lMtbc > lAcceptableMtbcUpper) ? true : false);

	/* TODO: move these checks to state changed callback */
	if (bWasMatched && !bIsMatched)
	{
		rc = BSYNClib_VideoSource_RateMismatchDetected(psVideoSource);
		if (rc) goto error;
	}
	else if (!bWasMatched && bIsMatched)
	{
		rc = BSYNClib_VideoSource_RateRematchDetected(psVideoSource);
		if (rc) goto error;
	}

	hDetector->sStatus.bSourceSinkMatched = bIsMatched;

	/* TODO: is state changed enough?  Think we need mismatch and rematch */
	/* state changed */
	if (bWasMatched ^ bIsMatched)
	{
		rc = BSYSlib_Callback_Invoke(&hDetector->sSettings.cbStateChange);
		if (rc) goto error;
	}

error:

end:

	BSYNClib_Channel_P_TimerExpired(hDetector->hParent, hTimer);

	(void)BSYNClib_Channel_P_Process(hDetector->hParent, 0);

	BDBG_LEAVE((BSYNClib_RateMismatchDetector_MismatchTimerExpired));
	return rc;
}

BERR_Code BSYNClib_RateMismatchDetector_RematchTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_RateMismatchDetector_Handle hDetector = pvParm1;

	BDBG_ENTER((BSYNClib_RateMismatchDetector_RematchTimerExpired));

	BSTD_UNUSED(iParm2);

	BDBG_ASSERT(hDetector);
	BDBG_ASSERT(hTimer);

	BDBG_MSG(("[%d]Rate rematch timer expired", hDetector->hParent->iIndex));

	/* TODO: check params */
	hDetector->sStatus.bSourceSinkMatched = true;

	BSYNClib_Channel_P_TimerExpired(hDetector->hParent, hTimer);

	(void)BSYNClib_Channel_P_Process(hDetector->hParent, 0);

	BDBG_LEAVE((BSYNClib_RateMismatchDetector_RematchTimerExpired));
	return rc;
}

bool BSYNClib_RateMismatchDetector_P_LooseRateMatchCheck_isr(
	BSYNClib_VideoFormat * psFormatA,
	BSYNClib_VideoFormat * psFormatB
)
{
	bool bLooseRateMatch;
	unsigned int uiFormatA;
	unsigned int uiOriginalFormatB;
	unsigned int uiUpconvertedFormatB;
	unsigned int uiDownconvertedFormatB;

	BDBG_ENTER(BSYNClib_RateMismatchDetector_P_LooseRateMatchCheck_isr);

	BDBG_ASSERT(psFormatA);
	BDBG_ASSERT(psFormatB);

	bLooseRateMatch = false;
	uiFormatA = BSYNClib_VideoFormat_P_GetFramePeriod_isrsafe(psFormatA);
	uiOriginalFormatB = BSYNClib_VideoFormat_P_GetFramePeriod_isrsafe(psFormatB);
	uiUpconvertedFormatB = BSYNClib_VideoFormat_P_GetUpconvertedFramePeriod_isrsafe(psFormatB);
	uiDownconvertedFormatB = BSYNClib_VideoFormat_P_GetDownconvertedFramePeriod_isrsafe(psFormatB);

	if
	(
		uiFormatA
		&&
		uiOriginalFormatB
		&&
		(
			(
				(uiFormatA % uiOriginalFormatB == 0) /* format A period is perfect multiple of original format B period */
				||
				(uiOriginalFormatB % uiFormatA == 0) /* original format B period is perfect multiple of format A period */
			)
			||
			(
				(uiFormatA % uiDownconvertedFormatB == 0) /* format A period is perfect multiple of downconverted format B period */
				||
				(uiDownconvertedFormatB % uiFormatA == 0) /* downconverted format B period is perfect multiple of format A period */
			)
			||
			(
				(uiFormatA % uiUpconvertedFormatB == 0) /* format A period is perfect multiple of upconverted format B period */
				||
				(uiUpconvertedFormatB % uiFormatA == 0) /* upconverted format B period is perfect multiple of format A period */
			)
		)
	)
	{
		bLooseRateMatch = true;
	}

	BDBG_LEAVE(BSYNClib_RateMismatchDetector_P_LooseRateMatchCheck_isr);
	return bLooseRateMatch;
}

void BSYNClib_RateMismatchDetector_P_CompareSinkFormats_isr(
	BSYNClib_RateMismatchDetector_Handle hDetector
)
{
	BSYSlib_List_IteratorHandle hIterator;
	BSYNClib_RateMismatchDetector_Sink * psSink;
	BSYNClib_RateMismatchDetector_Sink * psMaster;
	BSYNClib_RateMismatchDetector_Sink * psSlave;
	bool bIsMatched;

	BDBG_ENTER(BSYNClib_RateMismatchDetector_P_TestSourceAndSinkFormats_isr);

	BDBG_ASSERT(hDetector);

	bIsMatched = true;
	psMaster = NULL;

	hIterator = BSYSlib_List_AcquireIterator_isr(hDetector->sData.hSinks);

	while (BSYSlib_List_HasNext_isr(hIterator))
	{
		psSink = (BSYNClib_RateMismatchDetector_Sink *)BSYSlib_List_Next_isr(hIterator);
		if (psSink->bSyncLocked)
		{
			psMaster = psSink;
			break;
		}
	}

	BSYSlib_List_ReleaseIterator_isr(hIterator);

	if (psMaster)
	{
		hIterator = BSYSlib_List_AcquireIterator_isr(hDetector->sData.hSinks);

		while (BSYSlib_List_HasNext_isr(hIterator))
		{
			psSlave = (BSYNClib_RateMismatchDetector_Sink *)BSYSlib_List_Next_isr(hIterator);

			if (psSlave && (psSlave->uiIndex != psMaster->uiIndex) && psMaster->sFormat.sData.bValid && psSlave->sFormat.sData.bValid)
			{
				BDBG_MSG(("[%d] sink%u = %s Fps %s, sink%u = %s Fps %s",
					hDetector->hParent->iIndex,
					psMaster->uiIndex,
					BSYNClib_VideoFormat_P_GetFrameRateName_isrsafe(psMaster->sFormat.sData.eFrameRate),
					psMaster->sFormat.sData.bInterlaced ? "interlaced" : "progressive",
						psSlave->uiIndex,
						BSYNClib_VideoFormat_P_GetFrameRateName_isrsafe(psSlave->sFormat.sData.eFrameRate),
						psSlave->sFormat.sData.bInterlaced ? "interlaced" : "progressive"));

				bIsMatched &= BSYNClib_RateMismatchDetector_P_LooseRateMatchCheck_isr(&psMaster->sFormat, &psSlave->sFormat);
			}
		}

		BSYSlib_List_ReleaseIterator_isr(hIterator);

		BDBG_MSG(("[%d] sink rates %s", hDetector->hParent->iIndex, bIsMatched ? "matched" : "not matched"));
		hDetector->sStatus.bSinkSinkMatched = bIsMatched;
	}
	else
	{
		BDBG_MSG(("Sync locked master sink not yet found"));
		hDetector->sStatus.bSinkSinkMatched = false;
	}

	BDBG_LEAVE(BSYNClib_RateMismatchDetector_P_TestSourceAndSinkFormats_isr);
}


bool BSYNClib_RateMismatchDetector_P_StrictRateMatchCheck_isr(
	BSYNClib_VideoFormat * psFormatA,
	BSYNClib_VideoFormat * psFormatB,
	bool bMasterFrameRateEnabled,
	bool bSyncLocked
)
{
	bool bStrictRateMatch;
	unsigned int uiFormatA;
	unsigned int uiOriginalFormatB;
	unsigned int uiUpconvertedFormatB;
	unsigned int uiDownconvertedFormatB;

	BDBG_ENTER(BSYNClib_RateMismatchDetector_P_StrictRateMatchCheck_isr);

	BDBG_ASSERT(psFormatA);
	BDBG_ASSERT(psFormatB);

	bStrictRateMatch = false;
	uiFormatA = BSYNClib_VideoFormat_P_GetFramePeriod_isrsafe(psFormatA);
	uiOriginalFormatB = BSYNClib_VideoFormat_P_GetFramePeriod_isrsafe(psFormatB);
	uiUpconvertedFormatB = BSYNClib_VideoFormat_P_GetUpconvertedFramePeriod_isrsafe(psFormatB);
	uiDownconvertedFormatB = BSYNClib_VideoFormat_P_GetDownconvertedFramePeriod_isrsafe(psFormatB);

	if
	(
		uiFormatA
		&&
		uiOriginalFormatB
		&&
		(
			(
				(uiFormatA % uiOriginalFormatB == 0) /* format A period is perfect multiple of original format B period */
				||
				(uiOriginalFormatB % uiFormatA == 0) /* original format B period is perfect multiple of format A period */
			)
			||
			(
				bMasterFrameRateEnabled
				&&
				bSyncLocked
				&&
				(
					(
						(uiFormatA % uiDownconvertedFormatB == 0) /* format A period is perfect multiple of downconverted format B period */
						||
						(uiDownconvertedFormatB % uiFormatA == 0) /* downconverted format B period is perfect multiple of format A period */
					)
					||
					(
						(uiFormatA % uiUpconvertedFormatB == 0) /* format A period is perfect multiple of upconverted format B period */
						||
						(uiUpconvertedFormatB % uiFormatA == 0) /* upconverted format B period is perfect multiple of format A period */
					)
				)
			)
		)
	)
	{
		bStrictRateMatch = true;
	}

	BDBG_LEAVE(BSYNClib_RateMismatchDetector_P_StrictRateMatchCheck_isr);
	return bStrictRateMatch;
}

void BSYNClib_RateMismatchDetector_P_TestSourceAndSinkFormats_isr(
	BSYNClib_RateMismatchDetector_Handle hDetector,
	BSYNClib_RateMismatchDetector_Source * psSource,
	BSYNClib_RateMismatchDetector_Sink * psSink
)
{
	bool bWasMatched;
	bool bIsMatched;

	BDBG_ENTER(BSYNClib_RateMismatchDetector_P_TestSourceAndSinkFormats_isr);

	BDBG_ASSERT(hDetector);
	BDBG_ASSERT(psSource);
	BDBG_ASSERT(psSink);

	bIsMatched = false;
	bWasMatched = hDetector->sStatus.bSourceSinkMatched;

	if (psSource->sFormat.sData.bValid && psSink->sFormat.sData.bValid)
	{
		BDBG_MSG(("[%d] source = %s Fps %s, sink = %s Fps %s (master frame rate %s)",
			hDetector->hParent->iIndex,
			BSYNClib_VideoFormat_P_GetFrameRateName_isrsafe(psSource->sFormat.sData.eFrameRate),
			psSource->sFormat.sData.bInterlaced ? "interlaced" : "progressive",
				BSYNClib_VideoFormat_P_GetFrameRateName_isrsafe(psSink->sFormat.sData.eFrameRate),
				psSink->sFormat.sData.bInterlaced ? "interlaced" : "progressive",
					psSink->bMasterFrameRateEnabled ? "enabled" : "disabled"));

		bIsMatched = BSYNClib_RateMismatchDetector_P_StrictRateMatchCheck_isr(&psSource->sFormat, &psSink->sFormat, psSink->bMasterFrameRateEnabled, psSink->bSyncLocked);

		if (!bWasMatched && bIsMatched)
		{
			hDetector->sStatus.bSourceSinkMatched = true;

			/* TODO: we need to implement scheduling task time calls, not this silly multiple timer business */
			BSYNClib_Channel_P_StartTimer_isr(
				hDetector->hParent,
				hDetector->psStaticRateMatchTimer,
				0,
				&BSYNClib_RateMismatchDetector_P_StaticRateMatchTimerExpired,
				hDetector,
				psSource->uiIndex);
		}
		else if (bWasMatched && !bIsMatched)
		{
			hDetector->sStatus.bSourceSinkMatched = false;

			/* TODO: we need to implement scheduling task time calls, not this silly multiple timer business */
			BSYNClib_Channel_P_StartTimer_isr(
				hDetector->hParent,
				hDetector->psStaticRateMismatchTimer,
				0,
				&BSYNClib_RateMismatchDetector_P_StaticRateMismatchTimerExpired,
				hDetector,
				psSource->uiIndex);
		}
	}

	BDBG_LEAVE(BSYNClib_RateMismatchDetector_P_TestSourceAndSinkFormats_isr);
}

BERR_Code BSYNClib_RateMismatchDetector_P_StaticRateMatchTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_RateMismatchDetector_Handle hDetector = pvParm1;
	BSYNClib_VideoSource * psSource;
	unsigned int uiSource = iParm2;

	BDBG_ENTER(BSYNClib_RateMismatchDetector_P_StaticRateMatchTimerExpired);

	BDBG_ASSERT(hDetector);
	BDBG_ASSERT(hTimer);

	BDBG_MSG(("[%d] static rate match task timer expired: source %u", hDetector->hParent->iIndex, uiSource));

	psSource = (BSYNClib_VideoSource *)BSYSlib_List_GetByIndex(hDetector->hParent->sVideo.hSources, uiSource);

	rc = BSYNClib_VideoSource_RateRematchDetected(psSource);
	if (rc) goto end;

	BSYNClib_Channel_P_TimerExpired(hDetector->hParent, hTimer);

	BSYNClib_Channel_P_Process(hDetector->hParent, 0);

end:

	BDBG_LEAVE(BSYNClib_RateMismatchDetector_P_StaticRateMatchTimerExpired);
	return rc;
}

BERR_Code BSYNClib_RateMismatchDetector_P_StaticRateMismatchTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYNClib_RateMismatchDetector_Handle hDetector = pvParm1;
	BSYNClib_VideoSource * psSource;
	unsigned int uiSource = iParm2;

	BDBG_ENTER(BSYNClib_RateMismatchDetector_P_StaticRateMismatchTimerExpired);

	BDBG_ASSERT(hDetector);
	BDBG_ASSERT(hTimer);

	BDBG_MSG(("[%d] static rate mismatch task timer expired: source %u", hDetector->hParent->iIndex, uiSource));

	psSource = (BSYNClib_VideoSource *)BSYSlib_List_GetByIndex(hDetector->hParent->sVideo.hSources, uiSource);

	rc = BSYNClib_VideoSource_RateMismatchDetected(psSource);
	if (rc) goto end;

	BSYNClib_Channel_P_TimerExpired(hDetector->hParent, hTimer);

	BSYNClib_Channel_P_Process(hDetector->hParent, 0);

end:

	BDBG_LEAVE(BSYNClib_RateMismatchDetector_P_StaticRateMismatchTimerExpired);
	return rc;
}
