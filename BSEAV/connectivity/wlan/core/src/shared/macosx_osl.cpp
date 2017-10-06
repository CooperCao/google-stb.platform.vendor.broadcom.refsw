/*
 * Mac OS X Independent Layer
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
 * Copyright (c) 2002 Apple Computer, Inc.  All rights reserved.
 *
 * $Id$
 */
/* FILE-CSTYLED */

#include "typedefs.h"
#include "osl.h"

#include <kern/task.h>
#include <kern/debug.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IODMACommand.h>
#include <IOKit/network/IOMbufMemoryCursor.h>
#include <IOKit/apple80211/apple80211_var.h>
#include <libkern/version.h>
#include <libkern/OSDebug.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/sysctl.h>
#include <IOKit/IOLib.h>

#include <pcicfg.h>

#ifdef MESSAGE_TRACER
#if VERSION_MAJOR < 10
#error "Message tracer defined for wrong OS version"
#endif

#include <IOKit/apple80211/IO8Log.h>
#if VERSION_MAJOR > 12
#include <IOKit/apple80211/apple80211_common.h>
#endif
#endif /* MESSAGE_TRACER */

extern "C" {

#include "bcmutils.h"
#include "wlioctl.h"
#include "wl_dbg.h"

#ifdef BCM_BACKPLANE_TIMEOUT
#include <siutils.h>
#endif

#include <proto/802.1d.h>

#define OS_HANDLE_MAGIC		0x1234abcd	/* Magic # to recognize osh */
#define OSL_MAXSEGS		10		// max # of phys memory segs in a mbuf chain

#define OSL_1_GIG		0x40000000	// One gigabyte for 30 bit DMA address range checks
#define OSL_POOL_SIZE		512		// Keep this many private mbufs in the
						// < 1 GB address range if needed
#define OSL_BAD_MBUF_LIMIT	512		// Allow this many > 1GB addr allocations before
						// giving up on creating a private mbuf pool
#define OSL_MBUF_TAG_MODULE		"com.broadcom.iokit.osl"	// mbuf tag ID string
#define OSL_MBUF_TAG_TYPE_PKTTAG	0	//  mbuf tag ID for PKTTAG
#define OSL_MBUF_TAG_TYPE_PRIV		1	//  mbuf tag ID for private pool

#ifdef BCMDBG
#define DPRINT( ARGS... )	printf( ARGS )
#else
#define DPRINT( ARGS... )
#endif

#ifdef BCMDBG_MEM
#define BCM_MEM_FILENAME_LEN 	24		/* Mem. filename length */

typedef struct bcm_mem_link {
	struct bcm_mem_link *prev;
	struct bcm_mem_link *next;
	uint64_t timestamp;
	uint	size;
	int	line;
	char	file[BCM_MEM_FILENAME_LEN];
} bcm_mem_link_t;
#endif /* BCMDBG_MEM */

struct osl_info {
	uint magic;			// OSL ID for program error protection
	IOPCIDevice *pcidev;		// PCIDevice for PCI operations
	bool use_private_mbufs;		// true when < 1 GB bus addresses are required
	mbuf_t private_pool;		// pool of mbufs with bus addrs < 1 GB
	uint dma_addrwidth;		// address width our core DMA supports
	uint malloced;			// current MALLOCed bytes
	uint failed;			// count of MALLOC calles that failed
	pktfree_cb_fn_t tx_fn;
	void *tx_ctx;

#ifdef BCMDMASGLISTOSL
	IOMbufNaturalMemoryCursor * mbuf_cursor;

	/* Following are actually valid only for one mapping but it's ok as it's used immediately and
	 * then discarded
	 */
	struct IOPhysicalSegment seg_list[MAX_DMA_SEGS];
	UInt32 nsegs;

#endif /* BCMDMASGLISTOSL */

#if defined(BCM_BACKPLANE_TIMEOUT)
	void *sih;
	void * wl;
	bool checkAllWrappers;
	wl_fatal_error_fp wl_fatal_error_cb;
#endif /* BCM_BACKPLANE_TIMEOUT */

#ifdef PCI_ACCESS_HISTORY
	uint32 pci_access_history_entries;
	uint32 pci_access_log_flags;
	uint32 pci_access_idx;
	pci_access_t * pci_access_history;
	uint32 bar0 = 0;
	uint32 bar0_win2 = 0;
	uint32 sec_bar0_win = 0;
	uint32 sec_bar0_win_wrap = 0;
#ifdef PCI_ACCESS_HISTORY_DEBUG_REG_ACCESS
	uint32 prev_access_type;
	uint32 prev_access_addr;
	void * prev_mmio_backtrace[MAX_BACKTRACE_DEPTH];
	int prev_mmio_backtrace_depth;
#endif /* PCI_ACCESS_HISTORY_DEBUG_REG_ACCESS */
#endif /* PCI_ACCESS_HISTORY */
};

int osl_printf(const char *fmt, ...);
static void sysctlattach(void *);
static void sysctldetach(void *);

#ifdef BCMDONGLEHOST
typedef struct osl_spin_lock
{
	lck_grp_t *lock_grp;
	lck_spin_t *lock;
} osl_spin_lock_t;
#endif /* BCMDONGLEHOST */

#ifdef BCMDBG_MEM
bcm_mem_link_t *g_brcm_dbgmem_list = NULL;
#endif

#if ENABLE_DBGMEM

// Commom macros for geting DBGMEM boot-args

#define DBGMEM_INFO_GETBOOTARG_TRACEFLAGS()													\
do {																						\
	int32_t value = 0;																		\
	if (PE_parse_boot_argn("io80211.dbgmeminfo.traceflags", &value, sizeof(value)))			\
	{																						\
		DBGMEM_INFO_SETTRACEFLAGS( value );													\
		printf(  "DBGMEM Info: Found \"io80211.dbgmeminfo.traceflags\", set to [0x%08x]\n",	\
					DBGMEM_INFO_GETTRACEFLAGS() );											\
	}																						\
} while( 0 );

#define DBGMEM_INFO_GETBOOTARG_ALLOCSIZE()													\
do {																						\
	int32_t value = 0;																		\
	if (PE_parse_boot_argn("io80211.dbgmeminfo.allocsize", &value, sizeof(value)))			\
	{																						\
		value *= (1024*1024);																\
		DBGMEM_INFO_SETALLOCSIZE( LIMIT_TO_RANGE(value, DBGMEM_INFO_ALLOCSIZE_MIN,			\
														DBGMEM_INFO_ALLOCSIZE_MAX));		\
		printf(  "DBGMEM Info: Found \"io80211.dbgmeminfo.allocsize\", set to [0x%08x]\n",	\
					DBGMEM_INFO_GETALLOCSIZE() );											\
	}																						\
} while( 0 );

#define DBGMEM_INFO_GETALLBOOTARGS()														\
do	{																						\
	DBGMEM_INFO_GETBOOTARG_TRACEFLAGS();													\
	DBGMEM_INFO_GETBOOTARG_ALLOCSIZE();														\
} while(0);


#if defined(BCMDBG)

#define DBGMEM_INFO_CIRCULAR_ARRAY_MAX_ENTRIES			(128*1024)
struct brcm_dbgmem_info_element_s g_brcm_dbgmem_info_elements[DBGMEM_INFO_CIRCULAR_ARRAY_MAX_ENTRIES] = {};


/* Static/compile time macros */
#define DBGMEM_INFO_INITIALIZE()															\
do {																						\
	DBGMEM_INFO_GETALLBOOTARGS();															\
} while(0);

#define DBGMEM_INFO_CLEANUP()																\
do {																						\
																							\
} while(0);


// Static (BCMDBG build) initialization
struct g_brcm_dbgmem_info_s g_brcm_dbgmem_info =
{	.enable= 1,
	.alloced= 0, .allocsize= 0,
	.traceflags= (DBGMEM_INFO_TRACEFLAG_DMAALLOC | DBGMEM_INFO_TRACEFLAG_MALLOC | 0 /* DBGMEM_INFO_TRACEFLAG_FREE */),
	.idx= 0, .max= DBGMEM_INFO_CIRCULAR_ARRAY_MAX_ENTRIES,
	.elements= &g_brcm_dbgmem_info_elements[0]
};
#else /* !BCMDBG */

/* Dynamic/runtime initialization */
struct g_brcm_dbgmem_info_s g_brcm_dbgmem_info =
{	.enable= 1,
	.alloced= 0, .allocsize= 0 /* implies disabled */,
	.traceflags= (0),
	.idx= 0, .max= 0,
	.elements= NULL
};

/* Dynamic/runtime macros */
#define DBGMEM_INFO_INITIALIZE()															\
do {																						\
	DBGMEM_INFO_GETALLBOOTARGS();															\
																							\
    if( DBGMEM_INFO_GETENABLE() && DBGMEM_INFO_GETALLOCSIZE() ) {                            \
        size_t reqsize = 0, size = 0;                                                                \
        reqsize = size = DBGMEM_INFO_GETALLOCSIZE();                                         \
                                                                                             \
        while( (g_brcm_dbgmem_info.elements == NULL) &&                                      \
                    (size >= DBGMEM_INFO_ALLOCSIZE_MIN))                                     \
        {                                                                                    \
            /* Try to allocate requested size */                                             \
            if( g_brcm_dbgmem_info.elements =                                                \
                        (struct brcm_dbgmem_info_element_s*)IOMalloc(size))                  \
	{																						\
                /* Allocated size */                                                         \
                DBGMEM_INFO_SETALLOCSIZE( size );                                            \
				DBGMEM_INFO_SETALLOCED( 1 );														\
				DBGMEM_INFO_SETENABLE( 1 );															\
				DBGMEM_INFO_SETMAX( DBGMEM_INFO_GETALLOCSIZE()/										\
									sizeof(struct brcm_dbgmem_info_element_s) );			\
                printf(  "DBGMEM Info: Alloced size[0x%08x], traceflags[0x%08x], max[%u],"   \
                         " requested size[0x%08x]\n",                                        \
							DBGMEM_INFO_GETALLOCSIZE(), DBGMEM_INFO_GETTRACEFLAGS(),			\
                            DBGMEM_INFO_GETMAX(), reqsize );                                 \
																							\
                break;                                                                       \
            }                                                                                \
                                                                                             \
            if (size <= DBGMEM_INFO_ALLOCSIZE_MIN ) /* at minimum and failed, break out */   \
                break;                                                                       \
                                                                                             \
            /* Reduce size, try allocation again */                                          \
            size -= DBGMEM_INFO_ALLOCSIZE_MIN;                                               \
        }                                                                                    \
                                                                                             \
        if( DBGMEM_INFO_GETALLOCSIZE() && (g_brcm_dbgmem_info.elements == NULL) )             \
        { /* failed allocation, then disable */                                              \
            printf(  "DBGMEM Info: Failed requested allocation size[%0x08x], disabling\n",   \
						DBGMEM_INFO_GETALLOCSIZE() );										\
																							\
		DBGMEM_INFO_SETENABLE( 0 );															\
        }                                                                                    \
    } else                                                                                   \
	{																						\
		DBGMEM_INFO_SETENABLE( 0 );															\
	}																						\
} while( 0 );


#define DBGMEM_INFO_CLEANUP()																\
do {																						\
	if( DBGMEM_INFO_GETALLOCED() && g_brcm_dbgmem_info.elements ) {							\
		DBGMEM_INFO_SETENABLE( 0 );															\
		DBGMEM_INFO_SETALLOCED( 0 );														\
		IOFree(g_brcm_dbgmem_info.elements, DBGMEM_INFO_GETALLOCSIZE());					\
		g_brcm_dbgmem_info.elements = NULL;													\
	}																						\
} while( 0 );

#endif /* !BCMDBG */

#else /* !ENABLE_DBGMEM */

// NULL macros if feature is not enabled
#define DBGMEM_INFO_INITIALIZE()
#define DBGMEM_INFO_CLEANUP()

#endif /* !ENABLE_DBGMEM */



/* Global ASSERT type */
uint32 g_assert_type = 1; /* Don't kernel panic by default,     0: panic, 1: print */

static errno_t	osl_add_pkttag(osl_t *osh, mbuf_t m);
static void	osl_free_private_mbufs(osl_t *osh);
static bool	osl_is_private_mbuf(mbuf_t m);
static void	osl_mbuf_enqueue(mbuf_t *list, mbuf_t m);
static mbuf_t	osl_mbuf_dequeue(mbuf_t *list);
static UInt64	osl_ram_size();
static bool	osl_mchain_high_phys_addr(osl_t *osh, mbuf_t m);

static mbuf_tag_id_t g_osl_mbuf_tag_id = {0};	// mbuf tag ID created at startup

static errno_t bcmerrormap[] =
{	0, 		/* 0 */
	ENXIO,		/* BCME_ERROR */
	ENXIO,		/* BCME_BADARG */
	ENXIO,		/* BCME_BADOPTION */
	EPWROFF,	/* BCME_NOTUP */
	ENXIO,		/* BCME_NOTDOWN */
	ENXIO,		/* BCME_NOTAP */
	ENXIO,		/* BCME_NOTSTA */
	ENXIO,		/* BCME_BADKEYIDX */
	ENXIO,		/* BCME_RADIOOFF */
	ENODEV,		/* BCME_NOTBANDLOCKED */
	ENXIO, 		/* BCME_NOCLK */
	ENODEV, 	/* BCME_BADRATESET */
	ENODEV, 	/* BCME_BADBAND */
	ENXIO,		/* BCME_BUFTOOSHORT */
	ENXIO,		/* BCME_BUFTOOLONG */
	EBUSY, 		/* BCME_BUSY */
	ENXIO, 		/* BCME_NOTASSOCIATED */
	ENXIO, 		/* BCME_BADSSIDLEN */
	ENODEV, 	/* BCME_OUTOFRANGECHAN */
	EINVAL, 	/* BCME_BADCHAN */
	EINVAL, 	/* BCME_BADADDR */
	ENXIO, 		/* BCME_NORESOURCE */
	EOPNOTSUPP,	/* BCME_UNSUPPORTED */
	EINVAL,		/* BCME_BADLENGTH */
	ENXIO,		/* BCME_NOTREADY */
	ENXIO,		/* BCME_NOTPERMITTED */
	ENOMEM, 	/* BCME_NOMEM */
	ENXIO, 		/* BCME_ASSOCIATED */
	EINVAL, 	/* BCME_RANGE */
	ENXIO, 		/* BCME_NOTFOUND */
	ENXIO,		/* BCME_WME_NOT_ENABLED */
	ENXIO,		/* BCME_TSPEC_NOTFOUND */
	ENXIO,		/* BCME_ACM_NOTSUPPORTED */
	ENXIO,		/* BCME_NOT_WME_ASSOCIATION */
	ENXIO,		/* BCME_SDIO_ERROR */
	ENXIO,		/* BCME_DONGLE_DOWN */
	EINVAL,		/* BCME_VERSION */
	ENXIO,		/* BCME_TXFAIL */
	ENXIO,		/* BCME_RXFAIL */
	EINVAL,		/* BCME_NODEVICE */
	EINVAL,		/* BCME_NMODE_DISABLED */
	EINVAL,		/* BCME_NONRESIDENT */
	EINVAL,		/* BCME_SCANREJECT */
	EINVAL,		/* BCME_USAGE_ERROR */
	EIO,		/* BCME_IOCTL_ERROR */
	EIO,		/* BCME_SERIAL_PORT_ERR */
	EOPNOTSUPP,	/* BCME_DISABLED */
	ENXIO,		/* BCME_DECERR */
	ENXIO,		/* BCME_ENCERR */
	ENXIO,		/* BCME_MICERR */
	ENXIO,		/* BCME_REPLAY */
	ENODEV,		/* BCME_IE_NOTFOUND */
	ENXIO,		/* BCME_DATA_NOTFOUND */
	EINVAL,     /* BCME_NOT_GC */
	EINVAL,     /* BCME_PRS_REQ_FAILED */
	EINVAL,     /* BCME_NO_P2P_SE */
	EINVAL,     /* BCME_NOA_PND */
	EINVAL,     /* BCME_FRAG_Q_FAILED */
	EINVAL,     /* BCME_GET_AF_FAILED */
	EINVAL,		/* BCME_MSCH_NOTREADY */
	EINVAL,		/* BCME_IOV_LAST_CMD */
};

/* When an new error code is added to bcmutils.h, add os
 * spcecific error translation here as well
 */
/* check if BCME_LAST changed since the last time this table was updated */
#if BCME_LAST != -61
#error "You need to add an OS error translation in the bcmerrormap \
	for new error code defined in bcmutils.h"
#endif

/*
 * Print to local system log versus printing to firewire.
 */
#if !defined(PRINT_FIREWIRE)   /* Over-rideable from project/makefile */
#define PRINT_FIREWIRE 0
#endif
static u_int brcmfirewirelog = PRINT_FIREWIRE;

osl_t *
osl_attach(IOPCIDevice *pcidev)
{
	osl_t *osh = (osl_t*)IOMalloc(sizeof(osl_t));
	errno_t err = BCME_OK;

	if (!osh) {
		printf("osl_attach: IOMalloc failed to allocate %lu bytes for osl_t\n",
		       sizeof(osl_t));
		goto error;
	}

	bzero(osh, sizeof(osl_t));

	/* Check that error map has the right number of entries in it */
	ASSERT(ABS(BCME_LAST) == (ARRAYSIZE(bcmerrormap) - 1));

	osh->magic = OS_HANDLE_MAGIC;
	osh->pcidev = pcidev;
#ifdef BCMDBG_MEM
	g_brcm_dbgmem_list = NULL;
#endif /* BDMDBG_MEM */

	DBGMEM_INFO_INITIALIZE();

	/* packet tag setup */
	err = mbuf_tag_id_find(OSL_MBUF_TAG_MODULE, &g_osl_mbuf_tag_id);
	if (err != 0) {
		printf("osl_attach: mbuf_tag_id_find() returned error %d\n", err);
		goto error;
	}

#ifdef BCMDMASGLISTOSL
	osh->mbuf_cursor = IOMbufNaturalMemoryCursor::withSpecification( PAGE_SIZE, MAX_DMA_SEGS );
	if( !osh->mbuf_cursor )
		goto error;

#endif /* BCMDMASGLISTOSL */

	sysctlattach(osh);

#ifdef PCI_ACCESS_HISTORY
	if ((err = osl_pci_access_history_init(osh)) != BCME_OK) {
		goto error;
	}
#endif /* PCI_ACCESS_HISTORY */

	return osh;

error:
	if (osh) {
#ifdef BCMDMASGLISTOSL
		if( osh->mbuf_cursor )
			osh->mbuf_cursor->release();
#endif /* BCMDMASGLISTOSL */

		IOFree(osh, sizeof(osl_t));
	}

	return NULL;
}

#if defined(BCM_BACKPLANE_TIMEOUT)
void osl_set_sih(osl_t * osh, void * sih)
{
	osh->sih = sih;
}

void osl_set_wl(osl_t * osh, void * wl, wl_fatal_error_fp cb, bool checkAllWrappers)
{
	osh->wl = wl;
	osh->wl_fatal_error_cb = cb;
	osh->checkAllWrappers = checkAllWrappers;
}
#endif

void osl_detach(osl_t *osh)
{
	if (!osh)
		return;

	osl_free_private_mbufs(osh);

#ifdef BCMDMASGLISTOSL
	if( osh->mbuf_cursor )
		osh->mbuf_cursor->release();
#endif /* BCMDMASGLISTOSL */

#ifdef PCI_ACCESS_HISTORY
	osl_pci_access_history_deinit(osh);
#endif /* PCI_ACCESS_HISTORY */

	if (MALLOCED(osh)) {
		printf("Memory leak of bytes %d\n", MALLOCED(osh));
#ifdef BCMDBG_MEM
		MALLOC_DUMP(osh, NULL);
#endif
	}

	sysctldetach(osh);
	IOFree(osh, sizeof(osl_t));

	DBGMEM_INFO_CLEANUP();
}

void
osl_pktfree_cb_set(osl_t *osh, pktfree_cb_fn_t tx_fn, void *tx_ctx)
{
	osh->tx_fn = tx_fn;
	osh->tx_ctx = tx_ctx;
}

/* translate bcmerrors into MacOSX errors */
int
osl_error(int bcmerror)
{
	if (bcmerror > 0)
		bcmerror = 0;
	else if (bcmerror < BCME_LAST)
		bcmerror = BCME_ERROR;

	/* Array bounds covered by ASSERT in osl_attach */
	return bcmerrormap[-bcmerror];
}

void *
osl_malloc(osl_t *osh, uint size)
{
	void *addr = NULL;

	ASSERT(size);

	/* only ASSERT if osh is defined */
	if (osh)
		ASSERT(osh->magic == OS_HANDLE_MAGIC);

	if ((addr = IOMalloc(size)) == NULL) {
		if (osh)
			osh->failed++;
	} else {
		if (osh)
			osh->malloced += size;
	}

	DBGMEM_INFO_RECORDTRACE( DBGMEM_INFO_TYPE_MALLOC, 0x0 /* tag */,
		__FUNCTION__ /* tagstr */,
		__FILE__ /* file */, __LINE__ /* line */,
		OSL_SYSUPTIME(), (size_t)size, (void*)addr,
		(void*)NULL /* ptrdata */,
		(void*)NULL /* physa */,
		(void*)NULL /* arg0 */, (void*)NULL /* arg1 */, (void*)NULL /* arg2 */, (void*)NULL /* arg3 */ );

	return (addr);
}

void *
osl_mallocz(osl_t *osh, uint size)
{
	void *ptr = osl_malloc(osh, size);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void
osl_mfree(osl_t *osh, void *addr, uint size)
{
#ifdef BCMDBG
	deadbeef((char*)addr, size);
#endif /* BCMDBG */
	if (osh) {
		ASSERT(osh->magic == OS_HANDLE_MAGIC);
		osh->malloced -= size;
	}

	DBGMEM_INFO_RECORDTRACE( DBGMEM_INFO_TYPE_FREE, 0x0 /* tag */,
		__FUNCTION__ /* tagstr */,
		__FILE__ /* file */, __LINE__ /* line */,
		OSL_SYSUPTIME(), (size_t)size, (void*)addr,
		(void*)NULL /* ptrdata */,
		(void*)NULL /* physa */,
		(void*)NULL /* arg0 */, (void*)NULL /* arg1 */, (void*)NULL /* arg2 */, (void*)NULL /* arg3 */ );

	IOFree(addr, size);
}

uint
osl_malloced(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	return (osh->malloced);
}

uint
osl_malloc_failed(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	return (osh->failed);
}

#ifdef BCMDBG_MEM
void*
osl_debug_malloc(osl_t *osh, uint size, int line, const char* file)
{
	bcm_mem_link_t *p = NULL;
	const char* basename = NULL;

	ASSERT(size);

	if ((p = (bcm_mem_link_t*)osl_malloc(osh, sizeof(bcm_mem_link_t) + size)) == NULL)
		return (NULL);

	p->timestamp = OSL_SYSUPTIME();
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
	p->next = g_brcm_dbgmem_list;
	if (p->next)
		p->next->prev = p;
	g_brcm_dbgmem_list = p;

	DBGMEM_INFO_RECORDTRACE( DBGMEM_INFO_TYPE_MALLOC, 0 /* tag */,
		__FUNCTION__ /* tagstr */,
		file, line,
		p->timestamp, (size_t)size, (p+1),
		(void*)p /* ptrdata */,
		(void*)NULL /* physa */,
		(void*)NULL /* arg0 */, (void*)NULL /* arg1 */, (void*)NULL /* arg2 */, (void*)NULL /* arg3 */ );

	return (void *)(p + 1);
}

void*
osl_debug_mallocz(osl_t *osh, uint size, int line, const char* file)
{
	void *ptr = osl_debug_malloc(osh, size, line, file);
	
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

	//ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (p->size == 0) {
		WL_NONE(("osl_debug_mfree: double free on addr %p size %d at line %d file %s\n",
			 OSL_OBFUSCATE_BUF(addr), size, line, file));
		ASSERT(p->size);
		return;
	}

	if (p->size != size) {
		WL_NONE(("osl_debug_mfree: dealloc size %d does not match alloc size %d on addr %p"
		       " at line %d file %s\n",
		       size, p->size, OSL_OBFUSCATE_BUF(addr), line, file));
		ASSERT(p->size == size);
		return;
	}

	/* unlink this block */
	if (p->prev)
		p->prev->next = p->next;
	if (p->next)
		p->next->prev = p->prev;
	if (g_brcm_dbgmem_list == p)
		g_brcm_dbgmem_list = p->next;
	p->next = p->prev = NULL;

	DBGMEM_INFO_RECORDTRACE( DBGMEM_INFO_TYPE_FREE, 0x0 /* tag */,
		__FUNCTION__ /* tagstr */,
		file, line,
		OSL_SYSUPTIME(), (size_t)size, (void*)addr,
		(void*)p /* ptrdata */,
		(void*)NULL /* physa */,
		(void*)NULL /* arg0 */, (void*)NULL /* arg1 */, (void*)NULL /* arg2 */, (void*)NULL /* arg3 */ );

	osl_mfree(osh, (char *)p, size + sizeof(bcm_mem_link_t));
}

int
osl_debug_memdump(osl_t *osh, struct bcmstrbuf *b)
{
	bcm_mem_link_t *p = NULL;

	//ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (b) {
		bcm_bprintf(b, "Address                Size      age (ms)  File:line\n");
	} else {
		printf("Address                Size      age (ms)  File:line\n");
	}
	for (p = g_brcm_dbgmem_list; p; p = p->next) {
		uint64_t age = OSL_SYSUPTIME() - p->timestamp;
		if (b) {
			bcm_bprintf(b, "%p  %7d   %7u.%03u  %s:%d\n",
				OSL_OBFUSCATE_BUF(p + 1), p->size, (age/1000), (age%1000), p->file, p->line);
		} else {
			printf("%p  %7d   %7u.%03u  %s:%d\n",
				OSL_OBFUSCATE_BUF(p + 1), p->size, (age/1000), (age%1000), p->file, p->line);
		}
	}
	return 0;
}

