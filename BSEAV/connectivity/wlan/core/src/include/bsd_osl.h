/*
 * NetBSD  2.0 OS Independent Layer
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef _bsd_osl_h_
#define _bsd_osl_h_

#include <typedefs.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/malloc.h>
#include <sys/systm.h>
#ifdef __HAVE_NEW_STYLE_BUS_H
#include <sys/bus.h>
#else
#include <machine/bus.h>
#endif
#include <sys/resource.h>
#include <machine/param.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>
#include <sys/mbuf.h>

/* libkern.h included from systm.h defines memcpy and memcmp causing problems */

#ifdef WLC_NET80211
#define WPA_SUITE_LEN 4
#endif

/* The magic cookie */
#define OS_HANDLE_MAGIC		0x1234abcd /* Magic number  for osl_t */

/* Assert */
#ifdef BCMDBG_ASSERT
#define ASSERT(exp) \
	do { if (!(exp)) osl_assert(#exp, __FILE__, __LINE__); } while (0)
extern void osl_assert(const char *exp, const char *file, int line);
#else
#define	ASSERT(exp)		do {} while (0)
#endif /* BCMDBG_ASSERT */

#ifdef __mips__
#define OSL_PREF_RANGE_LD(va, sz) prefetch_range_PREF_LOAD_RETAINED(va, sz)
#define OSL_PREF_RANGE_ST(va, sz) prefetch_range_PREF_STORE_RETAINED(va, sz)
#else /* __mips__ */
#define OSL_PREF_RANGE_LD(va, sz) BCM_REFERENCE(va)
#define OSL_PREF_RANGE_ST(va, sz) BCM_REFERENCE(va)
#endif /* __mips__ */

/* PCI configuration space access macros */
#define	OSL_PCI_READ_CONFIG(osh, offset, size) \
	osl_pci_read_config((osh), (offset), (size))
#define	OSL_PCI_WRITE_CONFIG(osh, offset, size, val) \
	osl_pci_write_config((osh), (offset), (size), (val))
extern uint32 osl_pci_read_config(osl_t *osh, uint size, uint offset);
extern void osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val);

/* PCMCIA attribute space access macros, not suppotred */
#define	OSL_PCMCIA_READ_ATTR(osh, offset, buf, size) 	\
	({ \
	 BCM_REFERENCE(osh); \
	 BCM_REFERENCE(buf); \
	 ASSERT(0); \
	 })
#define	OSL_PCMCIA_WRITE_ATTR(osh, offset, buf, size) 	\
	({ \
	 BCM_REFERENCE(osh); \
	 BCM_REFERENCE(buf); \
	 ASSERT(0); \
	 })

enum {
	OSL_PCI_BUS,
	OSL_CARDBUS_BUS
};

/* OSL initialization */
extern osl_t *osl_attach(void *pdev, bool pkttag, const char *dev_name,
	int bustype, bus_dma_tag_t dmat,
	bus_space_tag_t space, bus_space_handle_t handle);
extern void osl_detach(osl_t *osh);

extern void osl_pktfree_cb_set(osl_t *osh, pktfree_cb_fn_t tx_fn, void *tx_ctx);
#define PKTFREESETCB(osh, tx_fn, tx_ctx) osl_pktfree_cb_set(osh, tx_fn, tx_ctx)

/* Host/bus architecture-specific byte swap */
#if !defined(IL_BIGENDIAN) || defined(__mips__)
#define BUS_SWAP32(v)		(v)
#else
#define BUS_SWAP32(v)		htol32(v)
#endif

#ifndef bsd_osl_c
/* Undefine the generic BSD kernel MALLOC and MFREE macros to avoid clash
 *
 * Do this only if we are not in bsd_osl.c itself.
 */
#undef MALLOC
#undef MFREE

#ifdef BCMDBG_MEM
#define	MALLOC(osh, size)	osl_debug_malloc((osh), (size), __LINE__, __FILE__)
#define	MALLOCZ(osh, size)	osl_debug_mallocz((osh), (size), __LINE__, __FILE__)
#define	MFREE(osh, addr, size)	osl_debug_mfree((osh), (addr), (size), __LINE__, __FILE__)
#define MALLOCED(osh)		osl_malloced((osh))
#define	MALLOC_DUMP(osh, b) osl_debug_memdump((osh), (b))

#else /* BCMDBG_MEM */

#define	MALLOC(osh, size)	osl_malloc((osh), (size))
#define	MALLOCZ(osh, size)	osl_mallocz((osh), (size))
#define	MFREE(osh, addr, size)	osl_mfree((osh), (addr), (size))
#define MALLOCED(osh)		osl_malloced((osh))

#endif	/* BCMDBG_MEM */
#endif /* bsd_osl_c */

#define	MALLOC_FAILED(osh)	osl_malloc_failed((osh))

extern void *osl_debug_malloc(osl_t *osh, uint size, int line, const char* file);
extern void *osl_debug_mallocz(osl_t *osh, uint size, int line, const char* file);
extern void osl_debug_mfree(osl_t *osh, void *addr, uint size, int line, const char* file);
struct bcmstrbuf;
extern int osl_debug_memdump(osl_t *osh, struct bcmstrbuf *b);
extern void *osl_malloc(osl_t *osh, uint size);
extern void *osl_mallocz(osl_t *osh, uint size);
extern void osl_mfree(osl_t *osh, void *addr, uint size);
extern uint osl_malloced(osl_t *osh);
extern uint osl_malloc_failed(osl_t *osh);

/* Allocate/free shared (dma-able) consistent memory */

#define	DMA_CONSISTENT_ALIGN	PAGE_SIZE

#define	DMA_ALLOC_CONSISTENT(osh, size, align, tot, pap, dmah) \
	osl_dma_alloc_consistent((osh), (size), (align), (tot), (pap), (dmah))
#define	DMA_FREE_CONSISTENT(osh, va, size, pa, dmah) \
	osl_dma_free_consistent((osh), (void*)(va), (size), (pa), (dmah))

extern void *osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align_bits, uint *alloced,
	ulong *pap, osldma_t **dmah);
