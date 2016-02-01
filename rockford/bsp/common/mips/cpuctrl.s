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
#ifndef _ASMLANGUAGE
#define _ASMLANGUAGE
#endif

#include "bmips.h"
#if defined(BMIPS3300)
    #include "bmips3300.h"
#elif defined(BMIPS4380)
    #include "bmips4380.h"
#elif defined(BMIPS5000)
    #include "bmips_5xxx.h"
#else
    #error error unknown CPU defined.
#endif
#include "mips_config.h"

/************************************************************************
 *  CpuIdGet() : return the MIPS CPU id number (NEC or QED)             
 ************************************************************************/
		.globl CpuIdGet
		.ent CpuIdGet
CpuIdGet:
		.set noreorder
		mfc0	v0,$15
		/*andi	v0,v0,0xff00*/
		nop
		jr	ra
		nop	
		.set reorder
		.end CpuIdGet

/************************************************************************
 *  CpuCountGet() : return the MIPS CPU Count register value             
 ************************************************************************/
		.globl CpuCountGet
		.ent CpuCountGet
CpuCountGet:
		.set noreorder
		mfc0	v0,$9
		nop
		jr	ra
		nop	
		.set reorder
		.end CpuCountGet
		
/************************************************************************
 *  CpuCountSet() : Set the MIPS CPU Count register value             
 ************************************************************************/
		.global CpuCountSet
		.ent CpuCountSet
CpuCountSet:
		.set noreorder
		mtc0	a0,$9
		nop
		jr	ra
		nop	
		.set reorder
		.end CpuCountSet


#if defined(BMIPS3300)

#define PM_16MB               0x01ffe000
#define BARRIER               ssnop; ssnop; ssnop

/************************************************************************
 *  AddTBLEntry(unsigned int entry_hi, unsigned int entry_lo_0, unsigned int entry_lo_1) :
 ************************************************************************/
		.global AddTBLEntry
		.ent AddTBLEntry
AddTBLEntry:
		.set noreorder

		addi	t2, a0, 0					
		addi	t3, a1, 0
		addi	t4, a2, 0					

		li	t0, PM_16MB
		mtc0	t0, CP0_PAGE_MASK
		mtc0	t2, CP0_ENTRY_HI

		mtc0	t3, CP0_ENTRY_LO_0
		mtc0	t4, CP0_ENTRY_LO_1

		mfc0	t0, CP0_WIRED
		mtc0	t0, CP0_INDEX
		addiu	t0, 1
		mtc0	t0, CP0_WIRED
		BARRIER
		tlbwi
		BARRIER

		jr		ra
		nop
	
		.set reorder
		.end AddTBLEntry

#else

#define PM_64MB               0x07ffe000
#define BARRIER               ssnop; ssnop; ssnop

/************************************************************************
 *  AddTBLEntry(unsigned int entry_hi, unsigned int entry_lo_0, unsigned int entry_lo_1) :
 ************************************************************************/
		.global AddTBLEntry
		.ent AddTBLEntry
AddTBLEntry:
		.set noreorder

		addi	t2, a0, 0					
		addi	t3, a1, 0
		addi	t4, a2, 0					

		li	t0, PM_64MB
		mtc0	t0, CP0_PAGE_MASK
		mtc0	t2, CP0_ENTRY_HI

		mtc0	t3, CP0_ENTRY_LO_0
		mtc0	t4, CP0_ENTRY_LO_1

		mfc0	t0, CP0_WIRED
		mtc0	t0, CP0_INDEX
		addiu	t0, 1
		mtc0	t0, CP0_WIRED
		BARRIER
		tlbwi
		BARRIER

		jr		ra
		nop
	
		.set reorder
		.end AddTBLEntry

#endif

/************************************************************************
 *  CpuCompareGet() : return the MIPS CPU Compare register value             
 ************************************************************************/
		.globl CpuCompareGet
		.ent CpuCompareGet
CpuCompareGet:
		.set noreorder
		mfc0	v0,$11
		nop
		jr	ra
		nop	
		.set reorder
		.end CpuCompareGet
		
