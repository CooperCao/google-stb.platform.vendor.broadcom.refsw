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
 * Transcoder service provides transcoding for the following
 * input and output combination -
 * Input sources  1. Live Video  2. linear recording
 * Output source  1. Transcoded linear recording 2. Transcoded TSB Recording 3. Transcoded memory buffers
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/

#ifndef _B_DVR_TRANSCODESERVICE_H
#define _B_DVR_TRANSCODESERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_video_decoder.h"
#include "nexus_parser_band.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_adj.h"
#include "nexus_video_input.h"
#include "nexus_core_utils.h"
#if NEXUS_HAS_STREAM_MUX
#include "nexus_video_encoder.h"
#include "nexus_audio_encoder.h"
#endif
#include "nexus_video_decoder.h"
#include "nexus_audio_decoder.h"
#if NEXUS_HAS_STREAM_MUX
#include "nexus_stream_mux.h"
#endif
#include "nexus_audio_mixer.h"
#if (BCHP_VER >= BCHP_VER_B1)
#include "nexus_audio_dummy_output.h"
#endif
#if NEXUS_NUM_HDMI_INPUTS
#include "nexus_hdmi_input.h"
#endif
BDBG_OBJECT_ID_DECLARE(B_DVR_TranscodeService);
/******************************************************************************************
Summary:
B_DVR_TranscodeServiceHandle shall be an unique indentifier for an instance of Transcode service
********************************************************************************************/

typedef struct B_DVR_TranscodeService *B_DVR_TranscodeServiceHandle;

/***********************************************************************************************
 Summary:
 B_DVR_TranscodeServiceInput shall specify what's the input to the transcoder.
 ***********************************************************************************************/
typedef enum B_DVR_TranscodeServiceInput
{
    eB_DVR_TranscodeServiceInput_Qam,
    eB_DVR_TranscodeServiceInput_File,
    eB_DVR_TranscodeServiceInput_Hdmi,
    eB_DVR_TranscodeServiceInput_Max
}B_DVR_TranscodeServiceInput;

/***********************************************************************************************
 Summary:
 B_DVR_TranscodeServiceOutput shall specify where the output of the transcoder is directed.
***********************************************************************************************/
typedef enum B_DVR_TranscodeServiceOutput
{
    eB_DVR_TranscodeServiceOutput_File,
    eB_DVR_TranscodeServiceOutput_Tsb,
    eB_DVR_TranscodeServiceOutput_Memory,
    eB_DVR_TranscodeServiceOutput_Max
}B_DVR_TranscodeServiceOutput;

/***********************************************************************************************
 Summary:
 B_DVR_TranscodeServiceType shall specifiy if transcoding is done in realtime or non real time.
 ***********************************************************************************************/
typedef enum B_DVR_TranscodeServiceType
{
    eB_DVR_TranscodeServiceType_RealTime,
    eB_DVR_TranscodeServiceType_NonRealTime,
    eB_DVR_TranscodeServiceType_Max
}B_DVR_TranscodeServiceType;

/*******************************************************************************************
Summary:
B_DVR_TranscodeServiceRequest shall contain the parameters used for transcoding the input.
********************************************************************************************/
typedef struct B_DVR_TranscodeServiceRequest
{
    unsigned displayIndex;
    /*
     * This stcChannel should have the same time base as that of the audio and video stc channels
     */
    NEXUS_StcChannelHandle transcodeStcChannel; 
    B_DVR_TranscodeServiceType transcodeType;
}B_DVR_TranscodeServiceRequest;

/*******************************************************************************************
Summary:
B_DVR_TranscodeServiceVideoEncodeParams shall have parameters specific to video encoding.
********************************************************************************************/
typedef struct B_DVR_TranscodeServiceVideoEncodeParams
{
    NEXUS_VideoDecoderHandle videoDecoder;
    bool interlaced;
    NEXUS_VideoCodec codec;
    #if NEXUS_HAS_STREAM_MUX
    NEXUS_VideoCodecProfile profile;
    NEXUS_VideoCodecLevel level;
    #endif
}B_DVR_TranscodeServiceVideoEncodeParams;

/*******************************************************************************************
Summary:
B_DVR_TranscodeServiceAudioEncodeParams shall have parameters specific to audio encoding.
********************************************************************************************/
typedef struct B_DVR_TranscodeServiceAudioEncodeParams
{
    bool audioPassThrough; 
    bool rateSmoothingEnable;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioCodec codec; 
    #if (BCHP_VER >= BCHP_VER_B1)
    NEXUS_AudioDummyOutputHandle audioDummy;
    #endif
}B_DVR_TranscodeServiceAudioEncodeParams;

