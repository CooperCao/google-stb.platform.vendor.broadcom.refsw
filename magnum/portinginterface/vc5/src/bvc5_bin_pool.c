/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
#include "bstd.h"
#include "bvc5.h"
#include "bvc5_bin_pool_priv.h"
#include "bvc5_priv.h"

BDBG_MODULE(BVC5_P);

#define BVC5_P_BINMEM_ALIGN            12 /* See GFXH-1179 */
#define BVC5_P_BINMEM_MIN_NUM_BLOCKS   2
#define BVC5_P_BINMEM_CHUNK_SIZE       16

/* The list of over-sized blocks */
typedef struct BVC5_P_BinPoolOversizedBlocks
{
   uint32_t                                  uiSize;
   BLST_Q_HEAD(sQueue, BVC5_P_BinPoolBlock)  sQueue;
} BVC5_P_BinPoolOversizedBlocks;

typedef struct BVC5_P_BinPool
{
   BMMA_Heap_Handle     hMMAHeap;
   uint32_t             uiSize;    /* Number of items in the main pool */
   uint32_t             uiMinSize;

   uint32_t             uiCapacity;
   BVC5_BinBlockHandle  *ppBlocks;

   BVC5_P_BinPoolOversizedBlocks sOversizedBlocks;
} BVC5_P_BinPool;

/***************************************************************************/

static uint32_t BVC5_P_BinBlockLockOffset(
   BVC5_BinBlockHandle  hBlock
   )
{
   BDBG_ASSERT(hBlock->uiLockOffset == 0);
   BDBG_ASSERT(hBlock->uiPhysOffset == 0);

   BVC5_P_BinPoolBlock_LockMem(hBlock->hBlock, &hBlock->uiLockOffset, &hBlock->uiPhysOffset);

   /*BKNI_Printf("BVC5_P_BinBlockLockOffset %p = %x\n", hBlock, hBlock->uiPhysOffset);*/
   return hBlock->uiPhysOffset;
}

/***************************************************************************/

uint32_t BVC5_P_BinBlockGetPhysical(
   BVC5_BinBlockHandle  hBlock
   )
{
   BDBG_ASSERT(hBlock->uiPhysOffset != 0);
   return hBlock->uiPhysOffset;
}

/***************************************************************************/
static void BVC5_P_BinBlockUnlockOffset(
   BVC5_BinBlockHandle  hBlock
   )
{
   /*BKNI_Printf("BVC5_P_BinBlockUnlockOffset %p (%x)\n", hBlock, hBlock->uiPhysOffset);*/

   BDBG_ASSERT(hBlock->uiLockOffset != 0);
   BDBG_ASSERT(hBlock->uiPhysOffset != 0);

   BVC5_P_BinPoolBlock_UnlockMem(hBlock->hBlock, hBlock->uiLockOffset);

   hBlock->uiLockOffset = 0;
   hBlock->uiPhysOffset = 0;
}

/***************************************************************************/
BERR_Code BVC5_P_BinPoolCreate(
   BMMA_Heap_Handle     hMMAHeap,
   BVC5_BinPoolHandle  *phBinPool
)
{
   BVC5_BinPoolHandle hBinPool = NULL;

   if (phBinPool == NULL)
      return BERR_INVALID_PARAMETER;

   hBinPool = (BVC5_BinPoolHandle)BKNI_Malloc(sizeof(BVC5_P_BinPool));
   if (hBinPool == NULL)
      goto exit;

   BKNI_Memset(hBinPool, 0, sizeof(BVC5_P_BinPool));

   hBinPool->ppBlocks = (BVC5_BinBlockHandle*)BKNI_Malloc(sizeof(BVC5_BinBlockHandle) * BVC5_P_BINMEM_CHUNK_SIZE);

   if (hBinPool->ppBlocks == NULL)
      goto exit1;

   hBinPool->hMMAHeap   = hMMAHeap;
   hBinPool->uiSize     = 0;
   hBinPool->uiMinSize  = 0;
   hBinPool->uiCapacity = BVC5_P_BINMEM_CHUNK_SIZE;

   BLST_Q_INIT(&hBinPool->sOversizedBlocks.sQueue);
   hBinPool->sOversizedBlocks.uiSize = 0;

   *phBinPool = hBinPool;

   return BERR_SUCCESS;

exit1:
   BVC5_P_BinPoolDestroy(hBinPool);

exit:
   return BERR_OUT_OF_SYSTEM_MEMORY;
}

/***************************************************************************/
static BVC5_BinBlockHandle BVC5_P_BinPoolBlockAlloc(
   BVC5_BinPoolHandle hBinPool,
   uint32_t           uiNumBytes
   )
{
   BVC5_BinBlockHandle     pData = (BVC5_BinBlockHandle)BKNI_Malloc(sizeof(BVC5_P_BinPoolBlock));
   if (pData == NULL)
      return NULL;

   /* BKNI_Printf("BlockAlloc\n"); */

   pData->hBlock = BVC5_P_BinPoolBlock_AllocMem(hBinPool->hMMAHeap, uiNumBytes, 1 << BVC5_P_BINMEM_ALIGN);
   pData->uiLockOffset = 0;
   pData->uiPhysOffset = 0;
   pData->uiNumBytes = uiNumBytes;

   if (pData->hBlock == NULL)
   {
      BKNI_Free(pData);
      pData = NULL;
   }

   return pData;
}

/***************************************************************************/
static void BVC5_P_BinPoolBlockFree(
   BVC5_BinBlockHandle  hBlock
   )
{
   /* BKNI_Printf("BlockFree\n"); */
   if (hBlock != NULL)
   {
      BDBG_ASSERT(hBlock->uiLockOffset == 0);
      BVC5_P_BinPoolBlock_FreeMem(hBlock->hBlock);
      BKNI_Free(hBlock);
   }
}

/***************************************************************************/
void BVC5_P_BinPoolDestroy(
   BVC5_BinPoolHandle hBinPool
)
{
   if (hBinPool != NULL)
   {
      /* Free all heap blocks */
      if (hBinPool->ppBlocks != NULL)
      {
         uint32_t i;
         for (i = 0; i < hBinPool->uiSize; i++)
            BVC5_P_BinPoolBlockFree(hBinPool->ppBlocks[i]);

         BKNI_Free(hBinPool->ppBlocks);
      }

      /* Free any over-sized blocks still remaining */
      if (!BLST_Q_EMPTY(&hBinPool->sOversizedBlocks.sQueue))
      {
         BVC5_BinBlockHandle pBlock = BLST_Q_FIRST(&hBinPool->sOversizedBlocks.sQueue);
         while (pBlock != NULL)
         {
            BVC5_BinBlockHandle pNext = BLST_Q_NEXT(pBlock, sChain);

            BVC5_P_BinBlockUnlockOffset(pBlock);
            BVC5_P_BinPoolBlockFree(pBlock);

            pBlock = pNext;
         }

         BLST_Q_INIT(&hBinPool->sOversizedBlocks.sQueue);
         hBinPool->sOversizedBlocks.uiSize = 0;
      }

      BKNI_Free(hBinPool);
   }
}

/***************************************************************************/

static BVC5_BinBlockHandle BVC5_P_BinPoolAlloc(
   BVC5_BinPoolHandle hBinPool,
   uint32_t           *uiPhysOffset
)
{
   BVC5_BinBlockHandle  pBlock;

   if (hBinPool == NULL)
      return NULL;

   if (hBinPool->uiSize == 0)
   {
      /* Somehow the pool has not been replenished.  Try to get some memory now */
      pBlock = BVC5_P_BinPoolBlockAlloc(hBinPool, BVC5_BIN_MEM_MIN_POOL_BYTES);
   }
   else
   {
      /* Otherwise recycle a previously allocated block */
      hBinPool->uiSize--;

      pBlock = hBinPool->ppBlocks[hBinPool->uiSize];
   }

   if (hBinPool->uiSize < hBinPool->uiMinSize)
      hBinPool->uiMinSize = hBinPool->uiSize;

   *uiPhysOffset = 0;

   if (pBlock != NULL)
      *uiPhysOffset = BVC5_P_BinBlockLockOffset(pBlock);

   return pBlock;
}

/***************************************************************************/

BVC5_BinBlockHandle BVC5_P_BinPoolAllocAtLeast(
   BVC5_BinPoolHandle hBinPool,
   uint32_t           uiMinBytes,
   uint32_t           *uiPhysOffset
   )
{
   BVC5_BinBlockHandle pBlock;

   if (hBinPool == NULL)
      return NULL;

   /* uiMinBytes must be a multiple of 4K, so round up if necessary */
   uiMinBytes = (uiMinBytes + 4095) & ~4095;

   /* Make enough room for the minimum, plus some actual memory for bin lists.
    * We'll use double the minimum size for the initial block, unless the block would get huge.
    * This will cover most real use cases, but not huge resolutions with a lot of MRTs. Those
    * will just get the minimum and then use more overspill blocks. */
   if (uiMinBytes < 2 * 1024 * 1024)
      uiMinBytes += uiMinBytes;

   *uiPhysOffset = 0;

   /* If a regular pool block is big enough, just use one */
   if (uiMinBytes <= BVC5_BIN_MEM_MIN_POOL_BYTES)
      return BVC5_P_BinPoolAlloc(hBinPool, uiPhysOffset);

   /* Otherwise, allocate a new block and put in the over-sized list */
   pBlock = BVC5_P_BinPoolBlockAlloc(hBinPool, uiMinBytes);
   if (pBlock != NULL)
   {
      BLST_Q_INSERT_TAIL(&hBinPool->sOversizedBlocks.sQueue, pBlock, sChain);
      hBinPool->sOversizedBlocks.uiSize += pBlock->uiNumBytes;
      *uiPhysOffset = BVC5_P_BinBlockLockOffset(pBlock);
   }

   return pBlock;
}

