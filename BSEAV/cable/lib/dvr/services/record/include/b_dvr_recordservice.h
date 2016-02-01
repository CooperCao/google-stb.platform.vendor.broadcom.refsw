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
 * Recording Service provides APIs for back ground or scheduled recording 
 * either in segmented or non segmented format.
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef _B_DVR_RECORDSERVICE_H
#define _B_DVR_RECORDSERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "b_dvr_const.h"
#include "b_dvr_datatypes.h"

BDBG_OBJECT_ID_DECLARE(B_DVR_RecordService);
/**********************************************************************************
Summary:
B_DVR_RecordServiceHandle shall be an unique identifier for a record service instance
***********************************************************************************/
typedef struct B_DVR_RecordService  *B_DVR_RecordServiceHandle;

typedef enum B_DVR_RecordServiceInput
{
    eB_DVR_RecordServiceInputQam,
    B_DVR_RecordServiceInputTranscode,    
    B_DVR_RecordServiceInputStreamingVOD,
    B_DVR_RecordServiceInputIp,
    B_DVR_RecordServiceInputMax
}B_DVR_RecordServiceInput;

/**********************************************************************************
Summary:
B_DVR_RecordServiceRequest shall be for a back ground permanent recording or 
conversion of the current TSB buffer to a permanent recording.
***********************************************************************************/
typedef struct B_DVR_RecordServiceRequest
{
    char programName[B_DVR_MAX_FILE_NAME_LENGTH];      
    unsigned volumeIndex;
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH]; 
    bool segmented;                         
    unsigned recpumpIndex;
    NEXUS_HeapHandle recpumpCdbHeap;
    NEXUS_HeapHandle recpumpIndexHeap;
    B_DVR_RecordServiceInput input;
}B_DVR_RecordServiceRequest; 


/**********************************************************************************************
Summary: 
B_DVR_RecordServiceSettings shall be for sending information on PID remapping at the
record service start time.
**********************************************************************************************/
typedef struct B_DVR_RecordServiceSettings
{
    B_DVR_EsStreamInfo RemapppedEsStreamInfo[B_DVR_MAX_PIDS];
    unsigned esStreamCount;
    NEXUS_ParserBand parserBand; /* ParserBand valid when B_DVR_RecordServiceInput = eB_DVR_RecordServiceInputQam */           
    void *transcodeService;   /* transcodeService valid when B_DVR_RecordServiceInput = B_DVR_RecordServiceInputTranscode */
    NEXUS_PlaypumpHandle playpumpIp; /* valid if (B_DVR_RecordServiceInput = eB_DVR_RecordServiceInputIp) */     
    B_DVR_DataInjectionServiceHandle dataInjectionService;
}B_DVR_RecordServiceSettings;

/***********************************************************************************************
Summary:
B_DVR_RecordServiceStatus shall have the current information on the file being recorded.
************************************************************************************************/
typedef struct B_DVR_RecordServiceStatus
{
    off_t linearCurrentOffset;
    off_t linearStartOffset;
    unsigned long startTime;
    unsigned long currentTime;
 }B_DVR_RecordServiceStatus;

/***********************************************************************************************
Summary:
B_DVR_RecordServiceInputEsStream shall have the ES stream parameters and pidChannel.
For non-transcoded content recording (QAM source), the pidChannel is always NULL since 
the PID channels are opened inside the record service and for transcoded content (transcode output),
the pidChannels are obtained from transcode service. In the transcoding context,
the PID channels are controlled by the nexus stream mux modules which is in turn controlled in
the transcode service.
************************************************************************************************/
typedef struct B_DVR_RecordServiceInputEsStream
{
    B_DVR_EsStreamInfo esStreamInfo;
    NEXUS_PidChannelHandle pidChannel; 
}B_DVR_RecordServiceInputEsStream;

/***************************************************************************
Summary:
B_DVR_RecordService_Open shall open all the resources required for
starting a recording instance. This shall be used for only scheduled and background
recording. Both the scheduled and background recordings shall have not 
decoders and outputs associated.
Param[in]
recordServiceRequest - Request parameters for a record service instance.
return value
B_DVR_RecordServiceHandle - Handle for a record service instance.
****************************************************************************/
B_DVR_RecordServiceHandle B_DVR_RecordService_Open(
    B_DVR_RecordServiceRequest *recordServiceRequest);

