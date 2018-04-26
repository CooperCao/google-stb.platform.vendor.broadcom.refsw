/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description: Converts startcode index to bcmplayer index
 *
 ***************************************************************************/
#ifndef USE_LEGACY_KERNAL_INTERFACE
/* Magnum debug interface */
#include "bstd.h"
#include "bkni.h"
#include "bioatom.h"
#include "biobits.h"
BDBG_MODULE(bcmindexer);
#else
#error "Not Supported"
/* Original debug interface */
#define BRCM_DBG_MODULE_NAME "bcmindexer"
#include "brcm_dbg.h"
#define BDBG_ERR(X) BRCM_DBG_ERR(X)
#define BDBG_WRN(X) BRCM_DBG_WRN(X)
#define BDBG_MSG(X) BRCM_DBG_MSG(X)
#define BDBG_DEBUG_BUILD 1
#endif

#if 0
/* easy way to turn on debug */
#include <stdio.h>
#undef BDBG_MSG
#define BDBG_MSG(X) do{printf X; printf("\n");}while(0)
#undef BDBG_WRN
#define BDBG_WRN(X) do{printf X; printf("\n");}while(0)
#endif

#define BDBG_MSG_TRACE(X) BDBG_MSG(X)

#include "bcmindexer.h"
#include "bcmindexerpriv.h"
#include "mpeg2types.h"
#include "avstypes.h"
#include "bvlc.h"
#include <string.h>
#include <stdlib.h>
#ifdef LINUX
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#elif defined __vxworks
#include <time.h>
#elif defined _WIN32_WCE
#include <Windows.h>
#include <mmsystem.h>
#elif defined _WIN32
#include <Windows.h>
typedef __int64 off_t;
#endif

/* 64 bit support */
#define getLo64(X) (unsigned long)((X)&0xFFFFFFFF)
#define getHi64(X) (unsigned long)((X)>>32)
#define create64(HI,LO) ((((uint64_t)(HI))<<32)|(unsigned long)(LO))
#define INVALID_OFFSET ((uint64_t)-1)

/* enable this to add start code validation
   debug messages at the compile time */
/* #define VALIDATE_SC_NAV */
#ifdef VALIDATE_SC_NAV
#define VALIDATE_MPEG_SC(handle,p_curBfr,sc) \
        if(((p_curBfr->startCodeBytes&0xff) > 0xbc)||!(sc==SC_ANY_NON_PTS||sc==SC_PTS||sc==SC_PES||sc==SC_SEQUENCE \
            ||sc==SC_PICTURE||sc==SC_FIRST_SLICE||sc==SC_EXTENSION||sc==SC_SEQ_END||sc==SC_GOP||sc==SC_PES_END \
            ||sc==PC_I_FRAME||sc==PC_P_FRAME||sc==PC_B_FRAME||sc==PC_ANY_FRAME))\
        {\
          BDBG_WRN(("Invalid SCT Entry: %02x pkt offset 0x%02lx%08lx,sc offset 0x%02x",sc, \
          handle->parsingErrors++; \
          (p_curBfr->recordByteCountHi>>24)&0xff,p_curBfr->recordByteCount,p_curBfr->startCodeBytes & 0xff));\
        }

#define VALIDATE_AVC_SC(handle,p_curSct4,sc)                   \
        if((p_curSct4->startCodeBytes&0xff)>0xbc)       \
        {                                               \
          BDBG_WRN(("Invalid SCT Entry: %02x pkt offset 0x%02lx%08lx,offset 0x%02x", sc, \
          handle->parsingErrors++; \
          (p_curSct4->recordByteCountHi>>24) & 0xFF, p_curSct4->recordByteCount,p_curSct4->startCodeBytes & 0xff));\
        }

#define VALIDATE_AVS_SC(handle,p_curBfr,sc) \
        if(((p_curBfr->startCodeBytes&0xff) > 0xbc)||!(sc==SC_AVS_ANY_NON_PTS||sc==SC_AVS_PTS||sc==SC_AVS_PES||sc==SC_AVS_SEQUENCE \
            ||sc==SC_AVS_PICTURE_I ||sc==SC_AVS_PICTURE_PB ||sc==SC_AVS_EXTENSION||sc==SC_AVS_SEQ_END||sc==SC_AVS_PES_END \
            ||sc==PC_AVS_P_FRAME||sc==PC_AVS_B_FRAME || sc==SC_AVS_FIRST_SLICE  ))\
        {\
          BDBG_WRN(("Invalid AVS SCT Entry: %02x pkt offset 0x%02lx%08lx,sc offset 0x%02x",sc, \
          handle->parsingErrors++; \
          (p_curBfr->recordByteCountHi>>24)&0xff,p_curBfr->recordByteCount,p_curBfr->startCodeBytes & 0xff));\
        }

#define VALIDATE_MPEG_NAV(handle, frameType)\
        if(!(frameType == eSCTypeIFrame || frameType == eSCTypePFrame ||\
           frameType == eSCTypeBFrame || eSCTypeBFrame == eSCTypeRPFrame))\
        {\
        }
#else

#define VALIDATE_MPEG_SC(handle, sc_offset,sc)
#define VALIDATE_AVC_SC(handle, sc_offset,sc)
#define VALIDATE_AVS_SC(handle, p_curBfr,sc)
#define VALIDATE_MPEG_NAV(handle, frameType)

#endif


/*---------------------------------------------------------------
- PRIVATE FUNCTIONS
---------------------------------------------------------------*/
static unsigned long BNAV_Indexer_returnPts(const BSCT_Entry *p_sct );
static int BNAV_Indexer_completeFrame( BNAV_Indexer_Handle handle );
static long BNAV_Indexer_FeedAVC(BNAV_Indexer_Handle handle, const void *p_bfr, long numEntries);
static long BNAV_Indexer_FeedHEVC(BNAV_Indexer_Handle handle, const void *p_bfr, long numEntries);
static long BNAV_Indexer_FeedAVS(BNAV_Indexer_Handle handle, void *p_bfr, long numEntries);
static long BNAV_Indexer_FeedMPEG2(BNAV_Indexer_Handle handle, void *p_bfr, long numEntries);
#if !B_REFSW_MINIMAL
static int BNAV_Indexer_Flush(BNAV_Indexer_Handle handle);
#endif
static int BNAV_Indexer_completeTimestampFrame(BNAV_Indexer_Handle handle);
static long BNAV_Indexer_P_FilterFullPacket(BNAV_Indexer_Handle handle, const BSCT_SixWord_Entry *sct, long numEntries, long (*feed)(BNAV_Indexer_Handle , const void *, long ));
static void BNAV_Indexer_P_ClearSpsPpsTable(BNAV_Indexer_Handle handle);
static void BNAV_Indexer_FeedHEVC_Init(BNAV_Indexer_Handle handle);

/**
* currentTimestamp returns the # of milliseconds from some fixed point in the past.
* I currently have a LINUX version. The generic version is only 1 second accurate,
* which might be sufficient if the requirement is to make +/- 30 second jumps.
**/
static unsigned int currentTimestamp(void) {
#if defined LINUX
    struct timespec now;
    if(clock_gettime(CLOCK_MONOTONIC, &now)!=0) {
        return 0;
    } else {
        return now.tv_sec * 1000 + now.tv_nsec / 1000000;
    }
#elif defined __vxworks
    return time(NULL) * 1000;
#elif defined _WIN32_WCE
    return GetTickCount();
#elif defined _WIN32
    return GetTickCount();
#endif
}

void BNAV_Indexer_GetDefaultSettings(BNAV_Indexer_Settings *settings)
{
    memset(settings, 0, sizeof(*settings));
    settings->simulatedFrameRate = 0;
    settings->sctVersion = BSCT_Version6wordEntry;
    settings->videoFormat = BNAV_Indexer_VideoFormat_MPEG2;
    settings->navVersion = BNAV_VersionLatest;
    settings->maxFrameSize = 5 * 1024 * 1024; /* We've seen 500K HD I frames. This is 10x larger. */
}

/******************************************************************************
* INPUTS:   cb = pointer to a function that stores table entries (same format as fwrite)
*           fp = general pointer that is passed in as param 3 into cb function
*           pid = pid of the transport stream used for table generation
* OUTPUTS:  None.
* RETURNS:  pointer to sBcmIndexer structure
* FUNCTION: This function allocates and initializes an indexer structure.
******************************************************************************/
int BNAV_Indexer_Open(BNAV_Indexer_Handle *handle, const BNAV_Indexer_Settings *settings)
{
    *handle = (BNAV_Indexer_Handle)malloc( sizeof( struct BNAV_Indexer_HandleImpl) );
    if (BNAV_Indexer_Reset(*handle, settings)) {
        free(*handle);
        return -1;
    }
    return 0;
}

int BNAV_Indexer_Reset(BNAV_Indexer_Handle handle, const BNAV_Indexer_Settings *settings)
{
    int result = 0;
    uint64_t offset;

    if (!settings->writeCallback ||
        !settings->filePointer)
    {
        BDBG_ERR(("Missing required settings"));
        return -1;
    }

    if (settings->videoFormat == BNAV_Indexer_VideoFormat_AVC &&
        settings->navVersion != BNAV_Version_AVC)
    {
        BDBG_ERR(("You need to select an AVC BNAV_Version for bcmindexer."));
        return -1;
    }
    else if(
            (settings->navVersion == BNAV_Version_AVC_Extended &&
             !(settings->videoFormat == BNAV_Indexer_VideoFormat_AVC_SVC || settings->videoFormat == BNAV_Indexer_VideoFormat_AVC_MVC))
         || ((settings->navVersion != BNAV_Version_AVC_Extended) &&
              (settings->videoFormat == BNAV_Indexer_VideoFormat_AVC_SVC || settings->videoFormat == BNAV_Indexer_VideoFormat_AVC_MVC))
        ) {
        BDBG_ERR(("You need to select an AVC_Extended BNAV_Version for bcmindexer."));
        return -1;
    }
    else if ( (settings->videoFormat != BNAV_Indexer_VideoFormat_AVC && settings->videoFormat != BNAV_Indexer_VideoFormat_HEVC) &&
        settings->navVersion == BNAV_Version_AVC)
    {
        BDBG_ERR(("You need to select an MPEG2 BNAV_Version for bcmindexer."));
        return -1;
    }
    else if (settings->videoFormat != BNAV_Indexer_VideoFormat_AVS &&
        settings->navVersion == BNAV_Version_AVS)
    {
        BDBG_ERR(("You need to select an MPEG2 BNAV_Version for bcmindexer."));
        return -1;
    }

    else if (settings->videoFormat != BNAV_Indexer_VideoFormat_HEVC &&
        settings->navVersion == BNAV_Version_HEVC)
    {
        BDBG_ERR(("You need to select an HEVC BNAV_Version for bcmindexer."));
        return -1;
    }
    else if (settings->navVersion >= BNAV_VersionUnknown) {
        BDBG_ERR(("Unsupported BNAV_Version"));
        return -1;
    }
    if (settings->sctVersion != BSCT_Version6wordEntry) {
        BDBG_ERR(("only BSCT_Version6wordEntry supported"));
        return -1;
    }

    memset(handle, 0, sizeof(*handle));
    BNAV_Indexer_FeedSVC_Init(handle);
    BNAV_Indexer_FeedHEVC_Init(handle);

    handle->settings = *settings;

    if (!settings->simulatedFrameRate && !settings->ptsBasedFrameRate) {
        /* real, not simulated */
        handle->lasttime = handle->starttime = currentTimestamp();
    }

    BNAV_set_frameType(&(handle->curEntry), eSCTypeUnknown);
    BNAV_set_frameType(&(handle->avcEntry), eSCTypeUnknown);

    offset = create64(handle->settings.append.offsetHi, handle->settings.append.offsetLo);
    BNAV_set_frameOffset(&(handle->curEntry), offset);
    BNAV_set_frameOffset(&(handle->avcEntry), offset);

    handle->isHits = 1;  /* Assume HITS until we're proven wrong */
    handle->avc.current_sps = handle->avc.current_pps = -1;
    BNAV_Indexer_P_ClearSpsPpsTable(handle);

    return result;
}

