/***************************************************************************
 *     Copyright (c) 2002-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Private types and prototypes for bcmindexer & bcmplayer
 *
 * Revision History:
 *
 * $brcm_Log: $
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
    uint32_t        seqHdrStartOffset;      /* Sequence header offset (in bytes) relative to frame offset */
    uint32_t        picStart;               /* Offset of start of picture */
    uint32_t        picEnd;                 /* Offset of end of picture */
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
            uint32_t offset;
            unsigned size;
            int sps_id;
        } pps[TOTAL_PPS_ID];
        struct {
            uint32_t offset;
            unsigned size;
        } sps[TOTAL_SPS_ID];
        uint32_t current_sei;
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
        unsigned offset, offsetHi;
    } lastPacketOffset;
    BNAV_Indexer_SVC_Info svc;
    struct {
        struct {
            /* uint8_t pps_pic_parameter_set_id; */
            bool valid;
            uint8_t pps_seq_parameter_set_id;
            bool dependent_slice_segments_enabled_flag;
            uint8_t num_extra_slice_header_bits;
        } pps[64];
        bool frameTypeValid; /* set to true if frameType was set */
        bool newFrame;
        bool sliceStartSet;
        uint32_t sliceStart;       /* Offset of first slice NAL */
        uint32_t sliceStartHi;  /* Offset of first NAL after PTS */
        uint32_t accessUnitStart;  /* Offset of first NAL after PTS */
    } hevc;
    struct {
        unsigned last; /* last value seen from HW SCT */
        unsigned cnt;  /* number of overflows */
    } hiOverflow;
};

int BNAV_P_FeedPES_VC1(BNAV_Indexer_Handle handle, uint8_t *p_bfr, unsigned size);

int BNAV_Indexer_completeFrameAux(BNAV_Indexer_Handle handle,
    void *data, /* data points to either a BNAV_Entry or a BNAV_AVC_Entry. */
    int size  /* size of data */
    );

void BNAV_Indexer_StampEntry(BNAV_Indexer_Handle handle, BNAV_Entry *entry);

int BNAV_Indexer_P_AddFullPacketPayload(BNAV_Indexer_Handle handle, const BSCT_SixWord_Entry *next);
void BNAV_Indexer_getOffset( BNAV_Indexer_Handle handle, const BSCT_Entry *p_entry, uint32_t *pHi, uint32_t *pLo );

/* If we offset two words into an SCT6 entry, it looks just like an SCT4 entry,
ignoring the extra payload bytes */
#define CONVERT_TO_SCT4(P_SCT6_ENTRY) \
    ((BSCT_Entry*)((char*)P_SCT6_ENTRY+(sizeof(BSCT_SixWord_Entry)-sizeof(BSCT_Entry))))

#endif
