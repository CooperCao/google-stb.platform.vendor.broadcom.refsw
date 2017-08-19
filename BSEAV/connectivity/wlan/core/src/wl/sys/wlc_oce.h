/*
 * OCE declarations/definitions for
 * Broadcom 802.11abgn Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 *
 */

/**
 * WFA Optimized Connectivity Experience (OCE) certification program certifies features
 * that deliver a better overall connectivity experience by taking advantage of systemic
 * information available within planned and managed networks (e.g. hotspot, workplace,
 * operator-deployed networks). The program is intended to address issues identified by
 * operators including very long connection setup times, poor wireless local area network (WLAN)
 * connectivity, and spectrum consumed by management frames.
 */

#ifndef _wlc_oce_h_
#define _wlc_oce_h_

#include <proto/mbo_oce.h>
#include <proto/oce.h>

wlc_oce_info_t * wlc_oce_attach(wlc_info_t *wlc);
void wlc_oce_detach(wlc_oce_info_t *oce);
bool wlc_oce_send_probe(wlc_oce_info_t *oce, void *p);
uint8 wlc_oce_get_probe_defer_time(wlc_oce_info_t *oce);
bool wlc_oce_is_oce_environment(wlc_oce_info_t *oce);
uint8 wlc_oce_get_pref_channels(chanspec_t *chanspec_list);
bool wlc_oce_is_pref_channel(chanspec_t chanspec);

#endif	/* _wlc_oce_h_ */
