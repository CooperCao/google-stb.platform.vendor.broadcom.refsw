/******************************************************************************
 * Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
#elif ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30) ) && ( LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37) )
#include <linux/autoconf.h>
#else
#include <linux/config.h>
#endif
#include <linux/module.h>
#include <linux/utsname.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include "bcm_driver.h"
#include "bcmdriver_common.h"
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0)
#include <linux/seq_file.h>
#include <linux/uidgid.h>
#endif
#include "b_memory_regions.h"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,1)
#include <linux/brcmstb/brcmstb.h>
#endif
#include "b_virtual_irq.h"
#include "b_virtual_irq_usermode.h"
#include "b_shared_gpio.h"
#include "b_shared_gpio_usermode.h"

/*
* Macros to help debugging
*/
#undef PDEBUG
#undef PWARN
#undef PERR
#undef PINFO

#define DEBUG
#ifdef DEBUG
    #define PERR(fmt, args...) printk( KERN_ERR "BCMDRV: " fmt, ## args)
    #define PWARN(fmt, args...) printk( KERN_WARNING "BCMDRV: " fmt, ## args)
    #define PINFO(fmt, args...) printk( KERN_INFO "BCMDRV: " fmt, ## args)
    #define PDEBUG(fmt, args...) printk( KERN_DEBUG "BCMDRV: " fmt, ## args)
#else
    #define PERR(fmt, args...)
    #define PWARN(fmt, args...)
    #define PINFO(fmt, args...)
    #define PDEBUG(fmt, args...)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
#include "linux/brcmstb/brcmstb.h"
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#include "asm/brcmstb/brcmstb.h"
#endif

#define BDBG_LOG(x)
#define BDBG_FILE_MODULE(x) extern int __unused
#define BDBG_MODULE_MSG(x,y) PDEBUG y
#define BDBG_ASSERT(x)  ((x) ? (void)0 : (void)0)
#define BDBG_CASSERT(expr) do switch(0){case 0: case (expr):;} while(0)
#define BKNI_Memcmp     memcmp
#define BKNI_Free(x) kfree(x)
#define BKNI_Malloc(x) kmalloc(x, GFP_KERNEL)

static struct {
    unsigned bchp_physical_offset;
} g_breg;

static uint32_t breg_read(uint32_t addr)
{
    uint32_t value;
    volatile uint32_t *reg;
    reg = ioremap_nocache(addr+g_breg.bchp_physical_offset, 4);
    if (!reg) {
        value = 0;
        PERR("breg_read %x failure\n", addr);
    }
    else {
        value = *reg;
        iounmap(reg);
    }
    return value;
}
static void breg_write(uint32_t addr, uint32_t value)
{
    volatile uint32_t *reg;
    reg = ioremap_nocache(addr+g_breg.bchp_physical_offset, 4);
    if (reg) {
        *reg = value;
        iounmap(reg);
    }
    else {
        PERR("breg_write %x failure\n", addr);
    }
}

#define BERR_TRACE(rc) do {if (rc) PERR("error %d at line %d\n", rc, __LINE__);}while(0)
#define BDBG_CWARNING(expr) do {if(0){int unused = 1/(expr);unused++;}} while(0)

/* Kernel newer than 3.8.1: utilize an irq_start_index of 32 (works for
 * ARM, ARM64)
 * Kernel newer than 3.14 on MIPS: utilize an irq_start_index of 32
 * (works for MIPS on 4.1 kernel w/ Device Tree)
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,1) && !defined(CONFIG_MIPS) || \
    (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0) && defined(CONFIG_MIPS))
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#include "bcmdriver_arm.c"
#include "linux/brcmstb/cma_driver.h"
/* temp stub */
#define dma_cache_wback_inv(ADDR,SIZE)  printk( KERN_ERR "BCMDRV: need to port dma_cache_wback_inv for arm linux\n\n")
#endif
static int g_irq_start_index = 32;
#define make_linux_irq(i)   ((i)+g_irq_start_index-1)
#define unmake_linux_irq(i) ((i)-g_irq_start_index+1)
#else
#define make_linux_irq(i) (i)
#define unmake_linux_irq(i) (i)
#endif

#ifndef BRCMSTB_H_VERSION
#define BRCMSTB_H_VERSION   0
#endif

#if BRCMSTB_H_VERSION >= 5
#ifdef NEXUS_VERSION_MAJOR
#include "linux/brcmstb/nexus_version_check.h"
#endif
#endif


#define PINFO_TRACE(fmt, args...) /* PINFO(fmt, ## args) */

/*
  Since smart card is part of the UPG and to support smart card beast
  service, we has to create high status and mask here. The high mask
  and high status is only used by software IRQ that created by Linux
  kernel. When we report this high status back to application, we have
  to map it to one of the low status. For example, when we receive virtual
  IRQ 40 and 41 for SCA and SCB, we need to map it to UPG IRQ 10.
  This design is also based on the assumption that we will not receive IRQ
  10 in this user mode driver. We will receive IRQ 40 and 41 instead and
  then map it to 10.
*/
typedef struct
{
    unsigned long   reportedStatus[BCM_INTC_MAX_SIZE]; /* Interrupts that have been reported by driver to calling code */
    unsigned long   globalmask[BCM_INTC_MAX_SIZE];     /* HW interrupts that the driver controls */
    unsigned long   status[BCM_INTC_MAX_SIZE];         /* HW interrupts we've received. This is equivalent to reading L1 status bits, but via kernel. */
    unsigned long   reportedInvalidWait[BCM_INTC_MAX_SIZE];
}intmgr_t;


#define MODULE_NAME "bcmdriver"
#define INFINITE 0xFFFFFFFF
#define BRCM_MAJOR 30 /* Major device number */

static int  brcm_stb_init(void);
static int  brcm_open(struct inode *inode, struct file *file);
static int  brcm_close(struct inode * inode, struct file * file);
static int  brcm_mmap(struct file *file, struct vm_area_struct *vma);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
static long  brcm_ioctl(struct file * file,unsigned int cmd, unsigned long arg);
#else
static int  brcm_ioctl(struct inode *inode, struct file * file,unsigned int cmd, unsigned long arg);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
static irqreturn_t brcm_interrupt(int irq, void *dev_id);
#else
static irqreturn_t brcm_interrupt(int irq, void *dev_id, struct pt_regs *regs);
#endif
static void brcm_reenable_irqs(intmgr_t *p_intmgr, const uint32_t *Mask);

static unsigned vmalloc_size(unsigned required_vmalloc, uint64_t highest_installed_dram, unsigned lowest_bmem_addr);

int gBcmDebuglevel=0;
int gBcmDebug=0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
static spinlock_t gSpinLock = __SPIN_LOCK_UNLOCKED(gSpinLock);
#else
static spinlock_t gSpinLock = SPIN_LOCK_UNLOCKED;
#endif
static wait_queue_head_t g_WaitQueue;
static intmgr_t g_intmgr;
static struct proc_dir_entry *umdrv_dir, *version_file, *interrupt_file, *latency_file, *meminfo_file, *debug_file;
char debug_info[256];

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
	static spinlock_t g_magnum_spinlock = __SPIN_LOCK_UNLOCKED(g_magnum_spinlock);
	#define brcm_magnum_spinlock g_magnum_spinlock
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
    #include <linux/brcmstb/brcmapi.h>
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    #include <asm/brcmstb/brcmapi.h>
#else
#include "asm/brcmstb/common/brcmstb.h"
#ifndef BRCM_MAGNUM_SPINLOCK
static spinlock_t g_magnum_spinlock = SPIN_LOCK_UNLOCKED;
#else
extern spinlock_t g_magnum_spinlock;
#endif
#define brcm_magnum_spinlock g_magnum_spinlock
#endif

#ifdef NEXUS_WEBCPU
/* webcpu (core1) does not have brcm_magnum_spinlock */
spinlock_t brcm_magnum_spinlock;
#endif

#if 1
void print_int_status(void)
{
    if (gBcmDebug) {
        unsigned i;
        unsigned long flags;
        spin_lock_irqsave(&gSpinLock, flags);
        for(i=0;i<g_sChipConfig.IntcSize;i++) {
            PWARN("L1 W%d: mask %08lx,stat %08lx\n", i, g_intmgr.globalmask[i], g_intmgr.status[i]);
        }
        spin_unlock_irqrestore(&gSpinLock, flags);
    }
}
#else
#define print_int_status()
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,31)
/* temporarily disabled module_param(gBcmDebug,int,0000); */
#else
module_param(gBcmDebug,int,0000);
#endif
#else
MODULE_PARM(gBcmDebug,"0-1i");
#endif

static DEFINE_MUTEX(gBcmLock);

#ifdef CONFIG_COMPAT

typedef struct s_bcm_linux_mem_addr_range_32
{
    unsigned address; /* Virtual memory address */
    unsigned length;       /* Length of range (in bytes) */
} t_bcm_linux_mem_addr_range_32;

#define BRCM_IOCTL_FLUSH_DCACHE_RANGE_32       _IOW(101, 29, t_bcm_linux_mem_addr_range_32) /* use t_bcm_linux_mem_addr_range with this IOCTL */

