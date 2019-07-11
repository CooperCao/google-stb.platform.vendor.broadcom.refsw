/*
 * Wireless interface translation utility functions
 *
 * Copyright (C) 2018, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: wlif_utils.c 669425 2016-11-09 12:26:48Z $
 */

#include <typedefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <bcmparams.h>
#include <bcmtimer.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <netconf.h>
#include <nvparse.h>
#include <shutils.h>
#include <wlutils.h>
#include <wlif_utils.h>
#include <pthread.h>
#include <signal.h>
#ifdef TARGETENV_android
#include <osl.h>
#endif /* TARGETENV_android */

#ifndef MAX_NVPARSE
#define MAX_NVPARSE 255
#endif

#define WLIF_MIN_BUF	128

/* From wlc_rate.[ch] */
#define MCS_TABLE_SIZE				33

/* IOCTL swapping mode for Big Endian host with Little Endian dongle.  Default to off */
/* The below macros handle endian mis-matches between wl utility and wl driver. */
bool g_swap = FALSE;
#define htod64(i) (g_swap?bcmswap64(i):(uint64)(i))
#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define htod16(i) (g_swap?bcmswap16(i):(uint16)(i))
#define dtoh64(i) (g_swap?bcmswap64(i):(uint64)(i))
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh16(i) (g_swap?bcmswap16(i):(uint16)(i))
#define htodchanspec(i) (g_swap?htod16(i):i)
#define dtohchanspec(i) (g_swap?dtoh16(i):i)
#define htodenum(i) (g_swap?((sizeof(i) == 4) ? htod32(i) : ((sizeof(i) == 2) ? htod16(i) : i)):i)
#define dtohenum(i) (g_swap?((sizeof(i) == 4) ? dtoh32(i) : ((sizeof(i) == 2) ? htod16(i) : i)):i)

int
get_wlname_by_mac(unsigned char *mac, char *wlname)
{
	char eabuf[18];
	char tmptr[] = "wlXXXXX_hwaddr";
	char *wl_hw;
	int i, j;

	ether_etoa(mac, eabuf);
	/* find out the wl name from mac */
	for (i = 0; i < MAX_NVPARSE; i++) {
		sprintf(wlname, "wl%d", i);
		sprintf(tmptr, "wl%d_hwaddr", i);
		wl_hw = nvram_get(tmptr);
		if (wl_hw) {
			if (!strncasecmp(wl_hw, eabuf, sizeof(eabuf)))
				return 0;
		}

		for (j = 1; j < WL_MAXBSSCFG; j++) {
			sprintf(wlname, "wl%d.%d", i, j);
			sprintf(tmptr, "wl%d.%d_hwaddr", i, j);
			wl_hw = nvram_get(tmptr);
			if (wl_hw) {
				if (!strncasecmp(wl_hw, eabuf, sizeof(eabuf)))
					return 0;
			}
		}
	}

	return -1;
}

bool
wl_wlif_is_psta(char *ifname)
{
	int32 psta = FALSE;

	if (wl_probe(ifname) < 0)
		return FALSE;

	if (wl_iovar_getint(ifname, "psta_if", &psta) < 0)
		return FALSE;

	return psta ? TRUE : FALSE;
}

bool
wl_wlif_is_dwds(char *ifname)
{
	int32 wds_type = FALSE;

	if (wl_probe(ifname) < 0)
		return FALSE;

	return (!wl_iovar_getint(ifname, "wds_type", &wds_type) && wds_type == WL_WDSIFTYPE_DWDS);
}

/*
 * Get LAN or WAN ifname by wl mac
 * NOTE: We pass ifname in case of same mac in vifs (like URE TR mode)
 */
char *
get_ifname_by_wlmac(unsigned char *mac, char *name)
{
	char nv_name[16], os_name[16], if_name[16];
	char tmptr[] = "lanXX_ifnames";
	char *ifnames, *ifname;
	int i;

	/*
	  * In case of URE mode, wl0.1 and wl0 have same mac,
	  * we need extra identity (name).
	  */
	if (name && !strncmp(name, "wl", 2))
		snprintf(nv_name, sizeof(nv_name), "%s", name);
	else if (get_wlname_by_mac(mac, nv_name))
		return 0;

	if (nvifname_to_osifname(nv_name, os_name, sizeof(os_name)) < 0)
		return 0;

	if (osifname_to_nvifname(os_name, nv_name, sizeof(nv_name)) < 0)
		return 0;

	/* find for lan */
	for (i = 0; i < WLIFU_MAX_NO_BRIDGE; i++) {
		if (i == 0) {
			ifnames = nvram_get("lan_ifnames");
			ifname = nvram_get("lan_ifname");
			if (ifname) {
				/* the name in ifnames may nvifname or osifname */
				if (find_in_list(ifnames, nv_name) ||
				    find_in_list(ifnames, os_name))
					return ifname;
			}
		}
		else {
			sprintf(if_name, "lan%d_ifnames", i);
			sprintf(tmptr, "lan%d_ifname", i);
			ifnames = nvram_get(if_name);
			ifname = nvram_get(tmptr);
			if (ifname) {
				/* the name in ifnames may nvifname or osifname */
				if (find_in_list(ifnames, nv_name) ||
				    find_in_list(ifnames, os_name))
					return ifname;
			}
		}
	}

	/* find for wan  */
	ifnames = nvram_get("wan_ifnames");
	ifname = nvram_get("wan0_ifname");
	/* the name in ifnames may nvifname or osifname */
	if (find_in_list(ifnames, nv_name) ||
	    find_in_list(ifnames, os_name))
		return ifname;

	return 0;
}

#define CHECK_NAS(mode) ((mode) & (WPA_AUTH_UNSPECIFIED | WPA_AUTH_PSK | \
				   WPA2_AUTH_UNSPECIFIED | WPA2_AUTH_PSK | WPA2_AUTH_FT))
#define CHECK_PSK(mode) ((mode) & (WPA_AUTH_PSK | WPA2_AUTH_PSK | WPA2_AUTH_FT))
#define CHECK_RADIUS(mode) ((mode) & (WPA_AUTH_UNSPECIFIED | WLIFU_AUTH_RADIUS | \
				      WPA2_AUTH_UNSPECIFIED))

