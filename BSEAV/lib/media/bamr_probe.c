/***************************************************************************
 *     Copyright (c) 2013, Broadcom Corporation
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
#include "bamr_probe.h"
#include "bamr_util.h"
#include "bkni.h"


BDBG_MODULE(bamr_probe);

typedef struct bamr_probe  *bamr_probe_t; 



struct bamr_probe {
    BDBG_OBJECT(bamr_probe)
    batom_factory_t factory;
};

BDBG_OBJECT_ID(bamr_probe);


static bmedia_probe_base_t 
b_amr_probe_create(batom_factory_t factory)
{
    bamr_probe_t    probe;

    probe = BKNI_Malloc(sizeof(*probe));
    if(!probe) {
        BDBG_ERR(("b_amr_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(probe, bamr_probe);
    probe->factory = factory;
    return (bmedia_probe_base_t)probe;

err_alloc:
    return NULL;
}

static void 
b_amr_probe_destroy(bmedia_probe_base_t probe_)
{
    bamr_probe_t probe = (bamr_probe_t) probe_;

    BDBG_OBJECT_ASSERT(probe, bamr_probe);
    BDBG_OBJECT_DESTROY(probe, bamr_probe);
    BKNI_Free(probe);
    return;
}


static const bmedia_probe_stream *
b_amr_probe_parse(bmedia_probe_base_t probe_, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config)
{
    bamr_probe_t probe = (bamr_probe_t)probe_;
    off_t off;
    size_t read_len = BMEDIA_PROBE_FEED_SIZE;
    batom_t atom;
    bfile_buffer_result result;
    size_t atom_len;
    batom_cursor cursor;
    baudio_format codec;

    BSTD_UNUSED(pipe);

    BDBG_OBJECT_ASSERT(probe, bamr_probe);

    off=0;

    BDBG_MSG(("b_amr_probe_parse: %p reading %u:%u", (void *)probe, (unsigned)(off+config->parse_offset), (unsigned)read_len));
    atom = bfile_buffer_read(buf, off+config->parse_offset, read_len, &result);
    if(!atom) {
        return NULL;
    }
    atom_len = batom_len(atom);
    BDBG_MSG(("b_amr_probe_parse: %p read %u:%u -> %p", (void *)probe, (unsigned)(off+config->parse_offset), (unsigned)atom_len, (void *)atom));
    batom_cursor_from_atom(&cursor, atom);

    codec = bamr_parse_header(&cursor);
    batom_release(atom);
    if(codec==baudio_format_amr_nb || codec==baudio_format_amr_wb) {
        bamr_probe_stream *stream;
        bamr_probe_track *track;
        stream = BKNI_Malloc(sizeof(*stream));
        if(!stream) { (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_stream; }
        bmedia_probe_stream_init(&stream->media, bstream_mpeg_type_amr);
        track = BKNI_Malloc(sizeof(*track));
        if(!track) { (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_track_audio; }
        bmedia_probe_track_init(&track->media);
        track->media.type = bmedia_track_type_audio;
        track->media.number = 1;
        track->media.info.audio.channel_count = 1;
        track->media.info.audio.sample_size = 16;
        track->media.info.audio.sample_rate = codec==baudio_format_amr_wb ? 16000 : 8000;
        track->media.info.audio.codec = codec;
        stream->media.type = bstream_mpeg_type_amr;
        /* coverity[address_free] */
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
b_amr_probe_header_match(batom_cursor *cursor)
{
    baudio_format codec;
    
    codec = bamr_parse_header(cursor);
    return codec != baudio_format_unknown;
}

static const bmedia_probe_file_ext b_amr_ext[] =  {
    {"amr"},
    {""}
};

const bmedia_probe_format_desc bamr_probe = {
    bstream_mpeg_type_amr,
    b_amr_ext, /* ext_list */
    BAMR_HEADER_LENGTH,
    b_amr_probe_header_match, /* header_match */
    b_amr_probe_create, /* create */
    b_amr_probe_destroy, /* destroy */
    b_amr_probe_parse, /* parse */
    bmedia_probe_basic_stream_free /* stream free */
};

