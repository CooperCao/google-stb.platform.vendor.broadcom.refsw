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
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/

#include "b_dvr_manager.h"
#include "b_dvr_manager_priv.h"
#include "b_dvr_transcodeservice.h"
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif
BDBG_MODULE(b_dvr_transcodeservice);
BDBG_OBJECT_ID(B_DVR_TranscodeService);
struct B_DVR_TranscodeService
{
    BDBG_OBJECT(B_DVR_TranscodeService)
    unsigned index;
    B_DVR_TranscodeServiceRequest transcodeServiceRequest;
    #if NEXUS_HAS_STREAM_MUX
    NEXUS_VideoEncoderHandle videoEncode;
    NEXUS_VideoEncoderDelayRange videoEncodeDelay;
    NEXUS_VideoEncoderStartSettings videoEncoderStartSettings;
    #endif
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_AudioMixerHandle encodedAudioMixer;
    NEXUS_AudioMixerHandle passThroughAudioMixer[NEXUS_NUM_AUDIO_DECODERS];
    #if NEXUS_HAS_STREAM_MUX
    NEXUS_AudioEncoderHandle audioEncode;
    NEXUS_AudioEncoderSettings audioEncodeSettings;
    NEXUS_AudioMuxOutputDelayStatus audioMuxOutputDelay;
    NEXUS_AudioMuxOutputStartSettings audioMuxOutputStartSettings[NEXUS_MAX_MUX_PIDS];
    NEXUS_AudioMuxOutputHandle audioMuxOutput[NEXUS_MAX_MUX_PIDS];
    NEXUS_StreamMuxHandle streamMux;
    NEXUS_StreamMuxOutput streamMuxOutput;
    NEXUS_StreamMuxStartSettings streamMuxStartSettings;
    NEXUS_StreamMuxCreateSettings streamMuxCreateSettings;
    #endif
    #if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelHandle syncChannel;
    #endif
    B_DVR_TranscodeServiceInputSettings inputSettings;
    B_DVR_TranscodeServiceOutputSettings outputSettings;
    B_DVR_TranscodeServiceSettings transcodeSettings;
    B_DVR_TranscodeServiceInputEsStream inputVideoEsStream;
    #if NEXUS_HAS_STREAM_MUX
    B_DVR_TranscodeServiceInputEsStream inputAudioEsStream[NEXUS_MAX_MUX_PIDS];
    #endif
    B_DVR_TranscodeServiceInputEsStream inputPcrEsStream;
    #if NEXUS_HAS_STREAM_MUX
    NEXUS_PlaypumpHandle  audioEncodePlaypump[NEXUS_MAX_MUX_PIDS];
    #endif
    NEXUS_PlaypumpHandle  videoEncodePlaypump;
    NEXUS_PlaypumpHandle  pcrPlaypump;
    NEXUS_PidChannelHandle pidChannelTranscodePcr;
    NEXUS_PidChannelHandle pidChannelTranscodePsi;
    B_DVR_ServiceCallback registeredCallback;
    void *appContext;
    bool transcoderStarted;
    unsigned videoEncodeCount;
    unsigned audioEncodeCount;
    unsigned audioPassthroughCount;
    unsigned audioMuxOutputCount;
    B_MutexHandle transcodeMutex;
};

#if NEXUS_HAS_STREAM_MUX
static void B_DVR_TranscodeService_P_StreamMuxFinish(void *transcodeContext, int param);
B_DVR_ERROR B_DVR_TranscodeService_P_AddInputVideoEsStream(B_DVR_TranscodeServiceHandle transcodeService,B_DVR_TranscodeServiceInputEsStream *inputEsStream);
B_DVR_ERROR B_DVR_TranscodeService_P_AddInputAudioEsStream(B_DVR_TranscodeServiceHandle transcodeService,B_DVR_TranscodeServiceInputEsStream *inputEsStream);
B_DVR_ERROR B_DVR_TranscodeService_P_RemoveInputVideoEsStream(B_DVR_TranscodeServiceHandle transcodeService,B_DVR_TranscodeServiceInputEsStream *inputEsStream);
B_DVR_ERROR B_DVR_TranscodeService_P_RemoveInputAudioEsStream(B_DVR_TranscodeServiceHandle transcodeService,B_DVR_TranscodeServiceInputEsStream *inputEsStream);

static void B_DVR_TranscodeService_P_StreamMuxFinish(void *transcodeContext, int param)
{
    B_DVR_TranscodeServiceHandle transcodeService = (B_DVR_TranscodeServiceHandle)transcodeContext;
    B_DVR_Event event;
    B_DVR_Service service;
    BSTD_UNUSED(param);
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_MSG(("B_DVR_TranscodeService_P_StreamMuxFinish index %u >>>",transcodeService->index));
    if(transcodeService->registeredCallback)
    {
        event = eB_DVR_EventTranscodeFinish;
        service = eB_DVR_ServiceTranscode;
        transcodeService->registeredCallback(transcodeService->appContext,transcodeService->index,event,service);
    }
    BDBG_MSG(("B_DVR_TranscodeService_P_StreamMuxFinish %u <<<",transcodeService->index));
    return;
}

