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
 *  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 * TSB service provides APIs for TSB recording and playback
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/

#ifndef _B_DVR_TSBSERVICE_H
#define _B_DVR_TSBSERVICE_H

#ifdef __cplusplus
extern "C" {
#endif
BDBG_OBJECT_ID_DECLARE(B_DVR_TSBService);
/******************************************************************************************
Summary:
B_DVR_TSBServiceHandle shall be an unique indentifier for an instance of TSB service
********************************************************************************************/

typedef struct B_DVR_TSBService *B_DVR_TSBServiceHandle;

typedef enum B_DVR_TSBServiceInput
{
    eB_DVR_TSBServiceInputQam,
    eB_DVR_TSBServiceInputTranscode,
    eB_DVR_TSBServiceInputIp,
    eB_DVR_TSBServiceInputMax
}B_DVR_TSBServiceInput;

/*******************************************************************************************
B_DVR_TSBServiceRequest shall be having TSB service requested properties for an
instance of TSB service.
********************************************************************************************/
typedef struct B_DVR_TSBServiceRequest
{
    char programName[B_DVR_MAX_FILE_NAME_LENGTH]; 
    unsigned recpumpIndex;
    NEXUS_HeapHandle recpumpCdbHeap;
    NEXUS_HeapHandle recpumpIndexHeap;
    unsigned playpumpIndex;
    NEXUS_HeapHandle playpumpHeap;
    unsigned volumeIndex;
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH]; 
    unsigned maxTSBBufferCount;
    unsigned long maxTSBTime; /* in milli seconds */
    B_DVR_TSBServiceInput input;
}B_DVR_TSBServiceRequest;

/*******************************************************************************************
B_DVR_TSBServicePermanentRecordingRequest shall be having user parameters 
for TSB to permanent recording request;
********************************************************************************************/
typedef struct B_DVR_TSBServicePermanentRecordingRequest
{
    char programName[B_DVR_MAX_FILE_NAME_LENGTH];
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH];
    unsigned long recStartTime;
    unsigned long recEndTime;
}B_DVR_TSBServicePermanentRecordingRequest;

/*********************************************************************************************
Summary:
B_DVR_TSBServiceRecordSettings shall have pid remapping info for TSB recording.
********************************************************************************************/
typedef struct B_DVR_TSBServiceRecordSettings
{
    B_DVR_EsStreamInfo RemappedEsStreamInfo[B_DVR_MAX_PIDS]; 
    unsigned esStreamCount;
}B_DVR_TSBServiceRecordSettings;

/******************************************************************************************
Summary:
B_DVR_TSBServicePlaybackSettings shall have settings info for TSB playback.
*******************************************************************************************/
typedef struct B_DVR_TSBServicePlaybackSettings
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
}B_DVR_TSBServicePlaybackSettings;


/***************************************************************************************
Summary:
B_DVR_TSBServiceSettings shall have settings for record and playback
**************************************************************************************/
typedef struct B_DVR_TSBServiceSettings
{
   B_DVR_TSBServicePlaybackSettings tsbPlaybackSettings;
   B_DVR_TSBServiceRecordSettings tsbRecordSettings;
   NEXUS_ParserBand parserBand;  /* valid if (B_DVR_TSBServiceInput = eB_DVR_TSBServiceInputQam) 
                                    Used for creating record PID channels in TSB service */
   NEXUS_PlaypumpHandle playpumpIp; /* valid if (B_DVR_TSBServiceInput = eB_DVR_TSBServiceInputIp) 
                                     Used for creating record PID channels in TSB service */            
   B_DVR_DataInjectionServiceHandle dataInjectionService;
   unsigned long tsbConvEndTime;
}B_DVR_TSBServiceSettings;

/*********************************************************************************************
Summary:
B_DVR_TSBServicStatus shall provide status of TSB recording and TSB playback.
**********************************************************************************************/
typedef struct B_DVR_TSBServicStatus
{
    unsigned long tsbCurrentPlayTime;
    off_t tsbCurrentPlayOffset;
    bool tsbPlayback;
    off_t tsbRecLinearStartOffset;
    off_t tsbRecLinearEndOffset;
    unsigned long tsbRecStartTime;
    unsigned long tsbRecEndTime;
    bool tsbCoversion;
    off_t permRecLinearCurrentOffset;
    unsigned long permRecCurrentTime;
    NEXUS_PlaybackState state;
}B_DVR_TSBServiceStatus;

