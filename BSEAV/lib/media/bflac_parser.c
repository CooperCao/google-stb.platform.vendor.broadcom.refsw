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
 * FLAC parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bflac_parser.h"
#include "bkni.h"

BDBG_MODULE(bflac_parser);
#define BDBG_MSG_TRACE(x)   BDBG_MSG(x)

#define B_FLAC_VEC_BCMA_FRAME	0

typedef enum b_flac_parser_state {
    b_flac_parser_state_top, /* looking for 'fLaC' */
    b_flac_parser_state_metadata_parse, /* parse metadata object */
    b_flac_parser_state_metadata_discard, /* discard current metadata object */
    b_flac_parser_state_frame_parse, /* parse frame */
    b_flac_parser_state_frame_capture, /* capture frame */
    b_flac_parser_state_bad_stream /* bad stream, drop all data */
} b_flac_parser_state;

BDBG_OBJECT_ID(bflac_parser_t);

struct bflac_parser {
    BDBG_OBJECT(bflac_parser_t)
    batom_accum_t acc;
    batom_factory_t factory;
    BLST_S_HEAD(b_flac_block_handlers, bflac_parser_handler) block_handlers; /* there is one-to-one relation between object and stream handlers */
    struct {
        b_flac_parser_state state;
        size_t object; /* number of bytes to capture or discard */
        bflac_off_t acc_off; /* stream offset for accumulator */
        bflac_metadata_block_header block_header;
        bmedia_parsing_errors errors;
        bflac_streaminfo streaminfo;
        bool streaminfo_valid;
        bool parser_return;
    } state;
    bflac_parser_cfg cfg;
};


/* flac object handlers, keep list sorted by the id, it allows O/2 search
 * complexity and ease handle of multiple handlers for the same id code */
void 
bflac_parser_install_handler(bflac_parser_t flac, bflac_parser_handler *handler, uint32_t block_type, bflac_parser_handler_cb object_handler)
{
    BDBG_OBJECT_ASSERT(flac, bflac_parser_t);
    BDBG_MSG_TRACE(("bflac_parser_install_handler:> %#lx  %#lx %#x", (unsigned long)flac, (unsigned long)handler, (unsigned)block_type));
    BDBG_ASSERT(handler);
    handler->block_type = block_type;
    handler->handler = object_handler;

    BLST_S_INSERT_HEAD(&flac->block_handlers, handler, link);
    BDBG_MSG_TRACE(("bflac_parser_install_handler:< %#lx  %#lx %#x", (unsigned long)flac, (unsigned long)handler, block_type));
    return ;
}


void 
bflac_parser_remove_handler(bflac_parser_t flac, bflac_parser_handler *handler)
{
    BDBG_OBJECT_ASSERT(flac, bflac_parser_t);
    BDBG_ASSERT(handler);
    BDBG_MSG_TRACE(("bflac_parser_remove_handler:> %#lx  %#lx %#x", (unsigned long)flac, (unsigned long)handler, (unsigned)handler->block_type));
    BLST_S_REMOVE(&flac->block_handlers, handler, bflac_parser_handler, link);
    BDBG_MSG_TRACE(("bflac_parser_remove_handler:< %#lx  %#lx %#x", (unsigned long)flac, (unsigned long)handler, (unsigned)handler->block_type));
    return;
}

static bflac_parser_handler *
b_flac_find_block_handler(bflac_parser_t flac, uint32_t block_type)
{
    bflac_parser_handler *cur;

    for(cur = BLST_S_FIRST(&flac->block_handlers); cur; cur=BLST_S_NEXT(cur, link)) {
        if (cur->block_type == block_type) {
            return cur;
        }
    }
    return NULL;
}

static void 
b_flac_trim(bflac_parser_t flac, batom_cursor *cursor)
{
    BDBG_MSG_TRACE(("b_flac_trim>: %p %u:%u", (void *)flac, (unsigned)batom_accum_len(flac->acc), (unsigned)batom_cursor_pos(cursor) ));
    flac->state.acc_off += batom_cursor_pos(cursor);
    batom_accum_trim(flac->acc, cursor);
    BDBG_MSG_TRACE(("b_flac_trim< %p %u", (void *)flac, (unsigned)batom_accum_len(flac->acc)));
    return;
}

