/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 *****************************************************************************/
#if defined(LINUX) || defined(__vxworks)
#if defined(LINUX)
#include <sys/syscall.h>
#ifdef __mips__
#ifndef DMS_CROSS_PLATFORMS
#include <asm/cachectl.h>
#endif /* DMS_CROSS_PLATFORMS */
#endif /* __mips__ */
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <math.h>

#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_utils.h"
#include "b_playback_ip_nav_indexer.h"

#include "bmedia_probe.h"
#include "bmpeg_video_probe.h"
#include "bmpeg2ts_probe.h"
#include "bhevc_video_probe.h"
#if B_HAS_ASF
#include "basf_probe.h"
#endif
#include "bfile_stdio.h"
#endif
#include "bcmindexer_nav.h"

BDBG_MODULE(b_playback_ip_file_streaming);

#define SEARCH_NUM_GOPS 10
#define SEARCH_MAX_FRAMES_PER_GOP 300
/* This value is the multipler used to achieve a desired playspeed when combined with iFrameModifier (how many I-Frames to advance the bcm player) */
/* speed = FRAME_RATE_MULTIPLIER * IFrameModifier */
/* we fix the FRAME_RATE_MULTIPLIER to 6. That means if we play all i-frames (iFrameModifier ==1), then we will get 6x speed */
/* 6 is chosen with following in mind: since I-frames are much larger in size, restricting this number will control the n/w b/w */
/* otherwise, if client's frame rate in trickmode is set higher, then it will consume more i-frames leading to higher n/w b/w utilization */
#define FRAME_RATE_MULTIPLIER 6
static const struct bfile_io_write net_io_data_write = {
    B_PlaybackIp_UtilsStreamingCtxWrite,
    NULL,
    BIO_DEFAULT_PRIORITY
};

typedef struct B_PlaybackIpIndexer {
    void *context;  /* returned by the nav player */
    int framesPerGop;
    double framesPerSec;
    double frameTime;
    double minISpeed;
    double bitRate;
    int    frameRepeat;
    int    msPerGop;
    int    tmModifier;
    /* TODO: take these out */
    unsigned long first_pts;
    unsigned long duration;
} B_PlaybackIpIndexer;

/* # of io vectors for the sendmsg() call, currently set to 2 to avoid IP fragmentation */
/* should be increased when our NIC starts supporting Segmentation Offload technology to allow single sendmsg call */
typedef struct B_PlaybackIpFileStreaming
{
    B_PlaybackIpFileStreamingOpenSettings settings;
    int fd;
    FILE *fp;
    int indexFileFd;
    FILE *indexFileFp;
    struct bfile_io_write_net data;
    struct bfile_io_write_net dataLocalStreaming;
    B_ThreadHandle streamingThread;
    bool stop;  /* set when app wants to stop the streaming session */
    bool threadRunning; /* set when file streaming thread is running */
    BKNI_EventHandle stopStreamingEvent;
    off_t totalBytesStreamed;
    B_PlaybackIpConnectionState connectionState;    /* state of the socket: active, timeouts, error, eof */
    B_PlaybackIpIndexer indexer;
    B_PlaybackIpPsiInfo psi;
    unsigned char *streamingBuf;
    unsigned char *streamingBufOrig;
    unsigned streamingBufSize;
    unsigned char *pkt; /* buffer for storing BTPs */
    unsigned char *pktOrig; /* original buffer for storing BTPs */
    BKNI_EventHandle bufferAvailableEvent;
    unsigned nPrograms; /* number of programs detected  in FileStream*/
    bool ipVerboseLog;
} B_PlaybackIpFileStreaming;

/***************************************************************************
Summary:
This function stops streaming content from a file.
***************************************************************************/
void
B_PlaybackIp_FileStreamingClose(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle
    )
{
    BDBG_MSG(("%s: %p", __FUNCTION__, (void *)fileStreamingHandle));

    if (fileStreamingHandle) {
        if (fileStreamingHandle->fp) {
            fclose(fileStreamingHandle->fp);
        }
        else if (fileStreamingHandle->fd >= 0) {
            close(fileStreamingHandle->fd);
        }
        if (fileStreamingHandle->indexFileFp) {
            if (fileStreamingHandle->indexer.context)
                nav_indexer_close(fileStreamingHandle->indexer.context);

            if (fileStreamingHandle->indexFileFp) {
                fclose(fileStreamingHandle->indexFileFp);
            }
            else if (fileStreamingHandle->indexFileFd >= 0) {
                close(fileStreamingHandle->indexFileFd);
            }
        }

        if (fileStreamingHandle->bufferAvailableEvent)
            BKNI_DestroyEvent(fileStreamingHandle->bufferAvailableEvent);
        BKNI_Free(fileStreamingHandle);
    }
    return;
}

void B_PlaybackIp_FileStreaming_WriteCompleteCallback(void* context)
{
    BKNI_SetEvent((BKNI_EventHandle)context);
}

B_PlaybackIpFileStreamingHandle
B_PlaybackIp_FileStreamingOpen(
    const B_PlaybackIpFileStreamingOpenSettings *fileStreamingSettings
    )
{
    B_PlaybackIpFileStreamingHandle fileStreamingHandle = NULL;

    BDBG_MSG(("%s:\n", __FUNCTION__));
    if (!fileStreamingSettings) {
        BDBG_ERR(("%s: Invalid param: Need to pass-in file open Settings\n", __FUNCTION__));
        goto error;
    }

    fileStreamingHandle = BKNI_Malloc(sizeof(B_PlaybackIpFileStreaming));
    if (!fileStreamingHandle) {
        BDBG_ERR(("%s: memory allocation failure\n", __FUNCTION__));
        goto error;
    }
    memset(fileStreamingHandle, 0, sizeof(B_PlaybackIpFileStreaming));

    {
        char *pValue = NULL;
        pValue = getenv("ipVerboseLog");
        if (pValue)
            fileStreamingHandle->ipVerboseLog = true;
        else
            fileStreamingHandle->ipVerboseLog = false;
    }

    fileStreamingHandle->settings = *fileStreamingSettings;
    if (fileStreamingHandle->settings.playSpeed == 0)
        fileStreamingHandle->settings.playSpeed = 1;

    fileStreamingHandle->fd = open(fileStreamingSettings->fileName, O_RDONLY | O_LARGEFILE);
    if (fileStreamingHandle->fd >= 0)
        fileStreamingHandle->fp = fdopen(fileStreamingHandle->fd, "r");
    if (fileStreamingHandle->fd < 0 || !fileStreamingHandle->fp) {
        BDBG_ERR(("%s: failed to open file (%s), fd %d, fp %p, errno %d\n", __FUNCTION__, fileStreamingSettings->fileName, fileStreamingHandle->fd, (void *)fileStreamingHandle->fp, errno));
        goto error;
    }

    if (fileStreamingSettings->mediaProbeOnly) {
        BDBG_MSG(("Skipping streaming session setup as mediaProbeOnly flag is set"));
        goto out;
    }
    BDBG_MSG(("%s: open file (%s), fd %d, fp %p, protocol %d", __FUNCTION__, fileStreamingSettings->fileName, fileStreamingHandle->fd, (void *)fileStreamingHandle->fp, fileStreamingHandle->settings.protocol));

out:
    fileStreamingHandle->connectionState = B_PlaybackIpConnectionState_eSetup;
    return fileStreamingHandle;

error:
    B_PlaybackIp_FileStreamingClose(fileStreamingHandle);
    return NULL;
}

void
preparePlaySpeedList(
    int minISpeed,
    B_PlaybackIpPsiInfoHandle psi
    )
{
    /* these speeds are chosen based on how we can achieve them */
    psi->playSpeed[0] = -16*FRAME_RATE_MULTIPLIER;
    psi->playSpeed[1] = -8*FRAME_RATE_MULTIPLIER;
    psi->playSpeed[2] = -4*FRAME_RATE_MULTIPLIER;
    psi->playSpeed[3] = -2*FRAME_RATE_MULTIPLIER;
    psi->playSpeed[4] = -1*FRAME_RATE_MULTIPLIER;
    psi->playSpeed[5] = 1*FRAME_RATE_MULTIPLIER;
    psi->playSpeed[6] = 2*FRAME_RATE_MULTIPLIER;
    psi->playSpeed[7] = 4*FRAME_RATE_MULTIPLIER;
    psi->playSpeed[8] = 8*FRAME_RATE_MULTIPLIER;
    psi->playSpeed[9] = 16*FRAME_RATE_MULTIPLIER;
    psi->numPlaySpeedEntries = 10;

    /* TODO: this is being deprecated and will be removed once new server side trickmode approach gets fully tested */
    psi->httpMinIFrameSpeed = minISpeed;
}

static bool
getNextNameValuePair(FILE *fp, char *buf, char **name, char **value)
{
    char *tmp_buf = NULL;

    /* each line in info file is of name=value format */
    if (!fgets(buf, 128, fp))
        goto error;
    *name = buf;

    if ((tmp_buf = strstr(buf, "=")) == NULL)
        goto error;
    *tmp_buf = '\0';

    *value = tmp_buf + 1;
    BDBG_MSG(("%s: name %s, value %s", __FUNCTION__, *name, *value));
    return true;
error:
    return false;
}

bool
parseMediaInfo(
    char *fileName,
    FILE *fp,
    B_PlaybackIpPsiInfoHandle psi
    )
{
    char buf[128];
    char *name, *value;
    bool foundVideo = false, foundAudio = false;

    while (getNextNameValuePair(fp, buf, &name, &value) == true) {
        if (strncasecmp(name, "containerType", 128) == 0) {
            psi->mpegType = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "duration", 128) == 0) {
            psi->duration = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "bitRate", 128) == 0) {
            psi->avgBitRate = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "vidPid", 128) == 0) {
            psi->videoPid = strtol(value, NULL, 10);
            foundVideo = true;
        }
        else if (strncasecmp(name, "videoCodec", 128) == 0) {
            psi->videoCodec = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "extraVideoCodec", 128) == 0) {
            psi->extraVideoCodec = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "extraVidPid", 128) == 0) {
            psi->extraVideoPid = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "videoWidth", 128) == 0) {
            psi->videoWidth = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "videoHeight", 128) == 0) {
            psi->videoHeight = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "videoBitrate", 128) == 0) {
            psi->videoBitrate = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "videoFrameRate", 128) == 0) {
            psi->videoFrameRate = strtof(value, NULL);
        }
        else if (strncasecmp(name, "colorDepth", 128) == 0) {
            psi->colorDepth = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "audPid", 128) == 0) {
            psi->audioPid = strtol(value, NULL, 10);
            foundAudio = true;
        }
        else if (strncasecmp(name, "audioCodec", 128) == 0) {
            psi->audioCodec = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "audioBitsPerSample", 128) == 0) {
            psi->audioBitsPerSample = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "audioSampleRate", 128) == 0) {
            psi->audioSampleRate = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "audioNumChannels", 128) == 0) {
            psi->audioNumChannels = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "audioBitRate", 128) == 0) {
            psi->audioBitrate = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "pcrPid", 128) == 0) {
            psi->pcrPid = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "tts", 128) == 0) {
            psi->transportTimeStampEnabled = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "contentLength", 128) == 0) {
            psi->contentLength = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "numPlaySpeed", 128) == 0) {
            psi->numPlaySpeedEntries = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "minISpeed", 128) == 0) {
            psi->httpMinIFrameSpeed = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "frameRepeat", 128) == 0) {
            psi->httpFrameRepeat = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "frameRateInTrickMode", 128) == 0) {
            psi->frameRateInTrickMode = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "firstPts", 128) == 0) {
            psi->firstPts = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "psiValid", 128) == 0) {
            psi->psiValid = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "numPrograms", 128) == 0) {
            psi->numPrograms = strtol(value, NULL, 10);
        }
        else if (strncasecmp(name, "numTracks", 128) == 0) {
            psi->numTracks = strtol(value, NULL, 10);
        }
    }
    /* now get the playspeed entries */
    if (psi->numPlaySpeedEntries)
        preparePlaySpeedList(psi->httpMinIFrameSpeed, psi);

    BDBG_MSG(("Successfully read info file for media name %s\n", fileName));
    return true;
}

