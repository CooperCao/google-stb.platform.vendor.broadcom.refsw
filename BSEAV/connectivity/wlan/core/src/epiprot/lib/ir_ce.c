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

#include "epictrl.h" //#include "bcmpdlib.h"
#include "assert.h"

#include <ntddndis.h>
#include <nuiouser.h>

#include <iphlpapi.h>

WINERR ir_init(HANDLE *irh)
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

/* Certain virtual adapters like cisco etc. from 4.01 version onwards
   show up as a valid adapter. We need to remove those from the list.
   Until a better method can be found to do so, for now strip it out
   using the description */
BOOL IsValidIPHlpAdapter(LPCTSTR pDesc)
{
	TCHAR cBuf[512];
	LPTSTR p, p1;
	lstrcpy(cBuf, pDesc);
	_tcsupr(cBuf);
	p = _tcsstr(cBuf, _T("VPN"));
	p1 = _tcsstr(cBuf, _T("RAS"));
	if (p || p1)
		return FALSE;
	else
		return TRUE;
}

DWORD get_adapter_type(HANDLE lirh, PADAPTER pdev)
{
    DWORD len, status = ERROR_SUCCESS, instance, speed = 0;
    
	len = sizeof(instance);
	status = ir_queryinformation(lirh, OID_GEN_PHYSICAL_MEDIUM, (PUCHAR) &instance, &len);
	if (status != ERROR_SUCCESS)
		status = ir_queryinformation(lirh, OID_GEN_MEDIA_SUPPORTED, (PUCHAR) &instance, &len);
	if (status == ERROR_SUCCESS)
	{
		switch (instance)
		{
			case NdisPhysicalMediumWirelessLan:
				pdev->type = IR_WIRELESS;
				break;

			case NdisMedium802_3:
				pdev->type = IR_802_3;
				break;

			case NdisPhysicalMediumWirelessWan:
				pdev->type = IR_WWAN;
				break;

			default:
				break;
		}
	}

	// If we have still not determined the type, check to see if this 
	// is an usb adapter
	if (pdev->type == IR_UNKNOWN)
	{
		TCHAR cBuf[512];
		lstrcpy(cBuf, pdev->adaptername);
		_tcsupr(cBuf);
		if (_tcsstr(cBuf, _T("USB CABLE")))
			pdev->type = IR_USB;
	}

	return status;
}

WINERR ir_adapter_list(HANDLE handle, ADAPTER devlist[], PDWORD	maxdevs)
{
    WINERR  status;
    DWORD   ndevs = 0, index;
    PADAPTER pdev;

    MIB_IFROW IfRow;
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pInfo;
    ULONG needlen, ifindex = 0;
    HANDLE  lirh;  // local handle for duration of this routine.
    
    status = ir_init(&lirh);
    if (status == NO_ERROR) {
	needlen = 0;
	status = GetAdaptersInfo(NULL, &needlen);
	if (status == ERROR_BUFFER_OVERFLOW) {
	    pAdapterInfo = (PIP_ADAPTER_INFO) malloc(needlen);
	    memset(pAdapterInfo, 0, needlen);
	    status = GetAdaptersInfo(pAdapterInfo, &needlen);
	    
	    index = 0;
	    pInfo = pAdapterInfo;
	    for (pInfo = pAdapterInfo; pInfo; pInfo = pInfo->Next) {
		if (ndevs >= *maxdevs) 
		    break;

		IfRow.dwIndex = pInfo->Index;
		status = GetIfEntry(&IfRow);
		if (status != NO_ERROR || IfRow.dwAdminStatus != MIB_IF_ADMIN_STATUS_UP)
		    continue;
		
		pdev = &devlist[ndevs];
		memset(pdev, 0, sizeof(ADAPTER));
		pdev->type = IR_UNKNOWN;
		pdev->instance = pInfo->Index;
		memcpy(pdev->macaddr, pInfo->Address,sizeof(pdev->macaddr) );
		pdev->dhcpenabled = pInfo->DhcpEnabled;

#if _UNICODE
		mbstowcs(pdev->adaptername, pInfo->AdapterName, sizeof(pdev->adaptername));
		mbstowcs(pdev->description, pInfo->Description, sizeof(pdev->description));
		mbstowcs(pdev->dhcpserver, pInfo->DhcpServer.IpAddress.String, sizeof(pdev->dhcpserver));
		mbstowcs(pdev->ipaddr, pInfo->IpAddressList.IpAddress.String, sizeof(pdev->ipaddr));
		mbstowcs(pdev->ipgateway, pInfo->GatewayList.IpAddress.String, sizeof(pdev->ipgateway));
#else
		memcpy(pdev->adaptername, pInfo->AdapterName, sizeof(pdev->adaptername) );
		strncpy(pdev->description, pInfo->Description, sizeof(pdev->description));
		strncpy(pdev->dhcpserver, pInfo->DhcpServer.IpAddress.String, sizeof(pdev->dhcpserver));
		strncpy(pdev->ipaddr, pInfo->IpAddressList.IpAddress.String, sizeof(pdev->ipaddr));
		strncpy(pdev->ipgateway, pInfo->GatewayList.IpAddress.String, sizeof(pdev->ipgateway));
#endif

		if (ir_bind(lirh, pdev->adaptername) == ERROR_SUCCESS) {
			if (IsValidIPHlpAdapter(pdev->adaptername)) {
				get_adapter_type(lirh, pdev);
				ndevs++;
			}
			ir_unbind(lirh);
		}
	    }
	    free(pAdapterInfo);
	}
	ir_exit(lirh);
    }

    *maxdevs = ndevs;

    return status;
}

WINERR ir_exit(HANDLE irh)
{
	CloseHandle(((dll_private_t *)irh)->handle);
	free(irh);
	return NO_ERROR;
}

WINERR ir_bind(HANDLE irh, LPCTSTR devicename)
{ 
	((dll_private_t *)irh)->param = (PVOID)devicename;
	return NO_ERROR;
}

WINERR ir_unbind(HANDLE irh)
{
	((dll_private_t *)irh)->param = NULL;
	return NO_ERROR;
}

WINERR ir_queryinformation(HANDLE irh, ULONG oid, PUCHAR inbuf, PDWORD inlen)
{
	PNDISUIO_QUERY_OID pndisuio_oid;
	dll_private_t *ip = (dll_private_t *) irh;
	DWORD size, status, outlen;

	size = sizeof(NDISUIO_QUERY_OID) + *inlen - (*inlen ? 4 : 0);
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
//		_ftprintf(stderr, _T("ndisuio IOCTL_NDISUIO_QUERY_OID_VALUE error 0x%x\n"),status);
	}

	free(pndisuio_oid);
	return status;
}

WINERR ir_setinformation(HANDLE irh, ULONG oid, PUCHAR inbuf, PDWORD inlen)
{
	dll_private_t *ip = (dll_private_t *) irh;
	PNDISUIO_SET_OID pndisuio_oid;
	DWORD size, status, outlen;

	size = sizeof(NDISUIO_SET_OID) + *inlen - (*inlen ? 4 : 0);
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
//	if (status != NO_ERROR)
//		_ftprintf(stderr, _T("ndisuio IOCTL_NDISUIO_SET_OID_VALUE error 0x%x\n"),status);
	free(pndisuio_oid);
	return status;
}