/* Get wireless security setting by interface name */
int
get_wsec(wsec_info_t *info, unsigned char *mac, char *osifname)
{
	int i, unit, wds = 0, wds_wsec = 0;
	char nv_name[16], os_name[16], wl_prefix[16], comb[32], key[8];
	char wds_role[8], wds_ssid[48], wds_psk[80], wds_akms[16], wds_crypto[16],
	        remote[ETHER_ADDR_LEN];
	char akm[16], *akms, *akmnext, *value, *infra;

	if (info == NULL || mac == NULL)
		return WLIFU_ERR_INVALID_PARAMETER;

	if (nvifname_to_osifname(osifname, os_name, sizeof(os_name))) {
		if (get_wlname_by_mac(mac, nv_name))
			return WLIFU_ERR_INVALID_PARAMETER;
		else if (nvifname_to_osifname(nv_name, os_name, sizeof(os_name)))
			return WLIFU_ERR_INVALID_PARAMETER;
	}
	else if (osifname_to_nvifname(os_name, nv_name, sizeof(nv_name)))
			return WLIFU_ERR_INVALID_PARAMETER;

	/* check if i/f exists and retrieve the i/f index */
	if (wl_probe(os_name) ||
		wl_ioctl(os_name, WLC_GET_INSTANCE, &unit, sizeof(unit)))
		return WLIFU_ERR_NOT_WL_INTERFACE;

	/* get wl_prefix.
	 *
	 * Due to DWDS and WDS may be enabled at the same time,
	 * checking whether this is WDS interface in order to
	 * get per WDS interface security settings from NVRAM.
	 */
	if (strstr(os_name, "wds") && (wl_wlif_is_dwds(os_name) == FALSE)) {
		/* the wireless interface must be configured to run NAS */
		snprintf(wl_prefix, sizeof(wl_prefix), "wl%d", unit);
		wds = 1;
	}
	else if (wl_wlif_is_psta(os_name))
		snprintf(wl_prefix, sizeof(wl_prefix), "wl%d", unit);
	else if (osifname_to_nvifname(os_name, wl_prefix, sizeof(wl_prefix)))
		return WLIFU_ERR_INVALID_PARAMETER;

	strcat(wl_prefix, "_");
	memset(info, 0, sizeof(wsec_info_t));

	/* get wds setting */
	if (wds) {
		/* remote address */
		if (wl_ioctl(os_name, WLC_WDS_GET_REMOTE_HWADDR, remote, ETHER_ADDR_LEN))
			return WLIFU_ERR_WL_REMOTE_HWADDR;
		memcpy(info->remote, remote, ETHER_ADDR_LEN);

		/* get per wds settings */
		for (i = 0; i < MAX_NVPARSE; i ++) {
			char macaddr[18];
			uint8 ea[ETHER_ADDR_LEN];

			if (get_wds_wsec(unit, i, macaddr, wds_role, wds_crypto, wds_akms, wds_ssid,
			                 wds_psk) &&
			    ((ether_atoe(macaddr, ea) && !bcmp(ea, remote, ETHER_ADDR_LEN)) ||
			     ((mac[0] == '*') && (mac[1] == '\0')))) {
			     /* found wds settings */
			     wds_wsec = 1;
			     break;
			}
		}
	}

	/* interface unit */
	info->unit = unit;
	/* interface os name */
	strcpy(info->osifname, os_name);
	/* interface address */
	memcpy(info->ea, mac, ETHER_ADDR_LEN);
	/* ssid */
	if (wds && wds_wsec)
		strncpy(info->ssid, wds_ssid, MAX_SSID_LEN);
	else {
		value = nvram_safe_get(strcat_r(wl_prefix, "ssid", comb));
		strncpy(info->ssid, value, MAX_SSID_LEN);
	}
	/* auth */
	if (nvram_match(strcat_r(wl_prefix, "auth", comb), "1"))
		info->auth = 1;
	/* nas auth mode */
	value = nvram_safe_get(strcat_r(wl_prefix, "auth_mode", comb));
	info->akm = !strcmp(value, "radius") ? WLIFU_AUTH_RADIUS : 0;
	if (wds && wds_wsec)
		akms = wds_akms;
	else
		akms = nvram_safe_get(strcat_r(wl_prefix, "akm", comb));
	foreach(akm, akms, akmnext) {
		if (!strcmp(akm, "wpa"))
			info->akm |= WPA_AUTH_UNSPECIFIED;
		if (!strcmp(akm, "psk"))
			info->akm |= WPA_AUTH_PSK;
		if (!strcmp(akm, "wpa2"))
			info->akm |= WPA2_AUTH_UNSPECIFIED;
		if (!strcmp(akm, "psk2"))
			info->akm |= WPA2_AUTH_PSK;
		if (!strcmp(akm, "psk2ft"))
			info->akm |= WPA2_AUTH_PSK | WPA2_AUTH_FT;
	}
	/* wsec encryption */
	value = nvram_safe_get(strcat_r(wl_prefix, "wep", comb));
	info->wsec = !strcmp(value, "enabled") ? WEP_ENABLED : 0;
	if (wds && wds_wsec)
		value = wds_crypto;
	else
		value = nvram_safe_get(strcat_r(wl_prefix, "crypto", comb));
	if (CHECK_NAS(info->akm)) {
		if (!strcmp(value, "tkip"))
			info->wsec |= TKIP_ENABLED;
		else if (!strcmp(value, "aes"))
			info->wsec |= AES_ENABLED;
		else if (!strcmp(value, "tkip+aes"))
			info->wsec |= TKIP_ENABLED|AES_ENABLED;
	}
	/* nas role setting, may overwrite later in wds case */
	value = nvram_safe_get(strcat_r(wl_prefix, "mode", comb));
	infra = nvram_safe_get(strcat_r(wl_prefix, "infra", comb));
	if (!strcmp(value, "ap")) {
		info->flags |= WLIFU_WSEC_AUTH;
	}
	else if (!strcmp(value, "sta") || !strcmp(value, "wet") ||
	         !strcmp(value, "psr") || !strcmp(value, "psta")) {
		if (!strcmp(infra, "0")) {
			/* IBSS, so we must act as Authenticator and Supplicant */
			info->flags |= WLIFU_WSEC_AUTH;
			info->flags |= WLIFU_WSEC_SUPPL;
			/* Adhoc Mode */
			info->ibss = TRUE;
		}
		else {
			info->flags |= WLIFU_WSEC_SUPPL;
		}
	}
	else if (!strcmp(value, "wds")) {
		;
	}
	else {
		/* Unsupported network mode */
		return WLIFU_ERR_NOT_SUPPORT_MODE;
	}
	/* overwrite flags */
	if (wds) {
		char buf[32];
		unsigned char *ptr, lrole;

		/* did not find WDS link configuration, use wireless' */
		if (!wds_wsec)
			strcpy(wds_role, "auto");

		/* get right role */
		if (!strcmp(wds_role, "sup"))
			lrole = WL_WDS_WPA_ROLE_SUP;
		else if (!strcmp(wds_role, "auth"))
			lrole = WL_WDS_WPA_ROLE_AUTH;
		else /* if (!strcmp(wds_role, "auto")) */
			lrole = WL_WDS_WPA_ROLE_AUTO;

		strcpy(buf, "wds_wpa_role");
		ptr = (unsigned char *)buf + strlen(buf) + 1;
		bcopy(info->remote, ptr, ETHER_ADDR_LEN);
		ptr[ETHER_ADDR_LEN] = lrole;
		if (wl_ioctl(os_name, WLC_SET_VAR, buf, sizeof(buf)))
			return WLIFU_ERR_WL_WPA_ROLE;
		else if (wl_ioctl(os_name, WLC_GET_VAR, buf, sizeof(buf)))
			return WLIFU_ERR_WL_WPA_ROLE;
		lrole = *buf;

		/* overwrite these flags */
		info->flags = WLIFU_WSEC_WDS;
		if (lrole == WL_WDS_WPA_ROLE_SUP) {
			info->flags |= WLIFU_WSEC_SUPPL;
		}
		else if (lrole == WL_WDS_WPA_ROLE_AUTH) {
			info->flags |= WLIFU_WSEC_AUTH;
		}
		else {
			/* unable to determine WPA role */
			return WLIFU_ERR_WL_WPA_ROLE;
		}
	}
	/* user-supplied psk passphrase */
	if (CHECK_PSK(info->akm)) {
		if (wds && wds_wsec) {
			strncpy((char *)info->psk, wds_psk, MAX_USER_KEY_LEN);
			info->psk[MAX_USER_KEY_LEN] = 0;
		}
		else {
			value = nvram_safe_get(strcat_r(wl_prefix, "wpa_psk", comb));
			strncpy((char *)info->psk, value, MAX_USER_KEY_LEN);
			info->psk[MAX_USER_KEY_LEN] = 0;
		}
	}
	/* user-supplied radius server secret */
	if (CHECK_RADIUS(info->akm))
		info->secret = nvram_safe_get(strcat_r(wl_prefix, "radius_key", comb));
	/* AP specific settings */
	value = nvram_safe_get(strcat_r(wl_prefix, "mode", comb));
	if (!strcmp(value, "ap")) {
		/* gtk rekey interval */
		if (CHECK_NAS(info->akm)) {
			value = nvram_safe_get(strcat_r(wl_prefix, "wpa_gtk_rekey", comb));
			info->gtk_rekey_secs = (int)strtoul(value, NULL, 0);
		}
		/* wep key */
		if (info->wsec & WEP_ENABLED) {
			/* key index */
			value = nvram_safe_get(strcat_r(wl_prefix, "key", comb));
			info->wep_index = (int)strtoul(value, NULL, 0);
			/* key */
			sprintf(key, "key%s", nvram_safe_get(strcat_r(wl_prefix, "key", comb)));
			info->wep_key = nvram_safe_get(strcat_r(wl_prefix, key, comb));
		}
		/* radius server host/port */
		if (CHECK_RADIUS(info->akm)) {
			/* update radius server address */
			info->radius_addr = nvram_safe_get(strcat_r(wl_prefix, "radius_ipaddr",
			                                            comb));
			value = nvram_safe_get(strcat_r(wl_prefix, "radius_port", comb));
			info->radius_port = htons((int)strtoul(value, NULL, 0));
			/* 802.1x session timeout/pmk cache duration */
			value = nvram_safe_get(strcat_r(wl_prefix, "net_reauth", comb));
			info->ssn_to = (int)strtoul(value, NULL, 0);
		}
	}
	/* preauth */
	value = nvram_safe_get(strcat_r(wl_prefix, "preauth", comb));
	info->preauth = (int)strtoul(value, NULL, 0);

	/* verbose */
	value = nvram_safe_get(strcat_r(wl_prefix, "nas_dbg", comb));
	info->debug = (int)strtoul(value, NULL, 0);


	/* get mfp setting */
	info->mfp = atoi(nvram_safe_get(strcat_r(wl_prefix, "mfp", comb)));

	return WLIFU_WSEC_SUCCESS;
}

