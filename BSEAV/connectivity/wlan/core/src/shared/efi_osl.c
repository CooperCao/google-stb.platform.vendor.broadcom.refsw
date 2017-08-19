/*
 * EFI (Extensible Firmware Interface) OS Independent Layer
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <pcicfg.h>
#include <proto/ethernet.h>
#include <stdarg.h>

/* Translate bcmerrors into Efi errors */
#define OSL_BCMERRORMAP	EfiErrorMap

const STATIC EFI_STATUS EfiErrorMap[] =
{	EFI_SUCCESS,			/* 0 */
	EFI_INVALID_PARAMETER,		/* BCME_ERROR */
	EFI_INVALID_PARAMETER,		/* BCME_BADARG */
	EFI_INVALID_PARAMETER,		/* BCME_BADOPTION */
	EFI_INVALID_PARAMETER,		/* BCME_NOTUP */
	EFI_INVALID_PARAMETER,		/* BCME_NOTDOWN */
	EFI_INVALID_PARAMETER,		/* BCME_NOTAP */
	EFI_INVALID_PARAMETER,		/* BCME_NOTSTA */
	EFI_INVALID_PARAMETER,		/* BCME_BADKEYIDX */
	EFI_INVALID_PARAMETER,		/* BCME_RADIOOFF */
	EFI_INVALID_PARAMETER,		/* BCME_NOTBANDLOCKED */
	EFI_INVALID_PARAMETER,	/* BCME_NOCLK */
	EFI_INVALID_PARAMETER,	/* BCME_BADRATESET */
	EFI_INVALID_PARAMETER,	/* BCME_BADBAND */
	EFI_BAD_BUFFER_SIZE,		/* BCME_BUFTOOSHORT */
	EFI_BAD_BUFFER_SIZE,		/* BCME_BUFTOOLONG */
	EFI_NOT_READY,		/* BCME_BUSY */
	EFI_INVALID_PARAMETER,	/* BCME_NOTASSOCIATED */
	EFI_INVALID_PARAMETER,	/* BCME_BADSSIDLEN */
	EFI_INVALID_PARAMETER,	/* BCME_OUTOFRANGECHAN */
	EFI_INVALID_PARAMETER,	/* BCME_BADCHAN */
	EFI_NO_MAPPING,	/* BCME_BADADDR */
	EFI_OUT_OF_RESOURCES,	/* BCME_NORESOURCE */
	EFI_UNSUPPORTED,	/* BCME_UNSUPPORTED */
	EFI_BAD_BUFFER_SIZE,	/* BCME_BADLENGTH */
	EFI_INVALID_PARAMETER,		/* BCME_NOTREADY */
	EFI_ACCESS_DENIED,		/* BCME_NOTPERMITTED */
	EFI_OUT_OF_RESOURCES,	/* BCME_NOMEM */
	EFI_INVALID_PARAMETER,	/* BCME_ASSOCIATED */
	EFI_INVALID_PARAMETER,	/* BCME_RANGE */
	EFI_INVALID_PARAMETER,	/* BCME_NOTFOUND */
	EFI_INVALID_PARAMETER,		/* BCME_WME_NOT_ENABLED */
	EFI_INVALID_PARAMETER,		/* BCME_TSPEC_NOTFOUND */
	EFI_INVALID_PARAMETER,		/* BCME_ACM_NOTSUPPORTED */
	EFI_INVALID_PARAMETER,		/* BCME_NOT_WME_ASSOCIATION */
	EFI_INVALID_PARAMETER,		/* BCME_SDIO_ERROR */
	EFI_INVALID_PARAMETER,		/* BCME_DONGLE_DOWN */
	EFI_UNSUPPORTED,		/* BCME_VERSION */
	EFI_INVALID_PARAMETER,		/* BCME_TXFAIL */
	EFI_INVALID_PARAMETER,		/* BCME_RXFAIL */
	EFI_INVALID_PARAMETER,		/* BCME_NODEVICE */
	EFI_INVALID_PARAMETER,		/* BCME_NMODE_DISABLED */
	EFI_INVALID_PARAMETER,		/* BCME_NONRESIDENT */
	EFI_INVALID_PARAMETER,		/* BCME_SCANREJECT */
	EFI_INVALID_PARAMETER,		/* BCME_USAGE_ERROR */
	EFI_INVALID_PARAMETER,		/* BCME_IOCTL_ERROR */
	EFI_INVALID_PARAMETER,		/* BCME_SERIAL_PORT_ERR */
	EFI_UNSUPPORTED,		/* BCME_DISABLED */
	EFI_ABORTED,			/* BCME_DECERR */
	EFI_ABORTED,			/* BCME_ENCERR */
	EFI_CRC_ERROR	,		/* BCME_MICERR */
	EFI_PROTOCOL_ERROR,		/* BCME_REPLAY */
	EFI_NOT_FOUND,			/* BCME_IE_NOTFOUND */
	EFI_NOT_FOUND,			/* BCME_DATA_NOTFOUND */
	EFI_INVALID_PARAMETER,	/* BCME_NOT_GC */
	EFI_NOT_FOUND,			/* BCME_PRS_REQ_FAILED */
	EFI_NOT_FOUND,			/* BCME_NO_P2P_SE */
	EFI_NOT_FOUND,			/* BCME_NOA_PND */
	EFI_NOT_FOUND,			/* BCME_FRAG_Q_FAILED */
	EFI_NOT_FOUND,			/* BCME_GET_AF_FAILED */
	EFI_NOT_FOUND,			/* BCME_MSCH_NOTREADY */
	EFI_NOT_FOUND,			/* BCME_IOV_LAST_CMD */
};

/* Global ASSERT type */
uint32 g_assert_type = 0;

uint lmtest = FALSE;

#ifdef BCMDBG
#define OSL_ERROR_VAL 0x0001
#define OSL_TRACE_VAL 0x0002

STATIC uint32 osl_msg_level = OSL_ERROR_VAL;
#endif /* BCMDBG */

/* osl_msg_level is a bitvector with defs in wlioctl.h */
#ifdef	BCMDBG
#define	OSL_ERR(args)		do {if (osl_msg_level & OSL_ERROR_VAL) printf args;} while (0)
#define	OSL_TRACE(args)		do {if (osl_msg_level & OSL_TRACE_VAL) printf args;} while (0)
#else
#ifdef BCMDBG_ERR
#define	OSL_ERR(args)		do {if (osl_msg_level & OSL_ERROR_VAL) printf args;} while (0)
#else
#define	OSL_ERR(args)
#endif /* BCMDBG_ERR */
#define	OSL_TRACE(args)
#endif /* BCMDBG */