void
replaceFileExtension(char *newName, int newNameSize, char *curName, char *newExtension)
{
    char *tmpPtr, *curPtr = NULL, *prevPtr = NULL;

    strncpy(newName, curName, newNameSize);
    curPtr = newName;
    while (true) {
        tmpPtr = strstr(curPtr, ".");
        if (tmpPtr) {
            curPtr = tmpPtr+1; /* point to location of current . and search for next one */
            prevPtr = tmpPtr;   /* location of current . */
            continue;
        }
        else {
            /* no more . found, break */
            break;
        }
    }
    if (prevPtr)
        *prevPtr = '\0';   /* overwrite the existing . char in the file name for following strcat to work */
    strncat(newName, newExtension, newNameSize);
}

/* build info file as follows: */
/* <infoFilesDir>/<mediaFileName>_<programIndex>.info */
char *
buildInfoFileName(char *infoFilesDir, char *mediaFileName, unsigned programIndex)
{
    int infoFileNameSize, len;
    char *infoFileName, *mediaRelativeFileNamePtr, *tmp;
    struct stat st;
    char *curPtr, *tmpPtr, *prevPtr = NULL;

    /* extract the actual file name by removing the directory path prefix */
    mediaRelativeFileNamePtr = mediaFileName;
    tmp = mediaFileName;
    while ((tmp = strstr(tmp, "/")) != NULL) {
        tmp += 1; /* move past "/" char */
        mediaRelativeFileNamePtr = tmp;
    }
    /* now mediaRelativeFileNamePtr points to the relative media file name */

    if (stat(mediaFileName, &st) < 0 ) {
        BDBG_ERR(("%s: stat() failed on the media file %s, errno %d", __FUNCTION__, mediaFileName, errno));
        perror("stat");
        return NULL;
    }
    if(programIndex)
    {
        infoFileNameSize = strlen(infoFilesDir) + 33 + strlen(mediaRelativeFileNamePtr) +2 + 10; /*  2 for "_<index>" 32 bytes for i-node number, 10 extra bytes for storing optional extensions */
    }
    else
    {
        infoFileNameSize = strlen(infoFilesDir) + 33 + strlen(mediaRelativeFileNamePtr) + 10; /* 32 bytes for i-node number, 10 extra bytes for storing optional extensions */
    }
    infoFileName = (char *)BKNI_Malloc(infoFileNameSize+1);
    if (!infoFileName) {
        BDBG_ERR(("%s: memory allocation failure at %d\n", __FUNCTION__, __LINE__));
        return NULL;
    }
    memset(infoFileName, 0, infoFileNameSize);
    /* concatenate the info dir path to the info file name */
    len = snprintf(infoFileName, infoFileNameSize-1, "%s/%s", infoFilesDir, mediaRelativeFileNamePtr);
    /* now insert the inode number into the info file name so that two files with same name residing in two different directories can still have the same info file when info files are kept in one directory */
    curPtr = infoFileName;
    while (true) {
        tmpPtr = strstr(curPtr, ".");
        if (tmpPtr) {
            curPtr = tmpPtr+1; /* point to location of current . and search for next . if present */
            prevPtr = tmpPtr;   /* location of current . */
            continue;
        }
        else
            /* no more . found, break */
            break;
    }
    if (!prevPtr) {
        /* media file name didn't have any . in it, so append info extension towards its end */
        prevPtr = infoFileName + len;
    }
    len = prevPtr - infoFileName;
    if(programIndex > 0)
    {
        snprintf(prevPtr, infoFileNameSize-1-len, "_%d_%d%s", (int) st.st_size, programIndex, B_PLAYBACK_IP_INFO_FILE_EXTENSION);
    }
    else
    {
        snprintf(prevPtr, infoFileNameSize-1-len, "_%d%s", (int) st.st_size, B_PLAYBACK_IP_INFO_FILE_EXTENSION);
    }
    BDBG_MSG(("info file name %s", infoFileName));
    return infoFileName;
}

int
getStreamGopInfo(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle,
    B_PlaybackIpPsiInfoHandle psi
    )
{
    B_PlaybackIpIndexer *idx = &fileStreamingHandle->indexer;
    int iFrameRateRounded; /* number of i-frames in a sec: usually 1 or 2 */
    float iFrameRate; /* number of i-frames in a sec: usually 1 or 2 */

    struct index_entry_t entry1;
    struct index_entry_t *entry = &entry1;
    int count[3];
    int j,k;
    unsigned long pcr1, pcr2;
    ssize_t  start, end;
    int totalFrames=0;
    int minISpeed;

    if (idx->context == NULL)
        /* index doesn't exist for this format, shouldn't be called */
        return -1;

    if (nav_indexer_seek(idx->context, 0, &idx->duration, &idx->first_pts)) {
        BDBG_WRN(("%s: failed to seek to start of file\n", __FUNCTION__));
        return -1;
    }

    /* skip initial few I-frames */
    for (k=0;k<3;k++) {
        entry->type = 0;
        for (j=0;j<SEARCH_MAX_FRAMES_PER_GOP ;j++) {
            nav_indexer_next(idx->context, entry);
            if (entry->type == eSCTypeIFrame) {
                BDBG_MSG(("%s: found %d frames in a GOP, size %lu", __FUNCTION__, j, entry->size));
                break;
            }
        }
    }

    /* Now we have I-frame */
    pcr1 = entry->pcr;

    count[0] = 0;
    count[1] = 0;
    count[2] = 0;

    start = entry->offset;

    /* count I, B, and P frames in 1st SEARCH_NUM_GOPS */
    for (k=0; k<SEARCH_NUM_GOPS; k++) {
        entry->type = 0;
        for (j=0; j<SEARCH_MAX_FRAMES_PER_GOP; j++) {
            if (nav_indexer_next(idx->context, entry)) {
                break;
            }
            if (entry->type == eSCTypePFrame)
                count[1]++;
            else if (entry->type == eSCTypeBFrame)
                count[2]++;
            else if (entry->type == eSCTypeIFrame) {
                count[0]++;
                BDBG_MSG(("%s: found %d frames in a GOP, frame size %lu", __FUNCTION__, j, entry->size));
                break;
            }
        }
    }

    pcr2 = entry->pcr;
    end =  entry->offset;
    totalFrames = count[0]+count[1]+count[2];

    idx->framesPerGop = totalFrames / SEARCH_NUM_GOPS;
    idx->framesPerSec = totalFrames / ((pcr2 - pcr1 + .001)/45000.);
    idx->bitRate = 8 * (end - start) / ((pcr2 - pcr1 + .001)/45000.);
    idx->msPerGop = (1000.*idx->framesPerGop/idx->framesPerSec);
    iFrameRate = count[0] / ((pcr2 - pcr1 + .001)/45000.);
    if (iFrameRate > 1.0 && iFrameRate < 1.5)
        /* special case to handle cases where frameRate is just about 1, but it is really 1 i-frame per sec */
        iFrameRateRounded = 1;
    else
        iFrameRateRounded = (int) ceil(iFrameRate);

    /* TODO: deprecate following code once new playSpeed changes are fully tested */
    if (psi->videoCodec == NEXUS_VideoCodec_eH264) {
        if (fileStreamingHandle->settings.playSpeed == 2)
            idx->frameRepeat = 1;
        else
            idx->frameRepeat = 4;
        idx->minISpeed = idx->msPerGop/ AVC_FRAME_TIME;
        idx->minISpeed = 1;
    }
    else {
        if (fileStreamingHandle->settings.playSpeed == -1)
            idx->frameRepeat = 8;
        else if (fileStreamingHandle->settings.playSpeed == -2)
            idx->frameRepeat = 6;
        else if (fileStreamingHandle->settings.playSpeed == -4)
            idx->frameRepeat = 6;
        else
            idx->frameRepeat = 2;
        idx->minISpeed = 16;
    }
    psi->httpFrameRepeat = idx->frameRepeat;
    minISpeed = ceil(idx->minISpeed);
    /* end deprecated code */

    BDBG_WRN(("StreamInfo: iFrameRate %f, %d, BitRate=%lf(mbps) FramesPerSec=%lf FramesPerGoP=%d, msPerGOP=%d frameRepeat=%d minISpeed=%lf, # of I=%d P=%d B=%d, file %s",
        iFrameRate, iFrameRateRounded, idx->bitRate, idx->framesPerSec, idx->framesPerGop,idx->msPerGop,idx->frameRepeat,idx->minISpeed, count[0],count[1],count[2], fileStreamingHandle->settings.fileName));
    preparePlaySpeedList(minISpeed, psi);
    psi->frameRateInTrickMode = FRAME_RATE_MULTIPLIER * iFrameRateRounded;
    return true;
}

bool
setIndexerMode(B_PlaybackIpIndexer *idx, int speed, B_PlaybackIpPsiInfoHandle psi)
{
    trick_mode_t tm;

    tm.speed = abs(speed);
    if (speed > 0)
        tm.direction = 1; /* forward */
    else
        tm.direction = -1; /* backward */
    tm.videoType = psi->videoCodec;
    tm.modifier = tm.speed / FRAME_RATE_MULTIPLIER;
    if (tm.modifier == 0)
        tm.modifier = 1;
    if (nav_indexer_mode(idx->context, &tm) == false) {
        return false;
    }
    return true;
}

