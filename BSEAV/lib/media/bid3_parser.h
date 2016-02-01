/***************************************************************************
 *     Copyright (c) 2008-2009, Broadcom Corporation
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
#ifndef _BID3_PARSER_H__
#define _BID3_PARSER_H__

#include "bioatom.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bid3v2_header {
    uint16_t version;
    uint32_t size;
    struct {
        bool unsynchronisation;
        bool extended_header;
        bool experimental_indicator;
        bool footer_present;
    } flags;
} bid3v2_header;

typedef struct bid3v2_extended_header {
    unsigned unused;
} bid3v2_extended_header;

typedef struct bid3v2_frame {
    uint32_t id;
    uint32_t size;
    struct {
        bool tag_alter_preservation;
        bool file_alter_preservation;
        bool read_only;
        bool grouping_identity;
        bool compression;
        bool encryption;
        bool unsynchronisation;
        bool data_length_indicator;
    } flags;
    struct {
        uint8_t     grouping_identity;
        uint8_t     encryption_method;
        uint32_t    data_length;
    } data;
} bid3v2_frame;


uint32_t bid3v2_read_integer(batom_cursor *cursor);
bool bid3v2_parse_header(batom_cursor *cursor, bid3v2_header *header);
bool bid3v2_parse_extended_header(batom_cursor *cursor, bid3v2_extended_header *header);
bool bid3v2_parse_frame(batom_cursor *cursor, const bid3v2_header *header, bid3v2_frame *frame);
bool bid3v2_parse_frame_flags(batom_cursor *cursor, const bid3v2_header *header, bid3v2_frame *frame);
bool bid3v2_parser_probe(batom_cursor *header);


#ifdef __cplusplus
}
#endif


#endif /* _BID3_PARSER_H__ */

