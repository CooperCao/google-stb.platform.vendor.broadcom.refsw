/*
 * NDIS Independent Layer
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

#define	NDIS_OSL

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <osl.h>
#include <bcmutils.h>
#include <pcicfg.h>
#include <proto/ethernet.h>
#include <ndiserrmap.h>
#ifdef BCMASSERT_LOG
#include <bcm_assert_log.h>
#endif
#if defined(WL_NDWDI)
#include <wl_ndwdi_txrx.h>
#endif /* defined(WL_NDWDI) */

#define PCI_CFG_RETRY	10

#define LBOFF(lb) ((uintptr)lb->data - (uintptr)lb->head)

#define OS_HANDLE_MAGIC         0xabcd1234 /* Magic # to recognise osh */
#define BCM_MEM_FILENAME_LEN    24	   /* Mem. filename length */

typedef struct bcm_mem_link {
	struct bcm_mem_link *prev;
	struct bcm_mem_link *next;
	uint    size;
	int     line;
	char    file[BCM_MEM_FILENAME_LEN];
} bcm_mem_link_t;

struct osl_dmainfo {
	uint addrwidth;   /* Maximum addressability */
	uint32 hiaddrmask; /* Mask for upper 32 bits */
	uint32 loaddrmask; /* Mask for lower 32 bits */
};

struct osl_info {
	void *pdev;
	uint magic;
	uint malloced;
	uint failed;
	bcm_mem_link_t *dbgmem_list;
	osldma_t dma_cap;
	pktfree_cb_fn_t tx_fn;
	void *tx_ctx;
	NDIS_SPIN_LOCK malloc_lock;
#if defined(OSLREGOPS)
	osl_rreg_fn_t rreg_fn;	/* Read Register function */
	osl_wreg_fn_t wreg_fn;	/* Write Register function */
	void *reg_ctx;		/* Context to the reg callback functions */
#endif
#if defined(BCM_BACKPLANE_TIMEOUT) && defined(BCMDBG)
	void *curmap;
#endif

};

void wl_txq_lock(void *wl);
void wl_txq_unlock(void *wl);

/* Global ASSERT type */
uint32 g_assert_type = 0;

osl_t *
osl_attach(void *pdev, NDIS_HANDLE adapterhandle)
{
	NDIS_STATUS status;
	osl_t *osh;
	int last_error = BCME_LAST;

#if (0< 0x0600)
	status = NdisAllocateMemoryWithTag(&osh, sizeof(osl_t), MEMORY_TAG);
#else /* (NDISVER >= 0x0600) */
	osh = NdisAllocateMemoryWithTagPriority(adapterhandle, sizeof(osl_t),
	      MEMORY_TAG, HighPoolPriority);
	if (!osh)
		status = NDIS_STATUS_FAILURE;
	else
		status = NDIS_STATUS_SUCCESS;
#endif 

	if (status != NDIS_STATUS_SUCCESS)
		return NULL;

	ASSERT(osh);
	NdisZeroMemory(osh, sizeof(osl_t));
	/*
	 * check the cases where
	 * 1.Error code Added to bcmerror table, but forgot to add it to the OS
	 * dependent error code
	 * 2. Error code is added to the bcmerror table, but forgot to add the
	 * corresponding errorstring (dummy call to bcmerrorstr)
	 */
	ndisstatus2bcmerror(NDIS_STATUS_SUCCESS);
	bcmerrorstr(0);

	osh->magic = OS_HANDLE_MAGIC;
	osh->malloced = 0;
	osh->failed = 0;
	osh->dbgmem_list = NULL;
	osh->pdev = pdev;
	NdisAllocateSpinLock(&(osh->malloc_lock));

	return osh;
}

void
osl_detach(osl_t *osh)
{
	if (osh == NULL)
		return;

	ASSERT(osh->magic == OS_HANDLE_MAGIC);
	NdisFreeMemory(osh, sizeof(osl_t), 0);
}

void
osl_pktfree_cb_set(osl_t *osh, pktfree_cb_fn_t tx_fn, void *tx_ctx)
{
	osh->tx_fn = tx_fn;
	osh->tx_ctx = tx_ctx;
}

void
osl_delay(uint usec)
{
#define MAX_BUSY_DEL  50

	/* Delay should not be too long at DISPATCH or higher IRQL */
	ASSERT((usec <= 100) || (KeGetCurrentIrql() < DISPATCH_LEVEL));

	/* As per MS documentation NdisStallExecution should not be called for more than 50 us.
	 * However NdisMSleep cannot be called at DISPATCH or higher IRQL
	 * For poerformance reasons use NdisStallExecution till 1 ms
	 */

	if ((usec <= 1000) || (KeGetCurrentIrql() >= DISPATCH_LEVEL))  {
		int i, div, rem;
		div = usec / MAX_BUSY_DEL;
		rem = usec % MAX_BUSY_DEL;

		for (i = 0; i < div; i++) {
			NdisStallExecution(MAX_BUSY_DEL);
		}
		NdisStallExecution(rem);
	} else {
		NdisMSleep(usec);
	}
}

#if defined(BCM_BACKPLANE_TIMEOUT) && defined(BCMDBG)

void
setCurMap(osl_t *osh, void *curmap)
{
	osh->curmap = curmap;
}

