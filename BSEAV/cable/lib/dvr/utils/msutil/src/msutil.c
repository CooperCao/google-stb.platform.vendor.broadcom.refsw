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
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <fnmatch.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/vfs.h> 
#include <sys/statvfs.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <mntent.h>
#include <sys/mount.h>
#include <libgen.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "nexus_platform.h"
#if DRM_SUPPORT
#include "nexus_security.h"
#endif
#include "nexus_dma.h"
#include "nexus_memory.h"
#include "b_os_lib.h"
#include "msutil.h"
#include "msdiag.h"
#include "b_dvr_const.h"

#if 0
#define ENABLE_PROXY
#endif

#if 0
#define PRINT_DBG(...) {printf("*(%s:%d) ",__FUNCTION__,__LINE__);printf(__VA_ARGS__);}
#else
#define PRINT_DBG(...) {while(0) {printf(__VA_ARGS__);}}
#endif

#if 0
#define PRINT_FUNCIN(...)  printf("+++++++ %s\n",__FUNCTION__)
#define PRINT_FUNCOUT(...) printf("------- %s\n",__FUNCTION__)
#else
#define PRINT_FUNCIN() {while(0) ;}
#define PRINT_FUNCOUT() {while(0) ;}
#endif

#define PRINT_ERR(...) {fprintf(stderr,"#(%s:%d) ",__FUNCTION__,__LINE__);fprintf(stderr,__VA_ARGS__);}

#ifdef LABEL_SUPPORT
typedef struct {
    unsigned int size;
    unsigned char *data;
} MS_Label_t;

static int gLabelEnable = 0;
static MS_Label_t gLabel;
#endif

#ifdef ENABLE_PROXY
B_ThreadHandle gRegProxyThread=NULL;
B_EventHandle gRegProxyEvent=NULL;
unsigned gRegProxyIndex[MS_MAX_DVRVOLUME]={0,0,0,0};
#endif

#ifdef NO_REGFILE
static MS_Registry_t gRegistry[MS_MAX_DVRVOLUME];
#endif
static int check_device(const char *device);

static int create_null_file(const char *fname,off_t size)
{
    int fd;

    fd = open(fname,O_CREAT | O_RDWR | O_CLOEXEC,0666);
    if(fd<0) 
    {
        PRINT_ERR(" %s create failed",fname);
        return eMS_ERR_SYSTEM;
    }

#ifdef FALLOCATE_SUPPORT
    while (posix_fallocate(fd,(off_t)0,(off_t)size) != 0) {
        if (errno==EINTR) {
            PRINT_ERR(" %s pre-allocate returned EINTR: %s\n",fname, strerror(errno));
            continue;
        } else {
            PRINT_ERR(" %s pre-allocate failed: %s\n",fname, strerror(errno));
            close(fd);
            return eMS_ERR_SYSTEM;
        }
    }
#else
    if (ftruncate(fd,(off_t)size) < 0)
    {
        PRINT_ERR(" %s pre-allocate failed",fname);
        close(fd);
        return eMS_ERR_SYSTEM;
    }
#endif
    close(fd);
    return eMS_OK;

}

unsigned long get_total_media_segments (const char *path)
{
    int err;
    struct statvfs vstfs;
    unsigned segSize;
    unsigned long long totalSize;
    unsigned long numTotalSegments;
    struct stat buf_stat;
    int majorNo;

    stat(path, &buf_stat);
    majorNo = major(buf_stat.st_dev);
    err = statvfs(path, &vstfs);    
    segSize = MS_MEDIA_SEGMENT_SIZE;
    totalSize = (unsigned long long)vstfs.f_frsize * (unsigned long long)vstfs.f_blocks;
    
    numTotalSegments = totalSize/segSize;
    /* check for loop device */
    if(majorNo == 7) 
    {
        numTotalSegments = MS_VIRTUAL_RESERVE_SEG(numTotalSegments); 
    }
    else
    {  
        numTotalSegments = MS_RESERVE_SEG(numTotalSegments); 
    }

    PRINT_DBG("media partition size=%lluMB,segment size=%uMB, number of segments=%lu \n",
            totalSize/(1024*1024), segSize/(1024*1024), numTotalSegments);

    return numTotalSegments;
}

/* create null segment files in <path>*/
static int create_media_null_segment (const char *path, unsigned long *segNum)
{
    char fname[MS_MAX_PATHLEN];
    int err;
    unsigned long numTotalSegments;	/* number of segments that can be created */

    unsigned i;

    numTotalSegments = get_total_media_segments(path);

    for (i=0; i<numTotalSegments; i++) {
        snprintf(fname,MS_MAX_PATHLEN,"%s/seg%06u.ts",path,i);
        err = create_null_file(fname,B_DVR_MEDIA_SEGMENT_SIZE);
        if (err) goto error;
    }

    *segNum = numTotalSegments;
    return eMS_OK;

error:
    return eMS_ERR_SYSTEM;		
}


/* create null segment files in <path>*/
static int create_nav_null_segment (const char *path, unsigned long segNum)
{
    char fname[MS_MAX_PATHLEN];
    int err;

    unsigned i;

    for (i=0; i<segNum; i++) {
        snprintf(fname,MS_MAX_PATHLEN,"%s/seg%06u.nav",path,i);
        err = create_null_file(fname,B_DVR_NAV_SEGMENT_SIZE);
        if (err) goto error;
    }

    return eMS_OK;

error:
    return eMS_ERR_SYSTEM;		
}

static int create_media_folder(const char *path)
{
    char folder[MS_MAX_PATHLEN];

    /* create directories depot, record */
    snprintf(folder,MS_MAX_PATHLEN,"%s/depot",path);
    mkdir (folder,0700);
    PRINT_DBG("create folder: %s\n", folder);

    snprintf(folder,MS_MAX_PATHLEN,"%s/record",path);
    mkdir (folder,0700);
    PRINT_DBG("create folder: %s\n", folder);

    snprintf(folder,MS_MAX_PATHLEN,"%s/preallocated",path);
    mkdir (folder,0700);
    PRINT_DBG("create folder: %s\n", folder);

    snprintf(folder,MS_MAX_PATHLEN,"%s/temp",path);
    mkdir (folder,0700);
    PRINT_DBG("create folder: %s\n", folder);

    return eMS_OK;
}

/*
 *  check if the virtual device (a directory ) used
 *  in VSFS is valid
 */
static int check_virtual_device(const char *device)
{
    struct stat buf_stat;
    int ret;

    ret = stat(device, &buf_stat);
    if (ret<0) 
    {
        PRINT_ERR("%s: %s\n", device, strerror(errno));
        return eMS_ERR_INVALID_PARAM;
    }

    if (!S_ISDIR(buf_stat.st_mode)) 
    {
        PRINT_ERR("Not a virtual device\n");
        return eMS_ERR_INVALID_PARAM;
    }

    return eMS_OK;
}

/*
 *  check if a loop device used in the VSFS is valid
 */
static uint64_t check_loop_device(const char *loopDevice)
{
    struct stat buf_stat;
    int ret;
    
    ret = stat(loopDevice, &buf_stat);
    if (ret<0) 
    {
        PRINT_ERR("%s: %s\n", loopDevice, strerror(errno));
        return -1;
    }

    if (!S_ISREG(buf_stat.st_mode)) 
    {
        PRINT_ERR("Not a loop devic\n");
        return -1;
    }
    return buf_stat.st_blocks*buf_stat.st_blksize;
}

/* 
    prepare sfdisk partition temp file

return

*/

/* check if the device is valid */
static int check_device(const char *device)
{
    struct stat buf_stat;
    int ret;
    int i;

    for (i=0;i<10;i++) {
    ret = stat(device, &buf_stat);
        if (ret<0) {
            PRINT_DBG("---%s is missing. retrying in 1 sec..\n", device);
            sleep(1);
        } else {
            PRINT_DBG("---%s is ready.\n", device);
            break;
        }
    }

    if (ret<0) {
        PRINT_ERR("%s: %s\n", device, strerror(errno));
        return eMS_ERR_INVALID_PARAM;
    }

    if (!S_ISBLK(buf_stat.st_mode)) {
        PRINT_ERR("Not a block device\n");
        return eMS_ERR_INVALID_PARAM;
    }

    return eMS_OK;
}

/* 3 partitions in the end are the new partitions. add those! */
static int prepare_sgdisk(int numPartitions, MS_Partition_t *partitionTable, char *buf)
{
    int i;

    if (numPartitions<3)
        return eMS_ERR_INVALID_PARAM;
    
    i=numPartitions-3; 
    
    snprintf(buf,MS_MAX_OPTIONBUF," -n %d:%llu:%llu -c %d:\"%s\" -t %d:%04x -u %d:%s  -n %d:%llu:%llu -c %d:\"%s\" -t %d:%04x -u %d:%s  -n %d:%llu:%llu -c %d:\"%s\" -t %d:%04x -u %d:%s ", 
                partitionTable[i].partitionNum, 
                partitionTable[i].start,
                partitionTable[i].end,
                partitionTable[i].partitionNum, 
                partitionTable[i].name,
                partitionTable[i].partitionNum, 
                partitionTable[i].type,
                partitionTable[i].partitionNum, 
                partitionTable[i].guid,

                partitionTable[i+1].partitionNum, 
                partitionTable[i+1].start,
                partitionTable[i+1].end,
                partitionTable[i+1].partitionNum, 
                partitionTable[i+1].name,
                partitionTable[i+1].partitionNum, 
                partitionTable[i+1].type,
                partitionTable[i+1].partitionNum, 
                partitionTable[i+1].guid,

                partitionTable[i+2].partitionNum, 
                partitionTable[i+2].start,
                partitionTable[i+2].end,
                partitionTable[i+2].partitionNum, 
                partitionTable[i+2].name,
                partitionTable[i+2].partitionNum, 
                partitionTable[i+2].type,
                partitionTable[i+2].partitionNum, 
                partitionTable[i+2].guid
                );

    return eMS_OK;
}

#if 0
/* return 1 if (a,b) includes (x,y) */
static int check_include(uint64_t a, uint64_t b, uint64_t x, uint64_t y)
{
    if (a>b || x>y) return 0;   /* parameter error. return as not include */

    if (x>a && y<=b) return 1;
    return 0;
}
#endif


#ifdef LABEL_SUPPORT
/*
    find label partition
*/
int label_find_partition(int num, MS_Partition_t *partitions, int *labelIndex)
{
    int i;
    int ret;
    int found = 0;

    PRINT_FUNCIN();
    
    for (i=0; i<num; i++) {
        PRINT_DBG("guid=%s:%s\n",partitions[i].guid, MS_GUID_LABEL_PARTITION);
        if (!strcmp (partitions[i].guid, MS_GUID_LABEL_PARTITION)) { 
            *labelIndex = i; 
            found=1; 
            PRINT_DBG("label partition found\n");
            break;
        }
    }
    
    if (found) {
        PRINT_DBG("Label partition found\n");
        ret = eMS_OK;
    } else {
        PRINT_ERR("Label partition not found\n");
        ret = eMS_ERR_INVALID_PARAM;        
    }

    PRINT_FUNCOUT();
    return ret;
}


