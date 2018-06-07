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
******************************************************************************/
#include "nexus_platform_priv.h"
#include "bkni.h"
#include "bdbg_log.h"
#include "nexus_interrupt_map.h"
#include "nexus_generic_driver_impl.h"
#if NEXUS_HAS_GPIO
#include "nexus_gpio_init.h"
#endif
#include "int1.h"
#if UCOS_VERSION==1
    #include <ucos.h>
#elif UCOS_VERSION==3
    #include <os.h>
#endif
#include "bsu-api.h"
#include "bsu-api2.h"

#define BDBG_TRACE_L2(x) /*BDBG_MSG(x)*/

BDBG_MODULE(nexus_platform_os);

#define PERR(fmt, args...)
#define PWARN(fmt, args...)
#define PINFO(fmt, args...)
#define PDEBUG(fmt, args...)

#ifndef NEXUS_CPU_ARM
    #include "memmap.h"
    #include "bcm_mips_defs.h"
#endif

/*
The interrupt code is meant to stay in sync with nexus/build/nfe_driver/b_bare_os.c's
interrupt code.
*/

typedef void (*b_bare_os_special_interrupt_handler)(int linux_irq);
#define BDBG_MSG_IRQ(X) /* BDBG_MSG(X) */

#define NUM_IRQS (32*NEXUS_NUM_L1_REGISTERS)
/* b_bare_os L1 interrupts are 0-based. linux is 1-based. */
#define LINUX_IRQ(i) (i)
#define NEXUS_IRQ(i) (i)

struct b_bare_interrupt_entry {
    const char *name;
    void (*handler)(void *,int);
    void *context_ptr;
    int context_int;
    b_bare_os_special_interrupt_handler special_handler;
    bool requested;      /* request_irq called. must be defered until first enable. */
    bool enabled;        /* external "state" of irq, enabled by caller. Nexus needs to balance disable/enable_irq calls and */
                         /* keeps this state in "taskletEnabled" */
    bool taskletEnabled; /* true if irq is enabled in the kernel (enable_irq), false if irq is off at kernel (disable_irq) */
    bool shared;
    unsigned count;
    bool print;
};

static struct b_bare_interrupt_state {
/*    spinlock_t lock; */
    bool started;
    struct {
        uint32_t IntrStatus;
        uint32_t IntrMaskStatus;
    } processing[NEXUS_NUM_L1_REGISTERS], pending[NEXUS_NUM_L1_REGISTERS];
    struct b_bare_interrupt_entry table[NUM_IRQS];
/*    struct work_struct task; */
} b_bare_interrupt_state;

static void NEXUS_Platform_P_Isr(unsigned long data);

#define ISR_TASK_PRIORITY 1
#define ISR_TASK_STACK_SIZE 0x8000
#if UCOS_VERSION==1
    OS_EVENT *pEvent;
    UBYTE IsrTaskStack[ISR_TASK_STACK_SIZE];
#elif UCOS_VERSION==3
    OS_TCB IsrTaskTCB;
    CPU_STK IsrTaskStack[ISR_TASK_STACK_SIZE];
#else
    #error unknown UCOS_VERSION
#endif

#if NEXUS_CPU_ARM
#include "nexus_platform_cma.c"
#endif

NEXUS_Error
NEXUS_Platform_P_InitOS(void)
{
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    unsigned i;
#if UCOS_VERSION==1
    OS_STATUS ucosStatus;
#elif UCOS_VERSION==3
    OS_ERR err;
#endif

    BKNI_Memset(&state->table, 0, sizeof(state->table));
    for(i=0;i<NEXUS_NUM_L1_REGISTERS;i++) {
        state->processing[i].IntrMaskStatus = ~0;
        state->processing[i].IntrStatus = 0;
        state->pending[i] = state->processing[i];
    }
    state->started = true;

    /* use g_platformMemory to pass OS value to NEXUS_Platform_P_SetCoreModuleSettings */
    g_platformMemory.maxDcacheLineSize = 128; /* TBD nexus_driver_state.settings.maxDcacheLineSize; */

    /* create ISR task */
#if UCOS_VERSION==1
    pEvent = OSSemCreate(0);
    ucosStatus = OSTaskCreate(NEXUS_Platform_P_Isr,
                              (void *)0,
                              (void *)((UBYTE *)&IsrTaskStack[0] + ISR_TASK_STACK_SIZE),
                              ISR_TASK_PRIORITY);
#elif UCOS_VERSION==3
    OSTaskCreate((OS_TCB     *)&IsrTaskTCB,
                 (CPU_CHAR   *)((void *)"NEXUS_Platform_P_Isr"),
                 (OS_TASK_PTR )NEXUS_Platform_P_Isr,
                 (void       *)0,
                 (OS_PRIO     )ISR_TASK_PRIORITY,
                 (CPU_STK    *)&IsrTaskStack[0],
                 (CPU_STK_SIZE)ISR_TASK_STACK_SIZE/10,
                 (CPU_STK_SIZE)ISR_TASK_STACK_SIZE/4,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 (OS_ERR     *)&err);
    if (err != OS_ERR_NONE)
    {
        BDBG_ERR(("OSTaskCreate did not return OS_ERR_NONE\n"));
        BDBG_ASSERT(false);
    }
#else
    #error unknown UCOS_VERSION
#endif

#if 0
    rc = nexus_driver_scheduler_init();
    if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc);goto err_driver; }
