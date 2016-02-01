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
 * Linear file cache
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bfile_cache.h"

BDBG_MODULE(bfile_cache);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */


void 
bfile_segment_clear(bfile_segment *segment)
{
	segment->start = -1;
	segment->len = 0;
	return;
}

void
bfile_segment_set(bfile_segment *segment, off_t start, uint64_t len)
{
	segment->start = start;
	segment->len = len;
	return;
}

bool
bfile_segment_test(const bfile_segment *segment)
{
	return segment->len>0 && segment->start>0;
}

BDBG_OBJECT_ID(bfile_cache_t);

struct bfile_cache {
	BDBG_OBJECT(bfile_cache_t)
	size_t cur_pos; /* current position in the buffer */
	size_t atom_size;
	size_t cur_size; /* number of bytes loaded into the buffer */
	size_t cur_off; /* current offset from start of segment */
	bfile_segment segment;
	size_t buf_size;
    bool file_error;
	bfile_io_read_t  fd;
	uint8_t buffer[1]; /* variable size array for the cache */
};

bfile_cache_t 
bfile_cache_create(bfile_io_read_t fd, const bfile_segment *segment, size_t buf_size, size_t atom_size)
{
    bfile_cache_t cache;
    BDBG_INT64_DEC_BUF(buf);

	BDBG_ASSERT(fd);
	BDBG_ASSERT(segment);
	if(atom_size>buf_size) {
		BDBG_ERR(("bfile_cache_create: buf_size%u shall be larger then atom_size:%u", (unsigned)buf_size, (unsigned)atom_size));
		return NULL;
	}
	if(!bfile_segment_test(segment)) {
		BDBG_ERR(("bfile_cache_create: segment doesn't point to valid data"));
		return NULL;
	}
	cache = BKNI_Malloc(sizeof(*cache)+buf_size);
	if(!cache) {
		BDBG_ERR(("bfile_cache_create: can't allocate %u bytes", (unsigned)(sizeof(*cache)+buf_size)));
		return NULL;
	}
	BDBG_OBJECT_INIT(cache, bfile_cache_t);
	cache->segment = *segment;
	cache->fd = fd;
	cache->atom_size = atom_size;
	cache->buf_size = buf_size;
	cache->cur_size = 0;
	cache->cur_pos = 0;
	cache->cur_off = 0;
	cache->cur_size = 0;
    cache->file_error = false;
	BDBG_MSG(("bfile_cache_create: %p buf_size:%u atom_size:%u size:%u start:" BDBG_INT64_DEC_FMT, (void *)cache, (unsigned)cache->buf_size, (unsigned)cache->atom_size, (unsigned)cache->segment.len, BDBG_INT64_DEC_ARG(buf, cache->segment.start)));
	return cache;
}

void
bfile_cache_destroy( bfile_cache_t  cache)
{
	BDBG_OBJECT_ASSERT(cache, bfile_cache_t);
	BDBG_OBJECT_DESTROY(cache, bfile_cache_t);
	BKNI_Free(cache);
	return;
}

void
bfile_cache_clear( bfile_cache_t  cache)
{
	BDBG_OBJECT_ASSERT(cache, bfile_cache_t);

	cache->cur_size = 0;
	cache->cur_pos = 0;
	cache->cur_off = 0;
	cache->cur_size = 0;
    cache->file_error = false;
	return;
}

void 
bfile_segment_reset( bfile_cache_t  cache, bfile_segment *segment)
{
	BDBG_OBJECT_ASSERT(cache, bfile_cache_t);
	
	cache->segment = *segment;
	return;
}
	

