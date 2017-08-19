/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 ***************************************************************************/

#include "nexus_file_module.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "blst_queue.h"
#include "bcmplayer.h"
#include "bcmindexer.h"
#include "bfile_io.h"
#include "nexus_file_pvr.h"
#include "nexus_file_types.h"
#include "nexus_file_posix.h"

BDBG_MODULE(nexus_file_chunk);

#if 0
/* just to exercise more corner cases */
#define B_DEFAULT_CHUNK_SIZE    (73*4095)
#else
/* for now just reasonable big number */
#define B_DEFAULT_CHUNK_SIZE    (256*1024*1024)
#endif
#define B_MAX_FILE_LEN  128

struct bfile_io_write_chunk {
    struct bfile_io_write self;
    NEXUS_FileRecordHandle data; /* current data file */
    unsigned chunk; /* current chunk */
    size_t  chunk_budget; /* number of bytes left in the chunk */
    NEXUS_FileRecordHandle (*open_out)(const char *fname, const char *indexname);
    NEXUS_ChunkedFileRecordOpenSettings settings;
    char base_name[B_MAX_FILE_LEN];
};

void NEXUS_ChunkedFilePlay_GetDefaultOpenSettings(
    NEXUS_ChunkedFilePlayOpenSettings *pSettings /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_ChunkedFilePlayOpenSettings));
    pSettings->chunkSize = B_DEFAULT_CHUNK_SIZE;
    b_strncpy(pSettings->chunkTemplate, "%s/chunk.%u", sizeof(pSettings->chunkTemplate));
}

void NEXUS_ChunkedFileRecord_GetDefaultOpenSettings(
    NEXUS_ChunkedFileRecordOpenSettings *pSettings /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_ChunkedFileRecordOpenSettings));
    pSettings->chunkSize = B_DEFAULT_CHUNK_SIZE;
    b_strncpy(pSettings->chunkTemplate, "%s/chunk.%u", sizeof(pSettings->chunkTemplate));
}


static void
get_chunk_name(char *buf, unsigned bufsize, const char *chunkTemplate, const char *filename, unsigned chunk)
{
    /* ChunkedFilePlay/Record only support "%s %u" templates */
    BKNI_Snprintf(buf, bufsize, chunkTemplate, filename, chunk);
}

static ssize_t
chunk_write(bfile_io_write_t fd, const void *buf_, size_t length)
{
    struct bfile_io_write_chunk *file = (struct bfile_io_write_chunk *) fd;
    ssize_t result;
    const void *buf = buf_;


    for(result=0;length>0;) {
        size_t to_write = length;
        ssize_t rc;
        char name[B_MAX_FILE_LEN];

        /* NOTE: we were getting a zero-byte chunk 0 file. This fixes that. */
        if (!file->data || file->chunk_budget==0) { /* time to open new file */
            if(file->data) {
                NEXUS_FileRecord_Close(file->data);
                file->chunk++;
            }
            get_chunk_name(name, sizeof(name), file->settings.chunkTemplate, file->base_name, file->chunk);
            BDBG_MSG(("write opening %s", name));
            file->data = file->open_out(name, NULL);
            file->chunk_budget = file->settings.chunkSize;
        }

        if (to_write>file->chunk_budget) {
            to_write = file->chunk_budget;
        }
        BDBG_MSG((">writing [%u/%u] (%u/%u/%u) %lx", file->chunk, (unsigned)result, (unsigned)to_write, (unsigned)length, (unsigned)file->chunk_budget, (unsigned long)buf));
        rc = file->data->data->write(file->data->data, buf, to_write);
        BDBG_MSG(("<writing [%u/%u] (%u/%u/%u) %lx->%d", file->chunk, (unsigned)result, (unsigned)to_write, (unsigned)length, (unsigned)file->chunk_budget, (unsigned long)buf, (int)rc));
        if (rc<=0) {
            if  (result==0) {
                result = rc; /* propagate error */
            }
            break;
        }
        result += rc;
        length -= (size_t) rc;
        BDBG_ASSERT((size_t)rc <= file->chunk_budget);
        file->chunk_budget -=  (size_t)rc;
        buf = (const uint8_t*)buf + rc;
    }
    return result;
}

