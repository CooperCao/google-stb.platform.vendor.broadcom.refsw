/******************************************************************************
 *  Copyright (C) 200 4-2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom Inc. and/or its licensors,
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
 *****************************************************************************/
#include "nexus_platform.h"
#if NEXUS_HAS_PLAYBACK
#include "bfile_io.h"
#include "nexus_file.h"
#include "nexus_file_pvr.h"
#include "bstd.h"
#include "bkni.h"
#include "fileio_custom.h"

BDBG_MODULE(fileio_custom);
BDBG_OBJECT_ID(FileIoCustom);
BDBG_OBJECT_ID(FileIoCustomIo);


typedef struct FileIoCustom FileIoCustom;


struct FileIoCustomIo {
    struct bfile_io_read interface; /* must be first */
    unsigned count;
    FileIoCustomProbabilities probabilities;
    bfile_io_read_t parent;
    FileIoCustom *master;
    BDBG_OBJECT(FileIoCustomIo)
};

struct FileIoCustom {
    struct NEXUS_FilePlay interface; /* must be the first member */
    NEXUS_FilePlayHandle parent; /* parent */
    struct FileIoCustomIo data;
    struct FileIoCustomIo index;
    BDBG_OBJECT(FileIoCustom)
};


static bool
b_test_dice(unsigned dice, unsigned probability)
{
    /* BDBG_ERR(("%u %u", dice, probability)); */
    return (dice%100) < probability;
}

#define B_FILE_TYPE(io) ((io == &io->master->index)?"INDEX":"DATA")
static ssize_t 
b_custom_read(bfile_io_read_t fd, void *buf, size_t length)
{
    struct FileIoCustomIo *io;
    unsigned dice;

    io = (void*)fd;
    BDBG_OBJECT_ASSERT(io, FileIoCustomIo);

    io->count++;
    dice = io->count;

    if(b_test_dice(dice, io->probabilities.error)) {
        BDBG_WRN(("%s:returning error", B_FILE_TYPE(io)));
        return -1;
    }
    if(b_test_dice(dice, io->probabilities.nodata)) {
        BDBG_WRN(("%s:returning no-data", B_FILE_TYPE(io)));
        return BFILE_ERROR_NO_DATA;
    }
    if(b_test_dice(dice, io->probabilities.partial_read) && length>=2) {
        BDBG_WRN(("%s:partial read:%u:%u", B_FILE_TYPE(io), (unsigned)length, (unsigned)(length/2)));
        length /= 2;
    }
    return io->parent->read(io->parent, buf, length);
}

static off_t 
b_custom_seek(bfile_io_read_t fd, off_t offset, int whence)
{
    struct FileIoCustomIo *io;
    unsigned dice;

    io = (void*)fd;
    BDBG_OBJECT_ASSERT(io, FileIoCustomIo);

    dice = io->count;
    if(b_test_dice(dice, io->probabilities.error)) {
        BDBG_WRN(("%s:seek returning error", B_FILE_TYPE(io)));
        return -1;
    }
    return io->parent->seek(io->parent, offset, whence);
}

static int 
b_custom_bounds(bfile_io_read_t fd, off_t *first, off_t *last)
{
    struct FileIoCustomIo *io;
    unsigned dice;

    io = (void*)fd;
    BDBG_OBJECT_ASSERT(io, FileIoCustomIo);
    
    dice = io->count;
    if(b_test_dice(dice, io->probabilities.error)) {
        BDBG_WRN(("%s:bounds returning error", B_FILE_TYPE(io)));
        return -1;
    }
    return io->parent->bounds(io->parent, first, last);
}

static void 
FileIoCustomIo_Init(struct FileIoCustomIo *io, bfile_io_read_t parent, FileIoCustom *master)
{
    BDBG_OBJECT_INIT(io, FileIoCustomIo);
    io->parent = parent;
    io->master = master;
    io->count = 0;
    io->interface.read = b_custom_read;
    io->interface.seek = b_custom_seek;
    io->interface.bounds = b_custom_bounds;
    io->interface.priority = parent->priority;
    BKNI_Memset(&io->probabilities, 0, sizeof(io->probabilities));
    return;
}

static void 
b_custom_close(struct NEXUS_FilePlay *f)
{
    FileIoCustom *file = (void *)f;
    BDBG_OBJECT_ASSERT(file, FileIoCustom);
    file->parent->file.close(file->parent);
    BDBG_OBJECT_DESTROY(file, FileIoCustom);
    BKNI_Free(file);
}

NEXUS_FilePlayHandle 
FileIoCustom_Attach(NEXUS_FilePlayHandle parent)
{
    FileIoCustom *file;

    file = BKNI_Malloc(sizeof(*file));
    if(!file) { (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);return NULL;}
    BDBG_OBJECT_INIT(file, FileIoCustom);
    file->parent = parent;
    FileIoCustomIo_Init(&file->data, parent->file.data, file);
    file->interface.file.data = &file->data.interface;
    file->interface.file.close = b_custom_close;
    if(parent->file.index) {
        FileIoCustomIo_Init(&file->index, parent->file.index, file);
        file->interface.file.index = &file->index.interface;
    } else {
        file->interface.file.index = NULL;
    }
    return &file->interface;
}

