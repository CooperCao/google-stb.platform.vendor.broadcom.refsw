/***************************************************************************
 *     Copyright (c) 2001-2013, Broadcom Corporation
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
#include "bkni.h"

BDBG_MODULE(BMEM_DBG);
BDBG_FILE_MODULE(BMEM_ALLOCATED);
BDBG_FILE_MODULE(BMEM_FREE);

#define BMEM_MAP_SIZE       (80*40)
#define BMEM_MAP_ADDR(addr) ((addr) * BMEM_MAP_SIZE / (pheap->pEnd - pheap->pStart + 1))

BLST_AA_TREE_GENERATE_FIRST(BMEM_P_AddrTree, BMEM_P_BlockInfo, stAddrNode)
BLST_AA_TREE_GENERATE_NEXT(BMEM_P_AddrTree, BMEM_P_BlockInfo, stAddrNode)

BLST_AA_TREE_GENERATE_FIRST(BMEM_P_SizeTree, BMEM_P_BlockInfo, stSizeNode)
BLST_AA_TREE_GENERATE_NEXT(BMEM_P_SizeTree, BMEM_P_BlockInfo, stSizeNode)


/**********************************************************************func*
 * BMEM_Dbg_Map
 *
 */
void BMEM_Dbg_Map
(
	BMEM_Handle pheap
)
{
	int               i;
	BMEM_P_BlockInfo *pbi;
	char             *ach;

	BDBG_OBJECT_ASSERT(pheap, BMEM_Heap);

	/* Allocate memory for ach */
	ach = (char *)(BKNI_Malloc((BMEM_MAP_SIZE+1)*sizeof(char)));
	if(!ach)
	{
		return;
	}

	for(i = 0; i < BMEM_MAP_SIZE; ++i)
	{
		ach[i] = '#';
	}

	/* now by address order instead of allocated order */
/*
	for(pbi = pheap->pAllocTop; pbi != NULL; pbi = pbi->pnext)
*/
	for(pbi=BLST_AA_TREE_FIRST(BMEM_P_AddrTree, &pheap->stAllocTopAddrTree);
		pbi != NULL;
		pbi=BLST_AA_TREE_NEXT(BMEM_P_AddrTree, &pheap->stAllocTopAddrTree, pbi))
	{
		uint32_t addrStart = BMEM_P_GetAddress( pheap, pbi ) - pbi->ulFrontScrap -
			(uint32_t) pheap->pStart;
		uint32_t addrFront = BMEM_P_GetAddress(pheap, pbi) - (uint32_t)pheap->pStart;
		uint32_t addrBack = addrStart+pbi->ulSize-pbi->ulBackScrap;
		uint32_t addrEnd = addrStart+pbi->ulSize;
		uint32_t ul;

		addrStart = (uint32_t)BMEM_MAP_ADDR(addrStart);
		addrFront = (uint32_t)BMEM_MAP_ADDR(addrFront);
		addrBack = (uint32_t)BMEM_MAP_ADDR(addrBack);
		addrEnd = (uint32_t)BMEM_MAP_ADDR(addrEnd);
		for(ul = addrStart; ul < addrFront; ++ul)
		{
			ach[ul] = ',';
		}
		for(ul = addrFront+1; ul < addrBack; ++ul)
		{
			ach[ul]='=';
		}
		for(ul = addrBack; ul<addrEnd; ++ul)
		{
			ach[ul]='.';
		}
		if(ach[addrFront] == '#')
		{
			ach[addrFront] = 'a';
		}
		else
		{
			ach[addrFront]++;
		}
	}


	for(pbi=BLST_AA_TREE_FIRST(BMEM_P_SizeTree, &pheap->stFreeTopSizeTree);
		pbi != NULL;
		pbi=BLST_AA_TREE_NEXT(BMEM_P_SizeTree, &pheap->stFreeTopSizeTree, pbi))
	{
		uint32_t addrStart = BMEM_P_GetAddress( pheap, pbi ) - pbi->ulFrontScrap -
			(uint32_t) pheap->pStart;
		uint32_t addrEnd = addrStart + pbi->ulSize;
		uint32_t ul;

		addrStart = (uint32_t)BMEM_MAP_ADDR(addrStart);
		addrEnd = (uint32_t)BMEM_MAP_ADDR(addrEnd);
		for(ul = addrStart+1; ul < addrEnd; ++ul)
		{
			ach[ul] = '-';
		}


		if(ach[addrStart] == '#')
		{
			ach[addrStart] = 'a';
		}
		else
		{
			ach[addrStart]++;
		}
	}

	ach[BMEM_MAP_SIZE] = '\0';
	BDBG_MSG(("%s", ach));
	BKNI_Free((char *)ach);
}

#if BDBG_DEBUG_BUILD
/**********************************************************************func*
 * BMEM_Dbg_DumpBlock
 *
 */
static void BMEM_Dbg_DumpBlock(BMEM_Handle pheap, BMEM_P_BlockInfo *pbi, bool freeBlock)
{
	const char *filename;
	unsigned linenum;

	BDBG_OBJECT_ASSERT(pheap, BMEM_Heap);
	if (freeBlock) {
		/* free block has possibly uninitialized filename/linenum */
		filename = NULL;
		linenum = 0;
	}
	else {
		/* eliminate the path which is usually unnecessary and very long */
		filename = pbi->pchFilename;
	    while (*filename) filename++;
		while (*filename != '/' && filename > pbi->pchFilename) filename--;
		if (*filename == '/') filename++;

	    linenum = pbi->ulLine;
	}

	/* This format is directly importable to Excel where you can sort by address.
	It is not DBG output. It should be defaulted off because it will always print. */
	{
        uint32_t ulFrontBookKeepingSize = (pheap->eBookKeeping == BMEM_BOOKKEEPING_LOCAL) ?
                                          BMEM_FRONT_BOOKKEEPING_SIZE_LOCAL : BMEM_FRONT_BOOKKEEPING_SIZE_SYSTEM;
        uint8_t *addr = (uint8_t *)(BMEM_P_GetAddress( pheap, pbi ) + (uint32_t)ulFrontBookKeepingSize);
        if(freeBlock) {
            if(pbi->ulSize>=256) {
                BDBG_MODULE_LOG(BMEM_FREE, ("%#x,%-8u", (unsigned)(pheap->ulOffset +(addr - (uint8_t *)pheap->pvHeap)), (unsigned)pbi->ulSize));
            } else {
                BDBG_MODULE_MSG(BMEM_FREE, ("%#x,%-8u", (unsigned)(pheap->ulOffset +(addr - (uint8_t *)pheap->pvHeap)), (unsigned)pbi->ulSize));
            }
        } else {
            unsigned userSize = ((pbi)->ulSize - (pbi)->ulFrontScrap - (pbi)->ulBackScrap - ((pheap->eBookKeeping == BMEM_BOOKKEEPING_LOCAL) ? BMEM_FRONT_BOOKKEEPING_SIZE_LOCAL : BMEM_FRONT_BOOKKEEPING_SIZE_SYSTEM) - BMEM_BACK_BOOKKEEPING_SIZE);
            BDBG_MODULE_LOG(BMEM_ALLOCATED, ("%#x,%-8u,%s:%u", (unsigned)(pheap->ulOffset +(addr - (uint8_t *)pheap->pvHeap)), userSize, filename, linenum));
            BDBG_MSG((" %p,%p", addr, addr+userSize));
        }
	}

#ifdef BMEM_TRACK_FILE_AND_LINE
	if(pheap->pSafetyConfigInfo->bTrackFileAndLine)
	{
		BDBG_MSG(("(%s:%d)", filename, pbi->ulLine));
	}
#endif
	/* The front scrap area comes before the pbi data structure */
	BDBG_MSG(("%p[%#x..%#x..%#x]: [%lu][%lu][%lu] [%p][%p]",
		pbi,    /* base address of the pbi */
		pheap->ulOffset + ((uint32_t)BMEM_P_GetAddress(pheap, pbi) - pbi->ulFrontScrap - (uint32_t)pheap->pvHeap), /* start of memory allocation block */
		pbi->ulSize, /* size */
		pheap->ulOffset + ((uint32_t)BMEM_P_GetAddress(pheap, pbi) - pbi->ulFrontScrap - (uint32_t)pheap->pvHeap) + pbi->ulSize, /* end of memory allocation block */
		(unsigned long int)pbi->ulFrontScrap,
		(unsigned long int)pbi->ulSize,
		(unsigned long int)pbi->ulBackScrap,
		&pbi->stAddrNode,
		&pbi->stSizeNode));
}
#endif  /* BDBG_DEBUG_BUILD */

