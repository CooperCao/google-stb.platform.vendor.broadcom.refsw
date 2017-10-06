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
 * Helper routines to find valid network adapters.
 *
 */

#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windef.h>
#include <devguid.h>

#include <aclapi.h>
#include <SetupAPI.h>
#include <CfgMgr32.h>
//#include <NewDev.h>

#include <wtypes.h>
#include <TCHAR.H>
#include "epictrl.h"
#include "epictrl_private.h"

extern WINERR get_mac_address(HANDLE m_dh, UCHAR macaddr[]);
extern WINERR get_description(HANDLE m_dh, LPTSTR desc, DWORD desclen);
extern DWORD get_adapter_type(HANDLE lirh, PADAPTER pdev);

#define SETUPDI_DLL		"setupapi.dll"

typedef HDEVINFO
(WINAPI *SetupDiGetClassDevsProc) (
    IN CONST GUID *ClassGuid,
    IN PCSTR       Enumerator,
    IN HWND        hwndParent,
    IN DWORD       Flags
    );

typedef WINSETUPAPI BOOL
(WINAPI *SetupDiEnumDeviceInfoProc) (
    IN  HDEVINFO         DeviceInfoSet,
    IN  DWORD            MemberIndex,
    OUT PSP_DEVINFO_DATA DeviceInfoData
    );

typedef BOOL
(WINAPI *SetupDiGetDeviceRegistryPropertyProc) (
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType,
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize
    );

typedef HKEY
(WINAPI *SetupDiOpenDevRegKeyProc) (
    IN HDEVINFO  DeviceInfoSet,
    IN PSP_DEVINFO_DATA  DeviceInfoData,
    IN DWORD  Scope,
    IN DWORD  HwProfile,
    IN DWORD  KeyType,
    IN REGSAM  samDesired
    );

typedef BOOL
(WINAPI *SetupDiDestroyDeviceInfoListProc) (
	HDEVINFO DeviceInfoSet
	);

typedef BOOL
(WINAPI *SetupDiSetClassInstallParamsProc) (
    IN HDEVINFO  DeviceInfoSet,
    IN PSP_DEVINFO_DATA  DeviceInfoData,
    IN PSP_CLASSINSTALL_HEADER  ClassInstallParams,
    IN DWORD  ClassInstallParamsSize
    );

typedef BOOL
(WINAPI *SetupDiCallClassInstallerProc) (
    IN DI_FUNCTION  InstallFunction,
    IN HDEVINFO  DeviceInfoSet,
    IN PSP_DEVINFO_DATA  DeviceInfoData
    );

static SetupDiGetClassDevsProc SetupDiGetClassDevsFn;
static SetupDiEnumDeviceInfoProc SetupDiEnumDeviceInfoFn;
static SetupDiGetDeviceRegistryPropertyProc SetupDiGetDeviceRegistryPropertyFn;
static SetupDiOpenDevRegKeyProc SetupDiOpenDevRegKeyFn;
static SetupDiDestroyDeviceInfoListProc SetupDiDestroyDeviceInfoListFn;
static SetupDiSetClassInstallParamsProc SetupDiSetClassInstallParamsFn;
static SetupDiCallClassInstallerProc SetupDiCallClassInstallerFn;

struct ProcEntry {
	PVOID addr;
	char *name;
};

static struct ProcEntry SetupDiProcs[] = {
	{ &SetupDiGetClassDevsFn,				"SetupDiGetClassDevsA"},
	{ &SetupDiEnumDeviceInfoFn,				"SetupDiEnumDeviceInfo" },
	{ &SetupDiGetDeviceRegistryPropertyFn,	"SetupDiGetDeviceRegistryPropertyA"},
	{ &SetupDiOpenDevRegKeyFn,				"SetupDiOpenDevRegKey" },
	{ &SetupDiDestroyDeviceInfoListFn,		"SetupDiDestroyDeviceInfoList" },
	{ &SetupDiSetClassInstallParamsFn,		"SetupDiSetClassInstallParamsA" },
	{ &SetupDiCallClassInstallerFn,			"SetupDiCallClassInstaller" },
	{ NULL,									 NULL }
};

