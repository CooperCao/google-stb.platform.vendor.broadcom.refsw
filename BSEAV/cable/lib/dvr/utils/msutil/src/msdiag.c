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
#include "msutil.h"
#include "msdiag.h"
#include <assert.h>
#include <time.h>

#include "b_os_lib.h"
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"
#include "b_dvr_const.h"
#include "b_dvr_datatypes.h"
#include "b_dvr_segmentedfile.h"

BDBG_MODULE(houseclean);

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

static int gLogLevel=7;
static FILE *gfpLog;
static int gErrorCount=0;
static int gDeleteCount=0;

#define PRINT_L1(...) {if(gLogLevel&1) {printf(__VA_ARGS__);fprintf(gfpLog,__VA_ARGS__);}}    /* print level 1. console and log file*/
#define PRINT_L2(...) {if(gLogLevel&2) {fprintf(gfpLog,__VA_ARGS__);}}                        /* print level 2. log file only*/
#define PRINT_L3(...) {if(gLogLevel&4) printf(__VA_ARGS__);}                                  /* print level 3. console only*/

extern int count_segments(const char *path, MS_SegmentType_t type);
extern unsigned long get_total_media_segments (const char *path);

void dump_file_refcnt(const char *path,const char *infofile, MS_Registry_t *registry,int log)
{
    char tspath[MS_MAX_PATHLEN];
    char navpath[MS_MAX_PATHLEN];
    char infopath[MS_MAX_PATHLEN];
    int fd;
    int fd_ts,fd_nav;
    B_DVR_Media *media;
    B_DVR_SegmentedFileNode segment_ts, segment_nav;
    int i=0;
    int segnum;

    snprintf(infopath,MS_MAX_PATHLEN,"%s/%s",path,infofile);

    fd = open(infopath,O_RDONLY|O_CLOEXEC);
    if (fd<0) {
        printf("%s:%s\n",infopath,strerror(errno));
        return;
    }

    media = malloc(sizeof(B_DVR_Media));
    if(!media) {
        PRINT_ERR("malloc failed\n");
        close(fd);
        return;
    }
    memset(media,0,sizeof(B_DVR_Media));
    read(fd,media,sizeof(B_DVR_Media));
    close(fd);
    
    memset(tspath,0,MS_MAX_PATHLEN);
    memset(navpath,0,MS_MAX_PATHLEN);
    snprintf(tspath,MS_MAX_PATHLEN,"%s/%s",path,media->mediaFileName);
    snprintf(navpath,MS_MAX_PATHLEN,"%s/%s",path,media->navFileName);

    free(media);

    if (log) 
        PRINT_L1("---%-40s-----------------------------------\n",infofile)
    else
        PRINT_L3("---%-40s-----------------------------------\n",infofile)

    fd_ts = open(tspath,O_RDONLY|O_CLOEXEC);
    if (fd_ts<0) {
        if (log)
            PRINT_L1("%s:%s\n",tspath,strerror(errno))
        else
            PRINT_L3("%s:%s\n",tspath,strerror(errno))
        return;
    }
    fd_nav= open(navpath,O_RDONLY|O_CLOEXEC);
    if (fd_nav<0) {
        if (log)
            PRINT_L1("%s:%s\n",navpath,strerror(errno))
        else
            PRINT_L3("%s:%s\n",navpath,strerror(errno))
        close(fd_ts);
        return;
    }

    memset(&segment_ts,0,sizeof(B_DVR_SegmentedFileNode));
    memset(&segment_nav,0,sizeof(B_DVR_SegmentedFileNode));

    if (log)
        PRINT_L1("   num  fileName           refcnt\n")
    else
        PRINT_L3("   num  fileName           refcnt\n")

    i=0;
    while(read(fd_ts,&segment_ts,sizeof(B_DVR_SegmentedFileNode))) {
        memset(tspath,0,MS_MAX_PATHLEN);
        strncpy(tspath,&segment_ts.fileName[3],6); /* get segment number */
        segnum = atoi(tspath);               
        if (log)
            PRINT_L1("%6d: %s\t%8d\n",i++,
               segment_ts.fileName,
               registry->mediaSegRefCount[segnum])
        else
            PRINT_L3("%6d: %s\t%8d\n",i++,
               segment_ts.fileName,
               registry->mediaSegRefCount[segnum])
        memset(&segment_ts,0,sizeof(B_DVR_SegmentedFileNode));
    }

    i=0;
    while (read(fd_nav,&segment_nav,sizeof(B_DVR_SegmentedFileNode))) {        
        memset(tspath,0,MS_MAX_PATHLEN);
        strncpy(tspath,&segment_nav.fileName[3],6); /* get segment number */
        segnum = atoi(tspath);         
        if (log)
            PRINT_L1("%6d: %s\t%8d\n",i++,
               segment_nav.fileName,
               registry->navSegRefCount[segnum])
        else               
            PRINT_L3("%6d: %s\t%8d\n",i++,
               segment_nav.fileName,
               registry->navSegRefCount[segnum])
        memset(&segment_nav,0,sizeof(B_DVR_SegmentedFileNode));
    }
    close(fd_ts);
    close(fd_nav);
    
    if (log)
        PRINT_L1("\n")
    else
        PRINT_L3("\n")

}

/*
    interval: in minutes
*/
void dump_refcnt(const char *mountname, int count, int interval, int log)
{
    char path[MS_MAX_PATHLEN];
    MS_Registry_t *registry;
    DIR *pdir;    
    struct dirent *pent;
    int i;
    
    registry = malloc(sizeof(MS_Registry_t));
    if(!registry) return;

    memset(path,0,MS_MAX_PATHLEN);    /* use tspath for mountname */
    snprintf(path,MS_MAX_PATHLEN,"%s-metadata",mountname);


    i = 1;
    while(1) {
        printf("*** count %-33i***********************************\n",i);
        pdir = opendir(path);

        if (!pdir) {
            fprintf(stderr,"%s opendir error:%s\n", path, strerror(errno));
            goto err_registry;
        }

        while ((pent = readdir(pdir))) {
            if (!fnmatch("*.info", pent->d_name, 0)) {  /* found */
                ms_read_registry(mountname,registry);
                dump_file_refcnt(path,pent->d_name,registry,log);
            }
        }
        
        closedir(pdir);

        if (++i>count) break;

        sleep(60*interval);
    }

err_registry:
    free (registry);
}

