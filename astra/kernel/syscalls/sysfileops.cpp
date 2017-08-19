/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


#include <cstdint>
#include "tzerrno.h"

#include "tzfcntl.h"
#include "tzmman.h"
#include "fs/ramfs.h"
#include "svcutils.h"
#include "tztask.h"


//TODO: Make this per-cpu and remove the fopsLock
static const int MaxFilePathSize=1024;
static char filePath[MaxFilePathSize];
SpinLock SysCalls::fopsLock;

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

void SysCalls::doOpen(TzTask *currTask) {
    SpinLocker locker(&fopsLock);

    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);

    const char *uFilePath = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uFilePath, filePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    int rv = currTask->open(filePath, (int)arg1, (int)arg2);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doOpenat(TzTask *currTask) {
    SpinLocker locker(&fopsLock);

    unsigned long fd = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r2);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r3);

    if(fd!=AT_FDCWD) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOSYS);
        return;
    }

    const char *uFilePath = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uFilePath, filePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    int rv = currTask->open(filePath, (int)arg1, (int)arg2);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doAccess(TzTask *currTask) {
    SpinLocker locker(&fopsLock);

    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    const char *uFilePath = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uFilePath, filePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    int rv = currTask->access(filePath, (int)arg1);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doCreat(TzTask *currTask) {
    SpinLocker locker(&fopsLock);

    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    const char *uFilePath = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uFilePath, filePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    int rv = currTask->open(filePath, O_WRONLY|O_CREAT|O_TRUNC, (int)arg1);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doDup(TzTask *currTask) {
    int oldFd = (int)currTask->userReg(TzTask::UserRegs::r0);
    int rv = currTask->dup(oldFd);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doDup2(TzTask *currTask) {
    int oldFd = (int)currTask->userReg(TzTask::UserRegs::r0);
    int newFd = (int)currTask->userReg(TzTask::UserRegs::r1);
    int rv = currTask->dup2(oldFd, newFd);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::dogetdents(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);

    //printf("dogetdents Enter \n");
    int fd = (int)arg0;
    size_t size = (size_t)arg2;
    void *uBuffer = (void *)arg1;
    if (!validateUserMemAccess(uBuffer, size)) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    int numPages = 0;
    TzMem::VirtAddr userBuffer = (TzMem::VirtAddr)(arg1);
    TzMem::VirtAddr kva = mapToKernel(userBuffer, size, &numPages);
    if (kva == nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
        return;
    }

    int pageOffset = (uint8_t *)userBuffer - (uint8_t *)PAGE_START_4K(userBuffer);
    uint8_t *buffer = (uint8_t *)kva + pageOffset;

    int rv = currTask->readdir(fd, buffer, size);
    //printf("readdir fd %d - %d  \n",fd,rv);

    unmap(kva, numPages);

    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}


void SysCalls::doRead(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);

    int fd = (int)arg0;
    size_t size = (size_t)arg2;
    void *uBuffer = (void *)arg1;
    if (!validateUserMemAccess(uBuffer, size)) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    int numPages = 0;
    TzMem::VirtAddr userBuffer = (TzMem::VirtAddr)(arg1);
    TzMem::VirtAddr kva = mapToKernel(userBuffer, size, &numPages);
    if (kva == nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
        return;
    }

    int pageOffset = (uint8_t *)userBuffer - (uint8_t *)PAGE_START_4K(userBuffer);
    uint8_t *buffer = (uint8_t *)kva + pageOffset;

    int rv = currTask->read(fd, buffer, size);

    unmap(kva, numPages);

    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doWrite(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);

    int fd = (int)arg0;
    size_t size = (size_t)arg2;
    void *uBuffer = (void *)arg1;
    if (!validateUserMemAccess(uBuffer, size)) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    int numPages = 0;
    TzMem::VirtAddr userBuffer = (TzMem::VirtAddr)(arg1);
    TzMem::VirtAddr kva = mapToKernel(userBuffer, size, &numPages);
    if (kva == nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
        return;
    }

    int pageOffset = (uint8_t *)userBuffer - (uint8_t *)PAGE_START_4K(userBuffer);
    uint8_t *buffer = (uint8_t *)kva + pageOffset;

    int rv = currTask->write(fd, buffer, size);

    unmap(kva, numPages);

    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}


void SysCalls::doWritev(TzTask *currTask) {
    unsigned long fd = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long iovecs = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long iovcnt = currTask->userReg(TzTask::UserRegs::r2);

    struct iovec *userIovecs = (struct iovec *)iovecs;
    struct iovec *kernelIovecs = (struct iovec *)paramsPage.cpuLocal();
    bool rc = fromUser(userIovecs, kernelIovecs, iovcnt * sizeof(struct iovec));
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }


    uint8_t *startPos = (uint8_t *)paramsPage.cpuLocal() + iovcnt * sizeof(struct iovec);
    uint8_t *currPos = startPos;
    uint8_t *lastPos = startPos + PAGE_SIZE_4K_BYTES - 1;

    for (int i=0; i<iovcnt; i++) {
        size_t len = kernelIovecs[i].iov_len;
        if ((currPos + len) > lastPos) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
            return;
        }
        /* Unused iovec from fflush standard C func, ignore here;
            Look for length before dereference the address at respective write function handler */
        if ((0 == kernelIovecs[i].iov_base) && (0 == len))
            continue;

        rc = fromUser(kernelIovecs[i].iov_base, currPos, len);
        if (!rc) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
            return;
        }
        kernelIovecs[i].iov_base = currPos;
        currPos += len;
    }

