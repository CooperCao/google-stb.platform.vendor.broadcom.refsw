/***************************************************************************
 *     (c)2003-2013 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
/*
*********************************************************************************************************
*                                               uC/OS-III
*                                         The Real-Time Kernel
*
*                          (c) Copyright 1992-2010, Micrium, Inc., Weston, FL
*                                          All Rights Reserved
*
*                                              MIPS32 4K
*
*                                                MPLAB
*
* File   : os_cpu_c.c
* Version: v3.01
*********************************************************************************************************
*/

#include <stdio.h>
#include "bstd.h"
#include <os.h>
#include <os_cfg_app.h>
#include "bcm_mips.h"
#include "int1.h"
#ifdef USE_WEBHIF_TIMER
    #include "bchp_hif_cpu_intr1.h"
    #include "bchp_webhif_timer.h"
#endif
#include "bsu-api.h"
#include "bsu-api2.h"

unsigned int cp0_count_clocks_per_os_tick;

/* Initial stack frame size for OSTaskCreate()(incl. args passed along to the task being created) */
/* Note: OS_FRAME_SIZE needs to be a multiple of 2 (see OSTaskStkInit)*/
/* Note: add +4 in case call to OSTaskCreate is erroneously passed address beyond the stack's first (last) valid mem. loc. */
#if OS_STK_GROWTH != 1u
  #error OSTaskStkInit() is implemented assuming OS_STK_GROWTH == 1
#else
  /* Add +1 assuming that top of stack may have been passed using (invalid) addr just beyond top(end) of allocated stack space */
  #define OS_FRAME_SIZE (ROFF_NUM_REG + 1)
#endif

/*
************************************************************************************************************************
*                                                OS INITIALIZATION HOOK
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : none
*
* Note(s)    : none
************************************************************************************************************************
*/

#ifdef USE_WEBHIF_TIMER
void TimerInit(void)
{
    *((volatile int *)(0xb0000000 | BCHP_WEBHIF_TIMER_TIMER_IE0)) = 0; /* disable timer 0 */
    *((volatile int *)(0xb0000000 | BCHP_WEBHIF_TIMER_TIMER0_CTRL)) = 0;
    *((volatile int *)(0xb0000000 | BCHP_WEBHIF_TIMER_TIMER_IS)) = 0;
    *((volatile int *)(0xb0000000 | BCHP_WEBHIF_TIMER_TIMER0_CTRL)) = ((1 << 31) | (1 << 30) | 270000); /* 10 ms */
    *((volatile int *)(0xb0000000 | BCHP_WEBHIF_TIMER_TIMER_IE0)) = 1; /* enable timer 0 */
}

void TimerDisable(void)
{
    *((volatile int *)(0xb0000000 | BCHP_WEBHIF_TIMER_TIMER_IE0)) = 0; /* disable timer 0 */
    *((volatile int *)(0xb0000000 | BCHP_WEBHIF_TIMER_TIMER0_CTRL)) = 0;
    *((volatile int *)(0xb0000000 | BCHP_WEBHIF_TIMER_TIMER_IS)) = 0;
}

void TimerEnable(void)
{
    *((volatile int *)(0xb0000000 | BCHP_WEBHIF_TIMER_TIMER0_CTRL)) = ((1 << 31) | (1 << 30) | 270000); /* 10 ms */
    *((volatile int *)(0xb0000000 | BCHP_WEBHIF_TIMER_TIMER_IE0)) = 1; /* enable timer 0 */
}
#endif

void OSInitHook(void)
{
    OSInitVectors();
#ifdef USE_WEBHIF_TIMER
    *((volatile int *)(0xb0000000 | BCHP_HIF_CPU_INTR1_INTR_W2_MASK_CLEAR)) = (1 << 24);      /* Clear L1 mask */
    TimerInit();
#endif
}

/*
************************************************************************************************************************
*                                                  TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : p_tcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : none
************************************************************************************************************************
*/

