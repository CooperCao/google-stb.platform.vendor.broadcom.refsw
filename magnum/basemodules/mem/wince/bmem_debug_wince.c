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
#include "blst_aa_tree.h"

#if defined(_WIN32_WCE)
#include <windows.h>
#endif

BDBG_MODULE(BMEM_DEBUG_WINCE);

#define BMEM_MAP_SIZE       (80*40)
#define BMEM_MAP_ADDR(addr) ((addr) * BMEM_MAP_SIZE / (pheap->pEnd - pheap->pStart))

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

	for(pbi=BLST_AA_TREE_FIRST(BMEM_P_AddrTree, &pheap->stAllocTopAddrTree);
		pbi != NULL;
		pbi=BLST_AA_TREE_NEXT(BMEM_P_AddrTree, &pheap->stAllocTopAddrTree, pbi))
	{
		uintptr_t addrStart = BMEM_P_GetAddress( pheap, pbi ) - pbi->ulFrontScrap -
			(uintptr_t) pheap->pStart;
		uintptr_t addrFront = BMEM_P_GetAddress(pheap, pbi) - (uintptr_t)pheap->pStart;
		uintptr_t addrBack = addrStart+pbi->ulSize-pbi->ulBackScrap;
		uintptr_t addrEnd = addrStart+pbi->ulSize;
		uintptr_t ul;

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
		uintptr_t addrStart = BMEM_P_GetAddress( pheap, pbi ) - pbi->ulFrontScrap -
			(uintptr_t) pheap->pStart;
		uintptr_t addrEnd = addrStart + pbi->ulSize;
		uintptr_t ul;

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
	RETAILMSG(1, (L"%S\n", ach));
	BKNI_Free((char *)ach);
}

/**********************************************************************func*
 * BMEM_Dbg_DumpBlock_Verbose
 *
 */
void BMEM_Dbg_DumpBlock_Verbose(BMEM_Handle pheap, BMEM_P_BlockInfo *pbi)
{
	const char *filename = NULL;
	unsigned linenum = 0;

#if defined(_WIN32_WCE)
    __try {
#endif

#ifdef BMEM_TRACK_FILE_AND_LINE
	filename = pbi->pchFilename;
	if(pheap->pSafetyConfigInfo->bTrackFileAndLine) {
		/* eliminate the path which is usually unnecessary and very long */
		while (*filename) filename++;
		while (*filename != '/' && filename > pbi->pchFilename) filename--;
		if (*filename == '/') filename++;
	    linenum = pbi->ulLine;
	}
#endif

#ifdef BMEM_TRACK_EXCEL_STYLE

	/* This format is directly importable to Excel where you can sort by address. 
	It is not DBG output. It should be defaulted off because it will always print. */
	{
	uint32_t ulFrontBookKeepingSize = (pheap->eBookKeeping == BMEM_BOOKKEEPING_LOCAL) ?
	                                  BMEM_FRONT_BOOKKEEPING_SIZE_LOCAL : BMEM_FRONT_BOOKKEEPING_SIZE_SYSTEM;
	uint8_t *addr = BMEM_P_GetAddress( pheap, pbi ) + (uintptr_t)ulFrontBookKeepingSize;

		if(pheap->pSafetyConfigInfo->bTrackFileAndLine) {
			BKNI_Printf("%p,%#x,%#x,%#x,%lu,%s,%d\n",
        		pheap,
        		pheap->ulOffset,
        		addr,
        		addr + pbi->ulSize,
        		(unsigned long int)pbi->ulSize,
        		filename,
        		linenum);
	}

#else /* BMEM_TRACK_EXCEL_STYLE */

#ifdef BMEM_TRACK_FILE_AND_LINE
	if(pheap->pSafetyConfigInfo->bTrackFileAndLine)
	{
		RETAILMSG(1, (L"(%S:%d) ", filename, pbi->ulLine));
	}
#endif

    /* The front scrap area comes before the pbi data structure */
	RETAILMSG(1, (L"%p[%#x..%#x..%#x]: [%lu][%lu][%lu]  [%p] [%p]\n",
		pbi,    /* base address of the pbi */
		pheap->ulOffset + ((uint32_t)BMEM_P_GetAddress(pheap, pbi) - pbi->ulFrontScrap - (uint32_t)pheap->pvHeap), /* start of memory allocation block */
		pbi->ulSize, /* size */
		pheap->ulOffset + ((uint32_t)BMEM_P_GetAddress(pheap, pbi) - pbi->ulFrontScrap - (uint32_t)pheap->pvHeap) + pbi->ulSize, /* end of memory allocation block */
		(unsigned long int)pbi->ulFrontScrap,
		(unsigned long int)pbi->ulSize,
		(unsigned long int)pbi->ulBackScrap,
		&pbi->stAddrNode,
		&pbi->stSizeNode));

#endif
#if defined(_WIN32_WCE)
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    	RETAILMSG(1, (L"Invalid PBI=%p\n", pbi));
    }
#endif

    BSTD_UNUSED(pheap);
    BSTD_UNUSED(pbi);
}

/**********************************************************************func*
 * BMEM_Dbg_DumpHeap
 *
 */
void BMEM_Dbg_DumpHeap(BMEM_Handle pheap)
{
	BMEM_P_BlockInfo *pbi;
    unsigned long ulFree = 0;


	RETAILMSG(1, (L"Allocated:\n"));
#ifdef BMEM_TRACK_EXCEL_STYLE
		RETAILMSG(1,(L"heap,offset,addr,endaddr,totalsize,filename,line\n"));
#else
		RETAILMSG(1,(L"pbi[start..size..end]: [fscrap][totalsize][endscrap] [nextpbi]"));
#endif
	for(pbi=BLST_AA_TREE_FIRST(BMEM_P_AddrTree, &pheap->stAllocTopAddrTree);
		pbi != NULL;
		pbi=BLST_AA_TREE_NEXT(BMEM_P_AddrTree, &pheap->stAllocTopAddrTree, pbi))
	{
		BMEM_Dbg_DumpBlock_Verbose(pheap, pbi);
	}

	RETAILMSG(1,(L"Free:\n"));
#ifdef BMEM_TRACK_EXCEL_STYLE
	RETAILMSG(1,(L"heap,offset,addr,endaddr,totalsize,filename,line\n"));
#else
	RETAILMSG(1,(L"pbi[start..size..end]: [fscrap][totalsize][endscrap] [nextpbi]"));
#endif
	for(pbi=BLST_AA_TREE_FIRST(BMEM_P_SizeTree, &pheap->stFreeTopSizeTree);
		pbi != NULL;
		pbi=BLST_AA_TREE_NEXT(BMEM_P_SizeTree, &pheap->stFreeTopSizeTree, pbi))
	{
        ulFree += pbi->ulSize;
		BMEM_Dbg_DumpBlock_Verbose(pheap, pbi);
	}

	RETAILMSG(1, (L"Heap Summary:"));
	RETAILMSG(1, (L"Total Size: %ld, Allocated: %ld, Free: %ld\n", pheap->zSize, pheap->ulTotalAllocated, ulFree));
}

/**********************************************************************func*
 * BMEM_Dbg_GetErrorCount
 *
 */
uint32_t BMEM_Dbg_GetErrorCount(BMEM_Handle pheap)
{
	return pheap->ulNumErrors;
}

/**********************************************************************func*
 * BMEM_Dbg_ClearErrorCount
 *
 */
void BMEM_Dbg_ClearErrorCount(BMEM_Handle pheap)
{
	pheap->ulNumErrors = 0;
}

/* End of File */