int
bfile_cache_seek(bfile_cache_t cache, size_t off)
{
	BDBG_OBJECT_ASSERT(cache, bfile_cache_t);
	if(off>cache->segment.len) {
		BDBG_WRN(("seek out of bounds %u (%u)", (unsigned)off, (unsigned)cache->segment.len));
		return -1;
	}
	if(off>= cache->cur_off && (off + cache->atom_size) < (cache->cur_off + cache->cur_size)) {
		/* location is already in cache */
		cache->cur_pos = off - cache->cur_off;
	} else {
		cache->cur_size = 0; /* forces reload on next read */
		if(off > cache->cur_off) { /* seek forward, preload from new offset */
			cache->cur_off = off;
			cache->cur_pos = 0;
		} else { /* seek back, preload from tail */
			if(off >= (cache->buf_size-cache->atom_size)) {
				cache->cur_off = off - (cache->buf_size-cache->atom_size);
				cache->cur_pos = (cache->buf_size-cache->atom_size);
			} else {
				cache->cur_off = 0;
				cache->cur_pos = off;
			}
		}
	}
    cache->file_error = false;
	BDBG_ASSERT(cache->cur_off + cache->cur_pos == off); /* verify our calculcations */
	return 0;
}

const uint8_t *
bfile_cache_next(bfile_cache_t cache)
{
	size_t new_off;
	size_t to_read;
	ssize_t read_result;
	off_t seek_result;
	off_t seek_dest;
	size_t atom_size;
    BDBG_INT64_DEC_BUF(buf1);
    BDBG_INT64_DEC_BUF(buf2);

	BDBG_OBJECT_ASSERT(cache, bfile_cache_t);
	atom_size = cache->atom_size;
	for(;;) {
		size_t cur_size = cache->cur_size;

		new_off = cache->cur_pos + atom_size;
		if(new_off <= cur_size) {
			const uint8_t *buf=&cache->buffer[cache->cur_pos];
			cache->cur_pos = new_off;
			return buf;
		}
		new_off = cache->cur_off;
		if(cur_size > 0) { /* normal read */
			new_off += cache->cur_pos; /* advance to new offset */
		} 
		to_read = cache->buf_size;
		if(new_off + to_read >= cache->segment.len) {
			if(new_off >= cache->segment.len) {
				BDBG_MSG(("bfile_cache_next:%p EOF reached %lu(%lu)", (void *)cache, (unsigned long)new_off, (unsigned long)cache->segment.len));
				return NULL;
			}
			to_read = cache->segment.len - new_off;
		}
		seek_dest = cache->segment.start + new_off;
		BDBG_MSG(("bfile_cache_next:%p read %u bytes at " BDBG_INT64_DEC_FMT "", (void *)cache, (unsigned)to_read, BDBG_INT64_DEC_ARG(buf1,seek_dest)));
		seek_result = cache->fd->seek(cache->fd, seek_dest, SEEK_SET);
		if(seek_result!=seek_dest) {
            cache->file_error = true;
            BDBG_WRN(("bfile_cache_next:%p seek error " BDBG_INT64_DEC_FMT "("  BDBG_INT64_DEC_FMT ")", (void *)cache, BDBG_INT64_DEC_ARG(buf1,seek_result), BDBG_INT64_DEC_ARG(buf2,seek_dest)));
            return NULL;
		}
		read_result =  cache->fd->read(cache->fd, cache->buffer, to_read);
		if(read_result<=0 || read_result<(ssize_t)atom_size) {
            cache->file_error = true;
            BDBG_WRN(("bfile_cache_next:%p read error %ld (" BDBG_INT64_DEC_FMT ":" BDBG_INT64_DEC_FMT ")", (void *)cache, (long)read_result, BDBG_INT64_DEC_ARG(buf1, seek_dest), BDBG_INT64_DEC_ARG(buf2,(cache->segment.start+cache->segment.len))));
            return NULL;
		}
		if(cur_size > 0) {
			cache->cur_pos = 0; /* clear position */
		} else { /* fill read */
			if(read_result < (ssize_t)(cache->cur_pos+atom_size)) {
                if((size_t)read_result<to_read) {
                    BDBG_WRN(("bfile_cache_next: %p haven't read enough data %ld:%u:%u(%u:%u)", (void *)cache, (long)read_result, (unsigned)to_read, (unsigned)(cache->cur_pos+atom_size), (unsigned)cache->cur_pos, (unsigned)atom_size));
                } else {
                    BDBG_WRN(("bfile_cache_next: %p not enough data %ld:%u:%u(%u:%u)", (void *)cache, (long)read_result, (unsigned)to_read, (unsigned)(cache->cur_pos+atom_size), (unsigned)cache->cur_pos, (unsigned)atom_size));
                }
                return NULL;
			}
		} 
		cache->cur_size = read_result;
		cache->cur_off = new_off;
        cache->file_error = false;
		BDBG_ASSERT(cache->cur_pos + atom_size <=  cache->cur_size); /* verify that we did our math right */
	}
}

