/*
 * wlc_tsync.c
 *
 * Timestamp synchronization and Packet timestamping
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

/**
 * @file
 * @brief
 * Timestamp synchronization allows different clock domains on the host
 * and the device to be synchronized. Packets on ingress and egress are timestamped
 * and the timestamp information is provided in the completion work items.
 */


#include <wlc_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <sbchipc.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmmsgbuf.h>
#include <d11.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wl_export.h>
#include <wlc.h>
#include <wlc_hw_priv.h>
#include <wlc_rsdb.h>
#include <wlc_bmac.h>
#include <wlc_tsync.h>


static void wlc_tsync_mpc(wlc_tsync_t *tsync);
static void wlc_tsync_awake(wlc_tsync_t *tsync);
static void wlc_tsync_validate_gci(wlc_tsync_t *tsync);
static clk_id_t wlc_tsync_active_clkid(wlc_tsync_t *wlc);
static int wlc_tsync_compute_delta(wlc_tsync_t *tsync, void *clk_buf, int buflen);
static int wlc_tsync_initiate_delta(wlc_tsync_t *tsync);
static bool wlc_tsync_inject_error(wlc_tsync_t *tsync, void *tlv_buf, int tlv_len);
static void wlc_tsync_reset_tsclk(wlc_tsync_t *tsync, uint8 clk_idx);
static int wlc_tsync_validate_ts(wlc_tsync_t *tsync, uint32 clk_low, uint32 ts_low,
		uint32 ts_high, uint8 fwd);
static void wlc_tsync_adjust_tsclk(wlc_tsync_t *tsync, uint32 *clk_low, uint32 *clk_high,
		clk_id_t clk_id);
static int wlc_tsync_notify_ucode(wlc_tsync_t *tsync, void *mac_buf, int buflen);
static int wlc_tsync_frame_tlv(wlc_tsync_t *tsync, void *tlv_buf, int buflen);
static int wlc_tsync_validate_tlv(wlc_tsync_t *tsync, void *tlv_buf, int buflen);
static void wlc_tsync_configure_macstate(wlc_tsync_t *tsync, clk_id_t clk_id);
static int wlc_tsync_check_macstate(wlc_tsync_t *tsync);
static int wlc_tsync_parse_tlv(wlc_tsync_t *tsync, void *tlv_buf, int buflen);
static int wlc_tsync_sm(wlc_tsync_t *tsync, ts_event_t event, void *buf, int buflen);
static void wlc_tsync_configure_tmr(wlc_tsync_t *tsync, tsync_tmr_t tmr, bool enable);
static void wlc_tsync_wdg_tmr(void *context);
static void wlc_tsync_periodic_tmr(void *context);
static int wlc_tsync_doiovar(void *context, uint32 actionid,
	void *params, uint plen, void *arg, uint alen, uint vsize, struct wlc_if *wlcif);


const char *clk_src[MAX_FW_CLK] = {"PMU",
				   "TSF",
				   "AVB"};

enum {
	IOV_TSYNC_INJECTERR = 0
};

static const bcm_iovar_t wlc_tsync_iovars[] = {
	{ "tsync_inject_error", IOV_TSYNC_INJECTERR, (0), IOVT_UINT32, 0 },
	{ NULL, 0, 0, 0, 0 }
};

tlv_info_t tsync_tlv[BCMMSGBUF_MAX_TSYNC_TAG] = {
	{BCMMSGBUF_FW_CLOCK_INFO_TAG,
	sizeof(ts_fw_clock_info_t) - sizeof(_bcm_xtlv_t)},
	{BCMMSGBUF_HOST_CLOCK_INFO_TAG,
	sizeof(ts_host_clock_info_t) - sizeof(_bcm_xtlv_t)},
	{BCMMSGBUF_HOST_CLOCK_SELECT_TAG,
	sizeof(ts_host_clock_sel_t) - sizeof(_bcm_xtlv_t)},
	{BCMMSGBUF_D2H_CLOCK_CORRECTION_TAG,
	sizeof(ts_d2h_clock_correction_t) - sizeof(_bcm_xtlv_t)},
	{BCMMSGBUF_HOST_TIMESTAMPING_CONFIG_TAG,
	sizeof(ts_host_timestamping_config_t) - sizeof(_bcm_xtlv_t)}
	};

/*
 * Update timestamp in the packet, both on ingress and egress
 * If packet received on non primary, pre-computed delta applied
 */
void
wlc_tsync_update_ts(wlc_tsync_t *tsync, void *p, uint32 clk_tsf, uint32 clk_avb, uint8 pkt_dir)
{
	wlc_info_t *wlc = tsync->wlc;
	wlc_cmn_info_t *wlc_cmn = wlc->cmn;
	wlc_info_t *other_wlc;
	wlc_d11rxhdr_t *wrxh;
	uint32 clk_low, clk_high, save_tsf, save_rxtsf;
	uint32 pmu_delta, tsf_delta, avb_delta;
	int idx;

	wrxh = (wlc_d11rxhdr_t *)PKTDATA(wlc->osh, p);
	pmu_delta = tsf_delta = avb_delta = 0;

	if (wlc != WLC_RSDB_GET_PRIMARY_WLC(wlc)) {
		FOREACH_WLC(wlc_cmn, idx, other_wlc) {
			if (wlc != other_wlc) {
				tsync = (wlc_tsync_t *)other_wlc->tsync;

				pmu_delta = tsync->pmu_delta;
				tsf_delta = tsync->tsf_delta;
				avb_delta = tsync->avb_delta;
			}
		}
	}

	if (tsync->sync_period == 0) {
		clk_low = clk_high = TS_CLKID_INVL;
		tsync->stats.pkt_inv++;
		goto end;
	}

	if (wlc_tsync_active_clkid(tsync) >= CLK_TSF) {
		if ((clk_tsf == 0) && (clk_avb == 0)) {
			clk_low = clk_high = TS_CLKID_INVL;
			tsync->stats.pkt_inv++;
			goto end;
		}
	}

	if (wlc_tsync_active_clkid(tsync) == CLK_TSF) {

		clk_low = clk_tsf;
		clk_high = 0;

		save_tsf = wrxh->tsf_l;
		wlc_read_tsf(wlc, &clk_low, &clk_high);
		wrxh->tsf_l = htol32(clk_low);

		if (pkt_dir) {
			save_rxtsf = wrxh->rxhdr.RxTSFTime;
			wrxh->rxhdr.RxTSFTime = clk_tsf;
		}

		clk_low = wlc_recover_tsf32(wlc, wrxh);
		wrxh->tsf_l = save_tsf;

		if (pkt_dir)
			wrxh->rxhdr.RxTSFTime = save_rxtsf;

		bcm_add_64(&clk_high, &clk_low, tsf_delta);

		wlc_tsync_adjust_tsclk(tsync, &clk_low, &clk_high, CLK_TSF);
		tsync->stats.pkt_tsf++;

	} else if (wlc_tsync_active_clkid(tsync) == CLK_AVB) {

		clk_low = clk_avb;
		clk_high = 0;
		bcm_add_64(&clk_high, &clk_low, avb_delta);

		wlc_tsync_adjust_tsclk(tsync, &clk_low, &clk_high, CLK_AVB);
		tsync->stats.pkt_avb++;

	} else if (wlc_tsync_active_clkid(tsync) == CLK_PMU) {

		clk_low = wlc_current_pmu_time(wlc);
		clk_high = 0;
		bcm_add_64(&clk_high, &clk_low, pmu_delta);

		wlc_tsync_adjust_tsclk(tsync, &clk_low, &clk_high, CLK_PMU);
		tsync->stats.pkt_pmu++;

	} else {
		ASSERT(0);
	}

end:
	if (pkt_dir) {
		wl_timesync_add_tx_timestamp(wlc->wl, p, clk_low, clk_high);
	} else {
		wl_timesync_add_rx_timestamp(wlc->wl, p, clk_low, clk_high);
	}

	WL_INFORM(("wl%d: %s:F clk_low = 0x%x clk_high = 0x%x pkt_dir =  0x%x pkt_inv = 0x%x\n"
			"\t\tpkt_tsf = 0x%x pkt_avb = 0x%x pkt_pmu = 0x%x\n"
			"\t\tclk_tsf = 0x%x clk_avb = 0x%x pmu_delta = 0x%x\n"
			"\t\ttsf_delta = 0x%x avb_delta = 0x%x\n\n",
			wlc->pub->unit, __FUNCTION__, clk_low, clk_high, pkt_dir,
			tsync->stats.pkt_inv, tsync->stats.pkt_tsf, tsync->stats.pkt_avb,
			tsync->stats.pkt_pmu, clk_tsf, clk_avb, pmu_delta, tsf_delta,
			avb_delta));

	return;
}

