/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: preflibhlp.cpp,v 1.6 2008/05/06 21:43:28 Exp $
 */

#include <windows.h>
#include "wpshlp.h"
#include "prefstdefex.h"
#include "wps_win32_api.h"

/*******************************************************************************************

Preflib API Runtime DLL loading & linking


********************************************************************************************/

#define VER_UNKNOWN 0
#define VER_WIN9X 1
#define VER_WIN2K 2
#define VER_WINNT 3
#define VER_VISTA 4

#define PREFLIB_DLL_FILENAME TEXT("preflib.dll")

static HMODULE ghPreflibApi=NULL;

void LoadPreflibApi()
{
	if(wps_osl_wksp->os_ver != VER_VISTA && ghPreflibApi==NULL)
	{
		ghPreflibApi=LoadLibrary(PREFLIB_DLL_FILENAME);
	}
}

PVOID GetPreflibFunction(LPCSTR FunctionName)
{
	if(wps_osl_wksp->os_ver != VER_VISTA)
	{
		LoadPreflibApi();
		if(ghPreflibApi)
		{
			return GetProcAddress(ghPreflibApi, FunctionName); 
		}
	}

	return NULL;
}

void UnloadPreflibApi()
{
	if(wps_osl_wksp->os_ver != VER_VISTA && ghPreflibApi!=NULL)
	{
		FreeLibrary(ghPreflibApi);
		ghPreflibApi=NULL;
	}
}


/* Call this function before all others.  Supply PREF_VERSION for
   dwClientVersion, and zero for reserved. */
PN_STATUS BCMInitialize(DWORD dwClientVersion, LPPREF_VER_ST pVersion)
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)(DWORD , LPPREF_VER_ST );
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMInitialize")); 
	if(ProcAdd)
	{
		retval=ProcAdd(dwClientVersion, pVersion);
	}

	return retval;
}

/* Call this function to clean-up. */
PN_STATUS BCMUninitialize()
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)();
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMUninitialize")); 
	if(ProcAdd)
	{
		retval=ProcAdd();
	}

	return retval;
}

/* Fill in the preferred network list. If pList is NULL, pdwSize will
   return the size for the required list. In the case pList is not big
   enough, the return status will be PN_MORE_DATA */
PN_STATUS BCMGetPN(LPCTSTR adapter, LPCSTR sid, LPPREF_LIST_ST pList, PDWORD pdwSize)
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)(LPCTSTR adapter, LPCSTR sid, LPPREF_LIST_ST pList, PDWORD pdwSize);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMGetPN")); 
	if(ProcAdd)
	{
		retval=ProcAdd(adapter, sid, pList, pdwSize);
	}

	return retval;
}

/* Set the preferred networks */
PN_STATUS BCMSetPN(LPCTSTR adapter, LPCSTR sid, LPPREF_LIST_ST pList)
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)(LPCTSTR adapter, LPCSTR sid, LPPREF_LIST_ST pList);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMSetPN")); 
	if(ProcAdd)
	{
		retval=ProcAdd(adapter, sid, pList);
	}

	return retval;
}

/* Fill in the preferred network list. If pList is NULL, pdwSize will
   return the size for the required list. In the case pList is not big
   enough, the return status will be PN_MORE_DATA */
PN_STATUS BCMGetPNReg(HKEY hkey, BYTE* bzKey, LPPREF_LIST_ST pList, PDWORD pdwSize)
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)(HKEY, BYTE* bzKey, LPPREF_LIST_ST pList, PDWORD pdwSize);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMGetPNReg")); 
	if(ProcAdd)
	{
		retval=ProcAdd(hkey, bzKey, pList, pdwSize);
	}

	return retval;
}

/* Set the preferred networks */
PN_STATUS BCMSetPNReg(HKEY hkey, BYTE* bzKey, LPPREF_LIST_ST pList)
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)(HKEY hkey, BYTE* bzKey, LPPREF_LIST_ST pList);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMSetPNReg")); 
	if(ProcAdd)
	{
		retval=ProcAdd(hkey, bzKey, pList);
	}

	return retval;
}

/* Encrypt the given string, pString will contain the encrypted string 
   and pdwStringLength will be the length of this encrypted string on return.
   The return status is set to PN_ENCRYPTION_FAILED if encryption fails */
PN_STATUS BCMEncryptStr(char* pString, PDWORD pdwStringLength)
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)(char* pString, PDWORD pdwStringLength);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMEncryptStr")); 
	if(ProcAdd)
	{
		retval=ProcAdd(pString, pdwStringLength);
	}

	return retval;
}

