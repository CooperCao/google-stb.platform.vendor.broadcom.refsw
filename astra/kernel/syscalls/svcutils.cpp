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
 * svcutils.cpp
 *
 *  Created on: Jan 26, 2015
 *      Author: gambhire
 */

#include <cstdint>

#include "tzerrno.h"
#include "svcutils.h"
#include "tztask.h"
#include "scheduler.h"
#include "ldt.h"
#include "poll.h"
#include "crypto/csprng.h"
#include "seccomp.h"
#include "arm/gic.h"

SysCalls::SvcPerformer SysCalls::dispatchTable[NUM_SYS_CALLS];
SysCalls::SvcPerformer SysCalls::dispatchTableExt[NUM_EXT_SYS_CALLS];
TzMem::PhysAddr SysCalls::paramsPagePhys;
TzMem::VirtAddr SysCalls::paramsPage;

void SysCalls::init() {
    for (int i=0; i<NUM_SYS_CALLS; i++)
        dispatchTable[i] = nullptr;

    dispatchTable[SYS_restart_syscall] = notImpl;
    dispatchTable[SYS_exit] = doExit;
    dispatchTable[SYS_fork] = doFork;
    dispatchTable[SYS_read] = doRead;
    dispatchTable[SYS_write] = doWrite;
    dispatchTable[SYS_open] = doOpen;
    dispatchTable[SYS_close] = doClose;
    dispatchTable[SYS_creat] = doCreat;
    dispatchTable[SYS_link] = doLink;
    dispatchTable[SYS_unlink] = doUnlink;
    dispatchTable[SYS_execve] = doExecve;
    dispatchTable[SYS_chdir] = doChdir;
    dispatchTable[SYS_mknod] = doMknod;
    dispatchTable[SYS_chmod] = doChmod;
    dispatchTable[SYS_lchown] = doChown;
    dispatchTable[SYS_lseek] = dolSeek;
    dispatchTable[SYS_getpid] = doGetPid;
    dispatchTable[SYS_mount] = doMount;
    dispatchTable[SYS_setuid] = doSetUid;
    dispatchTable[SYS_getuid] = doGetUid;
    dispatchTable[SYS_ptrace] = doPtrace;
    dispatchTable[SYS_pause] = doPause;
    dispatchTable[SYS_access] = doAccess;
    dispatchTable[SYS_nice] = doNice;
    dispatchTable[SYS_sync] = doSync;
    dispatchTable[SYS_kill] = doKill;
    dispatchTable[SYS_rename] = doRename;
    dispatchTable[SYS_mkdir] = doMkdir;
    dispatchTable[SYS_rmdir] = doRmdir;
    dispatchTable[SYS_dup] = doDup;
    dispatchTable[SYS_pipe] = notImpl;
    dispatchTable[SYS_times] = notImpl;
    dispatchTable[SYS_brk] = doBrk;
    dispatchTable[SYS_setgid] = doSetGid;
    dispatchTable[SYS_getgid] = doGetGid;
    dispatchTable[SYS_geteuid] = doGetUid;
    dispatchTable[SYS_getegid] = doGetGid;
    dispatchTable[SYS_acct] = notImpl;
    dispatchTable[SYS_umount2] = doUmount;
    dispatchTable[SYS_ioctl] = doIoctl;
    dispatchTable[SYS_fcntl] = notImpl;
    dispatchTable[SYS_setpgid] = notImpl;
    dispatchTable[SYS_umask] = notImpl;
    dispatchTable[SYS_chroot] = notImpl;
    dispatchTable[SYS_ustat] = notImpl;
    dispatchTable[SYS_dup2] = doDup2;
    dispatchTable[SYS_getppid] = notImpl;
    dispatchTable[SYS_getpgrp] = notImpl;
    dispatchTable[SYS_setsid] = notImpl;
    dispatchTable[SYS_sigaction] = notImpl;
    dispatchTable[SYS_setreuid] = doSetUid;
    dispatchTable[SYS_setregid] = doSetGid;
    dispatchTable[SYS_sigsuspend] = notImpl;
    dispatchTable[SYS_sigpending] = notImpl;
    dispatchTable[SYS_sethostname] = notImpl;
    dispatchTable[SYS_setrlimit] = notImpl;
    dispatchTable[SYS_getrusage] = notImpl;
    dispatchTable[SYS_gettimeofday] = notImpl;
    dispatchTable[SYS_settimeofday] = notImpl;
    dispatchTable[SYS_getgroups] = notImpl;
    dispatchTable[SYS_setgroups] = notImpl;
    dispatchTable[SYS_symlink] = notImpl;
    dispatchTable[SYS_readlink] = notImpl;
    dispatchTable[SYS_uselib] = notImpl;
    dispatchTable[SYS_swapon] = notImpl;
    dispatchTable[SYS_reboot] = notImpl;
    dispatchTable[SYS_munmap] = doUnmap;
    dispatchTable[SYS_truncate] = notImpl;
    dispatchTable[SYS_ftruncate] = notImpl;
    dispatchTable[SYS_fchmod] = doFchmod;
    dispatchTable[SYS_fchown] = doFchown;
    dispatchTable[SYS_getpriority] = notImpl;
    dispatchTable[SYS_setpriority] = notImpl;
    dispatchTable[SYS_statfs] = notImpl;
    dispatchTable[SYS_fstatfs] = notImpl;
    dispatchTable[SYS_syslog] = notImpl;
    dispatchTable[SYS_setitimer] = notImpl;
    dispatchTable[SYS_getitimer] = notImpl;
    dispatchTable[SYS_stat] = notImpl;
    dispatchTable[SYS_lstat] = notImpl;
    dispatchTable[SYS_fstat] = notImpl;
    dispatchTable[SYS_vhangup] = notImpl;
    dispatchTable[SYS_wait4] = doWait4;
    dispatchTable[SYS_swapoff] = notImpl;
    dispatchTable[SYS_sysinfo] = notImpl;
    dispatchTable[SYS_fsync] = doFSync;
    dispatchTable[SYS_sigreturn] = doSigReturn;
    dispatchTable[SYS_clone] = doClone;
    dispatchTable[SYS_setdomainname] = notImpl;
    dispatchTable[SYS_uname] = notImpl;
    dispatchTable[SYS_adjtimex] = notImpl;
    dispatchTable[SYS_mprotect] = doMProtect;
    dispatchTable[SYS_sigprocmask] = doSigProcMask;
    dispatchTable[SYS_init_module] = notImpl;
    dispatchTable[SYS_delete_module] = notImpl;
    dispatchTable[SYS_quotactl] = notImpl;
    dispatchTable[SYS_getpgid] = notImpl;
    dispatchTable[SYS_fchdir] = doFchdir;
    dispatchTable[SYS_bdflush] = notImpl;
    dispatchTable[SYS_sysfs] = notImpl;
    dispatchTable[SYS_personality] = notImpl;
    dispatchTable[SYS_setfsuid] = notImpl;
    dispatchTable[SYS_setfsgid] = notImpl;
    dispatchTable[SYS__llseek] = dollSeek;
    dispatchTable[SYS_getdents] = dogetdents;
    dispatchTable[SYS__newselect] = notImpl;
    dispatchTable[SYS_flock] = notImpl;
    dispatchTable[SYS_msync] = notImpl;
    dispatchTable[SYS_readv] = doReadv;
    dispatchTable[SYS_writev] = doWritev;
    dispatchTable[SYS_getsid] = notImpl;
    dispatchTable[SYS_fdatasync] = notImpl;
    dispatchTable[SYS__sysctl] = notImpl;
    dispatchTable[SYS_mlock] = notImpl;
    dispatchTable[SYS_munlock] = notImpl;
    dispatchTable[SYS_mlockall] = notImpl;
    dispatchTable[SYS_munlockall] = notImpl;
    dispatchTable[SYS_sched_setparam] = doSetSchedulerParam;
    dispatchTable[SYS_sched_getparam] = doGetSchedulerParam;
    dispatchTable[SYS_sched_getscheduler] = doGetScheduler;
    dispatchTable[SYS_sched_setscheduler] = doSetScheduler;
    dispatchTable[SYS_sched_yield] = notImpl;
    dispatchTable[SYS_sched_get_priority_max] = notImpl;
    dispatchTable[SYS_sched_get_priority_min] = notImpl;
    dispatchTable[SYS_sched_rr_get_interval] = notImpl;
    dispatchTable[SYS_nanosleep] = doNanoSleep;
    dispatchTable[SYS_mremap] = notImpl;
    dispatchTable[SYS_setresuid] = notImpl;
    dispatchTable[SYS_getresuid] = notImpl;
    dispatchTable[SYS_poll] = doPoll;
    dispatchTable[SYS_nfsservctl] = notImpl;
    dispatchTable[SYS_setresgid] = notImpl;
    dispatchTable[SYS_getresgid] = notImpl;
    dispatchTable[SYS_prctl] = notImpl;
    dispatchTable[SYS_rt_sigreturn] = doSigReturn;
    dispatchTable[SYS_rt_sigaction] = doSigAction;
    dispatchTable[SYS_rt_sigprocmask] = doSigProcMask;
    dispatchTable[SYS_rt_sigpending] = notImpl;
    dispatchTable[SYS_rt_sigtimedwait] = notImpl;
    dispatchTable[SYS_rt_sigqueueinfo] = doRtSigQueueInfo;
    dispatchTable[SYS_rt_sigsuspend] = notImpl;
    dispatchTable[SYS_pread64] = notImpl;
    dispatchTable[SYS_pwrite64] = notImpl;
    dispatchTable[SYS_chown] = doChown;
    dispatchTable[SYS_getcwd] = notImpl;
    dispatchTable[SYS_capget] = notImpl;
    dispatchTable[SYS_capset] = notImpl;
    dispatchTable[SYS_sigaltstack] = notImpl;
    dispatchTable[SYS_sendfile] = notImpl;
    dispatchTable[SYS_vfork] = doFork;
    dispatchTable[SYS_ugetrlimit] = notImpl;
    dispatchTable[SYS_mmap2] = doMmap;
    dispatchTable[SYS_truncate64] = notImpl;
    dispatchTable[SYS_ftruncate64] = notImpl;
    dispatchTable[SYS_stat64] = doStat64;
    dispatchTable[SYS_fstat64] = doFStat64;
    dispatchTable[SYS_lstat64] = notImpl;
    dispatchTable[SYS_lchown32] = doChown;
    dispatchTable[SYS_getuid32] = doGetUid;
    dispatchTable[SYS_getgid32] = doGetGid;
    dispatchTable[SYS_geteuid32] = doGetUid;
    dispatchTable[SYS_getegid32] = doGetGid;
    dispatchTable[SYS_setreuid32] = doSetUid;
    dispatchTable[SYS_setregid32] = doSetGid;
    dispatchTable[SYS_getgroups32] = notImpl;
    dispatchTable[SYS_setgroups32] = notImpl;
    dispatchTable[SYS_fchown32] = doFchown;
    dispatchTable[SYS_setresuid32] = notImpl;
    dispatchTable[SYS_getresuid32] = notImpl;
    dispatchTable[SYS_setresgid32] = notImpl;
    dispatchTable[SYS_getresgid32] = notImpl;
    dispatchTable[SYS_chown32] = doChown;
    dispatchTable[SYS_setuid32] = doSetUid;
    dispatchTable[SYS_setgid32] = doSetGid;
    dispatchTable[SYS_setfsuid32] = notImpl;
    dispatchTable[SYS_setfsgid32] = notImpl;
    dispatchTable[SYS_getdents64] = dogetdents;
    dispatchTable[SYS_pivot_root] = notImpl;
    dispatchTable[SYS_mincore] = notImpl;
    dispatchTable[SYS_madvise] = notImpl;
    dispatchTable[SYS_fcntl64] = notImpl;
    dispatchTable[SYS_gettid] = doGetTid;
    dispatchTable[SYS_readahead] = notImpl;
    dispatchTable[SYS_setxattr] = notImpl;
    dispatchTable[SYS_lsetxattr] = notImpl;
    dispatchTable[SYS_fsetxattr] = notImpl;
    dispatchTable[SYS_getxattr] = notImpl;
    dispatchTable[SYS_lgetxattr] = notImpl;
    dispatchTable[SYS_fgetxattr] = notImpl;
    dispatchTable[SYS_listxattr] = notImpl;
    dispatchTable[SYS_llistxattr] = notImpl;
    dispatchTable[SYS_flistxattr] = notImpl;
    dispatchTable[SYS_removexattr] = notImpl;
    dispatchTable[SYS_lremovexattr] = notImpl;
    dispatchTable[SYS_fremovexattr] = notImpl;
    dispatchTable[SYS_tkill] = notImpl;
    dispatchTable[SYS_sendfile64] = notImpl;
    dispatchTable[SYS_futex] = doFutex;
    dispatchTable[SYS_sched_setaffinity] = notImpl;
    dispatchTable[SYS_sched_getaffinity] = notImpl;
    dispatchTable[SYS_io_setup] = notImpl;
    dispatchTable[SYS_io_destroy] = notImpl;
    dispatchTable[SYS_io_getevents] = notImpl;
    dispatchTable[SYS_io_submit] = notImpl;
    dispatchTable[SYS_io_cancel] = notImpl;
    dispatchTable[SYS_exit_group] = notImpl;
    dispatchTable[SYS_lookup_dcookie] = notImpl;
    dispatchTable[SYS_epoll_create] = notImpl;
    dispatchTable[SYS_epoll_ctl] = notImpl;
    dispatchTable[SYS_epoll_wait] = notImpl;
    dispatchTable[SYS_remap_file_pages] = notImpl;
    dispatchTable[SYS_set_tid_address] = doSetTidAddress;
    dispatchTable[SYS_timer_create] = notImpl;
    dispatchTable[SYS_timer_settime] = notImpl;
    dispatchTable[SYS_timer_gettime] = notImpl;
    dispatchTable[SYS_timer_getoverrun] = notImpl;
    dispatchTable[SYS_timer_delete] = notImpl;
    dispatchTable[SYS_clock_settime] = doClockSetTime;
    dispatchTable[SYS_clock_gettime] = doClockGetTime;
    dispatchTable[SYS_clock_getres] = doClockGetRes;
    dispatchTable[SYS_clock_nanosleep] = doNanoSleep;
    dispatchTable[SYS_statfs64] = notImpl;
    dispatchTable[SYS_fstatfs64] = notImpl;
    dispatchTable[SYS_tgkill] = notImpl;
    dispatchTable[SYS_utimes] = notImpl;
    dispatchTable[SYS_fadvise64_64] = notImpl;
    dispatchTable[SYS_pciconfig_iobase] = notImpl;
    dispatchTable[SYS_pciconfig_read] = notImpl;
    dispatchTable[SYS_pciconfig_write] = notImpl;
    dispatchTable[SYS_mq_open] = doMqOpen;
    dispatchTable[SYS_mq_unlink] = doMqUnlink;
    dispatchTable[SYS_mq_timedsend] = doMqTimedSend;
    dispatchTable[SYS_mq_timedreceive] = doMqTimedRecv;
    dispatchTable[SYS_mq_notify] = doMqNotify;
    dispatchTable[SYS_mq_getsetattr] = doMqGetSetAttr;
    dispatchTable[SYS_waitid] = notImpl;
    dispatchTable[SYS_getrandom] = doGetRandom;
    dispatchTable[SYS_seccomp] = doSecComp;

    for (int i=0; i<NUM_EXT_SYS_CALLS; i++)
        dispatchTableExt[i] = nullptr;

    dispatchTableExt[EXT_get_curr_cpu - EXT_SYS_CALL_BASE] = doGetCurrCpu;
    dispatchTableExt[EXT_gen_sgi - EXT_SYS_CALL_BASE] = doGenSgi;
    dispatchTableExt[EXT_cache_inval - EXT_SYS_CALL_BASE] = doCacheInval;
    dispatchTableExt[EXT_cache_clean - EXT_SYS_CALL_BASE] = doCacheClean;
    dispatchTableExt[EXT_set_thread_area - EXT_SYS_CALL_BASE] = doSetThreadArea;
    dispatchTableExt[EXT_get_thread_area - EXT_SYS_CALL_BASE] = doGetThreadArea;

    paramsPagePhys = TzMem::allocPage(KERNEL_PID);
    if (paramsPagePhys == nullptr) {
        err_msg("%s: Could not get memory !\n", __FUNCTION__);
        paramsPage = nullptr;
        return;
    }

    PageTable *pt = PageTable::kernelPageTable();
    paramsPage = pt->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanDirection::ScanForward);
    if (paramsPage == nullptr) {
        err_msg("%s: address space exhausted\n", __FUNCTION__);
        TzMem::freePage(paramsPagePhys);
        return;
    }
    pt->mapPage(paramsPage, paramsPagePhys, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true);

    spinlock_init("SysCalls::exec", &execLock);
    spinlock_init("SysCalls::fops", &fopsLock);
}