/******************************************************************************
* INPUTS:   handle = pointer to a previously allocated sBcmIndexer structure
* OUTPUTS:  None.
* RETURNS:  None.
* FUNCTION: This function frees any memory used by an indexer structure.
******************************************************************************/
void BNAV_Indexer_Close(BNAV_Indexer_Handle handle)
{
    free((void *)handle);
}

long BNAV_Indexer_Feed(BNAV_Indexer_Handle handle, void *p_bfr, long numEntries)
{
    switch(handle->settings.videoFormat) {
    case BNAV_Indexer_VideoFormat_AVC_MVC:
        return BNAV_Indexer_FeedAVC_MVC(handle, p_bfr, numEntries);
    case BNAV_Indexer_VideoFormat_AVC_SVC:
        return BNAV_Indexer_FeedAVC_SVC(handle, p_bfr, numEntries);
    case BNAV_Indexer_VideoFormat_AVC:
        return BNAV_Indexer_P_FilterFullPacket(handle, p_bfr, numEntries, BNAV_Indexer_FeedAVC);
    case BNAV_Indexer_VideoFormat_HEVC:
        return BNAV_Indexer_P_FilterFullPacket(handle, p_bfr, numEntries, BNAV_Indexer_FeedHEVC);
    case BNAV_Indexer_VideoFormat_AVS:
        return BNAV_Indexer_FeedAVS(handle, p_bfr, numEntries);
    case BNAV_Indexer_VideoFormat_MPEG2:
        return BNAV_Indexer_FeedMPEG2(handle, p_bfr, numEntries);
    default:
        BDBG_ERR(("not supported videoFormat: %u", handle->settings.videoFormat));
        return -1;
    }
}

/* Filter SCT by TF_ENTRY_TYPE (word0 31:24). Return non-zero if SCT can be ignored by indexer.
RAI from 0x01 TPIT is consumed here, but the SCT is discarded for indexing.
All 0x80 TS Packet SCT's should have been handled before this function. So only 0x00 SCT is processed.

Other known packets which are discarded include 0x10 and 0x11 (record sct for 16 byte extraction),
0x02 (seamless pause), 0x06 (scrambling control), 0x81 (transponder bonding), 0x82 (record btp),
0x83 (tsio), 0x84 (general channel bonding). They should not occur under normal circumstances,
so a BDBG_WRN is issued.
*/
static int bcmindexer_p_filter_sct(BNAV_Indexer_Handle handle, const BSCT_SixWord_Entry *p_curBfr)
{
    unsigned type = p_curBfr->word0>>24;
    switch (type) {
    case 0x01:
        /* Check for TPIT Entry to extract RAI. discard all other TPIT entries. */
        if ((p_curBfr->startCodeBytes & 0x20)>>5)
        {
            handle->random_access_indicator = true;
        }
        return -1;
    case 0x00:
        /* SCT */
        return 0;
    default:
        /* discard all others */
        BDBG_WRN(("discarding SCT %02x", type));
        return -1;
    }
}

static int BNAV_Indexer_ProcessMPEG2SCT(BNAV_Indexer_Handle handle, const BSCT_Entry *p_curBfr)
{
    BNAV_Entry *navEntry = &handle->curEntry;
    uint64_t offset = 0;
    int sc = returnStartCode(p_curBfr->startCodeBytes); /* parse once */

    /* detect invalid start code and offsets */
    VALIDATE_MPEG_SC(handle, p_curBfr,sc);

    BDBG_MSG_TRACE(("SCT Entry: %02x %08x %08x %08x %08x", sc, p_curBfr->startCodeBytes, p_curBfr->recordByteCount, p_curBfr->recordByteCountHi, p_curBfr->flags));

    if (sc != SC_PTS) {
        offset = BNAV_Indexer_getOffset(handle, p_curBfr);
        if (offset == INVALID_OFFSET) return 0;
    }

    if (handle->settings.navVersion == BNAV_Version_TimestampOnly) {
        switch (sc) {
        /* BNAV_Version_TimestampOnly does not require PTS, but it will index it if found */
        case SC_PTS:
            handle->next_pts = BNAV_Indexer_returnPts(p_curBfr);
            break;
        default:
            /* BNAV_Version_TimestampOnly requires at least one non-PTS startcode so it can get
            the file offset. Without the file offset, the timestamp has no use. */
            {
                unsigned currenttime = currentTimestamp();
                unsigned framesize = offset - handle->picStart;

                /* don't write out NAV's more than 1 per 30 msec or that are too small. */
                if (currenttime >= handle->lasttime + 30 && framesize > 1000) {
                    BNAV_set_frameSize(navEntry, framesize);
                    BNAV_Indexer_completeTimestampFrame( handle );

                    /* record the start of the next frame */
                    BNAV_set_framePts(navEntry, handle->next_pts);
                    BNAV_set_frameOffset(navEntry, offset);
                    handle->picStart = offset;
                }
            }
            break;
        }
        return 0;
    }

    if (handle->seqHdrFlag) {
        if (sc != SC_PES && sc != SC_PTS && sc != SC_EXTENSION) {
            handle->seqHdrFlag = 0;
            handle->seqHdrSize = offset - handle->seqHdrStartOffset;
        }
    }

    switch(sc)
    {
    case SC_FIRST_SLICE:
        /* I-slice check here */
        if (returnIntraSliceBit(p_curBfr->startCodeBytes) && handle->isHits) {
            BNAV_set_frameType(navEntry, eSCTypeRPFrame);
            handle->isISlice = 1;
        }
        break;

    case SC_PES:
        break;

    case SC_PTS:
        handle->next_pts = BNAV_Indexer_returnPts(p_curBfr);
        break;

    case SC_SEQUENCE:
        handle->seqHdrStartOffset = offset;
        handle->seqHdrFlag = 1; /* set on next qualifying sct */

        /* complete any pending frame */
        handle->picEnd = handle->seqHdrStartOffset;
        handle->fieldEncoded.count = 0;
        BNAV_Indexer_completeFrame( handle );
        break;

    case SC_SEQ_END:   /* TODO: Change me to any non-slice */
        /* complete any pending frame */
        handle->picEnd = offset;
        handle->fieldEncoded.count = 0;
        BNAV_Indexer_completeFrame( handle );
        break;

    case SC_EXTENSION:
        {
        unsigned char payload0;
        payload0 = (p_curBfr->startCodeBytes >> 16) & 0xFF;
        if ((payload0 & 0xF0) == 0x80) { /* extension_start_code_identifier == picture coding extension ID */
            unsigned char field = (p_curBfr->recordByteCountHi >> 16) & 0x03;

            if (field == 0x1 || field == 0x2) {
                handle->fieldEncoded.count++; /* field */
                BDBG_MSG(("field encoded %#x, %d", field, handle->fieldEncoded.count));
            }
            else {
                handle->fieldEncoded.count = 0; /* frame */
            }

            if (handle->fieldEncoded.count == 2 && field == handle->fieldEncoded.lastField && handle->fieldEncoded.skippedPicture) {
                int rc;
                BDBG_MSG(("repeat field means previous skipped SC_PICTURE should have been start of new picture"));
                handle->fieldEncoded.count = 0;
                BNAV_set_frameType(navEntry, eSCTypeUnknown); /* just skip the dangling field */
                rc = BNAV_Indexer_ProcessMPEG2SCT(handle, &handle->fieldEncoded.skippedPictureSct);
                if (rc) return BERR_TRACE(rc);
                handle->fieldEncoded.count = 1;
            }

            handle->fieldEncoded.lastField = field;
        }
        }

        break;

    case SC_PICTURE:
        {
        unsigned picture_coding_type;
        unsigned temporal_reference;
        bool skip_complete = false;

        temporal_reference = (p_curBfr->startCodeBytes >> 14) & 0x3FF;
        picture_coding_type = returnPictureCode(p_curBfr->startCodeBytes);

        if (handle->fieldEncoded.count == 1) {
            /* this picture could be a field pair. evaluate what we can now. evaluate the rest when the next 0xB5 arrives.
            Dangling field means temporal_reference changed, or because it's not a B/B or I/I,I/P,P/P pair.
            see SW7425-2365 for an overview of the decision logic. */
            if (temporal_reference == handle->fieldEncoded.temporal_reference &&
                ((picture_coding_type == PC_B_FRAME) == (handle->fieldEncoded.picture_coding_type == PC_B_FRAME))) {
                /* we've only processed one field of a field encoded picture. skip this one,
                but save the information in case it turns out to be a new picture. */
                handle->fieldEncoded.skippedPicture = true;
                handle->fieldEncoded.skippedPictureSct = *p_curBfr;
                break;
            }
            /* skip the previous field as dangling, but continue indexing this picture. */
            BDBG_MSG(("discard dangling field: %d=%d, %d=%d", temporal_reference, handle->fieldEncoded.temporal_reference, picture_coding_type, handle->fieldEncoded.picture_coding_type));
            handle->fieldEncoded.count = 0;
            skip_complete = true;
        }
        handle->fieldEncoded.skippedPicture = false;
        handle->fieldEncoded.temporal_reference = temporal_reference;
        handle->fieldEncoded.picture_coding_type = picture_coding_type;

        if (!skip_complete) {
            /* complete any pending frame */
            handle->picEnd = offset;
            handle->fieldEncoded.count = 0;
            handle->fieldEncoded.lastField = 0;
            BNAV_Indexer_completeFrame( handle );
        }

        /* start a new frame */
        handle->picStart = offset;

        BNAV_set_frameOffset(navEntry, offset);

        switch(picture_coding_type)
        {
        case PC_I_FRAME:
            handle->isISlice = 1;
            handle->isHits = 0;
            BNAV_set_frameType(navEntry, eSCTypeIFrame);
            if (handle->prev_pc == PC_I_FRAME) {
                handle->prev_I_rfo = 0;
            }
            break;
        case PC_P_FRAME:
            handle->prev_I_rfo = 0;
            /* First P after first I allows open GOP B's to be saved */
            if (handle->hitFirstISlice) {
                handle->allowOpenGopBs = 1;
            }
            BNAV_set_frameType(navEntry, eSCTypePFrame);
            break;
        case PC_B_FRAME:
            BNAV_set_frameType(navEntry, eSCTypeBFrame);
            break;
        default:
            handle->parsingErrors++;
            BDBG_ERR(("unknown picture_coding_type 0x%x", picture_coding_type));
            break;
        }
        handle->prev_pc = picture_coding_type;

        /* make sequence header offset relative to current frame rather than */
        /* relative to reference frame to allow removal of b-frames */
        BNAV_set_seqHdrSize(navEntry, (unsigned short)handle->seqHdrSize);
        BNAV_set_seqHdrStartOffset(navEntry, handle->picStart - handle->seqHdrStartOffset);

        /* Sets the refFrameOffset after adding the prev_I_rfo to the rfo.
        prev_I_rfo will be non-zero ONLY for open gop B's, which are B's that come
        after an I but before a P. */
        BNAV_set_refFrameOffset(navEntry, handle->rfo + handle->prev_I_rfo);

        if (handle->settings.navVersion >= BNAV_VersionOpenGopBs) {
            if (picture_coding_type == PC_I_FRAME) {
                handle->prev_I_rfo = handle->rfo;
            }
        }

        if (handle->hitFirstISlice) {
            handle->rfo++;
        }

        BNAV_set_framePts(navEntry, handle->next_pts);
        }
        break;
    default:
        break;
    }
    return 0;
}

static long BNAV_Indexer_FeedMPEG2(BNAV_Indexer_Handle handle, void *p_bfr, long numEntries)
{
    long i;
    const BSCT_SixWord_Entry *p_cur6WordBfr = p_bfr;
    for(i=0; i<numEntries; ++i, ++p_cur6WordBfr) {
        if (bcmindexer_p_filter_sct(handle, p_cur6WordBfr)) {
            continue;
        }
        (void)BNAV_Indexer_ProcessMPEG2SCT(handle, CONVERT_TO_SCT4(p_cur6WordBfr));
    }
    return i;
}

