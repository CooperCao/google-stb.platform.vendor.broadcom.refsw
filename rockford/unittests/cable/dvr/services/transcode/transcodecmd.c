/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description: command based transcode exercising program
 *  
 ****************************************************************************/


#include "b_dvr_test.h"

BDBG_MODULE(dvr_transcode);


#define MAX_AVPATH 1

static int currentChannel = 2;
char VideoCodecName[8], AudioCodecName[8];
B_EventHandle PlaybackEnd;

bool DataInjectionEnabled = 0;




void DvrTest_TuneChannel(int index)
{
    NEXUS_Error rc;
    int currentChannel = g_dvrTest->streamPath[index].currentChannel;
    printf("\n %s: index %d <<<",__FUNCTION__,index);

    NEXUS_Frontend_GetUserParameters(g_dvrTest->streamPath[index].frontend, &g_dvrTest->streamPath[index].userParams);
    printf("\n %s: Input band=%d ",__FUNCTION__,g_dvrTest->streamPath[index].userParams.param1);
    g_dvrTest->streamPath[index].parserBand = (NEXUS_ParserBand)g_dvrTest->streamPath[index].userParams.param1;
    NEXUS_ParserBand_GetSettings(g_dvrTest->streamPath[index].parserBand, &g_dvrTest->streamPath[index].parserBandSettings);
    g_dvrTest->streamPath[index].parserBandSettings.sourceTypeSettings.inputBand = (NEXUS_InputBand)g_dvrTest->streamPath[index].userParams.param1;
    g_dvrTest->streamPath[index].parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    g_dvrTest->streamPath[index].parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(g_dvrTest->streamPath[index].parserBand, &g_dvrTest->streamPath[index].parserBandSettings);

    NEXUS_Frontend_GetDefaultQamSettings(&g_dvrTest->streamPath[index].qamSettings);
    g_dvrTest->streamPath[index].qamSettings.frequency =  channelInfo[currentChannel].frequency;
    g_dvrTest->streamPath[index].qamSettings.mode = channelInfo[currentChannel].modulation;
    g_dvrTest->streamPath[index].qamSettings.annex = channelInfo[currentChannel].annex;
    g_dvrTest->streamPath[index].qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;

    rc = NEXUS_Frontend_TuneQam(g_dvrTest->streamPath[index].frontend, &g_dvrTest->streamPath[index].qamSettings);
    BDBG_ASSERT(!rc);
    BKNI_Sleep(2000);
    rc = NEXUS_Frontend_GetQamStatus(g_dvrTest->streamPath[index].frontend, &g_dvrTest->streamPath[index].qamStatus);
    BDBG_ASSERT(!rc);
    printf("\n %s: receiver lock = %d",__FUNCTION__,g_dvrTest->streamPath[index].qamStatus.receiverLock);
    printf("\n %s: Symbol rate = %d",__FUNCTION__,g_dvrTest->streamPath[index].qamStatus.symbolRate);

    printf("\n %s: index %d >>>",__FUNCTION__,index);
    return;
}

void DvrTest_LiveDecodeStart(int index)
{
    int currentChannel = g_dvrTest->streamPath[index].currentChannel;
    NEXUS_TimebaseSettings timebaseSettings;
    printf("\n %s:index %d >>>",__FUNCTION__,index);
    /* Open the pid channels */
    if(g_dvrTest->streamPath[index].tsbService)
    { 
        /* NEXUS_PidChannelSettings settings; */

        g_dvrTest->streamPath[index].videoPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                    channelInfo[currentChannel].videoPids[0], NULL);
    }
    else
    {
    g_dvrTest->streamPath[index].videoPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                   channelInfo[currentChannel].videoPids[0], NULL);
    }
    NEXUS_VideoDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].videoProgram);
    g_dvrTest->streamPath[index].videoProgram.codec = channelInfo[currentChannel].videoFormat[0];
    g_dvrTest->streamPath[index].videoProgram.pidChannel = g_dvrTest->streamPath[index].videoPidChannels[0];
    g_dvrTest->streamPath[index].videoProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;

    NEXUS_StcChannel_GetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);
    NEXUS_Timebase_GetSettings(g_dvrTest->streamPath[index].stcSettings.timebase, &timebaseSettings);
    timebaseSettings.freeze = false;
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
    timebaseSettings.sourceSettings.pcr.pidChannel = g_dvrTest->streamPath[index].videoProgram.pidChannel;
    timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
    timebaseSettings.sourceSettings.pcr.maxPcrError = 0xFF;
    NEXUS_Timebase_SetSettings(g_dvrTest->streamPath[index].stcSettings.timebase, &timebaseSettings);

    g_dvrTest->streamPath[index].stcSettings.autoConfigTimebase = true;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.offsetThreshold = 8;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.maxPcrError = 255;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.disableTimestampCorrection = false;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.disableJitterAdjustment = false;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.pidChannel = g_dvrTest->streamPath[index].videoProgram.pidChannel;
    g_dvrTest->streamPath[index].stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */

    NEXUS_StcChannel_SetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(g_dvrTest->streamPath[index].videoDecoder, &g_dvrTest->streamPath[index].videoProgram);
    if(!index)
    {

        if(g_dvrTest->streamPath[index].tsbService)
        {
            /* NEXUS_PidChannelSettings settings; */
            g_dvrTest->streamPath[index].audioPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                        channelInfo[currentChannel].audioPids[0], NULL);
        }
        else
        { 
            g_dvrTest->streamPath[index].audioPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                        channelInfo[currentChannel].audioPids[0], NULL);
        }
       NEXUS_AudioDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].audioProgram);
       g_dvrTest->streamPath[index].audioProgram.codec = channelInfo[currentChannel].audioFormat[0];
       g_dvrTest->streamPath[index].audioProgram.pidChannel = g_dvrTest->streamPath[index].audioPidChannels[0];
       g_dvrTest->streamPath[index].audioProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;
       NEXUS_AudioDecoder_Start(g_dvrTest->streamPath[index].audioDecoder,&g_dvrTest->streamPath[index].audioProgram);

    }
    g_dvrTest->streamPath[index].liveDecodeStarted=true;
    
    printf("\n %s:index %d <<<",__FUNCTION__,index);
    return;
}


void DvrTest_LiveDecodeStop(int index)
{
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    printf("\n %s: index %d >>>",__FUNCTION__,index);
    NEXUS_VideoDecoder_GetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    NEXUS_VideoDecoder_SetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    if(!index)
    {
        NEXUS_AudioDecoder_Stop(g_dvrTest->streamPath[index].audioDecoder);
        NEXUS_PidChannel_Close( g_dvrTest->streamPath[index].audioPidChannels[0]);
    }
    NEXUS_VideoDecoder_Stop(g_dvrTest->streamPath[index].videoDecoder);
    NEXUS_PidChannel_Close( g_dvrTest->streamPath[index].videoPidChannels[0]);
    g_dvrTest->streamPath[index].liveDecodeStarted = false;
    printf("\n %s: index %d <<<",__FUNCTION__,index);
    return;
}

