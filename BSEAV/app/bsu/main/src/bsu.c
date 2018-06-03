/******************************************************************************
* Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
******************************************************************************/
#include <stdio.h>

#if UCOS_VERSION==1
#include <ucos.h>
#elif UCOS_VERSION==3
#include <os.h>
#endif

#ifdef MIPS_SDE
#include "bcmmips.h"
#endif

#include "bchp_common.h"
#include "nexus_platform_features.h"
#include "nexus_platform.h"
#include "bchp_timer.h"
#include "bsu_task.h"
#include "bsu_memory.h"

#ifdef BSU_GUI
    #include "bsu_gui.h"
#endif

#ifdef BSU_TEST
    #include "bsu_prompt.h"
    #include "bsu_test.h"
#endif

#ifdef MIPS_SDE
    #include "mipsclock.h"
    #include "cpuctrl.h"
    #include "setup_tlb.h"

    #if MIPS_BSU_HEAP_SIZE > 0
        #include "bsu_memory.h"
    #endif
#else
    #include "uart.h"
    #include "interrupts.h"
#endif

#include "bsu-api.h"
#include "bsu-api2.h"

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/
#ifdef BSU_TEST
    bool frontend_already_opened;
#endif

struct bsu_api *xapi;

/***********************************************************************
 *                      External References
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif

extern void cfe_main(int,int);
extern void init_os_hw_ticks(unsigned int cp0_count_clocks_per_sec);
extern void stdlib_init(void);

#ifdef MIPS_SDE
extern void timer_int(void);
#endif

extern uint32_t g_cp0_count_clocks_per_usec;
extern uint32_t g_cp0_count_clocks_per_sec;
extern uint32_t g_cpu_freq_hz;
extern void dbg_print(char *);
extern void dbg_print_dec32(unsigned int num);
extern void dbg_print_hex32(unsigned int num);
extern uint32_t mem0_size;
extern uint32_t mem1_size;

#ifdef MIPS_SDE
    extern unsigned int __edata;
    extern unsigned int __end;
#endif

uint32_t get_mips_freq(void);
void system_init(void);

#ifdef MIPS_SDE_STDLIB /* Needed if we use SDE standard C library */
    int devinit (void);
#endif

#if __cplusplus
}
#endif

extern unsigned int __getstack(void);
uintptr_t __stack_chk_guard = BSU_STACK_CHECK_VAL;

void __attribute__((noreturn)) __stack_chk_fail(void)
{
	unsigned long *sp = (unsigned long *)__getstack();

	dbg_print("stack_chk_fail ");
	dbg_print_hex32(__stack_chk_guard);
	dbg_print(" STACK @ ");
	dbg_print_hex32((unsigned long)sp);
	dbg_print(" (");
	dbg_print_hex32(*(unsigned long *)sp);
	dbg_print(")");
	while (1);
}

/***********************************************************************
 *                        Local Functions
 ***********************************************************************/

uint32_t arch_get_timer_freq_hz(void);

void root2(void)
{
    NEXUS_PlatformSettings platformSettings;

    /* system_init is called here because the OS (ucos) needs to be initialized before KNI is used */
    system_init();

    NEXUS_Platform_GetDefaultSettings(&platformSettings);

    #ifdef BSU_TEST
        {
            char input_char;
            input_char = det_in_char();

            if (input_char == 'f') {
                platformSettings.openFrontend = frontend_already_opened = false;
            }
            else {
                platformSettings.openFrontend = frontend_already_opened = true;
            }
        }
    #else
        platformSettings.openFrontend = true;
    #endif

    NEXUS_Platform_Init(&platformSettings);

    #ifdef BSU_TEST
        bsu_menu(); /* Bsu Test Menu */
    #else
        bsu_gui();  /* Bsu GUI Demo */
    #endif
}

/***********************************************************************
    *
    *  root: Code for the ROOT task
    *
    *  NOTE: This executes as the 'ROOT' task.  It suspends itself
    *        when finished.
    *
    ***********************************************************************/
void root_task(void * param)
{
    BSTD_UNUSED(param);

    dbg_print("in root_task\n");

    #ifndef MIPS_SDE
        interrupts_init();
    #endif

    for (;;)
    {
        root2();
    }
}

