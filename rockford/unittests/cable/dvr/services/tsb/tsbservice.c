#include "b_dvr_test.h"
#include <stdlib.h>
#include <time.h>

#define MAX_AV_PATH 1

static int currentChannel = 1;
static unsigned TSBServiceReady = 0;


int B_DVR_Generate_Random(int limit)
{
    int r;
    time_t t;
    unsigned int seed;

    t = time(NULL);
    seed = (unsigned int)t;
    srand(seed);
    r = rand() % limit;

    return (r);
}


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
    char *eventString[eB_DVR_EventMax+1] = {"recStart","recEnd","hdStream","sdStream","tsbConvCompleted","overFlow","underFlow","pbStart","pbEnd","pbAlarm","abortPb","abortRec","abortTSBRec","abortTSBpb","dataInjection","noMediaDiskSpace","noNavDiskSpace","noMetaDataDiskSpace","invalidEvent"};
    char *serviceString[eB_DVR_ServiceMax+1]= {"tsbService","playbackService","recordService","mediaUtil","drmService","storagaService","dataInjectionService","invalidService"};

    printf("\n DvrTest_Callback >>>> index %u event %s service %s",(unsigned)index,eventString[event],serviceString[service]);
    BSTD_UNUSED(appContext);
    switch(event)
    {
    case eB_DVR_EventStartOfRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
                B_DVR_TSBServiceStatus tsbServiceStatus;

                B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService,&tsbServiceStatus);
                printf("\n Beginning Of TSB Conversion start time %lu endTime %lu",tsbServiceStatus.tsbRecStartTime,tsbServiceStatus.tsbRecEndTime);
                TSBServiceReady = 1;
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
        case eB_DVR_EventStartOfPlayback:
            {
                if(service == eB_DVR_ServicePlayback)
                {
                  printf("\n Beginnning of playback stream. Pausing");
                }
                else
                {
                    printf("\n Beginning of TSB. Pausing\n");
                 }
            }
            break;
    case eB_DVR_EventEndOfPlayback:
        {
            printf("\n End of playback stream. Pausing");
            printf("\n Stop either TSBPlayback or Playback to enter live mode");
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
            printf("\n data Injection complete");
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


void B_DVR_TSB_Service_Status_Print(B_DVR_TSBServiceStatus tsbServiceStatus)
{
    printf("**********************************************\n");
    printf("tsbCurrentPlayTime = %lu\n", tsbServiceStatus.tsbCurrentPlayTime);
    printf("tsbCurrentPlayOffset = %lu\n", (unsigned long)tsbServiceStatus.tsbCurrentPlayOffset);

    if (tsbServiceStatus.tsbPlayback)
        printf("tsbPlayback is true\n");
    else 
        printf("tsbPlayback is false\n");

    printf("tsbRecLinearStartOffset = %lu\n", (unsigned long)tsbServiceStatus.tsbRecLinearStartOffset);
    printf("tsbRecLinearEndOffset = %lu\n", (unsigned long)tsbServiceStatus.tsbRecLinearEndOffset);

    printf("tsbRecStartTime = %lu\n", tsbServiceStatus.tsbRecStartTime);
    printf("tsbRecEndTime = %lu\n", tsbServiceStatus.tsbRecEndTime);

    printf("tsbConversion = %d\n", tsbServiceStatus.tsbCoversion);

    printf("permRecLinearCurrentOffset = %lu\n", (unsigned long)tsbServiceStatus.permRecLinearCurrentOffset);
    printf("permRecCurrentTime = %lu\n", tsbServiceStatus.permRecCurrentTime);
    printf("tsbServiceStatus = %d ", tsbServiceStatus.state);
    switch (tsbServiceStatus.state) {
    case 0: 
        printf("(playback status is Stopped)\n");
    break;
    case 1:
        printf("(playback status is Playing)\n");
    break;
    case 2:
        printf("(playback status is Paused)\n");
    break;
    case 3:
        printf("(playback status is TrickMode)\n");
    break;
    case 4:
        printf("(playback status is Aborted)\n");
    break;
    case 5:
        printf("(playback status is eMax)\n");
    break;
    }
    printf("**********************************************\n");
}

void B_DVR_TSB_Service_Start(int pathIndex)
{
    B_DVR_TSBServiceInputEsStream inputEsStream;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;
    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;

    printf("TSB buffering begins\n");
    if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
        DvrTest_LiveDecodeStart(pathIndex);
    }
    printf("\n TSBService starting on channel %u and path %d",currentChannel,pathIndex);
    BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
    BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
            sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName),
            "%s%d",TSB_SERVICE_PREFIX,pathIndex);
    printf("\n TSBService programName %s",g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName);
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
    sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.recpumpIndex = pathIndex;
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.input = eB_DVR_TSBServiceInputQam;
    g_dvrTest->streamPath[pathIndex].tsbService = B_DVR_TSBService_Open(&g_dvrTest->streamPath[pathIndex].tsbServiceRequest);
    B_DVR_TSBService_InstallCallback(g_dvrTest->streamPath[pathIndex].tsbService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);

    BKNI_Memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
    dataInjectionOpenSettings.fifoSize = 188*30; /* max number of packets that can be dumped in one shot */
    g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
    tsbServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
    tsbServiceSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
    B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);

    BKNI_Memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].videoPids[0];
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    inputEsStream.esStreamInfo.codec.videoCodec = channelInfo[currentChannel].videoFormat[0];
    inputEsStream.pidChannel = NULL;
    B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService, &inputEsStream);

    inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
    B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService, &inputEsStream);

    dvrError = B_DVR_TSBService_Start(g_dvrTest->streamPath[pathIndex].tsbService,NULL);
    if(dvrError!=B_DVR_SUCCESS)
    {
        printf("\n Error in starting the tsbService %d",pathIndex);
    }
    else
    {
        printf("tsbService started %d\n",pathIndex);
    }

}

