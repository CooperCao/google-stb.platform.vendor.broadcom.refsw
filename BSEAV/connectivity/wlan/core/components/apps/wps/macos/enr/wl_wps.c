/*
 * Broadcom 802.11 device interface
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

#ifdef __linux__
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>
#endif // __linux__

#include <portability.h>
#include <wps_enrapi.h>
#include <wps_sta.h>
#include "wlioctl.h"
#include <wps_staeapsm.h>

extern char *ether_ntoa(const struct ether_addr *addr);
int tolower(int);
int wps_wl_ioctl(int cmd, void *buf, int len, bool set);
static int wps_iovar_set(const char *iovar, void *param, int paramlen);
static int wps_iovar_setbuf(const char *iovar,
	void *param, int paramlen, void *bufptr, int buflen);
static uint wps_iovar_mkbuf(const char *name, char *data, uint datalen,
	char *iovar_buf, uint buflen, int *perr);
static int wps_ioctl_get(int cmd, void *buf, int len);
static int wps_ioctl_set(int cmd, void *buf, int len);
static char *get_scan_results();

#define WPS_DUMP_BUF_LEN (127 * 1024)
#define WPS_SSID_FMT_BUF_LEN 4*32+1	/* Length for SSID format string */

#define WPS_SCAN_MAX_WAIT_SEC 10
#define WPS_JOIN_MAX_WAIT_SEC 60	/*
					 * Do not define this value too short,
					 * because AP/Router may reboot after got new
					 * credential and apply it.
					 */

wps_ap_list_info_t ap_list[WPS_MAX_AP_SCAN_LIST_LEN];
static char scan_result[WPS_DUMP_BUF_LEN];

char *get_scan_results()
{
	int ret, retry;
	wl_scan_params_t* params;
	wl_scan_results_t *list = (wl_scan_results_t*)scan_result;
	int params_size = WL_SCAN_PARAMS_FIXED_SIZE + WL_NUMCHANNELS * sizeof(uint16);

	params = (wl_scan_params_t*)malloc(params_size);
	if (params == NULL) {
		fprintf(stderr, "Error allocating %d bytes for scan params\n", params_size);
		return 0;
	}

	memset(params, 0, params_size);
	params->bss_type = DOT11_BSSTYPE_ANY;
	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->scan_type = -1;
	params->nprobes = -1;
	params->active_time = -1;
	params->passive_time = -1;
	params->home_time = -1;
	params->channel_num = 0;

	wps_ioctl_set(WLC_SCAN, params, params_size);

	/* Poll for the results once a second until the scan is done */
	for (retry = 0; retry < WPS_SCAN_MAX_WAIT_SEC; retry++) {
		WpsSleep(1000);

		list->buflen = WPS_DUMP_BUF_LEN;
		ret = wps_ioctl_get(WLC_SCAN_RESULTS, scan_result, WPS_DUMP_BUF_LEN);

		/* break out if the scan result is ready */
		if (ret == 0)
			break;
	}

	free(params);
	if (ret < 0)
		return NULL;
	return scan_result;
}

