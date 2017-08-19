/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: brcmmgmt.cpp,v 1.2 2008/05/01 23:13:19 Exp $
 */

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include "wpshlp.h"
#include "epictrl.h" //#include "bcmpdlib.h"
#include "wlioctl.h"

#include "Winsvc.h"

#define WZCSVC				_T("WZCSVC")

#define WZC_ISACTIVE(cf)	((cf&0x8000)!=0)
#define WZC_SETACTIVE(cf)	(cf |= 0x8000)
#define WZC_CLRACTIVE(cf)	(cf &= ~0x8000)

#define WZC_RUNNING			(1<<0)
#define WZC_STOPPED			(1<<1)

#define WZC_ACTIVE			(1<<2)
#define WZC_INACTIVE		(1<<3)

DWORD	WZC(LPCTSTR szIfName, PDWORD pdwFlags);

#define WLTRYSVC	_T("WLTRYSVC")

class Service {

 private:
    static int s_bInitialized;
    static const int NUMBER_OF_PENDING_ATTEMPTS;

    /* function pointers from advapi32.dll */
    static SC_HANDLE (PASCAL *OpenSCManagerA)(LPCSTR,LPCSTR,DWORD);
    static SC_HANDLE (PASCAL *OpenServiceA)(SC_HANDLE,LPCSTR,DWORD);
    static BOOL (PASCAL *StartServiceA)(SC_HANDLE,DWORD,LPCSTR*);
    static BOOL (PASCAL *QueryServiceStatus)(SC_HANDLE,LPSERVICE_STATUS);
    static BOOL (PASCAL *CloseServiceHandle)(SC_HANDLE);
    static BOOL (PASCAL *ControlService)(SC_HANDLE,DWORD,LPSERVICE_STATUS);

 public:
    static BOOL	Stop(LPCTSTR lpServiceName);
    static BOOL	Start(LPCTSTR lpServiceName);
    static BOOL	IsRunning(LPCTSTR lpServiceName);
    static BOOL	Initialized();
    
};

