/*
 * WPS station
 *
 * Copyright (C) 2018, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wps_sta.h 715747 2017-08-14 10:47:34Z $
 */
#ifndef __WPS_STA_H__
#define __WPS_STA_H__
#include <wlioctl.h>
#define WPS_MAX_AP_SCAN_LIST_LEN 50
#define PBC_WALK_TIME 120

wps_ap_list_info_t *wps_get_ap_list(void);
wps_ap_list_info_t *create_aplist(void);
int add_wps_ie(unsigned char *p_data, int length, bool pbc, bool b_wps_version2);
int rem_wps_ie(unsigned char *p_data, int length, unsigned int pktflag);
int join_network(char* ssid, uint32 wsec);
int leave_network(void);
int wps_get_bssid(unsigned char *bssid);
int wps_get_ssid(char *ssid, int *len);
int wps_get_bands(uint *band_num, uint *active_band);
int do_wpa_psk(WpsEnrCred* credential);
int join_network_with_bssid(char* ssid, uint32 wsec, unsigned char *bssid);
int do_wps_scan(void);
char* get_wps_scan_results(void);
uint32 wps_eap_parse_scan_result(wl_event_msg_t* event, char* ifname);
void wpssta_display_aplist_set(uint8 *aplist_diplay, bool set);

/* BEGIN escan patch */
int do_wps_escan(void);
char* get_wps_escan_results(void);
wps_ap_list_info_t *create_aplist_escan(void);
/* END escan patch */
#if defined(MULTIAP)
int do_map_wps_escan(char *ssid);
int do_map_wps_scan(char *ssid);
#endif	/* MULTIAP */

#ifdef WFA_WPS_20_TESTBED
int set_wps_ie_frag_threshold(int threshold);
int set_update_partial_ie(uint8 *updie_str, unsigned int pktflag);
#endif /* WFA_WPS_20_TESTBED */

bool wps_wl_init(void *caller_ctx, void *callback);
void wps_wl_deinit();
wps_ap_list_info_t *wps_wl_surveying(bool b_pbc, bool b_v2, bool b_add_wpsie);
bool wps_wl_join(uint8 *bssid, char *ssid, uint8 wep);
#ifdef BCMWPSAPSTA
int wps_escan_timeout_handler(uint32 timout_state);
uint32 wps_eap_reset_scan_result();
uint32 get_wps_escan_state(void);
#endif
int wpssta_display_aplist(wps_ap_list_info_t *ap);
/* Escan timeout value in seconds */
#define ESCAN_TIMER_INTERVAL_S 10
/* wps escan States */
typedef enum wps_escan_state_t {
	WPS_ESCAN_NOT_STARTED = 0,	/* default */
	WPS_ESCAN_INPROGRESS = 1,	/* The wps Escan is in Progress */
	WPS_ESCAN_DONE = 2,	/* The wps Escan Done */
	WPS_ESCAN_TIMEDOUT = 3
} wps_escan_state_t;

/* wps escan Timeout set variables */
typedef enum wps_escan_timeout_state_t {
	WPS_ESCAN_STARTED = 0,        /**< default */
	WPS_ESCAN_CHECK_TIMEOUT = 1    /**< The wps Escan is in Progress */
} wps_escan_timeout_state_t;

struct escan_bss {
        struct escan_bss *next;
        wl_bss_info_t bss[1];
};
#endif /* __WPS_STA_H__ */
