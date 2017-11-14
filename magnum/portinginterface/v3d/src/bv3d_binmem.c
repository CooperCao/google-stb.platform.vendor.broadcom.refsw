/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 **************************************************************************/
#include "bv3d.h"
#include "bv3d_binmem_priv.h"
#include "bkni.h"

BDBG_MODULE(BV3D_P);

/* Bin Memory
 *
 * The binner memory is allocated as a single contiguous pool which is subdivided into chunks.
 * Each chunk is big enough to supply enough memory for a multisample 1080p display.
 *
 * A chunk is free if its client id is zero and it is not allocated for overspill.
 * The overspill chunk is used initially to remove an out-of-memory condition and is then used
 * to satisfy binner out-of-memory demands.
 *
 * Chunks are referenced by handles.  Handles are indices into the pool (with some bits masked in
 * for safety).
 */

/* Records the ownership of a chunk
 */
typedef struct BV3D_P_BinMemInfo
{
   uint32_t uiClient;                     /* 0 => unused            */
   bool     bInUse;                       /* true = chunk is in use */
   BV3D_Job *psJob;                       /* 0 => none              */
} BV3D_P_BinMemInfo;

/* Bin memory manager
 *
 * Holds the memory pool and ownership info
 */
typedef struct BV3D_P_BinMemManager
{
   /* Allocation */
   BMMA_Heap_Handle  hMma;                /* Handle to the heap for destructor               */
   BMMA_Block_Handle hBlock;              /* Allocated block which contains all the bin mem  */
   uint32_t          pPool;               /* The pool of bin memory shared between clients   */
   uint32_t          uiNumChunks;         /* Pool size in bytes                              */
   uint32_t          uiChunkSize;         /* Size of each chunk                              */
   BV3D_P_BinMemInfo *pInfo;              /* Record of pool usage                            */
   uint32_t          uiOffset;            /* Counter used to round robin the chunks          */
   uint32_t          uiFreeChunks;        /* Number of free chunks                           */

   /* The overspill chunk is a special case in the allocator */
   BV3D_BinMemHandle hOverspill;          /* Next available chunk for binner                 */

} BV3D_P_BinMemManager;

/***************************************************************************/

static bool BV3D_P_BinMemIsSuitablePool(
   uint32_t uiPool,
   uint32_t uiSize)
{
   uint32_t uiStart = uiPool;
   uint32_t uiEnd   = uiPool + uiSize - 1;

   return (uiStart & BV3D_BIN_MEM_BOUND_BITS) == (uiEnd & BV3D_BIN_MEM_BOUND_BITS);
}

/***************************************************************************/

/* Chunks must be at least 80 bytes (TILE_STATE_SIZE + CL_BLOCK_SIZE_MIN) per tile
 * Number of tiles = 1920 (max res) * 1080 (max res) * 4 (multisample) / 64 / 64 = 158k
 * Rounding off this gives 256K, so make chunks a multiple (default 1) of 256k
 */
static bool BV3D_P_BinMemAllocatePool(
   BV3D_BinMemManagerHandle hBinMemManager,
   uint32_t                 uiMemMegs,
   uint32_t                 uiChunkPow)
{
   BMMA_Block_Handle          hBlock;
   BMMA_AllocationSettings    allocationSettings;
   bool     ok                = false;
   uint32_t uiMemSize         = uiMemMegs * 1024 * 1024;
   uint32_t uiChunkSize       = (1 << uiChunkPow) * 256 * 1024;
   uint32_t uiNumChunks       = uiMemSize / uiChunkSize;

   hBinMemManager->uiNumChunks = 0;
   hBinMemManager->uiChunkSize = 0;

   if (uiNumChunks < 8)
      return false;

   BMMA_GetDefaultAllocationSettings(&allocationSettings);
   allocationSettings.boundary = 1 << BV3D_BIN_MEM_BOUND_POW2;
   hBlock = BMMA_Alloc(hBinMemManager->hMma, uiMemSize, 1 << BV3D_BIN_MEM_ALIGN_POW2, &allocationSettings);
   if (hBlock)
   {
      hBinMemManager->pPool = BMMA_LockOffset(hBlock);
      ok = BV3D_P_BinMemIsSuitablePool(hBinMemManager->pPool, uiMemSize);
      if (!ok)
      {
         BDBG_ERR(("BMMA_Alloc did not honor the boundary param"));
         BMMA_Free(hBlock);
      }
   }

   if (ok)
   {
      hBinMemManager->hBlock = hBlock;
      hBinMemManager->uiNumChunks = uiNumChunks;
      hBinMemManager->uiFreeChunks = uiNumChunks;
      hBinMemManager->uiChunkSize = uiChunkSize;
   }

   return ok;
}

