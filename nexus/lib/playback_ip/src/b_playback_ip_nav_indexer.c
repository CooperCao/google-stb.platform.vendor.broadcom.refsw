/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 *  ANY LIMITED REMEDY.
 *
 * Module Description: wrapper module for NAV indexer
 *
 *************************************************************/
#define _FILE_OFFSET_BITS 64
#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "bcmplayer.h"
#include "b_playback_ip_nav_indexer.h"
#include "b_playback_ip_utils.h"
#include "nexus_types.h"
#include "tsindexer.h"
#include "bcmindexer.h"
#include "bcmindexer_nav.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

BDBG_MODULE(b_playback_ip_nav_indexer);
/*---------------------------------------------------------------
- PRIVATE DEFINITIONS AND GLOBALS
---------------------------------------------------------------*/
#ifdef BRCM_DBG
    #define PLAY_DBGMSG(x) { printf("%s:%d-",__FUNCTION__,__LINE__); printf x ; }
#else
    #define PLAY_DBGMSG(x) ((void)0)
#endif

#ifndef MIN
    #define MIN(a,b) ((a)<(b) ? (a) : (b))  /* min macro */
#endif

static long bplay_read(void *buffer, long size, long count, void *fp )
{
    return fread(buffer,size,count,fp);
}

static int bplay_bounds(BNAV_Player_Handle handle, void *filePointer, long *firstIndex, long *lastIndex)
{
    FILE * fp = filePointer;
    long size, cur_pos;
    *firstIndex = -1;
    *lastIndex = -1;

    BSTD_UNUSED(handle);
    cur_pos = ftell( fp);
    if (cur_pos<0) {
        BDBG_ERR(("%s: ftell (1) failed to provide the offset of the current position: errno %d", __FUNCTION__, errno));
        cur_pos=0;
    }

    fseek( fp, 0 , SEEK_END);
    size = ftell(fp);
    if (size < 0) {
        BDBG_ERR(("%s: ftell (2) failed to provide the offset of the current position: errno %d", __FUNCTION__, errno));
        return -1;
    }

    if (fseeko( fp, cur_pos,  SEEK_SET) < 0) {
        BDBG_ERR(("%s: fseeko failed to seek to offset: errno %d", __FUNCTION__, errno));
        return -1;
    }
    *firstIndex = 0;
    *lastIndex = (size/sizeof(BNAV_Entry)) -1;

    return 0;
}

static long bplay_tell( void *fp )
{
    return ftell(fp);
}

static int bplay_seek( void *fp, long offset, int origin )
{
    return fseek(fp,offset,origin);
}

int  nav_indexer_open(
    void **context,
    FILE *fp,
    B_PlaybackIpPsiInfoHandle psi
    )
{
    BNAV_Player_Settings cfg;
    BNAV_DecoderFeatures features = {1,1,1,1};
    BNAV_Player_PlayMode playMode;
    long firstIndex, lastIndex;
    BNAV_Player_Handle h ;
    BNAV_Player_Position p0;

    BNAV_Player_GetDefaultSettings(&cfg);

    BDBG_MSG(("%s\n",__FUNCTION__));
    cfg.filePointer = fp;
    cfg.normalPlayBufferSize = STREAMING_BUF_SIZE;
    cfg.decoderFeatures = features;
    cfg.readCb = bplay_read;
    cfg.tellCb = bplay_tell;
    cfg.seekCb = bplay_seek;
    cfg.boundsCb = bplay_bounds;
    cfg.transportTimestampEnabled = psi->transportTimeStampEnabled;
    switch (psi->videoCodec)
    {
        case NEXUS_VideoCodec_eH264:
            cfg.navVersion = BNAV_Version_AVC;
            break;
        case NEXUS_VideoCodec_eH265:
            cfg.navVersion = BNAV_Version_HEVC;
            break;
        default:
            cfg.navVersion = BNAV_Version2;
            break;
    }
    cfg.skipSanityCheck = 1;
    cfg.videoPid = psi->videoPid;
    /* Note: ssood - 07/01/2014 */
    /* This flag enabled BCM Player to insert BTP packets in outgoing frames. */
    /* We had disabled this flag due to some interop testing but we need to re-enable the BTP flag */
    /* as we could have streams where i-frames dont begin & end on the clean TS packet boundaries and */
    /* this leads to partial frame data in the next TS packet. This partial data causes our client rave & decoder */
    /* to not able to extrac the correct PTS values from the stream. This becomes an issue when we use the simulated STC */
    /* for controlling the trickplay rate at the client as decoder uses the TSM logic to display frames at the correct rate. */
    /* so I am enabling BTP insertion. If this becomes an issue w/ some non-Broadcom client, then we will add an config option to disable it. */
    cfg.useBtpsForHostTrickModes = true;

    if (BNAV_Player_Open( (BNAV_Player_Handle *)context, &cfg)!=0)
    {
        BDBG_ERR(("BCM Player Creation Failed\n"));
        return -ENOMEM;
    }

    BDBG_MSG(("BNAV Player = %p\n",*context));

    BNAV_Player_GetSettings((BNAV_Player_Handle) *context, &cfg);
    playMode.playMode = eBpPlayNormal;
    playMode.playMode = eBpPlayNormalByFrames;
    playMode.playModeModifier = 1;
    /*playMode.loopMode = eBpLoopForever;*/
                /*eBpSinglePlay;*/
    playMode.loopMode = eBpSinglePlay;
    playMode.disableExtraBOptimization = 0;
    BNAV_Player_SetPlayMode((BNAV_Player_Handle) *context, &playMode);

    h = (BNAV_Player_Handle) *context;
    BNAV_Player_GetSettings(h, &cfg);
    BNAV_Player_DefaultGetBounds(h, cfg.filePointer, &firstIndex, &lastIndex);
    BNAV_Player_SetCurrentIndex(h, firstIndex);
    if (BNAV_Player_GetPosition(h, &p0)) {
        BDBG_WRN(("BNAV_Player_GetPosition failed ...\n"));
    }
    else
        psi->firstPts = p0.pts;
    return 0;
}

void nav_indexer_close(void *context)
{
    BNAV_Player_Handle h = (BNAV_Player_Handle) context;
    BNAV_Player_Close(h);
}

int nav_indexer_seek(void *context, unsigned long targetPts, unsigned long *duration, unsigned long *first_pts)
{
    BNAV_Player_Handle h = (BNAV_Player_Handle) context;
    BNAV_Player_Position p, p0,p1;
    BNAV_Player_Settings cfg;
#if 0
    /* see if these are needed ? */
    BNAV_Player_PlayEntry navEntry;
    BNAV_Player_PlayMode mode;
#endif
    long firstIndex, lastIndex,currentIndex;
    int rc=0;

    memset(&p0,0,sizeof(p0));

    BNAV_Player_GetSettings(h, &cfg);
    BNAV_Player_DefaultGetBounds(h, cfg.filePointer, &firstIndex, &lastIndex);
    BDBG_MSG(("firstIndex=%ld lastIndex=%ld",firstIndex,lastIndex));

    /* Get First PTS */
    BNAV_Player_SetCurrentIndex(h, firstIndex);
    while(1){

        rc = BNAV_Player_GetPosition(h, &p0);
        if(rc){
            BDBG_ERR(("BNAV_Player_GetPosition failed ...\n"));
            return rc;
        }

        BDBG_MSG(("Index=%ld FirstPTS = %lu\n",firstIndex, p0.pts));
        if(p0.pts != 0)
            break;

        if(firstIndex == lastIndex){
            BDBG_MSG(("No PTS found\n"));
            break;
        }
        firstIndex ++;

    }

    /* Get Last PTS */
    lastIndex &= ~1; /*Force to even number */
    BNAV_Player_SetCurrentIndex(h, lastIndex);
    rc = BNAV_Player_GetPosition(h, &p1);
    if(rc){
        BDBG_ERR(("BNAV_Player_GetPosition failed ...\n"));
        return rc;
    }

    printf("LastPTS = %lu\n",p1.pts);
    *duration = (p1.pts - p0.pts);
        *first_pts = p0.pts;

    if(targetPts && ((targetPts - p0.pts)>0) && ((p1.pts - targetPts)>0)){
        BDBG_MSG(("targetPts %lu, p0.pts %lu, p1.pts %lu\n", targetPts, p0.pts, p1.pts));
        currentIndex = 1.* lastIndex * (targetPts - p0.pts)/(p1.pts - p0.pts);
        currentIndex &= ~1;
    }
    else{
        BDBG_MSG(("in 2\n"));
        currentIndex = firstIndex;
    }

    /* TODO: look into current index calc above */
    BNAV_Player_SetCurrentIndex(h, currentIndex);
    rc = BNAV_Player_GetPosition(h, &p);
    if(rc){
        BDBG_ERR(("BNAV_Player_GetPosition failed ...\n"));
        return rc;
    }
    BDBG_MSG(("currentIndex=%ld, pts %lu\n",currentIndex, p.pts));

    return 0;
}

