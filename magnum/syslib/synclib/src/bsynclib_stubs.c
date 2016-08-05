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
#include "bsynclib.h"

BDBG_MODULE(synclib_stubs);

void BSYNClib_GetDefaultSettings
(
	BSYNClib_Settings * psSettings /* [out] */
)
{
	BSTD_UNUSED(psSettings);
}

BERR_Code BSYNClib_Open
(
	const BSYNClib_Settings * psSettings,
	BSYNClib_Handle * phSync /* [out] */
)
{
	BSTD_UNUSED(psSettings);
	BSTD_UNUSED(phSync);
	return BERR_SUCCESS;
}

void BSYNClib_Close
(
	BSYNClib_Handle hSync
)
{
	BSTD_UNUSED(hSync);
}

void BSYNClib_GetChannelDefaultSettings
(
	BSYNClib_Channel_Settings * psSettings /* [out] */
)
{
	BSTD_UNUSED(psSettings);
}

BERR_Code BSYNClib_CreateChannel
(
	BSYNClib_Handle hSync, 
	const BSYNClib_Channel_Settings * psSettings, 
	BSYNClib_Channel_Handle * phChn /* [out] */
)
{
	BSTD_UNUSED(hSync);
	BSTD_UNUSED(psSettings);
	BSTD_UNUSED(phChn);
	return BERR_SUCCESS;
}

void BSYNClib_DestroyChannel
(
	BSYNClib_Handle hSync, 
	BSYNClib_Channel_Handle hChn
)
{
	BSTD_UNUSED(hSync);
	BSTD_UNUSED(hChn);
}

void BSYNClib_Channel_GetSettings
(
	const BSYNClib_Channel_Handle hChn,
	BSYNClib_Channel_Settings * psSettings /* [out] */
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(psSettings);
}

BERR_Code BSYNClib_Channel_SetSettings
(
	BSYNClib_Channel_Handle hChn,
	const BSYNClib_Channel_Settings * psSettings
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(psSettings);
	return BERR_SUCCESS;
}

void BSYNClib_Channel_GetConfig
(
	const BSYNClib_Channel_Handle hChn,
	BSYNClib_Channel_Config * psConfig /* [out] */
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(psConfig);
}

BERR_Code BSYNClib_Channel_SetConfig
(
	BSYNClib_Channel_Handle hChn,
	const BSYNClib_Channel_Config * psConfig
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(psConfig);
	return BERR_SUCCESS;
}

void BSYNClib_Channel_GetStatus
(
	const BSYNClib_Channel_Handle hChn, 
	BSYNClib_Channel_Status * psStatus /* [out] */
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(psStatus);
}

void BSYNClib_Channel_GetVideoSourceConfig_isr
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	BSYNClib_VideoSource_Config * psConfig /* [out] */
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(uiSource);
	BSTD_UNUSED(psConfig);
}

BERR_Code BSYNClib_Channel_SetVideoSourceConfig_isr
(
	BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	const BSYNClib_VideoSource_Config * psConfig
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(uiSource);
	BSTD_UNUSED(psConfig);
	return BERR_SUCCESS;
}

void BSYNClib_Channel_GetVideoSinkConfig_isr
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	BSYNClib_VideoSink_Config * psConfig /* [out] */
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(uiSink);
	BSTD_UNUSED(psConfig);
}

BERR_Code BSYNClib_Channel_SetVideoSinkConfig_isr
(
	BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	const BSYNClib_VideoSink_Config * psConfig
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(uiSink);
	BSTD_UNUSED(psConfig);
	return BERR_SUCCESS;
}

void BSYNClib_Channel_GetAudioSourceConfig_isr
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	BSYNClib_AudioSource_Config * psConfig /* [out] */
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(uiSource);
	BSTD_UNUSED(psConfig);
}

BERR_Code BSYNClib_Channel_SetAudioSourceConfig_isr
(
	BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	const BSYNClib_AudioSource_Config * psConfig
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(uiSource);
	BSTD_UNUSED(psConfig);
	return BERR_SUCCESS;
}

void BSYNClib_Channel_GetAudioSinkConfig_isr
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	BSYNClib_AudioSink_Config * psConfig /* [out] */
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(uiSink);
	BSTD_UNUSED(psConfig);
}

BERR_Code BSYNClib_Channel_SetAudioSinkConfig_isr
(
	BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	const BSYNClib_AudioSink_Config * psConfig
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(uiSink);
	BSTD_UNUSED(psConfig);
	return BERR_SUCCESS;
}

void BSYNClib_Channel_GetVideoSourceStatus
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	BSYNClib_Source_Status * psStatus /* [out] */
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(uiSource);
	BSTD_UNUSED(psStatus);
}

void BSYNClib_Channel_GetVideoSinkStatus
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	BSYNClib_Sink_Status * psStatus /* [out] */
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(uiSink);
	BSTD_UNUSED(psStatus);
}

void BSYNClib_Channel_GetAudioSourceStatus
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	BSYNClib_Source_Status * psStatus /* [out] */
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(uiSource);
	BSTD_UNUSED(psStatus);
}

void BSYNClib_Channel_GetAudioSinkStatus
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	BSYNClib_Sink_Status * psStatus /* [out] */
)
{
	BSTD_UNUSED(hChn);
	BSTD_UNUSED(uiSink);
	BSTD_UNUSED(psStatus);
}
