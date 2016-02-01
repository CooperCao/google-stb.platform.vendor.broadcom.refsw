#include "dvr_test.h"
BDBG_MODULE(hn8TSBTo8StreamNet2PBInjection);

#define MAX_TSBPATH 8
#define DURATION1    100
#define DURATION2    200
#define DURATION3    500
#define DURATION4    1000
#define DURATION     DURATION3

#define STREAMSIZE0  BUFFER_LEN*4   /*2.5MB*/ 
#define STREAMSIZE1  BUFFER_LEN*8   /*5MB*/ 
#define STREAMSIZE2  BUFFER_LEN*16  /*10MB*/ 
#define STREAMSIZE3  BUFFER_LEN*32  /*20MB*/ 
#define STREAMSIZE4  BUFFER_LEN*80  /*50MB*/ 
#define STREAMSIZE5  BUFFER_LEN*800  /*500MB*/ 
#define STREAMSIZE6  BUFFER_LEN*1600  /*1000MB*/ 
#define STREAMSIZE7  BUFFER_LEN*240  /*150MB*/ 
#define STREAMSIZE8  BUFFER_LEN*320  /*200MB*/ 

#define STREAMSIZE   STREAMSIZE7
#define UDPCHUNK     PACKET_CHUNK*CHUNK_ELEMENT
#define CHUNKREADCNT 2  

typedef struct {
    int *cmds;
	void * ptr;
} TSBTrickPlayThreadParam;

typedef struct {
    unsigned injectionType;
    unsigned PAT_PID;
    unsigned PMT_PID;
	void * ptr;
} DataInjectThreadParam;

static int currentChannel = 2;
static char hnmulstreamingtxt[] = "hnmulstreaming.txt";

B_EventHandle TSBServiceReady;
B_EventHandle DataInjectionComplete[MAX_TSBPATH];
B_EventHandle DataInjectionStop;
B_EventHandle MultipleStreamReady;
unsigned int  MulTSBServiceReadyNum = 0;
unsigned long bufSize = PACKET_CHUNK*CHUNK_ELEMENT+B_DVR_IO_BLOCK_SIZE*2; 

static void DvrTest_MulCallback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service);
static void OperationLiveDecodeStart(CuTest * tc, int index);
static void OperationPlaybackDecodeStop(CuTest * tc, int index);
static void OperationTSBServiceStart(CuTest * tc, int pathIndex);
static void OperationTSBServiceStop(CuTest * tc, int pathIndex);
static B_DVR_ERROR OperationTSBDataInjectionStart(CuTest * tc, unsigned injectionType, unsigned pid, int pathIndex);
static void OperationTSBDataInjectionStop(CuTest * tc, int pathIndex);
static void OperationTSBBufferPreallocate(CuTest * tc, int numAVPath);
static void OperationTSBBufferDeallocate(CuTest * tc, int numAVPath);
static void OperationDataInject(void *context);
static void InitHnMulStreamingpara(void);
static void CheckMulStreamTestName(char *buffer);
static int  HnMulStreamingLoadTxt(void);
static int  InitMulStreamUDPSocket(void);
static void stream_UDP(int pathIndex);
static void stream_MulThread_Created(int pathIndex);
static void stream_MulThread_Destroy(int pathIndex);
static void stream_Action(void *context);
static void stream_MulThread_Active(void);


static void DvrTest_MulCallback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service)
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
                ++MulTSBServiceReadyNum;
                if (MulTSBServiceReadyNum == MAX_TSBPATH)
                {
                    B_Event_Set(TSBServiceReady);
                }
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
            printf("\n TSB Conversion completed for the index path %d", index);
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
            printf("\n Data Injection is complete for the index path %d", index);
            B_Event_Set(DataInjectionComplete[index]);
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


static void OperationLiveDecodeStart(CuTest * tc, int index)
{
    int currentChannel = g_dvrTest->streamPath[index].currentChannel;
    NEXUS_TimebaseSettings timebaseSettings;
    BSTD_UNUSED(tc);
    printf("\n %s:index %d >>>",__FUNCTION__,index);
    /* Open the pid channels */
    if(g_dvrTest->streamPath[index].tsbService)
    { 
        /* NEXUS_PidChannelSettings settings; */
        printf("\n tsbservice index= %d \n",index);

        g_dvrTest->streamPath[index].videoPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                    channelInfo[currentChannel].videoPids[0], NULL);
    }
    else
    {
        printf("\n Before NOT -- tsbservice index= %d Input band=%d\n",index,g_dvrTest->streamPath[index].parserBand);
    g_dvrTest->streamPath[index].videoPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                   channelInfo[currentChannel].videoPids[0], NULL);
        printf("\n After NOT -- tsbservice index= %d \n",index);

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


