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

#include "config.h"
#include "arm/arm.h"

#define ZIMAGE_MAGIC_NUMBER   0x016f2818
#define ARM_IMAGE_HEADER      0x324d5241  /* ARM image header for 32bit ("ARM\x32") */

.section ".text.bootstrap"

.arm

.global tz_init
.global tz_bootstrap
.global mon_stack_start

tz_init:

    /* The image must begin with a zImage header. This makes
     * BOLT not-care whether its booting a TZ kernel or a regular Linux kernel
     */
    .rept 8
    nop
    .endr
    b   tz_bootstrap

    .word   ARM_IMAGE_HEADER

tz_bootstrap:

    /* The bootloader has loaded the following registers:
     *   r0: 0x00000000
     *   r1: Machine ID
     *   r2: Device tree memory location
     *   r3: UART base
     */

    /*
     * Figure out where we have been loaded in memory.
     */
    ldr     r4, =tz_init   /* r4 now points to the Link address of 'tz_init' as set by the linker */
    adr     r0, tz_init    /* r0 now points to the address where the boot-loader loaded 'tz_init' */
    sub     r5, r0, r4     /* r5 is the offset between our link address and our load address.
                            * We will obtain load addresses by adding r5 to link addresses.
                            */

    /* Figure out our CPU number in the multicore complex: Read MPIDR */
    mrc     p15, 0, r4, c0, c0, 5
    and     r4, r4, #3

    /*
     * Clear the BSS section
     */
    cmp     r4, #0
    bne     stack_setup

    ldr     r8, =_bss_start
    add     r8, r5
    ldr     r9, =_bss_end
    add     r9, r5
    mov     r6, #0
2:
    str     r6, [r8], #4
    cmp     r8, r9
    blo     2b

stack_setup:
    ldr     r6, =bootstrap_stack_start
    mov     r7, #INIT_STACK_SIZE
    mul     r7, r4
    sub     r6, r7
    add     r6, r5
    mov     sp, r6

    mov     r9, r2 /* r9 is device tree address */

    mov     r0, r5
    ldr     r6, =bootstrap_main
    add     r6, r5
    blx     r6

    /* The MMU and cache are now on. We no longer need r5 based indirection for data accesses. */

    ldr     r6, =boot_mode
    ldr     r7, [r6]
    cmp     r7, #ARMV8_BOOT_MODE
    beq     skip_monitor

    cpsid   f, #Mode_MON  /* Switch to secure monitor mode */
    ldr     r6, =mon_stack_start
    mov     r7, #MON_STACK_SIZE
    mul     r7, r4
    sub     r6, r7
    mov     sp, r6
    /* Setup the exception vector base address (MVBAR) */
    ldr     r8, =mon_mode_vectors
    mcr     p15, 0, r8, c12, c0, 1

    mov     r0, r9
    mov     r1, r6
    ldr     r8, =tzMonitorInit
    blx     r8

skip_monitor:
    /*setup the fiq mode stack */
    cpsid   f, #Mode_FIQ
    ldr     r6, =fiq_stack_start
    mov     r7, #FIQ_STACK_SIZE
    mul     r7, r4
    sub     r6, r7
    mov     sp, r6

    /*setup the abt mode stack */
    cpsid   f, #Mode_ABT
    ldr     r6, =data_abt_stack_start
    mov     r8, #INIT_STACK_SIZE
    mul     r8, r4
    sub     r6, r8
    mov     sp, r6

    /*setup the svc mode stack */
    cpsid   f, #Mode_SVC

    /* Setup the exception vector base address (VBAR) */
    ldr     r8, =secure_mode_vectors
    mcr     p15, 0, r8, c12, c0, 0

    /* enable floating register point access in CPACR */
    mov     r6, #0x00f00000
    mcr     p15, 0, r6, c1, c0, 2

    /* enable SIMD instruction execution at all levels */
    vmrs r0, fpexc
    orr  r0, r0, #0x40000000
    vmsr fpexc, r0

    /* Now setup the main stack. */
    ldr     r6, =init_stack_start
    mov     r8, #INIT_STACK_SIZE
    mul     r8, r4
    sub     r6, r8
    mov     sp, r6

    mov     r0, r9
    ldr     r8, =tzKernelInit
    bx      r8

halt:
    b       halt

.section ".text"

    /*
     * Monitor mode interrupt vectors
     */
     .align 5
mon_mode_vectors:
    b       .                               /* Not defined for monitor mode */
    b       .                               /* Not defined for monitor mode */
    b       mon_smc_handler              /*  SMC call handler */
    b       .                               /* Instruction fetch abort handler: Instruction fetch aborts are not routed to monitor mode */
    b       .                               /* Data fetch abort handler: Not routed to monitor mode */
    b       .                               /* Not defined for monitor mode */
    b       .                               /* IRQ handler: Not routed to monitor mode */
    b       mon_fiq_handler


    /*
     * Secure mode vectors
     */
     .align 5
secure_mode_vectors:
    b       tz_bootstrap
    b       .
    b       tz_secure_svc_handler
    b       .
    b       tz_secure_data_abort_handler
    b       .
    b       .
    b       tz_fiq_handler


tz_secure_prefetch_abort_handler:
    b       tz_secure_prefetch_abort_handler

tz_secure_undef_handler:
    b       tz_secure_undef_handler


/* The stack during bootstrap */
.section ".data.bootstrap"
    .align 5
    .space   INIT_STACK_SIZE*MAX_NUM_CPUS
bootstrap_stack_start:

.section ".data"
    .align 5
    .space   INIT_STACK_SIZE*MAX_NUM_CPUS
init_stack_start:

.section ".data"
    .align 5
    .space   FIQ_STACK_SIZE*MAX_NUM_CPUS
fiq_stack_start:

.section ".data"
    .align 5
    .space   MON_STACK_SIZE*MAX_NUM_CPUS
mon_stack_start:

.section ".data"
    .align 5
    .space   INIT_STACK_SIZE*MAX_NUM_CPUS
data_abt_stack_start:
