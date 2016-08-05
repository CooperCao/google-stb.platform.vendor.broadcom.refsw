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
	BDBG_ENTER(BSYNClib_Channel_SetSettings);
	BDBG_ASSERT(hChn);
	BDBG_ASSERT(psSettings);
	hChn->sSettings = *psSettings;
	BDBG_LEAVE(BSYNClib_Channel_SetSettings);
	return BERR_SUCCESS;
}


void BSYNClib_Channel_GetStatus(
	BSYNClib_Channel_Handle hChn,
	BSYNClib_Channel_Status * psStatus /* [out] */
)
{
	BDBG_ENTER(BSYNClib_Channel_GetStatus);

	BDBG_ASSERT(hChn);

	if (BSYNClib_P_Enabled_isrsafe(hChn->hParent))
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

	if (BSYNClib_P_Enabled_isrsafe(hChn->hParent))
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

	if (BSYNClib_P_Enabled_isrsafe(hChn->hParent))
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

	if (BSYNClib_P_Enabled_isrsafe(hChn->hParent))
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
