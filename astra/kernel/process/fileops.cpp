/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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

/*
 * fileops.cpp
 *
 *  Created on: Feb 23, 2015
 *      Author: gambhire
 */

#include "tzfcntl.h"
#include "arm/arm.h"
#include "arm/spinlock.h"

#include "system.h"
#include "tztask.h"
#include "tzmemory.h"
#include "tzmman.h"
#include "objalloc.h"
#include "kernel.h"
#include "scheduler.h"
#include "console.h"

#include "lib_printf.h"

#include "fs/ramfs.h"

static const char *extractEndPoint(char *path) {
    // First remove any trailing slashes from the path.
    int pathLen = strlen(path);
    while (path[pathLen-1] == '/') {
        path[pathLen-1] = 0;
        pathLen = strlen(path);
    }

    // Now seek to the next slash.
    const char *endPoint = path + pathLen;
    while ((endPoint != path) && (endPoint[-1] != '/'))
        endPoint--;

    return endPoint;
}

void TzTask::initFileTable() {
    PageTable *kernelPageTable = PageTable::kernelPageTable();

    // Allocate the file descriptor array on the heap
    int numPages = (sizeof(FileTableEntry) * MAX_FD)/PAGE_SIZE_4K_BYTES + 1;
    TzMem::VirtAddr va = kernelPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, numPages*PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (va == nullptr) {
        err_msg("Ran out of user virtual address space\n");
        System::halt();
    }

    files = (FileTableEntry *)va;

    for (int i=0; i<numPages; i++) {
        TzMem::PhysAddr pa = TzMem::allocPage(tid);
        if (pa == nullptr) {
            err_msg("Out of memory\n");
            System::halt();
        }

        kernelPageTable->mapPage(va, pa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true, false);
        va = (uint8_t *)va + PAGE_SIZE_4K_BYTES;
    }


    // Initialize the std file descriptors
    files[0] = {File, Console::stdin, 0, true, false, false};
    files[1] = {File, Console::stdout, 0, false, true, false};
    files[2] = {File, Console::stderr, 0, false, true, false};
    files[3] = {File, Console::stdlog, 0, false, true, false};

    // Clear the rest of file descriptors
    for (int i=4; i<MAX_FD; i++) {
        files[i].data = nullptr;
        files[i].offset = 0;
        files[i].closeOnExec = false;
    }

    filesCloned = false;
}

void TzTask::inheritFileTable(const TzTask& parentTask) {

    // Initialize the file table
    initFileTable();

    // Copy the rest of file descriptors
    for (int i=4; i<MAX_FD; i++) {
        if (parentTask.files[i].data == nullptr)
            continue;

        memcpy(&files[i], &parentTask.files[i], sizeof(FileTableEntry));

        // Open file and directory if necessary
        if (files[i].type == File)
            RamFS::File::open((RamFS::File *)files[i].file);
        if (files[i].type == Directory)
            RamFS::Directory::open((RamFS::Directory *)files[i].dir);
    }
}

int TzTask::access(char *filePath, int mode) {
    IDirectory *dir = nullptr;
    IFile *file = nullptr;

    IDirectory *root = System::root();

    // Attempt to open the filePath relative to the current
    // working directory.
    int rc = currWorkDir->resolvePath(filePath, &dir, &file);
    if (rc != 0) {
        // Perhaps the filePath is an absolute path.
        // Attempt to open the filePath as is.
        rc = root->resolvePath(filePath, &dir, &file);
    }

    if (rc != 0)
        return rc;


    rc = 0;

    int perms;
    uint16_t owner;
    uint16_t group;
    if (file != nullptr) {
        perms = file->permissions();
        owner = file->owner();
        group = file->group();
    }
    else {
        perms = dir->permissions();
        owner = dir->owner();
        group = dir->group();
    }

    if ((owner == uid) || (uid == System::UID)) {
        if ((mode & R_OK) && (!(OWNER_ACCESS_PERMS(perms) & PERMS_READ_BIT)))
            rc = -EACCES;
        if ((mode & W_OK) && (!(OWNER_ACCESS_PERMS(perms) & PERMS_WRITE_BIT)))
            rc = -EACCES;
        if ((mode & X_OK) && (!(OWNER_ACCESS_PERMS(perms) & PERMS_EXECUTE_BIT)))
            rc = -EACCES;
    }
    else if (group == gid) {
        if ((mode & R_OK) && (!(GROUP_ACCESS_PERMS(perms) & PERMS_READ_BIT)))
            rc = -EACCES;
        if ((mode & W_OK) && (!(GROUP_ACCESS_PERMS(perms) & PERMS_WRITE_BIT)))
            rc = -EACCES;
        if ((mode & X_OK) && (!(GROUP_ACCESS_PERMS(perms) & PERMS_EXECUTE_BIT)))
            rc = -EACCES;
    }
    else {
        if ((mode & R_OK) && (!(OTHERS_ACCESS_PERMS(perms) & PERMS_READ_BIT)))
            rc = -EACCES;
        if ((mode & W_OK) && (!(OTHERS_ACCESS_PERMS(perms) & PERMS_WRITE_BIT)))
            rc = -EACCES;
        if ((mode & X_OK) && (!(OTHERS_ACCESS_PERMS(perms) & PERMS_EXECUTE_BIT)))
            rc = -EACCES;
    }

    return rc;

}