/* 
    check label data
*/
int label_check(const char *device, int partitionNum)
{
    MS_Label_t label;
    char partition[MS_MAX_PATHLEN];
    int fd;
    int ret = eMS_OK;
    int readSize;
    int i;

    PRINT_FUNCIN();

    label.size = gLabel.size;
    label.data = malloc(label.size);
    if (!label.data) {
        return eMS_ERR_SYSTEM;
    }
    snprintf(partition,MS_MAX_PATHLEN,"%s%d",device,partitionNum);

    fd = open(partition,O_RDONLY|O_CLOEXEC);

    if (fd<0) {
        PRINT_ERR("%s open failed\n",partition);
        free(label.data);
        PRINT_FUNCOUT();
        return eMS_ERR_INVALID_PARAM;
    }

    PRINT_DBG("%s opened\n",partition);

    readSize = read(fd,label.data,label.size);
    if (readSize!=(int)label.size) {
        PRINT_ERR("%s read error\n",partition);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    for (i=0; i<(int)label.size; i++) {
        if(label.data[i]!=gLabel.data[i]) {
            PRINT_ERR("HDD label doesn't match with system.\n");
            ret = eMS_ERR_WRONG_LABEL;
            goto error;
        }        
    }

    PRINT_DBG("label matches!\n");

error:
    close(fd);
    free(label.data);
    PRINT_FUNCOUT();
    return ret;
}



/*
    write label data
*/
int label_write(const char *device, int partitionNum)
{
    char partition[MS_MAX_PATHLEN];
    int ret = eMS_OK;
    int fd;
    int i;
    struct stat buf_stat;
    MS_Label_t label;

    PRINT_FUNCIN();

    snprintf(partition,MS_MAX_PATHLEN,"%s%d",device,partitionNum);

    label.size = gLabel.size;
    label.data = malloc(gLabel.size);
    if (!label.data) {
        return eMS_ERR_SYSTEM;
    }
    memcpy(label.data, gLabel.data,label.size);
    
    for (i=0;i<10;i++) {
        ret = stat(partition, &buf_stat);
        if (ret<0) {
            PRINT_DBG("---%s is missing. retrying in 1 sec..\n", partition);
            sleep(1);
        } else {
            PRINT_DBG("---%s is ready.\n", partition);
            break;
        }
    }

    fd = open(partition,O_WRONLY|O_CLOEXEC);
 
    if (fd<0) {
        PRINT_ERR("%s open failed:%s\n",partition, strerror(errno));
        PRINT_FUNCOUT();
        free(label.data);
        return eMS_ERR_INVALID_PARAM;
    }

    PRINT_DBG("%s opened\n",partition);

    ret = write(fd,label.data,label.size);
    if (ret!=(int)label.size) {
        PRINT_ERR("%s write error. (%d,%d)\n",partition,ret,label.size);
        ret = eMS_ERR_SYSTEM;
    }

    PRINT_DBG("label write sucess %s:%d\n",partition,label.size);

    close(fd);
    free(label.data);
    PRINT_FUNCOUT();
    return ret;
}

/*
    create label partition in given region with pSettings. pSettings.startSec and size will be changed accordingly.
    sgdisk -n -c -t -u device
*/
int label_create_partition(const char *device, uint64_t *pStartSec, uint64_t *pEndSec, MS_Partition_t *partitionTable, int *num)
{
    uint64_t labelStart,labelEnd;
    char cmdBuf[MS_MAX_CMDLEN];
    int ret = eMS_OK;
    int labelPartNum;

    PRINT_FUNCIN();

    PRINT_DBG("num= %d\n", *num);

    labelPartNum = *num+1;

    if (*pStartSec%MS_SECTOR_BOUNDARY) {
        labelStart = MS_SECTOR_BOUNDARY * ((*pStartSec/MS_SECTOR_BOUNDARY)+1);    /* align sector boundary */
    } else {
        labelStart = *pStartSec;
    }

    labelEnd = labelStart+1;    /*use 2 sectors */
    *pStartSec = labelStart+MS_SECTOR_BOUNDARY; /* next boundary */
    *pEndSec -= 2*MS_SECTOR_BOUNDARY;

    /* 
    sgdisk -n *num:startSec:endSec -c *num:"SYSEND" -t *num:0x8301 -u *num:MS_GUID_LABEL_PARTITION /dev/sda
    sgdisk -n 4:8192:8193 -c 4:"DVR" -t 4:8301 -u 4:"44565220-5041-5254-4954-494f4e000000" /dev/sda
    */

    snprintf(cmdBuf,MS_MAX_CMDLEN,"sgdisk  -n %d:%llu:%llu -c %d:\"%s\" -t %d:%04x -u %d:%s %s",
        labelPartNum,labelStart,labelEnd,
        labelPartNum,"SYSEND",
        labelPartNum,0x8301,
        labelPartNum,MS_GUID_LABEL_PARTITION,
        device);

    PRINT_DBG(cmdBuf);PRINT_DBG("\n");
    
    if (system(cmdBuf)) {
        PRINT_ERR("label partition create failed:%s\n",strerror(errno));
        ret = eMS_ERR_SYSTEM;
    }

    PRINT_DBG("start sec = %llu, endsec = %llu\n", (unsigned long long)(*pStartSec),(unsigned long long)(*pEndSec));

    partitionTable[*num].partitionNum = labelPartNum;
    partitionTable[*num].start = labelStart;
    partitionTable[*num].end = labelEnd;
    snprintf(partitionTable[*num].device,MS_DEVICE_NAME_LEN,"%s",device);
    snprintf(partitionTable[*num].guid,MS_GUID_LENGTH,MS_GUID_LABEL_PARTITION);

    *num=labelPartNum;

    PRINT_FUNCOUT();
    return ret;
}
#endif

/* return 1 if (x,y) ovelaps (a,b) */
static int check_overlap(uint64_t a, uint64_t b, uint64_t x, uint64_t y)
{
    if (a>b || x>y) return 1;   /* parameter error. return as overlap */

    if (a<=y && b>=x) return 1;
    return 0;
}

/* check for media partitions. 
    return
    eMS_OK yes, it is a valid volume having all 3 media partitions.
    eMS_ERR_INVALID_VOLUME no, this is not a valid volume
    */
static int find_partition(int num, MS_Partition_t*partitions, int *mediaIndex, int *navIndex, int *metaIndex)
{
    int i;
    int ret = eMS_ERR_INVALID_VOLUME;
    int check_counter = 3;
    
    *mediaIndex = MS_INVALID_INDEX;
    *navIndex = MS_INVALID_INDEX;
    *metaIndex = MS_INVALID_INDEX;

    for (i=0; i<num; i++) {
        if (!strcmp (partitions[i].guid, MS_GUID_MEDIA_PARTITION)) { *mediaIndex = i; check_counter--; }
        else if (!strcmp (partitions[i].guid, MS_GUID_NAV_PARTITION)) { *navIndex = i; check_counter--; }
        else if (!strcmp (partitions[i].guid, MS_GUID_META_PARTITION)) { *metaIndex = i; check_counter--; }
    }
    
    if (check_counter==0)
        ret = eMS_OK;

    return ret;
}

/* 
    check if mounted. if mounted, return mount point dir.
    return -1 if mounted (VSFS)
*/
static int check_mounted_loop_device(const char *loopDevice, char *path) 
{
    FILE * file;
    struct mntent * mnt;
    
    if ((file = setmntent (MOUNTED, "r")) == NULL) {
        return 0;
    }

    while ((mnt = getmntent (file)) != NULL) {
//        PRINT_DBG("mnt->mnt_fsname = %s mnt_dir = %s\n", mnt->mnt_fsname, mnt->mnt_dir);
        if (strcmp (loopDevice, mnt->mnt_fsname) == 0) {
            strncpy(path,mnt->mnt_dir,MS_MAX_PATHLEN);
            break;
        }
    }

    endmntent (file);

    if (!mnt)
        return 0;

    return -1;
}

/* 
    check if mounted. if mounted, return mount point dir.
    return -1 if mounted (SFS)
*/
static int check_mounted_partition(const MS_Partition_t *gpt, char *path) {
    FILE * file;
    struct mntent * mnt;
    char partition[MS_MAX_PATHLEN];

    if ((file = setmntent (MOUNTED, "r")) == NULL) {
        return 0;
    }

    snprintf(partition,MS_MAX_PATHLEN,"%s%d",gpt->device,gpt->partitionNum);
    while ((mnt = getmntent (file)) != NULL) {
//        PRINT_DBG("mnt->mnt_fsname = %s mnt_dir = %s\n", mnt->mnt_fsname, mnt->mnt_dir);
        if (strcmp (partition, mnt->mnt_fsname) == 0) {
            strncpy(path,mnt->mnt_dir,MS_MAX_PATHLEN);
            break;
        }
    }

    endmntent (file);

    if (!mnt)
        return 0;

    return -1;
}

static int get_guid(char *guid)
{
    FILE *fp;
    int fd;
    char fileLine[256];

    fp=fopen(MS_SGDISK_GUID_TEMP_FILE, "r");
    if (!fp) {
        PRINT_ERR("open %s failed:%s",MS_SGDISK_GUID_TEMP_FILE,strerror(errno));
        return -1;
    }

    fd = fileno(fp);
    if (fd == -1) {
        PRINT_ERR("fileno %s failed:%s",MS_SGDISK_GUID_TEMP_FILE,strerror(errno));
        goto error;
    }
    
    if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1) {
        PRINT_ERR("fcntl %s failed:%s",MS_SGDISK_GUID_TEMP_FILE,strerror(errno));
        goto error;
    }
    
    if (!fgets(fileLine, MS_SGDISK_LINE_SIZE, fp)) {
        goto error;
    }    
    sscanf (fileLine, "%*s %*s %*s %s",guid);

//    PRINT_DBG("GUID:%s\n", guid);

    fclose(fp);
    return 0;

error:    
    fclose(fp);
    return -1;

}

/*
get_mount_path
    find a mount path index which is not mounted yet.    
    all three partitions should not be mounted.
    mount path = /mnt/dvr<index>-media,/mnt/dvr<index>-nav,/mnt/dvr<index>-metadata

parameters
    void

return 
    index
    -1 if fail.

 
*/
static int get_mount_index(void)
{
    int i;
    int ret;
    char name[MS_MAX_PATHLEN];
    char cmd[MS_MAX_CMDLEN];


    for (i=0; i<MS_MAX_DVRVOLUME; i++) {
        memset(name,0,MS_MAX_PATHLEN);
        memset(cmd,0,MS_MAX_CMDLEN);
        snprintf(name,MS_MAX_PATHLEN,"/mnt/dvr%d-media",i);
        snprintf(cmd,MS_MAX_CMDLEN,"cat /proc/mounts | grep %s",name);
        ret = system(cmd);
        if (ret==0) continue;   // mounted already
        
        memset(name,0,MS_MAX_PATHLEN);
        memset(cmd,0,MS_MAX_CMDLEN);
        snprintf(name,MS_MAX_PATHLEN,"/mnt/dvr%d-nav",i);
        snprintf(cmd,MS_MAX_CMDLEN,"cat /proc/mounts | grep %s",name);
        ret = system(cmd);
        if (ret==0) continue;   // mounted already

        memset(name,0,MS_MAX_PATHLEN);
        memset(cmd,0,MS_MAX_CMDLEN);
        snprintf(name,MS_MAX_PATHLEN,"/mnt/dvr%d-metadata",i);
        snprintf(cmd,MS_MAX_CMDLEN,"cat /proc/mounts | grep %s",name);
        ret = system(cmd);
        if (ret==0) continue;   // mounted already

        /* found !*/
        return i;        
    }

    /* not found */
    return -1;
}

/*
read_partition
    do sgdisk -p <device> and parse the output to save.

parameters
    device[in] device name
    max_partition[in] max partition
    partitionTable[in/out] pointer to partition array
    numPartitions[out] number of partitions found

return
    eMS_OK
    eMS_ERR_INVALID_PARAM  
    eMS_ERR_SYSTEM 
*/

