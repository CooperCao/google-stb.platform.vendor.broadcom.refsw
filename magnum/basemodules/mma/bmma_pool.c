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
 ***************************************************************************/
#include "bstd.h"
#include "blst_list.h"
#include "bkni.h"
#include "bmma_pool.h"

BDBG_MODULE(BMMA_Pool);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */

typedef struct b_alloc_object {
    BLST_S_ENTRY(b_alloc_object) link;
    void *type;
    int instance;
} b_alloc_object;

typedef struct b_alloc_entry {
    BLST_D_ENTRY(b_alloc_entry) link;
} b_alloc_entry;

typedef struct BMMA_PoolAllocator_Bitmap {
    unsigned bitmap;
    void *block; /* data */
} BMMA_PoolAllocator_Bitmap;

typedef struct BMMA_PoolAllocator_Blocks {
    void *data; /* data */
    unsigned nelements;
    unsigned firstBitmap;
} BMMA_PoolAllocator_Blocks;

BDBG_OBJECT_ID(BMMA_PoolAllocator);

struct BMMA_PoolAllocator {
    BDBG_OBJECT(BMMA_PoolAllocator)
    BMMA_PoolAllocator_Bitmap *bitmaps;
    BMMA_PoolAllocator_Blocks *blocks;
    unsigned max_block_size; /* cap on number of allocations in single block */
    unsigned last_allocation; /* index of last element used for allocation */
    unsigned current_bitmap_count;
    unsigned previous_bitmap_count;
    unsigned next_block;
    unsigned current_block_count;
    unsigned previous_block_count;
    BMMA_PoolAllocator_CreateSettings settings;
#if BKNI_TRACK_MALLOCS
    const char *file;
    unsigned line;
#endif
};

void BMMA_PoolAllocator_GetDefaultCreateSettings(BMMA_PoolAllocator_CreateSettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
    settings->allocationSize = 0;
    settings->maxBlockSize = 16384;
    return;
}

#define B_POOL_BITS_IN_MAP (sizeof(unsigned)*8)

static BERR_Code
BMMA_PoolAllocator_P_GrowBlocks(BMMA_PoolAllocator_Handle a)
{
    BMMA_PoolAllocator_Blocks *entries;
    size_t count = a->current_block_count + a->previous_block_count; /* use Fibonacci sequence to grow pool size */
    size_t blockSize;
    unsigned i;

    count = a->current_block_count + a->previous_block_count; /* use Fibonacci sequence to grow pool size */
    BDBG_MSG(("BMMA_PoolAllocator_P_GrowBlocks:%p %u->%u", (void *)a, a->current_block_count, (unsigned)count));
    blockSize = count * sizeof(*entries);
#if BKNI_TRACK_MALLOCS
    entries = BKNI_Malloc_tagged(blockSize, a->file, a->line);
#else
    entries = BKNI_Malloc(blockSize);
#endif
    BDBG_MSG(("BMMA_PoolAllocator_P_GrowBlocks:%p bitmap:%u->%p", (void *)a, (unsigned)blockSize, (void *)entries));
    if(entries==NULL) {return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);}
    i=0;
    if(a->current_block_count) {
        BDBG_ASSERT(a->blocks);
        for(;i<a->current_block_count;i++) {
            entries[i] = a->blocks[i];
        }
        BKNI_Free(a->blocks);
    }
    for(;i<count;i++) {
        entries[i].data = 0;
        entries[i].nelements = 0;
        entries[i].firstBitmap = 0;
    }
    a->blocks = entries;
    a->previous_block_count = a->current_block_count ? a->current_block_count : 1;
    a->current_block_count = count;
    BDBG_ASSERT(a->next_block < count);
    return BERR_SUCCESS;
}

