/*
 * NDIS OS Independent Layer
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

#ifndef _ndis_osl_h_
#define _ndis_osl_h_

#include <typedefs.h>

#if defined(NDIS50) || defined(NDIS51)
#include <ndshared5.h>
#else
#include <ndshared.h>
#endif /* ( defined (NDIS50) || defined (NDIS51) ) */

#define DECLSPEC_ALIGN(x) __declspec(align(x))

/* make WHQL happy */
#ifndef MEMORY_TAG
#define MEMORY_TAG 'MCRB'	/* BRCM reversed */
#endif /* MEMORY_TAG */
#if (0>= 0x0600)
#define TIMER_TAG 'RMTB'	/* timer tag */
#endif 
#ifdef WDM
#define NdisAllocateMemory(__va, __len, __dummy1, __dummy2) \
	NdisAllocateMemoryWithTag(__va, __len, (ULONG) MEMORY_TAG)
#endif /* WDM */


#ifdef _PREFAST_
#define ASSERT(exp)	__assume(exp)
/* an assert with descriptive text used instead of #exp */
#define ASSERT_DESCR(exp, descr) __assume(exp)
#else /* _PREFAST_ */
#if defined(BCMDBG_ASSERT) || defined(BCMASSERT_LOG)
#undef ASSERT
#define ASSERT(exp)     if (exp); else osl_assert(#exp, __FILE__, __LINE__)
/* an assert with descriptive text used instead of #exp */
#undef ASSERT_DESCR
#define ASSERT_DESCR(exp, descr) \
	if (exp); else osl_assert((char*)descr, __FILE__, __LINE__)

extern void osl_assert(char *exp, char *file, int line);
#else
#undef ASSERT
#define	ASSERT(exp)
#undef ASSERT_DESCR
#define ASSERT_DESCR(exp, descr)
#endif /* BCMASSERT_LOG || BCMASSERT_LOG */
#endif /* _PREFAST_ */

#define	DMA_TX	1	/* TX direction for DMA */
#define	DMA_RX	2	/* RX direction for DMA */

#if !defined(COOKIE_SHARED)
#define COOKIE_SHARED(cookie)		(cookie)
#endif /* !defined (COOKIE_SHARED) */


#define OSL_PCMCIA_READ_ATTR(osh, offset, buf, size) osl_pcmcia_read_attr((osh), (offset), (buf), \
				(size))
#define OSL_PCMCIA_WRITE_ATTR(osh, offset, buf, size) osl_pcmcia_write_attr((osh), (offset), \
				(buf), (size))
extern void osl_pcmcia_read_attr(osl_t *osh, uint offset, void *buf, int size);
extern void osl_pcmcia_write_attr(osl_t *osh, uint offset, void *buf, int size);

#define	OSL_PCI_READ_CONFIG(osh, offset, size)		osl_pci_read_config((osh), (offset), (size))
#define OSL_PCI_WRITE_CONFIG(osh, offset, size, val) osl_pci_write_config((osh), (offset), (size), \
				(val))
extern uint32 osl_pci_read_config(osl_t *osh, uint size, uint offset);
extern void osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val);

#define OSL_PCI_BUS(osh)	(0)
#define OSL_PCI_SLOT(osh)	(0)
#define OSL_PCIE_DOMAIN(osh)	(0)
#define OSL_PCIE_BUS(osh)	(0)

#define	OSL_UNCACHED(va)	(va)
#define	OSL_CACHED(va)		(va)

#define OSL_PREF_RANGE_LD(va, sz)
#define OSL_PREF_RANGE_ST(va, sz)

/* won't need this until we boot NDIS on one of our chips */
#define	REG_MAP(pa, size)	NULL			/* not supported */
#define	REG_UNMAP(va)		0			/* not supported */


extern int sprintf(char *buf, const char *fmt, ...);
#define	printf	 if (KeGetCurrentIrql() <= DISPATCH_LEVEL) DbgPrint