void SysCalls::dispatch() {
    TzTask *currTask = TzTask::current();
    if (currTask == nullptr) {
        err_msg("%s: sys call made from an invalid task !\n", __PRETTY_FUNCTION__);
        return;
    }

    unsigned int sysCallNum = currTask->userReg(TzTask::UserRegs::r7);

    /* When the current task has been sandboxed as seccomp strict,
     * only read, write, sigreturn and exit system calls are allowed.
     * Any other system call will result in process termination by
     * sigkill.
     */
    if (currTask->isSecCompStrict()) {
        switch (sysCallNum) {
        case SYS_read:
        case SYS_write:
        case SYS_exit:
        case SYS_rt_sigreturn:
        case SYS_sigreturn:
            break;

        default:
            currTask->sigQueue(SIGKILL);
            currTask->writeUserReg(TzTask::UserRegs::r0, -EACCES);
            return;
        }
    }

    if (sysCallNum < NUM_SYS_CALLS) {
        if (dispatchTable[sysCallNum] != nullptr)
            dispatchTable[sysCallNum](currTask);
        else {
            // Return -ENOSYS for unimplemented system call.
            printf("Task %d %p unhandled system call %d\n", currTask->id(), currTask, sysCallNum);
            currTask->writeUserReg(TzTask::UserRegs::r0, -ENOSYS);
        }
    }
    else if (sysCallNum >= EXT_SYS_CALL_BASE &&
             sysCallNum < EXT_SYS_CALL_LAST)
    {
        unsigned int extCallNum = sysCallNum - EXT_SYS_CALL_BASE;

        if (dispatchTableExt[extCallNum] != nullptr)
            dispatchTableExt[extCallNum](currTask);
        else {
            // Return -ENOSYS for unimplemented system call.
            printf("Task %d %p unhandled system call %d\n", currTask->id(), currTask, sysCallNum);
            currTask->writeUserReg(TzTask::UserRegs::r0, -ENOSYS);
        }
    }
}