/***************************************************************************
Summary:
B_DVR_RecordService_Close shall close an instance of a recording service.
Param[in]
recordService - Handle for a record service instance.
return value
B_DVR_ERROR - Error code returned.
****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_Close(
    B_DVR_RecordServiceHandle recordService);

/***************************************************************************
Summary:
B_DVR_RecordService_Start shall start a recording instance.
Param[in]
recordService - Handle for a record service instance.
Param[in]
recordServiceSettings - record service instance settings.
return value
B_DVR_ERROR - Error code returned.
*******************************************************************************/
B_DVR_ERROR B_DVR_RecordService_Start(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_RecordServiceSettings *recordServiceSettings);

/***************************************************************************
Summary:
B_DVR_RecordService_Stop shall stop a recording instance
Param[in]
recordService - Handle for a record service instance.
return value
B_DVR_ERROR - Error code returned.
*****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_Stop(
    B_DVR_RecordServiceHandle recordService);

/********************************************************************************
Summary: 
B_DVR_RecordService_Read shall sequetially read the data from RecPump FIFO
Param[in]
recordService - Handle for a record service instance.
Param[in/out]
buffer - buffer pointer passed by the application and used by 
the dvr library to copy the data from the RecPump FIFO.
Param[in] - size of the buffer to be read.
return value
returns number of bytes read.
if this is less then size passed in, then either end of file occured
or an error occured.
*********************************************************************************/
ssize_t B_DVR_RecordService_Read(
    B_DVR_RecordServiceHandle recordService,
    unsigned *buffer, ssize_t size);

/***************************************************************************
Summary:
B_DVR_RecordService_GetStatus shall the status on the current recording
instance.
Param[in]
recordService - Handle for a record service instance.
Param[out]
pStatus - Status of a record service instance.
return value
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_GetStatus(
    B_DVR_RecordServiceHandle recordServiceHandle,
    B_DVR_RecordServiceStatus *pStatus);


/***************************************************************************
Summary:
B_DVR_RecordService_GetSettings shall have the current settings
for a recording instance.
Param[in]
recordService - Handle for a record service instance.
Param[out]
pSettings - Settings of a record service instance.
return value
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR  B_DVR_RecordService_GetSettings(
    B_DVR_RecordServiceHandle recordServiceHandle, 
    B_DVR_RecordServiceSettings *pSettings);

/***************************************************************************
Summary:
B_DVR_RecordService_SetSettings shall set the settings for a instance 
of a recording.
Param[in]
recordService - Handle for a record service instance.
Param[in]
pSettings - Settings of a record service instance.
return value
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_SetSettings(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_RecordServiceSettings *pSettings);


/***************************************************************************
Summary:
B_DVR_RecordService_InstallCallback shall install an application provided
callback for sending information from a recording instance to the upper level.
Param[in]
recordService - Handle for a record service instance.
Param[in]
registeredCallback - Application provided callback
Param[in]
appContext - Application context
return value
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_InstallCallback(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_ServiceCallback registeredCallback,
    void * appContext);

/***************************************************************************
Summary:
B_DVR_RecordService_RemoveCallback shall remove the application provided
callback from a recording instance.
Param[in]
recordService - Handle for a record service instance.
return value
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_RemoveCallback(
    B_DVR_RecordServiceHandle recordService);

/***************************************************************************
Summary:
B_DVR_RecordService_AddInputEsStream shall be used for adding an ES stream
to the recording.
Param[in]
recordService - Handle for a record service instance.
Param[in]
esStreamInfo - ES stream info passed in for adding to the current record 
               service instance.
return value
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_AddInputEsStream(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_RecordServiceInputEsStream *inputEsStreamInfo);

/***************************************************************************
Summary:
B_DVR_RecordService_RemoveEsStream shall be used for remove an ES stream
from a recording instance even after starting the recording.
Param[in]
recordService - Handle for a record service instance.
Param[in]
esStreamInfo - ES stream info passed in for removing from the current record 
               service instance.
return value
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_RemoveInputEsStream(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_RecordServiceInputEsStream *inputEsStream);

/*****************************************************************************
Summary:
B_DVR_RecordService_InjectData shall insert a buffer into recording 
either in one shot or periodically as specified in 
B_DVR_DataInjectionService_SetSettings
Param[in]
recordService - Handle for a record service instance.
Param[in]
buf  - data buffer to be inserted into a recording.
Param[in]
size  - size of the data buffer to be inserted into a recording.
return value
B_DVR_ERROR - Error code returned
******************************************************************************/
B_DVR_ERROR B_DVR_RecordService_InjectDataStart(
    B_DVR_RecordServiceHandle recordService,
    uint8_t *buf,
    unsigned length);

