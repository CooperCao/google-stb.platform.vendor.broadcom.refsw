/*
 * Copyright 2001, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 * $Id$
 *
 * Exposed API for the bcmrly library under Windows CE.  
 * Provides OID access to network adapters.
 *
 */
 
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <tchar.h>
#include <ndis.h>
#include <nuiouser.h>

#include "epictrl.h"
#include "assert.h"

#ifndef NDISUIO_DEVICE_NAME
#define NDISUIO_DEVICE_NAME TEXT("UIO1:")
#endif

static void *driver_name[] = {
	/* DHD(dongle) SDIO device */
	{TEXT("BCMSDDHD1")},
	{TEXT("BCMSDDHD2")},
	/* POCKET PC SDIO device */
	{TEXT("BCMSD1")},
	{TEXT("BCMSD2")},
	/* POCKET PC PCMCIA device */
	{TEXT("BCMWL51")},
	{TEXT("BCMWL52")},
	/* CE PCI device */
	{TEXT("PCI\\BCMWL51")},
	{TEXT("PCI\\BCMWL52")},
	/* Device name for Intermec CN30 running Windows CE5.0 */ 
	{TEXT("BCMCF1")}
};

EPICTRL_API WINERR ir_init(HANDLE *irh)
{
	HANDLE uiohandle; 
	dll_private_t *ip;
	DWORD status;

	assert(irh);
	if (!irh)
		return ERROR_INVALID_HANDLE;

	if ((ip = (dll_private_t *) malloc(sizeof(dll_private_t)))) {
		memset(ip, 0, sizeof(dll_private_t));
		/* open NDIS UIO device */
		uiohandle = CreateFile(
						NDISUIO_DEVICE_NAME, 
						0,
						0,
					     NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
						INVALID_HANDLE_VALUE);
		if (uiohandle != INVALID_HANDLE_VALUE) {
			ip->handle = uiohandle;
			*irh = (HANDLE) ip;
			return NO_ERROR;
		}
		status = GetLastError();
		_ftprintf(stderr, _T("Failed loading ndisuio.dll. status 0x%x\n"), status);
	} else {
		status = GetLastError();
		_ftprintf(stderr, _T("malloc error.  status 0x%x\n"), status);
	}

	free(ip);
	return status;
}

EPICTRL_API WINERR 
ir_adapter_list(
    HANDLE		handle, 
    ADAPTER		devlist[], 
    PDWORD		maxdevs
)
{
	DWORD i, devices, buflen, outlen, status, names;
	PCHAR buffer;
	NIC_STATISTICS uioreq;
	dll_private_t *ip = (dll_private_t *)handle;

	assert(ip);
	assert(ip->handle);

	devices = 0;
	names = sizeof(driver_name) / sizeof(driver_name[0]);

	for (i = 0; i < names; i++) {
		devlist[devices].valid = FALSE;
		uioreq.Size = sizeof(NIC_STATISTICS);
		uioreq.ptcDeviceName = driver_name[i];
		if (!DeviceIoControl(
					ip->handle,
					IOCTL_NDISUIO_NIC_STATISTICS, 
					(LPVOID)&uioreq, 
					sizeof(NIC_STATISTICS),
					(LPVOID)&uioreq, 
					sizeof(NIC_STATISTICS),
					&outlen, 
					NULL))
			continue;

		memcpy(devlist[devices].name, driver_name[i], sizeof(devlist[devices].name));
		ir_bind(handle, devlist[devices].name);

		buflen = sizeof(devlist[devices].macaddr);
		ir_queryinformation(handle, OID_802_3_PERMANENT_ADDRESS, devlist[devices].macaddr, &buflen);
		buflen = sizeof(devlist[devices].description);
		buffer = _alloca(buflen);
		if (ir_queryinformation(handle, 
							OID_GEN_VENDOR_DESCRIPTION, 
							buffer, 
							&buflen) == NO_ERROR) {
#ifdef _UNICODE
			MultiByteToWideChar(CP_ACP, 0, buffer, buflen,  
					devlist[devices].description, sizeof(devlist[devices].description));
#else
			memcpy(devlist[devices].description, buffer, sizeof(devlist[devices].description));
#endif
		}
		devlist[devices].type = IR_WIRELESS;
		devlist[devices].valid = TRUE;

		ir_unbind(handle);
		devices++;
	}

	*maxdevs = devices;
	if (devices == 0) {
		status = GetLastError();
		_ftprintf(stderr, _T("No adapter found from ndisuio IOCTL_NDISUIO_NIC_STATISTICS.\n"));		
	} else 
		status = NO_ERROR;

	return status;
}

