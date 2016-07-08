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

#include "config.h"
#include "arm/gic.h"
#include "interrupt.h"
#include "tztask.h"
#include "tzioc.h"
#include "cpulocal.h"

#include "lib_printf.h"

PerCPU<Interrupt::HandlerArray> Interrupt::intrRecs;

void Interrupt::init() {
    for (int i=0; i<MaxNumInterrupts; i++) {
        Interrupt::HandlerArray& handlers = intrRecs.cpuLocal();
        handlers[i].gicIntrNum = TZ_INVALID_IRQ;
    }
}

Interrupt::ISRef Interrupt::isrRegister(int gicIntrNum, TzIsr isr) {
    for (int i=0; i<MaxNumInterrupts; i++) {
        Interrupt::HandlerArray& handlers = intrRecs.cpuLocal();
        if (handlers[i].gicIntrNum == TZ_INVALID_IRQ) {
            handlers[i].gicIntrNum = gicIntrNum;
            handlers[i].isr = isr;
            return i;
        }
    }

    err_msg("Out of ISR slots\n");
    return MaxNumInterrupts;
}

void Interrupt::isrRemove(Interrupt::ISRef ref) {
    Interrupt::HandlerArray& handlers = intrRecs.cpuLocal();
    handlers[ref].gicIntrNum = TZ_INVALID_IRQ;
}

void fiqService() {
    int src;
    int intrNum = GIC::currIntr(&src);

    for (int i=0; i<Interrupt::MaxNumInterrupts; i++) {
        Interrupt::HandlerArray& handlers = Interrupt::intrRecs.cpuLocal();
        if (handlers[i].gicIntrNum == intrNum) {
            ISRStatus status = handlers[i].isr(intrNum);
            if (status == IntrDone) {
                GIC::endIntr(intrNum);
            }

            // Call tzioc processing when coming from normal world
            TzTask *currTask = TzTask::current();
            if (currTask->id() == 0) {
                TzIoc::proc();
            }
            return;
        }
    }

    printf("%s: interrupt %d - No handler registered\n", __FUNCTION__, intrNum);
}
