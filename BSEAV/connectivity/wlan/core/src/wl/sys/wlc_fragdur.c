/**
 * @file
 * @brief
 * Maximum TX duration enforcement via fragmentation.
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

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_scb_ratesel.h>
#include <wlc_scb.h>
#include <wlc_fragdur.h>
#include <wlc_rate.h>
#include <wlc_tx.h>

/* iovar definitions */

enum {
	IOV_FRAG_DUR = 1,
	IOV_LAST
};

static const bcm_iovar_t wlc_fragdur_iovars[] = {
	{"frag_dur", IOV_FRAG_DUR,
	0, 0, IOVT_UINT32, sizeof(int32)
	},
	{NULL, 0, 0, 0, 0, 0}
};

struct wlc_fragdur_info {
	wlc_info_t	*wlc;		/* pointer to main wlc structure */
	wlc_pub_t	*pub;		/* public common code handler */

	uint32		length;		/* Maximum frame fragment transmission duration */
};

/* fw declaration for doiovar */

static int wlc_fragdur_doiovar(void *ctx, uint32 actionid,
	void *params, uint p_len, void *arg, uint len, uint val_size, struct wlc_if *wlcif);
static int wlc_fragdur_calc_txaggthreshold(wlc_info_t *wlc, struct wlc_if *wlcif);

/* attach declaration */

#include <wlc_patch.h>

wlc_fragdur_info_t *
BCMATTACHFN(wlc_fragdur_attach)(wlc_info_t *wlc)
{

	wlc_fragdur_info_t *fragdur;

	if (!(fragdur = (wlc_fragdur_info_t *)MALLOCZ(wlc->osh, sizeof(wlc_fragdur_info_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	fragdur->wlc = wlc;
	fragdur->pub = wlc->pub;

	fragdur->length = 0;

	/* register module */
	if (wlc_module_register(fragdur->pub, wlc_fragdur_iovars, "fragdur", fragdur,
			wlc_fragdur_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s:wlc_module_register failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	return fragdur;

fail:
	MFREE(wlc->osh, fragdur, sizeof(wlc_fragdur_info_t));
	return NULL;
}

/* detach declaration */

void
BCMATTACHFN(wlc_fragdur_detach)(wlc_fragdur_info_t *fragdur)
{
	if (!fragdur)
		return;

	wlc_module_unregister(fragdur->pub, "fragdur", fragdur);

	MFREE(fragdur->pub->osh, fragdur, sizeof(wlc_fragdur_info_t));
}

/* doiovar declaration */

static int
wlc_fragdur_doiovar(void *ctx, uint32 actionid,
	void *params, uint p_len, void *arg, uint len, uint val_size, struct wlc_if *wlcif)
{
	wlc_fragdur_info_t *fragdur;
	wlc_info_t *wlc;
	uint32 *p_arg;
	int status = BCME_OK;

	fragdur = (wlc_fragdur_info_t *)ctx;
	ASSERT(fragdur != NULL);
	wlc = fragdur->wlc;
	p_arg = (uint32 *)arg;

	switch (actionid) {
		case IOV_SVAL(IOV_FRAG_DUR):
			fragdur->length = *p_arg;

			/* calcultae tx aggregation threshold */
			status = wlc_fragdur_calc_txaggthreshold(wlc, wlcif);
			if (status != BCME_OK)
				fragdur->length = 0;

			break;
		case IOV_GVAL(IOV_FRAG_DUR):
			*p_arg = fragdur->length;
			status = BCME_OK;
			break;
		default:
			status = BCME_UNSUPPORTED;
			break;
	}

	return status;
}

/* wlc_frag_threshold:
 *
 * - Used by wlc_frag to restrict maximum duration for transmitted frames according to the
 *   configured value by frag_dur iovar.
 * - Determines the frame fragmentation threshold based on current fallback rate
 *   for the respective WMM AC, which is mapped from the fifo number available to  wlc_frag().
 * - Assumes only one fallback rate is available, based on inherent limitations from
 *   Southpaw, which is the only supported platform for this feature.
 * - Fragmentation is not supported by 11n and 11ac, therefore this feature is applicable
 *   only to 11a/g/b rates.
 */

uint
wlc_fragdur_threshold(wlc_fragdur_info_t *fragdur, struct scb *scb, uint8 ac)
{
	ratespec_t fbrrspec;
	uint8 preamble_type;
	uint thresh;
	wlc_info_t *wlc = fragdur->wlc;

	fbrrspec = wlc_scb_ratesel_getfbrspec_fragdur(wlc, scb, ac);
	if ((scb->flags & SCB_SHORTPREAMBLE) && (RSPEC2RATE(fbrrspec) != WLC_RATE_1M))
		preamble_type = WLC_SHORT_PREAMBLE;
	else
		preamble_type = WLC_LONG_PREAMBLE;

	thresh = wlc_calc_frame_len_ex(wlc, fbrrspec, preamble_type, (uint) fragdur->length);

	thresh = thresh < DOT11_MIN_FRAG_LEN ? DOT11_MIN_FRAG_LEN : thresh;

	return thresh;
}

/* calculate tx aggregation threshold */
static int
wlc_fragdur_calc_txaggthreshold(wlc_info_t *wlc, struct wlc_if *wlcif)
{
	struct wlc_bsscfg *bsscfg;
	int status = BCME_OK;
	struct scb *scb;
	struct scb_iter scbiter;

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		status = wlc_scb_ratesel_calc_txaggthreshold(wlc->wrsi, scb);
		if (status != BCME_OK)
			break;
	}

	return status;
}

/* return maximum frame fragment transmission duration */
int
wlc_fragdur_length(wlc_fragdur_info_t *fragdur)
{
	if (!fragdur)
		return 0;

	return fragdur->length;
}
