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

#include <hwtimer.h>
#include "arm/spinlock.h"

#include "tztask.h"
#include "tzmemory.h"
#include "objalloc.h"
#include "kernel.h"
#include "scheduler.h"

#include "lib_printf.h"

bool TzTask::TaskStartInfo::fits(char *curr, char *stop, const char *paramStr) {
    return ((curr + strlen(paramStr) + 1) < stop);
}

TzTask::TaskStartInfo::TaskStartInfo(ElfImage *image, const char *exeName, int nargs, char **args, int nenvs, char **envps) {

    state = false;
    this->image = image;

    char *paramsStart = (char *)(image->paramsPageVA());
    char *paramsStop = paramsStart + ElfImage::ParamsBlockSize;

    char *curr = paramsStart;

    //if image is dynamically linked then use loader to start
    if (image->LinkFlag == true) {
        exeName = image->ldNamePtr();
    }

    // Copy program name into the param page
    if (!fits(curr, paramsStop, exeName)) {
        err_msg("could not copy exeName. curr %p paramsStop %p\n", curr, paramsStop);
        return;
    }

    strcpy(curr, exeName);
    progName = curr;
    curr += strlen(exeName) + 1;

    // Copy each argument into the param page
    unsigned int argCount = (nargs < MAX_NUM_ARGS) ? nargs : MAX_NUM_ARGS;
    for (int i=0; i<argCount; i++) {
        if (!fits(curr, paramsStop, args[i])){
            err_msg("could not copy arg %d. curr %p paramsStop %p\n", i, curr, paramsStop);
            return;
        }

        strcpy(curr, args[i]);
        argv[i] = curr;
        curr += strlen(args[i]) + 1;
    }
    numArgs = argCount;

    if(!strcmp(progName,MUSL_LINKER_NAME)){
        numArgs++;
        argv[numArgs] = 0;
        for (int i = (numArgs - 1); i > 0; i--)
            argv[i] = argv[i-1];
    }

    // Copy each environment variable into the param page
    unsigned int envCount = (nenvs < MAX_NUM_ENVS) ? nenvs : MAX_NUM_ENVS;
    for (int i=0; i<envCount; i++) {
        if (!fits(curr, paramsStop, envps[i])){
            err_msg("could not copy envp %d. curr %p paramsStop %p\n", i, curr, paramsStop);
            return;
        }

        strcpy(curr, envps[i]);
        envs[i] = curr;
        curr += strlen(envps[i]) + 1;
    }
    numEnvs = envCount;

    // Copy the program headers.
    unsigned int phdrCount = image->numProgramHeaders();
    if (phdrCount > MAX_NUM_PROG_HEADERS)
        phdrCount = MAX_NUM_PROG_HEADERS;
    if ((curr + (phdrCount * sizeof(Elf_Phdr))) >= paramsStop){
        err_msg("could not copy program headers %d. curr %p run to %p paramsStop %p\n", phdrCount, curr, curr + (phdrCount * sizeof(Elf_Phdr)), paramsStop);
        return;
    }

    if(strcmp(progName,MUSL_LINKER_NAME)){
    for (int i=0; i<phdrCount; i++) {
        image->programHeader(i, (Elf_Phdr *)curr);
        phdrs[i] = (Elf_Phdr *)curr;
        curr += sizeof(Elf_Phdr);
    }
    }
    numProgHeaders = phdrCount;

    // hardware capability bit field value
    if ((curr + (sizeof(unsigned int))) >= paramsStop){
        err_msg("could not copy hwCaps %d. curr %p run to %p paramsStop %p\n", hwCaps, curr, curr + (sizeof(unsigned int)), paramsStop);
        return;
    }

    *((unsigned int *) curr) = hwCaps;

    state = true;
}

void *TzTask::TaskStartInfo::toUser(void *va) {
    char *kBase = (char *)image->paramsPageVA();
    char *uBase = (char *)image->paramsPageUser();

    int offset = (char *)va - kBase;
    char *rv = uBase + offset;

    return rv;
}

int TzTask::TaskStartInfo::prepareUserStack(TzMem::VirtAddr userStack) {

    if (!state)
        return 0;

    unsigned char *stack = (unsigned char *)userStack;

    /*
     * Stack begins with ELF Auxiliary vector table
     */
    struct AuxEntry {
        size_t type;
        size_t val;
    };
    stack -= sizeof(struct AuxEntry);
    AuxEntry *auxTop = (AuxEntry *)stack;


    // Start with AT_NULL
    auxTop->type = AT_NULL;
    auxTop->val = 0;
    auxTop--;

    // Next put the program header size AT_PHENT entry.
    auxTop->type = AT_PHENT;
    auxTop->val = sizeof(Elf_Phdr);
    auxTop--;

    // Next put the number of program headers
    auxTop->type = AT_PHNUM;
    auxTop->val = numProgHeaders;
    auxTop--;

    // The program headers follow
    if(!strcmp(progName, MUSL_LINKER_NAME)){
        auxTop->type = AT_PHDR;
        auxTop->val = (size_t)(LINKER_LOAD_ADDR+image->phOffset());
    }else{
    auxTop->type = AT_PHDR;
    auxTop->val = (size_t )toUser(phdrs[0]);
    }
    auxTop--;

    // The page size
    auxTop->type = AT_PAGESZ;
    auxTop->val = PAGE_SIZE_4K_BYTES;
    auxTop--;

    // Next set the UID, EUID, GID and EGID
    auxTop->type = AT_UID;
    auxTop->val = 0;
    auxTop--;
    auxTop->type = AT_EUID;
    auxTop->val = 0;
    auxTop--;
    auxTop->type = AT_GID;
    auxTop->val = 0;
    auxTop--;
    auxTop->type = AT_EGID;
    auxTop->val = 0;
    auxTop--;
    auxTop->type = AT_SECURE;
    auxTop->val = 1;
    auxTop--;

    // Features string entry comes next
    auxTop->type = AT_HWCAP;
    auxTop->val = hwCaps;

    /*
     * Next in the stack come in the envps
     */
    // A NULL pointer demarcates the envps
    char **envpTop = (char **)((unsigned long *)auxTop - 1);
    *envpTop = nullptr;
    envpTop--;

    for (int i=numEnvs-1; i>=0; i--) {
        *envpTop = (char *)toUser(envs[i]);
        envpTop--;
    }

    /*
     * Next in the stack come in the argvs
     */
    // A NULL pointer demarcates the argvs
    char **argvTop = (char **)envpTop;
    *argvTop = nullptr;
    argvTop--;

    for (int i=numArgs-1; i>=0; i--) {
        *argvTop = (char *)toUser(argv[i]);
        argvTop--;
    }

    /*
     * Next goes the program name string pointer (argv[0])
     */
    if (numArgs == 0) {
        *argvTop = (char *)toUser(progName);
        argvTop--;
        numArgs = 1;
    }

    /*
     * Top off the stack with the argc count.
     * Note this is number of argvs plus 1.
     */
    unsigned int *top = (unsigned int *)argvTop;
    *top = numArgs;

    return (unsigned char *)userStack - (unsigned char *)top;
}

TzTask::TaskStartInfo::~TaskStartInfo() {

}
