/***************************************************************************
 *     Copyright (c) 2007-2008, Broadcom Corporation
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
 * MKV container parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "blst_slist.h"
#include "biobits.h"
#include "bkni.h"

#include "bmkv_parser.h"

BDBG_MODULE(bmkv_parser);

BDBG_OBJECT_ID(bmkv_parser_t);
#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */
#define B_MKV_MIN(a,b) ((a)<(b)?(a):(b))


typedef struct b_mkv_header {
	uint64_t size; /* size of the header */
	uint64_t left; /* number of bytes left in the header */
	bmkv_parser_handler *handler; /* current handler for the header */
	bmkv_id id; /* header id */
} b_mkv_header;

#define B_MKV_MAX_ELEMENTES 16
typedef struct b_mkv_header_stack {
	unsigned size;
	b_mkv_header stack[B_MKV_MAX_ELEMENTES];
} b_mkv_header_stack;

typedef enum b_mkv_parser_state {
	b_mkv_parser_state_parse, /* parse header structure */
	b_mkv_parser_state_discard, /* discard current header */
	b_mkv_parser_state_capture, /* capture current header */
	b_mkv_parser_state_format_error /* error in the format */
} b_mkv_parser_state;

#define B_MKV_ASSERT(x) BDBG_ASSERT(x)


struct bmkv_parser {
	BDBG_OBJECT(bmkv_parser_t)
	batom_accum_t acc;
	batom_factory_t factory;
	BLST_S_HEAD(b_mkv_header_handlers, bmkv_parser_handler) header_handlers;
	struct {
		b_mkv_parser_state state;
		uint64_t acc_off; /* stream offset for accumulator */
		bmkv_parser_handler *handler; /* handler for the current leaf header */
		uint32_t handler_id ; /* current header id*/
		bool parser_return;
		bmkv_parser_action header_action;
	} state;
	b_mkv_header_stack elements; 
	bmkv_parser_cfg cfg;
};

#define B_MKV_ID_FORMAT " %#x"
#define B_MKV_ID_ARG(id) (unsigned)(id)

/* mkv header_handlers, keep list sorted by the id , it allows O/2 search
 * complexity and ease handle of multiple handlers for the same header id */
void 
bmkv_parser_install_handler(bmkv_parser_t mkv, bmkv_parser_handler *handler, uint32_t id, bmkv_parser_handler_cb header_handler)
{
	bmkv_parser_handler *cur, *prev;

	BDBG_OBJECT_ASSERT(mkv, bmkv_parser_t);
	BDBG_MSG_TRACE(("bmkv_parser_install_handler:> %#lx  %#lx" B_MKV_ID_FORMAT, (unsigned long)mkv, (unsigned long)handler, B_MKV_ID_ARG(id)));

	for(prev=NULL, cur = BLST_S_FIRST(&mkv->header_handlers); cur; cur=BLST_S_NEXT(cur, link)) {
        BDBG_ASSERT(handler != cur); /* check that we don't insert duplicate handlers */
		if (id<cur->id) {
			break;
		}
		prev = cur;
	}
	handler->id = id;
	handler->handler = header_handler;
	if (prev) {
		BLST_S_INSERT_AFTER(&mkv->header_handlers, prev, handler, link);
	} else {
		BLST_S_INSERT_HEAD(&mkv->header_handlers, handler, link);
	}
	BDBG_MSG_TRACE(("bmkv_parser_install_handler:< %#lx  %#lx" B_MKV_ID_FORMAT, (unsigned long)mkv, (unsigned long)handler, B_MKV_ID_ARG(id)));
	return ;
}

