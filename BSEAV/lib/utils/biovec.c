/***************************************************************************
 *     Copyright (c) 2006-2007, Broadcom Corporation
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
 * ASF parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#include "bstd.h"
#include "biovec.h"
#include "bkni.h"

BDBG_MODULE(bio_vec);

#define BDBG_MSG_FLOW(x)   
#define BDBG_MSG_TRACE(x) 

void 
bio_array_init(bio_array *array)
{
	array->size = B_IO_ARRAY_INLINE;
	array->count = 0;
	array->vec = array->vec_;
	array->owner = array->owner_;
	array->length = 0;
	return;
}


void 
bio_array_reset(bio_array *array)
{
	array->count = 0;
	array->length = 0;
	return;
}

void 
bio_array_shutdown(bio_array *array)
{
	if(array->size>B_IO_ARRAY_INLINE) {
		/* free mmemory if not embedded */
		BKNI_Free(array->vec);
	}
	return;
}


size_t
bio_array_add_vec(bio_array *array, const bio_vec *v)
{
	BDBG_ASSERT(array);
	BDBG_ASSERT(array->count <= array->size);
	if (v->len==0) {
		BDBG_MSG_TRACE(("%#lx adding zero length vector %#lx:%#lx", (unsigned long)array, (unsigned long)v, (unsigned long)v->base));
		return array->length;
	}
	BDBG_MSG_TRACE(("array_add : %#lx %u %#lx", (unsigned long)array, v->len, (unsigned long)v->base));
	if(array->count >= array->size) {
		unsigned i;
		unsigned size = array->size + 3;
		bio_vec *vec;
		bio_vec const **owner;

		BDBG_MSG_TRACE(("growing array %#lx %u->%u", (unsigned long)array, array->size, size));
		vec =  BKNI_Malloc((sizeof(*vec)+sizeof(void *))*size);
		BDBG_ASSERT(vec);
		if (!vec) {
			return BIO_EOF;
		}
		owner = (bio_vec const **)(vec+size);
		for(i=0;i<array->size;i++) {
			vec[i] = array->vec[i];
			owner[i] = array->owner[i];
		}
		if(array->size>B_IO_ARRAY_INLINE) {
			/* free mmemory if not embedded */
			BKNI_Free(array->vec);
		}
		array->vec = vec;
		array->owner = owner;
		array->size = size;
	}
	BDBG_ASSERT(array->size > array->count);
	array->vec[array->count] = *v;
	array->owner[array->count] = v;
	array->count++;
	array->length += v->len;
	return array->length;
}


void 
bio_cursor_init(bio_cursor *cursor, const bio_array *array)
{
	BDBG_ASSERT(array);
	BDBG_ASSERT(cursor);
	BDBG_MSG_TRACE(("cursor_init: %#lx %#lx %u %u", (unsigned long)cursor, (unsigned long)array, array->count, array->length));
	cursor->array = array;
	cursor->pos = 0;
	/* empty array, trigger refill on first 'next' */
	cursor->left = 0;
	cursor->cursor = 0;
}

size_t 
bio_array_trim(bio_array *array, bio_cursor *cursor, void (*iov_free)(void *cnxt, const bio_vec *v), void *cnxt)
{
	unsigned i;
	unsigned trim_pos;

	BDBG_ASSERT(cursor->array == array);
	BDBG_ASSERT(array->size >= array->count);

	BDBG_ASSERT(cursor->pos <= cursor->array->count);
	BDBG_MSG_TRACE(("array_trim+: %#lx %u %u  cursor: %#lx %u %u", (unsigned long)array, array->count, array->length, (unsigned long)cursor, cursor->pos, cursor->left));
	trim_pos = cursor->pos;
	if (cursor->left > 0) {
		trim_pos--;
	}
	if(trim_pos>0) {
		/* remove dead entries */
		for(i=0;i<trim_pos;i++) {
			iov_free(cnxt, array->owner[i]);
			array->length -= array->vec[i].len;
		}
		for(i=0;i+trim_pos<array->count;i++) {
			array->vec[i] = array->vec[i+trim_pos];
			array->owner[i] = array->owner[i+trim_pos];
		}
		array->count -= trim_pos;
	}
	if (cursor->left > 0) {
		/* adjust for the partial iov */
		array->vec[0].base = (void *)cursor->cursor;
		array->length -= array->vec[0].len - cursor->left;
		array->vec[0].len = cursor->left;
	}

	/* adjust cursor */
	cursor->pos = 0;
	cursor->left = 0;
	cursor->cursor = 0;
	BDBG_MSG_TRACE(("array_trim-: %#lx %u %u", (unsigned long)array, array->count, array->length));

	return 0;
}

