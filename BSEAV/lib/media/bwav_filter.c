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
 * BMedia library, wav stream filter module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bwav_filter.h"
#include "bkni.h"
#include "briff_parser.h"
#include "bmedia_util.h"
#include "bmedia_pes.h"
#include "biobits.h"

BDBG_MODULE(bwav_filter);

#define BDBG_MSG_TRACE(x)   BDBG_MSG(x)

typedef struct b_wav_filter_object_handler {
	briff_parser_handler handler; /* must be first */
	bwav_filter_t filter; /* pointer to the filter */ 
} b_wav_filter_object_handler;


struct bwav_filter {
    BDBG_OBJECT(bwav_filter_t)
    briff_parser_t riff_parser;
    batom_pipe_t pipe_out;
    batom_accum_t accum;
    briff_parser_action (*payload_handler)(bwav_filter_t filter, batom_t object);
    const batom_vec *vecs[1];
    batom_vec meta_data_vec;
    unsigned offset; /* offset is used to calculate PTS'es */
    bmedia_waveformatex fmt;
    bwav_filter_cfg cfg;
    b_wav_filter_object_handler fmt_handler;
    b_wav_filter_object_handler data_handler;
    union {
        struct {
            unsigned packet_size;
        } pcm;
    } u;
};

BDBG_OBJECT_ID(bwav_filter_t);

static briff_parser_action 
b_wav_filter_file_type(void *context, briff_chunk_id file_type)
{
    bwav_filter_t    filter  = context;
    BSTD_UNUSED(filter);
    BDBG_OBJECT_ASSERT(filter, bwav_filter_t);

    return file_type==BMEDIA_FOURCC('W','A','V','E') ? briff_parser_action_none : briff_parser_action_return;
}

#define B_WAV_STREAM_VEC_FRAME	0

static void 
b_wav_filter_error(bwav_filter_t filter)
{
    filter->cfg.error(filter->cfg.user_context);
    return;
}

static void 
b_wav_filter_riff_error(void *context)
{
    bwav_filter_t filter = context;
    BDBG_OBJECT_ASSERT(filter, bwav_filter_t);
    b_wav_filter_error(filter);
    return;
}


static briff_parser_action
b_wav_filter_pcm(bwav_filter_t filter, batom_t object)
{
    size_t packet_size;

    if(object) {
	    batom_accum_add_atom(filter->accum, object);
        batom_release(object);
    }
    packet_size = filter->u.pcm.packet_size;
    for(;;) {
        batom_cursor first, last;
        bmedia_player_pos timestamp;
        bmedia_bcma_hdr hdr;
        batom_t pes_atom;
        batom_t frame;
        size_t data_len;

        data_len = batom_accum_len(filter->accum);

        BDBG_MSG_TRACE(("b_wav_filter_pcm: %p data %u:%u", (void *)filter, (unsigned)data_len, (unsigned)packet_size));
        if(data_len<packet_size) {
            break;
        }
        batom_cursor_from_accum(&first, filter->accum);
        BATOM_CLONE(&last, &first);
        data_len = batom_cursor_skip(&last, packet_size);
        BDBG_ASSERT(data_len==packet_size);
        frame = batom_accum_extract(filter->accum, &first, &last, NULL, NULL);
        batom_accum_trim(filter->accum, &last);
        if(!frame) {
            return briff_parser_action_return;
        }

        timestamp = (8*BMPEG2PES_PTS_UNITS*(uint64_t)filter->offset) / (filter->fmt.nSamplesPerSec * filter->fmt.wBitsPerSample * filter->fmt.nChannels);
        BMEDIA_PACKET_HEADER_INIT(&hdr.pes);
        hdr.pes.pts = timestamp;
        hdr.pes.pts_valid = true;
        hdr.pes.key_frame = true;
        BDBG_MSG(("b_wav_filter_pcm:%p %u audio %s:%u data_len:%u:%u pcm_data_len:%u", (void *)filter, (unsigned)timestamp, hdr.pes.pts_valid?"pts":"no-pts", (unsigned)hdr.pes.pts, (unsigned)data_len, (unsigned)batom_len(frame), filter->offset));
        filter->offset += packet_size;

        hdr.pes.header_off = 4;
        hdr.pes.header_type = B_WAV_STREAM_VEC_FRAME;
        bmedia_bcma_hdr_init(&hdr, packet_size);

        pes_atom = batom_from_vec_and_atom(&filter->meta_data_vec, frame, &bmedia_bcma_atom, &hdr.pes);
        batom_release(frame);
        if(!pes_atom) {
            return briff_parser_action_return;
        }
        batom_pipe_push(filter->pipe_out, pes_atom);
    }
    return briff_parser_action_none;
}