extern void osl_dma_free_consistent(osl_t *osh, void *va, uint size, ulong pa, osldma_t *dmah);
extern void osl_dma_addrwidth(osl_t *osh, uint width);

/* Map/unmap direction */
#define	DMA_TX	1 	/* DMA TX flag */
#define	DMA_RX	2	/* DMA RX flag */

/* Map/unmap shared (dma-able) memory */

#define	DMA_MAP(osh, va, size, direction, p, dmah) \
	osl_dma_map((osh), (va), (size), (direction), (p), (dmah))
#define	DMA_UNMAP(osh, pa, size, direction, p, dmah) \
	osl_dma_unmap((osh), (pa), (size), (direction), (dmah))

extern uint osl_dma_map(osl_t *osh, void *va, uint size, int direction,
                        struct mbuf *mbuf, hnddma_seg_map_t *segmap);
extern void osl_dma_unmap(osl_t *osh, uint pa, uint size, int direction,
                          hnddma_seg_map_t *segmap);
extern void osl_dmamap_put(osl_t *osh, osldma_t *map);

/* API for DMA addressing capability */
#define OSL_DMADDRWIDTH(osh, width)	osl_dma_addrwidth(osh, width)

/* map/unmap physical to virtual, not supported */

/* map/unmap physical to virtual */
#ifdef __mips__
/* assume a 1:1 mapping if KSEG2 addresses are used */
#if __NetBSD_Version__ >= 500000003
#define	REG_MAP(pa, size)	osl_reg_map((pa), (size))
#define	REG_UNMAP(va)		osl_reg_unmap((va))
extern void *osl_reg_map(uint32 pa, uint size);
extern void osl_reg_unmap(void *va);
#else
#define REG_MAP(pa, size)       ((void *)(IS_KSEG2(pa) ? (void *)(pa) : OSL_UNCACHED(pa)))
#define REG_UNMAP(va)           BCM_REFERENCE(va)
#endif /* _NetBSD_Version__ >= 500000003 */
#else
#define	REG_MAP(pa, size)	({BCM_REFERENCE(pa); ((void *)NULL);})
#define	REG_UNMAP(va)		({BCM_REFERENCE(va); ASSERT(0);})
#endif /* __mips__ */

