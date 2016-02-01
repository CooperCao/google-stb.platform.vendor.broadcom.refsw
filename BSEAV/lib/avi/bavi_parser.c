/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 * AVI parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bavi_parser.h"
#include "bavi_util.h"
#include "bkni.h"
#include "blst_list.h"

BDBG_MODULE(bavi_parser);

#define BDBG_MSG_FLOW(x)  /* BDBG_MSG(x) */
#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */

#define B_AVI_MAX_OBJECTS	8

typedef struct b_avi_object {
	bavi_atom size; /* size of the object */
	bavi_atom left; /* number of bytes left in the object */
	bavi_parser_handler *handler; /* current handler for the object */
	bavi_atom fourcc; /* object type */
	size_t	offset; /* offset from the object header */
} b_avi_object;

typedef struct b_avi_object_stack {
	unsigned size;
	b_avi_object stack[B_AVI_MAX_OBJECTS];
} b_avi_object_stack;

typedef enum b_avi_parser_state {
	b_avi_parser_state_parse, /* parse RIFF structure */
	b_avi_parser_state_discard, /* discard current object */
	b_avi_parser_state_capture, /* capture current object */
	b_avi_parser_state_resync, /* detected error in the stream, resyncing */
	b_avi_parser_state_format_error /* error in the format */
} b_avi_parser_state;

BDBG_OBJECT_ID(bavi_parser_t);
struct bavi_parser {
	BDBG_OBJECT(bavi_parser_t)
	batom_accum_t acc;
	batom_factory_t factory;
	BLST_S_HEAD(b_avi_object_handlers, bavi_parser_handler) object_handlers;
	struct {
		b_avi_parser_state state;
		bavi_off_t acc_off; /* stream offset for accumulator */
		bavi_parser_handler *handler; /* handler for the current leaf object */
		bavi_fourcc handler_elem; /* current element */
        bavi_fourcc prev_bad_fourcc;
        bavi_fourcc last_bad_fourcc;
		bool parser_return;
        bool data_discontinuity; /* set to true if some data from the stream has been dropped or lost since the last buffer was fed */
        bmedia_parsing_errors errors;
	} state;
	b_avi_object_stack objects; 
	bavi_parser_cfg cfg;
    bavi_parser_payload_sink sink;
};

#define B_AVI_ASSERT(x)	  BDBG_ASSERT(x)


#define B_AVI_RIFF 	BMEDIA_FOURCC('R','I','F','F')
#define B_AVI_AVI	BMEDIA_FOURCC('A','V','I', ' ')
#define B_AVI_AVIX	BMEDIA_FOURCC('A','V','I', 'X')

#define B_AVI_LIST 	BMEDIA_FOURCC('L','I','S','T')

#define B_AVI_JUNK 	BMEDIA_FOURCC('J','U','N','K')




/* avi object handlers, keep list sorted by the fourcc, it allows O/2 search
 * complexity and ease handle of multiple handlers for the same fourcc code */
void 
bavi_parser_install_handler(bavi_parser_t avi, bavi_parser_handler *handler, bavi_fourcc fourcc, bavi_parser_handler_cb object_handler)
{
	bavi_parser_handler *cur, *prev;

	BDBG_OBJECT_ASSERT(avi, bavi_parser_t);
	BDBG_MSG_TRACE(("bavi_parser_install_handler:> %#lx  %#lx" BMEDIA_FOURCC_FORMAT, (unsigned long)avi, (unsigned long)handler, BMEDIA_FOURCC_ARG(fourcc)));

	for(prev=NULL, cur = BLST_S_FIRST(&avi->object_handlers); cur; cur=BLST_S_NEXT(cur, link)) {
		if (fourcc<cur->fourcc) {
			break;
		}
		prev = cur;
	}
	handler->fourcc = fourcc;
	handler->handler = object_handler;
	if (prev) {
		BLST_S_INSERT_AFTER(&avi->object_handlers, prev, handler, link);
	} else {
		BLST_S_INSERT_HEAD(&avi->object_handlers, handler, link);
	}
	BDBG_MSG_TRACE(("bavi_parser_install_handler:< %#lx  %#lx" BMEDIA_FOURCC_FORMAT, (unsigned long)avi, (unsigned long)handler, BMEDIA_FOURCC_ARG(fourcc)));
	return ;
}

void 
bavi_parser_remove_handler(bavi_parser_t avi, bavi_parser_handler *handler)
{
	BDBG_OBJECT_ASSERT(avi, bavi_parser_t);
	BDBG_ASSERT(handler);
	BDBG_MSG_TRACE(("bavi_parser_remove_handler:> %#lx  %#lx" BMEDIA_FOURCC_FORMAT, (unsigned long)avi, (unsigned long)handler, BMEDIA_FOURCC_ARG(handler->fourcc)));
	BLST_S_REMOVE(&avi->object_handlers, handler, bavi_parser_handler, link);
	BDBG_MSG_TRACE(("bavi_parser_remove_handler:< %#lx  %#lx" BMEDIA_FOURCC_FORMAT, (unsigned long)avi, (unsigned long)handler, BMEDIA_FOURCC_ARG(handler->fourcc)));
	return;
}

