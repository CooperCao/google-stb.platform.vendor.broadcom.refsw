/**
 * Required functions exported by the wlc_aibss.c to common (os-independent) driver code.
 *
 * Advanced IBSS mode
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef _WLC_AIBSS_H_
#define _WLC_AIBSS_H_

#ifdef	WLAIBSS


/* AC_VI: 4,5	AC_VO: 6,7 */
#define IS_HI_PRIO_PKT(pkt) ((PKTPRIO(pkt) == 4) || (PKTPRIO(pkt) == 5) || \
		(PKTPRIO(pkt) == 6) || (PKTPRIO(pkt) == 7))

/* bit mask of force wake bits */
#define	WLC_AIBSS_FORCE_WAKE_NON_PS_PEER	0x1
#define WLC_AIBSS_FORCE_WAKE_BT_COEX		0x2
#define	WLC_AIBSS_FORCE_WAKE_RMC		0x4
#define WLC_AIBSS_FORCE_WAKE_INITIAL		0x8

#define WLC_AIBSS_INITIAL_WAKEUP_PERIOD		16 /* in second */
#define WLC_AIBSS_DEFAULT_ATIM_FAILURE		5

extern wlc_aibss_info_t *wlc_aibss_attach(wlc_info_t *wlc);
extern void wlc_aibss_detach(wlc_aibss_info_t *aibss);
extern void wlc_aibss_tbtt(wlc_aibss_info_t *aibss);
extern bool wlc_aibss_sendpmnotif(wlc_aibss_info_t *aibss, wlc_bsscfg_t *cfg,
	ratespec_t rate_override, int prio, bool track);
extern void wlc_aibss_atim_window_end(wlc_info_t *wlc);
extern void wlc_aibss_ps_start(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_aibss_ps_stop(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_aibss_ps_process_atim(wlc_info_t *wlc, struct ether_addr *ea);
extern void wlc_aibss_stay_awake(wlc_aibss_info_t * aibss_info);
extern void wlc_aibss_tx_pkt(wlc_aibss_info_t * aibss_info, struct scb *scb, void *pkt);
extern void wlc_aibss_set_wake_override(wlc_aibss_info_t *aibss_info, uint32 wake_reason, bool set);
extern void wlc_aibss_back2homechan_notify(wlc_info_t *wlc);
extern void wlc_aibss_ps_update_request(wlc_aibss_info_t *aibss_info, bool set);
#else
#define wlc_aibss_atim_window_end(a)	do {} while (0)
#define wlc_aibss_ps_start(a, b)	do {} while (0)
#define wlc_aibss_ps_stop(a, b)	do {} while (0)
#define wlc_aibss_ps_process_atim(a, b)	do {} while (0)
#define wlc_aibss_stay_awake(a) do {} while (0)
#define wlc_aibss_tx_pkt(a, b, c) do {} while (0)
#define wlc_aibss_set_wake_override(a, b, c) do {} while (0)
#define wlc_aibss_back2homechan_notify(a) do {} while (0)
#define wlc_aibss_ps_update_request(a, b) do {} while (0)
#endif /* WLAIBSS */

#endif /* _WLC_AIBSS_H_ */
