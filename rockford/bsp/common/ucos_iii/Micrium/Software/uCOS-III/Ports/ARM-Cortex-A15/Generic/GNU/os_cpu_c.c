/*
*********************************************************************************************************
*                                                uC/OS-III
*                                          The Real-Time Kernel
*
*
*                           (c) Copyright 2009-2012; Micrium, Inc.; Weston, FL
*                    All rights reserved.  Protected by international copyright laws.
*
*                                            ARM Cortex-A9 Port
*
* File      : OS_CPU_C.C
* Version   : V3.03.1
* By        : NB
*             JPB
*
* LICENSING TERMS:
* ---------------
*           uC/OS-III is provided in source form for FREE short-term evaluation, for educational use or
*           for peaceful research.  If you plan or intend to use uC/OS-III in a commercial application/
*           product then, you need to contact Micrium to properly license uC/OS-III for its use in your
*           application/product.   We provide ALL the source code for your convenience and to help you
*           experience uC/OS-III.  The fact that the source is provided does NOT mean that you can use
*           it commercially without paying a licensing fee.
*
*           Knowledge of the source code may NOT be used to develop a similar product.
*
*           Please help us continue to provide the embedded community with the finest software available.
*           Your honesty is greatly appreciated.
*
*           You can contact us at www.micrium.com, or by phone at +1 (954) 217-2036.
*
* For       : ARM Cortex-A9 (ARMv7)
* Mode      : ARM or Thumb
* Toolchain : GNU C
**********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS
#include <os.h>


/*
*********************************************************************************************************
*                                          LOCAL VARIABLES
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
static  CPU_INT16U  OSTmrCtr;
#endif


/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/

void  OSInitHook (void)
{
    CPU_INT32U   size;
    CPU_STK     *pstk;

                                                           /* Clear exception stack for stack checking.*/
    pstk = &OS_CPU_ExceptStk[0];
    size = OS_CPU_EXCEPT_STK_SIZE;
    while (size > 0u) {
        size--;
       *pstk++ = (CPU_STK)0;
    }
                                                          /* Align the ISR stack to 8-bytes           */
    OS_CPU_ExceptStkBase = (CPU_STK *)&OS_CPU_ExceptStk[OS_CPU_EXCEPT_STK_SIZE];
    OS_CPU_ExceptStkBase = (CPU_STK *)((CPU_STK)(OS_CPU_ExceptStkBase) & 0xFFFFFFF8);
    

#if OS_TMR_EN > 0u
    OSTmrCtr = 0u;
#endif
}



/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : p_tcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void  OSTaskCreateHook (OS_TCB *p_tcb)
{
#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppTaskCreateHookPtr != (OS_APP_HOOK_TCB)0) {
        (*OS_AppTaskCreateHookPtr)(p_tcb);
    }
#else
    (void)p_tcb;                                            /* Prevent compiler warning                           */
#endif
}


/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void  OSTaskDelHook (OS_TCB *p_tcb)
{
#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppTaskDelHookPtr != (OS_APP_HOOK_TCB)0) {
        (*OS_AppTaskDelHookPtr)(p_tcb);
    }
#else
    (void)p_tcb;                                            /* Prevent compiler warning                 */
#endif
}


/*
*********************************************************************************************************
*                                             IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do
*              such things as STOP the CPU to conserve power.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/

void  OSIdleTaskHook (void)
{
#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppIdleTaskHookPtr != (OS_APP_HOOK_VOID)0) {
        (*OS_AppIdleTaskHookPtr)();
    }
#endif
}


/*
*********************************************************************************************************
*                                            TASK RETURN HOOK
*
* Description: This function is called if a task accidentally returns.  In other words, a task should
*              either be an infinite loop or delete itself when done.
*
* Arguments  : p_tcb     is a pointer to the task control block of the task that is returning.
*
* Note(s)    : none
*********************************************************************************************************
*/

void  OSTaskReturnHook (OS_TCB  *p_tcb)
{
#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppTaskReturnHookPtr != (OS_APP_HOOK_TCB)0) {
        (*OS_AppTaskReturnHookPtr)(p_tcb);
    }
#else
    (void)p_tcb;