int nav_indexer_pts(void *context, index_entry_t *index)
{
    BNAV_Player_Handle h = (BNAV_Player_Handle) context;
    BNAV_Player_Position position;
    BNAV_Entry entry;
    BNAV_AVC_Entry avcEntry;
    int rc;
    BNAV_Player_Status status;
    BNAV_Player_Settings settings;

    rc = BNAV_Player_GetPosition(h, &position);

    if(rc){
        BDBG_ERR(("BNAV_Player_GetPosition failed ..."));
        return rc;
    }

    /* Seek to the proper offset in the stream */
    index->pts = position.pts;

    /* now get the frame type */
    BNAV_Player_GetStatus(h, &status);
    BNAV_Player_GetSettings(h, &settings);
    if (settings.navVersion == BNAV_Version_AVC ||
        settings.navVersion == BNAV_Version_HEVC) {
        rc = BNAV_Player_ReadAvcIndex(h, status.currentIndex, &avcEntry);
        if (rc) { BDBG_ERR(("BNAV_Player_ReadAvcIndex failed ...")); return -1;}
        index->type = BNAV_get_frameType(&avcEntry);
    }
    else {
        rc = BNAV_Player_ReadIndex(h, status.currentIndex, &entry);
        if (rc) { BDBG_ERR(("BNAV_Player_ReadIndex failed ...")); return -1;}
        index->type = BNAV_get_frameType(&entry);
    }
    return 0;
}

int nav_indexer_next(void *context, index_entry_t *index)
{
    BNAV_Player_Handle h = (BNAV_Player_Handle) context;
    BNAV_Player_PlayEntry navEntry;
    index_entry_t index2;

    int rc;
    memset(&navEntry, 0, sizeof(navEntry));
    rc = BNAV_Player_GetNextPlayEntry( h, &navEntry, index->pkt );
    if(rc) {
        BDBG_WRN(("BCMplayer returned nothing ...\n"));
        return -1;
    }
    /* Seek to the proper offset in the stream */
    index->offset =(((uint64_t)navEntry.startOffsetHi)<<32) | navEntry.startOffset;
    index->size = navEntry.byteCount;
    index->insertPkt = navEntry.isInsertedPacket;

    nav_indexer_pts(context, &index2);
    index->pts = index2.pts;
    index->pcr = index2.pts;
    index->type = index2.type;
    /*printf("NAV %lld %lu\n",index->offset,index->size);*/
    if (navEntry.zeroByteCountBegin || navEntry.zeroByteCountEnd)
        BDBG_MSG(("!!!!! zeroByteCountBegin %ul, zeroByteCountEnd %ul !!!!! ", (unsigned int)navEntry.zeroByteCountBegin, (unsigned int)navEntry.zeroByteCountEnd));
    return 0;
}

void nav_indexer_setIndexByByteOffset(void *context, uint64_t byteOffset)
{
    BNAV_Player_Handle h = (BNAV_Player_Handle) context;
    long currentIndex;

    currentIndex = BNAV_Player_FindIndexFromOffset(h, (unsigned int)(byteOffset>>32), (unsigned int)(byteOffset));
    BDBG_MSG(("%s: NAV: byteOffset %"PRId64 ", current index =%ld \n", __FUNCTION__, byteOffset, currentIndex));
    BNAV_Player_SetCurrentIndex(h, currentIndex);
}

int
nav_indexer_setIndexByTimeOffset(void *context, double timeOffset)
{
    BNAV_Player_Handle h = (BNAV_Player_Handle) context;
    index_entry_t index;
    unsigned long pts, first_pts, duration;

    nav_indexer_pts(h, &index);
    pts = (unsigned long) index.pts + (unsigned long) (timeOffset * 45000);
    BDBG_MSG(("%s: timeOffset %.3f, pts: 1st %lu, %lu", __FUNCTION__, timeOffset, index.pts, pts));
#if 1
    nav_indexer_seek(h, pts, &duration, &first_pts);
#else
    duration = timeOffset * 1000; /* convert to msec */
    if ( (currentIndex = BNAV_Player_FindIndexFromTimestamp(h, duration)) == -1) {
        BDBG_WRN(("Failed to find an index entry for npt %0.3f, msec %u", timeOffset, duration));
        return -1;
    }
    if (BNAV_Player_SetCurrentIndex(h, currentIndex)) {
        BDBG_ERR(("BNAV_Player_SetCurrentIndex failed to set index to %ld...", currentIndex));
        return -1;
    }
    if (BNAV_Player_GetPosition(h, &p)) {
        BDBG_ERR(("BNAV_Player_GetPosition failed ..."));
        return -1;
    }
    BDBG_MSG(("timeOffset %lld, msec %u, currentIndex=%ld, pts %lu",timeOffset, duration, currentIndex, p.pts));
#if 0
        nav_indexer_setIndexByTimeOffset: timeOffset 16.000, pts: 1st 27532821, 28252821
        timeOffset 4625196817309499392, msec 16000, currentIndex=484, pts 27897685
        setReadOffset: file offset 20632576 for entry offset 20632624 delta offset 48,
#endif
#endif
    return 0;
}

int  nav_indexer_rewind(void *context)
{
    BNAV_Player_Handle h = (BNAV_Player_Handle) context;
    BNAV_Player_PlayMode mode;
    BNAV_Player_Settings cfg;
    long firstIndex = -1, lastIndex = -1;
    int rc = -1;

    BNAV_Player_GetPlayMode(h, &mode);
    BNAV_Player_GetSettings(h, &cfg);
    if (BNAV_Player_DefaultGetBounds(h, cfg.filePointer, &firstIndex, &lastIndex) < 0) {
        BDBG_ERR(("%s: failed to get the nav file bounds, possible nav file corruption?", __FUNCTION__));
        rc = -1;
        goto out;
    }

    BDBG_WRN(("%s: Rewinding: firstIndex=%ld lastIndex=%ld",__FUNCTION__, firstIndex,lastIndex));

    if(mode.playModeModifier > 0 )
        rc = BNAV_Player_SetCurrentIndex(h, firstIndex);
    else
        rc = BNAV_Player_SetCurrentIndex(h, lastIndex);
out:
    return rc;
}


bool
nav_indexer_mode(void *context, trick_mode_t *tm)
{
    BNAV_Player_Handle h = (BNAV_Player_Handle) context;

    BNAV_Player_PlayMode mode;
    BDBG_MSG(("BCMPlayer ModeChange: Speed=%d DIR=%d, modeModifier %d\n",tm->speed, tm->direction, tm->modifier));
    mode.playModeModifier = 1;
    BNAV_Player_GetPlayMode(h, &mode);

    if (tm->videoType == NEXUS_VideoCodec_eH264 || tm->videoType == NEXUS_VideoCodec_eH265) {
        /* AVC Trickmodes (+ve speeds) */
        if(tm->direction>0){
            switch(tm->speed){
                case 1:
                case 2:
                    mode.playMode = eBpPlayNormal;
                    break;
                default:
                    mode.playMode = eBpPlayI;
                    mode.playModeModifier = tm->modifier;
                    break;
            }
        }
        else {
            /* AVC Trickmodes (-ve speeds) */
            switch(tm->speed){
                /* no -1x support for AVC, we just use the regular i-frame rewind */
            default:
                mode.playMode = eBpPlayI;
                mode.playModeModifier = -tm->modifier;
                break;

            }
        }
    }
    else {
            /* Mpeg2 Trickmodes (+ve speeds) */
            if(tm->direction> 0) {
                switch(tm->speed) {
                case 1:
                case 2:
                    /* use a more efficient block read-mode as it allows read by large fixed size blocks */
                    mode.playMode = eBpPlayNormal;
                    break;
                default:
                    mode.playMode = eBpPlayI;
                    mode.playModeModifier = tm->modifier;
                    break;
                }
            }
            else {
                /* Mpeg2 Trickmodes (-ve speeds) */
                switch(tm->speed) {
                default:
                    /* we are now just sending I-frames for all rewind speeds */
                    mode.playMode = eBpPlayI;
                    mode.playModeModifier = -tm->modifier;
                    break;
                }
            }
    }

    mode.disableExtraBOptimization = 0;

    if (BNAV_Player_SetPlayMode(h, &mode) < 0) {
        BDBG_WRN(("%s: Failed to set the play mode\n", __FUNCTION__));
        return false;
    }

    return true;
}

