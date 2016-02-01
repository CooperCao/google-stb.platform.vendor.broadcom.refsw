/***************************************************************************
*     (c)2008-2013 Broadcom Corporation
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
* API Description:
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef __INCasmMipsh
#define __INCasmMipsh

#ifdef __cplusplus
extern "C" {
#endif

#include "archMips.h"
/*
 * The LEADING_UNDERSCORE macro should be defined to TRUE for toolchains
 * that do NOT prefix a leading underscore character, i.e. "_", to
 * symbols.  Define the macro to FALSE when using a toolchain that
 * does add a leading underscore character to symbols.
 */

#define LEADING_UNDERSCORE FALSE 

#if (LEADING_UNDERSCORE == TRUE)
#define FUNC(func)          _##func
#define FUNC_LABEL(func)    _##func:
#else
#define FUNC(func)          func
#define FUNC_LABEL(func)    func:
#endif

#define FUNC_DECL(range,func)
#define VAR_DECL(var)   var
#define VAR(var)        var(r0)

/*
 * These macros are used to declare assembly language symbols that need
 * to be typed properly(func or data) to be visible to the OMF tool.  
 * So that the build tool could mark them as an entry point to be linked
 * by another PD.
 */

#define GTEXT(sym) FUNC(sym) ;  .type   FUNC(sym),@function
#define GDATA(sym) FUNC(sym) ;  .type   FUNC(sym),@object

#define FUNCREF(func)	func


/*
*  MIPS register definitions
*/

#define zero	$0	/* wired zero */
#define AT	$1	/* assembler temp */
#define v0	$2	/* return reg 0 */
#define v1	$3	/* return reg 1 */
#define a0	$4	/* arg reg 0 */
#define a1	$5	/* arg reg 1 */
#define a2	$6	/* arg reg 2 */
#define a3	$7	/* arg reg 3 */
#define t0	$8	/* caller saved 0 */
#define t1	$9	/* caller saved 1 */
#define t2	$10	/* caller saved 2 */
#define t3	$11	/* caller saved 3 */
#define t4	$12	/* caller saved 4 */
#define t5	$13	/* caller saved 5 */
#define t6	$14	/* caller saved 6 */
#define t7	$15	/* caller saved 7 */
#define s0	$16	/* callee saved 0 */
#define s1	$17	/* callee saved 1 */
#define s2	$18	/* callee saved 2 */
#define s3	$19	/* callee saved 3 */
#define s4	$20	/* callee saved 4 */
#define s5	$21	/* callee saved 5 */
#define s6	$22	/* callee saved 6 */
#define s7	$23	/* callee saved 7 */
#define t8	$24	/* caller saved 8 */
#define t9	$25	/* caller saved 9 */
#define k0	$26	/* kernel temp 0 */
#define k1	$27	/* kernel temp 1 */
#define gp	$28	/* global pointer */
#define sp	$29	/* stack pointer */
#define s8	$30	/* callee saved 8 */
#define ra	$31	/* return address */

/*
* MIPS Coprocessor 0 regs
*/
#define C0_IBASE	$0	/* R4650: instruction base xlate address */
#define C0_IBOUND	$1	/* R4650: instruction xlate address bound */
#define C0_DBASE	$2	/* R4650: data base xlate address */
#define C0_DBOUND	$3	/* R4650: data xlate address bound */
#define	C0_INX		$0	/* tlb index */
#define	C0_RAND		$1	/* tlb random */

/* Begin CPUs: R3000, CW4000, CW4011 */
#define	C0_TLBLO	$2	/* tlb entry low */
/* End R3000, CW4000, CW4011 */

/* Begin CPUs: R4000, VR5000, VR5400, VR4100 */
#define C0_TLBLO0	$2	/* tlb entry low 0 */
#define C0_TLBLO1	$3	/* tlb entry low 1 */
/* End R4000, VR5000, VR5400, VR4100 */

#define	C0_CTXT		$4	/* tlb context */

/* Begin CPUs: R4000, VR5000, VR5400, VR4100 */
#define C0_PAGEMASK	$5	/* page mask */
#define C0_PGMASK	$5	/* page mask */
#define C0_WIRED	$6	/* lb wired entries */
/* End R4000, VR5000, VR5400, VR4100 */

#define	C0_BADVADDR	$8		/* bad virtual address */

/* Begin CPUs: R4000, R4650, VR5000, VR5400, CW4011, VR4100 */
#define	C0_COUNT	$9	/* count */
/* End R4000, R4650, VR5000, VR5400, CW4011, VR4100 */

