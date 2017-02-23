/***************************************************************************
 * Copyright (C) 2012-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * BMedia library, aiff stream probe module
 * 
 *******************************************************************************/
#include "bstd.h"
#include "baiff_probe.h"
#include "bkni.h"
#include "briff_parser.h"
#include "baiff_util.h"

BDBG_MODULE(baiff_probe);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */

typedef struct baiff_probe  *baiff_probe_t; 

typedef struct b_aiff_probe_object_handler {
    briff_parser_handler handler; /* must be first */
    baiff_probe_t probe; /* pointer to the probe */ 
} b_aiff_probe_object_handler;

struct baiff_probe {
    BDBG_OBJECT(baiff_probe_t)
    briff_parser_t riff_parser;
    bmedia_probe_track *track;
    bmedia_probe_stream *stream;
    b_aiff_probe_object_handler comm_handler;
};

BDBG_OBJECT_ID(baiff_probe_t);

static const bmedia_probe_file_ext b_aiff_ext[] =  {
    {"aif"},
    {""}
};


static bool
b_aiff_probe_header_match(batom_cursor *header)
{
    briff_chunk_id riff;
    briff_chunk_id aiff;
    
    riff = batom_cursor_uint32_le(header);
    batom_cursor_skip(header, sizeof(briff_atom) /* size */);
    aiff = batom_cursor_uint32_le(header);
    if(BATOM_IS_EOF(header)) {
        return false;
    }
    return riff==BAIFF_FORM && BMEDIA_FOURCC_AIFF(aiff);
}

static briff_parser_action 
b_aiff_probe_file_type(void *context, briff_chunk_id file_type)
{
    baiff_probe_t    probe  = context;
    BSTD_UNUSED(probe);
    BDBG_OBJECT_ASSERT(probe, baiff_probe_t);

    return BMEDIA_FOURCC_AIFF(file_type) ? briff_parser_action_none : briff_parser_action_return;
}




static briff_parser_action 
b_aiff_probe_comm(briff_parser_handler *handler, briff_chunk_id chunk_id, unsigned object_size, unsigned object_offset, batom_t object )
{
    baiff_probe_t    probe = ((b_aiff_probe_object_handler *)handler)->probe;
    briff_parser_action  action = briff_parser_action_return;

    BDBG_OBJECT_ASSERT(probe, baiff_probe_t);
    BDBG_MSG(("b_aiff_probe_comm:%p probe " BMEDIA_FOURCC_FORMAT " %u:%u:%#lx(%u)",  (void *)probe, BMEDIA_FOURCC_ARG(chunk_id), object_size, object_offset, (unsigned long)object, object?(unsigned)batom_len(object):0));
    if(probe->track) {
        /* duplicated FMT record */
        goto done;
    }
    if(chunk_id==BAIFF_COMM && object && object_offset==0 && object_size==batom_len(object)) {
        baiff_comm comm;
        batom_cursor cursor;
        bmedia_probe_track *track;
        bmedia_probe_stream *stream;

        batom_cursor_from_atom(&cursor, object);
        if(!baiff_read_comm(&cursor, &comm)) {
            goto done;
        }
        track = BKNI_Malloc(sizeof(*track));
        if(!track) {
            goto done;
        }
        bmedia_probe_track_init(track);
        track->type = bmedia_track_type_audio;

        track->info.audio.codec = baudio_format_pcm;
        track->number = 1;
        track->info.audio.channel_count = comm.numChannels;
        track->info.audio.sample_size = comm.sampleSize;
        track->info.audio.sample_rate = comm.sampleRate;
        track->info.audio.bitrate = 0;
        action = briff_parser_action_none;

        stream = BKNI_Malloc(sizeof(*stream));
        if(!stream) {
            BKNI_Free(track);
            goto done;
        }
        bmedia_probe_stream_init(stream, bstream_mpeg_type_aiff);
        bmedia_probe_add_track(stream, track); 
        probe->stream = stream;
        probe->track = NULL;
        
    }

done:
    if(object) {
        batom_release(object);
    }
    return action;
}


static bmedia_probe_base_t 
b_aiff_probe_create(batom_factory_t factory)
{
    baiff_probe_t    probe;
    briff_parser_cfg riff_cfg;

    BDBG_MSG_TRACE(("b_aiff_probe_create"));

    probe = BKNI_Malloc(sizeof(*probe));
    if(!probe) {
        BDBG_ERR(("b_aiff_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(probe, baiff_probe_t);
    probe->track = NULL;
    probe->stream = NULL;
    briff_parser_default_cfg(&riff_cfg);
    riff_cfg.user_cntx = probe;
    riff_cfg.file_type = b_aiff_probe_file_type;
    riff_cfg.le_endian = false;
    riff_cfg.top_id = BAIFF_FORM;
    probe->riff_parser = briff_parser_create(factory, &riff_cfg);
    if(!probe->riff_parser) {
        goto err_create_aiff_probe;
    }
    probe->comm_handler.probe = probe;
    briff_parser_install_handler(probe->riff_parser, &probe->comm_handler.handler, BAIFF_COMM, b_aiff_probe_comm); 

    return (bmedia_probe_base_t)probe;

err_create_aiff_probe:
    BKNI_Free(probe);
err_alloc:
    return NULL;
}

static void 
b_aiff_probe_destroy(bmedia_probe_base_t probe_)
{
    baiff_probe_t probe = (baiff_probe_t)probe_;
    BDBG_OBJECT_ASSERT(probe, baiff_probe_t);

    briff_parser_remove_handler(probe->riff_parser, &probe->comm_handler.handler);

    briff_parser_destroy(probe->riff_parser);
    BDBG_OBJECT_DESTROY(probe, baiff_probe_t);
    BKNI_Free(probe);
    return;
}


static const bmedia_probe_stream *
b_aiff_probe_parse(bmedia_probe_base_t probe_, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config)
{
    baiff_probe_t probe = (baiff_probe_t)probe_;
    size_t read_len = BMEDIA_PROBE_FEED_SIZE;
    bfile_buffer_result result;
    bmedia_probe_stream *stream=NULL;
    batom_t atom;


    BDBG_OBJECT_ASSERT(probe, baiff_probe_t);

    BDBG_MSG_TRACE(("b_aiff_probe_parse:%#lx", (unsigned long)probe));

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

const bmedia_probe_format_desc baiff_probe = {
    bstream_mpeg_type_es,
    b_aiff_ext, /* ext_list */
    /* FORM     size                AIFF */
    sizeof(briff_chunk_id)+sizeof(briff_atom)+sizeof(briff_chunk_id), /* header_size */
    b_aiff_probe_header_match, /* header_match */
    b_aiff_probe_create, /* create */
    b_aiff_probe_destroy, /* destroy */
    b_aiff_probe_parse, /* parse */
    bmedia_probe_basic_stream_free /* stream free */
};



