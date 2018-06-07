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
#ifndef __BVC5_BIN_MEM_PRIV_H__
#define __BVC5_BIN_MEM_PRIV_H__

#include "bvc5_bin_pool_priv.h"

/* BVC5_P_BinMemArray

   Each job holds an array of bin memory allocations.
   The array grows as needed.
   The first entry is the initial allocation.
   Subsequent entries hold any overspill.

 */
typedef struct BVC5_P_BinMemArray
{
   BVC5_BinBlockHandle *pvBinMemoryBlocks;
   uint32_t             uiNumBinBlocks;
   uint32_t             uiTotalBinBlocks;
   BVC5_BinPoolHandle   hBinPool;
} BVC5_P_BinMemArray;

/* BVC5_P_BinMemArrayCreate

   Initialize a BVC5_P_BinMemArray

 */
BERR_Code BVC5_P_BinMemArrayCreate(
   BVC5_BinPoolHandle   hBinPool,
   BVC5_P_BinMemArray  *psArray
);

/* BVC5_P_BinMemArrayDestroy

   Destroy a BVC5_P_BinMemArray
   Frees all allocated bin memory in this array
 */
void BVC5_P_BinMemArrayDestroy(
   BVC5_P_BinMemArray  *psArray
);

/* BVC5_P_BinMemArrayAdd

   Add a new bin memory allocation to the array

 */
BVC5_BinBlockHandle BVC5_P_BinMemArrayAdd(
   BVC5_P_BinMemArray  *psArray,
   uint32_t             uiMinBlockSizeBytes,
   uint64_t            *puiPhysOffset
);

/* BVC5_P_BinMemArrayGetBlock

   Obtain bin memory from slot

 */
BVC5_BinBlockHandle BVC5_P_BinMemArrayGetBlock(
   BVC5_P_BinMemArray *psArray,
   uint32_t uiIndex
);

/* BVC5_BinMemArrayReplenishPool

   Replenish the bin memory pool
*/
void BVC5_P_BinMemArrayReplenishPool(
   BVC5_P_BinMemArray *psArray
);

#endif /* __BVC5_BIN_MEM_PRIV_H__ */