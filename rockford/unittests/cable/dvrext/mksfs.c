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
 *****************************************************************************/

/*
 mksfs - make segmented file system
 An app to create segmented file system using conventional linux file system APIs.
 Contents in /dev/sda would be destroyed using this applications and a new parition table
 and partitions and file system are created.
 Refer to dvrlib.pdf for detailed description of segmented file system.
 usage:
 ./nexus mksfs
 */

#include "util.h"

int main(void)
{
    B_DVR_MediaStorageHandle mStorage;
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    B_DVR_MediaStorageRegisterVolumeSettings registerSettings;
    B_DVR_MediaStorageStatus mStorageStatus;
    unsigned i=VOLUME_INDEX;
    unsigned proceed=0;
    printf("\n****************WARNING*************************");
    printf("\n running mksfs would zap %s storage device",STORAGE_DEVICE);
    printf("\n in order to mksfs working, rootfs must be on nor/nand flash partition");
    printf("\n all the data in storage device would be lost");
    printf("\n****************WARNING*************************");
    printf("\n do you want to proceed? -> 0 [no] / 1 [yes] : ");
    scanf("%u",&proceed);
    if(!proceed)
    {
        goto error;
    }
    /* create GPT based partition table */
    system(STORAGE_ZAP_COMMAND);
    B_Os_Init();
    BKNI_Init();
    mediaStorageOpenSettings.storageType = eB_DVR_MediaStorageTypeBlockDevice;
    mStorage = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);
    assert(mStorage);
    memset(&registerSettings,0,sizeof(registerSettings));
    sprintf(registerSettings.device,STORAGE_DEVICE);
    registerSettings.startSec = 0;
    B_DVR_MediaStorage_RegisterVolume(mStorage,&registerSettings,&i);
    B_DVR_MediaStorage_FormatVolume(mStorage,i);
    B_DVR_MediaStorage_GetStatus(mStorage,&mStorageStatus);
    printf("\n maxVolumeNumber %u",mStorageStatus.maxVolumeNumber);
    printf("\n maxVolumeNumber %u",mStorageStatus.numMountedVolumes);
    printf("\n numRegisteredVolumes %u",mStorageStatus.numRegisteredVolumes);
    for(i = 0;i < (unsigned)mStorageStatus.numRegisteredVolumes;i++) 
    {
        printf("\n device:%s",mStorageStatus.volumeInfo[i].device);
        printf("\n startSector:%u", (unsigned)mStorageStatus.volumeInfo[i].startSector);
        printf("\n endSector:%u",(unsigned)mStorageStatus.volumeInfo[i].endSector);
        printf("\n registered:%u",mStorageStatus.volumeInfo[i].registered);
        printf("\n formatted:%u",mStorageStatus.volumeInfo[i].formatted);
        printf("\n mounted:%u",mStorageStatus.volumeInfo[i].mounted);
        printf("\n mountName:%s",mStorageStatus.volumeInfo[i].mountName);
        printf("\n media Partition:%s",mStorageStatus.volumeInfo[i].mediaPartition);
        printf("\n nav Partition:%s",mStorageStatus.volumeInfo[i].navPartition);
        printf("\n metadata Partition:%s",mStorageStatus.volumeInfo[i].metadataPartition);
    }
    B_DVR_MediaStorage_Close(mStorage);
    system(STORAGE_PRINT_PARTITION_TABLE);
    B_Os_Uninit();
    BKNI_Uninit();
    return 0;
error:
    printf("\n mksfs didn't run");
    return 0;   
}
