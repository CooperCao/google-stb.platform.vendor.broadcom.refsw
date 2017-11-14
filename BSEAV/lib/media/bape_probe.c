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
 * BMedia library, ape stream probe module
 *
 *******************************************************************************/
#include "bstd.h"
#include "bape_probe.h"
#include "bape_util.h"
#include "bkni.h"

BDBG_MODULE(bape_probe);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */

typedef struct bape_probe  *bape_probe_t; 

struct bape_probe {
    BDBG_OBJECT(bape_probe_t)
    unsigned unused;
};

BDBG_OBJECT_ID(bape_probe_t);

static const bmedia_probe_file_ext b_ape_ext[] =  {
    {"ape"},
    {""}
};

static bool
b_ape_probe_header_match(batom_cursor *header)
{
    bape_file_descriptor file_descriptor;

    return bape_parse_file_descriptor(header, &file_descriptor);
}


static bmedia_probe_base_t 
b_ape_probe_create(batom_factory_t factory)
{
    bape_probe_t    probe;

    BSTD_UNUSED(factory);

    BDBG_MSG_TRACE(("b_ape_probe_create"));

    probe = BKNI_Malloc(sizeof(*probe));
    if(!probe) {
        BDBG_ERR(("b_ape_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(probe, bape_probe_t);

    return (bmedia_probe_base_t)probe;

err_alloc:
    return NULL;
}

static void 
b_ape_probe_destroy(bmedia_probe_base_t probe_)
{
    bape_probe_t probe = (bape_probe_t)probe_;
    BDBG_OBJECT_ASSERT(probe, bape_probe_t);

    BDBG_OBJECT_DESTROY(probe, bape_probe_t);
    BKNI_Free(probe);
    return;
}


static const bmedia_probe_stream *
b_ape_probe_parse(bmedia_probe_base_t probe_, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config)
{
    bape_probe_t probe = (bape_probe_t)probe_;
    bfile_buffer_result result;
    bmedia_probe_stream *stream;
    bmedia_probe_track *track;
    batom_t atom;
    batom_cursor cursor;
    bape_file_descriptor file_descriptor;
    bape_frame_header frame_header;

    BDBG_OBJECT_ASSERT(probe, bape_probe_t);
    BSTD_UNUSED(pipe);

    BDBG_MSG_TRACE(("b_ape_probe_parse:%#lx", (unsigned long)probe));

    
    atom = bfile_buffer_read(buf, config->parse_offset+0, BAPE_FILE_DESCRIPTOR_LENGTH+BAPE_FRAME_HEADER_LENGTH, &result);
    if(!atom) {
        goto err_read;
    }
    batom_cursor_from_atom(&cursor, atom);
    if(!bape_parse_file_descriptor(&cursor, &file_descriptor)) {
        batom_release(atom);
        goto err_descriptor;
    }
    if(!bape_parse_frame_header(&cursor, &frame_header) || frame_header.sample_rate==0 ) {
        batom_release(atom);
        goto err_frame;
    }
    batom_release(atom);
	stream = BKNI_Malloc(sizeof(*stream));
    if(!stream) {
        goto err_stream;
    }
    bmedia_probe_stream_init(stream, bstream_mpeg_type_ape);
    track = BKNI_Malloc(sizeof(*track));
    if(!track) {
        goto err_track;
    }
    bmedia_probe_track_init(track);
    stream->index = bmedia_probe_index_required;
    stream->duration = ((frame_header.total_frames * frame_header.blocks_per_frame + frame_header.final_frame_blocks) * (uint64_t)1000)/frame_header.sample_rate;
    track->type = bmedia_track_type_audio;
    track->info.audio.codec = baudio_format_ape;
    track->number = 1;
    track->info.audio.channel_count = frame_header.channels;
    track->info.audio.sample_size = frame_header.bits_per_sample;
    track->info.audio.sample_rate = frame_header.sample_rate;
    track->info.audio.bitrate = 0;
    bmedia_probe_add_track(stream, track); 
    return stream;

err_track:
    BKNI_Free(stream);
err_stream:
err_frame:
err_descriptor:
err_read:
    return NULL;
}

const bmedia_probe_format_desc bape_probe = {
    bstream_mpeg_type_es,
    b_ape_ext, /* ext_list */
	BAPE_FILE_DESCRIPTOR_LENGTH,
    b_ape_probe_header_match, /* header_match */
    b_ape_probe_create, /* create */
    b_ape_probe_destroy, /* destroy */
    b_ape_probe_parse, /* parse */
    bmedia_probe_basic_stream_free /* stream free */
};



