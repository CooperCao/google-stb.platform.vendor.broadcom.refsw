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
 * console.h
 *
 *  Created on: Jan 28, 2015
 *      Author: gambhire
 */

#ifndef INCLUDE_UARTFOPS_H_
#define INCLUDE_UARTFOPS_H_

#include "system.h"
#include "fs/fs.h"
#include "uart.h"
#include "tzfcntl.h"

#include "lib_string.h"

namespace UartFops {

void init(void);

class UartFile : public IFile {
public:
    uint32_t permissions() const { return MAKE_PERMS(PERMS_READ_WRITE, PERMS_READ_WRITE, PERMS_READ_WRITE); };
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
    bool execPermissionsCheck() { return false; }

    void accessTimes(AccessTimes *tm) {
        memset(tm, 0, sizeof(AccessTimes));
    }

    int mmap(void *addr, void **mappedAddr, size_t length, int prot, int flags, uint64_t offset, PageTable *pageTable = NULL) {
        UNUSED(addr); UNUSED(mappedAddr); UNUSED(length); UNUSED(prot); UNUSED(pageTable);
        UNUSED(flags); UNUSED(offset);
        return -ENOSYS;
    }

    static UartFile *getInstance(void) {
        if(!instance) {
            init();
        }
        return instance;
    }
private:
    static UartFile *instance;
    ~UartFile() {}
    UartFile() {}
    static void init();
    void* operator new(size_t sz, void* where);
};
}
#endif /* INCLUDE_UARTFOPS_H_ */
