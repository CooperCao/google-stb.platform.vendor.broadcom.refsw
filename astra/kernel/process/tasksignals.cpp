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
 * tasksignals.cpp
 *
 *  Created on: Jan 30, 2015
 *      Author: gambhire
 */

#include <hwtimer.h>
#include <ioctl.h>

#include "arm/spinlock.h"

#include "tztask.h"
#include "tzmemory.h"
#include "objalloc.h"
#include "kernel.h"
#include "scheduler.h"
#include "console.h"
#include "eventqueue.h"
#include "wait.h"
#include "pgtable.h"

#include "lib_printf.h"

void TzTask::initSignalState() {

    PageTable *kernelPageTable = PageTable::kernelPageTable();

    // Allocate the signals array on the heap
    int numPages = (sizeof(Signal) * NumSignals)/PAGE_SIZE_4K_BYTES + 1;
    TzMem::VirtAddr va = kernelPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, numPages*PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (va == nullptr) {
        err_msg("Ran out of user virtual address space\n");
        System::halt();
    }

    signals = (Signal *)va;

    for (int i=0; i<numPages; i++) {
        TzMem::PhysAddr pa = TzMem::allocPage(tid);
        if (pa == nullptr) {
            err_msg("Out of memory\n");
            System::halt();
        }

        kernelPageTable->mapPage(va, pa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true, false);
        va = (uint8_t *)va + PAGE_SIZE_4K_BYTES;
    }
    //printf("signals array at %p\n", signals);

    for (int i=0; i<NumSignals; i++) {
        signals[i].handler = nullptr;
        signals[i].status = 0;
    }

    signals[SIGHUP].defaultAction = Terminate;
    signals[SIGINT].defaultAction = Terminate;
    signals[SIGQUIT].defaultAction = Terminate;
    signals[SIGPIPE].defaultAction = Terminate;
    signals[SIGALRM].defaultAction = Terminate;
    signals[SIGTERM].defaultAction = Terminate;
    signals[SIGUSR1].defaultAction = Terminate;
    signals[SIGSEGV].defaultAction = Terminate;
    signals[SIGUSR2].defaultAction = Terminate;
    signals[SIGCHLD].defaultAction = Ignore;
    signals[SIGCONT].defaultAction = ContinueTask;
    signals[SIGSTOP].defaultAction = StopTask;
    signals[SIGTSTP].defaultAction = StopTask;
    signals[SIGTTIN].defaultAction = StopTask;
    signals[SIGTTOU].defaultAction = StopTask;
    signals[SIGPOLL].defaultAction = Terminate;
    signals[SIGPROF].defaultAction = Terminate;
    signals[SIGVTALRM].defaultAction = Terminate;
    signals[SIGWINCH].defaultAction = Ignore;
    signals[SIGIO].defaultAction = Terminate;
    signals[SIGPWR].defaultAction = Terminate;


    va = kernelPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (va == nullptr) {
        err_msg("Ran out of user virtual address space\n");
        System::halt();
    }
    TzMem::PhysAddr pa = TzMem::allocPage(tid);
    if (pa == nullptr) {
        err_msg("Out of memory\n");
        System::halt();
    }

    kernelPageTable->mapPage(va, pa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true, false);
    pageTable->mapPage(va, pa, MAIR_MEMORY, MEMORY_ACCESS_RO_USER, true, true);

    sigParamsStack = (SigReturn *)va;
    sigParamsStackTop = -1;
    sigParamsStackMax = PAGE_SIZE_4K_BYTES/sizeof(SigReturn);

    signalled = false;
    signalsCloned = false;

    //printf("signals ret stack at %p\n", sigParamsStack);
}