static HANDLE hSetupDiDll = NULL;

static VOID ProcessNTAdapter(
	IN HKEY hKey,
	IN OUT PADAPTER pdev
)
{
	DWORD  Status;
	ULONG  ResultLength;
	BOOL bIsPhysicalAdapter = FALSE;
	DWORD dCharacteristics;

	ResultLength = sizeof(pdev->adaptername);
	Status = RegQueryValueEx(hKey, "NetCfgInstanceId", 0, NULL,
							 pdev->adaptername, &ResultLength);

	if (Status == ERROR_SUCCESS) {
		// Test to see if it is a physical adapter.
		Status = RegQueryValueEx(hKey, "Characteristics", 0, NULL, (LPBYTE)&dCharacteristics, &ResultLength);
		if (Status == ERROR_SUCCESS && (dCharacteristics & 0x4)) {
			/* Get the Component ID to match incase someone wants to enable a disabled device */
			ResultLength = sizeof(pdev->componentId);
			Status = RegQueryValueEx(hKey, "ComponentId", 0, NULL,
					 pdev->componentId, &ResultLength);
			if (Status == ERROR_SUCCESS)
				pdev->valid = TRUE;
		}
	}
}

static DWORD SetupDiLoad(void)
{
	DWORD rc = ERROR_SUCCESS;
	struct ProcEntry *p;

	do {
		hSetupDiDll = LoadLibrary(SETUPDI_DLL);

		if (hSetupDiDll == NULL) {
			rc = GetLastError();
			break;
		}

		for (p = &SetupDiProcs[0]; p->addr; p++) {
			*((PVOID*)(p->addr)) = GetProcAddress(hSetupDiDll, p->name);
			if (*((PVOID*)(p->addr)) == NULL) {
				rc = GetLastError();
				break;
			}
		}
	} while (0);

	return rc;
}

