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

#include <arch.h>
#include <context.h>
#include <smcc.h>

#include "asm_helpers.h"
#include "monitor.h"
#include "service.h"

.global mon_vectors
.global mon_exit

/* -----------------------------------------------------
 * This macro handles and dispatches sync exceptions.
 * -----------------------------------------------------
 */
.macro	handle_sync_exception
	cmp	w0, #0
	b.eq	mon_entry_point

	/* Enable the SError interrupt */
	msr	daifclr, #DAIF_ABT_BIT

	/* Secure timer access function */
	tbnz	w0, #FUNCID_MBZ_SHIFT, stimer_access

	/* Save LR(x30) which is used */
	str	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]

	/* Check SMC coming from 32-bit or 64-bit */
	mrs	x30, esr_el3
	ubfx	x30, x30, #ESR_EC_SHIFT, #ESR_EC_LENGTH

	cmp	x30, #EC_AARCH32_SMC
	b.eq	smc_handler32

	cmp	x30, #EC_AARCH64_SMC
	b.eq	smc_handler64

	/* Handle any other sync exceptions (TBD) */
	bl	report_unhandled_exception
.endm

/* -----------------------------------------------------
 * This macro handles FIQ or IRQ interrupts
 * ---------------------------------------------------------------------
 */
.macro	handle_intr_exception label
	/* Enable the SError interrupt */
	msr	daifclr, #DAIF_ABT_BIT

	/* Save all general purpose registers, including LR(x30) and SP */
	str	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	bl	gpregs_context_save

	/* Save the SPSR_EL3, ELR_EL3, & SCR_EL3 just in case */
	mrs	x16, spsr_el3
	mrs	x17, elr_el3
	mrs	x18, scr_el3
	stp	x16, x17, [sp, #CTX_EL3STATE_OFFSET + CTX_SPSR_EL3]
	str	x18, [sp, #CTX_EL3STATE_OFFSET + CTX_SCR_EL3]

	/* Prepare to call the interrupt processing function
	 * - x0: interrupt ID (not used)
	 * - x1: pointer to the context
	 * - x2: flags
	 */
	mov	x0, xzr
	mov	x1, sp

	/* Copy scr_el3.NS bit to flags to indicate caller's security */
	mov	x2, #0
	bfi	x2, x18, #0, #1

	/* Restore sp_el0 with saved runtime stack and switch to it */
	ldr	x17, [sp, #CTX_EL3STATE_OFFSET + CTX_RUNTIME_SP]
	msr	spsel, #0
	mov	sp, x17

	/* Call the interrupt processing function */
	bl	interrupt_proc
	b	mon_exit
.endm

#ifdef SMM64
.section .vectors, "ax"
	b	mon_entry_point
#endif

/* -----------------------------------------------------
 * Monitor exception vectors
 * -----------------------------------------------------
 */
vector_base mon_vectors

/* -----------------------------------------------------
 * Current EL with sp_el0 : 0x0 - 0x200
 * -----------------------------------------------------
 */
vector_entry sync_sp_el0
	bl	report_unhandled_exception
	assert_vector_size sync_sp_el0

vector_entry irq_sp_el0
	bl	report_unhandled_exception
	assert_vector_size irq_sp_el0

vector_entry fiq_sp_el0
	bl	report_unhandled_exception
	assert_vector_size fiq_sp_el0

vector_entry serror_sp_el0
	bl	report_unhandled_exception
	assert_vector_size serror_sp_el0

/* -----------------------------------------------------
 * Current EL with sp_elx : 0x200 - 0x400
 * -----------------------------------------------------
 */
vector_entry sync_sp_elx
	bl	report_unhandled_exception
	assert_vector_size sync_sp_elx

vector_entry irq_sp_elx
	bl	report_unhandled_exception
	assert_vector_size irq_sp_elx

vector_entry fiq_sp_elx
	bl	report_unhandled_exception
	assert_vector_size fiq_sp_elx

vector_entry serror_sp_elx
	bl	report_unhandled_exception
	assert_vector_size serror_sp_elx

/* -----------------------------------------------------
 * Lower EL using AArch64 : 0x400 - 0x600
 * -----------------------------------------------------
 */
vector_entry sync_aarch64
	handle_sync_exception
	assert_vector_size sync_aarch64

vector_entry irq_aarch64
	handle_intr_exception irq_aarch64
	assert_vector_size irq_aarch64

vector_entry fiq_aarch64
	handle_intr_exception fiq_aarch64
	assert_vector_size fiq_aarch64

vector_entry serror_aarch64
	bl	report_unhandled_exception
	assert_vector_size serror_aarch64

/* -----------------------------------------------------
 * Lower EL using AArch32 : 0x600 - 0x800
 * -----------------------------------------------------
 */
vector_entry sync_aarch32
	handle_sync_exception
	assert_vector_size sync_aarch32

vector_entry irq_aarch32
	handle_intr_exception irq_aarch32
	assert_vector_size irq_aarch32

vector_entry fiq_aarch32
	handle_intr_exception fiq_aarch32
	assert_vector_size fiq_aarch32

vector_entry serror_aarch32
	bl	report_unhandled_exception
	assert_vector_size serror_aarch32

vector_end mon_vectors

/* -----------------------------------------------------
 * Report unhandled exception (TBD)
 * -----------------------------------------------------
 */
func_vectors report_unhandled_exception
	b	.
endfunc report_unhandled_exception

/* -----------------------------------------------------
 * This function handles secure monitor calls.
 *
 * Depending upon the execution state from where the
 * SMC has been invoked, it frees some general purpose
 * registers to perform the remaining tasks. They
 * involve finding the runtime service handler that is
 * the target of the SMC & switching to runtime stacks
 * (SP_EL0) before calling the handler.
 *
 * Note that x30 has been explicitly saved and can be
 * used here
 * -----------------------------------------------------
 */
func_vectors smc_handler
smc_handler32:
	/* Check whether aarch32 issued an SMC64 */
	tbnz	x0, #FUNCID_CC_SHIFT, smc_prohibited

	/* Save x8-x18 which are NOT saved in aarch64 */
	stp	x8, x9, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X8]
	stp	x10, x11, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X10]
	stp	x12, x13, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X12]
	stp	x14, x15, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X14]
	stp	x16, x17, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X16]

	/* x0-x7, x18-x29, sp_el0 are saved below */

