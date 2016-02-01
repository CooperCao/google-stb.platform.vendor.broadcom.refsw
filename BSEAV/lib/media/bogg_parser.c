/***************************************************************************
 *     Copyright (c) 2011 Broadcom Corporation
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
 * OGG parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bogg_parser.h"
#include "bkni.h"

BDBG_MODULE(bogg_parser);
#define BDBG_MSG_TRACE(x)   BDBG_MSG(x)

#define B_OGG_VEC_BCMA_FRAME	0

typedef enum b_ogg_parser_state {
    b_ogg_parser_state_parse, /* parse OGG structure */
    b_ogg_parser_state_discard, /* discard current object */
    b_ogg_parser_state_resync /* resync with the stream */
} b_ogg_parser_state;

BDBG_OBJECT_ID(bogg_parser_t);
typedef enum {
    b_ogg_stream_type_vorbis,
    b_ogg_stream_type_opus,
    b_ogg_stream_type_unknown
} b_ogg_stream_type;

typedef struct b_ogg_stream_handler {
    BLST_S_ENTRY(b_ogg_stream_handler) link; /* field that is used to link handlers together */
    bogg_parser_handler *handler;
    b_ogg_stream_type type;
    batom_accum_t accum;
    union {
        struct {
            batom_t headers[3];
            void *meta;
            size_t meta_size;
            unsigned sample_rate;
        } vorbis;
        struct {
            void *meta;
            size_t meta_size;
            unsigned sample_rate;
        } opus;
    } data;
} b_ogg_stream_handler;

struct bogg_parser {
    BDBG_OBJECT(bogg_parser_t)
    batom_accum_t acc;
    batom_factory_t factory;
    BLST_S_HEAD(b_ogg_object_handlers, bogg_parser_handler) object_handlers;
    BLST_S_HEAD(b_ogg_stream_handlers, b_ogg_stream_handler) stream_handlers; /* there is one-to-one relation between object and stream handlers */
    const batom_vec *vecs[1];
    struct {
        b_ogg_parser_state state;
        size_t object; /* number of bytes to capture or discard */
        bogg_off_t acc_off; /* stream offset for accumulator */
        bogg_parser_handler *handler; /* handler for the current leaf object */
        bmedia_parsing_errors errors;
        bool data_discontinuity; /* set to true if some data from the stream has been dropped or lost since the last buffer was fed */
        bool parser_return;
    } state;
    bogg_parser_cfg cfg;
};



/* ogg object handlers, keep list sorted by the id, it allows O/2 search
 * complexity and ease handle of multiple handlers for the same id code */
void 
bogg_parser_install_handler(bogg_parser_t ogg, bogg_parser_handler *handler, uint32_t id, bogg_parser_handler_cb object_handler)
{
    bogg_parser_handler *cur, *prev;
    b_ogg_stream_handler *stream;
    b_ogg_stream_handler *stream_cur, *stream_prev;

    BDBG_OBJECT_ASSERT(ogg, bogg_parser_t);
    BDBG_MSG_TRACE(("bogg_parser_install_handler:> %#lx  %#lx %#x", (unsigned long)ogg, (unsigned long)handler, (unsigned)id));

    stream = BKNI_Malloc(sizeof(*stream));
    if(stream==NULL) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return;
    }
    BKNI_Memset(stream, 0, sizeof(*stream));
    stream->accum = batom_accum_create(ogg->factory);
    if(!stream->accum) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BKNI_Free(stream);
        return;
    }
    stream->type = b_ogg_stream_type_unknown;
    stream->handler = handler;

    for(prev=NULL, stream_prev=NULL, stream_cur = BLST_S_FIRST(&ogg->stream_handlers), cur = BLST_S_FIRST(&ogg->object_handlers); cur; stream_cur = BLST_S_NEXT(stream_cur, link), cur=BLST_S_NEXT(cur, link)) {
        BDBG_ASSERT(stream_cur);
        if (id<cur->id) {
            break;
        }
        prev = cur;
        stream_prev = stream_cur;
    }
    handler->id = id;
    handler->handler = object_handler;
    if (prev) {
        BLST_S_INSERT_AFTER(&ogg->object_handlers, prev, handler, link);
        BLST_S_INSERT_AFTER(&ogg->stream_handlers, stream_prev, stream, link);
    } else {
        BLST_S_INSERT_HEAD(&ogg->object_handlers, handler, link);
        BLST_S_INSERT_HEAD(&ogg->stream_handlers, stream, link);
    }
    BDBG_MSG_TRACE(("bogg_parser_install_handler:< %#lx  %#lx %#x", (unsigned long)ogg, (unsigned long)handler, id));
    return ;
}

