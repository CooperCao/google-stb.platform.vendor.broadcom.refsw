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
 * Block based cached file I/O
 *
 *******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bfile_buffer.h"
#include "blst_queue.h"

BDBG_MODULE(bfile_buffer);
BDBG_OBJECT_ID(bfile_buffer_t);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x)  */

typedef struct b_file_buffer_segment {
	BLST_Q_ENTRY(b_file_buffer_segment)  link;
	off_t off;
	size_t	size;
	unsigned ref_cnt;
	void	*base;
} b_file_buffer_segment;

BLST_Q_HEAD(b_file_buffer_list, b_file_buffer_segment);

struct bfile_buffer
{
	BDBG_OBJECT(bfile_buffer_t)
	struct b_file_buffer_list busy; /* blocks with ref_cnt > 0 */
	struct b_file_buffer_list free; /* blocks with ref_cnt == 0 */
	size_t segment_size;
	batom_accum_t acc;
	struct {
		off_t off;
		size_t size;
		b_file_buffer_segment *seg_read;
		bool wait_for_seg;
		bool active;
		void *cntx;
		void (*read_complete)(void *cntx, batom_t atom, bfile_buffer_result result);
	} async_state;
	bfile_buffer_cfg cfg;
	b_file_buffer_segment segs[1]; /* variable length array, shall be last entry */
};

#define B_FILE_BUFFER_SEG(buf, base) (&(buf)->segs[(((uint8_t *)(base) - (uint8_t *)(buf)->cfg.buf)/(buf)->segment_size)])
#define B_FILE_BUFFER_BASE(buf, seg) ((uint8_t *)(buf)->cfg.buf + ((seg)-(buf)->segs)*(buf)->segment_size)
/* #define B_FILE_BUFFER_OFF(buf, off)  ((off)-((off)%(buf)->segment_size)) */
#define B_FILE_BUFFER_OFF(buf, off)  (off - (off % (buf)->segment_size))

static void b_file_buffer_read_complete(void *cont, ssize_t size);
static void b_file_buffer_insert_segment(bfile_buffer_t buf, b_file_buffer_segment *seg);
static void b_file_buffer_async_complete(bfile_buffer_t buf, bfile_buffer_result result);
static void b_file_buffer_recycle_empty(bfile_buffer_t buf, b_file_buffer_segment *seg);

#if 0
static void 
b_file_buffer_dump(bfile_buffer_t buf, const char *name)
{
	b_file_buffer_segment *seg;
	unsigned busy_cnt=0;
	unsigned free_cnt=0;

	for(seg=BLST_Q_FIRST(&buf->busy); seg; seg=BLST_Q_NEXT(seg, link)) {
		BDBG_MSG_TRACE(("b_file_buffer_dump:%s %#lx seg:%#lx(%lu:%u) ref_cnt:%u", name, (unsigned long)buf, (unsigned long)seg,(unsigned long)seg->off, (unsigned)seg->size, seg->ref_cnt));
		busy_cnt++;
	}
	for(seg=BLST_Q_FIRST(&buf->free); seg; seg=BLST_Q_NEXT(seg, link)) {
		free_cnt++;
	}
	BDBG_MSG_TRACE(("b_file_buffer_dump:%s %#lx busy:%u free:%u total:%u(%u)", name, (unsigned long)buf, busy_cnt, free_cnt, busy_cnt+free_cnt, buf->cfg.nsegs));
	return;
}
#else
#define b_file_buffer_dump(buf, name)
#endif

static void
b_file_buffer_init_segment(bfile_buffer_t buf, b_file_buffer_segment *seg, off_t off)
{
	off_t read_off;

    BSTD_UNUSED(buf);

	read_off = B_FILE_BUFFER_OFF(buf, off);

	BDBG_ASSERT(read_off <= off);
	BDBG_ASSERT(read_off + (int)buf->segment_size >= off);
    seg->off = read_off;
    seg->size = 0;
    seg->ref_cnt = 0;
    return;
}