/* NetBSD needs 2 handles the bus_space_tag at attach time
 * and the bus_space_handle
 */
/* Pkttag flag should be part of public information */
struct osl_pubinfo {
	bus_space_tag_t 	space;
	bus_space_handle_t	handle;
	uint8 *vaddr;
};

#define OSL_PUB(osh) ((struct osl_pubinfo *)(osh))

/* IO bus mapping routines */
#ifdef IL_BIGENDIAN
#define rreg32(osh, r)	(\
	(osh)? bus_space_read_4(OSL_PUB(osh)->space, OSL_PUB(osh)->handle, \
		(bus_size_t)(((uintptr)(r)) - \
		((uintptr)(OSL_PUB(osh)->vaddr)))):\
		(*(volatile uint32 *)(r)))
#define rreg16(osh, r)	(\
	(osh)? bus_space_read_2(OSL_PUB(osh)->space, OSL_PUB(osh)->handle, \
		(bus_size_t)(((uintptr)(r)) - \
		((uintptr)(OSL_PUB(osh)->vaddr)))):\
		(*(volatile uint16 *)((uint32)(r)^2)))
#define rreg8(osh, r)	(\
	(osh)? bus_space_read_1(OSL_PUB(osh)->space, OSL_PUB(osh)->handle, \
		(bus_size_t)(((uintptr)(r)) - \
		((uintptr)(OSL_PUB(osh)->vaddr)))):\
		(*(volatile uint8 *)((uint32)(r)^3)))

#define wreg32(osh, r, v) do { \
	if (osh) \
		bus_space_write_4(OSL_PUB(osh)->space, OSL_PUB(osh)->handle, \
		  (bus_size_t)(((uintptr)(r)) - \
			((uintptr)(OSL_PUB(osh)->vaddr))), (uint32)(v)); \
	else \
		(*(volatile uint32 *)(r) = (uint32)(v)); \
} while (0)
#define wreg16(osh, r, v) do { \
	if (osh) \
		bus_space_write_2(OSL_PUB(osh)->space, OSL_PUB(osh)->handle, \
		  (bus_size_t)(((uintptr)(r)) - \
			((uintptr)(OSL_PUB(osh)->vaddr))), (uint16)(v)); \
	else \
		(*(volatile uint16 *)((uint32)(r)^2) = (uint16)(v)); \
} while (0)
#define wreg8(osh, r, v) do { \
	if (osh) \
		bus_space_write_1(OSL_PUB(osh)->space, OSL_PUB(osh)->handle, \
		  (bus_size_t)(((uintptr)(r)) - \
			((uintptr)(OSL_PUB(osh)->vaddr))), (uint8)(v)); \
	else \
		(*(volatile uint8 *)((uint32)(r)^3) = (uint8)(v)); \
} while (0)
#else	/* !IL_BIGENDIAN */
#define rreg32(osh, r)	(\
	(osh)? bus_space_read_4(OSL_PUB(osh)->space, OSL_PUB(osh)->handle, \
		(bus_size_t)(((uintptr)(r)) - \
		((uintptr)(OSL_PUB(osh)->vaddr)))):\
		(*(volatile uint32 *)(r)))
