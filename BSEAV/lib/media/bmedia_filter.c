/***************************************************************************
 * Copyright (C) 2007-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Media filter library
 *
 *******************************************************************************/
#include "bstd.h"
#include "bmedia_filter.h"
#include "bmpeg1_parser.h"
#include "bkni.h"
#include "balloc.h"
#include "blst_squeue.h"
#include "bmedia_es.h"
#include "bwav_filter.h"
#include "baiff_filter.h"
#include "bogg_parser.h"
#include "bflac_parser.h"
#include "bamr_parser.h"
#if B_HAS_RMFF
#include "brmff_parser.h"
#include "brmff_stream.h"
#endif

#define BDBG_MSG_TRACE(x)   BDBG_MSG(x) 

#if B_HAS_FLV
#include "bflv_parser.h"
#endif


BDBG_MODULE(bmedia_filter);

BDBG_OBJECT_ID(bmedia_filter_t);
BDBG_OBJECT_ID(bmedia_stream_t);

#define B_MEDIA_POOL_BLOCKS 64

struct b_media_mpeg1_parser_handler {
    bmpeg1_parser_handler handler;
    bmedia_stream_t stream;
};

#if B_HAS_FLV
struct b_media_flv_parser_handler {
    bflv_parser_handler handler;
    bmedia_stream_t stream;
};
#endif

struct b_media_ogg_parser_handler {
    bogg_parser_handler handler;
    bmedia_stream_t stream;
};

struct bmedia_stream  {
    BDBG_OBJECT(bmedia_stream_t)
    BLST_SQ_ENTRY(bmedia_stream) link; /* field that is used to link streams together */
    enum {b_media_stream_none, b_media_stream_pes, b_media_stream_es, b_media_stream_ts=bstream_mpeg_type_ts} type;
#if B_HAS_AVI
    bavi_stream_t avi;
#endif
#if B_HAS_FLV
    struct b_media_flv_parser_handler flv;
#endif
#if B_HAS_ASF
    basf_stream_t asf;
#endif
#if B_HAS_RMFF
    brmff_stream_t rmff;
#endif
    struct b_media_ogg_parser_handler ogg;
    struct b_media_mpeg1_parser_handler mpeg1;
    bmp4_fragment_stream_t mp4_fragment;
    bmedia_pes_t pes;
    bmedia_es_t es;
    batom_pipe_t pipe_pes;
    unsigned stream_id;
    batom_pipe_t pipe_sync;
    uint32_t sync_last_pts;
    bool sync_complete;
    bmedia_stream_cfg stream_cfg;
};

typedef struct b_media_flac_header_handler {
	bflac_parser_handler handler; /* must be first */
	bmedia_filter_t filter; /* pointer to filter */ 
} b_media_flac_header_handler;

struct bmedia_filter {
    BDBG_OBJECT(bmedia_filter_t)
    batom_factory_t factory;
    batom_pipe_t    pipe_out;
    balloc_iface_t  data_alloc;
    bmedia_filter_cfg cfg;
    bstream_mpeg_type stream_type;
    bpool_t pool; /* allocater for data chunks */
    unsigned pool_element;
    unsigned time_scale;
    unsigned sync_after_pts;
    unsigned user_sync_after_pts;
    bool feed_stall; /* PES, or ES feed was stalled (i.e., due to lack of resource) */
    bool key_frame_only;
    bool sync_complete;
    bmedia_stream_t master_stream;
#if B_HAS_AVI
    struct {
        bavi_parser_t parser;
        bavi_demux_t demux;
    } avi;
#endif
#if B_HAS_ASF
    struct {
        basf_parser_t parser;
        basf_demux_t demux;
    } asf;
#endif
#if B_HAS_FLV
    struct {
        bflv_parser_t parser;
    } flv;
#endif
#if B_HAS_RMFF
    struct {
        brmff_parser_t parser;
        brmff_demux_t demux;
    } rmff;
#endif
    struct {
        bogg_parser_t parser;
    } ogg;
    struct {
        bamr_parser_t parser;
    } amr;

    struct {
        bflac_parser_t parser;
        batom_accum_t frame_accum;
        size_t sample_rate;
        bmedia_stream_t stream; /* FLAC is just an ES stream so handle it inside filter */
        uint32_t pts;
        uint64_t sample_count;
        uint64_t sample_count_next_header;
		batom_vec streaminfo;
        b_media_flac_header_handler streaminfo_handler;
        const batom_vec *vecs[1];
    } flac;

    struct {
        bmpeg1_parser_t parser;
    } mpeg1;

    struct {
        bwav_filter_t filter;
    } wav;

    struct {
        baiff_filter_t filter;
    } aiff;

    struct {
        bmp4_fragment_demux_t demux;
    } mp4_fragment;

    BLST_SQ_HEAD(b_media_streams, bmedia_stream) free_streams;
    struct bmedia_stream streams[32];
};

void 
bmedia_filter_default_cfg(bmedia_filter_cfg *cfg)
{
    BKNI_Memset(cfg, 0, sizeof(*cfg)); /* initalize everything to 0 */
    cfg->sync_streams_after_seek = false;
    return ;
}

void
bmedia_stream_default_cfg(bmedia_stream_cfg *cfg)
{
    BKNI_Memset(cfg, 0, sizeof(*cfg)); /* initalize everything to 0 */
    cfg->reorder_timestamps = true;
    return;
}


#if B_HAS_AVI || B_HAS_ASF || B_HAS_RMFF
static void 
b_media_stream_error(void *cntx)
{
    bmedia_filter_t filter = cntx;
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    if(filter->cfg.stream_error) {
        filter->cfg.stream_error(filter->cfg.application_cnxt);
    }
    return;
}
#endif

#if B_HAS_AVI
static bavi_parser_action 
b_media_avi_object_end(void *cntx, bavi_fourcc fourcc, bavi_off_t offset)
{
    bmedia_filter_t filter = cntx;

    BSTD_UNUSED(offset);
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);

    if(fourcc==BMEDIA_FOURCC('m','o','v','i')) {
        bavi_demux_movi_end(filter->avi.demux);
    }
    return bavi_parser_action_none;
}

static int
b_media_avi_init(bmedia_filter_t filter)
{
    bavi_parser_cfg avi_cfg;
    bavi_demux_cfg avi_demux_cfg;

    bavi_parser_default_cfg(&avi_cfg);
    avi_cfg.object_end = b_media_avi_object_end;
    avi_cfg.stream_error = b_media_stream_error;
    avi_cfg.user_cntx = filter;
    filter->avi.parser = bavi_parser_create(filter->factory, &avi_cfg);
    if(!filter->avi.parser) {
        goto err_avi_parser_create;
    }
    bavi_demux_default_cfg(&avi_demux_cfg);
    BDBG_ASSERT(filter->pool_element >= avi_demux_cfg.pes_hdr_size);
    filter->avi.demux = bavi_demux_create(filter->avi.parser, filter->factory, bpool_alloc_iface(filter->pool), &avi_demux_cfg);
    if(!filter->avi.demux) {
        goto err_avi_demux;
    }
    return 0;

err_avi_demux:
    bavi_parser_destroy(filter->avi.parser);
err_avi_parser_create:
    return -1;

}


static void 
b_media_avi_uninit(bmedia_filter_t filter)
{
    bavi_demux_destroy(filter->avi.demux);
    bavi_parser_destroy(filter->avi.parser);
    return;
}
#endif /* B_HAS_AVI */

#if B_HAS_ASF
static int
b_media_asf_init(bmedia_filter_t filter)
{
    basf_parser_cfg asf_cfg;
    basf_demux_cfg asf_demux_cfg;

    basf_parser_default_cfg(&asf_cfg);
    asf_cfg.application_cnxt = filter;
    asf_cfg.stream_error = b_media_stream_error;
    filter->asf.parser = basf_parser_create(filter->factory, &asf_cfg);
    if(!filter->asf.parser) {
        goto err_asf_parser_create;
    }
    basf_demux_default_cfg(&asf_demux_cfg);
    BDBG_ASSERT(filter->pool_element >= asf_demux_cfg.stream_hdr_size);
    filter->asf.demux = basf_demux_create(filter->asf.parser, filter->factory, bpool_alloc_iface(filter->pool), &asf_demux_cfg);
    if(!filter->asf.demux) {
        goto err_asf_demux;
    }
    return 0;

err_asf_demux:
    basf_parser_destroy(filter->asf.parser);
err_asf_parser_create:
    return -1;

}


static void 
b_media_asf_uninit(bmedia_filter_t filter)
{
    basf_demux_destroy(filter->asf.demux);
    basf_parser_destroy(filter->asf.parser);
    return;
}

static void* b_media_asf_get_handle(bmedia_filter_t filter)
{
        return (void*)filter->asf.parser;
}

#endif /* B_HAS_ASF */

static int
b_media_mpeg1_init(bmedia_filter_t filter)
{
    bmpeg1_parser_cfg cfg;
    
    bmpeg1_parser_default_cfg(&cfg);

    filter->mpeg1.parser = bmpeg1_parser_create(filter->factory, &cfg);
    if(!filter->mpeg1.parser) {
        return -1;
    }
    return 0;
}

static void 
b_media_mpeg1_uninit(bmedia_filter_t filter)
{
    bmpeg1_parser_destroy(filter->mpeg1.parser);
    return;
}

static int
b_media_wav_init(bmedia_filter_t filter)
{
    bwav_filter_cfg cfg;
    
    bwav_filter_default_cfg(&cfg);
    cfg.factory = filter->factory;
    cfg.alloc = filter->data_alloc;
    filter->wav.filter = bwav_filter_create(&cfg);
    if(!filter->wav.filter) {
        return -1;
    }
    return 0;
}

static void 
b_media_wav_uninit(bmedia_filter_t filter)
{
    bwav_filter_destroy(filter->wav.filter);
    return;
}

static int
b_media_aiff_init(bmedia_filter_t filter)
{
    baiff_filter_cfg cfg;
    
    baiff_filter_default_cfg(&cfg);
    cfg.factory = filter->factory;
    cfg.alloc = filter->data_alloc;
    filter->aiff.filter = baiff_filter_create(&cfg);
    if(!filter->aiff.filter) {
        return -1;
    }
    return 0;
}

static void 
b_media_aiff_uninit(bmedia_filter_t filter)
{
    baiff_filter_destroy(filter->aiff.filter);
    return;
}

