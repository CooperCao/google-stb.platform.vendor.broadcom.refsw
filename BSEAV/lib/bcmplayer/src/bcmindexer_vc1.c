/***************************************************************************
 *     Copyright (c) 2006-2009, Broadcom Corporation
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
 * Module Description: Converts startcode index to bcmplayer index
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bcmindexer.h"
#include "bcmindexerpriv.h"
#include "mpeg2types.h"

#if !B_REFSW_MINIMAL

BDBG_MODULE(bcmindexer_vc1);

#if 0
#undef BDBG_MSG
#define BDBG_MSG BDBG_WRN
#endif

struct vc1_vlc_code {
    unsigned long bitstring;
    int length;
    eSCType frameType;
};

static const struct vc1_vlc_code g_fcm[] = {
    {0x0, 1, 0}, /* 0 - progressive */
    {0x2, 2, 0}, /* 10 - frame-interlace */
    {0x3, 2, 0}, /* 11 - field-interlace */
    {0,0,0} /* terminal */
};
static const struct vc1_vlc_code g_ptype[] = {
    {0x6, 3, eSCTypeIFrame}, /* I */
    {0x0, 1, eSCTypePFrame}, /* P */
    {0x2, 2, eSCTypeBFrame}, /* B */
    {0xE, 4, eSCTypeBFrame}, /* BI */
    {0xF, 4, eSCTypeBFrame}, /* skipped */
    {0,0,0} /* terminal */
};
static const struct vc1_vlc_code g_fptype[] = {
    {0x0, 3, eSCTypeIFrame}, /* I,I */
    {010, 3, eSCTypePFrame}, /* I,P */
    {0x2, 3, eSCTypePFrame}, /* P,I */
    {0x3, 3, eSCTypePFrame}, /* P,P */
    {0x4, 3, eSCTypeBFrame}, /* B,B */
    {0x5, 3, eSCTypeBFrame}, /* B,BI */
    {0x6, 3, eSCTypeBFrame}, /* BI,B */
    {0x7, 3, eSCTypeBFrame}, /* BI,BI */
    {0,0,0} /* terminal */
};

static const unsigned char g_mask[] = {0,1,3,7,0xf};

static int BNAV_Indexer_completeVC1Frame(BNAV_Indexer_Handle handle)
{
    return BNAV_Indexer_completeFrameAux(handle, &handle->avcEntry, sizeof(handle->avcEntry));
}

static int b_vc1_vlc_decode(const uint8_t *data, unsigned size, unsigned current_index,
    unsigned current_bit, unsigned *next_index, unsigned *next_bit,const struct vc1_vlc_code *pattern)
{
    int i;
    BSTD_UNUSED(size);
    /* TODO: make it span bytes, if needed. don't think so for vc1. */
    for (i=0;pattern[i].length;i++) {
        unsigned char d = data[0] >> (current_bit - pattern[i].length + 1);
        d &= g_mask[pattern[i].length];
        if (d == pattern[i].bitstring) {
            *next_index = current_index;
            *next_bit = current_bit - pattern[i].length;
            return i;
        }
    }
    return -1;
}

