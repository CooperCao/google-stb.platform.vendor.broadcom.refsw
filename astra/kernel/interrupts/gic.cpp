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

#include "arm/arm.h"
#include "arm/gic.h"

#include "pgtable.h"

#include "lib_printf.h"

void *GIC::distributorBase;
void *GIC::cpuInterfaceBase;
spinlock_t GIC::lock;

void GIC::init(void *devTree) {

    UNUSED(devTree);

    if (arm::smpCpuNum() != 0) {
        secondaryCpuInit();
        return;
    }

    unsigned long *gicMemBase, *gicMemEnd;
    PageTable *kernPageTable = PageTable::kernelPageTable();

    // Read the GIC base address from CBAR register
    asm volatile("mrc p15, 4, %[rt], c15, c0, 0" : [rt] "=r" (gicMemBase) : :);
    gicMemBase = (unsigned long *)((unsigned long)gicMemBase & 0xffff0000);

    gicMemEnd = gicMemBase + GIC_MEMMAP_SIZE;
    kernPageTable->mapPageRange(gicMemBase, gicMemEnd, gicMemBase, MAIR_DEVICE, MEMORY_ACCESS_RW_KERNEL);

    distributorBase = (uint8_t *)gicMemBase +  ARM_CORTEX_A15_GICD_BASE;
    cpuInterfaceBase = (uint8_t *)gicMemBase + ARM_CORTEX_A15_GICC_BASE;

    // Disable the distributor
    uint32_t *gicDistributor = (uint32_t *)distributorBase;
    gicDistributor[GICD_CTL] = 0;

    //Assign interrupts to groups:
    //      Group 0: All secure interrupts meant for us (the TZ OS ).
    //      Group 1: All normal world interrupts.
    //
    // The TZ OS is only interested in one PPI interrupts: Secure clock.
    uint32_t ppiGroups = ~( 1 << ARM_CORTEX_A15_SECURE_TIMER_IRQ);
    gicDistributor[GICD_IGROUP] = ppiGroups;
    int numIntrLines = (gicDistributor[GICD_TYPE] & 0xf) + 1;
    for (int i=1; i<numIntrLines; i++) {
        gicDistributor[GICD_IGROUP + i] = 0xffffffff;
    }

    // Enable the Secure clock and secure SGI interrupt
    gicDistributor[GICD_ISENABLE] = (1 << ARM_CORTEX_A15_SECURE_TIMER_IRQ);

    // Reset all SPIs. The normal world OS will enable them
    for (int i=32; i<numIntrLines; i+=16)
        gicDistributor[GICD_ICFG + (i/16)] = 0x00000000;
    for (int i=32; i<numIntrLines; i+= 32)
        gicDistributor[GICD_ICENABLE + (i/32)] = 0xFFFFFFFF;

    // Set default priorities for all PPIs
    for (int i=0; i<32; i+=4)
        gicDistributor[GICD_IPRIORITY + (i/4)] = 0xabababab;

    // Enable the distributor: Both groups
    gicDistributor[GICD_CTL] = 0x3;

    // Now initialize the CPU interface:
    // Configure the CPU interface priority
    uint32_t *gicCpuIntrf = (uint32_t *)cpuInterfaceBase;
    gicCpuIntrf[GICC_PMR] = 0xf8;
    gicCpuIntrf[GICC_BPR] = 0x0;

    // Enable the CPU interface:
    //      EOIModeS = 0: Writing GICC_EOIR causes both priority drop and deactivation.
    //      CBPR = 1    : Use GICC_BPR for prioritization of both secure and non-secure interrupts.
    //      FIQEn = 1   : Group 0 interrupts cause FIQ interrupt on CPU
    //      EnableGrp1 = 1
    //      EnableGrp0 = 1
    gicCpuIntrf[GICC_CTLR] = 0x1B;

    spinlock_init("gic.lock", &lock);

    printf("gic primary init done\n");
}

