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
 * execve.cpp
 *
 *  Created on: Feb 18, 2015
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

#include "fs/ramfs.h"

#include "lib_printf.h"

extern void *_system_link_base;

int TzTask::execve(IFile *exeFile, IDirectory *exeDir, char **argv, char **envp) {
    if (!exeFile->isExecutable())
        return -ENOEXEC;
    /*
     * Load the new image.
     */
    ElfImage *execImage = new ElfImage(tid, exeFile);
    if (execImage == nullptr){
        err_msg("Could not load elf image\n");
        return -ENOEXEC;
    }
    if (execImage->status != ElfImage::Valid) {
        err_msg("Could not load elf image. status %d\n", execImage->status);
        return -ENOEXEC;
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

    unsigned long imageStackTop = (unsigned long)execImage->stackBase();
    TzMem::VirtAddr userStackVa = execImage->stackTopPageVA();

    TaskStartInfo tsInfo(execImage, taskName, numArgs, argv, numEnvs, envp);
    if (!tsInfo.constructed())
        return -ENOMEM;

    int numBytesWritten = tsInfo.prepareUserStack(userStackVa);

    unsigned long userStackTop = imageStackTop - numBytesWritten;

    execImage->unmapStackFromKernel();

    /*
     * Point user space to the new image. Reset the stack.
     */
    savedRegs[SAVED_REG_LR] = (unsigned long)execImage->entry();
    savedRegs[SAVED_REG_SP] = (unsigned long)stackKernel;
    savedRegs[SAVED_REG_SP_USR] = userStackTop;
    savedRegs[SAVED_REG_LR_USR] = (unsigned long)execImage->entry();

    register unsigned long spsr = 0x80 | Mode_USR; // No IRQs. IRQs belong in normal world.
    if (savedRegs[SAVED_REG_LR] & 0x1) {
        // The executable is in thumb mode;
        spsr |= 0x20;
    }
    savedRegs[SAVED_REG_SPSR] = spsr;

    /*
     * Delete the current elf image and replace it with the new image
     */
    if (image != nullptr)
        delete image;
    image = execImage;
    pageTable = &image->userPageTable;
    //pageTable->dump();

    /*
     * Prepare TLS region
     */
    TzMem::PhysAddr tiPA = TzMem::allocPage(tid);
    if (tiPA == nullptr) {
        err_msg("system memory exhausted !\n");
        return -ENOMEM;
    }
    threadInfo = pageTable->reserveAddrRange(userStackVa, PAGE_SIZE_4K_BYTES, PageTable::ScanDirection::ScanForward);
    if (threadInfo == nullptr) {
        err_msg("No virtual address space left in user page table\n");
        return -ENOMEM;
    }
    pageTable->mapPage(threadInfo, tiPA, MAIR_MEMORY, MEMORY_ACCESS_RW_USER);
    threadInfo =(uint8_t *)threadInfo + THREAD_INFO_OFFSET;

    // Close file descriptors marked for closeOnExec
    for (int i=0; i<MAX_FD; i++) {
        if ((files[i].data != nullptr) && (files[i].closeOnExec)) {
            if (files[i].type == File)
                RamFS::File::close((RamFS::File *)files[i].file);
            else if (files[i].type == Directory)
                RamFS::Directory::close((RamFS::Directory *)files[i].dir);

            files[i].data = nullptr;
            files[i].offset = 0;
        }
    }

    // Reset the memory allocation trackers
    brkStart = execImage->dataSegmentBrk();
    brkCurr = brkStart;
    brkMax = (uint8_t *)brkStart + BRK_MAX;
    mmapMaxVa = &_system_link_base;

    // Change the working directory
    currWorkDir = exeDir;

    return 0;
}
