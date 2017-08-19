/*
 * NDIS port of dhd command line utility.
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

#include <winsock2.h>
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
#include <proto/802.11.h>
#include <oidencap.h>
#include <bcmcdc.h>
#include <typedefs.h>
#include <dhdioctl.h>
#include <signal.h>

#if defined(RWL_WIFI) || defined(RWL_DONGLE) || defined(RWL_SOCKET) \
	||defined(RWL_SERIAL)
#define RWL_ENABLE
#endif /* RWL_WIFI | RWL_DONGLE | RWL_SOCKET | RWL_SERIAL */

#include "dhdu.h"
#ifdef RWL_ENABLE
#include "wlu_remote.h"
#include "wlu_client_shared.h"
#include "wlu_pipe.h"
#endif /* RWL_ENABLE */
#include "dhdu_common.h"
#ifdef DHD_VISTA
#include "wlu_remote_vista.h"
#endif /* DHD_VISTA */

static int rwl_os_type = WIN32_OS;
#ifdef DHD_VISTA
GUID devlist[10];
GUID dev;
DWORD ndevs;
#else
static DWORD ndevs;
static ADAPTER devlist[10];
#endif
HANDLE hWaitThread;
HANDLE g_killThreadEvent;

#ifdef RWL_SOCKET
/* to validate hostname/ip given by the client */
int validate_server_address()
{
	struct hostent *he;
	struct ipv4_addr temp;
	if (!dhd_atoip(g_rwl_servIP, &temp)) {
	/* Wrong IP address format check for hostname */
		if ((he = gethostbyname(g_rwl_servIP)) != NULL) {
			if (!dhd_atoip(*he->h_addr_list, &temp)) {
				g_rwl_servIP =
				inet_ntoa(*(struct in_addr *)*he->h_addr_list);
				if (g_rwl_servIP == NULL) {
					DPRINT_ERR(ERR, "Error at inet_ntoa \n");
					return FAIL;
				}
			} else {
				DPRINT_ERR(ERR, "Error in IP address \n");
				return FAIL;
			}
		} else {
			DPRINT_ERR(ERR, "Enter correct IP address/hostname format\n");
			return FAIL;
		}
	}
	return SUCCESS;
}
#endif /* RWL_SOCKET */

#ifdef RWL_ENABLE
/* This function handles ctr+c and sends server to stop the process */
void rwl_shell_ctrlcHandler(void *wl)
{
	uchar *input_buf = NULL;
	int err;
	signal(SIGINT, ctrlc_handler);
	while ((g_sig_ctrlc) && (WaitForSingleObject(g_killThreadEvent, 0) != WAIT_OBJECT_0));
	if (!g_sig_ctrlc)
		err = remote_CDC_tx(wl, -1, input_buf, 0, 0, CTRLC_FLAG, 0);
	return;
}
/* closes thread handle in case of normal exit of the process */
void rwl_shell_killproc(int pid)
{
	SetEvent(g_killThreadEvent);
	if (hWaitThread != NULL)
		CloseHandle(hWaitThread);
	return;
}

/* creates thread which handles ctr+c */
int rwl_shell_createproc(void *wl)
{
	DWORD RecvPartialThread;

	if ((g_killThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == 0) {
		DPRINT_DBG(OUTPUT, "\n create event failed = %d\n", GetLastError());
		}
	hWaitThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)rwl_shell_ctrlcHandler,
	(LPVOID)wl, 0, &RecvPartialThread);
	if (hWaitThread == NULL) {
		DPRINT_DBG(OUTPUT, "\n shell partial not created\n");
	}
	return 1;
}
#else
/* dummy functions for wince client */
int rwl_shell_createproc(void *wl)
{
	return 1;
}

void rwl_shell_killproc(int pid)
{
	return;
}

#endif /* RWL_ENABLE */

/* dhd_get/dhd_set is called by several functions in dhdu.c. This used to call dhd_ioctl
 * directly. However now we need to execute the dhd commands remotely.
 * So we make use of wl pipes to execute this.
 * wl_get or wl_set functions also check if it is a local command hence they inturn
 * call ir_set/query info if required. Name wl_get/wl_set is retained because these functions are
 * also called by wlu_pipe.c wlu_client_shared.c
 */
int
dhd_get(void *dhd, int cmd, void *buf, int len)
{
	return wl_get(dhd, cmd, buf, len);
}

int
dhd_set(void *dhd, int cmd, void *buf, int len)
{
	return wl_set(dhd, cmd, buf, len);
}

/* The function is replica of wl_get in wlu_ndis.c. Optimize when we have some
 * common code between wlu_ndis.c and dhdu_ndis.c
 */
int
wl_get(void *wl, int cmd, void *buf, int len)
{
	DWORD dwlen = len;
	int error;
	/* rwl_os_type specifies RemoteWL server OS.
	 * remote)type specifies which type of transport (or none for local mode).
	 */
	if ((rwl_os_type == LINUX_OS) && (remote_type != NO_REMOTE)) {
		error = (int)ir_queryinformation_fe((HANDLE)wl, cmd, buf, &dwlen);
	} else {
		error = (int)ir_queryinformation_fe((HANDLE)wl, WL_OID_BASE+cmd, buf, &dwlen);

	}
	if (error != 0) {
		return BCME_IOCTL_ERROR;
	}
	return 0;

}

/* The function is replica of wl_set in wlu_ndis.c. Optimize when we have some
 * common code between wlu_ndis.c and dhdu_ndis.c
 */
int
wl_set(void *wl, int cmd, void *buf, int len)
{
	DWORD dwlen = len;
	int error;
	/* rwl_os_type specifies RemoteWL server OS.
	 * remote)type specifies which type of transport (or none for local mode).
	 */
	if ((rwl_os_type == LINUX_OS) && (remote_type != NO_REMOTE)) {
		error = (int)ir_setinformation_fe((HANDLE)wl, cmd, buf, &dwlen);
	} else {

		error = (int)ir_setinformation_fe((HANDLE)wl, WL_OID_BASE+cmd, buf, &dwlen);

	}
	if (error != 0) {
		return BCME_IOCTL_ERROR;
	}
	return 0;
}

/* This function is replica of wlu_ndis.c. Optimize when we have common
 * code between wlu_ndis.c and dhdu_ndis.c
 */
static void
wl_close_rwl(int remote_type, HANDLE irh)
{
	switch (remote_type) {
	case NO_REMOTE:
	case REMOTE_WIFI:
		if (irh != INVALID_HANDLE_VALUE) {
#ifdef DHD_VISTA
			ir_vista_exit(irh);
#else
			ir_unbind(irh);
			ir_exit(irh);
#endif
		}
		break;
	case REMOTE_SOCKET:
#ifdef RWL_SOCKET
		rwl_terminate_ws2_32dll();
#endif /* RWL_SOCKET */
		break;
	case REMOTE_SERIAL:
	case REMOTE_DONGLE:
#if defined(RWL_SERIAL) || defined(RWL_DONGLE)
		rwl_close_pipe(remote_type, irh);
#endif /* RWL_SERIAL | RWL_DONGLE */
		break;
	}
}

void
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
		fprintf(stderr, "%s: Error 0x%x: %s -- %s\n", dhdu_av0, status, prefix, ErrStr);
		LocalFree(ErrStr);
	} else
		fprintf(stderr, "%s: Error 0x%x: %s -- Unknown error\n", dhdu_av0, status, prefix);
}

int
geterror(void)
{
	return (int)GetLastError();
}

char *
strerror(WINERR status)
{
	static char errmsg[128];
	PCHAR   ErrStr;
	DWORD   ErrLen;

	ErrLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, status,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &ErrStr, 0, NULL);
	if (ErrLen > 0) {
		_snprintf(errmsg, sizeof(errmsg), "Error 0x%x (%s)", status, ErrStr);
		LocalFree(ErrStr);
	} else
		_snprintf(errmsg, sizeof(errmsg), "Error 0x%x (Unknown)", status);

	return errmsg;
}

