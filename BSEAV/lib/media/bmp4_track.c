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
 * MP4 library, media track interface
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bmp4_track.h"
#include "bmedia_index.h"

BDBG_MODULE(bmp4_track);

#define BDBG_MSG_TRACE(x)	/* BDBG_MSG(x) */


#define B_MP4_INVALID_SIZE ((size_t)(0xFFFFFFFFul))
#define B_MP4_GET_UINT32(p,off) \
			(((uint32_t)(((uint8_t *)(p))[(off)+0]<<24) | \
			((uint32_t)(((uint8_t *)(p))[(off)+1])<<16) | \
			((uint32_t)(((uint8_t *)(p))[(off)+2])<<8) | \
			((uint32_t)(((uint8_t *)(p))[(off)+3]))))

#define B_MP4_GET_UINT24(p,off) \
			(((uint32_t)(((uint8_t *)(p))[(off)+0]<<16) | \
			((uint32_t)(((uint8_t *)(p))[(off)+1])<<8) | \
			((uint32_t)(((uint8_t *)(p))[(off)+2]))))

#define B_MP4_GET_UINT16(p,off) \
			(((uint32_t)(((uint8_t *)(p))[(off)+0]<<8) | \
			((uint32_t)(((uint8_t *)(p))[(off)+1]))))

#define B_MP4_GET_UINT64(p,off) \
			((((uint64_t)B_MP4_GET_UINT32(p,off))<<32) | B_MP4_GET_UINT32(p,(off)+4))

#define B_MP4_CACHE_FRAMES	128
#define B_MP4_CACHE_CHUNKS	64

void 
bmp4_track_info_init(bmp4_track_info *track)
{
	BKNI_Memset(track, 0, sizeof(*track));
	bfile_segment_clear(&track->decode_t2s);
	bfile_segment_clear(&track->composition_t2s);
	bfile_segment_clear(&track->samplesize);
	bfile_segment_clear(&track->usamplesize);
	bfile_segment_clear(&track->sampletochunk);
	bfile_segment_clear(&track->chunkoffset);
	bfile_segment_clear(&track->chunkoffset64);
	bfile_segment_clear(&track->syncsample);
	bfile_segment_clear(&track->edit);
    track->movieheader = NULL;
	return;
}

static bool
b_mp4_test_fullbox(bfile_cache_t cache, uint8_t version, uint32_t flags) 
{
	const uint8_t *buf;
	int rc;

	buf = bfile_cache_next(cache);
	if(!buf) {
		return false;
	}
	if(*buf!=version) {
		BDBG_WRN(("b_mp4_test_fullbox: %#lx unsupported version %u(%u)",  (unsigned long)cache, (unsigned)*buf, (unsigned)version));
		return false;
	}
	rc = bfile_cache_seek(cache, 1);
	if(rc!=0) {
		return false;
	}
	buf = bfile_cache_next(cache);
	if(!buf) {
		return false;
	}
	if(B_MP4_GET_UINT24(buf,0)!=flags) {
		BDBG_WRN(("b_mp4_test_fullbox: %#lx unsupported flags %#x(%#x)",  (unsigned long)cache, (unsigned)B_MP4_GET_UINT24(buf,0), (unsigned)flags));
	}
	rc = bfile_cache_seek(cache, BMP4_FULLBOX_SIZE);
	if(rc!=0) {
		return false;
	}
	return true;
}

typedef struct b_mp4_samplesize_stream {
	bfile_cache_t  cache;
	uint32_t sample_size;
	uint32_t sample_count;
	uint32_t count; /* current count */
} b_mp4_samplesize_stream;

static int
b_mp4_samplesize_stream_init(b_mp4_samplesize_stream *stream, bfile_io_read_t fd, const bfile_segment *segment)
{
	const uint8_t *buf;

	BDBG_ASSERT(stream);
	BDBG_ASSERT(fd);
	BDBG_ASSERT(segment);
	stream->cache = bfile_cache_create(fd, segment, B_MP4_CACHE_FRAMES*sizeof(uint32_t), sizeof(uint32_t));
	if(!stream->cache) {
		goto err_cache;
	}
	stream->count = 0;

	if(!b_mp4_test_fullbox(stream->cache, 0, 0)) {
		goto err_fullbox;
	}
	buf = bfile_cache_next(stream->cache);
	if(!buf) {
		goto err_samplesize;
	}
	stream->sample_size = B_MP4_GET_UINT32(buf,0);
	buf = bfile_cache_next(stream->cache);
	if(!buf) {
		goto err_sample_count;
	}
	stream->sample_count = B_MP4_GET_UINT32(buf,0);
	BDBG_MSG(("b_mp4_samplesize_stream_init:%#lx sample_size:%u sample_count:%u", (unsigned long)stream, (unsigned)stream->sample_size, (unsigned)stream->sample_count));
	return 0;
err_samplesize:
err_sample_count:
err_fullbox:
	BDBG_ERR(("b_mp4_samplesize_stream_init: error parsing box"));
	bfile_cache_destroy(stream->cache);
err_cache:
	return -1;
}

static void
b_mp4_samplesize_stream_shutdown(b_mp4_samplesize_stream *stream)
{
	bfile_cache_destroy(stream->cache);
	return;
}

static size_t
b_mp4_samplesize_stream_next(b_mp4_samplesize_stream *stream)
{
	if(stream->count<stream->sample_count) {
		stream->count++;
		if(stream->sample_size==0) {
			const uint8_t *buf = bfile_cache_next(stream->cache);
			if(buf) {
				size_t size = B_MP4_GET_UINT32(buf,0);
				BDBG_MSG_TRACE(("b_mp4_samplesize_stream_next: %#lx size:%u", (unsigned long)stream, (unsigned)size));
				return size;
			}
		} else {
			BDBG_MSG_TRACE(("b_mp4_samplesize_stream_next: %#lx size:%u", (unsigned long)stream, (unsigned)stream->sample_size));
			return stream->sample_size;
		}
	}
	return B_MP4_INVALID_SIZE;
}

static int
b_mp4_samplesize_stream_seek(b_mp4_samplesize_stream *stream, unsigned sample)
{
	if(sample <= stream->sample_count) {
		if(stream->sample_size==0) {
			int rc = bfile_cache_seek(stream->cache, BMP4_FULLBOX_SIZE+sizeof(uint32_t)+sizeof(uint32_t)+sample*sizeof(uint32_t));
			if(rc!=0) { goto error;}
        }
		stream->count = sample;
        return 0;
	}
error:
	return -1;
}

/* compact sample size, p 30 */
typedef struct b_mp4_samplesize2_stream {
	bfile_cache_t  cache;
	uint8_t field_size;
	uint8_t prev_size; /* previous size, used if field_size==4 */
	uint32_t sample_count;
	uint32_t count; 
} b_mp4_samplesize2_stream;

static int
b_mp4_samplesize2_stream_init(b_mp4_samplesize2_stream *stream, bfile_io_read_t fd, const bfile_segment *segment)
{
	const uint8_t *buf;
	int rc;

	BDBG_ASSERT(stream);
	BDBG_ASSERT(fd);
	BDBG_ASSERT(segment);
	stream->cache = bfile_cache_create(fd, segment, B_MP4_CACHE_FRAMES*sizeof(uint8_t), sizeof(uint8_t));
	BDBG_CASSERT(B_MP4_CACHE_FRAMES*sizeof(uint8_t)>4+4); /* cache shall have entire header */
	if(!stream->cache) {
		goto err_cache;
	}

	if(!b_mp4_test_fullbox(stream->cache, 0, 0)) {
		goto err_fullbox;
	}
	buf = bfile_cache_next(stream->cache);
	if(!buf) {
		goto err_samplecount;
	}
	stream->count = 0;
	stream->field_size = buf[3];
	switch(stream->field_size) {
	case 4: case 8:  case 16:
		break;
	default:
		BDBG_WRN(("b_mp4_samplesize2_stream: %#lx invalid field_size:%u", (unsigned long)stream, (unsigned)stream->field_size));
		goto err_fieldsize;
	}
	stream->sample_count = B_MP4_GET_UINT32(buf,4);
	BDBG_MSG(("b_mp4_samplesize2_stream_init:%#lx field_size:%u sample_count:%u", (unsigned long)stream, (unsigned)stream->field_size, (unsigned)stream->sample_count));
	rc = bfile_cache_seek(stream->cache, BMP4_FULLBOX_SIZE+sizeof(uint32_t)+3/*reserved*/+sizeof(uint8_t));
	if(rc!=0) {
		goto err_seek;
	}
	return 0;
err_seek:
err_fieldsize:
err_samplecount:
err_fullbox:
	BDBG_ERR(("b_mp4_samplesize_stream_init: error parsing box"));
	bfile_cache_destroy(stream->cache);
err_cache:
	return -1;
}

