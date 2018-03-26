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

#include "arm/arm.h"

.arm

.section ".text"

.global mon_fiq_handler

mon_fiq_handler:

    sub lr, lr, #4 /* the PC to return to after handling this FIQ */

    cpsid   if

    push {r1}
    push {r0}

    /* Read the SCR. If (SCR.NS == 0) we were in
     * secure mode before this interrupt happened.
     * Otherwise we were in normal mode
     */
    mrc p15, 0, r0, c1, c1, 0
    tst r0, #1
    bne save_ns_regs

    /* If we are here, SCR.NS==0 and hence we were already
     * in secure mode. No need to save NS mode registers
     */

    pop {r0}
    pop {r1}
    b call_tz_mon_fiq_handler

save_ns_regs:
    /* If here, we were in normal mode. Clear the NS bit in SCR */
    bic r0, #1
    mcr p15, 0, r0, c1, c1, 0

    isb

    /* Now save all the normal mode registers */

    push {r2}

    /* Figure out our CPU number in the multicore complex: Read MPIDR */
    mrc p15, 0, r0, c0, c0, 5
    and r0, r0, #3

    ldr r1, =nsRegs;
    mov r2, #V7AR_NUM_CORE_REGS*4
    mul r0, r2
    add r1, r0

    pop {r2}
    pop {r0}

    str r0, [r1, #(REG_R0*4)]

    str r2, [r1, #(REG_R2*4)]
    mov r2, r1
    pop {r1}
    str r1, [r2, #(REG_R1*4)]
    mov r1, r2

    str r3, [r1, #(REG_R3*4)]
    str r4, [r1, #(REG_R4*4)]
    str r5, [r1, #(REG_R5*4)]
    str r6, [r1, #(REG_R6*4)]
    str r7, [r1, #(REG_R7*4)]
    str r8, [r1, #(REG_R8*4)]
    str r9, [r1, #(REG_R9*4)]
    str r10, [r1, #(REG_R10*4)]
    str r11, [r1, #(REG_R11*4)]
    str r12, [r1, #(REG_R12*4)]

    // str  sp, [r1, #(REG_SP*4)]
    // str  lr, [r1, #(REG_LR*4)]
    mrs r2, sp_usr
    str r2, [r1, #(REG_SP*4)]
    mrs r2, lr_usr
    str r2, [r1, #(REG_LR*4)]
    mrs r2, cpsr
    str r2, [r1, #(REG_CPSR*4)]

    mrs r2, sp_svc
    str r2, [r1, #(REG_SVC_SP*4)]
    mrs r2, lr_svc
    str r2, [r1, #(REG_SVC_LR*4)]
    mrs r2, spsr_svc
    str r2, [r1, #(REG_SVC_SPSR*4)]

    mrs r2, sp_irq
    str r2, [r1, #(REG_IRQ_SP*4)]
    mrs r2, lr_irq
    str r2, [r1, #(REG_IRQ_LR*4)]
    mrs r2, spsr_irq
    str r2, [r1, #(REG_IRQ_SPSR*4)]

    mrs r2, sp_abt
    str r2, [r1, #(REG_ABT_SP*4)]
    mrs r2, lr_abt
    str r2, [r1, #(REG_ABT_LR*4)]
    mrs r2, spsr_abt
    str r2, [r1, #(REG_ABT_SPSR*4)]

    mrs r2, sp_und
    str r2, [r1, #(REG_UND_SP*4)]
    mrs r2, lr_und
    str r2, [r1, #(REG_UND_LR*4)]
    mrs r2, spsr_und
    str r2, [r1, #(REG_UND_SPSR*4)]

    mrs r2, sp_hyp
    str r2, [r1, #(REG_HYP_SP*4)]
    mrs r2, elr_hyp
    str r2, [r1, #(REG_HYP_LR*4)]
    mrs r2, spsr_hyp
    str r2, [r1, #(REG_HYP_SPSR*4)]

    mrs r2, sp_mon
    str r2, [r1, #(REG_MON_SP*4)]
    mrs r2, lr_mon
    str r2, [r1, #(REG_MON_LR*4)]
    mrs r2, spsr_mon
    str r2, [r1, #(REG_MON_SPSR*4)]

    /* Save the FPU/Neon regs */

    /* cpuid */
    mrc p15, 0, r1, c0, c0, 5
    and r1, r1, #3

    ldr  r0, =nsFPEXC
    mov  r2, #4
    mul  r1, r2
    add  r0, r1

    vmrs r1, FPEXC
    str  r1, [r0]

    mov  r1, #0x40000000
    vmsr FPEXC, r1

    /* cpuid */
    mrc p15, 0, r1, c0, c0, 5
    and r1, r1, #3

    ldr  r0, =nsFPSCR
    mov  r2, #4
    mul  r1, r2
    add  r0, r1

    vmrs r1, FPSCR
    str  r1, [r0]

    /* cpuid */
    mrc p15, 0, r1, c0, c0, 5
    and r1, r1, #3

    ldr  r0, =nsNeonRegs
    mov  r2, #256
    mul  r1, r2
    add  r0, r1

    vstmia r0!, {d0-d15}
    vstmia r0!, {d16-d31}

    /* Now load all the core registers
     * from the secure side
     */
load_s_regs:
    ldr r0, =sRegs

    /* Figure out our CPU number in the multicore complex: Read MPIDR */
    mrc p15, 0, r1, c0, c0, 5
    and r1, r1, #3

    mov r2, #V7AR_NUM_CORE_REGS*4
    mul r1, r2
    add r0, r1

    ldr r1, [r0, #(REG_R1*4)]
    ldr r2, [r0, #(REG_R2*4)]
    ldr r3, [r0, #(REG_R3*4)]
    ldr r4, [r0, #(REG_R4*4)]
    ldr r5, [r0, #(REG_R5*4)]
    ldr r6, [r0, #(REG_R6*4)]
    ldr r7, [r0, #(REG_R7*4)]
    ldr r8, [r0, #(REG_R8*4)]
    ldr r9, [r0, #(REG_R9*4)]
    ldr r10, [r0, #(REG_R10*4)]
    ldr r11, [r0, #(REG_R11*4)]
    ldr r12, [r0, #(REG_R12*4)]

    push {r1}

    // ldr sp, [r0, #(REG_SP*4)]
    // ldr lr, [r0, #(REG_LR*4)]
    ldr r1, [r0, #(REG_SP*4)]
    msr sp_usr, r1
    ldr r1, [r0, #(REG_LR*4)]
    msr lr_usr, r1
    ldr r1, [r0, #(REG_CPSR*4)]
    msr cpsr, r1

    ldr r1, [r0, #(REG_SVC_SP*4)]
    msr sp_svc, r1
    ldr r1, [r0, #(REG_SVC_LR*4)]
    msr lr_svc, r1
    ldr r1, [r0, #(REG_SVC_SPSR*4)]
    msr spsr_svc, r1

    ldr r1, [r0, #(REG_IRQ_SP*4)]
    msr sp_irq, r1
    ldr r1, [r0, #(REG_IRQ_LR*4)]
    msr lr_irq, r1
    ldr r1, [r0, #(REG_IRQ_SPSR*4)]
    msr spsr_irq, r1

    ldr r1, [r0, #(REG_ABT_SP*4)]
    msr sp_abt, r1
    ldr r1, [r0, #(REG_ABT_LR*4)]
    msr lr_abt, r1
    ldr r1, [r0, #(REG_ABT_SPSR*4)]
    msr spsr_abt, r1

    ldr r1, [r0, #(REG_UND_SP*4)]
    msr sp_und, r1
    ldr r1, [r0, #(REG_UND_LR*4)]
    msr lr_und, r1
    ldr r1, [r0, #(REG_UND_SPSR*4)]
    msr spsr_und, r1

    ldr r1, [r0, #(REG_HYP_SP*4)]
    msr sp_hyp, r1
    ldr r1, [r0, #(REG_HYP_LR*4)]
    msr elr_hyp, r1
    ldr r1, [r0, #(REG_HYP_SPSR*4)]
    msr spsr_hyp, r1

    // ldr r1, [r0, #(REG_MON_SP*4)]
    // msr sp_mon, r1
    ldr r1, [r0, #(REG_MON_LR*4)]
    msr lr_mon, r1
    ldr r1, [r0, #(REG_MON_SPSR*4)]
    msr spsr_mon, r1

    pop {r1}

    ldr r0, [r0]

    /* Load FPU/Neon registers */
    push {r0}
    push {r1}
    push {r2}

    mov r0, #0x40000000
    vmsr FPEXC, r0

    /* cpuid */
    mrc p15, 0, r1, c0, c0, 5
    and r1, r1, #3

    ldr r0, =sNeonRegs
    mov r2, #256
    mul r1, r2
    add r0, r1

    vldmia r0!, {d0-d15}
    vldmia r0!, {d16-d31}

    /* cpuid */
    mrc p15, 0, r1, c0, c0, 5
    and r1, r1, #3

    ldr r0, =sFPSCR
    mov r2, #4
    mul r1, r2
    add r0, r1

    ldr r0, [r0]
    vmsr FPSCR, r0

    /* cpuid */
    mrc p15, 0, r1, c0, c0, 5
    and r1, r1, #3

    ldr r0, =sFPEXC
    mov r2, #4
    mul r1, r2
    add r0, r1

    ldr r0, [r0]
    vmsr FPEXC, r0

    pop {r2}
    pop {r1}
    pop {r0}

call_tz_mon_fiq_handler:
    b   tz_mon_fiq_handler
