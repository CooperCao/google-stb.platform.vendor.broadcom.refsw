/*
 * Common (OS-independent) portion of
 * Broadcom 802.11abg Networking Device Driver
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
 */

/* Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_bsscfg.h>
#include <wlc_bsscfg_psq.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wlc_assoc.h>
#include <wlc_tx.h>
#ifdef PROP_TXSTATUS
#include <wlc_ampdu.h>
#include <wlc_apps.h>
#include <wlc_wlfc.h>
#include <wlc_scb.h>
#endif /* PROP_TXSTATUS */
#ifdef WLMCHAN
#include <wlc_mchan.h>
#include <wlc_p2p.h>
#endif /* WLMCHAN */
#include <wlc_lq.h>
#ifdef WLTDLS
#include <wlc_tdls.h>
#endif
#include <wlc_msch.h>
#include <wlc_chanctxt.h>
#include <wlc_ap.h>
#include <wlc_dfs.h>
#include <wlc_pm.h>
#include <wlc_srvsdb.h>
#include <wlc_sta.h>
#include <phy_chanmgr_api.h>
#ifdef TXQ_MUX
#include <wlc_bcmc_txq.h>
#endif

#define SCHED_MAX_TIME_GAP	200
#define SCHED_TIMER_DELAY	100

/* bss chan context */
struct wlc_chanctxt {
	struct wlc_chanctxt *next;
	wlc_txq_info_t *qi;			/* tx queue */
	chanspec_t	chanspec;		/* channel specific */
	uint16		count;			/* count for txq attached */
	uint16	fragthresh[NFIFO];	/* context based fragthreshold */
	uint8	usage_countdown;	/* keeping track of cleaup if stale */
	bool	in_passive_use;		/* retain context while creating new one */
	bool	piggyback;		/* share the channel */
};

struct wlc_chanctxt_info {
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_chanctxt_t *chanctxt_list;	/* chan context link list */
	wlc_chanctxt_t *curr_chanctxt;	/* current chan context */
	int cfgh;			/* bsscfg cubby handle */
	wlc_chanctxt_t *piggyback_chanctxt;	/* piggyback chan context */
	chanspec_t excursion_chanspec;	/* excursion queue request channel */
	uint16 excursion_count;		/* count for excursion queue attached */
};

/* per BSS data */
typedef struct {
	wlc_chanctxt_t *chanctxt;	/* current chan context for this bss */
	bool		on_channel;	/* whether the bss is on channel */
	bool		multi_channel;	/* support multi channels */
	uint16		num_channel;	/* how many channel sequence */
	wlc_chanctxt_t **mchanctxt;	/* chan context list for multi chan seq */
} chanctxt_bss_info_t;

/* locate per BSS data */
#define CHANCTXT_BSS_CUBBY_LOC(chanctxt_info, cfg) \
	((chanctxt_bss_info_t **)BSSCFG_CUBBY((cfg), (chanctxt_info)->cfgh))
#define CHANCTXT_BSS_INFO(chanctxt_info, cfg) \
	(*CHANCTXT_BSS_CUBBY_LOC(chanctxt_info, cfg))

/* local prototypes */
static void wlc_chanctxt_bsscfg_state_upd(void *ctx, bsscfg_state_upd_data_t *evt);
static int wlc_chanctxt_bss_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_chanctxt_bss_deinit(void *ctx, wlc_bsscfg_t *cfg);
static bool wlc_chanctxt_in_use(wlc_info_t *wlc, wlc_chanctxt_t *chanctxt,
	wlc_bsscfg_t *filter_cfg);
static bool wlc_tx_qi_exist(wlc_info_t *wlc, wlc_txq_info_t *qi);
static wlc_txq_info_t *wlc_get_qi_with_no_context(wlc_info_t *wlc);
static void wlc_chanctxt_set_channel(wlc_chanctxt_info_t *chanctxt_info,
	wlc_chanctxt_t *chanctxt, chanspec_t chanspec, bool destroy_old);
static wlc_chanctxt_t *wlc_chanctxt_alloc(wlc_chanctxt_info_t *chanctxt_info,
	chanspec_t chanspec);
static void wlc_chanctxt_free(wlc_chanctxt_info_t *chanctxt_info, wlc_chanctxt_t *chanctxt);
static wlc_chanctxt_t *wlc_get_chanctxt(wlc_chanctxt_info_t *chanctxt_info,
	chanspec_t chanspec);
static bool wlc_all_cfg_chanspec_is_same(wlc_info_t *wlc,
	wlc_chanctxt_t *chanctxt, chanspec_t chanspec);
static void wlc_chanxtxt_delete_bsscfg_ctxt(wlc_chanctxt_info_t *chanctxt_info,
	wlc_bsscfg_t *bsscfg);
static wlc_chanctxt_t *wlc_chanxtxt_find_bsscfg_ctxt(wlc_chanctxt_info_t *chanctxt_info,
	wlc_bsscfg_t *bsscfg, chanspec_t chanspec);
static void wlc_chanctxt_switch_queue(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wlc_chanctxt_t *chanctxt,
	uint oldqi_stopped_flag);
static wlc_chanctxt_t *wlc_chanctxt_create_txqueue(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	chanspec_t chanspec);
static int wlc_chanctxt_delete_txqueue(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	wlc_chanctxt_t *chanctxt, uint *oldqi_info);

static int wlc_chanctxt_down(void *context);
static void wlc_chanctxt_watchdog(void *context);
static void wlc_chanxtxt_delete_unused_ctxt(wlc_chanctxt_info_t *chanctxt_info,
	wlc_chanctxt_t *chanctxt);

#ifdef SRHWVSDB
static void wlc_chanctxt_srvsdb_upd(wlc_chanctxt_info_t *chanctxt_info);
#endif

/* debugging... */
#ifdef EVENT_LOG_COMPILE
#define WL_CHANCTXT_TRACE(args)	do { \
	if (EVENT_LOG_IS_LOG_ON(EVENT_LOG_TAG_CHANCTXT_TRACE)) \
		EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_CHANCTXT_TRACE, args); \
	} while (0)
#define WL_CHANCTXT_INFO(args)	do { \
	if (EVENT_LOG_IS_LOG_ON(EVENT_LOG_TAG_CHANCTXT_INFO)) \
		EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_CHANCTXT_INFO, args); \
	} while (0)
#define WL_CHANCTXT_WARN(args)	\
	EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_CHANCTXT_WARN, args)
#define WL_CHANCTXT_ERROR(args)	\
	EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_CHANCTXT_ERROR, args)
#define WL_CHANCTXT_ASSERT(exp, args) do { \
		if (!(exp)) { \
			WL_CHANCTXT_ERROR(args); \
			ASSERT(FALSE); \
		} \
	} while (0)
#else /* EVENT_LOG_COMPILE */
#define WL_CHANCTXT_TRACE(args)
#define WL_CHANCTXT_INFO(args)
#define WL_CHANCTXT_WARN(args)
#define WL_CHANCTXT_ERROR(args)
#define WL_CHANCTXT_ASSERT(exp, args)	ASSERT(exp)
#endif /* EVENT_LOG_COMPILE */

wlc_chanctxt_info_t *
BCMATTACHFN(wlc_chanctxt_attach)(wlc_info_t *wlc)
{
	wlc_chanctxt_info_t *chanctxt_info;

	/* module states */
	chanctxt_info = (wlc_chanctxt_info_t *)MALLOCZ(wlc->osh, sizeof(wlc_chanctxt_info_t));
	if (!chanctxt_info) {
		WL_ERROR(("wl%d: wlc_chanctxt_attach: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, MALLOCED(wlc->osh)));
		return NULL;
	}

	chanctxt_info->wlc = wlc;

#ifdef EVENT_LOG_COMPILE
	event_log_tag_start(EVENT_LOG_TAG_CHANCTXT_ERROR, EVENT_LOG_SET_WL,
		EVENT_LOG_TAG_FLAG_LOG | EVENT_LOG_TAG_FLAG_PRINT);
	event_log_tag_start(EVENT_LOG_TAG_CHANCTXT_WARN, EVENT_LOG_SET_WL,
		EVENT_LOG_TAG_FLAG_LOG);
#endif /* EVENT_LOG_COMPILE */

	/* register module */
	if (wlc_module_register(wlc->pub, NULL, "chanctxt", chanctxt_info, NULL,
		wlc_chanctxt_watchdog, NULL, wlc_chanctxt_down)) {
		WL_ERROR(("wl%d: wlc_chanctxt_attach: wlc_module_register() failed\n",
		          wlc->pub->unit));
		goto fail;
	}

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((chanctxt_info->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(chanctxt_bss_info_t *),
		wlc_chanctxt_bss_init, wlc_chanctxt_bss_deinit, NULL, chanctxt_info)) < 0) {
		WL_ERROR(("wl%d: wlc_chanctxt_attach: wlc_bsscfg_cubby_reserve() failed\n",
			wlc->pub->unit));
		goto fail;
	}

	if (wlc_bsscfg_state_upd_register(wlc, wlc_chanctxt_bsscfg_state_upd, chanctxt_info)
		!= BCME_OK) {
		WL_ERROR(("wl%d: wlc_chanctxt_attach: wlc_bsscfg_state_upd_register() failed\n",
		          wlc->pub->unit));
		goto fail;
	}

	return chanctxt_info;

fail:
	/* error handling */
	MODULE_DETACH(chanctxt_info, wlc_chanctxt_detach);

	return NULL;
}

void
BCMATTACHFN(wlc_chanctxt_detach)(wlc_chanctxt_info_t *chanctxt_info)
{
	wlc_info_t *wlc;
	wlc_chanctxt_t *chanctxt_entity;

	ASSERT(chanctxt_info);
	wlc = chanctxt_info->wlc;

	/* delete all chan contexts */
	while ((chanctxt_entity = chanctxt_info->chanctxt_list)) {
		wlc_chanctxt_free(chanctxt_info, chanctxt_entity);
	}

	(void)wlc_bsscfg_state_upd_unregister(wlc, wlc_chanctxt_bsscfg_state_upd, chanctxt_info);

	/* unregister module */
	wlc_module_unregister(wlc->pub, "chanctxt", chanctxt_info);

	MFREE(wlc->osh, chanctxt_info, sizeof(wlc_chanctxt_info_t));
}

static int
wlc_chanctxt_bss_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_chanctxt_info_t *chanctxt_info = (wlc_chanctxt_info_t *)ctx;
	wlc_info_t *wlc = chanctxt_info->wlc;
	chanctxt_bss_info_t **cbi_loc;
	chanctxt_bss_info_t *cbi;

	/* allocate cubby info */
	if ((cbi = MALLOCZ(wlc->osh, sizeof(chanctxt_bss_info_t))) == NULL) {
		WL_ERROR(("wl%d: wlc_chanctxt_bss_init: MALLOC failed, malloced %d bytes\n",
			wlc->pub->unit, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}

	cbi_loc = CHANCTXT_BSS_CUBBY_LOC(chanctxt_info, cfg);
	*cbi_loc = cbi;

	return BCME_OK;
}

static void
wlc_chanctxt_bss_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_chanctxt_info_t *chanctxt_info = (wlc_chanctxt_info_t *)ctx;
	chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(chanctxt_info, cfg);

	if (cbi != NULL) {
		wlc_chanxtxt_delete_bsscfg_ctxt(chanctxt_info, cfg);
		if (cbi->multi_channel) {
			MFREE(chanctxt_info->wlc->osh, cbi->mchanctxt, cbi->num_channel *
				sizeof(wlc_chanctxt_t *));
		}

		MFREE(chanctxt_info->wlc->osh, cbi, sizeof(chanctxt_bss_info_t));
	}
}

static void
wlc_chanctxt_bsscfg_state_upd(void *ctx, bsscfg_state_upd_data_t *evt)
{
	wlc_bsscfg_t *cfg = evt->cfg;
	ASSERT(cfg != NULL);

	if (evt->old_enable && !cfg->enable) {
		wlc_chanctxt_info_t *chanctxt_info = (wlc_chanctxt_info_t *)ctx;
		chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(chanctxt_info, cfg);

		ASSERT(cbi);

		wlc_chanxtxt_delete_bsscfg_ctxt(chanctxt_info, cfg);
		cbi->chanctxt = NULL;
	}
}

static int
wlc_chanctxt_down(void * context)
{
	wlc_chanctxt_info_t *chanctxt_info = (wlc_chanctxt_info_t *)context;
	wlc_info_t *wlc = chanctxt_info->wlc;
	wlc_chanctxt_t *ctxt_iter = chanctxt_info->chanctxt_list;

	while (ctxt_iter) {
		/* immediately free up, if no one is using this chanctxt */
		if (!wlc_chanctxt_in_use(wlc, ctxt_iter, NULL))  {
			wlc_chanctxt_t *chanctxt_iter_save = ctxt_iter->next;
			wlc_chanxtxt_delete_unused_ctxt(chanctxt_info, ctxt_iter);
			ctxt_iter = chanctxt_iter_save;
			continue;
		}
		ctxt_iter = ctxt_iter->next;
	}
	return 0;
}

static void
wlc_chanctxt_watchdog(void * context)
{
	wlc_chanctxt_info_t *chanctxt_info = (wlc_chanctxt_info_t *)context;
	wlc_info_t *wlc = chanctxt_info->wlc;
	wlc_chanctxt_t *ctxt_iter = chanctxt_info->chanctxt_list;

	while (ctxt_iter) {
		/* first check if no cfg is using this context */
		if (!wlc_chanctxt_in_use(wlc, ctxt_iter, NULL))  {
			/*
			* next check if countdown expired or not
			* if not decrement the value and defer cleanup till 0
			*/
			if (--ctxt_iter->usage_countdown > 0) {
				goto next;
			}
			else {
				/* usage timed out. Clean this stale context */
				wlc_chanctxt_t *chanctxt_iter_save = ctxt_iter->next;
				wlc_chanxtxt_delete_unused_ctxt(chanctxt_info, ctxt_iter);
				ctxt_iter = chanctxt_iter_save;
				continue;
			}
		}
next:
		ctxt_iter = ctxt_iter->next;
	}
}

static void
wlc_chanxtxt_delete_unused_ctxt(wlc_chanctxt_info_t *chanctxt_info,
	wlc_chanctxt_t *chanctxt)
{
	wlc_lq_chanim_delete_bss_chan_context(chanctxt_info->wlc, chanctxt->chanspec);
	wlc_chanctxt_free(chanctxt_info, chanctxt);
}

static void
wlc_chanxtxt_delete_bsscfg_ctxt(wlc_chanctxt_info_t *chanctxt_info,
	wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc = chanctxt_info->wlc;
	chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(chanctxt_info, bsscfg);

	if (cbi != NULL) {
		if (cbi->multi_channel) {
			int i;
			for (i = 0; i < cbi->num_channel; i++) {
				if (cbi->mchanctxt[i]) {
					wlc_chanctxt_delete_txqueue(wlc, bsscfg,
						cbi->mchanctxt[i], NULL);
				}
			}
		} else if (cbi->chanctxt) {
			wlc_chanctxt_delete_txqueue(wlc, bsscfg, cbi->chanctxt, NULL);
		}
	}
}

static wlc_chanctxt_t *
wlc_chanxtxt_find_bsscfg_ctxt(wlc_chanctxt_info_t *chanctxt_info, wlc_bsscfg_t *bsscfg,
	chanspec_t chanspec)
{
	wlc_info_t *wlc = chanctxt_info->wlc;
	chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(chanctxt_info, bsscfg);
	wlc_chanctxt_t *chanctxt = NULL;

	if (cbi != NULL) {
		int i, idx = -1;
		if (cbi->multi_channel) {
			for (i = 0; i < cbi->num_channel; i++) {
				chanctxt = cbi->mchanctxt[i];
				if (chanctxt) {
					if (WLC_CHAN_COEXIST(chanctxt->chanspec,
						chanspec)) {
						wlc_chanctxt_switch_queue(wlc, bsscfg,
							chanctxt, 0);
						idx = i;
						break;
					}
					chanctxt = NULL;
				} else if (idx < 0) {
					idx = i;
				}
			}
			WL_CHANCTXT_INFO(("wl%d.%d: wlc_chanxtxt_find_bsscfg_ctxt: "
				"chanctxt[%d] = 0x%08x\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), idx,
				(uint32)chanctxt));
			ASSERT(idx < cbi->num_channel);
			cbi->chanctxt = chanctxt;
		} else {
			chanctxt = cbi->chanctxt;
		}

		if (chanctxt == NULL || ((chanctxt->chanspec != chanspec) &&
			(!WLC_CHAN_COEXIST(chanctxt->chanspec, chanspec) ||
			CHSPEC_BW_GT(chanspec, CHSPEC_BW(chanctxt->chanspec))))) {
			chanctxt = wlc_chanctxt_create_txqueue(wlc, bsscfg, chanspec);
			if (cbi->multi_channel) {
				cbi->mchanctxt[idx] = chanctxt;
			}
		}
	}

	return chanctxt;
}

void
wlc_chanctxt_set_chan_num(wlc_info_t *wlc, wlc_bsscfg_t *cfg, int num_chan)
{
	wlc_chanctxt_info_t *chanctxt_info = wlc->chanctxt_info;
	chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(chanctxt_info, cfg);

	WL_CHANCTXT_TRACE(("[ENTRY]wl%d.%d: wlc_chanctxt_set_chan_num: "
		"num %d\n", wlc->pub->unit, WLC_BSSCFG_IDX(cfg), num_chan));

	ASSERT(cbi);

	wlc_chanxtxt_delete_bsscfg_ctxt(chanctxt_info, cfg);
	cbi->chanctxt = NULL;

	if (cbi->multi_channel) {
		MFREE(wlc->osh, cbi->mchanctxt, cbi->num_channel *
			sizeof(wlc_chanctxt_t *));
		cbi->mchanctxt = NULL;
	}

	if (num_chan > 1) {
		cbi->num_channel = (uint16)num_chan;
		cbi->mchanctxt = MALLOCZ(wlc->osh, num_chan * sizeof(wlc_chanctxt_t *));
	}

	cbi->multi_channel = (cbi->mchanctxt != NULL);
}

void
wlc_txqueue_start(wlc_info_t *wlc, wlc_bsscfg_t *cfg, chanspec_t chanspec,
	wlc_chanctxt_restore_fn_t restore_fn)
{
	wlc_chanctxt_info_t *chanctxt_info = wlc->chanctxt_info;
	chanctxt_bss_info_t *cbi;
	wlc_chanctxt_t *chanctxt;
	wlc_txq_info_t *qi;
	DBGONLY(char chanbuf[CHANSPEC_STR_LEN]; )

	WLCNTINCR(wlc->pub->_cnt->txqueue_start);
	WL_CHANCTXT_TRACE(("[ENTRY]wl%d.%d: wlc_txqueue_start: "
		"channel 0x%04x, call by %08x\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
		chanspec, CALL_SITE));

	WL_CHANCTXT_ASSERT(WLCNTVAL(wlc->pub->_cnt->txqueue_end) == 0,
		("[ASSERT]wl%d.%d: wlc_txqueue_start: wlc_txqueue_end in process\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));

	WL_MQ(("wl%d.%d: %s: channel %s\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
		wf_chspec_ntoa(chanspec, chanbuf)));

	if (cfg != NULL) {
		cbi = CHANCTXT_BSS_INFO(chanctxt_info, cfg);
		ASSERT(cbi);

		chanctxt = wlc_chanxtxt_find_bsscfg_ctxt(chanctxt_info, cfg, chanspec);
		ASSERT(chanctxt);

		if (cbi->on_channel) {
			WL_CHANCTXT_WARN(("[WARNING]wl%d.%d: wlc_txqueue_start: "
				"cfg is already on channel, count <%d, %d>\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(cfg), chanctxt->count,
				chanctxt_info->excursion_count));
			goto done;
		}
		cbi->on_channel = TRUE;

		/* Restore the bsscfg saving packets */
		if (restore_fn) {
			void *pkt;
			int prec;
			qi = chanctxt->qi;
			while ((pkt = restore_fn(cfg, &prec))) {
				WL_MQ(("Enq: pkt %p, prec %d\n", OSL_OBFUSCATE_BUF(pkt), prec));
				pktq_penq(WLC_GET_TXQ(qi), prec, pkt);
			}
		}
	} else {
		if (chanctxt_info->excursion_count == 0) {
			chanctxt_info->excursion_chanspec = chanspec;
		} else {
			WL_CHANCTXT_ASSERT(chanctxt_info->excursion_chanspec != 0 &&
				WLC_CHAN_COEXIST(chanctxt_info->excursion_chanspec,
				chanspec), ("[ASSERT]wl%d: wlc_txqueue_start: excursion queue "
				"chanspec 0x%04x is not equal to current chanspec 0x%04x\n",
				wlc->pub->unit, chanctxt_info->excursion_chanspec,
				chanspec));
		}

		chanctxt_info->excursion_count++;

		chanctxt = wlc_get_chanctxt(chanctxt_info, chanspec);
		if (chanctxt == NULL || chanctxt->count == 0) {
			if (chanctxt_info->excursion_count == 1) {
				wlc_excursion_start(wlc);
			} else {
				WL_CHANCTXT_ASSERT(wlc->excursion_active,
					("[ASSERT]wl%d: wlc_txqueue_start: excursion queue "
					"is not active yet, count[%d]\n", wlc->pub->unit,
					chanctxt_info->excursion_count));
			}
			WL_CHANCTXT_INFO(("wl%d: wlc_txqueue_start: using excursion queue: "
				"active %d, count %d\n", wlc->pub->unit,
				wlc->excursion_active, chanctxt_info->excursion_count));
			goto done;
		}

		chanctxt->piggyback = TRUE;
		chanctxt_info->piggyback_chanctxt = chanctxt;
	}

	qi = chanctxt->qi;
	WL_MQ(("wl%d.%d: %s: qi %p, primary %p, active %p\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
		OSL_OBFUSCATE_BUF(qi), OSL_OBFUSCATE_BUF(wlc->primary_queue),
		OSL_OBFUSCATE_BUF(wlc->active_queue)));

	/* Attaches tx qi */
	chanctxt->count++;
	WL_CHANCTXT_INFO(("wl%d.%d: wlc_txqueue_start: using chanctxt queue: count <%d, %d>\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg), chanctxt->count,
		chanctxt_info->excursion_count));

	if (chanctxt->count == 1) {
		uint8 idx;

		if (chanctxt_info->excursion_count > 0) {
			WL_CHANCTXT_ASSERT(chanctxt_info->excursion_chanspec != 0 &&
				WLC_CHAN_COEXIST(chanctxt_info->excursion_chanspec,
				chanspec), ("[ASSERT]wl%d.%d: wlc_txqueue_start: excursion queue "
				"chanspec 0x%04x is not equal to current chanspec 0x%04x\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
				chanctxt_info->excursion_chanspec, chanspec));

			chanctxt->count += chanctxt_info->excursion_count;
			chanctxt->piggyback = TRUE;
			chanctxt_info->piggyback_chanctxt = chanctxt;

			wlc_excursion_end(wlc);
		}

		WL_CHANCTXT_ASSERT(!wlc->excursion_active,
			("[ASSERT]wl%d.%d: wlc_txqueue_start: excursion queue is still active\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));

		wlc_suspend_mac_and_wait(wlc);
		wlc_primary_queue_set(wlc, qi);
		wlc_enable_mac(wlc);

		/* restore the chancontext->fragthreshold to wlc while going on channel */
		for (idx = 0; idx < NFIFO; idx++) {
			wlc_fragthresh_set(wlc, idx, chanctxt->fragthresh[idx]);
		}

		WL_MQ(("wl%d.%d: %s: attach qi %p, primary %p, active %p\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
			OSL_OBFUSCATE_BUF(qi), OSL_OBFUSCATE_BUF(wlc->primary_queue),
			OSL_OBFUSCATE_BUF(wlc->active_queue)));
	}
	else {
		/* now excursion cannot be active, only piggyback should happen */
		ASSERT(wlc->excursion_active == FALSE);
	}

	WL_CHANCTXT_ASSERT(wlc->active_queue == qi,
		("[ASSERT]wl%d.%d: wlc_txqueue_start: chanctxt queue is not active queue: "
		"count <%d, %d>\n", wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
		chanctxt->count, chanctxt_info->excursion_count));

	wlc_txflowcontrol_override(wlc, qi, OFF, TXQ_STOP_FOR_MSCH_FLOW_CNTRL);

	/* run txq if not empty */
	if (WLC_TXQ_OCCUPIED(wlc)) {
		wlc_send_q(wlc, qi);
	}

	if (chanctxt_info->curr_chanctxt != NULL &&
		chanctxt_info->curr_chanctxt->chanspec != chanctxt->chanspec) {
		/* Tell CHANIM that we're about to switch channels */
		if (wlc_lq_chanim_adopt_bss_chan_context(wlc, chanctxt->chanspec,
			chanctxt_info->curr_chanctxt->chanspec) != BCME_OK) {
			WL_INFORM(("wl%d.%d %s: chanim adopt blocked scan/assoc/rm: \n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		}
	}

	chanctxt_info->curr_chanctxt = chanctxt;
	chanctxt->usage_countdown = (uint8)wlc->pub->tunables->max_wait_for_ctxt_delete;

done:
	WLCNTDECR(wlc->pub->_cnt->txqueue_start);
	WL_CHANCTXT_TRACE(("[EXIT]wl%d.%d: wlc_txqueue_start\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
}

/**
 * Context structure used by wlc_txqueue_end_filter() while filtering a pktq
 */
struct wlc_txqueue_end_filter_info {
	int cfgidx;         /**< index of bsscfg who's packets are being deleted */
	wlc_bsscfg_t *cfg;  /**< bsscfg who's packets are being deleted */
	wlc_chanctxt_backup_fn_t backup_fn;  /**< function that takes possesion of filtered pkts */
	int prec;           /**< prec currently being filtered */
};

/**
 * pktq filter function to filter out pkts associated with a BSSCFG index,
 * passing them to a "backup" function.
 * Used by wlc_txqueue_end().
 */
static pktq_filter_result_t
wlc_txqueue_end_filter(void* ctx, void* pkt)
{
	struct wlc_txqueue_end_filter_info *info = (struct wlc_txqueue_end_filter_info *)ctx;
	int idx;
	pktq_filter_result_t ret;

	/* if the bsscfg index matches, filter out the packet and pass to the backup_fn */
	idx = WLPKTTAGBSSCFGGET(pkt);
	if (idx == info->cfgidx) {
		WL_MQ(("Deq: pkt %p, prec %d\n", OSL_OBFUSCATE_BUF(pkt), info->prec));
		(info->backup_fn)(info->cfg, pkt, info->prec);
		ret = PKT_FILTER_REMOVE;
	} else {
		ret = PKT_FILTER_NOACTION;
	}

	return ret;
}

void
wlc_txqueue_end(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wlc_chanctxt_backup_fn_t backup_fn)
{
	wlc_chanctxt_info_t *chanctxt_info = wlc->chanctxt_info;
	chanctxt_bss_info_t *cbi;
	wlc_chanctxt_t *chanctxt;
	wlc_txq_info_t *qi;
	bool start_excursion = FALSE;

	WLCNTINCR(wlc->pub->_cnt->txqueue_end);
	WL_CHANCTXT_TRACE(("[ENTRY]wl%d.%d: wlc_txqueue_end: call by %08x\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg), CALL_SITE));

	WL_CHANCTXT_ASSERT(WLCNTVAL(wlc->pub->_cnt->txqueue_start) == 0,
		("[ASSERT]wl%d.%d: wlc_txqueue_end: wlc_txqueue_start in process\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));

	if (cfg != NULL) {
		cbi = CHANCTXT_BSS_INFO(chanctxt_info, cfg);
		ASSERT(cbi);

		chanctxt = cbi->chanctxt;
		if (!cbi->on_channel || !chanctxt) {
			WL_CHANCTXT_WARN(("[WARNING]wl%d.%d: wlc_txqueue_end: "
				"cfg is already off channel, count <%d, %d>\n", wlc->pub->unit,
				WLC_BSSCFG_IDX(cfg), chanctxt? chanctxt->count : -100,
				chanctxt_info->excursion_count));
			goto done;
		}
		cbi->on_channel = FALSE;

#ifdef PROP_TXSTATUS
		if (PROP_TXSTATUS_ENAB(wlc->pub)) {
			wlc_wlfc_mchan_interface_state_update(wlc, cfg,
				WLFC_CTL_TYPE_INTERFACE_CLOSE, FALSE);
		}
#endif /* PROP_TXSTATUS */
	} else {
		if (chanctxt_info->excursion_count == 0) {
			WL_CHANCTXT_ASSERT(chanctxt_info->excursion_chanspec == 0 &&
				!chanctxt_info->piggyback_chanctxt,
				("[ASSERT]wl%d: wlc_txqueue_end: excursion chanspec is not "
				"cleared yet\n", wlc->pub->unit));
			WL_CHANCTXT_WARN(("[WARNING]wl%d: wlc_txqueue_end: "
				"excursion is already off channel, active %d, count %d\n",
				wlc->pub->unit, wlc->excursion_active,
				chanctxt_info->excursion_count));
			goto done;
		}

		WL_CHANCTXT_ASSERT(chanctxt_info->excursion_count > 0,
			("[ASSERT]wl%d: wlc_txqueue_end: excursion count [%d]\n",
			wlc->pub->unit, chanctxt_info->excursion_count));
		chanctxt_info->excursion_count--;

		chanctxt = chanctxt_info->piggyback_chanctxt;
		if (chanctxt == NULL) {
			if (chanctxt_info->excursion_count == 0) {
				chanctxt_info->excursion_chanspec = 0;
				wlc_excursion_end(wlc);
			} else {
				WL_CHANCTXT_ASSERT(wlc->excursion_active,
					("[ASSERT]wl%d: wlc_txqueue_end: excursion queue "
					"has been inactive, count[%d]\n", wlc->pub->unit,
					chanctxt_info->excursion_count));
			}
			WL_CHANCTXT_INFO(("wl%d: wlc_txqueue_end: ending excursion queue: "
				"active %d, count %d\n", wlc->pub->unit,
				wlc->excursion_active, chanctxt_info->excursion_count));
			goto done;
		} else {
			/*
			* if piggyback is ON, it must be using
			* current chanctxt's chanspec
			*/
			WL_CHANCTXT_ASSERT(WLC_CHAN_COEXIST(chanctxt_info->excursion_chanspec,
				chanctxt_info->curr_chanctxt->chanspec),
				("[ASSERT]wl%d: wlc_txqueue_end: excursion queue "
				"chanspec 0x%04x is not equal to current chanspec 0x%04x\n",
				wlc->pub->unit, chanctxt_info->excursion_chanspec,
				chanctxt_info->curr_chanctxt->chanspec));
		}

		if (chanctxt_info->excursion_count == 0) {
			chanctxt->piggyback = FALSE;
			chanctxt_info->excursion_chanspec = 0;
			chanctxt_info->piggyback_chanctxt = NULL;
		}
	}

	ASSERT(chanctxt->count > 0);

	chanctxt->count--;
	if (chanctxt != chanctxt_info->curr_chanctxt) {
		WL_CHANCTXT_INFO(("wl%d.%d: wlc_txqueue_end: chanctxt is not current chanctxt\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
		goto done;
	} else {
		if ((chanctxt_info->excursion_count > 0) &&
			(chanctxt_info->excursion_count == chanctxt->count)) {
			/*
			* chanctxt accomodating piggyback has ended
			* end piggyback now (and start excursion?)
			*/
			start_excursion = TRUE;
			chanctxt_info->piggyback_chanctxt = NULL;
			chanctxt->piggyback = FALSE;
			chanctxt->count = 0;
		}
	}

	qi = chanctxt->qi;
	WL_MQ(("wl%d.%d: %s: qi %p, primary %p, active %p\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
		OSL_OBFUSCATE_BUF(qi), OSL_OBFUSCATE_BUF(wlc->primary_queue),
		OSL_OBFUSCATE_BUF(wlc->active_queue)));

	/* Detaches tx qi */
	WL_CHANCTXT_INFO(("wl%d.%d: wlc_txqueue_end: ending chanctxt queue: count <%d, %d>\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg), chanctxt->count,
		chanctxt_info->excursion_count));
	if (chanctxt->count == 0) {
		uint idx;

		wlc_txflowcontrol_override(wlc, qi, ON, TXQ_STOP_FOR_MSCH_FLOW_CNTRL);

		wlc_suspend_mac_and_wait(wlc);
		wlc_primary_queue_set(wlc, NULL);
		wlc_enable_mac(wlc);

		/* save the chancontext->fragthreshold from wlc while going off channel */
		for (idx = 0; idx < NFIFO; idx++) {
			chanctxt->fragthresh[idx] = wlc->fragthresh[idx];
		}

		WL_MQ(("wl%d.%d: %s: detach qi %p, primary %p, active %p\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
			OSL_OBFUSCATE_BUF(qi), OSL_OBFUSCATE_BUF(wlc->primary_queue),
			OSL_OBFUSCATE_BUF(wlc->active_queue)));
	} else {
		wlc_txflowcontrol_override(wlc, qi, OFF, TXQ_STOP_FOR_MSCH_FLOW_CNTRL);
	}

	/* Backup the bsscfg packets and saving them */
	if (backup_fn && cfg != NULL && !pktq_empty(WLC_GET_TXQ(qi))) {
		struct wlc_txqueue_end_filter_info info;
		struct pktq *pq;
		int prec;

		WL_MQ(("wl%d.%d: %s: Backup the bsscfg packets and saving them\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));

		info.cfgidx = WLC_BSSCFG_IDX(cfg);
		info.cfg = cfg;
		info.backup_fn = backup_fn;

		/* Dequeue the packets from tx qi that belong to the bsscfg */
		pq = WLC_GET_TXQ(qi);
		PKTQ_PREC_ITER(pq, prec) {
			info.prec = prec;
			wlc_txq_pktq_filter(wlc, pq, wlc_txqueue_end_filter, &info);
		}
	}

done:
	if (start_excursion) {
		wlc_excursion_start(wlc);
	}
	WLCNTDECR(wlc->pub->_cnt->txqueue_end);
	WL_CHANCTXT_TRACE(("[EXIT]wl%d.%d: wlc_txqueue_end\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
}

bool
wlc_txqueue_active(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(wlc->chanctxt_info, cfg);

	ASSERT(cbi);

	return (cbi->chanctxt && cbi->on_channel);
}

bool
wlc_has_chanctxt(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(wlc->chanctxt_info, cfg);

	ASSERT(cbi);

	return cbi->chanctxt != NULL;
}

chanspec_t
wlc_get_chanspec(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(wlc->chanctxt_info, cfg);
	wlc_chanctxt_t *chanctxt;

	ASSERT(cbi);
	chanctxt = cbi->chanctxt;

	return (chanctxt ? chanctxt->chanspec : INVCHANSPEC);
}

bool
_wlc_shared_chanctxt(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wlc_chanctxt_t *shared_chanctxt)
{
	chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(wlc->chanctxt_info, cfg);
	wlc_chanctxt_t *chanctxt;

	ASSERT(cbi);
	chanctxt = cbi->chanctxt;

	if (chanctxt) {
		if (chanctxt == shared_chanctxt)
			return TRUE;

		if (cbi->multi_channel) {
			int i;
			for (i = 0; i < cbi->num_channel; i++) {
				if (cbi->mchanctxt[i] == shared_chanctxt)
					return TRUE;
			}
		}
	}
	return FALSE;
}

bool
wlc_shared_chanctxt_on_chan(wlc_info_t *wlc, wlc_bsscfg_t *cfg, chanspec_t chan)
{
	chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(wlc->chanctxt_info, cfg);
	wlc_chanctxt_t *chanctxt;

	ASSERT(cbi);
	chanctxt = cbi->chanctxt;

	if (chanctxt) {
		if (chanctxt->chanspec == chan)
			return TRUE;

		if (cbi->multi_channel) {
			int i;
			for (i = 0; i < cbi->num_channel; i++) {
				if (cbi->mchanctxt[i]->chanspec == chan)
					return TRUE;
			}
		}
	}
	return FALSE;
}

bool
wlc_shared_chanctxt(wlc_info_t *wlc, wlc_bsscfg_t *cfg1, wlc_bsscfg_t *cfg2)
{
	chanctxt_bss_info_t *cbi1 = CHANCTXT_BSS_INFO(wlc->chanctxt_info, cfg1);
	chanctxt_bss_info_t *cbi2 = CHANCTXT_BSS_INFO(wlc->chanctxt_info, cfg2);

	ASSERT(cbi1 && cbi2);

	return cbi1->chanctxt != NULL && cbi2->chanctxt != NULL &&
		cbi1->chanctxt == cbi2->chanctxt;
}

bool
wlc_shared_current_chanctxt(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlc_chanctxt_info_t *chanctxt_info = wlc->chanctxt_info;
	chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(chanctxt_info, cfg);
	wlc_chanctxt_t *chanctxt;

	ASSERT(cbi);
	chanctxt = cbi->chanctxt;

	return ((chanctxt != NULL) && (chanctxt == chanctxt_info->curr_chanctxt));
}

bool
_wlc_ovlp_chanspec(wlc_info_t *wlc, wlc_bsscfg_t *cfg, chanspec_t chspec, uint chbw)
{
	chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(wlc->chanctxt_info, cfg);
	wlc_chanctxt_t *chanctxt;

	ASSERT(cbi);
	chanctxt = cbi->chanctxt;

	return chanctxt == NULL ||
	        CHSPEC_CTLOVLP(chanctxt->chanspec, chspec, (int)chbw);
}

bool
wlc_ovlp_chanspec(wlc_info_t *wlc, wlc_bsscfg_t *cfg1, wlc_bsscfg_t *cfg2, uint chbw)
{
	chanctxt_bss_info_t *cbi1 = CHANCTXT_BSS_INFO(wlc->chanctxt_info, cfg1);
	chanctxt_bss_info_t *cbi2 = CHANCTXT_BSS_INFO(wlc->chanctxt_info, cfg2);
	wlc_chanctxt_t *chantxt1, *chantxt2;

	ASSERT(cbi1 && cbi2);
	chantxt1 = cbi1->chanctxt;
	chantxt2 = cbi2->chanctxt;

	return chantxt1 == NULL || chantxt2 == NULL || chantxt1 == chantxt2 ||
	        CHSPEC_CTLOVLP(chantxt1->chanspec, chantxt2->chanspec, (int)chbw);
}

/*
* set TRUE if required to preserve old chanctxt while reqeusting new one
* set to FALSE by default
*/
void
wlc_chanctxt_set_passive_use(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool value)
{
	chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(wlc->chanctxt_info, cfg);
	wlc_chanctxt_t *chanctxt;

	ASSERT(cbi);
	chanctxt = cbi->chanctxt;
	chanctxt->in_passive_use = value;
}

static bool
wlc_chanctxt_in_use(wlc_info_t *wlc, wlc_chanctxt_t *chanctxt, wlc_bsscfg_t *filter_cfg)
{
	int idx;
	wlc_bsscfg_t *cfg;

	ASSERT(chanctxt != NULL);
	if (chanctxt->piggyback) {
		return (TRUE);
	}

	FOREACH_BSS(wlc, idx, cfg) {
		if ((cfg != filter_cfg) &&
			_wlc_shared_chanctxt(wlc, cfg, chanctxt)) {
			return (TRUE);
		}
	}
	return (FALSE);
}

static bool
wlc_tx_qi_exist(wlc_info_t *wlc, wlc_txq_info_t *qi)
{
	wlc_txq_info_t *qi_list;

	/* walk through all queues */
	for (qi_list = wlc->tx_queues; qi_list; qi_list = qi_list->next) {
		if (qi == qi_list) {
			/* found it in the list, return true */
			return TRUE;
		}
	}

	return FALSE;
}

static wlc_txq_info_t *
wlc_get_qi_with_no_context(wlc_info_t *wlc)
{
	wlc_chanctxt_info_t *chanctxt_info = wlc->chanctxt_info;
	wlc_chanctxt_t *chanctxt_entity;
	wlc_txq_info_t *qi_list;
	bool found;

	/* walk through all queues */
	for (qi_list = wlc->tx_queues; qi_list; qi_list = qi_list->next) {
		found = FALSE;
		/* walk through all context */
		chanctxt_entity = chanctxt_info->chanctxt_list;
		for (; chanctxt_entity; chanctxt_entity = chanctxt_entity->next) {
			if (chanctxt_entity->qi == qi_list) {
				found = TRUE;
				break;
			}
		}
		/* make sure it is not excursion_queue */
		if (qi_list == wlc->excursion_queue) {
			found = TRUE;
		}
		/* if found is false, current qi is without context */
		if (!found) {
			WL_MQ(("found qi %p with no associated ctx\n", OSL_OBFUSCATE_BUF(qi_list)));
			return (qi_list);
		}
	}

	return (NULL);
}

/** Take care of all the stuff needed when a ctx channel is set */
static void
wlc_chanctxt_set_channel(wlc_chanctxt_info_t *chanctxt_info,
	wlc_chanctxt_t *chanctxt, chanspec_t chanspec, bool destroy_old)
{
	if (destroy_old) {
#ifdef PHYCAL_CACHING
		/* delete phy context for calibration */
		phy_chanmgr_destroy_ctx((phy_info_t *) WLC_PI(chanctxt_info->wlc),
			chanctxt->chanspec);
#endif /* PHYCAL_CACHING */
	}
	chanctxt->chanspec = chanspec;

#ifdef PHYCAL_CACHING
	/* create phy context for calibration */
	phy_chanmgr_create_ctx((phy_info_t *) WLC_PI(chanctxt_info->wlc), chanspec);
#endif /* PHYCAL_CACHING */
}

#ifdef SRHWVSDB
static void
wlc_chanctxt_srvsdb_upd(wlc_chanctxt_info_t *chanctxt_info)
{
	wlc_chanctxt_t *chanctxt = chanctxt_info->chanctxt_list;
	wlc_info_t *wlc = chanctxt_info->wlc;

	if (SRHWVSDB_ENAB(wlc->pub)) {
		/* reset the engine so that previously saved stuff will be gone */
		wlc_srvsdb_reset_engine(wlc);

		/* Attach SRVSDB module */
		if (wlc_multi_chanctxt(wlc)) {
			wlc_srvsdb_set_chanspec(wlc, chanctxt->chanspec, chanctxt->next->chanspec);
			if (wlc_bmac_activate_srvsdb(wlc->hw, chanctxt->chanspec,
				chanctxt->next->chanspec) != BCME_OK) {
				wlc_bmac_deactivate_srvsdb(wlc->hw);
			}
		} else {
			wlc_bmac_deactivate_srvsdb(wlc->hw);
		}
	}
}
#endif /* SRHWVSDB */

static wlc_chanctxt_t *
wlc_chanctxt_alloc(wlc_chanctxt_info_t *chanctxt_info, chanspec_t chanspec)
{
	wlc_info_t *wlc = chanctxt_info->wlc;
	osl_t *osh = wlc->osh;
	wlc_chanctxt_t *chanctxt;
	wlc_txq_info_t *qi;
	uint idx;
	DBGONLY(char chanbuf[CHANSPEC_STR_LEN]; )

	WL_MQ(("wl%d: %s: channel %s\n", wlc->pub->unit, __FUNCTION__,
		wf_chspec_ntoa(chanspec, chanbuf)));

	chanctxt = (wlc_chanctxt_t *)MALLOCZ(osh, sizeof(wlc_chanctxt_t));
	if (chanctxt == NULL) {
		WL_ERROR(("wl%d: wlc_chanctxt_alloc: failed to allocate "
			"mem for chan context\n", wlc->pub->unit));
		return (NULL);
	}

	qi = wlc_get_qi_with_no_context(wlc);
	/* if qi with no context found, then use it */
	if (qi) {
		chanctxt->qi = qi;
		WL_MQ(("use existing qi = %p\n", OSL_OBFUSCATE_BUF(qi)));
	}
	/* else allocate a new queue */
	else {
		chanctxt->qi = wlc_txq_alloc(wlc, osh);
		WL_MQ(("allocate new qi = %p\n", OSL_OBFUSCATE_BUF(chanctxt->qi)));
	}

	ASSERT(chanctxt->qi != NULL);
	wlc_chanctxt_set_channel(chanctxt_info, chanctxt, chanspec, FALSE);

	/* initialize chanctxt specific fragthreshold to default value */
	for (idx = 0; idx < NFIFO; idx++) {
		chanctxt->fragthresh[idx] = wlc->usr_fragthresh;
	}

	/* add chanctxt to head of context list */
	chanctxt->next = chanctxt_info->chanctxt_list;
	chanctxt_info->chanctxt_list = chanctxt;

#ifdef SRHWVSDB
	wlc_chanctxt_srvsdb_upd(chanctxt_info);
#endif /* SRHWVSDB */

	return (chanctxt);
}

static void
wlc_chanctxt_free(wlc_chanctxt_info_t *chanctxt_info, wlc_chanctxt_t *chanctxt)
{
	wlc_info_t *wlc = chanctxt_info->wlc;
	osl_t *osh = wlc->osh;
	wlc_txq_info_t *oldqi;
#ifdef PHYCAL_CACHING
	chanspec_t chanspec = chanctxt->chanspec; /* Need to know which chanspec to delete */
#endif /* PHYCAL_CACHING */
	wlc_chanctxt_t *prev, *next;
	int idx,  prec;
	wlc_bsscfg_t *cfg;
	void * pkt;
	DBGONLY(char chanbuf[CHANSPEC_STR_LEN]; )

	WL_MQ(("wl%d: %s: channel %s\n", wlc->pub->unit, __FUNCTION__,
		wf_chspec_ntoa(chanctxt->chanspec, chanbuf)));

	ASSERT(chanctxt != NULL);

	/* save the qi pointer, delete from context list and free context */
	oldqi = chanctxt->qi;

	prev = (wlc_chanctxt_t *)&chanctxt_info->chanctxt_list;
	while ((next = prev->next)) {
		if (next == chanctxt) {
			prev->next = chanctxt->next;
			break;
		}
		prev = next;
	}
	MFREE(osh, chanctxt, sizeof(wlc_chanctxt_t));

#ifdef PHYCAL_CACHING
	/* delete phy context for calibration */
	phy_chanmgr_destroy_ctx((phy_info_t *) WLC_PI(wlc), chanspec);
#endif /* PHYCAL_CACHING */

#ifdef SRHWVSDB
	wlc_chanctxt_srvsdb_upd(chanctxt_info);
#endif /* SRHWVSDB */

	chanctxt = chanctxt_info->chanctxt_list;

	/* continue with deleting this queue only if there are more contexts */
	if (chanctxt == NULL) {
		/* flush the queue */
		WL_MQ(("wl%d: %s: our queue is only context queue, don't delete, "
			"simply flush the queue, qi 0x%p len %d\n", wlc->pub->unit,
			__FUNCTION__, OSL_OBFUSCATE_BUF(oldqi), (WLC_GET_TXQ(oldqi))->n_pkts_tot));
		wlc_txq_pktq_flush(wlc, WLC_GET_TXQ(oldqi));
		ASSERT(pktq_empty(WLC_GET_TXQ(oldqi)));
		return;
	}

	/* if this queue is the primary_queue, detach it first */
	if (oldqi == wlc->primary_queue) {
		ASSERT(oldqi != chanctxt->qi);
		WL_MQ(("wl%d: %s: detach primary queue %p\n",
			wlc->pub->unit, __FUNCTION__,
			OSL_OBFUSCATE_BUF(wlc->primary_queue)));
		wlc_suspend_mac_and_wait(wlc);
		wlc_primary_queue_set(wlc, NULL);
		wlc_enable_mac(wlc);
	}

	/* Before freeing this queue, any bsscfg that is using this queue
	 * should now use the primary queue.
	 * Active queue can be the excursion queue if we're scanning so using
	 * the primary queue is more appropriate.  When scan is done, active
	 * queue will be switched back to primary queue.
	 * This can be true for bsscfg's that never had contexts.
	 */
	FOREACH_BSS(wlc, idx, cfg) {
		if (cfg->wlcif->qi == oldqi) {
			WL_MQ(("wl%d.%d: %s: cfg's qi %p is about to get deleted, "
				"move to queue %p\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
				__FUNCTION__, oldqi, OSL_OBFUSCATE_BUF(chanctxt->qi)));
			cfg->wlcif->qi = chanctxt->qi;
		}
	}

	/* flush the queue */
	/* wlc_txq_pktq_flush(wlc, &oldqi->q); */
	while ((pkt = pktq_deq(WLC_GET_TXQ(oldqi), &prec))) {
#ifdef PROP_TXSTATUS
		if (PROP_TXSTATUS_ENAB(wlc->pub)) {
			wlc_process_wlhdr_txstatus(wlc,
				WLFC_CTL_PKTFLAG_DISCARD, pkt, FALSE);
		}
#endif /* PROP_TXSTATUS */
		PKTFREE(osh, pkt, TRUE);
	}

	ASSERT(pktq_empty(WLC_GET_TXQ(oldqi)));

	/* delete the queue */
	wlc_txq_free(wlc, osh, oldqi);
	WL_MQ(("wl%d %s: flush and free q\n", wlc->pub->unit, __FUNCTION__));
}

/**
 * This function returns channel context pointer based on chanspec passed in
 * If no context associated to chanspec, will return NULL.
 */
static wlc_chanctxt_t *
wlc_get_chanctxt(wlc_chanctxt_info_t *chanctxt_info, chanspec_t chanspec)
{
	wlc_chanctxt_t *chanctxt = chanctxt_info->chanctxt_list;

	while (chanctxt) {
		if (WLC_CHAN_COEXIST(chanctxt->chanspec, chanspec)) {
			return chanctxt;
		}
		chanctxt = chanctxt->next;
	}

	return NULL;
}

/**
 * Given a chanctxt and chanspec as input, looks at all bsscfg using this
 * chanctxt and see if bsscfg->chanspec matches exactly with chanspec.
 * Returns TRUE if all bsscfg->chanspec matches chanspec.  Else returns FALSE.
 */
static bool
wlc_all_cfg_chanspec_is_same(wlc_info_t *wlc,
	wlc_chanctxt_t *chanctxt, chanspec_t chanspec)
{
	int idx;
	wlc_bsscfg_t *cfg;

	FOREACH_AS_BSS(wlc, idx, cfg) {
		chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(wlc->chanctxt_info, cfg);
		if (cbi->chanctxt == chanctxt &&
		    !WLC_CHAN_COEXIST(cfg->current_bss->chanspec, chanspec)) {
			return (FALSE);
		}
	}
	return (TRUE);
}

static void
wlc_chanctxt_switch_queue(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wlc_chanctxt_t *chanctxt,
	uint oldqi_stopped_flag)
{
	wlc_txq_info_t *oldqi = cfg->wlcif->qi;
	bool flowcontrol;

	cfg->wlcif->qi = chanctxt->qi;

	/* check old qi to see if we need to perform flow control on new qi.
	 * Only need to check TXQ_STOP_FOR_PKT_DRAIN because it is cfg specific.
	 * All other TXQ_STOP types are queue specific.
	 * Existing flow control setting only pertinent if we already have chanctxt.
	 */
	flowcontrol = wlc_txflowcontrol_override_isset(wlc, oldqi, TXQ_STOP_FOR_PKT_DRAIN);

#ifdef TXQ_MUX
	/* Not merging this to the main block below to allow some refactoring flexibility */
	if (oldqi != chanctxt->qi) {
		/* Move mux sources */
		WL_NONE(("wlc_chanctxt_create_txqueue(%d)\n", __LINE__));
		wlc_bsscfg_recreate_muxsrc(wlc, cfg, chanctxt->qi);
	}
#endif

	/* if we switched queue/context, need to take care of flow control */
	if (oldqi != chanctxt->qi) {
		/* evaluate to see if we need to enable flow control for PKT_DRAIN
		 * If flowcontrol is true, this means our old chanctxt has flow control
		 * of type TXQ_STOP_FOR_PKT_DRAIN enabled.  Transfer this setting to new qi.
		 */
		if (flowcontrol) {
			WL_MQ(("wl%d.%d: %s: turn ON flow control for "
				  "TXQ_STOP_FOR_PKT_DRAIN on new qi %p\n",
				  wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
				  OSL_OBFUSCATE_BUF(chanctxt->qi)));
			wlc_txflowcontrol_override(wlc, chanctxt->qi,
				ON, TXQ_STOP_FOR_PKT_DRAIN);
			/* Check our oldqi, if it is different and exists,
			 * then we need to disable flow control on it
			 * since we just moved queue.
			 */
			if (wlc_tx_qi_exist(wlc, oldqi)) {
				WL_MQ(("wl%d.%d: %s: turn OFF flow control for "
					  "TXQ_STOP_FOR_PKT_DRAIN on old qi %p\n",
					  wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
					  OSL_OBFUSCATE_BUF(oldqi)));
				wlc_txflowcontrol_override(wlc, oldqi, OFF,
					TXQ_STOP_FOR_PKT_DRAIN);
			}
		}
		else {
			/* This is to take care of cases where old context was deleted
			 * while flow control was on without TXQ_STOP_FOR_PKT_DRAIN being set.
			 * This means our cfg's interface is flow controlled at the wl layer.
			 * Need to disable flow control now that we have a new queue.
			 */
			if (oldqi_stopped_flag) {
				WL_MQ(("wl%d.%d: %s: turn OFF flow control for "
					  "TXQ_STOP_FOR_PKT_DRAIN on new qi %p\n",
					  wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
					  OSL_OBFUSCATE_BUF(oldqi)));
				wlc_txflowcontrol_override(wlc, chanctxt->qi, OFF,
					TXQ_STOP_FOR_PKT_DRAIN);
			}
		}
	}
}

/** before bsscfg register to MSCH, call this function */
static wlc_chanctxt_t *
wlc_chanctxt_create_txqueue(wlc_info_t *wlc, wlc_bsscfg_t *cfg, chanspec_t chanspec)
{
	wlc_chanctxt_info_t *chanctxt_info = wlc->chanctxt_info;
	chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(chanctxt_info, cfg);
	wlc_chanctxt_t *chanctxt, *new_chanctxt;
	uint oldqi_stopped_flag = 0;
#ifdef ENABLE_FCBS
	wlc_phy_t *pi = WLC_PI(wlc);
#endif
	DBGONLY(char chanbuf[CHANSPEC_STR_LEN]; )

	ASSERT(cbi);
	chanctxt = cbi->chanctxt;

	WL_CHANCTXT_TRACE(("[ENTRY]wl%d.%d: wlc_chanctxt_create_txqueue: "
		"chanspec 0x%04x, call by %08x\n", wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
		chanspec, CALL_SITE));

	WL_MQ(("wl%d.%d: %s: called on %s chanspec %s\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
		BSSCFG_AP(cfg) ? "AP" : "STA",
		wf_chspec_ntoa_ex(chanspec, chanbuf)));

	/* fetch context for this chanspec */
	new_chanctxt = wlc_get_chanctxt(chanctxt_info, chanspec);

	/* This cfg has an existing context, do some checks to determine what to do */
	if (chanctxt) {
		WL_MQ(("wl%d.%d: %s: cfg alredy has context!\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));

		if (chanctxt->chanspec != chanspec) {
			wlc_lq_chanim_create_bss_chan_context(wlc, chanspec, chanctxt->chanspec);
		}

		if (WLC_CHAN_COEXIST(chanctxt->chanspec, chanspec)) {
			/* we know control channel is same but if chanspecs are not
			 * identical, will want chan context to adopt to new chanspec,
			 * if no other cfg's using it or all cfg's using that context
			 * are using new chanspec.
			 */
			if ((chanctxt->chanspec != chanspec) &&
				(!wlc_chanctxt_in_use(wlc, chanctxt, cfg) ||
				wlc_all_cfg_chanspec_is_same(wlc, chanctxt,
					chanspec)) &&
				((chanspec == wlc_get_cur_wider_chanspec(wlc, cfg)))) {

				WL_MQ(("context is the same but need to adopt "
				          "from chanspec 0x%x to 0x%x\n",
				          chanctxt->chanspec, chanspec));
#ifdef ENABLE_FCBS
				wlc_phy_fcbs_uninit(pi, chanctxt->chanspec);
				wlc_phy_fcbs_arm(pi, chanspec, 0);
#endif
				wlc_chanctxt_set_channel(chanctxt_info, chanctxt, chanspec,
					!BSSCFG_IS_MODESW_BWSW(cfg));
			}
			else {
				WL_MQ(("context is the same as new one, do nothing\n"));
			}
			return chanctxt;
		}
		/* Requested nonoverlapping channel. */
		else {
			if (!wlc_chanctxt_in_use(wlc, chanctxt, cfg) &&
				!chanctxt->in_passive_use) {
#ifdef TXQ_MUX
				wlc_bsscfg_delete_muxsrc(wlc, cfg);
#endif

				/* delete if not required to preserve chancxt immediately */
				wlc_chanctxt_delete_txqueue(wlc, cfg, chanctxt,
					&oldqi_stopped_flag);
			}
			/*
			* else chanctxt will get deleted in watchdog
			* if not used for 'usage_countdown' seconds
			*/
		}
		cbi->chanctxt = NULL;
	}

	/* check to see if a context for this chanspec already exist */
	if (new_chanctxt == NULL) {
		wlc_lq_chanim_create_bss_chan_context(wlc, chanspec, 0);

		/* context for this chanspec doesn't exist, create a new one */
		WL_MQ(("wl%d.%d: %s: allocate new context\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		new_chanctxt = wlc_chanctxt_alloc(chanctxt_info, chanspec);
#ifdef ENABLE_FCBS
		if (!wlc_phy_fcbs_arm(pi, chanspec, 0))
			WL_MQ(("%s: wlc_phy_fcbs_arm FAILed\n", __FUNCTION__));
#endif
	}

	ASSERT(new_chanctxt != NULL);
	if (new_chanctxt == NULL)
		return NULL;

	/* assign context to cfg */
	wlc_chanctxt_switch_queue(wlc, cfg, new_chanctxt, oldqi_stopped_flag);
	cbi->chanctxt = new_chanctxt;

	if (CHSPEC_BW_GT(chanspec, CHSPEC_BW(new_chanctxt->chanspec))) {
		WL_MQ(("wl%d.%d: %s: Upgrade chanctxt chanspec "
			"from 0x%x to 0x%x\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
			new_chanctxt->chanspec, chanspec));
#ifdef ENABLE_FCBS
		wlc_phy_fcbs_uninit(pi, new_chanctxt->chanspec);
		wlc_phy_fcbs_arm(pi, chanspec, 0);
#endif
		wlc_chanctxt_set_channel(chanctxt_info, new_chanctxt, chanspec,
			!BSSCFG_IS_MODESW_BWSW(cfg));
	}

	WL_MQ(("wl%d.%d: %s: cfg chanctxt chanspec set to %s\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
		wf_chspec_ntoa_ex(new_chanctxt->chanspec, chanbuf)));

	return new_chanctxt;
}

/** Everytime bsscfg becomes disassociated, call this function */
static int
wlc_chanctxt_delete_txqueue(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wlc_chanctxt_t *chanctxt,
	uint *oldqi_info)
{
	wlc_chanctxt_info_t *chanctxt_info = wlc->chanctxt_info;
	chanctxt_bss_info_t *cbi = CHANCTXT_BSS_INFO(chanctxt_info, cfg);
	bool saved_state = cfg->associated;
	DBGONLY(char chanbuf[CHANSPEC_STR_LEN]; )

	ASSERT(cbi);

	WL_CHANCTXT_TRACE(("[ENTRY]wl%d.%d: wlc_chanctxt_delete_txqueue: "
		"on_channel %d, call by %08x\n", wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
		cbi->on_channel, CALL_SITE));

	/* if context was not created due to AP up being deferred and we called down,
	 * can have a chan_ctxt that is NULL.
	 */
	if (chanctxt == NULL) {
		return (BCME_OK);
	}

	WL_MQ(("wl%d.%d: %s: called on %s chanspec %s\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
		BSSCFG_AP(cfg) ? "AP" : "STA",
		wf_chspec_ntoa_ex(chanctxt->chanspec, chanbuf)));

	if (cbi->on_channel) {
		ASSERT(chanctxt->count > 0);
		chanctxt->count--;
		cbi->on_channel = FALSE;
	}

	wlc_lq_chanim_delete_bss_chan_context(wlc, chanctxt->chanspec);

	/* temporarily clear the cfg associated state.
	 * during roam, we are associated so when we do roam,
	 * we can potentially be deleting our old context first
	 * before creating our new context.  Just want to make sure
	 * that when we delete context, we're not associated.
	 */
	cfg->associated = FALSE;

#ifdef STA
	if (BSSCFG_STA(cfg)) {
#if defined(WLMCHAN) && defined(WLP2P)
		if (MCHAN_ENAB(wlc->pub) && P2P_CLIENT(wlc, cfg)) {
			if (wlc_p2p_noa_valid(wlc->p2p, cfg)) {
				mboolclr(cfg->pm->PMblocked, WLC_PM_BLOCK_MCHAN_ABS);
			}
		} else
#endif /* WLP2P */
		{
			mboolclr(cfg->pm->PMblocked, WLC_PM_BLOCK_CHANSW);
		}
		wlc_update_pmstate(cfg, TX_STATUS_NO_ACK);
	}
#endif /* STA */
#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		/* Change the state of interface to OPEN so that pkts will be drained */
		wlc_wlfc_mchan_interface_state_update(wlc, cfg,
			WLFC_CTL_TYPE_INTERFACE_OPEN, TRUE);
		wlc_wlfc_flush_pkts_to_host(wlc, cfg);
		wlfc_sendup_ctl_info_now(wlc->wlfc);
	}
#endif /* PROP_TXSTATUS */
	wlc_txflowcontrol_override(wlc, chanctxt->qi, OFF,
		TXQ_STOP_FOR_MSCH_FLOW_CNTRL);

	/* if txq currently in use by cfg->wlcif->qi is deleted in
	 * wlc_mchanctxt_free(), cfg->wlcif->qi will be set to the primary queue
	 */

	/* Take care of flow control for this cfg.
	 * If it was turned on for the queue, then this cfg has flow control on.
	 * Turn it off for this cfg only.
	 * Allocate a stale queue and temporarily attach cfg to this queue.
	 * Copy over the qi->stopped flag and turn off flow control.
	 */
	if (chanctxt->qi->stopped) {
		if (oldqi_info != NULL) {
			*oldqi_info = chanctxt->qi->stopped;
		}
		else
		{
			wlc_txq_info_t *qi = wlc_txq_alloc(wlc, wlc->osh);
			wlc_txq_info_t *orig_qi = cfg->wlcif->qi;

			WL_MQ(("wl%d.%d: %s: flow control on, turn it off!\n", wlc->pub->unit,
			          WLC_BSSCFG_IDX(cfg), __FUNCTION__));
			/* Use this new q to drain the wl layer packets to make sure flow control
			 * is turned off for this interface.
			 */
			qi->stopped = chanctxt->qi->stopped;
			cfg->wlcif->qi = qi;
			/* reset_qi() below will not allow tx flow control to turn back on */
			wlc_txflowcontrol_reset_qi(wlc, qi);
			/* if txq currently in use by cfg->wlcif->qi is deleted in
			 * wlc_infra_sched_free(), cfg->wlcif->qi will be set to the primary queue
			 */
			wlc_txq_pktq_flush(wlc, WLC_GET_TXQ(qi));
			ASSERT(pktq_empty(WLC_GET_TXQ(qi)));
			wlc_txq_free(wlc, wlc->osh, qi);
			cfg->wlcif->qi = orig_qi;
		}
	}

	/* no one else using this context, safe to delete it */
	if (!wlc_chanctxt_in_use(wlc, chanctxt, cfg)) {
#ifdef ENABLE_FCBS
		if (!wlc_phy_fcbs_uninit(WLC_PI(wlc), chanctxt->chanspec))
			WL_INFORM(("%s: wlc_phy_fcbs_uninit() FAILed\n", __FUNCTION__));
#endif
		if (chanctxt == chanctxt_info->curr_chanctxt) {
#ifdef STA
			/* If we were in the middle of transitioning to PM state because of this
			 * BSS, then cancel it
			 */
			if (wlc->PMpending) {
				wlc_bsscfg_t *icfg;
				int idx;
				FOREACH_AS_STA(wlc, idx, icfg) {
					if (icfg->pm->PMpending &&
						_wlc_shared_chanctxt(wlc, icfg, chanctxt)) {
						wlc_update_pmstate(icfg, TX_STATUS_BE);
					}
				}
			}
#endif /* STA */
			chanctxt_info->curr_chanctxt = NULL;
		}

		wlc_chanctxt_free(chanctxt_info, chanctxt);
		chanctxt = NULL;
	}

	/* restore cfg associated state */
	cfg->associated = saved_state;

	return BCME_OK;
}