size_t 
bio_array_len(const bio_array *array)
{
	return array->length;
}

static size_t
b_io_cursor_refill(bio_cursor *cursor)
{
	BDBG_ASSERT(cursor->left<=0);
	if (cursor->left<0) {
		return 0;
	}
	BDBG_ASSERT(cursor->array);
	BDBG_ASSERT(cursor->pos <= cursor->array->count);
	for(;;) {
		if(cursor->pos >= cursor->array->count) {
			BDBG_MSG_TRACE(("cursor_refill: %#lx %u %u EOF", (unsigned long)cursor, cursor->pos, cursor->array->count));
			/* reached EOF */
			cursor->left = BIO_EOF;
			return 0;
		}
		cursor->left = cursor->array->vec[cursor->pos].len;
		if (cursor->left==0) {
			cursor->pos++;
			continue; /* if empty entry, try fetch new one */
		}
		cursor->cursor = cursor->array->vec[cursor->pos].base;
		BDBG_MSG_TRACE(("cursor_refill: %#lx %u %u %#lx", (unsigned long)cursor, cursor->pos, cursor->left, (unsigned long)cursor->cursor));
		cursor->pos++;
		break;
	}
	BDBG_ASSERT(cursor->left>0);
	return (size_t)cursor->left;
}

int 
bio_cursor_next(bio_cursor *cursor)
{
	int d;

	BDBG_ASSERT(cursor);
	if (cursor->left<=0) {
		if (b_io_cursor_refill(cursor)==0) {
			return BIO_EOF;
		}
	}
	BDBG_ASSERT(cursor->left>0);
	cursor->left--;
	d = *(cursor->cursor++);
	BDBG_MSG_FLOW(("cursor_next: %#lx %d 0x%02x", (unsigned long)cursor, cursor->left, d));
	return d;
}

bool 
bio_cursor_eof(const bio_cursor *cursor)
{
	BDBG_ASSERT(cursor);
	BDBG_ASSERT(cursor->array);
	BDBG_ASSERT(cursor->pos <= cursor->array->count);
	return cursor->left < 0;
}

void
bio_cursor_skip(bio_cursor *cursor, size_t count)
{
	BDBG_MSG_TRACE(("cursor_skip+: %#lx %d %u", (unsigned long)cursor, cursor->left, count));
	BDBG_ASSERT((int)count>=0);
	for(;;) {
		if (cursor->left>=(int)count) {
			cursor->cursor+=count;
			cursor->left-=count;
	 		break;
		} else {
			if (cursor->left>0) {
				count -= cursor->left;
				cursor->left = 0;
			}
			if (b_io_cursor_refill(cursor)==0) {
				break;
			}
		}
	}
	BDBG_MSG_TRACE(("cursor_skip-: %#lx %d %u", (unsigned long)cursor, cursor->left, count));
	return ;
}

#if B_IOVEC_memcpy
#include <string.h>
#endif

