/***************************************************************************
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
 ***************************************************************************/


#include <stdint.h>

#include "arm/arm.h"
#include "arm/gic.h"
#include "fs/ramfs.h"
#include "config.h"

#include "kernel.h"
#include "tztask.h"
#include "scheduler.h"
#include "hwtimer.h"
#include "waitqueue.h"

#include "system.h"

extern "C" void tzKernelSecondary();

#define assert(cond) if (!(cond)) { err_msg("%s:%d - Assertion failed", __PRETTY_FUNCTION__, __LINE__); while (true) {} }

static WaitQueue wq;

TzTask *idler;
TzTask *userT;
TzTask *testT;

static int numCycles = 0;

void timerFired(Timer t, void *ctx) {
    printf("%s: cycle=%d timer=0x%x%x ctx=%p\n", __FUNCTION__,
           numCycles, (unsigned int)(t >> 32), (unsigned int)(t & 0xffffffff), ctx);

    wq.signal();
}

int idleTask(void *task, void *ctx) {
    UNUSED(task);
    UNUSED(ctx);

    printf("starting idler\n");
    ARCH_SPECIFIC_ENABLE_INTERRUPTS;
    ARCH_SPECIFIC_MEMORY_BARRIER;
    while (true) {
        asm volatile("wfi": : :);
    }

    return 0;
}

int testTask(void *task, void *ctx) {
    UNUSED(ctx);

    printf("starting test task\n");


    wq.init();

    while (numCycles < 10) {

        uint64_t now = TzHwCounter::timeNow();
        unsigned long freq = TzHwCounter::frequency();
        TzTimers::create(now+freq, timerFired, nullptr);

        printf("wait for task %p to awake\n", task);
        wq.wait((TzTask *)task);
        printf("woken up\n");

        numCycles++;
    }

    success_msg("Timer interrupt and context switch test passed\n");

    while (true)
        wq.wait((TzTask *)task);

    return 0;
}

void tzKernelInit(const void *devTree) {

    printf("%s: User space tests\n", __FUNCTION__);

    if (arm::smpCpuNum() != 0) {
        tzKernelSecondary();
        return;
    }

    System::init(devTree);

    IDirectory *root = System::root();
    IFile *userFile;
    IDirectory *dir;

    int rc = root->resolvePath("user.elf", &dir, &userFile);
    assert(rc == 0);
    assert(dir == nullptr);
    printf("%s: resolved user.elf\n", __FUNCTION__);
    char *argv[2] = {(char *)"user.elf", (char *)NULL};

    idler = new TzTask(idleTask, nullptr, 10, "idler");
    testT = new TzTask(testTask, nullptr, 50, "testT");
    userT = new TzTask(userFile, 50, dir, "userT", NULL, argv, argv);

    assert(idler);
    assert(testT);
    assert(userT);
    printf("%s: tasks created\n", __FUNCTION__);

    Scheduler::addTask(idler);
    Scheduler::addTask(testT);
    Scheduler::addTask(userT);

    ARCH_SPECIFIC_DISABLE_INTERRUPTS;

    schedule();
}

void tzKernelSecondary() {

    System::initSecondaryCpu();

    ARCH_SPECIFIC_DISABLE_INTERRUPTS;

    schedule();
}

void kernelHalt(const char *reason) {
    err_msg("%s\n", reason);
    while (true) {}
}

extern "C" void __cxa_pure_virtual() {
    err_msg("Pure virtual function called !\n");
    kernelHalt("Pure virtual function call");
}