#endif  /* BCMDBG_MEM */

void *
osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align_bits, uint *alloced,
	dmaaddr_t *pa, void **dmah)
{
	vm_address_t				virtual_addr = NULL;
	IOBufferMemoryDescriptor	*descr = NULL;
	IOReturn					result = 0;
	uint64 align = (1ul << align_bits);
	uint64 addrmask = 0;

	/* fix up the alignment requirements first */
	align = MAX(align, PAGE_SIZE);
#ifdef BCMDMA64OSL
	addrmask = 0xFFFFFFFFFFFFFFFFULL - (align - 1);
#else
	addrmask = 0x00000000FFFFFFFFULL - (align - 1);
#endif

	*alloced = size;
	ASSERT(pa);
	if (!pa) return 0;		// sanity check

	ASSERT(osh->dma_addrwidth != 0);
	// allocate the descriptor and memory in one fell swoop
	descr = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task,
					kIOMemoryPhysicallyContiguous|kIODirectionInOut,
					size, addrmask);
	ASSERT( descr );
	if (descr == 0)
		return 0;

	result = descr->prepare();		// finish setting it up
	ASSERT( result == kIOReturnSuccess );
	if (result != kIOReturnSuccess) {
		descr->release();
		return 0;
	}

	IODMACommand *dmaCmd = NULL;
