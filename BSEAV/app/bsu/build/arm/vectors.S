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
	.section .vectors

	b	reset_handler
	b	undef_handler
	b	svc_handler
	b	dabort_handler
	b	pabort_handler
	b	hyp_handler
//	b	irq_handler
	b	OS_CPU_ARM_ExceptIrqHndlr
	b	fiq_handler

reset_handler:
	/* Jumping to 0 will take the reset path */
	b	startup

undef_handler:
	b	undef_handler

svc_handler:
	b	svc_handler

dabort_handler:
	b	dabort_handler

pabort_handler:
	b	pabort_handler

hyp_handler:
	b	hyp_handler

irq_handler:
	b	irq_handler

fiq_handler:
	b	fiq_handler

	.global setup_vectors
setup_vectors:
	mrc	p15, 0, r0, c0, c1, 1
	ands	r0, #(1 << 4)
	beq	1f

	ldr	r0, =.vectors
	mcr	p15, 0, r0, c12, c0, 0

	mrc	p15, 0, r0, c1, c0, 0
	and	r0, #~(1 << 13)
	mcr	p15, 0, r0, c1, c0, 0

1:
	bx	lr
