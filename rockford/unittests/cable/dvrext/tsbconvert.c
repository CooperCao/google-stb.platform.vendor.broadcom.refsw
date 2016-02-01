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
 * tsbconvert:
 *  App demonstrates conversion of TSB into permanent recording without copying
 *  the media data. TSB conversion is achieved by creating another set of
 *  metadata files for permanent recording. TSB buffers are shared between
 *  permanent recording and TSB. After permanent recording is done,
 *  shared buffers aren't reused in the TSB context rather new set of
 *  buffers are used. Note TSB buffers are nothing but fixed sized
 *  files. PSI data is also injected in the recordings so that the
 *  converted recordings can be streamed over the network to VLC client or
 *  any other IP client. TSB conversion can be kicked off at any point in time
 *  after TSB buffering has started. This app is starting the conversion
 *  immediately after TSB buffering is started. The converted recording
 *  would be 15 minutes long for test purposes.
 *  
 *  This requires segmented file system to be created using mksfs app and
 *  also a QAM source.
 *   
 *  usage :
 *  ./nexus tsbconvert
 */

#include "util.h"

#define TSB_NAME "tsb_0"   /* meta data name for a TSB instance */
#define TSB_DIRECTORY "tsb" /* meta data sub directory for a tsb meta data */
#define PERMANENT_REC_NAME "tsbConverted_0"   /* meta data name for a tsb converted permanent recording instance */
#define PERMANENT_REC_DIRECTORY "tsbConv" /* meta data sub directory for a tsb converted permanent recording instance */
#define PERMANENT_REC_LEN 15*60*1000 /* 15 minutes recording. Units in ms. Note the TSB limit is for 5 min (5*60*60 ms)*/

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

typedef struct tsb_conv_ctx_t
{
    NEXUS_PlatformConfiguration platformconfig;
    NEXUS_ParserBand parserBand;
    NEXUS_FrontendHandle frontend;
    BKNI_EventHandle lockChangedEvent;
    B_DVR_TSBServiceHandle tsbService;
    B_DVR_ServiceCallback serviceCallback;
    B_EventHandle tsbStarted;
    B_EventHandle tsbConversionComplete;
    psiInfo_t psiInfo;
    B_DVR_ManagerHandle manager;
    B_DVR_MediaStorageHandle storage;
    B_MutexHandle schedulerMutex;
    B_SchedulerHandle scheduler;
    B_ThreadHandle schedulerThread;
}tsb_conv_ctx_t;

tsb_conv_ctx_t *tsb_conv_ctx = NULL;
B_DVR_Media media; 
char tsbName[B_DVR_MAX_FILE_NAME_LENGTH]=TSB_NAME;
char tsbSubDir[B_DVR_MAX_FILE_NAME_LENGTH]=TSB_DIRECTORY;
char recName[B_DVR_MAX_FILE_NAME_LENGTH]=PERMANENT_REC_NAME;
char recSubDir[B_DVR_MAX_FILE_NAME_LENGTH]=PERMANENT_REC_DIRECTORY;

void print_tsb_converted_recording_info(tsb_conv_ctx_t *tsb_conv_ctx)
{
    B_DVR_MediaNodeSettings programInfo;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    programInfo.programName = recName;
    programInfo.subDir = recSubDir;
    programInfo.volumeIndex = STORAGE_VOLUME_INDEX;
    rc = B_DVR_Manager_GetMediaNode(tsb_conv_ctx->manager,&programInfo,&media);
    assert(rc == 0);
    app_print_rec_metadata(&media);      
    return;
}
void tsb_conv_scheduler(void * param)
{
    BSTD_UNUSED(param);
    B_Scheduler_Run(tsb_conv_ctx->scheduler);
    return;
}

