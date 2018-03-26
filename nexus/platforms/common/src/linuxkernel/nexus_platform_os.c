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
#include "nexus_platform_virtual_irq.h"
#include "b_virtual_irq.h"
#if NEXUS_HAS_GPIO
#include "nexus_platform_shared_gpio.h"
#include "b_shared_gpio.h"
#endif
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,1)
#include <linux/kconfig.h>
#include <linux/dma-mapping.h>
#include <linux/uaccess.h>
#undef BCHP_PHYSICAL_OFFSET
#include <linux/brcmstb/brcmstb.h> /* for SWLINUX-3535 */
#undef BCHP_PHYSICAL_OFFSET
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/kmod.h>
#include <linux/sched.h>

#define LINUX_EXTRAVERSION(MAJOR,MINOR) ((MAJOR)<<8 | (MINOR))
#define LINUX_EXTRAVERSION_CODE LINUX_EXTRAVERSION(LINUX_EXTRAVERSION_MAJOR,LINUX_EXTRAVERSION_MINOR)

#ifndef USE_LIBFDT
#if NEXUS_CPU_ARM || NEXUS_CPU_ARM64
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)) || (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,28) && LINUX_EXTRAVERSION_CODE >= LINUX_EXTRAVERSION(1,13))
#define USE_LIBFDT 1
#endif
#endif
#endif

#if USE_LIBFDT
#include "libfdt.h"
#include <linux/of_fdt.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,1)
    static spinlock_t g_magnum_spinlock = __SPIN_LOCK_UNLOCKED(g_magnum_spinlock);
    #define brcm_magnum_spinlock g_magnum_spinlock
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
    #include <linux/kconfig.h>
    #include <linux/brcmstb/brcmapi.h>
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
/* DTV */
#if  BMIPS_ZEPHYR_40NM
    #include <brcmapi.h>
    #ifndef BRCM_MAGNUM_SPINLOCK
        static spinlock_t g_magnum_spinlock = SPIN_LOCK_UNLOCKED;
    #else
        extern spinlock_t g_magnum_spinlock;
    #endif
    #define brcm_magnum_spinlock g_magnum_spinlock
    #define UMH_WAIT_PROC 1
#else
    #include <asm/brcmstb/brcmapi.h>
#endif
#else
    #include <asm/uaccess.h>
    #include <asm/brcmstb/common/brcmstb.h>
    #ifndef BRCM_MAGNUM_SPINLOCK
    static spinlock_t g_magnum_spinlock = SPIN_LOCK_UNLOCKED;
    #else
    extern spinlock_t g_magnum_spinlock;
    #endif
    #define brcm_magnum_spinlock g_magnum_spinlock
    #define UMH_WAIT_PROC 1
#endif

#ifndef BRCMSTB_H_VERSION
#define BRCMSTB_H_VERSION   0
#endif

#if BRCMSTB_H_VERSION >= 5
#define NEXUS_VERSION_MAJOR NEXUS_PLATFORM_VERSION_MAJOR
#define NEXUS_VERSION_MINOR NEXUS_PLATFORM_VERSION_MINOR
#include "linux/brcmstb/nexus_version_check.h"
#undef NEXUS_VERSION_MAJOR
#undef NEXUS_VERSION_MINOR
#endif

#define BDBG_TRACE_L2(x) /*BDBG_MSG(x)*/

BDBG_MODULE(nexus_platform_os);


/* TODO: need a Linux macro to detect 2631-3.2 or greater */
#if BMIPS4380_40NM || BMIPS5000_40NM || BMIPS_ZEPHYR_40NM 
#define LINUX_2631_3_2_OR_GREATER 1
#endif

#undef PDEBUG
#undef PWARN
#undef PERR
#undef PINFO

#define DEBUG
#ifdef DEBUG
    #define PERR(fmt, args...) printk( KERN_ERR "nexus.ko: " fmt, ## args)
    #define PWARN(fmt, args...) printk( KERN_WARNING "nexus.ko: " fmt, ## args)
    #define PINFO(fmt, args...) printk( KERN_INFO "nexus.ko: " fmt, ## args)
    #define PDEBUG(fmt, args...) printk( KERN_DEBUG "nexus.ko: " fmt, ## args)
#else
    #define PERR(fmt, args...)
    #define PWARN(fmt, args...)
    #define PINFO(fmt, args...)
    #define PDEBUG(fmt, args...)
#endif

#if NEXUS_CPU_ARM
#include "bcmdriver_arm.h"
#endif

/*
The interrupt code is meant to stay in sync with nexus/build/nfe_driver/b_bare_os.c's 
interrupt code.
*/

typedef void (*b_bare_os_special_interrupt_handler)(int linux_irq);
#define BDBG_MSG_IRQ(X) /*BDBG_MSG(X)*/

#if NEXUS_CPU_ARM || LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
/*
MIPS 3.3 and 3.14 uses old API's. MIPS 4.1 and beyond uses new API's.
All ARM (3.8 and beyond) uses new API's.
So, for purposes of this code, we'll call these new API's ARM_LINUX_APIS.
*/
#define ARM_LINUX_APIS 1
#endif

#define NUM_IRQS (32*NEXUS_NUM_L1_REGISTERS)
/* b_bare_os L1 interrupts are 0-based. linux is 1-based. */
#if ARM_LINUX_APIS
#define LINUX_IRQ(i) (i+32)
#define NEXUS_IRQ(i) (i-32)
#else
#define LINUX_IRQ(i) (i+1)
#define NEXUS_IRQ(i) (i-1)
#endif

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
    bool virtual;
};