int TzTask::open(char *filePath, int flags, int mode) {

    IDirectory *dir = nullptr;
    IFile *file = nullptr;
    IDirectory *parentDir = nullptr;

    IDirectory *root = System::root();

    // Make sure there's a free file descriptor slot
    int fd = 0;
    while ((fd < MAX_FD) && (files[fd].data != nullptr))
        fd++;
    if (fd == MAX_FD)
        return -ENFILE;

    // Interpret the flags
    bool createFile = (flags & (O_CREAT | O_TMPFILE));
    bool exclusive = (flags & O_EXCL);
    bool readFile = ((flags & O_RW_MASK) == O_RDONLY) || (flags & O_RDWR);
    bool writeFile = ((flags & O_WRONLY) || (flags & O_RDWR));
    bool appendFile = (flags & (O_WRONLY | O_RDWR)) && (flags & O_APPEND);
    bool truncateFile = (flags & (O_WRONLY | O_RDWR)) && (flags & O_TRUNC);
    bool closeOnExec = (flags & O_CLOEXEC);
    bool dirOnly = (flags & O_DIRECTORY);

    //printf("---->createFile %d\n\treadFile %d\n\twriteFile %d\n\tappendFile %d\n\ttruncateFile %d\n\tcloseOnExec %d\n\tnoDir %d\n",
        //      createFile, readFile, writeFile, appendFile, truncateFile, closeOnExec, noDir);

    // Extract the file name from the path.
    const char *fileName = extractEndPoint(filePath);
    //printf("Extract End Point %s from %s \n",fileName, filePath);

    // Attempt to open the filePath relative to the current
    // working directory.
    int rc = currWorkDir->resolvePath(filePath, &dir, &file, &parentDir);
    if (rc != 0) {
        // Perhaps the filePath is an absolute path.
        // Attempt to open the filePath as is.
        rc = root->resolvePath(filePath, &dir, &file, &parentDir);
        //printf("Path Resolved from root %d\n",rc);
    }

     //printf("Open: resolve path result %d dir %p file %p parentDir %p\n", rc, dir, file, parentDir);

    // If path did not resolve and creation flags are not set, fail.
    if ((file == nullptr) && (dir == nullptr) && (!createFile))
        return -ENOENT;

    // If dirOnly and the path resolved to a non-directory, fail.
    if ((dirOnly) && (dir == nullptr))
        return -EISDIR;

    // If path resolved to directory, check flags for consistency
    // and add dir to the file table.
    if (dir != nullptr) {
        fd = openDir(dir, fd, closeOnExec);
        if(fd != 0)
            return fd;
    }

    // If path resolved and creation flags are set, fail.
    if ((file != nullptr) && (createFile)){
        if(exclusive)
            return -EEXIST;

        createFile = 0;
    }

    // If creation flags are set and directory did not resolve, fail
    if ((createFile) && (parentDir == nullptr))
        return -ENOTDIR;

    // If path resolved and permissions do not match, fail.
    if (file != nullptr) {
        int perms = file->permissions();
        uint16_t fileOwner = file->owner();
        uint16_t fileGroup = file->group();
        //printf("File Permission %x %x %x\n",OWNER_ACCESS_PERMS(perms), GROUP_ACCESS_PERMS(perms),OTHERS_ACCESS_PERMS(perms));

        if ((fileOwner == uid) || (uid == System::UID)) {
            if ((readFile) && (!(OWNER_ACCESS_PERMS(perms) & PERMS_READ_BIT)))
                    return -EACCES;
            if ((writeFile) && (!(OWNER_ACCESS_PERMS(perms) & PERMS_WRITE_BIT)))
                    return -EACCES;
        }
        else if (fileGroup == gid) {
            if ((readFile) && (!(GROUP_ACCESS_PERMS(perms) & PERMS_READ_BIT)))
                return -EACCES;
            if ((writeFile) && (!(GROUP_ACCESS_PERMS(perms) & PERMS_WRITE_BIT)))
                return -EACCES;
        }
        else {
            if ((readFile) && (!(OTHERS_ACCESS_PERMS(perms) & PERMS_READ_BIT)))
                return -EACCES;
            if ((writeFile) && (!(OTHERS_ACCESS_PERMS(perms) & PERMS_WRITE_BIT)))
                return -EACCES;
        }

    }

    if (createFile) {
        // Attempt file creation
        file = RamFS::File::create(parentDir->owner(), parentDir->group(), mode);
        if (file == nullptr)
            return -ENOMEM;

        rc = parentDir->addFile(fileName, file);
        if (rc != 0) {
            RamFS::File::destroy(file);
            return rc;
        }
    }

    // At this point we have a valid File.
    // Truncate it if specified.
    if (truncateFile)
        file->erase();

    // Work out the file offset
    uint64_t offset = 0;
    if (readFile)
        offset = 0;
    if (truncateFile)
        offset = 0;
    if (appendFile)
        offset = file->size();

    // Add it to the task's file descriptor table.
    files[fd].type = File;
    files[fd].file = file;
    files[fd].offset = offset;
    files[fd].read = readFile;
    files[fd].write = writeFile;
    files[fd].closeOnExec = closeOnExec;

    RamFS::File::open((RamFS::File *)file);
    return fd;
}

