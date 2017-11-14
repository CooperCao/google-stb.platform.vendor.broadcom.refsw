/***************************************************************************
 *  Copyright (C) 2012-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 **************************************************************************/
#include "bstd.h"
#include "barena.h"
#include "blst_selist.h"
#include "blst_queue.h"
BDBG_MODULE(barena);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x)  */

BDBG_OBJECT_ID(barena_t);

typedef struct b_arena_block {
	BLST_Q_ENTRY(b_arena_block) link;
	uint16_t size; /* size in  sizeof(unsigned) units of entire block (include headers)  */
	uint16_t tag; /* this is offset in to the arean, only used to verify validity of block */
	/* user data located here */
} b_arena_block;

struct barena {
	struct balloc_iface alloc_iface; /* must be a first member */
	BLST_SE_ENTRY(barena);
	unsigned free_head; /* number of free bytes in the head of arena */
	unsigned free_tail; /* number of free bytes in the tail of arena */
    BLST_Q_HEAD(b_arena_head, b_arena_block) blocks;
	unsigned size; /* size of arena, without header */
	balloc_iface_t alloc;
	BDBG_OBJECT(barena_t)
	/*
	  memory pool located here
	*/
};

#define B_ARENA_MAX_ALLOC	((0xFFFF)*sizeof(unsigned))
#define B_ARENA_ROUND(size) (((size)+(sizeof(unsigned)-1))&~(sizeof(unsigned)-1))

#define B_BLOCK_TAG(arena, block) ((uint16_t)((uint8_t *)(block) - (uint8_t *)(arena))/sizeof(unsigned))

/* coverity[+alloc] */
void *
barena_alloc(barena_t arena, size_t size)
{
	BDBG_MSG_TRACE(("barena_alloc: >%#lx %u", (unsigned long)arena, (unsigned)size));
	BDBG_OBJECT_ASSERT(arena, barena_t);
	size = B_ARENA_ROUND(size);
	BDBG_CASSERT(B_ARENA_MAX_ALLOC>sizeof(b_arena_block));
	if (size >= B_ARENA_MAX_ALLOC-sizeof(b_arena_block)) { 
		goto err_size;
	}
	size += sizeof(b_arena_block);
	BDBG_MSG_TRACE(("barena_alloc: %#lx head %u tail %u", (unsigned long)arena, arena->free_head, arena->free_tail));
	do {
		b_arena_block *block;
		BDBG_OBJECT_ASSERT(arena, barena_t);
		if (arena->free_head >= size) {
			/* allocate entry from the head */
			block = (b_arena_block*)((uint8_t *)arena+sizeof(*arena)+ (arena->free_head - size));
			BDBG_MSG_TRACE(("barena_alloc: %#lx head block %#lx", (unsigned long)arena, (unsigned long)block));
			arena->free_head -= size;
			BLST_Q_INSERT_HEAD(&arena->blocks, block, link); /* this would keep blocks sorted by their address */
			block->size = size/sizeof(unsigned);
			block->tag = B_BLOCK_TAG(arena, block);
		} else if (arena->free_tail >= size) {
			block = (b_arena_block*)((uint8_t *)arena+sizeof(*arena)+ (arena->size - arena->free_tail));
			BDBG_MSG_TRACE(("barena_alloc: %#lx tail block %#lx", (unsigned long)arena, (unsigned long)block));
			arena->free_tail -= size;
			BLST_Q_INSERT_TAIL(&arena->blocks, block, link); /* this would keep blocks sorted by their address */
			block->size = size/sizeof(unsigned);
			block->tag = ((uint8_t *)block - (uint8_t *)arena)/sizeof(unsigned);
		} else {
			continue;
		}
		BDBG_MSG_TRACE(("barena_alloc: <%#lx %#lx", (unsigned long)arena, (unsigned long)block+sizeof(*block)));
		return (uint8_t *)block+sizeof(*block);
	} while(NULL!=(arena=BLST_SE_NEXT(arena)));

	return NULL;
err_size:
	BDBG_WRN(("barena_alloc: requested size %u exceeds limit %u", (unsigned)size, (unsigned)(B_ARENA_MAX_ALLOC - sizeof(b_arena_block))));
	/* too large block */
	return NULL;
}

static bool 
b_arena_test_one(barena_t arena, void *ptr)
{
	if ((uint8_t *)ptr >= (uint8_t *)arena + sizeof(*arena) && (uint8_t *)arena <= (uint8_t *)arena + (sizeof(*arena) - sizeof(b_arena_block)) + arena->size) {
		return true;
	} else {
		return false;
	}
}

