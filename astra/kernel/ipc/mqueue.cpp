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
 * mqueue.c
 *
 *  Created on: Apr 6, 2015
 *      Author: gambhire
 */

#include "tzmqueue.h"
#include "tztask.h"

#include "lib_string.h"
#include "arm/spinlock.h"

tzutils::Vector<MsgQueue::Entry> MsgQueue::msgQueues;
spinlock_t MsgQueue::creationLock;
ObjCacheAllocator<MsgQueue> MsgQueue::mqAllocator;

void MsgQueue::init() {
    msgQueues.init();
    mqAllocator.init();

    spinlock_init("MsgQueue::creation::lock", &creationLock);
}

void *MsgQueue::operator new(size_t sz) {
    UNUSED(sz);
    return mqAllocator.alloc();
}

void MsgQueue::operator delete(void *mq) {
    mqAllocator.free((MsgQueue *)mq);
}

MsgQueue::MsgQueue(size_t msgPoolSize, mq_attr *attr, uint16_t ownr, uint16_t grp, int mod) : pool(KERNEL_PID, msgPoolSize) {
    waitQueue.init();
    spinlock_init("MsgQueue::lock", &lock);
    listener.task = nullptr;

    nonBlock = (attr->mq_flags == O_NONBLOCK);
    memcpy(&currAttr, attr, sizeof(mq_attr));

    mqueue.init();
    owner = ownr;
    group = grp;
    mode = mod;
}

MsgQueue::~MsgQueue() {

}

MsgQueue *MsgQueue::open(const char *mqName, int mode, mq_attr *attr, uint16_t owner, uint16_t group) {
    SpinLocker locker(&creationLock);

    MsgQueue *mq = nullptr;
    int numQueues = msgQueues.numElements();

    for (int i=0; i<numQueues; i++) {
        if (!strcmp(msgQueues[i].mqName, mqName)) {
            mq = msgQueues[i].mq;
            atomic_incr(&msgQueues[i].refCount);
            break;
        }
    }

    if (mq == nullptr) {
        size_t poolSize = attr->mq_maxmsg * attr->mq_msgsize;
        mq = new MsgQueue(poolSize, attr, owner, group, mode);

        Entry entry;
        strcpy(entry.mqName,mqName);
        entry.mq = mq;
        entry.refCount = 1;
        entry.linkCount = 1;
        msgQueues.pushBack(entry);

    }
    return mq;
}

MsgQueue *MsgQueue::lookUp(const char *mqName) {
    SpinLocker locker(&creationLock);

    MsgQueue *mq = nullptr;
    int numQueues = msgQueues.numElements();
    for (int i=0; i<numQueues; i++) {
        if (!strcmp(msgQueues[i].mqName, mqName)) {
            mq = msgQueues[i].mq;
            atomic_incr(&msgQueues[i].refCount);
            break;
        }
    }

    return mq;
}

bool MsgQueue::close(MsgQueue *mq) {
    SpinLocker locker(&creationLock);

    int numQueues = msgQueues.numElements();
    for (int i=0; i<numQueues; i++) {
        if (msgQueues[i].mq != mq)
            continue;

        atomic_decr(&msgQueues[i].refCount);

        Entry entry = msgQueues[i];
        if ((entry.linkCount == 0) && (entry.refCount == 0)) {
            delete entry.mq;

            msgQueues[i] = msgQueues[numQueues - 1];
            msgQueues.popBack(nullptr);
        }

        return true;
    }

    return false;
}

bool MsgQueue::unlink(const char *mqName) {
    SpinLocker locker(&creationLock);

    int numQueues = msgQueues.numElements();
    for (int i=0; i<numQueues; i++) {
        if (strcmp(msgQueues[i].mqName, mqName))
            continue;

        atomic_decr(&msgQueues[i].linkCount);

        Entry entry = msgQueues[i];
        if ((entry.linkCount == 0) && (entry.refCount == 0)) {
            delete entry.mq;

            msgQueues[i] = msgQueues[numQueues - 1];
            msgQueues.popBack(nullptr);
        }

        return true;
    }

    return false;
}

