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
#include "bogg_probe.h"
#include "bogg_parser.h"
#include "bkni.h"
#include "biobits.h"
#include "bfile_cache.h"


BDBG_MODULE(bogg_probe);

typedef struct bogg_probe  *bogg_probe_t; 

struct bogg_probe {
    BDBG_OBJECT(bogg_probe)
    batom_factory_t factory;
};

BDBG_OBJECT_ID(bogg_probe);

static bmedia_probe_base_t 
b_ogg_probe_create(batom_factory_t factory)
{
    bogg_probe_t    probe;

    probe = BKNI_Malloc(sizeof(*probe));
    if(!probe) {
        BDBG_ERR(("b_ogg_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(probe, bogg_probe);
    probe->factory = factory;
    return (bmedia_probe_base_t)probe;
err_alloc:
    return NULL;
}

static void 
b_ogg_probe_destroy(bmedia_probe_base_t probe_)
{
    bogg_probe_t probe = (bogg_probe_t) probe_;

    BDBG_OBJECT_ASSERT(probe, bogg_probe);
    BDBG_OBJECT_DESTROY(probe, bogg_probe);
    BKNI_Free(probe);
    return;
}


static const bmedia_probe_stream *
b_ogg_probe_parse(bmedia_probe_base_t probe_, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config)
{
    bogg_probe_t probe = (bogg_probe_t)probe_;
    int rc;
    bogg_probe_stream *stream;
    bogg_probe_track *track;
    baudio_format audio_codec = baudio_format_unknown;
    union {
        struct {
            bvorbis_frame_header frame_header;
            bvorbis_indentification_header indentification_header;
        } vorbis;
        struct {
            bopus_indentification_header indentification_header;
        } opus;
    } codec_data;
    batom_cursor payload;
    batom_cursor header;
    bogg_file_header_state header_parser;
    bfile_segment segment;

    BSTD_UNUSED(config);
    BSTD_UNUSED(pipe);

    BDBG_OBJECT_ASSERT(probe, bogg_probe);
    bfile_segment_set(&segment, 0, 1*1024*1024);
    rc = bogg_file_header_read(&header_parser, probe->factory, buf, &segment);
    if(rc<0) {goto err_header;}

    if(header_parser.header.header_type!=BOGG_HEADER_TYPE_FIRST_PAGE || header_parser.header.page_segments!=1 ||  header_parser.spanning) { goto err_header;}

    stream = BKNI_Malloc(sizeof(*stream));
    if(!stream) { goto err_stream; }
    bmedia_probe_stream_init(&stream->media, bstream_mpeg_type_ogg);

    BATOM_CLONE(&payload, &header_parser.segment.cursor);
    if(batom_cursor_skip(&header_parser.segment.cursor, header_parser.payload_size)!=(size_t)header_parser.payload_size) {goto err_stream_no_data;}

    BATOM_CLONE(&header, &payload);
    if(bvorbis_parse_frame_header(&payload, &codec_data.vorbis.frame_header) && codec_data.vorbis.frame_header.packet_type==BVORBIS_IDENTIFICATION_HEADER) {
        if(!bvorbis_parse_indentification_header(&payload, &codec_data.vorbis.indentification_header)) {goto err_stream_no_data;}
        audio_codec = baudio_format_vorbis;
        goto audio_codec;
    }

    BATOM_CLONE(&payload, &header);
    if(bopus_parse_indentification_header(&payload, &codec_data.opus.indentification_header)) {
        audio_codec = baudio_format_opus;
        goto audio_codec;
    } else {
        goto stream_no_codec;
    }

audio_codec:
    /* return result of parsing */
    track = BKNI_Malloc(sizeof(*track));
    if(!track) { goto err_track_audio; }
    bmedia_probe_track_init(&track->media);
    BLST_SQ_INSERT_TAIL(&stream->media.tracks, &track->media, link);
    track->media.type = bmedia_track_type_audio;
    track->media.number = header_parser.header.bitstream_serial_number+1;
    track->media.info.audio.codec = audio_codec;
    switch(audio_codec) {
    case baudio_format_vorbis:
        track->media.info.audio.channel_count = codec_data.vorbis.indentification_header.audio_channels;
        track->media.info.audio.sample_size = 16;
        track->media.info.audio.sample_rate = codec_data.vorbis.indentification_header.audio_sample_rate;
        track->media.info.audio.bitrate = codec_data.vorbis.indentification_header.bitrate_nominal/1000;
        break;
    case baudio_format_opus:
        track->media.info.audio.sample_size = 16;
        track->media.info.audio.channel_count = codec_data.opus.indentification_header.channelCount;
        track->media.info.audio.sample_rate = codec_data.opus.indentification_header.inputSampleRate;
        break;
        /* coverity[dead_error_begin] */
    default:
        break;
    }

stream_no_codec:

    bogg_file_header_shutdown(&header_parser);

    return &stream->media;

err_track_audio:
err_stream_no_data:
    BKNI_Free(stream);
err_stream:
    bogg_file_header_shutdown(&header_parser);
err_header:
    return NULL;
}

static bool 
b_ogg_probe_header_match(batom_cursor *cursor)
{
    bogg_page_header header;

    if(!bogg_parse_page_header(cursor, &header)) {
        return false;
    }
    if(header.header_type!=BOGG_HEADER_TYPE_FIRST_PAGE) {
        return false;
    }
    return true;
}

static const bmedia_probe_file_ext b_ogg_ext[] =  {
    {"ogg"},
    {""}
};

const bmedia_probe_format_desc bogg_probe = {
    bstream_mpeg_type_ogg,
    b_ogg_ext, /* ext_list */
    BOGG_PAGE_MAX_HEADER_LENGTH,
    b_ogg_probe_header_match, /* header_match */
    b_ogg_probe_create, /* create */
    b_ogg_probe_destroy, /* destroy */
    b_ogg_probe_parse, /* parse */
    bmedia_probe_basic_stream_free /* stream free */
};


int
bogg_file_header_read(bogg_file_header_state *state, batom_factory_t factory, bfile_buffer_t buf, const bfile_segment *segment)
{
    int rc;

    state->segment_valid = false;
    rc = bfile_cached_segment_init(&state->segment, buf, factory, 4);
    if(rc<0) {goto err_segment;}
    bfile_cached_segment_set(&state->segment, segment->start, segment->len);
    if(!bfile_cached_segment_reserve(&state->segment, BOGG_PAGE_MAX_HEADER_LENGTH)) { goto err_no_data; }
    state->segment_valid = true;
    if(!bogg_parse_page_header(&state->segment.cursor, &state->header)) { goto err_no_data;}
    if(!bfile_cached_segment_reserve(&state->segment, 16)) { goto err_no_data; }
    if(!bogg_page_payload_parser_init(&state->parser, &state->segment.cursor, &state->header)) {goto err_no_data;}
    state->payload_size = bogg_page_payload_parser_next(&state->parser, &state->header, &state->spanning);
    BDBG_MSG(("bogg_file_header_read:%p first_packet:%d", (void *)state, state->payload_size));
    if(state->payload_size<=0) { goto err_no_data;}
    if(!bfile_cached_segment_reserve(&state->segment, state->payload_size)) { goto err_no_data; }

    return 0;

err_no_data:
    bfile_cached_segment_shutdown(&state->segment);
    state->segment_valid = false;
err_segment:
    return -1;
}

void
bogg_file_header_shutdown(bogg_file_header_state *state)
{
    if(state->segment_valid) {
        state->segment_valid = false;
        bfile_cached_segment_shutdown(&state->segment);
    }
    return;
}

