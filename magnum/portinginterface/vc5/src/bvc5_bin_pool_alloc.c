/***************************************************************************
 *     (c)2015 Broadcom Corporation
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
#include "bmma.h"

BDBG_MODULE(BVC5_P);

/***************************************************************************/
BMMA_Block_Handle BVC5_P_BinPoolBlock_AllocMem(
   BMMA_Heap_Handle   hHeap,
   size_t             zSize,
   uint32_t           uiAlign
   )
{
   BMMA_AllocationSettings pSettings;
   BMMA_Block_Handle hBlock = NULL;

   BMMA_GetDefaultAllocationSettings(&pSettings);
#ifdef BMMA_ALLOC_HAS_DESC
   pSettings.desc = "Binner Pool";
#endif
   hBlock = BMMA_Alloc(hHeap, zSize, uiAlign, &pSettings);
   if (hBlock != NULL)
      BMMA_MarkDiscarable(hBlock);

   return hBlock;
}

/***************************************************************************/
void BVC5_P_BinPoolBlock_FreeMem(
   BMMA_Block_Handle hBlock
   )
{
   BMMA_Free(hBlock);
}

/***************************************************************************/
void BVC5_P_BinPoolBlock_LockMem(
   BMMA_Block_Handle hBlock,
   BMMA_DeviceOffset *puiLockOffset,
   uint32_t          *puiPhysOffset
   )
{
   *puiLockOffset = BMMA_LockOffset(hBlock);
   *puiPhysOffset = *puiLockOffset & 0xFFFFFFFF;
   BDBG_ASSERT(*puiLockOffset >> 32 == 0);
}

/***************************************************************************/
void BVC5_P_BinPoolBlock_UnlockMem(
   BMMA_Block_Handle hBlock,
   BMMA_DeviceOffset uiLockOffset)
{
   BMMA_UnlockOffset(hBlock, uiLockOffset);
   BMMA_MarkDiscarable(hBlock);
}