void 
bmkv_parser_remove_handler(bmkv_parser_t mkv, bmkv_parser_handler *handler)
{
	BDBG_OBJECT_ASSERT(mkv, bmkv_parser_t);
	BDBG_ASSERT(handler);
	BDBG_MSG_TRACE(("bmkv_parser_remove_handler:> %#lx  %#lx" B_MKV_ID_FORMAT, (unsigned long)mkv, (unsigned long)handler, B_MKV_ID_ARG(handler->id)));
	BLST_S_REMOVE(&mkv->header_handlers, handler, bmkv_parser_handler, link);
	BDBG_MSG_TRACE(("bmkv_parser_remove_handler:< %#lx  %#lx" B_MKV_ID_FORMAT, (unsigned long)mkv, (unsigned long)handler, B_MKV_ID_ARG(handler->id)));
	return;
}

static bmkv_parser_handler *
b_mkv_find_handler(bmkv_parser_t mkv, uint32_t id)
{
	bmkv_parser_handler *cur;

	for(cur = BLST_S_FIRST(&mkv->header_handlers); cur; cur=BLST_S_NEXT(cur, link)) {
		if (cur->id == id) {
			return cur;
		}
		if(id < cur->id) {
			return NULL;
		}
	}
	return NULL;
}

static void
b_mkv_call_handlers(bmkv_parser_t mkv, bmkv_parser_handler *handler, uint32_t id, batom_cursor *cursor, size_t len)
{
	bmkv_parser_handler *cur;
	bmkv_parser_action action;

	BSTD_UNUSED(mkv);

	for(cur = handler; cur; cur=BLST_S_NEXT(cur, link)) {
		if(cur->id != handler->id) {
			break;
		}
		BDBG_MSG(("user_header: " B_MKV_ID_FORMAT "[" B_MKV_ID_FORMAT "] %u -> %#lx", B_MKV_ID_ARG(handler->id), B_MKV_ID_ARG(id), (unsigned)len, (unsigned long)handler));
		if (cursor) {
			batom_cursor temp_cursor;
			size_t skip_len;
			batom_t atom;

			BATOM_CLONE(&temp_cursor, cursor);
			skip_len = batom_cursor_skip(&temp_cursor, len);
			BDBG_ASSERT(skip_len==len);
			atom = batom_accum_extract(mkv->acc, cursor, &temp_cursor, NULL, NULL);
			BDBG_ASSERT(atom);
			action = cur->handler(cur, id, atom);
		} else {
			action = cur->handler(cur, id, NULL);
		}
		mkv->state.parser_return |= action==bmkv_parser_action_return;
	}
	return ;
}

/* add new entry into the stack, size if the header size (as in file), and offset number of bytes already consumed */
static bool
b_mkv_header_push(bmkv_parser_t mkv, const bmkv_header *mkv_header, bmkv_parser_handler *handler, unsigned offset)
{
	b_mkv_header *header = &mkv->elements.stack[mkv->elements.size];

	if(mkv->elements.size >= B_MKV_MAX_ELEMENTES) {
		return false;
	}
    header->size = mkv_header->size;
	header->left = mkv_header->size;
	header->id = mkv_header->id;
	header->handler = handler;
	mkv->elements.size++;
	mkv->state.header_action = mkv->cfg.element_begin(mkv->cfg.user_cntx, mkv_header->id, mkv_header->size, mkv->state.acc_off+offset);
	mkv->state.parser_return |= mkv->state.header_action==bmkv_parser_action_return;
	BDBG_MSG_TRACE(("push[%u:" B_MKV_ID_FORMAT "]: left %u size %u", mkv->elements.size-1, B_MKV_ID_ARG(header->id), header->left, header->size));
	return true;
}

static void
b_mkv_header_pop(bmkv_parser_t mkv, size_t chunk_size)
{
	BDBG_ASSERT(mkv->elements.size > 0 );
	BDBG_ASSERT(mkv->elements.size<=B_MKV_MAX_ELEMENTES);
	while(mkv->elements.size>0) {
		const b_mkv_header *prev;
		bmkv_parser_action action;

		mkv->elements.size--;
		prev =  &mkv->elements.stack[mkv->elements.size];
		action = mkv->cfg.element_end(mkv->cfg.user_cntx, prev->id, mkv->state.acc_off + chunk_size);
		mkv->state.parser_return |= action==bmkv_parser_action_return;
		if (mkv->elements.size > 0) {
			b_mkv_header *header = &mkv->elements.stack[mkv->elements.size-1];
			BDBG_MSG_TRACE(("pop[%u:" B_MKV_ID_FORMAT "]: left %u prev " B_MKV_ID_FORMAT " size %u", mkv->elements.size-1, B_MKV_ID_ARG(header->id), header->left, B_MKV_ID_ARG(prev->id), prev->size));
			B_MKV_ASSERT(header->left >=  prev->size);
			if (prev->size >= header->left) {
				/* header completed, delete it */
				continue; 
			}
			header->left -=  prev->size;
		}
		break;
	}
	return ;
}

