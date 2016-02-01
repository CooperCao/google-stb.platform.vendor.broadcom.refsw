/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
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
 ****************************************************************************/

#include "b_dvr_file.h"
#include "string.h"
#include "errno.h"
#include "b_dvr_manager_priv.h"

BDBG_MODULE(b_dvr_file);

static ssize_t posix_read(bfile_io_read_t fd, void *buf, size_t length)
{
    B_DVR_File *dvrFile = (B_DVR_File *) fd;
    ssize_t rc;
    BDBG_MSG(("posix_read %s",dvrFile->fileName));
    if(dvrFile->directIO) 
    {
#if 0    
        rc = cacheflush((void *)buf, length, DCACHE);
        if (rc<0) 
        {
            BDBG_ERR(("cacheflush has returned error %d, addr %p, size %u, ignored", rc, buf, length));
        }
#endif
        NEXUS_FlushCache((void *)buf, length); 

    }
    if((length%B_DVR_IO_BLOCK_SIZE) && dvrFile->directIO) 
    {
        /*
         * Don't attempt direct IO read since the size to be read isn't direct IO aligned.
         */
        rc = -1;
    }
    else
    {
        rc = read(dvrFile->fd, buf, length);
        BDBG_MSG(("posix_read: %#lx %#lx %u:%u -> %d", (unsigned long)fd, 
                (unsigned long)buf, (unsigned)(lseek(dvrFile->fd, 0, SEEK_CUR)-rc),
                (unsigned)length, (int)rc));
    }

    if (rc<0 && dvrFile->directIO==true) 
    {
        int flags;
        if(!(length%B_DVR_IO_BLOCK_SIZE)) 
        {
            BDBG_WRN(("%s: unexpected Direct IO read failed %s buf=%lx, length=%d",
                      __FUNCTION__,
                      dvrFile->fileName, 
                      buf,length));
        }
        else
        {
            BDBG_MSG(("%s:expected Direct IO read failed %s buf=%lx, length=%d",
                      __FUNCTION__,
                      dvrFile->fileName, 
                      buf,length));
        }
        dvrFile->directIO=false;
        flags = fcntl(dvrFile->fd, F_GETFL);
        if (flags!=-1) 
        {
            flags &= ~O_DIRECT;
            if (fcntl(dvrFile->fd, F_SETFL, flags)==0) 
            {
                rc = read(dvrFile->fd, buf, length);
                goto read_done;
            }
        }
        BDBG_ERR(("Can't remove O_DIRECT flag"));
        BDBG_ERR(("err: %s %#lx %#lx %u:%u -> %d direct %u",dvrFile->fileName, (unsigned long)fd, 
                 (unsigned long)buf, (unsigned)(lseek(dvrFile->fd, 0, SEEK_CUR)-rc),
                 (unsigned)length, (int)rc,(unsigned)dvrFile->directIO));
    }
read_done:
    return rc;
}

static off_t posix_seek(bfile_io_read_t fd, off_t offset, int whence)
{
    B_DVR_File *dvrFile = (B_DVR_File *) fd;
    off_t off;
    off = lseek(dvrFile->fd,offset,whence);
    BDBG_MSG(("posix_seek: %#lx %lu(%d) -> %ld", (unsigned long)fd, (unsigned long)offset, whence, (long)off));
    return off;
}


static int posix_bounds(bfile_io_read_t fd, off_t *first, off_t *last)
{
    struct B_DVR_File *dvrFile = (B_DVR_File *) fd;
    struct stat sb;
    *first = 0;
    *last = 0;
    if (fstat(dvrFile->fd, &sb)<0) 
    {
        return -1;
    }
    *last=sb.st_size;
    return 0;
}

static const struct bfile_io_read posix_io_read = 
{
    posix_read,
    posix_seek,
    posix_bounds,
    BIO_DEFAULT_PRIORITY
};


static ssize_t posix_write(bfile_io_write_t fd, const void *buf, size_t length)
{
    B_DVR_File *dvrFile = (B_DVR_File*) fd;
    ssize_t rc;
    rc = write(dvrFile->fd, (void*)buf, length);
   /* BDBG_MSG(("posix_write: %#lx %#lx %u:%u -> %d", (unsigned long)fd, (unsigned long)buf, (unsigned)(lseek(dvrFile->fd, 0, SEEK_CUR)-rc), (unsigned)length, (int)rc));*/
    if (rc < 0 && (errno == ENOSPC || errno == EIO)) 
    {
        BDBG_ERR(("%s:%d: Critical file error! %s", __FILE__, __LINE__,strerror(errno)));
    }
    return rc;
}

static const struct bfile_io_write posix_io_write =
{
    posix_write,
    NULL,
    BIO_DEFAULT_PRIORITY
};

void B_DVR_File_Close(B_DVR_FileHandle dvrFile)
{
    BDBG_MSG(("%s: %s",__FUNCTION__,dvrFile->fileName));
    if(dvrFile->fd>0) {
        close(dvrFile->fd);
        dvrFile->fd = -1;
    }
    return;
}

B_DVR_ERROR B_DVR_File_Open(B_DVR_FileHandle dvrFile, const char *fileName, B_DVR_FileSettings *pSettings)
{
    unsigned flags;
    B_DVR_ERROR rc=B_DVR_SUCCESS;

    BDBG_ASSERT(dvrFile);
    BDBG_ASSERT(fileName);
    if(pSettings->fileIO == eB_DVR_FileIOWrite)
    {
        flags = O_WRONLY;
        if(pSettings->create)
        {
            flags|= O_CREAT | O_EXCL;
        }
    }
    else
    {
        flags = O_RDONLY;
    }
    dvrFile->directIO = pSettings->directIO;
    dvrFile->fileIO = pSettings->fileIO;
    flags |= O_LARGEFILE;
    if(pSettings->directIO) 
    {
        #if BCHP_CHIP != 7145
        flags |= O_DIRECT;
        #else
        BDBG_ERR(("WARNING:Disabling Direct I/O"));
        #endif
    }
    dvrFile->fd = open(fileName, flags|O_CLOEXEC, 0666);
    strncpy(dvrFile->fileName,fileName,B_DVR_MAX_FILE_NAME_LENGTH);
    dvrFile->fileName[B_DVR_MAX_FILE_NAME_LENGTH-1] = '\0';
    BDBG_MSG(("%s: %s %d",__FUNCTION__,dvrFile->fileName,(unsigned)dvrFile->directIO));
    if (dvrFile->fd<0)
    {
        BDBG_ERR(("%s: %s %d",__FUNCTION__,dvrFile->fileName,(unsigned)dvrFile->directIO));
        BDBG_ERR(("B_DVR_File_Open : File Open failed :%s",strerror(errno)));
        rc = B_DVR_NOT_SUPPORTED;
        goto err_open;
    } 
    else 
    {    
	    BDBG_MSG(("%s: %s %d",__FUNCTION__,dvrFile->fileName,(unsigned)dvrFile->directIO));
    }	    

    switch(dvrFile->fileIO)
    {
    case eB_DVR_FileIORead :
        {
           dvrFile->fileInterface.io.readFD = posix_io_read;
           break;
        }
    case eB_DVR_FileIOWrite:
        {
            dvrFile->fileInterface.io.writeFD = posix_io_write;
            break;
        }
    }
    BDBG_MSG(("%s:success %d",__FUNCTION__,rc));
    return B_DVR_SUCCESS;
    BDBG_MSG(("%s:fail %d",__FUNCTION__,rc));
err_open:
    return rc;
}

