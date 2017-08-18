/*
 * ARM hand-optimized assembly routine for lwIP-specific IP checksum function
 *
 * This code has been benchmarked on Armulator to achieve 85MByte/s on 80MHz ARM7TDMI.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: csumthumb.S,v 1.2 2006-07-11 21:28:04 $
 */

/*
 * uint16 arm_inet_checksum(const uint8 *pb, int len)
 *	Input: r0 = pb, r1 = len
 *	Output: r0 = checksum
 */

	.code	16
	.text
	.align	2
	.global arm_inet_checksum
	.thumb_func
	.type	arm_inet_checksum, %function
arm_inet_checksum:
	push	{r4, r5, r6, r7, lr}
	mov	r5, #0			@ temp for unaligned start/end byte
	mov	r4, r1
	mov	r6, r0			@ r6 = start address odd aligned
	mov	r3, #1
	and	r6, r3
	beq	1f
	cmp	r1, #0			@    and not zero-length
	ble	1f
	ldrb	r3, [r0]		@ save odd byte in temp high byte
	lsl	r5, r3, #8
	add	r0, r0, #1
	sub	r4, r4, #1
1:	push	{r6}			@ free up r6 for main loop
	mov	r3, #3			@ start address 4-aligned?
	tst	r0, r3			@ yes, skip to chunks
	beq	7f
	cmp	r4, #1			@ if zero-length, skip to chunks
	ble	7f
	sub	r4, r4, #2
	ldrh	r1, [r0]		@ start sum with odd half-word
	add	r0, r0, #2
	cmp	r4, #31			@ handle 4-aligned chunks of 32
	bgt	9f
2:	ldr	r2, 11f			@ fold upper bits
	lsr	r3, r1, #16
	and	r2, r2, r1
	add	r2, r3, r2
	cmp	r4, #1			@ add 0-15 half-words remaining
	ble	4f
3:	ldrh	r3, [r0]
	sub	r4, r4, #2
	add	r2, r2, r3
	add	r0, r0, #2
	cmp	r4, #1
	bgt	3b
4:	cmp	r4, #1			@ save odd byte to temp low byte
	bne	5f
	ldrb	r3, [r0]
	orr	r5, r5, r3
5:	add	r3, r5, r2		@ add in temp
	ldr	r1, 11f			@ fold upper bits
	lsr	r2, r3, #16
	and	r3, r3, r1
	add	r2, r2, r3
	lsr	r3, r2, #16		@ fold upper bits again
	and	r2, r2, r1
	add	r0, r3, r2
	pop	{r6}			@ restore r6 (original alignment)
	cmp	r6, #0			@ byte-swap sum if orig. alignment odd
	beq	6f
	lsl	r3, r0, #8
	lsr	r2, r0, #16
	orr	r3, r3, r2
	mov	r0, r3
	and	r0, r0, r1
6:	pop	{r4, r5, r6, r7, pc}	@ return
7:	mov	r1, #0
8:	cmp	r4, #31
	ble	2b
9:	ldmia	r0!, {r2, r3, r6, r7}	@ process 16 bytes
	add	r1, r1, r2
	adc	r1, r1, r3
	adc	r1, r1, r6
	adc	r1, r1, r7
	ldmia	r0!, {r2, r3, r6, r7}	@ unroll, process another 16 bytes
	adc	r1, r1, r2
	adc	r1, r1, r3
	adc	r1, r1, r6
	adc	r1, r1, r7
	bcc	10f
	add	r1, r1, #1		@ add back last carry
10:	sub	r4, r4, #32
	b	8b
	.align	2
11:	.word	65535
