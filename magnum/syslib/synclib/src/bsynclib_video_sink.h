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
#ifndef BSYNCLIB_VIDEO_SINK_H__
#define BSYNCLIB_VIDEO_SINK_H__

#include "bstd.h"
#include "bsynclib_video_format.h"
#include "bsynclib_delay_element.h"

typedef struct BSYNClib_VideoSink_Data
{
	unsigned int uiMinDelay; /* TODO: needed? */
	
	bool bForcedCaptureEnabled; /* is forced capture enabled on this window */
	bool bMasterFrameRateEnabled; /* is master frame rate enabled on the main window for this display */
	bool bFullScreen; /* does window rect match display rect? */
	bool bVisible; /* is this window visible? */
	bool bSyncLocked; /* is this window sync-locked */
} BSYNClib_VideoSink_Data;

typedef struct BSYNClib_VideoSink_Results
{
	bool bValidated; /* TODO: needed? */
} BSYNClib_VideoSink_Results;

/*
Summary:
*/
typedef struct BSYNClib_VideoSink
{
	BSYNClib_DelayElement sElement;

	BSYNClib_VideoFormat sFormat;

	BSYNClib_VideoSink_Data sData;
	BSYNClib_VideoSink_Data sSnapshot;
	BSYNClib_VideoSink_Results sResults;
	BSYNClib_VideoSink_Config sConfig;
	BSYNClib_Sink_Status sStatus;
} BSYNClib_VideoSink;

BSYNClib_VideoSink * BSYNClib_VideoSink_Create(void);
void BSYNClib_VideoSink_Destroy(BSYNClib_VideoSink * psSink);
bool BSYNClib_VideoSink_SyncCheck(BSYNClib_VideoSink * psSink);
void BSYNClib_VideoSink_Reset_isr(BSYNClib_VideoSink * psSink);
void BSYNClib_VideoSink_GetDefaultConfig(BSYNClib_VideoSink_Config * psConfig);
void BSYNClib_VideoSink_P_SelfClearConfig_isr(BSYNClib_VideoSink * psSink);
BERR_Code BSYNClib_VideoSink_P_ProcessConfig_isr(BSYNClib_VideoSink * psSink);
void BSYNClib_VideoSink_Snapshot_isr(BSYNClib_VideoSink * psSink);
void BSYNClib_VideoSink_P_GetDefaultStatus(BSYNClib_Sink_Status * psStatus);

#endif /* BSYNCLIB_VIDEO_SINK_H__ */
