/*
 * Linux OS Independent Layer
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

#define LINUX_PORT

#include <typedefs.h>
#include <bcmendian.h>
#include <linuxver.h>
#include <bcmdefs.h>

#include <linux/random.h>

#include <osl.h>
#include <bcmutils.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <pcicfg.h>

#if defined(BCMASSERT_LOG) && !defined(OEM_ANDROID)
#include <bcm_assert_log.h>
#endif

#include <linux/fs.h>

#if (defined(BCM47XX_CA9) || defined(STB))
#include <linux/spinlock.h>
extern spinlock_t l2x0_reg_lock;
#endif /* BCM47XX_CA9 || STB */

#ifdef BCM_OBJECT_TRACE
#include <bcmutils.h>
#endif /* BCM_OBJECT_TRACE */
#include "linux_osl_priv.h"

#define PCI_CFG_RETRY		10

#define DUMPBUFSZ 1024

#if defined(STBLINUX) && defined(WLC_DUMP_MAC_EXT)
uint32 g_assert_delay = 3; //3600*24; /* 24h interval to capture the d11 reg list */
uint32 g_assert_type  = 1;	 /* eventually do kernel panic */
#else
#if defined(STB_SOC_WIFI) && !defined(BCMDBG)
uint32 g_assert_type = 1; /* For STB_SOC_WIFI without BCMDBG bypass Kernel Panic */
uint32 g_assert_delay = 300; /* For STB_SOC_WIFI without BCMDBG 5 min */
#else
uint32 g_assert_type  = 0; /* By Default Kernel Panic */
uint32 g_assert_delay = 3; /* Kernel Panic Delay */
#endif /* defined(STB_SOC_WIFI) && !defined(BCMDBG) */
#endif /* defined(WLC_DUMP_MAC_EXT) */

#if defined(BCMDBG_ASSERT) || defined(BCMASSERT_LOG)
uint8 g_assert_buf[256];
#endif /* defined(BCMDBG_ASSERT) || defined(BCMASSERT_LOG) */

module_param(g_assert_type, int, 0);
module_param(g_assert_delay, int, 0);