static off_t
chunk_trim(bfile_io_write_t fd, off_t trim_pos)
{
    BSTD_UNUSED(fd);
    BSTD_UNUSED(trim_pos);
    return 0;
}

/* impl of NEXUS_ChunkedFileRecord */
struct bfile_out_chunk {
    struct NEXUS_FileRecord self;
    struct bfile_io_write_chunk data;
    NEXUS_FileRecordHandle index; /* index file */
};


static void
chunk_close_out(NEXUS_FileRecordHandle f)
{
    struct bfile_out_chunk *file =  (struct bfile_out_chunk *)f;

    if (file->data.data) {
        NEXUS_FileRecord_Close(file->data.data);
    }
    file->index->close(file->index);
    BKNI_Free(file);
    return;
}


NEXUS_FileRecordHandle
NEXUS_ChunkedFileRecord_Open(const char *fname, const char *indexname, const NEXUS_ChunkedFileRecordOpenSettings *pSettings)
{
    struct bfile_out_chunk *file;
    char name[B_MAX_FILE_LEN];
    NEXUS_ChunkedFileRecordOpenSettings openSettings;

    file = BKNI_Malloc(sizeof(*file));
    if (!file) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    if ( NULL == pSettings )
    {
        NEXUS_ChunkedFileRecord_GetDefaultOpenSettings(&openSettings);
        pSettings = &openSettings;
    }
    file->data.settings = *pSettings;
    BKNI_Snprintf(file->data.base_name, sizeof(file->data.base_name), "%s", fname); /* dirty strcpy */
    file->data.chunk = 0;
    file->data.data = NULL;
    file->data.chunk_budget = 0; /* it would trigger open of data file on first call to write */
    get_chunk_name(name, sizeof(name), pSettings->chunkTemplate, file->data.base_name, file->data.chunk);
    file->data.open_out = NEXUS_FileRecord_OpenPosix; /* hard-coded, this may be exposed later if needed */
    file->index = file->data.open_out(name, indexname);
    if (!file->index) {
        BKNI_Free(file);
        return NULL;
    }
    /* wire things together */
    file->self.index = file->index->index;
    file->self.close = chunk_close_out;
    file->data.self = *file->index->data;
    file->data.self.write = chunk_write;
    file->data.self.trim = chunk_trim;
    file->self.data = &file->data.self;
    return &file->self;
}

struct bfile_io_read_chunk {
    struct bfile_io_read self;
    NEXUS_FilePlayHandle data; /* current data file */
    unsigned chunk; /* current chunk */
    size_t  chunk_budget; /* number of bytes left in the chunk */
    NEXUS_FilePlayHandle (*open_in)(const char *fname, const char *indexname);
    char base_name[B_MAX_FILE_LEN];
    off_t cur_pos;
    struct {
        unsigned last_chunk;
        off_t last_chunk_begin;
        struct bfile_io_read_posix last_file;
        bool last_file_open;
    } size;
    NEXUS_ChunkedFilePlayOpenSettings settings;
};


static NEXUS_FilePlayHandle
chunk_read_open(const struct bfile_io_read_chunk *file, unsigned chunk)
{
    char name[B_MAX_FILE_LEN];
    NEXUS_FilePlayHandle data;

    get_chunk_name(name, sizeof(name), file->settings.chunkTemplate, file->base_name, chunk);
    BDBG_MSG((">read opening %s", name));
    data = file->open_in(name, NULL);
    BDBG_MSG(("<read opening %s %p", name, (void*)data));
    return data;
}

