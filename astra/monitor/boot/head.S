/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include <arch.h>
#include <context.h>

#include "asm_helpers.h"
#include "config.h"
#include "monitor.h"

.global mon_entry_point
.global mon_secondary_entry_point

.section .text.bootstrap

func_bootstrap mon_entry_point
#ifdef SMM64
	/*
	 * SMM64 has populated the following registers:
	 * - x0: monitor params
	 * - x1: platform params
	 * - x2: uart base in use
	 */
	mov	x20, x0
	mov	x21, x1
	mov	x22, x2
#else
	/*
	 * Bolt has populated the following registers:
	 * - x0: 0
	 * - x1: monitor params
	 * - x2: platform params
	 * - x3: uart base in use
	 */
	mov	x20, x1
	mov	x21, x2
	mov	x22, x3
#endif
	/*
	 * Disable MMU and caches
	 */
	mrs	x0, sctlr_el3
	mov	x1, #(SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	bic	x0, x0, x1
	msr	sctlr_el3, x0
	isb

	/*
	 * Invalidate caches
	 * Dcache is invalidated at all levels
	 */
	dsb	sy
	ic	iallu

	mov	x0, DCISW
	bl	dcsw_op_all

	/*
	 * Calculate offset between load address and link address
	 */
	adr	x0, mon_entry_point
	ldr	x1, =mon_entry_point
	sub	x23, x0, x1

	/*
	 * Call reset handler
	 * Clobbers: x0-x18, x19, x30
	 */
	bl	reset_handler

	/*
	 * Clear data BSS section
	 */
	adr	x0, _bss_start
	adr	x1, _bss_end
1:
	stp	xzr, xzr, [x0], #16
	cmp	x0, x1
	blo	1b

	/*
	 * Use SP_EL0 as runtime stack
	 */
	msr	spsel, #0

	/*
	 * Set bootstrap stack that uses load address (PC-relative)
	 */
	adr	x0, cpu0_stack_top
	mov	sp, x0

	/*
	 * Call bootstrap C-function
	 */
	mov	x0, x20
	mov	x1, x21
	mov	x2, x22
	mov	x3, x23
	bl	mon_bootstrap

	/*
	 * Adjust stack to use link address after MMU is enabled
	 */
	sub	sp, sp, x23

	/*
	 * Jump to common init with load address (PC-relative)
         */
	bl	mon_common_init

	/*
	 * Jump to main function with link address
	 */
	ldr	x20, =mon_main
	blr	x20

	/*
	 * Clean the .data & .bss sections to main memory. This ensures
	 * that any global data which was initialised by the primary CPU
	 * is visible to secondary CPUs before they enable their data
	 * caches and participate in coherency.
	 */
	ldr	x20, =clean_dcache_range

	ldr	x0, =_data_start
	ldr	x1, =_data_end
	sub	x1, x1, x0
	blr	x20

	ldr	x0, =_bss_start
	ldr	x1, =_bss_end
	sub	x1, x1, x0
	blr	x20

	/*
	 * Exit to lower exception level
	 */
	ldr	x20, =mon_exit
	br	x20

	/* Point of no return */
endfunc mon_entry_point

func_bootstrap mon_secondary_entry_point
	/*
	 * Disable MMU and caches
	 */
	mrs	x0, sctlr_el3
	mov	x1, #(SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	bic	x0, x0, x1
	msr	sctlr_el3, x0
	isb

	/*
	 * Invalidate caches
	 * Dcache is invalidated at L1 level
	 */
	dsb	sy
	ic	ialluis

	mov	x0, DCISW
	bl	dcsw_op_level1

	/*
	 * Calculate offset between load address and link address
	 */
	adr	x0, mon_secondary_entry_point
	ldr	x1, =mon_secondary_entry_point
	sub	x23, x0, x1

	/*
	 * Call reset handler
	 * Clobbers: x0-x18, x19, x30
	 */
	bl	reset_handler

	/*
	 * Use SP_EL0 as runtime stack
	 */
	msr	spsel, #0

	/*
	 * Set bootstrap stack that uses load address (PC-relative)
	 */
	adr	x0, cpu0_stack_top
	mrs	x1, mpidr_el1
	and	x1, x1, #MPIDR_CPU_MASK
	add	x0, x0, x1, LSL #CPU_STACK_SIZE_SHIFT
	mov	sp, x0

	/*
	 * Call secondary bootstrap C-function
	 */
	bl	mon_secondary_bootstrap

	/*
	 * Adjust stack to use link address after MMU is enabled
	 */
	sub	sp, sp, x23

	/*
	 * Jump to common init
         */
	bl	mon_common_init

	/*
	 * Jump to secondary main function with link address
	 */
	ldr	x20, =mon_secondary_main
	blr	x20

	/*
	 * Exit to lower exception level
	 */
	ldr	x20, =mon_exit
	br	x20

	/* Point of no return */
endfunc mon_secondary_entry_point

func_bootstrap mon_common_init
	/*
	 * Set the exception vectors
	 */
	ldr	x0, =mon_vectors
	msr	vbar_el3, x0
	isb

	/*
	 * Enable execute-never, stack pointer alignment checks
	 */
	mrs	x0, sctlr_el3
	mov	x1, #SCTLR_WXN_BIT
	orr	x1, x1, #SCTLR_SA_BIT
	orr	x0, x0, x1
	msr	sctlr_el3, x0
	isb

	/*
	 * Set Secure Config Register:
	 * - Early set RES1 bits in SCR_EL3;
	 * - Set SIF bit to disable instruction fetches from Non-secure memory.
	 */
	mov	x0, #(SCR_RES1_BITS | SCR_SIF_BIT)
	msr	scr_el3, x0

	/*
	 * Enable External Aborts and SError Interrupts
	 */
	msr	daifclr, #DAIF_ABT_BIT

	/*
	 * The initial state of the Architectural Feature Trap Register
	 * (CPTR_EL3) is unknown and it must be set to a known state. All
	 * feature traps are disabled. Some bits in this register are marked as
	 * reserved and should not be modified.
	 *
	 * CPTR_EL3.TCPAC: This causes a direct access to the CPACR_EL1 from EL1
	 *  or the CPTR_EL2 from EL2 to trap to EL3 unless it is trapped at EL2.
	 *
	 * CPTR_EL3.TTA: This causes access to the Trace functionality to trap
	 *  to EL3 when executed from EL0, EL1, EL2, or EL3. If system register
	 *  access to trace functionality is not supported, this bit is RES0.
	 *
	 * CPTR_EL3.TFP: This causes instructions that access the registers
	 *  associated with Floating Point and Advanced SIMD execution to trap
	 *  to EL3 when executed from any exception level, unless trapped to EL1
	 *  or EL2.
	 */
	mrs	x0, cptr_el3
	bic	w0, w0, #TCPAC_BIT
	bic	w0, w0, #TTA_BIT
	bic	w0, w0, #TFP_BIT
	msr	cptr_el3, x0

	ret
endfunc mon_common_init

.section .stacks, "aw", %nobits
.space	CPU_STACK_SIZE
cpu0_stack_top:
.space	CPU_STACK_SIZE * (MAX_NUM_CPUS - 1)
