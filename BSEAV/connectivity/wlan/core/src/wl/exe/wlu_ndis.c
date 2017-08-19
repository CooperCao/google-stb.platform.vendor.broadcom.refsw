/*
 * Windows port of wl command line utility
 *
 * Copyright 2002, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 * Now the command line could be really long:
 * wl.exe [--linuxdut/linux] [--debug] [--serial port_no] [--ReadTimeout n] [--interactive] [--clientbatch]
 * where:
 * 		--linuxdut/linux removes the WLC_OID_BASE when sending the IOCTL command
  * 		--debug prints out tx packets sending down the serial line, and other debug info
 * 		--serial enables the remoteWL with serial port number
 *		--ReadTimeout is applied only when WL is used in remote mode, to specify the ReadTotalTimeoutConstant
 * 		--interactive enables using WL in interactive mode
 * 		--clientbatch enables command batchinng on the client, the default is batching on driver
 *
 * $Id$

 */

#define NEED_IR_TYPES

#include <winsock2.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winioctl.h>
#include <malloc.h>
#include <assert.h>
#include <ntddndis.h>
#include <signal.h>

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
#include "wlu_client_shared.h"
#include "wlu_pipe.h"
#if defined(WINPE)
#include "hwdiags_winpe.h"
#include <signal.h>
#endif

extern char* remote_vista_cmds[];
extern bool g_rwl_swap;
extern int vista_cmd_index;
extern char g_rem_ifname[IFNAMSIZ];
/* FILE-CSTYLED */
#define CHECK_STATUS(prefix, status)  do { if (status != NO_ERROR) display_err(prefix, status); } \
		while (0)


/* dword align allocation */
static union {
	char bufdata[WLC_IOCTL_MAXLEN];
	uint32 alignme;
}bufstruct_wlu;

static char *xpbuf = (char*) &bufstruct_wlu.bufdata;

static WINERR select_adapter(HANDLE, int adapter);
static void display_err(PCHAR prefix, WINERR status);
static char io_buf[WLC_IOCTL_MAXLEN];

/* This handle needs to be passed to all "ir_" fuinctions in the
 * shared DLL.  It is global because most of the APIs here were written
 * when the handle was maintained as a global in the shared dll.
 */

static DWORD ndevs;
static ADAPTER devlist[10];
static int ap_mode = 0;
static bool debug = FALSE;
static int ReadTotalTimeout = 0;

int remote_type = NO_REMOTE;

/* RemoteWL declarations */
static int rwl_os_type = WIN32_OS;
static bool rwl_dut_autodetect = TRUE;
extern char *g_rwl_buf_mac;
extern char  *g_rwl_device_name_serial;
extern int g_rwl_device_baud;
unsigned short g_rwl_servport;
char *g_rwl_servIP = NULL;
HANDLE hWaitThread;
HANDLE g_killThreadEvent;
#define RWL_WIFI_JOIN_DELAY 5000
extern int rwl_check_port_number(char *s, int len);

unsigned short defined_debug = DEBUG_ERR | DEBUG_INFO;

/* Flag bits for NDIS key structure */
#define WSEC_NDIS_TX_KEY		(1<<31)
#define WSEC_NDIS_PAIR_KEY		(1<<30)

/* interactive mode */
#define NDIS_NUM_ARGS	32
#define NDIS_MAXLEN_ARGS	512

static int ir_queryinformation_fe(void *wl, int cmd, void* input_buf, DWORD *input_len);
int ir_setinformation_fe(void *wl, int cmd, void* buf, DWORD *len);
static cmd_t *wl_find_cmd(char* name, int ap_mode, int remote_os_type);
static const char *imode2str(NDIS_802_11_NETWORK_INFRASTRUCTURE modeval);
static BOOL aton(const char *s, unsigned char *d, int n);
static const char *ndis_network_type_name(NDIS_802_11_NETWORK_TYPE type);
static WINERR process_args(HANDLE irh, char **argv);
static int buf_to_args(char *line, char *new_args[]);
static void do_interactive();
extern void rwl_wifi_swap_remote_type(int flag);
static cmd_func_t epi_list, epi_xlist;
static cmd_func_t ndis_shownetworks;
static cmd_func_t ndis_nettypes;
static cmd_func_t ndis_nettype;
static cmd_func_t ndis_showstats;
static cmd_func_t ndis_showconfig;
static cmd_func_t ndis_join;
static cmd_func_t ndis_ssid;
static cmd_func_t ndis_scan;
static cmd_func_t ndis_listen;
static cmd_func_t ndis_mac;
static cmd_func_t ndis_addkey;
static cmd_func_t ndis_addwep;
static cmd_func_t ndis_wepstatus;
static cmd_func_t ndis_wepdefaults;
static cmd_func_t ndis_infra;
static cmd_func_t ndis_disassoc;
static cmd_func_t ndis_generic_ulong;
static cmd_func_t sendpkt;


static cmd_t ndis_cmds[] = {
	{ "join", ndis_join, -1, -1,
	  "Join a specified network SSID.\n"
	  "\tJoin syntax is: join <name|ssid> [key <0-3>:xxxxx] [imode bss|ibss|auto] "
	  "[amode open|shared|openshared|wpa|wpapsk|wpanone|wpa2|wpa2psk]"
	},
	{ "ssid", ndis_ssid, OID_802_11_SSID, OID_802_11_SSID,
	  "Set or get the current SSID.\n"
	  "\tSetting will initiate an assoication attempt if in infrastructure mode,\n"
	  "\tor join/creation of an IBSS if in IBSS mode,\n"
	  "\tor creation of a BSS if in AP mode."},
	{ "list", epi_list, -1, -1,
	  "list all installed Wireless adapters" },
	{ "xlist", epi_xlist, -1, -1,
	  "list ALL installed network adapters" },
	{ "send", sendpkt, -1, -1,
	  "send SNAP formatted packet\n"
	  "\tsyntax: send [dest <f1:f2:f3:f4:f5:f6>] [oui <01:02:03>] "
	  "[type <888e>] [len <60>] [data <ca fe ba be ...>]\n"
	  "\texample: send dest 00:11:22:01:00:08 len 32 data be ef\n"
	  "\t\tsent to 00:11:22:01:00:08 oui 01:02:03 type 888e len 32\n"
	  "\t\tbe ef 01 02 03 04 05 06 07 08" },
	{ "force", wl_int, WLC_GET_FORCELINK, WLC_SET_FORCELINK,
	  "set the state of 'force link up':\n"
	  "\t0 - disable\n"
	  "\t1 - enable" },
	{ "legacylink", wl_int, WLC_LEGACY_LINK_BEHAVIOR, WLC_LEGACY_LINK_BEHAVIOR,
	  "set the 'IBSS legacy link behavior':\n"
	  "\t0 - disable\n"
	  "\t1 - enable" },
	{ "listen", ndis_listen, -1, -1,
	  "Set or query listen time in units of beacon interval." },
	{ "rndismac", ndis_mac, OID_802_3_CURRENT_ADDRESS, OID_802_3_CURRENT_ADDRESS,
	  "Get/Set current MAC address (set allowed with RNDIS-USB only)\n"
	  "\t(mac address, e.g. 00:11:20:11:33:33, colons optional)" },
	{ "encryptstrength", wl_int, WLC_ENCRYPT_STRENGTH, -1,
	  "get the current wep key length"  },
	{ "decryptstatus", wl_int, WLC_DECRYPT_STATUS, -1,
	  "the status of WEP decryption: one of {success=1, failure=2, unknown=3}" },
	{ "disassoc", ndis_disassoc, -1, OID_802_11_DISASSOCIATE,
	  "Disassociate from the current BSS/IBSS." },
	{ "addkey", ndis_addkey, -1, OID_802_11_ADD_KEY,
	  "The OID_802_11_ADD_KEY OID requests the miniport driver to set an\n"
	  "\t802.11 wired equivalent privacy (WEP) key to a specified value.\n"
	  "\taddkey <keyindex> <keydata> [notx] [xx:xx:xx:xx:xx:xx]" },
	{ "addwep", ndis_addwep, -1, OID_802_11_ADD_WEP,
	  "The OID_802_11_ADD_WEP OID requests the miniport driver to set an\n"
	  "\t802.11 wired equivalent privacy (WEP) key to a specified value.\n"
	  "\taddwep <keyindex> <keydata>" },
	{ "wepstatus", ndis_wepstatus, OID_802_11_WEP_STATUS, OID_802_11_WEP_STATUS,
	  "When set, the OID_802_11_WEP_STATUS OID requests the miniport driver\n"
	  "\tto set its NIC's 802.11 WEP status to enabled or disabled.\n"
	  "\tWhen queried, this OID requests the miniport driver to return its\n"
	  "\tNIC's 802.11 WEP status indication.\n"
	  "\twepstatus [on|off]" },
	{ "wepdefault", ndis_wepdefaults, -1, OID_802_11_RELOAD_DEFAULTS,
	  "Reset the WEP keys to their power-on defaults" },
	{ "shownetworks", ndis_shownetworks, OID_802_11_BSSID_LIST, -1,
	  "Pretty-print the BSSID list" },
	{ "ndisscan", ndis_scan, -1, OID_802_11_BSSID_LIST_SCAN,
	  "Initiate a broadcast SSID scan across all channels, no SSID argument." },
	{ "infra", ndis_infra, OID_802_11_INFRASTRUCTURE_MODE, OID_802_11_INFRASTRUCTURE_MODE,
	  "Set the Infrastructure Mode.\n"
	  "\t0 for IBSS\n"
	  "\t1 for Infrastructure BSS\n"
	  "\t2 for either"},
	{ "ndis_frag", ndis_generic_ulong, OID_802_11_FRAGMENTATION_THRESHOLD,
	  OID_802_11_FRAGMENTATION_THRESHOLD,
	  "Set the Fragmentation Threshold via the\n"
	  "\tNDIS OID OID_802_11_FRAGMENTATION_THRESHOLD. Range [256, 2346]"},
	{ "ndis_rts", ndis_generic_ulong, OID_802_11_RTS_THRESHOLD, OID_802_11_RTS_THRESHOLD,
	  "Set the RTS Threshhold via the\n"
	  "\tNDIS OID OID_802_11_RTS_THRESHOLD. Range [0, 2347]"},
	{ "stats", ndis_showstats, OID_802_11_STATISTICS, -1,
	  "Use OID_802_11_STATISTICS to dump statistics for the 802.11 interface" },
	{ "rcvok", ndis_generic_ulong, OID_GEN_RCV_OK, -1,
	  "Use OID_GEN_RCV_OK to get 'Frames received without errors'" },
	{ "xmitok", ndis_generic_ulong, OID_GEN_XMIT_OK, -1,
	  "Use OID_GEN_XMIT_OK to get 'Frames transmitted without errors'" },
	{ "rcvdropped", ndis_generic_ulong, OID_GEN_RCV_NO_BUFFER, -1,
	  "Use RNDIS_OID_GEN_RCV_NO_BUFFER to get 'Frames missed, no buffer'" },
	{ "config", ndis_showconfig, OID_802_11_CONFIGURATION, OID_802_11_CONFIGURATION,
	  "Get and Set the default configuration via OID_802_11_CONFIGURATION.\n"
	  "\tSyntax: config [beacon beacon_value atim atim_value freq freq_in_Khz]\n"
	  "\tfrequency must be in Khz.  Invalid frequencies will be rejected.\n"
	  "\tExample to set parameters: wl config beacon 100 atim 0 freq 2412000\n"
	  "\tExample to get parameters: wl config"
	},
	{ "nettypes_supported", ndis_nettypes, OID_802_11_NETWORK_TYPES_SUPPORTED, -1,
	  "Get supported network types via OID_802_11_NETWORK_TYPES_SUPPORTED."
	},
	{ "nettype", ndis_nettype, OID_802_11_NETWORK_TYPE_IN_USE, OID_802_11_NETWORK_TYPE_IN_USE,
	  "Get/Set network type in use via OID_802_11_NETWORK_TYPE_IN_USE.\n"
	  "\tSyntax: nettype [FH DS OFDM24 OFDM5]\n"
	  "\t  FH     Frequency Hopping\n"
	  "\t  DS     Direct-Sequencing, Spread-Spectrum (11b only mode)\n"
	  "\t  OFDM24 OFDM in 2.4 GHz (11g Auto mode)\n"
	  "\t  OFDM5  OFDM in 5 GHz (11a mode)"
	},
	{ "fips", wl_varint, WLC_GET_VAR, WLC_SET_VAR,
	  "get/set STA FIPS mode(STA only).  Set only before association\n"
	  "\t 0  - disable\n"
	  "\t 1  - enable\n"
	  "\t-1  - prohibited"
	},
	{ NULL, NULL, 0, 0, NULL }
};
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

