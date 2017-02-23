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

#if defined(LIBSTRING_ASM_MEMCPY)

	.arm

#if !CFG_UNCACHED
 #define __ARM_NEON__
#endif

#if defined(__ARM_NEON__)
	.fpu neon
#endif

	.section .text

.global lib_memcpy
.type lib_memcpy,"function"
lib_memcpy:
	/* r0=dst r1=src r2=len */

	push	{r0, r4, lr}
	cmp		r2, #0x0		@ Short-circuit 0 length copies
	beq		done
	ands	r4, r0, #0x3	@ Check if dst is word aligned
	bne		copy_bytes
	ands	r4, r1, #0x3	@ Check if src is word aligned
	bne		copy_bytes
	cmp		r2, #0x4		@ Check if len is at least 1 word
	blt		copy_bytes
	ands	r4, r2, #0x3	@ Check if len is word aligned
	bne		copy_bytes
#if defined(__ARM_NEON__)
	cmp		r2, #0x40		@ Check if len is >= NEON stride length
	blt		copy_words
	ldr		r3, =0x40000000	@ Enable NEON
	fmxr	FPEXC, r3
copy_words_neon:
	pld		[r1, #0xC0]		@ Cache preload hint
	vldm	r1!, {d0-d7}	@ *r1 => NEON registers
	vstm	r0!, {d0-d7}	@ *r0 <= NEON registers
	sub		r2, #0x40
	cmp		r2, #0x40
	bge		copy_words_neon
	cmp		r2, #0
	beq		done
	ldr		r3, =0x0		@ Disable NEON
	fmrx	r3, FPEXC
#endif
copy_words:
	ldr		r3, [r1], #4
	str		r3, [r0], #4
	subs	r2, #4
	bne		copy_words
	b		done
copy_bytes:
	ldrb	r3, [r1], #1
	strb	r3, [r0], #1
	subs	r2, #1
	bne		copy_bytes
done:
	pop 	{r0, r4, pc}

	.end
#endif