static bfile_buffer_result
b_file_buffer_async_read_to_segment(bfile_buffer_t buf, b_file_buffer_segment *seg)
{
	uint8_t *base;
	off_t seek_off;
	off_t seek_result;

	BDBG_ASSERT(seg);
    BDBG_ASSERT(seg->size < buf->segment_size);
	buf->async_state.active=true;
	buf->async_state.seg_read = seg;
	base = B_FILE_BUFFER_BASE(buf,seg);
    base += seg->size;
    seek_off = seg->off + seg->size;
	BDBG_MSG_TRACE(("b_file_buffer_async_read_to_segment: %#lx seg:%#lx:%lx %lld:%lld:%u", (unsigned long)buf, (unsigned long)seg, (unsigned long)base, seg->off, seek_off, (unsigned)buf->segment_size-seg->size));
	seek_result = buf->cfg.fd->seek(buf->cfg.fd, seek_off, SEEK_SET);
	if(seek_result==seek_off) {
		buf->cfg.async_read(buf->cfg.sync_cnxt, buf->cfg.fd, base, buf->segment_size - seg->size, b_file_buffer_read_complete, buf);
        return bfile_buffer_result_async;
	} else {
        buf->async_state.active=false;
        b_file_buffer_recycle_empty(buf, seg);
        return bfile_buffer_result_read_error;
	}
}

static ssize_t
b_file_buffer_read_to_segment(bfile_buffer_t buf, b_file_buffer_segment *seg)
{
	uint8_t *base;
	off_t seek_off;
	off_t seek_result;
	ssize_t read_result;
    BDBG_INT64_DEC_BUF(buf1);

	BDBG_ASSERT(seg);
    BDBG_ASSERT(seg->size < buf->segment_size);
	buf->async_state.seg_read = seg;
	base = B_FILE_BUFFER_BASE(buf,seg);
    base += seg->size;
    seek_off = seg->off + seg->size;
	BDBG_MSG_TRACE(("b_file_buffer_read_to_segment: %#lx seg:%#lx:%lx %lld:%lld:%u", (unsigned long)buf, (unsigned long)seg, (unsigned long)base, seg->off, seek_off, (unsigned)buf->segment_size-seg->size));
	seek_result = buf->cfg.fd->seek(buf->cfg.fd, seek_off, SEEK_SET);
	if(seek_result==seek_off) {
		read_result = buf->cfg.fd->read(buf->cfg.fd, base, buf->segment_size-seg->size);
	} else {
		read_result = -1;
	}
    b_file_buffer_dump(buf, "b_file_buffer_read_to_segment");
	BDBG_MSG(("b_file_buffer_read_to_segment:%p " BDBG_INT64_DEC_FMT ":%d", (void *)seg, BDBG_INT64_DEC_ARG(buf1,seek_off), (int)read_result));
	BDBG_MSG_TRACE(("b_file_buffer_read_to_segment<: %p seg:%p:%lx %lld:%lld -> %d", (void *)buf, (void *)seg, (void *)base, seg->off, seek_off, (int)read_result));
	return read_result;
}