#ifdef DHD_VISTA
/* select the numbered adapter */
WINERR
select_adapter(HANDLE irh, int adapter)
{
	ULONG i;
	WINERR status;

	/* If adapter == -1, choose the first appropriate adapter. */
	if (adapter == -1) {
		for (i = 0; i < ndevs; i++) {
			dev = devlist[i];
			printf("%s: %d\n", __FUNCTION__, __LINE__);
			if (dhd_check((void *)irh) >= 0)
				return ERROR_SUCCESS;
		}

		if (i == ndevs) {
			fprintf(stderr, "%s: No BRCM wireless adapters were found\n", dhdu_av0);
			return ERROR_INVALID_HANDLE;
		}
	}

	status = ERROR_SUCCESS;
	if (adapter < 0 || (ULONG) adapter >= ndevs) {
		fprintf(stderr, "%s: Cannot find wireless adapter #%d\n", dhdu_av0, adapter);
		status = ERROR_INVALID_HANDLE;
	} else {
		dev = devlist[adapter];
		if (dhd_check((void *)irh) < 0) {
			fprintf(stderr, "%s: Selected adapter #%d is not an BRCM "
				"wireless adapter\n", dhdu_av0, adapter);
			status = ERROR_INVALID_HANDLE;
		}
	}

	return status;
}
#else
static WINERR
select_adapter(HANDLE irh, int adapter)
{
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
			fprintf(stderr, "%s: No wireless adapters were found\n", dhdu_av0);
			return ERROR_INVALID_HANDLE;
		}
	}

	if (adapter < 0 || (ULONG) adapter >= ndevs) {
		fprintf(stderr, "%s: Cannot find wireless adapter #%d\n", dhdu_av0, adapter);
		status = ERROR_INVALID_HANDLE;
	} else {
		pdev = &devlist[adapter];
		if (pdev->type != IR_WIRELESS) {
			fprintf(stderr,
			        "%s: Selected adapter #%d is not an BRCM wireless adapter\n",
			        dhdu_av0, adapter);
			status = ERROR_INVALID_HANDLE;
		} else {
			status = ir_bind(irh, pdev->name);
			if (status != ERROR_SUCCESS)
				display_err("select_adapter:ir_bind:", status);
		}
	}

	return status;
}
#endif /* DHD_VISTA */

/* This function is replica of wlu_ndis.c
 * Front end for ir_setinformation
 * However for local dhd commans we check if it is not a remote/not a wifi
 * get set command. In this case add OID_DHD_IOCTLS to execute the local
 * dhd command
 */
static int
ir_queryinformation_fe(void *wl, int cmd, void* input_buf, DWORD *input_len)
{
	int error = BCME_OK;
	uint tx_len = MIN(*input_len, 1024);

	if ((remote_type == NO_REMOTE) && (strncmp(input_buf, RWL_WIFI_ACTION_CMD,
	strlen(RWL_WIFI_ACTION_CMD))) && (strncmp(input_buf, RWL_WIFI_GET_ACTION_CMD,
	strlen(RWL_WIFI_GET_ACTION_CMD)))) {
		cmd = cmd - WL_OID_BASE + OID_DHD_IOCTLS;
	}

	if (remote_type == NO_REMOTE) {
#ifdef DHD_VISTA
		error = (int)ir_vista_queryinformation((HANDLE)wl, &dev, cmd, input_buf, input_len);
#else
		error = (int)ir_queryinformation((HANDLE)wl, cmd, input_buf, input_len);
#endif
	}
#ifdef RWL_ENABLE
	else {
		/* rem_ioctl_t *rem_ptr; */
		error = rwl_queryinformation_fe(wl, cmd, input_buf, input_len, debug,
		RDHD_GET_IOCTL);
	}
#else /* RWL_ENABLE */
	error = FAIL;
#endif /* RWL_ENABLE */
	return error;
}

