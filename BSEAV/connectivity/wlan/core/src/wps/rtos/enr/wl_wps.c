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

#include <portability.h>
#include <wps_enrapi.h>
#include <wps_sta.h>
#include "wlioctl.h"
#include <wps_staeapsm.h>
#include <wpserror.h>

int tolower(int);
int wps_wl_ioctl(int cmd, void *buf, int len, bool set);
static int wl_iovar_get(char *iovar, void *bufptr, int buflen);
static int wps_iovar_set(const char *iovar, void *param, int paramlen);
static int wl_iovar_getbuf(char *iovar, void *param, int paramlen, void *bufptr, int buflen);
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
#define WPS_IE_BUF_LEN	VNDR_IE_MAX_LEN * 8	/* 2048 */
#define WPS_IE_FRAG_MAX (WLC_IOCTL_SMLEN - sizeof(vndr_ie_setbuf_t) - strlen("vndr_ie") - 1)

wps_ap_list_info_t ap_list[WPS_MAX_AP_SCAN_LIST_LEN];
static char scan_result[WPS_DUMP_BUF_LEN];
static uint8 wps_ie_setbuf[WPS_IE_BUF_LEN];

#ifdef WFA_WPS_20_TESTBED
static int frag_threshold = 0;
static uint8 prbreq_updie_setbuf[WPS_IE_BUF_LEN];
static uint8 prbreq_updie_len = 0;
static uint8 assocreq_updie_setbuf[WPS_IE_BUF_LEN];
static uint8 assocreq_updie_len = 0;

int
set_wps_ie_frag_threshold(int threshold)
{
	/* 72 = OUI + OUITYPE + TLV, V <=64 case */
	if (threshold > WPS_IE_FRAG_MAX || threshold < 72)
		return -1;

	frag_threshold = threshold;

	return 0;
}

int
set_update_partial_ie(uint8 *updie_str, unsigned int pktflag)
{
	uchar *src, *dest;
	uchar val;
	int idx, len;
	char hexstr[3];

	uint8 *updie_setbuf;
	uint8 *updie_len;

	switch (pktflag) {
	case VNDR_IE_PRBREQ_FLAG:
		updie_setbuf = prbreq_updie_setbuf;
		updie_len = &prbreq_updie_len;
		break;
	case VNDR_IE_ASSOCREQ_FLAG:
		updie_setbuf = assocreq_updie_setbuf;
		updie_len = &assocreq_updie_len;
		break;
	default:
		return -1;
	}
	/* reset first */
	*updie_len = 0;

	if (!updie_str)
		return 0;

	/* Ensure in 2 characters long */
	len = strlen((char*)updie_str);
	if (len % 2) {
		printf("Please specify all the data bytes for this IE\n");
		return -1;
	}
	*updie_len = (uint8) (len / 2);

	/* string to hex */
	src = updie_str;
	dest = updie_setbuf;
	for (idx = 0; idx < len; idx++) {
		hexstr[0] = src[0];
		hexstr[1] = src[1];
		hexstr[2] = '\0';

		val = (uchar) strtoul(hexstr, NULL, 16);

		*dest++ = val;
		src += 2;
	}

	return 0;
}
#endif /* WFA_WPS_20_TESTBED */

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
		WpsSleep(1);

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

	if (list->version > WL_BSS_INFO_VERSION)
	{
		fprintf(stderr, "your driver has bss_info_version %d "
			"but this program supports only version %d.\n",
			list->version, WL_BSS_INFO_VERSION);
	}

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


#ifdef _TUDEBUGTRACE
static char *
_pktflag_name(unsigned int pktflag)
{
	if (pktflag == VNDR_IE_BEACON_FLAG)
		return "Beacon";
	else if (pktflag == VNDR_IE_PRBRSP_FLAG)
		return "Probe Resp";
	else if (pktflag == VNDR_IE_ASSOCRSP_FLAG)
		return "Assoc Resp";
	else if (pktflag == VNDR_IE_AUTHRSP_FLAG)
		return "Auth Resp";
	else if (pktflag == VNDR_IE_PRBREQ_FLAG)
		return "Probe Req";
	else if (pktflag == VNDR_IE_ASSOCREQ_FLAG)
		return "Assoc Req";
	else if (pktflag == VNDR_IE_CUSTOM_FLAG)
		return "Custom";
	else
		return "Unknown";
}
#endif /* _TUDEBUGTRACE */