static briff_parser_action 
b_wav_filter_object_end( void *cntx, briff_chunk_id chunk_id, briff_off_t offset)
{
    bwav_filter_t filter = cntx;

    BDBG_OBJECT_ASSERT(filter, bwav_filter_t);
    BSTD_UNUSED(offset);
    BDBG_MSG(("b_wav_filter_object_end:%#lx filter " BMEDIA_FOURCC_FORMAT " %u",  (unsigned long)filter, BMEDIA_FOURCC_ARG(chunk_id), (unsigned)offset));

    if(chunk_id!=BMEDIA_FOURCC('d','a','t','a')) {goto done;}
    if(filter->payload_handler == b_wav_filter_pcm) {
        bmedia_bcma_hdr hdr;
        batom_t pes_atom;
        batom_t frame;
        size_t data_len;

        frame= batom_from_accum(filter->accum, NULL, NULL);
        if(frame) {
            data_len = batom_len(frame);
            if(data_len) {
                BMEDIA_PACKET_HEADER_INIT(&hdr.pes);
                hdr.pes.header_off = 4;
                hdr.pes.header_type = B_WAV_STREAM_VEC_FRAME | B_MEDIA_PACKET_FLAG_EOS;
                
                bmedia_bcma_hdr_init(&hdr, data_len);
                pes_atom = batom_from_vec_and_atom(&filter->meta_data_vec, frame, &bmedia_bcma_atom, &hdr.pes);
                if(pes_atom) {
                    batom_pipe_push(filter->pipe_out, pes_atom);
                }
            }
            batom_release(frame);
        }
    }
done:
    return briff_parser_action_none;
}

static briff_parser_action
b_wav_filter_generic(bwav_filter_t filter, batom_t object)
{
    if(object) {
    	bmedia_packet_header pes;
        batom_t pes_atom;

        BMEDIA_PACKET_HEADER_INIT(&pes);
        if(filter->offset==0) {
            pes.pts_valid = true;
            pes.pts = 0;
        }
        filter->offset += batom_len(object);
        pes_atom = batom_clone(object, bmedia_atom, &pes);
        batom_release(object);
        if(!pes_atom) {
            return briff_parser_action_return;
        }
        batom_pipe_push(filter->pipe_out, pes_atom);
    }
    return briff_parser_action_none;
}