static int16 linuxbcmerrormap[] =
{	0,				/* 0 */
	-EINVAL,		/* BCME_ERROR */
	-EINVAL,		/* BCME_BADARG */
	-EINVAL,		/* BCME_BADOPTION */
	-EINVAL,		/* BCME_NOTUP */
	-EINVAL,		/* BCME_NOTDOWN */
	-EINVAL,		/* BCME_NOTAP */
	-EINVAL,		/* BCME_NOTSTA */
	-EINVAL,		/* BCME_BADKEYIDX */
	-EINVAL,		/* BCME_RADIOOFF */
	-EINVAL,		/* BCME_NOTBANDLOCKED */
	-EINVAL, 		/* BCME_NOCLK */
	-EINVAL, 		/* BCME_BADRATESET */
	-EINVAL, 		/* BCME_BADBAND */
	-E2BIG,			/* BCME_BUFTOOSHORT */
	-E2BIG,			/* BCME_BUFTOOLONG */
	-EBUSY, 		/* BCME_BUSY */
	-EINVAL, 		/* BCME_NOTASSOCIATED */
	-EINVAL, 		/* BCME_BADSSIDLEN */
	-EINVAL, 		/* BCME_OUTOFRANGECHAN */
	-EINVAL, 		/* BCME_BADCHAN */
	-EFAULT, 		/* BCME_BADADDR */
	-ENOMEM, 		/* BCME_NORESOURCE */
	-EOPNOTSUPP,		/* BCME_UNSUPPORTED */
	-EMSGSIZE,		/* BCME_BADLENGTH */
	-EINVAL,		/* BCME_NOTREADY */
	-EPERM,			/* BCME_EPERM */
	-ENOMEM, 		/* BCME_NOMEM */
	-EINVAL, 		/* BCME_ASSOCIATED */
	-ERANGE, 		/* BCME_RANGE */
	-EINVAL, 		/* BCME_NOTFOUND */
	-EINVAL, 		/* BCME_WME_NOT_ENABLED */
	-EINVAL, 		/* BCME_TSPEC_NOTFOUND */
	-EINVAL, 		/* BCME_ACM_NOTSUPPORTED */
	-EINVAL,		/* BCME_NOT_WME_ASSOCIATION */
	-EIO,			/* BCME_SDIO_ERROR */
	-ENODEV,		/* BCME_DONGLE_DOWN */
	-EINVAL,		/* BCME_VERSION */
	-EIO,			/* BCME_TXFAIL */
	-EIO,			/* BCME_RXFAIL */
	-ENODEV,		/* BCME_NODEVICE */
	-EINVAL,		/* BCME_NMODE_DISABLED */
	-ENODATA,		/* BCME_NONRESIDENT */
	-EINVAL,		/* BCME_SCANREJECT */
	-EINVAL,		/* BCME_USAGE_ERROR */
	-EIO,			/* BCME_IOCTL_ERROR */
	-EIO,			/* BCME_SERIAL_PORT_ERR */
	-EOPNOTSUPP,		/* BCME_DISABLED, BCME_NOTENABLED */
	-EIO,			/* BCME_DECERR */
	-EIO,			/* BCME_ENCERR */
	-EIO,			/* BCME_MICERR */
	-ERANGE,		/* BCME_REPLAY */
	-EINVAL,		/* BCME_IE_NOTFOUND */
	-EINVAL,		/* BCME_DATA_NOTFOUND */
	-EINVAL,		/* BCME_NOT_GC */
	-EINVAL,		/* BCME_PRS_REQ_FAILED */
	-EINVAL,		/* BCME_NO_P2P_SE */
	-EINVAL,		/* BCME_NOA_PND */
	-EINVAL,		/* BCME_FRAG_Q_FAILED */
	-EINVAL,		/* BCME_GET_AF_FAILED */
	-EINVAL,		/* BCME_MSCH_NOTREADY */
	-EINVAL,		/* BCME_IOV_LAST_CMD */
	-EINVAL,		/* BCME_MINIPMU_CAL_FAIL */
	-EINVAL,		/* BCME_RCAL_FAIL */
	-EINVAL,		/* BCME_LPF_RCCAL_FAIL */
	-EINVAL,		/* BCME_DACBUF_RCCAL_FAIL */
	-EINVAL,		/* BCME_VCOCAL_FAIL */
	-EINVAL,		/* BCME_BANDLOCKED */
	-EINVAL,		/* BCME_BAD_IE_DATA */

/* When an new error code is added to bcmutils.h, add os
 * specific error translation here as well
 */
/* check if BCME_LAST changed since the last time this function was updated */
#if BCME_LAST != -68
#error "You need to add a OS error translation in the linuxbcmerrormap \
	for new error code defined in bcmutils.h"
#endif
};
uint lmtest = FALSE;

/* translate bcmerrors into linux errors */
int
osl_error(int bcmerror)
{
	if (bcmerror > 0)
		bcmerror = 0;
	else if (bcmerror < BCME_LAST)
		bcmerror = BCME_ERROR;

	/* Array bounds covered by ASSERT in osl_attach */
	return linuxbcmerrormap[-bcmerror];
}

