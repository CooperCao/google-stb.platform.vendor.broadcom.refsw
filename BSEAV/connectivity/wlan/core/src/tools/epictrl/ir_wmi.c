/*
 * Copyright 2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 * $Id$
 *
 * Exposed API for the WMI <-> driver interface.  
 * Provides OID access to network adapters.
 *
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <wmistr.h>
#include <ndisguid.h>
#include "epiioctl.h"
#include "epictrl.h"
#include "ntddndis.h"        // This defines the IOCTL constants.
#include "wmiprocs.h"
#include "oidencap.h"
#include "wl_wmi.h"

#define WMI_DLL		"wmi.dll"

static BOOL ctrl_opened = FALSE;
static HMODULE hWmiDll = NULL;

static WmiOpenBlockProc WmiOpenBlock;
static WmiQueryAllDataProc WmiQueryAllData;
static WmiCloseBlockProc WmiCloseBlock;
static WmiQuerySingleInstanceProc WmiQuerySingleInstance;
static WmiSetSingleInstanceProc WmiSetSingleInstance;

struct ProcEntry {
	PVOID addr;
	char *name;
};

static struct ProcEntry WMIProcs[] = {
	{ &WmiOpenBlock,			"WmiOpenBlock"},
	{ &WmiQueryAllData,			"WmiQueryAllDataA" },
	{ &WmiCloseBlock,			"WmiCloseBlock"},
	{ &WmiQuerySingleInstance,	"WmiQuerySingleInstanceA" },
	{ &WmiSetSingleInstance,	"WmiSetSingleInstanceA" },
	{ NULL,						NULL }
};

// load library and get entry points since Win98 & Win2k 
// wmi.dll's differ
static DWORD WMILoad(void)
{
	DWORD rc = ERROR_SUCCESS;
	struct ProcEntry *p;

	do {
		hWmiDll = LoadLibrary(WMI_DLL);

		if (hWmiDll == NULL) {
			rc = GetLastError();
			break;
		}

		for (p = &WMIProcs[0]; p->addr; p++) {
			*((PVOID*)(p->addr)) = GetProcAddress(hWmiDll, p->name);
			if (*((PVOID*)(p->addr)) == NULL) {
				rc = GetLastError();
				break;
			}
		}	
	} while (0);
		
	return rc;
}

DWORD WMIOpen ( void )
{
	ULONG ret = ERROR_SUCCESS;

	if (!ctrl_opened && ((ret = WMILoad()) == ERROR_SUCCESS))
		ctrl_opened = TRUE;

	return ret;
}

VOID WMIClose (VOID)
{
	if (hWmiDll != NULL)
		FreeLibrary(hWmiDll);
	hWmiDll = NULL;
	ctrl_opened = FALSE;
}

#if defined(NOT_YET)
static DWORD
WmiGetSingleInstance( WMIHANDLE DataBlockHandle, LPCSTR friendlyname, 
					  DWORD *iobufsize, PVOID iobuf,
					  PVOID *dataptr)
{
	PWNODE_SINGLE_INSTANCE pNode;
	WNODE_TOO_SMALL toosmall;
	int size;
	DWORD ret;
	BOOLEAN client_pnode;

	if (dataptr != NULL && *iobufsize > sizeof(WNODE_SINGLE_INSTANCE)) {
		// try to use the client's buffer in place
		size = *iobufsize;
		pNode = (PWNODE_SINGLE_INSTANCE) iobuf;
		client_pnode = TRUE;
	} else {
		size = sizeof(toosmall);
		pNode = (PWNODE_SINGLE_INSTANCE) &toosmall;
	}

	ZeroMemory(pNode, size);

	ret = WmiQuerySingleInstance(
		DataBlockHandle, friendlyname, 
		&size, pNode);

	// win2k returns ERROR_INSUFFICIENT_BUFFER & Win98 ERROR_SUCCESS
	// For Win98, check Flags
	if (ret == ERROR_INSUFFICIENT_BUFFER || 
		!(pNode->WnodeHeader.Flags & WNODE_FLAG_SINGLE_INSTANCE)) {
		size += *iobufsize;
		*iobufsize = size;

		if (iobuf == NULL)
			return ERROR_MORE_DATA;	// consistent w/IOCTL I/F

		pNode = (PWNODE_SINGLE_INSTANCE) malloc(size);
		if (pNode == NULL)
			return ERROR_NOT_ENOUGH_MEMORY;

		client_pnode = FALSE;
		ret = WmiQuerySingleInstance(
			DataBlockHandle, friendlyname, 
			&size, (PVOID) pNode);
	}

	if (ret == ERROR_SUCCESS  && pNode->DataBlockOffset != 0) {
		PCHAR pdata = ((PCHAR)pNode) + pNode->DataBlockOffset;

		if (client_pnode) {
			*dataptr = pdata;
		} else {
			memcpy(iobuf, pdata, pNode->SizeDataBlock);
			if (dataptr != NULL)
				*dataptr = iobuf;
		}
		*iobufsize = pNode->SizeDataBlock;
	}

	if (!client_pnode)
		free(pNode);
	
	return ret;
}
#endif

DWORD WMICmd(BOOL setoid, LPCSTR friendlyname, ULONG oid, PVOID iobuf, 
		   DWORD *iobufsize)
{
	WMIHANDLE DataBlockHandle;
	DWORD ret;
	GUID setguid = GUID_BCM_SET;
	
	if (iobuf == NULL || *iobufsize < sizeof(setinformation_t))
		return ERROR_MORE_DATA;

	if (!setoid)
		return ERROR_NOT_SUPPORTED;

	if (!ctrl_opened) {
		if ((ret = WMIOpen()) != ERROR_SUCCESS)
			return ret;
	}

	ret = WmiOpenBlock(&setguid, STANDARD_RIGHTS_READ | WMIGUID_QUERY | WMIGUID_SET, &DataBlockHandle);

	if (ret == ERROR_SUCCESS) {
		if (setoid)
			ret = WmiSetSingleInstance( DataBlockHandle, friendlyname, 0,
										*iobufsize, (PVOID) iobuf);
#if defined(NOT_YET)
		else
			ret = WmiGetSingleInstance( DataBlockHandle, friendlyname, 
										iobufsize, (PVOID) iobuf, NULL);
#endif

		WmiCloseBlock(DataBlockHandle);
	}
	
	if (ret == ERROR_INSUFFICIENT_BUFFER)
		ret = ERROR_MORE_DATA;	// consistent w/IOCTL I/F

	return ret;
}