static int
_del_vndr_ie(char *bufaddr, int buflen, uint32 frametype)
{
	int iebuf_len;
	int iecount, err;
	vndr_ie_setbuf_t *ie_setbuf;
#ifdef _TUDEBUGTRACE
	int i;
	int frag_len = buflen - 6;
	unsigned char *frag = (unsigned char *)(bufaddr + 6);
#endif

	iebuf_len = buflen + sizeof(vndr_ie_setbuf_t) - sizeof(vndr_ie_t);
	ie_setbuf = (vndr_ie_setbuf_t *) malloc(iebuf_len);
	if (!ie_setbuf) {
		printf("memory alloc failure\n");
		return -1;
	}

	/* Copy the vndr_ie SET command ("add"/"del") to the buffer */
	strcpy(ie_setbuf->cmd, "del");

	/* Buffer contains only 1 IE */
	iecount = 1;
	memcpy((void *)&ie_setbuf->vndr_ie_buffer.iecount, &iecount, sizeof(int));
	memcpy((void *)&ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].pktflag, &frametype, sizeof(uint32));
	memcpy(&ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data, bufaddr, buflen);

#ifdef _TUDEBUGTRACE
	printf("\n_del_vndr_ie (%s, frag_len=%d)\n", _pktflag_name(frametype), frag_len);
	for (i = 0; i < frag_len; i++) {
		if (i && !(i%16))
			printf("\n");
		printf("%02x ", frag[i]);
	}
	printf("\n");
#endif

	err = wps_iovar_set("vndr_ie", ie_setbuf, iebuf_len);

	free(ie_setbuf);

	return err;
}

static int
_set_vndr_ie(unsigned char *frag, int frag_len, unsigned char ouitype, unsigned int pktflag)
{
	vndr_ie_setbuf_t *ie_setbuf;
	int buflen, iecount, i;
	int err = 0;

	buflen = sizeof(vndr_ie_setbuf_t) + frag_len;
	ie_setbuf = (vndr_ie_setbuf_t *) malloc(buflen);
	if (!ie_setbuf) {
		printf("memory alloc failure\n");
		return -1;
	}

	/* Copy the vndr_ie SET command ("add"/"del") to the buffer */
	strcpy(ie_setbuf->cmd, "add");

	/* Buffer contains only 1 IE */
	iecount = 1;
	memcpy((void *)&ie_setbuf->vndr_ie_buffer.iecount, &iecount, sizeof(int));

	/* 
	 * The packet flag bit field indicates the packets that will
	 * contain this IE
	 */
	memcpy((void *)&ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].pktflag, &pktflag, sizeof(uint32));

	/* Now, add the IE to the buffer, +1: one byte OUI_TYPE */
	ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.len = (uint8) frag_len +
		VNDR_IE_MIN_LEN + 1;

	ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.oui[0] = 0x00;
	ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.oui[1] = 0x50;
	ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.oui[2] = 0xf2;
	ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.data[0] = ouitype;

	for (i = 0; i < frag_len; i++) {
		ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.data[i+1] = frag[i];
	}

#ifdef _TUDEBUGTRACE
	printf("\n_set_vndr_ie (%s, frag_len=%d)\n", _pktflag_name(pktflag), frag_len);
	for (i = 0; i < frag_len; i++) {
		if (i && !(i%16))
			printf("\n");
		printf("%02x ", frag[i]);
	}
	printf("\n");
#endif
	err = wps_iovar_set("vndr_ie", ie_setbuf, buflen);

	free(ie_setbuf);

	return err;
}

