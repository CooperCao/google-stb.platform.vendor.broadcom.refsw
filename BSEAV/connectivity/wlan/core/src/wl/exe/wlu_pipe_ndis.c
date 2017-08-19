/*
 * Ndis version of remote Wl transport mechanisms (pipes).
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

#define NEED_IR_TYPES

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
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

static rem_ioctl_t rem_cdc;
extern int g_rwl_device_baud;

int
remote_CDC_tx(void *wl, uint cmd, uchar *buf, uint buf_len, uint data_len, uint flags, int debug)
{
	uint numwritten;
	rem_ioctl_t *rem_ptr = &rem_cdc;
	uint i;

	rem_ptr->msg.cmd = cmd;
	rem_ptr->msg.len = buf_len;
	rem_ptr->msg.flags = flags;
	rem_ptr->data_len = data_len;

	if (data_len > buf_len) {
		printf("remote_CDC_tx: data_len (%d) > buf_len (%d)\n", data_len, buf_len);
		return (-1);
	}


	if (remote_type == REMOTE_SERIAL) {
		/* Write CDC header first */
		if (debug) {
			printf("remote_CDC_tx(), sending header=\n");
			for (i = 0; i < sizeof(rem_ioctl_t); i++) {
				if (i % 24 == 0) {
					printf("\n");
				}
				printf("%02X ", *(((char *)rem_ptr) + i));
			}
			printf("\n");
		}
		else {
			Sleep(50); /* for serial port flow control */
		}
		if (WriteFile(wl, (char *)rem_ptr, REMOTE_SIZE, &numwritten, NULL) == 0) {
			printf("CDC_Tx: Header: Write failed with error %d\n", GetLastError());
			return (-1);
		}
		if (numwritten != REMOTE_SIZE) {
			printf("CDC_Tx: Header: numwritten %d != REMOTE_SIZE %d\n",
			numwritten, REMOTE_SIZE);
			return (-1);
		}

		if (debug) {
			printf("remote_CDC_tx(), sending %d bytes data=\n", data_len);
			for (i = 0; i < data_len; i++) {
				if (i % 24 == 0) {
					printf("\n");
				}
				printf("%02X ", buf[i]);
			}
			printf("\n");
		}
		else {
			Sleep(50);
		}

		/* Write data second */
		if (WriteFile(wl, buf, data_len, &numwritten, NULL) == 0) {
			printf("CDC_Tx: Data: Write failed with error %d\n", GetLastError());
			return (-1);
		}
		if (numwritten != data_len) {
			printf("CDC_Tx: Data: numwritten %d != buf_len %d\n", numwritten, buf_len);
			return (-1);
		}
		return (buf_len);	/* Return size of actual buffer to satisfy accounting
					   going on above this level
					 */
	}
	return (0);
}

rem_ioctl_t *
remote_CDC_rx_hdr(void *remote, int debug)
{
	uint numread;
	rem_ioctl_t *rem_ptr = &rem_cdc;

	if (remote_type == REMOTE_SERIAL) {
		if (ReadFile(remote, (char *)rem_ptr, sizeof(rem_ioctl_t), &numread, NULL) == 0) {
			printf("remote_CDC_rx_hdr: Header Read failed with error %d\n",
				GetLastError());
			return (NULL);
		}
		if (numread != sizeof(rem_ioctl_t)) {
			printf("remote_CDC_rx_hdr: wanted to read %d but read %d\n",
				sizeof(rem_ioctl_t), numread);
			return (NULL);
		}

		if (debug) {
			char *buf_start;
			uint i;
			buf_start = (char *)rem_ptr;
			printf("remote_CDC_rx_hdr(), received %d bytes as following:", numread);
			for (i = 0; i < numread; i++) {
				if (i % 32 == 0)
					printf("\n");
				printf("%02x ", buf_start[i]);
			}
			printf("\n");
		}
	}
	return (rem_ptr);
}

