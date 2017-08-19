/**
 * @file
 * @brief
 * WLC NDIS module (TO BE RENAMED TO wlc_ndis.c)
 * - iovars/ioctls that are used in between wlc and per port
 * - ndis specific code used in common code by other modules
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$*
 *
 */

#include <wlc_cfg.h>

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <siutils.h>
#include <d11.h>
#include <wlioctl.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_ndis_iovar.h>
#include <wlc_keymgmt.h>
#include <wlc_scb.h>
#include <wlc_ap.h>
#include <wlc_scan.h>
#include <bcmendian.h>
#include <wlc_assoc.h>
#include <bcmwpa.h>
#include <epivers.h>
#include <wlc_ht.h>
#include <wlc_prot_n.h>
#include <wlc_event_utils.h>
#include <wlc_pm.h>
#include <wlc_frmutil.h>
#include <wlc_misc.h>
#include <wlc_wpa.h>
#include <wlc_dump.h>
#include <wlc_tx.h>
#include <wlc_iocv.h>

static int wlc_ndis_doiovar(void *hdl, uint32 actionid,
	void *params, uint p_len, void *arg, uint len, uint val_size,
	wlc_if_t *wlcif);
static void wlc_iov_wlif_stats_get(wlc_if_t *wlcif, wl_if_stats_t *wlif_stats);

enum {
	IOV_KEY_REMOVE_ALL = 1,
	IOV_ASSOC_DECISION, /* association decision */
	IOV_IF_ADD,
	IOV_IF_DEL,
	IOV_IF_PARAMS,
	IOV_BSSCFG,
	IOV_DEFAULT_RATESET,
	IOV_APS_ASSOC,
	IOV_STA_ASSOC,
	IOV_BSSCFG_FLAGS,
	IOV_IF_COUNTERS,
	IOV_SCAN_IN_PROGRESS,
	IOV_BCN_PRB,

	/* These are XP specific iovars */
	IOV_STATISTICS,
	IOV_SET_NMODE,
	IOV_KEY_ABSENT,
	IOV_PROT_N_MODE_RESET,
	IOV_SET_SSID,
	/* End of XP specific iovars */

	IOV_MEDIA_STREAM_MODE = 101,
	IOV_USE_GROUP_ENABLED = 102,
	IOV_WAKE_EVENT_ENABLE = 103,
	IOV_WAKE_EVENT_STATUS = 104,
	IOV_SCAN_SUPPRESS_SSID = 105, /* Flag: suppress ssids of hidden APs */
	IOV_EXEMPT_LIST = 106,
	IOV_ILLED_AP = 107,
	IOV_PS_ALLOWED = 108,
	IOV_PACKET_FILTER = 109,
	IOV_EXEMPT_M4 = 110,
	IOV_SAFE_MODE_ENABLE = 111,
	IOV_WAKEUP_SCAN = 112,

	IOV_LAST        /* In case of a need to check max ID number */
};

const bcm_iovar_t ndis_iovars[] = {
	{"key_remove_all", IOV_KEY_REMOVE_ALL,
	(0), 0, IOVT_BUFFER, 0
	},
	{"assoc_decision", IOV_ASSOC_DECISION,
	(0), 0, IOVT_BUFFER, sizeof(assoc_decision_t)
	},
	{"if_add", IOV_IF_ADD,
	(IOVF_OPEN_ALLOW), 0, IOVT_BUFFER, (sizeof(wl_if_add_t))
	},
	{"if_del", IOV_IF_DEL,
	(IOVF_OPEN_ALLOW), 0, IOVT_BUFFER, (sizeof(int32))
	},
	{"if_params", IOV_IF_PARAMS,
	(0), 0, IOVT_BOOL, 0
	},
	{"bsscfg", IOV_BSSCFG,
	(0), 0, IOVT_BUFFER, sizeof(wl_bsscfg_t)
	},
	{"default_rateset", IOV_DEFAULT_RATESET,
	(0), 0, IOVT_BUFFER, (sizeof(wl_rates_info_t))
	},
	{"aps_assoc", IOV_APS_ASSOC,
	(0), 0, IOVT_UINT8, 0
	},
	{"stas_assoc", IOV_STA_ASSOC,
	(0), 0, IOVT_UINT8, 0
	},
	{"bsscfg_flags", IOV_BSSCFG_FLAGS,
	(0), 0, IOVT_UINT32, 0
	},
	{ "if_counters", IOV_IF_COUNTERS,
	(IOVF_OPEN_ALLOW), 0, IOVT_BUFFER, sizeof(wl_if_stats_t)
	},
	{ "scan_in_progress", IOV_SCAN_IN_PROGRESS,
	(0), 0, IOVT_BOOL, 0
	},
	{ "bcn_prb", IOV_BCN_PRB,
	(0), 0, IOVT_BUFFER, sizeof(int)
	},

	/* These are XP specific iovars */
#ifdef WLC_NDIS5IOVAR

#ifdef WLCNT
	{"statistics", IOV_STATISTICS,
	(0), 0, IOVT_BUFFER, sizeof(wl_cnt_info_t)
	},
#endif /* WLCNT */
	{"set_nmode", IOV_SET_NMODE,
	(0), 0, IOVT_UINT32, 0
	},
	{"key_absent", IOV_KEY_ABSENT,
	(0), 0, IOVT_BOOL, 0
	},
	{"prot_n_mode_reset", IOV_PROT_N_MODE_RESET,
	(0), 0, IOVT_UINT32, 0
	},
	{ "set_ssid", IOV_SET_SSID,
	(0), 0, IOVT_BUFFER, sizeof(wlc_ssid_t)
	},

#endif /* WLC_NDIS5IOVAR */
	/* End of XP specific iovars */

	/* start EXT_STA */
	{"media_stream_mode", IOV_MEDIA_STREAM_MODE, 0, 0, IOVT_BOOL, 0},
	{"use_group_enabled", IOV_USE_GROUP_ENABLED, 0, 0, IOVT_BOOL, 0},
	{"wake_event_enable", IOV_WAKE_EVENT_ENABLE, 0, 0, IOVT_UINT32, 0},
	{"wake_event_status", IOV_WAKE_EVENT_STATUS, 0, 0, IOVT_UINT32, 0},
	{"scan_suppress_ssid", IOV_SCAN_SUPPRESS_SSID, 0, 0, IOVT_BOOL, 0},
	{"exempt_list", IOV_EXEMPT_LIST, 0, 0, IOVT_BUFFER, WLC_EXEMPT_LIST_LEN(0)},
	{"illed_ap", IOV_ILLED_AP, 0, 0, IOVT_UINT32, 0},
	{"ps_allowed", IOV_PS_ALLOWED, 0, 0, IOVT_UINT32, 0},
	{"packet_filter", IOV_PACKET_FILTER, 0, 0, IOVT_UINT32, 0},
	{"exempt_m4", IOV_EXEMPT_M4, 0, 0, IOVT_BOOL, 0},
	{"safe_mode_enable", IOV_SAFE_MODE_ENABLE, 0, 0, IOVT_BOOL, 0},
	{"wakeup_scan", IOV_WAKEUP_SCAN, 0, 0, IOVT_BOOL, 0},
	/* end EXT_STA */

	{ NULL, 0, 0, 0, 0, 0 }
};