#define BPP  (EFI_PAGE_SIZE / LBUFSZ)			/* number of buffers per page */
#define LBPP (EFI_PAGE_SIZE / sizeof(struct lbuf))	/* number of lbuf per page */
#define NPGS 1 /* Number of pages used by Allocate/FreeBuffer */

#define PCI_CFG_RETRY		10

STATIC EFI_STATUS osl_allocpage(IN osl_t *osh,
                                IN BOOLEAN shared_mem,
                                IN BOOLEAN cached,
                                IN uint dir,
                                OUT page_t *page);

STATIC EFI_STATUS osl_lblist_addpage(IN struct lbfree *l,
                                     IN BOOLEAN piomode,
                                     page_t *page,
                                     IN uint ipp,
                                     IN uint lbdatasz);

STATIC EFI_STATUS osl_lblist_alloc(IN osl_t *osh,
                                   IN struct lbfree *l,
                                   IN uint total,
                                   IN BOOLEAN shared_mem,
                                   IN BOOLEAN cached,
                                   IN BOOLEAN piomode,
                                   IN BOOLEAN data_buf,
                                   IN uint dir);

STATIC void osl_lblist_free(IN osl_t *osh,
                            IN struct lbfree *l,
                            IN BOOLEAN shared_mem,
                            IN BOOLEAN cached);
STATIC struct lbuf *osl_lb_get(IN struct lbfree *l);
STATIC void osl_lb_put(IN struct lbfree *l, IN struct lbuf *lb);
STATIC void osl_lbpool_free(IN osl_t *osh, IN BOOLEAN shared_mem);
STATIC EFI_STATUS osl_lbpool_alloc(IN osl_t *osh,
                                   IN uint tx_total,
                                   IN uint rx_total,
                                   IN BOOLEAN shared_mem,
                                   IN BOOLEAN cached,
                                   IN BOOLEAN piomode,
                                   IN BOOLEAN data_buf);

typedef struct osl_dmacap {
	uint addrwidth;   /* Maximum addressability */
	uint32 hiaddrmask; /* Mask for upper 32 bits */
	uint32 loaddrmask; /* Mask for lower 32 bits */
} osl_dmacap_t;

#define BCM_MEM_FILENAME_LEN 	24		/* Mem. filename length */

typedef struct bcm_mem_link {
	struct bcm_mem_link *prev;
	struct bcm_mem_link *next;
	size_t	size;
	int	line;
	char	file[BCM_MEM_FILENAME_LEN];
} bcm_mem_link_t;


struct osl_info {
	struct osl_pubinfo pub;			/* Bus mapping information */

	EFI_HANDLE pdev;
	EFI_PCI_IO_PROTOCOL  *PciIoIntf;	/* Handle to PCI protocol */

	/* OSL pkt callback */
	pktfree_cb_fn_t tx_fn;
	void *tx_ctx;

	uint	magic;	/* OS MAGIC number */
	uint	os_malloced;	/* Debug malloc count */
	uint	os_failed;	/* Debug malloc failed count */

	osl_dmacap_t dma_cap;

	uint ntxbuf;
	uint nrxbuf;
	BOOLEAN pktinited;	/* Flag to block packet allocation */
	lbfree_t	txfree;	/* tx packet freelist */
	lbfree_t	rxfree;	/* rx packet freelist */

	struct spktq *txdone;	/* Store transmitted OS packets */

	BOOLEAN piomode;
	bcm_mem_link_t *dbgmem_list;
};

#define EFI_PCIINT(osh) (osh)->PciIoIntf

#if defined(EFI_WINBLD)
#define ALLOCATE_ZERO_POOL(x) EfiLibAllocateZeroPool(x)
#else /* EFI_WINBLD */
#define ALLOCATE_ZERO_POOL(x) AllocateZeroPool(x)
#endif /* EFI_WINBLD */

/* OSL ATTACH/DETACH functions */
osl_t *
osl_attach(IN EFI_HANDLE pdev,
           IN uint8 *vaddr,
           IN EFI_PCI_IO_PROTOCOL *PciIoIntf,
           IN uint ntxbuf,
           IN uint nrxbuf,
           BOOLEAN piomode,
           IN struct spktq *txdone)
{
	osl_t *osh;


	/*
	 * check the cases where
	 * 1.Error code Added to bcmerror table, but forgot to add it to the OS
	 * dependent error code
	 * 2. Error code is added to the bcmerror table, but forgot to add the
	 * corresponding errorstring(dummy call to bcmerrorstr)
	 */

	bcmerrorstr(0);
	ASSERT(ABS(BCME_LAST) == (ARRAYSIZE(OSL_BCMERRORMAP) - 1));
	osh = ALLOCATE_ZERO_POOL(sizeof(osl_t));
	if (osh == NULL)
		return osh;

	osh->pdev = pdev;
	EFI_PCIINT(osh) = PciIoIntf;
	OSL_PUB(osh)->vaddr = vaddr; /* base memory pointer */
	osh->magic = OS_HANDLE_MAGIC;

	osh->ntxbuf = ntxbuf;
	osh->nrxbuf = nrxbuf;
	osh->txdone = txdone;
	osh->dbgmem_list = NULL;

	return osh;
}

void
osl_detach(osl_t *osh)
{
	if (osh == NULL)
		return;

	osl_lbpool_free(osh, (osh->piomode? FALSE: TRUE));

	gBS->FreePool(osh);
	osh = NULL;
}


/* End-of OSL ATTACH/DETACH functions */

/* OSL DMA API */
void
osl_dmaddrwidth(osl_t *osh, uint addrwidth)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	osh->dma_cap.addrwidth = addrwidth;

	switch (addrwidth) {
	case DMADDRWIDTH_30:
		ASSERT(0);
		osh->dma_cap.loaddrmask = DMADDR_MASK_30;
		osh->dma_cap.hiaddrmask = DMADDR_MASK_0;
		break;
	case DMADDRWIDTH_32:
		ASSERT(0);
		osh->dma_cap.loaddrmask = DMADDR_MASK_32;
		osh->dma_cap.hiaddrmask = DMADDR_MASK_0;
		break;
	case DMADDRWIDTH_64:
		osh->dma_cap.loaddrmask = DMADDR_MASK_32;
		osh->dma_cap.hiaddrmask = DMADDR_MASK_0;
		break;
	default:
		ASSERT(0);
	}

#ifdef BCMDBG
	OSL_TRACE(("osl_dmacap_register: 0x%x 0x%x\n", osh->dma_cap.loaddrmask,
	           osh->dma_cap.hiaddrmask));
#endif /* BCMDBG */
}