/*
 * Deep sleep integration with the PCIe core
 */
void
wlc_tsync_process_dsevt(wlc_tsync_t *tsync, void *buf, int len)
{
	ds_event_t ds_evt = *(uint32 *)buf;

	if (tsync->sync_period == 0) {
		tsync->stats.ds_evt++;
		goto end;
	}

	switch (ds_evt) {
		case DS_ENTER:
			if (tsync->ds_active == FALSE) {
				wlc_tsync_configure_tmr(tsync, WDG_TMR, FALSE);
				wlc_tsync_configure_tmr(tsync, SYNC_TMR, FALSE);

				tsync->ds_active = TRUE;
				tsync->stats.ds_enter++;
			}
			break;

		case DS_EXIT:
			if (tsync->ds_active == TRUE) {
				tsync->state = TS_SM_START;
				wlc_tsync_configure_tmr(tsync, SYNC_TMR, TRUE);

				tsync->ds_active = FALSE;
				tsync->stats.ds_exit++;
			}
			break;

		default:
			break;
	}

end:
	WL_INFORM(("wl%d: %s: ds_state = 0x%x prev_state = 0x%x ds_enter = 0x%x\n"
			"\t\tds_exit = 0x%x ds_evt = 0x%x\n",
			tsync->wlc->pub->unit, __FUNCTION__, *(uint32 *)buf, tsync->state,
			tsync->stats.ds_enter, tsync->stats.ds_exit, tsync->stats.ds_evt));

	return;
}

/*
 * D3/D0 transition with timesync, disables timers and configures mac to allow
 * power transitions. Host reconfigures timesync on entry to D0
 */
void
wlc_tsync_process_pmevt(wlc_tsync_t *tsync, bool hmem_acc)
{
	d3_event_t d3_evt = hmem_acc;
	psm_clk_t *clk_info;

	clk_info = &tsync->psm_clk;

	switch (d3_evt) {
		case D3_ENTER:
			if (tsync->d3_active == FALSE) {
				wlc_tsync_configure_tmr(tsync, WDG_TMR, FALSE);
				wlc_tsync_configure_tmr(tsync, SYNC_TMR, FALSE);

				clk_info->psm_0.clk_vld = 0;
				clk_info->psm_1.clk_vld = 0;
				tsync->sync_period = 0;
				tsync->psm_delta = FALSE;

				wlc_tsync_configure_macstate(tsync, CLK_INV);

				tsync->d3_active = TRUE;
				tsync->stats.pm_enter++;
			}
			break;

		case D3_EXIT:
			if (tsync->d3_active == TRUE) {
				tsync->state = TS_SM_INIT;

				tsync->d3_active = FALSE;
				tsync->stats.pm_exit++;
			}
			break;

		default:
			break;
	}

	WL_INFORM(("wl%d: %s: hmem_acc = 0x%x prev_state = 0x%x pm_enter = 0x%x pm_exit = 0x%x\n",
			tsync->wlc->pub->unit, __FUNCTION__, hmem_acc, tsync->state,
			tsync->stats.pm_enter, tsync->stats.pm_exit));

	return;
}

/*
 * Entry point for host messages containing the TLVs
 */
int
wlc_tsync_process_host(wlc_tsync_t *tsync, void *pkt, int len)
{
	uint8 *pkt_data = pkt;
	int pkt_len = len, res = BCME_OK;

	tsync->rcv_seq++;
	tsync->stats.host_rsp++;

	WLC_TSYNC_DBG(pkt_data, pkt_len);
	res = wlc_tsync_sm(tsync, EVT_HOST_MSG, pkt, len);

	WL_INFORM(("wl%d: %s: buf = 0x%x len = 0x%x host_rsp = 0x%x rcv_seq = 0x%x\n"
			"\t\tsnd_seq = 0x%x res = 0x%x\n",
			tsync->wlc->pub->unit, __FUNCTION__, (uint32)pkt, len,
			tsync->stats.host_rsp, tsync->rcv_seq, tsync->snd_seq, res));

	return res;
}

/*
 * Entry point for microcode responses containing captured timestamps
 */
int
wlc_tsync_process_ucode(wlc_tsync_t *tsync)
{
	wlc_info_t *wlc = tsync->wlc;
	wlc_cmn_info_t* wlc_cmn = wlc->cmn;
	wlc_info_t *other_wlc;
	fw_clk_t fw_clk;
	psm_src_t psm_vld;
	int idx, res = BCME_OK;

	if (wlc != WLC_RSDB_GET_PRIMARY_WLC(wlc)) {
		FOREACH_WLC(wlc_cmn, idx, other_wlc) {
			if (wlc != other_wlc) {
				tsync = (wlc_tsync_t *)other_wlc->tsync;
				psm_vld = PSM_1;
			}
		}
	} else {
		psm_vld = PSM_0;
	}

	if ((tsync->d3_active) || (tsync->ds_active)) {
		res = BCME_NOTREADY;
		goto end;
	}

	setbit(&tsync->psm_vld, psm_vld);
	memset(&fw_clk, 0, sizeof(fw_clk_t));

	setbit(&fw_clk.clk_vld, CLK_AVB);
	setbit(&fw_clk.clk_vld, CLK_TSF);
	setbit(&fw_clk.clk_vld, CLK_PMU);

	switch (wlc_tsync_active_clkid(tsync)) {
		case CLK_AVB:
			SET_LOW_16(AVB_LOW_32(fw_clk),
				wlc_read_shm(wlc, M_TS_SYNC_AVB_L(wlc)));
			SET_HIGH_16(AVB_LOW_32(fw_clk),
				wlc_read_shm(wlc, M_TS_SYNC_AVB_H(wlc)));
			setbit(&fw_clk.clk_active, CLK_AVB);

		case CLK_TSF:
			SET_LOW_16(TSF_LOW_32(fw_clk),
				wlc_read_shm(wlc, M_TS_SYNC_TSF_L(wlc)));
			SET_HIGH_16(TSF_LOW_32(fw_clk),
				wlc_read_shm(wlc, M_TS_SYNC_TSF_ML(wlc)));
			setbit(&fw_clk.clk_active, CLK_TSF);

		case CLK_PMU:
			SET_LOW_16(PMU_LOW_32(fw_clk),
				wlc_read_shm(wlc, M_TS_SYNC_PMU_L(wlc)));
			SET_HIGH_16(PMU_LOW_32(fw_clk),
				wlc_read_shm(wlc, M_TS_SYNC_PMU_H(wlc)));
			setbit(&fw_clk.clk_active, CLK_PMU);

		default:
			break;
	}

	tsync->snd_seq++;
	tsync->stats.sync_cnt++;
	tsync->stats.ucode_rsp++;

	wlc_tsync_validate_gci(tsync);

	tsync->stats.pulse_wd = wlc_read_shm(wlc, M_TS_SYNC_GPIOREALDLY(wlc));

	setbit(&fw_clk.ts_src, SRC_UCODE);
	wlc_tsync_sm(tsync, EVT_UCODE_RSP, (void *)&fw_clk, sizeof(fw_clk));

end:
	WL_INFORM(("wl%d: %s: event = 0x%x pmu = 0x%x tsf = 0x%x avb = 0x%x ucode_rsp = 0x%x\n"
			"\t\tpsm_vld = 0x%x clk_active = 0x%x res = 0x%x\n",
			wlc->pub->unit, __FUNCTION__, EVT_UCODE_RSP, PMU_LOW_32(fw_clk),
			TSF_LOW_32(fw_clk), AVB_LOW_32(fw_clk), tsync->stats.ucode_rsp,
			psm_vld, wlc_tsync_active_clkid(tsync), res));

	return res;
}

/*
 * Ensures MAC remain awake after TSF or AVB selected as clock source
 */
static void
wlc_tsync_mpc(wlc_tsync_t *tsync)
{
	wlc_cmn_info_t* wlc_cmn = tsync->wlc->cmn;
	wlc_info_t *wlc;
	int idx;

	FOREACH_WLC(wlc_cmn, idx, wlc) {
		if (tsync->tsync_mpc)
			wlc_mpc_off_req_set(wlc, MPC_OFF_REQ_TSYNC_ACTIVE,
					MPC_OFF_REQ_TSYNC_ACTIVE);
		else
			wlc_mpc_off_req_set(wlc, MPC_OFF_REQ_TSYNC_ACTIVE,
					0);
	}
}

/*
 * Ensures MAC remain awake after TSF or AVB selected as clock source
 */
