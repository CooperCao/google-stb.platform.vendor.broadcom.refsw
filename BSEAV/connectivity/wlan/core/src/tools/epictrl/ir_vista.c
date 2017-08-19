/*
 * Vista port of wl command line utility
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <typedefs.h>
#include <proto/ethernet.h>
#include <bcmendian.h>
#include <bcmutils.h>

#if defined(WLEXE_DBG)
#define WLEXE_DBGPRINT(arg) printf arg
#else
#define WLEXE_DBGPRINT(arg)
#endif

#include "wlanapi.h"
#include "oidencap.h"

#define WINERR	DWORD

typedef struct _WL_QUERY_OID_HEADER {
	ULONG oid;
	getinformation_t getinfo_hdr;
} WL_QUERY_OID_HEADER;

typedef struct _WL_SET_OID_HEADER {
	ULONG oid;
	setinformation_t setinfo_hdr;
} WL_SET_OID_HEADER;

WINERR
ir_vista_init(HANDLE *irh)
{
	WINERR err;
	DWORD dwNegotiatedVersion;

	WLEXE_DBGPRINT(("ir_init() called...\n"));

	do {
		/* open a handle to the service */
		err = WlanOpenHandle(WLAN_API_VERSION, NULL, &dwNegotiatedVersion, irh);
		if (err != ERROR_SUCCESS) {
			printf("WlanOpenHandle() failed...\n");
			break;
		}

		WLEXE_DBGPRINT(("WlanOpenHandle() succeeded...\n"));

		/* check for the right version */
		if (dwNegotiatedVersion == WLAN_API_VERSION_2_0) {
			WLEXE_DBGPRINT(("WLAN_API_VERSION 0x%x WinVista\n", dwNegotiatedVersion));
		} else if (dwNegotiatedVersion == WLAN_API_VERSION_1_0) {
			WLEXE_DBGPRINT(("WLAN_API_VERSION 0x%x WinXP\n", dwNegotiatedVersion));
		} else {
			printf("WLAN_API_VERSION 0x%x is not supported\n", dwNegotiatedVersion);
			err = ERROR_PRODUCT_VERSION;
			WlanCloseHandle(*irh, NULL);
			*irh = INVALID_HANDLE_VALUE;
		}
	} while (0);

	return err;
}

void
ir_vista_exit(HANDLE irh)
{
	WLEXE_DBGPRINT(("ir_exit called()...\n"));

	WlanCloseHandle(irh, NULL);
}

WINERR
ir_vista_adapter_list(HANDLE irh, GUID *devlist, DWORD *ndevs)
{
	WINERR err;
	WLAN_INTERFACE_INFO_LIST *iflist = NULL;
	UINT i;

	WLEXE_DBGPRINT(("ir_adapter_list() called...\n"));

	do {
			err = WlanEnumInterfaces(irh, NULL, &iflist);
			if (err != ERROR_SUCCESS)
					break;

			*ndevs = iflist->dwNumberOfItems;
			for (i = 0; i < *ndevs; i++) {
					devlist[i] = iflist->InterfaceInfo[i].InterfaceGuid;
					WLEXE_DBGPRINT(("Description: %ws\n",
						iflist->InterfaceInfo[i].strInterfaceDescription));
			}

			if (iflist != NULL)
					WlanFreeMemory(iflist);
	} while (0);

	return err;
}

WINERR
ir_vista_queryinformation(HANDLE irh, GUID *dev, ULONG oid, PUCHAR inbuf, PDWORD inlen)
{
	uint nbytes;
	WL_QUERY_OID_HEADER *req;
	WINERR status;
	DWORD bytes_returned, outlen;

	nbytes = sizeof(WL_QUERY_OID_HEADER) + *inlen;
	req = (WL_QUERY_OID_HEADER *) WlanAllocateMemory(nbytes);
	if (!req)
		return ERROR_NOT_ENOUGH_MEMORY;

	/* initialize the request header */
	req->oid = OID_BCM_GETINFORMATION;

	/* initialize the "getinfo" header */
	req->getinfo_hdr.oid = oid;
	req->getinfo_hdr.len = nbytes;
	req->getinfo_hdr.cookie = OIDENCAP_COOKIE;

	/* followed by the oid-specific input data (just copy entire buffer) */
	if (*inlen > 0)
		memcpy(&req[1], inbuf, *inlen);

	status = WlanIhvControl(irh, dev, wlan_ihv_control_type_driver, nbytes, req, nbytes, req,
		&bytes_returned);
	outlen = bytes_returned - sizeof(WL_QUERY_OID_HEADER);
	if (status == ERROR_SUCCESS) {
		if (outlen > *inlen)
			outlen = *inlen;
		if (outlen > 0) {
			memcpy(inbuf, &req[1], outlen);
			*inlen = outlen;
		} else {
			status = ERROR_NOT_ENOUGH_MEMORY;
		}
	} else {
		*inlen = outlen;
	}

	WlanFreeMemory(req);

	return status;
}

WINERR
ir_vista_setinformation(HANDLE irh, GUID *dev, ULONG oid, PUCHAR inbuf, PDWORD inlen)
{
	uint nbytes;
	WL_SET_OID_HEADER *req;
	WINERR status;
	DWORD bytes_returned;
	DWORD in_data_size = *inlen;

	if (inbuf == NULL)
		in_data_size = 0;

	nbytes = sizeof(WL_SET_OID_HEADER) + in_data_size;
	req = (WL_SET_OID_HEADER *) WlanAllocateMemory(nbytes);
	if (!req)
		return ERROR_NOT_ENOUGH_MEMORY;

	/* initialize the request header */
	req->oid = OID_BCM_SETINFORMATION;

	/* initialize the "getinfo" header */
	req->setinfo_hdr.oid = oid;
	req->setinfo_hdr.cookie = OIDENCAP_COOKIE;

	/* followed by the oid-specific input data */
	if (in_data_size > 0)
		memcpy(&req[1], inbuf, in_data_size);

	status = WlanIhvControl(irh, dev, wlan_ihv_control_type_driver, nbytes, req, nbytes, req,
		&bytes_returned);

	WlanFreeMemory(req);

	return status;
}
