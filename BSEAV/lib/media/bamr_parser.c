/***************************************************************************
 *     Copyright (c) 2013 Broadcom Corporation
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
 * AMR parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bamr_parser.h"
#include "bkni.h"

BDBG_MODULE(bamr_parser);
#define BDBG_MSG_TRACE(x)   BDBG_MSG(x)

#define B_AMR_VEC_BCMA_FRAME	0

typedef enum b_amr_parser_state {
    b_amr_parser_state_top, /* looking for 'fLaC' */
    b_amr_parser_state_frame_parse, /* parse frame */
    b_amr_parser_state_frame_capture, /* capture frame */
    b_amr_parser_state_bad_stream /* bad stream, drop all data */
} b_amr_parser_state;

BDBG_OBJECT_ID(bamr_parser);
#define B_AMR_VEC_BCMA_FRAME	0

struct bamr_parser {
    BDBG_OBJECT(bamr_parser)
    batom_accum_t acc;
    batom_factory_t factory;
    struct {
        b_amr_parser_state state;
        bamr_off_t acc_off; /* stream offset for accumulator */
        bmedia_parsing_errors errors;
        baudio_format streaminfo;
        unsigned frame_no;
        bool streaminfo_valid;
        bool parser_return;
    } state;
    const batom_vec *vecs[1];
    void *waveformatex; /* header for AMR audio */
    bamr_parser_cfg cfg;
};


static void 
b_amr_trim(bamr_parser_t amr, batom_cursor *cursor)
{
    BDBG_MSG_TRACE(("b_amr_trim>: %p %u:%u", (void *)amr, (unsigned)batom_accum_len(amr->acc), (unsigned)batom_cursor_pos(cursor) ));
    amr->state.acc_off += batom_cursor_pos(cursor);
    batom_accum_trim(amr->acc, cursor);
    BDBG_MSG_TRACE(("b_amr_trim< %p %u", (void *)amr, (unsigned)batom_accum_len(amr->acc)));
    return;
}

void
bamr_parser_get_status(bamr_parser_t amr, bamr_parser_status *status)
{
    BDBG_OBJECT_ASSERT(amr, bamr_parser);
    status->acc_length = batom_accum_len(amr->acc);
    status->offset = amr->state.acc_off  + status->acc_length;
    status->obj_length = 0;
    status->data_discontinuity = amr->state.state == b_amr_parser_state_bad_stream;
    status->errors = amr->state.errors;

    switch(amr->state.state) {
    case b_amr_parser_state_top:
        status->state = "top";
        break;
    case b_amr_parser_state_frame_parse:
        status->state = "frame_parse";
        break;
    case b_amr_parser_state_frame_capture:
        status->state = "frame_capture";
        break;
    case b_amr_parser_state_bad_stream:
        status->state = "bad_stream";
        break;
    }
    return;
}


void
bamr_parser_reset(bamr_parser_t amr)
{

    BDBG_MSG_TRACE(("bamr_parser_reset>: %#lx", (unsigned long)amr));
    BDBG_OBJECT_ASSERT(amr, bamr_parser);
    batom_accum_clear(amr->acc);
    BKNI_Memset(&amr->state, 0, sizeof(amr->state));
    BMEDIA_PARSING_ERRORS_INIT(&amr->state.errors);
    amr->state.state = b_amr_parser_state_top; 
    amr->state.acc_off = 0;
    amr->state.streaminfo_valid = false;
    amr->state.frame_no = 0;
    BDBG_MSG_TRACE(("bamr_parser_reset<: %#lx", (unsigned long)amr));
    return;
}

int
bamr_parser_seek(bamr_parser_t amr, bamr_off_t off)
{
    bamr_off_t acc_off;
    size_t acc_len;

    BDBG_OBJECT_ASSERT(amr, bamr_parser);
    BDBG_ASSERT(off>=0);

    acc_len = batom_accum_len(amr->acc);
    acc_off = amr->state.acc_off + (int)batom_accum_len(amr->acc);
    if(off == acc_off) {
        BDBG_MSG(("bamr_parser_seek: %p " BDBG_UINT64_FMT " no-op", (void *)amr, BDBG_UINT64_ARG(off)));
        return 0;
    }
    batom_accum_clear(amr->acc);

    BDBG_MSG(("bamr_parser_seek: %p " BDBG_UINT64_FMT "(" B_OFFT_FMT ")", (void *)amr, BDBG_UINT64_ARG(off), B_OFFT_ARG(amr->state.acc_off)));
    amr->state.acc_off = off;
    amr->state.frame_no = 0;
    amr->state.state = off == 0 ? b_amr_parser_state_top : b_amr_parser_state_bad_stream;
    return 0;
}