static void OperationPlaybackDecodeStop(CuTest * tc, int index)
{
    NEXUS_VideoDecoderSettings videoDecoderSettings;
	BSTD_UNUSED(tc);
    printf("\n %s: index %d >>>",__FUNCTION__,index);
    NEXUS_VideoDecoder_GetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    NEXUS_VideoDecoder_SetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    if(!index)
    {
        NEXUS_AudioDecoder_Stop(g_dvrTest->streamPath[index].audioDecoder);
    }
    NEXUS_VideoDecoder_Stop(g_dvrTest->streamPath[index].videoDecoder);
    if(!index)
    {
        NEXUS_Playback_ClosePidChannel(g_dvrTest->streamPath[index].playback,g_dvrTest->streamPath[index].audioPlaybackPidChannels[0]);
    }
    NEXUS_Playback_ClosePidChannel(g_dvrTest->streamPath[index].playback,g_dvrTest->streamPath[index].videoPlaybackPidChannels[0]);
    g_dvrTest->streamPath[index].playbackDecodeStarted = false;
    printf("\n %s: index %d <<<",__FUNCTION__,index);
    return;
}


static void OperationTSBServiceStart(CuTest * tc, int pathIndex)
{
    B_DVR_TSBServiceInputEsStream inputEsStream;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;
    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;


    printf("TSB buffering begins pathindex= %d\n",pathIndex);
    if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
        if ((pathIndex == 0) || (pathIndex == 1))
        {
            printf("before OperationLiveDecodeStart\n");
            OperationLiveDecodeStart(tc, pathIndex);
            printf("After OperationLiveDecodeStart\n");
        }
    }
    printf("\n TSBService starting on channel %u and path %d",currentChannel,pathIndex);
    BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
    BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
            sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName),
            "%s%d",STREAM_TSB_SERVICE_PREFIX,pathIndex);
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
	CuAssertTrue(tc, dvrError==B_DVR_SUCCESS);
}


static void OperationTSBServiceStop(CuTest *tc, int pathIndex)
{
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;

    if(g_dvrTest->streamPath[pathIndex].tsbService)
    {
        dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
        if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
        {
            OperationPlaybackDecodeStop(tc, pathIndex);
        }
        if(dvrError != B_DVR_SUCCESS)
        {
            printf("\n Error in stopping the timeshift service %d\n",pathIndex);
        }
        /* temporal workaround */
        BKNI_Sleep(1000);
        /* temporal workaround */
		CuAssertTrue(tc, dvrError==B_DVR_SUCCESS);
        B_DVR_TSBService_RemoveCallback(g_dvrTest->streamPath[pathIndex].tsbService);
		sleep(2);
        dvrError = B_DVR_TSBService_Close(g_dvrTest->streamPath[pathIndex].tsbService);
        B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
        g_dvrTest->streamPath[pathIndex].tsbService = NULL;
        g_dvrTest->streamPath[pathIndex].dataInjectionService=NULL;
        BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
        if(dvrError != B_DVR_SUCCESS)
        {
            printf("\n Error in closing the timeshift service %d\n",pathIndex);
        }
		CuAssertTrue(tc, dvrError==B_DVR_SUCCESS);
    }
    else
    {
        printf("\n timeshift srvc not started");
    }

}

