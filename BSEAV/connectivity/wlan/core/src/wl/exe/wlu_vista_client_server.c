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
#include <iphlpapi.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <typedefs.h>
#include <wlioctl.h>
#include <proto/ethernet.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <wlanapi.h>
#include <signal.h>
#include "epictrl.h"

#include "wlu.h"
#include "oidencap.h"
#include "wlu_remote_vista.h"
#include "wlu_remote.h"
#include "wlu_client_shared.h"
#include <irelay.h>

// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")

#define WINERR DWORD
#define MAX_PROFILE_SIZE 1024
#define stricmp _stricmp
#define strnicmp _strnicmp

#define MAX_TRIES 3
#define MAX_RETRIES 100

static union {
	char bufdata[WLC_IOCTL_MAXLEN];
	uint32 alignme;
}bufstruct_wlu;

#define VISTABUF_MAXLEN WLC_IOCTL_MAXLEN

static char *vistabuf = (char*) &bufstruct_wlu.bufdata;

/* Used to generate XML schema elements */
typedef struct {
	char SSID[DOT11_MAX_SSID_LEN+1];
	char key[DOT11_MAX_KEY_SIZE+1];
	char passphrase[WSEC_MAX_PSK_LEN+1];
	int connectionType;
	int authMode;
	int keyIndex;
	int cipher;
	int savePN; /* Save to Vista UI */
} vista_bss_info_t;

static enum connectionType {
	connectionType_infra = 0,
	connectionType_ibss,
	connectionType_auto
};

static enum authMode {
	authMode_open = 0,
	authMode_shared,
	authMode_wpapsk,
	authMode_wpa2psk
};

static enum cipherType {
	cipherType_none = 0,
	cipherType_wep,
	cipherType_tkip,
	cipherType_aes
};


int vista_cipher_select(void *wl, wl_instance_info_t *inst);
int vista_parse_and_validate(void *wl, vista_bss_info_t* bss, char** argv, wchar_t* profile,
wl_instance_info_t *inst);
void display_err(PCHAR prefix, WINERR status);
cmd_t* wl_find_cmd(char* name);
void vista_build_schema(vista_bss_info_t* bss, LPWSTR profile);
DWORD Bind(LPCTSTR lpszDeviceName);
DWORD Unbind();
DWORD QueryInformation(ULONG uOid, PUCHAR pInbuf, PDWORD dwInlen);
DWORD SetInformation(ULONG uOid, PUCHAR pInbuf, PDWORD dwInlen);
DWORD CallIRelay(HANDLE hDevice, PRelayHeader prh);
BOOL IsBroadcomAdapter();
int epi_xlist_all(void *wl, cmd_t *cmd, char **argv, BOOL xlist);
DWORD ConnectToVirtualAdapter(HANDLE wl);
DWORD FindBrcmAdapter(HANDLE irh, char * ifname);
#if !defined(VISTA_SERVER) && defined(WLP2P)
int vista_set_p2p_vndr_ie(void *wl, const char *command, uint32 pktflag_ok, char **argv);
#endif /* !VISTA_SERVER && WLP2P */

static HANDLE hDevHandle = INVALID_HANDLE_VALUE;
GUID dev;
DWORD ndevs;
GUID devlist[10];

/* added for vista specific commands */
cmd_func_t epi_list;
cmd_func_t epi_xlist;
cmd_func_t vista_join;
cmd_func_t vista_disassoc;
cmd_func_t vista_set_pmk;
cmd_func_t vista_sup_wpa;
cmd_func_t vista_wsec;
cmd_func_t vista_legacy;
#if !defined(VISTA_SERVER) && defined(WLP2P)
cmd_func_t vista_p2p_vndr_ie;
#endif /* !VISTA_SERVER && WLP2P */
cmd_func_t	wl_find_vif;

cmd_t vista_cmds[] = {
		/* list & xlist are common for all adapters (including virtual etc) */
/*[0]*/	{ "list", epi_list, -1, -1,
		"list all installed Wireless adapters" },
/*[1]*/	{ "xlist", epi_xlist, -1, -1,
		"list all installed adapters with extended information" },
		/* legacylink & sup_wpa are specific to vista wlan */
/*[2]*/	{ "legacylink", vista_legacy, WLC_LEGACY_LINK_BEHAVIOR, WLC_LEGACY_LINK_BEHAVIOR,
		  "set the 'Vista IBSS legacy link behavior' which simulates a STA joining this ibss\n"
		  "\t<mac address> - optional mac address indicating address of joining STA" },
/*[3]*/	{ "sup_wpa", vista_sup_wpa, -1, -1,
		  "Sets the WPA supplicant mode (DEPRECATED in Vista)" },
		/* Below commands are common and overriden for vista wlan */
/*[4]*/	{ "join", vista_join, -1, -1,
		"Join a specified network SSID.\n"
		"\tJoin syntax is: join <name|ssid> [key <0-3>:xxxxx] [imode bss|ibss|auto] "
		"[amode open|shared|wpapsk|wpa2psk] [psk xxxxxxxx] [SavePN 0/1]"},
/*[5]*/	{ "set_pmk", vista_set_pmk, WLC_GET_VAR, WLC_SET_VAR,
		  "Set the PSK used by the ACM layer" },
/*[6]*/	{ "wsec", vista_wsec, WLC_GET_VAR, WLC_SET_VAR,
		  "Wireless security bit-vector for Vista" },
/*[7]*/	{ "disassoc", vista_disassoc, -1, -1,
		"Disassociates from a network using the ACM interface." },
#if !defined(VISTA_SERVER) && defined(WLP2P)
/*[8]*/	{ "p2p_vndr_ie", vista_p2p_vndr_ie, WLC_GET_VAR, WLC_SET_VAR,
		"Add/del a vendor proprietary IE (global list) to/from all P2P 802.11 management "
		"and public action frames, or dump the list.\n"
		"Usage: wl p2p_vndr_ie add|del <pktflag> length OUI hexdata\n"
		"       wl p2p_vndr_ie\n"
		"<pktflag>: Bit  0 - Beacons           Bit  1 - Probe Rsp\n"
		"           Bit  2 - Assoc/Reassoc Rsp Bit  3 - Auth Rsp\n"
		"           Bit  4 - Probe Req         Bit  5 - Assoc/Reassoc Req\n"
		"           Bit 12 - GON Req           Bit 13 - GON Rsp\n"
		"           Bit 14 - GON Confirm       Bit 15 - Invite Req\n"
		"           Bit 16 - Invite Rsp        Bit 17 - Disc Req\n"
		"           Bit 18 - Disc Rsp          Bit 19 - Prov Disc Req\n"
		"           Bit 20 - Prov Disc Rsp\n"
		"Example: wl p2p_vndr_ie add 3 10 00:90:4C 0101050c121a03\n"
		"         to add this IE to all P2P beacons and probe responses\n"
		"         wl p2p_vndr_ie\n"
		"         to dump the global list" },
#endif /* !VISTA_SERVER && WLP2P */
#if !defined(VISTA_SERVER)
/*[9]*/	{ "find_vif", wl_find_vif, WLC_GET_VAR, WLC_SET_VAR, "Start Virutal interface cmd" },
#endif
/*[10]*/	{ NULL, NULL, 0, 0, NULL }
};


#if !defined(VISTA_SERVER)
int
wl_find_vif (void *wl, cmd_t *cmd, char **argv)
{
	int err;
	int argc = ARGCNT(argv);
	int ap;
	int *val;

	/*
	 * The default behaviour of find_vif was to bringup the VIF
	 * as AP, retain the behaviour.
	 */
	ap = 1;

	/* Invoked with ap/sta option */
	if ((argc == 2) && (stricmp(argv[1], "sta") == 0))
		ap = 0;

	err = wlu_var_getbuf (wl, cmd->name, &ap, sizeof(ap), (void **)&val);
	if (err) {
		printf("%s(): Failed \r\n", __FUNCTION__);
		return err;
	}

	return 0;
}
#endif

int
wl_get(void *wl, int cmd, void *buf, int len)
{
	DWORD dwlen = len;
	int error;

#ifdef VISTA_SERVER
	if(hDevHandle != INVALID_HANDLE_VALUE) {
		error = QueryInformation(WL_OID_BASE + cmd, (PUCHAR) buf, &dwlen);
		if (error != 0) {
			return BCME_IOCTL_ERROR;
		}
	} else {
		error = (int)ir_vista_queryinformation((HANDLE)wl, &dev, WL_OID_BASE + cmd, buf, &dwlen);
		if (error != 0) {
			return BCME_IOCTL_ERROR;
		}
	}
	return BCME_OK;
#else
	if (remote_type == NO_REMOTE) {

		if(hDevHandle != INVALID_HANDLE_VALUE) {
			error = QueryInformation(WL_OID_BASE + cmd, (PUCHAR) buf, &dwlen);
			if (error != 0) {
				return BCME_IOCTL_ERROR;
			}
		} else {
			error = (int)ir_vista_queryinformation((HANDLE)wl, &dev, WL_OID_BASE + cmd, buf, &dwlen);
			if (error != 0) {
				return BCME_IOCTL_ERROR;
			}
		}
		return BCME_OK;
	} else {
                error = rwl_queryinformation_fe(wl, cmd, buf, &dwlen, FALSE, REMOTE_GET_IOCTL);
		return error;
	}
#endif
}