int TzTask::openDir(IDirectory *dir, int fd, bool closeOnExec) {

    int perms = dir->permissions();
    uint16_t dirOwner = dir->owner();
    uint16_t dirGroup = dir->group();

    if (uid != 0) {
        if (dirOwner == uid) {
            if (!(OWNER_ACCESS_PERMS(perms) & PERMS_READ_BIT))
                return -EACCES;

        }
        else if (dirGroup == gid) {
            if (!(GROUP_ACCESS_PERMS(perms) & PERMS_READ_BIT))
                return -EACCES;
        }
        else {
            if (!(OTHERS_ACCESS_PERMS(perms) & PERMS_READ_BIT))
                return -EACCES;
        }
    }

    files[fd].type = Directory;
    files[fd].dir = dir;
    files[fd].offset = 0;
    files[fd].read = true;
    files[fd].write = true;
    files[fd].closeOnExec = closeOnExec;

    RamFS::Directory::open((RamFS::Directory *)dir);

    return fd;
}

int TzTask::dup(int fd) {
    if ((fd >= MAX_FD) || (fd < 0))
        return -ENOENT;

    if (files[fd].data == nullptr)
        return -ENOENT;

    int nfd = 0;
    while ((nfd < MAX_FD) && (files[nfd].data != nullptr))
        nfd++;
    if (nfd == MAX_FD)
        return -ENFILE;

    memcpy(&files[nfd], &files[fd], sizeof(FileTableEntry));
    return nfd;
}

int TzTask::dup2(int fd1, int fd2) {
    if ((fd1 >= MAX_FD) || (fd1 < 0))
        return -ENOENT;
    if (files[fd1].data == nullptr)
        return -ENOENT;

    if ((fd2 >= MAX_FD) || (fd2 < 0))
        return -ENOENT;

    if (files[fd2].data != nullptr) {
        if (files[fd2].type == File)
            RamFS::File::close((RamFS::File *)files[fd2].file);
        if (files[fd2].type == Directory)
            RamFS::Directory::close((RamFS::Directory *)files[fd2].dir);
    }

    memcpy(&files[fd2], &files[fd1], sizeof(FileTableEntry));
    return fd2;
}

int TzTask::readdir(int fd, void *userBuffer, size_t size) {

    if ((fd >= MAX_FD) || (fd < 0))
        return -ENOENT;

    IDirectory *dir = nullptr;

    dir = files[fd].dir;
    int rv = dir->readDir(userBuffer, size);
    //printf("readdir rv %d\n",rv);
    return rv;
}

int TzTask::read(int fd, void *userBuffer, size_t size) {
    if ((fd >= MAX_FD) || (fd < 0))
        return -ENOENT;

    IFile *file = files[fd].file;
    if (file == nullptr)
        return -ENOENT;

    if (!files[fd].read)
        return -EBADF;

    int rv = file->read(userBuffer, size, files[fd].offset);
    if (rv > 0)
        files[fd].offset += rv;

    return rv;
}

