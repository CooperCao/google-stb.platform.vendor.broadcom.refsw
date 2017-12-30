/***************************************************************************
 * Copyright (C) 2012-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 ***************************************************************************/
#include "bstd.h"
#include "blst_list.h"
#include "bkni.h"
#include "bmma_pool.h"
#include "bmma_range.h"
#include "blst_aa_tree.h"


BDBG_MODULE(BMMA_Range);


#define BDBG_MSG_TRACE(x) BDBG_MSG(x)

void BMMA_RangeAllocator_GetDefaultCreateSettings(BMMA_RangeAllocator_CreateSettings *settings)
{
    BKNI_Memset(settings, 0, sizeof(*settings));
    settings->minAlignment = sizeof(void *);
    settings->printLeakedRanges = true;
    settings->allocatorMinFreeSize = 0;
    settings->verbose = true;
    return;
}

void BMMA_RangeAllocator_GetDefaultAllocationSettings(BMMA_RangeAllocator_BlockSettings *settings)
{
    BKNI_Memset(settings, 0, sizeof(*settings));
    settings->alignment = sizeof(void *);
    settings->boundary = 0;
    return;
}


BDBG_OBJECT_ID(BMMA_RangeAllocator_Block);
struct  BMMA_RangeAllocator_Block {
    BDBG_OBJECT(BMMA_RangeAllocator_Block)
    BLST_D_ENTRY(BMMA_RangeAllocator_Block) link;
    BLST_AA_TREE_ENTRY(BMMA_RangeAllocator_SizeTree) size_node; /* tree of free nodes sorted by size */
    BMMA_RangeAllocator_Region region; /* address and length of the block */
    struct {
        bool allocated;
        bool hole; /* hole is the memory that can't be used for allocations, if hole==true, then allocated==true */
        size_t header;
        size_t size;
        BMMA_RangeAllocator_BlockSettings settings;
    } state;
    void *header[1]; /* variable length application controlled header */
};

BLST_AA_TREE_HEAD(BMMA_RangeAllocator_SizeTree, BMMA_RangeAllocator_Block);

static int BMMA_RangeAllocator_SizeTree_Compare_isrsafe(const struct BMMA_RangeAllocator_Block *node, const BMMA_RangeAllocator_Region *region)
{
    if(region->length > node->region.length) {
        return -1;
    } else if(region->length < node->region.length) {
        return 1;
    } else if(region->base > node->region.base) {
        return 1;
    } else if(region->base < node->region.base) {
        return -1;
    } else {
        return 0;
    }
}

#if 0
BLST_AA_TREE_GENERATE_FIND(BMMA_RangeAllocator_SizeTree, const BMMA_RangeAllocator_Region *, BMMA_RangeAllocator_Block, size_node, BMMA_RangeAllocator_SizeTree_Compare_isrsafe)
#endif
BLST_AA_TREE_GENERATE_INSERT(BMMA_RangeAllocator_SizeTree, const BMMA_RangeAllocator_Region *, BMMA_RangeAllocator_Block, size_node, BMMA_RangeAllocator_SizeTree_Compare_isrsafe)
BLST_AA_TREE_GENERATE_REMOVE(BMMA_RangeAllocator_SizeTree, BMMA_RangeAllocator_Block, size_node)
BLST_AA_TREE_GENERATE_FIRST(BMMA_RangeAllocator_SizeTree, BMMA_RangeAllocator_Block, size_node)
BLST_AA_TREE_GENERATE_NEXT(BMMA_RangeAllocator_SizeTree, BMMA_RangeAllocator_Block, size_node)
BLST_AA_TREE_GENERATE_LAST(BMMA_RangeAllocator_SizeTree, BMMA_RangeAllocator_Block, size_node)
BLST_AA_TREE_GENERATE_PREV(BMMA_RangeAllocator_SizeTree, BMMA_RangeAllocator_Block, size_node)

BDBG_OBJECT_ID(BMMA_RangeAllocator);
struct BMMA_RangeAllocator {
    BDBG_OBJECT(BMMA_RangeAllocator)
    BLST_D_HEAD(BMMA_P_RangeAllocator_BlocksList, BMMA_RangeAllocator_Block) blocks; /* list of free and allocated blocks, sorted by the base address */
    struct BMMA_RangeAllocator_SizeTree size_tree;
    BMMA_PoolAllocator_Handle blockAllocator; /* allocator to allocate headers */
    BMMA_RangeAllocator_Status status;
    BMMA_RangeAllocator_CreateSettings settings;
    BMMA_RangeAllocator_Region allocatorRegion; /* entire region, which includes added regions */
};