/**
* Feed from tsindexer to bcmindexer.
**/
static unsigned long
write_to_bcmindexer(
    const void *p_readBuf,
    unsigned long numEntries,
    unsigned long entrySize,
    void *fp
    )
{
    BSTD_UNUSED(entrySize);
    return BNAV_Indexer_Feed((BNAV_Indexer_Handle)fp, (void*)p_readBuf, numEntries);
}

static FILE *g_mpegFile = NULL; /* global hack */
int mpegSizeCallback(BNAV_Indexer_Handle handle, unsigned long *hi, unsigned long *lo)
{
#ifdef LINUX
    off_t o = ftello(g_mpegFile);
#else
    off_t o = ftell(g_mpegFile);
#endif
    BSTD_UNUSED(handle);
    if (o == -1)
        return -1;
    *hi = o >> 32;
    *lo = o & 0xFFFFFFFF;
    BDBG_MSG(("%s: offsets: hi %p, lo %p", __FUNCTION__, (void *)hi, (void *)lo));
    return 0;
}

#define MASTER_PLAYLIST_BUFFER_SIZE 1024
#define MEDIA_PLAYLIST_BUFFER_SIZE 1024
#define SEGMENT_START_SEQUENCE_NUMBER 0
#define HLS_VERSION 3 /* support version 3 for now! */
#define HLS_SEGMENT_DURATION 1000 /* msec */
#define HLS_TARGET_DURATION 2000 /* msec */
#if 0
#define HLS_MULTIPLE_ENCODING 1
#endif
typedef enum
{
#ifdef HLS_MULTIPLE_ENCODING
    /* Disabling the variant playlist logic for initial testing */
#if 0
    B_PlaybackIpHlsProfilesType_eLow,
#endif
    B_PlaybackIpHlsProfilesType_eMedium,
#endif
    B_PlaybackIpHlsProfilesType_eHigh,
    B_PlaybackIpHlsProfilesType_eMax
} B_PlaybackIpHlsProfilesType;
typedef struct
{
    unsigned transportBitrate;
    unsigned width;
    unsigned height;
    char *playlistString;  /* playlist specific string: low/medium/high */
    FILE *playlistFilePtr; /* playlist FILE pointer */
    char *playlistName;    /* pointer to the buffer containing the playlist name */
} B_PlaybackIpHlsProfilesInfo;

static B_PlaybackIpHlsProfilesInfo hlsProfiles[B_PlaybackIpHlsProfilesType_eMax] = {
#ifdef HLS_MULTIPLE_ENCODING
#if 0
    {385000, 320, 240, "_playlist_low", NULL, NULL},     /* low profile */
#endif
    {1100000, 640, 480, "_playlist_medium", NULL, NULL},    /* medium profile */
#endif
    {2750000, 1280, 720, "_playlist_high", NULL, NULL}    /* high profile */
};

typedef enum
{
    B_PlaybackIpDashProfilesType_eLow,
    B_PlaybackIpDashProfilesType_eMedium,
    B_PlaybackIpDashProfilesType_eHigh,
    B_PlaybackIpDashProfilesType_eMax
} B_PlaybackIpDashProfilesType;
typedef struct
{
    unsigned transportBitrate;
    unsigned width;
    unsigned height;
    char *playlistString;  /* playlist specific string: low/medium/high */
    FILE *playlistFilePtr; /* playlist FILE pointer */
    char *playlistName;    /* pointer to the buffer containing the playlist name */
} B_PlaybackIpDashProfilesInfo;

static B_PlaybackIpDashProfilesInfo dashProfiles[B_PlaybackIpDashProfilesType_eMax] = {
    {385000, 320, 240, "_playlist_low", NULL, NULL},     /* low profile */
    {1100000, 640, 480, "_playlist_medium", NULL, NULL},    /* medium profile */
    {2750000, 1280, 720, "_playlist_high", NULL, NULL}    /* high profile */
};


char *
extractMediaFileName(
    const char *mediaFileNameOrig
    )
{
    B_Error rc;
    char *mediaFileName, *tmpPtr1;
    char *mediaFileNameBuffer = NULL;

    mediaFileNameBuffer = BKNI_Malloc(strlen(mediaFileNameOrig)+1);
    PBIP_CHECK_GOTO((mediaFileNameBuffer), ( "BKNI_Malloc Failed" ), error, B_ERROR_UNKNOWN, rc );
    BKNI_Memset(mediaFileNameBuffer, 0, strlen(mediaFileNameOrig)+1);
    BKNI_Memcpy(mediaFileNameBuffer, mediaFileNameOrig, strlen(mediaFileNameOrig));
    mediaFileName = strrchr(mediaFileNameBuffer, '/'); /* mediaFileName contains the full path, take that out to just keep the media file name */
    if (!mediaFileName)
        mediaFileName = mediaFileNameBuffer;
    tmpPtr1 = strrchr(mediaFileName, '.'); /* find the extention that is preceded by . */
    if (tmpPtr1)
        *tmpPtr1 = '\0'; /* terminate the string there */
    /* now move it to the start */
    memmove(mediaFileNameBuffer, mediaFileName, strlen(mediaFileName));
    mediaFileNameBuffer[strlen(mediaFileName)] = '\0';
    BDBG_MSG(("%s: mediaFileName %s", __FUNCTION__, mediaFileNameBuffer));
    return mediaFileNameBuffer;
error:
    return NULL;
}

char *
buildAdaptiveStreamingPlaylistName(
    const char *playlistFileDirPath,
    const char *mediaFileName,
    const char *playlistSuffix
    )
{
    B_Error rc;
    char *playlistName = NULL;
    size_t playlistSize;

    /* build hls playlist name */
    playlistSize = strlen(playlistFileDirPath) + strlen(mediaFileName)+ strlen(playlistSuffix) + 1;
    playlistName = BKNI_Malloc( playlistSize );
    PBIP_CHECK_GOTO((playlistName), ( "BKNI_Malloc Failed" ), error, B_ERROR_UNKNOWN, rc );
    BKNI_Memset(playlistName, 0, playlistSize);
    snprintf(playlistName, playlistSize, "%s%s%s", playlistFileDirPath, mediaFileName, playlistSuffix);

    BDBG_MSG(("%s: playlistName %s", __FUNCTION__, playlistName));
    return playlistName;
error:
    return NULL;
}
#include <math.h>
B_Error updateTargetDurationInHlsMediaPlaylist(FILE *mediaPlaylistFp, unsigned long maxGopDuration)
{
    char *tmpPtr, *tmpPtr1;
    size_t bytesRead;
    B_Error rc = B_ERROR_UNKNOWN;
    int playlistBufferSize = 128;
    char *playlistBuffer;
    int roundedMaxGopDuration;

    roundedMaxGopDuration = (int)floor(maxGopDuration/1000.);
    if (roundedMaxGopDuration > 9) {
        BDBG_WRN(("%s: can't replace the current TARGETDURATION tag to %d as it is two digits and will need to re-write the playlist file!", __FUNCTION__, roundedMaxGopDuration));
        return B_ERROR_SUCCESS;
    }

    playlistBuffer = BKNI_Malloc( playlistBufferSize );
    PBIP_CHECK_GOTO((playlistBuffer), ( "BKNI_Malloc Failed" ), error, B_ERROR_UNKNOWN, rc );
    BDBG_MSG(("%s: mediaPlaylistFp %p, maxGopDuration %lu, playlistBuffer %p, playlistBufferSize %d", __FUNCTION__, (void *)mediaPlaylistFp, maxGopDuration, (void *)playlistBuffer, playlistBufferSize));
    fflush(mediaPlaylistFp);
    rewind(mediaPlaylistFp);
    BKNI_Memset(playlistBuffer, 0, playlistBufferSize);
    bytesRead = fread(playlistBuffer, 1, playlistBufferSize-1, mediaPlaylistFp);
    PBIP_CHECK_GOTO((bytesRead != 1), ( "fread of playlist Failed, errno %d", errno ), error, B_ERROR_UNKNOWN, rc );
    playlistBuffer[bytesRead] = '\0';

    BDBG_MSG(("%s: read playlist %s", __FUNCTION__, playlistBuffer));

    /* search for TARGETDURATION tag */
    tmpPtr = strstr(playlistBuffer, "#EXT-X-TARGETDURATION:");
    if (tmpPtr)
    {
        tmpPtr += strlen("#EXT-X-TARGETDURATION:");
        tmpPtr1 = strchr(tmpPtr, '\n');
        if (tmpPtr1) *tmpPtr1 = '\0';
        BDBG_MSG(("current duration string is %s, roundedMaxGopDuration %d", tmpPtr, roundedMaxGopDuration));
        snprintf(tmpPtr, playlistBufferSize, "%d", roundedMaxGopDuration);
        if (tmpPtr1) *tmpPtr1 = '\n';
        /* re-write the playlist with updated TARGETDURATION */
        rewind(mediaPlaylistFp);
        fwrite(playlistBuffer, 1, playlistBufferSize, mediaPlaylistFp);
        fflush(mediaPlaylistFp);
    }
    else {
        /* we should have the TARGETDURATION tag, something is not right in this playlist */
        BDBG_ASSERT(NULL);
    }

    rc = B_ERROR_SUCCESS;
error:
    return rc;
}

