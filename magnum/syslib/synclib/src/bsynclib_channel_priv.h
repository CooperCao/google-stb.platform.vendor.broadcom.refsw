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
#ifndef BSYNCLIB_CHANNEL_PRIV_H__
#define BSYNCLIB_CHANNEL_PRIV_H__

#include "bstd.h"
#include "bsyslib_list.h"
#include "bsynclib.h"
#include "bsynclib_audio_sink.h"
#include "bsynclib_audio_source.h"
#include "bsynclib_rate_mismatch_detector.h"
#include "bsynclib_resource_pool.h"
#include "bsynclib_state_machine.h"
#include "bsynclib_timer.h"
#include "bsynclib_video_sink.h"
#include "bsynclib_video_source.h"

#define BSYNCLIB_TIMER_COUNT 18

extern const unsigned int auiAudioSamplingRates[];

typedef struct
{
	int TBD;
} BSYNClib_Channel_Path_Data;

typedef struct
{
	unsigned int uiMaxSourceDelay;
	void * pvMaxDelaySource;

    bool bInconsistentSinkDomains;
	unsigned int uiMaxSinkDelay;
	void * pvMaxDelaySink;

	unsigned int uiMaxPathDelay;
} BSYNClib_Channel_Path_Results;

typedef struct
{
	BSYNClib_Channel_Handle hChn;
	BSYSlib_List_Handle hSources;
	unsigned int uiSourceCount;

	BSYSlib_List_Handle hSinks;
	unsigned int uiSinkCount;

	BSYNClib_Channel_Path_Data sData;
	BSYNClib_Channel_Path_Results sResults;
} BSYNClib_Channel_Path;

/*
Summary:
*/
typedef struct
{
	bool bEnabled;
} BSYNClib_Channel_Data;

/*
Summary:
*/
typedef struct
{
    bool bSyncTaskRequested;
	bool bSyncTaskScheduled;
	bool bMuteTaskScheduled;

	unsigned int uiMaxPathDelay;
	BSYNClib_Channel_Path * psMaxDelayPath;
} BSYNClib_Channel_Results;

/*
Summary:
*/
struct BSYNClib_Channel_Impl
{
	bool bEnabled;

	bool bPrecisionLipsyncEnabled;

	BSYNClib_Handle hParent;
	BSYNClib_Channel_Settings sSettings;
	BSYNClib_Channel_Config sConfig;
	BSYNClib_Channel_Status sStatus;

	BSYNClib_Channel_Path sVideo;
	BSYNClib_Channel_Path sAudio;

	BSYNClib_Channel_Results sResults;

	BSYNClib_ResourcePool * psTimers;

	BSYNClib_Timer * psTaskTimer;
	BSYNClib_Timer * psMuteControlTaskTimer;

	BSYNClib_StateMachine_Handle hMachine;
	BSYNClib_RateMismatchDetector_Handle hDetector;
	int iIndex;
};

/*
Summary:
Opens an SYNC lib channel
Description:
*/
BERR_Code BSYNClib_Channel_P_Create(
	const BSYNClib_Settings * psLibSettings,
	const BSYNClib_Channel_Settings * psSettings,
	BSYNClib_Channel_Handle * phChn /* [out] */
);

/*
Summary:
Closes an SYNC lib channel
Description:
*/
void BSYNClib_Channel_P_Destroy(
	BSYNClib_Channel_Handle hChn
);

/*
Summary:
Returns the default configuration of a sync channel
*/
void BSYNClib_Channel_P_GetDefaultConfig(
	BSYNClib_Channel_Config * psConfig
);

/*
Summary:
*/
BERR_Code BSYNClib_Channel_P_StartTimer_isr(
	BSYNClib_Channel_Handle hChn,
	BSYNClib_Timer * psTimer,
	unsigned long ulTimeout,
	BSYSlib_Timer_ExpiryHandler pfTimerExpired,
	void * pvParm1,
	int iParm2
);

void BSYNClib_Channel_P_CancelTimer_isr(
	BSYNClib_Channel_Handle hChn,
	BSYNClib_Timer * psTimer
);

/*
Summary:
Handles expiry of a sync timer
*/
void BSYNClib_Channel_P_TimerExpired(
	BSYNClib_Channel_Handle hChn,
	BSYSlib_Timer_Handle hTimer
);

/*
Summary:
*/
BERR_Code BSYNClib_Channel_P_ProcessConfig(
	BSYNClib_Channel_Handle hChn
);

/*
Summary:
*/
bool BSYNClib_Channel_P_SyncCheck(
	BSYNClib_Channel_Handle hChn
);

/*
Summary:
*/
void BSYNClib_Channel_P_Snapshot_isr(
	BSYNClib_Channel_Handle hChn
);

BERR_Code BSYNClib_Channel_P_Process(void * pvParm1, int iParm2);
BERR_Code BSYNClib_Channel_P_ApplyDelays(BSYNClib_Channel_Handle hChn);
bool BSYNClib_Channel_P_AudioVideoSyncCheck(BSYNClib_Channel_Handle hChn);
bool BSYNClib_Channel_P_AudioAudioSyncCheck(BSYNClib_Channel_Handle hChn);
bool BSYNClib_Channel_P_VideoVideoSyncCheck(BSYNClib_Channel_Handle hChn);
BERR_Code BSYNClib_Channel_P_Synchronize(BSYNClib_Channel_Handle hChn);
BERR_Code BSYNClib_Channel_P_GenerateCallbacks(BSYNClib_Channel_Handle hChn);
BERR_Code BSYNClib_Channel_P_GenerateDelayCallback(BSYNClib_Channel_SetDelay pfSetDelay, void * pvParm1, int iParm2, unsigned int uiDeviceIndex, BSYNClib_DelayElement * psData);
BERR_Code BSYNClib_Channel_P_GenerateDelayNotificationCallback(BSYNClib_Channel_SetDelayNotification pfSetDelayNotification, void * pvParm1, int iParm2, unsigned int uiDeviceIndex, BSYNClib_DelayElement * psData);
BERR_Code BSYNClib_Channel_P_ScheduleTask_isr(BSYNClib_Channel_Handle hChn);
BERR_Code BSYNClib_Channel_P_EnqueueTaskRequest_isr(BSYNClib_Channel_Handle hChn);
BERR_Code BSYNClib_Channel_P_DequeueTaskRequest_isr(BSYNClib_Channel_Handle hChn);
BERR_Code BSYNClib_Channel_P_TaskTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer);

/*
Summary:
*/
bool BSYNClib_Channel_P_Enabled_isrsafe(
	BSYNClib_Channel_Handle hChn
);

/*
Summary:
*/
bool BSYNClib_Channel_P_PredictMadStateChange_isr(
	BSYNClib_Channel_Handle hChn,
	BSYNClib_VideoSink * psSink
);

/*
Summary:
*/
void BSYNClib_Channel_P_ResetVideoSourceJtiFactor_isr(
	BSYNClib_Channel_Handle hChn
);

/*
Summary:
*/
void BSYNClib_Channel_P_GetDefaultStatus(
	BSYNClib_Channel_Status * psStatus
);

/*
Summary:
*/
void BSYNClib_Channel_P_Stop(
	BSYNClib_Channel_Handle hChn
);

#endif /* BSYNCLIB_CHANNEL_PRIV_H__ */