/*******************************************************************************************
Summary:
B_DVR_TranscoderServiceInputEsStream shall contain the parameters required
to process the ES streams to be transcoded by the transcoder service instance.
******************************************************************************************/
typedef struct B_DVR_TranscodeServiceInputEsStream
{
    #if NEXUS_HAS_STREAM_MUX
    B_DVR_TranscodeServiceAudioEncodeParams audioEncodeParams;
    B_DVR_TranscodeServiceVideoEncodeParams videoEncodeParams;
    #endif
    B_DVR_EsStreamInfo esStreamInfo;
    NEXUS_StcChannelHandle stcChannel;
    unsigned playpumpIndex;
    uint8_t pesId; /* pes stream ID */
}B_DVR_TranscodeServiceInputEsStream;

/*********************************************************************************************
Summary: 
B_DVR_TranscodeServiceSettings shall contain all the dynamic settings for video and audio
encode parameters.
*********************************************************************************************/
typedef struct B_DVR_TranscodeServiceSettings
{
    #if NEXUS_HAS_STREAM_MUX
    NEXUS_StreamMuxHandle streamMux; /* Output parameter for dataInjection*/
    #endif
    NEXUS_PlaypumpHandle  playpump;  /* Output parameter for dataInjection */
    #if NEXUS_HAS_STREAM_MUX
    NEXUS_VideoEncoderSettings videoEncodeSettings; /*Input parameter*/
    #endif
    NEXUS_VideoWindowDnrSettings dnrSettings;/*Input parameter*/
    NEXUS_VideoWindowMadSettings madSettings; /*Input parameter*/
    NEXUS_DisplaySettings displaySettings; /*Input parameter*/
    #if NEXUS_HAS_STREAM_MUX
    NEXUS_DisplayStgSettings stgSettings; /* input parameter*/
    #endif
    NEXUS_VideoWindowSettings windowSettings; /*Input parameter*/
    NEXUS_VideoWindowScalerSettings scalerSettings; /*Input parameter*/
}B_DVR_TranscodeServiceSettings;

/******************************************************************************************
Summary:
B_DVR_TranscodeServiceInputSettings shall have settings info for an instance of
transcode service's input.
*******************************************************************************************/
typedef struct B_DVR_TranscodeServiceInputSettings
{
   B_DVR_TranscodeServiceInput input;
   B_DVR_PlaybackServiceHandle playbackService;
   #if NEXUS_NUM_HDMI_INPUTS
   NEXUS_HdmiInputHandle hdmiInput;
   #endif
   NEXUS_ParserBand parserBand;  
}B_DVR_TranscodeServiceInputSettings;


/******************************************************************************************
Summary:
B_DVR_TranscodeServiceOutputSettings shall have settings info for an instance of
transcode service's output.
*******************************************************************************************/
typedef struct B_DVR_TranscodeServiceOutputSettings
{
    B_DVR_TranscodeServiceOutput output;
    B_DVR_RecordServiceHandle recordService;
    B_DVR_TSBServiceHandle tsbService;
}B_DVR_TranscodeServiceOutputSettings;


/*********************************************************************************************
Summary:
B_DVR_TranscodeServiceStatus shall provide status of an instance of transcode service
**********************************************************************************************/
typedef struct B_DVR_TranscodeServiceStatus
{
    unsigned numBytesTranscoded;
}B_DVR_TranscodeServiceStatus;

/*********************************************************************************
Summary: 
B_DVR_TranscodeService_Open shall open an instance of transcode service.
Param[in]
transcodeServiceRequest - Request parameters for the transcode service instance
Param[out]
B_DVR_TranscodeServiceHandle - Handle for a transcode service instance.
 **********************************************************************************/
B_DVR_TranscodeServiceHandle B_DVR_TranscodeService_Open(
    B_DVR_TranscodeServiceRequest *transcodeServiceRequest);


