/***************************************************************************
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
 *
 * Module Description: Transport Stream Index Player
 *
 ***************************************************************************/
#ifndef USE_LEGACY_KERNAL_INTERFACE
/* Magnum debug interface */
#include "bstd.h"
#include "bchp.h"
BDBG_MODULE(bcmplayer);
#else
/* Original debug interface */
#define BRCM_DBG_MODULE_NAME "bcmplayer"
#include "brcm_dbg.h"
#include "brcm_t.h" /* uint64_t */
#define BCHP_VER_C0 (0x00020000)

/* emulate magnum */
#define BDBG_ERR(X) BRCM_DBG_ERR(X)
#define BDBG_WRN(X) BRCM_DBG_WRN(X)
#define BDBG_MSG(X) BRCM_DBG_MSG(X)
#define BDBG_ASSERT(COND) \
    do {if (!(COND)) BDBG_ERR(("BDBG_ASSERT failed on %d", __LINE__)); } while (0)
#define BSTD_UNUSED(X)
#endif

#include "bcmindexer.h"
#include "bcmindexerpriv.h"
#include "bcmplayer.h"
#include "mpeg2types.h"
#include "bcmplayer_impl.h"
#include "bcmplayer_svc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define i_abs(X) ((X)<0?-(X):(X))

#define DEFAULT_CACHE_SIZE          100
#define BTP_PACKET_DATA_SIZE        10
#define MAX_NUM_FIFO_ENTRIES        250  /* set this to 4 x MAX GOP length if get "bcmplayer ERROR: fifo full!" errors */
#define MAX_REFERENCE_FRAME_OFFSET  30
#define DEFAULT_NORMAL_PLAY_BUFFER_SIZE (188*2*1024) /* enough to handle 720p, but should be tuned */

#define BDBG_MSG_PTSCACHE(X) /* BDBG_MSG(X) */

/*
   Set LOOSE_INDEX_CHECKING if you want bcmplayer to ignore certain errors found in the nav file.
   Disabled by default, as it is better to identify the source of the errors than ignore them.
*/
/* #define LOOSE_INDEX_CHECKING       1 */

