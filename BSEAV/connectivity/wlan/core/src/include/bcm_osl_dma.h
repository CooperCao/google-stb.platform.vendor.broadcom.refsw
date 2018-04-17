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
#ifndef _linux_osl_dma_h_
#define _linux_osl_dma_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <typedefs.h>
#include <linuxver.h>

#ifndef ISALIGNED
#define	ISALIGNED(a, x)		(((uintptr)(a) & ((x) - 1)) == 0)
#endif /* ISALIGNED */

/* allocate/free shared (dma-able) consistent memory */
#define	DMA_CONSISTENT_ALIGN	osl_dma_consistent_align()
#define	DMA_ALLOC_CONSISTENT(osh, size, align, tot, pap, dmah) \
		 osl_dma_alloc_consistent((osh), (size), (align), (tot), (pap))
#define	DMA_FREE_CONSISTENT(osh, va, size, pa, dmah) \
		 osl_dma_free_consistent((osh), (void*)(va), (size), (pa))
	 
#define	DMA_ALLOC_CONSISTENT_FORCE32(osh, size, align, tot, pap, dmah) \
		 osl_dma_alloc_consistent((osh), (size), (align), (tot), (pap))
#define	DMA_FREE_CONSISTENT_FORCE32(osh, va, size, pa, dmah) \
		 osl_dma_free_consistent((osh), (void*)(va), (size), (pa))
	 
extern uint osl_dma_consistent_align(void);
extern void *osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align, 
	uint *tot, dmaaddr_t *pap);
extern void osl_dma_free_consistent(osl_t *osh, void *va, uint size, dmaaddr_t pa);
	 
/* map/unmap direction */
#define DMA_NO	0	/* Used to skip cache op */
#define	DMA_TX	1	/* TX direction for DMA */
#define	DMA_RX	2	/* RX direction for DMA */
	 
/* map/unmap shared (dma-able) memory */
#define	DMA_UNMAP(osh, pa, size, direction, p, dmah) \
	osl_dma_unmap((osh), (pa), (size), (direction))
extern void osl_dma_flush(osl_t *osh, void *va, uint size, int direction, void *p,
	hnddma_seg_map_t *txp_dmah);
extern dmaaddr_t osl_dma_map(osl_t *osh, void *va, uint size, int direction, void *p,
	hnddma_seg_map_t *txp_dmah);
extern void osl_dma_unmap(osl_t *osh, dmaaddr_t pa, uint size, int direction);
	 
#define	PHYS_TO_VIRT(pa)	osl_phys_to_virt(pa)
#define	VIRT_TO_PHYS(va)	osl_virt_to_phys(va)
extern void * osl_phys_to_virt(void * pa);
extern void * osl_virt_to_phys(void * va);
	 
/* API for DMA addressing capability */
#define OSL_DMADDRWIDTH(osh, addrwidth) ({BCM_REFERENCE(osh); BCM_REFERENCE(addrwidth);})

#define	DMA_FLUSH(osh, va, size, direction, p, dmah) \
	osl_dma_flush((osh), (va), (size), (direction), (p), (dmah))
#define	DMA_MAP(osh, va, size, direction, p, dmah) \
	osl_dma_map((osh), (va), (size), (direction), (p), (dmah))

extern void osl_cache_flush(void *va, uint size);
extern void osl_cache_inv(void *va, uint size);
extern void osl_prefetch(const void *ptr);
#define OSL_CACHE_FLUSH(va, len)	osl_cache_flush((void *)(va), len)
#define OSL_CACHE_INV(va, len)		osl_cache_inv((void *)(va), len)
#define OSL_PREFETCH(ptr)			osl_prefetch(ptr)
extern int osl_arch_is_coherent(void);
#define OSL_ARCH_IS_COHERENT()		osl_arch_is_coherent()
extern int osl_acp_war_enab(void);
#define OSL_ACP_WAR_ENAB()			osl_acp_war_enab()

#if defined(__ARM_ARCH_7A__) || defined(STB_SOC_WIFI)
#define ACP_WAR_ENAB() 0
#define ACP_WIN_LIMIT 1
#define arch_is_coherent() 0
#endif /* __ARM_ARCH_7A__ */

/* bcm_prefetch_32B */
static inline void bcm_prefetch_32B(const uint8 *addr, const int cachelines_32B)
{
#if (defined(BCM47XX_CA9) || (defined(STB) && defined(__arm__))) && (__LINUX_ARM_ARCH__ \
	>= 5)
	switch (cachelines_32B) {
		case 4: __asm__ __volatile__("pld\t%a0" :: "p"(addr + 96) : "cc");
		case 3: __asm__ __volatile__("pld\t%a0" :: "p"(addr + 64) : "cc");
		case 2: __asm__ __volatile__("pld\t%a0" :: "p"(addr + 32) : "cc");
		case 1: __asm__ __volatile__("pld\t%a0" :: "p"(addr +  0) : "cc");
	}
#elif defined(__mips__)
	/* Hint Pref_Load = 0 */
	switch (cachelines_32B) {
		case 4: __asm__ __volatile__("pref %0, (%1)" :: "i"(0), "r"(addr + 96));
		case 3: __asm__ __volatile__("pref %0, (%1)" :: "i"(0), "r"(addr + 64));
		case 2: __asm__ __volatile__("pref %0, (%1)" :: "i"(0), "r"(addr + 32));
		case 1: __asm__ __volatile__("pref %0, (%1)" :: "i"(0), "r"(addr +  0));
	}
#endif /* BCM47XX_CA9, __mips__ */
}

/* uncached/cached virtual address */
#define OSL_UNCACHED(va)	((void *)va)
#define OSL_CACHED(va)		((void *)va)

#define OSL_PREF_RANGE_LD(va, sz) BCM_REFERENCE(va)
#define OSL_PREF_RANGE_ST(va, sz) BCM_REFERENCE(va)

#define DECLSPEC_ALIGN(x)	__attribute__ ((aligned(x)))

#define OSL_ACP_COHERENCE		(1<<1L)
#define OSL_FWDERBUF			(1<<2L)

/* map/unmap physical to virtual I/O */
#define	REG_MAP(pa, size)	ioremap_nocache((unsigned long)(pa), (unsigned long)(size))
#define	REG_UNMAP(va)		iounmap((va))

#ifdef __cplusplus
}
#endif

#endif	/* _linux_osl_dma_h_ */