static struct b_bare_interrupt_state {
    spinlock_t lock;
    bool started;
    struct {
        uint32_t IntrStatus;
        uint32_t IntrMaskStatus;
    } processing[NEXUS_NUM_L1_REGISTERS], pending[NEXUS_NUM_L1_REGISTERS];
    struct b_bare_interrupt_entry table[NUM_IRQS];
    struct work_struct task;
} b_bare_interrupt_state = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
    __SPIN_LOCK_UNLOCKED(b_bare_interrupt_state.lock),
#else
    SPIN_LOCK_UNLOCKED,
#endif
};

static void NEXUS_Platform_P_Isr(unsigned long data);
static DECLARE_TASKLET(NEXUS_Platform_P_IsrTasklet, NEXUS_Platform_P_Isr, 0);

static void NEXUS_Platform_P_InitSubmodules(void)
{
    NEXUS_Platform_P_InitVirtualIrqSubmodule();
#if NEXUS_HAS_GPIO
    NEXUS_Platform_P_InitSharedGpioSubmodule();
#endif
}

NEXUS_Error
NEXUS_Platform_P_InitOS(void)
{
    NEXUS_Error rc;
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    unsigned i;

    memset(&state->table, 0, sizeof(state->table));
    for(i=0;i<NEXUS_NUM_L1_REGISTERS;i++) {
        state->processing[i].IntrMaskStatus = ~0;
        state->processing[i].IntrStatus = 0;
        state->pending[i] = state->processing[i];
    }
    state->started = true;

    NEXUS_Platform_P_InitSubmodules();

    rc = BKNI_LinuxKernel_SetIsrTasklet(&NEXUS_Platform_P_IsrTasklet);
    if ( rc!=BERR_SUCCESS ) { rc = BERR_TRACE(rc); goto err_tasklet;}

    rc = nexus_driver_scheduler_init();
    if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc);goto err_driver; }
    
    return NEXUS_SUCCESS;
    
err_driver:
err_tasklet:
    return rc;
}

static void NEXUS_Platform_P_UninitSubmodules(void)
{
#if NEXUS_HAS_GPIO
    NEXUS_Platform_P_UninitSharedGpioSubmodule();
#endif
    NEXUS_Platform_P_UninitVirtualIrqSubmodule();
}

NEXUS_Error
NEXUS_Platform_P_UninitOS(void)
{
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    unsigned i;

    nexus_driver_server_close_module_headers();

    nexus_driver_scheduler_uninit();

    state->started = false;
    for(i=0;i<NUM_IRQS;i++) {
        if (state->table[i].handler) {
            NEXUS_Platform_P_DisconnectInterrupt(i);
        }
    }

    NEXUS_Platform_P_UninitSubmodules();

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Platform_P_InitOSMem()
{
    /* use g_platformMemory to pass OS value to NEXUS_Platform_P_SetCoreModuleSettings */
    g_platformMemory.maxDcacheLineSize = nexus_driver_state.settings.maxDcacheLineSize;
    return 0;
}

void NEXUS_Platform_P_UninitOSMem()
{
    /* nothing required */
}

#if ARM_LINUX_APIS
#define NEXUS_PLATFORM_LINUX_PAGE_SIZE 4096
#if BRCMSTB_H_VERSION < 4
#include <linux/brcmstb/cma_driver.h>
#endif

void* NEXUS_Platform_P_CmaVmap(NEXUS_Addr phyAddress, unsigned long length, bool cache)
{
    void *addr = NULL;
#if BRCMSTB_H_VERSION >= 4
    BDBG_MSG(("cma:mapping %lu bytes @ physical address " BDBG_UINT64_FMT ", cache %d",
              length, BDBG_UINT64_ARG(phyAddress), cache));

    if(!cache) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }
    if(length % NEXUS_PLATFORM_LINUX_PAGE_SIZE){
        BDBG_ERR(("%lu not page aligned", length));
        BERR_TRACE(BERR_OS_ERROR);
        return NULL;
    }
    addr = brcmstb_memory_kva_map_phys(phyAddress, length, cache);
#else /* BRCMSTB_H_VERSION < 4 */
    struct page *page;
    unsigned num_pages = length/NEXUS_PLATFORM_LINUX_PAGE_SIZE;

    BDBG_MSG(("cma:mapping %u bytes @ physical address " BDBG_UINT64_FMT ", cache %d, pages %d",
              (unsigned)length, BDBG_UINT64_ARG(phyAddress), cache,num_pages ));

    if(!cache) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }
    if(length % NEXUS_PLATFORM_LINUX_PAGE_SIZE){
        BDBG_ERR(("%u not page aligned", (unsigned)length));
        BERR_TRACE(BERR_OS_ERROR);
        return NULL;
    }
    page = (struct page *)phys_to_page(phyAddress);
    addr = cma_dev_kva_map(page, num_pages, PAGE_KERNEL);
#endif /* BRCMSTB_H_VERSION < 4 */
    if (!addr){
        BDBG_ERR(("cma:vmap addr " BDBG_UINT64_FMT ", length %lu bytes failed", BDBG_UINT64_ARG(phyAddress), length));
        BERR_TRACE(BERR_OS_ERROR);
        return NULL;
    }
    BDBG_MSG(("cma:mapped. %p", addr));
    return addr;
}