static long  brcm_compat_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
{
    int result=0;
    switch(cmd) {
    case BRCM_IOCTL_FLUSH_DCACHE_RANGE_32:
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
        {
            t_bcm_linux_mem_addr_range_32 mem_addr_range;

            if (copy_from_user(&mem_addr_range, (void*)arg, sizeof(mem_addr_range))) {
                result = -EFAULT;
                PERR("copy_from_user failed!\n");
                break;
            }

           /* address would get validated inside brcm_cpu_dcache_flush */
           /* coverity[tainted_data] */
           result = brcm_cpu_dcache_flush((void *)(unsigned long)mem_addr_range.address, mem_addr_range.length);
        }
#else
        PERR("Not implemented for this platfrom\n");
#endif
        break;
    default:
        result = brcm_ioctl(file, cmd, arg);
    }
    return result;
}
#endif

static struct file_operations brcm_fops = {
    owner:      THIS_MODULE,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
    unlocked_ioctl:      brcm_ioctl,
#else
    ioctl:      brcm_ioctl,
#endif
#ifdef CONFIG_COMPAT
    compat_ioctl: brcm_compat_ioctl,
#endif
    open:       brcm_open,
    release:    brcm_close,
    mmap:       brcm_mmap
};

/* sub-module support */
static bool g_virtual_irq_supported;
static bool g_shared_gpio_supported;
static void brcm_init_submodules(void);
static void brcm_uninit_submodules(void);
static int brcm_open_submodules(void);
static void brcm_close_submodules(void);
static void brcm_virtualize_l1s(void);

static void brcm_reenable_irqs(intmgr_t *p_intmgr, const uint32_t *Mask)
{
    unsigned i, irq;
    unsigned long flags;
    spin_lock_irqsave(&gSpinLock, flags);
    for (i=0,irq=1;i<g_sChipConfig.IntcSize;i++) {
        unsigned bitmask;
        for (bitmask=1;bitmask;bitmask <<= 1,irq++) {
            if ((Mask[i] & bitmask) && g_sChipConfig.pIntTable[irq].disabled && !g_sChipConfig.pIntTable[irq].l2share_func) {
                if (gBcmDebug) {
                    PWARN("Re-enabling %s irq (linux %d)\n", g_sChipConfig.pIntTable[irq].name, make_linux_irq(irq) );
                }
                enable_irq(make_linux_irq(irq));
                g_sChipConfig.pIntTable[irq].disabled = 0;
            }
        }
    }
    if (g_virtual_irq_supported)
    {
        b_virtual_irq_reenable_irqs_spinlocked();
    }
    if (g_shared_gpio_supported)
    {
        b_shared_gpio_reenable_irqs_spinlocked();
    }
    spin_unlock_irqrestore(&gSpinLock, flags);
    return;
}


/**
Reenable all interrupts which we manage where status is cleared.
This can happen because when cores are reset and they reset their
status bits. */
static void brcm_reset_irqs(intmgr_t *p_intmgr)
{
    int irq;
    unsigned long flags;

    /* reset to insmod state */

    /* normal irqs are acquired by bcmdriver on init, so re-enable these */
    for (irq = 1; irq < g_sChipConfig.maxNumIrq; ++irq)
    {
        spin_lock_irqsave(&gSpinLock, flags);
        if (g_sChipConfig.pIntTable[irq].disabled) {
            if(gBcmDebug)
                PINFO("Re-enabling %s irq (linux %d)\n", g_sChipConfig.pIntTable[irq].name, make_linux_irq(irq) );
            enable_irq(make_linux_irq(irq));
            g_sChipConfig.pIntTable[irq].disabled = 0;
        }
        spin_unlock_irqrestore(&gSpinLock, flags);
    }
    if (g_virtual_irq_supported)
    {
        b_virtual_irq_reset_irqs();
    }
    if (g_shared_gpio_supported)
    {
        b_shared_gpio_reset_irqs();
    }
    return;
}

static int g_jiffies_isr = 0;   /* time when an L1 top half was received */
static int g_jiffies_user = 0;  /* time when usermode task retrieves the pending L1's */
static struct {
    unsigned cnt;
    unsigned total, min, max;
} g_latency;

#define get_jiffies() (jiffies)

/****************************************************************
* brcm_interrupt
*
* INPUTS:   irq, dev_id - see linux kernel api
* OUTPUTS:  None
* RETURNS:  None
* FUNCTION: 7xxx interrupt handler
*
****************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
static irqreturn_t brcm_interrupt(int irq, void *dev_id)
#else
static irqreturn_t brcm_interrupt(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
    unsigned index = (unmake_linux_irq(irq)-1)/32;
    unsigned bit = (unmake_linux_irq(irq)-1)%32;
    /*irq doesnt point to the interrupt table, need an index into the table */
    unsigned interrupt_table_index = unmake_linux_irq(irq);   /* (irq - g_irq_start_index +1); */

    unsigned long flags;
    l2share_func_t l2share_func = g_sChipConfig.pIntTable[interrupt_table_index].l2share_func;

    if (!g_jiffies_isr) {
        g_jiffies_isr = get_jiffies(); /* measure the time from first receipt of an L1 */
    }

    if(gBcmDebug)
        PINFO("Got %s irq (linux %d) %d\n", g_sChipConfig.pIntTable[interrupt_table_index].name, irq,
                       (int)g_sChipConfig.pIntTable[interrupt_table_index].numInter );

    spin_lock_irqsave(&gSpinLock, flags);
    g_intmgr.status[index] |= 1<<bit;
    if (l2share_func)
    {
        unsigned long triggeredInts;
        l2share_func(irq, &triggeredInts);
    }
    else
    {
        if(gBcmDebug)
            PINFO("Disabling %s irq (linux %d) %d\n", g_sChipConfig.pIntTable[interrupt_table_index].name, irq,
                           (int)g_sChipConfig.pIntTable[interrupt_table_index].numInter );
        disable_irq_nosync(irq);
        g_sChipConfig.pIntTable[interrupt_table_index].disabled = 1;
    }
    spin_unlock_irqrestore(&gSpinLock, flags);
    print_int_status();

    g_sChipConfig.pIntTable[interrupt_table_index].numInter++;

    wake_up_interruptible(&g_WaitQueue);

    return IRQ_HANDLED;
}