void BMMA_RangeAllocator_Verify(BMMA_RangeAllocator_Handle a, bool printAllocations)
{
    BMMA_RangeAllocator_Block_Handle b,prev;
    unsigned n;
    unsigned freeBlocks;
#if BDBG_DEBUG_BUILD
    unsigned blocks;
#endif
    unsigned holes;
    size_t allocatedSpace,freeSpace;

    BDBG_OBJECT_ASSERT(a, BMMA_RangeAllocator);
    for(prev=NULL,n=0,b=BLST_AA_TREE_FIRST(BMMA_RangeAllocator_SizeTree, &a->size_tree); b!=NULL; b = BLST_AA_TREE_NEXT(BMMA_RangeAllocator_SizeTree, &a->size_tree,b),n++) {
        BDBG_OBJECT_ASSERT(b, BMMA_RangeAllocator_Block);
        BDBG_ASSERT(!b->state.allocated);
        BDBG_ASSERT(n<a->status.freeBlocks);
        if(printAllocations) {
            BDBG_LOG(("FREE: %p %p:[%c:%u,%u]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", (void *)a, (void *)b, b->state.allocated?(b->state.hole?'H':'A'):'F', (unsigned)b->region.length, (unsigned)b->state.size, BDBG_UINT64_ARG(b->region.base), BDBG_UINT64_ARG(b->region.base+b->region.length)));
        }

        if(prev) {
            BDBG_ASSERT(prev->region.length >= b->region.length);
            if(prev->region.length == b->region.length) {
                BDBG_ASSERT(prev->region.base < b->region.base);
            }
        }
        prev = b;
    }
    BDBG_ASSERT(n==a->status.freeBlocks);
#if BDBG_DEBUG_BUILD
    blocks = a->status.freeBlocks + a->status.allocatedBlocks;
#endif

    for(allocatedSpace=freeSpace=0, holes=freeBlocks=0, n=0,prev=NULL, b=BLST_D_FIRST(&a->blocks);b!=NULL;n++) {
        BMMA_RangeAllocator_Block_Handle next;

        BDBG_OBJECT_ASSERT(b, BMMA_RangeAllocator_Block);
        if(printAllocations) {
#if BDBG_DEBUG_BUILD
            BDBG_LOG(("%p %p:[%c:%u,%u]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " at %s:%u", (void *)a, (void *)b, b->state.allocated?(b->state.hole?'H':'A'):'F', (unsigned)b->region.length, (unsigned)b->state.size, BDBG_UINT64_ARG(b->region.base),  BDBG_UINT64_ARG(b->region.base+b->region.length), b->state.settings.fname, b->state.settings.line));
#else
            BDBG_LOG(("%p %p:[%c:%u,%u]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", (void *)a, (void *)b, b->state.allocated?(b->state.hole?'H':'A'):'F', (unsigned)b->region.length, (unsigned)b->state.size, BDBG_UINT64_ARG(b->region.base),  BDBG_UINT64_ARG(b->region.base+b->region.length)));
#endif
        }
        if(prev) {
            if(b->region.base != prev->region.base + prev->region.length) {
                BDBG_LOG(("%p verify %p:[%c]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " %p:[%c]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", (void *)a, (void *)prev, prev->state.allocated?'A':'F', BDBG_UINT64_ARG(prev->region.base), BDBG_UINT64_ARG(prev->region.base+prev->region.length), (void *)b, b->state.allocated?(b->state.hole?'H':'A'):'F', BDBG_UINT64_ARG(b->region.base), BDBG_UINT64_ARG(b->region.base+b->region.length)));
            }
            BDBG_ASSERT(b->region.base == prev->region.base + prev->region.length);
            BDBG_ASSERT(b->state.allocated || prev->state.allocated); /* no back-to-back free blocks */
            BDBG_ASSERT(!b->state.hole || !prev->state.hole); /* no back-to-back 'hole' blocks */
        } else {
            BDBG_ASSERT(b->region.base == a->allocatorRegion.base);
        }
        if(!b->state.allocated) {
            freeSpace += b->region.length;
            freeBlocks++;
        } else {
            if(b->state.hole) {
                holes++;
            } else {
                allocatedSpace += b->region.length;
            }
        }
        BDBG_ASSERT(n<blocks+holes);
        next = BLST_D_NEXT(b, link);
        if(next==NULL) {
            BDBG_ASSERT(b->region.base + b->region.length == a->allocatorRegion.base + a->allocatorRegion.length);
        }
        prev = b;
        b = next;
    }
    BDBG_ASSERT(n==blocks+holes);
    BDBG_ASSERT(freeBlocks==a->status.freeBlocks);
    BDBG_ASSERT(a->status.allocatedSpace == allocatedSpace);
    BDBG_ASSERT(a->status.freeSpace == freeSpace);
    BDBG_ASSERT(a->status.allocatedSpace + a->status.freeSpace == a->settings.size);
    return;
}

#define BMMA_P_WHEN_PRINT_HEAP(x) (0 && ((x)->status.allocatedBlocks + (x)->status.freeBlocks) % 1000 == 0)
#define BMMA_P_STATUS(x)  do {BDBG_MSG(("%s:%p allocated:%u,%u free:%u,%u", BSTD_FUNCTION, (void *)x, x->status.allocatedBlocks, (unsigned)x->status.allocatedSpace, x->status.freeBlocks, (unsigned)x->status.freeSpace));if(BMMA_P_WHEN_PRINT_HEAP(x)){BMMA_RangeAllocator_Verify(x,true);}}while(0)

static BMMA_DeviceOffset BMMA_P_RangeAllocator_Align(BMMA_DeviceOffset v, unsigned alignment)
{
    BMMA_DeviceOffset r;
    r = v + (alignment - 1);
    r -= r%alignment;
    return r;
}

static void BMMA_RangeAllocator_P_FreeNode(BMMA_RangeAllocator_Handle a, BMMA_RangeAllocator_Block_Handle b)
{
    if(b) {
        BDBG_OBJECT_DESTROY(b, BMMA_RangeAllocator_Block);
        BMMA_PoolAllocator_Free(a->blockAllocator, b);
    }
    return;
}

static BMMA_RangeAllocator_Block_Handle BMMA_RangeAllocator_P_AllocateNode(BMMA_RangeAllocator_Handle a, BMMA_DeviceOffset base, BMMA_Size size, BMMA_RangeAllocator_Block_Handle b)
{
    if(b==NULL) {
        b = BMMA_PoolAllocator_Alloc(a->blockAllocator);
        if(b==NULL) {(void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);return NULL;}
    }
    BDBG_OBJECT_INIT(b, BMMA_RangeAllocator_Block);
    b->region.base = base;
    b->region.length = size;
    b->state.size = 0;
#if BDBG_DEBUG_BUILD
    b->state.settings.fname = NULL;
    b->state.settings.line = 0;
#endif
    return b;
}


static void
BMMA_RangeAllocator_P_InsertFreeNodeSorted(BMMA_RangeAllocator_Handle a, BMMA_RangeAllocator_Block_Handle b)
{
    BMMA_RangeAllocator_Block_Handle blockInserted;

    b->state.size = 0;
    blockInserted = BLST_AA_TREE_INSERT(BMMA_RangeAllocator_SizeTree, &a->size_tree, &b->region, b);
    BSTD_UNUSED(blockInserted);
    BDBG_ASSERT(blockInserted==b);
    return;
}



static BERR_Code BMMA_RangeAllocator_P_InsertFreeNodeAfter(BMMA_RangeAllocator_Handle a, const BMMA_RangeAllocator_Region *block, BMMA_RangeAllocator_Block_Handle b, BMMA_RangeAllocator_Block_Handle prev)
{
    BMMA_Size block_length = block->length; /* save a copy of block->length, since it could actually mutate among with the node (due to pointer aliasing) */
    if(block_length==0) {
        BMMA_RangeAllocator_P_FreeNode(a, b);
        return BERR_SUCCESS;
    }
    if(prev) {
        BMMA_RangeAllocator_Block_Handle next = BLST_D_NEXT(prev, link);
        BDBG_MSG_TRACE(("%p:merge [%c]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT  " " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " [%c]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", (void *)a, prev->state.allocated?(prev->state.hole?'H':'A'):'F', BDBG_UINT64_ARG(prev->region.base), BDBG_UINT64_ARG(prev->region.base+prev->region.length), BDBG_UINT64_ARG(block->base), BDBG_UINT64_ARG(block->base+block->length), next?(next->state.allocated?(next->state.hole?'H':'A'):'F'):'N', BDBG_UINT64_ARG(next?next->region.base:0), BDBG_UINT64_ARG(next?next->region.base+next->region.length:0)));
        BDBG_ASSERT(prev->region.base + prev->region.length == block->base);
        if(next) {
            if(block->base + block->length != next->region.base) {
                BDBG_LOG(("%p:merge [%c]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT  " " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " [%c]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", (void *)a, prev->state.allocated?(prev->state.hole?'H':'A'):'F', BDBG_UINT64_ARG(prev->region.base), BDBG_UINT64_ARG(prev->region.base+prev->region.length), BDBG_UINT64_ARG(block->base), BDBG_UINT64_ARG(block->base+block->length), next?(next->state.allocated?(next->state.hole?'H':'A'):'F'):'N', BDBG_UINT64_ARG(next?next->region.base:0), BDBG_UINT64_ARG(next?next->region.base+next->region.length:0)));
            }
            BDBG_ASSERT(block->base + block->length == next->region.base);
        } else {
            BDBG_ASSERT(block->base + block->length == a->allocatorRegion.base+a->allocatorRegion.length);
        }
        if(!prev->state.allocated) {
            BDBG_MSG_TRACE(("%p:merged " BDBG_UINT64_FMT "..%u->%u", (void *)a, BDBG_UINT64_ARG(prev->region.base), (unsigned)prev->region.length, (unsigned)(prev->region.length+block->length)));
            prev->region.length += block->length; /* added in back of the prev node, combine 2 nodes */
            if(next && !next->state.allocated) { /* added between prev and next nodes, combine 3 nodes */
                BDBG_MSG_TRACE(("%p:merged " BDBG_UINT64_FMT "..%u->%u", (void *)a, BDBG_UINT64_ARG(prev->region.base), (unsigned)prev->region.length, (unsigned)(prev->region.length+next->region.length)));
                prev->region.length += next->region.length;
                BLST_D_REMOVE(&a->blocks, next, link);
                BLST_AA_TREE_REMOVE(BMMA_RangeAllocator_SizeTree, &a->size_tree, next);
                BMMA_RangeAllocator_P_FreeNode(a, next);
                a->status.freeBlocks --;
            }
            BLST_AA_TREE_REMOVE(BMMA_RangeAllocator_SizeTree, &a->size_tree, prev);
            BMMA_RangeAllocator_P_InsertFreeNodeSorted(a, prev);
            BMMA_RangeAllocator_P_FreeNode(a, b);
            goto done;
        } if(next && !next->state.allocated) { /* added in front of next nodes, combine 2 nodes */
            BDBG_MSG_TRACE(("%p:merged " BDBG_UINT64_FMT "..%u->%u", (void *)a, BDBG_UINT64_ARG(block->base), (unsigned)block->length, (unsigned)(block->length+next->region.length)));
            next->region.base = block->base;
            next->region.length += block->length;
            BLST_AA_TREE_REMOVE(BMMA_RangeAllocator_SizeTree, &a->size_tree, next);
            BMMA_RangeAllocator_P_InsertFreeNodeSorted(a, next);
            BMMA_RangeAllocator_P_FreeNode(a, b);
            goto done;
        }
    } else /* (prev==NULL) */ {
        BMMA_RangeAllocator_Block_Handle n;
        BDBG_ASSERT(block->base==a->allocatorRegion.base);
        n = BLST_D_FIRST(&a->blocks);
        BDBG_MSG_TRACE(("%p:merge " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "[%c]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", (void *)a, BDBG_UINT64_ARG(block->base), BDBG_UINT64_ARG(block->base+block->length), n?(n->state.allocated?(n->state.hole?'H':'A'):'F'):'N', BDBG_UINT64_ARG(n?n->region.base:0), BDBG_UINT64_ARG(n?n->region.base+n->region.length:0)));
        if(n) {
            BDBG_ASSERT(block->base+block->length==n->region.base);
            if(!n->state.allocated) { /* added in front of the very first node, no further collapsing possible */
                n->region.base = block->base;
                n->region.length += block->length;
                BDBG_MSG_TRACE(("%p:merged " BDBG_UINT64_FMT "..%u->" BDBG_UINT64_FMT, (void *)a, BDBG_UINT64_ARG(block->base), (unsigned)block->length, BDBG_UINT64_ARG(block->base+n->region.length)));
                BLST_AA_TREE_REMOVE(BMMA_RangeAllocator_SizeTree, &a->size_tree, n);
                BMMA_RangeAllocator_P_InsertFreeNodeSorted(a, n);
                BMMA_RangeAllocator_P_FreeNode(a, b);
                goto done;
            }
        } else {
            if(!a->status.dynamic) {
                BDBG_ASSERT(block->length==a->settings.size);
            }
        }
    }
    b = BMMA_RangeAllocator_P_AllocateNode(a, block->base, block->length, b);
    if(b==NULL) {return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);}
    a->status.freeBlocks ++;
    b->state.allocated = false;
    b->state.hole = false;
    if(prev) {
        BDBG_MSG_TRACE(("insert " BDBG_UINT64_FMT ":%u after " BDBG_UINT64_FMT "", BDBG_UINT64_ARG(b->region.base), (unsigned)b->region.length, BDBG_UINT64_ARG(prev->region.base)));
        BLST_D_INSERT_AFTER(&a->blocks, prev, b, link);
    } else {
        BDBG_MSG_TRACE(("insert " BDBG_UINT64_FMT ":%u head", BDBG_UINT64_ARG(b->region.base), (unsigned)b->region.length));
        BLST_D_INSERT_HEAD(&a->blocks, b, link);
    }
    BMMA_RangeAllocator_P_InsertFreeNodeSorted(a, b);
done:
    a->status.freeSpace += block_length;
    return BERR_SUCCESS;
}



BERR_Code BMMA_RangeAllocator_Create(BMMA_RangeAllocator_Handle *allocator, const BMMA_RangeAllocator_CreateSettings *settings)
{
    BERR_Code rc;
    BMMA_RangeAllocator_Handle  a;
    BMMA_PoolAllocator_CreateSettings poolSettings;
    BDBG_ASSERT(settings);

    if(settings->minAlignment==0) {
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    a = BKNI_Malloc(sizeof(*a));
    if(a==NULL) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BDBG_OBJECT_INIT(a, BMMA_RangeAllocator);
    a->settings = *settings;
#if 1
        /* XXX to be deleted */
    if(settings->minAlignment==0) {
        a->settings.minAlignment=sizeof(void*);
    }
#endif
    a->status.allocatedBlocks = 0;
    a->status.allocatedSpace = 0;
    a->status.highWatermark = 0;
    a->status.freeBlocks = 0;
    a->status.freeSpace = 0;
    a->status.dynamic = false;
    BLST_D_INIT(&a->blocks);
    BLST_AA_TREE_INIT(BMMA_RangeAllocator_SizeTree, &a->size_tree);
    BMMA_PoolAllocator_GetDefaultCreateSettings(&poolSettings);
    poolSettings.allocationSize = sizeof(struct BMMA_RangeAllocator_Block) + BMMA_P_RangeAllocator_Align(settings->allocationHeader, sizeof(void*));
    rc = BMMA_PoolAllocator_Create(&a->blockAllocator, &poolSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_pool;}
    a->allocatorRegion.base = settings->base;
    a->allocatorRegion.length = settings->size;
    rc = BMMA_RangeAllocator_P_InsertFreeNodeAfter(a, &a->allocatorRegion, NULL, NULL);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_node;}
    *allocator = a;
    BMMA_RangeAllocator_Verify(a,false);
    return BERR_SUCCESS;

err_node:
    BMMA_PoolAllocator_Destroy(a->blockAllocator);
err_pool:
    BKNI_Free(a);
err_alloc:
    return rc;
}


void BMMA_RangeAllocator_Destroy(BMMA_RangeAllocator_Handle a)
{
    BMMA_RangeAllocator_Block_Handle b;
    BDBG_OBJECT_ASSERT(a, BMMA_RangeAllocator);
    BMMA_RangeAllocator_Verify(a,false);
    if(a->settings.printLeakedRanges && a->status.allocatedBlocks) {
        BDBG_ERR(("Leaked resource -- %u unfreed ranges detected", a->status.allocatedBlocks));
        for(b=BLST_D_FIRST(&a->blocks);b!=NULL;b=BLST_D_NEXT(b, link)) {
            if(b->state.allocated && !b->state.hole) {
                BDBG_LOG(("%p %p:[%c:%u,%u]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", (void *)a, (void *)b, b->state.allocated?'A':'F', (unsigned)b->region.length, (unsigned)b->state.size, BDBG_UINT64_ARG(b->region.base), BDBG_UINT64_ARG(b->region.base+b->region.length)));
            }
        }
    }

    BMMA_PoolAllocator_Destroy(a->blockAllocator);
    BDBG_OBJECT_DESTROY(a, BMMA_RangeAllocator);
    BKNI_Free(a);
    return;
}

BMMA_DeviceOffset BMMA_RangeAllocator_GetAllocationBase_isrsafe(BMMA_RangeAllocator_Block_Handle b)
{
    BMMA_DeviceOffset addr;
    BDBG_OBJECT_ASSERT(b, BMMA_RangeAllocator_Block);
    BDBG_ASSERT(b->state.allocated);
    addr = b->region.base+b->state.header;
    BDBG_MSG_TRACE(("BMMA_RangeAllocator_GetAllocationBase:%p " BDBG_UINT64_FMT "->" BDBG_UINT64_FMT "", (void *)b, BDBG_UINT64_ARG(b->region.base), BDBG_UINT64_ARG(addr)));
    return addr;
}

size_t BMMA_RangeAllocator_GetAllocationSize_isrsafe(BMMA_RangeAllocator_Block_Handle b)
{
    BDBG_OBJECT_ASSERT(b, BMMA_RangeAllocator_Block);
    return b->state.size;
}

void *BMMA_RangeAllocator_GetAllocationHeader(BMMA_RangeAllocator_Block_Handle b)
{
    BDBG_OBJECT_ASSERT(b, BMMA_RangeAllocator_Block);
    return b->header;
}

typedef struct BMMA_P_RangeAllocator_Allocation {
    BMMA_RangeAllocator_Region block;
    BMMA_RangeAllocator_Region head;
    BMMA_RangeAllocator_Region tail;
    unsigned header;
} BMMA_P_RangeAllocator_Allocation;

bool BMMA_RangeAllocator_AllocateInRegion_InFront(const BMMA_RangeAllocator_Region *region, size_t size, const BMMA_RangeAllocator_BlockSettings *settings, BMMA_RangeAllocator_Region *allocation)
{
    BMMA_DeviceOffset base, block_end, allocation_end;
    base = BMMA_P_RangeAllocator_Align(region->base, settings->alignment);
    block_end = region->base + region->length;
    allocation_end = base + size;
    BDBG_MSG_TRACE(("BMMA_RangeAllocator_AllocateInRegion_InFront:%u(%u) %s " BDBG_UINT64_FMT "(%u) " BDBG_UINT64_FMT "(%u)", (unsigned)size, settings->alignment, block_end < allocation_end?"won't fit":"fit", BDBG_UINT64_ARG(region->base), (unsigned)region->length, BDBG_UINT64_ARG(base), (unsigned)size));
    if( block_end <  allocation_end ) { /* requested block wouldn't fit */
        return false;
    }
    if(settings->boundary) {
        if( (base/settings->boundary) != ((allocation_end - 1)/settings->boundary)) {
            BMMA_DeviceOffset boundary_base = BMMA_P_RangeAllocator_Align(base, settings->boundary);
            BDBG_MSG_TRACE(("BMMA_RangeAllocator_AllocateInRegion_InFront: boundary:%u %u(%u) %s " BDBG_UINT64_FMT "(%u) " BDBG_UINT64_FMT "(%u) (%#x/%#x)", settings->boundary, (unsigned)size, settings->alignment, block_end < allocation_end?"won't fit":"fit", BDBG_UINT64_ARG(region->base), (unsigned)region->length, BDBG_UINT64_ARG(base), (unsigned)size, (unsigned)((base)/settings->boundary), (unsigned)((allocation_end-1)/settings->boundary)));
            if(boundary_base == base) {
                return false;
            }
            base = boundary_base;
            allocation_end = base + size;
            BDBG_MSG_TRACE(("BMMA_RangeAllocator_AllocateInRegion_InFront: boundary:%u %u(%u) %s " BDBG_UINT64_FMT "(%u) " BDBG_UINT64_FMT "(%u) (%#x/%#x)", settings->boundary, (unsigned)size, settings->alignment, block_end < allocation_end?"won't fit":"fit", BDBG_UINT64_ARG(region->base), (unsigned)region->length, BDBG_UINT64_ARG(base), (unsigned)size, (unsigned)((base)/settings->boundary), (unsigned)((allocation_end-1)/settings->boundary)));
            if(allocation_end > block_end) { /* requested block wouldn't fit */
                return false;
            }
            if( (base/settings->boundary) != ((allocation_end - 1)/settings->boundary)) {
                BDBG_ASSERT(0);
                return false;
            }
        }
    }
    allocation->base = base;
    allocation->length = size;
    return true;
}

bool BMMA_RangeAllocator_AllocateInRegion_InBack(const BMMA_RangeAllocator_Region *region, size_t size, const BMMA_RangeAllocator_BlockSettings *settings, BMMA_RangeAllocator_Region *allocation)
{
    BMMA_DeviceOffset base, block_end, temp_base;
#if BDBG_DEBUG_BUILD
    BMMA_DeviceOffset allocation_end;
#endif
    block_end = region->base + region->length;
    if(block_end<size) { /* requested block wouldn't fit */
        return false;
    }
    temp_base = block_end - size;
    base = BMMA_P_RangeAllocator_Align(temp_base, settings->alignment);
    if(base > temp_base) {
        if(base < settings->alignment) { /* requested block wouldn't fit */
            return false;
        }
        base -= settings->alignment;
    }
#if BDBG_DEBUG_BUILD
    allocation_end = base + size;
#endif
    block_end = region->base + region->length;
    BDBG_MSG_TRACE(("BMMA_RangeAllocator_AllocateInRegion_InBack:%u(%u) %s " BDBG_UINT64_FMT "(%u) " BDBG_UINT64_FMT "(%u)", (unsigned)size, settings->alignment, block_end < allocation_end?"won't fit":"fit", BDBG_UINT64_ARG(region->base), (unsigned)region->length, BDBG_UINT64_ARG(base), (unsigned)size));
    if(base < region->base) { /* requested block wouldn't fit */
        return false;
    }
    allocation->base = base;
    allocation->length = size;
    return true;
}

static void BMMA_RangeAllocator_P_PlaceAllocation(const struct BMMA_RangeAllocator *a, const BMMA_RangeAllocator_Region *region, const BMMA_RangeAllocator_Region *block, const BMMA_RangeAllocator_BlockSettings *settings, BMMA_P_RangeAllocator_Allocation *allocation)
{
    BMMA_DeviceOffset block_end;
    BMMA_DeviceOffset allocation_end;
    BMMA_DeviceOffset base;
    unsigned hole_size;
    BMMA_Size size;

    allocation->block = *region;
    allocation->tail.length = allocation->head.length = 0;
    allocation->tail.base = allocation->head.base = 0;
    size = block->length;
    base = block->base;
    hole_size = base - allocation->block.base;
    if(hole_size >= a->settings.minAlignment) { /* need to allocate free block for hole due to alignment */
        BMMA_DeviceOffset hole_base;
        hole_base = allocation->block.base;
        allocation->block.base = hole_base + hole_size;
        allocation->block.length -= hole_size;
        allocation->head.base = hole_base;
        allocation->head.length = hole_size;
        BDBG_MSG_TRACE(("BMMA_RangeAllocator_AllocateInRegion:%p trimmed head " BDBG_UINT64_FMT "(%u)", (void *)a, BDBG_UINT64_ARG(hole_base), hole_size));
    }
    block_end = region->base + region->length;
    allocation_end = base + size;
    hole_size = block_end - allocation_end;
    if(hole_size >= a->settings.minAlignment) { /* need to allocate free block for hole at the end of allocation */
        allocation->block.length -= hole_size;
        allocation->tail.base = allocation_end;
        allocation->tail.length = hole_size;
        BDBG_MSG_TRACE(("BMMA_RangeAllocator_AllocateInRegion:%p trimmed tail " BDBG_UINT64_FMT "(%u)", (void *)a, BDBG_UINT64_ARG(allocation_end), hole_size));
    }
    allocation->header = base - allocation->block.base;
    BDBG_MSG_TRACE(("BMMA_RangeAllocator_AllocateInRegion:%p allocation " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " %u(%u) -> [head " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "(" BDBG_UINT64_FMT ") [" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " tail]", (void *)a, BDBG_UINT64_ARG(region->base), BDBG_UINT64_ARG(region->base+region->length), (unsigned)size, settings->alignment, BDBG_UINT64_ARG(allocation->head.base), BDBG_UINT64_ARG(allocation->head.base + allocation->head.length), BDBG_UINT64_ARG(allocation->block.base), BDBG_UINT64_ARG(allocation->block.base+allocation->block.length), BDBG_UINT64_ARG(allocation->block.base + allocation->header), BDBG_UINT64_ARG(allocation->tail.base), BDBG_UINT64_ARG(allocation->tail.base+allocation->tail.length)));
    BDBG_ASSERT(region->base == allocation->block.base - allocation->head.length);
    BDBG_ASSERT(region->length == allocation->head.length + allocation->block.length + allocation->tail.length);
    if((allocation->block.base + allocation->header) % settings->alignment != 0) {
        BDBG_ERR(("" BDBG_UINT64_FMT " = BMMA_RangeAllocator_Align(" BDBG_UINT64_FMT ", %u)", BDBG_UINT64_ARG(base),  BDBG_UINT64_ARG(region->base), (unsigned)settings->alignment));
        BDBG_ERR(("BMMA_P_RangeAllocator_AllocateInRegion:%p allocation " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " %u(%u) -> [head " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "(" BDBG_UINT64_FMT ") [" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " tail]", (void *)a, BDBG_UINT64_ARG(region->base), BDBG_UINT64_ARG(region->base+region->length), (unsigned)size, settings->alignment, BDBG_UINT64_ARG(allocation->head.base), BDBG_UINT64_ARG(allocation->head.base + allocation->head.length), BDBG_UINT64_ARG(allocation->block.base), BDBG_UINT64_ARG(allocation->block.base+allocation->block.length), BDBG_UINT64_ARG(allocation->block.base + allocation->header), BDBG_UINT64_ARG(allocation->tail.base), BDBG_UINT64_ARG(allocation->tail.base+allocation->tail.length)));
        BDBG_ASSERT(0);
    }
    return;
}

static bool BMMA_P_RangeAllocator_AllocateInRegion(const struct BMMA_RangeAllocator *a, const BMMA_RangeAllocator_Region *region, size_t size, const BMMA_RangeAllocator_BlockSettings *settings, BMMA_P_RangeAllocator_Allocation *allocation)
{
    BMMA_RangeAllocator_Region block;
    if(!BMMA_RangeAllocator_AllocateInRegion_InFront(region, size, settings, &block)) {
        return false;
    }
    BMMA_RangeAllocator_P_PlaceAllocation(a, region, &block, settings, allocation);
    return true;
}

#define BMMA_P_ALLOC_CUSTOM_MAX_BLOCKS  3
static BMMA_RangeAllocator_Block_Handle BMMA_RangeAllocator_P_AllocCustom(BMMA_RangeAllocator_Handle a, size_t size, const BMMA_RangeAllocator_BlockSettings *settings, BMMA_P_RangeAllocator_Allocation *allocation)
{
    BMMA_RangeAllocator_Block_Handle b;
    unsigned nFreeBlocks = 0;
    const BMMA_RangeAllocator_Region *freeRegions[BMMA_P_ALLOC_CUSTOM_MAX_BLOCKS];
    BMMA_RangeAllocator_Block_Handle freeBlocks[BMMA_P_ALLOC_CUSTOM_MAX_BLOCKS];
    BMMA_RangeAllocator_Region block;

    BDBG_OBJECT_ASSERT(a, BMMA_RangeAllocator);
    for(b=BLST_D_FIRST(&a->blocks);b!=NULL;b=BLST_D_NEXT(b, link)) {
        unsigned i;
        if(b->state.allocated || b->state.hole) {
            continue;
        }
        if(b->region.length < a->settings.allocatorMinFreeSize) {
            continue;
        }
        freeBlocks[nFreeBlocks] = b;
        freeRegions[nFreeBlocks] = &b->region;
        nFreeBlocks++;
        if(nFreeBlocks>=BMMA_P_ALLOC_CUSTOM_MAX_BLOCKS) {
            /* call custom allocator when all slots are filled */
            int allocated = a->settings.allocator(a->settings.context, freeRegions, nFreeBlocks, size, settings, &block);
            if(allocated>=0) {
                BDBG_ASSERT((unsigned)allocated < nFreeBlocks);
                b = freeBlocks[allocated];
                BMMA_RangeAllocator_P_PlaceAllocation(a, &b->region, &block, settings, allocation);
                return b;
            }
            BDBG_CASSERT(BMMA_P_ALLOC_CUSTOM_MAX_BLOCKS>0);
            for(i=0;i<BMMA_P_ALLOC_CUSTOM_MAX_BLOCKS-1;i++) {
                freeRegions[i] = freeRegions[i+1];
                freeBlocks[i] = freeBlocks[i+1];
            }
            nFreeBlocks = BMMA_P_ALLOC_CUSTOM_MAX_BLOCKS-1;
        }
    }
    while(nFreeBlocks>0) {
        unsigned i;
        /* call custom allocator when all blocks were visited */
        int allocated = a->settings.allocator(a->settings.context, freeRegions, nFreeBlocks, size, settings, &block);
        if(allocated>=0) {
            BDBG_ASSERT((unsigned)allocated < nFreeBlocks);
            b = freeBlocks[allocated];
            BMMA_RangeAllocator_P_PlaceAllocation(a, &b->region, &block, settings, allocation);
            return b;
        }
        nFreeBlocks--;
        for(i=0;i<nFreeBlocks;i++) {
            freeRegions[i] = freeRegions[i+1];
            freeBlocks[i] = freeBlocks[i+1];
        }
    }
    return NULL;
}

static BMMA_RangeAllocator_Block_Handle BMMA_RangeAllocator_P_AllocBestFit(BMMA_RangeAllocator_Handle a, size_t size, const BMMA_RangeAllocator_BlockSettings *settings, BMMA_P_RangeAllocator_Allocation *best_allocation)
{
    BMMA_RangeAllocator_Block_Handle b;
    BMMA_RangeAllocator_Block_Handle best_block=NULL;

    /* free list sorted list by the block size (largest block are first) and then by the block address (lower first) */
    for(b=BLST_AA_TREE_FIRST(BMMA_RangeAllocator_SizeTree, &a->size_tree); b!=NULL; b = BLST_AA_TREE_NEXT(BMMA_RangeAllocator_SizeTree, &a->size_tree,b)) {
        BMMA_P_RangeAllocator_Allocation allocation;
        BMMA_RangeAllocator_Region block;

        BDBG_MSG_TRACE(("BMMA_RangeAllocator_P_AllocBestFit:%p %u [%c]" BDBG_UINT64_FMT "(%u)", (void *)a, (unsigned)size, b->state.allocated?'A':'F', BDBG_UINT64_ARG(b->region.base), (unsigned)b->region.length));
        BDBG_ASSERT(!b->state.allocated);
        if(b->region.length < size) { /* terminate search if encountered free block with size less then requested */
            break;
        }
        if(!BMMA_RangeAllocator_AllocateInRegion_InFront(&b->region, size, settings, &block)) {
            continue;
        }
        BMMA_RangeAllocator_P_PlaceAllocation(a, &b->region, &block, settings, &allocation);
        BDBG_MSG_TRACE(("BMMA_RangeAllocator_P_AllocBestFit:%p candidate: %p [head " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "(" BDBG_UINT64_FMT ") [" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " tail] waste %u (old %p:%u)", (void *)a, (void *)b, BDBG_UINT64_ARG(allocation.head.base), BDBG_UINT64_ARG(allocation.head.base + allocation.head.length), BDBG_UINT64_ARG(allocation.block.base), BDBG_UINT64_ARG(allocation.block.base+allocation.block.length), BDBG_UINT64_ARG(allocation.block.base + allocation.header), BDBG_UINT64_ARG(allocation.tail.base), BDBG_UINT64_ARG(allocation.tail.base+allocation.tail.length), (unsigned)(allocation.tail.length+allocation.head.length), (void *)best_block, (unsigned)(best_block?(best_allocation->tail.length+best_allocation->head.length):0)));
        if(best_block == NULL || allocation.tail.length+allocation.head.length < best_allocation->tail.length+best_allocation->head.length) { /* pick block that creates less holes */
            best_block = b;
            *best_allocation = allocation;
            if(allocation.tail.length+allocation.head.length==0) {/* it can't get better */
                break;
            }
        }
    }
    return best_block;
}

static BERR_Code BMMA_RangeAllocator_P_SplitFreeBlock(BMMA_RangeAllocator_Handle a, BMMA_RangeAllocator_Block_Handle b, const BMMA_P_RangeAllocator_Allocation *allocation)
{
    BERR_Code rc;

    BLST_AA_TREE_REMOVE(BMMA_RangeAllocator_SizeTree, &a->size_tree, b);
    a->status.freeBlocks --;
    a->status.freeSpace -= b->region.length;

    b->state.allocated = true;
    b->region.base += allocation->head.length;
    b->region.length -= allocation->head.length;
    rc = BMMA_RangeAllocator_P_InsertFreeNodeAfter(a, &allocation->head, NULL, BLST_D_PREV(b, link));
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

    b->region.length -= allocation->tail.length;
    rc = BMMA_RangeAllocator_P_InsertFreeNodeAfter(a, &allocation->tail, NULL, b);
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}
    BDBG_ASSERT(b->region.length == allocation->block.length);
    BDBG_ASSERT(b->region.base == allocation->block.base);

    return BERR_SUCCESS;
}

BERR_Code BMMA_RangeAllocator_Alloc(BMMA_RangeAllocator_Handle a, BMMA_RangeAllocator_Block_Handle *block, size_t size, const BMMA_RangeAllocator_BlockSettings *settings)
{
    unsigned alignment;
    BERR_Code rc;
    BMMA_RangeAllocator_Block_Handle b;
    BMMA_P_RangeAllocator_Allocation allocation;
    BMMA_RangeAllocator_BlockSettings defaultSettings;
    BDBG_OBJECT_ASSERT(a, BMMA_RangeAllocator);

    alignment = a->settings.minAlignment;
    if(size==0) {
        size = 1;
    }
    if(settings) {
        defaultSettings = *settings;
        defaultSettings.alignment = alignment < settings->alignment ? settings->alignment : alignment;
        if(settings->boundary && size > settings->boundary) {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    } else {
        BMMA_RangeAllocator_GetDefaultAllocationSettings(&defaultSettings);
        defaultSettings.alignment = alignment;
    }
    settings = &defaultSettings;
    if(a->settings.allocator==NULL) {
        b = BMMA_RangeAllocator_P_AllocBestFit(a, size, settings, &allocation);
    } else {
        b = BMMA_RangeAllocator_P_AllocCustom(a, size, settings, &allocation);
    }

    if(b==NULL) {
        if(!a->settings.silent) {
            BDBG_ERR(("%p: Can't allocate block %u alignment %u (RANGE " BDBG_UINT64_FMT ":%u allocated:%u,%u free:%u,%u)", (void *)a, (unsigned)size, alignment, BDBG_UINT64_ARG(a->settings.base), (unsigned)a->settings.size, a->status.allocatedBlocks, (unsigned)a->status.allocatedSpace, a->status.freeBlocks, (unsigned)a->status.freeSpace));
            rc = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        } else {
            rc = BERR_OUT_OF_DEVICE_MEMORY;
        }
        if(a->settings.verbose) {
            BMMA_RangeAllocator_Verify(a, true);
        }
        goto done;
    }
    b->state.settings = *settings;
    b->state.size = size;
    b->state.header = allocation.header;
    a->status.allocatedBlocks ++;

    rc = BMMA_RangeAllocator_P_SplitFreeBlock(a, b, &allocation);
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

    *block = b;
    a->status.allocatedSpace += b->region.length;
    if(a->status.highWatermark < a->status.allocatedSpace) {
        a->status.highWatermark = a->status.allocatedSpace;
    }
    BDBG_MSG_TRACE(("BMMA_RangeAllocator_Alloc:%p %u allocated " BDBG_UINT64_FMT "(%u)", (void *)a, (unsigned)size, BDBG_UINT64_ARG(b->region.base), (unsigned)b->region.length));
    rc = BERR_SUCCESS;

done:
    BMMA_P_STATUS(a);
    return rc;
}

void BMMA_RangeAllocator_Free(BMMA_RangeAllocator_Handle a, BMMA_RangeAllocator_Block_Handle  b)
{
    BERR_Code rc;
    BMMA_RangeAllocator_Block_Handle  prev;

    BDBG_OBJECT_ASSERT(a, BMMA_RangeAllocator);
    BDBG_OBJECT_ASSERT(b, BMMA_RangeAllocator_Block);
    BDBG_MSG(("BMMA_RangeAllocator_Free:%p:%p " BDBG_UINT64_FMT ":%u", (void *)a, (void *)b, BDBG_UINT64_ARG(b->region.base), (unsigned)b->region.length));
    BDBG_ASSERT(b->state.allocated);
    BDBG_ASSERT(a->status.allocatedSpace >= b->region.length);
    a->status.allocatedBlocks --;
    a->status.allocatedSpace -= b->region.length;
    prev = BLST_D_PREV(b, link);
    BLST_D_REMOVE(&a->blocks, b, link);
    rc = BMMA_RangeAllocator_P_InsertFreeNodeAfter(a, &b->region, b, prev);
    if(rc!=BERR_SUCCESS) { (void)BERR_TRACE(rc);}
    BMMA_P_STATUS(a);
    return;
}

void BMMA_RangeAllocator_GetStatus(BMMA_RangeAllocator_Handle a, BMMA_RangeAllocator_Status *status)
{
    BMMA_RangeAllocator_Block_Handle b;

    BDBG_OBJECT_ASSERT(a, BMMA_RangeAllocator);
    BMMA_P_STATUS(a);
    *status = a->status;

    b=BLST_AA_TREE_FIRST(BMMA_RangeAllocator_SizeTree, &a->size_tree);
    if(b) {
        status->largestFree = b->region.length;
    }
    return;
}


/* this function would test whether blocks from first..last would fit into the base length, it returns number of bytes left, or 0 if blocks wouldn't */
static BMMA_Size BMMA_P_RangeAllocator_TestFit(BMMA_RangeAllocator_Handle a, BMMA_RangeAllocator_Block_Handle first, BMMA_RangeAllocator_Block_Handle last, const BMMA_RangeAllocator_Region *region)
{
    BMMA_RangeAllocator_Block_Handle b=first;
    BMMA_RangeAllocator_Region block = *region;
    for(;;) {
        BMMA_P_RangeAllocator_Allocation allocation;

        BDBG_OBJECT_ASSERT(b, BMMA_RangeAllocator_Block);
        BDBG_ASSERT(b->state.allocated);
        if(!BMMA_P_RangeAllocator_AllocateInRegion(a, &block, b->state.size, &b->state.settings, &allocation)) {
            return 0;
        }
        if(allocation.head.length!=0) {
            return 0;
        }
        block = allocation.tail;
        b = BLST_D_NEXT(b, link);
        BDBG_ASSERT(b);
        if(b==last) {
            BDBG_MSG_TRACE(("BMMA_P_RangeAllocator_TestFit:%p -> %u", (void *)a, (unsigned)allocation.tail.length));
            return allocation.tail.length;
        }
    }
}

static bool BMMA_P_RangeAllocator_TestRelocatable(BMMA_RangeAllocator_Handle a, BMMA_RangeAllocator_Block_Handle first, BMMA_RangeAllocator_Block_Handle last)
{
    if(a->settings.relocatable) {
        BMMA_RangeAllocator_Block_Handle b=first;
        for(;;) {
            BDBG_OBJECT_ASSERT(b, BMMA_RangeAllocator_Block);
            if(!a->settings.relocatable(a->settings.context, b->header)) {
                return false;
            }
            b = BLST_D_NEXT(b, link);
            BDBG_ASSERT(b);
            if(b==last) {
                break;
            }
        }
    }
    return true;
}

static BERR_Code BMMA_P_RangeAllocator_Relocate(BMMA_RangeAllocator_Handle a,  BMMA_RangeAllocator_Block_Handle first, BMMA_RangeAllocator_Block_Handle last, BMMA_RangeAllocator_CompactionStatus *status)
{
    BMMA_RangeAllocator_Block_Handle b;
    BMMA_RangeAllocator_Block_Handle free;

    BDBG_OBJECT_ASSERT(first, BMMA_RangeAllocator_Block);
    free = BLST_D_PREV(first, link);
    for(b=first;;) {
        BMMA_P_RangeAllocator_Allocation allocation;
        BMMA_RangeAllocator_Region block;
        bool allocated;

        BDBG_OBJECT_ASSERT(b, BMMA_RangeAllocator_Block);
        BDBG_OBJECT_ASSERT(free, BMMA_RangeAllocator_Block);

        BDBG_MSG_TRACE(("BMMA_P_RangeAllocator_Relocate:%p relocate [%c]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " [%c]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", (void *)a, free->state.allocated?'A':'F', BDBG_UINT64_ARG(free->region.base), BDBG_UINT64_ARG(free->region.base+free->region.length), b->state.allocated?'A':'F', BDBG_UINT64_ARG(b->region.base), BDBG_UINT64_ARG(b->region.base+b->region.length)));
        BDBG_ASSERT(!free->state.allocated);
        BDBG_ASSERT(free->region.base + free->region.length == b->region.base);
        block.base = free->region.base;
        block.length = free->region.length + b->region.length;
        allocated = BMMA_P_RangeAllocator_AllocateInRegion(a, &block, b->state.size, &b->state.settings, &allocation);
        BDBG_ASSERT(allocated);
        BDBG_ASSERT(allocation.head.length==0);
        /* swap b and free */
        BDBG_MSG_TRACE(("BMMA_P_RangeAllocator_Relocate:%p swapping %p and %p, [free]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " -> " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " [free]" BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", (void *)a, (void *)free, (void *)b, BDBG_UINT64_ARG(free->region.base),  BDBG_UINT64_ARG(free->region.base+free->region.length), BDBG_UINT64_ARG(b->region.base), BDBG_UINT64_ARG(b->region.base+b->region.length), BDBG_UINT64_ARG(allocation.block.base), BDBG_UINT64_ARG(allocation.block.base+allocation.block.length), BDBG_UINT64_ARG(allocation.tail.base), BDBG_UINT64_ARG(allocation.tail.base+allocation.tail.length)));
        if(a->settings.relocate) {
            BERR_Code rc = a->settings.relocate(a->settings.context, b->header, allocation.block.base + allocation.header, b->region.base + b->state.header,  b->state.size);
            if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
            status->bytesCopied += b->state.size;
        }
        a->status.allocatedSpace -= b->region.length;
        a->status.allocatedSpace += allocation.block.length;
        a->status.freeSpace -= free->region.length;
        a->status.freeSpace += allocation.tail.length;
        b->state.header = allocation.header;
        b->region = allocation.block;
        free->region = allocation.tail;
        BLST_D_REMOVE(&a->blocks, free, link);
        BLST_D_INSERT_AFTER(&a->blocks, b, free, link);
        b = BLST_D_NEXT(free, link);
        if(b==last) {
            break;
        }
        BMMA_P_STATUS(a);
        BSTD_UNUSED (allocated);
    }
    /* merge two free blocks */
    BDBG_OBJECT_ASSERT(b, BMMA_RangeAllocator_Block);
    BDBG_MSG_TRACE(("BMMA_P_RangeAllocator_Relocate:%p merging %p and %p, " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", (void *)a, (void *)free, (void *)b, BDBG_UINT64_ARG(free->region.base), BDBG_UINT64_ARG(free->region.base+free->region.length), BDBG_UINT64_ARG(b->region.base), BDBG_UINT64_ARG(b->region.base+b->region.length)));
    BDBG_ASSERT(!b->state.allocated);
    BDBG_ASSERT(free->region.base + free->region.length == b->region.base);
    BDBG_MSG_TRACE(("BMMA_P_RangeAllocator_Relocate:%p removing %p", (void *)a, (void *)free));
    BLST_AA_TREE_REMOVE(BMMA_RangeAllocator_SizeTree, &a->size_tree, free);
    BDBG_MSG_TRACE(("BMMA_P_RangeAllocator_Relocate:%p recycling %p", (void *)a, (void *)free));
    BLST_D_REMOVE(&a->blocks, free, link);
    b->region.length += free->region.length;
    b->region.base = free->region.base;
    a->status.freeBlocks --;
    BMMA_RangeAllocator_P_FreeNode(a, free);
    BMMA_P_STATUS(a);
    return BERR_SUCCESS;
}


BERR_Code BMMA_RangeAllocator_Compact(BMMA_RangeAllocator_Handle a, BMMA_RangeAllocator_CompactionStatus *status)
{
    BMMA_RangeAllocator_Block_Handle b;
    BMMA_RangeAllocator_Block_Handle prev=NULL;
    BERR_Code rc=BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(a, BMMA_RangeAllocator);
    BDBG_ASSERT(status);
    status->bytesCopied = 0;
    status->largestFreeBlock = 0;
    status->freeBlocks = a->status.freeBlocks;

    for(b=BLST_D_FIRST(&a->blocks);b!=NULL;b=BLST_D_NEXT(b, link)) {
        BMMA_RangeAllocator_Region block;
        BMMA_Size compacted;
        BMMA_RangeAllocator_Block_Handle first_allocated;
        bool relocated = false;

        BDBG_OBJECT_ASSERT(b, BMMA_RangeAllocator_Block);
        if(b->state.allocated) {
            continue;
        }
        if(prev==NULL) {
            if(status->largestFreeBlock < b->region.length) {
                status->largestFreeBlock = b->region.length;
            }
            prev = b;
            continue;
        }
        first_allocated = BLST_D_NEXT(prev, link);
        BDBG_OBJECT_ASSERT(first_allocated, BMMA_RangeAllocator_Block);
        BDBG_ASSERT(first_allocated->state.allocated);
        BDBG_ASSERT(prev->region.base < b->region.base);
        BDBG_ASSERT(prev->region.base < first_allocated->region.base);
        block.base = prev->region.base;
        block.length = b->region.base - prev->region.base;
        compacted = BMMA_P_RangeAllocator_TestFit(a, first_allocated, b, &block);
        if(compacted && BMMA_P_RangeAllocator_TestRelocatable(a, first_allocated, b)) {
            rc = BMMA_P_RangeAllocator_Relocate(a, first_allocated, b, status);
            if(rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);break;}
            status->freeBlocks -= 1;
            relocated = true;
        }
        prev = b;
        if(status->largestFreeBlock < b->region.length) {
            status->largestFreeBlock = b->region.length;
        }
        if(relocated) {
            BDBG_MSG(("BMMA_RangeAllocator_Compact:%p freeBlocks:%u largestFreeBlock:%u bytesCopied:%u", (void *)a, status->freeBlocks, (unsigned)status->largestFreeBlock, (unsigned)status->bytesCopied));
            if(a->settings.advance) {
                if(!a->settings.advance(a->settings.context, status)) {
                    break;
                }
            }
        }
    }
    BMMA_P_STATUS(a);
    return rc;
}


static bool BMMA_RangeAllocator_P_IsBoundary(BMMA_RangeAllocator_Block_Handle b)
{
    BMMA_RangeAllocator_Block_Handle prev, next;
    prev = BLST_D_PREV(b, link);
    next = BLST_D_NEXT(b, link);
    if(prev==NULL || next==NULL) {
        return true;
    }
    return (prev->state.allocated && prev->state.hole) || (next->state.allocated && next->state.hole);
}

BERR_Code BMMA_RangeAllocator_Iterate(BMMA_RangeAllocator_Handle a,  bool (*iterator)(void *, void *, const BMMA_RangeAllocator_Region *, bool), void *context, const BMMA_RangeAllocator_IteratorConfig *config)
{
    BMMA_RangeAllocator_Block_Handle b;
    BDBG_OBJECT_ASSERT(a, BMMA_RangeAllocator);
    if(config==NULL || !config->freeOnly) {
        if(config && config->order != BMMA_RangeAllocator_IteratorOrder_eUnordered) {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        for(b=BLST_D_FIRST(&a->blocks);b!=NULL;b=BLST_D_NEXT(b, link)) {
            if(b->state.hole) {
                continue;
            }
            if(!iterator(context, b->state.allocated ? b->header : NULL, &b->region, false)) {
                break;
            }
        }
    } else {
        switch(config->order) {
        case BMMA_RangeAllocator_IteratorOrder_eUnordered:
            for(b=BLST_D_FIRST(&a->blocks);b!=NULL;b=BLST_D_NEXT(b, link)) {
                if(b->state.allocated) {
                    continue;
                }
                if(!iterator(context, NULL, &b->region, BMMA_RangeAllocator_P_IsBoundary(b))) {
                    break;
                }
            }
            break;
        case BMMA_RangeAllocator_IteratorOrder_eLargestFirst:
            for(b=BLST_AA_TREE_FIRST(BMMA_RangeAllocator_SizeTree, &a->size_tree); b!=NULL; b = BLST_AA_TREE_NEXT(BMMA_RangeAllocator_SizeTree, &a->size_tree,b)) {
                if(!iterator(context, NULL, &b->region, BMMA_RangeAllocator_P_IsBoundary(b))) {
                    break;
                }
            }
            break;
        case BMMA_RangeAllocator_IteratorOrder_eSmallestFirst:
            for(b=BLST_AA_TREE_LAST(BMMA_RangeAllocator_SizeTree, &a->size_tree); b!=NULL; b = BLST_AA_TREE_PREV(BMMA_RangeAllocator_SizeTree, &a->size_tree,b)) {
                if(!iterator(context, NULL, &b->region, BMMA_RangeAllocator_P_IsBoundary(b))) {
                    break;
                }
            }
            break;
        }
    }
    return BERR_SUCCESS;
}

void BMMA_RangeAllocator_ResetHighWatermark(BMMA_RangeAllocator_Handle a)
{
    BDBG_OBJECT_ASSERT(a, BMMA_RangeAllocator);

    a->status.highWatermark = 0;
    return;
}

#define B_IS_INTERSECT(off1, len1, off2, len2) ((off1) <= ((off2)+(len2)-1) && (off2) <= ((off1)+(len1)-1))

BERR_Code BMMA_RangeAllocator_AddRegion(BMMA_RangeAllocator_Handle a, const BMMA_RangeAllocator_Region *region)
{
    BMMA_RangeAllocator_Block_Handle prev,b;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(a, BMMA_RangeAllocator);

    for(prev=NULL,b=BLST_D_FIRST(&a->blocks);b!=NULL;b=BLST_D_NEXT(b, link)) {
        if( B_IS_INTERSECT(b->region.base, b->region.length, region->base, region->length)) {
            if(!b->state.hole) {
                BDBG_ERR(("%p: region " BDBG_UINT64_FMT ":%u intersects with existing %p:" BDBG_UINT64_FMT "%u", (void *)a, BDBG_UINT64_ARG(region->base), (unsigned)region->length, (void *)b, BDBG_UINT64_ARG(b->region.base), (unsigned)b->region.length));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            } else if(b->region.base + b->region.length >= region->base + region->length) {
                break;
            }
        }
        if(b->region.base>region->base) {
            break;
        }
        prev = b;
    }
    if(B_IS_INTERSECT(a->allocatorRegion.base, a->allocatorRegion.length, region->base, region->length)) {
        /* if new region intersects with heap, it should be _within_ existing hole */
        if(b==NULL || !b->state.hole) {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        /* we need to adjust size of the hole */
        if(!B_IS_INTERSECT(b->region.base, b->region.length, region->base, region->length)) {
            BDBG_ASSERT(0);
        }
        BDBG_MSG(("AddRegion:%p: region " BDBG_UINT64_FMT ":%u intersects with existing hole %p:" BDBG_UINT64_FMT ":%u", (void *)a, BDBG_UINT64_ARG(region->base), (unsigned)region->length, (void *)b, BDBG_UINT64_ARG(b->region.base), (unsigned)b->region.length));
        if(b->region.base < region->base) {
            BMMA_Size new_hole_size = region->base - b->region.base;
            prev = b;
            b = NULL;
            if(prev->region.length >= new_hole_size + region->length) {  /* need to split hole in two */
                BMMA_RangeAllocator_Block_Handle newBlock;
                newBlock = BMMA_RangeAllocator_P_AllocateNode(a, region->base+region->length, prev->region.length - (new_hole_size + region->length), NULL);
                if(newBlock==NULL) {return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);}
                newBlock->state.allocated = true;
                newBlock->state.hole = true;
                BLST_D_INSERT_AFTER(&a->blocks, prev, newBlock, link);
            } else {
                /* just enough to adjust size of existing hole */
            }
            prev->region.length = new_hole_size;
        } else {
            BMMA_Size new_hole_size;
            if(b->region.base + b->region.length < region->base + region->length) {
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            new_hole_size = (b->region.base + b->region.length) - (region->base + region->length);
            if(new_hole_size!=0) {
                b->region.base = region->base + region->length;
                b->region.length = new_hole_size;
            } else {
                BLST_D_REMOVE(&a->blocks, b, link);
            }
            b=NULL;
        }
    } else {
        b=NULL;
        BDBG_MSG(("AddRegion:%p " BDBG_UINT64_FMT ":" BDBG_UINT64_FMT" " BDBG_UINT64_FMT ":" BDBG_UINT64_FMT, (void *)a, BDBG_UINT64_ARG(a->allocatorRegion.base), BDBG_UINT64_ARG(a->allocatorRegion.base+a->allocatorRegion.length), BDBG_UINT64_ARG(region->base), BDBG_UINT64_ARG(region->base + region->length)));
        if(region->base < a->allocatorRegion.base) {
            BMMA_Size new_hole_size;
            if(a->allocatorRegion.base < (region->base + region->length)) {
                BDBG_ASSERT(0);
            }
            new_hole_size = a->allocatorRegion.base - (region->base + region->length);
            BDBG_MSG(("AddRegion:%p: insert new hole %u at front", (void *)a, (unsigned)new_hole_size));
            if(new_hole_size) {
                BMMA_RangeAllocator_Block_Handle newBlock;
                newBlock = BMMA_RangeAllocator_P_AllocateNode(a, region->base+region->length, new_hole_size, NULL);
                if(newBlock==NULL) {return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);}
                newBlock->state.allocated = true;
                newBlock->state.hole = true;
                BDBG_ASSERT(prev==NULL);
                BLST_D_INSERT_HEAD(&a->blocks, newBlock, link);
            }
        } else {
            BMMA_Size new_hole_size;
            BMMA_RangeAllocator_Block_Handle newBlock;


            if(region->base < (a->allocatorRegion.base + a->allocatorRegion.length) ) {
                BDBG_ASSERT(0);
            }
            new_hole_size = region->base - (a->allocatorRegion.base + a->allocatorRegion.length);
            BDBG_MSG(("AddRegion:%p: insert new hole %u at back", (void *)a, (unsigned)new_hole_size));
            if(new_hole_size) {
                newBlock = BMMA_RangeAllocator_P_AllocateNode(a, a->allocatorRegion.base+a->allocatorRegion.length, new_hole_size, NULL);
                if(newBlock==NULL) {return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);}
                newBlock->state.allocated = true;
                newBlock->state.hole = true;
                BDBG_ASSERT(prev);
                BLST_D_INSERT_AFTER(&a->blocks, prev, newBlock, link);
                a->allocatorRegion.length += new_hole_size;
                prev = newBlock;
            }
        }
    }
    if(region->base < a->allocatorRegion.base) {
        a->allocatorRegion.length += a->allocatorRegion.base - region->base;
        a->allocatorRegion.base = region->base;
    }
    if(region->base + region->length > a->allocatorRegion.base + a->allocatorRegion.length) {
        a->allocatorRegion.length += (region->base + region->length) - (a->allocatorRegion.base + a->allocatorRegion.length);
    }
    a->status.dynamic = true;
    rc = BMMA_RangeAllocator_P_InsertFreeNodeAfter(a, region, b, prev);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
    a->settings.size += region->length;
    BMMA_RangeAllocator_Verify(a,false);
    return BERR_SUCCESS;
}

BERR_Code BMMA_RangeAllocator_RemoveRegion(BMMA_RangeAllocator_Handle a, const BMMA_RangeAllocator_Region *region)
{
    BMMA_RangeAllocator_Block_Handle b;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(a, BMMA_RangeAllocator);

    BDBG_MSG(("RemoveRegion:%p region " BDBG_UINT64_FMT ":%u:" BDBG_UINT64_FMT, (void *)a, BDBG_UINT64_ARG(region->base), (unsigned)region->length,BDBG_UINT64_ARG(region->base+region->length)));
    for(b=BLST_D_FIRST(&a->blocks);b!=NULL;b=BLST_D_NEXT(b, link)) {
        if( B_IS_INTERSECT(b->region.base, b->region.length, region->base, region->length)) {
            if(b->state.allocated) {
                BDBG_ERR(("%p: region " BDBG_UINT64_FMT ":%u still used by %p:" BDBG_UINT64_FMT "%u", (void *)a, BDBG_UINT64_ARG(region->base), (unsigned)region->length, (void *)b, BDBG_UINT64_ARG(b->region.base), (unsigned)b->region.length));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            } else { /* not allocated */
                if(b->region.base + b->region.length >= region->base + region->length) { /* region fully covered by the free region */
                    BMMA_P_RangeAllocator_Allocation hole;

                    a->status.dynamic = true;
                    BKNI_Memset(&hole, 0, sizeof(hole));
                    hole.block = *region;
                    hole.head.base = b->region.base;
                    hole.head.length = region->base - b->region.base;
                    hole.tail.base = region->base + region->length;
                    hole.tail.length = (b->region.base + b->region.length) - hole.tail.base;
                    rc = BMMA_RangeAllocator_P_SplitFreeBlock(a, b, &hole);
                    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}
                    /* now block is properly adjusted and marked as allocated, so it could be removed */
                    if( BLST_D_PREV(b,link)==NULL || BLST_D_NEXT(b,link)==NULL) {
                        /* only remove blocks at the boundaries of heap (to preserve continuity) */
                        if(BLST_D_PREV(b,link)==NULL) {
                            for(;;) {
                                BMMA_RangeAllocator_Block_Handle hole;

                                BDBG_MSG(("collaps head: %p " BDBG_UINT64_FMT ":" BDBG_UINT64_FMT" tail " BDBG_UINT64_FMT ":" BDBG_UINT64_FMT, (void *)a, BDBG_UINT64_ARG(a->allocatorRegion.base), BDBG_UINT64_ARG(a->allocatorRegion.base+a->allocatorRegion.length), BDBG_UINT64_ARG(b->region.base), BDBG_UINT64_ARG(b->region.base + b->region.length)));
                                BDBG_ASSERT(b->region.base == a->allocatorRegion.base);
                                a->allocatorRegion.length -= b->region.length;
                                a->allocatorRegion.base = b->region.base + b->region.length;
                                hole = BLST_D_NEXT(b,link);
                                BLST_D_REMOVE(&a->blocks, b, link);
                                BMMA_RangeAllocator_P_FreeNode(a, b);
                                if(hole && hole->state.hole) {
                                    b = hole;
                                } else {
                                    break;
                                }
                            }
                        } else if( BLST_D_NEXT(b,link)==NULL) {
                            for(;;) {
                                BMMA_RangeAllocator_Block_Handle hole;

                                BDBG_MSG(("collaps tail: %p " BDBG_UINT64_FMT ":" BDBG_UINT64_FMT" tail " BDBG_UINT64_FMT ":" BDBG_UINT64_FMT, (void *)a, BDBG_UINT64_ARG(a->allocatorRegion.base), BDBG_UINT64_ARG(a->allocatorRegion.base+a->allocatorRegion.length), BDBG_UINT64_ARG(b->region.base), BDBG_UINT64_ARG(b->region.base + b->region.length)));
                                BDBG_ASSERT(a->allocatorRegion.base + a->allocatorRegion.length == b->region.base + b->region.length);
                                a->allocatorRegion.length -= b->region.length;
                                hole = BLST_D_PREV(b, link);
                                BLST_D_REMOVE(&a->blocks, b, link);
                                BMMA_RangeAllocator_P_FreeNode(a, b);
                                if(hole && hole->state.hole) {
                                    b = hole;
                                } else {
                                    break;
                                }
                            }
                        }
                    } else {
                        BMMA_RangeAllocator_Block_Handle hole;
                        /* otherwise mark then as hole, and try to combine with other hole */
                        b->state.hole = true;
                        hole = BLST_D_PREV(b,link);
                        if(hole && hole->state.hole) {
                            BDBG_ASSERT(hole->region.base + hole->region.length == b->region.base);
                            b->region.base = hole->region.base;
                            b->region.length += hole->region.length;
                            BLST_D_REMOVE(&a->blocks, hole, link);
                            BMMA_RangeAllocator_P_FreeNode(a, hole);
                        }
                        hole = BLST_D_NEXT(b,link);
                        if(hole && hole->state.hole) {
                            BDBG_ASSERT(b->region.base + b->region.length == hole->region.base);
                            b->region.length += hole->region.length;
                            BLST_D_REMOVE(&a->blocks, hole, link);
                            BMMA_RangeAllocator_P_FreeNode(a, hole);
                        }
                    }
                    a->settings.size -= region->length;
                    BMMA_RangeAllocator_Verify(a,false);
                    return BERR_SUCCESS;
                }
            }
        }
        if(b->region.base>region->base) {
            break;
        }
    }
    BDBG_ERR(("%p: unknown region " BDBG_UINT64_FMT ":%u", (void *)a, BDBG_UINT64_ARG(region->base), (unsigned)region->length));
    return BERR_TRACE(BERR_INVALID_PARAMETER);
}

BERR_Code BMMA_RangeAllocator_CreateBlock(BMMA_RangeAllocator_Block_Handle *block, BMMA_DeviceOffset base, size_t size)
{
    BMMA_RangeAllocator_Block_Handle b;
    b = BKNI_Malloc(sizeof(*b));
    if(b==NULL) { return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);}
    BDBG_OBJECT_INIT(b, BMMA_RangeAllocator_Block);
    b->region.base = base;
    b->region.length = size;
    b->state.header = 0;
    b->state.size = size;
    b->state.allocated = true;
    *block = b;
    return BERR_SUCCESS;
}

void BMMA_RangeAllocator_DestroyBlock(BMMA_RangeAllocator_Block_Handle block)
{
    BDBG_OBJECT_DESTROY(block, BMMA_RangeAllocator_Block);
    BKNI_Free(block);
    return;
}