#define rreg16(osh, r)	(\
	(osh)? bus_space_read_2(OSL_PUB(osh)->space, OSL_PUB(osh)->handle, \
		(bus_size_t)(((uintptr)(r)) - \
		((uintptr)(OSL_PUB(osh)->vaddr)))):\
		(*(volatile uint16 *)(r)))
#define rreg8(osh, r)	(\
	(osh)? bus_space_read_1(OSL_PUB(osh)->space, OSL_PUB(osh)->handle, \
		(bus_size_t)(((uintptr)(r)) - \
		((uintptr)(OSL_PUB(osh)->vaddr)))):\
		(*(volatile uint8 *)(r)))

#define wreg32(osh, r, v) do { \
	if (osh) \
		bus_space_write_4(OSL_PUB(osh)->space, OSL_PUB(osh)->handle, \
		  (bus_size_t)((uintptr)(r) - \
			((uintptr)(OSL_PUB(osh)->vaddr))), (uint32)(v)); \
	else \
		(*(volatile uint32 *)(r) = (uint32)(v)); \
} while (0)
#define wreg16(osh, r, v) do { \
	if (osh) \
		bus_space_write_2(OSL_PUB(osh)->space, OSL_PUB(osh)->handle, \
		  (bus_size_t)((uintptr)(r) - \
			((uintptr)(OSL_PUB(osh)->vaddr))), (uint16)(v)); \
	else \
		(*(volatile uint16 *)(r) = (uint16)(v)); \
} while (0)
#define wreg8(osh, r, v) do { \
	if (osh) \
		bus_space_write_1(OSL_PUB(osh)->space, OSL_PUB(osh)->handle, \
		  (bus_size_t)((uintptr)(r) - \
			((uintptr)(OSL_PUB(osh)->vaddr))), (uint8)(v)); \
	else \
		(*(volatile uint8 *)(r) = (uint8)(v)); \
} while (0)
#endif /* IL_BIGENDIAN */

/* register access macros */
#ifdef mips
#define MIPS_SYNC __asm__ __volatile__("sync")
#else
#define MIPS_SYNC
#endif

#define R_REG(osh, r)	\
		({ \
			__typeof(*(r)) __osl_v; \
			MIPS_SYNC; \
			switch (sizeof(*(r))) { \
				case sizeof(uint8):	__osl_v = \
					rreg8((osh), (r)); break; \
				case sizeof(uint16):	__osl_v = \
					rreg16((osh), (r)); break; \
				case sizeof(uint32):	__osl_v = \
					rreg32((osh), (r)); break; \
			} \
			MIPS_SYNC; \
			__osl_v; \
		})

#define W_REG(osh, r, v)     do {\
			if (sizeof *(r) == sizeof (uint32)) \
				wreg32((osh), (r), (v)); \
			else if (sizeof *(r) == sizeof (uint16))\
				wreg16((osh), (r), (v)); \
			else \
				wreg8((osh), (r), (v)); \
			} while (0)


#define	AND_REG(osh, r, v)	W_REG(osh, (r), R_REG(osh, r) & (v))
#define	OR_REG(osh, r, v)	W_REG(osh, (r), R_REG(osh, r) | (v))

/* Shared memory access macros */
#define	R_SM(a)			*(a)
#define	W_SM(a, v)		(*(a) = (v))
#define	BZERO_SM(a, len)	bzero((char*)a, len)

/* Uncached/cached virtual address */
#ifdef __mips__
#define IS_KSEG2(_x)            (((_x) & MIPS_KSEG2_START) == MIPS_KSEG2_START)
#define OSL_UNCACHED(va)        ((void*)MIPS_PHYS_TO_KSEG1((va)))
#define OSL_CACHED(va)	        ((void*)MIPS_PHYS_TO_KSEG0((va)))
#include <cpuregs.h>
#else
#define OSL_UNCACHED(va)	(va)
#define OSL_CACHED(va)		({BCM_REFERENCE(va); ASSERT(0);})
#endif