int
bfile_cached_segment_init(bfile_cached_segment *segment, bfile_buffer_t buffer, batom_factory_t factory, size_t min_read_size)
{
	BDBG_ASSERT(segment);
	BDBG_ASSERT(buffer);
	BDBG_ASSERT(factory);
	BDBG_ASSERT(min_read_size>0);
	segment->accum = batom_accum_create(factory);
	if(!segment->accum) {
		return -1;
	}
	bfile_segment_clear(&segment->segment);
	batom_cursor_from_accum(&segment->cursor, segment->accum);
	segment->accum_offset = 0;
	segment->buffer = buffer;
	segment->min_read_size = min_read_size;
	segment->async.cntx = NULL;
	segment->async.read_complete = NULL;
	segment->async.accum_size = 0;
	return 0;
}

bool 
bfile_cache_is_file_error(bfile_cache_t cache)
{
    return cache->file_error;
}

void
bfile_cached_segment_shutdown(bfile_cached_segment *segment)
{
	batom_accum_destroy(segment->accum);
	return;
}


void
bfile_cached_segment_seek(bfile_cached_segment *segment, uint64_t offset)
{
	batom_accum_clear(segment->accum);
	batom_cursor_from_accum(&segment->cursor, segment->accum);
	segment->accum_offset = offset;
	if(offset>segment->segment.len) {
		BDBG_WRN(("bfile_cached_segment_seek: %#lx outside of bounds %u:%u", (unsigned long)segment, (unsigned)offset, (unsigned)segment->segment.len));
	}
	return;
}

uint64_t
bfile_cached_segment_tell(bfile_cached_segment *segment)
{
	return segment->accum_offset + batom_cursor_pos(&segment->cursor);
}

void
bfile_cached_segment_set(bfile_cached_segment *segment, off_t start, uint64_t len)
{
    BDBG_ASSERT(start>=0);
	bfile_cached_segment_seek(segment, 0);
	bfile_segment_set(&segment->segment, start, len);
	return;
}


static bfile_segment_async_result
b_file_cached_segment_convert_result(bfile_buffer_result buffer_result)
{
    switch(buffer_result) {
    case bfile_buffer_result_ok:
        return bfile_segment_async_result_success;
    case bfile_buffer_result_async:
        return bfile_segment_async_result_async;
    case bfile_buffer_result_no_data:
        return bfile_segment_async_result_no_data;
    case bfile_buffer_result_eof:
        return bfile_segment_async_result_eof;
    default:
        return bfile_segment_async_result_error;
    }
}

static void 
b_file_cached_segment_async_read_complete(void *cntx, batom_t atom, bfile_buffer_result result)
{
	bfile_cached_segment *segment = cntx;

    BSTD_UNUSED(result);
    segment->last_read_result = result;
	if(atom) {
		batom_accum_add_atom(segment->accum, atom);
	    BDBG_MSG_TRACE(("%s:%#lx read block data buffer:%#lx at %u size:%u total:%u", "b_file_cached_segment_async_read_complete", (unsigned long)segment, (unsigned long)segment->buffer, (unsigned)(segment->segment.start + segment->accum_offset+segment->async.accum_size), batom_len(atom), batom_accum_len(segment->accum)));
		batom_cursor_from_accum(&segment->cursor, segment->accum);
		batom_release(atom);
		segment->async.read_complete(segment->async.cntx, b_file_cached_segment_convert_result(result));
	} else {
	    BDBG_MSG_TRACE(("%s:%#lx read block failed buffer:%#lx at %u size:%u accum:%u", "b_file_cached_segment_async_read_complete", (unsigned long)segment, (unsigned long)segment->buffer, (unsigned)(segment->segment.start + segment->accum_offset+segment->async.accum_size), segment->async.load_size, batom_accum_len(segment->accum)));
		segment->async.read_complete(segment->async.cntx, b_file_cached_segment_convert_result(result));
	}
}