#if !B_REFSW_MINIMAL
long BNAV_Indexer_FeedReverse(BNAV_Indexer_Handle handle, const BSCT_Entry *p_bfr, long numEntries)
{
    int i,j;

    for(i=numEntries-1; i>=0; --i)
    {
        const BSCT_Entry *curSct = &p_bfr[i];
        BNAV_Entry *navEntry;

        int sc = returnStartCode(curSct->startCodeBytes); /* parse once */

        BDBG_MSG(("SCT Entry: %08x %08x %08x %08x", curSct->startCodeBytes, curSct->recordByteCount, curSct->recordByteCountHi, curSct->flags));

        /* Set navEntry to the current BNAV_Entry */
        if (handle->reverse.total_entries)
            navEntry = &handle->reverse.entry[handle->reverse.total_entries-1];
        else
            navEntry = NULL;

        switch(sc)
        {
#if 0
/* No HITS support for OTF PVR. Is this a requirement? */
        case SC_FIRST_SLICE:
            /* I-slice check here */
            if (returnIntraSliceBit(curSct->startCodeBytes) && handle->isHits) {
                BNAV_set_frameType(navEntry, eSCTypeRPFrame);
                handle->isISlice = 1;
            }
            break;
        case SC_PES:
        case SC_SEQ_END:
            break;
#endif
        case SC_PTS:
            {
            unsigned long pts = BNAV_Indexer_returnPts(curSct);

            /* Set PTS for all entries that don't have one. */
            for (j=handle->reverse.total_entries-1;j>=0;j--) {
                BNAV_Entry *entry = &handle->reverse.entry[j];

                if (BNAV_get_framePts(entry))
                    break;
                BNAV_set_framePts(entry, pts);
            }
            }
            break;

        case SC_SEQUENCE:
            handle->seqHdrStartOffset = BNAV_Indexer_getOffset(handle, curSct);
            if (handle->seqHdrStartOffset == INVALID_OFFSET) break;
            handle->seqHdrSize = handle->picEnd - handle->seqHdrStartOffset;

            /* Go through every entry and set seqhdr offset if not set already */
            for (j=handle->reverse.total_entries-1;j>=0;j--) {
                BNAV_Entry *entry = &handle->reverse.entry[j];

                if (BNAV_get_seqHdrStartOffset(entry))
                    break;

                BNAV_set_seqHdrSize(entry, (unsigned short)handle->seqHdrSize);
                BNAV_set_seqHdrStartOffset(entry, BNAV_get_frameOffset(entry) - handle->seqHdrStartOffset);
            }

            BNAV_Indexer_Flush(handle);
            break;

        case SC_PICTURE:
            {
            unsigned frameSize, picture_coding_type;
            if (!handle->picEnd) {
                /* If we don't know where this frame ends, we have to skip it. */
                break;
            }

            /* Select a new BNAV_Entry */
            if (handle->reverse.total_entries == MAX_GOP_SIZE) {
                BDBG_ERR(("MAX_GOP_SIZE exceeded. Bad data resulting."));
                handle->parsingErrors++;
                return -1;
            }
            navEntry = &handle->reverse.entry[handle->reverse.total_entries++];

            handle->picStart = BNAV_Indexer_getOffset(handle, curSct);
            if (handle->picStart == INVALID_OFFSET) break;
            frameSize = handle->picEnd - handle->picStart;
            if (frameSize > handle->settings.maxFrameSize) {
                BDBG_WRN(("Giant frame (%d bytes) detected and rejected: %d", frameSize, handle->totalEntriesWritten));
                handle->parsingErrors++;
                /* Throw away this entry */
                handle->reverse.total_entries--;
                continue;
            }

            BNAV_set_frameOffset(navEntry, handle->picStart);
            BNAV_set_frameSize(navEntry, frameSize);
            BNAV_Indexer_StampEntry(handle, navEntry);

            /* Set this to 0 so that we'll set it when we hit a seqhdr */
            BNAV_set_seqHdrStartOffset(navEntry, 0);
            BNAV_set_refFrameOffset(navEntry, 0);

            picture_coding_type = returnPictureCode(curSct->startCodeBytes);

            switch (picture_coding_type)
            {
            case PC_I_FRAME:
                BNAV_set_frameType(navEntry, eSCTypeIFrame);
                break;
            case PC_P_FRAME:
                BNAV_set_frameType(navEntry, eSCTypePFrame);
                break;
            case PC_B_FRAME:
                BNAV_set_frameType(navEntry, eSCTypeBFrame);
                break;
            default:
                BDBG_ERR(("unknown picture_coding_type 0x%x", picture_coding_type));
                handle->parsingErrors++;
                break;
            }


            /* When we hit an I frame, we can set the refoffset for the GOP */
            if (picture_coding_type == PC_I_FRAME) {
                int refoff = 0; /* Offset from I frame */
                int newgop = 1; /* Set at every I frame */
                int foundFirstP = 0; /* This is the marker for open GOP B's */
                for (j=handle->reverse.total_entries-1;j>=0;j--) {
                    BNAV_Entry *entry = &handle->reverse.entry[j];
                    int done = 0;

                    switch (BNAV_get_frameType(entry)) {
                    case eSCTypeBFrame:
                        /* If this is an open GOP B for the previous frame, skip it. */
                        if (newgop && !foundFirstP) {
                            refoff++;
                            continue;
                        }
                        break;
                    case eSCTypePFrame:
                        /* If we have a new GOP but we've already found a P, then we're
                        done setting the open GOP B's for the current reference frame.
                        We're done. */
                        /* coverity[dead_error_condition] */
                        if (newgop && foundFirstP) {
                            /* coverity[dead_error_begin] */
                            done = 1;
                            break;
                        }

                        /* Once we find a P, we're into the middle of the GOP so don't
                        skip open GOP B's. */
                        newgop = 0;
                        foundFirstP = 1;
                        break;
                    case eSCTypeIFrame:
                        newgop = 1;
                        break;
                    default:
                        BDBG_ERR(("Only GOP-based streams supported."));
                        handle->parsingErrors++;
                        return -1;
                    }

                    /* coverity[dead_error_condition] */
                    if (done)
                        /* coverity[dead_error_line] */
                        break;

                    BNAV_set_refFrameOffset(entry, refoff);
                    refoff++;
                }

                /* And now we can flush */
                BNAV_Indexer_Flush(handle);
            }
            }
            break;

        default:
            break;
        }

        if (sc != SC_PES && sc != SC_PTS && sc != SC_EXTENSION && sc != SC_FIRST_SLICE)
        {
            handle->picEnd = BNAV_Indexer_getOffset(handle, curSct);
            BDBG_MSG(("Set picEnd " BDBG_UINT64_FMT ", %d", BDBG_UINT64_ARG(handle->picEnd),sc));
        }
    }

    return numEntries;
}
#endif

#define BNAV_LARGE_TIME_GAP 3000 /* 3000 msec = 3 seconds */

/**
Set timestamp and vchip stamps for an entry based on current bcmindexer and system state.
**/
void BNAV_Indexer_StampEntry(BNAV_Indexer_Handle handle, BNAV_Entry *entry)
{
    unsigned new_timestamp;
    /* Note: simple simulatedFrameRate timestamp is inaccurate for mixed 3:2/2:2 content; the offline indexer should use ptsBased instead! */
    if (handle->settings.simulatedFrameRate) {
        /* calc a timestamp based on a simulated frame rate. actual system time is irrevelevant. NOTE: avoid accumulative error!! */
        new_timestamp = (handle->starttime++ * 1000)/handle->settings.simulatedFrameRate + handle->settings.append.timestamp;
    }
    else if (handle->settings.ptsBasedFrameRate) {
        int frameType = BNAV_get_frameType(entry);
        unsigned timestamp;

        /* 
        this algo converts from decode-order PTS to guaranteed-increasing timestamp. 
        it identifies PTS discontinuities. continuous PTS's are converted to timestamps.
        any pictures without new PTS's have interpolated timestamps.
        all MPEG B frames and some AVC B's and P's may have different display order; allow 1 second of reverse threshold for them. */
        if (handle->next_pts < handle->ptsBasedFrameRate.lastPts - 1*45000 ||
            handle->next_pts > handle->ptsBasedFrameRate.lastPts + 10*45000)
        {
            /* get across the pts discontinuity. don't change the rate and don't accumulate frameCount. just advance the lastPts.
            allow for infrequently coded PTS's, but bound it to 10 seconds. */
            handle->ptsBasedFrameRate.startPts = handle->next_pts;
            if (handle->lasttime) {
                if (!handle->ptsBasedFrameRate.rate) {
                    handle->ptsBasedFrameRate.rate = 30; /* initial rate. rate must always be non-zero to ensure timestamps are always increasing. */
                }
                timestamp = handle->lasttime + handle->ptsBasedFrameRate.rate;
            }
            else {
                timestamp = 0;
            }
            handle->ptsBasedFrameRate.startTimestamp = timestamp;
            handle->ptsBasedFrameRate.frameCount = 0;
            handle->ptsBasedFrameRate.lastPts = handle->next_pts;
            
            /* leave handle->ptsBasedFrameRate.rate alone */
        }
        else if (frameType != eSCTypeBFrame && handle->next_pts > handle->ptsBasedFrameRate.lastPts) {
            /* set the timestamp based on the pts difference. track it exactly. */
            unsigned span = (handle->next_pts - handle->ptsBasedFrameRate.startPts) / 45;
            
            timestamp = handle->ptsBasedFrameRate.startTimestamp + span;
            if (timestamp <= handle->lasttime) { 
                timestamp = handle->lasttime + 1;
            }
            handle->ptsBasedFrameRate.frameCount++;
            
            /* set rate */
            BDBG_ASSERT(handle->ptsBasedFrameRate.frameCount);
            handle->ptsBasedFrameRate.rate = (timestamp - handle->ptsBasedFrameRate.startTimestamp) / handle->ptsBasedFrameRate.frameCount;
            handle->ptsBasedFrameRate.lastPts = handle->next_pts;
        }
        else {
            unsigned span;
            /* interpolate the PTS based on rate since startPts */
            handle->ptsBasedFrameRate.frameCount++;
            span = handle->ptsBasedFrameRate.frameCount * handle->ptsBasedFrameRate.rate;
            
            timestamp = handle->ptsBasedFrameRate.startTimestamp + span;
            if (timestamp <= handle->lasttime) { 
                timestamp = handle->lasttime + 1;
            }
        }
        
        BDBG_ASSERT((!timestamp && !handle->lasttime) || timestamp > handle->lasttime); /* must be increasing */
        new_timestamp = timestamp + handle->settings.append.timestamp;
        handle->lasttime = timestamp;
    }
    else {
        unsigned currenttime = currentTimestamp();

        /* provide a minimal safe increment so that timestamps can be unique and guaranteed increasing. */
        if (currenttime <= (unsigned)handle->lasttime) {
            currenttime = handle->lasttime + 15; /* 15 msec increment is less than 60 fps (16.6 msec), so we shouldn't get ahead of real timestamps */
        }

        /* Any large jump in timestamps can only be caused by the source disappearing because of loss of signal, etc.
        The timestamps should skip over this gap. */
        if (!handle->settings.allowLargeTimeGaps && currenttime - handle->lasttime > BNAV_LARGE_TIME_GAP) {
            handle->starttime += (currenttime - handle->lasttime) - 30; /* turn any large gap into a 30 millisecond gap by adjusting the starttime */
        }
        new_timestamp = currenttime - handle->starttime + handle->settings.append.timestamp;
        handle->lasttime = currenttime;
    }

#define BNAV_TIMESTAMP_OVERFLOW_WARNING (0xF0000000)
    if (new_timestamp >= BNAV_TIMESTAMP_OVERFLOW_WARNING) {
        /* Timestamp is 32 bits in units of miliseconds, so it will overflow in about 49 days (2^32/1000/3600/24 ~ 49 days).
        Switching to 64 bit could not be abstracted from applications, so we require the app to stop the
        recording before overflow.

        If timeshifting, the app can start a second, parallel timeshift buffer then switch over to the second buffer when it
        is ready. Ideally this can be done when no HDMI output is connected, or in the middle of the night, etc.
        But if the app gets close enough to overflow, it must simply be done.

        If linear recording, which is very unlikely, it must simply be stopped. Another recording can be started and the app must
        splice them together during playback.

        Print a warning every 60 seconds after hitting the threshold. */
        if ((new_timestamp - BNAV_TIMESTAMP_OVERFLOW_WARNING) / 60000 > (handle->actual_lasttime - BNAV_TIMESTAMP_OVERFLOW_WARNING) / 60000) {
            BDBG_ERR(("%p about to overflow timestamp %#x. App must stop this recording and start another.", (void *)handle, new_timestamp));
        }
    }
    handle->actual_lasttime = new_timestamp;
    BNAV_set_timestamp(entry, new_timestamp);

    BNAV_set_packed_vchip(entry, handle->vchip);
    BNAV_set_version(entry, handle->settings.navVersion);
}

