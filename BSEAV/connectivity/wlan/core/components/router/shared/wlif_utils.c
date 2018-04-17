/*
 * Wireless interface translation utility functions
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

#include <typedefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <bcmparams.h>
#include <bcmtimer.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <netconf.h>
#include <nvparse.h>
#include <shutils.h>
#include <wlutils.h>
#include <wlif_utils.h>
//#include <osl.h>

#ifndef MAX_NVPARSE
#define MAX_NVPARSE 255
#endif

#define WL_WLIF_BUFSIZE_4K	4096
#define WL_MACMODE_MASK		0x6000
#define WL_MACPROBE_MASK	0x1000
#define WL_MACMODE_SHIFT	13
#define WL_MACPROBE_SHIFT	12
/* Static info set */
#define WL_SINFOSET_SHIFT	15

#define WL_MAC_PROB_MODE_SET(m, p, f)	((m << WL_MACMODE_SHIFT) | (p << WL_MACPROBE_SHIFT) \
		| (f << WL_SINFOSET_SHIFT))
#define	WL_MACMODE_GET(f)	(f & WL_MACMODE_MASK >> WL_MACMODE_SHIFT)
#define	WL_MACPROBE_GET(f)	(f & WL_MACPROBE_MASK >> WL_MACPROBE_SHIFT)
#define TIMER_MAX_COUNT	16
#define WL_MAC_ADD	1
#define WL_MAC_DEL	0

#ifdef BCMDBG
#define WLIF_BSSTRANS_DBG	1	/* Turn on the debug */
#else
#define WLIF_BSSTRANS_DBG	0	/* Turn off the debug */
#endif /* BCMDBG */

#if WLIF_BSSTRANS_DBG
#define WLIF_BSSTRANS(fmt, args...) printf("WLIF_BSSTRANS >> %s: " fmt, __FUNCTION__, ##args)
#else
#define WLIF_BSSTRANS(fmt, args...)
#endif /* WLIF_BSSTRANS_DBG */

/*
 * Struct for wlif handle data.
 * and app specific data.
 */
typedef struct wl_wlif_hdl_data {
	char ifname[IFNAMSIZ];		/* Interface name. */
	callback_hndlr ioctl_hndlr;	/* Ioctl call handler. */
	void *uschd_hdl;		/* Uschd lib handle. */
	void *data;			/* App specific data. */
} wl_wlif_hdl_data_t;

/*
 * Flag Bit Values
 * 0-0	: Static info set
 * 1-1	: Probe Mode
 * 2-3	: Mac mode
 * 4-7	: Reserved
 * 8-15 : Static mac list count
 */
typedef struct static_maclist_ {
	wl_wlif_hdl_data_t *hdl_data;
	short flag;
	struct ether_addr addr;
} static_maclist_t;

/* Steering lib default ioctl handler routine. */
static int wl_wlif_do_ioctl(char *ifname, int cmd, void *buf, int buflen,
        void *data);

/* Counter to match the response for BSS transition request */
static uint8 bss_token = 0;

int static wl_wlif_unblock_mac_cb(bcm_timer_id timer, static_maclist_t *data);

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
		if (get_wlname_by_mac(mac, nv_name)) {
			return WLIFU_ERR_INVALID_PARAMETER;
		} else if (nvifname_to_osifname(nv_name, os_name, sizeof(os_name))) {
			return WLIFU_ERR_INVALID_PARAMETER;
		}
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



	return WLIFU_WSEC_SUCCESS;
}

#ifdef __CONFIG_RSDB__
int
wl_wlif_get_rsdb_mode()
{
	char *mode;
	int rsdb_mode = WLIF_RSDB_MODE_2X2; /* default rsdb_mode is mimo */

	mode = nvram_get("rsdb_mode");
	if (mode)
		rsdb_mode = atoi(mode);
	return rsdb_mode;
}
#endif /* __CONFIG_RSDB__ */

/* Generic utility function to check for a known capability */
int
wl_wlif_get_chip_cap(char *ifname, char *cap)
{
	char caps[WLC_IOCTL_MEDLEN], var[WLC_IOCTL_SMLEN], *next;

	if (wl_iovar_get(ifname, "cap", (void *)caps, sizeof(caps)) != BCME_OK)
		return FALSE;

	foreach(var, caps, next) {
		if (strncmp(var, cap, sizeof(var)) == 0)
			return TRUE;
	}

	return FALSE;
}

#ifndef BCM_EVENT_HEADER_LEN
#define BCM_EVENT_HEADER_LEN   (sizeof(bcm_event_t))
#endif