uint32
read_bpt_reg(osl_t *osh, volatile void *r, uint32 size)
{
	uint32 status, errStatus, errCtrl;

	if (size == 4) {
		status = readl((uint32*)(r));
	}
	else if (size == 2) {
		status = readw((uint16*)(r));
	}
	else {
		status = readb((uint8*)(r));
	}

	errCtrl = readl((uint *)((uchar *)osh->curmap + 0x5900));

	/*
	* Check if errCtrl value is all F's
	* This means that there is an error in the backplane
	*/

	if (errCtrl != 0xFFFFFFFF) {
		if (errCtrl & 0x200) {
			/*
			* read the error control register
			* error control tells if the BPT is enabled or not
			*/

			OSL_DELAY(2);

			/*
			* read the error Status register
			*/

			errStatus = readl((uint *)((uchar *)osh->curmap + 0x5908));

			if (errStatus == 0xFFFFFFF) {
				printf("backplane returned all F's \n");
			}

			/* delay of 2 us */
			OSL_DELAY(2);
			if (errStatus & 0x3) {
				printf("error on BPT status %x errLogStatus %x"
					"Reg %x errlowAddr = %x errHighAddr = %x"
					"errLogId = %x errLogflags = %x\n",
					status, errStatus, r,
					readl((uint *)((uchar *)osh->curmap + 0x590C)),
					readl((uint *)((uchar *)osh->curmap + 0x5910)),
					readl((uint *)((uchar *)osh->curmap + 0x5914)),
					readl((uint *)((uchar *)osh->curmap + 0x591c)));

				OSL_DELAY(1000);
				ASSERT(0 && ("Backplane timeout in reading the register"));
			}
		}
	}
	else {
		printf("errCtrl returned %x\n", errCtrl);
	}

	return status;
}

#endif /* #if defined(BCM_BACKPLANE_TIMEOUT) && defined(BCMDBG) */

void
osl_dmaddrwidth(osl_t *osh, uint addrwidth)
{
	shared_info_t *shared;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	shared = osh->pdev;
	osh->dma_cap.addrwidth = addrwidth;
	shared->dmaddrwidth = addrwidth;

	switch (addrwidth) {
	case DMADDRWIDTH_26:
		osh->dma_cap.loaddrmask = DMADDR_MASK_26;
		osh->dma_cap.hiaddrmask = DMADDR_MASK_0;
		break;
	case DMADDRWIDTH_30:
		osh->dma_cap.loaddrmask = DMADDR_MASK_30;
		osh->dma_cap.hiaddrmask = DMADDR_MASK_0;
		break;
	case DMADDRWIDTH_32:
		osh->dma_cap.loaddrmask = DMADDR_MASK_32;
		osh->dma_cap.hiaddrmask = DMADDR_MASK_0;
		break;
	case DMADDRWIDTH_64:
		osh->dma_cap.loaddrmask = DMADDR_MASK_32;
#ifdef BCMDMA64OSL
		osh->dma_cap.hiaddrmask = DMADDR_MASK_32;
#else
		osh->dma_cap.hiaddrmask = DMADDR_MASK_0;
#endif
		break;
	default:
		ASSERT(0);
	}

	shared = osh->pdev;

#ifdef BCMDBG
	printf("osl_dmacap_register: 0x%x 0x%x 0x%x\n", addrwidth, osh->dma_cap.loaddrmask,
	       osh->dma_cap.hiaddrmask);
#endif
}

/* Return if a address is within the range of DMA's access capability */
bool
osl_dmaddr_valid(osl_t *osh, ulong loPart, ulong hiPart)
{
	ASSERT(osh->dma_cap.addrwidth);
	return !((loPart & osh->dma_cap.loaddrmask) ||
	         (hiPart & osh->dma_cap.hiaddrmask));
}

#ifdef BCMDBG_MEM
#define MEMDBG_SIZE_LIMIT 0x10000000

void*
osl_debug_malloc(osl_t *osh, uint size, int line, char* file)
{
	bcm_mem_link_t *p;
	char* basename;

	NdisAcquireSpinLock(&(osh->malloc_lock));

	ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	ASSERT(size);

	if ((p = (bcm_mem_link_t*)osl_malloc(osh, sizeof(bcm_mem_link_t) + size)) == NULL)
	{
		NdisReleaseSpinLock(&(osh->malloc_lock));
		return (NULL);
	}

	p->size = size;
	p->line = line;

	basename = strrchr(file, '\\');
	/* skip the '/' */
	if (basename)
		basename++;

	if (!basename)
		basename = file;

	strncpy(p->file, basename, BCM_MEM_FILENAME_LEN);
	p->file[BCM_MEM_FILENAME_LEN - 1] = '\0';

	/* link this block */
	p->prev = NULL;
	p->next = osh->dbgmem_list;
	if (p->next)
		p->next->prev = p;
	osh->dbgmem_list = p;

	NdisReleaseSpinLock(&(osh->malloc_lock));
	return p + 1;
}