static briff_parser_action 
b_wav_filter_fmt(briff_parser_handler *handler, briff_chunk_id chunk_id, unsigned object_size, unsigned object_offset, batom_t object )
{
    bwav_filter_t    filter = ((b_wav_filter_object_handler *)handler)->filter;

    BDBG_OBJECT_ASSERT(filter, bwav_filter_t);
    BDBG_MSG(("b_wav_filter_fmt:%p filter " BMEDIA_FOURCC_FORMAT " %u:%u:%p(%u)",  (void *)filter, BMEDIA_FOURCC_ARG(chunk_id), object_size, object_offset, (void *)object, object?(unsigned)batom_len(object):0));
    if(filter->payload_handler) {
        b_wav_filter_error(filter);
        goto done;
    }
    if(object_offset == 0) {
        batom_accum_clear(filter->accum);
    }
    if(object) {
        /* combine multiple FMT chunks into the single object */
        if(batom_accum_len(filter->accum)!=object_offset) {
            BDBG_WRN(("%p: out of sequence data", (void *)filter));
            batom_accum_clear(filter->accum);
            batom_release(object);
            goto done;
        }
        batom_accum_add_atom(filter->accum, object);
        batom_release(object);
        if(object_size == batom_accum_len(filter->accum)) {
            batom_cursor cursor;
            bool read_result;

            batom_cursor_from_accum(&cursor, filter->accum);
            read_result = bmedia_read_waveformatex(&filter->fmt, &cursor);
            batom_accum_clear(filter->accum);
            if(!read_result) {
                BDBG_WRN(("%p: invalid wav file", (void *)filter));
                b_wav_filter_error(filter);
                goto done;
            }
            filter->offset = 0;
            if(BMEDIA_WAVFMTEX_AUDIO_PCM(&filter->fmt) || BMEDIA_WAVFMTEX_AUDIO_ADPCM(&filter->fmt) || BMEDIA_WAVFMTEX_AUDIO_G711(&filter->fmt) || filter->fmt.wFormatTag==BMEDIA_WAVFMTEX_AUDIO_PCM_BE_TAG) {
                size_t max_wf_length = BMEDIA_WAVEFORMATEX_BASE_SIZE + BMEDIA_WAVEFORMATEX_MAX_PCM_EXTENSION_SIZE;
                uint8_t *meta_data;

                filter->u.pcm.packet_size = bmedia_waveformatex_pcm_block_size(&filter->fmt);

                meta_data = filter->cfg.alloc->bmem_alloc(filter->cfg.alloc, max_wf_length);
                if(meta_data) {
                    size_t wf_length = bmedia_copy_waveformatex(&filter->fmt, meta_data, max_wf_length);
                    BATOM_VEC_INIT(&filter->meta_data_vec, meta_data, wf_length);
                    filter->vecs[B_WAV_STREAM_VEC_FRAME] = &bmedia_frame_bcma;
                    filter->payload_handler = b_wav_filter_pcm;
                } else {
                    BERR_Code rc = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                    BSTD_UNUSED(rc);
                }
            } else {
                filter->payload_handler = b_wav_filter_generic;
            }
        }
    } 
done:
    return briff_parser_action_none;
}

static briff_parser_action 
b_wav_filter_data(briff_parser_handler *handler, briff_chunk_id chunk_id, unsigned object_size, unsigned object_offset, batom_t object )
{
    bwav_filter_t filter = ((b_wav_filter_object_handler *)handler)->filter;

    BSTD_UNUSED(object_size);
    BSTD_UNUSED(chunk_id);

    BDBG_OBJECT_ASSERT(filter, bwav_filter_t);
    BDBG_MSG(("b_wav_filter_data:%#lx filter " BMEDIA_FOURCC_FORMAT " %u:%u:%#lx(%u)",  (unsigned long)filter, BMEDIA_FOURCC_ARG(chunk_id), object_size, object_offset, (unsigned long)object, object?(unsigned)batom_len(object):0));
    if(filter->offset==0) {
        filter->offset = object_offset;
    }
    if(filter->payload_handler) {
        return filter->payload_handler(filter, object);
    } else {
        if(object) {
            batom_release(object);
        }
        b_wav_filter_error(filter);
    } 
    return briff_parser_action_none;
}