void dump_file(const char *path,const char *infofile)
{
    char tspath[MS_MAX_PATHLEN];
    char navpath[MS_MAX_PATHLEN];
    char infopath[MS_MAX_PATHLEN];
    int fd;
    int fd_ts,fd_nav;
    B_DVR_Media *media;
    B_DVR_SegmentedFileNode segment_ts, segment_nav;
    MS_Registry_t *registry;
    int i=0;
    int segnum;

    snprintf(infopath,MS_MAX_PATHLEN,"%s/%s",path,infofile);

    registry = malloc(sizeof(MS_Registry_t));
    if(!registry) return;

    memset(tspath,0,MS_MAX_PATHLEN);    /* use tspath for mountname */
    strncpy(tspath,path,sizeof("/mnt/dvrX")-1);

    ms_read_registry(tspath,registry);
    
    fd = open(infopath,O_RDONLY|O_CLOEXEC);
    if (fd<0) {
        printf("%s:%s\n",infopath,strerror(errno));
        goto err_registry;
    }

    media = malloc(sizeof(B_DVR_Media));
    if(!media) {
        PRINT_ERR("malloc failed\n");
        close(fd);
        goto err_registry;
    }
    memset(media,0,sizeof(B_DVR_Media));
    read(fd,media,sizeof(B_DVR_Media));
    close(fd);

    memset(tspath,0,MS_MAX_PATHLEN);
    memset(navpath,0,MS_MAX_PATHLEN);
    snprintf(tspath,MS_MAX_PATHLEN,"%s/%s",path,media->mediaFileName);
    snprintf(navpath,MS_MAX_PATHLEN,"%s/%s",path,media->navFileName);

    printf("---%-40s-----------------------------------\n",infofile);

    fd_ts = open(tspath,O_RDONLY|O_CLOEXEC);
    if (fd_ts<0) {
        printf("%s:%s\n",tspath,strerror(errno));
        goto err_media;
    }
    fd_nav= open(navpath,O_RDONLY|O_CLOEXEC);
    if (fd_nav<0) {
        printf("%s:%s\n",navpath,strerror(errno));
        close(fd_ts);
        goto err_media;
    }

    memset(&segment_ts,0,sizeof(B_DVR_SegmentedFileNode));
    memset(&segment_nav,0,sizeof(B_DVR_SegmentedFileNode));

    printf("   num  fileName           size          linearOffset        segOffset     type   refcnt\n");

    i=0;
    while(read(fd_ts,&segment_ts,sizeof(B_DVR_SegmentedFileNode))) {
        memset(tspath,0,MS_MAX_PATHLEN);
        strncpy(tspath,&segment_ts.fileName[3],6); /* get segment number */
        segnum = atoi(tspath);               
        printf("%6d: %s\t%12lu %16llx %16llx %8d %8d\n",i++,
               segment_ts.fileName,
               (unsigned long)segment_ts.size,
               segment_ts.linearStartOffset,
               segment_ts.segmentOffset,
               segment_ts.recordType,
               registry->mediaSegRefCount[segnum]);
        memset(&segment_ts,0,sizeof(B_DVR_SegmentedFileNode));
    }

    i=0;
    while (read(fd_nav,&segment_nav,sizeof(B_DVR_SegmentedFileNode))) {        
        memset(tspath,0,MS_MAX_PATHLEN);
        strncpy(tspath,&segment_nav.fileName[3],6); /* get segment number */
        segnum = atoi(tspath);         
        printf("%6d: %s\t%12lu %16llx %16llx %8d %8d\n",i++,
               segment_nav.fileName,
               (unsigned long)segment_nav.size,
               segment_nav.linearStartOffset,
               segment_nav.segmentOffset,
               segment_nav.recordType,
               registry->navSegRefCount[segnum]);
        memset(&segment_nav,0,sizeof(B_DVR_SegmentedFileNode));
    }
    close(fd_ts);
    close(fd_nav);
    
    printf("------------------------------------------------------------------------------\n");
    printf("    mediaAttributes:           %08x\n",media->mediaAttributes); 
    printf("    transportType:             %d\n",media->transportType);
    printf("    mediaFileName:             %s\n",media->mediaFileName);
    printf("    navFileName:               %s\n",media->navFileName);
    printf("    mediaNodeFileName:         %s\n",media->mediaNodeFileName);
    printf("    mediaNodeSubDir:           %s\n",media->mediaNodeSubDir);
    printf("    programName:               %s\n",media->programName);
    printf("    maxStreamBitRate:          %u\n",media->maxStreamBitRate);
    printf("    mediaStartTime:            %lu\n",media->mediaStartTime);
    printf("    mediaEndTime:              %lu\n",media->mediaEndTime);
    printf("    mediaLinearStartOffset:    %llu\n",(unsigned long long)media->mediaLinearStartOffset);
    printf("    mediaLinearEndOffset:      %llu\n",(unsigned long long)media->mediaLinearEndOffset);
    printf("    navLinearStartOffset:      %llu\n",(unsigned long long)media->navLinearStartOffset);
    printf("    navLinearEndOffset:        %llu\n",(unsigned long long)media->navLinearEndOffset);
    printf("    recording:                 %d\n",media->recording);
    printf("    drmServiceType:            %d\n",media->drmServiceType);
    printf("    drmServiceKeyType:         %d\n",media->drmServiceKeyType);
    printf("    esStreamCount:             %u\n",media->esStreamCount);
    for(i=0;i<(int)media->esStreamCount;i++) {
       if (media->esStreamInfo[0].pidType==eB_DVR_PidTypeVideo) {
          printf("      stream #%d: profile = %u\n",i, media->esStreamInfo[i].profile);   
          printf("      stream #%d: level = %u\n",i, media->esStreamInfo[i].level);
          printf("      stream #%d: frame rate = %u\n",i, media->esStreamInfo[i].videoFrameRate);
          printf("      stream #%d: video height = %u\n",i, media->esStreamInfo[i].videoHeight);
          printf("      stream #%d: video width = %u\n",i, media->esStreamInfo[i].videoWidth);
       } else {
          printf("      stream #%d: audio channel count = %u\n",i, media->esStreamInfo[i].audioChannelCount);
          printf("      stream #%d: audio sample size = %u\n",i, media->esStreamInfo[i].audioSampleSize);
          printf("      stream #%d: audio sample rate = %u\n",i, media->esStreamInfo[i].audioSampleRate);
       }
    }
    printf("\n");
        
err_media:
    free(media);
err_registry:
    free(registry);
}

void dump_dir(const char *path)
{
    DIR *pdir;    
    struct dirent *pent;
    char dir[MS_MAX_PATHLEN];;

    pdir = opendir(path);
    if (!pdir) {
        fprintf(stderr,"%s opendir error:%s\n", path, strerror(errno));
        return;
    }

    while ((pent = readdir(pdir))) {
        if (pent->d_type==DT_DIR && pent->d_name[0] != '.' && strncmp(pent->d_name,"lost+found",sizeof("lost+fount"))) {
            snprintf(dir,MS_MAX_PATHLEN,"%s/%s",path,pent->d_name);
            printf("    - subdir start %s\n",pent->d_name);
            dump_dir(dir);
            printf("    - subdir end   %s\n",pent->d_name);
        } else if (!fnmatch("*.info", pent->d_name, 0)) {	/* found */
            dump_file(path,pent->d_name);
        }                
    }
     
    closedir(pdir);
}
void dump_mount(const char *mountname)
{
    char path[MS_MAX_PATHLEN];

    printf("- file list\n");

    snprintf(path,MS_MAX_PATHLEN,"%s-metadata",mountname);

    dump_dir(path);

    return;
}    
void ms_dump_volume_status(const char *mountname)
{
    DIR *pdir;
    struct dirent *pent;    
    MS_Registry_t *registry;
    MS_DirQuota_t dirQuota;
    struct stat buf_stat;
    char path[MS_MAX_PATHLEN];
    char dirPath[MS_MAX_PATHLEN];
    int mediaCountDepot,mediaCountRecord,mediaCountPrealloc;
    int navCountDepot,navCountRecord;
    int ret;
    int i;
    uint32_t totalExpectedSeg;

    /* print registry */
    registry = malloc(sizeof(MS_Registry_t));
    if (!registry) {
        PRINT_ERR("malloc failed\n");
        return;
    }

    ms_read_registry(mountname,registry);
    printf("- media segment size : %u\n", registry->mediaSegmentSize);

    /* print number of segment in each folder */
    printf("- number of media segments\n");
    snprintf(path,MS_MAX_PATHLEN, "%s-media/depot",mountname);
    mediaCountDepot = count_segments(path, eMS_MediaSegment);
    printf("    depot        : %d\n", mediaCountDepot);
    snprintf(path,MS_MAX_PATHLEN, "%s-media/record",mountname);
    mediaCountRecord= count_segments(path, eMS_MediaSegment);
    printf("    record       : %d\n", mediaCountRecord);
    snprintf(path,MS_MAX_PATHLEN, "%s-media/preallocated",mountname);
    mediaCountPrealloc= count_segments(path, eMS_MediaSegment);
    printf("    preallocated : %d\n", mediaCountPrealloc);

    totalExpectedSeg = get_total_media_segments(path);

    printf("    total segments/expected = %d/%lu\n",
            mediaCountDepot+mediaCountRecord+mediaCountPrealloc,(unsigned long)totalExpectedSeg);

    printf("- number of nav segments\n");
    snprintf(path,MS_MAX_PATHLEN, "%s-nav/depot",mountname);
    navCountDepot = count_segments(path, eMS_NavSegment);
    printf("    depot        : %d\n", navCountDepot);
    snprintf(path,MS_MAX_PATHLEN, "%s-nav/record",mountname);
    navCountRecord = count_segments(path, eMS_NavSegment);
    printf("    record       : %d\n", navCountRecord);    

    printf("    total segments/expected = %d/%lu\n",
            navCountDepot+navCountRecord,(unsigned long)totalExpectedSeg);


    /* print sub registries for all folders */
    printf("- dir quota status\n");
    snprintf(path,MS_MAX_PATHLEN, "%s-metadata",mountname);
    pdir = opendir(path);
    while ((pent = readdir(pdir))) {
        if (pent->d_type == DT_DIR) {
            snprintf(dirPath,MS_MAX_PATHLEN,"%s/%s/%s",path,pent->d_name,MS_QUOTA_FILENAME);
            ret = stat(dirPath, &buf_stat);
            if (ret<0) {
                continue;   
            }
            ret = ms_read_quota(mountname,pent->d_name,&dirQuota);
            if (ret!=eMS_OK) {
                PRINT_ERR("ms_read_quota failed\n");
                continue;
            }
            printf("    %s quota  : %u\n", pent->d_name, dirQuota.quota);
            printf("    %s used   : %u\n", pent->d_name, dirQuota.used);
            printf("    %s shared : %u\n", pent->d_name, dirQuota.shared);
        }
    }

    closedir(pdir);

    printf("- media segment reference count\n");
    /* print reference counts for all segments(if exists) */
    for (i=0; i<MS_MAX_SEGMENTS;i++) {
        if (registry->mediaSegRefCount[i]) {
            printf("        seg%06d.ts %u\n",i, registry->mediaSegRefCount[i]);
        }
    }

    printf("- nav segment reference count\n");
    for (i=0; i<MS_MAX_SEGMENTS;i++) {
        if (registry->navSegRefCount[i]) {
            printf("        seg%06d.nav %u\n",i, registry->navSegRefCount[i]);
        }
    }
    free(registry);

    dump_mount(mountname);

}

