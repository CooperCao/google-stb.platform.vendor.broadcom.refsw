/***************************************************************************
 *     Copyright (c) 2007 Broadcom Corporation
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
 * ATOM playback data sink
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bsink_playback.h"
#include "bkni.h"

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */
BDBG_MODULE(bsink_playback);

/* only one should be enabled at the time, WRITE writes data to file and READ reads this data leter on */
/* #define WRITE_TIMED_DATA 1 */
/* #define READ_TIMED_DATA 1 */

/* #define DUMP_PES_OUT 1 */

#ifdef DUMP_PES_OUT
#include <stdio.h>
FILE *fp;
#endif

#if WRITE_TIMED_DATA  || READ_TIMED_DATA
#include "bsink_playback_debug.h"
#include <stdio.h>
#include <sys/time.h>

static FILE *
b_sink_playback_open_file(const char *mode, unsigned instance, unsigned *count)
{
    FILE *f;
    char name[64];

    BKNI_Snprintf(name, sizeof(name), "timed_data_%u_%u.dat",  instance, *count);
    BDBG_WRN(("using dump file '%s'", name));
    f = fopen(name, mode);
    BDBG_ASSERT(f);
    (*count)++;
    return f;
}

static long
b_sink_playback_dump_timediff(const struct timeval *future, const struct timeval *past)
{
    return 1000*(future->tv_sec - past->tv_sec) + (future->tv_usec - past->tv_usec)/1000;
}
#endif /* WRITE_TIMED_DATA  || READ_TIMED_DATA */

#if READ_TIMED_DATA
#include "balloc.h"
balloc_iface_t bsink_dump_reader_allocator = NULL;

typedef struct b_sink_playback_read {
    FILE *fp;
    bool get_time;
    unsigned count;
    unsigned instance;
    struct timeval start_time;
} b_sink_playback_read;

static void 
b_sink_playback_dump_init(b_sink_playback_read *reader)
{
    static int bsink_instance=0;

    reader->fp = NULL;
    reader->count = 0;
    reader->instance = bsink_instance;
    bsink_instance++;
    return;
}

static void 
b_sink_playback_dump_start(b_sink_playback_read *reader)
{
    reader->fp = b_sink_playback_open_file("rb", reader->instance, &reader->count);
#if 1
    reader->get_time = true;
#else
    reader->get_time = false;
    gettimeofday(&reader->start_time, NULL);
#endif
    return;
}

static void 
b_sink_playback_dump_stop(b_sink_playback_read *reader)
{
    fclose(reader->fp);
    reader->fp = NULL;
}

static void
b_sink_reader_free(batom_t atom, void *user)
{
    batom_t src_atom = *(batom_t *)user;
	unsigned nvecs, i;
    const batom_vec *vecs;

    BDBG_MSG(("b_sink_reader_free: %#lx %#lx", (unsigned long)atom, (unsigned long)src_atom));
	vecs = batom_get_vectors(atom, &nvecs);
    for(i=0;i<nvecs;i++) {
        bsink_dump_reader_allocator->bmem_free(bsink_dump_reader_allocator, vecs[i].base);
    }
    batom_release(src_atom);
    return;
}

static const batom_user b_sink_reader = {
	b_sink_reader_free,
	sizeof(batom_t)
};

