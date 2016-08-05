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
#ifndef BSYNCLIB_MUTE_CONTROL_PRIV_H__
#define BSYNCLIB_MUTE_CONTROL_PRIV_H__

#include "bsynclib.h"
#include "bsyslib.h"
#include "bsynclib_audio_source.h"
#include "bsynclib_video_source.h"

bool BSYNClib_MuteControl_P_FullScreenCheck(BSYNClib_Channel_Handle hChn);
BERR_Code BSYNClib_MuteControl_P_UnmuteAll(BSYNClib_Channel_Handle hChn);
BERR_Code BSYNClib_MuteControl_P_HandleVideoSourceMutePending(BSYNClib_Channel_Handle hChn, BSYNClib_VideoSource * psSource, bool bStarted);
BERR_Code BSYNClib_MuteControl_P_HandleAudioSourceMutePending(BSYNClib_Channel_Handle hChn, BSYNClib_AudioSource * psSource, bool bStarted);
BERR_Code BSYNClib_MuteControl_P_TaskTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer);
BERR_Code BSYNClib_MuteControl_P_Process(void * pvParm1, int iParm2);
BERR_Code BSYNClib_MuteControl_P_VideoSourceUnmuteTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer);
BERR_Code BSYNClib_MuteControl_P_AudioSourceUnmuteTimerExpired(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer);
#if BSYNCLIB_UNCONDITIONAL_AUDIO_UNMUTE_SUPPORT
BERR_Code BSYNClib_MuteControl_P_AudioSourceUnconditionalUnmuteTimerExpired(void *pvParm1,int iParm2,BSYSlib_Timer_Handle hTimer);
#endif
#if BSYNCLIB_UNCONDITIONAL_VIDEO_UNMUTE_SUPPORT
BERR_Code BSYNClib_MuteControl_P_VideoSourceUnconditionalUnmuteTimerExpired(void *pvParm1,int iParm2,BSYSlib_Timer_Handle hTimer);
#endif

#endif /*BSYNCLIB_MUTE_CONTROL_PRIV_H__ */

