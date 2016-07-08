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

#define assert(cond) if (!(cond)) { err_msg("%s:%d - Assertion failed", __PRETTY_FUNCTION__, __LINE__); while (true) {} }

extern "C" unsigned long _binary_ramfs_cpio_start;

void tzKernelSecondary() {

    System::initSecondaryCpu();

    asm volatile("cpsid if":::);

    schedule();
}

void tzKernelInit(const void *devTree) {

    printf("%s: init for user applications...\n", __FUNCTION__);

    if (arm::smpCpuNum() != 0) {
        tzKernelSecondary();
        return;
    }

    System::init(devTree);

    // Enable user control of performance monitoring
    asm volatile("mcr p15, 0, %[val], c9, c14, 0" : :[val] "r"(0x1));

    IDirectory *root = System::root();
    IFile *uappdFile;
    IDirectory *dir;

    int rc = root->resolvePath("uappd.elf", &dir, &uappdFile);
    assert(rc == 0);
    assert(dir == nullptr);
    printf("%s: resolved uappd.elf\n", __FUNCTION__);

    TzTask *uappdTask;

    uappdTask = new TzTask(uappdFile, 50, dir, "uappdTask");
    if (uappdTask == nullptr) {
        err_msg("Uappd task creation failed\n");
        kernelHalt("Could not create uappd task");
    }
    if (uappdTask->status() != TzTask::State::Ready) {
        err_msg("Uappd task did not reach ready state\n");
        kernelHalt("Uappd task creation failed");
    }
    printf("%s: uappd task %d ready\n", __FUNCTION__, uappdTask->id());

    Scheduler::addTask(uappdTask);
    printf("%s: added uappd task to scheduler\n", __FUNCTION__);

    asm volatile("cpsid if":::);

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