#ifdef BCMDMA64OSL
	if (osh->dma_addrwidth == DMADDRWIDTH_64) {
		// allocate an IODMACommand
		dmaCmd = IODMACommand::withSpecification(
			 kIODMACommandOutputHost64,	// SegmentFunction--outSegFunc,
			 64,			// UInt8		numAddressBits,
			 size,				// UInt64		maxSegmentSize,
			 IODMACommand::kMapped,		// MappingOptions	mappingOptions = kMapped,
			 size,				// UInt64		maxTransferSize = 0,
			 1,				// UInt32		alignment = 1,
			 0,				// IOMapper		*mapper = 0,
			 NULL);				// void			*refCon = 0);
	} else {
#endif
		// allocate an IODMACommand
		dmaCmd = IODMACommand::withSpecification(
			 kIODMACommandOutputHost32,	// SegmentFunction--outSegFunc,
			 32,			// UInt8		numAddressBits,
			 size,				// UInt64		maxSegmentSize,
			 IODMACommand::kMapped,		// MappingOptions	mappingOptions = kMapped,
			 size,				// UInt64		maxTransferSize = 0,
			 1,				// UInt32		alignment = 1,
			 0,				// IOMapper		*mapper = 0,
			 NULL);				// void			*refCon = 0);
#ifdef BCMDMA64OSL
	}
#endif
	ASSERT( dmaCmd );
	if (dmaCmd == 0)
		return 0;

	// tell the DMA Command about our memory (does a DMACommand->prepare() as part of this)
	result = dmaCmd->setMemoryDescriptor(descr);
	ASSERT( result == kIOReturnSuccess );
	if (result != kIOReturnSuccess)
		return 0;

	// get virtual address
	virtual_addr = (vm_address_t)descr->getBytesNoCopy();
	ASSERT( virtual_addr );

	// get physical address
#ifdef BCMDMA64OSL
	if (osh->dma_addrwidth == DMADDRWIDTH_64) {
		IODMACommand::Segment64	physSegment64;
		UInt32			numSegments = 1;
		UInt64			offset = 0;
		result = dmaCmd->gen64IOVMSegments( &offset, &physSegment64, &numSegments );

		ASSERT( !result && ( numSegments == 1 ) && physSegment64.fLength >= size);
		if ( result || ( numSegments != 1 ) || physSegment64.fLength < size)
			return 0;

		PHYSADDRLOSET(*pa, (uint32)(physSegment64.fIOVMAddr));
		PHYSADDRHISET(*pa, (uint32)(physSegment64.fIOVMAddr >> 32));
	} else {
#endif
		IODMACommand::Segment32	physSegment32;
		UInt32					numSegments = 1;
		UInt64					offset = 0;
		result = dmaCmd->gen32IOVMSegments( &offset, &physSegment32, &numSegments );

		ASSERT( !result && ( numSegments == 1 ) && physSegment32.fLength >= size);
		if ( result || ( numSegments != 1 ) || physSegment32.fLength < size)
			return 0;

		PHYSADDRLOSET(*pa, physSegment32.fIOVMAddr);
		PHYSADDRHISET(*pa, 0);
#ifdef BCMDMA64OSL
	}
#endif
	*dmah = (void *)dmaCmd;

	DBGMEM_INFO_RECORDTRACE( DBGMEM_INFO_TYPE_DMAALLOC, (0x0d3ac06400000000ULL) /* tag */,
		__FUNCTION__ /* tagstr */,
		__FILE__ /* file */, __LINE__ /* line */,
		OSL_SYSUPTIME(),
		(size_t)size, (void*)virtual_addr, (void*)dmaCmd,
		(void*)( ((((uint64_t)PHYSADDRHI(*pa))  << 32) | (PHYSADDRLO(*pa))) ) /* physa */,
		(void*)((uint64_t)align_bits) /* arg0 */, (void*)NULL /* arg1 */,
		(void*)NULL /* arg2 */, (void*)NULL /* arg3 */ );

	return (void *)virtual_addr;
}

void
osl_dma_free_consistent(osl_t *osh, void *va, uint size, void *dmah)
{
	IODMACommand *dmaCmd = (IODMACommand *)dmah;

	BCM_REFERENCE(osh);
	BCM_REFERENCE(va);
	BCM_REFERENCE(size);

	if (!dmaCmd)
		return;

	IOBufferMemoryDescriptor *desc = OSDynamicCast(	IOBufferMemoryDescriptor,
	                                                dmaCmd->getMemoryDescriptor() );

	DBGMEM_INFO_RECORDTRACE( DBGMEM_INFO_TYPE_DMAFREE, (0x0d3acff1dead0000ULL) /* tag */,
		__FUNCTION__ /* tagstr */,
		__FILE__ /* file */, __LINE__ /* line */,
		OSL_SYSUPTIME(),
		(size_t)size, (void*)va, (void*)dmaCmd,
		(void*)desc /* physa */,
		(void*)NULL /* arg0 */, (void*)NULL /* arg1 */,
		(void*)NULL /* arg2 */, (void*)NULL /* arg3 */ );


	if (desc) {
		desc->complete();			// clean up memory
		dmaCmd->clearMemoryDescriptor();	// remove from IODMACommand
							// (also does DMA->complete() for us)
		desc->release();			// free memory
	}

	dmaCmd->release();
}