int
wl_set(void *wl, int cmd, void *buf, int len)
{
	DWORD dwlen = len;
	int error;

#ifdef VISTA_SERVER
	if(hDevHandle != INVALID_HANDLE_VALUE) {
		error = SetInformation(WL_OID_BASE + cmd, (PUCHAR) buf, &dwlen);
		if (error != 0) {
			return BCME_IOCTL_ERROR;
		}
	} else {
		error = (int)ir_vista_setinformation((HANDLE)wl, &dev, WL_OID_BASE + cmd, buf, &dwlen);
		if (error != 0) {
			return BCME_IOCTL_ERROR;
		}
	}
	return BCME_OK;
#else
        if (remote_type == NO_REMOTE) {

		if(hDevHandle != INVALID_HANDLE_VALUE) {
			error = SetInformation(WL_OID_BASE + cmd, (PUCHAR) buf, &dwlen);
			if (error != 0) {
				return BCME_IOCTL_ERROR;
			}
		} else {
			error = (int)ir_vista_setinformation((HANDLE)wl, &dev, WL_OID_BASE + cmd, buf, &dwlen);
			if (error != 0) {
				return BCME_IOCTL_ERROR;
			}
		}
		return BCME_OK;
	} else {
		error = rwl_setinformation_fe(wl, cmd, buf, &dwlen, FALSE, REMOTE_SET_IOCTL);
		return error;
	}
#endif
}

int
epi_xlist_wlan(void *wl, cmd_t *cmd, char **argv, BOOL xlist)
{
	GUID tmpdev;
	HANDLE tempHandle;
	ULONG n;
	wl_instance_info_t inst;
	struct ether_addr *addr;
	char buf[16];
	WINERR status;
	WLAN_INTERFACE_INFO_LIST *iflist = NULL;


	status = WlanEnumInterfaces(wl, NULL, &iflist);
	if (status != ERROR_SUCCESS) {
		printf("WlanEnumInterfaces failed with %d\n", status);
		return status;
	}

	/* print extended info plus MAC address for each device */
	tmpdev = dev;
	tempHandle = hDevHandle;
	hDevHandle = INVALID_HANDLE_VALUE; // Make sure only physical adapter is used

	for (n = 0; n < iflist->dwNumberOfItems; n++) {
		dev = iflist->InterfaceInfo[n].InterfaceGuid;

		if (wl_check(wl) < 0) {
			/* can't get info for non-BRCM adapters */
			continue;
		}
		if (wl_get(wl, WLC_GET_INSTANCE, &inst, sizeof(inst)) < 0)
			inst.instance = -1;

	strcpy(buf, "cur_etheraddr");
	if (wl_get(wl, WLC_GET_VAR, buf, sizeof(buf)) < 0) {
#ifdef VISTA_SERVER
		sprintf(*argv, "%4d: wl%d MAC: error getting IOC_CUR_ETHERADDR\n", n,
		inst.instance);
#else
		printf("%4d: wl%d MAC: error getting IOV_CUR_ETHERADDR\n", n, inst.instance);
#endif
	}
	else {
		addr = (struct ether_addr *)buf;
#ifdef VISTA_SERVER
		sprintf(*argv, "%4d:\twl%-2d",n, inst.instance);

		if(xlist) {
			sprintf(*argv, "wireless ???? {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X} ",
				dev.Data1, dev.Data2, dev.Data3,
				dev.Data4[0], dev.Data4[1], dev.Data4[2], dev.Data4[3],
				dev.Data4[4], dev.Data4[5], dev.Data4[6], dev.Data4[7]);
		}

		sprintf(*argv,"MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
			addr->octet[0], addr->octet[1], addr->octet[2],
			addr->octet[3], addr->octet[4], addr->octet[5]);


#else
		printf("%4d:\twl%-2d",n, inst.instance);

		if(xlist) {
			printf("wireless ???? {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X} ",
				dev.Data1, dev.Data2, dev.Data3,
				dev.Data4[0], dev.Data4[1], dev.Data4[2], dev.Data4[3],
				dev.Data4[4], dev.Data4[5], dev.Data4[6], dev.Data4[7]);
		}

		printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
			addr->octet[0], addr->octet[1], addr->octet[2],
			addr->octet[3], addr->octet[4], addr->octet[5]);
#endif
		}
	}
	hDevHandle = tempHandle;
	dev = tmpdev;

	return BCME_OK;
}

/* Do whatever is necessary to join a network. */
int
vista_join(void *wl, cmd_t *cmd, char **argv) {
	ULONG connectStatus;
	DWORD addProfileStatus;
	PWLAN_CONNECTION_PARAMETERS pConnectionParameters;
	DWORD err;
	char* cmd_name;
	vista_bss_info_t *target;
	LPWSTR profile;
	wchar_t profile_name[DOT11_MAX_SSID_LEN+1];
	wl_instance_info_t inst;
	cmd_name = *argv++;

	if (wl == NULL) {
#ifdef VISTA_SERVER
		argv--;
		sprintf(*argv, "wl: wl handle is NULL\n");
#else
		fprintf(stderr, "wl: wl handle is NULL\n");
#endif

		return BCME_ERROR;
	}

	if (wl_get(wl, WLC_GET_INSTANCE, &inst, sizeof(inst)) < 0)
		inst.instance = -1;
	/* Disconnect from old PN first */
	if ((err = WlanDisconnect(wl, &dev, 0)) != ERROR_SUCCESS) {
		char errMsg[1024] = "Unknown Error";
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0,
			errMsg, sizeof(errMsg), NULL);
#ifdef VISTA_SERVER
		argv--;
		sprintf(*argv, "wl%d: %s\n", inst.instance, errMsg);
#else
		fprintf(stderr, "wl%d: %s\n", inst.instance, errMsg);
#endif

		return BCME_ERROR;
	};

	target = (vista_bss_info_t*)malloc(sizeof(vista_bss_info_t));
	profile = (wchar_t*)malloc(MAX_PROFILE_SIZE*sizeof(wchar_t));
	pConnectionParameters = (PWLAN_CONNECTION_PARAMETERS)
	        malloc(sizeof(WLAN_CONNECTION_PARAMETERS));

	if (!target || !profile || !pConnectionParameters) {
#ifdef VISTA_SERVER
		argv--;
		sprintf(*argv,"%c",'\0');
#endif
		return BCME_ERROR;
		}
	memset(target, 0, sizeof(*target));
	memset(profile, 0, MAX_PROFILE_SIZE*sizeof(wchar_t));
	memset(profile_name, 0, DOT11_MAX_SSID_LEN*sizeof(wchar_t));

	target->cipher = vista_cipher_select(wl, &inst);

	/* Profile contains XML string to pass to ACM */
	if(vista_parse_and_validate(wl, target, argv, profile, &inst) < 0) {
#ifdef VISTA_SERVER
		argv--;
		sprintf(*argv,"%c",'\0');
#endif
		return BCME_ERROR;
		}
	/* Build the connection using the ACM interface */
	/* Do not issue discrete OIDs */

	pConnectionParameters->wlanConnectionMode = wlan_connection_mode_temporary_profile;
	pConnectionParameters->pDot11Ssid = NULL;
	pConnectionParameters->pDesiredBssidList = NULL;
	pConnectionParameters->dot11BssType = (target->connectionType == connectionType_infra) ?
		dot11_BSS_type_infrastructure : dot11_BSS_type_independent;

	if (pConnectionParameters->dot11BssType == dot11_BSS_type_infrastructure)
		pConnectionParameters->dwFlags = 1; /* Connect to hidden network */
	else
		pConnectionParameters->dwFlags = 0; /* adhoc networks must broadcast SSID */

	pConnectionParameters->strProfile = profile;
	connectStatus = WlanConnect(wl, &dev, pConnectionParameters, NULL);

	if (connectStatus != ERROR_SUCCESS) {
#ifdef VISTA_SERVER
		argv--;
		sprintf(*argv, "wl%d: Could not connect to network with error %d\n",
			inst.instance, connectStatus);
#else
		fprintf(stderr, "wl%d: Could not connect to network with error %d\n",
			inst.instance, connectStatus);
#endif
	}
	else {
		if (target->savePN > 0)
			WlanSetProfile(wl, &dev, 0, profile, NULL, 1, NULL, &addProfileStatus);
		else if (!target->savePN) {
			mbstowcs(profile_name, target->SSID, strlen(target->SSID));
			WlanDeleteProfile(wl, &dev, profile_name, NULL);
		}
	}
	/* to send response back to client */
#ifdef VISTA_SERVER
		argv--;
		sprintf(*argv,"%c",'\0');
