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
 * Module Description: Private types and prototypes for bcmindexer & bcmplayer
 *
 ***************************************************************************/
#ifndef BCMINDEXERPRIV_H
#define BCMINDEXERPRIV_H

#include "bcmindexer_svc.h"
#include "bcmindexer_nav.h"

/* Convert eSCType to a string */
extern const char * const BNAV_frameTypeStr[eSCTypeUnknown+1];

struct BNAV_Indexer_HandleImpl
{
    BNAV_Indexer_Settings settings;

    char            seqHdrFlag;             /* Flag indicating if MPEG2 sequence header information was found.  */
    char            isISlice;               /* Flag indicating that we found an
                                               I-slice or I-frame */
    char            isHits;                 /* Flag indicating we're dealing with a
                                               HITS-stream */
    char            hitFirstISlice;         /* Flag indicating that we've hit the
                                               first I-slice */
    unsigned        seqHdrSize;             /* Number of bytes in the sequence
                                               header */
    unsigned        seqHdrStartOffset;      /* Sequence header offset (in bytes) relative to frame offset */
    uint64_t        picStart;               /* Offset of start of picture */
    uint64_t        picEnd;                 /* Offset of end of picture */
    struct {
        unsigned count;      /* count of fields in a field encoded picture */
        unsigned lastField;
        bool skippedPicture; /* SCT was skipped because it was believed to be a second field in an already indexed frame */
        BSCT_Entry skippedPictureSct; /* valid if skippedPicture is true */
        unsigned temporal_reference;
        unsigned picture_coding_type;
    } fieldEncoded;
    unsigned char   rfo;                    /* Reference frame offset counter */

    BNAV_Entry      curEntry;               /* The current MPEG2 structure that is being populated */
    BNAV_AVC_Entry  avcEntry;               /* The current AVC structure that is being populated. */

    unsigned long   next_pts;               /* pts buffer for next picture frame */
    bool random_access_indicator;

    unsigned starttime;              /* base timestamp for calculating NAV timestamp */
    unsigned lasttime;               /* intermediate last timestamp for timestamp algo, not true last */
    unsigned actual_lasttime;        /* last timestamp written to NAV entry */
    struct {
        /* pts and timestamp at beginning of this PTS continuity */
        unsigned startPts;
        unsigned startTimestamp;
        
        /* last PTS assigned. used for interpolated PTSs */
        unsigned frameCount; /* frame count since lastPts */
        unsigned lastPts;
        unsigned rate; /* in milliseconds/frame units */
    } ptsBasedFrameRate;

    unsigned        prev_pc;                /* previous picture code */
    int             prev_I_rfo;             /* This contains the rfo for the previous
                                                I frame, until the next P frame is detected.
                                                This value is added to the rfo of the open gop
                                                B frames. */
    unsigned short  vchip;                  /* Current packed vchip state */
    unsigned parsingErrors;

    struct {
#define MAX_GOP_SIZE 50
        BNAV_Entry      entry[MAX_GOP_SIZE];
        int total_entries;
    } reverse;

    /* this information is used to report BNAV_Indexer_GetPosition consistently */
    int             totalEntriesWritten;
    BNAV_Entry      lastEntryWritten;
    int allowOpenGopBs;

    /* define structures for AVC indexing */
#define TOTAL_PPS_ID 256
#define TOTAL_SPS_ID 32
    struct {
        int current_sps, current_pps; /* if -1, then no SPS or PPS being captured */
        int is_reference_picture; /* if true, then this picture has at least one slice that is referenced by another slice.
            this picture is not dropppable. */
        struct {
            uint64_t offset;
            unsigned size;
            int sps_id;
        } pps[TOTAL_PPS_ID];
        struct {
            uint64_t offset;
            unsigned size;
        } sps[TOTAL_SPS_ID];
        uint64_t current_sei;
        bool current_sei_valid;
    } avc;

    /* instead of complicated wraparound logic, I buffer a minimum amount before
    processing PES payload. MIN_PES_PAYLOAD includes the startcode byte itself. */
    #define MIN_PES_PAYLOAD 5
    struct {
        unsigned char buf[MIN_PES_PAYLOAD + 2];
        int bufsize;
        int sccount;
        uint64_t offset;

        bool vc1_interlace;
        uint64_t sequence_offset;
        int sequence_size;
        uint64_t entrypoint_offset;
        int entrypoint_size;
    } pes;

    /* bcmindexer "full packet" support allows bcmindexer to process a whole TS packet in the ITB.
    The whole TS packet is needed if the # of ITB's is greater than 10 per TS packet.
    You must #define BXPT_STARTCODE_BUFFER_WORKAROUND in XPT PI to enable this feature in the FW. */
    unsigned fullPacketCount;
    unsigned adaptFieldControl;
#define BCMINDEXER_TS_PACKET_SIZE 188
    uint8_t fullPacket[BCMINDEXER_TS_PACKET_SIZE];
    struct {
        uint64_t offset;
    } lastPacketOffset;
    BNAV_Indexer_SVC_Info svc;
    struct {
        struct {
            /* uint8_t pps_pic_parameter_set_id; */
            bool valid;
            uint8_t pps_seq_parameter_set_id;
            uint8_t num_extra_slice_header_bits;
        } pps[64];
        bool frameTypeValid; /* set to true if frameType was set */
        bool newFrame;
        bool sliceStartSet;
        bool streamHasIDR;
        bool randomAccessIndicator;
        uint64_t sliceStart;       /* Offset of first slice NAL */
        uint64_t accessUnitStart;  /* Offset of first NAL after PTS */
    } hevc;
    struct {
        uint64_t last; /* last value seen from HW SCT */
        unsigned cnt;  /* number of overflows */
        bool review; /* latest hiOverflow.cnt increment is subject review. if bad, decrement. */
    } hiOverflow;
    struct {
        uint64_t offset;
    } priming;
};

int BNAV_P_FeedPES_VC1(BNAV_Indexer_Handle handle, uint8_t *p_bfr, unsigned size);

int BNAV_Indexer_completeFrameAux(BNAV_Indexer_Handle handle,
    void *data, /* data points to either a BNAV_Entry or a BNAV_AVC_Entry. */
    int size  /* size of data */
    );

void BNAV_Indexer_StampEntry(BNAV_Indexer_Handle handle, BNAV_Entry *entry);

int BNAV_Indexer_P_AddFullPacketPayload(BNAV_Indexer_Handle handle, const BSCT_SixWord_Entry *next);
uint64_t BNAV_Indexer_getOffset( BNAV_Indexer_Handle handle, const BSCT_Entry *p_entry);

/* If we offset two words into an SCT6 entry, it looks just like an SCT4 entry,
ignoring the extra payload bytes */
#define CONVERT_TO_SCT4(P_SCT6_ENTRY) \
    ((BSCT_Entry*)((char*)P_SCT6_ENTRY+(sizeof(BSCT_SixWord_Entry)-sizeof(BSCT_Entry))))

#endif