void DvrTest_PlaybackDecodeStart(int index)
{
    B_DVR_PlaybackServiceSettings playbackServiceSettings;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    B_DVR_Media media;
    B_DVR_ERROR rc;
    printf("\n %s:index %d >>>",__FUNCTION__,index);

    if(g_dvrTest->streamPath[index].tsbService || g_dvrTest->streamPath[index].playbackService)
    {
        if(g_dvrTest->streamPath[index].tsbService)
        { 
            mediaNodeSettings.recordingType = eB_DVR_RecordingTSB;
            mediaNodeSettings.subDir = (char *)&g_dvrTest->streamPath[index].tsbServiceRequest.subDir[0];
            mediaNodeSettings.programName = (char *)&g_dvrTest->streamPath[index].tsbServiceRequest.programName[0];
            mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[index].tsbServiceRequest.volumeIndex;

            BSTD_UNUSED(playbackServiceSettings);
            B_DVR_TSBService_GetSettings(g_dvrTest->streamPath[index].tsbService,&tsbServiceSettings);
            g_dvrTest->streamPath[index].playback = tsbServiceSettings.tsbPlaybackSettings.playback;
            tsbServiceSettings.tsbPlaybackSettings.videoDecoder[0]=g_dvrTest->streamPath[index].videoDecoder;
            tsbServiceSettings.tsbPlaybackSettings.videoDecoder[1]=NULL;
            tsbServiceSettings.tsbPlaybackSettings.audioDecoder[0]=g_dvrTest->streamPath[index].audioDecoder;
            tsbServiceSettings.tsbPlaybackSettings.audioDecoder[1]=NULL;
            tsbServiceSettings.tsbPlaybackSettings.stcChannel =g_dvrTest->streamPath[index].stcChannel;
            B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[index].tsbService,&tsbServiceSettings);
            
            
        }
        else
        {
            mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
            mediaNodeSettings.subDir = (char *)&g_dvrTest->streamPath[index].playbackServiceRequest.subDir[0];
            mediaNodeSettings.programName = (char *)&g_dvrTest->streamPath[index].playbackServiceRequest.programName[0];
            mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[index].playbackServiceRequest.volumeIndex;
 
            BSTD_UNUSED(tsbServiceSettings);
            B_DVR_PlaybackService_GetSettings(g_dvrTest->streamPath[index].playbackService,&playbackServiceSettings);
            g_dvrTest->streamPath[index].playback = playbackServiceSettings.playback;
            playbackServiceSettings.videoDecoder[0]=g_dvrTest->streamPath[index].videoDecoder;
            playbackServiceSettings.videoDecoder[1]=NULL;
            playbackServiceSettings.audioDecoder[0]=g_dvrTest->streamPath[index].audioDecoder;
            playbackServiceSettings.audioDecoder[1]=NULL;
            playbackServiceSettings.stcChannel =g_dvrTest->streamPath[index].stcChannel;
            B_DVR_PlaybackService_SetSettings(g_dvrTest->streamPath[index].playbackService,&playbackServiceSettings);
        }

        rc = B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n media node not found");
            assert(!rc);
        }
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = media.esStreamInfo[0].codec.videoCodec;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.decoder = g_dvrTest->streamPath[index].videoDecoder;
        g_dvrTest->streamPath[index].videoPlaybackPidChannels[0] =
             NEXUS_Playback_OpenPidChannel(g_dvrTest->streamPath[index].playback,media.esStreamInfo[0].pid,&playbackPidSettings);

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.primary = g_dvrTest->streamPath[index].audioDecoder;
        g_dvrTest->streamPath[index].audioPlaybackPidChannels[0] = 
            NEXUS_Playback_OpenPidChannel(g_dvrTest->streamPath[index].playback,media.esStreamInfo[1].pid,&playbackPidSettings);

        NEXUS_VideoDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].videoProgram);
        g_dvrTest->streamPath[index].videoProgram.codec = media.esStreamInfo[0].codec.videoCodec;
        g_dvrTest->streamPath[index].videoProgram.pidChannel = g_dvrTest->streamPath[index].videoPlaybackPidChannels[0];
        g_dvrTest->streamPath[index].videoProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;
        NEXUS_StcChannel_GetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);
        g_dvrTest->streamPath[index].stcSettings.mode = NEXUS_StcChannelMode_eAuto; 
        NEXUS_StcChannel_SetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);
        NEXUS_VideoDecoder_Start(g_dvrTest->streamPath[index].videoDecoder, &g_dvrTest->streamPath[index].videoProgram);

        if(!index)
        {
            NEXUS_AudioDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].audioProgram);
            g_dvrTest->streamPath[index].audioProgram.codec = media.esStreamInfo[1].codec.audioCodec;
            g_dvrTest->streamPath[index].audioProgram.pidChannel = g_dvrTest->streamPath[index].audioPlaybackPidChannels[0];
            g_dvrTest->streamPath[index].audioProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;
            NEXUS_AudioDecoder_Start(g_dvrTest->streamPath[index].audioDecoder,&g_dvrTest->streamPath[index].audioProgram);
        }
        g_dvrTest->streamPath[index].playbackDecodeStarted=true;

    }
    else
    {
        printf("\n error!!!!!neither playback or tsbservice opened");
    }
    printf("\n %s:index %d <<<",__FUNCTION__,index);

}


void DvrTest_PlaybackDecodeStop(int index)
{
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    printf("\n %s: index %d >>>",__FUNCTION__,index);
    NEXUS_VideoDecoder_GetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    NEXUS_VideoDecoder_SetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    if(!index)
    {
        NEXUS_AudioDecoder_Stop(g_dvrTest->streamPath[index].audioDecoder);
    }
    NEXUS_VideoDecoder_Stop(g_dvrTest->streamPath[index].videoDecoder);
    NEXUS_Playback_ClosePidChannel(g_dvrTest->streamPath[index].playback,g_dvrTest->streamPath[index].audioPlaybackPidChannels[0]);
    NEXUS_Playback_ClosePidChannel(g_dvrTest->streamPath[index].playback,g_dvrTest->streamPath[index].videoPlaybackPidChannels[0]);
    g_dvrTest->streamPath[index].playbackDecodeStarted = false;
    printf("\n %s: index %d <<<",__FUNCTION__,index);
    return;
}

void DvrTest_AudioVideoPathOpen(int index)
{
     printf("\n %s:  index %d >>> ",__FUNCTION__,index);
    /* By default, StcChannel will configure NEXUS_Timebase with the info it has */
    NEXUS_StcChannel_GetDefaultSettings(index, &g_dvrTest->streamPath[index].stcSettings);
    g_dvrTest->streamPath[index].stcSettings.timebase = index?NEXUS_Timebase_e1:NEXUS_Timebase_e0;

    g_dvrTest->streamPath[index].stcChannel = NEXUS_StcChannel_Open(index, &g_dvrTest->streamPath[index].stcSettings);
    g_dvrTest->streamPath[index].videoProgram.stcChannel =  g_dvrTest->streamPath[index].stcChannel;
    g_dvrTest->streamPath[index].audioProgram.stcChannel =  g_dvrTest->streamPath[index].stcChannel;
    g_dvrTest->streamPath[index].window = NEXUS_VideoWindow_Open(g_dvrTest->display,index);
    g_dvrTest->streamPath[index].videoDecoder = NEXUS_VideoDecoder_Open(index, NULL);
    NEXUS_VideoWindow_AddInput(g_dvrTest->streamPath[index].window,
                               NEXUS_VideoDecoder_GetConnector(g_dvrTest->streamPath[index].videoDecoder));
    if(!index)
    {
        g_dvrTest->streamPath[index].audioDecoder = NEXUS_AudioDecoder_Open(index, NULL);
        NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]),
                                   NEXUS_AudioDecoder_GetConnector(g_dvrTest->streamPath[index].audioDecoder,
                                    NEXUS_AudioDecoderConnectorType_eStereo));
    }
    else
    {
        g_dvrTest->streamPath[index].audioDecoder = NULL;
    }
    
    printf("\n %s:  index %d <<<",__FUNCTION__,index);
    return;
}


