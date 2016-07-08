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
 * system.cpp
 *
 *  Created on: Feb 18, 2015
 *      Author: gambhire
 */

#include <hwtimer.h>
#include <stdint.h>
#include <waitqueue.h>

#include "arm/arm.h"
#include "arm/gic.h"
#include "fs/ramfs.h"
#include "config.h"

#include "lib_printf.h"
#include "libfdt.h"
#include "parse_utils.h"

#include "platform.h"
#include "kernel.h"
#include "tzmemory.h"
#include "pgtable.h"
#include "objalloc.h"
#include "interrupt.h"
#include "tztask.h"
#include "scheduler.h"
#include "svcutils.h"
#include "smcutils.h"
#include "console.h"
#include "clock.h"
#include "futex.h"
#include "crypto/csprng.h"

#include "system.h"

static uint8_t tzDevTree[MAX_DT_SIZE_BYTES];
static IDirectory *rootDir;

extern "C" unsigned long _initramfs_start;
extern "C" unsigned long _initramfs_end;

void System::init(const void *devTree) {

    // The bootstrap code has mapped device tree memory one-to-one VA and PA.
    int rc = fdt_check_header(devTree);
    if (rc) {
        err_msg("Corrupted device tree at %p.\n", devTree);
        return;
    }

    int dtSize = fdt_totalsize(devTree);
    if (dtSize > MAX_DT_SIZE_BYTES) {
        err_msg("Device tree is too large (size %d) . Increase MAX_DT_SIZE_BYTES\n", dtSize);
        return;
    }

    rc = fdt_move(devTree, tzDevTree, dtSize);
    if (rc) {
        err_msg("Device tree could not be relocated to tzDevTree: %s\n", fdt_strerror(rc));
        return;
    }

    PageTable::init();
    Platform::init(tzDevTree);
    TzMem::init(tzDevTree);
    GIC::init(tzDevTree);
    Interrupt::init();
    TzTimers::init();
    Scheduler::init();
    TzTask::init();
    ElfImage::init();
    RamFS::init();
    SysCalls::init();
    SmcCalls::init();
    Console::init(!Platform::hasUart());
    TzClock::init();
    Futex::init();
    MsgQueue::init();
    CryptoPRNG::init();

    int size = (int)&_initramfs_end - (int)&_initramfs_start;
    printf("File system start %p end %p. size %d\n", &_initramfs_start, &_initramfs_end, size);

    rootDir = RamFS::load(&_initramfs_start);
    if (rootDir == nullptr) {
        err_msg("Could not mount Root FS\n");
        kernelHalt("No root fs\n");
    }

    TzMem::freeInitRamFS();
    printf("File system loaded. Freed up %d kbytes\n", size/1024);

    /* Unmap the bootstrap part of the kernel */
    PageTable *kernPageTable = PageTable::kernelPageTable();
    kernPageTable->unmapBootstrap(devTree);

    // Create /dev/random and /dev/urandom
    IDirectory *devDir = RamFS::Directory::create(System::UID, System::GID, rootDir,
                                MAKE_PERMS(PERMS_READ_WRITE_EXECUTE, PERMS_READ_ONLY_EXECUTE, PERMS_READ_ONLY_EXECUTE));
    if (devDir == nullptr) {
        err_msg("Could not create devfs\n");
        System::halt();
    }

    rootDir->addDir("dev", devDir);

    IFile *devRand = (IFile *)RamFS::RandNumGen::create();
    if (devRand == nullptr) {
        err_msg("Could not create devfs random\n");
        System::halt();
    }

    devDir->addFile("random", devRand);
    devDir->addFile("urandom", devRand);

    PageTable::kernelPageTable()->dump();
    printf("System init done\n");

    Platform::setUart();
}

IDirectory *System::root() {
    return rootDir;
}

void System::initSecondaryCpu() {

}
