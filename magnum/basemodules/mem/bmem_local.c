/***************************************************************************
 *     Copyright (c) 2002-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bmem.h"
#include "bmem_config.h"
#include "bmem_priv.h"
#include "bmem_debug.h"
#include "berr.h"

BDBG_MODULE(BMEM);

BLST_AA_TREE_GENERATE_INSERT(BMEM_P_AddrTree, BMEM_P_AddrKey*, BMEM_P_BlockInfo, stAddrNode, BMEM_P_Addr_Map_Cmp)
BLST_AA_TREE_GENERATE_FIRST(BMEM_P_AddrTree, BMEM_P_BlockInfo, stAddrNode)
BLST_AA_TREE_GENERATE_INSERT(BMEM_P_SizeTree, BMEM_P_SizeKey*, BMEM_P_BlockInfo, stSizeNode, BMEM_P_Size_Map_Cmp)

/**********************************************************************func*
 * BMEM_P_LocalGetAddress - Get the managed memory base address for a
 *                           block.
 *
 * Returns:
 *    0 if a NULL BlockInfo is given, otherwise the base address.
 */

uint32_t BMEM_P_LocalGetAddress
(
	BMEM_Handle pheap,
	BMEM_P_BlockInfo *pbi
)
{
	BSTD_UNUSED( pheap );
	return (uint32_t) pbi;
}

/**********************************************************************func*
 * BMEM_P_LocalGetBlockInfo - Get the bookkeeping info for a managed base
 *                             address.
 *
 * Local implementation just returns the address, being also the location
 * of the bookkeeping info.
 *
 * Returns:
 *    On success, a pointer to the BlockInfo for the managed base address.
 *
 */

BMEM_P_BlockInfo *BMEM_P_LocalGetBlockInfo
(
	BMEM_Handle pheap,
	uint32_t addr
)
{
	BMEM_P_BlockInfo *pbi = (BMEM_P_BlockInfo *)addr;

	pbi->pHeap = pheap;

	return pbi;
}

/**********************************************************************func*
 * BMEM_P_LocalDropBlockInfo -
 *    Free up the bookkeeping info for a managed memory base address
 *
 * Returns:
 *    void
 */

void BMEM_P_LocalDropBlockInfo
(
	BMEM_Handle pheap,
	BMEM_P_BlockInfo *pbi
)
{
	BSTD_UNUSED( pheap );
	BSTD_UNUSED( pbi );
	return;
}


/**********************************************************************func*
 * BMEM_CreateHeapLocal - Initializes the heap with local bookkeeping.
 *
 * This function inititalizes a heap at a given location and size.
 * Any previous allocations in the chunk of memory handed over to this
 * function are lost. Every heap has a base minimum alignment for all of
 * the allocations within that heap. (However, you can specify a greater
 * alignment when actually doing an allocation.)
 *
 * In this implementation, the first bit of the heap is used to store heap
 * information such as pointers to the free and used list, as well as other
 * bookkeeping information.
 *
 * Returns:
 *   Returns true if heap is initialized, or false if the given memory
 *   chunk is too small to be a heap.
 *
 */
BERR_Code BMEM_CreateHeapLocal
(
	BMEM_Handle  *ppHeap,      /* Address to store pointer of new heap info structure */
	void         *pvHeap,      /* Pointer to beginning of memory chunk to manage */
	uint32_t      ulOffset,    /* Device offset of initial location */
	size_t        zSize,       /* Size of chunk to manage in bytes */
	unsigned int  uiAlignment  /* Minimum alignment for all allocations in the heap */
)
{
	BMEM_Heap_Settings stHeapSettings;
	BMEM_Heap_Settings *pHeapSettings = &stHeapSettings;

	BMEM_Heap_GetDefaultSettings(pHeapSettings);

	pHeapSettings->uiAlignment = uiAlignment;
	pHeapSettings->eBookKeeping = BMEM_BOOKKEEPING_LOCAL;

	return (BMEM_Heap_Create(NULL, pvHeap, ulOffset, zSize, pHeapSettings, ppHeap));
}

