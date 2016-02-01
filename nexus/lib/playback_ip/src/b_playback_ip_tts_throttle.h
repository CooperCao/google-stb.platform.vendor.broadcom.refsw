/***************************************************************************
*     (c)2003-2008 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Description: IP Applib Implementation for TTS.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

/* JJ - PR47230: TTS pacing */

#ifndef __B_PLAYBACK_IP_TTS_THROTTLE_H__
#define __B_PLAYBACK_IP_TTS_THROTTLE_H__

/*
Summary:
	Handle returned by B_PlaybackIp_TtsThrottle_Open().
*/
typedef struct B_PlaybackIp_TtsThrottle *B_PlaybackIp_TtsThrottleHandle;

/**
Summary:
 Structure used to return the TTS throttle status
**/
typedef struct B_PlaybackIp_TtsThrottle_Status {
	unsigned bufDepth;				/* the current buffer depth (aka FIFO fullness) */
	unsigned avgBufDepth;			/* the average buffer depth (aka FIFO fullness) currently in use by the pacing logic */
	bool pacingClockRailed;		/* true if pacing clock is railed */
	signed pacingClockAdjustment;	/* amount by which the pacing clock is adjusted (in ppm) from its nominal center frequency */

	unsigned pacingErrorCount;	/* count of the number of TTS timestamps out-of-range (since start) */
	unsigned bufViolationCount;	/* count of the number of max/min buffer threshold violations (since start) */
} B_PlaybackIp_TtsThrottle_Status;

/**
Summary:
 Opens a TTS throttle object
**/
B_PlaybackIp_TtsThrottleHandle 
B_PlaybackIp_TtsThrottle_Open(
	void
	); 

/**
Summary:
 Closes a TTS throttle object
**/
void B_PlaybackIp_TtsThrottle_Close(
	B_PlaybackIp_TtsThrottleHandle ttsThrottle
	); 

/**
Summary:
 Initializes TTS throttle parameters to default values
**/
void B_PlaybackIp_TtsThrottle_ParamsInit(
	B_PlaybackIp_TtsThrottle_Params *params 
	);

/**
Summary:
 Initializes TTS throttle parameters to default values
**/
void B_PlaybackIp_TtsThrottle_GetSettings(
	B_PlaybackIp_TtsThrottleHandle ttsThrottle,
	B_PlaybackIp_TtsThrottle_Params *params 
	);

/**
Summary:
 Initializes TTS throttle parameters to default values
**/
void B_PlaybackIp_TtsThrottle_SetSettings(
	B_PlaybackIp_TtsThrottleHandle ttsThrottle,
	B_PlaybackIp_TtsThrottle_Params *params 
	);

/**
Summary:
 Starts the TTS throttle function
**/
B_PlaybackIpError B_PlaybackIp_TtsThrottle_Start(
	B_PlaybackIp_TtsThrottleHandle ttsThrottle
	);

/**
Summary:
 Stops the TTS throttle function
**/
void B_PlaybackIp_TtsThrottle_Stop(
	B_PlaybackIp_TtsThrottleHandle ttsThrottle
	); 

/**
Summary:
 Gets the status of the TTS throttle function
**/
B_PlaybackIpError 
B_PlaybackIp_TtsThrottle_GetStatus(
	B_PlaybackIp_TtsThrottleHandle ttsThrottle, 
	B_PlaybackIp_TtsThrottle_Status *status
	); 

/**
Summary:
 Pauses the TTS throttle function
**/
void B_PlaybackIp_TtsThrottle_Pause(
	B_PlaybackIp_TtsThrottleHandle ttsThrottle, 
	bool pause); 

/**
Summary:
 Suspends the TTS pacing function so that data flows through the playback unimpeded
**/
B_PlaybackIpError 
B_PlaybackIp_TtsThrottle_SuspendPacing(
	B_PlaybackIp_TtsThrottleHandle ttsThrottle, 
	bool suspend
	); 

#endif /* __B_PLAYBACK_IP_TTS_THROTTLE_H__ */