int TzTask::write(int fd, void *userBuffer, size_t size) {
    if ((fd >= MAX_FD) || (fd < 0))
        return -ENOENT;

    IFile *file = files[fd].file;
    if (file == nullptr)
        return -ENOENT;

    if (!files[fd].write)
        return -EBADF;

    int rv = file->write(userBuffer, size, files[fd].offset);
    if (rv > 0)
        files[fd].offset += rv;

    return rv;
}

int TzTask::close(int fd) {
    if ((fd >= MAX_FD) || (fd < 0))
        return -ENOENT;

    if (files[fd].data == nullptr)
        return -ENOENT;

    if (files[fd].type == File)
        RamFS::File::close((RamFS::File *)files[fd].file);
    if (files[fd].type == Directory)
        RamFS::Directory::close((RamFS::Directory *)files[fd].dir);
    if (files[fd].type == MQueue)
        MsgQueue::close(files[fd].queue);

    files[fd].data = nullptr;
    files[fd].offset = 0;

    return 0;
}

int TzTask::llseek(int fd, uint64_t offset, loff_t *result, unsigned int whence) {
    if ((fd >= MAX_FD) || (fd < 0))
        return -ENOENT;

    if (files[fd].data == nullptr)
        return -ENOENT;

    if (files[fd].type != File)
        return -EISDIR;

    if (whence == SEEK_SET)
        files[fd].offset = offset;
    else if (whence == SEEK_CUR)
        files[fd].offset += offset;
    else
        files[fd].offset = files[fd].file->size() + offset;

    *result = files[fd].offset;
    return 0;
}

int TzTask::setCurrDir(const char *dirPath) {
    IDirectory *root = System::root();
    IDirectory *dir = nullptr;
    IFile *file = nullptr;

    int rc = root->resolvePath(dirPath, &dir, &file);
    if ((rc != 0) || (file != nullptr))
        rc = currWorkDir->resolvePath(dirPath, &dir, &file);

    if (rc != 0)
        return rc;

    if (file != nullptr)
        return -ENOTDIR;

    currWorkDir = dir;
    return 0;
}

int TzTask::setCurrDir(int fd) {
    if ((fd >= MAX_FD) || (fd < 0))
            return -ENOENT;

    if (files[fd].data == nullptr)
        return -ENOENT;

    if (files[fd].type != Directory)
        return -ENOTDIR;

    currWorkDir = files[fd].dir;
    return 0;
}

int TzTask::chmod(const char *path, uint32_t mode) {
    IDirectory *root = System::root();
    IDirectory *dir = nullptr;
    IFile *file = nullptr;

    int rc = root->resolvePath(path, &dir, &file);
    if ((rc != 0) || (file != nullptr))
        rc = currWorkDir->resolvePath(path, &dir, &file);

    if (rc != 0)
        return rc;

    if (file != nullptr)
        rc = file->changePermissions(mode);
    if (dir != nullptr)
        rc = dir->changePermissions(mode);

    return rc;
}

int TzTask::fchmod(int fd, uint32_t mode) {
    if ((fd >= MAX_FD) || (fd < 0))
        return -ENOENT;

    if (files[fd].data == nullptr)
        return -ENOENT;

    if (files[fd].type == Directory)
        return files[fd].dir->changePermissions(mode);

    return files[fd].file->changePermissions(mode);
}

int TzTask::chown(const char *path, uint16_t owner, uint16_t group) {
    IDirectory *root = System::root();
    IDirectory *dir = nullptr;
    IFile *file = nullptr;

    int rc = root->resolvePath(path, &dir, &file);
    if ((rc != 0) || (file != nullptr))
        rc = currWorkDir->resolvePath(path, &dir, &file);

    if (rc != 0)
        return rc;

    if (file != nullptr) {
        uint16_t oldGroup = file->group();
        rc = file->changeGroup(group);
        if (rc == 0) {
            rc = file->changeOwner(owner);
            if (rc != 0)
                file->changeGroup(oldGroup);
        }
        return rc;
    }

    if (dir != nullptr) {
        uint16_t oldGroup = dir->group();
        rc = dir->changeGroup(group);
        if (rc == 0) {
            rc = dir->changeOwner(owner);
            if (rc != 0)
                dir->changeGroup(oldGroup);
        }
        return rc;
    }

    return -ENOSYS;
}