B_Error addEndTagToHlsMediaPlaylist(FILE *mediaPlaylistFp, char **tmpPlaylistBuffer, int *tmpPlaylistBufferSize)
{
    B_Error rc = B_ERROR_UNKNOWN;
    int bytesToWrite;
    char *mediaPlaylistBuffer;

    mediaPlaylistBuffer = *tmpPlaylistBuffer;
    /* build the media playlist */
    bytesToWrite = snprintf(mediaPlaylistBuffer, *tmpPlaylistBufferSize, "#EXT-X-ENDLIST\n");
    fwrite(mediaPlaylistBuffer, 1, bytesToWrite, mediaPlaylistFp);
    fflush(mediaPlaylistFp);

    rc = B_ERROR_SUCCESS;
    return rc;
}

B_Error updateHlsMediaPlaylist(FILE *mediaPlaylistFp, char *playlistName, unsigned hlsSegmentNumber, unsigned long segmentDruation, char **tmpPlaylistBuffer, int *tmpPlaylistBufferSize)
{
    B_Error rc = B_ERROR_UNKNOWN;
    int bytesToWrite;
    char *mediaPlaylistBuffer;

    if (*tmpPlaylistBuffer == NULL)
    {
        *tmpPlaylistBufferSize = strlen(playlistName) + 64; /* 64 extra bytes to account for _Seg<#>.ts */
        mediaPlaylistBuffer = BKNI_Malloc( *tmpPlaylistBufferSize );
        PBIP_CHECK_GOTO((mediaPlaylistBuffer), ( "BKNI_Malloc Failed" ), error, B_ERROR_UNKNOWN, rc );
        *tmpPlaylistBuffer = mediaPlaylistBuffer;
    }
    else
    {
        mediaPlaylistBuffer = *tmpPlaylistBuffer;
    }

    /* build the media playlist */
    bytesToWrite = snprintf(mediaPlaylistBuffer, *tmpPlaylistBufferSize,
            "#EXTINF:%.3f,\n"
            "%s_Seg%u.ts\n"
            ,
            (float)segmentDruation/1000.,
            playlistName+1, hlsSegmentNumber /* +1 to skip the / from the playlistName */
            );
    fwrite(mediaPlaylistBuffer, 1, bytesToWrite, mediaPlaylistFp);
    fflush(mediaPlaylistFp);
    BDBG_MSG(("%s: mediaPlaylist %s", __FUNCTION__, mediaPlaylistBuffer));

    rc = B_ERROR_SUCCESS;
error:
    return rc;
}

B_Error updateHlsMediaPlaylistUsingBcmPlayer(FILE *mediaPlaylistFp, char *playlistName, unsigned hlsSegmentNumber, long segmentNavIndex, unsigned long segmentDruation, char **tmpPlaylistBuffer, int *tmpPlaylistBufferSize)
{
    B_Error rc = B_ERROR_UNKNOWN;
    int bytesToWrite;
    char *mediaPlaylistBuffer;

    if (*tmpPlaylistBuffer == NULL)
    {
        *tmpPlaylistBufferSize = strlen(playlistName) + 64; /* 64 extra bytes to account for _Seg<#>_Idx<#>.ts */
        mediaPlaylistBuffer = BKNI_Malloc( *tmpPlaylistBufferSize );
        PBIP_CHECK_GOTO((mediaPlaylistBuffer), ( "BKNI_Malloc Failed" ), error, B_ERROR_UNKNOWN, rc );
        *tmpPlaylistBuffer = mediaPlaylistBuffer;
    }
    else
    {
        mediaPlaylistBuffer = *tmpPlaylistBuffer;
    }

    /* build the media playlist */
    bytesToWrite = snprintf(mediaPlaylistBuffer, *tmpPlaylistBufferSize,
            "#EXTINF:%.3f,\n"
            "%s_Seg%u_Idx%ld.ts\n"
            ,
            (float)segmentDruation/1000.,
            playlistName+1, hlsSegmentNumber, segmentNavIndex /* +1 to skip the / from the playlistName */
            );
    fwrite(mediaPlaylistBuffer, 1, bytesToWrite, mediaPlaylistFp);
    fflush(mediaPlaylistFp);
    BDBG_MSG(("%s: mediaPlaylist %s", __FUNCTION__, mediaPlaylistBuffer));

    rc = B_ERROR_SUCCESS;
error:
    return rc;
}

B_Error initHlsMediaPlaylist(FILE *mediaPlaylistFp, int segmentTargetDuration)
{
    B_Error rc = B_ERROR_UNKNOWN;
    int bytesToWrite;
    char *mediaPlaylistBuffer = NULL;

    mediaPlaylistBuffer = BKNI_Malloc( MEDIA_PLAYLIST_BUFFER_SIZE );
    PBIP_CHECK_GOTO((mediaPlaylistBuffer), ( "BKNI_Malloc Failed" ), error, B_ERROR_UNKNOWN, rc );
    BKNI_Memset(mediaPlaylistBuffer, 0, MEDIA_PLAYLIST_BUFFER_SIZE);

    /* we seek to start of file and add the initial playlist tags */
    rewind(mediaPlaylistFp);

    /* build the media playlist */
    bytesToWrite = snprintf(mediaPlaylistBuffer, MEDIA_PLAYLIST_BUFFER_SIZE,
            "#EXTM3U\n"
            "#EXT-X-VERSION:%d\n"
            "#EXT-X-TARGETDURATION:%d\n"
            "#EXT-X-MEDIA-SEQUENCE:%d\n"
            ,
            HLS_VERSION,
            segmentTargetDuration/1000,
            SEGMENT_START_SEQUENCE_NUMBER
            );
    fwrite(mediaPlaylistBuffer, 1, bytesToWrite, mediaPlaylistFp);
    BDBG_MSG(("%s: mediaPlaylist %s", __FUNCTION__, mediaPlaylistBuffer));

    rc = B_ERROR_SUCCESS;
error:
    if (mediaPlaylistBuffer) BKNI_Free(mediaPlaylistBuffer);
    return rc;
}