#endif
	free(profile);
	free(target);
	free(pConnectionParameters);

	return (int) connectStatus;
}

/* Disassociates from the network using the ACM interface */
int
vista_disassoc(void *wl, cmd_t *cmd, char **argv) {
	DWORD dcStatus;
	dcStatus = WlanDisconnect(wl, &dev, 0);
	if (dcStatus != ERROR_SUCCESS)
		fprintf(stderr, "%s: Could not disconnect to network with error: \"%d\"\n",
			wlu_av0, dcStatus);
	/* to send response back to client */
#ifdef VISTA_SERVER
		sprintf(*argv,"%c",'\0');
#endif

	return (int) dcStatus;
}

int
vista_set_pmk(void *wl, cmd_t *cmd, char **argv) {
	wsec_pmk_t psk;
	size_t key_len;
	wl_instance_info_t inst;
	int buflen = WLC_IOCTL_MAXLEN, ret;
	strcpy(vistabuf,"set_pmk");

	if (wl_get(wl, WLC_GET_INSTANCE, &inst, sizeof(inst)) < 0)
		inst.instance = -1;

	if (!*++argv) {
		char str[WSEC_MAX_PSK_LEN];
		if ((ret = wl_get(wl, WLC_GET_VAR, &vistabuf[0], buflen)) < 0) {
#ifdef VISTA_SERVER
			argv--;
			sprintf(*argv, "wl%d: Error getting PMK from driver\n", inst.instance);
#else
			fprintf(stderr, "wl%d: Error getting PMK from driver\n", inst.instance);
#endif
			return ret;
		}

		memcpy(&psk, vistabuf, sizeof(psk));
		if (psk.key_len == 0) {
		/* to send response back to client */
#ifdef VISTA_SERVER
			argv--;
			sprintf(*argv, "wl%d: No PSK in driver\n", inst.instance);
#else
			printf("wl%d: No PSK in driver\n", inst.instance);
#endif
			return BCME_OK;
		}
		memcpy(str, psk.key, psk.key_len);
		str[psk.key_len] = 0;
		/* to send response back to client */
#ifdef VISTA_SERVER
		argv--;
		sprintf(*argv, "wl%d: PSK is %s\n", inst.instance, str);
#else
		printf("wl%d: PSK is %s\n", inst.instance, str);
#endif
		return BCME_OK;
	}

	key_len = strlen(*argv);
	if (key_len < WSEC_MIN_PSK_LEN || key_len > WSEC_MAX_PSK_LEN) {
#ifdef VISTA_SERVER
		argv--;
		sprintf(*argv, "Passphrase must be between %d and %d characters long\n",
			WSEC_MIN_PSK_LEN, WSEC_MAX_PSK_LEN);
#else
		fprintf(stderr, "Passphrase must be between %d and %d characters long\n",
		        WSEC_MIN_PSK_LEN, WSEC_MAX_PSK_LEN);
#endif
		return BCME_ERROR;
	}
	psk.key_len = (ushort) key_len;
	psk.flags = WSEC_PASSPHRASE;
	memcpy(psk.key, *argv, key_len);
	memcpy(vistabuf + strlen(vistabuf) + 1, &psk, sizeof(psk));
	/* to send response back to client */
#ifdef VISTA_SERVER
		argv--;
		sprintf(*argv,"%c",'\0');
#endif
	return wl_set(wl, WLC_SET_VAR, &vistabuf[0], buflen);
}


/* This command is deprecated in Vista. Do not let it communicate into the driver */
int
vista_sup_wpa(void *wl, cmd_t *cmd, char **argv) {
#ifdef VISTA_SERVER
		sprintf(*argv,"%c",'\0');
#endif
		return CMD_DEPRECATED;
}

int
vista_wsec(void *wl, cmd_t *cmd, char **argv) {
	int wsec, error;
	char *endptr;
	wl_instance_info_t inst;
	int buflen = WLC_IOCTL_MAXLEN;
	argv++;
	strcpy(vistabuf,"next_wsec");

	if (wl_get(wl, WLC_GET_INSTANCE, &inst, sizeof(inst)) < 0)
		inst.instance = -1;

	if (!*argv) {
		/* This is a GET */
		error = wl_get(wl, WLC_GET_VAR, &vistabuf[0], buflen);
		memcpy(&wsec, vistabuf, sizeof(wsec));
		if (wsec < 0) /* next_wsec wasn't set. Call the wsec ioctl and return it */
			error = wl_get(wl, WLC_GET_WSEC, &wsec, sizeof(wsec));
		/* to send response back to client */
#ifdef VISTA_SERVER
		argv = argv - 1;
		sprintf(*argv,"%d\n",wsec);
#else
		wl_printint(wsec);
#endif

		if (error < 0)
			return error;
	} else {
		/* This is a SET */
		if (!stricmp(*argv, "off"))
			wsec = 0;
		else {
			wsec = strtol(*argv, &endptr, 0);
			if (*endptr != '\0') {
#ifdef VISTA_SERVER
		argv--;
		sprintf(*argv,"%c",'\0');
#endif
				/* not all the value string was parsed by strtol */
				return BCME_ERROR;
			}
		}

#ifdef VISTA_SERVER
		argv--;
		sprintf(*argv,"%c",'\0');
#endif
		memcpy(vistabuf + strlen(vistabuf) + 1, &wsec, sizeof(int32));
		error = wl_set(wl, WLC_SET_VAR, &vistabuf[0], buflen);
		/* to send response back to client */
	}

	return error;
}
/* Tell vista that a MAC address has joined an ibss */
/* If no mac adddr is provided, then use a default */
int vista_legacy(void *wl, cmd_t *cmd, char **argv)
{
	wl_instance_info_t inst;
	struct ether_addr ea = { 0x72,0x6c,0x6f,0xf7,0x11,0xab };
	DWORD size = sizeof(WLAN_INTERFACE_STATE);
	int queryStatus, err;
	PWLAN_INTERFACE_STATE devState = WlanAllocateMemory(size);

	if (!devState)
		return BCME_ERROR;

	if (wl_get(wl, WLC_GET_INSTANCE, &inst, sizeof(inst)) < 0)
		inst.instance = -1;

	if (*++argv) {
		if (!wl_ether_atoe(*argv, &ea))
			return BCME_ERROR;
	}

	queryStatus = WlanQueryInterface(wl, &dev, wlan_intf_opcode_interface_state, NULL,
	                                 &size, &devState, NULL);

	if (queryStatus == ERROR_SUCCESS && *devState == wlan_interface_state_ad_hoc_network_formed)
		err = wl_set(wl, cmd->set, &ea, ETHER_ADDR_LEN);
	else {
#ifdef VISTA_SERVER
		argv--;
		sprintf(*argv, "wl%d: Use 'wl legacylink' after creating an IBSS network "
		        "with 'wl join' and with no IBSS peers\n", inst.instance);
#else
		fprintf(stderr, "wl%d: Use 'wl legacylink' after creating an IBSS network "
		        "with 'wl join' and with no IBSS peers\n", inst.instance);
#endif
		err = -1;
	}
	WlanFreeMemory(devState);
	return err;
}
/* Retrieve a list of all unique instance identifiers and mac
 * addresses for BRCM Wireless adapters.
 */
/* TODO: Currently epi_list operates only on Physical Wlan Adapters*/
int
epi_list(void *wl, cmd_t *cmd, char **argv)
{
	if(hDevHandle == INVALID_HANDLE_VALUE)
	{/* Make the legacy call */
		return epi_xlist_wlan(wl, cmd, argv, FALSE);
	}else {
		return epi_xlist_all(wl, cmd, argv, FALSE);
	}
}


/* Returns the cipher currently configured in the driver.
/* If a cipher is input by the wsec command, it is written into wl_info_t->next_wsec
 * and returned here for the next assocation. If next_wsec is null, return wlc->wsec.
 * WARNING: Changing wlc->wsec in Vista can cause
 * a race that will blow up and assert. Wsec can only be changed for the subsequent assocation.
 */