#endif

#if UCOS_VERSION==1
    /* Install main dispatcher for CAUSE_IP3 */
    IRQInstall(2, CPUINT1_Isr);
#endif

    return NEXUS_SUCCESS;
}

NEXUS_Error
NEXUS_Platform_P_UninitOS(void)
{
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    unsigned i;

    state->started = false;
    for(i=0;i<NUM_IRQS;i++) {
        if (state->table[i].handler) {
            NEXUS_Platform_P_DisconnectInterrupt(i);
        }
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Platform_P_InitOSMem()
{
    /* nothing required */
    return 0;
}

void NEXUS_Platform_P_UninitOSMem()
{
    /* nothing required */
}

/* IMPORTANT: the logic in NEXUS_Platform_P_MapMemory must be carefully paired with NEXUS_Platform_P_UnmapMemory */
void *
NEXUS_Platform_P_MapMemory(NEXUS_Addr offset, size_t length, NEXUS_AddrType type)
{
    /*unsigned*/ long addr = 0UL;
    bool cached = type==NEXUS_AddrType_eCached;

#if NEXUS_CPU_ARM
    BSTD_UNUSED(cached);
    addr = offset;
#else
    switch ((unsigned)offset & 0xf0000000)
    {
        case 0x00000000:    /* KSEG0/KSEZG1 */
                if (cached)
                    addr = offset + 0x80000000;
                else
                    addr = offset + 0xA0000000;
            break;
        case 0x10000000:    /* chips regs */
            addr = offset + 0xA0000000;
            break;

        case DRAM_0_PHYS_ADDR_START+0x00000000: /* memc0 */
        case DRAM_0_PHYS_ADDR_START+0x10000000: /* memc0 */
        case DRAM_0_PHYS_ADDR_START+0x20000000: /* memc0 */
                if (cached)
                    addr = offset + (DRAM_0_VIRT_CACHED_ADDR_START-DRAM_0_PHYS_ADDR_START);
                else
                    addr = offset + (DRAM_0_VIRT_ADDR_START-DRAM_0_PHYS_ADDR_START);
            break;

        case DRAM_1_PHYS_ADDR_START+0x00000000: /* memc1 */
        case DRAM_1_PHYS_ADDR_START+0x10000000: /* memc1 */
        case DRAM_1_PHYS_ADDR_START+0x20000000: /* memc1 */
        case DRAM_1_PHYS_ADDR_START+0x30000000: /* memc1 */
            if (cached)
                addr = offset + (DRAM_1_VIRT_CACHED_ADDR_START-DRAM_1_PHYS_ADDR_START);
            else
                addr = offset + (DRAM_1_VIRT_ADDR_START-DRAM_1_PHYS_ADDR_START);
            break;

        default:
            BDBG_ERR(("NEXUS_Platform_P_MapMemory:  unknown offset:  0x%08lx", offset));
    }
#endif

    BDBG_MSG(("mmap offset=%x=>%x size=%d MB 0x%08x fd=%d", (unsigned)offset, (unsigned)addr, length/(1024*1024), length, -1));

    return (void *)addr;
}

void
NEXUS_Platform_P_UnmapMemory(void *pMem, size_t length, NEXUS_AddrType memoryMapType)
{
    BSTD_UNUSED(pMem);
    BSTD_UNUSED(length);
    BSTD_UNUSED(memoryMapType);
    return;
}


void *
NEXUS_Platform_P_MapRegisterMemory(unsigned long offset, unsigned long length)
{
    return NEXUS_Platform_P_MapMemory(offset,length,NEXUS_AddrType_eUncached);
}


void
NEXUS_Platform_P_UnmapRegisterMemory(void *pMem,unsigned long length)
{
    return NEXUS_Platform_P_UnmapMemory(pMem, length, NEXUS_AddrType_eUncached);
}


NEXUS_Error NEXUS_Platform_P_GetHostMemory(NEXUS_PlatformMemory *pMemory)
{
#if NEXUS_CPU_ARM
    BSTD_UNUSED(pMemory);
#else
    /* The following information is obtained from /sys/devices/platform/brcmstb/bmem.* */

    /* Originally 192M@64M */
    pMemory->osRegion[0].base = 0x09000000;
    pMemory->osRegion[0].length = 0x07000000; /* 112MB */

    pMemory->memoryLayout.memc[0].region[0].addr = 0x08000000;
    pMemory->memoryLayout.memc[0].region[0].size = pMemory->memoryLayout.memc[0].region[0].size - pMemory->memoryLayout.memc[0].region[0].addr;

    if (((pMemory->memoryLayout.memc[0].size>>20) == 512) && ((pMemory->memoryLayout.memc[1].size>>20) == 0)) {
        /* Standard 256M@512M with MEMC0=512MB, i.e., 97241dcsfbtsff*/
        pMemory->osRegion[1].base = DRAM_0_PHYS_ADDR_START;
        pMemory->osRegion[1].length = 0x10000000; /* 256MB */
    } else {
        #if (((BCHP_CHIP==7563) && (NEXUS_PLATFORM_7563_USFF2L!=1)) || (BCHP_CHIP==7231))
            /* Standard 256M@512M with MEMC0=512MB*/
            pMemory->osRegion[1].base = DRAM_0_PHYS_ADDR_START;
            pMemory->osRegion[1].length = 0x10000000; /* 256MB */
        #elif (BCHP_CHIP==7344) || (BCHP_CHIP==7346) || (BCHP_CHIP==7584) || (BCHP_CHIP==75845) || (BCHP_CHIP==73465)
            /* Standard 512M@512M */
            pMemory->osRegion[1].base = DRAM_0_PHYS_ADDR_START;
            pMemory->osRegion[1].length = 0x20000000; /* 512MB */
        #elif (BCHP_CHIP==7422)
            pMemory->osRegion[1].base = DRAM_0_PHYS_ADDR_START;
            pMemory->osRegion[1].length = 0x0c000000; /* 192MB */
            pMemory->osRegion[2].base = DRAM_1_1024MB_PHYS_ADDR_START;
            pMemory->osRegion[2].length = 0x40000000; /* 1024MB */
        #elif (BCHP_CHIP==7425)
            pMemory->osRegion[1].base = DRAM_0_PHYS_ADDR_START;
            pMemory->osRegion[1].length = 0x1ca00000; /* 458MB */
            pMemory->osRegion[2].base = DRAM_1_PHYS_ADDR_START;
            pMemory->osRegion[2].length = 0x40000000; /* 1024MB */
        #elif (BCHP_CHIP==7435)
            pMemory->osRegion[1].base = DRAM_0_PHYS_ADDR_START;
            pMemory->osRegion[1].length = 0x20000000; /* 512MB */
            pMemory->osRegion[2].base = DRAM_1_PHYS_ADDR_START; // 0x90000000;
            pMemory->osRegion[2].length = 0x20000000; /* 512MB */
        #elif (BCHP_CHIP==7429) || (BCHP_CHIP==74295)
            pMemory->osRegion[1].base = DRAM_0_PHYS_ADDR_START;
            pMemory->osRegion[1].length = 0x30000000; /* 768MB */
        #endif
    }

    {
        int i;
        for (i=0; i<NEXUS_MAX_HEAPS; i++)
            BDBG_MSG(("pMemory->osRegion[%d].base=0x%x, pMemory->osRegion[%d].length=%d MB", i, pMemory->osRegion[i].base, i, pMemory->osRegion[i].length/(1024*1024)));
    }
#endif

    return BERR_SUCCESS;
}

void
NEXUS_Platform_P_ResetInterrupts(void)
{
    /* not needed */
}

/* ISR handler, calls L1 interrupt handler */
static void __attribute__((no_instrument_function))
NEXUS_Platform_P_Isr(unsigned long data)
{
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
#if UCOS_VERSION==1
    UBYTE ucosErr;
#elif UCOS_VERSION==3
    OS_ERR err;
    CPU_TS ts;
    CPU_SR_ALLOC();
#endif

    BSTD_UNUSED(data);

/*
    if(!state->started) {
        goto done;
    }
*/

    /* Loop until all is cleared */
    for(;;) {
        uint32_t re_enable[NEXUS_NUM_L1_REGISTERS];
        uint32_t status;
        unsigned bit;
        unsigned i;

        /* Suspend here until resumed by the ISR */
#if UCOS_VERSION==1
        OSSemPend(pEvent, 0, &ucosErr);
#elif UCOS_VERSION==3
        OSTaskSemPend(0,
                      OS_OPT_PEND_BLOCKING,
                      &ts,
                      &err);
        if (err != OS_ERR_NONE) {
            BDBG_ERR(("OSTaskSemPend did not return OS_ERR_NONE\n"));
        }
#else
    #error unknown UCOS_VERSION
#endif

        /* Mask interrupts only to read current status */
#if UCOS_VERSION==1
        OS_ENTER_CRITICAL();
#elif UCOS_VERSION==3
        CPU_CRITICAL_ENTER();
#else
    #error unknown UCOS_VERSION
#endif

        for(status=0,i=0;i<NEXUS_NUM_L1_REGISTERS;i++) {
            /* swap pending and current interrupt, then clear pending  and reenable interrupts */
            status |= state->pending[i].IntrStatus;
            /* coverity[use] */
            state->processing[i].IntrStatus = state->pending[i].IntrStatus;
            /* mask interrupts */
            state->processing[i].IntrStatus &= ~state->processing[i].IntrMaskStatus;

            /* Delay reenabling interrupts until after they have been serviced */
            /* disable_irq nests, so if the interrupt handler disables the interrupt */
            /* again, this is still safe. */
            re_enable[i] = state->processing[i].IntrStatus & state->pending[i].IntrStatus;

            /* clear list of delayed interrupts */
            state->pending[i].IntrStatus = 0;
        }

        /* Restore interrupts */
#if UCOS_VERSION==1
        OS_EXIT_CRITICAL();
#elif UCOS_VERSION==3
        CPU_CRITICAL_EXIT();
#else
        #error unknown UCOS_VERSION
#endif

        if(status==0) {
            continue;
        }


        /* then call L1 handlers inside critical section (KNI serializes with tasklet_disable so we do nothing here) */
        for(bit=0; bit<NUM_IRQS ; bit+=32) {

            status = state->processing[bit/32].IntrStatus;

            if(!status) {
                continue;
            }
            for(i=0;i<32;i++) {
                if(status & (1<<i)) {
                    unsigned irq = i+bit;
                    /* print on runaway L1 */
                    if (++state->table[irq].count % 10000 == 0) {
                        if (!state->table[irq].print /*TBD && !nexus_driver_state.uninit_pending*/) {
                            BDBG_ERR(("<0>### %s (W%d, bit %d) fired %d times\n", state->table[irq].name, irq/32, irq%32, state->table[irq].count));
                            state->table[irq].print = true; /* only print once to maximize chance that system keeps running */
                        }
                    }

                    state->table[irq].handler(state->table[irq].context_ptr, state->table[irq].context_int);
                }
            }
        }

        /* Now, restore any disabled interrupts (masking not required) */
        for(bit=0; bit<NUM_IRQS ; bit+=32) {
            status = re_enable[bit/32];
            if (!status)  {
                continue;
            }
            for(i=0;i<32;i++)
            {
                /* only enable interrupts which are not masked by the software */
                if (status & (1<<i))
                {
                    BDBG_MSG_IRQ(("BH enable[irq] %d", LINUX_IRQ(i+bit)));
                    if (!state->table[i+bit].special_handler)
                    {
                        if (state->table[i+bit].enabled && !state->table[i+bit].taskletEnabled) {
                            state->table[i+bit].taskletEnabled = true;
                            CPUINT1_Enable(LINUX_IRQ(i+bit));
                        }
                    }
                }
            }
        }
    }

    return;
}

void NEXUS_Platform_P_LinuxIsr(void *dev_id, int linux_irq)
{
    struct b_bare_interrupt_entry *entry = dev_id;
    unsigned irq = NEXUS_IRQ(linux_irq);
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    unsigned i;
#if UCOS_VERSION==1
    OS_STATUS ucosStatus;
#elif UCOS_VERSION==3
    CPU_SR_ALLOC();
    OS_ERR err;
#endif

    if (irq >= NUM_IRQS) {
        goto error;
    }

    /* Make sure we're serialized with the tasklet across multiple CPUs */
#if UCOS_VERSION==1
    OS_ENTER_CRITICAL();
#elif UCOS_VERSION==3
    CPU_CRITICAL_ENTER();
#else
    #error unknown UCOS_VERSION
#endif

    /* disable irq */
    BDBG_MSG_IRQ(("TH disable[irq] %d", linux_irq));
    if ( !entry->special_handler ) {
        if ( entry->taskletEnabled ) {
            entry->taskletEnabled = false;
            CPUINT1_Disable(linux_irq);
        }
    } else {
        entry->special_handler(irq);
    }

    for(i=0;i<NEXUS_NUM_L1_REGISTERS;i++,irq-=32) {
        if(irq<32) {
            state->pending[i].IntrStatus |= 1<<irq;
            break;
        }
    }

    /* This needs to run as a high-priority tasklet, which will immediately follow */
#if UCOS_VERSION==1
    ucosStatus = OSSemPost(pEvent);
    if (ucosStatus != OS_NO_ERR) {
        BDBG_ERR(("OSSemPost did not return OS_NO_ERR, ucosStatus=%d\n", ucosStatus));
        BKNI_Fail();
    }
#elif UCOS_VERSION==3
    OSTaskSemPost(&IsrTaskTCB,
                  OS_OPT_POST_NONE,
                  &err);
    if (err != OS_ERR_NONE) {
        BDBG_ERR(("OSTaskSemPost did not return OS_ERR_NONE, err=%d\n", err));
        BKNI_Fail();
    }
#else
    #error unknown UCOS_VERSION
#endif

#if UCOS_VERSION==1
    OS_EXIT_CRITICAL();
#elif UCOS_VERSION==3
    CPU_CRITICAL_EXIT();
#else
    #error unknown UCOS_VERSION
#endif

    return;

error:
    BDBG_ASSERT(0); /* fail hard on unknown irq */
#if 0
    BDBG_WRN(("unknown irq %d", linux_irq));
    CPUINT1_Disable(linux_irq);
#endif
    return;
}

NEXUS_Error NEXUS_Platform_P_ConnectInterrupt(unsigned irqNum,
    NEXUS_Core_InterruptFunction handler, void *context_ptr, int context_int)
{
    struct NEXUS_Platform_P_IrqMap *nexusMap = NEXUS_Platform_P_IrqMap;
    const char *name=NULL;
    bool shared = false;
    b_bare_os_special_interrupt_handler special_handler = NULL;
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    struct b_bare_interrupt_entry *entry;
#if UCOS_VERSION==3
    CPU_SR_ALLOC();
#endif
    unsigned i;

    if (irqNum>=NUM_IRQS || handler==NULL || !state->started) {
        return BERR_TRACE(-1);
    }

    /* search the nexus map first */
    for (i=0;nexusMap[i].name;i++) {
        if (nexusMap[i].linux_irq == (int16_t)(irqNum+1)) {
            name = nexusMap[i].name;
            shared = nexusMap[i].shared;
            special_handler = nexusMap[i].special_handler;
            break;
        }
    }
    if (name==NULL) {
        const BINT_P_IntMap *intMap;
        /* use BINT's record of managed L2's (and their corresponding L1's) to validate the L1 connect */
        intMap = g_pCoreHandles->bint_map;
        BDBG_ASSERT(intMap);

        /* find the first L2 that has this L1 */
        for (i=0;intMap[i].L1Shift!=-1;i++) {
            if ((unsigned)BINT_MAP_GET_L1SHIFT(&intMap[i]) == irqNum) {
                name = intMap[i].L2Name; /* use BINT's L2 name for the L1 name. in most cases it is a meaningful name. */
                shared = true;
                special_handler = NULL;
                break;
            }
        }
        if (intMap[i].L1Shift == -1) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
    }

    entry = &state->table[irqNum];
    if (entry->handler) {
        /* can't overwrite old handler, b_bare_disconnect_interrupt shall be called first */
        return BERR_TRACE(-1);
    }

#if UCOS_VERSION==1
    OS_ENTER_CRITICAL();
#elif UCOS_VERSION==3
    CPU_CRITICAL_ENTER();
#else
    #error unknown UCOS_VERSION
#endif
    entry->name = name;
    entry->handler = handler;
    entry->context_ptr = context_ptr;
    entry->context_int = context_int;
    entry->special_handler = special_handler;
    entry->shared = shared;
    BDBG_ASSERT(!entry->enabled);
    BDBG_ASSERT(!entry->taskletEnabled);
    BDBG_ASSERT(!entry->requested);
    /* request_irq deferred to first enable. */
#if UCOS_VERSION==1
    OS_EXIT_CRITICAL();
#elif UCOS_VERSION==3
    CPU_CRITICAL_EXIT();
#else
    #error unknown UCOS_VERSION
#endif

    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_MonitorOS(void)
{
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    unsigned irq;
    for (irq=0;irq<NUM_IRQS;irq++) {
        if (state->table[irq].print) {
            unsigned i = irq / 32;
            unsigned bit = irq % 32;
            BDBG_WRN(("%s (W%d, bit %d) fired %d times", state->table[irq].name, i, bit, state->table[irq].count));
        }
    }
    BKNI_EnterCriticalSection();
    for (irq=0;irq<NUM_IRQS;irq++) {
        state->table[irq].count = 0;
        state->table[irq].print = false;
    }
    BKNI_LeaveCriticalSection();
}

NEXUS_Error NEXUS_Platform_P_EnableInterrupt_isr( unsigned irqNum)
{
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    struct b_bare_interrupt_entry *entry;
    unsigned reg = irqNum/32;
#if UCOS_VERSION==3
    CPU_SR_ALLOC();
#endif

    if (irqNum>=NUM_IRQS || !state->started) {
        return BERR_TRACE(-1);
    }
    entry = &state->table[irqNum];
    if (!entry->handler) {
        return BERR_TRACE(-1);
    }

#if UCOS_VERSION==1
    OS_ENTER_CRITICAL();
#elif UCOS_VERSION==3
    CPU_CRITICAL_ENTER();
#else
    #error unknown UCOS_VERSION
#endif
    state->processing[reg].IntrMaskStatus &= ~(1 << (irqNum%32));
    if (!entry->requested) {
#if 0
        int irqflags;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        irqflags = 0; /* later versions of linux always enable interrupts on request_irq */
        if (entry->shared) {
            irqflags |= IRQF_SHARED;
        }
#else
        irqflags = SA_INTERRUPT;
        if (entry->shared) {
            irqflags |= SA_SHIRQ;
        }
#endif
#endif

#if UCOS_VERSION==1
        OS_EXIT_CRITICAL();
#elif UCOS_VERSION==3
        CPU_CRITICAL_EXIT();
#else
        #error unknown UCOS_VERSION
#endif
        BDBG_MSG(("connect interrupt %s %d (%d, %p)", entry->name, LINUX_IRQ(irqNum), entry->shared, entry->special_handler));
        entry->taskletEnabled = true; /* set before call request_irq as irq can immediately fire */
        if (!CPUINT1_ConnectIsr(irqNum, NEXUS_Platform_P_LinuxIsr, entry, LINUX_IRQ(irqNum))) {
            /* disable */
#if UCOS_VERSION==1
            OS_ENTER_CRITICAL();
#elif UCOS_VERSION==3
            CPU_CRITICAL_ENTER();
#else
            #error unknown UCOS_VERSION
#endif
            entry->handler = NULL;
            state->processing[reg].IntrMaskStatus |= (1 << (irqNum%32));
#if UCOS_VERSION==1
            OS_EXIT_CRITICAL();
#elif UCOS_VERSION==3
            CPU_CRITICAL_EXIT();
#else
            #error unknown UCOS_VERSION
#endif
            return BERR_TRACE(-1);
        }
        CPUINT1_Enable(LINUX_IRQ(irqNum));
        entry->requested = true;
        entry->enabled = true;
        return 0;
    }
    else if (!entry->enabled) {
        BDBG_MSG(("enable interrupt %d", LINUX_IRQ(irqNum)));
        if (!entry->special_handler) {
            BDBG_ASSERT(!entry->taskletEnabled);

            if ( !entry->taskletEnabled ) {
                entry->taskletEnabled = true;
                CPUINT1_Enable(LINUX_IRQ(irqNum));
            }
        }
        entry->enabled = true;
    }
#if UCOS_VERSION==1
    OS_EXIT_CRITICAL();
#elif UCOS_VERSION==3
    CPU_CRITICAL_EXIT();
#else
    #error unknown UCOS_VERSION
#endif
    return 0;
}

void NEXUS_Platform_P_DisableInterrupt_isr( unsigned irqNum)
{
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    struct b_bare_interrupt_entry *entry;
    unsigned reg = irqNum/32;
#if UCOS_VERSION==3
    CPU_SR_ALLOC();
#endif

    if (irqNum>=NUM_IRQS) {
        BDBG_ASSERT(0);
/*        return BERR_TRACE(-1); */
    }
    entry = &state->table[irqNum];
    if (!entry->handler) {
        BERR_TRACE(-1);
        return;
    }

    if (entry->enabled) {
        BDBG_ASSERT(entry->requested);
        BDBG_MSG(("disable interrupt %d", LINUX_IRQ(irqNum)));
#if UCOS_VERSION==1
        OS_ENTER_CRITICAL();
#elif UCOS_VERSION==3
        CPU_CRITICAL_ENTER();
#else
        #error unknown UCOS_VERSION
#endif
        state->processing[reg].IntrMaskStatus |= (1 << (irqNum%32));
        if (!entry->special_handler) {
            /* If the TH has received the interrupt but it has not been processed by the tasklet, don't nest the disable call. */
            if ( entry->taskletEnabled )
            {
                CPUINT1_Disable(LINUX_IRQ(irqNum));
                entry->taskletEnabled = false;
            }
        }
        entry->enabled = false;
#if UCOS_VERSION==1
        OS_EXIT_CRITICAL();
#elif UCOS_VERSION==3
        CPU_CRITICAL_EXIT();
#else
        #error unknown UCOS_VERSION
#endif
    }
}

void NEXUS_Platform_P_DisconnectInterrupt( unsigned irqNum)
{
    int rc;
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    struct b_bare_interrupt_entry *entry;
    unsigned reg = irqNum/32;
#if UCOS_VERSION==3
    CPU_SR_ALLOC();
#endif

    if(irqNum>=NUM_IRQS) {
        rc = BERR_TRACE(-1);
        return;
    }
    entry = &state->table[irqNum];
    if (!entry->handler) {
        BERR_TRACE(-1);
        return;
    }

#if UCOS_VERSION==1
    OS_ENTER_CRITICAL();
#elif UCOS_VERSION==3
    CPU_CRITICAL_ENTER();
#else
    #error unknown UCOS_VERSION
#endif
    BDBG_MSG(("disconnect interrupt %d", LINUX_IRQ(irqNum)));
    entry->handler = NULL;
    if (entry->enabled) {
        state->processing[reg].IntrMaskStatus |= (1 << (irqNum%32));
        entry->enabled = false;
    }
    if (entry->requested) {
        entry->requested = false;
        entry->taskletEnabled = false;
        /* kernel can sleep in free_irq, so must release the spinlock first */
        CPUINT1_Disable(LINUX_IRQ(irqNum));
        CPUINT1_DisconnectIsr(LINUX_IRQ(irqNum));
    }
#if UCOS_VERSION==1
    OS_EXIT_CRITICAL();
#elif UCOS_VERSION==3
    CPU_CRITICAL_EXIT();
#else
    #error unknown UCOS_VERSION
#endif
}

NEXUS_Error
NEXUS_Platform_P_EnableInterrupt( unsigned irqNum)
{
    NEXUS_Error rc;

    BKNI_EnterCriticalSection();
    rc = NEXUS_Platform_P_EnableInterrupt_isr(irqNum);
    BKNI_LeaveCriticalSection();

    return rc;
}

void
NEXUS_Platform_P_DisableInterrupt( unsigned irqNum)
{
    BKNI_EnterCriticalSection();
    NEXUS_Platform_P_DisableInterrupt_isr(irqNum);
    BKNI_LeaveCriticalSection();
    return;
}

void NEXUS_Platform_P_Os_SystemUpdate32_isrsafe(const NEXUS_Core_PreInitState *preInitState, uint32_t reg, uint32_t mask, uint32_t value, bool systemRegister)
{
    uint32_t temp_value;
#if NEXUS_CPU_ARM
#define REG_BASE BCHP_PHYSICAL_OFFSET
#else
#define REG_BASE BCM_PHYS_TO_KSEG1(BCHP_PHYSICAL_OFFSET)
#endif
#if UCOS_VERSION==3
    CPU_SR_ALLOC();
#endif

    BSTD_UNUSED(preInitState);
    BSTD_UNUSED(systemRegister);

#if UCOS_VERSION==1
    OS_ENTER_CRITICAL();
#elif UCOS_VERSION==3
    CPU_CRITICAL_ENTER();
#else
    #error unknown UCOS_VERSION
#endif

    temp_value = *((volatile uint32_t *)(REG_BASE | reg));
    temp_value = temp_value & ~mask;
    temp_value = temp_value | (mask & value);
    *((volatile uint32_t *)(REG_BASE | reg)) = temp_value;

#if UCOS_VERSION==1
    OS_EXIT_CRITICAL();
#elif UCOS_VERSION==3
    CPU_CRITICAL_EXIT();
#else
    #error unknown UCOS_VERSION
#endif
    return;
}

void NEXUS_Platform_P_AtomicUpdateCallback_isr(void *callbackContext, uint32_t reg, uint32_t mask, uint32_t value)
{
    uint32_t temp;
#if UCOS_VERSION==3
    CPU_SR_ALLOC();
#endif

    BSTD_UNUSED(callbackContext);

    /* this spinlock synchronizes with any kernel use of a set of shared registers.
    see BREG_P_CheckAtomicRegister in magnum/basemodules/reg/breg_mem.c for the list of registers. */
#if UCOS_VERSION==1
    OS_ENTER_CRITICAL();
#elif UCOS_VERSION==3
    CPU_CRITICAL_ENTER();
#else
    #error unknown UCOS_VERSION
#endif

    /* read/modify/write */
    temp = BREG_Read32(g_pCoreHandles->reg, reg);
    temp &= ~mask;
    temp |= value;
    BREG_Write32(g_pCoreHandles->reg, reg, temp);

#if UCOS_VERSION==1
    OS_EXIT_CRITICAL();
#elif UCOS_VERSION==3
    CPU_CRITICAL_EXIT();
#else
    #error unknown UCOS_VERSION
#endif
}


static bool g_NEXUS_magnum_init = false;
NEXUS_Error
NEXUS_Platform_P_Magnum_Init(void)
{
    BERR_Code rc;

    if(!g_NEXUS_magnum_init) {
        rc = BKNI_Init();
        if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
        rc = BDBG_Init();
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); BKNI_Uninit();return rc;}
        rc = NEXUS_Base_Core_Init();
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); BDBG_Uninit();BKNI_Uninit();return rc;}
        g_NEXUS_magnum_init = true;
    }
    return BERR_SUCCESS;
}

