/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
#ifdef TESTPROG
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#endif

#include "lib_types.h"
#include "lib_malloc.h"

/*  *********************************************************************
    *  Constants
    ********************************************************************* */

#define MEMNODE_SEAL 0xFAAFA123		/* just some random constant */
#define MINBLKSIZE 64

/*  *********************************************************************
    *  Types
    ********************************************************************* */

typedef enum { memnode_free = 0, memnode_alloc } memnode_status_t;

typedef struct memnode_s {
    unsigned int seal;
    struct memnode_s *next;		/* pointer to next node */
    unsigned int length;		/* length of the entire data section */
    memnode_status_t status;		/* alloc/free status */
    unsigned char *data;		/* points to actual user data */
    void *memnodeptr;			/* memnode back pointer (see comments) */
} memnode_t;

struct mempool_s {
    memnode_t *root;			/* pointer to root node */
    unsigned char *base;		/* base of memory region */
    unsigned int length;		/* size of memory region */
};

#define memnode_data(t,m) (t) (((memnode_t *) (m))+1)

/*  *********************************************************************
    *  Globals
    ********************************************************************* */

mempool_t kmempool;			/* default pool */

/*  *********************************************************************
    *  kmeminit(pool,buffer,length)
    *
    *  Initialize the memory manager, given a pointer to an area
    *  of memory and a size.  This routine simply initializes the
    *  root node to be a single block of empty space.
    *
    *  Input parameters:
    *      pool - pool pointer
    *  	   buffer - beginning of buffer area, must be pointer-aligned
    *  	   length - length of buffer area
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */


void kmeminit(mempool_t *pool,unsigned char *buffer,int length)
{
    pool->root = (memnode_t *) buffer;
    pool->root->seal = MEMNODE_SEAL;
    pool->root->length = length - sizeof(memnode_t);
    pool->root->data = memnode_data(unsigned char *,pool->root);
    pool->root->status = memnode_free;
    pool->root->next = NULL;

    pool->base = buffer;
    pool->length = length;
}


/*  *********************************************************************
    *  kmempoolbase(pool)
    *
    *  Returns the base address of the specified memory pool
    *
    *  Input parameters:
    *  	   pool - pool pointer
    *
    *  Return value:
    *  	   pointer to beginning of pool's memory
    ********************************************************************* */
void *kmempoolbase(mempool_t *pool)
{
    return pool->base;
}

/*  *********************************************************************
    *  kmempoolsize(pool)
    *
    *  Returns the total size of the specified memory pool
    *
    *  Input parameters:
    *  	   pool - pool pointer
    *
    *  Return value:
    *  	   size of pool in bytes
    ********************************************************************* */

int kmempoolsize(mempool_t *pool)
{
    return pool->length;
}

/*  *********************************************************************
    *  kmemcompact(pool)
    *
    *  Compact the memory blocks, coalescing consectutive free blocks
    *  on the list.
    *
    *  Input parameters:
    *  	   pool - pool descriptor
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */

static void kmemcompact(mempool_t *pool)
{
    memnode_t *m;
    int compacted;

    do {
	compacted = 0;

	for (m = pool->root; m; m = m->next) {

	    /* Check seal to be sure that we're doing ok */

	    if (m->seal != MEMNODE_SEAL) {
#ifdef TESTPROG
		printf("Memory list corrupted!\n");
#endif
		return;
		}

	    /*
	     * If we're not on the last block and both this
	     * block and the next one are free, combine them
	     */

	    if (m->next &&
		(m->status == memnode_free) &&
		(m->next->status == memnode_free)) {
		m->length += sizeof(memnode_t) + m->next->length;
		m->next->seal = 0;
		m->next = m->next->next;
		compacted++;
		}

	    /* Keep going till we make a pass without doing anything. */
	    }
	} while (compacted > 0);
}


/*  *********************************************************************
    *  kfree(ptr)
    *
    *  Return some memory to the pool.
    *
    *  Input parameters:
    *  	   ptr - pointer to something allocated via kmalloc()
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */

void kfree(mempool_t *pool,void *ptr)
{
    memnode_t **backptr;
    memnode_t *m;

    if (((unsigned char *) ptr < pool->base) ||
	((unsigned char *) ptr >= (pool->base+pool->length))) {
#ifdef TESTPROG
	printf("Pointer %08X does not belong to pool %08X\n",ptr,pool);
#endif
	return;
	}

    backptr = (memnode_t **) (((unsigned char *) ptr) - sizeof(memnode_t *));
    m = *backptr;

    if (m->seal != MEMNODE_SEAL) {
#ifdef TESTPROG
	printf("Invalid node freed: %08X\n",m);
#endif
	return;
	}

    m->status = memnode_free;

    kmemcompact(pool);
}


/*  *********************************************************************
    *  kmalloc_usable_size(ptr)
    *
    *  Return the number of bytes associated with this previously malloc'd
    * pointer.
    *
    *  Input parameters:
    *	ptr - pointer to something allocated via bmalloc()
    *
    *  Return value:
    *	size of allocation, may include extra alignment bytes.
    ********************************************************************* */