static bool
b_ogg_vorbis_init(bogg_parser_t ogg, b_ogg_stream_handler *handler, const batom_cursor *payload, size_t packet_size, batom_cursor *spare_cursor)
{
    BDBG_MSG_TRACE(("b_ogg_vorbis_init: %#lx %#lx", (unsigned long)ogg, (unsigned long)handler));
    BATOM_CLONE(spare_cursor, payload);
    batom_cursor_skip(spare_cursor, packet_size);
    BKNI_Memset(&handler->data.vorbis, 0, sizeof(handler->data.vorbis));
    handler->data.vorbis.headers[0] = batom_accum_extract(ogg->acc, payload, spare_cursor, NULL, NULL);
    return handler->data.vorbis.headers[0]!=NULL;
}

static void b_ogg_vorbis_shutdown(bogg_parser_t ogg, b_ogg_stream_handler *stream)
{
    unsigned i;
    for(i=0;i<sizeof(stream->data.vorbis.headers)/sizeof(*stream->data.vorbis.headers);i++) {
        if(stream->data.vorbis.headers[i]) {
            batom_release(stream->data.vorbis.headers[i]);
            stream->data.vorbis.headers[i] = NULL;
        }
    }
    if(stream->data.vorbis.meta) {
        ogg->cfg.alloc->bmem_free(ogg->cfg.alloc, stream->data.vorbis.meta);
    }
    stream->data.vorbis.meta = NULL;
    stream->data.vorbis.meta_size = 0;
    return;
}


