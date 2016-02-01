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
#ifndef COMMONTS_H__
#define COMMONTS_H__

#include "bstd.h"
#include "playertypes.h"

#ifdef __cplusplus
extern "C" {
#endif

const char *b_btp_mode_str(int mode);

/* Byteswap LE uint32_t to BE */
#define be(L) \
    ((((L) & 0xFF) << 24) | (((L) & 0xFF00) << 8) | (((L) & 0xFF0000) >> 8) | (((L) & 0xFF000000) >> 24))

unsigned short b_get_pid(
    const unsigned char *pkt    /* 188 byte transport packet */
    );

bool b_is_btp(
    const unsigned char *pkt    /* 188 byte transport packet */
    );

uint32_t b_get_btp_word(
    const unsigned char *pkt,   /* 188 byte transport packet */
    unsigned command_word            /* index 0..9 into BTP command word array */
    );

/* advance the sccount, looking for start codes. when
sccount gets to 3, the next byte is a start code. */
int b_check_for_start_code(unsigned char data, int sccount);

bool b_is_pes_stream_id(unsigned char stream_id);

typedef enum b_pes_packet_type {
    b_pes_packet_type_invalid,
    b_pes_packet_type_data, /* program_stream_map, private_stream_2, ECM, EMM, program_stream_directory,
                             DSMCC_stream, ITU-T Rec. H.222.1 type E stream */
    b_pes_packet_type_pes,
    b_pes_packet_type_padding
} b_pes_packet_type;

b_pes_packet_type b_get_pes_type(unsigned char stream_id);

typedef struct b_pes_header
{
    b_pes_packet_type pes_type;
    unsigned packet_length;
    unsigned header_data_length;
    unsigned pts_dts_flags;
    uint32_t pts;
    uint32_t dts;
} b_pes_header;

int b_get_pes_header(const unsigned char *buf, b_pes_header *pes_header);

void b_set_pes_header(unsigned char *buf, const b_pes_header *pes_header);


#ifdef __cplusplus
}
#endif

#endif /* COMMONTS_H__ */