void
NEXUS_Platform_P_Magnum_Uninit(void)
{
    if(g_NEXUS_magnum_init) {
        NEXUS_Base_Core_Uninit();
        BDBG_Uninit();
        BKNI_Uninit();
        g_NEXUS_magnum_init = false;
    }
    return;
}

void NEXUS_Platform_P_StopCallbacks(void *interfaceHandle)
{
    NEXUS_Base_P_StopCallbacks(interfaceHandle);
    return;
}

void NEXUS_Platform_P_StartCallbacks(void *interfaceHandle)
{
    NEXUS_Base_P_StartCallbacks(interfaceHandle);
    return;
}

unsigned NEXUS_Platform_P_ReadBoardId(void)
{
#ifdef MIPS_SDE
    return 0;
#else
    return xapi->xfd_board_id;
#endif
}
NEXUS_Error NEXUS_Platform_P_DropPrivilege(const NEXUS_PlatformSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    /* drop is performed in linuxuser.proxy */
    return 0;
}

#if !NEXUS_PLATFORM_P_READ_BOX_MODE
unsigned NEXUS_Platform_P_ReadBoxMode(void)
{
#ifdef MIPS_SDE
    return 0;
#else
    return xapi->xfn_boxmode();
#endif
}
#endif

unsigned NEXUS_Platform_P_ReadPMapId(void)
{
#if 0
    const char *env = NEXUS_GetEnv("B_REFSW_PMAP_ID");
    return env?NEXUS_atoi(env):0;
#else
    return 0;
#endif
}