bamr_parser_t
bamr_parser_create(batom_factory_t factory, const bamr_parser_cfg *cfg)
{
    bamr_parser_t amr;
    BERR_Code rc;

    BDBG_MSG_TRACE(("bamr_parser_create>: %p", (void *)cfg));
    BSTD_UNUSED(rc);

    BDBG_ASSERT(cfg);
    BDBG_ASSERT(factory);


    if(cfg->alloc==NULL) {
        rc=BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_config_amr;
    }

    amr = BKNI_Malloc(sizeof(*amr));
    if (!amr) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc_amr;
    }
    BDBG_OBJECT_INIT(amr, bamr_parser);

    amr->vecs[B_AMR_VEC_BCMA_FRAME] = &bmedia_frame_bcma;
    amr->cfg = *cfg;
    amr->factory = factory;
    amr->acc = batom_accum_create(factory);
    if(!amr->acc) {
        rc  = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc_accum;
    }
    amr->waveformatex = amr->cfg.alloc->bmem_alloc(amr->cfg.alloc, BMEDIA_WAVEFORMATEX_BASE_SIZE);
    if(!amr->waveformatex) {
        rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_wavformatex;
    }
    bamr_parser_reset(amr);
    BDBG_MSG_TRACE(("bamr_parser_create>: %p %p", (void *)cfg, (void *)amr));
    return amr;

err_wavformatex:
    batom_accum_destroy(amr->acc);
err_alloc_accum:
    BKNI_Free(amr);
err_alloc_amr:
err_config_amr:
    BDBG_MSG_TRACE(("bamr_parser_create>: %p %p", (void *)cfg, (void *)NULL));
    return NULL;
}

void
bamr_parser_get_stream_cfg(bamr_parser_t parser, bmedia_pes_stream_cfg *cfg)
{
    BDBG_OBJECT_ASSERT(parser, bamr_parser);
    BDBG_ASSERT(cfg);
    bmedia_pes_default_stream_cfg(cfg);
    cfg->nvecs = sizeof(parser->vecs)/sizeof(parser->vecs[0]);
    cfg->vecs = parser->vecs;
    return;
}

static void
b_amr_empty_stream_error_cb(void *cntx)
{
    BSTD_UNUSED(cntx);
    return ;
}

static bamr_parser_action 
b_amr_discard_frame_cb(void *user_cntx, batom_t frame)
{
    BSTD_UNUSED(user_cntx);
    batom_release(frame);
    return bamr_parser_action_return;
}

void 
bamr_parser_default_cfg(bamr_parser_cfg *cfg)
{
    BDBG_ASSERT(cfg);
    cfg->user_cntx = NULL;
    cfg->stream_error = b_amr_empty_stream_error_cb;
    cfg->frame = b_amr_discard_frame_cb;
    return;
}

void 
bamr_parser_destroy(bamr_parser_t amr)
{
    BDBG_OBJECT_ASSERT(amr, bamr_parser);

    BDBG_MSG_TRACE(("bamr_destroy>: %p", (void *)amr));
    BDBG_OBJECT_ASSERT(amr, bamr_parser);
    amr->cfg.alloc->bmem_free(amr->cfg.alloc, amr->waveformatex);
    bamr_parser_reset(amr);
    batom_accum_destroy(amr->acc);
    BDBG_OBJECT_DESTROY(amr, bamr_parser);
    BDBG_MSG_TRACE(("bamr_destroy<: %p", (void *)amr));
    BDBG_OBJECT_DESTROY(amr, bamr_parser);
    BKNI_Free(amr);
    return;
}

static void
b_amr_stream_error(bamr_parser_t amr)
{
    batom_cursor cursor;

    amr->state.state = b_amr_parser_state_bad_stream;
    batom_cursor_from_accum(&cursor, amr->acc);
    batom_cursor_skip(&cursor, 1);
    b_amr_trim(amr, &cursor);
    return;
}


