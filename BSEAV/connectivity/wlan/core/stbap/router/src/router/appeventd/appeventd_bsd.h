/*
 * Wireless Application Event Service
 * appeventd-bsd shared header file
 *
 * Copyright (C) 2018, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * $Id:  $
 */

#ifndef _appeventd_bsd_h_
#define _appeventd_bsd_h_

/* WiFi Application Event BSD status */
#define APP_E_BSD_STATUS_STEER_START  1
#define APP_E_BSD_STATUS_STEER_SUCC   2
#define APP_E_BSD_STATUS_STEER_FAIL   3
#define APP_E_BSD_STATUS_QUERY_STA    4
#define APP_E_BSD_STATUS_QUERY_RADIO  5

#define BSD_APPEVENT_BUFSIZE 2000

/* WiFi Application Event BSD data structure */
typedef struct app_event_bsd_sta {
	struct	ether_addr addr; /* STA mac addr */
	char	src_ifname[IFNAMSIZ];
	char	dst_ifname[IFNAMSIZ];
	int	steer_cnt;
	int	steer_fail_cnt;
	uint32	tx_rate;
	uint32	at_ratio;         /* airtime ratio */
	int	rssi;
	uint32	steer_flags;
} app_event_bsd_sta_t;

typedef struct app_event_bsd_radio {
	char	ifname[IFNAMSIZ];
	int	bw_util;
	uint32	throughput;
} app_event_bsd_radio_t;

#endif /* _appeventd_bsd_h_ */
