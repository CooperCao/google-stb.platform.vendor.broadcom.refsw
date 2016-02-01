#include "b_dvr_test.h"

#define MAX_AV_PATH 1


static int currentChannel = 1;

B_EventHandle TSBServiceReady;
B_EventHandle DataInjectionComplete;
B_EventHandle TSBConversionComplete;



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



B_DVR_ERROR B_DVR_TSB_Convert_Start(int pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_TSBServiceStatus tsbStatus;
    B_DVR_TSBServicePermanentRecordingRequest recReq;

    if(g_dvrTest->streamPath[pathIndex].tsbService)
    {
        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService, &tsbStatus);
        sprintf(recReq.programName, "DataInject");
        recReq.recStartTime = tsbStatus.tsbRecStartTime;
        recReq.recEndTime = tsbStatus.tsbRecEndTime;
    
        printf("\n \t program name %s \t start %lu \t end %lu",recReq.programName,recReq.recStartTime,recReq.recEndTime);
        sprintf(recReq.subDir,"tsbConv");
         rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[pathIndex].tsbService,&recReq);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n Invalid paramters");
        }
    }
    else
    {
        printf("TSB Service is not enabled\n");
    }
    return (rc);
}

void B_DVR_TSB_Convert_Stop(int pathIndex)
{

    B_DVR_ERROR rc = B_DVR_SUCCESS;

    rc = B_DVR_TSBService_ConvertStop(g_dvrTest->streamPath[pathIndex].tsbService);
    if(rc!=B_DVR_SUCCESS)
    {
        printf("\n Failed to stop conversion");
    }
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


B_DVR_ERROR B_DVR_TSB_Data_Injection_Start(unsigned injectionType, unsigned pid, int pathIndex)
{
    B_DVR_ERROR rc =B_DVR_SUCCESS;
    B_DVR_DataInjectionServiceSettings settings;
    char dataFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    uint8_t pkt[188*30];
    int dataFileID;
    int readSize;



    if (pid == 0x0)
        sprintf(dataFileName, "/mnt/nfs/PAT.TS");
    else
        sprintf(dataFileName, "/mnt/nfs/PMT.TS");
    
    if ((dataFileID = open(dataFileName, O_RDONLY,0666)) < 0)
    {
        printf("\n unable to open %s",dataFileName);
    }
    
    BKNI_Memset((void *)&pkt[0],0,188*30);
    
    readSize = (int) read(dataFileID,&pkt[0],188*30);
    
    if(readSize < 0)
    {
        printf("\n Data file is empty or not available");
    }
    
    B_DVR_DataInjectionService_InstallCallback(g_dvrTest->streamPath[pathIndex].dataInjectionService,
        g_dvrTest->dvrTestCallback,(void*)g_dvrTest);

    if ((injectionType != eB_DVR_DataInjectionServiceTypePSI) && 
         (injectionType!= eB_DVR_DataInjectionServiceTypeRawData))
         return B_DVR_INVALID_PARAMETER;

    settings.dataInjectionServiceType = injectionType;
    settings.pid = pid;
    B_DVR_DataInjectionService_SetSettings(g_dvrTest->streamPath[pathIndex].dataInjectionService,&settings);
    if(g_dvrTest->streamPath[pathIndex].tsbService)
    {
        B_DVR_TSBService_InjectDataStart(g_dvrTest->streamPath[pathIndex].tsbService,&pkt[0],readSize);
    }
    else
    {
        printf("TSB Service is not available\n");
    }
    return (rc);
}

void B_DVR_TSB_Data_Injection_Stop(int pathIndex)
{
        if(g_dvrTest->streamPath[pathIndex].tsbService)
        {
            B_DVR_TSBService_InjectDataStop(g_dvrTest->streamPath[pathIndex].tsbService);
        }
        else
        {
            printf("TSB Service is not available\n");
        }
}


B_DVR_ERROR B_DVR_Delete_Record(int volIndex, char *subDir, char *programName)
{

    B_DVR_MediaNodeSettings *mediaNodeSettings;
    B_DVR_ERROR err;
    mediaNodeSettings = malloc(sizeof(B_DVR_MediaNodeSettings));
    
    mediaNodeSettings->subDir = subDir;
    mediaNodeSettings->programName = programName;
    mediaNodeSettings->volumeIndex = volIndex;
    mediaNodeSettings->recordingType = eB_DVR_RecordingPermanent;

    printf("deleting volume: %u directory: %s program: %s\n",mediaNodeSettings->volumeIndex, mediaNodeSettings->subDir, mediaNodeSettings->programName);

    err = B_DVR_Manager_DeleteRecording(B_DVR_Manager_GetHandle(),mediaNodeSettings);
    if (err!=B_DVR_SUCCESS)
    {
        printf("check the file name!\n");
    }
    free(mediaNodeSettings);
    return err;
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
                B_Event_Set(TSBServiceReady);
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
            B_Event_Set(TSBConversionComplete);
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
            B_Event_Set(DataInjectionComplete);
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

B_DVR_ERROR CopyMediaFromMetaData(char *fileName)
{
    B_DVR_SegmentedFileNode fileNode;
    off_t returnOffset, fileNodeOffset;
    ssize_t sizeRead=0;
    unsigned index=0;
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_File dvrFile;
    B_DVR_FileSettings settings;
    char temp[512];

    settings.fileIO = eB_DVR_FileIORead;
    settings.directIO = false;
    B_DVR_File_Open(&dvrFile,fileName, &settings);

    printf("dumpMetaDataFile %s>>>\n",fileName);
    do
    {
        fileNodeOffset = index*sizeof(fileNode);
        returnOffset = dvrFile.fileInterface.io.readFD.seek(
            &dvrFile.fileInterface.io.readFD,
            fileNodeOffset,SEEK_SET);

        if(returnOffset != fileNodeOffset)
        {
            printf("End of metadata file\n");
            break;
        }
         sizeRead = dvrFile.fileInterface.io.readFD.read(
             &dvrFile.fileInterface.io.readFD,&fileNode,sizeof(fileNode));

        if(sizeRead != sizeof(fileNode))
        {
            printf("end of metaDataFile\n");
        }
        else
        {
            printf("file node-%u %s %lu %lu %d\n",
                      index,
                      fileNode.fileName,
                      (unsigned long)fileNode.linearStartOffset,
                      (unsigned long)fileNode.size,
                      fileNode.recordType);

            sprintf(temp, "cp /mnt/dvr0-media/record/%s /mnt/nfs/", fileNode.fileName);
            printf("command: %s\n", temp);
            system(temp);
        }
        index++;
    }while(sizeRead);


    B_DVR_File_Close(&dvrFile);
    printf("dumpMetaDataFile <<<\n");
    return rc;
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
    unsigned volumeIndex = 0;
    int i, index;
    int injectionType;
    B_DVR_ERROR rc = B_DVR_SUCCESS;

    B_DVR_MediaStorageStatus mediaStorageStatus;

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

    TSBServiceReady = B_Event_Create(NULL);
    DataInjectionComplete = B_Event_Create(NULL);
    TSBConversionComplete = B_Event_Create(NULL);

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

    B_DVR_TSB_Buffer_Preallocate(MAX_AV_PATH);

    if(!g_dvrTest->streamPath[0].tsbService)
    {
        B_DVR_TSB_Service_Start(0);
    }

    /* wait until TSB Service becomes ready */
    while(B_Event_Wait(TSBServiceReady, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);

    injectionType = eB_DVR_DataInjectionServiceTypeRawData;

    for (i=0; i < 10; i++) {

        /* Data Injection of PAT */
        B_DVR_TSB_Data_Injection_Start(injectionType, 0x0, 0);
        while(B_Event_Wait(DataInjectionComplete, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
        printf("\n$$$ Data Injection is Complete $$$\n");
        B_Event_Reset(DataInjectionComplete);
        B_DVR_TSB_Data_Injection_Stop(0);
        /* wait for 100 milisec */
        BKNI_Sleep(100);

        /* Data Injection of PMT */
        B_DVR_TSB_Data_Injection_Start(injectionType, 0x3f, 0);
        while(B_Event_Wait(DataInjectionComplete, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
        printf("\n$$$ Data Injection is Complete $$$\n");
        B_Event_Reset(DataInjectionComplete);
        B_DVR_TSB_Data_Injection_Stop(0);
        /* wait for 100 milisec */
        BKNI_Sleep(100);
    }

    B_DVR_TSB_Convert_Start(0);
    while(B_Event_Wait(TSBConversionComplete, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
    B_DVR_TSB_Convert_Stop(0);


    B_DVR_TSB_Service_Stop(0);

    CopyMediaFromMetaData("/mnt/dvr0-metadata/tsbConv/DataInject.ts");

    rc = B_DVR_Delete_Record(0, "tsbConv", "DataInject");
    if (rc == B_DVR_SUCCESS)
        printf("DataInjected File is removed successfully\n");
    else
        printf("DataInjected File removal is failed\n");
    
    printf("\n Closing all the services before stopping the test app");
    for(index=0;index < MAX_STREAM_PATH; index++)
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

    B_DVR_TSB_Buffer_Deallocate(MAX_AV_PATH);

    B_DVR_Manager_DestroyMediaNodeList(g_dvrTest->dvrManager, volumeIndex);

    B_DVR_MediaStorage_UnmountVolume(g_dvrTest->mediaStorage, volumeIndex);
    B_DVR_Manager_UnInit(g_dvrTest->dvrManager);
    B_Os_Uninit();
    NEXUS_Platform_Uninit();

    return 0;
}
