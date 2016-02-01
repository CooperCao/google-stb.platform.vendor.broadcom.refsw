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
 * BMedia library, stream probe module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bmedia_cdxa.h"
#include "bkni.h"
#include "bmedia_util.h"
#include "bfile_buffer.h"
#include "bfile_cache.h"

BDBG_MODULE(bmedia_cdxa);

typedef struct bcdxa_probe  *bcdxa_probe_t; 
#define B_CDXA_HEADER_SIZE   44
#define B_CDXA_SECTOR_SIZE   2352
#define B_CDXA_SECTOR_HEADER (16+8)
#define B_CDXA_SECTOR_TRAILER   4
#define B_CDXA_SECTOR_PAYLOAD (B_CDXA_SECTOR_SIZE-(B_CDXA_SECTOR_HEADER+B_CDXA_SECTOR_TRAILER))



struct bcdxa_probe {
	BDBG_OBJECT(bcdxa_probe_t)
    unsigned unused;
};

BDBG_OBJECT_ID(bcdxa_probe_t);

static bool 
b_cdxa_probe_header_match(batom_cursor *header)
{
    uint32_t data;

    data = batom_cursor_uint32_le(header);
    if(BATOM_IS_EOF(header) || data!=BMEDIA_FOURCC('R','I','F','F')) {
        return false;
    }
    batom_cursor_skip(header, sizeof(uint32_t));
    data = batom_cursor_uint32_le(header);
    if(BATOM_IS_EOF(header) || data!=BMEDIA_FOURCC('C','D','X','A')) {
        return false;
    }
    return true;
}

static const bmedia_probe_file_ext b_cdxa_ext[] =  {
	{"dat"},
	{""}
};