int get_refcnt_number(char *segfile)
{
    char filename[10];
    unsigned segnum;
    
    snprintf(filename,7,"%s",segfile+3);
    segnum = (unsigned)atoi(filename);

    if (segnum<MS_MAX_SEGMENTS)
        return segnum;
    else 
        return -1;  /* invalid number */
}      

void check_totalnum_fill(const char *mountname, MS_SegmentType_t type, uint32_t totalExpectedSeg)
{
    char recordpath[MS_MAX_PATHLEN];
    char preallocpath[MS_MAX_PATHLEN];
    char depotpath[MS_MAX_PATHLEN];
    char filepath[MS_MAX_PATHLEN];
    char postfix[MS_MAX_PATHLEN];
    uint32_t num;
    struct stat buf_stat;
    unsigned long size;
    int fd;

    if (type == eMS_MediaSegment) {
        snprintf(recordpath,MS_MAX_PATHLEN,"%s-media/record",mountname);
        snprintf(preallocpath,MS_MAX_PATHLEN,"%s-media/preallocated",mountname);
        snprintf(depotpath,MS_MAX_PATHLEN,"%s-media/depot",mountname);
        snprintf(postfix,MS_MAX_PATHLEN,".ts");
    }
    else { 
        snprintf(recordpath,MS_MAX_PATHLEN,"%s-nav/record",mountname);
        snprintf(depotpath,MS_MAX_PATHLEN,"%s-nav/depot",mountname);
        snprintf(postfix, MS_MAX_PATHLEN,".nav");
    }

    for (num=0; num<totalExpectedSeg; num++) {
        /* check record, preallocaated and depot */
        snprintf(filepath,MS_MAX_PATHLEN,"%s/seg%06lu%s",recordpath,(unsigned long)num,postfix);
        if (stat(filepath,&buf_stat)==0) continue;

        if (type == eMS_MediaSegment) {
            snprintf(filepath,MS_MAX_PATHLEN,"%s/seg%06lu%s",preallocpath,(unsigned long)num,postfix);
            if (stat(filepath,&buf_stat)==0) continue;
        }            

        snprintf(filepath,MS_MAX_PATHLEN,"%s/seg%06lu%s",depotpath,(unsigned long)num,postfix);
        if (stat(filepath,&buf_stat)==0) continue;            

        /* segment missing. create one in depot */
        fd = open(filepath,O_CREAT | O_RDWR | O_CLOEXEC,0666);
        if (fd>=0) {
            size = (type==eMS_MediaSegment)?B_DVR_MEDIA_SEGMENT_SIZE:B_DVR_NAV_SEGMENT_SIZE;
#ifdef FALLOCATE_SUPPORT
        while (posix_fallocate(fd,(off_t)0,(off_t)size) != 0) {
            if (errno==EINTR) {
                PRINT_ERR(" %s pre-allocate returned EINTR: %s\n",filepath, strerror(errno));
                continue;
            } else {
                PRINT_ERR(" %s pre-allocate failed: %s\n",filepath, strerror(errno));
                break;
            }
        }
#else            
            if (ftruncate(fd,(off_t)size) < 0)
            {
                PRINT_ERR("%s pre-allocate failed",filepath);
            }
#endif            
        close(fd);
        } else {
            PRINT_ERR("segment file create failed\n");
        }

        PRINT_L1("# missing %s is created\n",filepath);
        gErrorCount++;
    }
}

int check_totalnum(const char *mountname, int recover)
{
    int ret = eMS_OK;
    char path[MS_MAX_PATHLEN];
    int mediaCountDepot,mediaCountRecord,mediaCountPrealloc;
    int navCountDepot,navCountRecord;
    uint32_t totalExpectedSeg, totalSeg;

    /* print number of segment in each folder */
    PRINT_L2("- number of media segments\n");

    snprintf(path,MS_MAX_PATHLEN, "%s-media/depot",mountname);
    mediaCountDepot = count_segments(path, eMS_MediaSegment);
    PRINT_L2("  depot        : %d\n", mediaCountDepot);

    snprintf(path,MS_MAX_PATHLEN, "%s-media/record",mountname);
    mediaCountRecord= count_segments(path, eMS_MediaSegment);
    PRINT_L2("  record       : %d\n", mediaCountRecord);

    snprintf(path,MS_MAX_PATHLEN, "%s-media/preallocated",mountname);
    mediaCountPrealloc= count_segments(path, eMS_MediaSegment);
    PRINT_L2("  preallocated : %d\n", mediaCountPrealloc);

    totalExpectedSeg = get_total_media_segments(path);
    totalSeg = (uint32_t)(mediaCountDepot+mediaCountRecord+mediaCountPrealloc);

    PRINT_L2("  total media segments/expected = %lu/%lu\n",
            (unsigned long)totalSeg,(unsigned long)totalExpectedSeg);    

    if (totalSeg<totalExpectedSeg) {
        /* refill missing media segments */
        if (recover) {
            check_totalnum_fill(mountname,eMS_MediaSegment,totalExpectedSeg);
        } else {
            PRINT_L1("# media segments are missing. total/expected = %u/%u\n", totalSeg,totalExpectedSeg);
            gErrorCount++;
        }
    } else if (totalSeg>totalExpectedSeg) {
        /* boxes formatted previously will have more segments but will not create a problem */
        PRINT_L1("    # total Segments/Expected Media Segment = %lu/%lu\n",(unsigned long)totalSeg,(unsigned long)totalExpectedSeg)
    }

    PRINT_L2("- number of nav segments\n");

    snprintf(path,MS_MAX_PATHLEN, "%s-nav/depot",mountname);
    navCountDepot = count_segments(path, eMS_NavSegment);
    PRINT_L2("  depot        : %d\n", navCountDepot);

    snprintf(path,MS_MAX_PATHLEN, "%s-nav/record",mountname);
    navCountRecord = count_segments(path, eMS_NavSegment);
    PRINT_L2("  record       : %d\n", navCountRecord);    

    totalSeg = navCountDepot+navCountRecord;

    PRINT_L2("  total nav segments/expected = %lu/%lu\n",
            (unsigned long)totalSeg,(unsigned long)totalExpectedSeg);

    if (totalSeg<totalExpectedSeg) {
        /* refill missing nav segments */
        if (recover) {
            check_totalnum_fill(mountname,eMS_NavSegment,totalExpectedSeg);
        } else {
            PRINT_L1("# nav segments are missing. total/expected = %u/%u\n", totalSeg,totalExpectedSeg);
            gErrorCount++;
        }
    } else if (totalSeg>totalExpectedSeg) {
       /* boxes formatted previously will have more segments but will not create a problem */
        PRINT_L1("    # total Segments/Expected Nav Segment = %lu/%lu\n",(unsigned long)totalSeg,(unsigned long)totalExpectedSeg)
    }

    return ret;
}

/* 
    get number of segments from a TS file node 
*/
uint16_t check_quotanum_dir_file(const char *mountname, const char *dirname, const char *filename)
{
    char path[MS_MAX_PATHLEN];
    B_DVR_SegmentedFileNode segment_ts;
    int fd;
    uint16_t numSegments=0;

    snprintf(path,MS_MAX_PATHLEN,"%s-metadata/%s/%s",mountname,dirname,filename);

    fd = open(path,O_RDONLY|O_CLOEXEC);
    if (fd<0) {
        PRINT_ERR("%s open failed\n",path);
        return 0;
    }
    
    while(read(fd,&segment_ts,sizeof(B_DVR_SegmentedFileNode))) {
        numSegments++;
    }

    close(fd);
    PRINT_DBG("%s/%s:%d============\n",dirname,filename,numSegments);
    return numSegments;
}

/*
    check number of segments in a directory
*/
int check_quotanum_dir(const char *mountname, const char *dirname)
{
    DIR *pdir;
    struct dirent *pent;
    int ret = eMS_OK;
    char path[MS_MAX_PATHLEN];
    uint16_t numSegments=0;
    MS_DirQuota_t dirQuota;
    
    snprintf(path, MS_MAX_PATHLEN,"%s-metadata/%s",mountname,dirname);
    pdir = opendir(path);
    while ((pent = readdir(pdir))) {
        if (!fnmatch("*ts", pent->d_name, 0)) {  /* ts node found */
            /* sum up number of segments from ts nodes in the directory */
            numSegments += check_quotanum_dir_file(mountname, dirname ,pent->d_name);
        }
    }
    closedir(pdir);

    ret = ms_read_quota(mountname,dirname,&dirQuota);
    if (ret!=eMS_OK) return ret;

    /* reset used number from actual count.*/
    if (dirQuota.used != numSegments) {
        dirQuota.used = numSegments;
        dirQuota.shared = 0;    /* reset to 0 */
        PRINT_L1("  # quota.used is not matching. set to %d\n",(int)dirQuota.used);
        gErrorCount++;
    }        

    ret = ms_update_quota(mountname,dirname,&dirQuota);
    if (ret!=eMS_OK) return ret;

    return ret;
}


