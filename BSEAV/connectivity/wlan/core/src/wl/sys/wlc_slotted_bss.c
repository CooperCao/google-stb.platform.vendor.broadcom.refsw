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
 * $Id: wlc_slotted_bss.c $
 */
/* ---- Include Files ---------------------------------------------------- */
#ifdef WLSLOTTED_BSS
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_bsscfg.h>
#include <wlc_scb.h>
#include <wlc_utils.h>
#ifdef WLRSDB
#include <wlc_rsdb.h>
#endif /* WLRSDB */
#ifdef WLRSDB_POLICY_MGR
#include <wlc_rsdb_policymgr.h>
#endif /* WLRSDB_POLICY_MGR */
#include <wlc_event.h>
#include <wlc_ampdu.h>
#include <wlc_slotted_bss.h>
#include <wlc_vht.h>
#include <wlc_ht.h>
#include <wl_export.h>
#include <wlc_scb_ratesel.h>
#include <wlc_lq.h>
#include <wlc_stf.h>
#include <phy_calmgr_api.h>
#include <wlc_chanctxt.h>
#include <phy_chanmgr_api.h>
#include <phy_cache_api.h>

#define SLOTTED_BSS_AGG_EN			(1 << 0)	/* Bitmap of mode	*/
#define SLOTTED_BSS_AGG_LIMIT_DUR	(1 << 1)	/* Jira 49554		*/

/* Maximum number of active channels for slotted bss */
#define SLOTTED_BSS_CHANCAL_MAX_CHANS		16

typedef struct slotted_bss_cal_chan {
	chanspec_t chanspec;
	uint16	ref_cnt;
} slotted_bss_cal_chan_t;

typedef struct bsscfg_slotted_bss_chan_cal_info {
	uint8 phy_cal_mode;
	uint8 num_channels;
	slotted_bss_cal_chan_t *chan_list;
} bsscfg_slotted_bss_chan_cal_info_t;

typedef struct bsscfg_slotted_bss_type_cubby {
	uint8 subtype;
	bss_is_peer_avail_fn_t peer_avail_fn; /* Callback function to check peer availability */
	void *ctx;
	/* Callback function to reinit peer scb */
	bss_peer_scb_reinit_fn_t peer_scb_reinit_fn;
} bsscfg_slotted_bss_type_cubby_t;

typedef struct slotted_bss_cmn {
	bcm_notif_h st_change_notif; /* Notification context pointer which is global one */
} slotted_bss_cmn_t;

typedef struct wlc_slotted_bss_info {
	osl_t *osh;
	wlc_obj_registry_t *objr;
	int	cfgh;
	slotted_bss_cmn_t *slotted_bss_cmn;
	/* Channel calibration info for slotted bss */
	bsscfg_slotted_bss_chan_cal_info_t *chan_cal_info;
} wlc_slotted_bss_info_t;

/* bsscfg states access macros */
#define BSSCFG_SLOTTED_BSS_TYPE_CUBBY_LOC(sbi, cfg) ((bsscfg_slotted_bss_type_cubby_t **)\
	BSSCFG_CUBBY((cfg), (sbi)->cfgh))
#define BSSCFG_SLOTTED_BSS_TYPE_CUBBY(sbi, cfg) (*BSSCFG_SLOTTED_BSS_TYPE_CUBBY_LOC(sbi, cfg))

static int wlc_slotted_bss_init(void *ctx, wlc_bsscfg_t *bsscfg);
static void wlc_slotted_bss_deinit(void *ctx, wlc_bsscfg_t *bsscfg);
#ifdef PROP_TXSTATUS
static int wlc_slotted_bss_peer_bss_status(wlc_slotted_bss_info_t *sbi,
	wlc_bsscfg_t *bsscfg, wlfc_ctl_type_t open_close);
#endif

slotted_bss_cal_chan_t chan_list[SLOTTED_BSS_CHANCAL_MAX_CHANS];

