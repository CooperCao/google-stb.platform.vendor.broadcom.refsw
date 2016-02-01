/***************************************************************************
 *     Copyright (c) 2009-2013, Broadcom Corporation
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
 * BMedia library, monkeys audio stream probe module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BAPE_UTIL_H__
#define _BAPE_UTIL_H__


#ifdef __cplusplus
extern "C"
{
#endif

#define BAPE_FILE_ID    BMEDIA_FOURCC('M','A','C',' ')

#define BAPE_FILE_DESCRIPTOR_LENGTH 52
typedef struct bape_file_descriptor {
    uint16_t file_version;
    uint32_t descriptor_length;
    uint32_t header_length;
    uint32_t seektable_length;
    uint32_t wavheader_length;
    uint64_t audiodata_length;
    uint32_t wavtail_length;
} bape_file_descriptor;

#define BAPE_FRAME_HEADER_LENGTH    24
typedef struct bape_frame_header {
    uint16_t compression_type;
    uint16_t format_flags;
    uint32_t blocks_per_frame;
    uint32_t final_frame_blocks;
    uint32_t total_frames;
    uint16_t bits_per_sample;
    uint16_t channels;
    uint32_t sample_rate;
} bape_frame_header;

bool bape_parse_file_descriptor(batom_cursor *cursor, bape_file_descriptor *file_descriptor);
bool bape_parse_frame_header(batom_cursor *cursor, bape_frame_header *frame_header);

#ifdef __cplusplus
}
#endif


#endif /* _BAPE_UTIL_H__ */