/* module info */
struct wlc_ndis_info {
	wlc_info_t *wlc;
	int cfgh;
};

/* start EXT_STA */
/* ioctl table */
static const wlc_ioctl_cmd_t ndis_ioctls[] = {
	{WLC_SOFT_RESET, 0, 0},
	{WLC_GET_ALLOW_MODE, 0, 0},
	{WLC_SET_ALLOW_MODE, 0, 0},
	{WLC_DISASSOC_MYAP, 0, 0}
};

/* bsscfg cubby */
typedef struct {
	exempt_list_t	*exempt_list;	/**< exempt list */
	bool	exempt_unencrypt_m4;
	uint	packet_filter;		/**< packet filter */
	bool	ar_disassoc;		/**< disassociated in associated recreation */
} bss_ndis_info_t;

#define BSS_INFO(ndis, cfg) (bss_ndis_info_t *)BSSCFG_CUBBY(cfg, (ndis)->cfgh)

/* local functions */
static void wlc_disassociate_ap(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, scb_val_t *scb_val);
static void wlc_disassociate_myap(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb,
	struct ether_addr *ea, uint16 reason, bool clear_bssid);

/* bsscfg cubby */
static void
wlc_ndis_bss_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_ndis_info_t *ndis = ctx;
	wlc_info_t *wlc = ndis->wlc;
	bss_ndis_info_t *bni = BSS_INFO(ndis, cfg);

	/* free exempt list */
	if (WLEXTSTA_ENAB(wlc->pub)) {
		if (bni->exempt_list) {
			MFREE(wlc->osh, bni->exempt_list,
			      WLC_EXEMPT_LIST_LEN(bni->exempt_list->entries));
			bni->exempt_list = NULL;
		}
	}
}

/* return FALSE if fail exemption check */
bool
wlc_exempt_pkt(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb,
	struct wlc_frminfo *f, uint16 ether_type, uint8 prio)
{
	uint i;
	exempt_t *entry;
	wlc_key_info_t key_info;
	bss_ndis_info_t *bni = BSS_INFO(wlc->ndis, bsscfg);

	if (bni->exempt_list != NULL) {
		entry = &bni->exempt_list->exempt[0];
		/* apply exemption rule to pkt */
		for (i = 0; i < bni->exempt_list->entries; i++) {
			if (hton16(ether_type) != entry[i].exempt_type)
				continue;
			/* check if pkt type matches */
			if ((entry[i].exempt_pkttype != WLC_EXEMPT_BOTH) &&
				((f->ismulti && entry[i].exempt_pkttype != WLC_EXEMPT_MULTICAST) ||
				(!f->ismulti && entry[i].exempt_pkttype != WLC_EXEMPT_UNICAST)))
				continue;
			/* take action if apply */
			if (entry[i].exempt_action == WLC_EXEMPT_ALWAYS) {
				if (f->rx_wep) {
					WL_WSEC(("wl%d: %s: drop encrypted pkt with "
						"ether type 0x%x\n", wlc->pub->unit,
						__FUNCTION__, ether_type));
					return FALSE;
				}
			} else if (entry[i].exempt_action == WLC_EXEMPT_NO_KEY) {
				wlc_keymgmt_get_scb_key(wlc->keymgmt, scb, WLC_KEY_ID_PAIRWISE,
					WLC_KEY_FLAG_NONE, &key_info);
#ifdef BCMWAPI_WPI
				if (key_info.algo == CRYPTO_ALGO_OFF) {
					wlc_keymgmt_get_scb_key(wlc->keymgmt, scb,
						WLC_KEY_ID_PAIRWISE + 1, WLC_KEY_FLAG_NONE,
						&key_info);
				}
#endif /* BCMWAPI_WPI */

				if (!f->rx_wep && !f->ismulti &&
					key_info.algo != CRYPTO_ALGO_OFF) {
					uint body_offset =
						(uint)(f->pbody - (uchar*)PKTDATA(wlc->osh, f->p));
					if (!bsscfg->BSS && bni->exempt_unencrypt_m4 &&
						wlc_is_4way_msg(wlc, f->p, body_offset, PMSG4)) {
						WL_WSEC(("wl%d: %s: allowing unencrypted 802.1x "
							"4-Way M4\n",
							wlc->pub->unit, __FUNCTION__));
						return TRUE;
					}
					WL_WSEC(("wl%d: %s: drop unencrypted pkt with "
						"ether type 0x%x\n", wlc->pub->unit,
						__FUNCTION__, ether_type));
					return FALSE;
				}
			} else
				ASSERT(0);

			break;
		}
	}