/****************************************************************
 * brcm_get_dev_info - Extracts information from the minor device number
****************************************************************/
void brcm_get_dev_info(struct inode *inode, unsigned int *devnum,
                       unsigned int *devmode)
{
    *devnum = MINOR(inode->i_rdev);
    *devmode = 0;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static int proc_read_interrupts(char *buf, char **start, off_t offset,int count, int *eof, void *data)
#else
static int proc_read_interrupts(struct seq_file *m, void *v)
#endif
{
    int irq, len = 0, i;

    for (i=0, irq=1;i<g_sChipConfig.IntcSize;i++) {
        unsigned bitmask;
        for (bitmask=1;bitmask;bitmask <<= 1,irq++) {
            if (g_intmgr.globalmask[i] & bitmask) {
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
                unsigned temp_len;
                temp_len = snprintf(buf+len, 0, "%s %ld\n", g_sChipConfig.pIntTable[irq].name,g_sChipConfig.pIntTable[irq].numInter);  /* returns the number of characters needed, including \n but excluding \0 */
                len += snprintf(buf+len, temp_len+1, "%s %ld\n", g_sChipConfig.pIntTable[irq].name,g_sChipConfig.pIntTable[irq].numInter);
#else
                seq_printf(m, "%s %ld\n", g_sChipConfig.pIntTable[irq].name,g_sChipConfig.pIntTable[irq].numInter);  /* returns the number of characters needed, including \n but excluding \0 */
                seq_printf(m, "%s %ld\n", g_sChipConfig.pIntTable[irq].name,g_sChipConfig.pIntTable[irq].numInter);
#endif
            }
        }
    }
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
    *eof = 1;
#endif

    return len;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static ssize_t proc_write_interrupts(struct file *file,const char *buffer,unsigned long count,void *data)
#else
static ssize_t proc_write_interrupts(struct file *file, const char __user *buffer, size_t count, loff_t *data)
#endif
{
    int irq;

    for (irq=1; irq< g_sChipConfig.maxNumIrq; irq++)
    {
        g_sChipConfig.pIntTable[irq].numInter=0;
    }

    return irq;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static int proc_read_latency(char *buf, char **start, off_t offset,int count, int *eof, void *data)
#else
static int proc_read_latency(struct seq_file *m, void *v)
#endif
{
    int len = 0;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
    len += snprintf(buf+len, 64, "interrupts: %d\n", g_latency.cnt);
    len += snprintf(buf+len, 64, "avg: %d\n", g_latency.cnt?g_latency.total/g_latency.cnt:0);
    len += snprintf(buf+len, 64, "max: %d\n", g_latency.max);
    len += snprintf(buf+len, 64, "min: %d\n", g_latency.min);
    *eof = 1;
#else
    seq_printf(m, "interrupts: %d\n", g_latency.cnt);
    seq_printf(m, "avg: %d\n", g_latency.cnt?g_latency.total/g_latency.cnt:0);
    seq_printf(m, "max: %d\n", g_latency.max);
    seq_printf(m, "min: %d\n", g_latency.min);
#endif
    return len;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static ssize_t proc_write_latency(struct file *file,const char *buffer,unsigned long count,void *data)
#else
static ssize_t proc_write_latency(struct file *file, const char __user *buffer,
				size_t count, loff_t *_pos)
#endif
{
    /* reset counts. don't parse incoming data. just consume it all. */
    memset(&g_latency, 0, sizeof(g_latency));
    return count;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static int proc_read_version(char *page, char **start,off_t off, int count,int *eof, void *data)
#else
static int proc_read_version(struct seq_file *m, void *v)
#endif
{
    int len = 0;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)

    /* Note that "snprintf" returns the number of characters that would have been printed if there was enough room.
     * This means, for example, that in some cases it might be useful to call "snprintf" twice:
     * 1.> Once with N equal to zero. "snprintf" will not write any output,
     * but it will return how many characters would have been needed to hold the output.
     * You could use this value to "malloc" sufficient room to hold the output string.
     * 2.> Once with N equal to the return value from the first call, plus one.
     * If you have allocated sufficient space for the output string, this outputs the entire output string.
     */
    len = snprintf(page, 0, "%u\n", BCMDRIVER_VERSION);
    len = snprintf(page, (len +1), "%u\n", BCMDRIVER_VERSION);
#else
    seq_printf(m, "%u\n", BCMDRIVER_VERSION);
#endif

    return len;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static int proc_read_meminfo(char *page, char **start,off_t off, int count,int *eof, void *data)
{
    unsigned long s, l;
    bmem_region_info(0, &s, &l);
    return sprintf(page, "Kernel memory %d \n", (unsigned)s);
}
#else
static int proc_read_meminfo(struct seq_file *m, void *v)
{
    /* TODO: should use bmem to learn actual regions available */
    seq_printf(m, "Kernel memory %d \n", 0x10000000);
    return 0;
}
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static int proc_read_debug(char *page, char **start,off_t off, int count,int *eof, void *data)
#else
static int proc_read_debug(struct seq_file *m, void *v)
#endif
{
    return 0;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static ssize_t proc_write_debug(struct file *file, const char *buffer, unsigned long count, void *data)
#else
static ssize_t proc_write_debug(struct file *file, const char __user *buffer, size_t count, loff_t *data)
#endif
{
    unsigned len = 0;
    if (!count) return 0;
    while (len<sizeof(debug_info) && debug_info[len]) len++;
    if ((len ? len + count + 1 : count) >= sizeof(debug_info)) {
        return -1;
    }
    if (len) {
        debug_info[len++] = ' ';
    }
    /* coverity[overrun-local: FALSE] - coverity has problem with logic above. len+count will be <= sizeof(debug_info) */
    if (copy_from_user(&debug_info[len], buffer, count)) {
        return -1;
    }
    if (debug_info[len+count-1] == '\n') {
        debug_info[len+count-1] = 0;
    }
    return count;
}

/* only one open instance of bcm_driver can manage interrupts.
other instances are allowed, but only for other functions. */
static struct file *g_interrupt_file = NULL;

static unsigned b_get_euid(void)
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
    return current->euid;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0)
    return current_euid();
#else
    kuid_t euid;
    euid =  current_euid();
    return euid.val;
#endif
}

static int brcm_open_count = 0;

/****************************************************************
* brcm_open(struct inode *inode, struct file *file)
****************************************************************/
static int brcm_open(struct inode *inode, struct file *file)
{
    int err = 0;
    unsigned int devnum, devmode;
    unsigned long flags;
    int open_count;

    /* This must be the first operation to avoid auto-unload */

    brcm_get_dev_info(inode, &devnum, &devmode);

    if (gBcmDebug)
        printk("brcm_open devnum=%d devmode=%d\n",devnum,devmode);

    if ( devnum != 0 )
    {
        return -EFAULT;
    }

    file->private_data = (void*)(unsigned long)((b_get_euid() == 0) ? 0 : -1);

    spin_lock_irqsave(&gSpinLock, flags);
    open_count = brcm_open_count++;
    spin_unlock_irqrestore(&gSpinLock, flags);
    if (!open_count)
    {
        err = brcm_open_submodules();
    }

    return err;
}

/****************************************************************
* brcm_close(struct inode *inode, struct file *file)
****************************************************************/
static int brcm_close(struct inode *inode, struct file *file)
{
    bool final_close = false;
    unsigned long flags;

    if (file == g_interrupt_file) {
        g_interrupt_file = NULL;
    }
    wake_up_interruptible(&g_WaitQueue);
    spin_lock_irqsave(&gSpinLock, flags);
    if (brcm_open_count > 0)
    {
        --brcm_open_count;
        final_close = (brcm_open_count == 0) ? true : false;
    }
    spin_unlock_irqrestore(&gSpinLock, flags);
    if (final_close)
    {
        brcm_close_submodules();
    }
    return 0;
}

struct access_node {
    t_bcm_linux_mmap_access access;
    struct access_node *next;
};

static struct {
   struct access_node *access_list;
    b_memory_region_set dynamic_regions;
} g_state;

static int brcm_mmap(struct file *file, struct vm_area_struct *vma)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    int rc;
    uint64_t offset = ((uint64_t)vma->vm_pgoff) << PAGE_SHIFT;
    unsigned size = vma->vm_end - vma->vm_start;

    if (file->private_data) {
        /* validate access for non-root clients */
        struct access_node *node;
        mutex_lock(&gBcmLock);
        for (node=g_state.access_list;node;node=node->next) {
            if (node->access.pid == current->group_leader->pid) {
                /* verify mmap is within a range */
                unsigned j;
                for (j=0;j<BCM_MAX_HEAPS;j++) {
                    if (offset >= node->access.heap[j].offset && offset + size <= node->access.heap[j].offset + node->access.heap[j].size) {
                        /* it's good */
                        PINFO_TRACE("%s[%d] pid %d access granted! :)\n", __FUNCTION__, current->pid, node->access.pid);
                        break;
                    }
                }
                if (j<BCM_MAX_HEAPS) {
                    break;
                }
                else {
                    b_memory_region region;
                    region.base = offset;
                    region.length = size;
                    if(!b_memory_region_match(&g_state.dynamic_regions, &region)) {
                        PERR("%s[%d] pid %d access denied! :(\n", __FUNCTION__, current->pid, node->access.pid);
                        node = NULL; /* pid can only be stored once, so set failure condition */
                    }
                    break;
                }
            }
        }
        mutex_unlock(&gBcmLock);
        if (!node) {
            PERR("%s[%d:%s]: non-root mmap not allowed by server!\n", __FUNCTION__, current->pid, current->comm);
            return -EACCES;
        }
    }
#if defined(CONFIG_ARM)
    vma->vm_page_prot = __pgprot_modify(vma->vm_page_prot, L_PTE_MT_MASK, L_PTE_XN); /* /dev/brcm0 should not be used to map memory which is executable */
#endif

    /* allow uncached mmap */
    if (file->f_flags & O_SYNC) {
        vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    }
#if defined(CONFIG_ARM)
    else
    {
        /*
         * ARM has an explicit setting for WC. Use default for other
         * architectures.
         */
        vma->vm_page_prot = __pgprot_modify(vma->vm_page_prot,
                                    L_PTE_MT_MASK, L_PTE_MT_BUFFERABLE | L_PTE_MT_WRITEBACK);
    }
#endif

    rc = remap_pfn_range(vma,
        vma->vm_start,
        vma->vm_pgoff,
        vma->vm_end - vma->vm_start,
        vma->vm_page_prot);
    if (rc) {
        return -EAGAIN;
    }

    return rc;
#else
    /* only tested on 2.6 */
    return -ENODEV;
#endif
}

static void brcm_init_mmap_access(void)
{
    g_state.access_list = NULL;
}

static void brcm_uninit_mmap_access(void)
{
    mutex_lock(&gBcmLock);
    while (g_state.access_list) {
        struct access_node *node = g_state.access_list;
        g_state.access_list = node->next;
        kfree(node);
    }
    mutex_unlock(&gBcmLock);
}

static int brcm_set_mmap_access(const t_bcm_linux_mmap_access *access)
{
    struct access_node *node, *prev;
    unsigned i;
    int revoke;

    if (access->pid == 0) {
        /* root user is automatically granted access. don't bother storing.
        then we can use pid == 0 as indicator of unused slot. */
        return 0;
    }

    /* determine if this is granting or revoking */
    for (i=0;i<BCM_MAX_HEAPS;i++) {
        if (access->heap[i].offset) break;
    }
    revoke = (i == BCM_MAX_HEAPS);

    mutex_lock(&gBcmLock);
    /* look for pid or open space */
    for (node=g_state.access_list,prev=NULL;node;prev=node,node=node->next) {
        if (node->access.pid == access->pid) {
            if (revoke) {
                if (prev) {
                    prev->next = node->next;
                }
                else {
                    g_state.access_list = node->next;
                }
                PINFO_TRACE("remove pid %d mmap\n", access->pid);
                kfree(node);
            }
            else {
                /* update slot */
                node->access = *access;
                PINFO_TRACE("update pid %d mmap\n", access->pid);
            }
            break;
        }
    }
    if (revoke==0 && !node) {
        node = kmalloc(sizeof(*node), GFP_KERNEL);
        if (!node) {
            mutex_unlock(&gBcmLock);
            return -1;
        }

        PINFO_TRACE("add pid %d mmap\n", access->pid);
        node->access = *access;
        node->next = g_state.access_list;
        g_state.access_list = node;
    }
    mutex_unlock(&gBcmLock);
    return 0;
}

/*************************************************************************************************************
* brcm_ioctl(struct inode *inode, struct file * file,
*               unsigned int cmd, unsigned long arg)
*************************************************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
static long brcm_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
#else
static int brcm_ioctl(struct inode *inode, struct file * file, unsigned int cmd, unsigned long arg)
#endif
{
    int result;
    int i;
    unsigned long retValue;

    result = 0;

    /* non-root users are restricted on ioctl. */
    switch(cmd)
    {
    case BRCM_IOCTL_FLUSH_DCACHE_RANGE:
        /* allowed */
        break;
    default:
        if (file->private_data) {
            PERR("ioctl %x access limited to root user\n", cmd);
            return -EACCES;
        }
        break;
    }

    switch(cmd)
    {
    case BRCM_IOCTL_WAIT_FOR_INTERRUPTS:
        {
            tbcm_linux_dd_interrupt intStruct;
            unsigned long flags;

            if (g_jiffies_user) {
                int now = get_jiffies();
                if (now >= g_jiffies_user) { /* protect against wrap. may be zero. */
                    int measured_latency = now - g_jiffies_user;
                    g_jiffies_user = 0;
                    g_latency.cnt++;
                    g_latency.total += measured_latency;
                    if (measured_latency > g_latency.max) {
                        g_latency.max = measured_latency;
                    }
                    if (measured_latency < g_latency.min) {
                        g_latency.min = measured_latency;
                    }
                }
            }

            if( copy_from_user( &intStruct, (void *)arg, sizeof(tbcm_linux_dd_interrupt) ) )
            {
                result = -EFAULT;
                PERR("copy_from_user failed!\n");
                break;
            }

            if(gBcmDebug) {
                char buf[64], *ptr = buf;
                for (i=0;i<g_sChipConfig.IntcSize;i++) {
                    ptr += snprintf(ptr, 64, " W%d=0x%08X", i, (unsigned)intStruct.interruptmask[i]);
                }
                PWARN("waitfor%s with timeOut=%d\n", buf, intStruct.timeout);
            }

            for(i=0;i<g_sChipConfig.IntcSize;i++)
            {
                unsigned long difference = (intStruct.interruptmask[i] | g_intmgr.globalmask[i]) ^ g_intmgr.globalmask[i];
                if(difference)
                {
                    if( (g_intmgr.reportedInvalidWait[i] & difference) != difference)
                    {
                        g_intmgr.reportedInvalidWait[i] |= difference;
                        PERR("Invalid wait irpt mask[%d]=0x%08X,global mask[%d]=0x%08X\n",i,(unsigned)intStruct.interruptmask[i],i,(unsigned)g_intmgr.globalmask[i]);
                    }
                    /* mask off the invalid bits */
                    intStruct.interruptmask[i] &= g_intmgr.globalmask[i];
                }

            }
            /* Re-enable any interrupts that were previously reported and handled */
            brcm_reenable_irqs(&g_intmgr, intStruct.interruptmask);

            print_int_status();

            {
                unsigned long ticks;
                wait_queue_t wq_entry;
                unsigned pending;

                result = 0;
                if (intStruct.timeout != 0 && intStruct.timeout != INFINITE) {
                    ticks = (intStruct.timeout * HZ) / 1000;
                }
                else {
                    ticks = MAX_SCHEDULE_TIMEOUT;
                }

                init_waitqueue_entry(&wq_entry, current);
                add_wait_queue(&g_WaitQueue, &wq_entry);

                b_virtual_irq_test();

                /* Be sure to go half asleep before checking condition. */
                /* Otherwise we have a race condition between when we   */
                /* check the condition and when we call schedule().     */
                set_current_state(TASK_INTERRUPTIBLE);


                spin_lock_irqsave(&gSpinLock, flags);
                pending = 0;
                for (i=0;i<g_sChipConfig.IntcSize;i++) {
                    pending = pending || (intStruct.interruptmask[i] & g_intmgr.status[i]);
                }
                spin_unlock_irqrestore(&gSpinLock, flags);
                if (!pending) {
                    if(schedule_timeout(ticks) == 0)
                        result = -EIO;
                    if (signal_pending(current))
                        result = -EIO;
                }
                current->state = TASK_RUNNING;
                remove_wait_queue(&g_WaitQueue, &wq_entry);
            }

            spin_lock_irqsave(&gSpinLock, flags);
            for (i=0;i<g_sChipConfig.IntcSize;i++) {
                intStruct.interruptstatus[i] = g_intmgr.status[i] & intStruct.interruptmask[i];
                g_intmgr.reportedStatus[i] = intStruct.interruptstatus[i];
                g_intmgr.status[i] &= ~intStruct.interruptmask[i];
            }
            spin_unlock_irqrestore(&gSpinLock, flags);

            if (g_jiffies_isr) {
                g_jiffies_user = g_jiffies_isr; /* move this value to the second stage of the pipeline */
                g_jiffies_isr = 0;
            }

            print_int_status();

            if( copy_to_user( (void *)arg, &intStruct, sizeof(tbcm_linux_dd_interrupt) ) )
            {
                result = -EFAULT;
                PERR("copy_to_user failed!\n");
                break;
            }

            if(gBcmDebug) {
                char buf[64], *ptr = buf;
                for (i=0;i<g_sChipConfig.IntcSize;i++) {
                    ptr += snprintf(ptr, 64, " W%d=0x%08X", i, (unsigned)intStruct.interruptstatus[i]);
                }
                PWARN("returning interrupt%s\n", buf);
            }
        }
        break;

    case BRCM_IOCTL_CHANGE_REQUEST:
        /* The application would like to make a change right now. It doesn't
        have to wait until the next L1 interrupt. This aborts the current
        wait and causes BRCM_IOCTL_WAIT_FOR_INTERRUPTS to return immediately. */
        print_int_status();
        wake_up_interruptible(&g_WaitQueue);
        break;

    case BRCM_IOCTL_INT_RESET:
        /* If the software resets the L2 (and thus L1) interrupts, you may
        need to reenabled all L1 interrupts so that you don't get stuck. */
        /* nexus calls BRCM_IOCTL_INT_RESET immediately after open, so it's an
        ideal spot to check for duplicate interrupt management. */
        memset(g_intmgr.reportedInvalidWait,0,sizeof(g_intmgr.reportedInvalidWait));

        if (!g_interrupt_file) {
            g_interrupt_file = file;
        }
        else if (file != g_interrupt_file) {
            PERR("can't manage interrupts from two instances\n");
            return -EFAULT;
        }
        brcm_reset_irqs(&g_intmgr);
        break;

    case BRCM_IOCTL_DEBUG_LEVEL:
        gBcmDebuglevel = (int)arg;
        PWARN(" debug level is %d\n",  gBcmDebuglevel);
        break;

    case BRCM_IOCTL_GET_DCACHE_LINE_SIZE:
        {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30) && defined(CONFIG_ARM) || defined(CONFIG_ARM64)
            struct brcm_cache_info info;
            brcm_get_cache_info(&info);
            put_user(info.max_writeback, (unsigned __user *)arg);
#else
            unsigned max_dcache_line_size;
            max_dcache_line_size = cpu_data[0].dcache.linesz; /* L1 cache */
            if (cpu_data[0].scache.linesz > max_dcache_line_size) {
                max_dcache_line_size = cpu_data[0].scache.linesz; /* L2 cache */
            }
            copy_to_user((void *)arg, &max_dcache_line_size, sizeof(max_dcache_line_size));
#endif
        }
        break;

    case BRCM_IOCTL_ATOMIC_UPDATE:
        {
            t_bcm_linux_dd_atomic_update atomic_update_data;
            retValue = copy_from_user(&atomic_update_data, (void*)arg, sizeof(atomic_update_data));
#if defined(CONFIG_BRCMSTB_NEXUS_API) && BRCMSTB_H_VERSION >= 5
#if defined(CONFIG_ARM) && BRCMSTB_H_VERSION == 8
            /* This is a workaround for an issue inside of the 4.1-1.3 kernel release which fails
             * to handle address mappings for chips with BCHP_PHYSICAL_OFFSET of 0xD0000000. */
            if (g_breg.bchp_physical_offset == 0xD0000000) {
                atomic_update_data.reg &= ~0x20000000;
            }
#endif
            retValue = brcmstb_update32(atomic_update_data.reg, atomic_update_data.mask, atomic_update_data.value);
            if (retValue)
            {
                printk("brcmstb_update32(%#x,%#x,%#x) failed with %d\n",
                    atomic_update_data.reg,
                    atomic_update_data.mask,
                    atomic_update_data.value,
                    (int)retValue);
            }
#else
            {
                unsigned long flags = 0;
                uint32_t value;
                /* this spinlock synchronizes with any kernel use of a set of shared registers.
                see BREG_P_CheckAtomicRegister in magnum/basemodules/reg/breg_mem.c for the list of registers. */
                spin_lock_irqsave(&brcm_magnum_spinlock, flags);

                /* read/modify/write */
                value = breg_read(atomic_update_data.reg);
                value &= ~atomic_update_data.mask;
                value |= atomic_update_data.value;
                breg_write(atomic_update_data.reg, value);
                spin_unlock_irqrestore(&brcm_magnum_spinlock, flags);
            }
#endif
        }
        break;
#if BICM_ICAM2


    case BRCM_IOCTL_MULTI_ATOMIC_UPDATE:
        {
            unsigned long flags = 0;
            t_bcm_linux_dd_multi_atomic_update data;
            retValue = copy_from_user(&data, (void*)arg, sizeof(data));


            /* this spinlock synchronizes with any kernel use of a set of shared registers.
            see BREG_P_CheckAtomicRegister in magnum/basemodules/reg/breg_mem.c for the list of registers. */
            spin_lock_irqsave(&brcm_magnum_spinlock, flags);

            for (i=0;i<BCM_MULTI_ATOMIC_UPDATE_NUM;i++) {
        /*printk("\nset[%d], addr=0x%x, value=0x%x\n",i, data.set[i].reg, data.set[i].value);       */
                if (data.set[i].reg) {
                    uint32_t value;
                    /* read/modify/write */
                    value = breg_read(data.set[i].reg);
                    value &= ~data.set[i].mask;
                    value |= data.set[i].value;
                    breg_write(data.set[i].reg, value);
                }
                }
            spin_unlock_irqrestore(&brcm_magnum_spinlock, flags);

        }
        break;
#endif

    case BRCM_IOCTL_GET_TICKS:
        {
            tbcm_linux_dd_ticks tick;
            tick.jiffies = get_jiffies();
            tick.timerFreq = HZ;
            if (copy_to_user((void *)arg, &tick, sizeof(tbcm_linux_dd_ticks)))
            {
                result = -EFAULT;
                PERR("copy_to_user failed!\n");
                break;
            }
            break;
        }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    case BRCM_IOCTL_READ_RESERVED:
    case BRCM_IOCTL_WRITE_RESERVED:
        {
            t_bcm_linux_rw_reserved rw_reserved;
            volatile uint32_t *pMem;
            if (copy_from_user(&rw_reserved, (void *)arg, sizeof(rw_reserved)))
            {
                result = -EFAULT;
                PERR("RESERVED copy_from_user failed!\n");
                break;
            }
            if ( rw_reserved.address & 0x3 )
            {
                result = -EINVAL;
                PERR("Can only r/w reserved memory on 32-bit boundaries\n");
                break;
            }
            pMem = (volatile uint32_t *)phys_to_virt(rw_reserved.address);
            if ( cmd == BRCM_IOCTL_READ_RESERVED )
            {
                rw_reserved.value = *pMem;
                if ( copy_to_user((void *)arg, &rw_reserved, sizeof(rw_reserved)) )
                {
                    result = -EFAULT;
                    PERR("BRCM_IOCTL_READ_RESERVED copy_to_user failed!\n");
                    break;
                }
            }
            else
            {
                *pMem = rw_reserved.value;
                dma_cache_wback_inv((uint32_t)pMem, 4);
            }
        }
        break;
#ifdef CONFIG_BMIPS4380
    case BRCM_IOCTL_READ_CORE_REGISTER:
    case BRCM_IOCTL_WRITE_CORE_REGISTER:
        {
            t_bcm_linux_rw_core_register rw_reg;
            volatile uint32_t *pReg;
            if (copy_from_user(&rw_reg, (void *)arg, sizeof(rw_reg)))
            {
                result = -EFAULT;
                PERR("CORE_REGISTER copy_from_user failed!\n");
                break;
            }
            if ( rw_reg.offset & 0x3 || rw_reg.offset > BMIPS_RELO_VECTOR_CONTROL_1 )
            {
                result = -EINVAL;
                PERR("Invalid core register offset %#x\n", rw_reg.offset);
                break;
            }
            pReg = (volatile uint32_t *)(BMIPS_GET_CBR() + rw_reg.offset);
            if ( cmd == BRCM_IOCTL_READ_CORE_REGISTER )
            {
                rw_reg.value = *pReg;
                if ( copy_to_user((void *)arg, &rw_reg, sizeof(rw_reg)) )
                {
                    result = -EFAULT;
                    PERR("BRCM_IOCTL_READ_CORE_REGISTER copy_to_user failed!\n");
                    break;
                }
            }
            else
            {
                *pReg = rw_reg.value;
            }
        }
        break;
    case BRCM_IOCTL_READ_CMT_CONTROL:
        {
            uint32_t regval;
            regval = read_c0_brcm_cmt_ctrl();
            if ( copy_to_user((void *)arg, &regval, sizeof(regval)) )
            {
                result = -EFAULT;
                PERR("BRCM_IOCTL_READ_CMT_CONTROL copy_to_user failed!\n");
                break;
            }
        }
        break;
    case BRCM_IOCTL_WRITE_CMT_CONTROL:
        write_c0_brcm_cmt_ctrl(arg);
        break;
#endif  /* 4380 */
#endif  /* >= 2.6.30 */
    case BRCM_IOCTL_GET_DEBUG:
        if ( copy_to_user((void *)arg, debug_info, sizeof(debug_info)) )
            {
            result = -EFAULT;
            PERR("BRCM_IOCTL_SET_DEBUG copy_to_user failed!\n");
            break;
            }
        debug_info[0] = 0;
        break;

    case BRCM_IOCTL_SET_MMAP_ACCESS:
        {
            t_bcm_linux_mmap_access access;
            if (copy_from_user(&access, (void*)arg, sizeof(access))) {
                result = -EFAULT;
                PERR("BRCM_IOCTL_SET_MMAP_ACCESS copy_from_user failed!\n");
                break;
            }
            result = brcm_set_mmap_access(&access);
            if (result) {
                PERR("brcm_set_mmap_access failed: %d\n", result);
                result = -ENOMEM;
            }
        }
        break;
#if defined CONFIG_BMIPS5000 || defined CONFIG_CPU_BMIPS5000
    case BRCM_IOCTL_WRITE_CP0_25:
        {
            t_bcm_linux_write_cp0_25 cp0_25;
            if (copy_from_user(&cp0_25, (void*)arg, sizeof(cp0_25))) {
                result = -EFAULT;
                PERR("BRCM_IOCTL_WRITE_CP0_25 copy_from_user failed!\n");
                break;
            }
#define bcm_write_perf(sel, value)                  \
{       __asm__ __volatile__(".set\tpush\n\t"           \
            ".set\tmips32\n\t"                          \
            "mtc0\t%0, $25, " #sel "\n\t"   \
            ".set\tpop\n\t"                         \
            : /* none */                                \
            : "r" ((unsigned int)value));               \
}
            switch(cp0_25.select) {
            case 0: bcm_write_perf(0, cp0_25.value);break;
            case 1: bcm_write_perf(1, cp0_25.value);break;
            case 2: bcm_write_perf(2, cp0_25.value);break;
            case 3: bcm_write_perf(3, cp0_25.value);break;
            case 4: bcm_write_perf(4, cp0_25.value);break;
            case 5: bcm_write_perf(5, cp0_25.value);break;
            case 6: bcm_write_perf(6, cp0_25.value);break;
            case 7: bcm_write_perf(7, cp0_25.value);break;
            default:
                result = -EINVAL;
                PERR("BRCM_IOCTL_WRITE_CP0_25 invalid select:%u\n", cp0_25.select);
            }
        }
        break;
#endif
    case BRCM_IOCTL_FLUSH_DCACHE_RANGE:
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
        {
            t_bcm_linux_mem_addr_range mem_addr_range;

            if (copy_from_user(&mem_addr_range, (void*)arg, sizeof(mem_addr_range))) {
                result = -EFAULT;
                PERR("copy_from_user failed!\n");
                break;
            }

           /* address would get validated inside brcm_cpu_dcache_flush */
           /* coverity[tainted_data] */
           result = brcm_cpu_dcache_flush((void *)mem_addr_range.address, mem_addr_range.length);
        }
#else
        PERR("Not implemented for this platfrom\n");
#endif
        break; 

    case BRCM_IOCTL_PM_MEM_EXCLUDE:
#if defined(BRCMSTB_HAS_PM_MEM_EXCLUDE)
        {
            t_bcm_linux_pm_mem_exclude s;
            if (copy_from_user(&s, (void*)arg, sizeof(s))) {
                result = -EFAULT;
                PERR("BRCM_IOCTL_PM_MEM_EXCLUDE copy_from_user failed!\n");
                break;
            }
            PWARN("brcmstb_pm_mem_exclude(0x%llx, %llu)\n", s.addr, s.len);
            result = brcmstb_pm_mem_exclude(s.addr, s.len);
            if (result) {
                PERR("brcmstb_pm_mem_exclude failed: %d\n", result);
                result = -ENOMEM;
            }
        }
#else
        /* ignore */
        result = 0;
#endif
        break;

#if (defined(CONFIG_ARM) || defined(CONFIG_ARM64)) && defined(CONFIG_CMA)
    case BRCM_IOCTL_CMA_GETMEM:
        {
        struct bcmdriver_cma_getmem s;
        struct cma_dev *cma;
        result = copy_from_user(&s, (void*)arg, sizeof(s));
        if (result) {BERR_TRACE(result); break;}
        cma = cma_dev_get_cma_dev(s.cmaIndex);
        if (!cma) {result = -EFAULT; break; }
        result = cma_dev_get_mem(cma, &s.addr, s.size, s.alignment);
        if (!result) {
            result = copy_to_user((void*)arg, &s, sizeof(s));
            if (result) {BERR_TRACE(result); break;}
        }
        }
        break;
    case BRCM_IOCTL_CMA_PUTMEM:
        {
        struct bcmdriver_cma_putmem s;
        struct cma_dev *cma;
        result = copy_from_user(&s, (void*)arg, sizeof(s));
        if (result) {BERR_TRACE(result); break;}
        cma = cma_dev_get_cma_dev(s.cmaIndex);
        if (!cma) {result = -EFAULT; break; }
        result = cma_dev_put_mem(cma, s.addr, s.size);
        if (result) {BERR_TRACE(result); break;}
        }
        break;
    case BRCM_IOCTL_CMA_NUMREGIONS:
        {
        struct bcmdriver_cma_numregions s;
        struct cma_dev *cma;
        result = copy_from_user(&s, (void*)arg, sizeof(s));
        if (result) {BERR_TRACE(result); break;}
        cma = cma_dev_get_cma_dev(s.cmaIndex);
        if (!cma) {result = -EFAULT; break; }
        s.num = cma_dev_get_num_regions(cma);
        result = copy_to_user((void*)arg, &s, sizeof(s));
        if (result) {BERR_TRACE(result); break;}
        }
        break;
    case BRCM_IOCTL_CMA_GETREGIONINFO:
        {
        struct bcmdriver_cma_getregion s;
        struct cma_dev *cma;
        unsigned size;
        result = copy_from_user(&s, (void*)arg, sizeof(s));
        if (result) {BERR_TRACE(result); break;}
        cma = cma_dev_get_cma_dev(s.cmaIndex);
        if (!cma) {result = -EFAULT; break; }
        result = cma_dev_get_region_info(cma, s.region, &s.memc, &s.addr, &size);
        if (!result) {
            s.size = size;
            result = copy_to_user((void*)arg, &s, sizeof(s));
            if (result) {BERR_TRACE(result); break;}
        }
        }
        break;
    case BRCM_IOCTL_CMA_GETINFO:
        {
        struct bcmdriver_cma_getinfo s;
        struct cma_dev *cma;
        result = copy_from_user(&s, (void*)arg, sizeof(s));
        if (result) {BERR_TRACE(result); break;}
        cma = cma_dev_get_cma_dev(s.cmaIndex);
        if (!cma) {result = -EFAULT; break; }
        s.addr = cma->range.base;
        s.size = cma->range.size;
        s.memc = cma->memc;
        result = copy_to_user((void*)arg, &s, sizeof(s));
        if (result) {BERR_TRACE(result); break;}
        }
        break;
#endif /* defined(CONFIG_ARM) && defined(CONFIG_CMA) */
    case BRCM_IOCTL_GET_VERSION:
        {
        struct bcmdriver_version s;
        s.version = BCMDRIVER_VERSION;
        result = copy_to_user((void*)arg, &s, sizeof(s));
        if (result) {BERR_TRACE(result); break;}
        }
        break;
    case BRCM_IOCTL_DYNAMIC_REGION_ADD:
        {
        struct bcmdriver_dynamic_region s;
        b_memory_region region;
        result = copy_from_user(&s, (void*)arg, sizeof(s));
        if (result) {BERR_TRACE(result); break;}
        region.base = s.base;
        region.length = s.length;
        mutex_lock(&gBcmLock);
        result = b_memory_region_add(&g_state.dynamic_regions, &region);
        mutex_unlock(&gBcmLock);
        if(result) {
            BERR_TRACE(result);result = -EINVAL; break;
        }
        break;
        }
    case BRCM_IOCTL_DYNAMIC_REGION_REMOVE:
        {
        struct bcmdriver_dynamic_region s;
        b_memory_region region;
        result = copy_from_user(&s, (void*)arg, sizeof(s));
        if (result) {BERR_TRACE(result); break;}
        region.base = s.base;
        region.length = s.length;
        mutex_lock(&gBcmLock);
        result = b_memory_region_delete(&g_state.dynamic_regions, &region);
        mutex_unlock(&gBcmLock);
        if(result) {
            BERR_TRACE(result);result = -EINVAL; break;
        }
        break;
        }
    case BRCM_IOCTL_GET_MEMORY_INFO:
#if BRCMSTB_H_VERSION >= 1
    {
        struct {
            struct bcmdriver_memory_info info;
            struct brcmstb_memory memory;
        } *data;
        unsigned j;
        BDBG_CWARNING(BCMDRIVER_MAX_RANGES==MAX_BRCMSTB_RANGE);
        BDBG_CWARNING(BCMDRIVER_MAX_MEMC==MAX_BRCMSTB_MEMC);
        BDBG_CWARNING(BCMDRIVER_MAX_RESERVED_RANGES==MAX_BRCMSTB_RESERVED_RANGE);
        data=kmalloc(sizeof(*data), GFP_KERNEL);
        if(data==NULL) {
            BERR_TRACE(-ENOMEM); result = -ENOMEM; break;
        }
        memset(data, 0, sizeof(*data));
        brcmstb_memory_get(&data->memory);
        for(j=0;j<BCMDRIVER_MAX_MEMC;j++) {
            data->info.memc[j].count = data->memory.memc[j].count;
            for(i=0;i<BCMDRIVER_MAX_RANGES;i++) {
                if( (int)i>=data->memory.memc[j].count) {
                    break;
                }
                data->info.memc[j].range[i].addr = data->memory.memc[j].range[i].addr;
                data->info.memc[j].range[i].size = data->memory.memc[j].range[i].size;
            }
        }
        data->info.bmem.count = data->memory.bmem.count;
        for(i=0;i<BCMDRIVER_MAX_RANGES;i++) {
            data->info.bmem.range[i].addr = data->memory.bmem.range[i].addr;
            data->info.bmem.range[i].size = data->memory.bmem.range[i].size;
        }
        data->info.cma.count = data->memory.cma.count;
        for(i=0;i<BCMDRIVER_MAX_RANGES;i++) {
            data->info.cma.range[i].addr = data->memory.cma.range[i].addr;
            data->info.cma.range[i].size = data->memory.cma.range[i].size;
        }
        data->info.reserved.count = data->memory.reserved.count;
        for(i=0;i<BCMDRIVER_MAX_RESERVED_RANGES;i++) {
            data->info.reserved.range[i].addr = data->memory.reserved.range[i].addr;
            data->info.reserved.range[i].size = data->memory.reserved.range[i].size;
        }
        for(i=0;i<BCMDRIVER_MAX_RESERVED_RANGES;i++) {
#if BRCMSTB_H_VERSION >= 10
            BDBG_CASSERT(sizeof(data->info.reserved.range_name[i].name)==sizeof(data->memory.reserved.range_name[i].name));
            memcpy(data->info.reserved.range_name[i].name, data->memory.reserved.range_name[i].name, sizeof(data->memory.reserved.range_name[i].name));
#else
            memset(data->info.reserved.range_name, 0, sizeof(data->info.reserved.range_name));
#endif
        }
        data->info.lowmem.count = data->memory.lowmem.count;
        for(i=0;i<BCMDRIVER_MAX_RANGES;i++) {
            data->info.lowmem.range[i].addr = data->memory.lowmem.range[i].addr;
            data->info.lowmem.range[i].size = data->memory.lowmem.range[i].size;
        }
        result = copy_to_user( (void *)arg, &data->info, sizeof(data->info) );
        kfree(data);
        if(result != 0) {
            result = -EFAULT; break;
        }
        result = 0;
        break;
    }
#else
         result = -EINVAL; break;
#endif
    case BRCM_IOCTL_GET_VMALLOC:
    {
        struct bcmdriver_get_vmalloc s;
        result = copy_from_user(&s, (void*)arg, sizeof(s));
        if (result) {BERR_TRACE(result); break;}
        s.out.vmalloc = vmalloc_size(s.in.required_vmalloc, s.in.highest_installed_dram, s.in.lowest_bmem_addr);
        result = copy_to_user((void*)arg, &s, sizeof(s));
        if (result) {BERR_TRACE(result); break;}
        result = 0;
        break;
    }
    case BRCM_IOCTL_IRQ_CONTROL:
    {
        bcmdriver_irq_control irq_control;

        if (!g_virtual_irq_supported) { result = -ENOSYS; BERR_TRACE(result); break; }

        result = copy_from_user(&irq_control, (void*)arg, sizeof(irq_control));
        if (result) {BERR_TRACE(result); break;}
        switch(irq_control.command)
        {
            case bcmdriver_irq_command_status:
            {
                b_virtual_irq_get_status(&irq_control.data.status);
                result = copy_to_user( &(((bcmdriver_irq_control*)arg)->data.status.status_word), &irq_control.data.status.status_word, sizeof(irq_control.data.status.status_word) );
                if (result) {BERR_TRACE(result); break;}
                break;
            }
            case bcmdriver_irq_command_clear:
            {
                result = b_virtual_irq_clear(irq_control.data.clear.line);
                if (result) { BERR_TRACE(result); break; }
                break;
            }
            case bcmdriver_irq_command_mask:
            {
                result = b_virtual_irq_set_mask(irq_control.data.mask.line, irq_control.data.mask.disable);
                if (result) { BERR_TRACE(result); break; }
                break;
            }
            case bcmdriver_irq_command_control:
            {
                irq_control.data.control.os = b_virtual_irq_is_os_managed(irq_control.data.control.line);
                result = copy_to_user((void*)arg, &irq_control, sizeof(irq_control));
                if (result) {BERR_TRACE(result); break;}
                break;
            }
            default:
                result = -EINVAL; BERR_TRACE(result);
                break;
        }
        break;
    }
    case BRCM_IOCTL_IRQ_MAKE_GROUP:
    {
        b_virtual_irq_group group;
        if (!g_virtual_irq_supported) { result = -ENOSYS; BERR_TRACE(result); break; }
        result = copy_from_user(&group, (void*)arg, sizeof(group));
        if (result) { BERR_TRACE(result); break;}
        result = b_virtual_irq_make_group(&group);
        if (result) { BERR_TRACE(result); break; }
        break;
    }
    case BRCM_IOCTL_SHARED_GPIO_CONTROL:
    {
        bcmdriver_shared_gpio_control gpio_control;

        if (!g_shared_gpio_supported) { result = -ENOSYS; BERR_TRACE(result); break; }

        result = copy_from_user(&gpio_control, (void*)arg, sizeof(gpio_control));
        if (result) {BERR_TRACE(result); break;}
        switch(gpio_control.command)
        {
            case bcmdriver_shared_gpio_command_init_banks:
            {
                b_shared_gpio_init_banks(&gpio_control.data.init_banks);
                break;
            }
            case bcmdriver_shared_gpio_command_open_pin:
            {
                result = b_shared_gpio_open_pin(&gpio_control.data.open_pin.pin, gpio_control.data.open_pin.irq_type);
                if (result) { BERR_TRACE(result); break; }
                break;
            }
            case bcmdriver_shared_gpio_command_close_pin:
            {
                b_shared_gpio_close_pin(&gpio_control.data.close_pin.pin);
                break;
            }
            case bcmdriver_shared_gpio_command_get_int_status:
            {
                memset(&gpio_control.data.get_int_status, 0, sizeof(gpio_control.data.get_int_status));
                b_shared_gpio_get_int_status(&gpio_control.data.get_int_status);
                result = copy_to_user(&((bcmdriver_shared_gpio_control *)arg)->data.get_int_status, &gpio_control.data.get_int_status, sizeof(gpio_control.data.get_int_status));
                if (result) {BERR_TRACE(result); break;}
                break;
            }
            case bcmdriver_shared_gpio_command_clear_int:
            {
                result = b_shared_gpio_clear_int(&gpio_control.data.clear_int.pin);
                if (result) { BERR_TRACE(result); break; }
                break;
            }
            case bcmdriver_shared_gpio_command_set_int_mask:
            {
                result = b_shared_gpio_set_int_mask(&gpio_control.data.set_int_mask.pin, gpio_control.data.set_int_mask.disable);
                if (result) { BERR_TRACE(result); break; }
                break;
            }
            case bcmdriver_shared_gpio_command_set_standby:
            {
                result = b_shared_gpio_set_standby(&gpio_control.data.set_standby.pin, gpio_control.data.set_standby.enable);
                if (result) { BERR_TRACE(result); break; }
                break;
            }
            default:
                BERR_TRACE(-EINVAL); result = -EINVAL; break;
        }

        break;
    }
    case BRCM_IOCTL_GET_OS_CONFIG:
    {
        bcmdriver_os_config os_cfg;
        result = copy_from_user(&os_cfg, (void*)arg, sizeof(os_cfg));
        if (result) {BERR_TRACE(result); break;}
        os_cfg.virtual_irq_supported = g_virtual_irq_supported;
        os_cfg.shared_gpio_supported = g_shared_gpio_supported;
        os_cfg.os_64bit = false;
#if defined(CONFIG_64BIT)
        os_cfg.os_64bit = true;
#endif
        result = copy_to_user((void*)arg, &os_cfg, sizeof(os_cfg));
        if (result) {BERR_TRACE(result); break;}
        break;
    }
    case BRCM_IOCTL_SET_CHIP_INFO:
    {
        struct bcmdriver_chip_info chip_info;
        result = copy_from_user(&chip_info, (void*)arg, sizeof(chip_info));
        if (result) {BERR_TRACE(result); break;}
        g_breg.bchp_physical_offset = chip_info.bchp_physical_offset;
        break;
    }
    default:
        result = -ENOSYS;
        break;
    }

    return result;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0)
#define BUILD_RO_FOPS(name)					\
static int proc_open_##name(struct inode *inode, struct file *file) \
{								\
	return single_open(file, proc_read_##name, NULL);	\
}								\
static const struct file_operations read_fops_##name = {	\
	.open	=	proc_open_##name,			\
	.read	=	seq_read,				\
	.llseek =	seq_lseek,				\
	.release = 	seq_release,				\
};

#define BUILD_RW_FOPS(name)					\
static int proc_open_##name(struct inode *inode, struct file *file) \
{								\
	return single_open(file, proc_read_##name, NULL);	\
}								\
static const struct file_operations read_fops_##name = {	\
	.open	=	proc_open_##name,			\
	.read	=	seq_read,				\
	.write	=	proc_write_##name,			\
	.llseek =	seq_lseek,				\
	.release = 	seq_release,				\
};

BUILD_RW_FOPS(interrupts);
BUILD_RW_FOPS(debug);
BUILD_RW_FOPS(latency);
BUILD_RO_FOPS(version);
BUILD_RO_FOPS(meminfo);

#endif

/****************************************************************
* brcm_stb_init(void)
****************************************************************/
static int brcm_stb_init(void)
{
    unsigned i, irq;
    int rv = 0;
    unsigned long flags = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
    /* Leave interrupt disabled while we are handling  it */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    flags = IRQF_DISABLED;
#else
    flags = SA_INTERRUPT;
#endif
#endif

    PINFO("Initializing bcmdriver version $ %u\n", BCMDRIVER_VERSION);

    memset(&g_intmgr,0,sizeof(intmgr_t));
    brcm_init_mmap_access();
    b_memory_region_set_init(&g_state.dynamic_regions);

    /* Make sure to init this first, as we may get an interrupt as soon as we request it */
    init_waitqueue_head(&g_WaitQueue);

    /* init submodules */
    brcm_init_submodules();

    PINFO("Total intc words=%d,Total Irqs=%d\n",g_sChipConfig.IntcSize,g_sChipConfig.maxNumIrq);

    brcm_virtualize_l1s();

    /* Request interrupts that we manage */
    for (i=0,irq=1;i<g_sChipConfig.IntcSize;i++) {
        unsigned bitmask;
        g_intmgr.globalmask[i] = 0;
        for (bitmask=1;bitmask;bitmask <<= 1,irq++) {
            if ( g_sChipConfig.pIntTable[irq].manageInt & INT_ENABLE_MASK ) {
                unsigned long new_flags;

                if (gBcmDebug)
                    PWARN("Requesting %s irq (linux %d)\n", g_sChipConfig.pIntTable[irq].name,irq);

                new_flags = flags;
                if ( g_sChipConfig.pIntTable[irq].manageInt & INT_SHARABLE_MASK )
                {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
                    new_flags |= IRQF_SHARED;
#else
                    new_flags |= SA_SHIRQ;
#endif
                }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0)
                new_flags |= IRQF_TRIGGER_HIGH;
#endif
                /* Last parameter is don't care */
                if (request_irq( make_linux_irq(irq), brcm_interrupt, new_flags, g_sChipConfig.pIntTable[irq].name, (void*)(unsigned long)(irq+1)))
                {
                    PERR("request_irq failed, irq:%d name:%s\n", make_linux_irq(irq) , g_sChipConfig.pIntTable[irq].name);
                }

                g_intmgr.globalmask[i] |= bitmask;
            }
        }
    }

    print_int_status();

    /* Register driver last in case of errors during initialization */
    if (register_chrdev(BRCM_MAJOR,"brcm", &brcm_fops))
    {
        PERR("Unable to get major %d\n", BRCM_MAJOR);
        goto error;
    }
    /* create directory */
    umdrv_dir = proc_mkdir(MODULE_NAME, NULL);
    if (umdrv_dir == NULL)
    {
        rv = -ENOMEM;
        goto error;
    }
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
    umdrv_dir->owner = THIS_MODULE;
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
    version_file = create_proc_read_entry("version",0444, umdrv_dir,proc_read_version,NULL);
#else
    version_file = proc_create_data("version",0444, umdrv_dir,&read_fops_version,NULL);
#endif
    if (version_file == NULL)
    {
        rv  = -ENOMEM;
        goto no_version;
    }
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
    version_file->owner = THIS_MODULE;
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
    interrupt_file = create_proc_entry("interrupts", 0644, umdrv_dir);
    if (interrupt_file == NULL)
    {
        rv = -ENOMEM;
        goto no_version;
    }
    interrupt_file->read_proc = proc_read_interrupts;
    interrupt_file->write_proc = proc_write_interrupts;
#else
    interrupt_file = proc_create_data("interrupts", 0644, umdrv_dir, &read_fops_interrupts, NULL);
    if (interrupt_file == NULL)
    {
        rv = -ENOMEM;
        goto no_version;
    }
#endif
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
    interrupt_file->owner = THIS_MODULE;
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
    latency_file = create_proc_entry("latency", 0644, umdrv_dir);
    if (latency_file == NULL)
    {
        rv = -ENOMEM;
        goto no_version;
    }
    latency_file->read_proc = proc_read_latency;
    latency_file->write_proc = proc_write_latency;
#else
    latency_file = proc_create_data("latency", 0644, umdrv_dir, &read_fops_latency, NULL);
    if (latency_file == NULL)
    {
        rv = -ENOMEM;
        goto no_version;
    }
#endif
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
    latency_file->owner = THIS_MODULE;
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
    meminfo_file = create_proc_entry("meminfo", 0644, umdrv_dir);
    if (meminfo_file == NULL)
    {
        rv = -ENOMEM;
        goto no_version;
    }
    meminfo_file->read_proc = proc_read_meminfo;
#else
    meminfo_file = proc_create_data("meminfo", 0644, umdrv_dir, &read_fops_meminfo, NULL);
    if (meminfo_file == NULL)
    {
        rv = -ENOMEM;
        goto no_version;
    }
#endif
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
    meminfo_file->owner = THIS_MODULE;
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
    debug_file = create_proc_entry("debug", 0644, umdrv_dir);
    if (debug_file == NULL)
    {
        rv = -ENOMEM;
        goto no_version;
    }
    debug_file->read_proc = proc_read_debug;
    debug_file->write_proc = proc_write_debug;
#else
    debug_file = proc_create_data("debug", 0644, umdrv_dir, &read_fops_debug, NULL);
    if (debug_file == NULL)
    {
        rv = -ENOMEM;
        goto no_version;
    }
#endif
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
    debug_file->owner = THIS_MODULE;
#endif

    PINFO("Initialization complete\n");

    return 0;

no_version:
    remove_proc_entry(MODULE_NAME, NULL);

error:
    return rv;
}

