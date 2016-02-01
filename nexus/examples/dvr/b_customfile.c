/******************************************************************************
 *    (c)2011 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bfile_io.h"
#include "b_customfile.h"
#include "nexus_file_pvr.h"

BDBG_MODULE(b_customfile);

static bfile_io_read_t b_customfile_read_offset_attach(bfile_io_read_t source, const struct b_customfile_bounds *bounds);
static void b_customfile_read_offset_detach(bfile_io_read_t file);

/************************************
* b_customfile wrapper
**/

void
b_customfile_get_default_settings(b_customfile_settings *settings)
{
    BKNI_Memset(settings, 0, sizeof(*settings));
}

typedef struct b_customfile {
    struct NEXUS_FilePlay parent; /* must be first */
    NEXUS_FilePlayHandle original;
} b_customfile;

static void
NEXUS_P_FilePlayOffset_Close(struct NEXUS_FilePlay *file_)
{
    b_customfile *file = (b_customfile *)file_;
    b_customfile_read_offset_detach(file->parent.file.data);
    if(file->parent.file.index) {
        b_customfile_read_offset_detach(file->parent.file.index);
    }
    BKNI_Free(file);
    return;
}

NEXUS_FilePlayHandle
b_customfile_open( NEXUS_FilePlayHandle file, const b_customfile_settings *settings)
{
    b_customfile *fileOffset;

    BDBG_ASSERT(file);
    BDBG_ASSERT(settings);
    BDBG_ASSERT(file->file.data);

    fileOffset = BKNI_Malloc(sizeof(*fileOffset));
    if(fileOffset==NULL) {
        (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    fileOffset->parent.file.index = NULL;

    fileOffset->parent.file.data = b_customfile_read_offset_attach(file->file.data, &settings->data);
    if(fileOffset->parent.file.data==NULL) {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err_data;
    }

    if(file->file.index) {
        fileOffset->parent.file.index = b_customfile_read_offset_attach(file->file.index, &settings->index);
        if(fileOffset->parent.file.index==NULL) {
            (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
            goto err_index;
        }
    }
    fileOffset->parent.file.close = NEXUS_P_FilePlayOffset_Close;
    fileOffset->original = file;
    return &fileOffset->parent;

err_index:
    b_customfile_read_offset_detach(file->file.data);
err_data:
    BKNI_Free(fileOffset);
err_alloc:
    return NULL;
}

/************************************
* bfile abstraction
**/

BDBG_OBJECT_ID(bfile_read_offset);
struct bfile_read_offset {
    struct bfile_io_read ops; /* shall be the first member */
    struct b_customfile_bounds bounds;
    bfile_io_read_t source;
    BDBG_OBJECT(bfile_read_offset)
};


static ssize_t
b_customfile_offset_read(bfile_io_read_t fd, void *buf, size_t length)
{
    struct bfile_read_offset *f=(void *)fd;
    BDBG_OBJECT_ASSERT(f, bfile_read_offset);
    BDBG_MSG(("b_customfile_offset_read: %#lx %u", (unsigned long)fd, (unsigned)length));
    return f->source->read(f->source, buf, length);
}

/**
Convert actual file offset back to the virtual offset used by the caller.

NOTE: b_file_correct_offset in BSEAV/lib/bfile/bfile_utils.c uses a negative offset,
which is a little abstract. I'm using a positive offset here.
**/
static off_t
b_file_correct_offset(const struct bfile_read_offset *f, off_t offset)
{
    /* always apply bof bounds */
    if (offset >= f->bounds.bof) {
        /* apply eof bounds if non-zero */
        if (f->bounds.eof && offset > f->bounds.eof) {
            /* greater than eof is just eof-bof */
            offset = f->bounds.eof - f->bounds.bof;
        }
        else {
            /* in the middle of the file, just subtract off bof */
            offset -= f->bounds.bof;
        }
    }
    else {
        /* less than bof is just bof */
        offset = 0;
    }

    return offset;
}

static off_t
b_customfile_offset_seek(bfile_io_read_t fd, off_t offset, int whence)
{
    struct bfile_read_offset *f=(void *)fd;
    off_t rc;
    BDBG_OBJECT_ASSERT(f, bfile_read_offset);

    BDBG_MSG(("b_customfile_offset_seek: %#lx %lu:%d", (unsigned long)fd, (unsigned long)offset, whence));
    if (whence==SEEK_SET) {
        offset += f->bounds.bof;
        if (f->bounds.eof && offset > f->bounds.eof) {
            offset = f->bounds.eof;
        }
    }
    rc = f->source->seek(f->source, offset, whence);
    if (rc>=0) {
        /* if seeked in front of bounds.bof, seek back */
        if (rc<f->bounds.bof) {
            rc = f->source->seek(f->source, f->bounds.bof, SEEK_SET);
        }
        /* if seeked in back of bounds.bof, seek back */
        if (f->bounds.eof && rc > f->bounds.eof) {
            rc = f->source->seek(f->source, f->bounds.eof, SEEK_SET);
        }
    }
    if (rc>=0) { /* correct returned offset */
        rc = b_file_correct_offset(f, rc);
    }
    return rc;
}

static int
b_customfile_offset_bounds(bfile_io_read_t fd, off_t *first, off_t *last)
{
    off_t source_first=0, source_last=0;
    struct bfile_read_offset *f=(void *)fd;
    int rc;
    BDBG_OBJECT_ASSERT(f, bfile_read_offset);

    rc = f->source->bounds(f->source, &source_first, &source_last);
    source_first = b_file_correct_offset(f, source_first);
    source_last = b_file_correct_offset(f, source_last);
    if(first) {
        *first = source_first;
    }
    if(last) {
        *last = source_last;
    }
    return rc;
}

static const struct bfile_io_read b_customfile_offset_read_ops = {
    b_customfile_offset_read,
    b_customfile_offset_seek,
    b_customfile_offset_bounds,
    BIO_DEFAULT_PRIORITY
};

static bfile_io_read_t
b_customfile_read_offset_attach(bfile_io_read_t source, const struct b_customfile_bounds *bounds)
{
    struct bfile_read_offset *f;
    f=BKNI_Malloc(sizeof(*f));
    if (!f) { return NULL; }
    BDBG_OBJECT_INIT(f, bfile_read_offset);
    f->ops = b_customfile_offset_read_ops;
    f->source = source;
    f->bounds = *bounds;
    return &f->ops;
}

static void
b_customfile_read_offset_detach(bfile_io_read_t file)
{
    struct bfile_read_offset *f=(void *)file;
    BDBG_OBJECT_ASSERT(f, bfile_read_offset);

    BDBG_OBJECT_DESTROY(f, bfile_read_offset);
    BKNI_Free(f);

    return;
}
