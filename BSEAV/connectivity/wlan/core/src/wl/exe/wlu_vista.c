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

/* FILE-CSTYLED */

/* When generating the XML files in vista_build_schema the <> angle brackets
 * trigger false cstyle warnings
 */

#include <winsock2.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ntddndis.h>
#include <signal.h>

#include <typedefs.h>
#include <wlioctl.h>
#include <proto/ethernet.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include "wlu.h"
#include "wlu_remote.h"
#include "wlu_client_shared.h"
#include "wlu_pipe.h"

#include "wlanapi.h"
#include "oidencap.h"
#include "wlu_remote_vista.h"

DWORD Initialize();
DWORD SelectAdapter(HANDLE irh, char * ifname);
DWORD DeSelectAdapter();

#define NDIS_NUM_ARGS 32
#define NDIS_MAXLEN_ARGS	512

extern char* remote_vista_cmds[];
extern bool g_rwl_swap;
extern int vista_cmd_index;
extern char g_rem_ifname[IFNAMSIZ];

static bool debug = FALSE;
static int ReadTotalTimeout = 0;
int remote_type = NO_REMOTE;

/* RemoteWL declarations */
static int rwl_os_type = WIN32_OS;
static bool rwl_dut_autodetect = TRUE;
extern char *g_rwl_buf_mac;
extern char  *g_rwl_device_name_serial;
unsigned short g_rwl_servport;
char *g_rwl_servIP = NULL;
HANDLE hWaitThread;
HANDLE g_killThreadEvent;
#define RWL_WIFI_JOIN_DELAY 5000
extern int rwl_check_port_number(char *s, int len);

unsigned short defined_debug = DEBUG_ERR | DEBUG_INFO;

static void reg_query_mfgdhd_flag(void);
static cmd_t *wl_find_cmd(char* name);
static WINERR process_args(HANDLE irh, char **argv);
static int buf_to_args(char *line, char *new_args[]);
static void do_interactive(HANDLE irh);
static DWORD mfgdhd_flag = 0;

#define INTERACTIVE "interactive"
#define DASH_INTERACTIVE "--interactive"