void  OSTaskCreateHook (OS_TCB  *p_tcb)
{
#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppTaskCreateHookPtr != (OS_APP_HOOK_TCB)0) {
        (*OS_AppTaskCreateHookPtr)(p_tcb);
    }
#else
    (void)p_tcb;                                            /* Prevent compiler warning                               */
#endif
}

/*
************************************************************************************************************************
*                                                   TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : p_tcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : none
************************************************************************************************************************
*/

void  OSTaskDelHook (OS_TCB  *p_tcb)
{
#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppTaskDelHookPtr != (OS_APP_HOOK_TCB)0) {
        (*OS_AppTaskDelHookPtr)(p_tcb);
    }
#else
    (void)p_tcb;                                            /* Prevent compiler warning                               */
#endif
}

/*
************************************************************************************************************************
*                                                   IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do such things as
*              STOP the CPU to conserve power.
*
* Arguments  : none
*
* Note(s)    : none
************************************************************************************************************************
*/

void OSIdleTaskHook(void)
{
}

/*
************************************************************************************************************************
*                                                 STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-III's statistics task.  This allows your application to add
*              functionality to the statistics task.
*
* Arguments  : none
*
* Note(s)    : none
************************************************************************************************************************
*/

void  OSStatTaskHook (void)
{
#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppStatTaskHookPtr != (OS_APP_HOOK_VOID)0) {
        (*OS_AppStatTaskHookPtr)();
    }
#endif
}

/*
*********************************************************************************************************
*                                        INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created.  This function is processor-specific.
*
* Arguments  : task     is a pointer to the task code.
*
*              p_arg    is a pointer to a user supplied data area 
*
*              ptos     is a pointer to the top of stack.  OSTaskStkInit() assumes that 'ptos' points to 
*                       a free entry on the stack.  If OS_STK_GROWTH is set to 1 then 'ptos' will contain 
*                       the HIGHEST valid address of the stack.  Similarly, if OS_STK_GROWTH is set to 0, 
*                       'ptos' will contain the lowest valid address of the stack.
*
*              opt      specifies options that can be used to alter the behavior of OSTaskStkInit()
*                       (see ucos_ii.h for OS_TASK_OPT_*).
*
* Returns    : The location corresponding to the top of the stack
*
* Note(s)    : 1) Interrupts are enabled when each task starts executing.
* 
*              2) An initialized stack has the structure shown below.
*
*              OSTCBHighRdy->OSTCBStkPtr + 0x00    Free Entry                    (LOW Memory)
*                                        + 0x04    Status Register
*                                        + 0x08    EPC
*                                        + 0x0C    Special Purpose LO Register
*                                        + 0x10    Special Purpose HI Register
*                                        + 0x14    GPR[1]
*                                        + 0x18    GPR[2]
*                                        + 0x1C    GPR[3]
*                                        + 0x20    GPR[4]
*                                        + 0x24    GPR[5]
*                                        + 0x28    GPR[6]
*                                        + 0x2C    GPR[7]
*                                        + 0x30    GPR[8]
*                                        + 0x34    GPR[9]
*                                        + 0x38    GPR[10]
*                                        + 0x3C    GPR[11]
*                                        + 0x40    GPR[12]
*                                        + 0x44    GPR[13]
*                                        + 0x48    GPR[14]
*                                        + 0x4C    GPR[15]
*                                        + 0x50    GPR[16]
*                                        + 0x54    GPR[17]
*                                        + 0x58    GPR[18]
*                                        + 0x5C    GPR[19]
*                                        + 0x60    GPR[20]
*                                        + 0x64    GPR[21]
*                                        + 0x68    GPR[22]
*                                        + 0x6C    GPR[23]
*                                        + 0x70    GPR[24]
*                                        + 0x74    GPR[25]
*                                        + 0x78    GPR[26]
*                                        + 0x7C    GPR[27]
*                                        + 0x80    GPR[28]
*                                        + 0x84    GPR[30]
*                                        + 0x88    GPR[31]                       (HIGH Memory)
*********************************************************************************************************
*/

