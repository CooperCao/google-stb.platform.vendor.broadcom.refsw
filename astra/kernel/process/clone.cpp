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
 * clone.cpp
 *
 *  Created on: Mar 26, 2015
 *      Author: gambhire
 */
#include "hwtimer.h"
#include "arm/arm.h"
#include "arm/spinlock.h"

#include "tztask.h"
#include "tzmemory.h"
#include "objalloc.h"
#include "kernel.h"
#include "scheduler.h"
#include "console.h"
#include "cpulocal.h"
#include "fs/ramfs.h"

#include "tzclone.h"

#include "lib_printf.h"

TzTask::TzTask(TzTask& parentTask, unsigned long flags, void *stack, void *ptid, void *ctid, void *tls) :
        kernPageTable(PageTable::kernelPageTable()),
        parent(&parentTask), taskClock(this),
        uid(parentTask.uid), gid(parentTask.gid), seccompStrict(false) {

    tid = nextTaskId();
    if ((flags & CLONE_PARENT) || (flags & CLONE_THREAD)){
        parent = parentTask.parent;
    }

    if (flags & CLONE_THREAD)
        threadCloned = true;
    else
        threadCloned = false;

    if (flags & CLONE_VM) {
        image = parentTask.image;
        pageTable = parentTask.pageTable;

        vmCloned = true;
    }
    else {
        image = new ElfImage(tid, parentTask.image);
        if (image == nullptr) {
            state = Defunct;
            err_msg("Could not allocate elf image\n");
            return;
        }
        if (image->status != ElfImage::Valid) {
            state = Defunct;
            err_msg("Could not load elf image. status %d\n", image->status);
            return;
        }

        vmCloned = false;
        pageTable = &image->userPageTable;
    }

    mode = Mode::User;
    state = State::Defunct;
    type = parentTask.type;

    this->priority = parentTask.priority;
    totalRunTime = 0;
    cumRunTime = 0;
    slotTimeSlice = 0;
    lastScheduledAt = TzHwCounter::timeNow();
    startedAt = TzHwCounter::timeNow();

    wqState.task = this;
    wqState.wq = nullptr;
    wqState.prev = nullptr;
    wqState.next = nullptr;

    /* Initialize thread local storage */
    for (int i=0; i<MAX_NUM_TLS_ENTRIES; i++) {
        memset(&tlsEntry[i], 0, sizeof(struct user_desc));
        tlsEntry[i].entry_number = -1;
    }

    if (flags & CLONE_CHILD_CLEARTID) {
        clearChildTid = (int *)ctid;

        TzMem::PhysAddr pa = pageTable->lookUp(ctid);
        TzMem::VirtAddr va = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
        if (va == nullptr) {
            err_msg("Ran out of user virtual address space\n");
            System::halt();
        }

        kernPageTable->mapPage(va, pa);
        IDType *sct = (IDType *)((uint8_t *)va + ((uint8_t *)ctid - (uint8_t *)PAGE_START_4K(ctid)));
        *sct = tid;
        kernPageTable->unmapPage(va);
    }
    else {
        clearChildTid = nullptr;
    }

    if (flags & CLONE_CHILD_SETTID) {
        setChildTid = (int *)ctid;

        TzMem::PhysAddr pa = pageTable->lookUp(ctid);
        TzMem::VirtAddr va = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
        if (va == nullptr) {
            err_msg("Ran out of user virtual address space\n");
            System::halt();
        }

        kernPageTable->mapPage(va, pa);
        IDType *sct = (IDType *)((uint8_t *)va + ((uint8_t *)ctid - (uint8_t *)PAGE_START_4K(ctid)));
        *sct = tid;
        kernPageTable->unmapPage(va);
    }
    else {
        setChildTid = nullptr;
    }

    if (flags & CLONE_PARENT_SETTID) {
        TzMem::PhysAddr pa = pageTable->lookUp(ptid);
        TzMem::VirtAddr va = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
        if (va == nullptr) {
            err_msg("Ran out of user virtual address space\n");
            System::halt();
        }

        kernPageTable->mapPage(va, pa);
        IDType *sct = (IDType *)((uint8_t *)va + ((uint8_t *)ptid - (uint8_t *)PAGE_START_4K(ptid)));
        *sct = tid;
        kernPageTable->unmapPage(va);
    }

    if (flags & CLONE_FILES) {
        files = parentTask.files;
        filesCloned = true;
    }
    else {
        inheritFileTable(parentTask);
    }

    /* Initialize the current working directory */
    currWorkDir = parentTask.currWorkDir;

    if (flags & CLONE_SIGHAND) {
        signals = parentTask.signals;
        signalsCloned = true;

        sigParamsStack = parentTask.sigParamsStack;
        sigParamsStackTop = parentTask.sigParamsStackTop;
        sigParamsStackMax = parentTask.sigParamsStackMax;

        signalled = false;
    }
    else {
        inheritSignalState(parentTask);
        signalsCloned = false;
    }

    /*
     * Allocate the kernel mode stack
     */
    PageTable *kernPageTable = PageTable::kernelPageTable();

    TzMem::VirtAddr stackVa = kernPageTable->reserveAddrRange((void *)KERNEL_STACKS_START, PAGE_SIZE_4K_BYTES, PageTable::ScanBackward);
    if (stackVa == nullptr) {
        err_msg("[stackVa 3] kernel virtual address space exhausted !\n");
        pageTable->dump();
        return;
    }
    TzMem::PhysAddr stackPa = TzMem::allocPage(KERNEL_PID);
    if (stackPa == nullptr) {
        err_msg("system memory exhausted !\n");
        return;
    }

    kernPageTable->mapPage(stackVa, stackPa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL);
    pageTable->mapPage(stackVa, stackPa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL);

    TzMem::VirtAddr stackStart = (uint8_t *)stackVa + PAGE_SIZE_4K_BYTES;
    stackKernel = stackStart;

    /*
     * Prepare TLS region
     */
    threadInfo = tls;

    for (int i=0; i<NUM_SAVED_CPU_REGS; i++) {
        savedRegs[i] = parentTask.savedRegs[i];
    }
    savedRegs[SAVED_REG_R0] = 0;
    savedRegs[SAVED_REG_SP] = (unsigned long)stackKernel;

    if (vmCloned)
        savedRegs[SAVED_REG_SP_USR] = (unsigned long)stack;

    savedRegBase = &savedRegs[NUM_SAVED_CPU_REGS];

    spinlock_init("task.lock", &lock);

    state = State::Ready;
    for (int i=0; i<7; i++)
        taskName[i] = parentTask.taskName[i];
    taskName[0] = 'c';
    taskName[7] = 0;
    sprintf(taskName, "c%d", tid);

    children.init();

    if (!(flags & CLONE_THREAD))
        parentTask.children.pushBack(this);

    termCode = 0;
    termSignal = -1;
    childTermQueue.init();

    sleepUntil = 0;
    sleepTimer = INVALID_TIMER;

    brkStart = parentTask.brkStart;
    brkCurr = parentTask.brkCurr;;
    brkMax = parentTask.brkMax;
    mmapMaxVa = parentTask.mmapMaxVa;

    //TODO: Potential thread unsafe publication. Can we move this outside
    // the constructor ?
    tasks.pushBack(this);

}