/* get the Max NSS */
int
wl_wlif_get_max_nss(wl_bss_info_t *bi)
{
	int i = 0, mcs_idx = 0;
	int mcs = 0, isht = 0;
	int nss = 0;

	if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap) {
		if (bi->vht_cap) {
			uint mcs_cap = 0;

			for (i = 1; i <= VHT_CAP_MCS_MAP_NSS_MAX; i++) {
				mcs_cap = VHT_MCS_MAP_GET_MCS_PER_SS(i,
					dtoh16(bi->vht_txmcsmap));
				if (mcs_cap != VHT_CAP_MCS_MAP_NONE) {
					nss++; /* Calculate the number of streams */
				}
			}

			if (nss) {
				return nss;
			}
		}

		/* For 802.11n networks, use MCS table */
		for (mcs_idx = 0; mcs_idx < (MCSSET_LEN * 8); mcs_idx++) {
			if (isset(bi->basic_mcs, mcs_idx) && mcs_idx < MCS_TABLE_SIZE) {
				mcs = mcs_idx;
				isht = 1;
			}
		}

		if (isht) {
			int nss = 0;

			if (mcs > 32) {
				printf("MCS is Out of range \n");
			} else if (mcs == 32) {
				nss = 1;
			} else {
				nss = 1 + (mcs / 8);
			}

			return nss;
		}
	}

	return nss;
}

int
get_bridge_by_ifname(char* ifname, char** brname)
{
	char name[IFNAMSIZ], *next = NULL;
	char *br_ifnames = NULL;
	int found = 0;

	/* Search in LAN network */
	br_ifnames = nvram_safe_get("lan_ifnames");
	foreach(name, br_ifnames, next) {
		if (!strcmp(name, ifname)) {
			found = 1;
			break;
		}
	}

	if (found) {
		*brname = nvram_safe_get("lan_ifname");
		return 0;
	}

	/* Search in GUEST network */
	br_ifnames = nvram_safe_get("lan1_ifnames");
	foreach(name, br_ifnames, next) {
		if (!strcmp(name, ifname)) {
			found = 1;
			break;
		}
	}

	if (found) {
		*brname = nvram_safe_get("lan1_ifname");
		return 0;
	}

	return -1;
}

/* Get associated AP ifname for WDS link */
int
wl_wlif_wds_ap_ifname(char *ifname, char *apname)
{
	int ret;
	char wdsap_nvifname[IFNAMSIZ];

	if (wl_probe(ifname) < 0) {
		return -1;
	}

	/* Get associated AP ifname and convert it to OS ifname */
	ret = wl_iovar_get(ifname, "wds_ap_ifname", (void *)wdsap_nvifname, IFNAMSIZ);

	if (!ret) {
		ret = nvifname_to_osifname(wdsap_nvifname, apname, IFNAMSIZ);
	} else {
		printf("Err: get %s wds_ap_ifname fails %d\n", ifname, ret);
	}

	return ret;
}

