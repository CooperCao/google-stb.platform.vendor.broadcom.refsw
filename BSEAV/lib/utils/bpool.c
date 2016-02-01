/***************************************************************************
 *     Copyright (c) 2007-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Pool alloc library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bpool.h"
#include "blst_selist.h"

BDBG_MODULE(bpool);

#define BDBG_MSG_TRACE(x)  /* BDBG_MSG(x) */

BDBG_OBJECT_ID(bpool_t);

struct bpool {
	struct balloc_iface alloc_iface; /* must be a first member */
	uint16_t last_free;
	uint16_t nfree_elem;
	uint16_t elem_size;
	uint16_t nelem;
	uint32_t *bitmap; /* pointer to bitmap */
	BLST_SE_ENTRY(bpool);
	balloc_iface_t alloc;
	BDBG_OBJECT(bpool_t)
	/* 
	 allocated objects start here 
	*/
	/* 
	 object bitmap start here, and bitmap points here as well 
	*/
};
#define B_POOL_BITS_IN_MAP	32

static unsigned 
b_pool_first_zero(uint32_t word)
{
	unsigned i;
	for(i=0;i<B_POOL_BITS_IN_MAP;i++) {
		if (!(word&(1<<i))) {
			return i;
		}
	}
	BDBG_ASSERT(0);
	return 0;
}

void *
bpool_alloc(bpool_t pool, size_t size)
{
    uint16_t i;
    BSTD_UNUSED(size);

    BDBG_OBJECT_ASSERT(pool, bpool_t);
    if(size>pool->elem_size) {
        BDBG_ERR(("bpool_alloc:%p requested size %u exceeded element size:%u", (void *)pool, (unsigned)size, (unsigned)pool->elem_size));
        return NULL;
    }

	do {
		for(;pool->nfree_elem>0;) {
			for(i=pool->last_free;i<(pool->nelem/B_POOL_BITS_IN_MAP);i++) {
				if(pool->bitmap[i]!=(~0u)) {
					unsigned bit;

					pool->last_free = i;
					bit = b_pool_first_zero(pool->bitmap[i]);
					pool->bitmap[i] |= (1<<bit);
					BDBG_ASSERT(pool->nfree_elem>0);
					pool->nfree_elem--;
					BDBG_MSG_TRACE(("bpool_alloc: %#lx %#lx offset %u [%x:%u]", (unsigned long)pool, (unsigned long)((uint8_t *)pool + sizeof(*pool) + pool->elem_size*(i*B_POOL_BITS_IN_MAP + bit)), i, (unsigned)pool->bitmap[i], bit));
					return (void *)((uint8_t *)pool + sizeof(*pool) + pool->elem_size*(i*B_POOL_BITS_IN_MAP + bit));
				}
			}
			if(pool->last_free==0) {
				break; /* there is no empty space left */
			}
			pool->last_free=0; /* rescan entire bitmap */
		}
		BDBG_MSG(("bpool_alloc:%#lx exhausted", (unsigned long)pool));
	} while(NULL!=(pool=BLST_SE_NEXT(pool)));
	return NULL;
}

static bool 
b_pool_test_one(bpool_t pool, void *ptr)
{
	if ((uint8_t *)ptr >= (uint8_t *)pool + sizeof(*pool) && (uint8_t *)ptr <= (uint8_t *)pool->bitmap - pool->elem_size) {
		return true;
	} else {
		return false;
	}
}

bool
bpool_test_block(bpool_t pool, void *ptr)
{
	BDBG_OBJECT_ASSERT(pool, bpool_t);
	do {
		BDBG_OBJECT_ASSERT(pool, bpool_t);
		if(b_pool_test_one(pool, ptr)) {
			return true;
		}
	} while(NULL!=(pool=BLST_SE_NEXT(pool)));
	return false;
}