/***********************************************************************************************
Summary:
B_DVR_TSBServiceInputEsStream shall have the ES stream parameters and pidChannel.
For non-transcoded content TSB recording (QAM source), the pidChannel is always NULL since 
the PID channels are opened inside the TSB service and for transcoded content (transcode output),
the pidChannels are obtained from transcode service. In the transcoding context,
the PID channels are controlled by the nexus stream mux modules which is in turn controlled in
the transcode service.
************************************************************************************************/
typedef struct B_DVR_TSBServiceInputEsStream
{
    B_DVR_EsStreamInfo esStreamInfo;
    NEXUS_PidChannelHandle pidChannel; 
}B_DVR_TSBServiceInputEsStream;

/*********************************************************************************
Summary: 
B_DVR_TSBService_Open shall open an instance of TSB Service.
Param[in]
tsbServiceRequest - Request parameters for the TSB service instance
Param[out]
B_DVR_TSBServiceHandle - Handle for a TSB service instance.
 **********************************************************************************/
B_DVR_TSBServiceHandle B_DVR_TSBService_Open(
    B_DVR_TSBServiceRequest *tsbServiceRequest);


/*********************************************************************************
Summary:
B_DVR_TSBService_Close shall close an instance of TSB Service
Param[in]
tsbService - Handle for a TSB service instance.
Param[out]
B_DVR_ERROR - Error code returned.
**********************************************************************************/
B_DVR_ERROR B_DVR_TSBService_Close(
    B_DVR_TSBServiceHandle tsbService);

/*********************************************************************************
Summary:
B_DVR_TSBService_Start shall start an instance of the TSB Service 
Param[in]
tsbService - Handle for a TSB service instance.
Param[in]
tsbServiceSetttings - Settings for a TSB service instance.
Param[out]
B_DVR_ERROR - Error code returned.
***********************************************************************************/
B_DVR_ERROR B_DVR_TSBService_Start(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServiceSettings *tsbServiceSetttings);


/*************************************************************************************
Summary:
B_DVR_TSBService_Stop shall stop an instance of TSB Service
Param[in]
tsbService - Handle for a TSB service instance.
Param[out]
B_DVR_ERROR - Error code returned.
**************************************************************************************/
B_DVR_ERROR B_DVR_TSBService_Stop(
    B_DVR_TSBServiceHandle tsbService);

/*************************************************************************************
Summary:
B_DVR_TSBService_SetOperation shall perform the DVR operations requested on a TSB
Service
Param[in]
tsbService - Handle for a TSB service instance.
Param[in]
operationSettings - DVR trick mode settings.
Param[out]
B_DVR_ERROR - Error code returned.
**************************************************************************************/
B_DVR_ERROR B_DVR_TSBService_SetOperation(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_OperationSettings *operationSettings);

/************************************************************************************
Summary:
B_DVR_TSBService_Setettings shall set the segment size of both
the navigation and media files for an instance of a TSB Service.
Param[in]
tsbService - Handle for a TSB service instance.
Param[in]
tsbServiceSettings - TSB settings.
Param[out]
B_DVR_ERROR - Error code returned.
*************************************************************************************/
B_DVR_ERROR B_DVR_TSBService_SetSettings(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServiceSettings *tsbServiceSettings);

/**************************************************************************************
Summary:
B_DVR_TSBService_GetSSettings shall get the segment size of the navigation 
and media files for a TSB Service
Param[in]
tsbService - Handle for a TSB service instance.
Param[in]
pSettings - Settings of a TSB service instance.
Param[out]
B_DVR_ERROR - Error code returned.
**************************************************************************************/
B_DVR_ERROR B_DVR_TSBService_GetSettings(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServiceSettings *pSettings);

/*****************************************************************************************
Summary: 
B_DVR_TSBService_GetStatus shall get the TSB Service instance's status. 
Param[in]
tsbService - Handle for a TSB service instance.
Param[in]
tsbServiceStatus - Status of a TSB service instance
Param[out]
B_DVR_ERROR - Error code returned.
*******************************************************************************************/
B_DVR_ERROR B_DVR_TSBService_GetStatus(
    B_DVR_TSBServiceHandle tsbHandle,
    B_DVR_TSBServiceStatus *tsbServiceStatus);