bool MsgQueue::addListener(TzTask *task, int sigNum, union sigval sigVal) {
    SpinLocker locker(&lock);

    if (listener.task != nullptr)
        return false;

    listener.task = task;
    listener.notifySigNum = sigNum;
    listener.notifySigVal = sigVal;

    return true;
}

void MsgQueue::removeListener() {
    SpinLocker locker(&lock);

    listener.task = nullptr;
}

bool MsgQueue::send(const char *msgPtr, size_t msgLen, unsigned int priority) {
    SpinLocker locker(&lock);

    char *msgCopy = (char *)pool.alloc(msgLen);
    if (msgCopy == nullptr)
        return false;
    memcpy(msgCopy, msgPtr, msgLen);

    Msg msg;
    msg.data = msgCopy;
    msg.size = msgLen;
    msg.priority = priority;

    // Insert into a priority sorted message queue
    mqueue.pushBack(msg);
    int numElements = mqueue.numElements();
    int i = numElements - 1;
    while ((i > 0) && (mqueue[i-1].priority < msg.priority)) {
        mqueue[i].data = mqueue[i-1].data;
        mqueue[i].size = mqueue[i-1].size;
        mqueue[i].priority = mqueue[i-1].priority;

        i--;
    }

    if (i != numElements-1) {
        mqueue[i].data = msg.data;
        mqueue[i].size = msg.size;
        mqueue[i].priority = msg.priority;
    }

    // Notify any registered listener task.
    if (listener.task != nullptr) {
        siginfo_t sigInfo;
        memset(&sigInfo, 0, sizeof(siginfo_t));

        sigInfo.si_code = SI_MESGQ;
        sigInfo.si_signo = listener.notifySigNum;
        sigInfo.si_value = listener.notifySigVal;

        listener.task->sigQueue(sigInfo.si_signo, &sigInfo);
    }

    // Wake up any tasks pending on this message queue.
    waitQueue.signal();

    currAttr.mq_curmsgs++;

    return true;
}

long MsgQueue::recv(char *msgPtr, size_t msgLen, unsigned int *priority, uint64_t timeout) {

    spin_lock(&lock);

    int numElements = mqueue.numElements();
    if (numElements == 0) {
        if (nonBlock) {
            spin_unlock(&lock);
            return -EAGAIN;
        }

        waitQueue.unlockAndWait(&lock, TzTask::current(), timeout);

        // Update numElements
        numElements = mqueue.numElements();
        if (numElements == 0) {
            spin_unlock(&lock);
            return (timeout != -1) ? -ETIMEDOUT : -EINTR;
        }
    }

    Msg msg;
    msg.data = mqueue[0].data;
    msg.size = mqueue[0].size;
    msg.priority = mqueue[0].priority;

    if (msgLen < msg.size) {
        spin_unlock(&lock);
        return -EMSGSIZE;
    }

    memcpy(msgPtr, msg.data, msg.size);
    pool.free((void *)msg.data);

    *priority = msg.priority;

    for (int i=1; i<numElements; i++) {
        mqueue[i-1].data = mqueue[i].data;
        mqueue[i-1].size = mqueue[i].size;
        mqueue[i-1].priority = mqueue[i].priority;
    }
    mqueue.popBack(nullptr);

    spin_unlock(&lock);

    currAttr.mq_curmsgs--;

    return msg.size;

}

void MsgQueue::changeAttr(mq_attr *newAttr, mq_attr *oldAttr) {
    memcpy(oldAttr, &currAttr, sizeof(mq_attr));

    if ((newAttr->mq_flags == O_NONBLOCK) && (!nonBlock)) {
        nonBlock = true;
        currAttr.mq_flags = O_NONBLOCK;
    }

    if ((newAttr->mq_flags != O_NONBLOCK) && (nonBlock)) {
        nonBlock = false;
        currAttr.mq_flags = 0;
    }
}
