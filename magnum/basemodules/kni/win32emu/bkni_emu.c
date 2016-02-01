/***************************************************************************
*     Copyright (c) 2003, Broadcom Corporation
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

#include <string.h>
#include <stdlib.h>
#include "bstd.h"
#include "bmem.h"
#include "bkni.h"
#include "bkni_emu.h"

#undef BKNI_EMU_ReadMem
#undef BKNI_EMU_WriteMem
#undef BKNI_EMU_ReadReg
#undef BKNI_EMU_WriteReg

typedef struct PageEntry PageEntry;
typedef struct PageEntry
{
	DWORD      dwOldProtect;
	uintptr_t  uipAddress;
	PageEntry *pNext;
	bool       bStale;
};

static PageEntry *pReadOnlyHead = NULL;
static PageEntry *pReadWriteHead = NULL;

static SYSTEM_INFO g_SystemInfo;

static BKNI_EMU_Settings g_bemuSettings;

bool BKNI_EMU_P_PageMap( uintptr_t uipAddress, bool bWrite )
{
	PageEntry *pPageEntry;
	PageEntry *pTempEntry;
	DWORD      dwOldProtect;

	pPageEntry = malloc( sizeof (PageEntry) );
	if( !pPageEntry )
	{
		return false;
	}

	memset( pPageEntry, 0, sizeof *pPageEntry );

	pPageEntry->uipAddress = uipAddress & ~(g_SystemInfo.dwPageSize - 1);
	if( !VirtualProtect( (LPVOID) pPageEntry->uipAddress, 1, PAGE_READWRITE, &pPageEntry->dwOldProtect ) )
	{
		return false;
	}

	if( bWrite )
	{
		pPageEntry->pNext = pReadWriteHead;
		pReadWriteHead = pPageEntry;

		pTempEntry = pReadOnlyHead;
		while( pTempEntry )
		{
			if( pTempEntry->uipAddress == pPageEntry->uipAddress )
			{
				pPageEntry->dwOldProtect = pTempEntry->dwOldProtect;
				pTempEntry->bStale = true;
				return true;
			}
			pTempEntry = pTempEntry->pNext;
		}
	}
	else
	{
		pPageEntry->pNext = pReadOnlyHead;
		pReadOnlyHead = pPageEntry;
	}

	if ( g_bemuSettings.bemuReadHandler )
	{
		g_bemuSettings.bemuReadHandler(
			(uint32_t) (pPageEntry->uipAddress - (uintptr_t)g_bemuSettings.bmemAddress),
			(void *) pPageEntry->uipAddress,
			g_SystemInfo.dwPageSize );
	}

	if( !bWrite &&
		!VirtualProtect( (LPVOID) pPageEntry->uipAddress, 1, PAGE_READONLY, &dwOldProtect ) )
	{
		return false;
	}

	return true;
}

void BKNI_EMU_P_PageUnmapAll()
{
	PageEntry *pPageEntry;

	while( pReadWriteHead )
	{
		pPageEntry = pReadWriteHead;

		if ( g_bemuSettings.bemuWriteHandler )
		{
			g_bemuSettings.bemuWriteHandler(
				(uint32_t) (pPageEntry->uipAddress - (uintptr_t)g_bemuSettings.bmemAddress),
				(void *) pPageEntry->uipAddress,
				g_SystemInfo.dwPageSize );
		}

		VirtualProtect( (LPVOID) pPageEntry->uipAddress, 1, pPageEntry->dwOldProtect, &pPageEntry->dwOldProtect );

		pReadWriteHead = pPageEntry->pNext;
		free( pPageEntry );
	}

	while( pReadOnlyHead )
	{
		pPageEntry = pReadOnlyHead;

		if( !pPageEntry->bStale )
		{
			VirtualProtect( (LPVOID) pPageEntry->uipAddress, 1, pPageEntry->dwOldProtect, &pPageEntry->dwOldProtect );
		}

		pReadOnlyHead = pPageEntry->pNext;
		free( pPageEntry );
	}
}

void BKNI_EMU_Init( BKNI_EMU_Settings Settings )
{
	GetSystemInfo( &g_SystemInfo );
	BKNI_Memcpy( &g_bemuSettings, &Settings, sizeof(BKNI_EMU_Settings) );
}

void BKNI_EMU_Uninit()
{
	BKNI_EMU_P_PageUnmapAll();
}

int BKNI_EMU_MemoryHandler( LPEXCEPTION_POINTERS ExceptionPointers )
{
	PEXCEPTION_RECORD ExceptionRecord;
	ULONG_PTR         MemoryAddress;

	ExceptionRecord = ExceptionPointers->ExceptionRecord;

	if( ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION )
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	MemoryAddress = ExceptionRecord->ExceptionInformation[ 1 ];

	if( (MemoryAddress < (uintptr_t) g_bemuSettings.bmemAddress) ||
		(MemoryAddress >= ((uintptr_t) g_bemuSettings.bmemAddress + g_bemuSettings.bmemSize)) )
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	BKNI_EMU_P_PageMap( MemoryAddress, (bool)(ExceptionRecord->ExceptionInformation[ 0 ] != 0) );

	return EXCEPTION_CONTINUE_EXECUTION;
}