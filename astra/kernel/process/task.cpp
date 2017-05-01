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
#include "atomic.h"
#include "fs/ramfs.h"
#include "lib_printf.h"

extern void *_system_link_base;

extern "C" uintptr_t _regBaseOffset;
uintptr_t _regBaseOffset;

extern "C" uintptr_t _noenRegBaseOffset;
uintptr_t _neonRegBaseOffset;

static ObjCacheAllocator<TzTask> allocator;
static uint32_t nextTaskNum = 0;

SpinLock TzTask::termLock;
tzutils::Vector<TzTask *> TzTask::tasks;

PerCPU<TzTask *> TzTask::nwProxyTask;

extern "C" uint32_t smcService(uint32_t);

static int nswTask(void *task, void *ctx) {
    UNUSED(task);
    UNUSED(ctx);
    ARCH_SPECIFIC_NSWTASK;
    return 0;
}

void TzTask::initNoTask()
{
    _regBaseOffset = (uintptr_t)&(((TzTask *) 0)->savedRegBase);
    _neonRegBaseOffset = (uintptr_t)&(((TzTask *) 0)->savedNeonRegBase);

    allocator.init();
    spinLockInit(&termLock);
    tasks.init();
}

void TzTask::init() {
    allocator.init();
    spinLockInit(&termLock);
    tasks.init();
    _regBaseOffset = (uintptr_t)&(((TzTask *) 0)->savedRegBase);
    _neonRegBaseOffset = (uintptr_t)&(((TzTask *) 0)->savedNeonRegBase);


    TzTask *nwTask = new TzTask(nswTask, nullptr, NS_WORLD_PRIORITY, "NWOS");
    if (nwTask == nullptr) {
        err_msg("Normal world task creation failed\n");
        kernelHalt("Could not create nw task");
    }
    else if (nwTask->state != TzTask::State::Ready) {
        err_msg("Normal world task did not reach ready state\n");
        kernelHalt("nw task creation failed");
    }

    nwProxyTask.cpuLocal() = nwTask;
    Scheduler::addTask(nwTask);
}

TzTask::IDType TzTask::nextTaskId() {
    IDType rv = atomic_incr(&nextTaskNum);
    return rv;
}

void TzTask::initSecondaryCpu() {
    TzTask *nwTask = new TzTask(nswTask, nullptr, NS_WORLD_PRIORITY, "NWOS");
    if (nwTask == nullptr) {
        err_msg("Normal world task creation failed\n");
        kernelHalt("Could not create nw task");
    }
    else if (nwTask->state != TzTask::State::Ready) {
        err_msg("Normal world task did not reach ready state\n");
        kernelHalt("nw task creation failed");
    }

    nwProxyTask.cpuLocal() = nwTask;

    // Do not add this task to the scheduler's run queue. The schedule() method
    // in scheduler object running on non-boot CPU,  will automatically pick up this
    // task.
}

void TzTask::allocateStack(uint32_t size) {

    PageTable *kernPageTable = PageTable::kernelPageTable();

    TzMem::VirtAddr stackVa = kernPageTable->reserveAddrRange((void *)KERNEL_STACKS_START, size, PageTable::ScanBackward);
    if (stackVa == nullptr) {
        err_msg("[stackVa 1] kernel virtual address space exhausted !\n");
        kernPageTable->dump();
        return;
    }

    uint8_t *currVa = (uint8_t *)stackVa;
    while (currVa < (uint8_t *)stackVa + size) {
        TzMem::PhysAddr currPa = TzMem::allocPage(KERNEL_PID);
        if (currPa == nullptr) {
            err_msg("system memory exhausted !\n");
            kernPageTable->unmapPageRange(stackVa, currVa - 1);
            kernPageTable->releaseAddrRange(stackVa, size);
            return;
        }

        kernPageTable->mapPage(currVa, currPa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true, false);
        pageTable->mapPage(currVa, currPa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true, true);

        currVa += PAGE_SIZE_4K_BYTES;
    }

    stackKernel = (uint8_t *)stackVa + size;
    stackKernelSize = size;
}

