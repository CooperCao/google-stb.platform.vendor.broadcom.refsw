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
 *  stream_tsb:
 *  App that streams (UDP based) a live stream via TSB buffering (segmented)
 *  from a VMS server to a VLC client running on a PC [Window/Linux machine]
 *  This app mainly demonstrates the usage of dvr library media file APIs
 *  for streaming TSB content from a VMS to a client. This UDP streaming
 *  method in the app isn't optimal. TSB is available only through a
 *  QAM source in this app.
 *  User has to update the client IP and server ethernet interface
 *  based on the test environment. App requires the media storage
 *  in segmented file sytem format which can be created
 *  using mksfs app
 *  usage:
 *  ./nexus stream_tsb
 *  
 *  On client
 *  1. launch VLC player.
 *  2. Click on media drop down menu
 *  3. open network stream
 *  4. under URL udp://@xxx.xxx.xxx.xxx:5000
 *  5. click on show more options
 *  6. adjust the caching for jitter adjustment
 *  7. play
 *  
 */

#include "util.h"


/* 
 *  The below parameters are the default parameters used by the
 *  stream_tsb application. These might need modifications
 *  by the user based on a specific test environment.
 */

#define TSB_NAME "tsb_0"   /* meta data name for a TSB instance */
#define TSB_DIRECTORY "tsb" /* meta data sub directory for a tsb meta data */
#define CLIENT_PORT 5000    /* client port */
/*#define CLIENT_IP "10.22.65.127"  client IP */
#define CLIENT_IP "10.22.10.31" /* client IP */
#define SERVER_NET_IF "eth2"     /* server ethernet interface 
                                    On BCM97425VMS_SFF board, use CFE command ephycfg with option 4
                                    to configure the eth0 to use the Giga bit switch 
                                  */


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

typedef struct strm_tsb_ctx_t
{
    NEXUS_PlatformConfiguration platformconfig;
    NEXUS_ParserBand parserBand;
    NEXUS_FrontendHandle frontend;
    BKNI_EventHandle lockChangedEvent;
    B_DVR_TSBServiceHandle tsbService;
    B_DVR_ServiceCallback serviceCallback;
    B_EventHandle tsbStarted;
    psiInfo_t psiInfo;
    strm_ctx_t strm_ctx;
    B_DVR_ManagerHandle manager;
    B_DVR_MediaStorageHandle storage;
    B_MutexHandle schedulerMutex;
    B_SchedulerHandle scheduler;
    B_ThreadHandle schedulerThread;
}strm_tsb_ctx_t;

strm_tsb_ctx_t *strm_tsb_ctx = NULL;

void stream_tsb_scheduler(void * param)
{
    BSTD_UNUSED(param);
    B_Scheduler_Run(strm_tsb_ctx->scheduler);
    return;
}

void stream_tsb_callback(void *context, int index,B_DVR_Event event,B_DVR_Service service)
{
    strm_tsb_ctx_t *strm_tsb_ctx = (strm_tsb_ctx_t *)context;
    BSTD_UNUSED(index);
    BSTD_UNUSED(service);
    switch(event)
    {
        case eB_DVR_EventDataInjectionCompleted:
        {
          
           strm_tsb_ctx->psiInfo.timer = B_Scheduler_StartTimer(strm_tsb_ctx->scheduler,
                                                                strm_tsb_ctx->schedulerMutex,
                                                                INJECT_INTERVAL,
                                                                app_inject_psi,
                                                                (void*)&strm_tsb_ctx->psiInfo);
        }
        break;
    case eB_DVR_EventStartOfRecording:
        {
            printf("\n TSB buffering started");
        }
        break;
    case eB_DVR_EventEndOfRecording:
        {
            printf("\n TSB buffering stopped");
        }
        break;
    case eB_DVR_EventHDStreamRecording:
        {
            printf("\n TSB stream source is HD");
        }
        break;
    case eB_DVR_EventSDStreamRecording:
        {
            printf("\n TSB stream source is SD");
        }
    case eB_DVR_EventMediaProbed:
        {
            B_Event_Set(strm_tsb_ctx->tsbStarted);
            printf("\n TSB is probed and stream parameters are available");
        }
        break;
    default:
        printf("\n invalid event");
    }
    return;
}