static B_DVR_ERROR OperationTSBDataInjectionStart(CuTest * tc, unsigned injectionType, unsigned pid, int pathIndex)
{
    B_DVR_ERROR rc =B_DVR_SUCCESS;
    B_DVR_DataInjectionServiceSettings settings;
    char dataFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    uint8_t pkt[188*30];
    int dataFileID;
    int readSize;

	BSTD_UNUSED(tc);
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


static void OperationTSBDataInjectionStop(CuTest * tc, int pathIndex)
{
	BSTD_UNUSED(tc);
    if(g_dvrTest->streamPath[pathIndex].tsbService){
        B_DVR_TSBService_InjectDataStop(g_dvrTest->streamPath[pathIndex].tsbService);
	}
    else{
        printf("TSB Service is not available\n");
	}
}


static void OperationTSBBufferPreallocate(CuTest * tc, int numAVPath)
{
    int index;
	BSTD_UNUSED(tc);
    
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
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",STREAM_TSB_SERVICE_PREFIX,index);
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
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",STREAM_TSB_SERVICE_PREFIX,index);
        rc = B_DVR_Manager_PrintSegmentedFileRecord(g_dvrTest->dvrManager,
                                                    &mediaNodeSettings);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in printing the tsb segments");
        }
    }
}


static void OperationTSBBufferDeallocate(CuTest * tc, int numAVPath)
{
    int index;
	BSTD_UNUSED(tc);
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
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",STREAM_TSB_SERVICE_PREFIX,index);
        rc = B_DVR_Manager_FreeSegmentedFileRecord(g_dvrTest->dvrManager,
                                                   &mediaNodeSettings);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in freeing the tsb segments");
        }
    }
}