static void
b_file_buffer_atom_free(batom_t atom, void *user)
{
	bfile_buffer_t buf = *(bfile_buffer_t*)user;
	b_file_buffer_segment *seg, *buf_id;
	unsigned nvec;
	const batom_vec *vec;
	BDBG_OBJECT_ASSERT(buf, bfile_buffer_t);
	BDBG_MSG_TRACE(("b_file_buffer_atom_free: %#lx recycle atom:%#lx", (unsigned long)buf, (unsigned long)atom));
	for(nvec = 0; NULL!=(vec=batom_get_vec(atom, nvec));nvec++) {
		BDBG_ASSERT(vec->base >= buf->cfg.buf && (uint8_t *)vec->base <= (uint8_t *)buf->cfg.buf+buf->cfg.buf_len);
		buf_id = B_FILE_BUFFER_SEG(buf, vec->base);
#if BDBG_DEBUG_BUILD
		{
			for(seg=BLST_Q_FIRST(&buf->busy); seg; seg=BLST_Q_NEXT(seg, link)) {
			/* verify that buf_id belongs to tbe buffer */
				if(buf_id==seg) {
					break;
				}
			}
			if(seg==NULL) {
				BDBG_ERR(("b_file_buffer_atom_free: %#lx unknown buf_id:%lx", (unsigned long)buf, (unsigned long)buf_id));
				return;
			}
		}
#endif
		seg = buf_id;
		BDBG_MSG_TRACE(("b_file_buffer_atom_free: %#lx recycle atom:%#lx seg:%#lx(%lu:%u) ref_cnt:%u", (unsigned long)buf, (unsigned long)atom, (unsigned long)seg,(unsigned long)seg->off, (unsigned)seg->size, seg->ref_cnt));
		BDBG_ASSERT(seg->ref_cnt>0);
		seg->ref_cnt--;
		if(seg->ref_cnt==0) {
			BLST_Q_REMOVE(&buf->busy, seg, link);
			if(!buf->async_state.wait_for_seg) {
				BLST_Q_INSERT_HEAD(&buf->free, seg, link);
			} else {
                bfile_buffer_result result;
				BDBG_MSG_TRACE(("b_file_buffer_atom_free: %#lx restart_io seg:%#lx", (unsigned long)buf, (unsigned long)seg));
				buf->async_state.wait_for_seg = false;
                b_file_buffer_init_segment(buf, seg, buf->async_state.off);
                b_file_buffer_insert_segment(buf, seg);
				result = b_file_buffer_async_read_to_segment(buf, seg);
                if(result!=bfile_buffer_result_async) {
                    b_file_buffer_async_complete(buf, result);
                }
			}
		}
	}
    b_file_buffer_dump(buf, "b_file_buffer_atom_free");
	return;
}

static const batom_user b_file_buffer_atom = {
	b_file_buffer_atom_free,
	sizeof(bfile_buffer_t)
};

static void
b_file_buffer_insert_segment(bfile_buffer_t buf, b_file_buffer_segment *seg)
{
	b_file_buffer_segment *prev,*s;
	off_t off = seg->off;
	BDBG_MSG_TRACE(("b_file_buffer_insert_segment>: %#lx seg:%#lx %u", (unsigned long)buf, (unsigned long)seg, (unsigned)seg->off));
	for(prev=NULL, s=BLST_Q_FIRST(&buf->busy); s; s=BLST_Q_NEXT(s, link)) {
	    BDBG_MSG_TRACE(("b_file_buffer_insert_segment: %#lx seg:%#lx off:%u s->off:%u prev:%#lx", (unsigned long)buf, (unsigned long)seg, (unsigned)off, (unsigned)s->off, (unsigned long)prev));
		if(off > s->off) {
			break;
		}
		prev = s;
	}
	if(prev==NULL) {
	    BDBG_MSG_TRACE(("b_file_buffer_insert_segment<: %#lx seg:%#lx TOP %u", (unsigned long)buf, (unsigned long)seg, (unsigned)off));
		BLST_Q_INSERT_HEAD(&buf->busy, seg, link);
	} else {
	    BDBG_MSG_TRACE(("b_file_buffer_insert_segment<: %#lx seg:%#lx prev:%u %u", (unsigned long)buf, (unsigned long)seg, (unsigned)prev->off, (unsigned)off));
		BLST_Q_INSERT_AFTER(&buf->busy, prev, seg, link);
	}
	return;
}

