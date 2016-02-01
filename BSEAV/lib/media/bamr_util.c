/***************************************************************************
 *     Copyright (c) 2013 Broadcom Corporation
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
 * AMR file parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bamr_util.h"
#include "biobits.h"

BDBG_MODULE(bamr_util);

#define BDBG_MSG_TRACE(x)
baudio_format bamr_parse_header(batom_cursor *cursor)
{
    uint32_t data;
    int byte;

    data = batom_cursor_uint32_le(cursor);
    if(BATOM_IS_EOF(cursor)) { goto error;}
    if(data != BAMR_HEADER_TAG) { goto error;}

    byte = batom_cursor_next(cursor);
    if(byte==BATOM_EOF) {goto error;}
    if(byte!='R') {goto error;}

    byte = batom_cursor_next(cursor);
    if(byte==BATOM_EOF) {goto error;}
    if(byte=='\n') { 
        return baudio_format_amr_nb;
    }
    if(byte!='-') {goto error;}

    data = batom_cursor_uint24_le(cursor);
    if(BATOM_IS_EOF(cursor)) { goto error;}
    if(data==BMEDIA_FOURCC('W','B','\n','\0')) {
        return baudio_format_amr_wb;
    }

error:
    return baudio_format_unknown;
}

static int b_amr_nb_parse_frame_header(batom_cursor *cursor)
{
    /* 3GP TS 26.101 */
    static const uint8_t b_amr_nb_bytes[] = {
        /* 0 */ 12,
        /* 1 */ 13,
        /* 2 */ 15,
        /* 3 */ 17,
        /* 4 */ 19,
        /* 5 */ 20,
        /* 6 */ 26,
        /* 7 */ 31,
        /* 8 */ 5,
        /* 9 */ 0,
        /* 10 */ 0,
        /* 11 */ 0,
        /* 12 */ 0,
        /* 13 */ 0,
        /* 14 */ 0,
        /* 15 */ 0
    };
    unsigned type;
    unsigned payload;
    int data = batom_cursor_next(cursor);

    if(data==BATOM_EOF) {
        return -1;
    }
    /* Figure 1 */
    type = B_GET_BITS(data,6,3);
    if(type>=sizeof(b_amr_nb_bytes)/sizeof(*b_amr_nb_bytes)) {
        return -1;
    }
    payload = b_amr_nb_bytes[type];
    return  payload+1;
}

static int b_amr_wb_parse_frame_header(batom_cursor *cursor)
{
    /* 3GP TS 26.201 */
    /* Table A.1b */
    static const uint8_t b_amr_wb_octets[] = {
        /* 0 */ 18,
        /* 1 */ 23,
        /* 2 */ 33,
        /* 3 */ 37,
        /* 4 */ 41,
        /* 5 */ 47,
        /* 6 */ 51,
        /* 7 */ 59,
        /* 8 */ 61,
        /* 9 */ 6,
        /* 10 */ 0,
        /* 11 */ 0,
        /* 12 */ 0,
        /* 13 */ 0,
        /* 14 */ 1,
        /* 15 */ 1
    };
    unsigned type;
    unsigned payload;
    int data;
    
    data = batom_cursor_next(cursor);

    if(data==BATOM_EOF) {
        return -1;
    }
    /* Figure 1 */
    type = B_GET_BITS(data,7,3);
    if(type>=sizeof(b_amr_wb_octets)/sizeof(*b_amr_wb_octets)) {
        return -1;
    }
    payload = b_amr_wb_octets[type];
    if(payload==0) {
        return -1;
    } 
    return payload;
}

/* this function returns size of the frame, or -1 on error, it needs to know AMR codec type */
int bamr_parse_frame_header(batom_cursor *cursor, baudio_format codec)
{
    if(codec==baudio_format_amr_nb) {
        return b_amr_nb_parse_frame_header(cursor);
    } else if(codec==baudio_format_amr_wb) {
        return b_amr_wb_parse_frame_header(cursor);
    }
    BDBG_WRN(("unsupported codec %u", codec));
    return -1;
}

