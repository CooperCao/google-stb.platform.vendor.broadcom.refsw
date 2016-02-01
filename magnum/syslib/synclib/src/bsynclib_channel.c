/***************************************************************************
*     Copyright (c) 2004-2012, Broadcom Corporation
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
#include "bkni.h"
#include "bsyslib.h"
#include "bsynclib.h"
#include "bsynclib_priv.h"
#include "bsynclib_channel_priv.h"

BDBG_MODULE(synclib);

void BSYNClib_Channel_GetSettings
(
	const BSYNClib_Channel_Handle hChn,
	BSYNClib_Channel_Settings * psSettings /* [out] */
)
{
	BDBG_ENTER(BSYNClib_Channel_GetSettings);

	BDBG_ASSERT(hChn);
	BDBG_ASSERT(psSettings);

	*psSettings = hChn->sSettings;

	BDBG_LEAVE(BSYNClib_Channel_GetSettings);
}

BERR_Code BSYNClib_Channel_SetSettings
(
	BSYNClib_Channel_Handle hChn,
	const BSYNClib_Channel_Settings * psSettings
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BSYNClib_Channel_SetSettings);

	BDBG_ASSERT(hChn);
	BDBG_ASSERT(psSettings);

	hChn->sSettings = *psSettings;

	BDBG_LEAVE(BSYNClib_Channel_SetSettings);

	return rc;
}


void BSYNClib_Channel_GetStatus(
	BSYNClib_Channel_Handle hChn,
	BSYNClib_Channel_Status * psStatus /* [out] */
)
{
	BDBG_ENTER(BSYNClib_Channel_GetStatus);
	
	BDBG_ASSERT(hChn);

	if (BSYNClib_P_Enabled(hChn->hParent))
	{
		BDBG_ASSERT(psStatus);

		*psStatus = hChn->sStatus;
	}
	
	BDBG_LEAVE(BSYNClib_Channel_GetStatus);
}

void BSYNClib_Channel_GetVideoSourceStatus
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	BSYNClib_Source_Status * psStatus /* [out] */
)
{
	BDBG_ENTER(BSYNClib_Channel_GetVideoSourceStatus);

	BDBG_ASSERT(hChn);

	if (BSYNClib_P_Enabled(hChn->hParent))
	{
		BSYNClib_VideoSource * psSource;

		psSource = (BSYNClib_VideoSource *)BSYSlib_List_GetByIndex(hChn->sVideo.hSources, uiSource);

		if (psSource)
		{
			BDBG_ASSERT(psStatus);
			*psStatus = psSource->sStatus;
		}
	}

	BDBG_LEAVE(BSYNClib_Channel_GetVideoSourceStatus);
}

void BSYNClib_Channel_GetVideoSinkStatus_isr
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	BSYNClib_Sink_Status * psStatus /* [out] */
)
{
	BSYNClib_VideoSink * psSink;
	BDBG_ENTER(BSYNClib_Channel_GetVideoSinkStatus);

	BDBG_ASSERT(hChn);
	BDBG_ASSERT(psStatus);

	psSink = (BSYNClib_VideoSink *)BSYSlib_List_GetByIndex_isr(hChn->sVideo.hSinks, uiSink);

	if (psSink)
	{
	    BKNI_Memcpy_isr(psStatus, &psSink->sStatus, sizeof(BSYNClib_Sink_Status));
	}

	BDBG_LEAVE(BSYNClib_Channel_GetVideoSinkStatus);
}

void BSYNClib_Channel_GetAudioSourceStatus
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	BSYNClib_Source_Status * psStatus /* [out] */
)
{
	BDBG_ENTER(BSYNClib_Channel_GetAudioSourceStatus);

	BDBG_ASSERT(hChn);

	if (BSYNClib_P_Enabled(hChn->hParent))
	{
		BSYNClib_AudioSource * psSource;

		psSource = (BSYNClib_AudioSource *)BSYSlib_List_GetByIndex(hChn->sAudio.hSources, uiSource);

		if (psSource)
		{
			BDBG_ASSERT(psStatus);
			*psStatus = psSource->sStatus;
		}
	}

	BDBG_LEAVE(BSYNClib_Channel_GetAudioSourceStatus);
}

void BSYNClib_Channel_GetAudioSinkStatus
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	BSYNClib_Sink_Status * psStatus /* [out] */
)
{
	BDBG_ENTER(BSYNClib_Channel_GetAudioSinkStatus);

	BDBG_ASSERT(hChn);

	if (BSYNClib_P_Enabled(hChn->hParent))
	{
		BSYNClib_AudioSink * psSink;

		psSink = (BSYNClib_AudioSink *)BSYSlib_List_GetByIndex(hChn->sAudio.hSinks, uiSink);

		if (psSink)
		{
			BDBG_ASSERT(psStatus);
			*psStatus = psSink->sStatus;
		}
	}

	BDBG_LEAVE(BSYNClib_Channel_GetAudioSinkStatus);
}

