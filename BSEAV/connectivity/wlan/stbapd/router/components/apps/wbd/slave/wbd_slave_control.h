/*
 * WBD Weak Client identification Policy declarations
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wbd_slave_control.h 618311 2016-02-10 15:06:06Z chintanb $
 */

#ifndef _WBD_SLAVE_CONTROL_H_
#define _WBD_SLAVE_CONTROL_H_

#include "ieee1905.h"
#include "wbd.h"
#include "wbd_shared.h"
#include "wbd_ds.h"

/* Structure to hold params to check if the backhaul steered to target bssid */
typedef struct wbd_bh_steer_check_arg {
	ieee1905_backhaul_steer_msg *bh_steer_msg;
	char ifname[IFNAMSIZ];
	int check_cnt;
} wbd_bh_steer_check_arg_t;

/* Get the number of threshold policies defined */
extern int wbd_get_max_wc_thld();

/* Get the deciding weak client algorithms */
extern int wbd_get_max_wc_algo();

/* Get the threshold policy based on index */
extern wbd_wc_thld_t *wbd_get_wc_thld(wbd_slave_item_t *slave_item);

/* Create the timer to check for weak clients & send WEAK_CLIENT command to Master, if any */
extern int wbd_slave_create_identify_weak_client_timer(wbd_slave_item_t *slave_item);

/* Remove timer to check for weak clients */
extern int wbd_slave_remove_identify_weak_client_timer(wbd_slave_item_t *slave_item);

/* Callback from IEEE 1905 module when the agent gets STEER request */
extern void wbd_slave_process_map_steer_request_cb(wbd_info_t *info, ieee1905_steer_req *steer_req,
	ieee1905_vendor_data *in_vndr_tlv);

/* Callback from IEEE 1905 module when the agent gets block/unblock STA request */
extern void wbd_slave_process_map_block_unblock_sta_request_cb(wbd_info_t *info,
	ieee1905_block_unblock_sta *block_unblock_sta);

/* Callback from IEEE 1905 module for channel preference report */
void wbd_slave_prepare_local_channel_preference(i5_dm_interface_type *i5_intf,
	ieee1905_chan_pref_rc_map_array *cp);

/* Callback from IEEE 1905 module when the agent gets Channel Selection request */
extern void wbd_slave_process_chan_selection_request_cb(wbd_info_t *info, unsigned char *al_mac,
	unsigned char *interface_mac, ieee1905_chan_pref_rc_map_array *cp,
	unsigned char rclass_local_count, ieee1905_chan_pref_rc_map *local_chan_pref);

/* Callback from IEEE 1905 module when the agent gets set tx power limit */
extern void wbd_slave_process_set_tx_power_limit_cb(wbd_info_t *info,
	char *ifname, unsigned char tx_power_limit);

/* Callback from IEEE 1905 module to get the backhual link metric */
int wbd_slave_process_get_backhaul_link_metric_cb(wbd_info_t *info, char *ifname,
	unsigned char *interface_mac, ieee1905_backhaul_link_metric *metric);

/* This creates the BSS on the interface */
int wbd_slave_create_bss_on_ifr(wbd_info_t *info, char *ifname, unsigned char erase,
	ieee1905_client_bssinfo_type *bss, unsigned char restart);

/* Callback from IEEE 1905 module to steer backhaul to a different bss */
void wbd_slave_process_bh_steer_request_cb(wbd_info_t *info, char *ifname,
	ieee1905_backhaul_steer_msg *bh_steer_req);

/* Passes the association control request to the ieee1905 */
extern void wbd_slave_send_assoc_control(int blk_time, unsigned char *source_bssid,
	unsigned char *trgt_bssid, struct ether_addr *sta_mac);

/* Do Mandate and Opportunity steer */
extern void wbd_do_map_steer(wbd_slave_item_t *slave, ieee1905_steer_req *steer_req,
	ieee1905_sta_list *sta_info, ieee1905_bss_list *bss_info, int btm_supported,
	uint8 is_retry);
#endif /* _WBD_SLAVE_CONTROL_H_ */
