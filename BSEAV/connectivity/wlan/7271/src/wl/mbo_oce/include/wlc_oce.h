/*
 * OCE declarations/definitions for
 * Broadcom 802.11abgn Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 *
 */

/**
 * WFA Optimized Connectivity Experience (OCE) certification program certifies features
 * that deliver a better overall connectivity experience by taking advantage of systemic
 * information available within planned and managed networks (e.g. hotspot, workplace,
 * operator-deployed networks). The program is intended to address issues identified by
 * operators including very long connection setup times, poor wireless local area network (WLAN)
 * connectivity, and spectrum consumed by management frames.
 */

#ifndef _wlc_oce_h_
#define _wlc_oce_h_

#include <proto/mbo_oce.h>
#include <proto/oce.h>
#ifdef WL_OCE_AP
/* According to WFA OCE TechSpec The RSSI min threshold value shall be
 * within the range of -60 to -90 dBm.
 */
#define OCE_ASS_REJ_DEF_RSSI_THD		(-70)
#define OCE_ASSOC_REJECT_DEF_RETRY_DELAY	30
#define OCE_ASSOC_REJECT_DEF_RSSI_DELTA		2

/* Set default values to max for Uplink and Downlink from Reduced WAN Metrics */
#define OCE_REDUCED_WAN_METR_DEF_UPLINK_CAP	15
#define OCE_REDUCED_WAN_METR_DEF_DOWNLINK_CAP	15
#endif /* WL_OCE_AP */

wlc_oce_info_t * wlc_oce_attach(wlc_info_t *wlc);
void wlc_oce_detach(wlc_oce_info_t *oce);
bool wlc_oce_send_probe(wlc_oce_info_t *oce, void *p);
uint8 wlc_oce_get_probe_defer_time(wlc_oce_info_t *oce);
void wlc_oce_flush_prb_suppress_bss_list(wlc_oce_info_t *oce);
bool wlc_oce_is_oce_environment(wlc_oce_info_t *oce);
uint8 wlc_oce_get_pref_channels(chanspec_t *chanspec_list);
bool wlc_oce_is_pref_channel(chanspec_t chanspec);
void wlc_oce_set_max_channel_time(wlc_oce_info_t *oce, uint16 time);
int wlc_oce_recv_fils(wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg,
	uint action_id, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_management_header *hdr, uint8 *body, int body_len);
int wlc_oce_parse_fils_discovery(wlc_oce_info_t *oce, wlc_d11rxhdr_t *wrxh,
	struct ether_addr *bssid, uint8 *body, uint body_len, wlc_bss_info_t *bi);
void wlc_oce_process_assoc_reject(wlc_bsscfg_t *cfg, struct scb *scb,
	uint16 fk, uint8 *body, uint body_len);
void oce_calc_join_pref(wlc_bsscfg_t *cfg, wlc_bss_info_t **bip, uint bss_cnt,
	join_pref_t *join_pref);
uint16 wlc_oce_if_valid_assoc(wlc_oce_info_t *oce, int8 *rssi, uint8 *
rej_rssi_delta);
wifi_oce_cap_ind_attr_t* wlc_oce_find_cap_ind_attr(uint8 *parse, uint16 buf_len);
wifi_oce_probe_suppress_bssid_attr_t* wlc_oce_get_prb_suppr_bssid_attr(uint8 *parse, uint16 buf_len);
wifi_oce_probe_suppress_ssid_attr_t* wlc_oce_get_prb_suppr_ssid_attr(uint8 *parse, uint16 buf_len);
void wlc_oce_reset_rwan_statcs(wlc_bsscfg_t *cfg);
bool wlc_oce_get_trigger_rwan_roam(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
#if defined(WL_OCE_AP) && !defined(WLMCNX)
void wlc_oce_pretbtt_fd_callback(wlc_oce_info_t *oce);
#endif
#endif	/* _wlc_oce_h_ */