/* Get processor cycle count */
#ifdef __i386__
#define	OSL_GETCYCLES(x)	__asm__ __volatile__("rdtsc" : "=a" (x) : : "edx")
#else
#define OSL_GETCYCLES(x)	x = (uint)mono_time
#endif /* #ifdef __i386__ */

/* dereference an address that may target abort */
#define	BUSPROBE(val, addr)	osl_busprobe(&(val), (uint32) (addr))
extern int osl_busprobe(uint32 *val, uint32 addr);

/* Microsecond delay */
#define	OSL_DELAY(usec)		DELAY((usec))

/* Native mbuf packet tag ids */

#define PACKET_TAG_BRCM			PCI_VENDOR_BROADCOM
#define PACKET_TAG_BRCM_PKTPRIO		PACKET_TAG_BRCM + 1
#define PACKET_TAG_BRCM_CTX		PACKET_TAG_BRCM + 2

static INLINE void *
osl_pktlink(void *m)
{
	ASSERT(((struct mbuf *)(m))->m_flags & M_PKTHDR);
	return ((struct mbuf *)(m))->m_nextpkt;
}

static INLINE void
osl_pktsetlink(void *m, void *x)
{
	ASSERT(((struct mbuf *)(m))->m_flags & M_PKTHDR);
	((struct mbuf *)(m))->m_nextpkt = (struct mbuf *)(x);
}

static INLINE void *
osl_pkttag(void *m)
{
	struct m_tag *tag;

	ASSERT(((struct mbuf *)(m))->m_flags & M_PKTHDR);
	tag = m_tag_find(m, PACKET_TAG_BRCM_CTX, NULL);
	if (tag)
		return (tag +1);
	else
		return (NULL);

}

#define OSH_NULL   NULL

/* Packet primitives */
#define	PKTGET(osh, len, send)	osl_pktget((osh), (len), (send))
#define	PKTFREE(osh, m, send)	osl_pktfree((osh), (m), (send))
#define	PKTDATA(osh, m)		({BCM_REFERENCE(osh); ((struct mbuf *)(m))->m_data;})
#define	PKTDUP(osh, m)		osl_pktdup((osh), (m))
#define PKTHEADROOM(osh, m)	({BCM_REFERENCE(osh); M_LEADINGSPACE((struct mbuf *)(m));})
#define PKTTAILROOM(osh, m)	({BCM_REFERENCE(osh); M_TRAILINGSPACE((struct mbuf *)(m));})
#define	PKTNEXT(osh, m)		({BCM_REFERENCE(osh); ((struct mbuf *)(m))->m_next;})
#define	PKTSETNEXT(osh, m, x)	osl_pktsetnext((osh), (m), (x))
#define	PKTLEN(osh, m)		({BCM_REFERENCE(osh); ((struct mbuf *)((m)))->m_len;})
#define	PKTSETLEN(osh, m, len)	({BCM_REFERENCE(osh); ((struct mbuf *)((m)))->m_len = (len);})
#define	PKTPUSH(osh, m, bytes)	osl_pktpush((osh), (m), (bytes))
#define	PKTPULL(osh, m, bytes)	osl_pktpull((osh), (m), (bytes))
#define	PKTTAG(m)		osl_pkttag((m))
#define	PKTLINK(m)		osl_pktlink((m))
#define	PKTSETLINK(m, x)	osl_pktsetlink((m), (x))
#define PKTFRMNATIVE(osh, m)	osl_pkt_frmnative((osh), (struct mbuf *)(m))
#define PKTTONATIVE(osh, p)	osl_pkt_tonative((osh), (p))
#if __NetBSD_Version__ >= 500000003
#define PKTSHARED(p)            M_READONLY((struct mbuf *)(p))
#else
#define PKTSHARED(p)            MCLISREFERENCED((struct mbuf *)(p))
#endif /* __NetBSD_Version__ >= 500000003 */
#define PKTALLOCED(osh)		({BCM_REFERENCE(osh); 0;})
#define PKTSETPOOL(osh, m, x, y)	BCM_REFERENCE(osh)
#define PKTPOOL(osh, m)			({BCM_REFERENCE(osh); FALSE;})
#define PKTFREELIST(m)          PKTLINK(m)
#define PKTSETFREELIST(m, x)    PKTSETLINK((m), (x))
#define PKTPTR(m)               (m)
#define PKTID(m)                ({BCM_REFERENCE(m); 0;})
#define PKTSETID(m, id)         ({BCM_REFERENCE(m); BCM_REFERENCE(id);})
#ifdef BCMDBG_PKT
#define PKTLIST_DUMP(osh, buf)          ({BCM_REFERENCE(osh); ((void)buf);})
#else /* BCMDBG_PKT */
#define PKTLIST_DUMP(osh, buf)		BCM_REFERENCE(osh)
#endif /* BCMDBG_PKT */