CPU_STK * OSTaskStkInit(OS_TASK_PTR           p_task,
                        void                  *p_arg,
                        CPU_STK               *p_stk_base,
                        CPU_STK               *p_stk_limit,
                        CPU_STK_SIZE           stk_size,
                        OS_OPT                 opt)
{
    CPU_STK * new_top;
    int i;
    (void)p_stk_limit;
    (void)opt;
    new_top = (p_stk_base + stk_size - 1) - OS_FRAME_SIZE;
    /* Clear from the pos. just above the top of stack down to very bottom of stack */
    for(i = 0; i < OS_FRAME_SIZE; i+=2){
        new_top[i] = 0;
        new_top[i+1] = 0;
    }
    /* place p_arg onto the stack's a0 (i.e. $r4) */
    new_top[4] = (OS_STK)p_arg;
    /* place the task start addr. onto the stack */
    new_top[ROFF_PC] = (OS_STK)p_task;
    /* place the sp's context switch restore value */
    new_top[ROFF_SP] = (OS_STK)(p_stk_base + stk_size - 1);

    return new_top;
}

/*
************************************************************************************************************************
*                                                   TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other operations
*              during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdyPtr' points to the TCB of the task that will be
*                 'switched in' (i.e. the highest priority task) and, 'OSTCBCurPtr' points to the task being switched out
*                 (i.e. the preempted task).
************************************************************************************************************************
*/

void OSTaskSwHook(void)
{
}

/*$PAGE*/
/*
************************************************************************************************************************
*                                                      TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) This function is assumed to be called from the Tick ISR.
************************************************************************************************************************
*/

void  OSTimeTickHook (void)
{
#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppTimeTickHookPtr != (OS_APP_HOOK_VOID)0) {
        (*OS_AppTimeTickHookPtr)();
    }
#endif
}

/*$PAGE*/
/*
************************************************************************************************************************
*                                                   TASK RETURN HOOK
*
* Description: This function is called if a task accidentally returns.  In other words, a task should either be an
*              infinite loop or delete itself when done.
*
* Arguments  : p_tcb   is a pointer to the task control block of the task that is returning.
*
* Note(s)    : none
************************************************************************************************************************
*/

void  OSTaskReturnHook (OS_TCB  *p_tcb)
{
    (void)p_tcb;
}


void OSUnhandledException(register2_t * registers);
void OSSysCallException(register2_t * registers);
void OSInterrupt(register2_t * registers);

static exception_handler_t handler[32] = {
    OSInterrupt,                /* int */
    OSUnhandledException,       /* Mod */
    OSUnhandledException,       /* TLBL */
    OSUnhandledException,       /* TLBS */
    OSUnhandledException,       /* AdEL */
    OSUnhandledException,       /* AdES */
    OSUnhandledException,       /* IBE */
    OSUnhandledException,       /* DBE */
    OSSysCallException,         /* Sys */
    OSUnhandledException,       /* Bp */
    OSUnhandledException,       /* RI */
    OSUnhandledException,       /* CpU */
    OSUnhandledException,       /* Ov */
    OSUnhandledException,       /* Tr */
    OSUnhandledException,       /* 14 */
    OSUnhandledException,       /* FPE */
    OSUnhandledException,       /* 16 */
    OSUnhandledException,       /* 17 */
    OSUnhandledException,       /* C2E */
    OSUnhandledException,       /* 19 */
    OSUnhandledException,       /* 20 */
    OSUnhandledException,       /* 21 */
    OSUnhandledException,       /* 22 */
    OSUnhandledException,       /* WATCH */
    OSUnhandledException,       /* MCheck */
    OSUnhandledException,       /* 25 */
    OSUnhandledException,       /* 26 */
    OSUnhandledException,       /* 27 */
    OSUnhandledException,       /* 28 */
    OSUnhandledException,       /* 29 */
    OSUnhandledException,       /* CacheErr */
    OSUnhandledException        /* 31 */
};