static ssize_t
chunk_read(bfile_io_read_t fd, void *buf_, size_t length)
{
    struct bfile_io_read_chunk *file = (struct bfile_io_read_chunk *) fd;
    ssize_t result;
    void *buf = buf_;

    BDBG_MSG((">read %p", (void*)fd));
    for(result=0;length>0;) {
        size_t to_read = length;
        ssize_t rc;

        if (file->chunk_budget==0) { /* time to open new file */
            file->chunk++;
            if(file->data) { 
                BDBG_MSG(("read closing %p", (void*)file->data));
                NEXUS_FilePlay_Close(file->data);
            }
            if (file->chunk < file->settings.firstChunkNumber) {
                file->data = NULL;
            }
            else {
                file->data = chunk_read_open(file, file->chunk);
            }
            file->chunk_budget = file->settings.chunkSize;
        }
        if (to_read>file->chunk_budget) {
            to_read = file->chunk_budget;
        }
        if (file->chunk < file->settings.firstChunkNumber) {
            BKNI_Memset(buf, 0, to_read);
            rc = to_read;
        }
        else {
            if (!file->data) { /* if file isn't open, return EOF */
                break;
            }

            BDBG_MSG((">reading [%u/%u:%u] (%u/%u/%u) %#lx", (unsigned)result, file->chunk, (unsigned)file->cur_pos, (unsigned)to_read, (unsigned)length, (unsigned)file->chunk_budget, (unsigned long)buf));
            rc = file->data->file.data->read(file->data->file.data, buf, to_read);
            BDBG_MSG(("<reading [%u/%u:%u] (%u/%u/%u) %#lx->%d", (unsigned)result, file->chunk, (unsigned)file->cur_pos, (unsigned)to_read, (unsigned)length, (unsigned)file->chunk_budget, (unsigned long)buf, (int)rc));
            if (rc<=0) {
                if  (result==0) {
                    result = rc; /* propagate error */
                }
                break;
            }
        }
        result += rc;
        length -= (size_t) rc;
        BDBG_ASSERT((size_t)rc <= file->chunk_budget);
        file->chunk_budget -=  (size_t)rc;
        file->cur_pos += (off_t)rc;
        if ((size_t)rc<to_read) { /* if we read less than asked, return right the way */
            break;
        }
        buf = (uint8_t *)buf + rc;
    }
    BDBG_MSG(("<read %d", (int)result));
    return result;
}


static int
chunk_bounds(bfile_io_read_t fd, off_t *first, off_t *last)
{
    struct bfile_io_read_chunk *file = (struct bfile_io_read_chunk *) fd;

    *first = file->settings.firstChunkNumber * file->settings.chunkSize;

    BDBG_MSG((">bounds %p", (void*)fd));
    for(;;) {
        off_t size, chunk_first;
        if (!file->size.last_file_open) {
            char name[B_MAX_FILE_LEN];
            int rc;
            get_chunk_name(name, sizeof(name), file->settings.chunkTemplate, file->base_name, file->size.last_chunk);
            rc = bfile_io_read_posix_open(&file->size.last_file, name, false);
            if (rc) {
                *last = file->size.last_chunk_begin;
                goto done;
            }
            file->size.last_file_open = true;
        }
        file->size.last_file.self.bounds(&file->size.last_file.self, &chunk_first, &size);
        if (size!=file->settings.chunkSize) {
            *last = file->size.last_chunk_begin+size;
            goto done;
        }
        /* advance to the next file */
        bfile_io_read_posix_close(&file->size.last_file);
        file->size.last_file_open = false;
        file->size.last_chunk_begin += size;
        file->size.last_chunk++;
        BDBG_MSG(("<bounds next %u", file->size.last_chunk));
    }
done:
    BDBG_MSG(("<bounds %u(%u) %u(%u)", (unsigned)*first, (unsigned)(*first/file->settings.chunkSize), (unsigned)*last, (unsigned)(*last/file->settings.chunkSize)));
    return 0;
}

