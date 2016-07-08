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

#include "kernel.h"
#include "tzmemory.h"
#include "pgtable.h"
#include "objalloc.h"
#include "interrupt.h"
#include "tztask.h"
#include "scheduler.h"
#include "console.h"
#include "svcutils.h"

static uint8_t tzDevTree[MAX_DT_SIZE_BYTES];

#define assert(cond) if (!(cond)) { err_msg("%s:%d - Assertion failed", __PRETTY_FUNCTION__, __LINE__); while (true) {} }

TzTask *idler;
TzTask *testT;
TzTask *yieldT;

static int numCycles = 0;
static WaitQueue eq;

void timerFired(Timer t, void *ctx) {
    printf("%s: %x%x %p\n", __FUNCTION__, (unsigned int)(t >> 32),
                (unsigned int)(t & 0xffffffff), ctx);

    eq.signal();
}

int idleTask(void *task, void *ctx) {
    UNUSED(task);
    UNUSED(ctx);
    printf("starting idler\n");
    asm volatile("cpsie af":::);
    while (true) {
        //asm volatile("wfi": : :);
        asm volatile("smc #0": : :);
        printf("w");
    }

    return 0;
}

int testTask(void *task, void *ctx) {

    printf("in task\n");
    UNUSED(ctx);

    eq.init();

    while (true) {

        printf("await: task %p\n", task);

        eq.wait((TzTask *)task);

        printf("woken up\n");

        uint64_t now = TzHwCounter::timeNow();
        unsigned long freq = TzHwCounter::frequency();

        TzTimers::create(now+freq, timerFired, nullptr);

        numCycles++;
    }

    success_msg("Timer interrupt and context switch test passed\n");

    while (true)
        eq.wait((TzTask *)task);

    return 0;
}


extern "C" unsigned long _binary_ramfs_cpio_start;

void tzKernelInit(const void *devTree) {

    printf("%s: user space tests\n", __FUNCTION__);

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

    TzMem::init(tzDevTree);
    PageTable::init(devTree);
    GIC::init(tzDevTree);
    Interrupt::init();
    TzTask::init();
    ElfImage::init();
    TzTimers::init();
    Scheduler::init();
    RamFS::init();
    SysCalls::init();
    Console::init(tzDevTree);

    IDirectory *root = RamFS::load(&_binary_ramfs_cpio_start);
    IFile *userHello;
    IDirectory *dir;
    rc = root->resolvePath("hello.elf", &dir, &userHello);
    assert(rc == 0);
    assert(dir == nullptr);

    testT = new TzTask(userHello, 50, dir, "userHello");
    idler = new TzTask(idleTask, nullptr, 10, "idler");
    yieldT = new TzTask(testTask, nullptr, 50, "yieldT");

    printf("tasks created\n");

    Scheduler::addTask(testT);
    Scheduler::addTask(idler);
    Scheduler::addTask(yieldT);

    TzTimers::create(TzHwCounter::timeNow()+(25*TzHwCounter::frequency()), timerFired, nullptr);

    asm volatile("cpsid if":::);

    schedule();
}

void kernelHalt(const char *reason) {

    UNUSED(reason);
    while (true) {}
}

extern "C" void __cxa_pure_virtual() {
    err_msg("Pure virtual function called !\n");
    kernelHalt("Pure virtual function call");
}