static bavi_parser_handler *
b_avi_find_handler(bavi_parser_t avi, bavi_fourcc fourcc)
{
	bavi_parser_handler *cur;

	for(cur = BLST_S_FIRST(&avi->object_handlers); cur; cur=BLST_S_NEXT(cur, link)) {
		if (cur->fourcc == fourcc) {
			return cur;
		}
		if(fourcc < cur->fourcc) {
			return NULL;
		}
	}
	return NULL;
}

static void
b_avi_call_handlers(bavi_parser_t avi, bavi_parser_handler *handler, bavi_fourcc fourcc, batom_cursor *cursor, size_t len)
{
	bavi_parser_handler *cur;
	bavi_parser_action action;

	BSTD_UNUSED(avi);

	for(cur = handler; cur; cur=BLST_S_NEXT(cur, link)) {
		if(cur->fourcc != handler->fourcc) {
			break;
		}
		BDBG_MSG(("user_object: " BMEDIA_FOURCC_FORMAT "[" BMEDIA_FOURCC_FORMAT "] %u -> %#lx", BMEDIA_FOURCC_ARG(handler->fourcc), BMEDIA_FOURCC_ARG(fourcc), (unsigned)len, (unsigned long)handler));
		if (cursor) {
			batom_cursor temp_cursor;
			size_t skip_len;
			batom_t atom;

			BATOM_CLONE(&temp_cursor, cursor);
			skip_len = batom_cursor_skip(&temp_cursor, len);
			BDBG_ASSERT(skip_len==len);
			atom = batom_accum_extract(avi->acc, cursor, &temp_cursor, NULL, NULL);
			BDBG_ASSERT(atom);
			action = cur->handler(cur, fourcc, atom);
		} else {
			action = cur->handler(cur, fourcc, NULL);
		}
		avi->state.parser_return |= action==bavi_parser_action_return;
	}
	return ;
}

/* add new entry into the stack, size if the object size (as in file), and offset number of bytes already consumed */
static bool
b_avi_object_push(bavi_parser_t avi, bavi_fourcc type, bavi_atom size, unsigned offset, bavi_parser_handler *handler)
{
	b_avi_object *obj = &avi->objects.stack[avi->objects.size];
	bavi_parser_action action;

	if(avi->objects.size >= B_AVI_MAX_OBJECTS) {
		return false;
	}
	BDBG_ASSERT(offset<=size);
	obj->size = size;
	obj->left = size-offset;
	obj->handler = handler;
	obj->fourcc = type;
	obj->offset = offset;
	avi->objects.size++;
	b_avi_call_handlers(avi, handler, BAVI_FOURCC_BEGIN, NULL, 0);
	action = avi->cfg.object_begin(avi->cfg.user_cntx, type, size-offset, avi->state.acc_off+(offset+sizeof(bavi_fourcc)+sizeof(bavi_dword)));
	avi->state.parser_return |= action==bavi_parser_action_return;
	BDBG_MSG_TRACE(("push[%u]: left %u size %u", avi->objects.size-1, obj->left, obj->size));
	return true;
}

static void
b_avi_object_pop(bavi_parser_t avi, size_t chunk_size)
{
	BDBG_ASSERT(avi->objects.size > 0 );
	BDBG_ASSERT(avi->objects.size<=B_AVI_MAX_OBJECTS);
	while(avi->objects.size>0) {
		const b_avi_object *prev;
		bavi_parser_action action;

		avi->objects.size--;
		prev =  &avi->objects.stack[avi->objects.size];
		b_avi_call_handlers(avi, prev->handler, BAVI_FOURCC_END, NULL, 0);
		action = avi->cfg.object_end(avi->cfg.user_cntx, prev->fourcc, avi->state.acc_off + chunk_size);
		avi->state.parser_return |= action==bavi_parser_action_return;
		if (avi->objects.size > 0) {
			b_avi_object *obj = &avi->objects.stack[avi->objects.size-1];
			BDBG_MSG_TRACE(("pop[%u]: left %u prev size %u", avi->objects.size-1, obj->left, prev->size));
			B_AVI_ASSERT(obj->left >=  prev->size);
			if (prev->size >= obj->left) {
				/* object completed, delete it */
				continue; 
			}
			obj->left -=  prev->size;
		}
		break;
	}
	return ;
}

static b_avi_object *
b_avi_object_top(bavi_parser_t avi)
{
	b_avi_object *obj = &avi->objects.stack[avi->objects.size-1];
	if(avi->objects.size>0) {
		return obj;
	}
	return NULL;
}

static b_avi_object *
b_avi_object_parent(bavi_parser_t avi)
{
	if(avi->objects.size>1) {
		b_avi_object *obj = &avi->objects.stack[avi->objects.size-2];
		return obj;
	}
	return NULL;
}

static unsigned
b_avi_object_consume(bavi_parser_t avi, unsigned bytes)
{
	b_avi_object *obj = &avi->objects.stack[avi->objects.size-1];
	BDBG_ASSERT(avi->objects.size>0);
	BDBG_ASSERT(avi->objects.size<=B_AVI_MAX_OBJECTS);
	BDBG_MSG_TRACE(("consume[%u] left %u bytes %u", avi->objects.size-1, obj->left, bytes));
	if(obj->left>=bytes)
	  obj->left -= bytes;
	else
	  obj->left = 0;
	return obj->left;
}