/* Parsing TLV format WPS IE */
static unsigned char *
_get_frag_wps_ie(unsigned char *p_data, int length, int *frag_len, int max_frag_len)
{
	int next_tlv_len, total_len = 0;
	uint16 type;
	unsigned char *next;

	if (!p_data || !frag_len || max_frag_len < 4)
		return NULL;

	if (length <= max_frag_len) {
		*frag_len = length;
		return p_data;
	}

	next = p_data;
	while (1) {
		type = WpsNtohs(next);
		next += 2; /* Move to L */
		next_tlv_len = WpsNtohs(next) + 4; /* Include Type and Value 4 bytes */
		next += 2; /* Move to V */
		if (next_tlv_len > max_frag_len) {
			printf("Error, there is a TLV length %d bigger than "
				"Max fragment length %d. Unable to fragment it.\n",
				next_tlv_len, max_frag_len);
			return NULL;
		}

		/* Abnormal IE check */
		if ((total_len + next_tlv_len) > length) {
			printf("Error, Abnormal WPS IE.\n");
			*frag_len = length;
			return p_data;
		}

		/* Fragment point check */
		if ((total_len + next_tlv_len) > max_frag_len) {
			*frag_len = total_len;
			return p_data;
		}

		/* Get this TLV length */
		total_len += next_tlv_len;
		next += (next_tlv_len - 4); /* Move to next TLV */
	}

}

int create_wps_ie(bool pbc, unsigned int pktflag)
{
	int err = 0;

	if (pktflag == VNDR_IE_PRBREQ_FLAG) {
		if (pbc)
			err = wps_build_pbc_proberq(wps_ie_setbuf, sizeof(wps_ie_setbuf));
		else
			err = wps_build_def_proberq(wps_ie_setbuf, sizeof(wps_ie_setbuf));
	}
	else if (pktflag == VNDR_IE_ASSOCREQ_FLAG) {
		err = wps_build_def_assocrq(wps_ie_setbuf, sizeof(wps_ie_setbuf));
	}
	else
		return -1;

	if (err == WPS_SUCCESS)
		err = 0;

	return err;
}

/* add probe request for enrollee */
static int
_add_wps_ie(bool pbc, unsigned int pktflag)
{
	int frag_len;
	int wps_ie_len;
	int err = 0;
	unsigned char *frag, *wps_ie;
	int frag_max = WPS_IE_FRAG_MAX;
#ifdef WFA_WPS_20_TESTBED
	uint8 *updie_setbuf;
	uint8 *updie_len;
#endif /* WFA_WPS_20_TESTBED */


	if (pktflag != VNDR_IE_PRBREQ_FLAG && pktflag != VNDR_IE_ASSOCREQ_FLAG) {
#ifdef _TUDEBUGTRACE
		printf("_add_wps_ie : unsupported pktflag 0x%x\n", pktflag);
#endif
		return -1;
	}

	/* Generate wps_ie_setbuf */
	if (create_wps_ie(pbc, pktflag) != 0) {
#ifdef _TUDEBUGTRACE
		printf("_add_wps_ie : Create WPS IE failed\n");
#endif
		return -1;
	}

	/*
	 * wps_ie_setbuf:
	 * [0] = len
	 * [1~3] = 0x00:0x50:0xF2
	 * [4] = 0x04
	 */
	wps_ie = &wps_ie_setbuf[5];
	wps_ie_len = wps_ie_setbuf[0] - 4;

	/* try removing first in case there was one left from previous call */
	rem_wps_ie(NULL, 0, pktflag);

#ifdef WFA_WPS_20_TESTBED
	/* WPS IE fragment threshold */
	if (frag_threshold)
		frag_max = frag_threshold;


	if (pktflag == VNDR_IE_PRBREQ_FLAG) {
		updie_setbuf = prbreq_updie_setbuf;
		updie_len = &prbreq_updie_len;
	}
	else {
		/* VNDR_IE_ASSOCREQ_FLAG */
		updie_setbuf = assocreq_updie_setbuf;
		updie_len = &assocreq_updie_len;
	}

	/* Update partial WPS IE in probe request */
	if (*updie_len) {
		if (wps_update_partial_ie(wps_ie_setbuf, sizeof(wps_ie_setbuf),
			wps_ie, (uint8)wps_ie_len, updie_setbuf, *updie_len) != WPS_SUCCESS) {
			printf("Failed to update partial WPS IE in %s\n",
				(pktflag == VNDR_IE_PRBREQ_FLAG) ? "probereq" : "assocreq");
			return -1;
		}
		/* update new length */
		wps_ie_len = wps_ie_setbuf[0] - 4;
	}
#endif /* WFA_WPS_20_TESTBED */

	/* Separate a big IE to fragment IEs */
	frag = wps_ie;
	frag_len = wps_ie_len;
	while (wps_ie_len > 0) {
		if (wps_ie_len > frag_max)
			/* Find a appropriate fragment point */
			frag = _get_frag_wps_ie(frag, wps_ie_len, &frag_len, frag_max);

		if (!frag)
			return -1;

		/* Set fragment WPS IE */
		err |= _set_vndr_ie(frag, frag_len, 0x04, pktflag);

		/* Move to next */
		wps_ie_len -= frag_len;
		frag += frag_len;
		frag_len = wps_ie_len;
	}

	return (err);
}