void OSExceptionHandler(register2_t * registers)
{
    register2_t cause, status;
    status = registers[ROFF_SR];
    cause = registers[ROFF_CAUSE];
    /* Get exception number and call corresponding handler */
    cause = (cause >> 2) & 0x1f;
    if((0 == cause) && (0 == (status & ST0_IE))){
        /* Got interrupt while entering critical section. Just return. */
        return;
    }
    
    if (cause)
    {
        if (cause != 8) /* Syscalls are allowed */
        {
            printf("Cause exception code:  %d  \n", (int)cause);
            switch(cause)
            {
                case 0: printf("(int\n"); break;
                case 1: printf("(TLB mod\n"); break;
                case 2: printf("(TLB refill - load or instr fetch)\n"); break;
                case 3: printf("(TLB refill - store)\n"); break;
                case 4: printf("(Address error - load or instr fetch)\n"); break;
                case 5: printf("(Address error - store)\n"); break;
                case 6: printf("(Bus error - instr fetch)\n"); break;
                case 7: printf("(Bus error - data ref: load or store)\n"); break;
                case 8: printf("(Syscall)\n"); break;
                case 9: printf("(Breakpoint)\n"); break;
                case 10: printf("(Reserved instr)\n"); break;
                case 11: printf("(Coprocessor unusable)\n"); break;
                case 12: printf("(Integer overflow)\n"); break;
                case 13: printf("(Trap)\n"); break;
                case 14: printf("(Reserved)\n"); break;
                case 15: printf("(Floating point)\n"); break;
            }
            
            printf("zero (r00):  0x%08x\n", (int)registers[0]);
            printf("at (r01):  0x%08x\n", (int)registers[1]);
            printf("v0 (r02):  0x%08x\n", (int)registers[2]);
            printf("v1 (r03):  0x%08x\n", (int)registers[3]);
            printf("a0 (r04):  0x%08x\n", (int)registers[4]);
            printf("a1 (r05):  0x%08x\n", (int)registers[5]);
            printf("a2 (r06):  0x%08x\n", (int)registers[6]);
            printf("a3 (r07):  0x%08x\n", (int)registers[7]);
            printf("t0 (r08):  0x%08x\n", (int)registers[8]);
            printf("t1 (r09):  0x%08x\n", (int)registers[9]);
            printf("t2 (r10):  0x%08x\n", (int)registers[10]);
            printf("t3 (r11):  0x%08x\n", (int)registers[11]);
            printf("t4 (r12):  0x%08x\n", (int)registers[12]);
            printf("t5 (r13):  0x%08x\n", (int)registers[13]);
            printf("t6 (r14):  0x%08x\n", (int)registers[14]);
            printf("t7 (r15):  0x%08x\n", (int)registers[15]);
            printf("s0 (r16):  0x%08x\n", (int)registers[16]);
            printf("s1 (r17):  0x%08x\n", (int)registers[17]);
            printf("s2 (r18):  0x%08x\n", (int)registers[18]);
            printf("s3 (r19):  0x%08x\n", (int)registers[19]);
            printf("s4 (r20):  0x%08x\n", (int)registers[20]);
            printf("s5 (r21):  0x%08x\n", (int)registers[21]);
            printf("s6 (r22):  0x%08x\n", (int)registers[22]);
            printf("s7 (r23):  0x%08x\n", (int)registers[23]);
            printf("t8 (r24):  0x%08x\n", (int)registers[24]);
            printf("t9 (r25):  0x%08x\n", (int)registers[25]);
            printf("gp (r28):  0x%08x\n", (int)registers[28]);
            printf("sp (r29):  0x%08x\n", (int)registers[29]);
            printf("s8 (r30):  0x%08x\n", (int)registers[30]);
            printf("ra (r31):  0x%08x\n", (int)registers[31]);
            printf("sr:  0x%08x\n", (int)registers[32]);
            printf("lo:  0x%08x\n", (int)registers[33]);
            printf("hi:  0x%08x\n", (int)registers[34]);
            printf("badvaddr:  0x%08x\n", (int)registers[35]);
            printf("ra:  0x%08x\n", (int)registers[36]);
            printf("epc:  0x%08x\n", (int)registers[37]);
            printf("start pc:  0x%08x\n", (int)registers[38]);
            printf("error pc:  0x%08x\n", (int)registers[39]);
            printf("ra:  0x%08x\n", (int)registers[40]);
            
            for (;;){}
        }
    }
    
    (*handler[cause])(registers);
}

