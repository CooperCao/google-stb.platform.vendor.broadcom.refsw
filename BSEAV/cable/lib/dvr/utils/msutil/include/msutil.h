/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY..
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 * Media storage disk utility
 * 
 * msutil gives utility functions to for dvr extension library media storage.
 * it does create, format, mount and unmount. it is wrtten only with standard libraries 
 * so that it can be compiled and used as a stand alone utility.
 * msapp.c gives user interface.
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/

#ifndef _MS_PARTITION_H
#define _MS_PARTITION_H

#include <stdint.h>

/*
    msutil.h - media storage utility
*/
    
#if 1
#define NO_REGFILE
#endif

#define MS_VERSION                  0x00020000   /* version 2.0 */
#define MS_MEDIA_SEGMENT_SIZE    (188*4096*200)   /* from b_dvr_const.h. do not change!!
                                                   this should be the same with B_DVR_MEDIA_SEGMENT_SIZE */

#define MS_GUID_MEDIA_PARTITION    "4D454449-4120-5041-5054-5954494F4E00"  
#define MS_GUID_NAV_PARTITION      "4E415620-5041-5054-5954-494F4E000000"
#define MS_GUID_META_PARTITION     "4D455441-2050-4150-5459-54494F4E0000"
#define MS_GUID_LABEL_PARTITION    "44565220-5041-5254-4954-000000000000"

#if 0
#define _USE_EXT2
#endif

#ifdef _USE_EXT2
#define MS_MEDIA_FORMAT_CMD         "mkfs.ext2 -N 100000 -O dir_index"
#define MS_MEDIA_FSCK_CMD           "e2fsck -p"
#define MS_MEDIA_FILESYSTEMTYPE     "ext2"
#define MS_VIRTUAL_FORMAT_CMD         "mkfs.ext2 -F -N 100000 -O dir_index"
#else
#define MS_MEDIA_FORMAT_CMD         "mkfs.ext4 -N 100000 -O dir_index"
#define MS_MEDIA_FSCK_CMD           "e2fsck -p -E journal_only"
#define MS_MEDIA_FILESYSTEMTYPE     "ext4"
#define MS_VIRTUAL_FORMAT_CMD         "mkfs.ext4 -F -O dir_index"
#endif

#define MS_MAX_DVRVOLUME            4
#define MS_NAV_FILESYSTEMTYPE       "ext4"
#define MS_META_FILESYSTEMTYPE      "ext4"

#define MS_MAX_SEGMENTS             30000
#define MS_MAX_PATHLEN              128
#define MS_GUID_LENGTH              40
#define MS_DEVICE_NAME_LEN          256
#define MS_MAX_CMDLEN               256
#define MS_MAX_OPTIONBUF            1000
#define MS_INVALID_INDEX            0
#define MS_GPT_LIST_CMD             "sgdisk -p %s | sed -e '1,/Number.*Start.*End.*Size/d' >%s"
#define MS_GPT_GUID_GET_CMD         "sgdisk -i %d %s | grep unique > %s"

#define MS_SGDISK_TEMP_FILE         "/tmp/sgdisktmp.txt"
#define MS_SGDISK_GUID_TEMP_FILE    "/tmp/sguidtmp.txt"

#define MS_SGDISK_LINE_SIZE         (128)
#define MS_MBYTE_TO_SECTORS(X)      ((X)*2048)
#define MS_NAV_PT_SECTORS(X,Y)      ((Y-X)/120) /* reserve 0.83% for navigation partition */
#define MS_META_PT_SIZE             (2*1024)  /* reserve 2G for navigation partition */
#define MS_MIN_VOL_SIZE             (10*1024) /* minimum volume size */
#define MS_RESERVE_SEG(X)           ((9985*X)/10000) /* reserve 0.15% */
#define MS_VIRTUAL_RESERVE_SEG(X)   ((95*X)/100) /* reserve 5% */
#define MS_MAX_PARTITION_ARRAY         10
#define MS_SECTOR_BOUNDARY          2048
#define MS_MAX_KEYSIZE              64
#define MS_MAX_LABELSIZE            1024
#define DMA_JOBS    1

#define MS_REGISTRY_FILENAME        ".msreg"
#define MS_MSFSCKLOG_FILENAME       ".msfsck.log"
#define MS_MSFSCKLOG_FILENAME_BACKUP  ".msfsck.log.0"
#define MS_QUOTA_FILENAME           ".msreg_sub"
#define MS_METAFILE_EXT_INFO        ".info"
#define MS_METAFILE_EXT_TS          ".ts"
#define MS_METAFILE_EXT_NAV         ".nav"