static int
b_media_mp4_fragment_init(bmedia_filter_t filter)
{
    bmp4_fragment_demux_cfg cfg;
    
    bmp4_fragment_demux_default_cfg(&cfg);
    cfg.factory = filter->factory;
    cfg.alloc = filter->data_alloc;
    if(filter->cfg.update_mp4_fragment_cfg) {
        filter->cfg.update_mp4_fragment_cfg(filter->cfg.application_cnxt, &cfg);
    }
    filter->mp4_fragment.demux = bmp4_fragment_demux_create(&cfg);
    if(!filter->mp4_fragment.demux) {
        return -1;
    }
    return 0;
}

static void 
b_media_mp4_fragment_uninit(bmedia_filter_t filter)
{
    bmp4_fragment_demux_destroy(filter->mp4_fragment.demux);
    return;
}


#if B_HAS_FLV
static int
b_media_flv_init(bmedia_filter_t filter)
{
    bflv_parser_cfg cfg;
    
    bflv_parser_default_cfg(&cfg);
    cfg.alloc = filter->data_alloc;

    filter->flv.parser = bflv_parser_create(filter->factory, &cfg);
    if(!filter->flv.parser) {
        return -1;
    }
    return 0;
}

static void 
b_media_flv_uninit(bmedia_filter_t filter)
{
    bflv_parser_destroy(filter->flv.parser);
    return;
}
#endif

#if B_HAS_RMFF
static brmff_parser_action 
b_media_rmff_object_end(void *cntx, const brmff_object_header *header, brmff_off_t offset)
{
    bmedia_filter_t filter = cntx;

    BSTD_UNUSED(offset);
    BDBG_ASSERT(header);
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    if(header->id==BRMFF_ID_DATA) {
        brmff_demux_data_end(filter->rmff.demux);
    }
    return brmff_parser_action_none;
}

static int
b_media_rmff_init(bmedia_filter_t filter)
{
    brmff_parser_cfg rmff_cfg;
    brmff_demux_cfg rmff_demux_cfg;

    brmff_parser_default_cfg(&rmff_cfg);
    rmff_cfg.object_end = b_media_rmff_object_end;
    rmff_cfg.stream_error = b_media_stream_error;
    rmff_cfg.user_cntx = filter;
    filter->rmff.parser = brmff_parser_create(filter->factory, &rmff_cfg);
    if(!filter->rmff.parser) {
        goto err_rmff_parser_create;
    }
    brmff_demux_default_cfg(&rmff_demux_cfg);
    BDBG_ASSERT(filter->pool_element >= rmff_demux_cfg.pes_hdr_size);
    filter->rmff.demux = brmff_demux_create(filter->rmff.parser, filter->factory, bpool_alloc_iface(filter->pool), &rmff_demux_cfg);
    if(!filter->rmff.demux) {
        goto err_rmff_demux;
    }
    return 0;

err_rmff_demux:
    brmff_parser_destroy(filter->rmff.parser);
err_rmff_parser_create:
    return -1;

}


static void 
b_media_rmff_uninit(bmedia_filter_t filter)
{
    brmff_demux_destroy(filter->rmff.demux);
    brmff_parser_destroy(filter->rmff.parser);
    return;
}
#endif /* B_HAS_RMFF */

static int
b_media_ogg_init(bmedia_filter_t filter)
{
    bogg_parser_cfg cfg;
    
    bogg_parser_default_cfg(&cfg);
    cfg.alloc = filter->data_alloc;

    filter->ogg.parser = bogg_parser_create(filter->factory, &cfg);
    if(!filter->ogg.parser) {
        return -1;
    }
    return 0;
}

static void 
b_media_ogg_uninit(bmedia_filter_t filter)
{
    bogg_parser_destroy(filter->ogg.parser);
    return;
}

#define B_FLAC_VEC_BCMA_FRAME	0
static void 
b_media_flac_clear(bmedia_filter_t filter)
{
    filter->flac.streaminfo.base = NULL;
    filter->flac.pts = 0;
    filter->flac.sample_count = 0;
    filter->flac.sample_count_next_header = 0;
    filter->flac.vecs[B_FLAC_VEC_BCMA_FRAME] = &bmedia_frame_bcma;
    return;
}

#define B_FLAC_METADATA_HEADER_LEN (4+BFLAC_BLOCK_HEADER_LENGTH)
static bflac_parser_action 
b_media_flac_streaminfo(bflac_parser_handler *handler, batom_t object)
{
    bmedia_filter_t filter;
    batom_cursor cursor;
    bflac_streaminfo streaminfo;

    BDBG_ASSERT(handler);
    BDBG_ASSERT(object);
    filter = ((b_media_flac_header_handler*)handler)->filter;
    BDBG_MSG(("b_media_flac_header_handler:%p", (void *)filter));
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    if(filter->flac.streaminfo.base) {
        goto error;
    }
    batom_cursor_from_atom(&cursor, object);
    if(!bflac_parse_streaminfo(&cursor, &streaminfo)) {
        goto error;
    }
    b_media_flac_clear(filter);
    filter->flac.sample_rate = streaminfo.sample_rate;
    filter->flac.streaminfo.len = batom_len(object)+B_FLAC_METADATA_HEADER_LEN;
    filter->flac.streaminfo.base = filter->data_alloc->bmem_alloc(filter->data_alloc, filter->flac.streaminfo.len);
    if(filter->flac.streaminfo.base==NULL) {
        goto error;
    }
    batom_cursor_from_atom(&cursor, object);
    B_MEDIA_SAVE_UINT32_LE(((uint8_t*)filter->flac.streaminfo.base), BFLAC_HEADER_TAG);
    ((uint8_t *)filter->flac.streaminfo.base)[4+0] = (1<<7)|BFLAC_BLOCK_HEADER_STREAMINFO;
    ((uint8_t *)filter->flac.streaminfo.base)[4+1] = 0;
    B_MEDIA_SAVE_UINT16_BE(((uint8_t*)filter->flac.streaminfo.base)+4+2, filter->flac.streaminfo.len-B_FLAC_METADATA_HEADER_LEN);
    batom_cursor_copy(&cursor, (uint8_t *)filter->flac.streaminfo.base+B_FLAC_METADATA_HEADER_LEN, filter->flac.streaminfo.len-B_FLAC_METADATA_HEADER_LEN);
    batom_release(object);
    return bflac_parser_action_none;

error:
    batom_release(object);
    return bflac_parser_action_return;
}

static bflac_parser_action
b_media_flac_frame(void *user_cntx, batom_t frame, const bflac_frame_header *header)
{
    bmedia_filter_t filter = user_cntx;

    BDBG_MSG(("b_media_flac_frame:%p %p %u", (void *)filter, (void *)frame, header->samples));
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    if(filter->flac.sample_rate==0 || filter->flac.stream == NULL) { goto done; }
    batom_accum_add_atom(filter->flac.frame_accum, frame);
    if(filter->flac.pts==0) {
        filter->flac.pts = ((uint64_t)45*1000*filter->flac.sample_count)/filter->flac.sample_rate;
    }
    filter->flac.sample_count += header->samples;
    BDBG_MSG_TRACE(("b_media_flac_frame:%p samples %u next_header:%u", (void *)filter, (unsigned)filter->flac.sample_count, (unsigned)filter->flac.sample_count_next_header));
    if(filter->flac.sample_count>=filter->flac.sample_count_next_header) {
        batom_t payload = batom_from_accum(filter->flac.frame_accum, NULL, NULL);
        if(payload) {
            bmedia_bcma_hdr hdr;
            batom_t pes_atom;
            size_t frame_size = batom_len(payload)+filter->flac.streaminfo.len;

            BMEDIA_PACKET_HEADER_INIT(&hdr.pes);
            BMEDIA_PES_SET_PTS(&hdr.pes, filter->flac.pts);
            hdr.pes.header_off = 4;
            hdr.pes.header_type = B_FLAC_VEC_BCMA_FRAME;
            bmedia_bcma_hdr_init(&hdr, frame_size);
            filter->flac.sample_count_next_header = filter->flac.sample_count + (32*filter->flac.sample_rate)/1000; /* insert streaminfo header every 32 miliseconds */
            BDBG_MSG(("b_media_flac_frame:%p adding streaminfo at %u:%u frame_size:%u pts:%u", (void *)filter, (unsigned)filter->flac.sample_count, (unsigned)filter->flac.sample_count_next_header, (unsigned)frame_size, filter->flac.pts));
            filter->flac.pts = 0;
            pes_atom = batom_from_vec_and_atom(&filter->flac.streaminfo, payload, &bmedia_bcma_atom, &hdr);
            batom_release(payload);
            if(pes_atom) {
                batom_pipe_push(filter->flac.stream->pipe_pes, pes_atom);
            }
        }
    }
done:
    batom_release(frame);
    return bflac_parser_action_none;
}

static int
b_media_flac_init(bmedia_filter_t filter)
{
    bflac_parser_cfg cfg;
    
    bflac_parser_default_cfg(&cfg);

    cfg.user_cntx = filter;
    cfg.frame = b_media_flac_frame;
    filter->flac.parser = bflac_parser_create(filter->factory, &cfg);
    if(!filter->flac.parser) {
        goto err_parser;
    }
    filter->flac.frame_accum = batom_accum_create(filter->factory);
    if(!filter->flac.frame_accum) {
        goto err_frame_accum;
    }
    filter->flac.streaminfo_handler.filter = filter;
    filter->flac.stream = NULL;
    BATOM_VEC_INIT(&filter->flac.streaminfo, NULL, 0);
    bflac_parser_install_handler(filter->flac.parser, &filter->flac.streaminfo_handler.handler, BFLAC_BLOCK_HEADER_STREAMINFO, b_media_flac_streaminfo);
    return 0;

err_frame_accum:
    bflac_parser_destroy(filter->flac.parser);
err_parser:
    return -1;
}

static void 
b_media_flac_uninit(bmedia_filter_t filter)
{
    BDBG_ASSERT(filter->flac.streaminfo.base==NULL);
    bflac_parser_remove_handler(filter->flac.parser, &filter->flac.streaminfo_handler.handler);
    bflac_parser_destroy(filter->flac.parser);
    batom_accum_destroy(filter->flac.frame_accum);
    return;
}

static void 
b_media_flac_reset(bmedia_filter_t filter)
{
    batom_accum_clear(filter->flac.frame_accum);
    if(filter->flac.streaminfo.base) {
        filter->data_alloc->bmem_free(filter->data_alloc, filter->flac.streaminfo.base);
    }
    b_media_flac_clear(filter);
    bflac_parser_reset(filter->flac.parser);
    return;
}