/* add probe request for enrollee */
int add_wps_ie(unsigned char *p_data, int length, bool pbc, bool b_wps_version2)
{
	int err = 0;

	/* Add WPS IE in probe request */
	if ((err = _add_wps_ie(pbc, VNDR_IE_PRBREQ_FLAG)) != 0) {
#ifdef _TUDEBUGTRACE
		printf("add_wps_ie : Add WPS IE in probe request failed\n");
#endif
		return err;
	}

	/* Add WPS IE in associate request */
	if (b_wps_version2 && (err = _add_wps_ie(pbc, VNDR_IE_ASSOCREQ_FLAG)) != 0) {
#ifdef _TUDEBUGTRACE
		printf("add_wps_ie : Add WPS IE in associate request failed\n");
#endif
		return err;
	}

	return 0;
}

/* Remove probe request WPS IEs */
int rem_wps_ie(unsigned char *p_data, int length, unsigned int pktflag)
{
	int i, err = 0;
	char getbuf[WPS_IE_BUF_LEN] = {0};
	vndr_ie_buf_t *iebuf;
	vndr_ie_info_t *ieinfo;
	char wps_oui[4] = {0x00, 0x50, 0xf2, 0x04};
	char *bufaddr;
	int buflen = 0;
	int found = 0;
	uint32 ieinfo_pktflag;

	if (pktflag != VNDR_IE_PRBREQ_FLAG && pktflag != VNDR_IE_ASSOCREQ_FLAG) {
#ifdef _TUDEBUGTRACE
		printf("rem_wps_ie : unsupported pktflag 0x%x\n", pktflag);
#endif
		return -1;
	}

	/* Get all WPS IEs in probe request IE */
	if (wl_iovar_get("vndr_ie", getbuf, WPS_IE_BUF_LEN)) {
#ifdef _TUDEBUGTRACE
		printf("rem_wps_ie : No IE to remove\n");
#endif
		return -1;
	}

	iebuf = (vndr_ie_buf_t *) getbuf;
	bufaddr = (char*) iebuf->vndr_ie_list;

	/* Delete ALL specified ouitype IEs */
	for (i = 0; i < iebuf->iecount; i++) {
		ieinfo = (vndr_ie_info_t*) bufaddr;
		bcopy((char*)&ieinfo->pktflag, (char*)&ieinfo_pktflag, (int) sizeof(uint32));
		if (ieinfo_pktflag == pktflag) {
			if (!memcmp(ieinfo->vndr_ie_data.oui, wps_oui, 4)) {
				found = 1;
				bufaddr = (char*) &ieinfo->vndr_ie_data;
				buflen = (int)ieinfo->vndr_ie_data.len + VNDR_IE_HDR_LEN;
				/* Delete one vendor IE */
				err |= _del_vndr_ie(bufaddr, buflen, pktflag);
			}
		}
		bufaddr = (char*)(ieinfo->vndr_ie_data.oui + ieinfo->vndr_ie_data.len);
	}

	if (!found)
		return -1;

	return (err);
}