bfile_segment_async_result
bfile_cached_segment_async_reserve(bfile_cached_segment *segment, size_t reserve_size, void (*read_complete)(void *, bfile_segment_async_result ), void *cntx)
{
	size_t accum_size;
	batom_t atom;
	size_t load_size;
	size_t cursor_pos;

	accum_size = batom_accum_len(segment->accum);
	cursor_pos = batom_cursor_pos(&segment->cursor);
    segment->last_read_result = bfile_buffer_result_ok;
	if(accum_size >= reserve_size + cursor_pos) { /* we need min_size of _new_ data, so account here current position of cursor */
		return bfile_segment_async_result_success;
	}
	batom_accum_trim(segment->accum, &segment->cursor); /* recycle old data */
	segment->accum_offset += cursor_pos;
	accum_size -= cursor_pos;
	BDBG_ASSERT(accum_size == batom_accum_len(segment->accum));
	BDBG_ASSERT(reserve_size > accum_size);
	load_size = reserve_size - accum_size;
	if(load_size<segment->min_read_size) { /* read at least B_MKV_PLAYER_CLUSTER_ENTRY */
		load_size = segment->min_read_size;
	}
	if(segment->accum_offset + load_size >  segment->segment.len) { /* don't read outside of segment boundary */
		if(segment->segment.len <= segment->accum_offset) {
			BDBG_MSG_TRACE(("%s:%#lx end of data %u:%u", "bfile_cached_segment_async_reserve", (unsigned long)segment, (unsigned)segment->segment.len, (unsigned)segment->accum_offset));
			return bfile_segment_async_result_eof;
		}
		load_size = segment->segment.len - segment->accum_offset;
		BDBG_ASSERT(load_size>0);
	}
	BDBG_MSG_TRACE(("%s:%#lx reading block data buffer:%#lx at %u size:%u", "bfile_cached_segment_async_reserve", (unsigned long)segment, (unsigned long)segment->buffer, (unsigned)(segment->segment.start + segment->accum_offset+accum_size), load_size));
	segment->async.accum_size = accum_size;
	segment->async.load_size = load_size;
	segment->async.cntx = cntx;
	segment->async.read_complete = read_complete;
	atom = bfile_buffer_async_read(segment->buffer, segment->segment.start + segment->accum_offset + accum_size, load_size, &segment->last_read_result, b_file_cached_segment_async_read_complete, segment);
	if(segment->last_read_result == bfile_buffer_result_ok && atom) {
		batom_accum_add_atom(segment->accum, atom);
	    BDBG_MSG_TRACE(("%s:%#lx read block data buffer:%#lx at %u size:%u total:%u", "bfile_cached_segment_async_reserve", (unsigned long)segment, (unsigned long)segment->buffer, (unsigned)(segment->segment.start + segment->accum_offset+accum_size), batom_len(atom), batom_accum_len(segment->accum)));
		batom_cursor_from_accum(&segment->cursor, segment->accum);
		batom_release(atom);
		return bfile_segment_async_result_success;
	} else {
        if(atom) {
            batom_release(atom);
        }
        return b_file_cached_segment_convert_result(segment->last_read_result);
	}
}