static int __init __init_module(void)
{
    int ret=0;
    ret = brcm_stb_init();
    return ret;
}
module_init(__init_module);

static void __exit __cleanup_module(void)
{
    int i;

    PINFO("Cleanup_modules...\n");
    for ( i = 1; i <  g_sChipConfig.maxNumIrq; i++ )
    {
        if ( g_sChipConfig.pIntTable[i].manageInt & INT_ENABLE_MASK )
        {
            if (gBcmDebug)
                PWARN("Freeing %s irq (linux %d)\n", g_sChipConfig.pIntTable[i].name, make_linux_irq(i));

            /* if the function to disable the interrupt is provided call it now */
            if(g_sChipConfig.pIntTable[i].mask_func)
                g_sChipConfig.pIntTable[i].mask_func(0,NULL);

            free_irq(make_linux_irq(i), (void*)(unsigned long)(i+1));
        }
    }

    brcm_uninit_submodules();

    unregister_chrdev(BRCM_MAJOR, "brcm");

    remove_proc_entry("debug", umdrv_dir);
    remove_proc_entry("meminfo", umdrv_dir);
    remove_proc_entry("interrupts", umdrv_dir);
    remove_proc_entry("latency", umdrv_dir);
    remove_proc_entry("version", umdrv_dir);
    remove_proc_entry(MODULE_NAME, NULL);

    b_memory_region_set_shutdown(&g_state.dynamic_regions);
    brcm_uninit_mmap_access();

    PINFO("Cleanup complete\n");
}