static void
b_mp4_samplesize2_stream_shutdown(b_mp4_samplesize2_stream *stream)
{
	bfile_cache_destroy(stream->cache);
	return;
}

static size_t
b_mp4_samplesize2_stream_next(b_mp4_samplesize2_stream *stream)
{
	if(stream->count<stream->sample_count) {
		const uint8_t *buf;
		size_t sample_size;
		switch(stream->field_size) {
		default:
			BDBG_ASSERT(0); /* shall not happen, field_size was verified earlier */
		case 8:
			buf = bfile_cache_next(stream->cache);
			if(!buf) {
				goto end_of_stream;
			}
			sample_size = *buf;
			break;
			case 16:
				buf = bfile_cache_next(stream->cache);
				if(!buf) {
					goto end_of_stream;
				}
				sample_size = *buf;
				buf = bfile_cache_next(stream->cache);
				if(!buf) {
					goto end_of_stream;
				}
				sample_size = (sample_size<<8)|(*buf);
				break;
			case 4:
				if(stream->count%2) {
					sample_size = stream->prev_size&0x0F;
					break;
				}
				buf = bfile_cache_next(stream->cache);
				if(!buf) {
					goto end_of_stream;
				}
				stream->prev_size = *buf;
				sample_size = stream->prev_size>>4;
				break;
		}
		stream->count++;
		BDBG_MSG_TRACE(("b_mp4_samplesize2_stream_next: %#lx size:%u", (unsigned long)stream, (unsigned)sample_size));
		return sample_size;
	}
end_of_stream:
	return B_MP4_INVALID_SIZE;
}

static int
b_mp4_samplesize2_stream_seek(b_mp4_samplesize2_stream *stream, unsigned sample)
{
	if(sample <= stream->sample_count) {
		int rc = bfile_cache_seek(stream->cache, BMP4_FULLBOX_SIZE+sizeof(uint32_t)+sizeof(uint32_t)+((sample*stream->field_size)/8));
        if(rc!=0) { goto error;}
        stream->count = sample;
	    if(stream->field_size==4 && sample%2) {
	        const uint8_t *buf = bfile_cache_next(stream->cache);
		    if(buf) {
		        stream->prev_size = *buf;
			}
	    }
	    return 0;
	}
error:
	return -1;
}

#define BMP4_SAMPLETOCHUNK_ENTRY_SIZE	(sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t))
typedef struct bmp4_sampletochunk_entry {
	uint32_t first_chunk;
	uint32_t samples_per_chunk;
	uint32_t sample_description_index;
} bmp4_sampletochunk_entry;

typedef struct b_mp4_sampletochunk_stream {
	bfile_cache_t  cache;
	uint32_t entry_count;
	uint32_t count; /* current count */
} b_mp4_sampletochunk_stream;


static int
b_mp4_sampletochunk_stream_init(b_mp4_sampletochunk_stream *stream, bfile_io_read_t fd, const bfile_segment *segment)
{
	const uint8_t *buf;
	int rc;
    bfile_cache_t temp_cache;
	BDBG_ASSERT(stream);
	BDBG_ASSERT(fd);
	BDBG_ASSERT(segment);
	temp_cache = bfile_cache_create(fd, segment, 4, sizeof(uint32_t));
    if(!temp_cache) {
		goto err_temp_cache;
    }
	stream->cache = bfile_cache_create(fd, segment, B_MP4_CACHE_CHUNKS*BMP4_SAMPLETOCHUNK_ENTRY_SIZE, BMP4_SAMPLETOCHUNK_ENTRY_SIZE);
	if(!stream->cache) {
		goto err_cache;
	}
	stream->count = 0;

	if(!b_mp4_test_fullbox(temp_cache, 0, 0)) {
		goto err_fullbox;
	}
	buf = bfile_cache_next(temp_cache);
	if(!buf) {
		goto err_entry_count;
	}
	stream->entry_count = B_MP4_GET_UINT32(buf,0);
    bfile_cache_destroy(temp_cache);
    temp_cache = NULL;
	rc = bfile_cache_seek(stream->cache, BMP4_FULLBOX_SIZE+sizeof(uint32_t));
	if(rc!=0) {
		goto err_seek;
	}
	BDBG_MSG(("b_mp4_sampletochunk_stream_init:%#lx entry_count:%u", (unsigned long)stream, (unsigned)stream->entry_count));
	return 0;
err_seek:
err_entry_count:
err_fullbox:
	BDBG_ERR(("b_mp4_sampletochunk_stream: error parsing box"));
	bfile_cache_destroy(stream->cache);
err_cache:
    if(temp_cache) {
	    bfile_cache_destroy(temp_cache);
    }
err_temp_cache:
	return -1;
}

static void
b_mp4_sampletochunk_stream_shutdown(b_mp4_sampletochunk_stream *stream)
{
	bfile_cache_destroy(stream->cache);
	return;
}

static int
b_mp4_sampletochunk_stream_next(b_mp4_sampletochunk_stream *stream, bmp4_sampletochunk_entry *entry)
{
	BDBG_ASSERT(stream);
	BDBG_ASSERT(entry);
	BDBG_MSG_TRACE(("b_mp4_sampletochunk_stream_next:> %#lx count:%u entry:%u", (unsigned long)stream, stream->count, stream->entry_count));
	if(stream->count<stream->entry_count) {
		const uint8_t *buf = bfile_cache_next(stream->cache);
		stream->count++;
		if(buf) {
			entry->first_chunk = B_MP4_GET_UINT32(buf,0);
			entry->samples_per_chunk = B_MP4_GET_UINT32(buf,4);
			entry->sample_description_index = B_MP4_GET_UINT32(buf,8);
			BDBG_MSG(("b_mp4_sampletochunk_stream_next:%#lx first_chunk:%u samples_per_chunk:%u sample_description_index:%u", (unsigned long)stream, (unsigned)entry->first_chunk, (unsigned)entry->samples_per_chunk, (unsigned)entry->sample_description_index));
			return 0;
		}
	}
	return -1;
}

static int
b_mp4_sampletochunk_stream_seek(b_mp4_sampletochunk_stream *stream, unsigned chunk)
{
	if(chunk<=stream->entry_count) {
		int rc;
		size_t off = BMP4_FULLBOX_SIZE+sizeof(uint32_t)+chunk*BMP4_SAMPLETOCHUNK_ENTRY_SIZE;
		BDBG_MSG_TRACE(("b_mp4_sampletochunk_stream_seek: %#lx chunk:%u offset:%u", (unsigned long)stream, (unsigned)chunk, (unsigned)off));
		rc = bfile_cache_seek(stream->cache, off);
		if(rc!=0) { goto error; }
        stream->count = chunk;
	    return 0;
	}
error:
	return -1;
}

#define B_MP4_INVALID_CHUNKOFFSET ((off_t)(-1))
typedef struct b_mp4_chunkoffset_stream {
	bfile_cache_t  cache;
	uint32_t entry_count;
	uint32_t count; /* current count */
} b_mp4_chunkoffset_stream;

static int
b_mp4_chunkoffset_stream_init(b_mp4_chunkoffset_stream *stream, bfile_io_read_t fd, const bfile_segment *segment)
{
	const uint8_t *buf;
	BDBG_ASSERT(stream);
	BDBG_ASSERT(fd);
	BDBG_ASSERT(segment);
	stream->cache = bfile_cache_create(fd, segment, B_MP4_CACHE_CHUNKS*sizeof(uint32_t), sizeof(uint32_t));
	if(!stream->cache) {
		goto err_cache;
	}
	stream->count = 0;
	if(!b_mp4_test_fullbox(stream->cache, 0, 0)) {
		goto err_fullbox;
	}

	buf = bfile_cache_next(stream->cache);
	if(!buf) {
		goto err_entry_count;
	}
	stream->entry_count = B_MP4_GET_UINT32(buf,0);
	return 0;
err_entry_count:
err_fullbox:
	BDBG_ERR(("b_mp4_chunkoffset_stream: error parsing box"));
	bfile_cache_destroy(stream->cache);
err_cache:
	return -1;
}

static void
b_mp4_chunkoffset_stream_shutdown(b_mp4_chunkoffset_stream *stream)
{
	bfile_cache_destroy(stream->cache);
	return;
}