int main(void)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    char programName[B_DVR_MAX_FILE_NAME_LENGTH]=TSB_NAME;
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH]=TSB_DIRECTORY;
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
    B_DVR_MediaStorageType mediaStorageType;

    mediaStorageType = get_storage_device_type();
    if(mediaStorageType == eB_DVR_MediaStorageTypeMax) 
    {
        goto error;
    }
    
    /* 
     * allocate the app context
     */
    strm_tsb_ctx = malloc(sizeof(strm_tsb_ctx_t));
    assert(strm_tsb_ctx);
    memset(strm_tsb_ctx,0,sizeof(strm_tsb_ctx_t));

    /* 
     *  initialized the platform
     */
    rc = NEXUS_Platform_Init(NULL);
    if(rc) 
    {
        printf("\n NEXUS_Platform_Init failed \n ");
        free(strm_tsb_ctx);
        goto error;
    }

    /* 
     *  get platform configuration
     */
    NEXUS_Platform_GetConfiguration(&strm_tsb_ctx->platformconfig);

    /* 
     * acquire the frontend with qam capability
     */
    NEXUS_Frontend_GetDefaultAcquireSettings(&acquireSettings);
    acquireSettings.capabilities.qam = true;
    strm_tsb_ctx->frontend = NEXUS_Frontend_Acquire(&acquireSettings);
    if(!strm_tsb_ctx->frontend) 
    {
        printf("\n No QAM frontend found");
        printf("\n app works with QAM source only at present");
        printf("\n use a platform with QAM source\n");
        NEXUS_Platform_Uninit();
        free(strm_tsb_ctx);
        goto error;
    }

    /* 
     * create an event to receive the locked event from frontend
     */
    BKNI_CreateEvent(&strm_tsb_ctx->lockChangedEvent);
    assert(strm_tsb_ctx->lockChangedEvent);

    /* 
     * tune to a channel channel
     */
    strm_tsb_ctx->parserBand = app_tune_channel(strm_tsb_ctx->frontend,
                                                      strm_tsb_ctx->lockChangedEvent,
                                                      &channelInfo);

    if(strm_tsb_ctx->parserBand == NEXUS_ParserBand_eInvalid)
    {
        printf("\n check and change channelInfo parameters for tuning \n");
        BKNI_DestroyEvent(strm_tsb_ctx->lockChangedEvent);
        NEXUS_Platform_Uninit();
        free(strm_tsb_ctx);
        goto error;
    }
    /* 
     * open media storage
     */
    mediaStorageOpenSettings.storageType = mediaStorageType;
    strm_tsb_ctx->storage = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);
    assert(strm_tsb_ctx->storage);

    if(mediaStorageType == eB_DVR_MediaStorageTypeLoopDevice) 
    {
        unsigned volumeIndex=0;
        B_DVR_MediaStorageRegisterVolumeSettings registerSettings;
        memset(&registerSettings,0,sizeof(registerSettings));
        sprintf(registerSettings.device,VSFS_DEVICE);
        registerSettings.startSec = 0;
        registerSettings.length = VSFS_SIZE;
        B_DVR_MediaStorage_RegisterVolume(strm_tsb_ctx->storage,&registerSettings,&volumeIndex);
    }
    
    /* 
     * check if the media storage is in segmented file system format
     */
    B_DVR_MediaStorage_GetStatus(strm_tsb_ctx->storage,&storageStatus);
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
        B_DVR_MediaStorage_Close(strm_tsb_ctx->storage);
        BKNI_DestroyEvent(strm_tsb_ctx->lockChangedEvent);
        NEXUS_Platform_Uninit();
        free(strm_tsb_ctx);
        goto error;

    }
    /* 
     *  mount media storage -> media, nav and metadata partitions
     */
    rc = B_DVR_MediaStorage_MountVolume(strm_tsb_ctx->storage, STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    /* 
     *  initialize the DVR manager
     */
    strm_tsb_ctx->manager = B_DVR_Manager_Init(NULL);
    assert(strm_tsb_ctx->manager);

    /* 
     *  create the list of recording if available
     */
    rc = B_DVR_Manager_CreateMediaNodeList(strm_tsb_ctx->manager,STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    /* 
     * create app scheduler mutex
     */
    strm_tsb_ctx->schedulerMutex = B_Mutex_Create(NULL);
    assert(strm_tsb_ctx->schedulerMutex);

    /* 
     * create app scheduler
     */
    strm_tsb_ctx->scheduler = B_Scheduler_Create(NULL);
    assert(strm_tsb_ctx->scheduler);

    /* 
     *  create the scheduler thread
     */
    strm_tsb_ctx->schedulerThread = B_Thread_Create("stream_tsb",stream_tsb_scheduler,strm_tsb_ctx, NULL);
    assert(strm_tsb_ctx->schedulerThread);

    /* 
     *  allocate TSB buffers -> reserves n number of media and nav file segments
     *  to be used for TSB
     */
    memset((void *)&mediaNodeSettings,0,sizeof(mediaNodeSettings));
    mediaNodeSettings.programName = programName;
    mediaNodeSettings.subDir = subDir;
    mediaNodeSettings.volumeIndex=STORAGE_VOLUME_INDEX;
    B_DVR_Manager_AllocSegmentedFileRecord(strm_tsb_ctx->manager,
                                           &mediaNodeSettings,
                                           MAX_TSB_BUFFERS);
    
    strm_tsb_ctx->tsbStarted = B_Event_Create(NULL);
    assert(strm_tsb_ctx->tsbStarted);
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
     * open a TSB service
     */
    strm_tsb_ctx->tsbService = B_DVR_TSBService_Open(&tsbServiceRequest);
    assert(strm_tsb_ctx->tsbService);

    /* 
     * install a callback from TSB service to receive asynchronous events
     * from a TSB service instance
     */
    strm_tsb_ctx->serviceCallback = (B_DVR_ServiceCallback)stream_tsb_callback;
    B_DVR_TSBService_InstallCallback(strm_tsb_ctx->tsbService,
                                     strm_tsb_ctx->serviceCallback,
                                     (void*)strm_tsb_ctx);

    
    /* 
     * populate data injection settings
     */
    memset((void *)&injectOpenSettings,0,sizeof(injectOpenSettings));
    injectOpenSettings.fifoSize = 30*188; 
    strm_tsb_ctx->psiInfo.injectService  = B_DVR_DataInjectionService_Open(&injectOpenSettings);
    assert(strm_tsb_ctx->psiInfo.injectService);

    /* 
     * install a callback to receive events from data injection service
     */
    B_DVR_DataInjectionService_InstallCallback(strm_tsb_ctx->psiInfo.injectService,
                                               strm_tsb_ctx->serviceCallback,
                                               (void*)strm_tsb_ctx);

    /*
     * allocate a PID channel for data injection.
     */
    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    pidChannelSettings.pidChannelIndex = (NEXUS_NUM_PID_CHANNELS-2);
    pidChannelSettings.enabled = false;
    strm_tsb_ctx->psiInfo.injectChannel = NEXUS_PidChannel_Open(strm_tsb_ctx->parserBand,
                                                              0x1ffe,
                                                              &pidChannelSettings);
    assert(strm_tsb_ctx->psiInfo.injectChannel);

    /*
     * associate a PID channel with data injection service 
     * and also set the type of injection (transport packet or PAT/PMT data) 
     */
    memset(&injectSettings,0,sizeof(injectSettings));
    injectSettings.dataInjectionServiceType = eB_DVR_DataInjectionServiceTypeRawData;
    injectSettings.pidChannel = strm_tsb_ctx->psiInfo.injectChannel;
    B_DVR_DataInjectionService_SetSettings(strm_tsb_ctx->psiInfo.injectService,&injectSettings);

    /*
     * associate a data injection service instance and an inBand parser 
     * to a tsb service instance 
     */
    memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
    tsbServiceSettings.dataInjectionService = strm_tsb_ctx->psiInfo.injectService;
    tsbServiceSettings.parserBand = strm_tsb_ctx->parserBand;
    B_DVR_TSBService_SetSettings(strm_tsb_ctx->tsbService,&tsbServiceSettings);

    /*
     * add video es stream info to a TSB service instance
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.vPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    inputEsStream.esStreamInfo.codec.videoCodec = channelInfo.vCodec;
    inputEsStream.pidChannel = NULL;
    B_DVR_TSBService_AddInputEsStream(strm_tsb_ctx->tsbService,&inputEsStream);

    /*
     * add audio es stream info to a TSB service instance
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.aPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    inputEsStream.esStreamInfo.codec.audioCodec = channelInfo.aCodec;
    B_DVR_TSBService_AddInputEsStream(strm_tsb_ctx->tsbService,&inputEsStream);

    /*
     * add data stream info to a TSB service instance
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.pcrPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
    B_DVR_TSBService_AddInputEsStream(strm_tsb_ctx->tsbService,&inputEsStream);

    /*
     * create PAT and PMT packets based on A/V and data PID info
     */
    app_create_psi(&strm_tsb_ctx->psiInfo,
                   channelInfo.pcrPid,
                   channelInfo.vPid,
                   channelInfo.aPid,
                   channelInfo.vCodec,
                   channelInfo.aCodec);

    /*
     * start TSB buffering
     */
    rc = B_DVR_TSBService_Start(strm_tsb_ctx->tsbService,NULL);
    assert(rc == 0);

    /*
     * start data injection
     */
    strm_tsb_ctx->psiInfo.tsbService = strm_tsb_ctx->tsbService;
    strm_tsb_ctx->psiInfo.psiStop = false;
    rc = B_DVR_TSBService_InjectDataStart(strm_tsb_ctx->tsbService,
                                          (void *)&strm_tsb_ctx->psiInfo.pat[1],
                                          188);
    assert(rc == 0);
    strm_tsb_ctx->psiInfo.injectInProgress = true;

    /*
     * wait for start of recording event .i.e. 1st frame availability
     */
    B_Event_Wait(strm_tsb_ctx->tsbStarted, 1000);
    usleep(50000);

    /*
     * create a producer event used for reading data from 
     * the server 
     */
    strm_tsb_ctx->strm_ctx.producerEvent = B_Event_Create(NULL);
    assert(strm_tsb_ctx->strm_ctx.producerEvent);

    /*
     * create a consumer event used for writing the data into 
     * a network socket configured for a server-client 
     * connection 
     */
    strm_tsb_ctx->strm_ctx.consumerEvent = B_Event_Create(NULL);
    assert(strm_tsb_ctx->strm_ctx.consumerEvent);

    /*
     * scheduler event for producer
     */
    strm_tsb_ctx->strm_ctx.producerId =  B_Scheduler_RegisterEvent(strm_tsb_ctx->scheduler,
                                                                   strm_tsb_ctx->schedulerMutex,
                                                                   strm_tsb_ctx->strm_ctx.producerEvent,
                                                                   app_udp_producer,
                                                                   (void*)&strm_tsb_ctx->strm_ctx);
    assert(strm_tsb_ctx->strm_ctx.producerId);

    /*
     * scheduler event for consumer
     */
    strm_tsb_ctx->strm_ctx.consumerId =  B_Scheduler_RegisterEvent(strm_tsb_ctx->scheduler,
                                                                   strm_tsb_ctx->schedulerMutex,
                                                                   strm_tsb_ctx->strm_ctx.consumerEvent,
                                                                   app_udp_consumer,
                                                                   (void*)&strm_tsb_ctx->strm_ctx);
    assert(strm_tsb_ctx->strm_ctx.consumerId);

    memcpy(&strm_tsb_ctx->strm_ctx.programSettings,&mediaNodeSettings,sizeof(mediaNodeSettings));

    /*
     * open an UDP socket connection between client and server
     */
    strm_tsb_ctx->strm_ctx.tsbService = strm_tsb_ctx->tsbService;
    app_udp_open(&strm_tsb_ctx->strm_ctx,
                 CLIENT_PORT,
                 CLIENT_IP,
                 SERVER_NET_IF);

    /*
     * kick off the producer .i.e. reading 
     * from the tsb buffered data which 
     * in turn would kick off the consumer. 
     * Consumer will kick off the producer 
     * again if all the data read in 
     * producer is written over the network 
     */
    strm_tsb_ctx->strm_ctx.streamingStop = false;
    gettimeofday(&strm_tsb_ctx->strm_ctx.start,NULL);
    app_udp_producer((void *)&strm_tsb_ctx->strm_ctx);

    printf("\n press enter to quit tsb streaming");
    getchar();

    /*
     * prevent producer and consumer events 
     * from triggered and also prevent 
     * the data injection from triggering 
     * another packet injection 
     */
    B_Mutex_Lock(strm_tsb_ctx->schedulerMutex);
    strm_tsb_ctx->strm_ctx.streamingStop = true;
    strm_tsb_ctx->psiInfo.psiStop = true;
    B_Mutex_Unlock(strm_tsb_ctx->schedulerMutex);

    /*
     * de-register the consumer and producer 
     * events from the app scheduler
     */
    B_Scheduler_UnregisterEvent(strm_tsb_ctx->scheduler,
                                strm_tsb_ctx->strm_ctx.producerId);
    B_Scheduler_UnregisterEvent(strm_tsb_ctx->scheduler,
                                strm_tsb_ctx->strm_ctx.consumerId);

    /*
     * destroy producer and consumer events
     */
    B_Event_Destroy(strm_tsb_ctx->strm_ctx.producerEvent);
    B_Event_Destroy(strm_tsb_ctx->strm_ctx.consumerEvent);

    /*
     * close the UDP socket connection between 
     * client and server 
     */
    app_udp_close(&strm_tsb_ctx->strm_ctx);

    /*
     * close data injection service
     */
    if(strm_tsb_ctx->psiInfo.injectInProgress) 
    {
        if(strm_tsb_ctx->psiInfo.timer)
        {
            B_Scheduler_CancelTimer(strm_tsb_ctx->scheduler, 
                                    strm_tsb_ctx->psiInfo.timer);
        }
        B_DVR_TSBService_InjectDataStop(strm_tsb_ctx->tsbService);
    }

    /*
     * close TSB service
     */
    B_DVR_TSBService_Stop(strm_tsb_ctx->tsbService);

    /*
     * close data injection PID channel
     */
    NEXUS_PidChannel_Close(strm_tsb_ctx->psiInfo.injectChannel);

    /*
     * close data injection service
     */
    B_DVR_DataInjectionService_RemoveCallback(strm_tsb_ctx->psiInfo.injectService);
    B_DVR_DataInjectionService_Close(strm_tsb_ctx->psiInfo.injectService);

    /*
     * close TSB service
     */
    B_DVR_TSBService_RemoveCallback(strm_tsb_ctx->tsbService);
    B_DVR_TSBService_Close(strm_tsb_ctx->tsbService);
 
    /*
     * destroy list of recordings if available
     */
    B_DVR_Manager_DestroyMediaNodeList(strm_tsb_ctx->manager,STORAGE_VOLUME_INDEX);

    /*
     * de-initialize the dvr manager
     */
    B_DVR_Manager_UnInit(strm_tsb_ctx->manager);

    /*
     * unmount media storage
     */
    B_DVR_MediaStorage_UnmountVolume(strm_tsb_ctx->storage, STORAGE_VOLUME_INDEX);

    /*
     * close media storage
     */
    B_DVR_MediaStorage_Close(strm_tsb_ctx->storage);

    /*
     * stop the app scheduler
     */
    B_Scheduler_Stop(strm_tsb_ctx->scheduler);
    B_Scheduler_Destroy(strm_tsb_ctx->scheduler);
    B_Thread_Destroy(strm_tsb_ctx->schedulerThread);
    B_Mutex_Destroy(strm_tsb_ctx->schedulerMutex);

    B_Event_Destroy(strm_tsb_ctx->tsbStarted);
    BKNI_DestroyEvent(strm_tsb_ctx->lockChangedEvent);
    /* 
     * Uninitialize the platform
     */
    NEXUS_Platform_Uninit();

    free(strm_tsb_ctx);
error:
    return 0;
}