static void
wlc_tsync_awake(wlc_tsync_t *tsync)
{
	wlc_cmn_info_t* wlc_cmn = tsync->wlc->cmn;
	wlc_info_t *wlc;
	int idx;

	FOREACH_WLC(wlc_cmn, idx, wlc)
		wlc_user_wake_upd(wlc, WLC_USER_WAKE_REQ_TSYNC,
				tsync->tsync_awake);
}

/*
 * GCI validation for microcode GPIO assertion, deasertion
 */
static void
wlc_tsync_validate_gci(wlc_tsync_t *tsync)
{
	wlc_info_t *wlc = tsync->wlc;
	uint32 ring_idx = (tsync->tsync_gpio / 8);
	uint32 pos = (tsync->tsync_gpio % 8)*4;

	WL_INFORM(("wl%d: %s:gpiocontrol:0x%x, gci_indirect_addr:0x%x, gci_chipctrl:0x%x\n",
			wlc->pub->unit, __FUNCTION__,
			si_corereg(wlc->pub->sih, SI_CC_IDX,
			OFFSETOF(chipcregs_t, gpiocontrol), 0, 0),
			si_corereg(wlc->pub->sih, SI_CC_IDX,
			OFFSETOF(chipcregs_t, gci_indirect_addr), 0, 0),
			si_corereg(wlc->pub->sih, SI_CC_IDX,
			OFFSETOF(chipcregs_t, gci_chipctrl), 0, 0)));

	if ((si_gci_chipcontrol(wlc->pub->sih, ring_idx, 0, 0) & 1 << pos) == 0) {
			WL_INFORM((":wl%d: %s: Trap gpiocontrol:0x%x, gci_indirect_addr:0x%x\n"
					"\t\tgci_chipctrl:0x%x\n",
					wlc->pub->unit, __FUNCTION__,
					si_corereg(wlc->pub->sih, SI_CC_IDX,
					OFFSETOF(chipcregs_t, gpiocontrol), 0, 0),
					si_corereg(wlc->pub->sih, SI_CC_IDX,
					OFFSETOF(chipcregs_t, gci_indirect_addr), 0, 0),
					si_corereg(wlc->pub->sih, SI_CC_IDX,
					OFFSETOF(chipcregs_t, gci_chipctrl), 0, 0)));

			OSL_SYS_HALT();
	}

	return;
}

/*
 * Inject error in the firmware messages to allow host error validation
 */
static bool
wlc_tsync_inject_error(wlc_tsync_t *tsync, void *tlv_buf, int tlv_len)
{
	ts_fw_clock_info_t *ts_fw;
	bool res = TRUE;

	if (tsync->err_inject == 0)
		goto end;

	ts_fw = (ts_fw_clock_info_t *)tlv_buf;

	if (isset(&tsync->err_inject, ERR_OOO_SEQ)) {
		ts_fw->reset_cnt += 1;

		tsync->stats.seq_err++;
		clrbit(&tsync->err_inject, ERR_OOO_SEQ);

	} else if (isset(&tsync->err_inject, ERR_INV_TLVID)) {
		ts_fw->xtlv.id = BCMMSGBUF_MAX_TSYNC_TAG;

		tsync->stats.tlvid_err++;
		clrbit(&tsync->err_inject, ERR_INV_TLVID);

	} else if (isset(&tsync->err_inject, ERR_INV_TLVLEN)) {
		ts_fw->xtlv.len = sizeof(ts_fw_clock_info_t);

		tsync->stats.tlvlen_err++;
		clrbit(&tsync->err_inject, ERR_INV_TLVLEN);

	} else if (isset(&tsync->err_inject, ERR_DROP_FWTS)) {
		res = FALSE;

		tsync->stats.fwts_err++;
		clrbit(&tsync->err_inject, ERR_DROP_FWTS);
	} else {
		ASSERT(0);
	}

end:
	WL_INFORM(("wl%d: %s: seq_err = 0x%x tlvid_err = 0x%x tlvlen_err = 0x%x\n"
			"\t\tfwts_err = 0x%x res = 0x%x\n",
			tsync->wlc->pub->unit, __FUNCTION__, tsync->stats.seq_err,
			tsync->stats.tlvid_err, tsync->stats.tlvlen_err,
			tsync->stats.fwts_err, res));

	return res;
}

/*
 * Reset clock source if not in use
 */
static void
wlc_tsync_reset_tsclk(wlc_tsync_t *tsync, uint8 clk_idx)
{
	switch (clk_idx) {
		case CLK_PMU:
			PMU_LOW_32(tsync->fw_clk) = 0;
			PMU_HIGH_32(tsync->fw_clk) = 0;
			break;

		case CLK_TSF:
			TSF_LOW_32(tsync->fw_clk) = 0;
			TSF_HIGH_32(tsync->fw_clk) = 0;
			break;

		case CLK_AVB:
			AVB_LOW_32(tsync->fw_clk) = 0;
			AVB_HIGH_32(tsync->fw_clk) = 0;
			break;

		default:
			break;
	}

	return;
}

/*
 * Validation for timestamp before update to the timesync DB
 */
static int
wlc_tsync_validate_ts(wlc_tsync_t *tsync, uint32 clk_low, uint32 ts_low, uint32 ts_high, uint8 fwd)
{
	uint8 adj_ts = 1;

	if (fwd) {
		if ((ts_low == 0) && (ts_high == 0)) {
			adj_ts = 1;
			goto end;
		}

		if (!IS_DUP_RANGE(clk_low, ts_low)) {
			adj_ts = 0;
			goto end;
		}
	} else {
		if (!IS_DUP_RANGE(ts_low, clk_low)) {
			adj_ts = 1;
			goto end;
		} else {
			adj_ts = 0;
		}
	}

end:
	return adj_ts;
}

/*
 * Manages 64 bit counter for different clock sources
 */
static void
wlc_tsync_adjust_tsclk(wlc_tsync_t *tsync, uint32 *clk_low, uint32 *clk_high, clk_id_t clk_id)
{
	uint8 adj_ts = 1;

	switch (clk_id) {
		case CLK_PMU:
			if (!TS_GE(*clk_low, PMU_LOW_32(tsync->fw_clk))) {

				adj_ts = wlc_tsync_validate_ts(tsync, *clk_low,
						PMU_LOW_32(tsync->fw_clk),
						PMU_HIGH_32(tsync->fw_clk), 0);

				if (adj_ts)
					PMU_HIGH_32(tsync->fw_clk)++;

			} else {
				adj_ts = wlc_tsync_validate_ts(tsync, *clk_low,
						PMU_LOW_32(tsync->fw_clk),
						PMU_HIGH_32(tsync->fw_clk), 1);
			}

			if (adj_ts)
				PMU_LOW_32(tsync->fw_clk) = *clk_low;

			SET_CLKID(PMU_HIGH_32(tsync->fw_clk), clk_id);
			*clk_high = PMU_HIGH_32(tsync->fw_clk);

			break;

		case CLK_TSF:
			if (!TS_GE(*clk_low, TSF_LOW_32(tsync->fw_clk))) {
				adj_ts = wlc_tsync_validate_ts(tsync, *clk_low,
						TSF_LOW_32(tsync->fw_clk),
						TSF_HIGH_32(tsync->fw_clk), 0);

				if (adj_ts)
					TSF_HIGH_32(tsync->fw_clk)++;

			} else {
				adj_ts = wlc_tsync_validate_ts(tsync, *clk_low,
						TSF_LOW_32(tsync->fw_clk),
						TSF_HIGH_32(tsync->fw_clk), 1);
			}

			if (adj_ts)
				TSF_LOW_32(tsync->fw_clk) = *clk_low;

			SET_CLKID(TSF_HIGH_32(tsync->fw_clk), clk_id);
			*clk_high = TSF_HIGH_32(tsync->fw_clk);

			break;

		case CLK_AVB:
			if (!TS_GE(*clk_low, AVB_LOW_32(tsync->fw_clk))) {
				adj_ts = wlc_tsync_validate_ts(tsync, *clk_low,
						AVB_LOW_32(tsync->fw_clk),
						AVB_HIGH_32(tsync->fw_clk), 0);

				if (adj_ts)
					AVB_HIGH_32(tsync->fw_clk)++;

			} else {
				adj_ts = wlc_tsync_validate_ts(tsync, *clk_low,
						AVB_LOW_32(tsync->fw_clk),
						AVB_HIGH_32(tsync->fw_clk), 1);
			}

			if (adj_ts)
				AVB_LOW_32(tsync->fw_clk) = *clk_low;

			SET_CLKID(AVB_HIGH_32(tsync->fw_clk), clk_id);
			*clk_high = AVB_HIGH_32(tsync->fw_clk);

			break;

		default:
			ASSERT(0);
			break;
	}

	WL_INFORM(("wl%d: %s:F clk_low = 0x%x clk_high = 0x%x clk_id = 0x%x adj_ts = 0x%x\n"
			"\t\tpmu_low = 0x%x pmu_high = 0x%x tsf_low = 0x%x\n"
			"\t\ttsf_high = 0x%x avb_low = 0x%x avb_high = 0x%x\n",
			tsync->wlc->pub->unit, __FUNCTION__, (uint32)*clk_low, (uint32)*clk_high,
			clk_id, adj_ts, PMU_LOW_32(tsync->fw_clk), PMU_HIGH_32(tsync->fw_clk),
			TSF_LOW_32(tsync->fw_clk), TSF_HIGH_32(tsync->fw_clk),
			AVB_LOW_32(tsync->fw_clk), AVB_HIGH_32(tsync->fw_clk)));

	return;
}