#if 0
    for (int i=0; i<iovcnt; i++) {
        printf("iov %d iovBase %p len %d: ", i, kernelIovecs[i].iov_base, kernelIovecs[i].iov_len);
        for (int j=0; j<kernelIovecs[i].iov_len; j++) {
            char *iovBase = (char *)kernelIovecs[i].iov_base;
            printf("%c", iovBase[j]);
        }
        printf("\n");
    }
#endif

    ssize_t rv = currTask->writev(fd, kernelIovecs, iovcnt);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doReadv(TzTask *currTask) {

    int numFragPages[MAX_NUM_IOV];
    TzMem::VirtAddr vaFrag[MAX_NUM_IOV];
    TzMem::VirtAddr uvaFrag[MAX_NUM_IOV];

    for (int i=0; i<MAX_NUM_IOV; i++) {
        numFragPages[i] = 0;
        vaFrag[i] = nullptr;
        uvaFrag[i] = nullptr;
    }

    unsigned long fd = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long iovecs = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long iovcnt = currTask->userReg(TzTask::UserRegs::r2);
    if (iovcnt > MAX_NUM_IOV) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
        return;
    }

    if (!validateUserMemAccess((void *)iovecs, iovcnt * sizeof(struct iovec))) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    int numPages = 0;
    TzMem::VirtAddr kva = mapToKernel((void *)iovecs, iovcnt * sizeof(struct iovec), &numPages);
    int pageOffset = (uint8_t *)iovecs - (uint8_t *)PAGE_START_4K(iovecs);
    struct iovec *userIovecs = (struct iovec *)((uint8_t *)kva + pageOffset);

    for (int i=0; i<iovcnt; i++) {
        size_t len = userIovecs[i].iov_len;
        TzMem::VirtAddr va = userIovecs[i].iov_base;

        if (!validateUserMemAccess((void *)va, len)) {

            for (int j=0; j<i; j++) {
                unmap(vaFrag[j], numFragPages[j]);
                userIovecs[i].iov_base = uvaFrag[i];
            }
            unmap(kva, numPages);

            currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
            return;
        }

        uvaFrag[i] = va;
        vaFrag[i] = mapToKernel(va, len, &numFragPages[i]);
        int fragOffset = (uint8_t *)va - (uint8_t *)PAGE_START_4K(va);
        userIovecs[i].iov_base = ((uint8_t *)vaFrag[i] + fragOffset);

    }

    ssize_t rv = currTask->readv(fd, userIovecs, iovcnt);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);

#if 0
    for (int i=0; i<iovcnt; i++) {
        printf("iov %d iovBase %p len %d: \n", i, userIovecs[i].iov_base, userIovecs[i].iov_len);
        int *iovBase = (int *)userIovecs[i].iov_base;
        for (int j=0; j<userIovecs[i].iov_len/4; j++) {
            printf("%d ", iovBase[j]);
        }
        printf("\n");
    }
#endif

    for (int i=0; i<iovcnt; i++) {
        unmap(vaFrag[i], numFragPages[i]);
        userIovecs[i].iov_base = uvaFrag[i];
    }

    unmap(kva, numPages);

}

