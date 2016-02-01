/***************************************************************************
 *     Copyright (c) 2007-2011, Broadcom Corporation
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
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bfile_util.h"

BDBG_MODULE(bfile_util);

BDBG_OBJECT_ID(bfile_read_offset);
struct bfile_read_offset {
	struct bfile_io_read ops; /* shall be the first member */
    off_t source_offset;
    bfile_io_read_t source;
	BDBG_OBJECT(bfile_read_offset)
};


static ssize_t 
b_file_read_offset_read(bfile_io_read_t fd, void *buf, size_t length)
{
	struct bfile_read_offset *f=(void *)fd;
	BDBG_OBJECT_ASSERT(f, bfile_read_offset);
    BDBG_MSG(("b_file_read_offset_read: %#lx %u", (unsigned long)fd, (unsigned)length));
    return f->source->read(f->source, buf, length);
}

static off_t
b_file_read_offset_correct_offset(const struct bfile_read_offset *f, off_t offset)
{
    if(offset>=f->source_offset) {
        offset -= f->source_offset;
    } else {
        offset = 0;
    }    
    return offset;
}

static off_t 
b_file_read_offset_seek(bfile_io_read_t fd, off_t offset, int whence)
{
	struct bfile_read_offset *f=(void *)fd;
	off_t rc;
	BDBG_OBJECT_ASSERT(f, bfile_read_offset);

    BDBG_MSG(("b_file_read_offset_seek: %#lx %lu:%d", (unsigned long)fd, (unsigned long)offset, whence));
    if(whence==SEEK_SET) {
        offset +=  f->source_offset;
    }
    rc = f->source->seek(f->source, offset, whence);
    if(rc>=0 && rc<f->source_offset) { /* if seeked behind source_offset, seek back */
        rc = f->source->seek(f->source, f->source_offset, SEEK_SET);
    }
    if(rc>=0) { /* correct  returned offset */
        rc = b_file_read_offset_correct_offset(f, rc);
    }
    return rc;
}

static int 
b_file_read_offset_bounds(bfile_io_read_t fd, off_t *first, off_t *last)
{
    off_t source_first=0, source_last=0;
	struct bfile_read_offset *f=(void *)fd;
    int rc;
	BDBG_OBJECT_ASSERT(f, bfile_read_offset);

    rc = f->source->bounds(f->source, &source_first, &source_last);
    source_first = b_file_read_offset_correct_offset(f, source_first);
    source_last = b_file_read_offset_correct_offset(f, source_last);
    if(first) {
        *first = source_first;
    }
    if(last) {
        *last = source_last;
    }
	return rc;
}

static const struct bfile_io_read b_file_read_offset_read_ops = {
	b_file_read_offset_read,
	b_file_read_offset_seek,
	b_file_read_offset_bounds,
	BIO_DEFAULT_PRIORITY
};

bfile_io_read_t 
bfile_read_offset_attach(bfile_io_read_t source, off_t offset)
{
	struct bfile_read_offset *f;
	f=BKNI_Malloc(sizeof(*f));
	if (!f) { return NULL; }
	BDBG_OBJECT_INIT(f, bfile_read_offset);
	f->ops = b_file_read_offset_read_ops;
	f->source = source;
    f->source_offset = offset;
	return &f->ops;
}

void 
bfile_read_offset_detach(bfile_io_read_t file)
{
	struct bfile_read_offset *f=(void *)file;
	BDBG_OBJECT_ASSERT(f, bfile_read_offset);

	BDBG_OBJECT_DESTROY(f, bfile_read_offset);
	BKNI_Free(f);

	return;
}

BDBG_OBJECT_ID(bfile_read_header);
struct bfile_read_header {
	struct bfile_io_read ops; /* shall be the first member */
    const void *header;
    size_t header_size;
    off_t current_offset;
    bfile_io_read_t source;
	BDBG_OBJECT(bfile_read_header)
};


static ssize_t 
b_file_read_header_read(bfile_io_read_t fd, void *buf, size_t length)
{
	struct bfile_read_header *f=(void *)fd;
    ssize_t rc;
    size_t copied=0;
	BDBG_OBJECT_ASSERT(f, bfile_read_header);
    BDBG_MSG(("b_file_read_header_read: %#lx %u(%lu)", (unsigned long)fd, (unsigned)length, (unsigned long)f->current_offset));
    if((off_t)f->header_size>f->current_offset) {
        copied = f->header_size - f->current_offset;
        copied = length > copied ? copied : length;
        BKNI_Memcpy(buf, (uint8_t *)f->header + f->current_offset, copied);
        buf = (uint8_t *)buf + copied;
        f->current_offset += copied;
        length -= copied;
        if(length==0) {
            return copied;
        }
    }
    rc = f->source->read(f->source, buf, length);
    if(rc<0) {
        if(copied>0) {
            rc = copied;
        } 
    } else {
        f->current_offset += rc;
        rc = rc + copied;
    }
    BDBG_MSG(("b_file_read_header_read: %#lx %u->%d(%lu)", (unsigned long)fd, (unsigned)length, (int)rc, (unsigned long)f->current_offset));
    return rc;
}