void SysCalls::doSigProcMask(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);

    int how = (int)arg0;
    sigset_t *userSet = (sigset_t *)arg1;
    sigset_t kernelSet, kernelOldSet;
    int rv = 0;
    if (userSet != nullptr) {
        bool rc = copyFromUser(userSet, &kernelSet);
        if (!rc) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
            return;
        }

        rv = currTask->sigprocmask(how, &kernelSet, &kernelOldSet);
    }
    else
        rv = currTask->sigprocmask(how, nullptr, &kernelOldSet);

    sigset_t *userOldSet = (sigset_t *)arg2;
    if (userOldSet != nullptr) {
        copyToUser(userOldSet, &kernelOldSet);
    }

    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

bool SysCalls::fromUser(const void *userPtr, void *kernelPtr, size_t size) {
    int numPages = 0;
    TzMem::VirtAddr va = mapToKernel(userPtr, size, &numPages);
    if (va == nullptr)
        return false;

    int pageOffset = (uint8_t *)userPtr - (uint8_t *)PAGE_START_4K(userPtr);
    uint8_t *srcPtr = (uint8_t *)va + pageOffset;

    memcpy(kernelPtr, srcPtr, size);

    unmap(va, numPages);

    return true;
}

bool SysCalls::strFromUser(const char *userPtr, char *kernelStr, const size_t maxLen, bool *truncated) {
    int pageCount, strLen;
    TzMem::VirtAddr va = mapStrToKernel(userPtr, maxLen, &pageCount, &strLen, truncated);
    if (va == nullptr)
        return false;

    int pageOffset = (uint8_t *)userPtr - (uint8_t *)PAGE_START_4K(userPtr);
    uint8_t *srcPtr = (uint8_t *)va + pageOffset;

    memcpy(kernelStr, srcPtr, strLen);

    unmap(va, pageCount);

    return true;

}