wps_ap_list_info_t *wps_get_ap_list()
{
	return ap_list;
}
wps_ap_list_info_t *create_aplist()
{
	wl_scan_results_t *list = (wl_scan_results_t*)scan_result;
	wl_bss_info_t *bi;
	wl_bss_info_107_t *old_bi_107;
	uint i, wps_ap_count = 0;

	get_scan_results();

	memset(ap_list, 0, sizeof(ap_list));
	if (list->count == 0)
		return 0;

#ifdef LEGACY2_WL_BSS_INFO_VERSION
	if (list->version != WL_BSS_INFO_VERSION &&
		list->version != LEGACY_WL_BSS_INFO_VERSION &&
		list->version != LEGACY2_WL_BSS_INFO_VERSION) {
#else
	if (list->version != WL_BSS_INFO_VERSION &&
		list->version != LEGACY_WL_BSS_INFO_VERSION) {
#endif
			fprintf(stderr, "Sorry, your driver has bss_info_version %d "
				"but this program supports only version %d.\n",
				list->version, WL_BSS_INFO_VERSION);
			return 0;
	}

	fprintf(stderr, "your driver has bss_info_version %d "
		"but this program supports only version %d.\n",
		list->version, WL_BSS_INFO_VERSION);

	bi = list->bss_info;
	for (i = 0; i < list->count; i++) {

		/* Convert version 107 to 108 */
		if (bi->version == LEGACY_WL_BSS_INFO_VERSION) {
			old_bi_107 = (wl_bss_info_107_t *)bi;
			bi->chanspec = CH20MHZ_CHSPEC(old_bi_107->channel);
			bi->ie_length = old_bi_107->ie_length;
			bi->ie_offset = sizeof(wl_bss_info_107_t);
		}
		if (bi->ie_length) {
			if (wps_ap_count < WPS_MAX_AP_SCAN_LIST_LEN) {

				ap_list[wps_ap_count].used = TRUE;
				memcpy(ap_list[wps_ap_count].BSSID, bi->BSSID.octet, 6);
				strncpy((char *)ap_list[wps_ap_count].ssid, (char *)bi->SSID,
					bi->SSID_len);
				ap_list[wps_ap_count].ssid[bi->SSID_len] = '\0';
				ap_list[wps_ap_count].ssidLen = bi->SSID_len;
				ap_list[wps_ap_count].ie_buf = (uint8 *)(((uint8 *)bi) +
					bi->ie_offset);
				ap_list[wps_ap_count].ie_buflen = bi->ie_length;
				ap_list[wps_ap_count].channel = (uint8)(bi->chanspec &
					WL_CHANSPEC_CHAN_MASK);
				ap_list[wps_ap_count].band = (uint16)(bi->chanspec &
					WL_CHANSPEC_BAND_MASK);
				ap_list[wps_ap_count].wep = bi->capability & DOT11_CAP_PRIVACY;
				wps_ap_count++;
			}

		}
		bi = (wl_bss_info_t*)((int8*)bi + bi->length);
	}
	return ap_list;
}

static vndr_ie_setbuf_t *ie_setbuf = NULL;
int create_wps_pbc_ie()
{
	unsigned int pktflag;
	int iecount;

	pktflag = VNDR_IE_PRBREQ_FLAG;

	if (!ie_setbuf)
		ie_setbuf = (vndr_ie_setbuf_t *) malloc(VNDR_IE_MAX_LEN + sizeof(vndr_ie_setbuf_t));

	if (! ie_setbuf) {
#ifdef _TUDEBUGTRACE
		printf("memory alloc failure\n");
#endif
		return -1;
	}

	/* Buffer contains only 1 IE */
	iecount = 1;
	memcpy(&ie_setbuf->vndr_ie_buffer.iecount, &iecount, sizeof(int));

	/* 
	* The packet flag bit field indicates the packets that will
	* contain this IE
	*/
	memcpy(&ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].pktflag, &pktflag, sizeof(uint32));

	/* Now, add the IE to the buffer */

	wps_build_pbc_proberq(&(ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.len),
		VNDR_IE_MAX_LEN);

#ifdef _TUDEBUGTRACE
	{
		int buflen = ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.len;
		uint8 *buff = &(ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.len);
		int i;
		for (i = 0; i < buflen; i++)
		{
			printf("%x ", buff[i]);
			if (!(i%16))
				printf("\n");
		}
	}
#endif
	return 0;
}