void NEXUS_Platform_P_CmaVumap(void* virtAddr)
{
    if(!virtAddr) {
        BDBG_ERR(("cma:Invalid address"));
    }
    else{
#if BRCMSTB_H_VERSION >= 4
        brcmstb_memory_kva_unmap(virtAddr);
#else
        cma_dev_kva_unmap(virtAddr);
#endif
    }
}
#endif /* ARM_LINUX_APIS */

/* IMPORTANT: the logic in NEXUS_Platform_P_MapMemory must be carefully paired with NEXUS_Platform_P_UnmapMemory */
void *
NEXUS_Platform_P_MapMemory(NEXUS_Addr offset, size_t length, NEXUS_AddrType type)
{
    void *addr = NULL;
    bool cached = type==NEXUS_AddrType_eCached;

#if ARM_LINUX_APIS
    /* TODO: use #if NEXUS_USE_CMA; provide non-cma mmap */
    addr = NEXUS_Platform_P_CmaVmap(offset, length, type==NEXUS_AddrType_eCached);
    if (!addr) {
        BDBG_ERR(("vmap(" BDBG_UINT64_FMT ", %#x, %s) failed", BDBG_UINT64_ARG(offset), (unsigned)length, cached?"cached":"uncached"));
    }
#elif LINUX_2631_3_2_OR_GREATER  

    if (type==NEXUS_AddrType_eCached)
        addr = ioremap_cachable(offset, length);
    else
        addr = ioremap_nocache(offset, length);
    
    if (!addr) {
        BDBG_ERR(("ioremap(" BDBG_UINT64_FMT ", %#x, %s) failed", BDBG_UINT64_ARG(offset), length, cached?"cached":"uncached"));
    }
    else {
        BDBG_MSG(("ioremap(" BDBG_UINT64_FMT ", %#x, %s) addr %#x ", BDBG_UINT64_ARG(offset), length, cached?"cached":"uncached",addr));
    }
#else
#if 1 /* TODO: remove this code if/when __ioremap provides "zone normal" fixed mapping */
#if BMIPS4380_40NM || BMIPS5000_40NM
#error
#elif BMIPS5000_65NM
    /* for the lower 256MB of MEMC0 + 32 MB of register space, we use KSEG0/1 */
    if (offset < 0x12000000) {
        /* fixed map */
        if (cached) {
            addr = (void *)(offset | 0x80000000); /* KSEG0 */
        }
        else {
            addr = (void *)(offset | 0xA0000000); /* KSEG1 */
        }
    }
    /* for the upper 256MB of MEMC0 */
    else if (offset >= 0x20000000 && offset < 0x30000000) {
        /* fixed map */
        if (cached) {
            addr = (void *)((offset-0x20000000) | 0xc0000000);
        }
        else {
            addr = (void *)((offset-0x20000000) | 0xd0000000);
        }
    }
    /* else, fall into ioremap() case */
#else
    /* for the lower 256MB of MEMC0 + 32 MB of register space, we use KSEG0/1 */
    if (offset < 0x12000000) {
        /* fixed map */
        if (cached) {
            addr = (void *)(unsigned long)(offset | 0x80000000); /* KSEG0 */
        } else {
            addr = (void *)(unsigned long)(offset | 0xA0000000); /* KSEG1 */
        }
    }
    /* else, fall into ioremap() case */
#endif
    else
#endif
    {

        addr = __ioremap(offset, length, type==NEXUS_AddrType_eCached?_CACHE_CACHABLE_NONCOHERENT:_CACHE_UNCACHED);

        if (!addr) {
            BDBG_ERR(("ioremap(" BDBG_UINT64_FMT ", %#x, %s) failed", BDBG_UINT64_ARG(offset), length, cached?"cached":"uncached"));
        }
    }
#endif
    BDBG_MSG(("mmap  offset:" BDBG_UINT64_FMT " size:%u -> %p", BDBG_UINT64_ARG(offset), (unsigned)length, addr));
    return addr;
}

void
NEXUS_Platform_P_UnmapMemory(void *pMem, size_t length, NEXUS_AddrType memoryMapType)
{
    unsigned long addr = (unsigned long)pMem;

    BSTD_UNUSED(memoryMapType);
    BSTD_UNUSED(length);
    BDBG_MSG(("unmap: addr:%p size:%u", pMem, (unsigned)length));

#if NEXUS_USE_CMA
    BSTD_UNUSED(addr);
    NEXUS_Platform_P_CmaVumap(pMem);
#elif LINUX_2631_3_2_OR_GREATER 
     iounmap(pMem);
#else
#if BMIPS4380_40NM || BMIPS5000_40NM
#error
#elif BMIPS5000_65NM
    if ((addr >= 0x80000000 && addr < 0x92000000) ||
        (addr >= 0xa0000000 && addr < 0xb2000000) ||
        (addr >= 0xc0000000 && addr < 0xe0000000))
    {
        /* fixed map has no unmap */
    }
#else
    if ((addr >= 0x80000000 && addr < 0x92000000) ||
        (addr >= 0xa0000000 && addr < 0xb2000000))
    {
        /* fixed map has no unmap */
    }
#endif
    else {
        iounmap(pMem);
    }
#endif
    return;
}