int check_quotanum_move(char *srcpath, char *destpath, char *pSegment)
{
    DIR *pdir;
    struct dirent *pent;
    int ret = 0;

    pdir = opendir(srcpath);
    while ((pent = readdir(pdir))) {
        if (!fnmatch("seg*ts", pent->d_name, 0)) {  /* found */
            /* move to destination */
            snprintf(pSegment, MS_MAX_PATHLEN,"%s",pent->d_name);    /* give file name only */

            strncat(srcpath,pSegment,MS_MAX_PATHLEN-strlen(srcpath));
            strncat(destpath,pSegment,MS_MAX_PATHLEN-strlen(destpath));

            if(rename(srcpath,destpath)<0) {
                PRINT_ERR("%s->%s failed:%s\n",srcpath,destpath,strerror(errno));
                ret = eMS_ERR_SYSTEM;
                goto error_dir;
            }

            PRINT_L2("  moving %s -> %s\n", srcpath, destpath);   
            break;
        }
    }

error_dir:
    closedir(pdir);
    return ret;
}


/*
    b.  Quota 
        a. sum number of segments for the files in the dir
            1. update quota.used with it.
            2. if quota.used < sum of the numbers??? this should not happen
        i.  Sum of (Quota.reserved - quota.used ) for all dir with quota = segments in preallocated
        ii. If sum < preallocated
            1.  Move to depot
        iii.    If sum>preallocated
             1.  Move from depot to preallocated
*/
int check_quotanum(const char *mountname, int recover)
{
    int ret = eMS_OK;

    /* 
        check directories if it has quota set
        if set, read quota and sum quota.reserved-quota.used

        compare with preallocated.        
    */
    DIR *pdir;
    struct dirent *pent;    
    char path[MS_MAX_PATHLEN];
    char preallocPath[MS_MAX_PATHLEN];
    char depotPath[MS_MAX_PATHLEN];
    MS_DirQuota_t dirQuota;
    unsigned sumPrealloc=0; /* expected number of preallocated segments */
    unsigned countPrealloc; /* actual number of preallocated segmetns */
    int i, diff;


    /* count preallocatd segments */
    memset(path,0,MS_MAX_PATHLEN);
    snprintf(path,MS_MAX_PATHLEN,"%s-media/preallocated",mountname);
    countPrealloc = count_segments(path,eMS_MediaSegment);
    PRINT_L2("  countPrealloc=%u\n",countPrealloc);

    memset(path,0,MS_MAX_PATHLEN);
    snprintf(path,MS_MAX_PATHLEN,"%s-metadata",mountname);

    /* open metadata path */
    pdir = opendir(path);
    if (!pdir) {
        fprintf(stderr,"%s opendir error:%s\n", path, strerror(errno));
        ret = eMS_ERR_INVALID_PARAM;
        goto error;
    }
    
    PRINT_L2("- check segment files of recordings at %s\n", path);
    while ((pent = readdir(pdir))) {
        /* check directories */
        if (pent->d_type==DT_DIR && pent->d_name[0] != '.' && strncmp(pent->d_name,"lost+found",sizeof("lost+fount"))) {
            memset(&dirQuota,0,sizeof(MS_DirQuota_t));
            ret = ms_read_quota(mountname,pent->d_name,&dirQuota);
            if (ret != eMS_OK) continue; /* if quota is not set*/

            PRINT_L2("  %s quota is set to %d\n",path,dirQuota.quota)
            /* quota is set to this directory */
            PRINT_L2("  checking %s\n", pent->d_name);
            /* recover used quota */
            ret = check_quotanum_dir(mountname,pent->d_name);
            /* get expected number of segments in preallocated */
            ret = ms_read_quota(mountname,pent->d_name,&dirQuota);
            if (ret==eMS_OK) {
                sumPrealloc +=dirQuota.quota-dirQuota.used;
            }
        }
    }
    closedir(pdir);

    if (sumPrealloc!=countPrealloc) {
        PRINT_L1("  # expected number of preallocated segments : %u\n", sumPrealloc);
        PRINT_L1("  # total number of preallocated segments : %u\n", countPrealloc);
        gErrorCount++;
    }

    if (recover) {
        snprintf(preallocPath,MS_MAX_PATHLEN,"%s-media/preallocated/",mountname);
        snprintf(depotPath,MS_MAX_PATHLEN,"%s-media/depot/",mountname);
        if (sumPrealloc<countPrealloc) {    /* to many segments in preallocated */
            diff = countPrealloc-sumPrealloc;
            /* move segments from preallocated to depot*/
            PRINT_L1("  moving %d segments preallocated->depot\n",diff);
            for (i=0;i<diff;i++)
                check_quotanum_move(preallocPath,depotPath,path);
        } else if (sumPrealloc>countPrealloc) { /* more segments neeed in preallocated */
            diff = sumPrealloc-countPrealloc;
            /* move segments from depot to prealloc */
            PRINT_L1("  moving %d segments depot->preallocated\n",diff);
            for (i=0;i<diff;i++)
                check_quotanum_move(depotPath,preallocPath,path);
        }
    }
    
error:    
    return ret;
}

/*
    1.  Check reference count
        a.  Move any segments with refcnt 0 from record to depot
        b.  Clear reference count if segment file doesn't exist in record
    2.  Check record file set
        a.  Info, ts, nav
        b.  Deleting metadata files is enough. 
        c.  Segments will be left stale and moved to depot later 
    3.  Check recording
        a.  Check missing segment
            i.  Check if all the segments are in record for a recording(.ts, .nav)
            ii. If not, delete record file set
            iii.    Segments will be left  stale and moved to depot later
        b.  Move segments from record to 'safe'
    4.  Check numbers
        a.  Total segments
            i.  Segments in record,preallocated and depot should match with total number
            ii. If not, add more in depot or delete from depot
        b.  Quota 
            i.  Sum of (Quota.reserved - quota.used ) for all dir with quota = segments in preallocated
            ii. If sum < preallocated
                1.  Move to depot
            iii.    If sum>preallocated
                 1.  Move from depot to preallocated
*/

void fsck_logstart(const char *mountname)
{
    char path[MS_MAX_PATHLEN];    
    char pathbackup[MS_MAX_PATHLEN];    
    struct tm *local;
    time_t t;
    struct stat buf_stat;
    int fd;

    snprintf(path, MS_MAX_PATHLEN,"%s-metadata/%s",mountname,MS_MSFSCKLOG_FILENAME);
    snprintf(pathbackup, MS_MAX_PATHLEN,"%s-metadata/%s",mountname,MS_MSFSCKLOG_FILENAME_BACKUP);

    if (!stat(path,&buf_stat)) {    /* check size */
        PRINT_DBG("log size = %lu\n",(unsigned long)buf_stat.st_size);
        if (buf_stat.st_size>=MS_MAX_FSCKLOGSIZE) {
            unlink(pathbackup);
            link(path,pathbackup);
            unlink(path);
        }
    }

    gfpLog= fopen(path,"a");
    if (!gfpLog) {
        PRINT_ERR("open %s failed:%s",path,strerror(errno));
        return;
    }

    fd = fileno(gfpLog);
    if (fd== -1) {
        PRINT_ERR("fileno %s failed:%s",path,strerror(errno));
        fclose(gfpLog);
        gfpLog = NULL;  /* reset to NULL so that it can open next time */
        return;
    }
    
    if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1) {
        PRINT_ERR("fcntl %s failed:%s",path,strerror(errno));
        fclose(gfpLog);
        gfpLog = NULL;  /* reset to NULL so that it can open next time */
        return;
    }

    t = time(NULL);
    local = localtime(&t);
    fprintf(gfpLog,"\n--- volume check date: %s\n", asctime(local));
}

void fsck_logstop(void)
{
    fclose(gfpLog);
}

void ms_set_level(int level)
{
    switch (level) {
        case 1: 
            gLogLevel = 1;
            break;
        case 2:
            gLogLevel = 1+2;
            break;
        case 3:
            gLogLevel = 1+2+4;
            break;
        default:
            gLogLevel = 1;
            break;
    }
}

void delete_metafileset(const char *path,const char *name)
{
    char fullname[MS_MAX_PATHLEN];
    int ret;
    int count=0;

    PRINT_L1("# delete metadata file set for %s\n",name);
    
    snprintf(fullname,MS_MAX_PATHLEN,"%s/%s%s",path,name,MS_METAFILE_EXT_INFO);
    ret = unlink(fullname);
    if (ret==0) count++;
    snprintf(fullname,MS_MAX_PATHLEN,"%s/%s%s",path,name,MS_METAFILE_EXT_TS);
    ret = unlink(fullname);
    if (ret==0) count++;
    snprintf(fullname,MS_MAX_PATHLEN,"%s/%s%s",path,name,MS_METAFILE_EXT_NAV);
    ret = unlink(fullname);            
    if (ret==0) count++;

    if (count) {
        BDBG_ERR(("delete recording %s",name));
    gDeleteCount++;
}
}