#define MS_LOOP_DEVICE_MEDIA  "media"
#define MS_LOOP_DEVICE_NAVIGATION   "navigation"
#define MS_LOOP_DEVICE_METADATA "metadata"

typedef enum {
    eMS_StateInvalid=1,     /* not a media volume */
    eMS_StateEmpty,         /* partitions are ready but no media files */
    eMS_StateReady,         /* all set */
    eMS_StateMax
} MS_StorageState_t;

typedef struct {
    char device[MS_DEVICE_NAME_LEN];    /* device file name */
    char guid[MS_GUID_LENGTH];          /* guid in char type with dash */
    char name[MS_MAX_PATHLEN];          /* disk label */
    int partitionNum;
    uint64_t start;	                    /* start sector */
    uint64_t end;		                /* end sector */
    uint16_t type;			            /* partition type */
} MS_Partition_t;

typedef enum {
    eMS_MediaSegment,
    eMS_NavSegment
} MS_SegmentType_t;

/*
    3 partitions will be created within this range
*/
typedef struct {
    uint64_t startSec;  /* start sector */
    uint64_t size;      /* number of sectors */
    char mediaPartition[MS_DEVICE_NAME_LEN];   /* output: name of the partition device file for media */
    char navPartition[MS_DEVICE_NAME_LEN];     /* output: name of the partition device file navigation */
    char metadataPartition[MS_DEVICE_NAME_LEN];    /* output: name of the partition device file metadata */
} MS_createPartitionSettings_t;

/*
    data written in the <mount name>-metadata/.msreg
*/
typedef struct {
    uint32_t version;
    uint8_t mediaSegRefCount[MS_MAX_SEGMENTS];    /* referece counter for media segments */
    uint8_t navSegRefCount[MS_MAX_SEGMENTS];      /* referece counter for media segments */
    uint32_t mediaSegmentSize;          /* media segment size */
    uint32_t numberOfSegments;
} MS_Registry_t;

typedef struct {
    uint16_t quota;         /* number of assigned segments to a dir*/
    uint16_t used;          /* number of used segments in this dir*/
    uint16_t shared;        /* number of shared segments. not actually consuming physical segments */
} MS_DirQuota_t;

enum {
    eMS_OK = 0,
    eMS_ERR_INVALID_PARAM = -1000,
    eMS_ERR_INVALID_VOLUME,
    eMS_ERR_INVALID_RANGE,
    eMS_ERR_SYSTEM,
    eMS_ERR_PARTITION_FULL,
    eMS_ERR_EXT_EXISTS,
    eMS_ERR_VOLUME_EXISTS,
    eMS_ERR_NOT_SUPPORTED,
    eMS_ERR_PROXY,
    eMS_ERR_WRONG_LABEL,
    eMS_ERR_MAX
};

typedef struct {
    MS_StorageState_t state;           
    char mediaPartition[MS_DEVICE_NAME_LEN];       /* name of media partition. valid only if state is not invalid*/
    char navPartition[MS_DEVICE_NAME_LEN];         /* name of nav partition. valid only if state is not invalid*/
    char metadatapartition[MS_DEVICE_NAME_LEN];    /* name of metadata partition. valid only if state is not invalid*/
    uint64_t startSector;                          /* starting sector of media partition */
    uint64_t endSector;                            /* end sector of metadata partition */
    uint32_t mediaSegmentSize;                     /* media segment size */ 
    uint32_t mediaSegTotal;                        /* total number of media segments */
    uint32_t mediaSegInuse;                        /* number of media segments in use */
} MS_StorageStatus_t;

/* 
    ms_init
        initializae media storage utility
    parameters
    return
        eMS_OK
        eMS_ERR_INVALID_PARAM
*/
int ms_init(void);

/*
ms_check_virtual_volume
    check the volume state.
    invalid-create and format to use as a media volume
    empty-format needed
    ready-just mount and use!
parameters
    device[in]:directory on an existing partition with a file 
    result[out]: status
return
    eMS_OK
    eMS_ERR_INVALID_PARAM
*/
int ms_check_virtual_volume(
        const char *device, 
        MS_StorageStatus_t *status);
/*
ms_check_volume
    check the volume state.
    invalid-create and format to use as a media volume
    empty-format needed
    ready-just mount and use!
parameters
    device[in]:device name
    result[out]: status
return
    eMS_OK
    eMS_ERR_INVALID_PARAM
*/
int ms_check_volume(
        const char *device, 
        MS_StorageStatus_t *status);