void *
osl_dma_alloc_consistent_force32(osl_t *osh, uint size, uint16 align_bits, uint *alloced,
	dmaaddr_t *pa, void **dmah)
{
	vm_address_t				virtual_addr = NULL;
	IOBufferMemoryDescriptor	*descr = NULL;
	IOReturn					result = 0;
	uint64 align = (1ul << align_bits);

	ASSERT(dmah && *dmah);

	/* fix up the alignment requirements first */
	align = MAX(align, PAGE_SIZE);

	*alloced = size;

	if (!pa) return 0;		// sanity check

	ASSERT(osh->dma_addrwidth != 0);
	// Allocate the buffer
	// Need memory buffer to be in lower memory (32-bit addressable)
	descr = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task,
					kIOMemoryPhysicallyContiguous|kIODirectionInOut,
					 size, (0x00000000FFFFFFFFULL - (align-1)));
	ASSERT(descr);
	if (descr == 0)
		return 0;

	result = descr->prepare();		// finish setting it up
	ASSERT(result == kIOReturnSuccess);
	if (result != kIOReturnSuccess) {
		descr->release();
		return 0;
	}

	IODMACommand *dmaCmd = NULL;
#ifdef BCMDMA64OSL
	if (osh->dma_addrwidth == DMADDRWIDTH_64) {
		// allocate an IODMACommand
		dmaCmd = IODMACommand::withSpecification(
			 kIODMACommandOutputHost64,	// SegmentFunction--outSegFunc,
			 32,			// UInt8		numAddressBits,
			 size,				// UInt64		maxSegmentSize,
			 IODMACommand::kMapped,		// MappingOptions	mappingOptions = kMapped,
			 size,				// UInt64		maxTransferSize = 0,
			 1,				// UInt32		alignment = 1,
			 0,				// IOMapper		*mapper = 0,
			 NULL);				// void			*refCon = 0);
	} else {
#endif
		// allocate an IODMACommand
		dmaCmd = IODMACommand::withSpecification(
			 kIODMACommandOutputHost32,	// SegmentFunction--outSegFunc,
			 32,			// UInt8		numAddressBits,
			 size,				// UInt64		maxSegmentSize,
			 IODMACommand::kMapped,		// MappingOptions	mappingOptions = kMapped,
			 size,				// UInt64		maxTransferSize = 0,
			 1,				// UInt32		alignment = 1,
			 0,				// IOMapper		*mapper = 0,
			 NULL);				// void			*refCon = 0);
#ifdef BCMDMA64OSL
	}
#endif
	ASSERT(dmaCmd);
	if (dmaCmd == 0)
		return 0;

	// tell the DMA Command about our memory (does a DMACommand->prepare() as part of this)
	result = dmaCmd->setMemoryDescriptor(descr);
	ASSERT(result == kIOReturnSuccess);
	if (result != kIOReturnSuccess)
		return 0;

	// get virtual address
	virtual_addr = (vm_address_t)descr->getBytesNoCopy();
	ASSERT(virtual_addr);

	// get physical address
#ifdef BCMDMA64OSL
	if (osh->dma_addrwidth == DMADDRWIDTH_64) {
		IODMACommand::Segment64	physSegment64;
		UInt32			numSegments = 1;
		UInt64			offset = 0;
		result = dmaCmd->gen64IOVMSegments(&offset, &physSegment64, &numSegments);

		ASSERT(!result && (numSegments == 1) && physSegment64.fLength >= size);
		if (result || (numSegments != 1) || physSegment64.fLength < size)
			return 0;

		PHYSADDRLOSET(*pa, (uint32)(physSegment64.fIOVMAddr));
		PHYSADDRHISET(*pa, (uint32)(physSegment64.fIOVMAddr >> 32));
	} else {
#endif
		IODMACommand::Segment32	physSegment32;
		UInt32			numSegments = 1;
		UInt64			offset = 0;
		result = dmaCmd->gen32IOVMSegments(&offset, &physSegment32, &numSegments);

		ASSERT(!result);
		ASSERT(numSegments == 1);
		ASSERT(physSegment32.fLength >= size);
		if (result || (numSegments != 1) || physSegment32.fLength < size)
			return 0;

		PHYSADDRLOSET(*pa, physSegment32.fIOVMAddr);
		PHYSADDRHISET(*pa, 0);
#ifdef BCMDMA64OSL
	}
#endif
	*dmah = (void *)dmaCmd;

	DBGMEM_INFO_RECORDTRACE( DBGMEM_INFO_TYPE_DMAALLOC, (0x0d3ac03200000000ULL) /* tag */,
		__FUNCTION__ /* tagstr */,
		__FILE__ /* file */, __LINE__ /* line */,
		OSL_SYSUPTIME(),
		(size_t)size, (void*)virtual_addr, (void*)dmaCmd,
		(void*)( (((uint64_t)PHYSADDRHI(*pa) << 32) | (PHYSADDRLO(*pa))) ) /* physa */,
		(void*)((uint64_t)align_bits) /* arg0 */, (void*)NULL /* arg1 */,
		(void*)NULL /* arg2 */, (void*)NULL /* arg3 */ );

	return (void *)virtual_addr;
}

mbuf_t
osl_pktget(osl_t *osh, uint len)
{
	uint maxchunks = 1;
	mbuf_t m = NULL;
	mbuf_t osl_pkt = NULL;
	errno_t err = BCME_OK;

	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));

	if (len > MCLBYTES) {
#ifdef BCMDBG
		printf("%s: len (%d) > MCLBYTES\n", __FUNCTION__, len);
#endif
		return NULL;
	}

	(void)mbuf_allocpacket(MBUF_WAITOK, len, &maxchunks, &m);
	if (!m)
		return NULL;

	// if required, check the mapped address and use a private mbuf to replace
	if (osh->use_private_mbufs && osl_mchain_high_phys_addr(osh, m)) {
		mbuf_freem(m);
		m = NULL;
		osl_pkt = osl_mbuf_dequeue(&osh->private_pool);
		if (!osl_pkt)
			return NULL;
		if (mbuf_setdata(osl_pkt, mbuf_datastart(osl_pkt), len)) {
			osl_mbuf_enqueue(&osh->private_pool, osl_pkt);
			return NULL;
		}
	}

	// if we are still using a newly allocated mbuf, prepare for driver use
	if (m) {
		mbuf_setlen(m, len);
		err = osl_add_pkttag(osh, m);
		if (err)
			mbuf_freem(m);
		else
			osl_pkt = m;
	}

	if( osl_pkt ) {
		DBGMEM_INFO_RECORDTRACE( DBGMEM_INFO_TYPE_MBUFALLOC, 0x0 /* tag */,
			__FUNCTION__ /* tagstr */,
			__FILE__ /* file */, __LINE__ /* line */,
			OSL_SYSUPTIME(),
			(size_t)mbuf_len(osl_pkt), (void*)osl_pkt, (void*)mbuf_data(osl_pkt),
			(void*)mbuf_data_to_physical( mbuf_data(osl_pkt) ) /* physa */,
			(void*)NULL /* arg0 */, (void*)NULL /* arg1 */, (void*)NULL /* arg2 */, (void*)NULL /* arg3 */ );
	}

	return osl_pkt;
}

void
osl_pktfree(osl_t *osh, mbuf_t m, bool send)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));
	ASSERT(m);

	DBGMEM_INFO_RECORDTRACE( DBGMEM_INFO_TYPE_MBUFFREE, 0x0 /* tag */,
		__FUNCTION__ /* tagstr */,
		__FILE__ /* file */, __LINE__ /* line */,
		OSL_SYSUPTIME(),
		(size_t)mbuf_len(m), (void*)m, (void*)mbuf_data(m),
		(void*)mbuf_data_to_physical( mbuf_data(m) ) /* physa */,
		(void*)NULL /* arg0 */, (void*)NULL /* arg1 */, (void*)NULL /* arg2 */, (void*)NULL /* arg3 */ );

	if (send && osh->tx_fn)
		osh->tx_fn(osh->tx_ctx, m, 0);

	// if we are not using private pool mbufs, use the the regular mbuf_freem() call
	if (!osh->use_private_mbufs) {
		mbuf_freem(m);
		return;
	}

	// walk the mbuf chain freeing the system mbufs
	// and returning private mbufs to our private pool
	while (m) {
		if (osl_is_private_mbuf(m)) {
			void *pdata = NULL;
			mbuf_t n = mbuf_next(m);

			// reset the next pointer and zero the pkttag data
			mbuf_setnext(m, NULL);
			pdata = osl_pkt_gettag(m);
			if (pdata)
				bzero(pdata, OSL_PKTTAG_SZ);
			osl_mbuf_enqueue(&osh->private_pool, m);

			m = n;
		} else {
			m = mbuf_free(m);
		}
	}
}

mbuf_t
osl_pktdup(osl_t *osh, mbuf_t p)
{
	mbuf_t dup = NULL;
	size_t len = 0;
	errno_t err = BCME_OK;

	// the same room as incoming pkt.
	MBUF_PANICPARANOIDCHECK( p, __FUNCTION__, __LINE__ );

	len = mbuf_len(p);
	dup = osl_pktget(osh, (uint)len);
	if (dup) {
		mbuf_setdata(dup, mbuf_datastart(dup), len);
		err = mbuf_copydata(p, 0, len, mbuf_data(dup));
		if (err) {
			osl_pktfree(osh, dup, FALSE);
			return NULL;
		}
	}

	return dup;
}

uint8 *
osl_pktpush(osl_t *osh, mbuf_t m, uint nbytes)
{
	uint8 *data = NULL;

	BCM_REFERENCE(osh);

	MBUF_PANICPARANOIDCHECK( m, __FUNCTION__, __LINE__ );
	ASSERT(mbuf_flags(m) & MBUF_PKTHDR);
	ASSERT(nbytes <= mbuf_leadingspace(m));

	data = (uint8*)mbuf_data(m);
	data -= nbytes;

	mbuf_setdata(m, data, mbuf_len(m) + nbytes);

	return data;
}

uint8 *
osl_pktpull(osl_t *osh, mbuf_t m, uint nbytes)
{
	uint8 *data = BCME_OK;

	BCM_REFERENCE(osh);

	MBUF_PANICPARANOIDCHECK( m, __FUNCTION__, __LINE__ );
	ASSERT(nbytes <= mbuf_len(m));

	data = (uint8*)mbuf_data(m);
	data += nbytes;
	mbuf_setdata(m, data, mbuf_len(m) - nbytes);

	return data;
}

static errno_t
osl_add_pkttag(osl_t *osh, mbuf_t m)
{
	errno_t err = BCME_OK;
	void *pdata = NULL;

	BCM_REFERENCE(osh);

	err = mbuf_tag_allocate(m, g_osl_mbuf_tag_id, OSL_MBUF_TAG_TYPE_PKTTAG,
	                        OSL_PKTTAG_SZ, MBUF_WAITOK, &pdata);

	if (!err) {
		ASSERT(pdata != NULL);
		bzero(pdata, OSL_PKTTAG_SZ);
	}

	return err;
}