#endif
}


/*
*********************************************************************************************************
*                                           STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
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
**********************************************************************************************************
*                                       INITIALIZE A TASK'S STACK
*
* Description: This function is called by OS_Task_Create() or OSTaskCreateExt() to initialize the stack
*              frame of the task being created. This function is highly processor specific.
*
* Arguments  : p_task       Pointer to the task entry point address.
*
*              p_arg        Pointer to a user supplied data area that will be passed to the task
*                               when the task first executes.
*
*              p_stk_base   Pointer to the base address of the stack.
*
*              stk_size     Size of the stack, in number of CPU_STK elements.
*
*              opt          Options used to alter the behavior of OS_Task_StkInit().
*                            (see OS.H for OS_TASK_OPT_xxx).
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : (1) Interrupts are enabled when task starts executing.
*
*              (2) All tasks run in SVC mode.
*
*              (3) There are three different stack frames depending on whether or not the Floating-Point (FP) co-processor
*                  is enabled or not.
*
*                  (a) The stack frame shown in the diagram is used when the FP co-processor is present and
*                      OS_OPT_TASK_SAVE_FP is enabled. In this case the FP exception register, the FP registers and the
*                      FP control/status register are saved in the stack frame.
*
*                  (b) If the FP co-processor is present but the OS_OPT_TASK_SAVE_FP is not set, only the FP
*                      exception register is saved in the stack.
*
*                      (1) The FP exception register is saved twice in the stack frame to keep the 8-byte alignment.
*                          (See note #4.)
* 
*                    +-----------+   
*                    |   FPEXC   |     
*                    +-----------+   
*                    |    S0     |   
*                    +-----------+  
*                          .       
*                          .       
*                          .       
*                    +-----------+
*                    |    S29    |
*                    +-----------+
*                    |    S30    |
*                    +-----------+       +-----------+
*                    |    S31    |       |   FPEXC   |
*                    +-----------+       +-----------+ 
*                    |   FPSCR   |       |   FPEXC   | 
*                    +-----------+       +-----------+      +-----------+  
*                    |   CPSR    |       |   CPSR    |      |   CPSR    |
*                    +-----------+       +-----------+      +-----------+
*                    |    R0     |       |     R0    |      |    R0     |                   
*                    +-----------+       +-----------+      +-----------+
*                          .                  .                  . 
*                          .                  .                  .
*                          .                  .                  .
*                    +-----------+       +-----------+      +-----------+
*                    |    R10    |       |    R10    |      |    R10    |
*                    +-----------+       +-----------+      +-----------+
*                    |    R11    |       |    R11    |      |    R11    |
*                    +-----------+       +-----------+      +-----------+
*                    |    R12    |       |    R12    |      |    R12    |
*                    +-----------+       +-----------+      +-----------+
*                    |  R14 (LR) |       |  R14 (LR) |      |  R14 (LR) |
*                    +-----------+       +-----------+      +-----------+
*                    | PC = Task |       | PC = Task |      | PC = Task |                     
*                    +-----------+       +-----------+      +-----------+
*
*                        (a)                 (b)                (c)
*
*             (4) The SP must be 8-byte aligned in conforming to the Procedure Call Standard for the ARM architecture 
*
*                    (a) Section 2.1 of the  ABI for the ARM Architecture Advisory Note. SP must be 8-byte aligned 
*                        on entry to AAPCS-Conforming functions states : 
*                    
*                        The Procedure Call Standard for the ARM Architecture [AAPCS] requires primitive 
*                        data types to be naturally aligned according to their sizes (for size = 1, 2, 4, 8 bytes). 
*                        Doing otherwise creates more problems than it solves. 
*
*                        In return for preserving the natural alignment of data, conforming code is permitted 
*                        to rely on that alignment. To support aligning data allocated on the stack, the stack 
*                        pointer (SP) is required to be 8-byte aligned on entry to a conforming function. In 
*                        practice this requirement is met if:
*
*                           (1) At each call site, the current size of the calling functions stack frame is a multiple of 8 bytes.
*                               This places an obligation on compilers and assembly language programmers.
*
*                           (2) SP is a multiple of 8 when control first enters a program.
*                               This places an obligation on authors of low level OS, RTOS, and runtime library 
*                               code to align SP at all points at which control first enters 
*                               a body of (AAPCS-conforming) code. 
*              
*                       In turn, this requires the value of SP to be aligned to 0 modulo 8:
*
*                           (3) By exception handlers, before calling AAPCS-conforming code.
*
*                           (4) By OS/RTOS/run-time system code, before giving control to an application.
*
*                 (b) Section 2.3.1 corrective steps from the the SP must be 8-byte aligned on entry 
*                     to AAPCS-conforming functions advisory note also states.
* 
*                     " This requirement extends to operating systems and run-time code for all architecture versions 
*                       prior to ARMV7 and to the A, R and M architecture profiles thereafter. Special considerations 
*                       associated with ARMV7M are discussed in 2.3.3"
* 
*                     (1) Even if the SP 8-byte alignment is not a requirement for the ARMv7M profile, the stack is aligned
*                         to 8-byte boundaries to support legacy execution environments.
*
*                 (c) Section 5.2.1.2 from the Procedure Call Standard for the ARM 
*                     architecture states :  "The stack must also conform to the following 
*                     constraint at a public interface:
*
*                     (1) SP mod 8 = 0. The stack must be double-word aligned"
*
*                 (d) From the ARM Technical Support Knowledge Base. 8 Byte stack alignment.
*
*                     "8 byte stack alignment is a requirement of the ARM Architecture Procedure 
*                      Call Standard [AAPCS]. This specifies that functions must maintain an 8 byte 
*                      aligned stack address (e.g. 0x00, 0x08, 0x10, 0x18, 0x20) on all external 
*                      interfaces. In practice this requirement is met if:
*
*                      (1) At each external interface, the current stack pointer 
*                          is a multiple of 8 bytes.
* 
*                      (2) Your OS maintains 8 byte stack alignment on its external interfaces 
*                          e.g. on task switches"
**********************************************************************************************************
*/