void B_DVR_TSB_Service_Stop(int pathIndex)
{
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;

    if(g_dvrTest->streamPath[pathIndex].tsbService)
    {
        dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
        if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
        {
            DvrTest_PlaybackDecodeStop(pathIndex);
        }
        if(dvrError != B_DVR_SUCCESS)
        {
            printf("\n Error in stopping the timeshift service %d\n",pathIndex);
        }
        B_DVR_TSBService_RemoveCallback(g_dvrTest->streamPath[pathIndex].tsbService);
        dvrError = B_DVR_TSBService_Close(g_dvrTest->streamPath[pathIndex].tsbService);
        B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
        g_dvrTest->streamPath[pathIndex].tsbService = NULL;
        g_dvrTest->streamPath[pathIndex].dataInjectionService=NULL;
        BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
        if(dvrError != B_DVR_SUCCESS)
        {
            printf("\n Error in closing the timeshift service %d\n",pathIndex);
        }
    }
    else
    {
        printf("\n timeshift srvc not started");
    }

}

void B_DVR_TSB_Service_Play_Start(int pathIndex)
{
    B_DVR_OperationSettings operationSettings;
    B_DVR_TSBServiceStatus tsbServiceStatus;

    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceStatus);
    operationSettings.operation = eB_DVR_OperationTSBPlayStart;
    operationSettings.seekTime = tsbServiceStatus.tsbRecEndTime;
    if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
        DvrTest_LiveDecodeStop(pathIndex);
    }
    DvrTest_PlaybackDecodeStart(pathIndex);
    B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
    operationSettings.operation = eB_DVR_OperationPause; 
    B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
}