bool
setupMediaIndexerTrickMode(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle,
    int playSpeed,
    B_PlaybackIpPsiInfoHandle psi
    )
{
    B_PlaybackIpIndexer *indexer = &fileStreamingHandle->indexer;
    B_PlaybackIpFileStreamingOpenSettings *fileStreamingSettings;

    BDBG_MSG(("%s: file streaming handle %p, playSpeed %d ", __FUNCTION__, (void *)fileStreamingHandle, playSpeed));

    if (setIndexerMode(indexer, playSpeed, psi) == false) {
        BDBG_ERR(("%s: Failed to set the indexer mode for file session %p\n", __FUNCTION__, (void *)fileStreamingHandle));
        return false;
    }
    fileStreamingSettings = &fileStreamingHandle->settings;
    if (fileStreamingSettings->beginTimeOffset || fileStreamingSettings->endTimeOffset) {
        nav_indexer_setIndexByTimeOffset(indexer->context, fileStreamingSettings->beginTimeOffset);
    }
    else if (fileStreamingSettings->beginFileOffset) {
        nav_indexer_setIndexByByteOffset(indexer->context, fileStreamingSettings->beginFileOffset);
    }
    return true;
}

extern int generateHlsPlaylist( unsigned mediaDuration, const char *mediaFileNameFull, const char *playlistFileDirPath);
extern int generateMpdPlaylist( unsigned mediaDuration, const char *mediaFileNameFull, const char *playlistFileDirPath);

bool
openMediaIndexer(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle,
    const char *indexFileName,
    B_PlaybackIpPsiInfoHandle psi
    )
{
    bool rc = false;
    int fd = -1;
    FILE *fp = NULL;
    struct stat fileStats;
    int numRetries = 0;

    if (!psi)
        return false;

    BDBG_MSG(("%s: for index file %s", __FUNCTION__, indexFileName));
retry:
    fileStats.st_size = 0;
    if ( ((fd = open(indexFileName, O_RDONLY)) < 0) || ((fstat(fd, &fileStats) == 0) && fileStats.st_size <= 0) || ((fp = fdopen(fd, "r")) == NULL) ) {
        if (nav_indexer_create(fileStreamingHandle->settings.fileName, indexFileName, psi) != 0) {
            fileStreamingHandle->indexer.context = NULL;
            BDBG_WRN(("Failed to create the index file %s for media %s", indexFileName, fileStreamingHandle->settings.fileName));
            rc = true;    /* note: we treat this as a soft error as we can still stream the file, but can't do server side trickmodes */
            goto error;
        }
        if (numRetries++ > 1) {
            BDBG_WRN(("Failed to create the index file %s for med ../*ia %s, continue w/o it", indexFileName, fileStreamingHandle->settings.fileName));
            rc = true;
            goto error;
        }
        else
            goto retry;
    }

    /* index file exists, open it */
    if (nav_indexer_open(&fileStreamingHandle->indexer.context, fp, psi) < 0) {
        BDBG_ERR(("%s: Failed to open the index for file %s", __FUNCTION__, indexFileName));
        rc = true;
        goto error;
    }

    fileStreamingHandle->indexFileFd = fd;
    fileStreamingHandle->indexFileFp = fp;

    if (!fileStreamingHandle->settings.disableHlsPlaylistGeneration) {
        /* generate the HLS Playlist */
#if 1
        generateHlsPlaylist(psi->duration, fileStreamingHandle->settings.fileName, fileStreamingHandle->settings.mediaInfoFilesDir);
        generateMpdPlaylist(psi->duration, fileStreamingHandle->settings.fileName, fileStreamingHandle->settings.mediaInfoFilesDir);
#else

        /* For now, we are not using the BCM Player for generating the index */
        generateHlsPlaylistUsingBcmPlayer( fileStreamingHandle->indexer.context, fileStreamingHandle->settings.fileName, fileStreamingHandle->settings.mediaInfoFilesDir);
#endif
    }
    return true;

error:

    if (fp){ fclose( fp );}
    else if (fd>=0){ close( fd ); }
    return false;
}

int
createInfoFile(
    char *fileName,
    char *infoFileName,
    B_PlaybackIpPsiInfoHandle psi
    )
{
    FILE *fp;

    /* Now write the PSI info into the media info file */
    if ((fp = fopen(infoFileName, "w")) == NULL) {
        BDBG_ERR(("Failed to obtain fp for Info file (%s) errno %d\n", infoFileName, errno));
        return -1;
    }
    fprintf(fp,
            "mediaFile=%s\n"
            "contentLength=%lld\n"
            ,
            fileName,
            (long long)psi->contentLength
            );
    if (psi->psiValid) {
        fprintf(fp,
            "containerType=%d\n"
            "duration=%d\n"
            "bitRate=%d\n"
            "vidPid=%d\n"
            "videoCodec=%d\n"
            "videoWidth=%d\n"
            "videoHeight=%d\n"
            "videoBitrate=%d\n"
            "videoFrameRate=%.3f\n"
            "colorDepth=%d\n"
            "audPid=%d\n"
            "audioCodec=%d\n"
            "audioBitsPerSample=%d\n"
            "audioSampleRate=%d\n"
            "audioNumChannels=%d\n"
            "audioBitRate=%d\n"
            "pcrPid=%d\n"
            "tts=%d\n"
            "numPrograms=%d\n"
            "numTracks=%d\n"
            ,
            psi->mpegType,
            psi->duration,
            psi->avgBitRate,
            psi->videoPid,
            psi->videoCodec,
            psi->videoWidth,
            psi->videoHeight,
            psi->videoBitrate,
            psi->videoFrameRate,
            psi->colorDepth,
            psi->audioPid,
            psi->audioCodec,
            psi->audioBitsPerSample,
            psi->audioSampleRate,
            psi->audioNumChannels,
            psi->audioBitrate,
            psi->pcrPid,
            psi->transportTimeStampEnabled,
            psi->numPrograms,
            psi->numTracks
            );

        if (psi->extraVideoCodec != NEXUS_VideoCodec_eNone && psi->extraVideoPid != 0) {
            fprintf(fp, "extraVidPid=%d\n" "extraVideoCodec=%d\n", psi->extraVideoPid, psi->extraVideoCodec);
        }

        if (psi->numPlaySpeedEntries) {
            fprintf(fp,
                "numPlaySpeed=%d\n"
                "minISpeed=%d\n"
                "frameRepeat=%d\n"
                "frameRateInTrickMode=%d\n"
                "firstPts=%d\n"
                ,
                psi->numPlaySpeedEntries,
                psi->httpMinIFrameSpeed,
                psi->httpFrameRepeat,
                psi->frameRateInTrickMode,
                psi->firstPts
                );
        }
    }
    fprintf(fp, "psiValid=%d\n", psi->psiValid);
    fflush(fp);
    fclose(fp);
    return 0;
}