/*
 * Active clock source in use by device
 */
static clk_id_t
wlc_tsync_active_clkid(wlc_tsync_t *tsync)
{
	return tsync->min_clkid;
}

/*
 * Computes the delta as viewed by two PSMs for different clock sources
 * MAC re-enabled after the computation process
 */
static int
wlc_tsync_compute_delta(wlc_tsync_t *tsync, void *clk_buf, int buflen)
{
	wlc_info_t *wlc = tsync->wlc;
	wlc_cmn_info_t* wlc_cmn = wlc->cmn;
	wlc_info_t *other_wlc;
	fw_clk_t *psm_buf, psm_0, psm_1;
	uint32 pmu_delta, tsf_delta, avb_delta;
	int idx, res = BCME_OK;

	memset(&psm_0, 0, sizeof(fw_clk_t));
	memset(&psm_1, 0, sizeof(fw_clk_t));
	pmu_delta = tsf_delta = avb_delta = 0;

	if (isset(&tsync->psm_vld, PSM_0)) {
		psm_buf = &tsync->psm_clk.psm_0;
		memcpy(psm_buf, clk_buf, buflen);

		clrbit(&tsync->psm_vld, PSM_0);
		psm_buf = &tsync->psm_clk.psm_1;

		if (psm_buf->clk_vld == 0) {
			res = BCME_BUSY;
			psm_buf = &tsync->psm_clk.psm_0;
			goto busy;
		}
	} else if (isset(&tsync->psm_vld, PSM_1)) {
		psm_buf = &tsync->psm_clk.psm_1;
		memcpy(psm_buf, clk_buf, buflen);

		clrbit(&tsync->psm_vld, PSM_1);
		psm_buf = &tsync->psm_clk.psm_0;

		if (psm_buf->clk_vld == 0) {
			res = BCME_BUSY;
			psm_buf = &tsync->psm_clk.psm_1;
			goto busy;
		}
	}

	psm_0 = tsync->psm_clk.psm_0;
	psm_1 = tsync->psm_clk.psm_1;

	pmu_delta = PMU_LOW_32(psm_0) - PMU_LOW_32(psm_1);
	tsf_delta = TSF_LOW_32(psm_0) - TSF_LOW_32(psm_1);
	avb_delta = AVB_LOW_32(psm_0) - AVB_LOW_32(psm_1);

	wlc->tsync->pmu_delta = pmu_delta;
	wlc->tsync->tsf_delta = tsf_delta;
	wlc->tsync->avb_delta = avb_delta;

	FOREACH_WLC(wlc_cmn, idx, other_wlc) {
		wlc_enable_mac(other_wlc);
	}

	tsync->psm_delta = TRUE;

	WL_INFORM(("wl%d: %s: psm0_vld = 0x%x psm1_vld = 0x%x pmu_delta = 0x%x\n"
			"\t\ttsf_delta = 0x%x avb_delta = 0x%x res = 0x%x\n",
			wlc->pub->unit, __FUNCTION__, psm_0.clk_vld, psm_1.clk_vld,
			pmu_delta, tsf_delta, avb_delta, res));

	return res;

busy:
	WL_INFORM(("wl%d: %s: psm_vld = 0x%x pmu = 0x%x tsf = 0x%x\n"
			"\t\tavb = 0x%x res = 0x%x\n",
			wlc->pub->unit, __FUNCTION__, psm_buf->clk_vld, psm_buf->pmu_ticks.t_low,
			psm_buf->tsf_ticks.t_low, psm_buf->avb_ticks.t_low, res));

	return res;
}

/*
 * Initiate delta computation as viewed by two PSMs for different clock sources.
 * AVB is enabled and MAC suspended during the computation process
 */
static int
wlc_tsync_initiate_delta(wlc_tsync_t *tsync)
{
	wlc_info_t *wlc = tsync->wlc;
	wlc_cmn_info_t* wlc_cmn = wlc->cmn;
	wlc_info_t *other_wlc;
	int idx, res = BCME_BUSY;

	if ((RSDB_ENAB(wlc->pub) == 0)) {
		tsync->psm_delta = TRUE;
		res = BCME_OK;
		goto end;
	}

	FOREACH_WLC(wlc_cmn, idx, other_wlc) {
		if (wlc != other_wlc) {
			if (other_wlc->pub->up == 0) {
				res = BCME_NOTUP;
				goto end;
			}
		}
	}

	if (wlc->pub->up == 0) {
		res = BCME_NOTUP;
		goto end;
	}

	wlc_enable_avb_timer(wlc->hw, TRUE);
	wlc_suspend_mac_and_wait(wlc);
	tsync->psm_vld = 0;

	FOREACH_WLC(wlc_cmn, idx, other_wlc) {
		if (wlc != other_wlc) {
			wlc_enable_avb_timer(other_wlc->hw, TRUE);
			wlc_suspend_mac_and_wait(other_wlc);
		}
	}

	tsync->min_clkid = CLK_AVB;
	wlc_start_tsync(wlc);
	tsync->stats.ucode_req++;

end:
	WL_INFORM(("wl%d: %s: ucode_req = 0x%x ucode_rsp = 0x%x wlc = 0x%x\n"
			"\t\tpri_wlc = 0x%x res = 0x%x\n",
			wlc->pub->unit, __FUNCTION__, tsync->stats.ucode_req,
			tsync->stats.ucode_rsp, (uint32)wlc,
			(uint32)WLC_RSDB_GET_PRIMARY_WLC(wlc), res));

	return res;
}

/*
 * Triggers timestamp capture by microcode by writing to mac command register
 * If PMU selected as clock source, an internal state change event is generated
 */
static int
wlc_tsync_notify_ucode(wlc_tsync_t *tsync, void *mac_buf, int buflen)
{
	wlc_info_t *wlc = tsync->wlc;
	uint32 ts_low, ts_high, ts_gpio;
	fw_clk_t fw_clk;
	d11regs_t *regs;
	int res = BCME_OK;

	memset(&fw_clk, 0, sizeof(fw_clk_t));

	setbit(&fw_clk.clk_vld, CLK_AVB);
	setbit(&fw_clk.clk_vld, CLK_TSF);
	setbit(&fw_clk.clk_vld, CLK_PMU);

	switch (wlc_tsync_active_clkid(tsync)) {
		case CLK_AVB:
		case CLK_TSF:
			regs = wlc->regs;
			ts_gpio = ((mac_buf_t *)mac_buf)->gpio;

			OR_REG(wlc->osh, &regs->psm_gpio_oe, ts_gpio);
			wlc_tsync_validate_gci(tsync);

			tsync->stats.pulse_wd = wlc_read_shm(wlc, M_TS_SYNC_GPIOREALDLY(wlc));

			wlc_write_shm(wlc, M_TS_SYNC_GPIO(wlc), ts_gpio);
			tsync->stats.ucode_req++;
			wlc_start_tsync(wlc);
			goto end;

		case CLK_PMU:
			ts_low = ts_high = 0;
			ts_low = wlc_current_pmu_time(wlc);
			PMU_LOW_32(fw_clk) = ts_low;
			PMU_HIGH_32(fw_clk) = ts_high;
			setbit(&fw_clk.clk_active, CLK_PMU);

		default:
			break;
	}

	tsync->stats.fw_req++;
	tsync->state = TS_WAIT_UCODE;
	setbit(&fw_clk.ts_src, SRC_FW);
	res = wlc_tsync_sm(tsync, EVT_UCODE_RSP, &fw_clk, sizeof(fw_clk_t));

end:
	WL_INFORM(("wl%d: %s: mac_buf = 0x%x len = 0x%x ucode_req = 0x%x fw_req = 0x%x\n"
			"\t\tclk_vld = 0x%x clk_active = 0x%x res = 0x%x\n",
			wlc->pub->unit, __FUNCTION__, (uint32)mac_buf, buflen,
			tsync->stats.ucode_req, tsync->stats.fw_req, fw_clk.clk_vld,
			fw_clk.clk_active, res));

	return res;
}

