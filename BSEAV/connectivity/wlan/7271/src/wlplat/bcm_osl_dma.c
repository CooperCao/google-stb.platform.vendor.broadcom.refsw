/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include <typedefs.h>
#include <bcmendian.h>
#include <linuxver.h>
#include <bcmdefs.h>
#include <osl.h>
#include <osl_decl.h>
#include <linux_osl_priv.h>

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ethtool.h>
#include <linux/completion.h>
#include <linux/random.h>
#include <asm/cacheflush.h>

uint
osl_dma_consistent_align(void)
{
	return (PAGE_SIZE);
}
EXPORT_SYMBOL(osl_dma_consistent_align);

void*
osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align_bits, uint *alloced, dmaaddr_t *pap)
{
	void *va;
	dma_addr_t pap_lin;
	uint16 align = (1 << align_bits);
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (!ISALIGNED(DMA_CONSISTENT_ALIGN, align))
	 size += align;
	*alloced = size;

#if 0
	va = kmalloc(size, GFP_ATOMIC | __GFP_ZERO);
	if (va)
	 *pap = (ulong)__virt_to_phys((ulong)va);
#else
	va = dma_alloc_coherent(osh->pdev, size, &pap_lin, GFP_ATOMIC | __GFP_ZERO);
	*pap = (dmaaddr_t)pap_lin;
#endif
	return va;
}
EXPORT_SYMBOL(osl_dma_alloc_consistent);

void
osl_dma_free_consistent(osl_t *osh, void *va, uint size, dmaaddr_t pa)
{
ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	 
#if 0
	kfree(va);
#else
	dma_free_coherent(osh->pdev, size, va, pa);
#endif
}
EXPORT_SYMBOL(osl_dma_free_consistent);

void BCMFASTPATH
osl_dma_flush(osl_t *osh, void *va, uint size, int direction, void *p, hnddma_seg_map_t *dmah)
{
	if (direction == DMA_TX) { /* to device */
		osl_cache_flush(va, size);
	}
	else {		
		printk("ERROR %s[%d]\n", __FUNCTION__, __LINE__);
		ASSERT(0);
	}
	return;
}
EXPORT_SYMBOL(osl_dma_flush);

dmaaddr_t BCMFASTPATH
osl_dma_map(osl_t *osh, void *va, uint size, int direction, void *p, hnddma_seg_map_t *dmah)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (size==0) {
		printf("va=%p size=%d\n", va, size);
		return virt_to_phys(va); /* just return the pa, size should not be zero */
	}

#if 0
//#if (__LINUX_ARM_ARCH__ == 8)
	/* need to flush or invalidate the cache here */
	if (direction == DMA_TX) { /* to device */
		osl_cache_flush(va, size);
	} else /*dir == DMA_RX*/ { /* from device */
		osl_cache_inv(va, size);
	} /* bidirectional not valid value for "dir" in this function */

	return virt_to_phys(va);
#else /* (__LINUX_ARM_ARCH__ == 8) */
	return dma_map_single(osh->pdev, va, size, direction);
#endif /* (__LINUX_ARM_ARCH__ == 8) */
}
EXPORT_SYMBOL(osl_dma_map);

void BCMFASTPATH
osl_dma_unmap(osl_t *osh, dmaaddr_t pa, uint size, int direction)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

#if 0
//#if (__LINUX_ARM_ARCH__ == 8)
	if (dir == DMA_TX) { /* to device */
		dma_sync_single_for_device(OSH_NULL, pa, size, DMA_TX);
	} else /*dir == DMA_RX*/ { /* from device */
		dma_sync_single_for_cpu(OSH_NULL, pa, size, DMA_RX);
	} /* bidirectional not valid value for "dir" in this function*/

#else /* (__LINUX_ARM_ARCH__ == 8) */
	dma_unmap_single(osh->pdev, (uintptr)pa, size, direction);
#endif /* (__LINUX_ARM_ARCH__ == 8) */

}
EXPORT_SYMBOL(osl_dma_unmap);

void *
osl_virt_to_phys(void *va)
{
	return (void *)(uintptr)virt_to_phys(va);
}
EXPORT_SYMBOL(osl_virt_to_phys);

inline int BCMFASTPATH
osl_arch_is_coherent(void)
{
	return 0;
}
EXPORT_SYMBOL(osl_arch_is_coherent);

inline int BCMFASTPATH
osl_acp_war_enab(void)
{
	return 0;
}
EXPORT_SYMBOL(osl_acp_war_enab);

inline void BCMFASTPATH
osl_cache_flush(void *va, uint size)
{
	if (size > 0) {
		dma_sync_single_for_device(OSH_NULL, virt_to_phys(va), size, DMA_TX);
	}
}
EXPORT_SYMBOL(osl_cache_flush);

inline void BCMFASTPATH
osl_cache_inv(void *va, uint size)
{
	dma_sync_single_for_cpu(OSH_NULL, virt_to_phys(va), size, DMA_RX);
}
EXPORT_SYMBOL(osl_cache_inv);

inline void BCMFASTPATH
osl_prefetch(const void *ptr)
{
}
EXPORT_SYMBOL(osl_prefetch);
