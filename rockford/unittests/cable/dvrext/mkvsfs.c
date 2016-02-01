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
 mksfs - make virtual segmented file system
 An app to create virtual segmented file system using conventional linux file system APIs.
 App provides a directory in an existing partition with some some linux file system
 and also the size (MB) that can be used by the dvr library.
 On this app provided directory like /data/vsfs,
 media storage will pre-allocate 3 files 1. /data/vsfs/media 2. /data/vsfs/navigation
 3./data/vsfs/metadata in some ratio of the size user provided.
 These files would be used as loop devices which would be used as virtual partitions
 by the media storage. On these virtual partititons, generic Linux file systems would
 be created. Rest of details on how the SFS/VSFS manages files for recording are
 available in dvrlib.pdf
 usage:
 ./nexus mkvsfs
 */

#include "util.h"
#include <sys/vfs.h> /* or <sys/statfs.h> */

int main(void)
{
    B_DVR_MediaStorageHandle mStorage;
    B_DVR_MediaStorageRegisterVolumeSettings registerSettings;
    B_DVR_MediaStorageStatus mStorageStatus;
    B_DVR_MediaStorageOpenSettings openSettings;
    struct statfs buf_stat;
    unsigned i=VOLUME_INDEX;
    int ret=0;
    ret = statfs(MEDIA_DEVICE_MOUNT_PATH,&buf_stat);
    if (ret < 0) 
    {
        printf("\n /data directory needs to be created before running this app \n");
        goto error;
    }
    else
    {   
        uint64_t totalSize,freeSize;
        totalSize = buf_stat.f_blocks*buf_stat.f_bsize;
        printf("\n file system size ->");
        printf("%lld bytes \n",totalSize );
        totalSize/=1024*1024; /* size in MB */
        printf("\n file system free size ->");
        freeSize = buf_stat.f_bfree*buf_stat.f_bsize;
        printf("%lld bytes \n",freeSize);
        freeSize/=1024*1024; /* size in MB */
        if(freeSize < VSFS_SIZE) 
        {
            printf("\n Required free space 50GB for VSFSF not available");
            goto error;
        }
    }
    B_Os_Init();
    BKNI_Init();
    openSettings.storageType = eB_DVR_MediaStorageTypeLoopDevice;
    mStorage = B_DVR_MediaStorage_Open(&openSettings);
    assert(mStorage);
    memset(&registerSettings,0,sizeof(registerSettings));
    sprintf(registerSettings.device,VSFS_DEVICE);
    registerSettings.startSec = 0;
    registerSettings.length = VSFS_SIZE;
    B_DVR_MediaStorage_RegisterVolume(mStorage,&registerSettings,&i);
    B_DVR_MediaStorage_FormatVolume(mStorage,i);
    B_DVR_MediaStorage_GetStatus(mStorage,&mStorageStatus);
    printf("\n numRegisteredVolumes %u",mStorageStatus.numRegisteredVolumes);
    for(i = 0;i < (unsigned)mStorageStatus.numRegisteredVolumes;i++) 
    {
        printf("\n device:%s",mStorageStatus.volumeInfo[i].device);
        printf("\n registered:%u",mStorageStatus.volumeInfo[i].registered);
        printf("\n formatted:%u",mStorageStatus.volumeInfo[i].formatted);
        printf("\n media Partition:%s",mStorageStatus.volumeInfo[i].mediaPartition);
        printf("\n nav Partition:%s",mStorageStatus.volumeInfo[i].navPartition);
        printf("\n metadata Partition:%s",mStorageStatus.volumeInfo[i].metadataPartition);
    }
    B_DVR_MediaStorage_Close(mStorage);
    B_Os_Uninit();
    BKNI_Uninit();
error:
    return 0;
}