void TzTask::inheritSignalState(const TzTask& parent) {
    PageTable *kernelPageTable = PageTable::kernelPageTable();

    // Allocate the signals array on the heap
    int numPages = (sizeof(Signal) * NumSignals)/PAGE_SIZE_4K_BYTES + 1;
    TzMem::VirtAddr va = kernelPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, numPages*PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (va == nullptr) {
        err_msg("Ran out of user virtual address space\n");
        System::halt();
    }

    signals = (Signal *)va;

    for (int i=0; i<numPages; i++) {
        TzMem::PhysAddr pa = TzMem::allocPage(tid);
        if (pa == nullptr) {
            err_msg("Out of memory\n");
            System::halt();
        }

        kernelPageTable->mapPage(va, pa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true, false);
        va = (uint8_t *)va + PAGE_SIZE_4K_BYTES;
    }

    for (int i=0; i<NumSignals; i++) {
        signals[i].handler = parent.signals[i].handler;
        signals[i].restorer = parent.signals[i].restorer;
        signals[i].defaultAction = parent.signals[i].defaultAction;
        signals[i].haveSigInfo = false;
        signals[i].status = parent.signals[i].status;
    }

    va = kernelPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (va == nullptr) {
        err_msg("Ran out of user virtual address space\n");
        System::halt();
    }
    TzMem::PhysAddr pa = TzMem::allocPage(tid);
    if (pa == nullptr) {
        err_msg("Out of memory\n");
        System::halt();
    }

    kernelPageTable->mapPage(va, pa, MAIR_MEMORY, MEMORY_ACCESS_RO_USER, true, false);
    pageTable->mapPage(va, pa, MAIR_MEMORY, MEMORY_ACCESS_RO_USER, true, true);

    sigParamsStack = (SigReturn *)va;
    sigParamsStackTop = -1;
    sigParamsStackMax = PAGE_SIZE_4K_BYTES/sizeof(SigReturn);

    signalled = false;
    signalsCloned = false;
}

int TzTask::sigprocmask(int how, const sigset_t *set, sigset_t *oldSet) {

    // Clear the old set.
    for (int i=0; i<NumSignals/sizeof(unsigned long); i++) {
        oldSet->bits[i] = 0;
    }

    // Now fill it with the current blocked vs. unblocked status
    // of all the signals.
    int wordNum = 0;
    int bitNum = 0;
    for (int i=0; i<NumSignals; i++) {
        uint8_t blocked = (signals[i].status & SIGNAL_BLOCKED);
        oldSet->bits[wordNum] |= (blocked << bitNum);

        bitNum = (bitNum + 1)%sizeof(unsigned long);
        if (bitNum == 0)
            wordNum++;
    }

    if (set == nullptr)
        return 0;

    if (how == SIG_BLOCK) {
        // If bit n of set->bits is 1, block signal number n.
        wordNum = 0;
        bitNum = 0;
        for (int i=0; i<NumSignals; i++) {
            uint8_t block = (set->bits[wordNum] >> bitNum) & 1;
            if (block)
                signals[i].status |= SIGNAL_BLOCKED;

            bitNum = (bitNum + 1)%sizeof(unsigned long);
            if (bitNum == 0)
                wordNum++;
        }

        return 0;
    }

    if (how == SIG_UNBLOCK) {
        // If bit n of set->bits is 1, unblock signal number n.
        wordNum = 0;
        bitNum = 0;
        for (int i=0; i<NumSignals; i++) {
            uint8_t unblock = (set->bits[wordNum] >> bitNum) & 1;
            if (unblock)
                signals[i].status &= ~SIGNAL_BLOCKED;

            bitNum = (bitNum + 1)%sizeof(unsigned long);
            if (bitNum == 0)
                wordNum++;
        }

        return 0;
    }

    if (how == SIG_SETMASK) {
        // If bit n of set->bits is 1, block signal number n. Otherwise unblock signal number n.
        wordNum = 0;
        bitNum = 0;
        for (int i=0; i<NumSignals; i++) {
            uint8_t block = (set->bits[wordNum] >> bitNum) & 1;
            if (block)
                signals[i].status |= SIGNAL_BLOCKED;
            else
                signals[i].status &= ~SIGNAL_BLOCKED;

            bitNum = (bitNum + 1)%sizeof(unsigned long);
            if (bitNum == 0)
                wordNum++;
        }

        return 0;
    }

    return -EINVAL;
}