wl_get(void *wl, int cmd, void *buf, int len)
{
	DWORD dwlen = len;
	int error;

	if (rwl_os_type == WIN32_OS || rwl_os_type == WINVISTA_OS || remote_type == NO_REMOTE)
		cmd += WL_OID_BASE;
	error = (int)ir_queryinformation_fe((HANDLE)wl, cmd, buf, &dwlen);

	if (error == BCME_NODEVICE)
		return BCME_NODEVICE;
	else if (error != 0)
		return BCME_IOCTL_ERROR;

	return 0;
}

int
wl_set(void *wl, int cmd, void *buf, int len)
{
	DWORD dwlen = len;
	int error;

	if (rwl_os_type == WIN32_OS || rwl_os_type == WINVISTA_OS || remote_type == NO_REMOTE)
		cmd += WL_OID_BASE;
	error = (int)ir_setinformation_fe((HANDLE)wl, cmd, buf, &dwlen);

	if (error == BCME_NODEVICE)
		return BCME_NODEVICE;
	else if (error != 0)
		return BCME_IOCTL_ERROR;

	return 0;
}

int wl_os_type_get_rwl()
{
	return rwl_os_type;
}

void wl_os_type_set_rwl(int os_type)
{
	rwl_os_type = os_type;
}

int
wl_ir_init_rwl(HANDLE *irh)
{
	int err;

	/* initialize irelay and select default adapter */
	*irh = INVALID_HANDLE_VALUE;
	if ((err = ir_init(irh)) == ERROR_SUCCESS) {
		ndevs = ARRAYSIZE(devlist);
		if ((err = ir_adapter_list(*irh, &devlist[0], &ndevs)) == ERROR_SUCCESS) {
			err = select_adapter(*irh, -1);
		}
	}

	return err;
}

