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

#include <cstdint>

#include "config.h"
#include "kernel.h"
#include "pgtable.h"
#include "console.h"
#include "libfdt.h"
#include "parse_utils.h"
#include "gicv2.h"
#include "lib_printf.h"
static uint8_t *distributor;
static uint8_t *cpuInterface;

static int numInterruptLines;
static int numCpuInterfaces;

#define ARM_CORTEX_A15_SECURE_TIMER_IRQ   29
#define TZ_SECURE_SGI_IRQ   14

static inline void regWrite32(void *location, uint32_t value) {
	volatile uint32_t *ptr = (volatile uint32_t *)location;
	*ptr = value;
	COMPILER_BARRIER();
}

static inline uint32_t regRead32(void *location) {
	volatile uint32_t *ptr = (volatile uint32_t *)location;
	uint32_t rv = *ptr;
	COMPILER_BARRIER();
	return rv;
	COMPILER_BARRIER();
}

static void gicDistributorInit(void *va) {
	distributor = (uint8_t *)va;

	if (ARMV7_BOOT_MODE == boot_mode) {
		// Extract the interrupt line count and cpu interface count from GICD_TYPER
		uint32_t typer = regRead32(distributor+GICD_TYPER);
		numInterruptLines = 32 * ((typer & 0x1f) + 1);
		numCpuInterfaces = ((typer >> 5) & 0x7) + 1;

		// Disable the distributor
		regWrite32(distributor+GICD_CTLR, 0);

		//Assign interrupts to groups:
		//		Group 0: All secure interrupts meant for us (the TZ OS ).
		//		Group 1: All normal world interrupts.
		//
		// The TZ OS is only interested in one PPI interrupts: Secure clock.
		uint32_t ppiGroups = ~( 1 << ARM_CORTEX_A15_SECURE_TIMER_IRQ);
		regWrite32(distributor + GICD_IGROUPR, ppiGroups);


		for (int i=32; i < numInterruptLines; i+=32) {
			regWrite32(distributor + GICD_IGROUPR + (i/8), 0xffffffff);
		}

		// Route all SPIs to the current CPU.
		uint32_t cpuMask =regRead32(distributor+GICD_ITARGETSR);
		cpuMask |= (cpuMask << 8);
		cpuMask |= (cpuMask << 16);
		for (int i=32; i<numInterruptLines; i+=4)
			regWrite32(distributor+GICD_ITARGETSR+i, cpuMask);

		// Set all SPIs to level sensitive.
		for (int i=32; i<numInterruptLines; i+=16)
			regWrite32(distributor+GICD_ICFGR+(i/4), 0);

		// Set all SPIs to default priority
		for (int i=32; i<numInterruptLines; i+=4)
			regWrite32(distributor+GICD_IPRIORITYR+i, 0xa0a0a0a0);

		// Disable all SPIs.
		for (int i=32; i<numInterruptLines; i+=32)
			regWrite32(distributor+GICD_ICENABLER+(i/8), 0xffffffff);

		// Enable the distributor
		regWrite32(distributor+GICD_CTLR, 0x03);
	}
}

static void gicCpuInterfaceinit(void *va = nullptr) {
	cpuInterface = (uint8_t *)va;

	if (ARMV7_BOOT_MODE == boot_mode) {
		// Enable all SGIs
		regWrite32(distributor+GICD_ISENABLER, 0xffff0000);

		// Set the default interrupt priority threshold.
		for (int i=0; i<32; i+=4)
			regWrite32(distributor+GICD_IPRIORITYR+i, 0xa0a0a0a0);

		regWrite32(cpuInterface+GICC_PMR, 0xf8);
		regWrite32(cpuInterface+GICC_BPR, 0x0);

		// Enable the CPU interface.
		regWrite32(cpuInterface+GICC_CTLR, 0x1B);
	}
}