module_exit(__cleanup_module);


#include "b_memory_regions.inc"
#include "b_vmalloc.inc"

static void brcm_setmask_l1(unsigned l1, bool disable)
{
    unsigned index = l1 / 32;
    unsigned bit = l1 % 32;
    if (disable)
    {
        g_intmgr.globalmask[index] &= ~(1 << bit);
    }
    else
    {
        g_intmgr.globalmask[index] |= 1 << bit;
    }
}

static void brcm_set_l1_status(unsigned l1)
{
    unsigned index = l1 / 32;
    unsigned bit = l1 % 32;
    g_intmgr.status[index] |= 1 << bit;
}

static void brcm_fire_gpio_l2(bool aon)
{
    if (g_virtual_irq_supported)
    {
        b_virtual_irq_software_l2_isr(aon ? b_virtual_irq_line_gio_aon : b_virtual_irq_line_gio);
    }
}

#if B_VIRTUAL_IRQ_OS_SUPPORT && !defined(B_VIRTUAL_IRQ_DISABLED)
static void brcm_close_submodules(void)
{
    b_shared_gpio_close_submodule();
    b_virtual_irq_close_submodule();
}

static int brcm_open_submodules(void)
{
    int rc = 0;
    rc = b_virtual_irq_open_submodule();
    if (rc) goto err;
    rc = b_shared_gpio_open_submodule();
    if (rc) goto err;
err:
    return rc;
}

