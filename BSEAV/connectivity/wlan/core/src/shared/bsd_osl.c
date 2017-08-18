/*
 *  NetBSD 4.0 OS Support Layer
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
/* NetBSD-specific portion of
 * Broadcom 802.11abg Networking Device Driver
 */
/*
 * Copyright (c) 1997, 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Charles M. Hannum and by Jason R. Thorpe of the Numerical Aerospace
 * Simulation Facility, NASA Ames Research Center.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell and Rick Macklem.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#define bsd_osl_c
#include <bcm_cfg.h>
#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/pool.h>
#include <typedefs.h>
#include <bcmendian.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>

#if !defined(__mips__)
#include <wlc_cfg.h>
#endif
#include <pcicfg.h>

#if defined(__FreeBSD__)
#include <dev/cardbus/cardbusvar.h>
#endif

#include <bcmbsd.h>
#if __NetBSD_Version__ >= 500000003
#include <uvm/uvm_extern.h>
#ifndef caddr_t
#define caddr_t char *
#endif
#endif /* __NetBSD_Version__ >= 500000003 */

#define BCM_MEM_FILENAME_LEN 	24	/* BCM_MEM_FILENAME_LEN */

/* Macros to get at the members of struct pci_attach_args */
#define PCI_CHIPSET(x)	(x)->businfo.pci.pa_pc 	/* PCI bus */
#define PCI_TAG(x)	(x)->businfo.pci.pa_tag	/* PCI bus tuple */
#define PCI_DMAT(x)	(x)->os_dmat	/* PCI bus tuple */

/* Macros to get at the members of struct cardbus_attach_args */
#if defined(__FreeBSD__)
#define CARDBUS_TAG(osh)	(osh)->businfo.cardbus.ca_tag
#define CARDBUS_CT(osh)		(osh)->businfo.cardbus.ca_ct
#endif

/* DMA Magic Number */
#define DMA_HANDLE_MAGIC	OS_HANDLE_MAGIC + 10

/* DMA Map structure */
struct osl_dmainfo {
	volatile struct 	osl_dmainfo *dma_next;	/* pointer to next dma map node */
	bus_dmamap_t		dma_map;		/* dma handle describing the region */
	uint 			dma_magic;		/* dma magic number */
};

#ifdef BCMDBG
typedef struct bcm_mem_link {
	struct bcm_mem_link *prev;
	struct bcm_mem_link *next;
	uint	size;
	int	line;
	char	file[BCM_MEM_FILENAME_LEN];
} bcm_mem_link_t;
#endif /* BCMDBG */

struct osl_info {
	struct osl_pubinfo pub;			/* Bus mapping information */
	uint 			os_magic;	/* OS MAGIC number */
	bus_dma_tag_t		os_dmat;	/* parent bus dma tag */
	const char 		*os_xname;	/* Net device name */
	int			os_bustype;	/* PCI or Cardbus */
	int			os_bus;		/* bus # for osl_pci_bus */
	int			os_slot;	/* slot # for osl_pci_slot */
	union {					/* Bus config space tokens */
		struct {
			pci_chipset_tag_t	pa_pc;	/* chipset */
			pcitag_t		pa_tag;	/* config space tag */
		} pci;
#if defined(__FreeBSD__)
		struct {
			cardbus_devfunc_t	ca_ct;	/* device func tag */
			cardbustag_t		ca_tag;	/* config space tag */
		} cardbus;
#endif
	} businfo;

	/* DMA mapping data */
	volatile int 		os_num_dmaps;	/* Total number of DMA maps allocated */
	volatile osldma_t 	*os_free_dmaps;	/* List head of free DMA maps */
	uint dma_addrwidth;			/* address width our core DMA supports */

	uint 			os_malloced;	/* Debug malloc count */
	uint 			os_failed;	/* Debug malloc failed count */

#ifdef BCMDBG_MEM
	bcm_mem_link_t 		*os_dbgmem_list; /* List of malloced areas */
#endif

	int os_pkttag;
	pktfree_cb_fn_t tx_fn;	  /* Callback function for PKTFREE */
	void *tx_ctx;		  /* Context to the callback function */
#ifdef MBUFTRACE
	struct mowner mbuf_mowner;
#endif
};

/* Translate bcmerrors into NetBSD errors */
#define OSL_BCMERRORMAP	NetBSDbcmerrormap

/* Global ASSERT type */
uint32 g_assert_type = 0;

