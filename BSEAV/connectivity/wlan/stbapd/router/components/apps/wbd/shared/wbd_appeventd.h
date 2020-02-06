/*
 * WBD Appeventd include file
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wbd_appeventd.h 764064 2018-05-23 13:09:46Z cb888957 $
 */

#ifndef _WBD_APPEVENTD_H_
#define _WBD_APPEVENTD_H_

#include "appeventd.h"

/*
 * Routine to send  MAP init Start/End event from wbd to appeventd.
 * Params:
 * @evt: Event id
 * @device_id: AL MAC address of Controller device.
 * @status: Start = 1, end = 2.
 */
extern void wbd_appeventd_map_init(int evt, struct ether_addr *device_id, int status);

/*
 * Routine to send  weak sta event from wbd to appeventd.
 * Params:
 * @evt: Event id
 * @ifname: Interface name.
 * @sta_mac: Sta mac address.
 * @rssi: Sta rssi value.
 * @tx_failures: Tx fail count.
 * @tx_rate: Tx rate.
 */
extern void wbd_appeventd_weak_sta(int evt, char *ifname, struct ether_addr *sta_mac,
	int rssi, uint32 tx_failures, float tx_rate);

/*
 * Routine to send steer start event from wbd to appeventd.
 * Params:
 * @evt: Event id
 * @ifname: Interface name.
 * @sta_mac: Sta mac address.
 * @dst_mac: Destination slave's bssid.
 * @src_rssi: Sta rssi at source AP.
 * @dst_rssi: Sta rssi at target AP.
 */
extern void wbd_appeventd_steer_start(int evt, char *ifname, struct ether_addr *sta_mac,
	struct ether_addr *dst_mac, int src_ssi, int dst_rssi);

/*
 * Routine to send steer response event from wbd to appeventd.
 * Params:
 * @evt: Event id
 * @ifname: Interface name.
 * @sta_mac: Sta mac address.
 * @resp: Steer response.
 */
extern void wbd_appeventd_steer_resp(int evt, char *ifname, struct ether_addr *sta_mac, int resp);

/*
 * Routine to send steer complete event from wbd to appeventd.
 * Params:
 * @evt: Event id
 * @ifname: Interface name.
 * @sta_mac: Sta mac address.
 * @src_mac: Source slave bssid.
 * @dst_mac: Target slave bssid.
 * @rssi: Sta rssi.
 * @tx_rate: Sta tx-rate.
 */
extern void wbd_appeventd_steer_complete(int evt, char *ifname, struct ether_addr *sta_mac,
	struct ether_addr *src_mac, struct ether_addr *dst_mac, int rssi, float tx_rate);
#endif /* _WBD_APPEVENTD_H_ */