	return TRUE;
}

/* return FALSE if fail filter */
bool
wlc_packet_filter(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct wlc_frminfo *f)
{
	bss_ndis_info_t *bni = BSS_INFO(wlc->ndis, cfg);
	uint packet_filter = bni->packet_filter;

	if (!f->ismulti) {
		if (!(packet_filter & WLC_ACCEPT_DIRECTED) ||
		    bcmp(f->h->a1.octet, (char*)&cfg->cur_etheraddr, ETHER_ADDR_LEN) != 0)
			goto toss;
	}
	else if (ETHER_ISBCAST(&(f->h->a1))) {
		if (!(packet_filter & WLC_ACCEPT_BROADCAST))
			goto toss;
	}
	else if (!(packet_filter & WLC_ACCEPT_ALL_MULTICAST)) {
		if (packet_filter & WLC_ACCEPT_MULTICAST) {
			if (wlc_bsscfg_mcastfilter(cfg, &(f->h->a1)))
				goto toss;
		}
		else
			goto toss;
	}

	return TRUE;
toss:
	return FALSE;
}

/* ioctl dispatcher */
static int
wlc_ndis_doioctl(void *ctx, uint cmd, void *arg, uint len, struct wlc_if *wlcif)
{
	wlc_ndis_info_t *ndis = ctx;
	int err = BCME_OK;
	wlc_info_t *wlc = ndis->wlc;
	wlc_bsscfg_t *bsscfg;
	int val = 0, *pval;

	/* update bsscfg pointer */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* default argument is generic integer */
	pval = (int *) arg;

	/* This will prevent the misaligned access */
	if ((uint32)len >= sizeof(val))
		bcopy(pval, &val, sizeof(val));

	switch (cmd) {

	case WLC_SOFT_RESET:
		if (val) {
			wlc_bss_mac_event(wlc, bsscfg, WLC_E_RESET_COMPLETE, NULL,
			            WLC_E_STATUS_SUCCESS, 0, 0, 0, 0);
		} else {
			wlc_txq_info_t *qi;

			/* when reset AP interface do not abort STA roam or association */
			if (BSSCFG_STA(bsscfg)) {
				wlc_bsscfg_t *scan_cfg = wlc_scan_bsscfg(wlc->scan);
				if (bsscfg == scan_cfg)
					/* cancel any scan in progress in this interface */
					wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);
				/* cancel any assoc in progress */
				wlc_assoc_abort(bsscfg);
			}

			qi = wlc->tx_queues;

			/* clear common txq for if */
			wlc_txq_pktq_cfg_filter(wlc, WLC_GET_TXQ(qi), bsscfg);

			if (BSSCFG_STA(bsscfg))
				/* disassociate */
				wlc_disassociate_client(bsscfg, TRUE);
			else
				wlc_bsscfg_disable(wlc, bsscfg);
		}
		break;

	case WLC_GET_ALLOW_MODE:
		*pval = wlc->allow_mode;
		break;

	case WLC_SET_ALLOW_MODE:
		wlc->allow_mode = val;
		break;

	case WLC_DISASSOC_MYAP:
		wlc_disassociate_ap(wlc, bsscfg, (scb_val_t *)arg);
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static void
wlc_disassociate_ap(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, scb_val_t *scb_val)
{
	struct scb *scb;
	struct scb_iter scbiter;
	struct ether_addr *ea = &bsscfg->BSSID;
	uint16 reason =  DOT11_RC_DISASSOC_LEAVING;
	bool clear_bssid = TRUE;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif

	/* if scb_val exist use address and reason */
	if (scb_val != NULL) {
		ea = &(scb_val->ea);
		reason = (uint16)scb_val->val;
		clear_bssid = FALSE;
	}

	/* if AP down */
	if (ETHER_ISNULLADDR(bsscfg->BSSID.octet) || bsscfg->up == FALSE) {
		/* no need to deauth/disassoc...already gone */
		WL_INFORM(("wl: %s: no need to deauth/disassoc...already gone", __FUNCTION__));
		return;
	}

	if (ETHER_ISBCAST(ea)) {
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
			WL_INFORM(("wl: %s: send unicast deauth, %s\n",
				__FUNCTION__, bcm_ether_ntoa(&scb->ea, eabuf)));
			wlc_disassociate_myap(wlc, bsscfg, scb, &scb->ea, reason, clear_bssid);
		}
	} else {
		scb = wlc_scbfind(wlc, bsscfg, ea);
		if (scb) {
			WL_INFORM(("wl: %s: send unicast deauth, %s\n", __FUNCTION__,
				bcm_ether_ntoa(&scb->ea, eabuf)));
			wlc_disassociate_myap(wlc, bsscfg, scb, &scb->ea, reason, clear_bssid);
		} else {
			WL_INFORM(("wl%d: associated station %s not found\n",
				wlc->pub->unit,	bcm_ether_ntoa((struct ether_addr *)ea, eabuf)));
		}
	}
} /* wlc_disassociate_ap */