static bool
b_avi_verify_fourcc(bavi_parser_t avi, bavi_fourcc fourcc)
{
    if(avi->objects.size>0 && avi->objects.stack[avi->objects.size-1].fourcc  == BMEDIA_FOURCC('m','o','v','i')) {
        switch(BMEDIA_FOURCC(0, 0, 0xFF, 0xFF)&fourcc) {
        case BMEDIA_FOURCC(0, 0, 'd', 'b'):
        case BMEDIA_FOURCC(0, 0, 'd', 'c'):
        case BMEDIA_FOURCC(0, 0, 'd', 'd'):
        case BMEDIA_FOURCC(0, 0, 'd', 'x'):
		case BMEDIA_FOURCC(0, 0, 's', 'b'):
        case BMEDIA_FOURCC(0, 0, 'w', 'b'):
            if( 
                (((BMEDIA_FOURCC(0xFF, 0, 0, 0) & fourcc) >=  BMEDIA_FOURCC('0', 0, 0, 0)) &&  ((BMEDIA_FOURCC(0xFF, 0, 0, 0) & fourcc) <=  BMEDIA_FOURCC('9', 0, 0, 0))) &&
                (((BMEDIA_FOURCC(0, 0xFF, 0, 0) & fourcc) >=  BMEDIA_FOURCC(0, '0', 0, 0)) &&  ((BMEDIA_FOURCC(0, 0xFF, 0, 0) & fourcc) <=  BMEDIA_FOURCC(0, '9', 0, 0)))) {
                return true;
            }
            break;
        default:
            break;
        }
        switch(BMEDIA_FOURCC(0xFF, 0xFF, 0, 0)&fourcc) {
        case BMEDIA_FOURCC('i', 'x', 0, 0):
            if( 
                (((BMEDIA_FOURCC(0, 0, 0xFF, 0 ) & fourcc) >=  BMEDIA_FOURCC(0, 0, '0', 0)) &&  ((BMEDIA_FOURCC(0, 0, 0xFF, 0) & fourcc) <=  BMEDIA_FOURCC(0, 0, '9', 0))) &&
                (((BMEDIA_FOURCC(0, 0, 0, 0xFF ) & fourcc) >=  BMEDIA_FOURCC(0, 0, 0, '0')) &&  ((BMEDIA_FOURCC(0, 0, 0, 0xFF) & fourcc) <=  BMEDIA_FOURCC(0, 0, 0, '9')))) {
                return true;
            }
            break;
        default:
            break;
        }
        switch(fourcc) {
        case BMEDIA_FOURCC('J', 'U', 'N', 'K'):
        case BMEDIA_FOURCC('j', 'u', 'n', 'k'):
            return true;
        default:
            break;
        }
        switch(BMEDIA_FOURCC(0xFF, 0xFF, 0xFF, 0)&fourcc) {
        case BMEDIA_FOURCC('R', 'E', 'S', 0):
        case BMEDIA_FOURCC('r', 'e', 's', 0):
            if(((BMEDIA_FOURCC(0, 0, 0, 0xFF) & fourcc) >=  BMEDIA_FOURCC(0, 0, 0, '0')) &&  ((BMEDIA_FOURCC(0, 0, 0, 0xFF) & fourcc) <=  BMEDIA_FOURCC(0, 0, 0, '9'))) {
                return true;
            }
            break;
        default:
            break;
        }

        avi->state.prev_bad_fourcc = avi->state.last_bad_fourcc;
        if(avi->state.last_bad_fourcc != fourcc) {
            BDBG_WRN(("b_avi_verify_fourcc: %p detecting invalid element %#08x at %u", (void *)avi, (unsigned)fourcc, (unsigned)avi->state.acc_off));
        } else {
            BDBG_MSG(("b_avi_verify_fourcc: %p detecting invalid element %#08x at %u", (void *)avi, (unsigned)fourcc, (unsigned)avi->state.acc_off));
        }
        avi->state.last_bad_fourcc = fourcc;
        return false;
    }
    return true;
}