int TzTask::fchown(int fd, uint16_t owner, uint16_t group) {
    if ((fd >= MAX_FD) || (fd < 0))
        return -ENOENT;

    if (files[fd].data == nullptr)
        return -ENOENT;

    if (files[fd].type == File) {
        IFile *file = files[fd].file;
        uint16_t oldGroup = file->group();
        int rc = file->changeGroup(group);
        if (rc == 0) {
            rc = file->changeOwner(owner);
            if (rc != 0)
                file->changeGroup(oldGroup);
        }

        return rc;
    }

    if (files[fd].type == Directory) {
        IDirectory *dir = files[fd].dir;
        uint16_t oldGroup = dir->group();
        int rc = dir->changeGroup(group);
        if (rc == 0) {
            rc = dir->changeOwner(owner);
            if (rc != 0)
                dir->changeGroup(oldGroup);
        }

        return rc;
    }

    return -ENOSYS;
}

int TzTask::fstat(int fd, struct stat *statBuf) {
    if ((fd >= MAX_FD) || (fd < 0))
        return -ENOENT;

    if (files[fd].data == nullptr)
        return -ENOENT;

    memset(statBuf, 0, sizeof(struct stat));

    if (files[fd].type == Directory) {
        IDirectory *dir = files[fd].dir;
        statBuf->nlink = dir->numLinks();
        statBuf->uid = dir->owner();
        statBuf->gid = dir->group();

        AccessTimes tm;
        dir->accessTimes(&tm);
        statBuf->atime = tm.lastReadAt.tv_sec;
        statBuf->atimeNanoSec = tm.lastReadAt.tv_nsec;
        statBuf->mtime = tm.lastModifiedAt.tv_sec;
        statBuf->mtimeNanoSec = tm.lastModifiedAt.tv_nsec;
        statBuf->ctime = tm.lastStatusChangeAt.tv_sec;
        statBuf->ctimeNanoSec = tm.lastStatusChangeAt.tv_nsec;
    }

    if (files[fd].type == File) {
        IFile *file = files[fd].file;
        statBuf->nlink = file->numLinks();
        statBuf->uid = file->owner();
        statBuf->gid = file->group();

        AccessTimes tm;
        file->accessTimes(&tm);
        statBuf->atime = tm.lastReadAt.tv_sec;
        statBuf->atimeNanoSec = tm.lastReadAt.tv_nsec;
        statBuf->mtime = tm.lastModifiedAt.tv_sec;
        statBuf->mtimeNanoSec = tm.lastModifiedAt.tv_nsec;
        statBuf->ctime = tm.lastStatusChangeAt.tv_sec;
        statBuf->ctimeNanoSec = tm.lastStatusChangeAt.tv_nsec;

        statBuf->size = file->size();
        statBuf->blockSize = PAGE_SIZE_4K_BYTES;
        statBuf->blocks = file->numBlocks();
    }

    return 0;
}

int TzTask::mmap(TzMem::VirtAddr addr, TzMem::VirtAddr *allocated, size_t len, int prot, int flags, int fd, uint64_t offset) {

    if ((fd >= MAX_FD) || (fd < 0))
        return -ENOENT;

    if (files[fd].data == nullptr)
        return -ENOENT;

    if (files[fd].type != File)
        return -EINVAL;

    IFile *file = files[fd].file;
    if (offset > file->size())
        return -EINVAL;

    if ((offset + len) > file->size())
        return -EINVAL;

    uint8_t *desired = (uint8_t *)addr;
    int pageOffset = desired - (uint8_t *)PAGE_START_4K(desired);
    if ((pageOffset != 0) && (flags & MAP_FIXED))
        return -EINVAL; // We can only map at page boundaries.

    uint8_t *va = (pageOffset == 0) ? desired : (uint8_t *)PAGE_START_4K(desired) + PAGE_SIZE_4K_BYTES;

    int numPages = (len/PAGE_SIZE_4K_BYTES) + 1;

    // Check if the VA space is available
    if (!pageTable->isAddrRangeUnMapped(va, numPages*PAGE_SIZE_4K_BYTES)) {
        if (flags & MAP_FIXED)
            return -EINVAL;

        va = (uint8_t *)pageTable->reserveAddrRange(va, numPages*PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
        if (va == nullptr)
            return -ENOMEM;
    }

    int rv = file->mmap(va, allocated, len, prot, flags, offset);
    return rv;
}
