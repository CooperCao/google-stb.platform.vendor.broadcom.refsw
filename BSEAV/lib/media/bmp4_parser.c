/***************************************************************************
 *     Copyright (c) 2007-2013, Broadcom Corporation
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
 * MP4 container parser library
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

#include "bmp4_parser.h"

BDBG_MODULE(bmp4_parser);

BDBG_OBJECT_ID(bmp4_parser_t);

#define B_MP4_MIN(a,b) ((a)<(b)?(a):(b))

#define BDBG_MSG_TRACE(x) BDBG_MSG(x) 



typedef struct b_mp4_box {
	uint64_t size; /* size of the box */
	uint64_t left; /* number of bytes left in the box */
	bmp4_parser_handler *handler; /* current handler for the box */
	uint32_t type; /* box type */
	size_t	offset; /* offset from the box header */
} b_mp4_box;

#define B_MP4_MAX_BOXES 16
typedef struct b_mp4_box_stack {
	unsigned size;
	b_mp4_box stack[B_MP4_MAX_BOXES];
} b_mp4_box_stack;

typedef enum b_mp4_parser_state {
	b_mp4_parser_state_parse, /* parse box structure */
	b_mp4_parser_state_discard, /* discard current box */
	b_mp4_parser_state_capture, /* capture current box */
	b_mp4_parser_state_format_error /* error in the format */
} b_mp4_parser_state;

#define B_MP4_ASSERT(expr)  do{ if(!(expr)){b_mp4_parser_assert_failed(mp4, #expr, __FILE__, __LINE__); goto mp4_assert_handler;}}while(0) 


struct bmp4_parser {
	BDBG_OBJECT(bmp4_parser_t)
	batom_accum_t acc;
	batom_factory_t factory;
	BLST_S_HEAD(b_mp4_box_handlers, bmp4_parser_handler) box_handlers;
	struct {
		b_mp4_parser_state state;
		uint64_t acc_off; /* stream offset for accumulator */
		bmp4_parser_handler *handler; /* handler for the current leaf box */
		uint32_t handler_type ; /* current box type*/
		bool parser_return;
		bmp4_parser_action box_action;
	} state;
	b_mp4_box_stack boxes; 
	bmp4_parser_cfg cfg;
};

static void 
b_mp4_parser_assert_failed(bmp4_parser_t mp4, const char *expr, const char *file, unsigned line)
{
    BSTD_UNUSED(expr);
    BSTD_UNUSED(file);
    BSTD_UNUSED(line);
    BDBG_ERR(("%#lx B_MP4_ASSERT(%s) Failed at %s:%u", (unsigned long)mp4, expr, file, line));
    return;
}

/* mp4 box_handlers, keep list sorted by the type , it allows O/2 search
 * complexity and ease handle of multiple handlers for the same box type */
void 
bmp4_parser_install_handler(bmp4_parser_t mp4, bmp4_parser_handler *handler, uint32_t type, bmp4_parser_handler_cb box_handler)
{
	bmp4_parser_handler *cur, *prev;

	BDBG_OBJECT_ASSERT(mp4, bmp4_parser_t);
	BDBG_MSG_TRACE(("bmp4_parser_install_handler:> %#lx  %#lx" B_MP4_TYPE_FORMAT, (unsigned long)mp4, (unsigned long)handler, B_MP4_TYPE_ARG(type)));

	for(prev=NULL, cur = BLST_S_FIRST(&mp4->box_handlers); cur; cur=BLST_S_NEXT(cur, link)) {
		if (type<cur->type) {
			break;
		}
		prev = cur;
	}
	handler->type = type;
	handler->handler = box_handler;
	if (prev) {
		BLST_S_INSERT_AFTER(&mp4->box_handlers, prev, handler, link);
	} else {
		BLST_S_INSERT_HEAD(&mp4->box_handlers, handler, link);
	}
	BDBG_MSG_TRACE(("bmp4_parser_install_handler:< %#lx  %#lx" B_MP4_TYPE_FORMAT, (unsigned long)mp4, (unsigned long)handler, B_MP4_TYPE_ARG(type)));
	return ;
}