/*
 * Framing routine for TLVs. Host receives all available clock sources in the device.
 * Active bit in each clock source indicates the current state
 */
static int
wlc_tsync_frame_tlv(wlc_tsync_t *tsync, void *clk_buf, int buflen)
{
	wlc_info_t *wlc = tsync->wlc;
	fw_clk_t fw_clk, *tmp_clk = (fw_clk_t *)clk_buf;
	ts_fw_clock_info_t tmp_fw[MAX_FW_CLK], *ts_fw;
	void *ts_pkt;
	uint8 *dbg_pkt;
	int pktlen, fwclk_sz = 0;
	int res = BCME_OK;

	dbg_pkt = (uint8 *)tmp_fw;
	fw_clk = *tmp_clk;
	ts_fw = tmp_fw;

	memset(ts_fw, 0, sizeof(ts_fw_clock_info_t) * MAX_FW_CLK);

	if (isset(&fw_clk.clk_vld, CLK_PMU)) {
		wlc_tsync_adjust_tsclk(tsync, &fw_clk.pmu_ticks.t_low,
				&fw_clk.pmu_ticks.t_high, CLK_PMU);

		ts_fw->ts.ts_low = PMU_LOW_32(tsync->fw_clk);
		ts_fw->ts.ts_high = PMU_HIGH_32(tsync->fw_clk);
		SET_CLKID(ts_fw->ts.ts_high, CLK_PMU);

		memcpy(ts_fw->clk_src, clk_src[CLK_PMU], 3);
		ts_fw->clk_src[3] = '\0';
		ts_fw->nominal_clock_freq = FREQ_PMU;

		if (!isset(&fw_clk.clk_active, CLK_PMU) && isset(&tsync->clk_active, CLK_PMU))
			tsync->fw_rst[CLK_PMU]++;
		ts_fw->reset_cnt = tsync->fw_rst[CLK_PMU];

		ts_fw->flags = CAP_DEVICE_TS;
		if (isset(&fw_clk.clk_active, CLK_PMU))
			ts_fw->flags |= TS_CLK_ACTIVE;

		ts_fw->xtlv.id = BCMMSGBUF_FW_CLOCK_INFO_TAG;
		ts_fw->xtlv.len = sizeof(ts_fw_clock_info_t) - sizeof(_bcm_xtlv_t);

		fwclk_sz += sizeof(ts_fw_clock_info_t);
		ts_fw++;
	}

	if (isset(&fw_clk.clk_vld, CLK_TSF)) {
		if (isset(&fw_clk.ts_src, SRC_UCODE)) {
			wlc_tsync_adjust_tsclk(tsync, &fw_clk.tsf_ticks.t_low,
					&fw_clk.tsf_ticks.t_high, CLK_TSF);
		} else {
			TSF_HIGH_32(tsync->fw_clk) = TSF_HIGH_32(fw_clk);
			TSF_LOW_32(tsync->fw_clk) = TSF_LOW_32(fw_clk);
		}

		ts_fw->ts.ts_low = TSF_LOW_32(tsync->fw_clk);
		ts_fw->ts.ts_high = TSF_HIGH_32(tsync->fw_clk);
		SET_CLKID(ts_fw->ts.ts_high, CLK_TSF);

		memcpy(ts_fw->clk_src, clk_src[CLK_TSF], 3);
		ts_fw->clk_src[3] = '\0';
		ts_fw->nominal_clock_freq = FREQ_TSF;

		if (!isset(&fw_clk.clk_active, CLK_TSF) && isset(&tsync->clk_active, CLK_TSF))
			tsync->fw_rst[CLK_TSF]++;
		ts_fw->reset_cnt = tsync->fw_rst[CLK_TSF];

		ts_fw->flags = CAP_DEVICE_TS;
		if (isset(&fw_clk.clk_active, CLK_TSF))
			ts_fw->flags |= TS_CLK_ACTIVE;

		ts_fw->xtlv.id = BCMMSGBUF_FW_CLOCK_INFO_TAG;
		ts_fw->xtlv.len = sizeof(ts_fw_clock_info_t) - sizeof(_bcm_xtlv_t);

		fwclk_sz += sizeof(ts_fw_clock_info_t);
		ts_fw++;
	}

	if (isset(&fw_clk.clk_vld, CLK_AVB)) {
		wlc_tsync_adjust_tsclk(tsync, &fw_clk.avb_ticks.t_low,
			&fw_clk.avb_ticks.t_high, CLK_AVB);

		ts_fw->ts.ts_low = AVB_LOW_32(tsync->fw_clk);
		ts_fw->ts.ts_high = AVB_HIGH_32(tsync->fw_clk);
		SET_CLKID(ts_fw->ts.ts_high, CLK_AVB);

		memcpy(ts_fw->clk_src, clk_src[CLK_AVB], 3);
		ts_fw->clk_src[3] = '\0';
		ts_fw->nominal_clock_freq = FREQ_AVB;

		if (!isset(&fw_clk.clk_active, CLK_AVB) && isset(&tsync->clk_active, CLK_AVB))
			tsync->fw_rst[CLK_AVB]++;
		ts_fw->reset_cnt = tsync->fw_rst[CLK_AVB];

		ts_fw->flags = CAP_DEVICE_TS;
		if (isset(&fw_clk.clk_active, CLK_AVB))
			ts_fw->flags |= TS_CLK_ACTIVE;

		ts_fw->xtlv.id = BCMMSGBUF_FW_CLOCK_INFO_TAG;
		ts_fw->xtlv.len = sizeof(ts_fw_clock_info_t) - sizeof(_bcm_xtlv_t);

		fwclk_sz += sizeof(ts_fw_clock_info_t);
	}

	tsync->clk_vld = fw_clk.clk_vld;
	tsync->clk_active = fw_clk.clk_active;

	if (wlc_tsync_inject_error(tsync, tmp_fw, fwclk_sz) == FALSE) {
		goto fail;
	}

	pktlen = TSPOOL_PKTSIZE;
	if ((ts_pkt = PKTGET(wlc->osh, pktlen, FALSE)) == NULL) {
		res = BCME_NOMEM;
		ASSERT(0);
		goto fail;
	}

	PKTSETLEN(wlc->osh, ts_pkt, sizeof(cmn_msg_hdr_t) + fwclk_sz);
	PKTPULL(wlc->osh, ts_pkt, sizeof(cmn_msg_hdr_t));
	bcopy(tmp_fw, PKTDATA(wlc->osh, ts_pkt), fwclk_sz);

	tsync->stats.host_req++;
	WLC_TSYNC_DBG(dbg_pkt, fwclk_sz);
	wl_sendctl_tx(wlc->wl, PCIEDEV_FIRMWARE_TSINFO, 0, ts_pkt);

fail:
	WL_INFORM(("wl%d: %s: clk_buf = 0x%x len = 0x%x host_req = 0x%x\n"
			"\t\tclk_valid = 0x%x res = 0x%x\n",
			wlc->pub->unit, __FUNCTION__, (uint32)clk_buf,
			sizeof(cmn_msg_hdr_t) + fwclk_sz, tsync->stats.host_req,
			fw_clk.clk_vld, res));

	return res;
}

/*
 * Configures sync timer for periodic timesync message, watchdog timer for
 * tracking host response
 */
static void
wlc_tsync_configure_tmr(wlc_tsync_t *tsync, tsync_tmr_t tmr, bool enable)
{
	wlc_info_t *wlc = tsync->wlc;

	switch (tmr) {
		case WDG_TMR:
			if (enable) {
				if (tsync->wdg_active == FALSE) {
					wl_add_timer(wlc->wl, tsync->wdg_tmr,
						tsync->wdg_period * 1000, 0);
					tsync->wdg_active = TRUE;
				} else {
					WL_INFORM(("wl%d: %s: tmr = 0x%x enable = 0x%x\n"
							"\tsync_active = 0x%x\n"
							"\twdg_active = 0x%x\n",
							wlc->pub->unit, __FUNCTION__, tmr, enable,
							tsync->sync_active, tsync->wdg_active));
				}
			} else {
				if (tsync->wdg_active) {
					wl_del_timer(wlc->wl, tsync->wdg_tmr);
					tsync->wdg_active = FALSE;
				}
			}
			break;

		case SYNC_TMR:
			if (enable) {
				if (tsync->sync_active == FALSE) {
					wl_add_timer(wlc->wl, tsync->sync_tmr,
						tsync->sync_period, 1);
					tsync->sync_active = TRUE;
				}
			} else {
				if (tsync->sync_active) {
					wl_del_timer(wlc->wl, tsync->sync_tmr);
					tsync->sync_active = FALSE;
				}
			}
			break;

		default:
			ASSERT(0);
			break;
	}

	WL_INFORM(("wl%d: %s: tmr = 0x%x enable = 0x%x sync_active = 0x%x wdg_active = 0x%x\n",
			wlc->pub->unit, __FUNCTION__, tmr, enable,
			tsync->sync_active, tsync->wdg_active));

	return;
}

