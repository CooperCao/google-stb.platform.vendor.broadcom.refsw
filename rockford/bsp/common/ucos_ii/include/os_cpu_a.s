/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
 ***************************************************************************/
#include "bcm_mips.h"
#include "os_cfg.h"

#ifdef NO_L1
    #define ST0_IM_MASK 0x8000
#else
    #define ST0_IM_MASK 0x8400
#endif

#define TLB_REFILL_EXCEPTION_VECTOR 0xa0000000
#define GEN_EXCEPTION_VECTOR 0xa0000180
#define TIMER_IRQ_VECTOR	0xa0000200
#define INITIAL_SR      (ST0_CU0)
#define TIMER_PERIOD (HW_TICKS_PER_SEC/(OS_TICKS_PER_SEC))

#define EXPT_STACK_SIZE 0x1000

	.comm expt_stack, EXPT_STACK_SIZE, 0x4
	.set noreorder

	.text 
        .extern OSRunning
        .extern OSTaskSwHook
        .extern OSTCBHighRdy
/*
*********************************************************************************************************
*                                          DISABLE INTERRUPTS
*                                   OS_CPU_SR  OS_CPU_SR_Save(void);
*
* Description: This function saves the state of the Status register and then disables interrupts via this
*              register.  This objective is accomplished with a single instruction, di.  The di 
*              instruction's operand, $2, is the general purpose register to which the Status register's 
*              value is saved.  This value can be read by C functions that call OS_CPU_SR_Save().  
*
* Arguments  : None
*
* Returns    : The previous state of the Status register
*********************************************************************************************************
*/
	.global OS_CPU_SR_Save
    .ent OS_CPU_SR_Save
OS_CPU_SR_Save:

#if 0
    jr    $31
    di    $2                                   /* Disable interrupts, and move the old value of the... */
                                               /* ...Status register into v0 ($2)                      */
#else
	mfc0	v0,$12	/* get CPU Status Register */
	nop
	nop
	and		t0,v0,~1
	mtc0    t0,$12
	nop
	nop
    jr    $31
	nop
#endif
    .end OS_CPU_SR_Save

/*
*********************************************************************************************************
*                                          ENABLE INTERRUPTS
*                                  void OS_CPU_SR_Restore(OS_CPU_SR sr);
*
* Description: This function must be used in tandem with OS_CPU_SR_Save().  Calling OS_CPU_SR_Restore()
*              causes the value returned by OS_CPU_SR_Save() to be placed in the Status register. 
*
* Arguments  : The value to be placed in the Status register
*
* Returns    : None
*********************************************************************************************************
*/
	.global OS_CPU_SR_Restore
    .ent OS_CPU_SR_Restore
OS_CPU_SR_Restore:

    jr    $31
    mtc0  $4, $12, 0                           /* Restore the status register to its previous state    */

    .end OS_CPU_SR_Restore


/*
*********************************************************************************************************
*                                           OSStartHighRdy()
*
* Description: Starts the highest priority task that is available to run.  OSStartHighRdy() MUST:
*
*              a) Call OSTaskSwHook()
*              b) Set OSRunning to TRUE
*              c) Switch to the highest priority task, and restore the epc and ao resgister
*                              
* Note(s): 1) OSTaskStkInit(), which is responsible for initializing each task's stack, sets bit 0 of the
*             entry corresponding to the Status register.  Thus, interrupts will be enabled when each
*             task first runs.
*********************************************************************************************************
*/
        .global OSStartHighRdy
        .ent OSStartHighRdy
OSStartHighRdy:
#if OS_TASK_SW_HOOK_EN > 0
        jal OSTaskSwHook
        nop
#endif
        la      t0, OSRunning        # store OS_TRUE
        li      t1, 1                # in OSRunning variable
        sw      t1, 0(t0)

		/* program the timer interrupt */
        mfc0    t0, $9
#if 0
        ehb
#else
		nop
		nop
		nop
		nop
		nop
		nop
#endif
        addu    t0, TIMER_PERIOD
        mtc0    t0, $11
#if 0
        ehb
#else
		nop
		nop
		nop
		nop
		nop
		nop
#endif
        /* enable the interrupt */
        mfc0    t0, $12
#if 0
        ehb
#else
		nop
		nop
		nop
		nop
		nop
		nop
#endif
        nop
        nop
        nop
        and     t0, ~(ST0_ERL | ST0_EXL)
        or      t0, ST0_IE | ST0_IM_MASK
        mtc0    t0, $12
#if 0
        ehb
#else
		nop
		nop
		nop
		nop
		nop
		nop
#endif
        nop

        /* load stack pointer of a ready task */
        la      t0, OSTCBHighRdy       /* Update the current TCB   */
        lw      sp, 0(t0)              
        lw      sp, 0(sp)			   /* Load the new task's stack pointer  */
        lw      t1, ROFF_PC*4(sp)      /* Restore the EPC   */
        mtc0    t1, CP0_EPC
        /* load parameter passed in a0 register */
        lw      a0, 4*4(sp)
        add     sp, sp, ROFF_REG_SPACE
        eret
        nop
        .end OSStartHighRdy


        .global OSInitVectors
        .ent OSInitVectors
OSInitVectors:
#if 0
        li      t0, TLB_REFILL_EXCEPTION_VECTOR
        la      t1, gen_exception
        lw      t2, 0(t1)
        sw      t2, 0(t0)
        lw      t2, 4(t1)
        sw      t2, 4(t0)
#endif
        li      t0, GEN_EXCEPTION_VECTOR
        la      t1, gen_exception
        lw      t2, 0(t1)
        sw      t2, 0(t0)
        lw      t2, 4(t1)
        sw      t2, 4(t0)
		/* set timer irq vector. */
		li		t0, TIMER_IRQ_VECTOR
		la		t1, gen_exception
		lw		t2, 0(t1)
		sw		t2, 0(t0)
		lw		t2, 4(t1)
		sw		t2, 4(t0)
		/* end */
        li      t0, INITIAL_SR
        mtc0    t0, CP0_STATUS  # redirect exception vectors to memory
        jr      ra
        nop
        
        
        
        
gen_exception:
        j       mips_exception
        nop
        .end OSInitVectors



        .extern OSTCBCur
        .extern OSExceptionHandler
mips_exception:
        .set    noreorder
        .set    noat
        move    k1, sp
        subu    k1, k1, ROFF_REG_SPACE

        /* save registers */
        BCM_SAVE(k1)
           
		la		t2, OSTCBCur                      /* Save the current task's stack pointer                */
		lw		t3, 0(t2)
		sw		k1, 0(t3)
	
        la		sp, expt_stack + (EXPT_STACK_SIZE - 8)
        move    a0, k1							/* a0 is the offset the base register */
        jal     OSExceptionHandler
        nop
        
        /* restore registers */
		la		t0, OSTCBCur
		lw		t1, 0(t0)
		lw		k1, 0(t1)
        BCM_RESTORE(k1)
        .set    at
        eret
        nop
        
