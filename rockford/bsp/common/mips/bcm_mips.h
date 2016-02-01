/***************************************************************
 *    (c)2010-2011 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
****************************************************************/
#ifndef __BCM_MIPS__
#define __BCM_MIPS__

#if 1 /*def _ASMLANGUAGE*/
#define zero    $0      /* wired zero */
#define AT      $1      /* assembler temp  - uppercase because of ".set at" */
#define v0      $2      /* return value */
#define v1      $3
#define a0      $4      /* argument registers */
#define a1      $5
#define a2      $6
#define a3      $7
#define t0      $8      /* caller saved */
#define t1      $9
#define t2      $10
#define t3      $11
#define t4      $12
#define t5      $13
#define t6      $14
#define t7      $15
#define s0      $16     /* callee saved */
#define s1      $17
#define s2      $18
#define s3      $19
#define s4      $20
#define s5      $21
#define s6      $22
#define s7      $23
#define t8      $24     /* caller saved */
#define t9      $25
#define jp      $25     /* PIC jump register */
#define k0      $26     /* kernel scratch */
#define k1      $27
#define gp      $28     /* global pointer */
#define sp      $29     /* stack pointer */
#define fp      $30     /* frame pointer */
#define s8	$30	/* same like fp! */
#define ra      $31     /* return address */

/*
 * Coprocessor 0 register names
 */
#define CP0_INDEX $0
#define CP0_RANDOM $1
#define CP0_ENTRYLO0 $2
#define CP0_ENTRYLO1 $3
#define CP0_CONF $3
#define CP0_CONTEXT $4
#define CP0_PAGEMASK $5
#define CP0_WIRED $6
#define CP0_INFO $7
#define CP0_BADVADDR $8
#define CP0_COUNT $9
#define CP0_ENTRYHI $10
#define CP0_COMPARE $11
#define CP0_STATUS $12
#define CP0_CAUSE $13
#define CP0_EPC $14
#define CP0_PRID $15
#define CP0_EBASE $15,1
#define CP0_CONFIG $16
#define CP0_LLADDR $17
#define CP0_WATCHLO $18
#define CP0_WATCHHI $19
#define CP0_XCONTEXT $20
#define CP0_FRAMEMASK $21
#define CP0_DIAGNOSTIC $22
#define CP0_DEBUG $23
#define CP0_DEPC $24
#define CP0_PERFORMANCE $25
#define CP0_ECC $26
#define CP0_CACHEERR $27
#define CP0_TAGLO $28
#define CP0_DATALO $28,1
#define CP0_TAGHI $29
#define CP0_ERROREPC $30
#define CP0_DESAVE $31
#endif

#define ST0_CU0			0x10000000
#define ST0_BEV			0x00400000
#define ST0_IE			0x00000001
#define ST0_EXL			0x00000002
#define ST0_ERL			0x00000004
#define ST0_KSU			0x00000018
#  define KSU_USER		0x00000010
#  define KSU_SUPERVISOR	0x00000008
#  define KSU_KERNEL		0x00000000
#define ST0_UX			0x00000020
#define ST0_SX			0x00000040
#define ST0_KX 			0x00000080
#define ST0_DE			0x00010000
#define ST0_CE			0x00020000
#define ST0_IM				0x0000ff00
#define  STATUSB_IP0		8
#define  STATUSF_IP0		(1UL <<  8)
#define  STATUSB_IP1		9
#define  STATUSF_IP1		(1UL <<  9)
#define  STATUSB_IP2		10
#define  STATUSF_IP2		(1UL << 10)
#define  STATUSB_IP3		11
#define  STATUSF_IP3		(1UL << 11)
#define  STATUSB_IP4		12
#define  STATUSF_IP4		(1UL << 12)
#define  STATUSB_IP5		13
#define  STATUSF_IP5		(1UL << 13)
#define  STATUSB_IP6		14
#define  STATUSF_IP6		(1UL << 14)
#define  STATUSB_IP7		15
#define  STATUSF_IP7		(1UL << 15)


#define  CAUSEB_EXCCODE		2
#define  CAUSEF_EXCCODE		(31UL  <<  2)
#define  CAUSEB_IP		8
#define  CAUSEF_IP		(255UL <<  8)
#define  CAUSEB_IP0		8
#define  CAUSEF_IP0		(1UL   <<  8)
#define  CAUSEB_IP1		9
#define  CAUSEF_IP1		(1UL   <<  9)
#define  CAUSEB_IP2		10
#define  CAUSEF_IP2		(1UL   << 10)
#define  CAUSEB_IP3		11
#define  CAUSEF_IP3		(1UL   << 11)
#define  CAUSEB_IP4		12
#define  CAUSEF_IP4		(1UL   << 12)
#define  CAUSEB_IP5		13
#define  CAUSEF_IP5		(1UL   << 13)
#define  CAUSEB_IP6		14
#define  CAUSEF_IP6		(1UL   << 14)
#define  CAUSEB_IP7		15
#define  CAUSEF_IP7		(1UL   << 15)
#define  CAUSEB_IV		23
#define  CAUSEF_IV		(1UL   << 23)