void OSUnhandledException(register2_t * registers)
{
	BSTD_UNUSED(registers);
    printf("OSUnhandledException !!!\n");
    for(;;);
}

void OSIntCtxSw(void)
{
#if OS_TASK_SW_HOOK_EN > 0
    OSTaskSwHook();
#endif
    OSTCBCurPtr = OSTCBHighRdyPtr;
    OSPrioCur = OSPrioHighRdy;

}

void OSSysCallException(register2_t * registers)
{
    register2_t syscall;
    syscall = *((register2_t*)(registers[ROFF_PC]));
    syscall >>= 6;
    if(0xb == syscall){
        registers[ROFF_PC] += 4; /* advance past syscall */
        OSIntCtxSw();
    }
}

typedef void(*Handler_t)(void);

/*static Handler_t ihandler[2];*/

void init_os_hw_ticks(unsigned int cp0_count_clocks_per_sec)
{
	cp0_count_clocks_per_os_tick = cp0_count_clocks_per_sec / OS_CFG_TICK_RATE_HZ;
}

void OSInterrupt(register2_t * registers)
{
    register2_t cause, sr;
    /* Cause reg. IP bits */
    cause = registers[ROFF_CAUSE];
    sr = registers[ROFF_SR];

    OSIntEnter();
    /* Cause reg. IP bits */
    cause = (cause & sr) & 0xFF00u;
#ifndef USE_WEBHIF_TIMER
    if(cause & 0x8000u){         	/* timer interrupt */
        #ifdef DO_NOT_USE_FREE_RUNNING_COUNTER
            int reset = 0;
            __asm__ volatile("mtc0   %0,$11" : "+r"(cp0_count_clocks_per_os_tick));           /* Set up the period in the compare reg    */
            __asm__("nop");
            __asm__ volatile("mtc0   %0,$9" : "+r"(reset));           /* Set up the period in the compare reg    */
            __asm__("nop");
        #else
            register2_t old_period, period;
            period = cp0_count_clocks_per_os_tick;
            __asm__ volatile("mfc0   %0, $11" : "=r"(old_period));      /* Get the old compare time                */
            period += old_period;
            __asm__ volatile("mtc0   %0,$11" : "+r"(period));           /* Set up the period in the compare reg    */
            __asm__("nop");
        #endif
        OSTimeTick();
    }
#endif
    if(0x0400u & cause){    		/* chip interrupt */
#ifdef USE_WEBHIF_TIMER
        unsigned int w2_status =  *((volatile int *)(0xb0000000 | BCHP_HIF_CPU_INTR1_INTR_W2_STATUS));
        unsigned int w2_mask =  *((volatile int *)(0xb0000000 | BCHP_HIF_CPU_INTR1_INTR_W2_MASK_STATUS));

        /* Check to see if timer int first */
        if (w2_status & (1 << 24) & ~w2_mask) {
            TimerDisable();
            OSTimeTick();
            TimerEnable();
        } else {
            CPUINT1_Isr();
        }
#else
        CPUINT1_Isr();
#endif
    }
    if(~0x8400 & cause){            /* Unhandled external or S/W interrupt */
        OSUnhandledException(registers);            
    }
    OSIntExit();
}


INT8U OSRegisterException(INT8U vector, exception_handler_t new_handler, exception_handler_t ** old_handler)
{
    INT8U err;
    err = OS_ERR_NONE;
    if(vector < 32){
        if(0 != old_handler){
            *old_handler = (exception_handler_t *)handler[vector];
        }
        handler[vector] = new_handler;
    }else{
        err = OS_ERR_OPT_INVALID;
    }
    return err;
}