void *
NEXUS_Platform_P_MapRegisterMemory(unsigned long offset, unsigned long length)
{
#if ARM_LINUX_APIS
    void *addr = NULL;
    addr =  ioremap_nocache(offset, length);
    if (!addr) {
        BDBG_ERR(("map registers ioremap(%#lx, %#lx) failed", offset, length));
    }
    else {
        BDBG_MSG(("map registers ioremap(%#lx, %#lx) addr %p ", offset, length, addr));
    }
    return addr;
#else
    return NEXUS_Platform_P_MapMemory(offset, length, NEXUS_AddrType_eUncached);
#endif
}


void
NEXUS_Platform_P_UnmapRegisterMemory(void *pMem, unsigned long length)
{
#if ARM_LINUX_APIS
    iounmap(pMem);
#else
    return NEXUS_Platform_P_UnmapMemory(pMem, length, NEXUS_AddrType_eUncached);
#endif
}


#if !NEXUS_USE_CMA
NEXUS_Error NEXUS_Platform_P_GetHostMemory(NEXUS_PlatformMemory *pMemory)
{
    unsigned i;
    for (i=0;i<NEXUS_MAX_HEAPS && i<sizeof(nexus_driver_state.settings.region)/sizeof(nexus_driver_state.settings.region[0]);i++) {
        pMemory->osRegion[i].base = nexus_driver_state.settings.region[i].offset;
        pMemory->osRegion[i].length = nexus_driver_state.settings.region[i].size;
    }
    return 0;
}
#endif

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
    unsigned long flags;

    BSTD_UNUSED(data);

    if(!state->started) {
        goto done;
    }

    /* Loop until all is cleared */
    for(;;) {
        uint32_t re_enable[NEXUS_NUM_L1_REGISTERS];
        uint32_t status;
        unsigned bit;
        unsigned i;

        /* Mask interrupts only to read current status */
        spin_lock_irqsave(&state->lock, flags);

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
        spin_unlock_irqrestore(&state->lock, flags);

        if(status==0) {
            goto done;
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
                    if (++state->table[irq].count % 50000 == 0) {
                        if (!state->table[irq].print && !nexus_driver_state.uninit_pending) {
                            printk("<0>### %s (W%d, bit %d) fired %d times\n", state->table[irq].name, irq/32, irq%32, state->table[irq].count);
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
                    if (!state->table[i+bit].special_handler && !state->table[i+bit].virtual)
                    {
                        if (state->table[i+bit].enabled && !state->table[i+bit].taskletEnabled) {
                            state->table[i+bit].taskletEnabled = true;
                            enable_irq(LINUX_IRQ(i+bit));
                        }
                    }
                }
            }
        }

        /* Now, restore any disabled virtual and shared gpio interrupts (masking not required) */
        spin_lock_irqsave(&state->lock, flags);
        if (NEXUS_Platform_P_VirtualIrqSupported())
        {
            b_virtual_irq_reenable_irqs_spinlocked();
        }
        if (NEXUS_Platform_P_SharedGpioSupported())
        {
            b_shared_gpio_reenable_irqs_spinlocked();
        }
        spin_unlock_irqrestore(&state->lock, flags);
    }

done:
    return;
}