#if defined(CONFIG_HOSTAPD) && defined(BCA_HNDROUTER)
#define WLIF_WPS_LED_OFFSET			0
#define WLIF_WPS_LED_MASK			0x03L
#define WLIF_WPS_LED_STATUS_OFFSET		8
#define WLIF_WPS_LED_STATUS_MASK		0x0000ff00L
#define WLIF_WPS_LED_EVENT_OFFSET		16
#define WLIF_WPS_LED_EVENT_MASK			0x00ff0000L
#define WLIF_WPS_LED_BLINK_OFFSET		24
#define WLIF_WPS_LED_BLINK_MASK			0xff000000L

/* WPS led states */
#define WLIF_WPS_LED_OFF			0
#define WLIF_WPS_LED_ON				1
#define WLIF_WPS_LED_BLINK			2

/* WPS SM State */
#define WLIF_WPS_STATE_IDLE			0
#define WLIF_WPS_STATE_WAITING			1
#define WLIF_WPS_STATE_SUCC			2
#define WLIF_WPS_STATE_TIMEOUT			3
#define WLIF_WPS_STATE_FAIL			4
#define WLIF_WPS_STATE_OVERLAP			5

#define WLIF_WPS_EVENT_START			2
#define WLIF_WPS_EVENT_IDLE			(WLIF_WPS_STATE_IDLE + WLIF_WPS_EVENT_START)
#define WLIF_WPS_EVENT_WAITING			(WLIF_WPS_STATE_WAITING + WLIF_WPS_EVENT_START)
#define WLIF_WPS_EVENT_SUCC			(WLIF_WPS_STATE_SUCC + WLIF_WPS_EVENT_START)
#define WLIF_WPS_EVENT_TIMEOUT			(WLIF_WPS_STATE_TIMEOUT + WLIF_WPS_EVENT_START)
#define WLIF_WPS_EVENT_FAIL			(WLIF_WPS_STATE_FAIL + WLIF_WPS_EVENT_START)
#define WLIF_WPS_EVENT_OVERLAP			(WLIF_WPS_STATE_OVERLAP + WLIF_WPS_EVENT_START)

/* Sets the wps led status */
static void
wl_wlif_wps_led_set(int board_fp, int led_action, int led_blink_type,
	int led_event, int led_status)
{
	BOARD_IOCTL_PARMS ioctl_parms = {0};

	led_action &= WLIF_WPS_LED_MASK;
	led_action |= ((led_status << WLIF_WPS_LED_STATUS_OFFSET) & WLIF_WPS_LED_STATUS_MASK);
	led_action |= ((led_event << WLIF_WPS_LED_EVENT_OFFSET) & WLIF_WPS_LED_EVENT_MASK);
	led_action |= ((led_blink_type << WLIF_WPS_LED_BLINK_OFFSET) & WLIF_WPS_LED_BLINK_MASK);

	ioctl_parms.result = -1;
	ioctl_parms.string = (char *)&led_action;
	ioctl_parms.strLen = sizeof(led_action);
	ioctl(board_fp, BOARD_IOCTL_SET_SES_LED, &ioctl_parms);

	return;
}

/* Routine for changing wps led status */
void
wl_wlif_wps_gpio_led_blink(int board_fp, wlif_wps_blinktype_t blinktype)
{
	if (board_fp <= 0) {
		return;
	}

	switch ((int)blinktype) {
		case WLIF_WPS_BLINKTYPE_STOP:
			wl_wlif_wps_led_set(board_fp, WLIF_WPS_LED_OFF, 0, 0, 0);
			break;

		case WLIF_WPS_BLINKTYPE_INPROGRESS:
			wl_wlif_wps_led_set(board_fp, WLIF_WPS_LED_BLINK,
				kLedStateUserWpsInProgress, WLIF_WPS_EVENT_WAITING, 0);
			break;

		case WLIF_WPS_BLINKTYPE_ERROR:
			wl_wlif_wps_led_set(board_fp, WLIF_WPS_LED_BLINK,
				kLedStateUserWpsError, WLIF_WPS_EVENT_FAIL, 0);
			break;

		case WLIF_WPS_BLINKTYPE_OVERLAP:
			wl_wlif_wps_led_set(board_fp, WLIF_WPS_LED_BLINK,
				kLedStateUserWpsSessionOverLap, WLIF_WPS_EVENT_OVERLAP, 0);
			break;

		case WLIF_WPS_BLINKTYPE_SUCCESS:
			wl_wlif_wps_led_set(board_fp, WLIF_WPS_LED_ON,
				kLedStateOn, WLIF_WPS_EVENT_SUCC, 0);
			break;

		default:
			wl_wlif_wps_led_set(board_fp, WLIF_WPS_LED_OFF, 0, 0, 0);
			break;
	}
}

/* wps gpio init fn */
int
wl_wlif_wps_gpio_init()
{

	int board_fp = open("/dev/brcmboard", O_RDWR);

	if (board_fp <= 0) {
		return -1;
	}

	return board_fp;
}

/* wps gpio cleanup fn */
void
wl_wlif_wps_gpio_cleanup(int board_fp)
{
	if (board_fp > 0) {
		close(board_fp);
	}
}
#endif	/* CONFIG_HOSTAPD && BCA_HNDROUTER */

#ifdef CONFIG_HOSTAPD
// Wps modes
#define WLIF_WPS_ENROLEE	1
#define WLIF_WPS_REGISTRAR	2

// Flags to indicate hostapd wps states
#define WLIF_HAPD_WPS_ACTIVE	"PBC Status: Active"
#define WLIF_HAPD_WPS_OVER	"PBC Status: Disabled"
#define WLIF_HAPD_WPS_OVERLAP	"PBC Status: Overlap"
#define	WLIF_HAPD_WPS_SUCCESS	"Last WPS result: Success"
#define	WLIF_HAPD_WPS_FAILED	"Last WPS result: Failed"

// Flags to indicate wpa wps states
#define WLIF_WPA_WPS_SCANNING		"wpa_state=SCANNING"
#define WLIF_WPA_WPS_DISCONNECTED	"wpa_state=DISCONNECTED"
#define	WLIF_WPA_WPS_START		"key_mgmt=WPS"
#define	WLIF_WPA_WPS_COMPLETED		"wpa_state=COMPLETED"