/************************************************************************
 *  CpuCompareSet() : Set the MIPS CPU Compare register value             
 ************************************************************************/
		.globl CpuCompareSet
		.ent CpuCompareSet
CpuCompareSet:

		.set noreorder
		mtc0	a0,$11
		nop
		jr	ra
		nop	
		.set reorder
		.end CpuCompareSet
		
/************************************************************************
 *                                                                      *
 *     CpuIntGet()                                          
 *                                                              
 *     SYNTAX: 	unsigned long CpuIntGet(void)
 *     RETURNS: current imask value
 *              
 ************************************************************************/

		.globl CpuIntGet
		.ent CpuIntGet
CpuIntGet:
		.set noreorder

        mfc0    v0,$12		/* C0_STATUS */
        nop
        
        ori		t2,$0,0xfc00	/* imask mask */
        and		v0,v0,t2		/* mask off IM values */
        
        ori		t1,$0,10		/* imask offset */
		jr      ra
        srlv	v0,v0,t1		/* return the new imask */	
        

		.set	reorder
		.end CpuIntGet


/************************************************************************
 *                                                                      
 *     CpuIntSet()                                                   
 *                                                                      
 *     SYNTAX: 	unsigned long  CpuIntSet(unsigned long);             
 *		INPUT:	new Imask value
 *     RETURNS:	Current Imask value                           
 *
 *	NOTES: 
 *		NEC MIPS SR IM[7:0] 
 *			IM[0] and IM[1] are software interrupts and not accessible.
 *			IM[6:2] are wired to external devices
 *			IM[7] is the Internal Counter Compare Interrupt    
 *                                                                      
 ************************************************************************/
		.globl CpuIntSet
		.ent CpuIntSet

CpuIntSet:
		.set noreorder

        mfc0    t0,$12		/* C0_STATUS */
        nop
        
        ori		t1,$0,10		/* imask offset */
        sllv	a0,a0,t1		/* move to IM position */
        
        or		t0,t0,a0		/* create new SR value */
        
        ori		t2,$0,0xfc00	/* imask mask */
        and		v0,t0,t2		/* mask off IM values */
        beq		$0,v0,_cie1		/* IF some interrupt is enabled */
        nop						/* THEN enable them all */
        ori		t0,t0,1
        beq		$0,$0,_cie2
 		nop
_cie1:							/* ELSE disable them all */
		li		t2,~1
		and		t0,t0,t2
		       
_cie2:       
        mtc0    t0,$12		/* C0_STATUS */	/* write the new SR value */
		nop
		
		jr      ra
        srlv	v0,v0,t1		/* return the new imask */	
        
		.set	reorder
		.end CpuIntSet

#ifdef NO_OS_DIAGS
/*****************************************************************
 * InterruptHandler()
 *
 *	Located in ROM and RAM.  Override the behavior by changing 
 *  the RAM vector.  In order to share the exception processing 
 *  with MetroTRK, the overriding routine should decide whether 
 *  or not to handle the exception using ONLY the k0/k1 registers.
 *	Then, if the overriding routine is not going to handle the 
 *  exception, it should transfer control to this routine at its 
 *  ROM location using a branch or jump instruction.  It should 
 *  not use a 'jal' and should not expect the MetroTRK exception 
 *  handler to return control to it.
 *****************************************************************/
 		.align 4
		.comm ExGenRegs, 128;
		.comm ExCp0Regs, 128;
		.comm ExCp1Regs, 128;
		.set noat

		.globl InterruptHandler
		.ent InterruptHandler