static off_t
b_mp4_chunkoffset_stream_next(b_mp4_chunkoffset_stream *stream)
{
	BDBG_ASSERT(stream);
	BDBG_MSG_TRACE(("b_mp4_chunkoffset_stream_next:%#lx count:%u entry_count:%u", (unsigned long)stream, (unsigned)stream->count, (unsigned)stream->entry_count));
	if(stream->count<stream->entry_count) {
		const uint8_t *buf = bfile_cache_next(stream->cache);
		stream->count++;
		if(buf) {
			uint32_t off = B_MP4_GET_UINT32(buf,0);
			BDBG_MSG(("b_mp4_chunkoffset_stream_next:%#lx offset:%u", (unsigned long)stream, (unsigned)off));
			return off;
		}
	}
	return B_MP4_INVALID_CHUNKOFFSET;
}


static int
b_mp4_chunkoffset_stream_seek(b_mp4_chunkoffset_stream *stream, unsigned chunk)
{
	if(chunk<=stream->entry_count) {
		int rc = bfile_cache_seek(stream->cache, BMP4_FULLBOX_SIZE+sizeof(uint32_t)+chunk*sizeof(uint32_t));
        if(rc!=0) { goto error;}
	    stream->count = chunk;
	    return 0;
	}
error:
	return -1;
}

typedef struct b_mp4_chunkoffset64_stream {
	bfile_cache_t  cache;
	uint32_t entry_count;
	uint32_t count; /* current count */
} b_mp4_chunkoffset64_stream;

static int
b_mp4_chunkoffset64_stream_init(b_mp4_chunkoffset64_stream *stream, bfile_io_read_t fd, const bfile_segment *segment)
{
	const uint8_t *buf;
	int rc;

	BDBG_ASSERT(stream);
	BDBG_ASSERT(fd);
	BDBG_ASSERT(segment);
	stream->cache = bfile_cache_create(fd, segment, B_MP4_CACHE_CHUNKS*sizeof(uint64_t), sizeof(uint64_t));
	if(!stream->cache) {
		goto err_cache;
	}
	stream->count = 0;
	if(!b_mp4_test_fullbox(stream->cache, 0, 0)) {
		goto err_fullbox;
	}

	buf = bfile_cache_next(stream->cache);
	if(!buf) {
		goto err_entry_count;
	}
	stream->entry_count = B_MP4_GET_UINT32(buf,0);
	rc = bfile_cache_seek(stream->cache, BMP4_FULLBOX_SIZE+sizeof(uint32_t));
	if(rc!=0) {
		goto err_seek;
	}
	return 0;
err_seek:
err_entry_count:
err_fullbox:
	BDBG_ERR(("b_mp4_chunkoffset64_stream: error parsing box"));
	bfile_cache_destroy(stream->cache);
err_cache:
	return -1;
}

static void
b_mp4_chunkoffset64_stream_shutdown(b_mp4_chunkoffset64_stream *stream)
{
	bfile_cache_destroy(stream->cache);
	return;
}

off_t
b_mp4_chunkoffset64_stream_next(b_mp4_chunkoffset64_stream *stream)
{
	BDBG_ASSERT(stream);
	BDBG_MSG_TRACE(("b_mp4_chunkoffset64_stream_next:%p count:%u entry_count:%u", (void *)stream, (unsigned)stream->count, (unsigned)stream->entry_count));
	if(stream->count<stream->entry_count) {
		const uint8_t *buf = bfile_cache_next(stream->cache);
		stream->count++;
        if(buf) {
            uint64_t off = B_MP4_GET_UINT64(buf,0);
            BDBG_MSG(("b_mp4_chunkoffset64_stream_next:%p offset:" B_OFFT_FMT "", (void *)stream, B_OFFT_ARG(off)));
            return off;
        }
	}
	return B_MP4_INVALID_CHUNKOFFSET;
}


static int
b_mp4_chunkoffset64_stream_seek(b_mp4_chunkoffset64_stream *stream, unsigned chunk)
{
	if(chunk<=stream->entry_count) {
		int rc = bfile_cache_seek(stream->cache, BMP4_FULLBOX_SIZE+sizeof(uint32_t)+chunk*sizeof(uint64_t));
		if(rc!=0) { goto error;}
        stream->count = chunk;
	    return 0;
	}
error:
	return -1;
}

typedef struct b_mp4_timedelta_stream {
	bfile_cache_t  cache;
	uint32_t entry_count;
	uint32_t count;
} b_mp4_timedelta_stream;


#define B_MP4_TIMETOSAMPLE_ENTRY_SIZE (sizeof(uint32_t)+sizeof(uint32_t))

typedef struct b_mp4_timedelta_entry {
	uint32_t sample_count;
	int32_t sample_delta;
} b_mp4_timedelta_entry;

static int
b_mp4_timedelta_stream_init(b_mp4_timedelta_stream *stream, bfile_io_read_t fd, const bfile_segment *segment)
{
	const uint8_t *buf;
	int rc;
    bfile_cache_t temp_cache;
	BDBG_ASSERT(stream);
	BDBG_ASSERT(fd);
	BDBG_ASSERT(segment);
	temp_cache = bfile_cache_create(fd, segment, 4, sizeof(uint32_t));
    if(!temp_cache) {
		goto err_temp_cache;
    }
	stream->cache = bfile_cache_create(fd, segment, B_MP4_CACHE_FRAMES*B_MP4_TIMETOSAMPLE_ENTRY_SIZE, B_MP4_TIMETOSAMPLE_ENTRY_SIZE);
	if(!stream->cache) {
		goto err_cache;
	}
	stream->count = 0;

	if(!b_mp4_test_fullbox(temp_cache, 0, 0)) {
		goto err_fullbox;
	}

	buf = bfile_cache_next(temp_cache);
	if(!buf) {
		goto err_entry_count;
	}
	stream->entry_count = B_MP4_GET_UINT32(buf,0);
    bfile_cache_destroy(temp_cache);
    temp_cache = NULL;
	rc = bfile_cache_seek(stream->cache, BMP4_FULLBOX_SIZE+sizeof(uint32_t));
	if(rc!=0) {
		goto err_seek;
	}
	BDBG_MSG(("b_mp4_timedelta_stream_init:%#lx entry_count:%u", (unsigned long)stream, (unsigned)stream->entry_count));
	return 0;
err_seek:
err_entry_count:
err_fullbox:
	BDBG_ERR(("b_mp4_timedelta_stream: error parsing box"));
	bfile_cache_destroy(stream->cache);
err_cache:
    if(temp_cache) {
	    bfile_cache_destroy(temp_cache);
    }
err_temp_cache:
	return -1;
}

static void
b_mp4_timedelta_stream_shutdown(b_mp4_timedelta_stream *stream)
{
	bfile_cache_destroy(stream->cache);
	return;
}

static int
b_mp4_timedelta_stream_next(b_mp4_timedelta_stream *stream, b_mp4_timedelta_entry *entry)
{
	BDBG_ASSERT(stream);
	BDBG_ASSERT(entry);
	if(stream->count < stream->entry_count) {
		const uint8_t *buf = bfile_cache_next(stream->cache);
		if(buf) {
			stream->count++;
			entry->sample_count = B_MP4_GET_UINT32(buf,0);
			entry->sample_delta = B_MP4_GET_UINT32(buf,4);
			BDBG_MSG(("b_mp4_timedelta_stream_next:%#lx sample_count:%u sample_delta:%d", (unsigned long)stream, (unsigned)entry->sample_count, (int)entry->sample_delta));
			return 0;
		}
	}
	return -1;
}

static int
b_mp4_timedelta_stream_seek(b_mp4_timedelta_stream *stream, unsigned entry)
{
	if(entry<=stream->entry_count) {
		int rc = bfile_cache_seek(stream->cache, BMP4_FULLBOX_SIZE+sizeof(uint32_t)+entry*B_MP4_TIMETOSAMPLE_ENTRY_SIZE);
		if(rc!=0) { goto error;}
        stream->count = entry;
	    return 0;
	}
error:
	return -1;
}

typedef struct b_mp4_syncsample_stream {
	bfile_cache_t  cache;
	uint32_t entry_count;
	uint32_t count;
} b_mp4_syncsample_stream;