/**
* There are numerous places where functions are called, and if they return
* NULL, an error message is generated and the function returns immediately with
* -1. This macro does that.
**/
#define CHECKREAD(READ) \
    if (!(READ)) { \
        BDBG_ERR(("%s failed on line %d", #READ, __LINE__)); \
        return -1; \
    }
#define CHECKREAD_SILENT(READ) \
    if (!(READ)) { \
        return -1; \
    }

typedef enum
{
    eBpInsertNone = 0,  /* No inserted packet. This is used for data that must be read from file. */
    eBpInsertBTP,       /* Broadcom Transport Packet, used for Broadcom trick modes and delimited data */
    eBpInsertDummy,     /* one packet picture, used for advancing the decoder pipeline but not being displayed */
    eBpInsertNULL,      /* NULL packet (pid 0x1fff) */
    eBpInsertStartCode, /* insert packet with single start code */
    TotalPacketTypes
} eBpInsertPacketType;

typedef struct BNAV_sPktFifoEntry
{
    eBpInsertPacketType insertpackettype;
    unsigned long pktdata[BTP_PACKET_DATA_SIZE]; /* BTP payload */
    uint64_t startByte;
    uint64_t endByte;
    unsigned long zeroByteCountBegin;
    unsigned long zeroByteCountEnd;
    uint16_t startcode;
    uint16_t pid;
    struct BNAV_sPktFifoEntry *next;
} BNAV_PktFifoEntry;

struct BNAV_Player_P_PtsCacheEntry {
    uint32_t pts;
    long index;
    unsigned sectionCounter; /* on a PTS discontinuity, this is incremented */
};

struct BNAV_Player_P_PtsCache {
    /* PTS cache size is equal to number of pictures that can be stored in playback and video fifos.
    Worst case is very large fifo, very low bit rate stream and a high frame rate.
    If fifos are 3 MB, stream is 1Mbps and frame rate is 60fps, pts cache size will be 1440. */
#define PTS_CACHE_SIZE 600
/* set upper bound in case app is not calling BNAV_Player_DiscardToPts */
#define MAX_PTS_CACHE_SIZE (600*8)
    struct BNAV_Player_P_PtsCacheEntry *cache;
    unsigned cacheSize;
    unsigned wptr;               /* index to write next entry to fifo */
    unsigned rptr;               /* index of oldest entry in the fifo, beginning of a "backwards search" */
    bool doBackwardPtsSearch;    /* If set search backwards instead of forwards. this means starting the search from the most recently added PTS,
                                    which is not typical. the typical usage is to search from the oldest PTS, the one likely to be the decoder's current PTS. */
    unsigned sectionCounter;
};

struct BNAV_Player_HandleImpl
{
    BNAV_Player_Settings settings;

    /* callbacks */
    BP_READ_CB          readCb;
    BP_TELL_CB          tellCb;
    BP_SEEK_CB          seekCb;
    BP_BOUNDS_CB        boundsCb;
    void                *filePointer;

    /* nav cache */
    unsigned char       *navCache;
    long                navCacheIndexStart;
    long                navCacheIndexEnd;

    /* nav cache metrics */
    long                navCacheMisses;
    long                navCacheReads;
    long                navCacheFails;

    /* nav cache parameters */
    int                 navCacheSize;    /* size in entries */
    int                 navCacheBytes;   /* size in bytes */
    int                 navCacheOverlap; /* hardcoded to 1/4 of navCacheSize for now */

    /* file format parameters */
    long                navFileIndexEntryStart;
    long                navFileIndexEntrySize;  /* size of each record in NAV table. see BNAV_GetEntrySize */

    /* state */
    eBpPlayModeParam    playMode;
    eBpDirectionParam   playDir;
    eBpLoopModeParam    loopMode;
    long                currentIndex;
    int                 skipCount;     /* used for eBpPlaySkipB and eBpPlaySkipP */

    unsigned            advanceCount;  /* Number of frames to advance each
                                          picture (1=Normal) */
    unsigned short      pid;           /* Current video PID */
    char                gotEntry;      /* indicates that getNextEntry got an entry. */
    long                firstIndex;
    long                lastIndex;

    BNAV_PktFifoEntry   pktfifo[MAX_NUM_FIFO_ENTRIES];
    long                firstFifoEntry;
    long                lastFifoEntry;
    BNAV_PktFifoEntry   *pktfifo_tail;
    int                 curRefFrame;
    int                 skipNextAdvance;
    int                 iFrameRepeatCount;
    uint64_t            lastSeqHdrOffset64;
    int                 disableExtraBOptimization; /* see note in playertypes.h */
    BNAV_DecoderFeatures    decoderFeatures;
    BNAV_Version        navVersion;
    int                 useReferenceFrame;

    /* used for normal playback */
    uint64_t currentOffset;
    unsigned long normalPlayBufferSize;
    BNAV_Player_DebugMode debugMode;

    int                 pastPred, futurePred;

    int timestampOffset;    /* 0 or 4 */
    unsigned packetSize;    /* 188 or 192 */

    /* Data needed to interrupt and resume BNAV_Player_BuildReferenceFrame_Process */
    struct {
        int inProcess; /* if true, haven't finished reference frame */
        int needDisplayPicture;/* if true, haven't sent display frame that necessitated the reference frame */
        long curOffset;
        unsigned long lastFrameSize;
        long entry;
    } buildRef;


    struct BNAV_Player_P_PtsCache ptscache;

    /* used for eBpPlayDecoderGOPTrick, GOPIPTrick, MDqtTrick */
    struct {
        int currentStart, currentEnd;
        int pictureTag;
        int gopSkip; /* how many gops to skip between advances */
        unsigned count; /* total number of pictures sent for this GOP. limited by advanceCount. */

        /* mp dqt */
        int intraGopPictureIndex; /* -1 means waiting */
        unsigned openGopPictures;
        enum {
            dqt_state_ready,
            dqt_state_started,
            dqt_state_sent
        } dqt_state;
    } gopTrick;

    struct {
        bool set;
        unsigned firstTimestamp;
        unsigned pics;
    } timeSkip;
    BNAV_Player_SVC_HandleImpl svc;
};

/*---------------------------------------------------------------
- PRIVATE static FUNCTIONS
---------------------------------------------------------------*/

static void BNAV_Player_SetCacheSize(BNAV_Player_Handle handle, int cacheSize);
static int BNAV_Player_AddNormalPlay(BNAV_Player_Handle handle);
static int BNAV_Player_AddNormalPlayByFrames(BNAV_Player_Handle handle);
static int BNAV_Player_AddFrame(BNAV_Player_Handle handle, long entry);
static int BNAV_Player_AddFrameToFifo(BNAV_Player_Handle handle, long entry);
static int BNAV_Player_AdvanceIndex(BNAV_Player_Handle handle);
static BNAV_PktFifoEntry  *BNAV_Player_AddFifoEntry(BNAV_Player_Handle handle, unsigned pid);
static int BNAV_Player_AddSequenceHeader(BNAV_Player_Handle handle, BNAV_Entry *pStartEntry,
    int btpMode, int display, int skip);
static int BNAV_Player_AddFrameData(BNAV_Player_Handle handle, BNAV_Entry *pStartEntry);
static long BNAV_Player_P_FindPictureIndex(BNAV_Player_Handle handle, long startIndex, eSCType picCode, eBpDirectionParam dir);
static int  BNAV_Player_P_AddCurrentToPTSCache(BNAV_Player_Handle handle);
static void BNAV_Player_P_FlushPTSCache(BNAV_Player_Handle handle);


/* generate commands */
static void BNAV_Player_GenerateBTP(BNAV_Player_Handle handle, unsigned long *params, unsigned char *pkt, int discarding_tailend, unsigned pid);
static void BNAV_Player_GenerateDummyPicture(BNAV_Player_Handle handle, unsigned char *pkt, unsigned pid);
static void BNAV_Player_GenerateNullPacket(BNAV_Player_Handle handle, unsigned char *pkt);
static void BNAV_Player_GenerateStartCodePacket(BNAV_Player_Handle handle, unsigned char *pkt, uint16_t startcode, uint16_t pid);

static int BNAV_Player_GetFifoSize(const BNAV_Player_Handle handle);
static int BNAV_Player_BuildReferenceFrame_Process(BNAV_Player_Handle handle);

static int BNAV_Player_AddDummyFrame(BNAV_Player_Handle handle, BNAV_Entry *pStartEntry);
static int BNAV_Player_p_SanityCheck(BNAV_Player_Handle handle);

/* 64 bit support */
#define getLo64(X) (unsigned long)((X)&0xFFFFFFFF)
#define getHi64(X) (unsigned long)((X)>>32)
#define create64(HI,LO) ((((uint64_t)(HI))<<32)|(unsigned long)(LO))

/*! Function Definitions */

/**
* Auto-detects the NAV table version. If successful, the version is set in bcmplayer's
* state, and optionally returned through the version parameter (if not NULL).
*
* If BNAV_Player_detectNavTableVersion() is not called before
* the first call to BNAV_Player_getNextPlayEntry(), it will be called
* automatically.
*
* The reason it is not called by BNAV_Player_Allocate() and BNAV_Player_reset()
* is that the user may want to use a NULL boundsCb and assert the firstIndex and
* lastIndex. This must be done before auto-detecting the version.
*
* Return values:
* -1 on failure
* 0 on success
**/
static int BNAV_Player_DetectNavTableVersion(BNAV_Player_Handle handle, int isPes);

/* Important: when building general-purpose binaries, always default to the
* lowest common denominator feature set. */
static const BNAV_DecoderFeatures g_defaultDecoderFeatures = {1,1,0,0};

void BNAV_Player_GetDefaultSettings(BNAV_Player_Settings *settings)
{
    memset(settings, 0, sizeof(*settings));
    settings->lastIndex = 0x7FFFFFFF; /* this allows autodetect to work, but
        you might get lots of error messages from readEntry(). */
    memcpy(&settings->decoderFeatures, &g_defaultDecoderFeatures, sizeof(g_defaultDecoderFeatures));
    settings->cacheSize = DEFAULT_CACHE_SIZE;
    settings->normalPlayBufferSize = DEFAULT_NORMAL_PLAY_BUFFER_SIZE;
    settings->debugMode = BNAV_Player_DebugFramesAndAllPacketsSent;
    settings->navVersion = BNAV_VersionUnknown;
    settings->boundsCb = BNAV_Player_DefaultGetBounds;
    settings->useReferenceFrame = 1; /* true */
    settings->useBtpsForHostTrickModes = true;
}

int BNAV_Player_Open(BNAV_Player_Handle *handle, const BNAV_Player_Settings *settings)
{
    int rc;
    *handle = (BNAV_Player_Handle)malloc(sizeof(struct BNAV_Player_HandleImpl));
    if(*handle==NULL) {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    memset(*handle, 0, sizeof(**handle));

    (*handle)->ptscache.cacheSize = PTS_CACHE_SIZE;
    (*handle)->ptscache.cache = malloc((*handle)->ptscache.cacheSize * sizeof(struct BNAV_Player_P_PtsCacheEntry));
    if (!(*handle)->ptscache.cache) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }

    rc = BNAV_Player_Reset(*handle, settings);
    if (rc) {
        BERR_TRACE(rc);
        goto error;
    }

    return 0;

error:
    BNAV_Player_Close(*handle);
    *handle = NULL;
    return -1;
}

int BNAV_Player_Reset(BNAV_Player_Handle handle, const BNAV_Player_Settings *settings)
{
    int result = 0;

    if (!settings->readCb ||
        !settings->seekCb ||
        !settings->tellCb ||
        !settings->filePointer ||
        !settings->cacheSize)
    {
        BDBG_ERR(("Missing required BNAV_Player_Settings"));
        return -1;
    }

    if (!settings->videoPid) {
        BDBG_ERR(("Video PID required"));
        return -1;
    }

    /* TODO: we should replace handle->x with handle->settings.x throughout this code. */
    handle->settings = *settings;

    handle->readCb  = settings->readCb;
    handle->seekCb  = settings->seekCb;
    handle->tellCb  = settings->tellCb;
    handle->boundsCb    = settings->boundsCb;
    handle->filePointer = settings->filePointer;

    handle->navFileIndexEntrySize = BNAV_GetEntrySize(settings->navVersion);
    handle->navFileIndexEntryStart = 0;
    handle->playMode            = eBpPlayNormal;
    handle->playDir             = eBpForward;
    handle->advanceCount        = 1;
    handle->loopMode            = eBpSinglePlay;
    handle->currentIndex        = 0;
    handle->pid                 = settings->videoPid;
    handle->firstFifoEntry      = 0;
    handle->lastFifoEntry       = -1;
    handle->curRefFrame         = -1; /* 0 is a valid refframe */
    handle->skipNextAdvance     = 0;
    handle->currentOffset       = 0;
    handle->normalPlayBufferSize    = settings->normalPlayBufferSize;
    handle->debugMode           = settings->debugMode;
    handle->navVersion          = settings->navVersion;
    handle->useReferenceFrame   = settings->useReferenceFrame;
    memcpy(&handle->decoderFeatures, &settings->decoderFeatures, sizeof(settings->decoderFeatures));
    handle->firstIndex          = settings->firstIndex;
    handle->lastIndex           = settings->lastIndex;

    handle->gopTrick.currentStart = -1;
    handle->gopTrick.pictureTag = 0;

    BNAV_Player_P_FlushPTSCache(handle);

    if (settings->transportTimestampEnabled) {
        handle->timestampOffset = 4;
        handle->packetSize = 192;
    }
    else {
        handle->timestampOffset = 0;
        handle->packetSize = 188;
    }

    /* videoPid is required for Broadcom trick modes. */
    if (!settings->videoPid)
        handle->decoderFeatures.supportsBroadcomTrickModes = 0;

    handle->disableExtraBOptimization = 0;

    /* this only reallocates the cache if cacheSize or size changes. */
    BNAV_Player_SetCacheSize(handle, settings->cacheSize);

    if (handle->navVersion == BNAV_VersionUnknown) {
        long lastIndex = handle->lastIndex;
        if (handle->lastIndex == -1) {
            /* if no bounds, set temp bounds just for autodetection */
            handle->lastIndex = 0x7FFFFFFF;
        }
        result = BNAV_Player_DetectNavTableVersion(handle, settings->isPes);
        handle->lastIndex = lastIndex; /* restore */
        if (result) {
            BDBG_ERR(("Unable to detect BNAV_Version"));
            return -1;
        }

        /* reset cache for changes */
        handle->navFileIndexEntrySize = BNAV_GetEntrySize(handle->navVersion);
        BNAV_Player_SetCacheSize(handle, settings->cacheSize);
    }
    if (handle->navVersion >= BNAV_VersionUnknown) {
        BDBG_ERR(("Unknown BNAV_Version: %d", handle->navVersion));
        BDBG_ERR(("Hint: Indexes should be in host endianness. If the index is byteswapped, the first symptom is a bad version."));
        return -1;
    }
    if (handle->navVersion == 0) {
        /* There were a couple months where version 1 indexes were being produced
        but we weren't labelling the indexes as such. And a false detect is really
        bad. If you need to use version 0 indexes and you know that they really
        are correct, feel free to delete this check, but by default it should fail. */
        BDBG_ERR(("Unable to support version 0 indexes reliably."));
        BDBG_ERR(("Hint: Indexes should be in host endianness. If the index is byteswapped, the first symptom is a bad version."));
        return -1;
    }
    if (handle->navVersion == BNAV_Version_VC1_PES) {
        /* PES data, no packets */
        handle->timestampOffset = 0;
        handle->packetSize = 0;
    }

/* no sanity check for otf */
    if (!settings->skipSanityCheck && BNAV_Player_p_SanityCheck(handle)) {
        BDBG_ERR(("Internal check of bcm index failed. This is an invalid index. "
        "Is it the correct endianness? "
        "Please use the printindex utility to verify its integrity."));
        return -1;
    }
    result = BNAV_Player_SVC_Reset(&handle->svc, handle);

    return result;
}

void BNAV_Player_Close(BNAV_Player_Handle handle)
{
    if (handle->ptscache.cache) {
        free(handle->ptscache.cache);
    }
    if (handle->navCache) {
        free(handle->navCache);
    }
    free(handle);
}

static int BNAV_Player_UpdateBounds(BNAV_Player_Handle handle)
{
    int rc = 0;

    if (handle->firstIndex != -1 && handle->lastIndex != -1 && !handle->settings.changingBounds) {
        /* we know bounds and they are unchanging */
        return 0;
    }

    /* if installed, call the callback to update the bounds */
    if (handle->boundsCb) {
        long firstIndex, lastIndex;
        rc = (*handle->boundsCb)(handle, handle->filePointer, &firstIndex, &lastIndex);
        if (!rc) {
            /* if assigned to -1, default will supplement */
            handle->firstIndex = firstIndex;
            handle->lastIndex = lastIndex;
        }
    }

    /* if either firstIndex or lastIndex is -1, we need default bounds */
    if (handle->firstIndex == -1 || handle->lastIndex == -1) {
        long firstIndex, lastIndex;
        rc = BNAV_Player_DefaultGetBounds(handle, handle->filePointer, &firstIndex, &lastIndex);
        if (!rc) {
            if (handle->firstIndex == -1) {
                handle->firstIndex = firstIndex;
            }
            if (handle->lastIndex == -1) {
                handle->lastIndex = lastIndex;
            }
        }
    }

    return rc;
}

/**
* This is the only place where the seekCb, tellCb and readCb callbacks are used.
* The boundsCb calback is used by BNAV_Player_UpdateBounds().
**/
BNAV_Entry *BNAV_Player_ReadEntry(BNAV_Player_Handle handle, long index)
{
    if (BNAV_Player_UpdateBounds(handle)) {
        return NULL;
    }

    /* Test again, if we're still not in range then we're past the end of the file -- done! */
    if (index < handle->firstIndex || index > handle->lastIndex) {
        BDBG_MSG(("BNAV_Player_ReadEntry failed, index %d, firstIndex %d, lastIndex %d",
            (int)index, (int)handle->firstIndex, (int)handle->lastIndex));
        return NULL;
    }

    /* Check if this naventry is in our cache */
    if ((handle->navCacheIndexStart == -1)
        || !((index >= handle->navCacheIndexStart) && (index < handle->navCacheIndexEnd)))
    {
        unsigned long saveStart = handle->navCacheIndexStart;
        unsigned long saveEnd = handle->navCacheIndexEnd;
        unsigned long pos;
        long t;

        if (index < handle->navCacheIndexStart)
        {
            /* We are reading indexes backwards so cache backwards */
            handle->navCacheIndexStart = index - handle->navCacheSize + handle->navCacheOverlap + 1;
        }
        else
        {
            handle->navCacheIndexStart = index - handle->navCacheOverlap;
        }

        /* need to handle -1 case */
        if (handle->navCacheIndexStart < 0)
        {
            handle->navCacheIndexStart = 0;
        }
        /* need to handle being outside of trimmed bounds as well*/
        if (handle->navCacheIndexStart < handle->firstIndex)
        {
            handle->navCacheIndexStart = handle->firstIndex;
        }

        pos = handle->navFileIndexEntryStart + (handle->navCacheIndexStart*handle->navFileIndexEntrySize);
        if ((*handle->seekCb)(handle->filePointer, pos, SEEK_SET))
            t = -1;
        else {
            t = (*handle->tellCb)(handle->filePointer);
            if (t != -1)
                t = (t - handle->navFileIndexEntryStart) / handle->navFileIndexEntrySize;
        }
        /* We may not get exactly what we asked for, but it may work. */
        if (t != -1 && t <= index && t < index + handle->navCacheSize)
            handle->navCacheIndexStart = t;
        else {
            BDBG_WRN(("Seek was invalidated: %ld, %ld", t, index));
            handle->navCacheIndexStart = saveStart;
            handle->navCacheIndexEnd = saveEnd;
            handle->navCacheFails++;
            return 0;
        }

        handle->navCacheIndexEnd = handle->navCacheIndexStart;

        /* The end is equal to the start plus whatever we are able to read */
        handle->navCacheIndexEnd += (*handle->readCb)(handle->navCache, 1, handle->navFileIndexEntrySize * handle->navCacheSize, handle->filePointer) / handle->navFileIndexEntrySize;

        if (index >= handle->navCacheIndexEnd || handle->navCacheIndexEnd == handle->navCacheIndexStart) {
            /* The index bounds should prevent this, therefore it is an ERR. */
            BDBG_ERR(("Unable to read index: %ld in %ld..%ld",
                index, handle->navCacheIndexStart, handle->navCacheIndexEnd));
            handle->navCacheFails++;
            return 0;
        }
        handle->navCacheMisses++;
    }

    handle->navCacheReads++;
    return (BNAV_Entry *) &(handle->navCache[(index-handle->navCacheIndexStart) * handle->navFileIndexEntrySize]);
}

void BNAV_Player_GetStatus(const BNAV_Player_Handle handle, BNAV_Player_Status *status) {
    memset(status, 0, sizeof(*status));
    status->fifoSize = BNAV_Player_GetFifoSize(handle);
    status->currentIndex = handle->currentIndex;
}

int BNAV_Player_GetPosition(BNAV_Player_Handle handle, BNAV_Player_Position *position) {
    return BNAV_Player_GetPositionInformation(handle, handle->currentIndex, position);
}

int BNAV_Player_GetPositionInformation(BNAV_Player_Handle handle, long index, BNAV_Player_Position *position) {
    BNAV_Entry *pEntry = BNAV_Player_ReadEntry(handle, index);
    uint64_t offset;
    if (!pEntry)
        return -1;

    if(handle->navVersion == BNAV_Version_AVC_Extended) {
        return BNAV_Player_SVC_GetPositionInformation(&handle->svc, index, position);
    }

    position->index = index;
    position->pts = BNAV_get_framePts(pEntry);
    offset = BNAV_get_frameOffset(pEntry);
    position->offsetHi = getHi64(offset);
    position->offsetLo = getLo64(offset);

    if (handle->navVersion == BNAV_Version_AVC_Extended) {
        /* SPS preceeds PPS, so we set the greater number */
        position->metadataOffset = BNAV_get_SPS_Offset(pEntry);
    }
    else {
        position->metadataOffset = BNAV_get_seqHdrStartOffset(pEntry);
    }
    position->timestamp = BNAV_get_timestamp(pEntry);
    position->vchipState = BNAV_unpack_vchip(BNAV_get_packed_vchip(pEntry));

    return 0;
}

static bool is_dqt(eBpPlayModeParam playMode)
{
    switch (playMode) {
    case eBpPlayDecoderGOPTrick:
    case eBpPlayDecoderGOPIPTrick:
    case eBpPlayDecoderMDqtTrick:
        return true;
    default:
        return false;
    }
}

int BNAV_Player_GetNextPlayEntry(BNAV_Player_Handle handle, BNAV_Player_PlayEntry *navEntry, unsigned char *pkt)
{
    long        startIndex;
    BNAV_PktFifoEntry *fifoEntry;

    if (BNAV_Player_UpdateBounds(handle))
        return BNAV_GENERIC_IDX_FAIL;


    if (handle->currentIndex < handle->firstIndex ) {
        /* Index file has been trimmed ! */
        BDBG_MSG(("currentIndex %lx, first %lx , last %lx , current offset " BDBG_UINT64_FMT ", firstFifoEntry %d, lastFifoEntry %d",
            handle->currentIndex, handle->firstIndex, handle->lastIndex, BDBG_UINT64_ARG(handle->currentOffset), (unsigned)handle->firstFifoEntry,
            (unsigned)handle->lastFifoEntry));

        if( handle->playMode == eBpPlayNormal) {
            /* Tell caller, index has gone away from under us so they can take action */
            return BNAV_GENERIC_IDX_FAIL;
        }
    }

    /* If the fifo is empty, allow the algorithm to fill it. If we have even one
    entry, allow the app to get it before processing more.
    Throughout this loop, any function may fail because of trimming away the beginning/end of file. If this happens, this function should fail. */
    while (handle->firstFifoEntry > handle->lastFifoEntry)
    {
        /* reset the fifo */
        handle->firstFifoEntry = 0;
        handle->lastFifoEntry = -1;

        if(handle->navVersion == BNAV_Version_AVC_Extended) {
            if(BNAV_Player_SVC_PopulateFifo(&handle->svc)) {
                return BNAV_GENERIC_IDX_FAIL;
            }
            continue;
        }


        /* If our BNAV_Player_BuildReferenceFrame_Process was interrupted,
        continue it */
        if (handle->buildRef.inProcess) {
            if (BNAV_Player_BuildReferenceFrame_Process(handle))
                return BNAV_GENERIC_IDX_FAIL;
            continue;
        }

        if (handle->playMode == eBpPlayNormal) {
            if (BNAV_Player_AddNormalPlay(handle))
                return BNAV_GENERIC_IDX_FAIL;
        }
        else if (handle->playMode == eBpPlayNormalByFrames) {
            if (BNAV_Player_AddNormalPlayByFrames(handle))
                return BNAV_GENERIC_IDX_FAIL;
        }
        else {
            /* needsDisplayPicture will be set if the last call to BNAV_Player_AddFrameToFifo
            results in a BuildReferenceFrame. In that case, another trip round is
            required to build the actual display picture, so we don't need to advance
            again. */
            if (!handle->buildRef.needDisplayPicture) {
                if (BNAV_Player_AdvanceIndex(handle))
                    return BNAV_ADVANCE_IDX_FAIL;
            }

            if (handle->playMode == eBpPlayDecoderMDqtTrick) {
                if (handle->gopTrick.intraGopPictureIndex == -1 && handle->gopTrick.currentStart != -1 && handle->currentIndex == handle->gopTrick.currentEnd) {
                    /* short-circuit for MDqtTrick */
                    memset(navEntry, 0, sizeof(*navEntry));
                    navEntry->waitingForIntraGopPictureIndex = true;
                    return 0;
                }
            }

            /* now add it to the fifo */
            startIndex = handle->currentIndex;
            if (handle->playMode == eBpPlayBrcm) {
                if (BNAV_Player_AddFrameToFifo(handle, startIndex))
                    return BNAV_GENERIC_IDX_FAIL;
            }
            else {
                /* I, IP, SkipB, SkipP - just add the current frame to the fifo */
                if (BNAV_Player_AddFrame(handle, startIndex))
                    return BNAV_GENERIC_IDX_FAIL;
            }
            if (handle->playMode == eBpPlayDecoderMDqtTrick) {
                handle->currentIndex = handle->gopTrick.currentEnd;
            }
        }
    }

    handle->gotEntry = 1;
    fifoEntry = &handle->pktfifo[handle->firstFifoEntry];
    memset(navEntry, 0, sizeof(*navEntry));

    switch (fifoEntry->insertpackettype) {
    case eBpInsertStartCode:
        navEntry->isInsertedPacket = 1;
        BNAV_Player_GenerateStartCodePacket(handle, pkt, fifoEntry->startcode, fifoEntry->pid);
        break;

    case eBpInsertDummy:
        navEntry->isInsertedPacket = 1;
        BNAV_Player_GenerateDummyPicture(handle, pkt, fifoEntry->pid);
        break;

    case eBpInsertNULL:
        navEntry->isInsertedPacket = 1;
        BNAV_Player_GenerateNullPacket(handle, pkt);
        break;

    case eBpInsertBTP:
        /* Handle packet insertions */
        navEntry->isInsertedPacket = fifoEntry->insertpackettype;   /* For now... */
        BNAV_Player_GenerateBTP(handle, fifoEntry->pktdata, pkt,
            /* Test if we're discarding the tailend */
            (fifoEntry->pktdata[0] == TT_MODE_PROCESS && fifoEntry->pktdata[8] < 188), fifoEntry->pid);
        break;

    case eBpInsertNone:
        /* Specify frame data to send */
        navEntry->isInsertedPacket = 0;   /* For now... */

        navEntry->startOffsetHi = getHi64(fifoEntry->startByte);
        navEntry->startOffset = getLo64(fifoEntry->startByte);
        navEntry->zeroByteCountBegin = fifoEntry->zeroByteCountBegin;
        navEntry->zeroByteCountEnd = fifoEntry->zeroByteCountEnd;
        navEntry->byteCount = 1 + getLo64(
            fifoEntry->endByte -
            fifoEntry->startByte);
        break;

    default:
        BDBG_ERR(("Invalid HITS packet insertion type!"));
        return BNAV_GENERIC_IDX_FAIL;
    }

    if(handle->navVersion == BNAV_Version_AVC_Extended && handle->playMode != eBpPlayNormal) {
        navEntry->filteredPid = fifoEntry->pid;
    }

    /* tell host if this is the last piece of data in the fifo */
    if (handle->firstFifoEntry >= handle->lastFifoEntry)
        navEntry->isLastEntry = 1;
    else
        navEntry->isLastEntry = 0;

    /* Get next entry */
    handle->firstFifoEntry++;

    return 0;
}

void BNAV_Player_FlushFifo(BNAV_Player_Handle handle)
{
    handle->firstFifoEntry = 0;
    handle->lastFifoEntry = -1;
    BDBG_MSG(("flush fifo"));
    /* don't flush pts cache here. last picture may still be on display. */
}

int BNAV_Player_GetFifoSize(const BNAV_Player_Handle handle)
{
    return (handle->lastFifoEntry+1) - handle->firstFifoEntry;
}

/**
* In normal play, we read ALL data according to a specified size.
* However, keep the current index state variable up-to-date.
**/
static int BNAV_Player_AddNormalPlayByFrames(BNAV_Player_Handle handle)
{
    uint64_t nextOffset;
    BNAV_PktFifoEntry *curFifoEntry;
    BNAV_Entry *pEntry;

    /* add to the fifo */
    CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
    curFifoEntry->insertpackettype = eBpInsertNone;
    curFifoEntry->startByte = handle->currentOffset;

#ifdef LOOSE_INDEX_CHECKING
retry:
#endif
    /* find the end of the next frame */
    ++handle->currentIndex;
    pEntry = BNAV_Player_ReadEntry(handle, handle->currentIndex);
    if (!pEntry) {
        if (handle->loopMode == eBpSinglePlay)
            return -1;

        handle->currentIndex = handle->firstIndex;
        pEntry = BNAV_Player_ReadEntry(handle, handle->currentIndex);
        if (!pEntry)
            return -1;
    }

    if (BNAV_get_frameSize(pEntry) == 0) {
        BDBG_ERR(("invalid frame size 0 at index %d", (int)handle->currentIndex));
#ifdef LOOSE_INDEX_CHECKING
        goto retry;
#else
        return -1;
#endif
    }

    nextOffset = BNAV_get_frameOffset(pEntry) + BNAV_get_frameSize(pEntry);
    if (handle->packetSize) {
        nextOffset -= nextOffset % handle->packetSize;
    }

    /* feed everything from previous currentOffset to the next currentOffset. drop nothing. */
    curFifoEntry->endByte = nextOffset - 1;
    handle->currentOffset = curFifoEntry->endByte + 1;

    BNAV_Player_P_AddCurrentToPTSCache(handle);
    return 0;
}

static int BNAV_Player_AddNormalPlay(BNAV_Player_Handle handle)
{
    uint64_t nextOffset, targetOffset;
    BNAV_PktFifoEntry *curFifoEntry;
    BNAV_Entry *pEntry;

    if (handle->currentIndex >= handle->lastIndex) {
        return -1;
    }

    /* add to the fifo */
    CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
    curFifoEntry->insertpackettype = eBpInsertNone;
    curFifoEntry->startByte = handle->currentOffset;

    targetOffset = handle->currentOffset + handle->normalPlayBufferSize;
    if (handle->settings.normalPlayAlignment) {
        targetOffset &= ~(handle->settings.normalPlayAlignment-1);
    }

    /* find out how much we can read, up to normalPlayBufferSize */
    nextOffset = handle->currentOffset;
    while ((pEntry = BNAV_Player_ReadEntry(handle, handle->currentIndex))) {
        if (BNAV_get_frameSize(pEntry) == 0) {
            BDBG_ERR(("invalid frame size 0 at index %d", (int)handle->currentIndex));
#ifdef LOOSE_INDEX_CHECKING
            handle->currentIndex++;
            continue;
#else
            return -1;
#endif
        }
        
        nextOffset = BNAV_get_frameOffset(pEntry) + BNAV_get_frameSize(pEntry);
        if (handle->packetSize) {
            nextOffset += handle->packetSize - (nextOffset % handle->packetSize);
        }
        if (nextOffset > targetOffset) {
            curFifoEntry->endByte = targetOffset - 1;
            break;
        }
        curFifoEntry->endByte = nextOffset - 1;
        BNAV_Player_P_AddCurrentToPTSCache(handle);
        if (handle->currentIndex < handle->lastIndex) {
            handle->currentIndex++;
        }
        else {
            break;
        }
    }

    /* do we have something to play? */
    if (curFifoEntry->endByte > curFifoEntry->startByte) {
        handle->currentOffset = curFifoEntry->endByte + 1;
        return 0;
    }
    else {
        /* remove fifo entry - we should only have one entry, so flush will do it. */
        BNAV_Player_FlushFifo(handle);

        if (handle->loopMode == eBpLoopForever) {
            if (!BNAV_Player_UpdateBounds(handle)) {
                BNAV_Player_SetCurrentIndex(handle, handle->firstIndex);
                return BNAV_Player_AddNormalPlay(handle);
            }
        }
        return -1;
    }
}

static int BNAV_Player_P_FindGOP(BNAV_Player_Handle handle, int num_gops)
{
    long curIndex = handle->currentIndex;
    BNAV_Entry *pStartEntry;

    /* if we're starting, get on a GOP */
    if (handle->gopTrick.currentStart == -1) {
        BNAV_Entry *onePrevEntry = NULL;
        BDBG_MSG(("finding GOP"));
        while ((pStartEntry = BNAV_Player_ReadEntry(handle, curIndex))) {
            if (handle->navVersion == BNAV_Version_AVC || handle->navVersion == BNAV_Version_HEVC){
                if (BNAV_get_RandomAccessIndicator((const BNAV_AVC_Entry *)pStartEntry))
                    break;
            }
            else {
                if (BNAV_get_frameType(pStartEntry) == eSCTypeIFrame) {
                    /* bcmplayer indexes field-encoded streams as two I's. back up and take the first if so. */
                    onePrevEntry = BNAV_Player_ReadEntry(handle, curIndex-1);
                    if (onePrevEntry && BNAV_get_frameType(onePrevEntry) == eSCTypeIFrame) {
                        pStartEntry = onePrevEntry;
                        curIndex--;
                    }
                    else {
                        onePrevEntry = NULL;
                    }
                    break;
                }
            }
            curIndex += handle->playDir;
        }
        BDBG_MSG(("got %p at %d", (void *)pStartEntry, (int)curIndex));
        if (!pStartEntry) return -1; /* hit end */

        handle->gopTrick.currentStart = curIndex;

        /* find GopEnd */
        BDBG_MSG(("finding end of GOP"));
        if (onePrevEntry) curIndex++;
        while ((pStartEntry = BNAV_Player_ReadEntry(handle, ++curIndex))) {
            if (handle->navVersion == BNAV_Version_AVC || handle->navVersion == BNAV_Version_HEVC){
                if (BNAV_get_RandomAccessIndicator((const BNAV_AVC_Entry *)pStartEntry))
                    break;
            }
            else {
                if (BNAV_get_frameType(pStartEntry) == eSCTypeIFrame)
                    break;
            }
        }
        BDBG_MSG(("got %p at %d", (void *)pStartEntry, (int)(curIndex-1)));
        if (pStartEntry) {
            handle->gopTrick.currentEnd = --curIndex;
        }
        else {
            /* if we got a start but not an end, we may be at the end of the file. */
            handle->gopTrick.currentEnd = handle->gopTrick.currentStart;
            curIndex = handle->gopTrick.currentStart-1;
            /* go backward to find the start */
            while ((pStartEntry = BNAV_Player_ReadEntry(handle, curIndex))) {
                if (handle->navVersion == BNAV_Version_AVC || handle->navVersion == BNAV_Version_HEVC) {
                    if (BNAV_get_RandomAccessIndicator((const BNAV_AVC_Entry *)pStartEntry))
                        break;
                }
                else {
                    if (BNAV_get_frameType(pStartEntry) == eSCTypeIFrame)
                        break;
                }
                curIndex--;
            }
            if (!pStartEntry) {
                BDBG_ERR(("cannot do GOP trick mode on stream without one complete GOP"));
                return -1;
            }
            handle->gopTrick.currentStart = curIndex;
        }
    }
    BDBG_ASSERT(handle->gopTrick.currentStart != -1 && handle->gopTrick.currentEnd != -1);

    /* now skip gops */
    while (num_gops--) {
        if (handle->playDir > 0) {
            curIndex = handle->gopTrick.currentEnd;
            while ((pStartEntry = BNAV_Player_ReadEntry(handle, ++curIndex))) {
                if (handle->navVersion == BNAV_Version_AVC || handle->navVersion == BNAV_Version_HEVC){
                    if (BNAV_get_RandomAccessIndicator((const BNAV_AVC_Entry *)pStartEntry))
                        break;
                }
                else {
                    if (BNAV_get_frameType(pStartEntry) == eSCTypeIFrame)
                        break;
                }
            }
            if (!pStartEntry) return -1; /* hit end */
            handle->gopTrick.currentStart = handle->gopTrick.currentEnd;
            handle->gopTrick.currentEnd = curIndex;
        }
        else {
            curIndex = handle->gopTrick.currentStart;
            while ((pStartEntry = BNAV_Player_ReadEntry(handle, --curIndex))) {
                if (handle->navVersion == BNAV_Version_AVC || handle->navVersion == BNAV_Version_HEVC){
                    if (BNAV_get_RandomAccessIndicator((const BNAV_AVC_Entry *)pStartEntry))
                        break;
                }
                else {
                    if (BNAV_get_frameType(pStartEntry) == eSCTypeIFrame) {
                        /* bcmplayer indexes field-encoded streams as two I's. back up and take the first if so. */
                        BNAV_Entry *onePrevEntry = BNAV_Player_ReadEntry(handle, curIndex-1);
                        if (onePrevEntry && BNAV_get_frameType(onePrevEntry) == eSCTypeIFrame) {
                            pStartEntry = onePrevEntry;
                            curIndex--;
                        }
                        break;
                    }
                }
            }
            if (!pStartEntry) return -1; /* hit end */
            handle->gopTrick.currentEnd = handle->gopTrick.currentStart;
            handle->gopTrick.currentStart = curIndex;
        }
        BDBG_MSG(("GOP at [%d,%d]%s", handle->gopTrick.currentStart, handle->gopTrick.currentEnd, num_gops?" skip":""));
    }
    BDBG_ASSERT(handle->gopTrick.currentStart != -1 && handle->gopTrick.currentEnd != -1);

    return handle->gopTrick.currentStart;
}

void BNAV_Player_SetMDqtInfo( BNAV_Player_Handle handle, unsigned intraGopPictureIndex, unsigned openGopPictures )
{
    handle->gopTrick.intraGopPictureIndex = (int)intraGopPictureIndex;
    handle->gopTrick.openGopPictures = openGopPictures;
}

int BNAV_Player_AdvanceIndex(BNAV_Player_Handle handle)
{
    long        startIndex;
    BNAV_Entry *pStartEntry;

    /* updateBounds already called from getNextPlayEntry. */
    startIndex = handle->currentIndex;
    switch(handle->playMode)
    {
    case eBpPlayDecoderGOPTrick:
    case eBpPlayDecoderGOPIPTrick:
        /* now advance if we're in the middle of a GOP */
        if (handle->gopTrick.currentStart != -1 &&
            startIndex != handle->gopTrick.currentEnd &&
            handle->gopTrick.count < handle->advanceCount)
        {
            while (1) {
                startIndex++;
                /* always send currentEnd picture to keep logic consistent */
                if (startIndex == handle->gopTrick.currentEnd) break;
                
                pStartEntry = BNAV_Player_ReadEntry(handle, startIndex);
                if (!pStartEntry || BNAV_get_frameType(pStartEntry) != eSCTypeBFrame) {
                    break;
                }
                
                if (handle->playMode == eBpPlayDecoderGOPTrick && (handle->navVersion == BNAV_Version_AVC || handle->navVersion == BNAV_Version_HEVC || handle->gopTrick.count)) {
                    /* in eBpPlayDecoderGOPTrick, we send B unless it's an MPEG open gop */
                    break;
                }
                
                /* continue, which will skip the B */
            };
            handle->gopTrick.count++;
        }
        else
        {
            BNAV_PktFifoEntry  *curFifoEntry;

            BDBG_MSG(("start GOP tag=%d count=%d", handle->gopTrick.pictureTag, handle->advanceCount));
            /* find a starting GOP or the next GOP */
            startIndex = BNAV_Player_P_FindGOP(handle, handle->gopTrick.currentStart == -1 ? 0 : (handle->gopTrick.gopSkip+1));
            pStartEntry = BNAV_Player_ReadEntry(handle, startIndex);
            if (!pStartEntry) return -1;

            /* only add I frame to PTS cache; otherwise the reordering issues are quite difficult and probably unnecessary. */
            BNAV_Player_P_AddToPTSCache(handle, BNAV_get_framePts(pStartEntry), startIndex);

            /* before sending next I, send a flush command to prevent prediction before I */
            CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
            curFifoEntry->insertpackettype = eBpInsertBTP;
            curFifoEntry->pktdata[0] = TT_MODE_INLINE_FLUSH;

            /* Insert timing marker indicating the beginning of the next GOP */
            CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
            curFifoEntry->insertpackettype = eBpInsertBTP;
            curFifoEntry->pktdata[0] = TT_MODE_PICTURE_TAG;
            curFifoEntry->pktdata[9] = handle->gopTrick.pictureTag++;

            /* Picture Output "N"  marker
            ** AVD will ouput the first 'handle->advanceCount' pictures of this GOP.
            */
            CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
            curFifoEntry->insertpackettype = eBpInsertBTP;
            curFifoEntry->pktdata[0] = TT_MODE_PIC_OUTPUT_COUNT;
            curFifoEntry->pktdata[9] = handle->advanceCount;    /* number of pictures specified by the application */
            handle->gopTrick.count = 0;

            handle->lastSeqHdrOffset64 = 0;

            startIndex = handle->gopTrick.currentStart;

        }
        break;
    case eBpPlayDecoderMDqtTrick:
        {
            BNAV_PktFifoEntry  *curFifoEntry;

            if (handle->gopTrick.currentStart != -1) {
                if (handle->gopTrick.dqt_state == dqt_state_started) {
                    BDBG_MSG(("end GOP tag=%d", handle->gopTrick.pictureTag));
                    handle->gopTrick.pictureTag++;
                    CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
                    curFifoEntry->insertpackettype = eBpInsertBTP;
                    curFifoEntry->pktdata[0] = TT_MODE_INLINE_FLUSH;

                    CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
                    curFifoEntry->insertpackettype = eBpInsertBTP;
                    curFifoEntry->pktdata[0] = TT_MODE_PICTURE_LAST;

                    CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
                    curFifoEntry->insertpackettype = eBpInsertBTP;
                    curFifoEntry->pktdata[0] = TT_MODE_INLINE_FLUSH;

                    handle->gopTrick.dqt_state = dqt_state_sent;
                }

                if (handle->gopTrick.intraGopPictureIndex == -1) {
                    return 0;
                }
                handle->gopTrick.dqt_state = dqt_state_ready;
            }

            /* find a starting GOP or the next GOP.
            if intraGopPictureIndex is non-zero, we must repeat this gop.
            Nexus is responsible for not calling GetNextEntry after a GOP has been fed until it sets that intraGopPictureIndex. */
            startIndex = BNAV_Player_P_FindGOP(handle, handle->gopTrick.currentStart == -1 || handle->gopTrick.intraGopPictureIndex > 0 ? 0 : 1);
            pStartEntry = BNAV_Player_ReadEntry(handle, startIndex);
            if (!pStartEntry) return -1;
            BDBG_MSG(("start GOP tag=%d intraGopPictureIndex=%d openGop=%d %d..%d(%d pictures, start pts %08x)", handle->gopTrick.pictureTag, handle->gopTrick.intraGopPictureIndex,
                handle->gopTrick.openGopPictures,
                handle->gopTrick.currentStart, handle->gopTrick.currentEnd, handle->gopTrick.currentEnd-handle->gopTrick.currentStart, BNAV_get_framePts(pStartEntry)));

            /* only add I frame to PTS cache; otherwise the reordering issues are quite difficult and probably unnecessary. */
            BNAV_Player_P_AddToPTSCache(handle, BNAV_get_framePts(pStartEntry), startIndex);

            /* Insert timing marker indicating the beginning of the next GOP */
            CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
            curFifoEntry->insertpackettype = eBpInsertBTP;
            curFifoEntry->pktdata[0] = TT_MODE_PICTURE_TAG;
            curFifoEntry->pktdata[9] = (handle->gopTrick.pictureTag << 16) | handle->gopTrick.intraGopPictureIndex;
            handle->gopTrick.dqt_state = dqt_state_started;

            if (handle->gopTrick.intraGopPictureIndex != 0) {
                /* Picture Output "N"  marker
                ** AVD will ouput the first 'handle->advanceCount' pictures of this GOP.
                */
                CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
                curFifoEntry->insertpackettype = eBpInsertBTP;
                curFifoEntry->pktdata[0] = TT_MODE_OUTPUT_N;
                curFifoEntry->pktdata[9] = handle->gopTrick.intraGopPictureIndex + handle->gopTrick.openGopPictures;
            }
            handle->gopTrick.intraGopPictureIndex = -1; /* consumed */
            handle->lastSeqHdrOffset64 = 0;

            startIndex = handle->gopTrick.currentStart;
        }
        break;
    case eBpPlayBrcm:
        if (handle->playDir == eBpForward)
        {
            if (!handle->skipNextAdvance)
                startIndex = startIndex + handle->advanceCount;
            pStartEntry = BNAV_Player_ReadEntry(handle, startIndex);
            if (pStartEntry == NULL)
            {
                if ((handle->loopMode != eBpLoopForever) || (handle->gotEntry == 0))
                {
                    handle->gotEntry = 1;
                    handle->currentIndex = startIndex - handle->advanceCount;
                    return -1;
                }
                handle->gotEntry = 0;   /* Ensure we don't loop more than once without returning */
                startIndex = handle->firstIndex; /* Start over from the beginning. */
                pStartEntry = BNAV_Player_ReadEntry(handle, startIndex);
                if (!pStartEntry) return -1;
            }
        }
        else
        {
            if (!handle->skipNextAdvance)
                startIndex = startIndex - handle->advanceCount;
            if (startIndex < 0)
                pStartEntry = NULL;
            else
                pStartEntry = BNAV_Player_ReadEntry(handle, startIndex);
            if (pStartEntry == NULL)
            {
                if ((handle->loopMode != eBpLoopForever) || (handle->gotEntry == 0))
                {
                    handle->gotEntry = 1;
                    handle->currentIndex = startIndex + handle->advanceCount;
                    return -1;
                }
                /* We've reached beginning of stream.  Goto the end. */
                handle->gotEntry = 0;   /* Ensure we don't loop more than once without returning */
                startIndex = handle->lastIndex;
            }
        }
        break;

    case eBpPlayI:
    case eBpPlayIP:
    case eBpPlaySkipB:
    case eBpPlaySkipP:
        {
            unsigned long tempCount = handle->advanceCount;
            int loopAround = 0; /* have we already looped the index */
            int keepGoing = 1; /* should we look at the next frame */
            long previousIFrame = -1;

            while (keepGoing) {
                eSCType frameType;

                if (handle->skipNextAdvance) {
                    /* Only skip the first advance and take the first one we see, regardless of advanceCount. */
                    handle->skipNextAdvance = 0;
                    tempCount = 1;
                }
                else {
                    startIndex += handle->playDir;
                }
                if ((pStartEntry = BNAV_Player_ReadEntry(handle, startIndex)) == NULL)
                {
                    if (handle->loopMode == eBpSinglePlay) {
                        /* if we're not looping, then we need to push out the first/last I frame for a consistent stop */
                        if (handle->playMode == eBpPlayI) {
                            if (previousIFrame != -1) {
                                /* back up and send out the last i frame we skipped */
                                startIndex = previousIFrame;
                                keepGoing = 0;
                                continue;
                            }
                            else if (handle->iFrameRepeatCount++ < 2) {
                                /* repeat the last I frame 2 more times to get through the decoder's pipeline */
                                startIndex = handle->currentIndex;
                                keepGoing = 0;
                                continue;
                            }
                        }
                        /* we're done */
                        return -1;
                    }

                    /* prevent an infinite loop. */
                    if (loopAround)
                        return -1;

                    /* reset startIndex, taking into account the += playDir. */
                    if (handle->playDir > 0)
                        startIndex = handle->firstIndex - 1;
                    else
                        startIndex = handle->lastIndex + 1;
                    loopAround++;
                    continue;
                }

                frameType = BNAV_get_frameType(pStartEntry);
                switch (handle->playMode) {
                case eBpPlayIP:
                    if (frameType == eSCTypeRPFrame || frameType == eSCTypeIFrame || frameType == eSCTypePFrame) {
                        if (!--tempCount)
                            keepGoing = 0;
                    }
                    break;
                case eBpPlaySkipB:
                    keepGoing = 0;
                    if (frameType == eSCTypeBFrame) {
                        if ((unsigned)handle->skipCount == handle->advanceCount) {
                            handle->skipCount = 0;
                            keepGoing = 1;
                        }
                        else
                            handle->skipCount++;
                    }
                    break;
                case eBpPlaySkipP:
                    if (frameType == eSCTypeIFrame) {
                        keepGoing = 0;
                        /* reset skipCount for the next P */
                        handle->skipCount = 0;
                    }
                    else if (frameType == eSCTypePFrame && handle->skipCount != -1) {
                        if ((unsigned)handle->skipCount == handle->advanceCount) {
                            /* keep going until we hit a I frame */
                            handle->skipCount = -1;
                        }
                        else {
                            handle->skipCount++;
                            keepGoing = 0;
                        }
                    }
                    break;
                case eBpPlayI:
                    if (frameType == eSCTypeIFrame) {
                        previousIFrame = startIndex;
                        if (!--tempCount) {
                            /* For field encoded streams, you can get two I pictures which are adjacent to each other.
                            For purposes of I frame advanceCount, we should ignore the second I picture.
                            This will mess up I-only streams with advanceCount == 1, but that's not really a trick mode
                            anyway. */
                            if ((handle->navVersion == BNAV_Version_AVC || handle->navVersion == BNAV_Version_HEVC) && startIndex == handle->currentIndex+1) {
                                /* read to the tempCount and don't set keepGoing */
                                tempCount++;
                            }
                            else {
                                keepGoing = 0;
                            }
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }
        break;

    case eBpPlayTimeSkip:
        {
            long previousIFrame = -1;
            int keepGoing = 1; /* should we look at the next frame */

            while (keepGoing) {
                if (handle->skipNextAdvance) {
                    /* Only skip the first advance. We have to validate that we're on the
                    right kind of frame. */
                    handle->skipNextAdvance = 0;
                }
                else {
                    startIndex += handle->playDir;
                }
                if ((pStartEntry = BNAV_Player_ReadEntry(handle, startIndex)) == NULL)
                {
                    /* eBpPlayTimeSkip only supports eBpSinglePlay. nexus & bmedia only use eBpSinglePlay. */

                    /* if we're not looping, then we need to push out the first/last I frame for a consistent stop */
                    if (previousIFrame != -1) {
                        /* back up and send out the last i frame we skipped */
                        startIndex = previousIFrame;
                        keepGoing = 0;
                        continue;
                    }
                    else if (handle->iFrameRepeatCount++ < 2) {
                        /* repeat the last I frame 2 more times to get through the decoder's pipeline */
                        startIndex = handle->currentIndex;
                        keepGoing = 0;
                        continue;
                    }
                    return -1;
                }

                if (BNAV_get_frameType(pStartEntry) == eSCTypeIFrame) {
                    previousIFrame = startIndex;

                    if (!handle->timeSkip.set) {
                        handle->timeSkip.set = true;
                        handle->timeSkip.firstTimestamp = BNAV_get_timestamp(pStartEntry);
                        handle->timeSkip.pics = 1;
                        keepGoing = 0;
                        BDBG_MSG(("timeSkip set: %d", handle->timeSkip.firstTimestamp));
                    }
                    else {
                        unsigned timeSkip, averageTimeSkip;
                        if (handle->playDir > 0) {
                            timeSkip = BNAV_get_timestamp(pStartEntry) - handle->timeSkip.firstTimestamp;
                        }
                        else {
                            timeSkip = handle->timeSkip.firstTimestamp - BNAV_get_timestamp(pStartEntry);
                        }
                        averageTimeSkip = timeSkip / handle->timeSkip.pics;
                        if (averageTimeSkip < handle->advanceCount) {
                            /* skip it */
                            keepGoing = 1;
                        }
#if 0
/* by default, bcmplayer does not repeat I frames in order to hit a rate.
   if you want that behavior, please enable this code block. */
                        else if (averageTimeSkip > handle->advanceCount + (handle->advanceCount/10)) {
                            /* repeat it */
                            keepGoing = 0;
                            startIndex = handle->currentIndex;
                            handle->timeSkip.pics++;
                            BDBG_MSG(("timeSkip repeat %d", handle->currentIndex));
                        }
#endif
                        else {
                            /* send it */
                            keepGoing = 0;
                            handle->timeSkip.pics++;
                        }
                        BDBG_MSG(("timeSkip test: %s avg:%d/%d=%d target:%d",
                            keepGoing?"skip":"send", timeSkip, handle->timeSkip.pics, averageTimeSkip, handle->advanceCount));
                    }
                }
            }
        }
        break;

    default:
        break;
    }

    handle->currentIndex = startIndex;
    handle->skipNextAdvance = 0;
    if (!is_dqt(handle->playMode)) {
        BNAV_Player_P_AddCurrentToPTSCache(handle);
    }
    return 0;
}

/**
Feed a frame for non-brcm (host only) based trick mode.
Or feed a GOP for MDQT.
**/
static int BNAV_Player_AddFrame(BNAV_Player_Handle handle, long entry)
{
    BNAV_PktFifoEntry  *curFifoEntry;
    BNAV_Entry *pStartEntry;
    uint64_t seqHdrOffset64;
    int result = 0;
    bool feedgop = (handle->playMode == eBpPlayDecoderMDqtTrick);

    if (feedgop) {
        BDBG_MSG(("AddFrame GOP %d..%d", (int)entry, (int)handle->gopTrick.currentEnd));
    }
    else {
        BDBG_MSG(("AddFrame index=%d", (int)entry));
    }

    CHECKREAD(pStartEntry = BNAV_Player_ReadEntry(handle, entry));

    /* For AVC, send an EOS before every I picture in I frame trick mode to avoid
    picture_order_count (poc) reordering. This cannot be used for IP or IPB host trick modes
    because the P's and B's might predict before the I. */
    if ( (handle->navVersion == BNAV_Version_AVC || handle->navVersion == BNAV_Version_HEVC) && BNAV_get_frameType(pStartEntry) == eSCTypeIFrame &&
        (handle->playMode == eBpPlayI || handle->playMode == eBpPlayTimeSkip))
    {
        if(handle->navVersion == BNAV_Version_AVC) {
            CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
            curFifoEntry->insertpackettype = eBpInsertStartCode;
            curFifoEntry->startcode = 0x0A;
        }
        else {
            CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
            curFifoEntry->insertpackettype = eBpInsertStartCode;
            curFifoEntry->startcode = 0x4A01;
        }
        /* This will flush the SPS/PPS, so clear the state variable. */
        handle->lastSeqHdrOffset64 = 0;
    }

    if (handle->settings.useBtpsForHostTrickModes && !feedgop) {
        if (handle->navVersion != BNAV_Version_VC1_PES) {
            /* send sequence header if different. for AVC this means just checking the PPS, which is sufficient. */
            seqHdrOffset64 = BNAV_get_frameOffset(pStartEntry) - BNAV_get_seqHdrStartOffset(pStartEntry);
            if (seqHdrOffset64 != handle->lastSeqHdrOffset64)
            {
                if (BNAV_Player_AddSequenceHeader(handle, pStartEntry, TT_MODE_PROCESS, 0, 0))
                    return -1;
                handle->lastSeqHdrOffset64 = seqHdrOffset64;
            }

            result = BNAV_Player_AddFrameData(handle, pStartEntry);
            if (result) return result;

            return result;
        }
    }

    /* Add fifo entry */
    CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));

    curFifoEntry->insertpackettype = eBpInsertNone;

    /* back up to TS packet */
    curFifoEntry->startByte = BNAV_get_frameOffset(pStartEntry);
    if (handle->packetSize) {
        curFifoEntry->startByte -= curFifoEntry->startByte % handle->packetSize;
    }

    if (handle->navVersion == BNAV_Version_AVC || handle->navVersion == BNAV_Version_HEVC || handle->navVersion == BNAV_Version_VC1_PES) {
        BNAV_AVC_Entry *avcEntry = (BNAV_AVC_Entry *)pStartEntry;

        /* Calc the SPS offset */
        seqHdrOffset64 = BNAV_get_frameOffset(pStartEntry) - BNAV_get_SPS_Offset(avcEntry);
        /* For AVC host trick modes w/o BTP's, we have to assume the SPS and PPS are adjacent and in
        SPS,PPS order. If this isn't true, we'll require BTP's. */
        if (handle->packetSize) {
            seqHdrOffset64 -= seqHdrOffset64 % handle->packetSize;
        }
    }
    else {
        /* Calc the seqhdr offset */
        seqHdrOffset64 = BNAV_get_frameOffset(pStartEntry) - BNAV_get_seqHdrStartOffset(pStartEntry);
        if (handle->packetSize) {
            seqHdrOffset64 -= seqHdrOffset64 % handle->packetSize;
        }
    }
    /* if there's a sequence header in the previous packet (or, for PES streams, close by), also send it */
    if (curFifoEntry->startByte - seqHdrOffset64 <= handle->packetSize)
    {
        curFifoEntry->startByte = seqHdrOffset64;
    }

    /* Now that we know where the read must begin (including possible seqhdr), we can calc the zeroByteCountBegin
    so the host can zero out data the decoder shouldn't see */
    if (handle->packetSize) {
        curFifoEntry->zeroByteCountBegin = (unsigned long)(curFifoEntry->startByte % handle->packetSize);
    }

    if (feedgop) {
        /* set the end feed byte based on the gop */
        CHECKREAD(pStartEntry = BNAV_Player_ReadEntry(handle, handle->gopTrick.currentEnd - 1));
        curFifoEntry->endByte = BNAV_get_frameOffset(pStartEntry);
        if (handle->packetSize) {
            curFifoEntry->endByte -= curFifoEntry->endByte % handle->packetSize;
        }
    }
    else {
        /* set the end feed byte based on actual picture end */
        curFifoEntry->endByte = BNAV_get_frameOffset(pStartEntry) + BNAV_get_frameSize(pStartEntry);

        if (handle->packetSize) {
            /* Round end up to nearest transport packet, but tell caller to zero out any bytes after end of picture */
            curFifoEntry->zeroByteCountEnd = handle->packetSize - (unsigned long)(curFifoEntry->endByte % handle->packetSize);
            if (curFifoEntry->zeroByteCountEnd == handle->packetSize)
                curFifoEntry->zeroByteCountEnd = 0;
            curFifoEntry->endByte += curFifoEntry->zeroByteCountEnd;
        }
    }
    curFifoEntry->endByte -= 1; /* sub one since endByte is an offset to the last good byte */

    return result;
}

/**
Builds a reference frame.

This function has some similar logic to BNAV_Player_AddFrameToFifo, but there
are enough differences (especially in handling B frames and various optimizations),
that the logic needs to be separated. Building a reference frame is a very simple
process. Send all the frames needed to build a complete past predictor and load
it into the reference frame buffer.
**/
static int BNAV_Player_BuildReferenceFrame(BNAV_Player_Handle handle, long entry)
{
    BNAV_Entry *pStartEntry;
    unsigned long rfo;
    unsigned long numFrames, lastFrameSize;
    int i;
    eSCType frametype;
    long startingentry;

    CHECKREAD_SILENT(pStartEntry = BNAV_Player_ReadEntry(handle, entry));
    frametype = BNAV_get_frameType(pStartEntry);
    rfo = BNAV_get_refFrameOffset(pStartEntry);

    if (handle->navVersion >= BNAV_VersionOpenGopBs && frametype == eSCTypeIFrame) {
        /* send the one I frame only, ignore its rfo because it points to
        previous GOP */
        numFrames = 1;
        startingentry = entry;
        rfo = 0; /* fake it out */
    }
    else {
        /* Calc how many frames will be sent */
        numFrames = 0;
        for (i = rfo; i >= 1; i--) {
            CHECKREAD_SILENT(pStartEntry = BNAV_Player_ReadEntry(handle, entry - i));
            if (BNAV_get_frameType(pStartEntry) != eSCTypeBFrame)
                numFrames++;
        }
        startingentry = entry - rfo;
    }

    CHECKREAD(pStartEntry = BNAV_Player_ReadEntry(handle, startingentry));
    lastFrameSize = BNAV_get_frameSize(pStartEntry);

    BDBG_MSG(("BUILD_REFERENCE index=%ld %s, numFrames=%ld",
        entry, BNAV_frameTypeStr[BNAV_get_frameType(pStartEntry)], numFrames));
    BNAV_Player_AddSequenceHeader(handle, pStartEntry, TT_MODE_BUILD_REFERENCE, 0,
        /* the BUILDREF skip_count is needed. any picture sent after the skip_count
        will be decoded against the refframe and displayed. */
        numFrames);

    /* Add first frame after seqhdr (for I frames, this will be the only frame */
    BNAV_Player_AddFrameData(handle, pStartEntry);

    /* Set up a process which be interrupted when the fifo fills */
    handle->buildRef.inProcess = 1;
    handle->buildRef.curOffset = rfo-1;
    handle->buildRef.lastFrameSize = lastFrameSize;
    handle->buildRef.entry = entry;
    return BNAV_Player_BuildReferenceFrame_Process(handle);
}

/**
This is the inner loop inside BNAV_Player_BuildReferenceFrame, but it's implemented
using handle->buildRef so that it can be interrupted and resumed. This is necessary
to handle large GOP sizes with a fixed size fifo. Otherwise you never know when
the next stream might overflow your fifo.
**/
static int BNAV_Player_BuildReferenceFrame_Process(BNAV_Player_Handle handle)
{
    /* Now add the rest of the frames for the reference frame */
    for (; handle->buildRef.curOffset >= 1; handle->buildRef.curOffset--)
    {
        BNAV_Entry *pStartEntry;
        int frametype;
        int index = handle->buildRef.entry - handle->buildRef.curOffset;

        /* If we're getting close to filling the fifo, abort. We'll resume
        later. The required gap must be >= the maximum number of entries that may
        be generated.
        That is: seqhdr = 4, frame = 4
        */
        if (handle->lastFifoEntry >= MAX_NUM_FIFO_ENTRIES - 8) {
            BDBG_MSG(("Interrupting BNAV_Player_BuildReferenceFrame_Process %ld, %ld...",
                handle->firstFifoEntry, handle->lastFifoEntry));
            return 0;
        }

        CHECKREAD(pStartEntry = BNAV_Player_ReadEntry(handle, index));
        frametype = BNAV_get_frameType(pStartEntry);

        if (handle->buildRef.lastFrameSize > BNAV_get_seqHdrStartOffset(pStartEntry))
        {
            BNAV_Player_AddSequenceHeader(handle, pStartEntry, TT_MODE_PROCESS, 0, 0);
        }

        /* add only non-B frames until we get to the end */
        if (frametype != eSCTypeBFrame)
        {
            BDBG_MSG(("  %d: %s", index, BNAV_frameTypeStr[frametype]));
            BNAV_Player_AddFrameData(handle, pStartEntry);
        }
        handle->buildRef.lastFrameSize = BNAV_get_frameSize(pStartEntry);
    }

    /* This is now our current reference frame */
    handle->curRefFrame = handle->buildRef.entry;
    handle->buildRef.inProcess = 0;

    return 0;
}

/**
Add a frame for brcm trick mode.

If a reference frame needs to be built, this function will exit early and state
variables will be set to come back and complete the work on another pass.
This adds command packets as needed. One call to this function will generate
all of the fifo entries needed to display a frame.
**/
static int BNAV_Player_AddFrameToFifo(BNAV_Player_Handle handle, long entry)
{
    BNAV_Entry *pStartEntry;
    unsigned long rfo;
    unsigned long numFrames = 0, lastFrameSize;
    int i;
    eSCType frametype;
    int extraBFrames = 0;
    long startingentry;

    CHECKREAD_SILENT(pStartEntry = BNAV_Player_ReadEntry(handle, entry));
    rfo = BNAV_get_refFrameOffset(pStartEntry);
    frametype = BNAV_get_frameType(pStartEntry);
    BDBG_MSG(("%ld: %s", entry, BNAV_frameTypeStr[frametype]));

    /* Check if we need a reference frame. If we're restricted from
    building a reference, this must be handled differently later on. */
    if (handle->useReferenceFrame) {
        /* Check if our GOP size is too large. If so, subdivide it. This keeps
        bandwidth requirements limited. Some GOP's are known to be 90 frames or more. */
        if (rfo > MAX_REFERENCE_FRAME_OFFSET) {
            BDBG_MSG((" subdividing GOP, org rfo %ld, refframe %ld", rfo, entry-rfo));
            rfo = rfo % MAX_REFERENCE_FRAME_OFFSET;
            if (!rfo) rfo = MAX_REFERENCE_FRAME_OFFSET;

            /* back up to nearest I or P frame */
            while(1) {
                CHECKREAD_SILENT(pStartEntry = BNAV_Player_ReadEntry(handle, entry - rfo));
                if (BNAV_get_frameType(pStartEntry) == eSCTypeBFrame)
                    rfo++;
                else
                    break;
            };
            BDBG_MSG((" subdividing GOP, final rfo %ld, refframe %ld", rfo, entry-rfo));
        }
/* Causes playback to freeze in BCM mode, take out */
#if 0
        /* initial I frames (rfo == 0) are useless to send for display. They are only good
        for references frames. */
        if (!rfo)
            return -1;
#endif

        /* build a reference frame if needed */
        if (handle->curRefFrame != (int)(entry - rfo))
        {
            handle->buildRef.needDisplayPicture = 1;
            if (BNAV_Player_BuildReferenceFrame(handle, entry - rfo) == -1) {
                handle->buildRef.needDisplayPicture = 0;
                return -1;
            }

            /* TODO: I'm using disableExtraBOptimization as a marker for when we're in
            host-based frame reverse mode. In this case, we need to build reference and
            display a picture in one pass. We should rename this variable. */
            if (!handle->disableExtraBOptimization) {
                /* By always exiting, we make the processing path the same, whether
                the BuildReferenceFrame was interrupted or not. This also keeps fifo
                requirements smaller because the reference frame will be processed
                completely before the displayed picture will be processed. */
                return 0;
            }
        }
        handle->buildRef.needDisplayPicture = 0;
    }

    /* Figure out our starting point. This may be from a reference frame
    or not. */
    if (handle->navVersion >= BNAV_VersionOpenGopBs &&
        handle->useReferenceFrame)
    {
        CHECKREAD_SILENT(pStartEntry = BNAV_Player_ReadEntry(handle, entry - rfo));
        if (BNAV_get_frameType(pStartEntry) == eSCTypeIFrame) {
            /* Don't send the I frame again, so advance to the next non-B frame. */
            do {
                rfo--;
                CHECKREAD_SILENT(pStartEntry = BNAV_Player_ReadEntry(handle, entry - rfo));
            } while (BNAV_get_frameType(pStartEntry) == eSCTypeBFrame && rfo > 0);

            /* If there's no additional frames to send, then we (unfortunately)
            have to resend the reference frame. */
            if (rfo == 0)
                rfo = BNAV_get_refFrameOffset(pStartEntry);
        }
    }
    else if (!handle->useReferenceFrame) {
        /* If we're doing no reference frame, we need to back up further to
        get a complete picture. This is essentially saving memory (one less
        buffer needed by decoder) at the price of more bandwidth. */
        CHECKREAD_SILENT(pStartEntry = BNAV_Player_ReadEntry(handle, entry - rfo));
        if (BNAV_get_frameType(pStartEntry) != eSCTypeIFrame) {
            rfo += BNAV_get_refFrameOffset(pStartEntry);
        }
    }

    /* Figure out how many frames were going to have to send after this BTP */
    for (i = rfo; i >= 0; i--)
    {
        CHECKREAD_SILENT(pStartEntry = BNAV_Player_ReadEntry(handle, entry - i));
        frametype = BNAV_get_frameType(pStartEntry);
        if (frametype != eSCTypeBFrame || i == 0)
            numFrames++;

        /**
        * See if there are extra B frames we can send because they predict off the
        * same frames. See playertypes.h for discussion.
        * NOTE: I don't support the forward direction because you would only realize
        * a gain with a GOP format of >2 B's between I/P's, but maybe someone could use it.
        * Likewise, I don't support >1x rewind because this would require the same GOP
        * format. Maybe someday.
        **/
        if (frametype == eSCTypeBFrame &&
            handle->playDir < 0 && handle->advanceCount == 1 && /* 1x rewind only */
            handle->decoderFeatures.supportsCombiningBs && /* firmware must support */
            !handle->disableExtraBOptimization /* must be enabled by app */
            )
        {
            if (i != 0)
                extraBFrames++;
        }
        else {
            extraBFrames = 0;
        }
    }

    /* startingentry is the first frame to be sent in this sequence */
    startingentry = entry - rfo;

    CHECKREAD(pStartEntry = BNAV_Player_ReadEntry(handle, startingentry));
    lastFrameSize = BNAV_get_frameSize(pStartEntry);
    frametype = BNAV_get_frameType(pStartEntry);
    /*BDBG_ASSERT(frametype != eSCTypeBFrame);*/

    if (handle->playDir > 0)
    {
        BDBG_MSG(("DISPLAY %d, %ld", 1, numFrames - 1));
        BNAV_Player_AddSequenceHeader(handle, pStartEntry, TT_MODE_DISPLAY_FORWARD, 1, numFrames - 1);
    }
    else
    {

/**
  Check SW7231-451.
**/
#if 0
        /**
        * If we're going in reverse and our current entry matches what's in the future
        * prediction buffer, then we can display the past prediction buffer.
        **/
        if (handle->decoderFeatures.supportsDisplayPastBuffer &&
            handle->navVersion >= BNAV_VersionOpenGopBs &&
            handle->playDir == eBpReverse &&
/* TODO: This is hanging the driver/firmware for 3x rewind where alignment is such
that no B frames are sent. */
            /* (handle->futurePred == entry || frametype == eSCTypeIFrame)) */
            handle->futurePred == entry)
        {
            rfo = 0; /* this will force AddDummyFrame, and that's all we need */
            /* TODO: Why is a dummy frame needed after DISPLAY_PAST_BUFFER? */
            if (!handle->decoderFeatures.supportsDummyPicture) {
                BDBG_ERR(("DisplayPastBuffer requires DummyPicture"));
            }

            /* the one display frame isn't really needed. But we'll do that later. */
            BDBG_MSG(("DISPLAY_PAST_BUFFER %d, %d", 1 + extraBFrames, 0));
            BNAV_Player_AddSequenceHeader(handle, pStartEntry, TT_MODE_DISPLAY_PAST_BUFFER, 1 + extraBFrames, 0);
        }
        else
#endif
    {
            BDBG_MSG(("DISPLAY %d, %ld", 1 + extraBFrames, numFrames - 1));
            BNAV_Player_AddSequenceHeader(handle, pStartEntry, TT_MODE_DISPLAY_REWIND, 1 + extraBFrames, numFrames - 1);
        }
    }
    /* the prediction frames are invalid at this point */
    handle->pastPred = handle->futurePred = -1;

    /* The reason we have this unconditional AddFrame is that we've already sent the PROCESS
    BTP with the AddSequenceHeader. The possible subsequent AddFrame's in the for loop will
    generate new PROCESS BTP's. */
    if (rfo == 0 && handle->decoderFeatures.supportsDummyPicture) {
        BDBG_MSG(("  %ld: %s (dummy)", startingentry, BNAV_frameTypeStr[frametype]));
        BNAV_Player_AddDummyFrame(handle, pStartEntry);
    }
    else {
        BDBG_MSG(("  %ld: %s", startingentry, BNAV_frameTypeStr[frametype]));
        BNAV_Player_AddFrameData(handle, pStartEntry);
    }

    for (i = rfo-1; i >= 0; i--)
    {
        CHECKREAD(pStartEntry = BNAV_Player_ReadEntry(handle, entry - i));
        frametype = BNAV_get_frameType(pStartEntry);

        if (lastFrameSize > BNAV_get_seqHdrStartOffset(pStartEntry))
        {
            BNAV_Player_AddSequenceHeader(handle, pStartEntry, TT_MODE_PROCESS, 0, 0);
        }

        lastFrameSize = BNAV_get_frameSize(pStartEntry);

        /**
        * If we're in buildmode, add only non-B frames.
        * If we're not in buildmode, add only non-B frames until we get to the end,
        * then add a B-frame.
        **/
        if (frametype != eSCTypeBFrame || i == 0)
        {
            if (i == 0 && frametype != eSCTypeBFrame && handle->decoderFeatures.supportsDummyPicture) {
                BDBG_MSG(("  %ld: %s (dummy)", entry-i, BNAV_frameTypeStr[frametype]));
                BNAV_Player_AddDummyFrame(handle, pStartEntry);
            }
            else {
                BDBG_MSG(("  %ld: %s", entry-i, BNAV_frameTypeStr[frametype]));
                BNAV_Player_AddFrameData(handle, pStartEntry);
            }

            /* remember what's in the prediction frames */
/* TODO: The dummy frame goes into the future prediction buffer. If it didn't,
then we could use that past prediction buffer. */
            if (frametype != eSCTypeBFrame) {
                handle->pastPred = handle->futurePred;
                handle->futurePred = entry-i;
            }
        }
    }

    /* send any previous B's that predict against the same frames. */
    for (i=1; i<=extraBFrames; i++) {
        CHECKREAD(pStartEntry = BNAV_Player_ReadEntry(handle, entry - i));
        BNAV_Player_AddFrameData(handle, pStartEntry);
    }

    /* advance the current index because we sent some extra B's */
    if (extraBFrames)
        handle->currentIndex -= extraBFrames;

    return 0;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:   None.
+ OUTPUTS:  None.
+ RETURNS:  new FIFO entry for playback, NULL if failure
+ FUNCTION: This function allocates a new FIFO entry and adds it to the end
+           of the FIFO.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static BNAV_PktFifoEntry *BNAV_Player_AddFifoEntry(BNAV_Player_Handle handle, unsigned pid )
{
    BNAV_PktFifoEntry *p_fifo;

    handle->lastFifoEntry++;
    if (handle->lastFifoEntry >= MAX_NUM_FIFO_ENTRIES)
    {
        BDBG_ERR(("bcmplayer ERROR: fifo full!"));
        return NULL;
    }
    p_fifo = &(handle->pktfifo[handle->lastFifoEntry]);

    /* Initialize values that may not otherwise get initialized in code */
    p_fifo->zeroByteCountBegin = 0;
    p_fifo->zeroByteCountEnd = 0;
    p_fifo->pid = pid;
    memset(p_fifo->pktdata, 0x00, sizeof(p_fifo->pktdata));

    return p_fifo;
}

static int BNAV_Player_SendData(BNAV_Player_Handle handle,
    unsigned long picStartByte, /* index of first byte in first packet for this picture */
    unsigned long picEndByte,   /* index of last byte in last packet for this picture   */
    uint64_t feedStartByte,     /* packet-aligned absolute offset to start of data      */
    uint64_t feedEndByte,       /* packet-aligned absolute offset to last byte of data  */
    unsigned long pts,
    int mode, int display, int skip, unsigned pid)
{
/**
Original BTP format
Different BTP modes are used to send data. Multi-packet blocks require two BTP's, one for the block minus one packet, and
one for the last packet. This is because the video transport engine does not count packets.
**/
    BNAV_PktFifoEntry  *curFifoEntry;

    BSTD_UNUSED(mode);
    BSTD_UNUSED(skip);
    BSTD_UNUSED(display);


    /* Send a command packet */
    CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle,pid));
    curFifoEntry->insertpackettype = eBpInsertBTP;
    curFifoEntry->pktdata[0] = TT_MODE_PROCESS;    /* MODE */

    if (feedEndByte - feedStartByte < handle->packetSize)
    {
        curFifoEntry->pktdata[7] = picStartByte - handle->timestampOffset;    /* DISCARD_HEADEND */
        curFifoEntry->pktdata[8] = picEndByte + 1 - handle->timestampOffset;  /* DISCARD_TAILEND */
        curFifoEntry->pktdata[9] = pts;  /* PTS */

        /* Send the single-packet frame data */
        /* SW7346-477, Coverity: 34617 */
        CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle,pid));
        curFifoEntry->insertpackettype = eBpInsertNone;
        curFifoEntry->startByte = feedStartByte;
        curFifoEntry->endByte = feedEndByte;
        return 0;
    }

    curFifoEntry->pktdata[7] = picStartByte - handle->timestampOffset;  /* DISCARD_HEADEND */
    curFifoEntry->pktdata[8] = 188;  /* DISCARD_TAILEND */
    curFifoEntry->pktdata[9] = pts;  /* PTS */

    /* Send the frame data minus the final packet */
    CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle,pid));
    curFifoEntry->insertpackettype = eBpInsertNone;
    curFifoEntry->startByte = feedStartByte;
    curFifoEntry->endByte = feedEndByte - handle->packetSize;

    if (picEndByte == handle->packetSize - 1)
    {
        /* we're sending more than one packet, but our last byte comes at the end of the packet. so we're done */
        curFifoEntry->endByte = curFifoEntry->endByte + handle->packetSize;
        return 0;
    }

    /* Fragmented final packet-->send a command packet */
    CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle,pid));
    curFifoEntry->insertpackettype = eBpInsertBTP;
    curFifoEntry->pktdata[0] = TT_MODE_PROCESS;   /* MODE */
    curFifoEntry->pktdata[7] = 0;  /* DISCARD_HEADEND */
    curFifoEntry->pktdata[8] = picEndByte + 1 - handle->timestampOffset;  /* DISCARD_TAILEND */
    curFifoEntry->pktdata[9] = pts;  /* PTS */

    /* Send the final packet */
    CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle,pid));
    curFifoEntry->insertpackettype = eBpInsertNone;
    curFifoEntry->startByte = feedEndByte - (handle->packetSize - 1);
    curFifoEntry->endByte = feedEndByte;
    return 0;
}