static batom_t
b_sink_playback_dump_data(b_sink_playback_read *reader, batom_t atom, batom_factory_t factory)
{
	unsigned nvecs, i;
    int rc;
    batom_vec *dst_vec;
    batom_vec dst_static_vec[4];
    bsink_playback_packet packet;
    struct timeval now;
    uint32_t time_diff;

    BDBG_ASSERT(bsink_dump_reader_allocator);
    if(reader->get_time) {
        reader->get_time = false;
        gettimeofday(&reader->start_time, NULL);
    }
    gettimeofday(&now, NULL);
    for(;;) {
        rc = fread(&packet, sizeof(packet), 1, reader->fp);
        if(rc==1) {
            BDBG_ASSERT(packet.sync == BSINK_PLAYBACK_SYNC);
            break;
        }
        BDBG_ASSERT(rc==0);
        BDBG_WRN(("end of source data, loop"));
        fseek(reader->fp, 0, SEEK_SET);
        gettimeofday(&reader->start_time, NULL);
    }
    time_diff = b_sink_playback_dump_timediff(&now, &reader->start_time);
    if(packet.timestamp > time_diff) {
        BKNI_Sleep(packet.timestamp - time_diff);
    }
    nvecs = packet.sequence+1;

    if(nvecs<sizeof(dst_static_vec)/sizeof(*dst_static_vec)) {
        dst_vec = dst_static_vec;
    } else {
        dst_vec = BKNI_Malloc(sizeof(*dst_vec)*nvecs);
        BDBG_ASSERT(dst_vec);
    }
    for(i=0;;) {
        dst_vec[i].dirty = true;
        dst_vec[i].unused_flags = 0;
        dst_vec[i].len = packet.length;
        dst_vec[i].base = bsink_dump_reader_allocator->bmem_alloc(bsink_dump_reader_allocator, packet.length);
        BDBG_ASSERT(dst_vec[i].base);
        rc = fread(dst_vec[i].base, packet.length, 1, reader->fp);
        BDBG_ASSERT(rc==1);
        i++;
        if(i>=nvecs) {
            break;
        }
        rc = fread(&packet, sizeof(packet), 1, reader->fp);
        BDBG_ASSERT(rc==1);
        BDBG_ASSERT(packet.sync == BSINK_PLAYBACK_SYNC);
    }
    atom = batom_from_vectors(factory, dst_vec, nvecs, &b_sink_reader, &atom);
    BDBG_ASSERT(atom);
    if(dst_vec != dst_static_vec) {
        BKNI_Free(dst_vec);
    }
    return atom;
}
#elif WRITE_TIMED_DATA  
typedef struct b_sink_playback_dump {
    FILE *fp;
    unsigned count;
    unsigned instance;
    struct timeval start_time;
} b_sink_playback_dump;

static void 
b_sink_playback_dump_init(b_sink_playback_dump *writer)
{
    static int bsink_instance=0;

    writer->fp = NULL;
    writer->count = 0;
    writer->instance = bsink_instance;
    bsink_instance++;
    return;
}

static void 
b_sink_playback_dump_start(b_sink_playback_dump *writer)
{
    writer->fp = b_sink_playback_open_file("wb", writer->instance, &writer->count);
    gettimeofday(&writer->start_time, NULL);
    return;
}

static void 
b_sink_playback_dump_stop(b_sink_playback_dump *writer)
{
    fclose(writer->fp);
    writer->fp = NULL;
}


static batom_t
b_sink_playback_dump_data(b_sink_playback_dump *writer, batom_t atom, batom_factory_t factory)
{
	unsigned nvecs, i;
    const batom_vec *vecs;
    bsink_playback_packet packet;
    struct timeval now;

    BSTD_UNUSED(factory);
    gettimeofday(&now, NULL);
    packet.sync = BSINK_PLAYBACK_SYNC;
	vecs = batom_get_vectors(atom, &nvecs);
    packet.timestamp = b_sink_playback_dump_timediff(&now, &writer->start_time);
    for(i=0;i<nvecs;i++) {
        packet.sequence =  nvecs - i - 1;
        packet.length = vecs[i].len;
        fwrite(&packet, sizeof(packet), 1, writer->fp);
        fwrite(vecs[i].base, 1, vecs[i].len, writer->fp);
    }
    fflush(writer->fp);
    return atom;
}
#else
#define b_sink_playback_dump_init(writer)
#define b_sink_playback_dump_start(writer)
#define b_sink_playback_dump_stop(writer)
#define b_sink_playback_dump_data(writer, atom, factory) (atom)
#endif /* WRITE_TIMED_DATA */


BDBG_OBJECT_ID(bsink_playback_t);


#define B_SINK_PLAYBACK_NENTRIES	32
struct bsink_playback {
	BDBG_OBJECT(bsink_playback_t)
	bsink_playback_cfg cfg;
	struct {
		batom_t atom;
		unsigned vec;
	} in, out;
	batom_pipe_t out_pipe;
	unsigned nfreedesc;
	unsigned nplaydesc;
	BPVRlib_Feed_OffsetEntry entries[B_SINK_PLAYBACK_NENTRIES];
    batom_factory_t factory;
#if READ_TIMED_DATA
    struct b_sink_playback_read dumper;
#elif WRITE_TIMED_DATA 
    struct b_sink_playback_dump dumper;
#endif
};


void
bsink_playback_default_cfg(bsink_playback_cfg *cfg)
{
	cfg->addr_to_offset = NULL;
	cfg->cache_flush = NULL;
	return;
}

static void
b_sink_playback_reset(bsink_playback_t sink)
{
	sink->in.atom = NULL;
	sink->in.vec = 0;
	sink->out.atom = NULL;
	sink->out.vec = 0;
	sink->nfreedesc = 0;
	sink->nplaydesc = 0;
	return;
}