B_Error generateMpdMasterPlaylist(char *hlsMasterPlaylistName, unsigned mediaDuration, char *mediaFileName)
{
    FILE *masterPlaylistFp = NULL;
    B_Error rc = B_ERROR_UNKNOWN;
    int bytesLeft, bytesCopied;
    char *tmp=NULL, *mediaRelativeFileNamePtr=NULL, *relativeFileName=NULL;
    char *masterPlaylistBuffer = NULL;

    masterPlaylistBuffer = BKNI_Malloc( MASTER_PLAYLIST_BUFFER_SIZE );
    PBIP_CHECK_GOTO((masterPlaylistBuffer), ( "BKNI_Malloc Failed" ), error, B_ERROR_UNKNOWN, rc );
    BKNI_Memset(masterPlaylistBuffer, 0, MASTER_PLAYLIST_BUFFER_SIZE);

    masterPlaylistFp = fopen( hlsMasterPlaylistName, "w+");
    PBIP_CHECK_GOTO((masterPlaylistFp), ( "fopen Failed: errno %d", errno ), error, B_ERROR_UNKNOWN, rc );

    relativeFileName = B_PlaybackIp_UtilsStrdup( (char *)mediaFileName );
    PBIP_CHECK_GOTO((relativeFileName), ( "%s: strdup failed: errno %d",__FUNCTION__, errno ), error, B_ERROR_OUT_OF_MEMORY, rc );

    /* extract the actual file name by removing the directory path prefix */
    mediaRelativeFileNamePtr = relativeFileName;
    tmp = relativeFileName;
    while ((tmp = strstr(tmp, "/")) != NULL) {
        tmp += 1; /* move past "/" char */
        mediaRelativeFileNamePtr = tmp;
    }
    /* now mediaRelativeFileNamePtr points to the relative media file name */
    BDBG_MSG(("%s: relativeFileName %s", __FUNCTION__, relativeFileName));

    /* build the master playlist */
    bytesLeft = MASTER_PLAYLIST_BUFFER_SIZE;
    bytesCopied = snprintf(masterPlaylistBuffer, bytesLeft,
"<MPD type=\"static\" xmlns=\"urn:mpeg:dash:schema:mpd:2011\" profiles=\"urn:mpeg:dash:profile:full:2011\" mediaPresentationDuration=\"PT%dS\" minBufferTime=\"PT1.5S\">\n"
" <ProgramInformation>\n"
"  <Title>%s_master.mpd generated by Broadcom</Title>\n"
" </ProgramInformation>\n"
" <Period start=\"PT0S\">\n"
"  <AdaptationSet>\n"
"   <ContentComponent id=\"257\" contentType=\"video\"/>\n"
"   <ContentComponent id=\"258\" contentType=\"audio\"/>\n"
"   <Representation id=\"%s.txt_Seg\" bandwidth=\"%d\" width=\"%d\" height=\"%d\" codecs=\"avc1.4d001f,mp4a.40.2\" mimeType=\"video/mp2t\">\n"
"    <SegmentTemplate timescale=\"1\" duration=\"1\" startNumber=\"0\" media=\"%s$RepresentationID$$Number$.ts\"/>\n"
"   </Representation>\n"
"  </AdaptationSet>\n"
" </Period>\n"
"</MPD>",
      mediaDuration,
      mediaRelativeFileNamePtr,
      dashProfiles[B_PlaybackIpDashProfilesType_eHigh].playlistString,dashProfiles[B_PlaybackIpDashProfilesType_eHigh].transportBitrate,
       dashProfiles[B_PlaybackIpDashProfilesType_eHigh].width,dashProfiles[B_PlaybackIpDashProfilesType_eHigh].height,
      mediaRelativeFileNamePtr);

    bytesLeft = MASTER_PLAYLIST_BUFFER_SIZE - bytesCopied;

    fwrite(masterPlaylistBuffer, 1, bytesCopied, masterPlaylistFp);
    BDBG_MSG(("%s: masterPlaylist %s", __FUNCTION__, masterPlaylistBuffer));

    rc = B_ERROR_SUCCESS;
error:
    if (masterPlaylistFp) fclose(masterPlaylistFp);
    if (relativeFileName) BKNI_Free(relativeFileName);
    if (masterPlaylistBuffer) BKNI_Free(masterPlaylistBuffer);
    return rc;
}

B_Error generateHlsMasterPlaylist(char *hlsMasterPlaylistName)
{
    int i;
    FILE *masterPlaylistFp;
    B_Error rc = B_ERROR_UNKNOWN;
    int bytesLeft, bytesCopied;
    char *masterPlaylistBuffer = NULL;

    masterPlaylistBuffer = BKNI_Malloc( MASTER_PLAYLIST_BUFFER_SIZE );
    PBIP_CHECK_GOTO((masterPlaylistBuffer), ( "BKNI_Malloc Failed" ), error, B_ERROR_UNKNOWN, rc );
    BKNI_Memset(masterPlaylistBuffer, 0, MASTER_PLAYLIST_BUFFER_SIZE);

    masterPlaylistFp = fopen( hlsMasterPlaylistName, "w+");
    PBIP_CHECK_GOTO((masterPlaylistFp), ( "fopen Failed: errno %d", errno ), error, B_ERROR_UNKNOWN, rc );

    /* build the master playlist */
    bytesLeft = MASTER_PLAYLIST_BUFFER_SIZE;
    bytesCopied = snprintf(masterPlaylistBuffer, bytesLeft, "#EXTM3U\n");
    bytesLeft = MASTER_PLAYLIST_BUFFER_SIZE - bytesCopied;

    for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
    {
        bytesCopied += snprintf(masterPlaylistBuffer+bytesCopied, bytesLeft,
            "#EXT-X-STREAM-INF:PROGRAM-ID=1, BANDWIDTH=%d\n"
            "%s\n",
            hlsProfiles[i].transportBitrate, hlsProfiles[i].playlistName+1 /* +1 to skip the / from the playlist name */
            );
        bytesLeft = MASTER_PLAYLIST_BUFFER_SIZE - bytesCopied;
    }
    fwrite(masterPlaylistBuffer, 1, bytesCopied, masterPlaylistFp);
    fclose(masterPlaylistFp);
    BDBG_MSG(("%s: masterPlaylist %s", __FUNCTION__, masterPlaylistBuffer));

    rc = B_ERROR_SUCCESS;
error:
    if (masterPlaylistBuffer) BKNI_Free(masterPlaylistBuffer);
    return rc;
}

void trimHlsRelativeNameExtension(char *hlsRelativePlaylistName)
{
    char *tmpPtr;
    tmpPtr = strstr(hlsRelativePlaylistName, ".m3u8");
    if (tmpPtr)
        *tmpPtr = '\0';
    strncat(hlsRelativePlaylistName, ".txt", strlen(".txt"));
}