// Key mgmt types
#define WLIF_WPA_AKM_PSK	0x1	// WPA-PSK
#define WLIF_WPA_AKM_PSK2	0x2	// WPA2-PSK

// Auth algo type
#define WLIF_WPA_AUTH_OPEN	0x1	// OPEN
#define WLIF_WPA_AUTH_SHARED	0x2	// SHARED

// Encryption types
#define WLIF_WPA_ENCR_TKIP	0x1	// TKIP
#define WLIF_WPA_ENCR_AES	0x2	// AES

// SSID and psk size
#define WLIF_SSID_MAX_SZ	32
#define WLIF_PSK_MAX_SZ		64

#ifdef TARGETENV_android
#define WLIF_WPS_STATUS_FILE	"/data/tmp/wlif_wps.status"
#define WLIF_HAPD_DIR		"/data/var/run/hostapd/"
#else
#define WLIF_WPS_STATUS_FILE	"/tmp/wlif_wps.status"
#define WLIF_HAPD_DIR		"/var/run/hostapd/"
#endif /* TARGETENV_android */

// Struct to hold the network settings received using wps.
typedef struct wlif_wps_nw_settings_ {
	char ssid[WLIF_SSID_MAX_SZ + /* '\0' */ 1];	// SSID.
	char nw_key[WLIF_PSK_MAX_SZ + /* '\0' */ 1];	// Network Key.
	uint8 akm;		// Key mgmt.
	uint8 auth_type;	// Auth type can be open/shared.
	uint8 encr;		// Encryption types TKIP/AES.
	uint8 scan_ssid;	// Scan this SSID with Probe Requests.
} wlif_wps_nw_creds_t;

// Struct ho pass the data to thread function which in turn checks the wps status.
typedef struct wlif_wps_args_ {
	char *prefix;	// Interface prefix.
	char *ifname;	// Interface name.
	char *cmd;	// wpa cli command.
	int start;	// Timestamp when wps process started.
	wlif_wps_mode_t mode;	// Wps mode ENROLEE/REGISTRAR.
	wlif_wps_nw_creds_t *creds;	// Saves the n/w credentials received in M8 msg.
} wlif_wps_args_t;

// WPS Status id and code value pairs
typedef struct wlif_wps_status_ {
	wlif_wps_ui_status_code_id_t idx;
	char *val;
} wlif_wps_ui_status_t;

// Array of wps ui status codes
static wlif_wps_ui_status_t wlif_wps_ui_status_arr[] =
{
	{WLIF_WPS_UI_INIT, "0"},
	{WLIF_WPS_UI_ASSOCIATED, "1"},
	{WLIF_WPS_UI_OK, "2"},
	{WLIF_WPS_UI_ERR, "3"},
	{WLIF_WPS_UI_TIMEOUT, "4"},
	{WLIF_WPS_UI_PBCOVERLAP, "8"},
	{WLIF_WPS_UI_FIND_PBC_AP, "9"}
};

// Free the wps args
static void
wl_wlif_free_wps_arg(wlif_wps_args_t *arg)
{
	if (arg) {
		if (arg->cmd) {
			free(arg->cmd);
		}
		if (arg->prefix) {
			free(arg->prefix);
		}
		if (arg->ifname) {
			free(arg->ifname);
		}
		if (arg->creds) {
			free(arg->creds);
		}
		free(arg);
	}
}

/* Updates the wps_proc_status nvram to reflect the wps status in wps.asp page */
static void
wl_wlif_update_wps_ui(wlif_wps_ui_status_code_id_t idx)
{
	if (idx < 0 || idx >= ARRAYSIZE(wlif_wps_ui_status_arr)) {
		return;
	}

	nvram_set("wps_proc_status", wlif_wps_ui_status_arr[idx].val);
}

/* Gets the status code from the wps_proc_status nvram value */
int
wl_wlif_get_wps_status_code()
{
	wlif_wps_ui_status_code_id_t idx = WLIF_WPS_UI_INIT;
	char *nvval = nvram_safe_get("wps_proc_status");

	if (nvval[0] == '\0') {
		return -1;
	}

	for (idx = 0; idx < ARRAYSIZE(wlif_wps_ui_status_arr); idx++) {
		if (!strcmp(nvval, wlif_wps_ui_status_arr[idx].val)) {
			return idx;
		}
	}

	return -1;
}
/* Saves the network credentials received from the wps session */
static void
wl_wlif_save_wpa_settings(char *type, char *val, wlif_wps_nw_creds_t *creds)
{

	int len = 0;

	/* In wpa_supplicant config file double quotes (") being added for ssid and
	 * psk configurations eg. ssid="BCM_TST". Before saving these values to nvrams we need to
	 * remove the double quotes from start and end of the string.
	 */
	if (strstr(type, "scan_ssid")) {
		creds->scan_ssid = atoi(val);
	} else if (strstr(type, "ssid")) {
		len = strlen(val) > (WLIF_SSID_MAX_SZ + 1) ? WLIF_SSID_MAX_SZ + 1 : strlen(val);
		strncpy(creds->ssid, val + 1, sizeof(creds->ssid) -1);
		creds->ssid[len - 2 /* for " at the start and end of string */]  = '\0';
	} else if (strstr(type, "psk")) {
		len = strlen(val) > (WLIF_PSK_MAX_SZ + 1) ? WLIF_PSK_MAX_SZ + 1 : strlen(val);
		strncpy(creds->nw_key, val + 1, sizeof(creds->nw_key) -1);
		creds->nw_key[len - 2 /* for " at the start and end of string */] = '\0';
	} else if (strstr(type, "key_mgmt")) {
		if (strstr(val, "WPA-PSK")) {
			creds->akm |= WLIF_WPA_AKM_PSK;
		}
		if (strstr(val, "WPA2-PSK")) {
			creds->akm |= WLIF_WPA_AKM_PSK2;
		}
	} else if (strstr(type, "auth_alg")) {
		if (strstr(val, "OPEN")) {
			creds->auth_type |= WLIF_WPA_AUTH_OPEN;
		}
		if (strstr(val, "SHARED")) {
			creds->auth_type |= WLIF_WPA_AUTH_SHARED;
		}
	} else if (strstr(type, "pairwise")) {
		if (strstr(val, "TKIP")) {
			creds->encr |= WLIF_WPA_ENCR_TKIP;
		}
		if (strstr(val, "CCMP")) {
			creds->encr |= WLIF_WPA_ENCR_AES;
		}
	}
}