int
vista_cipher_select(void *wl, wl_instance_info_t* inst) {
	int wsec, err, buflen = WLC_IOCTL_MAXLEN, ret;

	strcpy(vistabuf,"next_wsec");

	if ((ret = wl_get(wl, WLC_GET_VAR, &vistabuf[0], buflen)) < 0) {
		fprintf(stderr, "wl%d: Error reading wsec\n", inst->instance);
			return ret;
	}
	memcpy(&wsec, vistabuf, sizeof(int32));

	if (wsec < 0) /* It wasn't previously set by the user */ {
		err = wl_get(wl, WLC_GET_WSEC, &wsec, sizeof(uint32));
		if (err < 0)
			return err;
	}

	/* At this point, we either have wl->next_wsec (iovar command) implying that
	 * wsec was called previously to this. Or, we grabbed it out of the driver
	 * via the IOCTL call. Either way....
	 */

	switch (wsec) {
		case WEP_ENABLED + TKIP_ENABLED + AES_ENABLED:
			return cipherType_aes;

		case WEP_ENABLED + TKIP_ENABLED:
			return cipherType_tkip;

		case WEP_ENABLED:
			return cipherType_wep;

		case 0:
			return cipherType_none;

		default:
			fprintf(stderr, "wl%d: Invalid wsec value in driver\n", inst->instance);
		return BCME_ERROR;
 	}
}
/* Build XML schema */
void
vista_build_schema(vista_bss_info_t* bss, LPWSTR profile) {

	char* tail = "</security></MSM></WLANProfile>\n";
	char tmpbuf[512];
	char* mbprofile = (char*)malloc(MAX_PROFILE_SIZE);

        if (mbprofile == NULL){
                printf("Memory allocation failed\n");
                return;
        }

	sprintf(mbprofile, "<?xml version=\"1.0\"?> "
		"<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">"
		"<name>%s</name><SSIDConfig><SSID><name>%s</name></SSID>"
	        "<nonBroadcast>%s</nonBroadcast></SSIDConfig><connectionType>%s</connectionType>"
	        "<connectionMode>%s</connectionMode><MSM><security>", bss->SSID,  bss->SSID,
	        (bss->connectionType == connectionType_infra) ? "true" : "false",
	        (bss->connectionType == connectionType_infra) ? "ESS" : "IBSS",
		(bss->connectionType == connectionType_infra) ? "auto" : "manual");

	sprintf(tmpbuf, "<authEncryption><authentication>%s</authentication>"
	        "<encryption>%s</encryption><useOneX>false</useOneX></authEncryption>",
	        (bss->authMode == authMode_open) ? "open" :
	        (bss->authMode == authMode_shared) ? "shared" :
	        (bss->authMode == authMode_wpapsk) ? "WPAPSK" :
	        (bss->authMode == authMode_wpa2psk) ? "WPA2PSK" : "Unknown",
	        (bss->cipher == cipherType_none) ? "none" :
	        (bss->cipher == cipherType_wep) ? "WEP" :
	        (bss->cipher == cipherType_tkip) ? "TKIP" :
	        (bss->cipher == cipherType_aes) ? "AES" : "unknown");

	strcat(mbprofile, tmpbuf);
	memset(tmpbuf, 0, strlen(tmpbuf));

	if (*(bss->key)) {
		sprintf(tmpbuf, "<sharedKey><keyType>networkKey</keyType><protected>false"
		        "</protected><keyMaterial>%s</keyMaterial></sharedKey>",
		        bss->key);
		strcat(mbprofile, tmpbuf);
	}

	else if (*(bss->passphrase)) {
		sprintf(tmpbuf, "<sharedKey><keyType>passPhrase</keyType><protected>false"
		        "</protected><keyMaterial>%s</keyMaterial></sharedKey>\n",
		        bss->passphrase);
		strcat(mbprofile, tmpbuf);
	}

	if (bss->keyIndex) {
		sprintf(tmpbuf,"<keyIndex>%d</keyIndex>", bss->keyIndex);
		strcat(mbprofile,tmpbuf);
		memset(tmpbuf, 0, strlen(tmpbuf));
	}
	strcat(mbprofile, tail);

	/* Convert profile from wchar to char, copy from mbprofile */
	/* And convert back to wchar */

	mbstowcs(profile, mbprofile, strlen(mbprofile));
	free(mbprofile);
}