B_Error buildHlsXcodeParamsFile(
    const char *playlistFileDirPath,
    const char *mediaFileName
    )
{
    int bytesCopied;
    B_Error rc;
    int i;
    char *hlsXcodeParamsBuffer = NULL;
    char *hlsXcodeParamFileName = NULL;
#define HLS_XCODE_PARAMS_BUFFER_SIZE 1024
    hlsXcodeParamsBuffer = BKNI_Malloc(HLS_XCODE_PARAMS_BUFFER_SIZE);
    PBIP_CHECK_GOTO((hlsXcodeParamsBuffer), ( "BKNI_Malloc Failed" ), error, B_ERROR_UNKNOWN, rc );
    for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
    {
        FILE *hlsXcodeParamFilePtr;
        hlsXcodeParamFileName = buildAdaptiveStreamingPlaylistName(playlistFileDirPath, hlsProfiles[i].playlistName, "");
        PBIP_CHECK_GOTO((hlsXcodeParamFileName), ( "buildAdaptiveStreamingPlaylistName Failed "), error, B_ERROR_UNKNOWN, rc );
        hlsXcodeParamFilePtr = fopen( hlsXcodeParamFileName, "w+");
        PBIP_CHECK_GOTO((hlsXcodeParamFilePtr), ( "fopen Failed: errno %d", errno ), error, B_ERROR_UNKNOWN, rc );

        /* build the xcode param list into a buffer */
        bytesCopied = snprintf(hlsXcodeParamsBuffer, HLS_XCODE_PARAMS_BUFFER_SIZE,
                "/File=%s;TranscodeEnabled=Yes;OutWidth=%d;OutHeight=%d;TransportBitrate=%d;"
                "OutVideoCodec=H264;OutFrameRate=30;OutAudio=Yes;OutAudioCodec=AAC;HlsEnabled=Yes;OutAspecRatio=16x9;NonRealTimeMode=Yes;",
                mediaFileName,
                hlsProfiles[i].width,
                hlsProfiles[i].height,
                hlsProfiles[i].transportBitrate
                );
        /* now write the xcode params to this file */
        fwrite(hlsXcodeParamsBuffer, 1, bytesCopied, hlsXcodeParamFilePtr);
        BKNI_Free(hlsXcodeParamFileName);
        hlsXcodeParamFileName = NULL;
        fclose(hlsXcodeParamFilePtr);
    }
    rc = B_ERROR_SUCCESS;
error:
    if (hlsXcodeParamFileName)
        BKNI_Free(hlsXcodeParamFileName);
    if (hlsXcodeParamsBuffer)
        BKNI_Free(hlsXcodeParamsBuffer);
    return rc;
}
int generateMpdPlaylist(
    unsigned mediaDuration,
    const char *mediaFileNameFull,
    const char *playlistFileDirPath
    )
{
    char *mediaFileName = NULL;
    B_Error rc = B_ERROR_UNKNOWN;
    char *mpdMasterPlaylistName = NULL;
    struct stat st;

    BDBG_MSG(("%s:playlistFileDirPath %s mediaFileName %s, duration %u", __FUNCTION__, playlistFileDirPath,  mediaFileName, mediaDuration));

    mediaFileName = extractMediaFileName( mediaFileNameFull );
    PBIP_CHECK_GOTO((mediaFileName), ( "extractMediaFileName Failed "), error, B_ERROR_UNKNOWN, rc );

    /* TODO change name to adaptive playListName */
    mpdMasterPlaylistName = buildAdaptiveStreamingPlaylistName(playlistFileDirPath, mediaFileName, "_master.mpd");
    PBIP_CHECK_GOTO((mpdMasterPlaylistName), ( "buildAdaptiveStreamingPlaylistName Failed "), error, B_ERROR_UNKNOWN, rc );

    rc = stat(mpdMasterPlaylistName, &st);
    if (rc == 0 /* success */ && st.st_size > 0) {
        BDBG_MSG(("%s: %s exists of size %"PRId64 ", returning", __FUNCTION__, mpdMasterPlaylistName, st.st_size));
        if (mediaFileName) BKNI_Free(mediaFileName);
        if (mpdMasterPlaylistName) BKNI_Free(mpdMasterPlaylistName);
        return B_ERROR_SUCCESS;
    }

    /* generate mpd master Playlist */
    rc = generateMpdMasterPlaylist(mpdMasterPlaylistName,mediaDuration, mediaFileName);
    PBIP_CHECK_GOTO((rc==B_ERROR_SUCCESS), ( "generateMpdPlaylist Failed" ), error, B_ERROR_UNKNOWN, rc );

    error:

    if (mpdMasterPlaylistName) BKNI_Free(mpdMasterPlaylistName);
    if (mediaFileName) BKNI_Free(mediaFileName);
     return rc;
}

int generateHlsPlaylist(
    unsigned mediaDuration,
    const char *mediaFileNameFull,
    const char *playlistFileDirPath
    )
{
    unsigned i, currentDuration;
    char *mediaFileName = NULL;
    B_Error rc = B_ERROR_UNKNOWN;
    char *hlsMasterPlaylistName = NULL;
    char *tmpPlaylistBuffer = NULL;
    int tmpPlaylistBufferSize;
    struct stat st;

    BDBG_MSG(("%s: mediaFileName %s, duration %u", __FUNCTION__, mediaFileName, mediaDuration));

    mediaFileName = extractMediaFileName( mediaFileNameFull );
    PBIP_CHECK_GOTO((mediaFileName), ( "extractMediaFileName Failed "), error, B_ERROR_UNKNOWN, rc );

    hlsMasterPlaylistName = buildAdaptiveStreamingPlaylistName(playlistFileDirPath, mediaFileName, "_master.m3u8");
    PBIP_CHECK_GOTO((hlsMasterPlaylistName), ( "buildAdaptiveStreamingPlaylistName Failed "), error, B_ERROR_UNKNOWN, rc );

    rc = stat(hlsMasterPlaylistName, &st);
    if (rc == 0 /* success */ && st.st_size > 0) {
        BDBG_MSG(("%s: %s exists of size %"PRId64 ", returning", __FUNCTION__, hlsMasterPlaylistName, st.st_size));
        if (mediaFileName) BKNI_Free(mediaFileName);
        if (hlsMasterPlaylistName) BKNI_Free(hlsMasterPlaylistName);
        return B_ERROR_SUCCESS;
    }

    /* build the relative playlist names */
    for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
    {
        hlsProfiles[i].playlistName = buildAdaptiveStreamingPlaylistName( mediaFileName, hlsProfiles[i].playlistString, ".m3u8");
        PBIP_CHECK_GOTO((hlsProfiles[i].playlistName), ( "buildAdaptiveStreamingPlaylistName Failed "), error, B_ERROR_UNKNOWN, rc );
    }
    /* generate master Playlist */
    rc = generateHlsMasterPlaylist(hlsMasterPlaylistName);
    PBIP_CHECK_GOTO((rc==B_ERROR_SUCCESS), ( "generateHlsPlaylist Failed" ), error, B_ERROR_UNKNOWN, rc );

    /* create different profiles of the media playlists */
    /* these are so chosen based on the Vice2 Firmware API document recommendation. */
    for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
    {
        char *playlistName;
        playlistName = buildAdaptiveStreamingPlaylistName(playlistFileDirPath, hlsProfiles[i].playlistName, "");
        PBIP_CHECK_GOTO((playlistName), ( "buildAdaptiveStreamingPlaylistName Failed "), error, B_ERROR_UNKNOWN, rc );
        hlsProfiles[i].playlistFilePtr = fopen( playlistName, "w+");
        PBIP_CHECK_GOTO((hlsProfiles[i].playlistFilePtr), ( "fopen Failed: errno %d", errno ), error, B_ERROR_UNKNOWN, rc );
        rc = initHlsMediaPlaylist(hlsProfiles[i].playlistFilePtr, HLS_TARGET_DURATION);
        PBIP_CHECK_GOTO((rc==B_ERROR_SUCCESS), ( "initHlsMediaPlaylist Failed" ), error, B_ERROR_UNKNOWN, rc );
        BKNI_Free(playlistName);
    }

    /* now trim the m3u8 extension from the relative playlist names as it is used part of the segment URI name */
    for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
    {
        trimHlsRelativeNameExtension(hlsProfiles[i].playlistName);
    }

    /* generate the txt file with all xcode parameters */
    buildHlsXcodeParamsFile(playlistFileDirPath, mediaFileNameFull);

    for (currentDuration = 0; currentDuration < mediaDuration/1000; currentDuration++)
    {
        for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
        {
            updateHlsMediaPlaylist(hlsProfiles[i].playlistFilePtr, hlsProfiles[i].playlistName, currentDuration, HLS_SEGMENT_DURATION, &tmpPlaylistBuffer, &tmpPlaylistBufferSize);
        }
    }
    for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
    {
        rc = addEndTagToHlsMediaPlaylist(hlsProfiles[i].playlistFilePtr, &tmpPlaylistBuffer, &tmpPlaylistBufferSize);
        PBIP_CHECK_GOTO((rc==B_ERROR_SUCCESS), ("addEndTagToHlsMediaPlaylist Failed" ), error, B_ERROR_UNKNOWN, rc );
    }
error:
    for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
    {
        if (hlsProfiles[i].playlistFilePtr) fclose(hlsProfiles[i].playlistFilePtr);
        if (hlsProfiles[i].playlistName) BKNI_Free(hlsProfiles[i].playlistName);
    }

    if (hlsMasterPlaylistName) BKNI_Free(hlsMasterPlaylistName);
    if (mediaFileName) BKNI_Free(mediaFileName);
    if (tmpPlaylistBuffer) BKNI_Free(tmpPlaylistBuffer);
    return rc;
}


