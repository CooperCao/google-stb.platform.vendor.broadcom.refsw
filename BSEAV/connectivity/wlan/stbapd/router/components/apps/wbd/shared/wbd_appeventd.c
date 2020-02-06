/*
 * WBD Appeventd routine handler file
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wbd_appeventd.c 764064 2018-05-23 13:09:46Z cb888957 $
 */

#include "wbd.h"
#include "bcm_steering.h"
#include "appeventd_wbd.h"

#define WBD_APPEVENT_BUFSIZE	1024

/* Copy device_id, init status, into buf and send as event data. */
void
wbd_appeventd_map_init(int evt, struct ether_addr *device_id, int status)
{
	unsigned char app_data[WBD_APPEVENT_BUFSIZE] = {0};
	app_event_wbd_map_init_t *map_init = NULL;

	/* Prepare event data. */
	map_init = (app_event_wbd_map_init_t*)app_data;
	eacopy(device_id, &(map_init->device_id));
	map_init->status = status; /* Start = 1, end = 2 */

	app_event_sendup(evt, APP_E_WBD_STATUS_SUCCESS,
		app_data, sizeof(*map_init));
}

/* Copy ifname, sta_mac, rssi, tx_failures, and tx_rate into buf and send as event data. */
void
wbd_appeventd_weak_sta(int evt, char *ifname, struct ether_addr *sta_mac, int rssi,
	uint32 tx_failures, float tx_rate)
{
	unsigned char app_data[WBD_APPEVENT_BUFSIZE] = {0};
	app_event_wbd_weak_sta_t *weak_sta = NULL;

	/* Prepare event data. */
	weak_sta = (app_event_wbd_weak_sta_t*)app_data;
	WBDSTRNCPY(weak_sta->ifname, ifname, sizeof(weak_sta->ifname));
	eacopy(sta_mac, &(weak_sta->sta_addr));
	weak_sta->rssi = rssi;
	weak_sta->tx_failures = tx_failures;
	weak_sta->tx_rate = tx_rate;

	app_event_sendup(evt, APP_E_WBD_STATUS_SUCCESS,
		app_data, sizeof(*weak_sta));
}

/* Copy ifname, sta_mac, dst_mac and rssi into buf and send as event data. */
void
wbd_appeventd_steer_start(int evt, char *ifname, struct ether_addr *sta_mac,
	struct ether_addr *dst_mac, int src_rssi, int dst_rssi)
{
	unsigned char app_data[WBD_APPEVENT_BUFSIZE] = {0};
	app_event_wbd_steer_sta_t *steer_sta = NULL;

	/* Prepare event data. */
	steer_sta = (app_event_wbd_steer_sta_t*)app_data;
	WBDSTRNCPY(steer_sta->ifname, ifname, sizeof(steer_sta->ifname));
	eacopy(sta_mac, &(steer_sta->sta_addr));
	eacopy(dst_mac, &(steer_sta->dst_addr));
	steer_sta->src_rssi = src_rssi;
	steer_sta->dst_rssi = dst_rssi;

	app_event_sendup(evt, APP_E_WBD_STATUS_SUCCESS,
		app_data, sizeof(*steer_sta));
}

/* copy ifname, sta_mac and resp into buf and send as event data. */
void
wbd_appeventd_steer_resp(int evt, char *ifname, struct ether_addr *sta_mac, int resp)
{
	unsigned char app_data[WBD_APPEVENT_BUFSIZE] = {0};
	app_event_wbd_steer_resp_t *steer_resp = NULL;
	char *resp_str = WBD_STEER_RESPONSE_ACCEPT;

	if (resp == WLIFU_BSS_TRANS_RESP_REJECT) {
		resp_str = WBD_STEER_RESPONSE_REJECT;
	} else if (resp == WLIFU_BSS_TRANS_RESP_UNKNOWN) {
		resp_str = WBD_STEER_RESPONSE_UNKNOWN;
	}

	/* Prepare event data. */
	steer_resp = (app_event_wbd_steer_resp_t*)app_data;
	WBDSTRNCPY(steer_resp->ifname, ifname, sizeof(steer_resp->ifname));
	eacopy(sta_mac, &(steer_resp->sta_addr));
	steer_resp->resp_code = resp;
	WBDSTRNCPY(steer_resp->resp, resp_str, sizeof(steer_resp->resp));

	app_event_sendup(evt, APP_E_WBD_STATUS_SUCCESS,
		app_data, sizeof(*steer_resp));
}

/* Copy ifname, sta_mac, src_mac and dst_mac into buf and send as event data. */
void
wbd_appeventd_steer_complete(int evt, char *ifname, struct ether_addr *sta_mac,
	struct ether_addr *src_mac, struct ether_addr *dst_mac, int rssi, float tx_rate)
{
	unsigned char app_data[WBD_APPEVENT_BUFSIZE] = {0};
	app_event_wbd_steer_complete_t *steer_complete = NULL;

	/* Prepare event data. */
	steer_complete = (app_event_wbd_steer_complete_t*)app_data;
	WBDSTRNCPY(steer_complete->ifname, ifname, sizeof(steer_complete->ifname));
	eacopy(sta_mac, &(steer_complete->sta_addr));
	eacopy(src_mac, &(steer_complete->src_addr));
	eacopy(dst_mac, &(steer_complete->dst_addr));
	eacopy(sta_mac, &(steer_complete->sta_stats.sta_addr));
	steer_complete->sta_stats.rssi = rssi;
	steer_complete->sta_stats.tx_rate = tx_rate;

	app_event_sendup(evt, APP_E_WBD_STATUS_SUCCESS,
		app_data, sizeof(*steer_complete));
}