static void
b_ogg_vorbis_payload_handler(bogg_parser_t ogg, const bogg_page_header *page_header, b_ogg_stream_handler *stream, unsigned packet_no, const batom_cursor *payload, size_t packet_size, bool spanning, batom_cursor *spare_cursor)
{
    batom_t data;
    bvorbis_frame_header vorbis_frame_header;

    BDBG_MSG_TRACE(("b_ogg_vorbis_payload_handler: %#lx %#lx %s %s %s %s", (unsigned long)ogg, (unsigned long)stream, stream->data.vorbis.meta?"meta":"", stream->data.vorbis.headers[0]?"header 1":"", stream->data.vorbis.headers[1]?"header 3":"", stream->data.vorbis.headers[2]?"header 5":""));
    BATOM_CLONE(spare_cursor, payload);
    batom_cursor_skip(spare_cursor, packet_size);
    if(spanning || batom_accum_len(stream->accum)) {
        batom_accum_append(stream->accum, ogg->acc, payload, spare_cursor);
        if(spanning) {
            return;
        }
        data = batom_from_accum(stream->accum, NULL, NULL);
    } else {
        data = batom_accum_extract(ogg->acc, payload, spare_cursor, NULL, NULL);
    }
    if(!data) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto done;
    }
    if(stream->data.vorbis.meta) {
        bmedia_bcma_hdr hdr;
        batom_t  pes_atom;

        BMEDIA_PACKET_HEADER_INIT(&hdr.pes);
        if(packet_no==0) {
            unsigned timestamp;
            BDBG_ASSERT(stream->data.vorbis.sample_rate>0);
            timestamp = (page_header->granule_position * 1000) / stream->data.vorbis.sample_rate;

            BMEDIA_PES_SET_PTS(&hdr.pes, bmedia_time2pts(timestamp, BMEDIA_TIME_SCALE_BASE));
        }
        hdr.pes.header_off = 4;
        hdr.pes.header_type = B_OGG_VEC_BCMA_FRAME;
        bmedia_bcma_hdr_init(&hdr, batom_len(data));
        pes_atom = batom_from_range_and_atom(stream->data.vorbis.meta, stream->data.vorbis.meta_size, data, &bmedia_bcma_atom, &hdr.pes);
        batom_release(data);
        if(pes_atom) {
            stream->handler->handler(stream->handler, pes_atom);
        }
    } else {
        batom_cursor_from_atom(spare_cursor, data);
        if(!bvorbis_parse_frame_header(spare_cursor, &vorbis_frame_header)) {
            goto stream_error;
        }
        BDBG_MSG_TRACE(("b_ogg_vorbis_payload_handler: %#lx %#lx meta:%u", (unsigned long)ogg, (unsigned long)stream, vorbis_frame_header.packet_type));
        switch(vorbis_frame_header.packet_type) {
        case 3:
            if(stream->data.vorbis.headers[0] && stream->data.vorbis.headers[1]==NULL && stream->data.vorbis.headers[2]==NULL) {
                stream->data.vorbis.headers[1] = data;
                break;
            }
            goto stream_error;
        case 5:
            if(stream->data.vorbis.headers[0] && stream->data.vorbis.headers[1] && stream->data.vorbis.headers[2]==NULL) {
                struct bvorbis_indentification_header vorbis_indentification_header;
                bmedia_waveformatex_header wf;
                size_t meta_size;
                unsigned i;

                stream->data.vorbis.headers[2] = data;
                data = NULL;
                batom_cursor_from_atom(spare_cursor, stream->data.vorbis.headers[0]);
                if(!bvorbis_parse_frame_header(spare_cursor, &vorbis_frame_header)) {
                    goto stream_error;
                }
                if(vorbis_frame_header.packet_type!=1) {
                    goto stream_error;
                }
                if(!bvorbis_parse_indentification_header(spare_cursor, &vorbis_indentification_header)) {
                    goto stream_error;
                }
                if(vorbis_indentification_header.audio_sample_rate==0) {
                    goto stream_error;
                }
                bmedia_init_waveformatex(&wf);
                stream->data.vorbis.sample_rate = vorbis_indentification_header.audio_sample_rate;
                wf.nSamplesPerSec = vorbis_indentification_header.audio_sample_rate;
                wf.nChannels = vorbis_indentification_header.audio_channels;
                wf.wFormatTag = 0xFFFF;
                for(wf.cbSize=0,i=0;i<3;i++) {
                    wf.cbSize += batom_len(stream->data.vorbis.headers[i]) + sizeof(uint16_t);
                }
                meta_size = wf.cbSize + BMEDIA_WAVEFORMATEX_BASE_SIZE;
                stream->data.vorbis.meta = ogg->cfg.alloc->bmem_alloc(ogg->cfg.alloc, meta_size);
                if(!stream->data.vorbis.meta) {
                    (void)BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                    goto stream_error;
                }
                stream->data.vorbis.meta_size = meta_size;
                meta_size=bmedia_write_waveformatex(stream->data.vorbis.meta, &wf);
                BDBG_ASSERT(meta_size==BMEDIA_WAVEFORMATEX_BASE_SIZE);
                for(i=0;i<3;i++) {
                    batom_t hdr = stream->data.vorbis.headers[i];
                    size_t hdr_len = batom_len(hdr);
                    stream->data.vorbis.headers[i] = 0;
                    batom_cursor_from_atom(spare_cursor, hdr);
                    B_MEDIA_SAVE_UINT16_BE((uint8_t *)stream->data.vorbis.meta + meta_size, hdr_len);
                    meta_size += 2;
                    meta_size += batom_cursor_copy(spare_cursor, (uint8_t *)stream->data.vorbis.meta + meta_size, hdr_len);
                    batom_release(hdr);
                }
                BDBG_ASSERT(meta_size == stream->data.vorbis.meta_size);
                break;
            }
            /* SW7344-189, Coverity 35329 */
            /* coverity[unterminated_case] */
            /* coverity[fallthrough] */
            default:
            goto stream_error;
        }
    }
done:
    return;

stream_error:
    BDBG_MSG(("b_ogg_vorbis_payload_handler: %#lx %#lx stream error", (unsigned long)ogg, (unsigned long)stream));
    if(data) {
        batom_release(data);
    }
    b_ogg_vorbis_shutdown(ogg, stream);
    return;
}

