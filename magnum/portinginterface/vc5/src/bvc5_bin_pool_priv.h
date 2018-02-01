/******************************************************************************
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
 ******************************************************************************/
#ifndef BVC5_BIN_POOL_PRIV__H
#define BVC5_BIN_POOL_PRIV__H

#include "bvc5.h"

#include "bkni.h"
#include "bkni_multi.h"
#include "bmma.h"
#include "blst_queue.h"

/* This is the size of all blocks in the pool - used for overspill
 * and (if it fits) the initial block too */
#define BVC5_BIN_MEM_MIN_POOL_BYTES          (128 * 1024)

typedef struct BVC5_P_BinPoolBlock
{
   BMMA_Block_Handle    hBlock;
   BMMA_DeviceOffset    uiLockOffset;
   uint32_t             uiNumBytes;

   /* Link pointers when used in the over-sized queue */
   BLST_Q_ENTRY(BVC5_P_BinPoolBlock)   sChain;

} BVC5_P_BinPoolBlock;

typedef struct BVC5_P_BinPool       *BVC5_BinPoolHandle;
typedef struct BVC5_P_BinPoolBlock  *BVC5_BinBlockHandle;

typedef struct BVC5_BinPoolBlock_MemInterface
{
   BMMA_Block_Handle (*BinPoolBlock_Alloc)(BMMA_Heap_Handle hHeap, size_t size, uint32_t align);
   void (*BinPoolBlock_Free)(BMMA_Block_Handle hBlock);
   void (*BinPoolBlock_Lock)(BMMA_Block_Handle hBlock, BMMA_DeviceOffset *lockOffset);
   void (*BinPoolBlock_Unlock)(BMMA_Block_Handle hBlock, BMMA_DeviceOffset lockOffset);

} BVC5_BinPoolBlock_MemInterface;

/***************************************************************************/
BVC5_BinPoolBlock_MemInterface *BVC5_P_GetBinPoolMemInterface(void);

/***************************************************************************/
BERR_Code BVC5_P_BinPoolCreate(
   BMMA_Heap_Handle     hMMAHeap,
   BVC5_BinPoolHandle  *phBinPool
);

/***************************************************************************/
void BVC5_P_BinPoolDestroy(
   BVC5_BinPoolHandle   hBinPool
);

/***************************************************************************/
BVC5_BinBlockHandle BVC5_P_BinPoolAllocAtLeast(
   BVC5_BinPoolHandle hBinPool,
   uint32_t           uiMinBytes,
   uint64_t          *uiPhysOffset
);

/***************************************************************************/
BERR_Code BVC5_P_BinPoolRecycle(
   BVC5_BinPoolHandle  hBinPool,
   BVC5_BinBlockHandle hBlock
);

/***************************************************************************/
bool BVC5_P_BinPoolReplenish(
   BVC5_BinPoolHandle   hBinPool
);

/***************************************************************************/
void BVC5_P_BinPoolPurge(
   BVC5_BinPoolHandle   hBinPool
);

/***************************************************************************/
void BVC5_P_BinPoolStats(
   BVC5_BinPoolHandle hBinPool,
   size_t             *puiCapacityBytes,
   size_t             *puiUsedBytes
);

/***************************************************************************/
uint64_t BVC5_P_BinBlockGetPhysical(
   BVC5_BinBlockHandle  hBlock
);

/***************************************************************************/

BMMA_Block_Handle BVC5_P_BinPoolBlock_AllocMem(
   BMMA_Heap_Handle   hHeap,
   size_t             zSize,
   uint32_t           uiAlign
   );

/***************************************************************************/
void BVC5_P_BinPoolBlock_FreeMem(
   BMMA_Block_Handle hBlock
   );

/***************************************************************************/
void BVC5_P_BinPoolBlock_LockMem(
   BMMA_Block_Handle hBlock,
   BMMA_DeviceOffset *puiLockOffset);

/***************************************************************************/
void BVC5_P_BinPoolBlock_UnlockMem(
   BMMA_Block_Handle hBlock,
   BMMA_DeviceOffset uiLockOffset);

#endif /* BVC5_BIN_POOL_PRIV__H */