/* listen to sockets for bss response event */
static int
wl_wlif_proc_event_socket(int event_fd, struct timeval *tv, char *ifreq, uint8 token,
	struct ether_addr *bssid)
{
	fd_set fdset;
	int fdmax;
	int width, status = 0, bytes;
	char buf_ptr[WL_WLIF_BUFSIZE_4K], *pkt = buf_ptr;
	char ifname[IFNAMSIZ+1];
	int pdata_len;
	dot11_bsstrans_resp_t *bsstrans_resp;
#if WLIF_BSSTRANS_DBG
	dot11_neighbor_rep_ie_t *neighbor;
#endif /* WLIF_BSSTRANS_DBG */
	bcm_event_t *dpkt;
	uint32 event_id;
	struct ether_addr *addr;

	if (bssid == NULL) {
		WLIF_BSSTRANS("bssid is NULL\n");
		return WLIFU_BSS_TRANS_RESP_UNKNOWN;
	}

	WLIF_BSSTRANS("For event_fd[%d] ifname[%s] bss_token[%d] bssid["MACF"]\n",
		event_fd, ifreq, token, ETHERP_TO_MACF(bssid));

	while (1) {
		/* init file descriptor set */
		FD_ZERO(&fdset);
		fdmax = -1;

		/* build file descriptor set now to save time later */
		if (event_fd != -1) {
			FD_SET(event_fd, &fdset);
			fdmax = event_fd;
		}

		width = fdmax + 1;

		/* listen to data availible on all sockets */
		status = select(width, &fdset, NULL, NULL, tv);

		if ((status == -1 && errno == EINTR) || (status == 0)) {
			WLIF_BSSTRANS("ifname[%s] status[%d] errno[%d]- No event\n",
				ifreq, status, errno);
			return WLIFU_BSS_TRANS_RESP_UNKNOWN;
		}

		if (status <= 0) {
			WLIF_BSSTRANS("ifname[%s]: err from select: %s\n", ifreq, strerror(errno));
			return WLIFU_BSS_TRANS_RESP_UNKNOWN;
		}

		/* handle brcm event */
		if (event_fd !=  -1 && FD_ISSET(event_fd, &fdset)) {
			memset(pkt, 0, sizeof(buf_ptr));
			if ((bytes = recv(event_fd, pkt, sizeof(buf_ptr), 0)) <= IFNAMSIZ) {
				WLIF_BSSTRANS("ifname[%s] BSS Transit Response: recv err\n", ifreq);
				return WLIFU_BSS_TRANS_RESP_UNKNOWN;
			}

			strncpy(ifname, pkt, IFNAMSIZ);
			ifname[IFNAMSIZ] = '\0';

			pkt = pkt + IFNAMSIZ;
			pdata_len = bytes - IFNAMSIZ;

			if (pdata_len <= BCM_EVENT_HEADER_LEN) {
				WLIF_BSSTRANS("ifname[%s] BSS Transit Response: "
					"data_len %d too small\n", ifreq, pdata_len);
				continue;
			}

			dpkt = (bcm_event_t *)pkt;
			event_id = ntohl(dpkt->event.event_type);
			if (event_id != WLC_E_BSSTRANS_RESP) {
				WLIF_BSSTRANS("ifname[%s] BSS Transit Response: "
					"wrong event_id %d\n", ifreq, event_id);
				continue;
			}

			pkt += BCM_EVENT_HEADER_LEN; /* payload (bss response) */
			pdata_len -= BCM_EVENT_HEADER_LEN;

			bsstrans_resp = (dot11_bsstrans_resp_t *)pkt;
			addr = (struct ether_addr *)(bsstrans_resp->data);
#if WLIF_BSSTRANS_DBG
			neighbor = (dot11_neighbor_rep_ie_t *)bsstrans_resp->data;
#endif /* WLIF_BSSTRANS_DBG */

			WLIF_BSSTRANS("BSS Transit Response: ifname[%s], event[%d], "
				"token[%d], status[%d], mac["MACF"]\n",
				ifname, event_id, bsstrans_resp->token,
				bsstrans_resp->status, ETHERP_TO_MACF(addr));

			/* check interface */
			if (strncmp(ifname, ifreq, strlen(ifreq)) != 0) {
				/* not for the requested interface */
				WLIF_BSSTRANS("BSS Transit Response: not for interface "
					"%s its for %s\n", ifreq, ifname);
				continue;
			}

			/* check token */
			if (bsstrans_resp->token != token) {
				/* not for the requested interface */
				WLIF_BSSTRANS("BSS Transit Response: not for "
					"token %x it's for %x\n",
					token, bsstrans_resp->token);
				continue;
			}

			/* reject */
			if (bsstrans_resp->status) {
				/* If there is some neigbor report */
				if (status == 6) {
					WLIF_BSSTRANS("id[%d] len[%d] bssid["MACF"] "
						"bssid_info[%d] reg[%d] "
						"channel[%d] phytype[%d] \n",
						neighbor->id, neighbor->len,
						ETHER_TO_MACF(neighbor->bssid),
						neighbor->bssid_info, neighbor->reg,
						neighbor->channel, neighbor->phytype);
				}
				WLIF_BSSTRANS("BSS Transit Response: STA reject\n");

				return WLIFU_BSS_TRANS_RESP_REJECT;
			}

			/* accept, but use another target bssid */
			if (eacmp(bssid, addr) != 0) {
				WLIF_BSSTRANS("BSS Transit Response: target bssid not same "
					"["MACF"] != ["MACF"]\n", ETHERP_TO_MACF(bssid),
					ETHERP_TO_MACF(addr));
				return WLIFU_BSS_TRANS_RESP_REJECT;
			}

			return WLIFU_BSS_TRANS_RESP_ACCEPT;
		}
	} /* While  */

