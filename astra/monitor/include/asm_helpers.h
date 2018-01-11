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

#ifndef _ASM_HELPERS_H_
#define _ASM_HELPERS_H_

#ifdef __ASSEMBLY__
/*
 * This macro is used to create a function label and place the
 * code into a separate text section based on the function name
 * to enable elimination of unused code during linking
 */
.macro func _name _p2align=0
.section .text.\_name, "ax"
.p2align \_p2align
.type \_name, %function
.func \_name
\_name:
.endm

/*
 * This macro is the same as func macro above, with the additional
 * delaration that makes the function name global
 */
.macro func_global _name _p2align=0
.section .text.\_name, "ax"
.p2align \_p2align
.global \_name
.type \_name, %function
.func \_name
\_name:
.endm

/*
 * This macro is used to create a function label and place the
 * code into .text.bootstrap section
 */
.macro func_bootstrap _name _p2align=0
.section .text.bootstrap, "ax"
.p2align \_p2align
.type \_name, %function
.func \_name
\_name:
.endm

/*
 * This macro is used to create a function label and place the
 * code into .vectors section
 */
.macro func_vectors _name _p2align=0
.section .vectors, "ax"
.p2align \_p2align
.type \_name, %function
.func \_name
\_name:
.endm

/*
 * This macro is used to prepad a function and place the pre-padding
 * into the same text section based on the function name
 */
.macro func_prepad _name _p2align=0
.section .text.\_name, "ax"
.p2align \_p2align
prepad_\_name:
.endm

/*
 * This macro is used to mark the end of a function.
 */
.macro endfunc _name
.endfunc
.size \_name, . - \_name
.endm

/*
 * This macro is used to mark the end of a function that has pre-padding,
 * so to include pre-padding in size.
 */
.macro endfunc_prepad _name
.endfunc
.size \_name, . - prepad_\_name
.endm

/*
 * Declare the exception vector table, enforcing it is aligned on a
 * 2KB boundary, as required by the ARMv8 architecture.
 * Use zero bytes as the fill value to be stored in the padding bytes
 * so that it inserts illegal AArch64 instructions. This increases
 * security, robustness and potentially facilitates debugging.
 */
.macro vector_base _label
.section .vectors, "ax"
.align 11, 0
\_label:
.endm

/*
 * Create an entry in the exception vector table, enforcing it is
 * aligned on a 128-byte boundary, as required by the ARMv8 architecture.
 * Use zero bytes as the fill value to be stored in the padding bytes
 * so that it inserts illegal AArch64 instructions. This increases
 * security, robustness and potentially facilitates debugging.
 */
.macro vector_entry _label
.align 7, 0
\_label:
.endm

/*
 * This macro is used to mark the end of vectors.
 * Use zero bytes as the fill value to be stored in the padding bytes
 * so that it inserts illegal AArch64 instructions. This increases
 * security, robustness and potentially facilitates debugging.
 */
.macro vector_end _label
.align 7, 0
.endm

/*
 * This macro verifies that the given vector doesn't exceed the
 * architectural limit of 32 instructions. This is meant to be placed
 * immediately after the last instruction in the vector. It takes the
 * vector entry as the parameter.
 */
.macro assert_vector_size _label
  .if (. - \_label) > 128
    .error "Vector exceeds 128 bytes"
  .endif
.endm

/*
 * These 2 macros simulate the traditional push and pop intsructions
 * for a pair of registers.
 */
.macro push ra, rb, sp=sp
	stp \ra, \rb, [\sp,#-16]!
.endm

.macro pop ra, rb, sp=sp
	ldp \ra, \rb, [\sp], #16
.endm

#endif /* __ASSEMBLY__ */

#endif /* _ASM_HELPERS_H_ */