void DvrTest_AudioVideoPathClose(int index)
{
    printf("\n %s:index %d >>>",__FUNCTION__,index);
    if(!index)
    {
        NEXUS_AudioOutput_Shutdown(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
        NEXUS_AudioDecoder_Close(g_dvrTest->streamPath[index].audioDecoder);
    }
    NEXUS_VideoOutput_Shutdown(NEXUS_HdmiOutput_GetVideoConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
    NEXUS_VideoWindow_RemoveAllInputs(g_dvrTest->streamPath[index].window);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(g_dvrTest->streamPath[index].videoDecoder));
    NEXUS_VideoWindow_Close(g_dvrTest->streamPath[index].window);
    NEXUS_VideoDecoder_Close(g_dvrTest->streamPath[index].videoDecoder);
    printf("\n %s: index %d <<<",__FUNCTION__,index);
    return;
}



static void DvrTest_Callback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service)
{
    struct DvrTest_StreamPath *streamPath = (struct DvrTest_StreamPath *)appContext;
    char *eventString[eB_DVR_EventMax+1] = {"eB_DVR_EventStartOfRecording","eB_DVR_EventEndOfRecording", "eB_DVR_EventHDStreamRecording",
 "eB_DVR_EventSDStreamRecording","eB_DVR_EventTSBConverstionCompleted","eB_DVR_EventOverFlow","eB_DVR_EventUnderFlow",
 "eB_DVR_EventStartOfPlayback", "eB_DVR_EventEndOfPlayback","eB_DVR_EventPlaybackAlarm","eB_DVR_EventAbortPlayback",
 "eB_DVR_EventAbortRecord","eB_DVR_EventAbortTSBRecord","eB_DVR_EventAbortTSBPlayback","eB_DVR_EventDataInjectionCompleted",
 "eB_DVR_EventTranscodeFinish","eB_DVR_EventMediaProbed","eB_DVR_EventOutOfMediaStorage","eB_DVR_EventOutOfNavigationStorage",
 "eB_DVR_EventOutOfMetaDataStorage","eB_DVR_EventFormatSuccess","eB_DVR_EventFormatFail","eB_DVR_EventFormatFail_InvalidIndex",
 "eB_DVR_EventFormatFail_NotRegistered",  "eB_DVR_EventFormatFail_VolumeMounted", "eB_DVR_EventFormatFail_SystemErr", 
 "eB_DVR_EventMountSuccss","eB_DVR_EventMountFail", "eB_DVR_EventMountFail_InvalidIndex", "eB_DVR_EventMountFail_NotRegistered",
 "eB_DVR_EventMountFail_VolumeMounted","eB_DVR_EventMountFail_NotFormatted","eB_DVR_EventMountFail_SystemErr",
 "eB_DVR_EventMountFail_WrongLabel","eB_DVR_EventMax"};
    char *serviceString[eB_DVR_ServiceMax+1]= {"tsbService","playbackService","recordService","mediaUtil","drmService",
                                               "storagaService","dataInjectionService","transcodeService","invalidService"};
    printf("\n DvrTest_Callback >>>> index %u event %s service %s",(unsigned)index,eventString[event],serviceString[service]);
    switch(event)
    {
    case eB_DVR_EventStartOfRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
                printf("\n Beginning Of TSB");
            }
            else
            {
                printf("\n Beginning of background recording");

            }
        }
        break;
    case eB_DVR_EventEndOfRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
                printf("\n End Of TSB Conversion");
            }
            else
            {
                printf("\n End of background recording");

            }
        }
        break;
    case eB_DVR_EventHDStreamRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
                printf("\n HD Stream recording in TSB Service");
            }
            else
            {
                printf("\n HD Stream recording in Record Service");
            }

        }
        break;
    case eB_DVR_EventSDStreamRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
                printf("\n SD Stream recording in TSB Service");
            }
            else
            {
                printf("\n SD Stream recording in Record Service");
            }
        }
        break;
    case eB_DVR_EventTSBConverstionCompleted:
        {
            printf("\n TSB Conversion completed");
            if(streamPath->tsbConvEvent)
            B_Event_Set(streamPath->tsbConvEvent);
        }
        break;
    case eB_DVR_EventOverFlow:
        {
            printf("\n OverFlow");
        }
        break;
    case eB_DVR_EventUnderFlow:
        {
            printf("\nunderFlow");
        }
        break;
    case eB_DVR_EventEndOfPlayback:
        {
            if(streamPath->playbackService)
            { 
                B_DVR_PlaybackServiceStatus status;
                B_DVR_PlaybackService_GetStatus(streamPath->playbackService,&status);
                printf("\n Playback Status:");
                printf("\n Time -> start:%ld current:%ld end:%ld",status.startTime,status.currentTime,status.endTime);
                printf("\n Offset -> start:%lld current:%lld end:%lld",status.linearStartOffset,status.linearCurrentOffset,status.linearEndOffset);
            }
            printf("\n End of playback stream. Pausing");
            printf("\n Stop either TSBPlayback or Playback to enter live mode");

            B_Event_Set(PlaybackEnd);

        }
        break;
    case eB_DVR_EventStartOfPlayback:
        {
            if(service == eB_DVR_ServicePlayback)
            {
              printf("\n Beginnning of playback stream. Pausing");
            }
            else
            {
                printf("\n Beginning of TSB. Pausing");
            }
        }
        break;
    case eB_DVR_EventPlaybackAlarm:
        {
            printf("\n playback alarm event");
        }
        break;
    case eB_DVR_EventAbortPlayback:
        {
            printf("\n abort playback");
        }
        break;
    case eB_DVR_EventAbortRecord:
        {
            printf("\n abort record");
        }
        break;
    case eB_DVR_EventAbortTSBRecord:
        {
            printf("\n abort TSB record");
        }
        break;
    case eB_DVR_EventAbortTSBPlayback:
        {
            printf("\n abort TSB Playback");
        }
        break;
    case eB_DVR_EventDataInjectionCompleted:
        {
            uint8_t ccByte;
            if(streamPath->recordService && streamPath->dataInjectionStarted)
            {
                printf("\n data Injection complete for record service %d",index);
                B_DVR_RecordService_InjectDataStop(streamPath->recordService);
                BKNI_Sleep(100);
                ++streamPath->dataInsertionCount;
                if(streamPath->dataInsertionCount%2 == 0)
                {
                    ++streamPath->patCcValue;
                    ccByte = streamPath->pat[4]; /* the 1st byte of pat/pmt arrays is for TSheader builder use */
                    ccByte = (ccByte & 0xf0) | (streamPath->patCcValue & 0xf);
                    streamPath->pat[4] = ccByte;
                    B_DVR_RecordService_InjectDataStart(streamPath->recordService,((void *)&streamPath->pat[1]),188);
                }
                else
                {
                    ++streamPath->pmtCcValue;
                    ccByte = streamPath->pmt[4]; /* the 1st byte of pat/pmt arrays is for TSheader builder use */
                    ccByte = (ccByte & 0xf0) | (streamPath->pmtCcValue & 0xf);
                    streamPath->pmt[4] = ccByte;
                    B_DVR_RecordService_InjectDataStart(streamPath->recordService,((void *)&streamPath->pmt[1]),188);
                }
           }
           if(streamPath->tsbService && streamPath->dataInjectionStarted)
           {
               printf("\n data Injection complete for TSB service %d",index);
               B_DVR_TSBService_InjectDataStop(streamPath->tsbService);
               BKNI_Sleep(100);
               ++streamPath->dataInsertionCount;
               if(streamPath->dataInsertionCount%2 == 0)
               {
                    printf("\n injecting pat");
                    ++streamPath->patCcValue;
                    ccByte = streamPath->pat[4]; /* the 1st byte of pat/pmt arrays is for TSheader builder use */
                    ccByte = (ccByte & 0xf0) | (streamPath->patCcValue & 0xf);
                    streamPath->pat[4] = ccByte;
                    B_DVR_TSBService_InjectDataStart(streamPath->tsbService,((void *)&streamPath->pat[1]),188);
               }
               else
               {
                    printf("\n injecting pmt");
                    ++streamPath->pmtCcValue;
                    ccByte = streamPath->pmt[4]; /* the 1st byte of pat/pmt arrays is for TSheader builder use */
                    ccByte = (ccByte & 0xf0) | (streamPath->pmtCcValue & 0xf);
                    streamPath->pmt[4] = ccByte;
                    B_DVR_TSBService_InjectDataStart(streamPath->tsbService,((void *)&streamPath->pmt[1]),188);
                }
           }
                     
        }
        
        break;
    case eB_DVR_EventOutOfMediaStorage:
        {
            printf("\n no media Storage space");
        }
        break;
    case eB_DVR_EventOutOfNavigationStorage:
        {
            printf("\n no navigation Storage space");
        }
        break;
    case eB_DVR_EventOutOfMetaDataStorage:
        {
            printf("\n no metaData Storage space");
        }
        break;
    case eB_DVR_EventTranscodeFinish:
        {
            printf("\n Transcode finish");
            BKNI_SetEvent(g_dvrTest->streamPath[index].transcodeFinishEvent);
        }
        break;
    case eB_DVR_EventMediaProbed:
        {
            if(streamPath->tsbService)
            {
                printf("\n TSB Service media probe complete");
            }
            if(streamPath->recordService)
            {
                printf("\n record service media probe complete");
            }
        }
        break;
    default:
        {
            printf("\n invalid event");
        }
   }
    printf("\n DvrTest_Callback <<<<<");
    return;
}



static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi = pParam;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;

    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    printf( "\n HDMI hotplug event: %s", status.connected?"connected":"not connected");
    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( status.connected )
    {
        NEXUS_DisplaySettings displaySettings;
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !status.videoFormatSupported[displaySettings.format] )
        {
            fprintf(stderr, "\nCurrent format not supported by attached monitor. Switching to preferred format %d\n", status.preferredVideoFormat);
            displaySettings.format = status.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
}


void B_DVR_TSB_Buffer_Preallocate(int numAVPath)
{
    int index;
    
    for(index=0;index<numAVPath;index++)
    {
        B_DVR_ERROR rc =0;
        char programName[B_DVR_MAX_FILE_NAME_LENGTH];
        char subDir[B_DVR_MAX_FILE_NAME_LENGTH]="tsb";
        B_DVR_MediaNodeSettings mediaNodeSettings;
        BKNI_Memset((void *)&mediaNodeSettings,0,sizeof(mediaNodeSettings));
        mediaNodeSettings.programName = programName;
        mediaNodeSettings.subDir = subDir;
        mediaNodeSettings.volumeIndex=0;
        mediaNodeSettings.recordingType = eB_DVR_RecordingTSB;
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",TSB_SERVICE_PREFIX,index);
        rc = B_DVR_Manager_AllocSegmentedFileRecord(g_dvrTest->dvrManager,
                                                    &mediaNodeSettings,
                                                    MAX_TSB_BUFFERS);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in allocating the tsb segments");
        }
    }

    for(index=0;index<numAVPath;index++)
    {
        B_DVR_ERROR rc =0;
        char programName[B_DVR_MAX_FILE_NAME_LENGTH];
        char subDir[B_DVR_MAX_FILE_NAME_LENGTH]="tsb";
        B_DVR_MediaNodeSettings mediaNodeSettings;
        BKNI_Memset((void *)&mediaNodeSettings,0,sizeof(mediaNodeSettings));
        mediaNodeSettings.programName = programName;
        mediaNodeSettings.subDir = subDir;
        mediaNodeSettings.volumeIndex=0;
        mediaNodeSettings.recordingType = eB_DVR_RecordingTSB;
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",TSB_SERVICE_PREFIX,index);
        rc = B_DVR_Manager_PrintSegmentedFileRecord(g_dvrTest->dvrManager,
                                                    &mediaNodeSettings);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in printing the tsb segments");
        }
    }
}

void B_DVR_TSB_Buffer_Deallocate(int numAVPath)
{
    int index;

    for(index=0;index<numAVPath;index++)
    {
        B_DVR_ERROR rc =0;
        char programName[B_DVR_MAX_FILE_NAME_LENGTH];
        char subDir[B_DVR_MAX_FILE_NAME_LENGTH]="tsb";
        B_DVR_MediaNodeSettings mediaNodeSettings;
        BKNI_Memset((void *)&mediaNodeSettings,0,sizeof(mediaNodeSettings));
        mediaNodeSettings.programName = programName;
        mediaNodeSettings.subDir = subDir;
        mediaNodeSettings.volumeIndex=0;
        mediaNodeSettings.recordingType = eB_DVR_RecordingTSB;
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",TSB_SERVICE_PREFIX,index);
        rc = B_DVR_Manager_FreeSegmentedFileRecord(g_dvrTest->dvrManager,
                                                   &mediaNodeSettings);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in freeing the tsb segments");
        }
    }
}



void DvrTest_Create_PatPmt(
    int pathIndex,
    uint16_t pcrPid,
    uint16_t vidPid,
    uint16_t audPid,
    NEXUS_VideoCodec videoCodec,
    NEXUS_AudioCodec audioCodec
)
{
    uint8_t pat_pl_buf[BTST_TS_HEADER_BUF_LENGTH], pmt_pl_buf[BTST_TS_HEADER_BUF_LENGTH];
    size_t pat_pl_size, pmt_pl_size;
    size_t buf_used = 0;
    size_t payload_pked = 0;
    unsigned streamNum;
    uint8_t  vidStreamType=0x2;
    uint8_t  audStreamType=0x4;
    TS_PAT_state patState;
    TS_PSI_header psi_header;
    TS_PMT_state pmtState;
    TS_PAT_program program;
    TS_PMT_stream pmt_stream;
    TS_PID_info pidInfo;
    TS_PID_state pidState;
    unsigned index=0;

    switch(videoCodec)
    {
        case NEXUS_VideoCodec_eMpeg2:         vidStreamType = 0x2; break;
        case NEXUS_VideoCodec_eMpeg4Part2:    vidStreamType = 0x10; break;
        case NEXUS_VideoCodec_eH264:          vidStreamType = 0x1b; break;
        case NEXUS_VideoCodec_eVc1SimpleMain: vidStreamType = 0xea; break;
        default:
            printf("Video encoder codec %d is not supported!\n",videoCodec);
    }
    switch(audioCodec)
    {
            case NEXUS_AudioCodec_eMpeg:         audStreamType = 0x4; break;
            case NEXUS_AudioCodec_eMp3:          audStreamType = 0x4; break;
            case NEXUS_AudioCodec_eAacAdts:      audStreamType = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacPlusAdts:  audStreamType = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacLoas:      audStreamType = 0x11; break;/* LOAS */
            case NEXUS_AudioCodec_eAacPlusLoas:  audStreamType = 0x11; break;/* LOAS */
            case NEXUS_AudioCodec_eAc3:          audStreamType = 0x81; break;
            default:
                printf("Audio encoder codec %d is not supported!\n", audioCodec);
    }
    /* == CREATE PSI TABLES == */
    TS_PSI_header_Init(&psi_header);
    TS_PAT_Init(&patState, &psi_header, pat_pl_buf, BTST_TS_HEADER_BUF_LENGTH);

    TS_PAT_program_Init(&program, 1, BTST_MUX_PMT_PID);
    TS_PAT_addProgram(&patState, &pmtState, &program, pmt_pl_buf, BTST_TS_HEADER_BUF_LENGTH);

    TS_PMT_setPcrPid(&pmtState, pcrPid);

    TS_PMT_stream_Init(&pmt_stream, vidStreamType, vidPid);
    TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);

    if(audPid)
    {
        TS_PMT_stream_Init(&pmt_stream, audStreamType, audPid);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    TS_PAT_finalize(&patState, &pat_pl_size);
    TS_PMT_finalize(&pmtState, &pmt_pl_size);

    /* == CREATE TS HEADERS FOR PSI INFORMATION == */
    TS_PID_info_Init(&pidInfo, 0x0, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState,g_dvrTest->streamPath[pathIndex].pat, BTST_TS_HEADER_BUF_LENGTH, &buf_used, patState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)g_dvrTest->streamPath[pathIndex].pat + buf_used, pat_pl_buf, pat_pl_size);
    printf("pmt_pl_size %u buf_used %u",pat_pl_size, buf_used);
    TS_PID_info_Init(&pidInfo, BTST_MUX_PMT_PID, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState,g_dvrTest->streamPath[pathIndex].pmt, BTST_TS_HEADER_BUF_LENGTH, &buf_used, pmtState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)g_dvrTest->streamPath[pathIndex].pmt + buf_used, pmt_pl_buf, pmt_pl_size);
    printf("pmt_pl_size %u buf_used %u",pmt_pl_size,buf_used);
    printf("\n \n PAT Dump\n[");
    for (index=0;index<BTST_TS_HEADER_BUF_LENGTH;index++) 
    {
        printf("%02x ",g_dvrTest->streamPath[pathIndex].pat[index+1]);
        if ((index % 16) == 15)
        {
            printf("]\n[");
        }
    }
    printf("\n\n");
    BKNI_Printf("PMT Dump\n[");
    for (index=0;index<BTST_TS_HEADER_BUF_LENGTH;index++) 
    {
        printf("%02x ",g_dvrTest->streamPath[pathIndex].pmt[index+1]);
        if ((index % 16) == 15)
        {
            printf("]\n[");
        }
    }
    printf("\n\n");
    return;
}