void 
FileIoCustom_GetProbabilities(NEXUS_FilePlayHandle file_, FileIoCustomProbabilities *dataProbabilities, FileIoCustomProbabilities *indexProbabilities)
{
    FileIoCustom *file;

    file = (void*)file_;
    BDBG_OBJECT_ASSERT(file, FileIoCustom);
    if(dataProbabilities) {
        *dataProbabilities = file->data.probabilities;
    }
    if(indexProbabilities) {
        *indexProbabilities = file->index.probabilities;
    }
    return;
}

BERR_Code 
FileIoCustom_SetProbabilities(NEXUS_FilePlayHandle file_, const FileIoCustomProbabilities *dataProbabilities, const FileIoCustomProbabilities *indexProbabilities)
{
    FileIoCustom *file;

    file = (void*)file_;
    BDBG_OBJECT_ASSERT(file, FileIoCustom);
    if(dataProbabilities) {
        file->data.probabilities = *dataProbabilities;
    }
    if(indexProbabilities) {
        file->index.probabilities = *indexProbabilities;
    }
    return BERR_SUCCESS;
}

BDBG_OBJECT_ID(FileIoSticky);
BDBG_OBJECT_ID(FileIoStickyIo);

typedef struct FileIoSticky FileIoSticky;

struct FileIoStickyIo {
    struct bfile_io_read interface; /* must be first */
    bool failed;
    bfile_io_read_t parent;
    FileIoSticky *master;
    BDBG_OBJECT(FileIoStickyIo)
};

struct FileIoSticky {
    struct NEXUS_FilePlay interface; /* must be the first member */
    NEXUS_FilePlayHandle parent; /* parent */
    struct FileIoStickyIo data;
    struct FileIoStickyIo index;
    BDBG_OBJECT(FileIoSticky)
};

static ssize_t 
b_sticky_read(bfile_io_read_t fd, void *buf, size_t length)
{
    struct FileIoStickyIo *io;
    ssize_t rc;

    io = (void*)fd;
    BDBG_OBJECT_ASSERT(io, FileIoStickyIo);

    if(io->failed) {
        return -1;
    }

    rc = io->parent->read(io->parent, buf, length);
    if(rc>=0) {
        return rc;
    }
    if((io == &io->master->data) && rc==BFILE_ERROR_NO_DATA) {
        return rc;
    }
    BDBG_WRN(("b_sticky_read: %s failed %d", B_FILE_TYPE(io), (int)rc));
    io->failed = true;
    return rc;
}

static off_t 
b_sticky_seek(bfile_io_read_t fd, off_t offset, int whence)
{
    struct FileIoStickyIo *io;
    off_t rc;

    io = (void*)fd;
    BDBG_OBJECT_ASSERT(io, FileIoStickyIo);

    rc = io->parent->seek(io->parent, offset, whence);
    if(rc<0) {
        BDBG_WRN(("b_sticky_seek: %s failed %d", B_FILE_TYPE(io), (int)rc));
        io->failed = true;
    }
    return rc;
}

static int 
b_sticky_bounds(bfile_io_read_t fd, off_t *first, off_t *last)
{
    struct FileIoStickyIo *io;
    int rc;

    io = (void*)fd;
    BDBG_OBJECT_ASSERT(io, FileIoStickyIo);
    
    rc = io->parent->bounds(io->parent, first, last);
    if(rc<0) {
        BDBG_WRN(("b_sticky_bounds: %s failed %d", B_FILE_TYPE(io), rc));
        io->failed = true;
    }
    return rc;
}

static void 
FileIoStickyIo_Init(struct FileIoStickyIo *io, bfile_io_read_t parent, FileIoSticky *master)
{
    BDBG_OBJECT_INIT(io, FileIoStickyIo);
    io->failed = false;
    io->parent = parent;
    io->master = master;
    io->interface.read = b_sticky_read;
    io->interface.seek = b_sticky_seek;
    io->interface.bounds = b_sticky_bounds;
    io->interface.priority = parent->priority;
    return;
}

static void 
b_sticky_close(struct NEXUS_FilePlay *f)
{
    FileIoSticky *file = (void *)f;
    BDBG_OBJECT_ASSERT(file, FileIoSticky);
    file->parent->file.close(file->parent);
    BDBG_OBJECT_DESTROY(file, FileIoSticky);
    BKNI_Free(file);
}

NEXUS_FilePlayHandle 
FileIoSticky_Attach(NEXUS_FilePlayHandle parent)
{
    FileIoSticky *file;

    file = BKNI_Malloc(sizeof(*file));
    if(!file) { (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);return NULL;}
    BDBG_OBJECT_INIT(file, FileIoSticky);
    file->parent = parent;
    FileIoStickyIo_Init(&file->data, parent->file.data, file);
    file->interface.file.data = &file->data.interface;
    file->interface.file.close = b_sticky_close;
    if(parent->file.index) {
        FileIoStickyIo_Init(&file->index, parent->file.index, file);
        file->interface.file.index = &file->index.interface;
    } else {
        file->interface.file.index = NULL;
    }
    return &file->interface;
}

void 
FileIoSticky_GetFailBit(NEXUS_FilePlayHandle file_, bool *failed)
{
    FileIoSticky *file;

    file = (void*)file_;
    BDBG_OBJECT_ASSERT(file, FileIoSticky);
    *failed = file->data.failed || file->index.failed;
    return;
}

void 
FileIoSticky_ClearFailBit(NEXUS_FilePlayHandle file_)
{
    FileIoSticky *file;

    file = (void*)file_;
    BDBG_OBJECT_ASSERT(file, FileIoSticky);
    file->data.failed = false;
    file->index.failed = false;
    return;
}
#endif /* NEXUS_HAS_PLAYBACK */