int
vista_parse_and_validate(void *wl, vista_bss_info_t* target, char** argv, wchar_t* profile,
                         wl_instance_info_t *inst) {

	uint idx, key = -1, cipher = -1;
	int wsec, err, buflen = WLC_IOCTL_MAXLEN;

	/* verify that SSID was specified and is a valid length */
	if (!*argv || (strlen(*argv) > DOT11_MAX_SSID_LEN))
		return BCME_ERROR;

	strcpy(target->SSID, *argv);

	/* Unless specified otherwise, make the following assumptions */
	target->authMode = authMode_open;
	target->connectionType = connectionType_infra;
	target->savePN = -1; /* Don't delete PN if a behavior isn't specified */

	while (*++argv) {
		/* specified wep key */
		if (!stricmp(*argv, "wepkey") || !stricmp(*argv, "wep") ||
			!stricmp(*argv, "key")) {

			if (!*++argv)
				return BCME_ERROR;


			/* WEP index specified */
			else if (*(argv[0]+1) == ':') {
				idx = *argv[0] - 0x30;
				if (idx < 0 || idx > 3) {
					fprintf(stderr, "wl%d: Invalid key index %d specified\n",
					        inst->instance, idx);
					return BCME_ERROR;
				}
				target->keyIndex = idx;
				strcpy(target->key, *argv+2);
			} else {
				target->keyIndex = 0;
				strcpy(target->key, *argv);
			}
		}

		/* specify to save PN to Vista UI */
		if (!stricmp(*argv, "savepn")) {
			if (!*++argv)
				return BCME_ERROR;

			if (!strcmp(*argv, "1") || !stricmp(*argv, "true"))
				target->savePN = 1;
			else if (!strcmp(*argv, "0") || !stricmp(*argv, "false"))
				target->savePN = 0;
			else {
				fprintf(stderr, "wl%d: Invalid argument for 'savePN'\n",
				        inst->instance);
			}
		}

		/* specified infrastructure mode */
		else if (!stricmp(*argv, "imode") || !stricmp(*argv, "infra") ||
		         !stricmp(*argv, "mode")) {
			if (!*++argv) {
				fprintf(stderr, "wl%d: expected argument after \"infra\" keyword "
				        "but command line ended.\n", inst->instance);
				return BCME_ERROR;
			}
			if (!stricmp(*argv, "ibss") || !stricmp(*argv, "adhoc") ||
			    !stricmp(*argv, "ad-hoc"))
				target->connectionType = connectionType_ibss;
			else if (!stricmp(*argv, "bss") || !stricmp(*argv, "managed") ||
			         !stricmp(*argv, "auto") || !strnicmp(*argv, "infra", 5))
				target->connectionType = connectionType_infra;
			else {
				fprintf(stderr, "wl%d: unrecongnized parameter \"%s\" after "
				        "\"infra\" keyword\n", inst->instance, *argv);
				return BCME_ERROR;
			}
		}

		/* specified authentication mode */
		else if (!stricmp(*argv, "amode") || !strnicmp(*argv, "auth", 4)) {
			if (!*++argv)
				return BCME_ERROR;
			else if (!stricmp(*argv, "wpa2") || !stricmp(*argv, "wpa")) {
				fprintf(stderr, "wl%d: WPA and WPA2 Enterprise not supported\n",
				        inst->instance);
				return BCME_ERROR;
			}
			if (!stricmp(*argv, "open"))
				target->authMode = authMode_open;
			else if (!stricmp(*argv, "shared"))
				target->authMode = authMode_shared;
			else if (!stricmp(*argv, "wpapsk"))
				target->authMode = authMode_wpapsk;
			else if (!stricmp(*argv, "wpa2psk"))
				target->authMode = authMode_wpa2psk;
			else {
				fprintf(stderr, "wl%d: Ambiguous argument for auth parameter\n",
				        inst->instance);
				return BCME_ERROR;
			}
		}

		/* Passing optional Passphrase via command line */
		if (!stricmp(*argv, "psk")) {
			int key_len;
			if (!*++argv) {
				fprintf(stderr, "wl%d: Passphrase indicated but no "
				        "passphrase specified\n", inst->instance);
				return BCME_ERROR;
			}

			key_len  = strlen(*argv);
			if (key_len < WSEC_MIN_PSK_LEN || key_len > WSEC_MAX_PSK_LEN) {
				fprintf(stderr, "passphrase must be between %d and %d "
				        "characters long\n", WSEC_MIN_PSK_LEN, WSEC_MAX_PSK_LEN);
				return BCME_ERROR;
			}
			strcpy(target->passphrase, *argv);
		}
	}

	/* Validate parameters */

	if (target->cipher < 0) {
		fprintf(stderr, "wl%d: Invalid cipher detected in driver.\n", inst->instance);
		return BCME_ERROR;
	}

	if ((target->authMode == authMode_wpa2psk || target->authMode == authMode_wpapsk) &&
	    *(target->key) > 0) {
		fprintf(stderr, "wl%d: Can't specify a WEP key with WPA-Personal authentication "
		        "type. Use parameter 'PSK'\n", inst->instance);
		return BCME_ERROR;
	}

	if ((target->authMode == authMode_open || target->authMode == authMode_shared)
	    && *(target->passphrase) > 0) {
		fprintf(stderr, "wl%d: Can't specify a PSK key with open/shared authentication "
		        "type. Use parameter 'key'\n", inst->instance);
		return BCME_ERROR;
	}

	if (target->authMode ==  authMode_wpapsk || target->authMode == authMode_wpa2psk) {
		if (!(target->cipher == cipherType_aes || target->cipher == cipherType_tkip)) {
			fprintf(stderr, "wl%d: Cipher must be AES or TKIP to use PSK\n",
			        inst->instance);
			return BCME_ERROR;
		}
	}

	if (target->authMode == authMode_shared) {
		if (target->cipher != cipherType_wep) {
			fprintf(stderr, "wl%d: Cipher must be WEP for 'shared' auth\n",
			        inst->instance);
			return BCME_ERROR;
		}
	}

	if (target->authMode == authMode_open) {
		if (!(target->cipher == cipherType_wep || target->cipher == cipherType_none)) {
			fprintf(stderr, "wl%d: Cipher must be disabled or WEP for 'open' auth\n",
			        inst->instance);
			return BCME_ERROR;
		}
	}

	if (target->connectionType == connectionType_ibss) {
		if (!(target->authMode == authMode_wpa2psk || target->authMode == authMode_open)) {
			fprintf(stderr, "wl%d: Authentication mode for IBSS must be open or "
			        "WPA2PSK\n", inst->instance);
			return BCME_ERROR;
		}
		if (target->cipher == cipherType_tkip) {
			fprintf(stderr, "wl%d: Cipher type TKIP not valid for IBSS\n",
			        inst->instance);
			return BCME_ERROR;
		}

		if (target->keyIndex) {
			fprintf(stderr, "wl%d: Vista only supports key index 0 for IBSS\n",
			        inst->instance);
			return BCME_ERROR;
		}
	}

	/* If wsec is WEP but no key is specified, pull key from driver */
	/* Send to Vista only the Primary key  */
	if ((target->cipher == cipherType_wep && !(*(target->key)))) {
		int i = 0, val, found = 0;
		union {
			int index;
			wl_wsec_key_t key;
		} u;

		do {
			val = i;
			if (wl_get(wl, WLC_GET_KEY_PRIMARY, &val, sizeof(val)) < 0) {
				return -1;
			}
			if (val) {
				found = 1;
				break;
			}
		} while (++i < DOT11_MAX_DEFAULT_KEYS);

		if (!found) {
			fprintf(stderr, "wl%d: No primary key transmit keys found\n",
			        inst->instance);
			return BCME_ERROR;
		}

		u.index = i;
		if (wl_get(wl, WLC_GET_KEY, &u, sizeof(u)) < 0)
			return BCME_ERROR;

		switch (u.key.algo) {
		case CRYPTO_ALGO_TKIP:
		case CRYPTO_ALGO_AES_CCM:
		case CRYPTO_ALGO_AES_OCB_MSDU:
		case CRYPTO_ALGO_AES_OCB_MPDU:
			fprintf(stderr, "wl%d: Cannot plumb keys other than WEP40, WEP128 for "
			        "Vista\n", inst->instance);
			return BCME_ERROR;
		}

		memcpy(target->key, u.key.data, u.key.len);
		target->keyIndex = u.index;
	}

	/* If wsec is TKIP or AES but no passphrase, was specified, pull psk from driver */
	if ((target->cipher == cipherType_aes || target->cipher == cipherType_tkip) &&
	    !(*(target->passphrase))) {
		    int ret;
		    wsec_pmk_t psk;
		    strcpy(vistabuf,"set_pmk");

		if ((ret = wl_get(wl, WLC_GET_VAR, &vistabuf[0], buflen)) < 0) {
			fprintf(stderr, "wl%d: Error getting PMK from driver\n", inst->instance);
			return ret;
		}
		memcpy(&psk, vistabuf, sizeof(wsec_pmk_t));
		memcpy(&target->passphrase, &psk.key, psk.key_len);
	}

	/* Cipher specified must match driver. First check the next_wsec iovar, if set,
	 * then check the ioctl.
	 */
	memset(vistabuf, 0,  WLC_IOCTL_MAXLEN);
	strcpy(vistabuf,"next_wsec");
	err = wl_get(wl, WLC_GET_VAR, &vistabuf[0], buflen);
	memcpy(&wsec, vistabuf, sizeof(wsec));
	if (err < 0)
		return err;

	if (wsec < 0) {
		err = wl_get(wl, WLC_GET_WSEC, &wsec, sizeof(uint32));
		if (err < 0)
			return err;
	}

	err = 0;
	switch (wsec) {
	case WEP_ENABLED + TKIP_ENABLED + AES_ENABLED:
	case WEP_ENABLED + TKIP_ENABLED:
		if (*(target->key) != 0)
			err = 1;
		break;
	case WEP_ENABLED:
		if (*(target->passphrase) != 0)
			err = 1;
		break;
	case 0:
		if ((*(target->key) != 0) || (*(target->passphrase) != 0))
		err = 1;
	}

	if (err) {
		fprintf(stderr, "wl%d: Driver/parameter cipher mismatch\n", inst->instance);
		return BCME_ERROR;
	}

	/* Now that parameters are validated, generate the XML *profile */
	vista_build_schema(target, profile);
	return 0;
}
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
			if (wl_check((void *)irh) >= 0)
				return ERROR_SUCCESS;
		}

		if (i == ndevs) {
			fprintf(stderr, "%s: No BRCM wireless adapters were found\n", wlu_av0);
			return ERROR_INVALID_HANDLE;
		}
	}

	status = ERROR_SUCCESS;
	if (adapter < 0 || (ULONG) adapter >= ndevs) {
		fprintf(stderr, "%s: Cannot find wireless adapter #%d\n", wlu_av0, adapter);
		status = ERROR_INVALID_HANDLE;
	} else {
		dev = devlist[adapter];
		if (wl_check((void *)irh) < 0) {
			fprintf(stderr, "%s: Selected adapter #%d is not an BRCM \
				wireless adapter\n", wlu_av0, adapter);
			status = ERROR_INVALID_HANDLE;
		}
	}

	return status;
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
		fprintf(stderr, "%s: Error 0x%x: %s -- %s\n", wlu_av0, status, prefix, ErrStr);
		LocalFree(ErrStr);
	} else
		fprintf(stderr, "%s: Error 0x%x: %s -- Unknown error\n", wlu_av0, status, prefix);
}

void
PrintAdapter(PIP_ADAPTER_ADDRESSES pInfo, int inst, int index, BOOL xlist)
{

	int i;

	printf("%4d:",index);

        if((pInfo->IfType == IF_TYPE_IEEE80211)&&
                (!wcsstr(pInfo->Description, L"Virtual"))){
                printf("\twl%-2d",inst);
        }else if( (pInfo->IfType == IF_TYPE_ETHERNET_CSMACD) &&
                wcsstr(pInfo->Description, L"Broadcom") &&
                wcsstr(pInfo->Description, L"Virtual")){
                printf("\tvwl%-2d",inst);
        }else if( (pInfo->IfType == IF_TYPE_IEEE80211) &&
                wcsstr(pInfo->Description, L"Microsoft") &&
                wcsstr(pInfo->Description, L"Virtual")){
                printf("\tmsvwl%-2d",inst);
        }else {
                printf("\t????");
        }

	if(xlist) {
		switch (pInfo->IfType) {
		case IF_TYPE_OTHER:
			printf("\tOther");
			break;
		case IF_TYPE_ETHERNET_CSMACD:
			printf("\tEthernet");
			break;
		case IF_TYPE_IEEE80211:
			printf("\tWireless");
			break;
		case IF_TYPE_SOFTWARE_LOOPBACK:
			printf("\tLookback");
			break;
		default:
			printf("\tUnknown");
			break;
		}

		printf("\t%s", pInfo->AdapterName);
	}
	printf("\tMAC: ");
        for (i = 0; i < (int)pInfo->PhysicalAddressLength; i++) {
                if (i == (pInfo->PhysicalAddressLength - 1))
                        printf("%.2X", (int) pInfo->PhysicalAddress[i]);
		else
			printf("%.2X:", (int) pInfo->PhysicalAddress[i]);
	}
	printf("\t%ws\n", pInfo->Description);

}

int
epi_xlist_all(void *wl, cmd_t *cmd, char **argv, BOOL xlist)
{
    BOOL brcm = FALSE;
	WINERR  dwStatus;
    PIP_ADAPTER_ADDRESSES pAdapterInfo = NULL;
    PIP_ADAPTER_ADDRESSES pInfo = NULL;
    ULONG uNeedlen = 0, index = 0, Iterations = 0;
	wl_instance_info_t inst;

        while (((dwStatus = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAdapterInfo, &uNeedlen)) == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES)){
                if (pAdapterInfo){
                        free(pAdapterInfo);
                        pAdapterInfo = NULL;
                }
                if ((pAdapterInfo = (PIP_ADAPTER_ADDRESSES) malloc(uNeedlen)) == NULL){
                        printf("Memory allocation failed for IP_ADAPTER_ADDRESS struct\n");
                        return BCME_NOMEM;
                }
                memset(pAdapterInfo, 0, uNeedlen);
                Iterations++;
        }
        if (dwStatus == ERROR_SUCCESS)
        {
	    pInfo = pAdapterInfo;
	    for (pInfo = pAdapterInfo; pInfo; pInfo = pInfo->Next)
		{
			Unbind(); /* Unselect The Previous Adapter */
			/* Select this Adapter */
			dwStatus  = Bind(pInfo->AdapterName);
			inst.instance = -1;
			if (dwStatus == ERROR_SUCCESS){

				/* Verify if this is a Broadcom Adapter */
				brcm = IsBroadcomAdapter();

				if(brcm) {
					/* Try to connect Physical Adapter to Virtual Adapter */
					/* This is requirede to get the instance below */
					if(pInfo->IfType == IF_TYPE_ETHERNET_CSMACD)
					ConnectToVirtualAdapter(wl);

					if (wl_get(wl, WLC_GET_INSTANCE, &inst, sizeof(inst)) < 0)
						inst.instance = -1;
				}
			}

			if((!xlist) && (!brcm)) continue;

			PrintAdapter(pInfo, inst.instance, index++, xlist);
		}
		free(pAdapterInfo);
    }
	return BCME_OK;
}