bool SysCalls::ptrArrayFromUser(const char **userArray, char **kernelArray, const size_t maxLen) {
    int pageCount, arrayLen;
    TzMem::VirtAddr va = mapPtrArrayToKernel(userArray, maxLen, &pageCount, &arrayLen);
    if (va == nullptr)
        return false;

    int pageOffset = (uint8_t *)userArray - (uint8_t *)PAGE_START_4K(userArray);
    uint8_t *srcPtr = (uint8_t *)va + pageOffset;

    memcpy(kernelArray, srcPtr, arrayLen*sizeof(const char *));

    unmap(va, pageCount);

    return true;

}

bool SysCalls::toUser(void *userPtr, const void *kernelPtr, size_t size) {
    int numPages = 0;
    TzMem::VirtAddr va = mapToKernel(userPtr, size, &numPages);
    if (va == nullptr)
        return false;

    int pageOffset = (uint8_t *)userPtr - (uint8_t *)PAGE_START_4K(userPtr);
    uint8_t *srcPtr = (uint8_t *)va + pageOffset;

    memcpy(srcPtr, kernelPtr, size);

    unmap(va, numPages);

    return true;

}

bool SysCalls::validateUserMemAccess(const void *userPtr, size_t size) {
    TzTask *currTask = TzTask::current();
    if (currTask == nullptr) {
        err_msg("%s: sys call made from an invalid task !\n", __PRETTY_FUNCTION__);
        return false;
    }
    if (size == 0)
        size = 1;

    const PageTable *userPageTable = currTask->userPageTable();

    uint8_t *userPageStart = (uint8_t *)PAGE_START_4K(userPtr);
    uint8_t *userLastPage = (uint8_t *)PAGE_START_4K((uint8_t *)userPtr + size - 1);
    uint8_t *curr = userPageStart;
    while (curr <= userLastPage) {
        TzMem::PhysAddr pa = userPageTable->lookUp(curr);
        if (pa == nullptr)
            return false;

        curr += PAGE_SIZE_4K_BYTES;
    }

    return true;
}

