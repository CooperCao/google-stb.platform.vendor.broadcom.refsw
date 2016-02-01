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
 * BMedia library, stream probe module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMEDIA_PROBE_IMPL_H__
#define _BMEDIA_PROBE_IMPL_H__

#include "bmedia_probe.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BMEDIA_PROBE_FEED_SIZE	(BIO_BLOCK_SIZE*2)

typedef struct bmedia_probe_parser_config {
    off_t parse_offset;
    bool probe_index; /* allows probing for presence of the index data, usually it causes the media probe to read data toward end of the file */
    bool parse_index; /* allows parsing of the index data, The entire contents of the index table may be read (perhaps more than once) and analyzed to provide more details about the layout of the file */
    bmedia_probe_stream_specific_cfg stream_specific;
    off_t min_parse_request; /* Minimum amount of data to parse to find all tracks */
} bmedia_probe_parser_config;


typedef struct bmedia_probe_format *bmedia_probe_base_t;
typedef char bmedia_probe_file_ext[6];
typedef struct bmedia_probe_format_desc {
	bstream_mpeg_type type;
	const bmedia_probe_file_ext *ext_list; /* list of extensions */
	size_t header_size;
	bool (*header_match)(batom_cursor *header);
	bmedia_probe_base_t (*create)(batom_factory_t factory);
	void (*destroy)(bmedia_probe_base_t probe);
	const bmedia_probe_stream *(*parse)(bmedia_probe_base_t probe, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config);
	void (*stream_free)(bmedia_probe_base_t probe, bmedia_probe_stream *stream);
} bmedia_probe_format_desc;
	
void bmedia_probe_stream_init(bmedia_probe_stream *stream, bstream_mpeg_type type);
void bmedia_probe_track_init(bmedia_probe_track *track);
void bmedia_probe_basic_stream_free(bmedia_probe_base_t probe, bmedia_probe_stream *stream);
void bmedia_probe_add_track(bmedia_probe_stream *stream, bmedia_probe_track *track);
bool bmedia_probe_match_ext(const bmedia_probe_file_ext *ext_list, const char *ext);
void bmedia_probe_video_init(bmedia_probe_video *video);


typedef struct bmedia_timestamp_parser *bmedia_timestamp_parser_t;

typedef struct bmedia_timestamp {
	uint32_t timestamp;
	off_t offset;
} bmedia_timestamp;

typedef struct bmedia_timestamp_parser_methods {
	void (*seek)(bmedia_timestamp_parser_t parser, off_t offset);
	int (*parse)(bmedia_timestamp_parser_t parser, batom_cursor *cursor, bmedia_timestamp *timestamp);
	void (*destroy)(bmedia_timestamp_parser_t parser);
} bmedia_timestamp_parser_methods;

typedef struct bmedia_timestamp_parser {
	const bmedia_timestamp_parser_methods *methods;
} bmedia_timestamp_parser;





#ifdef __cplusplus
}
#endif


#endif /* _BMEDIA_PROBE_IMPL_H__ */

