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
#include "smcutils.h"
#include <cstdint>

#include "platform.h"
#include "smcutils.h"
#include "tztask.h"
#include "tzioc.h"

void *System::cpuBootAddress[MAX_NUM_CPUS];
SmcCalls::SmcPerformer SmcCalls::dispatchTable[NUM_SMC_CALLS];

uint64_t nsRegs[ARCH_SPECIFIC_NUM_CORE_REGS*MAX_NUM_CPUS];

extern "C" void tz_secure_smc_handler();


extern "C" void smcService() {
    SmcCalls::dispatch();
}

void SmcCalls::init() {
	uint32_t ret;
	printf("SmcCalls::init called\n");

    // Fill the dispatch table with handlers.
    dispatchTable[SMC_NORMAL_SWITCH] = doNormalSwitch;

	// Set the smc handler in the SM
	asm volatile("mov	x1, %[xt]":: [xt] "r" (tz_secure_smc_handler):"x1");
	asm volatile("mov	x0, #0x3c000000":::"x0");
	asm volatile("orr	x0, x0, #1":::"x0");
	asm volatile("smc #0": : :"x0");
	asm volatile("mov %[xt], x0": [xt] "=r" (ret)::"x0");
	if(ret) {
		printf("Failed to set NS smc handler\n");
	}
}

unsigned long SmcCalls::readNSReg(NSRegs reg) {
	return nsRegs[reg + (arm::smpCpuNum()*ARCH_SPECIFIC_NUM_CORE_REGS)];
}

void SmcCalls::writeNSReg(NSRegs reg, unsigned long value) {
	nsRegs[reg + (arm::smpCpuNum()*ARCH_SPECIFIC_NUM_CORE_REGS)] = value;
}

void SmcCalls::dispatch() {
    unsigned int smcCallNum = readNSReg(r0)&0xff;

    //printf("Got SMC call %lx cpu %d\n", readNSReg(r0), arm::smpCpuNum());
    if (smcCallNum < NUM_SMC_CALLS) {
        if (dispatchTable[smcCallNum] != nullptr)
            dispatchTable[smcCallNum]();
        else {
            // Return -ENOSYS for unimplemented system call.
            printf("unhandled smc call %d\n", smcCallNum);
			writeNSReg(r0, -ENOSYS);
        }
    }
	else {
		// Return -ENOSYS for out of range system call.
		printf("smc call %d out of range\n", smcCallNum);
		writeNSReg(r0, -ENOSYS);
	}
}

void SmcCalls::doNormalSwitch() {
    // Call tzioc processing
    TzIoc::proc();
}