void *
osl_pkt_gettag(mbuf_t m)
{
	size_t length = 0;
	errno_t err = BCME_OK;
	void *pdata = NULL;

	MBUF_PANICPARANOIDCHECK( m, __FUNCTION__, __LINE__ );
	ASSERT(mbuf_flags(m) & MBUF_PKTHDR);

	err = mbuf_tag_find(m, g_osl_mbuf_tag_id, OSL_MBUF_TAG_TYPE_PKTTAG, &length, &pdata);

	ASSERT(!err);
	ASSERT(length == OSL_PKTTAG_SZ);

	return pdata;
}

mbuf_t
osl_pkt_frmnative(osl_t *osh, mbuf_t m)
{
	errno_t err = BCME_OK;
	mbuf_t priv_m = NULL;
	int totlen = 0;
	mbuf_t osl_pkt = NULL;

	// replace the mbuf with one from our private pool if needed
	if (osh->use_private_mbufs && osl_mchain_high_phys_addr(osh, m)) {
		priv_m = osl_mbuf_dequeue(&osh->private_pool);
		totlen = pkttotlen(osh, m);
		if (priv_m == NULL || totlen > (int)mbuf_maxlen(priv_m))
			return NULL;
		mbuf_setdata(priv_m, mbuf_datastart(priv_m), totlen);
		err = mbuf_copydata(m, 0, totlen, mbuf_data(priv_m));
		if (err) {
			osl_mbuf_enqueue(&osh->private_pool, priv_m);
			return NULL;
		}
		mbuf_freem(m);
		osl_pkt = priv_m;
	} else {
		err = osl_add_pkttag(osh, m);
		if (err)
			return NULL;
		osl_pkt = m;
	}

	return osl_pkt;
}

mbuf_t
osl_pkt_tonative(osl_t *osh, mbuf_t m)
{
	mbuf_t n = NULL;
	bool priv = FALSE;
	size_t totlen = 0;

	ASSERT(m);

	// decide what to do based on presence of the private pool
	// clac totlen in the process
	if (osh->use_private_mbufs) {
		// Check for a private mbuf on the chain, and replace the entire
		// chain if found
		n = m;
		while (n) {
			if (!priv)
				priv = osl_is_private_mbuf(n);
			totlen += mbuf_len(n);
			n = mbuf_next(n);
		}
	} else {
		totlen = pkttotlen(osh, m);
	}

	// If there is a private pool mbuf, free the original chain
	// and return a duplicate.
	if (priv) {
		uint maxchunks = 1;
		mbuf_t new_m = NULL;

		(void)mbuf_allocpacket(MBUF_WAITOK, totlen, &maxchunks, &new_m);
		if (new_m) {
			mbuf_setdata(new_m, mbuf_datastart(new_m), totlen);
			mbuf_copydata(m, 0, totlen, (void *)mbuf_data(new_m));
			mbuf_pkthdr_setlen(new_m, totlen);
		}

		// free original chain
		osl_pktfree(osh, m, FALSE);

		// return the new mubf from the function
		return new_m;
	} else {
		// Otherwise, just clean off our packet tags and return this mbuf chain
		n = m;
		while (n) {
			mbuf_tag_free(n, g_osl_mbuf_tag_id, OSL_MBUF_TAG_TYPE_PKTTAG);
			n = mbuf_next(n);
		}
		mbuf_pkthdr_setlen(m, totlen);
		return m;
	}
}

/* Map Apple80211 AC to priority */
int
osl_getprio(mbuf_t m)
{
	long ac = APPLE80211_MBUF_WME_AC(m);
	switch (ac) {
	case APPLE80211_WME_AC_BE:
		return PRIO_8021D_BE;
	case APPLE80211_WME_AC_BK:
		return PRIO_8021D_BK;
	case APPLE80211_WME_AC_VI:
		return PRIO_8021D_VI;
	case APPLE80211_WME_AC_VO:
		return PRIO_8021D_VO;
	default:
		ASSERT(0);
		return 0;
	};
	return 0;
}

void
osl_setprio(mbuf_t m, int prio)
{
	int ac = 0;
	switch (prio) {
	case PRIO_8021D_NONE:
	case PRIO_8021D_BK:
		ac = APPLE80211_WME_AC_BK;
		break;
	case PRIO_8021D_BE:
	case PRIO_8021D_EE:
		ac = APPLE80211_WME_AC_BE;
		break;
	case PRIO_8021D_CL:
	case PRIO_8021D_VI:
		ac = APPLE80211_WME_AC_VI;
		break;
	case PRIO_8021D_VO:
	case PRIO_8021D_NC:
		ac = APPLE80211_WME_AC_VO;
		break;
	default:
		ASSERT(0);
	};

	APPLE80211_MBUF_SET_WME_AC(m, ac);
}

#if defined(BCM_BACKPLANE_TIMEOUT)

#ifdef STATS_REG_RET_FFFFFFFF

validRegReturningFF_t validRegRetFF;

/* Returns number of valid register reads returning -1 */
void getValidReadRegCntReturningFF(validRegReturningFF_t * ptr)
{
	/* Add the total reg returning -1 as valid value */
	validRegRetFF.total_32 += validRegRetFF.count_32;
	validRegRetFF.total_16 += validRegRetFF.count_16;
	validRegRetFF.total_8 += validRegRetFF.count_8;

	/* Copy the counters */
	*ptr = validRegRetFF;

	/* reset the current counters */
	validRegRetFF.count_32 = 0;
	validRegRetFF.count_16 = 0;
	validRegRetFF.count_8 = 0;

	return;
}

#endif /* STATS_REG_RET_FFFFFFFF */

uint32
#if defined(BCMDBG) && defined(STATS_REG_RET_FFFFFFFF)
read_bpt_reg(osl_t *osh, volatile void *r, uint32 size, const char * f, int l)
#else
read_bpt_reg(osl_t *osh, volatile void *r, uint32 size)
#endif
{
	uint32 status = 0;
	static int in_si_clear = FALSE;
	bool poll_timeout = FALSE;

	osl_pci_access_record(osh, PCI_ACCESS_TYPE_MMIO_READ, r, 0, size, 0);

	if (size == 4) {
		status = OSReadLittleInt32((uint32*)(r), 0);
		if (status == 0xFFFFFFFF) {
			poll_timeout = TRUE;
		}
	}
	else if (size == 2) {
		status = OSReadLittleInt16((uint16*)(r), 0);
		if (status == 0xFFFF) {
			poll_timeout = TRUE;
		}
	}
	else {
		status = *(volatile uint8*)(r);
		if (status == 0xFF) {
			poll_timeout = TRUE;
		}
	}

	osl_pci_access_record(osh, PCI_ACCESS_TYPE_MMIO_READ_COMP, r, status, size, 0);

	if (osh->sih && (in_si_clear == FALSE) && poll_timeout) {
		in_si_clear = TRUE;
		uint32 bpt_error;

		if (osh->checkAllWrappers) {
			bpt_error = si_clear_backplane_to((si_t*)osh->sih);
		} else {
			bpt_error = si_clear_backplane_to_fast((void*)osh->sih, (void*)r);
		}

		/* Poll and clear any back plane timeouts */
		if (bpt_error) {
			in_si_clear = FALSE;
#if defined(BCMDBG) && defined(STATS_REG_RET_FFFFFFFF)
			WL_ERROR(("Caller : %s:%d\n", f, l));
#endif /* BCMDBG && STATS_REG_RET_FFFFFFFF */
			if (osh->wl_fatal_error_cb) {
				osh->wl_fatal_error_cb((struct wl_info *)osh->wl,
					WL_REINIT_RC_AXI_BUS_ERROR);
			}
		}
#ifdef STATS_REG_RET_FFFFFFFF
		else {
#ifdef BCMDBG

			WL_INFORM(("ALLFFREG: %p, size:%d  returned -1, caller:%s:%d\n", OSL_OBFUSCATE_BUF(r), size, f, l));
#endif
			if (size == 4) {
				validRegRetFF.count_32++;
			} else if (size == 2) {
				validRegRetFF.count_16++;
			} else if (size == 1) {
				validRegRetFF.count_8++;
			} else {
				ASSERT(!"Invalid size\n");
			}
		}
#endif
		in_si_clear = FALSE;
	}

	return status;
}
#endif /* #if defined(BCM_BACKPLANE_TIMEOUT) */

uint32
osl_pci_read_config(osl_t *osh, uint offset, uint size)
{
	if (size == 4)
		return osh->pcidev->configRead32(offset);
	else if (size == 2)
		return osh->pcidev->configRead16(offset);
	else
		ASSERT(size == 2 || size == 4);

	return 0xFFFFFFFF;
}

void
osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val)
{
	if (size == 4)
		osh->pcidev->configWrite32((UInt8)offset, (UInt32)val);
	else if (size == 2)
		osh->pcidev->configWrite16((UInt8)offset, (UInt16)val);
	else
		ASSERT(size == 2 || size == 4);
}

uint32
osl_pci_read_config_parent(osl_t * osh, IOPCIBridge *parent, uint offset, uint size)
{
	uint32 val = ID32_INVALID;

	BCM_REFERENCE(osh);

	osl_pci_access_record(osh, PCI_ACCESS_TYPE_PCI_CFG_READ_PARENT, (void*)(uintptr)offset, 0, size, 0);

	if (size == 4)
		val = parent->configRead32(parent->getBridgeSpace(), offset);
	else if (size == 2)
		val = parent->configRead16(parent->getBridgeSpace(), offset);
	else
		ASSERT(size == 2 || size == 4);

	osl_pci_access_record(osh, PCI_ACCESS_TYPE_PCI_CFG_READ_PARENT_COMP, (void*)(uintptr)offset, val, size, 0);

	return val;
}

void
osl_pci_write_config_parent(osl_t * osh, IOPCIBridge *parent, uint offset, uint size, uint val)
{
	BCM_REFERENCE(osh);

	osl_pci_access_record(osh, PCI_ACCESS_TYPE_PCI_CFG_WRITE_PARENT, (void*)(uintptr)offset, val, size, 0);

	if (size == 4)
		parent->configWrite32(parent->getBridgeSpace(), (UInt8)offset, (UInt32)val);
	else if (size == 2)
		parent->configWrite16(parent->getBridgeSpace(), (UInt8)offset, (UInt16)val);
	else
		ASSERT(size == 2 || size == 4);

	osl_pci_access_record(osh, PCI_ACCESS_TYPE_PCI_CFG_WRITE_PARENT_COMP, (void*)(uintptr)offset, val, size, 0);
}

#ifdef BCMDMASGLISTOSL
static uint32
osl_dma_gen_physical_segments(osl_t *osh, mbuf_t m, UInt32 nvec)
{
	osh->nsegs = osh->mbuf_cursor->getPhysicalSegmentsWithCoalesce( m, osh->seg_list, nvec);
	return osh->nsegs;
}

static addr64_t
osl_dma_get_physical_segment_addr(osl_t *osh, mbuf_t m, uint32 i)
{

	BCM_REFERENCE(m);

	if( i < MAX_DMA_SEGS && i < osh->nsegs )
		return osh->seg_list[i].location;

	return 0;
}

