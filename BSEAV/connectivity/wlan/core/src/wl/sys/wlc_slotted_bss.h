/*
 * Slotted IBSS feature
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
 * $Id: wlc_slotted_bss.h $
 */
#ifndef _wlc_slotted_bss_h
#define _wlc_slotted_bss_h
#include <wlc_types.h>
#ifdef PROP_TXSTATUS
#include <wlc_wlfc.h>
#endif

typedef enum {
	SB_RNG_START = 1,
	SB_RNG_STOP  = 2
} sb_state_t;

typedef struct slotted_bss_notif_data
{
	sb_state_t state;
	wlc_bsscfg_t *bsscfg;
	uint32 duration;
	void *peer_ctx;
	struct ether_addr peer_mac_addr;
	chanspec_t chanspec;
} slotted_bss_notif_data_t;

typedef bool (*bss_is_peer_avail_fn_t)(void *ctx, struct scb *scb, chanspec_t chanspec);
typedef int (*bss_peer_scb_reinit_fn_t)(void *ctx, wlc_bsscfg_t *cfg,
		struct scb *scb);
typedef int (*slotted_bss_st_notif_fn_t) (void *arg, slotted_bss_notif_data_t *notif_data);

extern wlc_slotted_bss_info_t *wlc_slotted_bss_attach(wlc_info_t *wlc);
extern void wlc_slotted_bss_detach(wlc_slotted_bss_info_t *sbi);
extern void wlc_slotted_bss_register_cb(wlc_slotted_bss_info_t *sbi, wlc_bsscfg_t *bsscfg,
	bss_is_peer_avail_fn_t peer_avail_fn,
	bss_peer_scb_reinit_fn_t peer_scb_reinit_fn, void *ctx);
#ifdef PROP_TXSTATUS
extern int wlc_slotted_bss_update_peers(wlc_slotted_bss_info_t *sbi,
	wlc_bsscfg_t *bsscfg, chanspec_t chanspec,
	wlfc_ctl_type_t open_close, bool interface_update);
extern int wlc_slotted_bss_update_peers_for_rng(wlc_slotted_bss_info_t *sbi,
	wlc_bsscfg_t *bsscfg, struct ether_addr *peer_mac_addr, uint32 duration, void *rng_ctx,
	chanspec_t chanspec, sb_state_t start_stop);
extern void wlc_slotted_bss_update_iface(wlc_slotted_bss_info_t *sbi,
	wlc_info_t *wlc, wlfc_ctl_type_t open_close,
	bool interface_update);
#endif /* PROP_TXSTATUS */

extern void wlc_slotted_bss_peer_upd_ie(wlc_slotted_bss_info_t *sbi, wlc_bsscfg_t *bsscfg,
	struct scb *scb, uint8 mode, uint8 *peer_ie, int peer_ie_len, uint8 min_rate);
extern int wlc_slotted_bss_peer_add(wlc_slotted_bss_info_t *sbi, wlc_bsscfg_t *bsscfg,
	struct ether_addr *addr, struct scb **scb, uint8 max_peers);
extern void wlc_slotted_bss_peer_delete(wlc_slotted_bss_info_t *sbi, struct scb *scb);
extern void wlc_slotted_bss_peer_reinit_scb(struct scb *scb, uint8 *ies, uint ies_len, uint16 bw,
		uint8 min_rate);
extern int wlc_slotted_bss_st_notif_register(wlc_slotted_bss_info_t *sbi,
	slotted_bss_st_notif_fn_t fn, void *arg);
extern int wlc_slotted_bss_st_notif_unregister(wlc_slotted_bss_info_t *sbi,
	slotted_bss_st_notif_fn_t fn, void *arg);
bool wlc_slotted_bss_peer_in_valid_chan(wlc_slotted_bss_info_t *sbi,
		struct scb *scb, chanspec_t chanspec);
struct scb* wlc_slotted_bss_get_bcmc_scb(wlc_bsscfg_t *cfg);
void wlc_slotted_bss_set_scb_bandunit(wlc_bsscfg_t *cfg, uint32 bandunit);
bool wlc_slotted_bss_is_singleband_5g(wlc_info_t *wlc);

/* PHY cache related functions for slotted bss */
int wlc_slotted_bss_phy_cache_add(wlc_info_t *wlc, chanspec_t chan);
int wlc_slotted_bss_phy_cache_del(wlc_info_t *wlc, chanspec_t chan);
void wlc_slotted_bss_phy_calmode_set(wlc_info_t *wlc, uint8 newmode);
void wlc_slotted_bss_phy_calmode_restore(wlc_info_t *wlc);
uint8 wlc_slotted_bss_get_op_rxstreams(wlc_bsscfg_t *cfg, chanspec_t chanspec);
int wlc_slotted_bss_datapath_update(wlc_slotted_bss_info_t *sbi, wlc_bsscfg_t *bsscfg,
		chanspec_t chanspec, wlfc_ctl_type_t open_close);
int wlc_slotted_bss_switch_scb_and_if(wlc_slotted_bss_info_t *sbi, wlc_bsscfg_t *cfg,
		struct scb *scb, chanspec_t chanspec);
#endif /* _wlc_slotted_bss_h */