void *
osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align_bits, uint *alloced,
	ulong *pap, void **dmah)
{
	EFI_STATUS Status;
	void *HostAddress;
	UINTN NumberOfBytes;
	EFI_PHYSICAL_ADDRESS DeviceAddress;
	EFI_PCI_IO_PROTOCOL *PciIo;
	void* Map;
	uint16 align = (1 << align_bits);
	osldma_t **dma_h = (osldma_t **)dmah;

	ASSERT(osh);
	ASSERT(osh->magic == OS_HANDLE_MAGIC);
	ASSERT(pap != NULL);

	/* fix up the alignment requirements first */
	if (!ISALIGNED(DMA_CONSISTENT_ALIGN, align))
		size += align;
	*alloced = size;

	PciIo = EFI_PCIINT(osh);

	/*
	 * Allocate a common buffer from anywhere in system memory of type
	 * EfiBootServicesData.
	 */
	Status = PciIo->AllocateBuffer(PciIo,
	                               AllocateAnyPages,
	                               EfiBootServicesData,
	                               EFI_SIZE_TO_PAGES(size),
	                               &HostAddress,
	                               0);
	if (EFI_ERROR (Status)) {
		EFI_ERRMSG(Status, "osl_dma_alloc_consistent: AllocateBuffer Failed");
		return NULL;
	}

	/*
	 * Call Map() to retrieve the DeviceAddress to use for the bus master
	 * common buffer operation. If the Map() function cannot support
	 * a DMA operation for the entire length, then return an error.
	 */
	NumberOfBytes = size;
	Status = PciIo->Map(PciIo,
	                    EfiPciIoOperationBusMasterCommonBuffer,
	                    (VOID *)HostAddress,
	                    &NumberOfBytes,
	                    &DeviceAddress,
	                    &Map);

	if (!EFI_ERROR(Status) && NumberOfBytes != size) {
		PciIo->Unmap(PciIo, Map);
		Status = EFI_OUT_OF_RESOURCES;
		OSL_ERR(("Map failed. Out of resources %llu\n", NumberOfBytes));
	}

	if (EFI_ERROR(Status)) {
		PciIo->FreeBuffer(PciIo,
		                  EFI_SIZE_TO_PAGES(size),
		                  (VOID *)HostAddress);
		EFI_ERRMSG(Status, "osl_dma_alloc_consistent: DMA Mapping Failed");
		return NULL;
	}

	*dma_h = (osldma_t *)Map;

	*pap = (ulong)DeviceAddress;
	OSL_TRACE(("Physical address for device 0x%p Upper: 0x%x \r\n", (VOID *)DeviceAddress,
	           (unsigned)(DeviceAddress >> 32)));

	/* Return the virtual address */
	return HostAddress;
}

void
osl_dma_free_consistent(osl_t *osh, void *va, uint size, ulong pa, osldma_t *dmah)
{
	EFI_PCI_IO_PROTOCOL *PciIo;
	EFI_STATUS Status;
	void * Map = dmah;

	ASSERT(osh);
	ASSERT(osh->magic == OS_HANDLE_MAGIC);

	PciIo = EFI_PCIINT(osh);
	Status = PciIo->Unmap(PciIo, Map);

	if (EFI_ERROR (Status))
		EFI_ERRMSG(Status, "osl_dma_free_consistent: Unmap failed");

	Status = PciIo->FreeBuffer(PciIo,
	                           EFI_SIZE_TO_PAGES(size),
	                           (VOID *)va);

	if (EFI_ERROR (Status))
		EFI_ERRMSG(Status, "osl_dma_free_consistent: FreeBuffer failed");
}

#define LBOFF(lb) ((uintptr)lb->data - (uintptr)lb->head)

/* From EFI 1.10 specification 14.5.3 specifies that if 'Map' & 'Unmap' APIs are used
 * then there is not need for the controller to flush explicitly. However, the trick
 * here is to compute PA using known pa of the base address of the MAPPED page and calculating
 * pa offset of this LB
 */
uint
osl_dma_map(osl_t *osh, void *p)
{
	struct lbuf *lb = (struct lbuf*)p;
	ulong pa;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	pa = (ulong) lb->pa + (ulong) LBOFF(lb);

	EFI_PCIINT(osh)->Flush(EFI_PCIINT(osh));

	return pa;

}

void
osl_dma_unmap(osl_t *osh, int direction, void *p)
{
	ulong pa;
	struct lbuf *lb = (struct lbuf *)p;
	uint offset;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (lb) {
		/* sync caches and main memory for rxh and max data */
		offset = (uint)LBOFF(lb);
		pa = (ulong) (lb->pa + offset);
		if (direction == DMA_RX)
			EFI_PCIINT(osh)->Flush(EFI_PCIINT(osh));
	}
}

int
osl_busprobe(uint32 *val, uint32 addr)
{
	*val = R_REG(NULL, (volatile uint32 *)(uintptr)addr);

	return 0;
}
/* End-of OSL DMA API */

/* OSL PKT Allocation Private Library */
EFI_STATUS
osl_pktinit(osl_t *osh)
{
	EFI_STATUS Status;

	ASSERT(!osh->pktinited);

	Status = osl_lbpool_alloc(osh, osh->ntxbuf, osh->nrxbuf, !osh->piomode,
	                          TRUE, osh->piomode, TRUE);

	if (!EFI_ERROR(Status)) {
		osh->pktinited = TRUE;
	} else
		EFI_ERRMSG(Status, "lbpool Alloc failed");

	return Status;
}

/* allocate one page of cached, shared (dma-able) or regular memory */
STATIC EFI_STATUS
osl_allocpage(
	IN osl_t *osh,
	IN BOOLEAN shared_mem,
	IN BOOLEAN cached,
	IN uint dir,
	OUT page_t *page
)
{
	EFI_STATUS Status;
	EFI_PCI_IO_PROTOCOL *PciIo;

	/* uncached not supported */
	ASSERT(cached);

	bzero(page, sizeof(page_t));

	if (shared_mem) {
		if (osh->dma_cap.addrwidth == DMADDRWIDTH_30) {
			ASSERT(0);
		} else {
			UINTN NumberOfBytes;

			PciIo = EFI_PCIINT(osh);

			Status = PciIo->AllocateBuffer(PciIo,
			                               AllocateAnyPages,
			                               EfiBootServicesData,
			                               NPGS,
			                               (VOID **)&page->va,
			                               0);
			if (EFI_ERROR (Status)) {
				EFI_ERRMSG(Status, "osl_allocpage: AllocateBuffer Failed");
				return EFI_OUT_OF_RESOURCES;
			}

			/*
			 * Call Map() to retrieve the DeviceAddress to use for the bus master
			 * common buffer operation. If the Map() function cannot support
			 * a DMA operation for the entire length, then return an error.
			 *
			 * Direction helps here in EFI as you can specify the kind of operation a
			 * DMA master is going to use to access this buffer
			 */
			NumberOfBytes = EFI_PAGE_SIZE;
			Status = PciIo->Map(PciIo,
			                    (dir == DMA_TX)? EfiPciIoOperationBusMasterRead:
			                    EfiPciIoOperationBusMasterWrite,
			                    (VOID *)page->va,
			                    &NumberOfBytes,
			                    &page->pa,
			                    &page->PageMap.Map);

			if (!EFI_ERROR(Status) && NumberOfBytes != EFI_PAGE_SIZE) {
				PciIo->Unmap(PciIo, page->PageMap.Map);
				Status = EFI_OUT_OF_RESOURCES;
				OSL_ERR(("osl_allocpage: Map failed. Out of resources %llu\n",
				         NumberOfBytes));
			}

			if (EFI_ERROR(Status)) {
				PciIo->FreeBuffer(PciIo,
				                  NPGS,
				                  (VOID *)page->va);
				EFI_ERRMSG(Status, "osl_allocpage: Page Mapping Failed");
				return Status;
			}

		}
	} else
		page->va = ALLOCATE_ZERO_POOL(EFI_PAGE_SIZE);

	if (page->va == NULL) {
		OSL_ERR(("osl_allocpage: out of memory, malloced %d bytes\n", MALLOCED(osh)));
		return (EFI_OUT_OF_RESOURCES);
	}
	ASSERT(!shared_mem || ISALIGNED((uintptr)page->va, EFI_PAGE_SIZE));

	ASSERT(ISALIGNED(page->pa, EFI_PAGE_SIZE));

	Status = EFI_SUCCESS;

	return (Status);
}