#ifdef RWL_SOCKET
/* to validate hostname/ip given by the client */
int validate_server_address()
{
        struct hostent *he;
        struct ipv4_addr temp;
        if (!wl_atoip(g_rwl_servIP, &temp)) {
        /* Wrong IP address format check for hostname */
                if ((he = gethostbyname(g_rwl_servIP)) != NULL) {
                        if (!wl_atoip(*he->h_addr_list, &temp)) {
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

void wl_os_type_set_rwl(int os_type)
{
	rwl_os_type = os_type;
}

int
wl_ir_init_adapter_rwl(HANDLE *irh, int adapter)
{
	int err;
	GUID devlist[10];
	/* initialize irelay and select default adapter */
	*irh = INVALID_HANDLE_VALUE;
	if ((err = ir_vista_init(irh)) == ERROR_SUCCESS) {
		ndevs = (int)ARRAYSIZE(devlist);
		if ((err = ir_vista_adapter_list(*irh, &devlist[0], &ndevs)) == ERROR_SUCCESS) {
			err = select_adapter(*irh, adapter);
		}
	}

	return err;
}

void
wl_close_rwl(int remote_type, HANDLE irh)
{
        switch (remote_type){
        case NO_REMOTE:
        case REMOTE_WIFI:
        	DeSelectAdapter();
        	if (irh != INVALID_HANDLE_VALUE) {
                	ir_vista_exit(irh);
        	}
                break;
        case REMOTE_SOCKET:
#ifdef RWL_SOCKET
                rwl_terminate_ws2_32dll();
#endif
                break;
#if defined(RWL_SERIAL) || defined(RWL_DONGLE)|| defined(RWL_SOCKET)
        case REMOTE_SERIAL:
        case REMOTE_DONGLE:
                rwl_close_pipe(remote_type, irh);
                break;
#endif
        }
}

#if defined(BCMDLL) || defined(WLMDLL)
/*	In this case, this is linked as a library and hence there is no main
	(main is in the app, not the library).
	Example:  wl_lib("wl status");
*/
__declspec(dllexport)
wl_lib(char *input_str)
{
	HANDLE irh;
	WINERR err = ERROR_SUCCESS;
	char *tmp_argv[NDIS_NUM_ARGS];
	char **argv = tmp_argv;
	int argc;
	char **arguement = NULL, k = 0;
	char port_no[3];
	char* endptr;

	/* buf_to_args return 0 if no args or string too long
	 * or return NDIS_NUM_ARGS if too many args
	 */
	if (((argc = buf_to_args(input_str, argv)) == 0) || (argc == NDIS_NUM_ARGS)) {
		printf("wl:error: can't convert input string\n");
		return (-1);
	}
#else
int
main(int argc, char **argv)
{
	HANDLE irh;
	WINERR err = ERROR_SUCCESS;
	char port_no[3];
	char* endptr;
#endif /* BCMDLL */

	/* initialize irelay and select default wireless lan adapter */
	irh = INVALID_HANDLE_VALUE;
	wlu_av0 = argv[0];
	reg_query_mfgdhd_flag();

	wlu_init();

	argv++; /* Skip name of .exe */

        /* WINDOWS client looking for a BigEndian server */
        if (*argv && strncmp (*argv, "--be", strlen(*argv)) == 0) {
                g_rwl_swap = TRUE;
                *argv++;
        }

        /* NDIS RemoteWl client seeks linux server or checking indongle driver */
        if (*argv && (strncmp (*argv, "--linuxdut", strlen(*argv)) == 0 || strncmp(*argv,
                "--linux", strlen(*argv)) == 0)) {
                rwl_os_type = LINUX_OS;
                rwl_dut_autodetect = FALSE;
                argv++;
        }

        if (*argv && strncmp(*argv, "--indongle", strlen(*argv)) == 0) {
                rwl_os_type = INDONGLE;
                rwl_dut_autodetect = FALSE;
                argv++;
        }

        /* NDIS RemoteWl client seeks vista server */
        if (*argv && (strncmp (*argv, "--vistadut", strlen(*argv)) == 0 || strncmp(*argv,
                "--vista", strlen(*argv)) == 0)) {
                rwl_os_type = WINVISTA_OS;
                rwl_dut_autodetect = FALSE;
                argv++;
        }

        /* Provide option for disabling remote DUT autodetect */
        if (*argv && strncmp(*argv, "--nodetect", strlen(*argv)) == 0) {
                rwl_dut_autodetect = FALSE;
                argv++;
        }

        if (*argv && strncmp (*argv, "--debug", strlen(*argv)) == 0){
                debug = TRUE;
                argv++;
        }

        if (*argv && strncmp (*argv, "--serial", strlen(*argv)) == 0) {
                /* If running remote-wl over serial, use serial instead of local wl handle */
                remote_type = REMOTE_SERIAL;

                argv++;
                if (!(*argv)) {
                        rwl_usage(remote_type);
                        return err;
                }
                memset(port_no,0,3);
                strcpy(port_no,*argv);

                if (debug) {
                        printf("main(), port_no=%s\n", port_no);
                }

                argv++;
                if (!(*argv)) {
                        rwl_usage(remote_type);
                        return err;
                }

                if (*argv && strncmp (*argv, "--ReadTimeout", strlen(*argv)) == 0){
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
                        printf("main(), ReadTimeout=%d\n", ReadTotalTimeout );
                }
#if defined(RWL_SERIAL) || defined(RWL_DONGLE)|| defined(RWL_SOCKET)
                if ((irh = rwl_open_pipe(remote_type, port_no, ReadTotalTimeout, debug)) == NULL) {
                        printf("rwl_open_pipe failed\n");
                        return -1;
                }
#endif
        }

        /* RWL socket transport Usage: --socket ipaddr/hostname [port num] */
        if (*argv && strncmp (*argv, "--socket", strlen(*argv)) == 0) {
                remote_type = REMOTE_SOCKET;
                *argv++;
                if (!(*argv)) {
                        rwl_usage(remote_type);
                        return err;
                }
                /* IP address validation is done in client_shared file */
                g_rwl_servIP = *argv;
                 *argv++;

                 /* Port number validation is done in client_shared file */
                g_rwl_servport = DEFAULT_SERVER_PORT;
                if(*argv && (rwl_check_port_number(*argv, strlen(*argv)) == SUCCESS)) {
                        g_rwl_servport = atoi(*argv);
                        *argv++;
                }
#ifdef RWL_SOCKET
		rwl_init_ws2_32dll(); /* Initialise the Winsock DLL */
#endif
        }

        /* RWL from system serial port on client to uart dongle port on server */
        /* Usage: --dongle <COMX> */
        if (*argv && strncmp (*argv, "--dongle", strlen(*argv)) == 0) {
                remote_type = REMOTE_DONGLE;
                *argv++;
                if (!(*argv)) {
                        rwl_usage(remote_type);
                        return err;
                }

                g_rwl_device_name_serial = *argv; /* COM1 OR COM2 */
#if defined(RWL_SERIAL) || defined(RWL_DONGLE)|| defined(RWL_SOCKET)

                if ((irh = rwl_open_pipe(remote_type, "\0", 0, 0)) == NULL) {
                        printf("rwl_open_pipe failed\n");
                        return -1;
                }
#endif
                *argv++;
        }

        /* RWL over wifi.  Usage: --wifi mac_address */
        if (*argv && strncmp (*argv, "--wifi", strlen(*argv)) == 0) {
                remote_type = REMOTE_WIFI;
                *argv++;
                if (!(*argv)) {
                        rwl_usage(remote_type);
                        return err;
                }
                if (argc < 4) {
                        rwl_usage(remote_type);
                        return err;
                }
                if (!wl_ether_atoe(*argv, (struct ether_addr *)g_rwl_buf_mac)) {
                        fprintf(stderr, "Could not parse ethernet MAC address\n");
                        return FAIL;
                }
                *argv++;
        }

        if (remote_type == NO_REMOTE || remote_type == REMOTE_WIFI) {
                if (debug) {
                        printf("remote_type == NO_REMOTE, need to check local device\n");
                }
		if (!mfgdhd_flag) {
			/* Initialize the ACM mechanism */
			err = ir_vista_init(&irh);
			if (err == ERROR_SUCCESS){
				/* Try to Initialize through BCM42RLY.sys for advanced functionality*/
				/* Failure of this call will lead to legacy(only physical wireless adapter support) functionality */
				/* So no use of checking for failure */
				Initialize();
			}
			else {
				printf("%s:Initializaton failure\n", wlu_av0);
				goto done;
			}
		}
		else {
			/* MFG DHD requires BCM42RLY.sys */
			if (Initialize() != BCME_OK) {
				printf("%s:Initializaton failure\n", wlu_av0);
				goto done;
			}
		}

		err = SelectAdapter(irh, NULL);
		if(err != BCME_OK ) {
			printf("%s:No Broadcom Wireless Adapter found\n", wlu_av0);
			goto done;
		}
        }
        else {
                if (debug) {
                        printf("remote_type != NO_REMOTE, no need to check local device\n");
                }
        }

        if (err == ERROR_SUCCESS){
                /* No arguments?  Print help hint and bail. */
                if (!(*argv)){
			fprintf(stderr, "type -h for help\n");
			err = ERROR_BAD_COMMAND;
			goto done;
                }

                /* Autodetect remote DUT */
                if (remote_type != NO_REMOTE && rwl_dut_autodetect == TRUE) {
                        rwl_detect((void*)irh, debug, &rwl_os_type);
		}
		/* Interactive mode?  Accept either 'wl interactive' or 'wl --i*'
		The former needs to be spelled out to eliminate collisions
		with wl commands. The -- version can be abbrev since -- is unique.
		*/
		if (((strlen(*argv) > 2) && strncmp(*argv, DASH_INTERACTIVE, strlen(*argv)) == 0) ||
			(strncmp(*argv, INTERACTIVE, strlen(INTERACTIVE)) == 0)) {
			do_interactive(irh);
                } else {
                        /* RWL over Wifi supports 'lchannel' command which lets client
                         * (ie *this* machine) change channels since normal 'channel' command
                        * applies to the server (ie target machine) */
                        /* Execute a single, non-interactive command */
                        if (remote_type == REMOTE_WIFI && (!strcmp(argv[0], "lchannel"))) {
#ifdef RWL_WIFI
                               	strcpy(argv[0], "channel");
                                rwl_wifi_swap_remote_type(remote_type);
                                process_args(irh, argv);
                                rwl_wifi_swap_remote_type(remote_type);
#endif
                        } else {
                                err = process_args(irh, argv);
                        }
                }
        }
done:
	wl_close_rwl(remote_type, irh);

	return err;
}

static WINERR
process_args(HANDLE irh, char **argv)
{
	char *ifname = NULL;
	int help = 0, ifnum = -1;
	cmd_t *cmd = NULL;
	int status;
	WINERR err = ERROR_SUCCESS;

#ifdef RWL_WIFI
        int retry;
#endif
	while (*argv) {
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
                if (rwl_os_type == WINVISTA_OS) {
                        for (vista_cmd_index = 0; remote_vista_cmds[vista_cmd_index] &&
                                strcmp(remote_vista_cmds[vista_cmd_index],*argv); vista_cmd_index++);
                                if (remote_vista_cmds[vista_cmd_index] != NULL) {
                                        err = rwl_shell_cmd_proc((void *)irh, argv, VISTA_CMD);
                                if ((remote_type == REMOTE_WIFI) && ((!strcmp(*argv, "join")))) {
#ifdef RWL_WIFI
                                        DPRINT_INFO(OUTPUT, "\n Findserver is called to synchronize the channel\n\n");
                                        Sleep(RWL_WIFI_JOIN_DELAY);
                                        for (retry = 0; retry < RWL_WIFI_RETRY; retry++) {
                                                if ((rwl_find_remote_wifi_server(irh,
                                                &g_rwl_buf_mac[0]) == 0)) {
                                                        break;
                                                }
                                        }
#endif
                                }
                                return err;
                        }
                }

		/* command line option */
		if ((status = wl_option(&argv, &ifname, &help)) == CMD_OPT) {
			/* print usage */
			if (help)
				break;
			/* select different adapter */
			if (ifname) {
				if (remote_type == NO_REMOTE) {
					err = SelectAdapter(irh, ifname);
					/* select specified adapter */
					if (err != ERROR_SUCCESS) {
						printf("%s:Invalid Interface %s specified, use below format\n", wlu_av0, ifname);
						printf("%s -i 0 ver //Index from wl  xlist.\n",wlu_av0);
						printf("%s -i wl0 ver	//interface with inst, again from xlist.\n",wlu_av0);
						printf("%s -i vwl0 ver	//BRCM virtual.\n",wlu_av0);
						printf("%s -i msvwl0 ver	//MSFT Virtual.\n",wlu_av0);
						printf("%s -i aa:bb:cc:dd:ee:ff  ver   //Specifying Ethernet address.\n",wlu_av0);
						continue;
					}
				}else {
					strncpy(g_rem_ifname, ifname, IFNAMSIZ-1);
				}
			}
			continue;
		}

		else if (status == CMD_ERR)
			break;

		cmd = wl_find_cmd(*argv);

		/* defaults to using the set_var and get_var commands */
		if (!cmd)
			cmd = &wl_varcmd;

		/* If command is not a wc command, then don't do it. */
		if (!wc_cmd_check(cmd->name))
			cmd->name = NULL;
#ifdef RWL_WIFI
                if (!strcmp(cmd->name, "findserver")) {
                        remote_wifi_ser_init_cmds((void *) irh);
                }
#endif
		/* do command */
		if (cmd->name)
			err = (WINERR) (*cmd->func)((void *) irh, cmd, argv);
                /* Only when using RWL over Wifi: client is talking to server, client
                 * issues a join/ssid to server, server completes join and has switched to the
                 * APs channel.  Client and server are now on different channels and
                 * cannot communicate. Client needs to scan the channels to find its server.
                 */
                if ((remote_type == REMOTE_WIFI) && ((!strcmp(cmd->name, "join") ||
                        ((!strcmp(cmd->name, "ssid") && (*(++argv) != NULL)))))) {
#ifdef RWL_WIFI
                        DPRINT_INFO(OUTPUT, "\n Findserver is called to synchronize the channel\n\n");
                        Sleep(RWL_WIFI_JOIN_DELAY);
                        for (retry = 0; retry < RWL_WIFI_RETRY; retry++) {
                                if ((rwl_find_remote_wifi_server(irh,
                                &g_rwl_buf_mac[0]) == 0)) {
                                        break;
                                }
                        }
#endif
                }

		break;
	}

	if (help) {
		if (*argv) {
			cmd = wl_find_cmd(*argv);
			if (cmd)
				wl_cmd_usage(stdout, cmd);
			else {
				fprintf(stderr,
					"%s: Unrecognized command \"%s\", type -h for help\n",
					wlu_av0, *argv);
			}
		} else {
			/* outputs multiple screens of help information */
			wl_usage(stdout, wl_cmds);
			wl_usage(stdout, vista_cmds);
		}
	}
	else if (!cmd) {
		fprintf(stderr, "type -h for help\n");
		err = ERROR_BAD_COMMAND;
	}
	else if (!cmd->name)
		fprintf(stderr, "%s: Wrong command, type -h for help\n", wlu_av0);

	if (err != ERROR_SUCCESS) {
		if ((err == BCME_USAGE_ERROR) && cmd)
			wl_cmd_usage(stderr, cmd);
		else if (err == BCME_IOCTL_ERROR)
			wl_printlasterror((void *)irh);
		else if (err == BCME_NODEVICE)
			DPRINT_ERR(ERR, "%s : wl driver adapter not found\n", g_rem_ifname);
		/* Don't dump additional messages, BCME_BADARG caller already printed */
		else if (err == BCME_BADARG);
		else
			display_err("main:", err);
	}
	return err;
}

static void
do_interactive(HANDLE irh)
{
	char *local_argv[NDIS_NUM_ARGS];
	int i;
	char input_buf[NDIS_MAXLEN_ARGS];
        int wlitf = 0;
        WINERR err = ERROR_SUCCESS;

	while (1){
		int my_argc;
                int j;
		char **argv = local_argv;
		input_buf[0] = 0;

		do { 	/* Get a line of input, ignoring 0 length lines */
			printf("> ");
			fflush (0);        
#ifdef BCMDLL			
			ReadFile(dll_fd_in, input_buf, sizeof(input_buf) - 1, &i, 0);
			input_buf[i] = '\0';
#else
			fgets(input_buf, sizeof(input_buf), stdin);
#endif
			while (input_buf[0] &&
				(input_buf[strlen(input_buf)-1] == '\n' || input_buf[strlen(input_buf)-1] == '\r'))
					input_buf[strlen(input_buf)-1] = '\0';

		} while (strlen(input_buf) == 0);

		/* Convert user input into argv style,
		 * return 0 if no args or string too long
		 * return NDIS_NUM_ARGS if too many args
		 */
		if (((my_argc = buf_to_args(input_buf, local_argv)) == 0) ||
		    (my_argc == NDIS_NUM_ARGS))
			goto next;

                /*
                 * remove the quotes '"' from the beginning and ending of a string
                 */
                i = 0;
                while (local_argv[i]){
                        j = 0;
                        if (local_argv[i][j] && (local_argv[i][j] == '"')){
                                while (local_argv[i][j+1]){
                                        local_argv[i][j] = local_argv[i][j+1];
                                        j++;
                                }
                                if ((j > 0) && local_argv[i][j-1] == '"')
                                        local_argv[i][j-1] = '\0';
                                else
                                        local_argv[i][j] = '\0';
                        }
                        i++;
                }

#ifdef RWL_WIFI
                if (!strcmp(local_argv[0], "findserver")) {
                        remote_wifi_ser_init_cmds((void *) irh);
                }
#endif
		/* Exit? */
		if (!strcmp (local_argv[0], "q") || !strcmp (local_argv[0], "exit"))
			break;

                /* select different adapter */
                /* Execute command.  Strip off possible leading 'wl'
                 * since users are used to typing it */
                if (!strcmp (local_argv[0], "wl")){
                        argv++;
                }
                if (*argv) {
                                if (!strcmp(*argv, "-a") || !strcmp(*argv, "-i")) {
                                wlitf = 1;
                        } else {
                                if (wlitf == 1) {
                                        err = SelectAdapter(irh, NULL);
                                        if(err != BCME_OK ) {
                                                printf("%s:No Broadcom Wireless Adapter found\n", wlu_av0);
                                        }
                                }
                                wlitf = 0;
                        }
                } 

                /* Check for 'shell' cmd */
                if (remote_type != NO_REMOTE && strncmp (local_argv[0], "sh",
                        strlen(local_argv[0])) == 0)
                                process_args(irh, local_argv);
                        /* Execute command.  Strip off possible leading 'wl'
                         * since users are used to typing it
                        */
                else if (!strcmp (local_argv[0], "wl")) {
                        process_args(irh, argv);
                } else {
                        /* RWL over Wifi supports 'lchannel' command which lets client
                         * (ie *this* machine) change channels since normal 'channel' command
                         * applies to the server (ie target machine) */
                        if (remote_type == REMOTE_WIFI && (!strcmp(local_argv[0], "lchannel"))) {
#ifdef RWL_WIFI
                                strcpy(local_argv[0], "channel");
                                rwl_wifi_swap_remote_type(remote_type);
                                process_args(irh, local_argv);
                                rwl_wifi_swap_remote_type(remote_type);
#endif
                        } else {
                                process_args(irh, argv);
                        }
                }

next:
		/* free up malloced resources */
		for (i = 0; i < my_argc; i++) {
			if (local_argv[i]) {
				free(local_argv[i]);
				local_argv[i] = NULL;
			}
		}
	}
}

/* Search the vista_cmds and wl_cmds tables for a matching command name.
 * Return the matching command or NULL if no match found.
 */
static cmd_t*
wl_find_cmd(char* name)
{
	cmd_t *cmd = NULL;

	do {
		/* search the vista_cmds */
		if (remote_type == NO_REMOTE) {
			if (!mfgdhd_flag) {
				for (cmd = vista_cmds; cmd->name && strcmp(cmd->name, name); cmd++);

				if (cmd->name != NULL)
					break;
			}
		}

		/* search the wl_cmds */
		cmd = wlu_find_cmd(name);
	} while (0);

	return cmd;
}

/* Convert a single command line from user into argv/argc style buffers. */
static int
buf_to_args(char *line, char *new_args[])
{
	char *token;
	int argc = 0;

	while ((argc < (NDIS_NUM_ARGS - 1)) &&
	       ((token = strtok(argc ? NULL : line, " \t")) != NULL)) {
		new_args[argc] = malloc(strlen(token)+1);
		strncpy(new_args[argc], token, strlen(token)+1);
		argc++;
	}
	new_args[argc] = NULL;
	if (argc == (NDIS_NUM_ARGS - 1) && (token = strtok(NULL, " \t")) != NULL) {
		printf("wl:error: too many args; argc must be < %d\n", (NDIS_NUM_ARGS - 1));
		argc = NDIS_NUM_ARGS;
	}
	return argc;
}

#ifdef BCMDLL
HANDLE dll_fd_out, dll_fd_in;
void
__declspec(dllexport)
wl_lib_prep(HANDLE in_fd, HANDLE out_fd)
{
	dll_fd_out = out_fd;
	dll_fd_in = in_fd;
}
#endif

#ifdef BCMDLL
extern void
raw_puts(const char *buf, void *dll_fd_out)
{
	uint32 nbytes;
	WriteFile(dll_fd_out, buf, strlen(buf), &nbytes, 0);
}

/*	Can't figure out how to do variadic macros using cl
	so do it the hard way */
void
printf_to_fprintf(const char *fmt, ...)
{
	char myStr[5*1024];
	uint32 nbytes;
	va_list ap;
	va_start(ap, fmt);
	vsprintf(myStr, fmt, ap);
	va_end(ap);
	WriteFile(dll_fd_out, myStr, strlen(myStr), &nbytes, 0);
}

#define ERR_PREPEND_STR	"error: "
/* Send stderr to stdout */
void
fprintf_to_fprintf(FILE *stderror, const char *fmt, ...)
{
	char myStr[5*1024];
	uint32 nbytes;
	va_list ap;
	va_start(ap, fmt);
	vsprintf(myStr, fmt, ap);
	va_end(ap);
	WriteFile(dll_fd_out, ERR_PREPEND_STR, strlen(ERR_PREPEND_STR), &nbytes, 0);
	WriteFile(dll_fd_out, myStr, strlen(myStr), &nbytes, 0);
}
#endif /* BCMDLL */



void reg_query_mfgdhd_flag(void)
{
	HKEY	hKey = NULL;
	LONG	hRes;
	uint32	flagSize;

	hRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\BroadCom", 0, KEY_ALL_ACCESS, &hKey);
	if (ERROR_SUCCESS == hRes) {
		hRes = RegQueryValueEx(hKey, "MfgDhd", NULL, NULL, (BYTE*) &mfgdhd_flag, (DWORD*) &flagSize);
		RegCloseKey(hKey);
	}
}
