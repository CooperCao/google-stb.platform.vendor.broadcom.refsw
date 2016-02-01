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
 ****************************************************************************/
#include "util.h"

/*
 *  vod:
 *  App that demonstrates VOD streaming from MSO server. VOD streaming has
 *  trick modes supported from MSO server. Data from MSO  server
 *  comes through a QAM source in a VMS server and enters record service.
 *  Data from the record server is read and sent over the network
 *  to clients from the VMS server.
 *  This app mainly demonstrates the usage of dvr library record service
 *  usage for VOD. The UDP streaming method in the app isn't optimal.
 *  User has to update the client IP and server ethernet interface
 *  based on the test environment.
 *  
 *  usage:
 *  ./nexus vod
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



#define VOD_NAME "tsb_0"   /* meta data name for a VOD instance */
#define VOD_DIRECTORY "tsb" /* meta data sub directory for a VOD meta data */
#define CLIENT_PORT 5000    /* client port */
#define CLIENT_IP "10.22.10.31" /* client IP */
#define SERVER_NET_IF "eth2"    /* server ethernet interface 
                                   On BCM97425VMS_SFF board, use CFE command ephycfg with option 4
                                   to configure the eth0 to use the Giga bit switch 
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

typedef struct vod_ctx_t
{
    NEXUS_PlatformConfiguration platformconfig;
    NEXUS_ParserBand parserBand;
    NEXUS_FrontendHandle frontend;
    BKNI_EventHandle lockChangedEvent;
    B_DVR_ManagerHandle manager;
    B_DVR_MediaStorageHandle storage;
    B_DVR_RecordServiceHandle recordService;
    B_DVR_ServiceCallback serviceCallback;
    psiInfo_t  psiInfo;
    strm_ctx_t strm_ctx;
    B_MutexHandle mutex;
    B_MutexHandle schedulerMutex;
    B_SchedulerHandle scheduler;
    B_ThreadHandle schedulerThread;
}vod_ctx_t;

vod_ctx_t *vod_ctx = NULL;

void vod_scheduler(void * param)
{
    BSTD_UNUSED(param);
    B_Scheduler_Run(vod_ctx->scheduler);
    return;
}