void TzTask::sendSigChldToParent(bool exited, int exitCode,int origSigNum) {
    if (parent == nullptr)
        return;

    Signal *parentSigChld = &parent->signals[SIGCHLD];

    // If parent process has asked for it, send a SIGCHLD signal to parent.
    bool sendSignal = true;
    if (parentSigChld->handler == nullptr)
        sendSignal = false;
    else if (parentSigChld->flags & SA_NOCLDSTOP)
        sendSignal = false;


    if (!sendSignal)
        return;

    siginfo_t sigInfo;
    memset(&sigInfo, 0, sizeof(siginfo_t));
    sigInfo.si_pid = tid;
    sigInfo.si_uid = tid;

    if (exited)
        sigInfo.si_code = CLD_EXITED;
    else if ((origSigNum == SIGSTOP) || (origSigNum == SIGTSTP) || (origSigNum == SIGTTIN) || (origSigNum == SIGTTOU))
        sigInfo.si_code = CLD_STOPPED;
    else if (origSigNum == SIGCONT)
        sigInfo.si_code = CLD_CONTINUED;
    else if (origSigNum == SIGTRAP)
        sigInfo.si_code = CLD_TRAPPED;
    else
        sigInfo.si_code = CLD_KILLED;

    sigInfo.si_status = (exited) ? exitCode : origSigNum;

    parent->sigQueue(SIGCHLD, &sigInfo);
}

int TzTask::sigQueue(int sigNum, siginfo_t *sigInfo) {

    if (sigInfo != nullptr) {
        memcpy(&signals[sigNum].sigInfo, sigInfo, sizeof(siginfo_t));
        signals[sigNum].haveSigInfo = true;
    }
    else {
        signals[sigNum].haveSigInfo = false;
    }

    signals[sigNum].status |= SIGNAL_PENDING;

    signalled = true;

    awaken();

    return 0;
}