PN_STATUS BCMTrayIsActive(LPCSTR adapter, BOOL* bActive)
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)(LPCSTR adapter, BOOL* bActive);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMTrayIsActive")); 
	if(ProcAdd)
	{
		retval=ProcAdd(adapter, bActive);
	}

	return retval;
}

PN_STATUS BCMTrayActivate(LPCSTR adapter, BOOL bActive)
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)(LPCSTR adapter, BOOL bActive);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMTrayActivate")); 
	if(ProcAdd)
	{
		retval=ProcAdd(adapter, bActive);
	}

	return retval;
}

/* Retreive the current status from the tray service */
PN_STATUS BCMTrayStatus(LPCSTR adapter, PDWORD pdwStatus)
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)(LPCSTR adapter, PDWORD pdwStatus);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMTrayStatus")); 
	if(ProcAdd)
	{
		retval=ProcAdd(adapter, pdwStatus);
	}

	return retval;
}

/* Register for callback status notifications. bIgnoreDuplicate indicates whether
   duplicate statuses should be sent or not. funcCallback is the client specified
   callback which will receive the notifications. pCallbackContext is a client
   specified context it can use to specify a hint. pReserved should be set to NULL */
PN_STATUS BCMRegisterStatusNotification(LPCSTR adapter, BOOL bIgnoreDuplicate, BCM_STATUS_NOTIFICATION_CALLBACK funcCallback, \
												PVOID pCallbackContext, PVOID pReserved)
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)(LPCSTR adapter, BOOL bIgnoreDuplicate, BCM_STATUS_NOTIFICATION_CALLBACK funcCallback, \
												PVOID pCallbackContext, PVOID pReserved);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMRegisterStatusNotification")); 
	if(ProcAdd)
	{
		retval=ProcAdd(adapter, bIgnoreDuplicate,funcCallback, pCallbackContext,pReserved);
	}

	return retval;
}

/* Unregister callback status notifications */
PN_STATUS BCMUnRegisterStatusNotification()
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)();
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMUnRegisterStatusNotification")); 
	if(ProcAdd)
	{
		retval=ProcAdd();
	}

	return retval;
}

/*  Install BCRM GINA DLL as a GINA DLL in the GINA chain.  This routine
    caches the name of the previous GINA DLL, if any, so it can be
    used for chaining and possibly restored later by the UninstallGINA
    routine.
    This routine only put our GINA DLL into the chain and doesn't enable it.
 	This routine does nothing if our GINA is already installed.
	It returns status PN_SUCCESS on sucess and PN_FAILED on failure. */
PN_STATUS BCMInstallGina()
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)();
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMInstallGina")); 
	if(ProcAdd)
	{
		retval=ProcAdd();
	}

	return retval;
}

/*	This routine uninstall BRCM GINA dll.  If BRCM GINA dll is not currently
    installed as a	GINA, this routine makes no changes. It always disable 
	the GINA functionality.
	It returns status PN_SUCCESS on sucess and PN_FAILED on failure. */
PN_STATUS BCMUninstallGina()
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)();
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMUninstallGina")); 
	if(ProcAdd)
	{
		retval=ProcAdd();
	}

	return retval;
}

/*	This routine enables/disbales BRCM GINA on the local machine.
	The GINA (BCMInstallGina) should be successfull install before 
	enabled to function	properly.
	It returns status PN_SUCCESS on sucess and PN_FAILED on failure. */
PN_STATUS BCMEnableGina(BOOL Enable)
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)(BOOL Enable);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMEnableGina")); 
	if(ProcAdd)
	{
		retval=ProcAdd(Enable);
	}

	return retval;
}

/* 	This routine determines whether BRCM GINA is installed and enabled on the
    local machine.  If it is both installed and enabled, then bEnabled is TRUE;
    otherwise, FALSE.
	It returns status PN_SUCCESS on sucess and PN_FAILED on failure. */
PN_STATUS BCMIsGinaEnabled(BOOL *bEnabled)
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)(BOOL *bEnabled);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMIsGinaEnabled")); 
	if(ProcAdd)
	{
		retval=ProcAdd(bEnabled);
	}

	return retval;
}

/* Based on the bPreserve flag, either preserve the PN's for future use or 
   load the preserved PN's back. */
PN_STATUS BCMPreservePN(LPCTSTR adapter, LPCSTR sid, BOOL bPreserve)
{
	PN_STATUS retval=PN_FAILED;
	typedef PN_STATUS (*ApiCall)(LPCTSTR adapter, LPCSTR sid, BOOL bPreserve);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetPreflibFunction(TEXT("BCMPreservePN")); 
	if(ProcAdd)
	{
		retval=ProcAdd(adapter, sid,bPreserve);
	}

	return retval;
}