/* take a byte-accurate offset and size and compute the packet-aligned offsets and the starting point
in the first packet and ending point in the last packet. then call SendData which is responsible for
creating the BTP's to get the work done. */
int BNAV_Player_AddData(BNAV_Player_Handle handle, uint64_t offset, unsigned long size, unsigned long pts, int btpMode,
    int display, int skip, unsigned pid)
{
    unsigned long packet_start_byte, packet_end_byte;
    uint64_t end_offset = offset + size - 1; /* index of last byte */

    /* handle transport packet alignment */
    packet_start_byte = (unsigned long)(offset % handle->packetSize);
    packet_end_byte = (unsigned long)(end_offset % handle->packetSize);
    offset -= packet_start_byte;    /* set start byte of data to feed */
    end_offset += handle->packetSize - packet_end_byte - 1; /* set end byte of data to feed */

    if (!handle->settings.useBtpsForHostTrickModes) {
        /* This is a helpful technique, but should be unnecessary. Instead of trimming the beginning of the first packet,
        we can chose to send the whole packet. For many (but not all) streams, this will allow other data like the PES header,
        MPEG2 seqhdr or AVC SPS/PPS to be sent. This shouldn't be needed because the PTS should be set via the BTP and the seqhdr
        should be indexed and sent separately.
        PR 16794 - This is still required for legacy platforms. */
        if (handle->playMode != eBpPlayBrcm) {
            packet_start_byte = 0;
        }
    }

    BDBG_ASSERT((end_offset + 1) % handle->packetSize == 0);
    BDBG_ASSERT(offset % handle->packetSize == 0);

    return BNAV_Player_SendData(handle, packet_start_byte, packet_end_byte, offset, end_offset, pts, btpMode, display, skip, pid);
}

