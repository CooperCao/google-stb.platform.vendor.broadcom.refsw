/*
 * Linux port of dhd command line utility, hacked from wl utility.
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
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

/* Common header file for dhdu_linux.c and dhdu_ndis.c */

#ifndef _dhdu_common_h
#define _dhdu_common_h

#if !defined(RWL_WIFI) && !defined(RWL_DONGLE) && !defined(RWL_SOCKET) && \
	!defined(RWL_SERIAL)

#define NO_REMOTE       0
#define REMOTE_SERIAL 	1
#define REMOTE_SOCKET 	2
#define REMOTE_WIFI     3
#define REMOTE_DONGLE   4

/* For cross OS support */
#define LINUX_OS  	1
#define WIN32_OS  	2
#define MAC_OSX		3
#define BACKLOG 	4
#define WINVISTA_OS	5
#define INDONGLE	6
#define EFI			7

#define RWL_WIFI_ACTION_CMD   		"wifiaction"
#define RWL_WIFI_GET_ACTION_CMD         "rwlwifivsaction"
#define RWL_DONGLE_SET_CMD		"dongleset"

#define SUCCESS 	1
#define FAIL   		-1
#define NO_PACKET       -2

/* Added for debug utility support */
#define ERR		stderr
#define OUTPUT		stdout
#define DEBUG_ERR	0x0001
#define DEBUG_INFO	0x0002
#define DEBUG_DBG	0x0004

#define DPRINT_ERR	if (defined_debug & DEBUG_ERR) \
			    fprintf
#define DPRINT_INFO	if (defined_debug & DEBUG_INFO) \
			    fprintf
#define DPRINT_DBG	if (defined_debug & DEBUG_DBG) \
			    fprintf

extern int wl_get(void *wl, int cmd, void *buf, int len);
extern int wl_set(void *wl, int cmd, void *buf, int len);
#endif /* !RWL_WIFI & !RWL_DONGLE & !RWL_SOCKET & !RWL_SERIAL */

/* DHD utility function declarations */
extern int dhd_check(void *dhd);
extern int dhd_atoip(const char *a, struct ipv4_addr *n);
extern int dhd_option(char ***pargv, char **pifname, int *phelp);
void dhd_usage(cmd_t *port_cmds);

/* Remote DHD declarations */
int remote_type = NO_REMOTE;
unsigned short defined_debug = DEBUG_ERR | DEBUG_INFO;
extern char *g_rwl_buf_mac;
extern char* g_rwl_device_name_serial;
unsigned short g_rwl_servport;
char *g_rwl_servIP = NULL;

#ifdef LINUX

static int process_args(struct ifreq* ifr, char **argv);

#elif defined(WIN32)

#define NDIS_NUM_ARGS	16
static DWORD ndevs;
static int ap_mode = 0;
static int debug = 0;
static int ReadTotalTimeout = 0;
static bool batch_in_client = FALSE;

/* dword align allocation */
static union {
	char bufdata[WLC_IOCTL_MAXLEN];
	uint32 alignme;
} bufstruct_wlu;

static char *xpbuf = (char*) &bufstruct_wlu.bufdata;
static cmd_t * dhd_find_cmd(char* name);

static WINERR select_adapter(HANDLE, int adapter);
static void display_err(PCHAR prefix, WINERR status);
static char io_buf[WLC_IOCTL_MAXLEN];
static WINERR process_args(HANDLE irh, char **argv);

static int ir_queryinformation_fe(void *wl, int cmd, void* input_buf, DWORD *input_len);
static int ir_setinformation_fe(void *wl, int cmd, void* buf, DWORD *len);
static const char *imode2str(NDIS_802_11_NETWORK_INFRASTRUCTURE modeval);
static const char *ndis_network_type_name(NDIS_802_11_NETWORK_TYPE type);
static DWORD ndevs;
#endif /* LINUX */

#ifdef IL_BIGENDIAN
bool swap = FALSE;
#define htod32(i) (swap?bcmswap32(i):i)
#define dtoh32(i) (swap?bcmswap32(i):i)
#define dtohenum(i) ((sizeof(i) == 4) ? dtoh32(i) : ((sizeof(i) == 2) ? htod16(i) : i))
#else /* IL_BIGENDIAN */
#define dtoh32(i) i
#define dtoh16(i) i
#endif	/* IL_BIGENDIAN */

#endif  /* _dhdu_common_h_ */