static uint32
osl_dma_get_physical_segment_length(osl_t *osh, mbuf_t m, uint32 i)
{
	BCM_REFERENCE(m);

	if( i < MAX_DMA_SEGS && i < osh->nsegs )
		return osh->seg_list[i].length;

	return 0;
}

#endif /* BCMDMASGLISTOSL */

dmaaddr_t
osl_dma_map(osl_t *osh, void *va, int size, uint direction, mbuf_t m, hnddma_seg_map_t *map)
{
#ifdef BCMDMASGLISTOSL
	dmaaddr_t pa = {0,0};
	dmaaddr_t *ppa = NULL;
	addr64_t phys = m ? mbuf_data_to_physical(mbuf_data(m)) : 0;
	uint32 nsegs = 0;
	uint i = 0;
	UInt32 nvec;
	uint32 len;
#else
	dmaaddr_t pa = {0,0};
	addr64_t phys = NULL;
#endif

	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));
	ASSERT(m != NULL);
	ASSERT(mbuf_data(m) == va);
	ASSERT((int)mbuf_len(m) == size);
	/* make sure the start and end of the byte range are in the same page */
	ASSERT(((uint)((uintptr)va) & ~PAGE_MASK) == (((uint)((uintptr)va) + (uint)(size - 1)) & ~PAGE_MASK));

#ifndef BCMDMASGLISTOSL
	phys = mbuf_data_to_physical(mbuf_data(m));
	PHYSADDRLOSET(pa, (uint32)phys);
	ASSERT(((phys >> 32) == 0) || (osh->dma_addrwidth == DMADDRWIDTH_64));
	PHYSADDRHISET(pa, (uint32)(phys >> 32));

	return pa;
#else
	if (map)
		nvec = sizeof(map->segs)/sizeof(hnddma_seg_t);
	else
		nvec = 1;

	nsegs = osl_dma_gen_physical_segments(osh, m, nvec);

	ASSERT (nsegs <= nvec);
	ASSERT(nsegs > 0 && nsegs <= MAX_DMA_SEGS);
	ASSERT((direction == DMA_TX) || (nsegs == 1));

	/* Caller may not pass a map expecting to get a single segment back */
	if (map) {
		bzero(map, sizeof(hnddma_seg_map_t));
		map->origsize = size;
		map->nsegs = nsegs;
	}

	// Transfer the OS mapping to HNDDMA mapping taking care of
	// 64-bit DMA support
	for (i = 0; i <nsegs; i++) {
		phys = osl_dma_get_physical_segment_addr(osh, m, i);
		ASSERT(phys);
		len = osl_dma_get_physical_segment_length(osh, m, i);
		if (map) {
			map->segs[i].length = len;
			ppa = &map->segs[i].addr;
		} else
			ppa = &pa;

		PHYSADDRLOSET(*ppa, (uint32)(phys & 0xffffffff));
		ASSERT(((phys >> 32) == 0) || (osh->dma_addrwidth == DMADDRWIDTH_64));
		PHYSADDRHISET(*ppa, (uint32)(phys >> 32));

		DBGMEM_INFO_RECORDTRACE( DBGMEM_INFO_TYPE_DMAALLOC, (0x0d3a3a1042420000ULL | i) /* tag */,
			"osl_dma_map - Success, nsegs > 0" /* tagstr */,
			__FILE__ /* file */, __LINE__ /* line */,
			OSL_SYSUPTIME(),
			(size_t)mbuf_len(m), (void*)m, (void*)mbuf_data(m),
			(void*)mbuf_data_to_physical( mbuf_data(m) ) /* physa */,
			(void*)phys /* arg0 */, (void*)((uint64_t)(map->segs[i].length)) /* arg1 */,
			(void*)va /* arg2 */, (void*)NULL /* arg3 */ );
	}

	if (map)
		ppa = &map->segs[0].addr;

	if( nsegs == 0 )
	{
		// mbuf can't be mapped, mbuf could have been potentially freed ??
		// Worse case is, the kernel free'd it (assume that) is the case and we just need to ignore this mbuf and not
		// free it and assume kernel freed it, should be a low occurance

		DBGMEM_INFO_RECORDTRACE( DBGMEM_INFO_TYPE_DMAALLOC, (0x0d3a3a104242deadULL) /* tag */,
			"osl_dma_map - Failed, nsegs == 0" /* tagstr */,
			__FILE__ /* file */, __LINE__ /* line */,
			OSL_SYSUPTIME(),
			(size_t)0, (void*)m, (void*)NULL,
			(void*)phys /* physa */,
			(void*)phys /* arg0 */, (void*)NULL /* arg1 */,
			(void*)va /* arg2 */, (void*)NULL /* arg3 */ );

		printf( "'%s:%d': ERROR: Failed to map buffer !!!\n", __FUNCTION__, __LINE__ );

	}

	return *ppa;
#endif /* BCMDMASGLISTOSL */
}

void
osl_dma_addrwidth(osl_t *osh, uint width)
{
	osh->dma_addrwidth = width;
}

// Allocate a private mbuf pool of OSL_POOL_SIZE mbufs with PCI addresses below 1 GB if needed
errno_t
osl_alloc_private_mbufs(osl_t *osh)
{
	mbuf_t m = NULL;
	mbuf_t bad_mbufs = NULL;
	void *pdata = NULL;
	int i = 0;
	int bad_count = 0;
	errno_t err = BCME_OK;

	BCM_REFERENCE(pdata);
	// exit with no error if we do not need a private mbuf pool
	if (osh->dma_addrwidth != DMADDRWIDTH_30 || osl_ram_size() < (UInt64)OSL_1_GIG)
		return 0;

	osh->use_private_mbufs = TRUE;
#ifdef BCMDBG
	printf("wl: Using mbuf Private Pool\n");
#endif

	// Allocate OSL_POOL_SIZE mbufs with addresses below 1 GB. Since driver startup normally
	// happens at boot time, mbuf address should be low. Allow for OSL_BAD_MBUF_LIMIT
	// allocations above 1 GB while attempting to create the private pool.
	while ((i < OSL_POOL_SIZE) && (bad_count < OSL_BAD_MBUF_LIMIT)) {
		err = mbuf_getpacket(MBUF_WAITOK, &m);
		if (err)
			break;
		// if the mapped address is < 1G, add the PKTTAG for osl use and mark private
		if (!osl_mchain_high_phys_addr(osh, m)) {
			err = osl_add_pkttag(osh, m);
			if (!err)
				err = mbuf_tag_allocate(m, g_osl_mbuf_tag_id,
					OSL_MBUF_TAG_TYPE_PRIV, sizeof(int), MBUF_WAITOK, &pdata);
			if (err) {
				osl_mbuf_enqueue(&bad_mbufs, m);
				break;
			}
			osl_mbuf_enqueue(&osh->private_pool, m);
			i++;
		} else {
			// otherwise, toss the mbuf on the bad list
			osl_mbuf_enqueue(&bad_mbufs, m);
			bad_count++;
		}
	}

	if (bad_mbufs)
		mbuf_freem_list(bad_mbufs);

	if (i < OSL_POOL_SIZE && !err)
		err = ENOMEM;

	if (err)
		osl_free_private_mbufs(osh);

	return err;
}

static void
osl_free_private_mbufs(osl_t *osh)
{
	if (!osh->private_pool)
		return;

	mbuf_freem_list(osh->private_pool);
	osh->private_pool = NULL;
}

static bool
osl_is_private_mbuf(mbuf_t m)
{
	size_t length = 0;
	errno_t err = BCME_OK;
	void *pdata = NULL;

	BCM_REFERENCE(length);
	BCM_REFERENCE(pdata);
	ASSERT(mbuf_flags(m) & MBUF_PKTHDR);

	err = mbuf_tag_find(m, g_osl_mbuf_tag_id, OSL_MBUF_TAG_TYPE_PRIV, &length, &pdata);

	return !err;
}

static void
osl_mbuf_enqueue(mbuf_t *list, mbuf_t m)
{
	ASSERT(list);
	ASSERT(mbuf_nextpkt(m) == NULL);

	if (*list)
		mbuf_setnextpkt(m, *list);

	*list = m;
}

static mbuf_t
osl_mbuf_dequeue(mbuf_t *list)
{
	mbuf_t m = NULL;

	ASSERT(list);

	m = *list;
	if (m) {
		*list = mbuf_nextpkt(m);
		mbuf_setnextpkt(m, NULL);
		ASSERT(osl_pkt_gettag(m) != NULL);
	}

	return m;
}

static UInt64
osl_ram_size()
{
	UInt64 physRamBytes = 0;
	size_t len = sizeof(physRamBytes);

	if (sysctlbyname("hw.memsize", &physRamBytes, &len, NULL, 0)) {
		DPRINT("%s: sysctlbyname(hw.memsize) failed\n", __FUNCTION__);
		// return worst case large ram size
		return UQUAD_MAX;
	}

	return physRamBytes;
}

static bool
osl_mchain_high_phys_addr(osl_t *osh, mbuf_t m)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));
	ASSERT(m);

#ifndef BCMDMASGLISTOSL
	while (m) {
		if ((uint32)mbuf_data_to_physical(mbuf_data(m)) >= OSL_1_GIG)
			return true;
		m = mbuf_next(m);
	}
#else
	uint32 nsegs = osl_dma_gen_physical_segments(osh, m, MAX_DMA_SEGS);
	if( !nsegs )
		return true;	// something ain't right...

	addr64_t loc = 0;

	do {
		loc = osl_dma_get_physical_segment_addr(osh, m, nsegs - 1);

		if(!loc || loc >= OSL_1_GIG)
			return true;
	}while( --nsegs );
#endif /* BCMDMASGLISTOSL */

	return false;
}

char *
strrchr(const char *s, int c)
{
	const char *p = NULL;

	do {
		if (*s == (char)c)
			p = s;
	} while (*s++ != '\0');

	return (char*)p;
}

#define OSL_PANIC_BUF_SIZE		2048
char osl_panic_buf[OSL_PANIC_BUF_SIZE];

void osl_panic(const char *fmt, ...)
{
	va_list args = {};
	bzero(osl_panic_buf, sizeof(osl_panic_buf));

	va_start(args, fmt);
	vsnprintf(osl_panic_buf, OSL_PANIC_BUF_SIZE, fmt, args);
	va_end(args);

	panic( "%s", osl_panic_buf );
}


#ifdef BCMDBG_ASSERT

