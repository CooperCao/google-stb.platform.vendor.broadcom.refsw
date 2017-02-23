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

#ifndef INCLUDE_FUTEX_H_
#define INCLUDE_FUTEX_H_

#include <cstdint>

#include "waitqueue.h"
#include "objalloc.h"
#include "arm/spinlock.h"

#include "utils/vector.h"

class Futex {
public:
    static void init();

    Futex(int *physAddr);
    ~Futex();

    int wait(int ctrVal, uint64_t timeout);
    int wake(int numTasks);

    int requeue(int numTasks, Futex *other);
    int cmpRequeue(int ctrVal, int numTasks, Futex *other);

    void *operator new (size_t sz);
    void operator delete (void *task);

    int numWaitingTasks() { return waitQueue.numWaitingTasks(); }

    friend class FutexStore;

private:
    const int *ctrAddr;
    int *ctrVA;

    WaitQueue waitQueue;
    SpinLock lock;

    static ObjCacheAllocator<Futex> allocator;
};

class FutexStore {
public:
    static void init() { store.init(); }

    static void add(Futex *futex) {
        store.pushBack(futex);
    }

    static Futex *lookUp(int *addr) {
        int nf = store.numElements();
        for (int i=0; i<nf; i++) {
            if (store[i]->ctrAddr == addr)
                return store[i];
        }
        return nullptr;
    }

    static Futex *remove(int *addr) {
        int nf = store.numElements();
        for (int i=0; i<nf; i++) {
            if (store[i]->ctrAddr != addr)
                continue;

            Futex *rv = store[i];
            store[i] = store[nf-1];
            store.popBack(nullptr);
            return rv;
        }
        return nullptr;
    }

    static Futex *remove(Futex *futex) {
        int nf = store.numElements();
        for (int i=0; i<nf; i++) {
            if (store[i] != futex)
                continue;

            Futex *rv = store[i];
            store[i] = store[nf-1];
            store.popBack(nullptr);
            return rv;
        }
        return nullptr;
    }

private:
    static tzutils::Vector<Futex *> store;
};

#endif /* INCLUDE_FUTEX_H_ */
