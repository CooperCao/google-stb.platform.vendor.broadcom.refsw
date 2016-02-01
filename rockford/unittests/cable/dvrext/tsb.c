/******************************************************************************
 *    (c)2013-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
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
 * *****************************************************************************/

/*
 *  tsb:
 *  App demonstrates time shift recording and playback.
 *  TSB is in segmented file format.
 *  TSB source is QAM only for now. PSI packets are also inserted into TSB.
 *  TSB playback supports various trick modes.
 *  Switching between TSB and live is supported seamlessly based on user
 *  inputs. tsb application assumes that segmented file system is already
 *  created using mksfs application.
 *  
 *  usage
 *  ./nexus tsb
 */

#include "util.h"
#define TSB_NAME "tsb_0"   /* meta data name for a TSB instance */
#define TSB_DIRECTORY "tsb" /* meta data sub directory for a tsb meta data */

/* 
 *  Channel Info needs to be modified based on the
 *  user test environment.
 */ 
channelInfo_t channelInfo =
{
    NEXUS_FrontendQamAnnex_eB,
    NEXUS_FrontendQamMode_e64,
    5056941,
    429000000,
    0x11,0x14,0x11,
    NEXUS_VideoCodec_eMpeg2,
    NEXUS_AudioCodec_eAc3
};

typedef struct tsb_ctx_t
{
    NEXUS_PlatformConfiguration platformconfig;
    NEXUS_ParserBand parserBand;
    NEXUS_FrontendHandle frontend;
    BKNI_EventHandle lockChangedEvent;
    B_DVR_TSBServiceHandle tsbService;
    B_DVR_ServiceCallback serviceCallback;
    B_EventHandle tsbStarted;
    psiInfo_t psiInfo;
    B_DVR_ManagerHandle manager;
    B_DVR_MediaStorageHandle storage;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    decode_ctx_t live_decode_ctx;
    decode_ctx_t play_decode_ctx;
    B_MutexHandle mutex;
    B_MutexHandle schedulerMutex;
    B_SchedulerHandle scheduler;
    B_ThreadHandle schedulerThread;
}tsb_ctx_t;

tsb_ctx_t *tsb_ctx = NULL;

void print_tsb_menu(void)
{
    printf("\n***********************************");
    printf("\n************TSB Operations*********");
    printf("\n***********************************\n");
    printf("\n\t  0. quit ");
    printf("\n\t  1. pause ");
    printf("\n\t  2. play ");
    printf("\n\t  3. rewind");
    printf("\n\t  4. fast-forward");
    printf("\n\t  5. slow motion forward");
    printf("\n\t  6. slow motion reverse");
    printf("\n\t  7. frame advance");
    printf("\n\t  8. frame reverse");
    printf("\n\t  9. forwardjump");
    printf("\n\t 10. reversejump");
    printf("\n\t 11. weak signal handling");
    printf("\n************ ***********************\n");
    return;
}
void print_tsb_status(B_DVR_TSBServiceHandle tsbService)
{
    B_DVR_TSBServiceStatus tsbServiceStatus;
    unsigned long tsb_window =0;
    B_DVR_TSBService_GetStatus(tsbService,&tsbServiceStatus);
    printf("\n TSB Rec Status\n");
    printf("\n st:et %lu ms:%lu ms \n",
           tsbServiceStatus.tsbRecStartTime,tsbServiceStatus.tsbRecEndTime);
    tsb_window =((tsbServiceStatus.tsbRecEndTime-tsbServiceStatus.tsbRecStartTime)/(1000));
    printf("\n tsb window:%lu secs",tsb_window);
    if(tsbServiceStatus.tsbPlayback) 
    {
        printf("\n TSB Play Status\n");
        printf("\n st:ct:et %lu ms:%lu ms:%lu ms \n",
               tsbServiceStatus.tsbRecStartTime,
               tsbServiceStatus.tsbCurrentPlayTime,
               tsbServiceStatus.tsbRecEndTime);
    }
    return;
}

void tsb_switch_to_play(tsb_ctx_t *tsb_ctx)
{
    if(tsb_ctx->live_decode_ctx.started)
    { 
        B_DVR_TSBServiceStatus tsbServiceStatus;
        B_DVR_OperationSettings operationSettings;
        app_decode_stop(&tsb_ctx->live_decode_ctx);
        app_decode_start(&tsb_ctx->play_decode_ctx);
        B_DVR_TSBService_GetStatus(tsb_ctx->tsbService,&tsbServiceStatus);
        operationSettings.operation = eB_DVR_OperationTSBPlayStart;
        operationSettings.seekTime = tsbServiceStatus.tsbRecEndTime;
        B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
    }
    return;
}
void tsb_switch_to_live(tsb_ctx_t *tsb_ctx)
{
    B_DVR_OperationSettings operationSettings;
    if(tsb_ctx->play_decode_ctx.started)
    {   
        operationSettings.operation = eB_DVR_OperationTSBPlayStop;
        B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
        app_decode_stop(&tsb_ctx->play_decode_ctx);
        app_decode_start(&tsb_ctx->live_decode_ctx);
    }
    return;
}
void tsb_scheduler(void * param)
{
    BSTD_UNUSED(param);
    B_Scheduler_Run(tsb_ctx->scheduler);
    return;
}