static void OperationDataInject(void *context)
{
	CuTest * tc;
    DataInjectThreadParam *pParam;
    int index;
    unsigned injectionType;
    unsigned PAT_PID;
    unsigned PMT_PID;

    pParam = (DataInjectThreadParam*)context;

    injectionType = pParam->injectionType;
    PAT_PID = pParam->PAT_PID;
    PMT_PID = pParam->PMT_PID;
	tc = pParam->ptr;

    BKNI_Free(context);

   do {
       for (index = 0; index < MAX_TSBPATH; index++){
           /* Data Injection of PAT */
           OperationTSBDataInjectionStart(tc, injectionType, PAT_PID, index);
           while(B_Event_Wait(DataInjectionComplete[index], B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
           printf("\n$$$ Data Injection is complete for the index %d $$$\n", index);
           B_Event_Reset(DataInjectionComplete[index]);
           OperationTSBDataInjectionStop(tc, index);
           /* wait for 100 milisec */
           BKNI_Sleep(100);

           /* Data Injection of PMT */
           OperationTSBDataInjectionStart(tc, injectionType, PMT_PID, index);
           while(B_Event_Wait(DataInjectionComplete[index], B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
           printf("\n$$$ Data Injection is complete for the index %d $$$\n", index);
           B_Event_Reset(DataInjectionComplete[index]);
           OperationTSBDataInjectionStop(tc, index);
           /* wait for 100 milisec */
           BKNI_Sleep(100);
	   }
   }while(B_Event_Wait(DataInjectionStop, B_WAIT_NONE)!=B_ERROR_SUCCESS);
}

void InitHnMulStreamingpara(void)
{
    int  pathIndex;
   
    for (pathIndex = 0; pathIndex < MAX_TSBPATH; pathIndex++)
    {   
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.dstip[0] = '\0';   
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.interfaceName[0] = '\0';    
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.portnum[0] = '\0';    
    }
}

void CheckMulStreamTestName(char *buffer)
{
    char testpara[512];
    char strpara[512];
    int  paravalue = 0;
    int  n = 0, m = 0;
    int  pathIndex;

    n = sscanf(buffer, "%s\n", testpara);
    if (n > 0)
    {
       if(strncmp(testpara, "DestpIP",7) == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
             paravalue = atoi(&(testpara[7]));
             strcpy(g_dvrTest->streamPath[paravalue].hnMulStreamPara.dstip,strpara);
             printf("hnMulStreamPara.dstip = %s index=%d \n",g_dvrTest->streamPath[paravalue].hnMulStreamPara.dstip,paravalue);
          }
       }
       else if(strcmp(testpara, "LocalIFName") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
             for (pathIndex = 0; pathIndex < MAX_TSBPATH; pathIndex++)
             {
                strcpy(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.interfaceName,strpara);
                printf("hnMulStreamPara.interfaceName = %s index=%d\n",g_dvrTest->streamPath[pathIndex].hnMulStreamPara.interfaceName,pathIndex);
             }
          }
       }
       else if(strcmp(testpara, "PortNum") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
          {
             for (pathIndex = 0; pathIndex < MAX_TSBPATH; pathIndex++)
             {
                paravalue = atoi(strpara);
                if(pathIndex != 0)
                   paravalue += 2;
                BKNI_Snprintf(strpara,sizeof(strpara),"%d",paravalue);
                strcpy(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.portnum,strpara);
                printf("hnMulStreamPara.portnum = %s index=%d\n",g_dvrTest->streamPath[pathIndex].hnMulStreamPara.portnum,pathIndex);
             }
          }
       }
       else
          BDBG_WRN(("Parameter not recognized: %s\n", testpara));
    }
}

int HnMulStreamingLoadTxt(void)
{
    char buffer[512];
    FILE *fd;
    int rc = 0;

    printf("\n Load Hn Streaming input txt file \n");
    fd= fopen(hnmulstreamingtxt, "r");
    if(fd == NULL)
    {
        BDBG_MSG(("No input file found.\n"));
        rc = -1;
        return rc;
    }
	
    while (fgets(buffer, 512, fd) != NULL)
    {
      if (buffer[0] != '#')
      {
         CheckMulStreamTestName(buffer);
      }
    }
    fclose(fd);
    return rc;
}

int InitMulStreamUDPSocket(void)
{
   int pathIndex,rc=0;
   int  errorcode = 0;

   printf("\n Enter InitUDPSocket>>>> \n");  
   for (pathIndex = 0; pathIndex < MAX_TSBPATH; pathIndex++)
   {
   
     if ((g_dvrTest->streamPath[pathIndex].hnMulStreamPara.s=socket(AF_INET, SOCK_DGRAM, 0))<0)
     {
         printf(" socket open failed \n");   
         rc = -1;
         goto EndofInitUDPSocket;
     }
     g_dvrTest->streamPath[pathIndex].hnMulStreamPara.si_other.sin_family = AF_INET;
     g_dvrTest->streamPath[pathIndex].hnMulStreamPara.si_other.sin_port = htons(atoi(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.portnum));
     g_dvrTest->streamPath[pathIndex].hnMulStreamPara.si_other.sin_addr.s_addr = inet_addr(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.dstip);
     strncpy(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.ifr.ifr_name, g_dvrTest->streamPath[pathIndex].hnMulStreamPara.interfaceName, 
             sizeof(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.ifr.ifr_name)-1);

     if ((errorcode = setsockopt(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.s, SOL_SOCKET, SO_BINDTODEVICE, (void *)&(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.ifr), 
          sizeof(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.ifr))) < 0 )
     {
        printf("Set Bingparater error = %d!\n",errorcode);
        rc = -1;
        goto EndofInitUDPSocket;
     }
     else
     {
        printf("Set Bingparater Success = %d!\n",errorcode);
     }
     g_dvrTest->streamPath[pathIndex].dt = 0;
     g_dvrTest->streamPath[pathIndex].rate = 0;
   }
EndofInitUDPSocket:
   printf("\n Exit InitUDPSocket<<<<\n");  
   return rc;
}

void stream_MulThread_Created(int pathIndex)
{
    B_DVR_MediaFileSeekSettings mediaFileSeekSettings;
    off_t returnOffset;
    B_DVR_MediaFileOpenMode openMode;
    B_DVR_MediaFilePlayOpenSettings openSettings;

    printf("Enter g_dvrTest->multipleStreamCreated>>>> \n");

    openMode = eB_DVR_MediaFileOpenModeStreaming;
    openSettings.playpumpIndex = pathIndex;
    openSettings.volumeIndex = 0;
    sprintf(openSettings.subDir,"tsb");
    BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].programName,
            sizeof(g_dvrTest->streamPath[pathIndex].programName),
            "%s%d",STREAM_TSB_SERVICE_PREFIX,pathIndex);
    printf("TSBService programName %s",g_dvrTest->streamPath[pathIndex].programName);

    g_dvrTest->streamPath[pathIndex].mediaFile = B_DVR_MediaFile_Open(g_dvrTest->streamPath[pathIndex].programName,openMode,&openSettings);
    if(!g_dvrTest->streamPath[pathIndex].mediaFile)
    {
        printf("Error in opening mediaFile pathindex= %d",pathIndex);
    }

    BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].threadName,
            sizeof(g_dvrTest->streamPath[pathIndex].threadName),
            "%s%d%s","Multiple Streampath",pathIndex," Thread");

    g_dvrTest->streamPath[pathIndex].orgbuffer = BKNI_Malloc(bufSize + B_DVR_IO_BLOCK_SIZE);
    g_dvrTest->streamPath[pathIndex].buffer = (void*)B_DVR_IO_ALIGN_ROUND((unsigned long)g_dvrTest->streamPath[pathIndex].orgbuffer);

    if(!g_dvrTest->streamPath[pathIndex].orgbuffer)
    {
        printf("Error_buf_alloc\n");
    }

    B_DVR_MediaFile_Stats(g_dvrTest->streamPath[pathIndex].mediaFile,&(g_dvrTest->streamPath[pathIndex].mediaFileStats));