static const int16 NetBSDbcmerrormap[] =
{	0, 			/* 0 */
	EINVAL,		/* BCME_ERROR */
	EINVAL,		/* BCME_BADARG */
	EINVAL,		/* BCME_BADOPTION */
	EINVAL,		/* BCME_NOTUP */
	EINVAL,		/* BCME_NOTDOWN */
	EINVAL,		/* BCME_NOTAP */
	EINVAL,		/* BCME_NOTSTA */
	EINVAL,		/* BCME_BADKEYIDX */
	EINVAL,		/* BCME_RADIOOFF */
	EINVAL,		/* BCME_NOTBANDLOCKED */
	EINVAL, 	/* BCME_NOCLK */
	EINVAL, 	/* BCME_BADRATESET */
	EINVAL, 	/* BCME_BADBAND */
	E2BIG,		/* BCME_BUFTOOSHORT */
	E2BIG,		/* BCME_BUFTOOLONG */
	EBUSY, 		/* BCME_BUSY */
	EINVAL, 	/* BCME_NOTASSOCIATED */
	EINVAL, 	/* BCME_BADSSIDLEN */
	EINVAL, 	/* BCME_OUTOFRANGECHAN */
	EINVAL, 	/* BCME_BADCHAN */
	EFAULT, 	/* BCME_BADADDR */
	ENOMEM, 	/* BCME_NORESOURCE */
	EOPNOTSUPP,	/* BCME_UNSUPPORTED */
	EMSGSIZE,	/* BCME_BADLENGTH */
	EINVAL,		/* BCME_NOTREADY */
	EPERM,		/* BCME_NOTPERMITTED */
	ENOMEM, 	/* BCME_NOMEM */
	EINVAL, 	/* BCME_ASSOCIATED */
	ERANGE, 	/* BCME_RANGE */
	EINVAL, 	/* BCME_NOTFOUND */
	EINVAL,		/* BCME_WME_NOT_ENABLED */
	EINVAL,		/* BCME_TSPEC_NOTFOUND */
	EINVAL,		/* BCME_ACM_NOTSUPPORTED */
	EINVAL,		/* BCME_NOT_WME_ASSOCIATION */
	EINVAL,		/* BCME_SDIO_ERROR */
	EINVAL,		/* BCME_DONGLE_DOWN */
	EINVAL,		/* BCME_VERSION */
	EIO,            /* BCME_TXFAIL */
	EIO,            /* BCME_RXFAIL */
	EINVAL,         /* BCME_NODEVICE */
	EINVAL,         /* BCME_NMODE_DISABLED */
	ENODATA,	/* BCME_NONRESIDENT */
	EINVAL,	        /* BCME_SCANREJECT */
	EINVAL,         /* unused */
	EINVAL,         /* unused */
	EINVAL,         /* unused */
	EOPNOTSUPP,	/* BCME_DISABLED */
	EIO,           /* BCME_DECERR */
	EIO,           /* BCME_ENCERR */
	EIO,           /* BCME_MICERR */
	ERANGE,        /* BCME_REPLAY */
	EINVAL,        /* BCME_IE_NOTFOUND */
	EINVAL,        /* BCME_IOV_LAST_CMD */
/* When an new error code is added to bcmutils.h, add os
 * spcecific error translation here as well
 */
/* check if BCME_LAST changed since the last time this function was updated */
#if BCME_LAST != -61
#error "You need to add a OS error translation in the NetBSDbcmerrormap \
	for new error code defined in bcmutils.h"
#endif /* BCME_LAST != -61 */
};

/* Local prototypes */

static void osl_free_dmamap_chain(osl_t *osh);
static osldma_t *osl_dmamap_get(osl_t *osh);
static INLINE bool osl_pkttag_add(void *p);
static INLINE void osl_pkttag_remove(void *p);