void 
bmp4_parser_remove_handler(bmp4_parser_t mp4, bmp4_parser_handler *handler)
{
	BDBG_OBJECT_ASSERT(mp4, bmp4_parser_t);
	BDBG_ASSERT(handler);
	BDBG_MSG_TRACE(("bmp4_parser_remove_handler:> %#lx  %#lx" B_MP4_TYPE_FORMAT, (unsigned long)mp4, (unsigned long)handler, B_MP4_TYPE_ARG(handler->type)));
	BLST_S_REMOVE(&mp4->box_handlers, handler, bmp4_parser_handler, link);
	BDBG_MSG_TRACE(("bmp4_parser_remove_handler:< %#lx  %#lx" B_MP4_TYPE_FORMAT, (unsigned long)mp4, (unsigned long)handler, B_MP4_TYPE_ARG(handler->type)));
	return;
}

static bmp4_parser_handler *
b_mp4_find_handler(bmp4_parser_t mp4, uint32_t type)
{
	bmp4_parser_handler *cur;

	for(cur = BLST_S_FIRST(&mp4->box_handlers); cur; cur=BLST_S_NEXT(cur, link)) {
		if (cur->type == type) {
			return cur;
		}
		if(type < cur->type) {
			return NULL;
		}
	}
	return NULL;
}

static void
b_mp4_call_handlers(bmp4_parser_t mp4, bmp4_parser_handler *handler, uint32_t type, batom_cursor *cursor, size_t len)
{
	bmp4_parser_handler *cur;
	bmp4_parser_action action;

	BSTD_UNUSED(mp4);

	for(cur = handler; cur; cur=BLST_S_NEXT(cur, link)) {
		if(cur->type != handler->type) {
			break;
		}
		BDBG_MSG(("user_box: " B_MP4_TYPE_FORMAT "[" B_MP4_TYPE_FORMAT "] %u -> %#lx", B_MP4_TYPE_ARG(handler->type), B_MP4_TYPE_ARG(type), (unsigned)len, (unsigned long)handler));
		if (cursor) {
			batom_cursor temp_cursor;
			size_t skip_len;
			batom_t atom;

			BATOM_CLONE(&temp_cursor, cursor);
			skip_len = batom_cursor_skip(&temp_cursor, len);
			BDBG_ASSERT(skip_len==len);
			atom = batom_accum_extract(mp4->acc, cursor, &temp_cursor, NULL, NULL);
			BDBG_ASSERT(atom);
			action = cur->handler(cur, type, atom);
		} else {
			action = cur->handler(cur, type, NULL);
		}
		mp4->state.parser_return |= action==bmp4_parser_action_return;
	}
	return ;
}

/* add new entry into the stack, size if the box size (as in file), and offset number of bytes already consumed */
static bool
b_mp4_box_push(bmp4_parser_t mp4, const bmp4_box *mp4_box, unsigned header_size, bmp4_parser_handler *handler)
{
	b_mp4_box *box = &mp4->boxes.stack[mp4->boxes.size];
	uint64_t size;

	if(mp4->boxes.size >= B_MP4_MAX_BOXES) {
		return false;
	}
	if(mp4_box->size==0) {
		size = ((uint64_t)1)<<62; /* use rather large number */
	} else {
		size = mp4_box->size;
	}
	BDBG_ASSERT(header_size<=size);
	box->size = size;
	box->left = size-header_size;
	box->type = mp4_box->type;
	box->handler = handler;
	box->offset = header_size;
	mp4->boxes.size++;
	b_mp4_call_handlers(mp4, handler, BMP4_TYPE_BEGIN, NULL, 0);
	mp4->state.box_action = mp4->cfg.box_begin(mp4->cfg.user_cntx, mp4_box->type, size-header_size, mp4->state.acc_off+header_size, header_size);
	mp4->state.parser_return |= mp4->state.box_action==bmp4_parser_action_return;
	BDBG_MSG_TRACE(("push[%u:" B_MP4_TYPE_FORMAT "]: left %u size %u", mp4->boxes.size-1, B_MP4_TYPE_ARG(box->type), (unsigned)box->left, (unsigned)box->size));
	return true;
}