/*    printf("index= %d startOffset=%llu endOffset=%llu\n",pathIndex,g_dvrTest->streamPath[pathIndex].mediaFileStats.startOffset,g_dvrTest->streamPath[pathIndex].mediaFileStats.endOffset);*/
    g_dvrTest->streamPath[pathIndex].readOffset = mediaFileSeekSettings.mediaSeek.offset = g_dvrTest->streamPath[pathIndex].mediaFileStats.startOffset;
    mediaFileSeekSettings.seekmode = eB_DVR_MediaFileSeekModeByteOffsetBased;
    returnOffset = B_DVR_MediaFile_Seek(g_dvrTest->streamPath[pathIndex].mediaFile,&mediaFileSeekSettings);
    if(returnOffset < 0)
    {
        printf("Error seeking \n");
    }
    printf("Exit g_dvrTest->multipleStreamFCreate<<<\n");
}

void stream_MulThread_Destroy(int pathIndex)
{
    BKNI_Free(g_dvrTest->streamPath[pathIndex].orgbuffer);
    B_DVR_MediaFile_Close(g_dvrTest->streamPath[pathIndex].mediaFile);
}

static double difftime1(int pathIndex)
{
    double dt = 1000000.*(g_dvrTest->streamPath[pathIndex].stop.tv_sec - g_dvrTest->streamPath[pathIndex].start.tv_sec) +
                         (g_dvrTest->streamPath[pathIndex].stop.tv_usec - g_dvrTest->streamPath[pathIndex].start.tv_usec);
    return dt;
}

void stream_UDP(int pathIndex)
{

/*    printf("\n Enter stream_UDP>>>\n");*/

    g_dvrTest->streamPath[pathIndex].adjbuf = g_dvrTest->streamPath[pathIndex].buffer;
    if(g_dvrTest->streamPath[pathIndex].returnSize != UDPCHUNK )
    {
        printf("\n Byteread != PACKET_CHUNK*CHUNK_ELEMENT =%d\n",g_dvrTest->streamPath[pathIndex].returnSize);
    }

    g_dvrTest->streamPath[pathIndex].tempSize = g_dvrTest->streamPath[pathIndex].returnSize;
    while(g_dvrTest->streamPath[pathIndex].tempSize!= 0)
    {
       if (g_dvrTest->streamPath[pathIndex].tempSize <= PACKET_CHUNK)
           g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesToWrite = g_dvrTest->streamPath[pathIndex].tempSize;
       else
           g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesToWrite = PACKET_CHUNK;

       memset(&(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.iovec_set[0]), 0, sizeof(struct iovec)*SOCKET_IO_VECTOR_ELEMENT);
       memset(&g_dvrTest->streamPath[pathIndex].hnMulStreamPara.msg, 0, sizeof(struct msghdr));
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.iovec_set[0].iov_base = g_dvrTest->streamPath[pathIndex].adjbuf;
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.iovec_set[0].iov_len = g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesToWrite; 

       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.msg.msg_name = (struct sockaddr *)&(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.si_other);
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.msg.msg_namelen = sizeof(struct sockaddr_in);
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.msg.msg_iov = &(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.iovec_set[0]);
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.msg.msg_iovlen = SOCKET_IO_VECTOR_ELEMENT;
       if ((g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesWritten = sendmsg(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.s, 
            &(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.msg), 0)) <= 0) 
       {
           printf("\n ERROR writing to socket\n");
           g_dvrTest->streamPath[pathIndex].returnSize -= g_dvrTest->streamPath[pathIndex].tempSize;
           goto StreamToVLC_error_sendmsg;
       }
       if(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesWritten != g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesToWrite)
       {
           printf("\n !!!!!ERROR writing to socket bytewritten =%d, bytesToWrite=%d\n",g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesWritten,
                   g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesToWrite);
       }
       g_dvrTest->streamPath[pathIndex].tempSize -=g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesToWrite;
       g_dvrTest->streamPath[pathIndex].adjbuf+=g_dvrTest->streamPath[pathIndex].hnMulStreamPara.bytesToWrite;
       g_dvrTest->streamPath[pathIndex].hnMulStreamPara.counter++;
    }
   
StreamToVLC_error_sendmsg:
    return;
}