/* Begin CPUs: R4000, VR5000, VR5400, R3000, CW4000, CW4011, VR4100 */
#define	C0_TLBHI	$10	/* tlb entry hi */
/* End R4000, VR5000, VR5400, R3000, CW4000, CW4011, VR4100 */

/* Begin CPUs: R4000, VR5000, VR5400, R4650, CW4011, VR4100 */
#define	C0_COMPARE	$11	/* compare */
/* End R4000, VR5000, VR5400, R4650, CW4011, VR4100*/

#define	C0_SR		$12	/* status register */
#define	C0_CAUSE	$13	/* exception cause */
#define	C0_EPC		$14	/* exception pc */

#define C0_PRID		$15

/* Begin CPUs: R4000, R4650, VR5000, VR5400, VR4100, CW4011 */
#define C0_CONFIG	$16

#define C0_CALG		$17	/* R4650: cache algorithm register */
#define C0_LLADDR	$17

#define C0_IWATCH	$18	/* R4650: instruction virt addr for watch */
#define C0_WATCHLO	$18

#define C0_DWATCH	$19	/* R4650: data virt addr for watch */
#define C0_WATCHHI	$19

#define C0_ECC		$26
#define C0_CACHEERR	$27
#define C0_TAGLO	$28

/* Begin CPUs: R4000, VR5000, VR5400, VR4100 */
#define C0_TAGHI	$29
/* End R4000, VR5000, VR5400, VR4100 */

#define C0_ERRPC	$30
/* End R4000, R4650, VR5000, VR5400, VR4100, CW4011 */

/*
*  MIPS floating point coprocessor register definitions
*/

#define fp0	$f0	/* return reg 0 */
#define fp1	$f1	/* return reg 1 */
#define fp2	$f2	/* return reg 2 */
#define fp3	$f3	/* return reg 3 */
#define fp4	$f4	/* caller saved 0 */
#define fp5	$f5	/* caller saved 1 */
#define fp6	$f6	/* caller saved 2 */
#define fp7	$f7	/* caller saved 3 */
#define fp8	$f8	/* caller saved 4 */
#define fp9	$f9	/* caller saved 5 */
#define fp10	$f10	/* caller saved 6 */
#define fp11	$f11	/* caller saved 7 */
#define fp12	$f12	/* arg reg 0 */
#define fp13	$f13	/* arg reg 1 */
#define fp14	$f14	/* arg reg 2 */
#define fp15	$f15	/* arg reg 3 */
#define fp16	$f16	/* caller saved 8 */
#define fp17	$f17	/* caller saved 9 */
#define fp18	$f18	/* caller saved 10 */
#define fp19	$f19	/* caller saved 11 */
#define fp20	$f20	/* callee saved 0 */
#define fp21	$f21	/* callee saved 1 */
#define fp22	$f22	/* callee saved 2 */
#define fp23	$f23	/* callee saved 3 */
#define fp24	$f24	/* callee saved 4 */
#define fp25	$f25	/* callee saved 5 */
#define fp26	$f26	/* callee saved 6 */
#define fp27	$f27	/* callee saved 7 */
#define fp28	$f28	/* callee saved 8 */
#define fp29	$f29	/* callee saved 9 */
#define fp30	$f30	/* callee saved 10 */
#define fp31	$f31	/* callee saved 11 */

#define C1_IR $0	/* implementation/revision reg */
#define C1_SR $31	/* control/status reg */

/*
* define aliases for operations that are different in 64bit mode
*/
#if (_WRS_INT_REGISTER_SIZE == 4)
#define SW	sw
#define LW	lw
#define MFC0	mfc0
#define MTC0	mtc0
#elif (_WRS_INT_REGISTER_SIZE == 8)
#define SW	sd		/* storing machine registers */
#define LW	ld		/* loading machine registers */
#define MFC0	dmfc0		/* reading wide cop0 register */
#define MTC0	dmtc0		/* writing wide cop0 register */
#else	/* _WRS_INT_REGISTER_SIZE */
#error "invalid _WRS_INT_REGISTER_SIZE value"
#endif	/* _WRS_INT_REGISTER_SIZE */

#if (_WRS_FP_REGISTER_SIZE == 4)
#define SWC1	swc1
#define LWC1	lwc1
#define MFC1	mfc1
#define MTC1	mtc1
#elif (_WRS_FP_REGISTER_SIZE == 8)
#define SWC1	sdc1
#define LWC1	ldc1
#define MFC1	dmfc1		/* reading wide fp register */
#define MTC1	dmtc1		/* writing wide fp register */
#else /* _WRS_FP_REGISTER_SIZE */
#error "invalid _WRS_FP_REGISTER_SIZE value"
#endif /* _WRS_FP_REGISTER_SIZE */

