/***************************************************************************
 *     (c)2007-2010 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 **************************************************************************/
#include "nexus_file_module.h"
#include "bfile_types.h"
#include "bfile_io.h"
#include "nexus_file_pvr.h"
#include "nexus_file_posix.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stat.h>

#include <ioLib.h>
#include <taskLib.h>
#include <dosFsLib.h>

BDBG_MODULE(nexus_file_posix);

static ssize_t
posix_read(bfile_io_read_t fd, void *buf, size_t length)
{
    struct bfile_io_read_posix *file = (struct bfile_io_read_posix *) fd;
    ssize_t rc;

    rc = read(file->fd, buf, length);
    if (file->direct) NEXUS_FlushCache((void *)buf, length);
    if (rc<0) errno = EIO; /* everything is just an error */
    return rc;
}

static off_t
posix_seek(bfile_io_read_t fd, off_t offset, int whence)
{
    struct bfile_io_read_posix *file = (struct bfile_io_read_posix *) fd;
    return bfile_io_seek(file->fd,offset,whence);
}

static int
posix_bounds(bfile_io_read_t fd, off_t *first, off_t *last)
{
    struct bfile_io_read_posix *file = (struct bfile_io_read_posix *) fd;
    off_t size = bfile_io_size(file->fd);

    *first = 0;
    if (size < 0) {
        *last = 0;
        return -1;
    } else {
        *last = size;
        return 0;
    }
}

static const struct bfile_io_read posix_io_read = {
    posix_read,
    posix_seek,
    posix_bounds,
    BIO_DEFAULT_PRIORITY
};

NEXUS_Error
bfile_io_read_posix_open(struct bfile_io_read_posix *file, const char *fname, bool direct)
{
    unsigned flags = O_RDONLY;
    NEXUS_Error rc;

    BDBG_ASSERT(file);
    BDBG_ASSERT(fname);

    file->direct = direct;
#ifdef DIRECT_IO_SUPPORT
    if (direct) flags |= O_DIRECT;
#endif
    file->fd = open(fname, flags, 0666);
#ifdef DIRECT_IO_SUPPORT
    if (file->fd < 0) {
        if (file->direct) {
            BDBG_WRN(("Error during O_DIRECT read open; removing O_DIRECT flag. Performance will suffer greatly."));
            flags &= ~O_DIRECT;
            file->direct = false;
        }
        file->fd = open(fname, flags, 0666);
    }
#endif
    if (file->fd < 0) { rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto err_open; }
    file->self = posix_io_read;
    return NEXUS_SUCCESS;

err_open:
    return rc;
}

void
bfile_io_read_posix_close(struct bfile_io_read_posix *file)
{
    if (file->fd > 0) {
        close(file->fd);
        file->fd = -1;
    }
    return;
}


static ssize_t
posix_write(bfile_io_write_t fd, const void *buf, size_t length)
{
    struct bfile_io_write_posix *file = (struct bfile_io_write_posix *) fd;
    ssize_t rc;

#ifdef FILEIO_TIMING
    struct timeval diff, end, begin,tmp,prev;
    gettimeofday(&begin, NULL);
    prev = file->st.cur;
#endif

    rc = write(file->fd, (void*)buf, length);

    if (rc < 0) {
        /* map the returned vxworks errno values to something more posix like for these specific errors */
        errno = (errno == S_dosFsLib_INVALID_PARAMETER || errno == S_dosFsLib_DISK_FULL || errno == S_dosFsLib_NO_CONTIG_SPACE) ? ENOSPC : EIO;
        BDBG_ERR(("%s:%d: Critical file error!", __FILE__, __LINE__));
    }

#ifdef FILEIO_TIMING
    gettimeofday(&end, NULL);
    timersub(&end, &begin, &diff);
    timeradd(&file->st.cur, &diff, &tmp);
    file->st.cur = tmp;
    if (diff.tv_usec > 50000 || diff.tv_sec > 0) {
        int msec_total, msec;
        fprintf(stderr, "(%p) bs=%d WHOA! time=%ld msec, # of good=%d", &file->st, length, diff.tv_usec/1000, file->st.good_cnt);
        timersub(&prev, &file->st.last_good, &diff);
        msec = (diff.tv_usec/1000) + (diff.tv_sec*1000);
        if (msec<=0) msec=1;
        timersub(&file->st.cur, &file->st.begin, &diff);
        msec_total = (diff.tv_usec/1000) + (diff.tv_sec*1000);
        fprintf(stderr, "(from good %d msec %dKBytes/sec, total %dKBytes/sec)\n", msec,
               (int)(((long long)(length/1000)*(file->st.good_cnt)*1024)/(msec)),
               (int)(((long long)(length/1000)*(file->st.writes+1)*1024)/(msec_total))
               );
          file->st.good_cnt=0;
          file->st.last_good=file->st.cur;
    } else {
        file->st.good_cnt++;
    }
    file->st.writes++;
#endif
    return rc;
}

