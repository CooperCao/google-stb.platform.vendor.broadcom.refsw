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
 * record:
 * App demonstrates background in segmented file format.  
 */

#include "util.h"
/* 
 *  application may or maynot create these sub groups. All the example applications are using the below
 *  sub groups. sub group creation request is sent through service open calls. Application typically
 *  remembers the sub group to speed up the searching of programs.
 */
char bgRecDir[B_DVR_MAX_FILE_NAME_LENGTH] = "bgrec"; /* back ground recording sub group. */
char tsbConvDir[B_DVR_MAX_FILE_NAME_LENGTH] = "tsbConv"; /* tsb converted recording sub group */

B_DVR_Media media;
int main(void)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    B_DVR_MediaStorageStatus storageStatus;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];
    unsigned recordCount = 0;
    B_DVR_ManagerHandle manager;
    B_DVR_MediaStorageHandle storage;
    B_DVR_MediaStorageType mediaStorageType;

    mediaStorageType = get_storage_device_type();
    if(mediaStorageType == eB_DVR_MediaStorageTypeMax) 
    {
        goto error;
    }
    B_Os_Init();
    BKNI_Init();
    /* 
     * open media storage
     */
    mediaStorageOpenSettings.storageType = mediaStorageType;
    storage = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);
    assert(storage);

    if(mediaStorageType == eB_DVR_MediaStorageTypeLoopDevice) 
    {
        unsigned volumeIndex=0;
        B_DVR_MediaStorageRegisterVolumeSettings registerSettings;
        memset(&registerSettings,0,sizeof(registerSettings));
        sprintf(registerSettings.device,VSFS_DEVICE);
        registerSettings.startSec = 0;
        registerSettings.length = VSFS_SIZE;
        B_DVR_MediaStorage_RegisterVolume(storage,&registerSettings,&volumeIndex);
    }

    /* 
     * check if the media storage is in segmented file system format
     */
    B_DVR_MediaStorage_GetStatus(storage,&storageStatus);
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
        B_DVR_MediaStorage_Close(storage);
        goto error1;
    }

    /* 
     *  mount media storage -> media, nav and metadata partitions
     */
    rc = B_DVR_MediaStorage_MountVolume(storage, STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    /* 
     *  initialize the DVR manager
     */
    manager = B_DVR_Manager_Init(NULL);
    assert(manager);

    /* 
     *  create the list of recording if available
     */
    rc = B_DVR_Manager_CreateMediaNodeList(manager,STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    mediaNodeSettings.subDir = tsbConvDir;
    mediaNodeSettings.programName = NULL;
    mediaNodeSettings.volumeIndex = STORAGE_VOLUME_INDEX;
    recordCount = B_DVR_Manager_GetNumberOfRecordings(manager,&mediaNodeSettings);
    printf("\n number of tsb Converted recordings %u",recordCount);
    if(recordCount)
    {
        unsigned index = 0;
        recordingList = malloc(sizeof(char *)*recordCount*B_DVR_MAX_FILE_NAME_LENGTH);
        B_DVR_Manager_GetRecordingList(manager,&mediaNodeSettings,recordCount,recordingList);
        printf("\n********************************************************************");
        printf("\n  TSB converted recording list                               ");
        printf("\n********************************************************************");
        for(index=0;index<recordCount;index++)
        {
            memset(&media,0,sizeof(media));
            mediaNodeSettings.programName = recordingList[index];
            mediaNodeSettings.subDir = tsbConvDir;
            mediaNodeSettings.volumeIndex = STORAGE_VOLUME_INDEX;
            B_DVR_Manager_GetMediaNode(manager,&mediaNodeSettings,&media);
            printf("\n %u: program: %s",index,recordingList[index]);
            app_print_rec_metadata(&media);
            printf("press enter to get info another program");
            getchar();
         }
         printf("\n\n");
         free(recordingList);
    }
    else
    {
        printf("\n no tsb converted recording available on the storage media");
    }


    mediaNodeSettings.subDir = bgRecDir;
    mediaNodeSettings.programName = NULL;
    mediaNodeSettings.volumeIndex = STORAGE_VOLUME_INDEX;
    recordCount = B_DVR_Manager_GetNumberOfRecordings(manager,&mediaNodeSettings);
    printf("\n number of background recordings %u",recordCount);
    if(recordCount)
    {
        unsigned index = 0;
        recordingList = malloc(sizeof(char *)*recordCount*B_DVR_MAX_FILE_NAME_LENGTH);
        B_DVR_Manager_GetRecordingList(manager,&mediaNodeSettings,recordCount,recordingList);
        printf("\n********************************************************************");
        printf("\n  background recording list                               ");
        printf("\n********************************************************************");
        for(index=0;index<recordCount;index++)
        {
            memset(&media,0,sizeof(media));
            mediaNodeSettings.programName = recordingList[index];
            mediaNodeSettings.subDir = bgRecDir;
            mediaNodeSettings.volumeIndex = STORAGE_VOLUME_INDEX;
            B_DVR_Manager_GetMediaNode(manager,&mediaNodeSettings,&media);
            printf("\n %u: program: %s",index,recordingList[index]);
            app_print_rec_metadata(&media);
            printf("press enter to get info another program");
            getchar();
         }
         printf("\n\n");
         free(recordingList);
    }
    else
    {
        printf("\n no background recording available on the storage media");
    }

    B_DVR_Manager_DestroyMediaNodeList(manager,STORAGE_VOLUME_INDEX);
    B_DVR_Manager_UnInit(manager);
    B_DVR_MediaStorage_UnmountVolume(storage, STORAGE_VOLUME_INDEX);
    B_DVR_MediaStorage_Close(storage);
error1:
    B_Os_Uninit();
    BKNI_Uninit();
error:
    return 0;
}