#ifdef PKTC
#define	CHAINED	(1 << 0)
#define	PKTCCLRFLAGS(m) (((struct mbuf *)m)->m_pkthdr.csum_flags = 0)
#define	PKTCFLAGS(m)	(((struct mbuf *)m)->m_pkthdr.csum_flags)
#define	PKTCCNT(m)	(0)
#define	PKTCLEN(m)	(0)
#define	PKTCSETCNT(m, c)
#define	PKTCINCRCNT(m)
#define	PKTCADDCNT(m, c)
#define	PKTCSETLEN(m, l)
#define	PKTCADDLEN(m, l)
#define	PKTCSETFLAG(m, fb)		(((struct mbuf *)m)->m_pkthdr.csum_flags |= (fb))
#define	PKTCCLRFLAG(m, fb)		(((struct mbuf *)m)->m_pkthdr.csum_flags &= ~(fb))
#define	PKTCLINK(m)				M_GETCTX((struct mbuf *)m, void *)
#define	PKTSETCLINK(m, x)		M_SETCTX((struct mbuf *)m, x)
#define PKTSETCHAINED(osh, m) (((struct mbuf *)m)->m_pkthdr.csum_flags |= CHAINED)
#define PKTCLRCHAINED(osh, m) (((struct mbuf *)m)->m_pkthdr.csum_flags &= ~CHAINED)
#define PKTISCHAINED(m)	(((struct mbuf *)m)->m_pkthdr.csum_flags & CHAINED)
#define FOREACH_CHAINED_PKT(skb, nskb) \
	for (; (skb) != NULL; (skb) = (nskb)) \
		if ((nskb) = (PKTISCHAINED(skb) ? PKTCLINK(skb) : NULL), \
		    PKTSETCLINK((skb), NULL), 1)
#define	PKTCFREE(osh, skb, send) \
do { \
	void *nskb; \
	ASSERT((skb) != NULL); \
	FOREACH_CHAINED_PKT((skb), nskb) { \
		PKTCLRCHAINED((osh), (skb)); \
		PKTCCLRFLAGS((skb)); \
		PKTFREE((osh), (skb), (send)); \
	} \
} while (0)

#define PKTCENQTAIL(h, t, p) \
do { \
	if ((t) == NULL) { \
		(h) = (t) = (p); \
	} else { \
		PKTSETCLINK((t), (p)); \
		(t) = (p); \
	} \
} while (0)
#endif /* PKTC */


static INLINE uint
osl_pktprio(void *mbuf)
{
	/* if MBUFTRACE is enabled use
	 * packet tags.  if its not use the m_owner struct to reduce cpu
	 * utilization
	 */
#ifdef MBUFTRACE
	struct m_tag	*mtag;
	struct mbuf 	*m  = (struct mbuf *)mbuf;

	ASSERT(m);
	ASSERT(m->m_flags & M_PKTHDR);

	mtag = m_tag_find(m, PACKET_TAG_BRCM_PKTPRIO, NULL);

	if (mtag == NULL)
		return 0;
	else
		return *(uint *)(mtag + 1);
#else
	struct mbuf 	*m  = (struct mbuf *)mbuf;
	return (uint)(m->m_owner);
#endif /* MBUFTRACE */
}

