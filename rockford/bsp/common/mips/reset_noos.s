/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:  low level initialization code
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef __ASSEMBLER__
#define __ASSEMBLER__
#endif

#ifndef _ASMLANGUAGE
#define _ASMLANGUAGE
#endif

#include "archMips.h"
#include "mips.h"
#include "bcmuart.h"
#include "board.h"
#include "bchp_common.h"
#include "bchp_uarta.h"

/* defines */
/* In 34k, CP0_STATUS_SR_MASK and CP0_STATUS_CU1_MASK can not be written.*/
#define INITIAL_SR   (SR_BEV | SR_ERL)

/* internals */

.globl  romInit                /* branch to start of system code    */
.globl  romReboot               /* direct entry to reboot            */
/* externals */

#if 0
.globl  romStart                /* system initialization routine     */
#endif
        
#define RVECENT(f,n) \
        b f; nop
#define XVECENT(f,bev) \
        b f; li k0,bev

.text
.align	4
.globl	__start
__start:
romInit:
_romInit:

.set    noreorder

        la      sp, STACK_ADRS
        subu    sp, 32                  /* give me some room                 */
        la      t0, bcm_main

#if 0
        la      a2, romInit
        
        /* If you load the boot code from jtag, use 0x8040,0000 */
        la      a3, ROM_TEXT_ADRS
        subu    a1, a1, a2
        addu    a1, a1, a3
#endif

        jal     t0
        nop

        li      ra, R_VEC		/* load prom reset address		*/
        j       ra				/* just in case					*/
        nop
        nop

/*******************************************************************************
*
* romReserved -  Handle a jump to an unknown vector
*
*/

        .ent    romReserved
romReserved:
        b       romInit        /* just start over */
        .end    romReserved

/*******************************************************************************
*
* romExcHandle - rom based exception/interrupt handler
*
* This routine is invoked on an exception or interrupt while
* the status register is using the bootstrap exception vectors.
* It saves a state frame to a known uncached location so someone
* can examine the data over the VME.  It also displays a summary of the
* error on the boards alphanumeric display.
*
* THIS ROUTIINE IS NOT CALLABLE FROM "C"
*
*/

        .ent    romExcHandle
romExcHandle:
        .set    noat

hangExc:
        b       hangExc                 /* HANG UNTIL REBOOT    */

        .end    romExcHandle            /* that's all folks */
                                                                                 
/********************************************************
 *  void InstallVector(unsigned long offset,unsigned long isr_addr);
********************************************************/
        .globl  InstallVector
    .ent    InstallVector
InstallVector:
    .set    noreorder
    lui     t1,0x3c1a              	# opcode for lui $k0,IMM
    lui     t2,0x375a   			# opcode for ori $k0,$k0,IMM
    li      t3,0x03400008          	# opcode for jr $k0
    ori     v0,zero,16              # extract upper half
    srlv    v0,a1,v0                # of ISR_ADDR
    or      v0,t1,v0                # create opcode
    ori     v1,zero,0xffff          # extract lower half
    and     v1,a1,v1                # of ISR_ADDR
    or      v1,v1,t2                # create opcode
    li      t0,K1BASE               # store to SDRAM K1 segment
    addu    t0,t0,a0                # add offset to vector table base
    sw      v0,0(t0)                # store: lui $k0,IMM
    sw      v1,4(t0)                # store: ori $k0,$k0,IMM
    sw      t3,8(t0)                # store: jr $k0
    sw      zero,0xc(t0)            # store: nop
    jr      ra
    nop
    .set    reorder
    .end    InstallVector

.globl wait
.ent wait
.set noreorder
.set at
wait:

    /* Set CPU clock to 1/8th speed: Bits 23:24 = 11 */
    mfc0    a0, $22, 4        # 22,4
    li      a1,0x01800000
    or      a0,a0,a1
    mtc0    a0, $22, 4        # 22,4

    wait
    nop
    j       ra
    nop