B_DVR_ERROR Dvr_DataInject_Start(unsigned pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;

    B_DVR_DataInjectionServiceSettings settings;
     g_dvrTest->streamPath[pathIndex].dataInsertionCount =0;
     g_dvrTest->streamPath[pathIndex].patCcValue=0;
     g_dvrTest->streamPath[pathIndex].pmtCcValue =0;
    
     B_DVR_DataInjectionService_InstallCallback(g_dvrTest->streamPath[pathIndex].dataInjectionService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
     settings.dataInjectionServiceType = eB_DVR_DataInjectionServiceTypeRawData;
     settings.pid = 0x0; /*unused*/
     B_DVR_DataInjectionService_SetSettings(
         g_dvrTest->streamPath[pathIndex].dataInjectionService,
         &settings);
     if(g_dvrTest->streamPath[pathIndex].tsbService)
     {
         B_DVR_TSBService_InjectDataStart(g_dvrTest->streamPath[pathIndex].tsbService,(void *)&g_dvrTest->streamPath[pathIndex].pat[1],188);
     }
     else
     {
         if(g_dvrTest->streamPath[pathIndex].recordService)
         {
             B_DVR_RecordService_InjectDataStart(g_dvrTest->streamPath[pathIndex].recordService,(void *)&g_dvrTest->streamPath[pathIndex].pat[1],188);
         }
         else
         {
             printf("\n neither record or tsb is active on path %d",pathIndex);
         }
     }
     g_dvrTest->streamPath[pathIndex].dataInjectionStarted=true;

     return rc;
}


B_DVR_ERROR Dvr_DataInject_Stop(unsigned pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;

    g_dvrTest->streamPath[pathIndex].dataInjectionStarted=false;
    if(g_dvrTest->streamPath[pathIndex].tsbService)
    {
        B_DVR_TSBService_InjectDataStop(g_dvrTest->streamPath[pathIndex].tsbService);
    }
    else
    {
        if(g_dvrTest->streamPath[pathIndex].recordService)
        {
            B_DVR_RecordService_InjectDataStop(g_dvrTest->streamPath[pathIndex].recordService);
        }
        else
        {
            printf("\n neither record or tsb is active on path %d",pathIndex);
        }
    }
    g_dvrTest->streamPath[pathIndex].dataInsertionCount =0;
    g_dvrTest->streamPath[pathIndex].patCcValue=0;
    g_dvrTest->streamPath[pathIndex].pmtCcValue =0;

    return rc;
}



B_DVR_ERROR Dvr_FileToFile_TranscodeStart(unsigned pathIndex, unsigned VideoCodec, 
unsigned AudioCodec, char *subDir, char *programName)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_Media playbackMedia;
    B_DVR_RecordServiceSettings recordServiceSettings;
    NEXUS_StcChannelSettings stcSettings;
    B_DVR_TranscodeServiceInputEsStream transcodeInputEsStream;
    B_DVR_TranscodeServiceInputSettings transcodeInputSettings;
    B_DVR_RecordServiceInputEsStream recordInputEsStream;
    NEXUS_PidChannelHandle transcodePidChannel;
    B_DVR_TranscodeServiceSettings transcodeServiceSettings;
    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : DvrTest_LiveDecodeStop >>>"));
    /* stop live decode*/
    printf("Dvr_FileToFile_TranscodeStart: Stop Live Decode\n");
    if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
       DvrTest_LiveDecodeStop(pathIndex);
    }

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : DvrTest_LiveDecodeStop <<<"));
    BKNI_Memset(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].playbackServiceRequest));


    sprintf(g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir, subDir);
    sprintf(g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName, programName);

    g_dvrTest->streamPath[pathIndex].playbackServiceRequest.volumeIndex = 0;
    g_dvrTest->streamPath[pathIndex].playbackServiceRequest.playpumpIndex = pathIndex;


    /* open playback service */
    printf("Dvr_FileToFile_TranscodeStart: Open Playback Service\n");
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_Open >>>"));
    g_dvrTest->streamPath[pathIndex].playbackService = B_DVR_PlaybackService_Open(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest);
    if(!g_dvrTest->streamPath[pathIndex].playbackService)
    {
        printf("\n playback service open failed for program %s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName);
        goto error_playbackService;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_Open <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_InstallCallback <<<"));
    B_DVR_PlaybackService_InstallCallback(g_dvrTest->streamPath[pathIndex].playbackService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_InstallCallback <<<"));
    BKNI_Memset((void*)&g_dvrTest->streamPath[pathIndex].transcodeServiceRequest,0,sizeof(B_DVR_TranscodeServiceRequest));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : NEXUS_StcChannel_Open <<<"));
    NEXUS_StcChannel_GetDefaultSettings(pathIndex?TRANSCODE1_STC_CHANNEL_INDEX:TRANSCODE0_STC_CHANNEL_INDEX, &stcSettings);
    stcSettings.timebase = pathIndex?NEXUS_Timebase_e1: NEXUS_Timebase_e0;/* should be the same timebase for end-to-end locking */
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */

    /* open transcode stc channel */
    printf("Dvr_FileToFile_TranscodeStart: Open Transcode STC Channel\n");
    g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeStcChannel = NEXUS_StcChannel_Open(pathIndex?TRANSCODE1_STC_CHANNEL_INDEX:TRANSCODE0_STC_CHANNEL_INDEX, &stcSettings);
    if(!g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeStcChannel)
    {
        printf("trancodeStcChannel open failed");
        goto error_transcodeStcChannel;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : NEXUS_StcChannel_Open <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_Open >>>"));
    g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.displayIndex = 3;
    g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeType = eB_DVR_TranscodeServiceType_NonRealTime;

    /* open transcode service */
    printf("Dvr_FileToFile_TranscodeStart: Open Transcode Service\n");
    g_dvrTest->streamPath[pathIndex].transcodeService = B_DVR_TranscodeService_Open(&g_dvrTest->streamPath[pathIndex].transcodeServiceRequest);
    if(!g_dvrTest->streamPath[pathIndex].transcodeService)
    {
        printf("\n transcode service open failed");
        goto error_transcodeService;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_Open <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_InstallCallback >>>"));
    B_DVR_TranscodeService_InstallCallback(g_dvrTest->streamPath[pathIndex].transcodeService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_InstallCallback <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetInputSettings >>>"));
    B_DVR_TranscodeService_GetInputSettings(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputSettings);
    transcodeInputSettings.input = eB_DVR_TranscodeServiceInput_File;
    transcodeInputSettings.playbackService =g_dvrTest->streamPath[pathIndex].playbackService;
    transcodeInputSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
    transcodeInputSettings.hdmiInput = NULL;
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetInputSettings <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetInputSettings >>>"));
    B_DVR_TranscodeService_SetInputSettings(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputSettings);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetInputSettings <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_GetMediaNode >>>"));
    B_DVR_PlaybackService_GetMediaNode(g_dvrTest->streamPath[pathIndex].playbackService,&playbackMedia);
    BDBG_MSG(("\n playback vPid %u aPid %u",playbackMedia.esStreamInfo[0].pid,playbackMedia.esStreamInfo[1].pid));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_GetMediaNode <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream vPid>>>"));
    BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
    BKNI_Memcpy((void *)&transcodeInputEsStream.esStreamInfo,&playbackMedia.esStreamInfo[0],sizeof(playbackMedia.esStreamInfo[0]));
    transcodeInputEsStream.stcChannel = g_dvrTest->streamPath[pathIndex].stcChannel;
    transcodeInputEsStream.playpumpIndex = NEXUS_ANY_ID;
    transcodeInputEsStream.videoEncodeParams.codec = VideoCodec;
    transcodeInputEsStream.videoEncodeParams.interlaced = false;
    transcodeInputEsStream.videoEncodeParams.profile = NEXUS_VideoCodecProfile_eBaseline;
    transcodeInputEsStream.videoEncodeParams.videoDecoder = g_dvrTest->streamPath[pathIndex].videoDecoder;
    transcodeInputEsStream.videoEncodeParams.level = NEXUS_VideoCodecLevel_e31;

    /* add video ES stream*/
    printf("Dvr_FileToFile_TranscodeStart: Add Video ES stream\n");
    B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream vPid <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream aPid >>>"));
    BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
    transcodeInputEsStream.audioEncodeParams.audioDecoder = g_dvrTest->streamPath[pathIndex].audioDecoder;
    transcodeInputEsStream.audioEncodeParams.audioPassThrough = false;
    transcodeInputEsStream.audioEncodeParams.codec = AudioCodec;
    transcodeInputEsStream.playpumpIndex = NEXUS_ANY_ID;
    if(g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)
    {
        transcodeInputEsStream.stcChannel = g_dvrTest->streamPath[pathIndex].stcAudioChannel;
    }
    else
    {
        transcodeInputEsStream.stcChannel = g_dvrTest->streamPath[pathIndex].stcChannel;
    }
    #if (BCHP_VER >= BCHP_VER_B1)
    transcodeInputEsStream.audioEncodeParams.audioDummy = g_dvrTest->platformconfig.outputs.audioDummy[pathIndex];
    #endif
    BKNI_Memcpy((void *)&transcodeInputEsStream.esStreamInfo,(void *)&playbackMedia.esStreamInfo[1],sizeof(playbackMedia.esStreamInfo[1]));

    /* add audio ES stream*/
    printf("Dvr_FileToFile_TranscodeStart: Add Audio ES stream\n");
    B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream aPid <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream pcrPid >>>"));
    BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
    transcodeInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
    transcodeInputEsStream.esStreamInfo.pid = playbackMedia.esStreamInfo[1].pid+1;
    transcodeInputEsStream.playpumpIndex = NEXUS_ANY_ID;

    /* add pcr ES stream*/
    printf("Dvr_FileToFile_TranscodeStart: Add PCR ES stream\n");
    B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream pcrPid <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetSettings >>>"));
    B_DVR_TranscodeService_GetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
                                       &transcodeServiceSettings);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetSettings <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetSettings >>>"));
    transcodeServiceSettings.displaySettings.format = NEXUS_VideoFormat_e1080i;
    transcodeServiceSettings.stgSettings.enabled = true;
    if(g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)
    {
        transcodeServiceSettings.stgSettings.nonRealTime = true;
    }
    else
    {
        transcodeServiceSettings.stgSettings.nonRealTime = false;
    }
    transcodeServiceSettings.madSettings.deinterlace = true;
    transcodeServiceSettings.madSettings.enable22Pulldown = true;
    transcodeServiceSettings.madSettings.enable32Pulldown = true;
    transcodeServiceSettings.videoEncodeSettings.bitrateMax = 10*1024*1024;
    transcodeServiceSettings.videoEncodeSettings.frameRate = NEXUS_VideoFrameRate_e29_97;
    transcodeServiceSettings.videoEncodeSettings.streamStructure.framesB = 0;
    transcodeServiceSettings.videoEncodeSettings.streamStructure.framesP = 23;
    transcodeServiceSettings.videoEncodeSettings.streamStructure.trackInput = false;
    transcodeServiceSettings.videoEncodeSettings.enableFieldPairing = true;
    
    B_DVR_TranscodeService_SetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
                                       &transcodeServiceSettings);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetSettings <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Open >>>"));
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = NEXUS_ANY_ID;
    BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName,
                  B_DVR_MAX_FILE_NAME_LENGTH,"%s_%s%s",
                  g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName,
                  VideoCodecName, AudioCodecName);
    sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"transcode");
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex = 0;
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.segmented = true;
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = B_DVR_RecordServiceInputTranscode;


    DvrTest_Create_PatPmt(pathIndex,
                          playbackMedia.esStreamInfo[1].pid +1,
                          playbackMedia.esStreamInfo[0].pid, 
                          playbackMedia.esStreamInfo[1].pid,
                          VideoCodec,
                          AudioCodec);

    /*Open record service*/
    printf("Dvr_FileToFile_TranscodeStart: Open Record Service\n");
    g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
    if(!g_dvrTest->streamPath[pathIndex].recordService)
    {
        printf("record service open for transcode operation failed");
        goto error_recordService;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Open <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_InstallCallback >>>"));
    B_DVR_RecordService_InstallCallback(g_dvrTest->streamPath[pathIndex].recordService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_InstallCallback <<<"));

    dataInjectionOpenSettings.fifoSize = 30*188;
    g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
    assert(g_dvrTest->streamPath[pathIndex].dataInjectionService);
    recordServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_SetSettings >>>"));
    BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
    recordServiceSettings.parserBand  = NEXUS_ANY_ID;
    recordServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
    recordServiceSettings.transcodeService = (void *)g_dvrTest->streamPath[pathIndex].transcodeService;
    B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_SetSettings <<<"));


    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_Start >>>"));

    /* start transcode */
    printf("Dvr_FileToFile_TranscodeStart: Start Transcoding\n");
    rc = B_DVR_TranscodeService_Start(g_dvrTest->streamPath[pathIndex].transcodeService);
    if (rc != B_DVR_SUCCESS)
    {
        printf("Error in Transcode Service Start\n");
        goto error_transcodeService;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_Start <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel vPid >>>"));
    BKNI_Memset((void *)&recordInputEsStream,0,sizeof(recordInputEsStream));
    recordInputEsStream.esStreamInfo.pid = playbackMedia.esStreamInfo[0].pid;
    recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    recordInputEsStream.esStreamInfo.codec.videoCodec = VideoCodec; /*playbackMedia.esStreamInfo[0].codec.videoCodec*/;
    transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
                                                                playbackMedia.esStreamInfo[0].pid);
    if(!transcodePidChannel)
    {
        printf("\n error in getting the transcode pid Channel for %u",playbackMedia.esStreamInfo[0].pid);
        goto error_transcodePidChannel;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel vPid <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream vPid >>>"));
    recordInputEsStream.pidChannel=transcodePidChannel;

    /*Add transcoded video ES to the record*/
    printf("Dvr_FileToFile_TranscodeStart: Add Transcoded Video ES to the record\n");
    B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&recordInputEsStream);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream vPid <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel aPid >>>"));
    BKNI_Memset((void *)&recordInputEsStream,0,sizeof(recordInputEsStream));
    recordInputEsStream.esStreamInfo.pid = playbackMedia.esStreamInfo[1].pid;
    recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    recordInputEsStream.esStreamInfo.codec.audioCodec = AudioCodec; /*playbackMedia.esStreamInfo[1].codec.audioCodec;*/
    transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
                                                                playbackMedia.esStreamInfo[1].pid);
    if(!transcodePidChannel)
    {
        printf("\n error in getting the transcode pid Channel for %u",playbackMedia.esStreamInfo[1].pid);
        goto error_transcodePidChannel;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel aPid <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream aPid >>>"));
    recordInputEsStream.pidChannel=transcodePidChannel;

    /*Add transcoded audio ES to the record*/
    printf("Dvr_FileToFile_TranscodeStart: Add Transcoded Audio ES to the record\n");
    B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&recordInputEsStream);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream aPid <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel pcrPid >>>"));
    BKNI_Memset((void *)&recordInputEsStream,0,sizeof(recordInputEsStream));
    recordInputEsStream.esStreamInfo.pid = playbackMedia.esStreamInfo[1].pid+1;
    recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
    transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
                                                                playbackMedia.esStreamInfo[1].pid+1);
    if(!transcodePidChannel)
    {
        printf("\n error in getting the transcode pid Channel for %u",playbackMedia.esStreamInfo[1].pid+1);
        goto error_transcodePidChannel;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel pcrPid <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream pcrPid >>>"));
    recordInputEsStream.pidChannel=transcodePidChannel;

    /*Add transcoded PCR ES to the record*/
    printf("Dvr_FileToFile_TranscodeStart: Add Transcoded PCR ES to the record\n");
    B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&recordInputEsStream);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream pcrPid <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Start >>>"));

    /* start recording */
    printf("Dvr_FileToFile_TranscodeStart: Start recording\n");
    B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Start <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : DvrTest_PlaybackDecodeStart >>>"));

    /* start playback decode */
    printf("Dvr_FileToFile_TranscodeStart: Start playback decode\n");
    DvrTest_PlaybackDecodeStart(pathIndex);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : DvrTest_PlaybackDecodeStart <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_Start >>>"));

    /* start playback */
    printf("Dvr_FileToFile_TranscodeStart: Start playback\n");
    B_DVR_PlaybackService_Start(g_dvrTest->streamPath[pathIndex].playbackService);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_Start <<<"));
    
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : BKNI_CreateEvent >>>"));
    g_dvrTest->streamPath[pathIndex].transcodeFinishEvent = NULL;
    BKNI_CreateEvent(&g_dvrTest->streamPath[pathIndex].transcodeFinishEvent);
    if(!g_dvrTest->streamPath[pathIndex].transcodeFinishEvent)
    {
        printf("\n transcodeFinishEvent create failed");
        goto error_transcodeFinishEvent;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : BKNI_CreateEvent <<<"));
    return rc;

error_transcodeFinishEvent:
error_transcodePidChannel:
    B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
error_recordService:
    B_DVR_TranscodeService_Close(g_dvrTest->streamPath[pathIndex].transcodeService);
error_transcodeService:
    NEXUS_StcChannel_Close(g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeStcChannel);
error_transcodeStcChannel:
    B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
error_playbackService:
    return rc;
}


B_DVR_ERROR Dvr_FileToFile_TranscodeStop(unsigned pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_TranscodeServiceSettings transcodeServiceSettings;

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_GetSettings >>>"));
    B_DVR_TranscodeService_GetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
                                       &transcodeServiceSettings);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_GetSettings <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : DvrTest_PlaybackDecodeStop >>>"));

    printf("Dvr_FileToFile_TranscodeStop: Stop Playback Decode\n");
    DvrTest_PlaybackDecodeStop(pathIndex);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : DvrTest_PlaybackDecodeStop <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_PlaybackService_Stop >>>"));

    printf("Dvr_FileToFile_TranscodeStop: Stop Playback Service\n");
    B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[pathIndex].playbackService);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_PlaybackService_Stop <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_Stop >>>"));

    printf("Dvr_FileToFile_TranscodeStop: Stop Transcode Service\n");
    B_DVR_TranscodeService_Stop(g_dvrTest->streamPath[pathIndex].transcodeService);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_Stop <<<"));

    /* wait for the encoder buffer model's data to be drained */
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : BKNI_WaitForEvent >>>"));
    if(BKNI_WaitForEvent(g_dvrTest->streamPath[pathIndex].transcodeFinishEvent,
                         (transcodeServiceSettings.videoEncodeSettings.encoderDelay/27000)*2)!=BERR_SUCCESS) 
    {
        printf("\n Transcode Finish timeout");
    }

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : BKNI_WaitForEvent <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_Stop >>>"));

    printf("Dvr_FileToFile_TranscodeStop: Stop Record Service\n");
    B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_Stop <<<"));
    /* B_DVR_RecordService_RemoveAllInputEsStreams(g_dvrTest->streamPath[pathIndex].recordService);*/
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_RemoveCallback >>>"));

    B_DVR_RecordService_RemoveCallback(g_dvrTest->streamPath[pathIndex].recordService);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_RemoveCallback <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_Close >>>"));

    B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_Close <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_PlaybackService_RemoveCallback >>>"));

    B_DVR_PlaybackService_RemoveCallback(g_dvrTest->streamPath[pathIndex].playbackService);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_PlaybackService_RemoveCallback <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_PlaybackService_Close >>>"));

    B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_PlaybackService_Close <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_RemoveCallback >>>"));

    B_DVR_TranscodeService_RemoveCallback(g_dvrTest->streamPath[pathIndex].transcodeService);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_RemoveCallback <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_RemoveAllInputEsStreams >>>"));

    B_DVR_TranscodeService_RemoveAllInputEsStreams(g_dvrTest->streamPath[pathIndex].transcodeService);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_RemoveAllInputEsStreams <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_Close >>>"));

    B_DVR_TranscodeService_Close(g_dvrTest->streamPath[pathIndex].transcodeService);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_Close <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : NEXUS_StcChannel_Close >>>"));

    NEXUS_StcChannel_Close(g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeStcChannel);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : NEXUS_StcChannel_Close <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : DvrTest_LiveDecodeStart >>>"));
    g_dvrTest->streamPath[pathIndex].transcodeService = NULL;
    BKNI_Memset((void*)&g_dvrTest->streamPath[pathIndex].transcodeServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].transcodeServiceRequest));

    printf("Dvr_FileToFile_TranscodeStop: Start Live Decode\n");
    if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
       DvrTest_LiveDecodeStart(pathIndex);
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : DvrTest_LiveDecodeStart <<<"));

    return rc;
}

void Dvr_Get_VideoCodec_Name(unsigned vCodecType)
{

    switch (vCodecType)
    {
        case NEXUS_VideoCodec_eMpeg1:
            printf("MPEG1");
        break;
        case NEXUS_VideoCodec_eMpeg2:
            printf("MPEG2");
        break;
        case NEXUS_VideoCodec_eMpeg4Part2:
            printf("MPEG4Part2");
        break;
        case NEXUS_VideoCodec_eH263:
            printf("H263");
        break;
        case NEXUS_VideoCodec_eH264:
            printf("H264");
        break;
        case NEXUS_VideoCodec_eH264_Svc:
            printf("H264SVC");
        break;
        case NEXUS_VideoCodec_eH264_Mvc:
            printf("H264MVC");
        break;
        case NEXUS_VideoCodec_eVc1:
            printf("VC1");
        break;
        case NEXUS_VideoCodec_eVc1SimpleMain:
            printf("VC1SimpleMain");
        break;
        case NEXUS_VideoCodec_eDivx311:
            printf("DIVX311");
        break;
        case NEXUS_VideoCodec_eAvs:
            printf("AVS");
        break;
        default:
        break;
    }
}

void Dvr_Get_AudioCodec_Name(unsigned aCodecType)
{

    switch (aCodecType)
    {
        case NEXUS_AudioCodec_eMpeg:
            printf("MPEG");
        break;
        case NEXUS_AudioCodec_eMp3:
            printf("MP3");
        break;
        case NEXUS_AudioCodec_eAac:
            printf("AAC");
        break;
#if 0
        case NEXUS_AudioCodec_eAacAdts:
            printf("AAC");
        break;
#endif
        case NEXUS_AudioCodec_eAacLoas:
            printf("AAC LOAS");
        break;
        case NEXUS_AudioCodec_eAacPlus:
            printf("AAC+");
        break;
#if 0
        case NEXUS_AudioCodec_eAacPlusLoas:
            printf("AAC+");
        break;
#endif
        case NEXUS_AudioCodec_eAacPlusAdts:
            printf("AAC+ ADTS");
        break;
        case NEXUS_AudioCodec_eAc3:
            printf("AC3");
        break;
        case NEXUS_AudioCodec_eAc3Plus:
            printf("AC3+");
        break;
        default:
        break;
    }

}