/**
Add a single frame (picture) to the fifo. Use whatever BTP's are necessary to get it decoded.
**/
static int BNAV_Player_AddFrameData(BNAV_Player_Handle handle, BNAV_Entry *pStartEntry)
{
    return BNAV_Player_AddData(handle,
        BNAV_get_frameOffset(pStartEntry),
        BNAV_get_frameSize(pStartEntry),
        BNAV_get_framePts(pStartEntry),
        TT_MODE_PROCESS, 0, 0, handle->pid);
}

static int BNAV_Player_AddDummyFrame(BNAV_Player_Handle handle, BNAV_Entry *pStartEntry)
{
    BNAV_PktFifoEntry  *curFifoEntry;
    unsigned long       pts = BNAV_get_framePts(pStartEntry);

    /* PROCESS BTP - only one is needed because the picture is only one packet */
    CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
    curFifoEntry->insertpackettype = eBpInsertBTP;
    curFifoEntry->pktdata[0] = TT_MODE_PROCESS;    /* MODE */
    curFifoEntry->pktdata[7] = 0;  /* DISCARD_HEADEND */
    curFifoEntry->pktdata[8] = 188;  /* DISCARD_TAILEND */
    curFifoEntry->pktdata[9] = pts;

    /* one packet picture */
    CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
    curFifoEntry->insertpackettype = eBpInsertDummy;
    return 0;
}