osl_t *
osl_attach(void *pdev, bool pkttag, const char *dev_name,
	int bustype, bus_dma_tag_t dmat,
	bus_space_tag_t space, bus_space_handle_t handle)
{
	osl_t *osh;

	ASSERT(bustype == OSL_CARDBUS_BUS || bustype == OSL_PCI_BUS);

	/* Check that error map has the right number of entries in it */
	ASSERT(ABS(BCME_LAST) == (ARRAYSIZE(OSL_BCMERRORMAP) - 1));

	/* Structure is zeroed on malloc by specifying the M_ZERO flag */
	osh = (osl_t *)malloc(sizeof(osl_t), M_DEVBUF, M_NOWAIT|M_ZERO);

	if (osh) {
		osh->os_magic = OS_HANDLE_MAGIC;
		osh->os_xname = dev_name;
		osh->os_bustype = bustype;
		if (osh->os_bustype == OSL_PCI_BUS) {
			struct pci_attach_args *pa =
			    (struct pci_attach_args *) pdev;
			PCI_CHIPSET(osh) = pa->pa_pc;
			PCI_TAG(osh) = pa->pa_tag;
			osh->os_bus = pa->pa_bus;
			osh->os_slot = pa->pa_device;
		}
#if defined(__FreeBSD__)
		else {
			struct cardbus_attach_args *ca =
			    (struct cardbus_attach_args *) pdev;
			CARDBUS_CT(osh) = ca->ca_ct;
			CARDBUS_TAG(osh) = ca->ca_tag;
			osh->os_bus = ca->ca_ct->ct_bus;
			osh->os_slot = ca->ca_ct->ct_func;
		}
#endif
		osh->os_dmat = dmat;

		/* Initialize pkttag structures */
		if (pkttag) {
			osh->os_pkttag = 1;
		}
		osh->pub.space = space;
		osh->pub.handle = handle;
		osh->pub.vaddr = bus_space_vaddr(space, handle);

#ifdef MBUFTRACE
		strlcpy(osh->mbuf_mowner.mo_name, dev_name,
			sizeof(osh->mbuf_mowner.mo_name));
		MOWNER_ATTACH(&osh->mbuf_mowner);
#endif
	}

	return osh;
}

void
osl_detach(osl_t *osh)
{
	if (osh == NULL)
		return;

	ASSERT(osh->os_magic == OS_HANDLE_MAGIC);

	/* Deallocate DMA map chains */
	osl_free_dmamap_chain(osh);

	free(osh, M_DEVBUF);
}

void
osl_pktfree_cb_set(osl_t *osh, pktfree_cb_fn_t tx_fn, void *tx_ctx)
{
	osh->tx_fn = tx_fn;
	osh->tx_ctx = tx_ctx;
}

static osldma_t *
osl_dmamap(osl_t *osh)
{
	osldma_t 	*cur;

	if ((cur = malloc(sizeof(osldma_t), M_DEVBUF, M_NOWAIT|M_ZERO)) == NULL)
		return NULL;

	/* Create the map */
	/* need MCLBYTES * 2 to handle AMSDU tx */
	if (bus_dmamap_create(PCI_DMAT(osh), MCLBYTES * 8, MAX_DMA_SEGS,
		MCLBYTES, PAGE_SIZE, BUS_DMA_NOWAIT, &cur->dma_map) != 0) {
			printf("%s: unable to create streaming DMA map\n",
				osh->os_xname);
		free(cur, M_DEVBUF);
		return NULL;
	}

	cur->dma_magic = DMA_HANDLE_MAGIC;

	ASSERT(osh->os_magic == OS_HANDLE_MAGIC);
	osh->os_num_dmaps++;

	return cur;
}

void
osl_dmamap_put(osl_t *osh, osldma_t  *map)
{

	ASSERT(map->dma_magic == DMA_HANDLE_MAGIC);

	map->dma_next = osh->os_free_dmaps;
	osh->os_free_dmaps = map;
}

static osldma_t *
osl_dmamap_get(osl_t *osh)
{
	osldma_t *map = NULL;

	map = (osldma_t *) (uintptr)osh->os_free_dmaps;

	if (map) {
		osh->os_free_dmaps = osh->os_free_dmaps->dma_next;
		map->dma_next = NULL;
		ASSERT(map->dma_magic == DMA_HANDLE_MAGIC);
	} else
		/* Maps are all used. Make one. */
		map = osl_dmamap(osh);

	return map;
}


static void
osl_free_dmamap_chain(osl_t *osh)
{
	osldma_t 	*cur, *tmp;
	int		ipl;

	cur = (osldma_t *)(uintptr)osh->os_free_dmaps;

	ipl = splnet();
	while (cur) {
		ASSERT(cur->dma_magic == DMA_HANDLE_MAGIC);
		bus_dmamap_destroy(PCI_DMAT(osh), cur->dma_map);
		tmp = cur;
		cur = (osldma_t *)(uintptr)cur->dma_next;
		free(tmp, M_DEVBUF);
		osh->os_num_dmaps--;
	}
	splx(ipl);
	ASSERT(osh->os_num_dmaps == 0);
}

uint32
osl_pci_read_config(osl_t *osh, uint offset, uint size)
{
	ASSERT(osh);
	ASSERT(osh->os_magic == OS_HANDLE_MAGIC);

	/* only 4byte access supported */
	ASSERT(size == 4);

	return ((uint32) pci_conf_read(PCI_CHIPSET(osh), PCI_TAG(osh), offset));
}