.set noat
.set reorder
.end wait

/*
#************************************************************************
#* _writeasm: Write character to Serial Port                            *
#*                                                                      *
#*      Syntax: _writeasm (char)                                        *
#*        Note:                                                         *
#************************************************************************
*/
.align  2
.globl _writeasm
.ent _writeasm

_writeasm:  
        li      t0, UART_ADR_BASE

        li      t2, THRE
1:      lw      t1, UART_SDW_LSR(t0)
        and     t1, t1, t2
        bne     t1, t2, 1b
        nop
        sw      a0, UART_SDW_THR(t0)
        j       ra
        nop
        .set   reorder

.end _writeasm

.align  2
.globl uartb_out
.ent uartb_out

uartb_out:
        li      t0, UARTB_ADR_BASE
        li      t2, THRE
1:      lw      t1, UART_SDW_LSR(t0)
        and     t1, t1, t2
        bne     t1, t2, 1b
        nop
        sw      a0, UART_SDW_THR(t0)
        j       ra
        nop
        .set   reorder

        .end uartb_out

#if 0
      .align  2
        .globl uartc_out
        .ent uartc_out

uartc_out:
        li      t0, UARTC_ADR_BASE
        li      t2, THRE
1:      lw      t1, UART_SDW_LSR(t0)
        and     t1, t1, t2
        bne     t1, t2, 1b
        nop
        sw      a0, UART_SDW_THR(t0)
        j       ra
        nop
        .set   reorder

        .end uartc_out
#endif

#ifndef BSU
/******************************************************************************
 * Function: uartout_hex32
 * Arguments: 	None
 * Returns:		
 * Description: 
 * Trashes:		
 *
 *	pseudo code:
 *	
 ******************************************************************************/

/*#define	PHYS_TO_K1(x)	((unsigned)(x)|0xA0000000)*/	/* physical to kseg1 */
#define	PHYS_TO_K2(x)	((x) | 0xA0000000)
#define BCHP_PHYSICAL_OFFSET                               0x10000000

LEAF(uartout_hex32)

        li		t0, 32
        li		t3, UART_ADR_BASE

.set noreorder

       li		t4, BCHP_UARTA_LSR_THRE_MASK

uart_out_hex32_1:

       addiu	t0, t0, -4
       srlv		t1, a0, t0
       andi		t1, t1, 0xf
       addi		t2, t1, -10
       bgez		t2, uart_out_hex32_2
       nop
       addi		t2, t1, '0'
       b		uart_out_hex32_3
       nop

uart_out_hex32_2:

       addi		t2, t2, 'A'
uart_out_hex32_3:

       lw		t1, UART_SDW_LSR(t3)
       and		t1, t1, t4
       bne		t1, t4, uart_out_hex32_3
       nop
       sw    t2, UART_SDW_THR(t3)
       bne   t0, $0, uart_out_hex32_1
       nop
       jr    ra
       nop

.set reorder

    jr		ra
    nop

END(uartout_hex32)
#endif

#ifndef GHS
/*************************************************************
*  sbrk(size)
*       returns a pointer to a block of memory of the requested size.
*       Returns zero if heap overflow is detected. Heap overflow occurs
*       when the upper limit of the requested size, overlaps the stack
*       pointer.
*/
.globl sbrk
.ent sbrk
.set noreorder
.set at
sbrk:
    li      v0,0
    la      t0,allocp1
    lw      t6,(t0)
    li      t1, STACK_ADRS-STACK_SIZE
    addu    t7,t6,a0
    blt     t7,t1,1f
    nop
    j       ra
    nop
1:      sw      t7,(t0)
    move    v0,t6
    j       ra
    nop
    .set noat
    .set reorder
    .end sbrk

    .data
    .globl allocp1
#ifdef GHS
    .word __ghsbegin_boot
allocp1: .word  __ghsend_stack
#else
    .word _ftext
    .word etext
allocp1: .word  end
#endif
#endif
