/*
 * ARM hand-optimized assembly routine for lwIP-specific IP checksum function
 *
 * This code has been benchmarked on Armulator to achieve 93MByte/s on 80MHz ARM7TDMI.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: csumarm.S,v 1.2 2006-07-11 21:28:04 $
 */

/*
 * uint16 arm_inet_checksum(const uint8 *pb, int len)
 *	Input: r0 = pb, r1 = len
 *	Output: r0 = checksum
 *
 * If compiled with -DTHUMB, this code switches into ARM mode temporarily.
 * See csumthumb.S for pure THUMB version.
 */

	.text
	.global arm_inet_checksum
	.type	arm_inet_checksum, %function
#ifdef THUMB
	.thumb_func
	.align	2
arm_inet_checksum:
	ldr	r2, =arm_inet_checksum_arm
	bx	r2			@ transfer to ARM (r2 bit 0 clear)
	.arm
	.align	4
	.global arm_inet_checksum_arm
	.type	arm_inet_checksum_arm, %function
arm_inet_checksum_arm:
#else /* ARM */
	.align	4
arm_inet_checksum:
#endif /* ARM */
	stmfd	sp!, {r4, r5, r6}
	and	ip, r0, #1		@ starts address not 2-aligned?
	cmp	r1, #0
	movle	r3, #0
	andgt	r3, ip, #1
	cmp	r3, #0
	sub	sp, sp, #4
	mov	r3, #0
	strh	r3, [sp, #2]
	ldrneb	r3, [r0], #1		@ save odd byte in stack temp high byte
	subne	r1, r1, #1
	strneb	r3, [sp, #3]
	ands	r3, r0, #3		@ start address not 4-aligned?
	movne	r3, #1
	cmp	r1, #1
	movle	r3, #0
	andgt	r3, r3, #1
	cmp	r3, #0
	ldrneh	r2, [r0], #2		@ start sum with odd half-word
	moveq	r2, r3
	subne	r1, r1, #2
	adds	r2, r2, #0		@ clear carry
	b	2f
1:	ldmia	r0!, {r3, r4, r5, r6}	@ load 16 bytes at a time
	adcs	r2, r2, r3		@ sum, adding back carries
	adcs	r2, r2, r4
	adcs	r2, r2, r5
	adcs	r2, r2, r6
	ldmia	r0!, {r3, r4, r5, r6}	@ unroll for another 16 bytes
	adcs	r2, r2, r3
	adcs	r2, r2, r4
	adcs	r2, r2, r5
	adcs	r2, r2, r6
	sub	r1, r1, #32		@ does not affect C flag
2:	bics	r3, r1, #31		@ does not affect C flag
	bne	1b			@ r1 > 31
	adcs	r2, r2, #0		@ add back last carry
	mov	r3, r2, asl #16		@ fold upper bits
	mov	r3, r3, lsr #16
	cmp	r1, #1			@ add 0-15 half-words remaining
	add	r2, r3, r2, lsr #16
	ble	4f
3:	sub	r1, r1, #2
	ldrh	r3, [r0], #2
	cmp	r1, #1
	add	r2, r2, r3
	bgt	3b
4:	cmp	r1, #1			@ save odd byte to stack temp low byte
	ldreqb	r3, [r0, #0]
	streqb	r3, [sp, #2]
	ldrh	r3, [sp, #2]		@ add in stack temp
	add	r3, r3, r2
	mov	r1, r3, asl #16		@ fold upper bits
	mov	r1, r1, lsr #16
	add	r1, r1, r3, lsr #16	@ fold upper bits again
	mov	r2, r1, asl #16
	mov	r2, r2, lsr #16
	cmp	ip, #0			@ byte-swap sum if orig. alignment odd
	add	r0, r2, r1, lsr #16
	movne	r3, r0, lsr #16
	orrne	r3, r3, r0, asl #8
	movne	r0, r3, asl #16
	movne	r0, r0, lsr #16
	add	sp, sp, #4
	ldmfd	sp!, {r4, r5, r6}
#ifdef THUMB
	bx	lr			@ return to ARM or THUMB caller
#else
	mov	pc, lr
#endif
