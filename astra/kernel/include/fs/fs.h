/***************************************************************************
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
 ***************************************************************************/

#ifndef INCLUDE_FS_FS_H_
#define INCLUDE_FS_FS_H_

#include <cstdint>
#include <cstddef>

#include "tzerrno.h"
#include "uio.h"
#include "tzfcntl.h"

#include "tzmemory.h"
#include "clock.h"
#include "config.h"

#define OWNER_ACCESS_PERMS(access) ((access >> 6) & 07)
#define GROUP_ACCESS_PERMS(access) ((access >> 3) & 07)
#define OTHERS_ACCESS_PERMS(access) ((access) & 07)

#define MAKE_PERMS(ownerAccess, groupAccess, othersAccess)  (((othersAccess)) | ((groupAccess) << 3) | ((ownerAccess) << 6))

#define PERMS_READ_BIT                     4
#define PERMS_WRITE_BIT             2
#define PERMS_EXECUTE_BIT           1

#define PERMS_READ_ONLY             (PERMS_READ_BIT )
#define PERMS_READ_WRITE            (PERMS_READ_BIT | PERMS_WRITE_BIT)
#define PERMS_READ_ONLY_EXECUTE     (PERMS_READ_BIT | PERMS_EXECUTE_BIT)
#define PERMS_WRITE_ONLY_EXECUTE    (PERMS_WRITE_BIT | PERMS_EXECUTE_BIT)
#define PERMS_READ_WRITE_EXECUTE    (PERMS_READ_BIT | PERMS_WRITE_BIT | PERMS_EXECUTE_BIT)

#define DEFAULT_PERMS_UMASK     MAKE_PERMS(PERMS_READ_WRITE, PERMS_READ_BIT, PERMS_READ_BIT)
#define DEFAULT_DIR_UMASK       MAKE_PERMS(PERMS_READ_WRITE_EXECUTE, PERMS_READ_ONLY_EXECUTE, PERMS_READ_ONLY_EXECUTE)

//typedef __SIZE_TYPE__ size_t;
typedef __SIZE_TYPE__ ssize_t;

class EventQueue;

struct AccessTimes {
    struct timespec lastReadAt;
    struct timespec lastModifiedAt;
    struct timespec lastStatusChangeAt;
};

class IFile {
public:
    virtual uint32_t permissions() const = 0;
    virtual int changePermissions(uint32_t newPerms) = 0;

    virtual uint16_t owner() const = 0;
    virtual int changeOwner(uint16_t newOwner) = 0;

    virtual uint16_t group() const = 0;
    virtual int changeGroup(uint16_t newGroup) = 0;

    virtual uint64_t size() const = 0;
    virtual long numBlocks() const = 0;
    virtual short numLinks() const = 0;

    virtual void erase() = 0;

    virtual size_t read(const void *data, const size_t numBytes, const uint64_t offset) = 0;
    virtual ssize_t readv(const struct iovec *iov, int iovcnt, const uint64_t offset) = 0;

    virtual size_t write(const void *data, const size_t numBytes, const uint64_t offset) = 0;
    virtual ssize_t writev(const struct iovec *iov, int iovcnt, const uint64_t offset) = 0;

    virtual void addWatcher(short pollEvent, short *pollResult, EventQueue *) = 0;

    virtual void dup() = 0;
    virtual IFile *clone() = 0;

    virtual bool isExecutable() = 0;

    virtual void accessTimes(AccessTimes *) = 0;

    virtual int mmap(void *addr, void **mappedAddr, size_t length, int prot, int flags, uint64_t offset) = 0;

protected:
    virtual ~IFile() {};

    void operator delete(void *ifp) { UNUSED(ifp); }
};

class IDirectory {
public:
    virtual uint32_t permissions() const = 0;
    virtual int changePermissions(uint32_t newPerms) = 0;

    virtual uint16_t owner() const = 0;
    virtual int changeOwner(uint16_t newOwner) = 0;

    virtual uint16_t group() const = 0;
    virtual int changeGroup(uint16_t newGroup) = 0;

    virtual IFile *file(const char *fileName) = 0;
    virtual IDirectory *dir(const char *dirName) = 0;

    virtual int addFile(const char *fileName, IFile *file) = 0;
    virtual int removeFile(const char *fileName) = 0;

    virtual int addDir(const char *dirName, IDirectory *dir) = 0;
    virtual int removeDir(const char *dirName) = 0;

    virtual int readDir(const void *data, const size_t size) = 0;
    virtual void openDir() = 0;
    virtual void closeDir() = 0;

    virtual int mount(const char *dirName, IDirectory *dir) = 0;
    virtual int unmount(const char *dirName) = 0;

    virtual int resolvePath(const char *path, IDirectory **dir, IFile **file, IDirectory **parentDir = nullptr) = 0;

    virtual bool isEmpty() = 0;

    virtual short numLinks() const = 0;

    virtual void dup() = 0;
    virtual IDirectory *clone() = 0;

    virtual void accessTimes(AccessTimes *) = 0;

protected:
    virtual ~IDirectory() {}

    void operator delete(void *ifp) { UNUSED(ifp); }
};

#endif /* INCLUDE_FS_FS_H_ */