void root(void)
{
#if UCOS_VERSION==1
    OS_STATUS ucosStatus;
#elif UCOS_VERSION==3
    OS_ERR err;
#endif

#if (UCOS_VERSION==1) || (UCOS_VERSION==3)
    #ifdef MIPS_SDE
        /* initialize os_hw_ticks variable */
        g_cp0_count_clocks_per_sec = g_cpu_freq_hz / INPUT_CLK_CYCLES_PER_COUNT_REG_INCR;
        g_cp0_count_clocks_per_usec = g_cp0_count_clocks_per_sec / 1000000;
        init_os_hw_ticks(g_cp0_count_clocks_per_sec);
        #if UCOS_VERSION==1
            timer_init();
            IRQInstall(7, timer_int);
        #endif
    #else
        #if UCOS_VERSION==1
            #error ARM not currently supported
        #endif
        g_cp0_count_clocks_per_sec = xapi->xfn_arch_get_timer_freq_hz();
        g_cp0_count_clocks_per_usec = g_cp0_count_clocks_per_sec / 1000000;
    #endif
#else
    #error unknown UCOS_VERSION
#endif

#if UCOS_VERSION==1
    ucosStatus = OSTaskCreate(root_task,
                              (void *)0,
                              (void *)((UBYTE *)&RootTaskStack[0] + ROOT_TASK_STACK_SIZE),
                              ROOT_TASK_PRIORITY);
#elif UCOS_VERSION==3
    OSTaskCreate((OS_TCB     *)&RootTaskTCB,
                    (CPU_CHAR   *)((void *)"Root Task"),
                    (OS_TASK_PTR )root_task,
                    (void       *)0,
                    (OS_PRIO     )ROOT_TASK_PRIORITY,
                    (CPU_STK    *)&RootTaskStack[0],
                    (CPU_STK_SIZE)ROOT_TASK_STACK_SIZE/10,
                    (CPU_STK_SIZE)ROOT_TASK_STACK_SIZE/4,
                    (OS_MSG_QTY  )0,
                    (OS_TICK     )0,
                    (void       *)0,
                    (OS_OPT      )OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                    (OS_ERR     *)&err);
    if (err != OS_ERR_NONE)
    {
        dbg_print("OSTaskCreate did not return OS_ERR_NONE\n");
        BKNI_Fail();
    }
#else
    #error unknown UCOS_VERSION
#endif


    /* start ucos */
#if UCOS_VERSION==1
    OSStart();
#elif UCOS_VERSION==3
    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OSStart did not return OS_ERR_NONE\n");
        BKNI_Fail();
    }
#else
    #error unknown UCOS_VERSION
#endif
}

#ifdef MIPS_SDE
    uint32_t total_ticks;
    uint32_t cpu_clock_rate_hz;

    /************************************************************************
     *
     *  get_cpu_clock_rate_hz()
     *
     *  Returns the global variable cpu_clock_rate_hz.
     *
     ************************************************************************/
    uint32_t get_cpu_clock_rate_hz(void)
    {
        return cpu_clock_rate_hz;
    }

    /************************************************************************
     *
     *  set_cpu_clock_rate_hz()
     *
     *  Sets the global variable cpu_clock_rate_hz.
     *
     ************************************************************************/
    void set_cpu_clock_rate_hz(uint32_t rate)
    {
        cpu_clock_rate_hz = rate;
    }
#endif

#ifndef MIPS_SDE
    uint32_t arch_get_cpsr(void)
    {
        uint32_t cpsr;

        /* CPSR */
        __asm__ ("mrc   p15, 0, %0, c14, c0, 0" : "=r" (cpsr));

        return cpsr;
    }

#if 0
    uint32_t arch_get_timer_freq_hz(void)
    {
        uint32_t timer_freq_hz;

        /* CNTFRQ */
        __asm__ ("mrc   p15, 0, %0, c14, c0, 0" : "=r" (timer_freq_hz));

        return timer_freq_hz;
    }

    uint32_t arch_getticks(void)
    {
        uint32_t val;

        /* CNTP_TVAL */
        __asm__ ("mrc   p15, 0, %0, c14, c2, 0" : "=r" (val));

        /* FIXME: The timer code was written for a count-up counter */
        return (0xFFFFFFFF - val);
    }
#endif

    uint32_t arch_get_cntpct(uint32_t *val_hi, uint32_t *val_lo)
    {
        uint32_t hi, lo;

        /* CNTPCT */
        __asm__ ("mrrc  p15, 0, %0, %1, c14" : "=r" (lo), "=r" (hi));

        *val_hi = hi;
        *val_lo = lo;

        /* FIXME: The timer code was written for a count-up counter */
        return lo;
    }
#endif

/************************************************************************
 *
 *  bcm_main()
 *
 *  This is Broadcom's main function.  It performs some software
 *  initialization and calls the root() functions.
 *
 ************************************************************************/