/* Allocate freelist of lbufs */
STATIC EFI_STATUS
osl_lblist_alloc(
	IN osl_t *osh,
	IN struct lbfree *l,
	IN uint total,
	IN BOOLEAN shared_mem,
	IN BOOLEAN cached,
	IN BOOLEAN piomode,
	IN BOOLEAN data_buf,
	IN uint dir
)
{
	EFI_STATUS status;
	page_t page;
	int maxpages;
	int i;
	uint ipp, lbdatasz;

	/* uncached not supported */
	ASSERT(cached);

	if (data_buf)
		total = ROUNDUP(total, BPP);
	else
		/* add one if LBPP is not page aligned */
		total = ROUNDUP(total, (LBPP + ((EFI_PAGE_SIZE % sizeof(struct lbuf)) ? 1 : 0)));

	OSL_TRACE(("osl_lblist_alloc: total %d\n", total));

	l->free = NULL;
	l->total = total;
	l->count = 0;
	l->size = data_buf ? LBUFSZ : sizeof(struct lbuf);
	l->pages = NULL;
	l->npages = 0;

	maxpages = (l->total * l->size) / EFI_PAGE_SIZE;

	/* allocate page list memory */
	if ((l->pages = (page_t*) ALLOCATE_ZERO_POOL(maxpages * sizeof(page_t))) == NULL)
		goto enomem;

	/* set item per page number and data size */
	if (data_buf) {
		ipp = BPP;
		lbdatasz = LBDATASZ;
	} else {
		ipp = LBPP;
		lbdatasz = 0;
	}

	/* fill the freelist */
	for (i = 0; i < maxpages; i++) {
		status = osl_allocpage(osh, shared_mem, cached, dir, &page);
		if (EFI_ERROR(status)) {
			EFI_ERRMSG(status, "osl_lblist_alloc: osl_allocpage failed\n");
			goto enomem;
		}
		status = osl_lblist_addpage(l, piomode, &page, ipp, lbdatasz);
		if (EFI_ERROR(status)) {
			EFI_ERRMSG(status, "osl_lblist_alloc: shared_addpage failed\n");
			goto enomem;
		}
	}

	return (EFI_SUCCESS);

enomem:
	OSL_ERR(("osl_lblist_alloc: out of memory, malloced %d bytes\n", MALLOCED(osh)));
	osl_lblist_free(osh, l, shared_mem, TRUE);
	return (EFI_OUT_OF_RESOURCES);
}

/* Free a freelist of lbufs */
STATIC void
osl_lblist_free(
	IN osl_t *osh,
	IN struct lbfree *l,
	IN BOOLEAN shared_mem,
	IN BOOLEAN cached
)
{
	uint i;
	int maxpages;

	OSL_TRACE(("osl_lblist_free %p\n", l));

	ASSERT(l->count <= l->total);
	/* uncached not supported */
	ASSERT(cached);

	/* free all buffer memory pages */
	for (i = 0; i < l->npages; i++) {
		if (!shared_mem)
			gBS->FreePool(l->pages[i].va);
		else {
			if (osh->dma_cap.addrwidth == DMADDRWIDTH_30)
				ASSERT(0);
			else {
				EFI_PCI_IO_PROTOCOL *PciIo;
				EFI_STATUS Status;

				PciIo = EFI_PCIINT(osh);
				Status = PciIo->Unmap(PciIo, l->pages[i].PageMap.Map);

				if (EFI_ERROR(Status))
					EFI_ERRMSG(Status, "osl_lbfree: Unmap failed");

				Status = PciIo->FreeBuffer(PciIo,
				                           NPGS,
				                           (VOID *)l->pages[i].va);

				if (EFI_ERROR (Status))
					EFI_ERRMSG(Status, "osl_lbfree: FreeBuffer failed");
			}
		}
	}

	/* free the page list */
	if (l->pages) {
		maxpages = (l->total * l->size) / EFI_PAGE_SIZE;
		gBS->FreePool((uchar *) l->pages);
	}

	l->free = NULL;
	l->total = 0;
	l->count = 0;
	l->pages = NULL;
	l->npages = 0;
}

/* add a page worth of packets to a freelist */
STATIC EFI_STATUS
osl_lblist_addpage(
	IN struct lbfree *l,
	IN BOOLEAN piomode,
	page_t *page,
	IN uint ipp,
	IN uint lbdatasz
)
{
	struct lbuf *lb;
	uint8 *va;
	ulong pa;
	uint i;

	ASSERT(l->npages < (ROUNDUP(l->total, ipp) / ipp));


	/* add it to the page list */
	l->pages[l->npages] = *page;
	l->npages++;

	/*
	 * Split each page into one or more LBUFSZ chunks,
	 * link them together, and put each on the freelist.
	 */

	va = page->va;
	pa = (uint32)page->pa;

	for (i = 0; i < ipp; i++) {
		char *vbuf = NULL;
		uint pbuf = 0;
		lb = (struct lbuf*)(va + lbdatasz);
		if (lbdatasz) {
			vbuf = (char *)va;
			pbuf = pa;
		}

		/* initialize lbuf fields */
		lb->link = lb->next = NULL;
		lb->head = lb->data = lb->tail = (uchar *)vbuf;
		lb->end = (uchar*)lb;
		lb->len = 0;
		lb->pa = piomode? 0xdeadbeef: pbuf;
		lb->p = NULL;
		lb->l = l;

		/* put it on the freelist */
		osl_lb_put(l, lb);

		if (lbdatasz) {
			va += LBUFSZ;
			pa += LBUFSZ;
		} else
			va += sizeof(struct lbuf);
	}

	return (EFI_SUCCESS);
}