/* 
    check .info, .ts, .nav metadata files.
    if given name is metdata file name, stat the all three. if any failes, delete.
    other than metadata file, just delete.
*/    
static void fileset_check_group(const char *path,const char *filename, int recover)
{
    int ret;
    struct stat buf_stat;
    int i;
    char name[MS_MAX_PATHLEN];  /* file name without extension */
    char fullname[MS_MAX_PATHLEN];  /* full path of a file */
    char *extension=0;            /* extension of input file */

    PRINT_FUNCIN();

    for (i=strlen(filename)-1;i>0;i--)
    {
        if (filename[i]=='.') {
            memset(name,0,MS_MAX_PATHLEN);
            snprintf(name,i+1,"%s",filename);
            extension = (char*)filename+i;
            break;
        }
    }

/*    PRINT_L2("- check %s\n",filename);  */

    PRINT_DBG("i=%d, filename:%s, name:%s, extension:%s\n",i, filename,name,extension);

    /* not a metadata file */
    if  ((extension == 0) ||
         (strncmp(extension,MS_METAFILE_EXT_INFO,strlen(extension)) && 
          strncmp(extension,MS_METAFILE_EXT_NAV,strlen(extension)) && 
          strncmp(extension,MS_METAFILE_EXT_TS,strlen(extension)))
        ) { 
        if(recover) {
           unlink(filename);   /* delete it */
           PRINT_L1("# %s deleted\n",filename);
           snprintf(fullname,MS_MAX_PATHLEN,"%s/%s",path,filename);
           unlink(fullname);
           gDeleteCount++;
        } else {
           PRINT_L1("# %s is not a metadata file. not deleted\n",filename);
        }
        gErrorCount++;
        return;
    } 

    /* stat 3 files */
    snprintf(fullname,MS_MAX_PATHLEN,"%s/%s%s",path,name,MS_METAFILE_EXT_INFO);    
    ret = stat(fullname, &buf_stat);
    if (ret<0) {
        PRINT_L1("# %s missing\n",fullname);
        gErrorCount++;
        goto error_set;
    }
    
    if (buf_stat.st_size < sizeof(B_DVR_MediaNode)) {
        PRINT_L1("# %s has invalid size %jd\n",fullname, buf_stat.st_size);
        BDBG_ERR(("%s has invalid size %jd\n",fullname, buf_stat.st_size));
        gErrorCount++;
        goto error_set;
    }
    
    snprintf(fullname,MS_MAX_PATHLEN,"%s/%s%s",path,name,MS_METAFILE_EXT_NAV);
    ret = stat(fullname, &buf_stat);
    if (ret<0) {
        PRINT_L1("# %s missing\n", fullname);
        gErrorCount++;
        goto error_set;
    }        

    if (buf_stat.st_size < sizeof(B_DVR_SegmentedFileNode)) {
        PRINT_L1("# %s has invalid size %jd\n",fullname, buf_stat.st_size);
        BDBG_ERR(("%s has invalid size %jd\n",fullname, buf_stat.st_size));
        gErrorCount++;
        goto error_set;
    }

    snprintf(fullname,MS_MAX_PATHLEN,"%s/%s%s",path,name,MS_METAFILE_EXT_TS);
    ret = stat(fullname, &buf_stat);
    if (ret<0) {
        PRINT_L1("# %s missing\n", fullname);
        gErrorCount++;
        goto error_set;
    }        

    if (buf_stat.st_size < sizeof(B_DVR_SegmentedFileNode)) {
        PRINT_L1("# %s has invalid size %jd\n",fullname, buf_stat.st_size);
        BDBG_ERR(("%s has invalid size %jd\n",fullname, buf_stat.st_size));
        gErrorCount++;
        goto error_set;
    }

    PRINT_FUNCOUT();
    return;
    
error_set:
    if(recover) {
       delete_metafileset(path,name);
    }
    PRINT_FUNCOUT();
}


/*
    1. check metadata files
        a. if any of metadata files among .info, .ts, .nav, delete rest of them.
*/
int fileset_check(const char *path, int recover)
{
    int ret = eMS_OK;
    DIR *pdir;
    struct dirent *pent;    
    char subdir[MS_MAX_PATHLEN];

    PRINT_FUNCIN();

    memset(subdir,0,MS_MAX_PATHLEN);

    pdir = opendir(path);
    if (!pdir) {
        PRINT_ERR("%s opendir error:%s\n", path, strerror(errno));
        return eMS_ERR_INVALID_PARAM;
    }

    while ((pent = readdir(pdir))) {
        if ( pent->d_type==DT_DIR) {
            if ((pent->d_name[0] != '.') && strncmp(pent->d_name,"lost+found",sizeof("lost+fount"))) {
                snprintf(subdir,MS_MAX_PATHLEN,"%s/%s",path,pent->d_name);
                PRINT_DBG("%s\n",subdir);
                fileset_check(subdir, recover);
            }
        } else  {
            if ( strncmp(pent->d_name,MS_REGISTRY_FILENAME,sizeof(MS_REGISTRY_FILENAME)) && 
                 strncmp(pent->d_name,MS_MSFSCKLOG_FILENAME,sizeof(MS_MSFSCKLOG_FILENAME)) &&
                 strncmp(pent->d_name,MS_MSFSCKLOG_FILENAME_BACKUP,sizeof(MS_MSFSCKLOG_FILENAME_BACKUP)) &&
                 strncmp(pent->d_name,MS_QUOTA_FILENAME,sizeof(MS_QUOTA_FILENAME))) {
                 fileset_check_group(path,pent->d_name, recover);
            }
        }
    }
    closedir(pdir);
    
    PRINT_FUNCOUT();
    return ret;
}

#if 0
/*
    count total number of recording by counting .info files
*/
int count_infofiles(const char *path)
{
    int ret = eMS_OK;
    DIR *pdir;
    struct dirent *pent;    
    char subdir[MS_MAX_PATHLEN];
    int i;
    char name[MS_MAX_PATHLEN];  /* file name without extension */
    char *extension=0;            /* extension of input file */

    PRINT_FUNCIN();

    memset(subdir,0,MS_MAX_PATHLEN);

    pdir = opendir(path);
    if (!pdir) {
        PRINT_ERR("%s opendir error:%s\n", path, strerror(errno));
        return eMS_ERR_INVALID_PARAM;
    }

    while ((pent = readdir(pdir))) {
        if ( pent->d_type==DT_DIR) {
            if ((pent->d_name[0] != '.') && strncmp(pent->d_name,"lost+found",sizeof("lost+fount"))) {
                snprintf(subdir,MS_MAX_PATHLEN,"%s/%s",path,pent->d_name);
                PRINT_DBG("%s\n",subdir);
                count_infofiles(subdir);
            }
        } else  {           
            for (i=strlen(pent->d_name)-1;i>0;i--)
            {
                if (pent->d_name[i]=='.') {
                    memset(name,0,MS_MAX_PATHLEN);
                    snprintf(name,i+1,"%s",pent->d_name);
                    extension = (char*)pent->d_name+i;
                    break;
                }
            }

            if(!memcmp_c(extension,"info",4)) { 
                gRecordingCount++;
                printf("%d: %s\n", gRecordingCount,name);
            }

        }
    }
    closedir(pdir);
    
    PRINT_FUNCOUT();
    return ret;
}
#endif

/*
    check mediafile name in medianode. if invalid, delete metadata files.
*/
int check_mediafilename(const char *path, const char *infofile, char *mediafile, int recover)
{
    char *extension;
    char filename[MS_MAX_PATHLEN];
    
    if (strncmp(infofile,mediafile,strlen(infofile)-strlen(MS_METAFILE_EXT_INFO))) { /* name is different */
        PRINT_L1("# wrong media file name %s:%s\n", infofile,mediafile);
        goto error;
    }

    extension = mediafile+strlen(mediafile)-strlen(MS_METAFILE_EXT_TS);
    PRINT_DBG("extension:%s\n",extension);
    if (strncmp(extension,MS_METAFILE_EXT_TS,strlen(MS_METAFILE_EXT_TS))) {
        PRINT_L1("# wrong extension %s\n", extension);
        goto error;
    }

    return eMS_OK;

error:
    if(recover) {
        memset(filename,0,MS_MAX_PATHLEN);
        snprintf(filename,strlen(infofile)-strlen(MS_METAFILE_EXT_INFO)+1,"%s",infofile);
        delete_metafileset(path,filename);
    }
    return eMS_ERR_INVALID_PARAM;
}