/***************************************************************************/
BERR_Code BVC5_P_BinPoolRecycle(
   BVC5_BinPoolHandle   hBinPool,
   BVC5_BinBlockHandle  pBlock
)
{
   if (hBinPool == NULL || pBlock == NULL)
      return BERR_INVALID_PARAMETER;

   if (pBlock->uiNumBytes > BVC5_BIN_MEM_MIN_POOL_BYTES)
   {
      /* This is an over-sized block, delete it and remove from the list */
      BLST_Q_REMOVE(&hBinPool->sOversizedBlocks.sQueue, pBlock, sChain);
      hBinPool->sOversizedBlocks.uiSize -= pBlock->uiNumBytes;

      BVC5_P_BinBlockUnlockOffset(pBlock);
      BVC5_P_BinPoolBlockFree(pBlock);
      return BERR_SUCCESS;
   }

   if (hBinPool->uiSize == hBinPool->uiCapacity)
   {
      uint32_t    uiNewCapacity = hBinPool->uiCapacity + BVC5_P_BINMEM_CHUNK_SIZE;
      uint32_t    uiNewBytes    = uiNewCapacity * sizeof(void *);
      uint32_t    uiOldBytes    = hBinPool->uiCapacity * sizeof(void *);
      BVC5_BinBlockHandle *ppNewBlocks = (BVC5_BinBlockHandle*)BKNI_Malloc(uiNewBytes);

      /* BKNI_Printf("Expanding from %d to %d\n", hBinPool->uiCapacity, uiNewCapacity); */

      if (ppNewBlocks == NULL)
         return BERR_OUT_OF_SYSTEM_MEMORY;

      BKNI_Memset(ppNewBlocks, 0, uiNewBytes);
      BKNI_Memcpy(ppNewBlocks, hBinPool->ppBlocks, uiOldBytes);

      hBinPool->uiCapacity = uiNewCapacity;

      BKNI_Free(hBinPool->ppBlocks);
      hBinPool->ppBlocks = ppNewBlocks;
   }

   hBinPool->ppBlocks[hBinPool->uiSize] = pBlock;
   hBinPool->uiSize++;

   BVC5_P_BinBlockUnlockOffset(pBlock);

   return BERR_SUCCESS;
}

/***************************************************************************/
bool BVC5_P_BinPoolReplenish(
   BVC5_BinPoolHandle   hBinPool
)
{
   uint32_t uiNeeds;
   uint32_t i;

   if (hBinPool == NULL)
      return false;

   if (hBinPool->uiSize >= BVC5_P_BINMEM_MIN_NUM_BLOCKS)
      return true;

   uiNeeds = BVC5_P_BINMEM_MIN_NUM_BLOCKS - hBinPool->uiSize;

   for (i = 0; i < uiNeeds; ++i)
   {
      BVC5_BinBlockHandle pBlock = BVC5_P_BinPoolBlockAlloc(hBinPool, BVC5_BIN_MEM_MIN_POOL_BYTES);
      if (pBlock != NULL)
      {
         BVC5_P_BinBlockLockOffset(pBlock);
         BVC5_P_BinPoolRecycle(hBinPool, pBlock);
      }
   }

   hBinPool->uiMinSize = hBinPool->uiSize;

   return hBinPool->uiSize >= BVC5_P_BINMEM_MIN_NUM_BLOCKS;
}

/***************************************************************************/

void BVC5_P_BinPoolPurge(
   BVC5_BinPoolHandle   hBinPool
)
{
   if (hBinPool == NULL)
      return;

   /* BKNI_Printf("Purge min: %d, size: %d\n", hBinPool->uiMinSize, hBinPool->uiSize); */
   if (hBinPool->uiMinSize > BVC5_P_BINMEM_MIN_NUM_BLOCKS &&
       hBinPool->uiSize    > BVC5_P_BINMEM_MIN_NUM_BLOCKS)
   {
      uint32_t uiNumToPurge = hBinPool->uiMinSize - BVC5_P_BINMEM_MIN_NUM_BLOCKS;
      uint32_t i;

      /*BKNI_Printf("Purging %d\n", uiNumToPurge);*/

      for (i = 0; i < uiNumToPurge; ++i)
      {
         uint32_t uiPhys;
         BVC5_BinBlockHandle pBlock = BVC5_P_BinPoolAlloc(hBinPool, &uiPhys);
         BVC5_P_BinBlockUnlockOffset(pBlock);
         BVC5_P_BinPoolBlockFree(pBlock);
      }
   }

   /* Reset min size to current size for next sample period */
   hBinPool->uiMinSize = hBinPool->uiSize;
}

/***************************************************************************/
void BVC5_P_BinPoolStats(
   BVC5_BinPoolHandle hBinPool,
   size_t             *puiCapacityBytes,
   size_t             *puiUsedBytes
   )
{
   *puiCapacityBytes = hBinPool->uiCapacity * BVC5_BIN_MEM_MIN_POOL_BYTES;
   *puiUsedBytes     = hBinPool->uiSize * BVC5_BIN_MEM_MIN_POOL_BYTES;

   *puiCapacityBytes += hBinPool->sOversizedBlocks.uiSize;
   *puiUsedBytes     += hBinPool->sOversizedBlocks.uiSize;
}