EPICTRL_API WINERR ir_exit(HANDLE irh)
{
	CloseHandle(((dll_private_t *)irh)->handle);
	free(irh);
	return NO_ERROR;
}

EPICTRL_API WINERR ir_bind(HANDLE irh, LPCTSTR devicename)
{ 
	((dll_private_t *)irh)->param = (PVOID)devicename;
	return NO_ERROR;
}

EPICTRL_API WINERR ir_unbind(HANDLE irh)
{
	((dll_private_t *)irh)->param = NULL;
	return NO_ERROR;
}

EPICTRL_API WINERR ir_queryinformation(HANDLE irh, ULONG oid, 
				       PUCHAR inbuf, PDWORD inlen)
{
	PNDISUIO_QUERY_OID pndisuio_oid;
	dll_private_t *ip = (dll_private_t *) irh;
	DWORD size, status, outlen;

	size = sizeof(NDISUIO_QUERY_OID) + *inlen - (*inlen>=4 ? 4 : 0);
	if (!(pndisuio_oid = (PNDISUIO_QUERY_OID)malloc(size)))
		return ERROR_OUTOFMEMORY;
	
	pndisuio_oid->Oid = oid;
	pndisuio_oid->ptcDeviceName = ip->param;
	if (inbuf && *inlen)
		memcpy(pndisuio_oid->Data, inbuf, *inlen);

	status = DeviceIoControl(
					ip->handle, 
					IOCTL_NDISUIO_QUERY_OID_VALUE, 
					(LPVOID)pndisuio_oid, 
					size,
					(LPVOID)pndisuio_oid, 
					size, 
					&outlen, 
					NULL);

	if (status) {
		memcpy(inbuf, pndisuio_oid->Data, *inlen);
		status = NO_ERROR;
	} else {
		status = GetLastError();
		_ftprintf(stderr, _T("ndisuio IOCTL_NDISUIO_QUERY_OID_VALUE error 0x%x\n"),status);
	}

	free(pndisuio_oid);
	return status;
}

EPICTRL_API WINERR ir_setinformation(HANDLE irh, ULONG oid, 
				     PUCHAR inbuf, PDWORD inlen)
{
	dll_private_t *ip = (dll_private_t *) irh;
	PNDISUIO_SET_OID pndisuio_oid;
	DWORD size, status, outlen;

	size = sizeof(NDISUIO_SET_OID) + *inlen - (*inlen>=4 ? 4 : 0);
	if (!(pndisuio_oid = (PNDISUIO_SET_OID)malloc(size)))
		return ERROR_OUTOFMEMORY;
	
	pndisuio_oid->Oid = oid;
	pndisuio_oid->ptcDeviceName = ip->param;
	if (inbuf && *inlen)
		memcpy(pndisuio_oid->Data, inbuf, *inlen);

	status = DeviceIoControl(
					ip->handle, 
					IOCTL_NDISUIO_SET_OID_VALUE, 
					(LPVOID)pndisuio_oid, 
					size,
					(LPVOID)pndisuio_oid, 
					0,
					&outlen, 
					NULL);

	status = status ? NO_ERROR : GetLastError();
	if (status != NO_ERROR)
		_ftprintf(stderr, _T("ndisuio IOCTL_NDISUIO_SET_OID_VALUE error 0x%x\n"),status);
	free(pndisuio_oid);
	return status;
}