B_DVR_ERROR B_DVR_TranscodeService_P_AddInputAudioEsStream(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputEsStream *inputEsStream)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    NEXUS_PlaypumpOpenSettings playpumpSettings;
    NEXUS_AudioMixerSettings audioMixerSettings;
    unsigned audioPassThroughMixerIndex = transcodeService->audioPassthroughCount;
    unsigned audioMuxOutputIndex = transcodeService->audioMuxOutputCount;
    NEXUS_Error   ret;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_ASSERT(inputEsStream);
    BDBG_MSG(("B_DVR_TranscodeService_P_AddInputAudioEsStream >>>"));

    /*each audio path needs a mux*/
    transcodeService->audioMuxOutput[audioMuxOutputIndex] = NEXUS_AudioMuxOutput_Create(NULL);
    if(!transcodeService->audioMuxOutput[audioMuxOutputIndex])
    {
        BDBG_ERR(("audio mux output create failed"));
        rc = B_DVR_INVALID_PARAMETER;
        goto error_audioMuxOutput;
    }

    if(inputEsStream->audioEncodeParams.audioPassThrough)
    {
        if(inputEsStream->audioEncodeParams.rateSmoothingEnable) 
        {
            NEXUS_AudioMixer_GetDefaultSettings(&audioMixerSettings);
            audioMixerSettings.mixUsingDsp = true;
            transcodeService->passThroughAudioMixer[audioPassThroughMixerIndex] = NEXUS_AudioMixer_Open(&audioMixerSettings);
            if(!transcodeService->passThroughAudioMixer[audioPassThroughMixerIndex])
            {
                BDBG_ERR(("audio mixer open failed"));
                rc = B_DVR_INVALID_PARAMETER;
                goto error_audioMixer;
            }
            /* Connect mixer  to decoder */
            NEXUS_AudioMixer_AddInput(transcodeService->passThroughAudioMixer[audioPassThroughMixerIndex],
                                      NEXUS_AudioDecoder_GetConnector(inputEsStream->audioEncodeParams.audioDecoder,
                                      NEXUS_AudioDecoderConnectorType_eCompressed));
            audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(inputEsStream->audioEncodeParams.audioDecoder,
                                                                       NEXUS_AudioDecoderConnectorType_eCompressed);
            NEXUS_AudioMixer_SetSettings(transcodeService->passThroughAudioMixer[audioPassThroughMixerIndex],
                                         &audioMixerSettings);
             /* Connect mux to mixer */
            NEXUS_AudioOutput_AddInput(NEXUS_AudioMuxOutput_GetConnector(transcodeService->audioMuxOutput[audioMuxOutputIndex]),
            NEXUS_AudioMixer_GetConnector(transcodeService->passThroughAudioMixer[audioPassThroughMixerIndex]));
        }
        else
        {
            /* Connect mux to decoder */
            ret = NEXUS_AudioOutput_AddInput(NEXUS_AudioMuxOutput_GetConnector(transcodeService->audioMuxOutput[audioMuxOutputIndex]),
            NEXUS_AudioDecoder_GetConnector(inputEsStream->audioEncodeParams.audioDecoder,
                                            NEXUS_AudioDecoderConnectorType_eCompressed));
            BDBG_MSG(("connect mux to bypass decoder %#x (ret = %d)\n",inputEsStream->audioEncodeParams.audioDecoder, ret));
        }
    }
    else
    {
        if(inputEsStream->audioEncodeParams.rateSmoothingEnable)
        {
            NEXUS_AudioMixer_GetDefaultSettings(&audioMixerSettings);
            audioMixerSettings.mixUsingDsp = true;
            transcodeService->encodedAudioMixer = NEXUS_AudioMixer_Open(&audioMixerSettings);
            if(!transcodeService->encodedAudioMixer)
            {
                BDBG_ERR(("audio mixer open failed"));
                rc = B_DVR_INVALID_PARAMETER;
                goto error_audioMixer;
            }

            /* Connect decoder to mixer and set as master */
            NEXUS_AudioMixer_AddInput(transcodeService->encodedAudioMixer,
                                      NEXUS_AudioDecoder_GetConnector(inputEsStream->audioEncodeParams.audioDecoder,
                                      NEXUS_AudioDecoderConnectorType_eStereo));
            audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(inputEsStream->audioEncodeParams.audioDecoder,
                                                                        NEXUS_AudioDecoderConnectorType_eStereo);
            NEXUS_AudioMixer_SetSettings(transcodeService->encodedAudioMixer,
                                         &audioMixerSettings);
        }
        NEXUS_AudioEncoder_GetDefaultSettings(&transcodeService->audioEncodeSettings);
        transcodeService->audioEncodeSettings.codec = inputEsStream->audioEncodeParams.codec;
        transcodeService->audioEncode = NEXUS_AudioEncoder_Open(&transcodeService->audioEncodeSettings);
        if(!transcodeService->audioEncode)
        {
            BDBG_ERR(("audio encode open failed"));
            rc = B_DVR_INVALID_PARAMETER;
            goto error_audioEncode;
        }
        if(inputEsStream->audioEncodeParams.rateSmoothingEnable)
        {
            /* Connect mixer to encoder */
            NEXUS_AudioEncoder_AddInput(transcodeService->audioEncode,
                                        NEXUS_AudioMixer_GetConnector(transcodeService->encodedAudioMixer));
        }
        else
        {
            /* Connect decoder to encoder */
            ret = NEXUS_AudioEncoder_AddInput(transcodeService->audioEncode,
                                              NEXUS_AudioDecoder_GetConnector(inputEsStream->audioEncodeParams.audioDecoder,
                                              NEXUS_AudioDecoderConnectorType_eStereo));
            BDBG_MSG(("connect encoder to  decoder %#x (ret = %d)\n",inputEsStream->audioEncodeParams.audioDecoder, ret));
        }
        /* Connect mux to encoder */
        NEXUS_AudioOutput_AddInput(NEXUS_AudioMuxOutput_GetConnector(transcodeService->audioMuxOutput[audioMuxOutputIndex]),
                                  NEXUS_AudioEncoder_GetConnector(transcodeService->audioEncode));
    }

    #if (BCHP_VER >= BCHP_VER_B1)
    if(transcodeService->transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_RealTime)
    { 
        /*Connect mixer to a dummy output*/
        NEXUS_AudioOutput_AddInput(NEXUS_AudioDummyOutput_GetConnector(inputEsStream->audioEncodeParams.audioDummy),
                                   NEXUS_AudioMixer_GetConnector(transcodeService->encodedAudioMixer));
        /* TODO: add mixer for audio pass through*/
    }
    #endif

    transcodeService->streamMuxStartSettings.audio[audioMuxOutputIndex].pid = inputEsStream->esStreamInfo.pid;
    transcodeService->streamMuxStartSettings.audio[audioMuxOutputIndex].pesId = inputEsStream->pesId;
    transcodeService->streamMuxStartSettings.audio[audioMuxOutputIndex].muxOutput = transcodeService->audioMuxOutput[audioMuxOutputIndex];
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpSettings);
    playpumpSettings.streamMuxCompatible = true;
    playpumpSettings.fifoSize = 16384;    /* reduce FIFO size allocated for playpump */
    playpumpSettings.numDescriptors = 64; /* set number of descriptors */
    transcodeService->audioEncodePlaypump[audioMuxOutputIndex] = NEXUS_Playpump_Open(inputEsStream->playpumpIndex,&playpumpSettings);
    if(!transcodeService->audioEncodePlaypump[audioMuxOutputIndex] )
    {
        BDBG_ERR(("playpump open failed"));
        rc = B_DVR_INVALID_PARAMETER;
        goto error_playpump;
    }
    transcodeService->streamMuxStartSettings.audio[audioMuxOutputIndex].playpump = transcodeService->audioEncodePlaypump[audioMuxOutputIndex];

    if(transcodeService->transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)
    {
        transcodeService->audioMuxOutputStartSettings[audioMuxOutputIndex].nonRealTime = true;
        transcodeService->audioMuxOutputStartSettings[audioMuxOutputIndex].stcChannel = inputEsStream->stcChannel;
    }
    else
    {
        transcodeService->audioMuxOutputStartSettings[audioMuxOutputIndex].nonRealTime = false;
        transcodeService->audioMuxOutputStartSettings[audioMuxOutputIndex].stcChannel = transcodeService->transcodeServiceRequest.transcodeStcChannel;
    }
    if(!audioMuxOutputIndex)
    {
        NEXUS_AudioMuxOutput_GetDelayStatus(transcodeService->audioMuxOutput[audioMuxOutputIndex],
                                            inputEsStream->audioEncodeParams.codec, &transcodeService->audioMuxOutputDelay);
    }
    else
    {
        NEXUS_AudioMuxOutputDelayStatus audioMuxOutputDelay;
        NEXUS_AudioMuxOutput_GetDelayStatus(transcodeService->audioMuxOutput[audioMuxOutputIndex],
                                            inputEsStream->audioEncodeParams.codec, &audioMuxOutputDelay);
        if(audioMuxOutputDelay.endToEndDelay > transcodeService->audioMuxOutputDelay.endToEndDelay)
        {
            transcodeService->audioMuxOutputDelay.endToEndDelay = audioMuxOutputDelay.endToEndDelay;
        }
    }
    BDBG_MSG(("Audio codec %d end-to-end delay = %u ms",
              inputEsStream->audioEncodeParams.codec,
              transcodeService->audioMuxOutputDelay.endToEndDelay));
    transcodeService->audioMuxOutputCount++;
    if(inputEsStream->audioEncodeParams.audioPassThrough)
    {
        transcodeService->audioPassthroughCount ++;
    }
    BDBG_MSG(("B_DVR_TranscodeService_P_AddInputAudioEsStream >>>"));
    return rc;
error_playpump:
    if(transcodeService->audioEncode)
    {
        NEXUS_AudioEncoder_Close(transcodeService->audioEncode);
        transcodeService->audioEncode = NULL;
    }
error_audioEncode:
   if(inputEsStream->audioEncodeParams.rateSmoothingEnable)
   {
       if(inputEsStream->audioEncodeParams.audioPassThrough)
       {    
           NEXUS_AudioMixer_Close(transcodeService->passThroughAudioMixer[audioPassThroughMixerIndex]);
           transcodeService->passThroughAudioMixer[audioPassThroughMixerIndex] = NULL;
       }
       else
       {
           NEXUS_AudioMixer_Close(transcodeService->encodedAudioMixer);
           transcodeService->encodedAudioMixer = NULL;
       }
   }
error_audioMixer:
    NEXUS_AudioMuxOutput_Destroy(transcodeService->audioMuxOutput[audioMuxOutputIndex]);
    transcodeService->audioMuxOutput[audioMuxOutputIndex]=NULL;
    transcodeService->streamMuxStartSettings.audio[audioMuxOutputIndex].muxOutput = NULL;
error_audioMuxOutput:
   BDBG_MSG(("B_DVR_TranscodeService_P_AddInputAudioEsStream error >>>"));
   return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_P_AddInputVideoEsStream(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputEsStream *inputEsStream)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    NEXUS_PlaypumpOpenSettings playpumpSettings;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_ASSERT(inputEsStream);
    BDBG_ASSERT(transcodeService->videoEncodeCount < 1);
    BDBG_MSG(("B_DVR_TranscodeService_P_AddInputVideoEsStream >>>"));
    transcodeService->streamMuxStartSettings.video[0].pid = inputEsStream->esStreamInfo.pid;
    transcodeService->streamMuxStartSettings.video[0].pesId = inputEsStream->pesId;
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpSettings);
    playpumpSettings.streamMuxCompatible = true;
    playpumpSettings.fifoSize = 16384;    /* reduce FIFO size allocated for playpump */
    playpumpSettings.numDescriptors = 64; /* set number of descriptors */
    transcodeService->videoEncodePlaypump = NEXUS_Playpump_Open(inputEsStream->playpumpIndex,&playpumpSettings);
    if(!transcodeService->videoEncodePlaypump)
    {
        BDBG_ERR(("playpump open failed"));
        rc = B_DVR_INVALID_PARAMETER;
        goto error_playpump;
    }
    transcodeService->streamMuxStartSettings.video[0].playpump = transcodeService->videoEncodePlaypump;
    transcodeService->streamMuxStartSettings.video[0].encoder = transcodeService->videoEncode;

    NEXUS_VideoEncoder_GetDefaultStartSettings(&transcodeService->videoEncoderStartSettings);
     /* to allow 23.976p passthru; TODO: allow user to configure minimum framerate to achieve lower delay!
     * Note: lower minimum framerate means longer encode delay */
    transcodeService->videoEncoderStartSettings.bounds.inputFrameRate.min = NEXUS_VideoFrameRate_e23_976;
    /* to allow 24 ~ 60p dynamic frame rate coding TODO: allow user to config higher minimum frame rate for lower delay! */
    transcodeService->videoEncoderStartSettings.bounds.outputFrameRate.min = NEXUS_VideoFrameRate_e23_976;
    transcodeService->videoEncoderStartSettings.bounds.outputFrameRate.max = NEXUS_VideoFrameRate_e60;
    /* max encode size allows 1080p encode; TODO: allow user to choose lower max resolution for lower encode delay */
    transcodeService->videoEncoderStartSettings.bounds.inputDimension.max.width = 1920;
    transcodeService->videoEncoderStartSettings.bounds.inputDimension.max.height = 1088;
    transcodeService->videoEncoderStartSettings.codec = inputEsStream->videoEncodeParams.codec;
    transcodeService->videoEncoderStartSettings.input = transcodeService->display;
    transcodeService->videoEncoderStartSettings.interlaced = inputEsStream->videoEncodeParams.interlaced;
    transcodeService->videoEncoderStartSettings.level = inputEsStream->videoEncodeParams.level;
    transcodeService->videoEncoderStartSettings.profile = inputEsStream->videoEncodeParams.profile;

    /* 0 to use default 750ms rate buffer delay; TODO: allow user to adjust it to lower encode delay at cost of quality reduction! */
    transcodeService->videoEncoderStartSettings.rateBufferDelay = 0;

    transcodeService->videoEncoderStartSettings.lowDelayPipeline = false;
    transcodeService->videoEncoderStartSettings.nonRealTime = 
        (transcodeService->transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)? true:false;
    transcodeService->videoEncoderStartSettings.stcChannel = inputEsStream->stcChannel;

    /* default ON cc user data */
    transcodeService->videoEncoderStartSettings.encodeUserData = true;

    NEXUS_VideoWindow_AddInput(transcodeService->window, NEXUS_VideoDecoder_GetConnector(inputEsStream->videoEncodeParams.videoDecoder));
    transcodeService->videoEncodeCount++;
    BDBG_MSG(("B_DVR_TranscodeService_P_AddInputVideoEsStream <<<"));
    return rc;
error_playpump:
    BDBG_MSG(("B_DVR_TranscodeService_P_AddInputVideoEsStream error <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_P_RemoveInputAudioEsStream(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputEsStream *inputEsStream)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned audioMuxOutputIndex = --transcodeService->audioMuxOutputCount;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);

    BDBG_ERR(("B_DVR_TranscodeService_P_RemoveInputAudioEsStream %u >>>",audioMuxOutputIndex));
        /* Reset the PID value */
        transcodeService->streamMuxStartSettings.audio[audioMuxOutputIndex].pid= 0;
        transcodeService->streamMuxStartSettings.audio[audioMuxOutputIndex].pesId = 0;

        /* Close playpump resource*/
        NEXUS_Playpump_Close(transcodeService->audioEncodePlaypump[audioMuxOutputIndex]);
        transcodeService->audioEncodePlaypump[audioMuxOutputIndex] = NULL;
        transcodeService->streamMuxStartSettings.audio[audioMuxOutputIndex].playpump  = NULL;

    if(inputEsStream->audioEncodeParams.audioPassThrough)
    {
        if(inputEsStream->audioEncodeParams.rateSmoothingEnable)
        {
            unsigned audioPassThroughMixerIndex = --transcodeService->audioPassthroughCount;
            /* Disconnect mixer from mux */
            NEXUS_AudioOutput_RemoveInput(NEXUS_AudioMuxOutput_GetConnector(transcodeService->audioMuxOutput[audioMuxOutputIndex]),
                                          NEXUS_AudioMixer_GetConnector(transcodeService->passThroughAudioMixer[audioPassThroughMixerIndex]));

            /* Disconnect decoder from mixer*/
            NEXUS_AudioMixer_RemoveInput(transcodeService->passThroughAudioMixer[audioPassThroughMixerIndex],
                                         NEXUS_AudioDecoder_GetConnector(inputEsStream->audioEncodeParams.audioDecoder,
                                         NEXUS_AudioDecoderConnectorType_eCompressed));

            /* Close audio mixer resource */
            NEXUS_AudioMixer_Close(transcodeService->passThroughAudioMixer[audioPassThroughMixerIndex]);
            transcodeService->passThroughAudioMixer[audioPassThroughMixerIndex] = NULL;
        }
        else
        {
            /* Disconnect decoder from mux */
            NEXUS_AudioOutput_RemoveInput(NEXUS_AudioMuxOutput_GetConnector(transcodeService->audioMuxOutput[audioMuxOutputIndex]),
            NEXUS_AudioDecoder_GetConnector(inputEsStream->audioEncodeParams.audioDecoder,
                                            NEXUS_AudioDecoderConnectorType_eCompressed));
        }
    }
    else
    {
        /* Disconnect mux from encoder */
        NEXUS_AudioOutput_RemoveInput(NEXUS_AudioMuxOutput_GetConnector(transcodeService->audioMuxOutput[audioMuxOutputIndex]),
                                      NEXUS_AudioEncoder_GetConnector(transcodeService->audioEncode));
        if(inputEsStream->audioEncodeParams.rateSmoothingEnable) 
        {
            /* Disconnect mixer from encoder */
            NEXUS_AudioEncoder_RemoveInput(transcodeService->audioEncode,
                                           NEXUS_AudioMixer_GetConnector(transcodeService->encodedAudioMixer));
        }
        else
        {
            /* Disconnect decoder from encoder */
            NEXUS_AudioEncoder_RemoveInput(transcodeService->audioEncode,
                                           NEXUS_AudioDecoder_GetConnector(inputEsStream->audioEncodeParams.audioDecoder,
                                           NEXUS_AudioDecoderConnectorType_eStereo));
        }
        /* Close encoder resource */
        NEXUS_AudioEncoder_Close(transcodeService->audioEncode);
        transcodeService->audioEncode = NULL;
        if(inputEsStream->audioEncodeParams.rateSmoothingEnable) 
        {
            /* Disconnect decoder from mixer*/
            NEXUS_AudioMixer_RemoveInput(transcodeService->encodedAudioMixer,
                                         NEXUS_AudioDecoder_GetConnector(inputEsStream->audioEncodeParams.audioDecoder,
                                         NEXUS_AudioDecoderConnectorType_eStereo));
            /* Close audio mixer resource */
            NEXUS_AudioMixer_Close(transcodeService->encodedAudioMixer);
            transcodeService->encodedAudioMixer = NULL;
        }
    }

    #if (BCHP_VER >= BCHP_VER_B1)
    if(transcodeService->transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_RealTime)
    {
        /* Disconnect mixer from the dummy output */
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(inputEsStream->audioEncodeParams.audioDummy));
        NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(inputEsStream->audioEncodeParams.audioDummy));
    }
    #endif

    /* Destroy audio mux output resource*/
    NEXUS_AudioMuxOutput_Destroy(transcodeService->audioMuxOutput[audioMuxOutputIndex]);
    transcodeService->audioMuxOutput[audioMuxOutputIndex] = NULL;
    transcodeService->streamMuxStartSettings.audio[audioMuxOutputIndex].muxOutput = NULL;
    BDBG_MSG(("B_DVR_TranscodeService_P_RemoveInputAudioEsStream <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_P_RemoveInputVideoEsStream(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputEsStream *inputEsStream)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_ASSERT(transcodeService);

    BDBG_MSG(("B_DVR_TranscodeService_P_RemoveInputVideoEsStream >>>"));
    NEXUS_VideoWindow_RemoveInput(transcodeService->window, NEXUS_VideoDecoder_GetConnector(inputEsStream->videoEncodeParams.videoDecoder));
    /* Reset the PID value */
    transcodeService->streamMuxStartSettings.video[0].pid = 0;
    transcodeService->streamMuxStartSettings.video[0].pesId = 0;
    /* Close playpump resource */
    NEXUS_Playpump_Close(transcodeService->videoEncodePlaypump);
    transcodeService->videoEncodePlaypump = NULL;
    transcodeService->streamMuxStartSettings.video[0].playpump = NULL;
    transcodeService->streamMuxStartSettings.video[0].encoder = NULL;
    transcodeService->videoEncodeCount--;
    BDBG_MSG(("B_DVR_TranscodeService_P_RemoveInputVideoEsStream <<<"));
    return rc;
}


B_DVR_TranscodeServiceHandle B_DVR_TranscodeService_Open(
    B_DVR_TranscodeServiceRequest *transcodeServiceRequest)
{
    B_DVR_TranscodeServiceHandle transcodeService=NULL;
    B_DVR_ManagerHandle dvrManager;
    NEXUS_StcChannelSettings stcSettings;
    #if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
    #endif
    unsigned index;
    dvrManager = B_DVR_Manager_GetHandle();
    BDBG_ASSERT(transcodeServiceRequest);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_TranscodeService_Open >>>"));

    transcodeService = BKNI_Malloc(sizeof(*transcodeService));
    if (!transcodeService)
    {
        BDBG_ERR(("transcodeService alloc failed"));
        goto error_alloc;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*transcodeService),
                                            true,__FUNCTION__,__LINE__);
    BKNI_Memset(transcodeService, 0, sizeof(*transcodeService));
    BDBG_OBJECT_SET(transcodeService,B_DVR_TranscodeService);
    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    for(index=0;index<B_DVR_MAX_TRANSCODE;index++)
    {
        if(!dvrManager->transcodeService[index])
        {
            transcodeService->index = index;
            BDBG_MSG(("transcodeService Index %u",index));
            break;
        }

    }
    if(index>=B_DVR_MAX_TRANSCODE)
    {
        B_Mutex_Unlock(dvrManager->dvrManagerMutex);
        BDBG_ERR(("No more transcode service resources available"));
        goto error_transcode;
    }

    dvrManager->transcodeServiceCount++;
    dvrManager->transcodeService[transcodeService->index]=transcodeService;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);

    transcodeService->transcodeMutex = B_Mutex_Create(NULL);
    if( !transcodeService->transcodeMutex)
    {
        BDBG_ERR(("mutex create failed"));
        goto error_mutex;
    }

    BKNI_Memcpy((void *)&transcodeService->transcodeServiceRequest,(void *)transcodeServiceRequest,sizeof(*transcodeServiceRequest));

    transcodeService->videoEncode = NEXUS_VideoEncoder_Open(transcodeService->index, NULL);
    if(!transcodeService->videoEncode)
    {
        BDBG_ERR(("video encode open failed"));
        goto error_videoEncode;

    }
   
    NEXUS_Display_GetDefaultSettings(&transcodeService->transcodeSettings.displaySettings);
    transcodeService->transcodeSettings.displaySettings.displayType = NEXUS_DisplayType_eAuto;
    transcodeService->transcodeSettings.displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    transcodeService->transcodeSettings.displaySettings.format = NEXUS_VideoFormat_e480p;;
    transcodeService->transcodeSettings.displaySettings.frameRateMaster = NULL;
    transcodeService->display = NEXUS_Display_Open(transcodeServiceRequest->displayIndex,
                                                   &transcodeService->transcodeSettings.displaySettings);
    if(!transcodeService->display)
    {
        BDBG_ERR(("transcode display open failed: display index %d",transcodeServiceRequest->displayIndex));
        goto error_display;
    }

    transcodeService->window = NEXUS_VideoWindow_Open(transcodeService->display,0);
    if(!transcodeService->window)
    {
        BDBG_ERR(("transcode window open for display failed"));
        goto error_window;
    }
    #if NEXUS_HAS_SYNC_CHANNEL    
    NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
    transcodeService->syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
    if(!transcodeService->syncChannel)
    {
        BDBG_ERR(("sync channel create failed"));
        goto error_syncChannel;
    }
    #endif
    NEXUS_StreamMux_GetDefaultCreateSettings(&transcodeService->streamMuxCreateSettings);
    transcodeService->streamMuxCreateSettings.finished.callback = B_DVR_TranscodeService_P_StreamMuxFinish;
    transcodeService->streamMuxCreateSettings.finished.context = (void *)transcodeService;
    transcodeService->streamMux = NEXUS_StreamMux_Create(&transcodeService->streamMuxCreateSettings);
    if(!transcodeService->streamMux)
    {
        BDBG_ERR(("streamMux create failed"));
        goto error_streamMux;
    }
    NEXUS_StreamMux_GetDefaultStartSettings(&transcodeService->streamMuxStartSettings);

    if(transcodeServiceRequest->transcodeStcChannel)
    { 
        NEXUS_StcChannel_GetSettings(transcodeServiceRequest->transcodeStcChannel, &stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
        NEXUS_StcChannel_SetSettings(transcodeServiceRequest->transcodeStcChannel, &stcSettings);
        transcodeService->streamMuxStartSettings.stcChannel = transcodeServiceRequest->transcodeStcChannel;
    }
    if(transcodeServiceRequest->transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)
    {
        transcodeService->streamMuxStartSettings.nonRealTime = true;
        transcodeService->streamMuxStartSettings.nonRealTimeRate = 32 * NEXUS_NORMAL_PLAY_SPEED; /* AFAP */
    }
    else
    {
        transcodeService->streamMuxStartSettings.nonRealTime = false;
    }

    transcodeService->streamMuxStartSettings.transportType = NEXUS_TransportType_eTs;
    BDBG_MSG(("B_DVR_TranscodeService_Open index %u<<<",transcodeService->index));
    return transcodeService;
error_streamMux:  
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannel_Destroy(transcodeService->syncChannel);
error_syncChannel:
#endif
    NEXUS_VideoWindow_Close(transcodeService->window);
error_window:
    NEXUS_Display_Close(transcodeService->display);
error_display:
    NEXUS_VideoEncoder_Close(transcodeService->videoEncode);
error_videoEncode:
    B_Mutex_Destroy(transcodeService->transcodeMutex);
error_mutex:
    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    dvrManager->transcodeServiceCount--;
    dvrManager->transcodeService[transcodeService->index]=NULL;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);
error_transcode:
    BDBG_OBJECT_DESTROY(transcodeService,B_DVR_TranscodeService);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*transcodeService),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(transcodeService);
error_alloc:
    return NULL;
}

B_DVR_ERROR B_DVR_TranscodeService_Close(
    B_DVR_TranscodeServiceHandle transcodeService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_MSG(("B_DVR_TranscodeService_Close index %d >>>",transcodeService->index));
    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    dvrManager->transcodeServiceCount--;
    dvrManager->transcodeService[transcodeService->index]=NULL;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);
    B_Mutex_Lock(transcodeService->transcodeMutex);
    NEXUS_StreamMux_Destroy(transcodeService->streamMux);
    #if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannel_Destroy(transcodeService->syncChannel);
    #endif
    NEXUS_VideoWindow_Close(transcodeService->window);
    NEXUS_Display_Close(transcodeService->display);
    NEXUS_VideoEncoder_Close(transcodeService->videoEncode);
    B_Mutex_Unlock(transcodeService->transcodeMutex);
    B_Mutex_Destroy(transcodeService->transcodeMutex);
    BDBG_OBJECT_DESTROY(transcodeService,B_DVR_TranscodeService);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*transcodeService),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(transcodeService);
    BDBG_MSG(("B_DVR_TranscodeService_Close  <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_Start(
    B_DVR_TranscodeServiceHandle transcodeService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned index=0;
    unsigned endToEndDelay;
    NEXUS_StcChannelPairSettings stcAudioVideoPair;
    #if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
    #endif
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_MSG(("B_DVR_TranscodeService_Start >>>"));
    B_Mutex_Lock(transcodeService->transcodeMutex);

    /* NOTE: video encoder delay is in 27MHz ticks */
    NEXUS_VideoEncoder_GetDelayRange(transcodeService->videoEncode,&transcodeService->transcodeSettings.videoEncodeSettings,
                                     &transcodeService->videoEncoderStartSettings, &transcodeService->videoEncodeDelay);
    BDBG_MSG(("Video encoder end-to-end delay = %u ms; maximum allowed: %u ms",
              transcodeService->videoEncodeDelay.min/27000, transcodeService->videoEncodeDelay.max/27000));

    endToEndDelay = transcodeService->audioMuxOutputDelay.endToEndDelay * 27000; /* in 27MHz ticks */

    if(endToEndDelay > transcodeService->videoEncodeDelay.min)
    {
        if(endToEndDelay > transcodeService->videoEncodeDelay.max)
        {
            BDBG_WRN((" Audio Delay is way too big! Use max video delay!"));
            endToEndDelay = transcodeService->videoEncodeDelay.max;
        }
        else
        {
            BDBG_MSG(("Use audio delay %u ms %u ticks@27Mhz!\n", endToEndDelay/27000, endToEndDelay));
        }
    }
    else
    {
        endToEndDelay = transcodeService->videoEncodeDelay.min;
        BDBG_MSG(("Use video Delay %u ms or %u ticks@27Mhz!\n\n", endToEndDelay/27000, endToEndDelay));
    }
    if((transcodeService->transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime) &&
      transcodeService->audioMuxOutputStartSettings[0].stcChannel)
    { 
        #if NEXUS_HAS_SYNC_CHANNEL
        NEXUS_SyncChannel_GetSettings(transcodeService->syncChannel, &syncChannelSettings);
        #endif
        /* NRT mode pairs AV stc channels */
        NEXUS_StcChannel_GetDefaultPairSettings(&stcAudioVideoPair);
        stcAudioVideoPair.connected = true;
        stcAudioVideoPair.window = 300; /* 300ms AV window means when source discontinuity occurs, up to 300ms transition could occur with NRT transcoded stream */
        NEXUS_StcChannel_SetPairSettings(transcodeService->videoEncoderStartSettings.stcChannel,transcodeService->audioMuxOutputStartSettings[0].stcChannel, &stcAudioVideoPair);
        #if NEXUS_HAS_SYNC_CHANNEL
        /* disable these flags for NRT mode to avoid startup mute */
        syncChannelSettings.enableMuteControl = false;
        syncChannelSettings.enablePrecisionLipsync = false;
        NEXUS_SyncChannel_SetSettings(transcodeService->syncChannel, &syncChannelSettings);
        #endif
    }

    NEXUS_VideoEncoder_GetSettings(transcodeService->videoEncode,&transcodeService->transcodeSettings.videoEncodeSettings);
    transcodeService->transcodeSettings.videoEncodeSettings.encoderDelay = endToEndDelay;
    NEXUS_VideoEncoder_SetSettings(transcodeService->videoEncode,&transcodeService->transcodeSettings.videoEncodeSettings);
    NEXUS_VideoEncoder_Start(transcodeService->videoEncode,
                             &transcodeService->videoEncoderStartSettings);
    for(index=0;index<transcodeService->audioMuxOutputCount;index++)
    { 
        transcodeService->audioMuxOutputStartSettings[index].presentationDelay = endToEndDelay/27000;
        NEXUS_AudioMuxOutput_Start(transcodeService->audioMuxOutput[index],
                                   &transcodeService->audioMuxOutputStartSettings[index]);
    }
    #if (BCHP_VER >= BCHP_VER_B1)
    for(index=0;index<transcodeService->audioPassthroughCount;index++)
    {
       if(transcodeService->passThroughAudioMixer[index])
       {
           NEXUS_AudioMixer_Start(transcodeService->passThroughAudioMixer[index]);
       }
    }
    if(transcodeService->encodedAudioMixer)
    {
        NEXUS_AudioMixer_Start(transcodeService->encodedAudioMixer);
    }
    #endif
    NEXUS_StreamMux_Start(transcodeService->streamMux,
                          &transcodeService->streamMuxStartSettings,
                          &transcodeService->streamMuxOutput);
    B_Mutex_Unlock(transcodeService->transcodeMutex);
    BDBG_MSG(("B_DVR_TranscodeService_Start <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_Stop(
    B_DVR_TranscodeServiceHandle transcodeService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned index=0;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_MSG(("B_DVR_TranscodeService_Stop index %u >>>",transcodeService->index));
    B_Mutex_Lock(transcodeService->transcodeMutex);
    NEXUS_StreamMux_Finish(transcodeService->streamMux);

    #if (BCHP_VER >= BCHP_VER_B1)
    for(index=0;index<transcodeService->audioPassthroughCount;index++)
    {
       if(transcodeService->passThroughAudioMixer[index])
       {
           NEXUS_AudioMixer_Stop(transcodeService->passThroughAudioMixer[index]);
       }
    }
    if(transcodeService->encodedAudioMixer)
    {
        NEXUS_AudioMixer_Stop(transcodeService->encodedAudioMixer);
    }
    #endif
    for(index=0;index<transcodeService->audioMuxOutputCount;index++)
    { 
        NEXUS_AudioMuxOutput_Stop(transcodeService->audioMuxOutput[index]);
    }
    NEXUS_VideoEncoder_Stop(transcodeService->videoEncode,NULL);
    B_Mutex_Unlock(transcodeService->transcodeMutex);
    BDBG_MSG(("B_DVR_TranscodeService_Stop <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_GetInputSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_ASSERT(pSettings);
    BDBG_MSG(("B_DVR_TranscodeService_GetInputSettings index %u >>>",transcodeService->index));
    B_Mutex_Lock(transcodeService->transcodeMutex);
    BKNI_Memcpy((void *)pSettings,(void *)&transcodeService->inputSettings,sizeof(*pSettings));
    B_Mutex_Unlock(transcodeService->transcodeMutex);
    BDBG_MSG(("B_DVR_TranscodeService_GetInputSettings index %u <<<",transcodeService->index));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_SetInputSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputSettings *pSettings)
{

    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_ASSERT(pSettings);
    BDBG_MSG(("B_DVR_TranscodeService_SetInputSettings index %u >>>",transcodeService->index));
    B_Mutex_Lock(transcodeService->transcodeMutex);
    BKNI_Memcpy((void *)&transcodeService->inputSettings,(void *)pSettings,sizeof(*pSettings));
    B_Mutex_Unlock(transcodeService->transcodeMutex);
    BDBG_MSG(("B_DVR_TranscodeService_SetInputSettings index %u <<<",transcodeService->index));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_GetOutputSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceOutputSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_ASSERT(pSettings);
    BDBG_MSG(("B_DVR_TranscodeService_GetOutputSettings index %u >>>",transcodeService->index));
    B_Mutex_Lock(transcodeService->transcodeMutex);
    BKNI_Memcpy((void *)pSettings,(void *)&transcodeService->outputSettings,sizeof(*pSettings));
    B_Mutex_Unlock(transcodeService->transcodeMutex);
    BDBG_MSG(("B_DVR_TranscodeService_GetOutputSettings index %u <<<",transcodeService->index));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_SetOutputSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceOutputSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_ASSERT(pSettings);
    BDBG_MSG(("B_DVR_TranscodeService_SetOutputSettings index %u >>>",transcodeService->index));
    B_Mutex_Lock(transcodeService->transcodeMutex);
    BKNI_Memcpy((void *)&transcodeService->outputSettings,(void *)pSettings,sizeof(*pSettings));
    B_Mutex_Unlock(transcodeService->transcodeMutex);
    BDBG_MSG(("B_DVR_TranscodeService_SetOutputSettings index %u <<<",transcodeService->index));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_GetSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_ASSERT(pSettings);
    BDBG_MSG(("B_DVR_TranscodeService_Getsettings index %u >>>",transcodeService->index));
    B_Mutex_Lock(transcodeService->transcodeMutex);
    if(transcodeService->window)
    {
        NEXUS_VideoWindow_GetDnrSettings(transcodeService->window,&transcodeService->transcodeSettings.dnrSettings);
        NEXUS_VideoWindow_GetMadSettings(transcodeService->window,&transcodeService->transcodeSettings.madSettings);
        NEXUS_VideoWindow_GetScalerSettings(transcodeService->window,&transcodeService->transcodeSettings.scalerSettings);
        NEXUS_VideoWindow_GetSettings(transcodeService->window,&transcodeService->transcodeSettings.windowSettings);
    }

    if(transcodeService->display)
    {
        NEXUS_Display_GetSettings(transcodeService->display,&transcodeService->transcodeSettings.displaySettings);
        NEXUS_Display_GetStgSettings(transcodeService->display, &transcodeService->transcodeSettings.stgSettings);
    }

    if(transcodeService->videoEncode)
    {
        NEXUS_VideoEncoder_GetSettings(transcodeService->videoEncode,
                                       &transcodeService->transcodeSettings.videoEncodeSettings);
    }

    
    BKNI_Memcpy((void *)pSettings,(void *)&transcodeService->transcodeSettings,sizeof(*pSettings));
    B_Mutex_Unlock(transcodeService->transcodeMutex);
    BDBG_MSG(("B_DVR_TranscodeService_Getsettings index %u <<<",transcodeService->index));
    return rc;
}


B_DVR_ERROR B_DVR_TranscodeService_SetSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_ASSERT(pSettings);
    BDBG_MSG(("B_DVR_TranscodeService_SetSettings index %u>>>",transcodeService->index));
    B_Mutex_Lock(transcodeService->transcodeMutex);
    BKNI_Memcpy((void *)&transcodeService->transcodeSettings,(void *)pSettings,sizeof(*pSettings));
    if(transcodeService->window)
    { 
        NEXUS_VideoWindow_SetDnrSettings(transcodeService->window,&transcodeService->transcodeSettings.dnrSettings);
        NEXUS_VideoWindow_SetMadSettings(transcodeService->window,&transcodeService->transcodeSettings.madSettings);
        NEXUS_VideoWindow_SetScalerSettings(transcodeService->window,&transcodeService->transcodeSettings.scalerSettings);
        NEXUS_VideoWindow_SetSettings(transcodeService->window,&transcodeService->transcodeSettings.windowSettings);
    }

    if(transcodeService->display)
    {
        NEXUS_Display_SetSettings(transcodeService->display,&transcodeService->transcodeSettings.displaySettings);
        NEXUS_Display_SetStgSettings(transcodeService->display, &transcodeService->transcodeSettings.stgSettings);
    }

    if(transcodeService->videoEncode)
    {
        NEXUS_VideoEncoder_SetSettings(transcodeService->videoEncode,
                                       &transcodeService->transcodeSettings.videoEncodeSettings);
    }
    B_Mutex_Unlock(transcodeService->transcodeMutex);
    BDBG_MSG(("B_DVR_TranscodeService_SetSettings index %u>>>",transcodeService->index));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_GetStatus(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceStatus *pStatus)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(transcodeService);
    BSTD_UNUSED(pStatus);
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_InstallCallback(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_ServiceCallback registeredCallback,
    void * appContext)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_ASSERT((registeredCallback));
    BDBG_MSG(("B_DVR_TranscodeService_InstallCallback >>>"));
    B_Mutex_Lock(transcodeService->transcodeMutex);
    transcodeService->registeredCallback = registeredCallback;
    transcodeService->appContext = appContext;
    B_Mutex_Unlock(transcodeService->transcodeMutex);
    BDBG_MSG(("B_DVR_TranscodeService_InstallCallback <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_RemoveCallback(
    B_DVR_TranscodeServiceHandle transcodeService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_MSG(("B_DVR_TranscodeService_RemoveCallback >>>"));
    B_Mutex_Lock(transcodeService->transcodeMutex);
    transcodeService->registeredCallback=NULL;
    transcodeService->appContext = NULL;
    B_Mutex_Unlock(transcodeService->transcodeMutex);
    BDBG_MSG(("B_DVR_TranscodeService_RemoveCallback <<<"));
    return rc;;
    
}

B_DVR_ERROR B_DVR_TranscodeService_AddInputEsStream(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputEsStream *inputEsStream)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    #if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
    #endif
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_ASSERT(inputEsStream);
    BDBG_MSG(("B_DVR_TranscodeService_AddInputEsStream index % >>>",transcodeService->index));
    B_Mutex_Lock(transcodeService->transcodeMutex);
    #if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannel_GetSettings(transcodeService->syncChannel, &syncChannelSettings);
    #endif
    switch(inputEsStream->esStreamInfo.pidType)
    {
    case eB_DVR_PidTypeVideo:
        {
            BDBG_MSG(("video pid %u",inputEsStream->esStreamInfo.pid));
            #if NEXUS_HAS_SYNC_CHANNEL
            syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(inputEsStream->videoEncodeParams.videoDecoder);
            #endif
            rc = B_DVR_TranscodeService_P_AddInputVideoEsStream(transcodeService,inputEsStream);
            if(!rc)
            {
                #if NEXUS_HAS_SYNC_CHANNEL
                NEXUS_SyncChannel_SetSettings(transcodeService->syncChannel, &syncChannelSettings);
                #endif
                BKNI_Memcpy((void *)&transcodeService->inputVideoEsStream,(void *)inputEsStream,sizeof(*inputEsStream));
            }
        }
        break;
    case eB_DVR_PidTypeAudio:
        {
            unsigned index=transcodeService->audioMuxOutputCount;
            BDBG_MSG(("audio pid %u",inputEsStream->esStreamInfo.pid));
            #if NEXUS_HAS_SYNC_CHANNEL
            syncChannelSettings.audioInput[transcodeService->audioMuxOutputCount] = 
                NEXUS_AudioDecoder_GetConnector(inputEsStream->audioEncodeParams.audioDecoder,
                                               inputEsStream->audioEncodeParams.audioPassThrough? 
                                               NEXUS_AudioDecoderConnectorType_eCompressed:
                                               NEXUS_AudioDecoderConnectorType_eStereo);
            #endif
            rc = B_DVR_TranscodeService_P_AddInputAudioEsStream(transcodeService,inputEsStream);
            if(!rc)
            {
                #if NEXUS_HAS_SYNC_CHANNEL
                NEXUS_SyncChannel_SetSettings(transcodeService->syncChannel, &syncChannelSettings);
                #endif
                BKNI_Memcpy((void *)&transcodeService->inputAudioEsStream[index],(void *)inputEsStream,sizeof(*inputEsStream));

            }
        }
        break;
    case eB_DVR_PidTypeData:
        {
            NEXUS_PlaypumpOpenSettings playpumpSettings;
            transcodeService->streamMuxStartSettings.pcr.pid = inputEsStream->esStreamInfo.pid;
            NEXUS_Playpump_GetDefaultOpenSettings(&playpumpSettings);
            playpumpSettings.streamMuxCompatible = true;
            playpumpSettings.fifoSize = 16384;    /* reduce FIFO size allocated for playpump */
            playpumpSettings.numDescriptors = 64; /* set number of descriptors */
            transcodeService->pcrPlaypump = NEXUS_Playpump_Open(inputEsStream->playpumpIndex,&playpumpSettings);
            if(transcodeService->pcrPlaypump)
            {
                transcodeService->streamMuxStartSettings.pcr.pid = inputEsStream->esStreamInfo.pid;
                transcodeService->streamMuxStartSettings.pcr.interval = 50;
                transcodeService->streamMuxStartSettings.pcr.playpump = transcodeService->pcrPlaypump;
                transcodeService->streamMuxStartSettings.stcChannel = transcodeService->transcodeServiceRequest.transcodeStcChannel;
                transcodeService->pidChannelTranscodePcr = NEXUS_Playpump_OpenPidChannel(transcodeService->pcrPlaypump,
                                                                                         inputEsStream->esStreamInfo.pid,
                                                                                         NULL);
                if(!transcodeService->pidChannelTranscodePcr)
                {
                    BDBG_ERR(("pcr pid Channel open failed"));
                    rc = B_DVR_INVALID_PARAMETER;
                    NEXUS_Playpump_Close(transcodeService->pcrPlaypump);
                }
                BKNI_Memcpy((void *)&transcodeService->inputPcrEsStream,(void *)inputEsStream,sizeof(*inputEsStream)); 

                transcodeService->pidChannelTranscodePsi = NEXUS_Playpump_OpenPidChannel(transcodeService->pcrPlaypump,
                                                                                         B_DVR_DATAINJECTION_DUMMY_PID-transcodeService->index,
                                                                                         NULL);
                if(!transcodeService->pidChannelTranscodePsi)
                {
                    BDBG_ERR(("Unable to open the data injection PID channel for transcode output"));
                }
            }
            else
            {
                BDBG_ERR(("pcr playpump open failed"));
                rc = B_DVR_INVALID_PARAMETER;
            }
        }
        break;
    case eB_DVR_PidTypeMax:
        {
            BDBG_ERR(("Invalid input Stream"));
            rc = B_DVR_INVALID_PARAMETER;
        }
  
    }
    B_Mutex_Unlock(transcodeService->transcodeMutex);
    BDBG_MSG(("B_DVR_TranscodeService_AddInputEsStream index % <<<",transcodeService->index));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_RemoveAllInputEsStreams(
    B_DVR_TranscodeServiceHandle transcodeService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    int index=0;
    unsigned encodeCount;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_MSG(("B_DVR_TranscodeService_RemoveAllInputEsStreams index % >>>",transcodeService->index));
    B_Mutex_Lock(transcodeService->transcodeMutex);
    NEXUS_StreamMux_Stop(transcodeService->streamMux);
    
    if (transcodeService->videoEncodeCount)
    {
        rc = B_DVR_TranscodeService_P_RemoveInputVideoEsStream(transcodeService,&transcodeService->inputVideoEsStream);
    }
    BKNI_Memset((void *) &transcodeService->inputVideoEsStream,0,sizeof(B_DVR_TranscodeServiceInputEsStream));
    transcodeService->videoEncodeCount=0;

    encodeCount = transcodeService->audioMuxOutputCount;               
    if (encodeCount > 0)
    {
       for(index=encodeCount - 1;index>=0;index--)
       {
          rc = B_DVR_TranscodeService_P_RemoveInputAudioEsStream(transcodeService,&transcodeService->inputAudioEsStream[index]);
          BKNI_Memset((void *) &transcodeService->inputAudioEsStream[index],0,sizeof(B_DVR_TranscodeServiceInputEsStream));
       }
       transcodeService->audioEncodeCount = 0;
    }


    if (transcodeService->pcrPlaypump)
    {
      if(transcodeService->pidChannelTranscodePsi)
      {
           NEXUS_Playpump_ClosePidChannel(transcodeService->pcrPlaypump, transcodeService->pidChannelTranscodePsi);
           transcodeService->pidChannelTranscodePsi = NULL;
      }
      if (transcodeService->pidChannelTranscodePcr)
      {
          NEXUS_Playpump_ClosePidChannel(transcodeService->pcrPlaypump, transcodeService->pidChannelTranscodePcr);
          transcodeService->pidChannelTranscodePcr = NULL;
      }
      NEXUS_Playpump_Close(transcodeService->pcrPlaypump);
    }
    transcodeService->streamMuxStartSettings.pcr.pid = 0;;
    transcodeService->streamMuxStartSettings.pcr.interval = 0;
    transcodeService->streamMuxStartSettings.pcr.playpump = NULL;
    transcodeService->pcrPlaypump=NULL;

    B_Mutex_Unlock(transcodeService->transcodeMutex);
    BDBG_MSG(("B_DVR_TranscodeService_RemoveAllInputEsStreams index % <<<",transcodeService->index));
    return rc;
}

NEXUS_PidChannelHandle B_DVR_TranscodeService_GetPidChannel(
    B_DVR_TranscodeServiceHandle transcodeService,
    unsigned pid)
{
    NEXUS_PidChannelHandle pidChannel=NULL;
    unsigned index;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_MSG(("B_DVR_TranescodeService_GetPidChannel index %u >>>",transcodeService->index));
    B_Mutex_Lock(transcodeService->transcodeMutex);

    /* Look through video pidChannels */
    for(index=0;index<NEXUS_MAX_MUX_PIDS;index++)
    {
        BDBG_MSG(("transcodeService->streamMuxStartSettings.video[%u].pid %u",
                  index,transcodeService->streamMuxStartSettings.video[index].pid));
        if(pid == transcodeService->streamMuxStartSettings.video[index].pid) 
        {
            pidChannel = transcodeService->streamMuxOutput.video[index];
            break;
        }


    }

    if(pidChannel)
    {
        goto pidChannelFound;
    }
    /* Look through audio pidChannels */
    for(index=0;index<NEXUS_MAX_MUX_PIDS;index++)
    {
        BDBG_MSG(("transcodeService->streamMuxStartSettings.audio[%u].pid %u",
                  index,transcodeService->streamMuxStartSettings.audio[index].pid));
        if(pid == transcodeService->streamMuxStartSettings.audio[index].pid) 
        {
            pidChannel = transcodeService->streamMuxOutput.audio[index];
            break;
        }
        
    }
    if(pidChannel)
    {
        goto pidChannelFound;
    }

    if(pid == transcodeService->streamMuxStartSettings.pcr.pid)
    { 
        pidChannel = transcodeService->pidChannelTranscodePcr;
    }

pidChannelFound:   
    B_Mutex_Unlock(transcodeService->transcodeMutex);
    BDBG_MSG(("B_DVR_TranescodeService_GetPidChannel index %u >>>",transcodeService->index));
    return pidChannel;
}

NEXUS_PidChannelHandle B_DVR_TranscodeService_GetDataInjectionPidChannel(B_DVR_TranscodeServiceHandle transcodeService)
{
    NEXUS_PidChannelHandle dataInjectionPidChannel=NULL;
    BDBG_OBJECT_ASSERT(transcodeService,B_DVR_TranscodeService);
    BDBG_MSG(("B_DVR_TranscodeService_GetDataInjectionPidChannel index %u >>>",transcodeService->index));
    B_Mutex_Lock(transcodeService->transcodeMutex);
    dataInjectionPidChannel = transcodeService->pidChannelTranscodePsi;
    B_Mutex_Unlock(transcodeService->transcodeMutex);
    BDBG_MSG(("B_DVR_TranscodeService_GetDataInjectionPidChannel index %u <<<",transcodeService->index));
    return dataInjectionPidChannel;
}
#else
B_DVR_TranscodeServiceHandle B_DVR_TranscodeService_Open(
    B_DVR_TranscodeServiceRequest *transcodeServiceRequest)
{
    BSTD_UNUSED(transcodeServiceRequest);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return NULL;
}


B_DVR_ERROR B_DVR_TranscodeService_Close(
    B_DVR_TranscodeServiceHandle transcodeService)
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}


B_DVR_ERROR B_DVR_TranscodeService_Start(
    B_DVR_TranscodeServiceHandle transcodeService)
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_Stop(
    B_DVR_TranscodeServiceHandle transcodeService)
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_GetInputSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_SetInputSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}


B_DVR_ERROR B_DVR_TranscodeService_GetOutputSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceOutputSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_SetOutputSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceOutputSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_GetSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_SetSettings(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_GetStatus(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceStatus *pStatus)
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BSTD_UNUSED(pStatus);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_InstallCallback(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_ServiceCallback registeredCallback,
    void * appContext
    )
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BSTD_UNUSED(registeredCallback);
    BSTD_UNUSED(appContext);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_RemoveCallback(
    B_DVR_TranscodeServiceHandle transcodeService)
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_AddInputEsStream(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputEsStream *esStream)
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BSTD_UNUSED(esStream);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_RemoveInputEsStream(
    B_DVR_TranscodeServiceHandle transcodeService,
    B_DVR_TranscodeServiceInputEsStream *esStream)
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BSTD_UNUSED(esStream);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}

B_DVR_ERROR B_DVR_TranscodeService_RemoveAllInputEsStreams(
    B_DVR_TranscodeServiceHandle transcodeService)
{
    B_DVR_ERROR rc = B_DVR_NOT_SUPPORTED; 
    BSTD_UNUSED(transcodeService);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return rc;
}

NEXUS_PidChannelHandle B_DVR_TranscodeService_GetPidChannel(
    B_DVR_TranscodeServiceHandle transcodeService,
    unsigned pid)
{
    BSTD_UNUSED(transcodeService);
    BSTD_UNUSED(pid);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return NULL;
}

NEXUS_PidChannelHandle B_DVR_TranscodeService_GetDataInjectionPidChannel(
    B_DVR_TranscodeServiceHandle transcodeService)
{
    BSTD_UNUSED(transcodeService);
    BDBG_WRN(("transcodeService not enabled on this platform"));
    return NULL;
}
#endif