	return WLIFU_BSS_TRANS_RESP_UNKNOWN;
}

/*
 * Init routine for wlif lib handle.
 */
wl_wlif_hdl*
wl_wlif_init(void *uschd_hdl, char *ifname,
	callback_hndlr ioctl_hndlr, void *data)
{
	wl_wlif_hdl_data_t *hdl = (wl_wlif_hdl_data_t*)calloc(1, sizeof(*hdl));

	if (!hdl) {
		WLIF_BSSTRANS("Malloc failed for hdl \n");
		return NULL;
	}

	memset(hdl, 0, sizeof(*hdl));
	hdl->uschd_hdl = uschd_hdl;
	strncpy(hdl->ifname, ifname, IFNAMSIZ-1);
	hdl->ifname[IFNAMSIZ -1] = '\0';
	/* App does not registers own handler the default will be wl_wlif_do_ioctl */
	if (ioctl_hndlr) {
		hdl->ioctl_hndlr = ioctl_hndlr;
	} else {
		hdl->ioctl_hndlr = wl_wlif_do_ioctl;
	}

	hdl->data = data;

	return hdl;
}

/*
 * Deinit routine to wlif handle.
 */
void
wl_wlif_deinit(wl_wlif_hdl *hdl)
{
	wl_wlif_hdl_data_t *hdldata = (wl_wlif_hdl_data_t*)hdl;

	if (hdldata) {
		free(hdldata);
		hdldata = NULL;
	}
}

/* Default Callback function for ioctl calls. */
static int
wl_wlif_do_ioctl(char *ifname, int cmd, void *buf, int buflen,
	void *data)
{
	BCM_REFERENCE(data);
	return wl_ioctl(ifname, cmd, buf, buflen);
}

/*
 * Creates the BSS-Trans action frame from ioctl data.
 * Returns the BSS-Trans response.
 */