static void
wlc_disassociate_myap(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb,
	struct ether_addr *ea, uint16 reason, bool clear_bssid)
{
	ASSERT(scb != NULL);

	if (SCB_AUTHENTICATED(scb) == FALSE || SCB_ASSOCIATED(scb) == FALSE)
		return;

	/* send disassoc packet */
	wlc_senddisassoc(wlc, bsscfg, scb, ea, &bsscfg->BSSID,
	                 &bsscfg->cur_etheraddr, reason);
	wlc_scb_disassoc_cleanup(wlc, scb);		/* cleanup keys, etc */

	/* change scb state */
	wlc_scb_clearstatebit(wlc, scb, ASSOCIATED | AUTHORIZED);

	wlc_disassoc_complete(bsscfg, WLC_E_STATUS_SUCCESS, ea,
		reason, DOT11_BSSTYPE_INFRASTRUCTURE);

	/* force STA to roam (allowing it to roam back) */
	if (clear_bssid) {
		wlc_bss_clear_bssid(bsscfg);
	}
}

static void
wlc_ibss_bsscfg_disassoc_peer(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	struct scb_iter scbiter;
	struct scb *scb;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
		if (SCB_IS_IBSS_PEER(scb)) {
			SCB_UNSET_IBSS_PEER(scb);
			wlc_scb_disassoc_cleanup(wlc, scb);
			wlc_bss_mac_event(wlc, cfg, WLC_E_DISASSOC, &scb->ea,
				WLC_E_STATUS_SUCCESS, DOT11_RC_DISASSOC_LEAVING,
				cfg->current_bss->bss_type, 0, 0);
		}
	}
}

static void
wlc_ndis_bss_updn(void *ctx, bsscfg_up_down_event_data_t *evt)
{
	wlc_ndis_info_t *ndis = ctx;
	wlc_info_t *wlc = ndis->wlc;
	wlc_bsscfg_t *cfg = evt->bsscfg;
	bss_ndis_info_t *bni = BSS_INFO(ndis, cfg);

	if (!evt->up &&
	    WLEXTSTA_ENAB(wlc->pub) &&
	    !WOWL_ACTIVE(wlc->pub) &&
	    ASSOC_RECREATE_ENAB(wlc->pub) &&
	    (cfg->flags & WLC_BSSCFG_PRESERVE)) {
		struct scb *scb = wlc_scbfind(wlc, cfg, &cfg->BSSID);
		if (scb) {
			wlc_scb_disassoc_cleanup(wlc, scb);
			if (SCB_ASSOCIATED(scb)) {
				wlc_scb_clearstatebit(wlc, scb, ASSOCIATED);
				wlc_bss_mac_event(wlc, cfg, WLC_E_DISASSOC, &cfg->BSSID,
					WLC_E_STATUS_SUCCESS, DOT11_RC_DISASSOC_LEAVING,
					cfg->current_bss->bss_type, 0, 0);
			}
			bni->ar_disassoc = TRUE;
		}
		if (!cfg->BSS)
			wlc_ibss_bsscfg_disassoc_peer(wlc, cfg);
	}
}

bool
wlc_ndis_ar_disassoc_st(wlc_ndis_info_t *ndis, wlc_bsscfg_t *cfg)
{
	bss_ndis_info_t *bni = BSS_INFO(ndis, cfg);

	return bni->ar_disassoc;
}

void
wlc_ndis_disassoc_complete(wlc_ndis_info_t *ndis, wlc_bsscfg_t *cfg)
{
	bss_ndis_info_t *bni = BSS_INFO(ndis, cfg);

	if (!bni->ar_disassoc) {
		/* indicate DISASSOC due to unreachability */
		wlc_disassoc_complete(cfg, WLC_E_STATUS_SUCCESS,
		                      &cfg->prev_BSSID, DOT11_RC_INACTIVITY,
		                      DOT11_BSSTYPE_INFRASTRUCTURE);
	}
	bni->ar_disassoc = FALSE;
}
/* end EXT_STA */

/* attach/detach */
wlc_ndis_info_t *
BCMATTACHFN(wlc_ndis_attach)(wlc_info_t *wlc)
{
	wlc_ndis_info_t *ndis;

	if ((ndis = MALLOCZ(wlc->osh, sizeof(*ndis))) == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, allocated %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	ndis->wlc = wlc;

#if defined(EXT_STA)

	BCM_REFERENCE(wlc_ndis_bss_updn);
	BCM_REFERENCE(wlc_ndis_bss_deinit);
	BCM_REFERENCE(wlc_ndis_doioctl);
#if !defined(EXT_STA_DISABLED)
	/* register bsscfg cubby as a way to clean up the bss private data */
	if ((ndis->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(bss_ndis_info_t), NULL,
			wlc_ndis_bss_deinit, NULL, ndis)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register bsscfg up/down callback */
	if (wlc_bsscfg_updown_register(wlc, wlc_ndis_bss_updn, ndis) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* ioctl dispatcher requires a unique "hdl" */
	if (wlc_module_add_ioctl_fn(wlc->pub, ndis, wlc_ndis_doioctl,
			ARRAYSIZE(ndis_ioctls), ndis_ioctls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_add_ioctl_fn() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	wlc->pub->_extsta = TRUE;
#endif /* EXT_STA_DISABLED */
#endif /* EXT_STA */

	if (wlc_module_register(wlc->pub, ndis_iovars, "wlc_ndis", ndis, wlc_ndis_doiovar,
			NULL, NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	return ndis;

fail:
	wlc_ndis_detach(ndis);
	return NULL;
}

void
BCMATTACHFN(wlc_ndis_detach)(wlc_ndis_info_t *ndis)
{
	wlc_info_t *wlc;

	if (ndis == NULL)
		return;

	wlc = ndis->wlc;

	(void)wlc_module_unregister(wlc->pub, "wlc_ndis", ndis);

#if defined(EXT_STA) && !defined(EXT_STA_DISABLED)
	(void)wlc_module_remove_ioctl_fn(wlc->pub, ndis);
	(void)wlc_bsscfg_updown_unregister(wlc, wlc_ndis_bss_updn, ndis);
#endif

	MFREE(wlc->osh, ndis, sizeof(*ndis));
}

/* iovar dispatcher */
/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */

#include <wlc_patch.h>

static int
wlc_ndis_doiovar(void *hdl, uint32 actionid,
	void *params, uint p_len, void *arg, uint len, uint val_size, struct wlc_if *wlcif)
{
	wlc_ndis_info_t *ndis = hdl;
	wlc_info_t *wlc = ndis->wlc;
	int err = BCME_OK;
	bool bool_val;
	bss_ndis_info_t *bni;
	int32 int_val = 0;
	wlc_bsscfg_t *bsscfg;
	int32 *ret_int_ptr;

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;
	bni = BSS_INFO(ndis, bsscfg);
	ret_int_ptr = (int32 *)arg;

	switch (actionid) {

	case IOV_SVAL(IOV_KEY_REMOVE_ALL):
		if (wlc->pub->up) {
			wlc_keymgmt_reset(wlc->keymgmt, bsscfg, NULL);
		}
		break;

#if defined(AP) && defined(EXT_STA)
	case IOV_SVAL(IOV_ASSOC_DECISION): {
		assoc_decision_t dc;

		bcopy(arg, &dc, sizeof(assoc_decision_t));
		wlc_ap_process_assocreq_decision(wlc, bsscfg, &dc);
		break;
	}
#endif /* AP && EXT_STA */

	case IOV_SVAL(IOV_IF_ADD): {
		int idx;
		wlc_bsscfg_t *cfg;
		wl_if_add_t if_buf;
		wlc_bsscfg_type_t type = {BSSCFG_TYPE_GENERIC, BSSCFG_SUBTYPE_NONE};

		if ((uint)len < sizeof(wl_if_add_t)) {
			WL_ERROR(("wl%d: input buffer too short\n", wlc->pub->unit));
			err = BCME_BUFTOOSHORT;
			break;
		}

		bcopy((char *)arg, (char*)&if_buf, sizeof(wl_if_add_t));
		/* allocate bsscfg */
		if ((idx = wlc_bsscfg_get_free_idx(wlc)) == BCME_ERROR) {
			WL_ERROR(("wl%d: no free bsscfg\n", wlc->pub->unit));
			err = BCME_NORESOURCE;
			break;
		}

		type.subtype = if_buf.ap != 0 ? BSSCFG_GENERIC_AP : BSSCFG_GENERIC_STA;
		cfg = wlc_bsscfg_alloc(wlc, idx, &type, if_buf.bsscfg_flags, &if_buf.mac_addr);
		if (!cfg) {
			WL_ERROR(("wl%d: can not allocate bsscfg\n", wlc->pub->unit));
			err = BCME_NORESOURCE;
			break;
		}

		/* init as sta to match init op mode */
		if (wlc_bsscfg_init(wlc, cfg) != BCME_OK) {
			WL_ERROR(("wl%d: can not init bsscfg\n", wlc->pub->unit));
			wlc_bsscfg_free(wlc, cfg);
			err = BCME_ERROR;
			break;
		}

		cfg->wlcif->if_flags |= if_buf.if_flags;
		/* Win7 does BSS bridging */
		cfg->ap_isolate = TRUE;
#ifdef BCMDBG
		printf("wl%d: if_add success\n", wlc->pub->unit);
#endif
		break;
	}

	case IOV_SVAL(IOV_IF_DEL): {
		if (WLC_BSSCFG_IDX(bsscfg) != 0) {
			if (bsscfg->enable) {
				wlc_bsscfg_disable(wlc, bsscfg);
			}
			wlc_bsscfg_free(wlc, bsscfg);
		} else {
			WL_ERROR(("wl%d: if_del failed: not delete primary bsscfg\n",
				wlc->pub->unit));
		}
		break;
	}

	case IOV_SVAL(IOV_IF_PARAMS):
		bsscfg->wlcif->if_flags |= int_val;
		break;

	case IOV_GVAL(IOV_BSSCFG): {
		wl_bsscfg_t *p = (wl_bsscfg_t *)arg;
		wlc_key_id_t key_id;

		ASSERT(p);

		if ((uint)len < sizeof(wl_bsscfg_t)) {
			WL_ERROR(("wl%d: input buffer too short: len %d\n", wlc->pub->unit, len));
			err = BCME_BUFTOOSHORT;
			break;
		}
		p->bsscfg_idx = (uint32)bsscfg->_idx;
		p->wsec = (uint32)bsscfg->wsec;
		p->WPA_auth = (uint32)bsscfg->WPA_auth;

		key_id = wlc_keymgmt_get_bss_tx_key_id(wlc->keymgmt, bsscfg, FALSE);
		p->wsec_index = (key_id == WLC_KEY_ID_INVALID) ? -1 : (uint32)key_id;

		p->associated = (uint32)bsscfg->associated;
		p->BSS = (uint32)bsscfg->BSS;
		p->phytest_on = wlc->pub->phytest_on;
		p->assoc_state = bsscfg->assoc ? bsscfg->assoc->state : AS_VALUE_NOT_SET;
		p->assoc_type = bsscfg->assoc ? bsscfg->assoc->type : AS_VALUE_NOT_SET;
		bcopy(&bsscfg->prev_BSSID, &p->prev_BSSID, sizeof(struct ether_addr));
		bcopy(&bsscfg->BSSID, &p->BSSID, sizeof(struct ether_addr));
		p->targetbss_wpa2_flags = 0;
		if (bsscfg->target_bss) {
			p->targetbss_wpa2_flags = bsscfg->target_bss->wpa2.flags;
		}
		break;
	}

	case IOV_GVAL(IOV_DEFAULT_RATESET): {
		wl_rates_info_t *rates_info = (wl_rates_info_t *)params;
		wl_rates_info_t *rates_out = (wl_rates_info_t *)arg;
		wlc_rateset_t rs_tgt;

		bzero(&rs_tgt, sizeof(wlc_rateset_t));

		wlc_rateset_default(&rs_tgt, NULL, rates_info->phy_type,
			rates_info->bandtype, rates_info->cck_only, rates_info->rate_mask,
			rates_info->mcsallow, rates_info->bw, rates_info->txstreams);

		/* copy fields to return struct */
		rates_out->rs_tgt.count = rs_tgt.count;
		memcpy(&rates_out->rs_tgt.rates, &rs_tgt.rates, WLC_NUMRATES * sizeof(uint8));
		break;
	}

	case IOV_GVAL(IOV_APS_ASSOC):
		*ret_int_ptr = wlc->aps_associated;
		break;

	case IOV_GVAL(IOV_STA_ASSOC):
		*ret_int_ptr = wlc->stas_associated;
		break;

	case IOV_GVAL(IOV_BSSCFG_FLAGS):
		*ret_int_ptr = bsscfg->flags;
		break;

	case IOV_SVAL(IOV_BSSCFG_FLAGS):
		bsscfg->flags |= int_val;
		break;

	case IOV_GVAL(IOV_IF_COUNTERS):
		wlc_iov_wlif_stats_get(wlcif, (wl_if_stats_t *)arg);
		break;

	case IOV_GVAL(IOV_SCAN_IN_PROGRESS):
		*ret_int_ptr = ANY_SCAN_IN_PROGRESS(wlc->scan);
		break;

	case IOV_GVAL(IOV_BCN_PRB):
		if ((uint)len < sizeof(uint32)+bsscfg->current_bss->bcn_prb_len) {
			return BCME_BUFTOOSHORT;
		}
		bcopy(&bsscfg->current_bss->bcn_prb_len, (char*)arg, sizeof(uint32));
		bcopy((char*)bsscfg->current_bss->bcn_prb, (char*)arg + sizeof(uint32),
			bsscfg->current_bss->bcn_prb_len);
#if defined(BCMDBG)
		printf("bcn_prb_len: %d\n", bsscfg->current_bss->bcn_prb_len);
		prhex("bcn_prb", (void*)bsscfg->current_bss->bcn_prb,
			bsscfg->current_bss->bcn_prb_len);
#endif /* BCMDBG */
		break;

	/* These are XP specific iovars */
#ifdef WLC_NDIS5IOVAR

#ifdef WLCNT
	case IOV_GVAL(IOV_STATISTICS):
		wlc_statsupd(wlc);
		err = wlc_get_all_cnts(wlc, arg, len);
		break;
#endif /* WLCNT */

#ifdef WL11N
	case IOV_SVAL(IOV_SET_NMODE):
		err = wlc_set_nmode(wlc->hti, int_val);
		break;
#endif /* WL11N */

	case IOV_GVAL(IOV_KEY_ABSENT): {
		wlc_key_info_t key_info;
		wlc_keymgmt_get_key_by_addr(wlc->keymgmt,
				bsscfg, &bsscfg->BSSID, WLC_KEY_FLAG_NONE, &key_info);
		*ret_int_ptr = (key_info.algo == CRYPTO_ALGO_OFF);
		break;
	}

	case IOV_SVAL(IOV_PROT_N_MODE_RESET):
		wlc_prot_n_mode_reset(wlc->prot_n, (bool)int_val);
		break;

	case IOV_SVAL(IOV_SET_SSID): {
		wlc_ssid_t ssid_info;
		int i;
		wlc_bss_info_t *current_bss = NULL;
#ifdef BCMDBG
		char ssidbuf[256];
#endif
		memcpy(&ssid_info, (wlc_ssid_t *)arg, sizeof(wlc_ssid_t));
		if (ssid_info.SSID_len == 32 && (bsscfg->BSS == 1)) {
			/* search non-control chars */
			for (i = 0; i < 32; i++) {
				if (ssid_info.SSID[i] >= 0x20)
					break;
			}

			if (i == 32) {
				WL_OID(("wl%d: %s: got a bogus SSID, disassociating\n",
				        wlc->pub->unit, __FUNCTION__));

				/* do a disassociation instead of an SSID set */
				wlc_ioctl(wlc, WLC_DISASSOC, NULL, 0, NULL);
				wlc->mpc_oidjoin = FALSE;
				wlc_radio_mpc_upd(wlc);
				break;
			}
		}
		if (bsscfg->associated == 1 && !bsscfg->BSS) {
			current_bss = wlc_get_current_bss(bsscfg);
			if (ssid_info.SSID_len == current_bss->SSID_len &&
			    !bcmp(ssid_info.SSID, (char*)current_bss->SSID, ssid_info.SSID_len) &&
			    (((current_bss->capability & DOT11_CAP_PRIVACY) ==
			      (wlc->default_bss->capability & DOT11_CAP_PRIVACY)) &&
			     current_bss->beacon_period == wlc->default_bss->beacon_period &&
			     current_bss->atim_window == wlc->default_bss->atim_window &&
			     current_bss->chanspec == wlc->default_bss->chanspec)) {
				WL_OID(("\tNew SSID is the same as current, ignoring.\n"));
				break;
			}
		}
		wlc->mpc_oidjoin = TRUE;
		wlc_radio_mpc_upd(wlc);

		/* if can't up, we're done */
		if (!wlc->pub->up) {
			wlc->mpc_oidjoin = FALSE;
			wlc_radio_mpc_upd(wlc);
			break;
		}
		/* attempt to join a BSS with the requested SSID */
		/* but don't create an IBSS if IBSS Lock Out is turned on */
		if (!((wlc->ibss_allowed == FALSE) && (bsscfg->BSS == 0))) {
			wl_join_assoc_params_t *assoc_params = NULL;
			int assoc_params_len = 0;

#ifdef BCMDBG
			bcm_format_ssid(ssidbuf, ssid_info.SSID, ssid_info.SSID_len);
			WL_OID(("wl%d: %s: set ssid %s\n", wlc->pub->unit,
			        __FUNCTION__, ssidbuf));
#endif
			if ((uint)len >= WL_JOIN_PARAMS_FIXED_SIZE) {
				bool reset_chanspec_num = FALSE;
				uint16 bssid_cnt;
				int32 chanspec_num;

				assoc_params = &((wl_join_params_t *)arg)->params;
				assoc_params_len = len - OFFSETOF(wl_join_params_t, params);
				bssid_cnt = load16_ua(&assoc_params->bssid_cnt);
				chanspec_num = load32_ua(&assoc_params->chanspec_num);
				if (bssid_cnt && (chanspec_num == 0))
				{
					reset_chanspec_num = TRUE;
					store32_ua((uint8 *)&assoc_params->chanspec_num,
							bssid_cnt);
				}

				err = wlc_assoc_chanspec_sanitize(wlc,
					(chanspec_list_t *)&assoc_params->chanspec_num,
					len - OFFSETOF(wl_join_params_t,
					params.chanspec_num), bsscfg);

				if (reset_chanspec_num)
					store32_ua((uint8 *)&assoc_params->chanspec_num, 0);

				if (err != BCME_OK)
					break;
			}
			wlc_join(wlc, wlc->cfg, ssid_info.SSID, ssid_info.SSID_len,
			         NULL, assoc_params, assoc_params_len);
		}

		wlc->mpc_oidjoin = FALSE;
		wlc_radio_mpc_upd(wlc);
		break;
	}

#endif /* WLC_NDIS5IOVAR */
	/* End of XP specific iovars */

#if defined(EXT_STA)
	case IOV_GVAL(IOV_USE_GROUP_ENABLED):
		*ret_int_ptr = (int32)wlc->use_group_enabled;
		break;
	case IOV_SVAL(IOV_USE_GROUP_ENABLED):
		wlc->use_group_enabled = bool_val;
		break;
	case IOV_GVAL(IOV_WAKE_EVENT_ENABLE):
		if (!WLEXTSTA_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
			break;
		}
		*ret_int_ptr = wlc->pub->wake_event_enable;
		break;
	case IOV_SVAL(IOV_WAKE_EVENT_ENABLE):
		if (!WLEXTSTA_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
			break;
		}
		wlc->pub->wake_event_enable = int_val;
		wlc->pub->wake_event_status = 0;
		break;
	case IOV_GVAL(IOV_WAKE_EVENT_STATUS):
		if (!WLEXTSTA_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
			break;
		}
		*ret_int_ptr = wlc->pub->wake_event_status;
		wlc->pub->wake_event_enable = 0; /* disable wake event */
		break;
	case IOV_GVAL(IOV_MEDIA_STREAM_MODE):
		*ret_int_ptr = (int32)wlc->media_stream_mode;
		break;

	case IOV_SVAL(IOV_MEDIA_STREAM_MODE):
		if (bool_val && !wlc->media_stream_mode) {
			/* enable */
			wlc->media_stream_mode = TRUE;

			wlc->band->cached_roam_trigger = wlc->band->roam_trigger;
			wlc->band->roam_trigger = WLC_RSSI_MINVAL;
			if (NBANDS(wlc) > 1) {
				uint obu = OTHERBANDUNIT(wlc);

				wlc->bandstate[obu]->cached_roam_trigger =
					wlc->bandstate[obu]->roam_trigger;
				wlc->bandstate[obu]->roam_trigger = WLC_RSSI_MINVAL;
			}
		} else if (!bool_val && wlc->media_stream_mode) {
			/* disable */
			wlc->media_stream_mode = FALSE;

			wlc->band->roam_trigger = wlc->band->cached_roam_trigger;
			if (NBANDS(wlc) > 1) {
				uint obu = OTHERBANDUNIT(wlc);

				wlc->bandstate[obu]->roam_trigger =
					wlc->bandstate[obu]->cached_roam_trigger;
			}
		}
		break;

	case IOV_GVAL(IOV_SCAN_SUPPRESS_SSID):
		*ret_int_ptr = (int32)wlc->scan_suppress_ssid;
		break;

	case IOV_SVAL(IOV_SCAN_SUPPRESS_SSID):
		wlc->scan_suppress_ssid = bool_val;
		break;

	case IOV_GVAL(IOV_EXEMPT_LIST): {
		exempt_list_t *list = (exempt_list_t *)arg;
		if (bni->exempt_list) {
			int list_len = WLC_EXEMPT_LIST_LEN(bni->exempt_list->entries);
			if ((int)len < list_len) {
				err = BCME_BUFTOOSHORT;
				break;
			}
			bcopy(bni->exempt_list, list, list_len);
		} else
			list->entries = 0;
		break;
	}

	case IOV_SVAL(IOV_EXEMPT_LIST): {
		exempt_list_t *list = (exempt_list_t *)arg;
		if (len != WLC_EXEMPT_LIST_LEN(list->entries)) {
			err = BCME_ERROR;
			break;
		}
		/* free current exempt list */
		if (bni->exempt_list) {
			MFREE(wlc->osh, bni->exempt_list,
				WLC_EXEMPT_LIST_LEN(bni->exempt_list->entries));
			bni->exempt_list = NULL;
		}
		if (list->entries) {
			/* save input exempt list */
			bni->exempt_list = MALLOC(wlc->osh, len);
			if (bni->exempt_list == NULL) {
				err = BCME_NOMEM;
				break;
			}
			bcopy(list, bni->exempt_list, len);
		}
		break;
	}

	case IOV_GVAL(IOV_ILLED_AP):
		*ret_int_ptr = (uint32) (bsscfg->associated &&
			(bsscfg->pm->PM_override || bsscfg->pm->WME_PM_blocked ||
			!bsscfg->dtim_programmed));

		break;

	case IOV_GVAL(IOV_PS_ALLOWED):
		*ret_int_ptr = (uint32) wlc_ps_allowed(bsscfg);
		break;

	case IOV_GVAL(IOV_PACKET_FILTER):
		*ret_int_ptr = bni->packet_filter;
		break;

	case IOV_SVAL(IOV_PACKET_FILTER):
		bni->packet_filter = int_val;
		break;

	case IOV_GVAL(IOV_EXEMPT_M4):
		*ret_int_ptr = bni->exempt_unencrypt_m4;
		break;

	case IOV_SVAL(IOV_EXEMPT_M4):
		bni->exempt_unencrypt_m4 = bool_val;
		break;

	case IOV_GVAL(IOV_WAKEUP_SCAN):
		*ret_int_ptr = (int)wlc->wakeup_scan;

		break;

	case IOV_SVAL(IOV_WAKEUP_SCAN):
		wlc->wakeup_scan = bool_val;
		WL_INFORM(("%s(): set wlc->wakeup_scan to %d\n",
			__FUNCTION__, bool_val));

		break;

	case IOV_GVAL(IOV_SAFE_MODE_ENABLE):
		*ret_int_ptr = (int)bsscfg->safeModeEnabled;
		break;

	case IOV_SVAL(IOV_SAFE_MODE_ENABLE):
		if (BSSCFG_AP(bsscfg)) {
			WL_ERROR(("safe mode is only for EXT_STA mode!\n"));
			err = BCME_UNSUPPORTED;
			break;
		}

		if (bsscfg->safeModeEnabled == bool_val)
			break;

		bsscfg->safeModeEnabled = bool_val;

		break;
#endif /* EXT_STA */

	default:
		WL_ERROR(("%s(): actionid = %d unsupported\n", __FUNCTION__, actionid));
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static void
wlc_iov_wlif_stats_get(wlc_if_t *wlcif, wl_if_stats_t *wl_if_stats)
{
	if ((wlcif == NULL) || (wl_if_stats == NULL))
		return;

	/*
	 * Aggregate errors from other errors
	 * These other errors are only updated when it makes sense
	 * that the error should be charged to a logical interface.
	 */
	wlcif->_cnt->txerror = wlcif->_cnt->txnobuf + wlcif->_cnt->txrunt;
	wlcif->_cnt->rxerror = wlcif->_cnt->rxnobuf + wlcif->_cnt->rxrunt + wlcif->_cnt->rxfragerr;

	memcpy(wl_if_stats, wlcif->_cnt, sizeof(wl_if_stats_t));
}

si_t *
wlc_get_sih(wlc_info_t *wlc)
{
	return wlc->pub->sih;
}

bool
wlc_cfg_associated(wlc_info_t *wlc)
{
	return wlc->cfg->associated;
}

void
wlc_set_hw_up(wlc_info_t *wlc, bool val)
{
	wlc->pub->hw_up = val;
}

void
wlc_set_delayed_down(wlc_info_t *wlc, bool val)
{
	wlc->pub->delayed_down = val;
}

bool
wlc_get_delayed_down(wlc_info_t *wlc)
{
	return wlc->pub->delayed_down;
}

int
wlc_dump_register_wrapper(wlc_info_t *wlc, const char *name,
	dump_fn_t dump_fn, void *dump_fn_arg)
{
	return wlc_dump_register(wlc->pub, name, dump_fn, dump_fn_arg);
}