/*****************************************************************************
Summary:
B_DVR_RecordService_InjectDataStop shall stop the dataInjection for a record service.
Param[in]
recordService - Handle for a record service instance.
return value
B_DVR_ERROR - Error code returned
******************************************************************************/
B_DVR_ERROR B_DVR_RecordService_InjectDataStop(
    B_DVR_RecordServiceHandle recordService);

/*****************************************************************************
Summary:
B_DVR_RecordService_GetDataInjectionPidChannel provide the 
pid Channel handle for a record service associated with a
dataInjection service;
Param[in]
recordService - Handle for a record service instance.
return value
NEXUS_PidChannelHandle - dataInjection pid channel. It will be NULL
if there is no dataInjection service associated with a record service.
******************************************************************************/
NEXUS_PidChannelHandle B_DVR_RecordService_GetDataInjectionPidChannel(
    B_DVR_RecordServiceHandle recordService);

/*****************************************************************************
Summary:
B_DVR_RecordService_GetPidChannel shall return a PIDChannelHandle for a PID
used in a Recording. 
See B_DVR_RecordService_AddEsStream 
Param[in]
recordService - handle for recordService
Param[in]
pid - A PID added for Recording using B_DVR_RecordService_AddEsStream.
return value 
NEXUS_PidChannelHandle - either NULL in case PID is not found in
Recording or a PID channel handle with with a DRM service Handle shall
be associated using B_DVR_DRMService_Start
******************************************************************************/
NEXUS_PidChannelHandle B_DVR_RecordService_GetPidChannel(
    B_DVR_RecordServiceHandle recordService,
    unsigned pid);

/****************************************************************************
Summary:
B_DVR_RecordService_AddDrmSettings shall set the record and mediaProbe
drmService handles for an encrypted recording.
drmService for mediaProbe during an encrypted recording shall be 
used for performing m2m dma decryption of media data for probing encrypted
media.
*****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_AddDrmSettings(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMUsage drmUsage);

/****************************************************************************
Summary:
B_DVR_RecordService_RemoveDrmSettings shall unset the record and mediaProbe
drmService handles for an encrypted recording.
*****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_RemoveDrmSettings(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_DRMUsage drmUsage);

/****************************************************************************
Summary:
B_DVR_RecordService_SetPlaybackUpdateEvent shall shall be called by the 
playback service internally to get the in-progress recording updates
from recordService. updatePlaybackEvent shall be created by the playbackService.
*****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_SetPlaybackUpdateEvent(
    B_DVR_RecordServiceHandle recordService,
    B_EventHandle updatePlaybackEvent);

/****************************************************************************
Summary:
B_DVR_RecordService_GetMediaNode shall return the mediaNode associated 
with a record service.
*****************************************************************************/
B_DVR_MediaNode B_DVR_RecordService_GetMediaNode(
    B_DVR_RecordServiceHandle recordService);

/****************************************************************************
Summary:
B_DVR_RecordService_Pause shall pause the linear recording in progress.
*****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_Pause(
    B_DVR_RecordServiceHandle recordService);

/****************************************************************************
Summary:B_DVR_RecordService_Resume shall resume the linear recording.
*****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_Resume(
    B_DVR_RecordServiceHandle recordService);

/****************************************************************************
Summary: 
B_DVR_RecordService_RemoveInProgressRecPlayback shall link the playback of in progress 
recording to the back ground recording to handle the EOF and play transition 
in the playback smoothly. 
*****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_AddInProgressRecPlayback(
   B_DVR_RecordServiceHandle recordService,
   NEXUS_PlaybackHandle playback);

/****************************************************************************
Summary: 
B_DVR_RecordService_RemoveInProgressRecPlayback shall unlink the playback of in progress 
recording from the back ground recording.
*****************************************************************************/
B_DVR_ERROR B_DVR_RecordService_RemoveInProgressRecPlayback(
   B_DVR_RecordServiceHandle recordService);


#ifdef __cplusplus
}
#endif

#endif /*_B_DVR_RECORDSERVICE_H */