bwav_filter_t
bwav_filter_create(const bwav_filter_cfg *config)
{
    bwav_filter_t    filter;
    briff_parser_cfg riff_cfg;

    BDBG_MSG_TRACE(("b_wav_filter_create"));
    if(config==NULL || config->alloc==NULL || config->factory==NULL) {
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_config;
    }

    filter = BKNI_Malloc(sizeof(*filter));
    if(!filter) {
        BDBG_ERR(("b_wav_filter_create: can't allocate %u bytes", (unsigned)sizeof(*filter)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(filter, bwav_filter_t);
    filter->offset = 0;
    filter->payload_handler =  NULL;
    filter->cfg = *config;
	filter->meta_data_vec.base = NULL;
    filter->pipe_out = NULL;
    briff_parser_default_cfg(&riff_cfg);
    riff_cfg.user_cntx = filter;
    riff_cfg.file_type = b_wav_filter_file_type;
    riff_cfg.stream_error = b_wav_filter_riff_error;
    riff_cfg.object_end = b_wav_filter_object_end;
	filter->riff_parser = briff_parser_create(config->factory, &riff_cfg);
	if(!filter->riff_parser) {
		goto err_create_riff_parser;
	}
    filter->accum = batom_accum_create(config->factory);
    if(!filter->accum) {
        goto err_create_accum;
    }
	filter->fmt_handler.filter = filter;
	filter->data_handler.filter = filter;
	briff_parser_install_handler(filter->riff_parser, &filter->fmt_handler.handler, BMEDIA_FOURCC('f','m','t',' '), b_wav_filter_fmt); 
	briff_parser_install_handler(filter->riff_parser, &filter->data_handler.handler, BMEDIA_FOURCC('d','a','t','a'), b_wav_filter_data); 

    return filter;

err_create_accum:
    briff_parser_destroy(filter->riff_parser);   
err_create_riff_parser:
    BKNI_Free(filter);
err_alloc:
err_config:
    return NULL;
}

void 
bwav_filter_destroy(bwav_filter_t filter)
{
    BDBG_OBJECT_ASSERT(filter, bwav_filter_t);

	briff_parser_remove_handler(filter->riff_parser, &filter->fmt_handler.handler);
	briff_parser_remove_handler(filter->riff_parser, &filter->data_handler.handler);
    bwav_filter_reset(filter);
    briff_parser_destroy(filter->riff_parser);
    batom_accum_destroy(filter->accum);
    BDBG_OBJECT_DESTROY(filter, bwav_filter_t);
    BKNI_Free(filter);
    return;
}

static void 
b_wav_filter_no_error(void *user_cntx)
{
    BSTD_UNUSED(user_cntx);
    return;
}

void 
bwav_filter_default_cfg(bwav_filter_cfg *cfg)
{
    BKNI_Memset(cfg, 0, sizeof(*cfg));
    cfg->error = b_wav_filter_no_error;

}

size_t
bwav_filter_feed(bwav_filter_t filter, batom_pipe_t pipe_in, batom_pipe_t pipe_out)
{
    size_t result;

    BDBG_OBJECT_ASSERT(filter, bwav_filter_t);
    BDBG_ASSERT(pipe_out);

    if(filter->payload_handler) {
        if(filter->payload_handler(filter, NULL)==briff_parser_action_return) {
            return 0;
        }
    }
    filter->pipe_out = pipe_out;

    BDBG_MSG_TRACE((">bwav_filter_feed:%p", (void *)filter));
    result = briff_parser_feed(filter->riff_parser, pipe_in);
    filter->pipe_out = NULL;
    BDBG_MSG_TRACE(("<bwav_filter_feed:%p %u", (void *)filter, (unsigned)result));
    return result;
}

void 
bwav_filter_flush(bwav_filter_t filter)
{
    BDBG_OBJECT_ASSERT(filter, bwav_filter_t);
    batom_accum_clear(filter->accum); 
    filter->offset = 0;
    return;
}

void 
bwav_filter_reset(bwav_filter_t filter)
{
    BDBG_OBJECT_ASSERT(filter, bwav_filter_t);
    bwav_filter_flush(filter);
    filter->payload_handler =  NULL;
    briff_parser_reset(filter->riff_parser);
	if (filter->meta_data_vec.base) {
		filter->cfg.alloc->bmem_free(filter->cfg.alloc, filter->meta_data_vec.base);
        filter->meta_data_vec.base = NULL;
	}
    return;
}

int 
bwav_filter_seek(bwav_filter_t filter, uint64_t off)
{
    int rc;

    rc = briff_parser_seek(filter->riff_parser, off);
    if(rc==0) {
        briff_parser_status status;

        briff_parser_get_status(filter->riff_parser, &status);
        if(status.data_discontinuity) {
            bwav_filter_flush(filter);
        }
    } else {
        bwav_filter_flush(filter);
    }
    return rc;
}

void 
bwav_filter_get_stream_cfg(bwav_filter_t filter, bmedia_pes_stream_cfg *cfg)
{
	BDBG_OBJECT_ASSERT(filter, bwav_filter_t);
	BDBG_ASSERT(cfg);
	bmedia_pes_default_stream_cfg(cfg);
	cfg->nvecs = sizeof(filter->vecs)/sizeof(filter->vecs[0]);
	cfg->vecs = filter->vecs;
	return;
}