void
osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val)
{
	ASSERT(osh);
	ASSERT(osh->os_magic == OS_HANDLE_MAGIC);

	/* only 4byte access supported */
	ASSERT(size == 4);

	pci_conf_write(PCI_CHIPSET(osh), PCI_TAG(osh), offset, val);
}

#ifdef BCMDBG_MEM

void *
osl_debug_malloc(osl_t *osh, uint size, int line, const char* file)
{
	bcm_mem_link_t *p;
	const char* basename;
	osl_t *h = osh;

	if (size == 0) {
		return NULL;
	}

	p = (bcm_mem_link_t*)osl_malloc(osh, sizeof(bcm_mem_link_t) + size);
	if (p == NULL)
		return p;

	p->size = size;
	p->line = line;

	basename = strrchr(file, '/');
	/* skip the '/' */
	if (basename)
		basename++;

	if (!basename)
		basename = file;

	strncpy(p->file, basename, BCM_MEM_FILENAME_LEN);
	p->file[BCM_MEM_FILENAME_LEN - 1] = '\0';

	/* link this block */
	p->prev = NULL;
	p->next = h->os_dbgmem_list;
	if (p->next)
		p->next->prev = p;
	h->os_dbgmem_list = p;

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
	osl_t *h = osh;

	ASSERT(h && (h->os_magic == OS_HANDLE_MAGIC));

	if (p->size == 0) {
		printf("osl_debug_mfree: double free on addr 0x%x size %d "
		       "at line %d file %s\n", (uint)addr, size, line, file);
		return;
	}

	if (p->size != size) {
		printf("osl_debug_mfree: dealloc size %d does not match "
		       "alloc size %d on addr 0x%x at line %d file %s\n",
		       size, p->size, (uint)addr, line, file);
		return;
	}

	/* unlink this block */
	if (p->prev)
		p->prev->next = p->next;
	if (p->next)
		p->next->prev = p->prev;
	if (h->os_dbgmem_list == p)
		h->os_dbgmem_list = p->next;
	p->next = p->prev = NULL;

	osl_mfree(osh, p, size + sizeof(bcm_mem_link_t));
}

int
osl_debug_memdump(osl_t *osh, struct bcmstrbuf *b)
{
	bcm_mem_link_t *p;
	osl_t *h = osh;

	ASSERT(h && (h->os_magic == OS_HANDLE_MAGIC));

	bcm_bprintf(b, "   Address\tSize\tFile:line\n");
	for (p = h->os_dbgmem_list; p; p = p->next)
		bcm_bprintf(b, "0x%08x\t%5d\t%s:%d\n",
			(int)p + sizeof(bcm_mem_link_t), p->size, p->file, p->line);

	return 0;
}

#endif	/* BCMDBG_MEM */

void *
osl_malloc(osl_t *osh, uint size)
{
	osl_t *h = osh;
	void *addr;

	if (h)
		ASSERT(h->os_magic == OS_HANDLE_MAGIC);

	addr = malloc(size, M_DEVBUF, M_NOWAIT);
	if (!addr && h)
		h->os_failed++;
	else if (h)
		h->os_malloced += size;
	return (addr);
}

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


void
osl_mfree(osl_t *osh, void *addr, uint size)
{
	osl_t *h = osh;

#ifdef BCMDBG
	deadbeef(addr, size);
#endif
	if (h) {
		ASSERT(h->os_magic == OS_HANDLE_MAGIC);
		h->os_malloced -= size;
	}
	free(addr, M_DEVBUF);
}

uint
osl_malloced(osl_t *osh)
{
	osl_t *h = osh;

	ASSERT(h && (h->os_magic == OS_HANDLE_MAGIC));
	return (h->os_malloced);
}

uint osl_malloc_failed(osl_t *osh)
{
	osl_t *h = osh;

	ASSERT(h && (h->os_magic == OS_HANDLE_MAGIC));
	return (h->os_failed);
}

