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
 * Exposed API for the bcmrly library.
 * Provides OID access to network adapters.
 *
 */

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <tchar.h>

#include "oidencap.h"
#include "epivers.h"
#include "epictrl.h"
#include "irelay.h"
#include "wmiprocs.h"
#include "vendor.h"
#include "assert.h"
#include "epictrl_private.h"
#include "ir_wmi.h"

BOOL g_useWMI = FALSE;

EPICTRL_API WINERR ir_init(HANDLE *irh)
{
	WINERR status = NO_ERROR;
	HANDLE handle = INVALID_HANDLE_VALUE;

	assert(irh);
	if (!irh)
		return ERROR_INVALID_HANDLE;

	GetOSVer();

	if (status == NO_ERROR) {
		dll_private_t *ip;

		ip = (dll_private_t *) malloc(sizeof(dll_private_t));
		if (ip != NULL) {
			memset(ip, 0, sizeof(dll_private_t));
			ip->handle = handle;
			*irh = (HANDLE) ip;
		}
	}

	return status;
}

EPICTRL_API WINERR
ir_adapter_list(
	HANDLE		handle,
	ADAPTER		devlist[],
	PDWORD		maxdevs
)
{
	return GetAdapterList(handle, devlist, maxdevs);
}

EPICTRL_API WINERR ir_exit(HANDLE irh)
{
	WINERR status = NO_ERROR;
	dll_private_t *ip = (dll_private_t *) irh;

	assert(irh);
	assert(irh != INVALID_HANDLE_VALUE);
	if (!irh || irh == INVALID_HANDLE_VALUE)
		return ERROR_INVALID_HANDLE;

	if (ip->handle != INVALID_HANDLE_VALUE && !CloseHandle(ip->handle)) {
		status = GetLastError();
	}

	ip->handle = (HANDLE) 0xdeadbeef;
	ip->param = NULL;
	free(ip);

	// CloseAndUnload(HWD_DRIVER_SERVICE);

	return status;
}

/* send an ioctl down to the irelay driver and wait for it to complete.
   If you want the wait to timeout, supply a timeout value for
   'millis' in milliseconds. Set millis to INFINITE if you do not want
   a timeout at all. */
WINERR ir_ioctl(HANDLE m_dh, DWORD ioctl,
		PCHAR buf, PDWORD plen, DWORD millis)
{
	DWORD status;
	int ok;
	OVERLAPPED ovlp = { 0, 0, 0, 0, 0 };
	HANDLE hEvent = 0;
	DWORD wait;
	dll_private_t *ip = (dll_private_t *) m_dh;

	hEvent = CreateEvent(0, TRUE, 0, NULL);
	if (hEvent == NULL) {
		return GetLastError();
	}
	ovlp.hEvent = hEvent;

	status = NO_ERROR;
	ok = DeviceIoControl(ip->handle, ioctl, buf, *plen,
			     buf, *plen, plen, &ovlp);
	if (!ok) {
		status = GetLastError();
		if (status == ERROR_IO_PENDING) {
			wait = WaitForSingleObject(hEvent, millis);
			switch (wait) {
			case WAIT_OBJECT_0:
				if (!GetOverlappedResult(ip->handle, &ovlp, plen, TRUE)) {
					status = GetLastError();
				} else {
					if (ovlp.Internal != 0) {
						status = ovlp.Internal;
					} else {
						status = ERROR_SUCCESS;
					}
				}
				break;
			case WAIT_FAILED:
				status = GetLastError();
				break;
			case WAIT_TIMEOUT:
				*plen = 0;
				status = ERROR_TIMEOUT;
				break;
			default:
				_ftprintf(stderr, _T("Received unexpected status from WaitForSingleObject = 0x%x"), wait);
				status = ERROR_INVALID_FUNCTION;
			}
		}
	}

	CloseHandle(hEvent);
	return status;
}

#define DEVICE_PREFIX	_T("\\\\.\\")