/* pick up osl required snprintf/vsnprintf */
#include <bcmstdlib.h>

#if !defined(WLSIM)

extern void osl_regops_set(osl_t *osh, osl_rreg_fn_t rreg, osl_wreg_fn_t wreg, void *ctx);
extern uint8 osl_readb(osl_t *osh, volatile uint8 *r);
extern uint16 osl_readw(osl_t *osh, volatile uint16 *r);
extern uint32 osl_readl(osl_t *osh, volatile uint32 *r);
extern void osl_writeb(osl_t *osh, volatile uint8 *r, uint8 v);
extern void osl_writew(osl_t *osh, volatile uint16 *r, uint16 v);
extern void osl_writel(osl_t *osh, volatile uint32 *r, uint32 v);
/* register access macros */
#if defined(OSLREGOPS)

#define REGOPSSET(osh, rreg, wreg, ctx)		osl_regops_set(osh, rreg, wreg, ctx)

#define R_REG(osh, r) (\
	sizeof(*(r)) == sizeof(uint8) ? osl_readb((osh), (volatile uint8*)(r)) : \
	sizeof(*(r)) == sizeof(uint16) ? osl_readw((osh), (volatile uint16*)(r)) : \
	osl_readl((osh), (volatile uint32*)(r)) \
)
#define W_REG(osh, r, v) do { \
	switch (sizeof(*(r))) { \
	case sizeof(uint8):	osl_writeb((osh), (volatile uint8*)(r), (uint8)(v)); break; \
	case sizeof(uint16):	osl_writew((osh), (volatile uint16*)(r), (uint16)(v)); break; \
	case sizeof(uint32):	osl_writel((osh), (volatile uint32*)(r), (uint32)(v)); break; \
	} \
} while (0)


#else /* OSLREGOPS */

static __inline uint32
readl(volatile uint32 *address)
{
	volatile uint32 v;

	NdisReadRegisterUlong((PULONG)address, (PULONG)&v);
	return (v);
}

static __inline uint16
readw(volatile uint16 *address)
{
	volatile uint16 v;

	NdisReadRegisterUshort((PUSHORT)address, (PUSHORT)&v);
	return (v);
}

static __inline uint8
readb(volatile uint8 *address)
{
	volatile uint8 v;

	NdisReadRegisterUchar((PUCHAR)address, (PUCHAR)&v);
	return (v);
}

#undef writew
#define writew(value, address) NdisWriteRegisterUshort((uint16 *)(address), (value))
#undef writel
#define writel(value, address)	NdisWriteRegisterUlong((uint32 *)(address), (value))
#undef writec
#define writeb(value, address)	NdisWriteRegisterUchar((uint8 *)(address), (value))



#if defined(BCM_BACKPLANE_TIMEOUT) && defined(BCMDBG)
/* register access macros when  BCM_BACKPLANE_TIMEOUT is enabled */
#define	R_REG(osh, r) ((sizeof *(r) == sizeof(uint32)) ? \
	(uint32)read_bpt_reg((osh), (r), (sizeof(*r))): \
	((sizeof *(r) == sizeof(uint16)) ? \
	(uint16)read_bpt_reg((osh), (r), (sizeof(*r))) : \
	(uint8)read_bpt_reg((osh), (r), (sizeof(*r)))))

#else
#define	R_REG(osh, r)	((sizeof *(r) == sizeof(uint32)) ? readl((volatile uint32*)(r)): \
			((sizeof *(r) == sizeof(uint16)) ? readw((volatile uint16*)(r)): \
			readb((volatile uint8*)(r))))

#endif /* #if defined(BCM_BACKPLANE_TIMEOUT) && defined(BCMDBG) */

#ifdef WAR10501
	\
