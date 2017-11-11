/******************************************************************************
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
 *****************************************************************************/

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
#include "tzpipe.h"

#include "lib_printf.h"

extern void *_system_link_base;

int TzTask::execve(IFile *exeFile, IDirectory *exeDir, char **argv, char **envp) {
    if (!exeFile->isExecutable())
        return -ENOEXEC;

    // Destroy the current elf image
    if ((!vmCloned) && (image != nullptr)) {
        delete image;
        image = NULL;
    }

    /*
     * Load the new image.
     */
    image = ElfImage::loadElf(id(), exeFile, exeDir);
    if (image == NULL) {
        err_msg("Could not load elf image\n");
        return -ENOEXEC;
    }
    vmCloned = false;

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
    /*
     * Populate the user mode stack
     */
    unsigned long imageStackTop = (unsigned long)image->stackBase();
    TzMem::VirtAddr userStackVa = image->stackTopPageVA();

    TaskStartInfo tslinkInfo(image, taskName, numArgs, argv, numEnvs, envp);
    if (!tslinkInfo.constructed())
            return -ENOMEM;
    int numBytesWritten = tslinkInfo.prepareUserStack(userStackVa);
    unsigned long userStackTop = imageStackTop - numBytesWritten;
    image->unmapStackFromKernel();
    /*
     * Point user space to the new image. Reset the stack
     */
    savedRegs[SAVED_REG_LR] = (unsigned long)image->entry(); // Linker entry point is already adjusted
    savedRegs[SAVED_REG_SP] = (unsigned long)stackKernel;
    savedRegs[SAVED_REG_SP_USR] = userStackTop;
    savedRegs[SAVED_REG_LR_USR] = (unsigned long)image->entry();

    register unsigned long spsr = 0x80 | Mode_USR; // No IRQs. IRQs belong in normal world.
    if (savedRegs[SAVED_REG_LR] & 0x1) {
        // The executable is in thumb mode;
        spsr |= 0x20;
    }
    savedRegs[SAVED_REG_SPSR] = spsr;

    pageTable = &image->userPageTable;

    // Close file descriptors marked for closeOnExec
    for (int i=0; i<MAX_FD; i++) {
        if ((files[i].data != nullptr) && (files[i].closeOnExec)) {
            if (files[i].type == File)
                RamFS::File::close((RamFS::File *)files[i].file);
            else if (files[i].type == Directory)
                RamFS::Directory::close((RamFS::Directory *)files[i].dir);
            else if (files[i].type == MQueue)
                MsgQueue::close((MsgQueue *)files[i].queue);
            else if (files[i].type == IpcPipe)
                Pipe::close((Pipe *)files[i].file, files[i].read);

            files[i].data = nullptr;
            files[i].offset = 0;
        }
    }

    // Reset the memory allocation trackers
    brkStart = image->dataSegmentBrk();
    brkCurr = brkStart;
    brkMax = (uint8_t *)brkStart + BRK_MAX;
    mmapMaxVa = &_system_link_base;

    // Change the working directory
    currWorkDir = exeDir;
    return 0;
}
