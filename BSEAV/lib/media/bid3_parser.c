/***************************************************************************
 *     Copyright (c) 2009, Broadcom Corporation
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
 * BMedia library, stream probe module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bid3_parser.h"
#include "biobits.h"

BDBG_MODULE(bid3_parser);

uint32_t 
bid3v2_read_integer(batom_cursor *cursor)
{
    uint32_t data;
    data = batom_cursor_uint32_be(cursor);
    BDBG_MSG(("bid3v2_read_integer: %#x", (unsigned)data));
    return  ((data&0x7F000000)>>3)|((data&0x7F0000)>>2)|((data&0x7F00)>>1)|(data&0x7F);
}

bool
bid3v2_parse_header(batom_cursor *cursor, bid3v2_header *header)
{
    uint8_t flags;
    uint32_t id = batom_cursor_uint24_be(cursor);

    if( id != ((((uint32_t)'I')<<16) | (((uint32_t)'D')<<8) | '3')) {
        return false;
    }
    header->version = batom_cursor_uint16_le(cursor);
    flags = batom_cursor_byte(cursor);
    header->flags.unsynchronisation = B_GET_BIT(flags, 7);
    header->flags.extended_header = B_GET_BIT(flags, 6);
    header->flags.experimental_indicator = B_GET_BIT(flags, 5);
    header->flags.footer_present = B_GET_BIT(flags, 4);
    header->size = bid3v2_read_integer(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    BDBG_MSG(("bid3v2_parse_header: version:%#x size:%u %s %s %s %s", (unsigned)header->version, (unsigned)header->size, header->flags.unsynchronisation?"unsynchronisation":"", header->flags.extended_header?"extended_header":"", header->flags.experimental_indicator?"experimental_indicator":"", header->flags.footer_present?"footer_present":""));
    return true;
}

bool
bid3v2_parse_extended_header(batom_cursor *cursor, bid3v2_extended_header *header)
{
    uint32_t size = bid3v2_read_integer(cursor);
    header->unused = 0;
    if(!BATOM_IS_EOF(cursor) && size>4) {
        BDBG_MSG(("bid3v2_extended_header: size:%u", (unsigned) size));
        batom_cursor_skip(cursor, size-4);
        return !BATOM_IS_EOF(cursor);
    }
    return false;
}

bool
bid3v2_parse_frame(batom_cursor *cursor, const bid3v2_header *header, bid3v2_frame *frame)
{
    uint16_t flags;


    frame->id = batom_cursor_uint32_be(cursor);
	if(header->version >=4) {
    	frame->size = bid3v2_read_integer(cursor);
	} else {
    	frame->size = batom_cursor_uint32_be(cursor);
	}
    flags = batom_cursor_uint16_be(cursor);
    frame->flags.tag_alter_preservation = B_GET_BIT(flags, 6+8);
    frame->flags.file_alter_preservation = B_GET_BIT(flags, 5+8);
    frame->flags.read_only = B_GET_BIT(flags, 4+8);
    
    frame->flags.grouping_identity = B_GET_BIT(flags, 6);
    frame->flags.compression = B_GET_BIT(flags, 3);
    frame->flags.encryption = B_GET_BIT(flags, 2);
    frame->flags.unsynchronisation = B_GET_BIT(flags, 1);
    frame->flags.data_length_indicator = B_GET_BIT(flags, 0);
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    BDBG_MSG(("bid3v2_parse_frame: frame:'%c%c%c%c' size:%u %s %s %s %s %s %s %s %s",  (((frame->id)>>24)&0xFF), (((frame->id)>>16)&0xFF), (((frame->id)>>8)&0xFF), ((frame->id)&0xFF), (unsigned)frame->size, frame->flags.tag_alter_preservation?"tag_alter_preservation":"", frame->flags.file_alter_preservation?"file_alter_preservation":"", frame->flags.read_only?"read_only":"", frame->flags.grouping_identity?"grouping_identity":"", frame->flags.compression?"compression":"", frame->flags.encryption?"encryption":"", frame->flags.unsynchronisation?"unsynchronisation":"", frame->flags.data_length_indicator?"data_length_indicator":""));
    return true;
}

bool
bid3v2_parse_frame_flags(batom_cursor *cursor, const bid3v2_header *header, bid3v2_frame *frame)
{
    BSTD_UNUSED(header);

    if(frame->flags.grouping_identity) {
        frame->data.grouping_identity = batom_cursor_byte(cursor);
    }
    if(frame->flags.encryption) {
        frame->data.encryption_method = batom_cursor_byte(cursor);
    }
    if(frame->flags.data_length_indicator) {
        frame->data.data_length = bid3v2_read_integer(cursor);
    }

    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    return true;
}

bool 
bid3v2_parser_probe(batom_cursor *header)
{
    uint32_t id;
    uint16_t version;

    BDBG_MSG(("bid3v2_parser_probe:%#lx", (unsigned long)header));

    /* http://www.id3.org/id3v2.4.0-structure */
    id = batom_cursor_uint24_be(header);
    if( id != ((((uint32_t)'I')<<16) | (((uint32_t)'D')<<8) | '3')) {
        return false;
    }
    version = batom_cursor_uint16_le(header);
    if(BATOM_IS_EOF(header)) {
        return false;
    }
    if( version > 4) {
        return false;
    }
    batom_cursor_skip(header, 1);
    id = batom_cursor_uint32_be(header);
    if(BATOM_IS_EOF(header)) {
        return false;
    }
    if((id&0x80808080)!=0) {
        return false;
    }

    return true;
}