void SysCalls::doClose(TzTask *currTask) {
    int fd = (int)currTask->userReg(TzTask::UserRegs::r0);
    int rv = currTask->close(fd);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::dollSeek(TzTask *currTask) {
    int fd = (int)currTask->userReg(TzTask::UserRegs::r0);
    unsigned long offsetHigh = (unsigned long)currTask->userReg(TzTask::UserRegs::r1);
    unsigned long offsetLow = (unsigned long)currTask->userReg(TzTask::UserRegs::r2);
    loff_t *userResult = (loff_t *)currTask->userReg(TzTask::UserRegs::r3);
    unsigned int whence = (unsigned int)currTask->userReg(TzTask::UserRegs::r4);

    loff_t resultPos;
    int rv = currTask->llseek(fd, ((uint64_t)offsetHigh << 32)|offsetLow, &resultPos, whence);
    if (rv == 0)
        copyToUser(userResult, &resultPos);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::dolSeek(TzTask *currTask) {
    int fd = (int)currTask->userReg(TzTask::UserRegs::r0);
    off_t offset = (off_t)currTask->userReg(TzTask::UserRegs::r1);
    unsigned int whence = (unsigned int)currTask->userReg(TzTask::UserRegs::r2);

    loff_t resultPos;
    int rv = currTask->llseek(fd, (uint64_t)offset, &resultPos, whence);
    if (rv == 0)
        currTask->writeUserReg(TzTask::UserRegs::r0, resultPos);
    else
        currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

static int linkFile(IDirectory *newParent, IFile *file, const char *linkName) {
    IFile *cloned = file->clone();
    int rv = newParent->addFile(linkName, cloned);
    if (rv != 0) {
        RamFS::File::destroy(cloned);
    }

    return rv;
}

void SysCalls::commonRename(TzTask *currTask, unsigned long arg0, unsigned long arg1) {
    static char oldFilePath[MaxFilePathSize];
    static char newFilePath[MaxFilePathSize];

    SpinLocker locker(&fopsLock);
    IDirectory *root = System::root();
    IDirectory *cwd = currTask->currDir();

    const char *uOldFilePath = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uOldFilePath, oldFilePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    // Extract the end point from the path.
    const char *oldEndPoint = extractEndPoint(oldFilePath);

    IDirectory *oldDir = nullptr, *oldParent = nullptr;
    IFile *oldFile = nullptr;
    if (root->resolvePath(oldFilePath, &oldDir, &oldFile, &oldParent)) {
        if (cwd->resolvePath(oldFilePath, &oldDir, &oldFile, &oldParent)) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -ENOENT);
            return;
        }
    }

    const char *uNewFilePath = (const char *)arg1;
    truncated = false;
    rc = strFromUser(uNewFilePath, newFilePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    // Extract the new end point from the path.
    const char *newEndPoint = extractEndPoint(newFilePath);

    IDirectory *newDir = nullptr, *newParent = nullptr;
    IFile *newFile = nullptr;
    if (newFilePath[0] == '/')
        root->resolvePath(newFilePath, &newDir, &newFile, &newParent);
    else
        cwd->resolvePath(newFilePath, &newDir, &newFile, &newParent);

    // If both old and new end points point to the same entity
    // there's nothing to rename. Return success.
    if (((newDir == oldDir) && (newDir != nullptr)) ||
            ((newFile == oldFile) && (newFile != nullptr))){
        currTask->writeUserReg(TzTask::UserRegs::r0, 0);
        return;
    }

    //printf("oldFilePath %s oldDir %p oldFile %p oldParent %p, oldEndPoint %s\n", oldFilePath, oldDir, oldFile, oldParent, oldEndPoint);
    //printf("newFilePath %s newDir %p newFile %p newParent %p, newEndPoint %s\n", newFilePath, newDir, newFile, newParent, newEndPoint);

    // If the end point of the new path exists, remove it.
    if (newDir != nullptr) {
        if (!newDir->isEmpty()) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -ENOTEMPTY);
            return;
        }

        // Keep a clone handy in case the subsequent insertion fails
        newDir = newDir->clone();
        newParent->removeDir(newEndPoint);
    }

    if (newFile != nullptr) {
        // Keep a clone handy in case the subsequent insertion fails
        newFile = newFile->clone();
        newParent->removeFile(newEndPoint);
    }

    rc = 0;

    // Now add end point of the old path to the new path
    // Is it pointing to a file ?
    if (oldFile != nullptr) {
        // Yes, add the file to new path.
        IFile *cloned = oldFile->clone();
        rc = newParent->addFile(newEndPoint, cloned);
        if (rc == 0)
            oldParent->removeFile(oldEndPoint);
    }
    else { // Old path points to a directory
        // Add the directory to the new path.
        IDirectory *cloned = oldDir->clone();
        rc = newParent->addDir(newEndPoint, cloned);
        if (rc == 0)
            oldParent->removeDir(oldEndPoint);
    }

    if (rc != 0) {
        // Rename failed. Restore any deleted end points.
        if (newDir != nullptr)
            newParent->addDir(newEndPoint, newDir);
        else if (newFile != nullptr)
            newParent->addFile(newEndPoint, newFile);

        currTask->writeUserReg(TzTask::UserRegs::r0, rc);
    }

    // Rename succeeded. Remove the clones.
    if (newDir != nullptr)
        RamFS::Directory::destroy(newDir);
    else if (newFile != nullptr)
        RamFS::File::destroy(newFile);

    /*
    printf("OLD:\n");
    ((RamFS::Directory *)oldParent)->print();

    printf("NEW:\n");
    ((RamFS::Directory *)newParent)->print();
    */

    currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}