void*
osl_debug_mallocz(osl_t *osh, uint size, int line, char* file)
{
	void *ptr;

	ptr = osl_debug_malloc(osh, size, line, file);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void
osl_debug_mfree(osl_t *osh, void *addr, uint size, int line, char* file)
{
	bcm_mem_link_t *p;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	/* no memory allocated, so return immediately */
	if (!osh || !addr || !size)
		return;

	NdisAcquireSpinLock(&(osh->malloc_lock));

	ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

	p = (bcm_mem_link_t *)((int8*)addr - sizeof(bcm_mem_link_t));
	/* If there is a double free, we should see deadbeef (p->size == 0xefbeadde)
	   as it is written all over the header. We also check for values of p->size > 64K
	   as a further light-weight validation.
	 */
	if (p->size != size) {
		printf("%s: dealloc size 0x%x does not match alloc size 0x%x on addr %p"
			" at line %d file %s\n",
			__FUNCTION__, size, p->size, addr, line, file);
		NdisReleaseSpinLock(&(osh->malloc_lock));
		ASSERT(p->size == size);
		return;
	}

	if (p->size > MEMDBG_SIZE_LIMIT) {
		printf("%s: free on addr %p size %d > MEMDBG_SIZE_LIMIT at line %d file %s\n",
			__FUNCTION__, addr, size, line, file);
		NdisReleaseSpinLock(&(osh->malloc_lock));
		ASSERT(p->size < MEMDBG_SIZE_LIMIT);
		return;
	}

	/* unlink this block */
	if (p->prev)
		p->prev->next = p->next;
	if (p->next)
		p->next->prev = p->prev;
	if (osh->dbgmem_list == p)
		osh->dbgmem_list = p->next;
	p->next = p->prev = NULL;

	osl_mfree(osh, (char *)p, size + sizeof(bcm_mem_link_t));
	NdisReleaseSpinLock(&(osh->malloc_lock));
}

int
osl_debug_memdump(osl_t *osh, struct bcmstrbuf *b)
{
	bcm_mem_link_t *p;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	bcm_bprintf(b, "   Address\tSize\tFile:line\n");
	for (p = osh->dbgmem_list; p; p = p->next)
		bcm_bprintf(b, "%p\t%5d\t%s:%d\n",
			p + 1, p->size, p->file, p->line);

	return 0;
}

#endif  /* BCMDBG_MEM */

void*
osl_malloc(osl_t *osh, uint size)
{
	NDIS_STATUS status;
	void *addr;

#if !(defined(BCMDBG) || defined(BCMDONGLEHOST))
	NdisAcquireSpinLock(&(osh->malloc_lock));
#endif /* !(defined(BCMDBG) || defined(BCMDONGLEHOST)) */

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

#if (0>= 0x0600)
	{
		shared_info_t *shared = osh->pdev;
		addr = NdisAllocateMemoryWithTagPriority(shared->adapterhandle, size, MEMORY_TAG,
		      NormalPoolPriority);
	}
	if (!addr)
		status = NDIS_STATUS_RESOURCES;
	else
		status = NDIS_STATUS_SUCCESS;
#else /* (NDISVER < 0x0600) */
	status = NdisAllocateMemoryWithTag(&addr, size, MEMORY_TAG);
#endif 
	if (status != NDIS_STATUS_SUCCESS) {
		osh->failed++;
#if !(defined(BCMDBG) || defined(BCMDONGLEHOST))
	NdisReleaseSpinLock(&(osh->malloc_lock));
#endif /* !(defined(BCMDBG) || defined(BCMDONGLEHOST)) */
		return (NULL);
	}

	osh->malloced += size;

#if !(defined(BCMDBG) || defined(BCMDONGLEHOST))
	NdisReleaseSpinLock(&(osh->malloc_lock));
#endif /* !(defined(BCMDBG) || defined(BCMDONGLEHOST)) */

	return (addr);
}

void*
osl_mallocz(osl_t *osh, uint size)
{
	void *ptr;

	ptr = osl_malloc(osh, size);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void
osl_mfree(osl_t *osh, char *addr, uint size)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	if (!osh || !addr || !size)
		return;

#if !(defined(BCMDBG) || defined(BCMDONGLEHOST))
	NdisAcquireSpinLock(&(osh->malloc_lock));
#endif /* !(defined(BCMDBG) || defined(BCMDONGLEHOST)) */

#ifdef BCMDBG
	deadbeef(addr, size);
#endif /* BCMDBG */

	osh->malloced -= size;
	NdisFreeMemory(addr, size, 0);

#if !(defined(BCMDBG) || defined(BCMDONGLEHOST))
	NdisReleaseSpinLock(&(osh->malloc_lock));
#endif /* !(defined(BCMDBG) || defined(BCMDONGLEHOST)) */
}

uint
osl_malloced(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	if (!osh)
		return 0;
	return (osh->malloced);
}

uint
osl_malloc_failed(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	if (!osh)
		return 0;
	return (osh->failed);
}

void*
osl_malloc_native(osl_t *osh, uint size)
{
	void *addr;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

#if (0>= 0x0600)
	{
		shared_info_t *shared = osh->pdev;
		addr = NdisAllocateMemoryWithTagPriority(shared->adapterhandle, size, MEMORY_TAG,
		      NormalPoolPriority);
	}
#else /* (NDISVER < 0x0600) */
	{
		NDIS_STATUS status;

		status = NdisAllocateMemoryWithTag(&addr, size, MEMORY_TAG);
		if (status != NDIS_STATUS_SUCCESS)
			return (NULL);
	}
#endif 

	return (addr);
}

void
osl_mfree_native(osl_t *osh, char *addr, uint size)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	if (!osh || !addr || !size)
		return;

	NdisFreeMemory(addr, size, 0);
}


