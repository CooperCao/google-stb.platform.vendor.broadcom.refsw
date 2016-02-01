/***************************************************************************
 *     (c)2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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

#include "bstd.h"
#include "bvc5_priv.h"
#include "bvc5_bin_mem_priv.h"
#include "bvc5_bin_pool_priv.h"

#include "bkni.h"
#include "bkni_multi.h"
#include "bmem.h"

#define BVC5_P_INITIAL_BIN_ARRAY_LENGTH   16

/*****************************************************************************/

BERR_Code BVC5_P_BinMemArrayCreate(
   BVC5_P_BinMemArray  *psArray
)
{
   psArray->pvBinMemoryBlocks = (BVC5_BinBlockHandle*)BKNI_Malloc(sizeof(BVC5_BinBlockHandle) * BVC5_P_INITIAL_BIN_ARRAY_LENGTH);
   if (psArray->pvBinMemoryBlocks != NULL)
      psArray->uiTotalBinBlocks = BVC5_P_INITIAL_BIN_ARRAY_LENGTH;
   else
      psArray->uiTotalBinBlocks = 0;

   psArray->uiNumBinBlocks = 0;

   return psArray->pvBinMemoryBlocks == NULL ? BERR_OUT_OF_SYSTEM_MEMORY : BERR_SUCCESS;
}

void BVC5_P_BinMemArrayDestroy(
   BVC5_P_BinMemArray  *psArray,
   BVC5_BinPoolHandle   hBinPool
)
{
   uint32_t i;

   if (psArray->pvBinMemoryBlocks == NULL)
      return;

   BDBG_ASSERT(hBinPool != NULL || psArray->uiNumBinBlocks == 0);

   /* Free any left over bin blocks */
   for (i = 0; i < psArray->uiNumBinBlocks; i++)
   {
      BDBG_ASSERT(psArray->pvBinMemoryBlocks[i] != NULL);

      BVC5_P_BinPoolRecycle(hBinPool, psArray->pvBinMemoryBlocks[i]);
      psArray->pvBinMemoryBlocks[i] = NULL;
   }

   BKNI_Free(psArray->pvBinMemoryBlocks);
   psArray->pvBinMemoryBlocks = NULL;
   psArray->uiNumBinBlocks    = 0;
   psArray->uiTotalBinBlocks  = 0;
}

BVC5_BinBlockHandle BVC5_P_BinMemArrayAdd(
   BVC5_P_BinMemArray  *psArray,
   BVC5_BinPoolHandle   hBinPool,
   uint32_t             uiMinBlockSizeBytes,
   uint32_t            *uiPhysOffset
)
{
   BVC5_BinBlockHandle  pBlock = NULL;

   if (psArray->uiNumBinBlocks == psArray->uiTotalBinBlocks)
   {
      /* Grow the array */
      uint32_t uiNewSize = psArray->uiTotalBinBlocks + BVC5_P_INITIAL_BIN_ARRAY_LENGTH;

      BVC5_BinBlockHandle *ppvNewArray = (BVC5_BinBlockHandle*)BKNI_Malloc(sizeof(void*) * uiNewSize);
      if (ppvNewArray == NULL)
         return NULL;

      BKNI_Memcpy(ppvNewArray, psArray->pvBinMemoryBlocks, sizeof(void*) * psArray->uiTotalBinBlocks);
      BKNI_Memset(ppvNewArray + psArray->uiTotalBinBlocks, 0, sizeof(void*) * BVC5_P_INITIAL_BIN_ARRAY_LENGTH);

      BKNI_Free(psArray->pvBinMemoryBlocks);
      psArray->pvBinMemoryBlocks = ppvNewArray;
      psArray->uiTotalBinBlocks  = uiNewSize;
   }

   pBlock = BVC5_P_BinPoolAllocAtLeast(hBinPool, uiMinBlockSizeBytes, uiPhysOffset);

   if (pBlock != NULL)
   {
      psArray->pvBinMemoryBlocks[psArray->uiNumBinBlocks] = pBlock;
      psArray->uiNumBinBlocks++;
   }

   return pBlock;
}

BVC5_BinBlockHandle BVC5_P_BinMemArrayGetBlock(
   BVC5_P_BinMemArray *psArray,
   uint32_t uiIndex
)
{
   if (uiIndex < psArray->uiNumBinBlocks)
      return psArray->pvBinMemoryBlocks[uiIndex];

   return NULL;
}