/***************************************************************************
Summary:
B_DVR_TSBService_InstallCallback shall add an application provided
callback to a TSB service instance.
Param[in]
tsbService - Handle for a TSB service instance.
Param[in]
registeredCallback - Application provided callback
Param[in]
appContext - Application context
Param[out]
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_TSBService_InstallCallback(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_ServiceCallback registeredCallback,
    void * appContext
    );

/***************************************************************************
Summary:
B_DVR_TSBService_RemoveCallback shall remove the application provided
callback from a recording instance.
Param[in]
tsbService - Handle for a TSB service instance.
Param[out]
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_TSBService_RemoveCallback(
    B_DVR_TSBServiceHandle tsbService);

/*******************************************************************************
Summary:
B_DVR_TSBService_ConvertStart shall start a permanent recording on a TSB service.
Since all the TSB recordings are segmented, the permanent recording from a TSB 
service shall be segmented. When this API is called, the current TSB segment is
filled in and  the entry for this segment is made in the permanent recording 
meta data. TSB recording is stopped i.e. files from the TSB pool wouldn't be used 
for permanent recording rather new segmented files shall be used for the proceeding
segmented recording.
Param[in]
tsbService - Handle for a TSB service instance.
Param[in]
mediaName - Name of the media used for permanent recording.
Param[in]
startTime - Start time of the permanent recording in milliseconds
Param[in]
endTime - end time of the permanent recording in milliseconds
Param[out]
B_DVR_ERROR - Error code returned
***********************************************************************************/
B_DVR_ERROR B_DVR_TSBService_ConvertStart(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServicePermanentRecordingRequest *permRecReq);

/*************************************************************************************
Summary:
B_DVR_TSBService_ConvertStop shall stop the current permanent recording on a TSB service.
The TSB recording shall continue after permanent recording is stopped.
Param[in]
tsbService - Handle for a TSB service instance.
Param[out]
B_DVR_ERROR - Error code returned
****************************************************************************************/
B_DVR_ERROR B_DVR_TSBService_ConvertStop(
    B_DVR_TSBServiceHandle tsbService);

/*****************************************************************************
Summary:
B_DVR_TSBService_InjectData shall insert a buffer into recording 
either in one shot or periodically as specified in 
B_DVR_DataInjectionService_SetSettings
Param[in]
tsbService - Handle for a tsb service instance.
Param[in]
buf  - data buffer to be inserted into a recording.
Param[in]
size  - size of the data buffer to be inserted into a recording.
return value
B_DVR_ERROR - Error code returned
******************************************************************************/
B_DVR_ERROR B_DVR_TSBService_InjectDataStart(
    B_DVR_TSBServiceHandle tsbService,
    uint8_t *buf,
    unsigned length);

/*****************************************************************************
Summary:
B_DVR_TSBService_InjectDataStop shall stop the dataInjection for a tsb service.
Param[in]
tsbService - Handle for a tsb service instance.
return value
B_DVR_ERROR - Error code returned
******************************************************************************/
B_DVR_ERROR B_DVR_TSBService_InjectDataStop(
    B_DVR_TSBServiceHandle tsbService);

/*****************************************************************************
Summary:
B_DVR_TSBService_GetDataInjectionPidChannel shall return a pidChannel
for a dataInjection service associated with a TSB recording.
Param[in]
tsbService - Handle for a tsb service instance.
return value
NEXUS_PidChannelHandle - pidChannel used for dataInjectionService associated with
a tsb service. If the dataInjectionService associated with a TSB service is null,
then the pid channel will also be null.
******************************************************************************/
NEXUS_PidChannelHandle B_DVR_TSBService_GetDataInjectionPidChannel(
    B_DVR_TSBServiceHandle tsbService);

/****************************************************************************
Summary:
B_DVR_PlaybackService_SetAlarmTime shall set an alarm time between the 
startTime and endTime of the stream being or about to be playedback.
Alarm time shall be used for sending a callback to the application
from the playback service when current playTime reaches the alarm time.
*****************************************************************************/
B_DVR_ERROR B_DVR_TSBService_SetAlarmTime(
    B_DVR_TSBServiceHandle tsbService,
    unsigned long alarmTime);


/***************************************************************************
Summary:
B_DVR_TSBService_AddInputEsStream shall be used for adding an ES stream
to the tsb recording before/after starting a TSB service instance.
Param[in]
tsbService - Handle for a tsb service instance.
Param[in]
esStreamInfo - ES stream info passed in for adding to the current tsb recording 
               service instance.
return value
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_TSBService_AddInputEsStream(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServiceInputEsStream *inputEsStream);

/***************************************************************************
Summary:
B_DVR_TSBService_RemoveInputEsStream shall be used for remove an ES stream
from a tsb recording instance even after starting the tsb recording.
Param[in]
tsbService - Handle for a tsb service instance.
Param[in]
esStreamInfo - ES stream info passed in for removing from the current tsb record 
               service instance.
return value
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_TSBService_RemoveInputEsStream(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServiceInputEsStream *inputEsStream);

/*****************************************************************************
Summary:
B_DVR_TSBService_GetPidChannel shall return a PIDChannelHandle for a PID
used in TSBRecording. 
See B_DVR_TSBService_AddEsStream 
Param[in]
tsbService - handle for TSBService
Param[in]
pid - A PID added for TSB Recording using B_DVR_TSBService_AddEsStream.
return value 
NEXUS_PidChannelHandle - either NULL in case PID is not found in
TSBRecording or a PID channel handle with with a DRM service Handle shall
be associated using B_DVR_DRMService_Start
******************************************************************************/
NEXUS_PidChannelHandle B_DVR_TSBService_GetPidChannel(
    B_DVR_TSBServiceHandle tsbService,
    unsigned pid);