void bcm_main(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3)
{
    struct bsu_meminfo *xfd_mem;
    unsigned int i;
#if UCOS_VERSION==3
    OS_ERR err;
#endif

    #ifdef MIPS_SDE
        uint32_t *f = (uint32_t *)&__edata;
        uint32_t *e = (uint32_t *)&__end;

        /* clear .bss section */
        while(f <= e) {
            *f = 0;
            f++;
        }
    #endif

    xapi = (struct bsu_api *)r3;
    if (!xapi)
        #if 1
            while (1);
        #else
            _exit2();
        #endif

    xfd_mem = xapi->xfd_mem;

#ifdef MIPS_SDE_STDLIB /* Needed if we use SDE standard C library */
    devinit();
#endif

    /* --- now we can use the xapi --- */

    printf("_main(r0:0x%08x r1:0x%08x r2:0x%08x r3:0x%08x)\n",
                    r0, r1, r2, r3);

    printf("BSU @ 0x%08x\n", r3);

    printf("Signature is ");
    if (xapi->xfd_signature == BSU_SIGNATURE) {
        printf("ok\n");
    } else {
        printf("BAD!\n");
        #if 1
            while (1);
        #else
            _exit2();
        #endif
    }

    /* all our free memory to use - NOT including us!
    */
    for(i=0; i < xapi->xfd_num_mem; i++) {
        if (xfd_mem[i].top - xfd_mem[i].base) {
            printf("%d \tmemc #%d 0x%08x%08x -> 0x%08x%08x %dMb\n",
                    i, xfd_mem[i].memc,
                    (uint32_t)(xfd_mem[i].base >> 32), (uint32_t)(xfd_mem[i].base & 0xffffffff),
                    (uint32_t)(xfd_mem[i].top >> 32), (uint32_t)(xfd_mem[i].top & 0xffffffff),
                    (uint32_t)((xfd_mem[i].top - xfd_mem[i].base) / (1024 * 1024)));
        }
    }

#if (UCOS_VERSION==1) || (UCOS_VERSION==3)
    #ifdef MIPS_SDE
        g_cpu_freq_hz = get_mips_freq();
        printf("g_cpu_freq_hz=%d\n", g_cpu_freq_hz);
        #ifndef CPU_CLOCK_RATE
            set_cpu_clock_rate_hz(g_cpu_freq_hz);
        #else
            set_cpu_clock_rate_hz(CPU_CLOCK_RATE);
        #endif
    #endif
#else
    #error unknown UCOS_VERSION
#endif

    /* initialize ucos */
    #if UCOS_VERSION==1
        OSInitVectors();
        OSInit();
    #elif UCOS_VERSION==3
        OSInit(&err);
        if (err != OS_ERR_NONE) {
            printf("OSInit did not return OS_ERR_NONE\n");
            BKNI_Fail();
        }
    #else
        #error unknown UCOS_VERSION
    #endif

#if MIPS_BSU_HEAP_SIZE > 0
    /* Once memory has been initialized, we can create the BSU memory heap */
    bsu_heap_init((unsigned char *)MIPS_BSU_HEAP_ADDR, MIPS_BSU_HEAP_SIZE);

#define TEST_SIZE 8*1024*1024

    #ifdef TEST_SIZE
    {
#ifdef USE_BSU_MALLOC_NAME
        void *ptr = bsu_malloc(TEST_SIZE);
#else
        void *ptr = malloc(TEST_SIZE);
#endif
        bsu_mem_test(ptr, ptr+TEST_SIZE-1);
#ifdef USE_BSU_MALLOC_NAME
        bsu_free(ptr);
#else
        free(ptr);
#endif
    }
    #endif
#endif

    root();
}

#ifdef MIPS_SDE
    /************************************************************************
     *
     *  get_mips_freq()
     *
     *  This procedure is called to clock speed of the mips.
     *
     ************************************************************************/
    uint32_t get_mips_freq(void)
    {
        uint32_t count=0, speed;
        #define TIMER_ENABLE 0x80000000
        #define TIMER_CNTDWN 0x40000000
        #define REG_BCHP_TIMER_TIMER_IS *((volatile uint32_t *)(BCM_PHYS_TO_K1(BCHP_PHYSICAL_OFFSET+BCHP_TIMER_TIMER_IS)))
        #define REG_BCHP_TIMER_TIMER0_CTRL *((volatile uint32_t *)(BCM_PHYS_TO_K1(BCHP_PHYSICAL_OFFSET+BCHP_TIMER_TIMER0_CTRL)))

        REG_BCHP_TIMER_TIMER_IS = 0x00000001;
        REG_BCHP_TIMER_TIMER0_CTRL = 27000000 | TIMER_CNTDWN | TIMER_ENABLE;
        CpuCountSet(0);

        while (!(REG_BCHP_TIMER_TIMER_IS & 0x00000001))
        count = CpuCountGet();
        speed = count * INPUT_CLK_CYCLES_PER_COUNT_REG_INCR;

        /* printf("calculated MIPS clock speed = %d Hz\n", speed); */ /* No printfs before OS is initialized */
        dbg_print("calculated MIPS clock speed = ");
        dbg_print_dec32(speed);
        dbg_print(" Hz\n");
        return speed;
    }
#endif

/************************************************************************
 *
 *  system_init()
 *
 *  This procedure is called to initialize the BCM97038.
 *
 ************************************************************************/
void system_init(void)
{
    #ifdef MIPS_SDE
        setup_tlb();    /* Setup TLB for extra additional memory, if necessary */
    #endif
}

#ifndef MIPS_SDE
    void c_init(void)
    {
        extern unsigned int _bss_start;
        extern unsigned int _bss_end;

        unsigned int *bss_start = &_bss_start;
        unsigned int *bss_end = &_bss_end;

        while (bss_start != bss_end)
            *bss_start++ = 0;
    }
#endif
