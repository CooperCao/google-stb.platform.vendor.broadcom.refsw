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
#include "plat_config.h"
#include "arch.h"

/* ARM image header for AARCH64 ("ARM\x64") */
#define ARM_IMAGE_MAGIC_AARCH64         0x644d5241

.global tz_init
.global tz_bootstrap

.section ".text.bootstrap"

tz_init:

    /* The image must begin with a ARM image header as used by Linux
     *
     *      u32 code0;                  # Executable code
     *      u32 code1;                  # Executable code
     *      u64 text_offset;            # Image load offset, little endian
     *      u64 image_size;             # Effective Image size, little endian
     *      u64 flags;                  # kernel flags, little endian
     *      u64 res2 = 0;               # reserved
     *      u64 res3 = 0;               # reserved
     *      u64 res4 = 0;               # reserved
     *      u32 magic = 0x644d5241;     # Magic number, little endian, "ARM\x64"
     *      u32 res5;                   # reserved (used for PE COFF offset)
     */

    b       tz_bootstrap
    .word   0

    .rept   6
    .word   0
    .word   0
    .endr

    .word   ARM_IMAGE_MAGIC_AARCH64
    .word   0

tz_bootstrap:

    /* The bootloader has loaded the following registers:
     * - x0: Device tree memory location (primary CPU), or 0 (secondary CPU)
     * - x1: UART base (brcmstb specific)
     * - x2: 0
     * - x3: 0
     */
    mov     x20, x0
    mov     x21, x1

    /* What EL are we in? */
    mrs     x0, CurrentEL
    and     x0, x0, #(0xC)

    /* Halt if not in EL1 */
    cmp     x0, #0x4
    bne     .

    /* Calculate offset between load address and link address */
    adr     x0, tz_init     /* Load address of tz_init */
    ldr     x1, =tz_init    /* Link address of tz_init */
    sub     x22, x0, x1     /* Load-to-link offset */

    /* Figure out current CPU number from MPIDR */
    mrs     x0, mpidr_el1
    and     x23, x0, #0x3
    cmp     x23, #0
    bne     2f

    /* Clear BSS section */
    ldr     x0, =_bss_start
    add     x0, x0, x22
    ldr     x1, =_bss_end
    add     x1, x1, x22
1:
    stp     xzr, xzr, [x0], #16
    cmp     x0, x1
    blo     1b
2:
    /* Set bootstrap stack to load address */
    ldr     x0, =bootstrap_stacks_start
    add     x0, x0, x22
    mov     x1, #INIT_STACK_SIZE
    mul     x1, x1, x23
    sub     x0, x0, x1
    mov     sp, x0

    /* Call bootstrap C-function
     * - x0: Device tree memory location (primary CPU), or 0 (secondary CPU)
     * - x1: UART base
     * - x2: Load-to-link offset
     */
    mov     x0, x20
    mov     x1, x21
    mov     x2, x22
    bl      bootstrapMain

    /* Set init stack to link address by adjusting bootstrap stack */
    sub     sp, sp, x22

    /* Enable floating point */
    mov     x0, #CPACR_EL1_FPEN(CPACR_EL1_FP_TRAP_NONE)
    msr     CPACR_EL1, x0

    /* Setup 64bit exception vectors */
    ldr     x0, =el1Vectors
    msr     VBAR_EL1, x0

    /* Enable SError interrupts */
    msr     daifclr, #DAIF_ABT_BIT

    /* Call kernel init function
     * - x0: Device tree memory location
     */
    mov     x0, x20
    ldr     x1, =tzKernelInit
    br      x1

.section ".data.bootstrap"
    .align  5
    .space  INIT_STACK_SIZE*MAX_NUM_CPUS
bootstrap_stacks_start:
init_stacks_start:
