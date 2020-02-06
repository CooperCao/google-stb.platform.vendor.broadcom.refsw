/*
 * WBD Communication Related Declarations for Master
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wbd_master_com_hdlr.h 764852 2018-06-06 10:14:07Z sp888952 $
 */

#ifndef _WBD_MASTER_COM_HDLR_H_
#define _WBD_MASTER_COM_HDLR_H_

#include "wbd.h"
#include "wbd_shared.h"

#define WBD_FLAGS_MASTER_FBT_ENABLED	0x0001	/* FBT enabled in Master */

#ifdef WLHOSTFBT
/* Check whether FBT is enabled or not */
#define WBD_FBT_ENAB(flags) ((flags) & (WBD_FLAGS_MASTER_FBT_ENABLED))
#else
#define WBD_FBT_ENAB(flags) 0
#endif /* WLHOSTFBT */

/* Initialize the communication module for master */
extern int wbd_init_master_com_handle(wbd_info_t *info);

/* Process 1905 Vendor Specific Messages at WBD Application Layer */
extern int wbd_master_process_vendor_specific_msg(ieee1905_vendor_data *msg_data);

/* Creates the blanket master for the blanket ID */
extern void wbd_master_create_master_info(wbd_info_t *info, uint8 bkt_id,
	char *bkt_name);

/* Got Associated STA link metric response */
extern void wbd_master_process_assoc_sta_metric_resp(wbd_info_t *info, unsigned char *al_mac,
	unsigned char *bssid, unsigned char *sta_mac, ieee1905_sta_link_metric *metric);

/* Got UnAssociated STA link metric response */
extern void wbd_master_unassoc_sta_metric_resp(unsigned char *al_mac,
	ieee1905_unassoc_sta_link_metric *metric);

/* Got Beacon Metrics metric response */
extern void wbd_master_beacon_metric_resp(unsigned char *al_mac, ieee1905_beacon_report *report);

/* Set AP configured flag */
extern void wbd_master_set_ap_configured(unsigned char *al_mac, unsigned char *radio_mac,
	int if_band);

/* Send channel selection request */
void wbd_master_send_channel_selection_request(wbd_info_t *info, unsigned char *al_mac,
	unsigned char *radio_mac);

/* Add the metric policy for a radio */
void wbd_master_add_metric_report_policy(wbd_info_t *info, unsigned char *al_mac,
	unsigned char* radio_mac, int if_band);

/* Update the AP channel report */
void wbd_master_update_ap_chan_report(wbd_master_info_t *master_info, i5_dm_interface_type *i5_ifr);

int wbd_master_send_link_metric_requests(wbd_master_info_t *master, i5_dm_clients_type *sta);

/* Get interface info */
int wbd_master_get_interface_info_cb(char *ifname, ieee1905_ifr_info *info);

/* Handle master restart; Send renew if the agent didn't AP Auto configuration */
int wbd_master_create_ap_autoconfig_renew_timer(wbd_info_t *info);

/* Create channel prefernce query timer */
int wbd_master_create_channel_select_timer(wbd_info_t *info, unsigned char *al_mac);

/* Handle operating channel report */
void wbd_master_process_operating_chan_report(wbd_info_t *info, unsigned char *al_mac,
	ieee1905_operating_chan_report *chan_report);

/* Update the lowest Tx Power of all BSS */
void wbd_master_update_lowest_tx_pwr(wbd_info_t *info, unsigned char *al_mac,
	ieee1905_operating_chan_report *chan_report);

/* Master get Vendor Specific TLV to send for 1905 Message */
extern void wbd_master_get_vendor_specific_tlv(i5_msg_types_with_vndr_tlv_t msg_type,
	ieee1905_vendor_data *vendor_tlv);

/* Store beacon metric response */
void
wbd_master_store_beacon_metric_resp(wbd_info_t *info, unsigned char *al_mac,
	ieee1905_beacon_report *report);

/* Send BSS capability query message */
extern int wbd_master_send_bss_capability_query(wbd_info_t *info, unsigned char *al_mac,
	unsigned char* radio_mac);

/* Called when some STA joins the slave */
extern int wbd_controller_refresh_blanket_on_sta_assoc(struct ether_addr *sta_mac,
	struct ether_addr *parent_bssid, wbd_wl_sta_stats_t *sta_stats);

/* Send BSS metrics query message */
extern int wbd_master_send_bss_metrics_query(unsigned char *al_mac);
#endif /* _WBD_MASTER_COM_HDLR_H_ */