static off_t
posix_trim(bfile_io_write_t fd, off_t trim_pos)
{
    BSTD_UNUSED(fd);
    BSTD_UNUSED(trim_pos);
    return 0;
}

off_t
bfile_io_write_posix_seek(struct bfile_io_write_posix *file, off_t offset, int whence)
{
    BDBG_ASSERT(file);
    return bfile_io_seek(file->fd,offset,whence);
}

static const struct bfile_io_write posix_io_write = {
    posix_write,
    posix_trim,
    BIO_DEFAULT_PRIORITY
};

struct bfile_out_posix {
    struct NEXUS_FileRecord self;
    struct bfile_io_write_posix data, index;
};

static NEXUS_Error
b_file_io_write_posix_open(struct bfile_io_write_posix *file, const char *fname, unsigned flags, bool direct)
{
    NEXUS_Error rc;

    BDBG_ASSERT(file);
    BDBG_ASSERT(fname);

    file->direct = direct;
#ifdef DIRECT_IO_SUPPORT
    if (direct) flags |= O_DIRECT;
#endif
    file->fd = open(fname, flags, 0666);
#ifdef DIRECT_IO_SUPPORT
    if (file->fd < 0) {
        if (file->direct) {
            BDBG_WRN(("Error during O_DIRECT write open; removing O_DIRECT flag. Performance will suffer greatly."));
            flags &= ~O_DIRECT;
            file->direct = false;
        }
        file->fd = open(fname, flags, 0666);
    }
#endif
    if (file->fd < 0) { rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto err_open; }
    file->self = posix_io_write;

#ifdef FILEIO_TIMING
    file->st.begin.tv_usec = file->st.begin.tv_sec = 0;
    file->st.cur=file->st.last_good=file->data.st.begin;
    file->st.writes = file->st.good_cnt = 0;
#endif
    return NEXUS_SUCCESS;

err_open:
    return rc;
}

NEXUS_Error
bfile_io_write_posix_open(struct bfile_io_write_posix *file, const char *fname, bool direct)
{
    return b_file_io_write_posix_open(file, fname, O_CREAT|O_TRUNC|O_WRONLY, direct);
}

NEXUS_Error
bfile_io_write_posix_append(struct bfile_io_write_posix *file, const char *fname, bool direct)
{
    return b_file_io_write_posix_open(file, fname, O_CREAT|O_APPEND|O_WRONLY, direct);
}

void
bfile_io_write_posix_close(struct bfile_io_write_posix *file)
{
    if (file->fd > 0) {
        close(file->fd);
        file->fd = -1;
    }
    return;
}

static void
posix_close_out(struct NEXUS_FileRecord *file)
{
    struct bfile_out_posix *posix=  (struct bfile_out_posix *)file;

    BDBG_MSG(("close_out %#x", (unsigned)file));
    if (file->index)
        bfile_io_write_posix_close(&posix->index);
    bfile_io_write_posix_close(&posix->data);
    BKNI_Free(posix);
    return;
}

NEXUS_FileRecordHandle
NEXUS_FileRecord_OpenPosix(const char *fname, const char *indexname)
{
    NEXUS_Error rc;
    struct bfile_out_posix *file =  BKNI_Malloc(sizeof(*file));

    if (!file) { rc  = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_alloc; }

    if (fname) {
        rc = bfile_io_write_posix_open(&file->data, fname, true);
        if (rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto err_open_data; }
        file->self.data = &file->data.self;
    } else {
        file->self.data = NULL;
    }

    if (indexname) {
        rc = bfile_io_write_posix_open(&file->index, indexname, false);
        if (rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto err_open_index; }
        file->self.index = &file->index.self;
    } else {
        file->self.index = NULL;
    }

    file->self.close = posix_close_out;
    return &file->self;

err_open_index:
    close(file->data.fd);
err_open_data:
    BKNI_Free(file);
err_alloc:
    return NULL;
}