/* Hazard definitions */

/* A MIPS Hazard is defined as any combination of instructions which
 * would cause unpredictable behavior in terms of pipeline delays,
 * cache misses, and exceptions.  Hazards are defined by the number
 * of CPU cycles that must pass between certain combinations of
 * instructions.  Because some MIPS CPUs single-issue nop instructions
 * while others dual-issue, the CPU cycles defined below are done so
 * according to the instruction issue mechanism available.
 */
#define SINGLE_ISSUE 0 	
#define DUAL_ISSUE   1 
#define CPU_CYCLES              DUAL_ISSUE

/* Using the issue mechanism definitions above, the MIPS CPU cycles
 * are defined below.
 */

#if (CPU_CYCLES == SINGLE_ISSUE)
#define CPU_CYCLES_ONE          ssnop
#define CPU_CYCLES_TWO          ssnop;ssnop
#elif (CPU_CYCLES == DUAL_ISSUE)
#define CPU_CYCLES_ONE          ssnop;ssnop
#define CPU_CYCLES_TWO          ssnop;ssnop;ssnop;ssnop
#endif

/* Sixteen instructions are required to handle the VR5432 errata in
 * order to fill its instruction prefetch.  See HAZARD_VR5400 macro
 * for details.
 */
#define CPU_CYCLES_SIXTEEN      ssnop;ssnop;ssnop;ssnop; \
                                ssnop;ssnop;ssnop;ssnop; \
                                ssnop;ssnop;ssnop;ssnop; \
                                ssnop;ssnop;ssnop;ssnop				    

/* To assist with handling MIPS hazards, a number of categories of
 * hazards have been defined here.  
 *
 * HAZARD_TLB        After modifying tlb CP0 registers, do not use the
 *                   TLB for two CPU cycles.
 * HAZARD_ERET       After modifying the SR, do not return from an
 *                   exception for two CPU cycles.
 * HAZARD_INTERRUPT  After modifying the SR, interrupts do not lock
 *                   for two CPU cycles.
 * HAZARD_CP_READ    After a read from a Coprocessor register, the
 *                   result is not available for one CPU cycle.
 * HAZARD_CP_WRITE   After a write to a Coprocessor register, the
 *                   result is not effective for two CPU cycles.
 * HAZARD_CACHE_TAG  Cache TAG load and store instructions should
 *                   not be used withing one CPU cycle of modifying
 *                   the TAG registers.
 * HAZARD_CACHE      Cache instructions should not be used within 2
 *                   CPU cycles of each other.
 * HAZARD_VR5400     For the VR5432 CPU only.  Serialized instructions
 *                   (mtc0, ctc0, mfc0, cfc0, etc) must not appear within
 *                   16 instructions after a conditional branch or label.
 *
 * These hazard macros are intended for use with MIPS architecture-
 * dependent assembly files which require handling hazards.  For example,
 * suppose interrupts are being locked, to address the hazard, please
 * do the following:
 *
 * mtc0     t0, C0_SR
 * HAZARD_INTERRUPT
 * lw       t0, 0(a0)
 *
 * Similarly, when reading from a coprocessor register, please do the
 * following:
 *
 * mfc0     t0, C0_SR
 * HAZARD_CP_READ
 * and      t0, t0, t1
 * mtc0     t0, C0_SR
 * HAZARD_CP_WRITE
 *
 *
 * For more details on these categories, please refer to MIPS hazard
 * documentation.
 */

/* Hazard macros */
#define HAZARD_TLB       CPU_CYCLES_TWO
#define HAZARD_ERET      CPU_CYCLES_TWO
#define HAZARD_CP_READ   CPU_CYCLES_ONE
#define HAZARD_CP_WRITE  CPU_CYCLES_TWO
#define HAZARD_CACHE_TAG CPU_CYCLES_ONE
#define HAZARD_CACHE     CPU_CYCLES_TWO
#define HAZARD_INTERRUPT CPU_CYCLES_TWO

#ifdef _WRS_MIPS_VR5400_ERRATA
#define HAZARD_VR5400    CPU_CYCLES_SIXTEEN
#else
#define HAZARD_VR5400
#endif

/*
 * Stack frame allocation.
 * These macros are used in assembly language routines to automate the
 * allocation and use of stack frames.
 */

/* Return the size of the frame for the named routine */
#define FRAMESZ(routine)	_##routine##Fsize
/*
 * Calculate the frame size for the named routine
 * 4 register slots allocated for subroutines to store argument registers
 * 8 bytes are allocated for saving RA (independent of register size).
 * nregs register locations are reserved for storing locally used registers.
 * stack is kept 8-byte aligned.
 */