NEXUS_Error NEXUS_Platform_P_SetStandbyExclusionRegion(unsigned heapIndex)
{
#if defined(BRCMSTB_HAS_PM_MEM_EXCLUDE)
    NEXUS_MemoryStatus status;
    int rc;

    rc = NEXUS_Heap_GetStatus(g_pCoreHandles->heap[heapIndex].nexus, &status);
    if (rc) return BERR_TRACE(rc);

    BDBG_WRN(("brcmstb_pm_mem_exclude(" BDBG_UINT64_FMT ", %d)", BDBG_UINT64_ARG(status.offset), status.size));
    rc = brcmstb_pm_mem_exclude(status.offset, status.size);
    if (rc) return BERR_TRACE(rc);
    return 0;
#else
    BSTD_UNUSED(heapIndex);
    /* ignore */
    return 0;
#endif
}

NEXUS_Error NEXUS_Platform_P_InitializeThermalMonitor(void)
{
    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_UninitializeThermalMonitor(void)
{
    return;
}

bool NEXUS_Platform_P_IsGisbTimeoutAvailable(void)
{
    bool is_available = true;
#if BDBG_DEBUG_BUILD && !defined(NEXUS_CPU_ARM)
#if defined(CONFIG_BRCMSTB_GISB_ARB)
    is_available = false;
#else
    is_available = true;
#endif
#endif
    return is_available;
}

NEXUS_Error NEXUS_Platform_P_AddDynamicRegion(NEXUS_Addr addr, unsigned size)
{
    BSTD_UNUSED(addr);
    BSTD_UNUSED(size);
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Platform_P_RemoveDynamicRegion(NEXUS_Addr addr, unsigned size)
{
    BSTD_UNUSED(addr);
    BSTD_UNUSED(size);
    return NEXUS_SUCCESS;
}

bool NEXUS_Platform_P_SharedGpioSupported(void)
{
    return false;
}

NEXUS_Error NEXUS_Platform_P_UpdateIntSettings(BINT_Settings *pIntSettings)
{
    BSTD_UNUSED(pIntSettings);
    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_GetGpioModuleOsSharedBankSettings(NEXUS_GpioModuleOsSharedBankSettings * pSettings)
{
    BSTD_UNUSED(pSettings);
}

#if NEXUS_USE_CMA
/* macros found in magnum/basemodules/chp */
#include "../../src/common/bchp_memc_offsets_priv.h"
NEXUS_Error NEXUS_Platform_P_CalcSubMemc(const NEXUS_Core_PreInitState *preInitState, NEXUS_PlatformMemoryLayout *pMemory)
{
    unsigned i;
    BCHP_MemoryInfo memoryInfo;

    BCHP_GetMemoryInfo_PreInit(preInitState->hReg, &memoryInfo);

    BDBG_CASSERT(NEXUS_MAX_MEMC <= sizeof(pMemory->memc)/sizeof(pMemory->memc[0]));
    for (i=0;i<NEXUS_MAX_MEMC;i++) {
        if (!memoryInfo.memc[i].valid) continue;
        switch (i) {
        case 0: pMemory->memc[i].region[0].addr = BCHP_P_MEMC_0_OFFSET; break;
#ifdef BCHP_P_MEMC_1_OFFSET
        case 1: pMemory->memc[i].region[0].addr = BCHP_P_MEMC_1_OFFSET; break;
#endif
#ifdef BCHP_P_MEMC_2_OFFSET
        case 2: pMemory->memc[i].region[0].addr = BCHP_P_MEMC_2_OFFSET; break;
#endif
        default:
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }

        /* this size calculation is not valid for LPDDR4. only use for older silicon. */
        pMemory->memc[i].size = (uint64_t)memoryInfo.memc[i].deviceTech / 8 * (memoryInfo.memc[i].width/memoryInfo.memc[i].deviceWidth) * 1024 * 1024;
        pMemory->memc[i].region[0].size = pMemory->memc[i].size;
        if (pMemory->memc[i].region[0].size > 1024*1024*1024) {
            pMemory->memc[i].region[0].size = 1024*1024*1024;
        }
    }
    return NEXUS_SUCCESS;
}

NEXUS_Addr NEXUS_Platform_P_AllocCma(const NEXUS_PlatformMemory *pMemory, unsigned memcIndex, unsigned subIndex, unsigned size, unsigned alignment)
{
    BSTD_UNUSED(pMemory);
    BSTD_UNUSED(memcIndex);
    BSTD_UNUSED(subIndex);
    BSTD_UNUSED(size);
    BSTD_UNUSED(alignment);
    return 0;
}

void NEXUS_Platform_P_FreeCma(const NEXUS_PlatformMemory *pMemory, unsigned memcIndex, unsigned subIndex, NEXUS_Addr addr, unsigned size)
{
    BSTD_UNUSED(pMemory);
    BSTD_UNUSED(memcIndex);
    BSTD_UNUSED(subIndex);
    BSTD_UNUSED(addr);
    BSTD_UNUSED(size);
}
#endif

NEXUS_Error nexus_platform_p_add_proc(NEXUS_ModuleHandle module, const char *filename, const char *module_name, void (*dbgPrint)(void))
{
    BSTD_UNUSED(module);
    BSTD_UNUSED(filename);
    BSTD_UNUSED(module_name);
    BSTD_UNUSED(dbgPrint);
    return NEXUS_SUCCESS;
}

void nexus_platform_p_remove_proc(NEXUS_ModuleHandle module, const char *filename)
{
    BSTD_UNUSED(module);
    BSTD_UNUSED(filename);
    return;
}

bool NEXUS_Platform_P_IsOs64(void)
{
    return false;
}

BCHP_PmapSettings *NEXUS_Platform_P_ReadPMapSettings(void)
{
    return NULL;
}

void NEXUS_Platform_P_FreePMapSettings(BCHP_PmapSettings *preInitState)
{
    BSTD_UNUSED(preInitState);
    return;
}