void GIC::secondaryCpuInit() {

    unsigned long *gicMemBase;

    // Read the GIC base address from CBAR register
    asm volatile("mrc p15, 4, %[rt], c15, c0, 0" : [rt] "=r" (gicMemBase) : :);
    gicMemBase = (unsigned long *)((unsigned long)gicMemBase & 0xffff0000);

    uint8_t *distributorBase = (uint8_t *)gicMemBase +  ARM_CORTEX_A15_GICD_BASE;
    uint8_t *cpuInterfaceBase = (uint8_t *)gicMemBase + ARM_CORTEX_A15_GICC_BASE;

    uint32_t *gicDistributor = (uint32_t *)distributorBase;
    uint32_t ppiGroups = ~(1 << ARM_CORTEX_A15_SECURE_TIMER_IRQ);
    gicDistributor[GICD_IGROUP] = ppiGroups;
    int numIntrLines = (gicDistributor[GICD_TYPE] & 0xf) + 1;
    for (int i=1; i<numIntrLines; i++) {
        gicDistributor[GICD_IGROUP + i] = 0xffffffff;
    }

    // Enable the Secure clock and secure SGI interrupt
    gicDistributor[GICD_ISENABLE] = (1 << ARM_CORTEX_A15_SECURE_TIMER_IRQ);

    // Set default priorities for all PPIs
    for (int i=0; i<32; i+=4)
        gicDistributor[GICD_IPRIORITY + (i/4)] = 0xabababab;

    // Enable the CPU interface
    uint32_t *gicCpuIntrf = (uint32_t *)cpuInterfaceBase;
    gicCpuIntrf[GICC_PMR] = 0xff;
    gicCpuIntrf[GICC_BPR] = 0x0;

    // Enable the CPU interface:
    //      EOIModeS = 0: Writing GICC_EOIR causes both priority drop and deactivation.
    //      CBPR = 1    : Use GICC_BPR for prioritization of both secure and non-secure interrupts.
    //      FIQEn = 1   : Group 0 interrupts cause FIQ interrupt on CPU
    //      EnableGrp1 = 1
    //      EnableGrp0 = 1
    gicCpuIntrf[GICC_CTLR] = 0x1B;

    //printf("%s: gidDistributor %p gicCpuIntrf %p\n", __PRETTY_FUNCTION__, gicDistributor, gicCpuIntrf);
}

void GIC::intrDisable(int irq) {
    SpinLocker locker(&lock);

    if ((irq != ARM_CORTEX_A15_SECURE_TIMER_IRQ) && (irq != TZ_SGI_IRQ)) {
        err_msg("TZ: Attempt to disable a normal world interrupt disallowed\n");
        return;
    }

    uint32_t *gicDistributor = (uint32_t *)distributorBase;
    uint32_t wordNum = irq / 32;
    uint32_t bitNum = irq % 32;

    uint32_t val = (uint32_t)( 1 << bitNum);
    uint32_t enabled = gicDistributor[GICD_ISENABLE + wordNum];
    enabled &= ~val;

    gicDistributor[GICD_ICENABLE + wordNum] |= val;
    gicDistributor[GICD_ISENABLE + wordNum] = enabled;
}

void GIC::intrEnable(int irq) {
    SpinLocker locker(&lock);

    if ((irq != ARM_CORTEX_A15_SECURE_TIMER_IRQ) && (irq != TZ_SGI_IRQ)) {
        err_msg("TZ: Attempt to enable a normal world interrupt disallowed\n");
        return;
    }

    uint32_t *gicDistributor = (uint32_t *)distributorBase;
    uint32_t wordNum = irq / 32;
    uint32_t bitNum = irq % 32;

    gicDistributor[GICD_ICENABLE + wordNum] |= ( 1 << bitNum);
}

void GIC::sgiGenerate(int irq) {
    uint32_t sgir = irq & 0xf;

    // SGI to CPU 0 only
    sgir |= ((1 << 0) & 0xff) << 16;
    sgir |= 1 << 15;

    asm volatile ("dmb\r\n":::);
    uint32_t *gicDistributor = (uint32_t *)distributorBase;
    gicDistributor[GICD_SGI] = sgir;

    // printf("Generate SGI %d (0x%x)\n", irq, (unsigned int)sgir);
}

int GIC::currIntr(int *srcCpu) {
    uint32_t *gicCpuIntrf = (uint32_t *)cpuInterfaceBase;
    uint32_t ack = gicCpuIntrf[GICC_IAR];
    *srcCpu = (ack >> 10) & 0x3;
    return (ack & 0x3FF);
}

void GIC::endIntr(int irq) {
    uint32_t *gicCpuIntrf = (uint32_t *)cpuInterfaceBase;
    gicCpuIntrf[GICC_EOIR] = irq & 0x3ff;
}
