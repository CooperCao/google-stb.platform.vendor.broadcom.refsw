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
#include "arm.h"

.global tz_secure_svc_handler

.section ".text"
#if 0
tz_secure_svc_handler:

	msr		daifset, #(DAIFBIT_FIQ | DAIFBIT_IRQ)
	stp		x0, x1, [sp, #-16]!
	stp		x2, x3, [sp, #-16]!
	stp		x8, x30, [sp, #-16]!        /* aarch64 EABI syscall number in x8 */
	mov		x0, #0
	mrs		x1, SPSR_EL1
	and     x1, x1, #0x1f
	bl	 	excYieldCurrentTask
	/* x0 now points to the array where we should save register context */


	/* Pop all registers except x0 */
	ldp		x8, x30, [sp], #16
	ldp		x2, x3, [sp], #16
    ldr     x1, [sp, #8]

	/*
	 * Save all the core regsiters
	 */
	str 	x1, [x0, #8]
	str 	x2, [x0, #16]
	str 	x3, [x0, #24]
	str 	x4, [x0, #32]
	str 	x5, [x0, #40]
	str 	x6, [x0, #48]
	str 	x7, [x0, #56]
	str 	x8, [x0, #64]
	str 	x9, [x0, #72]
	str 	x10, [x0, #80]
	str 	x11, [x0, #88]
	str 	x12, [x0, #96]

	mov		x1, sp
	add 	x1, x1, #8
	str		x1, [x0, #104]
	str 	x30, [x0, #112]

    mrs     x1, SPSR_EL1
	str		x1, [x0, #120]
    mrs     x1, SP_EL0
	str		x1, [x0, #128]
    mov    x1, x14
	str 	x1, [x0, #136]
	mov		x1, x0
    ldr     x0, [sp]
    add     sp, sp, #16
	str		x0, [x1]
	bl		svcService

	b		schedule
#else
tz_secure_svc_handler:

	msr		daifset, #(DAIFBIT_FIQ | DAIFBIT_IRQ)

	stp x0, x1, [sp, #-16]!
	stp x2, x3, [sp, #-16]!
	stp x4, x5, [sp, #-16]!
	stp x6, x7, [sp, #-16]!
	stp x8, x9, [sp, #-16]!
	stp x10, x11, [sp, #-16]!
	stp x12, x13, [sp, #-16]!
	stp x14, x15, [sp, #-16]!
	stp x16, x17, [sp, #-16]!
	stp x18, x19, [sp, #-16]!
	stp x30, xzr, [sp, #-16]!

	mov		x0, #0
	mrs		x1, SPSR_EL1
	and     x1, x1, #0x1f
	bl	 	excYieldCurrentTask
	/* x0 now points to the array where we should save register context */


/* Pop all registers except x0 */
	ldp x30, xzr, [sp], #16
	ldp x18, x19, [sp], #16
	ldp x16, x17, [sp], #16
	ldp x14, x15, [sp],#16
	ldp x12, x13, [sp], #16
	ldp x10, x11, [sp],#16
	ldp x8, x9, [sp], #16
	ldp x6, x7, [sp],#16
	ldp x4, x5, [sp], #16
	ldp x2, x3, [sp],#16

	/* save all registers */
	stp x30, xzr, [x0, #16]
	stp x28, x29, [x0, #32]
	mrs x28, elr_el1
	mrs x29, spsr_el1
	stp x28, x29, [x0], #48
	stp x26, x27, [x0], #16
	stp x24, x25, [x0], #16
	stp x22, x23, [x0], #16
	stp x20, x21, [x0], #16
	stp x18, x19, [x0], #16
	stp x16, x17, [x0], #16
	stp x14, x15, [x0], #16
	stp x12, x13, [x0], #16
	stp x10, x11, [x0], #16
	stp x8, x9, [x0], #16
	stp x6, x7, [x0], #16
	stp x4, x5, [x0], #16
	stp x2, x3, [x0], #48
	mrs x2, tpidr_el0
	mrs x3, tpidr_el1
	stp x2, x3, [x0], #-16
	add x2, sp, #16
	mrs x3, sp_el0
	stp x2, x3, [x0], #-16
	mov x2, x0
	ldp x0, x1, [sp],#16
	stp x0, x1, [x2]

	/* Figure out our CPU number in the multicore complex: Read MPIDR */
	mrs     x0, mpidr_el1
	and     x0, x0, #0x3

	bl		svcService

	b		schedule
#endif