void B_DVR_TSB_Service_Play_Stop(int pathIndex)
{
    B_DVR_OperationSettings operationSettings;

    if(!g_dvrTest->streamPath[pathIndex].tsbService)
    {
        printf("\n error TSBPlayStart without TSBServiceStart");
    }
    else
    {
        operationSettings.operation = eB_DVR_OperationTSBPlayStop;
        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
        DvrTest_PlaybackDecodeStop(pathIndex);
        if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
        {
            DvrTest_LiveDecodeStart(pathIndex);
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




int main(void)
{
    int decoderIndex,frontendIndex;
    int i, index;
    unsigned volumeIndex = 0;

    B_DVR_MediaStorageStatus mediaStorageStatus;
    B_DVR_TSBServiceStatus tsbServiceStatus;

    B_DVR_OperationSettings operationSettings;


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

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&g_dvrTest->platformconfig);

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


    for(decoderIndex=0;decoderIndex < MAX_AV_PATH; decoderIndex++)
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

    B_DVR_TSB_Buffer_Preallocate(1);

    if(!g_dvrTest->streamPath[0].tsbService)
    {
        printf("\nTSB Service is not started\n");
        B_DVR_TSB_Service_Start(0);
        printf("Starting TSB Service...\n");
    }

    /* wait until TSB Service becomes ready */
    do 
    {
        if (TSBServiceReady)
            break;
    } while (1);

    for (;;) {
        char cmd[16];
        int i;
        bool quit = false;

       printf("\nCMD:>");
       if(fgets(cmd, sizeof(cmd)-1, stdin)==NULL) {
           break;
       }
       for(i=0;cmd[i]!='\0';i++) {
           switch(cmd[i]) {
           case '?':
               printf("? - This help\n"
                        "0 - TSB palyback start/pause\n"
                        "1 - TSB playback rewind\n"
                        "2 - TSB playback play\n"
                        "3 - TSB playback fastforward\n" 
                        "4 - Convert to record\n"
                        "5 - Switch to Live mode\n"
                        "6 - Switch to TSB mode\n"
                        "7 - Print TSB Status\n"
                        "q - Quit\n");
            break;
            case '0':
                    printf("Command: TSB playback pause\n");
                    if(g_dvrTest->streamPath[0].tsbService) {
                        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[0].tsbService,&tsbServiceStatus);
                        if (!tsbServiceStatus.tsbPlayback)
                        {
                            printf("TSB Playback is not started\n");
                            B_DVR_TSB_Service_Play_Start(0);
                            printf("TSB Playback is starting...\n");
                        }
                        else
                        {
                            if(!g_dvrTest->streamPath[0].playbackDecodeStarted)
                            {
                                printf("TSB playback is not started. Start TSB playback\n");
                            }
                            else
                            {
                                B_DVR_OperationSettings operationSettings;
                                operationSettings.operation = eB_DVR_OperationPause;
                                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
                            }
                        }
                    }
                    else 
                    {
                        printf("TSB Service is not enabled\n");
                    }
            break;
            case '1':
                printf("Command: TSB playback rewind\n");
                if(g_dvrTest->streamPath[0].tsbService) {
                    if(!g_dvrTest->streamPath[0].playbackDecodeStarted)
                    {
                        printf("TSB playback is not started. Start TSB playback\n");
                    }
                    else
                    {
                        operationSettings.operation = eB_DVR_OperationFastRewind;
                        operationSettings.operationSpeed = 4;
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
                    }
                }
                else 
                {
                    printf("TSB Service is not enabled\n");
                }
            break;
            case '2':
                printf("Command: TSB playback play\n");
                if(g_dvrTest->streamPath[0].tsbService) {
                    if(!g_dvrTest->streamPath[0].playbackDecodeStarted)
                    {
                        printf("TSB playback is not started. Start TSB playback\n");
                    }
                    else
                    {
                        operationSettings.operation = eB_DVR_OperationPlay;
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
                    }
                }
                else 
                {
                    printf("TSB Service is not enabled\n");
                }
            break;
            case '3':
                printf("Command: TSB playback forward\n");
                if(g_dvrTest->streamPath[0].tsbService) {
                    if(!g_dvrTest->streamPath[0].playbackDecodeStarted)
                    {
                        printf("TSB playback is not started. Start TSB playback\n");
                    }
                    else
                    {
                        operationSettings.operation = eB_DVR_OperationFastForward;
                        operationSettings.operationSpeed = 4;
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
                    }
                }
                else 
                {
                    printf("TSB Service is not enabled\n");
                }
            break;
            case '4':
                printf("Command: Convert to Record\n");
                if(g_dvrTest->streamPath[0].tsbService)
                {
                    B_DVR_ERROR rc = B_DVR_SUCCESS;
                    B_DVR_TSBServiceStatus tsbStatus;
                    B_DVR_TSBServicePermanentRecordingRequest recReq;
                    unsigned fileSuffix;

                    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[0].tsbService, &tsbStatus);
                    fileSuffix = B_DVR_Generate_Random(100);
                    sprintf(recReq.programName, "PermRecord%d", fileSuffix);
                    recReq.recStartTime = tsbStatus.tsbRecStartTime;
                    recReq.recEndTime = tsbStatus.tsbRecEndTime;

                    printf("\n \t program name %s \t start %lu \t end %lu",recReq.programName,recReq.recStartTime,recReq.recEndTime);
                    sprintf(recReq.subDir,"tsbConv");
                     rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[0].tsbService,&recReq);
                    if(rc!=B_DVR_SUCCESS)
                    {
                        printf("\n Invalid paramters");
                    }
                }
                else
                {
                    printf("TSB Service is not enabled\n");
                }
            break;
            case '5':
                printf("Command: Switch to Live mode\n");
                if(!g_dvrTest->streamPath[0].tsbService)
                    printf("It is alreay in the Live mode!!!\n");
                else {
                    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[0].tsbService,&tsbServiceStatus);
                    if (tsbServiceStatus.tsbPlayback)
                    {
                        B_DVR_TSB_Service_Play_Stop(0);
                    }
                    B_DVR_TSB_Service_Stop(0);
                }
            break;
            case '6':
                printf("Command: Switch to TSB mode\n");
                if(!g_dvrTest->streamPath[0].tsbService)
                {
                    printf("Starting TSB Service...\n");
                    B_DVR_TSB_Service_Start(0);
                }
                else
                {
                    printf("TSB Service is already started\n");
                }
            break;
            case '7':
                printf("Command: Print TSB Status\n");
                if(!g_dvrTest->streamPath[0].tsbService)
                {
                    printf("TSB Service Status parameters can't be acquired since it is in the live mode\n");
                }
                else
                {
                    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[0].tsbService,&tsbServiceStatus);
                    B_DVR_TSB_Service_Status_Print(tsbServiceStatus);
                }
            break;
            case 'q':
                quit = true;
            break;
            default:
                break;
           }
        }
        if (quit)
            break;
    }


    printf("\n Closing all the services before stopping the test app");
    for(index=0;index< MAX_STREAM_PATH; index++)
    {
        if(index < MAX_AV_PATH)
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
            if(g_dvrTest->streamPath[index].playbackService)
            {
                B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[index].playbackService);
                B_DVR_PlaybackService_Close(g_dvrTest->streamPath[index].playbackService);
            }
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


    B_DVR_TSB_Buffer_Deallocate(1);

    B_DVR_Manager_DestroyMediaNodeList(g_dvrTest->dvrManager,0);

    B_DVR_MediaStorage_UnmountVolume(g_dvrTest->mediaStorage,0);
    B_DVR_Manager_UnInit(g_dvrTest->dvrManager);
    B_Os_Uninit();
    NEXUS_Platform_Uninit();

    return 0;
}