static bool
b_avi_parse_elem(bavi_parser_t avi, batom_cursor *cursor)
{
	bavi_fourcc elem;
	bavi_atom size;
	unsigned offset=0;
	batom_checkpoint chk;
	bool list=false;
	bavi_parser_handler *handler;
	const b_avi_object *parent;

	BATOM_SAVE(cursor, &chk);

	elem = bavi_read_fourcc(cursor);
	size = bavi_read_atom(cursor);

	if (!BATOM_IS_EOF(cursor)) { 
		if(elem==B_AVI_LIST || elem==B_AVI_RIFF) {
			/* list is a special case, read list type */
			/* BDBG_MSG_TRACE(("found list")); */
			list = true;
			elem = bavi_read_fourcc(cursor);
			offset = sizeof(elem);
		}
	}
	if (BATOM_IS_EOF(cursor)) { 
		goto rollback;
	}
	/* account for the consumed bytes */
	parent = b_avi_object_top(avi);
	BDBG_ASSERT(parent); /* parent shall be valid here */
	b_avi_object_consume(avi, sizeof(elem) + sizeof(size));
    if(list) {
	    BDBG_MSG(("found LIST elem "  BMEDIA_FOURCC_FORMAT " size %u",  BMEDIA_FOURCC_ARG(elem), (unsigned)size));
    } else {
	    BDBG_MSG_TRACE(("found elem "  BMEDIA_FOURCC_FORMAT " size %u",  BMEDIA_FOURCC_ARG(elem), (unsigned)size));
    }

	handler=b_avi_find_handler(avi, elem);
    if(!handler && !list && !b_avi_verify_fourcc(avi, elem)) {
        goto sync_error;
    }
	if(avi->objects.size>0 && avi->objects.stack[avi->objects.size-1].left<size) {
		BDBG_WRN(("b_avi_parse_elem: %#lx invalid object size %u:%u", (unsigned long)avi, (unsigned)size, avi->objects.stack[avi->objects.size-1].left));
        goto sync_error;
	}
	if (b_avi_object_push(avi, elem, size, offset, list?handler:NULL)) {
		/* new element added into the stack */
		if(list) {
			avi->state.state = b_avi_parser_state_parse;
			BDBG_MSG_TRACE(("parse " BMEDIA_FOURCC_FORMAT, BMEDIA_FOURCC_ARG(elem)));
		} else {
			if(handler || parent->handler) {
				/* capture element */
				BDBG_MSG_TRACE(("capture for %#lx " BMEDIA_FOURCC_FORMAT, (unsigned long)handler, BMEDIA_FOURCC_ARG(elem)));
				avi->state.state = b_avi_parser_state_capture;
				avi->state.handler = handler; 
				avi->state.handler_elem = elem; 
			} else {
				/* discard element */
				BDBG_MSG_TRACE(("discard " BMEDIA_FOURCC_FORMAT, BMEDIA_FOURCC_ARG(elem)));
				avi->state.state = b_avi_parser_state_discard;
			}
		}
	} else {
		/* there is no place for the new entry, discard _current_ element */
		BDBG_MSG_TRACE(("discard " BMEDIA_FOURCC_FORMAT, BMEDIA_FOURCC_ARG(elem)));
		avi->state.state = b_avi_parser_state_discard;
	}
	return true;
rollback:
	batom_cursor_rollback(cursor, &chk);
	return false;
sync_error:
    avi->state.state = b_avi_parser_state_resync;
    avi->state.errors.sync_errors++;
    avi->cfg.stream_error(avi->cfg.user_cntx);
    return true;
}

static bool
b_avi_parse_riff(bavi_parser_t avi, batom_cursor *cursor)
{
	bavi_fourcc riff;
	bavi_atom fileSize;
	bavi_fourcc fileType;
	batom_checkpoint chk;
	BATOM_SAVE(cursor, &chk);

	riff = bavi_read_fourcc(cursor);
	fileSize = bavi_read_atom(cursor);
	if (BATOM_IS_EOF(cursor)) { 
		goto rollback;
	}
	BDBG_MSG(("found header "  BMEDIA_FOURCC_FORMAT " size:%u", BMEDIA_FOURCC_ARG(riff), fileSize));
	if (riff==B_AVI_JUNK) {
		b_avi_object_push(avi, riff, fileSize, 0, NULL);
		avi->state.state = b_avi_parser_state_discard;
		return true;
	}

	fileType = bavi_read_fourcc(cursor);

	if (BATOM_IS_EOF(cursor)) { 
		goto rollback;
	}
	BDBG_MSG(("found header "  BMEDIA_FOURCC_FORMAT " type:" BMEDIA_FOURCC_FORMAT " size %u", BMEDIA_FOURCC_ARG(riff), BMEDIA_FOURCC_ARG(fileType), (unsigned)fileSize));
	if (riff==B_AVI_RIFF && (fileType==B_AVI_AVI || fileType==B_AVI_AVIX) && fileSize >= sizeof(bavi_fourcc)) {
		b_avi_object_push(avi, fileType, fileSize, sizeof(fileType), NULL);
	} else {
        /* we parse AVI RIFF files only */
        BDBG_WRN(("%p: invalid AVI file", (void *)avi));
        avi->state.state = b_avi_parser_state_format_error;
        avi->state.errors.format_errors ++;
        avi->cfg.stream_error(avi->cfg.user_cntx);
	}
	return true;
rollback:
	batom_cursor_rollback(cursor, &chk);
	return false; 
}


static void 
b_avi_trim(bavi_parser_t avi, batom_cursor *cursor)
{
	BDBG_MSG_TRACE(("b_avi_trim>: %#lx %u:%u", (unsigned long)avi, batom_accum_len(avi->acc), batom_cursor_pos(cursor) ));
	avi->state.acc_off += batom_cursor_pos(cursor);
	batom_accum_trim(avi->acc, cursor);
	BDBG_MSG_TRACE(("b_avi_trim< %#lx %u", (unsigned long)avi, batom_accum_len(avi->acc)));
	return;
}