int
epi_xlist(void *wl, cmd_t *cmd, char **argv)
{
	if(hDevHandle == INVALID_HANDLE_VALUE)
	{/* Make the legacy call */
		return epi_xlist_wlan(wl, cmd, argv, TRUE);
	}else {
		return epi_xlist_all(wl, cmd, argv, TRUE);
	}

}


#define RELAY_BASE		"BCM42RLY"
#define RELAY_FILE		"\\\\.\\BCM42RLY"


DWORD InstallDriver(SC_HANDLE SchSCManager, LPCTSTR lpszDriverName, LPCTSTR lpszServiceExe)
{
    SC_HANDLE schService;
    WINERR dwStatus = ERROR_SUCCESS;

    schService = CreateService (SchSCManager,          // SCManager database
                                lpszDriverName,        // name of service
                                lpszDriverName,        // name to display
                                SERVICE_ALL_ACCESS,    // desired access
                                SERVICE_KERNEL_DRIVER, // service type
                                SERVICE_DEMAND_START,  // start type
                                SERVICE_ERROR_NORMAL,  // error control type
                                lpszServiceExe,        // service's binary
                                NULL,                  // no load ordering group
                                NULL,                  // no tag identifier
                                NULL,                  // no dependencies
                                NULL,                  // LocalSystem account
                                NULL                   // no password
                                );

    if (schService == NULL){
        dwStatus = GetLastError();
		if(dwStatus == ERROR_SERVICE_EXISTS){
			dwStatus = BCME_OK;
		}else {
//			printf("%s: CreateService failed with %d\n",__FUNCTION__,dwStatus);
		}
    }
    return dwStatus;
}

DWORD RemoveDriver(SC_HANDLE  SchSCManager, LPCTSTR lpszDriverName)
{
    SC_HANDLE schService;
    DWORD dwError = 0;
    BOOL bReturn;

    schService = OpenService (SchSCManager, lpszDriverName, SERVICE_ALL_ACCESS);
    if (schService != NULL)
	{
//		printf("%s:OpenService failed\n", __FUNCTION__);

		bReturn = DeleteService (schService);
		if (bReturn == 0)
			dwError = GetLastError();

		CloseServiceHandle (schService);
    }

    return dwError;
}

DWORD StartDriver(SC_HANDLE SchSCManager, LPCTSTR lpszDriverName)
{
    SC_HANDLE schService;
    BOOL bReturn;
    DWORD dwError = 0;

    schService = OpenService (SchSCManager, lpszDriverName, SERVICE_ALL_ACCESS);
    if (schService == NULL){
		dwError = GetLastError();
//		printf("%s:OpenService failed with 0x%x\n", __FUNCTION__,dwError);
    }
	else
	{
		bReturn = StartService (schService, 0, NULL);
		if (bReturn == FALSE) {
			dwError = GetLastError();
//			printf("%s:StartService failed with 0x%x\n", __FUNCTION__,dwError);
		}

		CloseServiceHandle (schService);
    }

    return dwError;
}

DWORD StopDriver(SC_HANDLE SchSCManager, LPCTSTR lpszDriverName)
{
    SC_HANDLE schService;
    BOOL bReturn;
    DWORD dwError = 0;
    SERVICE_STATUS serviceStatus;

    schService = OpenService(SchSCManager, lpszDriverName, SERVICE_ALL_ACCESS);
    if (schService == NULL)
		dwError = GetLastError();
	else
	{
		bReturn = ControlService(schService, SERVICE_CONTROL_STOP, &serviceStatus);
		if (bReturn == FALSE)
		{
			dwError = GetLastError();
			printf("%s:Unable to stop driver error (0x%02x)\n",__FUNCTION__, dwError);
		}

		CloseServiceHandle(schService);
    }

    return dwError;
}

DWORD LoadAndOpen(LPCTSTR lpszDriverName, LPCTSTR lpszServiceExe)
{
    SC_HANDLE schSCManager;
    DWORD dwError = ERROR_SUCCESS;

    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL) {
//		printf("%s failed\n", __FUNCTION__);
		dwError = GetLastError();
    }
	else
	{
		InstallDriver(schSCManager, lpszDriverName, lpszServiceExe);
		StartDriver(schSCManager, lpszDriverName);
		CloseServiceHandle(schSCManager);
    }

	return dwError;
}


BOOL LoadRelayDriver(LPCTSTR lpszDriverService)
{
    DWORD dwStatus;
    TCHAR cDriverExe[256];

	sprintf(cDriverExe, "system32\\drivers\\%s.sys", lpszDriverService);
	dwStatus = LoadAndOpen(lpszDriverService, cDriverExe);
	switch (dwStatus)
	{
		case ERROR_SUCCESS:
		/* Fall-Through */
		case ERROR_SERVICE_EXISTS:
		return TRUE;

		default:
		return FALSE;
	}

	return FALSE;
}