InterruptHandler:

		.set noreorder
		/*
		 * first check IF this is an interrupt or exception
		 */
		addiu	sp,sp,-256
		sw	$0,(0*4)(sp)
		sw	$1,(1*4)(sp)
		sw	$2,(2*4)(sp)
		sw	$3,(3*4)(sp)
		sw	$4,(4*4)(sp)
		sw	$5,(5*4)(sp)
		sw	$6,(6*4)(sp)
		sw	$7,(7*4)(sp)
		sw	$8,(8*4)(sp)
		sw	$9,(9*4)(sp)
		sw	$10,(10*4)(sp)
		sw	$11,(11*4)(sp)
		sw	$12,(12*4)(sp)
		sw	$13,(13*4)(sp)
		sw	$14,(14*4)(sp)
		sw	$15,(15*4)(sp)
		sw	$16,(16*4)(sp)
		sw	$17,(17*4)(sp)
		sw	$18,(18*4)(sp)
		sw	$19,(19*4)(sp)
		sw	$20,(20*4)(sp)
		sw	$21,(21*4)(sp)
		sw	$22,(22*4)(sp)
		sw	$23,(23*4)(sp)
		sw	$24,(24*4)(sp)
		sw	$25,(25*4)(sp)
		sw	$26,(26*4)(sp)
		sw	$27,(27*4)(sp)
		sw	$28,(28*4)(sp)
		sw	$29,(29*4)(sp)
		sw	$30,(30*4)(sp)
		sw	$31,(31*4)(sp)	

		/* 
		 * Read the STATUS into t0
		 */
		mfc0	t0,$12		
		nop

#if 0   
		/* There was a delay between setting IE bit and reading it back.
		 * Therefore, reading IE bit and use it to determine if exception or
		 * interrupt occurred it not valid in some case.
		 * Alywas, rely on cause register
		 */
		ori		t3,$0,1
		and		t1,t0,t3		/* get the Intr Enable */
		beq		$0,t1,_ProcException	/* IF Interrupts aren't enabled */
		nop								/* THEN call ProcException() */
#endif										/* ELSE */
		ori		t3,$0,0xfe00
		sll		t3,t3,1			/* t3 = 0x0001fc00 (Interrupt Mask) */
		and		t2,t0,t3		/* get the Intr Mask */
		
		beq		$0,t2,_ProcException	/* IF no Interrupts are enabled */
		nop								/* THEN call ProcException() */
										/* ELSE */
		
		/* 
		 * Read the CAUSE into t1
		 */
		mfc0	t1,$13		
		nop
				
		srl		t2,t1,2
		ori		t3,$0,0x1f
		and		t2,t2,t3		/* get the ExcCode */
	
		bne		$0,t2,_ProcException	/* IF ExcCode is !Interrupt */
		nop								/* THEN call ProcException() */
		
		/*
		 * Process Interrupts 
		 *	t0 = SR
		 *	t1 = CAUSE 
		 */
_ProcInterrupt:		
		
		ori		t2,$0,(8+2)		/* Interrupt Offset */
		srlv	t0,t0,t2		/* normalized Status */
	
		srlv	t1,t1,t2		/* normalized Cause */	
	
		and		t2,t0,t1		/* find Enabled Pending interrupts */
							
		ori		t3,$0,(1<<(5))		/* IF IntrPending == Timer */			
		and		t3,t3,t2			
		beq		$0,t3,_NotTimer
		nop
				/*
				 * BCM7312 MIPS Timer 
				 */

#if 0		
		mfc0	t0,$9			/* read COUNT */
		lui		t1,0x1000		/* set new Compare value */
		addu	t0,t1,t0
		mtc0	t0,$11			/* write COMPARE to clear interrupt */
		nop
#endif
		jal		timer_int
		nop
		beq		$0,$0,_ExitProcInterrupt
		nop

_NotTimer:	
				/*
				 * BCM7312 internal interrupt. 
				 */
		/* t2 has the normalized int pending. t1 normalized SR, t0 normalized CAUSE.*/
		ori		t3,$0,1
		and		t3,t3,t2
		beq		$0,t3,_Not7312InternalInterrupt
		nop

      jal		CPUINT1_Isr
		nop
		beq		$0, $0, _ExitProcInterrupt
		nop
      
_Not7312InternalInterrupt:
#if 0   
      ori	t3,$0,1			/* shift right once to get rid of internal interrupt */
      srlv	t2,t2,t3

		ori	a0, t2, 0
		jal	BcmExtInterruptHandler
		nop
#endif
      