void SysCalls::doRename(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    commonRename(currTask, arg0, arg1);
}

void SysCalls::doRenameat(TzTask *currTask) {
    unsigned long fd = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long fd2 = currTask->userReg(TzTask::UserRegs::r2);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r3);
    UNUSED(fd2);

    if(fd==AT_FDCWD)
        commonRename(currTask, arg0, arg1);
    else
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOSYS);
}

static int linkDir(IDirectory *newParent, IDirectory *dir, const char *linkName) {
    IDirectory *cloned = dir->clone();
    int rv = newParent->addDir(linkName, cloned);
    if (rv != 0) {
        RamFS::Directory::destroy(cloned);
    }

    return rv;
}


void SysCalls::commonLink(TzTask *currTask, unsigned long arg0, unsigned long arg1) {
    SpinLocker locker(&fopsLock);

    IDirectory *root = System::root();
    IDirectory *cwd = currTask->currDir();

    const char *uOldFilePath = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uOldFilePath, filePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    IDirectory *oldDir = nullptr, *oldParent = nullptr;
    IFile *oldFile = nullptr;
    if (root->resolvePath(filePath, &oldDir, &oldFile, &oldParent)) {
        if (cwd->resolvePath(filePath, &oldDir, &oldFile, &oldParent)) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -ENOENT);
            return;
        }
    }

    const char *uNewFilePath = (const char *)arg1;
    truncated = false;
    rc = strFromUser(uNewFilePath, filePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    // Extract the file name from the path.
    const char *fileName = extractEndPoint(filePath);

    IDirectory *newDir = nullptr, *newParent = nullptr;
    IFile *newFile = nullptr;
    if (filePath[0] == '/')
        root->resolvePath(filePath, &newDir, &newFile, &newParent);
    else
        cwd->resolvePath(filePath, &newDir, &newFile, &newParent);


    if ((oldDir != nullptr) && (newDir != nullptr) && (oldDir == newDir)) {
        currTask->writeUserReg(TzTask::UserRegs::r0, 0);
        return;
    }

    if ((oldFile != nullptr) && (newFile != nullptr) && (oldFile == newFile)) {
        currTask->writeUserReg(TzTask::UserRegs::r0, 0);
        return;
    }

    if ((newFile != nullptr) || (newDir != nullptr)) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EEXIST);
        return;
    }

    int rv = 0;
    if (oldDir != nullptr)
        rv = linkDir(newParent, oldDir, fileName);
    if (oldFile != nullptr)
        rv = linkFile(newParent, oldFile, fileName);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doLink(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    commonLink(currTask, arg0, arg1);
}

void SysCalls::doLinkat(TzTask *currTask) {
    unsigned long fd = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long fd2 = currTask->userReg(TzTask::UserRegs::r2);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r3);
    UNUSED(fd2);

    if(fd==AT_FDCWD)
        commonLink(currTask, arg0, arg1);
    else
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOSYS);
}