static b_mkv_header *
b_mkv_header_top(bmkv_parser_t mkv)
{
	b_mkv_header *header = &mkv->elements.stack[mkv->elements.size-1];
	if(mkv->elements.size>0) {
		return header;
	}
	return NULL;
}

static size_t
b_mkv_header_consume(bmkv_parser_t mkv, size_t bytes)
{
	b_mkv_header *header = &mkv->elements.stack[mkv->elements.size-1];
	BDBG_ASSERT(mkv->elements.size>0);
	BDBG_ASSERT(mkv->elements.size<=B_MKV_MAX_ELEMENTES);
	BDBG_MSG_TRACE(("consume[%u:" B_MKV_ID_FORMAT "] left %u bytes %u", mkv->elements.size-1, B_MKV_ID_ARG(header->id), header->left, bytes));
	B_MKV_ASSERT(header->left>=bytes);
	header->left -= bytes;
	return header->left;
}

static bool
b_mkv_parse_header(bmkv_parser_t mkv, batom_cursor *cursor)
{
	batom_checkpoint chk;
	bmkv_parser_handler *handler;
	bmkv_header header;
	const b_mkv_header *parent;

	BATOM_SAVE(cursor, &chk);

	if(!bmkv_parse_header(cursor, &header)) {
		goto rollback;
	}
    if(header.id==BMKV_INVALID_ID || header.size==BMKV_INVALID_SIZE) {
        goto stream_error;
    }
	BDBG_MSG_TRACE(("found header "  B_MKV_ID_FORMAT " size %lu",  B_MKV_ID_ARG(header.id), (unsigned long)header.size));
	handler = b_mkv_find_handler(mkv, header.id);
	parent = b_mkv_header_top(mkv);
	if(b_mkv_header_push(mkv, &header, handler, batom_cursor_pos(cursor))) {
		/* new header added into the stack */
		switch(header.id) {
        case BMKV_SEGMENT_ID:
			mkv->state.state = b_mkv_parser_state_parse;
			BDBG_MSG_TRACE(("parse " B_MKV_ID_FORMAT, B_MKV_ID_ARG(header.id)));
			break;
		default: 
			{
				if(mkv->state.header_action!=bmkv_parser_action_discard && (handler || (parent && parent->handler))) {
					/* capture header */
					BDBG_MSG_TRACE(("capture for %#lx " B_MKV_ID_FORMAT, (unsigned long)handler, B_MKV_ID_ARG(header.id)));
					mkv->state.state = b_mkv_parser_state_capture;
					mkv->state.handler = handler; 
					mkv->state.handler_id = header.id; 
				} else {
					/* discard header */
					BDBG_MSG_TRACE(("discard " B_MKV_ID_FORMAT, B_MKV_ID_ARG(header.id)));
					mkv->state.state = b_mkv_parser_state_discard;
				}
			 }
		}
	} else  {
		/* there is no place for the new entry, discard _current_ element */
		BDBG_MSG_TRACE(("discard " B_MKV_ID_FORMAT, B_MKV_ID_ARG(header.id)));
		mkv->state.state = b_mkv_parser_state_discard;
	}
	return true;
rollback:
	batom_cursor_rollback(cursor, &chk);
stream_error:
	return false; 
}

static void 
b_mkv_trim(bmkv_parser_t mkv, batom_cursor *cursor)
{
	mkv->state.acc_off += batom_cursor_pos(cursor);
	batom_accum_trim(mkv->acc, cursor);
	return;
}