static void NEXUS_Platform_P_SetL1Status(unsigned l1)
{
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    unsigned i;
    for(i=0;i<NEXUS_NUM_L1_REGISTERS;i++,l1-=32) {
        if(l1<32) {
            state->pending[i].IntrStatus |= 1<<l1;
            break;
        }
    }
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
static irqreturn_t __attribute__((no_instrument_function))
NEXUS_Platform_P_LinuxIsr(int linux_irq, void *dev_id)
#else
static irqreturn_t __attribute__((no_instrument_function))
NEXUS_Platform_P_LinuxIsr(int linux_irq, void *dev_id, struct pt_regs *regs)
#endif
{
    struct b_bare_interrupt_entry *entry = dev_id;
    unsigned irq = NEXUS_IRQ(linux_irq);
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    unsigned i;
    unsigned long flags;

    if (irq >= NUM_IRQS) {
        goto error;
    }

    /* Make sure we're serialized with the tasklet across multiple CPUs */
    spin_lock_irqsave(&state->lock, flags);

    /* disable irq */
    BDBG_MSG_IRQ(("TH disable[irq] %d", linux_irq));
    if ( !entry->special_handler ) {
        if ( entry->taskletEnabled ) { 
            entry->taskletEnabled = false;
            disable_irq_nosync(linux_irq);
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
    tasklet_hi_schedule(&NEXUS_Platform_P_IsrTasklet);

    spin_unlock_irqrestore(&state->lock, flags);

    return IRQ_HANDLED;
    
error:
    BDBG_ASSERT(0); /* fail hard on unknown irq */
#if 0
    BDBG_WRN(("unknown irq %d", linux_irq));
    disable_irq_nosync(linux_irq);
#endif
    return IRQ_HANDLED;
}

NEXUS_Error NEXUS_Platform_P_ConnectInterrupt(unsigned irqNum, 
    NEXUS_Core_InterruptFunction handler_isr, void *context_ptr, int context_int)
{
    struct NEXUS_Platform_P_IrqMap *nexusMap = NEXUS_Platform_P_IrqMap;
    const char *name=NULL;
    bool shared;
    b_bare_os_special_interrupt_handler special_handler;
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    struct b_bare_interrupt_entry *entry;
    unsigned long flags;
    unsigned i;

    if (irqNum>=NUM_IRQS || handler_isr==NULL || !state->started) {
        return BERR_TRACE(-1);
    }
    
    /* search the nexus map first */    
    for (i=0;nexusMap[i].name;i++) {
        if (nexusMap[i].linux_irq == irqNum+1) {
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
            if (BINT_MAP_GET_L1SHIFT(&intMap[i]) == irqNum) {
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
    
    spin_lock_irqsave(&state->lock, flags);
    entry->name = name;
    entry->handler = handler_isr;
    entry->context_ptr = context_ptr;
    entry->context_int = context_int;
    entry->special_handler = special_handler;
    entry->shared = shared;\
    entry->virtual = NEXUS_Platform_P_IsVirtualIrq(name);
    BDBG_ASSERT(!entry->enabled);
    BDBG_ASSERT(!entry->taskletEnabled);
    BDBG_ASSERT(!entry->requested);
    /* request_irq deferred to first enable. */
    spin_unlock_irqrestore(&state->lock, flags);
    
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

static void NEXUS_Platform_P_SetmaskL1(unsigned l1, bool disable)
{
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    unsigned reg = l1/32;
    if (disable)
    {
        state->processing[reg].IntrMaskStatus |= 1 << (l1%32);
    }
    else
    {
        state->processing[reg].IntrMaskStatus &= ~(1 << (l1%32));
    }
}
    
NEXUS_Error NEXUS_Platform_P_EnableInterrupt_isr( unsigned irqNum)
{
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    struct b_bare_interrupt_entry *entry;
    unsigned reg = irqNum/32;
    unsigned long flags;

    if (irqNum>=NUM_IRQS || !state->started) {
        return BERR_TRACE(-1);
    }
    entry = &state->table[irqNum];
    if (!entry->handler) {
        return BERR_TRACE(-1);
    }
    
    spin_lock_irqsave(&state->lock, flags);
    state->processing[reg].IntrMaskStatus &= ~(1 << (irqNum%32));

    if (entry->virtual)
    {
        BDBG_MSG(("connect virtual interrupt %s (%d, %p)", entry->name, entry->shared, entry->special_handler));
        entry->enabled = true;
        entry->taskletEnabled = false; /* leave tasklet disabled, because all handling done in another layer */
        entry->requested = false; /* leave requested false, because all handling done in another layer */
        spin_unlock_irqrestore(&state->lock, flags);
        return 0;
    }

    if (!entry->requested) {
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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0)
        irqflags |= IRQF_TRIGGER_HIGH;
#endif

        spin_unlock_irqrestore(&state->lock, flags);
        BDBG_MSG(("connect interrupt %s %d (%d, %p)", entry->name, LINUX_IRQ(irqNum), entry->shared, entry->special_handler));
        entry->taskletEnabled = true; /* set before call request_irq as irq can immediately fire */
        if (request_irq(LINUX_IRQ(irqNum), NEXUS_Platform_P_LinuxIsr, irqflags, entry->name, entry)) {
            /* disable */
            spin_lock_irqsave(&state->lock, flags);
            entry->handler = NULL;
            state->processing[reg].IntrMaskStatus |= (1 << (irqNum%32));
            spin_unlock_irqrestore(&state->lock, flags);
            return BERR_TRACE(-1);
        }
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
                enable_irq(LINUX_IRQ(irqNum));
            }
        }
        entry->enabled = true;
    }
    spin_unlock_irqrestore(&state->lock, flags);
    return 0;
}

void NEXUS_Platform_P_DisableInterrupt_isr( unsigned irqNum)
{
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    struct b_bare_interrupt_entry *entry;
    unsigned reg = irqNum/32;
    unsigned long flags;

    if (irqNum>=NUM_IRQS) {
        return ;
    }
    entry = &state->table[irqNum];
    if (!entry->handler) {
        BERR_TRACE(-1);
        return;
    }
    
    if (entry->virtual)
    {
        BDBG_MSG(("disable virtual interrupt %d", LINUX_IRQ(irqNum)));
        spin_lock_irqsave(&state->lock, flags);
        state->processing[reg].IntrMaskStatus |= (1 << (irqNum%32));
        entry->enabled = false;
        spin_unlock_irqrestore(&state->lock, flags);
        return;
    }

    if (entry->enabled) {
        BDBG_ASSERT(entry->requested);
        BDBG_MSG(("disable interrupt %d", LINUX_IRQ(irqNum)));
        spin_lock_irqsave(&state->lock, flags);
        state->processing[reg].IntrMaskStatus |= (1 << (irqNum%32));
        if (!entry->special_handler) {
            /* If the TH has received the interrupt but it has not been processed by the tasklet, don't nest the disable call. */
            if ( entry->taskletEnabled ) 
            {
                disable_irq_nosync(LINUX_IRQ(irqNum));
                entry->taskletEnabled = false;
            }
        }
        entry->enabled = false;
        spin_unlock_irqrestore(&state->lock, flags);
    }
}

void NEXUS_Platform_P_DisconnectInterrupt( unsigned irqNum)
{
    int rc;
    struct b_bare_interrupt_state *state = &b_bare_interrupt_state;
    struct b_bare_interrupt_entry *entry;
    unsigned reg = irqNum/32;
    unsigned long flags;

    if(irqNum>=NUM_IRQS) {
        rc = BERR_TRACE(-1);
        return;
    }
    entry = &state->table[irqNum];
    if (!entry->handler) {
        BERR_TRACE(-1);
        return;
    }
    
    spin_lock_irqsave(&state->lock, flags);
    BDBG_MSG(("disconnect interrupt %d", LINUX_IRQ(irqNum)));
    entry->handler = NULL;
    if (entry->enabled) {
        state->processing[reg].IntrMaskStatus |= (1 << (irqNum%32));
        entry->enabled = false;
    }
    if (entry->requested) {
        entry->requested = false;
        entry->taskletEnabled = false;
        spin_unlock_irqrestore(&state->lock, flags);
        /* kernel can sleep in free_irq, so must release the spinlock first */
        free_irq(LINUX_IRQ(irqNum), entry);
    }
    else {
        spin_unlock_irqrestore(&state->lock, flags);
    }
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
    unsigned long flags;

#if defined(CONFIG_BRCMSTB_NEXUS_API) && BRCMSTB_H_VERSION >= 5
    int retValue;
    BSTD_UNUSED(flags);

    if (systemRegister)
    {
#if defined(CONFIG_ARM) && (BRCMSTB_H_VERSION == 8) && (BCHP_PHYSICAL_OFFSET == 0xd0000000)
        /* This is a workaround for an issue inside of the 4.1-1.3 kernel release which fails
         * to handle address mappings for chips with BCHP_PHYSICAL_OFFSET of 0xD0000000. */
        reg &= ~0x20000000;
#endif
        retValue = brcmstb_update32(reg, mask, value);
        if (retValue)
        {
            printk("brcmstb_update32(%#x,%#x,%#x) failed with %d\n",
                reg, mask, value, retValue);
        }
    }
    else
#endif
    {
        /* this spinlock synchronizes with any kernel use of a set of shared registers. */
        spin_lock_irqsave(&brcm_magnum_spinlock, flags);
        preInitState->privateState.regSettings.systemUpdate32_isrsafe(preInitState->hReg, reg, mask, value, false);
        spin_unlock_irqrestore(&brcm_magnum_spinlock, flags);
    }
    return;
}


NEXUS_Error
NEXUS_Platform_P_Magnum_Init(void)
{
    /* nothing to do here, magnum already initialized when driver loaded */
    return BERR_SUCCESS;
}

void
NEXUS_Platform_P_Magnum_Uninit(void)
{
    return;
}

unsigned long
copy_to_user_small(void * to, const void * from, unsigned long n)
{
    return copy_to_user(to, from, n);
}

unsigned long
copy_from_user_small(void * to, const void * from, unsigned long n)
{
    return copy_from_user(to, from, n);
}

void NEXUS_Platform_P_TerminateProcess(unsigned id)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
   struct task_struct *tsk;
   struct task_struct *found = NULL;
   rcu_read_lock();
   for_each_process(tsk) {
      if (tsk->flags & PF_KTHREAD) {
         continue;
      }
      if (tsk->pid == id) {
         found = tsk;
      }
   }
   if (found != NULL) {
       send_sig(SIGKILL, found, 0);
   }
   rcu_read_unlock();
#else
   int rc;
   char pidstr[16];
#  ifdef B_REFSW_ANDROID
   char *argv[] = {"/system/bin/kill", "-9", pidstr, NULL};
   snprintf(pidstr, 16, "%u", id);
   rc = call_usermodehelper("/system/bin/kill", argv, NULL, UMH_WAIT_PROC);
   if (rc) BERR_TRACE(rc);
#  else
   char *argv[] = {"/bin/kill", "-9", pidstr, NULL};
   snprintf(pidstr, 16, "%u", id);
   rc = call_usermodehelper("/bin/kill", argv, NULL, UMH_WAIT_PROC);
   if (rc) BERR_TRACE(rc);
#  endif
#endif
}


void NEXUS_Platform_P_StopCallbacks(void *interfaceHandle)
{
    if((uint8_t *)interfaceHandle >= (uint8_t *)NEXUS_BASEOBJECT_MIN_ID) {
        NEXUS_Error rc = b_objdb_verify_any_object(interfaceHandle);
        if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto done;}

        NEXUS_Base_P_StopCallbacks(interfaceHandle);
        NEXUS_P_Proxy_StopCallbacks(interfaceHandle);
    }
done:
    return;
}

void NEXUS_Platform_P_StartCallbacks(void *interfaceHandle)
{
    if((uint8_t *)interfaceHandle >= (uint8_t *)NEXUS_BASEOBJECT_MIN_ID) {
        NEXUS_Error rc = b_objdb_verify_any_object(interfaceHandle);
        if(rc!=NEXUS_SUCCESS) {
            /* XXX NEXUS_StartCallbacks could be used with bad handle, in particularly XXX_Close, is paired with auto generated Stop>Start callbacks, where Start called _after_ obhect was already destroyed */
            goto done;
        }

        NEXUS_Base_P_StartCallbacks(interfaceHandle);
        NEXUS_P_Proxy_StartCallbacks(interfaceHandle);
    }
done:
    return;
}

#if NEXUS_TRACK_STOP_CALLBACKS
void NEXUS_Platform_P_StopCallbacks_tagged(void *interfaceHandle, const char *pFileName, unsigned lineNumber, const char *pFunctionName)
{
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
    BSTD_UNUSED(pFunctionName);
    NEXUS_Platform_P_StopCallbacks(interfaceHandle);
}
void NEXUS_Platform_P_StartCallbacks_tagged(void *interfaceHandle, const char *pFileName, unsigned lineNumber, const char *pFunctionName)
{
    BSTD_UNUSED(pFileName);
    BSTD_UNUSED(lineNumber);
    BSTD_UNUSED(pFunctionName);
    NEXUS_Platform_P_StartCallbacks(interfaceHandle);
}
#endif

NEXUS_Error NEXUS_Platform_P_DropPrivilege(const NEXUS_PlatformSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    /* drop is performed in linuxuser.proxy */
    return 0;
}

static unsigned NEXUS_Platform_P_ReadDeviceTreeStr(const char *path, const char *prop)
{
    unsigned id = 0;
#if USE_LIBFDT
    int offset = fdt_path_offset(initial_boot_params, path);
    const char *val = fdt_getprop(initial_boot_params, offset, prop, NULL);
    if (val) {
       id  = NEXUS_atoi(val); /*Assuming 32bit. Need to check for length */
    }
#else
#if NEXUS_CPU_ARM
    BDBG_ERR(("Libfdt is not supported for current kernel version"));
#endif
#endif
    return id;
}

static unsigned NEXUS_Platform_P_ReadDeviceTreeInt(const char *path, const char *prop)
{
    unsigned id = 0;
#if USE_LIBFDT
    int offset = fdt_path_offset(initial_boot_params, path);
    const uint32_t *val = fdt_getprop(initial_boot_params, offset, prop, NULL);
    if (val) {
       id  = fdt32_to_cpu(*val); /*Assuming 32bit. Need to check for length */
    }
#else
#if NEXUS_CPU_ARM
    BDBG_ERR(("Libfdt is not supported for current kernel version"));
#endif
#endif
    return id;
}

#if !NEXUS_PLATFORM_P_READ_BOX_MODE
unsigned NEXUS_Platform_P_ReadBoxMode(void)
{
    unsigned boxMode = 0;
    const char *override;
    override = NEXUS_GetEnv("B_REFSW_BOXMODE");
    if (override) {
        boxMode = NEXUS_atoi(override);
    }
    if (!boxMode) {
        boxMode = NEXUS_Platform_P_ReadDeviceTreeStr("/bolt", "box");
    }
    return boxMode;
}
#endif

unsigned NEXUS_Platform_P_ReadBoardId(void)
{
    return NEXUS_Platform_P_ReadDeviceTreeInt("/bolt", "board-id");
}

unsigned NEXUS_Platform_P_ReadPMapId(void)
{
    return NEXUS_Platform_P_ReadDeviceTreeInt("/bolt", "pmap");
}

static unsigned NEXUS_Platform_P_ParseDeviceTreeCompatible(const char *compatible, BCHP_PmapSettings *pMapSettings)
{
    unsigned cnt = 0;
#if USE_LIBFDT
    int offset;

    for (offset = fdt_node_offset_by_compatible(initial_boot_params, -1, compatible);
        offset >= 0;
        offset = fdt_node_offset_by_compatible(initial_boot_params, offset, compatible))
    {
        if (pMapSettings) {
            int prop_offset;
            for (prop_offset = fdt_first_property_offset(initial_boot_params, offset);
                prop_offset >= 0;
                prop_offset = fdt_next_property_offset(initial_boot_params, prop_offset))
            {
                const char *pname;
                int *val;
                unsigned int len;

                val = (int*)fdt_getprop_by_offset(initial_boot_params, prop_offset, &pname, &len);
                if (val != NULL) {
                    unsigned value = fdt32_to_cpu(*val);
                    if (!strcmp(pname, "brcm,value")) {
                        pMapSettings[cnt].value = value;
                    }
                    else if (!strcmp(pname, "bit-shift")) {
                        pMapSettings[cnt].shift = value;
                    }
                    else if (!strcmp(pname, "bit-mask")) {
                        pMapSettings[cnt].mask = value;
                    }
                    else if (!strcmp(pname, "reg")) {
                        pMapSettings[cnt].reg = value;
                    }
                }
            }
        }
        cnt++;
    }
#endif

    return cnt;
}

BCHP_PmapSettings * NEXUS_Platform_P_ReadPMapSettings(void)
{
    char *compat[] = {"brcm,pmap-divider", "brcm,pmap-multiplier", "brcm,pmap-mux"};
    BCHP_PmapSettings *pMapSettings = NULL;
    unsigned cnt = 0, size, i;

    for (i=0; i<sizeof(compat)/sizeof(compat[0]); i++) {
        cnt += NEXUS_Platform_P_ParseDeviceTreeCompatible(compat[i], NULL);
    }
    BDBG_MSG(("Found %d compatible nodes", cnt));
    if (!cnt) return NULL;

    size = (cnt+1)*sizeof(BCHP_PmapSettings); /* One extra to indicate end data */
    pMapSettings = BKNI_Malloc(size);
    if(!pMapSettings) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); return NULL; }

    BKNI_Memset(pMapSettings, 0, size);
    cnt = 0;
    for (i=0; i<sizeof(compat)/sizeof(compat[0]); i++) {
        cnt += NEXUS_Platform_P_ParseDeviceTreeCompatible(compat[i], &pMapSettings[cnt]);
    }

    return pMapSettings;
}

