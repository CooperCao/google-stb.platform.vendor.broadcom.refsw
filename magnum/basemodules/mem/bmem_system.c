/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
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
 * This module provides the basic functions used to implement a memory
 * manager which stores bookkeeping information in OS-allocated memory. This
 * is advantageous when the memory being managed has significant access time
 * penalties. If accessing the managed memory has no access time penalties,
 * do NOT use this module; it is algorithmically slower than direct access
 * because of the additional overhead needed.
 *
 * All of the bookeeping information close to the CPU, in "system memory".
 * To do this, there needs to be some way of associating managed memory
 * block with info about the managed memory block. In the local allocator,
 * this is done by physically locating the BlockInfo in front of the managed
 * block. This same approach can't be used if the BlockInfo lives in system
 * memory.
 *
 * Instead, a hash table is used to index the BlockInfo structures. The
 * has index is based upon the lowest significant bits of the managed memory
 * address of the block. A linked list is used in each hash bucket to handle
 * collisions. The nodes in thist list are not ordered in any way.
 *
 * When looking up a BlockInfo for a manged memory address, the address is
 * shifted and masked to give the hash key. The list of nodes at that hash
 * key is iterated over until a match is found. If we find that the lists
 * grow to an unreasonable length, then the number of hash buckets can be
 * increased. This is not done dynamically.
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
#include "bkni.h"

BDBG_MODULE(BMEM);

/* The number of hash buckets to be used to store BlockInfo blocks */
#define BMEM_SYS_NUM_BUCKETS 256 /* must be a power of two */

/* Calculates the hash key given a managed memory address */
#define BUCKET_IDX(addr) (((addr) >> pshi->uiAlignment) & (BMEM_SYS_NUM_BUCKETS-1))

BLST_AA_TREE_GENERATE_INSERT(BMEM_P_AddrTree, BMEM_P_AddrKey*, BMEM_P_BlockInfo, stAddrNode, BMEM_P_Addr_Map_Cmp)
BLST_AA_TREE_GENERATE_FIRST(BMEM_P_AddrTree, BMEM_P_BlockInfo, stAddrNode)
BLST_AA_TREE_GENERATE_INSERT(BMEM_P_SizeTree, BMEM_P_SizeKey*, BMEM_P_BlockInfo, stSizeNode, BMEM_P_Size_Map_Cmp)


/***************************************************************************
 * BMEM_P_BINode - Stores managed block info for the system heap manger.
 *
 * Contains information about the managed block (the BlockInfo) as well as
 * members traversing the system-memory data structures.
 *
 */
typedef struct BMEM_P_BINode BMEM_P_BINode;
struct BMEM_P_BINode
{
	BMEM_P_BlockInfo pbi;   /* Managed block info. */
	                       /* Must be the first entry in this struct! */

	uint32_t addr;         /* Managed memory block base address */
	BMEM_P_BINode *pbiNext; /* Pointer to next node in the hash bucket */
	BMEM_P_BINode *pbiPrev; /* Pointer to previous node in the hash bucket */
};

/***************************************************************************
 * BMEM_P_SystemHeapInfo - Stores heap-wide info for the system heap manager.
 *
 * Heap-wide information specific to the System heap manager.
 *
 */
typedef struct BMEM_P_SystemHeapInfo
{
	unsigned int uiAlignment;
	BMEM_P_BINode *anodes[BMEM_SYS_NUM_BUCKETS];
} BMEM_P_SystemHeapInfo;

/**********************************************************************func*
 * BMEM_P_SystemGetAddress - Get the managed memory base address for a
 *                           block.
 *
 * Returns:
 *    0 if a NULL BlockInfo is given, otherwise the base address.
 */

uint32_t BMEM_P_SystemGetAddress
(
	BMEM_Handle pheap,
	BMEM_P_BlockInfo *pbi
)
{
	BSTD_UNUSED( pheap );

	if( pbi == NULL )
	{
		return 0;
	}

	/* We'll take advantage of BINode having the BlockInfo right at the
	 * front and just magically cast it.
	 */
	return ((BMEM_P_BINode *) pbi)->addr;
}

/**********************************************************************func*
 * BMEM_P_SystemGetBlockInfo - Get the bookkeeping info for a managed base
 *                             address.
 *
 * This function finds the bookeeping information for a given base address.
 * If the given base address has no bookkeeping information assigned to it,
 * a new BlockInfo is allocated from the system heap and added to the hash.
 * The new BlockInfo is then returned.
 *
 * Returns:
 *    On success, a pointer to the BlockInfo for the managed base address.
 *    On failure (which can occur if the system heap is full), NULL.
 *
 */