CPU_STK  *OSTaskStkInit (OS_TASK_PTR         p_task,
                         void               *p_arg,
                         CPU_STK            *p_stk_base,
                         CPU_STK            *p_stk_limit,
                         CPU_STK_SIZE        stk_size,
                         OS_OPT              opt)

{
    CPU_STK     *p_stk;
    CPU_INT32U   task_addr;
    
#if (OS_CPU_ARM_FP_EN > 0u)
    /*CPU_INT08U  i;*/
#endif    

    (void)p_stk_limit;
    (void)opt;

    p_stk     = &p_stk_base[stk_size];                          /* Load stack pointer                                     */

    p_stk     = (CPU_STK *)((CPU_STK)(p_stk) & 0xFFFFFFF8u);
    task_addr = (CPU_INT32U)p_task & ~1u;                       /* Mask off lower bit in case task is thumb mode          */

    *--p_stk  = (CPU_STK)task_addr;                             /* Entry Point                                            */
    *--p_stk  = (CPU_STK)OS_TaskReturn;                         /* R14 (LR)                                               */
    *--p_stk  = (CPU_STK)0x12121212u;                           /* R12                                                    */
    *--p_stk  = (CPU_STK)0x11111111u;                           /* R11                                                    */
    *--p_stk  = (CPU_STK)0x10101010u;                           /* R10                                                    */
    *--p_stk  = (CPU_STK)0x09090909u;                           /* R9                                                     */
    *--p_stk  = (CPU_STK)0x08080808u;                           /* R8                                                     */
    *--p_stk  = (CPU_STK)0x07070707u;                           /* R7                                                     */
    *--p_stk  = (CPU_STK)0x06060606u;                           /* R6                                                     */
    *--p_stk  = (CPU_STK)0x05050505u;                           /* R5                                                     */
    *--p_stk  = (CPU_STK)0x04040404u;                           /* R4                                                     */
    *--p_stk  = (CPU_STK)0x03030303u;                           /* R3                                                     */
    *--p_stk  = (CPU_STK)0x02020202u;                           /* R2                                                     */
    *--p_stk  = (CPU_STK)0x01010101u;                           /* R1                                                     */
    *--p_stk  = (CPU_STK)p_arg;                                 /* R0 : argument                                          */

    
#if (OS_CPU_ARM_ENDIAN_TYPE == OS_CPU_ARM_ENDIAN_LITTLE)
    if (((CPU_INT32U)p_task & 0x01u) == 0x01u) {                /* See if task runs in Thumb or ARM mode                  */
       *--p_stk = (CPU_STK)(OS_CPU_ARM_BIT_CPSR_MODE_SUPERVISOR /* Set supervisor mode.                                   */
                |           OS_CPU_ARM_BIT_CPSR_T);             /* Set Thumb mode.                                        */
    } else {
       *--p_stk = (CPU_STK)(0x00000100 | OS_CPU_ARM_BIT_CPSR_MODE_SUPERVISOR);
    }
#else    
    if (((CPU_INT32U)p_task & 0x01u) == 0x01u) {                /* See if task runs in Thumb or ARM mode                  */
       *--p_stk = (CPU_STK)(OS_CPU_ARM_BIT_CPSR_MODE_SUPERVISOR /* Set supervisor mode.                                   */
                |           OS_CPU_ARM_BIT_CPSR_T               /* Set Thumb mode.                                        */
                |           OS_CPU_ARM_BIT_CPSR_E);             /* Set Endiannes bit. Big Endian                          */
    } else {
       *--p_stk = (CPU_STK)(OS_CPU_ARM_BIT_CPSR_MODE_SUPERVISOR
                |           OS_CPU_ARM_BIT_CPSR_E);
    }
#endif

    
    return (p_stk);
}


