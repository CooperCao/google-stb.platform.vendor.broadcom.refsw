/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2002-2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ***************************************************************************/
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,1)
#include <linux/kconfig.h>
#include <linux/of.h>
#endif
#include <linux/delay.h>
#include <asm/cacheflush.h>
#include "bcmdriver_arm.h"

unsigned __brcm_cache_line_size(void);
void brcm_get_cache_info(struct brcm_cache_info*info)
{
    info->max_writeback = __brcm_cache_line_size();
    info->max_writethrough = info->max_writeback;
    info->prefetch_enabled = 1;
    return;
}

#if defined(CONFIG_ARM64)

#else /* #if defined(CONFIG_ARM64) */

int __brcm_cpu_dcache_flush(const void *addr, size_t size);
static int brcm_cpu_dcache_flush_only(const void *addr, size_t size)
{
#if BCHP_CHIP == 7271 || BCHP_CHIP == 7268 || BCHP_CHIP == 7260
    /* temporary removal of flushall for 32-bit B53 */
#else
    /* if range is > 3MB,dumping entire cache is faster
      than line by line flush with inline assembly */
    if( size >= 3*1024*1024) {
        __cpuc_flush_kern_all();
        return 0;
    }
#endif
    return __brcm_cpu_dcache_flush(addr, size);
}

int brcm_cpu_dcache_flush(const void *pvAddr, size_t ulNumBytes)
{
    int result = brcm_cpu_dcache_flush_only(pvAddr, ulNumBytes);

#ifdef USE_BRCMSTB_MEGA_BARRIER
    brcmstb_mega_barrier(); /* available in 3.8-2.3 and later */
#endif

    return result;
}
#endif /* #if defined(CONFIG_ARM64) */