static BERR_Code
BMMA_PoolAllocator_P_GrowBitmap(BMMA_PoolAllocator_Handle a)
{
    BMMA_PoolAllocator_Bitmap *entries;
    void *data;
    unsigned i;
    size_t blockSize;
    BERR_Code rc;
    size_t count;
    unsigned nelements;

    if(a->next_block >= a->current_block_count) {
        BMMA_PoolAllocator_P_GrowBlocks(a);
    }

    count = a->current_bitmap_count + a->previous_bitmap_count;  /* use Fibonacci sequence to grow pool size */
    nelements = (count - a->current_bitmap_count) * B_POOL_BITS_IN_MAP;
    if(nelements>a->max_block_size) {
        nelements = a->max_block_size;
    }
    count = a->current_bitmap_count + nelements/B_POOL_BITS_IN_MAP;
    BDBG_MSG(("BMMA_PoolAllocator_P_GrowBitmap:%p %u->%u", (void *)a, a->current_bitmap_count, (unsigned)count));
    blockSize = count * sizeof(*entries);
#if BKNI_TRACK_MALLOCS
    entries = BKNI_Malloc_tagged(blockSize, a->file, a->line);
#else
    entries = BKNI_Malloc(blockSize);
#endif
    BDBG_MSG(("BMMA_PoolAllocator_P_GrowBitmap:%p bitmap:%u->%p", (void *)a, (unsigned)blockSize, (void *)entries));
    if(entries==NULL) {return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);}

    blockSize = nelements * a->settings.allocationSize;
#if BKNI_TRACK_MALLOCS
    data = BKNI_Malloc_tagged(blockSize, a->file, a->line);
#else
    data = BKNI_Malloc(blockSize);
#endif
    BDBG_MSG(("BMMA_PoolAllocator_P_GrowBitmap: block:%u->%p", (unsigned)blockSize, (void *)data));
    if(data==NULL) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);BKNI_Free(entries);return rc;}
    i=0;
    if(a->current_bitmap_count>0) {
        BDBG_ASSERT(a->bitmaps);
        for(;i<a->current_bitmap_count;i++) {
            entries[i] = a->bitmaps[i];
        }
        BKNI_Free(a->bitmaps);
    }
    a->blocks[a->next_block].data = data;
    a->blocks[a->next_block].firstBitmap = i;
    a->blocks[a->next_block].nelements = nelements;
    a->next_block++;
    for(;i<count;i++) {
        unsigned sequence = i-a->current_bitmap_count;
        entries[i].bitmap = 0;
        entries[i].block = (uint8_t*)data+sequence*(a->settings.allocationSize*B_POOL_BITS_IN_MAP);
        BDBG_MSG_TRACE(("BMMA_PoolAllocator_P_GrowBitmap:%p entries[%u] <- %#lx[%u]", a, i, entries[i].block, sequence));
    }
    a->previous_bitmap_count = a->current_bitmap_count ? a->current_bitmap_count : 1;
    a->current_bitmap_count = count;
    a->bitmaps = entries;
    return BERR_SUCCESS;
}

#if BKNI_TRACK_MALLOCS
BERR_Code BMMA_PoolAllocator_Create_tagged(BMMA_PoolAllocator_Handle *allocator, const BMMA_PoolAllocator_CreateSettings *settings, const char *file, unsigned line)
#else
BERR_Code BMMA_PoolAllocator_Create(BMMA_PoolAllocator_Handle *allocator, const BMMA_PoolAllocator_CreateSettings *settings)
#endif
{
    BERR_Code rc;
    BMMA_PoolAllocator_Handle a;
    unsigned maxBlockSize; /* block size in number of elements */

    BDBG_ASSERT(allocator);
    BDBG_ASSERT(settings);
    if(settings->allocationSize==0) { rc=BERR_TRACE(BERR_INVALID_PARAMETER); goto err_parameter; }
    maxBlockSize = settings->maxBlockSize/settings->allocationSize;

    /* maxBlockSize should be at least large enough to contain B_POOL_BITS_IN_MAP (32) individual allocations */
    if(maxBlockSize < B_POOL_BITS_IN_MAP) {rc=BERR_TRACE(BERR_INVALID_PARAMETER); goto err_parameter; }
    maxBlockSize = maxBlockSize - maxBlockSize%B_POOL_BITS_IN_MAP; /* round number of elements */
#if BKNI_TRACK_MALLOCS
    a = BKNI_Malloc_tagged(sizeof(*a), file, line);
#else
    a = BKNI_Malloc(sizeof(*a));
#endif
    if(a==NULL) { rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BDBG_OBJECT_INIT(a, BMMA_PoolAllocator);
    a->max_block_size = maxBlockSize;
    a->last_allocation = 0;
    a->current_bitmap_count = 0;
    a->previous_bitmap_count = 1;
    a->current_block_count = 0;
    a->previous_block_count = 1;
    a->next_block = 0;
    a->settings = *settings;
    a->bitmaps = NULL;
    a->blocks = NULL;
    *allocator = a;
#if BKNI_TRACK_MALLOCS
    a->file = file;
    a->line = line;
#endif

    return BERR_SUCCESS;

err_alloc:
err_parameter:
    return rc;
}

void
BMMA_PoolAllocator_Destroy(BMMA_PoolAllocator_Handle a)
{
    unsigned i;

    BDBG_OBJECT_ASSERT(a, BMMA_PoolAllocator);
    if(a->blocks) {
        for(i=0;i<a->next_block;i++) {
            BKNI_Free(a->blocks[i].data);
        }
        BKNI_Free(a->blocks);
    }
    if(a->bitmaps) {
        BKNI_Free(a->bitmaps);
    }
    BDBG_OBJECT_DESTROY(a, BMMA_PoolAllocator);
    BKNI_Free(a);
    return;
}

static void *
BMMA_PoolAllocator_P_Alloc(BMMA_PoolAllocator_Handle a, unsigned offset)
{
    unsigned i;
    BMMA_PoolAllocator_Bitmap *bitmap = a->bitmaps+offset;

    a->last_allocation = offset;
    for(i=0;i<B_POOL_BITS_IN_MAP;i++) {
        unsigned bit = 1<<i;
        if(0==(bitmap->bitmap&bit)) {
            bitmap->bitmap |= bit;
            return (uint8_t *)bitmap->block + i*a->settings.allocationSize;
        }
    }
    BDBG_ASSERT(0);
    return NULL;
}


void *BMMA_PoolAllocator_Alloc(BMMA_PoolAllocator_Handle a)
{
    unsigned i;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(a, BMMA_PoolAllocator);
    for(i=a->last_allocation;i<a->current_bitmap_count;i++) {
        if(a->bitmaps[i].bitmap != ~(0u)) {
            return BMMA_PoolAllocator_P_Alloc(a, i);
        }
    }
    for(i=0;i<a->last_allocation;i++) {
        if(a->bitmaps[i].bitmap != ~(0u)) {
            return BMMA_PoolAllocator_P_Alloc(a, i);
        }
    }
    i = a->current_bitmap_count;
    rc = BMMA_PoolAllocator_P_GrowBitmap(a);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); return NULL;}
    return BMMA_PoolAllocator_P_Alloc(a, i);
}


