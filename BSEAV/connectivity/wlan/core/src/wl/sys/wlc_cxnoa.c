/*
 * BTCoex with NOA protection source file
 * Broadcom 802.11abg Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017, *
 * All Rights Reserved. *
 *  *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom; *
 * the contents of this file may not be disclosed to third parties, copied *
 * or duplicated in any form, in whole or in part, without the prior *
 * written permission of Broadcom. *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 * $Id$
 */
#ifdef BCMCOEXNOA
#ifndef WLMCNX
#error "WLMCNX needs to be defined for WLP2P"
#endif

#ifndef WL_BSSCFG_TX_SUPR
#error "WL_BSSCFG_TX_SUPR needs to be defined for WLP2P"
#endif

#include <typedefs.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_cfg.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_mcnx.h>
#include <wlc_p2p.h>
#include <wlc_btcx.h>
#include <wlc_hw_priv.h>
#include <wlc_cxnoa.h>

/* cxnoa info accessor */
#define CXNOA_BSSCFG_CUBBY_LOC(cxnoa, cfg) ((bss_cxnoa_info_t **)BSSCFG_CUBBY((cfg), (cxnoa)->cfgh))
#define CXNOA_BSSCFG_CUBBY(cxnoa, cfg) (*(CXNOA_BSSCFG_CUBBY_LOC(cxnoa, cfg)))
#define BSS_CXNOA_INFO(cxnoa, cfg) CXNOA_BSSCFG_CUBBY(cxnoa, cfg)


#define NUMOFDESC(presreq) ((presreq) ? 2 :  1)

/* As of now, on GO there is only one peridic schedule max at any given time;
 * on Client there is one periodic schedule plus one one-tiem requested schedule max.
 */
#define WLC_CXNOA_MAX_SCHED	2	/* two concurrent Absence schedules */

struct wlc_cxnoa_info {
	wlc_info_t	*wlc;
	wlc_bsscfg_t	*bsscfg;
	int		cfgh;		/* handle to private data in bsscfg (Client/GO) */
};

typedef struct {
	uint8 cnt;			/* the number of descriptors in 'desc' */
	uint8 flags;			/* see WLC_P2P_SCHED_XXX */
	wl_p2p_sched_desc_t *desc;	/* len = cnt * sizeof(wl_p2p_sched_desc_t) */
} wlc_cxnoa_sched_t;

#define DESC_NUM 2
typedef struct bss_cxnoa_info {
	uint16			flags;	/* see 'flags' below */
	wl_p2p_sched_desc_t	cur;	/* active schedule in h/w */
	/* back pointer to bsscfg */
	wlc_bsscfg_t		*bsscfg;
	uint8			desc_cnt;
	wl_p2p_sched_desc_t	desc_com[DESC_NUM];
	bool			is_presreq_avail;
	bool			is_go_rfa_avail;
	uint32			prev_rfactive_go;
	uint32			prev_rfactive_gc;
	uint32			counter_go;
	uint32			counter_gc;
	uint32			interval_sco_esco;
	bool				go_desc_status;
	bool				gc_desc_status;
} bss_cxnoa_info_t;

/* This value is to avoid confusion over ACL interleave eSCO traffic */
#define P2P_NOA_CHECK_SCO_ESCO	5
#define BTC_ECI_BT_TASK_SCO	2		/* BT SCO task type  */
#define BTC_ECI_BT_TASK_ESCO	3	/* BT eSCO task type  */

/* Passing function parameters in NOA coex struct */
typedef struct wlc_coex_noa {
	wl_p2p_sched_t		*local_sched;
	wlc_bsscfg_t		*bsscfg;
	bss_cxnoa_info_t	*bci;
	wlc_cxnoa_info_t	*wci;
	uint32			rfactive;
} wlc_coex_noa_t;