/* add probe request for enrollee */
int add_wps_ie(unsigned char *p_data, int length)
{

	int buflen;
	int err = 0;

	create_wps_pbc_ie();

	if (!ie_setbuf) {
#ifdef _TUDEBUGTRACE
		printf("wl_wps : No IE to remove\n");
#endif
		return -1;
	}

	/* try removing first in case there was one left from previous call */
	rem_wps_ie(NULL, 0);

	/* Copy the vndr_ie SET command ("add"/"del") to the buffer */
	strcpy(ie_setbuf->cmd, "add");

	buflen = ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.len -
		VNDR_IE_MIN_LEN + sizeof(vndr_ie_setbuf_t);

	err = wps_iovar_set("vndr_ie", ie_setbuf, buflen);
	return (err);
}

int rem_wps_ie(unsigned char *p_data, int length)
{
	int buflen;
	int err = 0;

	if (!ie_setbuf) {
#ifdef _TUDEBUGTRACE
		printf("wl_wps : No IE to remove\n");
#endif
		return -1;
	}

	/* Copy the vndr_ie SET command ("add"/"del") to the buffer */
	strcpy(ie_setbuf->cmd, "del");

	buflen = ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.len -
		VNDR_IE_MIN_LEN + sizeof(vndr_ie_setbuf_t);
	err = wps_iovar_set("vndr_ie", ie_setbuf, buflen);

	return (err);
}

int join_network(char* ssid, uint32 wsec)
{
	int ret = 0, retry;
	wlc_ssid_t ssid_t;
	int auth = 0, infra = 1;
	int wpa_auth = WPA_AUTH_DISABLED;
	char bssid[6];

#ifdef _TUDEBUGTRACE
	printf("Joining network %s - %d\n", ssid, wsec);
#endif
	/*
	 * If wep bit is on,
	 * pick any WPA encryption type to allow association.
	 * Registration traffic itself will be done in clear (eapol).
	*/
	if (wsec)
		wsec = 2; /* TKIP */
	ssid_t.SSID_len = strlen(ssid);
	strncpy((char *)ssid_t.SSID, ssid, ssid_t.SSID_len);
	ssid_t.SSID[ssid_t.SSID_len] = '\0';

	/* set infrastructure mode */
	if ((ret = wps_ioctl_set(WLC_SET_INFRA, &infra, sizeof(int))) < 0)
		return ret;

	/* set authentication mode */
	if ((ret = wps_ioctl_set(WLC_SET_AUTH, &auth, sizeof(int))) < 0)
		return ret;

	/* set wsec mode */
	if ((ret = wps_ioctl_set(WLC_SET_WSEC, &wsec, sizeof(int))) < 0)
		return ret;

	/* set WPA_auth mode */
	if ((ret = wps_ioctl_set(WLC_SET_WPA_AUTH, &wpa_auth, sizeof(wpa_auth))) < 0)
		return ret;

	/* set ssid */
	if ((ret = wps_ioctl_set(WLC_SET_SSID, &ssid_t, sizeof(wlc_ssid_t))) == 0) {
		/* Poll for the results once a second until we got BSSID */
		for (retry = 0; retry < WPS_JOIN_MAX_WAIT_SEC; retry++) {

			WpsSleep(1000);

			ret = wps_ioctl_get(WLC_GET_BSSID, bssid, 6);

			/* break out if the scan result is ready */
			if (ret == 0)
				break;

			if (retry != 0 && retry % 10 == 0) {
				if ((ret = wps_ioctl_set(WLC_SET_SSID, &ssid_t,
					sizeof(wlc_ssid_t))) < 0)
					return ret;
			}
		}
	}

	return ret;
}