void
bflac_parser_get_status(bflac_parser_t flac, bflac_parser_status *status)
{
    BDBG_OBJECT_ASSERT(flac, bflac_parser_t);
    status->acc_length = batom_accum_len(flac->acc);
    status->offset = flac->state.acc_off  + status->acc_length;
    status->obj_length = 0;
    status->data_discontinuity = flac->state.state == b_flac_parser_state_bad_stream;
    status->errors = flac->state.errors;

    switch(flac->state.state) {
    case b_flac_parser_state_top:
        status->state = "top";
        break;
    case b_flac_parser_state_metadata_parse:
        status->state = "metadata_parse";
        break;
    case b_flac_parser_state_metadata_discard:
        status->state  = "metadata_discard";
        break;
    case b_flac_parser_state_frame_parse:
        status->state = "frame_parse";
        break;
    case b_flac_parser_state_frame_capture:
        status->state = "frame_capture";
        break;
    case b_flac_parser_state_bad_stream:
        status->state = "bad_stream";
        break;
    }
    return;
}


void
bflac_parser_reset(bflac_parser_t flac)
{

    BDBG_MSG_TRACE(("bflac_parser_reset>: %#lx", (unsigned long)flac));
    BDBG_OBJECT_ASSERT(flac, bflac_parser_t);
    batom_accum_clear(flac->acc);
    BKNI_Memset(&flac->state, 0, sizeof(flac->state));
    BMEDIA_PARSING_ERRORS_INIT(&flac->state.errors);
    flac->state.state = b_flac_parser_state_top; 
    flac->state.acc_off = 0;
    flac->state.object = 0;
    flac->state.streaminfo_valid = false;
    BDBG_MSG_TRACE(("bflac_parser_reset<: %#lx", (unsigned long)flac));
    return;
}

int 
bflac_parser_seek(bflac_parser_t flac, bflac_off_t off)
{
    bflac_off_t acc_off; 
    size_t acc_len;

    BDBG_OBJECT_ASSERT(flac, bflac_parser_t);
    BDBG_ASSERT(off>=0);

    acc_len = batom_accum_len(flac->acc);
    acc_off = flac->state.acc_off + (int)batom_accum_len(flac->acc);
    if(off == acc_off) {
        BDBG_MSG(("bflac_parser_seek: %p " BDBG_UINT64_FMT " no-op", (void *)flac, BDBG_UINT64_ARG(off)));
        return 0;
    }
    batom_accum_clear(flac->acc);
    
    BDBG_MSG(("bflac_parser_seek: %p " BDBG_UINT64_FMT "(" B_OFFT_FMT ")", (void *)flac, BDBG_UINT64_ARG(off), B_OFFT_ARG(flac->state.acc_off)));
    flac->state.acc_off = off;
    flac->state.state = off == 0 ? b_flac_parser_state_top : b_flac_parser_state_bad_stream;
    return 0;
}


bflac_parser_t 
bflac_parser_create(batom_factory_t factory, const bflac_parser_cfg *cfg)
{
    bflac_parser_t flac;
    BERR_Code rc;

    BDBG_MSG_TRACE(("bflac_parser_create>: %p", (void *)cfg));
    BSTD_UNUSED(rc);

    BDBG_ASSERT(cfg);
    BDBG_ASSERT(factory);

    flac = BKNI_Malloc(sizeof(*flac));
    if (!flac) { 
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc_flac;
    }
    BDBG_OBJECT_INIT(flac, bflac_parser_t);

    flac->cfg = *cfg;
    flac->factory = factory;
    flac->acc = batom_accum_create(factory);
    if(!flac->acc) {
        rc  = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc_accum;
    }
    BLST_S_INIT(&flac->block_handlers);
    bflac_parser_reset(flac);
    BDBG_MSG_TRACE(("bflac_parser_create>: %p %p", (void *)cfg, (void *)flac));
    return flac;

err_alloc_accum:
    BKNI_Free(flac);
err_alloc_flac:
    BDBG_MSG_TRACE(("bflac_parser_create>: %p %p", (void *)cfg, (void *)NULL));
    return NULL;
}

static void
b_flac_empty_stream_error_cb(void *cntx)
{
    BSTD_UNUSED(cntx);
    return ;
}

static bflac_parser_action 
b_flac_discard_frame_cb(void *user_cntx, batom_t frame, const bflac_frame_header *header)
{
    BSTD_UNUSED(user_cntx);
    BSTD_UNUSED(header);
    batom_release(frame);
    return bflac_parser_action_return;
}

