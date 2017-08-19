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
#include "plat_config.h"
#include "arm.h"

.global tz_init
.global tz_bootstrap

.section ".text.bootstrap"

tz_init:
tz_bootstrap:
	// What EL are we in ?
    mrs x1, CurrentEL
    and x1, x1, #(0xC)
    cmp x1, #0xC //EL3
    beq el3_start

    cmp x1, #0x8 //EL2
    beq el2_start

	cmp x1, #0x4 //EL1
    beq el1_start

el0_start:
	// EL0 boot cannot work !
el3_start:
	// EL3 boot not supported
	wfi
	b el3_start

el2_start:
	// We are EL2 (hypervisor) mode and need to hand-off to a 64-bit OS running in EL1 mode.
	// 		Do not trap anything in hypervisor.
	//		Setup EL1 for AARCH64
	mov	x6, #((1 << 31)| ( 1 << 32) | ( 1 << 33))
	msr	HCR_EL2, x6


	// VTTBR_EL2 needs to be initialized with a zero VMID. Even in the
	// absence of a hypervisor, the VMID still tags cache lines.
	msr VTTBR_EL2, xzr

	// Setup VMPIDR_EL2 and VPIDR_EL2  to match the MPIDR/MIDR respectively.
	// This ensures that MPIDI/MIDR read from EL1 does not return uninitialized values
	mrs x7, MPIDR_EL1
	msr VMPIDR_EL2, x7
	mrs x7, MIDR_EL1
	msr VPIDR_EL2, x7

	// Do not trap any floating point operations in EL2 mode.
	msr	CPTR_EL2, xzr
	mov x7, #((1 << 0) | \
              (1 << 1) | \
              (1 << 4) | \
              (1 << 5) | \
              (1 << 6))
    msr ACTLR_EL2, x7

    // Finally drop to EL1 in the OS entry point:
    //		Disable all interrupts in EL1.
    adr x1, el1_start
	msr ELR_EL2, x1
	mov x1, #0
	mov x2, #0
	mov x3, #0

	mov x7, #(0b0101 | \
              (1 << 6)  | \
              (1 << 7)  | \
              (1 << 8))
    msr	SPSR_EL2, x7

    isb
    eret

el1_start:
	// This module is intended to be position independent:
	//		The linker script links all kernel symbols starting at
	//		_system_link_base. The OS loader however is free to load
	//		the OS anywhere in system memory. Use the MMU to remap the
	//		load addresses to the link addresses.
	//
	//		Before the MMU is enabled, all symbols refer to link addresses.
	//		Convert them to load addresses by adding the difference between
	//		_system_link_base and the address at which this module was loaded.

	adr	x1, tz_init		// This is the load address of the init symbol.
	ldr x2, =tz_init		// This is the link address of the init symbol.
	sub	x3, x1, x2		// From this point forwards until the MMU is enabled,
						// all symbol accesses must be offset with x3.

	// Carve out a stack from the init_stacks region for this CPU core
	ldr	x5, =bootstrap_stacks_start
	add	x5, x5, x3
	add x5, x5, #(INIT_STACK_SIZE-16)
	mov	sp, x5

	mrs     x5, mpidr_el1
	and     x5, x5, #0x3
	cmp     x5, #0
	bne     2f

	ldr	x5, =_bss_start
	add	x5, x5, x3
	ldr	x6, =_bss_end
	add x6, x6, x3
1:
	cmp	x5, x6
	beq 2f

	str xzr, [x5], #8
	ble 1b

2:
	// Enable floating point
	mov x6, #(3 << 20)
	msr CPACR_EL1, x6

	// Setup 64bit exception vectors
	ldr x1, =el1Vectors
	msr VBAR_EL1, x1

	// Enable SError but leave interrupts masked
	mov x1,  #((1 << 7) | ( 1 << 6))
	msr DAIF, x1

	// Call the C function to setup the MMU
	stp	x0, x1, [sp, #-16]!
	stp	x2, x3, [sp, #-16]!
	mov	x1, x3
	ldr	x2, =bootstrap
	add	x2, x2, x3
	blr	x2
	ldp x2, x3, [sp], #16
	ldp x0, x1, [sp], #16

	ldr	x5, =init_stacks_start
	add	x5, x5, #(INIT_STACK_SIZE-16)
	mov	sp, x5

	mov	x1, x3
	ldr x2, =tzKernelInit
	br	x2

aarch64InitSecondaryCpu:
	// What EL are we in ?
    mrs x1, CurrentEL
    and x1, x1, #(0xC)
    cmp x1, #0xC //EL3
    beq el3_sec_start

    cmp x1, #0x8 //EL2
    beq el2_sec_start

	cmp x1, #0x4 //EL1
    beq el1_sec_start

el0_sec_start:
	// EL0 boot cannot work !
el3_sec_start:
	// EL3 boot not supported
	wfi
	b el3_start

el2_sec_start:
	// SPF has put the stack physical address in X0
	// Do not change X0.

	// We are EL2 (hypervisor) mode and need to hand-off to a 64-bit OS running in EL1 mode.
	// 		Do not trap anything in hypervisor.
	//		Setup EL1 for AARCH64
	mov	x6, #((1 << 31)| ( 1 << 32) | ( 1 << 33))
	msr	HCR_EL2, x6


	// VTTBR_EL2 needs to be initialized with a zero VMID. Even in the
	// absence of a hypervisor, the VMID still tags cache lines.
	msr VTTBR_EL2, xzr

	// Setup VMPIDR_EL2 and VPIDR_EL2  to match the MPIDR/MIDR respectively.
	// This ensures that MPIDI/MIDR read from EL1 does not return uninitialized values
	mrs x7, MPIDR_EL1
	msr VMPIDR_EL2, x7
	mrs x7, MIDR_EL1
	msr VPIDR_EL2, x7

	// Do not trap any floating point operations in EL2 mode.
	msr	CPTR_EL2, xzr
	mov x7, #((1 << 0) | \
              (1 << 1) | \
              (1 << 4) | \
              (1 << 5) | \
              (1 << 6))
    msr ACTLR_EL2, x7

    // Finally drop to EL1 in the OS entry point:
    //		Disable all interrupts in EL1.
    adr x1, el1_sec_start
	msr ELR_EL2, x1
	mov x1, #0
	mov x2, #0
	mov x3, #0

	mov x7, #(0b0101 | \
              (1 << 6)  | \
              (1 << 7)  | \
              (1 << 8))
    msr	SPSR_EL2, x7

    isb
    eret

el1_sec_start:

	// SPF has put the stack physical address in X0 ?
	mov sp, x0

	adr	x2, bootstrapSecondary
	blr	x2

	// switch stack to VA
	ldr x1, =ARCH_KERNEL_BASE
	add sp, sp, x1

	// Setup the exception handlers
	ldr x1, =el1Vectors
	msr VBAR_EL1, x1

	// Enable SError but leave interrupts masked
	mov x1,  #((1 << 7) | ( 1 << 6))
	msr DAIF, x1

	ldr x2, =tzKernelSecondary
	br	x2

.section ".data.bootstrap"
	.align 5
bootstrap_stacks_start:
	.space INIT_STACK_SIZE

.section ".data"
	.align 5
init_stacks_start:
	.space INIT_STACK_SIZE
