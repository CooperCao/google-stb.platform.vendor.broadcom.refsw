/***************************************************************************
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
 ***************************************************************************/
#ifndef TIMER_H_
#define TIMER_H_

#include <cstdint>
#include <cstddef>

#include "utils/priorityqueue.h"

#include "objalloc.h"
#include "interrupt.h"
#include "system.h"
#include "cpulocal.h"

typedef uint64_t Timer;
#define INVALID_TIMER 0xffffffffffffffff

class TzHwCounter {
public:
    static inline unsigned long frequency() {
        register unsigned long rv;
        asm volatile("MRC p15, 0, %[rt], c14, c0, 0": [rt] "=r" (rv) : :);
        return rv;
    }

    static inline uint64_t timeNow() {
        register unsigned long timeLow, timeHigh;
        asm volatile("MRRC p15, 0, %[low], %[high], c14": [low] "=r" (timeLow), [high] "=r" (timeHigh) : : );
        uint64_t rv = ((uint64_t)timeHigh << 32) | timeLow;

#if 0
        rv += System::cpuBootedAt[arm::smpCpuNum()];
#endif

        return rv;
    }

    static void delay(unsigned long ticks) {
        uint64_t target = timeNow() + ticks;
        while (timeNow() < target);
    }

    static uint64_t usToTicks(uint64_t us) {
        return  ((uint64_t)frequency() * us)/1000000;
    }

    static uint64_t ticksToUs(uint64_t ticks) {
        return (ticks * 1000000)/frequency();
    }
};

class TzTimers {
public:
    typedef void (*OnExpiry)(Timer th, void *ctx);

public:
    static void init();
    static void secondaryCpuInit();

    static Timer create(unsigned long countDownDelta, OnExpiry handler, void *handlerCtx);
    static Timer create(uint64_t fireAt, OnExpiry handler, void *handlerCtx);
    static Timer create(uint64_t fireAt, uint64_t period, OnExpiry handler, void *handlerCtx);

    static void destroy(Timer th);

public:
    class TimerEntry {
    public:
        typedef Timer IDType;

        uint64_t fireAt;
        Timer handle;
        OnExpiry callback;
        void *ctx;
        uint64_t period;
        bool periodicTimer;
        static void init();

        IDType id() { return handle; }

        bool dominates(TimerEntry *other) {
            if (other == nullptr)
                return true;

            return (fireAt <= other->fireAt);
        }

        inline uint64_t pqValue() { return fireAt; }
        inline bool isPeriodic() { return periodicTimer; }

        void *operator new(size_t sz);
        void operator delete(void *);
    };

private:
    static void restartHwTimer();
    static void hwTimerFired();

    friend ISRStatus tzTimerISR(int);

    typedef tzutils::PriorityQueue<TimerEntry> Timers;

    static PerCPU<Timers> timers;
    static spinlock_t lock;
};


#endif /* TIMER_H_ */