int generateHlsPlaylistUsingBcmPlayer(
    void *context,
    const char *mediaFileNameFull,
    const char *playlistFileDirPath
    )
{
    int i;
    char *mediaFileName = NULL;
    BNAV_Player_Handle hBcmPlayer = (BNAV_Player_Handle) context;
    BNAV_Player_Settings cfg;
    long firstIndex, lastIndex;
    bool firstTime = true;
    unsigned hlsSegmentNumber = 0;
    B_Error rc = B_ERROR_UNKNOWN;
    unsigned long gopDuration = 0, maxGopDuration = 0;
    unsigned long totalGopDuration = 0;
    long currentRapIndex = 1; /* Rap is Random Access Position: I/IDR frames */
    BNAV_Player_Position segmentStartRapPosition, segmentEndRapPosition;
    char *hlsMasterPlaylistName = NULL;
    char *tmpPlaylistBuffer = NULL;
    int tmpPlaylistBufferSize;
    struct stat st;

    BDBG_MSG(("%s: hBcmPlayer %p, mediaFileName %s", __FUNCTION__, (void *)hBcmPlayer, mediaFileName));
    PBIP_CHECK_GOTO((hBcmPlayer), ( "NULL NAV file handle" ), error, B_ERROR_UNKNOWN, rc );

    mediaFileName = extractMediaFileName( mediaFileNameFull );
    PBIP_CHECK_GOTO((mediaFileName), ( "extractMediaFileName Failed "), error, B_ERROR_UNKNOWN, rc );

    hlsMasterPlaylistName = buildAdaptiveStreamingPlaylistName(playlistFileDirPath, mediaFileName, "_master.m3u8");
    PBIP_CHECK_GOTO((hlsMasterPlaylistName), ( "buildAdaptiveStreamingPlaylistName Failed "), error, B_ERROR_UNKNOWN, rc );

    rc = stat(hlsMasterPlaylistName, &st);
    if (rc == 0 /* success */ && st.st_size > 0) {
        BDBG_MSG(("%s: %s exists of size %"PRId64 ", returning", __FUNCTION__, hlsMasterPlaylistName, st.st_size));
        return B_ERROR_SUCCESS;
    }

    /* build the relative playlist names */
    for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
    {
        hlsProfiles[i].playlistName = buildAdaptiveStreamingPlaylistName( mediaFileName, hlsProfiles[i].playlistString, ".m3u8");
        PBIP_CHECK_GOTO((hlsProfiles[i].playlistName), ( "buildAdaptiveStreamingPlaylistName Failed "), error, B_ERROR_UNKNOWN, rc );
    }
    /* generate master Playlist */
    rc = generateHlsMasterPlaylist(hlsMasterPlaylistName);
    PBIP_CHECK_GOTO((rc==B_ERROR_SUCCESS), ( "generateHlsPlaylist Failed" ), error, B_ERROR_UNKNOWN, rc );

    /* create different profiles of the media playlists */
    /* these are so chosen based on the Vice2 Firmware API document recommendation. */
    for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
    {
        char *playlistName;
        playlistName = buildAdaptiveStreamingPlaylistName(playlistFileDirPath, hlsProfiles[i].playlistName, "");
        PBIP_CHECK_GOTO((playlistName), ( "buildAdaptiveStreamingPlaylistName Failed "), error, B_ERROR_UNKNOWN, rc );
        hlsProfiles[i].playlistFilePtr = fopen( playlistName, "w+");
        PBIP_CHECK_GOTO((hlsProfiles[i].playlistFilePtr), ( "fopen Failed: errno %d", errno ), error, B_ERROR_UNKNOWN, rc );
        rc = initHlsMediaPlaylist(hlsProfiles[i].playlistFilePtr, HLS_TARGET_DURATION);
        PBIP_CHECK_GOTO((rc==B_ERROR_SUCCESS), ( "initHlsMediaPlaylist Failed" ), error, B_ERROR_UNKNOWN, rc );
        BKNI_Free(playlistName);
    }

    /* now trim the m3u8 extension from the relative playlist names as it is used part of the segment URI name */
    for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
    {
        trimHlsRelativeNameExtension(hlsProfiles[i].playlistName);
    }

    /* generate the txt file with all xcode parameters */
    buildHlsXcodeParamsFile(playlistFileDirPath, mediaFileNameFull);

    /* reset the NAV player */
    BNAV_Player_GetSettings(hBcmPlayer, &cfg);
    rc = BNAV_Player_DefaultGetBounds(hBcmPlayer, cfg.filePointer, &firstIndex, &lastIndex);
    PBIP_CHECK_GOTO((rc==B_ERROR_SUCCESS), ( "BNAV_Player_DefaultGetBounds Failed" ), error, B_ERROR_UNKNOWN, rc );
    rc = BNAV_Player_SetCurrentIndex(hBcmPlayer, firstIndex);
    PBIP_CHECK_GOTO((rc==B_ERROR_SUCCESS), ( "BNAV_Player_SetCurrentIndex Failed" ), error, B_ERROR_UNKNOWN, rc );

    do
    {
        if (!firstTime)
        {
            /* find next i-frame */
            currentRapIndex = BNAV_Player_FindIFrameFromIndex(hBcmPlayer, currentRapIndex+2 /* start index */, 1 /* direction */);
            PBIP_CHECK_GOTO((currentRapIndex > 0), ("BNAV_Player_FindIFrameFromIndex Failed" ), error, B_ERROR_UNKNOWN, rc );
            /* get info about the this i-frame */
            rc = BNAV_Player_GetPositionInformation(hBcmPlayer, currentRapIndex, &segmentEndRapPosition);
            PBIP_CHECK_GOTO((rc==B_ERROR_SUCCESS), ("BNAV_Player_GetPositionInformation Failed" ), error, B_ERROR_UNKNOWN, rc );

            /* now find this GOP duration */
            gopDuration = (segmentEndRapPosition.pts >= segmentStartRapPosition.pts) ?  (segmentEndRapPosition.pts - segmentStartRapPosition.pts) : ((unsigned long)(-1) - segmentStartRapPosition.pts + 1 + segmentEndRapPosition.pts); /* PTS wrap */
            gopDuration /= 45; /* convert from 45Khz PTS units to msec */
            if (gopDuration < HLS_SEGMENT_DURATION)
            {
                /* this GOP doesn't meet the GOP duration, continue to check the next one! */
                BDBG_MSG(("%s: gop duration [%lu] msec doesn't yet meet HSL Segment duration %d: start[%ld]@pts=%08x, end[%ld] pts=%08x", __FUNCTION__, gopDuration, HLS_SEGMENT_DURATION*1000, segmentStartRapPosition.index, (unsigned int)segmentStartRapPosition.pts, currentRapIndex, (unsigned int)segmentEndRapPosition.pts));
                continue;
            }
            else {
                /* the time delta between the current starting GOP & this ending GOP is >= HLS Segment Duration. So we choose this index number as the URL index. */
                /* add segment to the playlist */
#if 1
                if (gopDuration > HLS_TARGET_DURATION && gopDuration > 5000) /* if GOP is larger than 5 sec, it most likely is a discontinutity. This needs more thinking, for now set it to max of 9sec */
                {
                    BDBG_MSG(("%s: !! Discontinuity: hlsSegmentNumber %u, gop duration [%lu] msec, start[%ld]@pts=%08x, end[%ld] pts=%08x", __FUNCTION__, hlsSegmentNumber, gopDuration, segmentStartRapPosition.index, (unsigned int)segmentStartRapPosition.pts, currentRapIndex, (unsigned int)segmentEndRapPosition.pts));
                    gopDuration = 5000;
                }
#endif
                for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
                {
                    updateHlsMediaPlaylistUsingBcmPlayer(hlsProfiles[i].playlistFilePtr, hlsProfiles[i].playlistName, hlsSegmentNumber, segmentStartRapPosition.index, gopDuration, &tmpPlaylistBuffer, &tmpPlaylistBufferSize);
                }
                if (gopDuration > maxGopDuration)
                    maxGopDuration = gopDuration;
                BDBG_MSG(("%s: hlsSegmentNumber %u, gop duration:max %lu:%lu msec, start[%ld]@pts=%08x, end[%ld] pts=%08x", __FUNCTION__, hlsSegmentNumber, gopDuration, maxGopDuration, segmentStartRapPosition.index, (unsigned int)segmentStartRapPosition.pts, currentRapIndex, (unsigned int)segmentEndRapPosition.pts));
                hlsSegmentNumber++;
                totalGopDuration += gopDuration;
                segmentStartRapPosition = segmentEndRapPosition;
            }
        }
        else
        {
            /* find first i-frame */
            currentRapIndex = BNAV_Player_FindIFrameFromIndex(hBcmPlayer, 1 /* start index */, 1 /* direction */);
            PBIP_CHECK_GOTO((currentRapIndex > 0), ("BNAV_Player_FindIFrameFromIndex Failed" ), error, B_ERROR_UNKNOWN, rc );
            /* get info about the 1st i-frame */
            rc = BNAV_Player_GetPositionInformation(hBcmPlayer, currentRapIndex, &segmentStartRapPosition);
            PBIP_CHECK_GOTO((rc==B_ERROR_SUCCESS), ("BNAV_Player_GetPositionInformation Failed" ), error, B_ERROR_UNKNOWN, rc );
            firstTime = false;
            hlsSegmentNumber = 0;
            BDBG_MSG(("%s ##### first idx %d, pts %ul, timestamp %ul",  __FUNCTION__, (int)currentRapIndex, (unsigned int)segmentStartRapPosition.pts, (unsigned int)segmentStartRapPosition.timestamp));
        }
    } while (true);

error:
    if (hlsSegmentNumber)
    {
        for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
        {
            rc = addEndTagToHlsMediaPlaylist(hlsProfiles[i].playlistFilePtr, &tmpPlaylistBuffer, &tmpPlaylistBufferSize);
            PBIP_CHECK_GOTO((rc==B_ERROR_SUCCESS), ("addEndTagToHlsMediaPlaylist Failed" ), error, B_ERROR_UNKNOWN, rc );
        }
        BDBG_MSG(("%s: hlsMasterPlaylistName %s, hlsSegmentNumber %u, totalGopDuration %lu msec, maxGopDuration %lu", __FUNCTION__, hlsMasterPlaylistName, hlsSegmentNumber, totalGopDuration, maxGopDuration));
        for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
        {
            if (maxGopDuration > HLS_TARGET_DURATION)
            {
                /* need to rewind the playlist file & update the TARGET-DURATION */
                rc = updateTargetDurationInHlsMediaPlaylist(hlsProfiles[i].playlistFilePtr, maxGopDuration);
                PBIP_CHECK_GOTO((rc==B_ERROR_SUCCESS), ("updateTargetDurationInHlsMediaPlaylist Failed" ), error, B_ERROR_UNKNOWN, rc );
            }
        }
        rc = BERR_SUCCESS;
    }
    for (i=0; i < B_PlaybackIpHlsProfilesType_eMax; i++)
    {
        if (hlsProfiles[i].playlistFilePtr) fclose(hlsProfiles[i].playlistFilePtr);
        if (hlsProfiles[i].playlistName) BKNI_Free(hlsProfiles[i].playlistName);
    }

    if (hlsMasterPlaylistName) BKNI_Free(hlsMasterPlaylistName);
    if (mediaFileName) BKNI_Free(mediaFileName);
    if (tmpPlaylistBuffer) BKNI_Free(tmpPlaylistBuffer);
    return rc;
}