static b_file_buffer_segment *
b_file_buffer_segment_find(bfile_buffer_t buf, off_t off)
{
	b_file_buffer_segment *seg;
    off_t read_off;

	BDBG_MSG_TRACE(("b_file_buffer_segment_find>:%#lx %lld", (unsigned long)buf, off));
	BDBG_OBJECT_ASSERT(buf, bfile_buffer_t);

	read_off = B_FILE_BUFFER_OFF(buf, off);

	/* 1. Validate that segment in the range of 'busy' segments (busy segments are sorted by offset) */
	seg = BLST_Q_LAST(&buf->busy); /* min offset */
    BDBG_MSG_TRACE(("seg:%p LAST off:%u %u", seg, (unsigned)off, seg?(unsigned)(seg->off):0));
	if(seg && read_off>=seg->off) {
		seg=BLST_Q_FIRST(&buf->busy); /* max offset */
        BDBG_ASSERT(seg);
        BDBG_MSG_TRACE(("seg:%p FIRST off:%u %u", seg, (unsigned)off, (unsigned)(seg->off+(int)seg->size)));
		if(read_off<=seg->off) {
            /* 2. Try to find segment in the sorted (high to low )list */
		 	do {
                BDBG_MSG_TRACE(("seg:%p search[1] off:%u %u", seg, (unsigned)off, (unsigned)(seg->off)));
				BDBG_ASSERT(seg->size < 1u<<31);
				if(read_off <= seg->off) {
                    BDBG_MSG_TRACE(("seg:%p search[2] off:%u %u", seg, (unsigned)off, (unsigned)(seg->off+(int)seg->size)));
                    if(read_off == seg->off) {
    					/* bingo, found segment */
    					BDBG_MSG_TRACE(("b_file_buffer_segment_find<:%#lx %lld seg:%#lx %lld:%u:%u", (unsigned long)buf, off, (unsigned long)seg, seg->off, (unsigned)seg->size, seg->ref_cnt));
    					return seg;
                    }  else {
                        /* because segments are sorted stop  search here */
                        break;
                    }
                }  
			} while (NULL!=(seg=BLST_Q_NEXT(seg, link)));
		}
	}
    /* 3. Do a search in the free list */
    for(seg=BLST_Q_FIRST(&buf->free);seg;seg=BLST_Q_NEXT(seg, link)) {
        BDBG_ASSERT(seg->size < 1u<<31);
        if(seg->size && seg->off == read_off) {
            BLST_Q_REMOVE(&buf->free, seg, link);
            b_file_buffer_insert_segment(buf, seg);
            BDBG_MSG_TRACE(("b_file_buffer_segment_find<:%#lx %lld seg:%#lx %lld:%u:%u", (unsigned long)buf, off, (unsigned long)seg, seg->off, (unsigned)seg->size, seg->ref_cnt));
            return seg;
        }
    }
    /* nothing found */
	BDBG_MSG_TRACE(("b_file_buffer_segment_find<:%#lx %lld NOT FOUND", (unsigned long)buf, off));
	return NULL;
}


static size_t
b_file_buffer_add_segment(bfile_buffer_t buf, b_file_buffer_segment *seg, off_t off, size_t size)
{
	unsigned buf_off;
	uint8_t *base=NULL;
	size_t atom_size=0;

	BDBG_ASSERT(seg);
	BDBG_ASSERT(off >= seg->off);
	BDBG_MSG_TRACE(("b_file_buffer_add_segment>: %#lx seg:%#lx(%u):%lld:%u %lld:%u", (unsigned long)buf, (unsigned long)seg, seg->ref_cnt, seg->off, (unsigned)seg->size, off, (unsigned)size));
	buf_off = off-seg->off;
    if(seg->size > buf_off) {
        atom_size = seg->size - buf_off;
        if(atom_size>size) {
            atom_size = size;
        }
        if(atom_size>BATOM_VEC_MAX_SIZE) {
            atom_size = BATOM_VEC_MAX_SIZE;
        }
        seg->ref_cnt++;
        base = B_FILE_BUFFER_BASE(buf, seg);
        batom_accum_add_range(buf->acc, base+buf_off, atom_size);
    }
	BDBG_MSG_TRACE(("b_file_buffer_add_segment<: %#lx seg:%#lx(%u) %lld:%u -> %#lx:%u", (unsigned long)buf, (unsigned long)seg, seg->ref_cnt, off, (unsigned)size, (unsigned long)(base+buf_off), (unsigned)atom_size));
	return atom_size;
}

static void
b_file_buffer_recycle_empty(bfile_buffer_t buf, b_file_buffer_segment *seg)
{
    if(seg->ref_cnt==0) {
        BLST_Q_REMOVE(&buf->busy, seg, link);
        BLST_Q_INSERT_TAIL(&buf->free, seg, link);
    }
    return;
}