static bool b_ogg_opus_init(bogg_parser_t ogg, b_ogg_stream_handler *stream, const batom_cursor *payload, size_t packet_size, batom_cursor *spare_cursor, const bopus_indentification_header *indentification_header)
{
    bmedia_waveformatex_header wf;
    size_t meta_size;

    bmedia_init_waveformatex(&wf);
    stream->data.opus.sample_rate = indentification_header->inputSampleRate;
    wf.nSamplesPerSec = indentification_header->inputSampleRate;
    wf.nChannels = indentification_header->channelCount;
    wf.wFormatTag = 0xFFFD;
    wf.cbSize=packet_size;
    meta_size = wf.cbSize + BMEDIA_WAVEFORMATEX_BASE_SIZE;
    stream->data.opus.meta = ogg->cfg.alloc->bmem_alloc(ogg->cfg.alloc, meta_size);
    if(!stream->data.opus.meta) {
        (void)BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto stream_error;
    }
    stream->data.opus.meta_size = meta_size;
    meta_size=bmedia_write_waveformatex(stream->data.opus.meta, &wf);
    BDBG_ASSERT(meta_size==BMEDIA_WAVEFORMATEX_BASE_SIZE);
    BATOM_CLONE(spare_cursor, payload);
    batom_cursor_copy(spare_cursor, (uint8_t *)stream->data.opus.meta + meta_size, packet_size);
    return true;

stream_error:
    return false;
}

static void b_ogg_opus_shutdown(bogg_parser_t ogg, b_ogg_stream_handler *stream)
{
    if(stream->data.opus.meta) {
        ogg->cfg.alloc->bmem_free(ogg->cfg.alloc, stream->data.opus.meta);
    }
    stream->data.opus.meta = NULL;
    stream->data.opus.meta_size = 0;
    return;
}

static void b_ogg_opus_payload_handler(bogg_parser_t ogg, const bogg_page_header *page_header, b_ogg_stream_handler *stream, unsigned packet_no, const batom_cursor *payload, size_t packet_size, bool spanning, batom_cursor *spare_cursor)
{
    bmedia_bcma_hdr hdr;
    batom_t  pes_atom;
    batom_t data;

    BDBG_MSG_TRACE(("b_ogg_opus_payload_handler: %p %p", (void *)ogg, (void *)stream));
    BATOM_CLONE(spare_cursor, payload);
    batom_cursor_skip(spare_cursor, packet_size);
    if(spanning || batom_accum_len(stream->accum)) {
        batom_accum_append(stream->accum, ogg->acc, payload, spare_cursor);
        if(spanning) {
            return;
        }
        data = batom_from_accum(stream->accum, NULL, NULL);
    } else {
        data = batom_accum_extract(ogg->acc, payload, spare_cursor, NULL, NULL);
    }
    if(!data) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto done;
    }
    BMEDIA_PACKET_HEADER_INIT(&hdr.pes);
    if(packet_no==0) {
        unsigned timestamp;
        BDBG_ASSERT(stream->data.opus.sample_rate>0);
        timestamp = (page_header->granule_position * 1000) / stream->data.opus.sample_rate;

        BMEDIA_PES_SET_PTS(&hdr.pes, bmedia_time2pts(timestamp, BMEDIA_TIME_SCALE_BASE));
    }
    hdr.pes.header_off = 4;
    hdr.pes.header_type = B_OGG_VEC_BCMA_FRAME;
    bmedia_bcma_hdr_init(&hdr, batom_len(data));
    pes_atom = batom_from_range_and_atom(stream->data.opus.meta, stream->data.opus.meta_size, data, &bmedia_bcma_atom, &hdr.pes);
    batom_release(data);
    if(pes_atom) {
        stream->handler->handler(stream->handler, pes_atom);
    }

done:
    return;
}


static void b_ogg_stream_shutdown(bogg_parser_t ogg, b_ogg_stream_handler *stream)
{
    batom_accum_clear(stream->accum);
    switch(stream->type) {
    case b_ogg_stream_type_vorbis:
        b_ogg_vorbis_shutdown(ogg, stream);
        break;
    case b_ogg_stream_type_opus:
        b_ogg_opus_shutdown(ogg, stream);
        break;
    default:
        break;
    }
    stream->type = b_ogg_stream_type_unknown;
    return;
}

static void b_ogg_stream_destroy(bogg_parser_t ogg, b_ogg_stream_handler *stream)
{
    b_ogg_stream_shutdown(ogg, stream);
    batom_accum_destroy(stream->accum);
    BKNI_Free(stream);
    return;
}