/* Apply the network credentials to radio interface received from the wps session */
static int
wl_wlif_apply_creds(char *prefix, wlif_wps_nw_creds_t *creds)
{
	char nv_name[WLIF_MIN_BUF] = {0};
	char *val = "";
	bool wps_v2 = FALSE;
	int ret = -1;

	dprintf("Info: shared %s received credentials after wps:"
		"\nssid = [%s] \nakm = [%d] \nencr = [%d] \npsk = [%s]\n",
		__func__, creds->ssid, creds->akm, creds->encr, creds->nw_key);

	if (creds->ssid[0] == '\0') {
		return -1;
	}

	// ssid
	snprintf(nv_name, sizeof(nv_name), "%s_ssid", prefix);
	if (!nvram_match(nv_name, creds->ssid)) {
		nvram_set(nv_name, creds->ssid);
		ret = 0;
	}

	val = nvram_safe_get("wps_version2");
	if (!strcmp(val, "enabled")) {
		wps_v2 = TRUE;
	}

	// scan_ssid
	snprintf(nv_name, sizeof(nv_name), "%s_scan_ssid", prefix);
	nvram_set(nv_name, creds->scan_ssid ? "1" : "0");

	// for wps version 2 force psk2 is psk1 is enabled.
	if ((wps_v2 == TRUE) && (creds->akm & WLIF_WPA_AKM_PSK)) {
		creds->akm = WLIF_WPA_AKM_PSK2;
	}

	// akm
	val = "";
	snprintf(nv_name, sizeof(nv_name), "%s_akm", prefix);
	switch (creds->akm) {
		case WLIF_WPA_AKM_PSK:
			val = "psk";
		break;

		case WLIF_WPA_AKM_PSK2:
			val = "psk2";
		break;

		case (WLIF_WPA_AKM_PSK | WLIF_WPA_AKM_PSK2):
			val = "psk psk2";
		break;
	}
	if (!nvram_match(nv_name, val)) {
		nvram_set(nv_name, val);
		ret = 0;
	}

	// crypto
	snprintf(nv_name, sizeof(nv_name), "%s_crypto", prefix);
	val = "";
	switch (creds->encr) {
		case WLIF_WPA_ENCR_TKIP:
			val = "tkip";
		break;

		case WLIF_WPA_ENCR_AES:
			val = "aes";
		break;

		case (WLIF_WPA_ENCR_TKIP | WLIF_WPA_ENCR_AES):
			val = "tkip+aes";
		break;
	}
	if (!nvram_match(nv_name, val)) {
		nvram_set(nv_name, val);
		ret = 0;
	}

	snprintf(nv_name, sizeof(nv_name), "%s_wpa_psk", prefix);
	if (!nvram_match(nv_name, creds->nw_key)) {
		nvram_set(nv_name, creds->nw_key);
		ret = 0;
	}

	return ret;
}

/* Parses the wpa supplicant config file to save the network credentials received from wps */
static int
wl_wlif_parse_wpa_config_file(char *prefix, wlif_wps_nw_creds_t *creds)
{
	FILE *fp;
	int sz = 0, skip = 1;
	char buf[256], *ptr = NULL, *val = NULL;

#ifdef TARGETENV_android
	snprintf(buf, sizeof(buf), "/data/tmp/%s_wpa_supplicant.conf", prefix);
#else
	snprintf(buf, sizeof(buf), "/tmp/%s_wpa_supplicant.conf", prefix);
#endif /* TARGETENV_android */

	fp = fopen(buf, "r");
	if (fp == NULL) {
		cprintf("Err: shared %s failed to open file = [%s] \n", __func__, buf);
		return -1;
	}

	sz = sizeof(buf) - 1;
	while (!feof(fp)) {
		if (fgets(buf, sz, fp)) {
			buf[strcspn(buf, "\r\n")] = 0;

			// We need to parse only network block other part we can skip.
			if (!strcmp(buf, "network={")) {
				skip = 0;
				memset(creds, 0, sizeof(*creds));
			}
			if (!strcmp(buf, "}") && !skip) {
				skip = 1;
			}

			if (skip) {
				continue;
			}
			ptr = strtok_r(buf, "=", &val);
			if (!ptr || !val) {
				continue;
			}
			/* Since after successful wps session the network settings will be updated
			 * to the last network block. So here we are overwriting the settings.
			 */
			wl_wlif_save_wpa_settings(ptr, val, creds);
		}
	}

	fclose(fp);

	return 0;
}

// Thread states
typedef enum state_ {
	WLIF_THRD_CLOSED	= 0,
	WLIF_THRD_TOBE_CLOSED	= 1,
	WLIF_THRD_RUNNING	= 2
} wlif_thrd_states;

// Mutex lock for thread  used to pool the wps status
static pthread_mutex_t g_wlif_thrd_lock = PTHREAD_MUTEX_INITIALIZER;

// State of the thread which polls the wps status using wpa cli
static wlif_thrd_states g_wlif_thrd_state = WLIF_THRD_CLOSED;

/* Compare and set the thread state */
static int
wl_wlif_compare_and_set_thrd_state(wlif_thrd_states old_state, wlif_thrd_states new_state)
{
	pthread_mutex_lock(&g_wlif_thrd_lock);
	if (g_wlif_thrd_state == old_state) {
		g_wlif_thrd_state = new_state;
		pthread_mutex_unlock(&g_wlif_thrd_lock);
		return 1;
	}
	pthread_mutex_unlock(&g_wlif_thrd_lock);

	return 0;
}

/* Sets the thread state */
static void
wl_wlif_set_thrd_state(wlif_thrd_states state)
{
	pthread_mutex_lock(&g_wlif_thrd_lock);
	g_wlif_thrd_state = state;
	pthread_mutex_unlock(&g_wlif_thrd_lock);
}

/* Compares the wpa supplicant wps status str and updates the wps state flags */
int
wl_wlif_supp_wps_status(char *status_str, int sz, uint16 *flags)
{
	if (!status_str || !flags || sz <= 0) {
		return  -1;
	}

	if (!strncmp(status_str, WLIF_WPA_WPS_SCANNING, sz) ||
		!strncmp(status_str, WLIF_WPA_WPS_DISCONNECTED, sz)) {
		*flags |= WLIF_WPS_STARTED;
	}

	if (!strncmp(status_str, WLIF_WPA_WPS_START, sz)) {
		*flags |= WLIF_WPS_STARTED;
	}

	if (!strncmp(status_str, WLIF_WPA_WPS_COMPLETED, sz)) {
		*flags |= WLIF_WPS_COMPLETED;
	}

	if (WLIF_IS_WPS_STARTED(*flags) && WLIF_IS_WPS_COMPLETED(*flags)) {
		*flags |= WLIF_WPS_SUCCESS;
		return 0;
	}

	return -1;
}