/*
    check navfile name in medianode. if invalid, delete metadata files.
*/
int check_navfilename(const char *path, const char *infofile, char *navfile, int recover)
{
    char *extension;
    char filename[MS_MAX_PATHLEN];
    
    if (strncmp(infofile,navfile,strlen(infofile)-strlen(MS_METAFILE_EXT_INFO))) { /* name is different */
        PRINT_L1("# wrong nav file name %s:%s\n", infofile,navfile);
        goto error;
    }

    extension = navfile+strlen(navfile)-strlen(MS_METAFILE_EXT_NAV);
    PRINT_DBG("extension:%s\n",extension);
    if (strncmp(extension,MS_METAFILE_EXT_NAV,strlen(MS_METAFILE_EXT_NAV))) {
        PRINT_L1("# wrong extension %s\n", extension);
        goto error;
    }

    return eMS_OK;

error:
    if(recover) {
        memset(filename,0,MS_MAX_PATHLEN);
        snprintf(filename,strlen(infofile)-strlen(MS_METAFILE_EXT_INFO)+1,"%s",infofile);
        delete_metafileset(path,filename);
    }
    return eMS_ERR_INVALID_PARAM;
}

int refcnt_build_file(const char *path, const char *infofile, int recover, MS_Registry_t *registry)
{
    char metadatafilePath[MS_MAX_PATHLEN];    /* .ts file path or <media partition>/record */
    B_DVR_SegmentedFileNode segmentNode;
    B_DVR_Media *media;
    int segnum;
    int readSize;
    int ret;
    int fd;

    PRINT_DBG("path:%s, infofile:%s\n",path,infofile);

    media = malloc(sizeof(B_DVR_Media));
    if (!media) {
        PRINT_ERR("malloc failed\n");
        return eMS_ERR_SYSTEM;
    }

    /* read media node */
    memset(metadatafilePath,0,MS_MAX_PATHLEN);
    snprintf(metadatafilePath,MS_MAX_PATHLEN,"%s/%s",path,infofile);
    fd = open(metadatafilePath,O_RDONLY|O_CLOEXEC);
    if (fd<0) {
        PRINT_ERR("open failed %s:%s\n",metadatafilePath,strerror(errno));
        free(media);
        return eMS_ERR_SYSTEM;
    }

    memset(media,0,sizeof(B_DVR_Media));
    readSize = read(fd,media,sizeof(B_DVR_Media));
    close(fd);
    if (readSize<0) {
        PRINT_ERR("read failed %s:%s\n",metadatafilePath,strerror(errno));
        free(media);
        return eMS_ERR_SYSTEM;
    } 

    /*  check mediafile name in medianode. if invalid, delete metadata files.  */
    ret = check_mediafilename(path,infofile,media->mediaFileName, recover);
    if (ret<0) {
        PRINT_DBG("check_mediafilename failed (%s:%d)\n",media->mediaFileName,ret);
        free(media);
        return ret;
    }

    /*  check navfile name in medianode. if invalid, delete metadata files.  */
     ret = check_navfilename(path,infofile,media->navFileName, recover);
    if (ret<0) {
        PRINT_DBG("check_navfilename failed (%s:%d)n",media->mediaFileName,ret);
        free(media);
        return ret;
    }    
    
    /* build reference count for ts segments*/
    memset(metadatafilePath,0,MS_MAX_PATHLEN);
    snprintf(metadatafilePath,MS_MAX_PATHLEN,"%s/%s",path,media->mediaFileName);
    fd = open(metadatafilePath,O_RDONLY|O_CLOEXEC);
    if (fd<0) {
        PRINT_ERR("open failed %s:%s\n",metadatafilePath,strerror(errno));
        free(media);
        return eMS_ERR_SYSTEM;
    }
    while((readSize = read(fd,&segmentNode,sizeof(B_DVR_SegmentedFileNode)))) {
        if (readSize<0) {
            PRINT_ERR("read failed %s:%s\n",metadatafilePath,strerror(errno));
            close(fd);
            free(media);
            return eMS_ERR_SYSTEM;
        } 
        segnum = get_refcnt_number(segmentNode.fileName);
        if (segnum<0) {
            PRINT_ERR("invalide segment number %s:%d\n", segmentNode.fileName,segnum);
            continue;
        }
        registry->mediaSegRefCount[segnum]++;   
        PRINT_DBG("%s:refcnt %06d: %d\n",segmentNode.fileName, segnum, registry->mediaSegRefCount[segnum]);
    }
    close(fd);

    /* build reference count for nav segments*/
    memset(metadatafilePath,0,MS_MAX_PATHLEN);
    snprintf(metadatafilePath,MS_MAX_PATHLEN,"%s/%s",path,media->navFileName);
    fd= open(metadatafilePath,O_RDONLY|O_CLOEXEC);
    if (fd<0) {
        PRINT_ERR("open failed %s:%s\n",metadatafilePath,strerror(errno));
        free(media);
        return eMS_ERR_SYSTEM;
    }
    while((readSize = read(fd,&segmentNode,sizeof(B_DVR_SegmentedFileNode)))) {
        if (readSize<0) {
            PRINT_ERR("read failed %s:%s\n",metadatafilePath,strerror(errno));
            close(fd);
            free(media);
            return eMS_ERR_SYSTEM;
        } 
        segnum = get_refcnt_number(segmentNode.fileName);
        if (segnum<0) {
            PRINT_ERR("invalide segment number %s:%d\n", segmentNode.fileName,segnum);
            continue;
        }
        registry->navSegRefCount[segnum]++;   
        PRINT_DBG("%s:refcnt %06d: %d\n",segmentNode.fileName, segnum, registry->navSegRefCount[segnum]);
    }
    close(fd);
    free(media);
    return eMS_OK;
}


/*
    1. build reference count from the metadata files
        a. if any segment in /record with refcnt 0, move to depot
*/
int refcnt_build(const char *path, int recover, MS_Registry_t *registry)
{
    int ret = eMS_OK;
    DIR *pdir;
    struct dirent *pent;    
    char subdir[MS_MAX_PATHLEN];

    PRINT_FUNCIN();

    memset(subdir,0,MS_MAX_PATHLEN);

    pdir = opendir(path);
    if (!pdir) {
        fprintf(stderr,"%s opendir error:%s\n", path, strerror(errno));
        return eMS_ERR_INVALID_PARAM;
    }

    PRINT_L2("- check segment files of recordings at %s\n", path);
    while ((pent = readdir(pdir))) {
        if ( pent->d_type==DT_DIR) {
            if ((pent->d_name[0] != '.') && strncmp(pent->d_name,"lost+found",sizeof("lost+fount"))) {
                snprintf(subdir,MS_MAX_PATHLEN,"%s/%s",path, pent->d_name);
                ret = refcnt_build(subdir, recover,registry);    /* check subdir */
                if (ret!=eMS_OK) goto error;
            }
        } else  {
            if (!fnmatch("*.info", pent->d_name, 0)) {	/* found */
                PRINT_L2("- check %s/%s\n", path,pent->d_name);
                ret = refcnt_build_file(path, pent->d_name, recover, registry);
                if (ret!=eMS_OK) goto error;
            }                
        }
    }
error:
    closedir(pdir);    
    PRINT_FUNCOUT();
    return ret;
}