NEXUS_FileRecordHandle
NEXUS_FileRecord_AppendPosix(const char *fname, const char *indexname)
{
    NEXUS_Error rc;
    struct bfile_out_posix *file;

    file =  BKNI_Malloc(sizeof(*file));
    if (!file) { rc  = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_alloc; }

    if (fname) {
        rc = bfile_io_write_posix_append(&file->data, fname, true);
        if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto err_open_data; }
        file->self.data = &file->data.self;
    } else {
        file->self.data = NULL;
    }

    if (indexname) {
        rc = bfile_io_write_posix_append(&file->index, indexname, false);
        if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto err_open_index; }
        file->self.index = &file->index.self;
    } else {
        file->self.index = NULL;
    }

    file->self.close = posix_close_out;
    return &file->self;

err_open_index:
    close(file->data.fd);
err_open_data:
    BKNI_Free(file);
err_alloc:
    return NULL;
}

struct bfile_in_posix {
    struct NEXUS_FilePlay self;
    struct bfile_io_read_posix data, index;
};

static void
posix_close_in(struct NEXUS_FilePlay *file)
{
    struct bfile_in_posix *posix=  (struct bfile_in_posix *)file;

    BDBG_MSG(("close_in %#x", (unsigned)file));
    if (file->file.index)
        bfile_io_read_posix_close(&posix->index);
    bfile_io_read_posix_close(&posix->data);
    BKNI_Free(posix);
    return;
}


NEXUS_FilePlayHandle
NEXUS_FilePlay_OpenPosix(const char *fname, const char *indexname)
{
    NEXUS_Error rc;
    struct bfile_in_posix *file =  BKNI_Malloc(sizeof(*file));

    if (!file) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_alloc; }

    rc = bfile_io_read_posix_open(&file->data, fname, true);
    if (rc!=NEXUS_SUCCESS) {
        BDBG_ERR(("Unable to open input file: '%s'", fname));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_open_data;
    }

    if (indexname) {
        rc = bfile_io_read_posix_open(&file->index, indexname, false);
        if (rc!=NEXUS_SUCCESS) {
            BDBG_ERR(("Unable to open index file: '%s'", indexname));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto err_open_index;
        }
        file->self.file.index = &file->index.self;
    } else {
        file->self.file.index = NULL;
    }

    file->self.file.data = &file->data.self;
    file->self.file.close = posix_close_in;
    return &file->self;

err_open_index:
    bfile_io_read_posix_close(&file->data);
err_open_data:
    BKNI_Free(file);
err_alloc:
    return NULL;
}

void
NEXUS_FilePlay_Close(NEXUS_FilePlayHandle file)
{
    BDBG_ASSERT(file);
    file->file.close(file);
    return;
}

void
NEXUS_FileRecord_Close(NEXUS_FileRecordHandle file)
{
    BDBG_ASSERT(file);
    file->close(file);
    return;
}

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

typedef struct stat64
{
    uint32_t  st_dev;         /* device ID number */
    uint32_t  st_ino;         /* file serial number */
    uint16_t  st_mode;        /* file mode (see below) */
    int16_t   st_nlink;       /* number of links to file */
    int16_t  st_uid;         /* user ID of file's owner */
    int16_t  st_gid;         /* group ID of file's group */
    uint32_t  st_rdev;        /* device ID, only if special file */
    off_t   st_size;        /* size of file, in bytes */
    uint32_t  st_atime;       /* time of last access */
    uint32_t  st_mtime;       /* time of last modification */
    uint32_t  st_ctime;       /* time of last change of file status */
    int     st_blksize;
    int     st_blocks;
    uint8_t   st_attrib;      /* file attribute byte (dosFs only) */
    int     reserved1;      /* reserved for future use */
    int     reserved2;      /* reserved for future use */
    int     reserved3;      /* reserved for future use */
    int     reserved4;      /* reserved for future use */
    int     reserved5;      /* reserved for future use */
}stat64;