bool
barena_test_block(barena_t arena, void *ptr)
{
	BDBG_OBJECT_ASSERT(arena, barena_t);
	do {
		BDBG_OBJECT_ASSERT(arena, barena_t);
		if(b_arena_test_one(arena, ptr)) {
			return true;
		}
	} while(NULL!=(arena=BLST_SE_NEXT(arena)));
	return false;
}

/* coverity[+free : arg-1] */
void
barena_free(barena_t arena, void *ptr)
{
	unsigned new_size;
	BDBG_OBJECT_ASSERT(arena, barena_t);
	BDBG_MSG_TRACE(("barena_free: >%#lx %#lx", (unsigned long)arena, (unsigned long)ptr));
	BDBG_ASSERT(barena_test_block(arena, ptr));
	do {
		b_arena_block *block;
		b_arena_block *next_block;
		unsigned arena_size; 
		if(!b_arena_test_one(arena, ptr)) {
			continue; /* try next arena */
		}
		block = (b_arena_block *)(((uint8_t *)ptr) - sizeof(*block));
		/* test if block belongs to us */
		BDBG_ASSERT(block->tag == B_BLOCK_TAG(arena, block));
		BDBG_MSG_TRACE(("barena_free: %#lx block %#lx(%u) first:%#lx  last:%#lx prev:%#lx next:%#lx", (unsigned long)arena, (unsigned long)block, block->size, (unsigned long)BLST_Q_FIRST(&arena->blocks), (unsigned long)BLST_Q_LAST(&arena->blocks), (unsigned long)BLST_Q_PREV(block,link), (unsigned long)BLST_Q_NEXT(block,link)));
		BDBG_ASSERT(BLST_Q_PREV(block,link)==NULL || b_arena_test_one(arena, BLST_Q_PREV(block,link)));
		BDBG_ASSERT(BLST_Q_PREV(block,link)!=NULL || BLST_Q_FIRST(&arena->blocks)==block);
		BDBG_ASSERT(BLST_Q_NEXT(block,link)==NULL || b_arena_test_one(arena, BLST_Q_NEXT(block,link)));
		BDBG_ASSERT(BLST_Q_NEXT(block,link)!=NULL || BLST_Q_LAST(&arena->blocks)==block);
		block->tag = 0; /* clear tag */
		arena_size = arena->size;
		if(block==BLST_Q_LAST(&arena->blocks)) {
			next_block = BLST_Q_PREV(block, link);
			if(next_block) {
				new_size = ((uint8_t *)arena + arena_size + sizeof(*arena))- ((uint8_t*)next_block + next_block->size*sizeof(unsigned));
				BDBG_ASSERT(new_size >= arena->free_tail+block->size*sizeof(unsigned));
				arena->free_tail = new_size;
			} else {
				arena->free_head = 0;
				arena->free_tail = arena_size;
			}
		} else if(block==BLST_Q_FIRST(&arena->blocks)) {
			next_block = BLST_Q_NEXT(block, link);
			if(next_block) {
				new_size = ((uint8_t*)next_block) - ((uint8_t *)arena + sizeof(*arena)); 
				BDBG_ASSERT(new_size >= arena->free_head+block->size*sizeof(unsigned));
				arena->free_head = new_size;
			} else {
				arena->free_head = 0;
				arena->free_tail = arena_size;
			}
		}
		BDBG_ASSERT(arena->free_head + arena->free_tail <= arena->size);
#if 0
		if (arena->free_head+arena->free_tail == arena->size) {
			arena->free_head = 0;
			arena->free_tail = arena->size;
		}
#endif
		BLST_Q_REMOVE(&arena->blocks, block, link);
		BDBG_MSG_TRACE(("barena_free: %#lx head %u tail %u", (unsigned long)arena, arena->free_head, arena->free_tail));
		BDBG_MSG_TRACE(("barena_free: <%#lx", (unsigned long)arena));
		return;
	} while(NULL!=(arena=BLST_SE_NEXT(arena)));
	BDBG_MSG_TRACE(("barena_free: <%#lx %#lx", (unsigned long)arena, (unsigned long)ptr));
	BDBG_ASSERT(0);
	return;
}

static void *
b_arena_alloc(balloc_iface_t alloc, size_t size)
{
	return barena_alloc((barena_t)alloc, size);
}