/* Get one lbuf from a free list */
STATIC
struct lbuf *
osl_lb_get(IN struct lbfree *l)
{
	struct lbuf	*lb;

	ASSERT(l->count <= l->total);

	if ((lb = l->free) != NULL) {
		l->free = lb->link;
		lb->link = lb->next = NULL;
		lb->data = lb->tail = lb->head;
		lb->priority = 0;
		lb->len = 0;
		l->count--;
		bzero(lb->pkttag, OSL_PKTTAG_SZ);
	}

	return (lb);
}

/* Return lbuf to a freelist */
STATIC void
osl_lb_put(
	IN struct lbfree *l,
	IN struct lbuf *lb
)
{
	ASSERT(l->count <= l->total);
	ASSERT(lb->p == NULL);
	ASSERT(lb->link == NULL);
	ASSERT(lb->next == NULL);
	ASSERT(lb->l == l);

	lb->data = lb->tail = (uchar*)(uintptr)0xdeadbeef;
	lb->len = 0;
	lb->link = l->free;
	l->free = lb;
	l->count++;
}

/* Allocate and populate the freelists for lbufs for a driver */
STATIC EFI_STATUS
osl_lbpool_alloc(IN osl_t *osh,
                 IN uint tx_total,
                 IN uint rx_total,
                 IN BOOLEAN shared_mem,
                 IN BOOLEAN cached,
                 IN BOOLEAN piomode,
                 IN BOOLEAN data_buf)
{
	EFI_STATUS status;

	OSL_TRACE(("osl_lblist_alloc for txfree %p\n", &osh->txfree));
	status = osl_lblist_alloc(osh, &osh->txfree, tx_total, shared_mem, cached,
	                          piomode, data_buf, DMA_TX);
	if (EFI_ERROR(status)) {
		EFI_ERRMSG(status, "osl_lbpool_alloc failed for txfree list\n");
		return status;
	}

	OSL_TRACE(("osl_lblist_alloc for rxfree %p\n", &osh->rxfree));
	status = osl_lblist_alloc(osh, &osh->rxfree, rx_total, shared_mem, cached,
	                          piomode, data_buf, DMA_RX);

	if (EFI_ERROR(status))
		EFI_ERRMSG(status, "osl_lbpool_alloc failed for rxfree list\n");

	return status;
}

/* Free the freelists for lbufs for a driver */
STATIC void
osl_lbpool_free(IN osl_t *osh, IN BOOLEAN shared_mem)
{
	osl_lblist_free(osh, &osh->txfree, shared_mem, TRUE);
	osl_lblist_free(osh, &osh->rxfree, shared_mem, TRUE);
}

/* End-of OSL PKT Allocation Private Library */

/* OSL PKT Allocation API */
void
osl_pktfreesetcb(osl_t *osh, pktfree_cb_fn_t tx_fn, void *tx_ctx)
{
	osh->tx_fn = tx_fn;
	osh->tx_ctx = tx_ctx;
}

void*
osl_pktget(osl_t *osh, uint len, bool send)
{
	struct lbuf	*lb;
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	ASSERT(osh->pktinited);

	if (len > LBDATASZ) {
		return (NULL);
	}

	if (send)
		lb = osl_lb_get(&osh->txfree);
	else
		lb = osl_lb_get(&osh->rxfree);

	if (lb)
		lb->len = len;

	return ((void*) lb);
}

void
osl_pktfree(osl_t *osh, void *p, bool send)
{
	struct lbuf	*lb = (struct lbuf *)p;
	struct lbuf *next;

	ASSERT(osh->pktinited);
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	ASSERT(lb);

	if (send && osh->tx_fn)
		osh->tx_fn(osh->tx_ctx, (void *)lb, 0);

	while (lb) {
		next = lb->next;
		lb->next = NULL;

		ASSERT((lb->l == &osh->txfree) || (lb->l == &osh->rxfree));

		if (lb->p && send) {
			ASSERT(osh->txdone);
			/* pktq being full is very serious especially on the
			 * transmit side as it's handed by upper-layer code
			 * that is expected to come back for it
			 */
			ASSERT(!spktq_full(osh->txdone));

			spktenq(osh->txdone, (void*)lb->p);
			lb->p = NULL;
		}

		ASSERT(lb->p == NULL);

		osl_lb_put(lb->l, lb);

		lb = next;
	}

}

void
osl_pktsetlen(osl_t *osh, void *p, uint len)
{
	struct lbuf *lb = (struct lbuf *)p;
	ASSERT(len + LBOFF(lb) <= LBDATASZ);

	/* ASSERT(len >= 0); */
	lb->len = len;
	lb->tail = lb->data + len;
}

void *
osl_pktpush(osl_t *osh, struct lbuf *lb, uint bytes)
{
	if (bytes) {
		ASSERT(LBOFF(lb) >= bytes);

		lb->data -= bytes;
		lb->len += bytes;
	}

	return (lb->data);
}

void *
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

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	ASSERT(lb);

	lbcopy = osl_lb_get(&osh->txfree);

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
osl_pkt_frmnative(osl_t *osh,  struct EFI_PKT *snp_pkt)
{
	struct lbuf* lb;

	ASSERT((osh && osh->magic == OS_HANDLE_MAGIC));

	if ((lb = osl_lb_get(&osh->txfree)) == NULL)
		return NULL;

	lb->len = (uint)snp_pkt->BufferSize;
	bcopy(snp_pkt->Buffer, lb->data, snp_pkt->BufferSize);
	lb->p = snp_pkt;
	lb->tail += lb->len;

	return ((void *)lb);
}

/* Converts a driver packet to native packet.
 * Allocates a EFI_PKT and copies the contents from driver
 */
struct EFI_PKT *
osl_pkt_tonative(osl_t *osh, struct lbuf* lb)
{
	struct EFI_PKT *snp_pkt;
	struct ether_header *eh;

	ASSERT((osh && osh->magic == OS_HANDLE_MAGIC));

	snp_pkt = MALLOC(osh, sizeof(struct EFI_PKT));

	if (snp_pkt == NULL)
		return NULL;

	snp_pkt->Link = NULL;
	snp_pkt->Buffer = MALLOC(osh, lb->len);
	bcopy(lb->data, snp_pkt->Buffer, lb->len);
	snp_pkt->BufferSize = lb->len;

	/* Following are being ignored for now */
	eh = (struct ether_header *)snp_pkt->Buffer;
	/*	snp_pkt->HeaderSize = MacHeaderSize; */
	snp_pkt->SrcAddr = (EFI_MAC_ADDRESS*)&eh->ether_shost;
	snp_pkt->DestAddr = (EFI_MAC_ADDRESS*) &eh->ether_dhost;
	snp_pkt->Protocol = &eh->ether_type;

	return (snp_pkt);
}
/* End-of OSL PKT Allocation API */