void *
osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align_bits, uint *alloced,
	ulong *pap, osldma_t **dmah)
{
	int 			error = -1;
	bus_dma_segment_t 	segs;
	int			num_segs;
#if __NetBSD_Version__ >= 500000003
	void		*va;
#else
	caddr_t 		va;
#endif
	bus_dmamap_t 		*map = (bus_dmamap_t *)dmah;
	int boundary = PAGE_SIZE;
	uint16 align = (1 << align_bits);

	ASSERT(osh);
	ASSERT(osh->os_magic == OS_HANDLE_MAGIC);
	ASSERT(pap != NULL);
	ASSERT(map != NULL);

	/* fix up the alignment requirements first */
	if (!ISALIGNED(DMA_CONSISTENT_ALIGN, align))
		size += align;
	*alloced = size;

	/* Fix the boundary to be power of 2 but greater than size
	 * This tells OS for continguous buffer that is maintained across this boundary
	 */
	while (boundary < size)
		boundary <<= 1;

	if ((error = bus_dmamem_alloc(PCI_DMAT(osh), size, 0,
	                              boundary, &segs, 16, &num_segs, 0)) != 0) {
		printf("%s: unable to allocate consistent dma memory, error = %d\n",
		    osh->os_xname, error);
		goto fail0;
	}

	if ((error = bus_dmamem_map(PCI_DMAT(osh), &segs, num_segs,
	   size, &va, BUS_DMA_COHERENT)) != 0) {
		printf("%s: unable to map consistent dma memory, error = %d\n",
		    osh->os_xname, error);
		goto fail1;
	}

	if ((error = bus_dmamap_create(PCI_DMAT(osh), size, 1,
	    size, 0, 0, map)) != 0) {
		printf("%s: unable to create consistent DMA map, "
		    "error = %d\n", osh->os_xname, error);
		goto fail2;
	}

	if ((error = bus_dmamap_load(PCI_DMAT(osh), *map, va,
	    size, NULL, 0)) != 0) {
		printf("%s: unable to load consistent DMA map, error = %d\n",
		    osh->os_xname, error);
		goto fail3;
	}

	/* pap points to the the physical address */
	*pap = (*map)->dm_segs[0].ds_addr;

	/* Return the virtual address */
	return va;

fail3:
	bus_dmamap_destroy(PCI_DMAT(osh), *map);
fail2:
	bus_dmamem_unmap(PCI_DMAT(osh), va, size);
fail1:
	bus_dmamem_free(PCI_DMAT(osh), &segs, num_segs);
fail0:
	return NULL;
}

void
osl_dma_free_consistent(osl_t *osh, void *va, uint size, ulong pa, osldma_t *dmah)
{
	bus_dmamap_t map = (bus_dmamap_t)(dmah);

	ASSERT(map != NULL);
	ASSERT(map->dm_nsegs != 0);
	ASSERT(osh);
	ASSERT(osh->os_magic == OS_HANDLE_MAGIC);

	/* Force DMA cache coherency before unmapping */
	bus_dmamap_sync(PCI_DMAT(osh), map, 0, size,
		BUS_DMASYNC_POSTWRITE|BUS_DMASYNC_POSTREAD);
	bus_dmamap_sync(PCI_DMAT(osh), map, 0, size,
		BUS_DMASYNC_PREWRITE|BUS_DMASYNC_PREREAD);

	bus_dmamem_free(PCI_DMAT(osh), map->dm_segs, map->dm_nsegs);
	bus_dmamap_destroy(PCI_DMAT(osh), map);
}

/*
 *  defrag an mbuf chain leaging the initial mbuf at the same address
 */

static int
m_defrag_bcm(struct mbuf *m, int how)
{
	struct mbuf *m0;

	if (m->m_next == NULL)
		return 0;

	if ((m0 = m_gethdr(how, m->m_type)) == NULL)
		return -1;

	if (m->m_pkthdr.len > MHLEN) {
		MCLGET(m0, how);
		if (!(m0->m_flags & M_EXT)) {
			m_free(m0);
			return -1;
		}
	}
	/* Copy the data */
	m_copydata(m, 0, m->m_pkthdr.len, mtod(m0, caddr_t));
	m0->m_pkthdr.len = m0->m_len = m->m_pkthdr.len;

	/* free up the old chain and data */
	if (m->m_flags & M_EXT) {
		int s = splvm();
#if __NetBSD_Version__ >= 500000003
		m_ext_free(m);
		ASSERT(0); /* UNTESTED */
#else
		m_ext_free(m, FALSE);
#endif
		splx(s);
	}
	m_freem(m->m_next);
	m->m_next = NULL;


	/* reorganize the initial mbuf to point to the new cluster
	 *  (or copy the data into it directly)
	 */

	if (m0->m_flags & M_EXT) {
		bcopy(&m0->m_ext, &m->m_ext, sizeof(struct _m_ext));
		MCLINITREFERENCE(m);
		m->m_flags |= M_EXT|M_CLUSTER;
		m->m_data = m->m_ext.ext_buf;
	} else {
		m->m_data = m->m_pktdat;
		bcopy(&m0->m_data, &m->m_data, m0->m_len);
	}
	m->m_pkthdr.len = m->m_len = m0->m_len;

	/* Free up the old mbuf (clearing the cluster flag since we moved it */
	m0->m_flags &= ~(M_EXT|M_CLUSTER);
	m_free(m0);

	return 0;
}