/*
 * Validation for incoming TLVs from the host
 */
static int
wlc_tsync_validate_tlv(wlc_tsync_t *tsync, void *tlv_buf, int buflen)
{
	_bcm_xtlv_t *ts_tlv;
	bcm_xtlv_opts_t opts = BCM_XTLV_OPTION_NONE;
	uint16 tlv_id = 0, tlv_len = 0;
	int res = BCME_OK;

	ts_tlv =  bcm_valid_xtlv((bcm_xtlv_t *)tlv_buf, buflen, opts) ?
			(_bcm_xtlv_t *)tlv_buf : NULL;

	if (!ts_tlv) {
		tsync->stats.tlv_invbuf++;
		res = BCME_BADARG;
		goto fail;
	}

	while (ts_tlv != NULL) {
		tlv_id = ts_tlv->id;
		tlv_len = ts_tlv->len;

		if (tlv_id >=  BCMMSGBUF_MAX_TSYNC_TAG) {
			tsync->stats.tlv_invid++;
			res = BCME_BADARG;
			goto fail;
		}

		if (tlv_len != tsync_tlv[tlv_id].tlv_len) {
			tsync->stats.tlv_invlen++;
			res = BCME_BADARG;
			goto fail;
		}

		ts_tlv = (_bcm_xtlv_t *)bcm_next_xtlv((bcm_xtlv_t *)ts_tlv, &buflen, opts);
	}

	if (buflen != 0) {
		tsync->stats.tlv_buflen++;
		res = BCME_BUFTOOSHORT;
		goto fail;
	}

	return res;

fail:
	WL_INFORM(("wl%d: %s: tlv_invbuf = 0x%x tlv_invid = 0x%x tlv_invlen = 0x%x\n"
			"\t\ttlv_buflen = 0x%x\n",
			 tsync->wlc->pub->unit, __FUNCTION__, tsync->stats.tlv_invbuf,
			 tsync->stats.tlv_invid, tsync->stats.tlv_invlen, tsync->stats.tlv_buflen));

	OSL_SYS_HALT();
	return res;
}

/*
 * Enables packet timestamping based on the clock source selected.
 * Ensures that MAC PM state in sync with the clock source
 */
static void
wlc_tsync_configure_macstate(wlc_tsync_t *tsync, clk_id_t clk_id)
{
	wlc_cmn_info_t* wlc_cmn = tsync->wlc->cmn;
	wlc_info_t *wlc;
	bool avb_active = tsync->avb_active;
	int idx;

	FOREACH_WLC(wlc_cmn, idx, wlc) {
		switch (clk_id) {
			case CLK_AVB:
				if (tsync->avb_active == FALSE) {
					wlc_enable_avb_timer(wlc->hw, TRUE);
				}
				avb_active = TRUE;

			case CLK_TSF:
				tsync->tsync_awake = TRUE;
				wlc_tsync_awake(tsync);

				tsync->tsync_mpc = TRUE;
				wlc_tsync_mpc(tsync);

				break;


			case CLK_INV:
				if (tsync->avb_active == TRUE) {
					if (wlc->pub->up) {
						wlc_enable_avb_timer(wlc->hw, FALSE);
					}
				}
				avb_active = FALSE;

				tsync->tsync_mpc = FALSE;
				wlc_tsync_mpc(tsync);

				tsync->tsync_awake = FALSE;
				wlc_tsync_awake(tsync);

				break;

			default:
				break;
		}
	}

	tsync->avb_active = avb_active;

	WL_INFORM(("wl%d: %s: clk_id = 0x%x avb_active = 0x%x tsync_mpc = 0x%x\n"
			"\t\ttsync_awake = 0x%x\n",
			wlc->pub->unit, __FUNCTION__, clk_id, avb_active,
			tsync->tsync_mpc, tsync->tsync_awake));

	return;
}

/*
 * Wakes up MAC if down for MPC
 */
static int
wlc_tsync_check_macstate(wlc_tsync_t *tsync)
{
	wlc_cmn_info_t* wlc_cmn = tsync->wlc->cmn;
	wlc_info_t *wlc;
	int idx, res = BCME_OK;

	FOREACH_WLC(wlc_cmn, idx, wlc) {
		if (wlc->pub->up == 0) {
			tsync->tsync_mpc = TRUE;
			wlc_tsync_mpc(tsync);

			if (wlc->pub->up == 0) {
				res = BCME_NOTUP;
				goto end;
			}

			tsync->stats.mpc_upd++;
		}
	}

end:
	WL_INFORM(("wl%d: %s: wl_up = 0x%x mpc_upd = 0x%x res = 0x%x\n",
			tsync->wlc->pub->unit, __FUNCTION__, wlc->pub->up,
			tsync->stats.mpc_upd, res));

	return res;
}

/*
 * Parses incoming TLVs from host. Configures device clock source, enables timesync
 * message periodicity
 */