/* OSL PCI API */
#ifndef DHD_EFI
uint32
osl_pci_read_config(osl_t *osh, uint offset, uint size)
{
	uint32 Value32 = 0;
	EFI_STATUS Status;
	ASSERT(osh);
	ASSERT(osh->magic == OS_HANDLE_MAGIC);

	/* only 4byte access supported */
	ASSERT(size == 4);

	Status = EFI_PCIINT(osh)->Pci.Read(EFI_PCIINT(osh),
	                                   EfiPciIoWidthUint32,
	                                   offset,
	                                   PCIREADCNT,
	                                   &Value32);

	if (EFI_ERROR(Status)) {
		EFI_ERRMSG(Status, "osl_pci_read_config Failed");
		OSL_ERR(("osl_pci_read_config: offset: 0x%x FAILED", offset));
		Value32 = (uint32) -1;
	}

	return Value32;
}

void
osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val)
{
	EFI_STATUS Status;
#ifdef BCMDBG
	UINT32 Value32;
#endif /* BCMDBG */

	ASSERT(osh);
	ASSERT(osh->magic == OS_HANDLE_MAGIC);

	/* only 4byte access supported */
	ASSERT(size == 4);

	Status = EFI_PCIINT(osh)->Pci.Write(EFI_PCIINT(osh),
	                                    EfiPciIoWidthUint32,
	                                    offset,
	                                    PCIREADCNT,
	                                    &val);

	if (EFI_ERROR(Status)) {
		EFI_ERRMSG(Status, "osl_pci_write_config Failed");
		OSL_ERR(("osl_pci_write_config: offset: 0x%x value 0x%x FAILED", offset, val));
	}

#ifdef BCMDBG
	Status = EFI_PCIINT(osh)->Pci.Read(EFI_PCIINT(osh),
	                                   EfiPciIoWidthUint32,
	                                   offset,
	                                   PCIREADCNT,
	                                   &Value32);

	ASSERT(Value32 == val);
#endif /* BCMDBG */

}

#else /* DHD_EFI */

uint32
osl_pci_read_config(osl_t *osh, uint offset, uint size)
{
	uint32 Value32 = 0;
	EFI_STATUS Status;
	int retry = PCI_CFG_RETRY;

	ASSERT(osh);
	ASSERT(osh->magic == OS_HANDLE_MAGIC);

	/* only 4byte access supported */
	ASSERT(size == 4);
	do {
		Status = EFI_PCIINT(osh)->Pci.Read(EFI_PCIINT(osh),
		                                   EfiPciIoWidthUint32,
		                                   offset,
		                                   PCIREADCNT,
		                                   &Value32);

		if (EFI_ERROR(Status)) {
			EFI_ERRMSG(Status, "osl_pci_read_config Failed");
			OSL_ERR(("osl_pci_read_config: offset: 0x%x FAILED", offset));
			Value32 = (uint32) -1;
			break;
		}

		if (Value32 != 0xffffffff)
			break;

	} while (retry--);

#ifdef BCMDBG
	if (retry < PCI_CFG_RETRY)
		OSL_ERR(("PCI CONFIG READ access to %d required %d retries\n", offset,
		       (PCI_CFG_RETRY - retry)));
#endif /* BCMDBG */

	return Value32;
}


void
osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val)
{
	EFI_STATUS Status;
	int retry = PCI_CFG_RETRY;

	ASSERT(osh);
	ASSERT(osh->magic == OS_HANDLE_MAGIC);

	/* only 4byte access supported */
	ASSERT(size == 4);

	do {
		Status = EFI_PCIINT(osh)->Pci.Write(EFI_PCIINT(osh),
		                                    EfiPciIoWidthUint32,
		                                    offset,
		                                    PCIREADCNT,
		                                    &val);

		if (EFI_ERROR(Status)) {
			EFI_ERRMSG(Status, "osl_pci_write_config Failed");
			OSL_ERR(("osl_pci_write_config: offset: 0x%x value 0x%x FAILED",
					offset, val));
			break;
		}
		if (offset != PCI_BAR0_WIN)
			break;

		if (osl_pci_read_config(osh, offset, size) == val)
			break;

	} while (retry--);

#ifdef BCMDBG
	if (retry < PCI_CFG_RETRY)
		OSL_ERR(("PCI CONFIG WRITE access to %d required %d retries\n", offset,
		       (PCI_CFG_RETRY - retry)));
#endif /* BCMDBG */

}
#endif /* DHD_EFI */

int
osl_pci_set_attributes(osl_t *osh, uint enable, uint64 flags, uint64 *result)
{
	EFI_STATUS status;

	status = EFI_PCIINT(osh)->Attributes(EFI_PCIINT(osh),
		enable ? EfiPciIoAttributeOperationEnable : EfiPciIoAttributeOperationDisable,
		flags,
		result);
	if (EFI_ERROR(status)) {
		EFI_ERRMSG(status, "osl_pci_set_attributes Failed\n");
		OSL_ERR(("osl_pci_set_attributes: FAILED \n"));
		return BCME_ERROR;
	}

	return BCME_OK;
}

uint
osl_pci_bus(osl_t *osh)
{
	UINTN Seg;
	UINTN Bus;
	UINTN Device;
	UINTN Fun;
	ASSERT(osh);
	ASSERT(osh->magic == OS_HANDLE_MAGIC);

	EFI_PCIINT(osh)->GetLocation(EFI_PCIINT(osh),
	                             &Seg,
	                             &Bus,
	                             &Device,
	                             &Fun);
	return (uint)Bus;
}

uint
osl_pci_slot(osl_t *osh)
{
	UINTN Seg;
	UINTN Bus;
	UINTN Device;
	UINTN Fun;
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));

	EFI_PCIINT(osh)->GetLocation(EFI_PCIINT(osh),
	                             &Seg,
	                             &Bus,
	                             &Device,
	                             &Fun);
	return (uint)Seg;
}

/* End-of OSL PCI API */

/* Misc API */
/* translate bcmerrors into NetBSD errors */
EFI_STATUS
osl_error(int bcmerror)
{
	int abs_bcmerror;
	int array_size = ARRAYSIZE(OSL_BCMERRORMAP);

	abs_bcmerror = ABS(bcmerror);

	if (bcmerror > 0)
		abs_bcmerror = 0;

	else if (abs_bcmerror >= array_size)
		abs_bcmerror = BCME_ERROR;

	return OSL_BCMERRORMAP[abs_bcmerror];
}
/* End-of Misc API */