/*
** Note: the following two functions are replacements for the standard library lseek and fstat functions
** that return a 64-bit size value.  Since they are called in place of the standard library functions they
** need to "act" like the standard library functions and return appropriate errno values.
** Also note that you can't use the error print system here because they may be called before the error print system is initialized.
*/

#include <errno.h>

static off_t lseek64(int fd, off_t offset, int whence)
{
    int status;
    off_t newPosition;

    switch (whence)
    {
    case SEEK_SET:
        newPosition = offset;
        break;

    case SEEK_CUR:
    {
        off_t  currPosition;
        
        /* Get the current file position */
        status = ioctl(fd, FIOWHERE64, (int)&currPosition);
        if (status != OK)
        {
            errno = EFAULT;
            return -1;
        }
        newPosition = currPosition + offset;
        break;
    }

    case SEEK_END:
    {
        off_t zero = 0;
        off_t  fileSize;
        
        /* Seek to start of file */
        status = ioctl(fd, FIOSEEK64, (int)&zero);
        if (status != OK)
        {
            errno = EFAULT;
            return -1;
        }

        /* Get number of bytes in file */
        status = ioctl(fd, FIONREAD64, (int)&fileSize);
        if (status != OK)
        {    
            errno = EFAULT;
            return -1;
        }
        newPosition = fileSize + offset;
        break;
    }

    default:
        errno = EACCES;
        return -1;
    }

    /* Seek to desired file position */
    status = ioctl(fd, FIOSEEK64, (int)&newPosition);
    if (status != OK)
    {
        errno = EFAULT;
        return -1;
    }

    return(newPosition);
}

static int fstat64(int  fd, stat64 *pStat)
{
    int status;
    struct stat Stat;
    off_t  filePosSave;
    off_t  zero = 0;
    off_t size;
    
    memset(&Stat, 0, sizeof(Stat));

    /* Get file information using regular stat function */
    status = fstat(fd, &Stat);
    if (status != OK)
    {
        /* no need to set a errno value -- should already be set. */
        return(ERROR);
    }

    /* Copy the stat info to the 64bit version of the stat struct */
    pStat->st_dev       = Stat.st_dev;
    pStat->st_mode      = Stat.st_mode;
    pStat->st_nlink     = Stat.st_nlink;
    pStat->st_size      = Stat.st_size;        /* Will override below */
    pStat->st_atime     = Stat.st_atime;
    pStat->st_mtime     = Stat.st_mtime;
    pStat->st_ctime     = Stat.st_ctime;
    pStat->st_blksize   = Stat.st_blksize;
    pStat->st_blocks    = Stat.st_blocks;
    pStat->st_attrib    = Stat.st_attrib;

    /* For directories, assume size is less than 4GB */
    if (S_ISDIR(Stat.st_mode))
        return(OK);

    /* Save current position */
    status = ioctl(fd, FIOWHERE64, (int)&filePosSave);
    if (status != OK)
        goto error;

    /* Seek to start of file */
    status = ioctl(fd, FIOSEEK64, (int)&zero);
    if (status != OK)
        goto error;

    /* Get unread bytes (same as file size) */
    status = ioctl(fd, FIONREAD64, (int)&size);
    if (status != OK)
        goto error;

    /* Restore file position */
    status = ioctl(fd, FIOSEEK64, (int)&filePosSave);
    if (status != OK)
        goto error;

    pStat->st_size = size; /* only set the size IF everything worked! */
    return(OK);

  error:
    errno = EFAULT; /* this is a general purpose "it didn't work" error value. */
    return(ERROR);
}

off_t bfile_io_size(int desc)
{
    stat64 sb;
    struct stat sb2;
 
    /* This can get called for files other than "/ata0" or "/bd0" (new dosFs) files.
    ** We don't know from the descriptor whether it is or not, so we try one then the other.
    */
    if (fstat64(desc, &sb) != ERROR)
        return sb.st_size;
    if (fstat(desc, &sb2) >= 0)
        return sb2.st_size;
    return -1;
}

off_t bfile_io_seek(int desc, off_t offset, int whence)
{
    return lseek64(desc, offset, whence);
}


