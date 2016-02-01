/***************************************************************************
*     Copyright (c) 2004-2013, Broadcom Corporation
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

#include "bsynclib.h"
#include "bsyslib.h"
#include "bsynclib_audio_source.h"
#include "bsynclib_video_source.h"

#ifndef BSYNCLIB_MUTE_CONTROL_PRIV_H__
#define BSYNCLIB_MUTE_CONTROL_PRIV_H__

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

