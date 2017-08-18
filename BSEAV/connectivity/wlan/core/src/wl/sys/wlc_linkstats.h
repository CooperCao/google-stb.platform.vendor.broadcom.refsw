/*
 * Link Layer statistics
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */


#ifndef _wlc_linkstats_h_
#define _wlc_linkstats_h_

typedef struct {
	uint32 preamble;
	uint32 nss;
	uint32 bw;
	uint32 rateMcsIdx;
	uint32 reserved;
	uint32 bitrate;
} wifi_rate;

typedef struct {
	uint16 version;
	uint16 length;
	uint32 tx_mpdu;
	uint32 rx_mpdu;
	uint32 mpdu_lost;
	uint32 retries;
	uint32 retries_short;
	uint32 retries_long;
	wifi_rate rate;
} wifi_rate_stat_t;

typedef int32 wifi_radio;

typedef struct {
	uint16 version;
	uint16 length;
	wifi_radio radio;
	uint32 on_time;
	uint32 tx_time;
	uint32 rx_time;
	uint32 on_time_scan;
	uint32 on_time_nbd;
	uint32 on_time_gscan;
	uint32 on_time_roam_scan;
	uint32 on_time_pno_scan;
	uint32 on_time_hs20;
	uint32 num_channels;
	uint8 channels[1];
} wifi_radio_stat;

void wlc_cckofdm_tx_inc(wlc_info_t *wlc, uint16 type, ratespec_t rate);
int wlc_rate_stat_get(wlc_info_t *wlc, void *arg, int len);
int wlc_radio_stat_get(wlc_info_t *wlc, void *arg, int len);

wlc_linkstats_info_t * wlc_linkstats_attach(wlc_info_t *wlc);
void wlc_linkstats_detach(wlc_linkstats_info_t *linkstats_info);

#endif /* _wlc_linkstats_h_ */
