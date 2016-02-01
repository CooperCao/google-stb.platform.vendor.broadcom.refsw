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
 *  stream_permrecording:
 *  App that streams a segmented permanent recording from a
 *  VMS server to a VLC client running on a PC [Window/Linux machine]
 *  App requires that the user runs the tsbconvert app before
 *  streaming. The network streaming (UDP) support is rudimentary and can
 *  be optimized further. This app demonstrates the usage of dvr library
 *  media file APIs for streaming a permanent recording. User has
 *  to update the client ip and server ethernet interface
 *  based on the test environment.
 *  
 *  usage:
 *  ./nexus stream_permrecording
 *
 *  On client
 *  1. launch VLC player.
 *  2. Click on media drop down menu
 *  3. open network stream
 *  4. under URL udp://@xxx.xxx.xxx.xxx:5000
 *  5. click on show more options
 *  6. adjust the caching for jitter adjustment
 *  7. play

 */

#include "util.h"

/* 
 *  The below parameters are the default parameters used by the
 *  stream_permrecording application. These might need modifications
 *  by the user based on a specific test environment.
 */

#define PERM_RECORDING_NAME "tsbConverted_0"
#define PERM_RECORDING_DIRECTORY "tsbConv"
#define CLIENT_PORT 5000
#define CLIENT_IP "10.22.10.31"
#define SERVER_NET_IF "eth2" /* server ethernet interface 
                                On BCM97425VMS_SFF board, use CFE command ephycfg with option 4
                                to configure the eth0 to use the Giga bit switch 
                              */

typedef struct strm_perm_rec_t
{
    strm_ctx_t strm_ctx;
    B_DVR_ManagerHandle manager;
    B_DVR_MediaStorageHandle storage;
    B_MutexHandle schedulerMutex;
    B_SchedulerHandle scheduler;
    B_ThreadHandle schedulerThread;
}strm_perm_rec_t;

strm_perm_rec_t *strm_perm_rec = NULL;

void stream_perm_rec_scheduler(void * param)
{
    BSTD_UNUSED(param);
    B_Scheduler_Run(strm_perm_rec->scheduler);
    return;
}

