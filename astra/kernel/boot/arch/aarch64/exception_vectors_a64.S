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

.section ".text"

.global el1Vectors

.align 11

/* -----------------------------------------------------
 * Current EL with SP0 : 0x0 - 0x180
 * -----------------------------------------------------
 */
el1Vectors:
tsync:
	b .

	.balign 0x80
tirq:
	b .

	.balign 0x80
tfiq:
	b .

	.balign 0x80
tserr:
	b .

/* -----------------------------------------------------
 * Current EL with SPx: 0x200 - 0x380
 * -----------------------------------------------------
 */
	.balign 0x80
hsync:
	b aarch64_sync_handler

	.balign 0x80
hirq:
	b aarch64_irq_handler

	.balign 0x80
hfiq:
	b aarch64_fiq_handler

	.balign 0x80
hserr:
	b .

/* -----------------------------------------------------
 * Lower EL using AArch64 : 0x400 - 0x580
 * -----------------------------------------------------
 */
	.balign 0x80
hsyncA64:
	b aarch64_sync_handler

	.balign 0x80
hirqA64:
	b aarch64_irq_handler

	.balign 0x80
hfiqA64:
	b aarch64_fiq_handler

	.balign 0x80
hserrA64:
	b .

/* -----------------------------------------------------
 * Lower EL using AArch32 : 0x0 - 0x180
 * -----------------------------------------------------
 */
	.balign 0x80
hsyncA32:
	b .

	.balign 0x80
hirqA32:
	b .

	.balign 0x80
hfiqA32:
	b .

	.balign 0x80
hserrA32:
	b .