static void brcm_uninit_submodules(void)
{
    b_shared_gpio_uninit_submodule();
    b_virtual_irq_uninit_submodule();
}

static void brcm_init_submodules(void)
{
    b_shared_gpio_module_init_settings gio_settings;
    b_shared_gpio_capabilities gio_caps;
    b_virtual_irq_capabilities irq_caps;

    b_virtual_irq_init_submodule();
    b_virtual_irq_get_capabilities(&irq_caps);
    g_virtual_irq_supported = irq_caps.feature_supported;
    if (g_virtual_irq_supported)
    {
        gio_settings.gio_linux_irq = b_virtual_irq_get_linux_irq(b_virtual_irq_line_gio);
        gio_settings.gio_aon_linux_irq = b_virtual_irq_get_linux_irq(b_virtual_irq_line_gio_aon);
    }
    else
    {
        gio_settings.gio_linux_irq = 0;
        gio_settings.gio_aon_linux_irq = 0;
    }
    b_shared_gpio_init_submodule(&gio_settings);
    b_shared_gpio_get_capabilities(&gio_caps);
    g_shared_gpio_supported = gio_caps.feature_supported;
}

static void brcm_virtualize_l1s(void)
{
    unsigned i;

    if (!g_virtual_irq_supported) return;

    /* mark virtual L1s as virtual */
    for (i = 0; i < 32 * g_sChipConfig.IntcSize; i++) {
        const char *irqName = g_sChipConfig.pIntTable[i].name;
        int flags = g_sChipConfig.pIntTable[i].manageInt;
        if (irqName == NULL) {
            continue;
        }
        if ((flags & INT_ENABLE_MASK) == 0) {
            continue;
        }
        if (b_virtual_irq_l1_is_virtual(irqName))
        {
            flags &= ~INT_ENABLE_MASK;
            flags |= INT_VIRTUAL_MASK;
            g_sChipConfig.pIntTable[i].manageInt = flags;
        }
    }
}
#else
static void brcm_init_submodules(void)
{
    g_virtual_irq_supported = false;
    g_shared_gpio_supported = false;
}