void vod_callback(void *context, int index,B_DVR_Event event,B_DVR_Service service)
{
    vod_ctx_t *vod_ctx = (vod_ctx_t *)context;
    BSTD_UNUSED(index);
    BSTD_UNUSED(service);
    switch(event)
    {
        case eB_DVR_EventDataInjectionCompleted:
        {
          
            vod_ctx->psiInfo.timer = B_Scheduler_StartTimer(vod_ctx->scheduler,
                                                            vod_ctx->schedulerMutex,
                                                            INJECT_INTERVAL,
                                                            app_inject_psi,
                                                            (void*)&vod_ctx->psiInfo);
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
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    B_DVR_MediaStorageStatus storageStatus;
    B_DVR_RecordServiceInputEsStream inputEsStream;
    B_DVR_RecordServiceSettings recordServiceSettings;
    B_DVR_RecordServiceRequest recordServiceRequest;
    B_DVR_DataInjectionServiceOpenSettings injectOpenSettings;
    B_DVR_DataInjectionServiceSettings injectSettings;
    NEXUS_FrontendAcquireSettings acquireSettings;
    NEXUS_PidChannelSettings pidChannelSettings; 
    B_DVR_MediaNodeSettings mediaNodeSettings;
    B_DVR_Media media;
    B_DVR_MediaStorageType mediaStorageType;

    mediaStorageType = get_storage_device_type();
    if(mediaStorageType == eB_DVR_MediaStorageTypeMax) 
    {
        goto error;
    }

    /*
     * allocate app context
     */
    vod_ctx = malloc(sizeof(vod_ctx_t));
    assert(vod_ctx);
    memset(vod_ctx,0,sizeof(vod_ctx_t));

      /*
     * initialize nexus platform
     */
    rc = NEXUS_Platform_Init(NULL);
    if(rc) 
    {
        printf("\n NEXUS_Platform_Init failed \n ");
        free(vod_ctx);
        goto error;
    }
    
    /*
     * acquire a QAM frontend
     */
    NEXUS_Platform_GetConfiguration(&vod_ctx->platformconfig);

    NEXUS_Frontend_GetDefaultAcquireSettings(&acquireSettings);
    acquireSettings.capabilities.qam = true;
    vod_ctx->frontend = NEXUS_Frontend_Acquire(&acquireSettings);
    if(!vod_ctx->frontend) 
    {
        printf("\n No QAM frontend found");
        printf("\n app works with QAM source only at present");
        printf("\n use a platform with QAM source\n");
        NEXUS_Platform_Uninit();
        free(vod_ctx);
        goto error;
    }

    /*
     * create a lock change event
     */
    BKNI_CreateEvent(&vod_ctx->lockChangedEvent);
    assert(vod_ctx->lockChangedEvent);

    /*
     * tune to a channel
     */
    vod_ctx->parserBand = app_tune_channel(vod_ctx->frontend,
                                           vod_ctx->lockChangedEvent,
                                           &channelInfo);

    if(vod_ctx->parserBand == NEXUS_ParserBand_eInvalid)
    {
        printf("\n check and change channelInfo parameters for tuning \n");
        BKNI_DestroyEvent(vod_ctx->lockChangedEvent);
        NEXUS_Platform_Uninit();
        free(vod_ctx);
        goto error;
    }
    /*
     * open media storage
     */
    mediaStorageOpenSettings.storageType = mediaStorageType;
    vod_ctx->storage = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);
    assert(vod_ctx->storage);

    if(mediaStorageType == eB_DVR_MediaStorageTypeLoopDevice) 
    {
        unsigned volumeIndex=0;
        B_DVR_MediaStorageRegisterVolumeSettings registerSettings;
        memset(&registerSettings,0,sizeof(registerSettings));
        sprintf(registerSettings.device,VSFS_DEVICE);
        registerSettings.startSec = 0;
        registerSettings.length = VSFS_SIZE;
        B_DVR_MediaStorage_RegisterVolume(vod_ctx->storage,&registerSettings,&volumeIndex);
    }

    /*
     * check if the media storage is in segmented file 
     * format 
     */
    B_DVR_MediaStorage_GetStatus(vod_ctx->storage,&storageStatus);
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
        B_DVR_MediaStorage_Close(vod_ctx->storage);
        BKNI_DestroyEvent(vod_ctx->lockChangedEvent);
        NEXUS_Platform_Uninit();
        free(vod_ctx);
        goto error;
    }

    /*
     * mount media storage
     */
    rc = B_DVR_MediaStorage_MountVolume(vod_ctx->storage, STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    /*
     * initialize dvr manager
     */
    vod_ctx->manager = B_DVR_Manager_Init(NULL);
    assert(vod_ctx->manager);

    /*
     * create recording list if available
     */
    rc = B_DVR_Manager_CreateMediaNodeList(vod_ctx->manager,STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    /*
     * create app scheduler mutex
     */
    vod_ctx->schedulerMutex = B_Mutex_Create(NULL);
    assert(vod_ctx->schedulerMutex);

    /*
     * create app scheduler
     */
    vod_ctx->scheduler = B_Scheduler_Create(NULL);
    assert(vod_ctx->scheduler);

    /*
     * create app scheduler thread
     */
    vod_ctx->schedulerThread = B_Thread_Create("vod",vod_scheduler,vod_ctx, NULL);
    assert(vod_ctx->schedulerThread);
  
    /*
     * populate record service request
     */
    memset((void *)&recordServiceRequest,0,sizeof(B_DVR_RecordServiceRequest));
    sprintf(recordServiceRequest.programName,"%s",VOD_NAME);
    printf("\n recording name %s \n",recordServiceRequest.programName);
    recordServiceRequest.volumeIndex = STORAGE_VOLUME_INDEX;
    sprintf(recordServiceRequest.subDir,"%s",VOD_DIRECTORY);
    recordServiceRequest.recpumpIndex = NEXUS_ANY_ID;
    recordServiceRequest.input = B_DVR_RecordServiceInputStreamingVOD;

    /*
     * check if the meta data files already exist.
     * if yes, delete them
     */
    mediaNodeSettings.programName = recordServiceRequest.programName;
    mediaNodeSettings.subDir = recordServiceRequest.subDir;
    mediaNodeSettings.volumeIndex = recordServiceRequest.volumeIndex;
    rc = B_DVR_Manager_GetMediaNode(vod_ctx->manager,&mediaNodeSettings,&media);
    if(!rc) 
    {
        B_DVR_Manager_DeleteRecording(vod_ctx->manager,&mediaNodeSettings);
    }

    /*
     * Open record service
     */
    vod_ctx->recordService = B_DVR_RecordService_Open(&recordServiceRequest);
    assert(vod_ctx->recordService);

    vod_ctx->serviceCallback = (B_DVR_ServiceCallback)vod_callback;
    B_DVR_RecordService_InstallCallback(vod_ctx->recordService,
                                        vod_ctx->serviceCallback,
                                        (void*)vod_ctx);
    
    /*
     * open injection service
     */
    memset((void *)&injectOpenSettings,0,sizeof(injectOpenSettings));
    injectOpenSettings.fifoSize = 30*188; 
    vod_ctx->psiInfo.injectService  = B_DVR_DataInjectionService_Open(&injectOpenSettings);
    assert(vod_ctx->psiInfo.injectService);

    B_DVR_DataInjectionService_InstallCallback(vod_ctx->psiInfo.injectService,
                                               vod_ctx->serviceCallback,
                                               (void*)vod_ctx);

    /*
     * open injection PID channel
     */
    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    pidChannelSettings.pidChannelIndex = (NEXUS_NUM_PID_CHANNELS-2);
    pidChannelSettings.enabled = false;
    vod_ctx->psiInfo.injectChannel = NEXUS_PidChannel_Open(vod_ctx->parserBand,
                                                           0x1ffe,
                                                           &pidChannelSettings);

    assert(vod_ctx->psiInfo.injectChannel);

    /*
     * associate the injection channel with injection service
     */
    memset(&injectSettings,0,sizeof(injectSettings));
    injectSettings.dataInjectionServiceType = eB_DVR_DataInjectionServiceTypeRawData;
    injectSettings.pidChannel = vod_ctx->psiInfo.injectChannel ;
    rc = B_DVR_DataInjectionService_SetSettings(vod_ctx->psiInfo.injectService,&injectSettings);
    assert(rc == 0);

    /*
     * associate a parser band and injection service with record service
     */
    memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
    recordServiceSettings.dataInjectionService = vod_ctx->psiInfo.injectService;
    recordServiceSettings.parserBand = vod_ctx->parserBand;
    rc = B_DVR_RecordService_SetSettings(vod_ctx->recordService,&recordServiceSettings);
    assert(rc == 0);

    /*
     * add video PID to record service
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.vPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    inputEsStream.esStreamInfo.codec.videoCodec = channelInfo.vCodec;
    inputEsStream.pidChannel = NULL;
    rc = B_DVR_RecordService_AddInputEsStream(vod_ctx->recordService,&inputEsStream);
    assert(rc == 0);

    /*
     * add audio PID to record service
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.aPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    inputEsStream.esStreamInfo.codec.audioCodec = channelInfo.aCodec;
    rc = B_DVR_RecordService_AddInputEsStream(vod_ctx->recordService,&inputEsStream);
    assert(rc == 0);

    /*
     * add PCR PID to record service
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.pcrPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
    rc = B_DVR_RecordService_AddInputEsStream(vod_ctx->recordService,&inputEsStream);
    assert(rc == 0);

    /*
     * create PAT/PMT packets
     */
    app_create_psi(&vod_ctx->psiInfo,
                   channelInfo.pcrPid,
                   channelInfo.vPid,
                   channelInfo.aPid,
                   channelInfo.vCodec,
                   channelInfo.aCodec);

    /*
     * start record service
     */
    rc = B_DVR_RecordService_Start(vod_ctx->recordService,NULL);
    assert(rc == 0);

    /*
     * start injection
     */
    vod_ctx->psiInfo.recordService = vod_ctx->recordService;
    vod_ctx->psiInfo.psiStop = false;
    rc = B_DVR_RecordService_InjectDataStart(vod_ctx->recordService,
                                          (void *)&vod_ctx->psiInfo.pat[1],
                                          188);
    assert(rc == 0);
    vod_ctx->psiInfo.injectInProgress = true;

    /*
     * create a producer event used for reading data from 
     * the server 
     */
    vod_ctx->strm_ctx.producerEvent = B_Event_Create(NULL);
    assert(vod_ctx->strm_ctx.producerEvent);

    /*
     * create a consumer event used for writing the data into 
     * a network socket configured for a server-client 
     * connection 
     */
    vod_ctx->strm_ctx.consumerEvent = B_Event_Create(NULL);
    assert(vod_ctx->strm_ctx.consumerEvent);

    /*
     * scheduler event for producer
     */
    vod_ctx->strm_ctx.producerId =  B_Scheduler_RegisterEvent(vod_ctx->scheduler,
                                                                   vod_ctx->schedulerMutex,
                                                                   vod_ctx->strm_ctx.producerEvent,
                                                                   app_udp_producer,
                                                                   (void*)&vod_ctx->strm_ctx);
    assert(vod_ctx->strm_ctx.producerId);

    /*
     * scheduler event for consumer
     */
    vod_ctx->strm_ctx.consumerId =  B_Scheduler_RegisterEvent(vod_ctx->scheduler,
                                                                   vod_ctx->schedulerMutex,
                                                                   vod_ctx->strm_ctx.consumerEvent,
                                                                   app_udp_consumer,
                                                                   (void*)&vod_ctx->strm_ctx);
    assert(vod_ctx->strm_ctx.consumerId);

    /*
     * open an UDP socket connection between client and server
     */
    memcpy(&vod_ctx->strm_ctx.programSettings,&mediaNodeSettings,sizeof(mediaNodeSettings));
    vod_ctx->strm_ctx.recordService = vod_ctx->recordService;
    app_udp_open(&vod_ctx->strm_ctx,
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
    vod_ctx->strm_ctx.streamingStop = false;
    gettimeofday(&vod_ctx->strm_ctx.start,NULL);
    app_udp_producer((void *)&vod_ctx->strm_ctx);

    printf("\n press enter to quit VOD streaming");
    getchar();

    /*
     * prevent producer and consumer events 
     * from triggered and also prevent 
     * the data injection from triggering 
     * another packet injection 
     */
    B_Mutex_Lock(vod_ctx->schedulerMutex);
    vod_ctx->strm_ctx.streamingStop = true;
    vod_ctx->psiInfo.psiStop = true;
    B_Mutex_Unlock(vod_ctx->schedulerMutex);

     /*
     * de-register the consumer and producer 
     * events from the app scheduler
     */
    B_Scheduler_UnregisterEvent(vod_ctx->scheduler,
                                vod_ctx->strm_ctx.consumerId);
    B_Scheduler_UnregisterEvent(vod_ctx->scheduler,
                                vod_ctx->strm_ctx.producerId);

    /*
     * destroy producer and consumer events
     */
    B_Event_Destroy(vod_ctx->strm_ctx.producerEvent);
    B_Event_Destroy(vod_ctx->strm_ctx.consumerEvent);

    /*
     * close the UDP socket connection between 
     * client and server 
     */
    app_udp_close(&vod_ctx->strm_ctx);

    /*
     * stop data injection service
     */
    if(vod_ctx->psiInfo.injectInProgress) 
    {
        if(vod_ctx->psiInfo.timer)
        {
            B_Scheduler_CancelTimer(vod_ctx->scheduler, 
                                    vod_ctx->psiInfo.timer);
        }
        B_DVR_RecordService_InjectDataStop(vod_ctx->recordService);
    }

    /*
     * close data injection pid channel
     */
    NEXUS_PidChannel_Close(vod_ctx->psiInfo.injectChannel);
    B_DVR_DataInjectionService_RemoveCallback(vod_ctx->psiInfo.injectService);

    /*
     * stop record service
     */
    B_DVR_RecordService_Stop(vod_ctx->recordService);

    /*
     * close injection service
     */
    B_DVR_DataInjectionService_Close(vod_ctx->psiInfo.injectService);

    /*
     * close record service
     */
    B_DVR_RecordService_RemoveCallback(vod_ctx->recordService);
    B_DVR_RecordService_Close(vod_ctx->recordService);

    /*
     * destroy lock change event
     */
    BKNI_DestroyEvent(vod_ctx->lockChangedEvent);

    /*
     * stop app scheduler
     */
    B_Scheduler_Stop(vod_ctx->scheduler);

    /*
     * stop app thread
     */
    B_Thread_Destroy(vod_ctx->schedulerThread);

    /*
     * destroy app scheduler 
     */
    B_Scheduler_Destroy(vod_ctx->scheduler);

    /*
     * destroy app scheduler mutex
     */
    B_Mutex_Destroy(vod_ctx->schedulerMutex);

    /*
     * destroy list of recordings if available
     */
    B_DVR_Manager_DestroyMediaNodeList(vod_ctx->manager,STORAGE_VOLUME_INDEX);

    /*
     * de-initialize dvr manager
     */
    B_DVR_Manager_UnInit(vod_ctx->manager);

    /*
     * unmount media storage
     */
    B_DVR_MediaStorage_UnmountVolume(vod_ctx->storage, STORAGE_VOLUME_INDEX);

    /*
     * close media storage
     */
    B_DVR_MediaStorage_Close(vod_ctx->storage);

    /*
     * uninitialize nexus platform
     */
    NEXUS_Platform_Uninit();

    free(vod_ctx);
error:
    return 0;
}