static bmedia_probe_base_t 
b_cdxa_probe_create(batom_factory_t factory)
{
	bcdxa_probe_t	probe;

    BSTD_UNUSED(factory);
	probe = BKNI_Malloc(sizeof(*probe));
	if(!probe) {
		BDBG_ERR(("b_cdxa_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(probe, bcdxa_probe_t);
	return (bmedia_probe_base_t)probe;
err_alloc:
	return NULL;
}

static void 
b_cdxa_probe_destroy(bmedia_probe_base_t probe_)
{
	bcdxa_probe_t probe = (bcdxa_probe_t)probe_;

	BDBG_OBJECT_ASSERT(probe, bcdxa_probe_t);
	BDBG_OBJECT_DESTROY(probe, bcdxa_probe_t);
	BKNI_Free(probe);
	return;
}

const bmedia_probe_stream *
b_cdxa_probe_parse(bmedia_probe_base_t probe_, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config)
{
	bcdxa_probe_t probe = (bcdxa_probe_t)probe_;
    batom_cursor cursor;
    bool header_match;
    bmedia_probe_stream *stream;
    batom_t atom;
    bfile_buffer_result result;

    BSTD_UNUSED(pipe);
    BSTD_UNUSED(probe);
	BDBG_OBJECT_ASSERT(probe, bcdxa_probe_t);
    BDBG_ASSERT(buf);
    BDBG_ASSERT(config);

    atom = bfile_buffer_read(buf, config->parse_offset, B_CDXA_HEADER_SIZE, &result);
    if(!atom) {
        return NULL;
    }
    batom_cursor_from_atom(&cursor, atom);
    header_match = b_cdxa_probe_header_match(&cursor);
    batom_release(atom);
    if(!header_match) {
        return NULL;
    }

    stream = BKNI_Malloc(sizeof(*stream));
    if(!stream) {
        return NULL;
    }
    bmedia_probe_stream_init(stream, bstream_mpeg_type_cdxa);
    return stream;
}


const bmedia_probe_format_desc bcdxa_probe = {
	bstream_mpeg_type_mp4,
	b_cdxa_ext, /* ext_list */
	B_CDXA_HEADER_SIZE, 
	b_cdxa_probe_header_match, /* header_match */
	b_cdxa_probe_create, /* create */
	b_cdxa_probe_destroy, /* destroy */
	b_cdxa_probe_parse, /* parse */
	bmedia_probe_basic_stream_free/* stream free */
};

BDBG_OBJECT_ID(bcdxa_file);

struct bcdxa_file {
	struct bfile_io_read ops; /* shall be the first member */
    BDBG_OBJECT(bcdxa_file)
    bfile_io_read_t source;
    off_t cdxa_offset;
    unsigned first_sector;
    batom_factory_t factory;
    bfile_buffer_t buffer;
    uint8_t buf[1]; /* variable size buffer */
};

static void 
b_cdxa_get_source_segment(bcdxa_file_t fd, off_t cdxa_offset, bfile_segment *segment)
{
    unsigned sector_no;
    size_t partial_sector;

    sector_no = cdxa_offset/B_CDXA_SECTOR_PAYLOAD;
    sector_no += fd->first_sector;
    partial_sector = cdxa_offset%B_CDXA_SECTOR_PAYLOAD;
    segment->len = B_CDXA_SECTOR_PAYLOAD - partial_sector;
    segment->start = B_CDXA_HEADER_SIZE  + sector_no*(off_t)B_CDXA_SECTOR_SIZE+B_CDXA_SECTOR_HEADER+partial_sector;
    return;
}

static off_t
b_cdxa_get_offset(bcdxa_file_t fd, off_t source_offset)
{
    unsigned sector_no;
    size_t partial_sector;

    source_offset = source_offset>B_CDXA_HEADER_SIZE?source_offset-B_CDXA_HEADER_SIZE:0;
    sector_no = source_offset/B_CDXA_SECTOR_SIZE;
    sector_no = sector_no>fd->first_sector?sector_no-fd->first_sector:0;
    partial_sector = source_offset%B_CDXA_SECTOR_SIZE;
    BDBG_MSG(("b_cdxa_get_offset:%p source_offset:%u sector_no:%u partial_sector:%u", (void *)fd, (unsigned)source_offset, sector_no, (unsigned)partial_sector));
    partial_sector = partial_sector>B_CDXA_SECTOR_HEADER?partial_sector-B_CDXA_SECTOR_HEADER:0;
    if(partial_sector>B_CDXA_SECTOR_PAYLOAD) {
        partial_sector = B_CDXA_SECTOR_PAYLOAD;
    }
    return  sector_no * (off_t)B_CDXA_SECTOR_PAYLOAD + partial_sector;
}

static ssize_t 
b_cdxa_read(bfile_io_read_t fd_, void *buf, size_t length)
{
	bcdxa_file_t fd =  (void*)fd_;
    ssize_t bytes_completed = 0;

	BDBG_OBJECT_ASSERT(fd, bcdxa_file);

    BDBG_MSG(("b_cdxa_read:%p offset:%u length:%u", (void *)fd, (unsigned)fd->cdxa_offset, (unsigned)length));
    while(length>0) {
        bfile_segment segment;
        size_t to_read;
        size_t bytes_read;
        batom_t atom;
        bfile_buffer_result result;
        batom_cursor cursor;

        b_cdxa_get_source_segment(fd, fd->cdxa_offset, &segment);
        to_read = segment.len;
        if(to_read>length) {
            to_read = length;
        }
        BDBG_MSG(("b_cdxa_read:%p reading offset:%u  length:%u(%u)", (void *)fd, (unsigned)segment.start, (unsigned)to_read, (unsigned)length));
        atom = bfile_buffer_read(fd->buffer, segment.start, to_read, &result);
        if(!atom) {
            if(bytes_completed==0) {
                switch(result) {
                case bfile_buffer_result_eof:
                    /* keep result at 0 */
                    break;
                case bfile_buffer_result_no_data:
                    bytes_completed = BFILE_ERROR_NO_DATA;
                    break;
                default:
                    bytes_completed = -1;
                    break;
                }
            }
            break;
        }
        batom_cursor_from_atom(&cursor, atom);
        bytes_read = batom_cursor_copy(&cursor, buf, to_read);
        BDBG_MSG(("b_cdxa_read:%p read offset:%u length:%u:%u(%u)", (void *)fd, (unsigned)segment.start, (unsigned)bytes_read, (unsigned)to_read, (unsigned)length));
        batom_release(atom);
        BDBG_ASSERT(bytes_read<=to_read);
        BDBG_ASSERT(length>=to_read);
        buf = (uint8_t *)buf+to_read;
        fd->cdxa_offset += to_read;
        length -= to_read;
        bytes_completed += to_read;

        if(bytes_read!=to_read) {
            break;
        }
    }
    BDBG_MSG(("b_cdxa_read:%p completed offset:%u length:%d", (void *)fd, (unsigned)fd->cdxa_offset, (unsigned)bytes_completed));
    return bytes_completed;
}

static off_t
b_cdxa_seek(bfile_io_read_t fd_, off_t offset, int whence)
{
	bcdxa_file_t fd =  (void*)fd_;
    off_t cdxa_offset;
    off_t source_offset;
    bfile_segment segment;

	BDBG_OBJECT_ASSERT(fd, bcdxa_file);
    BDBG_MSG(("b_cdxa_seek:%#lx offset:%u %u", (unsigned long)fd, (unsigned)offset, (unsigned)whence));
    switch(whence) {
    case SEEK_SET: 
        cdxa_offset = offset;
        break;
    case SEEK_CUR:
        cdxa_offset = fd->cdxa_offset + offset;
        break;
    case SEEK_END:
        source_offset = fd->source->seek(fd->source, 0, SEEK_END);
        if(source_offset<0) {
            return source_offset;
        }
        cdxa_offset = b_cdxa_get_offset(fd, source_offset) - offset;
        break;
    default:
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return -1;
    }
    BDBG_MSG(("b_cdxa_seek:%#lx cdxa_offset:%u", (unsigned long)fd, (unsigned)cdxa_offset));
    if(cdxa_offset<0) {
        cdxa_offset= 0;
    }
    b_cdxa_get_source_segment(fd, cdxa_offset, &segment);
    source_offset = segment.start;
    source_offset = fd->source->seek(fd->source, source_offset, SEEK_SET);
    BDBG_MSG(("b_cdxa_seek:%#lx source_offset:%u", (unsigned long)fd, (unsigned)source_offset));
    if(source_offset<0) {
        return source_offset;
    }
    cdxa_offset = b_cdxa_get_offset(fd, source_offset);
    BDBG_MSG(("b_cdxa_seek:%#lx cdxa_offset:%u->%u", (unsigned long)fd, (unsigned)fd->cdxa_offset, (unsigned)cdxa_offset));
    fd->cdxa_offset = cdxa_offset;
    return cdxa_offset;
}

static int
b_cdxa_bounds(bfile_io_read_t fd_, off_t *first, off_t *last)
{
	bcdxa_file_t fd =  (void*)fd_;
    int rc;
	BDBG_OBJECT_ASSERT(fd, bcdxa_file);
    BDBG_MSG(("b_cdxa_bounds:%#lx", (unsigned long)fd));
    rc = fd->source->bounds(fd->source, first, last);
    if(rc==0) {
        if(first) {
            BDBG_MSG(("b_cdxa_bounds:%#lx first_source:%u", (unsigned long)fd, (unsigned)*first));
            *first = b_cdxa_get_offset(fd, *first);
            BDBG_MSG(("b_cdxa_bounds:%#lx first:%u", (unsigned long)fd, (unsigned)*first));
        }
        if(last) {
            BDBG_MSG(("b_cdxa_bounds:%#lx last_source:%u", (unsigned long)fd, (unsigned)*last));
            *last = b_cdxa_get_offset(fd, *last);
            BDBG_MSG(("b_cdxa_bounds:%#lx last:%u", (unsigned long)fd, (unsigned)*last));
        }
    }
    return rc;
}

static const struct bfile_io_read b_cdxa_ops = {
	b_cdxa_read,
	b_cdxa_seek,
	b_cdxa_bounds,
	BIO_DEFAULT_PRIORITY
};

static int
b_cdxa_file_probe(bcdxa_file_t fd)
{
    unsigned i, max_sector=256;
    /* find first sector with MPEG (system) start-code */
    for(i=0;i<max_sector;i++) {
        off_t sector_offset;
        batom_t atom;
        bfile_buffer_result result;
        batom_cursor cursor;
        uint32_t scode;

        sector_offset = B_CDXA_HEADER_SIZE + B_CDXA_SECTOR_SIZE*i + B_CDXA_SECTOR_HEADER;
        atom = bfile_buffer_read(fd->buffer, sector_offset, B_CDXA_SECTOR_PAYLOAD, &result);
        if(!atom) {
            break;
        }
        batom_cursor_from_atom(&cursor, atom);
	    scode = batom_cursor_uint32_be(&cursor);
        batom_release(atom);
        BDBG_MSG(("b_cdxa_file_probe:%#lx sector:%u offset:%u scode:%#x", (unsigned long)fd, i, (unsigned)sector_offset, scode&(~0xFF)));
	    if((scode&(~0xFF))==0x0100) { 
            fd->first_sector = i;
            BDBG_MSG(("b_cdxa_file_probe:%#lx found MPEG-1 stream at sector %u", (unsigned long)fd, i));
            return 0;
        }
    }
    return -1;
}

bcdxa_file_t
bcdxa_file_create(bfile_io_read_t fd_)
{
    bcdxa_file_t fd =  (void*)fd_;
    const size_t buffer_size = 8*BIO_BLOCK_SIZE;
    bfile_buffer_cfg buffer_cfg;

    fd = BKNI_Malloc(sizeof(*fd)+buffer_size+BIO_BLOCK_SIZE);
    if(fd==NULL) { (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc; }
    BDBG_OBJECT_INIT(fd, bcdxa_file);
    fd->cdxa_offset = 0;
    fd->source = fd_;
    fd->ops = b_cdxa_ops;
    fd->factory = batom_factory_create(bkni_alloc, 8);
    if(fd->factory==NULL) { (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_factory; }
    bfile_buffer_default_cfg(&buffer_cfg);
    buffer_cfg.nsegs = buffer_size/BIO_BLOCK_SIZE;
    buffer_cfg.buf_len = buffer_size;
    buffer_cfg.buf = (void *)((((unsigned long)fd->buf + (BIO_BLOCK_SIZE-1))/BIO_BLOCK_SIZE)*BIO_BLOCK_SIZE); /* get aligned buffer */
    buffer_cfg.fd = fd_;
    BDBG_MSG(("bcdxa_file_create: %p buffer: %p(%p)", (void *)fd, (void *)fd->buf, (void *)buffer_cfg.buf));
    fd->buffer = bfile_buffer_create(fd->factory, &buffer_cfg);
    if(fd->buffer==NULL) { (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_buffer; }
    if(b_cdxa_file_probe(fd)<0) {(void)BERR_TRACE(BERR_NOT_SUPPORTED);goto err_stream;}

    return fd;
err_stream:
    bfile_buffer_destroy(fd->buffer);
err_buffer:
    batom_factory_destroy(fd->factory);
err_factory:
    BKNI_Free(fd);
err_alloc:
    return NULL;
}

void
bcdxa_file_destroy(bcdxa_file_t fd)
{
	BDBG_OBJECT_ASSERT(fd, bcdxa_file);
   
    bfile_buffer_destroy(fd->buffer);
    batom_factory_destroy(fd->factory);
    BDBG_OBJECT_DESTROY(fd, bcdxa_file);
    BKNI_Free(fd);
    return;
}

bfile_io_read_t
bcdxa_file_get_file_interface(bcdxa_file_t fd)
{
	BDBG_OBJECT_ASSERT(fd, bcdxa_file);
    return &fd->ops;
}