static int
wl_wlif_send_bss_transreq(wl_wlif_hdl_data_t *hdl_data,
	wl_wlif_bss_trans_data_t *ioctl_data, int event_fd)
{
	int ret, buflen;
	char *param, ioctl_buf[WLC_IOCTL_MAXLEN];
	struct timeval tv; /* timed out for bss response */
	dot11_bsstrans_req_t *transreq;
	dot11_neighbor_rep_ie_t *nbr_ie;
	wl_af_params_t *af_params;
	wl_action_frame_t *action_frame;

	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	strcpy(ioctl_buf, "actframe");
	buflen = strlen(ioctl_buf) + 1;
	param = (char *)(ioctl_buf + buflen);

	af_params = (wl_af_params_t *)param;
	action_frame = &af_params->action_frame;

	af_params->channel = 0;
	af_params->dwell_time = -1;

	memcpy(&action_frame->da, (char *)&(ioctl_data->addr), ETHER_ADDR_LEN);
	action_frame->packetId = (uint32)(uintptr)action_frame;
	action_frame->len = DOT11_NEIGHBOR_REP_IE_FIXED_LEN +
		TLV_HDR_LEN + DOT11_BSSTRANS_REQ_LEN;

	transreq = (dot11_bsstrans_req_t *)&action_frame->data[0];
	transreq->category = DOT11_ACTION_CAT_WNM;
	transreq->action = DOT11_WNM_ACTION_BSSTRANS_REQ;
	if (++bss_token == 0)
		bss_token = 1;
	transreq->token = bss_token;
	transreq->reqmode = DOT11_BSSTRANS_REQMODE_PREF_LIST_INCL;
	/* set bit1 to tell STA the BSSID in list recommended */
	transreq->reqmode |= DOT11_BSSTRANS_REQMODE_ABRIDGED;
	/*
		remove bit2 DOT11_BSSTRANS_REQMODE_DISASSOC_IMMINENT
		because WBD will deauth sta based on BSS response
	*/
	transreq->disassoc_tmr = 0x0000;
	transreq->validity_intrvl = 0x00;

	nbr_ie = (dot11_neighbor_rep_ie_t *)&transreq->data[0];
	nbr_ie->id = DOT11_MNG_NEIGHBOR_REP_ID;
	nbr_ie->len = DOT11_NEIGHBOR_REP_IE_FIXED_LEN;
	memcpy(&nbr_ie->bssid, &(ioctl_data->bssid), ETHER_ADDR_LEN);
	nbr_ie->bssid_info = 0x00000000;
	nbr_ie->reg = ioctl_data->rclass;
	nbr_ie->channel = wf_chspec_ctlchan(ioctl_data->chanspec);
	nbr_ie->phytype = 0x00;

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_VAR, ioctl_buf,
		WL_WIFI_AF_PARAMS_SIZE, hdl_data->data);


	if (ret < 0) {
		printf("Err: intf:%s actframe %d\n", hdl_data->ifname, ret);
	} else if (event_fd != -1) {
		/* read the BSS transition response only if event_fd is valid */
		usleep(1000*500);
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		/* wait for bss response and compare token/ifname/status/bssid etc  */
		return (wl_wlif_proc_event_socket(event_fd, &tv, hdl_data->ifname,
			bss_token, &(ioctl_data->bssid)));
	}
	return WLIFU_BSS_TRANS_RESP_UNKNOWN;
}

/* create blocking list of macs */
void static
wl_update_block_mac_list(maclist_t *static_maclist, maclist_t *maclist,
		int macmode, struct ether_addr *addr, unsigned char block)
{
	uint16 count = 0, idx, action;

	/* Action table
	 * ALLOW  BLOCK		DEL
	 * DENY   BLOCK		ADD
	 * ALLOW  UNBLOCK	ADD
	 * DENY   UNBLOCK	DEL
	 */
	action = ((macmode == WLC_MACMODE_ALLOW) ^ block) ? WL_MAC_ADD : WL_MAC_DEL;
	switch (action) {
		case WL_MAC_DEL:
			for (idx = 0; idx < static_maclist->count; idx++) {
				if (eacmp(addr, &static_maclist->ea[idx]) == 0) {
					continue;
				}
				memcpy(&(maclist->ea[count]), &static_maclist->ea[idx],
						ETHER_ADDR_LEN);
				count++;
			}
			maclist->count = count;
			break;

		case WL_MAC_ADD:
			for (idx = 0; static_maclist != NULL && idx < static_maclist->count;
					idx++) {
				memcpy(&(maclist->ea[count]), &static_maclist->ea[idx],
						ETHER_ADDR_LEN);
				count++;
			}

			memcpy(&(maclist->ea[count]), addr,  ETHER_ADDR_LEN);
			count++;
			maclist->count = count;
		break;

		default:
			printf("Wrong action= %d\n", action);
//			ASSERT(0);
	}
}

/*
 * Retrieves the current macmode, maclist and probe resp filter values
 * of interface, updates the data with STA address and sets them again.
 * If timeout is non zero than creates timer for unblock mac routine.
 */