void NEXUS_Platform_P_FreePMapSettings(BCHP_PmapSettings *pMapSettings)
{
    BKNI_Free(pMapSettings);
}

NEXUS_Error NEXUS_Platform_GetStandbyStatus(NEXUS_PlatformStandbyStatus *pStatus)
{
    return NEXUS_Platform_GetStandbyStatus_driver(pStatus);
}

NEXUS_Error NEXUS_Platform_SetStandbySettings( const NEXUS_StandbySettings *pSettings )
{
    return NEXUS_Platform_SetStandbySettings_driver(pSettings);
}

NEXUS_Error NEXUS_Platform_P_SetStandbyExclusionRegion(unsigned heapIndex)
{
#if defined(BRCMSTB_HAS_PM_MEM_EXCLUDE)
    NEXUS_MemoryStatus status;
    int rc;

    if (!g_pCoreHandles->heap[heapIndex].nexus) {
        return NEXUS_SUCCESS;
    }
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
#if defined(NEXUS_GISB_ARB)
    is_available = false;
#else
    is_available = true;
#endif
#endif
    return is_available;
}

#include "b_virtual_irq.h"

#if NEXUS_HAS_GPIO
#include "b_shared_gpio.h"

static void NEXUS_Platform_P_FireGpioL2(bool aon)
{
    if (NEXUS_Platform_P_VirtualIrqSupported())
    {
        b_virtual_irq_software_l2_isr(aon ? b_virtual_irq_line_gio_aon : b_virtual_irq_line_gio);
    }
    else
    {
        /* TODO */
    }
}

