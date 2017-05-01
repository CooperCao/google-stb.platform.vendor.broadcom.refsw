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
#include <linux/version.h>
#include "nexus_base_ioctl.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
#include <linux/kconfig.h>
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/signal.h>
#include <linux/kernel.h>
#include "bchp_common.h"

#if NEXUS_USE_CMA
    /* no #include */
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,1)
    /* ARM bmem */
    #include <linux/brcmstb/bmem.h>
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
    #include <linux/brcmstb/brcmapi.h> /* bmem_region_info */
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    #include <asm/brcmstb/brcmapi.h> /* bmem_region_info */
#else
#undef BCHP_PHYSICAL_OFFSET
    #include <asm/brcmstb/common/brcmstb.h>
#undef BCHP_PHYSICAL_OFFSET

    /* These are included in the 2.6.12-4.0 kernel and later. */
    #if HAVE_RSRVD
    extern unsigned long get_RAM_size();
    extern unsigned long get_RSVD_size();
    #endif
#endif

#include "priv/nexus_generic_driver.h"

#if NEXUS_CPU_ARM
#include "bcmdriver_arm.c"
#endif

static int      nexus_driver_open(struct inode *inode, struct file *file);
static int      nexus_driver_close(struct inode *inode, struct file * file);
static long nexus_driver_ioctl(struct file * file, unsigned int cmd, unsigned long arg, bool compat);
static int      nexus_driver_mmap(struct file *file, struct vm_area_struct *vma);

static long
nexus_driver_ioctl_native(struct file * file, unsigned int cmd, unsigned long arg)
{
    return nexus_driver_ioctl(file, cmd, arg, false);
}

#ifdef CONFIG_COMPAT
static long
nexus_driver_ioctl_compat(struct file * file, unsigned int cmd, unsigned long arg)
{
#if NEXUS_COMPAT_32ABI
    return nexus_driver_ioctl(file, cmd, arg, true);
#else
    /* printk("32-bit ABI is not supported\n"); */
    return -EINVAL;
#endif
}
#endif

static struct file_operations
nexus_driver_fops = {
    owner:          THIS_MODULE,
    read:           NULL,
    write:          NULL,
    unlocked_ioctl: nexus_driver_ioctl_native,
#ifdef CONFIG_COMPAT
    compat_ioctl: nexus_driver_ioctl_compat,
#endif
    mmap:           nexus_driver_mmap,
    open:           nexus_driver_open,
    release:        nexus_driver_close
};

static char b_env[256]="";
static char b_devname[16]="nexus";
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
module_param_string(config, b_env, sizeof(b_env), 0);
module_param_string(devname, b_devname, sizeof(b_devname), 0);
#endif

#ifdef B_REFSW_ANDROID
#include <linux/miscdevice.h>
static struct miscdevice nexus_miscdevice = {
   .minor = MISC_DYNAMIC_MINOR,
   .name = b_devname,
   .fops = &nexus_driver_fops,
};
#endif

static void
b_parse_env(char *env)
{
    char *s;
    const char *name;
    const char *value;
    /* traverse  string, and split it to name/value pairs */
    for(s=env, name=env, value=NULL;;s++) {
        switch(*s) {
        case '\0':
            goto done;
        case '=':
            *s = '\0';
            value = s+1;
            break;
        case ' ':
        case ':':
        case ';':
            *s = '\0';
            if (value==NULL) {
                value=s;
            }
            NEXUS_SetEnv(name, value);
            name = s+1;
            value = NULL;
            break;
        default:
            break;
        }
    }
done:
    if(*name) {
        if (value==NULL) {
            value=s;
        }
        NEXUS_SetEnv(name, value);
    }
    return;
}

static int
nexus_driver_open(struct inode *inode, struct file *file)
{
    int rc;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
    rc = nexus_generic_driver_open(0, &file->private_data, current->group_leader->pid, NULL, current->euid == 0);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0) /*TODO, find which kernel version has forced this change */
    rc = nexus_generic_driver_open(0, &file->private_data, current->group_leader->pid, current->comm, current_euid() == 0);
#else
    kuid_t euid;
    euid = current_euid();
    rc = nexus_generic_driver_open(0, &file->private_data, current->group_leader->pid, current->comm, euid.val == 0);
#endif
    if (rc) return -ENODEV;
    return 0;
}

static int
nexus_driver_close(struct inode *inode, struct file *file)
{
    nexus_generic_driver_close(0, file->private_data, signal_pending(current));
    return 0;
}

#define B_IS_INTERSECT(off1, len1, off2, len2) ((off1) <= ((off2)+(len2)-1) && (off2) <= ((off1)+(len1)-1))

static int nexus_driver_mmap(struct file *file, struct vm_area_struct *vma) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    int rc;
    uint64_t offset = ((uint64_t)vma->vm_pgoff) << PAGE_SHIFT;

    rc = nexus_generic_driver_validate_mmap(0 /* unused */, file->private_data, offset, vma->vm_end - vma->vm_start);
    if (rc) return -EINVAL;
    
    /* allow uncached mmap */    
    if (file->f_flags & O_SYNC) {
        vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    }
