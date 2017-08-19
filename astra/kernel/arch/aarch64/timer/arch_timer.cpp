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
#include "libfdt.h"
#include "arch_timer.h"
#include "plat_config.h"
#include "parse_utils.h"

#define CNTVCTL_ENABLE 0x1
#define PPI_START 16

static int intrNumNS;
static int intrNumS;

void Arch::Timer::init(void *deviceTree) {
	int nodeOffset = fdt_subnode_offset(deviceTree, 0, "timer");
	if(!(nodeOffset > 0))
		ARCH_HALT();

	// Parse the 'interrupts' property.
	int propLen = 0;
	const struct fdt_property *fpIntr =
				fdt_get_property(deviceTree, nodeOffset, "interrupts", &propLen);
	if(!(fpIntr != nullptr))
		ARCH_HALT();

	const char *rangeData = fpIntr->data;
	const char *secureTimerRange = rangeData;
	const char *nsTimerRange = secureTimerRange + 3*sizeof(uint32_t);
	const char *virtTimerRange = nsTimerRange + 3*sizeof(uint32_t);

	intrNumNS = PPI_START + parseInt(virtTimerRange+sizeof(uint32_t), sizeof(uint32_t));
	intrNumS = PPI_START + parseInt(secureTimerRange+sizeof(uint32_t), sizeof(uint32_t));

	/* Enable user access to virtual timer */
	register unsigned long cntkctl;;
	asm volatile("mrs %[xt], CNTKCTL_EL1": [xt] "=r" (cntkctl)::);
	cntkctl = cntkctl | 0x2; //Enable user accesses to the CNTFRQ_EL0 and CNTVCT_EL0.
	asm volatile("msr CNTKCTL_EL1, %[xt]": : [xt] "r" (cntkctl):);

}

uint64_t Arch::Timer::frequency() {
	register unsigned long rv;
	asm volatile("mrs %[xt], CNTFRQ_EL0": [xt] "=r" (rv)::);

	return rv;
}

uint32_t Arch::Timer::irqNum() {
	return intrNumS;
}

uint64_t Arch::Timer::ticks() {
	register unsigned long rv;
	asm volatile("mrs %[xt], CNTPCT_EL0": [xt] "=r" (rv)::);
	return rv;
}

void Arch::Timer::fireAt(uint64_t ticks) {
	asm volatile("msr CNTPS_CVAL_EL1, %[xt]": : [xt] "r" (ticks) :);

	register unsigned long cntvctl = CNTVCTL_ENABLE;
	asm volatile("msr cntps_ctl_el1, %[xt]": : [xt] "r" (cntvctl):);
}

uint64_t Arch::Timer::firesAt() {
	register unsigned long cntvctl = 0;
	asm volatile("mrs %[xt], cntps_ctl_el1": [xt] "=r" (cntvctl)::);

	if (!(cntvctl & CNTVCTL_ENABLE))
		return ~0UL;

	register unsigned long ticks = 0;
	asm volatile("mrs %[xt], CNTPS_CVAL_EL1": [xt] "=r" (ticks)::);
	return ticks;
}


void Arch::Timer::disable() {
	register uint32_t enable = 0;
	asm volatile("msr cntps_ctl_el1, %[xt]" : : [xt] "r" (enable) :);
}

void Arch::Timer::enable() {
	register uint32_t enable = 1;
	asm volatile("msr cntps_ctl_el1, %[xt]" : : [xt] "r" (enable) :);

}
