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
#include "arm/arm.h"

.arm

.global tz_secure_data_abort_handler

.section ".text"

 tz_secure_data_abort_handler:

    cpsid   if
    sub     lr, lr, #8

    push   {r0}
    push   {r1}
    push   {r2}
    push   {lr}
    bl get_regbase
    pop    {lr}
    pop    {r2}
    pop    {r1}

    /*
     * Save all the core regsiters
     */
    str     r1, [r0, #4]
    str     r2, [r0, #8]
    str     r3, [r0, #12]
    str     r4, [r0, #16]
    str     r5, [r0, #20]
    str     r6, [r0, #24]
    str     r7, [r0, #28]
    str     r8, [r0, #32]
    str     r9, [r0, #36]
    str     r10, [r0, #40]
    str     r11, [r0, #44]
    str     r12, [r0, #48]

    mrs     r1, spsr_abt
    str     r1, [r0, #64]

    and     r1, #0x1f
    cmp     r1, #Mode_SVC

    mrs     r1, sp_svc
    str     r1, [r0, #52]
    str     lr, [r0, #56]

    beq     1f

    mrs     r1, sp_usr
    str     r1, [r0, #68]
    mrs     r1, lr_usr
    str     r1, [r0, #72]

1:
    mov     r1, r0
    pop     {r0}
    str     r0, [r1]


    /*
     * Next we need to put the CPU in SVC mode and
     * then jump to the data abort exception handler.
     */
    /* Figure out our CPU number in the multicore complex: Read MPIDR */
    mrc     p15, 0, r4, c0, c0, 5
    and     r4, r4, #3

    /* setup the SVC mode interrupt stack */
    ldr     r6, =dabt_exc_stack_start
    mov     r7, #INIT_STACK_SIZE
    mul     r7, r4
    sub     r6, r7
    msr     SP_svc, r6

    mov     r0, #2
    mrs     r1, spsr_abt
    bl      yieldCurrentTask

    cpsid   if, #Mode_SVC
    bl      serviceDataAbort

    b       schedule

.section ".data"
    .align 5
    .space   INIT_STACK_SIZE*MAX_NUM_CPUS
dabt_exc_stack_start:
