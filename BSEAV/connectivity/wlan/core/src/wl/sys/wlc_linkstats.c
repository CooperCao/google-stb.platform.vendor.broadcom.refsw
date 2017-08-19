/*
 * Link Layer statistics
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
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc_hw.h>
#include <wlc_phy_hal.h>
#include <wlc_hw_priv.h>
#include <wlc.h>
#ifdef WLAMPDU
#include <wlc_ampdu.h>
#include <wlc_ampdu_rx.h>
#include <wlc_ampdu_cmn.h>
#endif
#include <wlc_pwrstats.h>
#include <wlc_linkstats.h>
#include <wlc_assoc.h>


/* iovar table */
enum {
	IOV_STATS_RATE = 1,
	IOV_STATS_RADIO,
	IOV_STATS_LAST
};

#define PREAMBLE_OFDM	0
#define PREAMBLE_HT	2
#define PREAMBLE_VHT	3
#define NSS_1X1		0
#define NSS_2X2		1
#define	CHAN_BW_20MHZ	0
#define	CHAN_BW_40MHZ	1
#define	CHAN_BW_80MHZ	2
#define MAX_NUM_RATES	32

static const uint8 ofdm_ridx[] = {
	DOT11_RATE_1M,
	DOT11_RATE_2M,
	DOT11_RATE_5M5,
	DOT11_RATE_6M,
	DOT11_RATE_9M,
	DOT11_RATE_11M,
	DOT11_RATE_12M,
	DOT11_RATE_18M,
	DOT11_RATE_24M,
	DOT11_RATE_36M,
	DOT11_RATE_48M,
	DOT11_RATE_54M };

static const uint32 ofdm_bitrate[] = {
	10,
	20,
	55,
	60,
	90,
	110,
	120,
	180,
	240,
	360,
	480,
	540 };

static const uint32 ht_mcs_bitrate[] = {
	150,
	300,
	450,
	600,
	900,
	1200,
	1350,
	1500,
	300,
	600,
	900,
	1200,
	1800,
	2400,
	2700,
	3000 };

static const uint32 vht_mcs_bitrate[] = {
	325,
	650,
	975,
	1300,
	1950,
	2600,
	2925,
	3250,
	3900,
	4333,
	650,
	1300,
	1950,
	2600,
	3900,
	5200,
	5850,
	6500,
	7800,
	8667 };

#define NUM_OFDM_RATES	(sizeof(ofdm_ridx)/sizeof(ofdm_ridx[0]))
#define NUM_HT_RATES	(sizeof(ht_mcs_bitrate)/sizeof(ht_mcs_bitrate[0]))
#define NUM_VHT_RATES	(sizeof(vht_mcs_bitrate)/sizeof(vht_mcs_bitrate[0]))

static const bcm_iovar_t wlc_linkstats_iovars[] = {
	{"ratestat", IOV_STATS_RATE,
	(IOVF_OPEN_ALLOW), 0, IOVT_BUFFER, MAX_NUM_RATES*sizeof(wifi_rate_stat_t),
	},
	{"radiostat", IOV_STATS_RADIO,
	(IOVF_OPEN_ALLOW), 0, IOVT_BUFFER, sizeof(wifi_radio_stat),
	},
	{NULL, 0, 0, 0, 0, 0}
};

struct wlc_linkstats_info {
	wlc_info_t *wlc;
	uint32 cckofdm_tx_count[NUM_OFDM_RATES];
};

static int
wlc_linkstats_doiovar(void *hdl, uint32 actionid,
	void *p, uint plen, void *arg, uint alen, uint vsize, struct wlc_if *wlcif);

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

