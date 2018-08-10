/*
 * Application-specific portion of EAPD
 * (mirror brcm event for custmer)
 *
 * Copyright (C) 2018, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <proto/ethernet.h>
#include <proto/eapol.h>
#include <proto/eap.h>
#include <bcmendian.h>
#include <wlutils.h>
#include <eapd.h>
#include <shutils.h>
#include <UdpLib.h>
#include <security_ipc.h>
#include <bcmnvram.h>

void
mevent_app_recv_handler(eapd_wksp_t *nwksp, eapd_cb_t *from, uint8 *pData,
	int *pLen)
{

}

void
mevent_app_set_eventmask(eapd_app_t *app)
{
	memset(app->bitvec, 0, sizeof(app->bitvec));

	setbit(app->bitvec, WLC_E_ASSOC);
	setbit(app->bitvec, WLC_E_ASSOC_IND);
	setbit(app->bitvec, WLC_E_AUTH);
	setbit(app->bitvec, WLC_E_AUTH_IND);
	setbit(app->bitvec, WLC_E_DISASSOC);
	setbit(app->bitvec, WLC_E_DISASSOC_IND);
	setbit(app->bitvec, WLC_E_DEAUTH);
	setbit(app->bitvec, WLC_E_DEAUTH_IND);
	setbit(app->bitvec, WLC_E_PRUNE);
	setbit(app->bitvec, WLC_E_AUTHORIZED);
	setbit(app->bitvec, WLC_E_PROBREQ_MSG_RX);
	setbit(app->bitvec, WLC_E_PRE_ASSOC_IND);
	setbit(app->bitvec, WLC_E_PRE_REASSOC_IND);
	setbit(app->bitvec, WLC_E_PRE_ASSOC_RSEP_IND);
	return;
}

int
mevent_app_init(eapd_wksp_t *nwksp)
{
	int reuse = 1;
	eapd_mevent_t *mevent;
	eapd_cb_t *cb;
	struct sockaddr_in addr;

	if (nwksp == NULL)
		return -1;

	mevent = &nwksp->mevent;
	mevent->appSocket = -1;

	cb = mevent->cb;
	if (cb == NULL) {
		EAPD_INFO("No any mevent application need to run.\n");
		return 0;
	}

	while (cb) {
		EAPD_INFO("mevent: init brcm interface %s \n", cb->ifname);
		cb->brcmSocket = eapd_add_brcm(nwksp, cb->ifname);
		if (!cb->brcmSocket)
			return -1;
		/* set this brcmSocket have MEVENT capability */
		cb->brcmSocket->flag |= EAPD_CAP_MEVENT;

		cb = cb->next;
	}

	/* appSocket for mevent */
	mevent->appSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mevent->appSocket < 0) {
		EAPD_ERROR("UDP Open failed.\n");
		return -1;
	}
#if defined(__ECOS)
	if (setsockopt(mevent->appSocket, SOL_SOCKET, SO_REUSEPORT, (char*)&reuse,
		sizeof(reuse)) < 0) {
		EAPD_ERROR("UDP setsockopt failed.\n");
		close(mevent->appSocket);
		mevent->appSocket = -1;
		return -1;
	}
#else
	if (setsockopt(mevent->appSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse,
		sizeof(reuse)) < 0) {
		EAPD_ERROR("UDP setsockopt failed.\n");
		close(mevent->appSocket);
		mevent->appSocket = -1;
		return -1;
	}
#endif /* __ECOS */

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = EAPD_UDP_SIN_ADDR;
	addr.sin_port = htons(EAPD_WKSP_MEVENT_UDP_RPORT);
	if (bind(mevent->appSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		EAPD_ERROR("UDP Bind failed, close mevent appSocket %d\n", mevent->appSocket);
		close(mevent->appSocket);
		mevent->appSocket = -1;
		return -1;
	}
	EAPD_INFO("MEVENT appSocket %d opened\n", mevent->appSocket);

	return 0;
}

int
mevent_app_deinit(eapd_wksp_t *nwksp)
{
	eapd_mevent_t *mevent;
	eapd_cb_t *cb, *tmp_cb;

	if (nwksp == NULL) {
		EAPD_ERROR("Wrong argument...\n");
		return -1;
	}

	mevent = &nwksp->mevent;
	cb = mevent->cb;
	while (cb) {
		/* close  brcm drvSocket */
		if (cb->brcmSocket) {
			EAPD_INFO("close mevent brcmSocket %d\n", cb->brcmSocket->drvSocket);
			eapd_del_brcm(nwksp, cb->brcmSocket);
		}

		tmp_cb = cb;
		cb = cb->next;
		free(tmp_cb);
	}

	/* close  appSocke */
	if (mevent->appSocket >= 0) {
		EAPD_INFO("close mevent appSocket %d\n", mevent->appSocket);
		close(mevent->appSocket);
		mevent->appSocket = -1;
	}

	return 0;
}