static int
b_mp4_syncsample_stream_init(b_mp4_syncsample_stream *stream, bfile_io_read_t fd, const bfile_segment *segment)
{
	const uint8_t *buf;
	int rc;
	BDBG_ASSERT(stream);
	BDBG_ASSERT(fd);
	BDBG_ASSERT(segment);
	stream->cache = bfile_cache_create(fd, segment, B_MP4_CACHE_FRAMES*sizeof(uint32_t), sizeof(uint32_t));
	if(!stream->cache) {
		goto err_cache;
	}
	stream->count = 0;

	if(!b_mp4_test_fullbox(stream->cache, 0, 0)) {
		goto err_fullbox;
	}

	buf = bfile_cache_next(stream->cache);
	if(!buf) {
		goto err_entry_count;
	}
	stream->entry_count = B_MP4_GET_UINT32(buf,0);
	rc = bfile_cache_seek(stream->cache, BMP4_FULLBOX_SIZE+sizeof(uint32_t));
	if(rc!=0) {
		goto err_seek;
	}
	BDBG_MSG(("b_mp4_syncsample_stream_init:%#lx entry_count:%u", (unsigned long)stream, (unsigned)stream->entry_count));
	return 0;
err_seek:
err_entry_count:
err_fullbox:
	BDBG_ERR(("b_mp4_syncsample_stream: error parsing box"));
	bfile_cache_destroy(stream->cache);
err_cache:
	return -1;
}

static void
b_mp4_syncsample_stream_shutdown(b_mp4_syncsample_stream *stream)
{
	bfile_cache_destroy(stream->cache);
	return;
}

static int
b_mp4_syncsample_stream_next(b_mp4_syncsample_stream *stream, uint32_t *sample_number)
{
	BDBG_ASSERT(stream);
	BDBG_ASSERT(sample_number);
    BDBG_MSG_TRACE(("b_mp4_syncsample_stream_next:%#lx count:%u entry_count:%u", (unsigned long)stream, stream->count, stream->entry_count));
	if(stream->count < stream->entry_count) {
		const uint8_t *buf = bfile_cache_next(stream->cache);
		if(buf) {
			stream->count++;
			*sample_number = B_MP4_GET_UINT32(buf,0);
			BDBG_MSG(("b_mp4_syncsample_stream_next:%#lx sample_number:%u", (unsigned long)stream, (unsigned)*sample_number));
			return 0;
		}
	}
	return -1;
}

static int
b_mp4_syncsample_stream_seek(b_mp4_syncsample_stream *stream, unsigned entry)
{
	if(entry<=stream->entry_count) {
		int rc = bfile_cache_seek(stream->cache, BMP4_FULLBOX_SIZE+sizeof(uint32_t)+entry*sizeof(uint32_t));
		if(rc!=0) { goto error;}
        stream->count = entry;
	    return 0;
	}
error:
	return -1;
}

typedef struct b_mp4_edit_stream {
	bfile_cache_t  cache;
    unsigned version;
    uint32_t entry_count;
	uint32_t count; /* current count */
    uint64_t initial_offset;
} b_mp4_edit_stream;

static bool
b_mp4_edit_stream_load(b_mp4_edit_stream *stream, unsigned flags)
{
	const uint8_t *buf;
    unsigned box_flags;
    int rc;
    uint32_t size;
    uint32_t type;
    unsigned offset;
    uint64_t segment_duration;
    int64_t  media_time;

	buf = bfile_cache_next(stream->cache);
	if(!buf) {
        return false;
	}
    size = B_MP4_GET_UINT32(buf,0);
    type = B_MP4_GET_UINT32(buf,4);
    offset = 8;
    if(size==1) {
        uint64_t size64;
        buf = bfile_cache_next(stream->cache);
        if(!buf) {
            return false;
        }
        size64 = B_MP4_GET_UINT64(buf,0);
        if(size64 >= 1u<<31) {
            return false;
        }
        size = (uint32_t)size64;
        offset += 8;
    }
    if(type != BMP4_TYPE('e','l','s','t')) {
        return false;
    }
    buf = bfile_cache_next(stream->cache);
    if(buf==NULL) {
        return false;
    }
    stream->version = *buf;
    if(!(stream->version==0 || stream->version==1)) {
        BDBG_ERR(("%s: %p Not supported version %u", "b_mp4_edit_stream_load", (void *)stream, stream->version));
        return false;
    }
    box_flags = B_MP4_GET_UINT24(buf,1);
	if(box_flags!=flags) {
        BDBG_ERR(("%s: %p Not supported flags %#x", "b_mp4_edit_stream_load", (void *)stream, box_flags));
        return false;
    }
    stream->entry_count = B_MP4_GET_UINT32(buf, 4);
    offset +=8;
    if(stream->entry_count==0) {
        return true;
    }
    if(stream->entry_count>2) {
        BDBG_WRN(("%s: %p multiple entries(%u) in the EDIT box aren't supported", "b_mp4_edit_stream_load", (void *)stream, stream->entry_count));
    }
	rc = bfile_cache_seek(stream->cache, offset);
	if(rc!=0) {
		return false;
	}
    buf = bfile_cache_next(stream->cache);
    if(!buf) {
        return false;
    }
    if(stream->version == 0) {
        segment_duration = B_MP4_GET_UINT32(buf, 0);
        media_time = (int)B_MP4_GET_UINT32(buf, 4);
    } else {
        segment_duration = B_MP4_GET_UINT64(buf, 0);
        buf = bfile_cache_next(stream->cache);
        if(!buf) {
            return false;
        }
        media_time = B_MP4_GET_UINT64(buf, 0);
    }
    if(media_time==-1) {
        stream->initial_offset = segment_duration;
        BDBG_MSG(("%s: %p delaying stream by %u units", "b_mp4_edit_stream_load", (void *)stream, (int)segment_duration));
    } else {
        BDBG_WRN(("%s: %p not supported entry in the EDIT box media_time:%d:%u", "b_mp4_edit_stream_load", (void *)stream, (int)media_time, (unsigned)segment_duration));
    }

    return true;
}

static int
b_mp4_edit_stream_init(b_mp4_edit_stream *stream, bfile_io_read_t fd, const bfile_segment *segment)
{
	BDBG_ASSERT(stream);
	BDBG_ASSERT(fd);
	BDBG_ASSERT(segment);
	stream->cache = bfile_cache_create(fd, segment, B_MP4_CACHE_FRAMES*2*sizeof(uint32_t), 2*sizeof(uint32_t));
	if(!stream->cache) {
		goto err_cache;
	}
	stream->count = 0;
    stream->initial_offset = 0;
    if(!b_mp4_edit_stream_load(stream, 0)) {
		goto err_load;
    }
	BDBG_MSG(("b_mp4_edit_stream_init:%#lx entry_count:%u offset:%u", (unsigned long)stream, (unsigned)stream->entry_count, (unsigned)stream->initial_offset));
	return 0;
err_load:
	BDBG_ERR(("b_mp4_edit_stream: error parsing box"));
	bfile_cache_destroy(stream->cache);
err_cache:
	return -1;
}

static void
b_mp4_edit_stream_shutdown(b_mp4_edit_stream *stream)
{
	bfile_cache_destroy(stream->cache);
	return;
}

/* this structure maintains all state that is needed for _fast_ random access within MP4 track */
typedef struct b_mp4_stream_state {
	bool chunk_valid;
	bool decoding_time_valid;
	bool composition_time_valid;
	bool syncsample_valid;
	bool next_chunk_valid;

	int64_t decoding_time;
	uint32_t syncsample_number;
	uint32_t count;	
	uint32_t samples_in_chunk;
	off_t chunk_offset;
	b_mp4_timedelta_entry composition_time_entry;
	b_mp4_timedelta_entry decoding_time_entry;
	uint32_t composition_time_count;
	uint32_t decoding_time_count;
	uint32_t chunk_count;
	uint32_t syncsample_count;
	bmp4_sampletochunk_entry chunk_entry;
	bmp4_sampletochunk_entry next_chunk_entry;

	unsigned samplesize_off;
	unsigned chunkoffset_off;
	unsigned sampletochunk_off;
	unsigned decoding_time_off;
	unsigned composition_time_off;
	unsigned syncsample_off;
	bmp4_sample	next_sample;
} b_mp4_stream_state;

static void
b_mp4_stream_state_init(b_mp4_stream_state *state)
{
	state->count = 0;
	state->samples_in_chunk = 0;
	state->chunk_valid = false;
	state->chunk_offset = 0;
	state->next_chunk_valid = false;
	state->chunk_count = 0;
	state->decoding_time_valid = false;
	state->decoding_time_count = 0;
	state->decoding_time = 0;
	state->composition_time_valid = false;
	state->composition_time_count = 0;
	state->syncsample_valid = false;
	state->syncsample_count = 0;
	state->composition_time_entry.sample_delta = 0;

	state->samplesize_off=0;
	state->chunkoffset_off=0;
	state->sampletochunk_off=0;
	state->decoding_time_off=0;
	state->composition_time_off=0;
	state->syncsample_off=0;

	state->next_sample.offset = 0;
	state->next_sample.len = 0;
	state->next_sample.time = 0;
	state->next_sample.syncsample = false;
	
	return;
}