WINERR
Get9x2kAdapterList(
    HANDLE		ignore,
    ADAPTER		devlist[],
    PDWORD		maxdevs,
    BOOL		win9x,
    BOOL		ignore_disabled
)
{
	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
	SP_DEVINFO_DATA devInfoData;
	TCHAR driverString[256];
	PTCHAR instance;
	HKEY hKey;
	DWORD		Status = ERROR_SUCCESS;
    DWORD   ndevs = 0, index;
    PADAPTER pdev;
    HANDLE  lirh;  // local handle for duration of this routine.
	DWORD req;

    Status = ir_init(&lirh);
    if (Status != NO_ERROR) {
		printf(TEXT("Get9x2kAdapterList: SetupDiLoad failed w/error %d\n"), Status);
		return Status;
	}

	Status = SetupDiLoad();
	if (Status != ERROR_SUCCESS) {
		printf(TEXT("Get9x2kAdapterList: SetupDiLoad failed w/error %d\n"), Status);
		return Status;
	}

	hDevInfo = SetupDiGetClassDevsFn(&GUID_DEVCLASS_NET, NULL, 0, DIGCF_PRESENT);

	if (hDevInfo == INVALID_HANDLE_VALUE) {
		printf(TEXT("Get9x2kAdapterList: SetupDiGetClassDevs failed w/error: %x\n"), GetLastError());
		return GetLastError();
	}

	memset(&devInfoData, '\0', sizeof(SP_DEVINFO_DATA));
	devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	index = 0;
	while (SetupDiEnumDeviceInfoFn(hDevInfo, index, &devInfoData)) {

	    pdev = &devlist[ndevs];
	    memset(pdev, 0, sizeof(ADAPTER));
		pdev->valid = FALSE;
		req = SPDRP_FRIENDLYNAME;

retry:
		/* get friendlyname for WMI access routines */
		if (!SetupDiGetDeviceRegistryPropertyFn(hDevInfo, &devInfoData, req, NULL,
			(PBYTE) pdev->wminame, sizeof(pdev->wminame), NULL)) {
			if (req == SPDRP_FRIENDLYNAME) {
				req = SPDRP_DEVICEDESC;
				goto retry;
			} else
				goto cleanup;
		}

		if (!SetupDiGetDeviceRegistryPropertyFn(hDevInfo, &devInfoData, SPDRP_DRIVER, NULL,
			(PBYTE) driverString, sizeof(driverString), NULL))
				goto cleanup;

		/* SPDRP_DRIVER is the Net Class enumerator registry entry for the device */
		instance = NULL;
		if (instance = strrchr(driverString, '\\'))
			strcpy(pdev->regkey, instance + 1);

		hKey = SetupDiOpenDevRegKeyFn(hDevInfo, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
		if (hKey ==  INVALID_HANDLE_VALUE)
			goto cleanup;

		ProcessNTAdapter(hKey, pdev);

		RegCloseKey(hKey);

	    	if (pdev->valid) {
			if (ir_bind1(lirh, pdev->adaptername) == ERROR_SUCCESS) {
				ndevs++;
				get_mac_address(lirh, pdev->macaddr);
				get_description(lirh, pdev->description,
					sizeof(pdev->description));
				get_adapter_type(lirh, pdev);
				ir_unbind1(lirh);
			}
			else if (!ignore_disabled)
				ndevs++;
		}

		pdev->instance = index;

	    if (ndevs >= *maxdevs)
			break;

cleanup:
		devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		++index;
	}

	ir_exit(lirh);

    *maxdevs = ndevs;

	SetupDiDestroyDeviceInfoListFn(hDevInfo);

	if (hSetupDiDll != NULL)
		FreeLibrary(hSetupDiDll);
	hSetupDiDll = NULL;

	return Status;
}

WINERR
EnableAdapter(
    DWORD		index,
    BOOL		enable
)
{
	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
	SP_DEVINFO_DATA devInfoData;
	DWORD Status = ERROR_SUCCESS;
    DWORD ndevs = 0;
    SP_PROPCHANGE_PARAMS PropChangeParams = {sizeof(SP_CLASSINSTALL_HEADER)};

	Status = SetupDiLoad();
	if (Status != ERROR_SUCCESS) {
		printf(TEXT("EnableAdapter: SetupDiLoad failed w/error %d\n"), Status);
		return Status;
	}

	hDevInfo = SetupDiGetClassDevsFn(&GUID_DEVCLASS_NET, NULL, 0, DIGCF_PRESENT);

	if (hDevInfo == INVALID_HANDLE_VALUE) {
		printf(TEXT("EnableAdapter: SetupDiGetClassDevs failed w/error: %x\n"), GetLastError());
		return GetLastError();
	}

	memset(&devInfoData, '\0', sizeof(SP_DEVINFO_DATA));
	devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	if (SetupDiEnumDeviceInfoFn(hDevInfo, index, &devInfoData))	{
		PropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
		PropChangeParams.Scope = DICS_FLAG_GLOBAL;
		PropChangeParams.StateChange = enable ? DICS_ENABLE : DICS_DISABLE;
		PropChangeParams.HwProfile = 0;
		if (!SetupDiSetClassInstallParamsFn(hDevInfo, &devInfoData, (SP_CLASSINSTALL_HEADER *)&PropChangeParams,sizeof(PropChangeParams)))
			return GetLastError();

		PropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
		PropChangeParams.Scope = DICS_FLAG_CONFIGSPECIFIC;
		PropChangeParams.StateChange = enable ? DICS_ENABLE : DICS_DISABLE;
		PropChangeParams.HwProfile = 0;
		if (!SetupDiSetClassInstallParamsFn(hDevInfo, &devInfoData, (SP_CLASSINSTALL_HEADER *)&PropChangeParams,sizeof(PropChangeParams))
			|| !SetupDiCallClassInstallerFn(DIF_PROPERTYCHANGE, hDevInfo, &devInfoData))
			return GetLastError();
	}
	else
		return GetLastError();

	return Status;
}