int gicV2Init(void *deviceTree) {
	int nodeOffset = fdt_subnode_offset(deviceTree, 0, "interrupt-controller");
	if(!(nodeOffset > 0))
		ARCH_HALT();

	// Parse the #address-cells property of the parent node.
	int propLen;
	unsigned long addrCellSize;
	int parentOffset = fdt_parent_offset(deviceTree, nodeOffset);
	const struct fdt_property *fpAddrCells =
			fdt_get_property(deviceTree, parentOffset, "#address-cells", &propLen);
	if ((!fpAddrCells) || ((unsigned int)propLen < sizeof(int)))
		addrCellSize = 1;
	else
		addrCellSize = parseInt((void *)fpAddrCells->data, propLen);
	int addrByteSize = addrCellSize * sizeof(int);

	// Parse the #size-cells property.
	unsigned long szCellSize;
	const struct fdt_property *fpSzCells =
			fdt_get_property(deviceTree, parentOffset, "#size-cells", &propLen);
	if ((!fpSzCells) || ((unsigned int)propLen < sizeof(int)))
		szCellSize = 1;
	else
		szCellSize = parseInt((void *)fpAddrCells->data, propLen);
	int szByteSize = szCellSize * sizeof(int);

	// Parse the 'reg' property and map the address spaces
	// in kernel page table.
	const struct fdt_property *fpAddrRanges =
			fdt_get_property(deviceTree, nodeOffset, "reg", &propLen);

	if(!(fpAddrRanges != nullptr))
		ARCH_HALT();

	if(!(propLen >= addrByteSize + szByteSize))
		ARCH_HALT();

	int numAddrSpaces = propLen / (addrByteSize + szByteSize);

	const char *rangeData = fpAddrRanges->data;

	for (int i = 0; i < numAddrSpaces; i++) {
		unsigned long rangeStart = (unsigned long)
							((addrCellSize == 1) ?
									parseInt(rangeData, addrByteSize) :
											parseInt64(rangeData, addrByteSize));
		rangeData += addrByteSize;

		unsigned long rangeSize = (unsigned long)
							((szCellSize == 1) ?
									parseInt(rangeData, szByteSize) :
											parseInt64(rangeData, szByteSize));
		rangeData += szByteSize;


		uint8_t *rangeStartAddr = (uint8_t *)rangeStart;
		uint8_t *rangeEndAddr = rangeStartAddr + rangeSize;

		size_t numPages = (rangeEndAddr - rangeStartAddr)/ARCH_PAGE_SIZE;
		uint8_t *va = rangeStartAddr;

		PageTable *kernPageTable = PageTable::kernelPageTable();

		kernPageTable->mapPageRange(
			va,             // virtual start
			(va + numPages * ARCH_PAGE_SIZE),               // virtual end
			rangeStartAddr,             // physical start
			MAIR_DEVICE,           // no caching
			MEMORY_ACCESS_RW_KERNEL, // user read/write
			true,                  // never execute
			true);
		if (i == 0)
			gicDistributorInit(va);
		else if (i == 1)
			gicCpuInterfaceinit(va);
	}

	return numInterruptLines;
}

void gicV2InitSecondary() {
	gicCpuInterfaceinit(cpuInterface);
}

uint32_t gicV2GetInterruptID(uint32_t *sourceCPU) {
	uint32_t giccIAR =  regRead32(cpuInterface+GICC_IAR);
	*sourceCPU = (giccIAR >> GICC_IAR_CPUID_BITPOS) & GICC_IAR_CPUID_MASK;
	return (giccIAR & GICC_IAR_INTID_MASK);
}

void gicV2InterruptProcessed(uint32_t intrId, uint32_t sourceCPU) {
	uint32_t giccEOIR = ((sourceCPU & GICC_EOIR_CPUID_MASK) << GICC_EOIR_CPUID_BITPOS) |
						((intrId & GICC_EOIR_EOIINTID_MASK) << GICC_EOIR_EOIINTID_BITPOS);
	regWrite32(cpuInterface+GICC_EOIR, giccEOIR);
}

void gicV2InterruptEnable(uint32_t intrId) {
	uint32_t isenableWordNum = intrId/GICD_ISENABLER_NUM_BITS;
	uint32_t mask =  1 << (intrId%GICD_ISENABLER_NUM_BITS);

	regWrite32(distributor+GICD_ISENABLER+isenableWordNum, mask);
}

void gicV2InterruptDisable(uint32_t intrId) {
	uint32_t isenableWordNum = intrId/GICD_ICENABLER_NUM_BITS;
	uint32_t mask =  1 << (intrId%GICD_ICENABLER_NUM_BITS);

	regWrite32(distributor+GICD_ICENABLER+isenableWordNum, mask);
}

void gicV2sgiGenerate(uint8_t cpuTargetList, uint32_t intrId) {
	uint32_t sgir = intrId & 0xf;

	// SGI to CPU Target List
	sgir |= ((cpuTargetList) & 0xff) << 16;

	if (TZ_SECURE_SGI_IRQ == intrId)
		sgir |= 0 << 15;
	else
		sgir |= 1 << 15;

	ARCH_SPECIFIC_DMB;
	regWrite32(distributor+GICD_SGIR, sgir);

	// printf("Generate SGI %d (0x%x)\n", irq, (unsigned int)sgir);
}

uint32_t gicV2currIntr(uint32_t *srcCpu) {
	uint32_t ack = regRead32(cpuInterface+GICC_IAR);
	*srcCpu = (ack >> 10) & 0x3;
	return (ack & 0x3FF);
}