/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCurPtr' points to the
*                 task being switched out (i.e. the preempted task).
*              3) If debug variables are enabled, the current process id is saved into the context ID register
*                 found in the system control coprocessor. The Embedded Trace Macrocell (ETM) and the debug logic
*                 use this register. The ETM can broadcast its value to indicate the process that is running currently.
*                         
*                     (a) The proccess id is formed by concatenating the current task priority with the lower 24 bits
*                         from the current task TCB.
*
*                            31              24                     0
*                             +---------------+---------------------+
*                             | OSPrioHighRdy | OSTCBHighRdy[23..0] |
*                             +---------------+---------------------+
*********************************************************************************************************
*/

void  OSTaskSwHook (void)

{
#if OS_CFG_TASK_PROFILE_EN > 0u
    CPU_TS  ts;
#endif
#ifdef  CPU_CFG_INT_DIS_MEAS_EN
    CPU_TS  int_dis_time;
#endif

#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppTaskSwHookPtr != (OS_APP_HOOK_VOID)0) {
        (*OS_AppTaskSwHookPtr)();
    }
#endif

#if OS_CFG_TASK_PROFILE_EN > 0u
    ts = OS_TS_GET();
    if (OSTCBCurPtr != OSTCBHighRdyPtr) {
        OSTCBCurPtr->CyclesDelta  = ts - OSTCBCurPtr->CyclesStart;
        OSTCBCurPtr->CyclesTotal += (OS_CYCLES)OSTCBCurPtr->CyclesDelta;
    }

    OSTCBHighRdyPtr->CyclesStart = ts;
#endif

#ifdef  CPU_CFG_INT_DIS_MEAS_EN
    int_dis_time = CPU_IntDisMeasMaxCurReset();             /* Keep track of per-task interrupt disable time          */
    if (OSTCBCurPtr->IntDisTimeMax < int_dis_time) {
        OSTCBCurPtr->IntDisTimeMax = int_dis_time;
    }
#endif

#if OS_CFG_SCHED_LOCK_TIME_MEAS_EN > 0u
                                                            /* Keep track of per-task scheduler lock time             */
    if (OSTCBCurPtr->SchedLockTimeMax < OSSchedLockTimeMaxCur) {
        OSTCBCurPtr->SchedLockTimeMax = OSSchedLockTimeMaxCur;
    }
    OSSchedLockTimeMaxCur = (CPU_TS)0;                      /* Reset the per-task value                               */
#endif
}