uintptr BCMFASTPATH
osl_dma_map(osl_t *osh, void *va, uint size, int direction, struct mbuf *mbuf,
            hnddma_seg_map_t *segmap)
{
	int 		dir;
	int 		error = -1;
	int             i;
	osldma_t 	*map = NULL;
	struct mbuf *m;
	int len = 0;

	ASSERT(osh->os_magic == OS_HANDLE_MAGIC);
	ASSERT(va != NULL);
	ASSERT(segmap != NULL);
	m = mbuf;

	if (mbuf->m_flags & M_PKTHDR) {
		m = mbuf;
		while (m) {
			len += m->m_len;
			m = m->m_next;
		}
		mbuf->m_pkthdr.len = len;
	}
	dir = (direction == DMA_TX) ? BUS_DMA_WRITE: BUS_DMA_READ;

	if ((map = osl_dmamap_get(osh)) == NULL) {
			printf("%s: unable to create %s streaming DMA map, error = %d\n",
				osh->os_xname, (direction == DMA_TX) ? "TX" : "RX", error);
			goto fail1;
	}

	ASSERT(map->dma_magic == DMA_HANDLE_MAGIC);

#ifdef BCMDMASGLISTOSL
	if ((direction == DMA_TX) && mbuf && (mbuf->m_flags & M_PKTHDR)) {
		if ((error = bus_dmamap_load_mbuf(PCI_DMAT(osh), map->dma_map, mbuf,
		                                  dir | BUS_DMA_NOWAIT)) != 0) {
			if (error == EFBIG) {
				if (m_defrag_bcm(mbuf, M_NOWAIT) == 0)
					error = bus_dmamap_load_mbuf(PCI_DMAT(osh),
					                             map->dma_map,
					                             mbuf,
					                             dir | BUS_DMA_NOWAIT);
			}
			if (error) {
				printf("%s: unable to load %s streaming DMA map, error = %d\n",
				       osh->os_xname, (direction == DMA_TX) ? "TX" : "RX", error);
				goto fail2;
			}
		}
	} else {
#endif /* BCMDMASGLISTOSL */
		if ((error = bus_dmamap_load(PCI_DMAT(osh), map->dma_map, va,
		                             size, NULL, dir | BUS_DMA_NOWAIT)) != 0) {
			printf("%s: unable to load %s streaming DMA map, error = %d\n",
			       osh->os_xname, (direction == DMA_TX) ? "TX" : "RX", error);
			goto fail2;
		}
#ifdef BCMDMASGLISTOSL
	}
#endif
	/* Flush the cache */
	if (direction == DMA_TX) {
		bus_dmamap_sync(PCI_DMAT(osh), map->dma_map, 0,
		                map->dma_map->dm_mapsize,
		                BUS_DMASYNC_PREWRITE);
	} else {
		bus_dmamap_sync(PCI_DMAT(osh), map->dma_map, 0, size,
		                BUS_DMASYNC_PREREAD);
	}

	ASSERT(map->dma_map->dm_segs[0].ds_addr != 0);
	ASSERT(map->dma_map->dm_nsegs != 0);

	/* Return the physical address */
	segmap->oshdmah = map;
	for (i = 0; i < map->dma_map->dm_nsegs; i++) {
		segmap->segs[i].addr = map->dma_map->dm_segs[i].ds_addr;
		segmap->segs[i].length = map->dma_map->dm_segs[i].ds_len;
	}
	segmap->nsegs = map->dma_map->dm_nsegs;
	segmap->origsize = size;
	return (uintptr)(map->dma_map->dm_segs[0].ds_addr);

fail2:
	osl_dmamap_put(osh, map);
fail1:
	return (uintptr)NULL;
}