TzMem::PhysAddr SysCalls::userMemPhysAddr(const void *userPtr) {
    TzTask *currTask = TzTask::current();
    if (currTask == nullptr) {
        err_msg("%s: sys call made from an invalid task !\n", __PRETTY_FUNCTION__);
        return nullptr;
    }

    const PageTable *userPageTable = currTask->userPageTable();

    uint8_t *userPage = (uint8_t *)PAGE_START_4K(userPtr);
    TzMem::PhysAddr pa = userPageTable->lookUp(userPage);

    int offset = (uint8_t *)userPtr - userPage;
    return ((uint8_t *)pa + offset);
}

TzMem::VirtAddr SysCalls::mapStrToKernel(const char *userStr, size_t maxLen, int *pageCount, int *strLen, bool *truncated) {
    TzTask *currTask = TzTask::current();
    if (currTask == nullptr) {
        err_msg("%s: sys call made from an invalid task !\n", __PRETTY_FUNCTION__);
        return nullptr;
    }

    const PageTable *userPageTable = currTask->userPageTable();
    PageTable *kernPageTable = PageTable::kernelPageTable();

    uint8_t *userPageStart = (uint8_t *)PAGE_START_4K(userStr);
    uint8_t *userLastPage = (uint8_t *)PAGE_START_4K((uint8_t *)userStr + maxLen - 1);
    int numPages = (userLastPage - userPageStart)/PAGE_SIZE_4K_BYTES + 1;

    TzMem::VirtAddr va = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, numPages*PAGE_SIZE_4K_BYTES,PageTable::ScanForward);
    if (va == nullptr) {
        err_msg("%s: va exhausted !\n", __PRETTY_FUNCTION__);
        return nullptr;
    }
    int npages = 1;

    uint8_t *curr = userPageStart;
    uint8_t *currVa = (uint8_t *)va;
    uint8_t *nextVa = currVa + PAGE_SIZE_4K_BYTES;
    uint8_t *currStr = currVa + ((uint8_t *)userStr - userPageStart);
    int rv = 0;
    while (curr <= userLastPage) {
        TzMem::PhysAddr pa = userPageTable->lookUp(curr);
        if (pa == nullptr)
            break;

        kernPageTable->mapPage(currVa, pa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true);
        while ((currStr < nextVa) && (*currStr)) {
            rv++;
            currStr++;
        }

        if (currStr < nextVa)
            break;

        curr += PAGE_SIZE_4K_BYTES;
        currVa += PAGE_SIZE_4K_BYTES;
        nextVa += PAGE_SIZE_4K_BYTES;

        npages++;
    }

    *truncated = (*currStr == 0) ? false : true;
    *pageCount = npages;
    *strLen = rv+1;

    return va;
}

