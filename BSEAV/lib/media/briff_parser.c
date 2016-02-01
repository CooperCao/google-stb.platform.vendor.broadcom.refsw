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
 * RIFF parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#include "bstd.h"
#include "briff_parser.h"
#include "bkni.h"

BDBG_MODULE(briff_parser);

#define BDBG_MSG_FLOW(x)  BDBG_MSG(x) 
#define BDBG_MSG_TRACE(x) BDBG_MSG(x) 

#define B_RIFF_MAX_OBJECTS	8

#define B_RIFF_MIN(a,b) ((a)<(b)?(a):(b))


typedef struct b_riff_object {
	briff_atom size; /* size of the object */
	briff_atom left; /* number of bytes left in the object */
	briff_parser_handler *handler; /* current handler for the object */
	briff_atom chunk_id; /* object type */
	size_t	offset; /* offset from the object header */
} b_riff_object;

typedef struct b_riff_object_stack {
	unsigned size;
	b_riff_object stack[B_RIFF_MAX_OBJECTS];
} b_riff_object_stack;

typedef enum b_riff_parser_state {
	b_riff_parser_state_parse, /* parse RIFF structure */
	b_riff_parser_state_discard, /* discard current object */
	b_riff_parser_state_capture, /* capture current object */
	b_riff_parser_state_resync, /* detected error in the stream, resyncing */
	b_riff_parser_state_format_error /* error in the format */
} b_riff_parser_state;

BDBG_OBJECT_ID(briff_parser_t);
struct briff_parser {
	BDBG_OBJECT(briff_parser_t)
	batom_accum_t acc;
	batom_factory_t factory;
	BLST_S_HEAD(b_riff_object_handlers, briff_parser_handler) object_handlers;
	struct {
		b_riff_parser_state state;
		briff_off_t acc_off; /* stream offset for accumulator */
		briff_parser_handler *handler; /* handler for the current leaf object */
		briff_chunk_id handler_elem; /* current element */
		bool parser_return;
        bool data_discontinuity; /* set to true if some data from the stream has been dropped or lost since the last buffer was fed */
        bmedia_parsing_errors errors;
	} state;
	b_riff_object_stack objects; 
	briff_parser_cfg cfg;
};

#define B_RIFF_ASSERT(x)	  BDBG_ASSERT(x)


#define B_RIFF_LIST 	BMEDIA_FOURCC('L','I','S','T')

/* riff object handlers, keep list sorted by the chunk_id, it allows O/2 search
 * complexity and ease handle of multiple handlers for the same chunk_id code */
void 
briff_parser_install_handler(briff_parser_t riff, briff_parser_handler *handler, briff_chunk_id chunk_id, briff_parser_handler_cb object_handler)
{
	briff_parser_handler *cur, *prev;

	BDBG_OBJECT_ASSERT(riff, briff_parser_t);
	BDBG_MSG_TRACE(("briff_parser_install_handler:> %#lx  %#lx" BMEDIA_FOURCC_FORMAT, (unsigned long)riff, (unsigned long)handler, BMEDIA_FOURCC_ARG(chunk_id)));

	for(prev=NULL, cur = BLST_S_FIRST(&riff->object_handlers); cur; cur=BLST_S_NEXT(cur, link)) {
		if (chunk_id<cur->chunk_id) {
			break;
		}
		prev = cur;
	}
	handler->chunk_id = chunk_id;
	handler->handler = object_handler;
	if (prev) {
		BLST_S_INSERT_AFTER(&riff->object_handlers, prev, handler, link);
	} else {
		BLST_S_INSERT_HEAD(&riff->object_handlers, handler, link);
	}
	BDBG_MSG_TRACE(("briff_parser_install_handler:< %#lx  %#lx" BMEDIA_FOURCC_FORMAT, (unsigned long)riff, (unsigned long)handler, BMEDIA_FOURCC_ARG(chunk_id)));
	return ;
}

void 
briff_parser_remove_handler(briff_parser_t riff, briff_parser_handler *handler)
{
	BDBG_OBJECT_ASSERT(riff, briff_parser_t);
	BDBG_ASSERT(handler);
	BDBG_MSG_TRACE(("briff_parser_remove_handler:> %#lx  %#lx" BMEDIA_FOURCC_FORMAT, (unsigned long)riff, (unsigned long)handler, BMEDIA_FOURCC_ARG(handler->chunk_id)));
	BLST_S_REMOVE(&riff->object_handlers, handler, briff_parser_handler, link);
	BDBG_MSG_TRACE(("briff_parser_remove_handler:< %#lx  %#lx" BMEDIA_FOURCC_FORMAT, (unsigned long)riff, (unsigned long)handler, BMEDIA_FOURCC_ARG(handler->chunk_id)));
	return;
}

