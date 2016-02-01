/***************************************************************************
 *     Copyright (c) 2011 Broadcom Corporation
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
 * FLAC parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#ifndef _BFLAC_UTIL_H__
#define _BFLAC_UTIL_H__

#include "bioatom.h"
#include "bmedia_util.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BFLAC_HEADER_TAG BMEDIA_FOURCC('f','L','a','C')

#define BFLAC_BLOCK_HEADER_STREAMINFO   0
#define BFLAC_BLOCK_HEADER_PADDING      1
#define BFLAC_BLOCK_HEADER_APPLICATION  2
#define BFLAC_BLOCK_HEADER_SEEKTABLE    3
#define BFLAC_BLOCK_HEADER_VORBIS_COMMENT 4
#define BFLAC_BLOCK_HEADER_CUESHEET     5
#define BFLAC_BLOCK_HEADER_PICTURE      6
#define BFLAC_BLOCK_HEADER_INVALID      127

#define BFLAC_BLOCK_HEADER_LENGTH      (1+3)
typedef struct bflac_metadata_block_header {
    bool last;
    uint8_t type;
    size_t length;
} bflac_metadata_block_header;

bool bflac_parse_metadata_block_header(batom_cursor *cursor, bflac_metadata_block_header *header);

typedef struct bflac_streaminfo {
    uint16_t blocksize_min;
    uint16_t blocksize_max;
    unsigned framesize_min;
    unsigned framesize_max;
    unsigned sample_rate;
    uint8_t channel_count;
    uint8_t sample_size;
    uint64_t total_samples;
} bflac_streaminfo;

bool bflac_parse_streaminfo(batom_cursor *cursor, bflac_streaminfo *streaminfo);
typedef struct bflac_frame_header {
    bool variable_blocksize;
    uint8_t channel_count;
    uint8_t sample_size;
    uint16_t samples;
    unsigned sample_rate;
} bflac_frame_header;

#define BFLAC_ERROR_EOS (-1)
/* no sync */
#define BFLAC_ERROR_BAD_SYNC (-2)
/* CRC8 not matched */
#define BFLAC_ERROR_BAD_CRC (-3)
/* invalid syntax */
#define BFLAC_ERROR_BAD_DATA (-4)

/* this function returns one of errors above or length of header on success. On success cursor advanced past end of the frame */
int bflac_parse_frame_header(const batom_cursor *cursor, bflac_frame_header *frame, const bflac_streaminfo *streaminfo);
/* this function finds end of the frame, it returns true if end of frame is found */
bool bflac_parser_find_frame_end(batom_cursor *cursor, size_t min_size, size_t header_size);

#define BFLAC_FRAME_SYNC    (0x3FFE)

#ifdef __cplusplus
}
#endif

#endif /* _BFLAC_UTIL_H__ */

