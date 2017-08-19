/*
 * Wl server for ndis
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
#include "wlu_pipe.h"


static void display_err(PCHAR prefix, WINERR status);
static WINERR select_adapter(HANDLE, int adapter);
static DWORD ndevs;
static ADAPTER devlist[10];
int remote_type = NO_REMOTE;

const char *wlu_av0 = "";
static int debug = 0;
unsigned short defined_debug = DEBUG_ERR;
int
main(int argc, char **argv)
{
	HANDLE irh;	/* wireless handle */
	HANDLE remote;	/* remote handle */
	WINERR err = ERROR_SUCCESS;
	DWORD dwlen;
	int download_flag;
	rem_ioctl_t *rem_ptr;
	uint len;
	uint32 start;
	uint i;
	char port_no[3];

	wlu_av0 = argv[0];

	if (argc < 2) {
		fprintf(stdout, "%s [--debug] [--serial port| serial ]\n", wlu_av0);
		return (0);
	}

	/* initialize irelay and select default adapter */
	irh = INVALID_HANDLE_VALUE;
	if ((err = ir_init(&irh)) == ERROR_SUCCESS) {
		ndevs = ARRAYSIZE(devlist);
		if ((err = ir_adapter_list(irh, &devlist[0], &ndevs)) == ERROR_SUCCESS) {
			err = select_adapter(irh, -1);
		}
	}

	if (err == ERROR_SUCCESS) {
		argv++; 	/* skip past name argument */

		if ((strncmp (*argv, "--debug", strlen(*argv)) == 0)) {
			debug = 1;
			argv++; 	/* skip past name argument */
		}

		if (((strlen(*argv) > 2) && strncmp (*argv, "--serial", strlen(*argv)) == 0)) {
			remote_type = REMOTE_SERIAL;

			argv++;
			memset(port_no, 0, 3);
			strcpy(port_no, *argv);

			if (debug) {
				printf("main(), port_no=%s\n", port_no);
			}

			if ((remote = rwl_open_pipe(remote_type, port_no, 0, debug)) == NULL) {
				return 0;
			}
		}

		/* Forever: read, process, send back results */
		while (1) {
			int error;
			uchar *buf_ptr;

			buf_ptr = NULL;
			/* Get CDC header in order to determine buffer requirements */
			if ((rem_ptr = remote_CDC_rx_hdr(remote, debug)) == NULL) {
				printf("Reading CDC header failed\n");
				continue;
			}

			if ((buf_ptr = malloc(rem_ptr->msg.len)) == NULL) {
				printf("malloc of %d bytes failed\n", rem_ptr->msg.len);
				continue;
			}

			/* Get data */
			if ((remote_CDC_rx(remote, rem_ptr,
			                   buf_ptr, rem_ptr->msg.len, debug)) == -1) {
				printf("Reading CDC %d data bytes failed\n", rem_ptr->msg.len);
				continue;
			}

			if (debug) {
				printf("received a %s cmd 0x%0x\n",
				       (rem_ptr->msg.flags == REMOTE_GET_IOCTL) ?
				       "REMOTE_GET_IOCTL" : "REMOTE_SET_IOCTL",
				       rem_ptr->msg.cmd);
				if (rem_ptr->msg.cmd == WLC_GET_VAR) {
					printf("received a WLC_GET_VAR command from client, "
					       "varname=%s\n", buf_ptr);
				}
				else if (rem_ptr->msg.cmd == WLC_SET_VAR) {
					printf("received a WLC_SET_VAR command from client, "
					       "varname=%s\n", buf_ptr);
				}
			}

			/* Process command */
			if (rem_ptr->msg.flags & REMOTE_SET_IOCTL) {
				dwlen = rem_ptr->msg.len;

				if (strstr(buf_ptr, "init")) {
					error = 0;
					buf_ptr = NULL;
				}
				else if (strstr(buf_ptr, "download")) {
					buf_ptr += strlen("download")+1;
					memcpy(&download_flag, buf_ptr, sizeof(int));
					if (debug) {
						printf("download flag =%d\n", download_flag);
					}
					error = 0;
					buf_ptr = NULL;
				}
				else if (strstr(buf_ptr, "membytes")) {
					if (debug) {
						for (i = 0; i < rem_ptr->data_len; i++) {
							if (i%4 == 0) {
								printf("\n");
							}
							printf("%02X ", buf_ptr[i]);
						}
						printf("\n");
					}
					/* print out the content for checking */
					buf_ptr += (strlen("membytes") + 1);
					memcpy(&start, buf_ptr, sizeof(int));
					if (debug) {
						for (i = 0; i < 4; i++) {
							if (i%4 == 0) {
								printf("\n");
							}
							printf("%02X ", buf_ptr[i]);
						}
						printf("\n");
					}
					buf_ptr += sizeof(int);
					if (debug) {
						for (i = 0; i < 4; i++) {
							if (i%4 == 0) {
								printf("\n");
							}
							printf("%02X ", buf_ptr[i]);
						}
						printf("\n");
					}
					memcpy(&len, buf_ptr, sizeof(int));
					buf_ptr += sizeof(int);

					if (debug) {
						printf("membytes received: start=0x%0X, len=%d\n",
						       start, len);
					}

					if (len > 1024) {
						printf("wrong len 0x%0x\n", len);
						continue;
					}

					if (debug) {
						printf("the membytes content = %s\n", buf_ptr);
						for (i = 0; i < len; i++) {
							if (i%4 == 0) {
								printf("\n");
							}
							printf("%02X ", buf_ptr[i]);
						}
						printf("\n");
					}

					error = 0;
					buf_ptr = NULL;
				}
				else /* pass the command to wl */
				{
					error = (int)ir_setinformation(irh,
					                               rem_ptr->msg.cmd,
					                               buf_ptr, &dwlen);
				}

				/* Send back error code to client */
				if (remote_CDC_tx(remote, error,
				                  buf_ptr, 0, 0, REMOTE_REPLY, debug) < 0)
					printf("wl_server: Return error code failed\n");
			}


			if (rem_ptr->msg.flags & REMOTE_GET_IOCTL) {
				dwlen = rem_ptr->msg.len;

				error = (int)ir_queryinformation(irh,
				                                 rem_ptr->msg.cmd,
				                                 buf_ptr, &dwlen);

				/* Send results back to client */
				if (remote_CDC_tx(remote, error, buf_ptr, dwlen,
					dwlen, REMOTE_REPLY, debug) < 0)
					printf("wl_server: Return results failed\n");
			}

			if (buf_ptr)
				free(buf_ptr);
		}
	}

	/* close wireless */
	if (irh != INVALID_HANDLE_VALUE) {
		ir_unbind(irh);
		ir_exit(irh);
	}

	/* close remote */
	if (remote_type == REMOTE_SERIAL)
		CloseHandle(remote);

	return err;
}

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
			fprintf(stderr, "%s: No wireless adapters were found\n", wlu_av0);
			return ERROR_INVALID_HANDLE;
		}
	}

	if (adapter < 0 || (ULONG) adapter >= ndevs) {
		fprintf(stderr, "%s: Cannot find wireless adapter #%d\n", wlu_av0, adapter);
		status = ERROR_INVALID_HANDLE;
	} else {
		pdev = &devlist[adapter];
		if (pdev->type != IR_WIRELESS) {
			fprintf(stderr,
			        "%s: Selected adapter #%d is not an BRCM wireless adapter\n",
			        wlu_av0, adapter);
			status = ERROR_INVALID_HANDLE;
		} else {
			status = ir_bind(irh, pdev->name);
			if (status != ERROR_SUCCESS)
				display_err("select_adapter:ir_bind:", status);
		}
	}

	return status;
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
		fprintf(stderr, "%s: Error 0x%x: %s -- %s\n", wlu_av0, status, prefix, ErrStr);
		LocalFree(ErrStr);
	} else
		fprintf(stderr, "%s: Error 0x%x: %s -- Unknown error\n", wlu_av0, status, prefix);
}