BERR_Code BMEM_P_LocalCreateHeap
(
	BMEM_Handle        *ppHeap,      /* Address to store pointer of new heap info structure */
	void               *pvHeap,      /* Pointer to beginning of memory chunk to manage */
	uint32_t            ulOffset,    /* Device offset of initial location */
	size_t              zSize,       /* Size of chunk to manage in bytes */
	BMEM_Heap_Settings *pHeapSettings
)
{
	BERR_Code         err = BERR_SUCCESS;
	BMEM_Handle       pheap;
	BMEM_P_BlockInfo *pbi;
	uint32_t          addrHeap;
	uint32_t          addrEnd = (uint32_t)pvHeap + zSize - 1;
	unsigned int      uiAlignment = pHeapSettings->uiAlignment;
	size_t            ulAlignedSize;
	uint32_t          eSafetyConfig = pHeapSettings->eSafetyConfig;
	BMEM_P_AddrKey    stAddrKey;
	BMEM_P_SizeKey    stSizeKey;

	BDBG_ENTER(BMEM_P_LocalCreateHeap);
	BDBG_MSG(("BMEM_P_LocalCreateHeap(%p, %lu, %d)",
		pvHeap, zSize, (int)pHeapSettings->uiAlignment));

	if (pHeapSettings->eSafetyConfig > BMEM_P_SafetyTableSize)
	{
		/* Invalid safety config. */
		err = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto done;
	}

	if (pHeapSettings->eSafetyConfig > BMEM_SAFETY_CONFIG)
	{
		/* Invalid run-time safety config. */
			BDBG_ERR(("Run-time safety config cannot be higher than compiled safety config!"));
			err = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto done;
	}


	/*
	 * The first bit of the heap is used to hold information about the heap
	 * itself. This is forced to a 4-byte alignment if for some reason we
	 * were given a non-aligned pointer.
	 */
	addrHeap = BMEM_HEAP_ALIGN( pvHeap );
	if( addrEnd < addrHeap + sizeof **ppHeap - 1)
	{
		/* There is no way to use this as a heap. It's too small. */
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		goto done;
	}

	/* get location of heap */
	*ppHeap = pheap = (BMEM_Handle) addrHeap;
	addrHeap += sizeof (*pheap);

	/* initialize heap */
	BDBG_OBJECT_SET(pheap, BMEM_Heap);
	pheap->ulAlignMask = 0;
	pheap->pStart = NULL;
	pheap->pEnd = NULL;
	pheap->ulNumErrors = 0;
	pheap->pvData = NULL;
	pheap->pvHeap = pvHeap;
	pheap->ulOffset = ulOffset;
	pheap->monitor = NULL;
	pheap->pvCache = pHeapSettings->pCachedAddress;
	pheap->pfFlushCb = pHeapSettings->flush;
	pheap->pfFlushCb_isr = pHeapSettings->flush_isr;
	pheap->zSize = zSize;
	pheap->ulNumAllocated = 0;
	pheap->ulNumFree = 0;
	pheap->ulTotalAllocated = 0;
	pheap->ulTotalFree = 0;
	pheap->ulHighWatermark = 0;

	pheap->pSafetyConfigInfo = &(BMEM_P_SafetyConfigTbl[eSafetyConfig]);
	pheap->ulGuardSizeBytes  = pheap->pSafetyConfigInfo->iGuardSize * BMEM_GUARD_UNIT_BYTES;

	pheap->eBookKeeping = pHeapSettings->eBookKeeping;
	pheap->bCacheLocked = false;

	pheap->pGetAddressFunc = &BMEM_P_LocalGetAddress;
	pheap->pGetBlockInfoFunc = &BMEM_P_LocalGetBlockInfo;
	pheap->pDropBlockInfoFunc = &BMEM_P_LocalDropBlockInfo;
	pheap->pDestroyHeapFunc = &BMEM_P_LocalDestroyHeap;

	BLST_AA_TREE_INIT(BMEM_P_AddrTree, &pheap->stFreeTopAddrTree);
	BLST_AA_TREE_INIT(BMEM_P_SizeTree, &pheap->stFreeTopSizeTree);
	BLST_AA_TREE_INIT(BMEM_P_AddrTree, &pheap->stAllocTopAddrTree);

#if (BMEM_REENTRANT_CONFIG==BMEM_REENTRANT)
	/* create mutex for re-entrant control */
	if(BKNI_CreateMutex(&(pheap->pMutex)) != BERR_SUCCESS)
	{
		err = BERR_OS_ERROR;
		goto done;
	}
#endif

	/*
	 * The minimum alignment allowed is 4 bytes since that is the required
	 * alignment for the bookkeeping information.
	 */
	if(uiAlignment < BMEM_HEAP_ALIGNMENT)
	{
		BDBG_WRN(("uiAlignment %d less than platform minimum %d.  Setting alignment to minimum.", uiAlignment, BMEM_HEAP_ALIGNMENT));
		uiAlignment = BMEM_HEAP_ALIGNMENT;
	}
	pheap->uiAlignment = uiAlignment;
	pheap->ulAlignMask = (1L << uiAlignment) - 1;

	/* Align the heap to be on the requested alignment. */
	pheap->pStart = (uint8_t *)((addrHeap+pheap->ulAlignMask) & ~pheap->ulAlignMask);
	/* Last usable byte after alignment. */
	pheap->pEnd = (uint8_t *)(((addrEnd + 1) & ~pheap->ulAlignMask) - 1);

	/* Make the first block, which represents the remaining (free) heap */
	pbi = BMEM_P_LocalGetBlockInfo(pheap, (uint32_t)pheap->pStart);

	ulAlignedSize = pheap->pEnd-pheap->pStart+1;

	if(ulAlignedSize > BMEM_BOOKKEEPING_SIZE_LOCAL)
	{
		/* Create a free block representing the entire heap. */
		pbi->ulSize = ulAlignedSize;
		pbi->ulFrontScrap = 0;
		pbi->ulBackScrap = 0;
		BMEM_P_FillGuard(pheap, pbi);

		stAddrKey.ulAddr = BMEM_P_GetAddress(pheap, pbi);
		stSizeKey.ulAddr = BMEM_P_GetAddress(pheap, pbi);
		stSizeKey.ulSize = pbi->ulSize;
		
		BLST_AA_TREE_INSERT(BMEM_P_AddrTree, &pheap->stFreeTopAddrTree, &stAddrKey, pbi);
		BLST_AA_TREE_INSERT(BMEM_P_SizeTree, &pheap->stFreeTopSizeTree, &stSizeKey, pbi);

		pheap->ulNumFree++;
		pheap->ulTotalFree = pbi->ulSize;
	}
	else
	{
		/* The chunk of memory we've been given is too small for a heap. */
		err = BERR_OS_ERROR;
	}

#if BMEM_CHECK_ALL_GUARDS
	if (pheap->pSafetyConfigInfo->bCheckAllGuards)
	{
		if (err)
		{
			BMEM_ValidateHeap(pheap);
		}
	}
#endif

done:
	BDBG_MSG(("BMEM_P_LocalCreateHeap = %d", err));
	BDBG_LEAVE(BMEM_P_LocalCreateHeap);
	return err;
}

/***************************************************************************/
void BMEM_P_LocalDestroyHeap
(
	BMEM_Handle pheap
)
{
#if BMEM_CHECK_ALL_GUARDS
	BERR_Code  err;
#endif

	BDBG_OBJECT_ASSERT(pheap, BMEM_Heap);

#if BMEM_CHECK_ALL_GUARDS
	if (pheap->pSafetyConfigInfo->bCheckAllGuards)
	{
		/* validate heap */
		err = BERR_TRACE(BMEM_ValidateHeap(pheap));
		if (err != BERR_SUCCESS)
		{
			/* error validating heap */
			goto done;
		}
	}
#endif

	/* blocks still allocated? */
	if (BLST_AA_TREE_FIRST(BMEM_P_AddrTree, &pheap->stAllocTopAddrTree))
	{
		/* leaked resource */
		BDBG_ERR(("Leaked resource -- unfreed blocks detected"));
	}

	BDBG_OBJECT_UNSET(pheap, BMEM_Heap);
#if (BMEM_REENTRANT_CONFIG==BMEM_REENTRANT)
	/* destroy mutex */
	BKNI_DestroyMutex(pheap->pMutex);
#endif

done:
	/* return */
	return;
}

/* End of File */