TzMem::VirtAddr SysCalls::mapPtrArrayToKernel(const char **userArray, size_t maxLen, int *pageCount, int *arrayLen) {

    typedef uint8_t* ptr_t;

    TzTask *currTask = TzTask::current();
    if (currTask == nullptr) {
        err_msg("%s: sys call made from an invalid task !\n", __PRETTY_FUNCTION__);
        return nullptr;
    }

    const PageTable *userPageTable = currTask->userPageTable();
    PageTable *kernPageTable = PageTable::kernelPageTable();

    uint8_t *userPageStart = (uint8_t *)PAGE_START_4K(userArray);
    uint8_t *userLastPage = (uint8_t *)PAGE_START_4K((uint8_t *)userArray + maxLen*sizeof(ptr_t) - 1);
    int numPages = (userLastPage - userPageStart)/PAGE_SIZE_4K_BYTES + 1;

    TzMem::VirtAddr va = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, numPages*PAGE_SIZE_4K_BYTES,PageTable::ScanForward);
    if (va == nullptr) {
        err_msg("%s: va exhausted !\n", __PRETTY_FUNCTION__);
        return nullptr;
    }

    int npages = 1;

    uint8_t *curr = userPageStart;
    uint8_t *currVa = (uint8_t *)va;
    uint8_t *nextVa = currVa + PAGE_SIZE_4K_BYTES;
    ptr_t *currPtr = (ptr_t *)(currVa + ((uint8_t *)userArray - userPageStart));
    int rv = 0;
    while (curr <= userLastPage) {
        TzMem::PhysAddr pa = userPageTable->lookUp(curr);
        if (pa == nullptr)
            break;

        kernPageTable->mapPage(currVa, pa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true);
        while (((uint8_t *)currPtr < nextVa) && (*currPtr)) {
            rv++;
            currPtr++;
        }

        if ((uint8_t *)currPtr < nextVa)
            break;

        curr += PAGE_SIZE_4K_BYTES;
        currVa += PAGE_SIZE_4K_BYTES;
        nextVa += PAGE_SIZE_4K_BYTES;

        npages++;
    }

    *pageCount = npages;
    *arrayLen = rv+1;
    return va;
}