static b_file_buffer_segment *
b_file_buffer_get_read_segment(bfile_buffer_t buf, off_t off)
{
    b_file_buffer_segment *seg;
    seg = b_file_buffer_segment_find(buf, off);
    BDBG_MSG_TRACE(("b_file_buffer_get_read_segment: %#lx %lld seg:%#lx", (unsigned long)buf, off, (unsigned long)seg));
    if(seg==NULL) { /* read to the new segment */
        if(BLST_Q_LAST(&buf->free)==NULL) {
            b_file_buffer_segment *next;

			for(seg=BLST_Q_FIRST(&buf->busy); seg; ) {
                next = BLST_Q_NEXT(seg,link);
                if(seg->ref_cnt==0) {
                    BLST_Q_REMOVE(&buf->busy, seg, link);
                    BLST_Q_INSERT_TAIL(&buf->free, seg, link);
                }
                seg = next;
            }
        }
        seg = BLST_Q_LAST(&buf->free); /* use element from the free queue */
        if(seg) {
            BLST_Q_REMOVE(&buf->free, seg, link);
            /* initialize new segment */
            b_file_buffer_init_segment(buf, seg, off);
            b_file_buffer_insert_segment(buf, seg);
        } else {
            BDBG_WRN(("%p: run out of memory (%u / %u)", (void *)buf, (unsigned)buf->cfg.buf_len, buf->cfg.nsegs));
        }
    }
    return seg;
}

static batom_t
b_file_buffer_complete(bfile_buffer_t buf)
{
	batom_t atom;
    if(batom_accum_len(buf->acc)>0) {
	    atom = batom_from_accum(buf->acc, &b_file_buffer_atom, &buf);
    } else {
        atom = NULL;
    }
	BDBG_MSG_TRACE(("b_file_buffer_complete: %#lx atom:%#lx(%u)", (unsigned long)buf, (unsigned long)atom, atom?batom_len(atom):0));
	b_file_buffer_dump(buf, "b_file_buffer_complete");
    return atom;
}

static void
b_file_buffer_async_complete(bfile_buffer_t buf, bfile_buffer_result result)
{
	batom_t atom = b_file_buffer_complete(buf);
    buf->async_state.active = false;
	buf->async_state.read_complete(buf->async_state.cntx, atom, result);
    return;
}


static batom_t
b_file_buffer_async_read(bfile_buffer_t buf, off_t off, size_t length, bfile_buffer_result *sync_result)
{
    size_t left;
    batom_t atom;
    bfile_buffer_result result = bfile_buffer_result_ok;

    BDBG_MSG_TRACE(("b_file_buffer_async_read>: %#lx %lld:%u", (unsigned long)buf, off, (unsigned)length));
    if(off<0) { 
        BDBG_WRN(("b_file_buffer_async_read: %#lx negative offset %ld", (unsigned long)buf, (long)off));
        goto error; 
    }
    for(left=length;left;) {
        size_t atom_size;
        b_file_buffer_segment *seg = b_file_buffer_get_read_segment(buf, off);
        if(seg==NULL) {
            switch(buf->cfg.async_overflow(buf->cfg.sync_cnxt)) {
            default:
            case bfile_buffer_overflow_action_abort:
                BDBG_MSG_TRACE(("b_file_buffer_async_read<: %#lx aborted request", (unsigned long)buf));
                result = bfile_buffer_result_buffer_overflow;
                goto done;
            case bfile_buffer_overflow_action_wait:
                BDBG_MSG_TRACE(("b_file_buffer_async_read<: %#lx waiting for segment", (unsigned long)buf));
                buf->async_state.wait_for_seg = true;
                buf->async_state.off = off;
                buf->async_state.size = left;
                if(sync_result) {
                    *sync_result = bfile_buffer_result_async;
                }
                return NULL;
            }
        }
        atom_size = b_file_buffer_add_segment(buf, seg, off, left);
        BDBG_ASSERT(atom_size<=left);
        left-=atom_size;
        off+=atom_size;
        if(left==0) {
            goto done;
        }
        if(seg->size != buf->segment_size) { /* need to read data */
            b_file_buffer_dump(buf, "b_file_buffer_async_read");
            buf->async_state.off = off;
            buf->async_state.size = left;
            result = b_file_buffer_async_read_to_segment(buf, seg);
            if(result!=bfile_buffer_result_async) { goto done;}
            if(sync_result) {
                *sync_result = result;
                BDBG_MSG_TRACE(("b_file_buffer_async_read<: %#lx async_read", (unsigned long)buf));
            }
            return NULL;
        } 
    }
done:
    atom = b_file_buffer_complete(buf);
completed:
    if(sync_result) {
        *sync_result = result;
        return atom;
    } else {
        buf->async_state.active = false;
        buf->async_state.read_complete(buf->async_state.cntx, atom, result);
        return NULL;
    }
error:
    result = bfile_buffer_result_read_error;
    atom = NULL;
    goto completed;
}

