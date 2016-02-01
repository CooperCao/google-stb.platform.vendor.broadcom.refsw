/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
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
 * User mode driver with 128 bit interrupt support.
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
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
#if B_NETIF_DMA
#include "netif_dma.h"
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0)
#include <linux/seq_file.h>
#include <linux/uidgid.h>
#endif
#include <stdbool.h>
#include "b_memory_regions.h"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,1)
#include <linux/brcmstb/brcmstb.h>
#endif

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
#else
#define BVIRTADDR(offset) (KSEG1 + 0x10000000 + offset)
#endif

#define BERR_TRACE(rc) do {if (rc) PERR("error %d at line %d\n", rc, __LINE__);}while(0)
#define BDBG_CWARNING(expr) do {if(0){int unused = 1/(expr);unused++;}} while(0)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,1)
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#include "bcmdriver_arm.c"
#include "linux/brcmstb/cma_driver.h"
#endif
/* temp stub */
#define dma_cache_wback_inv(ADDR,SIZE)  printk( KERN_ERR "BCMDRV: need to port dma_cache_wback_inv for arm linux\n\n")
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

#ifdef HAVE_RSRVD
/* don't call get_RAM_size(). it will return total memory on the board, even though upper memory may be unusable. e.g. 512M board may only have 256M usable.
this may need to change in the future. */
#define BOARD_MEM_SIZE       (get_RAM_size()>(256*1024*1024)?(256*1024*1024):get_RAM_size())
#define KERNEL_MEM_SIZE     ( BOARD_MEM_SIZE - get_RSVD_size() )
#else
/* default to these values for now */
#define KERNEL_MEM_SIZE     (32*1024*1024)
#define BOARD_MEM_SIZE      (256*1024*1024)
#endif

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
static void brcm_enable_irq(intmgr_t *p_intmgr, unsigned long Mask[]);

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
#define DEBUG_INFO_LEN 256
char debug_info[DEBUG_INFO_LEN];

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

static struct file_operations brcm_fops = {
    owner:      THIS_MODULE,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
    unlocked_ioctl:      brcm_ioctl,
#else
    ioctl:      brcm_ioctl,
#endif
    open:       brcm_open,
    release:    brcm_close,
    mmap:       brcm_mmap
};

static void brcm_enable_irq(intmgr_t *p_intmgr, unsigned long Mask[])
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
    spin_unlock_irqrestore(&gSpinLock, flags);
}

/**
Reenable all interrupts which we manage where status is cleared.
This can happen because when cores are reset and they reset their
status bits. */
static void brcm_reenable_irq(intmgr_t *p_intmgr)
{
    int irq;

    for (irq = 1; irq < g_sChipConfig.maxNumIrq; ++irq)
    {
        unsigned long flags;
        spin_lock_irqsave(&gSpinLock, flags);
        if (g_sChipConfig.pIntTable[irq].disabled) {
            if(gBcmDebug)
                PINFO("Re-enabling %s irq (linux %d)\n", g_sChipConfig.pIntTable[irq].name, make_linux_irq(irq) );
            enable_irq(make_linux_irq(irq));
            g_sChipConfig.pIntTable[irq].disabled = 0;
        }
        spin_unlock_irqrestore(&gSpinLock, flags);
    }
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
static int proc_write_interrupts(struct file *file,const char *buffer,unsigned long count,void *data)
#else
static int proc_write_interrupts(struct file *file, const char __user *buffer, size_t count, loff_t *data)
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
static int proc_write_latency(struct file *file,const char *buffer,unsigned long count,void *data)
#else
static int proc_write_latency(struct file *file, const char __user *buffer,
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
#else
static int proc_read_meminfo(struct seq_file *m, void *v)
#endif
{
    int len = 0, temp_len;
    static int kernelMem = -1;
    static int boardMem = -1;


    if ( -1 == kernelMem )
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0) /*to do use DT to determine this*/
        kernelMem = 0x10000000;
        boardMem = 0x40000000;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30) 
        unsigned long start = 0, len = 0;
        bmem_region_info(0, &start, &len);
        kernelMem = start;
        boardMem = start + len;
#else
        kernelMem = KERNEL_MEM_SIZE;
        boardMem = BOARD_MEM_SIZE;
#endif
    }

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
    len = snprintf(page, 0, "Kernel memory %d \n", kernelMem); /* returns the number of characters needed */
    len = snprintf(page, (len+1), "Kernel memory %d \n", kernelMem);

    temp_len = snprintf(page+len, 0, "Reserved Av memory %d \n", (boardMem-kernelMem));
    temp_len = snprintf(page+len, (temp_len+1), "Reserved Av memory %d \n", (boardMem-kernelMem));

    len += temp_len;