static void BNAV_P_ProcessPES(BNAV_Indexer_Handle handle)
{
    unsigned char *buf = handle->pes.buf;
    unsigned char sc = buf[0];
    const unsigned char *payload = &buf[1];
    unsigned index = 0, bit = 7;
    BNAV_AVC_Entry *entry = &handle->avcEntry;

#define VC1_SC_FRAME 0x0D
#define VC1_SC_ENTRYPOINT 0x0E
#define VC1_SC_SEQUENCE 0x0F

    /* TODO: detect PES start code and capture PTS */

    if (sc != VC1_SC_FRAME && sc != VC1_SC_ENTRYPOINT && sc != VC1_SC_SEQUENCE) {
        return;
    }

    BDBG_MSG(("SC %02x", sc));

    /* complete any pending SEQUENCE or ENTRYPOINT */
    if (handle->pes.sequence_size == -1) {
        handle->pes.sequence_size = handle->pes.offset - handle->pes.sequence_offset;
    }
    else if (handle->pes.entrypoint_size == -1) {
        handle->pes.entrypoint_size = handle->pes.offset - handle->pes.entrypoint_offset;
    }

    /* complete any pending frame */
    handle->picEnd = handle->pes.offset;
    BNAV_Indexer_completeVC1Frame(handle);

    /* coverity[dead_error_condition] */
    switch (sc) {
    case VC1_SC_FRAME:
        {
        int fcm, ptype;

/* TODO: I/P or P/I after EP can be I */
        if (handle->pes.vc1_interlace) {
            fcm = b_vc1_vlc_decode(payload, MIN_PES_PAYLOAD, index, bit, &index, &bit, g_fcm);
            BDBG_MSG(("  fcm %d", fcm));
        }
        else {
            fcm = 0;
        }

        switch (fcm) {
        case 0: /* progressive */
        case 1: /* frame interlace */
            ptype = b_vc1_vlc_decode(payload, MIN_PES_PAYLOAD, index, bit, &index, &bit, g_ptype);
            if (ptype > -1 && ptype < 5) {
                BNAV_set_frameType(entry, g_ptype[ptype].frameType);
            }
            else {
                BDBG_ERR(("  invalid PTYPE %d", ptype));
            }
            break;
        case 2: /* field interlace */
            ptype = b_vc1_vlc_decode(payload, MIN_PES_PAYLOAD, index, bit, &index, &bit, g_fptype);
            if (ptype > -1 && ptype < 8) {
                BNAV_set_frameType(entry, g_fptype[ptype].frameType);
            }
            else {
                BDBG_ERR(("  invalid FPTYPE %d", ptype));
            }
            break;
        default:
            BDBG_ERR(("  invalid FCM %d", fcm));
            return;
        }

        if (BNAV_get_frameType(entry) == eSCTypeIFrame && handle->pes.entrypoint_size && handle->pes.sequence_size) {
            handle->hitFirstISlice = 1;
        }

        handle->picStart = handle->pes.offset;
        BNAV_set_frameOffsetLo(entry, handle->pes.offset & 0xFFFFFFFF);
        BNAV_set_frameOffsetHi(entry, handle->pes.offset >> 32);

        /* Set Sequence offset (using SeqHdr) and Entrypoint offset (using SPS) */
        BNAV_set_seqHdrStartOffset(entry, (unsigned long)(handle->pes.offset - handle->pes.sequence_offset));
        BNAV_set_seqHdrSize(entry, handle->pes.sequence_size);
        BNAV_set_SPS_Offset(entry, (unsigned long)(handle->pes.offset - handle->pes.entrypoint_offset));
        BNAV_set_SPS_Size(entry, handle->pes.entrypoint_size);
        BDBG_MSG(("frame %x, %x, %x, %x, %x",handle->pes.offset,handle->pes.sequence_offset,handle->pes.sequence_size,
            handle->pes.entrypoint_offset,handle->pes.entrypoint_size));
        }
        break;

    case VC1_SC_ENTRYPOINT:
        {
        handle->pes.entrypoint_offset = handle->pes.offset;
        handle->pes.entrypoint_size = -1; /* pending completion */
        }
        break;

    case VC1_SC_SEQUENCE:
        {
        handle->pes.vc1_interlace = payload[5] & 0x40; /* 42nd bit after SC */
        BDBG_MSG(("  interlace? %s", handle->pes.vc1_interlace?"yes":"no"));

        handle->pes.sequence_offset = handle->pes.offset;
        handle->pes.sequence_size = -1; /* pending completion */
        }
        break;
    /* coverity[dead_error_begin] */
    default:
        break;
    }
}

int BNAV_P_FeedPES_VC1(BNAV_Indexer_Handle handle, uint8_t *p_bfr, unsigned size)
{
    unsigned i;

    BDBG_ASSERT(handle->settings.navVersion == BNAV_Version_VC1_PES);

    /* search for start codes */
    for (i=0; i<size; i++, handle->pes.offset++) {
        if (handle->pes.sccount == 3) {
            int required = MIN_PES_PAYLOAD - handle->pes.bufsize;
            int available = size - i;
            int n;

            /* memcpy as much as possible, up to required */
            n = required;
            if (n > available) n = available;
            BKNI_Memcpy((void*)&(handle->pes.buf[handle->pes.bufsize]), &p_bfr[i], n);
            handle->pes.bufsize += n;

            /* check if we have enough to process the header */
            if (handle->pes.bufsize < MIN_PES_PAYLOAD) {
                /* wait for next feed */
                return 0;
            }

            /* process the start code & header */
            handle->pes.offset -= 3; /* back up to start of 00 00 01 SC */
            BNAV_P_ProcessPES(handle);
            handle->pes.offset += 3;
            handle->pes.sccount = 0;
            handle->pes.bufsize = 0; /*: TODO: not actually correct */
        }

        switch (p_bfr[i]) {
        case 0:
            if (handle->pes.sccount >= 1)
                handle->pes.sccount = 2;
            else
                handle->pes.sccount = 1;
            break;
        case 1:
            if (handle->pes.sccount == 2) {
                /* we've got a start code! */
                handle->pes.sccount = 3;
            }
            break;
        default:
            handle->pes.sccount = 0;
            break;
        }
    }
    return 0;
}
#endif
