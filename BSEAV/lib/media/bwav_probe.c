/***************************************************************************
 * Copyright (C) 2009-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * BMedia library, wav stream probe module
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bwav_probe.h"
#include "bkni.h"
#include "briff_parser.h"

BDBG_MODULE(bwav_probe);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */

typedef struct bwav_probe  *bwav_probe_t; 

typedef struct b_wav_probe_object_handler {
	briff_parser_handler handler; /* must be first */
	bwav_probe_t probe; /* pointer to the probe */ 
} b_wav_probe_object_handler;

struct bwav_probe {
    BDBG_OBJECT(bwav_probe_t)
    briff_parser_t riff_parser;
    bmedia_probe_track *track;
    bmedia_probe_stream *stream;
    b_wav_probe_object_handler fmt_handler;
    b_wav_probe_object_handler data_handler;
};

BDBG_OBJECT_ID(bwav_probe_t);

static const bmedia_probe_file_ext b_wav_ext[] =  {
    {"wav"},
    {""}
};

static bool
b_wav_probe_header_match(batom_cursor *header)
{
	briff_chunk_id riff;
	briff_chunk_id wav;
	
	riff = batom_cursor_uint32_le(header);
	batom_cursor_skip(header, sizeof(briff_atom) /* size */);
	wav = batom_cursor_uint32_le(header);
	if(BATOM_IS_EOF(header)) {
		return false;
	}
	return riff==B_RIFF_RIFF && BMEDIA_FOURCC_WAVE(wav);
}

static briff_parser_action 
b_wav_probe_file_type(void *context, briff_chunk_id file_type)
{
    bwav_probe_t    probe  = context;
    BSTD_UNUSED(probe);
    BDBG_OBJECT_ASSERT(probe, bwav_probe_t);

    return BMEDIA_FOURCC_WAVE(file_type) ? briff_parser_action_none : briff_parser_action_return;
}



static briff_parser_action 
b_wav_probe_fmt(briff_parser_handler *handler, briff_chunk_id chunk_id, unsigned object_size, unsigned object_offset, batom_t object )
{
    bwav_probe_t    probe = ((b_wav_probe_object_handler *)handler)->probe;
    briff_parser_action  action = briff_parser_action_return;

    BDBG_OBJECT_ASSERT(probe, bwav_probe_t);
    BDBG_MSG(("b_wav_probe_fmt:%p probe " BMEDIA_FOURCC_FORMAT " %u:%u:%#lx(%u)",  (void *)probe, BMEDIA_FOURCC_ARG(chunk_id), object_size, object_offset, (unsigned long)object, object?(unsigned)batom_len(object):0));
    if(probe->track) {
        /* duplicated FMT record */
        goto done;
    }
    if(chunk_id==BMEDIA_FOURCC('f','m','t',' ') && object && object_offset==0 && object_size==batom_len(object)) {
	    bmedia_waveformatex fmt;
        batom_cursor cursor;
		bmedia_probe_track *track;

        batom_cursor_from_atom(&cursor, object);
        if(!bmedia_read_waveformatex(&fmt, &cursor)) {
            goto done;
        }
		track = BKNI_Malloc(sizeof(*track));
        if(!track) {
            goto done;
        }
	    bmedia_probe_track_init(track);
        track->type = bmedia_track_type_audio;

        if(BMEDIA_WAVFMTEX_AUDIO_PCM(&fmt) || fmt.wFormatTag==BMEDIA_WAVFMTEX_AUDIO_PCM_BE_TAG) {
            track->info.audio.codec = baudio_format_pcm;
        } else if(BMEDIA_WAVFMTEX_AUDIO_G711(&fmt)) {
            track->info.audio.codec = baudio_format_g711;
        } else if(BMEDIA_WAVFMTEX_AUDIO_ADPCM(&fmt)) {
            track->info.audio.codec = baudio_format_adpcm;
        } else if(BMEDIA_WAVFMTEX_AUDIO_MPEG(&fmt)) {
            track->info.audio.codec = baudio_format_mpeg;
        } else if(BMEDIA_WAVFMTEX_AUDIO_MP3(&fmt)) {
            track->info.audio.codec = baudio_format_mp3;
        } else if(BMEDIA_WAVFMTEX_AUDIO_AC3(&fmt)) {
            track->info.audio.codec = baudio_format_ac3;
        } else if(BMEDIA_WAVFMTEX_AUDIO_DTS(&fmt)) {
            track->info.audio.codec = baudio_format_dts;
        } else {
            track->info.audio.codec = baudio_format_unknown;
        }
        track->number = 1;
        track->info.audio.channel_count = fmt.nChannels;
	    track->info.audio.sample_size = fmt.wBitsPerSample;
	    track->info.audio.sample_rate = fmt.nSamplesPerSec;
        track->info.audio.bitrate = (fmt.nAvgBytesPerSec*8)/1000;
        probe->track = track;
        action = briff_parser_action_none;
    }
done:
    if(object) {
        batom_release(object);
    }
    return action;
}

