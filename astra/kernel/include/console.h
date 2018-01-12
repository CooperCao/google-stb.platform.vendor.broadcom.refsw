/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * console.h
 *
 *  Created on: Jan 28, 2015
 *      Author: gambhire
 */

#ifndef INCLUDE_CONSOLE_H_
#define INCLUDE_CONSOLE_H_

#include "system.h"
#include "fs/fs.h"
#include "uart.h"
#include "tzfcntl.h"

#include "lib_string.h"

extern "C" void kernel_write(const void *data, const size_t numBytes);

namespace Console {

extern IFile *stdout;
extern IFile *stdin;
extern IFile *stderr;
extern IFile *stdlog;

void init(bool isVuartChosen);


class Stdout : public IFile {
public:
    Stdout() {}

    static void init();

    uint32_t permissions() const { return MAKE_PERMS(PERMS_WRITE_BIT, PERMS_WRITE_BIT, PERMS_WRITE_BIT); };
    int changePermissions(uint32_t newPerms) { UNUSED(newPerms); return -ENOTSUP; }
    uint16_t owner() const { return System::UID; }
    int changeOwner(uint16_t newOwner) { UNUSED(newOwner); return -ENOTSUP; }
    uint16_t group() const { return System::GID; }
    int changeGroup(uint16_t newOwner) { UNUSED(newOwner); return -ENOTSUP; }

    uint64_t size() const { return 0; }
    long numBlocks() const { return 0; }
    short numLinks() const { return 0; }

    void erase() {}

    size_t read(const void *data, const size_t numBytes, const uint64_t offset);

    ssize_t readv(const iovec *iov, int iovcnt, const uint64_t offset);

    size_t write(const void *data, const size_t numBytes, const uint64_t offset);

    ssize_t writev(const iovec *iov, int iovcnt, const uint64_t offset);

    void addWatcher(short pollEvent, short *pollResult, EventQueue *eq);

    void dup() {}
    IFile *clone() { return nullptr; }

    bool isExecutable() { return false; }

    void accessTimes(AccessTimes *tm) {
        memset(tm, 0, sizeof(AccessTimes));
    }

    int mmap(void *addr, void **mappedAddr, size_t length, int prot, int flags, uint64_t offset, PageTable *pageTable = NULL) {
        UNUSED(addr); UNUSED(mappedAddr); UNUSED(length); UNUSED(prot); UNUSED(pageTable);
        UNUSED(flags); UNUSED(offset);
        return -ENOSYS;
    }

    static int setfops(IFile *ops) {
        if (NULL == ops) {
            return -EINVAL;
        }
        else {
            if (ops->permissions() & PERMS_WRITE_BIT) {
                fops = ops;
            }
            else {
                return -EPERM;
            }
        }
        return 0;
    }

private:
    ~Stdout() {}

    void* operator new(size_t sz, void* where);

private:
    static Stdout *out;
    static IFile *fops;
};

class Stdin : public IFile {
public:

    Stdin() {}

    static void init();

    uint32_t permissions() const { return MAKE_PERMS(PERMS_READ_BIT, PERMS_READ_BIT, PERMS_READ_BIT); };
    int changePermissions(uint32_t newPerms) { UNUSED(newPerms); return -ENOTSUP; }
    uint16_t owner() const { return System::UID; }
    int changeOwner(uint16_t newOwner) { UNUSED(newOwner); return -ENOTSUP; }
    uint16_t group() const { return System::GID; }
    int changeGroup(uint16_t newOwner) { UNUSED(newOwner); return -ENOTSUP; }

    uint64_t size() const { return 0; }
    long numBlocks() const { return 0; }
    short numLinks() const { return 0; }
    void erase() {}

    size_t read(const void *data, const size_t numBytes, const uint64_t offset);
    ssize_t readv(const iovec *iov, int iovcnt, const uint64_t offset);

    size_t write(const void *data, const size_t numBytes, const uint64_t offset);
    ssize_t writev(const iovec *iov, int iovcnt, const uint64_t offset);

    void addWatcher(short pollEvent, short *pollResult, EventQueue *eq);

    void dup() {}
    IFile *clone() { return nullptr; }

    bool isExecutable() { return false; }

    void accessTimes(AccessTimes *tm) {
        memset(tm, 0, sizeof(AccessTimes));
    }

    int mmap(void *addr, void **mappedAddr, size_t length, int prot, int flags, uint64_t offset, PageTable *pageTable = NULL) {
        UNUSED(addr); UNUSED(mappedAddr); UNUSED(length); UNUSED(prot); UNUSED(pageTable);
        UNUSED(flags); UNUSED(offset);
        return -ENOSYS;
    }

    static int setfops(IFile *ops) {
        if (NULL == ops) {
            return -EINVAL;
        }
        else {
            if (ops->permissions() & PERMS_READ_BIT) {
                fops = ops;
            }
            else {
                return -EPERM;
            }
        }
        return 0;
    }

private:
    ~Stdin() {}
    void* operator new(size_t sz, void* where);

private:
    static Stdin *in;
    static IFile *fops;
};

}



#endif /* INCLUDE_CONSOLE_H_ */