BMEM_P_BlockInfo *BMEM_P_SystemGetBlockInfo
(
	BMEM_Handle pheap,
	uint32_t addr
)
{
	BMEM_P_SystemHeapInfo *pshi = (BMEM_P_SystemHeapInfo *)(pheap->pvData);
	int idx = BUCKET_IDX(addr);

	BMEM_P_BINode *pbinode = pshi->anodes[idx];
	BMEM_P_BlockInfo *pbi;

	while(pbinode!=NULL && pbinode->addr!=addr)
	{
		pbinode = pbinode->pbiNext;
	}

	if(pbinode == NULL)
	{
		/* No entry found. Make a new one */
		pbinode = (BMEM_P_BINode *)BKNI_Malloc(sizeof(BMEM_P_BINode));
		if(pbinode == NULL)
		{
			BDBG_ERR(("Out of system memory. Can't allocate bookkeeping!"));
			return NULL;
		}
		pbinode->addr = addr;
		pbinode->pbiNext = pshi->anodes[idx];
		pbinode->pbiPrev = NULL;
		pshi->anodes[idx] = pbinode;
		pbinode->pbi.pHeap = pheap;

		if(pbinode->pbiNext!=NULL)
		{
			pbinode->pbiNext->pbiPrev = pbinode;
		}
	}

	/* We'll take advantage of BINode having the BlockInfo right at the
	 * front and just magically cast it.
	 */
	pbi = (BMEM_P_BlockInfo *)pbinode;

	return pbi;
}

/**********************************************************************func*
 * BMEM_P_SystemDropBlockInfo -
 *    Free up the bookkeeping info for a managed memory base address
 *
 * Returns:
 *    void
 */

void BMEM_P_SystemDropBlockInfo
(
	BMEM_Handle pheap,
	BMEM_P_BlockInfo *pbi
)
{
	BMEM_P_SystemHeapInfo *pshi = (BMEM_P_SystemHeapInfo *)pheap->pvData;
	/* We'll take advantage of BINode having the BlockInfo right at the
	 * front and just magically cast it.
	 */
	BMEM_P_BINode *pbinode = (BMEM_P_BINode *) pbi;
	int idx = BUCKET_IDX(pbinode->addr);

	/* Remove the BINode from the list in the hash table. */
	if(pbinode->pbiPrev != NULL)
	{
		pbinode->pbiPrev->pbiNext = pbinode->pbiNext;
	}
	if(pbinode->pbiNext != NULL)
	{
		pbinode->pbiNext->pbiPrev = pbinode->pbiPrev;
	}

	if(pshi->anodes[idx] == pbinode)
	{
		pshi->anodes[idx] = pbinode->pbiNext;
	}

	BKNI_Free(pbinode);
}

/**********************************************************************func*
 * bcmHeapCreate - Initializes the heap with local bookkeeping.
 *
 * This function inititalizes a heap at a given location and size.
 * Any previous allocations in the chunk of memory handed over to this
 * function are lost. Every heap has a base minimum alignment for all of
 * the allocations within that heap. (However, you can specify a greater
 * alignment when actually doing an allocation.)
 *
 * In this implementation, memory in the CPU's system heap is used to store
 * the bookkeeping information for the heap.
 *
 * Returns:
 *   Returns true if heap is initialized, or false if the size of the heap
 *   is too small to manage or there isn't enough system memory to allocate
 *   bookkeeping information.
 *
 */
BERR_Code BMEM_CreateHeapSystem
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
	pHeapSettings->eBookKeeping = BMEM_BOOKKEEPING_SYSTEM;

	return BMEM_Heap_Create(NULL, pvHeap, ulOffset, zSize, pHeapSettings, ppHeap);
}