/***************************************************************************/

/* Constructor for bin memory manager
 */
BERR_Code BV3D_P_BinMemCreate(
   BV3D_BinMemManagerHandle   *phObject,
   BMMA_Heap_Handle           hMma,
   uint32_t                   uiMemMegs,
   uint32_t                   uiChunkPow)
{
   BV3D_BinMemManagerHandle   hBinMemManager = (BV3D_BinMemManagerHandle)BKNI_Malloc(sizeof(BV3D_P_BinMemManager));

   if (hBinMemManager == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   /* Clear everything */
   hBinMemManager->hMma       = hMma;
   hBinMemManager->hBlock     = NULL;
   hBinMemManager->pPool      = 0;
   hBinMemManager->pInfo      = NULL;
   hBinMemManager->uiOffset   = 0;

   hBinMemManager->hOverspill = NULL;

   /* Allocate binner memory up front to maximise chances it will fit in the same 256Mbyte block */
   if (!BV3D_P_BinMemAllocatePool(hBinMemManager, uiMemMegs, uiChunkPow))
      goto error;

   /* Allocate info array */
   hBinMemManager->pInfo = (BV3D_P_BinMemInfo *)BKNI_Malloc(hBinMemManager->uiNumChunks * sizeof(BV3D_P_BinMemInfo));

   if (hBinMemManager->pInfo == NULL)
      goto error;

   /* Memset will also set bInUse to false at the same time */
   BKNI_Memset(hBinMemManager->pInfo, 0, hBinMemManager->uiNumChunks * sizeof(BV3D_P_BinMemInfo));

   /* Allocate the first overspill block */
   hBinMemManager->hOverspill = BV3D_P_BinMemAlloc(hBinMemManager, 0, NULL);

   *phObject = hBinMemManager;

   return BERR_SUCCESS;

error:
   *phObject = NULL;
   BV3D_P_BinMemDestroy(hBinMemManager);
   return BERR_OUT_OF_SYSTEM_MEMORY;
}

/***************************************************************************/

/* Destructor for bin memory manager
 */
void BV3D_P_BinMemDestroy(
   BV3D_BinMemManagerHandle hBinMemManager)
{
   if (hBinMemManager != NULL)
   {
      if (hBinMemManager->pInfo != NULL)
         BKNI_Free(hBinMemManager->pInfo);

      if (hBinMemManager->hBlock)
      {
         BMMA_UnlockOffset(hBinMemManager->hBlock, hBinMemManager->pPool);
         BMMA_Free(hBinMemManager->hBlock);
      }

      if (hBinMemManager != NULL)
         BKNI_Free(hBinMemManager);
   }
}

/***************************************************************************/

/* Allocate memory for binning and attach to the allocating client
 *
 * The overspill chunk is a special case as it does not belong to a client.
 * It is skipped during the search for a free chunk, and is not attached
 * when allocated.
 *
 * Chunks are allocated in a round-robin fashion to speed up the search for
 * a free chunk.
 *
 */
BV3D_BinMemHandle BV3D_P_BinMemAlloc(
   BV3D_BinMemManagerHandle   hBinMemManager,
   uint32_t                   uiClient,
   uint32_t                   *puiSize)
{
   uint32_t             i;
   BV3D_BinMemHandle    chunk = NULL;

   if (puiSize != NULL)
      *puiSize = 0;

   /* look through the list whilst there are some available and we haven't found a suitable
      candidate */
   for (i = 0; i < hBinMemManager->uiNumChunks && hBinMemManager->uiFreeChunks > 0 && chunk == NULL; ++i)
   {
      uint32_t          indx  = (i + hBinMemManager->uiOffset) % hBinMemManager->uiNumChunks;
      BV3D_BinMemHandle hAddr = BV3D_P_BinMemIndexToHandle(hBinMemManager, indx);

      /* Search for a free chunk */
      if (!hBinMemManager->pInfo[indx].bInUse)
      {
         /* Chunk found: use this one and mark as allocated */
         chunk = hAddr;

         hBinMemManager->pInfo[indx].psJob = NULL;
         hBinMemManager->pInfo[indx].uiClient = uiClient;
         hBinMemManager->pInfo[indx].bInUse = true;

         /* remove one of the free chunks */
         hBinMemManager->uiFreeChunks -= 1;

         /* Move on to next chunk */
         hBinMemManager->uiOffset += 1;
         if (hBinMemManager->uiOffset == hBinMemManager->uiNumChunks)
            hBinMemManager->uiOffset = 0;
      }
   }

   if (chunk != NULL && puiSize != NULL)
      *puiSize = hBinMemManager->uiChunkSize;

   BDBG_MSG(("BV3D_P_BinMemAlloc returning %p", chunk));

   return chunk;
}

/***************************************************************************/

/* Mark the chunk as being used by a client+job
 *
 * Use to attach a previously unattached overspill block.
 */
void BV3D_P_BinMemAttach(
   BV3D_BinMemManagerHandle   hBinMemManager,
   BV3D_BinMemHandle          hMem,
   BV3D_Job                   *psJob)
{
   uint32_t indx = BV3D_P_BinMemHandleToIndex(hBinMemManager, hMem);

   hBinMemManager->pInfo[indx].uiClient = psJob->uiClientId;
   hBinMemManager->pInfo[indx].psJob    = psJob;
}

/***************************************************************************/

/* Mark the chunk (which previously only belonged to a client) as being used
 * by this job.
 *
 * Use when a chunk is sent with a job to attach it to the job.  The chunk can
 * then be unattached when the job completes.
 *
 */
void BV3D_P_BinMemAttachToJob(
   BV3D_BinMemManagerHandle   hBinMemManager,
   BV3D_BinMemHandle          hMem,
   BV3D_Job                   *psJob)
{
   uint32_t indx = BV3D_P_BinMemHandleToIndex(hBinMemManager, hMem);

   BDBG_ASSERT(hBinMemManager->pInfo[indx].uiClient == psJob->uiClientId);

   hBinMemManager->pInfo[indx].psJob = psJob;
}

/***************************************************************************/

/* Release all the chunks associated with a job
 *
 * If the attachment count falls to zero, the chunk becomes free for
 * allocation again.
 */
void BV3D_P_BinMemReleaseByJob(
   BV3D_BinMemManagerHandle   hBinMemManager,
   BV3D_Job                   *psJob)
{
   uint32_t i;

   for (i = 0; i < hBinMemManager->uiNumChunks; ++i)
   {
      BV3D_P_BinMemInfo *psInfo = &hBinMemManager->pInfo[i];

      if (psInfo->psJob == psJob)
      {
         BDBG_MSG(("BV3D_P_BinMemReleaseByJob %p", BV3D_P_BinMemIndexToHandle(hBinMemManager, i)));

         hBinMemManager->uiFreeChunks += 1;
         psInfo->bInUse   = false;
         psInfo->uiClient = 0;
         psInfo->psJob    = NULL;
      }
   }
}

/***************************************************************************/

/* Release all the memory chunks associated with a client ID
 *
 * If the attachment count falls to zero, the chunk becomes free for
 * allocation again.
 */
void BV3D_P_BinMemReleaseByClient(
   BV3D_BinMemManagerHandle   hBinMemManager,
   uint32_t                   uiClient)
{
   uint32_t i;

   for (i = 0; i < hBinMemManager->uiNumChunks; ++i)
   {
      BV3D_P_BinMemInfo *psInfo = &hBinMemManager->pInfo[i];

      if (psInfo->uiClient == uiClient)
      {
         hBinMemManager->uiFreeChunks += 1;
         psInfo->bInUse   = false;
         psInfo->uiClient = 0;
         psInfo->psJob    = NULL;
      }
   }
}

/***************************************************************************/

/* Overspill has been used, so attach it to the job.
 * Some of the overspill memory may have been used
 */
void BV3D_P_BinMemOverspillUsed(
   BV3D_BinMemManagerHandle   hBinMemManager,
   BV3D_Job                   *psJob)
{
   /* The overspill was used, so this job needs to be recorded as the owner */
   BV3D_P_BinMemAttach(hBinMemManager, hBinMemManager->hOverspill, psJob);
   hBinMemManager->hOverspill = NULL;
}

/* Allocate the overspill memory */
bool BV3D_P_BinMemAllocOverspill(
   BV3D_BinMemManagerHandle   hBinMemManager)
{
   BDBG_ASSERT(hBinMemManager->hOverspill == NULL);

   hBinMemManager->hOverspill = BV3D_P_BinMemAlloc(hBinMemManager, 0, NULL);

   return hBinMemManager->hOverspill != NULL;
}

/***************************************************************************/

/* Return the current overspill pointer */
uint32_t BV3D_P_BinMemGetOverspill(
   BV3D_BinMemManagerHandle  hBinMemManager)
{
   return (uint32_t)hBinMemManager->hOverspill;
}

/***************************************************************************/

uint32_t BV3D_P_BinMemHandleToIndex(
   BV3D_BinMemManagerHandle hBinMemManager,
   BV3D_BinMemHandle        hMem)
{
   uint32_t diff = (uint32_t)hMem - hBinMemManager->pPool;
   uint32_t indx = diff / hBinMemManager->uiChunkSize;

   if (hMem == NULL)
      return ~0u;

   return indx;
}

/***************************************************************************/

BV3D_BinMemHandle BV3D_P_BinMemIndexToHandle(
   BV3D_BinMemManagerHandle hBinMemManager,
   uint32_t uiIndx)
{
   if (uiIndx == ~0u)
      return NULL;

   return (BV3D_BinMemHandle)(hBinMemManager->pPool + uiIndx * hBinMemManager->uiChunkSize);
}

/***************************************************************************/

void BV3D_P_BinMemDebugDump(BV3D_BinMemManagerHandle hBinMemManager)
{
   uint32_t i;

   BKNI_Printf("Bin Memory Manager:\n");
   BKNI_Printf(" NumChunks  = %d\n", hBinMemManager->uiNumChunks);
   BKNI_Printf(" FreeChunks = %d\n", hBinMemManager->uiFreeChunks);
   BKNI_Printf(" ChunkSize  = %d\n", hBinMemManager->uiChunkSize);
   BKNI_Printf(" Offset     = %d\n", hBinMemManager->uiOffset);
   BKNI_Printf(" Overspill  = %p\n", hBinMemManager->hOverspill);

   for (i = 0; i < hBinMemManager->uiNumChunks; ++i)
   {
      if (hBinMemManager->pInfo[i].bInUse)
      {
         BV3D_BinMemHandle hMem = BV3D_P_BinMemIndexToHandle(hBinMemManager, i);

         BKNI_Printf("Block %d : %p owned by %x : %p%s\n", i, hMem,
            hBinMemManager->pInfo[i].uiClient, (void *)hBinMemManager->pInfo[i].psJob,
            hBinMemManager->pInfo[i].uiClient == 0 ? " (OVERSPILL)" : "");
      }
   }
}

/***************************************************************************/

/* Calculate the number of free blocks
 * The overspill block is not treated as allocated.
 */
static uint32_t BV3D_P_BinMemNumFreeBlocks(
    BV3D_BinMemManagerHandle  hBinMemManager)
{
   return hBinMemManager->uiFreeChunks;
}

/***************************************************************************/

/* Are there enough free blocks to start binning or to allocate
 * bin memory for a job
 */
bool BV3D_P_BinMemEnoughFreeBlocks(
    BV3D_BinMemManagerHandle  hBinMemManager)
{
   uint32_t uiNumFree = BV3D_P_BinMemNumFreeBlocks(hBinMemManager);

   /* This is heuristic -- better metrics e.g. involving the number of clients could be devised
    * Why 2?  One for the current overspill and one for a future overspill -- we must always
    * have somewhere to allocate to let jobs finish
    */
   return uiNumFree > 2 && uiNumFree > (hBinMemManager->uiNumChunks * 1 / 4);
}

/***************************************************************************/

/* Reset the bin memory overspill if it is not valid
 */
void BV3D_P_BinMemReset(BV3D_BinMemManagerHandle hBinMemManager)
{
   /* We used to look to see if there are any owned blocks and return them to the pool.
    * BUT this should not occur unless there is a bug smewhere else and it caused problem
    * when memory was freed up that should not have been.
    */
   if (hBinMemManager->hOverspill == NULL)
      hBinMemManager->hOverspill = BV3D_P_BinMemAlloc(hBinMemManager, 0, NULL);
}

/***************************************************************************/

/* How big are the chunks? */
uint32_t BV3D_P_BinMemGetChunkSize(
   BV3D_BinMemManagerHandle hBinMemManager)
{
   return hBinMemManager->uiChunkSize;
}

/***************************************************************************/