void BCMFASTPATH
osl_dma_unmap(osl_t *osh, uint pa, uint size, int direction, hnddma_seg_map_t *segmap)
{
	osldma_t *map = (osldma_t*)segmap->oshdmah;

	ASSERT(osh);
	ASSERT(osh->os_magic == OS_HANDLE_MAGIC);
	ASSERT(segmap != NULL);

	ASSERT(map != NULL);
	ASSERT(map->dma_magic == DMA_HANDLE_MAGIC);
	ASSERT(map->dma_map->dm_nsegs != 0);

	/* Flush the cache */
	bus_dmamap_sync(PCI_DMAT(osh), map->dma_map, 0, size,
		(direction == DMA_TX)? BUS_DMASYNC_POSTWRITE: BUS_DMASYNC_POSTREAD);

	bus_dmamap_unload(PCI_DMAT(osh), map->dma_map);
	osl_dmamap_put(osh, map);
}

void
osl_dma_addrwidth(osl_t *osh, uint width)
{
	osh->dma_addrwidth = width;
}

#ifdef BCMDBG_ASSERT
void
osl_assert(const char *exp, const char *file, int line)
{
	char tempbuf[255];

	sprintf(tempbuf, "assertion \"%s\" failed: file \"%s\", line %d\n", exp, file, line);
	panic(tempbuf);
}
#endif	/* BCMDBG_ASSERT */

uint32
osl_sysuptime(void)
{
	struct timeval tv;
	uint32 ms;

	getmicrouptime(&tv);

	ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	return ms;
}

int
osl_busprobe(uint32 *val, uint32 addr)
{
#ifdef __mips__
	int ret;
	extern int get_dbe(char *addr, char *value, int size);

	ret = get_dbe((char *)addr, (char *)val, sizeof(*val));
	if (ret)
		*val = 0;
	return (ret);
#else
	*val = R_REG(NULL, (uint32 *)addr);
#endif
	return 0;
}

/* Allocate packet header, and optionally attach a cluster */
void * BCMFASTPATH
osl_pktget(osl_t *osh, uint len, bool send)
{
	struct mbuf *mbuf;
	bool gottag = FALSE;

	ASSERT(osh);
	ASSERT(len <= MCLBYTES);

	MGETHDR(mbuf, M_NOWAIT, MT_HEADER);
	if (mbuf == NULL)
		return (NULL);

	MCLAIM(mbuf, &osh->mbuf_mowner);

	if (len > MHLEN) {
		MCLGET(mbuf, M_NOWAIT);
		if ((mbuf->m_flags & M_EXT) == 0)
			goto bad1;
	}

	mbuf->m_len = len;

	/* Set pkttag if specified */
	if (osh->os_pkttag) {
		gottag = osl_pkttag_add(mbuf);

		if (!gottag && mbuf) {
			goto bad1;
		}
	}
	/* make sure to always init priority since
	 * wlc_recvdata assumes that its initialized to some sane value in the toss handler
	 */
	PKTSETPRIO(mbuf, 0);

	return ((void *) mbuf);
bad1:
	/* dont call osl_pktfree since with certain args you can
	 * end up calling wlc_pkt_callback...this assumes you have a packet tag
	 * which will not be the case if MCLGET fails
	 */
	m_freem(mbuf);
	return NULL;
}

void BCMFASTPATH
osl_pktfree(osl_t *osh, void *mbuf, bool send)
{
	int ipl;
	struct mbuf *m, *n;

	ASSERT(osh);
	ASSERT(mbuf);

	m = (struct mbuf *)mbuf;
	ipl = splnet();

	if (send && osh->tx_fn)
		osh->tx_fn(osh->tx_ctx, m, 0);

	do {
		MFREE(m, n);
		m = n;
	} while (m);

	splx(ipl);
}

uchar *
osl_pktpush(osl_t *osh, void *mbuf, int nbytes)
{

	ASSERT(mbuf);
	ASSERT(M_LEADINGSPACE((struct mbuf*)mbuf) >= (int)nbytes);

	((struct mbuf*)mbuf)->m_data -= nbytes;
	((struct mbuf*)mbuf)->m_len +=  nbytes;

	return (((struct mbuf*)mbuf)->m_data);
}

uchar *
osl_pktpull(osl_t *osh, void *mbuf, int nbytes)
{

	ASSERT(mbuf);
	ASSERT((int)nbytes <= ((struct mbuf*)mbuf)->m_len + M_TRAILINGSPACE((struct mbuf*)mbuf));

	((struct mbuf*)mbuf)->m_data += nbytes;
	((struct mbuf*)mbuf)->m_len -=  nbytes;

	return (((struct mbuf*)mbuf)->m_data);
}