/**
Send a BTP mode command which is always followed by a seqhdr.
If mode == TT_MODE_PROCESS, then this is not really a BTP mode command. It is just sending a seqhdr.
**/
int BNAV_Player_AddSequenceHeader(BNAV_Player_Handle handle, BNAV_Entry *pStartEntry,
    int btpMode, int display, int skip)
{
    if (handle->navVersion == BNAV_Version_AVC || handle->navVersion == BNAV_Version_HEVC || handle->navVersion == BNAV_Version_VC1_PES) {
        /* send SPS and PPS, for now, every time */
        uint64_t sps_offset, pps_offset;
        unsigned long sps_size, pps_size, pts;
        BNAV_AVC_Entry *avcEntry = (BNAV_AVC_Entry *)pStartEntry;

        sps_offset = BNAV_get_frameOffset(pStartEntry) - BNAV_get_SPS_Offset(avcEntry);
        sps_size = BNAV_get_SPS_Size(avcEntry);
        pps_offset = BNAV_get_frameOffset(pStartEntry) - BNAV_get_seqHdrStartOffset(pStartEntry);
        pps_size = BNAV_get_seqHdrSize(pStartEntry);

        /* at the beginning of a stream, we might have some SPS or PPS indexes with no data. We can't
        decode these frames. */
        if (/* !sps_size  ||  */ !pps_size ) return -1;

        pts = BNAV_get_framePts(pStartEntry);
        if (sps_offset + sps_size == pps_offset) {
            /* send both together */
            BNAV_Player_AddData(handle, sps_offset, sps_size + pps_size, pts, 0, 0, 0, handle->pid);
        }
        else {
            /* send separately */
            BNAV_Player_AddData(handle, sps_offset, sps_size, pts, 0, 0, 0, handle->pid);
            BNAV_Player_AddData(handle, pps_offset, pps_size, pts, 0, 0, 0, handle->pid);
        }
    }
    else {
        uint64_t offset = BNAV_get_frameOffset(pStartEntry) - BNAV_get_seqHdrStartOffset(pStartEntry);
        unsigned long size = BNAV_get_seqHdrSize(pStartEntry);

        if (!size) return -1;

        if (btpMode != TT_MODE_PROCESS) {
            BNAV_PktFifoEntry  *curFifoEntry;
            /* If not a PROCESS command, then send the BTP command separately from the seqhdr data. */
            CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
            curFifoEntry->insertpackettype = eBpInsertBTP;
            curFifoEntry->pktdata[0] = btpMode;   /* MODE */
            curFifoEntry->pktdata[1] = skip;     /* SKIP_COUNT */
            curFifoEntry->pktdata[2] = display;   /* DISPLAY_COUNT */
            curFifoEntry->pktdata[9] = BNAV_get_framePts(pStartEntry);
        }

        BNAV_Player_AddData(handle, offset, size, BNAV_get_framePts(pStartEntry), btpMode, display, skip, handle->pid);
    }
    return 0;
}

