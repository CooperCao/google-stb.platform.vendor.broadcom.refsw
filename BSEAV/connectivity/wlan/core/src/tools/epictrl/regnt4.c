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
#include "epictrl.h"

#define MAX_REG_PATH_LEN 512

 
static BOOLEAN ScanForWan(LPTSTR Data) 
{
	LPTSTR prohibited[] = { "WAN", "AsyMac", NULL };
	LPTSTR *pw;

	for (pw = &prohibited[0]; *pw; pw++) {
		if (strstr(Data, *pw) != NULL) 
			return FALSE;
	}
	return TRUE;
}


static BOOLEAN IsAdapterSafe(
	IN HKEY hKey
)
{
	DWORD		Status;
	char		Buffer[256];
	ULONG		ResultLength;
	BOOLEAN		IsOK = FALSE;

	// Query the description.  If it has the word "WAN" anywhere in 
	// it, bail.  We do not want to bind to any WAN adapters...
	//
	ResultLength = sizeof(Buffer);
	Status = RegQueryValueEx(hKey, "Description", 0, NULL, 
							 Buffer, &ResultLength);
	if ( Status == ERROR_SUCCESS ) {
		// Scan this descriptor for prohibited strings...
		//
		IsOK = ScanForWan(Buffer);
	}
	return IsOK;
}



static DWORD ProcessNTAdapter(
	IN HKEY hKey,
	IN OUT PADAPTER pdev
)
{
	DWORD  Status;
	ULONG  ResultLength;

	if (IsAdapterSafe(hKey) == FALSE) {
		return ERROR_INVALID_NAME;
	}
			
	ResultLength = sizeof(pdev->adaptername);
	Status = RegQueryValueEx(hKey, "ServiceName", 0, NULL, 
							 pdev->adaptername, &ResultLength);
	return Status;
}



#define REG_NETCARDS  "Software\\Microsoft\\Windows NT\\CurrentVersion\\NetWorkCards" 


WINERR SearchForNT4Adapters(
	IN DWORD index,
	IN OUT PADAPTER pdev
)
{
	DWORD		Status;
	HKEY		hKey1, hKey2;
	ULONG		ResultLength;
	FILETIME	ftLastWriteTime;

	Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_NETCARDS, 0, KEY_READ, &hKey1 );
	if ( Status == ERROR_SUCCESS ) {
		/* Iterate through all the network cards.  They will have simple 
		   numeric names like '1', '2', etc. */
		ResultLength = sizeof(pdev->regkey);
		Status = RegEnumKeyEx( hKey1, index, pdev->regkey, &ResultLength,
							   NULL, NULL, NULL, &ftLastWriteTime);
		if ( Status == ERROR_SUCCESS ) {
			Status = RegOpenKeyEx( hKey1, pdev->regkey, 0, KEY_READ, &hKey2 );
			if ( Status == ERROR_SUCCESS ) {
				Status = ProcessNTAdapter(hKey2, pdev);
				RegCloseKey( hKey2 );
			}
		}
		RegCloseKey( hKey1 );

	}

	return Status;
	
}