WINERR
ir_bind1(HANDLE m_dh, LPCTSTR DeviceName)
{
	DWORD status;
	dll_private_t *ip;
	TCHAR devicename[512];

	assert(m_dh);
	assert(m_dh != INVALID_HANDLE_VALUE);
	if (!m_dh || m_dh == INVALID_HANDLE_VALUE)
		return ERROR_INVALID_HANDLE;

	ip = (dll_private_t *) m_dh;
	status = RPC_S_INVALID_BINDING;

	assert(ip->handle == INVALID_HANDLE_VALUE);

	if (ip->handle != INVALID_HANDLE_VALUE)
		ir_unbind(m_dh);

	if (ip->handle == INVALID_HANDLE_VALUE) {
		_tcscpy(&(devicename[0]), DEVICE_PREFIX);
		_tcscat(&(devicename[0]), DeviceName);

		ip->handle = CreateFile(devicename,
					0, FILE_SHARE_READ, NULL,
					OPEN_EXISTING, 0, INVALID_HANDLE_VALUE);
		if (ip->handle == INVALID_HANDLE_VALUE)
			status = GetLastError();
		else
			status = NO_ERROR;
	} else {
		status = ERROR_INVALID_HANDLE;
	}

	return status;
}

static BOOLEAN
ir_findwminame(dll_private_t *ip, LPCTSTR DeviceName)
{
	ADAPTER devlist[MAX_ADAPTERS];
	DWORD ndevs = ARRAYSIZE(devlist);
	DWORD i;

	/* find the corresponding WMI devname and bind to it */
	if (ir_adapter_list(ip, &devlist[0], &ndevs) == NO_ERROR) {
		for (i = 0; i < ndevs; ++i)
			if (strcmp(devlist[i].name, DeviceName) == 0)
				return TRUE;
	}
	return FALSE;
}

EPICTRL_API WINERR ir_bind(HANDLE m_dh, LPCTSTR DeviceName)
{
	DWORD status = NO_ERROR;
	dll_private_t *ip;
	size_t len;

	if ((status = ir_bind1(m_dh, DeviceName)) == NO_ERROR) {
		ip = (dll_private_t *) m_dh;
		if (ir_findwminame(ip, DeviceName)) {
			if (ip->reserved)
				free(ip->reserved);
			len = strlen(DeviceName);
			if ((ip->reserved = malloc(len + 1))) {
				memcpy(ip->reserved, DeviceName, len);
				((PCHAR) (ip->reserved))[len] = '\0';
			}
		}
	}

	return status;
}

WINERR ir_unbind1(HANDLE m_dh)
{
	DWORD status = NO_ERROR;
	dll_private_t *ip = (dll_private_t *) m_dh;

	assert(m_dh);
	assert(m_dh != INVALID_HANDLE_VALUE);
	if (!m_dh || m_dh == INVALID_HANDLE_VALUE)
		return ERROR_INVALID_HANDLE;

	if (ip->handle != INVALID_HANDLE_VALUE) {
		if (!CloseHandle(ip->handle))
			status = GetLastError();
		else
			ip->handle = INVALID_HANDLE_VALUE;
	} else {
		status = ERROR_INVALID_HANDLE;
	}

	return status;
}

EPICTRL_API WINERR ir_unbind(HANDLE m_dh)
{
	DWORD status = NO_ERROR;
	dll_private_t *ip = (dll_private_t *) m_dh;

	status = ir_unbind1(m_dh);
	if (ip->reserved) {
		free(ip->reserved);
		ip->reserved = NULL;
	}

	return status;
}