static void
b_mp4_box_pop(bmp4_parser_t mp4, size_t chunk_size)
{
	BDBG_ASSERT(mp4->boxes.size > 0 );
	BDBG_ASSERT(mp4->boxes.size<=B_MP4_MAX_BOXES);
	while(mp4->boxes.size>0) {
		const b_mp4_box *prev;
		bmp4_parser_action action;

		mp4->boxes.size--;
		prev =  &mp4->boxes.stack[mp4->boxes.size];
		b_mp4_call_handlers(mp4, prev->handler, BMP4_TYPE_END, NULL, 0);
		action = mp4->cfg.box_end(mp4->cfg.user_cntx, prev->type, mp4->state.acc_off + chunk_size);
		mp4->state.parser_return |= action==bmp4_parser_action_return;
		if (mp4->boxes.size > 0) {
			b_mp4_box *box = &mp4->boxes.stack[mp4->boxes.size-1];
			BDBG_MSG_TRACE(("pop[%u:" B_MP4_TYPE_FORMAT "]: left %u prev " B_MP4_TYPE_FORMAT " size %u", mp4->boxes.size-1, B_MP4_TYPE_ARG(box->type), (unsigned)box->left, B_MP4_TYPE_ARG(prev->type), (unsigned)prev->size));
			B_MP4_ASSERT(box->left >=  prev->size);
			if (prev->size >= box->left) {
				/* box completed, delete it */
				continue; 
			}
			box->left -=  prev->size;
		}
		break;
mp4_assert_handler:
        /* just delete box */
        continue;
	}
	return ;
}

static b_mp4_box *
b_mp4_box_top(bmp4_parser_t mp4)
{
	b_mp4_box *box = &mp4->boxes.stack[mp4->boxes.size-1];
	if(mp4->boxes.size>0) {
		return box;
	}
	return NULL;
}

static b_mp4_box *
b_mp4_box_parent(bmp4_parser_t mp4)
{
	if(mp4->boxes.size>1) {
		b_mp4_box *box = &mp4->boxes.stack[mp4->boxes.size-2];
		return box;
	}
	return NULL;
}

static size_t
b_mp4_box_consume(bmp4_parser_t mp4, size_t bytes)
{
	b_mp4_box *box = &mp4->boxes.stack[mp4->boxes.size-1];
	BDBG_ASSERT(mp4->boxes.size>0);
	BDBG_ASSERT(mp4->boxes.size<=B_MP4_MAX_BOXES);
	BDBG_MSG_TRACE(("consume[%u:" B_MP4_TYPE_FORMAT "] left %u bytes %u", mp4->boxes.size-1, B_MP4_TYPE_ARG(box->type), (unsigned)box->left, (unsigned)bytes));
	B_MP4_ASSERT(box->left>=bytes);
	box->left -= bytes;
	return box->left;
mp4_assert_handler:
    box->left = 0;
    return box->left;
}

static bool
b_mp4_parse_box(bmp4_parser_t mp4, batom_cursor *cursor)
{
	batom_checkpoint chk;
	bmp4_parser_handler *handler;
	bmp4_box box;
	size_t header_size;
	const b_mp4_box *parent;

	BATOM_SAVE(cursor, &chk);
	header_size = bmp4_parse_box(cursor, &box);
	if(header_size==0) {
		goto rollback;
	}
	BDBG_MSG_TRACE(("found box "  B_MP4_TYPE_FORMAT " size %lu",  B_MP4_TYPE_ARG(box.type), (unsigned long)box.size));
	handler=b_mp4_find_handler(mp4, box.type);
	parent = b_mp4_box_top(mp4);
	if(b_mp4_box_push(mp4, &box, header_size, handler)) {
		/* new box added into the stack */
		switch(box.type) {
		case BMP4_MOVIE:
		case BMP4_TRACK:
		case BMP4_MEDIA:
		case BMP4_MEDIAINFORMATION:
		case BMP4_SAMPLETABLE:
		case BMP4_MOVIE_EXTENDS:
			mp4->state.state = b_mp4_parser_state_parse;
			BDBG_MSG_TRACE(("parse " B_MP4_TYPE_FORMAT, B_MP4_TYPE_ARG(box.type)));
			break;
		default: 
			{
				if(mp4->state.box_action!=bmp4_parser_action_discard && (handler || (parent && parent->handler))) {
					/* capture box */
					BDBG_MSG_TRACE(("capture for %#lx " B_MP4_TYPE_FORMAT, (unsigned long)handler, B_MP4_TYPE_ARG(box.type)));
					mp4->state.state = b_mp4_parser_state_capture;
					mp4->state.handler = handler; 
					mp4->state.handler_type = box.type; 
				} else {
					/* discard box */
					BDBG_MSG_TRACE(("discard " B_MP4_TYPE_FORMAT, B_MP4_TYPE_ARG(box.type)));
					mp4->state.state = b_mp4_parser_state_discard;
				}
			 }
		}
	} else  {
		/* there is no place for the new entry, discard _current_ element */
		BDBG_MSG_TRACE(("discard " B_MP4_TYPE_FORMAT, B_MP4_TYPE_ARG(box.type)));
		mp4->state.state = b_mp4_parser_state_discard;
	}
	return true;
rollback:
	batom_cursor_rollback(cursor, &chk);
	return false; 
}