void 
bogg_parser_remove_handler(bogg_parser_t ogg, bogg_parser_handler *handler)
{
    b_ogg_stream_handler *stream;

    BDBG_OBJECT_ASSERT(ogg, bogg_parser_t);
    BDBG_ASSERT(handler);
    BDBG_MSG_TRACE(("bogg_parser_remove_handler:> %#lx  %#lx %#x", (unsigned long)ogg, (unsigned long)handler, (unsigned)handler->id));
    for(stream=BLST_S_FIRST(&ogg->stream_handlers);stream;stream = BLST_S_NEXT(stream, link)) {
        if(stream->handler == handler) {
            break;
        }
    }
    if(stream) {
        BLST_S_REMOVE(&ogg->stream_handlers, stream, b_ogg_stream_handler, link);
        b_ogg_stream_destroy(ogg, stream);
    }
    BLST_S_REMOVE(&ogg->object_handlers, handler, bogg_parser_handler, link);
    BDBG_MSG_TRACE(("bogg_parser_remove_handler:< %#lx  %#lx %#x", (unsigned long)ogg, (unsigned long)handler, (unsigned)handler->id));
    return;
}

static b_ogg_stream_handler *
b_ogg_find_stream_handler(bogg_parser_t ogg, uint32_t id)
{
    b_ogg_stream_handler *cur;

    for(cur = BLST_S_FIRST(&ogg->stream_handlers); cur; cur=BLST_S_NEXT(cur, link)) {
        uint32_t cur_id;
        BDBG_ASSERT(cur->handler);
        cur_id = cur->handler->id;
        if (cur_id == id+1) {
            return cur;
        }
        if(id < cur_id) {
            return NULL;
        }
    }
    return NULL;
}

static void 
b_ogg_trim(bogg_parser_t ogg, batom_cursor *cursor)
{
    BDBG_MSG_TRACE(("b_ogg_trim>: %p %u:%u", (void *)ogg, (unsigned)batom_accum_len(ogg->acc), (unsigned)batom_cursor_pos(cursor) ));
    ogg->state.acc_off += batom_cursor_pos(cursor);
    batom_accum_trim(ogg->acc, cursor);
    BDBG_MSG_TRACE(("b_ogg_trim< %p %u", (void *)ogg, (unsigned)batom_accum_len(ogg->acc)));
    return;
}

void
bogg_parser_get_status(bogg_parser_t ogg, bogg_parser_status *status)
{
    BDBG_OBJECT_ASSERT(ogg, bogg_parser_t);
    status->acc_length = batom_accum_len(ogg->acc);
    status->offset = ogg->state.acc_off  + status->acc_length;
    status->obj_length = 0;
    status->data_discontinuity = ogg->state.data_discontinuity;
    status->errors = ogg->state.errors;

    switch(ogg->state.state) {
    case b_ogg_parser_state_parse:
        status->state = "parse";
        break;
    case b_ogg_parser_state_discard:
        status->state = "discard";
        break;
    case b_ogg_parser_state_resync:
        status->state = "resync";
        break;
    }
    return;
}


void
bogg_parser_reset(bogg_parser_t ogg)
{
    b_ogg_stream_handler *stream;

    BDBG_MSG_TRACE(("bogg_parser_reset>: %#lx", (unsigned long)ogg));
    BDBG_OBJECT_ASSERT(ogg, bogg_parser_t);
    batom_accum_clear(ogg->acc);
    for(stream=BLST_S_FIRST(&ogg->stream_handlers);stream;stream=BLST_S_NEXT(stream, link)) {
        b_ogg_stream_shutdown(ogg, stream);
    }
    BKNI_Memset(&ogg->state, 0, sizeof(ogg->state));
    BMEDIA_PARSING_ERRORS_INIT(&ogg->state.errors);
    ogg->state.state = b_ogg_parser_state_parse; 
    ogg->state.acc_off = 0;
    ogg->state.object = BOGG_PAGE_MIN_HEADER_LENGTH;
    ogg->state.data_discontinuity = false;
    BDBG_MSG_TRACE(("bogg_parser_reset<: %#lx", (unsigned long)ogg));
    return;
}

