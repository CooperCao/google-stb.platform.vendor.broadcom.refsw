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
#include <cortex_a53.h>

#include "asm_helpers.h"
#include "config.h"

.global reset_handler

/* -----------------------------------------------------
 * Errata Workaround for Cortex A53 Errata #826319.
 * This applies only to revision <= r0p2 of Cortex A53.
 *
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 *
 * Clobbers: x1
 * -----------------------------------------------------
 */
#if ERRATA_A53_826319
func_bootstrap errata_a53_826319_wa
	/*
	 * Compare x0 against revision r0p2
	 */
	cmp	x0, #2
	b.ls	apply_826319
	ret

apply_826319:
	mrs	x1, L2ACTLR_EL1
	bic	x1, x1, #L2ACTLR_ENABLE_UNIQUECLEAN
	orr	x1, x1, #L2ACTLR_DISABLE_CLEAN_PUSH
	msr	L2ACTLR_EL1, x1
	ret
#endif
endfunc errata_a53_826319_wa

/* ---------------------------------------------------------------------
 * Disable the cache non-temporal hint.
 *
 * This ignores the Transient allocation hint in the MAIR and treats
 * allocations the same as non-transient allocation types. As a result,
 * the LDNP and STNP instructions in AArch64 behave the same as the
 * equivalent LDP and STP instructions.
 *
 * This is relevant only for revisions <= r0p3 of Cortex-A53.
 * From r0p4 and onwards, the bit to disable the hint is enabled by
 * default at reset.
 *
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 *
 * Clobbers: x1
 * ---------------------------------------------------------------------
 */
#if ERRATA_A53_836870
func_bootstrap a53_disable_non_temporal_hint
	/*
	 * Compare x0 against revision r0p3
	 */
	cmp	x0, #3
	b.ls	disable_hint
	ret

disable_hint:
	mrs	x1, CPUACTLR_EL1
	orr	x1, x1, #CPUACTLR_DTAH
	msr	CPUACTLR_EL1, x1
	ret
#endif
endfunc a53_disable_non_temporal_hint

/* ---------------------------------------------------------------------
 * Reset handler.
 *
 * Clobbers: x0-x18, x19, x30
 * ---------------------------------------------------------------------
 */
func_bootstrap reset_handler
	mov	x19, x30

	/*
	 * Call platform reset handler
	 * Clobbers: x0-x18, x30
	 */
	bl plat_reset_handler

#if ERRATA_A53_826319 || ERRATA_A53_836870
	mrs	x0, midr_el1

	/*
	 * Extract the variant[20:23] and revision[0:3] from x0
	 * and pack it in x15[0:7] as variant[4:7] and revision[0:3].
	 */
	ubfx	x15, x0, #(MIDR_VAR_SHIFT - MIDR_REV_BITS), #(MIDR_REV_BITS + MIDR_VAR_BITS)
	bfxil	x15, x0, #MIDR_REV_SHIFT, #MIDR_REV_BITS

#if ERRATA_A53_826319
	mov	x0, x15
	bl	errata_a53_826319_wa
#endif

#if ERRATA_A53_836870
	mov	x0, x15
	bl	a53_disable_non_temporal_hint
#endif
#endif

	/*
	 * Enable write access of CPUECTLR from non-secure EL1
	 */
	mrs     x0, actlr_el3
	orr     x0, x0, #0x2
	msr     actlr_el3, x0

	mrs     x0, actlr_el2
	orr     x0, x0, #0x2
	msr     actlr_el2, x0

	/*
	 * Enable the SMP bit
	 */
	mrs	x0, CPUECTLR_EL1
	orr	x0, x0, #CPUECTLR_SMP_BIT
	msr	CPUECTLR_EL1, x0
	isb

	ret	x19
endfunc reset_handler