static bfile_buffer_result 
b_file_buffer_map_read_error(ssize_t result)
{
    switch(result) {
    case 0: 
        return bfile_buffer_result_eof;
    case BFILE_ERROR_NO_DATA:
        return bfile_buffer_result_no_data;
    default: 
        return bfile_buffer_result_read_error; 
    }
}

static void
b_file_buffer_read_complete(void *cont, ssize_t read_size)
{
	bfile_buffer_t buf = cont;
	b_file_buffer_segment *seg;
    bfile_buffer_result result = bfile_buffer_result_ok;

	BDBG_OBJECT_ASSERT(buf, bfile_buffer_t);
	BDBG_MSG_TRACE(("b_file_buffer_read_complete>: %#lx %d", (unsigned long)buf, (int)read_size));

	seg = buf->async_state.seg_read;
	BDBG_ASSERT(seg);
    if(read_size>0) {
	    size_t atom_size;
	    off_t off = buf->async_state.off;
	    size_t left = buf->async_state.size;

        seg->size += read_size;
        BDBG_ASSERT(seg->size <= buf->segment_size);

        atom_size = b_file_buffer_add_segment(buf, seg, off, left);
        BDBG_ASSERT(atom_size<=left);
        left-=atom_size;
        off+=atom_size;
        if(left!=0) {
            if(seg->size == buf->segment_size) {  /* keep on reading */
                b_file_buffer_async_read(buf, off, left, NULL);
                return;
            }
            result = bfile_buffer_result_no_data;
        }
    } else {
        b_file_buffer_recycle_empty(buf, seg);
        result = b_file_buffer_map_read_error(read_size);
    }
    b_file_buffer_async_complete(buf, result);
    return;
}

batom_t
bfile_buffer_async_read(bfile_buffer_t buf, off_t off, size_t length, bfile_buffer_result *result, void (*read_complete)(void *, batom_t, bfile_buffer_result ), void *cntx)
{
	batom_t atom;
	BDBG_OBJECT_ASSERT(buf, bfile_buffer_t);
	BDBG_MSG_TRACE(("bfile_buffer_async_read>: %#lx %lld:%u", (unsigned long)buf, off, (unsigned)length));

	BDBG_ASSERT(buf->cfg.async);
	BDBG_ASSERT(buf->cfg.async_read);
	BDBG_ASSERT(batom_accum_len(buf->acc)==0); /* accumulator shall be empty */
	BDBG_ASSERT(!buf->async_state.active); /* and we shall not be waiting for completion of asyncrhonous transaction */
	buf->async_state.read_complete = read_complete;
	buf->async_state.cntx = cntx;
	b_file_buffer_dump(buf, "bfile_buffer_async_read+");
	atom = b_file_buffer_async_read(buf, off, length, result);
	b_file_buffer_dump(buf, "bfile_buffer_async_read-");
	return atom;
}

