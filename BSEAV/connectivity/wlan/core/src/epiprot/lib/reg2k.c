//  
//  Copyright 1999, Broadcom Corporation
//  All Rights Reserved.
//  
//  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
//  the contents of this file may not be disclosed to third parties, copied or
//  duplicated in any form, in whole or in part, without the prior written
//  permission of Broadcom Corporation.
//  
//  registry.c - routines for finding adapters in the registry.  These routines 
//  are used for all versions of Win2K and NT4.
//

#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <wtypes.h>
#include <TCHAR.H>
#include "epictrl.h" //#include "bcmpdlib.h"

#include <iphlpapi.h>


#define ARRAYSIZE(a)   (sizeof(a)/sizeof(a[0]))

typedef DWORD (CALLBACK* GetAdaptersInfo_t)(PIP_ADAPTER_INFO, PULONG);
typedef DWORD (CALLBACK* GetIfEntry_t)( PMIB_IFROW);


HANDLE hModule = NULL;
GetAdaptersInfo_t GetAdaptersInfoProc = NULL;
GetIfEntry_t GetIfEntryProc = NULL;

BOOL LoadIpHlpApi()
{
    if ( GetAdaptersInfoProc == NULL ) {
	if (hModule == NULL) {
	    hModule = LoadLibrary(_T("IpHlpApi"));
	    if (hModule) {

		GetIfEntryProc = (GetIfEntry_t) GetProcAddress(hModule, "GetIfEntry"); 
		GetAdaptersInfoProc = (GetAdaptersInfo_t) GetProcAddress(hModule, "GetAdaptersInfo"); 
		if (GetAdaptersInfoProc == NULL || GetIfEntryProc == NULL) {
		    GetIfEntryProc = NULL;
		    GetAdaptersInfoProc = NULL;
		    FreeLibrary(hModule);
		    hModule = NULL;
		}
	    }
	}
    }
    return (GetAdaptersInfoProc != NULL);
}


DWORD SearchForNT5Adapters(
	IN DWORD index,
	IN OUT PADAPTER pdev
)
{
    DWORD Status = ERROR_NO_MORE_ITEMS;
    PIP_ADAPTER_INFO pAdapterInfoList, pInfo;
    MIB_IFROW IfRow;
    DWORD needlen;

    if (LoadIpHlpApi()) {
	needlen = 0;
	if (GetAdaptersInfoProc(NULL, &needlen) == ERROR_BUFFER_OVERFLOW) {
	    pAdapterInfoList = (PIP_ADAPTER_INFO) malloc(needlen);
	    if (GetAdaptersInfoProc(pAdapterInfoList, &needlen) == NO_ERROR) {
		for (pInfo = pAdapterInfoList; pInfo; pInfo = pInfo->Next) {
		    if (index-- == 0) {
			IfRow.dwIndex = pInfo->Index;
			if (GetIfEntryProc(&IfRow) || IfRow.dwAdminStatus != MIB_IF_ADMIN_STATUS_UP)
			    continue;
			pdev->instance = pInfo->Index;
			memcpy(pdev->macaddr, pInfo->Address, sizeof(pdev->macaddr) );
#ifdef _UNICODE
			MultiByteToWideChar( CP_ACP, 0, 
					     pInfo->AdapterName, strlen(pInfo->AdapterName),
					     pdev->adaptername, ARRAYSIZE(pdev->adaptername));
			MultiByteToWideChar( CP_ACP, 0, 
					     pInfo->Description, strlen(pInfo->Description),
					     pdev->description, ARRAYSIZE(pdev->description));
#else
			strncpy(pdev->adaptername, pInfo->AdapterName, sizeof(pdev->adaptername) );
			strncpy(pdev->description, pInfo->Description, sizeof(pdev->description));
#endif
			Status = NO_ERROR;
			break;
		    }
		}
	    }
	    free(pAdapterInfoList);
	}
    }
    
    return Status;
}