bsink_playback_t
bsink_playback_create(batom_factory_t factory, const bsink_playback_cfg *cfg)
{
	bsink_playback_t sink;

	BDBG_ASSERT(factory);
	BDBG_ASSERT(cfg);
	if(cfg->addr_to_offset == NULL || cfg->cache_flush==NULL) {
		BDBG_ERR(("bsink_playback_create: cfg.addr_to_offset and cfg.cache_flush shall be specified"));
		goto err_cfg;
	}
	sink = BKNI_Malloc(sizeof(*sink));
	if(!sink) {
		BDBG_ERR(("bsink_playback_create: can't allocate %u bytes", (unsigned)sizeof(*sink)));
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(sink, bsink_playback_t);
	sink->cfg = *cfg;
    sink->factory = factory;
	sink->out_pipe = batom_pipe_create(factory);
	if(!sink->out_pipe) {
		goto err_out_pipe;
	}
    b_sink_playback_dump_init(&sink->dumper);
	b_sink_playback_reset(sink);
	return sink;

err_out_pipe:
	BKNI_Free(sink);
err_alloc:
err_cfg:
	return NULL;
}

void
bsink_playback_destroy(bsink_playback_t sink)
{
	BDBG_OBJECT_ASSERT(sink, bsink_playback_t);
	batom_pipe_destroy(sink->out_pipe);
	BDBG_OBJECT_DESTROY(sink, bsink_playback_t);
	BKNI_Free(sink);
	return;
}

BERR_Code
bsink_playback_start(bsink_playback_t sink)
{
	BERR_Code rc;
	BPVRlib_Feed_Status status;

	BDBG_OBJECT_ASSERT(sink, bsink_playback_t);
	b_sink_playback_reset(sink);
	rc = BPVRlib_Feed_Start(sink->cfg.feed);
	if(rc!=BERR_SUCCESS) {
		rc = BERR_TRACE(rc);
		goto error;
	}
	BPVRlib_Feed_GetStatus(sink->cfg.feed, &status);
	sink->nfreedesc = status.freeDesc;
	sink->nplaydesc = status.freeDesc;
#ifdef DUMP_PES_OUT
	fp=fopen("media_out.mpg", "wb");
#endif
    b_sink_playback_dump_start(&sink->dumper);
error:
	BDBG_MSG(("bsink_playback_start: %#lx %#x", (unsigned long)sink, (unsigned)rc));
	return rc;
}

void
bsink_playback_stop(bsink_playback_t sink)
{
	BDBG_OBJECT_ASSERT(sink, bsink_playback_t);
	BDBG_MSG(("bsink_playback_stop: %#lx", (unsigned long)sink));

#if 0
	if(sink->in.atom) {
		batom_release(sink->in.atom);
		sink->in.atom = NULL;
	}
#endif
	if(sink->out.atom) {
		batom_release(sink->out.atom);
		sink->out.atom = NULL;
	}
	batom_pipe_flush(sink->out_pipe);
	BPVRlib_Feed_Stop(sink->cfg.feed);
    b_sink_playback_dump_stop(&sink->dumper);
#ifdef DUMP_PES_OUT
	fclose(fp);
#endif
	return;
}


size_t
bsink_playback_feed(bsink_playback_t sink, batom_pipe_t in_pipe)
{
	BERR_Code rc;
	unsigned i;
	unsigned nvecs, vec, last_vec;
	size_t ncompleted;
	size_t nentries;
	batom_t atom;
	size_t len=0;
	BPVRlib_Feed_OffsetEntry *entries = sink->entries;

	BDBG_OBJECT_ASSERT(sink, bsink_playback_t);
	BDBG_ASSERT(in_pipe);
	BDBG_MSG_TRACE(("bsink_playback_feed>: %#lx %#lx",  (unsigned long)sink, (unsigned long)in_pipe));

	/* step 1. get number of recycled vectors */
	rc = BPVRlib_Feed_GetCompleted(sink->cfg.feed, &ncompleted);
	if(rc!=BERR_SUCCESS) {
		rc = BERR_TRACE(rc);
		goto error;
	}
	BDBG_MSG_TRACE(("bsink_playback_feed: %#lx completed:%u",  (unsigned long)sink, ncompleted));
	/* step 2. recycle entries from out_pipe */
	atom = sink->out.atom; /* load state */
	vec = sink->out.vec;
	for(i=ncompleted;i>0;) {
		if(!atom) {
			vec = 0;
			atom = batom_pipe_pop(sink->out_pipe);
			if(!atom) {
				BDBG_ERR(("bsink_playback_feed: %#lx underflow left:%u", (unsigned long)sink, i));
				break;
			}
			BDBG_MSG_TRACE(("bsink_playback_feed: %#lx recycling:%#lx",  (unsigned long)sink, atom));
		}
		nvecs = batom_get_nvecs(atom);
		BDBG_ASSERT(nvecs>vec);
		last_vec = vec;
		vec+=i;
		if(vec < nvecs) {
			break;
		}
		/* completed entire atom */
		BDBG_ASSERT(i>=(nvecs-last_vec));
		i -= nvecs - last_vec;
		vec = 0;
		batom_release(atom);
		atom=NULL;
	}
	sink->out.vec = vec; /* save state for the next step */
	sink->out.atom = atom;

	/* step 3. Update number of free descriotrs */
	sink->nfreedesc += ncompleted;
	BDBG_ASSERT(sink->nfreedesc<=sink->nplaydesc);

	vec = sink->in.vec; /* load state */
	atom = sink->in.atom;

	/* step 4. Copy data from in_pipe into the BPVRlib_Feed entries */
	do {
		nentries = B_SINK_PLAYBACK_NENTRIES;
		if(nentries>sink->nfreedesc) {
			nentries = sink->nfreedesc;
		}
		BDBG_MSG_TRACE(("bsink_playback_feed: %#lx free:%u",  (unsigned long)sink, nentries));
		for(i=0;i<nentries;) {
			const batom_vec *vecs;
			if(!atom) {
				vec = 0;
				atom = batom_pipe_pop(in_pipe);
				if(!atom) {
					break;
				}
                atom = b_sink_playback_dump_data(&sink->dumper, atom, sink->factory);
				BDBG_MSG_TRACE(("bsink_playback_feed: %#lx feeding %#lx",  (unsigned long)sink, atom));
				batom_pipe_push(sink->out_pipe, atom);
			}
			vecs = batom_get_vectors(atom, &nvecs);
			BDBG_ASSERT(nvecs>vec);
			last_vec = vec + (nentries-i);
			if( last_vec > nvecs) {
				last_vec = nvecs;
			}
			BDBG_ASSERT(i+(last_vec-vec)<=nentries);
			/* copy vectors to the feed entries */
			for(;vec<last_vec;vec++,i++) {
				len += vecs[vec].len;
				entries[i].len = vecs[vec].len;
				if(vecs[vec].dirty) {
					sink->cfg.cache_flush(vecs[vec].base, vecs[vec].len);
				}
				entries[i].offset = sink->cfg.addr_to_offset(vecs[vec].base);
#ifdef DUMP_PES_OUT
				fwrite(vecs[vec].base, vecs[vec].len, 1, fp);
				fflush(fp);
#endif
			}
			if(vec!=nvecs) { /* some vectors are left in the atom */
				BDBG_ASSERT(i==nentries);
				break;
			}
			/* copied entire vector */
			atom = NULL;
		}
		sink->in.vec = vec;
		sink->in.atom = atom; /* save state for the next step */
		if(i==0) {
			break; /* nothing to send */
		}
		BDBG_ASSERT(sink->nfreedesc>=i);
		sink->nfreedesc -= i;
		rc = BPVRlib_Feed_AddOffsetEntries(sink->cfg.feed, entries, i, &ncompleted);
		if(rc!=BERR_SUCCESS) {
			rc = BERR_TRACE(rc);
			goto error;
		}
		if(i!=ncompleted) {
			BDBG_ERR(("bsink_playback_feed: %p BPVRlib_Feed haven't consumed expected number of entries %u:%u", (void *)sink, i, (unsigned)ncompleted));
			goto error;
		}
	} while(i==B_SINK_PLAYBACK_NENTRIES); /* repeat until we send less than complete chunk, in which case it means either BPVRlib_Feed is full or in.pipe is empty */
error:
	BDBG_MSG_TRACE(("bsink_playback_feed<: %#lx  ->%u",  (unsigned long)sink, len));
	return len;
}


void
bsink_playback_get_status(bsink_playback_t sink, bsink_playback_status *status)
{
	BDBG_OBJECT_ASSERT(sink, bsink_playback_t);
	/* sink is idle if no atom is borrowed and feed is idle (e.g. number of freedescriptors is equal to total number of descriptors */
	status->idle = sink->in.atom==NULL && sink->nfreedesc == sink->nplaydesc;
	BDBG_MSG_TRACE(("bsink_playback_status: %#lx %s", (unsigned long)sink, status->idle?"IDLE":""));
	return;
}