BOOL SESHLPAPI
GetRadioState(LPCTSTR szIfName, PDWORD pdwState)
{
	HANDLE h;
	ULONG status;
	DWORD inoutlen;
	BOOL ret = FALSE;

	if (ir_init(&h) == NO_ERROR && 
		h != INVALID_HANDLE_VALUE)
	{
		if (ir_bind(h, szIfName) == NO_ERROR)
		{
			status = 0;
			inoutlen = sizeof(status);
			if (ir_queryinformation(h, WL_OID_BASE + WLC_GET_RADIO, (PUCHAR)&status, &inoutlen) == NO_ERROR)
			{
				*pdwState = 0;
				*pdwState |= status & WL_RADIO_SW_DISABLE ? RADIOSTATE_SW_DISABLE : 0;
				*pdwState |= status & WL_RADIO_HW_DISABLE ? RADIOSTATE_HW_DISABLE : 0;
				ret = TRUE;
			}
			ir_unbind(h);
		}
		ir_exit(h);
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////

BOOL SESHLPAPI
SetRadioState(LPCTSTR szIfName, DWORD dwState)
{
	HANDLE h;
	int status;
	DWORD inoutlen;
	BOOL ret = FALSE;

	if (ir_init(&h) == NO_ERROR && 
		h != INVALID_HANDLE_VALUE)
	{
		if (ir_bind(h, szIfName) == NO_ERROR)
		{
			status = (WL_RADIO_SW_DISABLE) << 16;
			status |= dwState & RADIOSTATE_SW_DISABLE ? WL_RADIO_SW_DISABLE : 0;
			status |= dwState & RADIOSTATE_HW_DISABLE ? WL_RADIO_HW_DISABLE : 0;
			inoutlen = sizeof(status);
			if (ir_setinformation(h, WL_OID_BASE + WLC_SET_RADIO, (PUCHAR)&status, &inoutlen) == NO_ERROR)
				ret = TRUE;

			ir_unbind(h);
		}
		ir_exit(h);
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////

#define WZCCONFIGKEY "SOFTWARE\\Microsoft\\WZCSVC\\Parameters\\Interfaces"

/* Manage the Windows Zero Config Service (WZC).
   This routine controls whether the WZC service is running and
   if it is running, whether WZC also controls the wireless interface. 
*/
static DWORD WZC(LPCTSTR szIfName, PDWORD pdwFlags)
{
	BOOL wzcIsRunning = Service::IsRunning(WZCSVC);
	DWORD ret = 0;
	DWORD wzcFlags;
	DWORD dwSize;
	DWORD dwType;
	WINERR winerr;
	HKEY hKey;
	TCHAR szSubKey[256];

	if (pdwFlags)
	{
		DWORD& dwFlags = *pdwFlags;
		assert((*pdwFlags&(WZC_ACTIVE|WZC_INACTIVE)) != (WZC_ACTIVE|WZC_INACTIVE));
		assert((*pdwFlags&(WZC_RUNNING|WZC_STOPPED)) != (WZC_RUNNING|WZC_STOPPED));
		//TRACE("WZC(0x%x)\n", dwFlags);
		//TRACE("%s", (dwFlags & WZC_STOPPED) ? "WZC_STOPPED" : "WZC_RUNNING");
		//TRACE("|%s\n", (dwFlags & WZC_ACTIVE) ? "WZC_ACTIVE" : "WZC_INACTIVE");
	}

	ret |= wzcIsRunning ? WZC_RUNNING : WZC_STOPPED;

	_stprintf(szSubKey, WZCCONFIGKEY "\\%s", szIfName);
	if (szIfName)
	{
	/* fetch the control flag value that zero config stores 
	   in the registry. */
	winerr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_READ, &hKey);
	if (winerr == ERROR_SUCCESS)
	{
		dwType = REG_DWORD; dwSize = sizeof(wzcFlags);
		winerr = RegQueryValueEx(hKey, "ControlFlags", 0, &dwType, (LPBYTE)&wzcFlags, &dwSize);
		RegCloseKey(hKey);
	}
	if (winerr != ERROR_SUCCESS)
	{
		//TRACE("Unable to access windows zero config store\n");
		wzcFlags = 0x8002;  /* magic value, experimentally derived. */
	}
				}
	//TRACE("wzcFlags = 0x%x WZC_ISACTIVE=%d\n", wzcFlags, WZC_ISACTIVE(wzcFlags));

	ret |= WZC_ISACTIVE(wzcFlags) ? WZC_ACTIVE : WZC_INACTIVE;

	if (pdwFlags)
	{
		DWORD& dwFlags = *pdwFlags;
		/* ACTIVE/INACTIVE mean zero config is MANAGING/NOT MANAGING the
		   wireless adapter. */
		if ((dwFlags & WZC_ACTIVE) && !WZC_ISACTIVE(wzcFlags)) {
			Service::Stop(WZCSVC);
			WZC_SETACTIVE(wzcFlags);
			//TRACE("WZC_SETACTIVE\n");
			winerr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, WZCCONFIGKEY, 0, KEY_WRITE, &hKey);
			if (winerr != NO_ERROR)
			{
				dwType = REG_DWORD; dwSize = sizeof(wzcFlags);
				winerr = RegSetValueEx(hKey, szSubKey, 0, dwType, (LPBYTE)&wzcFlags, dwSize);
				RegCloseKey(hKey);
			}
			if (winerr != NO_ERROR) {
				//TRACE("SetDWORDValue failed - %d\n", winerr);
			}
		} else if (!(dwFlags & WZC_ACTIVE) && WZC_ISACTIVE(wzcFlags)) {
			Service::Stop(WZCSVC);
			WZC_CLRACTIVE(wzcFlags);
			//TRACE("WZC_CLRACTIVE\n");
			winerr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, WZCCONFIGKEY, 0, KEY_WRITE, &hKey);
			if (winerr != NO_ERROR)
			{
				dwType = REG_DWORD; dwSize = sizeof(wzcFlags);
				winerr = RegSetValueEx(hKey, szSubKey, 0, dwType, (LPBYTE)&wzcFlags, dwSize);
				RegCloseKey(hKey);
			}
			if (winerr != NO_ERROR) {
				//TRACE("SetDWORDValue failed - %d\n", winerr);
			}
		}

		if (dwFlags & WZC_STOPPED) {
			Service::Stop(WZCSVC);
		} else if ((dwFlags & WZC_RUNNING) || wzcIsRunning) {
			Service::Start(WZCSVC);
		}
	}

	return ret;
}

/////////////////////////////////////////////////////////////////////////////

#define KEY_NET    "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002bE10318}"

/* This routine opens an area in the registry where the advanced tab in
   device properties puts its data.  Note that this is a different
   area from where we store private application data. */
static HKEY OpenDriverRegKey(LPCSTR adapter, REGSAM samDesired) 
{
	FILETIME ftLastWriteTime;
	char szKeyName[10];
	char szInstanceId[60];
	DWORD status, cbData, dwIndex;
	HKEY hNetClass;				/* Handle to Net Class data */
	HKEY hNet;					/* Handle to a specific net instance */

	hNet = (HKEY) INVALID_HANDLE_VALUE;
	status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY_NET, 0, KEY_READ, &hNetClass);
    if (status == ERROR_SUCCESS) {
		dwIndex = 0;
		cbData = sizeof(szKeyName);
		status = RegEnumKeyEx(hNetClass, dwIndex, szKeyName, &cbData,
							  NULL, NULL, NULL, &ftLastWriteTime);
		while (status == ERROR_SUCCESS) {
			szKeyName[cbData] = '\0';
			status = RegOpenKeyEx(hNetClass, szKeyName, 0, samDesired, &hNet);
			if (status == ERROR_SUCCESS) {
				cbData = sizeof(szInstanceId);
				status = RegQueryValueEx(hNet, "NetCfgInstanceId", NULL,
										 NULL, (LPBYTE)szInstanceId, &cbData); 
				if (status == ERROR_SUCCESS) {
					szInstanceId[cbData] = '\0';
					if (strcmp(szInstanceId, adapter) == 0) {
						break;
					}
				}
				RegCloseKey(hNet);
				hNet = (HKEY) INVALID_HANDLE_VALUE;
			}

			dwIndex++;
			cbData = sizeof(szKeyName);
			status = RegEnumKeyEx(hNetClass, dwIndex, szKeyName, &cbData,
								  NULL, NULL, NULL, &ftLastWriteTime);
		}
		RegCloseKey(hNetClass);
	}

    return hNet;
}

