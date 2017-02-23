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


#include <cstdint>
#include "tzerrno.h"

#include "system.h"
#include "svcutils.h"
#include "tztask.h"
#include "scheduler.h"

#include "fs/ramfs.h"

static const int MAX_PROG_NAME_SIZE=256;
static const int MAX_ARGV_ELEM_SIZE=80;
static const int MAX_ENVP_ELEM_SIZE=160;

//TODO: For better SMP performance, replicate these arrays once for each CPU
//      Then remove the execLock.
static char kProgName[MAX_PROG_NAME_SIZE];
static char *kuArgv[TzTask::TaskStartInfo::MAX_NUM_ARGS];
static char kArgs[TzTask::TaskStartInfo::MAX_NUM_ARGS][MAX_ARGV_ELEM_SIZE];
static char *kArgv[TzTask::TaskStartInfo::MAX_NUM_ARGS];
static char *kuEnvp[TzTask::TaskStartInfo::MAX_NUM_ENVS];
static char kEnvs[TzTask::TaskStartInfo::MAX_NUM_ENVS][MAX_ENVP_ELEM_SIZE];
static char *kEnvp[TzTask::TaskStartInfo::MAX_NUM_ENVS];
SpinLock SysCalls::execLock;

TzTask *TzTask::taskFromId(int tid) {
    int numTasks = tasks.numElements();
    for (int i=0; i<numTasks; i++) {
        if (tasks[i]->tid == tid && !tasks[i]->exited())
            return tasks[i];
    }

    return nullptr;
}

void SysCalls::doWait4(TzTask *currTask) {

    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);

    int pid = (int)arg0;
    if ((pid < -1) || (pid == 0)) {
        err_msg("Process groups not implemented\n");
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOSYS);
        return;
    }

    int status = 0;
    int options = (int)arg2;
    int rv = currTask->wait4(pid, &status, options);

    int *userStatus = (int *)arg1;
    if (userStatus != nullptr)
        copyToUser(userStatus, &status);

    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doFork(TzTask *currTask) {
    TzTask *childTask = new TzTask(*currTask);
    if (childTask == nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
        return;
    }
    if (childTask->status() != TzTask::State::Ready) {
        delete childTask;
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
        return;
    }

    //printf("created child task %p\n", childTask);
    Scheduler::addTask(childTask);

    currTask->writeUserReg(TzTask::UserRegs::r0, childTask->id());
}

void SysCalls::doExecve(TzTask *currTask) {

    SpinLocker locker(&execLock);

    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);

    /*
     * Get and validate the program name.
     */
    const char *uProgName = (const char *)arg0;
    bool truncated;
    bool rc = strFromUser(uProgName, kProgName, MAX_PROG_NAME_SIZE, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    /*
     * Get the argvs.
     */
    const char **uArgv = (const char **)arg1;
    rc = ptrArrayFromUser(uArgv, kuArgv, TzTask::TaskStartInfo::MAX_NUM_ARGS);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    for (int i=0; i<TzTask::TaskStartInfo::MAX_NUM_ARGS; i++) {
        if (kuArgv[i] == 0)
            break;
    }
    for (int i=0; i<TzTask::TaskStartInfo::MAX_NUM_ARGS; i++) {
        if (kuArgv[i] == nullptr) {
            kArgs[i][0] = 0;
            break;
        }

        rc = strFromUser(kuArgv[i], kArgs[i], MAX_ARGV_ELEM_SIZE, &truncated);
        if (!rc) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
            return;
        }

        kArgs[i][MAX_ARGV_ELEM_SIZE-1] = 0;

        kArgv[i] = &kArgs[i][0];
        //printf("argv[%d] %p: %s\n", i,  kArgv+i, kArgv[i]);
    }
    /*
     * Get the envps.
     */
    if(arg2) {
        const char **uEnvp = (const char **)arg2;
        rc = ptrArrayFromUser(uEnvp, kuEnvp, TzTask::TaskStartInfo::MAX_NUM_ENVS);
        if (!rc) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
            return;
        }
        for (int i=0; i<TzTask::TaskStartInfo::MAX_NUM_ENVS; i++) {
            if (kuEnvp[i] == nullptr){
                kEnvs[i][0] = 0;
                break;
            }

            rc = strFromUser(kuEnvp[i], kEnvs[i], MAX_ENVP_ELEM_SIZE, &truncated);
            if (!rc) {
                currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
                return;
            }

            if (kEnvs[i][0] == 0)
                break;

            kEnvs[i][MAX_ENVP_ELEM_SIZE-1] = 0;
            kEnvp[i] = &kEnvs[i][0];
        }
    }
    /*
     * Resolve the file name
     */
    IFile *exeFile;
    IDirectory *dir, *parentDir;
    IDirectory *root = System::root();
    int rv = root->resolvePath(kProgName, &dir, &exeFile, &parentDir);
    if (rv != 0) {
        printf("could not resolve prog name %s\n", kProgName);
        currTask->writeUserReg(TzTask::UserRegs::r0, -rv);
        return;
    }
    if (dir != nullptr) { // Implies files==nullptr and this is a directory
        printf("cannot execute a directory\n");
        currTask->writeUserReg(TzTask::UserRegs::r0, -EISDIR);
        return;
    }

    rv = currTask->execve(exeFile, parentDir, kArgv, kEnvp);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doGetPid(TzTask *currTask) {
    currTask->writeUserReg(TzTask::UserRegs::r0, currTask->id());
}

void SysCalls::doSetUid(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    int rv = currTask->changeOwner((uint16_t)arg0);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doSetGid(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    int rv = currTask->changeGroup((uint16_t)arg0);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}


void SysCalls::doGetUid(TzTask *currTask) {
    uint16_t rv = currTask->owner();
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doGetGid(TzTask *currTask) {
    uint16_t rv = currTask->group();
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doPtrace(TzTask *currTask) {
    currTask->writeUserReg(TzTask::UserRegs::r0, -ENOTSUP);
}

void SysCalls::doPause(TzTask *currTask) {
    int rv = currTask->pause();
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doNice(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    int rv = currTask->nice((int)arg0);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}


static uint8_t taskDestructionStack[1024 * MAX_NUM_CPUS];
void SysCalls::doExit(TzTask *currTask) {

    int tc = currTask->userReg(TzTask::UserRegs::r0);
    currTask->terminate(tc);

    if (currTask->isCloneThread()) {
        // This task cannot be collected by a wait4 and needs
        // to be destroyed here. However destruction will cause
        // our current stack to also get released destroyed.
        // Hence switch stacks to an exception stack so that we
        // can release our own stack.
        int cpuid = arm::smpCpuNum();
        register uint8_t *excStack = &taskDestructionStack[cpuid * 1024] + 1024;
        // printf("set stack to %p %p\n", excStack, taskDestructionStack);

        asm volatile("mov sp, %[stack]"::[stack] "r" (excStack):);

        delete currTask;

        schedule();
    }

}

void SysCalls::doBrk(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    void *rv = currTask->brk((void *)arg0);
    currTask->writeUserReg(TzTask::UserRegs::r0, (unsigned long)rv);
}
