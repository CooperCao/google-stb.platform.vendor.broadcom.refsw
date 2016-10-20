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
 * termination.cpp
 *
 *  Created on: Feb 8, 2015
 *      Author: gambhire
 */
#include <hwtimer.h>
#include "arm/arm.h"
#include "arm/spinlock.h"

#include "fs/ramfs.h"

#include "tztask.h"
#include "tzmemory.h"
#include "tzioc.h"
#include "objalloc.h"
#include "kernel.h"
#include "scheduler.h"
#include "console.h"
#include "wait.h"
#include "futex.h"

#include "lib_printf.h"
#include "svcutils.h"

TzTask::~TzTask() {
    if (state != Defunct) {
        terminate();
    }

    PageTable *kernPageTable = PageTable::kernelPageTable();

    // Destroy the kernel mode stack.
    if (stackKernel != nullptr) {
        TzMem::VirtAddr stackVa = (uint8_t *)stackKernel - PAGE_SIZE_4K_BYTES;
        TzMem::PhysAddr stackPa = kernPageTable->lookUp(stackVa);

        kernPageTable->unmapPage(stackVa);
        TzMem::freePage(stackPa);
    }

    if (quickPages != nullptr) {
        kernPageTable->releaseAddrRange(quickPages, PAGE_SIZE_4K_BYTES*2);
    }

    // Destroy the TLS region
    if (threadInfo != nullptr) {
        TzMem::VirtAddr tiVa = PAGE_START_4K(threadInfo);
        TzMem::PhysAddr tiPa = kernPageTable->lookUp(tiVa);

        kernPageTable->unmapPage(tiVa);
        TzMem::freePage(tiPa);
    }

    //printf("Task %d %p collected\n", tid, this);
}

void TzTask::terminate(int tcode, int tsignal) {
    terminationLock();
    //printf("terminate task %d \n",this->id());
    Scheduler::removeTask(this);
#ifdef TASK_LOG
    Scheduler::idleQueue.remove(this->id());
#endif
    TzIoc::cleanupTask(this);

    destroyUContext();

    if (wqState.wq != nullptr) {
        wqState.wq->timedOut(this);
        // Terminating, no need to schedule
        Scheduler::removeTask(this);
#ifdef TASK_LOG
        Scheduler::idleQueue.remove(this->id());
#endif
    }

    // Close any opened files.
    for (int i=4; i<MAX_FD; i++) {
        if (files[i].data == nullptr)
            continue;

        if (files[i].type == File)
            RamFS::File::close((RamFS::File *)files[i].file);
        else if (files[i].type == Directory)
            RamFS::Directory::close((RamFS::Directory *)files[i].dir);

        files[i].data = nullptr;
        files[i].offset = 0;

    }

    state = Defunct;
    termCode = tcode;
    termSignal = tsignal;

    // Reparent children if possible
    if (children.numElements() > 0) {
        if (parent != nullptr) {
            const int numChildren = children.numElements();
            for (int i=0; i<numChildren; i++ ) {
                parent->children.pushBack(children[i]);
                children[i]->parent = parent;
            }
        }
    }

    // Signal our parent
    if (parent != nullptr) {

        parent->childTermQueue.signal();

        if (tsignal != -1)
            sendSigChldToParent(false, 0, tsignal);
        else
            sendSigChldToParent(true, tcode, 0);
    }

    PageTable *kernelPageTable = PageTable::kernelPageTable();

    if (!signalsCloned) {
        // Free the signal return stack
        TzMem::PhysAddr pa = kernelPageTable->lookUp(sigParamsStack);
        kernelPageTable->unmapPage(sigParamsStack);
        TzMem::freePage(pa);

        // Free the signals array
        int numPages = (sizeof(Signal) * NumSignals)/PAGE_SIZE_4K_BYTES + 1;
        uint8_t *currVa = (uint8_t *)signals;
        for (int i=0; i<numPages; i++) {
            TzMem::PhysAddr pa = kernelPageTable->lookUp(currVa);
            kernelPageTable->unmapPage(currVa);
            if (pa != nullptr)
                TzMem::freePage(pa);

            //printf("unmapped signal array %p pa %p\n", currVa, pa);
            currVa += PAGE_SIZE_4K_BYTES;
        }
    }

    // Free the file descriptor array
    if (!filesCloned) {
        int numPages = (sizeof(FileTableEntry) * MAX_FD)/PAGE_SIZE_4K_BYTES + 1;
        uint8_t *currVa = (uint8_t *)files;
        for (int i=0; i<numPages; i++) {
            TzMem::PhysAddr pa = kernelPageTable->lookUp(currVa);
            kernelPageTable->unmapPage(currVa);
            if (pa != nullptr)
                TzMem::freePage(pa);

            //printf("unmapped file descriptor array %p pa %p\n", currVa, pa);
            currVa += PAGE_SIZE_4K_BYTES;
        }
    }

    if (clearChildTid != nullptr) {
        TzMem::PhysAddr pa = pageTable->lookUp(clearChildTid);
        TzMem::VirtAddr va = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
        if (va == nullptr) {
            err_msg("Ran out of user virtual address space\n");
            System::halt();
        }

        kernelPageTable->mapPage(va, pa);
        IDType *sct = (IDType *)((uint8_t *)va + ((uint8_t *)clearChildTid - (uint8_t *)PAGE_START_4K(clearChildTid)));
        *sct = 0;
        kernelPageTable->unmapPage(va);

        Futex *futex = FutexStore::lookUp((int *)pa);
        if (futex != nullptr) {
            futex->wake(1);
        }
    }

    // Destroy the associated elf image
    if ((!vmCloned) && (image != nullptr)) {
        delete image;
    }

    terminationUnlock();
    //printf("Task %d %p zombied\n", tid, this);
}