void SysCalls::commonUnlink(TzTask *currTask, unsigned long arg0) {
    SpinLocker locker(&fopsLock);

    IDirectory *root = System::root();
    IDirectory *cwd = currTask->currDir();

    const char *uOldFilePath = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uOldFilePath, filePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    IDirectory *oldDir = nullptr, *oldParent = nullptr;
    IFile *oldFile = nullptr;
    if (root->resolvePath(filePath, &oldDir, &oldFile, &oldParent)) {
        if (cwd->resolvePath(filePath, &oldDir, &oldFile, &oldParent)) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -ENOENT);
            return;
        }
    }

    // Extract the file name from the path.
    const char *fileName = extractEndPoint(filePath);

    int rv = 0;
    if (oldFile != nullptr) {
        rv = oldParent->removeFile(fileName);
        if (rv == 0) RamFS::File::destroy(oldFile);
    }
    if (oldDir != nullptr) {
        rv = oldParent->removeDir(fileName);
        if (rv == 0) RamFS::Directory::destroy(oldDir);
    }

    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doUnlink(TzTask *currTask) {

    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    commonUnlink(currTask, arg0);
}

void SysCalls::doUnlinkat(TzTask *currTask) {
    unsigned long fd = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r1);
    if(fd==AT_FDCWD)
        commonUnlink(currTask, arg0);
    else
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOSYS);
}

void SysCalls::doChdir(TzTask *currTask) {
    SpinLocker locker(&fopsLock);

    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);

    const char *uFilePath = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uFilePath, filePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    int rv = currTask->setCurrDir(filePath);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doFchdir(TzTask *currTask) {

    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);

    int rv = currTask->setCurrDir((int)arg0);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doMknod(TzTask *currTask) {
    currTask->writeUserReg(TzTask::UserRegs::r0, -ENOTSUP);
}