int main(void)
{
    int decoderIndex,frontendIndex;
    int i, index;
    unsigned volumeIndex = 0;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned VideoCodec, AudioCodec;
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH];
    char programName[B_DVR_MAX_FILE_NAME_LENGTH];

    B_DVR_MediaStorageStatus mediaStorageStatus;

    B_DVR_Media media;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    unsigned esStreamIndex;
    unsigned VideoPID, AudioPID;
    unsigned record_videoCodec, record_audioCodec;



    system("umount /dev/sda1");
    system("umount /dev/sda2");
    system("umount /dev/sda3"); 

    g_dvrTest = BKNI_Malloc(sizeof(*g_dvrTest));

    BKNI_Memset(g_dvrTest,0,sizeof(*g_dvrTest));
    
    g_dvrTest->mediaStorage = B_DVR_MediaStorage_Open(NULL);

    B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);

    B_DVR_MediaStorage_MountVolume(g_dvrTest->mediaStorage, volumeIndex);

    g_dvrTest->maxChannels = MAX_STATIC_CHANNELS;
    g_dvrTest->streamPath[0].currentChannel = currentChannel;

    B_Os_Init();


    PlaybackEnd = B_Event_Create(NULL);


    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&g_dvrTest->platformconfig);

    printf("max stream path = %d\n", MAX_STREAM_PATH);
    printf("configured stream path = %d\n", MAX_AVPATH);

    for (frontendIndex=0,i=0;frontendIndex < MAX_STREAM_PATH; frontendIndex++ )
    {
        g_dvrTest->streamPath[frontendIndex].frontend = g_dvrTest->platformconfig.frontend[frontendIndex];
        if (g_dvrTest->streamPath[frontendIndex].frontend)
        {
            NEXUS_Frontend_GetCapabilities(g_dvrTest->streamPath[frontendIndex].frontend, &g_dvrTest->streamPath[frontendIndex].capabilities);
            if (g_dvrTest->streamPath[frontendIndex].capabilities.qam)
            {
                g_dvrTest->streamPath[i].frontend = g_dvrTest->platformconfig.frontend[frontendIndex];
                NEXUS_Frontend_GetQamStatus(g_dvrTest->streamPath[i].frontend, &g_dvrTest->streamPath[i].qamStatus);
                DvrTest_TuneChannel(i);
                g_dvrTest->streamPath[i].liveDecodeStarted= false;
                i++;
            }
        }
    }


    NEXUS_Display_GetDefaultSettings(&g_dvrTest->displaySettings);
    g_dvrTest->displaySettings.format = NEXUS_VideoFormat_e1080i;
    g_dvrTest->display = NEXUS_Display_Open(0, &g_dvrTest->displaySettings);
    NEXUS_Display_AddOutput(g_dvrTest->display, NEXUS_HdmiOutput_GetVideoConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
    NEXUS_HdmiOutput_GetSettings(g_dvrTest->platformconfig.outputs.hdmi[0], &g_dvrTest->hdmiSettings);
    g_dvrTest->hdmiSettings.hotplugCallback.callback = hotplug_callback;
    g_dvrTest->hdmiSettings.hotplugCallback.context = g_dvrTest->platformconfig.outputs.hdmi[0];
    g_dvrTest->hdmiSettings.hotplugCallback.param = (int)g_dvrTest->display;
    NEXUS_HdmiOutput_SetSettings(g_dvrTest->platformconfig.outputs.hdmi[0], &g_dvrTest->hdmiSettings);


    for(decoderIndex=0;decoderIndex < MAX_AVPATH; decoderIndex++)
    {
        DvrTest_AudioVideoPathOpen(decoderIndex);
        DvrTest_LiveDecodeStart(decoderIndex);
        g_dvrTest->streamPath[decoderIndex].liveDecodeStarted = true;
        g_dvrTest->streamPath[decoderIndex].playbackDecodeStarted = false;
    }

    g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)DvrTest_Callback;
    g_dvrTest->dvrManager = B_DVR_Manager_Init(NULL);
    if(!g_dvrTest->dvrManager)
    {
        printf("Error in opening the dvr manager\n");
    }
    B_DVR_Manager_CreateMediaNodeList(g_dvrTest->dvrManager,0);

    B_DVR_TSB_Buffer_Preallocate(MAX_AVPATH);

    printf("\n\n\n Enter a program name to be transcoded: ");
    scanf("%s", programName);

    printf("\n Enter the subDir for the program: ");
    scanf("%s", subDir);

    /* check the recording's video codec and audio codec to exclude them 
    from the selection of codecs for transcoding */

    mediaNodeSettings.subDir = subDir;
    mediaNodeSettings.programName = programName;
    mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
    mediaNodeSettings.volumeIndex = volumeIndex;

    printf("\n*******************************\n");
    printf("  Information of a record\n");
    printf("*******************************\n");

    B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager, &mediaNodeSettings, &media);

    for(esStreamIndex=0;esStreamIndex < media.esStreamCount;esStreamIndex++)
    {
        if(media.esStreamInfo[esStreamIndex].pidType == eB_DVR_PidTypeVideo)
        {
            VideoPID = media.esStreamInfo[esStreamIndex].pid;
            printf("Video PID: %d\n", VideoPID);
            record_videoCodec = media.esStreamInfo[esStreamIndex].codec.videoCodec;
            printf("Video Codec Type: %d (", record_videoCodec);
            Dvr_Get_VideoCodec_Name(record_videoCodec);
            printf(")\n");
        }
        if(media.esStreamInfo[esStreamIndex].pidType == eB_DVR_PidTypeAudio)
        {
            AudioPID = media.esStreamInfo[esStreamIndex].pid;
            printf("Audio PID: %d\n", AudioPID);
            record_audioCodec = media.esStreamInfo[esStreamIndex].codec.audioCodec;
            printf("Audio Codec Type: %d (", record_audioCodec);
            Dvr_Get_AudioCodec_Name(record_audioCodec);
            printf(")\n");
        }
    }
    printf("*******************************\n");




    for (;;)
    {
        char cmd[16];
        int i;
        bool exit = false;
        unsigned vCodec;


       printf("\nCMD: Video>");
       if(fgets(cmd, sizeof(cmd)-1, stdin)==NULL)
       {
           break;
       }
       for(i=0;cmd[i]!='\0';i++)
       {
           switch(cmd[i]) {
           case '?':
               printf("? - This help\n"
                        "2 - MPEG2 Video\n"
                        "5 - H264 Video\n"
                        "q - Quit\n");
            break;
#if 0
            case '1':
                vCodec = NEXUS_VideoCodec_eMpeg1;
                if (vCodec == record_videoCodec)
                {
                    printf("Transcoded Video Codec should be different from the Video Codec of a record\n");
                    printf("Choose a different Video Codec\n");
                }
                else
                {
                    printf("VideoCodec selection is MPEG1\n");
                    sprintf(VideoCodecName, "mpg1");
                    exit = true;
                }
            break;
#endif
            case '2':
                vCodec = NEXUS_VideoCodec_eMpeg2;
                if (vCodec == record_videoCodec)
                {
                    printf("Transcoded Video Codec should be different from the Video Codec of a record\n");
                    printf("Choose a different Video Codec\n");
                }
                else
                {
                    printf("VideoCodec selection is MPEG2\n");
                    sprintf(VideoCodecName, "mpg2");
                    exit = true;
                }
            break;
#if 0
            case '3':
                vCodec = NEXUS_VideoCodec_eMpeg4Part2;
                if (vCodec == record_videoCodec)
                {
                    printf("Transcoded Video Codec should be different from the Video Codec of a record\n");
                    printf("Choose a different Video Codec\n");
                }
                else
                {
                    printf("VideoCodec selection is MPEG4Part2\n");
                    sprintf(VideoCodecName, "mpg4");
                    exit = true;
                }
            break;
            case '4':
                vCodec = NEXUS_VideoCodec_eH263;
                if (vCodec == record_videoCodec)
                {
                    printf("Transcoded Video Codec should be different from the Video Codec of a record\n");
                    printf("Choose a different Video Codec\n");
                }
                else
                {
                    printf("VideoCodec selection is H263\n");
                    sprintf(VideoCodecName, "h263");
                    exit = true;
                }
            break;
#endif
            case '5':
                vCodec = NEXUS_VideoCodec_eH264;
                if (vCodec == record_videoCodec)
                {
                    printf("Transcoded Video Codec should be different from the Video Codec of a record\n");
                    printf("Choose a different Video Codec\n");
                }
                else
                {
                    printf("VideoCodec selection is H264\n");
                    sprintf(VideoCodecName, "h264");
                    exit = true;
                }
            break;
#if 0
            case '6':
                vCodec = NEXUS_VideoCodec_eH264_Svc;
                if (vCodec == record_videoCodec)
                {
                    printf("Transcoded Video Codec should be different from the Video Codec of a record\n");
                    printf("Choose a different Video Codec\n");
                }
                else
                {
                    printf("VideoCodec selection is H264 SVC\n");
                    sprintf(VideoCodecName, "h264s");
                    exit = true;
                }
            break;
            case '7':
                vCodec = NEXUS_VideoCodec_eH264_Mvc;
                if (vCodec == record_videoCodec)
                {
                    printf("Transcoded Video Codec should be different from the Video Codec of a record\n");
                    printf("Choose a different Video Codec\n");
                }
                else
                {
                    printf("VideoCodec selection is H264 MVC\n");
                    sprintf(VideoCodecName, "h264m");
                    exit = true;
                }
            break;
            case '8':
                vCodec = NEXUS_VideoCodec_eDivx311;
                if (vCodec == record_videoCodec)
                {
                    printf("Transcoded Video Codec should be different from the Video Codec of a record\n");
                    printf("Choose a different Video Codec\n");
                }
                else
                {
                    printf("VideoCodec selection is DIVX\n");
                    sprintf(VideoCodecName, "divx");
                    exit = true;
                }
            break;
            case '9':
                vCodec = NEXUS_VideoCodec_eAvs;
                if (vCodec == record_videoCodec)
                {
                    printf("Transcoded Video Codec should be different from the Video Codec of a record\n");
                    printf("Choose a different Video Codec\n");
                }
                else
                {
                    printf("VideoCodec selection is AVS\n");
                    sprintf(VideoCodecName, "avs");
                    exit = true;
                }
            break;
#endif
            case 'q':
                goto closing_services;
            break;
            default:
                break;
            }
        }
        if (exit)
        {
            VideoCodec = vCodec;
            break;
        }
    }

    for (;;)
    {
        char cmd[16];
        int i;
        bool exit = false;
        unsigned aCodec;

        printf("\nCMD: Audio>");
        if(fgets(cmd, sizeof(cmd)-1, stdin)==NULL)
        {
            break;
        }
        for(i=0;cmd[i]!='\0';i++)
        {
            switch(cmd[i]) {
            case '?':
                printf("? - This help\n"
                        "2 - MP3 Audio\n"
                        "3 - AAC Audio\n"
                        "4 - AAC LOAS Audio\n"
                        "5 - AAC+ Audio\n"
                        "6 - AAC+ ADTS Audio\n"
                        "7 - AC3 Audio\n"
                        "q - Quit\n");
            break;
#if 0
            case '1':
                aCodec = NEXUS_AudioCodec_eMpeg;
                printf("AudioCodec selection is MPEG\n");
                sprintf(AudioCodecName, "mpg");
                exit = true;
            break;
#endif
            case '2':
                aCodec = NEXUS_AudioCodec_eMp3;
                if (aCodec == record_audioCodec)
                {
                    printf("Transcoded Audio Codec should be different from the Audio Codec of a record\n");
                    printf("Choose a different Audio Codec\n");
                }
                else
                {
                    printf("AudioCodec selection is MP3\n");
                    sprintf(AudioCodecName, "mp3");
                    exit = true;
                }
            break;
            case '3':
                aCodec = NEXUS_AudioCodec_eAac;
                if (aCodec == record_audioCodec)
                {
                    printf("Transcoded Audio Codec should be different from the Audio Codec of a record\n");
                    printf("Choose a different Audio Codec\n");
                }
                else
                {
                    printf("AudioCodec selection is AAC\n");
                    sprintf(AudioCodecName, "aac");
                    exit = true;
                }
            break;
            case '4':
                aCodec = NEXUS_AudioCodec_eAacLoas;
                if (aCodec == record_audioCodec)
                {
                    printf("Transcoded Audio Codec should be different from the Audio Codec of a record\n");
                    printf("Choose a different Audio Codec\n");
                }
                else
                {
                    printf("AudioCodec selection is AAC LOAS\n");
                    sprintf(AudioCodecName, "aacl");
                    exit = true;
                }
            break;
            case '5':
                aCodec = NEXUS_AudioCodec_eAacPlus;
                if (aCodec == record_audioCodec)
                {
                    printf("Transcoded Audio Codec should be different from the Audio Codec of a record\n");
                    printf("Choose a different Audio Codec\n");
                }
                else
                {
                    printf("AudioCodec selection is AAC+\n");
                    sprintf(AudioCodecName, "aac+");
                    exit = true;
                }
            break;
            case '6':
                aCodec = NEXUS_AudioCodec_eAacPlusAdts;
                if (aCodec == record_audioCodec)
                {
                    printf("Transcoded Audio Codec should be different from the Audio Codec of a record\n");
                    printf("Choose a different Audio Codec\n");
                }
                else
                {
                    printf("AudioCodec selection is AAC+ADTS\n");
                    sprintf(AudioCodecName, "aac+a");
                    exit = true;
                }
            break;
            case '7':
                aCodec = NEXUS_AudioCodec_eAc3;
                if (aCodec == record_audioCodec)
                {
                    printf("Transcoded Audio Codec should be different from the Audio Codec of a record\n");
                    printf("Choose a different Audio Codec\n");
                }
                else
                {
                    printf("AudioCodec selection is AC3\n");
                    sprintf(AudioCodecName, "ac3");
                    exit = true;
                }
            break;
#if 0
            case '8':
                aCodec = NEXUS_AudioCodec_eAc3Plus;
                if (aCodec == record_audioCodec)
                {
                    printf("Transcoded Audio Codec should be different from the Audio Codec of a record\n");
                    printf("Choose a different Audio Codec\n");
                }
                else
                {
                    printf("AudioCodec selection is AC3+\n");
                    sprintf(AudioCodecName, "ac3+");
                    exit = true;
                }
            break;
#endif
            case 'q':
                goto closing_services;
            break;
            default:
            break;
            }
        }
        if (exit)
        {
            AudioCodec = aCodec;
            break;
        }
    }

    printf("\n\n************************************************\n");
    printf ("Video Codec Type: %d, Video Codec Name: %s\n", VideoCodec, VideoCodecName);
    printf ("Audio Codec Type: %d, Audio Codec Name: %s\n", AudioCodec, AudioCodecName);
    printf("************************************************\n");

    rc = Dvr_FileToFile_TranscodeStart(0, VideoCodec, AudioCodec, subDir, programName);
    if (rc == B_DVR_SUCCESS)
    {
        if (DataInjectionEnabled)
        {
            Dvr_DataInject_Start(0);
        }
        /* wailt until the playback point is reached to the end */
        while(B_Event_Wait(PlaybackEnd, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
        if (DataInjectionEnabled)
        {
            Dvr_DataInject_Stop(0);
        }
        Dvr_FileToFile_TranscodeStop(0);

    }

closing_services:

    printf("\n Closing all the services before stopping the test app");
    for(index=0;index< MAX_STREAM_PATH; index++)
    {
        if(index < MAX_AVPATH)
        {
            if(g_dvrTest->streamPath[index].playbackDecodeStarted)
            {
                DvrTest_PlaybackDecodeStop(index);
            }
            if(g_dvrTest->streamPath[index].tsbService)
            {
                B_DVR_TSBService_Stop(g_dvrTest->streamPath[index].tsbService);
                B_DVR_TSBService_Close(g_dvrTest->streamPath[index].tsbService);
            }
#if 0
            if(g_dvrTest->streamPath[index].playbackService)
            {
                B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[index].playbackService);
                B_DVR_PlaybackService_Close(g_dvrTest->streamPath[index].playbackService);
            }
#endif

            if(g_dvrTest->streamPath[index].liveDecodeStarted)
            {
                DvrTest_LiveDecodeStop(index);
            }
            DvrTest_AudioVideoPathClose(index);
        }
        else
        {
            if(g_dvrTest->streamPath[index].recordService)
            {
                B_DVR_RecordService_Stop(g_dvrTest->streamPath[index].recordService);
                B_DVR_RecordService_Close(g_dvrTest->streamPath[index].recordService);
            }
        }
    }


    B_DVR_TSB_Buffer_Deallocate(MAX_AVPATH);

    B_DVR_Manager_DestroyMediaNodeList(g_dvrTest->dvrManager, volumeIndex);

    B_DVR_MediaStorage_UnmountVolume(g_dvrTest->mediaStorage, volumeIndex);
    B_DVR_Manager_UnInit(g_dvrTest->dvrManager);
    B_Os_Uninit();
    NEXUS_Platform_Uninit();

    return 0;
}