static int read_partition(const char *device, int maxPartition, MS_Partition_t *partitionTable, int *numPartition)
{
    FILE *fp = NULL;
    int fd;
    char *pCmd = NULL;
    size_t cmdSize = 0;
    char fileLine[256];
    /*int n = 0;  Number of items read from the file line.*/
    int ret;
    int i;

    int partitionNum;
    uint64_t start; /* start sector */
    uint64_t end;   /* end sector */
    uint16_t type;  /* partition type */

    /* check if the device is valid */
    ret = check_device(device);
    if (ret<0) return ret;

    memset(partitionTable, 0, sizeof(MS_Partition_t)*maxPartition);

    /* bulid cmd */
    cmdSize = sizeof (MS_GPT_LIST_CMD) + strlen (device) + sizeof (MS_SGDISK_TEMP_FILE);
    pCmd = malloc (cmdSize);
    if (!pCmd ) {
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    /* read partition list */
    ret = snprintf (pCmd, cmdSize, MS_GPT_LIST_CMD, device, MS_SGDISK_TEMP_FILE);
    if (ret<0) {
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    if (system(pCmd)<0) {
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    fp=fopen(MS_SGDISK_TEMP_FILE, "r");
    if (!fp) {
        PRINT_ERR("open %s failed:%s",MS_SGDISK_TEMP_FILE,strerror(errno));
        ret = eMS_ERR_SYSTEM;
        goto error;
    }
    
    fd = fileno(fp);
    if (fd == -1) {
        PRINT_ERR("fileno %s failed:%s",MS_SGDISK_TEMP_FILE,strerror(errno));
        ret = eMS_ERR_SYSTEM;
        goto error_open;
    }
    
    if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1) {
        PRINT_ERR("fcntl %s failed:%s",MS_SGDISK_TEMP_FILE,strerror(errno));
        ret = eMS_ERR_SYSTEM;
        goto error_open;
    }
    
    memset(fileLine,0,sizeof(256));    
    for (i=0;!feof(fp) && i<maxPartition+1;i++) {
        const char delimiters[] = " ";
        char *token,*cp;
        if (!fgets(fileLine, MS_SGDISK_LINE_SIZE, fp)) break;
        cp = strdupa (fileLine); /* Make writable copy.  */
        token = strtok (cp, delimiters);
        partitionNum = atoi(token);

        token = strtok (NULL, delimiters);
        start = strtoull(token,NULL,0);

        token = strtok (NULL, delimiters);
        end = strtoull(token,NULL,0);

        token = strtok (NULL, delimiters);
        token = strtok (NULL, delimiters);
        token = strtok (NULL, delimiters);
        type = strtoul(token,NULL,0);

        /*n = sscanf (fileLine, "%d %llu %llu %*s %*s %x \n",
                    &partitionNum, &start, &end, (unsigned int*)&type);*/
        
        partitionTable[i].partitionNum = partitionNum;
        partitionTable[i].start = start;
        partitionTable[i].end = end;
        partitionTable[i].type= type;
        snprintf(partitionTable[i].device,MS_DEVICE_NAME_LEN, "%s",device);

        ret = snprintf (pCmd, cmdSize, MS_GPT_GUID_GET_CMD, partitionNum, device, MS_SGDISK_GUID_TEMP_FILE);
        if (system(pCmd)<0) {
            ret = eMS_ERR_SYSTEM;
            goto error_open;
        }

        if (get_guid(partitionTable[i].guid)) {
            ret = eMS_ERR_SYSTEM;
            goto error_open;
        }

    }

    if(i==maxPartition+1) { 
        PRINT_ERR("array too small\n");
        ret = eMS_ERR_INVALID_PARAM;
        goto error_open;
    }

    *numPartition = i;
    ret = eMS_OK;

error_open:
    fclose(fp);
error:
    if (pCmd) free(pCmd);
    return ret;
}

static int read_size(
        const char *device,
        unsigned long *size)
{
    char devname[MS_DEVICE_NAME_LEN];
    char path[MS_MAX_PATHLEN];
    FILE *fp;
    int  fd;
    int ret;
    char *pathc;

    pathc = strdup(device);
    snprintf(devname,MS_DEVICE_NAME_LEN,"%s",basename(pathc));
    free(pathc);
    snprintf(path,MS_MAX_PATHLEN,"/sys/block/%s/size",devname);
    fp = fopen(path,"r");
    if (fp==NULL) {
        PRINT_ERR("invalid device %s:%s\n",device,strerror(errno));
        return eMS_ERR_INVALID_PARAM;
    }

    fd = fileno(fp);
    if (fd == -1) {
        PRINT_ERR("fileno %s failed:%s",path,strerror(errno));
        goto error;
    }
    
    if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1) {
        PRINT_ERR("fcntl %s failed:%s",path,strerror(errno));
        goto error;
    }
    
    ret = fscanf(fp, "%lu",size);
    if(ret==EOF) {
        fclose (fp);
        PRINT_ERR("read error %s:%s\n",path,strerror(errno));
        goto error;
    }
    fclose(fp);
/*    *size -= 2048; */
    return eMS_OK;

error:
    fclose(fp);
    return eMS_ERR_SYSTEM;
}

/*
update_partition
    update partition to the given partition table
    volume state will be "ready" after update
parameters
    device[in]:device name
    numPartitions[in]: number of partitions
    partitionTable[in]: pointer to partition array
return
    eMS_OK
    eMS_ERR_SYSTEM
    eMS_ERR_INVALID_PARAM
*/

static int update_partition(const char *device, int numPartitions, MS_Partition_t *partitionTable)
{
    char *cmd;      
    char *optionBuf;
    int ret;

    /* check if the device is valid */
    ret = check_device(device);
    if (ret<0) return ret;

    cmd = malloc(MS_MAX_OPTIONBUF);
    optionBuf = malloc(MS_MAX_OPTIONBUF);
    ret = prepare_sgdisk(numPartitions, partitionTable, optionBuf);
    if (ret!=eMS_OK) {
        free(optionBuf);
        free(cmd);
        return ret;
    }

    snprintf(cmd,MS_MAX_OPTIONBUF,"sgdisk %s %s",optionBuf,device);
    PRINT_DBG("%s\n",cmd);
    free(optionBuf);

    ret = system(cmd);

    if (ret!=0) {
        PRINT_ERR("sgdisk returned %d\n",ret);
        free(cmd);
        return eMS_ERR_SYSTEM;
    }
    free (cmd);
    return eMS_OK;
}

/*
create_partition_table
    add media storage partitions to the given table
    
parameters
    device[in]:device name
    pSettings[in]:create parameter (start sector, size) default if null
    maxPartitions[in]: number of partitions
    partitionTable[in/out]: pointer to partition array
    numPartitions[out]: number of resulting partitions
return
    eMS_OK
    eMS_ERR_INVALID_PARAM  
    eMS_ERR_SYSTEM 

*/

static int create_table(
        const char *device, 
        MS_createPartitionSettings_t *pSettings,
        int maxPartitions,
        MS_Partition_t *partitionTable,
        int *pNumPartitions)
{
    int	ret;
    int lastPartition = 0; /* last partition number */
    uint64_t startSec; 
    uint64_t endSec; 
    uint64_t mediaStartSec = 0;
    uint64_t mediaEndSec = 0;
    uint64_t navStartSec = 0;
    uint64_t navEndSec = 0;
    uint64_t metaStartSec = 0;
    uint64_t metaEndSec = 0;
    uint64_t totalSectors = 0;
    int i;
#ifdef LABEL_SUPPORT
    int labelIndex;
#endif

    /* check if the device is valid */
    ret = check_device(device);
    if (ret<0) return ret;

    if( maxPartitions < *pNumPartitions+3 ) {
        PRINT_ERR("Not enough array size\n");
    }

    ret = read_size(device,(unsigned long*)&totalSectors);
    if (ret!=eMS_OK) {
        return ret;
    }

    if (pSettings == NULL) { /* set defaults */
        return eMS_ERR_INVALID_PARAM;
    } else {
        if (pSettings->startSec) { 
            startSec = pSettings->startSec;
        }
        else {    /* set default start sector */
            if (*pNumPartitions) {
                startSec = partitionTable[*pNumPartitions-1].end+1;
            } else {
                startSec = 4096;    /* no existing partitions */
            }
        }

        if (pSettings->size) {
            if (pSettings->size < MS_MIN_VOL_SIZE) {
                PRINT_ERR("size should be > %d\n", MS_MIN_VOL_SIZE);
                return eMS_ERR_INVALID_PARAM;
            }
            endSec = startSec+MS_MBYTE_TO_SECTORS(pSettings->size);
        }
        else { /* set default size */
            endSec = totalSectors-2048;  /* default max */
        }
            
        PRINT_DBG("start sector: %llu, end sector: %llu with given size %llu Mbytes\n", 
                startSec, endSec, pSettings->size);    
    }

   
    /* now check if the given range is available in the disk */
    if (totalSectors<endSec) {
        PRINT_ERR("invalid range: totalSector=%llu, endSector=%llu\n",totalSectors,endSec);
        return eMS_ERR_INVALID_RANGE;  /* invalide range */
    }

#ifdef LABEL_SUPPORT
    if (gLabelEnable) {
        /* 
        - find label partition : label_find_partition()
        - if exists, write label to the partition : label_write()
        - if not exist, create one and write label : label_create_partition(), label_write()
        */

        ret = label_find_partition(*pNumPartitions,partitionTable,&labelIndex);

        if(ret!=eMS_OK) {
            ret = label_create_partition(device, &startSec, &endSec, partitionTable, pNumPartitions);
            if (ret<0) {
                PRINT_DBG("failed to create label partition\n");
                PRINT_FUNCOUT();
                return eMS_ERR_SYSTEM;
            }
        }
        /* label will be written during format */
    }
#endif
    /*  
        check given partitions and get last partition number.
        count primary and logical partitins
        check if the range overlaps with other existing partitions
    */
    for (i=0; i<*pNumPartitions; i++) {
        /* range check */
        if (check_overlap( partitionTable[i].start,partitionTable[i].end, startSec,endSec)) {
            PRINT_ERR("invalid range\n");
            PRINT_ERR("range overlaps with a existing partition.%s%d:%llu:%llu\n",
                partitionTable[i].device, partitionTable[i].partitionNum, partitionTable[i].start, partitionTable[i].end);
            return eMS_ERR_INVALID_RANGE;  /* invalide range */
        } 
        if (lastPartition<partitionTable[i].partitionNum)
            lastPartition = partitionTable[i].partitionNum;        
    } /* end for */
    
    /* now it's ready to add partitions needed! */
    mediaStartSec = startSec;
    mediaEndSec = endSec - MS_NAV_PT_SECTORS(startSec,endSec) - MS_MBYTE_TO_SECTORS(MS_META_PT_SIZE);
    navStartSec = mediaEndSec+1;
    navEndSec = navStartSec + MS_NAV_PT_SECTORS(startSec,endSec);
    metaStartSec = navEndSec + 1;
    metaEndSec = endSec;

    i = *pNumPartitions; 
    /* add media partition at the end */
    memset(&partitionTable[i],0,sizeof(MS_Partition_t));
    strncpy(partitionTable[i].device,device,MS_DEVICE_NAME_LEN);
    partitionTable[i].type = 0x8301;
    partitionTable[i].start   = mediaStartSec;
    partitionTable[i].end	  = mediaEndSec;
    partitionTable[i].partitionNum = ++lastPartition;
    strncpy(partitionTable[i].guid,MS_GUID_MEDIA_PARTITION,MS_GUID_LENGTH);
    strncpy(partitionTable[i].name,"media partition",MS_MAX_PATHLEN);
    snprintf(pSettings->mediaPartition,MS_DEVICE_NAME_LEN,"%s%d",partitionTable[i].device,partitionTable[i].partitionNum);

    i++;
    /* add nav partition at the end */
    memset(&partitionTable[i],0,sizeof(MS_Partition_t));
    strncpy(partitionTable[i].device,device,MS_DEVICE_NAME_LEN);
    partitionTable[i].type = 0x8301;
    partitionTable[i].start   = navStartSec;
    partitionTable[i].end	  = navEndSec;
    partitionTable[i].partitionNum = ++lastPartition;
    strncpy(partitionTable[i].guid,MS_GUID_NAV_PARTITION,MS_GUID_LENGTH);
    strncpy(partitionTable[i].name,"nav partition",MS_MAX_PATHLEN);
    snprintf(pSettings->navPartition,MS_DEVICE_NAME_LEN,"%s%d",partitionTable[i].device,partitionTable[i].partitionNum);

    i++;
    /* add metadata partition at the end */
    memset(&partitionTable[i],0,sizeof(MS_Partition_t));
    strncpy(partitionTable[i].device,device,MS_DEVICE_NAME_LEN);
    partitionTable[i].type = 0x8301;
    partitionTable[i].start   = metaStartSec;
    partitionTable[i].end	  = metaEndSec;
    partitionTable[i].partitionNum = ++lastPartition;
    strncpy(partitionTable[i].guid,MS_GUID_META_PARTITION,MS_GUID_LENGTH);
    strncpy(partitionTable[i].name,"meta partition",MS_MAX_PATHLEN);
    snprintf(pSettings->metadataPartition,MS_DEVICE_NAME_LEN,"%s%d",partitionTable[i].device,partitionTable[i].partitionNum);
    
    *pNumPartitions = i+1;

    return eMS_OK;
}

/* count segments and return it. -1 if wrong type given */
int count_segments(const char *path, MS_SegmentType_t type)
{
    DIR *pdir;    
    struct dirent *pent;
    int counter=0;

    pdir = opendir(path);
    if (!pdir) {
        PRINT_ERR("%s opendir error:%s", path, strerror(errno));
        return -1;
    }
    if (type==eMS_MediaSegment) {
        while ((pent = readdir(pdir))) {
            if (!fnmatch("seg*ts", pent->d_name, 0)) {	/* found */
                counter++;
            } else {
/*                PRINT_DBG("skipping %s\n", pent->d_name); */
            }                
        }
    } else if (type==eMS_NavSegment) {
        while ((pent = readdir(pdir))) {
            if (!fnmatch("seg*nav", pent->d_name, 0)) {	/* found */
                counter++;
            } else {
/*                PRINT_DBG("skipping %s\n", pent->d_name); */
            }                
        }
    } else {
        PRINT_DBG("wrong type\n");
        return -1;
    }
    closedir(pdir);

    return counter;
}

static void print_partition(int num, MS_Partition_t *partitionTable)
{
    int i;

    PRINT_DBG("partition\t%10s %10s type         partition guid\n", "start","end");
    for(i=0; i<num; i++) {
        PRINT_DBG("%s%d\t%10llu %10llu %04x %s\n",
            partitionTable[i].device, partitionTable[i].partitionNum, 
            partitionTable[i].start, partitionTable[i].end, partitionTable[i].type, 
            partitionTable[i].guid);
    }

}


/*
    check a volume if it has valid partitions
    return 
    eMS_OK if it has.
*/
static int check_volume_partitions(const char *device)
{
    int ret = eMS_OK;
    MS_Partition_t *partition;
    int numPartitions;
    int mediaIndex, navIndex,metaIndex;
#ifdef LABEL_SUPPORT
        int labelIndex;
#endif

    partition = malloc(sizeof(MS_Partition_t)*MS_MAX_PARTITION_ARRAY);
    if (!partition) return eMS_ERR_SYSTEM;

    ret = read_partition(device,MS_MAX_PARTITION_ARRAY,partition,&numPartitions);
    if (ret!=eMS_OK) goto error;

    ret = find_partition(numPartitions, partition,&mediaIndex,&navIndex,&metaIndex);
    if (ret!=eMS_OK) goto error;
    
#ifdef LABEL_SUPPORT
    if (gLabelEnable) {
        ret = label_find_partition(numPartitions,partition,&labelIndex);
        if (ret!=eMS_OK) goto error;

        ret = label_check(device,partition[labelIndex].partitionNum);
        if (ret!=eMS_OK) goto error;
    }
#endif    

error:
    free(partition);
    return ret;
}

/*
    mounting a volume (VSFS) to tmp directory for ms_check_virtual_volume and ms_format_virtual volume
*/
int check_mount_virtual_volume(const char *mediaLoopDevice, 
                               const char *navLoopDevice, 
                               const char *metaLoopDevice, 
                               char *mountname)
{
    char targetMedia[MS_MAX_PATHLEN];
    char targetNav[MS_MAX_PATHLEN];
    char targetMeta[MS_MAX_PATHLEN];
    int ret = eMS_OK;
    char cmd[MS_MAX_CMDLEN];
    
    memset(targetMedia,0,MS_MAX_PATHLEN);
    memset(targetNav,0,MS_MAX_PATHLEN);
    memset(targetMeta,0,MS_MAX_PATHLEN);
   
    snprintf(mountname,MS_MAX_PATHLEN,"/tmp/dvr0");
    snprintf(targetMedia,MS_MAX_PATHLEN,"%s-media",mountname);
    snprintf(targetNav,MS_MAX_PATHLEN,"%s-nav",mountname);
    snprintf(targetMeta,MS_MAX_PATHLEN,"%s-metadata",mountname);
    
    mkdir(targetMedia,0700);
    mkdir(targetNav,0700);
    mkdir(targetMeta,0700);

    /* mount media partition */

    PRINT_DBG("mount media partition: %s:%s\n",  mediaLoopDevice, targetMedia);

    snprintf(cmd,MS_MAX_CMDLEN,"mount -o loop -t ext4 %s %s",mediaLoopDevice,targetMedia);
    PRINT_DBG(cmd);PRINT_DBG("\n");
    ret = system(cmd);
    if (ret<0) 
    {
        PRINT_ERR("%s:%s\n",cmd,strerror(errno));
        ret = eMS_ERR_SYSTEM;
        goto error;
    }
    
    
    PRINT_DBG("mount nav partition  : %s:%s\n",  navLoopDevice, targetNav);


    snprintf(cmd,MS_MAX_CMDLEN,"mount -o loop -t ext4 %s %s",navLoopDevice,targetNav);
    PRINT_DBG(cmd);PRINT_DBG("\n");
    ret = system(cmd);
    if (ret<0) 
    {
        PRINT_ERR("%s:%s\n",cmd,strerror(errno));
        ret = eMS_ERR_SYSTEM;
        umount(targetMedia);
        goto error;
    }

    
    PRINT_DBG("mount meta partition : %s:%s\n",  metaLoopDevice, targetMeta);

    snprintf(cmd,MS_MAX_CMDLEN,"mount -o loop -t ext4 %s %s",metaLoopDevice,targetMeta);
    PRINT_DBG(cmd);PRINT_DBG("\n");
    ret = system(cmd);
    if (ret<0) 
    {
        PRINT_ERR("%s:%s\n",cmd,strerror(errno));
        umount(targetNav);
        umount(targetMedia);
        ret = eMS_ERR_SYSTEM;
    }

error:
    return ret;
}

/*
    mounting a volume (SFS) to tmp directory for ms_check_volume for ms_format_volume
*/
int check_mount_volume(const char *device, char *mountname)
{
    char src[MS_MAX_PATHLEN];
    char targetMedia[MS_MAX_PATHLEN];
    char targetNav[MS_MAX_PATHLEN];
    char targetMeta[MS_MAX_PATHLEN];
    int ret = eMS_OK;
    MS_Partition_t *partition;
    int numPartitions;
    int mediaIndex, navIndex,metaIndex;
        
    partition = malloc(sizeof(MS_Partition_t)*MS_MAX_PARTITION_ARRAY);
    if (!partition) return eMS_ERR_SYSTEM;
        
    ret = read_partition(device,MS_MAX_PARTITION_ARRAY,partition,&numPartitions);
    if (ret!=eMS_OK)goto error;

    ret = find_partition(numPartitions, partition,&mediaIndex,&navIndex,&metaIndex);
    if (ret!=eMS_OK)goto error;
    
    memset(src,0,MS_MAX_PATHLEN); 
    memset(targetMedia,0,MS_MAX_PATHLEN);
    memset(targetNav,0,MS_MAX_PATHLEN);
    memset(targetMeta,0,MS_MAX_PATHLEN);
   
    snprintf(mountname,MS_MAX_PATHLEN,"/tmp/dvr%c",device[strlen(device)-1]);
    snprintf(targetMedia,MS_MAX_PATHLEN,"%s-media",mountname);
    snprintf(targetNav,MS_MAX_PATHLEN,"%s-nav",mountname);
    snprintf(targetMeta,MS_MAX_PATHLEN,"%s-metadata",mountname);
    
    mkdir(targetMedia,0700);
    mkdir(targetNav,0700);
    mkdir(targetMeta,0700);

    /* mount media partition */

    snprintf(src,MS_MAX_PATHLEN,"%s%d",partition[mediaIndex].device,partition[mediaIndex].partitionNum);

    PRINT_DBG("mount media partition: %s:%s\n",  src, targetMedia);
    ret = mount(src,targetMedia,MS_MEDIA_FILESYSTEMTYPE,MS_NOATIME|MS_NODIRATIME,0);

    if (ret!=0) {
        PRINT_ERR("mount failed:%s\n",strerror(errno));
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    /* mount nav partition */
    snprintf(src,MS_MAX_PATHLEN,"%s%d",partition[navIndex].device,partition[navIndex].partitionNum);
    
    PRINT_DBG("mount nav partition  : %s:%s\n",  src, targetNav);
    ret = mount(src,targetNav,MS_NAV_FILESYSTEMTYPE,MS_NOATIME|MS_NODIRATIME,0);

    if (ret!=0) {
        PRINT_ERR("mount failed:%s\n",strerror(errno));
        umount(targetMedia);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }
    
    /* mount metadata partition */
    snprintf(src,MS_MAX_PATHLEN,"%s%d",partition[metaIndex].device,partition[metaIndex].partitionNum);
    
    PRINT_DBG("mount meta partition : %s:%s\n",  src, targetMeta);
    ret = mount(src,targetMeta,MS_META_FILESYSTEMTYPE,MS_NOATIME|MS_NODIRATIME,0);

    if (ret!=0) {
        PRINT_ERR("mount failed:%s\n",strerror(errno));
        umount(targetNav);
        umount(targetMedia);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

error:
    free(partition);
    return ret;
}

/* 
create_mount_path
    create mount points at start.
    this should be called at system start 
*/
void create_mount_path(void)
{
    int i;
    char name[MS_MAX_PATHLEN];

    PRINT_FUNCIN();

    for (i=0; i<MS_MAX_DVRVOLUME; i++) {
        memset(name,0,20);
        snprintf(name,MS_MAX_PATHLEN,"/mnt/dvr%d-media",i);
        mkdir (name,0700);

        memset(name,0,20);
        snprintf(name,MS_MAX_PATHLEN,"/mnt/dvr%d-nav",i);
        mkdir (name,0700);

        memset(name,0,20);
        snprintf(name,MS_MAX_PATHLEN,"/mnt/dvr%d-metadata",i);
        mkdir (name,0700);
        }

    PRINT_FUNCOUT();
}

#ifdef ENABLE_PROXY
static void ms_registry_proxy(void *context)
{
    B_Error berr;
    char reg[MS_MAX_PATHLEN];
    int fd;
    int index;

    BSTD_UNUSED(context);

    while(1) {
        berr = B_Event_Wait(gRegProxyEvent,B_WAIT_FOREVER);
        if (berr!=B_ERROR_SUCCESS) continue;

        for (index=0; index<MS_MAX_DVRVOLUME; index++) {    /* check flags */
            if (gRegProxyIndex[index]==1) {     /* ms_registry_update called */
                gRegProxyIndex[index]=0;        /* reset flag */
                snprintf(reg,MS_MAX_PATHLEN,"/mnt/dvr%d-metadata/%s",index,MS_REGISTRY_FILENAME);

                fd= open(reg,O_WRONLY|O_CLOEXEC);
                if (fd<0) {
                    PRINT_ERR("%s open error:%s\n",reg, strerror(errno));
                    continue;
                }
                fdatasync(fd);
                close(fd);
            }
        }
        BKNI_Sleep(1000);
    }
}
#endif


/*
    create mount points
    create proxy thread
*/
int ms_init(void)
{
    int ret=eMS_OK;

    create_mount_path();

#ifdef NO_REGFILE
    memset(&gRegistry,0,sizeof(MS_Registry_t)*MS_MAX_DVRVOLUME);
#else

#ifdef ENABLE_PROXY
    if (!gRegProxyEvent) {
         gRegProxyEvent = B_Event_Create(NULL);

        if (!gRegProxyEvent) {
            PRINT_ERR("%s message queue create error", __FUNCTION__);
            ret = eMS_ERR_SYSTEM;
            goto error;
        }
    }
    
    if (!gRegProxyThread){
        gRegProxyThread = B_Thread_Create("gRegProxyThread", ms_registry_proxy,NULL, NULL);
        if (!gRegProxyThread) {
           PRINT_ERR("%s scheduler thread  create error\n", __FUNCTION__);
           B_Event_Destroy(gRegProxyEvent);
           gRegProxyEvent = NULL;
           ret = eMS_ERR_SYSTEM;
           goto error;
        }
    }
error:
#endif  /* ENABLE_PROXY */

#endif  /* NO_REGFILE */

    return ret;
}

int ms_check_virtual_volume(const char *device, 
                            MS_StorageStatus_t *status)
{
    int ret = eMS_OK;
    int err;
    char mountname[MS_MAX_PATHLEN];
    char pathname[MS_MAX_PATHLEN];
    struct stat buf_stat;
    MS_Registry_t *registry;
    int depotCount, recordCount, preallocCount, totalCount;
    uint32_t mediaTotalSegment;
    int64_t loopDeviceSize=0;

    PRINT_FUNCIN();

    status->state = eMS_StateInvalid; // let's assume this is invalid

    /* check if the device is valid */
    ret = check_virtual_device(device);
    if (ret<0) 
    {
        PRINT_ERR("(%s:%d)invalid virtual device %s\n",__FUNCTION__,__LINE__,device);
        goto error;
    }
    snprintf(status->mediaPartition,MS_DEVICE_NAME_LEN,"%s/%s", device,MS_LOOP_DEVICE_MEDIA);
    snprintf(status->navPartition,MS_DEVICE_NAME_LEN,"%s/%s", device,MS_LOOP_DEVICE_NAVIGATION);
    snprintf(status->metadatapartition,MS_DEVICE_NAME_LEN,"%s/%s", device,MS_LOOP_DEVICE_METADATA);
    
    loopDeviceSize = check_loop_device(status->metadatapartition);
    if (loopDeviceSize<=0) 
    {
        PRINT_ERR("(%s:%d)invalid metadata loop device %s\n",__FUNCTION__,__LINE__,status->metadatapartition);
        goto error;
    }

    loopDeviceSize = check_loop_device(status->navPartition);
    if (loopDeviceSize<=0) 
    {
        PRINT_ERR("(%s:%d)invalid nav loop device %s\n",__FUNCTION__,__LINE__,status->navPartition);
        goto error;
    }
    /* check if the device is valid */
    loopDeviceSize = check_loop_device(status->mediaPartition);
    if (loopDeviceSize<=0) 
    {
        PRINT_ERR("(%s:%d)invalid media loop device %s\n",__FUNCTION__,__LINE__,status->mediaPartition);
        goto error;
    }
    status->startSector = 0;                  /* virtual start sector */
    status->endSector = loopDeviceSize/MS_SECTOR_BOUNDARY; /* virtual end sector */
    status->state  = eMS_StateEmpty;   // ok, found media partitions. this is empty media volume at least.
    PRINT_DBG("%s:%d: Media volume %s has valid partitions. now checking folders and files.\n",__FUNCTION__,__LINE__, device);

    // this can falsely fail if max volumes are already mounted. 
    err = check_mount_virtual_volume(status->mediaPartition,
                                     status->navPartition,
                                     status->metadatapartition,
                                     mountname);
    if (err<0) 
    {
        PRINT_ERR("check mount volume error %s\n",device);
        PRINT_FUNCOUT();
        status->mediaSegmentSize = 0;
        status->mediaSegTotal = 0;
        status->mediaSegInuse = 0;
        goto error;
    }

    // now check the folders
    snprintf(pathname,MS_MAX_PATHLEN,"%s-media/depot",mountname); 
    err = stat(pathname, &buf_stat);
    if (err<0) 
    {
        PRINT_ERR("%s is missing\n", pathname);
        goto done;
    }
    PRINT_DBG("media/depot OK\n");

    mediaTotalSegment = get_total_media_segments(pathname);
    
    snprintf(pathname,MS_MAX_PATHLEN,"%s-media/record",mountname); 
    err = stat(pathname, &buf_stat);
    if (err<0) 
    {
        PRINT_ERR("%s is missing\n", pathname);
        goto done;
    }
    PRINT_DBG("media/record OK\n");

    snprintf(pathname,MS_MAX_PATHLEN,"%s-media/preallocated",mountname); 
    err = stat(pathname, &buf_stat);
    if (err<0) 
    {
        PRINT_ERR("%s is missing\n", pathname);
        goto done;
    }
    PRINT_DBG("media/record OK\n");
    
    snprintf(pathname,MS_MAX_PATHLEN,"%s-nav/depot",mountname); 
    err = stat(pathname, &buf_stat);
    if (err<0) 
    {
        PRINT_ERR("%s is missing\n", pathname);
        goto done;
    }
    PRINT_DBG("nav/depot OK\n");


    snprintf(pathname,MS_MAX_PATHLEN,"%s-nav/record",mountname); 
    err = stat(pathname, &buf_stat);
    if (err<0) 
    {
        PRINT_ERR("%s is missing\n", pathname);
        goto done;
    }
    PRINT_DBG("nav/record OK\n");


#ifndef NO_REGFILE
    /* check registry file */
    snprintf(pathname,MS_MAX_PATHLEN,"%s-metadata/%s",mountname,MS_REGISTRY_FILENAME); 
    err = stat(pathname, &buf_stat);
    if (err<0) {
        PRINT_ERR("%s is missing\n", pathname);
        goto done;
    }
#endif

    /* check segment file numbes */
    registry = malloc(sizeof(MS_Registry_t));
    if (!registry) {
        PRINT_ERR("malloc failed\n");
        ret = eMS_ERR_SYSTEM;
        goto done;
    }

    ret = ms_read_registry(mountname,registry);
    if (ret<0) {
        PRINT_DBG("registry is missing\n");
        free(registry);
        goto done;
    }
    PRINT_DBG("registry OK\n");

    snprintf(pathname,MS_MAX_PATHLEN,"%s-media/depot",mountname); 
    PRINT_DBG("counting %s\n",pathname);
    depotCount = count_segments(pathname,eMS_MediaSegment);

    snprintf(pathname,MS_MAX_PATHLEN,"%s-media/record",mountname); 
    PRINT_DBG("counting %s\n",pathname);
    recordCount = count_segments(pathname,eMS_MediaSegment);

    snprintf(pathname,MS_MAX_PATHLEN,"%s-media/preallocated",mountname); 
    PRINT_DBG("counting %s\n",pathname);
    preallocCount = count_segments(pathname,eMS_MediaSegment);

    totalCount = depotCount+recordCount+preallocCount;

    PRINT_DBG("volume:   media segments depot:%d, record:%d, preallocated:%d, total:%d\n", depotCount, recordCount, preallocCount,totalCount);
    PRINT_DBG("expected total media segments :%d \n", mediaTotalSegment);

    if (mediaTotalSegment == (uint32_t)totalCount) 
    {
        PRINT_DBG("Media volume %s is ready!\n", device);
        status->state  = eMS_StateReady;
    } else 
    {
        status->state  = eMS_StateReady;    /* clean up needed */
        PRINT_ERR("unmatching segment numbers\n");
    }

    status->mediaSegmentSize = registry->mediaSegmentSize;
    status->mediaSegTotal = totalCount;
    status->mediaSegInuse = recordCount+preallocCount;

    free(registry);

done:    
    ms_unmount_volume(mountname);
error:
    PRINT_FUNCOUT();
    return ret;
}

int ms_check_volume(const char *device, MS_StorageStatus_t *status)
{
    MS_Partition_t *partitionTable;
    int ret = eMS_OK;
    int err;
    int numPartitions;
    int media,nav,metadata;
    char mountname[MS_MAX_PATHLEN];
    char pathname[MS_MAX_PATHLEN];
    int mediaIndex, navIndex, metaIndex;
    struct stat buf_stat;
    MS_Registry_t *registry;
    int depotCount, recordCount, preallocCount, totalCount;
    uint32_t mediaTotalSegment;

    PRINT_FUNCIN();

    partitionTable = malloc(sizeof(MS_Partition_t)*MS_MAX_PARTITION_ARRAY);
    if (!partitionTable) return eMS_ERR_SYSTEM;

    status->state = eMS_StateInvalid; // let's assume this is invalid

    /* check if the device is valid */
    ret = check_device(device);
    if (ret<0) {
        PRINT_ERR("(%s:%d)invalid device %s\n",__FUNCTION__,__LINE__,device);
        goto error;
    }

    media=nav=metadata=0;
    
    ret = read_partition(device,MS_MAX_PARTITION_ARRAY,partitionTable,&numPartitions);
    if (ret<0) {
        PRINT_ERR("(%s:%d)read_partition error %s\n",__FUNCTION__,__LINE__,device);
        goto error;
    }

    ret = find_partition(numPartitions,partitionTable,&mediaIndex,&navIndex,&metaIndex);
    if (ret<0) {    // not a valid volume
        PRINT_ERR("(%s:%d)valid media partition not found in %s\n",__FUNCTION__,__LINE__,device);
        ret = eMS_OK;
        goto error;
    }

    snprintf(status->mediaPartition,MS_DEVICE_NAME_LEN,"%s%d", device,partitionTable[mediaIndex].partitionNum);
    snprintf(status->navPartition,MS_DEVICE_NAME_LEN,"%s%d", device,partitionTable[navIndex].partitionNum);
    snprintf(status->metadatapartition,MS_DEVICE_NAME_LEN,"%s%d", device,partitionTable[metaIndex].partitionNum);
    
    status->startSector = partitionTable[mediaIndex].start;
    status->endSector = partitionTable[metaIndex].end;
    status->state  = eMS_StateEmpty;   // ok, found media partitions. this is empty media volume at least.
    PRINT_DBG("%s:%d: Media volume %s has valid partitions. now checking folders and files.\n",__FUNCTION__,__LINE__, device);

    // this can falsely fail if max volumes are already mounted. 
    err = check_mount_volume(device,mountname);
    if (err<0) {
        PRINT_ERR("check mount volume error %s\n",device);
        PRINT_FUNCOUT();
        status->mediaSegmentSize = 0;
        status->mediaSegTotal = 0;
        status->mediaSegInuse = 0;
        goto error;
    }

    // now check the folders
    snprintf(pathname,MS_MAX_PATHLEN,"%s-media/depot",mountname); 
    err = stat(pathname, &buf_stat);
    if (err<0) {
        PRINT_ERR("%s is missing\n", pathname);
        goto done;
    }
    PRINT_DBG("media/depot OK\n");

    mediaTotalSegment = get_total_media_segments(pathname);
    
    snprintf(pathname,MS_MAX_PATHLEN,"%s-media/record",mountname); 
    err = stat(pathname, &buf_stat);
    if (err<0) {
        PRINT_ERR("%s is missing\n", pathname);
        goto done;
    }
    PRINT_DBG("media/record OK\n");

    snprintf(pathname,MS_MAX_PATHLEN,"%s-media/preallocated",mountname); 
    err = stat(pathname, &buf_stat);
    if (err<0) {
        PRINT_ERR("%s is missing\n", pathname);
        goto done;
    }
    PRINT_DBG("media/record OK\n");
    
    snprintf(pathname,MS_MAX_PATHLEN,"%s-nav/depot",mountname); 
    err = stat(pathname, &buf_stat);
    if (err<0) {
        PRINT_ERR("%s is missing\n", pathname);
        goto done;
    }
    PRINT_DBG("nav/depot OK\n");


    snprintf(pathname,MS_MAX_PATHLEN,"%s-nav/record",mountname); 
    err = stat(pathname, &buf_stat);
    if (err<0) {
        PRINT_ERR("%s is missing\n", pathname);
        goto done;
    }
    PRINT_DBG("nav/record OK\n");


#ifndef NO_REGFILE
    /* check registry file */
    snprintf(pathname,MS_MAX_PATHLEN,"%s-metadata/%s",mountname,MS_REGISTRY_FILENAME); 
    err = stat(pathname, &buf_stat);
    if (err<0) {
        PRINT_ERR("%s is missing\n", pathname);
        goto done;
    }
#endif

    /* check segment file numbes */
    registry = malloc(sizeof(MS_Registry_t));
    if (!registry) {
        PRINT_ERR("malloc failed\n");
        ret = eMS_ERR_SYSTEM;
        goto done;
    }

    ret = ms_read_registry(mountname,registry);
    if (ret<0) {
        PRINT_DBG("registry is missing\n");
        free(registry);
        goto done;
    }
    PRINT_DBG("registry OK\n");

    snprintf(pathname,MS_MAX_PATHLEN,"%s-media/depot",mountname); 
    PRINT_DBG("counting %s\n",pathname);
    depotCount = count_segments(pathname,eMS_MediaSegment);

    snprintf(pathname,MS_MAX_PATHLEN,"%s-media/record",mountname); 
    PRINT_DBG("counting %s\n",pathname);
    recordCount = count_segments(pathname,eMS_MediaSegment);

    snprintf(pathname,MS_MAX_PATHLEN,"%s-media/preallocated",mountname); 
    PRINT_DBG("counting %s\n",pathname);
    preallocCount = count_segments(pathname,eMS_MediaSegment);

    totalCount = depotCount+recordCount+preallocCount;

    PRINT_DBG("volume:   media segments depot:%d, record:%d, preallocated:%d, total:%d\n", depotCount, recordCount, preallocCount,totalCount);
    PRINT_DBG("expected total media segments :%d \n", mediaTotalSegment);

    if (mediaTotalSegment == (uint32_t)totalCount) {
        PRINT_DBG("Media volume %s is ready!\n", device);
        status->state  = eMS_StateReady;
    } else {
        status->state  = eMS_StateReady;    /* clean up needed */
        PRINT_ERR("unmatching segment numbers\n");
    }

    status->mediaSegmentSize = registry->mediaSegmentSize;
    status->mediaSegTotal = totalCount;
    status->mediaSegInuse = recordCount+preallocCount;

    free(registry);

done:    
    ms_unmount_volume(mountname);
error:
    free(partitionTable);
    PRINT_FUNCOUT();
    return ret;
}

int ms_format_virtual_volume(const char *mediaLoopDevice, 
                             const char *navLoopDevice, 
                             const char *metaLoopDevice, 
                             unsigned long *segments)
{

    int ret = eMS_OK;
    char cmd[MS_MAX_CMDLEN];
    char mountpoint[MS_MAX_PATHLEN];
    char mountname[MS_MAX_PATHLEN];
    unsigned long segnum=0;
    MS_Registry_t *registry;
    struct stat buf_stat;
    
    ret = check_mounted_loop_device(mediaLoopDevice,mountpoint);
    if (ret<0) 
    {
        PRINT_ERR("%s is mounted at %s. trying unmount..", 
                    mediaLoopDevice, mountpoint);
        
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    ret = check_mounted_loop_device(navLoopDevice,mountpoint);
    if (ret<0) 
    {
        PRINT_ERR("%s is mounted at %s. trying unmount..", 
                    navLoopDevice,mountpoint);
        
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    ret = check_mounted_loop_device(metaLoopDevice,mountpoint);
    if (ret<0) 
    {
        PRINT_ERR("%s is mounted at %s. trying unmount..", 
                 metaLoopDevice, mountpoint);
        
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    ret = stat(mediaLoopDevice, &buf_stat);
    if (ret<0) 
    {
        PRINT_ERR("%s not found, format failed.\n", mediaLoopDevice);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    snprintf(cmd,MS_MAX_CMDLEN,"%s %s", MS_VIRTUAL_FORMAT_CMD ,mediaLoopDevice);
    PRINT_DBG(cmd);PRINT_DBG("\n");
    ret = system(cmd);
    if (ret<0) 
    {
        PRINT_ERR("%s:%s\n",cmd,strerror(errno));
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    ret = stat(navLoopDevice, &buf_stat);
    if (ret<0) 
    {
        PRINT_ERR("%s not found, format failed.\n", navLoopDevice);
        ret = eMS_ERR_SYSTEM;        
        goto error;
    }

    snprintf(cmd,MS_MAX_CMDLEN,"mkfs.ext4 -F -N 100000 -O dir_index %s",navLoopDevice);
    PRINT_DBG(cmd);PRINT_DBG("\n");
    ret = system(cmd);
    if (ret<0) {
        PRINT_ERR("%s:%s\n",cmd,strerror(errno));
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    ret = stat(metaLoopDevice, &buf_stat);
    if (ret<0) 
    {
        PRINT_ERR("%s not found, format failed.\n", metaLoopDevice);
        ret = eMS_ERR_SYSTEM;        
        goto error;
    }
    snprintf(cmd,MS_MAX_CMDLEN,"mkfs.ext4 -F -O dir_index %s",metaLoopDevice);
    PRINT_DBG(cmd);PRINT_DBG("\n");
    ret = system(cmd);
    if (ret<0) 
    {
        PRINT_ERR("%s:%s\n",cmd,strerror(errno));
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    /* temporarily mount to create folders and files */
    ret = check_mount_virtual_volume(mediaLoopDevice,
                                     navLoopDevice,
                                     metaLoopDevice,
                                     mountname);
    if (ret!=eMS_OK) 
    {
        PRINT_ERR(" check_mount_virtual_volume failed \n");
        goto error;
    }

    /* create depot and record folder in media partition */
    memset(mountpoint,0,MS_MAX_PATHLEN);
    snprintf(mountpoint,MS_MAX_PATHLEN,"%s-media",mountname);
    create_media_folder(mountpoint);

    /* create null segment files in media partition/depot folder */
    memset(mountpoint,0,MS_MAX_PATHLEN);
    snprintf(mountpoint,MS_MAX_PATHLEN,"%s-media/depot",mountname);    
    create_media_null_segment(mountpoint,&segnum);

    /* create depot and record folder in navigation partition */
    memset(mountpoint,0,MS_MAX_PATHLEN);
    snprintf(mountpoint,MS_MAX_PATHLEN,"%s-nav",mountname);
    create_media_folder(mountpoint);

    /* create null segment files in media partition/depot folder */
    memset(mountpoint,0,MS_MAX_PATHLEN);
    snprintf(mountpoint,MS_MAX_PATHLEN,"%s-nav/depot",mountname);    
    create_nav_null_segment(mountpoint,segnum);

    registry = malloc(sizeof(MS_Registry_t));
    if (!registry) 
    {
        PRINT_ERR("malloc failed\n");
        ms_unmount_volume(mountname);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    memset(registry,0,sizeof(MS_Registry_t));
    registry->version = MS_VERSION;
    registry->mediaSegmentSize = MS_MEDIA_SEGMENT_SIZE;

    ret = ms_update_registry(mountname,registry);
    free(registry);
    if (ret!=eMS_OK) {
        PRINT_ERR("ms_update_registry failed\n");
        goto error;
    }

    ret = ms_unmount_volume(mountname);
    if (ret!=eMS_OK) {
        PRINT_ERR("ms_unmount_volume failed\n");
        goto error;
    }
    
    *segments = segnum;
    
error:
    PRINT_FUNCOUT();
    return ret;

}

int ms_format_volume(const char *device, unsigned long *segments)
{
    int ret = eMS_OK;
    MS_Partition_t *partition;
    int numPartitions;
    int mediaIndex, navIndex, metaIndex;
    char cmd[MS_MAX_CMDLEN];
    char mountpoint[MS_MAX_PATHLEN];
    char mountname[MS_MAX_PATHLEN];
    unsigned long segnum=0;
    MS_Registry_t *registry;
    struct stat buf_stat;
    char devname[MS_DEVICE_NAME_LEN];
    int i;

#ifdef LABEL_SUPPORT
    int labelIndex;
#endif
    
    PRINT_FUNCIN();

    partition = malloc(sizeof(MS_Partition_t)*MS_MAX_PARTITION_ARRAY);
    if (!partition) return eMS_ERR_SYSTEM;

    ret = read_partition(device,MS_MAX_PARTITION_ARRAY,partition,&numPartitions);

    ret = find_partition(numPartitions, partition,&mediaIndex,&navIndex,&metaIndex);
    if (ret!=eMS_OK){
        PRINT_ERR("%s is not ready for format. create volume first.\n", device);
        goto error;
    }

    ret = check_mounted_partition(&partition[mediaIndex],mountpoint);
    if (ret<0) {
        PRINT_ERR("%s%d is mounted at %s. unmount to format.\n", 
                    partition[mediaIndex].device,partition[mediaIndex].partitionNum, mountpoint);
        
        ret = eMS_ERR_SYSTEM;
        goto error;
        }
    

    ret = check_mounted_partition(&partition[navIndex],mountpoint);
    if (ret<0) {
        PRINT_ERR("%s%d is mounted at %s. trying unmount..\n", 
                    partition[navIndex].device,partition[navIndex].partitionNum, mountpoint);
        
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    ret = check_mounted_partition(&partition[metaIndex],mountpoint);
    if (ret<0) {
        PRINT_ERR("%s%d is mounted at %s. trying unmount..\n", 
                    partition[metaIndex].device,partition[metaIndex].partitionNum, mountpoint);
        
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

#ifdef LABEL_SUPPORT
     if (gLabelEnable) {
         /* 
         - find label partition : label_find_partition()
         - if exists, write label to the partition : label_write()
         - if not exist, create one and write label : label_create_partition(), label_write()
         */
 
         ret = label_find_partition(numPartitions,partition,&labelIndex);
         if(ret!=eMS_OK) {
             PRINT_ERR("label partition not found\n"); 
             ret = eMS_ERR_WRONG_LABEL;
             goto error;
         }
         
         ret = label_write(device,labelIndex+1);
         if (ret<0) {
             PRINT_DBG("failed to write label\n");
             ret = eMS_ERR_SYSTEM;
             goto error;
         }
     }
#endif

    snprintf(devname,MS_DEVICE_NAME_LEN,"%s%d", partition[mediaIndex].device,partition[mediaIndex].partitionNum);
    for (i=0;i<10;i++) {
        ret = stat(devname, &buf_stat);
        if (ret<0) {
            PRINT_DBG("---%s is missing. retrying in 1 sec..\n", devname);
            sleep(1);
        } else {
            PRINT_DBG("---%s is ready.\n", devname);
            break;
        }
    }

    if (i==10) {
        PRINT_ERR("%s not found, format failed.\n", devname);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    snprintf(cmd,MS_MAX_CMDLEN,"%s %s%d", MS_MEDIA_FORMAT_CMD ,partition[mediaIndex].device,partition[mediaIndex].partitionNum);
    PRINT_DBG(cmd);PRINT_DBG("\n");
    ret = system(cmd);
    if (ret<0) {
        PRINT_ERR("%s:%s\n",cmd,strerror(errno));
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    snprintf(devname,MS_DEVICE_NAME_LEN,"%s%d", partition[navIndex].device,partition[navIndex].partitionNum);
    ret = stat(devname, &buf_stat);
    for (i=0;i<10;i++) {
        ret = stat(devname, &buf_stat);
        if (ret<0) {
            PRINT_DBG("---%s is missing. retrying in 1 sec..\n", devname);
            sleep(1);
        } else {
            PRINT_DBG("---%s is ready.\n", devname);
            break;
        }
    }

    if (i==10) {
        PRINT_ERR("%s not found, format failed.\n", devname);
        ret = eMS_ERR_SYSTEM;        
        goto error;
    }

    snprintf(cmd,MS_MAX_CMDLEN,"mkfs.ext4 -N 100000 -O dir_index %s%d", partition[navIndex].device,partition[navIndex].partitionNum);
    PRINT_DBG(cmd);PRINT_DBG("\n");
    ret = system(cmd);
    if (ret<0) {
        PRINT_ERR("%s:%s\n",cmd,strerror(errno));
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    snprintf(devname,MS_DEVICE_NAME_LEN,"%s%d", partition[metaIndex].device,partition[metaIndex].partitionNum);
    for (i=0;i<10;i++) {
        ret = stat(devname, &buf_stat);
        if (ret<0) {
            PRINT_DBG("---%s is missing. retrying in 1 sec..\n", devname);
            sleep(1);
        } else {
            PRINT_DBG("---%s is ready.\n", devname);
            break;
        }
    }

    if (i==10) {
        PRINT_ERR("%s not found, format failed.\n", devname);
        ret = eMS_ERR_SYSTEM;        
        goto error;
    }

    snprintf(cmd,MS_MAX_CMDLEN,"mkfs.ext4 -O dir_index %s%d", partition[metaIndex].device,partition[metaIndex].partitionNum);
    PRINT_DBG(cmd);PRINT_DBG("\n");
    ret = system(cmd);
    if (ret<0) {
        PRINT_ERR("%s:%s\n",cmd,strerror(errno));
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    /* temporarily mount to create folders and files */
    ret = check_mount_volume(device, mountname);
    if (ret!=eMS_OK) goto error;

    /* create depot and record folder in media partition */
    memset(mountpoint,0,MS_MAX_PATHLEN);
    snprintf(mountpoint,MS_MAX_PATHLEN,"%s-media",mountname);
    create_media_folder(mountpoint);

    /* create null segment files in media partition/depot folder */
    memset(mountpoint,0,MS_MAX_PATHLEN);
    snprintf(mountpoint,MS_MAX_PATHLEN,"%s-media/depot",mountname);    
    create_media_null_segment(mountpoint,&segnum);

    /* create depot and record folder in navigation partition */
    memset(mountpoint,0,MS_MAX_PATHLEN);
    snprintf(mountpoint,MS_MAX_PATHLEN,"%s-nav",mountname);
    create_media_folder(mountpoint);

    /* create null segment files in media partition/depot folder */
    memset(mountpoint,0,MS_MAX_PATHLEN);
    snprintf(mountpoint,MS_MAX_PATHLEN,"%s-nav/depot",mountname);    
    create_nav_null_segment(mountpoint,segnum);

    registry = malloc(sizeof(MS_Registry_t));
    if (!registry) {
        PRINT_ERR("malloc failed\n");
        ms_unmount_volume(mountname);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    memset(registry,0,sizeof(MS_Registry_t));
    registry->version = MS_VERSION;
    registry->mediaSegmentSize = MS_MEDIA_SEGMENT_SIZE;

    ret = ms_update_registry(mountname,registry);
    free(registry);
    if (ret!=eMS_OK) {
        PRINT_ERR("ms_update_registry failed\n");
        goto error;
    }

    ret = ms_unmount_volume(mountname);
    if (ret!=eMS_OK) {
        PRINT_ERR("ms_unmount_volume failed\n");
        goto error;
    }
    
    *segments = segnum;
    
error:
    free (partition);
    PRINT_FUNCOUT();
    return ret;
}

int ms_mount_virtual_volume(const char *mediaLoopDevice,
                            const char *navLoopDevice,
                            const char *metaLoopDevice,
                            char *mountname,
                            int *pDeletedFileCount)
{
    char targetMedia[MS_MAX_PATHLEN];
    char targetNav[MS_MAX_PATHLEN];
    char targetMeta[MS_MAX_PATHLEN];
    char cmd[MS_MAX_CMDLEN];
    int ret = eMS_OK;
    int volIndex;

    PRINT_FUNCIN();
    
    volIndex = get_mount_index();

    if (volIndex<0 || volIndex>0) 
    {
        PRINT_ERR("maximum volume %d already mounted!\n", MS_MAX_DVRVOLUME);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    snprintf(targetMedia,MS_MAX_PATHLEN,"/mnt/dvr%d-media",volIndex);
    printf("check media partition %s\n", mediaLoopDevice);
#if 1
    snprintf(cmd,MS_MAX_CMDLEN,"%s %s",MS_MEDIA_FSCK_CMD,mediaLoopDevice);
    system(cmd);
#endif

    printf("mount media partition: %s:%s\n",  mediaLoopDevice, targetMedia);

    snprintf(cmd,MS_MAX_CMDLEN,"mount -o loop -t ext4 %s %s",mediaLoopDevice,targetMedia);
    ret = system(cmd);
    if (ret!=0) 
    {
        PRINT_ERR("mount failed:%s\n",strerror(errno));
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    /* mount nav partition */
    snprintf(targetNav,MS_MAX_PATHLEN,"/mnt/dvr%d-nav",volIndex);
    snprintf(cmd,MS_MAX_CMDLEN,"e2fsck -p %s",navLoopDevice);
    system(cmd);
    
    printf("mount nav partition  : %s:%s\n",  navLoopDevice, targetNav); 
       
    snprintf(cmd,MS_MAX_CMDLEN,"mount -o loop -t ext4 %s %s",navLoopDevice,targetNav);
    ret = system(cmd);
    if (ret!=0) 
    {
        PRINT_ERR("mount failed:%s\n",strerror(errno));
        umount(targetMedia);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }
    
    snprintf(targetMeta,MS_MAX_PATHLEN,"/mnt/dvr%d-metadata",volIndex);
    
    printf("check metadata partition %s\n", metaLoopDevice);
    snprintf(cmd,MS_MAX_CMDLEN,"e2fsck -p %s",metaLoopDevice);
    system(cmd);
    
    printf("mount meta partition : %s:%s\n",  metaLoopDevice, targetMeta);
    snprintf(cmd,MS_MAX_CMDLEN,"mount -o loop -t ext4 %s %s",metaLoopDevice,targetMeta);
    ret = system(cmd);
    if (ret!=0) 
    {
        PRINT_ERR("mount failed:%s\n",strerror(errno));
        umount(targetNav);
        umount(targetMedia);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    snprintf(mountname,MS_DEVICE_NAME_LEN,"/mnt/dvr%d",volIndex);   // virtual mount name. used for fuse

#if 1
    if (getenv("NO_HOUSE_CLEAN")) 
    { /* disable file delete */
        ms_recover_volume(mountname,0,pDeletedFileCount);
    } 
    else 
    {
        ms_recover_volume(mountname,1,pDeletedFileCount);
    }
#endif

error:
    PRINT_FUNCOUT();
    return ret;
}

int ms_mount_volume(const char *device, char *mountname, int *pDeletedFileCount)
{
    char src[MS_MAX_PATHLEN];
    char targetMedia[MS_MAX_PATHLEN];
    char targetNav[MS_MAX_PATHLEN];
    char targetMeta[MS_MAX_PATHLEN];
    char fsckCmd[MS_MAX_CMDLEN];
    int ret = eMS_OK;
    MS_Partition_t *partition;
    int numPartitions;
    int mediaIndex, navIndex,metaIndex;
    int volIndex;

    PRINT_FUNCIN();
    partition = malloc(sizeof(MS_Partition_t)*MS_MAX_PARTITION_ARRAY);
    if(!partition) return eMS_ERR_SYSTEM;
        
    ret = check_volume_partitions(device);
    if (ret != eMS_OK) { 
        PRINT_ERR("%s is an invalid media volume\n",device);
        ret = eMS_ERR_INVALID_PARAM;
        goto error;
    }

    volIndex = get_mount_index();

    if (volIndex<0) {
        PRINT_ERR("maximum volume %d already mounted!\n", MS_MAX_DVRVOLUME);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    ret = read_partition(device,MS_MAX_PARTITION_ARRAY,partition,&numPartitions);
    if (ret!=eMS_OK) {
        goto error;
    }

    ret = find_partition(numPartitions, partition,&mediaIndex,&navIndex,&metaIndex);
    if (ret!=eMS_OK) {
        goto error;
    }
    

    /* mount media partition */
    memset(src,0,MS_MAX_PATHLEN); memset(targetMedia,0,MS_MAX_PATHLEN);

    snprintf(src,MS_MAX_PATHLEN,"%s%d",partition[mediaIndex].device,partition[mediaIndex].partitionNum);
    snprintf(targetMedia,MS_MAX_PATHLEN,"/mnt/dvr%d-media",volIndex);

    printf("check media partition %s\n", src);
#if 1
    snprintf(fsckCmd,MS_MAX_CMDLEN,"%s %s",MS_MEDIA_FSCK_CMD,src);
    system(fsckCmd);
#endif

    printf("mount media partition: %s:%s\n",  src, targetMedia);
    ret = mount(src,targetMedia,MS_MEDIA_FILESYSTEMTYPE,MS_NOATIME|MS_NODIRATIME,0);

    if (ret!=0) {
        PRINT_ERR("mount failed:%s\n",strerror(errno));
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    /* mount nav partition */
    memset(src,0,MS_MAX_PATHLEN); memset(targetNav,0,MS_MAX_PATHLEN);

    snprintf(src,MS_MAX_PATHLEN,"%s%d",partition[navIndex].device,partition[navIndex].partitionNum);
    snprintf(targetNav,MS_MAX_PATHLEN,"/mnt/dvr%d-nav",volIndex);

    printf("check nav partition %s\n", src);
    snprintf(fsckCmd,MS_MAX_CMDLEN,"e2fsck -p %s",src);
    system(fsckCmd);
    
    printf("mount nav partition  : %s:%s\n",  src, targetNav);    
    ret = mount(src,targetNav,MS_NAV_FILESYSTEMTYPE,MS_NOATIME|MS_NODIRATIME,0);

    if (ret!=0) {
        PRINT_ERR("mount failed:%s\n",strerror(errno));
        umount(targetMedia);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }
    
    /* mount metadata partition */
    memset(src,0,MS_MAX_PATHLEN); memset(targetMeta,0,MS_MAX_PATHLEN);

    snprintf(src,MS_MAX_PATHLEN,"%s%d",partition[metaIndex].device,partition[metaIndex].partitionNum);
    snprintf(targetMeta,MS_MAX_PATHLEN,"/mnt/dvr%d-metadata",volIndex);
    
    printf("check metadata partition %s\n", src);
    snprintf(fsckCmd,MS_MAX_CMDLEN,"e2fsck -p %s",src);
    system(fsckCmd);
    
    printf("mount meta partition : %s:%s\n",  src, targetMeta);
    ret = mount(src,targetMeta,MS_META_FILESYSTEMTYPE,MS_NOATIME|MS_NODIRATIME,0);

    if (ret!=0) {
        PRINT_ERR("mount failed:%s\n",strerror(errno));
        umount(targetNav);
        umount(targetMedia);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    snprintf(mountname,MS_DEVICE_NAME_LEN,"/mnt/dvr%d",volIndex);   // virtual mount name. used for fuse

#if 1
    if (getenv("NO_HOUSE_CLEAN")) { /* disable file delete */
        ms_recover_volume(mountname,0,pDeletedFileCount);
    } else {
        ms_recover_volume(mountname,1,pDeletedFileCount);
    }
#endif

error:
    free(partition);
    PRINT_FUNCOUT();
    return ret;
}

int ms_unmount_volume(const char *mountname)
{
    char mountpoint[MS_MAX_PATHLEN];
    int ret = eMS_OK;

    PRINT_FUNCIN();

    sync();
    sync();
    sync();
    sleep(5);

    snprintf(mountpoint,MS_MAX_PATHLEN,"%s-media",mountname);
    PRINT_DBG("unmounting %s\n", mountpoint);
    ret = umount(mountpoint);
    if (ret<0) {
        PRINT_ERR("umount error:%s\n",strerror(errno));
        ret = eMS_ERR_SYSTEM;
    }

    snprintf(mountpoint,MS_MAX_PATHLEN,"%s-nav",mountname);
    PRINT_DBG("unmounting %s\n", mountpoint);
    ret = umount(mountpoint);
    if (ret<0) {
        PRINT_ERR("umount error:%s\n",strerror(errno));
        ret = eMS_ERR_SYSTEM;
    }

    snprintf(mountpoint,MS_MAX_PATHLEN,"%s-metadata",mountname);
    PRINT_DBG("unmounting %s\n", mountpoint);
    ret = umount(mountpoint);
    if (ret<0) {
        PRINT_ERR("umount error:%s\n",strerror(errno));
        ret = eMS_ERR_SYSTEM;
    }

    PRINT_FUNCOUT();
    return ret;
}

int ms_unmount_device(const char *device)
{
    char mountpoint[MS_MAX_PATHLEN];
    
    int ret = eMS_OK;
    MS_Partition_t *partition;
    int numPartitions;
    int mediaIndex, navIndex,metaIndex;

    PRINT_FUNCIN();
    partition = malloc(sizeof(MS_Partition_t)*MS_MAX_PARTITION_ARRAY);
    if(!partition) return eMS_ERR_SYSTEM;    
        
    ret = read_partition(device,MS_MAX_PARTITION_ARRAY,partition,&numPartitions);
    if (ret!=eMS_OK) {
        goto error;
    }

    ret = find_partition(numPartitions, partition,&mediaIndex,&navIndex,&metaIndex);
    if (ret!=eMS_OK) {
        goto error;
    }

    ret = check_mounted_partition(&partition[mediaIndex],mountpoint);
    if(ret<0) {
        umount(mountpoint);
        PRINT_DBG("unmounted %s\n",mountpoint);
    }

    ret = check_mounted_partition(&partition[navIndex],mountpoint);
    if(ret<0) {
        umount(mountpoint);
        PRINT_DBG("unmounted %s\n",mountpoint);
    }

    ret = check_mounted_partition(&partition[metaIndex],mountpoint);
    if(ret<0) {
        umount(mountpoint);
        PRINT_DBG("unmounted %s\n",mountpoint);
    }

error:
    PRINT_FUNCOUT();
    free(partition);
    return ret;
}

int ms_create_virtual_volume(const char *device, MS_createPartitionSettings_t *pSettings)
{
    int ret = eMS_OK;
    
    ret = check_virtual_device(device);
    if(ret < 0) 
    {
        ret=mkdir(device,0700);
        if(ret < 0) 
        {
            PRINT_ERR("creating %s directory failed %d",device,ret);
            ret = eMS_ERR_SYSTEM;
            goto error;
        }
        else
        {
            uint64_t mediaSize; 
            uint64_t navSize; 
            uint64_t metaSize;

            snprintf(pSettings->mediaPartition,MS_DEVICE_NAME_LEN,"%s/%s",device,MS_LOOP_DEVICE_MEDIA);
            snprintf(pSettings->navPartition,MS_DEVICE_NAME_LEN,"%s/%s",device,MS_LOOP_DEVICE_NAVIGATION);
            snprintf(pSettings->metadataPartition,MS_DEVICE_NAME_LEN,"%s/%s",device,MS_LOOP_DEVICE_METADATA);
            metaSize = MS_META_PT_SIZE;
            metaSize *= 1024*1024;
            mediaSize = pSettings->size*1024*1024;
            mediaSize -= metaSize;
            navSize = (mediaSize)/120; 
            mediaSize -= navSize;
            ret = create_null_file(pSettings->metadataPartition,metaSize);
            if(ret) 
            {
                PRINT_ERR("unable to pre-allocate metadata loop device file %s",pSettings->metadataPartition);
                rmdir(device);
                ret = eMS_ERR_SYSTEM;
                goto error;
            }
            ret = create_null_file(pSettings->navPartition,navSize);
            if(ret) 
            {
                PRINT_ERR("unable to pre-allocate nav loop device file %s",pSettings->navPartition);   
                unlink(pSettings->metadataPartition);
                rmdir(device);
                ret = eMS_ERR_SYSTEM;
                goto error;
            }
            ret = create_null_file(pSettings->mediaPartition,mediaSize);
            if(ret) 
            {
                PRINT_ERR("unable to pre-allocate media loop device file %s",pSettings->mediaPartition);      
                unlink(pSettings->metadataPartition);
                unlink(pSettings->navPartition);
                rmdir(device);
                ret = eMS_ERR_SYSTEM;
            }

        }

    }
    else
    {
        PRINT_DBG("loop device for media, navigation and meta data already available");
        ret = eMS_ERR_VOLUME_EXISTS;
        goto error;
    }
error:
    return ret;
}

int ms_create_volume(const char *device, MS_createPartitionSettings_t *pSettings)
{
    int ret = eMS_OK;
    int numPartitions;
    MS_Partition_t *partitionTable;
    int mediaIndex, navIndex, metaIndex;

    PRINT_FUNCIN();

    partitionTable = malloc(sizeof(MS_Partition_t)*MS_MAX_PARTITION_ARRAY);
    if (!partitionTable) return eMS_ERR_SYSTEM;

    PRINT_DBG("device = %s\n",device);
    ret = read_partition(device, MS_MAX_PARTITION_ARRAY, partitionTable, &numPartitions);
    if(ret<0) {
        PRINT_DBG("read_partition failed for %s:%d\n", device, ret);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }
    if (numPartitions==0) {
        PRINT_DBG("no partition found\n");
    }

    ret = find_partition(numPartitions,partitionTable,&mediaIndex,&navIndex,&metaIndex);
    if ( (mediaIndex!=MS_INVALID_INDEX) || (navIndex != MS_INVALID_INDEX) || (metaIndex != MS_INVALID_INDEX)) {
        PRINT_ERR("media partitions already exists in %s\n",device);
        ret = eMS_ERR_INVALID_PARAM;    // partition already there.
        goto error;
    }
    
    ret = create_table(device, pSettings, MS_MAX_PARTITION_ARRAY, partitionTable, &numPartitions);

    if(ret <0) {
        PRINT_DBG("ms_create_partition_table failed:%d\n", ret);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }   

    PRINT_DBG("now %d partitionTable\n", numPartitions);
    print_partition(numPartitions,partitionTable);

    ret = update_partition(device, numPartitions, partitionTable);
    if(ret<0) {
        PRINT_DBG("update_partition failed:%d\n", ret);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

error:
    free(partitionTable);
    PRINT_FUNCOUT();
    return ret;

}

int ms_read_registry(const char *mountname, MS_Registry_t *registry)
{
#ifdef NO_REGFILE
    unsigned int volnum;

    volnum = mountname[strlen(mountname)-1]-'0';   /* /mnt/dvr0 */ 
    if (volnum>=MS_MAX_DVRVOLUME) {
        volnum = mountname[strlen(mountname)-1]-'a';   /* /mnt/dvra */ 
        if (volnum>=MS_MAX_DVRVOLUME)        
            return eMS_ERR_INVALID_VOLUME;
    }
    memcpy(registry,&gRegistry[volnum],sizeof(MS_Registry_t));
#else
    int fd;
    char name[MS_MAX_PATHLEN];

    snprintf(name,MS_MAX_PATHLEN, "%s-metadata/%s",mountname,MS_REGISTRY_FILENAME);
    fd = open(name,O_RDONLY|O_CLOEXEC);
    if (fd<0) {
        PRINT_ERR("%s open error:%s\n",name, strerror(errno));
        PRINT_FUNCOUT();
        return eMS_ERR_SYSTEM;
    }

    read(fd, (void*)registry, sizeof(MS_Registry_t));
    close(fd);
#endif  /* NO_REGFILE */

    return eMS_OK;
}

int ms_update_registry(const char *mountname, MS_Registry_t *registry)
{
#ifdef NO_REGFILE
    unsigned int volnum;

    volnum = mountname[strlen(mountname)-1]-'0';   /* /mnt/dvr0 */ 
    if (volnum>=MS_MAX_DVRVOLUME) {
        volnum = mountname[strlen(mountname)-1]-'a';   /* /mnt/dvra */ 
        if (volnum>=MS_MAX_DVRVOLUME)        
            return eMS_ERR_INVALID_VOLUME;
    }

    memcpy(&gRegistry[volnum],registry,sizeof(MS_Registry_t));
#else 

    int fd;
    char name[MS_MAX_PATHLEN];

#ifdef ENABLE_PROXY
    unsigned index;
#endif

    snprintf(name,MS_MAX_PATHLEN, "%s-metadata/%s",mountname,MS_REGISTRY_FILENAME);
    PRINT_DBG("update registry %s\n",name);

    fd = open(name,O_WRONLY|O_CREAT|O_CLOEXEC,0666);
    if (fd<0) {
        PRINT_FUNCOUT();
        return eMS_ERR_SYSTEM;
    }

    write(fd, (void*)registry, sizeof(MS_Registry_t));
#ifndef ENABLE_PROXY
    fdatasync(fd);
#endif
    close(fd);

#ifdef ENABLE_PROXY
    index = mountname[strlen(mountname)-1]-'0';
    if (index<MS_MAX_DVRVOLUME) {
        gRegProxyIndex[index] = 1;
        B_Event_Set(gRegProxyEvent);
    }
#endif
            
#endif /* NO_REGFILE */

    return eMS_OK;
}

int ms_read_quota(const char *mountname, const char *dirname, MS_DirQuota_t *dirQuota)
{
    int ret = eMS_OK;
    int fd;
    char name[MS_MAX_PATHLEN];

    PRINT_FUNCIN();    

    snprintf(name,MS_MAX_PATHLEN, "%s-metadata/%s/%s",mountname,dirname,MS_QUOTA_FILENAME);

    fd = open(name,O_RDONLY|O_CLOEXEC);
    if (fd<0) {
        PRINT_DBG("quota is not set for %s\n",name);
        ret = eMS_ERR_SYSTEM;
        goto error;
    }

    read(fd, (void*)dirQuota, sizeof(MS_DirQuota_t));
    close(fd);

    PRINT_DBG("%s: %d used %d shared / %d quota\n", dirname, dirQuota->used, dirQuota->shared, dirQuota->quota);

error:
    PRINT_FUNCOUT();
    return ret;
            
}

/*
   update or create quota file
*/
int ms_update_quota(const char *mountname, const char *dirname, MS_DirQuota_t *dirQuota)
{
    int ret = eMS_OK;
    int fd;
    char name[MS_MAX_PATHLEN];

    PRINT_FUNCIN();    
    snprintf(name,MS_MAX_PATHLEN, "%s-metadata/%s/%s",mountname,dirname,MS_QUOTA_FILENAME);
  
    if (dirQuota->quota==0) {
        unlink(name);
    } else {
        fd = open(name,O_WRONLY|O_CREAT|O_CLOEXEC);
        if (fd<0) {
            PRINT_ERR("%s open error:%s\n",name, strerror(errno));
            ret = eMS_ERR_SYSTEM;
            goto error;
        }        
        write (fd, (void*)dirQuota, sizeof(MS_DirQuota_t));
        fsync(fd);
        close(fd);
    }

    PRINT_DBG("%s: %d used %d shared / %d quota\n", dirname, dirQuota->used, dirQuota->shared, dirQuota->quota);

error:
    PRINT_FUNCOUT();
    return ret;
            
}

int ms_get_segnum(
    const char *mountname, 
    int *mediaCountDepot, int *mediaCountRecord, int *mediaCountPrealloc,
    int *navCountDepot,int *navCountRecord)
{
    char path[MS_MAX_PATHLEN];
    PRINT_FUNCIN();

    /* print number of segment in each folder */
    snprintf(path,MS_MAX_PATHLEN, "%s-media/depot",mountname);
    *mediaCountDepot = count_segments(path, eMS_MediaSegment);
    snprintf(path,MS_MAX_PATHLEN, "%s-media/record",mountname);
    *mediaCountRecord= count_segments(path, eMS_MediaSegment);
    snprintf(path,MS_MAX_PATHLEN, "%s-media/preallocated",mountname);
    *mediaCountPrealloc= count_segments(path, eMS_MediaSegment);

    snprintf(path,MS_MAX_PATHLEN, "%s-nav/depot",mountname);
    *navCountDepot = count_segments(path, eMS_NavSegment);
    snprintf(path,MS_MAX_PATHLEN, "%s-nav/record",mountname);
    *navCountRecord = count_segments(path, eMS_NavSegment);

    PRINT_FUNCOUT();
    return eMS_OK;
}
    
int ms_set_label(int enable, unsigned char *label, unsigned int labelSize)
{
#ifdef LABEL_SUPPORT
    gLabelEnable = enable;
    gLabel.data = label;
    gLabel.size = labelSize;
    return eMS_OK;
#else
    BSTD_UNUSED(enable);
    BSTD_UNUSED(label);
    BSTD_UNUSED(labelSize);
    return eMS_ERR_NOT_SUPPORTED;
#endif
}


