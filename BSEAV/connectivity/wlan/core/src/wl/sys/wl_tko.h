/*
 * TKO - TCP Keepalive Offload
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
 * $Id:$
 */

#ifndef _wl_tko_h_
#define _wl_tko_h_

#include <wlc.h>

typedef struct tko_info wl_tko_info_t;

extern wl_tko_info_t *wl_tko_attach(wlc_info_t *wlc);
extern void wl_tko_detach(wl_tko_info_t *tko_info);
extern bool wl_tko_rx(wl_tko_info_t *tko_info, void *sdu, wlc_bsscfg_t * bsscfg);
extern bool wl_tko_is_running(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg);

#ifdef HOFFLOAD_TKO
/* TKO module loaded functions */
int tko_get(wl_tko_info_t *tko_info, void *p, uint plen, void *a, int alen, wlc_bsscfg_t * bsscfg);
int tko_set(wl_tko_info_t *tko_info, void *a, int alen, wlc_bsscfg_t * bsscfg);
bool tko_rx_proc(wl_tko_info_t *tko_info, void *sdu);
void tko_destroy_all_tcp(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg);
#endif	/* HOFFLOAD_TKO */

#endif	/* _wl_tko_h_ */