#include <sbconfig.h>
#define	W_REG(osh, r, v)						\
	do {							\
		if (sizeof *(r) == sizeof(uint32))		\
			writel((v), (r));			\
		else if (sizeof *(r) == sizeof(uint16))   					\
			writew((uint16)(v), (r));		\
		else    					\
			writeb((uint8)(v), (r));		\
		/* has to be a valid chip core access */	\
		ASSERT((readl((uint32*)(((uint32)(r) & ~(SI_CORE_SIZE - 1)) | \
	(uint32)(&((sbconfig_t*)SBCONFIGOFF)->sbidhigh))) >> SBIDH_VC_SHIFT) == SB_VEND_BCM);	\
		writel(0, (((uint32)(r) & ~(SI_CORE_SIZE - 1)) | \
		(uint32)(&((sbconfig_t*)SBCONFIGOFF)->sbidlow)));	\
	} while (0)
#else
#ifdef WAR19310		/* pcmcia rev5: consecutive writes are corruptted on slow clock */
#define	W_REG(osh, r, v)						\
	do {							\
		if (sizeof *(r) == sizeof(uint32))		\
			writel((v), (r));			\
		else if (sizeof *(r) == sizeof(uint16))   					\
			writew((uint16)(v), (r));		\
		else    					\
			writeb((uint8)(v), (r));		\
		OSL_DELAY(2);					\
	} while (0)
#else
#define	W_REG(osh, r, v)						\
	do {							\
		if (sizeof *(r) == sizeof(uint32))		\
			writel((v), (r));			\
		else if (sizeof *(r) == sizeof(uint16))   					\
			writew((uint16)(v), (r));		\
		else    					\
			writeb((uint8)(v), (r));		\
	} while (0)
#endif /* WAR19310 */
#endif /* WAR10501 */

#endif /* OSLREGOPS */

#define	AND_REG(osh, r, v)	W_REG(osh, (r), (R_REG(osh, r) & (v)))
#define	OR_REG(osh, r, v)	W_REG(osh, (r), (R_REG(osh, r) | (v)))

/* Host/Bus architecture specific swap. Noop for little endian systems, possible swap on big endian
 */
#define BUS_SWAP32(v)	(v)

/* bcopy, bcmp, and bzero */
#define	bcopy(src, dst, len)	NdisMoveMemory((dst), (src), (len))
#define	bcmp(b1, b2, len)	(!NdisEqualMemory((b1), (b2), (len)))
#define	bzero(b, len)		NdisZeroMemory((b), (len))

#endif /* WLSIM */

/* OSL initialization */
extern osl_t *osl_attach(void *pdev, NDIS_HANDLE adapter_handle);
extern void osl_detach(osl_t *osh);

extern void osl_pktfree_cb_set(osl_t *osh, pktfree_cb_fn_t tx_fn, void *tx_ctx);
#define PKTFREESETCB(osh, tx_fn, tx_ctx) osl_pktfree_cb_set(osh, tx_fn, tx_ctx)

#ifdef BCMDBG_MEM

#define MALLOC(osh, size)       osl_debug_malloc((osh), (size), __LINE__, __FILE__)
#define MALLOCZ(osh, size)      osl_debug_mallocz((osh), (size), __LINE__, __FILE__)
#define MFREE(osh, addr, size)  osl_debug_mfree((osh), (addr), (size), __LINE__, __FILE__)
#define MALLOCED(osh)           osl_malloced((osh))
#define MALLOC_DUMP(osh, b) osl_debug_memdump((osh), (b))
extern void *osl_debug_malloc(osl_t *osh, uint size, int line, char* file);
extern void *osl_debug_mallocz(osl_t *osh, uint size, int line, char* file);
extern void osl_debug_mfree(osl_t *osh, void *addr, uint size, int line, char* file);
extern int osl_debug_memdump(osl_t *osh, struct bcmstrbuf *b);

#else /* BCMDBG_MEM */

#define	MALLOC(osh, size)	osl_malloc((osh), (size))
#define	MALLOCZ(osh, size)	osl_mallocz((osh), (size))
#define	MFREE(osh, addr, size)	osl_mfree((osh), (char*)(addr), (size))
#define	MALLOCED(osh)		osl_malloced((osh))
#define	MALLOC_DUMP(osh, b)