static bamr_parser_action 
b_media_filter_amr_data(void *_filter, batom_t object)
{
    bmedia_filter_t filter = _filter;
    bmedia_stream_t stream;
    unsigned i;
    BDBG_MSG_TRACE(("b_media_filter_amr_data>: %p %p:%u", (void *)filter, (void *)object,object?(unsigned)batom_len(object):0));
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    for(stream=NULL,i=0;i<sizeof(filter->streams)/sizeof(*filter->streams);i++) {
        if(filter->streams[i].type !=  b_media_stream_none) {
            stream = &filter->streams[i];
            break;
        }
    }
    if(stream) {
        batom_pipe_push(stream->pipe_pes, object); 
    } else {
        batom_release(object);
    }
    return bamr_parser_action_none;
}

static int
b_media_amr_init(bmedia_filter_t filter)
{
    bamr_parser_cfg cfg;
    
    bamr_parser_default_cfg(&cfg);
    cfg.alloc = filter->data_alloc;
    cfg.user_cntx = filter;
    cfg.frame = b_media_filter_amr_data;

    filter->amr.parser = bamr_parser_create(filter->factory, &cfg);
    if(!filter->amr.parser) {
        return -1;
    }
    return 0;
}

static void 
b_media_amr_uninit(bmedia_filter_t filter)
{
    bamr_parser_destroy(filter->amr.parser);
    return;
}