void
osl_assert(const char* file, int line, const char* exp)
{
#ifdef ENABLE_CORECAPTURE
	char reason_string[CC_REASON_STR_LEN];
	char unknown[] = "UNKNOWN";
#endif /* ENABLE_CORECAPTURE */
	char *basename = NULL;

	basename = strrchr(file, '/');
	/* skip the '/' */
	if (basename)
		basename++;

	if (!basename)
		OSL_LOG((char*)"assert", (char*)"Assertion failed: file \"%s\", line %d: %s\n", file, line, exp );
	else
		OSL_LOG((char*)"assert", (char*)"Assertion failed: file \"%s\", line %d: %s\n", basename, line, exp );

#ifdef BCMDBG
	IOLog("Assertion failed: file \"%s\", line %d: %s\n",
	       file, line, exp);
#endif

	WIFICC_LOGDEBUG(("Assertion failed: file \"%s\", line %d: %s\n", file, line, exp));


#ifdef ENABLE_CORECAPTURE
	if (!basename) {
		basename = &unknown[0];
	}
	bzero(reason_string, sizeof(reason_string));
	snprintf(reason_string, sizeof(reason_string), "ASSERT:%s:%d[%s]", basename, line, exp);
	reason_string[sizeof(reason_string)-1] = 0;

	wl_log_system_state_wrapper(reason_string);

#endif /* ENABLE_CORECAPTURE */

	if (g_assert_type == 0) {
		/* Let give some time for the CoreCapture and logs to be flushed to disk */
		IOSleep(ASSERT_DELAY * 1000);

		osl_panic("Assertion failed: file \"%s\", line %d: %s\n",
		      file, line, exp);
	}
}
#endif /* BCMDBG_ASSERT */

uint32
osl_sysuptime()
{
	uint64_t nano, result;
	nano = mach_absolute_time();
	absolutetime_to_nanoseconds(nano, &result);
	return (uint32) (result / 1000000);
}

uint32
osl_sysuptime_us()
{
	uint64_t nano, result;
	nano = mach_absolute_time();
	absolutetime_to_nanoseconds(nano, &result);
	return (uint32) (result / 1000);
}

uint64
osl_sysuptime_us64()
{
	uint64_t nano, result;
	nano = mach_absolute_time();
	absolutetime_to_nanoseconds(nano, &result);
	return (result / 1000);
}

uint64
osl_calendartime_us64()
{
#if VERSION_MAJOR > 9
	clock_sec_t sec;
	clock_usec_t us;
#else
	uint32 sec, us;
#endif
	uint64 abs_us;

	clock_get_calendar_microtime(&sec, &us);

	abs_us = (sec * 1000000ULL) + us;

	return abs_us;
}

#define OSL_LOG_BUF_SIZE 800
#define DOMAIN_SIZE	100
char osl_log_buf[OSL_LOG_BUF_SIZE] = { 0 };
char domain[DOMAIN_SIZE] = { 0 };

void
osl_log(const char * tag, const char *fmt, ...)
{
	va_list args = {};
	bzero(osl_log_buf, OSL_LOG_BUF_SIZE);

	BCM_REFERENCE(tag);

	va_start(args, fmt);
	vsnprintf(osl_log_buf, OSL_LOG_BUF_SIZE, fmt, args);
	va_end(args);

#ifdef MESSAGE_TRACER

#if defined(MAC_OS_X_VERSION_10_9)
    strncpy(domain, "wifi.driver.broadcom.assertions", DOMAIN_SIZE);

    UInt32 kvCount = 1;
    const char *keys[] = {"com.apple.message.osl_log"};
    const char *values[] = {osl_log_buf};

    IO8LogMT(domain, keys, values, kvCount);
#else
		strncpy(domain, "airport.driver.assertions.broadcom", DOMAIN_SIZE);

	IO8LogMT((APPLE80211_MT_FLAG_DOMAIN | APPLE80211_MT_FLAG_SIG),
	         (char*)domain,
	         NULL,
	         osl_log_buf,
	         NULL, NULL, 0, 0, 0, 0, NULL, NULL, NULL);
#endif

#endif
}



// OS packet primitives
void* osl_pktdata(osl_t *osh, mbuf_t m)
{
    BCM_REFERENCE(osh);

    MBUF_PANICPARANOIDCHECK( m, __FUNCTION__, __LINE__ );
    return( mbuf_data( m ) );
}

size_t osl_pktlen(osl_t *osh, mbuf_t m)
{
    BCM_REFERENCE(osh);

    MBUF_PANICPARANOIDCHECK( m, __FUNCTION__, __LINE__ );
    return( mbuf_len( m ) );
}

size_t osl_pktsetlen(osl_t *osh, mbuf_t m, uint len )
{
    BCM_REFERENCE(osh);

    MBUF_PANICPARANOIDCHECK( m, __FUNCTION__, __LINE__ );
    mbuf_setlen( m, len );
    return( osl_pktlen( osh, m ) );
}

size_t osl_pktheadroom(osl_t *osh, mbuf_t m)
{
    BCM_REFERENCE(osh);

    MBUF_PANICPARANOIDCHECK( m, __FUNCTION__, __LINE__ );
    return( mbuf_leadingspace( m ) );
}

size_t osl_pkttailroom(osl_t *osh, mbuf_t m)
{
    BCM_REFERENCE(osh);

    MBUF_PANICPARANOIDCHECK( m, __FUNCTION__, __LINE__ );
    return( mbuf_trailingspace( m ) );
}

mbuf_t osl_pktnext(osl_t *osh, mbuf_t m)
{
    BCM_REFERENCE(osh);

    MBUF_PANICPARANOIDCHECK( m, __FUNCTION__, __LINE__ );
    return( mbuf_next( m ) );
}

errno_t osl_pktsetnext(osl_t *osh, mbuf_t m, mbuf_t n )
{
    BCM_REFERENCE(osh);

    MBUF_PANICPARANOIDCHECK( m, __FUNCTION__, __LINE__ );
    return( mbuf_setnext( m, n ) );
}

mbuf_t osl_pktlink(mbuf_t m)
{
    MBUF_PANICPARANOIDCHECK( m, __FUNCTION__, __LINE__ );
    return( mbuf_nextpkt( m ) );
}

errno_t osl_pktsetlink(mbuf_t m, mbuf_t n )
{
    if( m ) MBUF_PANICPARANOIDCHECK( m, __FUNCTION__, __LINE__ );
    if( n ) MBUF_PANICPARANOIDCHECK( n, __FUNCTION__, __LINE__ );
    mbuf_setnextpkt( m, n );
    return( 0 );
}

int osl_pktshared(osl_t *osh, mbuf_t m)
{
    BCM_REFERENCE(osh);

    MBUF_PANICPARANOIDCHECK( m, __FUNCTION__, __LINE__ );
    return( mbuf_mclhasreference( m ) );
}

uint32 osl_rand(void)
{
	return 1;
}


#define BUFLEN 1024

static int (*osl_printffunc)(const char *fmt, ...) =
  PRINT_FIREWIRE ? osl_fwlog_kprintf : osl_iolog_withtimestamp;

int osl_printf(const char *fmt, ...)
{
	static char buf[BUFLEN] = {};
	int len = 0;
	va_list ap = {};

	va_start(ap, fmt);
	vsnprintf(buf + len, sizeof(buf) - len, fmt, ap);
	va_end(ap);

	if ((brcmfirewirelog & 0x03) && (brcmfirewirelog & 0x1))
		osl_fwlog_kprintf("%s", buf);

	if (((brcmfirewirelog & 0x03) == 0) ||
	    ((brcmfirewirelog & 0x03) && (brcmfirewirelog & 0x2)))
	{
		osl_iolog_withtimestamp("%s", buf);
	}

	return len;
}

/*
 * Prepend messages with a timestamp
 */
int
osl_fwlog_kprintf(const char *fmt, ...)
{
	static char buf[BUFLEN] = {};
	int len = 0;
	va_list ap = {};

	if (ml_at_interrupt_context())
		return 0;

	va_start(ap, fmt);
	vsnprintf(buf + len, sizeof(buf) - len, fmt, ap);
	va_end(ap);

	kprintf("ARPT: %s", buf);

	return 0;
}

/*
 * Prepend messages with a timestamp
 */
int
osl_iolog_withtimestamp(const char *fmt, ...)
{
	static char buf[BUFLEN] = {};
	static int had_newline = true;  /* init to true, allows first message to have timestamp */
	char *bufptr = NULL;
	int len = 0;
	clock_sec_t sec = 0;
	clock_usec_t usec = 0;
	va_list ap = {};

	if (ml_at_interrupt_context())
		return (0);

	if (had_newline) {
		clock_get_system_microtime(&sec, &usec);
		len = snprintf(buf, sizeof(buf), "ARPT: %d.%06d: ", (uint32_t)sec, (uint32_t)usec);
	}

	bufptr = (char *)(buf + len);
	va_start(ap, fmt);
	len = vsnprintf(buf + len, sizeof(buf) - len, fmt, ap);
	va_end(ap);

	if ((char)*(bufptr + len - 1) == '\n')
		had_newline = true;
	else
		had_newline = false;

	IOLog("%s", buf);

	return 0;
}

static int
sysctl_brcmfirewirelog SYSCTL_HANDLER_ARGS
{
	int error = BCME_OK, retval = 0;

	BCM_REFERENCE(arg1);
	BCM_REFERENCE(arg2);

	error = sysctl_handle_int(oidp, oidp->oid_arg1, oidp->oid_arg2, req);

	if (!error && req->newptr) {
		/* write new value */
		osl_printffunc = brcmfirewirelog ? osl_fwlog_kprintf : osl_iolog_withtimestamp;
	}
	else if (req->newptr) {
		/* error. handle it? */
	}
	else {
		/* read */
		retval = brcmfirewirelog;
		error = SYSCTL_OUT(req, &retval, sizeof(retval));
	}
	return error;
}

/* "debug.brcmfirewirelog" */
SYSCTL_PROC(_debug, OID_AUTO, brcmfirewirelog, CTLTYPE_INT | CTLFLAG_RW,
	&brcmfirewirelog, 0, &sysctl_brcmfirewirelog, "I", "Broadcom WiFi firewire logging");

static void
sysctlattach(void *p)
{
#ifndef BCMDONGLEHOST
	sysctl_register_oid(&sysctl__debug_brcmfirewirelog);

	BCM_REFERENCE(p);

	// Allow over-ride brcmfirewirelog
	int32_t value = 0;
	if( PE_parse_boot_argn( "brcmfirewirelog", &value, sizeof(value) ) )
	{
		printf("Over-riding brcmfirewirelog, found \"brcmfirewirelog\" arg, value[0x%08x]\n", value );
		brcmfirewirelog = value;
	}
#endif /* !BCMDONGLEHOST */
}

static void
sysctldetach(void *p)
{
	BCM_REFERENCE(p);
#ifndef BCMDONGLEHOST
	sysctl_unregister_oid(&sysctl__debug_brcmfirewirelog);
#endif /* !BCMDONGLEHOST */
}

#ifdef BCMDONGLEHOST
void*
osl_spin_alloc_init()
{
	osl_spin_lock_t *lock = (osl_spin_lock_t*)IOMalloc(sizeof(*lock));
	if (!lock) {
		IOLog("Failed to allocate memory for spin lock\n");
		return NULL;
	}

	memset(lock, 0, sizeof(*lock));
	lock->lock_grp = lck_grp_alloc_init("DHDspinlock", LCK_GRP_ATTR_NULL);
	if (!lock->lock_grp) {
		IOLog("lck_grp_alloc_init failed\n");
		goto fail;
	}

	lock->lock = lck_spin_alloc_init(lock->lock_grp, LCK_ATTR_NULL);
	if (!lock->lock) {
		IOLog("lck_spin_alloc_init failed\n");
		goto fail;
	}

	return lock;

fail:
	if (lock->lock_grp)
		lck_grp_free(lock->lock_grp);
	IOFree(lock, sizeof(*lock));
	return NULL;
}

