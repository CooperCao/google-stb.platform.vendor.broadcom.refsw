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
 * clock.h
 *
 *  Created on: Feb 9, 2015
 *      Author: gambhire
 */

#ifndef INCLUDE_CLOCK_H_
#define INCLUDE_CLOCK_H_

#include "tztime.h"

class TzTask;

namespace TzClock {

void init();

class RealTime {
public:
    static void init();

    static void resolution(timespec *ts);

    static void time(timespec *ts);
    static void setTime(timespec *ts);
    static void adjustTime(timespec *delta, timespec *oldDelta);

private:
    RealTime();
    ~RealTime();

    static uint64_t timeBase;
    static uint64_t currAdjustment;
    static uint64_t adjustmentStep;
    static uint64_t lastAdjustedAt;
    static int numAdjSteps;
};

class Monotonic {
public:
    static void init();
    static Monotonic *instance();

    static void resolution(timespec *ts);

    static void time(timespec *ts);
    static void rawTime(timespec *ts);

    static bool adjustTime(timespec *delta, timespec *oldDelta);

private:
    Monotonic();
    ~Monotonic();

    static uint64_t currAdjustment;
    static uint64_t adjustmentStep;
    static uint64_t lastAdjustedAt;
    static int numAdjSteps;
};

class TaskClock {
public:
    TaskClock(const TzTask *task);
    ~TaskClock();

    static void resolution(timespec *ts);

    void time(timespec *ts);
    void setTime(timespec *ts);

private:
    uint64_t timeBase;
    const TzTask *task;
};

}

#endif /* INCLUDE_CLOCK_H_ */
