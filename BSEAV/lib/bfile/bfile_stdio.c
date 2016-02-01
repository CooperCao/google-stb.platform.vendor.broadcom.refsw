/***************************************************************************
 *     Copyright (c) 2006-2008, Broadcom Corporation
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
 ***************************************************************************/
#include "bstd.h"
#include "bfile_stdio.h"
#include "bkni.h"

BDBG_MODULE(bfile_stdio);

BDBG_OBJECT_ID(bfile_io_read_stdio);
struct bfile_io_read_stdio {
	struct bfile_io_read ops; /* shall be the first member */
	FILE *fin;
	BDBG_OBJECT(bfile_io_read_stdio)
};


static ssize_t 
b_stdio_read(bfile_io_read_t fd, void *buf, size_t length)
{
	struct bfile_io_read_stdio *f=(void *)fd;
	BDBG_OBJECT_ASSERT(f, bfile_io_read_stdio);
    BDBG_MSG(("b_stdio_read: %#lx %lu:%u", (unsigned long)fd, (unsigned long)ftello(f->fin), (unsigned)length));
	return fread(buf, 1, length, f->fin);
}

static off_t 
b_stdio_seek(bfile_io_read_t fd, off_t offset, int whence)
{
	struct bfile_io_read_stdio *f=(void *)fd;
	int rc;
	BDBG_OBJECT_ASSERT(f, bfile_io_read_stdio);

    BDBG_MSG(("b_stdio_seek: %#lx %lu:%d", (unsigned long)fd, (unsigned long)offset, whence));
	rc = fseeko(f->fin, offset, whence);
	if (rc!=0) { return -1;}
	return ftello(f->fin);
}

static int 
b_stdio_bounds(bfile_io_read_t fd, off_t *first, off_t *last)
{
	struct bfile_io_read_stdio *f=(void *)fd;
	off_t cur;
	int rc;
	BDBG_OBJECT_ASSERT(f, bfile_io_read_stdio);

    *first = *last = 0;
	cur = ftello(f->fin);
	if (cur<0) { return -1; }

	rc = fseeko(f->fin, 0, SEEK_END);
	if (rc<0) { return rc; }

	*first = 0;
	*last = ftello(f->fin);
	if (*last<0) { return -1; }
	rc = fseeko(f->fin, cur, SEEK_SET);
	if (rc<0) {return rc;}
	return 0;
}

static const struct bfile_io_read b_stdio_read_ops = {
	b_stdio_read,
	b_stdio_seek,
	b_stdio_bounds,
	BIO_DEFAULT_PRIORITY
};

bfile_io_read_t 
bfile_stdio_read_attach(FILE *fin)
{
	struct bfile_io_read_stdio *f;
	f=BKNI_Malloc(sizeof(*f));
	if (!f) { return NULL; }
	BDBG_OBJECT_INIT(f, bfile_io_read_stdio);
	f->ops = b_stdio_read_ops;
	f->fin = fin;
	return &f->ops;
}

void 
bfile_stdio_read_detach(bfile_io_read_t file)
{
	struct bfile_io_read_stdio *f=(void *)file;
	BDBG_OBJECT_ASSERT(f, bfile_io_read_stdio);

	BDBG_OBJECT_DESTROY(f, bfile_io_read_stdio);
	BKNI_Free(f);

	return;
}

BDBG_OBJECT_ID(bfile_io_write_stdio);
struct bfile_io_write_stdio {
	struct bfile_io_write ops;
	FILE *fout;
	BDBG_OBJECT(bfile_io_write_stdio)
};

static ssize_t 
b_stdio_write(bfile_io_write_t fd, const void *buf, size_t length)
{
	struct bfile_io_write_stdio *f=(void *)fd;
	BDBG_OBJECT_ASSERT(f, bfile_io_write_stdio );

	return fwrite(buf, 1, length, f->fout);
}

static off_t 
b_stdio_trim(bfile_io_write_t fd, off_t trim_pos)
{
	BSTD_UNUSED(fd);
	BSTD_UNUSED(trim_pos);
	BDBG_WRN(("trim is not supported"));
	return 0;
}

static const struct bfile_io_write b_stdio_write_ops = {
	b_stdio_write,
	b_stdio_trim,
	BIO_DEFAULT_PRIORITY
};


bfile_io_write_t 
bfile_stdio_write_attach(FILE *fout)
{
	struct bfile_io_write_stdio *f;
	f=BKNI_Malloc(sizeof(*f));
	if (!f) { return NULL; }
	BDBG_OBJECT_INIT(f, bfile_io_write_stdio );
	f->ops = b_stdio_write_ops;
	f->fout = fout;
	return &f->ops;
}

void 
bfile_stdio_write_detach(bfile_io_read_t fd)
{
	struct bfile_io_write_stdio *f=(void *)fd;
	BDBG_OBJECT_ASSERT(f, bfile_io_write_stdio );

	BDBG_OBJECT_DESTROY(f, bfile_io_write_stdio);
	BKNI_Free(f);

	return;
}

