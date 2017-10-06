/*
 * Wl server for WinCE
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * $Id$
 */

#define NEED_IR_TYPES

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winioctl.h>
#include <malloc.h>
#include <assert.h>
#include <ntddndis.h>

#include <typedefs.h>
#include <wlioctl.h>
#include <epictrl.h>
#include <irelay.h>
#include <proto/ethernet.h>
#include <nuiouser.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmcdc.h>
#include <proto/802.11.h>
#include "wlu.h"
#include "wlu_remote.h"

#define DEV_LIST	10
DWORD ndevs;
ADAPTER devlist[DEV_LIST];
void *g_wl_handle;
const char *rwl_server_name = "";

static void display_err(PCHAR prefix, WINERR status);
static WINERR select_adapter(HANDLE, int adapter);
unsigned short defined_debug = DEBUG_ERR | DEBUG_INFO;

extern int remote_server_exec(int argc, char **argv, void *irh);

int
main(int argc, char **argv)
{
	HANDLE irh;	/* wireless handle */
	WINERR err;
	char *ptr = NULL;
#ifdef RWL_WIFI
	dot11_action_wifi_vendor_specific_t  rec_frame;
#endif

	rwl_server_name = argv[0];

	/* initialize irelay and select default adapter */
	irh = INVALID_HANDLE_VALUE;
	if ((err = ir_init(&irh)) == ERROR_SUCCESS) {
		ndevs = ARRAYSIZE(devlist);
		if ((err = ir_adapter_list(irh, &devlist[0], &ndevs)) == ERROR_SUCCESS) {
			err = select_adapter(irh, -1);
		}
	}

	g_wl_handle = irh;
	err = remote_server_exec(argc, argv, irh);

	/* close wireless */
	if (irh != INVALID_HANDLE_VALUE) {
		ir_unbind(irh);
		ir_exit(irh);
	}

	return err;

}

/* The following functions are copied from wlu_ndis.c to check for the driver
 * adapter in the server machine.
 */

/* select the numbered adapter */
static WINERR
select_adapter(HANDLE irh, int adapter)
{
	/*  GLOBAL ADAPTER	devlist[]; */
	/*  GLOBAL uint		ndevs; */
	WINERR status;
	ULONG i;
	PADAPTER		pdev;

	/* If adapter == -1, choose the first appropriate adapter. */
	if (adapter == -1) {
		for (i = 0; i < ndevs; i++) {
			pdev = &devlist[i];
			if (pdev->type == IR_WIRELESS) {
				adapter = i;
				break;
			}
		}

		if (i == ndevs) {
			fprintf(stderr, "%s: No wireless adapters were found\n", rwl_server_name);
			return ERROR_INVALID_HANDLE;
		}
	}

	if (adapter < 0 || (ULONG) adapter >= ndevs) {
		fprintf(stderr, "%s: Cannot find wireless adapter #%d\n", rwl_server_name, adapter);
		status = ERROR_INVALID_HANDLE;
	} else {
		pdev = &devlist[adapter];
		if (pdev->type != IR_WIRELESS) {
			fprintf(stderr,
			        "%s: Selected adapter #%d is not an BRCM wireless adapter\n",
			        rwl_server_name, adapter);
			status = ERROR_INVALID_HANDLE;
		} else {
			status = ir_bind(irh, pdev->name);
			if (status != ERROR_SUCCESS)
				display_err("select_adapter:ir_bind:", status);
		}
	}

	return status;
}
int
wl_get(void *wl, int cmd, void *buf, int len)
{
	DWORD dwlen = len;
	int error;

	error = (int)ir_queryinformation((HANDLE)wl, WL_OID_BASE +cmd, buf, &dwlen);
	if (error != 0) {
		return BCME_IOCTL_ERROR;
	}
	return 0;
}

int
wl_set(void *wl, int cmd, void *buf, int len)
{
	DWORD dwlen = len;
	int error;

	error = (int)ir_setinformation((HANDLE)wl, WL_OID_BASE+cmd, buf, &dwlen);
	if (error != 0) {
		return BCME_IOCTL_ERROR;
	}
	return 0;
}
int
wl_atoip(const char *a, struct ipv4_addr *n)
{
	char *c;
	int i = 0;

	for (;;) {
		n->addr[i++] = (uint8)strtoul(a, &c, 0);
		if (*c++ != '.' || i == IPV4_ADDR_LEN)
			break;
		a = c;
	}
	return (i == IPV4_ADDR_LEN);
}
static void
display_err(PCHAR prefix, WINERR status)
{
	PCHAR   ErrStr;
	DWORD   ErrLen;

	ErrLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, status,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &ErrStr, 0, NULL);
	if (ErrLen > 0) {
		fprintf(stderr, "%s: Error 0x%x: %s -- %s\n",
		rwl_server_name, status, prefix, ErrStr);
		LocalFree(ErrStr);
	} else
		fprintf(stderr, "%s: Error 0x%x: %s -- Unknown error\n", rwl_server_name,
		                                                    status, prefix);
}