/* Packet priority tag added if it did not exist before
 * m_tags will survive movement across interfaces.
 * Removed as part of mbuf free operation
 */
static INLINE void
osl_pktsetprio(void *mbuf, uint x)
{
	/* if MBUFTRACE is enabled use
	 * packet tags.  if its not use the m_owner struct to reduce cpu
	 * utilization
	 */
#ifdef MBUFTRACE
	struct m_tag	*mtag;
	struct mbuf 	*m = (struct mbuf *)mbuf;


	ASSERT(m);
	ASSERT(m->m_flags & M_PKTHDR);

	/* Look for tag , if not present create it */

	mtag = m_tag_find(m, PACKET_TAG_BRCM_PKTPRIO, NULL);

	if (mtag == NULL) {
		mtag = m_tag_get(PACKET_TAG_BRCM_PKTPRIO,
			sizeof(uint), M_NOWAIT);
		if (mtag == NULL)
			return;
		/* Attach m_tag to mbuf */
		m_tag_prepend(m, mtag);
	}

	*(uint *)(mtag + 1) = x;
#else
	((struct mbuf *)mbuf)->m_owner = (struct mowner *)x;
#endif /* MBUFTRACE */
}

#define	PKTPRIO(m)		osl_pktprio((m))
#define	PKTSETPRIO(m, x)	osl_pktsetprio((m), (x))

/* OSL packet primitive functions  */
extern void *osl_pktget(osl_t *osh, uint len, bool send);
extern uchar *osl_pktpush(osl_t *osh, void *m, int bytes);
extern uchar *osl_pktpull(osl_t *osh, void *m, int bytes);
extern struct mbuf *osl_pkt_tonative(osl_t *osh, void *p);
extern void *osl_pkt_frmnative(osl_t *osh, struct mbuf *m);
extern void osl_pktfree(osl_t *osh, void *m, bool send);
extern void osl_pktsetlink(void *m, void *x);
extern void *osl_pktdup(osl_t *osh, void *m);
extern void osl_pktsetnext(osl_t *osh, void *m, void *x);

/* the largest reasonable packet buffer driver uses for ethernet MTU in bytes */
#define	PKTBUFSZ	MCLBYTES /* packet size */

/* PCI device bus # and slot # */
#define OSL_PCI_BUS(osh)	osl_pci_bus(osh)
#define OSL_PCI_SLOT(osh)	osl_pci_slot(osh)
extern uint osl_pci_bus(osl_t *osh);
extern uint osl_pci_slot(osl_t *osh);
#define OSL_PCIE_DOMAIN(osh)	({BCM_REFERENCE(osh); 0;})
#define OSL_PCIE_BUS(osh)	({BCM_REFERENCE(osh); 0;})

/* Translate bcmerrors into NetBSD errors */
#define OSL_ERROR(bcmerror)	osl_error(bcmerror)
extern int osl_error(int bcmerror);

/* Global ASSERT type */
extern uint32 g_assert_type;

/* get system up time in miliseconds */
#define OSL_SYSUPTIME()                 osl_sysuptime()
extern uint32 osl_sysuptime(void);

#if __NetBSD_Version__ >= 500000003
#define softintr_schedule(a) softint_schedule(a)
#define softintr_establish(a, b, c) softint_establish(a, b, c)
#define softintr_disestablish(a) softint_disestablish(a)
#endif /* __NetBSD_Version__ >= 500000003 */

#define OSL_SMP_WMB()				do { /* noop */ } while (0)

#endif	/* _bsd_osl_h_ */