_ExitProcInterrupt:
		lw	$0,(0*4)(sp)   /* restore all registers */
		lw	$1,(1*4)(sp)
		lw	$2,(2*4)(sp)
		lw	$3,(3*4)(sp)
		lw	$4,(4*4)(sp)
		lw	$5,(5*4)(sp)
		lw	$6,(6*4)(sp)
		lw	$7,(7*4)(sp)
		lw	$8,(8*4)(sp)
		lw	$9,(9*4)(sp)
		lw	$10,(10*4)(sp)
		lw	$11,(11*4)(sp)
		lw	$12,(12*4)(sp)
		lw	$13,(13*4)(sp)
		lw	$14,(14*4)(sp)
		lw	$15,(15*4)(sp)
		lw	$16,(16*4)(sp)
		lw	$17,(17*4)(sp)
		lw	$18,(18*4)(sp)
		lw	$19,(19*4)(sp)
		lw	$20,(20*4)(sp)
		lw	$21,(21*4)(sp)
		lw	$22,(22*4)(sp)
		lw	$23,(23*4)(sp)
		lw	$24,(24*4)(sp)
		lw	$25,(25*4)(sp)
		lw	$26,(26*4)(sp)
		lw	$27,(27*4)(sp)
		lw	$28,(28*4)(sp)
		lw	$29,(29*4)(sp)
		lw	$30,(30*4)(sp)
		lw	$31,(31*4)(sp)	
		addiu	sp,sp,256
		
		/*
		 * return from the interrupt
		 */
		eret
		nop	
		
		
		/*****************************************************
		 * ELSE This is an Exception
		 *****************************************************/	
_ProcException:
		lw	$0,(0*4)(sp)   /* restore all registers */
		lw	$1,(1*4)(sp)
		lw	$2,(2*4)(sp)
		lw	$3,(3*4)(sp)
		lw	$4,(4*4)(sp)
		lw	$5,(5*4)(sp)
		lw	$6,(6*4)(sp)
		lw	$7,(7*4)(sp)
		lw	$8,(8*4)(sp)
		lw	$9,(9*4)(sp)
		lw	$10,(10*4)(sp)
		lw	$11,(11*4)(sp)
		lw	$12,(12*4)(sp)
		lw	$13,(13*4)(sp)
		lw	$14,(14*4)(sp)
		lw	$15,(15*4)(sp)
		lw	$16,(16*4)(sp)
		lw	$17,(17*4)(sp)
		lw	$18,(18*4)(sp)
		lw	$19,(19*4)(sp)
		lw	$20,(20*4)(sp)
		lw	$21,(21*4)(sp)
		lw	$22,(22*4)(sp)
		lw	$23,(23*4)(sp)
		lw	$24,(24*4)(sp)
		lw	$25,(25*4)(sp)
		lw	$26,(26*4)(sp)
		lw	$27,(27*4)(sp)
		lw	$28,(28*4)(sp)
		lw	$29,(29*4)(sp)
		lw	$30,(30*4)(sp)
		lw	$31,(31*4)(sp)	
		/*
		 * Store the Exception context to memory
		 */
		la	k0,ExGenRegs
		lui	k1,0xa000			/* force to non-cached */
		or	k0,k0,k1
		sw	$0,(0*4)(k0)
		sw	$1,(1*4)(k0)
		sw	$2,(2*4)(k0)
		sw	$3,(3*4)(k0)
		sw	$4,(4*4)(k0)
		sw	$5,(5*4)(k0)
		sw	$6,(6*4)(k0)
		sw	$7,(7*4)(k0)
		sw	$8,(8*4)(k0)
		sw	$9,(9*4)(k0)
		sw	$10,(10*4)(k0)
		sw	$11,(11*4)(k0)
		sw	$12,(12*4)(k0)
		sw	$13,(13*4)(k0)
		sw	$14,(14*4)(k0)
		sw	$15,(15*4)(k0)
		sw	$16,(16*4)(k0)
		sw	$17,(17*4)(k0)
		sw	$18,(18*4)(k0)
		sw	$19,(19*4)(k0)
		sw	$20,(20*4)(k0)
		sw	$21,(21*4)(k0)
		sw	$22,(22*4)(k0)
		sw	$23,(23*4)(k0)
		sw	$24,(24*4)(k0)
		sw	$25,(25*4)(k0)
		sw	$26,(26*4)(k0)
		sw	$27,(27*4)(k0)
		sw	$28,(28*4)(k0)
		sw	$29,(29*4)(k0)
		sw	$30,(30*4)(k0)
		sw	$31,(31*4)(k0)	
		
		la	k0,ExCp0Regs
		lui	k1,0xa000			/* force to non-cached */
		or	k0,k0,k1
		mfc0	v0,$12		/* C0_STATUS */
		nop
		sw	v0,(12*4)(k0)
		mfc0	v0,$13		/* C0_CAUSE */
		nop
		sw	v0,(13*4)(k0)
		mfc0	v0,$14		/* C0_EPC */
		nop
		sw	v0,(14*4)(k0)

		la	k0,ExCp1Regs
		lui	k1,0xa000			/* force to non-cached */
		or	k0,k0,k1