void tsb_conv_callback(void *context, int index,B_DVR_Event event,B_DVR_Service service)
{
    tsb_conv_ctx_t *tsb_conv_ctx = (tsb_conv_ctx_t *)context;
    BSTD_UNUSED(index);
    BSTD_UNUSED(service);
    switch(event)
    {
        case eB_DVR_EventDataInjectionCompleted:
        {
          
           tsb_conv_ctx->psiInfo.timer = B_Scheduler_StartTimer(tsb_conv_ctx->scheduler,
                                                                tsb_conv_ctx->schedulerMutex,
                                                                INJECT_INTERVAL,
                                                                app_inject_psi,
                                                                (void*)&tsb_conv_ctx->psiInfo);
        }
        break;
    case eB_DVR_EventStartOfRecording:
        {
            B_Event_Set(tsb_conv_ctx->tsbStarted);
            printf("\n TSB buffering started \n");
        }
        break;
    case eB_DVR_EventEndOfRecording:
        {
            printf("\n TSB buffering stopped \n");
        }
        break;
    case eB_DVR_EventHDStreamRecording:
        {
            printf("\n TSB stream source is HD");
            printf("\n press enter to quit \n");
        }
        break;
    case eB_DVR_EventSDStreamRecording:
        {
            printf("\n TSB stream source is SD");
            printf("press enter to quit \n");
        }
        break;
    case eB_DVR_EventMediaProbed:
        {
            printf("\n TSB is probed and stream parameters are available");
            printf("\n press enter to quit \n");
        }
        break;
    case eB_DVR_EventOverFlow:
        {
           printf("\n TSB record overflow \n");
        }
        break;
    case eB_DVR_EventTSBConverstionCompleted:
        {
            printf("\n TSB Conversion completed \n");
        }
        break;
    default:
        printf("\n invalid event \n");
    }
    return;
}