#endif  /* BCMDBG_MEM */

#define	VMALLOC(osh, size)	MALLOC(osh, size)
#define	VMALLOCZ(osh, size)	MALLOCZ(osh, size)
#define	VMFREE(osh, addr, size)	MFREE(osh, addr, size)

#define NATIVE_MALLOC(osh, size)	osl_malloc_native((osh), (size))
#define NATIVE_MFREE(osh, addr, size)	osl_mfree_native((osh), (char*)(addr), (size))


#define MALLOC_FAILED(osh)	osl_malloc_failed((osh))

extern void *osl_malloc(osl_t *osh, uint size);
extern void *osl_mallocz(osl_t *osh, uint size);
extern void osl_mfree(osl_t *osh, char *addr, uint size);
extern uint osl_malloced(osl_t *osh);
extern uint osl_malloc_failed(osl_t *osh);

extern void *osl_malloc_native(osl_t *osh, uint size);
extern void osl_mfree_native(osl_t *osh, char *addr, uint size);

#define	DMA_CONSISTENT_ALIGN	sizeof(int)
#define	DMA_ALLOC_CONSISTENT(osh, size, align, tot, pap, dmah)\
	osl_dma_alloc_consistent((osh), (size), (align), (tot), (pap))
extern void *osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align,
	uint *tot, dmaaddr_t *pap);
#define	DMA_FREE_CONSISTENT(osh, va, size, pa, dmah)\
	osl_dma_free_consistent((osh), (va), (size), (pa))
extern void osl_dma_free_consistent(osl_t *osh, void *va, uint size, dmaaddr_t pa);

#define	DMA_ALLOC_CONSISTENT_FORCE32(osh, size, align, tot, pap, dmah)\
	osl_dma_alloc_consistent_force32((osh), (size), (align), (tot), (pap))
extern void *osl_dma_alloc_consistent_force32(osl_t *osh, uint size, uint16 align,
	uint *tot, dmaaddr_t *pap);

#define	DMA_FREE_CONSISTENT_FORCE32(osh, va, size, pa, dmah)\
	osl_dma_free_consistent_force32((osh), (va), (size), (pa))
extern void osl_dma_free_consistent_force32(osl_t *osh, void *va, uint size, dmaaddr_t pa);

#define	DMA_MAP(osh, va, size, direction, p, dmah)\
	osl_dma_map((osh), (va), (size), (direction), (p))
extern dmaaddr_t osl_dma_map(osl_t *osh, void *va, uint size, int direction, void *lb);
#define	DMA_UNMAP(osh, pa, size, direction, p, dmah)\
	osl_dma_unmap((osh), (size), (direction), (p))
extern void osl_dma_unmap(osl_t *osh, uint size, int direction, void *lb);

/* API for DMA addressing capability */
#define OSL_DMADDRWIDTH(osh, addrwidth) osl_dmaddrwidth((osh), (addrwidth))
extern void osl_dmaddrwidth(osl_t *osh, uint addrwidth);
extern bool osl_dmaddr_valid(osl_t *osh, ulong loPart, ulong hiPart);

/* microsecond delay */
#define	OSL_DELAY(usec)		osl_delay(usec)
extern void osl_delay(uint usec);


#if defined(_X86_)
INLINE uint
osl_getcycles()
{
	volatile uint cycles;
	__asm {
		PUSH eax
		PUSH edx
		_emit 0x0f
		_emit 0x31
		MOV cycles, eax
		POP edx
		POP eax
	}
	return cycles;
}
#define OSL_GETCYCLES(x) ((x) = osl_getcycles())
#else
#define OSL_GETCYCLES(x) ((x) = 0)
#endif /* defined (_X86_) */