uint32
osl_pci_read_config(osl_t *osh, uint offset, uint size)
{
	uint32 val = 0;
	uint read;
	uint retry = PCI_CFG_RETRY;
	uint fail;
	shared_info_t *shared;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	ASSERT(size <= 4);

	shared = osh->pdev;

	fail = 0xffffffff >> (4-size)*8;

	if (offset >= PCIE_EXTCFG_OFFSET) {
#ifdef BCMDBG
		printf("Extended configuration space not yet supported\n");
#endif
		return -1;
	}

	do {
#if (0< 0x0600)
		read = NdisReadPciSlotInformation(shared->adapterhandle, shared->slot,
			offset, &val, size);
#else
		read = NdisMGetBusData(shared->adapterhandle, PCI_WHICHSPACE_CONFIG,
		       offset, &val, size);
#endif
		if (val != fail)
			break;
	} while (retry--);

#ifdef BCMDBG
	if (retry < PCI_CFG_RETRY)
		printf("PCI CONFIG READ access to %d required %d retries\n", offset,
		       (PCI_CFG_RETRY - retry));
	ASSERT(read == size);
#endif /* BCMDBG */

	return (val);
}

void
osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val)
{
	uint wrote;
	uint retry = PCI_CFG_RETRY;
	shared_info_t *shared;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	shared = osh->pdev;

	if (offset >= PCIE_EXTCFG_OFFSET) {
#ifdef BCMDBG
		printf("Extended configuration space not yet supported\n");
#endif
		return;
	}

	do {
#if (0< 0x0600)
		wrote = NdisWritePciSlotInformation(shared->adapterhandle, shared->slot,
			offset, &val, size);
#else /* (NDISVER >= 0x0600) */
		wrote = NdisMSetBusData(shared->adapterhandle, PCI_WHICHSPACE_CONFIG,
		      offset, &val, size);
#endif 

		if (offset != PCI_BAR0_WIN)
			break;

		if (osl_pci_read_config(osh, offset, size) == val)
			break;
	} while (retry--);

#ifdef BCMDBG
	if (retry < PCI_CFG_RETRY)
		printf("PCI CONFIG WRITE access to %d required %d retries\n", offset,
		       (PCI_CFG_RETRY - retry));

	ASSERT(wrote == size);
#endif /* BCMDBG */
}


#if (0< 0x0600)
void
osl_pcmcia_read_attr(osl_t *osh, uint offset, void *buf, int size)
{
	int read;
	shared_info_t *shared;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	shared = osh->pdev;

	read = NdisReadPcmciaAttributeMemory(shared->adapterhandle, offset, buf, size);

	ASSERT(read == size);
}

void
osl_pcmcia_write_attr(osl_t *osh, uint offset, void *buf, int size)
{
	int wrote;
	shared_info_t *shared;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	shared = osh->pdev;

	wrote = NdisWritePcmciaAttributeMemory(shared->adapterhandle, offset, buf, size);

	ASSERT(wrote == size);
}
#else /* (NDISVER >= 0x0600) */
void
osl_pcmcia_read_attr(osl_t *osh, uint offset, void *buf, int size)
{
	ASSERT(0);
}

void
osl_pcmcia_write_attr(osl_t *osh, uint offset, void *buf, int size)
{
	ASSERT(0);
}
#endif 

#if defined(BCMDBG_ASSERT) || defined(BCMASSERT_LOG)
#ifdef BCMASSERT_LOG
static void
osl_assert_log(char *exp, char *file, int line)
{
	char tempbuf[64];

	snprintf(tempbuf, 64, "ASSERT \"%s\" : file \"%s\", line %d\n",
		exp, file, line);

	bcm_assert_log(tempbuf);
}
#endif /* BCMASSERT_LOG */

void
osl_assert(char *exp, char *file, int line)
{
	char *basename;

	basename = strrchr(file, '\\');
	/* skip the '/' */
	if (basename)
		basename++;

	if (!basename)
		basename = file;

	printf("assertion \"%s\" failed: file \"%s\", line %d\n",
		exp, basename, line);
	printf("\n");
#ifdef BCMASSERT_LOG
	osl_assert_log(exp, basename, line);
#endif /* BCMASSERT_LOG */
#ifdef BCMDBG_ASSERT
#ifndef NOBUGCHECK
	if (g_assert_type == 0) {
		/* put a delay of 3 sec before calling KeBugCheckEx(). */
		if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
			NdisMSleep(3000000);
		}
		KeBugCheckEx(line, 0, 0, 0, 0);
	}
#endif /* NOBUGCHECK */
#endif /* BCMDBG_ASSERT */

}
#endif /* BCMDBG_ASSERT || BCMASSERT_LOG  */

char*
osl_strncpy(char *d, char *s, uint n)
{
	return (strncpy(d, s, n));
}