bool TzTask::signalDispatch() {
    int sigNum = 0;
    for (sigNum=0; sigNum<NumSignals; sigNum++) {
        if ((signals[sigNum].status & SIGNAL_PENDING) && (!(signals[sigNum].status & SIGNAL_BLOCKED)))
            break;
    }
    if (sigNum == NumSignals)
        return false;

    signalled = false;

    // Handle special signals first
    switch (sigNum) {
    case SIGKILL: {
        // Exit this process. This signal cannot be caught.
        terminate(0, SIGKILL);
        return true;
    }

    case SIGSTOP: {
        // Remove current task from scheduler consideration.This signal cannot be caught.
        state =  TzTask::State::Wait;

        //printf("signal %d\n", (int) this->id() & 0xFFFFFFFF);
        Scheduler::removeTask(this);

        sendSigChldToParent(false, 0, SIGSTOP);
        return true;
    }

    case SIGTSTP:
    case SIGTTIN:
    case SIGTTOU: {
        // If no handler, remove current task from scheduler consideration.
        if (signals[sigNum].handler == nullptr) {
            state =  TzTask::State::Wait;

            //printf("signal %d\n", (int) this->id() & 0xFFFFFFFF);
            Scheduler::removeTask(this);

            sendSigChldToParent(false, 0, sigNum);
            return true;
        }

        break;
    }

    case SIGCONT: {
        if (signals[sigNum].handler == nullptr) {
            awaken();
            sendSigChldToParent(false, 0, sigNum);
            return true;
        }

        break;
    }

    case SIGCHLD: {
        if (signals[sigNum].handler != nullptr) {
            if (signals[sigNum].flags & SA_NOCLDWAIT) {
                int status;
                wait4(signals[sigNum].sigInfo.si_pid, &status, WNOHANG);
            }
        }
        break;
    }
    case SIGQUIT: {
        /* Core Dump data here*/
        doCoreDump();
        // Exit this process. This signal cannot be caught.
        terminate(0, SIGQUIT);
        return true;
        break;
    }

    default:
        break;

    }

    // If we do not have a handler for this signal, perform
    // the default action.
    bool haveHandler = (signals[sigNum].handler != nullptr);
    if (!haveHandler) {
        if ((signals[sigNum].defaultAction == Terminate) || (signals[sigNum].defaultAction == CoreDump)) {
            terminate(0, sigNum);
            return true;
        }

        if (signals[sigNum].defaultAction == Ignore)
            return false;
    }

    // Block signals specified in this signal's mask
    int wordNum = 0;
    int bitNum = 0;
    for (int j=0; j<NumSignals; j++) {
        uint8_t block = (signals[sigNum].maskOnActive.bits[wordNum] >> bitNum) & 1;

        if ((j == sigNum) && (!(signals[j].flags & SA_NODEFER)))
            block = 1;

        if (block)
            signals[j].status |= SIGNAL_BLOCKED;

        bitNum = (bitNum + 1)%sizeof(unsigned long);
        if (bitNum == 0)
            wordNum++;
    }

    // Set the return address to the restorer function and set LR
    // to jump to signal handler.
    unsigned long *base = savedRegBase - NUM_SAVED_CPU_REGS;

    sigParamsStackTop++;
    if (sigParamsStackTop == sigParamsStackMax) {
        err_msg("Too many nested signals. Ran out of room for return stack\n");
        terminate(0, SIGKILL);

        return true;
    }

    SigReturn *sr = &sigParamsStack[sigParamsStackTop];
    sr->r0 = base[SAVED_REG_R0];
    sr->r1 = base[SAVED_REG_R1];
    sr->r2 = base[SAVED_REG_R2];
    sr->r3 = base[SAVED_REG_R3];
    sr->r12 = base[SAVED_REG_R12];
    sr->lr = base[SAVED_REG_LR];
    sr->sp_usr = base[SAVED_REG_SP_USR];
    sr->lr_usr = base[SAVED_REG_LR_USR];
    sr->sigNum = sigNum;
    memcpy(&sr->sigInfo, &signals[sigNum].sigInfo, sizeof(siginfo_t));

    // Populate the ucontext
    ucontext_t *uctx = (ucontext_t *)ucontext;
    memset(uctx, 0, sizeof(ucontext_t));

    register int dfar;
    asm volatile("MRC p15, 0, %[rt], c6, c0, 0":[rt] "=r" (dfar)::);

    uctx->uc_stack.ss_sp = (void *)base[SAVED_REG_SP_USR];
    uctx->uc_stack.ss_size = USER_SPACE_STACK_SIZE;
    uctx->uc_mcontext.arm_r0 = base[REG_R0];
    uctx->uc_mcontext.arm_r1 = base[REG_R1];
    uctx->uc_mcontext.arm_r2 = base[REG_R2];
    uctx->uc_mcontext.arm_r3 = base[REG_R3];
    uctx->uc_mcontext.arm_r4 = base[REG_R5];
    uctx->uc_mcontext.arm_r5 = base[REG_R6];
    uctx->uc_mcontext.arm_r6 = base[REG_R7];
    uctx->uc_mcontext.arm_r7 = base[REG_R8];
    uctx->uc_mcontext.arm_r8 = base[REG_R9];
    uctx->uc_mcontext.arm_r9 = base[REG_R10];
    uctx->uc_mcontext.arm_r10 = base[REG_R11];
    uctx->uc_mcontext.arm_fp = base[REG_R12];
    uctx->uc_mcontext.arm_ip = base[REG_R12];
    uctx->uc_mcontext.arm_sp = base[SAVED_REG_SP_USR];
    uctx->uc_mcontext.arm_lr = base[SAVED_REG_LR_USR];
    uctx->uc_mcontext.arm_pc = base[REG_LR];
    uctx->uc_mcontext.arm_cpsr = base[REG_CPSR];
    uctx->uc_mcontext.fault_address = dfar;
    uctx->uc_sigmask = signals[sigNum].maskOnActive;


    base[SAVED_REG_R0] = sigNum;

    if ((signals[sigNum].flags & SA_SIGINFO) && (signals[sigNum].haveSigInfo)) {
        base[SAVED_REG_R1] = (unsigned long)&(sr->sigInfo);
        base[SAVED_REG_R2] = (unsigned long)&(sr->sigInfo.si_value);
    }
    else {
        base[SAVED_REG_R1] = 0;
        base[SAVED_REG_R2] = 0;
    }

    base[SAVED_REG_R3] = (unsigned long)ucontext;

    //TODO: set SP_USR to alt stack if necessary
    // if (signals[i].flag & SA_ONALTSTACK)
    //      base[SAVED_REG_SP_USR] = altStack;

    base[SAVED_REG_LR_USR] = (unsigned long)signals[sigNum].restorer;
    base[SAVED_REG_LR] = (unsigned long)signals[sigNum].handler;

    // printf("----> Deliver signal %d lr_usr 0x%lx lr 0x%lx\n", sigNum, base[SAVED_REG_LR_USR], base[SAVED_REG_LR]);

    signals[sigNum].status &= ~SIGNAL_PENDING;
    signals[sigNum].status |= SIGNAL_ACTIVE;
    signals[sigNum].haveSigInfo = false;

    // Check if we should continue to receive this signal
    if (signals[sigNum].flags & SA_RESETHAND) {
        signals[sigNum].handler = nullptr;
    }

    return false;

}