static void
b_arena_free(balloc_iface_t alloc, void *ptr)
{
	barena_free((barena_t)alloc, ptr);
}

barena_t
barena_create(balloc_iface_t alloc, size_t arena_size)
{
	barena_t arena;
	BDBG_ASSERT(alloc);

	arena_size = B_ARENA_ROUND(arena_size);
	arena = alloc->bmem_alloc(alloc, sizeof(*arena)+arena_size);
	BDBG_MSG(("barena_create: %p overhead %u allocated %u (bytes)", (void *)arena, (unsigned)sizeof(b_arena_block), (unsigned)(arena_size + sizeof(*arena))));
	if (!arena) {
		BDBG_ERR(("alloc failed (%u bytes)", (unsigned)(arena_size + sizeof(*arena))));
		return NULL;
	}
	BDBG_OBJECT_INIT(arena, barena_t);
	arena->alloc = alloc;
	BLST_SE_INIT(arena);
	arena->free_head = 0;
	arena->free_tail = arena_size;
	arena->size = arena_size;
	BLST_Q_INIT(&arena->blocks);
	arena->alloc_iface.bmem_alloc = b_arena_alloc;
	arena->alloc_iface.bmem_free = b_arena_free;
	return arena;
}

void
barena_destroy(barena_t arena)
{
	balloc_iface_t alloc;
	barena_t next;
	BDBG_MSG_TRACE(("barena_destroy: >%#lx", arena));

	BDBG_OBJECT_ASSERT(arena, barena_t);
	do {
		BDBG_OBJECT_ASSERT(arena, barena_t);
		alloc = arena->alloc;
		next = BLST_SE_NEXT(arena);
		BDBG_OBJECT_DESTROY(arena, barena_t);
		BDBG_MSG_TRACE(("barena_destroy: %#lx", arena));
		alloc->bmem_free(alloc, arena);
		arena = next;
	} while(arena);
	BDBG_MSG_TRACE(("barena_destroy: <%#lx", arena));
	return;
}

balloc_iface_t 
barena_alloc_iface(barena_t arena)
{
	BDBG_OBJECT_ASSERT(arena, barena_t);
	return &arena->alloc_iface;
}

void
barena_join(barena_t parent, barena_t child)
{
	BDBG_OBJECT_ASSERT(parent, barena_t);
	BDBG_OBJECT_ASSERT(child, barena_t);
	BDBG_ASSERT(parent != child);

	BLST_SE_INSERT_AFTER(parent, child);
	return;
}

static bool
barena_is_empty(barena_t arena)
{
	BDBG_OBJECT_ASSERT(arena, barena_t);
	/* this function doesn't query child pools */
	return BLST_Q_FIRST(&arena->blocks)==NULL;
}

barena_t
barena_last_child(barena_t arena)
{
	BDBG_OBJECT_ASSERT(arena, barena_t);
	/* adjust to the child right the way */
	while(NULL!=(arena=BLST_SE_NEXT(arena))) {
		if (BLST_SE_NEXT(arena)==NULL) {
			return arena;
		}
	}
	return NULL;
}

void 
barena_detach(barena_t parent, barena_t child)
{
	barena_t arena;

	BDBG_OBJECT_ASSERT(parent, barena_t);
	BDBG_OBJECT_ASSERT(child, barena_t);
	BDBG_ASSERT(parent != child);
	BDBG_ASSERT(barena_is_empty(child)); /* can't delete busy child */

	arena = parent;
	do {
		if(child==BLST_SE_NEXT(arena)) {
			BLST_SE_REMOVE_BEFORE(arena, child);
			return;
		}
	} while(NULL!=(arena=BLST_SE_NEXT(arena)));
	BDBG_ASSERT(0); /* child doesn't belong to a parent */
	return;
}

void 
barena_dump(barena_t arena)
{
	b_arena_block *block;
	size_t total;
	BDBG_OBJECT_ASSERT(arena, barena_t);
	for(total=0,block=BLST_Q_FIRST(&arena->blocks);block;block=BLST_Q_NEXT(block,link)) {
		total += sizeof(unsigned)*(unsigned)block->size;
		BDBG_WRN(("barena_dump: %p %p:%u", (void *)arena, (void *)block, (unsigned)(sizeof(unsigned)*(unsigned)block->size)));
	}
	BDBG_WRN(("barena_dump: %p size:%u head:%u tail:%u overhead:%u", (void *)arena, arena->size, arena->free_head, arena->free_tail, (unsigned)(arena->size-total)));
	return;
}
