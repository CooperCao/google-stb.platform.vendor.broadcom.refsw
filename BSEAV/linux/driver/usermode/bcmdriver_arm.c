/***************************************************************************
 *     Copyright (c) 2002-2014, Broadcom Corporation
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
 * This modules needs to be included from the source file
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,1)
#include <linux/kconfig.h>
#include <linux/of.h>
#endif
#include <linux/delay.h>
#include <asm/cacheflush.h>
#include "bcmdriver_arm.h"

#if defined(CONFIG_ARM64)
#define USE_INLINE_ASSEMBLY 0

inline void __cpuc_flush_kern_all(void)
{

}
#else
#define USE_INLINE_ASSEMBLY 1
#endif

static int brcm_cpu_dcache_flush_only(const void *addr, size_t size)
{
    /* start_addr is inclusive and end_addr is exclusive */
    int result = 0;
    uint32_t line_size;
    uint32_t shift;
    unsigned long aligned_start_addr;
    int num_of_lines;


    /* if range is > 3MB,dumping entire cache is faster
      than line by line flush with inline assembly */
    if( size >= 3*1024*1024) {
        __cpuc_flush_kern_all();
        return 0;
    }

    /* Get the D-cache line size, and it's associated shift value (in words) */
    dcache_line_size(&line_size, &shift);

    /* Also factor in bytes/words */
    shift += 2;
    /* Floor start address */
    aligned_start_addr = (unsigned long)addr;
    aligned_start_addr = (aligned_start_addr >> shift) << shift;
    /* Ceil number of cache lines */
    num_of_lines = (size + ((unsigned long)addr - aligned_start_addr) + (line_size - 1)) >> shift;

    /* printk("%p:%#lx %u -> %d\n", addr, aligned_start_addr, (unsigned)size, num_of_lines); */

    do {
#if USE_INLINE_ASSEMBLY
        /* DCCIMVAC:Invalidate data cache line by VA to PoC */
        __asm__ __volatile__ (
            "1: mcr p15, 0, %2, c7, c14, 1\n"
            "2:\n"
            "   .pushsection .fixup,\"ax\"\n"
            "   .align  2\n"
            "3: mov     %0, %1\n"
            "   b       2b\n"
            "   .popsection\n"
            "   .pushsection __ex_table,\"a\"\n"
            "   .align  3\n"
            "   .long   1b, 3b\n"
            "   .popsection\n"
            : "+r" (result)
            : "i" (-EFAULT), "r" (aligned_start_addr)
        );
#else
        result = dcache_clean_inval_line(aligned_start_addr);
#endif
        if(result!=0) {
            break;
        }
        aligned_start_addr += line_size;
        num_of_lines --;
    } while(num_of_lines > 0);
#if USE_INLINE_ASSEMBLY
    __asm__("dsb");
#endif
    return result;
}

int brcm_cpu_dcache_flush(const void *pvAddr, size_t ulNumBytes)
{
    int result = brcm_cpu_dcache_flush_only(pvAddr, ulNumBytes);

#ifdef USE_BRCMSTB_MEGA_BARRIER
    brcmstb_mega_barrier(); /* available in 3.8-2.3 and later */
#endif

    return result;
}


void brcm_get_cache_info(struct brcm_cache_info*info)
{
    uint32_t line_size;
    uint32_t shift;
    dcache_line_size(&line_size, &shift);
    info->max_writeback = line_size;
    info->max_writethrough = line_size;
    info->prefetch_enabled=1;
}