void*
osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align_bits, uint *alloced, dmaaddr_t *pap)
{
	char	*va;				/* kernel virtual address */
	NDIS_PHYSICAL_ADDRESS	npa;		/* physical address */
	shared_info_t *shared;
	uint alloc_size; /* Keep track of what's actually allocated */
	uint16 align = (1 << align_bits);

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	ASSERT(osh->dma_cap.addrwidth);

	/* fix up the alignment requirements first */
	if (!ISALIGNED(DMA_CONSISTENT_ALIGN, align))
		size += align;
	*alloced = size;

	shared = osh->pdev;
	alloc_size = size;

#if defined(NTDDKSIM)
	/*
	 * ntddksim restricted to PAGE_SIZE...common code more general
	 * due to alignment considerations
	 */
	if (size > PAGE_SIZE) {
		ND_TRACE(("NTDDKSIM: osl_dma_alloc_consistent() reducing size from 0x%x to"
			  " PAGE_SIZE (0x%x)\n", size, PAGE_SIZE));
		alloc_size = PAGE_SIZE;
	}
#endif /* defined (NTDDKSIM) */

#ifdef NDIS_DMAWAR
	/* MSFT-proposed WAR */
	if (osh->dma_cap.addrwidth == DMADDRWIDTH_30) {
		PHYSICAL_ADDRESS zpa;

		zpa.HighPart = 0x00000000;
		zpa.LowPart  = 0x00000000;
		npa.HighPart = 0x00000000;
		npa.LowPart  = 0x3fffffff;
		va = MmAllocateContiguousMemorySpecifyCache(alloc_size, zpa, npa, zpa, MmNonCached);
		npa = MmGetPhysicalAddress(va);
	} else if (osh->dma_cap.addrwidth == DMADDRWIDTH_26) {
		PHYSICAL_ADDRESS zpa;

		zpa.HighPart = 0x00000000;
		zpa.LowPart = 0x00000000;
		npa.HighPart = 0x00000000;
		npa.LowPart = 0x03ffffff;
		va = MmAllocateContiguousMemorySpecifyCache(alloc_size, zpa, npa, zpa, MmNonCached);
		npa = MmGetPhysicalAddress(va);
	} else
#endif /* NDIS_DMAWAR */
		NdisMAllocateSharedMemory(shared->adapterhandle, alloc_size, FALSE,
		                          (void **) &va, &npa);

#ifdef NDIS_DMAWAR
	/* Make sure that we got valid address */
	ASSERT(osl_dmaddr_valid(osh, npa.LowPart, npa.HighPart));
#else
	/* Fail if WAR is not being used and address is not valid */
	if (!(osl_dmaddr_valid(osh, npa.LowPart, npa.HighPart))) {
		NdisMFreeSharedMemory(shared->adapterhandle, alloc_size, FALSE, (PVOID) va, npa);
		*pap = NULL;
		return NULL;
	}
#endif /* NDIS_DMAWAR */

	PHYSADDRLOSET(*pap, npa.LowPart);
	PHYSADDRHISET(*pap, npa.HighPart);

	return va;
}

void*
osl_dma_alloc_consistent_force32(
	osl_t *osh,
	uint size,
	uint16 align_bits,
	uint *alloced,
	dmaaddr_t *pap)
{
	char	*va;				/* kernel virtual address */
	NDIS_PHYSICAL_ADDRESS	npa, zpa;	/* physical address */
	shared_info_t *shared;
	uint alloc_size; /* Keep track of what's actually allocated */
	uint16 align = (1 << align_bits);

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	ASSERT(osh->dma_cap.addrwidth);

	/* fix up the alignment requirements first */
	if (!ISALIGNED(DMA_CONSISTENT_ALIGN, align))
		size += align;
	*alloced = size;

	shared = osh->pdev;
	alloc_size = size;

#if defined(NTDDKSIM)
	/*
	 * ntddksim restricted to PAGE_SIZE...common code more general
	 * due to alignment considerations
	 */
	if (size > PAGE_SIZE) {
		ND_TRACE(("NTDDKSIM: osl_dma_alloc_consistent() reducing size from 0x%x to"
			  " PAGE_SIZE (0x%x)\n", size, PAGE_SIZE));
		alloc_size = PAGE_SIZE;
	}
#endif /* defined (NTDDKSIM) */

	/* MSFT-proposed WAR to allocate in lower 32-bit region */
	zpa.HighPart = 0x00000000;
	zpa.LowPart  = 0x00000000;
	npa.HighPart = 0x00000000;

	if (osh->dma_cap.addrwidth == DMADDRWIDTH_30) {
		npa.LowPart  = 0x3fffffff;
	} else if (osh->dma_cap.addrwidth == DMADDRWIDTH_26) {
		npa.LowPart = 0x03ffffff;
	} else {
		npa.LowPart  = 0xfbffffff;
	}

	va = MmAllocateContiguousMemorySpecifyCache(alloc_size, zpa, npa, zpa, MmNonCached);
	npa = MmGetPhysicalAddress(va);

	/* Make sure that we got valid address */
	ASSERT(osl_dmaddr_valid(osh, npa.LowPart, npa.HighPart));

	PHYSADDRLOSET(*pap, npa.LowPart);
	PHYSADDRHISET(*pap, npa.HighPart);

	return va;
}