int 
bogg_parser_seek(bogg_parser_t ogg, bogg_off_t off)
{
    bogg_off_t acc_off; 
    size_t acc_len;

    BDBG_OBJECT_ASSERT(ogg, bogg_parser_t);
    BDBG_ASSERT(off>=0);

    acc_len = batom_accum_len(ogg->acc);
    acc_off = ogg->state.acc_off + (int)batom_accum_len(ogg->acc);
    if(off == acc_off) {
        BDBG_MSG(("bogg_parser_seek:%p " BDBG_UINT64_FMT " no-op", (void *)ogg, BDBG_UINT64_ARG(off)));
        return 0;
    }
    batom_accum_clear(ogg->acc);
    ogg->state.data_discontinuity = true;
    ogg->state.state = b_ogg_parser_state_resync;
    BDBG_MSG(("bogg_parser_seek:%p " BDBG_UINT64_FMT "(" B_OFFT_FMT ")", (void *)ogg, BDBG_UINT64_ARG(off), B_OFFT_ARG(ogg->state.acc_off)));
    ogg->state.acc_off = off;
    return 0;
}


bogg_parser_t 
bogg_parser_create(batom_factory_t factory, const bogg_parser_cfg *cfg)
{
    bogg_parser_t ogg;
    BERR_Code rc;

    BDBG_MSG_TRACE(("bogg_parser_create>: %p", (void *)cfg));
    BSTD_UNUSED(rc);

    BDBG_ASSERT(cfg);
    BDBG_ASSERT(factory);

    ogg = BKNI_Malloc(sizeof(*ogg));
    if (!ogg) { 
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc_ogg;
    }
    BDBG_OBJECT_INIT(ogg, bogg_parser_t);

    ogg->cfg = *cfg;
    ogg->factory = factory;
    ogg->acc = batom_accum_create(factory);
    ogg->vecs[B_OGG_VEC_BCMA_FRAME] = &bmedia_frame_bcma;
    if(!ogg->acc) {
        rc  = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc_accum;
    }
    BLST_S_INIT(&ogg->object_handlers);
    BLST_S_INIT(&ogg->stream_handlers);
    bogg_parser_reset(ogg);
    BDBG_MSG_TRACE(("bogg_parser_create>: %#lx %#lx", (unsigned long)cfg, (unsigned long)ogg));
    return ogg;

err_alloc_accum:
    BKNI_Free(ogg);
err_alloc_ogg:
    BDBG_MSG_TRACE(("bogg_parser_create>: %#lx %#lx", (unsigned long)cfg, (unsigned long)NULL));
    return NULL;
}

static void
b_ogg_empty_stream_error_cb(void *cntx)
{
    BSTD_UNUSED(cntx);
    return ;
}

void 
bogg_parser_default_cfg(bogg_parser_cfg *cfg)
{
    BDBG_ASSERT(cfg);
    cfg->user_cntx = NULL;
    cfg->stream_error = b_ogg_empty_stream_error_cb;
    return;
}

void 
bogg_parser_destroy(bogg_parser_t ogg)
{
    bogg_parser_handler *handler;
    b_ogg_stream_handler *stream;

    BDBG_MSG_TRACE(("bogg_destroy>: %#lx", (unsigned long)ogg));
    BDBG_OBJECT_ASSERT(ogg, bogg_parser_t);
    for(handler=BLST_S_FIRST(&ogg->object_handlers); handler; handler=BLST_S_NEXT(handler, link)) {
        BDBG_WRN(("bogg_destroy: %#lx lost object_handler %#lx ...", (unsigned long)ogg, (unsigned long)handler));
        BDBG_WRN(("bogg_destroy: %#lx lost object_handler %#lx %#x", (unsigned long)ogg, (unsigned long)handler, (unsigned)handler->id));
    }
    while(NULL!=(stream=BLST_S_FIRST(&ogg->stream_handlers))) {
        BLST_S_REMOVE_HEAD(&ogg->stream_handlers, link);
        b_ogg_stream_destroy(ogg, stream);
    }
    bogg_parser_reset(ogg);
    batom_accum_destroy(ogg->acc);
    BDBG_OBJECT_DESTROY(ogg, bogg_parser_t);
    BDBG_MSG_TRACE(("bogg_destroy<: %#lx", (unsigned long)ogg));
    BKNI_Free(ogg);
    return;
}