static briff_parser_handler *
b_riff_find_handler(briff_parser_t riff, briff_chunk_id chunk_id)
{
	briff_parser_handler *cur;

	for(cur = BLST_S_FIRST(&riff->object_handlers); cur; cur=BLST_S_NEXT(cur, link)) {
		if (cur->chunk_id == chunk_id) {
			return cur;
		}
		if(chunk_id < cur->chunk_id) {
			return NULL;
		}
	}
	return NULL;
}

static void
b_riff_call_handlers(briff_parser_t riff, briff_parser_handler *handler, briff_chunk_id chunk_id, batom_cursor *cursor, size_t len, unsigned object_size, unsigned object_offset)
{
	briff_parser_handler *cur;
	briff_parser_action action;

	BSTD_UNUSED(riff);

	for(cur = handler; cur; cur=BLST_S_NEXT(cur, link)) {
		if(cur->chunk_id != handler->chunk_id) {
			break;
		}
		BDBG_MSG(("user_object: " BMEDIA_FOURCC_FORMAT "[" BMEDIA_FOURCC_FORMAT "] %u -> %p %u:%u:%u", BMEDIA_FOURCC_ARG(handler->chunk_id), BMEDIA_FOURCC_ARG(chunk_id), (unsigned)len, (void *)handler, object_size, object_offset, (unsigned)(object_offset+len)));
		if (cursor) {
			batom_cursor temp_cursor;
			size_t skip_len;
			batom_t atom;

			BATOM_CLONE(&temp_cursor, cursor);
			skip_len = batom_cursor_skip(&temp_cursor, len);
			BDBG_ASSERT(skip_len==len);
			atom = batom_accum_extract(riff->acc, cursor, &temp_cursor, NULL, NULL);
			BDBG_ASSERT(atom);
			action = cur->handler(cur, chunk_id, object_size, object_offset, atom);
		} else {
			action = cur->handler(cur, chunk_id, 0, 0, NULL);
		}
		riff->state.parser_return |= action==briff_parser_action_return;
	}
	return ;
}

/* add new entry into the stack, size if the object size (as in file), and offset number of bytes already consumed */
static bool
b_riff_object_push(briff_parser_t riff, briff_chunk_id type, briff_atom size, unsigned offset, briff_parser_handler *handler)
{
	b_riff_object *obj = &riff->objects.stack[riff->objects.size];
	briff_parser_action action;

	if(riff->objects.size >= B_RIFF_MAX_OBJECTS) {
		return false;
	}
	BDBG_ASSERT(offset<=size);
	obj->size = size;
	obj->left = size-offset;
	obj->handler = handler;
	obj->chunk_id = type;
	obj->offset = offset;
	riff->objects.size++;
	b_riff_call_handlers(riff, handler, BRIFF_CHUNK_ID_BEGIN, NULL, 0, size-offset, 0);
	action = riff->cfg.object_begin(riff->cfg.user_cntx, type, size-offset, riff->state.acc_off+(offset+sizeof(briff_chunk_id)+sizeof(briff_atom)));
	riff->state.parser_return |= action==briff_parser_action_return;
	BDBG_MSG_TRACE(("push[%u]: left %u size %u", riff->objects.size-1, obj->left, obj->size));
	return true;
}