void SysCalls::doChmod(TzTask *currTask) {
    SpinLocker locker(&fopsLock);

    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    const char *uFilePath = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uFilePath, filePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    int rv = currTask->chmod(filePath, arg1);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doFchmod(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    int rv = currTask->fchmod(arg0, arg1);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doChown(TzTask *currTask) {
    SpinLocker locker(&fopsLock);

    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);

    const char *uFilePath = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uFilePath, filePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    int rv = currTask->chown(filePath, (uint16_t)arg1, (uint16_t)arg2);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doFchown(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);

    int rv = currTask->fchown(arg0, (uint16_t)arg1, (uint16_t)arg2);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doMount(TzTask *currTask) {
    //TODO: Use the following format for arguments
    //  source: "<startAddress>:<numBytes>"
    //  fileSystemType: "ramfs"
    //  data: empty
    currTask->writeUserReg(TzTask::UserRegs::r0, -ENOTSUP);
}

void SysCalls::doUmount(TzTask *currTask) {
    //TODO:
    currTask->writeUserReg(TzTask::UserRegs::r0, -ENOTSUP);
}

void SysCalls::doPoll(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);

    struct pollfd *fdsUser = (struct pollfd *)arg0;
    nfds_t nfds = (nfds_t)arg1;
    if ((nfds >= TzTask::MAX_FD) || ((nfds * sizeof(struct pollfd)) >= PAGE_SIZE_4K_BYTES)) {
        err_msg("%s: too many FDs %d. Max supported per process %d.\n", __FUNCTION__, nfds, TzTask::MAX_FD);
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
        return;
    }
    //printf("%s: nfds %d\n", __FUNCTION__, nfds);

    struct pollfd *fdsKernel = (struct pollfd *)paramsPage.cpuLocal();
    bool rc = fromUser(fdsUser, fdsKernel, nfds*sizeof(struct pollfd));
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    //for (int i=0; i<nfds; i++)
    //  printf("\t%d: fd %d events 0x%x \n", i, fdsKernel[i].fd, fdsKernel[i].events);

    int timeout = (int)arg2;
    //printf("\t timeout %d\n", timeout);

    int rv = currTask->poll(fdsKernel, nfds, timeout);
    //printf("%s: after currTask->poll rv %d\n", __FUNCTION__, rv);
    //for (int i=0; i<nfds; i++)
    //  printf("\t%d: fd %d events 0x%x revents 0x%x\n", i, fdsKernel[i].fd, fdsKernel[i].events, fdsKernel[i].revents);

    toUser(fdsUser, fdsKernel, nfds*sizeof(struct pollfd));
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doPPoll(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);
    unsigned long arg3 = currTask->userReg(TzTask::UserRegs::r3);

    // We only support ppoll in the poll compatibility mode
    if(arg3) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOTSUP);
        return;
    }

    struct pollfd *fdsUser = (struct pollfd *)arg0;
    nfds_t nfds = (nfds_t)arg1;
    if ((nfds >= TzTask::MAX_FD) || ((nfds * sizeof(struct pollfd)) >= PAGE_SIZE_4K_BYTES)) {
        err_msg("%s: too many FDs %d. Max supported per process %d.\n", __FUNCTION__, nfds, TzTask::MAX_FD);
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
        return;
    }

    /* Check that the nfds structure does not span page boundaries */
    /* TODO: map multiple pages if the structure does span page boundaries */
    //bool nfds_spans_pb =
        //((uintptr_t)fdsUser - (uintptr_t)PAGE_START_4K(fdsUser) + (nfds * sizeof(struct pollfd))) >= PAGE_SIZE_4K_BYTES;
    //printf("nfds structure does %s span page boundary\n", nfds_spans_pb ? "" : "not");

    /* Map the user space paramater before copying it to kernel space */
    PageTable *kpt = PageTable::kernelPageTable();
    PageTable *upt = currTask->userPageTable();
    TzMem::PhysAddr pa = upt->lookUp(PAGE_START_4K(fdsUser));
    struct pollfd *fdsKernel = (struct pollfd *)paramsPage.cpuLocal();
    kpt->mapPage(PAGE_START_4K(fdsUser), pa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true);
    bool rc = fromUser(fdsUser, fdsKernel, nfds*sizeof(struct pollfd));
    if (!rc) {
        kpt->unmapPage(PAGE_START_4K(fdsUser));
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    //for (int i=0; i<nfds; i++)
      //printf("\t%d: fd %d events 0x%x \n", i, fdsKernel[i].fd, fdsKernel[i].events);

    int timeout = -1;
    const struct timespec *timeoutUser = (const struct timespec *)arg2;

    /* Check that the timespec structure does not span page boundaries */
    /* TODO: map multiple pages if the structure does span page boundaries */
    //bool timespec_spans_pb =
        //((uintptr_t)timeoutUser - (uintptr_t)PAGE_START_4K(timeoutUser) + sizeof(struct timespec)) >= PAGE_SIZE_4K_BYTES;
    //printf("timeoutUser structure does %s span page boundary\n",timespec_spans_pb ? "" : "not");

    /* Map the user space paramater before copying it to kernel space */
    pa = upt->lookUp(PAGE_START_4K(timeoutUser));
    if(kpt->isAddrRangeUnMapped(PAGE_START_4K(timeoutUser),PAGE_SIZE_4K_BYTES))
       kpt->mapPage(PAGE_START_4K(timeoutUser), pa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true);

    /* Skip past the nfds data in the paramsPage to map the timeout argument */
    struct timespec *timeoutKernel = (struct timespec *)((uint8_t *)paramsPage.cpuLocal() + nfds * sizeof(struct pollfd));
    rc = copyFromUser(timeoutUser, timeoutKernel);
    if (!rc) {
       if(kpt->isAddrRangeMapped(PAGE_START_4K(fdsUser),PAGE_SIZE_4K_BYTES))
           kpt->unmapPage(PAGE_START_4K(fdsUser));
       if(kpt->isAddrRangeMapped(PAGE_START_4K(timeoutUser),PAGE_SIZE_4K_BYTES))
           kpt->unmapPage(PAGE_START_4K(timeoutUser));
       currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
       return;
    }

    /* Convert the timeout to milliseconds for a poll call */
    timeout = (timeoutKernel == NULL) ? -1 :
             (timeoutKernel->tv_sec * 1000 + timeoutKernel->tv_nsec / 1000000);

    //sigset_t origmask;
    //sigprocmask(SIG_SETMASK, &sigmask, &origmask);
    int rv = currTask->poll(fdsKernel, nfds, timeout);
    //sigprocmask(SIG_SETMASK, &origmask, NULL);

    //for (int i=0; i<nfds; i++)
      //printf("\t%d: fd %d events 0x%x revents 0x%x\n", i, fdsKernel[i].fd, fdsKernel[i].events, fdsKernel[i].revents);

    toUser(fdsUser, fdsKernel, nfds*sizeof(struct pollfd));

    /* Defer the unmaps as they may be the same page */
    if(kpt->isAddrRangeMapped(PAGE_START_4K(timeoutUser),PAGE_SIZE_4K_BYTES))
       kpt->unmapPage(PAGE_START_4K(timeoutUser));
    if(kpt->isAddrRangeMapped(PAGE_START_4K(fdsUser),PAGE_SIZE_4K_BYTES))
        kpt->unmapPage(PAGE_START_4K(fdsUser));

    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doSync(TzTask *currTask) {
    //
    // We only support a RAMFS. Its always in
    // sync.
    //
    currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}

void SysCalls::doFSync(TzTask *currTask) {
    //
    // We only support a RAMFS. Its always in
    // sync.
    //
    currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}

void SysCalls::commonMkdir(TzTask *currTask, unsigned long arg0, unsigned long arg1) {
    SpinLocker locker(&fopsLock);

    const char *uFilePath = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uFilePath, filePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    // Extract the file name from the path.
    const char *dirName = extractEndPoint(filePath);

    IDirectory *dir = nullptr;
    IFile *file = nullptr;
    IDirectory *parentDir = nullptr;
    IDirectory *root = System::root();
    IDirectory *cwd = currTask->currDir();

    int rv = 0;
    if (filePath[0] == '/')
        rv = root->resolvePath(filePath, &dir, &file, &parentDir);
    else
        rv = cwd->resolvePath(filePath, &dir, &file, &parentDir);

    if (rv == 0) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EEXIST);
        return;
    }

    if (parentDir == nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, rv);
        return;
    }

    IDirectory *newDir = RamFS::Directory::create(currTask->owner(), currTask->group(), parentDir, (uint32_t)arg1);
    if (newDir == nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, rv);
        return;
    }

    rv = parentDir->addDir(dirName, newDir);
    if (rv != 0) {
        RamFS::Directory::destroy(newDir);
        currTask->writeUserReg(TzTask::UserRegs::r0, rv);
        return;
    }

    currTask->writeUserReg(TzTask::UserRegs::r0, 0);

}