static void
b_ogg_stream_error(bogg_parser_t ogg)
{
    batom_cursor cursor;

    ogg->state.state = b_ogg_parser_state_resync;
    batom_cursor_from_accum(&cursor, ogg->acc);
    batom_cursor_skip(&cursor, 1);
    b_ogg_trim(ogg, &cursor);
    return;
}

static void
b_ogg_handle_payload(bogg_parser_t ogg, const bogg_page_header *page_header, b_ogg_stream_handler *handler, unsigned packet_no, const batom_cursor *payload, size_t packet_size, bool spanning)
{
    batom_cursor cursor;
    BDBG_MSG_TRACE(("b_ogg_handle_payload: %p %p %u %#x %s", (void *)ogg, (void *)handler, handler->type, page_header->header_type, spanning?"spanning":""));
    switch(handler->type) {
    case b_ogg_stream_type_vorbis:
        b_ogg_vorbis_payload_handler(ogg, page_header, handler, packet_no, payload, packet_size, spanning, &cursor);
        return;
    case b_ogg_stream_type_opus:
        b_ogg_opus_payload_handler(ogg, page_header, handler, packet_no, payload, packet_size, spanning, &cursor);
        return;
    case b_ogg_stream_type_unknown:
        if(packet_no==0 && !spanning && (page_header->header_type & BOGG_HEADER_TYPE_FIRST_PAGE)==BOGG_HEADER_TYPE_FIRST_PAGE) {
            bvorbis_frame_header vorbis_frame_header;
            bopus_indentification_header opus_indentification_header;

            if(page_header->page_segments!=1) {
                break;
            }
            BATOM_CLONE(&cursor, payload);
            if(bvorbis_parse_frame_header(&cursor, &vorbis_frame_header)) {
                if(vorbis_frame_header.packet_type==1) {
                    if(b_ogg_vorbis_init(ogg, handler, payload, packet_size, &cursor)) {
                        handler->type = b_ogg_stream_type_vorbis;
                    }
                }
                break;
            }
            BATOM_CLONE(&cursor, payload);
            if(bopus_parse_indentification_header(&cursor, &opus_indentification_header)) {
                if(b_ogg_opus_init(ogg, handler, payload, packet_size, &cursor, &opus_indentification_header)) {
                    handler->type = b_ogg_stream_type_opus;
                }
                break;
            }
        }
        break;
    }
    return;
}