int TzTask::sigAction(int sigNum, k_sigaction *action, k_sigaction *old) {

    Signal *signal = &signals[sigNum];
    old->handler = signal->handler;
    old->mask[0] = signal->maskOnActive.bits[0];
    old->mask[1] = signal->maskOnActive.bits[1];
    old->flags = signal->flags;
    old->restorer = signal->restorer;

    if (action == nullptr) {
        signal->handler = nullptr;
        signal->status = 0;
    }
    else if (action->flags & SA_RESTART){
        return -ENOTSUP;
    }
    else {
        signal->handler = action->handler;
        signal->maskOnActive.bits[0] = action->mask[0];
        signal->maskOnActive.bits[1] = action->mask[1];
        signal->flags = action->flags;
        signal->restorer = action->restorer;
    }

    return 0;
}

void TzTask::sigReturn() {
    if (sigParamsStackTop < 0) {
        err_msg("Inconsistent state of signal params stack\n");
        System::halt();
    }

    SigReturn *sr = &sigParamsStack[sigParamsStackTop];
    sigParamsStackTop--;

    // printf("----> Returned from signal %d\n", sr->sigNum);

    unsigned long *base = savedRegBase - NUM_SAVED_CPU_REGS;
    base[SAVED_REG_R0] = sr->r0;
    base[SAVED_REG_R1] = sr->r1;
    base[SAVED_REG_R2] = sr->r2;
    base[SAVED_REG_R3] = sr->r3;
    base[SAVED_REG_R12] = sr->r12;
    base[SAVED_REG_SP_USR] = sr->sp_usr;
    base[SAVED_REG_LR_USR] = sr->lr_usr;
    base[SAVED_REG_LR] = sr->lr;

    // Un-block signals specified in this signal's mask
    int wordNum = 0;
    int bitNum = 0;
    for (int j=0; j<NumSignals; j++) {
        uint8_t block = (signals[sr->sigNum].maskOnActive.bits[wordNum] >> bitNum) & 1;
        if (j == sr->sigNum) {
            block = 1;
            signals[j].status &= ~SIGNAL_ACTIVE;
        }

        if (block)
            signals[j].status &= ~SIGNAL_BLOCKED;

        bitNum = (bitNum + 1)%sizeof(unsigned long);
        if (bitNum == 0)
            wordNum++;
    }

}

void TzTask::createUContext() {
    PageTable *kernPageTable = PageTable::kernelPageTable();

    ucontext = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (ucontext == nullptr) {
        err_msg("[ucontext] kernel virtual address space exhausted !\n");
        pageTable->dump();
        return;
    }
    TzMem::PhysAddr pa = TzMem::allocPage(KERNEL_PID);
    if (pa == nullptr) {
        err_msg("system memory exhausted !\n");
        return;
    }

    kernPageTable->mapPage(ucontext, pa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true, false);
    pageTable->mapPage(ucontext, pa, MAIR_MEMORY, MEMORY_ACCESS_RO_USER, true, true);
}

void TzTask::destroyUContext() {
    if (ucontext == nullptr)
        return;

    TzMem::PhysAddr pa = pageTable->lookUp(ucontext);
    if (pa == nullptr)
        return;

    kernPageTable->unmapPage(ucontext);
    pageTable->unmapPage(ucontext);

    TzMem::freePage(pa);
}