static void NEXUS_Platform_P_GetSharedGpioSubmoduleInitSettings(b_shared_gpio_module_init_settings * gio_settings)
{
    if (NEXUS_Platform_P_VirtualIrqSupported())
    {
        gio_settings->gio_linux_irq = b_virtual_irq_get_linux_irq(b_virtual_irq_line_gio);
        gio_settings->gio_aon_linux_irq = b_virtual_irq_get_linux_irq(b_virtual_irq_line_gio_aon);
    }
    else
    {
        gio_settings->gio_linux_irq = 0;
        gio_settings->gio_aon_linux_irq = 0;
    }
}

#define NEXUS_PLATFORM_P_SHARED_GPIO_SUPPORTED() (NEXUS_Platform_P_SharedGpioSupported())
#else
#define NEXUS_PLATFORM_P_SHARED_GPIO_SUPPORTED() (false)
#endif
#include "nexus_platform_virtual_irq.inc"
#define B_VIRTUAL_IRQ_SPIN_LOCK(flags) spin_lock_irqsave(&b_bare_interrupt_state.lock, flags)
#define B_VIRTUAL_IRQ_SPIN_UNLOCK(flags) spin_unlock_irqrestore(&b_bare_interrupt_state.lock, flags)
#define B_VIRTUAL_IRQ_GET_L1_WORD_COUNT() (NEXUS_NUM_L1_REGISTERS)
#define B_VIRTUAL_IRQ_MASK_L1(L1) NEXUS_Platform_P_SetmaskL1(L1, true)
#define B_VIRTUAL_IRQ_UNMASK_L1(L1) NEXUS_Platform_P_SetmaskL1(L1, false)
#define B_VIRTUAL_IRQ_SET_L1_STATUS(L1) NEXUS_Platform_P_SetL1Status(L1)
#define B_VIRTUAL_IRQ_INC_L1(L1)
#define B_VIRTUAL_IRQ_WAKE_L1_LISTENERS() tasklet_hi_schedule(&NEXUS_Platform_P_IsrTasklet)
#include "b_virtual_irq.inc"