void
bmkv_parser_get_status(bmkv_parser_t mkv, bmkv_parser_status *status)
{
	const b_mkv_header *header;

	BDBG_OBJECT_ASSERT(mkv, bmkv_parser_t);
	status->offset = mkv->state.acc_off;
	status->acc_length = batom_accum_len(mkv->acc);
	status->element_length = 0;
	header = b_mkv_header_top(mkv);
	if(header) {
		status->element_length = header->left;
	} else {
		status->element_length = 0;
	}

	switch(mkv->state.state) {
	case b_mkv_parser_state_parse:
		status->state = "parse";
		break;
	case b_mkv_parser_state_discard:
		status->state = "discard";
		break;
	case b_mkv_parser_state_capture:
		status->state  = "capture";
		break;
	case b_mkv_parser_state_format_error:
		status->state = "format_error";
		break;
	}
	return;
}


void
bmkv_parser_reset(bmkv_parser_t mkv)
{
	BDBG_MSG_TRACE(("bmkv_reset>: %#lx", (unsigned long)mkv));
	BDBG_OBJECT_ASSERT(mkv, bmkv_parser_t);
	BKNI_Memset(&mkv->state, 0, sizeof(mkv->state));
	batom_accum_clear(mkv->acc);
	mkv->state.acc_off = 0;
	mkv->elements.size = 0;
	BDBG_MSG_TRACE(("bmkv_reset<: %#lx", (unsigned long)mkv));
	return;
}

int 
bmkv_parser_seek(bmkv_parser_t mkv, uint64_t off)
{
	int64_t step;
	b_mkv_header *header;

	BDBG_OBJECT_ASSERT(mkv, bmkv_parser_t);

	if(off == mkv->state.acc_off + (int)batom_accum_len(mkv->acc)) {
		BDBG_MSG(("bmkv_parser_seek: %p " BDBG_UINT64_FMT " no-op", (void *)mkv, BDBG_UINT64_ARG(off)));
		return 0;
	}
	batom_accum_clear(mkv->acc);
	step = off - mkv->state.acc_off;
	BDBG_MSG(("bmkv_parser_seek: %p " BDBG_UINT64_FMT "(" B_OFFT_FMT ")", (void *)mkv, BDBG_UINT64_ARG(off), B_OFFT_ARG(step)));
	if(step>0) {

		/* jump forward, skip all completed elements */
		while(NULL != (header = b_mkv_header_top(mkv))) {
			size_t size;
			size_t left;

			BDBG_MSG_TRACE(("bmkv_parser_seek [%u] " B_MKV_ID_FORMAT " left %u size %u bytes %u", mkv->elements.size-1, B_MKV_ID_ARG(header->id), header->left, header->size, (unsigned)step));
			size = header->left;
			if (size > (uint64_t)step) {
				size = step;
				step = 0;
				b_mkv_header_consume(mkv, size);
				break;
			}
			step -= size;
			left = b_mkv_header_consume(mkv, size);
			BDBG_ASSERT(left==0);
			b_mkv_header_pop(mkv, size);
		}
		BDBG_ASSERT(step>=0);
		mkv->state.state = b_mkv_parser_state_parse;
	} else if(step<0) { /* if we jump back we shall jump to top level header */
		step = -step;
		/* jump back, skip all empty elements */
		while(NULL != (header = b_mkv_header_top(mkv))) {
			size_t size;
			BDBG_MSG_TRACE(("bmkv_parser_seek [%u] " B_MKV_ID_FORMAT " left %u size %u bytes %u", mkv->elements.size-1, B_MKV_ID_ARG(header->id), header->left, header->size, (unsigned)step));
			BDBG_ASSERT(header->size >= header->left);
			size = header->size - header->left;
			if (size > (uint64_t)step) {
				size = step;
				header->left += size;
				step = 0;
				break;
			}
			step -= size;
			BDBG_ASSERT(mkv->elements.size > 0 );
			BDBG_ASSERT(mkv->elements.size<=B_MKV_MAX_ELEMENTES);
			while(mkv->elements.size>0) {
				const b_mkv_header *prev;
				mkv->elements.size--;
				prev =  &mkv->elements.stack[mkv->elements.size];
				if (mkv->elements.size > 0) {
					header = &mkv->elements.stack[mkv->elements.size-1];
					BDBG_MSG_TRACE(("bmkv_parser_seek pop [%u]: left " B_MKV_ID_FORMAT " %u size %u, prev " B_MKV_ID_FORMAT " %u size %u", mkv->elements.size-1, B_MKV_ID_ARG(header->id), header->left, header->size, B_MKV_ID_ARG(prev->id), prev->left, prev->size));
					if(header->left == 0 ) {
						continue;
					}
				}
				break;
			}
		}
		mkv->state.state = b_mkv_parser_state_parse;
	}
	mkv->state.acc_off = off;
	return 0;
}