/*
ms_format_virtual_volume
    format empty or ready volume. cannot format invalid volume.
    it will do
    - format file system
    - create media directories
    - create media files(segments and registry)
    volume state will be "ready" after format
parameters
    mediaLoopDevice[in]
    navLoopDevice[in]
    metaLoopDevice[in]
    segments[out] number of segment files 
return
    eMS_OK
    eMS_ERR_INVALID_PARAM
    eMS_ERR_INVALID_VOLUME
*/
int ms_format_virtual_volume(
        const char *mediaLoopDevice,
        const char *navLoopDevice,
        const char *metaLoopDevice,
        unsigned long *segments);

/*
ms_format_volume
    format empty or ready volume. cannot format invalid volume.
    it will do
    - format file system
    - create media directories
    - create media files(segments and registry)
    volume state will be "ready" after format
parameters
    device[in]device name
    segments[out] number of segment files 
return
    eMS_OK
    eMS_ERR_INVALID_PARAM
    eMS_ERR_INVALID_VOLUME
*/
int ms_format_volume(
        const char *device,
        unsigned long *segments);

/*
ms_mount_virtual_volume
    mount media volume. mount name is automatically given as /mnt/dvr<number>
parameters
    mediaLoopDevice[in]
    navLoopDevice[in]
    metaLoopDevice[in]
    mountname[out]: mount name
    deletedFileCount[out]: number of deleted file by housecleaning
return
*/
int ms_mount_virtual_volume(
        const char *mediaLoopDevice, 
        const char *navLoopDevice, 
        const char *metaLoopDevice, 
        char *mountname,
        int *deletedFileCount);

/*
ms_mount_volume
    mount media volume. mount name is automatically given as /mnt/dvr<number>
parameters
    device[in]: device
    mountname[out]: mount name
    deletedFileCount[out]: number of deleted file by housecleaning
return
*/
int ms_mount_volume(
        const char *device, 
        char *mountname,
        int *deletedFileCount);


/*
ms_unmount_volume
    unmount media volume
parameters
    mountname[in]: mount name
return
*/
int ms_unmount_volume(
        const char *mountname);

/*
ms_unmount_device
    unmount media volume by device name
parameters
    device[in]: device
return
*/
int ms_unmount_device(
        const char *device);

/*
ms_create_volume
    create a new volume
parameters
    device[in]: a directory on an existing parition with a file system
    pSettings[in]:set start and size. 0 can be given for default.

*/
int ms_create_virtual_volume(
        const char *device, 
        MS_createPartitionSettings_t *pSettings);

/*
ms_create_volume
    create a new volume
parameters
    device[in]: device
    pSettings[in]:set start and size. 0 can be given for default.

*/
int ms_create_volume(
        const char *device, 
        MS_createPartitionSettings_t *pSettings);


/*
ms_read_registry
    read registry 
parameters
    device[in]: device
    pSettings[in]:set start and size. 0 can be given for default.

*/
int ms_read_registry(
        const char *mountname, 
        MS_Registry_t *registry);


/*
ms_update_registry
    update registry
parameters
    device[in]: device
    pSettings[in]:set start and size. 0 can be given for default.

*/
int ms_update_registry(
        const char *mountname, 
        MS_Registry_t *registry);

/*
ms_read_quota
    update quota
parameters
    mountname[in]: 
    dirname[in]:
    dirQuota[out]:
*/
int ms_read_quota(
        const char *mountname, 
        const char *dirname, 
        MS_DirQuota_t *dirQuota);

/*
ms_read_quota
    update quota
parameters
    mountname[in]: 
    dirname[in]:
    dirQuota[in]:
*/
int ms_update_quota(
        const char *mountname, 
        const char *dirname, 
        MS_DirQuota_t *dirQuota);

int ms_get_segnum(
    const char *mountname, 
    int *mediaCountDepot, int *mediaCountRecord, int *mediaCountPrealloc,
    int *navCountDepot,int *navCountRecord);

/*
ms_set_label
    set label partition
parameters
    key[in]: key for encryption
    keySize[in]: size of the keys < 64
    label[in]: pointer label data. data not copied, app should keep it.
    labelSize[in]: size of the label data <1024
*/

int ms_set_label(
    int enable,
    unsigned char *label, 
    unsigned int labelSize);
    
#endif /* _MS_PARTITION_H */
