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

#include "platform.h"
#include "smcutils.h"
#include "tztask.h"
#include "tzioc.h"

void *System::cpuBootAddress[MAX_NUM_CPUS];
SmcCalls::SmcPerformer SmcCalls::dispatchTable[NUM_SMC_CALLS];

extern "C" void tz_bootstrap();

extern uint32_t nsRegs[ARCH_SPECIFIC_NUM_CORE_REGS*MAX_NUM_CPUS];
//extern uint32_t nsFPEXC[MAX_NUM_CPUS];
//extern uint32_t nsFPSCR[MAX_NUM_CPUS];
//extern uint64_t nsNeonRegs[V7AR_NUM_NEON64_REGS*MAX_NUM_CPUS];

extern "C" void smcService();

void smcService() {
    SmcCalls::dispatch();
}

void SmcCalls::init() {
    // Fill the dispatch table with handlers.
    dispatchTable[SMC_CPU_ON] = doCpuPowerOn;
    dispatchTable[SMC_CPU_OFF] = doCpuPowerOff;
    dispatchTable[SMC_SYSTEM_RESET] = doReboot;
    dispatchTable[SMC_NORMAL_SWITCH] = doNormalSwitch;
}

unsigned long SmcCalls::readNSReg(NSRegs reg) {
	return nsRegs[reg + (arm::smpCpuNum()*ARCH_SPECIFIC_NUM_CORE_REGS)];
}

void SmcCalls::writeNSReg(NSRegs reg, unsigned long value) {
	nsRegs[reg + (arm::smpCpuNum()*ARCH_SPECIFIC_NUM_CORE_REGS)] = value;
}

void SmcCalls::dispatch() {
    unsigned int smcCallNum = readNSReg(r0);

    //printf("Got SMC call %x nsRegs %p cpu %d\n", smcCallNum, nsRegs, arm::smpCpuNum());
    if (smcCallNum < NUM_SMC_CALLS) {
        if (dispatchTable[smcCallNum] != nullptr)
            dispatchTable[smcCallNum]();
        else {
            // Return -ENOSYS for unimplemented system call.
            printf("unhandled smc call %d\n", smcCallNum);
            writeNSReg(r0, -ENOSYS);
        }
    }
}

void SmcCalls::doNormalSwitch() {
    // Call tzioc processing
    TzIoc::proc();
}

void SmcCalls::doCpuPowerOn() {
    unsigned long arg0 = readNSReg(r1);
    unsigned long arg1 = readNSReg(r2);

    int cpuNum = (int)arg0;
    void *physBootAddr = (void *)arg1;
    if ((cpuNum < 0) || (cpuNum > MAX_NUM_CPUS)) {
        writeNSReg(r0, -EINVAL);
        return;
    }

    System::cpuBootAddress[cpuNum] = physBootAddr;

    TzMem::PhysAddr entry = TzMem::virtToPhys((const void *)tz_bootstrap);

	uint64_t now;
	ARCH_SPECIFIC_GET_SECURE_TIMER_CURRENT_TIME(now);

    Platform::powerOn(cpuNum, entry);
    System::cpuBootedAt[cpuNum] = now;

    writeNSReg(r0, 0);
}

void SmcCalls::doCpuPowerOff() {
    unsigned long arg0 = readNSReg(r0);
    int cpuNum = (int)arg0;

    Platform::powerOff(cpuNum);

    writeNSReg(r0, 0);
}

void SmcCalls::doReboot() {
    Platform::reboot();
}