/*
*********************************************************************************************************
*                                               TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

void  OSTimeTickHook (void)
{
#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppTimeTickHookPtr != (OS_APP_HOOK_VOID)0) {
        (*OS_AppTimeTickHookPtr)();
    }
#endif
}


/*
*********************************************************************************************************
*                             INTERRUPT DISABLE TIME MEASUREMENT, START
*********************************************************************************************************
*/

#if OS_CPU_INT_DIS_MEAS_EN > 0u
void  OS_CPU_IntDisMeasInit (void)
{
    OS_CPU_IntDisMeasNestingCtr = 0u;
    OS_CPU_IntDisMeasCntsEnter  = 0u;
    OS_CPU_IntDisMeasCntsExit   = 0u;
    OS_CPU_IntDisMeasCntsMax    = 0u;
    OS_CPU_IntDisMeasCntsDelta  = 0u;
    OS_CPU_IntDisMeasCntsOvrhd  = 0u;
    OS_CPU_IntDisMeasStart();                              /* Measure the overhead of the functions    */
    OS_CPU_IntDisMeasStop();
    OS_CPU_IntDisMeasCntsOvrhd  = OS_CPU_IntDisMeasCntsDelta;
}


void  OS_CPU_IntDisMeasStart (void)
{
    OS_CPU_IntDisMeasNestingCtr++;
    if (OS_CPU_IntDisMeasNestingCtr == 1u) {               /* Only measure at the first nested level   */
        OS_CPU_IntDisMeasCntsEnter = OS_CPU_IntDisMeasTmrRd();
    }
}


void  OS_CPU_IntDisMeasStop (void)
{
    OS_CPU_IntDisMeasNestingCtr--;                                      /* Decrement nesting ctr       */
    if (OS_CPU_IntDisMeasNestingCtr == 0u) {
        OS_CPU_IntDisMeasCntsExit  = OS_CPU_IntDisMeasTmrRd();
        OS_CPU_IntDisMeasCntsDelta = OS_CPU_IntDisMeasCntsExit - OS_CPU_IntDisMeasCntsEnter;
        if (OS_CPU_IntDisMeasCntsDelta > OS_CPU_IntDisMeasCntsOvrhd) {  /* Ensure overhead < delta     */
            OS_CPU_IntDisMeasCntsDelta -= OS_CPU_IntDisMeasCntsOvrhd;
        } else {
            OS_CPU_IntDisMeasCntsDelta  = OS_CPU_IntDisMeasCntsOvrhd;
        }
        if (OS_CPU_IntDisMeasCntsDelta > OS_CPU_IntDisMeasCntsMax) {    /* Track MAXIMUM               */
            OS_CPU_IntDisMeasCntsMax = OS_CPU_IntDisMeasCntsDelta;
        }
    }
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                    INITIALIZE EXCEPTION VECTORS
*
* Description: This function initialize exception vectors to the default handlers.
*
* Arguments  : None.
*********************************************************************************************************
*/

void  OS_CPU_InitExceptVect (void)
{
    (*(CPU_INT32U *)OS_CPU_ARM_EXCEPT_VECT_ADDR_RST)               =             OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(CPU_INT32U *)OS_CPU_ARM_EXCEPT_HANDLER_ADDR_UND)            = (CPU_INT32U)OS_CPU_ARM_ExceptUndefInstrHndlr;

    (*(CPU_INT32U *)OS_CPU_ARM_EXCEPT_VECT_ADDR_SWI)               =             OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(CPU_INT32U *)OS_CPU_ARM_EXCEPT_HANDLER_ADDR_SWI)            = (CPU_INT32U)OS_CPU_ARM_ExceptSwiHndlr;

    (*(CPU_INT32U *)OS_CPU_ARM_EXCEPT_VECT_ADDR_ABORT_PREFETCH)    =             OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(CPU_INT32U *)OS_CPU_ARM_EXCEPT_HANDLER_ADDR_ABORT_PREFETCH) = (CPU_INT32U)OS_CPU_ARM_ExceptPrefetchAbortHndlr;

    (*(CPU_INT32U *)OS_CPU_ARM_EXCEPT_VECT_ADDR_ABORT_DATA)        =             OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(CPU_INT32U *)OS_CPU_ARM_EXCEPT_HANDLER_ADDR_ABORT_DATA)     = (CPU_INT32U)OS_CPU_ARM_ExceptDataAbortHndlr;

    (*(CPU_INT32U *)OS_CPU_ARM_EXCEPT_VECT_ADDR_IRQ)               =             OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(CPU_INT32U *)OS_CPU_ARM_EXCEPT_HANDLER_ADDR_IRQ)            = (CPU_INT32U)OS_CPU_ARM_ExceptIrqHndlr;

    (*(CPU_INT32U *)OS_CPU_ARM_EXCEPT_VECT_ADDR_FIQ)               =             OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(CPU_INT32U *)OS_CPU_ARM_EXCEPT_HANDLER_ADDR_FIQ)            = (CPU_INT32U)OS_CPU_ARM_ExceptFiqHndlr;
}