static BOOL setManagementState(LPCSTR adapter, BOOL bManaged)
{
	HKEY hNet;
	char *lpManaged;
	DWORD status;
	BOOL bRet = FALSE;

	if (!adapter) 
		return FALSE;

	hNet = OpenDriverRegKey(adapter, KEY_ALL_ACCESS);
	if (hNet != (HKEY) INVALID_HANDLE_VALUE) {
		if (bManaged)
			lpManaged = "1";
		else 
			lpManaged = "0";

		status = RegSetValueEx(hNet, "Managed", NULL, REG_SZ, (LPBYTE)lpManaged, (DWORD)strlen(lpManaged));
		if (status == ERROR_SUCCESS)
			bRet = TRUE;

		RegCloseKey(hNet);
	}

	return bRet;
}

static BOOL getManagementState(LPCSTR adapter)
{
	HKEY hNet;
	char szManaged[5];
	DWORD cbData, status;
	char *endptr;
	BOOL bManaged = FALSE;
	int val;

	if (!adapter) 
		return FALSE;

	hNet = OpenDriverRegKey(adapter, KEY_READ);
	if (hNet != (HKEY) INVALID_HANDLE_VALUE) {
		cbData = sizeof(szManaged);
		status = RegQueryValueEx(hNet, _T("Managed"), NULL,
								 NULL, (LPBYTE)szManaged, &cbData); 
		if (status == ERROR_SUCCESS) {
			val = strtol(szManaged, &endptr, 0);
			if (*endptr == '\0') {
				bManaged = (val ? TRUE : FALSE);
			}
		}
		RegCloseKey(hNet);
	}

	return bManaged;
}

/////////////////////////////////////////////////////////////////////////////