static int
wlc_tsync_parse_tlv(wlc_tsync_t *tsync, void *tlv_buf, int buflen)
{
	bcm_xtlv_opts_t opts = BCM_XTLV_OPTION_NONE;
	_bcm_xtlv_t *ts_tlv = NULL;
	ts_host_timestamping_config_t *ts_conf;
	ts_host_clock_info_t *hst_clk;
	ts_fw_clock_info_t *fw_clk;
	ts_host_clock_sel_t *clk_sel;
	uint8 clk_idx;
	bool conf_tmr = FALSE;
	int res = BCME_OK;

	res = wlc_tsync_validate_tlv(tsync, tlv_buf, buflen);
	if (res != BCME_OK)
		goto fail;

	ts_tlv =  (_bcm_xtlv_t *)tlv_buf;

	for (; ts_tlv != NULL && res == BCME_OK;
		ts_tlv = (_bcm_xtlv_t *)bcm_next_xtlv((bcm_xtlv_t *)ts_tlv, &buflen, opts)) {
		switch (ts_tlv->id) {
			case BCMMSGBUF_FW_CLOCK_INFO_TAG:
				fw_clk = (ts_fw_clock_info_t *)ts_tlv;

				if ((memcmp(fw_clk->clk_src, clk_src[CLK_PMU], 3) == 0) ||
					(memcmp(fw_clk->clk_src, clk_src[CLK_TSF], 3) == 0) ||
					(memcmp(fw_clk->clk_src, clk_src[CLK_AVB], 3) == 0)) {

					tsync->stats.fwclk_tag++;
				} else {
					res = BCME_NOCLK;
					goto fail;
				}
				break;

			case BCMMSGBUF_HOST_CLOCK_INFO_TAG:
				hst_clk = (ts_host_clock_info_t *)ts_tlv;

				tsync->stats.hclk_tag++;
				HTCK_LOW_32(tsync->hs_clk) = hst_clk->ticks.low;
				HTCK_HIGH_32(tsync->hs_clk) = hst_clk->ticks.high;
				HTME_LOW_32(tsync->hs_clk) = hst_clk->ns.low;
				HTME_HIGH_32(tsync->hs_clk) = hst_clk->ns.high;
				break;

			case BCMMSGBUF_HOST_CLOCK_SELECT_TAG:
				clk_sel = (ts_host_clock_sel_t *)ts_tlv;

				if ((clk_sel->min_clk_idx >= MAX_FW_CLK) ||
					(clk_sel->max_clk_idx >= MAX_FW_CLK)) {
					res = BCME_NOCLK;
					goto fail;
				}

				if (clk_sel->min_clk_idx >= CLK_TSF) {
					if ((res = wlc_tsync_check_macstate(tsync)) != BCME_OK)
						goto fail;

					if (tsync->psm_delta == FALSE) {
						tsync->state = TS_SM_INIT;
						res = wlc_tsync_sm(tsync, EVT_HOST_MSG,
								tlv_buf, buflen);
						goto fail;
					}
				}

				if (clk_sel->min_clk_idx > tsync->min_clkid) {
					clk_idx = tsync->min_clkid;
					for (; clk_idx < clk_sel->min_clk_idx; clk_idx++) {
						tsync->fw_rst[clk_idx]++;
						wlc_tsync_reset_tsclk(tsync, clk_idx);
					}
				}

				if (clk_sel->max_clk_idx < tsync->max_clkid) {
					clk_idx = tsync->max_clkid;
					for (; clk_idx > clk_sel->max_clk_idx; clk_idx--) {
						tsync->fw_rst[clk_idx]++;
						wlc_tsync_reset_tsclk(tsync, clk_idx);
					}
				}

				if (clk_sel->min_clk_idx < tsync->min_clkid) {
					clk_idx = clk_sel->min_clk_idx;
					for (; clk_idx < (CLK_AVB + 1); clk_idx++) {
						wlc_tsync_reset_tsclk(tsync, clk_idx);
					}
				}

				tsync->stats.clksel_tag++;
				tsync->min_clkid = clk_sel->min_clk_idx;
				tsync->max_clkid = clk_sel->max_clk_idx;

				if (wlc_tsync_active_clkid(tsync) >= CLK_AVB) {
					wlc_tsync_configure_macstate(tsync, CLK_AVB);
				} else if (wlc_tsync_active_clkid(tsync) >= CLK_TSF) {
					wlc_tsync_configure_macstate(tsync, CLK_TSF);
				} else {
					wlc_tsync_configure_macstate(tsync, CLK_INV);
				}

				WL_INFORM(("wl%d: %s: min_clkid = 0x%x max_clkid = 0x%x\n"
						"\t\tavb_active = 0x%x\n"
						"\t\ttsync_mpc = 0x%x\n"
						"\t\ttsync_awake = 0x%x\n",
						 tsync->wlc->pub->unit, __FUNCTION__,
						 tsync->min_clkid, tsync->max_clkid,
						 tsync->avb_active, tsync->tsync_mpc,
						 tsync->tsync_awake));

				break;

			case BCMMSGBUF_HOST_TIMESTAMPING_CONFIG_TAG:
				ts_conf = (ts_host_timestamping_config_t *)ts_tlv;

				if (ts_conf->flags & FLAG_HOST_RESET) {
					tsync->snd_seq = 0;
					tsync->rcv_seq = 0;
					tsync->err_inject = 0;
					tsync->sync_period = 0;
					tsync->psm_delta = FALSE;
					memset(&tsync->stats, 0, sizeof(ts_stats_t));
					tsync->hs_rst++;

					if (tsync->hs_rst != ts_conf->reset_cnt)
						ASSERT(0);
				}

				tsync->stats.tsconf_tag++;
				if (tsync->sync_period != ts_conf->period_ms) {
					tsync->sync_period = ts_conf->period_ms;
					conf_tmr = TRUE;
				}

				if (conf_tmr) {
					if (tsync->sync_period == 0) {
						wlc_tsync_configure_tmr(tsync, SYNC_TMR, FALSE);
						wlc_tsync_configure_tmr(tsync, WDG_TMR, FALSE);
						wlc_tsync_configure_macstate(tsync, CLK_INV);
					} else {
						if (tsync->sync_active) {
							wlc_tsync_configure_tmr(tsync, SYNC_TMR,
									FALSE);
						}

						wlc_tsync_configure_tmr(tsync, SYNC_TMR,
								TRUE);
					}
				}

				WL_INFORM(("wl%d: %s: sync_period = 0x%x sync_active = 0x%x\n"
						"\t\tconf_tmr = 0x%x\n",
						tsync->wlc->pub->unit, __FUNCTION__,
						ts_conf->period_ms,
						tsync->sync_active, conf_tmr));
				break;

			case BCMMSGBUF_D2H_CLOCK_CORRECTION_TAG:
				tsync->stats.tscorr_tag++;
			default:
				ASSERT(0);
				break;
		}

		WL_INFORM(("wl%d: %s: tlv_id = 0x%x tlv_len = 0x%x ts_tlv = 0x%x buflen = 0x%x\n",
				tsync->wlc->pub->unit, __FUNCTION__, ((_bcm_xtlv_t *)ts_tlv)->id,
				((_bcm_xtlv_t *)ts_tlv)->len, (uint32)ts_tlv, buflen));
	}

fail:
	WL_INFORM(("wl%d: %s: tlv_buf = 0x%x len = 0x%x fwclk_tag = 0x%x hclk_tag = 0x%x\n"
			"\t\tclksel_tag = 0x%x tsconf_tag = 0x%x tscorr_tag = 0x%x\n"
			"\t\tres = 0x%x\n",
			tsync->wlc->pub->unit, __FUNCTION__, (uint32)tlv_buf, buflen,
			tsync->stats.fwclk_tag, tsync->stats.hclk_tag, tsync->stats.clksel_tag,
			tsync->stats.tsconf_tag, tsync->stats.tscorr_tag, res));

	return res;
}

/*
 * Timesync state machine processing events from,
 *	Timer - Periodic, Watchdog
 *	Host - Timesync message
 *	Microcode - Timesync response
 */
static int
wlc_tsync_sm(wlc_tsync_t *tsync, ts_event_t event, void *buf, int buflen)
{
	ts_state_t save_state = tsync->state;
	int res = BCME_OK;

	switch (tsync->state) {
		case TS_SM_INIT:
			if (event == EVT_HOST_MSG) {
				res = wlc_tsync_initiate_delta(tsync);
				if ((res == BCME_OK) || (res == BCME_NOTUP)) {
					res = wlc_tsync_parse_tlv(tsync, buf, buflen);
					if (res == BCME_OK)
						tsync->state = TS_SM_START;
				} else if (res == BCME_BUSY) {
					if (buflen >= HS_MSG_SIZE)
						goto fail;

					memcpy(tsync->hs_msg, buf, buflen);
					tsync->hs_len = buflen;
				}
			} else if (event == EVT_UCODE_RSP) {
				res = wlc_tsync_compute_delta(tsync, buf, buflen);
				if (res == BCME_OK) {
					res = wlc_tsync_parse_tlv(tsync, tsync->hs_msg,
							tsync->hs_len);
					if (res == BCME_OK)
						tsync->state = TS_SM_START;
				}
			}

			break;

		case TS_SM_START:
			if (event == EVT_SYNC_REQ) {
				res = wlc_tsync_notify_ucode(tsync, buf, buflen);
				if (res == BCME_OK) {
					tsync->state = (wlc_tsync_active_clkid(tsync) >= CLK_TSF) ?
							tsync->state = TS_WAIT_UCODE : tsync->state;
				}

			} else if (event == EVT_HOST_MSG) {
				res = wlc_tsync_parse_tlv(tsync, buf, buflen);
				if (res == BCME_OK)
					tsync->state = TS_SM_START;

				wlc_tsync_configure_tmr(tsync, WDG_TMR, FALSE);
			} else if (event == EVT_WDOG_EXP) {
				wlc_tsync_configure_tmr(tsync, WDG_TMR, FALSE);
				wlc_tsync_configure_tmr(tsync, SYNC_TMR, FALSE);
				res = BCME_ERROR;

				goto fail;
			}
			break;

		case TS_WAIT_UCODE:
			if (event == EVT_UCODE_RSP) {
				res = wlc_tsync_frame_tlv(tsync, buf, buflen);
				if (res == BCME_OK)
					tsync->state = TS_SENT_HOST;

				wlc_tsync_configure_tmr(tsync, WDG_TMR, TRUE);
			} else if (event == EVT_HOST_MSG) {
				res = wlc_tsync_parse_tlv(tsync, buf, buflen);
				if (res == BCME_OK)
					tsync->state = TS_SM_START;

				wlc_tsync_configure_tmr(tsync, WDG_TMR, FALSE);
			} else if (event == EVT_WDOG_EXP) {
				wlc_tsync_configure_tmr(tsync, WDG_TMR, FALSE);
				wlc_tsync_configure_tmr(tsync, SYNC_TMR, FALSE);
				res = BCME_ERROR;

				goto fail;
			}
			break;

		case TS_SENT_HOST:
			if (event == EVT_HOST_MSG) {
				res = wlc_tsync_parse_tlv(tsync, buf, buflen);
				if (res == BCME_OK)
					tsync->state = TS_SM_START;

				wlc_tsync_configure_tmr(tsync, WDG_TMR, FALSE);
			} else if (event == EVT_SYNC_REQ) {
				res = wlc_tsync_notify_ucode(tsync, buf, buflen);
				if (res == BCME_OK) {
					tsync->state = (wlc_tsync_active_clkid(tsync) >= CLK_TSF) ?
							tsync->state = TS_WAIT_UCODE : tsync->state;
				}

			} else if (event == EVT_WDOG_EXP) {
				wlc_tsync_configure_tmr(tsync, WDG_TMR, FALSE);
				wlc_tsync_configure_tmr(tsync, SYNC_TMR, FALSE);
				res = BCME_ERROR;

				goto fail;
			}
			break;

		default:
			ASSERT(0);
			break;
	}

	tsync->stats.save_state = save_state;
	tsync->stats.sm_valid++;

	WL_INFORM(("wl%d: %s: prev_state = 0x%x, next_state = 0x%x sm_valid = 0x%x res = 0x%x\n",
			tsync->wlc->pub->unit, __FUNCTION__, save_state, tsync->state,
			tsync->stats.sm_valid, res));

	return res;

fail:
	WL_INFORM(("wl%d: %s: prev_state = 0x%x, next_state = 0x%x sm_valid = 0x%x res = 0x%x\n",
			tsync->wlc->pub->unit, __FUNCTION__, save_state, tsync->state,
			tsync->stats.sm_valid, res));

	OSL_SYS_HALT();
	return res;
}