static int wlc_cxnoa_multi_sched_abs_upd(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
static int wlc_coex_noa_set_num_of_desc(wlc_cxnoa_info_t *wci,
	wlc_bsscfg_t *cfg);
static int wlc_coex_noa_set(wlc_coex_noa_t *wcn, uint32 desc_cnt);
static void wlc_cxnoa_intr_cb(void *ctx, wlc_mcnx_intr_data_t *notif_data);
static void wlc_cxnoa_multi_desc_upd_cb(void *ctx,
	wlc_p2p_noa_upd_data_t *notif_data);
static void wlc_cxnoa_desc_upd_cb(void *ctx,
	wlc_p2p_noa_desc_upd_data_t *notif_data);
static int wlc_cxnoa_info_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_cxnoa_info_deinit(void *ctx, wlc_bsscfg_t *cfg);


#define P2P_FLAG_SCAN_ALL	0x02	/* scan both P2P and legacy devices in SCAN phase */
#define WLC_CXNOA_INFO_DLY	0x40	/* delay absence start init in SHM BSS block */

/* module attach/detach */
wlc_cxnoa_info_t *
BCMATTACHFN(wlc_cxnoa_attach)(wlc_info_t *wlc)
{
	wlc_cxnoa_info_t	*cxnoa;
	wlc_mcnx_info_t		*mcnx = wlc->mcnx;
	bss_cxnoa_info_t	*bci;

	/* module states */
	if ((cxnoa = (wlc_cxnoa_info_t *)MALLOCZ(wlc->osh, sizeof(*cxnoa))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	cxnoa->wlc = wlc;
	/* register callbacks with p2p uc module */
	ASSERT(mcnx != NULL);

	if (wlc_mcnx_intr_register(mcnx, wlc_cxnoa_intr_cb, cxnoa) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_p2p_intr_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* reserve cubby in the bsscfg container for per-bsscfg private data (for Client/GO) */
	if ((cxnoa->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(*bci),
	                wlc_cxnoa_info_init, wlc_cxnoa_info_deinit, NULL,
	                cxnoa)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (wlc_p2p_noa_upd_register(wlc->p2p, wlc_cxnoa_multi_desc_upd_cb, cxnoa) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_p2p_noa_upd_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (wlc_p2p_noa_desc_upd_register(wlc->p2p, wlc_cxnoa_desc_upd_cb, cxnoa) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_p2p_noa_desc_upd_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	return cxnoa;

fail:
	/* error handling */
	MODULE_DETACH(cxnoa, wlc_cxnoa_detach);
	return NULL;
}

void
BCMATTACHFN(wlc_cxnoa_detach)(wlc_cxnoa_info_t *cxnoa)
{
	wlc_info_t	*wlc;
	wlc_mcnx_info_t	*mcnx;

	if (cxnoa == NULL)
		return;

	wlc = cxnoa->wlc;
	ASSERT(wlc != NULL);

	mcnx = wlc->mcnx;
	ASSERT(mcnx != NULL);

	wlc_module_unregister(wlc->pub, "cxnoa", cxnoa);
	wlc_mcnx_intr_unregister(mcnx, wlc_cxnoa_intr_cb, cxnoa);
	wlc_p2p_noa_upd_unregister(wlc->p2p, wlc_cxnoa_multi_desc_upd_cb, cxnoa);
	MFREE(wlc->osh, cxnoa, sizeof(*cxnoa));
}

/* bsscfg cubby */
static int
wlc_cxnoa_info_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_cxnoa_info_t	*cxnoa = (wlc_cxnoa_info_t *)ctx;
	wlc_info_t		*wlc = cxnoa->wlc;
	bss_cxnoa_info_t	**pp2p = CXNOA_BSSCFG_CUBBY_LOC(cxnoa, cfg);
	bss_cxnoa_info_t	*bci;
	int err = BCME_OK;

	if (P2P_IF(wlc, cfg)) {
		if ((bci = MALLOCZ(wlc->osh, sizeof(*bci))) == NULL) {
			WL_ERROR(("wl%d: %s: wlc_p2p_info_alloc() failed\n",
			          wlc->pub->unit, __FUNCTION__));
			err = BCME_NOMEM;
			goto fail;
		}
		bci->bsscfg = cfg;
		*pp2p = bci;
	}

	return BCME_OK;

fail:
	wlc_cxnoa_info_deinit(cxnoa, cfg);
	return err;
}

static void
wlc_cxnoa_info_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_cxnoa_info_t	*cxnoa = (wlc_cxnoa_info_t *)ctx;
	wlc_info_t		*wlc = cxnoa->wlc;
	bss_cxnoa_info_t	**pp2p = CXNOA_BSSCFG_CUBBY_LOC(cxnoa, cfg);
	bss_cxnoa_info_t	*bci = *pp2p;

	if (bci == NULL)
		return;

	MFREE(wlc->osh, bci, sizeof(*bci));
	*pp2p = NULL;
}

#define P2P_NOA_DUR_ESCO	1250
#define P2P_NOA_CNT		65535
#define P2P_NOA_JITTER 300
#define WLC_CXNOA_INFO_CUR	0x01	/* 'cur' is valid */
#define WLC_CXNOA_INFO_NET	0x08	/* network parms valid (Client assoc'd or GO is up) */


/* This function decides whether to run two separate or single common
 * NOA schedules which are in sync with GO and GC's eSCO's RFActive.
 * eSCO's RFActive. If Two schedules overlap, the Duration starts
 * with starttime of first sched and ends with endtime of second sched.
 * If Two schedules dont overlap two schedules run separately.
 *
 * Non-Overlap schedules run like below:
 *
 * schedule1     |     +-----+                  +-----+   eSCO from GO
 * absence       | ... | ABS |                  | ABS |
 * --------------+-----+-----+------------------+-----+--------------
 *
 * schedule2     |                  +-----+                 +-----+  eSCO from GC
 * absence       |              ..  | ABS | ...             | ABS |
 * --------------+----------------- +-----+-----------------+-----+----
 *               |            <=prs=>     <=prs=>
 */
static int
wlc_cxnoa_multi_sched_abs_upd(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	bss_cxnoa_info_t	*bci;
	uint32			desc_one_start = 0;
	uint32			desc_two_start = 0;
	int32			start_diff = 0;

	bci = BSS_CXNOA_INFO(wlc->cxnoa, cfg);
	WL_TRACE(("Number descriptors: %d\n", bci->desc_cnt));

	if (bci->desc_cnt > 1) {
		desc_one_start = bci->desc_com[0].start;
		desc_two_start = bci->desc_com[1].start;

		if (desc_one_start != 0 && desc_two_start != 0) {
			start_diff = desc_one_start - desc_two_start;
			start_diff = ABS((int32)start_diff);
		} else {
			start_diff = 0;
		}

		if (start_diff != 0) {
			memset(&bci->cur, 0, sizeof(bci->cur));
			bci->flags &= ~WLC_CXNOA_INFO_CUR;
			/* Create one sched for overlapping schedules.
			  * Set 'start' to indicate in pre-tbtt intr that merged_sched
			  * should be set in shm's. Tbd later. 300us of jitter to
			  * compensate for shm writes and to compensate for
			  * divide-by-128us sched that's running on shm's
			  */
			if (start_diff < P2P_NOA_DUR_ESCO + P2P_NOA_JITTER) {
				if (desc_one_start <= desc_two_start) {
					bci->cur.start =  desc_one_start;
					bci->cur.duration = (desc_two_start - desc_one_start)
						+ P2P_NOA_DUR_ESCO;
				} else {
					bci->cur.start =  desc_two_start;
					bci->cur.duration = (desc_one_start - desc_two_start)
						+ P2P_NOA_DUR_ESCO;
				}
				desc_one_start += bci->interval_sco_esco;
				desc_two_start += bci->interval_sco_esco;
				bci->cur.interval = bci->interval_sco_esco;
				bci->cur.count = P2P_NOA_CNT;
				bci->flags |= WLC_CXNOA_INFO_CUR;
			} else {
				if (desc_one_start < desc_two_start) {
					bci->cur.start = desc_two_start;
					desc_one_start += bci->interval_sco_esco;
				} else {
					bci->cur.start = desc_one_start;
					desc_two_start += bci->interval_sco_esco;
				}
				bci->cur.duration = P2P_NOA_DUR_ESCO;
				bci->cur.interval = bci->interval_sco_esco;
				bci->cur.count = P2P_NOA_CNT;
				bci->flags |= WLC_CXNOA_INFO_CUR;
			}
		}
	}

	return TRUE;
}

static int
wlc_coex_noa_set(wlc_coex_noa_t *wcn, uint32 desc_cnt)
{
	wlc_info_t	*wlc = wcn->wci->wlc;
	uint32		err = 0;
	uint8		i = 0;

	if ((wcn->local_sched = MALLOCZ(wlc->osh,
		(sizeof(*wcn->local_sched) +
		(wcn->bci->desc_cnt - 1) * sizeof(wcn->local_sched->desc)))) == NULL) {
			WL_ERROR(("%s: out of mem\n",
				__FUNCTION__));
			return BCME_NOMEM;
	}

	wcn->local_sched->type =
		WL_P2P_SCHED_TYPE_ABS;
	wcn->local_sched->action =
		WL_P2P_SCHED_ACTION_NONE;
	wcn->local_sched->option =
		WL_P2P_SCHED_OPTION_NORMAL;

	for (i = 0; i < wcn->bci->desc_cnt; i++) {
		wcn->local_sched->desc[i].count =
			P2P_NOA_CNT;
		wcn->local_sched->desc[i].duration =
			P2P_NOA_DUR_ESCO;
		wcn->local_sched->desc[i].interval =
			wcn->bci->interval_sco_esco;
	}
	wcn->local_sched->desc[0].start = wcn->rfactive;
	wcn->local_sched->desc[1].start = wcn->bci->desc_com[1].start;
	/* Keep a copy of GO's eSCO schedule */
	memcpy(&wcn->bci->desc_com[0],
		&wcn->local_sched->desc[0],
		sizeof(wcn->local_sched->desc));
	err = wlc_p2p_noa_set(wlc->p2p, wcn->bsscfg,
		wcn->local_sched,
		((WL_P2P_SCHED_FIXED_LEN + 1) + (desc_cnt * sizeof(wcn->local_sched->desc))));

	if (wcn->local_sched != NULL) {
		MFREE(wlc->osh, wcn->local_sched,
		sizeof(*wcn->local_sched) +
		(desc_cnt - 1) * sizeof(wcn->local_sched->desc));
	}

	return err;
}


static int
wlc_coex_noa_set_num_of_desc(wlc_cxnoa_info_t *wci, wlc_bsscfg_t *cfg)
{
	bss_cxnoa_info_t	*bci;
	wlc_coex_noa_t		*wcn;
	wlc_info_t		*wlc = wci->wlc;
	uint16 rfactive_tsfh = 0, rfactive_tsfl = 0, bt_type = 0;
	uint32 rfactive = 0, err = 0;
	wl_p2p_sched_desc_t	noa;
	wlc_mcnx_info_t		*mcnx = wlc->mcnx;
	wl_p2p_sched_t		local_sched;

	bci = BSS_CXNOA_INFO(wci, cfg);

	ASSERT(bci != NULL);
	if ((wcn = MALLOCZ(wlc->osh, sizeof(*wcn))) == NULL) {
			WL_ERROR(("%s: out of mem\n",
				__FUNCTION__));
			return BCME_NOMEM;
	}
	/* GO: Pick 32bit BT RFActive timestamp from ucode and initiate a
	 * new NoA schedule
	 * GC: Send presence-request action frame
	 */
	if (P2P_GO(wlc, cfg)) {
		rfactive_tsfl = wlc_read_shm(wlc, M_BTCX_REQ_START(wlc));
		rfactive_tsfh = wlc_read_shm(wlc, M_BTCX_REQ_START_H(wlc));
		rfactive = ((rfactive_tsfh << 16) |
			rfactive_tsfl);
		bt_type = wlc_read_shm(wlc, M_BTCX_BT_TYPE(wlc));

		if ((bt_type == BTC_ECI_BT_TASK_ESCO) ||
			(bt_type == BTC_ECI_BT_TASK_SCO)) {
			bci->interval_sco_esco = wlc_read_shm(wlc, M_BTCX_PRED_PER(wlc));
			bci->is_go_rfa_avail = TRUE;
			bci->go_desc_status = TRUE;
			bci->desc_cnt = NUMOFDESC(bci->is_presreq_avail);
			wcn->bci = bci;
			wcn->wci = wci;
			wcn->bsscfg = cfg;
			wcn->rfactive = rfactive;
			err = wlc_coex_noa_set(wcn, bci->desc_cnt);
		}
	} else {
		if (P2P_CLIENT(wlc, cfg)) { /* GC sends Presence-request action frame */
			rfactive_tsfl = wlc_read_shm(wlc, M_BTCX_REQ_START(wlc));
			rfactive_tsfh = wlc_read_shm(wlc, M_BTCX_REQ_START_H(wlc));
			bt_type = wlc_read_shm(wlc, M_BTCX_BT_TYPE(wlc));
			if ((bt_type == BTC_ECI_BT_TASK_ESCO) ||
				(bt_type == BTC_ECI_BT_TASK_SCO)) {
				if (!cfg->up) {
					err = BCME_NOT_GC;
				} else {
					bci->interval_sco_esco = wlc_read_shm(wlc,
						M_BTCX_PRED_PER(wlc));
					rfactive = ((rfactive_tsfh << 16)|rfactive_tsfl);
					WL_P2P(("RFactive of GC: 0x%x\n", rfactive));
					noa.count = P2P_NOA_CNT;
					noa.duration = P2P_NOA_DUR_ESCO;
					noa.interval = bci->interval_sco_esco;
					noa.start = wlc_mcnx_l2r_tsf32(mcnx, cfg, rfactive);
					err = wlc_p2p_send_presence_req(wlc->p2p, cfg, &noa,
							&cfg->BSSID, NULL, NULL);
				}
			}
		}
	}

	if (bci->desc_com[0].start == 0 && bci->desc_com[1].start == 0) {
		WL_TRACE(("Both eSCO (GO or GC) is off..."
			"Cancel NoA...!\n"));
		if (bci->go_desc_status || bci->gc_desc_status) {
			bci->go_desc_status = FALSE;
			bci->gc_desc_status = FALSE;
			local_sched.type = WL_P2P_SCHED_TYPE_ABS;
			local_sched.action = WL_P2P_SCHED_ACTION_RESET;
			/* delete the NoA schedule */
			err = wlc_p2p_noa_set(wlc->p2p,
				cfg, &local_sched,
				WL_P2P_SCHED_FIXED_LEN);
		}
	}

	/* 'counter' logic to wait for 5 pre-tbtt's before declaring
	 * eSCO is off.
	 */
	if (bci->prev_rfactive_go != rfactive) {
		bci->counter_go = 0;
		bci->prev_rfactive_go = rfactive;
	} else {
		if ((bci->prev_rfactive_go != 0) &&
			(bci->counter_go == P2P_NOA_CHECK_SCO_ESCO)) {
			bci->is_go_rfa_avail = FALSE;
			bci->desc_com[0].start = 0;
		} else {
			bci->counter_go++;
		}
	}
	if (bci->prev_rfactive_gc != bci->desc_com[1].start) {
		bci->counter_gc = 0;
		bci->prev_rfactive_gc = bci->desc_com[1].start;
	} else {
		if ((bci->prev_rfactive_gc != 0) &&
			(bci->counter_gc == P2P_NOA_CHECK_SCO_ESCO)) {
			bci->is_presreq_avail = FALSE;
			bci->desc_com[1].start = 0;
		} else {
			bci->counter_gc++;
		}
	}

	if (wcn != NULL) {
		MFREE(wlc->osh, wcn, sizeof(*wcn));
	}

	return err;
}
/*  */
static void
wlc_cxnoa_multi_desc_upd_cb(void *ctx, wlc_p2p_noa_upd_data_t *notif_data)
{
	wlc_cxnoa_info_t	*cxnoa = (wlc_cxnoa_info_t *)ctx;
	wlc_info_t		*wlc = cxnoa->wlc;
	bss_cxnoa_info_t	*bci;
	wlc_bsscfg_t		*cfg;

	cfg = notif_data->cfg;
	ASSERT(cfg != NULL);
	bci = BSS_CXNOA_INFO(cxnoa, cfg);
	bci->desc_cnt = notif_data->cnt;
	memcpy(&bci->desc_com, notif_data->desc, (notif_data->cnt * sizeof(notif_data->desc)));
	wlc_cxnoa_multi_sched_abs_upd(wlc, cfg);
	memcpy(&notif_data->cur, &bci->cur, sizeof(bci->cur));
	notif_data->flags |= bci->flags;
}

static void
wlc_cxnoa_desc_upd_cb(void *ctx, wlc_p2p_noa_desc_upd_data_t *notif_data)
{
	wlc_cxnoa_info_t	*cxnoa = (wlc_cxnoa_info_t *)ctx;
	bss_cxnoa_info_t	*bci;
	wlc_bsscfg_t		*cfg;

	cfg = notif_data->cfg;
	ASSERT(cfg != NULL);
	bci = BSS_CXNOA_INFO(cxnoa, cfg);
	notif_data->is_go_rfact_avail = bci->is_go_rfa_avail;
	bci->is_presreq_avail = TRUE; /* This callback is from process_presence_req only */
	bci->gc_desc_status = TRUE;
	memcpy(&bci->desc_com[1], &notif_data->desc, sizeof(notif_data->desc));
}

static void
wlc_cxnoa_intr_cb(void *ctx, wlc_mcnx_intr_data_t *notif_data)
{
	wlc_cxnoa_info_t	*cxnoa = (wlc_cxnoa_info_t *)ctx;
	wlc_info_t		*wlc = cxnoa->wlc;
	wlc_bsscfg_t		*cfg;
	uint			intr;
	bool			p2p_status;

	(void)wlc;

	cfg = notif_data->cfg;
	ASSERT(cfg != NULL);

	p2p_status = wlc_p2p_cxnoa_bss_status(wlc->p2p, cfg);

	if (p2p_status == FALSE) {
		return;
	}

	intr = notif_data->intr;

	/* process each cxnoa interrupt */
	switch (intr) {
	case M_P2P_I_PRE_TBTT:
	{
		int err = 0;

		if (P2P_IF(wlc, cfg)) {
			err = wlc_coex_noa_set_num_of_desc(cxnoa, cfg);
		}
		if (err != BCME_OK) {
			WL_ERROR(("Error in setting NoA "
					"schedule for Btcx: 0x%x\n", err));
		}
	}
		break;
	case M_P2P_I_PRS:
	{
		bss_cxnoa_info_t	*bci;

		bci = BSS_CXNOA_INFO(cxnoa, cfg);
		if (bci->desc_cnt > 1) {
			wlc_cxnoa_multi_sched_abs_upd(wlc, cfg);
			wlc_p2p_cxnoa_upd(wlc, cfg);
		}
	}
		break;
	}
}
#endif /* BCMCOEXNOA */