static void
b_riff_object_pop(briff_parser_t riff, size_t chunk_size)
{
	BDBG_ASSERT(riff->objects.size > 0 );
	BDBG_ASSERT(riff->objects.size<=B_RIFF_MAX_OBJECTS);
	while(riff->objects.size>0) {
		const b_riff_object *prev;
		briff_parser_action action;

		riff->objects.size--;
		prev =  &riff->objects.stack[riff->objects.size];
	    BDBG_MSG_TRACE(("end elem "  BMEDIA_FOURCC_FORMAT,  BMEDIA_FOURCC_ARG(prev->chunk_id)));
		b_riff_call_handlers(riff, prev->handler, BRIFF_CHUNK_ID_END, NULL, 0, 0, 0);
		action = riff->cfg.object_end(riff->cfg.user_cntx, prev->chunk_id, riff->state.acc_off + chunk_size);
		riff->state.parser_return |= action==briff_parser_action_return;
		if (riff->objects.size > 0) {
			b_riff_object *obj = &riff->objects.stack[riff->objects.size-1];
			BDBG_MSG_TRACE(("pop[%u]: left %u prev size %u", riff->objects.size-1, obj->left, prev->size));
			B_RIFF_ASSERT(obj->left >=  prev->size);
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

static b_riff_object *
b_riff_object_top(briff_parser_t riff)
{
	b_riff_object *obj = &riff->objects.stack[riff->objects.size-1];
	if(riff->objects.size>0) {
		return obj;
	}
	return NULL;
}

static b_riff_object *
b_riff_object_parent(briff_parser_t riff)
{
	if(riff->objects.size>1) {
		b_riff_object *obj = &riff->objects.stack[riff->objects.size-2];
		return obj;
	}
	return NULL;
}

static unsigned
b_riff_object_consume(briff_parser_t riff, unsigned bytes)
{
	b_riff_object *obj = &riff->objects.stack[riff->objects.size-1];
	BDBG_ASSERT(riff->objects.size>0);
	BDBG_ASSERT(riff->objects.size<=B_RIFF_MAX_OBJECTS);
	BDBG_MSG_TRACE(("consume[%u] left %u bytes %u", riff->objects.size-1, obj->left, bytes));
    obj->offset += bytes;
	if(obj->left>=bytes) {
	  obj->left -= bytes;
    } else {
	  obj->left = 0;
    }
	return obj->left;
}

#define briff_read_atom(riff, cursor) (riff)->cfg.le_endian? batom_cursor_uint32_le(cursor):batom_cursor_uint32_be(cursor)
#define briff_read_chunk_id(riff, cursor) batom_cursor_uint32_le(cursor)

static bool
b_riff_parse_elem(briff_parser_t riff, batom_cursor *cursor)
{
	briff_chunk_id elem;
	briff_atom size;
	unsigned offset=0;
	batom_checkpoint chk;
	bool list=false;
	briff_parser_handler *handler;
	const b_riff_object *parent;

	BATOM_SAVE(cursor, &chk);

	elem = briff_read_chunk_id(riff, cursor);
	size = briff_read_atom(riff, cursor);

	if (!BATOM_IS_EOF(cursor)) { 
		if(elem==riff->cfg.list_id || elem==riff->cfg.top_id) {
			/* list is a special case, read list type */
			/* BDBG_MSG_TRACE(("found list")); */
			list = true;
			elem = briff_read_chunk_id(riff, cursor);
			offset = sizeof(elem);
		}
	}
	if (BATOM_IS_EOF(cursor)) { 
		goto rollback;
	}
	/* account for the consumed bytes */
	parent = b_riff_object_top(riff);
	BDBG_ASSERT(parent); /* parent shall be valid here */
	b_riff_object_consume(riff, sizeof(elem) + sizeof(size));
	BDBG_MSG_TRACE(("found elem "  BMEDIA_FOURCC_FORMAT " size %u",  BMEDIA_FOURCC_ARG(elem), (unsigned)size));
    if(size==0) {
        if(elem==BMEDIA_FOURCC('d','a','t','a')) {
            if(parent) {
                size = parent->left;
            } else {
                size = 0xFFFFFFF0u;
            }
        }
        BDBG_WRN(("b_riff_parse_elem:%p elem "  BMEDIA_FOURCC_FORMAT " substituting size:0 with %#x", (void *)riff, BMEDIA_FOURCC_ARG(elem), size));
    }

	handler=b_riff_find_handler(riff, elem);
	if(parent && parent->left<size) {
		BDBG_WRN(("b_riff_parse_elem: %#lx invalid object size %u:%u", (unsigned long)riff, (unsigned)size, parent->left));
        goto sync_error;
	}
	if (b_riff_object_push(riff, elem, size, offset, list?handler:NULL)) {
		/* new element added into the stack */
		if(list) {
			riff->state.state = b_riff_parser_state_parse;
			BDBG_MSG_TRACE(("parse " BMEDIA_FOURCC_FORMAT, BMEDIA_FOURCC_ARG(elem)));
		} else {
			if(handler || parent->handler) {
				/* capture element */
				BDBG_MSG_TRACE(("capture for %#lx " BMEDIA_FOURCC_FORMAT, (unsigned long)handler, BMEDIA_FOURCC_ARG(elem)));
				riff->state.state = b_riff_parser_state_capture;
				riff->state.handler = handler; 
				riff->state.handler_elem = elem; 
			} else {
				/* discard element */
				BDBG_MSG_TRACE(("discard " BMEDIA_FOURCC_FORMAT, BMEDIA_FOURCC_ARG(elem)));
				riff->state.state = b_riff_parser_state_discard;
			}
		}
	} else {
		/* there is no place for the new entry, discard _current_ element */
		BDBG_MSG_TRACE(("discard " BMEDIA_FOURCC_FORMAT, BMEDIA_FOURCC_ARG(elem)));
		riff->state.state = b_riff_parser_state_discard;
	}
	return true;
rollback:
	batom_cursor_rollback(cursor, &chk);
	return false;
sync_error:
    riff->state.state = b_riff_parser_state_resync;
    riff->state.errors.sync_errors++;
    riff->cfg.stream_error(riff->cfg.user_cntx);
    return true;
}

static bool
b_riff_parse_riff(briff_parser_t riff, batom_cursor *cursor)
{
	briff_chunk_id chunk_id;
	briff_atom fileSize;
	briff_chunk_id fileType;
	batom_checkpoint chk;
	BATOM_SAVE(cursor, &chk);

	chunk_id = briff_read_chunk_id(riff, cursor);
	fileSize = briff_read_atom(riff, cursor);
    if(fileSize==0) {
        fileSize = 0xFFFFFFF0u;
        BDBG_WRN(("b_riff_parse_riff:%p substituting size:0 with %#x", (void *)riff, fileSize));
    }
	if (BATOM_IS_EOF(cursor)) { 
		goto rollback;
	}
	BDBG_MSG(("found header "  BMEDIA_FOURCC_FORMAT " size:%u", BMEDIA_FOURCC_ARG(chunk_id), fileSize));

	fileType = briff_read_chunk_id(riff, cursor);

	if (BATOM_IS_EOF(cursor)) { 
		goto rollback;
	}
	BDBG_MSG(("found header "  BMEDIA_FOURCC_FORMAT " type:" BMEDIA_FOURCC_FORMAT " size %u", BMEDIA_FOURCC_ARG(chunk_id), BMEDIA_FOURCC_ARG(fileType), (unsigned)fileSize));
	if (chunk_id==riff->cfg.top_id && fileSize >= sizeof(briff_chunk_id)) {
        if(riff->cfg.file_type(riff->cfg.user_cntx, fileType)==briff_parser_action_return) {
            return false;
        }
		b_riff_object_push(riff, fileType, fileSize, sizeof(fileType), NULL);
	} else {
		/* we parse RIFF RIFF files only */
		BDBG_WRN(("%p: invalid RIFF file", (void *)riff));
		riff->state.state = b_riff_parser_state_format_error;
        riff->state.errors.format_errors ++;
        riff->cfg.stream_error(riff->cfg.user_cntx);
	}
	return true;
rollback:
	batom_cursor_rollback(cursor, &chk);
	return false; 
}


static void 
b_riff_trim(briff_parser_t riff, batom_cursor *cursor)
{
	BDBG_MSG_TRACE(("b_riff_trim>: %p %u:%u", (void *)riff, (unsigned)batom_accum_len(riff->acc), (unsigned)batom_cursor_pos(cursor) ));
	riff->state.acc_off += batom_cursor_pos(cursor);
	batom_accum_trim(riff->acc, cursor);
	BDBG_MSG_TRACE(("b_riff_trim< %p %u", (void *)riff, (unsigned)batom_accum_len(riff->acc)));
	return;
}

void
briff_parser_get_status(briff_parser_t riff, briff_parser_status *status)
{
	const b_riff_object *obj;

	BDBG_OBJECT_ASSERT(riff, briff_parser_t);
	status->acc_length = batom_accum_len(riff->acc);
	status->offset = riff->state.acc_off  + status->acc_length;
	status->obj_length = 0;
    status->data_discontinuity = riff->state.data_discontinuity;
    status->errors = riff->state.errors;
	obj = b_riff_object_top(riff);
	if(obj) {
		status->obj_length = obj->left;
	} else {
		status->obj_length = 0;
	}

	switch(riff->state.state) {
	case b_riff_parser_state_parse:
		status->state = "parse";
		break;
	case b_riff_parser_state_discard:
		status->state = "discard";
		break;
	case b_riff_parser_state_capture:
		status->state  = "capture";
		break;
	case b_riff_parser_state_format_error:
		status->state = "format_error";
		break;
	case b_riff_parser_state_resync:
		status->state = "resync";
		break;
	}
	return;
}


void
briff_parser_reset(briff_parser_t riff)
{
	BDBG_MSG_TRACE(("briff_reset>: %#lx", (unsigned long)riff));
	BDBG_OBJECT_ASSERT(riff, briff_parser_t);
	batom_accum_clear(riff->acc);
	BKNI_Memset(&riff->state, 0, sizeof(riff->state));
    BMEDIA_PARSING_ERRORS_INIT(&riff->state.errors);
	riff->state.acc_off = 0;
    riff->state.data_discontinuity = false;
	riff->objects.size = 0;
	BDBG_MSG_TRACE(("briff_reset<: %#lx", (unsigned long)riff));
	return;
}

int 
briff_parser_seek(briff_parser_t riff, briff_off_t off)
{
	briff_off_t step;
	b_riff_object *obj;
    briff_off_t acc_off; 
	size_t acc_len;

	BDBG_OBJECT_ASSERT(riff, briff_parser_t);
	BDBG_ASSERT(off>=0);

    acc_len = batom_accum_len(riff->acc);
    acc_off = riff->state.acc_off + (int)batom_accum_len(riff->acc);
    if(off == acc_off) {
        BDBG_MSG(("briff_parser_seek:%p " BDBG_UINT64_FMT " no-op", (void *)riff, BDBG_UINT64_ARG(off)));
        return 0;
    }
	batom_accum_clear(riff->acc);
	if(b_riff_object_top(riff) && acc_len>0) {
		b_riff_object_consume(riff, acc_len);
	}
    riff->state.data_discontinuity = true;
	riff->state.acc_off = acc_off;
	step = off - acc_off;
    BDBG_MSG(("briff_parser_seek:%p " BDBG_UINT64_FMT "(" B_OFFT_FMT ")", (void *)riff, BDBG_UINT64_ARG(off), B_OFFT_ARG(step)));
	if(step>0) {
        bool consumed=false;

		/* jump forward, skip all completed objects */
		while(NULL != (obj = b_riff_object_top(riff))) {
			size_t size;
			size_t left;

			BDBG_MSG_TRACE(("briff_parser_seek [%u] " BMEDIA_FOURCC_FORMAT " left %u size %u bytes %u", riff->objects.size-1, BMEDIA_FOURCC_ARG(obj->chunk_id), obj->left, obj->size, (unsigned)step));
			size = obj->left;
			if (size > (uint64_t)step) {
				size = step;
				step = 0;
				b_riff_object_consume(riff, size);
				break;
			}
			step -= size;
			left = b_riff_object_consume(riff, size);
			BDBG_ASSERT(left==0);
			b_riff_object_pop(riff, size);
            consumed = true;
		}
		BDBG_ASSERT(step>=0);

	    if(riff->state.state==b_riff_parser_state_resync) {
            riff->state.errors.resync_events++;
            riff->cfg.stream_error(riff->cfg.user_cntx);
        }
        if(!consumed && (riff->state.state==b_riff_parser_state_discard || riff->state.state==b_riff_parser_state_capture)) {
            BDBG_MSG_TRACE(("briff_parser_seek: advanced object to %u", b_riff_object_top(riff)?b_riff_object_top(riff)->size - b_riff_object_top(riff)->left:0));
        } else {
		    riff->state.state = b_riff_parser_state_parse;
        }
	} else if(step<0) { /* if we jump back we shall jump to top level object */
		step = -step;
		/* jump backward, skip all empty objects */
		while(NULL != (obj = b_riff_object_top(riff))) {
			size_t size;
			BDBG_MSG_TRACE(("briff_parser_seek [%u] " BMEDIA_FOURCC_FORMAT " left %u size %u bytes %u", riff->objects.size-1, BMEDIA_FOURCC_ARG(obj->chunk_id), obj->left, obj->size, (unsigned)step));
			BDBG_ASSERT(obj->size >= obj->left);
			size = obj->size - obj->left;
			if (size > (uint64_t)step) {
				size = step;
				obj->left += size;
				step = 0;
				break;
			}
			step -= size;
			BDBG_ASSERT(riff->objects.size > 0 );
			BDBG_ASSERT(riff->objects.size<=B_RIFF_MAX_OBJECTS);
			while(riff->objects.size>0) {
				const b_riff_object *prev;
				riff->objects.size--;
				prev =  &riff->objects.stack[riff->objects.size];
				riff->cfg.object_end(riff->cfg.user_cntx,  prev->chunk_id, 0 );	
				if (riff->objects.size > 0) {
					obj = &riff->objects.stack[riff->objects.size-1];
					BDBG_MSG_TRACE(("briff_parser_seek pop [%u]: left " BMEDIA_FOURCC_FORMAT " %u size %u, prev " BMEDIA_FOURCC_FORMAT " %u size %u", riff->objects.size-1, BMEDIA_FOURCC_ARG(obj->chunk_id), obj->left, obj->size, BMEDIA_FOURCC_ARG(prev->chunk_id), prev->left, prev->size));
					if(obj->left == 0 ) {
						continue;
					}
				}
				break;
			}
		}
	    if(riff->state.state==b_riff_parser_state_resync) {
            riff->state.errors.resync_events++;
            riff->cfg.stream_error(riff->cfg.user_cntx);
        }
		riff->state.state = b_riff_parser_state_parse;
	}
	riff->state.acc_off = off;
	return 0;
}


briff_parser_t 
briff_parser_create(batom_factory_t factory, const briff_parser_cfg *cfg)
{
	briff_parser_t riff;

	BDBG_MSG_TRACE(("briff_create>: %#lx", (unsigned long)cfg));

	BDBG_CASSERT(sizeof(briff_atom)==4);
	BDBG_CASSERT(sizeof(briff_chunk_id)==4);

	BDBG_ASSERT(cfg);
	BDBG_ASSERT(factory);

	riff = BKNI_Malloc(sizeof(*riff));
	if (!riff) { 
		BDBG_ERR(("briff_create: out of memory %u", (unsigned)sizeof(*riff)));
		goto err_alloc_riff;
	}
	BDBG_OBJECT_INIT(riff, briff_parser_t);

	riff->cfg = *cfg;
	riff->factory = factory;
	riff->acc = batom_accum_create(factory);
	if(!riff->acc) {
		goto err_alloc_accum;
	}
	BLST_S_INIT(&riff->object_handlers);
	briff_parser_reset(riff);
	BDBG_MSG_TRACE(("briff_create>: %#lx %#lx", (unsigned long)cfg, (unsigned long)riff));
	return riff;

err_alloc_accum:
	BKNI_Free(riff);
err_alloc_riff:
	BDBG_MSG_TRACE(("briff_create>: %#lx %#lx", (unsigned long)cfg, (unsigned long)NULL));
	return NULL;
}

static briff_parser_action 
b_riff_empty_object_begin_cb(void *cntx, briff_chunk_id chunk_id, unsigned size, briff_off_t offset)
{
	BSTD_UNUSED(cntx);
	BSTD_UNUSED(chunk_id);
	BSTD_UNUSED(size);
	BSTD_UNUSED(offset);
	return briff_parser_action_none;
}

static briff_parser_action 
b_riff_empty_object_end_cb(void *cntx, briff_chunk_id chunk_id, briff_off_t offset)
{
	BSTD_UNUSED(cntx);
	BSTD_UNUSED(chunk_id);
	BSTD_UNUSED(offset);
	return briff_parser_action_none;
}

static void
b_riff_empty_stream_error_cb(void *cntx)
{
	BSTD_UNUSED(cntx);
	return ;
}

static briff_parser_action 
b_riff_empty_file_type(void *context, briff_chunk_id file_type)
{
    BSTD_UNUSED(file_type);
    BSTD_UNUSED(context);
    return briff_parser_action_return;
}

void 
briff_parser_default_cfg(briff_parser_cfg *cfg)
{
	BDBG_ASSERT(cfg);
	cfg->user_cntx = NULL;
	cfg->object_begin = b_riff_empty_object_begin_cb;
	cfg->object_end = b_riff_empty_object_end_cb;
    cfg->stream_error = b_riff_empty_stream_error_cb;
    cfg->file_type = b_riff_empty_file_type;
    cfg->le_endian = true;
    cfg->top_id = B_RIFF_RIFF;
    cfg->list_id = B_RIFF_LIST;
	return;
}

void 
briff_parser_destroy(briff_parser_t riff)
{
	briff_parser_handler *handler;
	BDBG_MSG_TRACE(("briff_destroy>: %#lx", (unsigned long)riff));
	BDBG_OBJECT_ASSERT(riff, briff_parser_t);
	for(handler=BLST_S_FIRST(&riff->object_handlers); handler; handler=BLST_S_NEXT(handler, link)) {
		BDBG_WRN(("briff_destroy: %#lx lost object_handler %#lx ...", (unsigned long)riff, (unsigned long)handler));
		BDBG_WRN(("briff_destroy: %#lx lost object_handler %#lx " BMEDIA_FOURCC_FORMAT, (unsigned long)riff, (unsigned long)handler, BMEDIA_FOURCC_ARG(handler->chunk_id)));
	}
	briff_parser_reset(riff);
	batom_accum_destroy(riff->acc);
	BDBG_OBJECT_DESTROY(riff, briff_parser_t);
	BDBG_MSG_TRACE(("briff_destroy<: %#lx", (unsigned long)riff));
	BKNI_Free(riff);
	return;
}


size_t
briff_parser_feed(briff_parser_t riff, batom_pipe_t pipe)
{
	size_t len;


	BDBG_OBJECT_ASSERT(riff, briff_parser_t);
	BDBG_MSG_TRACE(("briff_parser_feed>:%p %p %u", (void *)riff, (void *)pipe, (unsigned)(riff->state.acc_off + batom_accum_len(riff->acc))));
	len=0;
	for(;;) {
		b_riff_object *obj;
		batom_cursor cursor;
		bool want_continue;
		size_t size;
		batom_t atom;

		if(!riff->state.parser_return) {
			atom=batom_pipe_pop(pipe);
			if(!atom) {
				break;
			}
			batom_accum_add_atom(riff->acc, atom);
			len += batom_len(atom);
			batom_release(atom);
		}
		riff->state.parser_return = false;
		do {
			size_t acc_len = batom_accum_len(riff->acc);
			batom_cursor_from_accum(&cursor, riff->acc);
			want_continue = false;
			riff->state.data_discontinuity = false;
			switch(riff->state.state) {
			case b_riff_parser_state_parse:

				/* header shall be 2 bytes aligned */
				size = riff->state.acc_off%sizeof(uint16_t);
				if (size && acc_len>=size) {
					BDBG_MSG_TRACE(("aligning stream by %u bytes (%u)", (unsigned)size, (unsigned)riff->state.acc_off));
					BDBG_ASSERT(size==1);
					want_continue = true;
					if (riff->objects.size>0) {
						if(b_riff_object_consume(riff, size) == 0) {
							b_riff_object_pop(riff, size);
						}
					}
					batom_cursor_skip(&cursor, size);
					b_riff_trim(riff, &cursor);
					if(riff->state.parser_return) {
						break;
					}
					batom_cursor_from_accum(&cursor, riff->acc);
				}

				if(riff->objects.size==0) { /* parse the top level object */
					want_continue = b_riff_parse_riff(riff, &cursor);
				} else {
					want_continue = b_riff_parse_elem(riff, &cursor);
				}
				if (want_continue) {
					b_riff_trim(riff, &cursor);
				}
				break;

			case b_riff_parser_state_resync:
				if (riff->objects.size==0) {
					BDBG_WRN(("briff_parser_feed: %#lx reached bottom of the file", (unsigned long)riff));
					riff->state.state = b_riff_parser_state_parse;
					riff->state.errors.resync_events++;
					riff->cfg.stream_error(riff->cfg.user_cntx);
					continue;
				}
				obj = b_riff_object_top(riff);
				BDBG_ASSERT(obj);
				BDBG_MSG_TRACE(("briff_parser_feed: resync %u %u", obj->left, (unsigned)acc_len));
				/* header shall be 2 bytes aligned */
				size = riff->state.acc_off%sizeof(uint16_t);
				if (size && acc_len>=size) {
					BDBG_MSG_TRACE(("aligning stream by %u bytes (%u)", (unsigned)size, (unsigned)riff->state.acc_off));
					BDBG_ASSERT(size==1);
					want_continue = true;
					BDBG_ASSERT(riff->objects.size>0);
					if(b_riff_object_consume(riff, size) == 0) {
						b_riff_object_pop(riff, size);
					}
					batom_cursor_skip(&cursor, size);
					b_riff_trim(riff, &cursor);
					if(riff->state.parser_return) {
						break;
					}
					batom_cursor_from_accum(&cursor, riff->acc);
				}
				want_continue = true;
				{
					briff_chunk_id elem = briff_read_chunk_id(riff, &cursor); 
					/* resync with a data by locating element that downstream module wants */
					if (BATOM_IS_EOF(&cursor)) { 
						want_continue = false;
						break;
					}
					batom_cursor_from_accum(&cursor, riff->acc);
					if(b_riff_find_handler(riff, elem)) {
						/* found valid element and restart parser */
						BDBG_WRN(("briff_parser_feed: %#lx resynced with " BMEDIA_FOURCC_FORMAT " at %lu", (unsigned long)riff, BMEDIA_FOURCC_ARG(elem), (unsigned long)riff->state.acc_off));  
						riff->state.state = b_riff_parser_state_parse;
						riff->state.errors.resync_events++;
						riff->cfg.stream_error(riff->cfg.user_cntx);
					} else {
						size = sizeof(uint16_t);
						BDBG_ASSERT(riff->objects.size>0);
						if(size>riff->objects.stack[riff->objects.size-1].left) {
							size = 1;
						}
						if(b_riff_object_consume(riff, size) == 0) {
							b_riff_object_pop(riff, size);
						}
						batom_cursor_skip(&cursor, size);
						b_riff_trim(riff, &cursor);
						if(riff->state.parser_return) {
							want_continue = false;
						}
					}
				}
				break;

			case b_riff_parser_state_discard:
				obj = b_riff_object_top(riff);
				BDBG_ASSERT(obj);
				BDBG_MSG_TRACE(("briff_parser_feed: discard %u %u", (unsigned)obj->left, (unsigned)acc_len));
				size = B_RIFF_MIN(acc_len, obj->left);
				batom_cursor_skip(&cursor, size);
				if(b_riff_object_consume(riff, size) == 0) {
					b_riff_object_pop(riff, size);
					want_continue = !riff->state.parser_return;
					riff->state.state = b_riff_parser_state_parse;
				}
				b_riff_trim(riff, &cursor);
				break;
			case b_riff_parser_state_capture:
				obj = b_riff_object_top(riff);
				BDBG_ASSERT(obj);
				BDBG_MSG_TRACE(("briff_parser_feed: capture %u %u", (unsigned)obj->left, (unsigned)acc_len));
				if (acc_len>0 || acc_len>=obj->left) {
					size_t obj_left = B_RIFF_MIN(acc_len,obj->left);
					b_riff_object *parent;
					/* entire object was captured */
					/* BDBG_ASSERT(riff->state.handler); */
					BDBG_MSG_TRACE(("briff_parser_feed: captured " BMEDIA_FOURCC_FORMAT " %u bytes", BMEDIA_FOURCC_ARG(obj->chunk_id), (unsigned)obj_left));

					/* call handler for this object */
					b_riff_call_handlers(riff, riff->state.handler, obj->chunk_id, &cursor, obj_left, obj->size, obj->offset);

					parent = b_riff_object_parent(riff);
					if(parent && parent->handler) { /* if parent list has a handler */
						b_riff_call_handlers(riff, parent->handler, obj->chunk_id, &cursor, obj_left, obj->size, obj->offset);
					}
					batom_cursor_skip(&cursor, obj_left);
					b_riff_trim(riff, &cursor);
					if(b_riff_object_consume(riff, obj_left) == 0) {
					    riff->state.handler = NULL; 
						b_riff_object_pop(riff, obj_left);
					    riff->state.state = b_riff_parser_state_parse;
					}
					want_continue = !riff->state.parser_return;
					if(!want_continue) {
						goto stopped_parsing;
					}
				} /* else stop parsing and wait for more data */
				break;
			case b_riff_parser_state_format_error:
				/* just drop all data */
				batom_cursor_skip(&cursor, acc_len);
				b_riff_trim(riff, &cursor);
				break;
			}
		} while(want_continue);
	}
stopped_parsing:
	return len;
}