EPICTRL_API WINERR ir_queryinformation(HANDLE m_dh, ULONG oid,
				       PUCHAR inbuf, PDWORD inlen)
{
	dll_private_t *ip = (dll_private_t *) m_dh;
	getinformation_t *getinfo = NULL;
	PUCHAR buf;
	ULONG nbytes = 0;
	DWORD status = NO_ERROR;
	int ok;
	DWORD millis = 500;
	OVERLAPPED ovlp;
	HANDLE hEvent;
	DWORD		wait;
	int offset = 0;

	assert(m_dh);
	assert(m_dh != INVALID_HANDLE_VALUE);
	if (!m_dh || m_dh == INVALID_HANDLE_VALUE)
		return ERROR_INVALID_HANDLE;

	if (ip->handle != INVALID_HANDLE_VALUE) {
		nbytes = GETINFORMATION_SIZE + *inlen;
		getinfo = (getinformation_t *) malloc(nbytes + 40);
		if (!getinfo)
			return ERROR_NOT_ENOUGH_MEMORY;

		getinfo->cookie = OIDENCAP_COOKIE;
		getinfo->oid = oid;
		getinfo->len = nbytes;
		memcpy(GETINFORMATION_DATA(getinfo), inbuf, *inlen);
		oid = OID_BCM_GETINFORMATION;
		buf = (PUCHAR) getinfo;

query_again:
		hEvent = 0;
		memset(&ovlp, '\0', sizeof(ovlp));
		hEvent = CreateEvent(0, TRUE, 0, NULL);
		if (hEvent == NULL) {
			status = GetLastError();
			goto end;
		}
		ovlp.hEvent = hEvent;

		status = NO_ERROR;
		ok = DeviceIoControl(ip->handle, IOCTL_NDIS_QUERY_GLOBAL_STATS, &oid,
				     sizeof(oid), buf, nbytes, &nbytes, &ovlp);
		if (!ok) {
			status = GetLastError();
			if (status == ERROR_IO_PENDING) {
				wait = WaitForSingleObject(hEvent, millis);
				switch (wait) {
				case WAIT_OBJECT_0:
					if (!GetOverlappedResult(ip->handle, &ovlp, &nbytes, TRUE)) {
						status = GetLastError();
					} else {
						if (ovlp.Internal != 0) {
							status = ovlp.Internal;
						} else {
							status = ERROR_SUCCESS;
						}
					}
					break;
				case WAIT_FAILED:
					status = GetLastError();
					break;
				case WAIT_TIMEOUT:
					*inlen = 0;
					status = ERROR_TIMEOUT;
					break;
				default:
					_ftprintf(stderr, _T("Received unexpected status from WaitForSingleObject = 0x%x"), wait);
					status = ERROR_INVALID_FUNCTION;
				}
			}
		}

		CloseHandle(hEvent);
	}

	if (oid == OID_BCM_GETINFORMATION) {
		if (status != ERROR_SUCCESS) {
			/* try using the old method */
			oid = getinfo->oid;
			buf = inbuf;
			nbytes = *inlen;
			goto query_again;
		} else {
			nbytes -= GETINFORMATION_SIZE;

			if (nbytes + offset > *inlen)
				nbytes = *inlen;

			memcpy(inbuf + offset, GETINFORMATION_DATA(getinfo), nbytes);

			if (((nbytes + offset) < *inlen) && getinfo->cookie != OIDENCAP_COOKIE) {
				offset += nbytes;
				nbytes = GETINFORMATION_SIZE + *inlen;
				goto query_again;
			}
		}
	}

	*inlen = offset + nbytes;

end:
	if (getinfo)
		free(getinfo);

	return status;
}

EPICTRL_API WINERR ir_setinformation(HANDLE m_dh, ULONG oid,
				     PUCHAR inbuf, PDWORD inlen)
{
	dll_private_t *ip = (dll_private_t *) m_dh;
	DWORD status = NO_ERROR;
	ULONG nbytes;
	setinformation_t *setinfo;
	OVERLAPPED ovlp = { 0, 0, 0, 0, 0 };
	HANDLE hEvent = 0;

	assert(m_dh);
	assert(m_dh != INVALID_HANDLE_VALUE);
	if (!m_dh || m_dh == INVALID_HANDLE_VALUE)
		return ERROR_INVALID_HANDLE;

	if (ip->handle != INVALID_HANDLE_VALUE) {

		nbytes = SETINFORMATION_SIZE + *inlen;
		setinfo = (setinformation_t *) malloc(nbytes);
		if (!setinfo)
			return ERROR_NOT_ENOUGH_MEMORY;

		setinfo->cookie = OIDENCAP_COOKIE;
		setinfo->oid = oid;
		memcpy(SETINFORMATION_DATA(setinfo), inbuf, *inlen);

		/* an RNDIS device only gets 48 bytes of a query OID's buffer */
		if (nbytes > 48 && ip->reserved && g_useWMI && (WMIOpen() == ERROR_SUCCESS)) {
			/* use WMI to issue a true set OID for the device to receive all data */
			if ((status = WMICmd (TRUE, (PCHAR) ip->reserved, OID_BCM_SETINFORMATION, (PVOID) setinfo, &nbytes)) != ERROR_SUCCESS) {
				if (status != ERROR_GEN_FAILURE) {
					char buf[1024];
					sprintf(buf, "ir_setinformation: WMICmd failed w/status: %d; wminame: %s\n", status,
							ip->reserved ? (char *) (ip->reserved) : "NULL!");
					OutputDebugString(buf);
				}
			}
			WMIClose();
		} else {
			hEvent = CreateEvent(0, TRUE, 0, NULL);
			if (hEvent == NULL) {
				free(setinfo);
				return GetLastError();
			}
			ovlp.hEvent = hEvent;

			oid = OID_BCM_SETINFORMATION;

			SetLastError(NO_ERROR);
			if(!DeviceIoControl(ip->handle,
					    IOCTL_NDIS_QUERY_GLOBAL_STATS,
					    &oid, sizeof(oid),
					    setinfo, nbytes, &nbytes, &ovlp)) {
				status = GetLastError();
				if (status == ERROR_IO_PENDING) {
					if (!GetOverlappedResult(ip->handle, &ovlp,
								 &nbytes, TRUE)) {
						status = GetLastError();
					} else {
						if (ovlp.Internal != 0) {
							status = ovlp.Internal;
						} else {
							status = ERROR_SUCCESS;
						}
					}
				}
			}
			CloseHandle(hEvent);
		}

		*inlen = (nbytes - SETINFORMATION_SIZE);
		free(setinfo);
	} else {
		status = RPC_S_INVALID_BINDING;
	}

	return status;
}

