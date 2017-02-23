/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 ***************************************************************************/
#define MODE_SVC	0x13
#define MODE_IRQ	0x12
#define MODE_ABT	0x17
#define MODE_UNDEF	0x1B

#define IRQ_FLAG	0x80
#define FIQ_FLAG	0x40

#define _LONG_	.word

#define DECLARE_LONG(x) \
	.global x ; \
x:	_LONG_  0

/* ------------------------------------------------------------------------- */
	.section .data
	.align  8

DECLARE_LONG(__saved_sp)
DECLARE_LONG(__saved_lr)
/* ------------------------------------------------------------------------- */

	.section .init

	.global	startup
startup:

	ldr	r4, =__saved_sp
	str	sp,	[r4]

	/* lr should also be on the stack on entry */
	ldr	r4, =__saved_lr
	str	lr,	[r4]

	bl	setup_stacks
	push	{r0-r2, r3}
	bl	setup_vectors
	bl	setup_cpu
	bl	c_init
    isb
    dsb
	pop		{r0-r2, r3}
	b	bcm_main

    .global mini_exit
mini_exit:
	ldr	r4, =__saved_sp
	ldr	sp,	[r4]
	pop	{ pc }
i_am_dead:
	b	i_am_dead

setup_stacks:
	msr	cpsr_c, #MODE_SVC | IRQ_FLAG | FIQ_FLAG
	ldr	sp, =_stack_svc_base
	msr	cpsr_c, #MODE_IRQ | IRQ_FLAG | FIQ_FLAG
	ldr	sp, =_stack_irq_base
	msr	cpsr_c, #MODE_ABT | IRQ_FLAG | FIQ_FLAG
	ldr	sp, =_stack_abt_base
	msr	cpsr_c, #MODE_UNDEF | IRQ_FLAG | FIQ_FLAG
	ldr	sp, =_stack_undef_base
	msr	cpsr_c, #MODE_SVC | IRQ_FLAG | FIQ_FLAG
	bx	lr

setup_cpu:
	bx lr

	.global __aeabi_unwind_cpp_pr0
__aeabi_unwind_cpp_pr0:
	mov	pc, lr

	.global raise
raise:
	mov	r0, #0
	mov	pc, lr

	.global __getstack
__getstack:
	mov r0, sp
	bx  lr