DWORD Initialize()
{
	WINERR err = BCME_OK;

	hDevHandle = CreateFile(RELAY_FILE, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hDevHandle == INVALID_HANDLE_VALUE) {
		if (LoadRelayDriver(RELAY_BASE))
			hDevHandle = CreateFile(RELAY_FILE, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	}

	if (hDevHandle == INVALID_HANDLE_VALUE) {
	    err = GetLastError();
//		printf("%s:CreateFile failed for %s with error %d\n",__FUNCTION__, RELAY_FILE, err);
	}
	return err;
}

DWORD Uninitialize()
{
    WINERR dwStatus = NO_ERROR;
    if (hDevHandle !=  INVALID_HANDLE_VALUE && !CloseHandle(hDevHandle))
		dwStatus = GetLastError();

    return dwStatus;
}

DWORD SendIoctl(DWORD dwIoctl, PCHAR pBuf, PDWORD pdwLen, DWORD dwMillis)
{
    DWORD dwStatus = NO_ERROR, dwWait;
    int iOk;
    OVERLAPPED ovlp = { 0, 0, 0, 0, 0 };
    HANDLE hEvent = 0;

    hEvent = CreateEvent(0, TRUE, 0, NULL);
    if (hEvent == NULL)
        return GetLastError();

	ovlp.hEvent = hEvent;
    iOk = DeviceIoControl(hDevHandle, dwIoctl, pBuf, *pdwLen, pBuf, *pdwLen, pdwLen, &ovlp);
    if (!iOk)
	{
		dwStatus = GetLastError();
		if (dwStatus == ERROR_IO_PENDING)
		{
			dwWait = WaitForSingleObject(hEvent, dwMillis);
			switch (dwWait)
			{
				case WAIT_OBJECT_0:
				if (!GetOverlappedResult(hDevHandle, &ovlp, pdwLen, TRUE))
					dwStatus = GetLastError();
				else
				{
					if (ovlp.Internal != 0)
						dwStatus = ovlp.Internal;
					else
						dwStatus = ERROR_SUCCESS;
				}
				break;

				case WAIT_FAILED:
				dwStatus = GetLastError();
				break;

				case WAIT_TIMEOUT:
				*pdwLen = 0;
				dwStatus = ERROR_TIMEOUT;
				break;

				default:
				printf("Received unexpected status from WaitForSingleObject = 0x%x", dwWait);
				dwStatus = ERROR_INVALID_FUNCTION;
				break;
			}
		}
    }

    CloseHandle(hEvent);
    return dwStatus;
}

DWORD Bind(LPCTSTR lpszDeviceName)
{
    DWORD dwStatus = ERROR_NOT_SUPPORTED, dwBrlen;
    CHAR  BindRequest[80];

	// convert to an ascii string
	memset(BindRequest, 0, sizeof(BindRequest));
	strncpy(BindRequest, lpszDeviceName, sizeof(BindRequest)-1 );
	dwBrlen = sizeof(BindRequest);
	dwStatus = SendIoctl(IOCTL_BIND, (PCHAR)BindRequest, &dwBrlen, INFINITE);
	if(dwStatus){
//		printf("%s: Bind failed for %s with error %d",__FUNCTION__, BindRequest, dwStatus);
	}
    return dwStatus;
}

DWORD Unbind()
{
    DWORD dwLen = 0, dwStatus = ERROR_NOT_SUPPORTED;
	dwStatus = SendIoctl(IOCTL_UNBIND, NULL, &dwLen, INFINITE);
    return dwStatus;
}

DWORD QueryInformation(ULONG uOid, PUCHAR pInbuf, PDWORD dwInlen)
{
	DWORD dwStatus = ERROR_NOT_SUPPORTED;
	PIRELAY pr;


	pr = (PIRELAY) malloc(sizeof(IRELAY) + *dwInlen);
        if (pr == NULL)
                return BCME_NOMEM;
	memset(pr, 0, sizeof(IRELAY) + *dwInlen);
	pr->rh.OID = uOid;
	pr->rh.IsQuery = TRUE;
	pr->rh.Status = ERROR_SUCCESS;

	memcpy(pr->GenOid.Buffer, pInbuf, *dwInlen);
	pr->rh.BufferLength = *dwInlen;

	dwStatus = CallIRelay(hDevHandle, (PRelayHeader)pr);
	if (dwStatus == ERROR_SUCCESS)
	{
		*dwInlen = pr->rh.BufferLength;
		memcpy(pInbuf, pr->Buffer, *dwInlen);
	}
	else
	{
		if (dwStatus == ERROR_MORE_DATA)
			*dwInlen = pr->rh.BufferLength;
	}

	free(pr);

	return dwStatus;
}

DWORD SetInformation(ULONG uOid, PUCHAR pInbuf, PDWORD dwInlen)
{
	DWORD dwStatus = ERROR_NOT_SUPPORTED;
	PIRELAY pr;

	pr = (PIRELAY) malloc(sizeof(IRELAY) + *dwInlen);
        if (pr == NULL)
                return BCME_NOMEM;
	memset(pr, 0, sizeof(IRELAY) + *dwInlen);
	pr->rh.OID = uOid;
	pr->rh.IsQuery = FALSE;
	pr->rh.Status = ERROR_SUCCESS;

	memcpy(pr->GenOid.Buffer, pInbuf, *dwInlen);
	pr->rh.BufferLength = *dwInlen;

	dwStatus = CallIRelay(hDevHandle, (PRelayHeader)pr);

	free(pr);

	return dwStatus;
}

BOOL IsBroadcomAdapter()
{
	DWORD dwVal = 0;
	DWORD dwLen;

	BOOL ret = FALSE;
	dwLen = sizeof(dwVal);
	if (QueryInformation(WL_OID_BASE + WLC_GET_MAGIC, (PUCHAR) &dwVal, &dwLen) == ERROR_SUCCESS)
		ret = (dwVal == WLC_IOCTL_MAGIC);

	if( ret ) {
		dwLen = sizeof(dwVal);
		if (QueryInformation(WL_OID_BASE + WLC_GET_VERSION, (PUCHAR) &dwVal, &dwLen) == ERROR_SUCCESS)
			ret = (dwVal == WLC_IOCTL_VERSION);
	}
	return ret;
}

DWORD ConnectToVirtualAdapter(HANDLE wl)
{
	HANDLE tempHandle;
	DWORD err;

	/* Connect Physical Adapter to the Virtual driver */
	strcpy(vistabuf,"find_vif");

	tempHandle = hDevHandle;
	hDevHandle = INVALID_HANDLE_VALUE; // Make sure only physical adapter is used

	err = wl_get(wl, WLC_GET_VAR, &vistabuf[0], WLC_IOCTL_MAXLEN);

	hDevHandle = tempHandle;

	return err;
}

DWORD CallIRelay(HANDLE hDevice, PRelayHeader prh)
{
    OVERLAPPED  ovlp = { 0, 0, 0, 0, 0 };
    HANDLE hEvent = 0;
	DWORD dwIosize, dwStatus = ERROR_SUCCESS, dwCount;

    hEvent = CreateEvent(0, TRUE, 0, NULL);
    if (hEvent == NULL)
        return 0;

	ovlp.hEvent = hEvent;
    dwIosize = sizeof(RelayHeader) + prh->BufferLength;

	dwCount = 0;
	SetLastError(NO_ERROR);
	if (!DeviceIoControl(hDevice, IOCTL_OID_RELAY, (PVOID)prh, dwIosize, (PVOID)prh, dwIosize, &dwCount, &ovlp))
	{
		dwStatus = GetLastError();
		if (dwStatus == ERROR_IO_PENDING)
		{
			DWORD dwWait = WaitForSingleObject(hEvent, INFINITE);
			switch (dwWait)
			{
				case WAIT_OBJECT_0:
				if (!GetOverlappedResult(hDevice, &ovlp, &dwCount, TRUE))
					dwStatus = GetLastError();
				else
				{
					if (ovlp.Internal != 0)
						dwStatus = ovlp.Internal;
					else
						dwStatus = ERROR_SUCCESS;
				}
				break;

				case WAIT_FAILED:
				dwStatus = GetLastError();
				break;

				case WAIT_TIMEOUT:
				dwCount = 0;
				dwStatus = ERROR_TIMEOUT;
				break;

				default:
				printf("Received unexpected status from WaitForSingleObject = 0x%x", dwWait);
				dwStatus = ERROR_INVALID_FUNCTION;
				break;
			}
		}

		prh->BufferLength = dwCount - sizeof(RelayHeader);
	}

	CloseHandle(hEvent);

	return dwStatus;
}


#define SELECT_PARAM_AUTO  1
#define SELECT_PARAM_INDEX 2
#define SELECT_PARAM_NAME  4
#define SELECT_PARAM_ADDR  8


/* select the virtual wlan adapter */
DWORD SelectBrcmAdapter(HANDLE irh, char *adapter, int flags)
{
	WINERR  dwStatus;
    PIP_ADAPTER_ADDRESSES pAdapterInfo = NULL;
    PIP_ADAPTER_ADDRESSES pInfo = NULL;

    ULONG uNeedlen = 0;
	INT index = 0, Iterations = 0;

	/* Input can be in below formats
		wl -i 0 ver			//Index of xlist
		wl -i wl0 ver		//interface with inst
		wl -i vwl0 ver		//BRCM virtual
		wl -i msvwl0 ver 	//MSFT Virtual
	*/
        while (((dwStatus = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAdapterInfo, &uNeedlen)) == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES)){
                if (pAdapterInfo){
                        free(pAdapterInfo);
                        pAdapterInfo = NULL;
                }
                if ((pAdapterInfo = (PIP_ADAPTER_ADDRESSES) malloc(uNeedlen)) == NULL){
                        printf("Memory allocation failed for IP_ADAPTER_ADDRESS struct\n");
                        return BCME_NOMEM;
                }
                memset(pAdapterInfo, 0, uNeedlen);
                Iterations++;
        }
        if (dwStatus == ERROR_SUCCESS)
        {
	    pInfo = pAdapterInfo;
	    for (pInfo = pAdapterInfo; pInfo; pInfo = pInfo->Next, index++)
		{
			BOOL tryit = FALSE;
			char * ininst = NULL;
			dwStatus = BCME_BADARG;

			/* Pre - selection test */
			switch(flags)
			{
			case SELECT_PARAM_AUTO :
				if (((pInfo->IfType == IF_TYPE_IEEE80211) ||
					(pInfo->IfType == IF_TYPE_ETHERNET_CSMACD)) &&
                                (!wcsstr(pInfo->Description, L"Virtual")) ){
					tryit = TRUE;
				}
				break;
			case SELECT_PARAM_INDEX :
					if( index == atoi(adapter)) {
					tryit = TRUE;
				}
				break;
			case SELECT_PARAM_ADDR :
				{
					struct ether_addr ifaddr;
					if(wl_ether_atoe(adapter, &ifaddr) &&
                                                (memcmp((void *)pInfo->PhysicalAddress, (void *)&ifaddr, 6) == 0))
						tryit = TRUE;
				}
				break;
			case SELECT_PARAM_NAME :
				if( (!strncmp(adapter,"wl",strlen("wl"))) &&
                                        (pInfo->IfType == IF_TYPE_IEEE80211) &&
                                        (!wcsstr(pInfo->Description, L"Virtual"))){
					tryit = TRUE;
					ininst = adapter +strlen("wl");
				}else if((!strncmp(adapter,"vwl",strlen("vwl"))) &&
                                        (pInfo->IfType == IF_TYPE_ETHERNET_CSMACD) &&
                                        wcsstr(pInfo->Description, L"Broadcom") &&
                                        wcsstr(pInfo->Description, L"Virtual")){
					tryit = TRUE;
					ininst = adapter +strlen("vwl");
				}else if((!strncmp(adapter,"msvwl",strlen("msvwl")))&&
                                        (pInfo->IfType == IF_TYPE_IEEE80211) &&
                                        wcsstr(pInfo->Description, L"Microsoft") &&
                                        wcsstr(pInfo->Description, L"Virtual")){
					tryit = TRUE;
					ininst = adapter + strlen("msvwl");
				}
				break;
			}

			if(tryit == FALSE) continue;

			/* De-select the previously selected adapter */
			Unbind();

			/* Bind means the selection of the adapter */
			if (Bind(pInfo->AdapterName) != ERROR_SUCCESS) continue;

			/* Verify if this is a Broadcom Adapter */
			if( IsBroadcomAdapter() != TRUE) continue;


			/* For brcm virtual adapter do some special operation */
			if(pInfo->IfType == IF_TYPE_ETHERNET_CSMACD) {
				/* Try to connect Physical Adapter to Virtual Adapter */
				/* This is requirede to get the instance below */
				ConnectToVirtualAdapter(irh);

				/* Remove the override of vista wlan commands */
				memset(&vista_cmds[2], 0, sizeof(vista_cmds[2]));
			}

			/* Adapter Looks good so far */
			dwStatus = BCME_OK;

			/* Post Selection Test */
			switch(flags)
			{
				case SELECT_PARAM_AUTO :
					break;

				case SELECT_PARAM_NAME :
				{
					wl_instance_info_t inst;
					DWORD dwLen= sizeof(inst);

					inst.instance = -1;

					if(ininst) {
						if (QueryInformation(WL_OID_BASE + WLC_GET_INSTANCE, (PUCHAR) &inst, &dwLen)
											!= ERROR_SUCCESS){
							inst.instance = -1;
						}
						if(atoi(ininst) != inst.instance){
							dwStatus = BCME_BADARG;
						}
					}
					break;
				}// caser SELECT_PARAM_NAME
			}// switch
			if( dwStatus == BCME_OK) {
				break; // Adapter Found
			}
		}//for
		if(pAdapterInfo)
			free(pAdapterInfo);
    }//if (dwStatus == ERROR_BUFFER_OVERFLOW)
	return dwStatus;
}