static void brcm_uninit_submodules(void) {}
static void brcm_close_submodules(void) {}
static int brcm_open_submodules(void) { return 0; }
static void brcm_virtualize_l1s(void) {}
#endif

#define B_VIRTUAL_IRQ_SPIN_LOCK(flags) spin_lock_irqsave(&gSpinLock, flags)
#define B_VIRTUAL_IRQ_SPIN_UNLOCK(flags) spin_unlock_irqrestore(&gSpinLock, flags)
#define B_VIRTUAL_IRQ_GET_L1_WORD_COUNT() (g_sChipConfig.IntcSize)
#define B_VIRTUAL_IRQ_MASK_L1(L1) brcm_setmask_l1(L1, true)
#define B_VIRTUAL_IRQ_UNMASK_L1(L1) brcm_setmask_l1(L1, false)
#define B_VIRTUAL_IRQ_SET_L1_STATUS(L1) brcm_set_l1_status(L1)
#define B_VIRTUAL_IRQ_INC_L1(L1) (g_sChipConfig.pIntTable[(L1)+1].numInter++)
#define B_VIRTUAL_IRQ_WAKE_L1_LISTENERS() wake_up_interruptible(&g_WaitQueue)
#include "b_virtual_irq.inc"
#include "b_virtual_irq_usermode.inc"
#define B_SHARED_GPIO_FIRE_GPIO_L2(AON) brcm_fire_gpio_l2(AON)
#define B_SHARED_GPIO_SPIN_LOCK(flags) spin_lock_irqsave(&gSpinLock, flags)
#define B_SHARED_GPIO_SPIN_UNLOCK(flags) spin_unlock_irqrestore(&gSpinLock, flags)
#include "b_shared_gpio.inc"
#include "b_shared_gpio_usermode.inc"
#define B_OS_IRQ_SPIN_LOCK(flags) spin_lock_irqsave(&gSpinLock, flags)
#define B_OS_IRQ_SPIN_UNLOCK(flags) spin_unlock_irqrestore(&gSpinLock, flags)
#include "b_os_irq.inc"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,18)
MODULE_LICENSE("Proprietary");
#endif
/* End of file */