#if !B_REFSW_MINIMAL
/* For FeedReverse only. Write out BNAV_Entry's which have a seqhdr and refoffset. */
static int BNAV_Indexer_Flush(BNAV_Indexer_Handle handle)
{
    int i;
    BDBG_MSG(("flush total %d", handle->reverse.total_entries));
    for (i=0;i<handle->reverse.total_entries; i++) {
        BNAV_Entry *entry = &handle->reverse.entry[i];

        BDBG_MSG(("  %ld, %d, %d",
            BNAV_get_seqHdrStartOffset(entry),
            BNAV_get_refFrameOffset(entry),
            BNAV_get_frameType(entry)
            ));

        if (BNAV_get_seqHdrStartOffset(entry) && BNAV_get_refFrameOffset(entry))
        {
           /* write using callback */
            int numBytes = (*handle->settings.writeCallback)(entry, 1,
                sizeof( BNAV_Entry ), handle->settings.filePointer);
            if (numBytes != sizeof( BNAV_Entry )) {
                BDBG_ERR(("Unable to write index entry."));
                return -1;
            }
            handle->totalEntriesWritten++;
            handle->lastEntryWritten = *entry;
        }
        else {
            /* When we hit the first entry we can't send, break out */
            break;
        }
    }

    /* Shift forward the entry array based on how many were sent */
    handle->reverse.total_entries -= i;
    memmove(handle->reverse.entry, &handle->reverse.entry[i],
        handle->reverse.total_entries * sizeof(BNAV_Entry));

    return 0;
}
#endif

static int BNAV_P_CheckEntry(BNAV_Indexer_Handle handle, BNAV_Entry *navEntry)
{
    unsigned long frameSize = BNAV_get_frameSize(navEntry);
    int frameType = BNAV_get_frameType(navEntry);

    if (frameSize > handle->settings.maxFrameSize) {
        BDBG_WRN(("Giant frame (%ld bytes) detected and rejected: %d", frameSize, handle->totalEntriesWritten));
        handle->parsingErrors++;
        /* discard everything until next I frame */
        handle->hitFirstISlice = 0;
        handle->rfo = 0;
        handle->isISlice = 0;
        handle->prev_I_rfo = 0;
        handle->prev_pc = 0;
        handle->allowOpenGopBs = 0; /* including open GOP B's after that first I */
        goto fail;
    }
    else if (frameSize == 0) {
        BDBG_ERR(("Invalid frame size 0 rejected, probably corrupt index at %d", handle->totalEntriesWritten));
        handle->parsingErrors++;
        goto fail;
    }
    else if (BNAV_get_seqHdrSize(navEntry) == 0) {
        BDBG_MSG(("Discarding picture with no sequence header"));
        goto fail;
    }

    /* Skip dangling open GOP B's */
    if (frameType == eSCTypeBFrame && handle->hitFirstISlice && !handle->allowOpenGopBs &&
        handle->settings.navVersion != BNAV_Version_AVC &&
        handle->settings.navVersion != BNAV_Version_HEVC &&
        handle->settings.navVersion != BNAV_Version_VC1_PES &&
        handle->settings.navVersion != BNAV_Version_AVS)
    {
        BDBG_MSG(("Discarding dangling open GOP B: %d", handle->totalEntriesWritten));
        handle->rfo--;
        goto fail;
    }

    /* success */
    return 0;

fail:
    BNAV_set_frameType(navEntry, eSCTypeUnknown);
    return -1;
}


/* Write a MPEG2 or AVC entry back to the write callback. */
int BNAV_Indexer_completeFrameAux(BNAV_Indexer_Handle handle, void *data, int size)
{
    BNAV_Entry *navEntry = (BNAV_Entry *)data;
    unsigned long frameSize = handle->picEnd - BNAV_get_frameOffset(navEntry);
    int frameType = BNAV_get_frameType(navEntry);

    BNAV_set_frameSize(navEntry,frameSize);

    /* perform sanity check before writing */
    if (BNAV_P_CheckEntry(handle, navEntry))
    {
        if (handle->hiOverflow.review) {
            handle->hiOverflow.cnt--;
            handle->hiOverflow.review = false;
        }
        return -1;
    }
    handle->hiOverflow.review = false;

    if (handle->isISlice)
        handle->hitFirstISlice = 1;

    if (frameType != eSCTypeUnknown && handle->hitFirstISlice)
    {
        int numBytes;
        BNAV_Indexer_StampEntry(handle, navEntry);

        BDBG_MSG(("Output: %08x %08x %08x %08x %08x %08x (%s)", navEntry->words[0], navEntry->words[1],
            navEntry->words[2], navEntry->words[3], navEntry->words[4], navEntry->words[5],
            BNAV_frameTypeStr[frameType]));

        /* write using callback */
        numBytes = (*handle->settings.writeCallback)(navEntry, 1, size, handle->settings.filePointer);
        if (numBytes != size) {
            BDBG_ERR(("Unable to write index entry."));
            return -1;
        }
        handle->totalEntriesWritten++;
        handle->lastEntryWritten = *navEntry;
    }
    /**
    * Clear the frametype, but don't clear the rest of the information. If
    * in the future the rest of the information must be cleared, watchout so that
    * BNAV_Indexer_GetPosition isn't broken.
    **/
    BNAV_set_frameType(navEntry, eSCTypeUnknown);

    if(handle->isISlice)
    {
        handle->rfo = 1;
        handle->isISlice = 0;
    }
    handle->picEnd = 0;

    return 0;
}

/* Write completed MPEG2 entry to write callback. */
static int BNAV_Indexer_completeFrame(BNAV_Indexer_Handle handle)
{
    return BNAV_Indexer_completeFrameAux(handle, &handle->curEntry, sizeof(handle->curEntry));
}

/* Write completed AVC entry to write callback. */
static int BNAV_Indexer_completeAVCFrame(BNAV_Indexer_Handle handle)
{
    BNAV_AVC_Entry *avcEntry = &handle->avcEntry;

    /* We're going to change the definition of P and B frame for AVC. For MPEG2, a B picture has two
    properties: it uses bidirectional prediction (forward and backward in display order, always backward in decode order)
    and it is not predicted against. The player only uses the second property. It can drop a B frame.
    For AVC, a B picture is has the first property but not the second. At this point, it's unclear if the player
    can use the B directional property for any trick mode (especially if not stored at a slice level).
    Therefore, for the NAV table AVC indexes, the definition of "B frame" will be "non-I frame which is not
    a referenced picture." This means B frames will have the second property - they are discardable. */
    if (BNAV_get_frameType(avcEntry) == eSCTypeBFrame) {
        if (handle->avc.is_reference_picture) {
            BDBG_MSG(("We have a reference B frame"));
            BNAV_set_frameType(avcEntry, eSCTypePFrame);
        }
    }
    else if (BNAV_get_frameType(avcEntry) == eSCTypePFrame) {
        if (!handle->avc.is_reference_picture) {
            BDBG_MSG(("We have a non-reference P frame"));
            BNAV_set_frameType(avcEntry, eSCTypeBFrame);
        }
    }

    return BNAV_Indexer_completeFrameAux(handle, &handle->avcEntry, sizeof(handle->avcEntry));
}