/* Location of pointer to the current exception handler */

#define VECTOR_START 	0x8000017C

/* 
 * Some debug CP0 Register locations within saved register space
 */
#define ROFF_NUM_REG	(40)	/* Number of 32-bit registers stacked in a context switch (see BCM_SAVE, below)*/
#define ROFF_REG_SPACE	(ROFF_NUM_REG	* 4)	/* Size of register space in bytes */
#define ROFF_SP			29		/* word offset to sp */
#define ROFF_RA			31		/* word offset to ra */
#define ROFF_SR			32		/* word offset to CP0_STATUS */
#define ROFF_LO			33		/* word offset to LO */
#define ROFF_HI			34		/* word offset to HI */
#define	ROFF_BADVA			35		/* word offset to CP0_BADVADDR */
#define ROFF_CAUSE			36		/* word offset to CP0_CAUSE */
#define ROFF_PC			37		/* word offset to CP0_EPC */
#define ROFF_START_PC		38		/* word offset to new task PC */
#define ROFF_ERROR_PC		39		/* word offset to ErrorPC */ 
#define ROFF_START_A0		40		/* word offset to startup a0 */ 

/* Don't save/restore gp register.  GreenHills uses this register as a global for i/o */

#if 1 /*def _ASMLANGUAGE*/
/* Macros */
#define BCM_SAVE(reg) \
	sw AT, 1*4(reg) ; \
	.set at		; \
	sw $0, 0(reg)   ; \
	sw v0, 2*4(reg) ; \
	sw v1, 3*4(reg) ; \
	sw a0, 4*4(reg) ; \
	sw a1, 5*4(reg) ; \
	sw a2, 6*4(reg) ; \
	sw a3, 7*4(reg) ; \
	sw t0, 8*4(reg) ; \
	sw t1, 9*4(reg) ; \
	sw t2,10*4(reg) ; \
	sw t3,11*4(reg) ; \
	sw t4,12*4(reg) ; \
	sw t5,13*4(reg) ; \
	sw t6,14*4(reg) ; \
	sw t7,15*4(reg) ; \
	sw s0,16*4(reg) ; \
	sw s1,17*4(reg) ; \
	sw s2,18*4(reg) ; \
	sw s3,19*4(reg) ; \
	sw s4,20*4(reg) ; \
	sw s5,21*4(reg) ; \
	sw s6,22*4(reg) ; \
	sw s7,23*4(reg) ; \
	sw t8,24*4(reg) ; \
	sw t9,25*4(reg) ; \
	sw sp,29*4(reg) ; \
	sw s8,30*4(reg) ; \
	sw ra,31*4(reg) ; \
	mfc0 t0,CP0_BADVADDR ; \
	mfc0 t1,CP0_STATUS ; \
	mfc0 a1,CP0_CAUSE ; \
	mfc0 t2,CP0_EPC ; \
	mflo t3 ; \
	mfhi t4 ; \
	mfc0 t5, CP0_ERROREPC; \
	sw t0, ROFF_BADVA * 4(reg) ; \
	sw t1, ROFF_SR * 4(reg) ; \
	sw a1, ROFF_CAUSE * 4(reg) ; \
	sw t2, ROFF_PC * 4(reg) ; \
	sw t3, ROFF_LO * 4(reg) ; \
	sw t4, ROFF_HI * 4(reg) ; \
	sw t5, ROFF_ERROR_PC * 4(reg) ;
	
    
#define BCM_RESTORE(reg) \
	lw t1,ROFF_PC * 4(reg)	; \
	mtc0 t1,CP0_EPC	; \
	lw t0,ROFF_LO * 4(reg)	; \
	lw t1,ROFF_HI * 4(reg)	; \
	mtlo t0	; \
	mthi t1	; \
	.set noat	; \
	lw AT, 1*4(reg)	; \
	.set at	; \
	lw v0, 2*4(reg)	; \
	lw v1, 3*4(reg)	; \
	lw a0, 4*4(reg)	; \
	lw a1, 5*4(reg)	; \
	lw a2, 6*4(reg)	; \
	lw a3, 7*4(reg)	; \
	lw t0, 8*4(reg)	; \
	lw t1, 9*4(reg)	; \
	lw t2,10*4(reg)	; \
	lw t3,11*4(reg)	; \
	lw t4,12*4(reg)	; \
	lw t5,13*4(reg)	; \
	lw t6,14*4(reg)	; \
	lw t7,15*4(reg)	; \
	lw s0,16*4(reg)	; \
	lw s1,17*4(reg)	; \
	lw s2,18*4(reg)	; \
	lw s3,19*4(reg)	; \
	lw s4,20*4(reg)	; \
	lw s5,21*4(reg)	; \
	lw s6,22*4(reg)	; \
	lw s7,23*4(reg)	; \
	lw t8,24*4(reg)	; \
	lw t9,25*4(reg)	; \
	lw sp,29*4(reg)	; \
	lw s8,30*4(reg)	; \
	lw ra,31*4(reg) ;
#endif


#define K0BASE          0x80000000
#define K1BASE          0xa0000000


#endif /* __BCM_MIPS__ */

