/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
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
 *  ANY LIMITED REMEDY..
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 * Playback Service gets a program name from the applications and looks up the 
 * the media data file associated with the program name and associates it with a 
 * media node. Playback service also is responsible for opening the 
 * DRM handles required for the active PIDs associated with a stream.
 * Playback service provides trick mode functionality also.
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef _B_DVR_PLAYBACKSERVICE_H
#define _B_DVR_PLAYBACKSERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "b_dvr_const.h"
#include "b_dvr_datatypes.h"
#include "b_dvr_drmservice.h"

BDBG_OBJECT_ID_DECLARE(B_DVR_PlaybackService);
typedef struct B_DVR_PlaybackService * B_DVR_PlaybackServiceHandle;

typedef struct B_DVR_PlaybackServiceRequest
{
    char programName[B_DVR_MAX_FILE_NAME_LENGTH];
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH];
    unsigned playpumpIndex;
    unsigned volumeIndex;
    bool loopMode; /* only applicable for permanent recordings and not for in-progress recording */
}B_DVR_PlaybackServiceRequest;

typedef struct B_DVR_PlaybackServiceStatus
{
   off_t linearCurrentOffset;
   off_t linearStartOffset;
   off_t linearEndOffset;
   unsigned long startTime;
   unsigned long endTime;
   unsigned long currentTime;
   NEXUS_PlaybackState state;
}B_DVR_PlaybackServiceStatus;

typedef struct B_DVR_PlaybackServiceSettings
{
    NEXUS_VideoDecoderHandle videoDecoder[2];
    NEXUS_AudioDecoderHandle audioDecoder[2];
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_SimpleVideoDecoderHandle simpleVideoDecoder[2];
    NEXUS_SimpleAudioDecoderHandle simpleAudioDecoder[2];
    NEXUS_SimpleStcChannelHandle simpleStcChannel;
    NEXUS_PlaybackHandle playback;
    unsigned long startTime; /* in milliseconds */
    unsigned long endTime;   /* in milliseconds */
}B_DVR_PlaybackServiceSettings;
/*****************************************************************************
Summary:
B_DVR_PlaybackService_Open shall open an instance of playback service.
Param[in]
playbackServiceRequest - playback service request parameters.
return value
B_DVR_PlaybackServiceHandle - playback service handle.
*****************************************************************************/
B_DVR_PlaybackServiceHandle B_DVR_PlaybackService_Open(
    B_DVR_PlaybackServiceRequest * playbackServiceRequest);

/*******************************************************************************
Summary:
B_DVR_PlaybackService_Close shall close an instance of a playback service.
Param[in]
playbackService - Playback service handle.
return value
B_DVR_ERROR
*******************************************************************************/
B_DVR_ERROR B_DVR_PlaybackService_Close(
    B_DVR_PlaybackServiceHandle playbackService);

/*******************************************************************************
Summary:
B_DVR_PlaybackService_Start shall an instance of playback service.
Param[in]
playbackService - Playback service handle.
return value
B_DVR_ERROR
********************************************************************************/
B_DVR_ERROR B_DVR_PlaybackService_Start(
    B_DVR_PlaybackServiceHandle playbackService);

/*********************************************************************************
Summary:
B_DVR_PlaybackService_Stop shall stop an instance of a playback service.
Param[in]
playbackService - Playback service handle.
return value
B_DVR_ERROR
***********************************************************************************/
B_DVR_ERROR B_DVR_PlaybackService_Stop(
    B_DVR_PlaybackServiceHandle playbackService);

/**********************************************************************************
Summary:
B_DVR_PlaybackService_Operation shall provide all the DVR operations for an 
instance of playback service.
Param[in]
playbackService - Playback service handle.
Param[in]
operationSetting - Playback operations like DVR trick modes, seek etc.
return value
B_DVR_ERROR
**********************************************************************************/
B_DVR_ERROR B_DVR_PlaybackService_SetOperation(
    B_DVR_PlaybackServiceHandle,
    B_DVR_OperationSettings *operationSetting);

/*********************************************************************************
Summary:
B_DVR_PlaybackService_GetStatus shall return the status of an instance of a
playback service.
Param[in]
playbackService - Playback service handle.
Param[out]
pStatus - status of a playback service instance
return value
B_DVR_ERROR
**********************************************************************************/
B_DVR_ERROR B_DVR_PlaybackService_GetStatus(
    B_DVR_PlaybackServiceHandle playbackService,
    B_DVR_PlaybackServiceStatus *pStatus);

/*********************************************************************************
Summary:
B_DVR_PlaybackService_GetSettings shall get the settings of an instance of a playback
service.
Param[in]
playbackService - Playback service handle.
Param[out]
playbackServiceSettings - settings of a playback service instance
return value
B_DVR_ERROR
**********************************************************************************/
B_DVR_ERROR B_DVR_PlaybackService_GetSettings(
    B_DVR_PlaybackServiceHandle playbackService,
    B_DVR_PlaybackServiceSettings *playbackServiceSettings);

/*********************************************************************************
Summary:
B_DVR_PlaybackService_SetSettings shall set the settings of an instance of a playback
service.
Param[in]
playbackService - Playback service handle.
Param[in]
playbackServiceSettings - settings of a playback service instance
return value
B_DVR_ERROR
**********************************************************************************/
B_DVR_ERROR B_DVR_PlaybackService_SetSettings(
    B_DVR_PlaybackServiceHandle playbackService,
    B_DVR_PlaybackServiceSettings *playbackServiceSettings);

/***************************************************************************
Summary:
B_DVR_PlaybackService_InstallCallback shall install the application provided
callback to a playback service instance.
Param[in]
playbackService - Playback service handle.
Param[in]
registeredCallback - application provided callback to be registered with a 
playback service instance.
Param[in]
appContext - application context pointer.
return value
B_DVR_ERROR
*****************************************************************************/
B_DVR_ERROR B_DVR_PlaybackService_InstallCallback(
    B_DVR_PlaybackServiceHandle playbackService,
    B_DVR_ServiceCallback registeredCallback,
    void * appContext);

/***************************************************************************
Summary:
B_DVR_PlaybackService_RemoveCallback shall remove the application provided
callback from a playback service instance.
Param[in]
playbackService - Playback service handle.
return value
B_DVR_ERROR
*****************************************************************************/
B_DVR_ERROR B_DVR_PlaybackService_RemoveCallback(
    B_DVR_PlaybackServiceHandle playbackService);

/***************************************************************************
Summary:
B_DVR_PlaybackService_GetMediaNode shall return the mediaNode associated
with the playbackService. This was required for B_DVR_MediaFile API.
Param[in]
playbackService - Playback service handle.
Param[out]
media - mediaNode
return value
B_DVR_ERROR
*****************************************************************************/
B_DVR_MediaNode B_DVR_PlaybackService_GetMediaNode(
    B_DVR_PlaybackServiceHandle playbackService);

/***************************************************************************
Summary:
B_DVR_PlaybackService_GetFileHandle shall a filePlay handle associated 
with a playback service. This was required by the B_DVR_MediaFile API.
Param[in]
playbackService - Playback service handle.
Param[out]
nexusFilePlay - file play handle.
return value
B_DVR_ERROR
*****************************************************************************/
B_DVR_ERROR B_DVR_PlaybackService_GetFileHandle(
    B_DVR_PlaybackServiceHandle playbackService,
    struct NEXUS_FilePlay *nexusFilePlay);

/****************************************************************************
Summary:
B_DVR_PlaybackService_SetAlarmTime shall set an alarm time between the 
startTime and endTime of the stream being or about to be playedback.
Alarm time shall be used for sending a callback to the application
from the playback service when current playTime reaches the alarm time.
*****************************************************************************/
B_DVR_ERROR B_DVR_PlaybackService_SetAlarmTime(
    B_DVR_PlaybackServiceHandle playbackService,
    unsigned long alarmTime);

/****************************************************************************
Summary:
B_DVR_PlaybackService_DrmSettings shall set dma and key handles for M2M
*****************************************************************************/
B_DVR_ERROR B_DVR_PlaybackService_AddDrmSettings(
    B_DVR_PlaybackServiceHandle playbackService,
    B_DVR_DRMServiceHandle drmService);

/****************************************************************************
Summary:
B_DVR_PlaybackService_RemoveDrmSettings shall remove dma and key handles for M2M
*****************************************************************************/
B_DVR_ERROR B_DVR_PlaybackService_RemoveDrmSettings(
    B_DVR_PlaybackServiceHandle playbackService);

/****************************************************************************
Summary:
B_DVR_PlaybackService_GetIFrameTimeStamp shall get the I Frame position in the 
forward or reverse direction.  If current time of tsb play is less than 
the passed in timeStamp, then 1st I frame time stamp in the reversion 
direction is sent back to the app else 1st I frame time in the forward 
direction is sent back to the app. 
*****************************************************************************/
B_DVR_ERROR B_DVR_PlaybackService_GetIFrameTimeStamp(
    B_DVR_PlaybackServiceHandle playbackService, 
    unsigned long *timeStamp);

#ifdef __cplusplus
}
#endif

#endif /*_B_DVR_PLAYBACKSERVICE_H */
