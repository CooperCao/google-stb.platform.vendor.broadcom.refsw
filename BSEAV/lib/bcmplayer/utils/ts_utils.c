/***************************************************************************
 *     Copyright (c) 1998-2012, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************/
#include "ts_utils.h"
#include <string.h>
#include <stdio.h>

const char *b_btp_mode_str(int mode)
{
    switch (mode) {
    case TT_MODE_PLAY: return "PLAY";
    case TT_MODE_DISPLAY: return "DISPLAY";
    case TT_MODE_BUILD_REFERENCE: return "BUILD REF";
    case TT_MODE_PROCESS: return "PROCESS";
    case TT_MODE_DISCARD_TILL_BTP: return "DISCARD TILL BTP";
    case TT_MODE_DISPLAY_FORWARD: return "DISPLAY FWD";
    case TT_MODE_DISPLAY_REWIND: return "DISPLAY REV";
    case TT_MODE_DISPLAY_PAST_BUFFER: return "DISPLAY PAST BUFFER";
    case TT_MODE_TIMING_MARKER: return "TIMING MARKER";
    case TT_MODE_PICTURE_TAG: return "PICTURE TAG";
    default: return "Unknown";
    }
}

unsigned short b_get_pid(const unsigned char *pkt)
{
    return (((unsigned short)pkt[1]&0x1f) << 8) | pkt[2];
}

bool b_is_btp(const unsigned char *pkt)
{
    return !memcmp(&pkt[8], "BRCM", 4);
}

uint32_t b_get_btp_word(const unsigned char *pkt, unsigned n)
{
    uint32_t *pktdata = (uint32_t *)&pkt[12];
    return be(pktdata[n]);
}

int b_check_for_start_code(unsigned char data, int sccount)
{
    switch (data) {
    case 0:
        if (sccount >= 1)
            sccount = 2;
        else
            sccount = 1;
        break;
    case 1:
        if (sccount == 2) {
            /* we've got a start code! */
            sccount = 3;
        }
        else {
            sccount = 0;
        }
        break;
    default:
        sccount = 0;
        break;
    }
    return sccount;
}

bool b_is_pes_stream_id(unsigned char stream_id) {
    return
        ((stream_id & 0xFC) == 0xBC) || /* 1011 11xx */
        ((stream_id & 0xC0) == 0xC0) || /* 110x xxxx */
        ((stream_id & 0xF0) == 0xE0) || /* 1110 xxxx */
        ((stream_id & 0xF0) == 0xF0);   /* 1111 xxxx */
}

#define CHECK(X) do {if (!(X)) printf("### invalid PES header (ts_utils.c:%d)\n", __LINE__); } while (0)

static int b_get_pts(const unsigned char *buf, int checkval, uint32_t *p_pts)
{
    uint64_t pts;
    /* check key value and marker bits */
    if ((buf[0] >> 4 != checkval) || !(buf[0] & 0x01) || !(buf[2] & 0x01) || !(buf[4] & 0x01)) {
        CHECK(-1);
        return -1;
    }

    /* read 33 bits of pts info */
    pts = (buf[0] & 0x0E) >> 1; /* 3 bits */
    pts <<= 8;
    pts |= buf[1]; /* 8 bits */
    pts <<= 7;
    pts |= (buf[2] >> 1); /* 7 bits */
    pts <<= 8;
    pts |= buf[3]; /* 8 bits */
    pts <<= 7;
    pts |= (buf[4] >> 1); /* 7 bits */

    /* downshift by one. we throw away the LSB. */
    pts >>= 1;

    *p_pts = (uint32_t)pts;
    return 0;
}

static void b_set_pts(unsigned char *buf, unsigned pts_dts_flags, uint32_t pts)
{
    uint64_t full_pts = pts << 1; /* upshift by one so that bitshift math matches spec */

    buf[0] = (((full_pts >> 30) & 0x7)  << 1) | 0x1 | (pts_dts_flags << 4);
    buf[1] = (((full_pts >> 22) & 0xff));
    buf[2] = (((full_pts >> 15) & 0x7f) << 1) | 0x1;
    buf[3] = (((full_pts >>  7) & 0xff));
    buf[4] = (((full_pts      ) & 0x7f) << 1) | 0x1;
}

/* decode and print the pes header. buf points to the stream_id and is guaranteed
to point to whole pes header. */
int b_get_pes_header(const unsigned char *buf, b_pes_header *pes_header)
{
    int rc;
    memset(pes_header, 0, sizeof(*pes_header));

    pes_header->pes_type = b_get_pes_type(buf[0]);
    pes_header->packet_length = (((unsigned short)buf[1])<<8) + buf[2];

    if (pes_header->pes_type == b_pes_packet_type_pes) {
        pes_header->pts_dts_flags = buf[4] >> 6;
        pes_header->header_data_length = buf[5];
        if ((buf[3] & 0xC0) != 0x80) {
            return -1;
        }

        if (pes_header->pts_dts_flags == 2) {
            rc = b_get_pts(&buf[6], 2, &pes_header->pts);
            if (rc) return -1;
        }
        else if (pes_header->pts_dts_flags == 3) {
            rc = b_get_pts(&buf[6], 3, &pes_header->pts);
            if (rc) return -1;
            rc = b_get_pts(&buf[11], 1, &pes_header->dts);
            if (rc) return -1;
        }
        /* TODO: ESCR flag, ES rate flag, DSM trick mode flag, etc. */
    }
    return 0;
}

void b_set_pes_header(unsigned char *buf, const b_pes_header *pes_header)
{
    if (pes_header->pts_dts_flags == 2) {
        b_set_pts(&buf[6], 2, pes_header->pts);
    }
    else if (pes_header->pts_dts_flags == 3) {
        b_set_pts(&buf[6], 3, pes_header->pts);
        b_set_pts(&buf[11], 1, pes_header->dts);
    }
}

b_pes_packet_type b_get_pes_type(unsigned char stream_id) {
    switch (stream_id) {
    case 0xBE: return b_pes_packet_type_padding;
    case 0xBA: return b_pes_packet_type_data; /* program_stream_map */
    case 0xBB:
    case 0xBC: return b_pes_packet_type_pes; /* private_stream_1, padding_stream */
    case 0xBF: return b_pes_packet_type_data; /* private_stream_2 */
    case 0xF0:           /* ECM */
    case 0xF1: return b_pes_packet_type_data; /* EMM */
    case 0xF2: return b_pes_packet_type_data; /* DSMCC */
    case 0xFF: return b_pes_packet_type_data; /* program_stream_directory */
    default:   break;
    }

    if (stream_id >= 0xF4 && stream_id <= 0xF8) { /* H.222.1 type A-E */
        return b_pes_packet_type_data;
    }
        
    if ((stream_id == 0xBD) ||  /* private_stream_1 */
        ((stream_id & 0xC0) == 0xC0) || /* 110x xxxx - audio stream, Cx & Dx */
        ((stream_id & 0xF0) == 0xE0) || /* 1110 xxxx - video stream, Ex */
        (stream_id == 0xF3) ||              /* ISO 13522 stream */
        (stream_id >= 0xF9 && stream_id <= 0xFE)) /* reserved data stream */
    {
        return b_pes_packet_type_pes;
    }

    return b_pes_packet_type_invalid; /* not PES */
}