EPICTRL_API WINERR
ir_adapter_reinitialize(
	LPCTSTR		DeviceName
)
{
	HANDLE irh;
	DWORD status = ERROR;
	ADAPTER devlist[MAX_ADAPTERS];
	int i, idevs = 0;

	irh = INVALID_HANDLE_VALUE;
	if (ir_init(&irh) == NO_ERROR) {
		idevs = ARRAYSIZE(devlist);
		status = GetAdapterListFromSetupDi(irh, &devlist[0], &idevs);
		ir_exit(irh);
	}
	if (status == NO_ERROR && idevs > 0) {
		for (i = 0; i < idevs; i++) {
			if (devlist[i].type == IR_WIRELESS) {
				if (strcmpi(devlist[i].name, DeviceName) == 0) {
					status = EnableAdapter(devlist[i].instance, FALSE);
					if (status == NO_ERROR)
						status = EnableAdapter(devlist[i].instance, TRUE);
					break;
				}
			}
		}
	}

	return status;
}

EPICTRL_API void ir_usewmiset(BOOL useWMI)
{
	g_useWMI = useWMI;
}

EPICTRL_API WINERR
ir_adapter_disable(
	LPCTSTR		DeviceName
)
{
	HANDLE irh;
	DWORD status = ERROR;
	ADAPTER devlist[MAX_ADAPTERS];
	int i, idevs = 0;

	irh = INVALID_HANDLE_VALUE;
	if (ir_init(&irh) == NO_ERROR) {
		idevs = ARRAYSIZE(devlist);
		status = GetAdapterListFromSetupDi(irh, &devlist[0], &idevs);
		ir_exit(irh);
	}
	if (status == NO_ERROR && idevs > 0) {
		for (i = 0; i < idevs; i++) {
			if (devlist[i].type == IR_WIRELESS) {
				if (strcmpi(devlist[i].name, DeviceName) == 0) {
					status = EnableAdapter(devlist[i].instance, FALSE);
					break;
				}
			}
		}
	}
	return status;
}

EPICTRL_API WINERR
ir_adapter_enable(LPCTSTR componentId)
{
	HANDLE irh;
	DWORD status = ERROR;
	ADAPTER devlist[MAX_ADAPTERS];
	int i, idevs = 0;

	irh = INVALID_HANDLE_VALUE;
	if (ir_init(&irh) == NO_ERROR) {
		idevs = ARRAYSIZE(devlist);
		status = GetAllAdapterListFromSetupDi(irh, &devlist[0], &idevs);
		ir_exit(irh);
	}
	if (status == NO_ERROR && idevs > 0) {
		for (i = 0; i < idevs; i++) {
			if (strcmpi(devlist[i].componentId, componentId) == 0) {
				status = EnableAdapter(devlist[i].instance, TRUE);
				break;
			}
		}
	}
	return status;
}
