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

#include <cstdint>

#include "config.h"
#include "arm/arm.h"
#include "kernel.h"

#include "lib_printf.h"
#include "libfdt.h"
#include "parse_utils.h"

#include "system.h"

uint32_t sRegs[V7AR_NUM_CORE_REGS*MAX_NUM_CPUS];
uint32_t sFPEXC[MAX_NUM_CPUS];
uint32_t sFPSCR[MAX_NUM_CPUS];
uint64_t sNeonRegs[V7AR_NUM_NEON64_REGS*MAX_NUM_CPUS];

uint32_t nsRegs[V7AR_NUM_CORE_REGS*MAX_NUM_CPUS];
uint32_t nsFPEXC[MAX_NUM_CPUS];
uint32_t nsFPSCR[MAX_NUM_CPUS];
uint64_t nsNeonRegs[V7AR_NUM_NEON64_REGS*MAX_NUM_CPUS];

uint32_t kernelDevTreeAddr;

extern unsigned long mon_stack_start;

void tzMonitorInit(const void *devTree, unsigned long spBase) {

    int cpu = arm::smpCpuNum();

    uint32_t *secRegs = &sRegs[cpu * V7AR_NUM_CORE_REGS];
    uint32_t *nsecRegs = &nsRegs[cpu * V7AR_NUM_CORE_REGS];

    for (int i=0; i<V7AR_NUM_CORE_REGS; i++) {
        secRegs[i] = 0xabababab;
        nsecRegs[i] = 0xcdcdcdcd;
    }

    uint64_t *secNeonRegs = &sNeonRegs[cpu * V7AR_NUM_CORE_REGS];
    uint64_t *nsecNeonRegs = &nsNeonRegs[cpu * V7AR_NUM_CORE_REGS];

    for (int i=0; i<V7AR_NUM_NEON64_REGS; i++) {
        secNeonRegs[i] = 0;
        nsecNeonRegs[i] = 0;
    }

    /* Initialize the secure config register and setup
     * access permissions for non-secure world */
    register uint32_t scr, nsacr;
    asm volatile("mrc p15, 0, %[rt], c1, c1, 0" : [rt] "=r" (scr) : :);
    scr |= (1 << 8) | (1 << 5) |(1 << 2);//(SCR_HYP_CALL_ENABLE | SCR_NS_ALLOW_DATA_ABORT_MASK | SCR_FIQ_IN_MON_MODE);
    asm volatile("mcr p15, 0, %[rt], c1, c1, 0" : : [rt] "r" (scr) :);

    asm volatile("mrc p15, 0, %[rt], c1, c1, 2" : [rt] "=r" (nsacr) : :);
    nsacr |= ( (1 << 19) | ( 1<<11 ) |  ( 1<<10 ) |  ( 1<<18 )); // RFR, SMP CP10 and CP11.
    asm volatile("mcr p15, 0, %[rt], c1, c1, 2" : : [rt] "r" (nsacr) :);

    if (cpu != 0) {

        nsecRegs[REG_R0] = 0;
        nsecRegs[REG_R1] = 0xffffffff;
        nsecRegs[REG_R2] = kernelDevTreeAddr;
        nsecRegs[REG_SP] = spBase;
        nsecRegs[REG_LR] = (unsigned long)System::cpuBootAddress[cpu];
        nsecRegs[REG_CPSR] = Mode_MON;
        nsecRegs[REG_MON_SP] = spBase;
        nsecRegs[REG_MON_LR] = (unsigned long)System::cpuBootAddress[cpu];
        nsecRegs[REG_MON_SPSR] = Mode_SVC;

        // printf("%s: CPU %d done. SP_MON 0x%lx 0x%lx\n", __PRETTY_FUNCTION__, cpu, nsecRegs[REG_MON_SP], spBase);
        return;
    }

    // BOOT CPU: setup the normal world OS
    //
    // The bootstrap code has mapped device tree memory one-to-one VA and PA.
    int rc = fdt_check_header(devTree);
    if (rc) {
        err_msg("Corrupted device tree at %p.\n", devTree);
        return;
    }


    /* Parse the 'nwos' node in the device tree and determine our range of usable memory.
     *
     * Example node:
     *
     * nwos {
     *      kernel = <0x8000>           // This property tells the location where bootloader has loaded the normal world kernel.
     *      device-tree = <0x762c000>   // Where the bootloader has loaded the normal world device tree.
     *  };
     *
     *
     */

    int nodeOffset = fdt_subnode_offset(devTree, 0, "nwos");
    if (nodeOffset < 0) {
        kernelHalt("No nwos node found in TZ device tree");
    }

    int propLen;
    const struct fdt_property *fpKernelLoc = fdt_get_property(devTree, nodeOffset, "kernel", &propLen);
    if (!fpKernelLoc) {
        kernelHalt("Normal world kernel load address not found in TZ device tree");
    }

    uint32_t kernelLoadAddr = (uint32_t)parseInt((void *)fpKernelLoc->data, propLen);

    const struct fdt_property *fpDevTreeLoc = fdt_get_property(devTree, nodeOffset, "device-tree", &propLen);
    if (!fpDevTreeLoc) {
        kernelHalt("Normal world kernel dtree address not found in TZ device tree");
    }

    kernelDevTreeAddr = (uint32_t)parseInt((void *)fpDevTreeLoc->data, propLen);

    nsecRegs[REG_R0] = 0;
    nsecRegs[REG_R1] = 0xffffffff;
    nsecRegs[REG_R2] = kernelDevTreeAddr;
    nsecRegs[REG_SP] = (uint32_t)&mon_stack_start;
    nsecRegs[REG_LR] = kernelLoadAddr;
    nsecRegs[REG_CPSR] = Mode_MON;
    nsecRegs[REG_MON_LR] = kernelLoadAddr;
    nsecRegs[REG_MON_SPSR] = Mode_SVC;
}