/* OSL Register Read/Write API */
#define EFI_REG_OP(_PciIoIntf, _Oper, _Width, _Offset, _v)		\
	(_PciIoIntf)->Mem._Oper((_PciIoIntf), (_Width), PCIBARIDX,	\
				  (_Offset), PCIREADCNT, (_v))

UINT32
osl_reg_rd32(osl_t *osh, UINT64 offset)
{
	UINT32 v;
	EFI_STATUS Status;

	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));

	Status = EFI_REG_OP(EFI_PCIINT(osh), Read,
	                    EfiPciIoWidthUint32,
	                    offset, (VOID*)(&v));

	if (EFI_ERROR(Status)) {
		EFI_ERRMSG(Status, "osl_reg_rd32 Failed");
		OSL_ERR(("osl_reg_rd32: offset 0x%llx\r\n", offset));
		v = (uint32)-1;
	}

	return v;
}

UINT16
osl_reg_rd16(osl_t *osh, UINT64 offset)
{
	UINT16 v;
	EFI_STATUS Status;

	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));

	Status = EFI_REG_OP(EFI_PCIINT(osh), Read,
	                    EfiPciIoWidthUint16,
	                    offset, (VOID*)(&v));

	if (EFI_ERROR(Status)) {
		EFI_ERRMSG(Status, "osl_reg_rd16 Failed");
		OSL_ERR(("osl_reg_rd16: offset 0x%llx\r\n", offset));
		v = (uint16)-1;
	}

	return v;
}

UINT8
osl_reg_rd8(osl_t *osh, UINT64 offset)
{
	UINT8 v;
	EFI_STATUS Status;

	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));

	Status = EFI_REG_OP(EFI_PCIINT(osh), Read,
	                    EfiPciIoWidthUint8,
	                    offset, (VOID*)(&v));

	if (EFI_ERROR(Status)) {
		EFI_ERRMSG(Status, "osl_reg_rd8 Failed");
		OSL_ERR(("osl_reg_rd8: offset 0x%llx\r\n", offset));
		v = (uint8)-1;
	}

	return v;
}

void
osl_reg_wr32(osl_t *osh, UINT64 offset,	UINT32 v)
{
	EFI_STATUS Status;
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));

	Status = EFI_REG_OP(EFI_PCIINT(osh), Write,
	                    EfiPciIoWidthUint32,
	                    offset,
	                    &v);

	if (EFI_ERROR(Status)) {
		EFI_ERRMSG(Status, "osl_reg_wr32 Failed");
		OSL_ERR(("osl_reg_wr32: offset 0x%llx value: 0x%x\r\n", offset, v));
	}

}

void
osl_reg_wr16(osl_t *osh, UINT64 offset,	UINT16 v)
{
	EFI_STATUS Status;

	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));

	Status = EFI_REG_OP(EFI_PCIINT(osh), Write,
	                    EfiPciIoWidthUint16,
	                    offset,
	                    &v);

	if (EFI_ERROR(Status)) {
		EFI_ERRMSG(Status, "osl_reg_wr16 Failed");
		OSL_ERR(("osl_reg_wr16: offset 0x%llx value: 0x%x\r\n", offset, v));
	}
}

void
osl_reg_wr8(osl_t *osh, UINT64 offset, UINT8 v)
{
	EFI_STATUS Status;

	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));

	Status = EFI_REG_OP(EFI_PCIINT(osh), Write,
	                    EfiPciIoWidthUint8,
	                    offset,
	                    &v);

	if (EFI_ERROR(Status)) {
		EFI_ERRMSG(Status, "osl_reg_wr8 Failed");
		OSL_ERR(("osl_reg_wr8: offset 0x%llx value: 0x%x\r\n", offset, v));
	}
}

/* End-of OSL Register Read/Write API */

#ifdef BCMDBG_MEM

void*
osl_debug_malloc(osl_t *osh, size_t size, int line, const char* file)
{
	bcm_mem_link_t *p;
	const char* basename;

	ASSERT(size);

	if ((p = (bcm_mem_link_t*)osl_malloc(osh, sizeof(bcm_mem_link_t) + size)) == NULL)
		return (NULL);

	p->size = size;
	p->line = line;

	basename = strrchr(file, '\\');
	/* skip the '\' (windows based compile) */
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

	return p + 1;
}

void *
osl_debug_mallocz(osl_t *osh, size_t size, int line, const char* file)
{
	void *ptr;

	ptr = osl_debug_malloc(osh, size, line, file);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void
osl_debug_mfree(osl_t *osh, void *addr, size_t size, int line, const char* file)
{
	bcm_mem_link_t *p = (bcm_mem_link_t *)((int8*)addr - sizeof(bcm_mem_link_t));

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (p->size == 0) {
		printf("osl_debug_mfree: double free on addr %p size %llu at line %d file %s\n",
			addr, size, line, file);
		ASSERT(p->size);
		return;
	}

	if (p->size != size) {
		printf("osl_debug_mfree: dealloc size %llu does not match alloc size %llu"
		       " on addr %p at line %d file %s\n",
		       size, p->size, addr, line, file);
		ASSERT(p->size == size);
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

	osl_mfree(osh, p, size + sizeof(bcm_mem_link_t));
}

int
osl_debug_memdump(osl_t *osh, struct bcmstrbuf *b)
{
	bcm_mem_link_t *p;

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	bcm_bprintf(b, "   Address\tSize\tFile:line\n");
	for (p = osh->dbgmem_list; p; p = p->next)
		bcm_bprintf(b, "0x%08x\t%5d\t%s:%d\n",
			(uintptr)p + sizeof(bcm_mem_link_t), p->size, p->file, p->line);

	return 0;
}

#endif	/* BCMDBG_MEM */

/* OSL Malloc/Free API */
void *
osl_malloc(osl_t *osh, size_t size)
{
	void *addr;
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));

#if defined(EFI_WINBLD)
	addr = EfiLibAllocatePool(size);
#else /* EFI_WINBLD */
	addr = AllocatePool(size);
#endif /* EFI_WINBLD */

	if (addr)
		osh->os_malloced += (uint)size;
	else
		osh->os_failed ++;

	return addr;
}

void *
osl_mallocz(osl_t *osh, size_t size)
{
	void *ptr;

	ptr = osl_malloc(osh, size);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void
osl_mfree(osl_t *osh, void *addr, size_t size)
{
	EFI_STATUS Status;
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));

	Status = gBS->FreePool(addr);

	osh->os_malloced -= (uint)size;
}

uint
osl_malloced(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));

	return osh->os_malloced;
}