size_t kmalloc_usable_size(mempool_t *pool, void *ptr)
{
	memnode_t **backptr, *m;

	if (!ptr ||
		((unsigned char *)ptr < pool->base) ||
		((unsigned char *)ptr >= (pool->base + pool->length))) {
#ifdef TESTPROG
		printf("Pointer %08X does not belong to pool %08X\n", ptr,
		       pool);
#endif
		return 0;
	}

	backptr = (memnode_t **) (((unsigned char *)ptr) - sizeof(memnode_t *));
	m = *backptr;

	if (m->seal != MEMNODE_SEAL) {
#ifdef TESTPROG
		printf("Invalid node: %08X\n", m);
#endif
		return 0;
	}

	return m->length;
}

/*  *********************************************************************
    *  lib_outofmemory()
    *
    *  Called when we run out of memory.
    *  XXX replace with something real someday
    *
    *  Input parameters:
    *  	   nothing
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */

void lib_outofmemory(void);
void lib_outofmemory(void)
{
    printf("PANIC: out of memory!\n");
}

/*  *********************************************************************
    *  kmalloc(pool,size,align)
    *
    *  Allocate some memory from the pool.
    *
    *  Input parameters:
    *      pool - pool structure
    *  	   size - size of item to allocate
    *  	   align - alignment (must be zero or a power of 2)
    *
    *  Return value:
    *  	   pointer to data, or NULL if no memory left
    ********************************************************************* */

void *kmalloc(mempool_t *pool,unsigned int size,unsigned int align)
{
    memnode_t *m;
    memnode_t *newm;
    memnode_t **backptr;
    uintptr_t daddr = 0;
    uintptr_t realsize = 0;
    uintptr_t extra;
    uintptr_t blkend;
    uintptr_t ptralign;

    /*
     * Everything should be aligned by at least the
     * size of an int64
     */

    ptralign = (uintptr_t) align;
    if (ptralign < sizeof(void *)) ptralign = sizeof(uint64_t);

    /*
     * Everything should be at least a multiple of the
     * size of a pointer.
     */

    if (size == 0) size = sizeof(void *);
    if (size & (sizeof(void *)-1)) {
	size += sizeof(void *);
	size &= ~(sizeof(void *)-1);
	}

    /*
     * Find a memnode at least big enough to hold the storage we
     * want.
     */

    for (m = pool->root; m; m = m->next) {

	if (m->status == memnode_alloc) continue;

	/*
	 * If we wanted a particular alignment, we will
	 * need to adjust the size.
	 */

	daddr = memnode_data(uintptr_t,m);
	extra = 0;
	if (daddr & (ptralign-1)) {
	    extra = ptralign - (daddr & (ptralign-1));
	    }
	realsize = size + extra;

	if (m->length < realsize) continue;
	break;
	}

    /*
     * If m is null, there's no memory left.
     */

    if (m == NULL) {
	lib_outofmemory();
	return NULL;
	}

    /*
     * Otherwise, use this block.  Calculate the address of the data
     * to preserve the alignment.
     */

    if (daddr & (ptralign-1)) {
	daddr += ptralign;
	daddr &= ~(ptralign-1);
	}

    /* Mark this node as allocated. */

    m->data   = (unsigned char *) daddr;
    m->status = memnode_alloc;

    /*
     * Okay, this is ugly.  Store a pointer to the original
     * memnode just before what we've allocated.  It's guaranteed
     * to be aligned at least well enough for this pointer.
     * If for some reason the memnode was already exactly
     * aligned, backing up will put us inside the memnode
     * structure itself... that's why the memnodeptr field
     * is there, as a placeholder for this eventuality.
     */

    backptr   = (memnode_t **) (m->data - sizeof(memnode_t *));
    *backptr  = m;

    /*
     * See if we need to split it.
     * Don't bother to split if the resulting size will be
     * less than MINBLKSIZE bytes
     */

    if (m->length - realsize < MINBLKSIZE) {
	return m->data;
	}

    /*
     * Split this block.  Align the address on a pointer-size
     * boundary.
     */

    daddr += size;
    if (daddr & (uintptr_t)(sizeof(void *)-1)) {
	daddr += (uintptr_t)sizeof(void *);
	daddr &= ~(uintptr_t)(sizeof(void *)-1);
	}

    blkend = memnode_data(uintptr_t,m) + (uintptr_t)(m->length);

    newm = (memnode_t *) daddr;

    newm->next   = m->next;
    m->length    = (unsigned int) (daddr - memnode_data(uintptr_t,m));
    m->next      = newm;
    m->status    = memnode_alloc;
    newm->seal   = MEMNODE_SEAL;
    newm->data    = memnode_data(unsigned char *,newm);
    newm->length = (unsigned int) (blkend - memnode_data(uintptr_t,newm));
    newm->status = memnode_free;

    return m->data;
}