int main(void)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    B_DVR_MediaStorageStatus storageStatus;
    B_DVR_Media media;
    char programName[B_DVR_MAX_FILE_NAME_LENGTH]=PERM_RECORDING_NAME;
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH]=PERM_RECORDING_DIRECTORY;
    B_DVR_MediaStorageType mediaStorageType;

    mediaStorageType = get_storage_device_type();
    if(mediaStorageType == eB_DVR_MediaStorageTypeMax) 
    {
        goto error;
    }

    B_Os_Init();
    BKNI_Init();

    /*
     * allocate app context
     */
    strm_perm_rec = malloc(sizeof(strm_perm_rec_t));
    assert(strm_perm_rec);
    memset(strm_perm_rec,0,sizeof(strm_perm_rec_t));

    /*
     * open media storage
     */
    mediaStorageOpenSettings.storageType = mediaStorageType;
    strm_perm_rec->storage = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);
    assert(strm_perm_rec->storage);

    if(mediaStorageType == eB_DVR_MediaStorageTypeLoopDevice) 
    {
        unsigned volumeIndex=0;
        B_DVR_MediaStorageRegisterVolumeSettings registerSettings;
        memset(&registerSettings,0,sizeof(registerSettings));
        sprintf(registerSettings.device,VSFS_DEVICE);
        registerSettings.startSec = 0;
        registerSettings.length = VSFS_SIZE;
        B_DVR_MediaStorage_RegisterVolume(strm_perm_rec->storage,&registerSettings,&volumeIndex);
    }

    /*
     * check if the media storage is formatted with 
     * segmented file system format 
     */
    B_DVR_MediaStorage_GetStatus(strm_perm_rec->storage,&storageStatus);
    if(!storageStatus.numRegisteredVolumes || 
       !storageStatus.volumeInfo[VOLUME_INDEX].formatted) 
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
        B_DVR_MediaStorage_Close(strm_perm_rec->storage);
        free(strm_perm_rec);
        goto error;
    }

    /*
     * mount media storage partitions
     */
    rc = B_DVR_MediaStorage_MountVolume(strm_perm_rec->storage, STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    /*
     * initialize dvr manager
     */
    strm_perm_rec->manager = B_DVR_Manager_Init(NULL);
    assert(strm_perm_rec->manager);

    /*
     * create recording list if available
     */
    rc = B_DVR_Manager_CreateMediaNodeList(strm_perm_rec->manager,STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    /*
     * populate the program info
     */
    strm_perm_rec->strm_ctx.programSettings.programName = programName;
    strm_perm_rec->strm_ctx.programSettings.subDir = subDir;
    strm_perm_rec->strm_ctx.programSettings.volumeIndex = STORAGE_VOLUME_INDEX;

    B_DVR_Manager_GetMediaNode(strm_perm_rec->manager,&strm_perm_rec->strm_ctx.programSettings,&media);
    if(strcmp(strm_perm_rec->strm_ctx.programSettings.programName,media.programName))
    {
        printf("\n program %s not found",programName);
        printf("\n run tsbconvert app to generate program: %s\n",programName);
        goto error_program;
    }

    /* 
     * create app scheduler mutex
     */
    strm_perm_rec->schedulerMutex = B_Mutex_Create(NULL);
    assert(strm_perm_rec->schedulerMutex);

    /* 
     * create app scheduler
     */
    strm_perm_rec->scheduler = B_Scheduler_Create(NULL);
    assert(strm_perm_rec->scheduler);

    /* 
     *  create the scheduler thread
     */
    strm_perm_rec->schedulerThread = B_Thread_Create("stream_perm_rec",stream_perm_rec_scheduler,strm_perm_rec, NULL);
    assert(strm_perm_rec->schedulerThread);


     /*
     * create a producer event used for reading data from 
     * the server 
     */
    strm_perm_rec->strm_ctx.producerEvent = B_Event_Create(NULL);
    assert(strm_perm_rec->strm_ctx.producerEvent);

    /*
     * create a consumer event used for writing the data into 
     * a network socket configured for a server-client 
     * connection 
     */
    strm_perm_rec->strm_ctx.consumerEvent = B_Event_Create(NULL);
    assert(strm_perm_rec->strm_ctx.consumerEvent);

    /*
     * scheduler event for producer
     */
    strm_perm_rec->strm_ctx.producerId =  B_Scheduler_RegisterEvent(strm_perm_rec->scheduler,
                                                                   strm_perm_rec->schedulerMutex,
                                                                   strm_perm_rec->strm_ctx.producerEvent,
                                                                   app_udp_producer,
                                                                   (void*)&strm_perm_rec->strm_ctx);
    assert(strm_perm_rec->strm_ctx.producerId);

    /*
     * scheduler event for consumer
     */
    strm_perm_rec->strm_ctx.consumerId =  B_Scheduler_RegisterEvent(strm_perm_rec->scheduler,
                                                                   strm_perm_rec->schedulerMutex,
                                                                   strm_perm_rec->strm_ctx.consumerEvent,
                                                                   app_udp_consumer,
                                                                   (void*)&strm_perm_rec->strm_ctx);
    assert(strm_perm_rec->strm_ctx.consumerId);

    /*
     * open an UDP socket connection between client and server
     */
    app_udp_open(&strm_perm_rec->strm_ctx,
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
    strm_perm_rec->strm_ctx.streamingStop = false;
    gettimeofday(&strm_perm_rec->strm_ctx.start,NULL);
    app_udp_producer((void *)&strm_perm_rec->strm_ctx);

    printf("\n press enter to quit tsb streaming");
    getchar();

    /*
     * prevent producer and consumer events 
     * from triggered 
     */
    B_Mutex_Lock(strm_perm_rec->schedulerMutex);
    strm_perm_rec->strm_ctx.streamingStop = true;
    B_Mutex_Unlock(strm_perm_rec->schedulerMutex);

    /*
     * de-register the consumer and producer 
     * events from the app scheduler
     */
    B_Scheduler_UnregisterEvent(strm_perm_rec->scheduler,
                                strm_perm_rec->strm_ctx.producerId);
    B_Scheduler_UnregisterEvent(strm_perm_rec->scheduler,
                                strm_perm_rec->strm_ctx.consumerId);

    /*
     * destroy producer and consumer events
     */
    B_Event_Destroy(strm_perm_rec->strm_ctx.producerEvent);
    B_Event_Destroy(strm_perm_rec->strm_ctx.consumerEvent);

    /*
     * close the UDP socket connection between 
     * client and server 
     */
    app_udp_close(&strm_perm_rec->strm_ctx);
    
    /*
     * stop the app scheduler
     */
    B_Scheduler_Stop(strm_perm_rec->scheduler);
    B_Scheduler_Destroy(strm_perm_rec->scheduler);
    B_Thread_Destroy(strm_perm_rec->schedulerThread);
    B_Mutex_Destroy(strm_perm_rec->schedulerMutex);

error_program:
    /*
     *  destroy the list of recordings if available
     */
    B_DVR_Manager_DestroyMediaNodeList(strm_perm_rec->manager,STORAGE_VOLUME_INDEX);

    /*
     * de-initialize the dvr manager
     */
    B_DVR_Manager_UnInit(strm_perm_rec->manager);

    /*
     * unmount the media storage
     */
    B_DVR_MediaStorage_UnmountVolume(strm_perm_rec->storage, STORAGE_VOLUME_INDEX);

    /*
     * close media storage
     */
    B_DVR_MediaStorage_Close(strm_perm_rec->storage);

    B_Os_Uninit();
    BKNI_Uninit();
    free(strm_perm_rec);
error:
    return 0;
}