size_t
bamr_parser_feed(bamr_parser_t amr, batom_pipe_t pipe)
{
    size_t len;

    BDBG_OBJECT_ASSERT(amr, bamr_parser);
    BDBG_MSG_TRACE(("bamr_parser_feed>:%p %p %u", (void *)amr, (void *)pipe, (unsigned)amr->state.acc_off + (unsigned)batom_accum_len(amr->acc)));
    len=0;
    amr->state.parser_return = false;
    for(;;) {
        batom_cursor cursor;
        bool want_continue;

        if(!amr->state.parser_return) {
            batom_t atom;

            atom=batom_pipe_pop(pipe);
            if(!atom) {
                break;
            }
            len += batom_len(atom);
            batom_accum_add_atom(amr->acc, atom); 
            batom_release(atom);
        }
        amr->state.parser_return = false;
        do {
            size_t acc_len = batom_accum_len(amr->acc);
            baudio_format stream_type;

            batom_cursor_from_accum(&cursor, amr->acc);
            want_continue = false;
            switch(amr->state.state) {
            default:
            case b_amr_parser_state_bad_stream:
                batom_cursor_skip(&cursor, acc_len);
                b_amr_trim(amr, &cursor);
                break;
            case b_amr_parser_state_top:
                stream_type = bamr_parse_header(&cursor);
                if(stream_type==baudio_format_amr_nb || stream_type==baudio_format_amr_wb) {
                    bmedia_waveformatex_header wf;
                    size_t waveformatex_size;

                    b_amr_trim(amr, &cursor);
                    amr->state.streaminfo_valid = true;
                    amr->state.streaminfo = stream_type;
                    amr->state.state = b_amr_parser_state_frame_parse;
                    amr->state.frame_no = 0;
                    bmedia_init_waveformatex(&wf);
                    wf.wFormatTag = stream_type == baudio_format_amr_nb ? 0x0057 : 0x0058; /* vlc_codecs.h */
                    wf.nChannels = 1;
                    wf.nSamplesPerSec = stream_type == baudio_format_amr_nb ? 8000:16000;
                    wf.nBlockAlign = 1;
                    wf.wBitsPerSample = 16;
                    wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
                    waveformatex_size = bmedia_write_waveformatex(amr->waveformatex, &wf);
                    BDBG_ASSERT(waveformatex_size == BMEDIA_WAVEFORMATEX_BASE_SIZE);
                    want_continue = true;
                } else {
                    if(!BATOM_IS_EOF(&cursor)) {
                        goto stream_error;
                    }
                }
                break;
            case b_amr_parser_state_frame_parse:
                {
                    int frame_length;
                    batom_cursor frame_start;
                    batom_t amr_frame;
                    batom_t bcma_frame;
                    bamr_parser_action action;
                    bmedia_bcma_hdr hdr;

                    BATOM_CLONE(&frame_start, &cursor);
                    frame_length = bamr_parse_frame_header(&frame_start, amr->state.streaminfo);
                    BDBG_MSG_TRACE(("bamr_parser_feed>:%#lx codec:%u frame_length:%d frame_no:%u", (unsigned long)amr, (unsigned)amr->state.streaminfo, frame_length, amr->state.frame_no));
                    if(frame_length<0) {
                        if(BATOM_IS_EOF(&frame_start)) {
                            break;
                        } else {
                            goto stream_error;
                        }
                    }
                    BATOM_CLONE(&frame_start, &cursor);
                    if(batom_cursor_skip(&cursor, frame_length)!=(unsigned)frame_length) {
                        break;
                    }
                    amr_frame = batom_accum_extract(amr->acc, &frame_start, &cursor, NULL, NULL);
                    if(amr_frame==NULL) {
                        break;
                    }
                    BMEDIA_PACKET_HEADER_INIT(&hdr.pes);
                    BMEDIA_PES_SET_PTS(&hdr.pes, bmedia_time2pts(20/*ms*/ * amr->state.frame_no, BMEDIA_TIME_SCALE_BASE));
                    hdr.pes.header_off = 4;
                    hdr.pes.header_type = B_AMR_VEC_BCMA_FRAME;
                    bmedia_bcma_hdr_init(&hdr, frame_length);
                    bcma_frame = batom_from_range_and_atom(amr->waveformatex, BMEDIA_WAVEFORMATEX_BASE_SIZE, amr_frame, &bmedia_bcma_atom, &hdr.pes);
                    batom_release(amr_frame);
                    if(bcma_frame==NULL) {
                        break;
                    }
                    b_amr_trim(amr, &cursor);
                    amr->state.frame_no++;
                    action = amr->cfg.frame(amr->cfg.user_cntx, bcma_frame);
                    if(action==bamr_parser_action_none) {
                        want_continue = true;
                    }
                }
                break;
stream_error:
                b_amr_stream_error(amr);
                want_continue = true;
                break;
            }
        } while(want_continue);
    }
/* stopped_parsing: */
    return len;
}

void bamr_parser_flush(bamr_parser_t amr)
{
    BDBG_OBJECT_ASSERT(amr, bamr_parser);
    batom_accum_clear(amr->acc);
    amr->state.state = b_amr_parser_state_top; 
    amr->state.acc_off = 0;
    amr->state.streaminfo_valid = false;
    return;
}