int kmemstats(mempool_t *pool,memstats_t *stats)
{
    memnode_t *m;
    memnode_t **backptr;
    uintptr_t daddr;

    stats->mem_totalbytes = pool->length;
    stats->mem_allocbytes = 0;
    stats->mem_freebytes = 0;
    stats->mem_allocnodes = 0;
    stats->mem_freenodes = 0;
    stats->mem_largest = 0;

    for (m = pool->root; m; m = m->next) {
	if (m->status) {
	    stats->mem_allocnodes++;
	    stats->mem_allocbytes += m->length;
	    }
	else {
	    stats->mem_freenodes++;
	    stats->mem_freebytes += m->length;
	    if (m->length > stats->mem_largest) {
		stats->mem_largest = m->length;
		}
	    }

	daddr = memnode_data(uintptr_t,m);
	if (m->seal != MEMNODE_SEAL) {
	    return -1;
	    }
	if (m->next && ((daddr + m->length) != (uintptr_t) m->next)) {
	    return -1;
	    }
	if (m->next && (m->next < m)) {
	    return -1;
	    }
	if (m->data < (unsigned char *) m) {
	    return -1;
	    }
	if (m->status == memnode_alloc) {
	    backptr = (memnode_t **) (m->data - sizeof(void *));
	    if (*backptr != m) {
		return -1;
		}
	    }
	}

    return 0;
}


/*  *********************************************************************
    *  kmemchk()
    *
    *  Check the consistency of the memory pool.
    *
    *  Input parameters:
    *      pool - pool pointer
    *
    *  Return value:
    *  	   0 - pool is consistent
    *  	   -1 - pool is corrupt
    ********************************************************************* */

#ifdef TESTPROG
int kmemchk(mempool_t *pool,int verbose)
{
    memnode_t *m;
    memnode_t **backptr;
    unsigned int daddr;

    for (m = pool->root; m; m = m->next) {
	if (verbose) {
	    printf("%08X: Next=%08X  Len=%5u  %s  Data=%08X ",
	       m,m->next,m->length,
	       m->status ? "alloc" : "free ",
	       m->data);
	    }
	daddr = memnode_data(uintptr_t,m);
	if (m->seal != MEMNODE_SEAL) {
	    if (verbose) printf("BadSeal ");
	    else return -1;
	    }
	if (m->next && (daddr + m->length != (unsigned int) m->next)) {
	    if (verbose) printf("BadLength ");
	    else return -1;
	    }
	if (m->next && (m->next < m)) {
	    if (verbose) printf("BadOrder ");
	    else return -1;
	    }
	if (m->data < (unsigned char *) m) {
	    if (verbose) printf("BadData ");
	    else return -1;
	    }
	if (m->status == memnode_alloc) {
	    backptr = (memnode_t **) (m->data - sizeof(void *));
	    if (*backptr != m) {
		if (verbose) printf("BadBackPtr ");
		else return -1;
		}
	    }
	if (verbose) printf("\n");
	}

    return 0;
}


#define MEMSIZE 1024*1024

unsigned char *ptrs[4096];
unsigned int sizes[4096];

/*  *********************************************************************
    *  main(argc,argv)
    *
    *  Test program for the memory allocator
    *
    *  Input parameters:
    *  	   argc,argv
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */


void main(int argc,char *argv[])
{
    unsigned char *mem;
    int items = 0;
    int idx;
    int size;
    int totalsize = 0;
    int nfree,freecnt;
    mempool_t *pool = &kmempool;

    mem = malloc(MEMSIZE);
    kmeminit(pool,mem,MEMSIZE);

    items = 0;

    for (;;) {

	for (;;) {
	    if (items == 4096) break;
	    size = rand() % 1024;
	    ptrs[items] = kmalloc(pool,size,1<<(rand() & 7));
	    if (!ptrs[items]) break;
	    sizes[items] = size;
	    items++;
	    totalsize += size;
	    }

	printf("%d items allocated, %d total bytes\n",items,totalsize);

	if (kmemchk(pool,0) < 0) {
	    kmemchk(pool,1);
	    exit(1);
	    }

	/* Scramble the pointers */
	idx = items - 1;

	while (idx) {
	    if (rand() & 2) {
		mem = ptrs[0];
		ptrs[0] = ptrs[idx];
		ptrs[idx] = mem;

		nfree = sizes[0];
		sizes[0] = sizes[idx];
		sizes[idx] = nfree;
		}
	    idx--;
	    }

	/* now free a random number of elements */

	nfree = rand() % items;
	freecnt = 0;

	for (idx = nfree; idx < items; idx++) {
	    kfree(pool,ptrs[idx]);
	    totalsize -= sizes[idx];
	    freecnt++;
	    ptrs[idx] = NULL;
	    sizes[idx] = 0;
	    if (kmemchk(pool,0) < 0) {
		kmemchk(pool,1);
		exit(1);
		}
	    }

	items -= freecnt;

	printf(".");

	}

    kmemchk(pool,1);

    exit(0);
}

#endif	 /* TESTPROG */