/* select the wlan adapter using ACM */
WINERR
SelectBrcmAdapterLegacy(HANDLE irh, char *adapter, int flags)
{
	WINERR status = BCME_ERROR;
	char name[256];
	do {
		DWORD i;
		WLAN_INTERFACE_INFO_LIST *iflist = NULL;

		status = WlanEnumInterfaces(irh, NULL, &iflist);
		if (status != ERROR_SUCCESS)
			break;

		for (i = 0; i < iflist->dwNumberOfItems; i++) {
			BOOL tryit = FALSE;
			char * ininst = NULL;
			status = BCME_BADARG;

			/* Convert from wide string to narrow string */
			sprintf(name, "%ws", iflist->InterfaceInfo[i].strInterfaceDescription);

			/* Pre - selection test */
			switch(flags)
			{
			case SELECT_PARAM_AUTO:
				tryit = TRUE;
				break;
			case SELECT_PARAM_INDEX :
				if(i == atoi(adapter)) {
					tryit = TRUE;
				}
				break;

			case SELECT_PARAM_ADDR :
				tryit = TRUE;
				break;

			case SELECT_PARAM_NAME:
				if(!strncmp((char *)adapter,"wl",strlen("wl"))){
					tryit = TRUE;
					ininst = adapter +strlen("wl");
				}
				break;
			}

			if(tryit == FALSE) continue;
			dev = iflist->InterfaceInfo[i].InterfaceGuid;
			if (wl_check(irh) != 0) continue;

			/* Adapter Looks good so far */
			status = BCME_OK;


			/* Post Selection Test */
			switch(flags)
			{
				case SELECT_PARAM_NAME :
				{
					wl_instance_info_t inst;

					if (wl_get(irh, WLC_GET_INSTANCE, &inst, sizeof(inst)) < 0)
						inst.instance = -1;

					if(atoi(ininst) != inst.instance){
						status = BCME_BADARG;
					}
					break;
				}
				case SELECT_PARAM_ADDR :
				{
					struct ether_addr ifaddr;
					char buf[16];

					if(!wl_ether_atoe(adapter, &ifaddr)) {
						status = BCME_BADARG;
						break;
					}

					strcpy(buf, "cur_etheraddr");
					if (wl_get(irh, WLC_GET_VAR, buf, sizeof(buf)) < 0){
						status = BCME_ERROR;
					} else {
						if (memcmp(&ifaddr, buf, 6) != 0) {
							status = BCME_ERROR;
						}
					}
					break;
				}
			}
			if(status == BCME_OK)break; // Adapter Found
		}
		if (i == iflist->dwNumberOfItems) {
			status = BCME_BADARG;
		}

		WlanFreeMemory(iflist);
	} while (0);

	return status;
}

/* select the adapter by interface name/index*/

DWORD FindBrcmAdapter(HANDLE irh, char * ifname)
{
        ULONG flags = 0;
        WINERR status = BCME_ERROR;
        struct ether_addr ifaddr;

        if(ifname == NULL){
                flags = SELECT_PARAM_AUTO;
        }else if ((ifname[0] >= '0' && ifname[0] <= '9') && ifname[1] == '\0') {
                flags = SELECT_PARAM_INDEX;
        }else if ((strlen(ifname) ==17 )&& wl_ether_atoe(ifname, &ifaddr)) {
                /* maybe we got passed a MAC address */
                /* aa:bb:cc:dd:ee:ff  = 17 chars*/
                flags = SELECT_PARAM_ADDR;
        }else {
                flags = SELECT_PARAM_NAME;
        }

        if(hDevHandle != INVALID_HANDLE_VALUE)  {
                status = SelectBrcmAdapter(irh, ifname, flags);
        }

        if(status == BCME_OK)
                flags = SELECT_PARAM_AUTO;// use physical for ACM

        if(irh != INVALID_HANDLE_VALUE) {
                status = SelectBrcmAdapterLegacy(irh, ifname, flags);
        }

        return status;
}

DWORD SelectAdapter(HANDLE irh, char * ifname)
{
        ULONG Iterations = 0;

        /* sometimes FindBrcmAdapter cannot successfully find BRCM adapter even it exists
           because the GetAdaptersAddresses or WlanEnumInterfaces may return unexpected/unknown ERROR,
           re-call FindBrcmAdapter in a limited loop to try to find BRCM adapter when error happens
        */
        while(Iterations < MAX_RETRIES){
                if (FindBrcmAdapter(irh, ifname) == BCME_OK)
                        break;
                Iterations++;
        }
        if(Iterations == MAX_RETRIES)
                return BCME_ERROR;
        return BCME_OK;
}

DWORD DeSelectAdapter()
{
	Unbind();
	Uninitialize();
	return BCME_OK;
}

#if !defined(VISTA_SERVER) && defined(WLP2P)

#define VNDR_IE_OK_FLAGS \
	(VNDR_IE_BEACON_FLAG | VNDR_IE_PRBRSP_FLAG | VNDR_IE_ASSOCRSP_FLAG | \
	 VNDR_IE_AUTHRSP_FLAG | VNDR_IE_PRBREQ_FLAG | VNDR_IE_ASSOCREQ_FLAG | \
	 VNDR_IE_GONREQ_FLAG | VNDR_IE_GONRSP_FLAG | VNDR_IE_GONCFM_FLAG |	\
	 VNDR_IE_INVREQ_FLAG | VNDR_IE_INVRSP_FLAG | VNDR_IE_DISREQ_FLAG |	\
	 VNDR_IE_DISRSP_FLAG | VNDR_IE_PRDREQ_FLAG | VNDR_IE_PRDRSP_FLAG)

int
vista_p2p_vndr_ie(void *wl, cmd_t *cmd, char **argv)
{
	int err;
	const char *cmd_str = argv[1];

	if (cmd_str == NULL) {
		err = wl_list_ie(wl, cmd, argv);

	} else if (!stricmp(cmd_str, "add") ||
			   !stricmp(cmd_str, "del")) {
		err = vista_set_p2p_vndr_ie(wl, cmd_str, VNDR_IE_OK_FLAGS, ++argv);

	} else {
		err = -1;
	}

	return err;
}

int
vista_set_p2p_vndr_ie(void *wl, const char *command, uint32 pktflag_ok, char **argv)
{
	uint8 *p = vistabuf;
	vndr_ie_setbuf_t *ie_setbuf;
	int ie_setbuf_len;
	int err;

	strcpy(p, "p2p_vndr_ie");
	p += sizeof("p2p_vndr_ie")-1;
	*p++ = '\0';

	if ((err = wl_mk_ie_setbuf(command, pktflag_ok, argv, &ie_setbuf, &ie_setbuf_len)) != 0)
		return err;

	if (ie_setbuf_len <= VISTABUF_MAXLEN - (int)(p-vistabuf)) {
		memcpy(p, ie_setbuf, ie_setbuf_len);
		p += ie_setbuf_len;
		err = wl_set(wl, WLC_SET_VAR, vistabuf, (int)(p-vistabuf));

	} else {
		err = BCME_BUFTOOLONG;
	}

	free(ie_setbuf);

	return (err);
}

#endif /* !VISTA_SERVER && WLP2P */