#else
    temp_len=0;
    seq_printf(m, "Kernel memory %d \n", kernelMem); /* returns the number of characters needed */
    seq_printf(m, "Kernel memory %d \n", kernelMem);

    seq_printf(m, "Reserved Av memory %d \n", (boardMem-kernelMem));
    seq_printf(m, "Reserved Av memory %d \n", (boardMem-kernelMem));
#endif

    return len;
}


#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static int proc_read_debug(char *page, char **start,off_t off, int count,int *eof, void *data)
#else
static int proc_read_debug(struct seq_file *m, void *v)
#endif
{
    return 0;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static int proc_write_debug(struct file *file, const char *buffer, unsigned long count, void *data)
#else
static int proc_write_debug(struct file *file, const char __user *buffer, size_t count, loff_t *data)
#endif
{
    if(count>=DEBUG_INFO_LEN) {
        /* SW7335-1355, Coverity: 26492 */
        count = (DEBUG_INFO_LEN-1);
        printk("text echo'd to debug exceeds max length, truncating. Max len=%d\n", DEBUG_INFO_LEN);
    }


    if (copy_from_user(debug_info, buffer, count))
    {
        count = -1;
        goto end;
    }

    if (count && debug_info[count-1] == '\n')
        debug_info[count - 1] = 0;
    else
        debug_info[count] = 0;

end:
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

/****************************************************************
* brcm_open(struct inode *inode, struct file *file)
****************************************************************/
static int brcm_open(struct inode *inode, struct file *file)
{
    int err = 0;
    unsigned int devnum, devmode;

    /* This must be the first operation to avoid auto-unload */

    brcm_get_dev_info(inode, &devnum, &devmode);

    if (gBcmDebug)
        printk("brcm_open devnum=%d devmode=%d\n",devnum,devmode);

    if ( devnum != 0 )
    {
        return -EFAULT;
    }

    file->private_data = (void*)((b_get_euid() == 0) ? 0 : -1);

    return err;
}

/****************************************************************
* brcm_close(struct inode *inode, struct file *file)
****************************************************************/
static int brcm_close(struct inode *inode, struct file *file)
{
    if (file == g_interrupt_file) {
        g_interrupt_file = NULL;
    }
    wake_up_interruptible(&g_WaitQueue);
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
    if (revoke==0) {
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

#if B_NETIF_DMA
    result = bnetif_ioctl(inode, file, cmd, arg);
    if (result!=-ENOSYS) {
        return  result;
    }
#endif
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
                    ptr += snprintf(ptr, 64, " W%d=0x%08lX", i, intStruct.interruptmask[i]);
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
                        PERR("Invalid wait irpt mask[%d]=0x%08lX,global mask[%d]=0x%08lX\n",i,intStruct.interruptmask[i],i,g_intmgr.globalmask[i]);
                    }
                    /* mask off the invalid bits */
                    intStruct.interruptmask[i] &= g_intmgr.globalmask[i];
                }

            }
            /* Re-enable any interrupts that were previously reported and handled */
            brcm_enable_irq(&g_intmgr, intStruct.interruptmask);

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
                    ptr += snprintf(ptr, 64, " W%d=0x%08lX", i, intStruct.interruptstatus[i]);
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
        brcm_reenable_irq(&g_intmgr);
        break;

    case BRCM_IOCTL_DEBUG_LEVEL:
        gBcmDebuglevel = (int)arg;
        PWARN(" debug level is %d\n",  gBcmDebuglevel);
        break;

    case BRCM_IOCTL_GET_DCACHE_LINE_SIZE:
        {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
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
            unsigned long flags = 0;
            uint32_t value;
            t_bcm_linux_dd_atomic_update atomic_update_data;
            retValue = copy_from_user(&atomic_update_data, (void*)arg, sizeof(atomic_update_data));

            /* this spinlock synchronizes with any kernel use of a set of shared registers.
            see BREG_P_CheckAtomicRegister in magnum/basemodules/reg/breg_mem.c for the list of registers. */
            spin_lock_irqsave(&brcm_magnum_spinlock, flags);

            /* read/modify/write */
            value = *(volatile uint32_t*) BVIRTADDR(atomic_update_data.reg);
            value &= ~atomic_update_data.mask;
            value |= atomic_update_data.value;
            *(volatile uint32_t*) BVIRTADDR(atomic_update_data.reg) = value;
            spin_unlock_irqrestore(&brcm_magnum_spinlock, flags);
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
                    value = *(volatile uint32_t*) BVIRTADDR(data.set[i].reg);
                    value &= ~data.set[i].mask;
                    value |= data.set[i].value;
                    *(volatile uint32_t*) BVIRTADDR(data.set[i].reg) = value;
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
        memset(debug_info, 0, sizeof(debug_info));
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
            PWARN("brcmstb_pm_mem_exclude(0x%llx, %zu)\n", s.addr, s.len);
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
        for(i=0;i<BCMDRIVER_MAX_RANGES;i++) {
            data->info.reserved.range[i].addr = data->memory.reserved.range[i].addr;
            data->info.reserved.range[i].size = data->memory.reserved.range[i].size;
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

    PINFO("Total intc words=%d,Total Irqs=%d\n",g_sChipConfig.IntcSize,g_sChipConfig.maxNumIrq);

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
                if (request_irq( make_linux_irq(irq), brcm_interrupt, new_flags, g_sChipConfig.pIntTable[irq].name, (void*)irq+1))
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

#if B_NETIF_DMA
    init_bnetif_dma();
#endif
    ret = brcm_stb_init();

    return ret;
}
module_init(__init_module);

static void __exit __cleanup_module(void)
{
    int i;

    PINFO("Cleanup_modules...\n");
#if B_NETIF_DMA
    cleanup_bnetif_dma();
#endif

    for ( i = 1; i <  g_sChipConfig.maxNumIrq; i++ )
    {
        if ( g_sChipConfig.pIntTable[i].manageInt & INT_ENABLE_MASK )
        {
            if (gBcmDebug)
                PWARN("Freeing %s irq (linux %d)\n", g_sChipConfig.pIntTable[i].name, make_linux_irq(i));

            /* if the function to disable the interrupt is provided call it now */
            if(g_sChipConfig.pIntTable[i].mask_func)
                g_sChipConfig.pIntTable[i].mask_func(0,NULL);

            free_irq(make_linux_irq(i), (void*)i+1);
        }
    }

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

#define BDBG_LOG(x)
#define BDBG_FILE_MODULE(x) extern int __unused
#define BDBG_MODULE_MSG(x,y) PDEBUG y
#define BDBG_ASSERT(x)  ((x) ? (void)0 : (void)0)
#define BKNI_Memcmp     memcmp
#define BKNI_Free(x) kfree(x)
#define BKNI_Malloc(x) kmalloc(x, GFP_KERNEL)

#include "b_memory_regions.inc"
#include "b_vmalloc.inc"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,18)
MODULE_LICENSE("Proprietary");
#endif
/* End of file */