static INLINE void
osl_pktsettag(void *m, void *x)
{
#ifdef OSL_USE_TAG
	struct m_tag *tag;
#endif

	ASSERT(((struct mbuf *)(m))->m_flags & M_PKTHDR);
#ifdef OSL_USE_TAG
	if (x) {
		tag = m_tag_get(PACKET_TAG_BRCM_CTX, sizeof(void *), M_NOWAIT);
		ASSERT(tag);
		*((void **)(tag+1)) = x;
		m_tag_prepend((struct mbuf *)m, tag);
	} else {
		tag = m_tag_find((struct mbuf *)m, PACKET_TAG_BRCM_CTX, NULL);
		if (tag) {
			m_tag_unlink((struct mbuf *)m, tag);
			m_tag_free(tag);
		}
	}
#else
	((struct mbuf *) m)->m_owner = x;
#endif /* OLS_USE_TAG */
}


static INLINE bool
osl_pkttag_add(void *p)
{
	struct mbuf *m = (struct mbuf *)p;
	struct m_tag *tag;

	tag = m_tag_find(m, PACKET_TAG_BRCM_CTX, NULL);
	if (tag == NULL) {
		tag = m_tag_get(PACKET_TAG_BRCM_CTX, OSL_PKTTAG_SZ, M_NOWAIT);
		if (!tag) {
			return FALSE;
		}
		m_tag_prepend(m, tag);
	}

	bzero(tag + 1, OSL_PKTTAG_SZ);
	return TRUE;
}

static INLINE void
osl_pkttag_remove(void *p)
{
	struct mbuf *m = (struct mbuf *)p;
	struct m_tag *tag;

	tag = m_tag_find(m, PACKET_TAG_BRCM_CTX, NULL);
	if (tag != NULL) {
		m_tag_unlink(m, tag);
		m_tag_free(tag);
	}
}

/* Convert a native(OS) packet to driver packet.
 * In the process, native packet is destroyed, there is no copying
 * Also, a packettag is attached if requested.
 * The conversion fails if pkttag cannot be allocated
 */
void *
osl_pkt_frmnative(osl_t *osh, struct mbuf *m)
{
	if (osh->os_pkttag) {
		if (!osl_pkttag_add(m)) {
			m_freem(m);
			return NULL;
		}
	}
	return (void *)m;
}

/* Convert a driver packet to native(OS) packet
 * In the process, packettag is removed
 */
struct mbuf*
osl_pkt_tonative(osl_t *osh, void *p)
{

	ASSERT(osh);
	ASSERT(p);

	osl_pkttag_remove(p);
	return (struct mbuf *)p;
}

void *
osl_pktdup(osl_t *osh, void *mbuf)
{
	void *p;

	ASSERT(mbuf);
	ASSERT(osh);

	/* m_copypacket() does not duplicate the data buffer if
	 * M_EXT bit of m_flags is set (eg in a cluster)
	 * it does copy the tag
	 */
	if ((p = m_copypacket((struct mbuf *)mbuf, M_NOWAIT)) == NULL)
		return p;

	osl_pktsetprio((struct mbuf *)p, 0);

	return (p);
}

/* Merge 2 mbuf chains, in the process pkttag from the
 * second chain m2 gets destroyed
 * Assume pkttag info dealt with by user
 */
void
osl_pktsetnext(osl_t *osh, void *p1, void *p2)
{
	struct mbuf *m1 = (struct mbuf *)p1;
	struct mbuf *m2 = (struct mbuf *)p2;

	ASSERT(p1);
	m1->m_next = m2;

}

uint
osl_pci_bus(osl_t *osh)
{
	ASSERT(osh);
	ASSERT(osh->os_magic == OS_HANDLE_MAGIC);

	return osh->os_bus;
}

uint
osl_pci_slot(osl_t *osh)
{
	ASSERT(osh);
	ASSERT(osh->os_magic == OS_HANDLE_MAGIC);

	return osh->os_slot;
}

/* translate bcmerrors into NetBSD errors */
int
osl_error(int bcmerror)
{
	if (bcmerror > 0)
		bcmerror = 0;
	else if (bcmerror < BCME_LAST)
		bcmerror = BCME_ERROR;

	/* Array bounds covered by ASSERT in osl_attach */
	return OSL_BCMERRORMAP[-bcmerror];
}
#if __mips__ && __NetBSD_Version__ >= 500000003
void *
osl_reg_map(uint32 pa, uint size)
{
	ASSERT(size <= PAGE_SIZE);

	return (pmap_mapdev((unsigned long)pa, (unsigned long)size));
}

void
osl_reg_unmap(void *va)
{
	pmap_unmapdev((unsigned long)va, PAGE_SIZE);
}
#endif /* __NetBSD_Version__ >= 500000003 */