#if 0
#ifdef SHARED_OSL_CMN
osl_t *
osl_attach(void *pdev, uint bustype, bool pkttag, void **osl_cmn)
{
#else
osl_t *
osl_attach(void *pdev, uint bustype, bool pkttag)
{
	void **osl_cmn = NULL;
#endif /* SHARED_OSL_CMN */
	osl_t *osh;
	gfp_t flags;

	flags = CAN_SLEEP() ? GFP_KERNEL: GFP_ATOMIC;
	if (!(osh = kmalloc(sizeof(osl_t), flags)))
		return osh;

	ASSERT(osh);

//HACKHACK
	printk("\n================%s\n", __TIME__);


	bzero(osh, sizeof(osl_t));

	if (osl_cmn == NULL) {
		if (!(osh->cmn = kmalloc(sizeof(osl_cmn_t), flags))) {
			kfree(osh);
			return NULL;
		}

		bzero(osh->cmn, sizeof(osl_cmn_t));
		if (osl_cmn) {
			/* osl_cmn can be non null if SHARED_OSL_CMN defined */
			/* coverity[dead_error_line] */
			*osl_cmn = osh->cmn;
		}
		atomic_set(&osh->cmn->malloced, 0);
		osh->cmn->dbgmem_list = NULL;
		spin_lock_init(&(osh->cmn->dbgmem_lock));

#ifdef BCMDBG_PKT
		spin_lock_init(&(osh->cmn->pktlist_lock));
#endif
		spin_lock_init(&(osh->cmn->pktalloc_lock));

	} else {
		osh->cmn = *osl_cmn;
	}
	atomic_add(1, &osh->cmn->refcount);

	bcm_object_trace_init();

	/* Check that error map has the right number of entries in it */
	ASSERT(ABS(BCME_LAST) == (ARRAYSIZE(linuxbcmerrormap) - 1));

	osh->failed = 0;
	osh->pdev = pdev;
	osh->pub.pkttag = pkttag;
	osh->bustype = bustype;
	osh->magic = OS_HANDLE_MAGIC;

	switch (bustype) {
		case PCI_BUS:
		case SI_BUS:
		case PCMCIA_BUS:
			osh->pub.mmbus = TRUE;
			break;
		case JTAG_BUS:
		case SDIO_BUS:
		case USB_BUS:
		case SPI_BUS:
		case RPC_BUS:
			osh->pub.mmbus = FALSE;
			break;
		default:
			ASSERT(FALSE);
			break;
	}

#ifdef BCMDBG_CTRACE
	spin_lock_init(&osh->ctrace_lock);
	INIT_LIST_HEAD(&osh->ctrace_list);
	osh->ctrace_num = 0;
#endif /* BCMDBG_CTRACE */


#ifdef BCMDBG_ASSERT
	if (pkttag) {
		struct sk_buff *skb;
		BCM_REFERENCE(skb);
		ASSERT(OSL_PKTTAG_SZ <= sizeof(skb->cb));
	}
#endif
	return osh;
}

void osl_set_bus_handle(osl_t *osh, void *bus_handle)
{
	osh->bus_handle = bus_handle;
}

void* osl_get_bus_handle(osl_t *osh)
{
	return osh->bus_handle;
}

#if defined(BCM_BACKPLANE_TIMEOUT)
void osl_set_bpt_cb(osl_t *osh, void *bpt_cb, void *bpt_ctx)
{
	if (osh) {
		osh->bpt_cb = (bpt_cb_fn)bpt_cb;
		osh->sih = bpt_ctx;
	}
}
#endif	/* BCM_BACKPLANE_TIMEOUT */
#endif

#if 0
void
osl_detach(osl_t *osh)
{
	if (osh == NULL)
		return;

#ifdef BCMDBG_MEM
	if (MEMORY_LEFTOVER(osh)) {
		static char dumpbuf[DUMPBUFSZ];
		struct bcmstrbuf b;

		printf("%s: MEMORY LEAK %d bytes\n", __FUNCTION__, MALLOCED(osh));
		bcm_binit(&b, dumpbuf, DUMPBUFSZ);
		MALLOC_DUMP(osh, &b);
		printf("%s", b.origbuf);
	}
#endif

	bcm_object_trace_deinit();

	ASSERT(osh->magic == OS_HANDLE_MAGIC);
	atomic_sub(1, &osh->cmn->refcount);
	if (atomic_read(&osh->cmn->refcount) == 0) {
			kfree(osh->cmn);
	}
	kfree(osh);
}
#endif

#if 0
/* APIs to set/get specific quirks in OSL layer */
void BCMFASTPATH
osl_flag_set(osl_t *osh, uint32 mask)
{
	osh->flags |= mask;
}

void
osl_flag_clr(osl_t *osh, uint32 mask)
{
	osh->flags &= ~mask;
}

inline bool BCMFASTPATH
osl_is_flag_set(osl_t *osh, uint32 mask)
{
	return (osh->flags & mask);
}

inline int BCMFASTPATH
osl_arch_is_coherent(void)
{
	return 0;
}

inline int BCMFASTPATH
osl_acp_war_enab(void)
{
	return 0;
}
#endif

#if 0
inline void BCMFASTPATH
osl_cache_flush(void *va, uint size)
{
	if (size > 0) {
		dma_sync_single_for_device(OSH_NULL, virt_to_phys(va), size, DMA_TX);
		//dma_sync_single_for_device(OSH_NULL, virt_to_dma(OSH_NULL, va), size,
		//	DMA_TO_DEVICE);
	}
}

inline void BCMFASTPATH
osl_cache_inv(void *va, uint size)
{
	dma_sync_single_for_cpu(OSH_NULL, virt_to_phys(va), size, DMA_RX);
	//dma_sync_single_for_cpu(OSH_NULL, virt_to_dma(OSH_NULL, va), size, DMA_FROM_DEVICE);
}

inline void BCMFASTPATH
osl_prefetch(const void *ptr)
{
}
#endif

#if 0
uint32
osl_pci_read_config(osl_t *osh, uint offset, uint size)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);
	return 0;
}