void
osl_dma_free_consistent(osl_t *osh, void *va, uint size, dmaaddr_t pa)
{
	NDIS_PHYSICAL_ADDRESS npa;
	shared_info_t *shared;
	uint free_size;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	shared = osh->pdev;

	npa.LowPart = PHYSADDRLO(pa);
	npa.HighPart = PHYSADDRHI(pa);
	free_size = size;

#if defined(NTDDKSIM)
	/*
	 * ntddksim restricted to PAGE_SIZE...common code more general
	 * due to alignment considerations
	 */
	if (size > PAGE_SIZE) {
		ND_TRACE(("NTDDKSIM: osl_dma_free_consistent() reducing size from 0x%x to PAGE_SIZE"
			  " (0x%x)\n", size, PAGE_SIZE));
		free_size = PAGE_SIZE;
	}
#endif /* defined (NTDDKSIM) */
#ifdef NDIS_DMAWAR
	if ((osh->dma_cap.addrwidth == DMADDRWIDTH_30) ||
	   (osh->dma_cap.addrwidth == DMADDRWIDTH_26))
		MmFreeContiguousMemorySpecifyCache(va, free_size, MmNonCached);
	else
#endif
		NdisMFreeSharedMemory(shared->adapterhandle, free_size, FALSE, (PVOID) va,
		                      npa);
}

void
osl_dma_free_consistent_force32(osl_t *osh, void *va, uint size, dmaaddr_t pa)
{
	NDIS_PHYSICAL_ADDRESS npa;
	shared_info_t *shared;
	uint free_size;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	shared = osh->pdev;

	npa.LowPart = PHYSADDRLO(pa);
	npa.HighPart = PHYSADDRHI(pa);
	free_size = size;

#if defined(NTDDKSIM)
	/*
	 * ntddksim restricted to PAGE_SIZE...common code more general
	 * due to alignment considerations
	 */
	if (size > PAGE_SIZE) {
		ND_TRACE(("NTDDKSIM: osl_dma_free_consistent() reducing size from 0x%x to PAGE_SIZE"
			  " (0x%x)\n", size, PAGE_SIZE));
		free_size = PAGE_SIZE;
	}
#endif /* defined (NTDDKSIM) */

	MmFreeContiguousMemorySpecifyCache(va, free_size, MmNonCached);
}

dmaaddr_t
osl_dma_map(osl_t *osh, void *va, uint size, int direction, struct lbuf *lb)
{
	dmaaddr_t pa;
	shared_info_t *shared;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	shared = osh->pdev;

	if (lb == NULL) {
		NDIS_PHYSICAL_ADDRESS npa;
		npa = MmGetPhysicalAddress(va);
		PHYSADDRLOSET(pa, npa.LowPart);
		PHYSADDRHISET(pa, npa.HighPart);
	} else {
		PHYSADDRLOSET(pa, PHYSADDRLO(lb->pa) + (ULONG) LBOFF(lb));
		PHYSADDRHISET(pa, PHYSADDRHI(lb->pa));
		shared_flush(shared, (uchar *) lb->data, pa,
			(ULONG)size, (BOOLEAN)(direction == DMA_TX));
	}

	return pa;

}

void
osl_dma_unmap(osl_t *osh, uint size, int direction, struct lbuf *lb)
{
	dmaaddr_t pa;
	uint offset;
	shared_info_t *shared;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	shared = osh->pdev;

	if (lb) {
		/* sync caches and main memory for rxh and max data */
		offset = (uint)LBOFF(lb);
		PHYSADDRLOSET(pa, (PHYSADDRLO(lb->pa) + offset));
		PHYSADDRHISET(pa, (PHYSADDRHI(lb->pa)));
		if (direction == DMA_RX)
			shared_flush(shared, (uchar *) lb->data, pa,
			             (ulong)(size - offset), FALSE);
	}
}

void*
osl_pktget(osl_t *osh, uint len, bool send)
{
	struct lbuf	*lb;
	shared_info_t *shared;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	shared = osh->pdev;

	if (len > LBDATASZ)
		return (NULL);

	if (send) {
		lb = shared_lb_get(shared, &shared->txfree);
	} else {
		lb = shared_lb_get(shared, &shared->rxfree);
	}
	if (lb)
		lb->len = len;

	return ((void*) lb);
}

void
osl_pktfree(osl_t *osh, struct lbuf	*lb, bool send)
{
	struct lbuf *next;
	shared_info_t *shared;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	ASSERT(lb);

	if (send && osh->tx_fn) {
		osh->tx_fn(osh->tx_ctx, (void *)lb, 0);
	}

	shared = osh->pdev;

	while (lb) {
		next = lb->next;
		lb->next = NULL;

		if (lb->p && send) {

			/* put original packet on the txdone list */

#if defined(BCMDONGLEHOST)
			wl_txq_lock(shared->wl);
#endif /* defined(BCMDONGLEHOST) */

			shared_enq(&shared->txdone, lb->p, FALSE);

#if defined(BCMDONGLEHOST)
			wl_txq_unlock(shared->wl);
#endif /* defined(BCMDONGLEHOST) */

			lb->p = NULL;
		}
		ASSERT(lb->p == NULL);

		shared_lb_put(shared, lb->l, lb);

		lb = next;
	}
}

void
osl_pktsetlen(osl_t *osh, struct lbuf *lb, uint len)
{
	ASSERT(len + LBOFF(lb) <= LBDATASZ);

	/* ASSERT(len >= 0); */
	lb->len = len;
	lb->tail = lb->data + len;
}

uchar*
osl_pktpush(osl_t *osh, struct lbuf *lb, uint bytes)
{
	if (bytes) {
		ASSERT(LBOFF(lb) >= bytes);

		lb->data -= bytes;
		lb->len += bytes;
	}

	return (lb->data);
}

