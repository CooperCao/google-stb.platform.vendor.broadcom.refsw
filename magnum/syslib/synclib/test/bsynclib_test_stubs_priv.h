/***************************************************************************
*     Copyright (c) 2004-2007, Broadcom Corporation
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
#include "blst_queue.h"

#ifndef BSYNCLIB_TEST_STUBS_PRIV_H__
#define BSYNCLIB_TEST_STUBS_PRIV_H__

struct BSYNClib_Impl
{
	BSYNClib_Settings sSettings;
	BLST_Q_HEAD(BSYNClib_Channel_List, BSYNClib_Channel_Impl) sChannels;
};

struct BSYNClib_Channel_Impl
{
	BLST_Q_ENTRY(BSYNClib_Channel_Impl) sLink;
	BSYNClib_Handle hParent;
	BSYNClib_Channel_Settings sSettings;
	BSYNClib_Channel_Config sConfig;
	BSYNClib_VideoSource_Config sVideoSourceConfig;
	BSYNClib_VideoSink_Config sVideoSinkConfig;
	BSYNClib_AudioSource_Config sAudioSourceConfig;
	BSYNClib_AudioSink_Config sAudioSinkConfig;
	BSYNClib_Channel_Status sStatus;
	BSYNClib_Source_Status sVideoSourceStatus;
	BSYNClib_Sink_Status sVideoSinkStatus;
	BSYNClib_Source_Status sAudioSourceStatus;
	BSYNClib_Sink_Status sAudioSinkStatus;
	BSYSlib_Timer_Handle hTimer1;
	BSYSlib_Timer_Handle hTimer2;
};

#define BSYNCLIB_VIDEO_UNMUTE_DEFAULT_TIMEOUT 133 /* ms */
#define BSYNCLIB_VIDEO_TSM_LOCK_TIMER_DEFAULT_TIMEOUT 100 /* ms */
#define BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_TIMER_DEFAULT_TIMEOUT 3000 /* ms */
#define BSYNCLIB_VIDEO_RATE_REMATCH_DETECTION_TIMER_DEFAULT_TIMEOUT 10000 /* ms */
#define BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_MTBC_LOWER 300
#define BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_MTBC_UPPER 1500
#define BSYNCLIB_VIDEO_RATE_MISMATCH_DETECTION_DEFAULT_ACCEPTABLE_TTLC 1500
#define BSYNCLIB_VIDEO_RATE_REMATCH_DETECTION_DEFAULT_ACCEPTABLE_TTLC 9000
#define BSYNCLIB_AUDIO_UNMUTE_DEFAULT_TIMEOUT 200 /* ms */
#define BSYNCLIB_AUDIO_RECEIVER_DELAY_COMPENSATION_DEFAULT 0 /* ms */
#define BSYNCLIB_AUDIO_UNCONDITIONAL_UNMUTE_DEFAULT_TIMEOUT 5000

#endif /* BSYNCLIB_TEST_STUBS_PRIV_H__ */