smc_handler64:
	/* Save argument registers x0-x7 */
	stp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	stp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	stp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]
	stp	x6, x7, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X6]

	/* Save callee-saved registers x19-x29 */
	stp	x18, x19, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X18]
	stp	x20, x21, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X20]
	stp	x22, x23, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X22]
	stp	x24, x25, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X24]
	stp	x26, x27, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X26]
	stp	x28, x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X28]

	/* Save sp_el0 since it will be over written */
	mrs	x15, sp_el0
	str	x15, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_SP_EL0]

	/* Get the unique owning entity number */
	ubfx	x15, x0, #FUNCID_OEN_SHIFT, #FUNCID_OEN_WIDTH

	/* Lookup descriptor index from the table */
	adr	x14, service_oen_to_desc_idx
	ldrb	w13, [x14, x15]

	/* Check bit 7 index for a valid descriptor index */
	tbnz	w13, 7, smc_unknown

	/* Get descriptor using the index */
	adr	x14, (_service_descs_start + SERVICE_DESC_PROC_OFFSET)
	lsl	w15, w13, #SERVICE_DESC_SIZE_LOG2
	ldr	x13, [x14, w15, uxtw]

	/* Save the SPSR_EL3, ELR_EL3, & SCR_EL3 just in case */
	mrs	x16, spsr_el3
	mrs	x17, elr_el3
	mrs	x18, scr_el3
	stp	x16, x17, [sp, #CTX_EL3STATE_OFFSET + CTX_SPSR_EL3]
	str	x18, [sp, #CTX_EL3STATE_OFFSET + CTX_SCR_EL3]

	/* Prepare to call the service processing function
	 * - x0: function ID
	 * - x1: pointer to the context (arguments and returns)
	 * - x2: flags
	 */
	mov	x1, sp

	/* Copy scr_el3.NS bit to flags to indicate caller's security */
	mov	x2, #0
	bfi	x2, x18, #0, #1

	/* Restore sp_el0 with saved runtime stack and switch to it */
	ldr	x17, [sp, #CTX_EL3STATE_OFFSET + CTX_RUNTIME_SP]
	msr	spsel, #0
	mov	sp, x17

	/* Call the service processing function */
	blr	x13
	b	mon_exit

smc_unknown:
	/* Only used x13-x15 */
	ldr	x13, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X13]
	ldp	x14, x15, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X14]

	/* Fall through */

smc_prohibited:
	ldr	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	mov	w0, #SMC_UNK
	eret
endfunc smc_handler

/* -----------------------------------------------------
 * The common monitor exit function.
 * It is assumed that the SP_EL3 is pointing to the
 * next context that holds all the saved registers.
 *
 * Inputs:
 * x0: flags indicating which registers to be restored
 * -----------------------------------------------------
 */
func_vectors mon_exit
	/* Save the current sp_el0 and switch to sp_el3 */
	mov	x17, sp
	msr	spsel, #1
	str	x17, [sp, #CTX_EL3STATE_OFFSET + CTX_RUNTIME_SP]

	/* Restore spsr_el3, elr_el3 and scr_el3 prior to eret */
	ldr	x18, [sp, #CTX_EL3STATE_OFFSET + CTX_SCR_EL3]
	ldp	x16, x17, [sp, #CTX_EL3STATE_OFFSET + CTX_SPSR_EL3]
	msr	scr_el3, x18
	msr	spsr_el3, x16
	msr	elr_el3, x17

	/* Restore saved general purpose registers and return */
	b	gpregs_context_restore_eret
endfunc mon_exit

func_vectors stimer_access
	/* Check bit for function call */
	ubfx	x1, x0, #FUNCID_MBZ_SHIFT, #FUNCID_MBZ_WIDTH
	tbnz	x1, 1, set_cntps_cval
	tbnz	x1, 2, get_cntps_cval
	tbnz	x1, 3, set_cntps_ctl
	tbnz	x1, 4, get_cntps_ctl
	eret
set_cntps_cval:
	orr	x2, x2, x3, lsl 32
	msr	cntps_cval_el1, x2
	eret
get_cntps_cval:
	mrs	x2, cntps_cval_el1
	asr	x3, x2, 32
	eret
set_cntps_ctl:
	msr	cntps_ctl_el1, x2
	eret
get_cntps_ctl:
	mrs	x2, cntps_ctl_el1
	eret
endfunc stimer_access