wlc_slotted_bss_info_t *
BCMATTACHFN(wlc_slotted_bss_attach)(wlc_info_t *wlc)
{
	wlc_slotted_bss_info_t *sbi = NULL;
	if ((sbi = MALLOCZ(wlc->osh, sizeof(*sbi))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	sbi->osh = wlc->osh;
	sbi->objr = wlc->objr;
	if ((sbi->cfgh = wlc_bsscfg_cubby_reserve(wlc,
		sizeof(bsscfg_slotted_bss_type_cubby_t *),
		wlc_slotted_bss_init, wlc_slotted_bss_deinit, NULL,
		(void *)sbi)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	sbi->slotted_bss_cmn = (slotted_bss_cmn_t*)obj_registry_get(wlc->objr,
		OBJR_SLOTTED_BSS_CMN_INFO);
	if (sbi->slotted_bss_cmn == NULL) {
		if ((sbi->slotted_bss_cmn = (slotted_bss_cmn_t *)MALLOCZ(wlc->osh,
				sizeof(*sbi->slotted_bss_cmn))) == NULL) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
				wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			goto fail;
		}
		obj_registry_set(wlc->objr, OBJR_SLOTTED_BSS_CMN_INFO, sbi->slotted_bss_cmn);
		(void)obj_registry_ref(wlc->objr, OBJR_SLOTTED_BSS_CMN_INFO);

		if (bcm_notif_create_list(wlc->notif,
			&sbi->slotted_bss_cmn->st_change_notif) != BCME_OK) {
			WL_ERROR(("wl%d: %s: bcm_notif_create_list() failed\n",
				wlc->pub->unit, __FUNCTION__));
			ASSERT(0);
			goto fail;
		}
	}

	if ((sbi->chan_cal_info = MALLOCZ(sbi->osh, sizeof(*sbi->chan_cal_info))) == NULL) {
		WL_ERROR(("wl%d: %s: Allocate memory for chan_cal_info failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if ((sbi->chan_cal_info->chan_list = MALLOCZ(sbi->osh,
		sizeof(*sbi->chan_cal_info->chan_list) * SLOTTED_BSS_CHANCAL_MAX_CHANS)) == NULL) {
		WL_ERROR(("wl%d: %s: Allocate memory for chan_list failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	return (sbi);

fail:
	MODULE_DETACH(sbi, wlc_slotted_bss_detach);
	return (NULL);
}

void
BCMATTACHFN(wlc_slotted_bss_detach)(wlc_slotted_bss_info_t *sbi)
{

	if (sbi) {
		if (sbi->chan_cal_info != NULL) {
			if (sbi->chan_cal_info->chan_list != NULL) {
				MFREE(sbi->osh, sbi->chan_cal_info->chan_list,
					sizeof(*sbi->chan_cal_info->chan_list) *
					SLOTTED_BSS_CHANCAL_MAX_CHANS);
			}

			MFREE(sbi->osh, sbi->chan_cal_info,
				sizeof(*sbi->chan_cal_info));
		}

		if (sbi->slotted_bss_cmn &&
			(obj_registry_unref(sbi->objr, OBJR_SLOTTED_BSS_CMN_INFO) == 0)) {
			obj_registry_set(sbi->objr, OBJR_SLOTTED_BSS_CMN_INFO, NULL);
			MFREE(sbi->osh, sbi->slotted_bss_cmn, sizeof(*sbi->slotted_bss_cmn));
			sbi->slotted_bss_cmn = NULL;
		}

		MFREE(sbi->osh, sbi, sizeof(*sbi));
	}
}

static int wlc_slotted_bss_init(void *ctx, wlc_bsscfg_t *bsscfg)
{
	wlc_slotted_bss_info_t *sbi = (wlc_slotted_bss_info_t *)ctx;
	wlc_info_t *wlc = bsscfg->wlc;
	bsscfg_slotted_bss_type_cubby_t **pslotted_bss =
		BSSCFG_SLOTTED_BSS_TYPE_CUBBY_LOC(sbi, bsscfg);
	bsscfg_slotted_bss_type_cubby_t *slotted_bss = NULL;
	if (bsscfg->type ==  BSSCFG_TYPE_SLOTTED_BSS) {
		if ((slotted_bss = MALLOCZ(wlc->osh,
			sizeof(*slotted_bss))) == NULL) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
				wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			return BCME_NOMEM;
		}
		*pslotted_bss = slotted_bss;
	}
	return BCME_OK;
}

static void wlc_slotted_bss_deinit(void *ctx, wlc_bsscfg_t *bsscfg)
{
	wlc_slotted_bss_info_t *sbi = (wlc_slotted_bss_info_t *)ctx;
	wlc_info_t *wlc = bsscfg->wlc;
	bsscfg_slotted_bss_type_cubby_t **pslotted_bss =
		BSSCFG_SLOTTED_BSS_TYPE_CUBBY_LOC(sbi, bsscfg);
	bsscfg_slotted_bss_type_cubby_t *slotted_bss = *pslotted_bss;

	if (slotted_bss) {
		MFREE(wlc->osh, slotted_bss, sizeof(*slotted_bss));
		*pslotted_bss = NULL;
	}
}
int
wlc_slotted_bss_st_notif_register(wlc_slotted_bss_info_t *sbi,
	slotted_bss_st_notif_fn_t fn, void *arg)
{
	return bcm_notif_add_interest(sbi->slotted_bss_cmn->st_change_notif,
		(bcm_notif_client_callback)fn, arg);
}

int
wlc_slotted_bss_st_notif_unregister(wlc_slotted_bss_info_t *sbi,
	slotted_bss_st_notif_fn_t fn, void *arg)
{
	return bcm_notif_remove_interest(sbi->slotted_bss_cmn->st_change_notif,
		(bcm_notif_client_callback)fn, arg);
}

void wlc_slotted_bss_register_cb(wlc_slotted_bss_info_t *sbi, wlc_bsscfg_t *bsscfg,
	bss_is_peer_avail_fn_t peer_avail_fn,
	bss_peer_scb_reinit_fn_t peer_scb_reinit_fn, void *ctx)
{
	bsscfg_slotted_bss_type_cubby_t *slotted_bss =
		BSSCFG_SLOTTED_BSS_TYPE_CUBBY(sbi, bsscfg);
	ASSERT(bsscfg->type == BSSCFG_TYPE_SLOTTED_BSS);
	slotted_bss->subtype = bsscfg->subtype;
	slotted_bss->peer_avail_fn = peer_avail_fn;
	slotted_bss->peer_scb_reinit_fn = peer_scb_reinit_fn;
	slotted_bss->ctx = ctx;
}


#ifdef PROP_TXSTATUS
static int
wlc_slotted_bss_peer_bss_status(wlc_slotted_bss_info_t *sbi, wlc_bsscfg_t *bsscfg,
	wlfc_ctl_type_t open_close)
{
	int rc = BCME_OK;
	BCM_REFERENCE(sbi);
	/* space for type(1), length(1) and value */
	uint8 results[1 + 1 + WLFC_CTL_VALUE_LEN_MAC];

	results[1] = WLFC_CTL_VALUE_LEN_MAC;
	results[0] = open_close;
	ASSERT((open_close == WLFC_CTL_TYPE_MAC_OPEN) ||
		(open_close == WLFC_CTL_TYPE_MAC_CLOSE) ||
		(open_close == WLFC_CTL_TYPE_INTERFACE_OPEN) ||
		(open_close == WLFC_CTL_TYPE_INTERFACE_CLOSE));

	if (bsscfg) {
		results[2] = wl_find_if(bsscfg->wlcif->wlif);
	} else {
		return BCME_ERROR;
	}

	rc = wlfc_push_signal_data(bsscfg->wlc->wlfc, results, sizeof(results), FALSE);
	return rc;
}

int
wlc_slotted_bss_update_peers(wlc_slotted_bss_info_t *sbi, wlc_bsscfg_t *bsscfg, chanspec_t chanspec,
	wlfc_ctl_type_t open_close, bool interface_update)
{
	struct scb *scb;
	struct scb_iter scbiter;
	wlc_info_t * wlc = bsscfg->wlc;
	int ret = BCME_OK;
	bsscfg_slotted_bss_type_cubby_t *slotted_bss =
		BSSCFG_SLOTTED_BSS_TYPE_CUBBY(sbi, bsscfg);
	WL_INFORM(("Slotted IBSS peer update %d, %u", open_close,
		R_REG(wlc->osh, &wlc->regs->tsf_timerlow)));

	/* If host has not configured explicit SLOTTED_IBSS interface updates, bail out */
	if (interface_update) {
		/* Control the Slotted_bss interface explicitly and
		 * map open_close to interface values.
		 */
		if (open_close == WLFC_CTL_TYPE_MAC_OPEN) {
			if (slotted_bss->peer_avail_fn) {
				ret = wlc_slotted_bss_peer_bss_status(sbi,
					bsscfg,
					WLFC_CTL_TYPE_INTERFACE_OPEN);
			}
		} else {
			ret = wlc_slotted_bss_peer_bss_status(sbi,
				bsscfg,
				WLFC_CTL_TYPE_INTERFACE_CLOSE);
		}
		if (ret != BCME_OK)
			return ret;
	}
	/* Slotted_bss peers get MAC updates */
	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
			if (open_close == WLFC_CTL_TYPE_MAC_OPEN) {
				if (slotted_bss->peer_avail_fn != NULL) {
					if ((slotted_bss->peer_avail_fn)(slotted_bss->ctx,
							scb, chanspec)) {
						ret  = wlc_wlfc_mac_status_upd(wlc->wlfc, scb,
							WLFC_CTL_TYPE_MAC_OPEN);
					}
					else {
						ret = wlc_wlfc_mac_status_upd(wlc->wlfc, scb,
							WLFC_CTL_TYPE_MAC_CLOSE);
					}
				}
			} else {
				ret = wlc_wlfc_mac_status_upd(wlc->wlfc, scb,
					WLFC_CTL_TYPE_MAC_CLOSE);
			}
			if (ret != BCME_OK)
				return ret;
	}
#ifdef WLAMPDU
	if (open_close == WLFC_CTL_TYPE_INTERFACE_CLOSE) {
		wlc_ampdu_flush_ampdu_q(wlc->ampdu_tx, bsscfg);
	}
#endif /* WLAMPDU */
	ret = wlfc_sendup_ctl_info_now(wlc->wlfc);
	return ret;
}
/* TODO: merge this api with wlc_slotted_bss_update_peers */
int
wlc_slotted_bss_update_peers_for_rng(wlc_slotted_bss_info_t *sbi, wlc_bsscfg_t *bsscfg,
	struct ether_addr *peer_mac_addr, uint32 duration, void *rng_ctx,
	chanspec_t chanspec, sb_state_t start_stop)
{
	slotted_bss_notif_data_t notif_data;
	int ret = BCME_OK;

	notif_data.state = start_stop;
	notif_data.bsscfg = bsscfg;
	notif_data.chanspec = chanspec;
	notif_data.duration = duration;
	notif_data.peer_ctx = rng_ctx;
	memcpy(&notif_data.peer_mac_addr, peer_mac_addr, sizeof(*peer_mac_addr));
	ret = bcm_notif_signal(sbi->slotted_bss_cmn->st_change_notif, &notif_data);
	return ret;
}
/* Updates all non-Slotted_bss interfaces */
void
wlc_slotted_bss_update_iface(wlc_slotted_bss_info_t *sbi, wlc_info_t *wlc,
	wlfc_ctl_type_t open_close, bool interface_update)
{
	int i;
	wlc_bsscfg_t *bsscfg;

	/* If host has not configured flow control updates, ignore */
	if (interface_update == FALSE) {
		return;
	}

	WL_INFORM(("non Slotted_IBSS if update %d, %u", open_close,
		R_REG(wlc->osh, &wlc->regs->tsf_timerlow)));

	/* All non-Slotted_bss interfaces have the inverse operation as Slotted_bss */
	FOREACH_ALL_WLC_BSS(wlc, i, bsscfg) {
		if (!BSSCFG_AWDL(wlc, bsscfg)) {
			wlc_slotted_bss_peer_bss_status(sbi, bsscfg, open_close);
		}
	}
	wlfc_sendup_ctl_info_now(wlc->wlfc);
}
#endif /* PROP_TXSTATUS */

void wlc_slotted_bss_peer_upd_ie(wlc_slotted_bss_info_t *sbi, wlc_bsscfg_t *bsscfg,
	struct scb *scb, uint8 mode, uint8 *peer_ie, int peer_ie_len, uint8 min_rate)
{
	wlc_info_t *wlc = bsscfg->wlc;
	/* save the IEs in the scb */
	ht_cap_ie_t *cap_ie = NULL;
	int i;
	BCM_REFERENCE(sbi);
	ASSERT(peer_ie != NULL);
	ASSERT(peer_ie_len > 0);
	cap_ie = wlc_read_ht_cap_ie(wlc, peer_ie, peer_ie_len);
	if (cap_ie) {
		ht_add_ie_t *add_ie;
		bool ampdutx_ori, ampdurx_ori;
#ifdef WL11AC
		vht_cap_ie_t vht_cap_ie;
		vht_cap_ie_t *vht_cap_ie_p = NULL;
#endif /* WL11AC */
		uint16 cap;
		uint16 htcap;

		add_ie = wlc_read_ht_add_ies(wlc, peer_ie, peer_ie_len);
		ampdutx_ori = wlc->pub->_ampdu_tx;
		ampdurx_ori = wlc->pub->_ampdu_rx;
#ifdef WLAMPDU
		if (!(mode & SLOTTED_BSS_AGG_EN)) {
			wlc_ampdu_tx_max_dur(wlc->ampdu_tx, scb, SLOTTED_BSS_MODULE,
				AMPDU_MAXDUR_INVALID_VAL);
			wlc->pub->_ampdu_tx = wlc->pub->_ampdu_rx = 0;
			scb->flags3 &= ~SCB3_AWDL_AGGR_CHANGE;
		} else if (mode & SLOTTED_BSS_AGG_LIMIT_DUR) {
			wlc_ampdu_tx_max_dur(wlc->ampdu_tx, scb, SLOTTED_BSS_MODULE,
				BTCX_AMPDU_MAX_DUR);
			scb->flags3 |= SCB3_AWDL_AGGR_CHANGE;
		} else {
			wlc_ampdu_tx_max_dur(wlc->ampdu_tx, scb, SLOTTED_BSS_MODULE,
				AMPDU_MAXDUR_INVALID_VAL);
			scb->flags3 &= ~SCB3_AWDL_AGGR_CHANGE;
		}
#endif /* WLAMPDU */
		scb->flags &= ~(SCB_HTCAP | SCB_AMSDUCAP | SCB_AMPDUCAP |
			SCB_NONGF | SCB_IS40 | SCB_RIFSCAP | SCB_HT40INTOLERANT |
			SCB_STBCCAP);
		scb->flags2 &= ~(SCB2_SGI20_CAP | SCB2_SGI40_CAP | SCB2_LDPCCAP);
			cap = ltoh16_ua(&cap_ie->cap);
		WL_INFORM(("%s(): HT cap: 0x%04x, wlc ht_cap: 0x%04x\n", __FUNCTION__,
			cap, wlc->ht_cap.cap));
		/* filter with its own HT capability, exclude MIMO_PS for now */
		htcap = wlc_ht_get_cap(wlc->hti);
		cap &= (htcap | HT_CAP_MIMO_PS_MASK | HT_CAP_RX_STBC_MASK);
		if (((cap & HT_CAP_RX_STBC_MASK) >> HT_CAP_RX_STBC_SHIFT) >
			((htcap & HT_CAP_RX_STBC_MASK) >> HT_CAP_RX_STBC_SHIFT)) {
			cap &= ~HT_CAP_RX_STBC_MASK;
			cap |= (htcap & HT_CAP_RX_STBC_MASK);
		}
		htol16_ua_store(cap, (uint8 *)&cap_ie->cap);
		WL_INFORM(("%s(): set HT cap to: 0x%04x\n", __FUNCTION__, cap));
			wlc_ht_update_scbstate(wlc->hti, scb, cap_ie, add_ie, NULL);
		wlc->pub->_ampdu_tx = ampdutx_ori;
		wlc->pub->_ampdu_rx = ampdurx_ori;

		if (min_rate) {
			for (i = 0; i < MCSSET_LEN; i++) {
				scb->rateset.mcs[i] = (scb->rateset.mcs[i] &
					scb->bsscfg->current_bss->rateset.mcs[i]);
			}
		}
#ifdef WL11AC
		vht_cap_ie_p = wlc_read_vht_cap_ie(wlc->vhti, peer_ie,
			peer_ie_len, &vht_cap_ie);
		if (scb->bandunit == BAND_5G_INDEX) {
			if ((vht_cap_ie_p != NULL) &&
				(WLC_VHT_FEATURES_5G((wlc->pub))))
					wlc_vht_update_scb_state(wlc->vhti,
						WLC_BAND_5G, scb, cap_ie,
						vht_cap_ie_p, NULL, 0);
			else if (SCB_VHT_CAP(scb))
					wlc_vht_update_scb_state(wlc->vhti,
					WLC_BAND_5G, scb, NULL, NULL, NULL, 0);
		}
		else
			wlc_vht_update_scb_state(wlc->vhti, WLC_BAND_2G, scb, NULL,
				NULL, NULL, 0);
#endif  /* WL11AC */
	}
}

int wlc_slotted_bss_peer_add(wlc_slotted_bss_info_t *sbi, wlc_bsscfg_t *bsscfg,
	struct ether_addr *addr, struct scb **scb, uint8 max_peers)
{
	struct scb_iter scbiter;
	struct scb *scb_p;
	uint8 n_peer = 0;
	wlc_info_t *wlc = bsscfg->wlc;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb_p) {
		if (scb_p->bsscfg == bsscfg) {
			n_peer++;
		}
	}
	if (n_peer >= max_peers) {
		return BCME_RANGE;
	}
	/* We will add scb to operating band and then switch bands if needed */
	if ((scb_p = wlc_scblookup(wlc, bsscfg, addr)) == NULL) {
#if defined(BCMDBG) || defined(WLMSG_ERR)
		char eabuf[ETHER_ADDR_STR_LEN];
		WL_ERROR(("wl%d: unable to create scb for %s\n",
			wlc->pub->unit, bcm_ether_ntoa(addr, eabuf)));
#endif
		return BCME_NOMEM;
	}
	SCB_PKTC_ENABLE(scb_p);
	scb_p->flags |= SCB_WMECAP;
	scb_p->permanent = TRUE;
	*scb = scb_p;
	return BCME_OK;
}


void wlc_slotted_bss_peer_delete(wlc_slotted_bss_info_t *sbi, struct scb *scb)
{
	BCM_REFERENCE(sbi);
	scb->permanent = FALSE;
	wlc_scbfree(scb->bsscfg->wlc, scb);
}

void wlc_slotted_bss_peer_reinit_scb(struct scb *scb, uint8 *ies, uint ies_len, uint16 bw,
		uint8 min_rate)
{
	wlc_info_t *wlc;
	/* Default rateset excluding MCS */
	wlc_rateset_filter(&scb->bsscfg->current_bss->rateset, &scb->rateset, FALSE,
		WLC_RATES_OFDM, RATE_MASK, FALSE);
	wlc = scb->bsscfg->wlc;
	/* get MCS rate from saved IEs */
	if (ies != NULL) {
		ht_cap_ie_t *cap_ie = NULL;
		ht_cap_ie_t ht_cap_ie;
		uint16 cap;
		int i;
#ifdef WL11AC
		vht_cap_ie_t vht_cap_ie;
		vht_cap_ie_t *vht_cap_ie_p = NULL;
#endif /* WL11AC */
		cap_ie = wlc_read_ht_cap_ie(wlc, ies, ies_len);
		if (cap_ie && SCB_HT_CAP(scb)) {
			bcopy(&cap_ie->supp_mcs[0], &scb->rateset.mcs[0], MCSSET_LEN);
			if (min_rate) {
				for (i = 0; i < MCSSET_LEN; i++) {
					scb->rateset.mcs[i] = (scb->rateset.mcs[i] &
					scb->bsscfg->current_bss->rateset.mcs[i]);
				}
			}
			/* turn off CAP_40MHZ based on the operating bandwidth */
			cap = ltoh16_ua(&cap_ie->cap);
			WL_INFORM(("%s(): HT cap: 0x%04x, wlc ht_cap: 0x%04x\n", __FUNCTION__,
				cap, wlc->ht_cap.cap));
			if (bw < WL_CHANSPEC_BW_40)
				cap &= ~HT_CAP_40MHZ;

			/* make a copy and leave the original alone */
			bcopy(cap_ie, &ht_cap_ie, sizeof(ht_cap_ie_t));
			htol16_ua_store(cap, (uint8 *)&(ht_cap_ie.cap));
			cap_ie = &ht_cap_ie;
			wlc_ht_update_scbstate(wlc->hti, scb, cap_ie, NULL, NULL);
		} else {
			bzero(&scb->rateset.mcs[0], MCSSET_LEN);
		}
#ifdef WL11AC
		vht_cap_ie_p = wlc_read_vht_cap_ie(wlc->vhti, ies, ies_len, &vht_cap_ie);
		if (scb->bandunit == BAND_5G_INDEX) {
			WL_INFORM(("%s(): update VHT in 5G, WLC_VHT_FEATURES_5G:0x%x\n",
				__FUNCTION__, WLC_VHT_FEATURES_GET((wlc->pub), 0xffff)));
			if ((vht_cap_ie_p != NULL) && (WLC_VHT_FEATURES_5G((wlc->pub))) &&
				(bw > WL_CHANSPEC_BW_40))
				wlc_vht_update_scb_state(wlc->vhti, WLC_BAND_5G, scb, cap_ie,
					vht_cap_ie_p, NULL, 0);
			else if (SCB_VHT_CAP(scb))
				wlc_vht_update_scb_state(wlc->vhti, WLC_BAND_5G, scb, NULL,
					NULL, NULL, 0);
		}
		else {
			wlc_vht_update_scb_state(wlc->vhti, WLC_BAND_2G, scb, NULL,
				NULL, NULL, 0);
		}
#endif /* WL11AC */
	}

	wlc_scb_ratesel_init(wlc, scb);

	wlc_lq_rssi_ma_reset(wlc, scb->bsscfg, scb, WLC_RSSI_INVALID);
}

bool
wlc_slotted_bss_peer_in_valid_chan(wlc_slotted_bss_info_t *sbi,
		struct scb *scb, chanspec_t chanspec)
{
	bsscfg_slotted_bss_type_cubby_t *slotted_bss =
		BSSCFG_SLOTTED_BSS_TYPE_CUBBY(sbi, scb->bsscfg);
	if (slotted_bss->peer_avail_fn != NULL) {
		return (slotted_bss->peer_avail_fn)(slotted_bss->ctx, scb, chanspec);
	} else {
		return FALSE;
	}
}

struct scb*
wlc_slotted_bss_get_bcmc_scb(wlc_bsscfg_t *cfg)
{
	return cfg->bcmc_scb;
}

void
wlc_slotted_bss_set_scb_bandunit(wlc_bsscfg_t *cfg, uint32 bandunit)
{
	cfg->bcmc_scb->bandunit = bandunit;
}

bool
wlc_slotted_bss_is_singleband_5g(wlc_info_t *wlc)
{
	return IS_SINGLEBAND_5G(wlc->deviceid, wlc->phy_cap);
}

int wlc_slotted_bss_phy_cache_add(wlc_info_t *wlc, chanspec_t chan)
{
	int i = 0;
	wlc_bsscfg_t *bsscfg;
	wlc_slotted_bss_info_t *sbi;
	bool need_cache = TRUE;

	ASSERT(wlc != NULL);
	sbi = wlc->sbi;

	if (wf_chspec_malformed(chan)) {
		WL_ERROR(("wl%d: %s: Malformed chanspec 0x%x\n", wlc->pub->unit, __FUNCTION__,
		          chan));
		return BCME_BADCHAN;
	}

	/* Check if the channel is already in the list */
	for (i = 0; i < sbi->chan_cal_info->num_channels; i++) {
		if (sbi->chan_cal_info->chan_list[i].chanspec == chan &&
			sbi->chan_cal_info->chan_list[i].ref_cnt > 0) {
			sbi->chan_cal_info->chan_list[i].ref_cnt++;
			return BCME_OK;
		}
	}

	if (sbi->chan_cal_info->num_channels == SLOTTED_BSS_CHANCAL_MAX_CHANS) {
		return BCME_NOMEM;
	}

	/* Check any bss (except slotted bss) has already associated on the channel.
	 * Note the check for slotted bss (e.g. AWDL, NAN) is done above by
	 * checking chan_cal_info->chan_list.
	 */
	FOREACH_ALL_WLC_BSS(wlc, i, bsscfg) {
		if ((WLC_BSS_CONNECTED(bsscfg) &&
			(WLC_CHAN_COEXIST(bsscfg->current_bss->chanspec, chan))) ||
			(wlc_shared_chanctxt_on_chan(wlc, bsscfg, chan) &&
			bsscfg->type != BSSCFG_TYPE_SLOTTED_BSS)) {
			need_cache = FALSE;
		}
	}

	BCM_REFERENCE(need_cache);

	sbi->chan_cal_info->chan_list[sbi->chan_cal_info->num_channels].chanspec = chan;
	sbi->chan_cal_info->chan_list[sbi->chan_cal_info->num_channels].ref_cnt++;
	sbi->chan_cal_info->num_channels++;

	if (need_cache) {
#ifdef PHYCAL_CACHING
		if (BCME_OK != phy_chanmgr_create_ctx((phy_info_t *)WLC_PI(wlc), chan)) {
			ASSERT(0);
			return BCME_NOMEM;
		}
		wlc_phy_update_chctx_glacial_time(WLC_PI(wlc), chan);
#endif
	}

	return BCME_OK;
}

int wlc_slotted_bss_phy_cache_del(wlc_info_t *wlc, chanspec_t chan)
{
	wlc_slotted_bss_info_t *sbi;
	int i;
	wlc_bsscfg_t *bsscfg;
	bool need_delete = TRUE;
	int ret = BCME_NOTFOUND;
	slotted_bss_cal_chan_t *chan_list;
	uint8 num_chan;

	ASSERT(wlc != NULL);
	sbi = wlc->sbi;
	chan_list = sbi->chan_cal_info->chan_list;
	num_chan = sbi->chan_cal_info->num_channels;

	if (wf_chspec_malformed(chan)) {
		WL_ERROR(("wl%d: %s: Malformed chanspec 0x%x\n", wlc->pub->unit, __FUNCTION__,
		          chan));
		return BCME_BADCHAN;
	}

	/* Check if any bss (except slotted bss) is still associated on the channel. For slotted
	 * bss, we can check the chan_list.
	 */
	FOREACH_ALL_WLC_BSS(wlc, i, bsscfg) {
		if (((WLC_CHAN_COEXIST(bsscfg->current_bss->chanspec, chan)) &&
			WLC_BSS_CONNECTED(bsscfg)) ||
			(wlc_shared_chanctxt_on_chan(wlc, bsscfg, chan) &&
			bsscfg->type != BSSCFG_TYPE_SLOTTED_BSS)) {
			need_delete = FALSE;
			break;
		}
	}

	BCM_REFERENCE(need_delete);

	for (i = 0; i < num_chan; i++) {
		if (chan_list[i].chanspec == chan) {
			chan_list[i].ref_cnt--;
			if (chan_list[i].ref_cnt == 0) {
				chan_list[i].chanspec = 0;
				/* Delete PHY cache if nobody is on this channel */
#ifdef PHYCAL_CACHING
				if (need_delete)
					phy_chanmgr_destroy_ctx((phy_info_t *)wlc->pi, chan);
#endif
				/* swap the deleted channel with the last channel to remove the
				 * gap in the array.
				 */
				if (i != num_chan - 1) {
					chan_list[i].chanspec = chan_list[num_chan-1].chanspec;
					chan_list[i].ref_cnt = chan_list[num_chan-1].ref_cnt;
					chan_list[num_chan-1].chanspec = 0;
					chan_list[num_chan-1].ref_cnt = 0;
				}

				sbi->chan_cal_info->num_channels--;
			}
			ret = BCME_OK;
		}
	}
	return ret;
}

void wlc_slotted_bss_phy_calmode_set(wlc_info_t *wlc, uint8 newmode)
{
	wlc_slotted_bss_info_t *sbi = wlc->sbi;
	uint8 cur_mode = phy_calmgr_get_calmode((phy_info_t *)WLC_PI(wlc));
	if (cur_mode != newmode) {
		sbi->chan_cal_info->phy_cal_mode = newmode;
		phy_calmgr_set_calmode((phy_info_t *)WLC_PI(wlc), newmode);
	}
}

void wlc_slotted_bss_phy_calmode_restore(wlc_info_t *wlc)
{
	int i;
	wlc_slotted_bss_info_t *sbi = wlc->sbi;

	/* Check if any channel is still in use, which means it still needs to be calibrated */
	for (i = 0; i < sbi->chan_cal_info->num_channels; i++) {
		if (sbi->chan_cal_info->chan_list[i].ref_cnt != 0)
			return;
	}

	phy_calmgr_set_calmode((phy_info_t *)WLC_PI(wlc),
		sbi->chan_cal_info->phy_cal_mode);
}

uint8
wlc_slotted_bss_get_op_rxstreams(wlc_bsscfg_t *cfg, chanspec_t chanspec)
{
	wlc_info_t *wlc = cfg->wlc;

	/* TODO: There is an inherent assumption here that RxStreams is constant across time.
	 * May or may not be true depending on chip (say 3x3 + 2x2 or 3x3 + 1x1).
	 * So, need to update it correctly
	 */

	BCM_REFERENCE(chanspec);
	return wlc->stf->op_rxstreams;
}

/* This function searches for all scbs on both the cfgs (for RSDB, it will search on two).
 * If SCB's respective peer is in current msch cb chanspec,
 *   a. If scb bandunit is not same as chanspec bandunit, does an scb move orr scb switch.
 *   b. Once all the scb's are moved current chanspec's corresponding cfg, rate_set_init is called
 * for all scbs on that cfg.
 *
 * Interface open/close (MAC open/close) will done for scb's with peer available in current msch cb
 * chanspec. Txqueue start is done, if scbs are present and it is not in active state.
 */
int
wlc_slotted_bss_datapath_update(wlc_slotted_bss_info_t *sbi, wlc_bsscfg_t *bsscfg,
		chanspec_t chanspec, wlfc_ctl_type_t open_close)
{
	wlc_info_t *wlc = bsscfg->wlc;
	int ret = BCME_OK;
	struct scb *scb;
	struct scb_iter scbiter;
	bool scbs_present = FALSE;
	bsscfg_slotted_bss_type_cubby_t *slotted_bss =
		BSSCFG_SLOTTED_BSS_TYPE_CUBBY(sbi, bsscfg);

	/* In case of RSDB wlc_bsscfg_rateset_init() is not required as
	 * there will two bsscfg, one for each wlc/band
	 */
	if ((!RSDB_ENAB(wlc->pub)) &&
			(CHSPEC_BAND(bsscfg->current_bss->chanspec) != CHSPEC_BAND(chanspec))) {
		if (wlc_bsscfg_rateset_init(wlc, bsscfg, WLC_RATES_OFDM,
				(CHSPEC_BW(chanspec) >> WL_CHANSPEC_BW_SHIFT),
				wlc_get_mcsallow(wlc, bsscfg)) != BCME_OK) {
			WL_ERROR(("bsscfg rteset init failed for cfg %p\n", bsscfg));
		}
	}
	bsscfg->current_bss->chanspec =
		bsscfg->target_bss->chanspec = chanspec;

	/* Search for scbs over the target cfg */
	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		wlc_slotted_bss_switch_scb_and_if(wlc->sbi, bsscfg, scb, chanspec);
	}
#ifdef WLRSDB
	if (RSDB_ENAB(wlc->pub)) {
		wlc_bsscfg_t *linked_cfg;

		/* Search for scbs over linked cfg as well */
		linked_cfg = wlc_rsdb_bsscfg_get_linked_cfg(wlc->rsdbinfo, bsscfg);
		FOREACH_BSS_SCB(linked_cfg->wlc->scbstate, &scbiter, linked_cfg, scb) {
			wlc_slotted_bss_switch_scb_and_if(wlc->sbi, bsscfg, scb, chanspec);
		}
	}
#endif /* WLRSDB */
	/* All the scbs are moved to target cfg */
	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		scbs_present = TRUE;
		if (slotted_bss->peer_scb_reinit_fn != NULL) {
			(slotted_bss->peer_scb_reinit_fn)(slotted_bss->ctx, bsscfg, scb);
		}
	}
	/* If there are no scbs, no need to start txqueue */
	if (!scbs_present) {
		return ret;
	}
	wlc_slotted_bss_update_peers(wlc->sbi, bsscfg, chanspec,
			open_close, FALSE);
	if (!wlc_txqueue_active(wlc, bsscfg)) {
		wlc_txqueue_start(wlc, bsscfg, chanspec, NULL);
	}

	return ret;
}

/* This function does the scb switch band/scb move to other cfg.
 * This function also does the interface update to the current
 * chanspec's bsscfg.
 */
int
wlc_slotted_bss_switch_scb_and_if(wlc_slotted_bss_info_t *sbi, wlc_bsscfg_t *cfg,
		struct scb *scb, chanspec_t chanspec)
{
	bsscfg_slotted_bss_type_cubby_t *slotted_bss =
		BSSCFG_SLOTTED_BSS_TYPE_CUBBY(sbi, cfg);

	if (slotted_bss->peer_avail_fn != NULL) {
		/* if peer is not available in the channel, return */
		if (!((slotted_bss->peer_avail_fn)(slotted_bss->ctx, scb, chanspec))) {
			return BCME_OK;
		}
	} else {
		ASSERT(0);
	}

	if (CHSPEC_BANDUNIT(chanspec) != scb->bandunit) {
#ifdef WLRSDB
		if (RSDB_ENAB(cfg->wlc->pub)) {
			wlc_bsscfg_t *linked_cfg = wlc_rsdb_bsscfg_get_linked_cfg(
					cfg->wlc->rsdbinfo, cfg);
			wlc_rsdb_move_scb(linked_cfg, cfg, scb, TRUE);
		} else
#endif
		{
			wlc_scb_switch_band(cfg->wlc, scb, CHSPEC_BANDUNIT(chanspec), cfg);
		}
	}
#ifdef WLRSDB
	if (RSDB_ENAB(cfg->wlc->pub)) {
		wlc_rsdb_switch_if_to_linked_cfg(cfg->wlc->rsdbinfo, cfg);
	}
#endif
	return BCME_OK;
}
#endif /* WLSLOTTED_BSS */