/****************************************************************************
Summary:
B_DVR_TSBService_AddDrmSettings shall set the record,playback and mediaProbe
drmService handles for an encrypted tsb recording.
drmService for mediaProbe during an encrypted tsb recording shall be 
used for performing m2m dma decryption of media data for probing encrypted
media and for tsb playback
*****************************************************************************/
B_DVR_ERROR B_DVR_TSBService_AddDrmSettings(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMUsage drmUsage);

/****************************************************************************
Summary:
B_DVR_TSBService_RemoveDrmSettings shall unset the record, playback and mediaProbe
drmService handles for an encrypted tsb recording.
*****************************************************************************/
B_DVR_ERROR B_DVR_TSBService_RemoveDrmSettings(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_DRMUsage drmUsage);

/****************************************************************************
Summary:
B_DVR_TSBService_GetFileRecordHandle shall return the write handle
for TSB recording to be used in the mediaFile streaming.
*****************************************************************************/
NEXUS_FileRecordHandle B_DVR_TSBService_GetFileRecordHandle(
    B_DVR_TSBServiceHandle tsbService);

/****************************************************************************
Summary:
B_DVR_TSBService_GetMediaNode shall return the mediaNode associated with
a TSB recording to be used in the mediaFile streaming.
*****************************************************************************/
B_DVR_MediaNode B_DVR_TSBService_GetMediaNode(
    B_DVR_TSBServiceHandle tsbService, bool permRec);

/****************************************************************************
Summary:
B_DVR_TSBService_SetPlaybackUpdateEvent shall be used by the playback
service internally to get the tsb conversion updates from the TSB service.
updatePlaybackEvent shall be created by the playbackService.
*****************************************************************************/
B_DVR_ERROR B_DVR_TSBService_SetPlaybackUpdateEvent(
    B_DVR_TSBServiceHandle tsbService,
    B_EventHandle updatePlaybackEvent);


/****************************************************************************
Summary:
B_DVR_TSBService_Pause shall pause the TSB buffering
*****************************************************************************/
B_DVR_ERROR B_DVR_TSBService_Pause(
    B_DVR_TSBServiceHandle tsbService);

/****************************************************************************
Summary:
B_DVR_TSBService_Pause shall resume the TSB buffering
*****************************************************************************/
B_DVR_ERROR B_DVR_TSBService_Resume(
    B_DVR_TSBServiceHandle tsbService);

/****************************************************************************
Summary:
B_DVR_TSBService_GetIFrameTimeStamp shall get the I Frame position in the 
forward or reverse direction.  If current time of tsb play is less than 
the passed in timeStamp, then 1st I frame time stamp in the reversion 
direction is sent back to the app else 1st I frame time in the forward 
direction is sent back to the app. 
*****************************************************************************/
B_DVR_ERROR B_DVR_TSBService_GetIFrameTimeStamp(
    B_DVR_TSBServiceHandle tsbService, 
    unsigned long *timeStamp);

/****************************************************************************
Summary: 
B_DVR_TSBService_AddInProgressPlayback shall link the playback of in progress 
recording (tsb converted recording) to the tsb recording to handle the 
EOF and play transition in the playback smoothly. 
*****************************************************************************/
B_DVR_ERROR B_DVR_TSBService_AddInProgressRecPlayback(
   B_DVR_TSBServiceHandle tsbService,
   NEXUS_PlaybackHandle playback);

/****************************************************************************
Summary: 
B_DVR_TSBService_RemoveInProgressPlayback shall unlink the playback of in progress 
recording (tsb converted recording) from the tsb recording.
*****************************************************************************/
B_DVR_ERROR B_DVR_TSBService_RemoveInProgressRecPlayback(
   B_DVR_TSBServiceHandle tsbService);

B_DVR_ERROR B_DVR_TSBService_AppendedConvertStart(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServicePermanentRecordingRequest *permRecReq);

B_DVR_ERROR B_DVR_TSBService_AppendedConvertStop(
    B_DVR_TSBServiceHandle tsbService);
#ifdef __cplusplus
}
#endif

#endif /*_B_DVR_TSBSERVICE_H */