/* This function is replica of wlu_ndis.c
 * Front end for ir_setinformation
 * However for local dhd commans we check if it is not a remote/not a wifi
 * set command. In this case add OID_DHD_IOCTLS to execute the local
 * dhd command
 */
static int
ir_setinformation_fe(void *wl, int cmd, void* buf, DWORD *len)
{
	int error;
	if ((remote_type == NO_REMOTE) && (strncmp(buf, RWL_WIFI_ACTION_CMD,
	strlen(RWL_WIFI_ACTION_CMD)))) {
		cmd = cmd - WL_OID_BASE + OID_DHD_IOCTLS;
	}
	if (remote_type == NO_REMOTE) {
#ifdef DHD_VISTA
		error = (int)ir_vista_setinformation((HANDLE)wl, &dev, cmd, buf, len);
#else
		error = (int)ir_setinformation((HANDLE)wl, cmd, buf, len);
#endif
	}
#ifdef RWL_ENABLE
	else {
		error = rwl_setinformation_fe(wl, cmd, buf, len, debug, RDHD_SET_IOCTL);
	}
#else /* RWL_ENABLE */
	error = FAIL;
#endif /* RWL_ENABLE */
	return error;
}

/* Verify the wl adapter found.
 * This is called by Process_args to check if it supports wl
 * The reason for checking wl adapter is that we can still send remote dhd commands over
 * wifi transport. The function is replica from wlu.c. Optimize later
 * when we have some common code between wlu_ndis.c and dhdu_ndis.c
 */
int
wl_check(void *wl)
{
	int ret;
	int val;

	if ((ret = wl_get(wl, WLC_GET_MAGIC, &val, sizeof(int))) < 0)
		return ret;
#ifdef IL_BIGENDIAN
	/* Detect if IOCTL swapping is necessary */
	if (val == bcmswap32(WLC_IOCTL_MAGIC))
	{
		val = bcmswap32(val);
		swap = TRUE;
	}
#endif
	if (val != WLC_IOCTL_MAGIC)
		return -1;
	if ((ret = wl_get(wl, WLC_GET_VERSION, &val, sizeof(int))) < 0)
		return ret;
	val = dtoh32(val);
	if (val > WLC_IOCTL_VERSION) {
		fprintf(stderr, "Version mismatch, please upgrade\n");
		return -1;
	}
	return 0;
}

/* Main client function
 * The code is mostly from wlu_ndis.c. This function takes care of executing remote dhd commands
 * along with the local dhd commands now.
 */