static int BNAV_Indexer_completeTimestampFrame(BNAV_Indexer_Handle handle)
{
    BNAV_Entry *navEntry = &handle->curEntry;
    unsigned size = sizeof(handle->curEntry);
    int numBytes;

    BNAV_Indexer_StampEntry(handle, navEntry);

    BDBG_MSG(("Output: %08x %08x %08x %08x %08x %08x", navEntry->words[0], navEntry->words[1],
        navEntry->words[2], navEntry->words[3], navEntry->words[4], navEntry->words[5]));

    /* write using callback */
    numBytes = (*handle->settings.writeCallback)(navEntry, 1, size, handle->settings.filePointer);
    if (numBytes != (int)size) {
        BDBG_ERR(("Unable to write index entry."));
        return -1;
    }
    handle->totalEntriesWritten++;
    handle->lastEntryWritten = *navEntry;

    return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:   p_sct = pointer to sct entry
+ OUTPUTS:  None.
+ RETURNS:  pts value (bits [32:1]
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
unsigned long BNAV_Indexer_returnPts(const BSCT_Entry *p_sct )
{
    return p_sct->recordByteCountHi;
}

/******************************************************************************
Get 64 bit byte offset from SCT
handle overflow of HW's 40 bit offset
apply optional append amount
******************************************************************************/
uint64_t BNAV_Indexer_getOffset(BNAV_Indexer_Handle handle, const BSCT_Entry *p_entry)
{
    uint64_t offset;
    unsigned sc = returnStartCode(p_entry->startCodeBytes);

    if (sc == SC_PTS || sc == SC_PES) {
        /* ByteOffsetHi not needed for PTS or PES entry, and handle->hiOverflow state can go wrong if allowed in
        because PTS/PES byte offset may be higher than next start code if start code payload does not complete
        until the next PES packet. */
        return 0;
    }

    offset = create64(p_entry->recordByteCountHi>>24, p_entry->recordByteCount + (p_entry->startCodeBytes & 0xff));
    if (handle->settings.transportTimestampEnabled) {
        offset += 4;
    }

    /* 40 bit HW SCT may overflow. bcmindexer supports 64 bit. */
    if (offset < handle->hiOverflow.last) {
        handle->hiOverflow.cnt++;
        handle->hiOverflow.review = true;
        BDBG_WRN(("SCT 40 bit offset overflow. " BDBG_UINT64_FMT " < " BDBG_UINT64_FMT ", handle->hiOverflow.cnt %u",
            BDBG_UINT64_ARG(offset), BDBG_UINT64_ARG(handle->hiOverflow.last), handle->hiOverflow.cnt));
    }
    handle->hiOverflow.last = offset;
    if (handle->hiOverflow.cnt) {
        offset += create64(handle->hiOverflow.cnt * 0x100, 0);
    }

    if (handle->settings.append.offsetHi || handle->settings.append.offsetLo) {
        /* add the append amount */
        offset += create64(handle->settings.append.offsetHi, handle->settings.append.offsetLo);
    }
    if (handle->priming.offset) {
        if (offset < handle->priming.offset) {
            return INVALID_OFFSET;
        }
        offset -= handle->priming.offset;
    }
    return offset;
}

const char * const BNAV_frameTypeStr[eSCTypeUnknown+1] = {
    "unused", /* eSCTypeSeqHdr*/
    "I",      /* eSCTypeIFrame    I-frame */
    "P",      /* eSCTypePFrame    P-frame */
    "B",      /* eSCTypeBFrame    B-frame */
    "unused", /* eSCTypeGOPHdr    GOP header */
    "RP",     /* eSCTypeRPFrame   Reference picture frame */
    "Unk"    /* eSCTypeUnknown   Unknown or "don't care" frame type */
};

#if !B_REFSW_MINIMAL
int BNAV_Indexer_IsHits(BNAV_Indexer_Handle handle) {
    return handle->isHits;
}

int BNAV_Indexer_SetVChipState(BNAV_Indexer_Handle handle, unsigned short vchipState)
{
    return BNAV_pack_vchip(vchipState, &handle->vchip);
}

unsigned short BNAV_Indexer_GetVChipState(BNAV_Indexer_Handle handle)
{
    return BNAV_unpack_vchip(handle->vchip);
}
#endif

unsigned short BNAV_unpack_vchip(unsigned short packed_vchip)
{
    /* unpack from 12 bits */
    packed_vchip = ((packed_vchip << 2) & 0x3f00) | (packed_vchip & 0x003f);
    /* reset the 1 bits & return */
    return packed_vchip | 0x4040;
}

#if !B_REFSW_MINIMAL
int BNAV_pack_vchip(unsigned short unpacked_vchip, unsigned short *packed_vchip)
{
    /* test the 1 bit in each byte */
    if ((unpacked_vchip & 0x4040) != 0x4040) {
        BDBG_ERR(("Invalid vchip value: 0x%04x", unpacked_vchip));
        return -1;
    }
    /* pack into 12 bits */
    *packed_vchip = ((unpacked_vchip & 0x3f00)>>2) | (unpacked_vchip & 0x003f);
    return 0;
}
#endif

int BNAV_Indexer_GetPosition(BNAV_Indexer_Handle handle, BNAV_Indexer_Position *position)
{
    if (!handle->totalEntriesWritten)
        return -1;
    else {
        BNAV_Entry *p_entry = &handle->lastEntryWritten;
        uint64_t offset;
        switch(handle->settings.videoFormat) {
        case BNAV_Indexer_VideoFormat_AVC_MVC:
        case BNAV_Indexer_VideoFormat_AVC_SVC:
            return BNAV_Indexer_FeedAVC_GetPosition(handle, position);
        default:
            position->index = handle->totalEntriesWritten-1;
            position->pts = BNAV_get_framePts(p_entry);
            offset = BNAV_get_frameOffset(p_entry);
            position->offsetHi = getHi64(offset);
            position->offsetLo = getLo64(offset);
            position->timestamp = BNAV_get_timestamp(p_entry);
            position->vchipState = BNAV_unpack_vchip(BNAV_get_packed_vchip(p_entry));
            return 0;
        }
    }
}

/*
BNAV_Indexer_P_ProcessFullPacketForFeedAvc takes a TS packet and generates SCT entries, just like the SCD hardware.
It must generate 3 SCT types: RAI, start code, PTS.
*/
static long BNAV_Indexer_P_ProcessFullPacket(BNAV_Indexer_Handle handle, long (*feed)(BNAV_Indexer_Handle , const void *, long ))
{
    unsigned offset = 4;
    bool error=false;
    uint8_t *pkt = handle->fullPacket + 4;
    BSCT_SixWord_Entry sct;
    uint32_t scode;
    long rc;

    BDBG_MSG_TRACE(("BNAV_Indexer_P_ProcessFullPacketForFeedAvc"));
#if 0
    {
        unsigned i;
        for (i=0;i<BCMINDEXER_TS_PACKET_SIZE;i++) {
            BKNI_Printf("%02x ", handle->fullPacket[i]);
            if((i+1)%16 == 0) {
                BKNI_Printf("\n");
            }
        }
        BKNI_Printf("\n");
    }
#endif
    BDBG_MSG_TRACE(("adaptation_field_control:%u", handle->adaptFieldControl));
    switch (handle->adaptFieldControl) {
    case 0x01:
        break;
    case 0x03:
        {
            unsigned adaptation_field_length = pkt[0];
            BDBG_MSG_TRACE(("adaptation_field_length:%u", pkt[0]));
            adaptation_field_length ++;
            offset += adaptation_field_length;
            pkt += adaptation_field_length;
            if(offset>=BCMINDEXER_TS_PACKET_SIZE-8) {
                goto done;
            }
        }
#if 0
        {
            unsigned i;
            for (i=0;i<BCMINDEXER_TS_PACKET_SIZE-offset;i++) {
                BKNI_Printf("%02x ", pkt[i]);
                if((i+1)%16 == 0) {
                    BKNI_Printf("\n");
                }
            }
            BKNI_Printf("\n");
        }
#endif
        break;
    default:
        goto done;
    }

    if(pkt[0]==0x00 && pkt[1]==0x00 && pkt[2]==0x01 && (pkt[3]&0xF0) == 0xE0) { /* PES video header */
        if (B_GET_BITS(pkt[7], 7, 6) & 2) {
            uint32_t pts =
             /* pts_32_30 */((uint32_t)B_GET_BITS(pkt[9], 3, 1)<<29) |
             /* pts_29_22 */((uint32_t)pkt[10]<<21) |
             /* pts_21_15 */((uint32_t)B_GET_BITS(pkt[11], 7, 1)<<14) |
             /* pts_14_7 */((uint32_t)pkt[12]<<6) |
             /* pts_6_0 */B_GET_BITS(pkt[13], 7, 2);
            BKNI_Memset(&sct, 0, sizeof(sct));
            sct.startCodeBytes = 0xFE << 24;
            sct.recordByteCountHi = pts;
            sct.recordByteCount = handle->lastPacketOffset.offset; /* lower 32 bits of packet offset */
            BDBG_MSG_TRACE(("  sending PTS SCT %u", pts));
            rc = feed(handle, &sct, 1);
            if(rc<0) { error=true;}
        }
        /* skip through PES start code */
        offset += 4;
        pkt += 4;
    }
    /* Find start codes and build SCT packets. */
    for (scode=0xFF;offset<BCMINDEXER_TS_PACKET_SIZE-8;offset++) { /* we need to capture 8 bytes of payload, so we should stop 8 bytes prior to the end of packet */
        scode = (scode<<8) | *pkt;
        pkt++;
        if((scode>>8) != 0x01) {
            continue;
        }
        BKNI_Memset(&sct, 0, sizeof(sct));
        BDBG_MSG_TRACE(("found scode at %u: %02x %02x %02x %02x %02x %02x %02x %02x %02x",  offset-3, scode&0xFF, pkt[0], pkt[1], pkt[2], pkt[3], pkt[4], pkt[5], pkt[6], pkt[7]));
        /* now we can finish out previous SCT. note that payload[] may extend past the next start code. this is how
        the SCD HW works as well. */
        sct.startCodeBytes = (scode&0xFF) << 24;
        scode = 0xFF;
        sct.startCodeBytes |= pkt[0] << 16;
        sct.startCodeBytes |= pkt[1] << 8;
        sct.recordByteCountHi |= pkt[2] << 16;
        sct.recordByteCountHi |= pkt[3] << 8;
        sct.recordByteCountHi |= pkt[4];
        sct.flags |= pkt[5] << 16;
        sct.flags |= pkt[6] << 8;
        sct.flags |= pkt[7];

        /* set offset */
        sct.startCodeBytes |= offset - 3; /* byte offset from the start of the packet to the start code */
        sct.recordByteCount = getLo64(handle->lastPacketOffset.offset);
        sct.recordByteCountHi |= getHi64(handle->lastPacketOffset.offset)<<24;

        BDBG_MSG_TRACE(("  sending SC SCT"));
        rc = feed(handle, &sct, 1);
        if(rc<0) { error=true;}
    }
done:
    return error?-1:0;
}

int
BNAV_Indexer_P_AddFullPacketPayload(BNAV_Indexer_Handle handle, const BSCT_SixWord_Entry *next)
{
    unsigned offset = handle->fullPacketCount;
    uint8_t *packet;

    if(offset<4 || offset>BCMINDEXER_TS_PACKET_SIZE-23) {
        BDBG_WRN(("out of order full packet SCT (%u:%u)", offset, BCMINDEXER_TS_PACKET_SIZE-23));
        handle->parsingErrors++;
        return -1;
    }
    packet = handle->fullPacket+offset;
    packet[0] = B_GET_BITS(next->word0, 23, 16);
    packet[1] = B_GET_BITS(next->word0, 15, 8);
    packet[2] = B_GET_BITS(next->word0, 7, 0);

    packet[3] = B_GET_BITS(next->word1, 31, 24);
    packet[4] = B_GET_BITS(next->word1, 23, 16);
    packet[5] = B_GET_BITS(next->word1, 15, 8);
    packet[6] = B_GET_BITS(next->word1, 7, 0);

    packet[7] = B_GET_BITS(next->startCodeBytes, 31, 24);
    packet[8] = B_GET_BITS(next->startCodeBytes, 23, 16);
    packet[9] = B_GET_BITS(next->startCodeBytes, 15, 8);
    packet[10] = B_GET_BITS(next->startCodeBytes, 7, 0);

    packet[11] = B_GET_BITS(next->recordByteCount, 31, 24);
    packet[12] = B_GET_BITS(next->recordByteCount, 23, 16);
    packet[13] = B_GET_BITS(next->recordByteCount, 15, 8);
    packet[14] = B_GET_BITS(next->recordByteCount, 7, 0);

    packet[15] = B_GET_BITS(next->recordByteCountHi, 31, 24);
    packet[16] = B_GET_BITS(next->recordByteCountHi, 23, 16);
    packet[17] = B_GET_BITS(next->recordByteCountHi, 15, 8);
    packet[18] = B_GET_BITS(next->recordByteCountHi, 7, 0);

    packet[19] = B_GET_BITS(next->flags, 31, 24);
    packet[20] = B_GET_BITS(next->flags, 23, 16);
    packet[21] = B_GET_BITS(next->flags, 15, 8);
    packet[22] = B_GET_BITS(next->flags, 7, 0);
    offset+= 23;
    handle->fullPacketCount = offset;
    return 0;
}

static long
BNAV_Indexer_P_FilterFullPacket(BNAV_Indexer_Handle handle, const BSCT_SixWord_Entry *sct, long numEntries, long (*feed)(BNAV_Indexer_Handle , const void *, long ))
{
    long i;
    long rc;
    bool error=false;
    const BSCT_SixWord_Entry *next;

    /* filter and process  sequence of full packet SCT entries */
    BDBG_MSG_TRACE(("processing %u entries from %p", (unsigned)numEntries, (void *)sct));
    for(next=sct,i=0;i<numEntries; i++,next++) {
        unsigned type = next->word0>>24;
        /* BDBG_MSG_TRACE(("%u:type %#x", i, type)); */
        if(type==0x80) {
            BDBG_MSG_TRACE(("%u:Full Packet offset %u", (unsigned)i, handle->fullPacketCount));
            if(next!=sct) {
                BDBG_MSG_TRACE(("%u:sending %u middle entries from %p", (unsigned)i, (unsigned)(next-sct), (void *)sct));
                rc = feed(handle, sct, next-sct);
                if(rc<0) {return rc;}
                sct = next;
            }
            sct++;
            if(BNAV_Indexer_P_AddFullPacketPayload(handle, next)!=0) {
                continue;
            }
            if(handle->fullPacketCount == BCMINDEXER_TS_PACKET_SIZE) {
                handle->fullPacketCount = 0;
                rc = BNAV_Indexer_P_ProcessFullPacket(handle, feed);
                if(rc!=0) {error=true;}
            }
        } else {
            if(type==0x01 && B_GET_BIT(next->startCodeBytes, 22)) {
                BDBG_MSG_TRACE(("SCT PAYLOAD_TO_FOLLOW"));
                handle->lastPacketOffset.offset = create64(next->startCodeBytes>>24,next->recordByteCount);
                handle->fullPacketCount = 4;
                handle->fullPacket[0] = 0;
                handle->fullPacket[1] = 0;
                handle->fullPacket[2] = 0;
                handle->fullPacket[3] = 0;
                handle->adaptFieldControl = B_GET_BITS(next->word0, 5, 4);
            } else if(type==0) {
                /* do nothing, SC entries could be between TPIT and RAW payload */
            } else {
                handle->fullPacketCount = 0;
            }
        }
    }
    if(next >= sct+1) {
        BDBG_MSG_TRACE(("sending %u last entries from %#lx", (unsigned)(next-sct), (unsigned long)sct));
        rc = feed(handle, sct, next-sct);
        if(rc<0) {return rc;}
    }
    return error?-1:i;
}

static void BNAV_Indexer_P_ClearSpsPpsTable(BNAV_Indexer_Handle handle)
{
    /* This causes pictures without SPS/PPS to be discarded because their PPS size will be zero */
    BKNI_Memset(handle->avc.sps, 0, sizeof(handle->avc.sps));
    BKNI_Memset(handle->avc.pps, 0, sizeof(handle->avc.pps));
    return;
}

static size_t BNAV_P_StripEmulationPrevention(uint8_t *buf, size_t len)
{
    uint32_t accum;
    unsigned dst_offset;
    unsigned src_offset;
    uint8_t *dst=buf;

    for(dst_offset=src_offset=0,accum=0xFFFFFFFF;src_offset<len;src_offset++) {
        uint8_t byte = buf[src_offset];

        accum = byte | (accum << 8);
        accum = B_GET_BITS(accum, 23, 0);
        if(accum==0x000001) { /* if found 00 00 01 sequence it means end of the NAL, so stop right there */
            break;
        }
        if(accum != 0x000003) {
            dst[dst_offset] = (uint8_t)byte;
            dst_offset++;
        }
    }
    return dst_offset;
}

/* NOTE: XPT has been configured to *not* remove start code emulation prevention for AVC indexing.
This is incorrect. Start codes could be missed. However, we have many years of AVC indexing without
this change, and no actual failing stream, so we're only adding a comment at this point. */
static long
BNAV_Indexer_FeedAVC(BNAV_Indexer_Handle handle, const void *p_bfr, long numEntries)
{
    long i;
    const BSCT_SixWord_Entry *p_curBfr;
    BNAV_AVC_Entry *avcEntry = &handle->avcEntry;

    /* no AVC hits */
    handle->isHits = 0;

    p_curBfr = (const BSCT_SixWord_Entry *)p_bfr;

    for(i=0; i<numEntries; i++, p_curBfr++)
    {
        unsigned char sc = returnStartCode(p_curBfr->startCodeBytes); /* parse once */
        uint64_t offset;
        unsigned nal_unit_type;
        unsigned nal_ref_idc;
#define TOTAL_PAYLOAD 8
        unsigned char payload[TOTAL_PAYLOAD];
        unsigned index = 0, bit = 7;
        BSCT_Entry *p_curSct4 = CONVERT_TO_SCT4(p_curBfr);

        BDBG_MSG(("SCT %08x %08x %08x %08x %08x %08x",
            ((uint32_t*)p_curBfr)[0],
            ((uint32_t*)p_curBfr)[1],
            ((uint32_t*)p_curBfr)[2],
            ((uint32_t*)p_curBfr)[3],
            ((uint32_t*)p_curBfr)[4],
            ((uint32_t*)p_curBfr)[5]));

        if (bcmindexer_p_filter_sct(handle, p_curBfr)) {
            continue;
        }

        /* Grab the PTS */
        if (sc == 0xfe) {
            handle->next_pts = BNAV_Indexer_returnPts(p_curSct4);
            BDBG_MSG_TRACE(("PTS %08lx", handle->next_pts));
            continue;
        }
        if (sc & 0x80) {
            /* if forbidden_zero_bit is set, this is not an AVC start code */
            continue;
        }

        offset = BNAV_Indexer_getOffset(handle, p_curSct4);
        if (offset == INVALID_OFFSET) continue;

        VALIDATE_AVC_SC(handle, p_curSct4,sc);

        BDBG_MSG(("sc %02x at " BDBG_UINT64_FMT, sc, BDBG_UINT64_ARG(offset)));
        nal_ref_idc = (sc >> 5) & 0x3;
        nal_unit_type = sc & 0x1F;

        /* extract 8 bytes of payload from BSCT_SixWord_Entry fields. see RDB for documentation on this. */
        payload[0] = (p_curBfr->startCodeBytes >> 16) & 0xFF;
        payload[1] = (p_curBfr->startCodeBytes >> 8) & 0xFF;
        payload[2] = (p_curBfr->recordByteCountHi >> 16) & 0xFF;
        payload[3] = (p_curBfr->recordByteCountHi >> 8) & 0xFF;
        payload[4] = p_curBfr->recordByteCountHi & 0xFF;
        payload[5] = (p_curBfr->flags >> 16) & 0xFF;
        payload[6] = (p_curBfr->flags >> 8) & 0xFF;
        payload[7] = p_curBfr->flags & 0xFF;

        BDBG_MSG(("payload %02x%02x%02x%02x%02x%02x%02x%02x",
            payload[0],payload[1],payload[2],payload[3],payload[4],payload[5],payload[6],payload[7]));

        /* complete pending PPS or SPS because we've hit the next NAL */
        if (handle->avc.current_pps >= 0) {
            /* complete the PPS */
            int id = handle->avc.current_pps;
            handle->avc.pps[id].size = offset - handle->avc.pps[id].offset;
            handle->avc.current_pps = -1;
        }
        else if (handle->avc.current_sps >= 0) {
            /* complete the SPS */
            int id = handle->avc.current_sps;
            handle->avc.sps[id].size = offset - handle->avc.sps[id].offset;
            handle->avc.current_sps = -1;
        }

        /* We must call BNAV_Indexer_completeAVCFrame UNLESS we have IDR/non-IDR slice
        with first_mb_in_slice != 0. So handle all the "other" cases in one spot. */
        if (nal_unit_type != 1 && nal_unit_type != 5 && nal_unit_type != 14 ) {
            handle->picEnd = offset;
            BNAV_Indexer_completeAVCFrame( handle );
        }

        /* if SEI and preceding was not an SEI, remember this offset for start point of next PPS, SPS or picture */
        if (nal_unit_type == 6 && !handle->avc.current_sei_valid) {
            handle->avc.current_sei_valid = true;
            handle->avc.current_sei = offset;
        }

        switch (nal_unit_type) {
        case 1: /* non-IDR slice */
        case 5: /* IDR slice */
            {
            unsigned first_mb_in_slice, slice_type, pic_parameter_set_id;
            unsigned sps_id;

            /* vlc decode the payload */
            first_mb_in_slice = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);
            slice_type = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);
            pic_parameter_set_id = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);

            /* check the pps. anything above this is an invalid AVC stream. */
            if (pic_parameter_set_id >= TOTAL_PPS_ID) {
                BDBG_ERR(("pic_parameter_set_id %d", pic_parameter_set_id));
                handle->parsingErrors++;
                return -1;
            }
            /* check the sps right away. if we use a PPS without receiving it's SPS first, we should discard. */
            sps_id = handle->avc.pps[pic_parameter_set_id].sps_id;
            if (sps_id >= TOTAL_SPS_ID) {
                BDBG_ERR(("seq_parameter_set_id %d from pps %d", sps_id, pic_parameter_set_id));
                handle->parsingErrors++;
                return -1;
            }

            BDBG_MSG(("%s slice: mb=%-4d type=%-2d pps=%-3d offset=" BDBG_UINT64_FMT,
                (nal_unit_type==1)?"non-IDR":"IDR", first_mb_in_slice, slice_type, pic_parameter_set_id, BDBG_UINT64_ARG(offset)));

            if (first_mb_in_slice == 0) {
                /* we have the beginning of a new frame. first, complete the previous frame. */
                handle->picEnd = offset;
                BNAV_Indexer_completeAVCFrame( handle );

                /* start the next frame */
                handle->picStart = handle->avc.current_sei_valid?handle->avc.current_sei:offset; /* start with preceding SEI's if present */
                BNAV_set_frameOffset(avcEntry, handle->picStart);

                /* default to I frame until P or B slices indicate differently */
                BNAV_set_frameType(avcEntry, eSCTypeIFrame);
                handle->isISlice = 1;
                handle->avc.is_reference_picture = 0;

                /* Set PPS and SPS */
                BNAV_set_seqHdrStartOffset(avcEntry, handle->picStart - handle->avc.pps[pic_parameter_set_id].offset);
                BNAV_set_seqHdrSize(avcEntry,
                    handle->avc.pps[pic_parameter_set_id].size);
                BNAV_set_SPS_Offset(avcEntry, handle->picStart - handle->avc.sps[sps_id].offset);
                BNAV_set_SPS_Size(avcEntry,
                    handle->avc.sps[sps_id].size);

                BDBG_MSG(("sps %d: %#lx, %ld", sps_id, BNAV_get_SPS_Offset(avcEntry), BNAV_get_SPS_Size(avcEntry)));

                /* check for P slice, which means it's not a B frame */
                if (slice_type == 0 || slice_type == 5) {
                    /* First P after first I allows open GOP B's to be saved */
                    if (handle->hitFirstISlice)
                        handle->allowOpenGopBs = 1;
                }

                BNAV_set_refFrameOffset(avcEntry, handle->rfo);

                if (handle->hitFirstISlice) {
                    handle->rfo++;
                }

                BNAV_set_framePts(avcEntry, handle->next_pts);

                /* an IDR with SPS/PPS within a packet is equivalent to an RAI, so mark it as such */
                if (!handle->random_access_indicator && nal_unit_type == 5) {
                    /* TODO: bcmindexer doesn't know if packetSize is 188 or 192. hardcode 188 for now. */
                    unsigned packetSize = 188;
                    /* SPS must precede PPS, so only test distance to SPS.
                    spsOffset is byte aligned distance from the byte aligned start of the picture to the start of the SPS.
                    if the SPS is within the packet or the previous packet, mark the RAI */
                    if (BNAV_get_SPS_Offset(avcEntry) <= packetSize + (BNAV_get_frameOffset(avcEntry) % packetSize)) {
                        handle->random_access_indicator = true;
                    }
                }
                BNAV_set_RandomAccessIndicator(avcEntry, handle->random_access_indicator);
                handle->random_access_indicator = false;
            }

            if (nal_unit_type == 5 && (slice_type !=2 && slice_type != 7)) {
                BDBG_ERR(("IDR frame with non-I slices"));
                handle->parsingErrors++;
                return -1;
            }

            /* test if the slice is a referenced by another slice */
            if (nal_ref_idc) {
                handle->avc.is_reference_picture = 1;
            }

            /* test every slice to determine frame type */
            switch (slice_type) {
            case 2:
            case 7:
                /* we've already defaulted to I frame */
                break;
            case 0:
            case 5:
                /* if we ever get one P slice, then it's either a P or B frame, cannot be I */
                if (BNAV_get_frameType(avcEntry) == eSCTypeIFrame) {
                    handle->isISlice = 0;
                    BNAV_set_frameType(avcEntry, eSCTypePFrame);
                }
                break;
            case 1:
            case 6:
                /* if we ever get one B slice, it's a B frame */
                handle->isISlice = 0;
                BNAV_set_frameType(avcEntry, eSCTypeBFrame);
                break;
            default:
                BDBG_ERR(("unsupported slice_type %d", slice_type));
                handle->parsingErrors++;
                break;
            }
            }
            break;
        case 7: /* sequence parameter set */
            {
            unsigned profile_idc, constraint_flags, level_idc, seq_parameter_set_id;

            /* parse and vlc decode the payload */
            profile_idc = payload[0];
            constraint_flags = payload[1];
            level_idc = payload[2];
            index = 3;
            seq_parameter_set_id = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);
            if (seq_parameter_set_id >= TOTAL_SPS_ID) {
                BDBG_ERR(("corrupt seq_parameter_set_id %u", seq_parameter_set_id));
                handle->parsingErrors++;
                /* clear SPS/PPS table on corruption to prevent lookup into old metadata */
                BNAV_Indexer_P_ClearSpsPpsTable(handle);
                return -1;
            }
            handle->avc.sps[seq_parameter_set_id].offset = handle->avc.current_sei_valid?handle->avc.current_sei:offset; /* start with preceding SEI's if present */
            handle->avc.current_sps = seq_parameter_set_id;

            BDBG_MSG(("SeqParamSet: profile_idc=%u flags=0x%x level_idc=%d SPS=%d offset=" BDBG_UINT64_FMT,
                profile_idc, constraint_flags, level_idc, seq_parameter_set_id, BDBG_UINT64_ARG(offset)));
            }
            break;
        case 8: /* picture parameter set */
            {
            unsigned pic_parameter_set_id, seq_parameter_set_id;

            /* vlc decode payload */
            pic_parameter_set_id = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);
            if (pic_parameter_set_id >= TOTAL_PPS_ID) {
                BDBG_ERR(("corrupt pic_parameter_set_id %u", pic_parameter_set_id));
                handle->parsingErrors++;
                BNAV_Indexer_P_ClearSpsPpsTable(handle);
                return -1;
            }
            seq_parameter_set_id = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);
            if (seq_parameter_set_id >= TOTAL_SPS_ID) {
                BDBG_ERR(("corrupt seq_parameter_set_id %u", seq_parameter_set_id));
                handle->parsingErrors++;
                BNAV_Indexer_P_ClearSpsPpsTable(handle);
                return -1;
            }

            handle->avc.pps[pic_parameter_set_id].offset = handle->avc.current_sei_valid?handle->avc.current_sei:offset; /* start with preceding SEI's if present */
            handle->avc.pps[pic_parameter_set_id].sps_id = seq_parameter_set_id;
            handle->avc.current_pps = pic_parameter_set_id;

            BDBG_MSG(("PicParamSet: PPS=%d, SPS=%d offset=" BDBG_UINT64_FMT,
                pic_parameter_set_id, seq_parameter_set_id, BDBG_UINT64_ARG(offset)));
            }
            break;