void
osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);
}

/* return bus # for the pci device pointed by osh->pdev */
uint
osl_pci_bus(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);
	return 0;
}

/* return slot # for the pci device pointed by osh->pdev */
uint
osl_pci_slot(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);
	return 0;
}

/* return domain # for the pci device pointed by osh->pdev */
uint
osl_pcie_domain(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

	return 0;
}

/* return bus # for the pci device pointed by osh->pdev */
uint
osl_pcie_bus(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

	return 0;
}

/* return the pci device pointed by osh->pdev */
struct pci_dev *
osl_pci_device(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

	return NULL;
}


void
osl_pcmcia_read_attr(osl_t *osh, uint offset, void *buf, int size)
{
}

void
osl_pcmcia_write_attr(osl_t *osh, uint offset, void *buf, int size)
{
}
#endif

#if 0
#ifdef BCMDBG_MEM
/* In BCMDBG_MEM configurations osl_malloc is only used internally in
 * the implementation of osl_debug_malloc.  Because we are using the GCC
 * -Wstrict-prototypes compile option, we must always have a prototype
 * for a global/external function.  So make osl_malloc static in
 * the BCMDBG_MEM case.
 */
static
#endif
void *
osl_malloc(osl_t *osh, uint size)
{
	void *addr;
	gfp_t flags;

	/* only ASSERT if osh is defined */
	if (osh)
		ASSERT(osh->magic == OS_HANDLE_MAGIC);

	flags = CAN_SLEEP() ? GFP_KERNEL: GFP_ATOMIC;
	if ((addr = kmalloc(size, flags)) == NULL) {
		if (osh)
			osh->failed++;
		return (NULL);
	}
	if (osh && osh->cmn)
		atomic_add(size, &osh->cmn->malloced);

	return (addr);
}

#ifndef BCMDBG_MEM
void *
osl_mallocz(osl_t *osh, uint size)
{
	void *ptr;

	ptr = osl_malloc(osh, size);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}
#endif

#ifdef BCMDBG_MEM
/* In BCMDBG_MEM configurations osl_mfree is only used internally in
 * the implementation of osl_debug_mfree.  Because we are using the GCC
 * -Wstrict-prototypes compile option, we must always have a prototype
 * for a global/external function.  So make osl_mfree static in
 * the BCMDBG_MEM case.
 */
static
#endif
void
osl_mfree(osl_t *osh, void *addr, uint size)
{
	if (osh && osh->cmn) {
		ASSERT(osh->magic == OS_HANDLE_MAGIC);

		ASSERT(size <= osl_malloced(osh));

		atomic_sub(size, &osh->cmn->malloced);
	}
	kfree(addr);
}

#ifdef BCMDBG_MEM
/* In BCMDBG_MEM configurations osl_vmalloc is only used internally in
 * the implementation of osl_debug_vmalloc.  Because we are using the GCC
 * -Wstrict-prototypes compile option, we must always have a prototype
 * for a global/external function.  So make osl_vmalloc static in
 * the BCMDBG_MEM case.
 */
static
#endif
void *
osl_vmalloc(osl_t *osh, uint size)
{
	void *addr;

	/* only ASSERT if osh is defined */
	if (osh)
		ASSERT(osh->magic == OS_HANDLE_MAGIC);
	if ((addr = vmalloc(size)) == NULL) {
		if (osh)
			osh->failed++;
		return (NULL);
	}
	if (osh && osh->cmn)
		atomic_add(size, &osh->cmn->malloced);

	return (addr);
}

#ifndef BCMDBG_MEM
void *
osl_vmallocz(osl_t *osh, uint size)
{
	void *ptr;

	ptr = osl_vmalloc(osh, size);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}
#endif

#ifdef BCMDBG_MEM
/* In BCMDBG_MEM configurations osl_vmfree is only used internally in
 * the implementation of osl_debug_vmfree.  Because we are using the GCC
 * -Wstrict-prototypes compile option, we must always have a prototype
 * for a global/external function.  So make osl_vmfree static in
 * the BCMDBG_MEM case.
 */