BERR_Code BMEM_P_SystemCreateHeap
(
   BMEM_ModuleHandle    hMem,        /* main handle from BMEM_Open() - NULL is possible for older chipsets (7038/3560 only) */
	BMEM_Handle        *ppHeap,      /* Address to store pointer of new heap info structure */
	void               *pvHeap,      /* Pointer to beginning of memory chunk to manage */
	uint32_t            ulOffset,    /* Device offset of initial location */
	size_t              zSize,       /* Size of chunk to manage in bytes */
	BMEM_Heap_Settings *pHeapSettings
)
{
	BERR_Code              err = BERR_SUCCESS;
	BMEM_P_SystemHeapInfo *pshi;
	BMEM_P_BlockInfo      *pbi;
	BMEM_P_Heap           *pheap;
	uint32_t               addrHeap = (uint32_t)pvHeap;
	uint32_t               addrEnd = (uint32_t)pvHeap + zSize - 1;
	int                    i;
	unsigned int           uiAlignment = pHeapSettings->uiAlignment;
	size_t                 ulAlignedSize;
	uint32_t               eSafetyConfig = pHeapSettings->eSafetyConfig;
	BMEM_P_AddrKey         stAddrKey;
	BMEM_P_SizeKey         stSizeKey;

	BDBG_ENTER(BMEM_P_SystemCreateHeap);

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

	/* allocate heap structure */
	*ppHeap = (BMEM_P_Heap*) BKNI_Malloc( sizeof (BMEM_P_Heap) );
	if( *ppHeap == NULL )
	{
		err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(( "Out of system memory. Can't allocate bookkeeping!" ));
		goto done;
	}

	/* initialize heap structure */
	pheap = (BMEM_P_Heap *) *ppHeap;
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

	pheap->pGetAddressFunc = &BMEM_P_SystemGetAddress;
	pheap->pGetBlockInfoFunc = &BMEM_P_SystemGetBlockInfo;
	pheap->pDropBlockInfoFunc = &BMEM_P_SystemDropBlockInfo;
	pheap->pDestroyHeapFunc = &BMEM_P_SystemDestroyHeap;

	pheap->hMem = hMem;

	BLST_AA_TREE_INIT(BMEM_P_AddrTree, &pheap->stFreeTopAddrTree);
	BLST_AA_TREE_INIT(BMEM_P_SizeTree, &pheap->stFreeTopSizeTree);
	BLST_AA_TREE_INIT(BMEM_P_AddrTree, &pheap->stAllocTopAddrTree);

#if (BMEM_REENTRANT_CONFIG==BMEM_REENTRANT)
	/* create mutex for re-entrant control */
	err = BERR_TRACE(BKNI_CreateMutex(&(pheap->pMutex)));
	if (err != BERR_SUCCESS)
	{
		goto done;
	}
#endif


	/* Force at least the minimum heap alignment */
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

	/* Allocate the hash buckets for storing the BlockInfos */
	pheap->pvData = BKNI_Malloc(sizeof(BMEM_P_SystemHeapInfo));
	if(pheap->pvData == NULL)
	{
		/* unable to allocate */
		err = BERR_OUT_OF_SYSTEM_MEMORY;
		BDBG_ERR(("Out of system memory. Can't allocate hash table!"));
		goto done;
	}
	pshi = (BMEM_P_SystemHeapInfo *)pheap->pvData;

	/* initialize table */
	for(i = 0; i<BMEM_SYS_NUM_BUCKETS; ++i)
	{
		pshi->anodes[i] = NULL;
	}
	pshi->uiAlignment = uiAlignment;

	/*
	 * Starting here, it's safe to make calls to the other heap management
	 * functions.
	 */

	/* Make the first block, which represents the remaining (free) heap */
	pbi = BMEM_P_SystemGetBlockInfo(pheap, (uint32_t)pheap->pStart);
	
	ulAlignedSize = pheap->pEnd-pheap->pStart+1;

	/* heap size greater than bookkeeping? */
	if(ulAlignedSize > BMEM_BOOKKEEPING_SIZE_SYSTEM)
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
	/* memory isn't even large enough for bookkeeping */
	{
		/* cannot create heap from this memory */
		err = BERR_INVALID_PARAMETER;
	}

#if BMEM_CHECK_ALL_GUARDS
	if (pheap->pSafetyConfigInfo->bCheckAllGuards)
	{
		/* heap has been created? */
		if (err == BERR_SUCCESS)
		{
			/* validate heap */
			err = BMEM_ValidateHeap(pheap);
		}
	}
#endif

done:
	/* free heap if error occured */
	if ((err != BERR_SUCCESS) && *ppHeap!=NULL)
	{
		/* destroy heap */
		BMEM_DestroyHeap(*ppHeap);
		*ppHeap = NULL;
	}

	/* return status */
	BDBG_MSG(("BMEM_P_SystemCreateHeap(%p) size=%lu, align=%d", *ppHeap, zSize, (int)uiAlignment));
	BDBG_LEAVE(BMEM_P_SystemCreateHeap);
	return err;
}

/***************************************************************************/
void BMEM_P_SystemDestroyHeap
(
	BMEM_Handle pheap
)
{
#if BMEM_CHECK_ALL_GUARDS
	BERR_Code err;
#endif
	BMEM_P_SystemHeapInfo *pshi;
	unsigned i;

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
		BMEM_Dbg_DumpHeap(pheap);
	}

	/* free system hash table (should be empty since there
	   are no outstanding allocations) */
	pshi = (BMEM_P_SystemHeapInfo *)(pheap->pvData);
	for (i=0;i<BMEM_SYS_NUM_BUCKETS;i++) {
		BMEM_P_BINode *pbi = pshi->anodes[i];
		BMEM_P_BINode *pbiNext;
		while(pbi) {
			pbiNext = pbi->pbiNext;
			BKNI_Free(pbi);
			pbi = pbiNext;
		}
	}

	BKNI_Free(pheap->pvData);

#if (BMEM_REENTRANT_CONFIG==BMEM_REENTRANT)
	/* destroy mutex */
	BKNI_DestroyMutex(pheap->pMutex);
#endif

	/* free heap handle */
	BDBG_OBJECT_UNSET(pheap, BMEM_Heap);
	BKNI_Free(pheap);

done:
	/* return */
	return;
}

/* End of File */