#define OSL_CACHE_FLUSH(va, len)	BCM_REFERENCE(va)
#define OSL_CACHE_INV(va, len)		BCM_REFERENCE(va)
#define OSL_PREFETCH(ptr)			BCM_REFERENCE(ptr)
#define OSL_CPU_RELAX()			(0)
#define OSL_DISABLE_PREEMPTION(osh) BCM_REFERENCE(osh)
#define OSL_ENABLE_PREEMPTION(osh)	BCM_REFERENCE(osh)

#if !defined(WLSIM)	/* shared memory access macros */
#define	R_SM(a)		*(a)
#define	W_SM(a, v)	(*(a) = (v))
#define	BZERO_SM(a, len)	NdisZeroMemory((a), (len))
#endif /* WLSIM */

/* the largest reasonable packet buffer driver uses for ethernet MTU in bytes */
#define	PKTBUFSZ	LBDATASZ

#define OSH_NULL	NULL

/* packet primitives */
#define	PKTGET(osh, len, send)		osl_pktget((osh), (len), (send))
extern void *osl_pktget(osl_t *osh, uint len, bool send);
#define	PKTFREE(osh, lb, send)		osl_pktfree((osh), (lb), (send))
extern void osl_pktfree(osl_t *osh, void *p, bool send);
#define	PKTDATA(osh, lb)		LBP(lb)->data
#define	PKTLEN(osh, lb)			LBP(lb)->len
#define PKTHEADROOM(osh, lb)		(PKTDATA(osh, lb)-(LBP(lb)->head))
#define PKTTAILROOM(osh, lb)		((LBP(lb)->end)-(LBP(lb)->tail))
#define	PKTNEXT(osh, lb)		LBP(lb)->next
#define	PKTSETNEXT(osh, lb, x)		LBP(lb)->next = (struct lbuf*)(x)
#define	PKTSETLEN(osh, lb, len)		osl_pktsetlen((osh), (lb), (len))
extern void osl_pktsetlen(osl_t *osh, void *lb, uint len);
#define	PKTPUSH(osh, lb, bytes)		osl_pktpush((osh), (lb), (bytes))
extern uchar *osl_pktpush(osl_t *osh, void *lb, uint bytes);
#define	PKTPULL(osh, lb, bytes)		osl_pktpull((osh), (lb), (bytes))
extern uchar *osl_pktpull(osl_t *osh, void *lb, uint bytes);
#define	PKTDUP(osh, lb)			osl_pktdup((osh), (lb))
extern void *osl_pktdup(osl_t *osh, void *lb);
#define	PKTTAG(lb)			(((void *) LBP(lb)->pkttag))
#define	PKTLINK(lb)			LBP(lb)->link
#define	PKTSETLINK(lb, x)		LBP(lb)->link = (struct lbuf*)(x)
#define	PKTPRIO(lb)			LBP(lb)->priority
#define	PKTSETPRIO(lb, x)		LBP(lb)->priority = (x)
#define	PKTFRMNATIVE(osh, x)		osl_pkt_frmnative((osh), (x))
#define	PKTTONATIVE(osh, lb)	osl_pkt_tonative((osh), (struct lbuf *)(lb))
#define PKTSHARED(lb)                   (0)
#define PKTALLOCED(osh)			(0)
#define PKTSETPOOL(osh, lb, x, y)   do {} while (0)
#define PKTPOOL(osh, lb)            FALSE
#define PKTFREELIST(lb)             PKTLINK(lb)
#define PKTSETFREELIST(lb, x)       PKTSETLINK((lb), (x))
#define PKTPTR(lb)                  (lb)
#define PKTID(lb)                   (0)
#define PKTSETID(lb, id)            do {} while (0)
#define PKTLIST_DUMP(osh, buf)
#define	PKTEXEMPT(lb)			((struct lbuf *)(lb))->exempt
#define PKTSETEXEMPT(lb, x)		((struct lbuf *)(lb))->exempt = (x)
#define PKTSETHEADROOM(l, x) (l.headroom = x);
#define PKTSUMNEEDED(lb)		(0)
#define PKTSHRINK(osh, m)		(m)
#define PKTORPHAN(pkt)			(pkt)