static off_t
chunk_seek(bfile_io_read_t fd, off_t offset, int whence)
{
    struct bfile_io_read_chunk *file = (struct bfile_io_read_chunk *) fd;
    NEXUS_FilePlayHandle data=NULL;
    unsigned dest_chunk;
    off_t off,size;


    /* same assumptions as in chunk_size */

    /* we might need to serialize with read, however because only not
     * serialized call is seek(0,SEEK_CUR) we shall be safe in most cases,
     * so leave it for now */

    BDBG_MSG((">seek %p %lu %d", (void *)fd, (unsigned long)offset, whence));
    chunk_bounds(fd, &off, &size);

    switch(whence) {
    case SEEK_CUR:
        offset = file->cur_pos + offset;
        break;
    case SEEK_END:
        offset = size + offset;
        break;
    case SEEK_SET:
        break;
    default:
        BDBG_ERR(("unknown seek whence %d", whence));
        return -1;
    }
    if (offset == file->cur_pos) {
        return offset;
    }
    if (offset < 0 || offset >= size) {
        BDBG_WRN(("<seek out of bounds %d..%d..%d", (unsigned)off, (int)offset, (int)size));
        return -1;
    }
    /* we don't use 64 bit mult and divide because they are known to be buggy */
    for(dest_chunk=0,off=0;offset-off>=(off_t)file->settings.chunkSize;off+=file->settings.chunkSize) {
        dest_chunk++;
    }
    /* we know chunk at this point */
    if (dest_chunk < file->settings.firstChunkNumber) {
        /* if before the first chunk, we will read zeroes */
        file->chunk_budget = file->settings.chunkSize - (offset % file->settings.chunkSize);
        goto done;
    }
    else if (dest_chunk == file->chunk && file->data) {
        /* use already opened file */
        data = file->data;
    } else {
        data = chunk_read_open(file, dest_chunk);
    }
    if (data==NULL) {
        if (off == offset) { /* try to open next time */
            file->chunk_budget = 0 ;
            off = 0;
            goto done;
        } else { /* file was lost */
            BDBG_WRN(("<seek %u -> error", (unsigned)offset));
            return -1;
        }
    }
    off = offset - off;
    off = data->file.data->seek(data->file.data, off, SEEK_SET);
    file->chunk_budget = file->settings.chunkSize - off;
done:
    if (file->data &&  data != file->data ) {
        /* close old file */
        file->data->file.close(file->data);
    }
    file->data = data;
    file->chunk = dest_chunk;
    file->cur_pos = offset;
    BDBG_MSG(("<seek %u -> %u:[%u/%u]", (unsigned)offset, file->chunk, (unsigned)file->chunk_budget, (unsigned)off));
    return offset;
}

/* impl of NEXUS_ChunkedFilePlay */
struct bfile_in_chunk {
    struct NEXUS_FilePlay self;
    struct bfile_io_read_chunk data;
    struct NEXUS_FilePlay *index;
};

static void
chunk_close_in(NEXUS_FilePlayHandle f)
{
    struct bfile_in_chunk *file =  (struct bfile_in_chunk *)f;

    if (file->data.data) {
        file->data.data->file.close(file->data.data);
    }
    if (file->data.size.last_file_open) {
        bfile_io_read_posix_close(&file->data.size.last_file);
        file->data.size.last_file_open = false;
    }
    if (file->index) {
        file->index->file.close(file->index);
    }
    BKNI_Free(file);
    return;
}

NEXUS_FilePlayHandle
NEXUS_ChunkedFilePlay_Open(const char *fname, const char *indexname, const NEXUS_ChunkedFilePlayOpenSettings *pSettings)
{
    struct bfile_in_chunk *file;
    char name[B_MAX_FILE_LEN];
    NEXUS_ChunkedFilePlayOpenSettings openSettings;

    if (!fname) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    file = BKNI_Malloc(sizeof(*file));
    if (!file) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    if ( NULL == pSettings )
    {
        NEXUS_ChunkedFilePlay_GetDefaultOpenSettings(&openSettings);
        pSettings = &openSettings;
    }
    file->data.settings = *pSettings;

    b_strncpy(file->data.base_name, fname, sizeof(file->data.base_name));
    file->data.chunk = pSettings->firstChunkNumber;
    file->data.data = NULL;
    file->data.chunk_budget = 0; /* it would trigger open of data file on first call to write */
    file->data.cur_pos = 0;
    file->data.size.last_chunk = pSettings->firstChunkNumber;
    file->data.size.last_chunk_begin = (off_t)pSettings->firstChunkNumber * pSettings->chunkSize;
    file->data.size.last_file_open = false;

    get_chunk_name(name, sizeof(name), file->data.settings.chunkTemplate, file->data.base_name, file->data.chunk);
    file->data.open_in = NEXUS_FilePlay_OpenPosix;

    file->index = file->data.open_in(name, indexname);
    if (!file->index) {
        BKNI_Free(file);
        return NULL;
    }
    /* wire things together */
    file->self.file.index = file->index->file.index;
    file->self.file.close = chunk_close_in;
    file->data.self = *file->index->file.data;
    file->data.self.read = chunk_read;
    file->data.self.seek = chunk_seek;
    file->data.self.bounds = chunk_bounds;
    file->self.file.data = &file->data.self;
    return &file->self;
}