int nav_indexer_create(
    const char *mediaFileName,
    const char *indexFileName,
    B_PlaybackIpPsiInfoHandle psi
    )
{
    int rc = -1;
    unsigned char *readBuf = NULL;
    FILE *fin = NULL, *fout = NULL;
    sTsIndexer *indexer = NULL;
    BNAV_Indexer_Handle bcmindexer = NULL;
    unsigned long readBytes;
    unsigned long numBytesToRead;
    BNAV_Indexer_Settings settings;
    tsindex_settings tsSettings;

    if (psi->mpegType != NEXUS_TransportType_eTs) {
        BDBG_ERR(("NAV index creation is not supported for this video codec %d container format %d", psi->videoCodec, psi->mpegType));
        return -1;
    }

    if (psi->videoPid == 0) {
        BDBG_WRN(("%s: creating index for video codec %d, container %d when video pid is 0", __FUNCTION__, psi->videoCodec, psi->mpegType));
    }

    if ((fin = fopen(mediaFileName, "rb" )) == NULL) {
        BDBG_ERR(("%s: Unable to open input file %s\n", __FUNCTION__, mediaFileName));
        goto error;
    }

    if ((fout = fopen(indexFileName, "wb" )) == NULL) {
        BDBG_ERR(("%s: Unable to open output file %s\n", __FUNCTION__, indexFileName));
        goto error;
    }
    numBytesToRead = psi->transportTimeStampEnabled ? (192*1024) : (188*1024);
    readBuf = BKNI_Malloc(numBytesToRead);
    if (!readBuf) {
        BDBG_ERR(("%s: memory allocation failure for %d bytes", __FUNCTION__, (int)numBytesToRead));
        goto error;
    }
    BDBG_WRN(("Creating NAV index (%s) for video codec %d, pid %d, container format %d, TTS %d, frame rate %d",
                indexFileName, psi->videoCodec, psi->videoPid, psi->mpegType, psi->transportTimeStampEnabled, (int)psi->videoFrameRate));

    BNAV_Indexer_GetDefaultSettings(&settings);
    settings.mpegSizeCallback = mpegSizeCallback;
    g_mpegFile = fin;
    settings.writeCallback = (INDEX_WRITE_CB)fwrite;
    settings.filePointer = (void *)fout;
    settings.transportTimestampEnabled = psi->transportTimeStampEnabled;
    /* use PTS based index timestamp to be accurate; simulatedFrameRate timestamps drifted away for film content! */
    settings.ptsBasedFrameRate = true;
    settings.sctVersion = BSCT_Version6wordEntry;
    if (psi->videoCodec == NEXUS_VideoCodec_eH264) {
        settings.videoFormat = BNAV_Indexer_VideoFormat_AVC;
        settings.navVersion = BNAV_Version_AVC;
    }
    else if (psi->videoCodec == NEXUS_VideoCodec_eH265) {
        settings.videoFormat = BNAV_Indexer_VideoFormat_HEVC;
        settings.navVersion = BNAV_Version_HEVC;
    }
    else if (psi->mpegType == NEXUS_TransportType_eTs) {
        /* default settings are for MPEG2 TS */
        settings.videoFormat = BNAV_Indexer_VideoFormat_MPEG2;
    }
    BDBG_MSG(("bcmindex settings: navVersion %d, ts %d, fr %d, format %d, sct v %d nav v %d \n",
        settings.navVersion, settings.transportTimestampEnabled, settings.simulatedFrameRate,
        settings.videoFormat, settings.sctVersion, settings.navVersion));

    if (BNAV_Indexer_Open(&bcmindexer, &settings)) {
        BDBG_ERR(("%s: BNAV_Indexer_Open failed for input file %s\n", __FUNCTION__, mediaFileName));
        goto error;
    }

    tsindex_settings_init(&tsSettings);
    tsSettings.pid = (unsigned short)psi->videoPid;
    if (psi->videoCodec == NEXUS_VideoCodec_eH264 || psi->videoCodec == NEXUS_VideoCodec_eH265) {
        tsSettings.entry_size = 6;
        tsSettings.start_code_lo = 0x00;
        tsSettings.start_code_hi = 0xFF;
        tsSettings.is_avc = 1;
    }
    else {
        /* default settings are for MPEG2 TS */
        tsSettings.entry_size = 6;
    }
    tsSettings.cb = (INDEX_WRITE_CB)write_to_bcmindexer;
    tsSettings.fp = (void*)bcmindexer;
    tsSettings.ts_packet_size = psi->transportTimeStampEnabled ? 192 : 188;

    BDBG_MSG(("ts indexer settings: pid %d, entry sz %d, settings.is_avc %d, ts_packet_size %d",
        tsSettings.pid, tsSettings.entry_size, tsSettings.is_avc, tsSettings.ts_packet_size));
    indexer = tsindex_allocate_ex(&tsSettings);
    if (!indexer) {
        BDBG_ERR(("%s: tsindex_allocate_ex() failed for input file %s\n", __FUNCTION__, mediaFileName));
        goto error;
    }

    /*time1 = getms();*/
    while ((readBytes = fread(readBuf, 1, numBytesToRead, fin)) != 0) {
        tsindex_feed(indexer, readBuf, readBytes);
    }
    rc = 0;
    BDBG_WRN(("%s: Created NAV index (%s)", __FUNCTION__, indexFileName));

error:
    if (bcmindexer)
        BNAV_Indexer_Close(bcmindexer);
    if (indexer)
        tsindex_free(indexer);

    if (fin) fclose(fin);
    if (fout) fclose(fout);
    if (readBuf) BKNI_Free(readBuf);
    return rc;
}