static batom_t
b_file_buffer_read(bfile_buffer_t buf, off_t off, size_t length, bfile_buffer_result *result)
{
    size_t left;
    batom_t atom;

    BDBG_MSG_TRACE(("b_file_buffer_read>: %#lx %lld:%u", (unsigned long)buf, off, (unsigned)length));
    if(off<0) { 
        BDBG_WRN(("b_file_buffer_read: %#lx negative offset %ld", (unsigned long)buf, (long)off));
        goto error; 
    }
    b_file_buffer_dump(buf, "b_file_buffer_read+");
    *result = bfile_buffer_result_ok;

    for(left=length;left;) {
        b_file_buffer_segment *seg;
        size_t atom_size;

        seg = b_file_buffer_get_read_segment(buf, off);
        BDBG_MSG_TRACE(("b_file_buffer_read: %#lx %lld:%u seg:%#lx", (unsigned long)buf, off, (unsigned)length, (unsigned long)seg));
        if(seg==NULL) {
            *result = bfile_buffer_result_buffer_overflow;
            break;
        }
        /* add segment */
        atom_size = b_file_buffer_add_segment(buf, seg, off, left);
        BDBG_ASSERT(atom_size<=left);
        left-=atom_size;
        off+=atom_size;
        if(left==0) {
            break;
        }
        if(seg->size != buf->segment_size) { /* need to read data */
            ssize_t read_result;
            read_result = b_file_buffer_read_to_segment(buf, seg);
            if(read_result>0) {
                seg->size += read_result;
                BDBG_ASSERT(seg->size <= buf->segment_size);
                /* add segmenta (new read data) */
                atom_size = b_file_buffer_add_segment(buf, seg, off, left);
                BDBG_ASSERT(atom_size<=left);
                left-=atom_size;
                off+=atom_size;
                if(left==0) {
                    break;
                }
            } else {
                b_file_buffer_recycle_empty(buf, seg);
                *result = b_file_buffer_map_read_error(read_result);
                break;
            }
        }
    }
    atom = b_file_buffer_complete(buf);
done:
    BDBG_MSG_TRACE(("b_file_buffer_read<: %#lx %lld:%u %#lx:%u", (unsigned long)buf, off, (unsigned)length, (unsigned long)atom, (unsigned)*result));
    return atom;
error:
    *result = bfile_buffer_result_read_error;
    atom = NULL;
    goto done;
}

batom_t 
bfile_buffer_read(bfile_buffer_t buf, off_t off, size_t length, bfile_buffer_result *result)
{
	batom_t atom;

	BDBG_MSG_TRACE(("bfile_buffer_read>: %#lx %lld:%u", (unsigned long)buf, off, (unsigned)length));
	BDBG_OBJECT_ASSERT(buf, bfile_buffer_t);
	BDBG_ASSERT(result);
	BDBG_ASSERT(!buf->async_state.active); /* we shall not be waiting for the completion of asyncrhonous transaction */
    BDBG_ASSERT(!buf->async_state.wait_for_seg);
	buf->async_state.read_complete = NULL;
	atom = b_file_buffer_read(buf, off, length, result);
	BDBG_MSG_TRACE(("bfile_buffer_read<: %#lx %lld:%u %#lx:%u", (unsigned long)buf, off, (unsigned)length, (unsigned long)atom, (unsigned)*result));
	return atom;
}


static bfile_buffer_overflow_action
b_file_default_async_overflow(void *sync_cnxt)
{
    BSTD_UNUSED(sync_cnxt);
    return bfile_buffer_overflow_action_abort;
}


void
bfile_buffer_default_cfg(bfile_buffer_cfg *cfg)
{
	cfg->buf = NULL;
	cfg->async = false;
	cfg->nsegs = 32;
    /* to handle MP4/MKV streams this should be as large as largest frame in the stream */
	cfg->buf_len = cfg->nsegs*(BIO_BLOCK_SIZE*15);
	cfg->fd = NULL;
	cfg->async_read = NULL;
	cfg->sync_cnxt = NULL;
    cfg->async_overflow = b_file_default_async_overflow;
	return;
}

