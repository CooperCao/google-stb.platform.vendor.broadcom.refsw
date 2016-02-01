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
 * BMedia library, ape stream probe module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bioatom.h"
#include "bmedia_util.h"
#include "bape_util.h"

BDBG_MODULE(bape_util);

#define B_APE_MIN_VERSION 3980
#define B_APE_MAX_VERSION 3990


bool bape_parse_file_descriptor(batom_cursor *cursor, bape_file_descriptor *file_descriptor)
{
    uint32_t signature;

    signature = batom_cursor_uint32_le(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    if(signature!=BAPE_FILE_ID) {
        return false;
    }

    file_descriptor->file_version = batom_cursor_uint16_le(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    if(file_descriptor->file_version < B_APE_MIN_VERSION || file_descriptor->file_version > B_APE_MAX_VERSION) {
        return false;
    }
    batom_cursor_skip(cursor, BMEDIA_FIELD_LEN(padding, uint16_t));
    file_descriptor->descriptor_length = batom_cursor_uint32_le(cursor);
    file_descriptor->header_length = batom_cursor_uint32_le(cursor);
    file_descriptor->seektable_length = batom_cursor_uint32_le(cursor);
    file_descriptor->wavheader_length = batom_cursor_uint32_le(cursor);
    file_descriptor->audiodata_length = batom_cursor_uint64_le(cursor);
    file_descriptor->wavtail_length = batom_cursor_uint32_le(cursor);
    batom_cursor_skip(cursor, 16); /* md5 */
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    BDBG_MSG(("bape_parse_file_descriptor: file_version:%u descriptor_length:%u header_length:%u seektable_length:%u wavheader_length:%u audiodata_length:%u wavtail_length:%u", file_descriptor->file_version, file_descriptor->descriptor_length, file_descriptor->header_length, file_descriptor->seektable_length, file_descriptor->wavheader_length, (unsigned)file_descriptor->audiodata_length, (unsigned)file_descriptor->wavtail_length));
    return true;
}

bool bape_parse_frame_header(batom_cursor *cursor, bape_frame_header *frame_header)
{
    frame_header->compression_type = batom_cursor_uint16_le(cursor);
    frame_header->format_flags = batom_cursor_uint16_le(cursor);
    frame_header->blocks_per_frame = batom_cursor_uint32_le(cursor);
    frame_header->final_frame_blocks = batom_cursor_uint32_le(cursor);
    frame_header->total_frames = batom_cursor_uint32_le(cursor);
    frame_header->bits_per_sample = batom_cursor_uint16_le(cursor);
    frame_header->channels  = batom_cursor_uint16_le(cursor);
    frame_header->sample_rate = batom_cursor_uint32_le(cursor);
    if(BATOM_IS_EOF(cursor) || frame_header->sample_rate==0) {
        return false;
    }
    BDBG_MSG(("bape_parse_frame_header: compression_type:%u format_flags:%#x blocks_per_frame:%u final_frame_blocks:%u total_frames:%u bits_per_sample:%u channels:%u sample_rate:%u", frame_header->compression_type, frame_header->format_flags, frame_header->blocks_per_frame, frame_header->final_frame_blocks, frame_header->total_frames, frame_header->bits_per_sample, frame_header->channels, frame_header->sample_rate));
    return true;
}