TzMem::VirtAddr SysCalls::mapToKernelNormal(const void *userPtr, size_t size, int *pageCount) {
    TzTask *currTask = TzTask::current();
    if (currTask == nullptr) {
        err_msg("%s: sys call made from an invalid task !\n", __PRETTY_FUNCTION__);
        return nullptr;
    }
    if (size == 0)
        size = 1;

    const PageTable *userPageTable = currTask->userPageTable();
    PageTable *kernPageTable = PageTable::kernelPageTable();

    uint8_t *userPageStart = (uint8_t *)PAGE_START_4K(userPtr);
    uint8_t *userLastPage = (uint8_t *)PAGE_START_4K((uint8_t *)userPtr + size - 1);
    int numPages = (userLastPage - userPageStart)/PAGE_SIZE_4K_BYTES + 1;

    TzMem::VirtAddr va = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, numPages*PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (va == nullptr) {
        err_msg("%s: va exhausted !\n", __PRETTY_FUNCTION__);
        return nullptr;
    }

    uint8_t *curr = userPageStart;
    uint8_t *currVa = (uint8_t *)va;
    while (curr <= userLastPage) {
        PageTable::EntryAttribs attribs;
        TzMem::PhysAddr pa = userPageTable->lookUp(curr, &attribs);
        if (pa == nullptr) {
            // printf("NOT mapped in user page table\n");
            kernPageTable->unmapPageRange(va, currVa - 1);
            kernPageTable->releaseAddrRange(va, numPages*PAGE_SIZE_4K_BYTES);
            return nullptr;
        }

        kernPageTable->mapPage(currVa, pa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true, false, attribs.nonSecure);

        curr += PAGE_SIZE_4K_BYTES;
        currVa += PAGE_SIZE_4K_BYTES;
    }

    *pageCount = numPages;
    return va;
}

void SysCalls::unmapNormal(TzMem::VirtAddr vaFirstPage, int numPages) {
    uint8_t *vaLastPage = (uint8_t *)vaFirstPage + numPages*PAGE_SIZE_4K_BYTES - 1;
    PageTable *kernPageTable = PageTable::kernelPageTable();
    kernPageTable->unmapPageRange(vaFirstPage, vaLastPage);
}

TzMem::VirtAddr SysCalls::mapToKernel(const void *userPtr, size_t size, int *pageCount) {
    TzTask *currTask = TzTask::current();
    if (currTask == nullptr) {
        err_msg("%s: sys call made from an invalid task !\n", __PRETTY_FUNCTION__);
        return nullptr;
    }
    if (size == 0)
        size = 1;

    uint8_t *userPageStart = (uint8_t *)PAGE_START_4K(userPtr);
    uint8_t *userLastPage = (uint8_t *)PAGE_START_4K((uint8_t *)userPtr + size - 1);
    int numPages = (userLastPage - userPageStart)/PAGE_SIZE_4K_BYTES + 1;
    if (currTask->quickPagesMapped || numPages > 2) {
        // Map with normal mapToKernel call
        return mapToKernelNormal(userPtr, size, pageCount);
    }

    const PageTable *userPageTable = currTask->userPageTable();
    PageTable *kernPageTable = PageTable::kernelPageTable();

    TzMem::VirtAddr va = currTask->quickPages;

    uint8_t *curr = userPageStart;
    uint8_t *currVa = (uint8_t *)va;
    while (curr <= userLastPage) {
        PageTable::EntryAttribs attribs;
        TzMem::PhysAddr pa = userPageTable->lookUp(curr, &attribs);
        if (pa == nullptr) {
            // printf("NOT mapped in user page table\n");
            kernPageTable->unmapPageRange(va, currVa - 1, false);
            return nullptr;
        }

        kernPageTable->mapPage(currVa, pa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true, false, attribs.nonSecure);

        curr += PAGE_SIZE_4K_BYTES;
        currVa += PAGE_SIZE_4K_BYTES;
    }
    currTask->quickPagesMapped = true;

    *pageCount = numPages;
    return va;
}