void SysCalls::doMkdir(TzTask *currTask) {

    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    commonMkdir(currTask, arg0, arg1);
}

void SysCalls::doMkdirat(TzTask *currTask) {

    unsigned long fd = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r2);
    if(fd==AT_FDCWD)
        commonMkdir(currTask, arg0, arg1);
    else
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOSYS);
}

void SysCalls::doRmdir(TzTask *currTask) {
    SpinLocker locker(&fopsLock);

    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);

    const char *uFilePath = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uFilePath, filePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    const char *dirName = extractEndPoint(filePath);

    IDirectory *dir = nullptr;
    IFile *file = nullptr;
    IDirectory *parentDir = nullptr;
    IDirectory *root = System::root();
    IDirectory *cwd = currTask->currDir();


    int rv = 0;
    if (filePath[0] == '/')
        rv = root->resolvePath(filePath, &dir, &file, &parentDir);
    else
        rv = cwd->resolvePath(filePath, &dir, &file, &parentDir);

    if (rv != 0) {
        ((RamFS::Directory *)root)->print();
        currTask->writeUserReg(TzTask::UserRegs::r0, rv);
        return;
    }

    if (file != nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOTDIR);
        return;
    }

    if (!dir->isEmpty()) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOTEMPTY);
        return;
    }

    rv = parentDir->removeDir(dirName);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);

}