uchar*
osl_pktpull(osl_t *osh, struct lbuf *lb, uint bytes)
{
	if (bytes) {
		ASSERT(bytes <= lb->len);

		lb->data += bytes;
		lb->len -= bytes;
	}

	return (lb->data);
}

void*
osl_pktdup(osl_t *osh, struct lbuf *lb)
{
	struct lbuf	*lbcopy;
	shared_info_t *shared;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	ASSERT(lb);

	shared = osh->pdev;

	lbcopy = shared_lb_get(shared, &shared->txfree);

	if (lbcopy) {
		lbcopy->data = lbcopy->head + LBOFF(lb);
		lbcopy->len = pktcopy(osh, lb, 0, pkttotlen(osh, lb), lbcopy->data);
		lbcopy->tail = lbcopy->data + lbcopy->len;
	}
	return (lbcopy);
}

/* Converts a native (OS) packet to Driver packet.
 * Allocates a new lbuf and copies the contents
 */
void *
osl_pkt_frmnative(osl_t *osh, ND_PKT *p)
{
	struct lbuf* lb;
	shared_info_t *sh;

	ASSERT((osh && osh->magic == OS_HANDLE_MAGIC));

	sh = osh->pdev;

	/* First convert the packet */
	if (!(lb =  shared_txlb_convert(sh, p)))
	    return NULL;

	return ((void *)lb);
}

#ifdef FIPS_TX_FRAGMENT
/* Converts a native (OS) packet to Driver packet.
 * Allocates a new list of lbuf and copies the contents
 * vista/win7 FIPS: Miscrosoft doesn't support Tx fragmentation
 * in safemode now. When they support it, this function will be
 * needed to handle the Tx fragmentation.
 */
void *
osl_pktlist_frmnative(osl_t *osh, ND_PKT *p)
{
	struct lbuf* lb;
	shared_info_t *sh;

	ASSERT((osh && osh->magic == OS_HANDLE_MAGIC));

	sh = osh->pdev;

	/* First convert the packet */
	if (!(lb =  shared_txlblist_convert(sh, p)))
	    return NULL;

	return ((void *)lb);
}
#endif /* FIPS_TX_FRAGMENT */

#if (0< 0x0600)
/* Converts a driver packet to native packet.
 * Allocates a PNDIS_PACKET and copies the contents from driver
 */
PNDIS_PACKET
osl_pkt_tonative(osl_t *osh, struct lbuf* lb)
{
	shared_info_t *sh;
	PNDIS_PACKET p;
	PNDIS_BUFFER b;
	NDIS_STATUS status;

	ASSERT((osh && osh->magic == OS_HANDLE_MAGIC));

	sh = osh->pdev;

	/* alloc ndis packet descriptor */
	NdisAllocatePacket(&status, &p, sh->rxpacketpool);
	if (NDIS_ERROR(status)) {
		NdisWriteErrorLogEntry(sh->adapterhandle,
		                       NDIS_ERROR_CODE_OUT_OF_RESOURCES, 1, 17);
		shared_lb_put(sh, &sh->rxfree, lb);
		return NULL;
	}

	NdisAllocateBuffer(&status, &b, sh->rxbufferpool, lb->data, lb->len);

	if (NDIS_ERROR(status)) {
		NdisWriteErrorLogEntry(sh->adapterhandle,
		                       NDIS_ERROR_CODE_OUT_OF_RESOURCES, 1, 18);
		shared_lb_put(sh, &sh->rxfree, lb);
		NdisFreePacket(p);
		return NULL;
	}

	/* attach the buffer to the packet */
	NdisChainBufferAtFront(p, b);
	NdisQueryPacket(p, NULL, NULL, &b, NULL);

	lb->p = p;
	NEXTP(p) = (PNDIS_PACKET) lb;

	/* set all the packet attributes */
	NDIS_SET_PACKET_HEADER_SIZE(p, ETHER_HDR_LEN);
	NdisSetPacketFlags(p, 0);

	/* Set the priority of the pkt */
	NDIS_PER_PACKET_INFO_FROM_PACKET(p, Ieee8021pPriority) = (PVOID)(uintptr)PKTPRIO(lb);

	return (p);
}


#else /* (NDISVER >= 0x0600) */

/* Converts a driver packet to native packet.
 * Allocates a PNET_BUFFER_LIST and copies the contents from driver
 */