#define LBF_SUM_GOOD	0x1
#define PKTSETSUMGOOD(lb, x)		LBP(lb)->flags = ((x) ? \
						(LBP(lb)->flags | LBF_SUM_GOOD) : \
						(LBP(lb)->flags & ~LBF_SUM_GOOD))
extern void* osl_pkt_frmnative(osl_t *osh, ND_PKT *p);
extern ND_PKT *osl_pkt_tonative(osl_t *osh, struct lbuf *lb);
#define OSL_ERROR(bcmerror)	bcmerror2ndisstatus(bcmerror)

#define LBF_CHAINED  0x1000
extern bool lb_sane(struct lbuf *lb);

static bool
lb_ischained(struct lbuf *lb)
{
	ASSERT(lb_sane(lb));
	return ((lb->flags & LBF_CHAINED) != 0);
}

static void
lb_clearchained(struct lbuf *lb)
{
	ASSERT(lb_sane(lb));
	lb->flags &= ~LBF_CHAINED;
}

static void
lb_setchained(struct lbuf *lb)
{
	ASSERT(lb_sane(lb));
	lb->flags |= LBF_CHAINED;
}


#ifdef PKTC
/* RX */
#define	PKTCSETCNT(lb, c)	BCM_REFERENCE(lb)
#define	PKTCCLRFLAGS(lb)	BCM_REFERENCE(lb)
#define	PKTCSETLEN(lb, l)	BCM_REFERENCE(lb)
#define	PKTCADDLEN(lb, l)	BCM_REFERENCE(lb)
#define	PKTCLINK(lb)		PKTLINK(lb)
#define	PKTSETCLINK(lb, x)	PKTSETLINK(lb, x)
#define	FOREACH_CHAINED_PKT(lb, nlb) \
	for (; (lb) != NULL; (lb) = (nlb)) \
		if ((nlb) = PKTCLINK(lb), PKTSETCLINK((lb), NULL), 1)
#define	PKTCFREE(osh, lb, send) \
do {	\
	void *nlb;	 \
	ASSERT((lb) != NULL);      \
	FOREACH_CHAINED_PKT((lb), nlb) {		\
	PKTFREE((osh), LBP(lb), (send));	\
	}					\
} while (0)
#define PKTCENQTAIL(h, t, p) \
do {	\
		if ((t) == NULL) {	\
			(h) = (t) = (p);	\
		} else {	\
			PKTSETCLINK((t), (p));	\
			(t) = (p);		\
		}				\
} while (0)

/* TX */
#define      LBP(lb)         ((struct lbuf *)(lb))

#define	PKTCLRCHAINED(o, lb)	{BCM_REFERENCE(o); lb_clearchained(LBP(lb));}
#define	PKTSETCHAINED(o, lb)	{BCM_REFERENCE(o); lb_setchained(LBP(lb));}
#define	PKTISCHAINED(lb)	lb_ischained(LBP(lb))

#endif /* PKTC */


/* get system up time in miliseconds */
extern uint32 osl_systemuptime(void);
extern uint64 osl_systemuptime_us(void);
#define OSL_SYSUPTIME()		osl_systemuptime()
#define OSL_SYSUPTIME_US()	osl_systemuptime_us()

/* Global ASSERT type */
extern uint32 g_assert_type;

#define OSL_RAND()		osl_rand()
extern uint32 osl_rand(void);

#define OSL_SMP_WMB()				do { /* noop */ } while (0)

extern void * wl_os_open_image(char * filename);
extern int wl_os_get_image_block(char * buf, int len, void * image);
extern void wl_os_close_image(void * image);

#define osl_os_open_image wl_os_open_image
#define osl_os_get_image_block wl_os_get_image_block
#define osl_os_close_image wl_os_close_image

#endif	/* _ndis_osl_h_ */