static briff_parser_action 
b_wav_probe_data(briff_parser_handler *handler, briff_chunk_id chunk_id, unsigned object_size, unsigned object_offset, batom_t object )
{
    bwav_probe_t probe = ((b_wav_probe_object_handler *)handler)->probe;
    briff_parser_action  action = briff_parser_action_return;
	bmedia_probe_stream *stream;

    BSTD_UNUSED(chunk_id);
    BSTD_UNUSED(object_offset);
    BDBG_OBJECT_ASSERT(probe, bwav_probe_t);
    BDBG_MSG(("b_wav_probe_data:%p probe " BMEDIA_FOURCC_FORMAT " %u:%u:%#lx(%u)",  (void *)probe, BMEDIA_FOURCC_ARG(chunk_id), object_size, object_offset, (unsigned long)object, object?(unsigned)batom_len(object):0));
    if(!probe->track) {
        /* no FMT record */
        goto done;
    }
	stream = BKNI_Malloc(sizeof(*stream));
    if(!stream) {
        goto done;
    }
    bmedia_probe_stream_init(stream, bstream_mpeg_type_wav);
    if(probe->track && probe->track->info.audio.bitrate>0) {
        stream->index = bmedia_probe_index_self;
        stream->max_bitrate = probe->track->info.audio.bitrate*1000;
        stream->duration = (object_size*(uint64_t)8)/probe->track->info.audio.bitrate;

        /* Need to verify it is not a DTS-CD track */
        if (probe->track->info.audio.codec == baudio_format_pcm){
            batom_cursor cursor;
            batom_cursor_from_atom(&cursor, object);
            if (bmedia_is_dts_cd(&cursor)) {
                probe->track->info.audio.channel_count = 6;
                probe->track->info.audio.codec = baudio_format_dts_cd;
                stream->type = bstream_mpeg_type_es;
            }
        }
    }
    bmedia_probe_add_track(stream, probe->track); 
    probe->track = NULL;
    probe->stream = stream;

done:
    if(object) {
        batom_release(object);
    }
    return action;
}


static bmedia_probe_base_t 
b_wav_probe_create(batom_factory_t factory)
{
    bwav_probe_t    probe;
    briff_parser_cfg riff_cfg;

    BDBG_MSG_TRACE(("b_wav_probe_create"));

    probe = BKNI_Malloc(sizeof(*probe));
    if(!probe) {
        BDBG_ERR(("b_wav_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(probe, bwav_probe_t);
    probe->track = NULL;
    probe->stream = NULL;
    briff_parser_default_cfg(&riff_cfg);
    riff_cfg.user_cntx = probe;
    riff_cfg.file_type = b_wav_probe_file_type;
	probe->riff_parser = briff_parser_create(factory, &riff_cfg);
	if(!probe->riff_parser) {
		goto err_create_wav_probe;
	}
	probe->fmt_handler.probe = probe;
	probe->data_handler.probe = probe;
	briff_parser_install_handler(probe->riff_parser, &probe->fmt_handler.handler, BMEDIA_FOURCC('f','m','t',' '), b_wav_probe_fmt); 
	briff_parser_install_handler(probe->riff_parser, &probe->data_handler.handler, BMEDIA_FOURCC('d','a','t','a'), b_wav_probe_data); 

    return (bmedia_probe_base_t)probe;

err_create_wav_probe:
    BKNI_Free(probe);
err_alloc:
    return NULL;
}

static void 
b_wav_probe_destroy(bmedia_probe_base_t probe_)
{
    bwav_probe_t probe = (bwav_probe_t)probe_;
    BDBG_OBJECT_ASSERT(probe, bwav_probe_t);

	briff_parser_remove_handler(probe->riff_parser, &probe->fmt_handler.handler);
	briff_parser_remove_handler(probe->riff_parser, &probe->data_handler.handler);

    briff_parser_destroy(probe->riff_parser);
    BDBG_OBJECT_DESTROY(probe, bwav_probe_t);
    BKNI_Free(probe);
    return;
}


static const bmedia_probe_stream *
b_wav_probe_parse(bmedia_probe_base_t probe_, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config)
{
    bwav_probe_t probe = (bwav_probe_t)probe_;
    size_t read_len = BMEDIA_PROBE_FEED_SIZE;
    bfile_buffer_result result;
    bmedia_probe_stream *stream=NULL;
    batom_t atom;


    BDBG_OBJECT_ASSERT(probe, bwav_probe_t);

    BDBG_MSG_TRACE(("b_wav_probe_parse:%#lx", (unsigned long)probe));

    if (config->min_parse_request != 0) {
        read_len = config->min_parse_request;
    }
    
    atom = bfile_buffer_read(buf, config->parse_offset+0, read_len, &result);
    if(!atom) {
        goto error;
    }
    batom_pipe_push(pipe, atom);
    briff_parser_feed(probe->riff_parser, pipe);
    stream = probe->stream;
    if(probe->track) {
        BKNI_Free(probe->track);
        probe->track = NULL;
    }

error:
    briff_parser_reset(probe->riff_parser);
    batom_pipe_flush(pipe);
    return stream;
}

const bmedia_probe_format_desc bwav_probe = {
    bstream_mpeg_type_es,
    b_wav_ext, /* ext_list */
	/* RIFF 			            size				WAV */
	sizeof(briff_chunk_id)+sizeof(briff_atom)+sizeof(briff_chunk_id), /* header_size */
    b_wav_probe_header_match, /* header_match */
    b_wav_probe_create, /* create */
    b_wav_probe_destroy, /* destroy */
    b_wav_probe_parse, /* parse */
    bmedia_probe_basic_stream_free /* stream free */
};