LPCSTR g_sid = NULL;

DWORD g_dwPrefClientVersion = 0;
static BOOL g_bBCMInitialized = FALSE;

void InitializeBCM()
{
	if(g_bBCMInitialized)
		return;

	// Query Preflib/TrayApp version number
	BCMUninitialize();
	PREF_VER_ST ClientVer;
	BCMInitialize(0, &ClientVer);  // Query the number
	g_dwPrefClientVersion = ClientVer.dwHigh;

	// Initialize with the client preflib version number
	BCMUninitialize();
	BCMInitialize(g_dwPrefClientVersion, 0);
	g_bBCMInitialized = TRUE;
}

/*///////////////////////////////////////////////////////////////////////////
   Fill the preferred networks list
/////////////////////////////////////////////////////////////////////////////*/
PREF_LIST_ST* GetPreferredNetworksListEx(LPCSTR adapter)
{
	DWORD cbLen;
	PN_STATUS pnstatus;
	PREF_LIST_ST* pPrefList = NULL;
	InitializeBCM();

	cbLen = 0;
	pnstatus = BCMGetPN(adapter, g_sid, NULL, &cbLen);
	if (pnstatus == PN_MORE_DATA)
	{
		/* There are some preferred networks available 
		   allocate space to read them. */
		pPrefList = (PREF_LIST_ST*) malloc(cbLen);
		BCMGetPN(adapter, g_sid, pPrefList, &cbLen);
	}

	return pPrefList;
}


//***************************************************************************************
//*																						*
//* Function:	GetPreferredNetworksList												*
//* 																					*
//* Purpose:	Uses the PREFLIB libraries to return a list of all of the configured	*
//*				802.11 networks in the configuration registry.							*
//*				Please refer to the PREFLIB documentation for further details.			*
//*																						*
//* Returns:	List of preferred networks in a PREF_LIST_ST structure					*
//*																						*
//***************************************************************************************
SESHLPAPI PREF_LIST_ST* GetPreferredNetworksList(LPCSTR adapter, LPCSTR sid)
{
	g_sid=sid;
	InitializeBCM();
	return GetPreferredNetworksListEx(adapter);
}


//***************************************************************************************
//*																						*
//* Function:	AddPreferredNetwork														*
//* 																					*
//* Purpose:	Uses the PREFLIB libraries to add a new network into the preferred 		*
//*				network list.															*
//*				Please refer to the PREFLIB documentation for further details.			*
//*																						*
//* Returns:	List of preferred networks in a PREF_LIST_ST structure					*
//*																						*
//***************************************************************************************
PN_STATUS AddPreferredNetworkEx(LPCSTR adapter, LPPREF_ST pNewPref, int nIndex)
{
	if(adapter == NULL || pNewPref == NULL)
		return PN_INVALID_PARAMETER;

	DWORD cbLen = 0;
	DWORD dwNumNewList = 0;
	PREF_LIST_ST* pPrefList = NULL;
	PREF_LIST_ST* pPrefNewList = NULL;
	PN_STATUS pnStatus = PN_FAILED;

	InitializeBCM();

	// calculate the new list if it were going to be empty.
	// (this is added to below).
	cbLen = FIELD_OFFSET(PREF_LIST_ST, stPref);

	// Get the size of PREF_ST by version to dwSizeOfPref which is used to calculate the memory size when
	// merging the PN profile list 
	DWORD dwSizeOfPref = GetPrefStructureSizeByVer(g_dwPrefClientVersion);
	
	cbLen += dwSizeOfPref;  // Add in enough room for the new network.
	dwNumNewList++;  // Increment dwNumNewList by 1 for the new network to add

	pPrefList = GetPreferredNetworksListEx(adapter);  // Release pPrefList when it is done
	if (pPrefList)
	{ 
		// Add in the size of the existing networks.
		cbLen += pPrefList->dwNum * dwSizeOfPref;
		dwNumNewList += pPrefList->dwNum;  // Add number of profiles in existing list
	}

	// allocate the new network profile list
	pPrefNewList = (PREF_LIST_ST*) malloc(cbLen);
	ZeroMemory(pPrefNewList, cbLen);
	
	// Set client version number
	pPrefNewList->dwVersion = g_dwPrefClientVersion;  
	
	// Set total number of profiles 
	pPrefNewList->dwNum = dwNumNewList;

	// copy the new network info place. We need to convert the new profile to the format that existing client can 
	// accept. This is done by simply changing the dwLength member of profile structure PREF_ST as from inside
	// preflib, it uses to dwLength to validate whether the profile data is acceptable or not
	pNewPref->dwLength = dwSizeOfPref;  // Set strucutre size by version
	CopyMemory((PBYTE)(&pPrefNewList->stPref[0]), pNewPref, dwSizeOfPref);

	// copy existing profiles to new list
	if (pPrefList)
	{
		PBYTE pPNStart = (PBYTE)(&pPrefNewList->stPref[0]) + dwSizeOfPref;
		CopyMemory(pPNStart, (PBYTE)&pPrefList->stPref[nIndex], pPrefList->dwNum*dwSizeOfPref);
	}
		
	//  write the new list
	pnStatus = BCMSetPN(adapter, g_sid, pPrefNewList);
		
	free(pPrefNewList);
		
	if (pPrefList)
		free(pPrefList);
	
	return pnStatus;
}