void
bavi_parser_get_status(bavi_parser_t avi, bavi_parser_status *status)
{
	const b_avi_object *obj;

	BDBG_OBJECT_ASSERT(avi, bavi_parser_t);
	status->acc_length = batom_accum_len(avi->acc);
	status->offset = avi->state.acc_off  + status->acc_length;
	status->obj_length = 0;
    status->data_discontinuity = avi->state.data_discontinuity;
    status->errors = avi->state.errors;
	obj = b_avi_object_top(avi);
	if(obj) {
		status->obj_length = obj->left;
	} else {
		status->obj_length = 0;
	}

	switch(avi->state.state) {
	case b_avi_parser_state_parse:
		status->state = "parse";
		break;
	case b_avi_parser_state_discard:
		status->state = "discard";
		break;
	case b_avi_parser_state_capture:
		status->state  = "capture";
		break;
	case b_avi_parser_state_format_error:
		status->state = "format_error";
		break;
	case b_avi_parser_state_resync:
		status->state = "resync";
		break;
	}
	return;
}


void
bavi_parser_reset(bavi_parser_t avi)
{
	BDBG_MSG_TRACE(("bavi_reset>: %#lx", (unsigned long)avi));
	BDBG_OBJECT_ASSERT(avi, bavi_parser_t);
	batom_accum_clear(avi->acc);
	BKNI_Memset(&avi->state, 0, sizeof(avi->state));
    BMEDIA_PARSING_ERRORS_INIT(&avi->state.errors);
	avi->state.acc_off = 0;
    avi->state.data_discontinuity = false;
    avi->state.prev_bad_fourcc = avi->state.last_bad_fourcc = BMEDIA_FOURCC(0, 0, 'd', 'b'); /* good one */
	avi->objects.size = 0;
	BDBG_MSG_TRACE(("bavi_reset<: %#lx", (unsigned long)avi));
	return;
}

int 
bavi_parser_seek(bavi_parser_t avi, bavi_off_t off)
{
	bavi_off_t step;
	b_avi_object *obj;
    bavi_off_t acc_off; 
	size_t acc_len;

	BDBG_OBJECT_ASSERT(avi, bavi_parser_t);
	BDBG_ASSERT(off>=0);

    acc_len = batom_accum_len(avi->acc);
    acc_off = avi->state.acc_off + (int)batom_accum_len(avi->acc);
	if(off == acc_off) {
		BDBG_MSG(("bavi_parser_seek: %p " BDBG_UINT64_FMT " no-op", (void *)avi, BDBG_UINT64_ARG(off)));
		return 0;
	}
    if(avi->sink.payload_flush) {
        avi->sink.payload_flush(avi->sink.sink_cntx);
    }
	batom_accum_clear(avi->acc);
	if(b_avi_object_top(avi) && acc_len>0) {
		b_avi_object_consume(avi, acc_len);
	}
    avi->state.data_discontinuity = true;
	avi->state.acc_off = acc_off;
	step = off - acc_off;
	BDBG_MSG(("bavi_parser_seek: %p " BDBG_UINT64_FMT "(" B_OFFT_FMT ")", (void *)avi, BDBG_UINT64_ARG(off), B_OFFT_ARG(step)));
	if(step>0) {

		/* jump forward, skip all completed objects */
		while(NULL != (obj = b_avi_object_top(avi))) {
			size_t size;
			size_t left;

			BDBG_MSG_TRACE(("bavi_parser_seek [%u] " BMEDIA_FOURCC_FORMAT " left %u size %u bytes %u", avi->objects.size-1, BMEDIA_FOURCC_ARG(obj->fourcc), obj->left, obj->size, (unsigned)step));
			size = obj->left;
			if (size > (uint64_t)step) {
				size = step;
				step = 0;
				b_avi_object_consume(avi, size);
				break;
			}
			step -= size;
			left = b_avi_object_consume(avi, size);
			BDBG_ASSERT(left==0);
			b_avi_object_pop(avi, size);
		}
		BDBG_ASSERT(step>=0);

	    if(avi->state.state==b_avi_parser_state_resync) {
            avi->state.errors.resync_events++;
            avi->cfg.stream_error(avi->cfg.user_cntx);
        }
		avi->state.state = b_avi_parser_state_parse;
	} else if(step<0) { /* if we jump back we shall jump to top level object */
		step = -step;
		/* jump backward, skip all empty objects */
		while(NULL != (obj = b_avi_object_top(avi))) {
			size_t size;
			BDBG_MSG_TRACE(("bavi_parser_seek [%u] " BMEDIA_FOURCC_FORMAT " left %u size %u bytes %u", avi->objects.size-1, BMEDIA_FOURCC_ARG(obj->fourcc), obj->left, obj->size, (unsigned)step));
			BDBG_ASSERT(obj->size >= obj->left);
			size = obj->size - obj->left;
			if (size > (uint64_t)step) {
				size = step;
				obj->left += size;
				step = 0;
				break;
			}
			step -= size;
			BDBG_ASSERT(avi->objects.size > 0 );
			BDBG_ASSERT(avi->objects.size<=B_AVI_MAX_OBJECTS);
			while(avi->objects.size>0) {
				const b_avi_object *prev;
				avi->objects.size--;
				prev =  &avi->objects.stack[avi->objects.size];
				avi->cfg.object_end(avi->cfg.user_cntx,  prev->fourcc, 0 );	
				if (avi->objects.size > 0) {
					obj = &avi->objects.stack[avi->objects.size-1];
					BDBG_MSG_TRACE(("bavi_parser_seek pop [%u]: left " BMEDIA_FOURCC_FORMAT " %u size %u, prev " BMEDIA_FOURCC_FORMAT " %u size %u", avi->objects.size-1, BMEDIA_FOURCC_ARG(obj->fourcc), obj->left, obj->size, BMEDIA_FOURCC_ARG(prev->fourcc), prev->left, prev->size));
					if(obj->left == 0 ) {
						continue;
					}
				}
				break;
			}
		}
	    if(avi->state.state==b_avi_parser_state_resync) {
            avi->state.errors.resync_events++;
            avi->cfg.stream_error(avi->cfg.user_cntx);
        }
		avi->state.state = b_avi_parser_state_parse;
	}
	avi->state.acc_off = off;
	return 0;
}