size_t 
bio_cursor_copy(bio_cursor *cursor, void *dest, size_t count)
{
	size_t i;
	uint8_t *d = dest;
	const uint8_t *s = cursor->cursor;
	int left = cursor->left;
#if B_IOVEC_memcpy
#else
	register uint8_t s0,s1;
#endif

	BDBG_MSG_TRACE(("cursor_copy+: %#lx %u", (unsigned long)cursor, count));

	BDBG_ASSERT(count>0);
	for(i=count;i>0;) {

		if (i>=8 &&  left>=8) {
		/* we optimize for copying 184 bytes at a time, which leads to 23*8 copies */
#if B_IOVEC_memcpy
			memcpy(d,s,8); 
			/* this will be inlned into 8 pipelined instructions for unaligned load/store */
#else
			s0 = s[0];
			s1 = s[1];
			d[0] = s0;
			d[1] = s1;

			s0 = s[2];
			s1 = s[3];
			d[2] = s0;
			d[3] = s1;

			s0 = s[4];
			s1 = s[5];
			d[4] = s0;
			d[5] = s1;

			s0 = s[6];
			s1 = s[7];
			d[6] = s0;
			d[7] = s1;
#endif

			d+=8;i-=8;
			s+=8;
			left-=8;
		} else if (left>=1) {
			d[0] = s[0];
			d++;i--;
			s++;
			left--;
		} else {
			cursor->left = left;
			cursor->cursor = s;
			left = b_io_cursor_refill(cursor);
			if (left==0) {
				goto done;
			}
			s = cursor->cursor;
			continue;
		}
	}
	cursor->left = left;
	cursor->cursor = s;
done:
	BDBG_MSG_TRACE(("cursor_copy-: %#lx %u", (unsigned long)cursor, count-i));
	return count-i;
}




void 
bio_cursor_save(const bio_cursor *cursor, bio_checkpoint *checkpoint)
{
	BDBG_MSG_TRACE(("cursor_save: %#lx", (unsigned long)cursor));
	checkpoint->left = cursor->left;
	checkpoint->pos = cursor->pos;
}

void 
bio_cursor_rollback(bio_cursor *cursor, const bio_checkpoint *checkpoint)
{
	BDBG_MSG_TRACE(("cursor_rollback: %#lx(%d:%u) %lx(%d:%u)", (unsigned long)cursor, cursor->left, cursor->pos, (unsigned long)checkpoint, checkpoint->left, checkpoint->pos));
	BDBG_ASSERT( cursor->pos >= checkpoint->pos );
	cursor->left = checkpoint->left;
	cursor->pos = checkpoint->pos;
	if (cursor->left<=0) {
		BDBG_MSG_TRACE(("cursor_rollback: %#lx %u %u EOF", (unsigned long)cursor, cursor->pos, cursor->array->count));
		return;
	}
	BDBG_ASSERT(cursor->array);
	BDBG_ASSERT(cursor->pos <= cursor->array->count && cursor->pos>0);
	BDBG_ASSERT(cursor->left <= (int)cursor->array->vec[cursor->pos-1].len);
	cursor->cursor = cursor->array->vec[cursor->pos-1].base;
	cursor->cursor += cursor->array->vec[cursor->pos-1].len - cursor->left;
	BDBG_MSG_TRACE(("cursor_rollback: %#lx(%d:%u:%#lx)", (unsigned long)cursor, cursor->left, cursor->pos, (unsigned long)cursor->cursor));
	return;
}

size_t 
bio_cursor_pos(const bio_cursor *cursor)
{
	size_t i;
	size_t size;

	/* count completed iovec's */
	for(size=0,i=0;i<cursor->pos;i++) {
		size += cursor->array->vec[i].len;
	}
	/* add current iovec */
	if (cursor->left>0) {
		size -= cursor->left;
	}
	BDBG_MSG_TRACE(("cursor_pos: %#lx %u", (unsigned long)cursor, size));
	return size;
}

