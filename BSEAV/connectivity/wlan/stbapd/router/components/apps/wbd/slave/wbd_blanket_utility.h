/*
 * WBD Blanket utility for Slave (Linux)
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wbd_blanket_utility.h 764918 2018-06-11 04:02:34Z sp888952 $
 */

#ifndef _WBD_BLANKET_UTILITY_H_
#define _WBD_BLANKET_UTILITY_H_

#include "wbd.h"


#ifdef WLHOSTFBT
/* Check whether FBT is enabled or not */
#define WBD_FBT_ENAB(flags) ((flags) & (WBD_FLAGS_BSS_FBT_ENABLED))
#else
#define WBD_FBT_ENAB(flags) 0
#endif /* WLHOSTFBT */

/* Get STA link metrics */
int wbd_slave_process_get_assoc_sta_metric(char *ifname, unsigned char *bssid,
	unsigned char *sta_mac, ieee1905_sta_link_metric *metric,
	ieee1905_sta_traffic_stats *traffic_stats,
	const ieee1905_sta_traffic_stats *old_traffic_stats, ieee1905_vendor_data *out_vndr_tlv);

/* Update BSS capability for all BSS in a interface */
void wbd_slave_update_bss_capability(i5_dm_interface_type *i5_ifr);

/** @brief STA has Associated or Disassociated on this device
 *
 * @param bssid		BSSID of the BSS on which STA  associated or disassociated
 * @param mac		MAC address of the STA
 * @param isAssoc	1 if STA Associated, 0 if STA Disassociated
 * @param time_elapsed	Seconds since assoc
 * @param notify	Notify immediately to neighbors
 * @param assoc_frame	Frame body of the (Re)Association request frame
 * @param assoc_frame_len	Length of the (Re)Association Request frame length
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_sta_assoc_disassoc(struct ether_addr *bssid, struct ether_addr *mac, int isAssoc,
	unsigned short time_elapsed, unsigned char notify, unsigned char *assoc_frame,
	unsigned int assoc_frame_len);

/** @brief Add All STA's Currently Associated with the BSS
 *
 * @param ifname	interface name of the BSS
 * @param bssid		BSSID of the BSS on which STA  associated
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_add_all_associated_stas(char *ifname, struct ether_addr *bssid);
#endif /* _WBD_BLANKET_UTILITY_H_ */
