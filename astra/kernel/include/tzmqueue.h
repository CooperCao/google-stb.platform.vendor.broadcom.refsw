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
 * tzmqueue.h
 *
 *  Created on: Apr 6, 2015
 *      Author: gambhire
 */

#ifndef INCLUDE_TZMQUEUE_H_
#define INCLUDE_TZMQUEUE_H_

#include "config.h"
#include "mempool.h"
#include "utils/vector.h"
#include "utils/priorityqueue.h"
#include "objalloc.h"
#include "waitqueue.h"
#include "tzsignal.h"

#include "arm/spinlock.h"

// POSIX message queue attribute structure.
struct mq_attr {
    long mq_flags;       /* Flags: 0 or O_NONBLOCK */
    long mq_maxmsg;      /* Max. # of messages on queue */
    long mq_msgsize;     /* Max. message size (bytes) */
    long mq_curmsgs;     /* # of messages currently in queue */
};

class MsgQueue {
public:
    static const int MaxNumMsgs = 16;
    static const int MaxMsgSize = 4096;
public:
    static void init();

    static MsgQueue *open(const char *mqName, int mode, mq_attr *attr, uint16_t owner, uint16_t group);
    static MsgQueue *lookUp(const char *mqName);
    static bool close(MsgQueue *mq);
    static bool unlink(const char *mqName);

    bool send(const char *msgPtr, size_t msgLen, unsigned int priority);
    long recv(char *msgPtr, size_t msgLen, unsigned int *priority, uint64_t timeout);

    bool addListener(TzTask *listenerTask, int notifySigNum, union sigval notifySigVal);
    void removeListener();

    void changeAttr(mq_attr *newAttr, mq_attr *oldAttr);

    uint16_t uid() { return owner; }
    uint16_t gid() { return group; }
    int access() { return mode; }


private:
    MsgQueue(size_t msgPoolSize, mq_attr *attr, uint16_t owner, uint16_t group, int mode);
    ~MsgQueue();

    void *operator new (size_t sz);
    void operator delete (void *task);

    MsgQueue() = delete;
    MsgQueue(const MsgQueue& rhs) = delete;
    MsgQueue& operator = (const MsgQueue& rhs) = delete;

private:
    struct Msg {
        const char *data;
        size_t size;
        unsigned int priority;

    };

    struct Entry {
        char mqName[256];
        MsgQueue *mq;
        int refCount;
        int linkCount;
      public:
            Entry& operator = (const Entry& rhs) {
                strcpy(mqName, rhs.mqName);
                mq = rhs.mq;
                refCount = rhs.refCount;
                linkCount = rhs.linkCount;

                return *this;
            }

    };
    static tzutils::Vector<Entry> msgQueues;
    static spinlock_t creationLock;
    static ObjCacheAllocator<MsgQueue> mqAllocator;

    struct NotifyCtx {
        TzTask *task;
        int notifySigNum;
        union sigval notifySigVal;
    };
    NotifyCtx listener;

private:
    MemPool pool;
    mq_attr currAttr;

    tzutils::Vector<Msg> mqueue;
    WaitQueue waitQueue;
    bool nonBlock;

    uint16_t owner;
    uint16_t group;
    int mode;

    spinlock_t lock;

};


#endif /* INCLUDE_TZMQUEUE_H_ */