size_t 
bio_cursor_get(bio_cursor *cursor, size_t size, bio_vec *vec, bio_vec const **owner)
{
	BDBG_ASSERT(cursor->array);
	BDBG_ASSERT((int)size>=0);
	BDBG_MSG_TRACE(("bio_cursor_get+: %#lx %u %d %u", (unsigned long)cursor, cursor->pos, cursor->left, cursor->array->count));
	if (cursor->left<0) {
		BDBG_MSG_TRACE(("bio_cursor_get-: %#lx %u %d %u EOF", (unsigned long)cursor, cursor->pos, cursor->left, cursor->array->count));
		return 0;
	}
	if (cursor->left==0) {
		if (b_io_cursor_refill(cursor)==0) {
			BDBG_MSG_TRACE(("bio_cursor_get-: %#lx %u %d %u EOF(refill)", (unsigned long)cursor, cursor->pos, cursor->left, cursor->array->count));
			return 0;
		}
	}
	BDBG_ASSERT(cursor->pos >= 1);
	if (owner) {
		BDBG_ASSERT(cursor->array->owner);
		*owner = cursor->array->owner[cursor->pos-1];
	}
	vec->base = (void *)cursor->cursor;
	if((int)size >= cursor->left) {
		vec->len = cursor->left;
		cursor->left = 0; /* forces refill */
	} else {
		vec->len = size;
		cursor->left -= size;
		cursor->cursor += size;
	}
	BDBG_MSG_TRACE(("bio_cursor_get-: %#lx %u %d %u -> %#lx:%u", (unsigned long)cursor, cursor->pos, cursor->left, cursor->array->count, (unsigned long)vec->base, vec->len));
	return vec->len;
}

size_t
bio_cursor_reserve(bio_cursor *cursor, size_t count)
{
	bio_checkpoint chk;
	size_t result;
	BDBG_MSG_TRACE(("bio_cursor_reserve+: %#lx(%d,%u) %u", (unsigned long)cursor, cursor->left, cursor->pos, count));

	BIO_SAVE(cursor, &chk);

	bio_cursor_skip(cursor, count);
	if (BIO_IS_EOF(cursor)) {
		int t;

		BDBG_MSG_TRACE(("bio_cursor_reserve=: %#lx(%d,%u) %u EOF", (unsigned long)cursor, cursor->left, cursor->pos, count));
		bio_cursor_rollback(cursor, &chk);
		result=count;
		for(;count>0;count--) {
			BIO_NEXT(t,cursor);
			if (t==BIO_EOF) {
				break;
			}
		}
		result=result-count;
	} else {
		result=count; 
	}

	bio_cursor_rollback(cursor, &chk);

	BDBG_MSG_TRACE(("bio_cursor_reserve-: %#lx(%d,%u) %u", (unsigned long)cursor, cursor->left, cursor->pos, result));
	return result;
}

void 
bio_cursor_clone(bio_cursor *dst, const bio_cursor *src) 
{
	/* do a plain copy */
	*dst = *src;
	return;
}

void
bio_array_from_iov(bio_array *array, const bio_vec *vec, unsigned count)
{
	array->size = 0;
	array->count = count;
	array->vec = (bio_vec *)vec;
	array->length = 0;
	array->owner = NULL;
	return;
}

void 
bio_cursor_from_iov(bio_cursor *cursor, bio_array *array, const bio_vec *vec, unsigned count)
{
	bio_array_from_iov(array, vec, count);
	bio_cursor_init(cursor, array);
	return;
}


size_t
bio_array_add(bio_array *array, const void *base, size_t len)
{
	bio_vec v;
	v.len = len;
	v.base = (void *)base;
	return bio_array_add_vec(array, &v);
}

void 
bio_cursor_from_range(bio_cursor *cursor, bio_array *array, const void *base, size_t len)
{
	bio_array_init(array);
	bio_array_add(array, base, len);
	bio_cursor_init(cursor, array);
	return;
}

static const bio_array b_empty_array = 
{
	0,
	NULL,
	{{NULL,0}},
	0,
	0,
	{NULL},
	NULL
};

void bio_cursor_empty(bio_cursor *cursor)
{
	bio_cursor_init(cursor, &b_empty_array);
}