bmkv_parser_t 
bmkv_parser_create(batom_factory_t factory, const bmkv_parser_cfg *cfg)
{
	bmkv_parser_t mkv;

	BDBG_MSG_TRACE(("bmkv_create>: %#lx", (unsigned long)cfg));

	BDBG_CASSERT(sizeof(uint16_t)==2);
	BDBG_CASSERT(sizeof(uint32_t)==4);
	BDBG_CASSERT(sizeof(uint64_t)==8);

	BDBG_ASSERT(cfg);
	BDBG_ASSERT(factory);

	mkv = BKNI_Malloc(sizeof(*mkv));
	if (!mkv) { 
		BDBG_ERR(("bmkv_create: out of memory %u", (unsigned)sizeof(*mkv)));
		goto err_alloc_mkv;
	}
	BDBG_OBJECT_INIT(mkv, bmkv_parser_t);

	mkv->cfg = *cfg;
	mkv->factory = factory;
	mkv->acc = batom_accum_create(factory);
	if(!mkv->acc) {
		goto err_alloc_accum;
	}
	BLST_S_INIT(&mkv->header_handlers);
	bmkv_parser_reset(mkv);
	BDBG_MSG_TRACE(("bmkv_create>: %#lx %#lx", (unsigned long)cfg, (unsigned long)mkv));
	return mkv;

err_alloc_accum:
	BKNI_Free(mkv);
err_alloc_mkv:
	BDBG_MSG_TRACE(("bmkv_create>: %#lx %#lx", (unsigned long)cfg, (unsigned long)NULL));
	return NULL;
}

static bmkv_parser_action 
b_mkv_empty_element_begin_cb(void *cntx, uint32_t id, uint64_t size, uint64_t offset)
{
	BSTD_UNUSED(cntx);
	BSTD_UNUSED(id);
	BSTD_UNUSED(size);
	BSTD_UNUSED(offset);
	return bmkv_parser_action_none;
}

static bmkv_parser_action 
b_mkv_empty_element_end_cb(void *cntx, uint32_t id, uint64_t offset)
{
	BSTD_UNUSED(cntx);
	BSTD_UNUSED(id);
	BSTD_UNUSED(offset);
	return bmkv_parser_action_none;
}

void 
bmkv_parser_default_cfg(bmkv_parser_cfg *cfg)
{
	BDBG_ASSERT(cfg);
	cfg->user_cntx = NULL;
	cfg->element_begin = b_mkv_empty_element_begin_cb;
	cfg->element_end = b_mkv_empty_element_end_cb;
	return;
}