static bool
getMediaInfo(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle,
    char *infoFileName,
    unsigned programIndex,
    B_PlaybackIpPsiInfoHandle psi
    )
{
    int fdesc;
    unsigned prevBmediaProgamId =-1;  /* this value does not necssarily start with 1 */

    unsigned convertedProgramIndex = 0;  /* starts with 0 and will change when bmediProgramId changes */
    bmedia_probe_t probe = NULL;
    bmedia_probe_config probe_config;
    const bmedia_probe_stream *stream = NULL;
    const bmedia_probe_track *track = NULL;
    bfile_io_read_t fd = NULL;
    bool foundAudio = false, foundVideo = false;
    char stream_info[512];
    FILE *fp = NULL;
    struct stat fileStats;
    struct stat infoFileStats;
    B_PlaybackIpFileStreamingOpenSettings *fileStreamingSettings;
    int infoFileNameLength;
    char *indexFileName;
    bool rc = false;

    fileStreamingSettings = &fileStreamingHandle->settings;
    if (fstat(fileStreamingHandle->fd, &fileStats) || fileStats.st_size <= 0) {
        BDBG_ERR(("Can't obtain file stats info on media file (%s) %d\n", fileStreamingSettings->fileName, errno));
        return false;
    }

    /* see if a valid info file already exists */
    if ((fdesc = open(infoFileName, O_RDONLY)) >= 0) {
        /* info file exists, check if it valid and media file is not newer than its info file (true for timeshifting content) */
        if ( (fstat(fdesc, &infoFileStats) == 0) && infoFileStats.st_size > 0 && infoFileStats.st_mtime > fileStats.st_mtime ) {
            /* file has some size, populate PSI structure from the info file */
            fp = fdopen(fdesc, "r");
            if (fp && parseMediaInfo(fileStreamingSettings->fileName, fp, psi)) {
                /* success in reading the info file */
                if (psi->psiValid == 0) {
                    /* but info file doesn't contains valid PSI info */
                    BDBG_MSG(("%s: current PSI info not valid, dont create info file for file (%s) \n", __FUNCTION__, fileStreamingSettings->fileName));
                }
                /* open index file */
                else if (psi->numPlaySpeedEntries) {
                    /* play speed entries only get set if a media format has separate index file and thus we can support server side trickmodes */
                    infoFileNameLength = strlen(infoFileName);
                    indexFileName = infoFileName;
                    replaceFileExtension(indexFileName, infoFileNameLength, infoFileName, B_PLAYBACK_IP_INDEX_FILE_EXTENSION);
                    if (openMediaIndexer(fileStreamingHandle, indexFileName, psi) == false) {
                        BDBG_WRN(("%s: Failed to open/obtain Index File for file (%s) \n", __FUNCTION__, fileStreamingSettings->fileName));
                        goto error;
                    }
                    if (psi->videoCodec == NEXUS_VideoCodec_eH264) {
                        if (fileStreamingHandle->settings.playSpeed == 2)
                            psi->httpFrameRepeat = 1;
                        else
                            psi->httpFrameRepeat = 4;
                    }
                    else {
                        if (fileStreamingHandle->settings.playSpeed == -1)
                            psi->httpFrameRepeat = 8;
                        else if (fileStreamingHandle->settings.playSpeed == -2)
                            psi->httpFrameRepeat = 6;
                        else if (fileStreamingHandle->settings.playSpeed == -4)
                            psi->httpFrameRepeat = 6;
                        else
                            psi->httpFrameRepeat = 2;
                    }
                }
                if(fp)
                {
                    fclose(fp);
                    fp = NULL;
                    fdesc = -1;
                }
                return true;
            }
        }
    }

    /* close out fp and fd to the info file  */
    if(fp)
    {
        fclose(fp);
        fp = NULL;
        fdesc = -1;
    }
    else if(fdesc >=0)
    {
        close(fdesc);
        fdesc =-1;
    }

    if ((probe = bmedia_probe_create()) == NULL) {
        BDBG_ERR(("Can't create a Media probe object for parsing file %s \n", fileStreamingSettings->fileName));
        rc = false;
        goto error;
    }

    fd = bfile_stdio_read_attach(fileStreamingHandle->fp);

    bmedia_probe_default_cfg(&probe_config);
    probe_config.file_name = fileStreamingSettings->fileName;
    probe_config.type = bstream_mpeg_type_unknown;
    stream = bmedia_probe_parse(probe, fd, &probe_config);

    /* now stream is either NULL, or stream descriptor with linked list of audio/video tracks */
    if (!stream) {
        BDBG_ERR(("Media probe can't parse stream '%s'", fileStreamingSettings->fileName));
        rc = false;
        psi->psiValid = false;
        goto writeInfoFile;
    }

#if 0
    /* TODO: this can be used to determine if the media format contains index info */
    if (stream->index == bmedia_probe_index_available || stream->index == bmedia_probe_index_required)
#endif
    bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
    BDBG_MSG(( "Media Probe for %s:\n" "%s\n\n", fileStreamingSettings->fileName, stream_info));

    psi->mpegType = B_PlaybackIp_UtilsMpegtype2ToNexus(stream->type);
    psi->duration = stream->duration;
    psi->avgBitRate = stream->max_bitrate;
    psi->psiValid = true;
    if (psi->mpegType == NEXUS_TransportType_eTs && ((((bmpeg2ts_probe_stream*)stream)->pkt_len) == 192)) {
        BDBG_MSG(("%s: TTS Stream\n", __FUNCTION__));
        psi->transportTimeStampEnabled = true;
    }
    else {
        psi->transportTimeStampEnabled = false;
    }


    for(track=BLST_SQ_FIRST(&stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {

        if(programIndex != 0) /*represents the old way of not having an index */
        {
            if(prevBmediaProgamId != track->program)
            {
                convertedProgramIndex ++;
                BDBG_WRN(("New program index %d (prevBmediaProgamId %d , track->program %d )", convertedProgramIndex, prevBmediaProgamId , track->program));
            }
            prevBmediaProgamId = track->program;
            if(convertedProgramIndex != programIndex)
            {

                BDBG_WRN(("%s: Skip getinfo, this track doesn't correspond to this Program. track program %d  programIndex %d type %d \n", __FUNCTION__, track->program, programIndex, track->type));
                continue;
            }
        }
        switch(track->type) {
        case bmedia_track_type_audio:
            if(track->info.audio.codec != baudio_format_unknown && !foundAudio) {
                psi->audioPid = track->number;
                psi->audioCodec = B_PlaybackIp_UtilsAudioCodec2Nexus(track->info.audio.codec);
                psi->audioBitsPerSample = track->info.audio.sample_size;
                psi->audioSampleRate = track->info.audio.sample_rate;
                psi->audioNumChannels = track->info.audio.channel_count;
                psi->audioBitrate = 1000*track->info.audio.bitrate; /* probe returns in Kbps, convert to bps */
                foundAudio = true;
            }
            break;
            case bmedia_track_type_video:
            if (track->info.video.codec == bvideo_codec_h264_svc || track->info.video.codec == bvideo_codec_h264_mvc) {
                psi->extraVideoPid = track->number;
                psi->extraVideoCodec = B_PlaybackIp_UtilsVideoCodec2Nexus(track->info.video.codec);
            }
            else if (track->info.video.codec != bvideo_codec_unknown && !foundVideo) {
                char string[16];
                psi->videoPid = track->number;
                psi->videoCodec = B_PlaybackIp_UtilsVideoCodec2Nexus(track->info.video.codec);
                psi->videoWidth = track->info.video.width;
                psi->videoHeight = track->info.video.height;
                psi->videoBitrate = 1000*track->info.video.bitrate; /* probe returns in Kbps, convert to bps */
                if (psi->videoCodec == NEXUS_VideoCodec_eMpeg2) {
                    snprintf(string, sizeof(string)-1, "%d.%d",     /* media probe returns in 1000 FPS units, convert to string & then into float */
                        ((bmedia_probe_mpeg_video*)&track->info.video.codec_specific)->framerate/1000,
                        ((bmedia_probe_mpeg_video*)&track->info.video.codec_specific)->framerate%1000);
                    psi->videoFrameRate = strtof(string, NULL);
                    if (psi->videoFrameRate <= 0)
                        psi->videoFrameRate = 30.;
                }
                psi->colorDepth = bmedia_probe_get_video_color_depth(track);

                foundVideo = true;

#if 0
                /* TODO: may need to extract extra media related info */
#if B_HAS_ASF
                if (stream->type == bstream_mpeg_type_asf) {
                    basf_probe_track *asf_track = (basf_probe_track *)track;
                    if (asf_track->aspectRatioValid) {
                        psi->aspectRatio = NEXUS_AspectRatio_eSar;
                        psi->sampleAspectRatio.x = asf_track->aspectRatio.x;
                        psi->sampleAspectRatio.y = asf_track->aspectRatio.y;
                    }
                }
#endif
#endif
            }
            break;
            case bmedia_track_type_pcr:
                psi->pcrPid = track->number;
            default:
            break;
        }


    }
    BDBG_MSG(("Media Details: container type %d, index %d, avg bitrate %d, duration %d, # of programs %d, # of tracks %d\n",
            psi->mpegType, stream->index, psi->avgBitRate, psi->duration, stream->nprograms, stream->ntracks));

     /* used to determine max number of programs */
     fileStreamingHandle->nPrograms = stream->nprograms;
     psi->numPrograms = stream->nprograms;
     psi->numTracks = stream->ntracks;

    if (!foundVideo && !foundAudio) {
        BDBG_ERR(("Media probe didn't find any audio or video tracks for media '%s'", fileStreamingSettings->fileName));
        rc = false;
        goto error;
    }

    /* open media index */
    if (!fileStreamingSettings->disableIndexGeneration && psi->mpegType == NEXUS_TransportType_eTs
            /* Note: fow now we are disabling index generation for H265 formats as it is not currently supported */
            && psi->videoCodec != NEXUS_VideoCodec_eH265 && programIndex == 0
#if 0
        /* TODO: server side trickmodes didn't seem to work for Mpeg2PES files, need to look into it later */
        || psi->mpegType == NEXUS_TransportType_eMpeg2Pes
#endif
       ) {
        /* NAV index is only being used for TS files */
        /* For ASF, MP4, index is already part of the container format */
        /* TODO: include PES & VOBs at some point */
        BDBG_MSG(("%s: NAV index is only being used for TS formats (current file format %d)", __FUNCTION__, psi->mpegType));

        infoFileNameLength = strlen(infoFileName);
        indexFileName = infoFileName;
        replaceFileExtension(indexFileName, infoFileNameLength, infoFileName, B_PLAYBACK_IP_INDEX_FILE_EXTENSION);
        if (openMediaIndexer(fileStreamingHandle, indexFileName, psi) == false) {
            BDBG_WRN(("%s: Failed to create/obtain Index File for file (%s) \n", __FUNCTION__, fileStreamingSettings->fileName));
            goto error;
        }
        /* now acquire GOP related info */
        getStreamGopInfo(fileStreamingHandle, psi);

        replaceFileExtension(infoFileName, infoFileNameLength, indexFileName, B_PLAYBACK_IP_INFO_FILE_EXTENSION);
    }

writeInfoFile:
    psi->contentLength = fileStats.st_size;

    if (createInfoFile(fileStreamingSettings->fileName, infoFileName, psi)) {
        BDBG_ERR(("%s: Failed to create info file %s", __FUNCTION__, fileStreamingSettings->fileName));
        rc = false;
        goto error;
    }
    rc = true;
    BDBG_MSG(("%s: Media info file %s created", __FUNCTION__, infoFileName));
error:
    if (probe) {
        if (stream) bmedia_probe_stream_free(probe, stream);
        if (fd) bfile_stdio_read_detach(fd);
        bmedia_probe_destroy(probe);
    }
    if(fp)
    {
        fclose(fp);
    }
    return rc;
}

bool
getReadOffset(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle,
    int *dioBeginOffsetAdjustment,
    int *dioEndOffsetAdjustment,
    off_t *dioBeginOffset
    )
{
    int rc;
    struct index_entry_t entry;

    while (true) {
        memset(&entry, 0, sizeof(entry));
        if ( (rc = nav_indexer_next(fileStreamingHandle->indexer.context, &entry)) ) {
            BDBG_WRN(("%s: nav_indexer_next returned %d, error or EOF", __FUNCTION__, rc));
            return false;
        }
        if (entry.insertPkt) {
            /* special pkt is sent, so goto to next entry */
            continue;
        }
        else {
            /* make the entry dio compatible */
            *dioBeginOffset = entry.offset & ~DIO_MASK;
            *dioBeginOffsetAdjustment = entry.offset - *dioBeginOffset;
            *dioEndOffsetAdjustment = DIO_BLK_SIZE - ( (entry.size+*dioBeginOffsetAdjustment) % DIO_BLK_SIZE);
            return true;
        }
    }
    return false;
}

bool
getByteRangeFromTimeRange(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle
    )
{
    B_PlaybackIpIndexer *indexer = &fileStreamingHandle->indexer;
    B_PlaybackIpFileStreamingOpenSettings *fileStreamingSettings;
    int dioEndOffsetAdjustment = 0;
    int dioBeginOffsetAdjustment = 0;

    fileStreamingSettings = &fileStreamingHandle->settings;

    if (setIndexerMode(indexer, 1/*fileStreamingSettings->playSpeed*/, &fileStreamingHandle->psi) == false) {
        BDBG_ERR(("%s: Failed to set the indexer mode for file session %p\n", __FUNCTION__, (void *)fileStreamingHandle));
        return false;
    }

    if((int)fileStreamingSettings->beginTimeOffset != 0) {
        nav_indexer_setIndexByTimeOffset(indexer->context, fileStreamingSettings->beginTimeOffset);
        getReadOffset(fileStreamingHandle, &dioBeginOffsetAdjustment, &dioEndOffsetAdjustment, &fileStreamingSettings->beginFileOffset);
        lseek(fileStreamingHandle->fd, 0, SEEK_SET);
        nav_indexer_rewind(indexer->context);
    }
    else {
        fileStreamingSettings->beginFileOffset = 0;
    }

    if( (int)fileStreamingSettings->endTimeOffset != 0) {
        nav_indexer_setIndexByTimeOffset(indexer->context, fileStreamingSettings->endTimeOffset);
        getReadOffset(fileStreamingHandle, &dioBeginOffsetAdjustment, &dioEndOffsetAdjustment, &fileStreamingSettings->endFileOffset);
        lseek(fileStreamingHandle->fd, 0, SEEK_SET);
        nav_indexer_rewind(indexer->context);
    }
    else {
        fileStreamingSettings->endFileOffset = 0;
    }
    return true;
}

/***************************************************************************
Summary:
This function returns the Media information associated with a file in psi.
***************************************************************************/
B_PlaybackIpError
B_PlaybackIp_FileStreamingGetMediaInfo(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle,
    B_PlaybackIpPsiInfoHandle psi
    )
{
    B_PlaybackIpError rc;
    char *infoFileName = NULL;
    B_PlaybackIpFileStreamingOpenSettings *fileStreamingSettings;
    unsigned programIndex;

    BDBG_MSG(("%s: %p", __FUNCTION__, (void *)fileStreamingHandle));
    BDBG_ASSERT(fileStreamingHandle);

    if (!fileStreamingHandle || !psi) {
        BDBG_ERR(("%s: Invalid params: fileStreamingHandle %p, psi %p\n", __FUNCTION__, (void *)fileStreamingHandle, (void *)psi));
        rc = B_ERROR_INVALID_PARAMETER;
        goto error;
    }
    fileStreamingSettings = &fileStreamingHandle->settings;
    memset(psi, 0, sizeof(B_PlaybackIpPsiInfo));


    if (fileStreamingSettings->programIndex)
        programIndex = fileStreamingSettings->programIndex;
    else
        programIndex = 0; /* default is 0, 0 will create an infor and nav with no programIndex to its file name, 1>=  programIndex follows bmedia program indexing */


    BDBG_MSG(("%s: programIndex: %d", __FUNCTION__, programIndex));


    /* programIndex 0 build info and nav file name without the index */
    /*programIndex >= 1  build infor and nav file with _{progamIndex}*/
    do
    {
        /* info file is <mediaInfoFilesDir>/<fileName>_<programIndex>.info */
        if ((infoFileName = buildInfoFileName(fileStreamingSettings->mediaInfoFilesDir, fileStreamingSettings->fileName, programIndex)) == NULL) {
            BDBG_WRN(("%s: Failed to allocate memory for Info File for (%s) \n", __FUNCTION__, fileStreamingSettings->fileName));
            rc = B_ERROR_UNKNOWN;
            goto error;
        }

        if (getMediaInfo(fileStreamingHandle, infoFileName, programIndex, psi) == false) {
            BDBG_WRN(("%s: Failed to create/obtain Info File for file (%s) \n", __FUNCTION__, fileStreamingSettings->fileName));
            rc = B_ERROR_UNKNOWN;
            goto error;
        }

        if (programIndex == fileStreamingSettings->programIndex) {
                 fileStreamingHandle->psi = *psi;  /* cache the PSI info */

            /*get byte range from time range using the indexer
             if the given end time offset is equal to total duration of the file ,
             over write the endByteOffset given by indexer with the file size
             this is needed because byte offset given by the indexer for the file end time is not matching with the end duraion given by probe*/
            if(fileStreamingHandle->settings.endTimeOffset || fileStreamingHandle->settings.beginTimeOffset) {
               if(getByteRangeFromTimeRange(fileStreamingHandle)) {
                    if((uint)fileStreamingHandle->settings.endTimeOffset == psi->duration/1000) {
                        fileStreamingHandle->settings.endFileOffset = psi->contentLength - 1;
                    }
                    psi->beginFileOffset = fileStreamingHandle->settings.beginFileOffset;
                    psi->endFileOffset = fileStreamingHandle->settings.endFileOffset;
                    fileStreamingHandle->psi = *psi;
               }
            }
        }

        if (infoFileName)
             BKNI_Free(infoFileName);

        if (fileStreamingSettings->generateAllProgramsInfoFiles == false)
        {    break;}

        BDBG_MSG(("%s: program Index %d nPrograms %d", __FUNCTION__,programIndex, fileStreamingHandle->nPrograms));
    } while (++programIndex <= fileStreamingHandle->nPrograms);

    rc = B_ERROR_SUCCESS;
    return rc;
error:
    if (infoFileName)
        BKNI_Free(infoFileName);

    return rc;
}

bool
setReadOffset(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle,
    int *dioBeginOffsetAdjustment,
    int *dioEndOffsetAdjustment,
    unsigned char **readBuf,
    int *bytesToRead
    )
{
    int rc;
    off_t dioBeginOffset;
    struct index_entry_t entry;

    while (!fileStreamingHandle->stop) {
        memset(&entry, 0, sizeof(entry));
        if ( (rc = nav_indexer_next(fileStreamingHandle->indexer.context, &entry)) ) {
            BDBG_WRN(("%s: nav_indexer_next returned %d, error or EOF: autoRewind %d", __FUNCTION__, rc, fileStreamingHandle->settings.autoRewind));
            if (fileStreamingHandle->settings.autoRewind) {
                /* we rewind only in trickmodes */
                if (nav_indexer_rewind(fileStreamingHandle->indexer.context) < 0) {
                    BDBG_ERR(("%s: nav_indexer_rewind failed, returning failure", __FUNCTION__));
                    return false;
                }
                BKNI_Sleep(500);
                continue;
            }
            return false;
        }
        if (entry.insertPkt) {
            int bytesWritten;
            unsigned char *pkt;
            /* this is a special EOS packet that needs to be sent for AVC streams */
            entry.size = fileStreamingHandle->psi.transportTimeStampEnabled ? 192 : 188;
            {
                uint32_t ctrl;
                uint32_t discard;
                uint32_t t;
                uint32_t pts;

                memcpy(&ctrl,    &entry.pkt[12], sizeof ctrl);
                memcpy(&discard, &entry.pkt[40], sizeof discard);
                memcpy(&t,       &entry.pkt[44], sizeof t);
                memcpy(&pts,     &entry.pkt[48], sizeof pts);

                BDBG_MSG(("inserting special pkts: entry size %d, insert pkt %d, sync byte %x, byte5 %x, ctrl %x discard %x t %x pts %x",
                            (int)entry.size, (int)entry.insertPkt,
                            *(char *)entry.pkt,
                            *((char *)&entry.pkt[5]),
                            ntohl(ctrl),
                            ntohl(discard),
                            ntohl(t),
                            ntohl(pts)
                         ));
            }
            pkt = fileStreamingHandle->pkt;
            /* FOR DTCP/IP library, it needs a uncached address, but how to handle such pkt in memory */
            memcpy(pkt, entry.pkt, entry.size);
            bytesWritten = B_PlaybackIp_UtilsStreamingCtxWriteAll((struct bfile_io_write *)&fileStreamingHandle->data, pkt, entry.size);
            if (bytesWritten < (int)(entry.size)) {
                BDBG_ERR(("%s: write failed to insert %d bytes of special pkt for streaming session %p, wrote %d bytes, errno %d\n", __FUNCTION__, (int)entry.size, (void *)fileStreamingHandle, (int)bytesWritten, errno));
                fileStreamingHandle->connectionState = B_PlaybackIpConnectionState_eError;
                return false;
            }
            /* special pkt is sent, so goto to next entry */
            continue;
        }
        else {
            /* make the entry dio compatible */
            dioBeginOffset = entry.offset & ~DIO_MASK;
            *dioBeginOffsetAdjustment = entry.offset - dioBeginOffset;
            *dioEndOffsetAdjustment = DIO_BLK_SIZE - ( (entry.size+*dioBeginOffsetAdjustment) % DIO_BLK_SIZE);
            if (lseek(fileStreamingHandle->fd, dioBeginOffset, SEEK_SET) != dioBeginOffset) {
                BDBG_ERR(("%s: Failed to set the offset to %" PRId64 " for fd %d, errno %d\n", __FUNCTION__, dioBeginOffset, fileStreamingHandle->fd, errno));
                return false;
            }

            if (entry.size > fileStreamingHandle->streamingBufSize) {
                unsigned char *xbuf;
                B_PlaybackIp_UtilsFreeMemory(fileStreamingHandle->streamingBufOrig);
                if ((fileStreamingHandle->streamingBufOrig = B_PlaybackIp_UtilsAllocateMemory(entry.size + 4*DIO_BLK_SIZE, fileStreamingHandle->settings.heapHandle)) == NULL) {
                    BDBG_ERR(("%s: memory allocation failure at %d\n", __FUNCTION__, __LINE__));
                    fileStreamingHandle->streamingBuf = NULL;
                    return false;
                }
                xbuf = fileStreamingHandle->streamingBufOrig + DIO_BLK_SIZE;
                BDBG_MSG(("Re-allocated the streaming buf (new %p) to %d size", (void *)xbuf, (int)entry.size));
#ifndef DMS_CROSS_PLATFORMS
#ifndef ANDROID
#define ENABLED_DIO
#endif
#endif
#ifdef ENABLED_DIO
                *readBuf = DIO_ALIGN(xbuf);
#else
                *readBuf = xbuf;
#endif
                fileStreamingHandle->streamingBuf = *readBuf;
                fileStreamingHandle->streamingBufSize = entry.size;
            }

            *bytesToRead = entry.size + *dioBeginOffsetAdjustment + *dioEndOffsetAdjustment; /* need to read full entry size + any extra dio related bytes */
            *readBuf = fileStreamingHandle->streamingBuf;
            BDBG_MSG(("%s: file offset %"PRId64 "for entry offset %"PRId64 "dio offset begin %d, end %d, entry size %d, bytesToRead %d",
                        __FUNCTION__, dioBeginOffset, entry.offset, *dioBeginOffsetAdjustment, *dioEndOffsetAdjustment, (int)entry.size, *bytesToRead));
            return true;
        }
    }
    return false;
}

double difftime1(
    struct timeval *start,
    struct timeval *stop)
{
    double dt = (1000000.*(stop->tv_sec - start->tv_sec)) + 1.0*(stop->tv_usec - start->tv_usec);
    return dt;
}

static void
fileHttpStreamingThread(
    void *data
    )
{
    B_PlaybackIpFileStreamingHandle fileStreamingHandle = (B_PlaybackIpFileStreamingHandle)data;
    B_PlaybackIpFileStreamingOpenSettings *fileStreamingSettings;
    int bufSize;
    unsigned char *xbuf = NULL, *readBuf;
    int fd, streamingFd;
    off_t mpegHdrBeginOffset = 0;
    off_t dioBeginOffset = 0;
    off_t bytesToStream = 0;
    off_t bytesRemaining = 0;
    int dioBeginOffsetAdjustment = 0;
    int cryptoBeginOffsetAdjustment = 0;
    int dioEndOffsetAdjustment = 0;
    int bytesWritten, bytesToWrite, bytesRead, bytesToRead;
    unsigned int loopCount = 0;
    bool useIndexer = false;
#ifdef ENABLE_SW_PACING
    double rate;
    struct timeval start, stop;
    double dt=0, maxrate;
#endif
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    unsigned char *clearBuf = NULL;
#endif
#ifdef ENABLED_DIO
    bool dioRetry = false;
#endif

    BDBG_ASSERT(fileStreamingHandle);

    fileStreamingSettings = &fileStreamingHandle->settings;
    fd = fileStreamingHandle->fd;
    streamingFd = fileStreamingSettings->streamingFd;
#ifdef B_HAS_DTCP_IP
    if (B_PlaybackIp_UtilsDtcpServerCtxOpen(&fileStreamingHandle->settings.securitySettings, &fileStreamingHandle->data) != B_ERROR_SUCCESS) {
         BDBG_ERR(("Failed to setup the streaming context\n"));
         goto error;
    }
#endif

    if ((fileStreamingHandle->pktOrig = B_PlaybackIp_UtilsAllocateMemory(2*224, fileStreamingSettings->heapHandle)) == NULL) {
        BDBG_ERR(("%s: memory allocation failure at %d\n", __FUNCTION__, __LINE__));
        goto error;
    }
    fileStreamingHandle->pkt = fileStreamingHandle->pktOrig + 224;

    if (fileStreamingSettings->transportTimestampEnabled)
        bufSize = (TS_PKT_SIZE+4) * HTTP_AES_BLOCK_SIZE * STREAMING_BUF_MULTIPLE * 2;
    else
        bufSize = TS_PKT_SIZE * HTTP_AES_BLOCK_SIZE * STREAMING_BUF_MULTIPLE * 2;

    if ((fileStreamingHandle->streamingBufOrig = B_PlaybackIp_UtilsAllocateMemory(bufSize + 3*DIO_BLK_SIZE, fileStreamingSettings->heapHandle)) == NULL) {
        BDBG_ERR(("%s: memory allocation failure at %d\n", __FUNCTION__, __LINE__));
        goto error;
    }
    xbuf = fileStreamingHandle->streamingBufOrig + DIO_BLK_SIZE; /* saving space for residual bytes left in clear during previous encryption operation as there were not mod 16 sized */
#ifdef ENABLED_DIO
    readBuf = DIO_ALIGN(xbuf);
#else
    readBuf = xbuf;
#endif
    fileStreamingHandle->streamingBuf = readBuf;
    fileStreamingHandle->streamingBufSize = bufSize;
#ifdef BDBG_DEBUG_BUILD
    if (fileStreamingHandle->ipVerboseLog)
        BDBG_WRN(("handle %p, fd %d, streamingFd %d, streaming buf %p, orig %p, size %d", (void *)fileStreamingHandle, fd, streamingFd, (void *)fileStreamingHandle->streamingBuf, (void *)xbuf, bufSize));
#endif

#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    if (fileStreamingHandle->data.pvrDecKeyHandle) {
        NEXUS_MemoryAllocationSettings allocSettings;
        NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
        if (fileStreamingSettings->heapHandle)
            allocSettings.heap = fileStreamingSettings->heapHandle;
        if (NEXUS_Memory_Allocate(bufSize, &allocSettings, (void *)(&clearBuf))) {
            BDBG_ERR(("%s: memory allocation failure at %d\n", __FUNCTION__, __LINE__));
            goto error;
        }
    }
#endif

#ifdef ENABLED_DIO
    /* setup flag for DirectIO operations */
    if (fcntl(fd, F_SETFL, O_DIRECT) < 0) {
        BDBG_WRN(("%s: fcntl to set the O_DIRECT flag failed, fd %d, errno %d, ignore it", __FUNCTION__, fd, errno));
    }
#endif

    if (fileStreamingSettings->endFileOffset && fileStreamingSettings->playSpeed == 1) {
        if (fileStreamingSettings->transportTimestampEnabled)
            mpegHdrBeginOffset = fileStreamingSettings->beginFileOffset & ~(TS_PKT_SIZE+4);
        else
            mpegHdrBeginOffset = fileStreamingSettings->beginFileOffset & ~TS_PKT_SIZE;
        dioBeginOffset = fileStreamingSettings->beginFileOffset & ~DIO_MASK;
        BDBG_MSG(("dioBeginOffset %"PRId64 ", mpegHdrBeginOffset %"PRId64 , dioBeginOffset, mpegHdrBeginOffset));
        if (dioBeginOffset > mpegHdrBeginOffset) {
            /* dioBeginOffset is after than mpeg HDR offset, so it will not cover the mpeg header */
            /* we will need to go back another DIO block to include this mpeg block */
            dioBeginOffset -= DIO_BLK_SIZE;
            BDBG_MSG(("Adjust DIO offset to cover MPEG Hdr, dioBeginOffset %"PRId64, dioBeginOffset));
        }
        dioBeginOffsetAdjustment = fileStreamingSettings->beginFileOffset - dioBeginOffset;
        cryptoBeginOffsetAdjustment = mpegHdrBeginOffset - dioBeginOffset;
        BDBG_MSG(("dioBeginOffsetAdjustment %d, cryptoBeginOffsetAdjustment %d", dioBeginOffsetAdjustment, cryptoBeginOffsetAdjustment));
        if (lseek(fd, dioBeginOffset, SEEK_SET) != dioBeginOffset) {
            BDBG_ERR(("%s: Failed to set the offset to %"PRId64 "for fd %d, errno %d\n", __FUNCTION__, dioBeginOffset, fd, errno));
            goto error;
        }
        bytesToStream = fileStreamingSettings->endFileOffset - fileStreamingSettings->beginFileOffset + 1;
        BDBG_MSG(("offset: end %"PRId64 ", begin %"PRId64 " dio %"PRId64 ", bytesToStream %"PRId64 " for fd %d, streamingFd %d", fileStreamingSettings->endFileOffset, fileStreamingSettings->beginFileOffset, dioBeginOffset, bytesToStream, fd, streamingFd));
    }
    else if (fileStreamingHandle->indexFileFp &&
            (fileStreamingSettings->playSpeed != 1 ||   /* accounts for trick play cases */
            fileStreamingSettings->beginTimeOffset)     /* accounts for resuming to play from trick play state */
            ) {
        /* can seek using beginTimeOffset only if we have index file */
        BDBG_MSG(("%s: stream content at %d speed from time offset %0.3f using index file", __FUNCTION__, fileStreamingSettings->playSpeed, fileStreamingSettings->beginTimeOffset));
        if (setupMediaIndexerTrickMode(fileStreamingHandle, fileStreamingSettings->playSpeed, &fileStreamingHandle->psi) != true) {
            BDBG_ERR(("%s: Failed to use the index file to stream at %d speed for file streaming session %p", __FUNCTION__, fileStreamingSettings->playSpeed, (void *)fileStreamingHandle));
            goto error;
        }
        useIndexer = true;
    }
    else {
        /* re-position the offset to beginging as media probe may have moved it */
        lseek(fd, 0, SEEK_SET);
    }

    fileStreamingHandle->stop = false;
    fileStreamingHandle->data.stopStreaming = false;
    fileStreamingHandle->connectionState = B_PlaybackIpConnectionState_eActive;
    bytesRemaining = bytesToStream;
    fileStreamingHandle->threadRunning = true;

#ifdef ENABLE_SW_PACING
    gettimeofday(&start, NULL);
    maxrate = (double)fileStreamingHandle->psi.avgBitRate / 1024*1024.0;
#endif

    if (fileStreamingSettings->appHeader.valid) {
        BDBG_MSG(("%s: appHeader Enabled %d, length %d", __FUNCTION__, fileStreamingSettings->appHeader.valid, fileStreamingSettings->appHeader.length));
        BDBG_MSG(("data: %x data: %x data: %x data: %x ", fileStreamingSettings->appHeader.data[0], fileStreamingSettings->appHeader.data[1], fileStreamingSettings->appHeader.data[2], fileStreamingSettings->appHeader.data[3]));
        bytesWritten = B_PlaybackIp_UtilsStreamingCtxWriteAll((struct bfile_io_write *)&fileStreamingHandle->data, fileStreamingSettings->appHeader.data, fileStreamingSettings->appHeader.length);
        if (bytesWritten != (int)fileStreamingSettings->appHeader.length) {
            /* this happens if client closed the connection or client connection is dead */
            BDBG_MSG(("%s: handle: %p, failed to write %d bytes of app header data of len %d for fd %d", __FUNCTION__, (void *)fileStreamingHandle, bytesWritten, fileStreamingSettings->appHeader.length, streamingFd));
            fileStreamingHandle->connectionState = B_PlaybackIpConnectionState_eError;
            goto error;
        }
        BDBG_MSG(("%s: handle: %p, wrote %d bytes of app header data of len %d for fd %d", __FUNCTION__, (void *)fileStreamingHandle, bytesWritten, fileStreamingSettings->appHeader.length, streamingFd));
    }

    while (!fileStreamingHandle->stop) {
        readBuf = fileStreamingHandle->streamingBuf;
        if (useIndexer) {
            if (setReadOffset(fileStreamingHandle, &dioBeginOffsetAdjustment, &dioEndOffsetAdjustment, &readBuf, &bytesToRead) != true) {
                BDBG_WRN(("%s: setReadOffset failed, breaking out of streaming loop, streamed %"PRId64 " bytes in %d send calls", __FUNCTION__, fileStreamingHandle->totalBytesStreamed, loopCount));
                fileStreamingHandle->connectionState = B_PlaybackIpConnectionState_eEof;
                break;
            }
        }
        else {
            /* streaming directly from file */
            bytesToRead = bufSize;
        }

readagain:
        BDBG_MSG(("issuing read() ... fd %d, readBUf %p, bytesToRead %d\n", fd, (void *)readBuf, bytesToRead));
        if ((bytesRead = read(fd, readBuf, bytesToRead)) <= 0) {
            BDBG_MSG(("%s: read returned %d for fd %d, errno %d, autoRewind %d", __FUNCTION__, bytesRead, fd, errno, fileStreamingSettings->autoRewind));
            if (errno == EINTR) {
                BDBG_WRN(("%s: read Interrupted for fd %d, retrying errno %d", __FUNCTION__, fd, errno));
                continue;
            }
#ifdef ENABLED_DIO
            /* On some chips, even with O_DIRECT enabled, we get a read() error. If that happens, try the read again with O_DIRECT removed. */
            else if (errno == EFAULT) /* Bad address */ {
                int flags;
                if (dioRetry == false) {
                    dioRetry = true;

                    flags = fcntl(fd, F_GETFL);
                    if (flags != -1) {
                        flags &= ~O_DIRECT;
                        if (fcntl(fd, F_SETFL, flags)==0) {
                            BDBG_WRN(("Successfully removed O_DIRECT flag from fd %d after initial read() failure", fd ));
                            goto readagain;
                        }
                    } else {
                        BDBG_ERR(("Could not remove O_DIRECT flag"));
                    }
                }
                else
                {
                    BDBG_ERR(("%s: fd=%d dioRetry is false: errno=%d", __FUNCTION__, fd, errno));
                }
            }
#endif
            else {
                BDBG_WRN(("%s: read for fd %d; unexpected errno %d (%s) ", __FUNCTION__, fd, errno, strerror(errno) ));
            }
            if (bytesRead == 0 || errno == EINVAL) {
                /* reached file end */
                if (fileStreamingSettings->autoRewind) {
                    lseek(fd, 0, SEEK_SET);
                    BDBG_MSG(("%s: rewinding for fd %d\n", __FUNCTION__, fd));
                    continue;
                }
                else {
                    fileStreamingHandle->connectionState = B_PlaybackIpConnectionState_eEof;
                    BDBG_WRN(("%s: read(fd %d) errno %d is EINVAL(str=%s) or read 0 bytes bytesRead %lu ", __FUNCTION__, fd, errno, strerror(errno), (long unsigned int)bytesRead ));
                }
            }
            else {
                BDBG_WRN(("%s: read(fd %d) unexpected errno %d (%s); bytesRead %lu ", __FUNCTION__, fd, errno, strerror(errno), (long unsigned int)bytesRead ));
                /* read error case */
                fileStreamingHandle->connectionState = B_PlaybackIpConnectionState_eError;
            }
            break;
        }
        if (bytesRead != bytesToRead) {
            BDBG_MSG(("%s: bytesRead %d bytesToRead %d, sd %d", __FUNCTION__, bytesRead, bytesToRead, streamingFd));
        }
        /* determine bytes to write: account for DIO related adjustments from begining & end of the read buffer */
        bytesToWrite = bytesRead - dioBeginOffsetAdjustment - dioEndOffsetAdjustment;
        if (bytesToStream && bytesToWrite > bytesRemaining)
            bytesToWrite = bytesRemaining;
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
        if (fileStreamingHandle->data.pvrDecKeyHandle) {
            /* since we read from the DIO aligned offset (512), so this read is already AES block size (16) aligned. */
            /* decrypt the buffer */
            if (B_PlaybackIp_UtilsPvrDecryptBuffer(&fileStreamingHandle->data, readBuf+cryptoBeginOffsetAdjustment, clearBuf, bytesRead, &bytesRead) != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: PVR Decryption Failed", __FUNCTION__));
                break;
            }
            readBuf = clearBuf;
            BDBG_MSG(("%s: decrypting of %d bytes buffer done", __FUNCTION__, bytesRead));
        }
#endif
        bytesWritten = B_PlaybackIp_UtilsStreamingCtxWriteAll((struct bfile_io_write *)&fileStreamingHandle->data, readBuf+dioBeginOffsetAdjustment, bytesToWrite);
        if (bytesWritten <= 0) {
            /* this happens if client closed the connection or client connection is dead */
            if (fileStreamingHandle->ipVerboseLog)
                BDBG_ERR(("%s: failed to write %d bytes, fd %d, handle %p, wrote %d bytes, errno %d, streamed %"PRId64 " bytes in %d send calls", __FUNCTION__, bytesToWrite, streamingFd, (void *)fileStreamingHandle, bytesWritten, errno, fileStreamingHandle->totalBytesStreamed, loopCount));
            fileStreamingHandle->connectionState = B_PlaybackIpConnectionState_eError;
            break;
        }
        BDBG_MSG(("%s: wrote %d bytes for streaming fd %d, bytesToRead %d, bytesRead %d, dioBeginOffsetAdjustment %d bytes, sync %x\n",
                    __FUNCTION__, bytesWritten, streamingFd, bytesToRead, bytesRead, dioBeginOffsetAdjustment, *(readBuf+dioBeginOffsetAdjustment)));
        dioBeginOffsetAdjustment = 0;
        cryptoBeginOffsetAdjustment = 0;
        dioEndOffsetAdjustment = 0;
        fileStreamingHandle->totalBytesStreamed += bytesWritten;
        if (!fileStreamingSettings->autoRewind && bytesToStream && fileStreamingSettings->playSpeed == 1) {
            if (fileStreamingHandle->totalBytesStreamed >= bytesToStream) {
                BDBG_MSG(("%s: breaking from streaming loop: streamed %"PRId64 " bytes for streaming fd %d, asked %"PRId64 " bytes", __FUNCTION__, fileStreamingHandle->totalBytesStreamed, streamingFd, bytesToStream));
                fileStreamingHandle->connectionState = B_PlaybackIpConnectionState_eEof;
                break;
            }
            bytesRemaining -= bytesWritten;
        }
        loopCount++;
#ifdef ENABLE_SW_PACING
        gettimeofday(&stop, NULL);
        dt = difftime1(&start, &stop);
        rate = 8.*fileStreamingHandle->totalBytesStreamed/dt;
        while (rate > maxrate) {
            usleep(10000);
            gettimeofday(&stop, NULL);
            dt = difftime1(&start,&stop);
            rate = 8.*fileStreamingHandle->totalBytesStreamed/dt;
        }

        if ((loopCount % 50) == 0) {
            BDBG_MSG(("[%6lu] Wrote %10lu Bytes in dt = %12.2fusec at rate=%2.1f/%2.1f\n", loopCount, fileStreamingHandle->totalBytesStreamed, dt, rate, maxrate));
        }
#endif
#if 0
        /* commenting this out as it was causing stutter for streaming out high bitrate MVC streams. This code introduces delay in streaming out which causes underflow at the client side */
        /* the original intent of this code was to not starve out the local playback thread on server by allowing it to run during this sleep time */
        /* TODO: 07/16/2012: remove this code once it is vetted out in the next couple of months */

        if (fileStreamingSettings->playSpeed < 4 && fileStreamingSettings->playSpeed > 0 ) {
            /* sleep is needed at lower speeds otherwise client thread on the local VMS server gets starved */
            BKNI_Sleep(30);
        }
#endif
    }
error:
    BDBG_MSG(("%s: handle %p, streamed %"PRId64 " bytes for streaming fd %d in %d send calls", __FUNCTION__, (void *)fileStreamingHandle, fileStreamingHandle->totalBytesStreamed, streamingFd, loopCount));
#ifdef BDBG_DEBUG_BUILD
    if (fileStreamingHandle->ipVerboseLog)
        BDBG_WRN(("%s: %s: fd %d, handle %p, errno %d, streamed %"PRId64 " bytes in %d send calls", __FUNCTION__,
                    fileStreamingHandle->connectionState == B_PlaybackIpConnectionState_eEof ? "Reached EOF": "Send Error",
                    streamingFd, (void *)fileStreamingHandle, errno, fileStreamingHandle->totalBytesStreamed, loopCount));
#endif
    if (fileStreamingHandle->streamingBufOrig) B_PlaybackIp_UtilsFreeMemory(fileStreamingHandle->streamingBufOrig);
    if (fileStreamingHandle->pktOrig) B_PlaybackIp_UtilsFreeMemory(fileStreamingHandle->pktOrig);
    if (fileStreamingSettings->eventCallback && !fileStreamingHandle->stop) {
        /* app has defined eventCallback function & hasn't yet issued the stop, invoke the callback */
        BDBG_MSG(("%s: invoking End of Streaming callback for ctx %p", __FUNCTION__, (void *)fileStreamingHandle));
        fileStreamingHandle->connectionState = B_PlaybackIpConnectionState_eError;
        fileStreamingSettings->eventCallback(fileStreamingSettings->appCtx, B_PlaybackIpEvent_eServerErrorStreaming);
    }
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    if (clearBuf)
        NEXUS_Memory_Free(clearBuf);
#endif
    BKNI_SetEvent(fileStreamingHandle->stopStreamingEvent);
    fileStreamingHandle->threadRunning = false;
    return;
}

static void
fileRtpUdpStreamingThread(
    void *data
    )
{
    B_PlaybackIpFileStreamingHandle fileStreamingHandle = (B_PlaybackIpFileStreamingHandle)data;
    B_PlaybackIpFileStreamingOpenSettings *fileStreamingSettings;
    int bufSize;
    unsigned char *xbuf = NULL, *readBuf;
    int fd, streamingFd;
    int bytesWritten, bytesToWrite, bytesRead, bytesToRead;
    unsigned int loopCount = 0;
    double rate = 19.4;
    struct timeval start, stop;
    double dt=0, maxrate = 19.4;

    BDBG_ASSERT(fileStreamingHandle);
    fileStreamingSettings = &fileStreamingHandle->settings;
    fileStreamingHandle->threadRunning = true;
    fd = fileStreamingHandle->fd;
    streamingFd = fileStreamingSettings->streamingFd;

    bufSize = STREAMING_BUF_SIZE + 2*DIO_BLK_SIZE;
    if ((fileStreamingHandle->streamingBufOrig = B_PlaybackIp_UtilsAllocateMemory(bufSize, fileStreamingSettings->heapHandle)) == NULL) {
        BDBG_ERR(("%s: memory allocation failure at %d\n", __FUNCTION__, __LINE__));
        goto error;
    }
    xbuf = fileStreamingHandle->streamingBufOrig + DIO_BLK_SIZE; /* saving space for residual bytes left in clear during previous encryption operation as there were not mod 16 sized */
#ifdef ENABLED_DIO_UDP
    readBuf = DIO_ALIGN(xbuf);
    /* setup flag for DirectIO operations */
    if (fcntl(fd, F_SETFL, O_DIRECT) < 0) {
        BDBG_WRN(("%s: fcntl to set the O_DIRECT flag failed, fd %d, errno %d, ignore it", __FUNCTION__, fd, errno));
    }
#else
    readBuf = xbuf;
#endif
    fileStreamingHandle->streamingBuf = readBuf;
    fileStreamingHandle->streamingBufSize = STREAMING_BUF_SIZE;

    /* reset the file pointer */
    lseek(fd, 0, SEEK_SET);
    fileStreamingHandle->stop = false;
    fileStreamingHandle->connectionState = B_PlaybackIpConnectionState_eActive;

    maxrate = (double)fileStreamingHandle->psi.avgBitRate / (1000.0*1000);
    if (maxrate == 0) maxrate = 14.0; /* just some number */
    gettimeofday(&start, NULL);
    BDBG_WRN(("%s: handle %p, fd %d, streamingFd %d, streaming buf %p, orig %p, streaming bitrate %.1f (bits/milli-sec)",
                __FUNCTION__, (void *)fileStreamingHandle, fd, streamingFd, (void *)fileStreamingHandle->streamingBuf, (void *)xbuf, maxrate));
#ifdef ENABLED_DIO_UDP
    bytesToRead = ((512 * (188>>2) * 7));
#else
    bytesToRead = 1316 * 48;
#endif
    gettimeofday(&start, NULL);
    while (!fileStreamingHandle->stop) {
        BDBG_MSG(("fd %d, readBuf %p, bytesToRead %d\n", fd, (void *)readBuf, bytesToRead));
        /* streaming directly from file */
        if ((bytesRead = read(fd, readBuf, bytesToRead)) <= 0) {
            BDBG_WRN(("%s: read returned %d for fd %d, errno %d, autoRewind %d", __FUNCTION__, bytesRead, fd, errno, fileStreamingSettings->autoRewind));
            if (errno == EINTR) {
                BDBG_WRN(("%s: read Interrupted for fd %d, retrying errno %d\n", __FUNCTION__, fd, errno));
                continue;
            }
            if (bytesRead == 0 || errno == EINVAL) {
                /* reached file end */
                if (fileStreamingSettings->autoRewind) {
                    lseek(fd, 0, SEEK_SET);
                    BDBG_WRN(("%s: rewinding for fd %d\n", __FUNCTION__, fd));
                    continue;
                }
                else {
                    fileStreamingHandle->connectionState = B_PlaybackIpConnectionState_eEof;
                }
            }
            else {
                perror("read:");
            }
            /* read error case */
            fileStreamingHandle->connectionState = B_PlaybackIpConnectionState_eError;
            break;
        }
        if (bytesRead != bytesToRead) {
            BDBG_MSG(("%s: bytesRead %d bytesToRead %d, sd %d", __FUNCTION__, bytesRead, bytesToRead, streamingFd));
        }
        /* determine bytes to write: account for DIO related adjustments from begining & end of the read buffer */
        bytesToWrite = bytesRead;
        bytesWritten = B_PlaybackIp_UtilsStreamingCtxWriteAll((struct bfile_io_write *)&fileStreamingHandle->data, readBuf, bytesToWrite);
        if (bytesWritten < bytesToWrite) {
            BDBG_MSG(("write failed to write %d bytes for streaming fd %d, wrote %d bytes, errno %d\n", bytesToWrite, streamingFd, bytesWritten, errno));
            fileStreamingHandle->connectionState = B_PlaybackIpConnectionState_eError;
            break;
        }
        BDBG_MSG(("%s: wrote %d bytes for streaming fd %d, bytesToRead %d, bytesRead %d, totalBytesWritten %lu",
                    __FUNCTION__, bytesWritten, streamingFd, bytesToRead, bytesRead, (long unsigned int) fileStreamingHandle->totalBytesStreamed));
        fileStreamingHandle->totalBytesStreamed += bytesWritten;
        loopCount++;

        gettimeofday(&stop, NULL);
        dt = difftime1(&start, &stop);
        rate = 8.*fileStreamingHandle->totalBytesStreamed/dt;
        if ((loopCount % 100) == 0) {
            BDBG_MSG(("[%6lu] Wrote %10lu Bytes in dt = %12.2fusec at rate=%2.1f/%2.1f\n", (long unsigned int)loopCount, (long unsigned int)fileStreamingHandle->totalBytesStreamed, dt, rate, maxrate));
        }
        while (rate > maxrate) {
            usleep(10000);
            gettimeofday(&stop, NULL);
            dt = difftime1(&start,&stop);
            rate = 8.*fileStreamingHandle->totalBytesStreamed/dt;
        }

    }
error:
    BKNI_Sleep(500);
    BDBG_WRN(("%s: handle %p, streamed %"PRId64 " bytes for streaming fd %d in %d send calls", __FUNCTION__, (void *)fileStreamingHandle, fileStreamingHandle->totalBytesStreamed, streamingFd, loopCount));
    BDBG_WRN(("[%6lu] Wrote %10lu Bytes in dt = %12.2fusec at rate=%2.1f/%2.1f\n", (long unsigned int)loopCount, (long unsigned int)fileStreamingHandle->totalBytesStreamed, dt, rate, maxrate));
    if (fileStreamingHandle->streamingBufOrig) B_PlaybackIp_UtilsFreeMemory(fileStreamingHandle->streamingBufOrig);
    BKNI_SetEvent(fileStreamingHandle->stopStreamingEvent);
    fileStreamingHandle->threadRunning = false;
    return;
}

/***************************************************************************
Summary:
This function starts streaming content from a file.
***************************************************************************/
B_PlaybackIpError
B_PlaybackIp_FileStreamingStart(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle
    )
{
    B_PlaybackIpFileStreamingOpenSettings *fileStreamingSettings = &fileStreamingHandle->settings;
    void (*threadFunc)(void *);

    fileStreamingHandle->data.streamingProtocol = fileStreamingSettings->protocol;
    switch (fileStreamingSettings->protocol) {
        case B_PlaybackIpProtocol_eHttp: {
            fileStreamingHandle->data.fd = fileStreamingSettings->streamingFd;
            fileStreamingHandle->data.self = net_io_data_write;
            if (B_PlaybackIp_UtilsStreamingCtxOpen(&fileStreamingHandle->data) != B_ERROR_SUCCESS) {
                BDBG_ERR(("Failed to setup the streaming context\n"));
                return B_ERROR_UNKNOWN;
            }
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
            if (B_PlaybackIp_UtilsPvrDecryptionCtxOpen(&fileStreamingSettings->securitySettings, &fileStreamingHandle->data) != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: Failed to setup pvr decryption", __FUNCTION__));
                return B_ERROR_UNKNOWN;
            }
#endif
            threadFunc = fileHttpStreamingThread;
            BDBG_MSG(("%s: complete, streaming socket %d\n", __FUNCTION__, fileStreamingHandle->data.fd));
            break;
        }
        case B_PlaybackIpProtocol_eRtp:
        case B_PlaybackIpProtocol_eUdp: {
            fileStreamingHandle->data.interfaceName = fileStreamingSettings->rtpUdpSettings.interfaceName;
            fileStreamingHandle->data.self = net_io_data_write;
            fileStreamingHandle->data.streamingSockAddr.sin_family = AF_INET;
            fileStreamingHandle->data.streamingSockAddr.sin_port = htons(fileStreamingSettings->rtpUdpSettings.streamingPort);
            if (inet_aton(fileStreamingSettings->rtpUdpSettings.streamingIpAddress, &fileStreamingHandle->data.streamingSockAddr.sin_addr) == 0) {
                BDBG_ERR(("%s: inet_aton() failed on %s", __FUNCTION__, fileStreamingSettings->rtpUdpSettings.streamingIpAddress));
                goto error;
            }
            BDBG_WRN(("Streaming URL is %s%s:%d", fileStreamingSettings->protocol == B_PlaybackIpProtocol_eRtp? "rtp://":"udp://",
                        inet_ntoa(fileStreamingHandle->data.streamingSockAddr.sin_addr), ntohs(fileStreamingHandle->data.streamingSockAddr.sin_port)));

            if (B_PlaybackIp_UtilsRtpUdpStreamingCtxOpen(&fileStreamingHandle->settings.securitySettings, &fileStreamingHandle->data) != B_ERROR_SUCCESS) {
                BDBG_ERR(("Failed to setup the streaming context\n"));
                goto error;
            }
            B_PlaybackIp_UtilsSetRtpPayloadType(fileStreamingHandle->psi.mpegType, &fileStreamingHandle->data.rtpPayloadType);
            threadFunc = fileRtpUdpStreamingThread;
            BDBG_MSG(("%s: complete for RTP/UDP streaming, socket %d\n", __FUNCTION__, fileStreamingHandle->data.fd));
            break;
        }
        default:
            BDBG_ERR(("%s: non-supported protocol %d", __FUNCTION__, fileStreamingSettings->protocol));
            goto error;
    }

    if (BKNI_CreateEvent(&fileStreamingHandle->stopStreamingEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", __FUNCTION__));
        goto error;
    }
    fileStreamingHandle->streamingThread = B_Thread_Create("HTTP Streamer", (B_ThreadFunc)threadFunc, (void *)fileStreamingHandle, NULL);
    if (fileStreamingHandle->streamingThread == NULL) {
        BDBG_ERR(("%s: Failed to create HTTP media file streaming thread \n", __FUNCTION__));
        goto error;
    }
    return B_ERROR_SUCCESS;

error:
#ifdef B_HAS_DTCP_IP
    B_PlaybackIp_UtilsDtcpServerCtxClose(&fileStreamingHandle->data);
#endif
    B_PlaybackIp_UtilsStreamingCtxClose(&fileStreamingHandle->data);
    return B_ERROR_UNKNOWN;
}


/***************************************************************************
Summary:
This function stops streaming content from a file.
***************************************************************************/
void
B_PlaybackIp_FileStreamingStop(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle
    )
{
    BERR_Code rc;
    BDBG_MSG(("%s: %p", __FUNCTION__, (void *)fileStreamingHandle));
    BDBG_ASSERT(fileStreamingHandle);
    fileStreamingHandle->stop = true;
    fileStreamingHandle->data.stopStreaming = true;

    if (fileStreamingHandle->stopStreamingEvent) {
        if (fileStreamingHandle->threadRunning) {
            rc = BKNI_WaitForEvent(fileStreamingHandle->stopStreamingEvent, 3000);
            if (rc == BERR_TIMEOUT)
                BDBG_WRN(("%s: stopStreamingEvent timed out", __FUNCTION__));
            else
            if (rc != 0) {
                BDBG_ERR(("%s: failed to stop the file streaming thread", __FUNCTION__));
            }
            fileStreamingHandle->threadRunning = false;
        }
        BKNI_DestroyEvent(fileStreamingHandle->stopStreamingEvent);
    }
    switch (fileStreamingHandle->settings.protocol) {
        case B_PlaybackIpProtocol_eRtp:
        case B_PlaybackIpProtocol_eUdp: {
            B_PlaybackIp_UtilsRtpUdpStreamingCtxClose(&fileStreamingHandle->data);
            break;
        }
        case B_PlaybackIpProtocol_eHttp:
        default:
            break;
    }
    if (fileStreamingHandle->streamingThread) {
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
        B_PlaybackIp_UtilsPvrDecryptionCtxClose(&fileStreamingHandle->data);
#endif
#ifdef B_HAS_DTCP_IP
        B_PlaybackIp_UtilsDtcpServerCtxClose(&fileStreamingHandle->data);
#endif
        B_PlaybackIp_UtilsStreamingCtxClose(&fileStreamingHandle->data);
        B_Thread_Destroy(fileStreamingHandle->streamingThread);
        fileStreamingHandle->streamingThread = NULL;
    }
    return;
}

void
B_PlaybackIp_FileStreamingGetStatus(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle,
    B_PlaybackIpFileStreamingStatus *status
    )
{
    BDBG_ASSERT(status);
    BDBG_ASSERT(fileStreamingHandle);
    status->bytesStreamed = fileStreamingHandle->totalBytesStreamed;
    status->connectionState = fileStreamingHandle->connectionState;
}


#endif /* LINUX || VxWorks */