void 
bflac_parser_default_cfg(bflac_parser_cfg *cfg)
{
    BDBG_ASSERT(cfg);
    cfg->user_cntx = NULL;
    cfg->stream_error = b_flac_empty_stream_error_cb;
    cfg->frame = b_flac_discard_frame_cb;
    return;
}

void 
bflac_parser_destroy(bflac_parser_t flac)
{
    bflac_parser_handler *handler;

    BDBG_MSG_TRACE(("bflac_destroy>: %#lx", (unsigned long)flac));
    BDBG_OBJECT_ASSERT(flac, bflac_parser_t);
    while(NULL!=(handler=BLST_S_FIRST(&flac->block_handlers))) {
        BDBG_WRN(("bflac_destroy: %#lx lost object_handler %#lx ...", (unsigned long)flac, (unsigned long)handler));
        BLST_S_REMOVE_HEAD(&flac->block_handlers, link);
    }
    bflac_parser_reset(flac);
    batom_accum_destroy(flac->acc);
    BDBG_OBJECT_DESTROY(flac, bflac_parser_t);
    BDBG_MSG_TRACE(("bflac_destroy<: %#lx", (unsigned long)flac));
    BKNI_Free(flac);
    return;
}

static void
b_flac_stream_error(bflac_parser_t flac)
{
    batom_cursor cursor;

    flac->state.state = b_flac_parser_state_top;
    batom_cursor_from_accum(&cursor, flac->acc);
    batom_cursor_skip(&cursor, 1);
    b_flac_trim(flac, &cursor);
    return;
}