int
mevent_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *from)
{
	eapd_mevent_t *mevent;

	if (nwksp == NULL) {
		EAPD_ERROR("Wrong argument...\n");
		return -1;
	}

	mevent = &nwksp->mevent;
	if (mevent->appSocket >= 0) {
		/* send to mevent */
		int sentBytes = 0;
		struct sockaddr_in to;

		to.sin_addr.s_addr = inet_addr(EAPD_WKSP_UDP_ADDR);
		to.sin_family = AF_INET;
		to.sin_port = htons(EAPD_WKSP_MEVENT_UDP_SPORT);

		sentBytes = sendto(mevent->appSocket, pData, pLen, 0,
			(struct sockaddr *)&to, sizeof(struct sockaddr_in));

		if (sentBytes != pLen) {
			EAPD_ERROR("UDP send failed; sentBytes = %d\n", sentBytes);
		}
		else {
			/* EAPD_ERROR("Send %d bytes to mevent\n", sentBytes); */
		}
	}
	else {
		EAPD_ERROR("mevent appSocket not created\n");
	}
	return 0;
}

#if EAPD_WKSP_AUTO_CONFIG
int
mevent_app_enabled(char *name)
{
	char value[128], comb[32],  prefix[8];
	char os_name[IFNAMSIZ];
	int unit;

	memset(os_name, 0, sizeof(os_name));

	if (nvifname_to_osifname(name, os_name, sizeof(os_name)))
		return 0;
	if (wl_probe(os_name) ||
		wl_ioctl(os_name, WLC_GET_INSTANCE, &unit, sizeof(unit)))
		return 0;
	if (osifname_to_nvifname(name, prefix, sizeof(prefix)))
		return 0;

	strcat(prefix, "_");
	/* ignore if disabled */
	eapd_safe_get_conf(value, sizeof(value), strcat_r(prefix, "radio", comb));
	if (atoi(value) == 0) {
		EAPD_INFO("MEVENT:ignored interface %s. radio disabled\n", os_name);
		return 0;
	}

	/* ignore if BSS is disabled */
	eapd_safe_get_conf(value, sizeof(value), strcat_r(prefix, "bss_enabled", comb));
	if (atoi(value) == 0) {
		EAPD_INFO("MEVENT: ignored interface %s, %s is disabled \n", os_name, comb);
		return 0;
	}

	/* if come to here return enabled */
	return 1;
}
#endif /* EAPD_WKSP_AUTO_CONFIG */

void
mevent_split_assoc_handle(int type, wl_event_msg_t *event)
{
	uchar *buf;
	assoc_decision_t *dc;
	uint8 *addr = (uint8 *)&(event->addr);
	char ea[ETHER_ADDR_STR_LEN];
	int total_len = 0;
	char *str;

	if (type == WLC_E_PRE_ASSOC_IND || type == WLC_E_PRE_REASSOC_IND ||
		type == WLC_E_PRE_ASSOC_RSEP_IND) {
		total_len = sizeof(assoc_decision_t) + WLC_DC_DWDS_DATA_LENGTH - 1;
		buf = malloc(total_len);
		if (buf == NULL) {
			EAPD_ERROR("assoc decision IE allocate failed...\n");
			return;
		}
		memset(buf, 0, total_len);
		dc = (assoc_decision_t *) buf;
		bcopy(addr, &dc->da, ETHER_ADDR_LEN);
		if (nvram_match("split_assoc_reject", "1")) {
			dc->assoc_approved = FALSE;
			dc->reject_reason = DOT11_SC_ASSOC_FAIL;
		} else {
			dc->assoc_approved = TRUE;
			dc->reject_reason = 0;
			str = nvram_safe_get("split_assoc_dwds");
			if (strcmp(str, "")) {
				dc->dc_info.len = 3;
				dc->dc_info.tlv[0].type = WLC_DC_DWDS;
				dc->dc_info.tlv[0].len = WLC_DC_DWDS_DATA_LENGTH;
				dc->dc_info.tlv[0].data[0] = (!strcmp(str, "1")) ?
				WL_ASSOC_DC_DWDS_ENABLE : WL_ASSOC_DC_DWDS_DISABLE;
			}
		}

		EAPD_EVENT("MEVENT: ifname(%s) addr(%s) association %s \n",
			event->ifname, ether_etoa(&event->addr, ea),
			dc->assoc_approved ? "ACCEPT" : "REJECT");

		if (wl_iovar_set(event->ifname, "assoc_decision", dc, total_len)) {
			EAPD_ERROR("%s: assoc_decision failed \n", __FUNCTION__);
		}
		free(buf);
	}
}

int
mevent_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from)
{
	int type;
	eapd_mevent_t *mevent;
	eapd_cb_t *cb;
	bcm_event_t *dpkt = (bcm_event_t *) pData;
	wl_event_msg_t *event = &(dpkt->event);
	type = ntohl(event->event_type);
	mevent = &nwksp->mevent;
	cb = mevent->cb;
	while (cb) {
		if (isset(mevent->bitvec, type) &&
			!strcmp(cb->ifname, from)) {

			/* prepend ifname,  we reserved IFNAMSIZ length already */
			pData -= IFNAMSIZ;
			Len += IFNAMSIZ;
			memcpy(pData, event->ifname, IFNAMSIZ);
			if (nvram_match("mevent_handle_split_assoc", "1")) {
				mevent_split_assoc_handle(type, event);
			}
			/* send to mevent use cb->ifname */
			mevent_app_sendup(nwksp, pData, Len, cb->ifname);
			break;
		}
		cb = cb->next;
	}

	return 0;
}