void tsb_callback(void *context, int index,B_DVR_Event event,B_DVR_Service service)
{
    tsb_ctx_t *tsb_ctx = (tsb_ctx_t *)context;
    BSTD_UNUSED(index);
    BSTD_UNUSED(service);
    B_Mutex_Lock(tsb_ctx->mutex);
    switch(event)
    {
        case eB_DVR_EventDataInjectionCompleted:
        {
          
           tsb_ctx->psiInfo.timer = B_Scheduler_StartTimer(tsb_ctx->scheduler,
                                                           tsb_ctx->schedulerMutex,
                                                           INJECT_INTERVAL,
                                                           app_inject_psi,
                                                           (void*)&tsb_ctx->psiInfo);
        }
        break;
    case eB_DVR_EventStartOfRecording:
        {
            B_Event_Set(tsb_ctx->tsbStarted);
            printf("\n TSB buffering started \n");
            print_tsb_menu();
        }
        break;
    case eB_DVR_EventEndOfRecording:
        {
            printf("\n TSB buffering stopped \n");
            print_tsb_menu();
        }
        break;
    case eB_DVR_EventHDStreamRecording:
        {
            printf("\n TSB stream source is HD \n");
            print_tsb_menu();
        }
        break;
    case eB_DVR_EventSDStreamRecording:
        {
            printf("\n TSB stream source is SD \n");
            print_tsb_menu();
        }
        break;
    case eB_DVR_EventMediaProbed:
        {
            printf("\n TSB is probed and stream parameters are available \n");
            print_tsb_menu();
        }
        break;
    case eB_DVR_EventStartOfPlayback:
        {
            unsigned skip_interval = 0; /* ms */
            B_DVR_OperationSettings operationSettings;
            B_DVR_TSBServiceStatus tsbServiceStatus;
            B_DVR_ERROR rc = B_DVR_SUCCESS;
            printf("\n BOF TSB \n");
            print_tsb_status(tsb_ctx->tsbService);
            B_DVR_TSBService_GetStatus(tsb_ctx->tsbService,&tsbServiceStatus);
            tsbServiceStatus.tsbRecStartTime += skip_interval;
            rc = B_DVR_TSBService_GetIFrameTimeStamp(tsb_ctx->tsbService,
                                                &tsbServiceStatus.tsbRecStartTime);
            if(rc == B_DVR_SUCCESS) 
            {
                operationSettings.operation = eB_DVR_OperationSeek;
                operationSettings.seekTime = tsbServiceStatus.tsbRecStartTime;
                B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
                operationSettings.operation = eB_DVR_OperationPlay;
                B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
            }
            else
            {
                printf("\n live decode started -> not enough data to start TSB playback \n");
                tsb_switch_to_live(tsb_ctx);
            }
            print_tsb_menu();
        }
        break;
    case eB_DVR_EventEndOfPlayback:
        {
            printf("\n EOF TSB \n");
            print_tsb_status(tsb_ctx->tsbService);
            printf("\n live decode started \n");
            tsb_switch_to_live(tsb_ctx);
            print_tsb_menu();
        }
        break;
    case eB_DVR_EventOverFlow:
        {
           printf("\n TSB record overflow \n");
        }
        break;
    default:
        printf("\n invalid event \n");
    }
    B_Mutex_Unlock(tsb_ctx->mutex);
    return;
}