bmedia_filter_t 
bmedia_filter_create(batom_factory_t factory, balloc_iface_t data_alloc, const bmedia_filter_cfg *cfg)
{
    bmedia_filter_t filter;
    bmedia_pes_cfg pes_cfg;
    unsigned i;

    filter = BKNI_Malloc(sizeof(*filter));
    if (!filter) {
        BDBG_ERR(("bmedia_filter_create: can't allocated %u bytes", (unsigned)sizeof(*filter)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(filter, bmedia_filter_t);
    filter->factory = factory;
    filter->data_alloc = data_alloc;
    filter->cfg = *cfg;
    filter->stream_type = bstream_mpeg_type_es; /* e.g. invalid type */
    filter->master_stream = NULL;
    filter->key_frame_only = false;
    filter->sync_complete = true;
    filter->sync_after_pts = B_MEDIA_INVALID_PTS;
    filter->user_sync_after_pts = B_MEDIA_INVALID_PTS;
    filter->time_scale = BMEDIA_TIME_SCALE_BASE;
    BLST_SQ_INIT(&filter->free_streams);
    for(i=0;i<sizeof(filter->streams)/sizeof(*filter->streams);i++) {
        filter->streams[i].type = b_media_stream_none;
#if B_HAS_AVI
        filter->streams[i].avi = NULL;
#endif
#if B_HAS_ASF
        filter->streams[i].asf = NULL;
#endif
        filter->streams[i].pes = NULL;
        filter->streams[i].pipe_pes = NULL;
        filter->streams[i].pipe_sync = NULL;
    }
    filter->pipe_out = batom_pipe_create(factory);
    if(!filter->pipe_out) {
        goto err_pipe_out;
    }

    bmedia_pes_default_cfg(&pes_cfg);
    pes_cfg.eos_len = cfg->eos_len;
    pes_cfg.eos_data = cfg->eos_data;

    filter->pool_element = pes_cfg.pes_hdr_size;
    filter->pool = bpool_create(filter->data_alloc, B_MEDIA_POOL_BLOCKS, pes_cfg.pes_hdr_size);
    if (!filter->pool) {
        goto err_pool;
    }

#if B_HAS_AVI
    if(b_media_avi_init(filter)!=0) {
        goto err_avi;
    }
#endif
#if B_HAS_ASF
    if(b_media_asf_init(filter)!=0) {
        goto err_asf;
    }
#endif
#if B_HAS_FLV
    if(b_media_flv_init(filter)!=0) {
        goto err_flv;
    }
#endif
#if B_HAS_RMFF
    if(b_media_rmff_init(filter)!=0) {
        goto err_rmff;
    }
#endif
    if(b_media_ogg_init(filter)!=0) {
        goto err_ogg;
    }
    if(b_media_amr_init(filter)!=0) {
        goto err_amr;
    }
    if(b_media_flac_init(filter)!=0) {
        goto err_flac;
    }
    if(b_media_mpeg1_init(filter)!=0) {
        goto err_mpeg1;
    }
    if(b_media_wav_init(filter)!=0) {
        goto err_wav;
    }
    if(b_media_aiff_init(filter)!=0) {
        goto err_aiff;
    }
    if(b_media_mp4_fragment_init(filter)!=0) {
        goto err_mp4_fragment;
    }
    for(i=0;i<sizeof(filter->streams)/sizeof(*filter->streams);i++) {
        filter->streams[i].sync_last_pts = B_MEDIA_INVALID_PTS;
        filter->streams[i].sync_complete = true;
        filter->streams[i].pes = bmedia_pes_create(factory, bpool_alloc_iface(filter->pool), &pes_cfg);
        if(!filter->streams[i].pes) {
            goto err_streams_pes;
        }
        filter->streams[i].es = bmedia_es_create(factory, bpool_alloc_iface(filter->pool), &pes_cfg);
        if(!filter->streams[i].es) {
            goto err_streams_pes;
        }
        filter->streams[i].pipe_pes = batom_pipe_create(factory);
        if(!filter->streams[i].pipe_pes) {
            goto err_streams_pes;
        }
        filter->streams[i].pipe_sync = batom_pipe_create(factory);
        if(!filter->streams[i].pipe_sync) {
            goto err_streams_pes;
        }
        BLST_SQ_INSERT_TAIL(&filter->free_streams, &filter->streams[i], link);
    }


    return filter;

err_streams_pes:
    for(i=0;i<sizeof(filter->streams)/sizeof(*filter->streams) && filter->streams[i].pes;i++) {
        bmedia_pes_destroy(filter->streams[i].pes);
        if(filter->streams[i].es) {
            bmedia_es_destroy(filter->streams[i].es);
        }
        if(filter->streams[i].pipe_pes) {
            batom_pipe_destroy(filter->streams[i].pipe_pes);
        }
        if(filter->streams[i].pipe_sync) {
            batom_pipe_destroy(filter->streams[i].pipe_sync);
        }
    }

    b_media_mp4_fragment_uninit(filter);
err_mp4_fragment:
    b_media_aiff_uninit(filter);
err_aiff:
    b_media_wav_uninit(filter);
err_wav:
    b_media_mpeg1_uninit(filter);
err_mpeg1:
    b_media_flac_uninit(filter);
err_flac:
    b_media_amr_uninit(filter);
err_amr:
    b_media_ogg_uninit(filter);
err_ogg:
#if B_HAS_RMFF
    b_media_rmff_uninit(filter);
err_rmff:
#endif
#if B_HAS_FLV
    b_media_flv_uninit(filter);
err_flv:
#endif
#if B_HAS_ASF
    b_media_asf_uninit(filter);
err_asf:
#endif
#if B_HAS_AVI
    b_media_avi_uninit(filter);
err_avi:
#endif
    bpool_destroy(filter->pool);
err_pool:
    batom_pipe_destroy(filter->pipe_out);
err_pipe_out:
    BKNI_Free(filter);
err_alloc:
    return NULL;
}

void 
bmedia_filter_destroy(bmedia_filter_t filter)
{
    unsigned i;
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    for(i=0;i<sizeof(filter->streams)/sizeof(*filter->streams);i++) {
        bmedia_pes_destroy(filter->streams[i].pes);
        bmedia_es_destroy(filter->streams[i].es);
        batom_pipe_destroy(filter->streams[i].pipe_pes);
        batom_pipe_destroy(filter->streams[i].pipe_sync);
    }
    b_media_mp4_fragment_uninit(filter);
    b_media_aiff_uninit(filter);
    b_media_wav_uninit(filter);
    b_media_mpeg1_uninit(filter);
    b_media_flac_uninit(filter);
    b_media_amr_uninit(filter);
    b_media_ogg_uninit(filter);
#if B_HAS_AVI
    b_media_avi_uninit(filter);
#endif
#if B_HAS_ASF
    b_media_asf_uninit(filter);
#endif
#if B_HAS_FLV
    b_media_flv_uninit(filter);
#endif
#if B_HAS_RMFF
    b_media_rmff_uninit(filter);
#endif
    bpool_destroy(filter->pool);
    batom_pipe_destroy(filter->pipe_out);
    BDBG_OBJECT_DESTROY(filter, bmedia_filter_t);
    BKNI_Free(filter);
    return;
}

batom_pipe_t 
bmedia_filter_start(bmedia_filter_t filter, bstream_mpeg_type type)
{
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    BDBG_MSG_TRACE(("bmedia_filter_start: %#lx %u", (unsigned long)filter, (unsigned)type));
    switch(type) {
#if B_HAS_ASF
    case bstream_mpeg_type_asf:
#endif
#if B_HAS_AVI
    case bstream_mpeg_type_avi:
#endif
#if B_HAS_FLV
    case bstream_mpeg_type_flv:
#endif
    case bstream_mpeg_type_amr:
    case bstream_mpeg_type_ogg:
    case bstream_mpeg_type_flac:
    case bstream_mpeg_type_ts:
    case bstream_mpeg_type_pes:
    case bstream_mpeg_type_mp4:
    case bstream_mpeg_type_ape:
    case bstream_mpeg_type_mpeg1:
#if B_HAS_RMFF
    case bstream_mpeg_type_rmff:
#endif
    case bstream_mpeg_type_wav:
    case bstream_mpeg_type_aiff:
    case bstream_mpeg_type_mp4_fragment:
        break;
    default:
        BDBG_WRN(("bmedia_filter_start: not supported format %u", (unsigned)type));
        return NULL;
    }
    filter->stream_type = type;
    filter->feed_stall = false;
    return filter->pipe_out;
}

static void 
b_media_filter_stream_close(bmedia_filter_t filter, bmedia_stream_t stream)
{
    unsigned i;

    BDBG_MSG_TRACE(("b_media_filter_stream_close: %#lx %lx", (unsigned long)filter, (unsigned long)stream));

    BDBG_OBJECT_UNSET(stream, bmedia_stream_t);
    BDBG_ASSERT(stream->type!=b_media_stream_none);
    BSTD_UNUSED(i);
    switch(filter->stream_type) {
#if B_HAS_AVI
    case bstream_mpeg_type_avi:     
        for(i=0; i<(sizeof(filter->cfg.avi)/sizeof(filter->cfg.avi[0])); i++){
            if(filter->cfg.avi[i].deactivate_avi_stream){
                filter->cfg.avi[i].deactivate_avi_stream(filter->cfg.application_cnxt, stream->stream_id, filter->avi.parser, stream->avi);
            }
        }
        bavi_stream_deactivate(stream->avi);
        break;
#endif
#if B_HAS_ASF
    case bstream_mpeg_type_asf:
        basf_stream_detach(filter->asf.demux, stream->asf);
        break;
#endif
#if B_HAS_FLV
    case bstream_mpeg_type_flv:
        bflv_parser_remove_handler(filter->flv.parser, &stream->flv.handler);
        break;
#endif
#if B_HAS_RMFF
    case bstream_mpeg_type_rmff:
        brmff_stream_deactivate(stream->rmff);
        break; 
#endif
    case bstream_mpeg_type_flac:
        filter->flac.stream = NULL;
        b_media_flac_reset(filter);
        break;
    case bstream_mpeg_type_ogg:
        bogg_parser_remove_handler(filter->ogg.parser, &stream->ogg.handler);
        break;
    case bstream_mpeg_type_mpeg1:
        bmpeg1_parser_remove_handler(filter->mpeg1.parser, &stream->mpeg1.handler);
        break;
    case bstream_mpeg_type_wav:
        bwav_filter_reset(filter->wav.filter);
        break; 
    case bstream_mpeg_type_aiff:
        baiff_filter_reset(filter->aiff.filter);
        break; 
    case bstream_mpeg_type_mp4_fragment:

        bmp4_fragment_stream_destroy(filter->mp4_fragment.demux, stream->mp4_fragment);
        stream->mp4_fragment = NULL;
        bmp4_fragment_demux_reset(filter->mp4_fragment.demux);
        break; 
    case bstream_mpeg_type_pes:
    case bstream_mpeg_type_mp4:
    case bstream_mpeg_type_ape:
    case bstream_mpeg_type_amr:
        break;
    default:
        break;
    }
    if(stream->type==b_media_stream_pes) {
        bmedia_pes_stop(stream->pes);
    } else {
        bmedia_es_stop(stream->es);
    }
    batom_pipe_flush(stream->pipe_pes);
    batom_pipe_flush(stream->pipe_sync);
    bmedia_pes_reset(stream->pes);
    stream->type = b_media_stream_none;
    if (filter->master_stream == stream) {
        filter->master_stream = NULL;
    }
    return;
}


void 
bmedia_filter_stop(bmedia_filter_t filter)
{
    unsigned i;
    BDBG_MSG_TRACE(("bmedia_filter_stop: %#lx", (unsigned long)filter));

    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    switch(filter->stream_type) {
#if B_HAS_AVI
    case bstream_mpeg_type_avi:
        bavi_parser_reset(filter->avi.parser);
        bavi_demux_reset(filter->avi.demux);
        break;
#endif
#if B_HAS_ASF
    case bstream_mpeg_type_asf:
        basf_parser_reset(filter->asf.parser);
        break;
#endif
#if B_HAS_FLV
    case bstream_mpeg_type_flv:
        bflv_parser_reset(filter->flv.parser);
        break;
#endif
#if B_HAS_RMFF
    case bstream_mpeg_type_rmff:
        brmff_parser_reset(filter->rmff.parser);
        break;
#endif
    case bstream_mpeg_type_flac:
        if(filter->flac.streaminfo.base) {
            filter->data_alloc->bmem_free(filter->data_alloc, filter->flac.streaminfo.base);
            b_media_flac_clear(filter);
        }
        break;
    case bstream_mpeg_type_amr:
        bamr_parser_reset(filter->amr.parser);
        break;
    case bstream_mpeg_type_ogg:
        bogg_parser_reset(filter->ogg.parser);
        break;
    case bstream_mpeg_type_mpeg1:
        bmpeg1_parser_reset(filter->mpeg1.parser);
        break;
    case bstream_mpeg_type_wav:
        bwav_filter_reset(filter->wav.filter);
        break; 
    case bstream_mpeg_type_aiff:
        baiff_filter_reset(filter->aiff.filter);
        break; 
    case bstream_mpeg_type_mp4_fragment:
        bmp4_fragment_demux_reset(filter->mp4_fragment.demux);
        break; 
    case bstream_mpeg_type_mp4:
    case bstream_mpeg_type_pes:
    case bstream_mpeg_type_ape:
        break;
    default:
        break;
    }
    batom_pipe_flush(filter->pipe_out);
    BLST_SQ_INIT(&filter->free_streams);
    for(i=0;i<sizeof(filter->streams)/sizeof(*filter->streams);i++) {
        if(filter->streams[i].type!=b_media_stream_none) {
            b_media_filter_stream_close(filter, &filter->streams[i]);
        }
        BDBG_OBJECT_UNSET(&filter->streams[i], bmedia_stream_t);
        BLST_SQ_INSERT_TAIL(&filter->free_streams, &filter->streams[i], link);
    }
#if B_HAS_ASF
    if(filter->stream_type == bstream_mpeg_type_asf) {
        basf_demux_reset(filter->asf.demux);
    }
#endif
    return;
}

static void 
b_media_filter_clear_streams(bmedia_filter_t filter)
{
    unsigned i;
    for(i=0;i<sizeof(filter->streams)/sizeof(*filter->streams);i++) {
        bmedia_stream_t stream = &filter->streams[i];
        switch(filter->streams[i].type) {
        case b_media_stream_pes:
            bmedia_pes_reset(stream->pes);
            break;
        case b_media_stream_es:
        case b_media_stream_none:
        default:
            break;
        }
        batom_pipe_flush(stream->pipe_pes);
        batom_pipe_flush(stream->pipe_sync);
    }
    batom_pipe_flush(filter->pipe_out);
    return;
}


void 
bmedia_filter_flush(bmedia_filter_t filter)
{
    BDBG_MSG_TRACE(("bmedia_filter_flush>: %#lx", (unsigned long)filter));

    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    switch(filter->stream_type) {
#if B_HAS_AVI
    case bstream_mpeg_type_avi:
        bavi_parser_reset(filter->avi.parser);
        bavi_demux_reset(filter->avi.demux);
        break;
#endif
#if B_HAS_ASF
    case bstream_mpeg_type_asf:
        basf_parser_reset(filter->asf.parser);
        basf_demux_flush(filter->asf.demux);
        break;
#endif
#if B_HAS_FLV
    case bstream_mpeg_type_flv:
        bflv_parser_reset(filter->flv.parser);
        break;
#endif
    case bstream_mpeg_type_mpeg1:
        bmpeg1_parser_reset(filter->mpeg1.parser);
        break;
    case bstream_mpeg_type_wav:
        bwav_filter_reset(filter->wav.filter);
        break;
    case bstream_mpeg_type_aiff:
        baiff_filter_reset(filter->aiff.filter);
        break;
    case bstream_mpeg_type_mp4_fragment:
        bmp4_fragment_demux_reset(filter->mp4_fragment.demux);
        break;
#if B_HAS_RMFF
    case bstream_mpeg_type_rmff:
        brmff_parser_reset(filter->rmff.parser);
        brmff_demux_reset(filter->rmff.demux);
        break;
#endif
    case bstream_mpeg_type_ogg:
        bogg_parser_flush(filter->ogg.parser);
        break;
    case bstream_mpeg_type_amr:
        bamr_parser_flush(filter->amr.parser);
        break;
    case bstream_mpeg_type_flac:
        b_media_flac_reset(filter);
        break;
    case bstream_mpeg_type_mp4:
    case bstream_mpeg_type_pes:
    case bstream_mpeg_type_ape:
    default:
        break;
    }
    b_media_filter_clear_streams(filter);
    return;
}

void 
bmedia_filter_clear(bmedia_filter_t filter)
{
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    switch(filter->stream_type) {
#if B_HAS_AVI
    case bstream_mpeg_type_avi:
        {
            bavi_parser_status status;
            bavi_parser_get_status(filter->avi.parser, &status);
            if(status.offset>0) {
                bavi_parser_seek(filter->avi.parser, status.offset-1);
            } else {
                bavi_parser_seek(filter->avi.parser, 1);
            }
            bavi_parser_seek(filter->avi.parser, status.offset);
            bavi_demux_flush(filter->avi.demux);
            break;
        }
#endif
#if B_HAS_ASF
    case bstream_mpeg_type_asf:
        {
            basf_parser_info info;
            basf_parser_get_info(filter->asf.parser, &info);
            if(info.data.data_offset>0) {
                basf_parser_seek(filter->asf.parser, info.data.data_offset);
                basf_parser_seek(filter->asf.parser, info.offset);
            }
            basf_demux_flush(filter->asf.demux);
            break;
        }
#endif
#if B_HAS_RMFF
    case bstream_mpeg_type_rmff:
        {
            brmff_parser_status status;
            brmff_parser_get_status(filter->rmff.parser, &status);
            if(status.offset>0) {
                brmff_parser_seek(filter->rmff.parser, status.offset-1);
            } else {
                brmff_parser_seek(filter->rmff.parser, 1);
            }
            brmff_parser_seek(filter->rmff.parser, status.offset);
            brmff_demux_flush(filter->rmff.demux);
        }
        break;
#endif
#if B_HAS_FLV
    case bstream_mpeg_type_flv:
        bflv_parser_reset(filter->flv.parser);
        break;
#endif
    case bstream_mpeg_type_wav:
        bwav_filter_flush(filter->wav.filter);
        break;
    case bstream_mpeg_type_aiff:
        baiff_filter_flush(filter->aiff.filter);
        break;
    case bstream_mpeg_type_ogg:
        bogg_parser_flush(filter->ogg.parser);
        break;
    case bstream_mpeg_type_amr:
        bamr_parser_flush(filter->amr.parser);
        break;
    case bstream_mpeg_type_flac:
        b_media_flac_reset(filter);
        break;
    case bstream_mpeg_type_mp4_fragment:
        bmp4_fragment_demux_flush(filter->mp4_fragment.demux);
        break;
    default:
        BDBG_WRN(("bmedia_filter_clear: %#lx not supported for stream type %u", (unsigned long)filter, (unsigned)filter->stream_type));
        break;
    }
    b_media_filter_clear_streams(filter);
    return;
}

static bmpeg1_parser_action 
b_media_filter_mpeg1_data(bmpeg1_parser_handler *handler, unsigned stream_id, batom_t object)
{
    bmedia_stream_t stream = ((struct b_media_mpeg1_parser_handler *)handler)->stream;
    BSTD_UNUSED(stream_id);
    BDBG_OBJECT_ASSERT(stream, bmedia_stream_t);
    BDBG_MSG_TRACE(("b_media_filter_mpeg1_data>: %#lx 0x%02x:%#lx", (unsigned long)stream, stream_id, (unsigned long)object));
    batom_pipe_push(stream->pipe_pes, object);
    return bmpeg1_parser_action_none;
}

#if B_HAS_FLV
static bflv_parser_action 
b_media_filter_flv_data(bflv_parser_handler *handler, batom_t object, uint8_t tag_meta)
{
    bmedia_stream_t stream = ((struct b_media_flv_parser_handler *)handler)->stream;
    BSTD_UNUSED(tag_meta);
    BDBG_MSG_TRACE(("b_media_filter_flv_data>: %p 0x%02x:%02x:%p:%u", (void *)stream, handler->tag_type, tag_meta, (void *)object,object?(unsigned)batom_len(object):0));
    BDBG_OBJECT_ASSERT(stream, bmedia_stream_t);
    batom_pipe_push(stream->pipe_pes, object);
    return bflv_parser_action_none;
}
#endif

static bogg_parser_action 
b_media_filter_ogg_data(bogg_parser_handler *handler, batom_t object)
{
    bmedia_stream_t stream = ((struct b_media_ogg_parser_handler *)handler)->stream;
    BDBG_MSG_TRACE(("b_media_filter_ogg_data>: %p %p:%u", (void *)stream, (void *)object,object?(unsigned)batom_len(object):0));
    BDBG_OBJECT_ASSERT(stream, bmedia_stream_t);
    batom_pipe_push(stream->pipe_pes, object);
    return bogg_parser_action_none;
}


bmedia_stream_t 
bmedia_filter_stream_open(bmedia_filter_t filter, unsigned stream_id, uint16_t pes_id, const bmedia_stream_cfg *stream_cfg)
{
    bmedia_stream_t  stream;
    bmedia_pes_stream_cfg pes_stream_cfg;

    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    stream = BLST_SQ_FIRST(&filter->free_streams);
    bmedia_pes_default_stream_cfg(&pes_stream_cfg);
    if (!stream) {
        return NULL;
    }
    if(stream_cfg) {
        stream->stream_cfg = *stream_cfg;
    } else {
        stream->stream_cfg.single_pts = false;
        stream->stream_cfg.reorder_timestamps = true;
    }

    stream->stream_id = stream_id;
    switch(filter->stream_type) {
#if B_HAS_AVI
    case bstream_mpeg_type_avi:
    {
        unsigned i;

        BDBG_MSG(("bmedia_filter_open: %#lx mapping AVI stream %u to pes %#x",  (unsigned long)filter, stream_id, pes_id));

        stream->avi = bavi_demux_get_stream(filter->avi.demux, stream_id-1);
        if(!stream->avi) {
            BDBG_ERR(("bmedia_filter_stream_open: %#lx can't open AVI stream %u:%u", (unsigned long)filter, stream_id, pes_id));
            return NULL;
        }
        for(i=0;i<(sizeof(filter->cfg.avi)/sizeof(filter->cfg.avi[0]));i++){
            if(filter->cfg.avi[i].activate_avi_stream){
                filter->cfg.avi[i].activate_avi_stream(filter->cfg.application_cnxt, stream_id, filter->avi.parser, stream->avi);
            }
        }
        {
            bavi_stream_config avi_stream_config;
            bavi_demux_default_stream_cfg(&avi_stream_config);
            avi_stream_config.reorder_timestamps = stream->stream_cfg.reorder_timestamps;
            bavi_stream_activate(stream->avi, stream->pipe_pes, &avi_stream_config);
            bavi_stream_get_stream_cfg(stream->avi, &pes_stream_cfg);
        }
        break;
    }
#endif
#if B_HAS_ASF
    case bstream_mpeg_type_asf: 
        {
            basf_stream_cfg cfg;
            basf_stream_initialize(&cfg);
            BDBG_MSG(("bmedia_filter_open: %#lx mapping ASF stream %u to pes %#x",  (unsigned long)filter, stream_id, pes_id));
            cfg.reorder_timestamps = stream->stream_cfg.reorder_timestamps;
            if(filter->cfg.update_asf_stream_cfg) {
                filter->cfg.update_asf_stream_cfg(filter->cfg.application_cnxt, stream_id, &cfg);
            }
            stream->asf = basf_stream_attach(filter->asf.demux, stream->pipe_pes, stream_id, &cfg);
            if(!stream->asf) {
                BDBG_ERR(("bmedia_filter_stream_open: %#lx can't open ASF stream %u:%u", (unsigned long)filter, stream_id, pes_id));
                return NULL;
            }
            basf_stream_get_stream_cfg(stream->asf, &pes_stream_cfg);
        }
        break;
#endif
#if B_HAS_FLV
    case bstream_mpeg_type_flv:
        BDBG_MSG(("bmedia_filter_open: %#lx mapping FLV stream %u to pes %#x",  (unsigned long)filter, stream_id, pes_id));
        stream->flv.stream = stream;
        bflv_parser_install_handler(filter->flv.parser, &stream->flv.handler, stream_id, b_media_filter_flv_data);
        bflv_parser_get_stream_cfg(filter->flv.parser, &pes_stream_cfg);
        break;
#endif
#if B_HAS_RMFF
    case bstream_mpeg_type_rmff:
        BDBG_MSG(("bmedia_filter_open: %#lx mapping RMFF stream %u to pes %#x",  (unsigned long)filter, stream_id, pes_id));
        stream->rmff = brmff_demux_get_stream(filter->rmff.demux, stream_id);
        if(!stream->rmff) {
            BDBG_ERR(("bmedia_filter_stream_open: %#lx can't open RMFF stream %u:%u", (unsigned long)filter, stream_id, pes_id));
            return NULL;
        }
        {
            brmff_stream_config rmff_stream_config;
            brmff_demux_default_stream_cfg(&rmff_stream_config);
            brmff_stream_activate(stream->rmff, stream->pipe_pes, &rmff_stream_config);
            brmff_stream_get_stream_cfg(stream->rmff, &pes_stream_cfg);
        }
        break;
#endif
    case bstream_mpeg_type_flac:
        BDBG_MSG(("bmedia_filter_open: %#lx mapping FLAC stream %u to pes %#x",  (unsigned long)filter, stream_id, pes_id));
        filter->flac.stream = stream;
        pes_stream_cfg.vecs =  filter->flac.vecs;
        pes_stream_cfg.nvecs = sizeof(filter->flac.vecs)/sizeof(filter->flac.vecs[0]);
        break;
    case bstream_mpeg_type_ogg:
        BDBG_MSG(("bmedia_filter_open: %#lx mapping OGG stream %u to pes %#x",  (unsigned long)filter, stream_id, pes_id));
        stream->ogg.stream = stream;
        bogg_parser_install_handler(filter->ogg.parser, &stream->ogg.handler, stream_id, b_media_filter_ogg_data);
        bogg_parser_get_stream_cfg(filter->ogg.parser, &pes_stream_cfg);
        break;
    case bstream_mpeg_type_amr:
        BDBG_MSG(("bmedia_filter_open: %#lx mapping AMR stream %u to pes %#x",  (unsigned long)filter, stream_id, pes_id));
        bogg_parser_get_stream_cfg(filter->ogg.parser, &pes_stream_cfg);
        break;
    case bstream_mpeg_type_mpeg1:
        BDBG_MSG(("bmedia_filter_open: %#lx mapping MPEG1 stream %u to pes %#x",  (unsigned long)filter, stream_id, pes_id));
        stream->mpeg1.stream = stream;
        bmpeg1_parser_install_handler(filter->mpeg1.parser, &stream->mpeg1.handler, stream_id, 0xFFFF, b_media_filter_mpeg1_data);
        bmedia_pes_default_stream_cfg(&pes_stream_cfg);
        break;
    case bstream_mpeg_type_wav:
        bwav_filter_get_stream_cfg(filter->wav.filter, &pes_stream_cfg);
        break; 
    case bstream_mpeg_type_aiff:
        baiff_filter_get_stream_cfg(filter->aiff.filter, &pes_stream_cfg);
        break; 
    case bstream_mpeg_type_mp4_fragment:
        {
            bmp4_fragment_stream_cfg cfg;
            bmp4_fragment_demux_get_default_stream_cfg(filter->mp4_fragment.demux, &cfg);
            stream->mp4_fragment = bmp4_fragment_stream_create(filter->mp4_fragment.demux, &cfg, stream_id, stream->pipe_pes);
            if(!stream->mp4_fragment) {
                BDBG_ERR(("bmedia_filter_stream_open: %#lx can't open BMP4_FRAGMENT stream %u:%u", (unsigned long)filter, stream_id, pes_id));
                return NULL;
            }
            bmp4_fragment_get_stream_cfg(stream->mp4_fragment, &pes_stream_cfg);
        }
        break; /* NOOP */
    case bstream_mpeg_type_mp4:
    case bstream_mpeg_type_ts:
    case bstream_mpeg_type_pes:
    case bstream_mpeg_type_ape:
        BDBG_MSG(("bmedia_filter_open: %#lx bypassing filter",  (unsigned long)filter));
        pes_id = 0;
        stream->type = b_media_stream_es;
        break;
    default:
        BDBG_WRN(("bmedia_filter_stream_open: %#lx not supported format %u", (unsigned long)filter, (unsigned)filter->stream_type));
        return NULL;
    }
    if(pes_id>=2) {
        pes_stream_cfg.single_pts = stream->stream_cfg.single_pts;
        bmedia_pes_start(stream->pes, filter->pipe_out, &pes_stream_cfg, pes_id);
        stream->type = b_media_stream_pes;
    } else {
        bmedia_es_start(stream->es, filter->pipe_out, &pes_stream_cfg, pes_id==0x01);
        stream->type = b_media_stream_es;
    }
    BDBG_OBJECT_SET(stream, bmedia_stream_t);
    BLST_SQ_REMOVE_HEAD(&filter->free_streams, link);
    return stream;
}

void 
bmedia_filter_stream_close(bmedia_filter_t filter, bmedia_stream_t stream)
{
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    BDBG_OBJECT_ASSERT(stream, bmedia_stream_t);
    b_media_filter_stream_close(filter, stream);
    BLST_SQ_INSERT_HEAD(&filter->free_streams, stream, link);
    return;
}

static void
bmedia_filter_reset_sync(bmedia_filter_t filter)
{
    if (filter->cfg.sync_streams_after_seek) {
        unsigned i;
        BDBG_MSG_TRACE(("bmedia_filter_reset_sync: Resetting sync"));
        filter->sync_complete = false;
        filter->sync_after_pts = B_MEDIA_INVALID_PTS;
        for(i=0;i<sizeof(filter->streams)/sizeof(*filter->streams);i++) {
            filter->streams[i].sync_complete = false;
            filter->streams[i].sync_last_pts = B_MEDIA_INVALID_PTS;
        }
    }
}

#if B_HAS_AVI || B_HAS_ASF

static uint32_t 
b_media_filter_sync_next_key_frame(bmedia_filter_t filter, bmedia_stream_t stream)
{
    batom_t atom = NULL;
    batom_t meta_atom = NULL;
    const bmedia_packet_header *hdr;
    unsigned header_type;
    bool pts_valid=false;
    uint32_t pts=B_MEDIA_INVALID_PTS;

    BSTD_UNUSED(header_type);

    /* Peek the next frame on the queue */
    atom=batom_pipe_peek(stream->pipe_pes);
    if (atom) {
        hdr = batom_userdata(atom);
        BDBG_ASSERT(hdr);
        header_type = hdr->header_type&(~B_MEDIA_PACKET_FLAG_EOS);
        pts_valid = hdr->pts_valid && !hdr->meta_header && (hdr->pts != B_MEDIA_INVALID_PTS);
        pts = pts_valid ? hdr->pts : B_MEDIA_INVALID_PTS;

        /* There are a few different situations to consider here when deciding if we should drop a frame.  We drop the 
           frame if any one of the conditions below are met:
           
           A. If this is not a key frame

           B. One or more streams has not yet found a valid frame and does not have a value set in stream->sync_last_pts
              In this case we must drop frames (one at a time) from every stream's pes pipe until we will find a frame in
              each stream. We also drop the frame if either the PTS is invalid or it matchs the PTS of this frame is the 
              same as the last time we exected the loop.  This check allows us to drop frames one at a time until 
              a point is reached where we have a PTS fro each stream.

           C. If we have found a frame for every stream and sync_after_pts is set, we still need to verify that any frames that
              would be sent have a PTS farther in time than filter->sync_after_pts.  If the PTS is before this time the frame 
              must be dropped. Also if the PTS or is not valid and we droped the last frame, then this frame should be 
              dropped as well.

           D. If the master stream has not finished syncing (started actually sending frames from a given PTS) then we must
              continue to drop frames one at a time for this stream.  The master stream may only be able to send a specific 
              points in the stream (key frames) which may be far apart. Only after a key frame and the master stream syncs 
              will we be able to start sending frames for all streams.
        */
        while ((!hdr->key_frame) ||                                                                                                /* case A */
               ((filter->sync_after_pts == B_MEDIA_INVALID_PTS) && 
                        (!pts_valid || (pts == stream->sync_last_pts))) ||                                                         /* case B */
               ((filter->sync_after_pts != B_MEDIA_INVALID_PTS) && 
                        ((pts < filter->sync_after_pts) || (!pts_valid && (stream->sync_last_pts < filter->sync_after_pts)))) ||   /* case C */
               (!filter->master_stream->sync_complete && (pts == stream->sync_last_pts))) {                                        /* case D */

            /* This frame contains no useful information.  Drop it and get another. */
            batom_pipe_drop(stream->pipe_pes);

            /* If this is a meta atom store it for now and only drop it if we drop the frame that follows */
            if (hdr->meta_header) {
                if (meta_atom) {
                    BDBG_MSG_TRACE(("b_media_filter_sync_next_key_frame: Stream %d releasing meta atom because a second one was found. This should not happen", stream->stream_id));
                    batom_release(meta_atom);
                }
                meta_atom = atom;
                BDBG_MSG_TRACE(("b_media_filter_sync_next_key_frame: Stream %d found meta %#lx", stream->stream_id, (unsigned long)meta_atom));
            } else {
                BDBG_MSG_TRACE(("b_media_filter_sync_next_key_frame: Stream %d **DROPPING %s**:", stream->stream_id, meta_atom ? "META+FRAME" : "FRAME"));
                BDBG_MSG_TRACE(("\t**FRAME INFO: key frame=%d, pts=%#x(valid=%d), atom=%#lx", hdr->key_frame, pts, pts_valid, (unsigned long)atom));
                BDBG_MSG_TRACE(("\t**STREAM INFO: last key PTS=%#x, master=%d", stream->sync_last_pts, stream==filter->master_stream));
                BDBG_MSG_TRACE(("\t**SYNC INFO: meta=%#lx, sync_after_pts=%#x, user_sync_after_pts=%#x, master complete=%d", 
                     (unsigned long)meta_atom, filter->sync_after_pts, filter->user_sync_after_pts, filter->master_stream->sync_complete));

                if (meta_atom) {
                    /* There was a meta atom stored.  Since we are dropping the frame, we can now drop the meta atom. */
                    BDBG_MSG_TRACE(("b_media_filter_sync_next_key_frame: Stream %d releasing meta atom because its frame is being dropped", stream->stream_id));
                    batom_release(meta_atom);
                    meta_atom = NULL;
                }
                batom_release(atom);
            }
            atom=batom_pipe_peek(stream->pipe_pes);
            if (atom) {
                hdr = batom_userdata(atom);
                BDBG_ASSERT(hdr);
                header_type = hdr->header_type&(~B_MEDIA_PACKET_FLAG_EOS);
                pts_valid = hdr->pts_valid && !hdr->meta_header && (hdr->pts != B_MEDIA_INVALID_PTS);
                pts = pts_valid ? hdr->pts : B_MEDIA_INVALID_PTS;
            } else {
                if (meta_atom) {
                    BDBG_MSG_TRACE(("b_media_filter_sync_next_key_frame: Stream %d releasing meta atom because there is no frame", stream->stream_id));
                    batom_release(meta_atom);
                    meta_atom = NULL;
                }
                break;
            }
        }
    }

    /* If we still have a stored meta atom, push it back to the from of the pipe */
    if (meta_atom){
        batom_t move_atom = NULL;

        BDBG_MSG_TRACE(("b_media_filter_sync_next_key_frame: Stream %d has meta atom %#lx being returned to the queue", stream->stream_id, (unsigned long)meta_atom));

        /* Ensure the sync queue is empty */
        batom_pipe_flush(stream->pipe_sync);

        /* Move all other atoms to the temporary sync pipe, after the meta atom */
        while ((move_atom = batom_pipe_pop(stream->pipe_pes)) != NULL) {
            batom_pipe_push(stream->pipe_sync, move_atom);
        }

        batom_pipe_push(stream->pipe_pes, meta_atom);

        /* Move all atoms back to the pes pipe */
        while ((move_atom = batom_pipe_pop(stream->pipe_sync))) {
            batom_pipe_push(stream->pipe_pes, move_atom);
        }
    }

    return (atom!=NULL && pts_valid) ? pts : B_MEDIA_INVALID_PTS;
}

static void
b_media_filter_sync(bmedia_filter_t filter)
{
    unsigned i;
    bool stream_updated = false;
    bmedia_stream_t stream;
    uint32_t key_frame_pts;
    uint32_t num_completed_streams = 0;

    if (!filter->cfg.sync_streams_after_seek || filter->key_frame_only || filter->sync_complete) {
        return;
    } else if (!filter->master_stream) {
        /* If we don't know the master stream, return */
        BDBG_ERR(("b_media_filter_sync: Syncing enabled, but it can't work without a master stream!"));
        return;
    }

    do {
        stream_updated = false;

        for(i=0;i<sizeof(filter->streams)/sizeof(*filter->streams);i++) {
            stream = &filter->streams[i];

            /* If we have a frame with a valid header and a valid PTS which is after filter->sync_after_pts, then this stream is sync'ed.  No need to check it anymore */
            stream->sync_complete = ((filter->sync_after_pts != B_MEDIA_INVALID_PTS) &&
                                     (filter->sync_after_pts <= stream->sync_last_pts) && 
                                     ((filter->master_stream == stream) || filter->master_stream->sync_complete));

            /* If this stream is not being converted or if it is already sync'ed, then no need to continue */
            if (stream->sync_complete || ((stream->type != b_media_stream_pes) && (stream->type != b_media_stream_es))) {
                num_completed_streams++;
                continue;
            }

            key_frame_pts = b_media_filter_sync_next_key_frame(filter, stream);
            if (key_frame_pts != B_MEDIA_INVALID_PTS) {
                /* Set a flag to indicate that some information has changed for this stream only if this is a different PTS. We will need to recheck all the streams. */
                stream_updated = (stream->sync_last_pts != key_frame_pts);
                stream->sync_last_pts = key_frame_pts;

                BDBG_MSG_TRACE(("b_media_filter_sync: Stream %d found KEY FRAME at PTS %#x", stream->stream_id, key_frame_pts));
                BDBG_MSG_TRACE(("\t**STREAM INFO: last key PTS=%#x, master=%d", stream->sync_last_pts, stream==filter->master_stream));
                BDBG_MSG_TRACE(("\t**SYNC INFO: sync_after_pts=%#x, user_sync_after_pts=%#x, master complete=%d", 
                     filter->sync_after_pts, filter->user_sync_after_pts, filter->master_stream->sync_complete));
            }
            else {
                BDBG_MSG_TRACE(("b_media_filter_sync: Stream %d all current frames dropped without finding a key frame", stream->stream_id));
            }
        }

        /* After updating each stream->sync_last_pts, the master stream must go through the list of streams and see if any are still set to INVALID_SYNC_PTS. 
           If so, this means that a frame has not yet been found for that stream and we must continue dropping frames. */
        if ((filter->sync_after_pts == B_MEDIA_INVALID_PTS) && batom_pipe_peek(filter->master_stream->pipe_pes)) {
            unsigned highest_pts = B_MEDIA_INVALID_PTS;
            unsigned lowest_pts = B_MEDIA_INVALID_PTS;

            /* This loop must be done inside the main loop because we need to know if filter->sync_after_pts is valid before updating stream->sync_complete*/
            for(i=0;i<sizeof(filter->streams)/sizeof(*filter->streams);i++) {
                stream = &filter->streams[i];
                if ((stream->type != b_media_stream_pes) && (stream->type != b_media_stream_es))
                    continue;

                /* If any stream has not found a frame, quite searching */
                if (stream->sync_last_pts == B_MEDIA_INVALID_PTS) {
                    lowest_pts = highest_pts = B_MEDIA_INVALID_PTS;
                    BDBG_MSG_TRACE(("b_media_filter_sync: Stream %d still has not found a key frame pts=0x%08x", stream->stream_id, stream->sync_last_pts));
                    break;
                }
                if ((highest_pts == B_MEDIA_INVALID_PTS) || (stream->sync_last_pts > highest_pts))
                    highest_pts = stream->sync_last_pts;
                if ((lowest_pts == B_MEDIA_INVALID_PTS) || (stream->sync_last_pts < lowest_pts))  
                    lowest_pts = stream->sync_last_pts;
            }

            if ((lowest_pts != B_MEDIA_INVALID_PTS) && (highest_pts != B_MEDIA_INVALID_PTS)) {
                if (filter->user_sync_after_pts <= highest_pts) {
                    /* For now, just sync to the master PTS */
                    filter->sync_after_pts = filter->master_stream->sync_last_pts;
                    BDBG_MSG_TRACE(("b_media_filter_sync: Resync after PTS %#x", filter->sync_after_pts));
                }
                /* Run an up date if the sync after pts has been set */
                stream_updated = true;
            }
        }

        /* Check to see if the the master stream is now complete */
        filter->master_stream->sync_complete = ((filter->sync_after_pts != B_MEDIA_INVALID_PTS) &&
                                              (filter->sync_after_pts <= filter->master_stream->sync_last_pts));

        /* If either a frame was dropped or a stream presentation time was updated, we need to restart the loop. This
           will allow us to advance each stream one frame at a time if needed so we lose as little data as possibile. */
    }while (stream_updated);

    if (num_completed_streams == sizeof(filter->streams)/sizeof(*filter->streams)) {
        filter->sync_complete = true;
    }
}
#endif /* B_HAS_AVI || B_HAS_ASF */

int
bmedia_filter_seek(bmedia_filter_t filter, int64_t off)
{
    int result = -1;
#if B_HAS_AVI || B_HAS_ASF || B_HAS_RMFF
    bool discontinuity_detected = false;
#endif

    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    BSTD_UNUSED(off);
    
    switch(filter->stream_type) {
#if B_HAS_AVI
    case bstream_mpeg_type_avi:
        {
            bavi_parser_status status;	    	  
            bavi_parser_seek(filter->avi.parser, off);
            bavi_parser_get_status(filter->avi.parser, &status);
            discontinuity_detected = status.data_discontinuity;
            result = 0;
            break;
        }
#endif
#if B_HAS_ASF
    case bstream_mpeg_type_asf:
        {
            basf_parser_info info;
            basf_parser_seek(filter->asf.parser, off);
            basf_parser_get_info(filter->asf.parser, &info);
            discontinuity_detected = info.data_discontinuity;
            result = 0;
        }
        break;
#endif
#if B_HAS_FLV
    case bstream_mpeg_type_flv:
        break;
#endif
#if B_HAS_RMFF
    case bstream_mpeg_type_rmff:
        {
            brmff_parser_status status;
            brmff_parser_seek(filter->rmff.parser, off);
            brmff_parser_get_status(filter->rmff.parser, &status);
            discontinuity_detected = status.data_discontinuity;
            result = 0;
        }
        break;
#endif
    case bstream_mpeg_type_flac:
    case bstream_mpeg_type_ogg:
    case bstream_mpeg_type_amr:
    case bstream_mpeg_type_mpeg1:
        result = 0;
        break;
    default:
        break;

    case bstream_mpeg_type_wav:
        result = bwav_filter_seek(filter->wav.filter, off);
        break;

    case bstream_mpeg_type_aiff:
        result = baiff_filter_seek(filter->aiff.filter, off);
        break;

    case bstream_mpeg_type_mp4_fragment:
        result = bmp4_fragment_demux_seek(filter->mp4_fragment.demux, off);
        break;

    case bstream_mpeg_type_mp4:
    case bstream_mpeg_type_pes:
    case bstream_mpeg_type_ts:
    case bstream_mpeg_type_ape:
        result = 0;
        break;
    }

#if B_HAS_AVI || B_HAS_ASF
    if (discontinuity_detected) {
        bmedia_filter_reset_sync(filter);
    }
#endif

    return result;
}

static bool
b_media_filter_pes_feed(bmedia_filter_t filter)
{
    unsigned i;
    BDBG_MSG_TRACE(("b_media_filter_pes_feed>: %#lx", (unsigned long)filter));
    for(i=0;i<sizeof(filter->streams)/sizeof(*filter->streams);i++) {
        bool want_continue;
        switch(filter->streams[i].type) {
        case bstream_mpeg_type_ts:
        case b_media_stream_pes:
            want_continue = bmedia_pes_feed(filter->streams[i].pes, filter->streams[i].pipe_pes);
            break;
        case b_media_stream_es:
            want_continue = bmedia_es_feed(filter->streams[i].es, filter->streams[i].pipe_pes);
            break;
        default:
        case b_media_stream_none:
            want_continue = true;
        }
        if(!want_continue) {
            filter->feed_stall = true;
            BDBG_MSG_TRACE(("b_media_filter_pes_feed<: %#lx PAUSE:%#lx", (unsigned long)filter, (unsigned long)filter->streams[i].pes));
            return false;
        }
    }
    filter->feed_stall = false;
    BDBG_MSG_TRACE(("b_media_filter_pes_feed<: %#lx", (unsigned long)filter));
    return true;
}


bool 
bmedia_filter_feed(bmedia_filter_t filter, batom_pipe_t pipe)
{
    size_t result;

    BDBG_MSG_TRACE(("bmedia_filter_feed>: %#lx %#lx",  (unsigned long)filter, (unsigned long)pipe));
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    switch(filter->stream_type) {
#if B_HAS_AVI
    case bstream_mpeg_type_avi:
        if(!filter->feed_stall || b_media_filter_pes_feed(filter)) {
            if(bavi_demux_step(filter->avi.demux)) {
                result = bavi_parser_feed(filter->avi.parser, pipe);
                b_media_filter_sync(filter);
                BDBG_MSG_TRACE(("bmedia_filter_feed: avi %p result %u", (void *)filter, (unsigned)result));
                bavi_demux_step(filter->avi.demux);
                return b_media_filter_pes_feed(filter);
            }
        }
        return false;
#endif
#if B_HAS_ASF
    case bstream_mpeg_type_asf:
        if(!filter->feed_stall || b_media_filter_pes_feed(filter)) {
            result = basf_parser_feed(filter->asf.parser, pipe);
            b_media_filter_sync(filter);
            BDBG_MSG_TRACE(("bmedia_filter_feed: asf %p result %u", (void *)filter, (unsigned)result));
            return b_media_filter_pes_feed(filter);
        }
        return false;
#endif
#if B_HAS_FLV
    case bstream_mpeg_type_flv:
        if(!filter->feed_stall || b_media_filter_pes_feed(filter)) {
            result = bflv_parser_feed(filter->flv.parser, pipe);
            BDBG_MSG_TRACE(("bmedia_filter_feed: flv %p result %u", (void *)filter, (unsigned)result));
            return b_media_filter_pes_feed(filter);
        }
        return false;
#endif
#if B_HAS_RMFF
    case bstream_mpeg_type_rmff:
        if(!filter->feed_stall || b_media_filter_pes_feed(filter)) {
            if(brmff_demux_step(filter->rmff.demux)) {
                result = brmff_parser_feed(filter->rmff.parser, pipe);
                BDBG_MSG_TRACE(("bmedia_filter_feed: rmff %p result %u",  (void *)filter, (unsigned)result));
                brmff_demux_step(filter->rmff.demux);
                return b_media_filter_pes_feed(filter);
            }
        }
        return false;
#endif
    case bstream_mpeg_type_ogg:
        if(!filter->feed_stall || b_media_filter_pes_feed(filter)) {
            result = bogg_parser_feed(filter->ogg.parser, pipe);
            BDBG_MSG_TRACE(("bmedia_filter_feed: ogg %p result %u",  (void *)filter, (unsigned)result));
            return b_media_filter_pes_feed(filter);
        }
        return false;
    case bstream_mpeg_type_amr:
        if(!filter->feed_stall || b_media_filter_pes_feed(filter)) {
            result = bamr_parser_feed(filter->amr.parser, pipe);
            BDBG_MSG_TRACE(("bmedia_filter_feed: amr %p result %u",  (void *)filter, (unsigned)result));
            return b_media_filter_pes_feed(filter);
        }
        return false;
    case bstream_mpeg_type_flac:
        if(!filter->feed_stall || b_media_filter_pes_feed(filter)) {
            result = bflac_parser_feed(filter->flac.parser, pipe);
            BDBG_MSG_TRACE(("bmedia_filter_feed: flac %p result %u",  (void *)filter, (unsigned)result));
            return b_media_filter_pes_feed(filter);
        }
        return false;
    case bstream_mpeg_type_mpeg1:
        if(!filter->feed_stall || b_media_filter_pes_feed(filter)) {
            result = bmpeg1_parser_feed(filter->mpeg1.parser, pipe);
            BDBG_MSG_TRACE(("bmedia_filter_feed: mpeg1 %p result %u",  (void *)filter, (unsigned)result));
            return b_media_filter_pes_feed(filter);
        }
        return false;
    case bstream_mpeg_type_wav:
        if(!filter->feed_stall || b_media_filter_pes_feed(filter)) {
            result = bwav_filter_feed(filter->wav.filter, pipe, filter->streams[0].pipe_pes); /* return data on first PES */
            BDBG_MSG_TRACE(("bmedia_filter_feed: wav %p result %u",  (void *)filter, (unsigned)result));
            return b_media_filter_pes_feed(filter);
        }
        return false;
    case bstream_mpeg_type_aiff:
        if(!filter->feed_stall || b_media_filter_pes_feed(filter)) {
            result = baiff_filter_feed(filter->aiff.filter, pipe, filter->streams[0].pipe_pes); /* return data on first PES */
            BDBG_MSG_TRACE(("bmedia_filter_feed: aiff %p result %u",  (void *)filter, (unsigned)result));
            return b_media_filter_pes_feed(filter);
        }
        return false;
    case bstream_mpeg_type_mp4_fragment:
        if(!filter->feed_stall || b_media_filter_pes_feed(filter)) {
            result = bmp4_fragment_demux_feed(filter->mp4_fragment.demux, pipe); /* return data on first PES */
            BDBG_MSG_TRACE(("bmedia_filter_feed: mp4_fragment %p result %u",  (void *)filter, (unsigned)result));
            return b_media_filter_pes_feed(filter) && result>0;
        }
        return false;
    case bstream_mpeg_type_mp4:
    case bstream_mpeg_type_pes:
    case bstream_mpeg_type_ts:
    case bstream_mpeg_type_ape:
    default:
        {
            batom_t atom;

            while(NULL!=(atom=batom_pipe_pop(pipe))) {
                batom_pipe_push(filter->pipe_out,atom);
            }
        }
        break;
    }
    return true;
}

void 
bmedia_filter_get_status(bmedia_filter_t filter, bmedia_filter_status *status)
{
    unsigned i;

    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    BDBG_ASSERT(status);

    BMEDIA_PARSING_ERRORS_INIT(&status->errors);
    status->last_pts_valid = false;
    status->last_pts = B_MEDIA_INVALID_PTS;

    BDBG_MSG(("bmedia_filter_get_status: %p pipe_out:%#lx %s", (void *)filter, (unsigned long)batom_pipe_peek(filter->pipe_out), filter->feed_stall?"STALL":""));
    for(i=0;i<sizeof(filter->streams)/sizeof(*filter->streams);i++) {
        if(filter->streams[i].type==b_media_stream_none) {
            continue;
        }
        if(!status->last_pts_valid && filter->streams[i].type == b_media_stream_pes) {
           bmedia_pes_status pes_status;
           bmedia_pes_get_status(filter->streams[i].pes, &pes_status);
           status->last_pts_valid = pes_status.last_pts_valid;
           status->last_pts = pes_status.last_pts;
        }
        BDBG_MSG(("bmedia_filter_get_status: %p stream:%u pipe_pes:%#lx", (void *)filter, i, (unsigned long)batom_pipe_peek(filter->streams[i].pipe_pes)));
    }
    switch(filter->stream_type) {
#if B_HAS_AVI
    case bstream_mpeg_type_avi:
        {
            bavi_parser_status avi_status;

            bavi_parser_get_status(filter->avi.parser, &avi_status);
            status->offset = avi_status.offset;
            status->acc_length = avi_status.acc_length;
            status->obj_length = avi_status.obj_length;
            status->state = avi_status.state;
            status->errors = avi_status.errors;
            return;
        }
#endif
#if B_HAS_ASF
    case bstream_mpeg_type_asf:
        {
            basf_parser_info asf_info;
            basf_demux_status asf_status;

            basf_demux_status_get(filter->asf.demux, &asf_status); /* call it just to get debug log from the basf_demux_status_get */
            basf_parser_get_info(filter->asf.parser, &asf_info);
            status->offset = asf_info.offset;
            status->acc_length = 0;
            status->obj_length = 0;
            status->state = "";
            status->errors = asf_info.errors;
            return;
        }
#endif
#if B_HAS_RMFF
    case bstream_mpeg_type_rmff:
        {
            brmff_parser_status rmff_status;

            brmff_parser_get_status(filter->rmff.parser, &rmff_status);
            status->offset = rmff_status.offset;
            status->acc_length = rmff_status.acc_length;
            status->obj_length = rmff_status.obj_length;
            status->state = rmff_status.state;
            status->errors = rmff_status.errors;
            return;
        }
#endif
    case bstream_mpeg_type_mp4_fragment:
    case bstream_mpeg_type_wav:
    case bstream_mpeg_type_aiff:
    case bstream_mpeg_type_flv:
    case bstream_mpeg_type_ogg:
    case bstream_mpeg_type_amr:
    case bstream_mpeg_type_flac:
    case bstream_mpeg_type_mpeg1:
    case bstream_mpeg_type_mp4:
    case bstream_mpeg_type_pes:
    case bstream_mpeg_type_ape:
    default:
        break;
    }
    status->offset = 0;
    status->acc_length = 0;
    status->obj_length = 0;
    status->state = "";
    return;
}


void 
bmedia_filter_set_offset(bmedia_filter_t filter, uint32_t timestamp, uint16_t  stream_id)
{
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    BSTD_UNUSED(timestamp);
    BSTD_UNUSED(stream_id);
   
    switch(filter->stream_type) {
#if B_HAS_AVI
    case bstream_mpeg_type_avi: 
        {
            unsigned i;
            for(i=0;i<sizeof(filter->streams)/sizeof(*filter->streams);i++) {
                if(!filter->streams[i].avi) {
                    continue;
                }
                if(filter->streams[i].stream_id == stream_id) {
                    bavi_demux_set_offset(filter->streams[i].avi, timestamp);
                    break;
                }
                if(stream_id == 0) {
                    bavi_demux_set_offset(filter->streams[i].avi, timestamp);
                }
            }
        }
        break;
#endif
#if B_HAS_RMFF
    case bstream_mpeg_type_rmff:
        {
            unsigned i;
            if(stream_id == BRMFF_MASTER_STREAM) {
                if(filter->rmff.parser) {
                    brmff_parser_set_first_packet(filter->rmff.parser, timestamp); /* reusing timestamp field to carry the packet counter */
                }
                break;
            }
            for(i=0;i<sizeof(filter->streams)/sizeof(*filter->streams);i++) {
                if(!filter->streams[i].rmff) {
                    continue;
                }
                if(filter->streams[i].stream_id == stream_id) {
                    brmff_stream_set_first_packet(filter->streams[i].rmff, timestamp); /* reusing timestamp field to carry the packet counter */
                    break;
                }
            }
        }
        break;
#endif
    case bstream_mpeg_type_asf:
    case bstream_mpeg_type_flv:
    case bstream_mpeg_type_ogg:
    case bstream_mpeg_type_amr:
    case bstream_mpeg_type_flac:
    case bstream_mpeg_type_mpeg1:
    case bstream_mpeg_type_wav:
    case bstream_mpeg_type_aiff:
    case bstream_mpeg_type_mp4_fragment:
    case bstream_mpeg_type_ape:
    default:
        break;
    }
    return;
}


void 
bmedia_filter_set_timescale(bmedia_filter_t filter, bmedia_time_scale time_scale)
{
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    BSTD_UNUSED(time_scale);
    filter->time_scale = time_scale;
    switch(filter->stream_type) {
#if B_HAS_AVI
    case bstream_mpeg_type_avi:
        bavi_demux_set_timescale(filter->avi.demux, time_scale);
        break;
#endif
#if B_HAS_ASF
    case bstream_mpeg_type_asf:
        basf_demux_set_timescale(filter->asf.demux, time_scale);
        break;
#endif
#if B_HAS_RMFF
    case bstream_mpeg_type_rmff:
        brmff_demux_set_timescale(filter->rmff.demux, time_scale);
        break;
#endif
#if B_HAS_FLV
    case bstream_mpeg_type_flv:
#endif
    case bstream_mpeg_type_ogg:
    case bstream_mpeg_type_amr:
    case bstream_mpeg_type_flac:
    case bstream_mpeg_type_wav:
    case bstream_mpeg_type_aiff:
    case bstream_mpeg_type_mp4_fragment:
    case bstream_mpeg_type_mpeg1:
    case bstream_mpeg_type_pes:
    case bstream_mpeg_type_mp4:
    case bstream_mpeg_type_ape:
    default:
        break;
    }
    return;
}

void 
bmedia_filter_set_keyframeonly(bmedia_filter_t filter, bool key_frame_only)
{
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    BSTD_UNUSED(key_frame_only);
    switch(filter->stream_type) {
#if B_HAS_ASF
    case bstream_mpeg_type_asf:
        basf_demux_set_keyframeonly(filter->asf.demux, key_frame_only);
        break;
#endif
#if B_HAS_RMFF
    case bstream_mpeg_type_rmff:
        brmff_demux_set_keyframeonly(filter->rmff.demux, key_frame_only);
        break;
#endif
    default:
        break;
    }
    filter->key_frame_only = key_frame_only;
    return;
}

void 
bmedia_filter_sync_after_time(bmedia_filter_t filter, uint32_t time)
{
    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    filter->user_sync_after_pts = bmedia_time2pts(time, BMEDIA_TIME_SCALE_BASE);
    BDBG_MSG_TRACE(("bmedia_filter_sync_after_time: User specified resync after PTS %#x", filter->user_sync_after_pts));
    if (filter->user_sync_after_pts != 0) {
        bmedia_filter_reset_sync(filter);
    }
}

void 
bmedia_filter_set_master_stream(bmedia_filter_t filter, uint32_t stream_id)
{
    uint32_t i;

    BDBG_OBJECT_ASSERT(filter, bmedia_filter_t);
    BDBG_MSG_TRACE(("bmedia_filter_set_master_stream: User specified resync with stream id %d as the master", stream_id));
    for(i=0;i<sizeof(filter->streams)/sizeof(*filter->streams);i++) {
        /* If this stream is not being converted or if it is already sync'ed, then no need to continue */
        if ((filter->streams[i].stream_id == stream_id) && (filter->streams[i].type != b_media_stream_none)) {
            filter->master_stream = &filter->streams[i];
        }
    }
}


void 
bmedia_filter_stream_get_config(bmedia_stream_t stream, bmedia_stream_cfg *cfg)
{
    BDBG_OBJECT_ASSERT(stream, bmedia_stream_t);
    BDBG_ASSERT(cfg);
    *cfg = stream->stream_cfg;
    return;
}

int
bmedia_filter_stream_set_config( bmedia_stream_t stream, const bmedia_stream_cfg *cfg)
{
    BDBG_OBJECT_ASSERT(stream, bmedia_stream_t);
    BDBG_ASSERT(cfg);

    if(stream->type == b_media_stream_pes) {
        bmedia_pes_stream_cfg pes_cfg;

        bmedia_pes_get_stream_cfg(stream->pes, &pes_cfg);
        pes_cfg.single_pts = cfg->single_pts;
        bmedia_pes_set_stream_cfg(stream->pes, &pes_cfg);
    }
    stream->stream_cfg = *cfg;
    return 0;
}