int join_network_with_bssid(char* ssid, uint32 wsec, char *bssid)
{
#if !defined(WL_ASSOC_PARAMS_FIXED_SIZE) || !defined(WL_JOIN_PARAMS_FIXED_SIZE)
	return (join_network(ssid, wsec));
#else
	int ret = 0, retry;
	int auth = 0, infra = 1;
	int wpa_auth = WPA_AUTH_DISABLED;
	char associated_bssid[6];
	wl_join_params_t join_params;
	wlc_ssid_t *ssid_t = &join_params.ssid;
	wl_assoc_params_t *params_t = &join_params.params;

	printf("Joining network %s - %d\n", ssid, wsec);

	memset(&join_params, 0, sizeof(join_params));

	/*
	 * If wep bit is on,
	 * pick any WPA encryption type to allow association.
	 * Registration traffic itself will be done in clear (eapol).
	*/
	if (wsec)
		wsec = 2; /* TKIP */

	/* ssid */
	ssid_t->SSID_len = strlen(ssid);
	strncpy((char *)ssid_t->SSID, ssid, ssid_t->SSID_len);
	ssid_t->SSID[ssid_t->SSID_len] = '\0';

	/* bssid (if any) */
	if (bssid)
		memcpy(&params_t->bssid, bssid, ETHER_ADDR_LEN);
	else
		memcpy(&params_t->bssid, &ether_bcast, ETHER_ADDR_LEN);

	/* set infrastructure mode */
	if ((ret = wps_ioctl_set(WLC_SET_INFRA, &infra, sizeof(int))) < 0)
		return ret;

	/* set authentication mode */
	if ((ret = wps_ioctl_set(WLC_SET_AUTH, &auth, sizeof(int))) < 0)
		return ret;

	/* set wsec mode */
	if ((ret = wps_ioctl_set(WLC_SET_WSEC, &wsec, sizeof(int))) < 0)
		return ret;

	/* set WPA_auth mode */
	if ((ret = wps_ioctl_set(WLC_SET_WPA_AUTH, &wpa_auth, sizeof(wpa_auth))) < 0)
		return ret;

	/* set ssid */
	if ((ret = wps_ioctl_set(WLC_SET_SSID, &join_params, sizeof(wl_join_params_t))) == 0) {
		/* Poll for the results once a second until we got BSSID */
		for (retry = 0; retry < WPS_JOIN_MAX_WAIT_SEC; retry++) {

			WpsSleep(1000);

			ret = wps_ioctl_get(WLC_GET_BSSID, associated_bssid, 6);

			/* break out if the scan result is ready */
			if (ret == 0)
				break;

			if (retry != 0 && retry % 10 == 0) {
				if ((ret = wps_ioctl_set(WLC_SET_SSID, &join_params,
					sizeof(wl_join_params_t))) < 0)
					return ret;
			}
		}
	}

	return ret;
#endif /* !defined(WL_ASSOC_PARAMS_FIXED_SIZE) || !defined(WL_JOIN_PARAMS_FIXED_SIZE) */
}

int leave_network()
{
	return wps_ioctl_set(WLC_DISASSOC, NULL, 0);
}

int wps_get_bssid(char *bssid)
{
	return wps_ioctl_get(WLC_GET_BSSID, bssid, 6);
}

int wps_get_ssid(char *ssid, int *len)
{
	int ret;
	wlc_ssid_t wlc_ssid;

	if ((ret = wps_ioctl_get(WLC_GET_SSID, &wlc_ssid, sizeof(wlc_ssid))) < 0)
		return ret;

	*len = wlc_ssid.SSID_len;
	strncpy(ssid, (char*)wlc_ssid.SSID, *len);

	return ret;
}

int
wps_get_bands(uint *band_num, uint *active_band)
{
	int ret;
	uint list[3];

	*band_num = 0;
	*active_band = 0;

	if ((ret = wps_ioctl_get(WLC_GET_BANDLIST, list, sizeof(list))) < 0) {
		return ret;
	}

	/* list[0] is count, followed by 'count' bands */
	if (list[0] > 2)
		list[0] = 2;
	*band_num = list[0];

	/* list[1] is current band type */
	*active_band = list[1];

	return ret;
}

