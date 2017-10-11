/*
 * Wl server for generic RTOS
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

#include <stdio.h>
#include "typedefs.h"
#include "bcmutils.h"
#include "bcmcdc.h"
#include "wlioctl.h"
#include "wlu.h"
#include "wlu_remote.h"
#include "wl_drv.h"

unsigned short defined_debug = DEBUG_ERR | DEBUG_INFO;

extern int remote_server_exec(int argc, char **argv, void *ifr);

int
rwl_create_dir(void)
{
	/* not supported */
	return 0;
}

int
remote_shell_execute(char *buf_ptr)
{
	/* not supported */
	UNUSED_PARAMETER(buf_ptr);
	return 0;
}

int
remote_shell_get_resp(char* shell_fname, void *wl)
{
	/* not supported */
	UNUSED_PARAMETER(shell_fname);
	UNUSED_PARAMETER(wl);
	return 0;
}
int
rwl_write_serial_port(void* hndle, char* write_buf, unsigned long size, unsigned long *numwritten)
{
	/* not invoked for dongle transports */
	UNUSED_PARAMETER(hndle);
	UNUSED_PARAMETER(write_buf);
	UNUSED_PARAMETER(size);
	UNUSED_PARAMETER(numwritten);
	return FAIL;
}

void*
rwl_open_transport(int remote_type, char *port, int ReadTotalTimeout, int debug)
{
	/* not invoked for dongle transports */
	UNUSED_PARAMETER(remote_type);
	UNUSED_PARAMETER(port);
	UNUSED_PARAMETER(ReadTotalTimeout);
	UNUSED_PARAMETER(debug);
	return NULL;
}

int
rwl_close_transport(int remote_type, void* Des)
{
	/* not invoked for dongle transports */
	UNUSED_PARAMETER(remote_type);
	UNUSED_PARAMETER(Des);
	return FAIL;
}

int
rwl_read_serial_port(void* hndle, char* read_buf, uint data_size, uint *numread)
{
	/* not invoked for dongle transports */
	UNUSED_PARAMETER(hndle);
	UNUSED_PARAMETER(read_buf);
	UNUSED_PARAMETER(data_size);
	UNUSED_PARAMETER(numread);
	return FAIL;
}

void
rwl_sleep(int delay)
{
	OSL_DELAY(delay * 1000);
}

void
rwl_sync_delay(uint noframes)
{
	if (noframes > 1) {
		rwl_sleep(200);
	}
}

int
wl_ioctl(void *wl, int cmd, void *buf, int len, bool set)
{
	int ret = 0;
	wl_drv_ioctl_t ioc;

	UNUSED_PARAMETER(wl);

	memset(&ioc, 0, sizeof(ioc));
	ioc.w.cmd = cmd;
	ioc.w.buf = buf;
	ioc.w.len = len;
	ioc.w.set = set;

	if (wl_drv_ioctl(NULL, &ioc) < 0) {
		ret = BCME_IOCTL_ERROR;
	}

	return (ret);
}

int
wlu_remote_server_main_args(int argc, char **argv)
{
	int err = 0;

	/* Main server process for all transport types */
	err = remote_server_exec(argc, argv, NULL);

	return err;
}