bfile_buffer_t
bfile_buffer_create(batom_factory_t factory, bfile_buffer_cfg *cfg)
{
	bfile_buffer_t buf;
	unsigned i;
	size_t segment_size;

	BDBG_ASSERT(factory);
	BDBG_ASSERT(cfg);
	if(!cfg->buf) {
		BDBG_ERR(("bfile_buffer_create: buffer wasn't provided"));
		goto err_param;
	}
	if(cfg->nsegs<=0) {
		BDBG_ERR(("bfile_buffer_create: invalid number of segments"));
		goto err_param;
	}
	if(cfg->buf_len<=0) {
		BDBG_ERR(("bfile_buffer_create: invalid buffer size"));
		goto err_param;
	}
	if(!cfg->fd) {
		BDBG_ERR(("bfile_buffer_create: file descriptor wasn't provided"));
		goto err_param;
	}
	if(cfg->async && !cfg->async_read) {
		BDBG_ERR(("bfile_buffer_create: invalid options for asyn I/O"));
		goto err_param;
	}
	segment_size = (cfg->buf_len/cfg->nsegs)&(~(BIO_BLOCK_SIZE-1));
	if(segment_size<BIO_BLOCK_SIZE) {
		BDBG_ERR(("bfile_buffer_create: segment size %u < %u (buf_len:%u nsegs:%u -> %u)", (unsigned)segment_size, (unsigned)BIO_BLOCK_SIZE, (unsigned)cfg->buf_len,  (unsigned)cfg->nsegs, (unsigned)cfg->buf_len/cfg->nsegs));
		goto err_param;
	}
	buf = BKNI_Malloc(sizeof(*buf)+ sizeof(b_file_buffer_segment)*cfg->nsegs);
    BDBG_MSG_TRACE(("bfile_buffer_create:%#lx  %#lx %u:%u(%u) %s", (unsigned long)buf, (unsigned long)cfg->buf, (unsigned)cfg->buf_len, (unsigned)cfg->nsegs, (unsigned)segment_size, cfg->async?"ASYNC":"")); 
	if(!buf) {
		BDBG_ERR(("bfile_buffer_create: can't allocate %u bytes", (unsigned)(sizeof(*buf)+ sizeof(b_file_buffer_segment)*cfg->nsegs)));
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(buf, bfile_buffer_t);
	buf->cfg = *cfg;	
	buf->segment_size = segment_size;
	buf->async_state.active=false;
    buf->async_state.wait_for_seg = false;
	buf->acc = batom_accum_create(factory);
	if(!buf->acc) {
		goto err_acc;

	}
	BLST_Q_INIT(&buf->free);
	BLST_Q_INIT(&buf->busy);
	for(i=0;i<cfg->nsegs;i++) {
		b_file_buffer_segment *seg=&buf->segs[i];

		seg->off = 0;
		seg->size = 0;
		seg->ref_cnt = 0;

		BLST_Q_INSERT_TAIL(&buf->free, seg, link);
	}
	return buf;
err_acc:
	BKNI_Free(buf);
err_alloc:
err_param:
	return NULL;
}

void 
bfile_buffer_clear(bfile_buffer_t buf)
{
	unsigned i;
    BDBG_INT64_DEC_BUF(buf);

	BDBG_OBJECT_ASSERT(buf, bfile_buffer_t);
	if(buf->async_state.active) {
		BDBG_WRN(("bfile_buffer_clear: %#lx clearing while doing async I/O", (unsigned long)buf));
	}
	if(BLST_Q_FIRST(&buf->busy)) {
		BDBG_WRN(("bfile_buffer_clear: %#lx clearing while have active buffers", (unsigned long)buf));
#if BDBG_DEBUG_BUILD
		{
			b_file_buffer_segment *seg;
			for(seg=BLST_Q_FIRST(&buf->busy); seg; seg=BLST_Q_NEXT(seg, link)) {
				BDBG_WRN(("bfile_buffer_clear: %p buffers %p:%u "BDBG_INT64_DEC_FMT ":%u", (void *)buf, (void *)seg, seg->ref_cnt, BDBG_INT64_DEC_ARG(buf,seg->off), (unsigned)seg->size ));
			}
		}
#endif
	}
	batom_accum_clear(buf->acc);
	BLST_Q_INIT(&buf->free);
	BLST_Q_INIT(&buf->busy);
	for(i=0;i<buf->cfg.nsegs;i++) {
		b_file_buffer_segment *seg=&buf->segs[i];

		seg->off = -1;
		seg->size = 0;
		seg->ref_cnt = 0;

		BLST_Q_INSERT_TAIL(&buf->free, seg, link);
	}
    return;
}


void 
bfile_buffer_destroy(bfile_buffer_t buf)
{
	BDBG_OBJECT_ASSERT(buf, bfile_buffer_t);
    bfile_buffer_clear(buf);
	batom_accum_destroy(buf->acc);
	BDBG_OBJECT_DESTROY(buf, bfile_buffer_t);
	BKNI_Free(buf);
	return;
}

int 
bfile_buffer_get_bounds(bfile_buffer_t buf, off_t *first, off_t *last)
{
	BDBG_OBJECT_ASSERT(buf, bfile_buffer_t);
	return buf->cfg.fd->bounds(buf->cfg.fd, first, last);
}
