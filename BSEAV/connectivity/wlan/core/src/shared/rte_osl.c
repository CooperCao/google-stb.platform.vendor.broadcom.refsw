/*
 * HND RTE OS Abstraction Layer
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#ifdef SBPCI
#include <hndpci.h>
#endif /* SBPCI */
#include <rte_heap.h>
#include <rte_trap.h>
#include <rte.h>

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
static void *osl_malloc_persist_ra(uint size, uint32 constraint, void *call_site);
static void *osl_malloc_ra(uint size, void *call_site);
#endif

osl_t *
#ifdef SHARED_OSL_CMN
BCMATTACHFN(osl_attach)(void *dev, void **osl_cmn)
#else
BCMATTACHFN(osl_attach)(void *dev)
#endif /* SHARED_OSL_CMN */
{
#ifndef SHARED_OSL_CMN
	void **osl_cmn = NULL;
#endif
	osl_t *osh;

	osh = (osl_t *)hnd_malloc(sizeof(osl_t));
	ASSERT(osh);
	bzero(osh, sizeof(osl_t));

	if (osl_cmn == NULL || *osl_cmn == NULL) {
		osh->cmn = (osl_cmn_t *)hnd_malloc(sizeof(osl_cmn_t));
		bzero(osh->cmn, sizeof(osl_cmn_t));
		if (osl_cmn)
			*osl_cmn = osh->cmn;
	} else
		if (osl_cmn)
			osh->cmn = *osl_cmn;

	osh->cmn->refcount++;

	ASSERT(osh->cmn);

	osh->dev = dev;
	return osh;
}

void
BCMATTACHFN(osl_detach)(osl_t *osh)
{
	if (osh == NULL)
		return;
	osh->cmn->refcount--;
	hnd_free(osh);
}

/* Mechanism to force persistent allocations */
static bool force_nopersist = FALSE;

void
osl_malloc_set_nopersist(void)
{
	force_nopersist = TRUE;
}

void
osl_malloc_clear_nopersist(void)
{
	force_nopersist = FALSE;
}

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
void *
osl_malloc(uint size)
{
	return osl_malloc_ra(size, CALL_SITE);
}

static void *
osl_malloc_ra(uint size, void *call_site)
{
	void *p;

	if (ATTACH_PART_RECLAIMED() || force_nopersist)
		p = hnd_malloc_align(size, 0, call_site);
	else
		p = hnd_malloc_persist_align(size, 0, call_site);

	return p;
}

