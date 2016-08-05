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
#ifndef BSYNCLIB_AUDIO_SOURCE_H__
#define BSYNCLIB_AUDIO_SOURCE_H__

#include "bstd.h"
#include "bsynclib_delay_element.h"
#include "bsynclib_timer.h"

typedef struct BSYNClib_AudioSource_Data
{
	bool bDigital;
	bool bSamplingRateReceived;
} BSYNClib_AudioSource_Data;

typedef struct BSYNClib_AudioSource_Results
{
	bool bMutePending;
	bool bMuteLastStarted;
} BSYNClib_AudioSource_Results;

typedef struct BSYNClib_AudioSource
{
	BSYNClib_DelayElement sElement;

	BSYNClib_Timer * psUnmuteTimer;
	BSYNClib_Timer * psUnconditionalUnmuteTimer;

	BSYNClib_AudioSource_Data sData;
	BSYNClib_AudioSource_Data sSnapshot;
	BSYNClib_AudioSource_Results sResults;
	BSYNClib_AudioSource_Config sConfig;
	BSYNClib_Source_Status sStatus;
} BSYNClib_AudioSource;

BSYNClib_AudioSource * BSYNClib_AudioSource_Create(void);
void BSYNClib_AudioSource_Destroy(BSYNClib_AudioSource * psSource);
bool BSYNClib_AudioSource_SyncCheck(BSYNClib_AudioSource * psSource);
void BSYNClib_AudioSource_Reset_isr(BSYNClib_AudioSource * psSource);
void BSYNClib_AudioSource_GetDefaultConfig(BSYNClib_AudioSource_Config * psConfig);
void BSYNClib_AudioSource_P_SelfClearConfig_isr(BSYNClib_AudioSource * psSource);
BERR_Code BSYNClib_AudioSource_P_ProcessConfig_isr(BSYNClib_AudioSource * psSource);
void BSYNClib_AudioSource_Snapshot_isr(BSYNClib_AudioSource * psSource);
void BSYNClib_AudioSource_P_GetDefaultStatus(BSYNClib_Source_Status * psStatus);
BERR_Code BSYNClib_AudioSource_SetMute(BSYNClib_AudioSource * psSource, bool bMute);

#endif /* BSYNCLIB_AUDIO_SOURCE_H__ */