static off_t
b_file_read_header_correct_offset(const struct bfile_read_header *f, off_t offset)
{
    return offset + f->header_size;
}

static off_t 
b_file_read_header_seek(bfile_io_read_t fd, off_t offset, int whence)
{
	struct bfile_read_header *f=(void *)fd;
	off_t rc;
    off_t source_offset;
    off_t target_offset = 0; 
    bool target_header = false;
	BDBG_OBJECT_ASSERT(f, bfile_read_header);

    BDBG_MSG(("b_file_read_header_seek: %#lx %ld:%d(%lu)", (unsigned long)fd, (long)offset, whence, (unsigned long)f->current_offset));
    switch(whence) {
    default:
    case SEEK_SET:
        if(offset>(off_t)f->header_size) {
            source_offset =  offset - f->header_size;
        } else {
            source_offset = 0;
            target_header = true;
            target_offset = offset;
        }
        break;
    case SEEK_END:
        /* XXX this is not fully supported and to get it supported requires knowledge of the source file size */
        source_offset = offset;
        break;
    case SEEK_CUR:
        target_offset = f->current_offset + offset;
        if(target_offset<0) {
            target_offset = 0;
        }

        if(f->current_offset > (off_t)f->header_size) { /* seek from source */
            if(target_offset > (off_t)f->header_size) { /* to source */
                source_offset = offset;
            } else { /* to header */
                source_offset = -(f->current_offset - f->header_size); /* set source back to 0 */
                target_header = true;
            }
        } else  { /* seek from header */
            if(target_offset > (off_t)f->header_size) { /* to source */
                source_offset = offset - f->header_size;
            } else { /* to header */
                source_offset = 0; /* stay at current location */
                target_header = true;
            }
        }
        break;
    }
    rc = f->source->seek(f->source, source_offset, whence);
    if(rc>=0) { /* correct  returned offset */
        if(target_header) {
            rc = target_offset;
        }  else {
            rc = b_file_read_header_correct_offset(f, rc);
        }
        f->current_offset = rc;
    }
    BDBG_MSG(("b_file_read_header_seek: %#lx %ld(%ld):%d(%lu)->%ld", (unsigned long)fd, (long)offset, (long)source_offset, whence, (unsigned long)f->current_offset, (long)rc));
    return rc;
}

static int 
b_file_read_header_bounds(bfile_io_read_t fd, off_t *first, off_t *last)
{
    off_t source_first=0, source_last=0;
	struct bfile_read_header *f=(void *)fd;
    int rc;
	BDBG_OBJECT_ASSERT(f, bfile_read_header);

    rc = f->source->bounds(f->source, &source_first, &source_last);
    source_first = 0;
    source_last = b_file_read_header_correct_offset(f, source_last);
    if(first) {
        *first = source_first;
    }
    if(last) {
        *last = source_last;
    }
	return rc;
}

static const struct bfile_io_read b_file_read_header_read_ops = {
	b_file_read_header_read,
	b_file_read_header_seek,
	b_file_read_header_bounds,
	BIO_DEFAULT_PRIORITY
};

bfile_io_read_t 
bfile_read_header_attach(bfile_io_read_t source, const void *header, size_t header_size)
{
	struct bfile_read_header *f;
    BDBG_ASSERT(header);
	f=BKNI_Malloc(sizeof(*f));
	if (!f) { return NULL; }
	BDBG_OBJECT_INIT(f, bfile_read_header);
	f->ops = b_file_read_header_read_ops;
	f->source = source;
    f->header = header;
    f->header_size = header_size;
    f->current_offset = 0;
	return &f->ops;
}

void 
bfile_read_header_detach(bfile_io_read_t file)
{
	struct bfile_read_header *f=(void *)file;
	BDBG_OBJECT_ASSERT(f, bfile_read_header);

	BDBG_OBJECT_DESTROY(f, bfile_read_header);
	BKNI_Free(f);

	return;
}