size_t
bflac_parser_feed(bflac_parser_t flac, batom_pipe_t pipe)
{
    size_t len;

    BDBG_OBJECT_ASSERT(flac, bflac_parser_t);
    BDBG_MSG_TRACE(("bflac_parser_feed>:%p %p %u", (void *)flac, (void *)pipe, (unsigned)flac->state.acc_off + (unsigned)batom_accum_len(flac->acc)));
    len=0;
    flac->state.parser_return = false;
    for(;;) {
        batom_cursor cursor;
        bool want_continue;

        if(!flac->state.parser_return) {
            batom_t atom;

            atom=batom_pipe_pop(pipe);
            if(!atom) {
                break;
            }
            len += batom_len(atom);
            batom_accum_add_atom(flac->acc, atom); 
            batom_release(atom);
        }
        flac->state.parser_return = false;
        do {
            size_t acc_len = batom_accum_len(flac->acc);
            bflac_parser_handler *handler;
            uint32_t data;

            batom_cursor_from_accum(&cursor, flac->acc);
            want_continue = false;
            switch(flac->state.state) {
            default:
            case b_flac_parser_state_bad_stream:
                batom_cursor_skip(&cursor, acc_len);
                b_flac_trim(flac, &cursor);
                break;
            case b_flac_parser_state_top:
                data = batom_cursor_uint32_le(&cursor);
                if(data != BFLAC_HEADER_TAG) { goto stream_error; }
                b_flac_trim(flac, &cursor);
                flac->state.state = b_flac_parser_state_metadata_parse;
                want_continue = true;
                break;
            case b_flac_parser_state_metadata_parse:
                if(!bflac_parse_metadata_block_header(&cursor, &flac->state.block_header)) { 
                    if(!BATOM_IS_EOF(&cursor)) {
                        goto stream_error;
                    }
                    break;
                }
                if(flac->state.block_header.type == BFLAC_BLOCK_HEADER_INVALID) {
                    BDBG_WRN(("found invalid metadata type %u, switching to payload mode", flac->state.block_header.type));
                    flac->state.state = b_flac_parser_state_frame_parse;
                    want_continue = true;
                    break;
                }
                handler = b_flac_find_block_handler(flac, flac->state.block_header.type);
                want_continue = true;
                if(handler || flac->state.block_header.type==BFLAC_BLOCK_HEADER_STREAMINFO) {
                    batom_cursor block_start;
                    BDBG_MSG_TRACE(("bflac_parser_feed:%p capturing:%u size:%u", (void *)flac, flac->state.block_header.type, (unsigned)flac->state.block_header.length));
                    BATOM_CLONE(&block_start, &cursor);
                    if(batom_cursor_skip(&cursor, flac->state.block_header.length)==flac->state.block_header.length) {
                        batom_t header = batom_accum_extract(flac->acc, &block_start, &cursor, NULL, NULL);
                        BDBG_MSG_TRACE(("bflac_parser_feed:%p captured:%u size:%d", (void *)flac, flac->state.block_header.type, (unsigned)flac->state.block_header.length));
                        b_flac_trim(flac, &cursor);
                        if(header) {
                            if(flac->state.block_header.type==BFLAC_BLOCK_HEADER_STREAMINFO) {
                                batom_cursor cursor_streaminfo;
                                batom_cursor_from_atom(&cursor_streaminfo, header);
                                flac->state.streaminfo_valid = bflac_parse_streaminfo(&cursor_streaminfo, &flac->state.streaminfo);
                            }
                            if(handler) {
                                bflac_parser_action action = handler->handler(handler, header);
                                flac->state.parser_return = action==bflac_parser_action_return;
                                if(flac->state.parser_return) {
                                    goto stopped_parsing;
                                }
                            }
                        }
                    }
                } else {
                    flac->state.object = flac->state.block_header.length + BFLAC_BLOCK_HEADER_LENGTH;
                    flac->state.state = b_flac_parser_state_metadata_discard;
                }
                break;
            case b_flac_parser_state_metadata_discard:
                BDBG_MSG_TRACE(("bflac_parser_feed:%p discarding:%u size:%d(%u)", (void *)flac, flac->state.block_header.type, (unsigned)flac->state.object, (unsigned)flac->state.block_header.length));
                if(acc_len>flac->state.object) {
                    acc_len = flac->state.object;
                }
                batom_cursor_skip(&cursor, acc_len);
                b_flac_trim(flac, &cursor);
                flac->state.object -= acc_len;
                if(flac->state.object==0) {
                    flac->state.state = flac->state.block_header.last ? b_flac_parser_state_frame_parse : b_flac_parser_state_metadata_parse;
                    want_continue = true;
                }
                break;
            case b_flac_parser_state_frame_parse:
                for(;;)
                {
                    bflac_frame_header frame_header;
                    int rc;
                    size_t min_size=0;
                    size_t header_size;
                    rc = bflac_parse_frame_header(&cursor, &frame_header, &flac->state.streaminfo);
                    if(rc<0) {
                        if(rc==BFLAC_ERROR_EOS) {
                            break;
                        }
                        goto stream_error;
                    }
                    header_size = rc;
                    for(;;) {
                        batom_checkpoint start;
                        batom_cursor cursor_peek;
                        bflac_frame_header frame_peek;
                        BATOM_SAVE(&cursor, &start);

                        if(!bflac_parser_find_frame_end(&cursor, min_size, header_size)) {
                            break;
                        }
                        /* found [potential] end of the frame */
                        BATOM_CLONE(&cursor_peek, &cursor);
                        rc = bflac_parse_frame_header(&cursor_peek, &frame_peek, &flac->state.streaminfo);
                        if(rc>0) { /* found good header for the next frame */
                            batom_cursor frame_start;
                            batom_t frame;
                            batom_cursor_from_accum(&frame_start, flac->acc);
                            frame = batom_accum_extract(flac->acc, &frame_start, &cursor, NULL, NULL);
                            b_flac_trim(flac, &cursor);
                            if(frame) {
                                bflac_parser_action action;
                                BDBG_MSG(("extracted frame %p:%u", (void *)frame, (unsigned)batom_len(frame)));
                                action = flac->cfg.frame(flac->cfg.user_cntx, frame, &frame_header);
                                flac->state.parser_return = action==bflac_parser_action_return;
                                BDBG_ASSERT(!flac->state.parser_return);
                                if(flac->state.parser_return) {
                                    goto stopped_parsing;
                                }
                            }
                            break;
                        } else if(rc==BFLAC_ERROR_EOS) {
                            break; /* get more data */
                        }
                        /* header is no good, consider as false end of frame and keep on matching data */
                        min_size = batom_cursor_pos(&cursor);
                        BATOM_ROLLBACK(&cursor, &start);
                        BDBG_MSG(("continue look for the frame end after %u", (unsigned)min_size));
                    }
                }
                break;
stream_error:
                b_flac_stream_error(flac);
                want_continue = true;
                break;
            }
        } while(want_continue);
    }
stopped_parsing:
    return len;
}

void bflac_parser_flush(bflac_parser_t flac)
{
    BDBG_OBJECT_ASSERT(flac, bflac_parser_t);
    batom_accum_clear(flac->acc);
    flac->state.object = 4;
    flac->state.state = b_flac_parser_state_top;
    return;
}