static void 
b_mp4_trim(bmp4_parser_t mp4, batom_cursor *cursor)
{
	mp4->state.acc_off += batom_cursor_pos(cursor);
	batom_accum_trim(mp4->acc, cursor);
	return;
}

void
bmp4_parser_get_status(bmp4_parser_t mp4, bmp4_parser_status *status)
{
	const b_mp4_box *box;

	BDBG_OBJECT_ASSERT(mp4, bmp4_parser_t);
	status->offset = mp4->state.acc_off;
	status->acc_length = batom_accum_len(mp4->acc);
	status->box_length = 0;
	box = b_mp4_box_top(mp4);
	if(box) {
		status->box_length = box->left;
	} else {
		status->box_length = 0;
	}

	switch(mp4->state.state) {
	case b_mp4_parser_state_parse:
		status->state = "parse";
		break;
	case b_mp4_parser_state_discard:
		status->state = "discard";
		break;
	case b_mp4_parser_state_capture:
		status->state  = "capture";
		break;
	case b_mp4_parser_state_format_error:
		status->state = "format_error";
		break;
	}
	return;
}


void
bmp4_parser_reset(bmp4_parser_t mp4)
{
	BDBG_MSG_TRACE(("bmp4_reset>: %#lx", (unsigned long)mp4));
	BDBG_OBJECT_ASSERT(mp4, bmp4_parser_t);
	BKNI_Memset(&mp4->state, 0, sizeof(mp4->state));
	batom_accum_clear(mp4->acc);
	mp4->state.acc_off = 0;
	mp4->boxes.size = 0;
	BDBG_MSG_TRACE(("bmp4_reset<: %#lx", (unsigned long)mp4));
	return;
}

int 
bmp4_parser_seek(bmp4_parser_t mp4, uint64_t off)
{
	int64_t step;
	b_mp4_box *box;

	BDBG_OBJECT_ASSERT(mp4, bmp4_parser_t);

	if(off == mp4->state.acc_off + (int)batom_accum_len(mp4->acc)) {
		BDBG_MSG(("bmp4_parser_seek: %p " BDBG_UINT64_FMT " no-op", (void *)mp4, BDBG_UINT64_ARG(off)));
		return 0;
	}
	batom_accum_clear(mp4->acc);
	step = off - mp4->state.acc_off;
	BDBG_MSG(("bmp4_parser_seek: %p " BDBG_UINT64_FMT "(" B_OFFT_FMT ")", (void *)mp4, BDBG_UINT64_ARG(off), B_OFFT_ARG(step)));
	if(step>0) {

		/* jump forward, skip all completed boxes */
		while(NULL != (box = b_mp4_box_top(mp4))) {
			size_t size;
			size_t left;

			BDBG_MSG_TRACE(("bmp4_parser_seek [%u] " B_MP4_TYPE_FORMAT " left %u size %u bytes %u", mp4->boxes.size-1, B_MP4_TYPE_ARG(box->type), (unsigned)box->left, (unsigned)box->size, (unsigned)step));
			size = box->left;
			if (size > (uint64_t)step) {
				size = step;
				step = 0;
				b_mp4_box_consume(mp4, size);
				break;
			}
			step -= size;
			left = b_mp4_box_consume(mp4, size);
			BDBG_ASSERT(left==0);
			b_mp4_box_pop(mp4, size);
		}
		BDBG_ASSERT(step>=0);
		mp4->state.state = b_mp4_parser_state_parse;
	} else if(step<0) { /* if we jump back we shall jump to top lovel box */
		step = -step;
		/* jump back, skip all empty boxes */
		while(NULL != (box = b_mp4_box_top(mp4))) {
			size_t size;
			BDBG_MSG_TRACE(("bmp4_parser_seek [%u] " B_MP4_TYPE_FORMAT " left %u size %u bytes %u", mp4->boxes.size-1, B_MP4_TYPE_ARG(box->type), (unsigned)box->left, (unsigned)box->size, (unsigned)step));
			BDBG_ASSERT(box->size >= box->left);
			size = box->size - box->left;
			if (size > (uint64_t)step) {
				size = step;
				box->left += size;
				step = 0;
				break;
			}
			step -= size;
			BDBG_ASSERT(mp4->boxes.size > 0 );
			BDBG_ASSERT(mp4->boxes.size<=B_MP4_MAX_BOXES);
			while(mp4->boxes.size>0) {
				const b_mp4_box *prev;
				mp4->boxes.size--;
				prev =  &mp4->boxes.stack[mp4->boxes.size];
				if (mp4->boxes.size > 0) {
					box = &mp4->boxes.stack[mp4->boxes.size-1];
					BDBG_MSG_TRACE(("bmp4_parser_seek pop [%u]: left " B_MP4_TYPE_FORMAT " %u size %u, prev " B_MP4_TYPE_FORMAT " %u size %u", mp4->boxes.size-1, B_MP4_TYPE_ARG(box->type), (unsigned)box->left, (unsigned)box->size, B_MP4_TYPE_ARG(prev->type), (unsigned)prev->left, (unsigned)prev->size));
					if(box->left == 0 ) {
						continue;
					}
				}
				break;
			}
		}
		mp4->state.state = b_mp4_parser_state_parse;
	}
	mp4->state.acc_off = off;
	return 0;
}

