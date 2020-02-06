/*
 * WBD WL utility for both Master and Slave (Linux)
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wbd_wl_utility.h 766139 2018-07-25 09:57:56Z ms888945 $
 */

#ifndef _WBD_WL_UTILITY_H_
#define _WBD_WL_UTILITY_H_

#include "wbd.h"


/* Fetch Interface Info for AP */
extern int wbd_wl_fill_interface_info(wbd_wl_interface_t *ifr_info);

/* Check If the STA is DWDS client using vendor IE if the IE data provided else use sta_info */
extern int wbd_wl_is_sta_type_dwds(char *ifname, struct ether_addr *addr, int *outerr,
	uint8 *data, int len);

/* Send assoc decision */
extern int wbd_wl_send_assoc_decision(char *ifname, uint32 info_flags, uint8 *data, int len,
	struct ether_addr *sta_mac, uint32 evt_type);

/* Send Action frame to STA, Measure rssi from the response */
extern int wbd_wl_actframe_to_sta(char *ifname, struct ether_addr* ea);

/* Get dfs pref chan list from driver */
extern int wbd_wl_get_dfs_forced_chspec(char* ifname, wl_dfs_forced_t *smbuf, int size);

/* Set dfs pref chan list */
extern int wbd_wl_set_dfs_forced_chspec(char* ifname, wl_dfs_forced_t* dfs_frcd,
	int ioctl_size);

/* Send beacon request to sta */
extern int wbd_wl_send_beacon_request(char *ifname, wlc_ssid_t *ssid,
	struct ether_addr *bssid, struct ether_addr *sta_mac,
	uint8 channel, int *delay);

/* Find ie for given id and type */
extern bcm_tlv_t *wbd_wl_find_ie(uint8 id, void *tlvs, int tlvs_len, const char *voui, uint8 *type,
	int type_len);

/* Get channel availablity in percentage */
extern int wbd_wl_get_link_availability(char *ifname, unsigned short *link_available);
#endif /* _WBD_WL_UTILITY_H_ */