#if NEXUS_CPU_ARM && !NEXUS_CPU_ARM64 /* Arm64 kernel doesn't use these PTE options */
    else {
        /*
         * ARM has an explicit setting for WC. Use default for other
         * architectures.
         */
         vma->vm_page_prot = __pgprot_modify(vma->vm_page_prot,
                                    L_PTE_MT_MASK, L_PTE_MT_BUFFERABLE | L_PTE_MT_WRITEBACK);
    }
#if 0
    vma->vm_page_prot = __pgprot_modify(vma->vm_page_prot, L_PTE_MT_MASK, L_PTE_XN); /* /dev/nexus should not be used to map memory which is executable */
#else
    if(B_IS_INTERSECT(vma->vm_start, vma->vm_end - vma->vm_start, BCHP_PHYSICAL_OFFSET, BCHP_PHYSICAL_OFFSET + BCHP_REGISTER_END)) {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED); /* mapping of register window is not supported */
        return -EINVAL;
    }
#endif
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

static long
nexus_driver_ioctl(struct file * file, unsigned int cmd, unsigned long arg, bool compat)
{
    int rc;
    unsigned module = NEXUS_IOCTL_NUM(cmd) / NEXUS_IOCTL_PER_MODULE;
    current->flags |= PF_NOFREEZE;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
    current->flags |= PF_FREEZER_SKIP;
#endif
    rc = nexus_generic_driver_ioctl(module, file->private_data, cmd, arg, compat);
    current->flags &= ~PF_NOFREEZE;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
    current->flags &= ~PF_FREEZER_SKIP;
#endif
    
    if (rc) return -EFAULT;
    return 0;
}

int
nexus_init_module(void)
{
    int rc;
    struct nexus_generic_driver_init_settings settings;
    unsigned i;

    memset(&settings, 0, sizeof(settings));
    BSTD_UNUSED(i);

#if NEXUS_USE_CMA
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    for (i=0;i<sizeof(settings.region)/sizeof(settings.region[0]);i++) {
  #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,1)
        phys_addr_t offset, size;
  #else
        unsigned long offset, size;
  #endif
        rc = bmem_region_info(i, &offset, &size);
        if (rc) break;
        settings.region[i].offset = offset;
        settings.region[i].size = size;
    }
#elif HAVE_RSRVD
#define LOWERMEM (256*1024*1024)
    if (get_RAM_size() > LOWERMEM) {
        /* get_RSVD_size() only applies to amount reserved from lower 256, not amount reserved from get_RAM_size() */
        settings.region[0].size = get_RSVD_size();
        settings.region[0].offset = LOWERMEM - settings.region[0].size;
        /* do not program settings.region[1] for upper 256. for 2.6.18, assume that it will be used by the kernel.
        2.6.31 and later gives options for nexus/magnum use. */
    }
    else {
        settings.region[0].size = get_RSVD_size();
        settings.region[0].offset = get_RAM_size() - settings.region[0].size;
    }
#else
#error
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)) && (NEXUS_CPU_ARM || NEXUS_CPU_ARM64)
    {
        struct brcm_cache_info info;
        brcm_get_cache_info(&info);
        settings.max_dcache_line_size = info.max_writeback;
}
    /* If MEM's alignment is not set to the MIPS L1 and (if present) L2 cache line size,
    we will have cache coherency problems (which lead to major system failures).
    This code verifies that Nexus's MEM configuration is compatible with the MIPS cache line size.
    If this code fails, please check to make sure the Linux kernel is configured right, then modify nexus_core_features.h to match. */
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)) && (NEXUS_CPU_ARM || NEXUS_CPU_ARM64)
    {
        struct brcm_cache_info info;
        brcm_get_cache_info(&info);
        settings.max_dcache_line_size = info.max_writeback;
    }
#else
    settings.max_dcache_line_size = cpu_data[0].dcache.linesz; /* L1 cache */
    if (cpu_data[0].scache.linesz > settings.max_dcache_line_size) {
        settings.max_dcache_line_size = cpu_data[0].scache.linesz; /* L2 cache */
    }
    /* Ignoring cpu_data[0].tcache.linesz for now. */
#endif

    b_parse_env(b_env);
    rc = nexus_generic_driver_init(&settings);
    if (rc) goto error;

#ifdef B_REFSW_ANDROID
    misc_register(&nexus_miscdevice);
#else
    register_chrdev(NEXUS_DRIVER_MAJOR, b_devname, &nexus_driver_fops);
#endif

    printk("nexus driver initialized\n");
    return 0;

error:
    return rc;
}

void
nexus_cleanup_module(void)
{
#ifdef B_REFSW_ANDROID
    misc_deregister(&nexus_miscdevice);
#else
    unregister_chrdev(NEXUS_DRIVER_MAJOR, b_devname);
#endif
    nexus_generic_driver_uninit();

    printk("nexus driver uninitialized\n");
    return;
}

/* needed for local init with proxy */
EXPORT_SYMBOL(nexus_generic_driver_open);
EXPORT_SYMBOL(nexus_generic_driver_close);

MODULE_LICENSE("Proprietary");

/* if NEXUS_EXTERNAL_MODULE_INIT is defined, nexus_driver.c is being compiled into
another module which will connect to Linux. */
#ifndef NEXUS_EXTERNAL_MODULE_INIT
#ifdef MODULE
module_init(nexus_init_module);
module_exit(nexus_cleanup_module);
#else
#error "Not supported"
#endif
#endif