#if 0
		cfc1	v0,$0
		nop
		sw	v0,(0)(k0)
		cfc1	v0,$31
		nop
#endif

		sw	v0,(31*4)(k0)
		mfc0	v0,$12
		nop
		sw	v0,(1*4)(k0)
		
		
		/*
		 * Print the Exception Report
		 */
		la		a0,ExGenRegs
		lui		v0,0xa000		/* force to non-cached */
		or		a0,a0,v0
		la		a1,ExCp0Regs
		lui		v0,0xa000		/* force to non-cached */
		or		a1,a1,v0
		la		a2,ExCp1Regs
		lui		v0,0xa000		/* force to non-cached */
		or		a2,a2,v0
		
		jal		ExceptReport
		nop
				
ExceptHang:
		beq	$0,$0,ExceptHang
		nop

		.set	reorder
		.end InterruptHandler
#endif

#if 0
/****************************************************************
 * ExceptReport() : print an Exception Context to console and hang
 ****************************************************************/
void ExceptReport(unsigned long *GenRegs,unsigned long *Cp0Regs,unsigned long *Cp1Regs)
{
	printf("\n\r\n\nMIPS Exception\n\r");
	printf("*************************************************************\n\r");
	printf("STATUS=%08x\tBEV=%x\tIMask=%x\tIEnable=%x\n\r",
			Cp0Regs[(12)],
			(Cp0Regs[(12)]>>22)&1,
			(Cp0Regs[(12)]>>8)&0xff,
			(Cp0Regs[(12)])&1);
	printf("CAUSE=%08x ExcCode=%d\n\r",
			Cp0Regs[(13)],(Cp0Regs[(13)]>>2)&0x1f);
	
	printf("-------------------------------------------------------------\n\r");
	printf(" $0=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[0],GenRegs[1],GenRegs[2],GenRegs[3]);

	printf(" $4=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[4],GenRegs[5],GenRegs[6],GenRegs[7]);
	printf(" $8=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[8],GenRegs[9],GenRegs[10],GenRegs[11]);
	printf("$12=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[12],GenRegs[13],GenRegs[14],GenRegs[15]);
	printf("$16=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[16],GenRegs[17],GenRegs[18],GenRegs[19]);
	printf("$20=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[20],GenRegs[21],GenRegs[22],GenRegs[23]);
	printf("$24=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[24],GenRegs[25],GenRegs[26],GenRegs[27]);
	printf("$28=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[28],GenRegs[29],GenRegs[30],GenRegs[31]);

	printf("-------------------------------------------------------------\n\r");
	printf("EPC=0x%08x\t%08x\n\r\n\r",
			Cp0Regs[(14)],*(long*)Cp0Regs[(14)]);

	printf("-------------------------------------------------------------\n\r");
	printf("test : %08x\n\r", Cp1Regs[1]);
	printf("floating point CP1 FCR0 : %08x\n\r", Cp1Regs[0]);
	printf("floating point CP1 FCR31 : %08x\n\r\n\r\n\r", Cp1Regs[31]);

}
#endif

/**************************************************************** 
 * CpuStatusGet() : Return CPU Status Register 
 ****************************************************************/
		.globl CpuStatusGet
		.ent CpuStatusGet
CpuStatusGet:

	.set noreorder
	mfc0	v0,$12	/* get CPU Status Register */
	nop
	jr		ra
	nop

	.set reorder
	.end CpuStatusGet

/**************************************************************** 
 * CpuStatusSet() : Set CPU Status Register 
 ****************************************************************/
		.globl CpuStatusSet
		.ent CpuStatusSet
CpuStatusSet:

	.set noreorder
	mtc0	a0,$12	/* set CPU Status Register */
	nop
	jr		ra
	nop
	.set reorder
	.end CpuStatusSet

/**************************************************************** 
 * CpuCauseGet() : Return CPU Cause Register 
 ****************************************************************/
		.globl CpuCauseGet
		.ent CpuCauseGet
CpuCauseGet:

	.set noreorder
	mfc0	v0,$13	/* get CPU Cause Register */
	nop
	jr		ra
	nop

	.set reorder
	.end CpuCauseGet


/**************************************************************** 
 * CpuConfigGet() : Return CPU Config Register 
 ****************************************************************/
		.globl CpuConfigGet
		.ent CpuConfigGet
CpuConfigGet:

	.set noreorder
	mfc0	v0,$16	/* get CPU Config Register */
	nop
	jr		ra
	nop

	.set reorder
	.end CpuConfigGet

/**************************************************************** 
 * CpuConfigSet() : Set CPU Config Register 
 ****************************************************************/
		.globl CpuConfigSet
		.ent CpuConfigSet
CpuConfigSet:

	.set noreorder
	mtc0	a0,$16	/* set CPU Config Register */
	nop
	jr		ra
	nop
	.set reorder
	.end CpuConfigSet

/**************************************************************** 
 * CpuBevSet(val) : if val =1 then set BEV else clear BEV in CPU SR
 ****************************************************************/
		.globl CpuBevSet
		.ent CpuBevSet
CpuBevSet:

	.set 	noreorder
	mfc0	t0,$12	/* get CPU Status Register */
	nop
	
	beq	$0,a0,clearBEV	/* if val==1 	*/
	li	t1,0x00400000	/* then 		*/
	beq	$0,$0,endSetBEV
	or	t0,t0,t1		/* set BEV bit in status */
	
clearBEV:				/* else			*/
	nor	t1,t1,t1			/* clear BEV bit in status */
	and	t0,t0,t1
	
endSetBEV:
	mtc0	t0,$12	/* set CPU Status Register */
	jr	ra
	nop
	.set	reorder
	.end CpuBevSet

/**************************************************************** 
 * CpuBusPllSet() : Set CPU BUS PLL Register 
 ****************************************************************/
		.globl CpuBusPllSet
		.ent CpuBusPllSet
CpuBusPllSet:

	.set noreorder
/*	mtc0	a0,$22*/	/* set CPU Config Register */
	.word 0x4084B004	/* writing a0 to CO 22(BUS PLL with select set to 4 */
	nop
	jr		ra
	nop
	.set reorder
	.end CpuBusPllSet

/**************************************************************** 
 * CpuModeGet() : Return CPU Mode Register 
 ****************************************************************/
		.globl CpuModeGet
		.ent CpuModeGet
CpuModeGet:

	.set noreorder
	mfc0	v0,$22,1	/* get CPU Config Register */
	nop
	jr		ra
	nop

	.set reorder
	.end CpuModeGet

/**************************************************************** 
 * CpuCbrGet() : Return CPU CBR Register 
 ****************************************************************/
		.globl CpuCbrGet
		.ent CpuCbrGet
CpuCbrGet:

	.set noreorder
	mfc0	v0,$22,6	/* get CPU CBR Register */
	nop
	jr		ra
	nop

	.set reorder
	.end CpuCbrGet