void stream_Action(void *context)
{
    int   pathIndex = 0;
    int   dataSize = 0;
    double testmaxrate = 30000*8; 

    printf("Enter stream_Action>>>> index= %d\n",pathIndex);
    B_DVR_MediaFile_Stats(g_dvrTest->streamPath[pathIndex].mediaFile,&(g_dvrTest->streamPath[pathIndex].mediaFileStats));

    while(dataSize < 100)
    {
        BKNI_Sleep(1000);
        B_DVR_MediaFile_Stats(g_dvrTest->streamPath[pathIndex].mediaFile,&(g_dvrTest->streamPath[pathIndex].mediaFileStats));
        dataSize = g_dvrTest->streamPath[pathIndex].mediaFileStats.endOffset - g_dvrTest->streamPath[pathIndex].mediaFileStats.startOffset;
        dataSize = dataSize/(1024*1024);
        printf("\n Index= %d DataSize in TSB buffer=%d (MB)\n",pathIndex,dataSize);
        if(pathIndex== 0)
           B_Event_Set(DataInjectionStop);
    }

    for(pathIndex = 0; pathIndex < MAX_TSBPATH;pathIndex++)
    {
        B_DVR_MediaFile_Stats(g_dvrTest->streamPath[pathIndex].mediaFile,&(g_dvrTest->streamPath[pathIndex].mediaFileStats));
    }
    pathIndex = 0;
    gettimeofday(&(g_dvrTest->streamPath[pathIndex].start), NULL);
    B_Mutex_Lock(g_dvrTest->glbstreamMutex);
    for(;(g_dvrTest->streamPath[pathIndex].readOffset<g_dvrTest->streamPath[pathIndex].mediaFileStats.endOffset)&&(g_dvrTest->streamPath[pathIndex].readSize < STREAMSIZE);
         g_dvrTest->streamPath[pathIndex].readOffset+=g_dvrTest->streamPath[pathIndex].returnSize) 
    {
         gettimeofday(&(g_dvrTest->streamPath[pathIndex].stop), NULL);
         g_dvrTest->streamPath[pathIndex].dt = difftime1(pathIndex);
         if(g_dvrTest->streamPath[pathIndex].zeroRateFlag == 1)
              g_dvrTest->streamPath[pathIndex].returnSize = g_dvrTest->streamPath[pathIndex].zeroBackupReturnSize;
         g_dvrTest->streamPath[pathIndex].rate = (8.*g_dvrTest->streamPath[pathIndex].returnSize*1000)/g_dvrTest->streamPath[pathIndex].dt;   /*rate unit is Kbit/s*/
         if (g_dvrTest->streamPath[pathIndex].rate > testmaxrate) 
         {
             g_dvrTest->streamPath[pathIndex].zeroBackupReturnSize = g_dvrTest->streamPath[pathIndex].returnSize;
             g_dvrTest->streamPath[pathIndex].returnSize = 0;
             g_dvrTest->streamPath[pathIndex].readSize += g_dvrTest->streamPath[pathIndex].returnSize;
             g_dvrTest->streamPath[pathIndex].zeroRateFlag = 1;
             printf("\n index=%d rate =%f > testmaxrate dt=%f \n",pathIndex,g_dvrTest->streamPath[pathIndex].rate,g_dvrTest->streamPath[pathIndex].dt);
             goto nextLoop;
         }
         memcpy(&(g_dvrTest->streamPath[pathIndex].start),&(g_dvrTest->streamPath[pathIndex].stop),sizeof(struct timeval));
 
         if(g_dvrTest->streamPath[pathIndex].zeroRateFlag == 1)
             g_dvrTest->streamPath[pathIndex].zeroRateFlag = 0;

         for(pathIndex = 0; pathIndex < MAX_TSBPATH;pathIndex++)
         {
            g_dvrTest->streamPath[pathIndex].ChunkLoopRead = CHUNKREADCNT;
            do 
            {
               if((g_dvrTest->streamPath[pathIndex].readOffset + UDPCHUNK) < g_dvrTest->streamPath[pathIndex].mediaFileStats.endOffset)
               {
                  g_dvrTest->streamPath[pathIndex].returnSize = B_DVR_MediaFile_Read(g_dvrTest->streamPath[pathIndex].mediaFile,g_dvrTest->streamPath[pathIndex].buffer,UDPCHUNK);
                  if(g_dvrTest->streamPath[pathIndex].returnSize <= 0)
                  {
                     printf("readsize <=0, close file !\n");
                     break;
                  }
                  else
                  {
                     stream_UDP(pathIndex);
                     g_dvrTest->streamPath[pathIndex].readSize += g_dvrTest->streamPath[pathIndex].returnSize;
                  }
               }
               else
               {
                  g_dvrTest->streamPath[pathIndex].returnSize = 0;
                  g_dvrTest->streamPath[pathIndex].readSize += g_dvrTest->streamPath[pathIndex].returnSize;
               }

               if((g_dvrTest->streamPath[pathIndex].ChunkLoopRead != 1) || (pathIndex != 0))
                  g_dvrTest->streamPath[pathIndex].readOffset+=g_dvrTest->streamPath[pathIndex].returnSize;
               g_dvrTest->streamPath[pathIndex].ChunkLoopRead--;

            }while(g_dvrTest->streamPath[pathIndex].ChunkLoopRead != 0);
         }
         pathIndex = 0;
nextLoop:
         if((g_dvrTest->streamPath[pathIndex].readSize >= STREAMSIZE/2)&&(g_dvrTest->streamPath[pathIndex].flag == 0))
         {
             g_dvrTest->streamPath[pathIndex].flag = 1;
             printf("\n indexpath=%d ,already read half of FULL-SIZE,totalread=%d (MB)\n",pathIndex,(g_dvrTest->streamPath[pathIndex].readSize/(1024*1024)));
         }

         for(pathIndex = 0; pathIndex < MAX_TSBPATH;pathIndex++)
         {
            B_DVR_MediaFile_Stats(g_dvrTest->streamPath[pathIndex].mediaFile,&(g_dvrTest->streamPath[pathIndex].mediaFileStats));
         }
         pathIndex = 0;
    }

    B_Mutex_Unlock(g_dvrTest->glbstreamMutex);

    printf("End of indexpath=%d write into UDP Socket\n",pathIndex);
    printf("\n Index= %d totalread=%d (MB)\n",pathIndex,g_dvrTest->streamPath[pathIndex].readSize/(1024*1024));
    close(g_dvrTest->streamPath[pathIndex].hnMulStreamPara.s);
    B_Mutex_Lock(g_dvrTest->glbstreamMutex);
    g_dvrTest->multipleStreamFlag = 0; 
    printf("Exit stream_Action<<<< index= %d multipleStreamFlag= %x\n\n",pathIndex,g_dvrTest->multipleStreamFlag);
    B_Event_Set(MultipleStreamReady);
    B_Mutex_Unlock(g_dvrTest->glbstreamMutex);

}

