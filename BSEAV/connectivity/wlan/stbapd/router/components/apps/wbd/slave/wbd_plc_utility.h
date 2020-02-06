/*
 * WBD PLC utility for Slave
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wbd_plc_utility.h 647252 2016-07-04 13:42:00Z spalanga $
 */

#ifndef _WBD_PLC_UTILITY_H_
#define _WBD_PLC_UTILITY_H_

#ifdef PLC_WBD
#include "wbd.h"

typedef struct wbd_plc_node_info {
	float tx_rate;
	float rx_rate;
	struct ether_addr mac;
} wbd_plc_node_info_t;

/* To hold the list of mac addresses */
typedef struct wbd_plc_assoc_info {
	uint count;
	wbd_plc_node_info_t node[1];
} wbd_plc_assoc_info_t;

/* Initialize PLC interface */
int wbd_plc_init(wbd_plc_info_t *plc_info);

/* Returns 1 if PLC is enabled */
int wbd_plc_is_enabled(wbd_plc_info_t *plc_info);

/* Get the local PLC MAC address */
int wbd_plc_get_local_plc_mac(wbd_plc_info_t *plc_info, struct ether_addr *mac);

/* Get PLC assoc info */
int wbd_plc_get_assoc_info(wbd_plc_info_t *plc_info, wbd_plc_assoc_info_t **out_assoc_info);

#endif /* PLC_WBD */
#endif /* _WBD_PLC_UTILITY_H_ */