/* Return a CDC type buffer */
int
remote_CDC_rx(void *wl, rem_ioctl_t *rem_ptr, uchar *readbuf, uint buflen, int debug)
{
	uint numread, total_numread;
	uchar *buf_start;

	buf_start = readbuf;

	if (remote_type == REMOTE_SERIAL) {
		if (rem_ptr->data_len > rem_ptr->msg.len) {
			printf("remote_CDC_rx: remote data len (%d) > msg len (%d)\n",
				rem_ptr->data_len, rem_ptr->msg.len);
			return (-1);
		}
		if (rem_ptr->msg.len > buflen) {
			printf("remote_CDC_rx: remote msg len (%d) > buflen (%d)\n",
				rem_ptr->msg.len, buflen);
			return (-1);
		}
		total_numread = 0;

		while (total_numread < rem_ptr->data_len) {
			if (ReadFile(wl, readbuf, rem_ptr->data_len - total_numread,
				&numread, NULL) == 0) {
				printf("remote_CDC_rx: Data Read failed with error %d\n",
					GetLastError());
			}
			if (numread != rem_ptr->data_len - total_numread) {
				printf("remote_CDC_rx: asked for %d bytes got %d bytes\n",
					rem_ptr->data_len - total_numread, numread);
				return (-1);
			}
			total_numread += numread;
			readbuf += numread;
		}

		if (debug) {
			uint i;

			printf("remote_CDC_rx(), received %d bytes as following:", total_numread);
			for (i = 0; i < total_numread; i++) {
				if (i % 32 == 0)
					printf("\n");
				printf("%02x ", buf_start[i]);
			}
			printf("\n");
		}
	}
	return (0);
}

HANDLE
open_serial(char *port, int ReadTotalTimeout, int debug, int baud)
{
	DCB dcb;
	COMMTIMEOUTS timeouts;
	HANDLE hCom;

	hCom = CreateFile(port,
		GENERIC_READ | GENERIC_WRITE,
		0,    		/* must be opened with exclusive-access */
		NULL, 		/* no security attributes */
		OPEN_EXISTING, 	/* must use OPEN_EXISTING */
		0,    		/* not overlapped I/O */
		NULL);  		/* hTemplate must be NULL for comm devices */

	if (hCom == INVALID_HANDLE_VALUE) {
		printf("CreateFile failed with error %d.\n", GetLastError());
		return (NULL);
	}

	if (GetCommState(hCom, &dcb) == 0) {
		printf("GetCommState failed with error %d.\n", GetLastError());
		return (NULL);
	}

	dcb.BaudRate = baud;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	if (SetCommState(hCom, &dcb) == 0) {
		printf("SetCommState failed with error %d.\n", GetLastError());
		return (NULL);
	}
	if (GetCommTimeouts(hCom, &timeouts) == 0) {
		printf("GetCommTimeouts failed with error %d.\n", GetLastError());
		return (NULL);
	}

	/* Set timeout so serial reads are syncronous.  For now anyway */
	/* in milliseconds. Just guessing */
	timeouts.ReadIntervalTimeout = 20;
	if (debug) {
		printf("timeouts.ReadIntervalTimeout=%d\n",
		       timeouts.ReadIntervalTimeout);
		printf("timeouts.ReadTotalTimeoutMultiplier=%d\n",
		       timeouts.ReadTotalTimeoutMultiplier);
		printf("timeouts.ReadTotalTimeoutConstant=%d\n",
		       timeouts.ReadTotalTimeoutConstant);
		printf("timeouts.WriteTotalTimeoutMultiplier=%d\n",
		       timeouts.WriteTotalTimeoutMultiplier);
		printf("timeouts.WriteTotalTimeoutConstant=%d\n",
		       timeouts.WriteTotalTimeoutConstant);
	}

	if (SetCommTimeouts(hCom, &timeouts) == 0) {
		printf("SetCommTimeouts failed with error %d.\n", GetLastError());
		return (NULL);
	}
	return (hCom);
}

void *
open_pipe(int remote_type, char *port, int ReadTotalTimeout, int debug)
{
	HANDLE irh;
	char port_str[10] = "\\\\.\\COM\0";

	strcat(port_str, port);

	if (debug) {
		printf("open_pipe(): port_str=%s\n", port_str);
	}

	switch (remote_type) {
	case REMOTE_SERIAL:
		if ((irh = open_serial(port_str, ReadTotalTimeout, debug,
				g_rwl_device_baud)) == NULL) {
			printf("Can't open serial port\n");
		}
		break;
	default:
		printf("open_pipe: Unknown remote_type %d\n", remote_type);
		irh = NULL;
		break;
	}
	return irh;
}