void stream_MulThread_Active(void)
{
    printf("Enter g_dvrTest->MulThread_Active >>>>MultipleThreadFlag = %x\n",g_dvrTest->multipleStreamFlag);
    BKNI_Snprintf(g_dvrTest->threadName,
            sizeof(g_dvrTest->threadName),
            "%s%s","Multiple Streampath"," Thread");

    g_dvrTest->threadID = B_Thread_Create(g_dvrTest->threadName, stream_Action, NULL, NULL);
    if (!g_dvrTest->threadID) 
    {
         printf("Error in creating a stream thread \n");
    }
    g_dvrTest->multipleStreamFlag |= 1;
    printf("Exit g_dvrTest->MulThread_Active <<<<<MultipleThreadFlag = %x\n",g_dvrTest->multipleStreamFlag);
}

void hn8TSBTo8StreamNet2PBInjectionTest(CuTest * tc)
{
    int index,returnValue; 
    DataInjectThreadParam *pDataInjectionParam;
    B_ThreadHandle DataInjectionThread;

    InitHnMulStreamingpara();
    returnValue = HnMulStreamingLoadTxt();
    if (returnValue != 0)
    {
        printf("\n Missing input file!!!\n");
        return;
    }

    returnValue = InitMulStreamUDPSocket();
    if (returnValue != 0)
    {
        printf("\n Init UDP Socket Error!!!\n");
        return;
    }

    TSBServiceReady = B_Event_Create(NULL);
    MultipleStreamReady = B_Event_Create(NULL);
    DataInjectionStop = B_Event_Create(NULL);
    
    for (index=0; index<MAX_TSBPATH; index++)
    {
        DataInjectionComplete[index] = B_Event_Create(NULL);
    }

    printf("max stream path = %d\n", MAX_STREAM_PATH);
    printf("configured stream path = %d\n", MAX_TSBPATH);


    g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)DvrTest_MulCallback;
    OperationTSBBufferPreallocate(tc, MAX_TSBPATH);

    for (index = 0; index < MAX_TSBPATH; index++)
    {
        if(!g_dvrTest->streamPath[index].tsbService)
        {
            printf("\nTSB Service for the path index %d is not started\n", index);
            OperationTSBServiceStart(tc, index);
            printf("Starting TSB Service for the path index %d...\n", index);
        }
    }

    /* wait until TSB Service becomes ready */
    while(B_Event_Wait(TSBServiceReady, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);

    /* starting Data Injection */
    pDataInjectionParam = BKNI_Malloc(sizeof(DataInjectThreadParam));
    if (!pDataInjectionParam)
    {
        printf("Error in creating a Data InjectionThreadParam\n");
    }
    pDataInjectionParam->injectionType = eB_DVR_DataInjectionServiceTypeRawData;
    pDataInjectionParam->PAT_PID = 0x0;
    pDataInjectionParam->PMT_PID = 0x3f;
    pDataInjectionParam->ptr = tc;

    DataInjectionThread = B_Thread_Create("Data Injection Thread", OperationDataInject, (void*) pDataInjectionParam, NULL);

    BKNI_Sleep(DURATION);

    /* starting create multiple threads for streamingtofile */
    for (index = 0; index < MAX_TSBPATH; index++)
    {
        printf("Create Mutiple thread index = %d \n", index);
        stream_MulThread_Created(index);
    }

    g_dvrTest->glbstreamMutex = B_Mutex_Create(NULL);

    BKNI_Sleep(DURATION);
    B_Mutex_Lock(g_dvrTest->glbstreamMutex);

    stream_MulThread_Active();

    B_Mutex_Unlock(g_dvrTest->glbstreamMutex);

cntWaitEvent:
    while(B_Event_Wait(MultipleStreamReady, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
    if(g_dvrTest->multipleStreamFlag != 0)
    {
        printf("Need to reset MultipleStreamReady --mutiple thread flag=%x\n",g_dvrTest->multipleStreamFlag);
        B_Event_Reset(MultipleStreamReady);
        goto cntWaitEvent;
    }

    printf("End of mutiple thread flag=%d\n",g_dvrTest->multipleStreamFlag);
    B_Mutex_Lock(g_dvrTest->glbstreamMutex);

    B_Thread_Destroy(g_dvrTest->threadID);

    for (index = 0; index < MAX_TSBPATH; index++)
    {
        printf("Destroy Mutiple thread index = %d \n", index);
        stream_MulThread_Destroy(index);
    }

    B_Mutex_Unlock(g_dvrTest->glbstreamMutex);

    B_Mutex_Destroy(g_dvrTest->glbstreamMutex);

    B_Thread_Destroy(DataInjectionThread);

    B_Event_Destroy(DataInjectionStop);
    B_Event_Destroy(MultipleStreamReady);
    B_Event_Destroy(TSBServiceReady);

    printf("\n Stopping all TSB Services");
    for(index=0;index< MAX_TSBPATH; index++)
    {
        if(g_dvrTest->streamPath[index].tsbService)
        {
            B_DVR_TSBServiceStatus tsbStatus;

            B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService, &tsbStatus);
            /* tsbConversion is false */
            if(!tsbStatus.tsbCoversion)
            {
                printf("\n Stopping TSB Conversion Service index %d", index);
                OperationTSBServiceStop(tc, index);
            }
        }
    }

    OperationTSBBufferDeallocate(tc, MAX_TSBPATH);

}