size_t
bogg_parser_feed(bogg_parser_t ogg, batom_pipe_t pipe)
{
    size_t len;

    BDBG_OBJECT_ASSERT(ogg, bogg_parser_t);
    BDBG_MSG_TRACE(("bogg_parser_feed>:%p %p %u", (void *)ogg, (void *)pipe, (unsigned)(ogg->state.acc_off + batom_accum_len(ogg->acc))));
    len=0;
    for(;;) {
        batom_cursor cursor;
        bool want_continue;
        size_t size;

        if(!ogg->state.parser_return) {
            batom_t atom;
            atom=batom_pipe_pop(pipe);
            if(!atom) {
                break;
            }
            len += batom_len(atom);
            batom_accum_add_atom(ogg->acc, atom);
            batom_release(atom);
        }
        ogg->state.parser_return = false;
        do {
            size_t acc_len = batom_accum_len(ogg->acc);
            bogg_page_header page_header;
            int rc;
            b_ogg_stream_handler *handler;

            batom_cursor_from_accum(&cursor, ogg->acc);
            want_continue = false;
            ogg->state.data_discontinuity = false;
            switch(ogg->state.state) {
            default:
            case b_ogg_parser_state_parse:
                if(acc_len<ogg->state.object) {
                    break;
                }
                if(!bogg_parse_page_header(&cursor, &page_header)) { goto stream_error; }
                ogg->state.object = BOGG_PAGE_MIN_HEADER_LENGTH + page_header.page_segments;
                if(acc_len<ogg->state.object) {
                    break;
                }
                rc = bogg_page_get_size(&page_header, &cursor);
                BDBG_MSG_TRACE(("bogg_parser_feed:%#lx stream:%u size:%d", (unsigned long)ogg, (unsigned)page_header.bitstream_serial_number, rc));
                if(rc<0) { goto stream_error; }
                ogg->state.object += rc;
                handler = b_ogg_find_stream_handler(ogg, page_header.bitstream_serial_number);
                if(handler) {
                    bogg_page_payload_parser parser;

                    if(acc_len<ogg->state.object) {
                        break;
                    }
                    if(!bogg_page_payload_parser_init(&parser, &cursor, &page_header)) { goto stream_error; }
                    for(;;) {
                        bool spanning;
                        rc = bogg_page_payload_parser_next(&parser, &page_header, &spanning);
                        if(rc<0) {
                            break;
                        }
                        b_ogg_handle_payload(ogg, &page_header, handler, parser.payload-1, &cursor, rc, spanning);
                        batom_cursor_skip(&cursor, rc);
                    }
                    b_ogg_trim(ogg, &cursor);
                    ogg->state.object = BOGG_PAGE_MIN_HEADER_LENGTH;
                    want_continue = true;
                } else {
                    BDBG_MSG_TRACE(("bogg_parser_feed:%p stream:%u discarding %u(%u)", (void *)ogg, (unsigned)page_header.bitstream_serial_number, (unsigned)ogg->state.object, (unsigned)acc_len));
                    ogg->state.state = b_ogg_parser_state_discard;
                    want_continue = true;
                }
                break;
stream_error:
                b_ogg_stream_error(ogg);
                want_continue = true;
                break;
            case b_ogg_parser_state_discard:
                BDBG_MSG_TRACE(("bogg_parser_feed: discard %u %u", (unsigned)ogg->state.object, (unsigned)acc_len));
                size = acc_len;
                if(size>=ogg->state.object) {
                    size = ogg->state.object;
                    ogg->state.state = b_ogg_parser_state_parse;
                    ogg->state.object = BOGG_PAGE_MIN_HEADER_LENGTH;
                }
                batom_cursor_skip(&cursor, size);
                b_ogg_trim(ogg, &cursor);
                break;
            case b_ogg_parser_state_resync:
                {
                    unsigned skip;
                    for(skip=0;;skip++) {
                        int byte = batom_cursor_next(&cursor);
                        if(byte==BATOM_EOF) {
                            break;
                        }
                        if(byte=='O') {
                            batom_cursor_from_accum(&cursor, ogg->acc);
                            batom_cursor_skip(&cursor, skip);
                            BDBG_MSG(("bogg_parser_feed:%p found sync after %u bytes", (void *)ogg, skip));
                            if(bogg_parse_page_header(&cursor, &page_header)) {
                                /* completed resync */
                                batom_cursor_from_accum(&cursor, ogg->acc);
                                batom_cursor_skip(&cursor, skip);
                                ogg->state.state = b_ogg_parser_state_parse; 
                                ogg->state.object = BOGG_PAGE_MIN_HEADER_LENGTH;
                                /* completed resync */
                                break;
                            }
                            batom_cursor_from_accum(&cursor, ogg->acc);
                            batom_cursor_skip(&cursor, skip+1); /* return cursor back */
                        }
                    }
                    BDBG_MSG(("bogg_parser_feed:%p resync discarding %u bytes", (void *)ogg, skip));
                    b_ogg_trim(ogg, &cursor);
                }
                break;
            }
        } while(want_continue);
    }
    return len;
}

void 
bogg_parser_get_stream_cfg(bogg_parser_t parser, bmedia_pes_stream_cfg *cfg)
{
    BDBG_OBJECT_ASSERT(parser, bogg_parser_t);
    BDBG_ASSERT(cfg);
    bmedia_pes_default_stream_cfg(cfg);
    cfg->nvecs = sizeof(parser->vecs)/sizeof(parser->vecs[0]);
    cfg->vecs = parser->vecs;
    return;
}

void bogg_parser_flush(bogg_parser_t ogg)
{
    const b_ogg_stream_handler *stream;

    BDBG_OBJECT_ASSERT(ogg, bogg_parser_t);
    batom_accum_clear(ogg->acc);
    for(stream=BLST_S_FIRST(&ogg->stream_handlers);stream;stream=BLST_S_NEXT(stream, link)) {
        batom_accum_clear(stream->accum);
    }
    ogg->state.object = BOGG_PAGE_MIN_HEADER_LENGTH;
    ogg->state.state = b_ogg_parser_state_parse;
    return;
}
