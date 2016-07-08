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
#include "svcutils.h"
#include "console.h"
#include "clock.h"
#include "system.h"

static uint8_t tzDevTree[MAX_DT_SIZE_BYTES];

#define assert(cond) if (!(cond)) { err_msg("%s:%d - Assertion failed", __PRETTY_FUNCTION__, __LINE__); while (true) {} }

TzTask *testT;

void tzKernelSecondary() {

    System::initSecondaryCpu();

    asm volatile("cpsid if":::);

    schedule();
}

void tzKernelInit(const void *devTree) {

    if (arm::smpCpuNum() != 0) {
        tzKernelSecondary();
        return;
    }

    printf("%s: fs user space tests\n", __FUNCTION__);

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

    System::init(tzDevTree);

    IDirectory *root = System::root();
    IFile *userHello;
    IDirectory *dir;
    rc = root->resolvePath("/unittests/fs_userspace/fs_tests.elf", &dir, &userHello);
    assert(rc == 0);
    assert(dir == nullptr);

    testT = new TzTask(userHello, 50, dir, "userHello");
    if (testT == nullptr) {
        err_msg("Init task creation failed\n");
        kernelHalt("Could not create init task");
    }
    else if (testT->status() != TzTask::State::Ready) {
        err_msg("Init task did not reach ready state\n");
        kernelHalt("Init task creation failed");
    }

    printf("tasks created\n");

    Scheduler::addTask(testT);

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