/*
 * Configures error inject policy in the device for host validation
 */
static int
wlc_tsync_doiovar(void *context, uint32 actionid,
	void *params, uint plen, void *arg, uint alen, uint vsize, struct wlc_if *wlcif)
{
	wlc_tsync_t *tsync = (wlc_tsync_t *)context;
	int int_val = 0;
	int *ret_int_ptr;
	int res = BCME_OK;

	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	ret_int_ptr = (int32 *)arg;

	switch (actionid) {
		case IOV_SVAL(IOV_TSYNC_INJECTERR):
			if (int_val & ~((1 << ERR_MAX_VAL) - 1)) {
				res = BCME_UNSUPPORTED;
				break;
			}

			tsync->err_inject = int_val;
			break;

		case IOV_GVAL(IOV_TSYNC_INJECTERR):
			*ret_int_ptr = tsync->err_inject;
			break;

		default:
			res = BCME_UNSUPPORTED;
			break;
	}

	WL_INFORM(("wl%d: %s: actionid = 0x%x params = 0x%x params_len = 0x%x res = 0x%x\n",
			tsync->wlc->pub->unit, __FUNCTION__, actionid, int_val, plen, res));

	return res;
}

/*
 * Watchdog timer for tracking host response. On timer expiration, resets the device
 */
static void
wlc_tsync_wdg_tmr(void *context)
{
	wlc_tsync_t *tsync = (wlc_tsync_t *)context;

	tsync->stats.wdg_cnt++;
	wlc_tsync_sm(tsync, EVT_WDOG_EXP, NULL, 0);

	WL_INFORM(("wl%d: %s: context = 0x%x, event = 0x%x wdg_cnt = 0x%x\n",
			tsync->wlc->pub->unit, __FUNCTION__, (uint32)context,
			EVT_WDOG_EXP, tsync->stats.wdg_cnt));

	return;
}

/*
 * Periodicity timer for initiating timesync message to the host. Ensures GPIO
 * configuration is setup before timesync messages sent to the host
 */
static void
wlc_tsync_periodic_tmr(void *context)
{
	wlc_tsync_t *tsync = (wlc_tsync_t *)context;
	wlc_info_t *wlc = tsync->wlc;
	mac_buf_t mac_buf;
	int res = BCME_OK;

	memset(&mac_buf, 0, sizeof(mac_buf_t));

	if (wlc_tsync_active_clkid(tsync) >= CLK_TSF) {
		if (wlc->pub->up == 0) {
			res = BCME_NOTUP;
			goto end;
		}
	}

	if (wlc_tsync_active_clkid(tsync) < CLK_TSF) {
		wlc_tsync_validate_gci(tsync);
		si_gci_time_sync_gpio_enable(wlc->pub->sih,
			tsync->tsync_gpio, TRUE);
		OSL_DELAY(TS_GPIO_PULSE_WD);
		si_gci_time_sync_gpio_enable(wlc->pub->sih,
			tsync->tsync_gpio, FALSE);
	} else {
		/* Give GPIO control to ucode */
		si_gpiocontrol(wlc->pub->sih, 1 << tsync->tsync_gpio,
			1 << tsync->tsync_gpio, GPIO_HI_PRIORITY);
		mac_buf.gpio = 1 << tsync->tsync_gpio;
		mac_buf.pulse_wd = TS_GPIO_PULSE_WD;
	}

	tsync->snd_seq++;
	tsync->stats.sync_cnt++;
	tsync->stats.period_tmr = res;
	wlc_tsync_sm(tsync, EVT_SYNC_REQ, &mac_buf, sizeof(mac_buf_t));

end:
	WL_INFORM(("wl%d: %s: context = 0x%x, event = 0x%x tsync_cnt = 0x%x snd_seq = 0x%x\n"
			"\t\tpulse_wd = 0x%x gpio_bmap = 0x%x clk_active = 0x%x\n"
			"\t\tres = 0x%x\n",
			tsync->wlc->pub->unit, __FUNCTION__, (uint32)context,
			EVT_SYNC_REQ, tsync->stats.sync_cnt, tsync->snd_seq,
			mac_buf.pulse_wd, mac_buf.gpio, wlc_tsync_active_clkid(tsync), res));

	return;
}

/*
 * Registration of timesync module. Timers and packet pool allocation for managing
 * timesync messages
 */
wlc_tsync_t*
BCMATTACHFN(wlc_tsync_attach)(wlc_info_t *wlc)
{
	wlc_tsync_t *tsync = NULL;
	int num_pkts = TSPOOL_SIZE;

	if (!wlc) {
		WL_ERROR(("%s - null wlc\n", __FUNCTION__));
		goto fail;
	}

	if ((tsync = (wlc_tsync_t *)MALLOCZ(wlc->osh, sizeof(wlc_tsync_t)))
		== NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	tsync->wlc = wlc;
	tsync->state = TS_SM_INIT;
	tsync->wdg_period = DEF_WDG_PERIOD;

	wlc->pub->_tsync = 1;

	if (wlc_module_register(wlc->pub, wlc_tsync_iovars, "tsync", tsync, wlc_tsync_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if ((tsync->sync_tmr = wl_init_timer(wlc->wl, wlc_tsync_periodic_tmr, tsync,
		"tsync_event_timer")) == NULL) {
		WL_ERROR(("wl%d: %s: tsync_event_timer init failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if ((tsync->wdg_tmr = wl_init_timer(wlc->wl, wlc_tsync_wdg_tmr, tsync,
		"tsync_wdog_timer")) == NULL) {
		WL_ERROR(("wl%d: %s: tsync_wdog_timer init failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (pktpool_init(wlc->pub->osh, &tsync->ts_pool, &num_pkts,
		TSPOOL_PKTSIZE, FALSE, lbuf_basic) || (num_pkts < TSPOOL_SIZE)) {
			WL_ERROR(("wl%d: %s: pktpool_init failed tsync = 0x%x\n"
					"		 req = 0x%x\n, recv = 0x%x\n",
					wlc->pub->unit, __FUNCTION__, tsync,
					TSPOOL_SIZE, num_pkts));
			goto fail;
	}

	if ((tsync->tsync_gpio = si_gci_time_sync_gpio_init(wlc->pub->sih)) ==
			CC_GCI_GPIO_INVALID) {
		WL_ERROR(("wl%d: %s: tsync_gpio init failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	WL_INFORM(("wl%d: %s: tsync = 0x%x wlc = 0x%x\n",
			wlc->pub->unit, __FUNCTION__, (uint32)tsync, (uint32)wlc));

	return tsync;

fail:
	wlc_tsync_detach(tsync);
	return NULL;
}

/*
 * De-Registration of timesync module. Timers and packet pool de-allocation
 */
void
BCMATTACHFN(wlc_tsync_detach)(wlc_tsync_t *tsync)
{
	wlc_info_t *wlc;

	if (tsync == NULL) {
		return;
	}

	wlc = tsync->wlc;

	if (tsync->sync_tmr)
		wl_free_timer(wlc->wl, tsync->sync_tmr);

	if (tsync->wdg_tmr)
		wl_free_timer(wlc->wl, tsync->wdg_tmr);

	wlc_module_unregister(wlc->pub, "tsync", tsync);
	pktpool_deinit(wlc->pub->osh, &tsync->ts_pool);

	WL_INFORM(("wl%d: %s: tsync = 0x%x wlc = 0x%x\n",
			wlc->pub->unit, __FUNCTION__, (uint32)tsync, (uint32)wlc));

	MFREE(wlc->osh, tsync, sizeof(wlc_tsync_t));
}