int
do_wpa_psk(WpsEnrCred* credential)
{
	int ret = 0, retry;
	wlc_ssid_t ssid_t;
	int auth = 0, infra = 1;
	int wpa_auth = WPA_AUTH_DISABLED;
	char bssid[6];
	uint8 wsec = 0;
	int sup_wpa;
	wl_wsec_key_t wlkey;
	wsec_pmk_t pmk;
	unsigned char *data = wlkey.data;
	char hex[] = "XX";
	char *keystr, keystr_buf[SIZE_64_BYTES+1];

	/* get SSID */
	ssid_t.SSID_len = credential->ssidLen;
	strncpy((char *)ssid_t.SSID, credential->ssid, ssid_t.SSID_len);
	ssid_t.SSID[ssid_t.SSID_len] = '\0';

	/* get auth */
	auth = (strstr(credential->keyMgmt, "SHARED")) ? 1 : 0;

	/* get wpa_auth */
	if (strstr(credential->keyMgmt, "WPA-PSK"))
		wpa_auth |= WPA_AUTH_PSK;

	if (strstr(credential->keyMgmt, "WPA2-PSK")) {
		/* Always use WPA2PSK when both WPAPSK and WPA2PSK enabled */
		if (wpa_auth & WPA_AUTH_PSK)
			wpa_auth &= ~WPA_AUTH_PSK;

		wpa_auth |= WPA2_AUTH_PSK;
	}

	/* get wsec */
	if (credential->encrType & ENCRYPT_WEP)
		wsec |= WEP_ENABLED;
	if (credential->encrType & ENCRYPT_TKIP)
		wsec |= TKIP_ENABLED;
	if (credential->encrType & ENCRYPT_AES)
		wsec |= AES_ENABLED;

	/* set infrastructure mode */
	if ((ret = wps_ioctl_set(WLC_SET_INFRA, &infra, sizeof(int))) < 0)
		return ret;

	/* set mac-layer auth */
	if ((ret = wps_ioctl_set(WLC_SET_AUTH, &auth, sizeof(int))) < 0)
		return ret;

	/* set wsec */
	if ((ret = wps_ioctl_set(WLC_SET_WSEC, &wsec, sizeof(int))) < 0)
		return ret;

	/* set upper-layer auth */
	if ((ret = wps_ioctl_set(WLC_SET_WPA_AUTH, &wpa_auth, sizeof(wpa_auth))) < 0)
		return ret;

	/* set in-driver supplicant */
	sup_wpa = ((wpa_auth & WPA_AUTH_PSK) == 0)? 0: 1;
	sup_wpa |= ((wpa_auth & WPA2_AUTH_PSK) == 0)? 0: 1;

	if ((ret = wps_iovar_set("sup_wpa", &sup_wpa, sizeof(sup_wpa))) < 0)
		return ret;

	/* set the key if wsec */
	if (wsec == WEP_ENABLED) {
		memset(&wlkey, 0, sizeof(wl_wsec_key_t));
		if (credential->wepIndex)
			wlkey.index = credential->wepIndex - 1;
		switch (credential->nwKeyLen) {
		/* ASIC */
		case 5:
		case 13:
		case 16:
			wlkey.len = credential->nwKeyLen;
			memcpy(data, credential->nwKey, wlkey.len + 1);
			break;
		case 10:
		case 26:
		case 32:
		case 64:
			wlkey.len = (credential->nwKeyLen) / 2;
			memcpy(keystr_buf, credential->nwKey, credential->nwKeyLen);
			keystr_buf[credential->nwKeyLen] = '\0';
			keystr = keystr_buf;
			while (*keystr) {
				strncpy(hex, keystr, 2);
				*data++ = (char) strtoul(hex, NULL, 16);
				keystr += 2;
			}
			break;
		default:
			return -1;
		}

		switch (wlkey.len) {
		case 5:
			wlkey.algo = CRYPTO_ALGO_WEP1;
			break;
		case 13:
			wlkey.algo = CRYPTO_ALGO_WEP128;
			break;
		case 16:
			/* default to AES-CCM */
			wlkey.algo = CRYPTO_ALGO_AES_CCM;
			break;
		case 32:
			wlkey.algo = CRYPTO_ALGO_TKIP;
			break;
		default:
			return -1;
		}

		/* Set as primary key by default */
		wlkey.flags |= WL_PRIMARY_KEY;

		if ((ret = wps_ioctl_set(WLC_SET_KEY, &wlkey, sizeof(wlkey))) < 0)
			return ret;
	}
	else if (wsec != 0) {
		memset(&pmk, 0, sizeof(wsec_pmk_t));
		if (credential->nwKeyLen < WSEC_MIN_PSK_LEN ||
			credential->nwKeyLen > WSEC_MAX_PSK_LEN) {
			printf("passphrase must be between %d and %d characters long\n",
				WSEC_MIN_PSK_LEN, WSEC_MAX_PSK_LEN);
			return -1;
		}

		pmk.key_len = credential->nwKeyLen;
		pmk.flags = WSEC_PASSPHRASE;
		strncpy((char *)pmk.key, credential->nwKey, credential->nwKeyLen);
		if ((ret = wps_ioctl_set(WLC_SET_WSEC_PMK, &pmk, sizeof(pmk))) < 0)
			return ret;
	}

	/* set ssid */
	if ((ret = wps_ioctl_set(WLC_SET_SSID, &ssid_t, sizeof(wlc_ssid_t))) == 0) {
		/* Poll for the results once a second until we got BSSID */
		for (retry = 0; retry < WPS_JOIN_MAX_WAIT_SEC; retry++) {

			WpsSleep(1000);

			ret = wps_ioctl_get(WLC_GET_BSSID, bssid, 6);

			/* break out if the scan result is ready */
			if (ret == 0)
				break;

			if (retry != 0 && retry % 10 == 0) {
				if ((ret = wps_ioctl_set(WLC_SET_SSID, &ssid_t,
					sizeof(wlc_ssid_t))) < 0)
					return ret;
			}
		}
	}

	return ret;

}