uint
osl_malloc_failed(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));

	return osh->os_failed;
}
/* End-of OSL Malloc/Free API */

#ifdef BCMDBG_ASSERT
void
osl_assert(char *exp, char *file, int line)
{
	printf("assertion \"%s\" failed: file \"%s\", line %d\n", exp, file, line);
	printf("\n");
}
#endif /* BCMDBG_ASSERT */


/* These are EFI optimized implementations
 */
void *
memcpy(void *dest, const void *src, size_t n)
{
#if defined(EFI_WINBLD)
	EfiCopyMem((VOID *) dest, (VOID *) src, (UINTN) n);
#else /* EFI_WINBLD */
	CopyMem((VOID *) dest, (VOID *) src, (UINTN) n);
#endif /* EFI_WINBLD */
	return dest;
}

void *
memset(void *dest, int c, size_t n)
{
#if defined(EFI_WINBLD)
	EfiSetMem((VOID *) dest, (UINTN) n, (UINT8) c);
#else /* EFI_WINBLD */
	SetMem((VOID *) dest, (UINTN) n, (UINT8) c);
#endif /* EFI_WINBLD */
	return dest;
}

int
osl_strn_ascii2unicode(char *src, int len, CHAR16 *dest)
{
	int i = 0, j = 0;
	if (!src || !dest)
		return -1;

	for (i = 0; i < len; i++, j++) {
		dest[j] = (CHAR16)src[i];
	}

	dest[j] = 0;

	return 0;
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

/* below code is copied from the lib_scanf.c in cfe
* copy was required because compiling lib_scanf.c in efi
* environment resulted in weird compiler errors
*/

#define GETAP(ap) \
    ap = (char *) va_arg(args, char *);

static int
match(char *fmt, char cc)
{
	int exc = 0;
	char *cp1;
	if (!cc)
		return 0;
	if (*fmt == '^') {
		exc = 1;
		fmt++;
	}

	for (cp1 = fmt; *cp1 && *cp1 != ']';) {
		if (cp1[1] == '-' && cp1[2] > cp1[0]) {
			if (cc >= cp1[0] && cc <= cp1[2])
				return exc^1;
			cp1 += 3;
		}
		else {
			if (cc == *cp1)
				return exc^1;
			cp1++;
		}
	}
	return exc;
}


static long
asclng(char **cp, int width, int base)
{
	long answer;
	char cc, sign, *cp1;

	answer = sign = 0;
	for (cp1 = *cp; *cp1; cp1++) {
		if (*cp1 > ' ')
			break;
	}
	if (*cp1 == '-' || *cp1 == '+')
		sign = *cp1++, width--;
	if (!*cp1 || !width)
		goto exi1;
	if (!base) {
		base = 10;
		if (*cp1 == '0') {
			base = 8;
			goto lab4;
		}
	}
	else if (base == 16) {
		if (*cp1 == '0') {
	lab4:       cp1++, width--;
			if (width > 0) {
				if (*cp1 == 'x' || *cp1 == 'X')
					base = 16, cp1++, width--;
			}
		}
	}
	for (; width && *cp1; cp1++, width--) {
		if ((cc = *cp1) < '0')
			break;
		if (cc <= '9')
			cc &= 0xf;
		else {
			cc &= 0xdf;
			if (cc >= 'A')
				cc -= 'A' - 10;
		}
		if (cc >= base)
			break;
		answer = base * answer + cc;
	}

	exi1:
	*cp = cp1;
	if (sign == '-')
		answer = -answer;
	return answer;
}

int
sscanf(char *buf, char *fmt, ...)
{
	int field;		/* field flag: 0 = background, 1 = %field */
	int sizdef;		/* size: 0 = default, 1 = short, 2 = long, 3 = long double */
	int width;		/* field width */
	int par1;
	long l1;
	int nfields;
	char fch;
	char *orgbuf, *prebuf;
	char *ap;
	va_list args;

	va_start(args,fmt);	/* get variable arg list address */
	if (!*buf)
	return -1;
	nfields = field = sizdef = 0;
	orgbuf = buf;
	while ((fch = *fmt++) != 0) {
	if (!field) {
		if (fch == '%') {
		if (*fmt != '%') {
			field = 1;
			continue;
		}
		fch = *fmt++;
		}
		if (fch <= ' ')
		for (; *buf == ' ' || *buf == '\t'; buf++);
		else if (fch == *buf)
		buf++;
	}
	else {
		width = 0x7fff;
		if (fch == '*') {
		width = va_arg(args, int);
		goto lab6;
		}
		else if (fch >= '0' && fch <= '9') {
		fmt--;
		width = asclng(&fmt, 9, 10);
	lab6:		fch = *fmt++;
		}
		if (fch == 'h') {
		sizdef = 1;
		goto lab7;
		}
		else if (fch == 'l') {
		sizdef = 2;
	lab7:		fch = *fmt++;
		}

		prebuf = buf;
			switch (fch) {
		case 'd':		/* signed integer */
		par1 = 10;
		goto lab3;
		case 'o':		/* signed integer */
		par1 = 8;
		goto lab3;
		case 'x':		/* signed integer */
		case 'X':		/* long signed integer */
		par1 = 16;
		goto lab3;
		case 'u':		/* unsigned integer */
		case 'i':		/* signed integer */
		par1 = 0;
	lab3:		GETAP(ap);
		l1 = asclng(&buf, width, par1);
		if (prebuf == buf)
			break;
		if (sizdef == 2)
			*(long *)ap = l1;
		else if (sizdef == 1)
			*(short *)ap = (short)l1;
		else
			*(int *)ap = l1;
		goto lab12;
		case 'c':		/* character */
		GETAP(ap);
		for (; width && *buf; width--) {
			*ap++ = *buf++;
			if (width == 0x7fff)
			break;
		}
		goto lab12;
		case '[':		/* search set */
		GETAP(ap);
		for (; width && match(fmt, *buf); width--)
			*ap++ = *buf++;
		while (*fmt++ != ']');
		goto lab11;
		case 's':		/* character array */
		GETAP(ap);
		for (; *buf == ' ' || *buf == 0x07; buf++)
			;
		for (; width && *buf && *buf > ' '; width--)
			*ap++ = *buf++;
	lab11:		if (prebuf == buf)
			break;
		*(char *)ap = 0;
		goto lab12;
		case 'n':		/* store # chars */
		GETAP(ap);
		*(int *)ap = (int)(buf - orgbuf);
		break;
		case 'p':		/* pointer */
		GETAP(ap);
		*(long *)ap = asclng(&buf, width, 16);
	lab12:		nfields++;
		break;
		default:		/* illegal */
		goto term;
		}
		field = 0;
	}
	if (!fch)
		break;
	}
	term:
	return nfields;
}