BOOL SESHLPAPI
GetManagementState(LPCTSTR szIfName, PDWORD pdwState)
{
	BOOL ret = FALSE;
	DWORD wzcState = 0;
	WORD managed = 0, status = 0;

	if (pdwState)
	{
	status |= Service::IsRunning(WLTRYSVC) ? MGMTSTATE_RUN_BCM : 0;
	status |= Service::IsRunning(WZCSVC) ? MGMTSTATE_RUN_WZC : 0;

	if (szIfName)
	{
		wzcState = WZC(szIfName, NULL);
		if (status & MGMTSTATE_RUN_BCM && getManagementState(szIfName))
			managed = MGMTSTATE_MANAGE_BCM;
		else if (wzcState & WZC_RUNNING && wzcState & WZC_ACTIVE)
			managed = MGMTSTATE_MANAGE_WZC;
		else
			managed = MGMTSTATE_MANAGE_NONE;

		ret = TRUE;
	}
	else
		ret = TRUE;

	*pdwState = MAKELONG(managed, status);
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////

BOOL SESHLPAPI
SetManagementState(LPCTSTR szIfName, DWORD dwState)
{
	BOOL ret = FALSE;
	DWORD wzcState = 0;

	if (szIfName)
	{
		switch (LOWORD(dwState))
		{
		case MGMTSTATE_MANAGE_NONE:
			wzcState = WZC(szIfName, NULL);
			wzcState &= ~WZC_ACTIVE;
			wzcState |= WZC_INACTIVE;
			WZC(szIfName, &wzcState);
			setManagementState(szIfName, FALSE);
			ret = TRUE;
			break;
		case MGMTSTATE_MANAGE_BCM:
			setManagementState(szIfName, TRUE);
			if (!Service::IsRunning(WLTRYSVC))
				Service::Start(WLTRYSVC);
			ret = TRUE;
			break;
		case MGMTSTATE_MANAGE_WZC:
			setManagementState(szIfName, FALSE);
			wzcState |= (WZC_ACTIVE | WZC_RUNNING);
			WZC(szIfName, &wzcState);
			ret = TRUE;
			break;
		}
	}
	else
	{
		switch (LOWORD(dwState))
		{
		case MGMTSTATE_MANAGE_NONE:
			// Always start WZC first
			if (HIWORD(dwState) & MGMTSTATE_RUN_WZC)
				if (!Service::IsRunning(WZCSVC))
					Service::Start(WZCSVC);
			if (HIWORD(dwState) & MGMTSTATE_RUN_BCM)
				if (!Service::IsRunning(WLTRYSVC))
					Service::Start(WLTRYSVC);

			// Always stop BCM first
			if (!(HIWORD(dwState) & MGMTSTATE_RUN_BCM))
			if (Service::IsRunning(WLTRYSVC))
				Service::Stop(WLTRYSVC);
			if (!(HIWORD(dwState) & MGMTSTATE_RUN_WZC))
				if (Service::IsRunning(WZCSVC))
					Service::Stop(WZCSVC);

			ret = TRUE;
			break;
		case MGMTSTATE_MANAGE_BCM:
			if (!Service::IsRunning(WLTRYSVC))
				Service::Start(WLTRYSVC);
			ret = TRUE;
			break;
		case MGMTSTATE_MANAGE_WZC:
			if (Service::IsRunning(WLTRYSVC))
				Service::Stop(WLTRYSVC);
			Service::Start(WZCSVC);
			ret = TRUE;
			break;
		}
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////

// These defines taken from "include/wlregstr.h"
#define REGSTR_PATH_APP 		TEXT("SOFTWARE\\Broadcom\\802.11")
#define REGSTR_VAL_PERUSERPN	TEXT("PerUserPN")

BOOL SESHLPAPI
IsUserPN(BOOL* pbResult)
{
	HKEY hKey;
	DWORD dwLength;
	DWORD dwPN = 0;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_APP, 0, KEY_READ, &hKey) == NO_ERROR)
	{
		DWORD dwType = REG_DWORD;
		dwLength = sizeof(dwLength);
		if (RegQueryValueEx(hKey, REGSTR_VAL_PERUSERPN, 0, &dwType, (LPBYTE)&dwPN, &dwLength) == NO_ERROR)
			*pbResult = dwPN;
		RegCloseKey(hKey);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////