/*********************************************************************************
Summary:
B_DVR_TranscodeService_Close shall close an instance of transcode Service
Param[in]
transcodeService - Handle for a transcode service instance.
Param[out]
B_DVR_ERROR - Error code returned.
**********************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_Close(
    B_DVR_TranscodeServiceHandle transcodeService);

/*********************************************************************************
Summary:
B_DVR_TranscodeService_Start shall start an instance of the transcode Service 
Param[in]
transcode - Handle for a transcode service instance.
Param[out]
B_DVR_ERROR - Error code returned.
***********************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_Start(
    B_DVR_TranscodeServiceHandle transcodeService);

/*************************************************************************************
Summary:
B_DVR_TranscodeService_Stop shall stop an instance of transcode Service
Param[in]
transcodeService - Handle for a transcode service instance.
Param[out]
B_DVR_ERROR - Error code returned.
**************************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_Stop(
    B_DVR_TranscodeServiceHandle transcodeService);

/**************************************************************************************
Summary:
B_DVR_TranscodeService_GetInputSettings 
Param[in]
transcodeService - Handle for a transcode service instance.
Param[in]
pSettings - Input settings of a transcode service instance.
Param[out]
B_DVR_ERROR - Error code returned.
**************************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_GetInputSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputSettings *pSettings);


/************************************************************************************
Summary:
B_DVR_TranscodeService_SetInputsettings 
Param[in]
transcodeService - Handle for a transcode service instance.
Param[in]
pSettings - Transcode service input settings.
Param[out]
B_DVR_ERROR - Error code returned.
*************************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_SetInputSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputSettings *pSettings);


/**************************************************************************************
Summary:
B_DVR_TranscodeService_GetOutputSettings 
Param[in]
transcodeService - Handle for a transcode service instance.
Param[in]
pSettings - Input settings of a transcode service instance.
Param[out]
B_DVR_ERROR - Error code returned.
**************************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_GetOutputSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceOutputSettings *pSettings);

/************************************************************************************
Summary:
B_DVR_TranscodeService_SetOutputsettings 
Param[in]
transcodeService - Handle for a transcode service instance.
Param[in]
pSettings - Transcode service output settings.
Param[out]
B_DVR_ERROR - Error code returned.
*************************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_SetOutputSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceOutputSettings *pSettings);

/************************************************************************************
Summary:
B_DVR_TranscodeService_Getsettings 
Param[in]
transcodeService - Handle for a transcode service instance.
Param[in]
pSettings - Transcode service settings.
Param[out]
B_DVR_ERROR - Error code returned.
*************************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_GetSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceSettings *pSettings);

/************************************************************************************
Summary:
B_DVR_TranscodeService_SetSettings 
Param[in]
transcodeService - Handle for a transcode service instance.
Param[in]
pSettings - Transcode service  settings.
Param[out]
B_DVR_ERROR - Error code returned.
*************************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_SetSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceSettings *pSettings);


/*****************************************************************************************
Summary: 
B_DVR_TranscodeService_GetStatus shall get the Transcode Service instance's status. 
Param[in]
transcodeService - Handle for a transcode service instance.
Param[in]
transcodeServiceStatus - Status of a transcode service instance
Param[out]
B_DVR_ERROR - Error code returned.
*******************************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_GetStatus(
    B_DVR_TranscodeServiceHandle transcodeHandle,
    B_DVR_TranscodeServiceStatus *pStatus);


/***************************************************************************
Summary:
B_DVR_TranscodeService_InstallCallback shall add an application provided
callback to a transcode service instance.
Param[in]
transcodeService - Handle for a transcode service instance.
Param[in]
registeredCallback - Application provided callback
Param[in]
appContext - Application context
Param[out]
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_InstallCallback(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_ServiceCallback registeredCallback,
    void * appContext
    );

/***************************************************************************
Summary:
B_DVR_TranscodeService_RemoveCallback shall remove the application provided
callback from a recording instance.
Param[in]
transcodeService - Handle for a transcode service instance.
Param[out]
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_RemoveCallback(
    B_DVR_TranscodeServiceHandle transcodeService);

/***************************************************************************
Summary:
B_DVR_TranscodeService_AddInputEsStream shall be used for adding an ES stream
to the input of a transcode service instance.
Param[in]
transcodeService - Handle for a transcode service instance.
Param[in]
esStreamInfo - ES stream info to be added as input to the transcoder input.
return value
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_AddInputEsStream(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputEsStream *esStream);

/***************************************************************************
Summary:
B_DVR_TranscoderService_RemoveInputEsStream shall be used for removing an ES stream
from the input of a transcoder service instance.
Param[in]
transcoderService - Handle for a transcoder service instance.
Param[in]
esStreamInfo -ES stream to be removed from the input of a transcoder service instance.
return value
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_RemoveInputEsStream(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputEsStream *esStream);

/***************************************************************************
Summary:
B_DVR_TranscoderService_RemoveAllInputEsStreams shall be used for removing all the
ES streams from the input of a transcoder service instance.
Param[in]
transcoderService - Handle for a transcoder service instance.
return value
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_TranscodeService_RemoveAllInputEsStreams(
    B_DVR_TranscodeServiceHandle transcodeService);

/*****************************************************************************
Summary:
B_DVR_TranscodeService_GetPidChannel shall return a PIDChannelHandle for a PID
used in a stream mux. 
See B_DVR_TranscodeService_AddInputStream 
Param[in]
transcodeService - handle for transcodeService
Param[in]
pid - A PID added for transcoding
return value 
NEXUS_PidChannelHandle - is the PID channel created by nexus stream mux
for transcoding. The same PID channel shall be passed to the record service
for recording the transcoded stream.
******************************************************************************/
NEXUS_PidChannelHandle B_DVR_TranscodeService_GetPidChannel(
    B_DVR_TranscodeServiceHandle transcodeService,
    unsigned pid);

/*****************************************************************************
Summary:
B_DVR_TranscodeService_GetDataInjectionPidChannel shall return a PIDChannelHandle
that would be used in the recordService for injecting data into a transcode output.
Param[in]
transcodeService - handle for transcodeService
return value 
NEXUS_PidChannelHandle - a data injection PID channel 
created on the same playback parser band that's used for pcr injection.
******************************************************************************/
NEXUS_PidChannelHandle B_DVR_TranscodeService_GetDataInjectionPidChannel(
    B_DVR_TranscodeServiceHandle transcodeService);

#ifdef __cplusplus
}
#endif

#endif /*_B_DVR_TRANSCODESERVICE_H */