static
#endif
void
osl_vmfree(osl_t *osh, void *addr, uint size)
{
	if (osh && osh->cmn) {
		ASSERT(osh->magic == OS_HANDLE_MAGIC);

		ASSERT(size <= osl_malloced(osh));

		atomic_sub(size, &osh->cmn->malloced);
	}
	vfree(addr);
}

uint
osl_check_memleak(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	if (atomic_read(&osh->cmn->refcount) == 1)
		return (atomic_read(&osh->cmn->malloced));
	else
		return 0;
}

uint
osl_malloced(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	return (atomic_read(&osh->cmn->malloced));
}

uint
osl_malloc_failed(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	return (osh->failed);
}
#endif /* 0/1 */

#ifdef BCMDBG_MEM
#if 0
extern void *osl_malloc(osl_t *osh, uint size);
extern void *osl_mallocz(osl_t *osh, uint size);
extern void osl_mfree(osl_t *osh, void *addr, uint size);
extern void *osl_vmalloc(osl_t *osh, uint size);
extern void *osl_vmallocz(osl_t *osh, uint size);
extern void osl_vmfree(osl_t *osh, void *addr, uint size);
extern uint osl_malloced(osl_t *osh);
extern uint osl_check_memleak(osl_t *osh);

#define MEMLIST_LOCK(osh, flags)	spin_lock_irqsave(&(osh)->cmn->dbgmem_lock, flags)
#define MEMLIST_UNLOCK(osh, flags)	spin_unlock_irqrestore(&(osh)->cmn->dbgmem_lock, flags)
void *
osl_debug_malloc(osl_t *osh, uint size, int line, const char* file)
{
	bcm_mem_link_t *p;
	const char* basename;
	unsigned long flags = 0;
	if (!size) {
		printk("%s: allocating zero sized mem at %s line %d\n", __FUNCTION__, file, line);
		ASSERT(0);
	}

	if ((p = (bcm_mem_link_t*)osl_malloc(osh, sizeof(bcm_mem_link_t) + size)) == NULL) {
		return (NULL);
	}

	if (osh) {
		MEMLIST_LOCK(osh, flags);
	}

	p->size = size;
	p->line = line;
	p->osh = (void *)osh;

	basename = strrchr(file, '/');
	/* skip the '/' */
	if (basename)
		basename++;

	if (!basename)
		basename = file;

	strncpy(p->file, basename, BCM_MEM_FILENAME_LEN);
	p->file[BCM_MEM_FILENAME_LEN - 1] = '\0';

	/* link this block */
	if (osh) {
		p->prev = NULL;
		p->next = osh->cmn->dbgmem_list;
		if (p->next)
			p->next->prev = p;
		osh->cmn->dbgmem_list = p;
		MEMLIST_UNLOCK(osh, flags);
	}

	return p + 1;
}