/* Compares the hostapd wps status str and updates the wps state flags */
int
wl_wlif_hapd_wps_status(char *status_str, int sz, uint16 *flags)
{
	if (!status_str || !flags || sz <= 0) {
		return  -1;
	}

	if (!strncmp(status_str, WLIF_HAPD_WPS_ACTIVE, sz)) {
		*flags |= WLIF_WPS_STARTED;
	}

	if (WLIF_IS_WPS_STARTED(*flags) && !strncmp(status_str, WLIF_HAPD_WPS_OVER, sz)) {
		*flags |= WLIF_WPS_COMPLETED;
	}

	if (!strncmp(status_str, WLIF_HAPD_WPS_OVERLAP, sz)) {
		wl_wlif_update_wps_ui(WLIF_WPS_UI_PBCOVERLAP);
		*flags |= (WLIF_WPS_COMPLETED | WLIF_WPS_PBCOVERLAP);
		return 0;
	}

	if (WLIF_IS_WPS_COMPLETED(*flags)) {
		if (!strncmp(status_str, WLIF_HAPD_WPS_SUCCESS, sz)) {
			wl_wlif_update_wps_ui(WLIF_WPS_UI_OK);
			*flags |= WLIF_WPS_SUCCESS;
			return 0;
		}
		if (!strncmp(status_str, WLIF_HAPD_WPS_FAILED, sz)) {
			wl_wlif_update_wps_ui(WLIF_WPS_UI_ERR);
			*flags |= WLIF_WPS_FAILED;
			return 0;
		}
	}

	return  -1;
}

/* Pools the wps status using wpa cli status command */
static int
wl_wlif_wps_status_hdlr(wlif_wps_args_t *wps_arg, wlif_wps_mode_t wps_mode, uint16 *flags)
{
	FILE *fp = NULL;
	int ret = -1;
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "rm -rf %s", WLIF_WPS_STATUS_FILE);
	if (system(cmd)) {
		cprintf("Info: shared %s : cmd %s failed to execute\n", __func__, cmd);
	}

	snprintf(cmd, sizeof(cmd), "%s >> %s", wps_arg->cmd, WLIF_WPS_STATUS_FILE);
	if (system(cmd)) {
		cprintf("Info: shared %s : cmd %s failed to execute\n", __func__, cmd);
	}

	fp = fopen(WLIF_WPS_STATUS_FILE, "r");
	if (!fp) {
		cprintf("Err: shared %s fopen failed for file [%s] \n", __func__,
			WLIF_WPS_STATUS_FILE);
		goto end;
	}

	while (!feof(fp)) {
		char buf[WLIF_MIN_BUF] = {0};
		int sz = sizeof(buf) - 1;
		if (fgets(buf, sz, fp)) {
			buf[strcspn(buf, "\r\n")] = 0;
			if (wps_mode == WLIF_WPS_REGISTRAR) {
				// hapd status
				if ((ret = wl_wlif_hapd_wps_status(buf, sz, flags)) == 0) {
					break;
				}
			} else if (wps_mode == WLIF_WPS_ENROLEE) {
				// wpa ststus
				if ((ret = wl_wlif_supp_wps_status(buf, sz, flags)) == 0) {
					break;
				}
			}
		}
	}

	fclose(fp);

	return ret;

end:
	wl_wlif_set_thrd_state(WLIF_THRD_TOBE_CLOSED);
	return ret;
}

/* Based on wlx_mode sets the initial wps state */
static void
wl_wlif_update_initial_wps_state(char *prefix)
{
	char mode[WLIF_MIN_BUF] = {0};

	snprintf(mode, sizeof(mode), "%s_mode", prefix);
	if (nvram_match(mode, "ap")) {
		wl_wlif_update_wps_ui(WLIF_WPS_UI_ASSOCIATED);
	} else {
		wl_wlif_update_wps_ui(WLIF_WPS_UI_FIND_PBC_AP);
	}
}

/* Returns configuration state of ap */
static bool
wl_wlif_is_ap_configured(char *ifname)
{
	char *lan_ifnames = nvram_safe_get("lan_ifnames");
	char *lan1_ifnames = nvram_safe_get("lan1_ifnames");

	if (find_in_list(lan_ifnames, ifname)) {
		if (nvram_match("lan_wps_oob", "disabled")) {
			return TRUE;
		}
	} else if (find_in_list(lan1_ifnames, ifname)) {
		if (nvram_match("lan1_wps_oob", "disabled")) {
			return TRUE;
		}
	}

	return FALSE;
}

/* Sets the AP state to configured */
static void
wl_wlif_set_ap_as_configured(char *ifname)
{
	char *lan_ifnames = nvram_safe_get("lan_ifnames");
	char *lan1_ifnames = nvram_safe_get("lan1_ifnames");

	if (find_in_list(lan_ifnames, ifname)) {
		nvram_set("lan_wps_oob", "disabled");
	} else if (find_in_list(lan1_ifnames, ifname)) {
		nvram_set("lan1_wps_oob", "disabled");
	}
}

/* Saves the ap settings updated by hostapd  */
static void
wl_wlif_save_hapd_settings(char *type, char *val, wlif_wps_nw_creds_t *creds)
{
	if (!strcmp(type, "ssid")) {
		WLIF_STRNCPY(creds->ssid, val, sizeof(creds->ssid));
	} else if (!strcmp(type, "passphrase")) {
		WLIF_STRNCPY(creds->nw_key, val, sizeof(creds->nw_key));
	} else if (!strcmp(type, "key_mgmt")) {
		if (strstr(val, "WPA-PSK")) {
			creds->akm |= WLIF_WPA_AKM_PSK;
		}
		if (strstr(val, "WPA2-PSK")) {
			creds->akm |= WLIF_WPA_AKM_PSK2;
		}
	} else if (!strcmp(type, "rsn_pairwise_cipher")) {
		if (strstr(val, "TKIP")) {
			creds->encr = WLIF_WPA_ENCR_TKIP;
		}
		if (strstr(val, "CCMP")) {
			creds->encr = WLIF_WPA_ENCR_AES;
		}
	}
}

/* Parse hostapd config which gets updated after wps completion */
static int
wl_wlif_parse_hapd_config(char *ifname, wlif_wps_nw_creds_t *creds)
{
	FILE *fp;
	int sz = 0;
	char cmd[WLIF_MIN_BUF], *ptr = NULL, *val = NULL;

	snprintf(cmd, sizeof(cmd), "hostapd_cli -p %s -i %s get_config", WLIF_HAPD_DIR, ifname);

	fp = popen(cmd, "r");
	if (fp == NULL) {
		dprintf("Err: shared %s failed to open cmd = [%s] \n", __func__, cmd);
		return -1;
	}

	memset(creds, 0, sizeof(*creds));
	while (!feof(fp)) {
		char buf[WLIF_MIN_BUF] = {0};
		sz = sizeof(buf) - 1;
		if (fgets(buf, sz, fp)) {
			buf[strcspn(buf, "\r\n")] = 0;

			ptr = strtok_r(buf, "=", &val);
			if (!ptr || !val) {
				continue;
			}
			wl_wlif_save_hapd_settings(ptr, val, creds);
		}
	}

	pclose(fp);

	return 0;
}