/*
    check segment files in .ts and .nav
    if any missing, delete metadata files
*/
int segment_check_file(const char *path, const char *infofile, int recover)
{
    char metadatafilePath[MS_MAX_PATHLEN];     /* metadata file path */
    char recordPath[MS_MAX_PATHLEN];           /* record dir path */
    char filepath[MS_MAX_PATHLEN];      /* segment file path */
    B_DVR_SegmentedFileNode segmentNode;
    B_DVR_Media *media;
    int readSize;
    int ret;
    int fd;
    struct stat buf_stat;

    PRINT_FUNCIN();
    PRINT_DBG("path:%s, infofile:%s\n",path,infofile);

    PRINT_L2("- check %s\n",infofile);
    /* read media node */
    media = malloc(sizeof(B_DVR_Media));
    if (!media) {
        PRINT_ERR("malloc failed\n");
        return eMS_ERR_SYSTEM;
    }

    memset(metadatafilePath,0,MS_MAX_PATHLEN);
    snprintf(metadatafilePath,MS_MAX_PATHLEN,"%s/%s",path,infofile);
    fd = open(metadatafilePath,O_RDONLY|O_CLOEXEC);
    if (fd<0) {
        PRINT_ERR("open failed %s:%s\n",metadatafilePath,strerror(errno));
        free(media);
        return eMS_ERR_SYSTEM;
    }

    memset(media,0,sizeof(B_DVR_Media));
    readSize = read(fd,media,sizeof(B_DVR_Media));
    close(fd);
    if (readSize<0) {
        PRINT_ERR("read failed %s:%s\n",metadatafilePath,strerror(errno));
        free(media);
        return eMS_ERR_SYSTEM;
    } 

    /*  check mediafile name in medianode. if invalid, delete metadata files.  */
    ret = check_mediafilename(path,infofile,media->mediaFileName, recover);
    if (ret<0) {
        PRINT_DBG("check_mediafilename failed (%s:%d)\n",media->mediaFileName,ret);
        free(media);
        return ret;
    }

    /*  check navfile name in medianode. if invalid, delete metadata files.  */
     ret = check_navfilename(path,infofile,media->navFileName, recover);
    if (ret<0) {
        PRINT_DBG("check_navfilename failed (%s:%d)n",media->mediaFileName,ret);
        free(media);
        return ret;
    }    
    
    /* check if ts segments exists in media partition /record*/
    memset(recordPath,0,MS_MAX_PATHLEN);
    snprintf(recordPath,MS_MAX_PATHLEN,"%s",path);
    snprintf(recordPath+strlen("/mnt/dvrX"),MS_MAX_PATHLEN-strlen("/mnt/dvrX"),"-media/record");
    
    memset(metadatafilePath,0,MS_MAX_PATHLEN);
    snprintf(metadatafilePath,MS_MAX_PATHLEN,"%s/%s",path,media->mediaFileName);
    fd = open(metadatafilePath,O_RDONLY|O_CLOEXEC);
    if (fd<0) {
        PRINT_ERR("open failed %s:%s\n",metadatafilePath,strerror(errno));
        free(media);
        return eMS_ERR_SYSTEM;
    }
    while((readSize = read(fd,&segmentNode,sizeof(B_DVR_SegmentedFileNode)))) {
        if (readSize<0) {
            PRINT_ERR("read failed %s:%s\n",metadatafilePath,strerror(errno));
            close(fd);
            free(media);
            return eMS_ERR_SYSTEM;
        } 
        snprintf(filepath,MS_MAX_PATHLEN,"%s/%s",recordPath,segmentNode.fileName);
        PRINT_DBG("filepath:%s\n",filepath);
        if (stat(filepath,&buf_stat)<0) {
            PRINT_L1("# %s is missing\n", filepath);
            close(fd);
            goto segerror;
        }
    }
    close(fd); /* end of .ts file check */

    /* check if nav segments exists in media partition /record*/
    memset(recordPath,0,MS_MAX_PATHLEN);
    snprintf(recordPath,MS_MAX_PATHLEN,"%s",path);
    snprintf(recordPath+strlen("/mnt/dvrX"),MS_MAX_PATHLEN-strlen("/mnt/dvrX"),"-nav/record");    

    memset(metadatafilePath,0,MS_MAX_PATHLEN);
    snprintf(metadatafilePath,MS_MAX_PATHLEN,"%s/%s",path,media->navFileName);
    fd= open(metadatafilePath,O_RDONLY|O_CLOEXEC);
    if (fd<0) {
        PRINT_ERR("open failed %s:%s\n",metadatafilePath,strerror(errno));
        free(media);
        return eMS_ERR_SYSTEM;
    }
    while((readSize = read(fd,&segmentNode,sizeof(B_DVR_SegmentedFileNode)))) {
        if (readSize<0) {
            PRINT_ERR("read failed %s:%s\n",metadatafilePath,strerror(errno));
            close(fd);
            free(media);
            return eMS_ERR_SYSTEM;
        } 
        snprintf(filepath,MS_MAX_PATHLEN,"%s/%s",recordPath,segmentNode.fileName);
        PRINT_DBG("filepath:%s\n",filepath);
        if (stat(filepath,&buf_stat)<0) {
            PRINT_L1("# %s is missing\n", filepath);
            close(fd);
            goto segerror;
        }
    }
    close(fd); /* end of .nav file check */
    free(media);
    return eMS_OK;
    
segerror:
    if(recover) {
        memset(filepath,0,MS_MAX_PATHLEN);
        snprintf(filepath,strlen(infofile)-strlen(MS_METAFILE_EXT_INFO)+1,"%s",infofile);
        delete_metafileset(path,filepath);
    }
    gErrorCount++;
    free(media);
    PRINT_FUNCOUT();
    return eMS_OK;
}


/*
    2. check if segments exist in /record for a recording.
        a. if not, delete recording(.info,ts,nav)
*/
int segment_check(const char *path, int recover)
{
    int ret = eMS_OK;
    DIR *pdir;
    struct dirent *pent;    
    char subdir[MS_MAX_PATHLEN];

    PRINT_FUNCIN();

    memset(subdir,0,MS_MAX_PATHLEN);

    pdir = opendir(path);
    if (!pdir) {
        fprintf(stderr,"%s opendir error:%s\n", path, strerror(errno));
        return eMS_ERR_INVALID_PARAM;        
    }

    while ((pent = readdir(pdir))) {
        PRINT_DBG("pent->d_name:%s\n",pent->d_name);
        if ( pent->d_type==DT_DIR) {
            if ((pent->d_name[0] != '.') && strncmp(pent->d_name,"lost+found",sizeof("lost+fount"))) {
                snprintf(subdir,MS_MAX_PATHLEN,"%s/%s",path, pent->d_name);
                ret = segment_check(subdir,recover);    /* check subdir */
                if (ret!=eMS_OK) goto error;
            }
        } else  {
            if (!fnmatch("*.info", pent->d_name, 0)) {	/* found */
                ret = segment_check_file(path, pent->d_name, recover);
                if (ret!=eMS_OK) goto error;
            }                
        }
    }

error:
    closedir(pdir);    
    PRINT_FUNCOUT();
    return ret;
}

int staleseg_media(const char *mountname, MS_Registry_t *registry, int recover)
{
    int ret = eMS_OK;
    DIR *pdir;
    struct dirent *pent;    
    char recorddir[MS_MAX_PATHLEN];
    char depotdir[MS_MAX_PATHLEN];
    char srcpath[MS_MAX_PATHLEN];
    char destpath[MS_MAX_PATHLEN];
    int segnum;

    PRINT_FUNCIN();

    memset(recorddir,0,MS_MAX_PATHLEN);
    memset(depotdir,0,MS_MAX_PATHLEN);

    snprintf(recorddir,MS_MAX_PATHLEN,"%s-media/record",mountname);
    snprintf(depotdir,MS_MAX_PATHLEN,"%s-media/depot",mountname);

    pdir = opendir(recorddir);
    if (!pdir) {
        fprintf(stderr,"%s opendir error:%s\n", recorddir, strerror(errno));
        return eMS_ERR_INVALID_PARAM;        
    }

    while ((pent = readdir(pdir))) {
        PRINT_DBG("pent->d_name:%s\n",pent->d_name);
        if (!fnmatch("*.ts", pent->d_name, 0)) {	/* found */
/*            PRINT_L2("- check %s/%s\n", recorddir,pent->d_name);   */
            segnum = get_refcnt_number(pent->d_name);
            if (segnum<0) {
                PRINT_ERR("invalid segment number %s:%d\n", pent->d_name,segnum);
                continue;
            }
            if (registry->mediaSegRefCount[segnum]==0) { /* stale segment */
                if (recover) {
                    PRINT_L1("# move stale segment to depot: %s\n", pent->d_name);            
                    snprintf(srcpath,MS_MAX_PATHLEN,"%s/%s",recorddir,pent->d_name);
                    snprintf(destpath,MS_MAX_PATHLEN,"%s/%s",depotdir,pent->d_name);                
                    if(rename(srcpath,destpath)<0) {
                        PRINT_ERR("%s->%s failed:%s\n",srcpath,destpath,strerror(errno));
                        ret = eMS_ERR_SYSTEM;
                        goto error;
                    }
                } else {
                    PRINT_L1("# stale segment(refcnt 0) found: %s\n", pent->d_name);            
                }
                gErrorCount++;
            }
        }  /* fnmatch */              
    } /* while */

error:
    closedir(pdir);    
    PRINT_FUNCOUT();
    return ret;
}

int staleseg_nav(const char *mountname, MS_Registry_t *registry, int recover)
{
    int ret = eMS_OK;
    DIR *pdir;
    struct dirent *pent;    
    char recorddir[MS_MAX_PATHLEN];
    char depotdir[MS_MAX_PATHLEN];
    char srcpath[MS_MAX_PATHLEN];
    char destpath[MS_MAX_PATHLEN];
    int segnum;

    PRINT_FUNCIN();

    memset(recorddir,0,MS_MAX_PATHLEN);
    memset(depotdir,0,MS_MAX_PATHLEN);

    snprintf(recorddir,MS_MAX_PATHLEN,"%s-nav/record",mountname);
    snprintf(depotdir,MS_MAX_PATHLEN,"%s-nav/depot",mountname);

    pdir = opendir(recorddir);
    if (!pdir) {
        fprintf(stderr,"%s opendir error:%s\n", recorddir, strerror(errno));
        return eMS_ERR_INVALID_PARAM;
    }

    while ((pent = readdir(pdir))) {
        PRINT_DBG("pent->d_name:%s\n",pent->d_name);
        if (!fnmatch("*.nav", pent->d_name, 0)) {	/* found */
/*            PRINT_L2("- check %s/%s\n", recorddir,pent->d_name);   */
            segnum = get_refcnt_number(pent->d_name);
            if (segnum<0) {
                PRINT_ERR("invalide segment number %s:%d\n", pent->d_name,segnum);
                continue;
            }
            if (registry->navSegRefCount[segnum]==0) { /* stale segment */
                if (recover) {
                    PRINT_L1("# move stale segment to depot: %s\n", pent->d_name);            
                    snprintf(srcpath,MS_MAX_PATHLEN,"%s/%s",recorddir,pent->d_name);
                    snprintf(destpath,MS_MAX_PATHLEN,"%s/%s",depotdir,pent->d_name);                
                    if(rename(srcpath,destpath)<0) {
                        PRINT_ERR("%s->%s failed:%s\n",srcpath,destpath,strerror(errno));
                        ret = eMS_ERR_SYSTEM;
                        goto error;
                    }
                } else {
                    PRINT_L1("# stale segment(refcnt 0) found: %s\n", pent->d_name);            
                }
                gErrorCount++;
            }
        }  /* fnmatch */              
    } /* while */

error:
    closedir(pdir);    
    PRINT_FUNCOUT();
    return ret;
}

