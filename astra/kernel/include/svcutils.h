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
 * svc_utils.h
 *
 *  Created on: Jan 26, 2015
 *      Author: gambhire
 */

#ifndef INCLUDE_SVCUTILS_H_
#define INCLUDE_SVCUTILS_H_

#include "tztask.h"
#include "pgtable.h"
#include "syscalls.h"

class SysCalls {
public:
    static void init();

    template <typename T>
    static bool copyFromUser(const T* userPtr, T* kernelPtr) {
        return fromUser(userPtr, kernelPtr, sizeof(T));
    }

    template <typename T>
    static bool copyToUser( T* userPtr, const T* kernelPtr) {
        return toUser(userPtr, kernelPtr, sizeof(T));
    }

    static bool fromUser(const void *userPtr, void *kernelPtr, const size_t size);

    static bool toUser(void *userPtr, const void *kernelPtr, const size_t size);

    static void dispatch();
    static TzMem::VirtAddr mapToKernel(const void *userPtr, size_t size, int *numPages);
    static void unmap(TzMem::VirtAddr va, int numPages);

private:
    static bool strFromUser(const char *userStr, char *kernelStr, const size_t maxLen, bool *truncated);

    static bool ptrArrayFromUser(const char **userArray, char **kernelArray, const size_t maxLen);

    static bool validateUserMemAccess(const void *userPtr, size_t size);
    static TzMem::PhysAddr userMemPhysAddr(const void *userPtr);

    static TzMem::VirtAddr mapToKernelNormal(const void *userPtr, size_t size, int *numPages);
    static TzMem::VirtAddr mapStrToKernel(const char *userPtr, size_t maxLen, int *numPages, int *strLen, bool *truncated);
    static TzMem::VirtAddr mapPtrArrayToKernel(const char **userPtr, size_t maxLen, int *numPages, int *strLen);

    static void unmapNormal(TzMem::VirtAddr va, int numPages);

private:
    static void doSetTidAddress(TzTask *currTask);

    static void doPoll(TzTask *currTask);

    static void doIoctl(TzTask *currTask);

    static const int MAX_NUM_IOV = 8;
    static void doWritev(TzTask *currTask);
    static void doReadv(TzTask *currTask);

    static void doExit(TzTask *currTask);

    static void doSigProcMask(TzTask *currTask);

    static void doFork(TzTask *currTask);

    static void doGetTid(TzTask *currTask);

    static void doWait4(TzTask *currTask);

    static void doClockGetRes(TzTask *currTask);
    static void doClockGetTime(TzTask *currTask);
    static void doClockSetTime(TzTask *currTask);

    static void doNanoSleep(TzTask *currTask);

    static void doExecve(TzTask *currTask);

    static void doOpen(TzTask *currTask);
    static void doRead(TzTask *currTask);
    static void doWrite(TzTask *currTask);
    static void doClose(TzTask *currTask);
    static void dollSeek(TzTask *currTask);
    static void dolSeek(TzTask *currTask);
    static void doCreat(TzTask *currTask);

    static void dogetdents(TzTask *currTask);
    static void doLink(TzTask *currTask);
    static void doUnlink(TzTask *currTask);

    static void doChdir(TzTask *currTask);
    static void doFchdir(TzTask *currTask);

    static void doMknod(TzTask *currTask);

    static void doChmod(TzTask *currTask);
    static void doFchmod(TzTask *currTask);

    static void doChown(TzTask *currTask);
    static void doFchown(TzTask *currTask);

    static void doGetPid(TzTask *currTask);

    static void doMount(TzTask *currTask);
    static void doUmount(TzTask *currTask);

    static void doSetUid(TzTask *currTask);
    static void doGetUid(TzTask *currTask);
    static void doSetGid(TzTask *currTask);
    static void doGetGid(TzTask *currTask);

    static void doPtrace(TzTask *currTask);

    static void doPause(TzTask *currTask);

    static void doAccess(TzTask *currTask);

    static void doNice(TzTask *currTask);

    static void doSync(TzTask *currTask);
    static void doFSync(TzTask *currTask);

    static void doRename(TzTask *currTask);
    static void doMkdir(TzTask *currTask);
    static void doRmdir(TzTask *currTask);

    static void doDup(TzTask *currTask);
    static void doDup2(TzTask *currTask);

    static void doBrk(TzTask *currTask);

    static void doGetSchedulerParam(TzTask *currTask);
    static void doSetSchedulerParam(TzTask *currTask);
    static void doGetScheduler(TzTask *currTask);
    static void doSetScheduler(TzTask *currTask);

    static void doStat64(TzTask *currTask);
    static void doFStat64(TzTask *currTask);

    static void doMmap(TzTask *currTask);
    static void doUnmap(TzTask *currTask);

    static void notImpl(TzTask *currTask);

    static void doKill(TzTask *currTask);
    static void doRtSigQueueInfo(TzTask *currTask);
    static void doSigAction(TzTask *currTask);
    static void doSigReturn(TzTask *currTask);

    static void doFutex(TzTask *currTask);
    static void doClone(TzTask *currTask);

    static void doMProtect(TzTask *currTask);

    static void doMqOpen(TzTask *currTask);
    static void doMqUnlink(TzTask *currTask);
    static void doMqTimedSend(TzTask *currTask);
    static void doMqTimedRecv(TzTask *currTask);
    static void doMqNotify(TzTask *currTask);
    static void doMqGetSetAttr(TzTask *currTask);

    static void doGetRandom(TzTask *currTask);

    static void doSecComp(TzTask *currTask);

    /* extended syscalls */
    static void doGetCurrCpu(TzTask *currTask);
    static void doGenSgi(TzTask *currTask);
    static void doSmcWait(TzTask *currTask);
    static void doCacheInval(TzTask *currTask);
    static void doCacheClean(TzTask *currTask);
    static void doSetThreadArea(TzTask *currTask);
    static void doGetThreadArea(TzTask *currTask);
    static void doTraceLogInval(TzTask *currTask);
    static void doTraceLogStart(TzTask *currTask);
    static void doTraceLogStop(TzTask *currTask);
    static void doTraceLogAdd(TzTask *currTask);

    enum CacheOp {
        CacheInval,
        CacheClean
    };

    static void doCacheOp(TzTask *currTask, CacheOp cacheOp);

private:
    typedef void (*SvcPerformer)(TzTask *currTask);

    static SvcPerformer dispatchTable[NUM_SYS_CALLS];
    static SvcPerformer dispatchTableExt[NUM_EXT_SYS_CALLS];
    static TzMem::PhysAddr paramsPagePhys;
    static TzMem::VirtAddr paramsPage;

    static spinlock_t execLock;
    static spinlock_t fopsLock;
};


#endif /* INCLUDE_SVCUTILS_H_ */