bmp4_parser_t 
bmp4_parser_create(batom_factory_t factory, const bmp4_parser_cfg *cfg)
{
	bmp4_parser_t mp4;

	BDBG_MSG_TRACE(("bmp4_create>: %#lx", (unsigned long)cfg));

	BDBG_CASSERT(sizeof(uint16_t)==2);
	BDBG_CASSERT(sizeof(uint32_t)==4);
	BDBG_CASSERT(sizeof(uint64_t)==8);

	BDBG_ASSERT(cfg);
	BDBG_ASSERT(factory);

	mp4 = BKNI_Malloc(sizeof(*mp4));
	if (!mp4) { 
		BDBG_ERR(("bmp4_create: out of memory %u", (unsigned)sizeof(*mp4)));
		goto err_alloc_mp4;
	}
	BDBG_OBJECT_INIT(mp4, bmp4_parser_t);

	mp4->cfg = *cfg;
	mp4->factory = factory;
	mp4->acc = batom_accum_create(factory);
	if(!mp4->acc) {
		goto err_alloc_accum;
	}
	BLST_S_INIT(&mp4->box_handlers);
	bmp4_parser_reset(mp4);
	BDBG_MSG_TRACE(("bmp4_create>: %#lx %#lx", (unsigned long)cfg, (unsigned long)mp4));
	return mp4;

err_alloc_accum:
	BKNI_Free(mp4);
err_alloc_mp4:
	BDBG_MSG_TRACE(("bmp4_create>: %#lx %#lx", (unsigned long)cfg, (unsigned long)NULL));
	return NULL;
}

static bmp4_parser_action 
b_mp4_empty_box_begin_cb(void *cntx, uint32_t type, uint64_t size, uint64_t offset, size_t header_size)
{
	BSTD_UNUSED(cntx);
	BSTD_UNUSED(type);
	BSTD_UNUSED(size);
	BSTD_UNUSED(offset);
	BSTD_UNUSED(header_size);
	return bmp4_parser_action_none;
}

static bmp4_parser_action 
b_mp4_empty_box_end_cb(void *cntx, uint32_t type, uint64_t offset)
{
	BSTD_UNUSED(cntx);
	BSTD_UNUSED(type);
	BSTD_UNUSED(offset);
	return bmp4_parser_action_none;
}

void 
bmp4_parser_default_cfg(bmp4_parser_cfg *cfg)
{
	BDBG_ASSERT(cfg);
	cfg->user_cntx = NULL;
	cfg->box_begin = b_mp4_empty_box_begin_cb;
	cfg->box_end = b_mp4_empty_box_end_cb;
	return;
}

void 
bmp4_parser_destroy(bmp4_parser_t mp4)
{
	bmp4_parser_handler *handler;
	BDBG_MSG_TRACE(("bmp4_destroy>: %#lx", (unsigned long)mp4));
	BDBG_OBJECT_ASSERT(mp4, bmp4_parser_t);
	for(handler=BLST_S_FIRST(&mp4->box_handlers); handler; handler=BLST_S_NEXT(handler, link)) {
		BDBG_WRN(("bmp4_destroy: %#lx lost box_handler %#lx ...", (unsigned long)mp4, (unsigned long)handler));
		BDBG_WRN(("bmp4_destroy: %#lx lost box_handler %#lx " B_MP4_TYPE_FORMAT, (unsigned long)mp4, (unsigned long)handler, B_MP4_TYPE_ARG(handler->type)));
	}
	bmp4_parser_reset(mp4);
	batom_accum_destroy(mp4->acc);
	BDBG_OBJECT_DESTROY(mp4, bmp4_parser_t);
	BDBG_MSG_TRACE(("bmp4_destroy<: %#lx", (unsigned long)mp4));
	BKNI_Free(mp4);
	return;
}