#ifndef NO_REGFILE
int ms_update_registry_noproxy(const char *mountname, MS_Registry_t *registry)
{
    int fd;
    char name[MS_MAX_PATHLEN];

    PRINT_FUNCIN();    
    snprintf(name, MS_MAX_PATHLEN, "%s-metadata/%s",mountname,MS_REGISTRY_FILENAME);
    PRINT_DBG("update registry %s\n",name);

    fd = open(name,O_WRONLY|O_CREAT|O_CLOEXEC,0666);
    if (fd<0) {
        PRINT_FUNCOUT();
        return eMS_ERR_SYSTEM;
    }

    write(fd, (void*)registry, sizeof(MS_Registry_t));
    fdatasync(fd);
    close(fd);
                        
    PRINT_FUNCOUT();
    return eMS_OK;
}
#endif

int ms_recover_volume(const char *mountname, int recover, int *pDeletedFileCount)
{
    int ret=eMS_OK;
    char path[MS_MAX_PATHLEN];
    MS_Registry_t *registry;    /* registry copy to be used when checking file segments */
    
    /*
        0. check metadata files
            a. if any of metadata files among .info, .ts, .nav, delete rest of them.
    */

    BDBG_ERR(("check %s",mountname));

    PRINT_FUNCIN();

    fsck_logstart(mountname);

    gErrorCount = 0;
    gDeleteCount = 0;

    registry = malloc(sizeof(MS_Registry_t));
    if (!registry) {
        PRINT_ERR("malloc failed\n");
        return eMS_ERR_SYSTEM;
    }
    
    memset(registry,0,sizeof(MS_Registry_t));
    registry->version = MS_VERSION;
    registry->mediaSegmentSize = MS_MEDIA_SEGMENT_SIZE;
    snprintf(path,MS_MAX_PATHLEN,"%s-media",mountname);
    registry->numberOfSegments = get_total_media_segments(path);

    snprintf(path,MS_MAX_PATHLEN,"%s-metadata",mountname);

    /*
        1. check if segments exist in /record for a recording.
            a. if not, delete recording(.info,ts,nav)
    */
    PRINT_L1("1. checking metadata file set\n");
    ret = fileset_check(path,recover);
    if (ret!=eMS_OK) {
        PRINT_ERR("metadata file check failed %d\n", ret);
        goto error;
    }
       
    /*  skip this: playback sanity check will cover this case.
        2. check if segments exist in /record for a recording.
           a. if not, delete recording(.info,ts,nav)

    PRINT_L1("2. checking segments of recordings\n");
    ret = segment_check(path,recover);
    if (ret!=eMS_OK) {
        PRINT_ERR("check segment failed %d\n", ret);
        goto error;
    }
    */

    /*
        3. build reference count from the metadata files
    */
    if (recover) {
        PRINT_L1("2. building reference counts\n");    
        ret = refcnt_build(path, recover, registry);
        if (ret!=eMS_OK) {
            PRINT_ERR("build reference count failed %d\n", ret);
            goto error;
        }   

#ifdef NO_REGFILE
        ret = ms_update_registry(mountname,registry);
#else
        ret = ms_update_registry_noproxy(mountname,registry);
#endif        
        if (ret<0) {
            PRINT_ERR("ms_registry_update failed %d\n",ret);
            goto error;
        }
    } else {
        PRINT_L1("2. skipped reference count rebuild\n")
        ms_read_registry(mountname,registry); /* use current registry instead */
    }

    
    /*
        4. if any segment's refcnt is not 0 at the end, move to depot    
    */
    PRINT_L1("3. checking stale media segments\n");    
    ret = staleseg_media(mountname, registry, recover);
    if (ret!=eMS_OK) {
        PRINT_ERR("staleseg_media failed %d\n", ret);
        goto error;
    }

    PRINT_L1("4. checking stale nav segments\n");    
    ret = staleseg_nav(mountname, registry, recover);
    if (ret!=eMS_OK) {
        PRINT_ERR("staleseg_media failed %d\n", ret);
        goto error;
    }

    /*
        a.  Total segments
            i.  Segments in record,preallocated and depot should match with total number
            ii. If not, add more in depot or delete from depot       
    */            
    PRINT_L1("5. checking number of segments\n");    
    ret = check_totalnum(mountname, recover);
    if (ret!=eMS_OK) goto error;

    /*
        b.  Quota 
            i.  Sum of (Quota.reserved - quota.used ) for all dir with quota = segments in preallocated
            ii. If sum < preallocated
                1.  Move to depot
            iii.    If sum>preallocated
                 1.  Move from depot to preallocated
    */
    PRINT_L1("6. checking quota\n");    
    ret = check_quotanum(mountname, recover);
    if (ret!=eMS_OK) goto error;


    if(gErrorCount) {
        if (recover) {
            PRINT_L1("- Fixed %d errors in %s\n", gErrorCount,mountname);           
        } else {
            PRINT_L1("- Found %d errors in %s\n", gErrorCount,mountname);           
        }
    } else {
        PRINT_L1("- No error found in %s\n", mountname);
    }

    if(gDeleteCount) {
        PRINT_L1("- Deleted %d recordings in %s\n", gDeleteCount,mountname);           
        *pDeletedFileCount = gDeleteCount;
    }

error:
    free(registry);
    fsck_logstop();
    PRINT_FUNCOUT();
    return ret;
}

int ms_check_registry(const char *mountname)
{
    MS_Registry_t *registryBuild;    
    MS_Registry_t *registryRun;    
    char path[MS_MAX_PATHLEN];
    int ret;
    uint32_t i;

    registryBuild= malloc(sizeof(MS_Registry_t));
    if (!registryBuild) {
        PRINT_ERR("malloc failed\n");
        return eMS_ERR_SYSTEM;
    }

    registryRun= malloc(sizeof(MS_Registry_t));
    if (!registryRun) {
        PRINT_ERR("malloc failed\n");
        free(registryBuild);
        return eMS_ERR_SYSTEM;
    }

    memset(registryBuild,0,sizeof(MS_Registry_t));
    memset(registryRun,0,sizeof(MS_Registry_t));

    ret = ms_read_registry(mountname,registryRun); 
    if (ret!=eMS_OK) {
        goto error;
    }

    registryBuild->version = registryRun->version;
    registryBuild->mediaSegmentSize = registryRun->mediaSegmentSize;
    registryBuild->numberOfSegments = registryRun->numberOfSegments;
    
    fsck_logstart(mountname);

    snprintf(path,MS_MAX_PATHLEN,"%s-metadata",mountname);
    ret = refcnt_build(path, 0, registryBuild);
    if (ret!=eMS_OK) { 
        goto error;
    }

    ret = memcmp(registryBuild,registryRun,sizeof(MS_Registry_t));
    if(ret!=0) {
        PRINT_L1("--- refchk: corrupted! %d\n",ret);
/*        dump_refcnt(mountname,1,1); */
        for (i=0; i<registryRun->numberOfSegments;i++) {
            if (registryBuild->mediaSegRefCount[i]!=registryRun->mediaSegRefCount[i]) {
                PRINT_L1("seg%06d.ts : expected %d, actual %d\n",i,registryBuild->mediaSegRefCount[i],registryRun->mediaSegRefCount[i]);
            }
            if (registryBuild->navSegRefCount[i]!=registryRun->navSegRefCount[i]) {
                PRINT_L1("seg%06d.nav: expected %d, actual %d\n",i,registryBuild->navSegRefCount[i],registryRun->navSegRefCount[i]);
            }            
        }
        ret = eMS_ERR_INVALID_VOLUME;
    } else {
        PRINT_L1("--- refchk: good\n");
#if 0
        for (i=0; i<registryRun->numberOfSegments;i++) {
            PRINT_L2("%02d ",registryRun->mediaSegRefCount[i]);
        }
        PRINT_L2("\n");
        for (i=0; i<registryRun->numberOfSegments;i++) {
            PRINT_L2("%02d ",registryRun->navSegRefCount[i]);
        }
        PRINT_L2("\n");
#endif        
    }

error:    
    fsck_logstop();
    free(registryBuild);
    free(registryRun);
    return ret;
}