void SysCalls::unmap(TzMem::VirtAddr vaFirstPage, int numPages) {
    TzTask *currTask = TzTask::current();
    if (currTask == nullptr) {
        err_msg("%s: sys call made from an invalid task !\n", __PRETTY_FUNCTION__);
        return;
    }

    if (!currTask->quickPagesMapped || vaFirstPage != currTask->quickPages) {
        // Mapped with normal mapToKernel call
        unmapNormal(vaFirstPage, numPages);
        return;
    }

    uint8_t *vaLastPage = (uint8_t *)vaFirstPage + numPages*PAGE_SIZE_4K_BYTES - 1;
    PageTable *kernPageTable = PageTable::kernelPageTable();
    kernPageTable->unmapPageRange(vaFirstPage, vaLastPage, false);
    currTask->quickPagesMapped = false;
}

void SysCalls::notImpl(TzTask *currTask) {
    currTask->writeUserReg(TzTask::UserRegs::r0, -ENOTSUP);
}

void SysCalls::doGetRandom(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    size_t buflen = (size_t)arg1;

    int numPages = 0;
    TzMem::VirtAddr userBuffer = (TzMem::VirtAddr)(arg0);
    TzMem::VirtAddr kva = mapToKernel(userBuffer, buflen, &numPages);
    if (kva == nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
        return;
    }

    int pageOffset = (uint8_t *)userBuffer - (uint8_t *)PAGE_START_4K(userBuffer);
    uint8_t *buffer = (uint8_t *)kva + pageOffset;

    size_t rv = CryptoPRNG::instance()->read(buffer, buflen);

    unmap(kva, numPages);

    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doSecComp(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);

    if (arg0 != SECCOMP_SET_MODE_STRICT) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
        return;
    }

    currTask->enableSecCompStrict();
    currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}

void SysCalls::doGetCurrCpu(TzTask *currTask) {
    currTask->writeUserReg(TzTask::UserRegs::r0, arm::smpCpuNum());
}

void SysCalls::doGenSgi(TzTask *currTask) {
    int irq = (int)currTask->userReg(TzTask::UserRegs::r0);
    GIC::sgiGenerate(irq);
    currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}

void SysCalls::doCacheInval(TzTask *currTask) {
    doCacheOp(currTask, CacheInval);
}

void SysCalls::doCacheClean(TzTask *currTask) {
    doCacheOp(currTask, CacheClean);
}

void SysCalls::doCacheOp(TzTask *currTask, CacheOp cacheOp) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    TzMem::VirtAddr userPtr = (TzMem::VirtAddr)(arg0);
    size_t size = (size_t)arg1;

    // CTR - Cache Type Register
    // register uint32_t ctr;
    // asm volatile ("mrc p15, 0, %0, c0, c0, 1" : "=r" (ctr));
    // size_t lineSize = 4 << ((ctr >> 16) & 0xf);
    size_t lineSize = CORTEX_A15_CACHE_LINE_SIZE;

    if (((uint32_t)userPtr & (lineSize - 1)) ||
        ((uint32_t)size & (lineSize - 1))) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
        return;
    }

    uint8_t *userPageStart = (uint8_t *)PAGE_START_4K(userPtr);
    uint8_t *userPageLast  = (uint8_t *)PAGE_START_4K((uint8_t *)userPtr + size - 1);

    uint8_t *userPage = userPageStart;
    int offsetStart = (uint8_t *)userPtr - userPageStart;
    int offsetLast  = ((uint8_t *)userPtr + size - 1) - userPageLast;

    while (userPage <= userPageLast) {
        int numPages = 0;
        uint8_t *kernPage = (uint8_t *)mapToKernel(userPage, PAGE_SIZE_4K_BYTES, &numPages);
        if (kernPage == nullptr) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
            return;
        }

        uint32_t kvaStart = (uint32_t)kernPage + ((userPage == userPageStart) ? offsetStart : 0);
        uint32_t kvaLast  = (uint32_t)kernPage + ((userPage == userPageLast)  ? offsetLast  : PAGE_SIZE_4K_BYTES - 1);

        register uint32_t mva;
        for (mva = kvaStart; mva < kvaLast; mva += lineSize) {
            if (cacheOp == CacheInval) {
                // DCIMVAC - Invalidate data cache by MVA to PoC
                asm volatile ("mcr p15, 0, %0, c7, c6, 1" : : "r" (mva));
            }
            else {
                // DCCMVAC - Clean data cache by MVA to PoC
                asm volatile ("mcr p15, 0, %0, c7, c10, 1" : : "r" (mva));
            }
        }

        unmap(kernPage, numPages);
        userPage += PAGE_SIZE_4K_BYTES;
    }

    asm volatile("dsb");
    currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}