int main(void)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    B_DVR_MediaStorageStatus storageStatus;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    B_DVR_TSBServiceInputEsStream inputEsStream;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    B_DVR_TSBServiceRequest tsbServiceRequest;
    B_DVR_DataInjectionServiceOpenSettings injectOpenSettings;
    B_DVR_DataInjectionServiceSettings injectSettings;
    NEXUS_FrontendAcquireSettings acquireSettings;
    NEXUS_PidChannelSettings pidChannelSettings; 
    B_DVR_TSBServicePermanentRecordingRequest permRecReq;
    B_DVR_TSBServiceStatus tsbServiceStatus;
    B_DVR_MediaStorageType mediaStorageType;
    mediaStorageType = get_storage_device_type();
    if(mediaStorageType == eB_DVR_MediaStorageTypeMax) 
    {
        goto error;
    }
    
    /* 
     * allocate the app context
     */
    tsb_conv_ctx = malloc(sizeof(tsb_conv_ctx_t));
    assert(tsb_conv_ctx);
    memset(tsb_conv_ctx,0,sizeof(tsb_conv_ctx_t));

        /* 
     *  initialized the platform
     */
    rc = NEXUS_Platform_Init(NULL);
    if(rc) 
    {
        printf("\n NEXUS_Platform_Init failed \n ");
        free(tsb_conv_ctx);
        goto error;
    }

    /* 
     *  get platform configuration
     */
    NEXUS_Platform_GetConfiguration(&tsb_conv_ctx->platformconfig);

    /* 
     * acquire the frontend with qam capability
     */
    NEXUS_Frontend_GetDefaultAcquireSettings(&acquireSettings);
    acquireSettings.capabilities.qam = true;
    tsb_conv_ctx->frontend = NEXUS_Frontend_Acquire(&acquireSettings);
    if(!tsb_conv_ctx->frontend)
    {
        printf("\n No QAM frontend found");
        printf("\n app works with QAM source only at present");
        printf("\n use a platform with QAM source\n");
        NEXUS_Platform_Uninit();
        free(tsb_conv_ctx);
        goto error;
    }

    /* 
     * create an event to receive the locked event from frontend
     */
    BKNI_CreateEvent(&tsb_conv_ctx->lockChangedEvent);
    assert(tsb_conv_ctx->lockChangedEvent);

     /* 
     * tune to a channel channel
     */
    tsb_conv_ctx->parserBand = app_tune_channel(tsb_conv_ctx->frontend,
                                                tsb_conv_ctx->lockChangedEvent,
                                                &channelInfo);

    if(tsb_conv_ctx->parserBand == NEXUS_ParserBand_eInvalid)
    {
        printf("\n check and change channelInfo parameters for tuning \n");
        BKNI_DestroyEvent(tsb_conv_ctx->lockChangedEvent);
        NEXUS_Platform_Uninit();
        free(tsb_conv_ctx);
        goto error;
    }

    /* 
     * open media storage
     */
    mediaStorageOpenSettings.storageType = mediaStorageType;
    tsb_conv_ctx->storage = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);
    assert(tsb_conv_ctx->storage);
    
    if(mediaStorageType == eB_DVR_MediaStorageTypeLoopDevice) 
    {
        unsigned volumeIndex=0;
        B_DVR_MediaStorageRegisterVolumeSettings registerSettings;
        memset(&registerSettings,0,sizeof(registerSettings));
        sprintf(registerSettings.device,VSFS_DEVICE);
        registerSettings.startSec = 0;
        registerSettings.length = VSFS_SIZE;
        B_DVR_MediaStorage_RegisterVolume(tsb_conv_ctx->storage,&registerSettings,&volumeIndex);
    }

    /* 
     * check if the media storage is in segmented file system format
     */
    B_DVR_MediaStorage_GetStatus(tsb_conv_ctx->storage,&storageStatus);
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
        B_DVR_MediaStorage_Close(tsb_conv_ctx->storage);
        BKNI_DestroyEvent(tsb_conv_ctx->lockChangedEvent);
        NEXUS_Platform_Uninit();
        free(tsb_conv_ctx);
        goto error;
    }
    /* 
     *  mount media storage -> media, nav and metadata partitions
     */
    rc = B_DVR_MediaStorage_MountVolume(tsb_conv_ctx->storage, STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    /* 
     *  initialize the DVR manager
     */
    tsb_conv_ctx->manager = B_DVR_Manager_Init(NULL);
    assert(tsb_conv_ctx->manager);

    /* 
     *  create the list of recording if available
     */
    rc = B_DVR_Manager_CreateMediaNodeList(tsb_conv_ctx->manager,STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    /* 
     * create app scheduler mutex
     */
    tsb_conv_ctx->schedulerMutex = B_Mutex_Create(NULL);
    assert(tsb_conv_ctx->schedulerMutex);

    /* 
     * create app scheduler
     */
    tsb_conv_ctx->scheduler = B_Scheduler_Create(NULL);
    assert(tsb_conv_ctx->scheduler);

    /* 
     *  create the scheduler thread
     */
    tsb_conv_ctx->schedulerThread = B_Thread_Create("tsb_conv",tsb_conv_scheduler,tsb_conv_ctx, NULL);
    assert(tsb_conv_ctx->schedulerThread);

    /*
     *  check if the permanent recording already exisits. If yes, delete
     *  and let the conversion create a recording with the same name.
     *  This deletion is required because in the real world applications,
     *  no 2 recordings would be of the same name.
     */
    memset((void *)&mediaNodeSettings,0,sizeof(mediaNodeSettings));
    mediaNodeSettings.programName = recName;
    mediaNodeSettings.subDir = recSubDir;
    mediaNodeSettings.volumeIndex = STORAGE_VOLUME_INDEX;
    rc = B_DVR_Manager_GetMediaNode(tsb_conv_ctx->manager,&mediaNodeSettings,&media);
    if (rc == B_DVR_SUCCESS) 
    {
        rc = B_DVR_Manager_DeleteRecording(tsb_conv_ctx->manager,&mediaNodeSettings);
        assert(rc == 0);
    }
    /* 
     *  allocate TSB buffers -> reserves n number of media and nav file segments
     *  to be used for TSB
     */
    memset((void *)&mediaNodeSettings,0,sizeof(mediaNodeSettings));
    mediaNodeSettings.programName = tsbName;
    mediaNodeSettings.subDir = tsbSubDir;
    mediaNodeSettings.volumeIndex=STORAGE_VOLUME_INDEX;
    B_DVR_Manager_AllocSegmentedFileRecord(tsb_conv_ctx->manager,
                                           &mediaNodeSettings,
                                           MAX_TSB_BUFFERS);

    tsb_conv_ctx->tsbStarted = B_Event_Create(NULL);
    assert(tsb_conv_ctx->tsbStarted);

    tsb_conv_ctx->tsbConversionComplete = B_Event_Create(NULL);
    assert(tsb_conv_ctx->tsbConversionComplete);

    /* 
     * populate TSBService request parameters
     */
    memset((void *)&tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
    strcpy(tsbServiceRequest.programName,tsbName);
    strcpy(tsbServiceRequest.subDir,tsbSubDir);
    tsbServiceRequest.input = eB_DVR_TSBServiceInputQam;
    tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
    tsbServiceRequest.maxTSBTime = MAX_TSB_TIME;
    tsbServiceRequest.recpumpIndex = NEXUS_ANY_ID;
    tsbServiceRequest.playpumpIndex = NEXUS_ANY_ID;
    tsbServiceRequest.volumeIndex = STORAGE_VOLUME_INDEX;

    /* 
     * open a TSB service
     */
    tsb_conv_ctx->tsbService = B_DVR_TSBService_Open(&tsbServiceRequest);
    assert(tsb_conv_ctx->tsbService);

    /* 
     * install a callback from TSB service to receive asynchronous events
     * from a TSB service instance
     */
    tsb_conv_ctx->serviceCallback = (B_DVR_ServiceCallback)tsb_conv_callback;
    B_DVR_TSBService_InstallCallback(tsb_conv_ctx->tsbService,
                                     tsb_conv_ctx->serviceCallback,
                                     (void*)tsb_conv_ctx);

    
    /* 
     * populate data injection settings
     */
    memset((void *)&injectOpenSettings,0,sizeof(injectOpenSettings));
    injectOpenSettings.fifoSize = 30*188; 
    tsb_conv_ctx->psiInfo.injectService  = B_DVR_DataInjectionService_Open(&injectOpenSettings);
    assert(tsb_conv_ctx->psiInfo.injectService);

    /* 
     * install a callback to receive events from data injection service
     */
    B_DVR_DataInjectionService_InstallCallback(tsb_conv_ctx->psiInfo.injectService,
                                               tsb_conv_ctx->serviceCallback,
                                               (void*)tsb_conv_ctx);

    /*
     * allocate a PID channel for data injection.
     */
    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    pidChannelSettings.pidChannelIndex = (NEXUS_NUM_PID_CHANNELS-2);
    pidChannelSettings.enabled = false;
    tsb_conv_ctx->psiInfo.injectChannel = NEXUS_PidChannel_Open(tsb_conv_ctx->parserBand,
                                                           0x1ffe,
                                                           &pidChannelSettings);
    assert(tsb_conv_ctx->psiInfo.injectChannel);

    /*
     * associate a PID channel with data injection service 
     * and also set the type of injection (transport packet or PAT/PMT data) 
     */
    memset(&injectSettings,0,sizeof(injectSettings));
    injectSettings.dataInjectionServiceType = eB_DVR_DataInjectionServiceTypeRawData;
    injectSettings.pidChannel = tsb_conv_ctx->psiInfo.injectChannel;
    B_DVR_DataInjectionService_SetSettings(tsb_conv_ctx->psiInfo.injectService,&injectSettings);

    /*
     * associate a data injection service instance and an inBand parser 
     * to a tsb service instance 
     */
    memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
    tsbServiceSettings.dataInjectionService = tsb_conv_ctx->psiInfo.injectService;
    tsbServiceSettings.parserBand = tsb_conv_ctx->parserBand;
    B_DVR_TSBService_SetSettings(tsb_conv_ctx->tsbService,&tsbServiceSettings);

    /*
     * add video es stream info to a TSB service instance
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.vPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    inputEsStream.esStreamInfo.codec.videoCodec = channelInfo.vCodec;
    inputEsStream.pidChannel = NULL;
    B_DVR_TSBService_AddInputEsStream(tsb_conv_ctx->tsbService,&inputEsStream);

    /*
     * add audio es stream info to a TSB service instance
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.aPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    inputEsStream.esStreamInfo.codec.audioCodec = channelInfo.aCodec;
    B_DVR_TSBService_AddInputEsStream(tsb_conv_ctx->tsbService,&inputEsStream);

    /*
     * add data stream info to a TSB service instance
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.pcrPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
    B_DVR_TSBService_AddInputEsStream(tsb_conv_ctx->tsbService,&inputEsStream);

    /*
     * create PAT and PMT packets based on A/V and data PID info
     */
    app_create_psi(&tsb_conv_ctx->psiInfo,
                   channelInfo.pcrPid,
                   channelInfo.vPid,
                   channelInfo.aPid,
                   channelInfo.vCodec,
                   channelInfo.aCodec);

    /*
     * start TSB recording
     */
    rc = B_DVR_TSBService_Start(tsb_conv_ctx->tsbService,NULL);
    assert(rc == 0);
    tsb_conv_ctx->psiInfo.tsbService = tsb_conv_ctx->tsbService;

    /*
     * kick off data injection
     */
    tsb_conv_ctx->psiInfo.psiStop = false;
    rc = B_DVR_TSBService_InjectDataStart(tsb_conv_ctx->tsbService,
                                          (void *)&tsb_conv_ctx->psiInfo.pat[1],
                                          188);
    assert(rc == 0);
    tsb_conv_ctx->psiInfo.injectInProgress = true;

    /*
     * wait for start of recording event .i.e. availability of 
     * 1st I frame. 
     */
    B_Event_Wait(tsb_conv_ctx->tsbStarted, 1000);
    
    /*
     * Get TSB status to know the tsb window
     */
    rc = B_DVR_TSBService_GetStatus(tsb_conv_ctx->tsbService, &tsbServiceStatus);
    assert(rc == 0);

    /*
     * populate the permanent recording request through tsb conversion start 
     * Conversion start time with in the tsb window. 
     */
    strcpy(permRecReq.programName,recName);
    strcpy(permRecReq.subDir,recSubDir);
    permRecReq.recStartTime = tsbServiceStatus.tsbRecStartTime;
    permRecReq.recEndTime = tsbServiceStatus.tsbRecStartTime + PERMANENT_REC_LEN;
    rc = B_DVR_TSBService_ConvertStart(tsb_conv_ctx->tsbService,&permRecReq);
    assert(rc == 0);
    printf("\n **************************************************************************************");
    printf("\n After conversion is complete, conversion complete message would appear on the console");
    printf("\n Press enter to exit");
    printf("\n **************************************************************************************\n");
    getchar();
    rc = B_DVR_TSBService_GetStatus(tsb_conv_ctx->tsbService, &tsbServiceStatus);
    assert(rc == 0);
    printf("\n Tsb end time %ld \n"
           "TsbConversion current time : %ld \n"
           "TsbConversion end time: %ld \n",
           tsbServiceStatus.tsbRecEndTime,
           tsbServiceStatus.permRecCurrentTime, permRecReq.recEndTime);
     B_DVR_TSBService_GetSettings(tsb_conv_ctx->tsbService,&tsbServiceSettings);
     tsbServiceSettings.tsbConvEndTime = tsbServiceStatus.tsbRecEndTime;
     B_DVR_TSBService_SetSettings(tsb_conv_ctx->tsbService,&tsbServiceSettings);
     B_Event_Wait(tsb_conv_ctx->tsbConversionComplete, 1000);
    /*
     * prevent injection thread from kicking off another injection
     */
    B_Mutex_Lock(tsb_conv_ctx->schedulerMutex);
    tsb_conv_ctx->psiInfo.psiStop = true;
    B_Mutex_Unlock(tsb_conv_ctx->schedulerMutex);

    /*
     * stop TSB conversion
     */
    rc = B_DVR_TSBService_ConvertStop(tsb_conv_ctx->tsbService);
    assert(rc == 0);

    /*
     * stop data injection service
     */
    if(tsb_conv_ctx->psiInfo.injectInProgress) 
    {
        if(tsb_conv_ctx->psiInfo.timer)
        {
            B_Scheduler_CancelTimer(tsb_conv_ctx->scheduler, 
                                    tsb_conv_ctx->psiInfo.timer);
        }
        B_DVR_TSBService_InjectDataStop(tsb_conv_ctx->tsbService);
    }
    
    /*
     * stop TSB service
     */
    B_DVR_TSBService_Stop(tsb_conv_ctx->tsbService);

    /*
     * close data injection PID channel
     */
    NEXUS_PidChannel_Close(tsb_conv_ctx->psiInfo.injectChannel);

    /*
     * close data injection service
     */
    B_DVR_DataInjectionService_RemoveCallback(tsb_conv_ctx->psiInfo.injectService);
    B_DVR_DataInjectionService_Close(tsb_conv_ctx->psiInfo.injectService);

    /*
     * close TSB service
     */
    B_DVR_TSBService_RemoveCallback(tsb_conv_ctx->tsbService);
    B_DVR_TSBService_Close(tsb_conv_ctx->tsbService);

    /*
     * print recording info
     */
    print_tsb_converted_recording_info(tsb_conv_ctx);

    /*
     * destroy the active recording list if available
     */
    B_DVR_Manager_DestroyMediaNodeList(tsb_conv_ctx->manager,STORAGE_VOLUME_INDEX);

    /*
     * de-initialize the dvr manager
     */
    B_DVR_Manager_UnInit(tsb_conv_ctx->manager);

    /*
     * unmount the storage media
     */
    B_DVR_MediaStorage_UnmountVolume(tsb_conv_ctx->storage, STORAGE_VOLUME_INDEX);

    /*
     * close media storage
     */
    B_DVR_MediaStorage_Close(tsb_conv_ctx->storage);

    /*
     * stop the tsbconvert app scheduler
     */
    B_Scheduler_Stop(tsb_conv_ctx->scheduler);
    B_Scheduler_Destroy(tsb_conv_ctx->scheduler);
    B_Thread_Destroy(tsb_conv_ctx->schedulerThread);
    B_Mutex_Destroy(tsb_conv_ctx->schedulerMutex);
    
    B_Event_Destroy(tsb_conv_ctx->tsbStarted);
    B_Event_Destroy(tsb_conv_ctx->tsbConversionComplete);
    BKNI_DestroyEvent(tsb_conv_ctx->lockChangedEvent);
    /* 
     * Uninitialize the platform
     */
    NEXUS_Platform_Uninit();
    free(tsb_conv_ctx);
error:
    return 0;
}