bavi_parser_t 
bavi_parser_create(batom_factory_t factory, const bavi_parser_cfg *cfg)
{
	bavi_parser_t avi;
    BERR_Code rc;

	BDBG_MSG_TRACE(("bavi_create>: %#lx", (unsigned long)cfg));
    BSTD_UNUSED(rc);

	BDBG_CASSERT(sizeof(bavi_byte)==1);
	BDBG_CASSERT(sizeof(bavi_word)==2);
	BDBG_CASSERT(sizeof(bavi_dword)==4);
	BDBG_CASSERT(sizeof(bavi_atom)==4);
	BDBG_CASSERT(sizeof(bavi_fourcc)==4);

	BDBG_ASSERT(cfg);
	BDBG_ASSERT(factory);

	avi = BKNI_Malloc(sizeof(*avi));
	if (!avi) { 
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto err_alloc_avi;
	}
	BDBG_OBJECT_INIT(avi, bavi_parser_t);

	avi->cfg = *cfg;
	avi->factory = factory;
	avi->acc = batom_accum_create(factory);
	if(!avi->acc) {
        rc  = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto err_alloc_accum;
	}
	BLST_S_INIT(&avi->object_handlers);
    bavi_parser_default_payload_sink(&avi->sink);
	bavi_parser_reset(avi);
	BDBG_MSG_TRACE(("bavi_create>: %#lx %#lx", (unsigned long)cfg, (unsigned long)avi));
	return avi;

err_alloc_accum:
	BKNI_Free(avi);
err_alloc_avi:
	BDBG_MSG_TRACE(("bavi_create>: %#lx %#lx", (unsigned long)cfg, (unsigned long)NULL));
	return NULL;
}

static bavi_parser_action 
b_avi_empty_object_begin_cb(void *cntx, bavi_fourcc fourcc, bavi_dword size, bavi_off_t offset)
{
	BSTD_UNUSED(cntx);
	BSTD_UNUSED(fourcc);
	BSTD_UNUSED(size);
	BSTD_UNUSED(offset);
	return bavi_parser_action_none;
}

static bavi_parser_action 
b_avi_empty_object_end_cb(void *cntx, bavi_fourcc fourcc, bavi_off_t offset)
{
	BSTD_UNUSED(cntx);
	BSTD_UNUSED(fourcc);
	BSTD_UNUSED(offset);
	return bavi_parser_action_none;
}

static void
b_avi_empty_stream_error_cb(void *cntx)
{
	BSTD_UNUSED(cntx);
	return ;
}

void 
bavi_parser_default_cfg(bavi_parser_cfg *cfg)
{
	BDBG_ASSERT(cfg);
	cfg->user_cntx = NULL;
	cfg->object_begin = b_avi_empty_object_begin_cb;
	cfg->object_end = b_avi_empty_object_end_cb;
    cfg->stream_error = b_avi_empty_stream_error_cb;
	return;
}

void 
bavi_parser_destroy(bavi_parser_t avi)
{
	bavi_parser_handler *handler;
	BDBG_MSG_TRACE(("bavi_destroy>: %#lx", (unsigned long)avi));
	BDBG_OBJECT_ASSERT(avi, bavi_parser_t);
	for(handler=BLST_S_FIRST(&avi->object_handlers); handler; handler=BLST_S_NEXT(handler, link)) {
		BDBG_WRN(("bavi_destroy: %#lx lost object_handler %#lx ...", (unsigned long)avi, (unsigned long)handler));
		BDBG_WRN(("bavi_destroy: %#lx lost object_handler %#lx " BMEDIA_FOURCC_FORMAT, (unsigned long)avi, (unsigned long)handler, BMEDIA_FOURCC_ARG(handler->fourcc)));
	}
	bavi_parser_reset(avi);
	batom_accum_destroy(avi->acc);
	BDBG_OBJECT_DESTROY(avi, bavi_parser_t);
	BKNI_Free(avi);
	BDBG_MSG_TRACE(("bavi_destroy<: %#lx", (unsigned long)avi));
	return;
}