void
osl_spin_free(void *l)
{
	osl_spin_lock_t *lock = (osl_spin_lock_t*)l;
	if (!lock || !lock->lock || !lock->lock_grp)
		return;

	lck_spin_free(lock->lock, lock->lock_grp);
	lck_grp_free(lock->lock_grp);
	IOFree(lock, sizeof(*lock));
}

void
osl_spin_lock(void *l)
{
	osl_spin_lock_t *lock = (osl_spin_lock_t*)l;
	ASSERT(lock);
	lck_spin_lock(lock->lock);
}

void
osl_spin_unlock(void *l)
{
	osl_spin_lock_t *lock = (osl_spin_lock_t*)l;
	ASSERT(lock);
	lck_spin_unlock(lock->lock);
}
#endif /* BCMDONGLEHOST */

#ifdef PCI_ACCESS_HISTORY

int osl_pci_access_history_init(osl_t * osh)
{
	uint32 value;

	if (!osh) {
		return BCME_NOTREADY;
	}

	osh->pci_access_idx = 0;

        if(PE_parse_boot_argn("bcom.pci-history-level", &value, sizeof(value)))
        {
                WL_PRINT(("bcom.pci-history-level= 0x%x\n", value ));

                osh->pci_access_log_flags = value;
        } else {
                osh->pci_access_log_flags =
			(1 << PCI_ACCESS_TYPE_MMIO_READ) |
			(1 << PCI_ACCESS_TYPE_MMIO_READ_COMP) |
			(1 << PCI_ACCESS_TYPE_MMIO_WRITE) |
			(1 << PCI_ACCESS_TYPE_MMIO_WRITE_COMP) |
			(1 << PCI_ACCESS_TYPE_PCI_CFG_READ) |
			(1 << PCI_ACCESS_TYPE_PCI_CFG_READ_COMP) |
			(1 << PCI_ACCESS_TYPE_PCI_CFG_WRITE) |
			(1 << PCI_ACCESS_TYPE_PCI_CFG_WRITE_COMP) |
			(1 << PCI_ACCESS_TYPE_PCI_CFG_READ_PARENT) |
			(1 << PCI_ACCESS_TYPE_PCI_CFG_READ_PARENT_COMP) |
			(1 << PCI_ACCESS_TYPE_PCI_CFG_WRITE_PARENT) |
			(1 << PCI_ACCESS_TYPE_PCI_CFG_WRITE_PARENT_COMP);
        }

        if(PE_parse_boot_argn("bcom.pci-history-entries", &value, sizeof(value)))
        {
                // Boot arg to control forced panic on backplane timeout
                WL_PRINT(("bcom.pci-history-entries = %d\n", value ));

                if (value < PCI_ACCESS_HISTORY_MIN_LEN) {
                        value = PCI_ACCESS_HISTORY_MIN_LEN;
                } else if (value > PCI_ACCESS_HISTORY_MAX_LEN) {
                        value = PCI_ACCESS_HISTORY_MAX_LEN;
                }

                osh->pci_access_history_entries = value;
        } else {
                osh->pci_access_history_entries = PCI_ACCESS_HISTORY_MIN_LEN;
        }

	osh->pci_access_history = (pci_access_t *)MALLOCZ(osh,
		(sizeof(pci_access_t) * osh->pci_access_history_entries));

	return (osh->pci_access_history ? BCME_OK : BCME_NOMEM);
}

int osl_pci_access_history_deinit(osl_t * osh)
{

	if (osh->pci_access_history) {
		MFREE(osh, osh->pci_access_history,
			(sizeof(pci_access_t) * osh->pci_access_history_entries));

		osh->pci_access_history = NULL;
	}

	return BCME_OK;
}

void osl_pci_access_record(void * iosh, uint32 access_type, volatile void * virt_addr,
	uint64 data, uint32 acc_len, uint32 area_type)
{
	uint32 offset = 0;
	osl_t * osh = (osl_t*)iosh;
	pci_access_t * ptr;

	BCM_REFERENCE(area_type);

	if ((!osh) || (!osh->pci_access_history)) {
		WL_ERROR(("%s Not initialised. Skip MMIO access history logging\n", __FUNCTION__));
		return;
	}

	/* Log only if corresponding log flags is enabled */
	if (!(osh->pci_access_log_flags & (1<<access_type))) {
		return;
	}

	ptr = &osh->pci_access_history[osh->pci_access_idx];

	osh->pci_access_idx++;
	if (osh->pci_access_idx == osh->pci_access_history_entries) {
		osh->pci_access_idx = 0;
	}

	clock_get_system_microtime(&ptr->sec, &ptr->usec);
        ptr->access_type = access_type;
        ptr->virt_addr = (void*)virt_addr;
        ptr->data = data;
        ptr->size = acc_len;
        ptr->area_type = 0;
	ptr->thread = (void*)IOThreadSelf();

	if (access_type ==  PCI_ACCESS_TYPE_PCI_CFG_WRITE_COMP) {
		switch ((uintptr)virt_addr) {
			case PCI_BAR0_WIN:
				/* 0 - 0xFFF */
				osh->bar0 = (uint32)data;
				break;
			case PCIE2_BAR0_WIN2:
			case PCI_BAR0_WIN2:
				/* 0x1000 - 0x2000 */
				osh->bar0_win2 = (uint32)data;
				break;
			case PCIE2_BAR0_CORE2_WIN:
				/* 0x4000 - 0x5000 */
				osh->sec_bar0_win = (uint32)data;
				break;
			case PCIE2_BAR0_CORE2_WIN2:
				/* 0x5000 - 0x6000 */
				osh->sec_bar0_win_wrap = (uint32)data;
				break;
		}
	}

	if ( osh && access_type <= PCI_ACCESS_TYPE_MMIO_WRITE_COMP) {
		offset = (uint32)((uint8*)virt_addr - (uint8*)si_coreregs((si_t *)osh->sih));

		if (offset < PCI_BAR0_WIN2_OFFSET) {
			ptr->addr = osh->bar0 + offset;
		} else if (offset < PCI_CORE_ENUM_OFFSET) {
			ptr->addr = osh->bar0_win2 + offset - PCI_BAR0_WIN2_OFFSET;
		} else if (offset < PCI_CC_CORE_ENUM_OFFSET) {
			ptr->addr = 0x10000000 + offset - PCI_CORE_ENUM_OFFSET;
		} else if (offset < PCI_SEC_BAR0_WIN_OFFSET) {
			ptr->addr = 0x18000000 + offset - PCI_CC_CORE_ENUM_OFFSET;
		} else if (offset < PCI_SEC_BAR0_WRAP_OFFSET) {
			ptr->addr = osh->sec_bar0_win + offset - PCI_SEC_BAR0_WIN_OFFSET;
		} else if (offset < PCI_CORE_ENUM2_OFFSET) {
			ptr->addr = osh->sec_bar0_win_wrap + offset - PCI_SEC_BAR0_WRAP_OFFSET;
		} else if (offset < PCI_CC_CORE_ENUM2_OFFSET) {
			ptr->addr = 0x20000000 + offset - PCI_CORE_ENUM2_OFFSET;
		} else if (offset < PCI_LAST_OFFSET) {
			ptr->addr = 0x18000000 + offset - PCI_CC_CORE_ENUM2_OFFSET;
		} else {
			// Address is beyond bar0 and bar0_win2 offset
			ptr->addr = offset;
		}
	}
	else {
		ptr->addr = 0;
	}

#ifdef PCI_ACCESS_HISTORY_DEBUG_REG_ACCESS
	/* If MMIO access RD/WR is not followed by same RD/WR completion log the error */
	if ((osh->prev_access_type & 1) &&
		(osh->pci_access_log_flags & (1<<(osh->prev_access_type+1))) &&
		((ptr->access_type != (osh->prev_access_type + 1)) ||
		 (ptr->addr != osh->prev_access_addr))) {

		/* Current register access */
		wl_print_backtrace("MMIO-ASYNC-CURR", NULL, 0);

		/* Previous access for which we are expecting completion */
		wl_print_backtrace("MMIO-ASYNC-PREV", (void*)&osh->prev_mmio_backtrace,
			osh->prev_mmio_backtrace_depth);

		ASSERT(!"Invalid MMIO ACCESS");
	}

	osh->prev_access_type = ptr->access_type;
	osh->prev_access_addr = ptr->addr;
	osh->prev_mmio_backtrace_depth = OSBacktrace(&osh->prev_mmio_backtrace[0], MAX_BACKTRACE_DEPTH);
#endif /* PCI_ACCESS_HISTORY_DEBUG_REG_ACCESS */
}


void osl_pci_access_display(void * iosh, uint32 access_type)
{
	osl_t * osh = (osl_t*)iosh;
	uint32 len = 0, idx;
	pci_access_t * ptr = NULL;
	static const char *type[] = {"ALL", "RD", "RDC", "WR", "WRC", "PCIRD", "PCIRDC", "PCIWR", "PCIWRC", "PCIRD_P", "PCIRD_PC", "PCIWR_P", "PCIWR_PC"};

	BCM_REFERENCE(access_type);
	BCM_REFERENCE(type);

	if ((!osh) || (!osh->pci_access_history)) {
		WL_ERROR(("%s Not initialised. Skip MMIO access history display\n", __FUNCTION__));
		return;
	}

	idx = osh->pci_access_idx;

	WIFICC_LOGDEBUG(("PCI ACCESS HISTORY...\n"));
	WIFICC_LOGDEBUG(("IDX\t\tTIME\t TYPE\t VADDR\t\t\t PADDR\t\tDATA\t\t SIZE\t THREAD\n"));

	/* Print in reverse order */
	for (len=0; len < osh->pci_access_history_entries; len++) {
		idx++;
		if (idx == osh->pci_access_history_entries) {
			idx = 0;
		}

		ptr = &osh->pci_access_history[idx];

		WIFICC_LOGDEBUG(("%4d\t %6u.%6u\t %s\t %16p\t %8x\t %-8x\t %2d\t %p\n", len,
			ptr->sec, ptr->usec,
			type[ptr->access_type], OSL_OBFUSCATE_BUF(ptr->virt_addr), ptr->addr,
			ptr->data, ptr->size,
			(void*)OSL_OBFUSCATE_BUF(ptr->thread)));
	}
}
#endif /* PCI_ACCESS_HISTORY */

#include <sys/random.h>

vm_offset_t
osl_buf_kernel_addrperm_addr(void * addr)
{

	static vm_offset_t kernel_addrperm = 0;

	/* Init */
	if (kernel_addrperm == 0) {
		read_random(&kernel_addrperm, sizeof(kernel_addrperm));
		kernel_addrperm |= 1;
	}
	if( (vm_offset_t)addr == 0)
		return 0;

	return ((vm_offset_t)addr + kernel_addrperm);
}

} /* extern "C" */