void 
bmkv_parser_destroy(bmkv_parser_t mkv)
{
	bmkv_parser_handler *handler;
	BDBG_MSG_TRACE(("bmkv_destroy>: %#lx", (unsigned long)mkv));
	BDBG_OBJECT_ASSERT(mkv, bmkv_parser_t);
	for(handler=BLST_S_FIRST(&mkv->header_handlers); handler; handler=BLST_S_NEXT(handler, link)) {
		BDBG_WRN(("bmkv_destroy: %#lx lost header_handler %#lx ...", (unsigned long)mkv, (unsigned long)handler));
		BDBG_WRN(("bmkv_destroy: %#lx lost header_handler %#lx " B_MKV_ID_FORMAT, (unsigned long)mkv, (unsigned long)handler, B_MKV_ID_ARG(handler->id)));
	}
	bmkv_parser_reset(mkv);
	batom_accum_destroy(mkv->acc);
	BDBG_OBJECT_DESTROY(mkv, bmkv_parser_t);
	BDBG_MSG_TRACE(("bmkv_destroy<: %#lx", (unsigned long)mkv));
	BKNI_Free(mkv);
	return;
}


size_t
bmkv_parser_feed(bmkv_parser_t mkv, batom_pipe_t pipe)
{
	size_t len;
	batom_t atom;


	BDBG_OBJECT_ASSERT(mkv, bmkv_parser_t);
	BDBG_MSG_TRACE(("bmkv_parser_feed>:%#lx %#lx %#lu", (unsigned long)mkv, (unsigned long)pipe, (unsigned long)mkv->state.acc_off + batom_accum_len(mkv->acc)));
	len=0;
	while(NULL!=(atom=batom_pipe_pop(pipe))) {
		b_mkv_header *header;
		batom_cursor cursor;
		bool want_continue;
		size_t size;

		mkv->state.parser_return = false;

		batom_accum_add_atom(mkv->acc, atom);
		batom_release(atom);
		len += batom_len(atom);
		do {
			size_t acc_len = batom_accum_len(mkv->acc);
			batom_cursor_from_accum(&cursor, mkv->acc);
			want_continue = false;
			switch(mkv->state.state) {
			case b_mkv_parser_state_parse:

				want_continue = b_mkv_parse_header(mkv, &cursor);
				if (want_continue) {
					b_mkv_trim(mkv, &cursor);
				}
				break;
			case b_mkv_parser_state_discard:
				header = b_mkv_header_top(mkv);
				BDBG_ASSERT(header);
				BDBG_MSG_TRACE(("bmkv_parser_feed: discard %u %u", header->left, acc_len));
				size = B_MKV_MIN(acc_len, header->left);
				batom_cursor_skip(&cursor, size);
				if(b_mkv_header_consume(mkv, size) == 0) {
					b_mkv_header_pop(mkv, size);
					want_continue = !mkv->state.parser_return;
					mkv->state.state = b_mkv_parser_state_parse;
				}
				b_mkv_trim(mkv, &cursor);
				break;
			case b_mkv_parser_state_capture:
				header = b_mkv_header_top(mkv);
				BDBG_ASSERT(header);
				BDBG_MSG_TRACE(("bmkv_parser_feed: capture %u %u", header->left, acc_len));
				if (acc_len>=header->left) {
					size_t element_left = header->left;
					/* entire headerect was captured */
					/* BDBG_ASSERT(mkv->state.handler); */
					BDBG_MSG_TRACE(("bmkv_parser_feed: captured " B_MKV_ID_FORMAT " %u bytes", B_MKV_ID_ARG(header->id), element_left));
					size = b_mkv_header_consume(mkv, element_left); 
					BDBG_ASSERT(size==0);

					/* call handler for this element */
					b_mkv_call_handlers(mkv, mkv->state.handler, header->id, &cursor, element_left);
					mkv->state.handler = NULL; 

					b_mkv_header_pop(mkv, element_left);

					want_continue = !mkv->state.parser_return;

					batom_cursor_skip(&cursor, element_left);
					b_mkv_trim(mkv, &cursor);
					mkv->state.state = b_mkv_parser_state_parse;
					if(!want_continue) {
						goto stopped_parsing;
					}
				} /* else stop parsing and wait for more data */
				break;
			case b_mkv_parser_state_format_error:
				/* just drop all data */
				batom_cursor_skip(&cursor, acc_len);
				b_mkv_trim(mkv, &cursor);
				break;
			}
		} while(want_continue);
	}
stopped_parsing:
	return len;
}


