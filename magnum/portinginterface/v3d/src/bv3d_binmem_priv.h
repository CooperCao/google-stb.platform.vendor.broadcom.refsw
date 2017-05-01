/***************************************************************************
 *     Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef BV3D_BIN_MEM_PRIV__H
#define BV3D_BIN_MEM_PRIV__H

#include "bv3d.h"
 
/* Bin memory should be 256byte aligned */
#define BV3D_BIN_MEM_ALIGN_POW2  (8)

/* Bin memory cannot cross 256M boundary */
#define BV3D_BIN_MEM_BOUND_POW2  (28)
#define BV3D_BIN_MEM_BOUND_BITS  ((~0u) << BV3D_BIN_MEM_BOUND_POW2)

/* The bin memory object tracks memory allocations for the binner
   Because of restrictions on the alignment and position of the binner memory
   we allocate a pool up-front.  This pool is shared between client processes.
   Allocated memory is flagged as in use and when known attached to
   a particular job.
   When the job completes, the bin memory assocaiated with the job can be freed.
*/

typedef struct BV3D_P_BinMemManager   *BV3D_BinMemManagerHandle;
typedef void                          *BV3D_BinMemHandle;

/* Constructor for the bin memory object */
BERR_Code BV3D_P_BinMemCreate(
   BV3D_BinMemManagerHandle  *hBinMemManager,
   BMMA_Heap_Handle           hMma,
   uint32_t                   uiMemMegs,
   uint32_t                   uiChunkPow
);

/* Destructor for the bin memory object */
void BV3D_P_BinMemDestroy(
   BV3D_BinMemManagerHandle hBinMemManager
);

/* Allocate a chunk; returns chunk and size or NULL */
BV3D_BinMemHandle BV3D_P_BinMemAlloc(
   BV3D_BinMemManagerHandle   hBinMemManager,
   uint32_t                   uiClientId,
   uint32_t                   *puiSize
);

/* Attach a chunk to a client and job */
void BV3D_P_BinMemAttach(
   BV3D_BinMemManagerHandle   hBinMemManager,
   BV3D_BinMemHandle          hMem,
   BV3D_Job                   *psJob
);

/* Incoming memory chunks need to be paired up with the job they get  */
void BV3D_P_BinMemAttachToJob(
   BV3D_BinMemManagerHandle   hBinMemManager,
   BV3D_BinMemHandle          hMem,
   BV3D_Job                   *psJob
);

/* Free all chunks associated with a job */
void BV3D_P_BinMemReleaseByJob(
   BV3D_BinMemManagerHandle   hBinMemManager,
   BV3D_Job                   *psJob
);

/* Free all chunks associated with a process */
void BV3D_P_BinMemReleaseByClient(
   BV3D_BinMemManagerHandle   hBinMemManager,
   uint32_t                   uiClient
);

/* Mark current overspill memory as used by the job */
void BV3D_P_BinMemOverspillUsed(
   BV3D_BinMemManagerHandle   hBinMemManager,
   BV3D_Job                   *psJob
);

/* Allocate overspill memory */
bool BV3D_P_BinMemAllocOverspill(
   BV3D_BinMemManagerHandle   hBinMemManager
);

/* Get the overspill pointer */
uint32_t BV3D_P_BinMemGetOverspill(
   BV3D_BinMemManagerHandle  hBinMemManager
);

/* How many blocks does this job have? */
uint32_t BV3D_P_BinMemNumBlocksForClient(
   BV3D_BinMemManagerHandle   hBinMemManager,
   uint32_t                   uiClient
);

/* Convert from handle to index */
uint32_t BV3D_P_BinMemHandleToIndex(
   BV3D_BinMemManagerHandle   hBinMemManager,
   BV3D_BinMemHandle          hMem
);

/* Convert from index to handle */
BV3D_BinMemHandle BV3D_P_BinMemIndexToHandle(
   BV3D_BinMemManagerHandle   hBinMemManager,
   uint32_t                   uiIndex
);

/* Heuristic test used to throttle launch of bin jobs */
bool BV3D_P_BinMemEnoughFreeBlocks(
   BV3D_BinMemManagerHandle   hBinMemManager
);

/* Reset the bin memory tags where possible */
void BV3D_P_BinMemReset(
   BV3D_BinMemManagerHandle hBinMemManager
);

/* Get the chunk size */
uint32_t BV3D_P_BinMemGetChunkSize(
   BV3D_BinMemManagerHandle hBinMemManager
);

/* For debug */
void BV3D_P_BinMemDebugDump(
   BV3D_BinMemManagerHandle hBinMemManager
);

#endif /* BV3D_BIN_MEM_PRIV__H */