TzTask::TzTask(TaskFunction entry, void *ctx, unsigned int priority, const char *tname, TzTask *parentTask) :
                        pageTable(PageTable::kernelPageTable()), kernPageTable(PageTable::kernelPageTable()), parent(parentTask), taskClock(this),
                        uid(System::UID), gid(System::GID), seccompStrict(false) {

    mode = Mode::Kernel;
    state = State::Defunct;
    type = Type::CFS_Task;

    this->priority = priority;
    totalRunTime = 0;
    cumRunTime = 0;
    slotTimeSlice = 0;
    lastScheduledAt = TzHwCounter::timeNow();
    startedAt = TzHwCounter::timeNow();

    wqState.task = this;
    wqState.wq = nullptr;
    wqState.prev = nullptr;
    wqState.next = nullptr;

    /* Initialize the file descriptors */
    initFileTable();

    /* Initialize the current working directory */
    currWorkDir = System::root();

    initSignalState();

    /* Allocate the kernel mode stack */
#ifdef __aarch64__
    allocateStack(PAGE_SIZE_4K_BYTES * 2);
#else
    allocateStack(PAGE_SIZE_4K_BYTES);
#endif

    /* Reserve some quick pages */
    quickPagesMapped = false;
    quickPages = kernPageTable->reserveAddrRange((void *)KERNEL_STACKS_START, PAGE_SIZE_4K_BYTES*2, PageTable::ScanForward);
    if (quickPages == nullptr) {
        err_msg("[quickPages 1] kernel virtual address space exhausted !\n");
        pageTable->dump();
        return;
    }

    /* Prepare TLS region */
    threadInfo = nullptr;

    savedRegs[SAVED_REG_R0] = (unsigned long)this;
    savedRegs[SAVED_REG_R1] = (unsigned long)ctx;
    savedRegs[SAVED_REG_LR] = (unsigned long)entry;
    savedRegs[SAVED_REG_SP] = (unsigned long)stackKernel;

    register unsigned long spsr;
    ARCH_SPECIFIC_GET_SPSR(spsr);
    savedRegs[SAVED_REG_SPSR] = spsr;
#ifdef __aarch64__
    savedRegs[SAVED_REG_SPSR] = 0x285;
#endif
    savedRegBase = &savedRegs[NUM_SAVED_CPU_REGS];
    savedNeonRegBase = &savedNeonRegs[NUM_SAVED_NEON_REGS];

    this->parent = parentTask;
    spinLockInit(&lock);

    tid = nextTaskId();
    state = State::Ready;
    for (int i=0; i<7; i++)
        taskName[i] = tname[i];
    taskName[7] = 0;

    image = nullptr;

    setChildTid = nullptr;
    clearChildTid = nullptr;

    children.init();
    termCode = 0;
    termSignal = -1;
    childTermQueue.init();

    sleepUntil = 0;
    sleepTimer = INVALID_TIMER;

    brkStart = nullptr;
    brkCurr = nullptr;
    brkMax = nullptr;
    mmapMaxVa = nullptr;

    createUContext();

    //TODO: Potential thread unsafe publication. Can we move this outside
    // the constructor ?
    tasks.pushBack(this);

}