size_t
bavi_parser_feed(bavi_parser_t avi, batom_pipe_t pipe)
{
	size_t len;


	BDBG_OBJECT_ASSERT(avi, bavi_parser_t);
	BDBG_MSG_TRACE(("bavi_parser_feed>:%#lx %#lx %#lu", (unsigned long)avi, (unsigned long)pipe, (unsigned long)avi->state.acc_off + batom_accum_len(avi->acc)));
	len=0;
	for(;;) {
		b_avi_object *obj;
		batom_cursor cursor;
		bool want_continue;
		size_t size;
		batom_t atom;

		if(!avi->state.parser_return) {
			atom=batom_pipe_pop(pipe);
			if(!atom) {
				break;
			}
			batom_accum_add_atom(avi->acc, atom);
			len += batom_len(atom);
			batom_release(atom);
		}
		avi->state.parser_return = false;
		do {
			size_t acc_len = batom_accum_len(avi->acc);
			batom_cursor_from_accum(&cursor, avi->acc);
			want_continue = false;
			avi->state.data_discontinuity = false;
			switch(avi->state.state) {
			case b_avi_parser_state_parse:

				/* header shall be bavi_word (2 bytes) aligned */
				size = avi->state.acc_off%sizeof(bavi_word);
				if (size && acc_len>=size) {
					BDBG_MSG_TRACE(("aligning stream by %u bytes (%u)", size, (unsigned)avi->state.acc_off));
					BDBG_ASSERT(size==1);
					want_continue = true;
					if (avi->objects.size>0) {
						if(b_avi_object_consume(avi, size) == 0) {
							b_avi_object_pop(avi, size);
						}
					}
					batom_cursor_skip(&cursor, size);
					b_avi_trim(avi, &cursor);
					if(avi->state.parser_return) {
						break;
					}
					batom_cursor_from_accum(&cursor, avi->acc);
				}

				if(avi->objects.size==0) { /* parse the top level object */
					want_continue = b_avi_parse_riff(avi, &cursor);
				} else {
                    obj = b_avi_object_top(avi);
                    BDBG_ASSERT(obj);
                    if(obj->left<sizeof(bavi_fourcc)+sizeof(bavi_atom)) {
                        size_t junk = obj->left;
                        if(junk>0) {
                            BDBG_WRN(("bavi_parser_feed: %#lx reached premature end of object:" BMEDIA_FOURCC_FORMAT ":%d,%d", (unsigned long)avi, BMEDIA_FOURCC_ARG(obj->fourcc),obj->size, obj->left));
                        }
                        if(b_avi_object_consume(avi, junk) == 0) {
                            b_avi_object_pop(avi, junk);
                        }
                        batom_cursor_skip(&cursor, junk);
                        b_avi_trim(avi, &cursor);
                        if(avi->state.parser_return) {
                            break;
                        }
                        batom_cursor_from_accum(&cursor, avi->acc);
                    }
					want_continue = b_avi_parse_elem(avi, &cursor);
				}
				if (want_continue) {
					b_avi_trim(avi, &cursor);
				}
				break;

			case b_avi_parser_state_resync:
				if (avi->objects.size==0) {
					BDBG_WRN(("bavi_parser_feed: %#lx reached bottom of the file", (unsigned long)avi));
					avi->state.state = b_avi_parser_state_parse;
					avi->state.errors.resync_events++;
					avi->cfg.stream_error(avi->cfg.user_cntx);
					continue;
				}
				obj = b_avi_object_top(avi);
				BDBG_ASSERT(obj);
				BDBG_MSG_TRACE(("bavi_parser_feed: resync %u %u", obj->left, acc_len));
				/* header shall be bavi_word (2 bytes) aligned */
				size = avi->state.acc_off%sizeof(bavi_word);
                want_continue = true;
				if (size && acc_len>=size) {
					BDBG_MSG_TRACE(("aligning stream by %u bytes (%u)", size, (unsigned)avi->state.acc_off));
					BDBG_ASSERT(size==1);
					BDBG_ASSERT(avi->objects.size>0);
					if(b_avi_object_consume(avi, size) == 0) {
						b_avi_object_pop(avi, size);
					}
					batom_cursor_skip(&cursor, size);
					b_avi_trim(avi, &cursor);
					if(avi->state.parser_return) {
                        want_continue = false;
						break;
					}
					batom_cursor_from_accum(&cursor, avi->acc);
				}
				{
                    bavi_fourcc elem;
                    bavi_atom elem_size;
	                bavi_atom parent_left;

                    BDBG_ASSERT(avi->objects.size>0);
                    parent_left = avi->objects.stack[avi->objects.size-1].left;
                    size=parent_left;
                    if(parent_left>=sizeof(elem)+sizeof(elem_size)) { 
                        /* there is enough data left to contain an object */
                        for(size=0;size<parent_left;size+=2) {
                            batom_cursor elem_cursor;
                            BATOM_CLONE(&elem_cursor, &cursor);
                            /* advance cursor(scan pointer) by 2 bytes */
                            batom_cursor_skip(&cursor, 2);
                            /* preread element and size */
                            elem = bavi_read_fourcc(&elem_cursor); 
	                        elem_size = bavi_read_atom(&elem_cursor);
                            if (BATOM_IS_EOF(&elem_cursor)) { 
                                want_continue = false;
                                break;
                            }
                            if(    (size+elem_size)<=parent_left /* size is valid */
                                && !(elem_size==0 || elem_size==0x10) /* if size is 0 or 0x10 it's likely we stumbled upon index frame entries */
                                && b_avi_find_handler(avi, elem) /* this is element we carry about */) {
                                if(avi->state.prev_bad_fourcc != avi->state.last_bad_fourcc) {
                                    BDBG_WRN(("bavi_parser_feed: %#lx resynced with " BMEDIA_FOURCC_FORMAT ":%u at %lu in " BMEDIA_FOURCC_FORMAT ":%u", (unsigned long)avi, BMEDIA_FOURCC_ARG(elem), elem_size, (unsigned long)avi->state.acc_off, BMEDIA_FOURCC_ARG(avi->objects.stack[avi->objects.size-1].fourcc), avi->objects.stack[avi->objects.size-1].left));  
                                } else {
                                    BDBG_MSG(("bavi_parser_feed: %#lx resynced with " BMEDIA_FOURCC_FORMAT ":%u at %lu in " BMEDIA_FOURCC_FORMAT ":%u", (unsigned long)avi, BMEDIA_FOURCC_ARG(elem), elem_size, (unsigned long)avi->state.acc_off, BMEDIA_FOURCC_ARG(avi->objects.stack[avi->objects.size-1].fourcc), avi->objects.stack[avi->objects.size-1].left));  
                                }
                                avi->state.state = b_avi_parser_state_parse;
                                avi->state.errors.resync_events++;
                                avi->cfg.stream_error(avi->cfg.user_cntx);
                                batom_cursor_from_accum(&cursor, avi->acc);
                                break;
                            }
                         }
                    }
                    if(b_avi_object_consume(avi, size) == 0) {
				        b_avi_object_pop(avi, size);
				    }
                    batom_cursor_from_accum(&cursor, avi->acc);
                    batom_cursor_skip(&cursor, size);
				    b_avi_trim(avi, &cursor);
				    if(avi->state.parser_return) {
				        want_continue = false;
                    }
				}
				break;

			case b_avi_parser_state_discard:
				obj = b_avi_object_top(avi);
				BDBG_ASSERT(obj);
				BDBG_MSG_TRACE(("bavi_parser_feed: discard %u %u", obj->left, acc_len));
				size = B_AVI_MIN(acc_len, obj->left);
				batom_cursor_skip(&cursor, size);
				if(b_avi_object_consume(avi, size) == 0) {
					b_avi_object_pop(avi, size);
					want_continue = !avi->state.parser_return;
					avi->state.state = b_avi_parser_state_parse;
				}
				b_avi_trim(avi, &cursor);
				break;
			case b_avi_parser_state_capture:
				obj = b_avi_object_top(avi);
				BDBG_ASSERT(obj);
				BDBG_MSG_TRACE(("bavi_parser_feed: capture %u %u", obj->left, acc_len));
				if (acc_len>=obj->left) {
					size_t obj_left = obj->left;
					b_avi_object *parent;
					/* entire object was captured */
					/* BDBG_ASSERT(avi->state.handler); */
					BDBG_MSG_TRACE(("bavi_parser_feed: captured " BMEDIA_FOURCC_FORMAT " %u bytes", BMEDIA_FOURCC_ARG(obj->fourcc), obj_left));
					size = b_avi_object_consume(avi, obj_left); 
					BDBG_ASSERT(size==0);

					/* call handler for this object */
                    if( !(obj->fourcc == BAVI_FOURCC_BEGIN || obj->fourcc == BAVI_FOURCC_END)) {
					b_avi_call_handlers(avi, avi->state.handler, obj->fourcc, &cursor, obj_left);
                    }
					avi->state.handler = NULL; 

					parent = b_avi_object_parent(avi);
					BDBG_ASSERT(parent);
					if(parent->handler) { /* if parent list has a handler */
                        if( !(obj->fourcc == BAVI_FOURCC_BEGIN || obj->fourcc == BAVI_FOURCC_END)) {
						b_avi_call_handlers(avi, parent->handler, obj->fourcc, &cursor, obj_left);
                        }
					}
					b_avi_object_pop(avi, obj_left);

					want_continue = !avi->state.parser_return;

					batom_cursor_skip(&cursor, obj_left);
					b_avi_trim(avi, &cursor);
					avi->state.state = b_avi_parser_state_parse;
					if(!want_continue) {
						goto stopped_parsing;
					}
				} /* else stop parsing and wait for more data */
				break;
			case b_avi_parser_state_format_error:
				/* just drop all data */
				batom_cursor_skip(&cursor, acc_len);
				b_avi_trim(avi, &cursor);
				break;
			}
		} while(want_continue);
	}
stopped_parsing:
	return len;
}

void 
bavi_parser_default_payload_sink(bavi_parser_payload_sink *sink)
{
    BDBG_ASSERT(sink);
    BKNI_Memset(sink, 0, sizeof(*sink));
    sink->sink_cntx = NULL;
    sink->payload_flush = NULL;
    return;
}

void 
bavi_parser_set_payload_sink(bavi_parser_t avi, const bavi_parser_payload_sink *sink)
{
	BDBG_OBJECT_ASSERT(avi, bavi_parser_t);
    BDBG_ASSERT(sink);
    BSTD_UNUSED(sink);
    return;
}