void *
osl_mallocz(uint size)
{
	void * ptr;

	ptr = osl_malloc_ra(size, CALL_SITE);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void *
osl_malloc_align(uint size, uint align_bits, void *call_site)
{
	void *p;

	if (call_site == NULL)
		call_site = CALL_SITE;

	if (ATTACH_PART_RECLAIMED() || force_nopersist)
		p = hnd_malloc_align(size, align_bits, call_site);
	else
		p = hnd_malloc_persist_align(size, align_bits, call_site);

	return p;
}

void *
osl_mallocz_nopersist(uint size)
{
	void *ptr;

	ptr = hnd_malloc_align(size, 0, CALL_SITE);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void *
osl_malloc_nopersist(uint size)
{
	return hnd_malloc_align(size, 0, CALL_SITE);
}

void *
osl_malloc_persist(uint size, uint32 constraint)
{
	return osl_malloc_persist_ra(size, constraint, CALL_SITE);
}

static void *
osl_malloc_persist_ra(uint size, uint32 constraint, void *call_site)
{
	void *p;

#ifdef BCM_RECLAIM
	if (!ATTACH_PART_RECLAIMED())
		printf("MALLOC warning: allocations are persistent by default in attach phase!\n");
#endif /* BCM_RECLAIM */

	if (constraint)
		p = hnd_malloc_persist_attach_align(size, 0, call_site);
	else
		p = hnd_malloc_persist_align(size, 0, call_site);

	return p;
}

void *
osl_mallocz_persist(uint size, uint32 constraint)
{
	void *ptr;

	ptr = osl_malloc_persist_ra(size, constraint, CALL_SITE);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}
#else
void *
osl_malloc(uint size)
{
	void *p;
	if (ATTACH_PART_RECLAIMED() || force_nopersist)
		p = hnd_malloc_align(size, 0);
	else
		p = hnd_malloc_persist_align(size, 0);

#ifdef BCMDBG_MEMNULL
	if (p == NULL)
		printf("MALLOC failed: size=%d ra=0x%x\n", size, CALL_SITE);
#endif

	return p;
}

void *
osl_malloc_persist(uint size, uint32 constraint)
{

#ifdef BCM_RECLAIM
	if (!ATTACH_PART_RECLAIMED())
		printf("MALLOC warning: allocations are persistent by default in attach phase!\n");
#endif /* BCM_RECLAIM */

	void *p;
	if (constraint)
		p = hnd_malloc_persist_attach_align(size, 0);
	else
		p = hnd_malloc_persist_align(size, 0);

#ifdef BCMDBG_MEMNULL
	if (p == NULL)
		printf("MALLOC failed: size=%d ra=%p\n", size, CALL_SITE);
#endif
	return p;
}

void *
osl_mallocz(uint size)
{
	void * ptr;

	ptr = osl_malloc(size);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void *
osl_mallocz_persist(uint size, uint32 constraint)
{
	void * ptr;

	ptr = osl_malloc_persist(size, constraint);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void *
osl_malloc_align(uint size, uint align_bits)
{
	void *p;

	if (ATTACH_PART_RECLAIMED() || force_nopersist)
		p = hnd_malloc_align(size, align_bits);
	else
		p = hnd_malloc_persist_align(size, align_bits);

#ifdef BCMDBG_MEMNULL
	if (p == NULL)
		printf("MALLOC failed: size=%d ra=0x%x\n", size, CALL_SITE);
#endif

	return p;
}

void *
osl_malloc_nopersist(uint size)
{
	void *p;

	p = hnd_malloc_align(size, 0);

#ifdef BCMDBG_MEMNULL
	if (p == NULL)
		printf("MALLOC failed: size=%d ra=0x%x\n", size, CALL_SITE);
#endif

	return p;
}

void *
osl_mallocz_nopersist(uint size)
{
	void *ptr;

	ptr = osl_malloc_nopersist(size);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}
#endif /* BCMDBG_MEM */

int
osl_mfree(void *addr)
{
	return hnd_free(addr);
}

uint
osl_malloced(osl_t *osh)
{
	return 0;
}

uint
osl_malloc_failed(osl_t *osh)
{
	return 0;
}

int
osl_busprobe(uint32 *val, uint32 addr)
{
	*val = *(uint32 *)addr;
	return 0;
}

/* translate these erros into hnd specific errors */
int
osl_error(int bcmerror)
{
	return bcmerror;
}

#ifdef SBPCI
#include <typedefs.h>
#include <rte_dev.h>

uint
osl_pci_bus(osl_t *osh)
{
	hnd_dev_t *dev = (hnd_dev_t *)osh->dev;
	pdev_t *pdev = (pdev_t *)dev->pdev;

	return pdev->bus;
}

uint
osl_pci_slot(osl_t *osh)
{
	hnd_dev_t *dev = (hnd_dev_t *)osh->dev;
	pdev_t *pdev = (pdev_t *)dev->pdev;

	return pdev->slot;
}

uint32
osl_pci_read_config(osl_t *osh, uint offset, uint size)
{
	hnd_dev_t *dev = (hnd_dev_t *)osh->dev;
	pdev_t *pdev = (pdev_t *)dev->pdev;
	uint32 data;

	if (extpci_read_config(pdev->sih, pdev->bus, pdev->slot, pdev->func, offset,
	                       &data, size) != 0)
		data = 0xffffffff;

	return data;
}

void
osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val)
{
	hnd_dev_t *dev = (hnd_dev_t *)osh->dev;
	pdev_t *pdev = dev->pdev;

	extpci_write_config(pdev->sih, pdev->bus, pdev->slot, pdev->func, offset, &val, size);
}
#endif /* SBPCI */

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

void
osl_sys_halt(void)
{
	hnd_die();
}

extern void*
osl_get_fatal_logbuf(osl_t *osh, uint32 size, uint32 *alloced)
{
	return (hnd_get_fatal_logbuf(size, alloced));
}

#if defined(BCMDBG_MEM) || defined(BCMDBG_HEAPCHECK)
extern int
osl_memcheck(const char *file, int line)
{
	return (hnd_memcheck(file, line));
}
#endif
