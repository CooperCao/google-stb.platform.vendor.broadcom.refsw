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
#include <stdio.h>
#include "bstd.h"
#include "ucos_ii.h"
#include "bcm_mips.h"
#include "int1.h"
#ifdef NO_L1
#include "bchp_hif_cpu_intr1.h"
#endif

unsigned int cp0_count_clocks_per_os_tick;

#ifdef NO_L1
int l1_interrupt()
{
    uint32_t w0_status = *((volatile uint32_t *)(0xb0000000 | BCHP_HIF_CPU_INTR1_INTR_W0_STATUS));
    uint32_t w0_mask_status = *((volatile uint32_t *)(0xb0000000 | BCHP_HIF_CPU_INTR1_INTR_W0_MASK_STATUS));
    uint32_t w1_status = *((volatile uint32_t *)(0xb0000000 | BCHP_HIF_CPU_INTR1_INTR_W1_STATUS));
    uint32_t w1_mask_status = *((volatile uint32_t *)(0xb0000000 | BCHP_HIF_CPU_INTR1_INTR_W1_MASK_STATUS));
    uint32_t w2_status = *((volatile uint32_t *)(0xb0000000 | BCHP_HIF_CPU_INTR1_INTR_W2_STATUS));
    uint32_t w2_mask_status = *((volatile uint32_t *)(0xb0000000 | BCHP_HIF_CPU_INTR1_INTR_W2_MASK_STATUS));
    uint32_t w3_status = *((volatile uint32_t *)(0xb0000000 | BCHP_HIF_CPU_INTR1_INTR_W3_STATUS));
    uint32_t w3_mask_status = *((volatile uint32_t *)(0xb0000000 | BCHP_HIF_CPU_INTR1_INTR_W3_MASK_STATUS));
    return (w0_status & ~w0_mask_status) || (w1_status & ~w1_mask_status) || (w2_status & ~w2_mask_status) || (w3_status & ~w3_mask_status);
}
#endif

/* Initial stack frame size for OSTaskCreate()(incl. args passed along to the task being created) */
/* Note: OS_FRAME_SIZE needs to be a multiple of 2 (see OSTaskStkInit)*/
/* Note: add +4 in case call to OSTaskCreate is erroneously passed address beyond the stack's first (last) valid mem. loc. */
#if OS_STK_GROWTH != 1u
  #error OSTaskStkInit() is implemented assuming OS_STK_GROWTH == 1
#else
  /* Add +1 assuming that top of stack may have been passed using (invalid) addr just beyond top(end) of allocated stack space */
  #define OS_FRAME_SIZE (ROFF_NUM_REG + 1)
#endif

OS_STK * OSTaskStkInit(void(*task)(void *p_arg), void *p_arg, OS_STK *ptos, INT16U opt)
{
    OS_STK * new_top;
    int i;
	BSTD_UNUSED(opt);
    new_top = ptos - OS_FRAME_SIZE;
    /* Clear from the pos. just above the top of stack down to very bottom of stack */
    for(i = 0; i < OS_FRAME_SIZE; i+=2){
        new_top[i] = 0;
        new_top[i+1] = 0;
    }
    /* place p_arg onto the stack's a0 (i.e. $r4) */
    new_top[4] = (OS_STK)p_arg;
    /* place the task start addr. onto the stack */
    new_top[ROFF_PC] = (OS_STK)task;
    /* place the sp's context switch restore value */
    new_top[ROFF_SP] = (OS_STK)ptos;

    return new_top;
}
#if OS_DEBUG_EN > 0
void OSDebugInit(void)
{
}
#endif
#if OS_TASK_RETURN_HOOK == 1
void  OSTaskReturnHook (OS_TCB *ptcb)
{
	BSTD_UNUSED(ptcb);
}
#endif
#if OS_CPU_HOOKS_EN > 0
void OSTaskCreateHook(OS_TCB *ptcb)
{
	BSTD_UNUSED(ptcb);
}
#endif

#if OS_CPU_HOOKS_EN > 0
void OSTaskDelHook(OS_TCB *ptcb)
{
	BSTD_UNUSED(ptcb);
}
#endif

#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0)
void OSTaskSwHook(void)
{
}
#endif

#if OS_CPU_HOOKS_EN > 0
void OSTaskStatHook(void)
{
}
#endif

#if (OS_CPU_HOOKS_EN > 0) && (OS_TIME_TICK_HOOK_EN > 0)
void OSTimeTickHook(void)
{
}
#endif

#if OS_INIT_HOOK_BEGIN_EN == 1
void OSInitHookBegin(void)
{
    OSInitVectors();
}
#endif

#if OS_CPU_HOOKS_EN > 0
void OSInitHookEnd(void)
{
}
#endif

#if OS_CPU_HOOKS_EN > 0
void OSTCBInitHook(OS_TCB *ptcb)
{
	BSTD_UNUSED(ptcb);
}
#endif

#if OS_CPU_HOOKS_EN > 0
void OSTaskIdleHook(void)
{

}
#endif

void OSIntCtxSw(void)
{
#if OS_TASK_SW_HOOK_EN > 0
    OSTaskSwHook();
#endif
    OSTCBCur = OSTCBHighRdy;
    OSPrioCur = OSPrioHighRdy;

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
	cp0_count_clocks_per_os_tick = cp0_count_clocks_per_sec / OS_TICKS_PER_SEC;
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
#ifdef NO_L1
    if(l1_interrupt()){    		/* chip interrupt */
#else
    if(0x0400u & cause){    		/* chip interrupt */
#endif
        #if 0
        if(0 !=  ihandler[0]){
            ihandler[0]();
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
        err = OS_ERR_INVALID_OPT;
    }
    return err;
}