/*
*********************************************************************************************************
*                              GET NUMBER OF FREE ENTRIES IN EXCEPTION STACK
*
* Description : This function computes the number of free entries in the exception stack.
*
* Arguments   : None.
*
* Returns     : The number of free entries in the exception stack.
*********************************************************************************************************
*/

CPU_INT32U  OS_CPU_ExceptStkChk (void)
{
    CPU_STK    *pchk;
    CPU_INT32U  nfree;
    CPU_INT32U  size;


    nfree = 0;
    size  = OS_CPU_EXCEPT_STK_SIZE;
    pchk  = &OS_CPU_ExceptStk[0];
    while ((*pchk++ == (CPU_STK)0) && (size > 0u)) {            /* Compute the number of zero entries on the stk        */
        nfree++;
        size--;
    }

    return (nfree);
}


/*
*********************************************************************************************************
*                                          ARM INTERRUPT/EXCEPTION HANDLER
*
* Description : Handle ARM exceptions.
*
* Argument(s) : src_id     ARM exception source identifier:
*
*                                  OS_CPU_ARM_EXCEPT_RESET             0x00
*                                  OS_CPU_ARM_EXCEPT_UNDEF_INSTR       0x01
*                                  OS_CPU_ARM_EXCEPT_SWI               0x02
*                                  OS_CPU_ARM_EXCEPT_ABORT_PREFETCH    0x03
*                                  OS_CPU_ARM_EXCEPT_ABORT_DATA        0x04
*                                  OS_CPU_ARM_EXCEPT_RSVD              0x05
*                                  OS_CPU_ARM_EXCEPT_IRQ               0x06
*                                  OS_CPU_ARM_EXCEPT_FIQ               0x07
*
* Return(s)   : none.
*
* Caller(s)   : OS_CPU_ARM_ExceptHndlr(), which is declared in os_cpu_a.s.
*
* Note(s)     : (1) Only OS_CPU_ARM_EXCEPT_FIQ and OS_CPU_ARM_EXCEPT_IRQ exceptions handler are implemented. 
*                   For the rest of the exception a infinite loop is implemented for debugging purposes. This behavior
*                   should be replaced with another behavior (reboot, etc).
*********************************************************************************************************
*/

extern void interrupt_handler(void);
void  OS_CPU_IntHandler (CPU_INT32U  src_id)
{
#if 1
    (void)src_id;
	interrupt_handler();
#else
	CPU_INT32U  int_id;


    switch (src_id) {
        case OS_CPU_ARM_EXCEPT_IRQ:
        case OS_CPU_ARM_EXCEPT_FIQ:
       	     int_id = *(CPU_INT32U*)0xF8F0010C;                 /* Interrupt acknowledge register (read serves as ack)  */
       	     int_id =  int_id & 0x000003FF;
             CSP_IntHandlerSrc((CSP_DEV_NBR)int_id);
             break;
       
        case OS_CPU_ARM_EXCEPT_RST:
        case OS_CPU_ARM_EXCEPT_UND:
        case OS_CPU_ARM_EXCEPT_SWI:
        case OS_CPU_ARM_EXCEPT_ABORT_DATA:
        case OS_CPU_ARM_EXCEPT_ABORT_PREFETCH:
        case OS_CPU_ARM_EXCEPT_RSVD:        
        default:        
             OS_CSP_BSP_ExceptHandler((CPU_INT08U)src_id);
             break;
    }
#endif
}