#define SETFRAME_EXTRA(routine,nregs,extra) \
	FRAMESZ(routine) = ((((_RTypeSize)*(4+(nregs)))+8+(extra)+7) & ~0x07)
#define SETFRAME(routine,nregs) \
	SETFRAME_EXTRA(routine,nregs,0)

/* The location at which to store the return address */
#define FRAMERA(routine) \
	(FRAMESZ(routine)-8)

/* Locations at which to store locally used registers */
#define FRAMER(routine,regn) \
	((_RTypeSize)*(4+(regn)))
#define FRAMER0(routine) FRAMER(routine,0)
#define FRAMER1(routine) FRAMER(routine,1)
#define FRAMER2(routine) FRAMER(routine,2)
#define FRAMER3(routine) FRAMER(routine,3)

/* Locations at which to store argument registers */
#define FRAMEA(routine, regn) \
	(FRAMESZ(routine)+(_RTypeSize)*(regn))
#define FRAMEA0(routine) FRAMEA(routine,0)
#define FRAMEA1(routine) FRAMEA(routine,1)
#define FRAMEA2(routine) FRAMEA(routine,2)
#define FRAMEA3(routine) FRAMEA(routine,3)

#ifdef __cplusplus
}
#endif

/* uC/OS-specific stuff from here on.. */
#define E_RA  0
#define E_AT  1
#define E_V0  2
#define E_V1  3
#define E_A0  4
#define E_A1  5
#define E_A2  6
#define E_A3  7
#define E_T0  8
#define E_T1  9
#define E_T2  10
#define E_T3  11
#define E_T4  12
#define E_T5  13
#define E_T6  14
#define E_T7  15
#define E_T8  16
#define E_T9  17
#define E_S0  18
#define E_S1  19
#define E_S2  20
#define E_S3  21
#define E_S4  22
#define E_S5  23
#define E_S6  24
#define E_S7  25
#define E_S8  26
#define E_HI  27
#define E_LO  28
#define E_EPC 29
#define E_SR  30
#define E_SIZE 32 /* must be even */

#ifdef LANGUAGE_C
/* Pointer to function with no arguments returning int */
typedef int (*PFI)(void);

/* Pointer to function with no arguments returning void */
typedef void (*PFV)(void);

/* Pointer to function with a pointer argument returning void */
typedef void (*PTV)(void *);

extern void    IRQEnable(int);
extern void    IRQDisable(int);
extern PFV     IRQInstall(int, PFV);
#endif

/* #define DRAM_SIZE          0x4000000*/
/* LED segment definitions.     */
#define LED_ON          0x80
#define LED_TOP_OFF     0x01
#define LED_RU_OFF      0x02 
#define LED_RL_OFF      0x04
#define LED_BOT_OFF     0x08
#define LED_LL_OFF      0x10
#define LED_LU_OFF      0x20
#define LED_MID_OFF     0x40

#define LED_A			(0x88)
#define LED_B			(0x80)
#define LED_C			(0xc6)
#define LED_D			(0xc0)
#define LED_E			(0x86)
#define LED_F			(0x8e)
#define LED_G			(0x82)
#define LED_H			(0x89)
#define LED_I			(0xf9)
#define LED_J			(0xf1)
#define LED_K			(0x89)
#define LED_L			(0xc7)
#define LED_M			(0x88)
#define LED_N			(0xc8)
#define LED_O			(0xc0)
#define LED_P			(0x8c)
#define LED_Q			(0xc0)
#define LED_R			(0x88)
#define LED_S			(0x92)
#define LED_T			(0xf8)
#define LED_U			(0xC1)
#define LED_V			(0xC1)
#define LED_W			(0x81)
#define LED_X			(0x89)
#define LED_Y			(0x91)
#define LED_Z			(0xa4)

#define LED_DASH		(0xbf)

#define LED_NUM0		(0xc0)
#define LED_NUM1		(0xf9)
#define LED_NUM2		(0xa4)
#define LED_NUM3		(0xb0)
#define LED_NUM4		(0x99)
#define LED_NUM5		(0x92)
#define LED_NUM6		(0x82)
#define LED_NUM7		(0xf8)
#define LED_NUM8		(0x80)
#define LED_NUM9		(0x98)


#define LEAF(name) \
        .text; \
        .globl  name; \
        .ent    name; \
name:

#define END(name) \
        .size name,.-name; \
	.end    name


#endif /* __INCasmMipsh */