PNET_BUFFER_LIST
osl_pkt_tonative(osl_t *osh, struct lbuf* lb)
{
	shared_info_t *sh;
	PNET_BUFFER_LIST p;
	PNET_BUFFER nb;
	PMDL b;
	NDIS_STATUS status;
#if defined(WL_NDWDI)
	struct ndwdi_txrx *txrx_ctxt;
	WDI_FRAME_METADATA *frame_meta;
	ndwdi_priv_frame_meta_t *priv_frame_meta;
#endif /* defined(WL_NDWDI) */

	ASSERT((osh && osh->magic == OS_HANDLE_MAGIC));

	sh = osh->pdev;
	/* alloc MDL */
	b = NdisAllocateMdl(sh->adapterhandle, lb->data, lb->len);
	if (b == NULL) {
		NdisWriteErrorLogEntry(sh->adapterhandle,
		                       NDIS_ERROR_CODE_OUT_OF_RESOURCES, 1, 17);
		goto free_lb;
	}

	/* alloc NET_BUFFER_LIST */
	p = NdisAllocateNetBufferAndNetBufferList(sh->rxpacketpool, 0, 0, b, 0, 0);
	if (p == NULL) {
		NdisWriteErrorLogEntry(sh->adapterhandle,
		      NDIS_ERROR_CODE_OUT_OF_RESOURCES, 1, 18);
		goto free_Mdl;
	}

#if defined(WL_NDWDI)
	txrx_ctxt = sh->txrx_ctxt;

	/* Allocate NBL meta-data */
	METADATA(p) = txrx_ctxt->datapath_api.AllocateWiFiFrameMetaData(txrx_ctxt->datapath_handle);
	if (METADATA(p) == NULL) {
		NdisWriteErrorLogEntry(sh->adapterhandle,
			NDIS_ERROR_CODE_OUT_OF_RESOURCES, 1, 19);
		goto free_nbl;
	}

	/* Init NBL meta-data */
	LBUF(p) = lb;
	frame_meta = METADATA(p);
	frame_meta->pNBL = p;
	frame_meta->u.rxMetaData.PayloadType = WDI_FRAME_MSDU;
	priv_frame_meta = PRIV_METADATA(p);
	priv_frame_meta->signature = MEMORY_TAG;
#endif /* defined(WL_NDWDI) */

	lb->p = p;

	NEXTP(p) = (PNET_BUFFER_LIST) lb;

	nb = NET_BUFFER_LIST_FIRST_NB(p);
	NET_BUFFER_DATA_LENGTH(nb) = lb->len;
	NET_BUFFER_LIST_NEXT_NBL(p) = NULL;

#ifdef EXT_STA
#endif /* EXT_STA */

	/* Set the priority of the pkt */
	NET_BUFFER_LIST_INFO(p, Ieee8021QNetBufferListInfo) = (PVOID)(uintptr)PKTPRIO(lb);

	return (p);

#if defined(WL_NDWDI)
free_nbl:
	NdisFreeNetBufferList(p);
#endif /* defined(WL_NDWDI) */
free_Mdl:
	NdisFreeMdl(b);
free_lb:
	shared_lb_put(sh, &sh->rxfree, lb);
	return NULL;
}
#endif 



/*
 * OSLREGOPS specifies the use of osl_XXX routines to be used for register access
 */
#if defined(OSLREGOPS)
void
osl_regops_set(osl_t *osh, osl_rreg_fn_t rreg, osl_wreg_fn_t wreg, void *ctx)
{
	osh->rreg_fn = rreg;
	osh->wreg_fn = wreg;
	osh->reg_ctx = ctx;
}

uint8
osl_readb(osl_t *osh, volatile uint8 *r)
{
	osl_rreg_fn_t rreg	= osh->rreg_fn;
	void *ctx		= osh->reg_ctx;

	return (uint8)((rreg)(ctx, (void*)r, sizeof(uint8)));
}

uint16
osl_readw(osl_t *osh, volatile uint16 *r)
{
	osl_rreg_fn_t rreg	= osh->rreg_fn;
	void *ctx		= osh->reg_ctx;

	return (uint16)((rreg)(ctx, (void*)r, sizeof(uint16)));
}

uint32
osl_readl(osl_t *osh, volatile uint32 *r)
{
	osl_rreg_fn_t rreg	= osh->rreg_fn;
	void *ctx		= osh->reg_ctx;

	return (uint32)((rreg)(ctx, (void*)r, sizeof(uint32)));
}

void
osl_writeb(osl_t *osh, volatile uint8 *r, uint8 v)
{
	osl_wreg_fn_t wreg	= osh->wreg_fn;
	void *ctx		= osh->reg_ctx;

	((wreg)(ctx, (void*)r, v, sizeof(uint8)));
}

void
osl_writew(osl_t *osh, volatile uint16 *r, uint16 v)
{
	osl_wreg_fn_t wreg	= osh->wreg_fn;
	void *ctx		= osh->reg_ctx;

	((wreg)(ctx, (void*)r, v, sizeof(uint16)));
}

void
osl_writel(osl_t *osh, volatile uint32 *r, uint32 v)
{
	osl_wreg_fn_t wreg	= osh->wreg_fn;
	void *ctx		= osh->reg_ctx;

	((wreg)(ctx, (void*)r, v, sizeof(uint32)));
}
#endif /* OSLREGOPS */

uint32
osl_systemuptime(void)
{
#if (0< 0x0600)
	ULONG ret;

	NdisGetSystemUpTime(&ret);

	return ret;
#else
	LARGE_INTEGER ret;

	NdisGetSystemUpTimeEx(&ret);

	return (ret.LowPart);
#endif 
}

uint64
osl_systemuptime_us(void)
{
	LARGE_INTEGER time;

	/* following call return time in 100ns units */
	NdisGetCurrentSystemTime(&time);

	return (uint64)(time.QuadPart/10);
}

uint32
osl_rand(void)
{
	uint32 x, hi, lo, t;

	x = OSL_SYSUPTIME();
	hi = x / 127773;
	lo = x % 127773;
	t = 16807 * lo - 2836 * hi;
	if (t <= 0) t += 0x7fffffff;
	return t;
}