TzTask::TzTask(IFile *exeFile, unsigned int priority, IDirectory *workDir, const char *tname, TzTask *parentTask, char **argv, char **envp) :
                kernPageTable(PageTable::kernelPageTable()), parent(parentTask), taskClock(this),
                uid(System::UID), gid(System::GID), seccompStrict(false){

    tid = nextTaskId();
    image = ElfImage::loadElf(tid, exeFile, workDir);
    if (image == nullptr) {
        state = Defunct;
        err_msg("Could not load elf image\n");
        return;
    }

    pageTable = &image->userPageTable;

    mode = Mode::User;
    state = State::Defunct;
    type = Type::CFS_Task;

    this->priority = priority;
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
    setChildTid = nullptr;
    clearChildTid = nullptr;

    /* Initialize the file descriptors */
    initFileTable();

    /* Initialize the current working directory */
    if (workDir == nullptr)
        currWorkDir = System::root();
    else
        currWorkDir = workDir;

    initSignalState();

    /* Allocate the kernel mode stack */
#ifdef __aarch64__
    allocateStack(PAGE_SIZE_4K_BYTES * 2);
#else
    allocateStack(PAGE_SIZE_4K_BYTES);
#endif

    /* Reserve some quick pages */
    quickPagesMapped = false;
    quickPages = kernPageTable->reserveAddrRange((void *)KERNEL_STACKS_START, PAGE_SIZE_4K_BYTES*2, PageTable::ScanForward);
    if (quickPages == nullptr) {
        err_msg("[quickPages 2] kernel virtual address space exhausted !\n");
        pageTable->dump();
        return;
    }

    /*
     * Populate the user mode stack
     */

    int numArgs = 0;
    int numEnvs = 0;
    if (argv != nullptr) {
        for (int i=0; argv[i] != 0; i++) {
            numArgs++;
        }
    }
    if (envp != nullptr) {
        for (int i=0; envp[i] != 0; i++)
            numEnvs++;
    }
    unsigned long imageStackTop = (unsigned long)image->stackBase();
    TzMem::VirtAddr userStackVa = image->stackTopPageVA();

    TaskStartInfo tsInfo(image, tname, numArgs, argv, 0, nullptr);
    if (!tsInfo.constructed())
        return;

    int numBytesWritten = tsInfo.prepareUserStack(userStackVa);
    unsigned long userStackTop = imageStackTop - numBytesWritten;

    image->unmapStackFromKernel();

    /*
     * Prepare TLS region
     */
    TzMem::VirtAddr tiVa = kernPageTable->reserveAddrRange((void *)KERNEL_LOW_MEMORY, PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (tiVa == nullptr) {
        err_msg("No virtual address space left in kernel page table\n");
        return;
    }
    TzMem::PhysAddr tiPa = TzMem::allocPage(tid);
    if (tiPa == nullptr) {
        err_msg("system memory exhausted !\n");
        return;
    }

    kernPageTable->mapPage(tiVa, tiPa, MAIR_MEMORY, MEMORY_ACCESS_RW_USER, true, false);
    pageTable->mapPage(tiVa, tiPa, MAIR_MEMORY, MEMORY_ACCESS_RW_USER, true, true);
    threadInfo =(uint8_t *)tiVa + THREAD_INFO_OFFSET;

    savedRegs[SAVED_REG_LR] = (unsigned long)image->entry();
    savedRegs[SAVED_REG_SP] = (unsigned long)stackKernel;
    savedRegs[SAVED_REG_SP_USR] = userStackTop;
    savedRegs[SAVED_REG_LR_USR] = (unsigned long)image->entry();
#ifdef __aarch64__
    savedRegs[SAVED_REG_TPIDR_EL0] = (unsigned long)threadInfo;
#endif
    register unsigned long spsr = 0x80 | Mode_USR; // No IRQs. IRQs belong in normal world.
    if (savedRegs[SAVED_REG_LR] & 0x1) {
        // The executable is in thumb mode;
        spsr |= 0x20;
    }
    savedRegs[SAVED_REG_SPSR] = spsr;

    savedRegBase = &savedRegs[NUM_SAVED_CPU_REGS];
    savedNeonRegBase = &savedNeonRegs[NUM_SAVED_NEON_REGS];

    spinLockInit(&lock);

    state = State::Ready;
    for (int i=0; i<7; i++)
        taskName[i] = tname[i];
    taskName[7] = 0;

    children.init();
    termCode = 0;
    termSignal = -1;
    childTermQueue.init();

    sleepUntil = 0;
    sleepTimer = INVALID_TIMER;

    brkStart = image->dataSegmentBrk();
    brkCurr = brkStart;
    brkMax = (uint8_t *)brkStart + BRK_MAX;
    mmapMaxVa = &_system_link_base;

    vmCloned = false;
    threadCloned = false;

    createUContext();

    //TODO: Potential thread unsafe publication. Can we move this outside
    // the constructor ?
    tasks.pushBack(this);
}

TzTask::TzTask(TzTask& parentTask) :
        kernPageTable(PageTable::kernelPageTable()), parent(&parentTask), taskClock(this),
        uid(parentTask.uid), gid(parentTask.gid), seccompStrict(false) {

    tid = nextTaskId();

    image = ElfImage::loadElf(tid, parentTask.image);
    if (image == nullptr) {
        state = Defunct;
        err_msg("Could not allocate elf image\n");
        return;
    }

    pageTable = &image->userPageTable;

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
    setChildTid = nullptr;
    clearChildTid = nullptr;

    /* Copy the file descriptors */
    inheritFileTable(parentTask);

    /* Initialize the current working directory */
    currWorkDir = parentTask.currWorkDir;

    inheritSignalState(parentTask);

    /* Allocate the kernel mode stack */
#ifdef __aarch64__
    allocateStack(PAGE_SIZE_4K_BYTES * 2);
#else
    allocateStack(PAGE_SIZE_4K_BYTES);
#endif

    quickPagesMapped = false;
    quickPages = kernPageTable->reserveAddrRange((void *)KERNEL_STACKS_START, PAGE_SIZE_4K_BYTES*2, PageTable::ScanForward);
    if (quickPages == nullptr) {
        err_msg("[quickPages 3] kernel virtual address space exhausted !\n");
        pageTable->dump();
        return;
    }

    /*
     * Prepare TLS region
     */
    // MUSL has moved the parent TLS region to user-space,
    // child inherits the page with AllocOnWrite by default.
    threadInfo = parentTask.threadInfo;

    for (int i=0; i<NUM_SAVED_CPU_REGS; i++) {
        savedRegs[i] = parentTask.savedRegs[i];
    }

    for (int i=0; i<NUM_SAVED_NEON_REGS; i++) {
        savedNeonRegs[i] = parentTask.savedNeonRegs[i];
    }

    savedRegs[SAVED_REG_R0] = 0;
    savedRegs[SAVED_REG_SP] = (unsigned long)stackKernel;
    savedRegBase = &savedRegs[NUM_SAVED_CPU_REGS];
    savedNeonRegBase = &savedNeonRegs[NUM_SAVED_NEON_REGS];

    spinLockInit(&lock);

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
    brkCurr = parentTask.brkCurr;
    brkMax = parentTask.brkMax;
    mmapMaxVa = parentTask.mmapMaxVa;

    vmCloned = false;
    threadCloned = false;

    createUContext();

    //TODO: Potential thread unsafe publication. Can we move this outside
    // the constructor ?
    tasks.pushBack(this);
}

void yieldCurrentTask(unsigned int reason, unsigned int spsr) {

    TzTask *currTask = currentTask[arm::smpCpuNum()];
    unsigned int mode = spsr & 0x1f;

    if ((reason == 0 && (mode != Mode_USR && mode != 5)) ||
        (reason == 2 && mode == Mode_SVC)) {
        err_msg("Exceptions taken in wrong mode, reason %d, mode %d\n", reason, mode);
        asm volatile("b .":::);
    }

    currTask->state = TzTask::Ready;
    currTask->savedRegBase += NUM_SAVED_CPU_REGS;
    currTask->savedNeonRegBase += NUM_SAVED_NEON_REGS;
    Scheduler::addTask(currTask);
}

void printReg()
{
    TzTask *currTask = currentTask[arm::smpCpuNum()];

    //printf("Current task id=%d : Prinitng User Reg %lx\n",currTask->id(),sizeof(unsigned long));

    currTask->printURegs();
}

bool TzTask::getScheduler(int *policy, int *schedPriority){

    *policy = type;
    *schedPriority = priority;
    return true;
}

bool TzTask::setScheduler(int policy, int schedPriority){

    if((policy != SCHED_OTHER) && (policy != SCHED_DEADLINE))
        policy = SCHED_OTHER;

    if(type != policy){
        totalRunTime = 0;
        cumRunTime = 0;
        slotTimeSlice = 0;
        lastScheduledAt = TzHwCounter::timeNow();
    }

    type = (Type) policy;
    priority = schedPriority;
    return true;
}
TzTask *TzTask::current() {
    return currentTask[arm::smpCpuNum()];
}

void TzTask::yield(){
    spinLockAcquire(&lock);

    state = State::Wait;

    //printf("yield %d\n", (int) this->id() & 0xFFFFFFFF);
    Scheduler::removeTask(this);

    unsigned long *cpuIdx = savedRegBase;
    long double *neonIdx = savedNeonRegBase;
    savedRegBase += NUM_SAVED_CPU_REGS;
    savedNeonRegBase += NUM_SAVED_NEON_REGS;

    spinLockRelease(&lock);

    ARCH_SPECIFIC_SAVE_STATE(neonIdx, cpuIdx);
}

int TzTask::setThreadArea(struct user_desc *desc) {
    int tlsIdx = desc->entry_number;
    if (tlsIdx == -1) {
        for (int i=0; i<MAX_NUM_TLS_ENTRIES; i++) {
            if (tlsEntry[i].entry_number != -1)
                continue;

            tlsIdx = i;
            break;
        }
    }

    if (tlsIdx == -1) {
        return -ESRCH;
    }

    desc->entry_number = tlsIdx;
    memcpy(&tlsEntry[tlsIdx], desc, sizeof(struct user_desc));

    return 0;
}

int TzTask::getThreadArea(struct user_desc *desc) {
    int tlsIdx = desc->entry_number;
    if ((tlsIdx < 0) || (tlsIdx >= MAX_NUM_TLS_ENTRIES))
        return -EINVAL;

    memcpy(desc, &tlsEntry[tlsIdx], sizeof(struct user_desc));
    return 0;
}

int TzTask::setTidAddress(int *tidPtr) {
    clearChildTid = tidPtr;

    TzMem::PhysAddr pa = pageTable->lookUp(tidPtr);
    TzMem::VirtAddr va = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (va == nullptr) {
        err_msg("Ran out of user virtual address space\n");
        System::halt();
    }

    kernPageTable->mapPage(va, pa);
    IDType *sct = (IDType *)((uint8_t *)va + ((uint8_t *)tidPtr - (uint8_t *)PAGE_START_4K(tidPtr)));
    *sct = tid;
    kernPageTable->unmapPage(va);

    return tid;
}

void TzTask::dataAbortException() {
    volatile register uintptr_t dfsr, dfar, align;
    ARCH_SPECIFIC_DATA_ABORT_EXCEPTION(dfsr, dfar, align);

    //printf("Task %d %p Invalid data access or COW needed. DFAR 0x%zx DFSR 0x%zx align=0x%zx\n", tid, this, dfar, dfsr, align);
    //printf("LR 0x%lx SPSR 0x%lx\n", savedRegs[SAVED_REG_LR], savedRegs[SAVED_REG_SPSR]);

    if (IS_CACHE_MAINTENANCE_FAULT(dfsr)) {
        err_msg("Cache maintenance fault: DFAR 0x%zx\n", dfar);
        kernelHalt("Bad cache access\n");
    }

    if (IS_EXT_DATA_ABORT(dfsr)) {
        err_msg("External data abort: DFAR 0x%zx\n", dfar);
        printf("LR 0x%lx\n", savedRegs[SAVED_REG_LR]);
        kernelHalt("Bad memory access\n");
    }

    if (!IS_WRITE_DATA_ABORT(dfsr)) {

        // err_msg("Task %d %p: Got unexpected DFSR 0x%x DFAR 0x%x\n", tid, this, dfsr, dfar);
        // printf("LR 0x%lx\n", savedRegs[SAVED_REG_LR]);
        err_msg("[1] Terminating Task %d for invalid memory access\n", tid);
        // PageTable::kernelPageTable()->dump();
        pageTable->dump();
        printURegs();

        siginfo_t info;
        memset(&info, 0, sizeof(siginfo_t));
        info.si_signo = SIGSEGV;
        info.si_addr = (void *)dfar;
        sigQueue(SIGSEGV, &info);

        return;
    }

    TzMem::VirtAddr faultVA = (TzMem::VirtAddr) dfar;
    PageTable::EntryAttribs attribs;
    TzMem::PhysAddr faultPA = pageTable->lookUp(faultVA, &attribs);
    if (faultPA == nullptr) {

        //err_msg("%p: Got data access to unmapped %p DFSR 0x%x \n", this, faultVA, dfsr);
        //printf("LR 0x%lx\n", savedRegs[SAVED_REG_LR]);
        err_msg("[2] Terminating Task %d for invalid memory access\n", tid);
        pageTable->dump();
        printURegs();

        siginfo_t info;
        memset(&info, 0, sizeof(siginfo_t));
        info.si_signo = SIGSEGV;
        info.si_addr = (void *)dfar;
        sigQueue(SIGSEGV, &info);

        return;
    }

    if (attribs.swAttribs != PageTable::SwAttribs::AllocOnWrite)  {

        err_msg("[%d] Got data access to %p DFSR 0x%zx. Bad attribs %d \n", tid, faultVA, dfsr, attribs.swAttribs);
        pageTable->dump();
        printf("parent page table:\n");
        parent->pageTable->dump();

        kernelHalt("Unexpected data fault on attributes\n");
    }

    //printf("%s: faultPA %p attribs %d\n", __PRETTY_FUNCTION__, faultPA, attribs.swAttribs);

    // This is a copy-on-write fault. Perform the copy
    PageTable *kernPageTable = PageTable::kernelPageTable();

    TzMem::VirtAddr srcPageVA = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (srcPageVA == nullptr) {
        err_msg("[cow srcPageVa] kernel virtual address space exhausted !\n");
        kernelHalt("VA exhaustion\n");
    }
    //printf("%s: SRC map va %p to pa %p\n", __PRETTY_FUNCTION__, srcPageVA, PAGE_START_4K(faultPA));
    kernPageTable->mapPage(srcPageVA, PAGE_START_4K(faultPA), MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL);

    TzMem::VirtAddr destPageVA = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (destPageVA == nullptr) {
        err_msg("[cow destPageVa] kernel virtual address space exhausted !\n");
        kernelHalt("VA exhaustion\n");
    }
    TzMem::PhysAddr destPagePA = TzMem::allocPage(tid);
    if (destPagePA == nullptr) {
        err_msg("system memory exhausted !\n");
        kernelHalt("Memory exhaustion\n");;
    }
    //printf("%s: DEST map va %p to pa %p\n", __PRETTY_FUNCTION__, destPageVA, destPagePA);
    kernPageTable->mapPage(destPageVA, destPagePA, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL);

    memcpy(destPageVA, srcPageVA, PAGE_SIZE_4K_BYTES);

    //printf("Unmapping %p\n", srcPageVA);
    kernPageTable->unmapPage(srcPageVA);
    //printf("Unmapping %p\n", destPageVA);
    kernPageTable->unmapPage(destPageVA);

    //printf("Task %d: copyOnWrite page %p\n", tid, PAGE_START_4K(faultVA));
    pageTable->mapPage(PAGE_START_4K(faultVA), destPagePA, MAIR_MEMORY, MEMORY_ACCESS_RW_USER, true);
}

extern "C" void taskAwaken(void *task);
void taskAwaken(Timer timer, void *t) {
    UNUSED(timer);
    TzTask *task = (TzTask *)t;
    task->awaken();
}

void TzTask::nanosleep(unsigned long ticks, unsigned long *remain) {
    if (signalled)
        return;

    sleepUntil = TzHwCounter::timeNow() + ticks;
    sleepTimer = TzTimers::create(sleepUntil, taskAwaken, this);

    yield();

    PageTable::kernelPageTable()->activate();
    Scheduler::addTask(this);

    uint64_t now = TzHwCounter::timeNow();
    *remain =  (now < sleepUntil) ? (sleepUntil - now) : 0;

    // printf("----> done sleeping. savedRegBase %p savedRegs %p\n", savedRegBase, savedRegs);

    //printf("nanosleep task %d done \n",this->id());
    sleepUntil = 0;
    if (*remain)
        TzTimers::destroy(sleepTimer);
}

void TzTask::awaken() {

    PageTable::kernelPageTable()->activate();
    SpinLocker locker(&lock);

    //printf("Wake up %d\n", this->id());
    if (state == State::Ready)
        return;

    state = State::Ready;
    Scheduler::addTask(this);
}

int TzTask::changeGroup(uint16_t newGroup) {
    UNUSED(newGroup);
    return -EPERM;
}

int TzTask::changeOwner(uint16_t newOwner) {
    if (uid != System::UID)
        return -EPERM;

    uid = newOwner;
    return 0;
}

int TzTask::pause() {

    yield();

    PageTable::kernelPageTable()->activate();
    return -EINTR;
}

int TzTask::nice(int incr) {
    //
    // Our scheduler uses a priority range of [1..100]
    // Higher priority number means increased proportion of a
    // timeslice. This is the opposite of traditional Unix
    // semantics. Hence we subtract the incr whereas the Nice spec
    // says it should be added.
    if (priority <= incr)
        priority = 1;
    else
        priority -= incr;

    return 0;
}

extern "C" void contextSwitch(long double *, unsigned long *);

void TzTask::run() {
    /*
     * NOTE: This function changes the page table. Do not
     * call any non-inlined functions from here. The stack
     * may not always be in a usable state. Do not call printf() !
     */

    state = TzTask::Running;
    COMPILER_BARRIER();

    // Set the threadinfo register: TPIDRURO
    register unsigned long ti = (unsigned long)threadInfo;
    ARCH_SPECIFIC_SET_TPIDRURO(ti);
    COMPILER_BARRIER();

    pageTable->activate();

    // Observed stack related calls here due to compiler reordering.
    // Adding compiler barrier below to prevent any code reordering.
    COMPILER_BARRIER();

    savedRegBase -= NUM_SAVED_CPU_REGS;
    savedNeonRegBase -= NUM_SAVED_NEON_REGS;
    COMPILER_BARRIER();

    contextSwitch(savedNeonRegBase, savedRegBase);
}

void *TzTask::operator new(size_t sz) {
    UNUSED(sz);
    void *rv = allocator.alloc();
    return rv;
}

void TzTask::operator delete(void *task) {
    allocator.free((TzTask *)task);
}

void TzTask::terminationLock() {
    spinLockAcquire(&termLock);
}

void TzTask::terminationUnlock() {
    spinLockRelease(&termLock);
}