void
bpool_join(bpool_t parent, bpool_t child)
{
	BDBG_OBJECT_ASSERT(parent, bpool_t);
	BDBG_OBJECT_ASSERT(child, bpool_t);
	BDBG_ASSERT(parent->elem_size == child->elem_size);
	BDBG_ASSERT(parent != child);

	BLST_SE_INSERT_AFTER(parent, child);
	return;
}

bool
bpool_is_empty(bpool_t pool)
{
	BDBG_OBJECT_ASSERT(pool, bpool_t);
	/* this function doesn't query child pools */
	return pool->nfree_elem == pool->nelem;
}

bpool_t 
bpool_last_child(bpool_t pool)
{
	BDBG_OBJECT_ASSERT(pool, bpool_t);
	/* adjust to the child right the way */
	while(NULL!=(pool=BLST_SE_NEXT(pool))) {
		if (BLST_SE_NEXT(pool)==NULL) {
			return pool;
		}
	}
	return NULL;
}

void 
bpool_detach(bpool_t parent, bpool_t child)
{
	bpool_t pool;

	BDBG_OBJECT_ASSERT(parent, bpool_t);
	BDBG_OBJECT_ASSERT(child, bpool_t);
	BDBG_ASSERT(parent->elem_size == child->elem_size);
	BDBG_ASSERT(parent != child);
	BDBG_ASSERT(bpool_is_empty(child)); /* can't delete busy child */

	pool = parent;
	do {
		if(child==BLST_SE_NEXT(pool)) {
			BLST_SE_REMOVE_BEFORE(pool, child);
			return;
		}
	} while(NULL!=(pool=BLST_SE_NEXT(pool)));
	BDBG_ASSERT(0); /* child doesn't belong to a parent */
	return;
}

void 
bpool_free(bpool_t pool, void *ptr)
{
	unsigned offset;
	unsigned bit;

	BDBG_OBJECT_ASSERT(pool, bpool_t);
	BDBG_ASSERT(ptr);
	BDBG_ASSERT(bpool_test_block(pool, ptr));
	do {
		if (!b_pool_test_one(pool,ptr)) {
			continue; /* try next pool */
		}
		offset = (uint8_t *)ptr - ((uint8_t *)pool + sizeof(*pool));
		BDBG_ASSERT((offset % pool->elem_size) == 0);
		offset = offset/pool->elem_size;
		bit = offset % B_POOL_BITS_IN_MAP;
		offset = offset/B_POOL_BITS_IN_MAP;
		BDBG_MSG_TRACE(("bpool_free: %#lx %#lx offset %u [%#x:%u]", (unsigned long)pool, (unsigned long)ptr, offset, (unsigned)pool->bitmap[offset], bit));
		BDBG_ASSERT(pool->bitmap[offset] & (1<<bit)); /* block shall have been allocated */
		pool->bitmap[offset] &= ~((1<<bit)); /* clear alloc bit */
		BDBG_ASSERT(pool->nfree_elem<pool->nelem);
		pool->nfree_elem++;
		return;
	} while(NULL!=(pool=BLST_SE_NEXT(pool)));
	BDBG_ASSERT(0); /* block wasn't allocated */
	return;
}

static void *
b_pool_alloc(balloc_iface_t alloc, size_t size)
{
	return bpool_alloc((bpool_t)alloc, size);
}

static void
b_pool_free(balloc_iface_t alloc, void *ptr)
{
	bpool_free((bpool_t)alloc, ptr);
}

