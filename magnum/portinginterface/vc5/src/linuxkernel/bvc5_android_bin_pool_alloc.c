/***************************************************************************
 *     Broadcom Proprietary and Confidential. (c)2015 Broadcom.  All rights reserved.
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

#include "bstd.h"
#include "bmma.h"
#include "bkni.h"

#include <linux/module.h>
#include "bvc5_bin_pool_alloc_priv.h"

BDBG_MODULE(BVC5_P);

/* We have no BVC5 module handle to store the interface under, so it has to be
 * static here. This means that only one Android can be present at any time. */
static BVC5_BinPoolBlock_MemInterface s_memInterface;

/***************************************************************************/
/* Register Android's bin memory interface                                 */
BERR_Code BVC5_RegisterAlternateMemInterface(
   struct module                  *psModule,
   BVC5_BinPoolBlock_MemInterface *pMemInterface
   )
{
   if (pMemInterface->BinPoolBlock_Alloc  == NULL ||
       pMemInterface->BinPoolBlock_Free   == NULL ||
       pMemInterface->BinPoolBlock_Lock   == NULL ||
       pMemInterface->BinPoolBlock_Unlock == NULL)
      return BERR_INVALID_PARAMETER;

   if (!try_module_get(psModule))
      return BERR_NOT_AVAILABLE;

   s_memInterface = *pMemInterface;

   return BERR_SUCCESS;
}

void BVC5_UnregisterAlternateMemInterface(
   struct module *psModule
   )
{
   BKNI_Memset(&s_memInterface, 0, sizeof(BVC5_BinPoolBlock_MemInterface));
   module_put(psModule);
}

EXPORT_SYMBOL(BVC5_RegisterAlternateMemInterface);
EXPORT_SYMBOL(BVC5_UnregisterAlternateMemInterface);

/***************************************************************************/
BMMA_Block_Handle BVC5_P_BinPoolBlock_AllocMem(
   BMMA_Heap_Handle   hHeap,
   size_t             zSize,
   uint32_t           uiAlign
   )
{
   if (s_memInterface.BinPoolBlock_Alloc != NULL)
      return s_memInterface.BinPoolBlock_Alloc(hHeap, zSize, uiAlign);
   return NULL;
}

/***************************************************************************/
void BVC5_P_BinPoolBlock_FreeMem(
   BMMA_Block_Handle hBlock
   )
{
   if (s_memInterface.BinPoolBlock_Free != NULL)
      s_memInterface.BinPoolBlock_Free(hBlock);
}

/***************************************************************************/
void BVC5_P_BinPoolBlock_LockMem(
   BMMA_Block_Handle hBlock,
   BMMA_DeviceOffset *puiLockOffset,
   uint32_t          *puiPhysOffset
   )
{
   if (s_memInterface.BinPoolBlock_Lock != NULL)
      s_memInterface.BinPoolBlock_Lock(hBlock, puiLockOffset, puiPhysOffset);
}

/***************************************************************************/
void BVC5_P_BinPoolBlock_UnlockMem(
   BMMA_Block_Handle hBlock,
   BMMA_DeviceOffset uiLockOffset)
{
   if (s_memInterface.BinPoolBlock_Unlock != NULL)
      s_memInterface.BinPoolBlock_Unlock(hBlock, uiLockOffset);
}
