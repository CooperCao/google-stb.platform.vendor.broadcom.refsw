/***************************************************************************
 *     Copyright (c) 2011-2013, Broadcom Corporation
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
#include "bflac_probe.h"
#include "bflac_parser.h"
#include "bkni.h"
#include "biobits.h"


BDBG_MODULE(bflac_probe);

typedef struct bflac_probe  *bflac_probe_t; 


typedef struct b_flac_probe_header_handler {
	bflac_parser_handler handler; /* must be first */
	bflac_probe_t probe; /* pointer to probe */ 
} b_flac_probe_header_handler;

struct bflac_probe {
    BDBG_OBJECT(bflac_probe)
    batom_factory_t factory;
	bflac_parser_t parser;
    b_flac_probe_header_handler streaminfo_handler;
    bflac_streaminfo streaminfo;
    bool streaminfo_valid;
};

BDBG_OBJECT_ID(bflac_probe);

static bflac_parser_action
b_flac_probe_streaminfo(bflac_parser_handler *handler, batom_t object)
{
    bflac_probe_t probe;
    batom_cursor cursor;

    BDBG_ASSERT(handler);
    BDBG_ASSERT(object);
    probe = ((b_flac_probe_header_handler *)handler)->probe;
    BDBG_MSG(("b_flac_probe_streaminfo:%p", (void *)probe));
    BDBG_OBJECT_ASSERT(probe, bflac_probe);
    batom_cursor_from_atom(&cursor, object);
    probe->streaminfo_valid = bflac_parse_streaminfo(&cursor, &probe->streaminfo);
    batom_release(object);

    return bflac_parser_action_return;
}

static bmedia_probe_base_t
b_flac_probe_create(batom_factory_t factory)
{
    bflac_probe_t    probe;
    bflac_parser_cfg cfg;

    probe = BKNI_Malloc(sizeof(*probe));
    if(!probe) {
        BDBG_ERR(("b_flac_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(probe, bflac_probe);
    bflac_parser_default_cfg(&cfg);
    probe->parser = bflac_parser_create(factory, &cfg);
    if(!probe->parser) {(void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_parser;}
    probe->factory = factory;
    probe->streaminfo_handler.probe = probe;
    bflac_parser_install_handler(probe->parser, &probe->streaminfo_handler.handler, BFLAC_BLOCK_HEADER_STREAMINFO, b_flac_probe_streaminfo);
    return (bmedia_probe_base_t)probe;

err_parser:
    BKNI_Free(probe);
err_alloc:
    return NULL;
}

static void 
b_flac_probe_destroy(bmedia_probe_base_t probe_)
{
    bflac_probe_t probe = (bflac_probe_t) probe_;

    BDBG_OBJECT_ASSERT(probe, bflac_probe);
    bflac_parser_remove_handler(probe->parser, &probe->streaminfo_handler.handler);
    bflac_parser_destroy(probe->parser);
    BDBG_OBJECT_DESTROY(probe, bflac_probe);
    BKNI_Free(probe);
    return;
}


static const bmedia_probe_stream *
b_flac_probe_parse(bmedia_probe_base_t probe_, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config)
{
    bflac_probe_t probe = (bflac_probe_t)probe_;
	off_t off;
	size_t read_len = BMEDIA_PROBE_FEED_SIZE;

    BDBG_OBJECT_ASSERT(probe, bflac_probe);
    probe->streaminfo_valid = false;

	for(off=0;off<=16*1024;) {
		batom_t atom;
		bfile_buffer_result result;
		size_t feed_len;
		size_t atom_len;

		BDBG_MSG(("b_flac_probe_parse: %p reading %u:%u", (void *)probe, (unsigned)(off+config->parse_offset), (unsigned)read_len));
		atom = bfile_buffer_read(buf, off+config->parse_offset, read_len, &result);
		if(!atom) {
			break;
		}
		atom_len = batom_len(atom);
		BDBG_MSG(("b_flac_probe_parse: %p read %u:%u -> %p", (void *)probe, (unsigned)(off+config->parse_offset), (unsigned)atom_len, (void *)atom));
		off += atom_len;
		batom_pipe_push(pipe, atom);
		feed_len = bflac_parser_feed(probe->parser, pipe);
        if(probe->streaminfo_valid) {
			break;
		}
		if(feed_len!=atom_len) {
			break;
		}
	}
    bflac_parser_reset(probe->parser);
    if(probe->streaminfo_valid) {
        bflac_probe_stream *stream;
        bflac_probe_track *track;
        stream = BKNI_Malloc(sizeof(*stream));
        if(!stream) { (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_stream; }
        bmedia_probe_stream_init(&stream->media, bstream_mpeg_type_flac);
        track = BKNI_Malloc(sizeof(*track));
        if(!track) { (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_track_audio; }
        bmedia_probe_track_init(&track->media);
        track->media.type = bmedia_track_type_audio;
        track->media.number = 1;
        track->media.info.audio.channel_count = probe->streaminfo.channel_count;
        track->media.info.audio.sample_size = probe->streaminfo.sample_size;
        track->media.info.audio.sample_rate = probe->streaminfo.sample_rate;
        track->media.info.audio.codec = baudio_format_flac;
        stream->media.type = bstream_mpeg_type_flac;
        if(probe->streaminfo.sample_rate) {
            stream->media.duration = (probe->streaminfo.total_samples * 1000)/probe->streaminfo.sample_rate;
            if(probe->streaminfo.blocksize_min || probe->streaminfo.blocksize_max) {
                track->media.info.audio.bitrate = probe->streaminfo.sample_rate*((probe->streaminfo.framesize_max + probe->streaminfo.framesize_min) * 8)/(probe->streaminfo.blocksize_max + probe->streaminfo.blocksize_min);
            }
        }
        bmedia_probe_add_track(&stream->media, &track->media);  
        return &stream->media;

    err_track_audio:
        BKNI_Free(stream);
    err_stream:
        ;
    }

    return NULL;
}

static bool 
b_flac_probe_header_match(batom_cursor *cursor)
{
    bflac_metadata_block_header header;

    if(batom_cursor_uint32_le(cursor) != BFLAC_HEADER_TAG) {
        return false;
    }
    if(!bflac_parse_metadata_block_header(cursor, &header)) {
        return false;
    }
    if(header.type != BFLAC_BLOCK_HEADER_STREAMINFO) {
        return false;
    }
    return true;
}

static const bmedia_probe_file_ext b_flac_ext[] =  {
    {"flac"},
    {""}
};

const bmedia_probe_format_desc bflac_probe = {
    bstream_mpeg_type_flac,
    b_flac_ext, /* ext_list */
    4+BFLAC_BLOCK_HEADER_LENGTH,
    b_flac_probe_header_match, /* header_match */
    b_flac_probe_create, /* create */
    b_flac_probe_destroy, /* destroy */
    b_flac_probe_parse, /* parse */
    bmedia_probe_basic_stream_free /* stream free */
};

