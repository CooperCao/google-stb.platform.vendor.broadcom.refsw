/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ***************************************************************************/

#include "libfdt.h"
#include "arch_timer.h"
#include "parse_utils.h"
#include "config.h"
#include "kernel.h"
#include "plat_config.h"
#include "arm32.h"

#define CNTPCTL_ENABLE 0x1
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
}

uint64_t Arch::Timer::frequency() {
	unsigned long freq = 0;
	ARCH_SPECIFIC_GET_SECURE_TIMER_FREQUENCY(freq);
	return freq;
}

uint32_t Arch::Timer::irqNum() {
	return intrNumS;
}

uint64_t Arch::Timer::ticks() {
	uint64_t val = 0;
	ARCH_SPECIFIC_GET_SECURE_TIMER_CURRENT_TIME(val);
	return val;
}

void Arch::Timer::fireAt(uint64_t ticks) {
	// Set the hardware timer to fire at headTime
	if (boot_mode == ARMV8_BOOT_MODE) {
		ARCH_V8ARM32_SPECIFIC_SET_STIMER_CVAL(ticks);
		ARCH_V8ARM32_SPECIFIC_SET_STIMER_CTL(CNTPCTL_ENABLE);
	}
	else {
		ARCH_V7ARM32_SPECIFIC_SET_STIMER_CVAL(ticks);
		ARCH_V7ARM32_SPECIFIC_SET_STIMER_CTL(CNTPCTL_ENABLE);
	}
}

uint64_t Arch::Timer::firesAt() {
	uint32_t cntpctl = 0;
	if (boot_mode == ARMV8_BOOT_MODE) {
		ARCH_V8ARM32_SPECIFIC_GET_STIMER_CTL(cntpctl);
	}
	else {
		ARCH_V7ARM32_SPECIFIC_GET_STIMER_CTL(cntpctl);
	}

	if (!((cntpctl & CNTPCTL_ENABLE)  && ((cntpctl & 2) == 0)))
		return 0xFFFFFFFFFFFFFFFF;

	uint64_t cval = 0;

	if (boot_mode == ARMV8_BOOT_MODE) {
		ARCH_V8ARM32_SPECIFIC_GET_STIMER_CVAL(cval);
	}
	else {
		ARCH_V7ARM32_SPECIFIC_GET_STIMER_CVAL(cval);
	}

	return cval;
}


void Arch::Timer::disable() {
	if (boot_mode == ARMV8_BOOT_MODE) {
		ARCH_V8ARM32_SPECIFIC_SET_STIMER_CTL(0);
	}
	else {
		ARCH_V7ARM32_SPECIFIC_SET_STIMER_CTL(0);
	}
}

void Arch::Timer::enable() {
	if (boot_mode == ARMV8_BOOT_MODE) {
		ARCH_V8ARM32_SPECIFIC_SET_STIMER_CTL(CNTPCTL_ENABLE);
	}
	else {
		ARCH_V7ARM32_SPECIFIC_SET_STIMER_CTL(CNTPCTL_ENABLE);
	}
}