int
main(int argc, char **argv)
{
	HANDLE irh;
	WINERR err = ERROR_SUCCESS;
#ifdef RWL_SERIAL
	char port_no[3];
	char* endptr;
#endif /* RWL_SERIAL */

	dhdu_av0 = argv[0];
	argv++; 	/* skip past name argument */

	/* NDIS RemoteWl client seeks linux server */
	if (*argv && (strncmp (*argv, "--linuxdut", strlen(*argv)) == 0 || strncmp(*argv,
		"--linux", strlen(*argv)) == 0)) {
		rwl_os_type = LINUX_OS;
		argv++;
	}
	if (*argv && strncmp (*argv, "--debug", strlen(*argv)) == 0) {
		debug = 1;
		argv++;
	}

	/* RWL Serial transport (system serial to system serial) */
	if (*argv && strncmp (*argv, "--serial", strlen(*argv)) == 0) {
		remote_type = REMOTE_SERIAL;
		argv++;
#ifdef RWL_SERIAL
		if (!(*argv)) {
			rwl_usage(remote_type);
			return err;
		}
		memset(port_no, 0, 3);
		strcpy(port_no, *argv);
		if (debug) {
			printf("main(), port_no=%s\n", port_no);
		}

		argv++;
		if (!(*argv)) {
			rwl_usage(remote_type);
			return err;
		}

		if (*argv && strncmp (*argv, "--ReadTimeout", strlen(*argv)) == 0) {
			argv++;
			if (!(*argv)) {
				rwl_usage(remote_type);
				return err;
			}
			ReadTotalTimeout = strtoul(*argv, &endptr, 0);
			if (*endptr != '\0') {
				/* not all the value string was parsed by strtoul */
				printf("Reading ReadTimeout failed, exit.\n");
				return -1;
			}
			argv++;
		}
		if (debug) {
			printf("main(), ReadTimeout=%d\n", ReadTotalTimeout);
		}
		if ((irh = rwl_open_pipe(remote_type, port_no, ReadTotalTimeout, debug)) == NULL) {
			printf("rwl_open_pipe failed\n");
			return -1;
		}
#endif /* RWL_SERIAL */
	}

	/* RWL socket transport Usage: --socket ipaddr [port num] */

	if (*argv && strncmp (*argv, "--socket", strlen(*argv)) == 0) {
		remote_type = REMOTE_SOCKET;
		*argv++;
#ifdef RWL_SOCKET
		if (!(*argv)) {
			rwl_usage(remote_type);
			return err;
		}
		 g_rwl_servIP = *argv;
		 *argv++;

		g_rwl_servport = DEFAULT_SERVER_PORT;
		if (*argv && isdigit(**argv)) {
			/* User port num override */
			g_rwl_servport = atoi(*argv);
			*argv++;
		}
		rwl_init_ws2_32dll(); 	/* Initialise the Winsock DLL */
#endif /* RWL_SOCKET */
	}

	/* RWL from system serial port on client to uart dongle port on server */
	/* Usage: --dongle <COMX> */
	if (*argv && strncmp (*argv, "--dongle", strlen(*argv)) == 0) {
		remote_type = REMOTE_DONGLE;
		*argv++;
#ifdef RWL_DONGLE
		if (!(*argv)) {
			rwl_usage(remote_type);
			return err;
		}

		g_rwl_device_name_serial = *argv; /* COM1 OR COM2 */

		if ((irh = rwl_open_pipe(remote_type, "\0", 0, 0)) == NULL) {
			printf("rwl_open_pipe failed\n");
			return -1;
		}
		*argv++;
#endif /* RWL_DONGLE */
	}

	/* RWL over wifi.  Usage: --wifi mac_address */
	if (*argv && strncmp (*argv, "--wifi", strlen(*argv)) == 0) {
		remote_type = REMOTE_WIFI;
		*argv++;
#ifdef RWL_WIFI
		if (!(*argv)) {
			rwl_usage(remote_type);
			return err;
		}
		if (argc < 4) {
			rwl_usage(remote_type);
			return err;
		}
		if (!dhd_ether_atoe(*argv, (struct ether_addr *)g_rwl_buf_mac)) {
			fprintf(stderr, "Could not parse ethernet MAC address\n");
			return FAIL;
		}
		*argv++;
#endif /* RWL_WIFI */
	}

	if (remote_type == NO_REMOTE || remote_type == REMOTE_WIFI) {
		if (debug) {
			printf("remote_type == NO_REMOTE, need to check local device\n");
		}
		/* initialize irelay and select default adapter */
		irh = INVALID_HANDLE_VALUE;
#ifdef DHD_VISTA
		if ((err = ir_vista_init(&irh)) == ERROR_SUCCESS) {
			ndevs = ARRAYSIZE(devlist);
			if ((err = ir_vista_adapter_list(irh, devlist, &ndevs)) == ERROR_SUCCESS) {
				err = select_adapter(irh, -1);
			}
		}
#else
		if ((err = ir_init(&irh)) == ERROR_SUCCESS) {
			ndevs = ARRAYSIZE(devlist);
			if ((err = ir_adapter_list(irh, &devlist[0], &ndevs)) == ERROR_SUCCESS) {
				err = select_adapter(irh, -1);
			}
		}
#endif
	}
	else {
		if (debug) {
			printf("remote_type == REMOTE_SERIAL, no need to check local device\n");
		}
	}

	if (err == ERROR_SUCCESS) {
		/* No arguments?  Print help screen and bail. */
		if (!(*argv)) {
			dhd_usage(dhd_cmds);
			wl_close_rwl(remote_type, irh);
			return err;
		}
		if (remote_type == NO_REMOTE) {
			err = process_args(irh, argv);
		}
#ifdef RWL_ENABLE
		else {
		err = process_args(irh, argv);
	}
#endif /* RWL_ENABLE */
	}
	wl_close_rwl(remote_type, irh);
	return err;
}