size_t
bmp4_parser_feed(bmp4_parser_t mp4, batom_pipe_t pipe)
{
	size_t len;
	batom_t atom;
    bool want_continue;


	BDBG_OBJECT_ASSERT(mp4, bmp4_parser_t);
	BDBG_MSG_TRACE(("bmp4_parser_feed>:%p %p %lu", (void *)mp4, (void *)pipe, (unsigned long)mp4->state.acc_off + batom_accum_len(mp4->acc)));
	len=0;
    do {
		b_mp4_box *box;
		batom_cursor cursor;
		size_t size;

		mp4->state.parser_return = false;
	    atom=batom_pipe_pop(pipe);
        if(atom) {
            BDBG_MSG_TRACE(("bmp4_parser_feed:%p adding atom %p(%u)", (void *)mp4, (void *)atom, (unsigned)batom_len(atom)));

            batom_accum_add_atom(mp4->acc, atom);
            len += batom_len(atom);
            batom_release(atom);
        }
        do {
            size_t acc_len = batom_accum_len(mp4->acc);

            batom_cursor_from_accum(&cursor, mp4->acc);
            want_continue = false;
            if(acc_len==0 && atom==NULL) {
                goto stopped_parsing;
            }
			switch(mp4->state.state) {
			case b_mp4_parser_state_parse:

				want_continue = b_mp4_parse_box(mp4, &cursor);
				if (want_continue) {
					b_mp4_trim(mp4, &cursor);
				}
				break;
			case b_mp4_parser_state_discard:
				box = b_mp4_box_top(mp4);
				BDBG_ASSERT(box);
				BDBG_MSG_TRACE(("bmp4_parser_feed: discard %u %u", (unsigned)box->left, (unsigned)acc_len));
				size = B_MP4_MIN(acc_len, box->left);
				batom_cursor_skip(&cursor, size);
				if(b_mp4_box_consume(mp4, size) == 0) {
					b_mp4_box_pop(mp4, size);
					want_continue = !mp4->state.parser_return;
					mp4->state.state = b_mp4_parser_state_parse;
				}
				b_mp4_trim(mp4, &cursor);
				break;
			case b_mp4_parser_state_capture:
				box = b_mp4_box_top(mp4);
				BDBG_ASSERT(box);
				BDBG_MSG_TRACE(("bmp4_parser_feed: capture %u %u", (unsigned)box->left, (unsigned)acc_len));
				if (acc_len>=box->left) {
					size_t box_left = box->left;
					b_mp4_box *parent;
					/* entire box was captured */
					/* BDBG_ASSERT(mp4->state.handler); */
					BDBG_MSG_TRACE(("bmp4_parser_feed: captured " B_MP4_TYPE_FORMAT " %u bytes", B_MP4_TYPE_ARG(box->type), (unsigned)box_left));
					size = b_mp4_box_consume(mp4, box_left); 
					BDBG_ASSERT(size==0);

					/* call handler for this box */
					b_mp4_call_handlers(mp4, mp4->state.handler, box->type, &cursor, box_left);
					mp4->state.handler = NULL; 

					parent = b_mp4_box_parent(mp4);
					if(parent && parent->handler) { /* if parent box has a handler */
						b_mp4_call_handlers(mp4, parent->handler, box->type, &cursor, box_left);
					}
					b_mp4_box_pop(mp4, box_left);

					want_continue = !mp4->state.parser_return;

					batom_cursor_skip(&cursor, box_left);
					b_mp4_trim(mp4, &cursor);
					mp4->state.state = b_mp4_parser_state_parse;
					if(!want_continue) {
						goto stopped_parsing;
					}
				} /* else stop parsing and wait for more data */
				break;
			case b_mp4_parser_state_format_error:
				/* just drop all data */
				batom_cursor_skip(&cursor, acc_len);
				b_mp4_trim(mp4, &cursor);
				break;
			}
		} while(want_continue);
        if(atom==NULL && mp4->state.state != b_mp4_parser_state_parse) {
            break;
        }
    } while(want_continue);
stopped_parsing:
	return len;
}