/* Thread function */
static void*
wl_wlif_wps_thrd_func(void *arg)
{
	wlif_wps_args_t *wps_arg = (wlif_wps_args_t*)arg;
	int ret = -1, end = time(NULL);
	uint16 flags = 0;

	wl_wlif_update_initial_wps_state(wps_arg->prefix);
	while ((end - wps_arg->start) <= WLIF_WPS_TIMEOUT) {
		end = time(NULL);
		sleep(1);
		ret = wl_wlif_wps_status_hdlr(wps_arg, wps_arg->mode, &flags);
		if (ret == 0 && WLIF_IS_WPS_COMPLETED(flags)) {
			break;
		}
		if (wl_wlif_compare_and_set_thrd_state(WLIF_THRD_TOBE_CLOSED, WLIF_THRD_CLOSED)) {
			break;
		}
	}

	switch (wps_arg->mode) {
		case WLIF_WPS_ENROLEE:
			ret = wl_wlif_parse_wpa_config_file(wps_arg->prefix, wps_arg->creds);
			if (!ret) {
				ret = wl_wlif_apply_creds(wps_arg->prefix, wps_arg->creds);
			}
		break;

		case WLIF_WPS_REGISTRAR:
			if (!wl_wlif_is_ap_configured(wps_arg->ifname)) {
				ret = wl_wlif_parse_hapd_config(wps_arg->ifname, wps_arg->creds);
				if (ret == 0) {
					ret = wl_wlif_apply_creds(wps_arg->prefix, wps_arg->creds);
					wl_wlif_set_ap_as_configured(wps_arg->ifname);
				}
			} else {
				ret = -1; // Since AP is configured rc restart is not required.
				dprintf("Info: shared %s ifname %s is configured\n",
					__func__, wps_arg->ifname);
			}
		break;

		default:
			goto end;
	}

	if (flags & WLIF_WPS_SUCCESS) {
		wl_wlif_update_wps_ui(WLIF_WPS_UI_OK);
	} else {
		wl_wlif_update_wps_ui(WLIF_WPS_UI_TIMEOUT);
	}

end:
	wl_wlif_set_thrd_state(WLIF_THRD_CLOSED);
	wl_wlif_free_wps_arg(wps_arg);

	if (ret == 0) {
		wl_wlif_update_wps_ui(WLIF_WPS_UI_INIT);
		nvram_commit();
		kill(1, SIGHUP);
	}

	return NULL;
}

/* Creates a thread to pool the wps status from the wpa cli status cmd */
int
wl_wlif_wps_check_status(wlif_wps_t *wps)
{
	pthread_t thread;
	pthread_attr_t attr;
	int ret = 0;
	wlif_wps_args_t *arg = NULL;

	if (!wps) {
		goto exit;
	}

	/* Check if thread is already running close it */
	switch (wps->op) {
		case WLIF_WPS_START:
		case WLIF_WPS_RESTART:
			pthread_mutex_lock(&g_wlif_thrd_lock);
			if (g_wlif_thrd_state == WLIF_THRD_RUNNING) {
				pthread_mutex_unlock(&g_wlif_thrd_lock);
				dprintf("Info: shared %s %d thread is already running\n",
					__func__, __LINE__);
				return 0;
			}
			pthread_mutex_unlock(&g_wlif_thrd_lock);
		break;

		case WLIF_WPS_STOP:
			wl_wlif_compare_and_set_thrd_state(WLIF_THRD_RUNNING,
				WLIF_THRD_TOBE_CLOSED);
			wl_wlif_update_wps_ui(WLIF_WPS_UI_INIT);
		return 0;

		default:
			cprintf("Info: shared %s %d received unknown wps operation %d\n",
				__func__, __LINE__, wps->op);
		return -1;
	}

	if (wps->cmd[0] == '\0') {
		goto exit;
	}

	arg = (wlif_wps_args_t *)calloc(1, sizeof(*arg));
	if (!arg) {
		cprintf("Err: shared %s %d calloc failed\n", __func__, __LINE__);
		goto exit;
	}

	arg->cmd = strdup(wps->cmd);
	if (!arg->cmd) {
		cprintf("Err: shared %s %d calloc failed\n", __func__, __LINE__);
		goto exit;
	}

	arg->prefix = strdup(wps->prefix);
	if (!arg->prefix) {
		cprintf("Err: shared %s %d calloc failed\n", __func__, __LINE__);
		goto exit;
	}

	arg->ifname = strdup(wps->ifname);
	if (!arg->ifname) {
		cprintf("Err: shared %s %d calloc failed\n", __func__, __LINE__);
		goto exit;
	}

	arg->creds = (wlif_wps_nw_creds_t*)calloc(1, sizeof(wlif_wps_nw_creds_t));
	if (!arg->creds) {
		cprintf("Err: shared %s %d calloc failed\n", __func__, __LINE__);
		goto exit;
	}

	arg->start = wps->start;
	arg->mode = wps->mode;

	ret = pthread_attr_init(&attr);
	if (ret != 0) {
		cprintf("Err : shared %s %d pthread_attr_init failed \n", __func__, __LINE__);
		goto exit;
	}

	if (!wps->wait_thread) {
		ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		if (ret != 0) {
			cprintf("Err : shared %s %d shared pthread_attr_setdetachstat failed\n",
				__func__, __LINE__);
			pthread_attr_destroy(&attr);
			goto exit;
		}
	}

	ret = pthread_create(&thread, &attr, wl_wlif_wps_thrd_func, (void*)arg);
	if (ret != 0) {
		cprintf("Err : shared %s %d pthread_create failed \n", __func__, __LINE__);
		pthread_attr_destroy(&attr);
		goto exit;
	}

	wl_wlif_set_thrd_state(WLIF_THRD_RUNNING);

	if (wps->wait_thread) {
		ret = pthread_join(thread, NULL);
		if (ret != 0) {
			cprintf("Err : shared %s %d pthread_join failed, ret = %d\n", __func__, __LINE__, ret);
			pthread_attr_destroy(&attr);
			goto exit;
		}
	}

	pthread_attr_destroy(&attr);

	return 0;

exit:
	wl_wlif_free_wps_arg(arg);
	return -1;
}
#endif	/* CONFIG_HOSTAPD */