wlc_linkstats_info_t *
BCMATTACHFN(wlc_linkstats_attach)(wlc_info_t *wlc)
{
	wlc_linkstats_info_t *linkstats_info = NULL;

	if (!(linkstats_info =
		(wlc_linkstats_info_t *)MALLOCZ(wlc->osh, sizeof(wlc_linkstats_info_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}
	linkstats_info->wlc = wlc;

	/* register module */
	if (wlc_module_register(wlc->pub, wlc_linkstats_iovars, "linkstats",
		linkstats_info, wlc_linkstats_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	wlc->pub->_link_stats = TRUE;

	return linkstats_info;

fail:
	wlc_linkstats_detach(linkstats_info);
	return NULL;
}

void
BCMATTACHFN(wlc_linkstats_detach)(wlc_linkstats_info_t *linkstats_info)
{
	wlc_module_unregister(linkstats_info->wlc->pub, "linkstats", linkstats_info);

	MFREE(linkstats_info->wlc->osh, linkstats_info, sizeof(wlc_linkstats_info_t));
}

static int
wlc_linkstats_doiovar(void *hdl, uint32 actionid,
	void *p, uint plen, void *arg, uint alen, uint vsize, struct wlc_if *wlcif)
{
	wlc_linkstats_info_t *linkstats_info = hdl;
	wlc_info_t *wlc = linkstats_info->wlc;
	int err = BCME_OK;

	switch (actionid) {
		case IOV_GVAL(IOV_STATS_RATE):
			err = wlc_rate_stat_get(wlc, arg, alen);
			break;

		case IOV_GVAL(IOV_STATS_RADIO):
			err = wlc_radio_stat_get(wlc, arg, alen);
			break;

		default:
		{
			err = BCME_UNSUPPORTED;
			break;
		}
	}

	return (err);
}

void
wlc_cckofdm_tx_inc(wlc_info_t *wlc, uint16 type, ratespec_t rate)
{
	wlc_linkstats_info_t *linkstats_info = wlc->linkstats_info;
	uint32 *count = linkstats_info->cckofdm_tx_count;

	if (type == FC_TYPE_DATA) {
		switch (rate) {
		case DOT11_RATE_1M:
			count[0]++;
			break;
		case DOT11_RATE_2M:
			count[1]++;
			break;
		case DOT11_RATE_5M5:
			count[2]++;
			break;
		case DOT11_RATE_6M:
			count[3]++;
			break;
		case DOT11_RATE_9M:
			count[4]++;
			break;
		case DOT11_RATE_11M:
			count[5]++;
			break;
		case DOT11_RATE_12M:
			count[6]++;
			break;
		case DOT11_RATE_18M:
			count[7]++;
			break;
		case DOT11_RATE_24M:
			count[8]++;
			break;
		case DOT11_RATE_36M:
			count[9]++;
			break;
		case DOT11_RATE_48M:
			count[10]++;
			break;
		case DOT11_RATE_54M:
			count[11]++;
			break;
		default:
			break;
		}
	}
}

int
wlc_rate_stat_get(wlc_info_t *wlc, void *arg, int len)
{
	wifi_rate_stat_t *ptr = (wifi_rate_stat_t *)arg;
	wlc_bsscfg_t	*cfg = wlc->cfg;
	wlc_bss_info_t	*bss = cfg->current_bss;
	uint32 *rxmbps = &wlc->pub->_cnt->rx1mbps;
	wlc_linkstats_info_t *linkstats_info = wlc->linkstats_info;
	int i;

	for (i = 0; i < NUM_OFDM_RATES; i++, ptr++, rxmbps++) {
		ptr->tx_mpdu = linkstats_info->cckofdm_tx_count[i];
		ptr->rx_mpdu = *rxmbps;
		ptr->rate.preamble = PREAMBLE_OFDM;
		ptr->rate.nss = NSS_1X1;
		ptr->rate.bw = CHAN_BW_20MHZ;
		ptr->rate.rateMcsIdx = ofdm_ridx[i];
		ptr->rate.bitrate = ofdm_bitrate[i];
	}

	if (bss) {
		if (bss->flags2 & WLC_BSS_VHT) {
			for (i = 0; i < NUM_VHT_RATES; i++, ptr++) {
				wlc_ampdu_txrates_get(wlc->ampdu_tx, ptr, i, 1);
				wlc_ampdu_rxrates_get(wlc->ampdu_rx, ptr, i, 1);
				ptr->rate.preamble = PREAMBLE_VHT;
				ptr->rate.bw = CHAN_BW_80MHZ;
				ptr->rate.bitrate = vht_mcs_bitrate[i];
				if (i < 10) {
					ptr->rate.nss = NSS_1X1;
					ptr->rate.rateMcsIdx = i;
				} else {
					ptr->rate.nss = NSS_2X2;
					ptr->rate.rateMcsIdx = i % 10;
				}
			}
		} else if (bss->flags & WLC_BSS_HT) {
			for (i = 0; i < NUM_HT_RATES; i++, ptr++) {
				wlc_ampdu_txrates_get(wlc->ampdu_tx, ptr, i, 0);
				wlc_ampdu_rxrates_get(wlc->ampdu_rx, ptr, i, 0);
				ptr->rate.preamble = PREAMBLE_HT;
				ptr->rate.bw = CHAN_BW_40MHZ;
				ptr->rate.bitrate = ht_mcs_bitrate[i];
				ptr->rate.rateMcsIdx = i;
				if (i < 8) {
					ptr->rate.nss = NSS_1X1;
				} else {
					ptr->rate.nss = NSS_2X2;
				}
			}
		}
	}

	return 0;
}

int
wlc_radio_stat_get(wlc_info_t *wlc, void *arg, int len)
{
	wifi_radio_stat *ptr = (wifi_radio_stat *)arg;
	wl_pwr_phy_stats_t phy;
	static uint32 last_pmdur = 0;
	static uint32 last_time = 0;
	static uint32 last_ontime = 0;

	wlc_pwrstats_get_active_dur((wlc_pwrstats_info_t *)wlc->pwrstats, (uint8 *)&phy,
		sizeof(wl_pwr_phy_stats_t));

	ptr->tx_time = phy.tx_dur/1000;
	ptr->rx_time = phy.rx_dur/1000;

	if (STA_ACTIVE(wlc)) {
		uint32 curr_time = OSL_SYSUPTIME();
		uint32 curr_pmdur = wlc_get_accum_pmdur(wlc);

		if (last_time != 0)
			ptr->on_time = last_ontime +
			(curr_time - last_time) - (curr_pmdur - last_pmdur);
		else
			ptr->on_time = 0;

		last_pmdur = curr_pmdur;
		last_time = curr_time;
		last_ontime = ptr->on_time;
	} else {
		if (wlc->mpc_laston_ts != 0)
			ptr->on_time = wlc->total_on_time;
		else
			ptr->on_time = 0;

	}

	return 0;
}