static bool BMMA_PoolAllocator_P_TryFree(BMMA_PoolAllocator_Handle a, void *block, unsigned index)
{
    BMMA_PoolAllocator_Bitmap *bitmap = a->bitmaps + index;

    BDBG_MSG_TRACE(("BMMA_PoolAllocator_P_TryFree:%p [%u] block:%lx %lx..%lx", a, index, (unsigned long)block, (unsigned long)bitmap->block, (unsigned long)(bitmap->block + a->settings.allocationSize * B_POOL_BITS_IN_MAP)));
    if( (uint8_t *)block >= (uint8_t *)bitmap->block && (uint8_t *)block < (uint8_t *)bitmap->block + a->settings.allocationSize * B_POOL_BITS_IN_MAP) {
        unsigned bitno = ((uint8_t *)block - (uint8_t *)bitmap->block) / a->settings.allocationSize;
        unsigned bit = 1<<bitno;
        if(bitmap->bitmap & bit && (uint8_t *)block == (uint8_t *)bitmap->block + (bitno * a->settings.allocationSize) ) {
            a->last_allocation = index;
            bitmap->bitmap ^= bit;
        } else {
            BDBG_ERR(("%p:block:%#lx not allocated", (void *)a, (unsigned long)block));
        }
        return true;
    }
    return false;
}

void
BMMA_PoolAllocator_Free(BMMA_PoolAllocator_Handle a, void *b)
{
    unsigned i;

    for(i=0;i<a->next_block;i++) {
        BMMA_PoolAllocator_Blocks *block = a->blocks + i;
        BDBG_MSG_TRACE(("BMMA_PoolAllocator_Free:%p [%u] block:%lx %lx..%lx", a, i, (unsigned long)b, (unsigned long)block->data, (unsigned long)(block->data + a->settings.allocationSize * block->nelements)));
        if( (uint8_t *)b >= (uint8_t *)block->data && (uint8_t *)b < (uint8_t *)block->data + a->settings.allocationSize * block->nelements) {
            unsigned index = block->firstBitmap + (((uint8_t *)b - (uint8_t *)block->data)/a->settings.allocationSize)/B_POOL_BITS_IN_MAP;
            if(!BMMA_PoolAllocator_P_TryFree(a, b, index)) {
                break;
            }
            return;
        }
    }
    BDBG_ERR(("BMMA_PoolAllocator_Free:%p unknown block %p", (void *)a, (void *)b));
    return;
}
