/*
 * Wl server for Windows Vista
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

#include <typedefs.h>
#include <wlioctl.h>
#include <proto/ethernet.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmcdc.h>

#include "wlanapi.h"
#include "oidencap.h"
#include "wlu.h"
#include "wlu_remote.h"
#include "wlu_remote_vista.h"

const char *g_rwl_server_name = "";
const char *wlu_av0 = "";


unsigned short defined_debug = DEBUG_ERR | DEBUG_INFO;

/* added for common file. which was earlier used by wlu_vista.c
 * This function is copied from wlu.c
 */
int
wl_ether_atoe(const char *a, struct ether_addr *n)
{
	char *c;
	int i = 0;

	memset(n, 0, ETHER_ADDR_LEN);
	for (;;) {
		n->octet[i++] = (uint8)strtoul(a, &c, 16);
		if (!*c++ || i == ETHER_ADDR_LEN)
			break;
		a = c;
	}
	return (i == ETHER_ADDR_LEN);
}

/* to execute vista specific commands from lookup table */
int
remote_vista_exec(void *wl, char **buf_ptr)
{
	cmd_t *cmd = NULL;
	int err;
	/* search the vista_cmds */
	for (cmd = vista_cmds; cmd->name && strcmp(cmd->name, *buf_ptr); cmd++);

	if (cmd->name) {
		err = (WINERR) (*cmd->func)(wl, cmd, buf_ptr);
		return err;
	}
	return BCME_OK;
}

/* This function is copied from wlu.c except removing condition for
 * checking BIG_ENDIAN
 */
int
wl_check(void *wl)
{
	int ret, val;

	if ((ret = wl_get(wl, WLC_GET_MAGIC, &val, sizeof(int))) < 0)
		return ret;
	if (val != WLC_IOCTL_MAGIC)
		return -1;

	if ((ret = wl_get(wl, WLC_GET_VERSION, &val, sizeof(int))) < 0)
		return ret;

	if (val > WLC_IOCTL_VERSION) {
		fprintf(stderr, "Version mismatch, please upgrade\n");
		return -1;
	}
	return 0;
}

int
main(int argc, char **argv)
{
	HANDLE irh;	/* wireless handle */
	WINERR err;

	g_rwl_server_name = argv[0];

	/* initialize irelay and select default adapter */
	irh = INVALID_HANDLE_VALUE;
	if ((err = ir_vista_init(&irh)) == ERROR_SUCCESS) {
		ndevs = ARRAYSIZE(devlist);
		if ((err = ir_vista_adapter_list(irh, &devlist[0], &ndevs)) == ERROR_SUCCESS)
			err = select_adapter(irh, -1);
	}
	err = remote_server_exec(argc, argv, irh);

	/* close wireless */
	if (irh != INVALID_HANDLE_VALUE)
		ir_vista_exit(irh);

	return err;

}

WINERR
ir_queryinformation(void *wl, int cmd, void *buf, int *len)
{
	return (int)ir_vista_queryinformation((HANDLE)wl, &dev, cmd, buf, len);
}

WINERR
ir_setinformation(void *wl, int cmd, void *buf, int *len)
{
	return (int)ir_vista_setinformation((HANDLE)wl, &dev, cmd, buf, len);
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