void *
osl_debug_mallocz(osl_t *osh, uint size, int line, const char* file)
{
	void *ptr;

	ptr = osl_debug_malloc(osh, size, line, file);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void
osl_debug_mfree(osl_t *osh, void *addr, uint size, int line, const char* file)
{
	bcm_mem_link_t *p = (bcm_mem_link_t *)((int8*)addr - sizeof(bcm_mem_link_t));
	unsigned long flags = 0;

	ASSERT(osh == NULL || osh->magic == OS_HANDLE_MAGIC);

	if (p->size == 0) {
		printk("osl_debug_mfree: double free on addr %p size %d at line %d file %s\n",
			addr, size, line, file);
		ASSERT(p->size);
		return;
	}

	if (p->size != size) {
		printk("%s: dealloca size does not match alloc size\n", __FUNCTION__);
		printk("Dealloc addr %p size %d at line %d file %s\n", addr, size, line, file);
		printk("Alloc size %d line %d file %s\n", p->size, p->line, p->file);
		ASSERT(p->size == size);
		return;
	}

	if (osh && ((osl_t*)p->osh)->cmn != osh->cmn) {
		printk("osl_debug_mfree: alloc osh %p does not match dealloc osh %p\n",
			((osl_t*)p->osh)->cmn, osh->cmn);
		printk("Dealloc addr %p size %d at line %d file %s\n", addr, size, line, file);
		printk("Alloc size %d line %d file %s\n", p->size, p->line, p->file);
		ASSERT(((osl_t*)p->osh)->cmn == osh->cmn);
		return;
	}

	/* unlink this block */
	if (osh && osh->cmn) {
		MEMLIST_LOCK(osh, flags);
		if (p->prev)
			p->prev->next = p->next;
		if (p->next)
			p->next->prev = p->prev;
		if (osh->cmn->dbgmem_list == p)
			osh->cmn->dbgmem_list = p->next;
		p->next = p->prev = NULL;
	}
	p->size = 0;

	if (osh && osh->cmn) {
		MEMLIST_UNLOCK(osh, flags);
	}
	osl_mfree(osh, p, size + sizeof(bcm_mem_link_t));
}

void *
osl_debug_vmalloc(osl_t *osh, uint size, int line, const char* file)
{
	bcm_mem_link_t *p;
	const char* basename;
	unsigned long flags = 0;
	if (!size) {
		printk("%s: allocating zero sized mem at %s line %d\n", __FUNCTION__, file, line);
		ASSERT(0);
	}

	if ((p = (bcm_mem_link_t*)osl_vmalloc(osh, sizeof(bcm_mem_link_t) + size)) == NULL) {
		return (NULL);
	}

	if (osh) {
		MEMLIST_LOCK(osh, flags);
	}

	p->size = size;
	p->line = line;
	p->osh = (void *)osh;

	basename = strrchr(file, '/');
	/* skip the '/' */
	if (basename)
		basename++;

	if (!basename)
		basename = file;

	strncpy(p->file, basename, BCM_MEM_FILENAME_LEN);
	p->file[BCM_MEM_FILENAME_LEN - 1] = '\0';

	/* link this block */
	if (osh) {
		p->prev = NULL;
		p->next = osh->cmn->dbgvmem_list;
		if (p->next)
			p->next->prev = p;
		osh->cmn->dbgvmem_list = p;
		MEMLIST_UNLOCK(osh, flags);
	}

	return p + 1;
}

void *
osl_debug_vmallocz(osl_t *osh, uint size, int line, const char* file)
{
	void *ptr;

	ptr = osl_debug_vmalloc(osh, size, line, file);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void
osl_debug_vmfree(osl_t *osh, void *addr, uint size, int line, const char* file)
{
	bcm_mem_link_t *p = (bcm_mem_link_t *)((int8*)addr - sizeof(bcm_mem_link_t));
	unsigned long flags = 0;

	ASSERT(osh == NULL || osh->magic == OS_HANDLE_MAGIC);

	if (p->size == 0) {
		printk("osl_debug_mfree: double free on addr %p size %d at line %d file %s\n",
			addr, size, line, file);
		ASSERT(p->size);
		return;
	}

	if (p->size != size) {
		printk("%s: dealloca size does not match alloc size\n", __FUNCTION__);
		printk("Dealloc addr %p size %d at line %d file %s\n", addr, size, line, file);
		printk("Alloc size %d line %d file %s\n", p->size, p->line, p->file);
		ASSERT(p->size == size);
		return;
	}

	if (osh && ((osl_t*)p->osh)->cmn != osh->cmn) {
		printk("osl_debug_mfree: alloc osh %p does not match dealloc osh %p\n",
			((osl_t*)p->osh)->cmn, osh->cmn);
		printk("Dealloc addr %p size %d at line %d file %s\n", addr, size, line, file);
		printk("Alloc size %d line %d file %s\n", p->size, p->line, p->file);
		ASSERT(((osl_t*)p->osh)->cmn == osh->cmn);
		return;
	}

	/* unlink this block */
	if (osh && osh->cmn) {
		MEMLIST_LOCK(osh, flags);
		if (p->prev)
			p->prev->next = p->next;
		if (p->next)
			p->next->prev = p->prev;
		if (osh->cmn->dbgvmem_list == p)
			osh->cmn->dbgvmem_list = p->next;
		p->next = p->prev = NULL;
	}
	p->size = 0;

	if (osh && osh->cmn) {
		MEMLIST_UNLOCK(osh, flags);
	}
	osl_vmfree(osh, p, size + sizeof(bcm_mem_link_t));
}
#endif

int
osl_debug_memdump(osl_t *osh, struct bcmstrbuf *b)
{
	bcm_mem_link_t *p;
	unsigned long flags = 0;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	spin_lock_irqsave(&(osh)->cmn->dbgmem_lock, flags);

	if (osl_check_memleak(osh) && osh->cmn->dbgmem_list) {
		if (b != NULL)
			bcm_bprintf(b, "   Address   Size File:line\n");
		else
			printk("   Address   Size File:line\n");

		for (p = osh->cmn->dbgmem_list; p; p = p->next) {
			if (b != NULL)
				bcm_bprintf(b, "%p %6d %s:%d\n", (char*)p + sizeof(bcm_mem_link_t),
					p->size, p->file, p->line);
			else
				printk("%p %6d %s:%d\n", (char*)p + sizeof(bcm_mem_link_t),
					p->size, p->file, p->line);

			/* Detects loop-to-self so we don't enter infinite loop */
			if (p == p->next) {
				if (b != NULL)
					bcm_bprintf(b, "WARNING: loop-to-self "
						"p %p p->next %p\n", p, p->next);
				else
					printk("WARNING: loop-to-self "
						"p %p p->next %p\n", p, p->next);

				break;
			}
		}
	}
	if (osl_check_memleak(osh) && osh->cmn->dbgvmem_list) {
		if (b != NULL)
			bcm_bprintf(b, "Vmem\n   Address   Size File:line\n");
		else
			printk("Vmem\n   Address   Size File:line\n");

		for (p = osh->cmn->dbgvmem_list; p; p = p->next) {
			if (b != NULL)
				bcm_bprintf(b, "%p %6d %s:%d\n", (char*)p + sizeof(bcm_mem_link_t),
					p->size, p->file, p->line);
			else
				printk("%p %6d %s:%d\n", (char*)p + sizeof(bcm_mem_link_t),
					p->size, p->file, p->line);

			/* Detects loop-to-self so we don't enter infinite loop */
			if (p == p->next) {
				if (b != NULL)
					bcm_bprintf(b, "WARNING: loop-to-self "
						"p %p p->next %p\n", p, p->next);
				else
					printk("WARNING: loop-to-self "
						"p %p p->next %p\n", p, p->next);

				break;
			}
		}
	}

	spin_unlock_irqrestore(&(osh)->cmn->dbgmem_lock, flags);

	return 0;
}

#endif	/* BCMDBG_MEM */

#if 0
uint
osl_dma_consistent_align(void)
{
	return (PAGE_SIZE);
}

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

void *
osl_virt_to_phys(void *va)
{
	return (void *)(uintptr)virt_to_phys(va);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
#include <asm/cacheflush.h>
void BCMFASTPATH
osl_dma_flush(osl_t *osh, void *va, uint size, int direction, void *p, hnddma_seg_map_t *dmah)
{
	if (direction == DMA_TX) { /* to device */
		osl_cache_flush(va, size);
	}
	else {
		ASSERT(0);
	}
	return;
}
#endif /* LINUX_VERSION_CODE >= 2.6.36 */

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
#endif 


#if 0
/* OSL function for CPU relax */
inline void BCMFASTPATH
osl_cpu_relax(void)
{
	cpu_relax();
}

extern void osl_preempt_disable(osl_t *osh)
{
	preempt_disable();
}

extern void osl_preempt_enable(osl_t *osh)
{
	preempt_enable();
}
#endif

#if defined(BCMDBG_ASSERT) || defined(BCMASSERT_LOG)
void
osl_assert(const char *exp, const char *file, int line)
{
	const char *basename;

	basename = strrchr(file, '/');
	/* skip the '/' */
	if (basename)
		basename++;

	if (!basename)
		basename = file;

#ifdef BCMASSERT_LOG
	snprintf(g_assert_buf, 64, "\"%s\": file \"%s\", line %d\n",
		exp, basename, line);
#ifndef OEM_ANDROID
	bcm_assert_log(g_assert_buf);
#endif /* OEM_ANDROID */
#endif /* BCMASSERT_LOG */

#ifdef BCMDBG_ASSERT
	snprintf(g_assert_buf, 256, "assertion \"%s\" failed: file \"%s\", line %d\n",
		exp, basename, line);
	dump_stack();

	/* Print assert message and give it time to be written to /var/log/messages */
	if (!in_interrupt() && g_assert_type != 1 && g_assert_type != 3) {
		const int delay = g_assert_delay;
		printk("%s", g_assert_buf);
		printk("panic in %d seconds\n", delay);
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(delay * HZ);
	}
#endif /* BCMDBG_ASSERT */

	switch (g_assert_type) {
	case 0:
		panic("%s", g_assert_buf);
		break;
	case 1:
		/* fall through */
	case 3:
		printk("%s", g_assert_buf);
		break;
	case 2:
		printk("%s", g_assert_buf);
		BUG();
		break;
	default:
		break;
	}
}
#endif /* BCMDBG_ASSERT || BCMASSERT_LOG */

#if 0
void
osl_delay(uint usec)
{
	uint d;

	while (usec > 0) {
		d = MIN(usec, 1000);
		udelay(d);
		usec -= d;
	}
}

void
osl_sleep(uint ms)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	if (ms < 20)
		usleep_range(ms*1000, ms*1000 + 1000);
	else
#endif
	msleep(ms);
}

uint64
osl_sysuptime_us(void)
{
	struct timeval tv;
	uint64 usec;

	do_gettimeofday(&tv);
	/* tv_usec content is fraction of a second */
	usec = (uint64)tv.tv_sec * 1000000ul + tv.tv_usec;
	return usec;
}


/*
 * BINOSL selects the slightly slower function-call-based binary compatible osl.
 */

uint32
osl_rand(void)
{
	uint32 rand;

	get_random_bytes(&rand, sizeof(rand));

	return rand;
}
#endif

#if 0
/* Linux Kernel: File Operations: start */
void *
osl_os_open_image(char *filename)
{
	struct file *fp;

	fp = filp_open(filename, O_RDONLY, 0);
	/*
	 * 2.6.11 (FC4) supports filp_open() but later revs don't?
	 * Alternative:
	 * fp = open_namei(AT_FDCWD, filename, O_RD, 0);
	 * ???
	 */
	 if (IS_ERR(fp))
		 fp = NULL;

	 return fp;
}

int
osl_os_get_image_block(char *buf, int len, void *image)
{
	struct file *fp = (struct file *)image;
	int rdlen;

	if (!image)
		return 0;

	rdlen = kernel_read(fp, fp->f_pos, buf, len);
	if (rdlen > 0)
		fp->f_pos += rdlen;

	return rdlen;
}

void
osl_os_close_image(void *image)
{
	if (image)
		filp_close((struct file *)image, NULL);
}

int
osl_os_image_size(void *image)
{
	int len = 0, curroffset;

	if (image) {
		/* store the current offset */
		curroffset = generic_file_llseek(image, 0, 1);
		/* goto end of file to get length */
		len = generic_file_llseek(image, 0, 2);
		/* restore back the offset */
		generic_file_llseek(image, curroffset, 0);
	}
	return len;
}

#if defined(BCM_BACKPLANE_TIMEOUT)
inline void osl_bpt_rreg(osl_t *osh, ulong addr, volatile void *v, uint size)
{
	bool poll_timeout = FALSE;
	static int in_si_clear = FALSE;
	switch (size) {
	case sizeof(uint8):
		*(volatile uint8*)v = readb((volatile uint8*)(addr));
		if (*(volatile uint8*)v == 0xff)
			poll_timeout = TRUE;
		break;
	case sizeof(uint16):
		*(volatile uint16*)v = readw((volatile uint16*)(addr));
		if (*(volatile uint16*)v == 0xffff)
			poll_timeout = TRUE;
		break;
	case sizeof(uint32):
		*(volatile uint32*)v = readl((volatile uint32*)(addr));
		if (*(volatile uint32*)v == 0xffffffff)
			poll_timeout = TRUE;
		break;
	case sizeof(uint64):
		*(volatile uint64*)v = *((volatile uint64*)(addr));
		if (*(volatile uint64*)v == 0xffffffffffffffff)
			poll_timeout = TRUE;
		break;
	}

	if (osh && osh->sih && (in_si_clear == FALSE) && poll_timeout && osh->bpt_cb) {
		in_si_clear = TRUE;
		osh->bpt_cb((void *)osh->sih, (void *)addr);
		in_si_clear = FALSE;
	}
}
#endif /* BCM_BACKPLANE_TIMEOUT */
#endif