#if 0
/* This code is optional because we are detecting the picture type by examining each slice. */
        case 9: /* access unit delimiter */
            {
#if BDBG_DEBUG_BUILD
            static const char *primary_pic_type_str[] = {
                "I",
                "I,P",
                "I,P,B",
                "SI",
                "SI,SP",
                "I,SI",
                "I,SI,P,SP",
                "I,SI,P,SP,B"
            };
#endif
            int primary_pic_type;
            primary_pic_type = (payload[0] >> 5) & 0x3;

            BDBG_MSG(("AUD %d (%s)", primary_pic_type, primary_pic_type_str[primary_pic_type]));
            }
            break;
#endif
        }

        /* after any non-SEI, the current_sei is no longer valid */
        if (nal_unit_type != 6) {
            handle->avc.current_sei_valid = false;
        }
    }

    return i;
}

static long
BNAV_Indexer_FeedAVS(BNAV_Indexer_Handle handle, void *p_bfr, long numEntries)
{
    long i;
    const BSCT_Entry *p_curBfr=NULL;
    const BSCT_SixWord_Entry *p_cur6WordBfr=NULL;
    BNAV_Entry *navEntry = &handle->curEntry;

    p_cur6WordBfr = (const BSCT_SixWord_Entry *)p_bfr;

    for (i=0; i<numEntries; i++,p_cur6WordBfr++)
    {
        int sc;
        uint64_t offset = 0;

        p_curBfr = CONVERT_TO_SCT4(p_cur6WordBfr);

        if (bcmindexer_p_filter_sct(handle, p_cur6WordBfr)) {
            continue;
        }

        sc = returnStartCode(p_curBfr->startCodeBytes); /* parse once */
        if (sc != SC_AVS_PTS) {
            offset = BNAV_Indexer_getOffset(handle, p_curBfr);
            if (offset == INVALID_OFFSET) continue;
        }

        /* detect invalid start code and offsets */
        VALIDATE_AVS_SC(handle, p_curBfr,sc);

        BDBG_MSG(("AVS SCT Entry: %02x " BDBG_UINT64_FMT, sc, BDBG_UINT64_ARG(offset)));
        if (handle->seqHdrFlag)
        {
            if (sc != SC_AVS_PES && sc != SC_AVS_PTS && sc != SC_AVS_EXTENSION && sc != SC_AVS_USER_DATA)
            {
                handle->seqHdrFlag = 0;
                handle->seqHdrSize = offset - handle->seqHdrStartOffset;
            }
        }

        switch (sc)
        {
        case SC_AVS_FIRST_SLICE:
            break;

        case SC_AVS_PES:
            break;

        case SC_AVS_PTS:
            handle->next_pts = BNAV_Indexer_returnPts(p_curBfr);
            break;

        case SC_AVS_SEQUENCE:
            handle->seqHdrStartOffset = offset;
            handle->seqHdrFlag = 1; /* set on next qualifying sct */

            /* new complete any pending frame */
            handle->picEnd = handle->seqHdrStartOffset;
            BNAV_Indexer_completeFrame( handle );
            break;

        case SC_AVS_SEQ_END:   /* TODO: Change me to any non-slice */
            /* complete any pending frame */
            handle->picEnd = offset;
            BNAV_Indexer_completeFrame( handle );
            break;

        case SC_AVS_PICTURE_I:
        case SC_AVS_PICTURE_PB:
            /* complete any pending frame */
            handle->picEnd = offset;
            BNAV_Indexer_completeFrame( handle );

            /* start a new frame */
            handle->picStart = offset;

            BNAV_set_frameOffset(navEntry, offset);

            if (sc == SC_AVS_PICTURE_I)
            {
                handle->isISlice = 1;   /* indicated I picture */
                handle->isHits = 0;     /* for AVS this should always be 0 */
                BNAV_set_frameType(navEntry, eSCTypeIFrame);
            }
            else
            {
                switch (returnAvsPictureCode(p_curBfr->recordByteCountHi))
                {
                case PC_AVS_P_FRAME:
                    /* First P after first I allows open GOP B's to be saved */
                    /* if (handle->hitFirstISlice)
                        handle->allowOpenGopBs = 1; */
                    BNAV_set_frameType(navEntry, eSCTypePFrame);
                    break;
                case PC_B_FRAME:
                    BNAV_set_frameType(navEntry, eSCTypeBFrame);
                    break;
                }
            }

            /* make sequence header offset relative to current frame rather than */
            /* relative to reference frame to allow removal of b-frames */
            BNAV_set_seqHdrSize(navEntry, (unsigned short)handle->seqHdrSize);
            BNAV_set_seqHdrStartOffset(navEntry, handle->picStart - handle->seqHdrStartOffset);

            BNAV_set_refFrameOffset(navEntry, handle->rfo);
            if (handle->hitFirstISlice) {
                handle->rfo++;
            }

            BNAV_set_framePts(navEntry, handle->next_pts);
            break;
        default:
            break;

        }

    }

    return i;
}