bpool_t 
bpool_create(balloc_iface_t alloc, size_t nelem, size_t elem_size)
{
	bpool_t pool;
	size_t map_size;

	BDBG_ASSERT(nelem);
	BDBG_ASSERT(elem_size);
	BDBG_ASSERT(alloc);
	BDBG_ASSERT(nelem<65536);
	BDBG_ASSERT(elem_size<65536);

	BDBG_CASSERT(sizeof(uint32_t)==(B_POOL_BITS_IN_MAP/8));
	nelem = (nelem+(B_POOL_BITS_IN_MAP-1))&(~(B_POOL_BITS_IN_MAP-1));
	map_size = nelem/B_POOL_BITS_IN_MAP;

    pool = alloc->bmem_alloc(alloc, nelem*elem_size + sizeof(*pool) + map_size*sizeof(uint32_t));
    BDBG_MSG(("bpool_create: %p overhead %u allocated %u (bytes)", (void *)pool, (unsigned)sizeof(*pool), (unsigned)(nelem*elem_size + sizeof(*pool) + map_size*sizeof(uint32_t))));
    if (!pool) {
        BDBG_ERR(("alloc failed (%u bytes)", (unsigned)(nelem*elem_size + sizeof(*pool) + map_size*sizeof(uint32_t))));
        return NULL;
    }
	BDBG_OBJECT_INIT(pool, bpool_t);
	pool->alloc_iface.bmem_alloc = b_pool_alloc;
	pool->alloc_iface.bmem_free = b_pool_free;
	pool->alloc = alloc;
	BLST_SE_INIT(pool);
	pool->bitmap = (uint32_t *)((uint8_t *)pool + sizeof(*pool) + nelem*elem_size);
	pool->nelem = nelem;
	pool->elem_size = elem_size;
	pool->last_free = 0;
	pool->nfree_elem = nelem;
	BKNI_Memset(pool->bitmap, 0, map_size*sizeof(uint32_t)); /* mark all as free */
	return pool;
}

void
bpool_destroy(bpool_t pool)
{
	balloc_iface_t alloc;
	bpool_t next;

	BDBG_MSG_TRACE(("bpool_destroy: >%#lx", pool));
	BDBG_OBJECT_ASSERT(pool, bpool_t);
	do {
		BDBG_MSG_TRACE(("bpool_destroy: +%#lx", pool));
		BDBG_OBJECT_ASSERT(pool, bpool_t);
		alloc = pool->alloc;
		next = BLST_SE_NEXT(pool);
		BDBG_OBJECT_DESTROY(pool, bpool_t);
		alloc->bmem_free(alloc, pool);
		BDBG_MSG_TRACE(("bpool_destroy: -%#lx", pool));
		pool=next;
	} while(pool);
	BDBG_MSG_TRACE(("bpool_destroy: <%#lx", pool));
	return;
}


balloc_iface_t 
bpool_alloc_iface(bpool_t pool)
{
	BDBG_OBJECT_ASSERT(pool, bpool_t);
	return &pool->alloc_iface;
}

void
bpool_get_status(bpool_t pool, bpool_status *status)
{
	BDBG_OBJECT_ASSERT(pool, bpool_t);
	BDBG_ASSERT(status);

	status->pools = 0;
	status->free = 0;
	status->allocated = 0;
	for(;pool;pool=BLST_SE_NEXT(pool)) {
		BDBG_OBJECT_ASSERT(pool, bpool_t);
		status->pools ++;
		status->free += pool->nfree_elem;
		status->allocated += pool->nelem - pool->nfree_elem;
	}
}

void
bpool_dump(bpool_t pool)
{
	uint16_t i,bit;
	BDBG_OBJECT_ASSERT(pool, bpool_t);

	for(;pool;pool=BLST_SE_NEXT(pool)) {
		BDBG_WRN(("bpool_dump: pool %#lx free:%u allocated:%u", (unsigned long)pool, pool->nfree_elem, pool->nelem-pool->nfree_elem));
		for(i=0;i<(pool->nelem/B_POOL_BITS_IN_MAP);i++) {
			if(pool->bitmap[i]) {
				for(bit=0;bit<B_POOL_BITS_IN_MAP;bit++) {
					if(pool->bitmap[i]&(1<<bit)) {
						BDBG_WRN(("bpool_dump: %#lx %#lx offset %u [%#x:%u]", (unsigned long)pool, (unsigned long)((uint8_t *)pool + sizeof(*pool) + pool->elem_size*(i*B_POOL_BITS_IN_MAP + bit)), i, (unsigned)pool->bitmap[i], bit));
					}
				}
			}
		}
	}
}