void SysCalls::commonStat(TzTask *currTask, unsigned long arg0, unsigned long arg1) {
    SpinLocker locker(&fopsLock);

    const char *uFilePath = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uFilePath, filePath, MaxFilePathSize, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    struct stat statBuf;
    struct stat *userBuf = (struct stat *)arg1;
    rc = copyFromUser(userBuf, &statBuf);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    IDirectory *dir = nullptr;
    IFile *file = nullptr;
    IDirectory *parentDir = nullptr;
    IDirectory *root = System::root();
    IDirectory *cwd = currTask->currDir();

    int rv = 0;
    if (filePath[0] == '/')
        rv = root->resolvePath(filePath, &dir, &file, &parentDir);
    else
        rv = cwd->resolvePath(filePath, &dir, &file, &parentDir);
    if (rv != 0) {
        currTask->writeUserReg(TzTask::UserRegs::r0, rv);
        return;
    }

    memset(&statBuf, 0, sizeof(struct stat));
    if (dir != nullptr) {
        statBuf.mode = dir->permissions();
        statBuf.nlink = dir->numLinks();
        statBuf.uid = dir->owner();
        statBuf.gid = dir->group();

        AccessTimes tm;
        dir->accessTimes(&tm);
        statBuf.atime = tm.lastReadAt.tv_sec;
        statBuf.atimeNanoSec = tm.lastReadAt.tv_nsec;
        statBuf.mtime = tm.lastModifiedAt.tv_sec;
        statBuf.mtimeNanoSec = tm.lastModifiedAt.tv_nsec;
        statBuf.ctime = tm.lastStatusChangeAt.tv_sec;
        statBuf.ctimeNanoSec = tm.lastStatusChangeAt.tv_nsec;
    }
    if (file != nullptr) {
        statBuf.mode = file->permissions();
        statBuf.nlink = file->numLinks();
        statBuf.uid = file->owner();
        statBuf.gid = file->group();
        statBuf.size = file->size();
        statBuf.blockSize = PAGE_SIZE_4K_BYTES;
        statBuf.blocks = file->numBlocks();

        AccessTimes tm;
        file->accessTimes(&tm);
        statBuf.atime = tm.lastReadAt.tv_sec;
        statBuf.atimeNanoSec = tm.lastReadAt.tv_nsec;
        statBuf.mtime = tm.lastModifiedAt.tv_sec;
        statBuf.mtimeNanoSec = tm.lastModifiedAt.tv_nsec;
        statBuf.ctime = tm.lastStatusChangeAt.tv_sec;
        statBuf.ctimeNanoSec = tm.lastStatusChangeAt.tv_nsec;
    }

    copyToUser(userBuf, &statBuf);
    currTask->writeUserReg(TzTask::UserRegs::r0, 0);

}

void SysCalls::doFstatat(TzTask *currTask) {
    unsigned long fd = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);

    if(fd==AT_FDCWD)
        commonStat(currTask, arg1, arg2);
    else
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOSYS);
}

void SysCalls::doStat64(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    commonStat(currTask, arg0, arg1);
}

void SysCalls::doFStat64(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    struct stat statBuf;
    struct stat *userBuf = (struct stat *)arg1;
    bool rc = copyFromUser(userBuf, &statBuf);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    int rv = currTask->fstat((int)arg0, &statBuf);
    if (rv == 0)
        copyToUser(userBuf, &statBuf);

    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doMmap(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);
    unsigned long arg3 = currTask->userReg(TzTask::UserRegs::r3);
    unsigned long arg4 = currTask->userReg(TzTask::UserRegs::r4);
    unsigned long arg5 = currTask->userReg(TzTask::UserRegs::r5);

    TzMem::VirtAddr va = (TzMem::VirtAddr)arg0;
    size_t length = (size_t)arg1;
    int prot = (int)arg2;
    int flags = (int)arg3;

    if (va == nullptr) {
        // Do not map address zero.
        if (flags & MAP_FIXED) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -1);
            return;
        }

        va = (void *)(PAGE_SIZE_4K_BYTES);
    }

    // printf("addr %p length %ld prot %d flags %d fd %d offset %d\n", va, arg1, prot, flags, (int)arg4, (int)arg5);

    TzMem::VirtAddr result;
    int rv;
    if (!(flags & MAP_ANONYMOUS)) {
        int fd = (int)arg4;
#ifdef __aarch64__
        uint64_t offset = (uint64_t)arg5;
#else
        uint64_t offset = (uint64_t)arg5 * 4096;
#endif
        rv = currTask->mmap(va, &result, length, prot, flags, fd, offset);
    }
    else {
        rv = currTask->mmap(va, &result, length, prot, flags);
    }

    // printf("%s: mmap returned %d page table: \n", __FUNCTION__, rv);
    // currTask->userPageTable()->dump();

    if (rv != 0)
        currTask->writeUserReg(TzTask::UserRegs::r0, -1);
    else
        currTask->writeUserReg(TzTask::UserRegs::r0, (unsigned long)result);
}

void SysCalls::doUnmap(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    TzMem::VirtAddr va = (TzMem::VirtAddr)arg0;
    size_t length = (size_t)arg1;

    if (!validateUserMemAccess(va, length)) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    int rv = currTask->munmap(va, length);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doMProtect(TzTask *currTask) {
    void *addr = (void *)currTask->userReg(TzTask::UserRegs::r0);
    size_t len = (size_t)currTask->userReg(TzTask::UserRegs::r1);
    int prot = (int)currTask->userReg(TzTask::UserRegs::r2);

    if (!validateUserMemAccess(addr, len)) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    int rv = currTask->mprotect(addr, len, prot);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}
