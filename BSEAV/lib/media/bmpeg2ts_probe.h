/***************************************************************************
 *     Copyright (c) 2007-2010, Broadcom Corporation
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
 * MPEG-2 TS Parser/Demux library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef __BMPEG2TS_PROBE_H__
#define __BMPEG2TS_PROBE_H__


#include "bmedia_probe_impl.h"
#ifdef __cplusplus
extern "C"
{
#endif

bool bmpeg2ts_probe_header_match(batom_cursor *cursor);
bool bmpeg2ts192_probe_header_match(batom_cursor *cursor);

extern const bmedia_probe_file_ext bmpeg2ts_probe_ext[];
extern const bmedia_probe_file_ext bmpeg2ts192_probe_ext[];
extern const bmedia_probe_format_desc bmpeg2ts_probe;
extern const bmedia_probe_format_desc bmpeg2ts192_probe;

typedef struct bmpeg2ts_probe_stream {
    bmedia_probe_stream media;
    unsigned pkt_len;
} bmpeg2ts_probe_stream;

typedef struct bmpeg2ts_probe_track {
    bmedia_probe_track media;
    size_t parsed_payload; /* amount of payload (in bytes) encountered when parsing stream */
} bmpeg2ts_probe_track;

bmedia_timestamp_parser_t bmpeg2ts_pcr_parser_create(uint16_t pid, size_t packet_len);

#ifdef __cplusplus
}
#endif


#endif  /* __BMPEG2TS_PROBE_H__ */