#if 0

TzTask::TzTask(const TzTask& parentTask, unsigned long flags, void *stack, void *ptid, void *ctid) :
    taskClock(this), uid(parentTask.uid), gid(parentTask.gid) {

    kernPageTable = PageTable::kernelPageTable();

    tid = nextTaskId();

    image = new ElfImage(nextTaskId, parentTask.image);
    if (image == nullptr) {
        state = Defunct;
        err_msg("Could not allocate elf image\n");
        return;
    }
    if (image->status != ElfImage::Valid) {
        state = Defunct;
        err_msg("Could not load elf image. status %d\n", image->status);
        return;
    }

    if (flags & CLONE_VM) {
        pageTable = parentTask.pageTable;
    }
    else {
        pageTable = &image->userPageTable;
    }

    mode = Mode::User;
    state = State::Defunct;

    this->priority = parentTask.priority + 1;
    totalRunTime = 0;
    lastScheduledAt = 0;
    startedAt = TzHwCounter::timeNow();

    wqState.task = this;
    wqState.wq = nullptr;
    wqState.prev = nullptr;
    wqState.next = nullptr;

    /* Initialize thread local storage */
    for (int i=0; i<MAX_NUM_TLS_ENTRIES; i++) {
        memset(&tlsEntry[i], 0, sizeof(struct user_desc));
        tlsEntry[i].entry_number = -1;
    }

    if (flags & CLONE_CHILD_CLEARTID) {
        clearChildTid = (int *)ctid;
        *clearChildTid = tid;
    }
    if (flags & CLONE_CHILD_SETTID) {
        setChildTid = (int *)ctid;
        *setChildTid = tid;
    }


    if (flags & CLONE_FILES) {
        files = parentTask.files;
        filesCloned = true;
    }
    else {
        inheritFileTable(parentTask);
    }

    /* Initialize the current working directory */
    currWorkDir = parentTask.currWorkDir;

    if (flags & CLONE_SIGHAND) {
        signals = parentTask.signals;
        signalsCloned = true;
    }
    else {
        inheritSignalState(parentTask);
    }

    /*
     * Allocate the kernel mode stack
     */
    PageTable *kernPageTable = PageTable::kernelPageTable();

    TzMem::VirtAddr stackVa = kernPageTable->reserveAddrRange((void *)KERNEL_STACKS_START, PAGE_SIZE_4K_BYTES, PageTable::ScanBackward);
    if (stackVa == nullptr) {
        err_msg("[stackVa 3] kernel virtual address space exhausted !\n");
        pageTable->dump();
        return;
    }
    TzMem::PhysAddr stackPa = TzMem::allocPage(KERNEL_PID);
    if (stackPa == nullptr) {
        err_msg("system memory exhausted !\n");
        return;
    }

    kernPageTable->mapPage(stackVa, stackPa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL);
    pageTable->mapPage(stackVa, stackPa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL);

    TzMem::VirtAddr stackStart = (uint8_t *)stackVa + PAGE_SIZE_4K_BYTES;
    stackKernel = stackStart;

    /*
     * Prepare TLS region
     */
    TzMem::PhysAddr tiPA = TzMem::allocPage(tid);
    if (tiPA == nullptr) {
        err_msg("system memory exhausted !\n");
        return;
    }

    TzMem::VirtAddr userStackVa = image->stackTopPageVA();

    threadInfo = pageTable->reserveAddrRange(userStackVa, PAGE_SIZE_4K_BYTES, PageTable::ScanDirection::ScanForward);
    if (threadInfo == nullptr) {
        err_msg("No virtual address space left in user page table\n");
        return;
    }
    pageTable->mapPage(threadInfo, tiPA, MAIR_MEMORY, MEMORY_ACCESS_RW_USER);
    threadInfo =(uint8_t *)threadInfo + THREAD_INFO_OFFSET;

    for (int i=0; i<NUM_SAVED_CPU_REGS; i++) {
        savedRegs[i] = parentTask.savedRegs[i];
    }
    savedRegs[SAVED_REG_R0] = 0;
    savedRegs[SAVED_REG_SP] = (unsigned long)stackKernel;
    savedRegBase = &savedRegs[NUM_SAVED_CPU_REGS];

    spinlock_init("task.lock", &lock);

    state = State::Ready;
    for (int i=0; i<7; i++)
        taskName[i] = parentTask.taskName[i];
    taskName[0] = 'c';
    taskName[7] = 0;
    sprintf(taskName, "c%d", tid);

    children.init();
    parentTask.children.pushBack(this);
    termCode = 0;
    termSignal = -1;
    childTermQueue.init();

    sleepUntil = 0;
    sleepTimer = INVALID_TIMER;

    brkStart = parentTask.brkStart;
    brkCurr = parentTask.brkCurr;;
    brkMax = parentTask.brkMax;
    mmapMaxVa = parentTask.mmapMaxVa;

    //TODO: Potential thread unsafe publication. Can we move this outside
    // the constructor ?
    tasks.pushBack(this);
}

#endif
