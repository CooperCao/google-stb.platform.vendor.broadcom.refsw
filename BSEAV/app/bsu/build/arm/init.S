/***************************************************************************
 *     Copyright (c) 2012-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
	.align  4

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