int
wl_wlif_block_mac(wl_wlif_hdl *hdl, struct ether_addr addr, int timeout)
{
	char maclist_buf[WLC_IOCTL_MAXLEN];
	char smaclist_buf[WLC_IOCTL_MAXLEN];
	int macmode, ret, macprobresp;
	maclist_t *s_maclist = (maclist_t *)smaclist_buf;
	maclist_t *maclist = (maclist_t *)maclist_buf;
	static_maclist_t *data = NULL;
	static short flag;
	bcm_timer_module_id timer;
	bcm_timer_id timerid;
	struct itimerspec  its;
	wl_wlif_hdl_data_t *hdl_data;

	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		return FALSE;
	}

	hdl_data = (wl_wlif_hdl_data_t *)hdl;
	memset(maclist_buf, 0, WLC_IOCTL_MAXLEN);
	memset(smaclist_buf, 0, WLC_IOCTL_MAXLEN);
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_MACMODE, &(macmode),
		sizeof(macmode), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("Err: get %s macmode fails %d\n", hdl_data->ifname, ret);
		return FALSE;
	}

	/* retrive static maclist */
	if (hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_MACLIST, (void *)s_maclist,
		sizeof(maclist_buf), hdl_data->data) < 0) {
		WLIF_BSSTRANS("Err: get %s maclist fails\n", hdl_data->ifname);
		return FALSE;
	}

	ret = wl_iovar_getint(hdl_data->ifname, "probresp_mac_filter", &macprobresp);
	if (ret != 0) {
		WLIF_BSSTRANS("Err: %s Probresp mac filter set failed %d\n", hdl_data->ifname, ret);
	}

	if (flag == 0) {
		bcm_timer_module_init(TIMER_MAX_COUNT, &timer);
		flag = s_maclist->count;
		flag |= WL_MAC_PROB_MODE_SET(macmode, macprobresp, TRUE);
	}

	if (timeout != 0) {
		data = (static_maclist_t *) malloc(sizeof(*data));
		if (!data) {
			WLIF_BSSTRANS("Err: %s Malloc failed for maclist data\n", hdl_data->ifname);
			return FALSE;
		}

		memset(data, 0, sizeof(*data));
		data->hdl_data = hdl_data;
		data->flag = flag;
		eacopy(&addr, &(data->addr));
		if (hdl_data->uschd_hdl == NULL) {
			if (bcm_timer_create(timer, &timerid)) {
				goto end;
			} else if (bcm_timer_connect(timerid, (bcm_timer_cb)wl_wlif_unblock_mac_cb,
					data)) {
				goto end;
			}

			/* set up retry timer */
			its.it_interval.tv_sec = timeout;
			its.it_interval.tv_nsec = 0;
			its.it_value.tv_sec = timeout;
			its.it_value.tv_nsec = 0;
			if (bcm_timer_settime(timerid, &its)) {
				goto end;
			}
		} else {
			goto end;
		}
	}

	wl_update_block_mac_list(s_maclist, maclist, macmode, &addr, TRUE);
	macmode = (macmode == WLC_MACMODE_ALLOW) ? WLC_MACMODE_ALLOW : WLC_MACMODE_DENY;

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_MACLIST, maclist,
		ETHER_ADDR_LEN * maclist->count + sizeof(uint), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("Err: %s maclist set failed %d\n", hdl_data->ifname, ret);
	}

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_MACMODE, &macmode,
		sizeof(int), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("Err: %s macmode set failed %d\n", hdl_data->ifname, ret);
	}

	ret = wl_iovar_setint(hdl_data->ifname, "probresp_mac_filter", TRUE);
	if (ret != 0) {
		WLIF_BSSTRANS("Err: %s Probresp mac filter set failed %d\n", hdl_data->ifname, ret);
	}

	return TRUE;

end:
	if (data) {
		free(data);
		data = NULL;
	}

	return FALSE;
}

/*
 * If BSS-Trans response is not ACCEPT invokes block MAC
 * and deauthincates STA from current BSS.
 */