int
wps_ioctl_get(int cmd, void *buf, int len)
{
	return wps_wl_ioctl(cmd, buf, len, FALSE);
}

int
wps_ioctl_set(int cmd, void *buf, int len)
{
	return wps_wl_ioctl(cmd, buf, len, TRUE);
}

/*
 * set named iovar given the parameter buffer
 * iovar name is converted to lower case
 */
static int
wps_iovar_set(const char *iovar, void *param, int paramlen)
{
	char smbuf[WLC_IOCTL_SMLEN];

	memset(smbuf, 0, sizeof(smbuf));

	return wps_iovar_setbuf(iovar, param, paramlen, smbuf, sizeof(smbuf));
}

/*
 * set named iovar providing both parameter and i/o buffers
 * iovar name is converted to lower case
 */
static int
wps_iovar_setbuf(const char *iovar,
	void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	int iolen;

	iolen = wps_iovar_mkbuf(iovar, param, paramlen, bufptr, buflen, &err);
	if (err)
		return err;

	return wps_ioctl_set(WLC_SET_VAR, bufptr, iolen);
}

/*
 * format an iovar buffer
 * iovar name is converted to lower case
 */
static uint
wps_iovar_mkbuf(const char *name, char *data, uint datalen, char *iovar_buf, uint buflen, int *perr)
{
	uint iovar_len;
	char *p;

	iovar_len = strlen(name) + 1;

	/* check for overflow */
	if ((iovar_len + datalen) > buflen) {
		*perr = -1;
		return 0;
	}

	/* copy data to the buffer past the end of the iovar name string */
	if (datalen > 0)
		memmove(&iovar_buf[iovar_len], data, datalen);

	/* copy the name to the beginning of the buffer */
	strcpy(iovar_buf, name);

	/* wl command line automatically converts iovar names to lower case for
	 * ease of use
	 */
	p = iovar_buf;
	while (*p != '\0') {
		*p = tolower((int)*p);
		p++;
	}

	*perr = 0;
	return (iovar_len + datalen);
}