BDBG_OBJECT_ID(bmp4_stream_sample_t);
struct bmp4_stream_sample {
	BDBG_OBJECT(bmp4_stream_sample_t)
	bool has_composition_time;
	bool has_syncsample;
    bool has_edit;
    bool next_sample_valid;
	uint32_t timescale;
	uint32_t movie_timescale;
	bmedia_index_t index;
	bmedia_player_pos index_wait_time;
	b_mp4_stream_state state;
	enum {b_mp4_samplesize_type_32,b_mp4_samplesize_type_compact} samplesize_type;
	union {
		b_mp4_samplesize_stream sz_32;
		b_mp4_samplesize2_stream sz_compact;
	} samplesize;
	enum {b_mp4_chunkoffset_type64, b_mp4_chunkoffset_type32} chunkoffset_type;
	union {
		b_mp4_chunkoffset_stream co32;
		b_mp4_chunkoffset64_stream co64;
	} chunkoffset;
    bmp4_sample_sound_frame_info frame_info;
	b_mp4_sampletochunk_stream sampletochunk;
	b_mp4_timedelta_stream decoding_time;
	b_mp4_timedelta_stream composition_time;
	b_mp4_syncsample_stream syncsample;
    b_mp4_edit_stream edit;
};

bmp4_stream_sample_t
bmp4_stream_sample_create(bfile_io_read_t fd, const bmp4_track_info *track)
{
	int rc;
	bmp4_stream_sample_t stream;
	const bmp4_sampleentry *entry;

	BDBG_ASSERT(fd);
	BDBG_ASSERT(track);

	if(track->mediaheader.timescale==0) {
		BDBG_ERR(("bmp4_stream_sample_t: invalid  timescale %u", (unsigned)track->mediaheader.timescale));
		goto err_invalid_timescale;
	}
	stream = BKNI_Malloc(sizeof(*stream));
	if(!stream) {
		BDBG_ERR(("bmp4_stream_sample_create: can't allocate %u bytes", (unsigned)sizeof(*stream)));
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(stream, bmp4_stream_sample_t);
    stream->timescale = track->mediaheader.timescale;
	stream->has_composition_time = false;
	stream->has_syncsample = false;
    stream->has_edit = false;
	stream->next_sample_valid = false;
    if(track->movieheader) {
        stream->movie_timescale = track->movieheader->timescale;
    } else {
        stream->movie_timescale = track->mediaheader.timescale;
    }
    stream->frame_info.samplesPerPacket = 0;
    stream->frame_info.bytesPerFrame = 0;
    entry = track->sample_info.entries[0];
    switch(entry->sample_type) {
    case bmp4_sample_type_twos:
        if(entry->codec.twos.audio.version==1) { /* frame info is stored in file */
            stream->frame_info = entry->codec.twos.frame_info;
        }
        break;
    case bmp4_sample_type_qt_ima4_adpcm:
        stream->frame_info = entry->codec.ima4.frame_info;
        break;
    default:
        break;
    }

	b_mp4_stream_state_init(&stream->state);
	stream->index = bmedia_index_create( (bmedia_player_pos)((1000*track->mediaheader.duration)/track->mediaheader.timescale), sizeof(stream->state), 100);
	if(!stream->index) {
		goto err_index;
	}
	stream->index_wait_time = bmedia_index_next(stream->index);

	if(bfile_segment_test(&track->samplesize)) {
		rc = b_mp4_samplesize_stream_init(&stream->samplesize.sz_32, fd, &track->samplesize);
		if(rc<0) {
			goto err_samplesize;
		}
		stream->samplesize_type = b_mp4_samplesize_type_32;
	} else if(bfile_segment_test(&track->usamplesize)) {
		rc = b_mp4_samplesize2_stream_init(&stream->samplesize.sz_compact, fd, &track->usamplesize);
		if(rc<0) {
			goto err_samplesize;
		}
		stream->samplesize_type = b_mp4_samplesize_type_compact;
	} else {
		BDBG_WRN(("bmp4_stream_sample_create: %#lx track:%u without samplesize box", (unsigned long)fd, (unsigned)track->trackheader.track_ID));
		goto err_samplesize;
	}

	if(bfile_segment_test(&track->chunkoffset64)) {
		rc = b_mp4_chunkoffset64_stream_init(&stream->chunkoffset.co64, fd, &track->chunkoffset64);
		if(rc<0) {
			goto err_chunkoffset;
		}
		stream->chunkoffset_type = b_mp4_chunkoffset_type64;
	} else if(bfile_segment_test(&track->chunkoffset)) {
		rc = b_mp4_chunkoffset_stream_init(&stream->chunkoffset.co32, fd, &track->chunkoffset);
		if(rc<0) {
			goto err_chunkoffset;
		}
		stream->chunkoffset_type = b_mp4_chunkoffset_type32;
	} else {
		BDBG_WRN(("bmp4_stream_sample_create: %#lx track:%u without chunkoffset box", (unsigned long)fd, (unsigned)track->trackheader.track_ID));
		goto err_chunkoffset;
	}

	if(bfile_segment_test(&track->sampletochunk)) {
		rc = b_mp4_sampletochunk_stream_init(&stream->sampletochunk, fd, &track->sampletochunk);
		if(rc<0) {
			goto err_sampletochunk;
		}
	} else {
		BDBG_WRN(("bmp4_stream_sample_create: %#lx track:%u without sampletochunk box", (unsigned long)fd, (unsigned)track->trackheader.track_ID));
		goto err_sampletochunk;
	}
	if(bfile_segment_test(&track->decode_t2s)) {
		rc = b_mp4_timedelta_stream_init(&stream->decoding_time, fd, &track->decode_t2s);
		if(rc<0) {
			goto err_decoding_time;
		}
	} else {
		BDBG_WRN(("bmp4_stream_sample_create: %#lx track:%u without decoding_time box", (unsigned long)fd, (unsigned)track->trackheader.track_ID));
		goto err_decoding_time;
	}
	if(bfile_segment_test(&track->composition_t2s)) {
		rc = b_mp4_timedelta_stream_init(&stream->composition_time, fd, &track->composition_t2s);
		if(rc<0) {
			goto err_composition_time;
		}
		stream->has_composition_time = true;
	} 
	if(bfile_segment_test(&track->syncsample)) {
		rc = b_mp4_syncsample_stream_init(&stream->syncsample, fd, &track->syncsample);
		if(rc<0) {
			goto err_syncsample;
		}
        if(stream->syncsample.entry_count==0) {
		    b_mp4_syncsample_stream_shutdown(&stream->syncsample);
        } else {
		    stream->has_syncsample = true;
        }
	}
    if(bfile_segment_test(&track->edit)) {
   		rc = b_mp4_edit_stream_init(&stream->edit, fd, &track->edit);
		if(rc<0) {
			goto err_edit;
		}
		stream->has_edit = true;
    }

	return stream;

err_edit:
    if(stream->has_syncsample) {
		b_mp4_syncsample_stream_shutdown(&stream->syncsample);
    }
err_syncsample:
	if(stream->has_composition_time) {
		b_mp4_timedelta_stream_shutdown(&stream->composition_time);
	}
err_composition_time:
	b_mp4_timedelta_stream_shutdown(&stream->decoding_time);
err_decoding_time:
	b_mp4_sampletochunk_stream_shutdown(&stream->sampletochunk);
err_sampletochunk:
	if(stream->chunkoffset_type == b_mp4_chunkoffset_type64) {
		b_mp4_chunkoffset64_stream_shutdown(&stream->chunkoffset.co64);
	} else {
		b_mp4_chunkoffset_stream_shutdown(&stream->chunkoffset.co32);
	}
err_chunkoffset:
	if(stream->samplesize_type == b_mp4_samplesize_type_32) {
		b_mp4_samplesize_stream_shutdown(&stream->samplesize.sz_32);
	} else {
		b_mp4_samplesize2_stream_shutdown(&stream->samplesize.sz_compact);
	}
err_samplesize:
	bmedia_index_destroy(stream->index);
err_index:
	BKNI_Free(stream);
err_alloc:
err_invalid_timescale:
	BDBG_WRN(("bmp4_stream_sample_create: %#lx invalid track:%u", (unsigned long)fd, (unsigned)track->trackheader.track_ID));
	return NULL;
}

void 
bmp4_stream_sample_destroy(bmp4_stream_sample_t stream)
{
	BDBG_OBJECT_ASSERT(stream, bmp4_stream_sample_t);
    if(stream->has_edit) {
        b_mp4_edit_stream_shutdown(&stream->edit);
    }
	if(stream->has_syncsample) {
		b_mp4_syncsample_stream_shutdown(&stream->syncsample);
	}
	if(stream->has_composition_time) {
		b_mp4_timedelta_stream_shutdown(&stream->composition_time);
	}
	b_mp4_timedelta_stream_shutdown(&stream->decoding_time);
	b_mp4_sampletochunk_stream_shutdown(&stream->sampletochunk);
	if(stream->chunkoffset_type == b_mp4_chunkoffset_type64) {
		b_mp4_chunkoffset64_stream_shutdown(&stream->chunkoffset.co64);
	} else {
		b_mp4_chunkoffset_stream_shutdown(&stream->chunkoffset.co32);
	}
	if(stream->samplesize_type == b_mp4_samplesize_type_32) {
		b_mp4_samplesize_stream_shutdown(&stream->samplesize.sz_32);
	} else {
		b_mp4_samplesize2_stream_shutdown(&stream->samplesize.sz_compact);
	}
	bmedia_index_destroy(stream->index);
	BDBG_OBJECT_DESTROY(stream, bmp4_stream_sample_t);
	BKNI_Free(stream);
	return;
}

static int
b_mp4_stream_sample_next(bmp4_stream_sample_t stream, bmp4_sample *sample, size_t max_sample_count)
{
	BDBG_OBJECT_ASSERT(stream, bmp4_stream_sample_t);
    BKNI_Memset(sample, 0, sizeof(*sample)); 
    sample->endofstream = false;
	for(;;) {
		int rc;
        bool file_error = false;

		BDBG_MSG_TRACE(("b_mp4_stream_sample_next: %#lx chunk_offset:%u chunk_valid:%u next_chunk_valid:%u samples_in_chunk:%u", (unsigned long)stream, (unsigned)stream->state.chunk_offset, (unsigned)stream->state.chunk_valid, (unsigned)stream->state.next_chunk_valid, (unsigned)stream->state.samples_in_chunk));
		if(stream->state.chunk_valid && stream->state.samples_in_chunk>0) {
			size_t len;
			bmedia_player_pos time;

			if(stream->has_composition_time) {
				for(;;) {
					if(stream->state.composition_time_valid && stream->state.composition_time_entry.sample_count>0) {
						stream->state.composition_time_entry.sample_count--;
						break;
					}
					rc = b_mp4_timedelta_stream_next(&stream->composition_time, &stream->state.composition_time_entry);
					if(rc<0) {
						BDBG_ERR(("b_mp4_stream_sample_next: %#lx can't read composition_time (%u)", (unsigned long)stream, (unsigned)stream->state.composition_time_count));
						return -1;
					}
					stream->state.composition_time_off++;
					stream->state.composition_time_valid = true;
					stream->state.composition_time_count++;
				}
			} 
			for(;;) {
				if(stream->state.decoding_time_valid && stream->state.decoding_time_entry.sample_count>0) {
                    int64_t mp4_time;
					stream->state.decoding_time_entry.sample_count--;
					mp4_time = ((stream->state.decoding_time+stream->state.composition_time_entry.sample_delta) * 1000)/stream->timescale;
                    time = mp4_time>=0 ? (bmedia_player_pos) mp4_time : 0; /* bmedia_player_pos can't be negative */
                    time = mp4_time <= (int64_t)time ? time :  BMEDIA_PLAYER_INVALID; /* bmedia_player_pos shouldn't get truncated */
		            BDBG_MSG_TRACE(("b_mp4_stream_sample_next: %#lx decoding_time:%u composition_sample_delta:%d decoding_sample_delta:%d timescale:%u time:%u", (unsigned long)stream, (unsigned)stream->state.decoding_time, (int)stream->state.composition_time_entry.sample_delta, (int)stream->state.decoding_time_entry.sample_delta, (unsigned)stream->timescale, (unsigned)time));
					stream->state.decoding_time += stream->state.decoding_time_entry.sample_delta;
					break;
				}
				rc = b_mp4_timedelta_stream_next(&stream->decoding_time, &stream->state.decoding_time_entry);
				if(rc<0) {
					BDBG_ERR(("b_mp4_stream_sample_next: %#lx can't read decoding_time (%u)", (unsigned long)stream, (unsigned)stream->state.decoding_time_count));
					return -1;
				}
				stream->state.decoding_time_off++;
				stream->state.decoding_time_valid = true;
				stream->state.decoding_time_count++;
			}
			if(!stream->has_syncsample) {
				sample->syncsample = true;
			} else {
				for(;;) {
					if(!stream->state.syncsample_valid || stream->state.count >= stream->state.syncsample_number) { /* syncsample_number starts from 1 */
						rc = b_mp4_syncsample_stream_next(&stream->syncsample, &stream->state.syncsample_number);
						if(rc<0) {
							BDBG_MSG(("b_mp4_stream_sample_next: %#lx can't read syncsample(%u)", (unsigned long)stream, (unsigned)stream->state.syncsample_count));
							stream->state.syncsample_valid = false;
							break;
						}
						stream->state.syncsample_off++;
						stream->state.syncsample_valid = true;
						continue;
					}
					break;
				}
				sample->syncsample = stream->state.syncsample_valid && (stream->state.syncsample_number == (stream->state.count+1));
			}
            if(stream->frame_info.samplesPerPacket==0 || stream->frame_info.bytesPerFrame==0) {
                if(stream->samplesize_type == b_mp4_samplesize_type_32) {
                    len = b_mp4_samplesize_stream_next(&stream->samplesize.sz_32);
                } else {
                    len = b_mp4_samplesize2_stream_next(&stream->samplesize.sz_compact);
                }
                if(len!=B_MP4_INVALID_SIZE) {
                    stream->state.samplesize_off++;
                    stream->state.count++;
                    stream->state.samples_in_chunk--;
                    sample->len = len;
                    sample->time = time;
                    sample->sample_count = 1;
                    if(stream->has_edit) {
                        sample->time += (stream->edit.initial_offset*1000)/stream->movie_timescale;
                    }
                    sample->offset = stream->state.chunk_offset;
                    stream->state.chunk_offset += len;
                }
            } else {
                /* we must read samplesPerPacket at a time, and here we expect that single packet would _not_ span over the chunk boundary */
                BDBG_ASSERT(stream->state.samples_in_chunk>0);
                if(stream->state.samples_in_chunk<stream->frame_info.samplesPerPacket) {
                    BDBG_ERR(("b_mp4_stream_sample_next:%#lx not alligned chunk (%u:%u)", (unsigned long)stream, stream->state.samples_in_chunk, stream->frame_info.samplesPerPacket));
                }
                for(len=0;len<stream->frame_info.samplesPerPacket;) {
                    size_t sample_len;
                    if(stream->samplesize_type == b_mp4_samplesize_type_32) {
                        sample_len = b_mp4_samplesize_stream_next(&stream->samplesize.sz_32);
                    } else {
                        sample_len = b_mp4_samplesize2_stream_next(&stream->samplesize.sz_compact);
                    }
                    BDBG_MSG_TRACE(("b_mp4_stream_sample_next:%p loading frame (%u:%u)->%u", (void *)stream, len, stream->frame_info.samplesPerPacket, sample_len));
			        if(sample_len==B_MP4_INVALID_SIZE) {
                        if(len!=0) {
                            BDBG_ERR(("b_mp4_stream_sample_next:%p not alligned frame (%u:%u)", (void *)stream, (unsigned)len, stream->frame_info.samplesPerPacket));
                        }
                        len = B_MP4_INVALID_SIZE;
                        break;
                    }
                    stream->state.samples_in_chunk --;
                    if(len) {
                        stream->state.decoding_time_entry.sample_count --;
					    stream->state.decoding_time += stream->state.decoding_time_entry.sample_delta;
                    }
                    if(len+sample_len>stream->frame_info.samplesPerPacket) {
                        BDBG_ERR(("b_mp4_stream_sample_next:%p not alligned sample %u+%u(%u:%u)", (void *)stream, (unsigned)len, (unsigned)sample_len, (unsigned)(len+sample_len), stream->frame_info.samplesPerPacket));
                    }
                    len+=sample_len;
                }
			    if(len!=B_MP4_INVALID_SIZE) {
                    sample->sample_count = len;
                    sample->len = stream->frame_info.bytesPerFrame;
                    sample->offset = stream->state.chunk_offset;
                    stream->state.chunk_offset += stream->frame_info.bytesPerFrame;
                    sample->time = time;
                }
            }
			if(len!=B_MP4_INVALID_SIZE) {
				if(stream->index_wait_time != BMEDIA_PLAYER_INVALID && time >= stream->index_wait_time) {
					BDBG_MSG(("b_mp4_stream_sample_next:%#lx saving index (%u:%u) chunk:%u", (unsigned long)stream, (unsigned)time, (unsigned)stream->index_wait_time, (unsigned)stream->state.count));
					stream->state.next_sample = *sample;
					bmedia_index_add(stream->index, time, &stream->state);
					stream->index_wait_time = bmedia_index_next(stream->index);
				}
			    if(!stream->has_syncsample && !stream->has_composition_time && max_sample_count>1) {
                    unsigned min_samples = max_sample_count-1;
                    if(min_samples > stream->state.decoding_time_entry.sample_count) {
                        min_samples = stream->state.decoding_time_entry.sample_count;
                    } 
                    if(min_samples > stream->state.samples_in_chunk) {
                        min_samples = stream->state.samples_in_chunk;
                    }
                    BDBG_MSG_TRACE(("decoding_time_entry.sample_count:%u stream->state.samples_in_chunk:%u min_samples:%u", stream->state.decoding_time_entry.sample_count,  stream->state.samples_in_chunk, min_samples));

                    if(stream->frame_info.samplesPerPacket==0 || stream->frame_info.bytesPerFrame==0) {
                        unsigned i;
                        for(i=0;i<min_samples;i++) {
                            stream->state.decoding_time_entry.sample_count --;
                            stream->state.samples_in_chunk --;
                            stream->state.decoding_time += stream->state.decoding_time_entry.sample_delta;
                            if(stream->samplesize_type == b_mp4_samplesize_type_32) {
                                len = b_mp4_samplesize_stream_next(&stream->samplesize.sz_32);
                            } else {
                                len = b_mp4_samplesize2_stream_next(&stream->samplesize.sz_compact);
                            }
                            if(len==B_MP4_INVALID_SIZE) {
                                break;
                            }
                            /* len *= stream->size_scale; */
                            sample->sample_count ++;
                            sample->len += len;
                            stream->state.chunk_offset += len;
                        }
                    } else if(min_samples>0) {
                        /* we must read samplesPerPacket at a time, and here we expect that single packet would _not_ span over the chunk boundary */
                        BDBG_ASSERT(stream->state.samples_in_chunk>0);
                        if(stream->state.samples_in_chunk<stream->frame_info.samplesPerPacket) {
                            BDBG_ERR(("b_mp4_stream_sample_next:%#lx not alligned chunk (%u:%u)", (unsigned long)stream, stream->state.samples_in_chunk, stream->frame_info.samplesPerPacket));
                        }
                        while(sample->sample_count<min_samples) {
                            BDBG_MSG_TRACE(("b_mp4_stream_sample_next:%#lx filing samples (%u:%u)", (unsigned long)stream, sample->sample_count,min_samples));
                            for(len=0;len<stream->frame_info.samplesPerPacket;) {
                                size_t sample_len;
                                stream->state.decoding_time_entry.sample_count --;
                                stream->state.samples_in_chunk --;
                                stream->state.decoding_time += stream->state.decoding_time_entry.sample_delta;
                                if(stream->samplesize_type == b_mp4_samplesize_type_32) {
                                    sample_len = b_mp4_samplesize_stream_next(&stream->samplesize.sz_32);
                                } else {
                                    sample_len = b_mp4_samplesize2_stream_next(&stream->samplesize.sz_compact);
                                }
                                BDBG_MSG_TRACE(("b_mp4_stream_sample_next:%#lx loading frame (%u:%u)->%u", (unsigned long)stream, len, stream->frame_info.samplesPerPacket, sample_len));
                                if(sample_len==B_MP4_INVALID_SIZE) {
                                    if(len!=0) {
                                        BDBG_ERR(("b_mp4_stream_sample_next:%p not alligned frame (%u:%u)", (void *)stream, (unsigned)len, stream->frame_info.samplesPerPacket));
                                    }
                                    len = B_MP4_INVALID_SIZE;
                                    break;
                                }
                                if(len+sample_len>stream->frame_info.samplesPerPacket) {
                                    BDBG_ERR(("b_mp4_stream_sample_next:%p not alligned sample %u+%u(%u:%u)", (void *)stream, (unsigned)len, (unsigned)sample_len, (unsigned)(len+sample_len), stream->frame_info.samplesPerPacket));
                                }
                                len+=sample_len;
                            }
                            if(len!=B_MP4_INVALID_SIZE) {
                                sample->sample_count += len;
                                sample->len += stream->frame_info.bytesPerFrame;
                                stream->state.chunk_offset += stream->frame_info.bytesPerFrame;
                            }
                        }
                    }
                }
                BDBG_MSG(("b_mp4_stream_sample_next:%p offset:" B_OFFT_FMT " len:%u(%u) time:%u sample_count:%u(%u) %s", (void *)stream, B_OFFT_ARG(sample->offset), (unsigned)sample->len, (unsigned)stream->frame_info.bytesPerFrame, (unsigned)sample->time, sample->sample_count, (unsigned)stream->frame_info.samplesPerPacket, sample->syncsample?"syncsample":""));
                return 0;
			}
			return -1;
		}
		if(stream->chunkoffset_type == b_mp4_chunkoffset_type32) {
			rc = b_mp4_chunkoffset_stream_seek(&stream->chunkoffset.co32, stream->state.chunk_count);
		} else {
			rc = b_mp4_chunkoffset64_stream_seek(&stream->chunkoffset.co64, stream->state.chunk_count);
		}
		if(rc<0) {
			BDBG_MSG(("b_mp4_stream_sample_next: %#lx EOF reached  %u", (unsigned long)stream, (unsigned)stream->state.chunk_count));
            sample->endofstream = true;
			return 0;
		}
		if(stream->chunkoffset_type == b_mp4_chunkoffset_type32) {
			stream->state.chunk_offset = b_mp4_chunkoffset_stream_next(&stream->chunkoffset.co32);
            file_error = bfile_cache_is_file_error(stream->chunkoffset.co32.cache);
		} else {
			stream->state.chunk_offset = b_mp4_chunkoffset64_stream_next(&stream->chunkoffset.co64);
            file_error = bfile_cache_is_file_error(stream->chunkoffset.co64.cache);
		}
		
		if(stream->state.chunk_offset==B_MP4_INVALID_CHUNKOFFSET) {
			BDBG_MSG(("b_mp4_stream_sample_next: %#lx can't read chunkoffset %u (EOF)", (unsigned long)stream, (unsigned)stream->state.chunk_count));
            sample->endofstream = true;
			return file_error?-1:0;
		}
		stream->state.chunkoffset_off++;
		stream->state.chunk_count++;
		if(stream->state.next_chunk_valid && stream->state.chunk_count < stream->state.next_chunk_entry.first_chunk) {
			/* reuse current SampleToChunkBox entry */
			stream->state.samples_in_chunk = stream->state.chunk_entry.samples_per_chunk;
			BDBG_MSG_TRACE(("b_mp4_stream_sample_next: %#lx samples_in_chunk:%u from same chunk_count:%u first_chunk:%u", (unsigned long)stream, (unsigned)stream->state.samples_in_chunk, (unsigned)stream->state.chunk_count, (unsigned)stream->state.next_chunk_entry.first_chunk));
			continue;
        } else if (stream->state.next_chunk_valid && stream->state.chunk_count > stream->state.next_chunk_entry.first_chunk) {
            BDBG_WRN(("b_mp4_stream_sample_next: %#lx found entry start:%u before currnet chunk:%u. Skipping entry", (unsigned long)stream, (unsigned)stream->state.next_chunk_entry.first_chunk, (unsigned)stream->state.chunk_count));
			stream->state.next_chunk_valid = false;
        }
		/* need to load new SampleToChunkBox entry */
		if(stream->state.next_chunk_valid) {
			stream->state.chunk_entry = stream->state.next_chunk_entry;
			stream->state.next_chunk_valid = false;
		} else {
			rc = b_mp4_sampletochunk_stream_next(&stream->sampletochunk, &stream->state.chunk_entry);
			if(rc==0) {
				stream->state.sampletochunk_off++;
				stream->state.chunk_valid = true;
			} else {
				if(stream->state.chunk_valid) {
					/* if chunk offset have returned valid data, then there are still chunks left, use current SampleToChunkBox */
					stream->state.samples_in_chunk = stream->state.chunk_entry.samples_per_chunk;
					BDBG_MSG_TRACE(("b_mp4_stream_sample_next: %#lx samples_in_chunk:%u from old", (unsigned long)stream, (unsigned)stream->state.samples_in_chunk));
					continue;
				}
				BDBG_WRN(("b_mp4_stream_sample_next: %#lx end of SampleToChunkBox", (unsigned long)stream));
				return -1;
			} 
		}
		stream->state.samples_in_chunk = stream->state.chunk_entry.samples_per_chunk;

        if( !(stream->frame_info.samplesPerPacket==0 || stream->frame_info.bytesPerFrame==0)  &&  (stream->state.samples_in_chunk % stream->frame_info.samplesPerPacket)!=0) {
            BDBG_WRN(("b_mp4_stream_sample_next:%#lx not alligned chunk (%u:%u)", (unsigned long)stream, stream->state.samples_in_chunk, stream->frame_info.samplesPerPacket));
        }
		BDBG_MSG_TRACE(("b_mp4_stream_sample_next: %#lx samples_in_chunk:%u from current samplesPerPacket:%u", (unsigned long)stream, (unsigned)stream->state.samples_in_chunk, (unsigned)stream->frame_info.samplesPerPacket));
		rc = b_mp4_sampletochunk_stream_next(&stream->sampletochunk, &stream->state.next_chunk_entry);
		if(rc==0) {
			stream->state.sampletochunk_off++;
			stream->state.next_chunk_valid = true;
		} else {
			stream->state.next_chunk_valid = false;
		}
	}
}

int
bmp4_stream_sample_next(bmp4_stream_sample_t stream, bmp4_sample *sample, size_t max_sample_count)
{
	int rc;

	if(!stream->next_sample_valid) {
		rc = b_mp4_stream_sample_next(stream, sample, max_sample_count);
	} else {
		stream->next_sample_valid = false;
		*sample = stream->state.next_sample;
		rc=0;
	}
    BDBG_MSG(("bmp4_stream_sample_next:%p offset:" B_OFFT_FMT " len:%u time:%u %s", (void *)stream, B_OFFT_ARG(sample->offset), (unsigned)sample->len, (unsigned)sample->time, sample->syncsample?"syncsample":""));
    return rc;
}

#if 0
/* unused function */
bmedia_player_pos 
bmp4_stream_sample_tell(bmp4_stream_sample_t stream)
{
	BDBG_OBJECT_ASSERT(stream, bmp4_stream_sample_t);
	if(stream->state.decoding_time_valid && (!stream->state.composition_time_valid || stream->has_composition_time)) {
		bmedia_player_pos time;
        uint64_t offset = 0;
        if(stream->state.decoding_time==0 && stream->has_edit) {
            offset = stream->edit.initial_offset;
        }
		time = (bmedia_player_pos)(((stream->state.decoding_time+offset+stream->state.composition_time_entry.sample_delta) * 1000)/stream->timescale);
		return time;
	}
	return BMEDIA_PLAYER_INVALID;
}
#endif

bmedia_player_pos 
bmp4_stream_sample_seek(bmp4_stream_sample_t stream, bmedia_player_pos pos, bool *endofstream)
{
	int rc;
	bmedia_player_pos time;
	bmp4_sample sample;

	BDBG_MSG_TRACE(("bmp4_stream_sample_seek:> %#lx pos:%u", (unsigned long)stream, (unsigned) pos));

	BDBG_OBJECT_ASSERT(stream, bmp4_stream_sample_t);
    *endofstream = false;
	time = bmedia_index_get(stream->index, pos, &stream->state);
	if(time==BMEDIA_PLAYER_INVALID) {
		b_mp4_stream_state_init(&stream->state);
	}
	if(stream->has_syncsample) {
		rc = b_mp4_syncsample_stream_seek(&stream->syncsample, stream->state.syncsample_off);
		if(rc<0) {
			BDBG_WRN(("bmp4_stream_sample_seek: %#lx can't seek syncsample to %u", (unsigned long)stream, (unsigned)stream->state.syncsample_off));
			goto error;
		}
	}
	if(stream->has_composition_time) {
		rc = b_mp4_timedelta_stream_seek(&stream->composition_time, stream->state.composition_time_off);
		if(rc<0) {
			BDBG_WRN(("bmp4_stream_sample_seek: %#lx can't seek composition_time to %u", (unsigned long)stream, (unsigned)stream->state.composition_time_off));
			goto error;
		}
	}
	rc = b_mp4_timedelta_stream_seek(&stream->decoding_time, stream->state.decoding_time_off);
	if(rc<0) {
		BDBG_WRN(("bmp4_stream_sample_seek: %#lx can't seek decoding_time to %u", (unsigned long)stream, (unsigned)stream->state.decoding_time_off));
		goto error;
	}
	rc = b_mp4_sampletochunk_stream_seek(&stream->sampletochunk, stream->state.sampletochunk_off);
	if(rc<0) {
		BDBG_WRN(("bmp4_stream_sample_seek: %#lx can't seek sampletochunk to %u", (unsigned long)stream, (unsigned)stream->state.sampletochunk_off));
		goto error;
	}
	if(stream->chunkoffset_type == b_mp4_chunkoffset_type32) {
		rc = b_mp4_chunkoffset_stream_seek(&stream->chunkoffset.co32, stream->state.chunk_count);
	} else {
		rc = b_mp4_chunkoffset64_stream_seek(&stream->chunkoffset.co64, stream->state.chunk_count);
	}
	if(rc<0) {
		BDBG_WRN(("bmp4_stream_sample_seek: %#lx can't seek chunkoffset to %u", (unsigned long)stream, (unsigned)stream->state.chunkoffset_off));
		goto error;
	}
	if(stream->samplesize_type == b_mp4_samplesize_type_32) {
		rc = b_mp4_samplesize_stream_seek(&stream->samplesize.sz_32, stream->state.samplesize_off);
	} else {
		rc = b_mp4_samplesize2_stream_seek(&stream->samplesize.sz_compact, stream->state.samplesize_off);
	}
	if(rc<0) {
		BDBG_WRN(("bmp4_stream_sample_seek: %#lx can't seek samplesize to %u", (unsigned long)stream, (unsigned)stream->state.samplesize_off));
		goto error;
	}
	for(time=BMEDIA_PLAYER_INVALID,sample=stream->state.next_sample;;) {
		BDBG_MSG_TRACE(("bmp4_stream_sample_seek: %#lx %u:%u chunk:%u len:%u %s", (unsigned long)stream, (unsigned)pos, (unsigned)sample.time, (unsigned)stream->state.chunk_count, (unsigned)sample.len, sample.syncsample?"SYNC":""));
		if(sample.syncsample && sample.time>=pos) {
			time = sample.time;
			break;
		}
		rc = b_mp4_stream_sample_next(stream, &sample, 1);
		if(rc<0) {
			goto error;
        }
        if(sample.endofstream ) {
            *endofstream = true; 
            break;
		}
	}
	stream->next_sample_valid = true;
	stream->state.next_sample = sample;
	BDBG_MSG_TRACE(("bmp4_stream_sample_seek:< %#lx pos:%u time:%u", (unsigned long)stream, (unsigned)pos, (unsigned)time));
	return time;

error:
	BDBG_MSG_TRACE(("bmp4_stream_sample_seek:> %#lx pos:%u ERROR", (unsigned long)stream, (unsigned)pos));
	return BMEDIA_PLAYER_INVALID;
}

void 
bmp4_stream_sample_get_status(bmp4_stream_sample_t stream, bmp4_stream_sample_status *status)
{
    uint32_t samplesize_count;
    uint32_t sampletochunk_count;
    uint32_t chunkoffset_count;
    unsigned sample_count;

	BDBG_MSG_TRACE(("bmp4_stream_sample_get_status:> %#lx", (unsigned long)stream));
	BDBG_OBJECT_ASSERT(stream, bmp4_stream_sample_t);
    switch(stream->samplesize_type) {
    case b_mp4_samplesize_type_32:
        samplesize_count = stream->samplesize.sz_32.sample_count;
        break;
    case b_mp4_samplesize_type_compact:
        samplesize_count = stream->samplesize.sz_compact.sample_count;
        break;
    default:
        samplesize_count = 0;
        break;
    }
    switch(stream->chunkoffset_type) {
	case b_mp4_chunkoffset_type64:
        chunkoffset_count = stream->chunkoffset.co32.entry_count;
        break;
    case b_mp4_chunkoffset_type32:
        chunkoffset_count = stream->chunkoffset.co64.entry_count;
        break;
    default:
        chunkoffset_count = 0;
        break;
    }
    sampletochunk_count = stream->sampletochunk.entry_count;
    sample_count = sampletochunk_count<chunkoffset_count ? sampletochunk_count:chunkoffset_count;
    sample_count = samplesize_count <sample_count ? samplesize_count:sample_count;
    status->sample_count = sample_count;
	BDBG_MSG_TRACE(("bmp4_stream_sample_get_status:< %#lx sample_count:%u", (unsigned long)stream, status->sample_count));
    return;
}

