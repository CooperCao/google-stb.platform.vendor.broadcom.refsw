/**
 * @file
 * @brief
 *
 * Pool reorg and revert to support offloaded packet forwarding
 * in D3 sleep state (powersave state)
 *
 * Broadcom 802.11abg Networking Device Driver
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

#include <wlc_types.h>
#include <typedefs.h>
#include <osl.h>
#include <d11_cfg.h>
#include <wlioctl.h>
#include <wlc.h>
#include <wlc_bsscfg.h>
#include <wlc_assoc.h>
#include <wlc_wlfc.h>
#include <wlc_pub.h>
#include <wlc_bmac.h>
#include <wlc_rsdb.h>
#include <wl_export.h>
#include <hnd_poolreorg.h>
#include <wlc_poolreorg.h>

static void
wlc_poolreorg_update_rxpost_rxfill(wlc_info_t *wlc, uint8 fifo_no, uint8 nrxpost)
{
	wlc_cmn_info_t *wlc_cmn = wlc->cmn;
	wlc_info_t *wlc_iter;
	int idx;

	FOREACH_WLC(wlc_cmn, idx, wlc_iter) {
		wlc_bmac_update_rxpost_rxfill(wlc_iter->hw, fifo_no, nrxpost);
	}
}

#ifdef PROP_TXSTATUS
static void
wlc_poolreorg_flush_frwd_pkts(wlc_info_t *wlc)
{
	wlc_bsscfg_t *cfg;
	int idx;

	FOR_ALL_UP_BSS(wlc, idx, cfg) {
		wlc_wlfc_flush_pkts_to_host(cfg->wlc, cfg);
	}
}
#endif /* PROP_TXSTATUS */

void
wlc_poolreorg_rx_hostmem_access(wlc_info_t *wlc, bool hostmem_access_enabled)
{
#ifdef BCMPCIEDEV
	wlc_cmn_info_t *wlc_cmn = wlc->cmn;
	wlc_info_t *wlc_iter;
	int idx;

	if (BCMPCIEDEV_ENAB()) {
		FOREACH_WLC(wlc_cmn, idx, wlc_iter) {
			/* Handle power state change handler for each wlc here to handle RSDB. */
			wlc_bmac_enable_rx_hostmem_access(wlc_iter->hw,
					hostmem_access_enabled);
		}
	}
#endif /* BCMPCIEDEV */
}

int
wlc_poolreorg_config(wlc_info_t *wlc, int reorg)
{
	poolreorg_info_t *poolreorg_info = wlc->pub->poolreorg_info;

	if (!BCMSPLITRX_ENAB()) {
		return BCME_OK;
	}

	if (reorg == TRUE) {
		/* pool reorg path */
		if (!frwd_pools_refcnt(poolreorg_info, reorg)) {
			/* Previous reference exists, so no need to pool reorg again */
			return BCME_OK;
		}

		/* reduce rxpost of split fifo and get back the buffers from dma. */
		wlc_poolreorg_update_rxpost_rxfill(wlc, RX_FIFO, NRXBUFPOST_FRWD);

		/* increase the rxpost and update rxfill for classified fifo */
		wlc_poolreorg_update_rxpost_rxfill(wlc, PKT_CLASSIFY_FIFO,
				NRXBUFPOST_CLASSIFIED_FIFO_FRWD);

		/* reorg tx frag, rx frag to shared buffers pool */
		frwd_pools_reorg(poolreorg_info);

		/* Mark to use only classify fifo for data path also */
		wlc->cmn->rxfrags_disabled = TRUE;
	} else {
		/* pool revert path */
		if (!frwd_pools_refcnt(poolreorg_info, reorg)) {
			/* Its not last reference, so don't revert pools now. */
			return BCME_OK;
		}

		/* reducing rxpost of classified fifo and getting back the buffers from dma. */
		wlc_poolreorg_update_rxpost_rxfill(wlc, PKT_CLASSIFY_FIFO,
				NRXBUFPOST_CLASSIFIED_FIFO);

		/* increasing / reverting the rxpost and update rxfill for split fifo */
		if (wlc_rsdb_mode(wlc) == PHYMODE_RSDB) {
			wlc_poolreorg_update_rxpost_rxfill(wlc, RX_FIFO, NRXBUFPOST_SMALL);
		} else {
			wlc_poolreorg_update_rxpost_rxfill(wlc, RX_FIFO, NRXBUFPOST);
		}

		/* revert shared buffer pool to tx frag, rx frag pools */
		frwd_pools_revert(poolreorg_info);

		/* Mark to use only classify fifo for data path also */
		wlc->cmn->rxfrags_disabled = FALSE;
	}

	wl_upd_frwd_resrv_bufcnt(wlc->wl);

	return BCME_OK;
}

int
wlc_poolreorg_devpwrstchg(wlc_info_t *wlc, bool hostmem_access_enabled)
{
	wlc_cmn_info_t *wlc_cmn = wlc->cmn;
	int rx_hostmem_access_enabled;

	if (!hostmem_access_enabled) {
		/* D3 powersave mode entry */
		if (NATOE_ENAB(wlc->pub)) {
			/* flush internally generated forwarding pkts
			 * which use tx / rx frags,
			 * while going into D3 mode.
			 */
#ifdef PROP_TXSTATUS
			if (PROP_TXSTATUS_ENAB(wlc->pub)) {
				wlc_poolreorg_flush_frwd_pkts(wlc);
			}
#endif /* PROP_TXSTATUS */

			wlc_cmn->poolreorg_during_D3 = TRUE;
			wlc_poolreorg_config(wlc, TRUE);
		}
	} else {
		/* D3 powersave mode exit */
		if (wlc_cmn->poolreorg_during_D3) {
			wlc_cmn->poolreorg_during_D3 = FALSE;
			wlc_poolreorg_config(wlc, FALSE);
		}
	}
	rx_hostmem_access_enabled = (hostmem_access_enabled &&
			!wlc_cmn->rxfrags_disabled);
	return rx_hostmem_access_enabled;
}