int join_network(char* ssid, uint32 wsec)
{
	int ret = 0, retry;
	wlc_ssid_t ssid_t;
	int auth = 0, infra = 1;
	int wpa_auth = WPA_AUTH_DISABLED;
	char bssid[6];
	uint wsec_ext;

#ifdef _TUDEBUGTRACE
	printf("Joining network %s - %d\n", ssid, wsec);
#endif
	/*
	 * If wep bit is on,
	 * pick any WPA encryption type to allow association.
	 * Registration traffic itself will be done in clear (eapol).
	*/
	if (wsec)
		wsec = 4; /* AES */
	ssid_t.SSID_len = strlen(ssid);
	strncpy((char *)ssid_t.SSID, ssid, ssid_t.SSID_len);

	/* set infrastructure mode */
	if ((ret = wps_ioctl_set(WLC_SET_INFRA, &infra, sizeof(int))) < 0)
		return ret;

	/* set authentication mode */
	if ((ret = wps_ioctl_set(WLC_SET_AUTH, &auth, sizeof(int))) < 0)
		return ret;

	/* set wsec mode */
	wsec_ext = (uint)wsec;
	if ((ret = wps_ioctl_set(WLC_SET_WSEC, &wsec_ext, sizeof(int))) < 0)
		return ret;

	/* set WPA_auth mode */
	if ((ret = wps_ioctl_set(WLC_SET_WPA_AUTH, &wpa_auth, sizeof(wpa_auth))) < 0)
		return ret;

	/* set ssid */
	if ((ret = wps_ioctl_set(WLC_SET_SSID, &ssid_t, sizeof(wlc_ssid_t))) == 0) {
		/* Poll for the results once a second until we got BSSID */
		for (retry = 0; retry < WPS_JOIN_MAX_WAIT_SEC; retry++) {
			WpsSleep(1);

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
	uint wsec_ext;

	printf("Joining network %s - %d\n", ssid, wsec);

	memset(&join_params, 0, sizeof(join_params));

	/*
	 * If wep bit is on,
	 * pick any WPA encryption type to allow association.
	 * Registration traffic itself will be done in clear (eapol).
	*/
	if (wsec)
		wsec = 4; /* AES */

	/* ssid */
	ssid_t->SSID_len = strlen(ssid);
	strncpy((char *)ssid_t->SSID, ssid, ssid_t->SSID_len);

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
	wsec_ext = (uint)wsec;
	if ((ret = wps_ioctl_set(WLC_SET_WSEC, &wsec_ext, sizeof(int))) < 0)
		return ret;

	/* set WPA_auth mode */
	if ((ret = wps_ioctl_set(WLC_SET_WPA_AUTH, &wpa_auth, sizeof(wpa_auth))) < 0)
		return ret;

	/* set ssid */
	if ((ret = wps_ioctl_set(WLC_SET_SSID, &join_params, sizeof(wl_join_params_t))) == 0) {
		/* Poll for the results once a second until we got BSSID */
		for (retry = 0; retry < WPS_JOIN_MAX_WAIT_SEC; retry++) {
			WpsSleep(1);

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
	/* Add in PF #3, zero padding in SSID */
	if (strlen(credential->ssid) == (credential->ssidLen - 1))
		credential->ssidLen--;

	ssid_t.SSID_len = credential->ssidLen;
	strncpy((char *)ssid_t.SSID, credential->ssid, ssid_t.SSID_len);

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

	/* Add in PF#3, use AES when encryptoin type in mixed-mode */
	if (wsec == (TKIP_ENABLED | AES_ENABLED))
		wsec &= ~TKIP_ENABLED;

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

		if (strlen(credential->nwKey) == (credential->nwKeyLen - 1))
			credential->nwKeyLen--;

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
			WpsSleep(1);

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

static int
wl_iovar_get(char *iovar, void *bufptr, int buflen)
{
	char smbuf[WLC_IOCTL_SMLEN];
	int ret;

	/* use the return buffer if it is bigger than what we have on the stack */
	if (buflen > sizeof(smbuf)) {
		ret = wl_iovar_getbuf(iovar, NULL, 0, bufptr, buflen);
	} else {
		ret = wl_iovar_getbuf(iovar, NULL, 0, smbuf, sizeof(smbuf));
		if (ret == 0)
			memcpy(bufptr, smbuf, buflen);
	}

	return ret;
}

static int
wl_iovar_getbuf(char *iovar, void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	uint namelen;
	uint iolen;

	namelen = strlen(iovar) + 1;	 /* length of iovar name plus null */
	iolen = namelen + paramlen;

	/* check for overflow */
	if (iolen > buflen)
		return (-1);

	memcpy(bufptr, iovar, namelen);	/* copy iovar name including null */
	memcpy((int8*)bufptr + namelen, param, paramlen);

	err = wps_ioctl_set(WLC_GET_VAR, bufptr, buflen);

	return (err);
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