/**********************************************************************func*
 * BMEM_Dbg_DumpHeap
 *
 */
void BMEM_Dbg_DumpHeap(BMEM_Handle pheap)
{
#if BDBG_DEBUG_BUILD
	BMEM_P_BlockInfo *pbi;
	unsigned long ulFree = 0;

	BDBG_OBJECT_ASSERT(pheap, BMEM_Heap);

    BDBG_MODULE_LOG(BMEM_ALLOCATED, ("heap:%p(%#x):", pheap, (unsigned long)pheap->ulOffset));
    BDBG_MODULE_LOG(BMEM_ALLOCATED, ("offset,totalsize,filename,line"));
    BDBG_MSG((" addr,endaddr"));
    BDBG_MSG(("pbi[start..size..end]: [fscrap][totalsize][endscrap] [nextpbi]"));
    for(pbi = BLST_AA_TREE_FIRST(BMEM_P_AddrTree, &pheap->stAllocTopAddrTree);
        pbi != NULL;
        pbi = BLST_AA_TREE_NEXT(BMEM_P_AddrTree, &pheap->stAllocTopAddrTree, pbi))
    {
        BMEM_Dbg_DumpBlock(pheap, pbi, false);
    }

	BDBG_MODULE_LOG(BMEM_FREE, ("heap:%p(%#x):", pheap, (unsigned long)pheap->ulOffset));
	BDBG_MODULE_LOG(BMEM_FREE, ("offset,totalsize"));
	BDBG_MSG(("pbi[start..size..end]: [fscrap][totalsize][endscrap] [nextpbi]"));
	for(pbi = BLST_AA_TREE_FIRST(BMEM_P_SizeTree, &pheap->stFreeTopSizeTree);
		pbi != NULL;
		pbi = BLST_AA_TREE_NEXT(BMEM_P_SizeTree, &pheap->stFreeTopSizeTree, pbi))
	{
		ulFree += pbi->ulSize;
		BMEM_Dbg_DumpBlock(pheap, pbi, true);
	}

	BDBG_LOG(("Heap %p Total Size: %ld, Allocated: %ld, Free: %ld", pheap, pheap->zSize, pheap->ulTotalAllocated, ulFree));
#else
    BSTD_UNUSED(pheap);
#endif  /* BDBG_DEBUG_BUILD */
    return;
}

/**********************************************************************func*
 * BMEM_Dbg_GetErrorCount
 *
 */
uint32_t BMEM_Dbg_GetErrorCount(BMEM_Handle pheap)
{
    BDBG_OBJECT_ASSERT(pheap, BMEM_Heap);
	return pheap->ulNumErrors;
}

/**********************************************************************func*
 * BMEM_Dbg_ClearErrorCount
 *
 */
void BMEM_Dbg_ClearErrorCount(BMEM_Handle pheap)
{
    BDBG_OBJECT_ASSERT(pheap, BMEM_Heap);
	pheap->ulNumErrors = 0;
}

/* End of File */