int main(void)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    char programName[B_DVR_MAX_FILE_NAME_LENGTH]=TSB_NAME;
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH]=TSB_DIRECTORY;
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    B_DVR_MediaStorageStatus storageStatus;
    B_DVR_Media media;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    B_DVR_TSBServiceInputEsStream inputEsStream;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    B_DVR_TSBServiceRequest tsbServiceRequest;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    B_DVR_DataInjectionServiceOpenSettings injectOpenSettings;
    B_DVR_DataInjectionServiceSettings injectSettings;
    NEXUS_FrontendAcquireSettings acquireSettings;
    NEXUS_PidChannelSettings pidChannelSettings; 
    NEXUS_DisplaySettings displaySettings;
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputSettings hdmiSettings;
    #endif
    NEXUS_StcChannelSettings stcSettings;
    B_DVR_MediaStorageType mediaStorageType;
    
    mediaStorageType = get_storage_device_type();
    if(mediaStorageType == eB_DVR_MediaStorageTypeMax) 
    {
        goto error;
    }

    /* 
     * allocate the app context
     */
    tsb_ctx = malloc(sizeof(tsb_ctx_t));
    assert(tsb_ctx);
    memset(tsb_ctx,0,sizeof(tsb_ctx_t));

    /* 
     *  initialized the platform
     */
    rc = NEXUS_Platform_Init(NULL);
    if(rc) 
    {
        printf("\n NEXUS_Platform_Init failed \n ");
        free(tsb_ctx);
        goto error;
    }

    /* 
     *  get platform configuration
     */
    NEXUS_Platform_GetConfiguration(&tsb_ctx->platformconfig);

    /* 
     * acquire the frontend with qam capability
     */
    NEXUS_Frontend_GetDefaultAcquireSettings(&acquireSettings);
    acquireSettings.capabilities.qam = true;
    tsb_ctx->frontend = NEXUS_Frontend_Acquire(&acquireSettings);
    if(!tsb_ctx->frontend) 
    {
        printf("\n No QAM frontend found");
        printf("\n app works with QAM source only at present");
        printf("\n use a platform with QAM source\n");
        NEXUS_Platform_Uninit();
        free(tsb_ctx);
        goto error;
    }

    /* 
     * create an event to receive the locked event from frontend
     */
    BKNI_CreateEvent(&tsb_ctx->lockChangedEvent);
    assert(tsb_ctx->lockChangedEvent);

    /* 
     * tune to a channel channel
     */
    tsb_ctx->parserBand = app_tune_channel(tsb_ctx->frontend,
                                           tsb_ctx->lockChangedEvent,
                                           &channelInfo);

    if(tsb_ctx->parserBand == NEXUS_ParserBand_eInvalid)
    {
        printf("\n check and change channelInfo parameters for tuning \n");
        BKNI_DestroyEvent(tsb_ctx->lockChangedEvent);
        NEXUS_Platform_Uninit();
        free(tsb_ctx);
        goto error;
    }

    /* 
     * open media storage
     */
    mediaStorageOpenSettings.storageType = mediaStorageType;
    tsb_ctx->storage = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);
    assert(tsb_ctx->storage);
    
    if(mediaStorageType == eB_DVR_MediaStorageTypeLoopDevice) 
    {
        unsigned volumeIndex=0;
        B_DVR_MediaStorageRegisterVolumeSettings registerSettings;
        memset(&registerSettings,0,sizeof(registerSettings));
        sprintf(registerSettings.device,VSFS_DEVICE);
        registerSettings.startSec = 0;
        registerSettings.length = VSFS_SIZE;
        B_DVR_MediaStorage_RegisterVolume(tsb_ctx->storage,&registerSettings,&volumeIndex);
    }
    /* 
     * check if the media storage is in segmented file system format
     */
    B_DVR_MediaStorage_GetStatus(tsb_ctx->storage,&storageStatus);
    if(!storageStatus.numRegisteredVolumes || 
       !storageStatus.volumeInfo[STORAGE_VOLUME_INDEX].formatted) 
    {
        printf("\n no storage device registered and formatted");
        if(mediaStorageType == eB_DVR_MediaStorageTypeLoopDevice) 
        {
            printf("\n use mkvsfs app to register and format a storage device \n");
        }
        else
        {
            printf("\n use mksfs app to register and format a storage device \n");
        }
        B_DVR_MediaStorage_Close(tsb_ctx->storage);
        NEXUS_Platform_Uninit();
        free(tsb_ctx);
        goto error;
    }
    /* 
     *  mount media storage -> media, nav and metadata partitions
     */
    rc = B_DVR_MediaStorage_MountVolume(tsb_ctx->storage, STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    /* 
     *  initialize the DVR manager
     */
    tsb_ctx->manager = B_DVR_Manager_Init(NULL);
    assert(tsb_ctx->manager);

    /* 
     *  create the list of recording if available
     */
    rc = B_DVR_Manager_CreateMediaNodeList(tsb_ctx->manager,STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    /* 
     * create app scheduler mutex
     */
    tsb_ctx->schedulerMutex = B_Mutex_Create(NULL);
    assert(tsb_ctx->schedulerMutex);

    /* 
     * create app scheduler
     */
    tsb_ctx->scheduler = B_Scheduler_Create(NULL);
    assert(tsb_ctx->scheduler);

    /* 
     *  create the scheduler thread
     */
    tsb_ctx->schedulerThread = B_Thread_Create("tsb",tsb_scheduler,tsb_ctx, NULL);
    assert(tsb_ctx->schedulerThread);

    tsb_ctx->mutex = B_Mutex_Create(NULL);
    assert(tsb_ctx->mutex);
    
    /* display open */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    tsb_ctx->display = NEXUS_Display_Open(0, &displaySettings);
    assert(tsb_ctx->display);

    /* configure HDMI output */
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(tsb_ctx->display, 
                            NEXUS_HdmiOutput_GetVideoConnector(tsb_ctx->platformconfig.outputs.hdmi[0]));
    NEXUS_HdmiOutput_GetSettings(tsb_ctx->platformconfig.outputs.hdmi[0], &hdmiSettings);
    hdmiSettings.hotplugCallback.callback = app_hotplug_callback;
    hdmiSettings.hotplugCallback.context = tsb_ctx->platformconfig.outputs.hdmi[0];
    hdmiSettings.hotplugCallback.param = (int)tsb_ctx->display;
    NEXUS_HdmiOutput_SetSettings(tsb_ctx->platformconfig.outputs.hdmi[0], &hdmiSettings);
    #endif

    /* 
     * open STC channel and share it between live and TSB playback mode 
     * since live and TSB playback are mutually exclusive 
     */ 
    NEXUS_StcChannel_GetDefaultSettings(0,&stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    tsb_ctx->live_decode_ctx.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    tsb_ctx->play_decode_ctx.stcChannel = tsb_ctx->live_decode_ctx.stcChannel;
    assert(tsb_ctx->live_decode_ctx.stcChannel);

    /* 
     *  open video window and share it between live and tsb mode since live and
     *  tsb mode are mutually exclusive
     */
    tsb_ctx->window = NEXUS_VideoWindow_Open(tsb_ctx->display,0);
    assert(tsb_ctx->window);

    /* 
     *  open video decoder and share it between live and tsb mode since live and
     *  tsb mode are mutually exclusive
     */
    tsb_ctx->live_decode_ctx.videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    tsb_ctx->play_decode_ctx.videoDecoder = tsb_ctx->live_decode_ctx.videoDecoder;
    assert(tsb_ctx->live_decode_ctx.videoDecoder);

    /* 
     *  add video decoder as input to the video window
     */
    NEXUS_VideoWindow_AddInput(tsb_ctx->window,
                               NEXUS_VideoDecoder_GetConnector(tsb_ctx->live_decode_ctx.videoDecoder));


    /* 
     *  open audio decoder and share it between live and tsb mode.
     */
    tsb_ctx->live_decode_ctx.audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    tsb_ctx->play_decode_ctx.audioDecoder = tsb_ctx->live_decode_ctx.audioDecoder;
    assert(tsb_ctx->live_decode_ctx.audioDecoder);

    /* 
     *  add audio input for the HDMI audio output
     */
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(tsb_ctx->platformconfig.outputs.hdmi[0]),
                               NEXUS_AudioDecoder_GetConnector(tsb_ctx->live_decode_ctx.audioDecoder,
                                                               NEXUS_AudioDecoderConnectorType_eStereo));
    #endif

    /*
     *  Open video PID channel for live mode
     */
    tsb_ctx->live_decode_ctx.vPidChannel = NEXUS_PidChannel_Open(tsb_ctx->parserBand,channelInfo.vPid,NULL);
    assert(tsb_ctx->live_decode_ctx.vPidChannel);

    /*
     *  Open audio PID channel for live mode
     */
    tsb_ctx->live_decode_ctx.aPidChannel  = NEXUS_PidChannel_Open(tsb_ctx->parserBand,channelInfo.aPid,NULL);
    assert(tsb_ctx->live_decode_ctx.aPidChannel);
    tsb_ctx->live_decode_ctx.playback = false;
    
    /*
     *  open PCR PID channel if different from both audio and video PID channels.
     */
    if(channelInfo.vPid == channelInfo.pcrPid) 
    {
      tsb_ctx->live_decode_ctx.pcrPidChannel = tsb_ctx->live_decode_ctx.vPidChannel;
    }
    else
    { 
        if(channelInfo.aPid == channelInfo.pcrPid) 
        {
            tsb_ctx->live_decode_ctx.pcrPidChannel = tsb_ctx->live_decode_ctx.aPidChannel;
        }
        else
        {
            tsb_ctx->live_decode_ctx.pcrPidChannel  = NEXUS_PidChannel_Open(tsb_ctx->parserBand,channelInfo.pcrPid,NULL);
            assert(tsb_ctx->live_decode_ctx.pcrPidChannel);
        }
    }

    /*
     *  populate video program settings for video decoder
     */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&tsb_ctx->live_decode_ctx.videoProgram);
    tsb_ctx->live_decode_ctx.videoProgram.codec = channelInfo.vCodec;
    tsb_ctx->live_decode_ctx.videoProgram.pidChannel = tsb_ctx->live_decode_ctx.vPidChannel;
    tsb_ctx->live_decode_ctx.videoProgram.stcChannel = tsb_ctx->live_decode_ctx.stcChannel;

    /*
     *  populate audio program settings for audio decoder
     */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&tsb_ctx->live_decode_ctx.audioProgram);
    tsb_ctx->live_decode_ctx.audioProgram.codec = channelInfo.aCodec;
    tsb_ctx->live_decode_ctx.audioProgram.pidChannel = tsb_ctx->live_decode_ctx.aPidChannel;
    tsb_ctx->live_decode_ctx.audioProgram.stcChannel = tsb_ctx->live_decode_ctx.stcChannel;;
    app_decode_start(&tsb_ctx->live_decode_ctx);

    /* 
     *  allocate TSB buffers -> reserves n number of media and nav file segments
     *  to be used for TSB and also creates the .info file for TSB.
     */
    memset((void *)&mediaNodeSettings,0,sizeof(mediaNodeSettings));
    mediaNodeSettings.programName = programName;
    mediaNodeSettings.subDir = subDir;
    mediaNodeSettings.volumeIndex=STORAGE_VOLUME_INDEX;
    B_DVR_Manager_AllocSegmentedFileRecord(tsb_ctx->manager,
                                           &mediaNodeSettings,
                                           MAX_TSB_BUFFERS);
    
    tsb_ctx->tsbStarted = B_Event_Create(NULL);
    assert(tsb_ctx->tsbStarted);
    /* 
     * populate TSBService request parameters
     */
    memset((void *)&tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
    strcpy(tsbServiceRequest.programName,programName);
    strcpy(tsbServiceRequest.subDir,subDir);
    tsbServiceRequest.input = eB_DVR_TSBServiceInputQam;
    tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
    tsbServiceRequest.maxTSBTime = MAX_TSB_TIME;
    tsbServiceRequest.recpumpIndex = NEXUS_ANY_ID;
    tsbServiceRequest.playpumpIndex = NEXUS_ANY_ID;
    tsbServiceRequest.volumeIndex = STORAGE_VOLUME_INDEX;

    /* 
     * open a TSB service instance
     */
    tsb_ctx->tsbService = B_DVR_TSBService_Open(&tsbServiceRequest);
    assert(tsb_ctx->tsbService);

    /* 
     * install a callback from TSB service to receive asynchronous events
     * from a TSB service instance
     */
    tsb_ctx->serviceCallback = (B_DVR_ServiceCallback)tsb_callback;
    B_DVR_TSBService_InstallCallback(tsb_ctx->tsbService,
                                     tsb_ctx->serviceCallback,
                                     (void*)tsb_ctx);

    /* 
     * populate data injection settings
     */
    memset((void *)&injectOpenSettings,0,sizeof(injectOpenSettings));
    injectOpenSettings.fifoSize = 30*188; 
    tsb_ctx->psiInfo.injectService  = B_DVR_DataInjectionService_Open(&injectOpenSettings);
    assert(tsb_ctx->psiInfo.injectService);

    /* 
     * install a callback to receive events from data injection service
     */
    B_DVR_DataInjectionService_InstallCallback(tsb_ctx->psiInfo.injectService,
                                               tsb_ctx->serviceCallback,
                                               (void*)tsb_ctx);

    /*
     * allocate a PID channel for data injection.
     */
    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    pidChannelSettings.pidChannelIndex = (NEXUS_NUM_PID_CHANNELS-2);
    pidChannelSettings.enabled = false;
    tsb_ctx->psiInfo.injectChannel = NEXUS_PidChannel_Open(tsb_ctx->parserBand,
                                                           0x1ffe,
                                                           &pidChannelSettings);
    assert(tsb_ctx->psiInfo.injectChannel);

    /*
     * associate a PID channel with data injection service 
     * and also set the type of injection (transport packet or PAT/PMT data) 
     */
    memset(&injectSettings,0,sizeof(injectSettings));
    injectSettings.dataInjectionServiceType = eB_DVR_DataInjectionServiceTypeRawData;
    injectSettings.pidChannel = tsb_ctx->psiInfo.injectChannel;
    B_DVR_DataInjectionService_SetSettings(tsb_ctx->psiInfo.injectService,&injectSettings);

    /*
     * associate a data injection service instance and an inBand parser 
     * to a tsb service instance 
     */
    memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
    tsbServiceSettings.dataInjectionService = tsb_ctx->psiInfo.injectService;
    tsbServiceSettings.parserBand = tsb_ctx->parserBand;
    B_DVR_TSBService_SetSettings(tsb_ctx->tsbService,&tsbServiceSettings);

    /*
     * add video es stream info to a TSB service instance
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.vPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    inputEsStream.esStreamInfo.codec.videoCodec = channelInfo.vCodec;
    inputEsStream.pidChannel = NULL;
    B_DVR_TSBService_AddInputEsStream(tsb_ctx->tsbService,&inputEsStream);

    /*
     * add audio es stream info to a TSB service instance
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.aPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    inputEsStream.esStreamInfo.codec.audioCodec = channelInfo.aCodec;
    B_DVR_TSBService_AddInputEsStream(tsb_ctx->tsbService,&inputEsStream);

    /*
     * add data stream info to a TSB service instance
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.pcrPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
    B_DVR_TSBService_AddInputEsStream(tsb_ctx->tsbService,&inputEsStream);

    /*
     * create PAT and PMT packets based on A/V and data PID info
     */
    app_create_psi(&tsb_ctx->psiInfo,
                   channelInfo.pcrPid,
                   channelInfo.vPid,
                   channelInfo.aPid,
                   channelInfo.vCodec,
                   channelInfo.aCodec);

    /*
     *  start TSB recording
     */
    rc = B_DVR_TSBService_Start(tsb_ctx->tsbService,NULL);
    assert(rc == 0);

    /*
     * kick of the PSI injection for TSB recording
     */ 
    tsb_ctx->psiInfo.tsbService = tsb_ctx->tsbService;
    tsb_ctx->psiInfo.psiStop = false;
    rc = B_DVR_TSBService_InjectDataStart(tsb_ctx->tsbService,
                                          (void *)&tsb_ctx->psiInfo.pat[1],
                                          188);
    assert(rc == 0);
    tsb_ctx->psiInfo.injectInProgress = true;

    /*
     * wait for start of recording event i.e. 1st I frame picture availability.
     */ 
    B_Event_Wait(tsb_ctx->tsbStarted, 1000);
    
    /*
     *  pass the user provided A/V decode handles and STC channel handle to the
     *  TSB service to trick mode support.
     */ 
    rc = B_DVR_TSBService_GetSettings(tsb_ctx->tsbService,&tsbServiceSettings);
    assert(rc==0);
    tsbServiceSettings.tsbPlaybackSettings.videoDecoder[0]= tsb_ctx->play_decode_ctx.videoDecoder;
    tsbServiceSettings.tsbPlaybackSettings.audioDecoder[0]= tsb_ctx->play_decode_ctx.audioDecoder;
    tsbServiceSettings.tsbPlaybackSettings.stcChannel = tsb_ctx->play_decode_ctx.stcChannel;
    rc = B_DVR_TSBService_SetSettings(tsb_ctx->tsbService,&tsbServiceSettings);
    assert(rc==0);

    /*
     * Open TSB playback PID channels based on the PIDs getting recorded 
     */
    rc = B_DVR_Manager_GetMediaNode(tsb_ctx->manager,&mediaNodeSettings,&media);
    assert(rc==0);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = media.esStreamInfo[0].codec.videoCodec;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = tsb_ctx->play_decode_ctx.videoDecoder;

    /*
     * Assuming zero index for video. pidType is available in esStreamInfo[0].pidType 
     * and also total pid count is available from media.esStreamCount
     */

    tsb_ctx->play_decode_ctx.vPidChannel = 
        NEXUS_Playback_OpenPidChannel(tsbServiceSettings.tsbPlaybackSettings.playback,
                                      media.esStreamInfo[0].pid,
                                      &playbackPidSettings);
    assert(tsb_ctx->play_decode_ctx.vPidChannel);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = tsb_ctx->play_decode_ctx.audioDecoder;
    /* Assuming index 1 for audio */
    tsb_ctx->play_decode_ctx.aPidChannel = 
        NEXUS_Playback_OpenPidChannel(tsbServiceSettings.tsbPlaybackSettings.playback,
                                      media.esStreamInfo[1].pid,
                                      &playbackPidSettings);
    assert(tsb_ctx->play_decode_ctx.aPidChannel);

    /*
     * populate video program settings for TSB playback
     */
    tsb_ctx->play_decode_ctx.playback = true;
    NEXUS_VideoDecoder_GetDefaultStartSettings(&tsb_ctx->play_decode_ctx.videoProgram);
    tsb_ctx->play_decode_ctx.videoProgram.codec = channelInfo.vCodec;
    tsb_ctx->play_decode_ctx.videoProgram.pidChannel = tsb_ctx->play_decode_ctx.vPidChannel;
    tsb_ctx->play_decode_ctx.videoProgram.stcChannel = tsb_ctx->play_decode_ctx.stcChannel;

    /*
     * populate audio program settings for TSB playback
     */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&tsb_ctx->play_decode_ctx.audioProgram);
    tsb_ctx->play_decode_ctx.audioProgram.codec = channelInfo.aCodec;
    tsb_ctx->play_decode_ctx.audioProgram.pidChannel = tsb_ctx->play_decode_ctx.aPidChannel;
    tsb_ctx->play_decode_ctx.audioProgram.stcChannel = tsb_ctx->play_decode_ctx.stcChannel;

    /*
     * various operations for TSB playback
     */
    while(1)
    {   
        unsigned operation;
        B_DVR_OperationSettings operationSettings;
        printf("\n enter the operation => ");
        fflush(stdin);
        scanf("%u",&operation);
        print_tsb_status(tsb_ctx->tsbService);
        if(operation == eQuit) 
        {
            break;
        }
        print_tsb_menu();
        B_Mutex_Lock(tsb_ctx->mutex);
        switch(operation) 
        {
            case ePause:
            {
                tsb_switch_to_play(tsb_ctx);
                operationSettings.operation = eB_DVR_OperationPause; 
                B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);   
            }
            break;
        case ePlay:
            {
                if(tsb_ctx->play_decode_ctx.started) 
                {
                    operationSettings.operation = eB_DVR_OperationPlay; 
                    B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);   
                }
                else
                {
                    printf("\n play op in live mode is a no-operation \n");
                }
            }
            break;
        case eRewind:
            {
                tsb_switch_to_play(tsb_ctx);
                operationSettings.operation = eB_DVR_OperationFastRewind;
                operationSettings.operationSpeed = 4;
                B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
            }
            break;
        case eFastForward:
            {
                if(!tsb_ctx->live_decode_ctx.started) 
                {
                    operationSettings.operation = eB_DVR_OperationFastForward;
                    operationSettings.operationSpeed = 4;
                    B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
                }
                else
                {
                    printf("\n forward trick mode in live mode is a no-operation \n");
                }
            }
            break;
        case eSlowMotionForward:
            {
                if(!tsb_ctx->live_decode_ctx.started) 
                {
                    operationSettings.operation = eB_DVR_OperationSlowForward;
                    operationSettings.operationSpeed = 4;
                    B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
                }
                else
                {
                   printf("\n forward trick mode in live mode is a no-operation \n");
                }

            }
            break;
        case eSlowMotionReverse:
            {
                tsb_switch_to_play(tsb_ctx);
                operationSettings.operation = eB_DVR_OperationSlowRewind;
                operationSettings.operationSpeed = 4;
                B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
            }
            break;
        case eFrameAdvance:
            {
                if(!tsb_ctx->live_decode_ctx.started) 
                {
                    operationSettings.operation = eB_DVR_OperationForwardFrameAdvance;
                    B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
                }
                else
                {
                    printf("\n forward trick mode in live mode is a no-operation \n");
                }
            }
            break;
        case eFrameReverse:
            {
                tsb_switch_to_play(tsb_ctx);
                operationSettings.operation = eB_DVR_OperationReverseFrameAdvance;
                B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);

            }
            break;
        case eForwardJump:
            {
                unsigned skip_interval=0;
                B_DVR_TSBServiceStatus tsbServiceStatus;
                if(!tsb_ctx->live_decode_ctx.started) 
                {
                    B_Mutex_Unlock(tsb_ctx->mutex);
                    printf("\n Enter the skip interval (ms):");
                    fflush(stdin);
                    scanf("%u",&skip_interval);
                    B_Mutex_Lock(tsb_ctx->mutex);
                    B_DVR_TSBService_GetStatus(tsb_ctx->tsbService,&tsbServiceStatus);
                    tsbServiceStatus.tsbCurrentPlayTime += skip_interval;
                    rc = B_DVR_TSBService_GetIFrameTimeStamp(tsb_ctx->tsbService,
                                                         &tsbServiceStatus.tsbCurrentPlayTime);
                    if(rc == B_DVR_SUCCESS) 
                    {
                        operationSettings.operation = eB_DVR_OperationPause;
                        B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
                        operationSettings.operation = eB_DVR_OperationSeek;
                        operationSettings.seekTime = tsbServiceStatus.tsbCurrentPlayTime;
                        B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
                        operationSettings.operation = eB_DVR_OperationPlay;
                        B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
                    }
                    else
                    {
                        printf("\n invalid skip interval \n");
                    }
                }
                else
                {
                    printf("\n forward trick mode in live mode is a no-operation \n");
                }
            }
            break;
        case eReverseJump:
            {
                unsigned skip_interval=0;
                B_DVR_TSBServiceStatus tsbServiceStatus;
                B_Mutex_Unlock(tsb_ctx->mutex);
                printf("\n Enter the skip interval (ms):");
                fflush(stdin);
                scanf("%u",&skip_interval);
                B_Mutex_Lock(tsb_ctx->mutex);
                tsb_switch_to_play(tsb_ctx);
                B_DVR_TSBService_GetStatus(tsb_ctx->tsbService,&tsbServiceStatus);
                tsbServiceStatus.tsbCurrentPlayTime -= skip_interval;
                rc = B_DVR_TSBService_GetIFrameTimeStamp(tsb_ctx->tsbService,
                                                         &tsbServiceStatus.tsbCurrentPlayTime);
                if(rc == B_DVR_SUCCESS) 
                {
                    operationSettings.operation = eB_DVR_OperationPause;
                    B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
                    operationSettings.operation = eB_DVR_OperationSeek;
                    operationSettings.seekTime = tsbServiceStatus.tsbCurrentPlayTime;
                    B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
                    operationSettings.operation = eB_DVR_OperationPlay;
                    B_DVR_TSBService_SetOperation(tsb_ctx->tsbService,&operationSettings);
                }
                else
                { 
                    printf("\n invalid skip interval \n");
                }
            }
            break;
        case eBadSignalHandling:
            {
                /* In the field input signal becomes weak some times causing input signal to unlock
                   and hence no data flows into recording. This creates huge gaps
                   which creates anamolies in the playback. The solution is to pause the record
                   on signal loss and unpaused the record on signal lock
                   */
                unsigned signal_loss_period; /* ms*/
                B_Mutex_Unlock(tsb_ctx->mutex);
                printf("\n how long the signal is bad (ms) => ");
                scanf("%u",&signal_loss_period);

                B_Mutex_Lock(tsb_ctx->schedulerMutex);
                tsb_ctx->psiInfo.psiStop = true;
                B_Mutex_Unlock(tsb_ctx->schedulerMutex);

                B_Mutex_Lock(tsb_ctx->mutex);
                if(tsb_ctx->psiInfo.injectInProgress) 
                {
                    if(tsb_ctx->psiInfo.timer)
                    {
                       B_Scheduler_CancelTimer(tsb_ctx->scheduler, 
                                              tsb_ctx->psiInfo.timer);
                    }
                    B_DVR_TSBService_InjectDataStop(tsb_ctx->tsbService);
                    tsb_ctx->psiInfo.injectInProgress = false;
                }
                B_DVR_TSBService_Pause(tsb_ctx->tsbService);
                B_Mutex_Unlock(tsb_ctx->mutex);
                BKNI_Sleep(signal_loss_period);
                B_Mutex_Lock(tsb_ctx->mutex);
                B_DVR_TSBService_Resume(tsb_ctx->tsbService);
                rc = B_DVR_TSBService_InjectDataStart(tsb_ctx->tsbService,
                                                      (void *)&tsb_ctx->psiInfo.pat[1],
                                                      188);
                assert(rc == 0);
                tsb_ctx->psiInfo.injectInProgress = true;

               
            }
            break;
        default:
            printf("\n invalid TSB operation \n");
        }
        B_Mutex_Unlock(tsb_ctx->mutex);
    }

    /*
     *  stop the injection thread from kicking off another
     *  injection.
     */
    B_Mutex_Lock(tsb_ctx->schedulerMutex);
    tsb_ctx->psiInfo.psiStop = true;
    B_Mutex_Unlock(tsb_ctx->schedulerMutex);

    /*
     * stop injection in case it was running.
     */
    if(tsb_ctx->psiInfo.injectInProgress) 
    {
        if(tsb_ctx->psiInfo.timer)
        {
            B_Scheduler_CancelTimer(tsb_ctx->scheduler, 
                                    tsb_ctx->psiInfo.timer);
        }
        B_DVR_TSBService_InjectDataStop(tsb_ctx->tsbService);
    }

     /*
      * switch to live mode in case in tsb playback mode.
      */
    tsb_switch_to_live(tsb_ctx);

    /*
     * close TSB playback PID channels
     */
    NEXUS_Playback_ClosePidChannel(tsbServiceSettings.tsbPlaybackSettings.playback,
                                  tsb_ctx->play_decode_ctx.vPidChannel);
    NEXUS_Playback_ClosePidChannel(tsbServiceSettings.tsbPlaybackSettings.playback,
                                  tsb_ctx->play_decode_ctx.aPidChannel);

    /*
     * Stop TSB recording
     */
    B_DVR_TSBService_Stop(tsb_ctx->tsbService);

    /*
     * close data injection PID channel
     */
    NEXUS_PidChannel_Close(tsb_ctx->psiInfo.injectChannel);

    /*
     * close data injection service 
     */
    B_DVR_DataInjectionService_RemoveCallback(tsb_ctx->psiInfo.injectService);
    B_DVR_DataInjectionService_Close(tsb_ctx->psiInfo.injectService);

    /* 
     * close TSB service 
     */
    B_DVR_TSBService_RemoveCallback(tsb_ctx->tsbService);
    B_DVR_TSBService_Close(tsb_ctx->tsbService);

    /*
     * stop live decode
     */
    app_decode_stop(&tsb_ctx->live_decode_ctx);

    /*
     * close live A/V pid channels
     */
    NEXUS_PidChannel_Close(tsb_ctx->live_decode_ctx.vPidChannel);
    NEXUS_PidChannel_Close(tsb_ctx->live_decode_ctx.aPidChannel);

    /*
     * close PCR pid channel if opened
     */
    if( (tsb_ctx->live_decode_ctx.pcrPidChannel != tsb_ctx->live_decode_ctx.vPidChannel) &&
        (tsb_ctx->live_decode_ctx.pcrPidChannel != tsb_ctx->live_decode_ctx.aPidChannel) )
    {
        NEXUS_PidChannel_Close(tsb_ctx->live_decode_ctx.pcrPidChannel);
    }

    /*
     * remove all inputs for HDMI audio output 
     */
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(tsb_ctx->platformconfig.outputs.hdmi[0]));
    #endif

    /*
     * remove all inputs for video window
     */
    NEXUS_VideoWindow_RemoveAllInputs(tsb_ctx->window);

    /*
     * window close
     */
    NEXUS_VideoWindow_Close(tsb_ctx->window);

    /*
     * stc channel close
     */
    NEXUS_StcChannel_Close(tsb_ctx->live_decode_ctx.stcChannel);

    /*
     * video decoder close
     */
    NEXUS_VideoDecoder_Close(tsb_ctx->live_decode_ctx.videoDecoder);

    /*
     * audio decoder close
     */
    NEXUS_AudioDecoder_Close(tsb_ctx->live_decode_ctx.audioDecoder);
    /*
     * remove all outputs from display
     */
    NEXUS_Display_RemoveAllOutputs(tsb_ctx->display);

    /*
     * close display
     */
    NEXUS_Display_Close(tsb_ctx->display);

    /*
     * destroy the active program list in dvr library
     */
    B_DVR_Manager_DestroyMediaNodeList(tsb_ctx->manager,STORAGE_VOLUME_INDEX);

    /*
     * de-initialize dvr manager
     */
    B_DVR_Manager_UnInit(tsb_ctx->manager);

    /*
     * unmount the storage device
     */
    B_DVR_MediaStorage_UnmountVolume(tsb_ctx->storage, STORAGE_VOLUME_INDEX);

    /*
     * close media storage handle
     */
    B_DVR_MediaStorage_Close(tsb_ctx->storage);

    /*
     * close the TSB app scheduler
     */
    B_Scheduler_Stop(tsb_ctx->scheduler);
    B_Scheduler_Destroy(tsb_ctx->scheduler);
    B_Thread_Destroy(tsb_ctx->schedulerThread);
    B_Mutex_Destroy(tsb_ctx->mutex);
    B_Mutex_Destroy(tsb_ctx->schedulerMutex);

    B_Event_Destroy(tsb_ctx->tsbStarted);
    BKNI_DestroyEvent(tsb_ctx->lockChangedEvent);

    /* 
     * Uninitialize the platform
     */

    NEXUS_Platform_Uninit();

    free(tsb_ctx);
error:
    return 0;
}