int TzTask::wait4(IDType pid, int *status, int options) {

    if (pid == -1) {
        return waitAnyChild(status, options);
    }

    //printf("%s: check for pid %d options %d\n", __PRETTY_FUNCTION__, pid, options);
    while (!signalled) {
        volatile int nc = children.numElements();

        bool foundChild = false;
        for (int i=0; i<nc; i++) {

            if (children[i]->id() == pid) {
                TzTask *zombie = children[i];
                foundChild = true;

                bool exited = zombie->exited();
                bool signaled = zombie->terminatedBySignal();

                if ((!exited) && (!signaled)) {
                    if ((options & WNOHANG))
                        return -EAGAIN;
                    else
                        break;
                }

                if (exited)
                    SETEXITCODE(*status, zombie->termCode);
                else
                    SETTERMSIGNAL(*status, zombie->termSignal);

                delete zombie;

                children[i] = children[nc-1];
                children.popBack(nullptr);

                return pid;

            }
        }

        if (!foundChild) {
            return -ECHILD;
        }

        // Wait for any child to exit.
        childTermQueue.wait(this);
    }

    return -EINTR;

}

int TzTask::waitAnyChild(int *status, int options) {

    while (!signalled) {
        // Has any child terminated
        volatile int nc = children.numElements();
        if (nc == 0) {
            return -ECHILD;
        }

        for (int i=0; i<nc; i++) {
            //printf("\tChild %d state %d pid %d\n", i, children[i]->state, children[i]->id());
            if (children[i]->state == Defunct) {
                TzTask *zombie = children[i];
                bool exited = zombie->exited();

                IDType id = zombie->id();

                if (exited)
                    SETEXITCODE(*status, zombie->termCode);
                else
                    SETTERMSIGNAL(*status, zombie->termSignal);

                delete zombie;

                children[i] = children[nc-1];
                children.popBack(nullptr);

                return id;
            }
        }

        // No child has terminated yet.
        if (options & WNOHANG)
            return -EAGAIN;
        // We do not support PTRACE and SIGCONT.

        // Wait for any child to exit.
        childTermQueue.wait(this);
    }

    return -EINTR;
}

typedef struct coredump_memregion {
    unsigned int startAddr,endAddr, offset,flags;
} coredump_memregion;

void TzTask::doCoreDump() {
    IFile *uappdFile;
    IDirectory *dir=nullptr;
    IDirectory *parentDir = nullptr;
    IDirectory *root = System::root();
    /* Resolving uappd to get root folder */
    int rc = root->resolvePath("uappd.elf", &dir, &uappdFile,&parentDir);
    if(rc!=0) {
        err_msg("Could Not resolve UAPPD Path\n");
        System::halt();
    }
    if(parentDir == nullptr) {
        err_msg("Could Not resolve UAPPD Parent Dir NULL\n");
        System::halt();
    }
    /* Create Core Data File */
    IFile *core = RamFS::File::create(parentDir->owner(), parentDir->group());

    /* Add Core Data File to filesystem */
    char fname[80];
    strcpy(fname, "Core.core");
    rc = parentDir->addFile(fname, core);
    if(rc!=0) {
        err_msg("Could Not Add file to diretory\n");
        System::halt();
    }

    /* Copy CPU Registers */
    unsigned long *regBase = savedRegBase - NUM_SAVED_CPU_REGS;
    uint64_t file_offset=0;
    unsigned long reg[20];
    memcpy(reg,regBase,NUM_SAVED_CPU_REGS*sizeof(unsigned long));

    /* Overwrite Kernel space pointers with respective User space values*/
    reg[SAVED_REG_SP]=reg[SAVED_REG_SP_USR];
    reg[SAVED_REG_PC]=reg[SAVED_REG_LR];
    reg[SAVED_REG_LR]=reg[SAVED_REG_LR_USR];

    core->write(reg,NUM_SAVED_CPU_REGS*sizeof(unsigned long),file_offset);
    file_offset += NUM_SAVED_CPU_REGS*sizeof(unsigned long);

    /* Copy Memory Sections */
    int numMemSections=0;
    coredump_memregion memregion[32];
    memset(memregion,0xffffffff,sizeof(coredump_memregion)*32);
    //pageTable->dump();
    pageTable->dumpInMem((void *)memregion);
    int i=0;
    while(memregion[i].startAddr != 0xFFFFFFFF) {
        if(memregion[i].flags != 0)
        {
            numMemSections++;
        }
        i++;
    }

    //printf("numMemSections=%d\n",numMemSections);
    core->write(&numMemSections,sizeof(int),file_offset);
    file_offset += sizeof(int);

    /* Writing Page Table Entries in file */
    //read_offset = file_offset;
    i=0;
    while(memregion[i].startAddr != 0xFFFFFFFF) {
        if(memregion[i].flags != 0)
        {
            core->write(&memregion[i],sizeof(coredump_memregion),file_offset);
            file_offset += sizeof(coredump_memregion);
        }
        i++;
    }

    /* Copy Actual Pages in File */
    i=0;
    while(memregion[i].startAddr != 0xFFFFFFFF ) {
        int numPages=0;
        TzMem::VirtAddr kernelVa=nullptr;
        if(memregion[i].flags != 0)
        {
            /* Map userspace pointers to Kernel and copy in the file */
            kernelVa = SysCalls::mapToKernel((void *)memregion[i].startAddr,(size_t)(memregion[i].endAddr - memregion[i].startAddr + 1),&numPages);
            core->write((void *)kernelVa,(memregion[i].endAddr - memregion[i].startAddr +1),file_offset);
            file_offset += (memregion[i].endAddr - memregion[i].startAddr + 1);
            SysCalls::unmap( kernelVa , numPages);
        }
        i++;
    }
}