static long
BNAV_Indexer_FeedHEVC(BNAV_Indexer_Handle handle, const void *p_bfr, long numEntries)
{
    long i;
    const BSCT_SixWord_Entry *p_curBfr;
    BNAV_AVC_Entry *avcEntry = &handle->avcEntry;

    /* no HEVC hits */
    handle->isHits = 0;

    p_curBfr = (const BSCT_SixWord_Entry *)p_bfr;

    for(i=0; i<numEntries; i++, p_curBfr++)
    {
        unsigned char sc = returnStartCode(p_curBfr->startCodeBytes); /* parse once */
        uint32_t nal_header;
#define TOTAL_PAYLOAD 8
        unsigned char payload[TOTAL_PAYLOAD];
        const BSCT_Entry *p_curSct4 = CONVERT_TO_SCT4(p_curBfr);
        uint64_t offset;
        bool forbidden_zero_bit;
        unsigned nal_unit_type;
        size_t payload_len;
        batom_vec vec;
        batom_cursor cursor;
        batom_bitstream bs;

        BDBG_MSG(("SCT %08x %08x %08x %08x %08x %08x",
            ((uint32_t*)p_curBfr)[0],
            ((uint32_t*)p_curBfr)[1],
            ((uint32_t*)p_curBfr)[2],
            ((uint32_t*)p_curBfr)[3],
            ((uint32_t*)p_curBfr)[4],
            ((uint32_t*)p_curBfr)[5]));

        if (bcmindexer_p_filter_sct(handle, p_curBfr)) {
            continue;
        }

        /* Grab the PTS */
        if (sc == 0xfe) {
            /* we are using PTS entry as frame separator */
            handle->hevc.newFrame = true;
            handle->next_pts = BNAV_Indexer_returnPts(p_curSct4);
            BDBG_MSG_TRACE(("PTS %08lx", handle->next_pts));
            continue;
        }
        offset = BNAV_Indexer_getOffset(handle, p_curSct4);
        if (offset == INVALID_OFFSET) continue;

        /* extract 8 bytes of payload from BSCT_SixWord_Entry fields. see RDB for documentation on this. */
        payload[0] = (p_curBfr->startCodeBytes >> 16) & 0xFF;
        payload[1] = (p_curBfr->startCodeBytes >> 8) & 0xFF;
        payload[2] = (p_curBfr->recordByteCountHi >> 16) & 0xFF;
        payload[3] = (p_curBfr->recordByteCountHi >> 8) & 0xFF;
        payload[4] = p_curBfr->recordByteCountHi & 0xFF;
        payload[5] = (p_curBfr->flags >> 16) & 0xFF;
        payload[6] = (p_curBfr->flags >> 8) & 0xFF;
        payload[7] = p_curBfr->flags & 0xFF;

        BDBG_MSG((BDBG_UINT64_FMT ":sc %02x payload %02x%02x%02x%02x%02x%02x%02x%02x", BDBG_UINT64_ARG(offset),
            sc, payload[0],payload[1],payload[2],payload[3],payload[4],payload[5],payload[6],payload[7]));

        /* ITU-T H.265 (04/2013) */
        /* 7.3.1.2 NAL unit header syntax */
        nal_header = (sc << 8) | payload[0];
        forbidden_zero_bit = B_GET_BIT(nal_header, 15);
        nal_unit_type = B_GET_BITS(nal_header, 14, 9);
        BDBG_MSG(("nal_header:%#x forbidden_zero_bit:%u nal_unit_type:%u", nal_header, forbidden_zero_bit, nal_unit_type));
        if(forbidden_zero_bit) {
            /* if forbidden_zero_bit is set, this is not an HEVC start code, so ignore  it */
            /* This is _NOT_ an error */
            continue;
        }
        /* complete pending PPS or SPS because we've hit the next NAL */
        if (handle->avc.current_pps >= 0) {
            /* complete the PPS */
            int id = handle->avc.current_pps;
            handle->avc.pps[id].size = offset - handle->avc.pps[id].offset;
            handle->avc.current_pps = -1;
        }
        else if (handle->avc.current_sps >= 0) {
            /* complete the SPS */
            int id = handle->avc.current_sps;
            handle->avc.sps[id].size = offset - handle->avc.sps[id].offset;
            handle->avc.current_sps = -1;
        }
        if(handle->hevc.newFrame) {
            handle->hevc.newFrame = false;
            if(handle->hevc.frameTypeValid) {
                handle->picEnd = offset;
                handle->picStart = handle->hevc.sliceStart;
                if(handle->hevc.sliceStart != handle->hevc.accessUnitStart) {
                    unsigned offset = handle->picStart - handle->hevc.accessUnitStart;
                    /* 'sequence' is everything from start of access unit to the first slice */
                    BNAV_set_seqHdrStartOffset(avcEntry, offset);
                    BNAV_set_seqHdrSize(avcEntry, handle->hevc.sliceStart - handle->hevc.accessUnitStart);
                    /* SPS is empty */
                    BNAV_set_SPS_Offset(avcEntry, offset);
                    BNAV_set_SPS_Size(avcEntry, 0);
#if 0
                    BDBG_LOG((BDBG_UINT64_FMT " " BDBG_UINT64_FMT, BDBG_UINT64_ARG(handle->picStart - handle->hevc.accessUnitStart), BDBG_UINT64_ARG(handle->hevc.sliceStart - handle->hevc.accessUnitStart)));
#endif
                }
                BNAV_set_frameOffset(avcEntry, handle->hevc.sliceStart);
                handle->avc.is_reference_picture = BNAV_get_frameType(avcEntry) != eSCTypeBFrame; /* to undo frame type conversion in BNAV_Indexer_completeAVCFrame */
                BNAV_Indexer_completeAVCFrame( handle );
            }
            /* default to I frame until P or B slices indicate differently */
            BNAV_set_frameType(avcEntry, eSCTypeIFrame);
            handle->hevc.accessUnitStart = offset;
            handle->hevc.frameTypeValid = false; /* but masked by this */
            handle->isISlice = 1;
            handle->avc.is_reference_picture = 0;
            handle->hevc.sliceStartSet = false;
        }
        payload_len = BNAV_P_StripEmulationPrevention(payload+1, TOTAL_PAYLOAD-1);
        BATOM_VEC_INIT(&vec, payload+1, payload_len);
        batom_cursor_from_vec(&cursor, &vec, 1);
        /* batom_cursor_dump(&cursor, "nalu"); */
        batom_bitstream_init(&bs, &cursor);

        /* Table 7-1 € NAL unit type codes and NAL unit type classes */
        switch(nal_unit_type) {
        case 39: /* PREFIX_SEI_NUT */ case 40: /* SUFFIX_SEI_NUT */
            handle->avc.current_sei_valid = true;
            handle->avc.current_sei = offset;
            break;

        case 33: /* SPS_NUT */
            {
                /* 7.3.2.2 Sequence parameter set RBSP syntax */
                unsigned  sps_video_parameter_set_id;
                sps_video_parameter_set_id = batom_bitstream_bits(&bs, 4);

                if ( sps_video_parameter_set_id >= sizeof(handle->avc.sps)/sizeof(handle->avc.sps[0]) ) {
                    handle->parsingErrors++;
                    BDBG_WRN(("Invalid SPS Entry at offset " BDBG_UINT64_FMT, BDBG_UINT64_ARG(offset)));
                    break;
                }

                if(!batom_bitstream_eof(&bs)) {
                    handle->avc.sps[sps_video_parameter_set_id].offset = handle->avc.current_sei_valid?handle->avc.current_sei:offset; /* start with preceding SEI's if present */
                    handle->avc.current_sps = sps_video_parameter_set_id;
                }
                BDBG_MSG(("SeqParamSet: SPS=%d offset=" BDBG_UINT64_FMT, sps_video_parameter_set_id, BDBG_UINT64_ARG(offset)));
            }
            break;
        case 34: /* PPS_NUT */
            {
                /* 7.3.2.3 Picture parameter set RBSP syntax */
                unsigned pps_pic_parameter_set_id;
                unsigned pps_seq_parameter_set_id;
                unsigned num_extra_slice_header_bits;
                pps_pic_parameter_set_id = batom_bitstream_exp_golomb(&bs);
                pps_seq_parameter_set_id = batom_bitstream_exp_golomb(&bs);
                /* dependent_slice_segments_enabled_flag */ batom_bitstream_drop_bits(&bs,1);
                /* output_flag_present_flag */ batom_bitstream_drop_bits(&bs,1);
                num_extra_slice_header_bits = batom_bitstream_bits(&bs, 3);
                if(batom_bitstream_eof(&bs) || pps_pic_parameter_set_id >= sizeof(handle->hevc.pps)/sizeof(handle->hevc.pps[0])) {
                    handle->parsingErrors++;
                    BDBG_WRN(("Invalid PPS Entry at offset" BDBG_UINT64_FMT, BDBG_UINT64_ARG(offset)));
                    break;
                }
                handle->hevc.pps[pps_pic_parameter_set_id].valid = true;
                handle->hevc.pps[pps_pic_parameter_set_id].pps_seq_parameter_set_id = pps_seq_parameter_set_id;
                handle->hevc.pps[pps_pic_parameter_set_id].num_extra_slice_header_bits = num_extra_slice_header_bits;

                handle->avc.pps[pps_pic_parameter_set_id].offset = handle->avc.current_sei_valid?handle->avc.current_sei:offset; /* start with preceding SEI's if present */
                handle->avc.pps[pps_pic_parameter_set_id].sps_id = pps_seq_parameter_set_id;
                handle->avc.current_pps = pps_pic_parameter_set_id;
                BDBG_MSG(("PicParamSet: PPS=%d, SPS=%d offset=" BDBG_UINT64_FMT, pps_pic_parameter_set_id, pps_seq_parameter_set_id, BDBG_UINT64_ARG(offset)));
            }
            break;

        /* SLICES */
        /* 3.15 broken link access (BLA) picture */
        /* 3.23 clean random access (CRA) picture */
        /* 3.59 instantaneous decoding refresh (IDR) picture */
        /* 3.109 random access decodable leading (RADL) picture */
        /* 3.111 random access skipped leading (RASL) picture */
        /* 3.138 step-wise temporal sub-layer access (STSA) picture */
        /* 3.151 temporal sub-layer access (TSA) picture */
        case 0: case 1: /* Coded slice segment of a non-TSA, non-STSA trailing picture */
        case 2: case 3: /* Coded slice segment of a TSA picture */
        case 4: case 5: /* Coded slice segment of an STSA picture */
        case 6: case 7: /* Coded slice segment of a RADL picture */
        case 8: case 9: /* Coded slice segment of a RASL picture */
        /* case 10: case 12: case 14: */ /* Reserved non-IRAP sub-layer non-reference VCL NAL unit types */
        case 16: case 17: case 18: /* Coded slice segment of a BLA picture */
        case 19: case 20: /* Coded slice segment of an IDR picture */
        case 21:  /* Coded slice segment of a CRA picture */
            {
                bool first_slice_segment_in_pic_flag;
                unsigned slice_pic_parameter_set_id;
                unsigned i;

                if(!handle->hevc.sliceStartSet) {
                    bool random_access_indicator = handle->random_access_indicator;

                    random_access_indicator = false;
                    handle->hevc.sliceStartSet = true;
                    handle->hevc.sliceStart = offset;
                    if(handle->avc.current_sei_valid) {
                        handle->avc.current_sei_valid = false;
                        handle->hevc.sliceStart = handle->avc.current_sei;
                    }
                    BNAV_set_framePts(avcEntry, handle->next_pts);
                    switch(nal_unit_type) {
                    case 19: case 20: /* Coded slice segment of an IDR picture */
                    case 21:  /* Coded slice segment of a CRA picture */
                    case 16: case 17: case 18: /* Coded slice segment of a BLA picture */
                        handle->rfo=0;
                        random_access_indicator = true;
                        handle->hevc.streamHasIDR = true;
                        break;
                    default:
                        break;
                    }
                    BNAV_set_refFrameOffset(avcEntry, handle->rfo);
                    BNAV_set_RandomAccessIndicator(avcEntry, random_access_indicator);
                    handle->hevc.randomAccessIndicator = random_access_indicator;
                    handle->rfo++;
                }
                /*
                 * 7.3.2.9 Slice segment layer RBSP syntax
                 *
                 * slice_segment_layer_rbsp( ) { Descriptor
                 * slice_segment_header( )
                 *
                 * 7.3.6.1 General slice segment header syntax
                 * slice_segment_header( ) {
                 */
                first_slice_segment_in_pic_flag = batom_bitstream_bit(&bs);
                if(nal_unit_type>=16 /*BLA_W_LP*/ && nal_unit_type<=23 /* RSV_IRAP_VCL23 */) {
                    batom_bitstream_drop_bits(&bs,1); /* no_output_of_prior_pics_flag */
                }
                slice_pic_parameter_set_id = batom_bitstream_exp_golomb(&bs);
                if(batom_bitstream_eof(&bs) || slice_pic_parameter_set_id >= sizeof(handle->hevc.pps)/sizeof(handle->hevc.pps[0])) {
                    handle->parsingErrors++;
                    BDBG_WRN(("Invalid Slice header: offset " BDBG_UINT64_FMT, BDBG_UINT64_ARG(offset)));
                    break;
                }
                if(!handle->hevc.pps[slice_pic_parameter_set_id].valid) {
                    break;
                }
                if(!first_slice_segment_in_pic_flag) {
                    if(handle->hevc.streamHasIDR && !handle->hevc.randomAccessIndicator) {
                        /* if stream has random access indicator (IDR/CRA/BLA), but current slice is not marked as such, demote frame type to P */
                        if(handle->hevc.frameTypeValid && BNAV_get_frameType(avcEntry) == eSCTypeIFrame) {
                            BNAV_set_frameType(avcEntry, eSCTypePFrame);
                            handle->isISlice = 0;
                        }
                    }
                    break; /* can't read slice_segment_address, due to limited number of captured bytes */
                }
                {
                    unsigned slice_type;
                    for(i=0;i<handle->hevc.pps[slice_pic_parameter_set_id].num_extra_slice_header_bits;i++) {
                        batom_bitstream_drop_bits(&bs, 1); /* slice_reserved_flag[i] */
                    }
                    slice_type = batom_bitstream_exp_golomb(&bs);
                    if(batom_bitstream_eof(&bs)) {
                        handle->parsingErrors++;
                        BDBG_WRN(("Invalid Slice header: offset " BDBG_UINT64_FMT, BDBG_UINT64_ARG(offset)));
                        break;
                    }
                    /* test every slice to determine frame type */
                    /* Table 7-7 € Name association to slice_type */
                    BDBG_MSG_TRACE(("frameTypeValid:%u slice_type:%u", handle->hevc.frameTypeValid, slice_type));
                    switch (slice_type) {
                    case 0: /* B slice */
                        BNAV_set_frameType(avcEntry, eSCTypeBFrame);
                        handle->isISlice = 0;
                        handle->hevc.frameTypeValid = true;
                        break;
                    case 1: /* P slice */
                        if(!handle->hevc.frameTypeValid || BNAV_get_frameType(avcEntry) == eSCTypeIFrame) {
                            BNAV_set_frameType(avcEntry, eSCTypePFrame);
                        }
                        handle->isISlice = 0;
                        handle->hevc.frameTypeValid = true;
                        break;
                    case 2: /* I slice */
                        if(!handle->hevc.frameTypeValid) {
                            BNAV_set_frameType(avcEntry, eSCTypeIFrame);
                            handle->isISlice = 1;
                            handle->hevc.frameTypeValid = true;
                        }
                        break;
                    default:
                        BDBG_WRN(("unsupported slice_type %d", slice_type));
                        handle->parsingErrors++;
                        break;
                    }
                }

            }

            break;

        default:
            break;
        }
        /* after any non-SEI, the current_sei is no longer valid */
        if( !(nal_unit_type == 39 || nal_unit_type == 40)) {
            handle->avc.current_sei_valid = false;
        }
    }

    return i;
}

static void BNAV_Indexer_FeedHEVC_Init(BNAV_Indexer_Handle handle)
{
    unsigned i;
    for(i=0;i<sizeof(handle->hevc.pps)/sizeof(handle->hevc.pps[0]);i++) {
        handle->hevc.pps[i].valid = false;
    }
    handle->hevc.frameTypeValid = false;
    handle->hevc.newFrame = false;
    handle->hevc.sliceStartSet = false;
    handle->hevc.randomAccessIndicator = false;
    handle->hevc.streamHasIDR = false;
    return;
}

int BNAV_Indexer_GetStatus( BNAV_Indexer_Handle handle, BNAV_Indexer_Status *pStatus )
{
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->parsingErrors = handle->parsingErrors;
    return 0;
}

void BNAV_Indexer_SetPrimingStart(BNAV_Indexer_Handle handle, uint64_t offset)
{
    handle->priming.offset = offset;
    if (!handle->settings.simulatedFrameRate && !handle->settings.ptsBasedFrameRate) {
        /* real, not simulated */
        handle->lasttime = handle->starttime = currentTimestamp();
    }
}