bool 
bfile_cached_segment_reserve_custom_buffer_min(bfile_cached_segment *segment, size_t reserve_size, size_t min_size, bfile_buffer_t buffer)
{
	size_t accum_size;
	batom_t atom;
	size_t load_size;
	size_t cursor_pos;

    BDBG_ASSERT(reserve_size>=min_size);

	accum_size = batom_accum_len(segment->accum);
	cursor_pos = batom_cursor_pos(&segment->cursor);
    segment->last_read_result = bfile_buffer_result_ok;
	if(accum_size >= reserve_size + cursor_pos) { /* we need min_size of _new_ data, so account here current position of cursor */
		return true;
	}
	batom_accum_trim(segment->accum, &segment->cursor); /* recycle old data */
    batom_cursor_from_accum(&segment->cursor, segment->accum); /* reseed cursor */
	segment->accum_offset += cursor_pos;
	accum_size -= cursor_pos;
	BDBG_ASSERT(accum_size == batom_accum_len(segment->accum));
	BDBG_ASSERT(reserve_size > accum_size);
	load_size = reserve_size - accum_size;
	if(load_size<segment->min_read_size) { /* read at least min_read_size */
		load_size = segment->min_read_size;
	}
	if(segment->accum_offset + load_size >  segment->segment.len) { /* don't read outside of segment boundary */
		if(segment->segment.len <= segment->accum_offset) {
			BDBG_MSG_TRACE(("%s:%#lx end of data %u:%u", "bfile_cached_segment_reserve_custom_buffer_min", (unsigned long)segment, (unsigned)segment->segment.len, (unsigned)segment->accum_offset));
			return false;
		}
		load_size = segment->segment.len - segment->accum_offset;
		BDBG_ASSERT(load_size>0);
	}
	BDBG_MSG_TRACE(("%s:%#lx reading block data buffer:%#lx at %u size:%u(%u:%u)", "bfile_cached_segment_reserve_custom_buffer_min", (unsigned long)segment, (unsigned long)buffer, (unsigned)(segment->segment.start + segment->accum_offset+accum_size), load_size, reserve_size, min_size));
	atom = bfile_buffer_read(buffer, segment->segment.start + segment->accum_offset + accum_size, load_size, &segment->last_read_result);
	if(atom) {
		batom_accum_add_atom(segment->accum, atom);
	    BDBG_MSG_TRACE(("%s:%#lx read block data buffer:%#lx at %u size:%u total:%u", "bfile_cached_segment_reserve_custom_buffer_min", (unsigned long)segment, (unsigned long)buffer, (unsigned)(segment->segment.start + segment->accum_offset+accum_size), batom_len(atom), batom_accum_len(segment->accum)));
		batom_cursor_from_accum(&segment->cursor, segment->accum);
		batom_release(atom);
	} else {
	    BDBG_MSG_TRACE(("%s:%#lx read block failed buffer:%#lx at %u size:%u(%u) result:%u accum:%u", "bfile_cached_segment_reserve_custom_buffer_min", (unsigned long)segment, (unsigned long)buffer, (unsigned)(segment->segment.start + segment->accum_offset+accum_size), load_size, min_size, segment->last_read_result, batom_accum_len(segment->accum)));
	}
    return batom_accum_len(segment->accum)>=min_size;
}

bool 
bfile_cached_segment_reserve_custom_buffer(bfile_cached_segment *segment, size_t reserve_size, bfile_buffer_t buffer)
{
    return bfile_cached_segment_reserve_custom_buffer_min(segment, reserve_size, reserve_size, buffer);
}

bool
bfile_cached_segment_reserve(bfile_cached_segment *segment, size_t reserve_size)
{
    return bfile_cached_segment_reserve_custom_buffer_min(segment, reserve_size, reserve_size, segment->buffer);
}

bool
bfile_cached_segment_reserve_min(bfile_cached_segment *segment, size_t reserve_size, size_t min_size)
{
    return bfile_cached_segment_reserve_custom_buffer_min(segment, reserve_size, min_size, segment->buffer);
}