int
wl_ir_init_adapter_rwl(HANDLE *irh, int adapter)
{
	int err;

	/* initialize irelay and select default adapter */
	*irh = INVALID_HANDLE_VALUE;
	if ((err = ir_init(irh)) == ERROR_SUCCESS) {
		ndevs = ARRAYSIZE(devlist);
		if ((err = ir_adapter_list(*irh, &devlist[0], &ndevs)) == ERROR_SUCCESS) {
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
		if (irh != INVALID_HANDLE_VALUE) {
			ir_unbind(irh);
			ir_exit(irh);
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
	char **arguement = NULL, k = 0;
	char port_no[3];
	char* endptr;
#endif /* BCMDLL */
#if defined(WINPE)
	int exit_code;

	signal(SIGINT, pet_ctrlc_handler);

	ir_usewmiset(FALSE);

	err = wl_ir_init_rwl(&irh);

	if (err == ERROR_SUCCESS)
		exit_code = pet_start(irh, argc, argv);
	else
		exit_code = RET_NO_WIRELESS_CARD_FOUND;
#else
	wlu_av0 = argv[0];
	argv++; 	/* skip past name argument */

	/* reset wlu flags */
	/* changing location from process arg */
	/* at previous location, batching mode operation was failing in windows wl */
	wlu_init();

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
		if (*argv && strncmp (*argv, "--baud", strlen(*argv)) == 0) {
			*argv++;
			if (!(*argv)) {
				rwl_usage(remote_type);
				return err;
			}
			if ((*argv) && (rwl_check_port_number(*argv, strlen(*argv)) == SUCCESS)) {
				g_rwl_device_baud = atoi(*argv);
				*argv++;
			}
		}
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
		rwl_init_ws2_32dll(); 	/* Initialise the Winsock DLL */
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
		err = wl_ir_init_rwl(&irh);
	}
	else {
		if (debug) {
			printf("remote_type == REMOTE_SERIAL, no need to check local device\n");
		}
	}

	if (err == ERROR_SUCCESS){
		/* No arguments?  Print help hint and bail. */
		if (!(*argv)){
			fprintf(stderr, "type -h for help\n");
			err = ERROR_BAD_COMMAND;
			wl_close_rwl(remote_type, irh);
			return err;
		}

		/* Autodetect remote DUT */
		if (remote_type != NO_REMOTE && rwl_dut_autodetect == TRUE)
			rwl_detect((void*)irh, debug, &rwl_os_type);

		/* Interactive mode? Accept either 'wl interactive' or 'wl --i*' */
		if (((strlen(*argv) > 2) && strncmp (*argv, "--interactive", strlen(*argv)) == 0) ||
		    (strncmp (*argv, "interactive", strlen("interactive")) == 0)){
			do_interactive(irh, ++argv);
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
#endif /* WINPE */
	wl_close_rwl(remote_type, irh);
#if defined(WINPE)
	exit(exit_code);
#endif
	return err;
}

static WINERR
process_args(HANDLE irh, char **argv)
{
	char *ifname = NULL;
	int help = 0;
	int init_err = FALSE;
	int status = 0;
	WINERR err = ERROR_SUCCESS;
	cmd_t *cmd = NULL;

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

#ifdef RWLASD
		if (remote_type != NO_REMOTE && strncmp (*argv, "asd",
			strlen(*argv)) == 0) {
			*argv++;
			if (*argv) {
				err = rwl_shell_cmd_proc((void *)irh, argv, ASD_CMD);
			} else {
				DPRINT_ERR(ERR, "Enter the asd command (e.g ca_get_version \n");
				err = -1;
			}
			return err;
		}
#endif
		if (rwl_os_type == WINVISTA_OS) {
			for (vista_cmd_index = 0; remote_vista_cmds[vista_cmd_index] &&
				strcmp(remote_vista_cmds[vista_cmd_index],*argv); vista_cmd_index++);
				if (remote_vista_cmds[vista_cmd_index] != NULL) {
					err = rwl_shell_cmd_proc((void *)irh, argv, VISTA_CMD);
				if ((remote_type == REMOTE_WIFI) && ((!strcmp(*argv, "join")))) {
#ifdef RWL_WIFI
					DPRINT_INFO(OUTPUT, "\n Findserver is called to	synchronize the channel\n\n");
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

		/* Check for help or specific interface (ifname) */
		if ((status = wl_option(&argv, &ifname, &help)) == CMD_OPT) {
			/* print usage */
			if (help)
				break;
			/* select different adapter */
			if (ifname) {
				if (remote_type == NO_REMOTE) {
					ir_unbind(irh);
					if ((err = select_adapter(irh, atoi(ifname))) != ERROR_SUCCESS)
						break;
					if (wl_check((void *)irh))
						break;
				}
				else {
                                        strncpy(g_rem_ifname, ifname, IFNAMSIZ);
                                }
			}
			continue;
		}

		/* Could not open new interface */
		else if (status == CMD_ERR)
			break;

		/* make sure the adapter is supported */
		if (wl_check((void *)irh)) {
			fprintf(stderr, "Adapter unsupported\n");
			break;
		}

		if (remote_type == NO_REMOTE) {
			/* Referesh ap_mode prior to each command in case user
			   switched modes via 'wl ap' */
			wl_get((void *)irh, WLC_GET_AP, &ap_mode, sizeof(int));
			if (debug) {
				printf("process_args(), ap_mode=%d\n", ap_mode);
			}
		}
		cmd = wl_find_cmd(*argv, ap_mode, rwl_os_type);

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

	 /* provide for help on a particular command */
	if (help) {
		if  (*argv) {
			cmd = wl_find_cmd(*argv, ap_mode, rwl_os_type);
			if (cmd)
				wl_cmd_usage(stdout, cmd);
			else {
				fprintf(stderr,
					"%s: Unrecognized command \"%s\", type -h for help\n",
					wlu_av0, *argv);
			}
		} else {
			/* outputs multiple screens of help information */
			wl_usage(stdout, ap_mode ? wl_cmds : ndis_cmds);
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
		else
			display_err("main:", err);
	}
	return err;
}

static void
do_interactive(HANDLE irh, char **argv)
{
	char *local_argv[NDIS_NUM_ARGS];
	int i;
	char input_buf[NDIS_MAXLEN_ARGS];
	char *ifname = NULL;
	int help = 0;

	/* this is just to let the wlu.c check if there is a --clientbatch option */
	wl_option(&argv, &ifname, &help);

	while (1) {
		int my_argc;

		/* Get a line of input, ignoring 0 length lines */
		do {
			input_buf[0] = 0;
			printf("> ");
			fflush(0);

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
#ifdef RWL_WIFI
		if (!strcmp(local_argv[0], "findserver")) {
			remote_wifi_ser_init_cmds((void *) irh);
		}
#endif

		/* Exit? */
		if (!strcmp (local_argv[0], "q") || !strcmp (local_argv[0], "exit"))
			break;

		/* Check for 'shell' cmd */
		if (remote_type != NO_REMOTE && strncmp (local_argv[0], "sh",
			strlen(local_argv[0])) == 0)
				process_args(irh, local_argv);
			/* Execute command.  Strip off possible leading 'wl'
			 * since users are used to typing it
			*/
		else if (!strcmp (local_argv[0], "wl")) {
			process_args(irh, &local_argv[1]);
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
				process_args(irh, local_argv);
			}
		}
next:
		/* free up malloced resources */
		for (i = 0; i < my_argc; i++) {
			if (local_argv[i]) {
				free(local_argv[i]);
				local_argv[i] = NULL;
			} else
				break;
		}
	}
}

static int
buf_to_args(char *tmp, char *new_args[])
{
	char line[NDIS_MAXLEN_ARGS];
	char *token;
	int argc = 0;

	if (strlen(tmp) >= NDIS_MAXLEN_ARGS) {
		printf("wl:error: Input string too long; must be < %d bytes\n", NDIS_MAXLEN_ARGS);
		return 0;
	}

	strcpy(line, tmp);
	while ((argc < (NDIS_NUM_ARGS - 1)) &&
	       ((token = strtok(argc ? NULL : line, " \t")) != NULL)) {

		/* Specifically make sure empty arguments (like SSID) are empty */
		if (token[0] == '"' && token[1] == '"') {
			token[0] = '\0';
		}

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

/* Search the ndis_cmds and wl_cmds tables for a matching command name.
 * Return the matching command or NULL if no match found.
 * If 'ap_mode' is true, prefer a common 'wl_cmds' entry over the ndis specific
 * commands.
 * Added remote_os_type for cross OS support
 * To talk with Linux server wl cmds needs to be send to Linux server. Ndis cmds
 * are not supported in Linux. Also, sta mode in Linux is not supported
 * so when remote_os_type == LINUX_OS, prefer a base wl_cmd
 */
static cmd_t*
wl_find_cmd(char* name, int ap_mode, int remote_os_type)
{
	cmd_t *cmd = NULL;
	cmd_t *ndis_cmd = NULL;
	cmd_t *base_cmd = NULL;

	/* search the ndis_cmds */
	for (cmd = ndis_cmds; cmd->name && strcmp(cmd->name, name); cmd++);
	if (cmd->name != NULL)
		ndis_cmd = cmd;

	/* search the wl_cmds */
	cmd = wlu_find_cmd(name);
	if (cmd != NULL)
		base_cmd = cmd;

	if (remote_type != NO_REMOTE) {
		/* prefer a wl cmd always for remote server */
		cmd = (base_cmd)?base_cmd:ndis_cmd;
	} else {
		/* if in ap mode, prefer a base wl_cmd over an ndis cmd,
		 * if in sta mode, prefer the ndis cmds
		 */
		if (ap_mode)
			cmd = (base_cmd)?base_cmd:ndis_cmd;
		else
			cmd = (ndis_cmd)?ndis_cmd:base_cmd;
	}

	return cmd;
}

/* Retrieve a list of all unique instance identifiers and mac
 * addresses for BRCM Wireless adapters.
 */
int
epi_list(void *wl, cmd_t *cmd, char **argv)
{
	ULONG n;
	PADAPTER pdev;
	struct ether_addr *id;

	for (n = 0; n < ndevs; n++) {
	    pdev = &devlist[n];
	    if (pdev->type == IR_WIRELESS) {
		id = (struct ether_addr *) pdev->macaddr;
		printf("%4d: wl%d MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
		       n, pdev->instance,
		       id->octet[0], id->octet[1], id->octet[2],
		       id->octet[3], id->octet[4], id->octet[5]);
	    }
	}

	return ERROR_SUCCESS;
}

PCHAR type2str(IRTYPE type)
{
	if (type < 0 || type >= IR_MAXTYPE) {
		return "<unkown type>";
	} else {
		return ir_typestrings[type];
	}
}

PCHAR type2prefix(IRTYPE type)
{
	if (type < 0 || type >= IR_MAXTYPE) {
		return "<unkown type>";
	} else {
		return ir_prefixstrings[type];
	}
}

/* Retrieve a list of all unique instance identifiers and mac
 * addresses for ALL network adapters (not just Wireless) discovered by
 * the irelay pseudo-device.
 */
int
epi_xlist(void *wl, cmd_t *cmd, char **argv)
{
	ULONG n;
	const char *typestr;
	const char *prefix;
	PADAPTER pdev;
	struct ether_addr *id;

	for (n = 0; n < ndevs; n++) {
		pdev = &devlist[n];
		id = (struct ether_addr *) pdev->macaddr;
		if (pdev->type >= 0 && pdev->type < IR_MAXTYPE) {
			typestr = type2str(pdev->type);
			prefix = type2prefix(pdev->type);
			printf("%4d:\t%s%-2d %-8s %4s  %s ",
			       n, prefix, pdev->instance, typestr, pdev->shortname, pdev->name);
		} else {
			printf("%4d:\t???? %-8s %4s  %s ",
			       n, typestr, pdev->shortname, pdev->name);
		}
		printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
		       id->octet[0], id->octet[1], id->octet[2],
		       id->octet[3], id->octet[4], id->octet[5]);
	}

	return ERROR_SUCCESS;
}

static int
ndis_mac(void *wl, cmd_t *cmd, char **argv)
{
	int status, length;
	struct ether_addr ea;

	if (!*++argv) {
		length = sizeof(ea);
		status = ir_queryinformation_fe(wl, cmd->get, (PUCHAR) &ea, &length);
		if (status == NO_ERROR)
			printf("%s is %s\n", cmd->name, wl_ether_etoa(&ea));
	} else {
		if (!wl_ether_atoe(*argv, &ea))
			return -1;
		length = sizeof(ea);
		status = ir_setinformation_fe(wl, cmd->set, (PUCHAR)&ea, &length);
	}

	return status;
}

static int
ndis_listen(void *wl, cmd_t *cmd, char **argv)
{
	int status, length;
	char buf[4096];
	NDIS_802_11_ASSOCIATION_INFORMATION *ai = (NDIS_802_11_ASSOCIATION_INFORMATION *)buf;

	if (*++argv) {
		memset(ai, 0, sizeof(*ai));
		ai->AvailableRequestFixedIEs |= NDIS_802_11_AI_REQFI_LISTENINTERVAL;
		ai->RequestFixedIEs.ListenInterval = (USHORT)strtoul(*argv, 0, 0);
		length = sizeof(*ai);
		status = ir_setinformation_fe(wl, OID_802_11_ASSOCIATION_INFORMATION,
		                           (PUCHAR)ai, &length);
		CHECK_STATUS("OID_802_11_ASSOCIATION_INFORMATION", status);
	} else {
		length = sizeof(buf);
		status = ir_queryinformation_fe(wl, OID_802_11_ASSOCIATION_INFORMATION,
		                             (PUCHAR)buf, &length);
		CHECK_STATUS("OID_802_11_ASSOCIATION_INFORMATION", status);
		if (status != NO_ERROR ||
		    !ai->RequestIELength ||
		    !(ai->AvailableRequestFixedIEs & NDIS_802_11_AI_REQFI_LISTENINTERVAL))
			printf("Listen Interval: unavailable\n");
		else
			printf("Listen Interval: %d\n", ai->RequestFixedIEs.ListenInterval);
	}

	return status;
}

static int
ndis_ssid(void *wl, cmd_t *cmd, char **argv)
{
	ULONG length;
	char ssidbuf[SSID_FMT_BUF_LEN];
	NDIS_802_11_SSID ssid;
	int status;

	if (!*++argv) {
		length = sizeof(ssid);
		status = ir_queryinformation_fe(wl, OID_802_11_SSID, (PUCHAR) &ssid, &length);
		CHECK_STATUS("OID_802_11_SSID", status);
		if (status != NO_ERROR)
			return status;

		wl_format_ssid(ssidbuf, ssid.Ssid, ssid.SsidLength);
		printf("Current SSID: \"%s\"\n", ssidbuf);
	} else {
		if (strlen(*argv) > 32) {
			fprintf(stderr, "SSID arg \"%s\" must be 32 chars or less\n", *argv);
			return -1;
		}

		strncpy(ssid.Ssid, *argv, sizeof(ssid.Ssid));
		ssid.SsidLength = strlen(*argv);

		wl_format_ssid(ssidbuf, ssid.Ssid, ssid.SsidLength);
		printf("Setting SSID \"%s\"\n", ssidbuf);
		length = sizeof(ssid);
		status = ir_setinformation_fe(wl, OID_802_11_SSID, (PUCHAR) &ssid, &length);
		CHECK_STATUS("OID_802_11_SSID", status);
	}
	return status;
}

static PNDIS_802_11_BSSID_LIST_EX wl_bssid_list(void *wl)
{
	PNDIS_802_11_BSSID_LIST_EX pBssidList = NULL;
	ULONG status;
	long length;
	int retry = 3;

	/* we first query the oid with a buffer whose length is intentionally too short.
	 * that way the driver will tell us the length of buffer that we need.
	 * We may have to do this a few times if BSSID list is in a state of flux.
	 */
	length = 10000;
	do {
		pBssidList = (PNDIS_802_11_BSSID_LIST_EX) malloc(length);
		memset(pBssidList, 0, length);
		status = ir_queryinformation_fe(wl, OID_802_11_BSSID_LIST, (PUCHAR) pBssidList,
		                             &length);
		if (status == ERROR_MORE_DATA) {
			free(pBssidList);
			pBssidList = NULL;
			Sleep(100);
		}
	} while (status == ERROR_MORE_DATA && --retry >= 0);

	CHECK_STATUS("OID_802_11_BSSID_LIST", status);

	return pBssidList;
}

static PNDIS_WLAN_BSSID_EX wl_bssid(void *wl, char *ssid, PNDIS_WLAN_BSSID_EX bssid)
{
	PNDIS_802_11_BSSID_LIST_EX pBssidList = NULL;
	PNDIS_WLAN_BSSID_EX pBssid;
	int i;

	if ((pBssidList = wl_bssid_list(wl)) != NULL) {
		for (i = 0; i < (int) pBssidList->NumberOfItems; i++) {
			pBssid = &(pBssidList->Bssid[i]);
			if (strlen(ssid) == pBssid->Ssid.SsidLength &&
			    strncmp(pBssid->Ssid.Ssid, ssid, pBssid->Ssid.SsidLength) == 0) {
				*bssid = *pBssid;
				return bssid;
			}
		}
	}
	return NULL;
}

/* Is this body of this tlvs entry a WPA entry? If */
/* not update the tlvs buffer pointer/length */
static bool
is_wpa_ie(uint8 **wpaie, uint8 **tlvs, int *tlvs_len)
{

	uint8 *ie = *wpaie;

	/* If the contents match the WPA_OUI and type=1 */
	if ((ie[1] >= 6) && !memcmp(&ie[2], WPA_OUI "\x01", 4)) {
		return TRUE;
	}

	/* point to the next ie */
	ie += ie[1] + 2;
	/* calculate the length of the rest of the buffer */
	*tlvs_len -= (int)ie - (int)*tlvs;
	/* update the pointer to the start of the buffer */
	*tlvs = ie;

	return FALSE;
}

/* WPA IE decoding taken from "Wi-Fi Protected Access (WPA)",
 * "Appendix D: WPA information element reference implementation", V1.3
 * March 1, 2003.
 */

#define NONE		-1	/* None */
#define WEP40		0	/* Wep-40bit */
#define TKIP		1	/* TKIP */
#define AESCCMP		2	/* AES-CCMP */
#define AESWRAP		3	/* AES-WRAP */
#define WEP104		4	/* WEP-104 */
#define IEEE802_1X	0	/* 802.1x */
#define ELEMENTID	0xdd	/* Elememnt ID */
#define GROUPFLAG	0x02	/* Flag for group */
#define REPLAYBITSSHIFT 2	/* # of bits for Replay bit */
#define REPLAYBITS	0x03	/* Mask for Replay bits */

struct _IE {
	uchar Elementid;
	uchar length;
	uchar oui[4];
	ushort version;
	uchar multicast[4];
	ushort ucount;
	struct {
		uchar oui[4];
	} unicast[1]; /* the rest is variable so need to overlay ieauth structure */
};

struct _ieauth {
	ushort acount;
	struct {
		uchar oui[4];
	} auth[1];
};

int multicast;
int unicast[4];
ushort ucount;
int auth[4];
ushort acount;
int unicastasgroup;
int replayindex;

void wpa_test(struct _IE *IE, int length)
{
	uchar oui00[4] = { 0x00, 0x50, 0xf2, 0x00 };
	uchar oui01[4] = { 0x00, 0x50, 0xf2, 0x01 };
	uchar oui02[4] = { 0x00, 0x50, 0xf2, 0x02 };
	uchar oui03[4] = { 0x00, 0x50, 0xf2, 0x03 };
	uchar oui04[4] = { 0x00, 0x50, 0xf2, 0x04 };
	uchar oui05[4] = { 0x00, 0x50, 0xf2, 0x05 };
	int i = 0, j, m, n;
	struct _ieauth *ieauth;
	char *caps;

	multicast = TKIP;
	unicast[0] = TKIP;
	ucount = 1;
	auth[0] = IEEE802_1X;
	acount = 1;
	unicastasgroup = 0;
	replayindex = 2;

	/* information element header makes sense */
	if ((IE->length+2 == length) && (IE->length >= 6) &&
	    (IE->Elementid == ELEMENTID) &&
	    !memcmp(IE->oui, oui01, 4) && (IE->version == 1)) {
		/* update each variable if IE is long enough to contain the  variable */
		if (IE->length >= 10) {
			if (!memcmp(IE->multicast, oui01, 4))
				multicast = WEP40;
			else if (!memcmp(IE->multicast, oui02, 4))
				multicast = TKIP;
			else if (!memcmp(IE->multicast, oui03, 4))
				multicast = AESCCMP;
			else if (!memcmp(IE->multicast, oui04, 4))
				multicast = AESWRAP;
			else if (!memcmp(IE->multicast, oui05, 4))
				multicast = WEP104;
			else
				/* any vendor checks here */
				multicast = -1;
		}
		if (IE->length >= 12) {
			j = 0;
			for (i = 0; (i < IE->ucount) &&
			             (j < sizeof(unicast)/sizeof(int)); i++) {
				if (IE->length >= 12+i*4+4) {
					if (!memcmp(IE->unicast[i].oui, oui00, 4))
						unicast[j++] = NONE;
					else if (!memcmp(IE->unicast[i].oui, oui02, 4))
						unicast[j++] = TKIP;
					else if (!memcmp(IE->unicast[i].oui, oui03, 4))
						unicast[j++] = AESCCMP;
					else if (!memcmp(IE->unicast[i].oui, oui04, 4))
						unicast[j++] = AESWRAP;
					else
						/* any vendor checks here */
						;
				}
				else
					break;
			}
			ucount = j;
		}
		m = i;
		if (IE->length >= 14+m*4) {
			/* overlay ieauth structure into correct place */
			ieauth = (struct _ieauth *)IE->unicast[m].oui;
			j = 0;
			for (i = 0; (i < ieauth->acount) &&
			             (j < sizeof(auth)/sizeof(int)); i++) {
				if (IE->length >= 14+4+(m+i)*4) {
					if (!memcmp(ieauth->auth[i].oui, oui00, 4))
						auth[j++] = IEEE802_1X;
					else
						/* any vendor checks here */
						;
				}
				else
					break;
			}
			if (j > 0)
				acount = j;
		}
		n = i;
		if (IE->length+2 >= 14+4+(m+n)*4) {
			caps = (char *)ieauth->auth[n].oui;
			unicastasgroup = (*caps)&GROUPFLAG;
			replayindex = 2<<((*caps>>REPLAYBITSSHIFT)&REPLAYBITS);
		}
	}
}


char *cip[] = { "", "WEP40", "TKIP", "AES-CCMP", "AES-WRAP", "WEP104" };
char *cip1[] = { "NONE", "WEP40", "TKIP", "AES-CCMP", "AES-WRAP", "WEP104" };
char *aip[] = { "", "802.1X" };

/* Pretty print the BSS list */
static void
ndis_dump_networks(PNDIS_802_11_BSSID_LIST_EX pBssidList)
{
	PNDIS_WLAN_BSSID_EX pBssid;
	struct ether_addr *mac;
	float freq;
	int i, j;
	char ssid_buffer[SSID_FMT_BUF_LEN];
	const char *network_type_name;

	pBssid = pBssidList->Bssid;
	for (i = 0; i < (int) pBssidList->NumberOfItems; i++) {
		wl_format_ssid(ssid_buffer, pBssid->Ssid.Ssid, pBssid->Ssid.SsidLength);

		network_type_name = ndis_network_type_name(pBssid->NetworkTypeInUse);
		if (network_type_name)
			printf("Type: %s\t", network_type_name);
		else
			printf("Type: unknown net type 0x%x\t", (uint)pBssid->NetworkTypeInUse);
		printf("SSID: \"%s\"\t", ssid_buffer);
		printf("Mode: %s", imode2str(pBssid->InfrastructureMode));
		printf("\n");

		printf("RSSI: %d dBm\t", pBssid->Rssi);
		/* convert frequency from KHz to GHz */
		freq = (float) pBssid->Configuration.DSConfig;
		freq /= 1000000;
		printf("Frequency: %5.3fGHz\t", freq);
		/* convert KHz to Mhz then to channel number */
		printf("Channel: %d", freq2channel(pBssid->Configuration.DSConfig/1000));
		printf("\n");

		mac = (struct ether_addr *) &(pBssid->MacAddress);
		printf("BSSID: %02X:%02X:%02X:%02X:%02X:%02X\t",
		       mac->octet[0], mac->octet[1], mac->octet[2],
		       mac->octet[3], mac->octet[4], mac->octet[5]);
		printf("\n");
		printf("Supported Rates: ");
		dump_rateset(pBssid->SupportedRates, 16);
		printf("\n");

		if (pBssid->IELength) {
			struct dot11_bcn_prb *prb;
			uint8 *parse;
			uint parse_len;
			bcm_tlv_t *cp;
			prb = (struct dot11_bcn_prb *) pBssid->IEs;
			cp = (bcm_tlv_t *)prb;

			/* WPA parameters */
			parse = (uint8 *)prb + sizeof(NDIS_802_11_FIXED_IEs);
			parse_len = pBssid->IELength-sizeof(NDIS_802_11_FIXED_IEs);
			while ((cp = bcm_parse_tlvs(parse, parse_len, DOT11_MNG_WPA_ID))) {
				if (is_wpa_ie(&(uint8*)cp, &parse, &parse_len)) {
					wpa_test((struct _IE *)cp, cp->len + 2);
					printf("Multicast %s ", cip1[(multicast+1)]);
					printf("Unicast");
					for (j = 0; j < ucount; j++) {
						printf(" %s", cip[unicast[j]+1]);
					}
					printf(" Auth");
					for (j = 0; j < acount; j++) {
						printf(" %s", aip[auth[j]+1]);
					}
					printf("\n");
					break;
				}
			}

		}
		printf("\n");

		pBssid = (PNDIS_WLAN_BSSID_EX)((uint) pBssid + pBssid->Length);
	}
}

static int
ndis_shownetworks(void *wl, cmd_t *cmd, char **argv)
{
	PNDIS_802_11_BSSID_LIST_EX pBssidList = NULL;

	if ((pBssidList = wl_bssid_list(wl)) != NULL) {
		ndis_dump_networks(pBssidList);
	}
	return 0;
}

static int
ndis_showconfig(void *wl, cmd_t *cmd, char **argv)
{
	ULONG status, length;
	NDIS_802_11_CONFIGURATION config;
	float freq;
	char* endptr;

	++argv;
	if (*argv) {
		config.BeaconPeriod = 0xbeef;
		config.DSConfig = 0xbeef;
		config.ATIMWindow = 0xbeef;
		while (*argv) {
			if (!stricmp(*argv, "beac")) {
				++argv;
				config.BeaconPeriod = strtoul(*argv, &endptr, 0);
				if (*endptr != '\0') {
					/* not all the value string was parsed by strtoul */
					return -1;
				}
			}
			if (!stricmp(*argv, "atim")) {
				++argv;
				config.ATIMWindow = strtoul(*argv, &endptr, 0);
				if (*endptr != '\0') {
					/* not all the value string was parsed by strtoul */
					return -1;
				}
			}
			if (!stricmp(*argv, "freq")) {
				++argv;
				config.DSConfig = strtoul(*argv, &endptr, 0);
				if (*endptr != '\0') {
					/* not all the value string was parsed by strtoul */
					return -1;
				}
			}
			++argv;
		}
		if (config.BeaconPeriod == 0xbeef || config.DSConfig == 0xbeef ||
		    config.ATIMWindow == 0xbeef) {
			printf("Bad parameters\n");
			status = NO_ERROR;
			return (-1);
		}

		length = sizeof(config);
		status = ir_setinformation_fe(wl, cmd->set, (PUCHAR)&config, &length);
		if (status != NO_ERROR) {
			printf("set CONFIG failed, status = 0x%x\n", status);
		}
	} else {
		length =  sizeof(config);
		status = ir_queryinformation_fe(wl, cmd->get, (PUCHAR) &config, &length);
		if (status == NO_ERROR) {
			printf("Beacon Period\t%d kusecs\t\tATIM Window\t%d kusecs\n",
			       config.BeaconPeriod, config.ATIMWindow);

			/* convert frequency from KHz to GHz */
			freq = (float) config.DSConfig;
			freq /= 1000000;
			printf("Channel Freq\t%d Khz (%5.3f GHz)\tHop pattern\t0x%x\n",
			       config.DSConfig, freq, config.FHConfig.HopPattern);

			printf("Hop set\t\t0x%x\t\t\tDwell time\t%d kusecs\n",
			       config.FHConfig.HopSet, config.FHConfig.DwellTime);

		} else {
			printf("Config ioctl FAILED error = %d\n", status);
		}
	}

	return 0;
}

static int
ndis_nettypes(void *wl, cmd_t *cmd, char **argv)
{
	int status, length, i;
	const char *name;
	PNDIS_802_11_NETWORK_TYPE_LIST type_list;

	type_list = (PNDIS_802_11_NETWORK_TYPE_LIST)io_buf;

	length = WLC_IOCTL_MAXLEN;
	status = ir_queryinformation_fe(wl, cmd->get, io_buf, &length);
	if (status != NO_ERROR)
		return status;

	printf("Network types: count %d\n", type_list->NumberOfItems);
	for (i = 0; i < (int)type_list->NumberOfItems; i++) {
		name = ndis_network_type_name(type_list->NetworkType[i]);
		if (name)
			printf("%d: %s\n", i, name);
		else
			printf("%d: unknown type %d\n", i, type_list->NetworkType[i]);
	}

	return status;
}

static int
ndis_nettype(void *wl, cmd_t *cmd, char **argv)
{
	DWORD status = -1, length;
	NDIS_802_11_NETWORK_TYPE val;
	const char *name;

	length = sizeof(NDIS_802_11_NETWORK_TYPE);

	if (*++argv) {
		if (!stricmp(*argv, "FH"))
			val = Ndis802_11FH;
		else if (!stricmp(*argv, "DS"))
			val = Ndis802_11DS;
		else if (!stricmp(*argv, "OFDM24"))
			val = Ndis802_11OFDM24;
		else if (!stricmp(*argv, "OFDM5"))
			val = Ndis802_11OFDM5;
		else {
			printf("%s: Network type \"%s\" was not FH, DS, OFDM24, or OFDM5.\n",
			       cmd->name, *argv);
			return -1;
		}
		status = ir_setinformation_fe(wl, cmd->set, (PBYTE)&val, &length);
	} else {
		status = ir_queryinformation_fe(wl, cmd->get, (PBYTE)&val, &length);
		if (status == NO_ERROR) {
			name = ndis_network_type_name(val);
			if (name)
				printf("%s\n", name);
			else
				printf("unknown network type 0x%x\n", (uint)val);
		}
	}
	return status;
}

/* Helper routine to convert the infrastructure mode to a string */
static const char *
ndis_network_type_name(NDIS_802_11_NETWORK_TYPE type)
{
	switch (type) {
	case Ndis802_11FH:
		return "Ndis802_11FH";
	case Ndis802_11DS:
		return "Ndis802_11DS";
	case Ndis802_11OFDM5:
		return "Ndis802_11OFDM5";
	case Ndis802_11OFDM24:
		return "Ndis802_11OFDM24";
	default:
		return NULL;
	}
}

static int
ndis_showstats(void *wl, cmd_t *cmd, char **argv)
{
	ULONG status, length;
	NDIS_802_11_STATISTICS stats;

	length = sizeof(stats);
	status = ir_queryinformation_fe(wl, cmd->get, (PUCHAR) &stats, &length);
	if (status == NO_ERROR) {
		printf("\nTx Counters:\n");
		printf("Frag Count\t\t%d\t\tMcast Frame Count\t%d\n",
			stats.TransmittedFragmentCount, stats.MulticastTransmittedFrameCount);
		printf("Fail Count\t\t%d\t\tRetryCount\t\t%d\n",
			stats.FailedCount, stats.RetryCount);
		printf("Multiple Retry Count\t%d\t\tRTS Success Count\t%d\n",
			stats.MultipleRetryCount, stats.RTSSuccessCount);
		printf("RTS Failure Count\t%d\t\tACK Failure Count\t%d\n",
			stats.RTSFailureCount, stats.ACKFailureCount);

		printf("\nRx Counters:\n");
		printf("Frame Dup Count\t\t%d\t\tFrag Count\t\t%d\n",
			stats.FrameDuplicateCount, stats.ReceivedFragmentCount);
		printf("Mcast Frame Count\t%d\t\tFCS Error Count\t\t%d\n",
			stats.MulticastReceivedFrameCount, stats.FCSErrorCount);
	} else {
		printf("Stats ioctl FAILED error = %d\n", status);
	}
	return 0;
}

static PNDIS_802_11_WEP
ndis_parse_wep(const char *keystr)
{
	char hex[] = "XX";
	PNDIS_802_11_WEP key = NULL;
	UCHAR keydata[32], *data = &keydata[0];
	int keylen = 0;
	int keystlen;

	switch (strlen(keystr)) {
	case 5:
	case 13:
	case 16:
	keylen = strlen(keystr);
	strcpy(data, keystr);
	break;
	case 12:
	case 28:
	case 34:
	case 66:
	/* strip leading 0x */
	if (!strnicmp(keystr, "0x", 2))
	    keystr += 2;
	else
	    return NULL;
	/* fall through */
	case 10:
	case 26:
	case 32:
	case 64:
	keylen = strlen(keystr) / 2;
	while (*keystr) {
	    strncpy(hex, keystr, 2);
	    *data++ = (char) strtoul(hex, NULL, 16);
	    keystr += 2;
	}
	break;
	default:
	return NULL;
	}

	if (keylen > 0) {
	keystlen = sizeof(NDIS_802_11_WEP)-1+keylen;

	key = (PNDIS_802_11_WEP) malloc(keystlen);
	memset(key, 0, keystlen);
	key->Length = keystlen;
	key->KeyIndex = 0;
	key->KeyLength = keylen;
	memcpy(&(key->KeyMaterial[0]), keydata, keylen);
	}

	return key;
}

/* Helper routine to convert the infrastructure mode to a string */
static const char *imode2str(NDIS_802_11_NETWORK_INFRASTRUCTURE modeval)
{
	switch (modeval) {
	case Ndis802_11IBSS:
	return "0";
	case Ndis802_11Infrastructure:
	return "1";
	case Ndis802_11AutoUnknown:
	return "2";
	default:
	return "<unknown>";
	}
}


static int
ndis_infra(void *wl, cmd_t *cmd, char **argv)
{
	NDIS_802_11_NETWORK_INFRASTRUCTURE imode;
	DWORD status = -1, length;

	if (*++argv) {
	if (!stricmp(*argv, "0") || !stricmp(*argv, "ibss") || !stricmp(*argv, "adhoc") ||
		!stricmp(*argv, "ad-hoc"))
	    imode = Ndis802_11IBSS;
	else if (!stricmp(*argv, "1") || !stricmp(*argv, "bss") || !stricmp(*argv, "managed") ||
		!strnicmp(*argv, "infra", 5))
	    imode = Ndis802_11Infrastructure;
	else if (!stricmp(*argv, "2") || !stricmp(*argv, "auto"))
	    imode = Ndis802_11AutoUnknown;
	else
	    return -1;
	length = sizeof(imode);
	status = ir_setinformation_fe(wl, cmd->set, (PUCHAR)&imode, &length);
	} else {
	length = sizeof(imode);
	status = ir_queryinformation_fe(wl, cmd->get, (PUCHAR)&imode, &length);
	if (status == NO_ERROR) {
	    printf("%s\n", imode2str(imode));
	}
	}
	return status;
}

static int
ndis_wepdefaults(void *wl, cmd_t *cmd, char **argv)
{
	DWORD status = -1, length;
	NDIS_802_11_RELOAD_DEFAULTS wep;

	length = sizeof(wep);
	status = ir_setinformation_fe(wl, cmd->set, (PUCHAR)&wep, &length);
	return status;
}

static int
ndis_generic_ulong(void *wl, cmd_t *cmd, char **argv)
{
	DWORD status = -1, length;
	ULONG val;
	char* endptr;

	if (*++argv) {
	val = strtoul(*argv, &endptr, 0);
	if (*endptr != '\0') {
	    /* not all the value string was parsed by strtoul */
	    return -1;
	}
	length = sizeof(ULONG);
	status = ir_setinformation_fe(wl, cmd->set, (PUCHAR)&val, &length);
	} else {
	length = sizeof(ULONG);
	status = ir_queryinformation_fe(wl, cmd->get, (PUCHAR)&val, &length);
	if (status == NO_ERROR) {
	    printf("%lu\n", val);
	}
	}
	return status;
}

/*
 * Do whatever is necessary to join a network.
 */
/* wl join <name|ssid> [key <0-3>:xxxxxx] [imode <>] [amode <>] */
static int
ndis_join(void *wl, cmd_t *cmd, char **argv)
{
	ULONG status, length, idx = 0, buflen = WLC_IOCTL_MAXLEN, ret;
	char *wep = NULL;
	NDIS_802_11_AUTHENTICATION_MODE amode = Ndis802_11AuthModeOpen;
	NDIS_802_11_NETWORK_INFRASTRUCTURE imode = Ndis802_11Infrastructure;
	NDIS_802_11_WEP_STATUS wstatus = Ndis802_11WEPDisabled;
	NDIS_802_11_SSID ssid;
	PNDIS_802_11_WEP wepkey = NULL;
	char* cmd_name;
	uint wsec = 0;
#if defined(BCMDBG)
	wl_wsec_key_t keys[DOT11_MAX_DEFAULT_KEYS];
	int keys_error[DOT11_MAX_DEFAULT_KEYS];
	uint i = 0;
	union {
		int index;
		wl_wsec_key_t key;
	} u;
#endif /* defined(BCMDBG) */

	cmd_name = *argv++;

	/* verify that SSID was specified and is a valid length */
	if (!*argv || (strlen(*argv) > DOT11_MAX_SSID_LEN))
		return -1;

	ssid.SsidLength = strlen(*argv);
	strncpy(ssid.Ssid, *argv, ssid.SsidLength);

	/* OID_802_11_INFRA clears all keys per the MSDN spec.
	 * If not specifying a key, but wsec == WEP_ENABLED, preserve keys, set the OID,
	 * and write keys back.
	 */

	memset(xpbuf, 0, WLC_IOCTL_MAXLEN);
	strcpy(xpbuf,"wsec");

	if ((ret = wl_get(wl, WLC_GET_VAR, &xpbuf[0], buflen)) < 0) {
		fprintf(stderr, "Could not query wsec status\n");
		return -1;
	}
	memcpy(&wsec, xpbuf, sizeof(wsec));

	if (wsec >= WEP_ENABLED)
		wstatus = Ndis802_11WEPEnabled;

	while (*++argv) {
		/* specified wep key */
		if (!stricmp(*argv, "wepkey") || !stricmp(*argv, "wep") || !stricmp(*argv, "key")) {
			if (!*++argv)
				return -1;

			/* WEP index specified */
			 if (*(argv[0]+1) == ':') {
				idx = *argv[0] - 0x30;
				if (idx < 0 || idx > 3) {
					fprintf(stderr, "Invalid key index %d specified\n", idx);
					return -1;
				}
				argv[0] += 2; /* colon + digit */
			}

			if ((wepkey = ndis_parse_wep(*argv)) == NULL)
				return -1;
		}
		/* specified infrastructure mode */
		else if (!stricmp(*argv, "imode") ||
		         !stricmp(*argv, "infra") ||
		         !stricmp(*argv, "mode")) {
			if (!*++argv) {
				fprintf(stderr, "%s %s: expected argument after \"infra\" keyword "
				        "but command line ended.\n", wlu_av0, cmd_name);
				return -1;
			} else if (!stricmp(*argv, "ibss") ||
			           !stricmp(*argv, "adhoc") ||
			           !stricmp(*argv, "ad-hoc")) {
				imode = Ndis802_11IBSS;
			} else if (!stricmp(*argv, "bss") ||
			           !stricmp(*argv, "managed") ||
			           !strnicmp(*argv, "infra", 5)) {
				imode = Ndis802_11Infrastructure;
			} else if (!stricmp(*argv, "openshared")) {
				imode = Ndis802_11AutoUnknown;
			} else {
				fprintf(stderr, "%s %s: unrecongnized parameter \"%s\" after "
				        "\"infra\" keyword\n", wlu_av0, cmd_name, *argv);
				return -1;
			}
		}
		/* specified authentication mode */
		else if (!stricmp(*argv, "amode") || !strnicmp(*argv, "auth", 4)) {
			if (!*++argv)
				return -1;
			if (!stricmp(*argv, "open"))
				amode = Ndis802_11AuthModeOpen;
			else if (!stricmp(*argv, "shared"))
				amode = Ndis802_11AuthModeShared;
			else if (!stricmp(*argv, "auto"))
				amode = Ndis802_11AuthModeAutoSwitch;
			else if (!stricmp(*argv, "wpa"))
				amode = Ndis802_11AuthModeWPA;
			else if (!stricmp(*argv, "wpapsk"))
				amode = Ndis802_11AuthModeWPAPSK;
			else if (!stricmp(*argv, "wpanone"))
				amode = Ndis802_11AuthModeWPANone;
			else if (!stricmp(*argv, "wpa2"))
				amode = Ndis802_11AuthModeWPA2;
			else if (!stricmp(*argv, "wpa2psk"))
				amode = Ndis802_11AuthModeWPA2PSK;
			else
				return -1;
		}
	}
#if defined(BCMDBG)
	/* wsec == WEP only, but no key specified; preserve existing keys */
	if (!wepkey && wstatus == Ndis802_11WEPEnabled) {
		for (i = 0; i < DOT11_MAX_DEFAULT_KEYS; i++) {
			u.index = i;
			keys_error[i] = wl_get(wl, WLC_GET_KEY, &u, sizeof(u));
			if (keys_error[i] && (u.key.algo == CRYPTO_ALGO_WEP1 || u.key.algo == CRYPTO_ALGO_WEP128)) {
				memcpy(&keys[i], &u.key, sizeof(u.key));
			}
		}
	}
#endif /* defined(BCMDBG) */

	/* set infrastructure mode */
	length = sizeof(imode);
	status = ir_setinformation_fe(wl, OID_802_11_INFRASTRUCTURE_MODE, (PUCHAR)&imode, &length);
	CHECK_STATUS("OID_802_11_INFRASTRUCTURE_MODE", status);

	/* set authentication mode */
	length = sizeof(amode);
	status = ir_setinformation_fe(wl, OID_802_11_AUTHENTICATION_MODE, (PUCHAR)&amode, &length);
	CHECK_STATUS("OID_802_11_AUTHENTICATION_MODE", status);

	/* set wep mode */
	if (wepkey) {
		length = wepkey->Length;
		wepkey->KeyIndex = idx | WSEC_NDIS_TX_KEY;	/* set as tx key by default */
		status = ir_setinformation_fe(wl, OID_802_11_ADD_WEP, (PUCHAR) wepkey, &length);
		CHECK_STATUS("OID_802_11_ADD_WEP", status);

		length = sizeof(wstatus);
		status = ir_setinformation_fe(wl, OID_802_11_WEP_STATUS, (PUCHAR)&wstatus,
		                           &length);
		/* Don't check error status here because OID_802_11_WEP_STATUS
		 * will return an error if there are no WEP keys are set.
		 * CHECK_STATUS("OID_802_11_WEP_STATUS", status);
		 */
	}
#if defined(BCMDBG)
	/* Copy keys back! */
	if (!wepkey && wstatus == Ndis802_11WEPEnabled) {
		for (i = 0; i < DOT11_MAX_DEFAULT_KEYS; i++) {
			if(!keys_error[i]) {
				wl_set(wl, WLC_SET_KEY, &keys[i], sizeof(keys[i]));
			}
		}
	}
#endif /* defined(BCMDBG) */

	/* set ssid */
	length = sizeof(ssid);
	status = ir_setinformation_fe(wl, OID_802_11_SSID, (PUCHAR) &ssid, &length);
	CHECK_STATUS("OID_802_11_SSID", status);

	return (int) status;
}


/* Initiate an active scan for wireless networks. */
static int ndis_scan(void *wl, cmd_t *cmd, char **argv)
{
	ULONG status;
	long length;

	if (!*++argv) {
		length = 0;
		status = ir_setinformation_fe((HANDLE) wl, OID_802_11_BSSID_LIST_SCAN, NULL, &length);
	} else {
		printf("NDIS broadcast scan takes no SSID argument\n");
		status = -1;
	}
	return status;
}

static int
ndis_addkey(void *wl, cmd_t *cmd, char **argv)
{
	PNDIS_802_11_WEP wepkey = NULL;
	PNDIS_802_11_KEY key;
	ULONG length;
	ULONG keyindex;

	if (!*++argv)
		return -1;

	keyindex = strtol(argv[0], NULL, 0);

	if (!*++argv)
		return -1;

	if ((wepkey = ndis_parse_wep(argv[0])) == NULL)
		return -1;

	length = sizeof(NDIS_802_11_KEY) - 1 + wepkey->KeyLength;
	key = (PNDIS_802_11_KEY) malloc(length);
	memset(key, 0, length);

	/* Tx flag set by default; may be cleared below. */
	key->KeyIndex = keyindex | WSEC_NDIS_TX_KEY;

	memcpy(key->KeyMaterial, wepkey->KeyMaterial, wepkey->KeyLength);
	key->KeyLength = wepkey->KeyLength;
	free(wepkey);

	while (*++argv) {
		if (!strnicmp("notx", *argv, 4)) {
			key->KeyIndex &= ~WSEC_NDIS_TX_KEY;

		} else if (aton(*argv, key->BSSID, ETHER_ADDR_LEN)) {
			key->KeyIndex |= WSEC_NDIS_PAIR_KEY;

		} else {
			return -1;
		}
	}

	/* paired key without Tx flag is illegal */
	if (key->KeyIndex & WSEC_NDIS_PAIR_KEY)
		key->KeyIndex |= WSEC_NDIS_TX_KEY;
	else
		memcpy(&key->BSSID, &ether_bcast, ETHER_ADDR_LEN);

	return ir_setinformation_fe((HANDLE) wl, OID_802_11_ADD_KEY,
	                         (PUCHAR) key, &length);
}

static int
ndis_addwep(void *wl, cmd_t *cmd, char **argv)
{
	PNDIS_802_11_WEP wepkey = NULL;
	ULONG length;
	ULONG keyindex;

	if (!*++argv)
		return -1;

	keyindex = strtol(argv[0], NULL, 0);

	if (!*++argv)
		return -1;

	if ((wepkey = ndis_parse_wep(argv[0])) == NULL)
		return -1;

	wepkey->KeyIndex = keyindex;
	wepkey->KeyIndex |= WSEC_NDIS_TX_KEY;	/* set as tx key by default */

	length = wepkey->Length;
	return ir_setinformation_fe((HANDLE) wl, OID_802_11_ADD_WEP, (PUCHAR) wepkey, &length);
}

static int
ndis_wepstatus(void *wl, cmd_t *cmd, char **argv)
{
	long data = 0, length;
	ULONG status;

	if (*++argv) {
		if (strnicmp(*argv, "off", strlen(*argv)) == 0)
			data = Ndis802_11WEPDisabled;
		else if (strnicmp(*argv, "on", strlen(*argv)) == 0)
			data = Ndis802_11WEPEnabled;
		length = sizeof(data);
		status = ir_setinformation_fe(wl, cmd->set, (PUCHAR) &data, &length);
		return status;
	} else {
		length = sizeof(data);
		status = ir_queryinformation_fe(wl, cmd->get, (PUCHAR)&data, &length);
		if (status == NO_ERROR) {
			if (length != sizeof(data)) {
				printf("ir_queryinformation returned a bogus length %d "
				       "(expected %lu)\n", length,
				       (unsigned long)sizeof(data));
			} else {
				switch (data) {
				case Ndis802_11WEPEnabled:
					printf("WEP is enabled.\n");
					break;
				case Ndis802_11WEPDisabled:
					printf("WEP is disabled.\n");
					break;
				case Ndis802_11WEPKeyAbsent:
					printf("The WEP key is absent.\n");
					break;
				case Ndis802_11WEPNotSupported:
					printf("The WEP key is not supported.\n");
					break;
				default:
					printf("Unknown return value = 0x%x\n");
				}
			}
		}
	}
	return status;
}

static int
ndis_disassoc(void *wl, cmd_t *cmd, char **argv)
{
	int length = 0;
	return ir_setinformation_fe(wl, OID_802_11_DISASSOCIATE, (PUCHAR) NULL, &length);
}

/* Front end for ir_queryinformation */
static int
ir_queryinformation_fe(void *wl, int cmd, void* input_buf, DWORD *input_len)
{
	int error = NO_ERROR;
		uint tx_len = MIN(*input_len, 1024);

	if (remote_type == NO_REMOTE) {
		error = (int)ir_queryinformation((HANDLE)wl, cmd, input_buf, input_len);
	}
	else {
		error = rwl_queryinformation_fe(wl, cmd, input_buf, input_len, debug,
										REMOTE_GET_IOCTL);
	}
	return error;
}
#define MAX_SETOID_RETRIES 10
/* Front end for ir_setinformation */
int
ir_setinformation_fe(void *wl, int cmd, void* buf, DWORD *len)
{
	int error;
	ULONG Iterations = 0;

	if (remote_type == NO_REMOTE) {
		while(Iterations < MAX_SETOID_RETRIES){
			error = (int)ir_setinformation((HANDLE)wl, cmd, buf, len);
				if (error != ERROR_GEN_FAILURE)
					break;
			Iterations++;
		}
	}
	else {
		error = rwl_setinformation_fe(wl, cmd, buf, len, debug, REMOTE_SET_IOCTL);
	}
	return error;
}

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

static HANDLE
sendpkt_open_uio(CHAR *dev_name)
{
	DWORD   DesiredAccess;
	DWORD   ShareMode;
	LPSECURITY_ATTRIBUTES   lpSecurityAttributes = NULL;

	DWORD   CreationDistribution;
	DWORD   FlagsAndAttributes;
	HANDLE  TemplateFile;
	HANDLE  Handle;
	DWORD   BytesReturned;

	DesiredAccess = GENERIC_READ|GENERIC_WRITE;
	ShareMode = 0;
	CreationDistribution = OPEN_EXISTING;
	FlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	TemplateFile = (HANDLE)INVALID_HANDLE_VALUE;

        Handle = CreateFile(
                    (const char*)dev_name,
                    DesiredAccess,
                    ShareMode,
                    lpSecurityAttributes,
                    CreationDistribution,
                    FlagsAndAttributes,
                    TemplateFile);

	/* Wait for the driver to finish binding. */
	if (!DeviceIoControl(
	            Handle,
	            IOCTL_NDISUIO_BIND_WAIT,
	            NULL,
	            0,
	            NULL,
	            0,
	            &BytesReturned,
	            NULL)) {
		printf("open_uio: IOCTL_NDISIO_BIND_WAIT failed, error %d\n",
		       GetLastError());
		CloseHandle(Handle);
		Handle = INVALID_HANDLE_VALUE;
	}
	return (Handle);
}

static BOOL
sendpkt_open_dev(HANDLE Handle, CHAR *pDeviceName)
{
	WCHAR   wNdisDeviceName[MAX_PATH];
	INT     wNameLength;
	INT     NameLength = strlen(pDeviceName);
	DWORD   BytesReturned;
	INT     i;

	/* Convert to unicode string - non-localized... */
	wNameLength = 0;
	for (i = 0; i < NameLength && i < MAX_PATH-1; i++)
	{
	    wNdisDeviceName[i] = (WCHAR)pDeviceName[i];
	    wNameLength++;
	}
	wNdisDeviceName[i] = L'\0';

	return (DeviceIoControl(
	            Handle,
	            IOCTL_NDISUIO_OPEN_DEVICE,
	            (LPVOID)&wNdisDeviceName[0],
	            wNameLength*sizeof(WCHAR),
	            NULL,
	            0,
	            &BytesReturned,
	            NULL));
}

static BOOL
sendpkt_set_filter(HANDLE Handle, ULONG FilterValue)
{
	UCHAR       SetBuffer[sizeof(NDISUIO_SET_OID)];
	PNDISUIO_SET_OID  pSetOid;
	DWORD       BytesReturned;

	pSetOid = (PNDISUIO_SET_OID)&SetBuffer[0];
	pSetOid->Oid = OID_GEN_CURRENT_PACKET_FILTER;
	memcpy(&pSetOid->Data[0], &FilterValue, sizeof(FilterValue));

	return DeviceIoControl(Handle,
	                        IOCTL_NDISUIO_SET_OID_VALUE,
	                        (LPVOID)&SetBuffer[0],
	                        sizeof(SetBuffer),
	                        (LPVOID)&SetBuffer[0],
	                        0,
	                        &BytesReturned,
	                        NULL);
}

static BOOL
sendpkt_get_ether_shost(HANDLE Handle, PUCHAR pSrcMacAddr)
{
	BOOL	ret;
	DWORD       BytesReturned;
	UCHAR       QueryBuffer[sizeof(NDISUIO_QUERY_OID) + ETHER_ADDR_LEN];
	PNDISUIO_QUERY_OID  pQueryOid;

	pQueryOid = (PNDISUIO_QUERY_OID)&QueryBuffer[0];
	pQueryOid->Oid = OID_802_3_CURRENT_ADDRESS;

	ret = DeviceIoControl(Handle,
	                      IOCTL_NDISUIO_QUERY_OID_VALUE,
	                      (LPVOID)&QueryBuffer[0],
	                      sizeof(QueryBuffer),
	                      (LPVOID)&QueryBuffer[0],
	                      sizeof(QueryBuffer),
	                      &BytesReturned,
	                      NULL);
	if (ret)
		memcpy(pSrcMacAddr, pQueryOid->Data, ETHER_ADDR_LEN);
	return ret;
}

static int
sendpkt_do(HANDLE Handle, uint8 *ether_dhost, uint8 *ether_shost, uint8 *oui,
           uint16 type, BOOL snap, DWORD pktlen, DWORD datalen, PUCHAR data)
{
	PUCHAR      pWriteBuf = NULL;
	int		retcode = 0;
	PUCHAR      pData;
	UINT        i;
	struct ether_header *eh;
	DWORD	hdrlen;
	DWORD	len;
	DWORD       BytesWritten;
	BOOL        bSuccess;
	struct dot11_llc_snap_header *lsh;

	pWriteBuf = malloc(pktlen);

	if (pWriteBuf == NULL) {
		printf("sendpkt_do: Failed to malloc %d bytes\n", pktlen);
		return -1;
	}

	eh = (struct ether_header *)pWriteBuf;
	memcpy(eh->ether_dhost, ether_dhost, ETHER_ADDR_LEN);
	memcpy(eh->ether_shost, ether_shost, ETHER_ADDR_LEN);
	eh->ether_type = hton16(type);
	pData = (PUCHAR)(eh + 1);
	hdrlen = sizeof(struct ether_header);

	if (snap) {
		eh->ether_type = hton16((uint16)(pktlen - sizeof(struct ether_header)));
		lsh = (struct dot11_llc_snap_header *)&eh[1];
		lsh->dsap = 0xaa;
		lsh->ssap = 0xaa;
		lsh->ctl  = 0x03;
		memcpy(lsh->oui, oui, DOT11_OUI_LEN);
		lsh->type = hton16(type);
		pData  += sizeof(struct dot11_llc_snap_header);
		hdrlen += sizeof(struct dot11_llc_snap_header);
	}

	len = min(datalen, pktlen - hdrlen);
	memcpy(pData, data, len);
	pData += len;
	for (i = 1; i <= pktlen - hdrlen - len; i++)
		*pData++ = (UCHAR)i;

	bSuccess = WriteFile(Handle,
	                     pWriteBuf,
	                     pktlen,
	                     &BytesWritten,
	                     NULL);
	if (!bSuccess) {
		printf("sendpkt_do: WriteFile failed on Handle %p\n", Handle);
		retcode = -1;
	} else {
		printf("sent to %02x:%02x:%02x:%02x:%02x:%02x ",
		       ether_dhost[0], ether_dhost[1], ether_dhost[2],
		       ether_dhost[3], ether_dhost[4], ether_dhost[5]);
		if (snap)
			printf("oui %02x:%02x:%02x ",
			       lsh->oui[0], lsh->oui[1], lsh->oui[2]);
		printf("type %04x len %d\n", type, pktlen);
		pData = (PUCHAR)(eh + 1);
		if (snap)
			pData  += sizeof(struct dot11_llc_snap_header);
		for (i = 1; i <= pktlen - hdrlen; i++) {
			printf("%02x ", *pData++);
			if (i % 16 == 0)
				printf("\n");
		}
	}

	if (pWriteBuf)
		free(pWriteBuf);
	return retcode;
}

static BOOL
aton(const char *s, unsigned char *d, int n)
{
	    char *c = (char *) s;
	    int i = 0;

	    memset(d, 0, n);
	    for (;;) {
	            d[i++] = (unsigned char) strtoul(c, &c, 16);
	            if (!*c++ || i == n)
	                    break;
	    }
	    return (i == n);
}

static int
sendpkt(void *wl, cmd_t *cmd, char **argv)
{
	int		retcode = -1;
	CHAR	ndisuio[] = "\\\\.\\\\bcmuio";
	HANDLE	uio;
	PADAPTER	pdev;
	ULONG	n;
	CHAR	devname[120] = "";
	ULONG	filter;
	uint8	ether_shost[ETHER_ADDR_LEN];
	BOOL	default_dest = TRUE;
	uint8       default_dhost[ETHER_ADDR_LEN] = {0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6};
	uint8       ether_dhost[ETHER_ADDR_LEN];
	uint8	oui[DOT11_OUI_LEN] = {0x01, 0x02, 0x03};
	uint16      type = 0x888e;			/* 802.1X Authentication */
	BOOL	snap = TRUE;
	DWORD	pktlen = 60;
	UINT	datalen = 0;
	UCHAR	data[1500] = "";

	while (*++argv) {
		/* specified dest mac address */
		if (!stricmp(*argv, "dest")) {
			if (!*++argv)
				return -1;
			if (!aton(*argv, ether_dhost, ETHER_ADDR_LEN))
				return -1;
			default_dest = FALSE;
		}
		else if (!stricmp(*argv, "oui")) {
			if (!*++argv)
				return -1;
			if (!aton(*argv, oui, DOT11_OUI_LEN))
				return -1;
		}
		else if (!stricmp(*argv, "type")) {
			if (!*++argv)
				return -1;
			sscanf(*argv, "%x", &type);
		}
		else if (!stricmp(*argv, "eth")) {
			snap = FALSE;
		}
		else if (!stricmp(*argv, "len")) {
			if (!*++argv)
				return -1;
			sscanf(*argv, "%d", &pktlen);
		}
		else if (!stricmp(*argv, "data")) {
			PUCHAR d = data;
			PUCHAR s;
			if (!*++argv)
				return -1;
			memset(d, 0, 1500);
			while (*argv) {
				s = *argv;
				*d++ = (UCHAR)strtoul(s, &s, 16);
				datalen++;
				argv++;
			}
			break;
		}
		else
			return -1;
	}

	uio = sendpkt_open_uio(ndisuio);
	if (uio == INVALID_HANDLE_VALUE) {
		printf("Failed to open %s\n", ndisuio);
		return -1;
	}

	/* Find wireless adapter to form its device name */
	for (n = 0; n < ndevs; n++) {
		pdev = &devlist[n];
		if (pdev && pdev->type == IR_WIRELESS)
			sprintf(devname, "\\DEVICE\\%s", pdev->name);
	}
	if (devname[0] == '\0') {
		printf("Failed to find wireless adapter.\n");
		goto cleanup;
	}

	if (!sendpkt_open_dev(uio, devname)) {
		printf("Failed to open %s\n", devname);
		goto cleanup;
	}
	printf("opened %s\n", devname);

	if (!sendpkt_get_ether_shost(uio, ether_shost)) {
		printf("Failed to get source ethernet address.\n");
		return -1;
	}
	if (default_dest)
		memcpy(ether_dhost, default_dhost, ETHER_ADDR_LEN);

	filter = NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_BROADCAST;
	if (!sendpkt_set_filter(uio, filter)) {
		printf("Failed to set filter %x\n", filter);
		goto cleanup;
	}

	retcode = sendpkt_do(uio, ether_dhost, ether_shost, oui, type, snap,
	                     pktlen, datalen, data);

cleanup:
	if (uio != INVALID_HANDLE_VALUE)
	CloseHandle(uio);
	return retcode;
}
