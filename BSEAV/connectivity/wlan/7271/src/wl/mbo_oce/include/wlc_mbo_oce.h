/*
 * MBO-OCE declarations/definitions for
 * Broadcom 802.11abgn Networking Device Driver
 *
 * $Copyright (C) 2016 Broadcom Corporation$
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 *
 */

#ifndef _wlc_mbo_oce_h_
#define _wlc_mbo_oce_h_

wlc_mbo_oce_info_t * wlc_mbo_oce_attach(wlc_info_t *wlc);
void wlc_mbo_oce_detach(wlc_mbo_oce_info_t *mbo);

#endif	/* _wlc_mbo_oce_h_ */