#if NEXUS_HAS_GPIO
#include "nexus_platform_shared_gpio.inc"
#define B_SHARED_GPIO_FIRE_GPIO_L2(AON) NEXUS_Platform_P_FireGpioL2(AON)
#define B_SHARED_GPIO_SPIN_LOCK(flags) spin_lock_irqsave(&b_bare_interrupt_state.lock, flags)
#define B_SHARED_GPIO_SPIN_UNLOCK(flags) spin_unlock_irqrestore(&b_bare_interrupt_state.lock, flags)
#include "b_shared_gpio.inc"
#endif

#define B_OS_IRQ_SPIN_LOCK(flags) spin_lock_irqsave(&b_bare_interrupt_state.lock, flags)
#define B_OS_IRQ_SPIN_UNLOCK(flags) spin_unlock_irqrestore(&b_bare_interrupt_state.lock, flags)
#include "b_os_irq.inc"

bool NEXUS_Platform_P_IsOs64(void)
{
#if NEXUS_CPU_ARM64
    return true;
#else
    return false;
#endif
}

bool NEXUS_Platform_P_LazyUnmap(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
    return false;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
#if LINUX_EXTRAVERSION_CODE >= LINUX_EXTRAVERSION(1,2)
    return false;
#else
    return true;
#endif
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
#if LINUX_EXTRAVERSION_CODE >= LINUX_EXTRAVERSION(1,15)
    return false;
#else
    return true;
#endif
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0)
#if LINUX_EXTRAVERSION_CODE >= LINUX_EXTRAVERSION(1,20)
    return false;
#else
    return true;
#endif
#else
    return true;
#endif
}