/* The code is mostly from wlu_ndis.c. Optimize when we merge wlu_ndis.c and dhdu_ndis.c
 * Some changes are it uses dhd_option instead of wl_options, dhd_find_cmd instead of
 * wl_find_cmd. It also uses some dhd specific functions dhd_usage, dhd_printlasterror
 */
static WINERR
process_args(HANDLE irh, char **argv)
{
	char *ifname = NULL;
	int help = 0;
	int init_err = FALSE;
	int status = 0;
	WINERR err = ERROR_SUCCESS;
	cmd_t *cmd = NULL;

	while (*argv) {
#ifdef RWL_ENABLE
		if (remote_type != NO_REMOTE && strncmp (*argv, "sh",
			strlen(*argv)) == 0) {
			*argv++;
			if (*argv) {
				err = rwl_shell_cmd_proc((void *)irh, argv, SHELL_CMD);
			} else {
				DPRINT_ERR(ERR, "Enter the shell command (e.g sh dir \n");
				err = -1;
			}
			return err;
		}
#endif /* RWL_ENABLE */

		/* Check for help or specific interface (ifname) */
		if ((status = dhd_option(&argv, &ifname, &help)) == CMD_OPT) {
			/* print usage */
			if (help)
				break;
			/* select different adapter */
			if (ifname) {
#ifndef DHD_VISTA
				ir_unbind(irh);
#endif
				if ((err = select_adapter(irh, atoi(ifname))) != ERROR_SUCCESS)
					break;
				if (dhd_check((void *)irh))
					break;
			}
			continue;
		}

		/* Could not open new interface */
		else if (status == CMD_ERR)
			break;

		if (rwl_os_type == WIN32_OS) {
			/* Update the apmode status based on final selected adapter */
			if (debug) {
				printf("process_args(), ap_mode=%d\n", ap_mode);
			}
		}

		cmd = dhd_find_cmd(*argv);

		/* defaults to using the set_var and get_var commands */
		if (!cmd)
			cmd = &dhd_varcmd;

		/* do command */
		if (cmd->name)
			err = (WINERR) (*cmd->func)((void *) irh, cmd, argv);


		break;
	}

	 /* provide for help on a particular command */
	if (help && *argv) {
		cmd = dhd_find_cmd(*argv);
		if (cmd)
			dhd_cmd_usage(cmd);
		else {
			fprintf(stderr,
				"%s: Unrecognized command \"%s\", type -h for help\n",
				dhdu_av0, *argv);
		}
	}
	else if (!cmd) {
			dhd_usage(dhd_cmds);
		}
	else if (!cmd->name)
		fprintf(stderr, "%s: Wrong command, type -h for help\n", dhdu_av0);

	if (err != ERROR_SUCCESS) {
		if ((err == BCME_USAGE_ERROR) && cmd)
			dhd_cmd_usage(cmd);
		else if (err == BCME_IOCTL_ERROR)
			dhd_printlasterror((void *)irh);
		else
			display_err("main:", err);
	}

	return err;
}

int
wl_atoip(const char *a, struct ipv4_addr *n)
{
	return dhd_atoip(a, n);
}

/* Search the dhd_cmds table for a matching command name.
 * Return the matching command or NULL if no match found.
 */
static cmd_t *
dhd_find_cmd(char* name)
{
	cmd_t *cmd = NULL;
	/* search the dhd_cmds for a matching name */
	for (cmd = dhd_cmds; cmd->name && strcmp(cmd->name, name); cmd++);
	if (cmd->name == NULL)
		cmd = NULL;
	return cmd;
}