/****************
The following "Generate" functions are called at GetNextPlayEntry time
when the internal fifo is consumed.
**/

/**
Generate a 1 packet dummy picture. Write it to memory pointed to by pkt parameter.
**/
static void BNAV_Player_GenerateDummyPicture(BNAV_Player_Handle handle, unsigned char *pkt, unsigned pid)
{
    static const unsigned char dummy_picture[] = {
        0x00, 0x00, 0x01, 0x00, 0x00, 0x09, 0x57, 0xa8,
        0x00, 0x00, 0x01, 0xb5, 0x8f, 0xff, 0xff, 0x1c, /* The 0xB5 extension is required for field-encoded MPEG2 brcm trick modes.
                                                           It will cause a problem with MPEG1 streams. If we encounter MPEG1 streams,
                                                           we may need to extend the API to get bcmplayer more information. */
        0x00, 0x00, 0x01, 0x01, 0x2a, 0x8b, 0xf0, 0xe4,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    pkt[handle->timestampOffset+0] = 0x47;   /* SYNC BYTE */
    pkt[handle->timestampOffset+1] = (pid >> 8) & 0x1f;
    pkt[handle->timestampOffset+2] = pid & 0xff;  /* PID */
    pkt[handle->timestampOffset+3] = 0x30; /* not scrambled, adaptation_field then payload, 0 continuity counter */
    pkt[handle->timestampOffset+4] = 188 - 5 - sizeof( dummy_picture ); /* leave only 32 bytes of payload */
    pkt[handle->timestampOffset+5] = 0x82;
    memset(&pkt[handle->timestampOffset+6], 0, 188-6-sizeof(dummy_picture) ); /* zero out adaptation field */
    memcpy(&pkt[handle->timestampOffset+188-sizeof(dummy_picture)], dummy_picture, sizeof(dummy_picture)); /* copy dummy picture */
}

/**
Generate a null packet. Write it to memory pointed to by pkt parameter.
**/
static void BNAV_Player_GenerateNullPacket(BNAV_Player_Handle handle, unsigned char *pkt)
{
    memset(pkt, 0, handle->packetSize);
    pkt[handle->timestampOffset+0] = 0x47;   /* SYNC BYTE */
    pkt[handle->timestampOffset+1] = 0;
    pkt[handle->timestampOffset+2] = 0;     /* zero pid so it doesn't get filtered out */
    pkt[handle->timestampOffset+3] = 0x10; /* not scrambled, not adaptation field, 0 continuity counter */
}

/**
Generate a packet with a single start code.
**/
static void BNAV_Player_GenerateStartCodePacket(BNAV_Player_Handle handle, unsigned char *pkt, uint16_t startcode, uint16_t pid)
{
    unsigned payload_size;
    unsigned char *payload;

    pkt[handle->timestampOffset+0] = 0x47;   /* SYNC BYTE */
    pkt[handle->timestampOffset+1] = (pid >> 8) & 0x1f;
    pkt[handle->timestampOffset+2] = pid & 0xff;  /* PID */
    pkt[handle->timestampOffset+3] = 0x30; /* not scrambled, adaptation_field then payload, 0 continuity counter */
    payload_size = handle->navVersion == BNAV_Version_HEVC ? 5 : 4;
    pkt[handle->timestampOffset+4] = 188-(5+payload_size); /* leave only payload_size bytes of payload */
    memset(&pkt[handle->timestampOffset+5], 0, 188-5); /* zero out rest of MPEG-2 TS packet */
    payload = pkt + handle->timestampOffset+ (188-(5+payload_size));
    payload[0] = 0x00;
    payload[1] = 0x00;
    payload[2] = 0x01;
    if (handle->navVersion == BNAV_Version_HEVC) {
        payload[3] = startcode>>8;
        payload[4] = startcode;
    } else {
        payload[3] = startcode;
    }
    return;
}

/**
Generate a BTP packet. Write it to memory pointed to by pkt parameter.
**/
void BNAV_Player_GenerateBTP(BNAV_Player_Handle handle, unsigned long *params, unsigned char *pkt,
    int discarding_tailend, unsigned pid)
{
    unsigned i;

    pkt[handle->timestampOffset+0] = 0x47;   /* SYNC BYTE */
    pkt[handle->timestampOffset+1] = (pid >> 8) & 0x1f;
    pkt[handle->timestampOffset+2] = pid & 0xff;  /* PID */
    pkt[handle->timestampOffset+3] = 0x20; /* not scrambled, adaptation_field and no payload, 0 continuity counter */
    pkt[handle->timestampOffset+4] = 0xb7; /* adaptation field length is 183 - the remainder of the packet */
    if (discarding_tailend)
        pkt[handle->timestampOffset+5] = 0x02; /* don't set discontinuity_indicator in this condition */
    else
        pkt[handle->timestampOffset+5] = 0x82;
    pkt[handle->timestampOffset+6] = 45; /* Number of relevant bytes */
    pkt[handle->timestampOffset+7] = 0x00;  /* Align byte */
    pkt[handle->timestampOffset+8] = 0x42;  /* B */
    pkt[handle->timestampOffset+9] = 0x52;  /* R */
    pkt[handle->timestampOffset+10] = 0x43; /* C */
    pkt[handle->timestampOffset+11] = 0x4d; /* M */
    for(i=0; i<10; ++i)
    {
        int base = handle->timestampOffset + 12 + i*4;
        pkt[base] = (unsigned char) ((params[i] & 0xff000000) >> 24);
        pkt[base+1] = (unsigned char) ((params[i] & 0x00ff0000) >> 16);
        pkt[base+2] = (unsigned char) ((params[i] & 0x0000ff00) >> 8);
        pkt[base+3] = (unsigned char) (params[i] & 0x000000ff);
    }
    for(i=handle->timestampOffset+52; i<handle->packetSize; ++i)
    {
        pkt[i] = 0x00;
    }
}

#if !B_REFSW_MINIMAL
long BNAV_Player_FindStartCodeIndex(BNAV_Player_Handle handle, long startIndex, eSCType startCode, eBpDirectionParam dir)
{
    BNAV_Entry      *pEntry;
    char            startCodeFound = 0;

    while (!startCodeFound)
    {
        CHECKREAD(pEntry = BNAV_Player_ReadEntry(handle, startIndex));

        if (startCode == eSCTypeUnknown)
        {
            startCodeFound = 1;
        }
        else if (BNAV_get_frameType(pEntry) == startCode)
        {
            startCodeFound = 1;
        }
        else
        {
            startIndex += dir;
        }
    }

    return startIndex;
}
#endif

static long BNAV_Player_P_FindPictureIndex(BNAV_Player_Handle handle, long startIndex, eSCType picCode, eBpDirectionParam dir)
{
    eBpDirectionParam searchDir = dir;

    while (1)
    {
        BNAV_Entry *pEntry;
        eSCType frametype;

        pEntry = BNAV_Player_ReadEntry(handle, startIndex);
        if (!pEntry)
        {
            /* TODO: please consider moving this switch of direction to the app. I don't think it's general case. */
            if (searchDir == dir)
            {
                startIndex -= dir;      /* go back one and switch directions */
                searchDir = (eBpDirectionParam)(dir * -1);
                continue;
            }
            else
            {
                return -1;              /* no match found */
            }
        }

        frametype = BNAV_get_frameType(pEntry);
        if ((picCode == eSCTypeIFrame && frametype == eSCTypeRPFrame) ||
            (picCode == eSCTypeRPFrame && frametype == eSCTypeIFrame))
        {
            /* If we're looking for I and find RP, or vice versa, bail out because
            we may never find the right one. This isn't good for a mixed
            HITS/GOP-based stream, but there's no way to handle this well. */
            BDBG_WRN(("BNAV_Player_P_FindPictureIndex: detected HITS/GOP-based mismatch."));
            return -1;
        }
        if (picCode == eSCTypeUnknown || frametype == picCode)
        {
            break;
        }
        else
        {
            startIndex += dir;
        }
    }
    return startIndex;
}

#define B_PTSCACHE_PREV(handle, index) ((index)?(index)-1:((handle)->ptscache.cacheSize)-1)
#define B_PTSCACHE_NEXT(handle, index) ((index)==((handle)->ptscache.cacheSize)-1?0:(index)+1)

/* Enable this code for debug of cache entries */
static void print_pts_cache(BNAV_Player_Handle handle)
{
#if 0
    const struct BNAV_Player_P_PtsCache *ptscache = &handle->ptscache;
    unsigned i;
    /* dump entire PTS cache from oldest to newest */
    BKNI_Printf("PTS cache: %d..%d\n", ptscache->rptr, ptscache->wptr);
    for (i=ptscache->rptr;i!=ptscache->wptr;i=B_PTSCACHE_NEXT(handle, i)) {
        BKNI_Printf("%d: %#x @ %d\n", i, ptscache->cache[i].pts, ptscache->cache[i].index);
    }
#else
    BSTD_UNUSED(handle);
#endif
}

void BNAV_Player_P_AddToPTSCache(BNAV_Player_Handle handle, uint32_t pts, long index)
{
    if (handle->ptscache.wptr != handle->ptscache.rptr) {
        /* discard if this duplicates the previous */
        unsigned prev = B_PTSCACHE_PREV(handle, handle->ptscache.wptr);
        if (handle->ptscache.cache[prev].pts == pts) {
            return;
        }

/* assumes that decode order may be reordered from display order by a maximum of 1 second.
for MPEG, this is simply the maximum number of adjacent B's. for AVC, this could include explicit display reordering.
it is limited by GOP size, which is practically limited by channel change time. */
#define B_REORDER_THRESHOLD (45*1000)

/* the maximum that TSM will hold a picture. anything greater will be labeled as a PTS discontinuity. */
#define B_DISCARD_THRESHOLD (45*5000)

/* coded for worst case of one PTS per GOP, GOP of 2 seconds in 45Khz PTS units */
#define B_REQUIRED_MIN_DIFF (45000*2)

        /* only reorder and mark discontinuities only in normal play */
        if (handle->playMode == eBpPlayNormal) {
            /* check thresholds,
            newer-older gives the right result even if newer<older, as long as all values are unsigned */
            if (handle->ptscache.cache[prev].pts - pts <= (unsigned)B_REORDER_THRESHOLD) {
                /* reorder pictures so that bcmplayer's ptscache is in display order.
                for MPEG, this is only B frames, but for AVC is could be complex. so just search backwards and insert. */
                unsigned i = handle->ptscache.wptr;
                while (1) {
                    handle->ptscache.cache[i] = handle->ptscache.cache[prev];
                    i = prev;
                    prev = B_PTSCACHE_PREV(handle, prev);
                    if (prev == handle->ptscache.rptr ||
                        handle->ptscache.cache[prev].pts <= pts ||
                        handle->ptscache.cache[prev].sectionCounter != handle->ptscache.sectionCounter)
                    {
                        break;
                    }
                    /* BDBG_MSG_PTSCACHE((" reordering %d, %#x %#x", prev, handle->ptscache.cache[prev].pts, pts)); */
                }
                handle->ptscache.cache[i].pts = pts;
                handle->ptscache.cache[i].index = index;
                handle->ptscache.cache[i].sectionCounter = handle->ptscache.sectionCounter;
                BDBG_MSG_PTSCACHE(("AddToPTSCache (%ld,%#lx) reordered to %d instead of %d (section %d)", index, pts, i, handle->ptscache.wptr, handle->ptscache.sectionCounter));
                goto advance;
            }
            else if (pts - handle->ptscache.cache[prev].pts > (unsigned)B_DISCARD_THRESHOLD || pts - handle->ptscache.cache[prev].pts > (unsigned)B_REORDER_THRESHOLD) {
                /* mark a discontinuity */
                /* BDBG_MSG_PTSCACHE(("AddToPTSCache discontinuity %u-%u=%u > %u || %u-%u=%u > %u, %d %d",
                    pts, handle->ptscache.cache[prev].pts, pts - handle->ptscache.cache[prev].pts, B_DISCARD_THRESHOLD,
                    pts, handle->ptscache.cache[prev].pts, handle->ptscache.cache[prev].pts-pts,B_REORDER_THRESHOLD,
                    (unsigned)pts - (unsigned)handle->ptscache.cache[prev].pts > (unsigned)B_DISCARD_THRESHOLD,
                    (unsigned)pts - (unsigned)handle->ptscache.cache[prev].pts > (unsigned)B_REORDER_THRESHOLD)); */
                handle->ptscache.sectionCounter++;
            }
        }
    }

    handle->ptscache.cache[handle->ptscache.wptr].pts = pts;
    handle->ptscache.cache[handle->ptscache.wptr].index = index;
    handle->ptscache.cache[handle->ptscache.wptr].sectionCounter = handle->ptscache.sectionCounter;
    BDBG_MSG_PTSCACHE(("AddToPTSCache (%ld,%#lx) at %d (section %d)", index, pts, handle->ptscache.wptr, handle->ptscache.sectionCounter));
advance:
    handle->ptscache.wptr = B_PTSCACHE_NEXT(handle, handle->ptscache.wptr);
    if (handle->ptscache.wptr == handle->ptscache.rptr) {
        /* the cache is full. try to increate the buffer. */
        unsigned larger_size = handle->ptscache.cacheSize * 2;
        if (larger_size > MAX_PTS_CACHE_SIZE) {
            BDBG_ERR(("bcmplayer ptscache full at %u. Is BNAV_Player_DiscardToPts being called?", handle->ptscache.cacheSize));
        }
        else {
            struct BNAV_Player_P_PtsCacheEntry *ptr;
            BDBG_WRN(("expanding bcmplayer ptscache from %u to %u", handle->ptscache.cacheSize, larger_size));
            ptr = malloc(larger_size * sizeof(struct BNAV_Player_P_PtsCacheEntry));
            if (!ptr) {
                BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                /* keep going with the memory we have */
            }
            else {
                unsigned i = handle->ptscache.rptr, wptr = 0;
                do {
                    ptr[wptr++] = handle->ptscache.cache[i];
                    i = B_PTSCACHE_NEXT(handle, i);
                } while (i != handle->ptscache.wptr);
                handle->ptscache.cacheSize = larger_size;
                handle->ptscache.rptr = 0;
                handle->ptscache.wptr = wptr;
                free(handle->ptscache.cache);
                handle->ptscache.cache = ptr;
            }
        }
    }
    return;
}

static void BNAV_Player_P_FlushPTSCache(BNAV_Player_Handle handle)
{
    BDBG_MSG_PTSCACHE(("BNAV_Player_P_FlushPTSCache"));
    handle->ptscache.wptr = handle->ptscache.rptr = 0; /* empty */
    handle->ptscache.sectionCounter = 0;
}

static int BNAV_Player_P_AddCurrentToPTSCache(BNAV_Player_Handle handle)
{
    BNAV_Entry *pEntry;
    pEntry = BNAV_Player_ReadEntry(handle, handle->currentIndex);
    if (!pEntry)
        return -1;

    BNAV_Player_P_AddToPTSCache(handle, BNAV_get_framePts(pEntry), handle->currentIndex);

    return 0;
}

void BNAV_Player_SetBackwardPtsCacheSearch( BNAV_Player_Handle handle ) {

    handle->ptscache.doBackwardPtsSearch = 1;
}

/* internal function that returns index into ptscache */
static int BNAV_Player_P_FindIndexFromPts(BNAV_Player_Handle handle, unsigned long targetPts)
{
    unsigned mindiff = 0xFFFFFFFF;
    unsigned minindex = 0;
    struct BNAV_Player_P_PtsCache *ptscache = &handle->ptscache;
    unsigned i;
    unsigned startindex;
    unsigned endindex; /* not inclusive */
    int increment;
    bool ptsdecreasing;

    if (ptscache->wptr == ptscache->rptr) {
        BDBG_ERR(("pts search: no entries in cache"));
        return -1;
            }

    /* because we've inserted into the cache in display order, we just search until we pass or match it,
    then decide on the best match at that point */
    if (ptscache->doBackwardPtsSearch) {
        /* doBackwardPtsSearch probably should be deprecated. no real use case and will likely return
        bad results for streams with PTS continuities (which is most streams). */
        startindex = B_PTSCACHE_PREV(handle, ptscache->wptr);
        endindex = B_PTSCACHE_NEXT(handle, ptscache->rptr);
        increment = -1;
        ptsdecreasing = (handle->playDir < 0);
    }
    else {
        /* a normal search means starting from the oldest and searching to the newest.
        for normal play or fwd trick modes, PTS's are decreasing in the cache. for rev trick modes,
        PTS's are increasing in the cache. */
        startindex = ptscache->rptr;
        endindex = ptscache->wptr;
        increment = 1;
        ptsdecreasing = (handle->playDir > 0);
    }

    /* search until
    1) we get an exact match
    2) we pass the target PTS but already found a match <= B_REQUIRED_MIN_DIFF
    3) we search the entire cache
    */
    for (i=startindex; i!=endindex; i=((increment>0)?B_PTSCACHE_NEXT(handle, i):B_PTSCACHE_PREV(handle, i))) {
        unsigned thisdiff;
        bool past = false;

        if (ptsdecreasing) {
            if (ptscache->cache[i].pts >= targetPts) {
                past = true;
                thisdiff = ptscache->cache[i].pts - targetPts;
        }
            else {
                thisdiff = targetPts - ptscache->cache[i].pts;
                }
            }
        else {
            if (ptscache->cache[i].pts <= targetPts) {
                past = true;
                thisdiff = targetPts - ptscache->cache[i].pts;
            }
            else {
                thisdiff = ptscache->cache[i].pts - targetPts;
            }
                }
        if (thisdiff < mindiff) {
            mindiff = thisdiff;
            minindex = i;
            }
        if (!mindiff || (past && mindiff < B_REQUIRED_MIN_DIFF)) {
            break;
        }
    }

    /* best result is minindex */
    BDBG_MSG_PTSCACHE(("pts search: found %#x @ %d (%d in %d..%d) for %#x", ptscache->cache[minindex].pts, ptscache->cache[minindex].index,
                                                                            i, ptscache->rptr, ptscache->wptr, targetPts));

    if (mindiff <= B_REQUIRED_MIN_DIFF) {
        /* match */
        return minindex;
    }

    /* not found. don't print error in this internal function. */
    return -1;
}

/* external function that returns NAV index */
long BNAV_Player_FindIndexFromPts(BNAV_Player_Handle handle, unsigned long targetPts, int searchWindow)
{
    int index;
    BSTD_UNUSED(searchWindow);

    index = BNAV_Player_P_FindIndexFromPts(handle, targetPts);
    if (index != -1) {
        return handle->ptscache.cache[index].index;
    }
    else {
        /* if this happens, please follow these steps:
        ensure that BNAV_Player_FindIndexFromPts is only being called for the decoder's current PTS; any other use
            lacks context and is not reliable. use timestamp instead.
        consider accepting the failure and setting your current index, not based on PTS, but based
        on your current player location (BNAV_Player_GetPosition) plus an offset based on playback/video depth.
        it will not be frame accurate, but will likely be better than reworking this algorithm. */
        BDBG_ERR(("pts search: couldn't find %#x", (unsigned)targetPts));
        print_pts_cache(handle);
#if 0
        BKNI_Fail();
#endif
        return -1;
    }
}

void BNAV_Player_DiscardToPts(BNAV_Player_Handle handle, uint32_t pts)
{
    int index = BNAV_Player_P_FindIndexFromPts(handle, pts);
    BDBG_MSG_PTSCACHE(("BNAV_Player_DiscardToPts %#x ==> %d", pts, index));
    if (index != -1) {
        handle->ptscache.rptr = index;
    }
}


long BNAV_Player_FindIndexFromPtsAndStartingIndex( BNAV_Player_Handle handle, unsigned long target_pts, long startingIndex, long endingIndex, unsigned threshold )
{
    BNAV_Entry *pEntry;
    long index;
    int direction = (endingIndex>startingIndex) ? 1 : -1;

    if (threshold == (unsigned)-1) {
        threshold = 1500; /* 33 msec in 32 MHz PTS units */
    }

    for (index = startingIndex; (endingIndex>startingIndex) ? index < endingIndex : index > endingIndex; index += direction) {
        uint32_t pts;

        pEntry = BNAV_Player_ReadEntry(handle, index);
        if (!pEntry) break;

        pts = BNAV_get_framePts(pEntry);
        /* is this PTS within the search threshold? this algo assumes PTSs can be discontinuous, so stopping when pts goes past
        target PTS is not terminal. */
        if (pts > target_pts) {
            if (pts - target_pts <= threshold) return index;
        }
        else {
            if (target_pts - pts <= threshold) return index;
        }
    }
    return -1;
}

long BNAV_Player_P_FindIndexForOffset(BNAV_Player_Handle handle, uint64_t offset, void *context, int (*read_offset)(void *, long, uint64_t *))
{
    int rc;
    char            entryFound = 0;
    long            index;
    int             dir = 0;
    uint64_t        firstByteOffset, lastByteOffset, testOffset = 0;
    int count = 0;
    
    /* get first and last timestamp of mpeg */
    if (BNAV_Player_UpdateBounds(handle))
        return -1;    

    rc = read_offset(context, handle->firstIndex, &firstByteOffset);
    if(rc!=0) {goto error;}
    rc = read_offset(context, handle->lastIndex, &lastByteOffset);
    if(rc!=0) {goto error;}

    /* check bounds */
    if (handle->settings.strictFind) {
        if (offset < firstByteOffset || offset > lastByteOffset) {
            return -1;
        }
        else if (offset == firstByteOffset) {
            return handle->firstIndex;
        }
        else if (offset == lastByteOffset) {
            return handle->lastIndex;
        }
    }
    else {
        if (firstByteOffset >= offset)
            return handle->firstIndex;
        if (offset >= lastByteOffset)
            return handle->lastIndex;
    }    

    /**
    * interpolate to an index
    *
    * Here's the straightforward math for clarity:
    *
    * index = (long)(((offset - firstByteOffset) / (float)(lastByteOffset - firstByteOffset) *
    *   (handle->lastIndex - handle->firstIndex)) + handle->firstIndex;
    *
    **/
    {
        uint64_t offsetWholeDiff = lastByteOffset - firstByteOffset;
        uint64_t offsetPartDiff = offset - firstByteOffset;
        unsigned indexDiff = handle->lastIndex - handle->firstIndex;

        /* reduce the terms before doing the interpolation to prevent overflow. */
        while (offsetPartDiff >= ((uint64_t)1)<<32) {
            offsetPartDiff /= 2;
            offsetWholeDiff /= 2;
        }
        if (offsetWholeDiff == 0) {
            index = handle->firstIndex;
        }
        else {
            uint64_t index64 = (unsigned)handle->firstIndex + ((indexDiff * offsetPartDiff) / offsetWholeDiff);
            if(index64 < (unsigned)handle->lastIndex) {
                index = (long) index64;
            } else {
                index = handle->lastIndex;
            }
        }
    }

    /* linear search */
    while (!entryFound) {
        rc = read_offset(context, index, &testOffset);
        if(rc!=0) {goto error;}

        if (testOffset > offset)
        {
            if (dir == 0)
                dir = -1;
            if (dir > 0)
            {
                entryFound = 1;
                /* back up one */
                index -= dir;
            }
            else
                index += dir;
        }
        else
        {
            if (dir == 0)
                dir = 1;
            if (dir < 0)
                entryFound = 1;
            else
                index += dir;
        }
        count++;
    }

    BDBG_MSG(("IndexFromOffset, %d loops, diff %ld", count,
        (unsigned long)(testOffset - offset)));
    return index;

error:
    return -1;
}

static int BNAV_Player_P_ReadOffset(void *handle_, long index, uint64_t *offset)
{
    BNAV_Player_Handle handle = handle_;
    BNAV_Entry  *pEntry;

    CHECKREAD(pEntry= BNAV_Player_ReadEntry(handle, index))
    *offset = BNAV_get_frameOffset(pEntry);
    return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:   handle = pointer to a previously allocated sTsPlayer structure
+           offsetHi = upper 32 bits of the search offset
+           offset = lower 32 bits of the search offset
+ OUTPUTS:  None.
+ RETURNS:  SCT index covering this byte offset. -1 if seeks fail.
+ FUNCTION: This function searches the SCT table for an entry that corresponding
+           to the specified byte offset.  If the byte offset is less than the
+           offset of the first entry if will return the first entry.  If the
+           offset is greater than the offset of the last entry, it will return
+           the last entry.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
long BNAV_Player_FindIndexFromOffset(BNAV_Player_Handle handle,
    unsigned long offsetHi, unsigned long offsetLow)
{
    uint64_t        offset;

    offset = create64(offsetHi, offsetLow);
    return BNAV_Player_P_FindIndexForOffset(handle, offset, handle, BNAV_Player_P_ReadOffset);
}

long BNAV_Player_P_SearchIndexByTimestamp(BNAV_Player_Handle handle, unsigned long timestamp, void *context, int (*read_timestamp)(void *, long, unsigned long *))
{
    char            entryFound = 0;
    long            index;
    int             dir = 0;
    unsigned long firstTimestamp, lastTimestamp, testTimestamp = 0;
    int count = 0;
    int rc;

    /* get first and last timestamp of mpeg */
    if (BNAV_Player_UpdateBounds(handle))
        return -1;

    rc = read_timestamp(context, handle->firstIndex, &firstTimestamp);
    if(rc<0) {goto error;}

    rc = read_timestamp(context, handle->lastIndex, &lastTimestamp);
    if(rc<0) {goto error;}

    BDBG_MSG(("IndexFromTimestamp: timestamps %ld,%ld,%ld",
        firstTimestamp,timestamp,lastTimestamp));

    /* check bounds */
    if (handle->settings.strictFind) {
        if (timestamp < firstTimestamp || timestamp > lastTimestamp) {
            return -1;
        }
        else if (timestamp == firstTimestamp) {
            return handle->firstIndex;
        }
        else if (timestamp == lastTimestamp) {
            return handle->lastIndex;
        }
    }
    else {
        if (firstTimestamp >= timestamp)
            return handle->firstIndex;
        if (timestamp >= lastTimestamp)
            return handle->lastIndex;
    }

    /* interpolate to an index */
    {
        unsigned timestampPartDiff = timestamp - firstTimestamp;
        unsigned timestampWholeDiff = lastTimestamp - firstTimestamp;
        unsigned indexDiff = handle->lastIndex - handle->firstIndex;

        if (timestampWholeDiff == 0) {
            index = handle->firstIndex;
        }
        else {
            uint64_t index64 = (timestampPartDiff * (uint64_t)indexDiff / timestampWholeDiff) + handle->firstIndex;
            /* because of rounding errors in the /2 logic, we must rebound it */
            if (index64 > (unsigned)handle->lastIndex) {
                index = handle->lastIndex;
            } else {
                index = (long)index64;
            }
        }
    }

    BDBG_MSG(("IndexFromTimestamp: indexes %ld,%ld,%ld, cur %ld",
        handle->firstIndex,index,handle->lastIndex,handle->currentIndex));
    if ((index == handle->firstIndex) || (index == handle->lastIndex)) {
        /* Shortcut if this is a boundary condition */
        return index;
    }
    /* linear search */
    while (!entryFound) {
        rc = read_timestamp(context, index, &testTimestamp);
        if(rc<0) {goto error;}

        if (testTimestamp > timestamp)
        {
            if (dir == 0)
                dir = -1;
            if (dir > 0)
            {
                entryFound = 1;
                /* back up one */
                index -= dir;
            }
            else
                index += dir;
        }
        else if (testTimestamp == timestamp)
        {
            entryFound = 1;
        }
        else
        {
            if (dir == 0)
                dir = 1;
            if (dir < 0)
                entryFound = 1;
            else
                index += dir;
        }
        count++;
    }

    BDBG_MSG(("IndexFromTimestamp, %d loops, diff %ld, index %ld", count, testTimestamp - timestamp, index));
    return index;
error:
    return -1;
}

static int BNAV_Player_P_ReadTimestamp(void *handle_, long index, unsigned long *timestamp)
{
    BNAV_Player_Handle handle = handle_;
    BNAV_Entry  *pEntry;

    CHECKREAD(pEntry= BNAV_Player_ReadEntry(handle, index))
    *timestamp = BNAV_get_timestamp(pEntry);
    return 0;
}

/**
* IndexFromTimestamp has identical logic to IndexFromOffset
**/
long BNAV_Player_FindIndexFromTimestamp(BNAV_Player_Handle handle, unsigned long timestamp)
{
    if(handle->navVersion == BNAV_Version_AVC_Extended) {
        return BNAV_Player_SVC_FindIndexFromTimestamp(&handle->svc, timestamp);
    } else {
        return BNAV_Player_P_SearchIndexByTimestamp(handle, timestamp, handle, BNAV_Player_P_ReadTimestamp);
    }
}

#if !B_REFSW_MINIMAL
void BNAV_Player_PrintCachePerformance(const BNAV_Player_Handle handle) {
    int hits = handle->navCacheReads-handle->navCacheMisses;
    printf("cache: %d hits, %ld misses, %ld fails. %0.1f%% hit ratio\n",
        hits,
        handle->navCacheMisses,
        handle->navCacheFails,
        100.0*hits/handle->navCacheReads);
}
#endif

int BNAV_Player_ReadIndex(BNAV_Player_Handle handle, int index, BNAV_Entry *entry) {
    const BNAV_Entry *pEntry;
    if (sizeof(*entry) != BNAV_GetEntrySize(handle->settings.navVersion)) {
        return BERR_TRACE(-1);
    }
    pEntry = BNAV_Player_ReadEntry(handle, index);
    if (!pEntry) {
        return BERR_TRACE(-1);
    }
    *entry = *pEntry;
    return 0;
}

int BNAV_Player_ReadAvcIndex(BNAV_Player_Handle handle, int index, BNAV_AVC_Entry *entry) {
    const BNAV_AVC_Entry *pEntry;
    if (sizeof(*entry) != BNAV_GetEntrySize(handle->settings.navVersion)) {
        return BERR_TRACE(-1);
    }
    pEntry = (const BNAV_AVC_Entry *)BNAV_Player_ReadEntry(handle, index);
    if (!pEntry) {
        return BERR_TRACE(-1);
    }
    *entry = *pEntry;
    return 0;
}

#if !B_REFSW_MINIMAL
int BNAV_Player_IsHits(BNAV_Player_Handle handle, int searchRange) {
    int index = 0;

    if(handle->navVersion == BNAV_Version_AVC_Extended) {
        return 0;
    }

    while (searchRange--) {
        int frametype;
        BNAV_Entry *i = BNAV_Player_ReadEntry(handle, index++);
        if (!i)
            break;
        frametype = BNAV_get_frameType(i);
        if (frametype == eSCTypeRPFrame)
            return 1;
        else if (frametype == eSCTypeIFrame)
            return 0;
    }
    return -1;
}
#endif

static int BNAV_Player_DetectNavTableVersion(BNAV_Player_Handle handle, int isPes)
{
    BNAV_Entry *pEntry;
    long checkIndex;

    BSTD_UNUSED(isPes);

    if (BNAV_Player_UpdateBounds(handle)) {
        return -1;
    }
    /* AVC NAV entry is a multiple of 2 of an MPEG NAV entry. so, a simpler solution is to insist that navVersion
    detection is on even index numbers. */
    checkIndex = handle->firstIndex;
    if (checkIndex % 2) {
        checkIndex++;
        if (checkIndex > handle->lastIndex) {
            BDBG_ERR(("cannot detect NAV version if index is non-zero"));
            return -1;
        }
    }
    pEntry = BNAV_Player_ReadEntry(handle, checkIndex);
    if (!pEntry) {
        BDBG_ERR(("BNAV_Player_DetectNavTableVersion: no entry %d", (int)checkIndex));
        return -1;
    }
    handle->navVersion = BNAV_get_version(pEntry);
    return 0;
}

void BNAV_Player_GetPlayMode(const BNAV_Player_Handle handle, BNAV_Player_PlayMode *mode) {
    mode->playMode = handle->playMode;
    mode->playModeModifier = handle->advanceCount * handle->playDir;
    mode->loopMode = handle->loopMode;
    mode->disableExtraBOptimization = handle->disableExtraBOptimization;
}

int BNAV_Player_SetPlayMode(BNAV_Player_Handle handle, const BNAV_Player_PlayMode *mode)
{
    int playModeModifier = mode->playModeModifier;

    BDBG_MSG(("setPlayMode mode=%d,mode_modifier=%d,loop=%d", mode->playMode, mode->playModeModifier, mode->loopMode));

    if (handle->navVersion == BNAV_Version_TimestampOnly) {
        if (mode->playMode != eBpPlayNormal) {
            BDBG_ERR(("BNAV_Version_TimestampOnly only supports eBpPlayNormal"));
            return -1;
        }
    }

    /* Validate the parameters. */
    if (mode->playMode == eBpPlayIP || mode->playMode == eBpPlayNormal || mode->playMode == eBpPlayNormalByFrames) {
        playModeModifier = 1;
    }
    else if (is_dqt(mode->playMode)) {
        /* Check if DQT can be supported using the nav file */
        if (handle->navVersion == BNAV_Version_AVC || handle->navVersion == BNAV_Version_HEVC){
            long        startIndex;
            startIndex = BNAV_Player_P_FindGOP(handle, 0);
            if(!BNAV_Player_ReadEntry(handle, startIndex)){
                BDBG_ERR(("AVC DQT requires Random Access Indicator (RAI) in index. Index must be re-recorded, and if there are no IDR's it must include TPIT filtering for RAI."));
                return -1;
            }
        }
        switch (mode->playMode) {
        case eBpPlayDecoderMDqtTrick:
            handle->gopTrick.gopSkip = 0;
            playModeModifier = -1;
            break;
        default:
            handle->gopTrick.gopSkip = i_abs(playModeModifier)/100;
            playModeModifier = playModeModifier % 100;
            if (playModeModifier == 0) {
                BDBG_ERR(("DQT cannot have playModeModifier == 0."));
                return -1;
            }
            break;
        }
    }
    else if (playModeModifier == 0) {
        BDBG_ERR(("PlayMode cannot have playModeModifier == 0."));
        return -1;
    }
    else if (playModeModifier < 0 && mode->playMode != eBpPlayI && mode->playMode != eBpPlayBrcm && mode->playMode != eBpPlayTimeSkip) {
        BDBG_ERR(("PlayModeModifier cannot be <0 for this mode."));
        return -1;
    }

    if (handle->navVersion == BNAV_Version_VC1_PES || handle->navVersion == BNAV_Version_AVC || handle->navVersion == BNAV_Version_HEVC || handle->navVersion == BNAV_Version_AVC_Extended) {
        if (mode->playMode == eBpPlayBrcm) {
            BDBG_ERR(("VC1/AVC Broadcom Trick Modes not supported"));
            return -1;
        }
    }
    if (handle->navVersion == BNAV_Version_AVC || handle->navVersion == BNAV_Version_HEVC || handle->navVersion == BNAV_Version_AVC_Extended) {
        if (mode->playMode == eBpPlaySkipP) {
            /* AVC P frames don't have unidirectional prediction, so we can't drop some P's. */
            BDBG_ERR(("SkipP does not work with the AVC codec"));
            return -1;
        }
    }

    if (mode->playMode == eBpPlayBrcm && !handle->decoderFeatures.supportsBroadcomTrickModes) {
        BDBG_ERR(("Decoder does not support Broadcom Trick Modes."));
        return -1;
    }
    if (mode->playMode == eBpPlayBrcm && playModeModifier == 1) {
        BDBG_ERR(("Broadcom Trick Modes cannot have mode modifier of 1."));
        return -1;
    }

    /* loop mode can change */
    handle->loopMode = mode->loopMode;

    /* if we're staying in playmode, don't change anything past this point */
    if (mode->playMode == handle->playMode && handle->playMode == eBpPlayNormal) {
        return 0;
    }

    handle->playMode = mode->playMode;
    handle->playDir = playModeModifier>0?eBpForward:eBpReverse;
    handle->advanceCount = playModeModifier>0?playModeModifier:-playModeModifier;
    handle->curRefFrame = -1;
    handle->disableExtraBOptimization = mode->disableExtraBOptimization;
    BNAV_Player_SetCurrentIndex(handle, handle->currentIndex);
    return 0;
}

long BNAV_Player_FindIFrameFromIndex(BNAV_Player_Handle handle, long index, eBpDirectionParam dir)
{
    if (index == -1)
        index = handle->currentIndex;

    if(handle->navVersion == BNAV_Version_AVC_Extended) {
        return BNAV_Player_SVC_FindIFrameFromIndex(&handle->svc, index, dir);
    }

    if (handle->navVersion == BNAV_Version_TimestampOnly) {
        return index;
    }

    /* verify that dir is always 1/-1, because this algorithm breaks if you pass in 0. */
    dir = (dir > 0)?1:-1;

    index = BNAV_Player_P_FindPictureIndex(handle, index, eSCTypeIFrame, dir);

    /* TODO: bcmindexer now indexes MPEG field encoded streams as a single NAV entry per field pair.
    if we can do the same for AVC, then this code can be removed. this would be best because backing up one
    I picture is basically a guess. */
    if (dir < 0) {
        long previousIndex;
        previousIndex = BNAV_Player_P_FindPictureIndex(handle, index-1, eSCTypeIFrame, dir);
        if (index-previousIndex == 1) {
            /* When searching backwards in field encoded streams, return the 1st field in the field pair, not the 2nd */
            BDBG_MSG(("1st field index=%d, 2nd field index=%d", (int)previousIndex, (int)index));
            index = previousIndex;
        }
    }

    return index;
}

bool BNAV_Player_IndexIsBFrame( BNAV_Player_Handle handle, long index ) {
    BNAV_Entry *pEntry = 0;
    eSCType frametype;
    bool rc=false;

    if(handle->navVersion == BNAV_Version_AVC_Extended) {
        return BNAV_Player_SVC_IndexIsBFrame(&handle->svc, index);
    }

    pEntry = BNAV_Player_ReadEntry(handle, index);
    if( pEntry ) {
            frametype = BNAV_get_frameType(pEntry);
            if ( frametype == eSCTypeBFrame ) rc = true;
    }

    return rc;
}

int BNAV_Player_SetCurrentIndex(BNAV_Player_Handle handle, long index)
{
    BNAV_Entry *pEntry;

    BDBG_MSG(("SetCurrentIndex %ld", index));
    pEntry = BNAV_Player_ReadEntry(handle, index);
    if (!pEntry) {
        BDBG_WRN(("SetCurrentIndex failed: %ld ReadEntry", index));
        return -1;
    }

    handle->currentIndex = index;
    if(handle->navVersion == BNAV_Version_AVC_Extended) {
        BNAV_Player_SVC_SetCurrentIndex(&handle->svc, index);
    } else if (handle->playMode == eBpPlayNormal) {
        /* Back up to the TS-packet aligned sequence header. From this point,
        bcmplayer sends all data. */
        uint64_t temp = BNAV_get_frameOffset(pEntry);
        temp = temp - BNAV_get_seqHdrStartOffset(pEntry);
        if (handle->packetSize) {
            handle->currentOffset = temp - temp % handle->packetSize;
        }
        else {
            handle->currentOffset = temp;
        }
        handle->lastSeqHdrOffset64 = handle->currentOffset;
    }
    else {
        handle->lastSeqHdrOffset64 = 0;
    }
    BNAV_Player_FlushFifo(handle);

    handle->skipCount = 0;
    handle->skipNextAdvance = 1;
    handle->iFrameRepeatCount = 0;
    handle->buildRef.inProcess = 0;
    handle->buildRef.needDisplayPicture = 0;
    handle->gopTrick.currentStart = -1;
    handle->gopTrick.pictureTag = 0;
    memset(&handle->timeSkip, 0, sizeof(handle->timeSkip));

    /* we can flush the pts cache here because there is new data coming */
    BNAV_Player_P_FlushPTSCache(handle);
    return 0;
}

/* resize buffer and clear cache. even if not resizing, always reset for consistent behavior. */
static void BNAV_Player_SetCacheSize(BNAV_Player_Handle handle, int cacheSize)
{
    int cacheBytes = handle->navFileIndexEntrySize * cacheSize;

    if (handle->navCacheSize == cacheSize && handle->navCacheBytes == cacheBytes)
        goto reset;

    handle->navCacheSize = cacheSize;
    handle->navCacheBytes = cacheBytes;
    handle->navCacheOverlap = cacheSize/4;

    /* realloc memory */
    if (handle->navCache)
        free(handle->navCache);
    handle->navCache = (unsigned char *)malloc(handle->navCacheBytes);

reset:
    handle->navCacheFails = 0;
    handle->navCacheMisses = 0;
    handle->navCacheReads = 0;
    handle->navCacheIndexStart = -1;
    handle->navCacheIndexEnd = -1;
}

int BNAV_Player_SetBounds(BNAV_Player_Handle handle, long firstIndex, long lastIndex)
{
    /* validate */
    if (firstIndex < 0 && firstIndex != -1) {
        return -1;
    }
    if (lastIndex < 0 && lastIndex != -1) {
        return -1;
    }
    if (firstIndex != -1 && lastIndex != -1 && firstIndex > lastIndex) {
        return -1;
    }

    /* apply if not -1 */
    if (firstIndex != -1) {
        handle->firstIndex = firstIndex;
    }
    if (lastIndex != -1) {
        handle->lastIndex = lastIndex;
    }

    /*TODO: should we check SetCurrentIndex and force it within bounds?*/
    return 0;
}

void BNAV_Player_GetSettings(BNAV_Player_Handle handle, BNAV_Player_Settings *settings)
{
    memset(settings, 0, sizeof(*settings));
    settings->cacheSize = handle->navCacheSize;
    settings->normalPlayBufferSize = handle->normalPlayBufferSize;
    settings->debugMode = handle->debugMode;
    memcpy(&settings->decoderFeatures, &handle->decoderFeatures, sizeof(handle->decoderFeatures));
    settings->videoPid = handle->pid;
    settings->enhancementVideoPid = handle->settings.enhancementVideoPid;
    settings->navVersion = handle->navVersion;
    settings->readCb = handle->readCb;
    settings->tellCb = handle->tellCb;
    settings->seekCb = handle->seekCb;
    settings->boundsCb = handle->boundsCb;
    settings->filePointer = handle->filePointer;
    settings->firstIndex = handle->firstIndex;
    settings->lastIndex = handle->lastIndex;
    /*settings->isPes = handle->isPes;*/
}

#if !B_REFSW_MINIMAL
void BNAV_Player_UpdateChangingBounds( BNAV_Player_Handle handle, bool changingBounds )
{
    handle->settings.changingBounds = changingBounds;
}

void BNAV_Player_SetDebugMode(BNAV_Player_Handle handle, BNAV_Player_DebugMode debugMode)
{
    handle->debugMode = debugMode;
}
#endif

int BNAV_Player_DefaultGetBounds(BNAV_Player_Handle handle, void *filePointer, long *firstIndex, long *lastIndex)
{
    long size;
    if ((*handle->seekCb)(filePointer, 0, SEEK_END))
        return -1;
    size = (*handle->tellCb)(filePointer);
    if (size == -1)
        return -1;

    *firstIndex = 0;
    *lastIndex = (size/handle->navFileIndexEntrySize) - 1;
    return 0;
}

#define TOTAL_SANITY_CHECKS 100
static int BNAV_Player_p_SanityCheck(BNAV_Player_Handle handle)
{
    int i;
    uint64_t last = 0;
    int version = -1;

    if(handle->navVersion == BNAV_Version_AVC_Extended) {
        return 0;
    }
    if (BNAV_Player_UpdateBounds(handle))
        return -1;
    for (i=0;i<TOTAL_SANITY_CHECKS;i++) {
        uint64_t next;
        BNAV_Entry *pEntry = BNAV_Player_ReadEntry(handle, i);
        if (!pEntry)
            return 0; /* it's ok to have nothing */

        /* offsets should always increase */
        next = BNAV_get_frameOffset(pEntry);
        if (next < last)
            return -1;
        last = next;

        /* version should be the same */
        if (version == -1)
            version = BNAV_get_version(pEntry);
        else if (version != BNAV_get_version(pEntry))
            return -1;
    }
    return 0;
}


void BNAV_Player_P_GetState(BNAV_Player_Handle handle, BNAV_Player_State *state)
{
    memset(state, 0, sizeof(*state));
    state->playMode = handle->playMode;
    state->playDir = handle->playDir;
    state->loopMode = handle->loopMode;
    state->advanceCount = handle->advanceCount;
    state->timestampOffset = handle->timestampOffset;
    state->packetSize = handle->packetSize;
    state->videoPid = handle->pid;
    state->enhancementVideoPid = handle->settings.enhancementVideoPid;
    state->normalPlayBufferSize = handle->normalPlayBufferSize;
    return;
}

int BNAV_Player_AddSimpleData(BNAV_Player_Handle handle, uint64_t offset, unsigned long size)
{
    BNAV_PktFifoEntry *curFifoEntry;

    /* add to the fifo */
    CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
    curFifoEntry->insertpackettype = eBpInsertNone;
    curFifoEntry->startByte = offset;
    curFifoEntry->endByte = offset + size - 1;
    return 0;
}

int BNAV_Player_AddStartCode(BNAV_Player_Handle handle, uint8_t startcode)
{
    BNAV_PktFifoEntry *curFifoEntry;
    CHECKREAD(curFifoEntry = BNAV_Player_AddFifoEntry(handle, handle->pid));
    curFifoEntry->insertpackettype = eBpInsertStartCode;
    curFifoEntry->startcode = startcode;
    return 0;
}

unsigned BNAV_GetEntrySize( BNAV_Version navVersion )
{
    switch (navVersion) {
    case BNAV_Version_VC1_PES:
    case BNAV_Version_AVC:
    case BNAV_Version_HEVC:
        return sizeof(BNAV_AVC_Entry);
    default:
        return sizeof(BNAV_Entry);
    }
}


long BNAV_Player_P_GetFirstIndex(BNAV_Player_Handle handle)
{
    if (BNAV_Player_UpdateBounds(handle)) {
        return -1;
    }
    return handle->firstIndex;
}

#if !B_REFSW_MINIMAL
void BNAV_Player_SetNormalPlayBufferSize(BNAV_Player_Handle handle, unsigned long normalPlayBufferSize)
{
    handle->normalPlayBufferSize = normalPlayBufferSize;
}
#endif