int
wl_wlif_do_bss_trans(wl_wlif_hdl *hdl,
	wl_wlif_bss_trans_data_t *ioctl_hndlr_data, int event_fd)
{
	int ret = -1, no_deauth = 0;
	char *value;
	wl_wlif_hdl_data_t *hdl_data;

	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		goto end;
	}

	hdl_data = (wl_wlif_hdl_data_t*)hdl;
	ret = wl_wlif_send_bss_transreq(hdl_data, ioctl_hndlr_data, event_fd);
	/* If the ret WLIFU_BSS_TRANS_RESP_ACCEPT:
	 *	STA will steer, no need to send deauth
	 * If the ret WLIFU_BSS_TRANS_RESP_REJECT:
	 *	Just return
	 * For WLIFU_BSS_TRANS_RESP_UNKNOWN: (Legacy sta)
	 *	Will be based on NVRAM setting to determine if send deauth or not
	 */
	if (ret == WLIFU_BSS_TRANS_RESP_REJECT) {
		WLIF_BSSTRANS("STA["MACF"] Rejected from BSSID["MACF"]\n",
			ETHER_TO_MACF(ioctl_hndlr_data->addr),
			ETHER_TO_MACF(ioctl_hndlr_data->bssid));
		goto end;
	} else if (ret == WLIFU_BSS_TRANS_RESP_UNKNOWN) {
		/* Read the NVRAM to deauth or not */
		value = nvram_safe_get(WLIFU_NVRAM_BSS_TRANS_NO_DEAUTH);
		no_deauth = (int)strtoul(value, NULL, 0);
		if (no_deauth) {
			WLIF_BSSTRANS("Not sending deauth to STA["MACF"] from %s by "
				"NVRAM WLIFU_NVRAM_BSS_TRANS_NO_DEAUTH setting\n",
				ETHER_TO_MACF(ioctl_hndlr_data->addr), hdl_data->ifname);
			goto end;
		}
	}

	wl_wlif_block_mac(hdl_data, ioctl_hndlr_data->addr,
		ioctl_hndlr_data->timeout);

	if (ret == WLIFU_BSS_TRANS_RESP_UNKNOWN) {
		/* no_deauth = 0 here, so send deauth */
		if (hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SCB_DEAUTHENTICATE,
			&(ioctl_hndlr_data->addr), ETHER_ADDR_LEN,
			hdl_data->data) < 0) {
			WLIF_BSSTRANS("Deauth to STA["MACF"] from %s failed\n",
				ETHER_TO_MACF(ioctl_hndlr_data->addr), hdl_data->ifname);
		}
	}

end:
	return ret;
}

/*
 * Retrieves the current macmode, maclist and proberesp filter values.
 * Updates maclist with STA addr entry. Based on flag bits
 * updates macmode and probe resp filter values.
 */
int
wl_wlif_unblock_mac(wl_wlif_hdl *hdl, struct ether_addr addr, int flag)
{
	char maclist_buf[WLC_IOCTL_MAXLEN];
	char smaclist_buf[WLC_IOCTL_MAXLEN];
	int macmode, ret = 0;
	wl_wlif_hdl_data_t *hdl_data;
	maclist_t *s_maclist = (maclist_t *)smaclist_buf;
	maclist_t *maclist = (maclist_t *)maclist_buf;

	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		return FALSE;
	}

	hdl_data = (wl_wlif_hdl_data_t*)hdl;

	memset(maclist_buf, 0, WLC_IOCTL_MAXLEN);
	memset(smaclist_buf, 0, WLC_IOCTL_MAXLEN);

	if (hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_MACMODE, &(macmode),
		sizeof(macmode), hdl_data->data) != 0) {
		WLIF_BSSTRANS("Err: get %s macmode fails \n", hdl_data->ifname);
		return FALSE;
	}

	/* retrive static maclist */
	if (hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_MACLIST, (void *)s_maclist,
		sizeof(maclist_buf), hdl_data->data) < 0) {
		WLIF_BSSTRANS("Err: get %s maclist fails\n", hdl_data->ifname);
		return FALSE;
	}

	wl_update_block_mac_list(s_maclist, maclist, macmode, &(addr), FALSE);
	macmode = (macmode == WLC_MACMODE_ALLOW) ? WLC_MACMODE_ALLOW : WLC_MACMODE_DENY;

	if (flag && (maclist->count == (flag & 0xFF))) {
		macmode = WL_MACMODE_GET(flag);
		if (wl_iovar_setint(hdl_data->ifname, "probresp_mac_filter",
			WL_MACPROBE_GET(flag)) != 0) {
			WLIF_BSSTRANS("Error:%s Probresp mac filter set failed \n",
				hdl_data->ifname);
		}
	}

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_MACLIST, maclist,
		ETHER_ADDR_LEN * maclist->count + sizeof(uint), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("Err: %s maclist filter set failed %d\n", hdl_data->ifname, ret);
	}

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_MACMODE, &macmode,
		sizeof(int), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("Err: %s macmode set failed %d\n", hdl_data->ifname, ret);
	}

	return TRUE;
}

/* Callback handler for unblock mac */
int static
wl_wlif_unblock_mac_cb(bcm_timer_id timer, static_maclist_t *data)
{
	int ret;

	ret = wl_wlif_unblock_mac(data->hdl_data, data->addr, data->flag);

	free(data);
	bcm_timer_delete(timer);

	return ret;
}

int
get_bridge_by_ifname(char* ifname, char** brname)
{
	char name[IFNAMSIZ] = {0}, *next = NULL;
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
	char wdsap_nvifname[IFNAMSIZ] = {0};

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