SESHLPAPI PN_STATUS AddPreferredNetwork(LPCSTR adapter, LPCSTR sid, LPPREF_ST pNewPref, int nIndex)
{
	g_sid=sid;
	InitializeBCM();
	return AddPreferredNetworkEx(adapter, pNewPref, nIndex);
}

/*/////////////////////////////////////////////////////////////////////////
   Remove preferred network specified by an index.
   Pass in a negative index to delete all preferred networks.
///////////////////////////////////////////////////////////////////////////*/
PN_STATUS RemovePreferredNetworkEx(LPCSTR adapter, int nIndex)
{
	PREF_LIST_ST* pPrefList = NULL;
	PN_STATUS pnStatus = PN_FAILED;

	InitializeBCM();

	pPrefList = GetPreferredNetworksListEx(adapter);
	if (pPrefList)
	{
		if (nIndex < 0) {
			pPrefList->dwNum = 0;
		} else {
			if (nIndex >= (int) pPrefList->dwNum)
				nIndex = pPrefList->dwNum-1;
			
			if (nIndex+1 < (int) pPrefList->dwNum)
				/* Copy the first half of the new list. */
				CopyMemory((PBYTE)&pPrefList->stPref[nIndex], 
						   (PBYTE)&pPrefList->stPref[nIndex+1],
						   (pPrefList->dwNum-nIndex-1)*sizeof(PREF_ST_VER_7));
			
			/* The new list has one fewer elements than the old one. */
			pPrefList->dwNum--;
			
		}
		
		/* Set the new list. */
		pnStatus = BCMSetPN(adapter, g_sid, pPrefList);
		free(pPrefList);
	}

	return pnStatus;
}

//***************************************************************************************
//*																						*
//* Function:	RemovePreferredNetwork													*
//* 																					*
//* Purpose:	Uses the PREFLIB libraries to remove a new network into the preferred 	*
//*				network list.															*
//*				Please refer to the PREFLIB documentation for further details.			*
//*																						*
//* Returns:	List of preferred networks in a PREF_LIST_ST structure					*
//*																						*
//***************************************************************************************
SESHLPAPI PN_STATUS RemovePreferredNetwork(LPCSTR adapter, LPCSTR sid, DWORD nIndex)
{
	g_sid=sid;
	InitializeBCM();
	return RemovePreferredNetworkEx(adapter,nIndex);
}
///////////////////////////////////////////////////////////////////////////////////

SESHLPAPI BOOL FindPreferredNetwork(LPCSTR adapter, LPCSTR sid, LPCSTR ssid, DWORD *nIndex)
{
	BOOL ret = FALSE;
	PREF_LIST_ST* pPrefList;

	InitializeBCM();

	/* Get the current PN list */
	pPrefList = GetPreferredNetworksList(adapter, sid);
	if (pPrefList)
	{
		/* Walk through the list and locate the ssid */
		for (DWORD i=0; i < pPrefList->dwNum && !ret; i++)
		{
			if (strlen(ssid) == pPrefList->stPref[i].dwSsidLength)
				if (strncmp(pPrefList->stPref[i].cSsid, ssid, strlen(ssid)) == 0)
				{
					//TRACE(_T("FindPreferredNetwork located an ssid=%s at index=%d\n"), ssid, i);
					*nIndex = i;
					ret = TRUE;
				}
		}
		free(pPrefList);
	}
	return ret;
}
/*
#define SIZE_OF_PREF_ST(ver) 
*/
DWORD GetPrefStructureSizeByVer(const DWORD dwClientVer)
{
	DWORD dwSize = 0;
	switch(dwClientVer)
	{
	case 7:
		dwSize = sizeof(PREF_ST_VER_7);
		break;
	case 8:
		dwSize = sizeof(PREF_ST_VER_8);
		break;
	case 9:
		dwSize = sizeof(PREF_ST_VER_9);
		break;
	default:
		dwSize = 0;
	}
	return dwSize;
}
