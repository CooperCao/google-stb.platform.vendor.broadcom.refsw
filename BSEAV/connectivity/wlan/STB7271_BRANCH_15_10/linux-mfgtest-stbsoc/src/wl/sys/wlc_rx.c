/*
 * wlc_rx.c
 *
 * Common receive datapath components
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
 *
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <d11_cfg.h>
#include <bcmwifi_channels.h>
#include <siutils.h>
#include <pcie_core.h>
#include <nicpci.h>
#include <bcmendian.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#ifdef PKTC
#include <proto/ethernet.h>
#endif
#if defined(PKTC) || defined(PKTC_TX_DONGLE)
#include <proto/802.3.h>
#endif
#include <proto/802.11e.h>
#include <proto/802.11ax.h>
#ifdef	BCMCCX
#include <proto/802.11_ccx.h>
#endif	/* BCMCCX */
#include <proto/bcmip.h>
#include <proto/wpa.h>
#include <proto/vlan.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <pcicfg.h>
#include <bcmsrom.h>
#if defined(WLTEST) || defined(BCMDBG_DUMP)
#include <bcmnvram.h>
#endif
#include <wlioctl.h>
#include <epivers.h>
#if defined(BCMSUP_PSK) || defined(BCMCCX) || defined(STA)
#include <proto/eapol.h>
#endif
#include <bcmwpa.h>
#ifdef BCMCCX
#include <bcmcrypto/ccx.h>
#endif /* BCMCCX */
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <hndpmu.h>
#include <hndd11.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_cca.h>
#include <wlc_interfere.h>
#include <wlc_keymgmt.h>
#include <wlc_bsscfg.h>
#include <wlc_mbss.h>
#include <wlc_channel.h>
#include <wlc.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#include <wlc_apps.h>
#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <phy_utils_api.h>
#include <phy_ana_api.h>
#include <phy_misc_api.h>
#include <phy_radio_api.h>
#include <phy_antdiv_api.h>
#include <phy_rssi_api.h>
#include <phy_noise_api.h>
#include <wlc_iocv_high.h>
#include <phy_high_api.h>
#include <wlc_bmac_iocv.h>
#include <wlc_antsel.h>
#include <wlc_led.h>
#include <wlc_frmutil.h>
#include <wlc_stf.h>
#include <wlc_rsdb.h>
#include <wlc_act_frame.h>
#ifdef WLNAR
#include <wlc_nar.h>
#endif
#if defined(SCB_BS_DATA)
#include <wlc_bs_data.h>
#endif /* SCB_BS_DATA */
#ifdef WLAMSDU
#include <wlc_amsdu.h>
#endif
#ifdef WLAMPDU
#include <wlc_ampdu.h>
#include <wlc_ampdu_rx.h>
#include <wlc_ampdu_cmn.h>
#endif
#ifdef WLTDLS
#include <wlc_tdls.h>
#endif
#ifdef WLDLS
#include <wlc_dls.h>
#endif
#ifdef L2_FILTER
#include <wlc_l2_filter.h>
#endif
#ifdef WLMCNX
#include <wlc_mcnx.h>
#include <wlc_tbtt.h>
#endif
#include <wlc_p2p.h>
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif
#include <wlc_scb_ratesel.h>
#ifdef WL_LPC
#include <wlc_scb_powersel.h>
#endif
#include <wlc_event.h>
#ifdef WOWL
#include <wlc_wowl.h>
#endif
#ifdef WOWLPF
#include <wlc_wowlpf.h>
#endif
#include <wlc_seq_cmds.h>
#ifdef WLOTA_EN
#include <wlc_ota_test.h>
#endif /* WLOTA_EN */
#ifdef WLDIAG
#include <wlc_diag.h>
#endif
#include <wl_export.h>
#include "d11ucode.h"
#if defined(BCMSUP_PSK) || defined(BCMCCX)
#include <wlc_sup.h>
#endif
#include <wlc_pmkid.h>
#if defined(BCMAUTH_PSK)
#include <wlc_auth.h>
#endif
#ifdef WET
#include <wlc_wet.h>
#endif
#ifdef WET_TUNNEL
#include <wlc_wet_tunnel.h>
#endif
#ifdef WMF
#include <wlc_wmf.h>
#endif
#ifdef PSTA
#include <wlc_psta.h>
#endif /* PSTA */
#if defined(BCMNVRAMW) || defined(WLTEST)
#include <bcmotp.h>
#endif
#ifdef BCMCCMP
#include <bcmcrypto/aes.h>
#endif
#include <wlc_rm.h>
#ifdef BCMCCX
#include <wlc_ccx.h>
#endif
#include "wlc_cac.h"
#include <wlc_ap.h>
#ifdef AP
#include <wlc_apcs.h>
#endif
#include <wlc_scan_utils.h>
#ifdef WL11K
#include <wlc_rrm.h>
#endif /* WL11K */
#ifdef WLWNM
#include <wlc_wnm.h>
#endif
#if defined(RWL_DONGLE) || defined(UART_REFLECTOR)
#include <rwl_shared.h>
#include <rwl_uart.h>
#endif /* RWL_DONGLE || UART_REFLECTOR */
#include <wlc_extlog.h>
#include <wlc_alloc.h>
#include <wlc_assoc.h>
#ifdef BCMASSERT_LOG
#include <bcm_assert_log.h>
#endif
#if defined(RWL_WIFI) || defined(WIFI_REFLECTOR)
#include <wlc_rwl.h>
#endif
#ifdef WLPFN
#include <wl_pfn.h>
#endif
#ifdef STA
#include <wlc_wpa.h>
#endif /* STA */
#include <wlc_lq.h>
#include <wlc_11h.h>
#include <wlc_tpc.h>
#include <wlc_csa.h>
#include <wlc_quiet.h>
#include <wlc_dfs.h>
#include <wlc_11d.h>
#include <wlc_cntry.h>
#include <bcm_mpool_pub.h>
#include <wlc_utils.h>
#include <wlc_hrt.h>
#include <wlc_prot.h>
#include <wlc_prot_g.h>
#define _inc_wlc_prot_n_preamble_	/* include static INLINE uint8 wlc_prot_n_preamble() */
#include <wlc_prot_n.h>
#if defined(WL_PROT_OBSS) && !defined(WL_PROT_OBSS_DISABLED)
#include <wlc_prot_obss.h>
#endif
#if defined(WL_OBSS_DYNBW) && !defined(WL_OBSS_DYNBW_DISABLED)
#include <wlc_obss_dynbw.h>
#endif
#include <wlc_11u.h>
#include <wlc_probresp.h>
#ifdef WLTOEHW
#include <wlc_tso.h>
#endif /* WLTOEHW */
#include <wlc_vht.h>
#include <wlc_txbf.h>
#if defined(BCMWAPI_WPI) || defined(BCMWAPI_WAI)
#include <wlc_wapi.h>
#include <bcmcrypto/sms4.h>
#endif
#include <wlc_pcb.h>
#include <wlc_txc.h>
#ifdef MFP
#include <wlc_mfp.h>
#endif
#include <wlc_macfltr.h>
#include <wlc_addrmatch.h>
#ifdef WL_RELMCAST
#include "wlc_relmcast.h"
#endif
#include <wlc_btcx.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_helper.h>
#include <wlc_ie_reg.h>
#include <wlc_akm_ie.h>
#include <wlc_ht.h>
#include <wlc_obss.h>
#ifdef ANQPO
#include <wl_anqpo.h>
#endif
#include <wlc_hs20.h>
#ifdef STA
#include <wlc_pm.h>
#endif /* STA */
#ifdef WLFBT
#include <wlc_fbt.h>
#endif
#if defined(BCMCCX) && defined(BCMINTSUP)
#include <wlc_sup_ccx.h>
#endif
#if defined(WL_OKC) || defined(WLRCC)
#include <wlc_okc.h>
#endif
#ifdef WLOLPC
#include <wlc_olpc_engine.h>
#endif /* OPEN LOOP POWER CAL */
#ifdef WL_STAPRIO
#include <wlc_staprio.h>
#endif /* WL_STAPRIO */
#include <wlc_monitor.h>
#include <proto/monitor.h>
#include <wlc_stamon.h>
#ifdef WDS
#include <wlc_wds.h>
#endif
#ifdef	WLAIBSS
#include <wlc_aibss.h>
#endif /* WLAIBSS */
#ifdef WLIPFO
#include <wlc_ipfo.h>
#endif
#if defined(WL_STATS)
#include <wlc_stats.h>
#endif /* WL_STATS */
#include <wlc_objregistry.h>
#include <wlc_tx.h>
#ifdef WL_PROXDETECT
#include <wlc_pdsvc.h>
#endif /* PROXIMITY DETECTION */
#ifdef BCMSPLITRX
#include <wlc_ampdu_rx.h>
#include <wlc_pktfetch.h>
#endif
#ifdef WL_TBOW
#include <wlc_tbow.h>
#endif
#include <wlc_utils.h>
#include <wlc_rx.h>
#include <wlc_dbg.h>
#ifdef WL_MODESW
#include <wlc_modesw.h>
#endif
#ifdef WL_NAN
#include <wlc_nan.h>
#endif
#ifdef WL_PWRSTATS
#include <wlc_pwrstats.h>
#endif
#include <wlc_smfs.h>
#if defined(GTKOE)
#include <wl_gtkrefresh.h>
#endif
#ifdef TRAFFIC_MGMT
#include <wlc_traffic_mgmt.h>
#endif
#ifdef WLMESH
#include <proto/802.11s.h>
#include <wlc_mesh.h>
#include <wlc_mesh_peer.h>
#include <wlc_mesh_route.h>
#endif
#include <wlc_pktc.h>
#ifdef PROP_TXSTATUS
#include <wlfc_proto.h>
#endif
#include <wlc_misc.h>
#include <event_trace.h>
#include <wlc_pspretend.h>
#include <wlc_frag.h>
#include <wlc_event_utils.h>
#include <wlc_perf_utils.h>
#ifdef EXT_STA
#include <wlc_ndis_iovar.h>
#endif
#ifdef WL_MU_RX
#include <wlc_murx.h>
#endif /* WL_MU_RX */
#include <wlc_iocv.h>
#ifdef WLC_SW_DIVERSITY
#include <wlc_swdiv.h>
#endif /* WLC_SW_DIVERSITY */
#include <wlc_log.h>
#ifdef WL11AX
#include <wlc_he.h>
#endif

#ifdef HEALTH_CHECK
#include <hnd_hchk.h>
#endif
#ifdef WL_LEAKY_AP_STATS
#include <wlc_leakyapstats.h>
#endif /* WL_LEAKYAP_STATS */

#ifndef DONGLEBUILD
static void wlc_monitor_amsdu(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, void *p);
#endif /* !DONGLEBUILD */
static void wlc_recv_mgmt_ctl(wlc_info_t *wlc, osl_t *osh, wlc_d11rxhdr_t *wrxh, void *p);

static void wlc_stamon_monitor(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, void *p,
	struct wlc_if *wlcif);
static void wlc_monitor(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, void *p, struct wlc_if *wlcif);
static uint16 wlc_recvfilter(wlc_info_t *wlc, wlc_bsscfg_t **pbsscfg, struct dot11_header *h,
	wlc_d11rxhdr_t *wrxh, struct scb **pscb, int body_len);

static void wlc_recv_mgmtact(wlc_info_t *wlc, struct scb *scb, struct dot11_management_header *hdr,
	uint8 *body, int body_len, wlc_d11rxhdr_t *wrxh, uint8 *plcp);
static void wlc_recv_process_prbreq(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_management_header *hdr, uint8 *body, int body_len);
static void wlc_recv_process_prbresp(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_management_header *hdr, uint8 *body, int body_len);

static void wlc_recv_process_beacon(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_management_header *hdr, uint8 *body, int body_len, bool* current_bss_bcn);
static void wlc_frameaction_public(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr,
	uint8 *body, int body_len, wlc_d11rxhdr_t *wrxh, uint32 rspec);
static void wlc_frameaction_vs(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len,
	wlc_d11rxhdr_t *wrxh, uint32 rspec);
static bool wlc_is_vsaction(uint8 *hdr, int len);
#ifdef WLMESH
static bool wlc_is_spaction(uint8 *hdr, int len);
#endif

/* ** STA-only routines ** */
#ifdef STA
/* power save and tbtt sync */
static void wlc_recv_PSpoll_resp(wlc_bsscfg_t *cfg, uint16 fc);
static void wlc_roam_process_beacon(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_management_header *hdr,
	uint8 *body, int bcn_len);

static void
wlc_process_target_bss_beacon(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
                              struct dot11_bcn_prb *bcn,
                              uint8 *body, int body_len,
                              wlc_d11rxhdr_t *wrxh, uint8 *plcp);
static void
wlc_reset_cfpstart(wlc_info_t *wlc, struct dot11_bcn_prb *bcn);
static void
wlc_check_for_retrograde_tsf(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, wlc_d11rxhdr_t *wrxh,
	uint8 *plcp, struct dot11_management_header *hdr, uint8 *body);

static uint32
wlc_tbtt_calc(wlc_info_t *wlc, bool short_preamble, ratespec_t rspec,
	struct dot11_bcn_prb *bcn, bool adopt);

static bool wlc_check_tbtt(wlc_bsscfg_t *cfg);

static bool
wlc_ibss_coalesce_allowed(wlc_info_t *wlc, wlc_bsscfg_t *cfg);

/* regular scan */
static void
wlc_tbtt_align(wlc_bsscfg_t *cfg, bool short_preamble, ratespec_t rspec, struct dot11_bcn_prb *bcn);

static uint8
wlc_bw_update_required(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	chanspec_t bss_chspec, bool bss_allow40);
#endif /* STA */

static void wlc_convert_restricted_chanspec(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh,
	struct dot11_management_header *hdr, uint8 *body, int bcn_len);

static int BCMFASTPATH
wlc_bss_sendup_post_filter(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct scb *scb,
	void *pkt, bool multi);

static int BCMFASTPATH
wlc_recvdata_sendpkt(wlc_info_t *wlc, void *p, wlc_if_t *wlcif);

static void
wlc_recv_bcn(wlc_info_t *wlc, osl_t *osh, wlc_bsscfg_t *bsscfg_current, wlc_bsscfg_t *bsscfg_target,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, int len, bool update_roam);

static bool
wlc_is_vsaction(uint8 *hdr, int len);

static void
wlc_ht_bcn_scb_upd(wlc_info_t *wlc, struct scb *scb,
	ht_cap_ie_t *ht_cap, ht_add_ie_t *ht_op, obss_params_t *ht_obss);

static int BCMFASTPATH
wlc_bss_sendup_pre_filter(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct scb *scb,
	void *pkt, bool multi);

#if defined(WLTEST)
static uint8 wlc_rxpkt_rate_count(wlc_info_t *wlc, ratespec_t rspec);
#endif

#define TBTT_ALIGN_LEEWAY_TU    10      /**< min leeway before first TBTT in TU (1024 us) */
#define TBTT_ALIGN_LEEWAY_TU_QT 2       /**< min leeway before first TBTT in TU (1024 us) */

/* hack to fool the compiler... */
#define DONOTHING() do {} while (0)

#ifdef WL_MIMOPS_CFG
static bool wlc_bcn_len_valid(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh,
	int bcn_len, bool *cur_bss_bcn);
#endif /* WL_MIMOPS_CFG */

#ifdef WL_RX_STALL
/* RX toss reason enums are declared in wlc.h
 * Changes in the wlc_rx_status_t enum should be accompanied by corresponding changes in the
 * below toss_strings structure.
 */
static const char * rx_sts_string[RX_STS_RSN_MAX] = {
	RX_STS_REASON_STRINGS
};
#endif /* WL_RX_STALL */

INLINE uint16*
wlc_get_mrxs(wlc_info_t *wlc, d11rxhdr_t *rxh)
{
#ifdef WL11AX
	if (D11REV_GE(wlc->pub->corerev, 80)) {
		return &rxh->ge80.mrxs;
	} else
#endif /* WL11AX */
	{	/* short receive header. first and intermediate frags */
		d11rxhdrshort_t *srxh = NULL;

		srxh = (d11rxhdrshort_t*) rxh;
		return &srxh->mrxs;
	}
}

/**
 * Process received data/ctrl/mgmt frames
 * Return TRUE if more frames need to be processed. FALSE otherwise.
 * Param 'bound' indicates max. # frames to process before break out.
 */
void BCMFASTPATH
wlc_recv(wlc_info_t *wlc, void *p)
{
	wlc_d11rxhdr_t *wrxh;
	d11rxhdr_t *rxh;
	struct dot11_header *h;
	osl_t *osh;
	uint16 fc;
	uint16 pad = 0;
	uint len;
	bool is_amsdu;
	bool full_status = TRUE;
	bool pad_present;
	uint16 *mrxs;
	wlc_rx_status_t toss_reason = WLC_RX_STS_TOSS_UNKNOWN;
	uint16 *prxs0;

	WL_TRACE(("wl%d: wlc_recv\n", wlc->pub->unit));

	osh = wlc->osh;

#if defined(PKTC) || defined(PKTC_DONGLE)
	/* Clear DA for pktc */
	PKTC_HDA_SET(wlc->pktc_info, NULL);
#endif

	/* frame starts with rxhdr */
	wrxh = (wlc_d11rxhdr_t *)PKTDATA(osh, p);
	rxh = &wrxh->rxhdr;

	/* strip off rxhdr */
	PKTPULL(osh, p, wlc->hwrxoff);

	if (RXS_SHORT_ENAB(wlc->pub->corerev) &&
	    (D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev, dma_flags) & RXS_SHORT_MASK)) {
		/* short rx status received */
		mrxs = wlc_get_mrxs(wlc, rxh);

		full_status = FALSE;
		pad_present = ((*mrxs & RXSS_PBPRES) != 0);
		/* short rx status is for AMSDU */
		is_amsdu = TRUE;
		if ((*mrxs & RXSS_AMSDU_MASK) == 0) {
			WL_AMSDU(("wl%d:%s Short Rx Status does not have AMSDU bit set. MRXS:%#x\n",
				wlc->pub->unit, __FUNCTION__, *mrxs));
#ifdef BCMDBG
			wlc_print_hdrs(wlc, "rxpkt hdr (invalid short rxs)", NULL, NULL, wrxh, 0);
			prhex("rxpkt body (invalid short rxs)", PKTDATA(osh, p), PKTLEN(osh, p));
#endif /* BCMDBG */
			ASSERT((*mrxs & RXSS_AMSDU_MASK) != 0);
		}
	} else {
		/* long rx status received */
		if (D11REV_LT(wlc->pub->corerev, 80)) {
			pad_present = ((D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev, RxStatus1) &
				RXS_PBPRES) != 0);
			is_amsdu = ((D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev, RxStatus2) &
				RXS_AMSDU_MASK) != 0);
		} else {
			mrxs = wlc_get_mrxs(wlc, rxh);
			pad_present = ((*mrxs & RXSS_PBPRES) != 0);
			is_amsdu = ((*mrxs & RXS_AMSDU_MASK) != 0);
		}
	}

	/* MAC inserts 2 pad bytes for a4 headers or QoS or A-MSDU subframes */
	if (pad_present) {
		/* When pad bit is present, padding is always 2 */
		pad = 2;
		if (PKTLEN(osh, p) < pad) {
			WLCNTINCR(wlc->pub->_cnt->rxrunt);
			WL_ERROR(("wl%d: %s: rcvd runt of len %d\n",
				wlc->pub->unit, __FUNCTION__, PKTLEN(osh, p)));
#if defined(BCMDBG)
			wlc_print_hdrs(wlc, "runt", NULL, NULL, wrxh, 0);
#endif
			goto toss_silently;
		}
		PKTPULL(osh, p, pad);
	}

	h = (struct dot11_header *)(PKTDATA(osh, p) + D11_PHY_HDR_LEN);
	len = PKTLEN(osh, p) + PKTFRAGUSEDLEN(osh, p);

	/* use rx header to verify that we can process the packet */
	prxs0 = D11RXHDR_ACCESS_REF(rxh, wlc->pub->corerev, PhyRxStatus_0);

	if (((wlc->band->phytype == PHY_TYPE_LCN20) || (wlc->band->phytype == PHY_TYPE_LCN)) &&
		(*prxs0 & PRXS0_PLCPFV))
		*prxs0 &= (~PRXS0_PLCPFV);

	if (full_status && (*prxs0 & (PRXS0_PLCPFV | PRXS0_PLCPHCF | PRXS0_UNSRATE))) {

		if (*prxs0 & (PRXS0_PLCPFV | PRXS0_PLCPHCF))
			WL_PRHDRS_MSG(("wl%d: Invalid PLCP header PhyRxStatus = 0x%04x\n",
				wlc->pub->unit, *prxs0));

		if (*prxs0 & PRXS0_UNSRATE)
			WL_PRHDRS_MSG(("wl%d: Unsupported rate PhyRxStatus = 0x%04x\n",
				wlc->pub->unit, *prxs0));

		WL_PRHDRS(wlc, "rxpkt hdr (invalid)", (uint8*)h, NULL, wrxh,
			len - D11_PHY_HDR_LEN);
		WL_PRPKT("rxpkt body (invalid)", PKTDATA(osh, p), len);

		goto toss_silently;
	}

	/* monitor mode. Send all runt/bad fcs/bad proto up as well */
	if (MONITOR_ENAB(wlc) &&
		(MONITOR_PROMISC_ENAB((wlc)->mon_info) ||
		(STAMON_ENAB(wlc->pub) && STA_MONITORING(wlc, &h->a2)))) {
#ifndef DONGLEBUILD
		if (is_amsdu) {
			/* wlc_monitor_amsdu() calls wlc_monitor() when a
			 * complete AMSDU frame is ready to be sent up
			 */
			wlc_monitor_amsdu(wlc, wrxh, p);
		} else
#endif /* !DONGLEBUILD */
		{
			wlc_monitor(wlc, wrxh, p, NULL);
		}

		if (!wlc->pub->associated) {
#ifdef DONGLEBUILD
			return;
#else
			goto toss_silently;
#endif
		}
	} else if (STAMON_ENAB(wlc->pub) && STA_MONITORING(wlc, &h->a2)) {
		/* At present we are only monitoring Non AMSDU packet
		 * TODO : modify the STA_MON handling for AMSDU packets
		 */
		if (!is_amsdu) {
			/* toss this frame after evaluating the stats */
			wlc_stamon_monitor(wlc, wrxh, p, NULL);
			goto toss;
		}
	} else {
		if (full_status)
			ASSERT((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev, RxStatus1) &
				RXS_FCSERR) == 0);
	}

	if (full_status && ((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev, RxStatus1) &
		RXS_FCSERR) == RXS_FCSERR))
		goto toss_silently;

	/* check received pkt has at least frame control field */
	if (len >= D11_PHY_HDR_LEN + sizeof(h->fc)) {
		fc = ltoh16(h->fc);
	} else {
		WLCNTINCR(wlc->pub->_cnt->rxrunt);
		goto toss_silently;
	}

	/* explicitly test bad src address to avoid sending bad deauth */
	if (!is_amsdu) {
		/* CTS and ACK CTL frames are w/o a2 */
		if (FC_TYPE(fc) == FC_TYPE_DATA || FC_TYPE(fc) == FC_TYPE_MNG) {
			if ((ETHER_ISNULLADDR(&h->a2) || ETHER_ISMULTI(&h->a2))) {
				WL_ERROR(("wl%d: %s: dropping a frame with invalid"
				          " src mac address" MACF"\n", wlc->pub->unit, __FUNCTION__,
				          ETHER_TO_MACF(h->a2)));
				WLCNTINCR(wlc->pub->_cnt->rxbadsrcmac);
				toss_reason = WLC_RX_STS_TOSS_INV_SRC_MAC;
				goto toss;
			}
			WLCNTINCR(wlc->pub->_cnt->rxfrag);
		}
#if defined(WLAMSDU)
		if (BCMSPLITRX_ENAB() && !PKTISRXFRAG(osh, p)) {
			/* Skip amsdu flush if pkt are non rxfrags */
			/* non rxfrag pkts come from fifo-1 and shouldnt */
			/* reset amsdu state of fifo-0 [RX Frag pkts ] */
		} else {
			wlc_amsdu_flush(wlc->ami);
		}
#endif
	}

#if defined(PKTC) || defined(PKTC_DONGLE)
	/* amsdu deagg'd already */
	/* All unchained amsdu frames should terminate here */
	if (WLPKTTAG(p)->flags & WLF_HWAMSDU) {
		void *amsdu_head = p;
		/* p is the head of an AMSDU chain. Find the last MPDU in the chain.
		 * The last MPDU has full receive status, which should be used to
		 * process all the MPDUs in the chain. Strip off rx header and pad
		 * for all non-first MPDUs in the chain.
		 */
		uint pad = 0;
		while (PKTNEXT(osh, p)) {
			p = PKTNEXT(osh, p);
			wrxh = (wlc_d11rxhdr_t*) PKTDATA(osh, p);
			if (RXS_SHORT_ENAB(wlc->pub->corerev) &&
				(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev,
				dma_flags) & RXS_SHORT_MASK)) {
				mrxs = wlc_get_mrxs(wlc, &wrxh->rxhdr);
				pad = ((*mrxs & RXSS_PBPRES) != 0) ? 2 : 0;
			} else {
				pad = ((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
					wlc->pub->corerev, RxStatus1) &
					RXS_PBPRES) != 0) ? 2 : 0;
			}
			PKTPULL(wlc->osh, p, wlc->hwrxoff + pad);
		}
		/* Call recvdata for the head of the AMSDU chain, but the receive status
		 * for the last MPDU in the chain.
		 */
		wlc_recvdata(wlc, osh, wrxh, amsdu_head);
		return;
	}
#endif  /* defined(PKTC) || defined(PKTC_DONGLE) */

	if (is_amsdu) {
#ifdef WLAMSDU
		if (wlc->_amsdu_rx) {
			/*
			 * check, qualify, chain MSDUs,
			 * calls wlc_recvdata() when AMSDU is completely received
			 * Restore rxh before recvamsdu call
			 */
			PKTPUSH(osh, p, wlc->hwrxoff + (pad_present ? 2 : 0));
			wlc_recvamsdu(wlc->ami, wrxh, p, &pad, FALSE);
		}
		else
#endif
		{
			goto toss;
		}

	} else if (FC_TYPE(fc) == FC_TYPE_DATA) {
#ifdef WL11N
#ifdef BCMDBG
		if (D11REV_LT(wlc->pub->corerev, 80)) {
			if (*prxs0 & PRXS0_RXANT_UPSUBBAND) {
				WL_PROTO(("wlc_recv: receive frame on 20U side band!\n"));
			}

			if (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
				wlc->pub->corerev, PhyRxStatus_3)) {
				WL_PROTO(("wlc_recv: nphy, mixedmode, MMPlcplength %d, rate %d\n",
				           D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev,
				           PhyRxStatus_3) & 0x0FFF,
				           (D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev,
				           PhyRxStatus_3) & 0xF000) >> 12));
			}
		}
#endif	/* BCMDBG */
#endif	/* WL11N */
		/*
		 * Call common receive, dispatch, and sendup code.
		 * The latter drops the perimeter lock and anything can happen!
		 */
		wlc_recvdata(wlc, osh, wrxh, p);
	} else if ((FC_TYPE(fc) == FC_TYPE_MNG) || (FC_TYPE(fc) == FC_TYPE_CTL)) {

		/* Complete the mapping of the remaining buffer before processing
		 * control or mgmt frames.
		 */
		PKTCTFMAP(osh, p);

		wlc_recv_mgmt_ctl(wlc, osh, wrxh, p);

	} else {
		WLCNTINCR(wlc->pub->_cnt->rxbadproto);
		toss_reason = WLC_RX_STS_TOSS_BAD_PROTO;
		goto toss;
	}

	return;

toss:
	wlc_log_unexpected_rx_frame_log_80211hdr(wlc, h, toss_reason);
#ifdef WL_RX_STALL
	wlc_rx_healthcheck_update_counters(wlc, AC_BE, NULL, *wlc->bsscfg, toss_reason, 1);
#endif
toss_silently:
	PKTFREE(osh, p, FALSE);
} /* wlc_recv */

void BCMFASTPATH
wlc_recvdata(wlc_info_t *wlc, osl_t *osh, wlc_d11rxhdr_t *wrxh, void *p)
{
	d11rxhdr_t *rxh = &wrxh->rxhdr;
	uchar *plcp;
	uint min_len;
	bool promisc_frame = FALSE;
	struct scb *scb = NULL;
	struct wlc_frminfo f;	/* frame info to be passed to intermediate functions */
	uint offset;
#if defined(BCMDBG_ERR) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
	wlc_pkttag_t *pkttag;
	ratespec_t rspec;
	bool tome;
	struct ether_addr *bssid = NULL;
	wlc_bsscfg_t *bsscfg = NULL;
	wlc_roam_t *roam;
	uint bandunit;
#ifdef WLCNTSCB
	uint32 totpktlen;
#endif
	bool in_bss;
	uint16 qoscontrol = 0;
	wlc_rx_status_t toss_reason = WLC_RX_STS_TOSS_UNKNOWN;

	int me;
#ifdef BCMSPLITRX
	bool protected = FALSE;
#endif
#if defined(WL11AC) && defined(WL_MU_RX) && defined(WLCNT)
	uint8 gid;
#endif

	(void)me;

	WL_TRACE(("wl%d: wlc_recvdata\n", wlc->pub->unit));

	bandunit = CHSPEC_BANDUNIT(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev, RxChan));

	/* since most of the fields in the struct frminfo is initialized
	 * through this function, it not necessary to bzero the frminfo for
	 * perf issue.  Initial each field as needed.
	 */
	f.key = NULL;
	f.prio = 0;
	f.ac = 0;
	f.ht = FALSE;
	f.apsd_eosp = FALSE;
	f.ismulti = FALSE;
	f.istdls = 0;
	f.p = p;
	f.rxh = rxh;
	f.wrxh = wrxh;

	min_len = D11_PHY_HDR_LEN + DOT11_A3_HDR_LEN + DOT11_FCS_LEN;

	/* check for runts */
	if ((uint)PKTLEN(osh, f.p) < min_len) {
		WL_ERROR(("wl%d: %s: runt frame\n", wlc->pub->unit, __FUNCTION__));
		WLCNTINCR(wlc->pub->_cnt->rxrunt);
		goto toss_silently;
	}
	/* Short rx status do not have phy rx status fields */
	if (D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev, dma_flags) & RXS_SHORT_MASK) {
		WL_ERROR(("wl%d: %s: Toss frame with short rx status\n",
			wlc->pub->unit, __FUNCTION__));
		ASSERT((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev,
			dma_flags) & RXS_SHORT_MASK) == 0);
		goto toss;
	}

	plcp = PKTDATA(osh, p);
	rspec = wlc_recv_compute_rspec(wlc->d11_info, wrxh, plcp);

	/* Save the rspec in pkttag */
	WLPKTTAG(p)->rspec = rspec;

#if defined(WLTEST)
	wlc_phy_pkteng_rxstats_update(WLC_PI(wlc),
		wlc_rxpkt_rate_count(wlc, rspec));
#endif 


	f.h = (struct dot11_header *)(plcp + D11_PHY_HDR_LEN);
	pkttag = WLPKTTAG(p);

	WL_PRHDRS(wlc, "rxpkt hdr (data)", (uint8*)f.h, NULL, wrxh,
	          PKTLEN(osh, f.p) - D11_PHY_HDR_LEN);
	WL_PRPKT("rxpkt body (data)", plcp, PKTLEN(osh, f.p));
	WL_APSTA_RX(("wl%d: %s: frame from %s\n",
		wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&f.h->a2, eabuf)));

	f.fc = ltoh16(f.h->fc);
#ifdef BCMSPLITRX
	protected = (f.fc & 0x4000) ? TRUE : FALSE;
#endif

	f.type = FC_TYPE(f.fc);
	f.subtype = (f.fc & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT;
	ASSERT(f.type == FC_TYPE_DATA);

	f.wds = ((f.fc & (FC_TODS | FC_FROMDS)) == (FC_TODS | FC_FROMDS));
	f.qos = (f.type == FC_TYPE_DATA && FC_SUBTYPE_ANY_QOS(f.subtype));
#ifdef WL11N
	f.ht = (((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev, PhyRxStatus_0) &
		PRXS_FT_MASK(wlc->pub->corerev)) == PRXS0_PREN) &&
		(f.fc & FC_ORDER) && FC_SUBTYPE_ANY_QOS(f.subtype));
#endif

	/* check for runts */
	if (f.wds)
		min_len += ETHER_ADDR_LEN;
	/* WME: account for QoS Control Field */
	if (f.qos)
		min_len += DOT11_QOS_LEN;
	if (f.ht)
		min_len += DOT11_HTC_LEN;
	if ((uint)PKTLEN(osh, f.p) < min_len) {
		WL_ERROR(("wl%d: %s: runt frame from %s\n", wlc->pub->unit, __FUNCTION__,
			bcm_ether_ntoa(&(f.h->a2), eabuf)));
		WLCNTINCR(wlc->pub->_cnt->rxrunt);
		goto toss_silently;
	}

#ifdef STA
#ifdef BCMCCX
	if (STA_ACTIVE(wlc) && CCX_ENAB(wlc->pub) && wlc->rm_info->rm_state->ccx->frame_active)
		/* radio measurement: frame report.  CCXv2 S36.15 and CCXv4 S57 */
		wlc_ccx_rm_frm(wlc->ccx, wrxh, f.h);
#endif
#endif /* STA */

	/* check for a broadcast/multicast A1 */
	f.ismulti = ETHER_ISMULTI(&(f.h->a1));

	f.pbody = (uchar*)f.h + DOT11_A3_HDR_LEN;
	if (f.wds)
		f.pbody += ETHER_ADDR_LEN;

	f.isamsdu = FALSE;
	if (f.qos) {
		qoscontrol = ltoh16_ua(f.pbody);
		f.isamsdu = (qoscontrol & QOS_AMSDU_MASK) != 0;
	}

#ifdef WLCNTSCB
	/* cache the total pkt len now */
	totpktlen = pkttotlen(osh, p);
#endif

	/* strip off PLCP header */
	PKTPULL(osh, f.p, D11_PHY_HDR_LEN);

	/* check if this frame is in our BSS */
	bsscfg = NULL;
	if (f.wds) {
		/* this is a WDS frame, and WDS frames are not part of a BSS */
		f.bssid_match = FALSE;
	}
	else {
		if ((f.fc & (FC_FROMDS | FC_TODS)) == 0) {
			bssid = &f.h->a3;
			scb = wlc_scbibssfindband(wlc, &f.h->a2, bandunit, &bsscfg);
		} else {
			if (f.fc & FC_TODS)
				bssid = &f.h->a1;
			else
				bssid = &f.h->a2;

#ifdef PSTA
			if (PSTA_ENAB(wlc->pub) && !f.ismulti)
				bsscfg = wlc_bsscfg_find_by_hwaddr_bssid(wlc, &f.h->a1, bssid);
#endif
		}

		/* broadcast or multicast frames */
		if (bsscfg == NULL)
			bsscfg = wlc_bsscfg_find_by_bssid(wlc, bssid);

		f.bssid_match = bsscfg != NULL;
	}

	in_bss = bsscfg != NULL;
	/* is this frame sent to one of our addresses? */
	if (!f.ismulti && wlc_bsscfg_find_by_hwaddr(wlc, &f.h->a1) != NULL)
		tome = TRUE;
	else
		tome = FALSE;


	/* strip off FCS, A-MSDU has FCS on the tail buffer */
	/* For split rx case, last 4 bytes are in host and not in TCM */
	/* So adjust length of frag and not lcl pkt */
	if (!f.isamsdu) {
		PKTFRAG_TRIM_TAILBYTES(osh, f.p, DOT11_FCS_LEN, TAIL_BYTES_TYPE_FCS);
	} else {
		void *pt = pktlast(osh, f.p);
		PKTFRAG_TRIM_TAILBYTES(osh, pt, DOT11_FCS_LEN, TAIL_BYTES_TYPE_FCS);
	}

	if (MONITOR_ENAB(wlc) || PROMISC_ENAB(wlc->pub)) {

		/* we are not participating in a BSS */
		if (!wlc->pub->associated)
			in_bss = FALSE;

		/* determine if frame is for us or received only due to promisc setting */
		promisc_frame = TRUE;


		/* A1 is the destination or receiver address for all FromDS/ToDS combination,
		 * so the unicast check is done on A1.
		 * Multicast/Broadcast is only allowed
		 *     1) when ToDS==0 and the STA is participating in a BSS or IBSS,
		 *  OR 2) both ToDS and FromDS (a WDS multicast)
		 */
		if (/* unicast match */
		    tome ||
		    /* multicast in BSS (from AP) or in IBSS */
		    (!(f.fc & FC_TODS) && f.ismulti && in_bss) ||
		    /* multicast WDS */
		    (f.wds && f.ismulti)) {
			promisc_frame = FALSE;
		}
		else if (PROMISC_ENAB(wlc->pub)) {
			/* pass up frames from inside our BSS */
			if (!in_bss) {
				toss_reason = WLC_RX_STS_TOSS_NOT_IN_BSS;
				goto toss;
			}
			/* ensure promisc frames are tossed for AP */
			if (BSSCFG_AP(bsscfg)) {
				toss_reason = WLC_RX_STS_TOSS_AP_BSSCFG;
				goto toss;
			}
		}
		else {
			/* In monitor mode (sniffing while operating as a normal interface),
			 * don't continue processing packets that aren't for us
			 */
			ASSERT(MONITOR_ENAB(wlc));
			toss_reason = WLC_RX_STS_TOSS_PROMISC_OTHER;
			goto toss;
		}

		/* These frames survive above filters:
		 * unicast for us (!promisc_frame)
		 * - in all BSS/IBSS
		 * - WDS
		 * unicast but not for us (promisc_frame)
		 * - in our BSS/IBSS
		 * multicast (!promisc_frame)
		 * - in our BSS/IBSS
		 * - WDS
		 */
	}
	else if (!f.ismulti && !tome) {
		/* a race window exists while changing the RX MAC addr match registers where
		 * we may receive a fame that does not have our MAC address
		 */
		toss_reason = WLC_RX_STS_TOSS_MAC_ADDR_RACE;
		goto toss;
	}
	else {
		/* These frames survive PSM filters:
		 * unicast for us (!promisc_frame)
		 * - in all BSS/IBSS
		 * - WDS
		 * multicast (!promisc_frame)
		 * - in our BSS/IBSS
		 * - WDS
		 */
	}

	/* get rid of non-WDS frame that is not in our BSS/IBSS */
	if (tome && !f.wds &&
	    !in_bss) {
		goto toss_silently;
	}

#ifdef EXT_STA
	/* Take care of packet filtering here */
	if (WLEXTSTA_ENAB(wlc->pub) && bsscfg != NULL) {
		if (!wlc_packet_filter(wlc, bsscfg, &f)) {
			goto toss;
		}
	}
#endif /* EXT_STA */

	/* account for QoS Control Field */
	if (f.qos) {
		if (f.isamsdu) {
			if (BSSCFG_SAFEMODE(bsscfg)) {
				WL_ERROR(("%s(): safe mode no AMSDU support!\n", __FUNCTION__));
				toss_reason = WLC_RX_STS_TOSS_SAFE_MODE_AMSDU;
				goto toss;
			}
#ifdef WLAMSDU
			if (wlc->_amsdu_rx) {
				WLPKTTAG(f.p)->flags |= WLF_AMSDU;
				WL_AMSDU(("%s: is A-MSDU frame, rspec 0x%x\n",
					__FUNCTION__, rspec));
			} else
#endif
			{
				WL_ERROR(("%s: amsdu is not enabled, toss\n", __FUNCTION__));
				toss_reason = WLC_RX_STS_TOSS_AMSDU_DISABLED;
				goto toss;
			}
		}

		f.prio = (uint8)QOS_PRIO(qoscontrol);
		f.ac = WME_PRIO2AC(f.prio);
		f.apsd_eosp = QOS_EOSP(qoscontrol);
		f.pbody += DOT11_QOS_LEN;
	}

	/* assign f.len now */
	f.len = PKTLEN(osh, f.p) + PKTFRAGUSEDLEN(osh, f.p);
	offset = (uint)(f.pbody - (uint8*)PKTDATA(osh, f.p));
	f.body_len = f.len - offset;

	/* A-MSDU may be chained */
	f.totlen = pkttotlen(osh, p) - offset;

	/* save seq in host endian */
	f.seq = ltoh16(f.h->seq);

	/* 802.11-specific rx processing... */

	/* For reference, DS bits on Data frames and addresses:
	 *
	 *         ToDS FromDS   a1    a2    a3    a4
	 *
	 * IBSS      0      0    DA    SA   BSSID  --
	 * To STA    0      1    DA   BSSID  SA    --
	 * To AP     1      0   BSSID  SA    DA    --
	 * WDS       1      1    RA    TA    DA    SA
	 *
	 * For A-MSDU frame
	 *         ToDS FromDS   a1    a2    a3    a4
	 *
	 * IBSS      0      0    DA    SA   BSSID  --
	 * To STA    0      1    DA   BSSID BSSID  --
	 * To AP     1      0   BSSID  SA   BSSID  --
	 * WDS       1      1    RA    TA   BSSID  BSSID
	 */

	/* state based filtering */
	if (!promisc_frame && wlc_recvfilter(wlc, &bsscfg, f.h, wrxh, &scb, f.len))
		goto toss_nolog;

	/* create scb for class 1 IBSS data frame for which recvfilter() won't */
	if (!promisc_frame && (f.fc & (FC_FROMDS | FC_TODS)) == 0) {
		if (bsscfg == NULL) {
			WL_INFORM(("wl%d: %s: drop frame received from %s\n",
			           wlc->pub->unit, __FUNCTION__,
			           bcm_ether_ntoa(&f.h->a2, eabuf)));
			toss_reason = WLC_RX_STS_TOSS_DROP_FRAME;
			goto toss;
		}
		if (BSSCFG_SLOTTED_BSS(bsscfg)) {
			scb = wlc_scbfind_dualband(wlc, bsscfg, &f.h->a2);
#ifdef WLRSDB
			if (RSDB_ACTIVE(wlc->pub)) {
				if (scb) {
						bsscfg = scb->bsscfg;
						wlc = bsscfg->wlc;
				}
			}
#endif /* WLRSDB */
			if (!scb) {
				goto toss_silently;
			}
		} else {
			scb = wlc_scbfindband(wlc, bsscfg, &f.h->a2, bandunit);
			if (scb == NULL && NBANDS(wlc) > 1) {
				uint otherbandunit =
				        (bandunit == BAND_2G_INDEX) ? BAND_5G_INDEX: BAND_2G_INDEX;
				scb = wlc_scbfindband(wlc, bsscfg, &f.h->a2, otherbandunit);
			}
		}
#ifdef WLTDLS
		/*
		"If ToDS and FromDS are both 0, and if A3 is the BSSID of
		the AP to which the receiving STA is currently associated, and the reeiving STA
		does not have a valid TDLS link with the transmitting STA, the receiving STA
		shall send a TDLS Setup request through the AP to the transmitting STA."
		*/
		if (!scb &&
			/* Check TDLS Link only in INFRA network */
			 bsscfg->BSS) {
			int idx;
			wlc_bsscfg_t *cfg;
			int auto_op;

			FOREACH_AS_STA(wlc, idx, cfg) {
				if (!(BSS_TDLS_ENAB(wlc, cfg)) &&
					!bcmp(&f.h->a3, &cfg->current_bss->BSSID, ETHER_ADDR_LEN) &&
					/* Confirming INFRA network */
					cfg->BSS) {
					if (TDLS_ENAB(wlc->pub) && wlc_tdls_isup(wlc->tdls)) {
						tdls_iovar_t setup_info;

						memset(&setup_info, 0, sizeof(tdls_iovar_t));
						bcopy(&(f.h->a2), &setup_info.ea, ETHER_ADDR_LEN);
						setup_info.mode = TDLS_MANUAL_EP_CREATE;
						WL_TDLS(("wl%d:%s(): cannot find TDLS peer %s"
							", try TDLS setup with peer.\n",
							wlc->pub->unit, __FUNCTION__,
							bcm_ether_ntoa(&(f.h->a2), eabuf)));
						wlc_iovar_op(wlc, "tdls_auto_op", NULL, 0,
							&auto_op, sizeof(int), IOV_GET, cfg->wlcif);

						if (auto_op)
						    wlc_iovar_op(wlc, "tdls_endpoint", NULL, 0,
						    &setup_info, sizeof(tdls_iovar_t),
						    IOV_SET, cfg->wlcif);
					} else {
					/*
					In case TDLS just got disabled by wl cmd so that
					the TDLS link which burst data packets has been
					transmitted on just became disconnected,
					the x-peer might send a few more data packets
					with the TDLS link. Then just toss the packets
					in order not to create the SCB for the link.
					*/
						WL_ERROR(("wl%d: %s: TDLS packet on"
						" the disconnected link.\n",
						wlc->pub->unit, __FUNCTION__));
					}
					WLCNTINCR(wlc->pub->_cnt->rxnoscb);
					toss_reason = WLC_RX_STS_TOSS_NO_SCB;
					goto toss;
				}
			}
		}
#endif /* WLTDLS */

		if (scb == NULL) {
			if ((scb = wlc_scblookupband(wlc, bsscfg, &f.h->a2, bandunit)) == NULL) {
				WL_ERROR(("wl%d: %s: out of scbs for IBSS data\n",
				          wlc->pub->unit, __FUNCTION__));
				WLCNTINCR(wlc->pub->_cnt->rxnoscb);
			toss_reason = WLC_RX_STS_TOSS_NO_SCB;
				goto toss;
			}
		}
#if defined(WLTDLS)
		f.istdls = BSSCFG_IS_TDLS(bsscfg);
		if (TDLS_ENAB(wlc->pub) && wlc_tdls_isup(wlc->tdls)) {
			if (f.istdls) {
			    if (!in_bss || wlc_tdls_recvfilter(wlc->tdls, bsscfg)) {
					WL_ERROR(("wl%d: %s(): drop invalid TDLS packet.\n",
						wlc->pub->unit, __FUNCTION__));
					goto toss;
				} else {
					/* Kick Start PM2 mechanism for TDLS */
					wlc_pm2_ret_upd_last_wake_time(SCB_BSSCFG(scb),
						&f.wrxh->tsf_l);
					wlc_tdls_pm_timer_action(wlc->tdls, bsscfg);
			    }
			}
		}
#endif /* WLTDLS */
#ifdef PSPRETEND
		/* This IBSS peer is alive. Get out of PS PRETEND */
		if (BSSCFG_IBSS(bsscfg) && SCB_PS_PRETEND_THRESHOLD(scb) &&
			SCB_PS(scb))
			wlc_apps_process_ps_switch(wlc, &scb->ea, PS_SWITCH_OFF);
#endif /* PSPRETEND */
	}

	/* filter out bad frames with invalid toDS/fromDS combinations in promisc mode */
	if (promisc_frame &&
	    /* rx IBSS data frame but our bsscfg is BSS */
	    ((((f.fc & (FC_FROMDS | FC_TODS)) == 0) && bsscfg->BSS) ||
	     /* rx BSS data frame but our bsscfg is IBSS */
	     (((f.fc & (FC_FROMDS | FC_TODS)) != 0) && !f.wds && !bsscfg->BSS))) {
		WL_ERROR(("wl%d: wlc_recvdata: bad DS from %s\n", wlc->pub->unit,
		          bcm_ether_ntoa(&(f.h->a2), eabuf)));
		WLCNTINCR(wlc->pub->_cnt->rxbadds);
		toss_reason = WLC_RX_STS_TOSS_BAD_DS;
		goto toss;
	}

	/* create scb for class 3 BSS and class 1 IBSS data frames
	 * for other nodes received in promisc mode
	 */
	if (promisc_frame) {
		if ((scb = wlc_scblookupband(wlc, bsscfg, &f.h->a2, bandunit)) == NULL) {
			WL_INFORM(("wl%d: %s: out of scbs for promisc data\n",
			           wlc->pub->unit, __FUNCTION__));
			toss_reason = WLC_RX_STS_TOSS_NO_SCB;
			goto toss;
		}
	}
	else {
		ASSERT(scb != NULL);
		if (!bsscfg)
			bsscfg = scb->bsscfg;
	}

	/* set pkttag to simplify scb lookup */
	WLPKTTAGSCBSET(f.p, scb);

	/* Update scb timestamp. Exculde b/mcast because of keepalive */
	if (tome)
		scb->used = wlc->pub->now;

	ASSERT(bsscfg != NULL);
	ASSERT(bsscfg == scb->bsscfg);

	roam = (BSSCFG_PSTA(bsscfg) ? wlc_bsscfg_primary(wlc)->roam : bsscfg->roam);
	BCM_REFERENCE(roam);

	/* toss frames with invalid DS bits */
	if (!promisc_frame && !f.wds &&
	     /* multicast frame to AP */
	    ((f.ismulti && (f.fc & FC_TODS) != 0) ||
	     /* frame w/o ToDS to AP */
	     (BSSCFG_AP(bsscfg) && (f.fc & FC_TODS) == 0) ||
	     /* frame w/o FromDS to infra STA or with FromDS and/or ToDS to IBSS STA */
		(BSSCFG_STA(bsscfg) &&
		(f.fc & (FC_TODS | FC_FROMDS)) != ((bsscfg->BSS ||
		BSSCFG_MESH(bsscfg)) ? FC_FROMDS : 0)))) {
		WL_ERROR(("wl%d: %s: bad DS from %s (%s %s%s %s %s)\n",
		          wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&f.h->a2, eabuf),
		          f.ismulti ? "MC" : "UC",
		          (f.fc & FC_FROMDS) ? "FromDS" : "",
		          (f.fc & FC_TODS) ? "ToDS" : "",
		          BSSCFG_AP(bsscfg) ? "AP" : "STA",
		          bsscfg->BSS ? "BSS" : "IBSS"));
		WLCNTINCR(wlc->pub->_cnt->rxbadds);
		toss_reason = WLC_RX_STS_TOSS_BAD_DS;
		goto toss;
	}

#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wlc->pub)) {
		pkttag->rxchannel = (uint8)CHSPEC_CHANNEL(D11RXHDR_ACCESS_VAL(rxh,
			wlc->pub->corerev, RxChan));
	}
#endif /* EXT_STA */

	/* update info based on rx of any new traffic */

#if defined(WL_RELMCAST)
	/* This is need for fast rssi data collection on all scbs */
	if (RMC_ENAB(wlc->pub)) {
		wlc_rmc_mgmtctl_rssi_update(wlc->rmc, wrxh, scb, TRUE);
	}
#endif

	/* collect per-scb rssi samples */
	if (WLC_RX_LQ_SAMP_ENAB(scb)) {
		rx_lq_samp_t lq_samp;

		wlc_lq_sample(wlc, bsscfg, scb, wrxh, tome, &lq_samp);

		pkttag->pktinfo.misc.rssi = lq_samp.rssi;
		pkttag->pktinfo.misc.snr = lq_samp.snr;
	}

#ifdef WLTDLS
	if (BSS_TDLS_ENAB(wlc, bsscfg)) {
		WL_TDLS(("wl%d.%d:%s(): TDLS recv data pkt from %s.\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__,
			bcm_ether_ntoa(&(f.h->a2), eabuf)));
		if (BSS_TDLS_BUFFER_STA(bsscfg)) {
			wlc_apps_process_ps_switch(wlc, &(f.h->a2),
				(f.fc & FC_PM) ? PS_SWITCH_PMQ_ENTRY : PS_SWITCH_OFF);
		}
	}
#endif
	if (f.wds && !BSSCFG_MESH(bsscfg)) {
		/* the packet should be from one of our WDS partners */
		ASSERT(SCB_A4_DATA(scb));
		f.WPA_auth = bsscfg->WPA_auth;
	}
	else if (BSSCFG_AP(bsscfg) || BSS_TDLS_BUFFER_STA(bsscfg)) {
		/* If we are an AP only the packet should be from one
		 * of our clients even if we are in promiscuous mode.
		 */
		ASSERT((BSSCFG_AP(bsscfg) && SCB_ASSOCIATED(scb)) || BSS_TDLS_ENAB(wlc, bsscfg));
		/*
		 * use per scb WPA_auth to handle 802.1x frames differently
		 * in WPA/802.1x mixed mode when having a pairwise key:
		 *  - accept unencrypted 802.1x frames in plain 802.1x mode
		 *  - drop unencrypted 802.1x frames in WPA mode
		 */
		f.WPA_auth = scb->WPA_auth;
		/*
		 * Trigger Automatic Power-Save Delivery as early as possible.
		 * WMM/APSD: x.y.z: Do not trigger on the frame that transitions
		 * to power save mode.
		 */
		if (SCB_WME(scb) && f.qos &&
		    ((f.seq & FRAGNUM_MASK) == 0) &&
		    SCB_PS(scb) && !SCB_ISMULTI(scb) &&
		    (f.fc & FC_PM) && (scb->flags & SCB_RECV_PM) &&
		    AC_BITMAP_TST(scb->apsd.ac_trig, f.ac)) {
			if (BSS_TDLS_BUFFER_STA(bsscfg) && f.apsd_eosp &&
			    !wlc_apps_apsd_ac_buffer_status(wlc, scb)) {
				WL_INFORM(("wl%d.%d:%s(): eosp=1, skip wlc_apps_apsd_trigger().\n",
					wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
			} else
#ifdef WLTDLS
			/* when TDLS is doing PTI hand shake do not handle any data packet
			 * as a trigger to send the buffered frame
			 */
			if (TDLS_ENAB(wlc->pub) && BSSCFG_IS_TDLS(bsscfg)) {
				if (!wlc_tdls_wait_for_pti_resp(wlc->tdls, scb)) {
					WL_TRACE(("wl%d.%d:%s(): call wlc_apps_apsd_trigger().\n",
						wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
						__FUNCTION__));
					wlc_apps_apsd_trigger(wlc, scb, f.ac);
				}
			} else
#endif /* WLTDLS */
			{
				WL_TRACE(("wl%d.%d:%s(): call wlc_apps_apsd_trigger().\n",
					wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
				wlc_apps_apsd_trigger(wlc, scb, f.ac);
			}
		}
		else {
			WL_TRACE(("wl%d.%d:%s(): f.fc=%x, scb->flags=0x%x, scb->apsd.ac_trig=0x%x"
				"f.ac=%d\n", wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__,
				f.fc, scb->flags, scb->apsd.ac_trig, f.ac));
		}

		/*
		 * Remember the PM bit from the last data frame received so
		 * power save transitions can be detected synchronously.
		 */
		if (f.fc & FC_PM)
			scb->flags |= SCB_RECV_PM;
		else
			scb->flags &= ~SCB_RECV_PM;
	} else {
		if (bsscfg->BSS && !promisc_frame) {
			/* If we are a BSS STA, the packet should be from the AP
			 * or TDLS peer unless we are in promiscuous mode.
			 */
			ASSERT(SCB_ASSOCIATED(scb) || f.istdls);
		}
		f.WPA_auth = scb->WPA_auth;
		if (BSSCFG_IBSS(bsscfg) && (bsscfg->WPA_auth & WPA_AUTH_NONE))
			f.WPA_auth = bsscfg->WPA_auth;
	}

#ifdef STA
	if (BSSCFG_STA(bsscfg) && bsscfg->BSS) {
		/* This pkt must have come from the AP so we know it is up */

		/* Adjust time_since_bcn only for directed traffic to us since
		 * only directed traffic indicates the AP considers us associated.
		 * We want to handle the case of an AP rebooting by "roaming" to
		 * the AP, but we also want to also detect the case of going out
		 * then in range of an AP.
		 * If our rebooted old AP sends multicast traffic to other newly
		 * associated STAs, we do not want to stop the roam mechanism.
		 * But if we get directed traffic from our old AP since we just
		 * moved back into range, we want to stop the roam mechanism.
		 */
		if (!f.ismulti && !promisc_frame) {
			roam->bcn_interval_cnt = 0;
			if (roam->time_since_bcn != 0) {
#ifdef EVENT_LOG_COMPILE
				EVENT_LOG(EVENT_LOG_TAG_BEACON_LOG, "Beacon miss reset by RX");
#endif
				roam->time_since_bcn = 0;
			}
		}
	}
#endif /* STA */

#ifdef WL_EXCESS_PMWAKE
	wlc->excess_pmwake->ca_txrxpkts++;
#endif /* WL_EXCESS_PMWAKE */

	/* multicast address filtering */
	if (f.ismulti) {
		if (BSSCFG_AP(bsscfg)) {
			/* APs do not accept data traffic with a multicast a1 address */
			toss_reason = WLC_RX_STS_TOSS_MCAST_AP;
			goto toss;
		}

		/* toss multicast frames we sent forwarded back to us by the AP */
		if (bsscfg->BSS &&
		    bcmp((char*)&(f.h->a3), (char*)&bsscfg->cur_etheraddr, ETHER_ADDR_LEN) == 0) {
			goto toss_nolog;
		}

#ifdef PSTA
		if (PSTA_ENAB(wlc->pub)) {
			/* toss bcast/mcast frames that were forwarded back from UAP */
			if (wlc_psta_find(wlc->psta, (uint8 *)&f.h->a3) != NULL) {
				WL_PSTA(("wl%d: Tossing mcast frames sent by us\n",
				         wlc->pub->unit));
				goto toss;
			}
		}
#endif /* PSTA */

		/* filter with multicast list */
		if (!ETHER_ISBCAST(&(f.h->a1)) &&
			!bsscfg->allmulti && wlc_bsscfg_mcastfilter(bsscfg, &(f.h->a1)))
			goto toss_nolog;

		/* DWDS APs send 3 addr and 4 address multicast frames.
		 * if bsscfg is sta and dwds then drop all 3 address multicast so we dont get dups
		 */
		if (BSSCFG_STA(bsscfg) && SCB_DWDS(scb) && !f.wds) {
			toss_reason = WLC_RX_STS_TOSS_STA_DWDS;
			goto toss;
		}

#if defined(WL_RELMCAST)
		/* Check the RMC multi-cast addresses and the seq number     */
		/* drop it if seq == prev seq number                         */
		/* update seq number                                         */
		if (RMC_SUPPORT(wlc->pub)) {
			if (wlc_rmc_verify_dup_pkt(wlc, &f) == TRUE) {
				goto toss_nolog;
			}
		}
#endif
	}

	/* Now that the frame is accepted, update some rx packet related data */
	pkttag->rspec = (uint32)rspec;
	pkttag->seq = f.seq;
#ifdef WLFCTS
	if (WLFCTS_ENAB(wlc->pub) && (wlc->wlfc_flags & WLFC_FLAGS_PKT_STAMP_SIGNALS)) {
		if (f.fc & FC_RETRY)
			pkttag->rspec |= WLFC_TSFLAGS_RX_RETRY;
		if (bsscfg->pm->PMenabled || bsscfg->pm->PMpending)
			pkttag->rspec |= WLFC_TSFLAGS_PM_ENABLED;
		pkttag->shared.rx_tstamp = wlc_recover_tsf32(wlc, wrxh);
	}
#endif /* WLFCTS */

	/* Update per SCB receive stats */
	/* check a1 only for STA, keep original AP checking for a3 */
	if (BSSCFG_AP(bsscfg) ? ETHER_ISMULTI(&(f.h->a3)) : f.ismulti) {
		WLCNTSCBINCR(scb->scb_stats.rx_mcast_pkts);
		WLCNTSCBADD(scb->scb_stats.rx_mcast_bytes, totpktlen);
	} else {
		WLCNTSCBINCR(scb->scb_stats.rx_ucast_pkts);
		WLCNTSCBADD(scb->scb_stats.rx_ucast_bytes, totpktlen);
#ifdef WL_MIMOPS_CFG
		if (WLC_MIMOPS_ENAB(wlc->pub)) {
			wlc_mimo_ps_cfg_t *mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);
			if (WLC_STF_LEARNING_ENABLED(mimo_ps_cfg))
				wlc_stf_update_mimo_ps_cfg_data(bsscfg, rspec);
		}
#endif
	}

#ifdef WL11K
	wlc_rrm_stat_bw_counter(wlc, scb, FALSE);
#endif

#ifdef WL11N
	/* skip non-first MPDU in an A-MPDU */
	if (!f.ismulti && (plcp[0] | plcp[1] | plcp[2])) {
		wlc_scb_ratesel_upd_rxstats(wlc->wrsi, pkttag->rspec,
			D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev, RxStatus2));
		WLCNTSCBSET(scb->scb_stats.rx_rate, rspec);
	}
#else
	WLCNTSCB_COND_SET(!f.ismulti, scb->scb_stats.rx_rate, rspec);
#endif /* WL11N */

#if defined(WLCNT)
	if (PLCP3_ISSGI(plcp[3]))
		WLCNTINCR(wlc->pub->_cnt->rxmpdu_sgi);
	if (PLCP3_ISSTBC(plcp[3]))
		WLCNTINCR(wlc->pub->_cnt->rxmpdu_stbc);

#if defined(WL11AC) && defined(WL_MU_RX) && defined(WLCNT)
	if (MU_RX_ENAB(wlc)) {
		if (!f.ismulti && (D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev, PhyRxStatus_0) &
			PRXS_FT_MASK(wlc->pub->corerev)) == PRXS0_STDN) {
			/* In case of AMPDU, since PLCP is valid only on first MPDU in an AMPDU,
			 * a MU-PLCP marks the start of an AMPDU, and end is determined my another
			 * SU/MU PLCP. Till this duration we count all MPDUs recieved as MU MPDUs
			 */
			if (plcp[0] | plcp[1] | plcp[2]) {
				gid = wlc_vht_get_gid(plcp);
				if ((gid > VHT_SIGA1_GID_TO_AP) &&
					(gid < VHT_SIGA1_GID_NOT_TO_AP)) {
					WLC_UPDATE_MURX_INPROG(wlc, TRUE);
				} else {
					WLC_UPDATE_MURX_INPROG(wlc, FALSE);
				}

			}

			if (WLC_GET_MURX_INPROG(wlc))
				WLCNTINCR(wlc->pub->_cnt->rxmpdu_mu);
		} else {
			WLC_UPDATE_MURX_INPROG(wlc, FALSE);
		}
	}
#endif /* defined(WL11AC) && defined(WL_MU_RX) && defined(WLCNT) */

	/* per-rate rx for unicast only */
	if (!f.ismulti) {
		wl_cnt_wlc_t *cnt = wlc->pub->_cnt;

		/* update per-rate rx count */
		switch (RSPEC2RATE(pkttag->rspec)) {

		case WLC_RATE_1M:
			WLCNTINCR(cnt->rx1mbps);
			break;
		case WLC_RATE_2M:
			WLCNTINCR(cnt->rx2mbps);
			break;
		case WLC_RATE_5M5:
			WLCNTINCR(cnt->rx5mbps5);
			break;
		case WLC_RATE_6M:
			WLCNTINCR(cnt->rx6mbps);
			break;
		case WLC_RATE_9M:
			WLCNTINCR(cnt->rx9mbps);
			break;
		case WLC_RATE_11M:
			WLCNTINCR(cnt->rx11mbps);
			break;
		case WLC_RATE_12M:
			WLCNTINCR(cnt->rx12mbps);
			break;
		case WLC_RATE_18M:
			WLCNTINCR(cnt->rx18mbps);
			break;
		case WLC_RATE_24M:
			WLCNTINCR(cnt->rx24mbps);
			break;
		case WLC_RATE_36M:
			WLCNTINCR(cnt->rx36mbps);
			break;
		case WLC_RATE_48M:
			WLCNTINCR(cnt->rx48mbps);
			break;
		case WLC_RATE_54M:
			WLCNTINCR(cnt->rx54mbps);
			break;
		default:
			break;
		}
	}
#endif /* WLCNT */

	f.promisc_frame = promisc_frame;

#ifdef WLC_SW_DIVERSITY
	if (!f.ismulti && WLSWDIV_ENAB(wlc) && WLSWDIV_BSSCFG_SUPPORT(bsscfg)) {
		wlc_swdiv_rxpkt_recv(wlc->swdiv, scb, wrxh,
			RSPEC_ISCCK(rspec));
	}
#endif /* WLC_SW_DIVERSITY */

#ifdef WL_LEAKY_AP_STATS
	if (WL_LEAKYAPSTATS_ENAB(wlc->pub)) {
		wlc_leakyapstats_pkt_stats_upd(wlc, bsscfg, f.seq,
				wlc_recover_tsf32(wlc, wrxh), rspec,
				wrxh->rssi, f.prio, f.len);
	}
#endif /* WL_LEAKY_AP_STATS */

#ifdef BCMSPLITRX
	if (BCMSPLITRX_ENAB() && !protected) {
		if (wlc_pktfetch_required(wlc, bsscfg, &f, NULL, FALSE)) {
			wlc_recvdata_schedule_pktfetch(wlc, scb, &f, f.promisc_frame, TRUE, FALSE);
			return;
		}
	}
#endif
#ifdef WLAMPDU
	if (SCB_AMPDU(scb) && !f.ismulti && !promisc_frame)
		wlc_ampdu_recvdata(wlc->ampdu_rx, scb, &f);
	else
#endif
	wlc_recvdata_ordered(wlc, scb, &f);


	return;

toss:
	wlc_log_unexpected_rx_frame_log_80211hdr(wlc, f.h, toss_reason);
#ifdef WL_RX_STALL
	wlc_rx_healthcheck_update_counters(wlc, f.ac, scb, bsscfg, toss_reason, 1);
#endif
	if (WME_ENAB(wlc->pub) && !f.ismulti) {
		WLCNTINCR(wlc->pub->_wme_cnt->rx_failed[WME_PRIO2AC(PKTPRIO(f.p))].packets);
		WLCNTADD(wlc->pub->_wme_cnt->rx_failed[WME_PRIO2AC(PKTPRIO(f.p))].bytes,
		                                     pkttotlen(osh, f.p));
	}
toss_nolog:
	/* Handle error case here */

toss_silently:
	PKTFREE(osh, f.p, FALSE);
	return;
} /* wlc_recvdata */

static void
wlc_rx_scb_upd(struct scb *scb, struct wlc_frminfo *f)
{

	if (!bcmp(wlc_802_1x_hdr, f->pbody, DOT11_LLC_SNAP_HDR_LEN)) {
		scb->flags |= SCB_8021XHDR;
	}
	else {
		scb->flags &= ~SCB_8021XHDR;
	}

#ifdef BCMWAPI_WAI
	if (WAPI_WAI_SNAP(f->pbody)) {
		scb->flags2 |= SCB2_WAIHDR;
	} else {
		scb->flags2 &= ~SCB2_WAIHDR;
	}
#endif /* BCMWAPI_WAI */
}

/** Performs reassembly and per-msdu operations. Expects the frames to be in order */
void BCMFASTPATH
wlc_recvdata_ordered(wlc_info_t *wlc, struct scb *scb, struct wlc_frminfo *f)
{
	struct dot11_llc_snap_header *lsh;
	struct ether_addr a1, a2, a3, a4;
	uint body_offset;
	uint16 prev_seqctl = 0;
	wlc_bsscfg_t *bsscfg;
	osl_t *osh = wlc->osh;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */
	uint8 prio = 0; /* Best-effort */
	bool more_frag = ((f->fc & FC_MOREFRAG) != 0);
	bool sendup_80211;
#if defined(EXT_STA)
	bool decryptFailed;
#endif /* EXT_STA */
	int err;
	wlc_rx_status_t toss_reason = WLC_RX_STS_TOSS_UNKNOWN;

#ifdef STA
	bool is4way_M1 = FALSE;
#endif
	uint16 ether_type = ETHER_TYPE_MIN;

	ASSERT(scb != NULL);

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

#if defined(GTKOE)
	if (GTKOE_ENAB(wlc->pub)) {
		if (wlc_gtk_refresh(wlc, scb, f) == TRUE) {
			WL_TRACE(("wl%d: GTK update/4 way handshake detected in D2\n",
				wlc->pub->unit));
			return;
		}
	}
#endif /* GTKOE */

	if (BSSCFG_SAFEMODE(bsscfg))
		more_frag = FALSE;

	/* For WME, update the prio */
	/* f->prio is already updated with the proper value by the caller(s) */
	if (f->qos)
		prio = f->prio;

	/* Detect and discard duplicates */
	if (!f->ismulti) {
		if (f->fc & FC_RETRY) {
			WLCNTINCR(wlc->pub->_cnt->rxrtry);
			WLCNTSCBINCR(scb->scb_stats.rx_pkts_retried);
#ifdef WL11K
			if (f->qos) {
				wlc_rrm_stat_qos_counter(scb, prio,
					OFFSETOF(rrm_stat_group_qos_t, rxretries));
			}
#endif
			if (scb->seqctl[prio] == f->seq) {
				WL_TRACE(("wl%d: %s: discarding duplicate MPDU %04x "
					"received from %s prio: %d\n", wlc->pub->unit, __FUNCTION__,
					f->seq, bcm_ether_ntoa(&(f->h->a2), eabuf), prio));
				WLCNTINCR(wlc->pub->_cnt->rxdup);
#ifdef WL11K
				if (f->qos) {
					wlc_rrm_stat_qos_counter(scb, prio,
						OFFSETOF(rrm_stat_group_qos_t, rxdup));
				}
#endif
				goto toss_silently;
			}
		}
	}

#ifdef STA
	if (BSSCFG_STA(bsscfg)) {
		wlc_pm_st_t *pm = bsscfg->pm;

		WL_NONE(("%s: psp=%d wme=%d usp=%d qos=%d eosp=%d\n",
		         __FUNCTION__, wlc->PSpoll, WME_ENAB(wlc->pub),
		         pm->apsd_sta_usp, f->qos, f->apsd_eosp));

		/* Check for a PS-Poll response */
		if (pm->PSpoll &&	/* PS-Poll outstanding */
		    !f->ismulti &&	/* unicast */
		    !more_frag &&	/* last frag in burst, or non-frag frame */
		    !AC_BITMAP_TST(scb->apsd.ac_delv, f->ac)) /* not on a delivery-enabled AC */
			wlc_recv_PSpoll_resp(bsscfg, f->fc);

#ifdef WME
		/* Check for end of APSD unscheduled service period */
		if (WME_ENAB(wlc->pub) && pm->apsd_sta_usp &&
		    f->qos && f->apsd_eosp) {
			WL_PS(("wl%d.%d: APSD sleep\n",
			       wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
#ifdef WLTDLS
				if (BSS_TDLS_ENAB(wlc, bsscfg)) {
					wlc_tdls_return_to_base_ch_on_eosp(wlc->tdls, scb);
				}
#endif /* WLTDLS */

			wlc_set_apsd_stausp(bsscfg, FALSE);
#ifdef WLP2P
			/* Update if APSD re-trigger is no longer expected */
			if (BSS_P2P_ENAB(wlc, bsscfg) &&
			    (f->fc & FC_MOREDATA) == 0)
				wlc_p2p_apsd_retrigger_upd(wlc->p2p, bsscfg, FALSE);
#endif
		}
#endif /* WME */
	}
#endif /* STA */

#ifdef WLTDLS
	/* indicating recv data frame for TDLS channel switch, before dropping the NULL data pkt */
	if (BSS_TDLS_ENAB(wlc, bsscfg)) {
		wlc_tdls_rcv_data_frame(wlc->tdls, scb, f->rxh);
	}
#endif /* WLTDLS */

	/* null data frames */
	if (FC_SUBTYPE_ANY_NULL(f->subtype)) {
#ifdef WLWNM_AP
		/* update rx_tstame for BSS Max Idle Period */
		if (WLWNM_ENAB(wlc->pub) && !wlc_wnm_bss_idle_opt(wlc, bsscfg))
			wlc_wnm_rx_tstamp_update(wlc, scb);
#endif /* WLWNM_AP */
		if (WME_ENAB(wlc->pub) && (f->subtype == FC_SUBTYPE_QOS_NULL)) {
			WLCNTINCR(wlc->pub->_cnt->rxqosnull);
		} else {
			WLCNTINCR(wlc->pub->_cnt->rxnull);
		}
		goto toss_silently;
	}

#ifdef EXT_STA
	/* Update 802.11 seq even if decryption failed.
	 */
	if (WLEXTSTA_ENAB(wlc->pub))
		decryptFailed = FALSE;
#endif /* EXT_STA */
	/* the key, if any, will be provided by wsec_recvdata */
	f->rx_wep = f->fc & FC_WEP;

	/* If WLF_RX_KM bit is set, then key processing was already done:
	 * This is a pktfetched pkt, so skip key processing again
	*/
	if (!BSSCFG_SAFEMODE(bsscfg) && !(WLPKTTAG(f->p)->flags & WLF_RX_KM) &&
		(f->rx_wep || (WSEC_ENABLED(bsscfg->wsec) && bsscfg->wsec_restrict)) &&
			(!BYPASS_ENC_DATA(bsscfg))) {
		if ((err = wlc_keymgmt_recvdata(wlc->keymgmt, f)) != BCME_OK) {
#ifdef BCMSPLITRX
			if (PKTFRAGUSEDLEN(osh, f->p) > 0 && err == BCME_DATA_NOTFOUND)
				goto pktfetch;
#endif
			WLCNTSCB_COND_INCR((f->rx_wep), scb->scb_stats.rx_decrypt_failures);
#ifdef EXT_STA
			if (WLEXTSTA_ENAB(wlc->pub))
				decryptFailed = TRUE;
			else
#endif /* EXT_STA */
			{
				toss_reason = WLC_RX_STS_TOSS_DECRYPT_FAIL;
				goto toss;
			}
		}
#ifdef BCMSPLITRX
		else if (PKTFRAGUSEDLEN(osh, f->p) > 0) {
			/* Security checks clear for pktfetch;
			 * Check for TDLS setup frames and 802.1x
			 * At this point, the frame is protected and wlc_keymgmt_recvdata
			 * has already moved the pkt body ptr ahead of IV
			 * To make sure wlc_pktfetch_required does not adjust
			 * pbody ptr for IV_len again, pass protected=FALSE here
			*/
			if (wlc_pktfetch_required(wlc, bsscfg, f,
				&f->key_info, FALSE))
				goto pktfetch;
		}
#endif /* BCMSPLITRX */
		WLCNTSCB_COND_INCR((f->rx_wep), scb->scb_stats.rx_decrypt_succeeds);
	}
#ifdef WLMESH
	if (WLMESH_ENAB(wlc->pub) && BSSCFG_MESH(bsscfg)) {
		if (wlc_mroute_recvdata_ordered(wlc->mesh_info, bsscfg, scb, f) != BCME_OK) {
			return;
		}
	}
#endif /* WLMESH */
	if (!f->ismulti) {
		/* Update cached seqctl */
		prev_seqctl = scb->seqctl[prio];
		scb->seqctl[prio] = f->seq;
	}

#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wlc->pub)) {
		if (decryptFailed)
			goto toss;
	}
#endif /* EXT_STA */

	/* possible frag reassembly */
	if (!f->ismulti) {
		/* non frag or first frag */
		if ((f->seq & FRAGNUM_MASK) == 0) {
			/* Discard partially-received frame */
			if (scb->flags & SCB_DEFRAG_INPROG) {
				wlc_defrag_prog_reset(wlc->frag, scb, prio, prev_seqctl);
			}
			/* first frag */
			if (more_frag) {
				if (!wlc_defrag_first_frag_proc(wlc->frag, scb, prio, f)) {
						toss_reason = WLC_RX_STS_TOSS_BUF_FAIL;
					goto toss;
				}
				wlc_rx_scb_upd(scb, f);
				return;
			}
			/* non frag */
		}
		/* subsequent frag */
		else {
			if (!wlc_defrag_other_frag_proc(wlc->frag, scb, prio, f, prev_seqctl,
					more_frag, BSSCFG_SAFEMODE(bsscfg) == TRUE)) {
					toss_reason = WLC_RX_STS_TOSS_RXFRAG_ERR;
				goto toss;
			}
			/* non-last frag... */
			if (more_frag) {
				return;
			}
		}
	} /* unicast */

	/* f->p is full msdu... */
	ASSERT(!more_frag);

	sendup_80211 = (bsscfg->wlcif->if_flags & WLC_IF_PKT_80211) != 0;

	if (sendup_80211) {

#if defined(WLTDLS)
		/* patch up the dpt header so that looks like coming from AP */
		if (f->istdls) {

			struct dot11_header *h = f->h;

			wlc_bsscfg_t *parent = NULL;
			if (TDLS_ENAB(wlc->pub) && BSSCFG_IS_TDLS(SCB_BSSCFG(scb))) {
				parent = wlc_tdls_get_parent_bsscfg(wlc, scb);
				WL_TDLS(("wl%d:%s(): TDLS parent BSSCFG 0x%p\n",
					wlc->pub->unit, __FUNCTION__, OSL_OBFUSCATE_BUF(parent)));
				WL_TDLS(("a1:%s, ", bcm_ether_ntoa(&(h->a1), eabuf)));
				WL_TDLS(("a2:%s, ", bcm_ether_ntoa(&(h->a2), eabuf)));
				WL_TDLS(("a3:%s\n ", bcm_ether_ntoa(&(h->a3), eabuf)));
				WL_TDLS(("parent BSSID %s\n, ",
					bcm_ether_ntoa(&(parent->BSSID), eabuf)));
			}
			ASSERT(parent != NULL);
			h->fc |= htol16(FC_FROMDS);
			/* TDLS src address goes to a3; AP bssid into a2 */
			bcopy(&h->a2, &h->a3, ETHER_ADDR_LEN);
			bcopy(&parent->BSSID, &h->a2, ETHER_ADDR_LEN);
		}
#endif /* WLTDLS */
	}

#ifdef WLAMSDU
	/* A-MSDU deagg */
	if (WLPKTTAG(f->p)->flags & WLF_AMSDU) {
		if (BSSCFG_SAFEMODE(bsscfg)) {
			WL_ERROR(("%s(): safe mode no AMSDU support!\n", __FUNCTION__));
			toss_reason = WLC_RX_STS_TOSS_SAFE_MODE_AMSDU;
			goto toss;
		}

#if defined(STA)
		if (BSSCFG_STA(bsscfg)) {
			wlc_pm2_ret_upd_last_wake_time(bsscfg, &f->wrxh->tsf_l);
		}
#endif

		if (WLPKTTAG(f->p)->flags & WLF_HWAMSDU) {
			wlc_amsdu_deagg_hw(wlc->ami, scb, f);
			return;
		} else {
#ifdef WLAMSDU_SWDEAGG
			wlc_amsdu_deagg_sw(wlc->ami, scb, f);
			return;
#else
			WL_ERROR(("%s: sw deagg is not supported\n", __FUNCTION__));
			ASSERT(0);
			goto toss;
#endif /* WLAMSDU_SWDEAGG */
		}
	}
#endif /* WLAMSDU */

	/*
	 * 802.11 -> 802.3/Ethernet header conversion
	 * Part 1: eliminate possible overwrite problems, find right eh pointer
	 */
	body_offset = (uint)(f->pbody - (uchar*)PKTDATA(osh, f->p));

#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB() && PKTISHDRCONVTD(osh, f->p)) {
		uint8* data;
		f->eh = (struct ether_header *)PKTPULL(osh, f->p,
			body_offset + 2);
		data = (uint8*)PKTDATA(osh, f->p);
		f->da = (struct ether_addr*) data;
		f->sa = (struct ether_addr*)(data + ETHER_ADDR_LEN);
		goto skip_conv;
	}
#endif /* BCMPCIEDEV */

	eacopy((char *)&(f->h->a1), (char *)&a1);
	eacopy((char *)&(f->h->a2), (char *)&a2);
	eacopy((char *)&(f->h->a3), (char *)&a3);
	if (f->wds)
		eacopy((char *)&(f->h->a4), (char *)&a4);

	/*
	 * 802.11 -> 802.3/Ethernet header conversion
	 * Part 2: find sa/da pointers
	 */
	if ((f->fc & FC_TODS) == 0) {
		f->da = &a1;
		if ((f->fc & FC_FROMDS) == 0)
			f->sa = &a2;
		else
			f->sa = &a3;
	} else {
		f->da = &a3;
		if ((f->fc & FC_FROMDS) == 0)
			f->sa = &a2;
		else
			f->sa = &a4;
	}


	/* WME/TKIP support - set packet priority based on first received MPDU */
	PKTSETPRIO(f->p, f->prio);

	if ((f->key != NULL) && (f->key_info.algo == CRYPTO_ALGO_TKIP) && f->rx_wep) {

		struct ether_header eh, *ehp;

		memcpy(&eh, f->pbody - sizeof(eh), sizeof(eh));

		ehp = (struct ether_header*)PKTPULL(osh, f->p, body_offset - ETHER_HDR_LEN);
		eacopy((char*)(f->da), ehp->ether_dhost);
		eacopy((char*)(f->sa), ehp->ether_shost);
		ehp->ether_type = hton16((uint16)f->body_len);

		if (wlc_key_rx_msdu(f->key, f->p, f->rxh) != BCME_OK)
			goto toss;

		memcpy(ehp, &eh, sizeof(eh));
		PKTPUSH(osh, f->p, body_offset - ETHER_HDR_LEN);
	}

	if (sendup_80211) {
		f->eh = (struct ether_header *)f->h;

		/* Find the snap hdr in order to find the ether_type which we will
		 * examine in order to detect cram/802.1x
		 */
		if (!BSSCFG_SAFEMODE(bsscfg)) {

			lsh = (struct dot11_llc_snap_header *)(f->pbody);
			if (rfc894chk(wlc, lsh)) {
				ether_type = ntoh16(lsh->type);
			} else {
				ether_type = (uint16)f->body_len;
			}

#ifdef EXT_STA
			if (WLEXTSTA_ENAB(wlc->pub)) {
				if (!wlc_exempt_pkt(wlc, bsscfg, scb, f, ether_type, prio))
					goto toss;
			}
#endif /* EXT_STA */
#ifdef STA
			if (wlc_keymgmt_b4m4_enabled(wlc->keymgmt, bsscfg)) {
				WL_WSEC(("wl%d:%s(): body_offset=%d, body_len=%d, pkt_len=%d\n",
					wlc->pub->unit, __FUNCTION__, body_offset, f->body_len,
					PKTLEN(osh, f->p)));
				if (wlc_is_4way_msg(wlc, f->p, body_offset, PMSG1)) {
					WL_WSEC(("wl%d: %s: allowing unencrypted 802.1x 4-Way M1\n",
						wlc->pub->unit, __FUNCTION__));
					wlc_keymgmt_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_M1_RX,
						bsscfg, scb, NULL /* key */, NULL /* pkt */);
				}
			}
#endif /* STA */
		}

		if (ETHER_ISMULTI(f->da)) {
			WLCNTINCR(wlc->pub->_cnt->rxmulti);
			WLCNTINCR(bsscfg->wlcif->_cnt->rxmulti);
		}

		if (!BSSCFG_SAFEMODE(bsscfg)) {

			/* reposition 802.11 header to strip IV and/or QoS field */
			if (f->rx_wep) {
				uchar *dst;
				uint excess = 0;

				excess += f->key_info.iv_len;

				for (dst = f->pbody - 1; dst >= (uchar *)PKTDATA(osh, f->p); dst--)
				        *dst = *(dst - excess);

				f->h = (struct dot11_header *)PKTPULL(osh, f->p, excess);
			}

			if (!(APSTA_ENAB(wlc->pub) && AP_ACTIVE(wlc)))
				ASSERT((f->fc & FC_TODS) == 0);

#if defined(WLTDLS)
			/* patch up the dpt header so that looks like coming from AP */
			if (f->istdls) {
				struct dot11_header *h = f->h;
				wlc_bsscfg_t *parent = NULL;
				if (TDLS_ENAB(wlc->pub) && BSSCFG_IS_TDLS(SCB_BSSCFG(scb)))
					parent = wlc_tdls_get_parent_bsscfg(wlc, scb);
				ASSERT(parent != NULL);
				h->fc |= htol16(FC_FROMDS);
				/* TDLS src address goes to a3; AP bssid into a2 */
				bcopy(&h->a2, &h->a3, ETHER_ADDR_LEN);
				bcopy(&parent->BSSID, &h->a2, ETHER_ADDR_LEN);
			}
#endif /* WLTDLS */

#ifdef STA
			WL_RTDC(wlc, "wlc_recvdata_ordered %u", PKTLEN(osh, f->p), 0);
			if (BSSCFG_STA(bsscfg)) {
				if (!ETHER_ISMULTI(f->da)) {
					bsscfg->pm->pm2_rx_pkts_since_bcn++;
					wlc_pm2_ret_upd_last_wake_time(bsscfg, &f->wrxh->tsf_l);
					wlc_update_sleep_ret(bsscfg, TRUE, FALSE,
						pkttotlen(osh, f->p), 0);
				}
			}
#endif /* STA */

#if (defined(BCMSUP_PSK) || defined(BCMCCX)) && defined(BCMINTSUP)
			if (ether_type == ETHER_TYPE_802_1X &&
			    SUP_ENAB(wlc->pub) &&
			    BSS_SUP_TYPE(wlc->idsup, bsscfg) != SUP_UNUSED) {
				eapol_header_t *eapol_hdr = (eapol_header_t *)((uint8 *)f->h +
					DOT11_A3_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN - ETHER_HDR_LEN);

				if (f->qos == TRUE) {
					eapol_hdr = (eapol_header_t *)((uint8 *)eapol_hdr +
						DOT11_QOS_LEN);
				}

				/* ensure that we have a valid eapol - header/length */
				if (!wlc_valid_pkt_field(osh, f->p, (void*)eapol_hdr,
					OFFSETOF(eapol_header_t, body)) ||
					!wlc_valid_pkt_field(osh, f->p, (void*)&eapol_hdr->body[0],
					(int)ntoh16(eapol_hdr->length))) {
					WL_WSEC(("wl%d: %s: bad eapol - header too small.\n",
						wlc->pub->unit, __FUNCTION__));
					WLCNTINCR(wlc->pub->_cnt->rxrunt);
					toss_reason = WLC_RX_STS_TOSS_RUNT_FRAME;
					goto toss;
				}

				if (wlc_sup_eapol(wlc->idsup, bsscfg,
					eapol_hdr, (f->rx_wep != 0)))
					goto toss_silently;
			}
#endif /* BCMSUP_PSK || BCMCCX && BCMINTSUP */

#if defined(BCMAUTH_PSK)
			if (ether_type == ETHER_TYPE_802_1X &&
			    BCMAUTH_PSK_ENAB(wlc->pub) &&
			    BSS_AUTH_TYPE(wlc->authi, bsscfg) != AUTH_UNUSED) {
				eapol_header_t *eapol_hdr = (eapol_header_t *)((uint8 *)f->h +
					DOT11_A3_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN - ETHER_HDR_LEN);

				if (f->qos == TRUE) {
					eapol_hdr = (eapol_header_t *)((uint8 *)eapol_hdr +
						DOT11_QOS_LEN);
				}

				/* ensure that we have a valid eapol - header/length */
				if (!wlc_valid_pkt_field(osh, f->p, (void*)eapol_hdr,
					OFFSETOF(eapol_header_t, body)) ||
					!wlc_valid_pkt_field(osh, f->p, (void*)&eapol_hdr->body[0],
					(int)ntoh16(eapol_hdr->length))) {
					WL_WSEC(("wl%d: %s: bad eapol - header too small.\n",
						wlc->pub->unit, __FUNCTION__));
					WLCNTINCR(wlc->pub->_cnt->rxrunt);
					toss_reason = WLC_RX_STS_TOSS_RUNT_FRAME;
					goto toss;
				}
				if (wlc_auth_eapol(wlc->authi, bsscfg, eapol_hdr,
				                   (f->rx_wep != 0), scb))
					goto toss_silently;
			}
#endif /* BCMAUTH_PSK */

#if defined(BCMCCX) && defined(CCX_SDK)
			if (CCX_ENAB(wlc->pub) && ether_type <= ETHER_MAX_DATA &&
			    BSSCFG_STA(bsscfg) && !BSS_P2P_ENAB(wlc, cfg) && bsscfg->BSS &&
			    (f->body_len >= (DOT11_LLC_SNAP_HDR_LEN + CCX_IAPP_HDR_LEN) &&
			     !bcmp((uint8*)f->pbody, CISCO_AIRONET_SNAP, DOT11_LLC_SNAP_HDR_LEN)) &&
			    bsscfg->associated) {
				if (wlc_ccx_chk_iapp_frm(wlc->ccx, f, bsscfg))
					goto toss;
			}
#endif /* BCMCCX && CCX_SDK */

			/* Process a regular packet with 802.11 hdr */

#ifdef WLTDLS
			if (TDLS_ENAB(wlc->pub) && BSSCFG_STA(bsscfg)) {
				WL_TDLS(("wl%d:%s(): ether_type=0x%04x\n", wlc->pub->unit,
					__FUNCTION__, ether_type));
				if (ether_type == ETHER_TYPE_89_0D) {
					uint offset = ETHER_HDR_LEN;
					if (WLEXTSTA_ENAB(wlc->pub))
						offset = DOT11_A3_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN;
					wlc_tdls_rcv_action_frame(wlc->tdls, scb, f, offset);
					return;
				}
			}
#endif
		}

	} else {

		lsh = (struct dot11_llc_snap_header *)(f->pbody);

#ifdef STA
		/* check For M1 pkt in 4 way handshake */
		if ((f->body_len >= DOT11_LLC_SNAP_HDR_LEN) &&
			(ntoh16(lsh->type) == ETHER_TYPE_802_1X))
			is4way_M1 = wlc_is_4way_msg(wlc, f->p, body_offset, PMSG1);
#endif
		/* calc the bytes included in the 802.11 header (A3 or A4 format + qos + iv) */

		/*
		 * 802.11 -> 802.3/Ethernet header conversion part 1:
		 * - if ([AA AA 03 00 00 00] and protocol is not in SST) or
		 *   if ([AA AA 03 00 00 F8] and protocol is in SST), convert to RFC894
		 * - otherwise,
		 *	 preserve 802.3 (including RFC1042 with protocol in SST)
		 *
		 * NOTE: Need to be careful when changing this because MIC calculation
		 * depends on 802.3 header still being present
		 */
		if ((f->body_len >= DOT11_LLC_SNAP_HDR_LEN) && rfc894chk(wlc, lsh)) {
			/* RFC894 */
			f->eh = (struct ether_header *)PKTPULL(osh, f->p,
				body_offset + DOT11_LLC_SNAP_HDR_LEN - ETHER_HDR_LEN);
		} else {
			/* RFC1042 */
#ifdef BCMCCX
			if ((f->body_len >= DOT11_LLC_SNAP_HDR_LEN) &&
				WSEC_CKIP_MIC_ENABLED(bsscfg->wsec) &&
				!bcmp(lsh, ckip_llc_snap, CKIP_LLC_SNAP_LEN)) {
				f->eh = (struct ether_header *)PKTPULL(osh, f->p,
					body_offset +
					(2 + CKIP_LLC_SNAP_LEN + CKIP_MIC_SIZE + CKIP_SEQ_SIZE)
					- ETHER_HDR_LEN);
			}
			else
#endif
			{
				 if (f->body_len <= DOT11_LLC_HDR_LEN_MIN) {
					WL_ERROR(("%s: packet len too short (%d)<"
						"DOT11_LLC_HDR_LEN_MIN\n",
						__FUNCTION__, PKTLEN(osh, f->p)));
					toss_reason = WLC_RX_STS_TOSS_PKT_LEN_SHORT;
					goto toss;
				}
				f->eh = (struct ether_header *)PKTPULL(osh, f->p,
					body_offset - ETHER_HDR_LEN);
				f->eh->ether_type = hton16((uint16)f->body_len);	/* length */
			}
		}

		ether_type = ntoh16(f->eh->ether_type);

		/*
		 * 802.11 -> 802.3/Ethernet header conversion
		 * Part 3: do the conversion
		 */
		eacopy((char*)(f->da), (char*)&(f->eh->ether_dhost));
		eacopy((char*)(f->sa), (char*)&(f->eh->ether_shost));
#ifdef BCMPCIEDEV
skip_conv:
#endif /* BCMPCIEDEV */
#ifdef STA
		if ((ether_type == ETHER_TYPE_802_1X) &&
			(wlc_keymgmt_b4m4_enabled(wlc->keymgmt, bsscfg))) {
			/* Start buffering once we recieve M1 pkt */
			if (is4way_M1) {
				WL_WSEC(("wl%d: %s: allowing unencrypted "
				         "802.1x 4-Way M1\n", wlc->pub->unit, __FUNCTION__));
				wlc_keymgmt_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_M1_RX,
					bsscfg, scb, NULL /* key */, NULL /* pkt */);
			}
		}
#endif /* STA */

		if (ETHER_ISMULTI(f->da)) {
			WLCNTINCR(wlc->pub->_cnt->rxmulti);
			WLCNTINCR(bsscfg->wlcif->_cnt->rxmulti);
		}

		/* toss if station not yet authorized to transmit non-802.1X frames */
		if (bsscfg->eap_restrict && !SCB_AUTHORIZED(scb)) {
			if (ether_type != ETHER_TYPE_802_1X) {
				WL_ERROR(("wl%d: non-802.1X frame from unauthorized station %s\n",
					wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf)));
				toss_reason = WLC_RX_STS_TOSS_8021X_FRAME;
				goto toss;
			}
		}

#ifdef BCMWAPI_WAI
		/* toss if station not yet authorized to transmit non-WAI
		 * frames except 802.1X frame
		 */
		if (WAPI_WAI_RESTRICT(wlc, bsscfg) && !SCB_AUTHORIZED(scb)) {
			if (ether_type != ETHER_TYPE_WAI &&
			    ether_type != ETHER_TYPE_802_1X) {
				WL_ERROR(("wl%d: non-WAI frame from unauthorized station %s\n",
					wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf)));
				goto toss;
			}
		}
#endif /* BCMWAPI_WAI */

#ifdef STA
		WL_RTDC(wlc, "wlc_recvdata_ordered %u", PKTLEN(osh, f->p), 0);
		if (BSSCFG_STA(bsscfg)) {
			if (!ETHER_ISMULTI(f->da)) {
				bsscfg->pm->pm2_rx_pkts_since_bcn++;
				wlc_pm2_ret_upd_last_wake_time(bsscfg, &f->wrxh->tsf_l);
				wlc_update_sleep_ret(bsscfg, TRUE, FALSE,
					pkttotlen(osh, f->p), 0);
			}
		}

#if (defined(BCMSUP_PSK) || defined(BCMCCX)) && defined(BCMINTSUP)
		if (ether_type == ETHER_TYPE_802_1X &&
		    SUP_ENAB(wlc->pub) &&
		    BSS_SUP_TYPE(wlc->idsup, bsscfg) != SUP_UNUSED) {
			eapol_header_t *eapol_hdr = (eapol_header_t *)f->eh;

			/* ensure that we have a valid eapol - header/length */
			if (!wlc_valid_pkt_field(osh, f->p, (void*)eapol_hdr,
				OFFSETOF(eapol_header_t, body)) ||
				!wlc_valid_pkt_field(osh, f->p, (void*)&eapol_hdr->body[0],
				(int)ntoh16(eapol_hdr->length))) {
				WL_WSEC(("wl%d: %s: bad eapol - header too small.\n",
					wlc->pub->unit, __FUNCTION__));
				WLCNTINCR(wlc->pub->_cnt->rxrunt);
				toss_reason = WLC_RX_STS_TOSS_RUNT_FRAME;
				goto toss;
			}

			if (wlc_sup_eapol(wlc->idsup, bsscfg,
				eapol_hdr, (f->rx_wep != 0)))
				goto toss_silently;
		}
#endif /* BCMSUP_PSK || BCMCCX && BCMINTSUP */

#if defined(BCMAUTH_PSK)
		if (ether_type == ETHER_TYPE_802_1X &&
		    BCMAUTH_PSK_ENAB(wlc->pub) &&
		    BSS_AUTH_TYPE(wlc->authi, bsscfg) != AUTH_UNUSED) {
			eapol_header_t *eapol_hdr = (eapol_header_t *)f->eh;
			/* ensure that we have a valid eapol - header/length */
			if (!wlc_valid_pkt_field(osh, f->p, (void*)eapol_hdr,
				OFFSETOF(eapol_header_t, body)) ||
				!wlc_valid_pkt_field(osh, f->p, (void*)&eapol_hdr->body[0],
				(int)ntoh16(eapol_hdr->length))) {
				WL_WSEC(("wl%d: %s: bad eapol - header too small.\n",
					wlc->pub->unit, __FUNCTION__));
				WLCNTINCR(wlc->pub->_cnt->rxrunt);
				toss_reason = WLC_RX_STS_TOSS_RUNT_FRAME;
				goto toss;
			}

			if (wlc_auth_eapol(wlc->authi, bsscfg, eapol_hdr,
				(f->rx_wep != 0), scb))
				goto toss_silently;
		}
#endif /* BCMAUTH_PSK */

#ifdef BCMCCX
		if (CCX_ENAB(wlc->pub) && (ether_type <= ETHER_MAX_DATA) &&
		    BSSCFG_STA(bsscfg) && !BSS_P2P_ENAB(wlc, bsscfg) &&
		    bsscfg->BSS &&
		    (PKTLEN(osh, f->p) >=
			(ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN + CCX_IAPP_HDR_LEN) &&
		     !bcmp((uint8*)f->eh + ETHER_HDR_LEN, CISCO_AIRONET_SNAP,
			DOT11_LLC_SNAP_HDR_LEN)) &&
		    bsscfg->associated)
			if (wlc_ccx_chk_iapp_frm(wlc->ccx, f, bsscfg))
				goto toss;
#endif /* BCMCCX */
#endif	/* STA */


#ifdef WLTDLS
		if (TDLS_ENAB(wlc->pub) && BSSCFG_STA(bsscfg)) {
			WL_TDLS(("wl%d:%s(): ether_type=0x%04x\n, TDLS link=%s", wlc->pub->unit,
				__FUNCTION__, ether_type,
				f->istdls ? "TRUE" : "FALSE"));
			if (ether_type == ETHER_TYPE_89_0D) {
				wlc_tdls_rcv_action_frame(wlc->tdls, scb, f, ETHER_HDR_LEN);
				return;
			}
		}
#endif /* WLTDLS */
	}

#ifndef DWDS_ACCEPT_3ADDR
	if (SCB_A4_DATA(scb) && !f->wds &&
	    (ether_type != ETHER_TYPE_802_1X)) {
#ifdef BCMDBG
		WL_NONE(("wl%d: Tossing a 3-addr data frame from %s for DWDS\n",
			wlc->pub->unit, bcm_ether_ntoa(&a2, eabuf)));
#endif
		toss_reason = WLC_RX_STS_TOSS_3_ADDR_FRAME;
		goto toss;
	}
#endif /* DWDS_ACCEPT_3ADDR */
#ifdef WLWNM
	/* Do the WNM processing */
	if (bsscfg && BSSCFG_STA(bsscfg) && WLWNM_ENAB(wlc->pub) && f->ismulti) {
		if (wlc_wnm_packets_handle(bsscfg, f->p, 0) == WNM_DROP)
			goto toss;
	}
#endif /* WLWNM */

	wlc_recvdata_sendup(wlc, scb, f->wds, f->da, f->p);
	return;

#ifdef BCMSPLITRX
pktfetch:
	wlc_recvdata_schedule_pktfetch(wlc, scb, f, f->promisc_frame, TRUE, FALSE);
	return;
#endif

toss:
	wlc_log_unexpected_rx_frame_log_80211hdr(wlc, f->h, toss_reason);
#ifdef WL_RX_STALL
	wlc_rx_healthcheck_update_counters(wlc, f->ac, scb, bsscfg, toss_reason, 1);
#endif
	if (WME_ENAB(wlc->pub)) {
		WLCNTINCR(wlc->pub->_wme_cnt->rx_failed[WME_PRIO2AC(PKTPRIO(f->p))].packets);
		WLCNTADD(wlc->pub->_wme_cnt->rx_failed[WME_PRIO2AC(PKTPRIO(f->p))].bytes,
		                                     pkttotlen(osh, f->p));
	}
toss_silently:
	PKTFREE(osh, f->p, FALSE);
	return;
}

int wlc_process_eapol_frame(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct scb *scb, struct wlc_frminfo *f, void *pkt)
{
#if ((defined(BCMSUP_PSK) || defined(BCMCCX)) && defined(BCMINTSUP)) || \
	defined(BCMAUTH_PSK)
	eapol_header_t *eapol_hdr = (eapol_header_t *)PKTDATA(wlc->osh, pkt);
#endif
#if (defined(BCMSUP_PSK) || defined(BCMCCX)) && defined(BCMINTSUP)
	if (SUP_ENAB(wlc->pub) &&
	(BSS_SUP_TYPE(wlc->idsup, bsscfg) != SUP_UNUSED)) {
		/* ensure that we have a valid eapol - header/length */
		if (!wlc_valid_pkt_field(wlc->osh, pkt, (void*)eapol_hdr,
			OFFSETOF(eapol_header_t, body)) ||
			!wlc_valid_pkt_field(wlc->osh, pkt, (void*)&eapol_hdr->body[0],
			(int)ntoh16(eapol_hdr->length))) {
			WL_ERROR(("wl%d: %s: bad eapol - header too small.\n",
				wlc->pub->unit, __FUNCTION__));
			WLCNTINCR(wlc->pub->_cnt->rxrunt);
			return FALSE;
		}

		if (wlc_sup_eapol(wlc->idsup, bsscfg,
			eapol_hdr, (f->rx_wep != 0)))
			return TRUE;
	}
#endif /* BCMSUP_PSK || BCMCCX && BCMINTSUP */
#if defined(BCMAUTH_PSK)
	if (BCMAUTH_PSK_ENAB(wlc->pub) &&
	    BSS_AUTH_TYPE(wlc->authi, bsscfg) != AUTH_UNUSED) {
		eapol_hdr = (eapol_header_t *)f->eh;
		/* ensure that we have a valid eapol - header/length */
		if (!wlc_valid_pkt_field(wlc->osh, f->p, (void*)eapol_hdr,
			OFFSETOF(eapol_header_t, body)) ||
			!wlc_valid_pkt_field(wlc->osh, f->p, (void*)&eapol_hdr->body[0],
			(int)ntoh16(eapol_hdr->length))) {
			WL_WSEC(("wl%d: %s: bad eapol - header too small.\n",
				wlc->pub->unit, __FUNCTION__));
			WLCNTINCR(wlc->pub->_cnt->rxrunt);
			return FALSE;
		}

		if (wlc_auth_eapol(wlc->authi, bsscfg, eapol_hdr,
			(f->rx_wep != 0), scb))
			return TRUE;
	}
#endif /* BCMAUTH_PSK */
	return FALSE;
}


/* BSS specific data frames sendup filters */

/* filter return code */
#define WLC_SENDUP_CONT	0	/* continue the sendup */
#define WLC_SENDUP_STOP	1	/* stop the sendup */

/** pre filter, called in the beginning of the sendup path before packet processing */
static int BCMFASTPATH
wlc_bss_sendup_pre_filter(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct scb *scb,
	void *pkt, bool multi)
{
	BCM_REFERENCE(wlc);
	BCM_REFERENCE(pkt);
	BCM_REFERENCE(multi);
	BCM_REFERENCE(scb);

	ASSERT(cfg != NULL);

	if (BSSCFG_AP(cfg)) {
#ifdef WMF
		/* Do the WMF processing for multicast packets */
		if (WMF_ENAB(cfg) && multi) {
			switch (wlc_wmf_packets_handle(wlc->wmfi, cfg, scb, pkt, 1)) {
			case WMF_TAKEN:
				/* The packet is taken by WMF return */
				return WLC_SENDUP_STOP;
			case WMF_DROP:
				/* The packet drop decision by WMF free and return */
				PKTFREE(wlc->osh, pkt, FALSE);
				return WLC_SENDUP_STOP;
			default:
				/* Continue the forwarding path */
				break;
			}
		}
#endif /* WMF */

#ifdef L2_FILTER
		if (L2_FILTER_ENAB(wlc->pub)) {
			if (wlc_l2_filter_rcv_data_frame(wlc, cfg, pkt) == 0)
				return WLC_SENDUP_STOP;
		}
#endif /* L2_FILTER */
	}

	return WLC_SENDUP_CONT;
}

/** post filter, called at the end the sendup path after packet processing */
static int BCMFASTPATH
wlc_bss_sendup_post_filter(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct scb *scb,
	void *pkt, bool multi)
{
	BCM_REFERENCE(wlc);
	BCM_REFERENCE(pkt);
	BCM_REFERENCE(multi);
	BCM_REFERENCE(scb);
	/* sanity checks */
	ASSERT(cfg != NULL);

#ifdef L2_FILTER
	if (L2_FILTER_ENAB(wlc->pub) && BSSCFG_STA(cfg)) {
		if (wlc_l2_filter_rcv_data_frame(wlc, cfg, pkt) == 0)
			return WLC_SENDUP_STOP;
	}
#endif

#ifdef TRAFFIC_MGMT
	if (TRAFFIC_MGMT_ENAB(wlc->pub)) {
		if (wlc_trf_mgmt_handle_pkt(wlc->trf_mgmt_ctxt, cfg, scb, pkt, FALSE) == BCME_OK)
			return WLC_SENDUP_STOP;
	}
#endif

	return WLC_SENDUP_CONT;
}

int BCMFASTPATH
wlc_send_fwdpkt(wlc_info_t *wlc, void *p, wlc_if_t *wlcif)
{
	return wlc_recvdata_sendpkt(wlc, p, wlcif);
}

static int BCMFASTPATH
wlc_recvdata_sendpkt(wlc_info_t *wlc, void *p, wlc_if_t *wlcif)
{
	ASSERT(PKTCLINK(p) == NULL);

	/* Clear pkttag information saved in recv path */
	WLPKTTAGCLEAR(p);

	/* Before forwarding, fix the priority.
	 * For priority set in rxfrag will be shifted to
	 * tx frag in wl_create_fwdpkt()
	 */
	if (QOS_ENAB(wlc->pub) && (PKTPRIO(p) == 0))  {
		pktsetprio(p, FALSE);
	}

#if defined(BCMPCIEDEV) && defined(BCMFRAGPOOL)
	if (BCMPCIEDEV_ENAB() && (PKTISRXFRAG(wlc->osh, p))) {
		if ((p = wl_create_fwdpkt(wlc->wl, p, wlcif->wlif)) == NULL) {
			return BCME_NOMEM;
		}
	}
#endif /* BCMPCIEDEV && BCMFRAGPOOL */

	if (wlc_sendpkt(wlc, p, wlcif)) {
		return BCME_ERROR;
	} else {
		return BCME_OK;
	}
}

/* forward and/or sendup a received data frame */
void BCMFASTPATH
wlc_recvdata_sendup(wlc_info_t *wlc, struct scb *scb, bool wds, struct ether_addr *da, void *p)
{
	struct ether_header *eh;
	osl_t *osh;
	wlc_bsscfg_t *bsscfg;
	uint16 ether_type;
	struct ether_addr *ether_dhost;
	bool multi;
#if defined(PKTC) || defined(PKTC_DONGLE)
	bool rfc_update = TRUE;
#endif
	uint pktlen;
#if defined(BCMDBG)
	char eabuf1[ETHER_ADDR_STR_LEN];
	char eabuf2[ETHER_ADDR_STR_LEN];
	char eabuf3[ETHER_ADDR_STR_LEN];
	eabuf1[0] = 0;
	eabuf2[0] = 0;
	eabuf3[0] = 0;
#endif /* BCMDBG */

	ASSERT(scb != NULL);
	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	osh = wlc->osh;
	eh = (struct ether_header*) PKTDATA(osh, p);
	ether_type = ntoh16(eh->ether_type);
	pktlen = PKTISHDRCONVTD(osh, p) ? PKTFRAGUSEDLEN(osh, p) : pkttotlen(osh, p);
	BCM_REFERENCE(pktlen);

	/*
	 * If VLAN Mode is active, and an 802.1Q VLAN Priority Tag (VLAN ID 0) is present,
	 * extract the priority from it and strip the tag (satisfies requirements of WMM
	 * section A.6 and WHQL).
	 */
	if (ether_type == ETHER_TYPE_8021Q &&
#if defined(EXT_STA)
	    /* N.B.: VLAN tag managed by nwifi.sys */
	    !WLEXTSTA_ENAB(wlc->pub) &&
#endif
	    !BSSCFG_SAFEMODE(bsscfg) &&
	    wlc->vlan_mode != OFF) {
		struct ethervlan_header *vh = (struct ethervlan_header *)eh;

		if (!(ntoh16(vh->vlan_tag) & VLAN_VID_MASK)) {
			struct ether_header da_sa;

			PKTSETPRIO(p, (ntoh16(vh->vlan_tag) >> VLAN_PRI_SHIFT) & VLAN_PRI_MASK);

			bcopy(vh, &da_sa, VLAN_TAG_OFFSET);
			eh = (struct ether_header *)PKTPULL(osh, p, VLAN_TAG_LEN);
			ether_type = ntoh16(eh->ether_type);
			pktlen = pkttotlen(osh, p);
			bcopy(&da_sa, eh, VLAN_TAG_OFFSET);
		}
	}

	/* Make sure it is not a BRCM event pkt from the air */
	if (ether_type == ETHER_TYPE_BRCM) {
		if (is_wlc_event_frame(eh, pktlen, 0, NULL) != BCME_NOTFOUND) {
			PKTFREE(osh, p, FALSE);
			WLCNTINCR(wlc->pub->_cnt->rxbadproto);
			return;
		}
	}

	/* Update the PKTCCNT and PKTCLEN for unchained sendup cases */
	PKTCSETCNT(p, 1);
	PKTCSETLEN(p, pktlen);

	ether_dhost = (struct ether_addr *)eh->ether_dhost;
	multi = ETHER_ISMULTI(ether_dhost);

	/* forward */
	if (BSSCFG_AP(bsscfg)) {
		void *tmp;
		struct scb *dst;
		struct wlc_if *wlcif;

		if (wlc_bss_sendup_pre_filter(wlc, bsscfg, scb, p, multi) != WLC_SENDUP_CONT)
			return;

		if (wds)
			wlcif = SCB_WDS(scb);
		else
			wlcif = bsscfg->wlcif;
		ASSERT(wlcif != NULL);

		WL_APSTA_RX(("wl%d: %s: %s pkt %p da %s as AP: shost %s dhost %s\n",
			wlc->pub->unit, __FUNCTION__, (wds ? "WDS" : ""), OSL_OBFUSCATE_BUF(p),
			bcm_ether_ntoa(da, eabuf1),
			bcm_ether_ntoa((struct ether_addr*)eh->ether_shost, eabuf2),
			bcm_ether_ntoa((struct ether_addr*)eh->ether_dhost, eabuf3)));
		WL_APSTA_RX(("wl%d: %s: bsscfg %p %s iface %p\n",
			wlc->pub->unit, __FUNCTION__,
			OSL_OBFUSCATE_BUF(bsscfg), BSSCFG_AP(bsscfg) ? "(AP)" : "(STA)",
			OSL_OBFUSCATE_BUF(wlcif)));

#ifdef WET_TUNNEL
		if (WET_TUNNEL_ENAB(wlc->pub)) {
			wlc_wet_tunnel_recv_proc(wlc->wetth, p);
		}
#endif /* WET_TUNNEL */
#ifdef WLWNM_AP
		/* Do the WNM processing */
		if (BSSCFG_AP(bsscfg) && WLWNM_ENAB(wlc->pub)) {
			int ret = wlc_wnm_packets_handle(bsscfg, p, 0);
			switch (ret) {
			case WNM_TAKEN:
				/* The packet is taken by WNM return */
				return;
			case WNM_DROP:
				/* The packet drop decision by WNM free and return */
				PKTFREE(osh, p, FALSE);
				return;
			default:
				/* Continue the forwarding path */
				break;
			}
			/* update rx time stamp for BSS Max Idle Period */
			wlc_wnm_rx_tstamp_update(wlc, scb);
		}
#endif /* WLWNM_AP */

		/* forward packets destined within the BSS */
		if ((bsscfg->ap_isolate & AP_ISOLATE_SENDUP_ALL) || wds) {
			/* in ap_isolate mode, packets are passed up not forwarded */
			/* WDS endpoint, packets just go up the stack */
		} else if (multi) {
			WL_PRPKT("rxpkt (Ethernet)", (uchar *)eh, pktlen);
			WL_APSTA_RX(("wl%d: %s: forward bcmc pkt %p\n",
			             wlc->pub->unit, __FUNCTION__, OSL_OBFUSCATE_BUF(p)));
#ifdef WET_TUNNEL
			if (WET_TUNNEL_ENAB(wlc->pub)) {
				/* Process multicast frames since sender address
				 * should be writen back
				 */
				wlc_wet_tunnel_multi_packet_forward(wlc, osh, scb, wlcif, p);
			} else
#endif /* WET_TUNNEL */
			if ((bsscfg->ap_isolate & AP_ISOLATE_SENDUP_MCAST) && !ETHER_ISBCAST(da)) {
				/* This is a pure mcast packet and ap_isolate is set for multicast
				 * packets. So, the packet should only be sent up.
				 */
			} else
#ifdef WLP2P
			/* In case of P2P GO, do not send unregistered multicast packets up
			 * to the host unless allmulti is set.
			 */
			if ((BSS_P2P_ENAB(wlc, bsscfg)) && !ETHER_ISBCAST(da) &&
				!(bsscfg->allmulti || MONITOR_ENAB(wlc) ||
				PROMISC_ENAB(wlc->pub)) && wlc_bsscfg_mcastfilter(bsscfg, da)) {
				wlc_recvdata_sendpkt(wlc, p, wlcif);
				return;
			}
			else
#endif
			if ((tmp = PKTDUP(osh, p)))
				wlc_recvdata_sendpkt(wlc, tmp, wlcif);
		} else if ((eacmp(ether_dhost, &bsscfg->cur_etheraddr) != 0) &&
		           (dst = wlc_scbfind(wlc, bsscfg, ether_dhost))) {
			/* check that the dst is associated to same BSS before
			 * forwarding within the BSS
			 */

			if (SCB_ASSOCIATED(dst)) {
				WL_APSTA_RX(("wl%d: %s: forward pkt %p in BSS\n",
				             wlc->pub->unit, __FUNCTION__, OSL_OBFUSCATE_BUF(p)));
#ifdef WLCNT
				if (WME_ENAB(wlc->pub)) {
					wl_traffic_stats_t *forward =
					    &wlc->pub->_wme_cnt->forward[WME_PRIO2AC(PKTPRIO(p))];
					WLCNTINCR(forward->packets);
					WLCNTADD(forward->bytes, pkttotlen(osh, p));
				}
#endif

#if !defined(USBAP) && !defined(BCM47XX_CA9)
				if (PROMISC_ENAB(wlc->pub) && (tmp = PKTDUP(osh, p))) {
					/* both forward and send up stack */
					wlc_recvdata_sendpkt(wlc, tmp, wlcif);
#ifdef HNDCTF
					PKTSETSKIPCT(osh, p);
#endif
#if defined(PKTC) || defined(PKTC_DONGLE)
					rfc_update = FALSE;
#endif
				} else
#endif /* !USBAP && !BCM47XX_CA9 */
				{
					wlc_recvdata_sendpkt(wlc, p, wlcif);
					return;
				}
			}
		}
	}
#ifdef WLMESH
	if (WLMESH_ENAB(wlc->pub) && BSSCFG_MESH(bsscfg)) {
		if (wlc_mroute_recvdata_sendup(wlc->mesh_info, bsscfg, da, eh) == BCME_OK) {
			wlc_recvdata_sendpkt(wlc, p, bsscfg->wlcif);
			return;
		}
	}
#endif /* WLMESH */
#ifdef	WLIPFO
	/* ADHOC forwarding */
	if (BSSCFG_IBSS(bsscfg) && IPFO_ENAB(wlc->pub)) {
		WLPKTTAGSCBSET(p, scb);
		if (wlc_ipfo_forward(wlc->ipfo_info, bsscfg, p)) {
			return;
		}
	}
#endif /* WLIPFO */

	WL_PRPKT("rxpkt (Ethernet)", (uchar *)eh, pktlen);
	WL_PRUSR("rx", (uchar*)eh, ETHER_HDR_LEN);
#ifdef ENABLE_CORECAPTURE
	if (ether_type ==  ETHER_TYPE_802_1X) {
		char dst[ETHER_ADDR_STR_LEN] = {0};
		char src[ETHER_ADDR_STR_LEN] = {0};
		WIFICC_LOGDEBUG(("wl%d: %s: 802.1x dst %s src %s\n",
			wlc->pub->unit, __FUNCTION__,
			bcm_ether_ntoa((struct ether_addr*)eh->ether_dhost, dst),
			bcm_ether_ntoa((struct ether_addr*)eh->ether_shost, src)));
	}
#endif /* ENABLE_CORECAPTURE */
	WL_WSEC(("wl%d: %s: rx sendup, len %d\n", WLCWLUNIT(wlc), __FUNCTION__, pktlen));

	/* update stat counters */
	WLCNTINCR(wlc->pub->_cnt->rxframe);
	WLCNTADD(wlc->pub->_cnt->rxbyte, pktlen);
	WLPWRSAVERXFINCR(wlc);

	/* update interface stat counters */
	WLCNTINCR(SCB_WLCIFP(scb)->_cnt->rxframe);
	WLCNTADD(SCB_WLCIFP(scb)->_cnt->rxbyte, pktlen);

#ifdef WL11K
	wlc_rrm_stat_bw_counter(wlc, scb, FALSE);
#endif

	if (WME_ENAB(wlc->pub)) {
		WLCNTINCR(wlc->pub->_wme_cnt->rx[WME_PRIO2AC(PKTPRIO(p))].packets);
		WLCNTADD(wlc->pub->_wme_cnt->rx[WME_PRIO2AC(PKTPRIO(p))].bytes, pktlen);
	}

#if defined(STA) && defined(DBG_BCN_LOSS)
	scb->dbg_bcn.last_rx = wlc->pub->now;
#endif

#ifdef WLLED
	wlc_led_start_activity_timer(wlc->ledh);
#endif

#ifdef EXT_STA
	/* IBSS peer discovery */
	if (WLEXTSTA_ENAB(wlc->pub) && !bsscfg->BSS && !SCB_IS_IBSS_PEER(scb) &&
		!BSSCFG_IS_TDLS(SCB_BSSCFG(scb))) {
		SCB_SET_IBSS_PEER(scb);
		wlc_bss_mac_event(wlc, bsscfg, WLC_E_IBSS_ASSOC, &scb->ea, 0, 0,
		            DOT11_BSSTYPE_INDEPENDENT, 0, 0);
	}
#endif /* EXT_STA */

	if (ether_type == ETHER_TYPE_802_1X) {
		if (WLEIND_ENAB(wlc->pub)) {
			wlc_eapol_event_indicate(wlc, p, scb, bsscfg, eh);
		}
	}
#ifdef BCMWAPI_WAI
	else if (ether_type == ETHER_TYPE_WAI) {
		if (WLEIND_ENAB(wlc->pub)) {
			wlc_wai_event_indicate(wlc, p, scb, bsscfg, eh);
		}
	}
#endif

#ifdef MCAST_REGEN
	/* Apply WMF reverse translation only the STA side */
	if (MCAST_REGEN_ENAB(bsscfg) && !SCB_WDS(scb) && BSSCFG_STA(SCB_BSSCFG(scb))) {
		if (wlc_mcast_reverse_translation(eh) ==  BCME_OK) {
#ifdef PSTA
			/* Change bsscfg to primary bsscfg for unicast-multicast packets */
			if (PSTA_ENAB(wlc->pub) && BSSCFG_STA(bsscfg)) {
				bsscfg = wlc_bsscfg_primary(bsscfg->wlc);
			}
#endif /* PSTA */
		}
	}
#endif /* MCAST_REGEN */

#ifdef WET
	/* Apply WET only on the STA side */
	if (wlc->wet && BSSCFG_STA(bsscfg))
		wlc_wet_recv_proc(wlc->weth, p);
#endif /* WET */

#ifdef PSTA
	/* Restore the original src addr before sending up */
	if (PSTA_ENAB(wlc->pub) && BSSCFG_STA(bsscfg) &&
	    (multi || BSSCFG_PSTA(bsscfg)))
		wlc_psta_recv_proc(wlc->psta, p, eh, &bsscfg);
#endif /* PSTA */

	/* Before sending the packet up the stack, set the packet prio correctly */
	if (QOS_ENAB(wlc->pub) && (PKTPRIO(p) == 0))
		pktsetprio(p, FALSE);

	/* apply sendup filters */
	if (L2_FILTER_ENAB(wlc->pub) || TRAFFIC_MGMT_ENAB(wlc->pub))
		if (wlc_bss_sendup_post_filter(wlc, bsscfg, scb, p, multi) != WLC_SENDUP_CONT)
			return;

#if defined(PKTC) || defined(PKTC_DONGLE)
	/* Initialize the receive cache */
	if (!multi && SCB_PKTC_ENABLED(scb) && rfc_update) {
		wlc_rfc_t *rfc = PKTC_RFC(wlc->pktc_info, BSSCFG_AP(bsscfg));
		rfc->scb = scb;
		rfc->bsscfg = SCB_BSSCFG(scb);
		rfc->prio = (uint8)PKTPRIO(p);
		rfc->ac = WME_PRIO2AC(rfc->prio);
		PKTC_HSCB_SET(wlc->pktc_info, NULL);
#ifdef HNDCTF
		rfc->wlif_dev = wl_get_ifctx(wlc->wl, IFCTX_NETDEV, bsscfg->wlcif->wlif);
#endif
#ifdef PSTA
		/* Copy restored original dest address */
		eacopy(eh->ether_dhost, &rfc->ds_ea);
#endif
	}

	/* Update pps counter */
	scb->pktc_pps++;
#endif /* PKTC || PKTC_DONGLE */

	wlc_sendup(wlc, bsscfg, scb, p);
}


/* sendup data frames to the OS interface associated with the BSS or the link.
 * 'cfg' represents the BSS and 'scb' represents the link.
 * 'scb' may be NULL based on 'cfg' type.
 */
static wlc_if_t *
wlc_sendup_dest_wlcif(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct scb *scb, void *pkt)
{
	wlc_if_t *wlcif;

	ASSERT(cfg != NULL);

	if (FALSE) {
		DONOTHING();
	}
#ifdef PSTA
	else if (PSTA_ENAB(wlc->pub) && BSSCFG_STA(cfg))
		wlcif = cfg->wlcif;
#endif
#ifdef WLTDLS
	else if (TDLS_ENAB(wlc->pub) && BSSCFG_IS_TDLS(cfg)) {
		wlc_bsscfg_t *parent;

		ASSERT(scb != NULL);

		parent = wlc_tdls_get_parent_bsscfg(wlc, scb);
		ASSERT(parent != NULL);

		wlcif = parent->wlcif;
	}
#endif
	else if (scb != NULL) {
		wlcif = SCB_WLCIFP(scb);
		if (SCB_DWDS(scb)) {
			struct ether_header *eh;
			uint16 ether_type;

			eh = (struct ether_header*) PKTDATA(wlc->osh, pkt);
			ether_type = ntoh16(eh->ether_type);

			/* make sure 802.1x frames are sent on primary interface. */
			if ((ether_type == ETHER_TYPE_802_1X) ||
#ifdef BCMWAPI_WAI
			    (ether_type == ETHER_TYPE_WAI) ||
#endif /* BCMWAPI_WAI */
			    (ether_type == ETHER_TYPE_802_1X_PREAUTH)) {
				wlcif = cfg->wlcif;
			}
		}
	} else
		wlcif = cfg->wlcif;

	return wlcif;
}

void BCMFASTPATH
wlc_sendup(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct scb *scb, void *pkt)
{
	wlc_if_t *wlcif = wlc_sendup_dest_wlcif(wlc, cfg, scb, pkt);
	wl_if_t *wlif;

	ASSERT(wlcif != NULL);

	wlif = wlcif->wlif;

	WL_APSTA_RX(("wl%d: %s: sending pkt %p upto wlif %p iface %s\n",
	             wlc->pub->unit, __FUNCTION__, OSL_OBFUSCATE_BUF(pkt),
			OSL_OBFUSCATE_BUF(wlif), wl_ifname(wlc->wl, wlif)));

	wl_sendup(wlc->wl, wlif, pkt, BSSCFG_SAFEMODE(cfg)? pktsegcnt(wlc->osh, pkt) : 1);
}

void
wlc_sendup_event(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct scb *scb, void *pkt)
{
	wlc_if_t *wlcif = wlc_sendup_dest_wlcif(wlc, cfg, scb, pkt);
	wl_if_t *wlif;

	ASSERT(wlcif != NULL);

	wlif = wlcif->wlif;

	WL_APSTA_RX(("wl%d: %s: sending event %p upto wlif %p iface %s\n",
	             wlc->pub->unit, __FUNCTION__, OSL_OBFUSCATE_BUF(pkt),
			OSL_OBFUSCATE_BUF(wlif), wl_ifname(wlc->wl, wlif)));

	wl_sendup_event(wlc->wl, wlif, pkt);
}

#ifndef DONGLEBUILD
/* AMSDU monitor - calls wlc_monitor() when AMSDU data is ready to be sent up
 */
static void
wlc_monitor_amsdu(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, void *p)
{
	void *pkt = PKTDUP(wlc->osh, p);

	/* Collect AMSDU subframe packets */
	if (pkt == NULL) {
		WL_ERROR(("wlc_monitor_amsdu: no memory available\n"));
		/* Drop any previously collected data */
		if (wlc->monitor_amsdu_pkts)
			PKTFREE(wlc->osh, wlc->monitor_amsdu_pkts, FALSE);
		wlc->monitor_amsdu_pkts = NULL;
	} else {
		uint16 aggtype = (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
			wlc->pub->corerev, RxStatus2) &
			RXS_AGGTYPE_MASK) >> RXS_AGGTYPE_SHIFT;

		switch (aggtype) {
		case RXS_AMSDU_FIRST:
		case RXS_AMSDU_N_ONE:
			/* Flush any previously collected */
			if (wlc->monitor_amsdu_pkts) {
				WL_PRHDRS_MSG(("wl%d: "
					"Last AMSDU packet was not received - skip AMSDU\n",
					wlc->pub->unit));
				PKTFREE(wlc->osh, wlc->monitor_amsdu_pkts, FALSE);
				wlc->monitor_amsdu_pkts = NULL;
			}

			if (aggtype == RXS_AMSDU_FIRST) {
				/* Save the new starting AMSDU subframe */
				wlc->monitor_amsdu_pkts = pkt;
			} else {
				/* Send up all-in-one AMSDU subframe */
				wlc_monitor(wlc, wrxh, pkt, NULL);
				PKTFREE(wlc->osh, pkt, FALSE);
			}
			break;

		case RXS_AMSDU_INTERMEDIATE:
		case RXS_AMSDU_LAST:
		default:
			/* Check for previously collected */
			if (wlc->monitor_amsdu_pkts) {
				/* Append next AMSDU subframe */
				void *tail = pktlast(wlc->osh, wlc->monitor_amsdu_pkts);
				PKTSETNEXT(wlc->osh, tail, pkt);

				/* Send up complete AMSDU frame */
				if (aggtype == RXS_AMSDU_LAST) {
					wlc_monitor(wlc, wrxh, wlc->monitor_amsdu_pkts, NULL);
					PKTFREE(wlc->osh, wlc->monitor_amsdu_pkts, FALSE);
					wlc->monitor_amsdu_pkts = NULL;
				}
			} else {
				/* Out of sync, just flush */
				PKTFREE(wlc->osh, pkt, FALSE);
			}
			break;
		}
	}
}
#endif /* !DONGLEBUILD */

static void
wlc_recv_mgmt_ctl(wlc_info_t *wlc, osl_t *osh, wlc_d11rxhdr_t *wrxh, void *p)
{
	struct dot11_management_header *hdr;
	int len_mpdu, body_len, htc_len, total_frame_len;
	uint8 *plcp, *body;
	struct scb *scb = NULL;
	uint16 fc, ft, fk;
	uint16 reason;
	bool htc = FALSE;
	bool multi;
	bool tome = FALSE;
	bool short_preamble;
	/* - bsscfg_current is the bsscfg whose BSSID matches the incoming frame's BSSID
	 * - bsscfg_target is the bsscfg whose target_bss->BSSID matches the incoming frame's
	 *   BSSID
	 * - bsscfg_hwaddr is the first bsscfg whose cur_etheraddr matches the incoming
	 *   frame's DA
	 */
	wlc_bsscfg_t *bsscfg_current = NULL, *bsscfg_target = NULL, *bsscfg_hwaddr = NULL;
	/* bsscfg is a summary pointer whose value is equal to either bsscfg_current
	 * or bsscfg_target.
	 * - check bsscfg before directly reference it in cases where bsscfg_current
	 *   and bsscfg_target don't matter
	 * - check bsscfg_current and then reference bsscfg in cases where bsscfg_current
	 *   is interested
	 * - check bsscfg_target and then reference bsscfg in cases where bsscfg_target
	 *   is interested
	 */
	wlc_bsscfg_t *bsscfg = NULL;
	int rx_bandunit;
#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_ASSOC) || defined(WLEXTLOG) \
	|| defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char bss_buf[ETHER_ADDR_STR_LEN];
#endif
	bool responded = FALSE;
	int32 idx;
	wlc_bsscfg_t *cfg;
	int me;
	uint pull_len = 0;
#ifdef WLP2P
	char chanbuf[CHANSPEC_STR_LEN];
	BCM_REFERENCE(chanbuf);
#endif

	BCM_REFERENCE(me);

	WL_TRACE(("wl%d: wlc_recv_mgmt_ctl\n", wlc->pub->unit));


	plcp = PKTDATA(osh, p);
	hdr = (struct dot11_management_header*)(plcp + D11_PHY_HDR_LEN);
	ASSERT(ISALIGNED(hdr, sizeof(uint16)));
#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_ASSOC) || defined(WLEXTLOG) \
	|| defined(WLMSG_INFORM)
	bcm_ether_ntoa(&hdr->sa, eabuf);
#endif

	fc = ltoh16(hdr->fc);
	ft = FC_TYPE(fc);
	fk = (fc & FC_KIND_MASK);
#ifdef WL11N
	htc = ((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev, PhyRxStatus_0) &
		PRXS_FT_MASK(wlc->pub->corerev)) == PRXS0_PREN) &&
	       (fc & FC_ORDER) && (ft == FC_TYPE_MNG);
#endif
	ASSERT((ft == FC_TYPE_CTL) || (ft == FC_TYPE_MNG));
	short_preamble = ((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
		wlc->pub->corerev, PhyRxStatus_0) & PRXS0_SHORTH) != 0);

	total_frame_len = PKTLEN(osh, p);

	/* mac header+body length, exclude CRC and plcp header */
	len_mpdu = total_frame_len - D11_PHY_HDR_LEN - DOT11_FCS_LEN;

	/* check for runts */
	htc_len = htc ? DOT11_HTC_LEN : 0;
	if (((ft == FC_TYPE_MNG) && (len_mpdu < (DOT11_MGMT_HDR_LEN + htc_len))) ||
	    (((fk == FC_PS_POLL) || (fk == FC_BLOCKACK_REQ) || (fk == FC_BLOCKACK)) &&
	     (len_mpdu < DOT11_CTL_HDR_LEN))) {
		WLCNTINCR(wlc->pub->_cnt->rxrunt);
		goto toss_runt;
	}

	multi = ETHER_ISMULTI(&hdr->da);

	/* Is this unicast sent to me? */
	if (!multi) {
		bsscfg_hwaddr = wlc_bsscfg_find_by_hwaddr(wlc, &hdr->da);
		tome =  bsscfg_hwaddr != NULL;
	}

	if (ft == FC_TYPE_MNG) {
		struct ether_addr *bssid = &hdr->bssid;

		/* Is this in our BSS? */
		if (!multi) {
			uint chan = WLC_RX_CHANNEL(wlc->pub->corerev, &wrxh->rxhdr);
			scb = wlc_scbbssfindband(wlc, &hdr->da, &hdr->sa,
			          CHANNEL_BANDUNIT(wlc, chan),
			          &bsscfg_current);
		} else {
			/* Find scb and bss for broadcast mgmt frames (beacons, probe req, ...) */
			FOREACH_BSS(wlc, idx, cfg) {
				if (bcmp(&cfg->BSSID, &hdr->bssid, ETHER_ADDR_LEN) &&
					!BSSCFG_MESH(cfg))
					continue;
				/* Find the scb matching peer mac */
				if ((!(cfg->BSS	|| BSSCFG_MESH(cfg)) && !BSS_TDLS_ENAB(wlc, cfg)) ||
				    ((scb = wlc_scbfind(wlc, cfg, &hdr->sa)) != NULL)) {
					bsscfg_current = cfg;
					break;
				}
			}
		}

		bsscfg = bsscfg_current;

		/* Is this in the BSS we are associating to? */
		if (bsscfg == NULL) {
			bsscfg = bsscfg_target = wlc_bsscfg_find_by_target_bssid(wlc, bssid);
		}
	}

	if (MONITOR_ENAB(wlc) || PROMISC_ENAB(wlc->pub)) {

		/* In monitor or promiscuous mode
		 * don't continue processing packets that aren't for us.
		 */

		/* ctl pkts: only handle PS-Poll/BAR/BA, must be unicast */
		if ((fk == FC_PS_POLL || fk == FC_BLOCKACK_REQ || fk == FC_BLOCKACK) &&
		    tome) {
			/* accept */
		}
		/* continue to process Bcns or Probe Resp, since we are normally promisc to these */
		else if (fk == FC_BEACON || fk == FC_PROBE_RESP) {
			/* accept */
		}
		/* Management frames can match unicast DA or
		 * broadcast DA with matching BSSID or broadcast BSSID
		 */
		else if ((ft == FC_TYPE_MNG) &&
		         (tome ||
		          (ETHER_ISBCAST(&hdr->da) &&
		           (bsscfg != NULL ||
		            ETHER_ISBCAST(&hdr->bssid))))) {
			/* accept */
		}
		/* Toss otherwise */
		else
			goto toss;
	}
	else if (!multi) {
		/* a race window exists while changing the RX MAC addr match registers where
		 * we may receive a frame that does not have our MAC address
		 */
		if (!tome ||
		    /* In MBSS environment also allow frames sent to us but
		     * w/o a BSSID we know about, i.e, DEAUTH frame from WDS
		     * peer using it's AP's hwaddr as BSSID when key is out
		     * of sync.
		     */
		    (MBSS_ENAB(wlc->pub) && ft == FC_TYPE_MNG &&
		     bsscfg == NULL && bsscfg_hwaddr == NULL)) {
			WLCNTINCR(wlc->pub->_cnt->rxbadda);
			goto toss;
		}
	}

	if (!MONITOR_ENAB(wlc)) {
		wlc_bsscfg_t *mcfg = bsscfg ? bsscfg : wlc->cfg;
		if (WLC_SENDUP_MGMT_ENAB(mcfg))
			wlc_monitor(wlc, wrxh, p, mcfg->wlcif);
	}

	/* CTL Frames with missing a2 need to be tossed, else scbfind
	 * will attempt to find SCB for them
	 */
	if (ft == FC_TYPE_CTL &&
	    !(fk == FC_CTS || fk == FC_ACK) &&
	    (ETHER_ISNULLADDR(&hdr->sa) || ETHER_ISMULTI(&hdr->sa))) {
		goto toss;
	}

	/* strip off CRC and plcp */
	PKTPULL(osh, p, D11_PHY_HDR_LEN);
	PKTSETLEN(osh, p, len_mpdu);

	WLCNTINCR(wlc->pub->_cnt->rxctl);

	/* MAC is promisc wrt beacons and probe responses, trace other packets */
	if ((fk != FC_PROBE_RESP) && (fk != FC_PROBE_REQ) && (fk != FC_BEACON)) {
		WL_PRHDRS(wlc, "rxpkt hdr (ctrl/mgmt)", (uint8*)hdr, NULL, wrxh, len_mpdu +
			DOT11_FCS_LEN);
		WL_PRPKT("rxpkt (ctrl/mgmt)", (uchar*)plcp, len_mpdu + D11_PHY_HDR_LEN +
			DOT11_FCS_LEN);
	}

	body_len = PKTLEN(osh, p);	/* w/o 4 bytes CRC */
	if (wlc_recvfilter(wlc, &bsscfg, (struct dot11_header *)hdr, wrxh,
	                   &scb, body_len)) {
		WLCNTINCR(wlc->pub->_cnt->rxfilter);
		goto toss;
	}

	/* Update scb used as we just received a control frame from the STA */
	if (scb) {
		scb->used = wlc->pub->now;
#if defined(STA) && defined(DBG_BCN_LOSS)
		scb->dbg_bcn.last_tx = wlc->pub->now;
#endif

#if defined(WL_RELMCAST) && defined(AP)
		if (RMC_ENAB(wlc->pub))
			wlc_rmc_mgmtctl_rssi_update(wlc->rmc, wrxh, scb, FALSE);
#endif
	}

	/* a. ctl pkts, we only process PS_POLL/BAR/BA, others are handled in ucode */
	if (ft == FC_TYPE_CTL) {
		if (fk != FC_PS_POLL &&
#ifdef WLAMPDU
		    fk != FC_BLOCKACK_REQ && fk != FC_BLOCKACK &&
#endif /* WLAMPDU */
		    TRUE) {
			WL_ERROR(("wl%d: wlc_recv_mgmt_ctl: unhandled frametype (fk 0x%x)\n",
				wlc->pub->unit, fk));
			goto toss;
		}

		/* set pointer to frame body and set frame body length */
		body = (uchar*)PKTPULL(osh, p, sizeof(struct dot11_ctl_header));
		pull_len = sizeof(struct dot11_ctl_header);
		if (htc) {
			body = (uchar*)PKTPULL(osh, p, DOT11_HTC_LEN);
			pull_len += DOT11_HTC_LEN;
		}
		body_len = PKTLEN(osh, p);	/* w/o 4 bytes CRC */

		if (fk == FC_PS_POLL) {
			ASSERT(scb != NULL);
			if (BSSCFG_AP(scb->bsscfg) && SCB_PS(scb)) {
#ifdef WLWNM_AP
				if (WLWNM_ENAB(wlc->pub) && SCB_ASSOCIATED(scb))
					wlc_wnm_rx_tstamp_update(wlc, scb);
#endif /* WLWNM_AP */
				wlc_apps_send_psp_response(wlc, scb, fc);
			}
		}
#ifdef WLAMPDU
		else if (AMPDU_ENAB(wlc->pub) &&
		    ((fk == FC_BLOCKACK_REQ) || (fk == FC_BLOCKACK))) {
			wlc_ampdu_recv_ctl(wlc, scb, body, body_len, fk);
		}
#endif
		else {
			WL_ERROR(("wl%d: %s: unhandled frametype (0x%x)\n",
			          wlc->pub->unit, __FUNCTION__, fc));
		}
		goto toss;
	}

	/* b. mgmt pkts, ATIM, probe_request and beacon are handled in ucode first */

	rx_bandunit = CHSPEC_BANDUNIT(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
		wlc->pub->corerev, RxChan));

	/* Detect and discard duplicates */
	if (!multi) {
		uint16 seqctl = ltoh16(hdr->seq);

		if (!scb) {
			if (BSSCFG_SLOTTED_BSS(bsscfg))
				scb = wlc_scbfind_dualband(wlc, bsscfg, &hdr->sa);
			else
				scb = ((bsscfg != NULL) ? wlc_scbfindband(wlc, bsscfg,
					&hdr->sa, rx_bandunit) : NULL);
		}
		if (fc & FC_RETRY) {
			WLCNTINCR(wlc->pub->_cnt->rxrtry);
			WLCNTSCB_COND_INCR(scb, scb->scb_stats.rx_pkts_retried);
			if (scb != NULL && scb->seqctl_nonqos == seqctl) {
				WL_TRACE(("wl%d: %s: discarding duplicate MMPDU"
						" seq %04x from %s\n",
						wlc->pub->unit, __FUNCTION__,
						seqctl, eabuf));
				WLCNTINCR(wlc->pub->_cnt->rxdup);
				goto toss;
			}
		}

		/* Update cached seqctl */
		if (scb != NULL)
			scb->seqctl_nonqos = seqctl;

		/* Duplicate detection based on sa w/o creating an SCB which is heavy on memory */
		if (!scb) {
			uint8 dup_idx = 0;
			struct sa_seqctl *ea_seq = NULL;

			/* Look up the table for if matching ea already exists */
			for (; dup_idx <= wlc->ctl_dup_det_idx; dup_idx++) {
				/* Reached empty entry -- bail out */
				if (ETHER_ISNULLADDR(&wlc->ctl_dup_det[dup_idx].sa))
					break;

				/* Match found */
				if (!bcmp(hdr->sa.octet, wlc->ctl_dup_det[dup_idx].sa.octet,
				          ETHER_ADDR_LEN)) {
					ea_seq = &wlc->ctl_dup_det[dup_idx];
					break;
				}
			}

			if (fc & FC_RETRY) {
				if (ea_seq != NULL && ea_seq->seqctl_nonqos == seqctl) {
					WL_TRACE(("wl%d: %s: discarding duplicate MMPDU using"
							" ea seq %04x from %s\n",
							wlc->pub->unit, __FUNCTION__,
							seqctl, eabuf));
					WLCNTINCR(wlc->pub->_cnt->rxdup);
					goto toss;
				}
			}

			/* Create the entry since one didn't exist */
			if (!ea_seq) {
				ea_seq = &wlc->ctl_dup_det[wlc->ctl_dup_det_idx++];
				bcopy((const char*)&hdr->sa, (char*)&ea_seq->sa, ETHER_ADDR_LEN);
				wlc->ctl_dup_det_idx %= SA_SEQCTL_SIZE;
			}
			ea_seq->seqctl_nonqos = seqctl;
		}
	}

#ifdef STA
	/* Check if this was response to PSpoll sent by associated AP to STA */
	if (tome &&
	    bsscfg_current != NULL && BSSCFG_STA(bsscfg) &&
	    bsscfg->pm->PSpoll)
	        wlc_recv_PSpoll_resp(bsscfg, fc);
#endif

	/* set pointer to frame body and set frame body length */
	body = (uchar*)PKTPULL(osh, p, sizeof(struct dot11_management_header));
	pull_len = sizeof(struct dot11_management_header);
	if (htc) {
		body = (uchar*)PKTPULL(osh, p, DOT11_HTC_LEN);
		pull_len += DOT11_HTC_LEN;
	}
	body_len = PKTLEN(osh, p);	/* w/o 4 bytes CRC */

#if defined(BCMCCX) && defined(CCX_SDK)
	if (CCX_ENAB(wlc->pub) && IHV_ENAB(wlc->ccx, bsscfg) && bsscfg_current != NULL &&
	    BSSCFG_STA(bsscfg) && bsscfg->BSS && bsscfg->associated &&
	    (fk == FC_DEAUTH || fk == FC_DISASSOC || fk == FC_ACTION)) {

		/* no broadcast management frames transmitted from CCXv5 network */
		if (bsscfg->current_bss->ccx_version >= 4 && ETHER_ISBCAST(&hdr->da)) {
			if (fk == FC_DEAUTH)
				WLCNTINCR(wlc->ccx->mgmt_cnt.bcastmgmtdeauth);
			else if (fk == FC_DISASSOC)
				WLCNTINCR(wlc->ccx->mgmt_cnt.bcastmgmtdisassoc);
			else if (fk == FC_ACTION)
				WLCNTINCR(wlc->ccx->mgmt_cnt.bcastmgmtaction);
			WL_ERROR(("wl%d: %s: discard broadcast mgmt frame from %s\n",
				WLCWLUNIT(wlc), __FUNCTION__, bcm_ether_ntoa(&hdr->sa, bss_buf)));
			goto toss;
		}
		if (!scb)
			scb = wlc_scbfindband(wlc, bsscfg, &hdr->sa, rx_bandunit);
		if (SCB_CCX_MFP(scb)) {
			WLPKTTAGSCBSET(p, scb);
			if (wlc_ccx_mfp_rx(wlc->ccx, wrxh, hdr, p, scb) != BCME_OK)
				goto toss;
			body = (uchar*)PKTDATA(osh, p);
			body_len = PKTLEN(osh, p);
			goto skip_11w;
		}
	}
#endif /* BCMCCX && CCX_SDK */

#ifdef MFP
	if (scb != NULL && WLC_MFP_ENAB(wlc->pub)) {
		WLPKTTAGSCBSET(p, scb);
		if (!wlc_mfp_rx(wlc->mfp, bsscfg, scb, &wrxh->rxhdr, hdr, p))
			goto toss;
		body = (uchar*)PKTDATA(osh, p);
		body_len = PKTLEN(osh, p);
	} else
#endif /* MFP */
	/* only auth  mgmt frame seq 3  can be encrypted(WEP) */
	/* deal with encrypted frame */
	if (fc & FC_WEP) {
		if (body_len < WLC_WEP_IV_SIZE + WLC_WEP_ICV_SIZE) {
			WLCNTINCR(wlc->pub->_cnt->rxrunt);
			goto toss;
		}

		if (fk != FC_AUTH) {
			WLCNTINCR(wlc->pub->_cnt->rxundec);
			goto toss;
		}
		/*
		   decryption error is now checked in authresp_ap so we can
		   check if it was supposed to use a different key for another ssid.
		*/
	}

#ifdef WLWNM_AP
	if (bsscfg && BSSCFG_AP(bsscfg) && WLWNM_ENAB(wlc->pub) && scb && SCB_ASSOCIATED(scb))
		wlc_wnm_rx_tstamp_update(wlc, scb);
#endif /* WLWNM_AP */

#if defined(BCMCCX) && defined(CCX_SDK)
skip_11w:
#endif /* BCMCCX && CCX_SDK */

	switch (fk) {

	case FC_AUTH:
		WL_MBSS(("RX: Auth pkt on BSS %s\n", bcm_ether_ntoa(&hdr->bssid, bss_buf)));

		if (body_len < DOT11_AUTH_FIXED_LEN)
			goto rxbadproto;

		/* Auth seq 1 frames are received by AP only. So we can do bsscfg lookup
		 * using bssid and are sure to find unique one.
		 */
		if (ltoh16(((struct dot11_auth *)body)->seq) == 1)
			bsscfg = wlc_bsscfg_find_by_bssid(wlc, &hdr->bssid);

#ifdef WLMESH
		if (WLMESH_ENAB(wlc->pub) && (bsscfg == NULL)) {
			bsscfg = wlc_bsscfg_find_by_hwaddr(wlc, &hdr->da);
			if ((bsscfg) && !BSSCFG_MESH(bsscfg)) {
				bsscfg = NULL;
			}
		}
#endif

		if (bsscfg == NULL) {
			WL_ASSOC(("wl%d: %s: FC_AUTH: no bsscfg for BSS %s\n",
				WLCWLUNIT(wlc), __FUNCTION__,
				bcm_ether_ntoa(&hdr->bssid, bss_buf)));
			break;  /* we couldn't match the incoming frame to a BSS config */
		}

		/* Is this request for an AP config or a STA config */
		if (BSSCFG_AP(bsscfg) && bsscfg->up) {
#ifdef EXT_STA
			if (WLEXTSTA_ENAB(wlc->pub)) {
				/* for best effort to abort low priority scans */
				wlc_mac_event(wlc, WLC_E_AUTH_REQ, NULL, 0, 0, 0, NULL, 0);
			}
#endif /* EXT_STA */

#ifdef MFP
			if (WLC_MFP_ENAB(wlc->pub) && SCB_MFP(scb) && SCB_AUTHENTICATED(scb)) {
				/* send auth packet */
				wlc_sendauth(bsscfg, &scb->ea, &bsscfg->target_bss->BSSID, scb,
					DOT11_OPEN_SYSTEM, 2, DOT11_SC_SUCCESS,
					NULL, DOT11_CAP_SHORT);
			}
			else
#endif /* MFP */
			{
#ifdef AP
			/* pkt data points to body - must point to mgmt header */
			PKTPUSH(osh, p, pull_len);
			wlc_ap_authresp(wlc->ap, bsscfg, hdr, body, body_len,
					p, short_preamble, &wrxh->rxhdr);
			PKTPULL(osh, p, pull_len);
#endif
			}
		} else if (BSSCFG_INFRA_STA(bsscfg)) {
#ifdef STA
			wlc_authresp_client(bsscfg, hdr, body, body_len, short_preamble);
#endif
		} else {
			WL_ASSOC(("wl%d: %s: FC_AUTH: unknown bsscfg ap %d BSS %d\n",
				WLCWLUNIT(wlc), __FUNCTION__, BSSCFG_AP(bsscfg), bsscfg->BSS));
		}
		break;

#ifdef STA
	case FC_ASSOC_RESP:
	case FC_REASSOC_RESP:

		WL_MBSS(("RX: (Re)AssocResp pkt on BSS %s\n",
		         bcm_ether_ntoa(&hdr->bssid, bss_buf)));

		if (body_len < DOT11_ASSOC_RESP_FIXED_LEN)
			goto rxbadproto;

		if (bsscfg == NULL) {
			WL_ASSOC(("wl%d: %s: FC_(RE)ASSOC_RESP: no bsscfg for BSS %s\n",
				WLCWLUNIT(wlc), __FUNCTION__,
				bcm_ether_ntoa(&hdr->bssid, bss_buf)));
			break;  /* we couldn't match the incoming frame to a BSS config */
		}
		if (!BSSCFG_STA(bsscfg)) {
			WL_ASSOC(("wl%d: %s: FC_(RE)ASSOC_RESP: bsscfg for BSS %s is an "
				"AP\n", WLCWLUNIT(wlc),
				__FUNCTION__, bcm_ether_ntoa(&hdr->bssid, bss_buf)));
			break;
		}

		wlc_assocresp_client(bsscfg, scb, hdr, body, body_len);
		break;
#endif /* STA */

	case FC_PROBE_REQ:
		/* forward probe requests to higher level  */

#ifdef BCMDBG
		wlc_update_perf_stats(wlc, WLC_PERF_STATS_PRB_REQ);
#endif

		if (wlc_eventq_test_ind(wlc->eventq, WLC_E_PROBREQ_MSG))
			wlc_bss_mac_event(wlc, wlc->cfg, WLC_E_PROBREQ_MSG, &hdr->sa,
				0, 0, 0, (char *)hdr, body_len + DOT11_MGMT_HDR_LEN);

		if (wlc_eventq_test_ind(wlc->eventq, WLC_E_PROBREQ_MSG_RX)) {
			wl_event_rx_frame_data_t rxframe_data;
			wlc_recv_prep_event_rx_frame_data(wlc, wrxh, plcp, &rxframe_data);
			wlc_bss_mac_rxframe_event(wlc, wlc->cfg, WLC_E_PROBREQ_MSG_RX, &hdr->sa,
				0, 0, 0, (char *)hdr, body_len + DOT11_MGMT_HDR_LEN, &rxframe_data);
		}
		/* Probe responses are handled thru the PRQ Fifo */

		/* generic prbreq processing */
		wlc_recv_process_prbreq(wlc, wrxh, plcp, hdr, body, body_len);

#ifdef WLP2P
		WL_NONE(("wl%d: probe req frame received from %s on chanspec %s, seqnum %d\n",
		         wlc->pub->unit, bcm_ether_ntoa(&hdr->sa, eabuf),
		         wf_chspec_ntoa_ex(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
		         wlc->pub->corerev, RxChan), chanbuf),
		         (ltoh16(hdr->seq) >> SEQNUM_SHIFT)));

		if (P2P_ENAB(wlc->pub))
			responded = wlc_p2p_recv_process_prbreq(wlc->p2p, hdr, body,
				body_len, wrxh, plcp, FALSE);
#endif /* WLP2P */
		if (!responded)
			wlc_probresp_recv_process_prbreq(wlc->mprobresp, wrxh,
			        plcp, hdr, body, body_len);
		break;

	case FC_PROBE_RESP:

#ifdef BCMDBG
		wlc_update_perf_stats(wlc, WLC_PERF_STATS_PRB_RESP);
#endif
		WL_MBSS(("RX: Probe resp on BSS %s\n", bcm_ether_ntoa(&hdr->bssid, bss_buf)));
		if (body_len < DOT11_BCN_PRB_LEN)
			goto rxbadproto;

		/* if there is any restricted chanspec in the locale, convert the channel */
		/* receiving the probe_resp to active channel if needed */
		if (CLEAR_RESTRICTED_CHANNEL_ENAB(wlc->pub) &&
			(wlc->passive_on_restricted == WL_PASSACTCONV_DISABLE_NONE) &&
			wlc_has_restricted_chanspec(wlc->cmi))
			wlc_convert_restricted_chanspec(wlc, wrxh, hdr, body, body_len);

		/* generic prbrsp processing */
		wlc_recv_process_prbresp(wlc, wrxh, plcp, hdr, body, body_len);

#ifdef WLP2P
#if defined(BCMDBG)
		WL_NONE(("wl%d: probe resp frame received from %s on chanspec %s, seqnum %d\n",
		        wlc->pub->unit, bcm_ether_ntoa(&hdr->sa, eabuf),
		        wf_chspec_ntoa_ex(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
		        wlc->pub->corerev, RxChan), chanbuf),
			(ltoh16(hdr->seq) >> SEQNUM_SHIFT)));
#endif
		if (P2P_ENAB(wlc->pub))
			wlc_p2p_recv_process_prbresp(wlc->p2p, body, body_len);
#endif /* WLP2P */

		/* check for our probe responses:
		 * - must be a packet with our MAC addr
		 * - or may have broadcast MAC addr if it has OCE
		 * (but if we're no longer scanning, ignore it)
		 */
		if (SCAN_IN_PROGRESS(wlc->scan) &&
			(tome ||
#ifdef WL_OCE
			(multi && (OCE_ENAB(wlc->pub))) ||
#endif
			FALSE)) {
				wlc_recv_scan_parse(wlc, wrxh, plcp, hdr, body, body_len);
		}
#if defined(BCMCCX) && defined(STA)
		else if (CCX_ENAB(wlc->pub) && wlc->rm_info->rm_state->ccx->scan_active)
			wlc_recv_scan_parse(wlc, wrxh, plcp, hdr, body, body_len);
#endif /* BCMCCX && STA */
		break;

	case FC_BEACON: {
#ifdef BCMDBG
		wlc_update_perf_stats(wlc, WLC_PERF_STATS_BCN_ISR);
#endif
		wlc_recv_bcn(wlc, osh, bsscfg_current, bsscfg_target,
			wrxh, plcp, total_frame_len, TRUE);

		break;
	}

	case FC_DISASSOC:
		WL_MBSS(("RX: Disassoc on BSS %s\n", bcm_ether_ntoa(&hdr->bssid, bss_buf)));
		if (body_len < 2)
			goto rxbadproto;

		if (bsscfg == NULL) {
			WL_ASSOC(("wl%d: %s: FC_DISASSOC: no bsscfg for BSS %s\n",
				WLCWLUNIT(wlc), __FUNCTION__,
				bcm_ether_ntoa(&hdr->bssid, bss_buf)));
			break;  /* we couldn't match the incoming frame to a BSS config */
		}

		/* make sure the disassoc frame is addressed to the MAC for this bsscfg */
		if (!ETHER_ISMULTI(&hdr->da) &&
			(eacmp(hdr->da.octet, bsscfg->cur_etheraddr.octet) != 0)) {
			WL_ASSOC(("wl%d: %s: FC_DISASSOC: bsscfg mismatch BSSID. "
				"Ignore Disassoc frame to %s", WLCWLUNIT(wlc), __FUNCTION__,
				bcm_ether_ntoa(&hdr->da, bss_buf)));
			break;
		}

		reason = ltoh16(*(uint16*)body);

		if (BSS_SMFS_ENAB(wlc, bsscfg)) {
			(void)wlc_smfs_update(wlc->smfs, bsscfg, SMFS_TYPE_DISASSOC_RX, reason);
		}

		/* wlc_recvfilter(): frame tossed if node not authenticated with us */
		ASSERT(scb && SCB_AUTHENTICATED(scb));

		WLC_EXTLOG(wlc, LOG_MODULE_ASSOC, FMTSTR_DISASSOC_ID,
		           WL_LOG_LEVEL_ERR, 0, reason, eabuf);

		if (BSSCFG_STA(bsscfg)) {
			wlc_scb_disassoc_cleanup(wlc, scb);

			if (!(scb->flags & SCB_MYAP)) {
				WL_ERROR(("wl%d: %s: STA DISASSOC from non-AP SCB %s\n",
					wlc->pub->unit, __FUNCTION__, eabuf));
			}
#ifdef STA
			/* STA */
			WL_ASSOC(("wl%d: %s%s from %s (reason %d)\n", wlc->pub->unit,
				ETHER_ISBCAST(&hdr->da) ? "broadcast " : "",
				  "disassociation", eabuf, reason));
			if (SCB_ASSOCIATED(scb)) {
				wlc_scb_clearstatebit(wlc, scb, ASSOCIATED);
				if (!(scb->state & PENDING_ASSOC))
					wlc_disassoc_ind_complete(wlc, bsscfg, WLC_E_STATUS_SUCCESS,
						&hdr->sa, reason, 0, body, body_len);
			}


			/* allow STA to roam back to disassociating AP */
			if (bsscfg_current != NULL) {
				if (bsscfg->assoc->state != AS_IDLE)
					wlc_assoc_abort(bsscfg);

#if defined(WLP2P) && !defined(WLMCHAN)
				if (!BSS_P2P_ENAB(wlc, bsscfg))
#endif
					wlc_roamscan_start(bsscfg, WLC_E_REASON_DISASSOC);
			}
#endif /* STA */
		} else if (scb->bsscfg == bsscfg) { /* AP */
			/* Only clean up if this SCB is still pointing to the BSS config to
			 * which the packet was destined.
			 */
			wlc_scb_disassoc_cleanup(wlc, scb);

			if (SCB_ASSOCIATED(scb)) {
				WL_ASSOC(("DISASSOC: wl%d: %s disassociated (reason %d)\n",
				          wlc->pub->unit, eabuf, reason));
				/* return node to state 2 */
				wlc_scb_clearstatebit(wlc, scb, ASSOCIATED | AUTHORIZED);
				wlc_disassoc_ind_complete(wlc, bsscfg, WLC_E_STATUS_SUCCESS,
					&hdr->sa, reason, 0, body, body_len);
				/* If the STA has been configured to be monitored before
				 * association then continue sniffing that STA frames.
				 */
				if (STAMON_ENAB(wlc->pub) && STA_MONITORING(wlc, &hdr->sa))
					wlc_stamon_sta_sniff_enab(wlc->stamon_info, &hdr->sa, TRUE);
#ifdef DONGLEBUILD
#ifdef WLP2P
				if (!(BSSCFG_AP(bsscfg) && BSS_P2P_ENAB(wlc, bsscfg)))
#endif /* WLP2P */
					wlc_scbfree(wlc, scb);
#endif /* DONGLEBUILD */
			} else {
				WL_ASSOC(("DISASSOC: wl%d: rcvd disassociation from unassociated "
				          "STA %s (reason %d)\n", wlc->pub->unit, eabuf, reason));
				wlc_scb_clearstatebit(wlc, scb, AUTHORIZED);
			}
		}
		break;

	case FC_DEAUTH: {
		wlc_bsscfg_t *bsscfg_deauth;

		WL_MBSS(("RX: Deauth on BSS %s\n", bcm_ether_ntoa(&hdr->bssid, bss_buf)));
		if (body_len < 2)
			goto rxbadproto;

		/* Allow it in case the frame is for us but w/o a BSSID that we know
		 * about, i.e, frame from WDS peer using it's AP's hwaddr as the BSSID.
		 */
		if ((bsscfg_deauth = bsscfg) == NULL &&
		    (bsscfg_deauth = bsscfg_hwaddr) == NULL) {
			WL_ASSOC(("wl%d: %s: FC_DEAUTH: no bsscfg for BSS %s\n",
				WLCWLUNIT(wlc), __FUNCTION__,
				bcm_ether_ntoa(&hdr->bssid, bss_buf)));
			break;
		}

		/* make sure the deauth frame is addressed to the MAC for this bsscfg */
		if (!ETHER_ISMULTI(&hdr->da) &&
			(eacmp(hdr->da.octet, bsscfg_deauth->cur_etheraddr.octet) != 0)) {
			WL_ASSOC(("wl%d: %s: FC_DEAUTH: bsscfg mismatch BSSID. "
				"Ignore Deauth frame to %s", WLCWLUNIT(wlc), __FUNCTION__,
				bcm_ether_ntoa(&hdr->da, bss_buf)));
			break;
		}

		reason = ltoh16(*(uint16*)body);

		if (bsscfg &&
		    BSS_SMFS_ENAB(wlc, bsscfg)) {
			(void)wlc_smfs_update(wlc->smfs, bsscfg, SMFS_TYPE_DEAUTH_RX, reason);
		}

		if (!scb)
			scb = wlc_scbfindband(wlc, bsscfg_deauth, &hdr->sa, rx_bandunit);
		if (scb)
			wlc_scb_disassoc_cleanup(wlc, scb);

		WLC_EXTLOG(wlc, LOG_MODULE_ASSOC, FMTSTR_DEAUTH_ID,
		           WL_LOG_LEVEL_ERR, 0, reason, eabuf);

#ifdef WLTDLS
		/*  recv DEAUTH packet, disconnect the peer */
		if (scb && TDLS_ENAB(wlc->pub) && BSSCFG_IS_TDLS(scb->bsscfg)) {
			tdls_iovar_t setup_info;

			memset(&setup_info, 0, sizeof(tdls_iovar_t));
			bcopy(&hdr->sa, &setup_info.ea, ETHER_ADDR_LEN);
			setup_info.mode = TDLS_MANUAL_EP_DELETE;
			WL_TDLS(("wl%d:%s(): Received DEAUTH from TDLS peer %s, del endp"
				"setup with peer.\n", wlc->pub->unit, __FUNCTION__,
				bcm_ether_ntoa(&hdr->sa, eabuf)));
			wlc_iovar_op(wlc, "tdls_endpoint", NULL, 0, &setup_info,
				sizeof(tdls_iovar_t), IOV_SET, scb->bsscfg->wlcif);
		} else
#endif /* WLTDLS */
		if (BSSCFG_STA(bsscfg_deauth)) {
			if (scb && !(scb->flags & SCB_MYAP) && !SCB_IS_IBSS_PEER(scb)) {
				WL_ERROR(("wl%d: %s: STA DEAUTH from "
				          "non-AP SCB %s\n", wlc->pub->unit, __FUNCTION__, eabuf));
			}
#ifdef STA
			/* STA */
			if (scb) {
				WL_ASSOC(("wl%d: %s%s from %s (reason %d)\n", wlc->pub->unit,
					!ETHER_ISUCAST(&hdr->da) ? "non-unicast " : "",
					  "deauthentication", eabuf, reason));
				wlc_scb_clearstatebit(wlc, scb, ASSOCIATED | AUTHORIZED);
				if (SCB_AUTHENTICATED(scb) || SCB_IS_IBSS_PEER(scb)) {
					wlc_scb_clearstatebit(wlc, scb, AUTHENTICATED);
					/* DEAUTH w/reason 13-22 is considered a 4-way handshake
					 * failure
					 */
					if (reason > 12 && reason < 23)
						WLCNTINCR(wlc->pub->_cnt->fourwayfail);
#ifdef WLEXTLOG
					if (reason == DOT11_RC_4WH_TIMEOUT) {
						WLC_EXTLOG(wlc, LOG_MODULE_ASSOC,
							FMTSTR_PWD_MISMATCH_ID,
							WL_LOG_LEVEL_ERR, 0, 0, NULL);
					}
#endif
#if defined(BCMSUP_PSK)
					if (SUP_ENAB(wlc->pub)) {
						wlc_wpa_send_sup_status(wlc->idsup, bsscfg_deauth,
						                      WLC_E_SUP_DEAUTH);
					}
#endif /* defined(BCMSUP_PSK) */
					if (!(scb->state & PENDING_AUTH))
						wlc_deauth_ind_complete(wlc, bsscfg_deauth,
							WLC_E_STATUS_SUCCESS,
							&hdr->sa, reason,
							(SCB_IS_IBSS_PEER(scb) ?
						         DOT11_BSSTYPE_INDEPENDENT :
						         DOT11_BSSTYPE_INFRASTRUCTURE),
						        body, body_len);

					SCB_UNSET_IBSS_PEER(scb);
				}
			} else {
				WL_ASSOC(("wl%d: Invalid %s%s from %s (reason %d). %s\n",
					wlc->pub->unit,
					!ETHER_ISUCAST(&hdr->da) ? "non-unicast " : "",
					  "deauthentication", eabuf, reason,
					  "Ignoring (not part of our BSS)\n"));

			}


			/* allow STA to roam back to deauthenticating AP */
			if (bsscfg_current != NULL && bsscfg->BSS) {
				/* do not abort roaming */
				if ((bsscfg->assoc->type != AS_ROAM) ||
				    (bsscfg->assoc->state < AS_SENT_ASSOC))  {
					if (bsscfg->assoc->state != AS_IDLE)
						wlc_assoc_abort(bsscfg);
#ifdef PSTA
					/* Cleanup proxy bss state to allow start
					 * of new association.
					 */
					if (PSTA_ENAB(wlc->pub) && BSSCFG_PSTA(bsscfg)) {
						wlc_disassociate_client(bsscfg, FALSE);
						wlc_bsscfg_disable(wlc, bsscfg);
						wlc_bsscfg_free(wlc, bsscfg);
					} else
#endif /* PSTA */

#if defined(WLP2P) && !defined(WLMCHAN)
					if (!BSS_P2P_ENAB(wlc, bsscfg))
#endif
						wlc_roamscan_start(bsscfg, WLC_E_REASON_DEAUTH);
				}
			}
#endif /* STA */
		} else if ((scb != NULL) && (scb->bsscfg == bsscfg_deauth)) { /* AP */
			if (SCB_LEGACY_WDS(scb)) {
				WL_ASSOC(("wl%d: %s: FC_DEAUTH frame over WDS link %s\n",
				          WLCWLUNIT(wlc), __FUNCTION__, eabuf));
				scb->flags &= ~SCB_WDS_LINKUP;
			}
			/* Only clean up if this SCB is still pointing to the BSS config to
			 * which the packet was destined.
			 */
			if (SCB_AUTHENTICATED(scb)) {
				WL_ASSOC(("wl%d: %s deauthenticated (reason %d)\n",
				          wlc->pub->unit, eabuf, reason));
				wlc_scb_clearstatebit(wlc, scb, AUTHENTICATED);
				wlc_deauth_ind_complete(wlc, bsscfg_deauth, WLC_E_STATUS_SUCCESS,
					&hdr->sa, reason, 0, body, body_len);
				/* remove all known state on this node (deauthenticate it) */
				wlc_scbfree(wlc, scb);
				/* If the STA has been configured to be monitored before
				 * association then continue sniffing that STA frames.
				 */
				if (STAMON_ENAB(wlc->pub) && STA_MONITORING(wlc, &hdr->sa))
					wlc_stamon_sta_sniff_enab(wlc->stamon_info, &hdr->sa, TRUE);
			}
		}
		break;
	}

	case FC_ASSOC_REQ:
	case FC_REASSOC_REQ:
		WL_MBSS(("RX: (Re)Assoc on from %s on BSS %s\n", bcm_ether_ntoa(&hdr->sa, eabuf),
			bcm_ether_ntoa(&hdr->bssid, bss_buf)));
		if (body_len < 4)
			goto rxbadproto;

		if (bsscfg == NULL) {
			WL_ASSOC(("wl%d: %s: FC_(RE)ASSOC_REQ: no bsscfg for BSS %s\n",
				WLCWLUNIT(wlc), __FUNCTION__,
				bcm_ether_ntoa(&hdr->bssid, bss_buf)));
			break;
		}

		ASSERT(scb && SCB_AUTHENTICATED(scb));

		if (!bsscfg->up || !BSSCFG_AP(bsscfg))
			break;

		/* Check that this (re)associate is for us */
		if (!bcmp(hdr->bssid.octet, bsscfg->BSSID.octet, ETHER_ADDR_LEN))
			wlc_ap_process_assocreq(wlc->ap, bsscfg, hdr, body, body_len, scb,
				short_preamble);
		break;

	case FC_ACTION:
		wlc_recv_mgmtact(wlc, scb, hdr, body, body_len, wrxh, plcp);
		break;

#ifdef WLAIBSS
	case FC_ATIM:
		if (bsscfg == NULL) {
			WL_PS(("wl%d: %s: FC_ATIM: no bsscfg for IBSS %s\n",
				WLCWLUNIT(wlc), __FUNCTION__,
				bcm_ether_ntoa(&hdr->bssid, bss_buf)));
			break;
		}

		if (BSSCFG_IBSS(bsscfg) && AIBSS_ENAB(wlc->pub) && AIBSS_BSS_PS_ENAB(bsscfg)) {
			WL_PS(("wl%d: %s: FC_ATIM: From %s\n", WLCWLUNIT(wlc), __FUNCTION__,
				bcm_ether_ntoa(&hdr->sa, bss_buf)));

			wlc_aibss_ps_process_atim(wlc, &hdr->sa);
		}
		break;
#endif /* WLAIBSS */

	rxbadproto:
		WLCNTINCR(wlc->pub->_cnt->rxbadproto);

	default:
		break;
	}

toss:
	/* Handle error case here */

toss_runt:
	PKTFREE(osh, p, FALSE);
}

/* Convert RX hardware status to standard format and send to wl_monitor
 * assume p points to plcp header
 */
static void
wlc_monitor(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, void *p, struct wlc_if *wlcif)
{
#ifndef DONGLEBUILD
	wl_rxsts_t sts;
	uint32 rx_tsf_l;
	ratespec_t rspec;
	uint16 chan_num;
	uint8 *plcp;
	rx_lq_samp_t sample;

	ASSERT(p != NULL);
	BCM_REFERENCE(chan_num);

	rx_tsf_l = wlc_recover_tsf32(wlc, wrxh);

	bzero((void *)&sts, sizeof(wl_rxsts_t));

	sts.mactime = rx_tsf_l;

	/**
	 * TBD: Handle corerev >= 80 formats (to be clubbed with
	 * rest of DHD & FW changes for supporting 11ax (>=rev80)
	 * monitor mode).
	 */
	if (D11REV_LT(wlc->pub->corerev, 80)) {
		sts.antenna = (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
			wlc->pub->corerev, PhyRxStatus_0) &
			PRXS0_RXANT_UPSUBBAND) ? 1 : 0;

		sts.sq = ((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
			wlc->pub->corerev, PhyRxStatus_1) &
			PRXS1_SQ_MASK) >> PRXS1_SQ_SHIFT);
	}

#if defined(WL_MONITOR) && defined(WLTEST)
	/* special rssi sampling without peer address */
	wlc_lq_monitor_sample(wlc, wrxh, &sample);
#else
	sample.rssi = wlc_lq_recv_rssi_compute(wlc, wrxh);
#endif
	sts.signal = sample.rssi;

	sts.chanspec = D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev, RxChan);
	chan_num = wf_chspec_ctlchan(sts.chanspec);

	sts.noise = phy_noise_avg((wlc_phy_t *)
		WLC_PI_BANDUNIT(wlc, CHSPEC_WLCBANDUNIT(sts.chanspec)));

	plcp = PKTDATA(wlc->osh, p);
	rspec = wlc_recv_compute_rspec(wlc->d11_info, wrxh, plcp);

	wlc_ht_monitor(wlc->hti, wrxh, plcp, rspec, &sts);
	if (!wlc_vht_prep_rate_info(wlc->vhti, wrxh, plcp, rspec, &sts) &&
		RSPEC_ISHT(rspec)) {
		wlc_ht_prep_rate_info(wlc->hti, wrxh, rspec, &sts);
	} else {
		/* round non-HT data rate to nearest 500bkps unit */
		sts.datarate = RSPEC2KBPS(rspec)/500;
	}

	sts.pktlength = D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
		wlc->pub->corerev, RxFrameSize) - D11_PHY_HDR_LEN;

	if (CHSPEC_IS2G(sts.chanspec)) {
		sts.phytype = wlc->bandstate[BAND_2G_INDEX]->phytype;
	} else {
		sts.phytype = wlc->bandstate[BAND_5G_INDEX]->phytype;
	}


#if (PHY_TYPE_N != WL_RXS_PHY_N)
#error "phy type macro mismatch PHY_TYPE_X and WL_RXS_PHY_X"
#endif

	/* phytype should be one of WL_RXS_PHY_[BANG] */
	/* map from one set of defines to another */
	switch (sts.phytype) {
		case PHY_TYPE_HT:
			sts.phytype = WL_RXS_PHY_N;
			break;
		case PHY_TYPE_N:
/*
		no-op -- above compile time check ensures that is correct
*/
			break;
		default:
			sts.phytype = WL_RXS_PHY_N;
			break;
	}

	if (RSPEC_ISCCK(rspec)) {
		sts.encoding = WL_RXS_ENCODING_DSSS_CCK;
		sts.preamble = (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
			wlc->pub->corerev, PhyRxStatus_0) & PRXS0_SHORTH) ?
			WL_RXS_PREAMBLE_SHORT : WL_RXS_PREAMBLE_LONG;
	} else if (RSPEC_ISOFDM(rspec)) {
		sts.encoding = WL_RXS_ENCODING_OFDM;
		sts.preamble = WL_RXS_PREAMBLE_SHORT;
	}
#ifdef WL11N
	else {	/* MCS rate */
		sts.encoding = WL_RXS_ENCODING_HT;
		if (RSPEC_ISVHT(rspec))
			sts.encoding = WL_RXS_ENCODING_VHT;
		if (WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) {
			sts.preamble = (D11HT_MMPLCPLen(&wrxh->rxhdr) != 0) ?
				WL_RXS_PREAMBLE_HT_MM : WL_RXS_PREAMBLE_HT_GF;
		} else {
			sts.preamble = (D11N_MMPLCPLen(&wrxh->rxhdr) != 0) ?
				WL_RXS_PREAMBLE_HT_MM : WL_RXS_PREAMBLE_HT_GF;
		}
	}
#endif
	/* translate error code */
	if (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev, RxStatus1) & RXS_DECERR)
		sts.pkterror |= WL_RXS_DECRYPT_ERR;
	if (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev, RxStatus1) & RXS_FCSERR)
		sts.pkterror |= WL_RXS_CRC_ERROR;

	/* If it's a runt frame or with FCS error, don't process it further */
	if (!((uint)PKTLEN(wlc->osh, p) >= D11_PHY_HDR_LEN + sizeof(uint16)) ||
	    (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev, RxStatus1) & RXS_FCSERR))
		goto sendup;

#if defined(BCMCCX) && defined(CCX_SDK)
	if (IHV_ENAB(wlc->ccx, wlc->cfg) && wlc->ccx->frame_log)
		wlc_ccx_log_rx_frame(wlc->ccx, (uint8*)PKTDATA(wlc->osh, p) + D11_PHY_HDR_LEN,
			PKTLEN(wlc->osh, p) - D11_PHY_HDR_LEN, &sts);
#endif /* BCMCCX && CCX_SDK */

sendup:

	wl_monitor(wlc->wl, &sts, p);
#else /* DONGLEBUILD */
#if defined(WL_MONITOR) && defined(WLTEST)
	/* special rssi sampling without peer address */
	rx_lq_samp_t sample;
	wlc_lq_monitor_sample(wlc, wrxh, &sample);
	BCM_REFERENCE(sample);
#endif
	if (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev, RxStatus1) & RXS_PBPRES)
		PKTPUSH(wlc->osh, p, 2);
	wl_monitor(wlc->wl, NULL, p);
#endif /* DONGLEBUILD */
}

static void
wlc_recv_bcn(wlc_info_t *wlc, osl_t *osh, wlc_bsscfg_t *bsscfg_current, wlc_bsscfg_t *bsscfg_target,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, int len, bool update_roam)
{
	struct dot11_management_header *hdr;
	int body_len;
	uint8 *body;
	bool current_bss_bcn = FALSE;
	bool htc = FALSE;
	uint16 fc, ft;

	/* rxhdr is valid only for first beacon in case of aggregated beacons.
	 * Others simply use the same value again and again
	 */
	int plen = 0, delta = 0;

	struct dot11_bcn_prb *bcn;
#ifdef STA
	wlc_bsscfg_t *bsscfg_bcn, *bsscfg = bsscfg_current? bsscfg_current : bsscfg_target;
#endif
	BCM_REFERENCE(osh);

	ASSERT(ISALIGNED(plcp, sizeof(uint16)));

#ifdef BCMDBG
	wlc_update_perf_stats(wlc, WLC_PERF_STATS_BCNS);
#endif

	WL_EVENT_LOG((EVENT_LOG_TAG_BEACON_LOG, WLC_E_BEACON_FRAME_RX));

	plen = len - D11_PHY_HDR_LEN;

	hdr = (struct dot11_management_header*)(plcp + D11_PHY_HDR_LEN);
	body = (uint8 *)(hdr + 1);

	fc = ltoh16(hdr->fc);
	ft = FC_TYPE(fc);
#ifdef WL11N
	htc = ((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev, PhyRxStatus_0) &
		PRXS_FT_MASK(wlc->pub->corerev)) == PRXS0_PREN) &&
		(fc & FC_ORDER) && (ft == FC_TYPE_MNG);
#endif
	ASSERT((ft == FC_TYPE_CTL) || (ft == FC_TYPE_MNG));

	delta = DOT11_MGMT_HDR_LEN + DOT11_FCS_LEN;

	if (htc) {
		body += DOT11_HTC_LEN;
		delta += DOT11_HTC_LEN;
	}

	body_len = plen - delta;

	ASSERT(ISALIGNED(body, sizeof(uint16)));

	bcn = (struct dot11_bcn_prb *)body;

	if (body_len < DOT11_BCN_PRB_LEN)
		return;

#if defined(WL_STATS) && defined(WLTEST) && !defined(WLTEST_DISABLED)
	if (WL_STATS_ENAB(wlc->pub)) {
		/* Update Beacon lengths for the wl utility 'bcnlenhist' */
		wlc_stats_update_bcnlenhist(wlc->stats_info, plen);
	}
#endif
#ifdef STA
#if defined(BCMDBG) || defined(WLMSG_PRPKT)
	if (WL_PRPKT_ON() && bsscfg != NULL) {
		printf("\nrxpkt (beacon)\n");
		wlc_print_bcn_prb((uint8*)plcp, plen + D11_PHY_HDR_LEN - DOT11_FCS_LEN);
	}
#ifdef DEBUG_TBTT
	if (WL_INFORM_ON() && bsscfg_current != NULL) {
		uint32 bcn_tsf_h, bcn_tsf_l, rx_tsf_l;
		uint32 bi;
		uint32 offset;
		uint32 rxstart2tsf;
		ratespec_t rx_rspec;

		bcn_tsf_l = ltoh32_ua(&bcn->timestamp[0]);
		bcn_tsf_h = ltoh32_ua(&bcn->timestamp[1]);

		rx_rspec = wlc_recv_compute_rspec(wlc->d11_info, wrxh, plcp);
		rxstart2tsf = wlc_compute_bcn_payloadtsfoff(wlc, rx_rspec);
		rx_tsf_l = wlc_recover_tsf32(wlc, wrxh);

		WL_INFORM(("wl%d: RxTime 0x%04x RxTime32 0x%08x TSF Offset 0x%x "
			"local TSF 0x%08x Bcn TSF 0x%x:%08x Diff 0x%04x\n",
			wlc->pub->unit, (int)D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
			wlc->pub->corerev, RxTSFTime), rx_tsf_l,
			(int)rxstart2tsf,
			(int)(rx_tsf_l + rxstart2tsf), bcn_tsf_h, bcn_tsf_l,
			(int)((rx_tsf_l + rxstart2tsf) - bcn_tsf_l)));

		bi = ltoh16(bcn->beacon_interval);

		if (bi != 0) {	/* bad AP, skip division */
			offset = wlc_calc_tbtt_offset(bi, bcn_tsf_h, bcn_tsf_l);

			WL_INFORM(("BCN_INFO: seq 0x%x BCN_TSF 0x%08x:%08x OFFSET 0x%x "
				   "prevTBTT 0x%x (tsf_l-prevTBTT) 0x%x %s\n",
				   ltoh16(hdr->seq), bcn_tsf_h, bcn_tsf_l,
				   offset, wlc->prev_TBTT, (bcn_tsf_l - wlc->prev_TBTT),
				   wlc->bad_TBTT ? " badTBTT" : ""));
		}
	}
#endif /* DEBUG_TBTT */
#endif /* BCMDBG || WLMSG_PRPKT */
#endif /* STA */

	/* if there is any restricted chanspec in the locale,  convert
	 * the channel receiving the beacon to active channel if needed
	 */
	if (CLEAR_RESTRICTED_CHANNEL_ENAB(wlc->pub) &&
		(wlc->passive_on_restricted == WL_PASSACTCONV_DISABLE_NONE) &&
		wlc_has_restricted_chanspec(wlc->cmi))
		wlc_convert_restricted_chanspec(wlc, wrxh, hdr, body, body_len);

	/* TODO: an opportunity to use the notification! */

	if (SCAN_IN_PROGRESS(wlc->scan))
		wlc_recv_scan_parse(wlc, wrxh, plcp, hdr, body, body_len);
#if defined(BCMCCX) && defined(STA)
	else if (CCX_ENAB(wlc->pub) && wlc->rm_info->rm_state->ccx->scan_active)
		wlc_recv_scan_parse(wlc, wrxh, plcp, hdr, body, body_len);
#endif /* BCMCCX && STA */

#ifdef WL_NAN
	if (NAN_ENAB(wlc->pub)) {
		/* NAN beacon processing hook ...
		   if the return value is TRUE, it means the beacon frame has been identified as
		   NAN beacon and processed. No further beacon processing is required.
		*/
		/* bsscfg_current can be NULL if the received beacon is from other NAN cluster */
		if (wlc_nan_recv_bcn(wlc, bsscfg_current, wrxh, plcp, hdr, body, body_len))
			return;
	}
#endif

#ifdef PSTA
	/* Process the beacon for each associated bss */
	if (PSTA_ENAB(wlc->pub)) {
		int non_as_sta = 0;
		int32 idx;
		wlc_bsscfg_t *cfg;
		FOREACH_STA(wlc, idx, cfg) {
			if (bcmp(&cfg->BSSID, &hdr->bssid, ETHER_ADDR_LEN) == 0) {
				wlc_recv_process_beacon(wlc, cfg, wrxh, plcp,
				                        hdr, body, body_len, &current_bss_bcn);
#ifdef STA
				if (current_bss_bcn && update_roam) {
					wlc_roam_process_beacon(wlc, bsscfg_current, wrxh, plcp,
					                        hdr, body, body_len);
				}
#endif /* STA */
			} else
				non_as_sta++;
		}
		if (non_as_sta || AP_ACTIVE(wlc)) {
			wlc_recv_process_beacon(wlc, NULL, wrxh, plcp, hdr,
				body, body_len, &current_bss_bcn);
		}
	} else
#endif /* PSTA */
	{
	wlc_recv_process_beacon(wlc, bsscfg_current, wrxh, plcp,
	                        hdr, body, body_len, &current_bss_bcn);
#ifdef STA
	if (current_bss_bcn && update_roam) {
		wlc_roam_process_beacon(wlc, bsscfg_current, wrxh, plcp,
		                        hdr, body, body_len);
	}
#endif
	}

#ifdef STA
	 /* Call wlc_watchdog if we got a primary cfg beacon &&
	  * if we are waiting for beacon. The check for primary cfg beacon
	  * is done because the watchdog TBTT alignmet is done only for
	  * default bsscfg.
	  */
	if (bsscfg_current && BSSCFG_STA(bsscfg_current) &&
		mboolisset(wlc->wd_state, WD_DEFERRED_PM0_BCNRX)) {
		mboolclr(wlc->wd_state, WD_DEFERRED_PM0_BCNRX);
		wlc_watchdog_run_wd_now(wlc);
	}
#endif /* STA */

	/* If we get an AP beacon on a radar quiet channel and
	 * temporary conversions of passive channels to active is not disabled,
	 * signal that the channel is temporarily clear for transmissions.
	 * On the 2G band, insist on [matching] DS_PARMS element.
	 */
	if ((ltoh16(bcn->capability) & DOT11_CAP_ESS) &&
	    !wlc_csa_quiet_mode(wlc->csa, body, body_len) &&
	    wlc->passive_on_restricted != WL_PASSACTCONV_DISABLE_ALL) {
		uint8 chan;
		bcm_tlv_t *ds_params;

		chan = WLC_RX_CHANNEL(wlc->pub->corerev, &wrxh->rxhdr);

		if (CHSPEC_IS2G(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev, RxChan))) {
			ds_params = bcm_parse_tlvs(body + DOT11_BCN_PRB_LEN,
			                           body_len - DOT11_BCN_PRB_LEN,
			                           DOT11_MNG_DS_PARMS_ID);
			if ((ds_params && ds_params->len >= 1) &&
			    (ds_params->data[0] == chan) &&
			    (chan == CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC))) {
				wlc_scan_radar_clear(wlc->scan);
			}
		} else {
			if (wf_chspec_ctlchan(wlc->chanspec) ==
				wf_chspec_ctlchan(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
				wlc->pub->corerev, RxChan))) {
				wlc_scan_radar_clear(wlc->scan);
			}
		}
	}

#ifdef STA
#ifdef PSTA
	/* See if we are waiting for this beacon to join a BSS/IBSS */
	if (PSTA_ENAB(wlc->pub)) {
		int32 idx;
		wlc_bsscfg_t *cfg;
		FOREACH_STA(wlc, idx, cfg) {
			bsscfg_bcn = cfg;
			if (bcmp(&cfg->target_bss->BSSID, &hdr->bssid, ETHER_ADDR_LEN))
				continue;
			wlc_process_target_bss_beacon(wlc, bsscfg_bcn, bcn,
			                              body, body_len, wrxh, plcp);
		}
	} else
#endif /* PSTA */
	if (((bsscfg_bcn = bsscfg_target) != NULL ||
	     (bsscfg_bcn = wlc_bsscfg_find_by_target_bssid(wlc, &hdr->bssid)) != NULL) &&
	    BSSCFG_STA(bsscfg_bcn)) {
		wlc_process_target_bss_beacon(wlc, bsscfg_bcn, bcn,
		                              body, body_len, wrxh, plcp);
	}

	if (PSTA_ENAB(wlc->pub)) {
		int32 idx;
		wlc_bsscfg_t *cfg;
		/* BSS config lookup done at the start of this function finds
		 * the first bss config with a matching BSSID. But for proxy sta
		 * associations we want to find bss config that is in
		 * AS_SYNC_RCV_BCN state and matches bssid in the beacon. So we
		 * need walk thru' the bss configs again to find the right one
		 * among multiple that may match the bssid.
		 */
		bsscfg = NULL;
		FOREACH_STA(wlc, idx, cfg) {
			if (!bcmp(&cfg->BSSID, &hdr->bssid, ETHER_ADDR_LEN) &&
			    (cfg->assoc->state == AS_SYNC_RCV_BCN)) {
				bsscfg = cfg;
				break;
			}
		}
	} else {
		/* Clear bsscfg pointer if we are not waiting for resync beacon */
		if (bsscfg_current == NULL ||
		    bsscfg->assoc->state != AS_SYNC_RCV_BCN)
			bsscfg = NULL;
	}

	if (bsscfg != NULL) {
		wlc_resync_recv_bcn(wlc, bsscfg, wrxh, plcp, bcn);
	}
#endif /* STA */
}

static void
wlc_stamon_monitor(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, void *p, struct wlc_if *wlcif)
{
	struct wl_rxsts sts;
	int status;
	struct dot11_header * mac_header;

	sts.signal = wlc_lq_recv_rssi_compute(wlc, wrxh);
	mac_header = (struct dot11_header *)(PKTDATA(wlc->osh, p) + D11_PHY_HDR_LEN);

	status = wlc_stamon_stats_update(wlc, &mac_header->a2, sts.signal);
	if (status != BCME_OK) {
		WL_ERROR((" STATS update failed in %s at line %d \n",
			__FUNCTION__, __LINE__));
	}
}

/*
 * Filters frames based on class and the current authentication and
 * association states of the remote station.
 * State 1 is unauthenticated and unassociated.
 * State 2 is authenticated and unassociated.
 * State 3 is authenticated and associated.
 * Class 1 frames are permitted from within states 1, 2, and 3.
 * Class 2 frames are permitted if and only if authenticated with the remote station
 *	(i.e., allowed from within States 2 and 3 only).
 * Class 3 frames are permitted if and only if associated with the remote station
 *	(i.e., allowed only from within State 3).
 *
 * Returns 0 on success (accept frame) and non-zero reason code on failure (toss frame).
 * Updates pointer to scb if provided.
 */
static uint16 BCMFASTPATH
wlc_recvfilter(wlc_info_t *wlc, wlc_bsscfg_t **pbsscfg, struct dot11_header *h,
	wlc_d11rxhdr_t *wrxh, struct scb **pscb, int len)
{
#if defined(BCMDBG) || defined(WLMSG_ASSOC) || defined(WLMSG_BTA) || \
	((defined(WLMSG_INFORM) || defined(BCMDBG_ERR)) && defined(AP) && defined(WDS))
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
	uint16 fc = ltoh16(h->fc), rc;
	uint16 fk = fc & FC_KIND_MASK;
	struct scb *scb;
	int class, rx_bandunit;
	bool ibss_accept = FALSE;
	wlc_bsscfg_t *bsscfg;
#if defined(WLTDLS)
	bool dpt_accept = FALSE;
#endif
	struct ether_addr *bssid;

	ASSERT(pbsscfg != NULL);
	bsscfg = *pbsscfg;

	ASSERT(pscb != NULL);
	scb = *pscb;

	rx_bandunit = CHSPEC_BANDUNIT(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
		wlc->pub->corerev, RxChan));

	/* Default is class 1 */
	class = 1;
	rc = 0;

	switch (FC_TYPE(fc)) {

	case FC_TYPE_CTL:
	case FC_TYPE_MNG:
		/* Control and management frames should have no DS bits set */
		if (fc & (FC_TODS | FC_FROMDS))
			/* Some Conexant AP has FROMDS set on Probe Responses */
			if ((fk != FC_PROBE_RESP) &&
			    (fk != FC_PS_POLL))
				goto bad;

		switch (fk) {

		/* Class 2 control and management frames */
		case FC_ASSOC_REQ:
		case FC_ASSOC_RESP:
		case FC_REASSOC_REQ:
		case FC_REASSOC_RESP:
		case FC_DISASSOC:
			WLCNTCONDINCR((fk == FC_ASSOC_REQ), wlc->pub->_cnt->rxassocreq);
			WLCNTCONDINCR((fk == FC_ASSOC_RESP), wlc->pub->_cnt->rxassocrsp);
			WLCNTCONDINCR((fk == FC_REASSOC_REQ), wlc->pub->_cnt->rxreassocreq);
			WLCNTCONDINCR((fk == FC_REASSOC_RESP), wlc->pub->_cnt->rxreassocrsp);
			WLCNTCONDINCR((fk == FC_DISASSOC), wlc->pub->_cnt->rxdisassoc);

			class = 2;
			rc = DOT11_RC_INVAL_CLASS_2;
			break;

		/* Class 3 control and management frames */
		case FC_ACTION:
			WLCNTINCR(wlc->pub->_cnt->rxaction);
			if (wlc_is_vsaction((uint8 *)h, len)) {
				goto done;
			}
			if (bsscfg == NULL) {
				scb = wlc_scbbssfindband(wlc, &h->a1, &h->a2,
				                         rx_bandunit, &bsscfg);
			}
			switch (wlc_is_publicaction(wlc, h, len, scb, bsscfg)) {
			case -1:
				goto bad;
			case 1 :
				goto done;
			}
#ifdef WLMESH
			if (WLMESH_ENAB(wlc->pub) && wlc_is_spaction((uint8 *)h, len)) {
				if (bsscfg == NULL) {
					int idx;
					wlc_bsscfg_t *cfg;

					FOREACH_MESH_BSS(wlc, idx, cfg) {

						if (!eacmp(&cfg->cur_etheraddr, &h->a1)) {
							*pbsscfg = cfg;
							break;
						}
					}
				}
				goto done;
			}
#endif /* WLMESH */
			/* Fall Through */
		case FC_PS_POLL:
		case FC_BLOCKACK:
		case FC_BLOCKACK_REQ:
			WLCNTCONDINCR((fk == FC_PS_POLL), wlc->pub->_cnt->rxpspoll);
			/* We have this counter from Ucode */
			/* WLCNTCONDINCR((fk == FC_BLOCKACK), wlc->pub->_cnt->rxback); */
			WLCNTCONDINCR((fk == FC_BLOCKACK_REQ), wlc->pub->_cnt->rxbar);

			class = 3;
			rc = DOT11_RC_INVAL_CLASS_3;
			if (bsscfg == NULL)
				scb = wlc_scbbssfindband(wlc, &h->a1, &h->a2,
				                         rx_bandunit, &bsscfg);
			break;

		/* All other frame types are class 1 */
		case FC_AUTH:
		case FC_PROBE_REQ:
		case FC_PROBE_RESP:
#ifndef MFP
		case FC_DEAUTH:
			WLCNTCONDINCR((fk == FC_DEAUTH), wlc->pub->_cnt->rxdeauth);
#endif
			WLCNTCONDINCR((fk == FC_AUTH), wlc->pub->_cnt->rxauth);
			WLCNTCONDINCR((fk == FC_PROBE_REQ), wlc->pub->_cnt->rxprobereq);
			WLCNTCONDINCR((fk == FC_PROBE_RESP), wlc->pub->_cnt->rxprobersp);
		default:
			goto done;
		}
		break;

	case FC_TYPE_DATA:
		/* Data frames with no DS bits set are class 1 */
		if (!(fc & (FC_TODS | FC_FROMDS)))
			goto done;

		/* Data frames with both DS bits set are WDS (class 4) */
		if ((fc & (FC_TODS | FC_FROMDS)) == (FC_TODS | FC_FROMDS)) {
			/* 802.11 does not define class 4, just used here for filtering */
			class = 4;
			rc = DOT11_RC_UNSPECIFIED;
			if (bsscfg == NULL) {
				scb = wlc_scbbssfindband(wlc, &h->a1, &h->a2,
				                         rx_bandunit, pbsscfg);
				bsscfg = *pbsscfg;
			}
		} else {
			/* Data frames with DS bits set are class 3 */
			class = 3;
			rc = DOT11_RC_INVAL_CLASS_3;
#ifdef WLMESH
			/* For mesh STA the scb should be found out from a2 in case of
			 * broadcast packet.
			 */
			if (WLMESH_ENAB(wlc->pub) && ETHER_ISMULTI(&h->a1)) {
				int idx;
				wlc_bsscfg_t *cfg;

				FOREACH_MESH_BSS(wlc, idx, cfg) {
					scb = wlc_scbfind(wlc, cfg, &h->a2);
					if (scb != NULL) {
						bsscfg = cfg;
						break;
					}
				}
			}
#endif /* WLMESH */
		}

		break;

	bad:
	default:
		WL_ERROR(("wl%d: %s: bad frame control 0x%.4x\n",
		          wlc->pub->unit, __FUNCTION__, fc));
		rc = DOT11_RC_UNSPECIFIED;
		WLCNTINCR(wlc->pub->_cnt->rxbadcm);
		goto done;
	}

	/* by this point, incoming packets are either class 2, 3, or 4 (WDS) */

	if (bsscfg && scb == NULL) {
		if (BSSCFG_SLOTTED_BSS(bsscfg))
			scb = wlc_scbfind_dualband(wlc, bsscfg, &h->a2);
		else
			scb = wlc_scbfindband(wlc, bsscfg, &h->a2, rx_bandunit);
	}
	if (scb != NULL) {
		ASSERT(bsscfg == SCB_BSSCFG(scb));
		ASSERT(bsscfg != NULL);
		/* Allow specified Class 3 control and management frames in
		 * IBSS mode to pass through.
		 * Current HT or .11 spec do not address how to handle
		 * this types of frame in IBSS network
		 */
		if (BSSCFG_STA(bsscfg) && !bsscfg->BSS && bsscfg->associated &&
		    ((fk == FC_BLOCKACK) ||
		     (fk == FC_BLOCKACK_REQ) ||
		     ((fk == FC_ACTION) && (eacmp(&bsscfg->BSSID, &h->a3) == 0))))
			ibss_accept = TRUE;
#if defined(WLTDLS)
		/* allow specified class 3 control and mgmt frames from a dpt
		 * connection to go through
		 */
		if ((BSSCFG_IS_TDLS(bsscfg) ||
			FALSE) &&
		    ((fk == FC_BLOCKACK) ||
		     (fk == FC_BLOCKACK_REQ) ||
		     ((fk == FC_ACTION) && bcmp(&bsscfg->BSSID, &h->a3, ETHER_ADDR_LEN) == 0)))
			dpt_accept = TRUE;
#endif 
	}

	if (class == 4) {
#if defined(AP) && defined(WDS)
		/* create an scb for every station in "Lazy WDS" mode */
		if ((scb == NULL) && wlc_wds_lazywds_is_enable(wlc->mwds)) {
			wlc_bsscfg_t *bc;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
			char da[ETHER_ADDR_STR_LEN];
#endif


			WL_INFORM(("wl%d: Creating wds link\n", wlc->pub->unit));

			if ((bc = wlc_bsscfg_find_by_hwaddr(wlc, &h->a1)) == NULL) {
				WL_INFORM(("wl%d: BSS %s not found, ignore WDS link from %s\n",
					wlc->pub->unit, bcm_ether_ntoa(&h->a1, da),
					bcm_ether_ntoa(&h->a2, eabuf)));
			}
			else if (!BSSCFG_AP(bc)) {
				WL_INFORM(("wl%d: BSS %s isn't an AP, ignore WDS link from %s\n",
				        wlc->pub->unit, bcm_ether_ntoa(&h->a1, da),
					bcm_ether_ntoa(&h->a2, eabuf)));
			}
			else if (bc != wlc->cfg) {
				WL_INFORM(("wl%d: BSS %s not allow WDS, ignore WDS link from %s\n",
				        wlc->pub->unit, bcm_ether_ntoa(&h->a1, da),
					bcm_ether_ntoa(&h->a2, eabuf)));
			}
			else if ((scb = wlc_scblookup(wlc, bc, &h->a2)) == NULL) {
				WL_ERROR(("wl%d: unable to create SCB for WDS link %s\n",
				        wlc->pub->unit, bcm_ether_ntoa(&h->a2, eabuf)));
			}
			else {
				/* Set up WDS on this interface now */
				int err;
				if ((err = wlc_wds_create(wlc, scb, 0)) != BCME_OK) {
					WL_ERROR(("wl%d: Error %s creating WDS interface\n",
					          wlc->pub->unit, bcmerrorstr(err)));
					wlc_scbfree(wlc, scb);
					scb = NULL;
				} else
					*pbsscfg = bc;
			}
		}
#endif /* AP && WDS */
		if (scb == NULL) {
			WL_ASSOC(("wl%d: %s: unhandled WDS frame from unknown station %s\n",
				wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&h->a2, eabuf)));
			WLCNTINCR(wlc->pub->_cnt->rxbadproto);
		}
		else if (!(SCB_A4_DATA(scb)) && !BSSCFG_MESH(bsscfg)) {
			WL_ASSOC(("wl%d: %s: invalid WDS frame from non-WDS station %s\n",
				wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&h->a2, eabuf)));
			WLCNTINCR(wlc->pub->_cnt->rxbadproto);
		}
#if defined(WL_NAN) && defined(NAN_DEBUG)
		else if (BSSCFG_NAN_DATA(bsscfg) && (nan_filter_rxpkt(wlc->nan, bsscfg, scb,
				D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev, RxChan)))) {
			WL_ERROR(("NAN data packets received outside of NA\n"));
		}
#endif /* WL_NAN && NAN_DEBUG */
		else {
			rc = 0;	/* Accept frame */
		}
	}
	/* early bail-out, all frames accepted at associated or wds state */
	/* BAXXX: This allows all packets thru a WDS link; probably not a good idea */
	/* Need to restrict to some subset */
	else if (scb && (SCB_ASSOCIATED(scb) || SCB_LEGACY_WDS(scb) ||
#if defined(WLTDLS)
	                 dpt_accept ||
#endif
	                 ibss_accept)) {
		rc = 0;		/* Accept frame */
	}
	/* Toss if nonauthenticated */
	else if (!scb || !SCB_AUTHENTICATED(scb)) {
		if (scb == NULL) {
			WL_ASSOC(("wl%d: %s: invalid class %d frame from unknown station %s\n",
				wlc->pub->unit, __FUNCTION__, class,
				bcm_ether_ntoa(&h->a2, eabuf)));
		}
		else
			WL_ASSOC(("wl%d: %s: invalid class %d frame from non-authenticated "
				  "station %s\n", wlc->pub->unit, __FUNCTION__, class,
				  bcm_ether_ntoa(&h->a2, eabuf)));
		/* do not send a deauth if we are on the way to authenticating since
		 * our deauth and an incoming auth response may cross paths
		 * also, don't send if we're not currently on-channel
		 */
		if (!ETHER_ISMULTI(&h->a1) && !(scb && (scb->state & PENDING_AUTH)) &&
		    (rx_bandunit == (int) wlc->band->bandunit)) {
			struct ether_addr *cur_etheraddr;

			if ((fc & (FC_TODS | FC_FROMDS)) == FC_TODS)
				bssid = &h->a1;
			else if ((fc & (FC_TODS | FC_FROMDS)) == FC_FROMDS)
				bssid = &h->a2;
			else if (FC_TYPE(fc) != FC_TYPE_CTL)
				bssid = &h->a3;
			else
				bssid = NULL;


			/* If scb is NULL, the bsscfg will be NULL, either.
			 * Search list with the desired bssid to locate the
			 * correct bsscfg.
			 */
			if (bsscfg == NULL) {
				scb = wlc_scbbssfindband(wlc, &h->a1, &h->a2,
				                         rx_bandunit, pbsscfg);
				bsscfg = *pbsscfg;
			}

			if (bsscfg) {
				cur_etheraddr = &bsscfg->cur_etheraddr;
				if (bssid == NULL)
					bssid = &bsscfg->BSSID;
			} else {
#ifdef STA
				/*
				 *	This is done to avoid sending deauth message
				 *	if a message is delivered to the wlc which doesnt
				 *	host the bsscfg anymore.
				 *
				 * This will happen in case when the bsscfg is
				 * moved to a different WLC during RSDB bsscfg clone
				 */
				if (RSDB_ENAB(wlc->pub) &&
					wlc_bsscfg_find_by_unique_bssid(wlc, &h->a2)) {
						WL_ERROR(("Frame received to a wlc "
						"which doesnt host bsscfg\n"));
						goto done;
				}

				if (RSDB_ENAB(wlc->pub) &&
					wlc_find_assoc_in_progress(wlc, &h->a2)) {
						WL_ERROR(("Frame received to a wlc "
						"due to assoc in progress\n"));
						goto done;
				}
#endif /* STA */
				/* bsscfg not found, use default */
				cur_etheraddr = &h->a1;
				if (bssid == NULL)
					bssid = &h->a2;
				bsscfg = wlc->cfg;
			}

			if (scb == NULL)
				scb = wlc->band->hwrs_scb;

			if (!(AIBSS_ENAB(wlc->pub) && BSSCFG_IBSS(bsscfg))) {
				(void)wlc_senddeauth(wlc, bsscfg, scb,
					&h->a2, bssid, cur_etheraddr, rc);

				wlc_deauth_complete(wlc, bsscfg,
					WLC_E_STATUS_SUCCESS, &h->a2, rc, 0);
			}
		}
		WLCNTINCR(wlc->pub->_cnt->rxbadproto);
	}
	/* Toss if nonassociated */
	else if (class == 3 && !SCB_ASSOCIATED(scb)) {
		WL_ASSOC(("wl%d: %s: invalid class %d frame from non-associated station %s\n",
			wlc->pub->unit, __FUNCTION__, class, bcm_ether_ntoa(&h->a2, eabuf)));
		/* do not send a disassoc if we are on the way to associating since
		 * our disassoc and an incoming assoc response may cross paths
		 */
		if (!ETHER_ISMULTI(&h->a1) && !(scb->state & PENDING_ASSOC) &&
		    (rx_bandunit == (int) wlc->band->bandunit)) {

			bsscfg = SCB_BSSCFG(scb);
			ASSERT(bsscfg != NULL);

			wlc_senddisassoc(wlc, bsscfg, scb, &h->a2, &bsscfg->BSSID,
			                 &bsscfg->cur_etheraddr, rc);
		}
		WLCNTINCR(wlc->pub->_cnt->rxbadproto);
	}
	/* Accept frame */
	else
		rc = 0;

done:
	*pscb = scb;
	return rc;
}

/* Action Frame: byte 0: Category; byte 1: Action field */
static void
wlc_recv_mgmtact(wlc_info_t *wlc, struct scb *scb, struct dot11_management_header *hdr,
                 uint8 *body, int body_len, wlc_d11rxhdr_t *wrxh, uint8 *plcp)
{
	uint action_category;
	uint action_id = 0;
	wlc_bsscfg_t *bsscfg = NULL;
	int8 rssi;
	ratespec_t rspec;
#if defined(WLP2P) && defined(BCMDBG)
	char eabuf[ETHER_ADDR_STR_LEN];
	char chanbuf[CHANSPEC_STR_LEN];
#endif
#if defined(WLFBT) && (defined(BCMDBG) || defined(WLMSG_ASSOC))
	char bss_buf[ETHER_ADDR_STR_LEN];
#endif

	rssi = wlc_lq_recv_rssi_compute(wlc, wrxh);
	rspec = wlc_recv_compute_rspec(wlc->d11_info, wrxh, plcp);

	BCM_REFERENCE(rssi);
	BCM_REFERENCE(rspec);

	if (body_len < 1) {
		WLCNTINCR(wlc->pub->_cnt->rxbadproto);
		return;
	}

	action_category = body[DOT11_ACTION_CAT_OFF];
	if (body_len >= DOT11_ACTION_HDR_LEN)
		action_id = body[DOT11_ACTION_ACT_OFF];

#if defined(WLP2P) && defined(BCMDBG)
	WL_P2P(("wl%d: action frame received from %s on chanspec %s, "
	        "seqnum %d, action_category %d, action_id %d\n",
	        wlc->pub->unit, bcm_ether_ntoa(&hdr->sa, eabuf),
	        wf_chspec_ntoa_ex(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
	        wlc->pub->corerev, RxChan), chanbuf),
	        (ltoh16(hdr->seq) >> SEQNUM_SHIFT), action_category, action_id));
#endif


	/* bsscfg is looked up mainly for sending event, use it otherwise with caution! */
	if (scb != NULL) {
		bsscfg = SCB_BSSCFG(scb);
		ASSERT(bsscfg != NULL);
	}
	else if ((bsscfg = wlc_bsscfg_find_by_bssid(wlc, &hdr->bssid)) != NULL ||
	         (bsscfg = wlc_bsscfg_find_by_hwaddr(wlc, &hdr->da)) != NULL) {
		;	/* empty */
	}
#if defined(BCM_DCS) && defined(AP)
	if (action_category == DOT11_ACTION_CAT_VS && wlc->ap->dcs_enabled)
		wlc_bss_mac_event(wlc, bsscfg, WLC_E_DCS_REQUEST, &hdr->sa, 0, 0, 0,
		                  body, body_len);
#endif /* BCM_DCS && AP */
	/* Send the action frame to the host
	* some OS, OSL process action frames, so pass all up
	* when the build target in question does that
	* exclude block ack - not needed and for perf reasons
	*/
	if (
		/* TODO: this is now OS feature dependent
		* for TOT, create a new iovar to set action frame filter
		* then use the current values to filter here
		* default value can be the one in the ifndef clause
		*/
#ifndef WL_OSL_ACTION_FRAME_SUPPORT
		(action_category == DOT11_ACTION_CAT_PUBLIC) ||
		(action_category == DOT11_ACTION_CAT_PDPA) ||
		(action_category == DOT11_ACTION_CAT_VS) ||
		(action_category == DOT11_ACTION_CAT_QOS) ||
		((action_category == DOT11_ACTION_CAT_WNM) &&
#ifdef WLWNM
		(action_id != DOT11_WNM_ACTION_BSSTRANS_REQ)) ||
#else
		(TRUE)) ||
#endif /* WLWNM */

#else
		(action_category != DOT11_ACTION_CAT_BLOCKACK) ||
#endif /* WL_OSL_ACTION_FRAME_SUPPORT */
		FALSE) {

		wl_event_rx_frame_data_t rxframe_data;

		wlc_bss_mac_event(wlc, bsscfg, WLC_E_ACTION_FRAME, &hdr->sa, 0, 0, 0,
		                  body, body_len);

#if !defined(MACOSX) && !defined(P2PO) && !defined(ANQPO)
	if (wlc_eventq_test_ind(wlc->eventq, WLC_E_ACTION_FRAME_RX))
#endif
	 {
			wlc_recv_prep_event_rx_frame_data(wlc, wrxh, plcp, &rxframe_data);
			if (WLEXTSTA_ENAB(wlc->pub)) {
				wlc_bss_mac_rxframe_event(wlc, bsscfg, WLC_E_ACTION_FRAME_RX,
					&hdr->sa, 0, 0, 0,
					hdr, body_len + DOT11_MGMT_HDR_LEN, &rxframe_data);
			} else {
				wlc_bss_mac_rxframe_event(wlc, bsscfg, WLC_E_ACTION_FRAME_RX,
					&hdr->sa, 0, 0, 0, body, body_len, &rxframe_data);
			}
		}

	}

	switch (action_category) {
	case DOT11_ACTION_CAT_SPECT_MNG:
		/* Spectrum Management */
		if (!WL11H_ENAB(wlc))
			break;
		if (body_len < DOT11_ACTION_HDR_LEN)
			goto rxbadproto;
#ifdef CLIENT_CSA
		if ((body[DOT11_ACTION_ACT_OFF] == DOT11_SM_ACTION_CHANNEL_SWITCH) &&
			scb) {
			if (!SCB_DWDS(scb)) {
				WL_TRACE(("Received CSA frame from NON DWDS STA "MACF""
					"not processing\n", ETHERP_TO_MACF(&scb->ea)));
				break;
			} else {
				WL_TRACE(("Received CSA from DWDS STA "MACF"\n",
					ETHERP_TO_MACF(&scb->ea)));
			}
		}
#endif /* CLIENT_CSA */
		wlc_recv_frameaction_specmgmt(wlc->m11h, hdr, body, body_len, rssi, rspec);
		break;
	case DOT11_ACTION_NOTIFICATION:
		/* WME Management Notification */
		if (bsscfg && !WME_ENAB(wlc->pub))
			break;
		if (body_len <
		    (int)(DOT11_MGMT_NOTIFICATION_LEN + WME_TSPEC_HDR_LEN + WME_TSPEC_LEN))
			goto rxbadproto;
		if (!CAC_ENAB(wlc->pub))
			break;
		wlc_frameaction_cac(bsscfg, action_id, wlc->cac, hdr, body, body_len);
		break;
	case DOT11_ACTION_CAT_VS:
		/* Vendor specific action frame */
		wlc_frameaction_vs(wlc, bsscfg, hdr, body, body_len, wrxh, rspec);
		break;
#ifdef WL11K
	case DOT11_ACTION_CAT_RRM: {
		if ((scb == NULL) || (bsscfg == NULL))
			break;
		if (!WL11K_ENAB(wlc->pub) || !wlc_rrm_enabled(wlc->rrm_info, bsscfg))
			break;
		if (!SCB_RRM(scb))
			break;
		if (body_len < DOT11_RM_ACTION_LEN)
			goto rxbadproto;
		wlc_frameaction_rrm(wlc->rrm_info, bsscfg, scb, action_id, body, body_len,
			rssi, rspec);
		break;
	}
#endif /* WL11K */
#ifdef WLWNM
	case DOT11_ACTION_CAT_WNM:
#ifdef WL_PROXDETECT
		if (PROXD_ENAB(wlc->pub))
			wlc_proxd_recv_action_frames(wlc, bsscfg, hdr, body, body_len, wrxh, rspec);
#endif
		if (!WLWNM_ENAB(wlc->pub))
			break;
		wlc_wnm_recv_process_wnm(wlc->wnm_info, bsscfg, action_id, scb, hdr,
			body, body_len);
		break;
#if defined STA
	case DOT11_ACTION_CAT_UWNM:
#ifdef WL_PROXDETECT
		if (PROXD_ENAB(wlc->pub))
			wlc_proxd_recv_action_frames(wlc, bsscfg, hdr, body, body_len, wrxh, rspec);
#endif
		if (!WLWNM_ENAB(wlc->pub))
			break;
		wlc_wnm_recv_process_uwnm(wlc->wnm_info, bsscfg, action_id, scb, hdr,
			body, body_len);
		break;
#endif /* STA */
#endif /* WLWNM */
#ifdef WLDLS
	case DOT11_ACTION_CAT_DLS:
		if (!WLDLS_ENAB(wlc->pub))
			break;
		wlc_dls_recv_process_dls(wlc->dls, action_id, hdr, body, body_len);
		break;
#endif
#ifdef WLFBT
	case DOT11_ACTION_CAT_FBT:
		if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(wlc->fbt, bsscfg)) {
			dot11_ft_req_t *ftreq;
			WL_FBT(("wl%d: %s: FB ACTION \n", WLCWLUNIT(wlc), __FUNCTION__));

			ftreq = (dot11_ft_req_t*)body;
#ifdef AP
			if (ftreq->action == DOT11_FT_ACTION_FT_REQ)
			{
				if (bsscfg == NULL) {
					WL_ASSOC(("wl%d: %s: FT_REQ: no bsscfg for BSS %s\n",
					          WLCWLUNIT(wlc), __FUNCTION__,
					          bcm_ether_ntoa(&hdr->bssid, bss_buf)));
					/* we couldn't match the incoming frame to a BSS config */
					break;
				}
				if (body_len < DOT11_FT_REQ_FIXED_LEN)
					goto rxbadproto;
				wlc_fbt_recv_overds_req(wlc->fbt, bsscfg, hdr, body, body_len);
			}
			else
#endif /* AP */
#ifdef STA
			if (ftreq->action == DOT11_FT_ACTION_FT_RES)
			{
				if (bsscfg == NULL) {
					WL_ASSOC(("wl%d: %s: FT_RES: no bsscfg for BSS %s\n",
					          WLCWLUNIT(wlc), __FUNCTION__,
					          bcm_ether_ntoa(&hdr->bssid, bss_buf)));
					/* we couldn't match the incoming frame to a BSS config */
					break;
				}
				if (body_len < DOT11_FT_RES_FIXED_LEN)
					goto rxbadproto;
				wlc_fbt_recv_overds_resp(wlc->fbt, bsscfg, hdr, body, body_len);
			}
			else
#endif /* STA */
				goto rxbadproto;

			break;
		}
		break;
#endif /* WLFBT */

	case DOT11_ACTION_CAT_HT:
		/* Process HT Management */
		if (N_ENAB(wlc->pub))
			wlc_frameaction_ht(wlc->hti, action_id, scb, hdr, body, body_len);
		break;
#ifdef WL11AC
	case DOT11_ACTION_CAT_VHT:
		if (VHT_ENAB(wlc->pub))
			wlc_frameaction_vht(wlc->vhti, action_id, scb, hdr, body, body_len);
		break;
#endif
#ifdef WL11AX
	case DOT11_ACTION_CAT_HE:
		if (HE_ENAB(wlc->pub)) {
			wlc_he_actframe_proc(wlc->hei, action_id, scb, body, body_len);
		}
		break;
#endif
	case DOT11_ACTION_CAT_PUBLIC: /* Public Management Action */
	case DOT11_ACTION_CAT_PDPA: /* Protected dual of Public Management Action */
		wlc_frameaction_public(wlc, bsscfg, hdr, body, body_len, wrxh, rspec);
		break;
#ifdef MFP
	case DOT11_ACTION_CAT_SA_QUERY:
		if (WLC_MFP_ENAB(wlc->pub))
			wlc_mfp_handle_sa_query(wlc->mfp, scb, action_id, hdr,
				body, body_len);
		break;
#endif /* MFP */

#ifdef WLMESH
	case DOT11_ACTION_CAT_MESH:
		if (WLMESH_ENAB(wlc->pub)) {
			wlc_mroute_frameaction(wlc->mesh_info, scb, hdr, body, body_len);
		}
		break;
	case DOT11_ACTION_CAT_SELFPROT:
		if (WLMESH_ENAB(wlc->pub)) {
			wlc_mpeer_frameaction_mesh_selfprot(wlc->mesh_info, bsscfg, hdr,
				body, body_len, scb);
		}
		break;
#endif /* WLMESH */

#ifdef WL11U
	case DOT11_ACTION_CAT_QOS:
		if (WL11U_ENAB(wlc) && action_id == DOT11_QOS_ACTION_QOS_MAP &&
			body_len > DOT11_ACTION_HDR_LEN) {
			wlc_11u_set_rx_qos_map_ie(wlc->m11u, bsscfg,
				(bcm_tlv_t *)&body[DOT11_ACTION_HDR_LEN],
				body_len - DOT11_ACTION_HDR_LEN);
		}
		break;
#endif /* WL11U */
	default:
#ifdef WLAMPDU
		/* AMPDU management frames */
		if ((action_category & DOT11_ACTION_CAT_MASK) == DOT11_ACTION_CAT_BLOCKACK) {
			/* Process BA */
			wlc_frameaction_ampdu(wlc, scb, hdr, body, body_len);
			break;
		}
#endif
		if ((action_category & DOT11_ACTION_CAT_ERR_MASK) != 0) {
			break;
		}
		/* Unrecognized (or disabled) action category */
		wlc_send_action_err(wlc, hdr, body, body_len);
		break;
	}

	return;

rxbadproto:
	wlc_send_action_err(wlc, hdr, body, body_len);
	WLCNTINCR(wlc->pub->_cnt->rxbadproto);
}

static void
wlc_recv_process_prbreq(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	BCM_REFERENCE(wlc);
	BCM_REFERENCE(wrxh);
	BCM_REFERENCE(plcp);
	BCM_REFERENCE(hdr);
	BCM_REFERENCE(body);
	BCM_REFERENCE(body_len);

}

static void
wlc_recv_process_prbresp(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	wlc_bsscfg_t *cfg;
	wlc_iem_upp_t upp;
	wlc_iem_pparm_t pparm;


	cfg = wlc_bsscfg_find_by_hwaddr(wlc, &hdr->da);
	if (cfg == NULL) {
#if defined(BCMDBG) || defined(WLMSG_INFORM)
		char eabuf[ETHER_ADDR_STR_LEN];
#endif
		WL_INFORM(("wl%d: %s: unknown DA %s\n",
		           wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&hdr->da, eabuf)));
		return;
	}

	body += DOT11_BCN_PRB_LEN;
	body_len -= DOT11_BCN_PRB_LEN;

	/* prepare IE mgmt calls */
	wlc_iem_parse_upp_init(wlc->iemi, &upp);
	bzero(&pparm, sizeof(pparm));
	pparm.wrxh = wrxh;
	pparm.plcp = plcp;
	pparm.hdr = (uint8 *)hdr;

	/* parse IEs, no error checking */
	wlc_iem_parse_frame(wlc->iemi, cfg, FC_PROBE_RESP, &upp, &pparm, body, body_len);
}

#ifdef STA
/* Reuse wlc_recv_scan_parse_bcn_prb() for IBSS beacon parsing. */
static int
wlc_recv_parse_ibss_bcn_prb(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh,
	struct ether_addr *bssid, bool beacon,
	uint8 *body, uint body_len, wlc_bss_info_t *bi)
{
	return wlc_recv_scan_parse_bcn_prb(wlc, wrxh, bssid, beacon, body, body_len, bi);
}
#endif

#ifdef WL_MIMOPS_CFG
static bool
wlc_bcn_len_valid(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, int bcn_len, bool *cur_bss_bcn)
{
	int delta;
	int expected_bcn_len;

	delta = (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
		wlc->pub->corerev, RxStatus1) & RXS_PBPRES) ?
	        (DOT11_MGMT_HDR_LEN + D11_PHY_HDR_LEN + DOT11_FCS_LEN + 2) :
	        (DOT11_MGMT_HDR_LEN + D11_PHY_HDR_LEN + DOT11_FCS_LEN);
	expected_bcn_len = D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
		wlc->pub->corerev, RxFrameSize) - delta;

	if (bcn_len != expected_bcn_len) {
		WL_STF_MIMO_PS_INFO((
			"Beacon_process: invalid bcn_len=%i, expected=%u,"
			"cur_bss_bcn=%u. Filtered.\n",
			bcn_len, expected_bcn_len, *cur_bss_bcn));
		return FALSE;
	}
	return TRUE;
}
#endif /* WL_MIMOPS_CFG */

/* 'bsscfg' is from wlc_bsscfg_find_by_bssid(wlc, &hdr->bssid) */
static void
wlc_recv_process_beacon(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_management_header *hdr, uint8 *body, int bcn_len, bool *cur_bss_bcn)
{
	struct scb *scb;
	struct dot11_bcn_prb *bcn = (struct dot11_bcn_prb *)body;
	uint8 *tags;
	int tags_len;
	bcm_tlv_t *sup_rates = NULL, *ext_rates = NULL;
	uint8 *erp = NULL;
	int erp_len = 0;
	bool is_erp;
	bool current_bss_bcn;
	uint8 rx_channel;
	uint8 bcn_channel;
	int rx_bandunit;
#ifdef STA
	bcm_tlv_t *ssid_ie = NULL;
	bool short_preamble;
#endif
	ratespec_t rspec;
	ht_add_ie_t *add_ie = NULL;
	ht_cap_ie_t *cap_ie = NULL;
	obss_params_t *obss_ie = NULL;
	uint8 *brcm_ht_ie = NULL;
	uint brcm_ht_ie_len = 0;
#ifdef WL11AC
	int beacon_band;
	vht_cap_ie_t vht_cap_ie;
	vht_cap_ie_t *vht_cap_ie_p = NULL;
	vht_op_ie_t vht_op_ie;
	vht_op_ie_t *vht_op_ie_p = NULL;
	uint8 vht_ratemask = 0;
	uint8 *brcm_vht_ie = NULL;
	uint brcm_vht_ie_len = 0;
#endif /* WL11AC */
	uint16 cap = ltoh16(bcn->capability);
#ifdef STA
	wlc_pm_st_t *pm;
#endif
	chanspec_t node_chspec;
	bool node_bw40;
	wlc_bsscfg_t *bc;
	wlc_iem_upp_t upp;
	wlc_iem_ft_pparm_t ftpparm;
	wlc_iem_pparm_t pparm;
#ifdef STA
	bool ibss_coalesce = FALSE;
#endif
#ifdef WL11AX
	he_cap_ie_t *he_cap_ie = NULL;
	he_op_ie_t *he_op_ie = NULL;
#endif
#ifdef STA
	bcn_notif_data_ext_t data_ext;
#endif /* STA */

#ifdef WL11AX
	BCM_REFERENCE(he_cap_ie);
	BCM_REFERENCE(he_op_ie);
#endif
	ASSERT(ISALIGNED(body, sizeof(uint16)));

	rx_channel = (uint8)wlc_recv_mgmt_rx_channel_get(wlc, wrxh);
	rx_bandunit = CHSPEC_BANDUNIT(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
		wlc->pub->corerev, RxChan));
#ifdef STA
	short_preamble = ((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
		wlc->pub->corerev, PhyRxStatus_0) & PRXS0_SHORTH) != 0);
#endif
	rspec = wlc_recv_compute_rspec(wlc->d11_info, wrxh, plcp);

	/* When there is no DS channel info, use the channel on which we received the beacon */
	/* For 40MHz, rx_channel has been set to the 20MHz side band channel on
	 * which the bcn was recieved.
	 */
	bcn_channel = rx_channel;
#ifdef WL11AC
	beacon_band = CHANNEL_BAND(wlc, bcn_channel);
#endif

	/* Pessimistic approach */
	*cur_bss_bcn = FALSE;

	/* Pre-parse the few IEs that are dependent on each other here... */
	/* TODO: Find a way to exit the while loop as early as possible */
	tags = body + DOT11_BCN_PRB_LEN;
	tags_len = bcn_len - DOT11_BCN_PRB_LEN;
	while (tags_len >= TLV_HDR_LEN &&
	       tags_len >= TLV_HDR_LEN + tags[TLV_LEN_OFF]) {
		bcm_tlv_t *tag = (bcm_tlv_t *)tags;
		int tag_len = TLV_HDR_LEN + tags[TLV_LEN_OFF];

		switch (tag->id) {
		case DOT11_MNG_SSID_ID:
#ifdef STA
			ssid_ie = tag;
#endif /* STA */
			break;
		case DOT11_MNG_RATES_ID:
			sup_rates = tag;
			break;
		case DOT11_MNG_DS_PARMS_ID:
			if (tag_len > 0) {
				/* Do we need to validate the channel? */
				bcn_channel = tag->data[0];
#ifdef WL11AC
				beacon_band = CHANNEL_BAND(wlc, bcn_channel);
#endif
			}
			break;
		case DOT11_MNG_ERP_ID:
			erp = tag->data;
			erp_len = tag->len;
			break;
		case DOT11_MNG_EXT_RATES_ID:
			ext_rates = tag;
			break;
		case DOT11_MNG_HT_CAP:
			cap_ie = wlc_read_ht_cap_ie(wlc, (uint8 *)tag, tag_len);
			break;
		case DOT11_MNG_HT_ADD:
			add_ie = wlc_read_ht_add_ie(wlc, (uint8 *)tag, tag_len);
			break;
		case DOT11_MNG_HT_OBSS_ID:
			obss_ie = wlc_ht_read_obss_scanparams_ie(wlc, (uint8 *)tag, tag_len);
			break;
#ifdef WL11AC
		case DOT11_MNG_VHT_CAP_ID:
			vht_cap_ie_p = wlc_read_vht_cap_ie(wlc->vhti, (uint8 *)tag, tag_len,
				&vht_cap_ie);
			break;
		case DOT11_MNG_VHT_OPERATION_ID:
			vht_op_ie_p = wlc_read_vht_op_ie(wlc->vhti, (uint8 *)tag, tag_len,
				&vht_op_ie);
			break;
#endif /* WL11AC */
		case DOT11_MNG_VS_ID:
			if (tag->len >= DOT11_OUI_LEN + 1 &&
			    bcmp(tag->data, BRCM_PROP_OUI"\x33", DOT11_OUI_LEN + 1) == 0) {
				brcm_ht_ie = (uint8 *)tag;
				brcm_ht_ie_len = tag_len;
				break;
			}
#ifdef WL11AC
			else
			if (tag->len >= DOT11_OUI_LEN + 1 &&
			    bcmp(tag->data, BRCM_PROP_OUI"\x04", DOT11_OUI_LEN + 1) == 0) {
				brcm_vht_ie = (uint8 *)tag;
				brcm_vht_ie_len = tag_len;
				break;
			}
#endif

			break;
#ifdef WL11AX
		case DOT11_MNG_HE_CAP_ID:
			he_cap_ie = (he_cap_ie_t *)tag;
			break;
		case DOT11_MNG_HE_OP_ID:
			he_op_ie = (he_op_ie_t *)tag;
			break;
#endif
		}

		/* next IE */
		tags += tag_len;
		tags_len -= tag_len;
	}

	/* Check BRCM Prop IE if there is no regular HT Cap IE seen */
	if (brcm_ht_ie != NULL && cap_ie == NULL) {
		cap_ie = wlc_read_brcm_ht_cap_ie(wlc, brcm_ht_ie, brcm_ht_ie_len);
	}
	/* Check BRCM Prop IE unconditionally on 2.4G band */
	/*
	 * Prop IE appears if we are running VHT in
	 * 2.4G or the extended rates are enabled
	 */
#ifdef WL11AC
	if (brcm_vht_ie != NULL &&
	    (BAND_2G(beacon_band) || WLC_VHT_FEATURES_RATES(wlc->pub))) {
		uint8 *prop_tlv;
		int prop_tlv_len;

		prop_tlv = wlc_read_vht_features_ie(wlc->vhti, brcm_vht_ie, brcm_vht_ie_len,
		                                    &vht_ratemask, &prop_tlv_len, NULL);
		if (prop_tlv != NULL) {
			vht_cap_ie_p = wlc_read_vht_cap_ie(wlc->vhti,
			                prop_tlv, prop_tlv_len, &vht_cap_ie);
			vht_op_ie_p = wlc_read_vht_op_ie(wlc->vhti,
			                prop_tlv, prop_tlv_len, &vht_op_ie);
		}
	}
#endif /* WL11AC */

#ifdef STA
	/* Check for an IBSS Coalesce, when we get a beacon with a different
	 * BSSID, but the same SSID, and a later TSF.
	 */
	if (bsscfg == NULL &&
	    (cap & DOT11_CAP_IBSS) &&
	    ssid_ie != NULL &&
	    (bc = wlc_bsscfg_find_by_ssid(wlc, ssid_ie->data, ssid_ie->len)) != NULL &&
	    !bc->BSS && bc->associated &&
	    bcn_channel == wf_chspec_ctlchan(bc->current_bss->chanspec)) {

		/* Found an IBSS who's operating on the bcn channel based on the SSID match */
		bsscfg = bc;

		/* Coalesce if allowed and necessary */
		if (wlc_ibss_coalesce_allowed(wlc, bsscfg) &&
		    wlc_bcn_tsf_later(bsscfg, wrxh, plcp, bcn)) {
			wlc_bss_info_t *target_bss = bsscfg->target_bss;

#if defined(BCMDBG) || defined(WLMSG_ASSOC)
			if (WL_ASSOC_ON()) {
				char from_id[ETHER_ADDR_STR_LEN];
				char to_id[ETHER_ADDR_STR_LEN];
				uint len;

				bcm_ether_ntoa(&bsscfg->BSSID, from_id);
				bcm_ether_ntoa(&hdr->bssid, to_id);
				WL_ASSOC(("JOIN: IBSS Coalesce from BSSID %s to BSSID %s\n",
				          from_id, to_id));
				len = bcn_len + DOT11_MGMT_HDR_LEN + D11_PHY_HDR_LEN;
				wlc_print_bcn_prb(plcp, len);
			}
#endif /* BCMDBG || WLMSG_ASSOC */

			/* The received beacon has a greater TSF time, so adopt its parameters */
			if (wlc_recv_parse_ibss_bcn_prb(wlc, wrxh, &hdr->bssid, TRUE,
			                           (uint8 *)bcn, bcn_len, target_bss) == 0) {
				/* IBSS PS capability mis match, don't join the network */
				if (!target_bss->atim_window ||
					(target_bss->atim_window && AIBSS_PS_ENAB(wlc->pub))) {
					wlc_rate_hwrs_filter_sort_validate(&target_bss->rateset,
					        &wlc->band->hw_rateset, FALSE, wlc->stf->txstreams);
					wlc_join_complete(bsscfg, wrxh, plcp, bcn, bcn_len);
					wlc_bss_mac_event(wlc, bsscfg, WLC_E_BSSID, &bsscfg->BSSID,
						0, WLC_E_STATUS_SUCCESS,
						bsscfg->target_bss->bss_type, 0, 0);
					ibss_coalesce = TRUE;
				}
			}
		}
	}
#endif /* STA */

	/* Lookup the scb in case we already have info on it */
	scb = NULL;
	if (bsscfg != NULL) {
		if (AIBSS_ENAB(wlc->pub)) {
			int myband = CHSPEC_BANDUNIT(bsscfg->current_bss->chanspec);
			if (myband != rx_bandunit) {
				WL_ERROR(("do not process from invalid band %d %d\n",
					myband, rx_bandunit));
				return;
			}
		}
#ifdef STA
		/* Don`t process beacons that don`t match the SSID (i.e some other IBSS Network) */
		if (BSSCFG_IBSS(bsscfg) &&
			(ssid_ie != NULL) &&
			(bsscfg != wlc_bsscfg_find_by_ssid(wlc, ssid_ie->data, ssid_ie->len))) {
			WL_ERROR(("Do not process from some other AIBSS network\n"));
			return;
		}
#endif /* STA */
		scb = wlc_scbfindband(wlc, bsscfg, &hdr->sa, rx_bandunit);
		/* create an scb if one does not exist (for ibss's) */
		if (scb == NULL && !bsscfg->BSS) {
			scb = wlc_scblookupband(wlc, bsscfg, &hdr->sa, rx_bandunit);
			if (scb) {
				wlc_lq_sample_req_enab(scb, RX_LQ_SAMP_REQ_IBSS_STA, TRUE);
			}
		}
	}
	/* Lookup the scb for WDS */
	else if (AP_ACTIVE(wlc)) {
		scb = wlc_scbapfind(wlc, &hdr->sa, &bc);
	}

	/* check rates for ERP capability */
	is_erp = FALSE;
	if (scb == NULL || !(scb->flags & SCB_NONERP)) {
		if ((ext_rates != NULL &&
		     wlc_rateset_isofdm(ext_rates->len, ext_rates->data)) ||
		    (sup_rates != NULL &&
		     wlc_rateset_isofdm(sup_rates->len, sup_rates->data)))
			is_erp = TRUE;
		if (scb != NULL && !is_erp)
			scb->flags |= SCB_NONERP;
	}

	/* check for beacon in our BSS (STA's AP or IBSS member) */
	/* APSTA: Still valid -- in APSTA mode, current_bss is the STA's BSS */
	/* is the beacon from the channel I am operating on? */
	if (bsscfg != NULL && BSSCFG_STA(bsscfg) && bsscfg->associated) {
		node_chspec = bsscfg->current_bss->chanspec;
		node_bw40 = CHSPEC_IS40(node_chspec);
		current_bss_bcn = wf_chspec_ctlchan(node_chspec) == bcn_channel;
	}
	else {
		node_chspec = 0;
		node_bw40 = FALSE;
		current_bss_bcn = FALSE;
	}

	WL_TRACE(("wl%d: beacon received on ch%d (RxChan 0x%04x), bcn_channel %d, "
	          "current_bss_bcn %d\n",
	          wlc->pub->unit, rx_channel, D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
	          wlc->pub->corerev, RxChan), bcn_channel, current_bss_bcn));

	/* ===== AP and IBSS ===== */
	if (AP_ACTIVE(wlc) || wlc->ibss_bsscfgs > 0) {
		/* TODO:
		 *
		 * Rework fns wlc_ht_update_scbstate and wlc_vht_update_scbstate to
		 * independently update relevant states based on each IE...which then
		 * provides a possibility to move these inter-dependable IEs...
		 */

		/* process the brcm_ie and ht ie in a beacon frame */
		if (AP_ACTIVE(wlc) &&
		    scb != NULL && SCB_LEGACY_WDS(scb)) {

			if (N_ENAB(wlc->pub)) {
				/* Update HT based features (should be smarter than this...) */
				if (cap_ie != NULL)
					wlc_ht_bcn_scb_upd(wlc, scb, cap_ie, add_ie, obss_ie);
				/* Ensure HT based features are cleared here */
				else if (SCB_HT_CAP(scb))
					wlc_ht_bcn_scb_upd(wlc, scb, NULL, NULL, NULL);
			}
#ifdef WL11AC
			if (VHT_ENAB_BAND(wlc->pub, beacon_band)) {
				if (cap_ie != NULL && vht_cap_ie_p != NULL) {
					wlc_vht_bcn_scb_upd(wlc->vhti, beacon_band, scb, cap_ie,
					        vht_cap_ie_p, vht_op_ie_p, vht_ratemask);
				} else if (SCB_VHT_CAP(scb)) {
					wlc_vht_bcn_scb_upd(wlc->vhti, beacon_band, scb, NULL,
					        NULL, NULL, 0);
				}
			}

			if (!SCB_VHT_CAP(scb) && !(scb->flags & SCB_WDS_LINKUP)) {
				wlc_vht_update_scb_state(wlc->vhti,
					beacon_band, scb, NULL, NULL, NULL, 0);
				wlc_scb_ratesel_init(wlc, scb);
			}
#endif /* WL11AC */
		}

		/* TODO:
		 *
		 * Rework function wlc_prot_n_ovlp_upd to independently update
		 * relevant states based on each IE...which then provides a possibility
		 * to move these inter-dependable IEs to their own parsers...
		 */

		/* For G APs or IBSSs, check overlapping beacon information */
		if (wlc->band->gmode)
			wlc_prot_g_ovlp_upd(wlc->prot_g, CH20MHZ_CHSPEC(bcn_channel),
			                    erp, erp_len, is_erp, (cap & DOT11_CAP_SHORT) == 0);
		/* For HT APs or IBSSs, check overlapping beacon information */
		if (N_ENAB(wlc->pub))
			wlc_prot_n_ovlp_upd(wlc->prot_n, CH20MHZ_CHSPEC(bcn_channel),
			                    add_ie, cap_ie, is_erp, node_bw40);

		/* Caution:
		 *
		 * The callbacks invoked here could be getting NULL bsscfg because
		 * the beacon received may not belong to any BSSes we know of.
		 */
		if (AP_ACTIVE(wlc)) {
			BCM_REFERENCE(current_bss_bcn);

			/* Individual IE processing... */
			tags = body + DOT11_BCN_PRB_LEN;
			tags_len = bcn_len - DOT11_BCN_PRB_LEN;

			wlc_iem_parse_upp_init(wlc->iemi, &upp);
			bzero(&ftpparm, sizeof(ftpparm));
			ftpparm.bcn.scb = scb;
			ftpparm.bcn.chan = bcn_channel;
			ftpparm.bcn.cap = cap;
			ftpparm.bcn.erp = is_erp;
			bzero(&pparm, sizeof(pparm));
			pparm.ft = &ftpparm;
			pparm.wrxh = wrxh;
			pparm.plcp = plcp;
			pparm.hdr = (uint8 *)hdr;
			pparm.body = (uint8 *)bcn;
			pparm.rxchan = rx_channel;
			pparm.rspec = rspec;
			pparm.ht = N_ENAB(wlc->pub);
#ifdef WL11AC
			pparm.vht = VHT_ENAB(wlc->pub);
#endif
#ifdef WL11AX
			pparm.he = HE_ENAB(wlc->pub);
#endif
			wlc_iem_parse_frame(wlc->iemi, NULL, WLC_IEM_FC_AP_BCN, &upp, &pparm,
			                    tags, tags_len);
		}
	}

#ifdef STA
	/* ===== STA and IBSS ===== */
	/* Exit only if we don't have any STA processing to do */
	if (wlc->stas_associated == 0) {
#ifdef WL_MONITOR
		/* While starting the monitor mode, if monitoring channel is a quiet channel, then
		 * channel calibration would be skipped.
		 * Since a beacon is received on current channel, it is OK to clear the quiet and
		 * calibrate the channel
		 */
		if (wlc->monitor &&
			wlc->aps_associated == 0 &&
			(wf_chspec_ctlchan(wlc->chanspec) ==
			wf_chspec_ctlchan(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
			wlc->pub->corerev, RxChan)))) {
			if (wlc_phy_ismuted(WLC_PI(wlc))) {
				wlc_monitor_phy_cal_timer_start(wlc->mon_info, 0);
			}
		}
#endif /* WL_MONITOR */
		return;
	}

	/* FROM THIS POINT ON ALL CODE ARE DEALING WITH BEACON FROM THE BSS */
	if (!current_bss_bcn)
		return;

	/* FROM THIS POINT ON 'bsscfg' MUST BE OF STA (Infra or IBSS) */
	ASSERT(bsscfg != NULL);
	ASSERT(BSSCFG_STA(bsscfg));

	/* Upgrade/Downgrade bandwidth as necessary... */
	if (N_ENAB(wlc->pub) &&
	    add_ie != NULL && cap_ie != NULL &&
	    (WL11H_ENAB(wlc) ? wlc_csa_get_csa_count(wlc->csa, bsscfg) : 0) == 0) {
		wlc_bss_info_t *current_bss = bsscfg->current_bss;
		chanspec_t bss_chspec = wlc_ht_chanspec(wlc, add_ie->ctl_ch, add_ie->byte1,
			bsscfg);
#ifdef WL_MODESW
		chanspec_t max_opermode_chanspec = bss_chspec;
#endif
		uint b_u = CHSPEC_IS2G(node_chspec) ? BAND_2G_INDEX : BAND_5G_INDEX;
		wlcband_t *band = wlc->bandstate[b_u];

		/* check if current BSS support 40MHz */
		bool bss_allow40 =
			((current_bss->flags & WLC_BSS_HT) &&
		         (ltoh16_ua(&cap_ie->cap) & HT_CAP_40MHZ)) ?
		        (add_ie->byte1 & HT_BW_ANY) != 0 : FALSE;

#ifdef WL11AC
		if (VHT_ENAB_BAND(wlc->pub, beacon_band) && vht_op_ie_p) {
#ifdef WL_MODESW
			if (WLC_MODESW_ENAB(wlc->pub)) {
				/* Retain the BSS's chanspec inside max_opermode_chanspec
				* This is useful information whenever we want to upgrade the
				* STA interface.
				* wlc_vht_chanspec() will return the BSS's chanspec since
				* operating mode
				* is intentionally set to FALSE while calling it.
				*/
				max_opermode_chanspec = wlc_vht_chanspec(wlc->vhti,
					vht_op_ie_p, bss_chspec, FALSE, FALSE, bsscfg);
			}
#endif /* WL_MODESW */
			/* Get the chanspec as per our operating mode */
			bss_chspec = wlc_vht_chanspec(wlc->vhti, vht_op_ie_p, bss_chspec,
				bsscfg->oper_mode_enabled, bsscfg->oper_mode, bsscfg);
		}
		else
#endif /* WL11AC */
#ifdef WL_OBSS_DYNBW
		if (WLC_OBSS_DYNBW_ENAB(wlc->pub)) {
			bss_chspec = wlc_obss_dynbw_ht_chanspec_override(wlc->obss_dynbw,
				bsscfg, bss_chspec);
		}
#endif /* WL_OBSS_DYNBW */

		/* wf_chspec_ctlchan() below expects a proper chspec,
		 * Check for malformed chspec (unsupported etc.) and return if TRUE
		 */
		if (wf_chspec_malformed(bss_chspec)) {
			return;
		}

		if ((wf_chspec_ctlchan(bss_chspec) == rx_channel) &&
			(wf_chspec_ctlchan(bss_chspec) == wf_chspec_ctlchan(node_chspec))) {
			if (bss_chspec != node_chspec) {
				/* find the max bw channel that we can use from beacon channel */
				bss_chspec =
					wlc_max_valid_bw_chanspec(wlc, band, bsscfg, bss_chspec);
				if (bss_chspec == INVCHANSPEC) {
					WL_ERROR(("Beacon received on invalid channel \n"));
					return;
				}

				/* update bandwidth */
				if (wlc_bw_update_required(wlc, bsscfg, bss_chspec, bss_allow40)) {
#ifdef WL_MIMOPS_CFG
					if (WLC_MIMOPS_ENAB(wlc->pub)) {
						wlc_hw_config_t *bsscfg_hw_cfg =
						    wlc_stf_bss_hw_cfg_get(bsscfg);
						if (!bsscfg_hw_cfg->chanspec_override) {
							bsscfg_hw_cfg->current_chanspec_bw =
							    bss_chspec;
							wlc_update_bandwidth(wlc, bsscfg,
							    bss_chspec);
						}
						/* store default chanspec if it has changed */
						bsscfg_hw_cfg->original_chanspec_bw = bss_chspec;
					} else
#endif /* WL_MIMOPS_CFG */
					{
						wlc_update_bandwidth(wlc, bsscfg, bss_chspec);
					}
				}
			}
#ifdef WL_MODESW
			if (WLC_MODESW_ENAB(wlc->pub)) {
#ifdef WL11AC
				if (BSS_VHT_ENAB(wlc, bsscfg)) {
					if (scb != NULL) {
						uint8 ap_oper_mode;
						/* Overwrite the original chanspec for future use
						 * during upgrade
						 */
						wlc_modesw_set_max_chanspec(wlc->modesw, bsscfg,
								max_opermode_chanspec);
						if ((wlc_vht_get_scb_opermode_enab(wlc->vhti,
								scb))) {
							uint8 ap_rxnss = DOT11_OPER_MODE_RXNSS(
									wlc_vht_get_scb_opermode(
										wlc->vhti, scb));
							ap_oper_mode = wlc_modesw_derive_opermode(
									wlc->modesw,
									max_opermode_chanspec,
									bsscfg, ap_rxnss);
							/* Perform ratesel init to allow new rate
							 * spec to be used based upon
							 * AP's operating mode
							 */
							wlc_vht_update_scb_oper_mode(wlc->vhti,
									scb, ap_oper_mode);
						}
					}
				}
				else /* HT case Opermode set */
#endif /* WL11AC */
				wlc_modesw_set_max_chanspec(wlc->modesw, bsscfg,
					max_opermode_chanspec);
			}
#endif /* WL_MODESW */
		}
	}

#ifdef WL_MIMOPS_CFG
	if (WLC_MIMOPS_ENAB(wlc->pub)) {
		if (!wlc_bcn_len_valid(wlc, wrxh, bcn_len, cur_bss_bcn))
			return;
	}
#endif /* WL_MIMOPS_CFG */

	/* If we reached till this point, then it means the beacon is from this bss */
	*cur_bss_bcn = TRUE;

	pm = bsscfg->pm;

	/* TODO:
	 *
	 * Rework fns wlc_ht_update_scbstate and wlc_vht_update_scbstate to
	 * independently update relevant states based on each IE...which then
	 * provides a possibility to move these inter-dependable IEs to their
	 * own parsers in order to fit in to the IE mgmt. archicture...
	 */

	/* the cap and additional IE are only in the bcn/prb response pkts,
	 * when joining a bss parse the bcn_prb to check for these IE's.
	 */
	if (N_ENAB(wlc->pub) && scb != NULL) {
		/* update ht state or if we are in IBSS mode then clear HT state */
		if (cap_ie != NULL)
			wlc_ht_bcn_scb_upd(wlc, scb, cap_ie, add_ie, obss_ie);
		/* Ensure HT based features are cleared for IBSS STAs */
		else if (!bsscfg->BSS && SCB_HT_CAP(scb))
			wlc_ht_bcn_scb_upd(wlc, scb, NULL, NULL, NULL);
	}
#ifdef WL11AC
	if (VHT_ENAB_BAND(wlc->pub, beacon_band) && scb != NULL) {
		if (cap_ie && vht_cap_ie_p) {
			wlc_vht_bcn_scb_upd(wlc->vhti, beacon_band, scb, cap_ie,
			                    vht_cap_ie_p, vht_op_ie_p, vht_ratemask);
		} else {
			wlc_vht_bcn_scb_upd(wlc->vhti, beacon_band, scb, NULL, NULL, NULL, 0);
		}
	}
#endif
#ifdef WL11AX
	/* collapse all these blocks with scb != NULL into a single if (scb != NULL) { } */
	if (HE_ENAB_BAND(wlc->pub, beacon_band) && scb != NULL) {
		if (he_cap_ie != NULL && he_op_ie != NULL) {
			wlc_he_bcn_scb_upd(wlc->hei, beacon_band, scb,
				he_cap_ie, he_op_ie);
		} else {
			wlc_he_bcn_scb_upd(wlc->hei, beacon_band, scb,
				NULL, NULL);
		}
	}
#endif

#ifdef WL11K
	if (WL11K_ENAB(wlc->pub) && scb != NULL) {
		/* Update the scb for RRM Capability */
		if (bsscfg->associated && wlc_rrm_enabled(wlc->rrm_info, bsscfg)) {
			if (cap & DOT11_CAP_RRM)
				scb->flags3 |= SCB3_RRM;
			else
				scb->flags3 &= ~SCB3_RRM;
		}
	}
#endif /* WL11K */

	/* Clear out the interval count */
	bsscfg->roam->bcn_interval_cnt = 0;

	/* TODO:
	 *
	 * Add a registry for others to register callbacks for further
	 * complex beacon processing. Use IE parsing callbacks otherwise.
	 */

	/* Infra STA beacon processing... */
	if (bsscfg->BSS) {

		wlc_check_for_retrograde_tsf(wlc, bsscfg, wrxh, plcp, hdr, body);

#ifdef WLMCHAN
		if (MCHAN_ENAB(wlc->pub)) {
			wlc_mchan_recv_process_beacon(wlc, bsscfg, scb, wrxh, plcp, body, bcn_len);
		}
#endif

		/* Check for PM states. */

		/* If we were waiting for a beacon before allowing doze,
		 * clear the flag and update ps_ctrl
		 */
		if (pm->PMawakebcn) {
#ifdef BCMDBG
			WL_PS(("wl%d.%d: got beacon, clearing PMawakebcn\n",
			       wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
#endif
			wlc_set_pmawakebcn(bsscfg, FALSE);
		}

		/* APSTA: APSTA only does STA-like TSF behavior if APs off */
		/* APSTA: May want to still check for retrograde TSF? */

		if (!pm->check_for_unaligned_tbtt &&
#ifdef WLMCNX
		    !MCNX_ENAB(wlc->pub) &&
#endif
		    wlc_check_tbtt(bsscfg))
			wlc_set_uatbtt(bsscfg, TRUE);

		if (pm->check_for_unaligned_tbtt) {
#ifdef WLMCNX
			/* update connection specific TSF */
			if (MCNX_ENAB(wlc->pub))
				;	/* empty */
			else
#endif
			wlc_tbtt_align(bsscfg, short_preamble, rspec, bcn);

			if (!wlc->ebd->thresh ||
				wlc->ebd->detect_done)
				wlc_set_uatbtt(bsscfg, FALSE);
			/* Extend uatbtt time to detect early beacons */
			else if (((OSL_SYSUPTIME() - wlc->ebd->uatbtt_start_time) >
				EARLY_DETECT_PERIOD)) {
				wlc->ebd->detect_done = TRUE;
				/* allow core to sleep again */
				wlc_set_uatbtt(bsscfg, FALSE);
			}
		}
#if defined(WLMCNX)
		/* Check if this is an early beacon */
		if (MCNX_ENAB(wlc->pub) && wlc->ebd->thresh) {
			uint32 bcn_l, bcn_h;
			uint32 bp, bcn_offset, bmi_bcn_offset;
			uint32 ts_off = wlc_compute_bcntsfoff(wlc, rspec, short_preamble, TRUE);
			bp = ltoh16(bcn->beacon_interval);
			/* sick AP? */
			if (bp == 0)
				bp = 100;
			bcn_l = ltoh32_ua(&bcn->timestamp[0]);
			bcn_h = ltoh32_ua(&bcn->timestamp[1]);

			/* Timestamp is at a offset in beacon,
			 * wakeup should be at tbtt, so adjust for timestamp offset
			 */
			bcm_sub_64(&bcn_h, &bcn_l, ts_off);
			bcn_offset = wlc_calc_tbtt_offset(bp, bcn_h, bcn_l);
			bmi_bcn_offset = wlc_get_bmi_bcn_offset(wlc->mcnx, bsscfg);
			if (bcn_offset < bmi_bcn_offset) {
				bcn_offset += bp*1024;
			}
			bcn_offset -= bmi_bcn_offset;
			/* Early bcn? */
			if (bcn_offset > (bp * 1024 - wlc->ebd->thresh)) {
				/* earlier than earliest beacon? */
				if (bcn_offset < (bp * 1024 - wlc->ebd->earliest_offset)) {
					wlc->ebd->earliest_offset = bp * 1024 - bcn_offset;
					wlc_mcnx_tbtt_upd(wlc->mcnx, bsscfg, TRUE);
				}
			}
		}
#endif /* WLMCNX */

		/* Track network ShortSlotTime */
		if (wlc->band->gmode)
			wlc_switch_shortslot(wlc, (cap & DOT11_CAP_SHORTSLOT) != 0);

		/* . Make sure this part is done before wlc_11h_process_beacon, which will
		 * process Channel Switch Announcement (CSA) information.
		 * . if this channel is marked as quiet, we can clear the bit and un-mute
		 * after switching channels when we reestablished our AP connection
		 */
		if (wlc_quiet_chanspec(wlc->cmi, node_chspec)) {
			if ((WL11H_ENAB(wlc) ? wlc_csa_get_csa_count(wlc->csa, bsscfg) : 0) == 0) {
				wlc_clr_quiet_chanspec(wlc->cmi, node_chspec);
				if (WLC_BAND_PI_RADIO_CHANSPEC == node_chspec) {
					wlc_mute(wlc, OFF, 0);
				}
			}
		}


#ifdef WLMCNX
		/* sync up TSF */
		if (MCNX_ENAB(wlc->pub)) {
			wlc_mcnx_adopt_bcn(wlc->mcnx, bsscfg, wrxh, plcp, bcn);
		}
#endif /* WLMCNX */
#ifdef WLP2P
		/* sync up NoA on P2P connections */
		if (BSS_P2P_ENAB(wlc, bsscfg))
			wlc_p2p_recv_process_beacon(wlc->p2p, bsscfg, bcn, bcn_len);
#endif
		if ((pm->ps_resend_mode == PS_RESEND_MODE_BCN_NO_SLEEP ||
		     pm->ps_resend_mode == PS_RESEND_MODE_BCN_SLEEP) &&
		    pm->pm_immed_retries == PM_IMMED_RETRIES_MAX) {
			wlc_sendnulldata(wlc, bsscfg, &bsscfg->BSSID, 0, 0,
				PKTPRIO_NON_QOS_DEFAULT, NULL, NULL);
		}
#ifdef WL_PWRSTATS
		if (PWRSTATS_ENAB(wlc->pub))
			wlc_pwrstats_bcn_process(bsscfg, wlc->pwrstats, hdr->seq>>SEQNUM_SHIFT);
#endif /* WL_PWRSTATS */
	}
	/* IBSS beacon processing... */
	else {
		bool shortpreamble;

		shortpreamble = (is_erp || (cap & DOT11_CAP_SHORT) != 0);

		/* Record short preamble capability and process brcm ie */
		if (scb) {
			if (shortpreamble)
				scb->flags |= SCB_SHORTPREAMBLE;
			else
				scb->flags &= ~SCB_SHORTPREAMBLE;
		}

		if (wlc->band->gmode)
			wlc_prot_g_to_upd(wlc->prot_g, bsscfg, erp, erp_len,
			                  is_erp, shortpreamble);
		if (N_ENAB(wlc->pub))
			wlc_prot_n_to_upd(wlc->prot_n, bsscfg, add_ie, cap_ie,
			                  is_erp, node_bw40);
#if defined(IBSS_PEER_DISCOVERY_EVENT)
		/* IBSS peer discovery */
		if (IBSS_PEER_DISCOVERY_EVENT_ENAB(wlc->pub) &&
		    !BSS_TDLS_ENAB(wlc, bsscfg)) {
			if (scb == NULL) {
				WL_ERROR(("wl%d: %s: no scb for IBSS peer\n",
				          wlc->pub->unit, __FUNCTION__));
				WLCNTINCR(wlc->pub->_cnt->rxnoscb);
			}
			else {
				scb->used = wlc->pub->now;
#if defined(DBG_BCN_LOSS)
				scb->dbg_bcn.last_rx = wlc->pub->now;
#endif
			}
		}
#endif /* defined(IBSS_PEER_DISCOVERY_EVENT) */

		if (!ibss_coalesce) {
#ifdef WLMCNX
			/* sync up TSF */
			if (MCNX_ENAB(wlc->pub) &&
#ifndef WLTSFSYNC
			    bsscfg != wlc->cfg &&
#endif
			    wlc_bcn_tsf_later(bsscfg, wrxh, plcp, bcn))
				wlc_tsf_adopt_bcn(bsscfg, wrxh, plcp, bcn);
#endif /* WLMCNX */

			/* Reset CFPSTART if tbtt is too far in the future which will cause
			 * IBSS to stop beacon
			 */
			if (!AP_ACTIVE(wlc) && bsscfg == wlc->cfg) {
				if (wlc_check_tbtt(bsscfg)) {
					wlc_reset_cfpstart(wlc, bcn);
				}
			}
		}
#ifdef PSPRETEND
		/* The SCB's peer is alive. Get out of PS PRETEND */
		if (scb && SCB_PS_PRETEND_THRESHOLD(scb) &&
			SCB_PS(scb))
			wlc_apps_process_ps_switch(wlc, &scb->ea, PS_SWITCH_OFF);
#endif /* PSPRETEND */
	}

	/* bcn notify data extention */
	bzero(&data_ext, sizeof(bcn_notif_data_ext_t));
	data_ext.cap_ie = cap_ie;
#ifdef WL11AC
	data_ext.vht_cap_ie_p = (void *)vht_cap_ie_p;
#endif
	data_ext.sup_rates = sup_rates;
	data_ext.ext_rates = ext_rates;
	/* indicate the arrival of beacon */
	wlc_bss_rx_bcn_signal(wlc, bsscfg, scb, wrxh, plcp, hdr, body, bcn_len, &data_ext);

	/* Individual IE processing... */
	tags = body + DOT11_BCN_PRB_LEN;
	tags_len = bcn_len - DOT11_BCN_PRB_LEN;

	/* prepare IE mgmt call */
	wlc_iem_parse_upp_init(wlc->iemi, &upp);
	bzero(&ftpparm, sizeof(ftpparm));
	ftpparm.bcn.scb = scb;
	ftpparm.bcn.chan = bcn_channel;
	ftpparm.bcn.cap = cap;
	ftpparm.bcn.erp = is_erp;
	bzero(&pparm, sizeof(pparm));
	pparm.ft = &ftpparm;
	pparm.wrxh = wrxh;
	pparm.plcp = plcp;
	pparm.hdr = (uint8 *)hdr;
	pparm.body = (uint8 *)bcn;
	pparm.rxchan = rx_channel;
	pparm.rspec = rspec;
	pparm.ht = N_ENAB(wlc->pub);
#ifdef WL11AC
	pparm.vht = VHT_ENAB(wlc->pub);
#endif
#ifdef WL11AX
	pparm.he = HE_ENAB(wlc->pub);
#endif
	/* parse IEs */
	wlc_iem_parse_frame(wlc->iemi, bsscfg, FC_BEACON, &upp, &pparm, tags, tags_len);
#endif /* STA */
}

static void
wlc_frameaction_vs(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len,
	wlc_d11rxhdr_t *wrxh, uint32 rspec)
{
	BCM_REFERENCE(bsscfg);
	BCM_REFERENCE(wrxh);
	BCM_REFERENCE(rspec);

#ifdef WLP2P
	/* P2P action frame */
	if (P2P_ENAB(wlc->pub) &&
	    body_len >= P2P_AF_FIXED_LEN &&
	    bcmp(((wifi_p2p_action_frame_t *)body)->OUI, WFA_OUI, WFA_OUI_LEN) == 0 &&
	    ((wifi_p2p_action_frame_t *)body)->type == WFA_OUI_TYPE_P2P) {
		ASSERT(((wifi_p2p_action_frame_t *)body)->category == DOT11_ACTION_CAT_VS);
		wlc_p2p_process_action(wlc->p2p, hdr, body, body_len);
		return;
	}
#endif
#if defined(WIFI_REFLECTOR) || defined(RWL_WIFI)
	/* this is a Remote WL command */
	if (body_len > DOT11_OUI_LEN &&
	    ((body[1+DOT11_OUI_LEN] == RWL_WIFI_FIND_MY_PEER) ||
	     (body[1+DOT11_OUI_LEN] == RWL_WIFI_FOUND_PEER) ||
	     (body[1+DOT11_OUI_LEN] == RWL_WIFI_DEFAULT) ||
	     (body[1+DOT11_OUI_LEN] == RWL_ACTION_WIFI_FRAG_TYPE))) {
		wlc_recv_wifi_mgmtact(wlc->rwl, body, &hdr->sa);
		return;
	}
#endif
#ifdef WL_PROXDETECT
	if (PROXD_ENAB(wlc->pub) &&
	   body_len >= PROXD_AF_FIXED_LEN &&
	   bcmp(((dot11_action_vs_frmhdr_t *)body)->OUI, BRCM_PROP_OUI, DOT11_OUI_LEN) == 0 &&
	   ((dot11_action_vs_frmhdr_t *)body)->type == PROXD_AF_TYPE) {
	       ASSERT(((dot11_action_vs_frmhdr_t *)body)->category == DOT11_ACTION_CAT_VS);
	       wlc_proxd_recv_action_frames(wlc, bsscfg, hdr, body, body_len, wrxh, rspec);
		return;
	}
#endif

#ifdef WL_RELMCAST
	if (RMC_SUPPORT(wlc->pub) && wlc_rmc_check_actframe(wlc, body, body_len)) {
		wlc_rmc_recv_action_frames(wlc, hdr, body, body_len, wrxh);
		return;
	}
#endif /* WL_RELMCAST */

#ifdef WL_FTM
	if (PROXD_ENAB(wlc->pub) &&
		body_len >= DOT11_ACTION_VS_HDR_LEN &&
		memcmp(((dot11_action_vs_frmhdr_t *) body)->OUI, BRCM_PROP_OUI,
		DOT11_OUI_LEN) == 0 &&
		((dot11_action_vs_frmhdr_t *) body)->type == BRCM_FTM_VS_AF_TYPE) {
		ASSERT(((dot11_action_vs_frmhdr_t *) body)->category == DOT11_ACTION_CAT_VS);
		wlc_proxd_recv_action_frames(wlc, bsscfg, hdr, body, body_len, wrxh, rspec);
		return;
	}
#endif

}

static void
wlc_frameaction_public(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct dot11_management_header *hdr,
	uint8 *body, int body_len, wlc_d11rxhdr_t *wrxh, ratespec_t rspec)
{
#ifdef WLP2P
	/* P2P public action frame */
	if (P2P_ENAB(wlc->pub) &&
	    body_len >= P2P_PUB_AF_FIXED_LEN &&
	    ((wifi_p2p_pub_act_frame_t *)body)->action == P2P_PUB_AF_ACTION &&
	    bcmp(((wifi_p2p_pub_act_frame_t *)body)->oui, WFA_OUI, WFA_OUI_LEN) == 0 &&
	    ((wifi_p2p_pub_act_frame_t *)body)->oui_type == WFA_OUI_TYPE_P2P) {
		ASSERT(((wifi_p2p_pub_act_frame_t *)body)->category == DOT11_ACTION_CAT_PUBLIC);
		wlc_p2p_process_public_action(wlc->p2p, hdr, body, body_len);
		return;
	}
#endif /* WLP2P */
#ifdef WLTDLS
	if (TDLS_ENAB(wlc->pub) &&
		(body_len >= TDLS_PUB_AF_FIXED_LEN) &&
		(((tdls_pub_act_frame_t *)body)->action == TDLS_DISCOVERY_RESP)) {
		int8 rssi = wlc_lq_recv_rssi_compute(wlc, wrxh);
		wlc_tdls_process_discovery_resp(wlc->tdls, hdr, body, body_len, rssi);
		return;
	}
#endif /* WLTDLS */

	if (N_ENAB(wlc->pub) &&
	    body_len >= DOT11_ACTION_HDR_LEN)
		wlc_ht_publicaction(wlc->hti, cfg, hdr, body, body_len, wrxh);

#ifdef WL_NAN
	if (NAN_ENAB(wlc->pub) && BSSCFG_NAN_MGMT(cfg) &&
		wlc_nan_recv_action_frame(wlc, cfg, hdr, body, body_len, wrxh)) {
		/* if the return value was TRUE, it means the frame has been identified as
		   NAN action frame and processed. No further processing of the frame is required
		*/
		return;
	}
#endif /* WL_NAN */

#ifdef WL_FTM
	if (PROXD_ENAB(wlc->pub) && (body_len >= DOT11_ACTION_HDR_LEN)) {
		uint8 action = body[DOT11_ACTION_ACT_OFF];
		if (action == DOT11_PUB_ACTION_FTM_REQ ||
			action == DOT11_PUB_ACTION_FTM) {
			wlc_proxd_recv_action_frames(wlc, cfg, hdr, body, body_len, wrxh, rspec);
			return;
		}
	}
#else
	BCM_REFERENCE(rspec);
#endif
}


/*
 * Convert a restricted channel to non-restricted if we receive a Beacon
 * or Probe Response from an AP on the channel.
 *
 * An active AP on a channel is an indication that use of the channel is
 * allowed in the current country, so we can remove the channel restriction.
 */
static void
wlc_convert_restricted_chanspec(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh,
        struct dot11_management_header *hdr, uint8 *body, int bcn_len)
{
	struct dot11_bcn_prb *bcn = (struct dot11_bcn_prb *)body;
	uint8 *tag_params;
	int tag_params_len;
	bcm_tlv_t *ds_params;
	chanspec_t rx_chanspec;
	ht_add_ie_t *add_ie = NULL;
#ifdef WL11AC
	vht_op_ie_t vht_op_ie;
#endif
	uint16 cap = ltoh16(bcn->capability);
	char chanbuf[CHANSPEC_STR_LEN];

	BCM_REFERENCE(hdr);
	BCM_REFERENCE(chanbuf);

	/* only convert channel of BSS AP */
	if (!(cap & DOT11_CAP_ESS) || (cap & DOT11_CAP_IBSS))
		return;

	ASSERT(ISALIGNED((bcn), sizeof(uint16)));
	tag_params = ((uint8*)bcn + DOT11_BCN_PRB_LEN);
	tag_params_len = bcn_len - DOT11_BCN_PRB_LEN;

	/*
	* Determine the operational channel of the AP
	*/

	add_ie = wlc_read_ht_add_ies(wlc, tag_params, tag_params_len);
	if (add_ie) {
		/* use channel info from HT IEs if present */
		rx_chanspec = wlc_ht_chanspec(wlc, add_ie->ctl_ch, add_ie->byte1, NULL);

#ifdef WL11AC
		/* use channel info from VHT IEs if present */
		if (wlc_read_vht_op_ie(wlc->vhti, tag_params, tag_params_len, &vht_op_ie) != NULL) {
			rx_chanspec = wlc_vht_chanspec(wlc->vhti, &vht_op_ie,
					rx_chanspec, FALSE, FALSE, NULL);
		}
#endif
	}
	else if ((ds_params = bcm_parse_tlvs(tag_params, tag_params_len, DOT11_MNG_DS_PARMS_ID)) &&
		(ds_params->len == 1)) {
		/* use channel info from DS Params IE if present */
		rx_chanspec = CH20MHZ_CHSPEC(ds_params->data[0]);
	}
	else {
		/* use the channel to which we were tuned when we received the packet */
		rx_chanspec = wf_chspec_ctlchspec(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
			wlc->pub->corerev, RxChan));
	}

	/* if the channel of the Bcn/ProbeResp is invalid/unrestricted, nothing to do */
	if (!CH_NUM_VALID_RANGE(CHSPEC_CHANNEL(rx_chanspec)) ||
		wf_chspec_malformed(rx_chanspec)) {
		WL_REGULATORY(("wl%d:%s: bad chan fr bcnprb - so no-op\n",
			wlc->pub->unit, __FUNCTION__));
		return;
	} else if (!wlc_restricted_chanspec(wlc->cmi, rx_chanspec)) {
		return;
	}

	WL_REGULATORY(("wl%d:%s(): convert channel %s to non-restricted\n",
		wlc->pub->unit, __FUNCTION__, wf_chspec_ntoa(rx_chanspec, chanbuf)));

	wlc_clr_restricted_chanspec(wlc->cmi, rx_chanspec);

	/* clear the quiet bit if this channel is only quiet due to the restricted bit
	* and not radar.
	*/
	if (wlc_quiet_chanspec(wlc->cmi, rx_chanspec) &&
	    !wlc_radar_chanspec(wlc->cmi, rx_chanspec)) {
		wlc_clr_quiet_chanspec(wlc->cmi, rx_chanspec);

		/* if we are on this channel, we can unmute since we just cleared the quiet bit */
		if (rx_chanspec == wlc->chanspec)
			wlc_mute(wlc, OFF, 0);
	}

	return;
}

#ifdef STA

static bool
wlc_check_tbtt(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	d11regs_t *regs = wlc->regs;
	uint32 bi;
	uint32 tsf_l, tbtt;

	if (cfg != wlc->cfg)
		return FALSE;

	/* APSTA If an AP, don't sync to upstream TSF */
	if (AP_ACTIVE(wlc))
		return FALSE;

	if (cfg->pm->check_for_unaligned_tbtt)
		return FALSE;

	/* set threshold to be twice the target tbtt */
	bi = 2 * wlc->cfg->current_bss->beacon_period;
	if (wlc->cfg->BSS) {
		bi *= wlc->cfg->current_bss->dtim_period;

		if (wlc->bcn_li_dtim)
			bi *= wlc->bcn_li_dtim;
		else if (wlc->bcn_li_bcn)
			/* re-calculate bi based on bcn_li_bcn */
			bi = 2*wlc->bcn_li_bcn*wlc->cfg->current_bss->beacon_period;
	}

	bi <<= 10; /* convert TU into us */

	tsf_l = R_REG(wlc->osh, &regs->tsf_timerlow);
	tbtt = R_REG(wlc->osh, &regs->tsf_cfpstart);

	/*
	 * There is a rare case when tbtt wrap around and this check become true even
	 * though the value is in range. But a misfire in hours should be ok as this is
	 * a safety check anyway
	*/
	if ((!AIBSS_BSS_PS_ENAB(cfg) && (tbtt < tsf_l)) || (tbtt > tsf_l + bi)) {
		WL_ASSOC(("wl%d: %s: cfpstart = 0x%x, tsf = 0x%x, bi = 0x%x\n",
		          wlc->pub->unit, __FUNCTION__, tbtt, tsf_l, bi));
		return TRUE;
	}
	return FALSE;
}

static void
wlc_recv_PSpoll_resp(wlc_bsscfg_t *cfg, uint16 fc)
{
	wlc_info_t *wlc = cfg->wlc;

#ifdef WLP2P
	/* Check if PSpoll is still expected when presence period starts */
	if (BSS_P2P_ENAB(wlc, cfg))
		wlc_p2p_pspoll_resend_upd(wlc->p2p, cfg, FALSE);
#endif

	/* If there's more, send another PS-Poll */
	if (fc & FC_MOREDATA) {
		cfg->pm->pspoll_md_counter++;
		if ((cfg->pm->PM != PM_FAST) || !cfg->pm->pm2_md_sleep_ext ||
			(cfg->pm->pspoll_md_counter < cfg->pm->pspoll_md_cnt)) {

			if (wlc_sendpspoll(wlc, cfg) == FALSE) {
				WL_ERROR(("wl%d: %s: wlc_sendpspoll() failed\n",
					wlc->pub->unit, __FUNCTION__));
			}
			return;
		}
		if (!cfg->pm->PMblocked) {
			/* Reset PM2 Rx Pkts Count */
			cfg->pm->pm2_rx_pkts_since_bcn = 0;
			if ((cfg->pm->pm2_md_sleep_ext == PM2_MD_SLEEP_EXT_USE_BCN_FRTS) &&
				cfg->pm->pm2_bcn_sleep_ret_time) {
				cfg->pm->pm2_sleep_ret_threshold = cfg->pm->pm2_bcn_sleep_ret_time;
			}
			wlc_pm2_sleep_ret_timer_start(cfg, 0);
			if (PM2_RCV_DUR_ENAB(cfg)) {
				/* Start the receive throttle timer to limit how
				* long we can receive data before returning to
				* PS mode.
				*/
				wlc_pm2_rcv_timer_start(cfg);
			}
			cfg->pm->pspoll_md_counter = 0;
			return;
		}
	}
	cfg->pm->pspoll_md_counter = 0;

	/* Done, let the core go back to sleep */
	wlc_set_pspoll(cfg, FALSE);
}

static void
wlc_process_target_bss_beacon(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
                              struct dot11_bcn_prb *bcn,
                              uint8 *body, int body_len,
                              wlc_d11rxhdr_t *wrxh, uint8 *plcp)
{
	wlc_assoc_t *as = bsscfg->assoc;
	wlc_roam_t *roam = bsscfg->roam;
	bcm_tlv_t *ssid_ie = bcm_parse_tlvs(body + DOT11_BCN_PRB_LEN,
	                                    body_len - DOT11_BCN_PRB_LEN,
	                                    DOT11_MNG_SSID_ID);

	if (as->state == AS_WAIT_RCV_BCN) {
		WL_ASSOC(("wl%d: JOIN: got the desired beacon...update TBTT...\n",
		          WLCWLUNIT(wlc)));
		wlc_join_complete(bsscfg, wrxh, plcp, bcn, body_len);
	} else if (ASSOC_RECREATE_ENAB(wlc->pub) &&
	           as->state == AS_RECREATE_WAIT_RCV_BCN) {
		WL_ASSOC(("wl%d: JOIN: RECREATE got the desired beacon...\n",
		          WLCWLUNIT(wlc)));
		wlc_join_recreate_complete(bsscfg, wrxh, plcp, bcn, body_len);
	}
	/* we need to mark whether we can piggyback roam scans on the
	 * periodic scans for this ESS
	 */
	if (bsscfg->BSS && roam->piggyback_enab && SCAN_IN_PROGRESS(wlc->scan)) {
		bool ssid_match = wlc_scan_ssid_match(wlc->scan, ssid_ie, TRUE);
		if (ssid_match && !roam->roam_scan_piggyback) {
			WL_ASSOC(("wl%d: JOIN: AP is broadcasting its SSID, will "
			          "use periodic scans as full roams scans\n",
			          wlc->pub->unit));
			roam->roam_scan_piggyback = TRUE;
		} else if (!ssid_match && roam->roam_scan_piggyback) {
			WL_ASSOC(("wl%d: JOIN: AP is not broadcasting its SSID, "
			          "not using periodic scans as full roams scans\n",
			          wlc->pub->unit));
			roam->roam_scan_piggyback = FALSE;
		}
	}
}

/* 'bsscfg' is from wlc_bsscfg_find_by_bssid(wlc, &hdr->bssid) */
static void
wlc_roam_process_beacon(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_management_header *hdr, uint8 *body, int bcn_len)
{
	wlc_roam_t *roam = NULL;
	BCM_REFERENCE(hdr);
	BCM_REFERENCE(plcp);
	BCM_REFERENCE(bcn_len);
	BCM_REFERENCE(body);

	/* APSTA: Exit only if we don't have any STA processing to do */
	if ((wlc->stas_associated == 0) || (bsscfg == NULL))
		return;

	roam = bsscfg->roam;

	if (BSSCFG_STA(bsscfg) && bsscfg->BSS) {
		if (TRUE &&
#ifdef WLP2P
		    !BSS_P2P_ENAB(wlc, bsscfg) &&
#endif
		    !roam->off) {
			wlc_roamscan_start(bsscfg, WLC_E_REASON_LOW_RSSI);
		}
	}

	roam->time_since_bcn = 0;
#ifdef BCMDBG
	roam->tbtt_since_bcn = 0;
#endif

	roam->original_reason = WLC_E_REASON_INITIAL_ASSOC;

	/* If we are in an IBSS, indicate link up if we are currently down.
	 * bcns_lost indicates whether the link is marked 'down' or not
	 */
	if (!bsscfg->BSS && roam->bcns_lost == TRUE) {
		roam->bcns_lost = FALSE;
#ifdef WLAMPDU
		/* detect link up/down for IBSS so that ba sm can be cleaned up */
		scb_ampdu_cleanup_all(wlc, bsscfg);
#endif
		wlc_bss_mac_event(wlc, bsscfg, WLC_E_BEACON_RX, NULL,
		                  WLC_E_STATUS_SUCCESS, 0, 0, 0, 0);
	}

#ifdef WL_NAP
	if (NAP_ENAB(wlc->pub)) {
		wlc_stf_nap_rssi_thresh_handling(bsscfg);
	}
#endif /* WL_NAP */
}

/* Check for retrograde TSF; wraparound is treated as a retrograde TSF */
/* In PS mode the watchdog is driven (if configured to) from tbtt interrupt
 * in order to align the tbtt and the watchdog to save power. To keep the tbtt
 * interrupt w/o disruption from a retrograde TSF we need to sync up the cfpstart
 * if we see a beacon from our AP with a retrograde TSF. Also send a null data to
 * the AP to verify that we are still associated and to expect the AP to send a
 * disassoc/deauth if it does not consider us associated which will trigger a
 * roam eventually...
 */

static void
wlc_check_for_retrograde_tsf(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, wlc_d11rxhdr_t *wrxh,
	uint8 *plcp, struct dot11_management_header *hdr, uint8 *body)
{
	struct dot11_bcn_prb *bcn = (struct dot11_bcn_prb *)body;

	if (bsscfg != NULL && BSSCFG_STA(bsscfg) && bsscfg->BSS) {
		uint32 bcn_l, bcn_h;
		wlc_roam_t *roam = bsscfg->roam;

		bcn_l = ltoh32_ua(&bcn->timestamp[0]);
		bcn_h = ltoh32_ua(&bcn->timestamp[1]);
		if ((roam->tsf_h || roam->tsf_l) &&
			(bcn_h < roam->tsf_h ||
			(bcn_h == roam->tsf_h && bcn_l < roam->tsf_l))) {
				struct ether_addr *bssid = WLC_BSS_CONNECTED(bsscfg) ?
					&bsscfg->BSSID : &bsscfg->prev_BSSID;

			wlc_bss_mac_event(wlc, bsscfg, WLC_E_RETROGRADE_TSF,
				&hdr->bssid, 0, 0, 0, 0, 0);
			wlc_tsf_adopt_bcn(bsscfg, wrxh, plcp, bcn);
			wlc_sendnulldata(wlc, bsscfg, bssid, 0, 0, PRIO_8021D_BE,
				NULL, NULL);
		}
		roam->tsf_l = bcn_l;
		roam->tsf_h = bcn_h;
	}
}

static bool
wlc_ibss_coalesce_allowed(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	BCM_REFERENCE(cfg);
	return wlc->ibss_coalesce_allowed;
}

static void
wlc_reset_cfpstart(wlc_info_t *wlc, struct dot11_bcn_prb *bcn)
{
	uint32 bcn_l, bcn_h;
	uint32 tbtt_offset, bi;
	uint32 bi_us;
	uint32 tbtt_l, tbtt_h;

	bcn_l = ltoh32_ua(&bcn->timestamp[0]);
	bcn_h = ltoh32_ua(&bcn->timestamp[1]);

	bi = ltoh16(bcn->beacon_interval);

	/* set beacon interval to WECA maximum, if 0 */
	if (bi == 0)
		bi = 1024;

	bi_us = bi * DOT11_TU_TO_US;

	tbtt_offset = wlc_calc_tbtt_offset(bi, bcn_h, bcn_l);

	tbtt_l = bcn_l;
	tbtt_h = bcn_h;

	wlc_uint64_sub(&tbtt_h, &tbtt_l, 0, tbtt_offset);

	wlc_uint64_add(&tbtt_h, &tbtt_l, 0, bi_us);

	/* CFP start is the next beacon interval after timestamp */
	W_REG(wlc->osh, &wlc->regs->tsf_cfpstart, tbtt_l);

	WL_ERROR(("wl%d:%s(): current bcn_tsf 0x%x:0x%x, reset CFPSTART to 0x%x \n",
		wlc->pub->unit, __FUNCTION__, bcn_h, bcn_l, tbtt_l));

}

#endif /* STA */

static bool
wlc_is_vsaction(uint8 *hdr, int len)
{
	if (len < (DOT11_MGMT_HDR_LEN + DOT11_ACTION_HDR_LEN))
		return FALSE;

	hdr += DOT11_MGMT_HDR_LEN;	/* peek for category field */
	if (*hdr == DOT11_ACTION_CAT_VS)
		return TRUE;
	return FALSE;
}

#ifdef WLMESH
static bool
wlc_is_spaction(uint8 *hdr, int len)
{
	if (len < (DOT11_MGMT_HDR_LEN + DOT11_ACTION_HDR_LEN))
		return FALSE;

	hdr += DOT11_MGMT_HDR_LEN;	/* peek for category field */
	if (*hdr == DOT11_ACTION_CAT_SELFPROT)
		return TRUE;
	return FALSE;
}
#endif /* WLMESH */

static void
wlc_ht_bcn_scb_upd(wlc_info_t *wlc, struct scb *scb,
	ht_cap_ie_t *ht_cap, ht_add_ie_t *ht_op, obss_params_t *ht_obss)
{
	wlc_bsscfg_t *cfg;

	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	/* Make sure to verify AP also advertises WMM IE before updating HT IE
	 * Some AP's advertise HT IE in beacon without WMM IE, but AP's behaviour
	 * is unpredictable on using HT/AMPDU
	 */
	if (ht_cap != NULL &&
	    cfg->associated &&
	    (cfg->current_bss->flags & WLC_BSS_HT) &&
	    (cfg->current_bss->flags & WLC_BSS_WME)) {
#ifdef WLP2P
		if (P2P_CLIENT(wlc, cfg) && !SCB_IS_BRCM(scb))
			scb->flags2 |= SCB2_IGN_SMPS;
#endif
		wlc_ht_update_scbstate(wlc->hti, scb, ht_cap, ht_op, ht_obss);
#ifdef WLP2P
		scb->flags2 &= ~SCB2_IGN_SMPS;
#endif
		return;
	}

	if (SCB_HT_CAP(scb)) {
		wlc_ht_update_scbstate(wlc->hti, scb, NULL, NULL, NULL);
		return;
	}
}

#ifdef STA
static uint8
wlc_bw_update_required(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	chanspec_t bss_chspec, bool bss_allow40)
{
	uint8 bw_upd_reqd = 0;
#ifdef WL11AC
	int vht_allowed;

	/* VHT bandwidths are allowed if VHT is enabled generally for the bsscfg,
	 * and if we have not cleared the VHT flag for this particular BSS for
	 * some other reason.
	 */
	vht_allowed = (BSS_VHT_ENAB(wlc, bsscfg) &&
	               (bsscfg->current_bss->flags2 & WLC_BSS_VHT) != 0);
#endif /* WL11AC */
	if ((CHSPEC_BW(bsscfg->current_bss->chanspec) != CHSPEC_BW(bss_chspec)) &&
		(!BSSCFG_IBSS(bsscfg))) {
		/* don't update bsscfg bandwidth if STA is in ibss || aibss */
		if ((CHSPEC_IS20(bss_chspec)) ||
			(CHSPEC_IS40(bss_chspec) && bss_allow40) ||
#ifdef WL11AC
			(CHSPEC_IS80(bss_chspec) && vht_allowed) ||
			(CHSPEC_IS8080(bss_chspec) && vht_allowed) ||
#endif /* WL11AC */
			FALSE)
		{
			bw_upd_reqd = 1;
		}
	}
	return bw_upd_reqd;
}

static uint32
wlc_tbtt_calc(wlc_info_t *wlc, bool short_preamble, ratespec_t rspec,
	struct dot11_bcn_prb *bcn, bool adopt)
{
	uint32 bo = ltoh16(bcn->beacon_interval);
	uint32 bp;
	uint32 tbtt, tbtt_leeway;
	uint32 bcn_l, bcn_h, bcn_offset, bcn_tbtt;

	bcn_l = ltoh32_ua(&bcn->timestamp[0]);
	bcn_h = ltoh32_ua(&bcn->timestamp[1]);

	/* bad AP beacon frame, set beacon interval to WECA maximum */
	if (bo == 0) {
		WL_ERROR(("%s: !bad AP, change beacon period\n", __FUNCTION__));
		bp = 1024;
	} else
		bp = bo;

	if (!adopt && wlc->check_for_unaligned_tbtt) {
		uint tsf2preamble;

		/* use beacon start to determine if beacon is really unaligned
		 * [--------------------------[--------------------------[
		 *                                       -------TSF(1)
		 *                      -------TSF(2)
		 *                            <-- bcn_tbtt(based on tsf timestamp)
		 * <-- bcn_tbtt(based on beacon frame starting timestamp)
		 *
		 * case(1) is normal, tsf_offset can determine if beacon is unaligned
		 * case(2) is unusual, tsf_offset will result in aligned condition(false)
		 *   subtracting tsf2preamble out of bcn_l before calling wlc_calc_tbtt_offset
		 * will result in a large bcn_offset, which is out of WLC_MAX_TBTT_OFFSET and
		 * trigger adjustment
		 */
		tsf2preamble = wlc_compute_bcntsfoff(wlc, rspec, short_preamble, FALSE);
		wlc_uint64_sub(&bcn_h, &bcn_l, 0, tsf2preamble);
		WL_ASSOC(("wl%d: TBTT: tsf2preamble %d rspec x%x, short_preamble %d\n",
			wlc->pub->unit, tsf2preamble, rspec, short_preamble));
	}

	/* CFP start should be set to a future TBTT. Start by finding the most recent TBTT. */
	bcn_offset = wlc_calc_tbtt_offset(bp, bcn_h, bcn_l);

	/* TBTT = Beacon TSF Timestamp(uint64) - bcn_tsf_offset, but only interested in low 32 bits
	 */
	bcn_tbtt = bcn_l - bcn_offset;

#ifdef DEBUG_TBTT
	{
	uint32 tbtt2 = bcn_l - (bcn_l % ((uint32)bp << 10));
	/* if the tsf is still in 32 bits, we can check the calculation directly */
	if (tbtt2 != bcn_tbtt && bcn_h == 0) {
		WL_ERROR(("tbtt calc error, tbtt2 %d tbtt %d\n", tbtt2, bcn_tbtt));
	}
	}
#endif /* defined(DEBUG_TBTT) && !defined(linux) */

	WL_ASSOC(("wl%d: bcn_h 0x%x bcn_l 0x%x bcn_offset 0x%x bcn_tbtt 0x%x\n", wlc->pub->unit,
		bcn_h, bcn_l, bcn_offset, bcn_tbtt));

	if (adopt || !wlc->check_for_unaligned_tbtt) {
		WL_ASSOC(("wl%d: adopting aligned TBTT\n", wlc->pub->unit));
	} else {
		if (bcn_offset >> 10 >= (uint32)wlc->bcn_wait_prd - 1) {
			bcn_tbtt += bcn_offset;
			WL_ASSOC(("wl%d: adjust to unaligned (%d us) TBTT, new_tbtt 0x%x\n",
				wlc->pub->unit, bcn_offset, bcn_tbtt));
		} else {
			WL_ASSOC(("wl%d: adjust to aligned TBTT\n", wlc->pub->unit));
		}
	}

	/* Add a leeway to make sure we still have enough time in the future */
	tbtt_leeway = ROUNDUP((bcn_offset +
		((ISSIM_ENAB(wlc->pub->sih) ? TBTT_ALIGN_LEEWAY_TU_QT :
		TBTT_ALIGN_LEEWAY_TU) << 10)), (bp << 10));

	tbtt = bcn_tbtt + tbtt_leeway;

	return tbtt;
}

static void
wlc_tbtt_align(wlc_bsscfg_t *cfg, bool short_preamble, ratespec_t rspec, struct dot11_bcn_prb *bcn)
{
	wlc_info_t *wlc = cfg->wlc;
	uint32 tbtt;

	if (cfg != wlc->cfg)
		return;

	if (AP_ACTIVE(wlc))
		return;

	tbtt = wlc_tbtt_calc(wlc, short_preamble, rspec, bcn, FALSE);

	WL_ASSOC(("wl%d: %s: old cfpstart = 0x%x, new = 0x%x\n",
	          wlc->pub->unit, __FUNCTION__,
	          R_REG(wlc->osh, &wlc->regs->tsf_cfpstart), tbtt));

	W_REG(wlc->osh, &wlc->regs->tsf_cfpstart, tbtt);
}
#endif /* STA */

/* This function should be called only when receiving mgmt packets.
 * The channel returned will always be a 20MHz channel.
 */
uint16
wlc_recv_mgmt_rx_channel_get(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh)
{
	uint16 channel;

	channel = WLC_RX_CHANNEL(wlc->pub->corerev, &wrxh->rxhdr);

	/* This tells us that we received a 20MHz packet on a 40MHz channel */
	if (CHSPEC_IS40(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
		wlc->pub->corerev, RxChan))) {
		int is_upper;

		/* Channel reported by WLC_RX_CHANNEL() macro will be the 40MHz center channel.
		 * Since we are assuming the pkt is 20MHz for this fn,
		 * we need to find on which sideband the packet was received
		 * and adjust the channel value to reflect the true RX channel.
		 */
		if (WLCISACPHY(wlc->band)) {
			uint16 subband;
			uint16 frametype = (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
				wlc->pub->corerev, PhyRxStatus_0) &
				PRXS_FT_MASK(wlc->pub->corerev));

			if (ACREV_GE(wlc->band->phyrev, 32)) {
				subband = (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
					wlc->pub->corerev, PhyRxStatus_1) &
					PRXS1_ACPHY2_SUBBAND_MASK) >>
				        PRXS1_ACPHY2_SUBBAND_SHIFT;
			} else {
				subband = (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
					wlc->pub->corerev, PhyRxStatus_0) &
					PRXS0_ACPHY_SUBBAND_MASK) >>
				        PRXS0_ACPHY_SUBBAND_SHIFT;
			}

			/* For 20in40 BPHY frames the FinalBWClassification field indicates bandsel:
			 * 0 == 20L and 1 == 20U
			 */
			if (frametype == PRXS0_CCK)
				is_upper = (subband == PRXS_SUBBAND_BPHY_20U);
			else
				is_upper = (subband == PRXS_SUBBAND_20LU);
		} else {
			is_upper = (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
				wlc->pub->corerev, PhyRxStatus_0) &
				PRXS0_RXANT_UPSUBBAND) != 0;
		}

		if (is_upper) {
			channel = UPPER_20_SB(channel);
		} else {
			channel = LOWER_20_SB(channel);
		}
	} else if (CHSPEC_IS80(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
		wlc->pub->corerev, RxChan))) {
		uint16 subband;

		/* adjust the channel to be the 20MHz center at the low edge of the 80Hz channel */
		channel = channel - CH_40MHZ_APART + CH_10MHZ_APART;

		if (ACREV_GE(wlc->band->phyrev, 32)) {
			subband = (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
				wlc->pub->corerev, PhyRxStatus_1) &
				PRXS1_ACPHY2_SUBBAND_MASK) >>
				PRXS1_ACPHY2_SUBBAND_SHIFT;
		} else {
			subband = (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
				wlc->pub->corerev, PhyRxStatus_0) &
				PRXS0_ACPHY_SUBBAND_MASK) >> PRXS0_ACPHY_SUBBAND_SHIFT;
		}

		switch (subband) {
		case PRXS_SUBBAND_20LL:
			/* channel is correct */
			break;
		case PRXS_SUBBAND_20LU:
			channel += CH_20MHZ_APART;
			break;
		case PRXS_SUBBAND_20UL:
			channel += 2 * CH_20MHZ_APART;
			break;
		case PRXS_SUBBAND_20UU:
			channel += 3 * CH_20MHZ_APART;
			break;
		default:
			break;
		}
	}
	else if (CHSPEC_IS8080(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
		wlc->pub->corerev, RxChan))) {
		uint16 subband;

		/* lower nibble of subband is derived from  byte 1(bits 4 to 8) of PhyRxStatus
		 * and higher nibble of subband is derived from byte 9 of PhyRxStatus
		 */
		if (WLCISACPHY(wlc->band)) {
			if (ACREV_GE(wlc->band->phyrev, 32)) {
				subband = (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
					wlc->pub->corerev, PhyRxStatus_1) &
					PRXS1_ACPHY2_SUBBAND_MASK) >>
				        PRXS1_ACPHY2_SUBBAND_SHIFT;
			} else {
				subband = ((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
					wlc->pub->corerev, PhyRxStatus_0) &
					PRXS0_ACPHY_SUBBAND_MASK) >>
					PRXS0_ACPHY_SUBBAND_SHIFT);
				subband |= ((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
					wlc->pub->corerev, PhyRxStatus_4) &
					PRXS4_ACPHY_SUBBAND_MASK) << 4);
			}

			channel = wf_chspec_primary80_channel(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
				wlc->pub->corerev, RxChan)) -
				CH_40MHZ_APART + CH_10MHZ_APART;
			switch (subband) {
				case PRXS_SUBBAND_20LLL:
				case PRXS_SUBBAND_20ULL:
					/* channel is correct */
					break;
				case PRXS_SUBBAND_20LLU:
				case PRXS_SUBBAND_20ULU:
					channel += CH_20MHZ_APART;
					break;
				case PRXS_SUBBAND_20LUL:
				case PRXS_SUBBAND_20UUL:
					channel += 2 * CH_20MHZ_APART;
					break;
				case PRXS_SUBBAND_20LUU:
				case PRXS_SUBBAND_20UUU:
					channel += 3 * CH_20MHZ_APART;
					break;
				default:
					break;
			}
		}
	}
	else if (CHSPEC_IS160(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
		wlc->pub->corerev, RxChan))) {
		uint16 subband;

		/* adjust the channel to be the 20MHz center at the low edge of the 80Hz channel */
		channel = channel - CH_80MHZ_APART + CH_10MHZ_APART;

		/* lower nibble of subband is derived from  byte 1(bits 4 to 8) of PhyRxStatus 0
		 * and higher nibble of subband is derived from byte 9 0f of PhyRxStatus 0
		 */
		if (WLCISACPHY(wlc->band)) {
			if (ACREV_GE(wlc->band->phyrev, 32)) {
				subband = (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
					wlc->pub->corerev, PhyRxStatus_1) &
					PRXS1_ACPHY2_SUBBAND_MASK) >>
					PRXS1_ACPHY2_SUBBAND_SHIFT;
			} else {
				subband = ((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
					wlc->pub->corerev, PhyRxStatus_0) &
					PRXS0_ACPHY_SUBBAND_MASK) >>
					PRXS0_ACPHY_SUBBAND_SHIFT);
				subband |= ((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
					wlc->pub->corerev, PhyRxStatus_4) &
					PRXS4_ACPHY_SUBBAND_MASK) << 4);
			}
			switch (subband) {
			case PRXS_SUBBAND_20LLL:
				/* channel is correct */
				break;
			case PRXS_SUBBAND_20LLU:
				channel += CH_20MHZ_APART;
				break;
			case PRXS_SUBBAND_20LUL:
				channel += 2 * CH_20MHZ_APART;
				break;
			case PRXS_SUBBAND_20LUU:
				channel += 3 * CH_20MHZ_APART;
				break;
			case PRXS_SUBBAND_20ULL:
				channel += 4 * CH_20MHZ_APART;
				break;
			case PRXS_SUBBAND_20ULU:
				channel += 5 * CH_20MHZ_APART;
				break;
			case PRXS_SUBBAND_20UUL:
				channel += 6 * CH_20MHZ_APART;
				break;
			case PRXS_SUBBAND_20UUU:
				channel += 7 * CH_20MHZ_APART;
				break;
			default:
				break;
			}
		}
	}

	return (channel);
}

/* This function is called only when receiving mgmt packets.
 * The assumption made in this function is that the received packet is always on 20Mhz channel.
 * This assumption holds as long as we only call this function when receiving mgmt packets.
 */
void
wlc_recv_prep_event_rx_frame_data(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
                             wl_event_rx_frame_data_t *rxframe_data)
{
	uint16 channel;
	uint32 tsf, rate;
	int rssi;

	rxframe_data->version = hton16(BCM_RX_FRAME_DATA_VERSION);
	channel = wlc_recv_mgmt_rx_channel_get(wlc, wrxh);
	channel = CH20MHZ_CHSPEC(channel);
	rxframe_data->channel = hton16(channel);
	rssi = wlc_lq_recv_rssi_compute(wlc, wrxh);
	rxframe_data->rssi = hton32(rssi);
	tsf = wlc_recover_tsf32(wlc, wrxh);
	rxframe_data->mactime = hton32(tsf);
	rate = wlc_recv_compute_rspec(wlc->d11_info, wrxh, plcp);
	rxframe_data->rate = hton32(rate);
}

#ifdef STA
/** Calculate the local TSF time at beacon TS reception */
void
wlc_bcn_ts_tsf_calc(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, void *plcp, uint32 *tsf_h, uint32 *tsf_l)
{
	ratespec_t rspec;
	uint32 hdratime;

	/* 802.11 header airtime */
	rspec = wlc_recv_compute_rspec(wlc->d11_info, wrxh, plcp);
	hdratime = wlc_compute_bcn_payloadtsfoff(wlc, rspec);
	/* local TSF at past 802.11 header */
	wlc_read_tsf(wlc, tsf_l, tsf_h);
	wlc_recover_tsf64(wlc, wrxh, tsf_h, tsf_l);
	wlc_uint64_add(tsf_h, tsf_l, 0, hdratime);
}

void
wlc_tsf_adopt_bcn(wlc_bsscfg_t *cfg, wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_bcn_prb *bcn)
{
	wlc_info_t *wlc = cfg->wlc;
	uint32 bi;
	uint32 tsf_l, tsf_h;
	uint32 bcn_tsf_h, bcn_tsf_l;
	uint32 cfpstart;

#ifdef WLMCNX
	/* Infra STA uses SHM TSF offset, non h/w beacon IBSS uses SHM TSF offset too */
	if (MCNX_ENAB(wlc->pub) &&
		(cfg->BSS || !IBSS_HW_ENAB(cfg))) {
		wlc_mcnx_adopt_bcn(wlc->mcnx, cfg, wrxh, plcp, bcn);
		return;
	}
#endif

	/* primary STA or IBSS with hardware beacon enabled */
	if (cfg != wlc->cfg && !IBSS_HW_ENAB(cfg))
		return;

	/* We should sync to upstream AP if PSTA is configured
	 * Otherwise if just an AP, don't sync to upstream TSF
	 */
	if (!PSTA_ENAB(wlc->pub) && AP_ACTIVE(wlc))
		return;

	tsf_l = ltoh32_ua(&bcn->timestamp[0]);
	tsf_h = ltoh32_ua(&bcn->timestamp[1]);

	bi = ltoh16(bcn->beacon_interval);
	if (bi == 0)	/* bad AP, change to a valid value */
		bi = 1024;

	/* calc TBTT time in the core */
	cfpstart = wlc_tbtt_calc(wlc, FALSE, 0, bcn, TRUE);

	/* calculate the beacon TS reception time */
	wlc_bcn_ts_tsf_calc(wlc, wrxh, plcp, &bcn_tsf_h, &bcn_tsf_l);

	/* write beacon's timestamp directly to TSF timer */
	wlc_tsf_adj(wlc, cfg, tsf_h, tsf_l, bcn_tsf_h, bcn_tsf_l, cfpstart, bi << 10, TRUE);
}

/* difference between our TSF and TS field in beacon (ours - theirs) */
void
wlc_bcn_tsf_diff(wlc_bsscfg_t *cfg, wlc_d11rxhdr_t *wrxh, void *plcp, struct dot11_bcn_prb *bcn,
                  int32 *diff_h, int32 *diff_l)
{
	wlc_info_t *wlc = cfg->wlc;
	uint32 tsf_l, tsf_h;
	uint32 bcn_tsf_l, bcn_tsf_h;

	/* read the tsf from our chip */
#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub))
		/* remote TSF time */
		wlc_mcnx_read_tsf64(wlc->mcnx, cfg, &tsf_h, &tsf_l);
	else
#endif
	wlc_read_tsf(wlc, &tsf_l, &tsf_h);

	/* read the tsf from the beacon */
	bcn_tsf_l = ltoh32_ua(&bcn->timestamp[0]);
	bcn_tsf_h = ltoh32_ua(&bcn->timestamp[1]);

	/* if the bcn_tsf is later than the current tsf, then we know it is later than the
	 * tsf at the time of the beacon
	 */
	if (wlc_uint64_lt(tsf_h, tsf_l, bcn_tsf_h, bcn_tsf_l)) {
		WL_ASSOC(("wlc_bcn_tsf_later: current local time: %08x:%08x\n", tsf_h, tsf_l));
		WL_ASSOC(("wlc_bcn_tsf_later: beacon tsf time: %08x:%08x\n", bcn_tsf_h, bcn_tsf_l));
		goto diff;
	}

	/* local TSF time at beacon TS reception */
	wlc_bcn_ts_tsf_calc(wlc, wrxh, plcp, &tsf_h, &tsf_l);

#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub))
		/* remote TSF time */
		wlc_mcnx_l2r_tsf64(wlc->mcnx, cfg, tsf_h, tsf_l, &tsf_h, &tsf_l);
#endif

	if (wlc_uint64_lt(tsf_h, tsf_l, bcn_tsf_h, bcn_tsf_l)) {
		WL_ASSOC(("wlc_bcn_tsf_later:  local tsf time: %08x:%08x\n", tsf_h, tsf_l));
		WL_ASSOC(("wlc_bcn_tsf_later: beacon tsf time: %08x:%08x\n", bcn_tsf_h, bcn_tsf_l));
		goto diff;
	}

	/* calculate the difference */
diff:
	wlc_uint64_sub(&tsf_h, &tsf_l, bcn_tsf_h, bcn_tsf_l);
	*diff_h = tsf_h;
	*diff_l = tsf_l;
}

int
wlc_bcn_tsf_later(wlc_bsscfg_t *cfg, wlc_d11rxhdr_t *wrxh, void *plcp, struct dot11_bcn_prb *bcn)
{
	int32 diff_h, diff_l;

	wlc_bcn_tsf_diff(cfg, wrxh, plcp, bcn, &diff_h, &diff_l);
	return diff_h < 0;
}
#endif /* STA */

/* Parse a handful pre-selected IEs in a frame */
#include <wlc_ie_mgmt_lib.h>

static int
_wlc_pre_parse_sup_rates_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_rateset_t *sup = (wlc_rateset_t *)ctx;
	bcm_tlv_t *sup_rates = (bcm_tlv_t *)data->ie;

	if (sup == NULL)
		return BCME_OK;

	/* Supported Rates override */
	if (sup_rates != NULL && data->ie_len > TLV_BODY_OFF) {
		/* subtract the length for the elt from the total and
		 * we will add it back in below if needed
		 */
		if (sup_rates->len <= WLC_NUMRATES) {
			sup->count = sup_rates->len;
			bcopy(sup_rates->data, sup->rates, sup_rates->len);
		}
	}

	return BCME_OK;
}

static int
_wlc_pre_parse_ext_rates_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_rateset_t *ext = (wlc_rateset_t *)ctx;
	bcm_tlv_t *ext_rates = (bcm_tlv_t *)data->ie;

	if (ext == NULL)
		return BCME_OK;

	/* Extended Supported Rates override */
	if (ext_rates != NULL && data->ie_len > TLV_BODY_OFF) {
		/* subtract the length for the elt from the total and
		 * we will add it back in below if needed
		 */
		if (ext_rates->len <= WLC_NUMRATES) {
			ext->count = ext_rates->len;
			bcopy(ext_rates->data, ext->rates, ext_rates->len);
		}
	}

	return BCME_OK;
}

static int
_wlc_pre_parse_ds_parms_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	uint16 *chan = (uint16 *)ctx;
	bcm_tlv_t *dsparms = (bcm_tlv_t *)data->ie;

	if (chan == NULL || data->ie == NULL || data->ie_len <= TLV_BODY_OFF) {
		return BCME_OK;
	}

	/* check the freq. */
	if (dsparms != NULL &&
	    dsparms->len > 0)
		*chan = (uint8)dsparms->data[0];

	return BCME_OK;
}

static int
_wlc_pre_parse_ht_add_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	uint16 *chan = (uint16 *)ctx;
	bcm_tlv_t *ht_add = (bcm_tlv_t *)data->ie;

	if ((chan == NULL) || (*chan != 0xffff))
		return BCME_OK;

	/* check the freq. */
	if (ht_add != NULL &&
	    ht_add->len > 0)
		*chan = ((ht_add_ie_t *)(ht_add->data))->ctl_ch;

	return BCME_OK;
}

#ifdef STA
int
wlc_arq_pre_parse_frame(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *ies, uint ies_len, wlc_pre_parse_frame_t *ppf)
{
	/* tags must be sorted in ascending order */
	wlc_iem_tag_t parse_tag[] = {
		DOT11_MNG_RATES_ID,
		DOT11_MNG_EXT_RATES_ID,
	};
	wlc_iem_pe_t parse_cb[] = {
		{_wlc_pre_parse_sup_rates_ie, ppf->sup},
		{_wlc_pre_parse_ext_rates_ie, ppf->ext},
	};
	BCM_REFERENCE(wlc);
	ASSERT(ARRAYSIZE(parse_tag) == ARRAYSIZE(parse_cb));

	return wlc_ieml_parse_frame(cfg, ft, parse_tag, TRUE, parse_cb, ARRAYSIZE(parse_cb),
	                            NULL, NULL, ies, ies_len);
}
#endif /* STA */

int
wlc_scan_pre_parse_frame(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *ies, uint ies_len, wlc_pre_parse_frame_t *ppf)
{
	/* tags must be sorted in ascending order */
	wlc_iem_tag_t parse_tag[] = {
		DOT11_MNG_RATES_ID,
		DOT11_MNG_DS_PARMS_ID,
		DOT11_MNG_EXT_RATES_ID,
		DOT11_MNG_HT_ADD,
	};
	wlc_iem_pe_t parse_cb[] = {
		{_wlc_pre_parse_sup_rates_ie, ppf->sup},
		{_wlc_pre_parse_ds_parms_ie, ppf->chan},
		{_wlc_pre_parse_ext_rates_ie, ppf->ext},
		{_wlc_pre_parse_ht_add_ie, ppf->chan},
	};
	BCM_REFERENCE(wlc);
	ASSERT(ARRAYSIZE(parse_tag) == ARRAYSIZE(parse_cb));

	return wlc_ieml_parse_frame(cfg, ft, parse_tag, TRUE, parse_cb, ARRAYSIZE(parse_cb),
	                            NULL, NULL, ies, ies_len);
}

#if defined(WLTEST)
static uint8 wlc_rxpkt_rate_count(wlc_info_t *wlc, ratespec_t rspec)
{
	uint8 index = NUM_80211_RATES;
	BCM_REFERENCE(wlc);

	/* stat counter array:
	  * first 4 for b rates, 8 for ag, 32 for mcs
	  */

	/* update per-rate rx count */
	if (RSPEC_ISVHT(rspec)) {
		index = NUM_80211b_RATES + NUM_80211ag_RATES
			+ (rspec & RSPEC_VHT_MCS_MASK);
	} else if (RSPEC_ISHT(rspec)) {
		index = NUM_80211b_RATES + NUM_80211ag_RATES
			+ (rspec & RSPEC_RATE_MASK);
	} else if (RSPEC_ISLEGACY(rspec)) {
		switch (RSPEC2RATE(rspec)) {
		case WLC_RATE_1M:
			index = 3;
			break;
		case WLC_RATE_2M:
			index = 1;
			break;
		case WLC_RATE_5M5:
			index = 2;
			break;
		case WLC_RATE_6M:
			index = 7;
			break;
		case WLC_RATE_9M:
			index = 11;
			break;
		case WLC_RATE_11M:
			index = 0;
			break;
		case WLC_RATE_12M:
			index = 6;
			break;
		case WLC_RATE_18M:
			index = 10;
			break;
		case WLC_RATE_24M:
			index = 5;
			break;
		case WLC_RATE_36M:
			index = 9;
			break;
		case WLC_RATE_48M:
			index = 4;
			break;
		case WLC_RATE_54M:
			index = 8;
			break;
		default:
			break;
		}
	}

	/* If the indexis out of bound for some reason then put it to others */
	if (index > NUM_80211_RATES)
		index = NUM_80211_RATES;

	return index;
}
#endif 

#ifdef BCMSPLITRX
/* Look up scb from frminfo, required for Rx pktfetch cases.
 * Similar to scb lookup in wlc_recvdata
 */
void
wlc_pktfetch_get_scb(wlc_info_t *wlc, wlc_frminfo_t *f,
	wlc_bsscfg_t **bsscfg, struct scb **scb, bool promisc_frame, uint32 ctx_assoctime)
{
	struct ether_addr *bssid;
	uint bandunit;
	struct dot11_header *h = f->h;

	bandunit = CHSPEC_BANDUNIT(D11RXHDR_ACCESS_VAL(f->rxh,
		wlc->pub->corerev, RxChan));

	if (f->wds) {
		/* WDS frames */
		*scb = wlc_scbbssfindband(wlc, &h->a1, &h->a2,
			bandunit, bsscfg);
	} else {
		if ((f->fc & (FC_FROMDS | FC_TODS)) == 0) {
			/* IBSS case */
			bssid = &h->a3;
			*scb = wlc_scbibssfindband(wlc, &h->a2, bandunit, bsscfg);
		} else {
			if (f->fc & FC_TODS) {
				bssid = &h->a1;
			} else {
				bssid = &h->a2;
			}
#ifdef PSTA
			if (PSTA_ENAB(wlc->pub) && !f->ismulti)
				*bsscfg = wlc_bsscfg_find_by_hwaddr_bssid(wlc, &h->a1, bssid);
#endif
		}

		/* broadcast or multicast frames */
		if (*bsscfg == NULL)
			*bsscfg = wlc_bsscfg_find_by_bssid(wlc, bssid);

		/* Get scb info */
		if (*bsscfg && *scb == NULL)
			*scb = wlc_scbfindband(wlc, *bsscfg, &h->a2, bandunit);

	}

	/* Additional sanity for derived scb
	 * Make sure saved assoc time is same as derived scb assoc time
	 */
	if (*scb) {
		if ((*scb)->assoctime != ctx_assoctime)
			*scb = NULL;
	}
}
#endif /* BCMSPLITRX */

#if defined(PKTC_DONGLE) && defined(WLAMSDU)
static bool
wlc_pktc_dongle_rx_chainable(wlc_info_t *wlc, struct scb *scb, uint16 fc)
{
	if (BSSCFG_SAFEMODE(SCB_BSSCFG(scb)) ||
		(BSSCFG_STA(scb->bsscfg) && PROMISC_ENAB(wlc->pub)))
		return FALSE;

	/* Disable this for WDS now */
	if ((fc & (FC_TODS | FC_FROMDS)) == (FC_TODS | FC_FROMDS))
		return FALSE;

	if (!((scb->bsscfg->pm->PM == PM_OFF) || (scb->bsscfg->pm->PM == PM_FAST)))
		return FALSE;

#if defined(STA) && defined(BCMCCX)
	if (CCX_ENAB(wlc->pub) && STA_ACTIVE(wlc) && wlc->rm_info->rm_state->ccx->frame_active)
		return FALSE;
#endif /* STA */

	return TRUE;
}
#else /* PKTC_DONGLE & WLAMSDU */
#define wlc_pktc_dongle_rx_chainable(wlc, scb, fc)	TRUE
#endif /* PKTC_DONGLE & WLAMSDU */

#if defined(STA) && defined(PKTC_DONGLE)
static int32
wlc_pktc_wsec_process(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb, void *p,
	wlc_d11rxhdr_t *wrxh, struct dot11_header *h, struct ether_header *eh, uint32 *tsf_l,
	uint16 body_offset)
{
	int is_8021x = (eh->ether_type == HTON16(ETHER_TYPE_802_1X)) ? TRUE : FALSE;

	if (is_8021x &&	wlc_keymgmt_b4m4_enabled(wlc->keymgmt, bsscfg)) {
		if (wlc_is_4way_msg(wlc, p, ETHER_HDR_LEN - DOT11_LLC_SNAP_HDR_LEN, PMSG1)) {
			WL_WSEC(("wl%d: %s: allowing unencrypted 802.1x 4-Way M1\n",
				wlc->pub->unit, __FUNCTION__));
			wlc_keymgmt_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_M1_RX, bsscfg,
				NULL, NULL, NULL);
		}
	}

	if (BSSCFG_STA(bsscfg)) {
		bsscfg->pm->pm2_rx_pkts_since_bcn++;
		wlc_pm2_ret_upd_last_wake_time(bsscfg, tsf_l);
		wlc_update_sleep_ret(bsscfg, TRUE, FALSE, pkttotlen(wlc->osh, p), 0);
	}

#if (defined(BCMSUP_PSK) || defined(BCMCCX)) && defined(BCMINTSUP)
	if (is_8021x &&
	    SUP_ENAB(wlc->pub) &&
	    BSS_SUP_TYPE(wlc->idsup, bsscfg) != SUP_UNUSED) {

		eapol_header_t *eapol_hdr = (eapol_header_t *)eh;

		/* ensure that we have a valid eapol - header/length */
		if (!wlc_valid_pkt_field(wlc->osh, p, (void*)eapol_hdr,
			OFFSETOF(eapol_header_t, body)) ||
		    !wlc_valid_pkt_field(wlc->osh, p, (void*)&eapol_hdr->body[0],
			(int)ntoh16(eapol_hdr->length))) {
			WL_WSEC(("wl%d: %s: bad eapol - header too small.\n",
				wlc->pub->unit, __FUNCTION__));
			WLCNTINCR(wlc->pub->_cnt->rxrunt);
			return BCME_ERROR;
		}

		if (wlc_sup_eapol(wlc->idsup, bsscfg,
			eapol_hdr, ((h->fc & HTOL16(FC_WEP)) != 0)))
			return BCME_ERROR;
	}
#endif /* BCMSUP_PSK || BCMCCX && BCMINTSUP */

#if defined(BCMAUTH_PSK)
	if (is_8021x &&
	    BCMAUTH_PSK_ENAB(wlc->pub) &&
	    BSS_AUTH_TYPE(wlc->authi, bsscfg) != AUTH_UNUSED) {

		eapol_header_t *eapol_hdr = (eapol_header_t *)eh;

		/* ensure that we have a valid eapol - header/length */
		if (!wlc_valid_pkt_field(wlc->osh, p, (void*)eapol_hdr,
				OFFSETOF(eapol_header_t, body)) ||
		    !wlc_valid_pkt_field(wlc->osh, p, (void*)&eapol_hdr->body[0],
				(int)ntoh16(eapol_hdr->length))) {
			WL_WSEC(("wl%d: %s: bad eapol - header too small.\n",
			         wlc->pub->unit, __FUNCTION__));
			WLCNTINCR(wlc->pub->_cnt->rxrunt);
			return BCME_ERROR;
		}
		if (wlc_auth_eapol(wlc->authi, bsscfg, eapol_hdr,
				((h->fc & HTOL16(FC_WEP)) != 0), scb))
			return BCME_ERROR;
	}
#endif /* BCMAUTH_PSK */

#ifdef BCMCCX
	if (CCX_ENAB(wlc->pub) && (ntoh16(eh->ether_type) <= ETHER_MAX_DATA) &&
		BSSCFG_STA(bsscfg) && bsscfg->BSS && !BSS_P2P_ENAB(wlc, bsscfg) &&
		(PKTLEN(wlc->osh, p) >=
		(ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN + CCX_IAPP_HDR_LEN) &&
		!bcmp((uint8*)eh + ETHER_HDR_LEN, CISCO_AIRONET_SNAP,
		DOT11_LLC_SNAP_HDR_LEN)) &&
		bsscfg->associated) {
		struct wlc_frminfo f;
		f.p = p;
		f.h = h;
		f.pbody = (uint8*)h + body_offset;
		f.len = PKTLEN(wlc->osh, p);
		f.body_len = f.len - body_offset;
		f.rxh = &wrxh->rxhdr;
		f.wrxh = wrxh;
		if ((h->fc & htol16(FC_TODS)) == 0) {
			f.sa = &h->a3;
			f.da = &h->a1;
		} else {
			f.sa = &h->a2;
			f.da = &h->a3;
		}

		if (wlc_ccx_chk_iapp_frm(wlc->ccx, &f, bsscfg))
			return BCME_ERROR;
	}
#endif /* BCMCCX */

	return BCME_OK;
}
#else	/* STA && PKTC_DONGLE */
#define wlc_pktc_wsec_process(wlc, bsscfg, scb, p, wrxh, h, eh, tsf_l, body_offset) BCME_OK
#endif	/* STA && PKTC_DONGLE */

#if defined(PKTC) || defined(PKTC_DONGLE)
static int32 BCMFASTPATH
wlc_wsec_recv(wlc_info_t *wlc, struct scb *scb, void *p, d11rxhdr_t *rxh,
	struct dot11_header *h, wlc_key_t **key, wlc_key_info_t *key_info,
	wlc_rfc_t *rfc, uint16 *body_offset)
{
	struct wlc_frminfo f;
	int err = BCME_OK;

	/* optimizn: use provided key, if available */
	ASSERT(key != NULL && key_info != NULL);
	if (*key != NULL) {
		err = wlc_key_rx_mpdu(*key, p, rxh);
		goto done;
	}

	f.p = p;
	f.h = h;
	f.fc = ltoh16(h->fc);

	f.rx_wep = f.fc & FC_WEP;
	ASSERT(f.rx_wep);

	f.pbody = (uint8*)h + *body_offset;
	f.len = PKTLEN(wlc->osh, p);
	f.body_len = f.len - *body_offset;
	f.totlen = pkttotlen(wlc->osh, p) - *body_offset;
	f.ismulti = FALSE;
	f.rxh = rxh;
	f.WPA_auth = scb->WPA_auth;
	f.isamsdu = (*(uint16 *)(f.pbody - 2) & htol16(QOS_AMSDU_MASK)) != 0;
	f.prio = rfc->prio;

	WL_NONE(("wl%d: %s: RxStatus1 0x%x\n", wlc->pub->unit, __FUNCTION__,
		D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev, RxStatus1)));

	/* the key, if any, will be provided by wlc_keymgmt_recvdata */
	if ((h->fc & htol16(FC_TODS)) == 0) {
		f.sa = &h->a3;
		f.da = &h->a1;
	} else {
		f.sa = &h->a2;
		f.da = &h->a3;
	}

	/* the key, if any, will be provided by wsec_recvdata */
	f.key = NULL;
	err = wlc_keymgmt_recvdata(wlc->keymgmt, &f);
	if (err != BCME_OK)
		goto done;

	*key = f.key;
	*key_info = f.key_info;

	/* TKIP needs mic check etc. not handled here */
	ASSERT(f.key_info.algo != CRYPTO_ALGO_TKIP);

done:
	if (err != BCME_OK) {
		WLCNTSCB_COND_INCR((f.key_info.algo != CRYPTO_ALGO_OFF) ? 1 : 0,
			scb->scb_stats.rx_decrypt_failures);
	} else {
		*body_offset += key_info->iv_len; /* strip iv from body */
		WLCNTSCB_COND_INCR((key_info->algo != CRYPTO_ALGO_OFF) ? 1 : 0,
			scb->scb_stats.rx_decrypt_succeeds);
	}

	return err;
}

#define PKTC_MIN_FRMLEN	(D11_PHY_HDR_LEN + DOT11_A3_HDR_LEN + DOT11_QOS_LEN + DOT11_FCS_LEN)

bool BCMFASTPATH
wlc_rxframe_chainable(wlc_info_t *wlc, void **pp, uint16 index)
{
	void *p = *pp;
	wlc_d11rxhdr_t *wrxh = (wlc_d11rxhdr_t *)PKTDATA(wlc->osh, p);
	d11rxhdr_t *rxh = &wrxh->rxhdr;
	struct dot11_header *h;
	uint16 fc, prio = 0, seq, pad = 0;
	wlc_rfc_t *rfc;
	struct scb *scb;
	bool amsdu, chainable = TRUE;
	uchar *pbody;
	/* Can assume rxh is in host byte order. This code is only called when
	 * driver is not split. When driver is not split, endian conversion of
	 * receive status is done in wlc_bmac_recv().
	 */
	if (RXS_SHORT_ENAB(wlc->pub->corerev) && (D11RXHDR_ACCESS_VAL(rxh,
		wlc->pub->corerev, dma_flags) & RXS_SHORT_MASK)) {
		/* short rx status received */
		uint16 *mrxs;

		mrxs = wlc_get_mrxs(wlc, rxh);

		pad = (*mrxs & RXSS_PBPRES) ? 2 : 0;
		/* short rx status is for AMSDU */
		amsdu = TRUE;
		if ((*mrxs & RXSS_AMSDU_MASK) == 0) {
			WL_ERROR(("wl%d:%s Short Rx Status does not have AMSDU bit set. MRXS:%#x\n",
				wlc->pub->unit, __FUNCTION__, *mrxs));
#ifdef BCMDBG
			wlc_print_hdrs(wlc, "rxpkt hdr (invalid short rxs)", NULL, NULL,
				wrxh, 0);
			prhex("rxpkt body (invalid short rxs)", PKTDATA(wlc->osh, p),
				PKTLEN(wlc->osh, p));
#endif /* BCMDBG */
			ASSERT((*mrxs & RXSS_AMSDU_MASK) != 0);
		}
	} else {
		/* long rx status received */
		pad = ((D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev,
			RxStatus1) & RXS_PBPRES) ? 2 : 0);
		amsdu = ((D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
			wlc->pub->corerev, RxStatus2) &
			RXS_AMSDU_MASK) != 0);
	}

#ifdef WLAMSDU
	if (amsdu) {
		if (!wlc->_rx_amsdu_in_ampdu)
			return FALSE;

		/* check, qualify and chain MSDUs */
		*pp = wlc_recvamsdu(wlc->ami, wrxh, p, &pad, TRUE);

		if (*pp != NULL) {
			/* amsdu deagg successful */
			p = *pp;
		} else {
			/* non-tail or out-of-order sub-frames */
			return TRUE;
		}
	} else
		wlc_amsdu_flush(wlc->ami);

#endif /* WLAMSDU */

	h = (struct dot11_header *)(PKTDATA(wlc->osh, p) + wlc->hwrxoff + pad + D11_PHY_HDR_LEN);

	if (STAMON_ENAB(wlc->pub) && STA_MONITORING(wlc, &h->a2))
		return FALSE;
	fc = ltoh16(h->fc);
	seq = ltoh16(h->seq);

	if ((FC_TYPE(fc) != FC_TYPE_DATA) ||
	    (PKTLEN(wlc->osh, p) - wlc->hwrxoff <= (uint)PKTC_MIN_FRMLEN + pad)) {
		return FALSE;
	}

	rfc = PKTC_RFC(wlc->pktc_info, fc & FC_TODS);
	scb = rfc->scb;
	/* if chaining is not allowed for rfc scb return false */
	if ((scb == NULL) || !SCB_PKTC_ENABLED(scb) || (scb->pktc_pps <= 16))
		return FALSE;

	/* proceed only if packet is for same scb as previous */
	if (PKTC_HSCB(wlc->pktc_info) == NULL) {
		PKTC_HSCB_SET(wlc->pktc_info, scb);
	} else if (scb != PKTC_HSCB(wlc->pktc_info)) {
		return FALSE;
	}

	if (!wlc_pktc_dongle_rx_chainable(wlc, scb, fc))
		return FALSE;

	if (!amsdu) {
		uint8 *da;
#ifdef PSTA
		da = (uchar *)(BSSCFG_STA(rfc->bsscfg) ?
		              (PSTA_IS_REPEATER(wlc) ? &rfc->ds_ea : &h->a1) : &h->a3);
#else
		da = (uchar *)(BSSCFG_STA(rfc->bsscfg) ? &h->a1 : &h->a3);
#endif
		if (ETHER_ISNULLDEST(da))
			return FALSE;

		/* Init DA for the first valid packet of the chain */
		if (PKTC_HDA(wlc->pktc_info) == NULL)
			PKTC_HDA_SET(wlc->pktc_info, (struct ether_addr *)da);

		chainable = TRUE &&
#ifdef HNDCTF
			CTF_HOTBRC_CMP(wlc->pub->brc_hot, da, rfc->wlif_dev) &&
#else
			!ETHER_ISMULTI(da) &&
#endif /* ! HNDCTF */
			!eacmp(PKTC_HDA(wlc->pktc_info), da);
	}

	pbody = (uchar*)h + DOT11_A3_HDR_LEN;

	chainable = (chainable && !eacmp(&h->a1, &rfc->bsscfg->cur_etheraddr) &&
	             !eacmp(&h->a2, &scb->ea));

	chainable = (chainable &&
	             (FC_SUBTYPE_ANY_QOS((fc & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT) &&
	             (prio = QOS_PRIO(ltoh16_ua(pbody)), (prio == rfc->prio)) &&
	             (!FC_SUBTYPE_ANY_NULL((fc & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT)) &&
	             ((seq & FRAGNUM_MASK) == 0) && ((fc & FC_MOREFRAG) == 0) &&
	             (((fc & FC_RETRY) == 0) || ((scb->seqctl[prio] >> 4) + index != (seq >> 4)))));

	if (chainable && wlc_ampdu_chainable(wlc->ampdu_rx, p, scb, seq, prio))
		return TRUE;

	WL_NONE(("wl%d: %s: pkt %p not chainable\n", wlc->pub->unit,
		__FUNCTION__, OSL_OBFUSCATE_BUF(p)));
	WL_NONE(("wl%d: %s: prio %d rfc seq %d pkt seq %d index %d\n",  wlc->pub->unit,
	         __FUNCTION__, prio, scb->seqctl[prio] >> 4, seq >> 4, index));
	return FALSE;
}

/* Receive descriptor is already in host byte order */
void BCMFASTPATH
wlc_sendup_chain(wlc_info_t *wlc, void *head)
{
	struct dot11_llc_snap_header *lsh;
	struct ether_header *eh = NULL;
	struct dot11_header *h;
	wlc_d11rxhdr_t *wrxh;
	d11rxhdr_t *rxh;
	ratespec_t rspec;
	uint16 body_offset, count = 0;
	uint pkt_len, byte_count = 0;
	wlc_rfc_t *rfc = NULL;
	struct scb *scb = NULL;
	wlc_bsscfg_t *bsscfg = NULL;
	void *temp, *next, *p = head, *prev = NULL;
	uint8 *plcp;
	bool chained_sendup;
	wlc_key_t *key = NULL;
	wlc_key_info_t key_info;
	wlc_rx_status_t toss_reason = WLC_RX_STS_TOSS_UNKNOWN;

	uint16 body_offset_sav = 0;
	uint16 body_offset_diff = 0;

#ifdef PKTC_DONGLE
	bool process_head_pkt = TRUE;
	uint32 tsf_l;
#endif
#ifdef PKTC_INTRABSS
	bool intrabss_fwd = FALSE;
#endif
	int err;
	bool is_llc_snap_pkt = FALSE;
#ifdef BCMSPLITRX
	bool need_pktfetch = FALSE;
#endif

#if defined(WL11AC) && defined(WL_MU_RX) && defined(WLCNT)
	uint8 gid;
#endif

#if defined(STA) && defined(PKTC_DONGLE)
	uint32 prev_cnt = 0;
#endif

#ifdef PKTC_DONGLE
	tsf_l = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);
#endif

	chained_sendup = (wlc->pktc_info->policy == PKTC_POLICY_SENDUPC);

	memset(&key_info, 0, sizeof(key_info));
	while (p != NULL) {
		wrxh = (wlc_d11rxhdr_t *)PKTDATA(wlc->osh, p);

		/* If short rx status, assume start of an AMSDU chain. Walk the AMSDU chain
		 * to the end. Last frag has long rx status. Use that rx status to process
		 * AMSDU chain.
		 */
		if (RXS_SHORT_ENAB(wlc->pub->corerev) &&
			(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, wlc->pub->corerev,
			dma_flags) & RXS_SHORT_MASK)) {
			void *amsdu_frag = p;
			while (PKTNEXT(wlc->osh, amsdu_frag)) {
				amsdu_frag = PKTNEXT(wlc->osh, amsdu_frag);
				wrxh = (wlc_d11rxhdr_t*) PKTDATA(wlc->osh, amsdu_frag);
			}
		}
#ifdef PKTC_DONGLE
		wrxh->tsf_l = htol32(tsf_l);
#endif
		rxh = &wrxh->rxhdr;
		plcp = PKTPULL(wlc->osh, p, wlc->hwrxoff +
			((D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev, RxStatus1) &
			htol16(RXS_PBPRES)) ? 2 : 0));
		rspec = wlc_recv_compute_rspec(wlc->d11_info, wrxh, plcp);

		/* Save the rspec in pkttag */
		WLPKTTAG(p)->rspec = rspec;
#ifdef EXT_STA
		if (WLEXTSTA_ENAB(wlc->pub)) {
			WLPKTTAG(p)->rxchannel =
			(uint8)CHSPEC_CHANNEL(D11RXHDR_ACCESS_VAL(rxh,
				wlc->pub->corerev, RxChan));
		}
#endif /* EXT_STA */

		h = (struct dot11_header *)PKTPULL(wlc->osh, p, D11_PHY_HDR_LEN);
		body_offset = DOT11_A3_HDR_LEN + DOT11_QOS_LEN;
		pkt_len = pkttotlen(wlc->osh, p);

		if ((D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev, RxStatus2) &
			RXS_AMSDU_MASK) == 0) {
			/* For split RX case, FCS bytes are in host */
			/* drop length of host fragment */
			PKTFRAG_TRIM_TAILBYTES(wlc->osh, p, DOT11_FCS_LEN, TAIL_BYTES_TYPE_FCS);
		} else {
			ASSERT(WLPKTTAG(p)->flags & WLF_HWAMSDU);
		}

		rfc = PKTC_RFC(wlc->pktc_info, h->fc & HTOL16(FC_TODS));
		scb = rfc->scb;

		WLPKTTAGSCBSET(p, scb);
		ASSERT(scb != NULL);
		ASSERT(!SCB_A4_DATA(scb));
		bsscfg = SCB_BSSCFG(scb);
#ifdef WL_MIMOPS_CFG
		if (WLC_MIMOPS_ENAB(wlc->pub)) {
			wlc_mimo_ps_cfg_t *mimo_ps_cfg = wlc_stf_mimo_ps_cfg_get(bsscfg);
			if (WLC_STF_LEARNING_ENABLED(mimo_ps_cfg))
				wlc_stf_update_mimo_ps_cfg_data(bsscfg, rspec);
		}
#endif /* WL_MIMOPS_CFG */

		/* update per-scb stat counters */
		WLCNTSCBINCR(scb->scb_stats.rx_ucast_pkts);
		WLCNTSCBADD(scb->scb_stats.rx_ucast_bytes, pkt_len);
#ifdef WL_RX_STALL
		wlc_rx_healthcheck_update_counters(wlc, WME_PRIO2AC(PKTPRIO(p)),
			scb, bsscfg, WLC_RX_STS_OK, 1);
#endif
#ifdef WL11N
		/* for first MPDU in an A-MPDU collect ratesel stats for antsel */
		if (plcp[0] | plcp[1] | plcp[2]) {
			wlc_scb_ratesel_upd_rxstats(wlc->wrsi, rspec, D11RXHDR_ACCESS_VAL(rxh,
				wlc->pub->corerev, RxStatus2));
			WLCNTSCBSET(scb->scb_stats.rx_rate, rspec);
		}
#else
		WLCNTSCBSET(scb->scb_stats.rx_rate, rspec);
#endif

#if defined(WL11AC) && defined(WL_MU_RX) && defined(WLCNT)
	if (MU_RX_ENAB(wlc)) {
		/* In case of AMPDU, since PLCP is valid only on first MPDU in an AMPDU,
		 * a MU-PLCP marks the start of an MU-AMPDU, and end is determined my
		 * another SU/MU PLCP. Till this duration we count all MPDUs recieved as
		 * MU MPDUs.
		 */
		if ((D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev,
			PhyRxStatus_0) & PRXS_FT_MASK(wlc->pub->corerev)) == PRXS0_STDN) {
			if (plcp[0] | plcp[1] | plcp[2]) {
				gid = wlc_vht_get_gid(plcp);
				if ((gid > VHT_SIGA1_GID_TO_AP) &&
					(gid < VHT_SIGA1_GID_NOT_TO_AP)) {
					WLC_UPDATE_MURX_INPROG(wlc, TRUE);
				} else {
					WLC_UPDATE_MURX_INPROG(wlc, FALSE);
				}

			}

			if (WLC_GET_MURX_INPROG(wlc))
				WLCNTINCR(wlc->pub->_cnt->rxmpdu_mu);
		} else {
			WLC_UPDATE_MURX_INPROG(wlc, FALSE);
		}
	}
#endif  /* defined(WL11AC) && defined(WL_MU_RX) && defined(WLCNT) */

		/* Now that the frame is accepted, update some rx packet related data */
#ifdef WLFCTS
		if (WLFCTS_ENAB(wlc->pub) && (wlc->wlfc_flags & WLFC_FLAGS_PKT_STAMP_SIGNALS)) {
			WLPKTTAG(p)->seq = ltoh16(h->seq);

			if (h->fc & htol16(FC_RETRY))
				WLPKTTAG(p)->rspec |= WLFC_TSFLAGS_RX_RETRY;
			if (bsscfg->pm->PMenabled || bsscfg->pm->PMpending)
				WLPKTTAG(p)->rspec |= WLFC_TSFLAGS_PM_ENABLED;
			WLPKTTAG(p)->shared.rx_tstamp = wlc_recover_tsf32(wlc, wrxh);
		}
#endif /* WLFCTS */

#ifdef PKTC_DONGLE
		/* for pktc dongle only */
		if (bsscfg->roam->time_since_bcn != 0) {
#ifdef EVENT_LOG_COMPILE
			EVENT_LOG(EVENT_LOG_TAG_BEACON_LOG, "Beacon miss reset by RXC");
#endif
			bsscfg->roam->time_since_bcn = 0;
		}

		/* compute the RSSI from d11rxhdr and record it in wlc_rxd11hr */
		phy_rssi_compute_rssi((phy_info_t *)WLC_PI(wlc), wrxh);

		if (process_head_pkt == TRUE) {
			rx_lq_samp_t lq_samp;
			wlc_lq_sample(wlc, bsscfg, scb, wrxh, TRUE, &lq_samp);
			WLPKTTAG(p)->pktinfo.misc.rssi = lq_samp.rssi;
			WLPKTTAG(p)->pktinfo.misc.snr = lq_samp.snr;

			process_head_pkt = FALSE;
		}
#endif /* PKTC_DONGLE */

		/* save the offset as the security portion will be added */
		body_offset_sav = body_offset;

		next = PKTCLINK(p);
		if ((h->fc & htol16(FC_WEP)) ||
		    (WSEC_ENABLED(bsscfg->wsec) && bsscfg->wsec_restrict)) {
			/* disable replay check for all but the first in the chain */
			if (prev != NULL)
				WLPKTTAG(p)->flags |= WLF_RX_PKTC_NOTFIRST;
			if (next != NULL)
				WLPKTTAG(p)->flags |= WLF_RX_PKTC_NOTLAST;

			err = wlc_wsec_recv(wlc, scb, p, rxh, h, &key, &key_info,
				rfc, &body_offset);

			/* clear pktc pkt tags regardless */
			WLPKTTAG(p)->flags &= ~(WLF_RX_PKTC_NOTFIRST |
				WLF_RX_PKTC_NOTLAST);

			if (err != BCME_OK) {
				/* remove the packet from chain */
#ifdef BCMSPLITRX
				if (err == BCME_DATA_NOTFOUND) {
					/* Push data ptr back to before the processing above */
					PKTPUSH(wlc->osh, p, wlc->hwrxoff +
						((D11RXHDR_ACCESS_VAL(rxh,
						wlc->pub->corerev, RxStatus1) &
						htol16(RXS_PBPRES)) ? 2 : 0) +
						D11_PHY_HDR_LEN);
					need_pktfetch = TRUE;
				}
#endif
				toss_reason = WLC_RX_STS_TOSS_DECRYPT_FAIL;
				goto toss;
			}
		}

		/* if lsh header available, then check for pktfetch required */
		lsh = (struct dot11_llc_snap_header *) (PKTDATA(wlc->osh, p) + body_offset
			+ ((WLPKTTAG(p)->flags & WLF_HWAMSDU) ? ETHER_HDR_LEN : 0));
		if (lsh->dsap == 0xaa && lsh->ssap == 0xaa && lsh->ctl == 0x03 &&
		    lsh->oui[0] == 0 && lsh->oui[1] == 0 &&
		    ((lsh->oui[2] == 0x00 && !SSTLOOKUP(ntoh16(lsh->type))) ||
		     (lsh->oui[2] == 0xf8 && SSTLOOKUP(ntoh16(lsh->type))))) {
			is_llc_snap_pkt = TRUE;
#ifdef BCMSPLITRX
			if (wlc_sendup_chain_pktfetch_required(wlc, bsscfg, p, body_offset)) {
				/* Push data ptr back to before the processing above */
				PKTPUSH(wlc->osh, p, wlc->hwrxoff +
					((D11RXHDR_ACCESS_VAL(rxh,
					wlc->pub->corerev, RxStatus1) &
					htol16(RXS_PBPRES)) ? 2 : 0) +
					D11_PHY_HDR_LEN);
				need_pktfetch = TRUE;
				goto toss;
			}
#endif /* BCMSPLITRX */
		}

#ifdef WLC_SW_DIVERSITY
		if (WLSWDIV_ENAB(wlc) && WLSWDIV_BSSCFG_SUPPORT(bsscfg)) {
			wlc_swdiv_rxpkt_recv(wlc->swdiv, scb, wrxh,
				RSPEC_ISCCK(rspec));
		}
#endif /* WLC_SW_DIVERSITY */

		/* update cached seqctl */
		scb->seqctl[rfc->prio] = ltoh16(h->seq);

		/* WME: set packet priority based on first received MPDU */
		PKTSETPRIO(p, rfc->prio);

		wlc_ampdu_update_rxcounters(wlc->ampdu_rx,
			D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev,
			PhyRxStatus_0) & PRXS_FT_MASK(wlc->pub->corerev),
			scb, h, p, rfc->prio);

#if (defined(WL_MU_RX) && defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || \
	defined(BCMDBG_MU) || defined(BCMDBG_DUMP)))
		if (MU_RX_ENAB(wlc)) {
			wlc_murx_update_rxcounters(wlc->murx,
				D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev, PhyRxStatus_0) &
				PRXS_FT_MASK(wlc->pub->corerev), scb, h);
		}
#endif

#ifdef MCAST_REGEN
		/* apply WMF reverse translation only the STA side */
		if (chained_sendup && MCAST_REGEN_ENAB(bsscfg) && BSSCFG_STA(bsscfg))
			chained_sendup = FALSE;
#endif

		body_offset_diff = (body_offset - body_offset_sav);

#ifdef WLAMSDU
		if (WLPKTTAG(p)->flags & WLF_HWAMSDU) {
			PKTPULL(wlc->osh, p, body_offset);
			if (wlc_amsdu_pktc_deagg_hw(wlc->ami, &p, rfc,
				&count, &chained_sendup, scb, body_offset_diff) != BCME_OK) {
				toss_reason = WLC_RX_STS_TOSS_BAD_DEAGG_SF_LEN;
				goto toss;
			}
		} else
#endif /* WLAMASDU */
		{
			if (WLEXTSTA_ENAB(wlc->pub) &&
				(bsscfg->wlcif->if_flags & WLC_IF_PKT_80211)) {

				/* Interfaces that use 802.11 format. */
				/* Remove the encrypted IV header */
				if (h->fc & FC_WEP) {
					/* only if encrytption and qos are enabled offsets
					 * are different
					 * remove the encryption header  and qos info
					 */
					uchar *dst;

					dst = (uchar *)PKTDATA(wlc->osh, p) + body_offset_diff;
					memmove(dst, (uchar *)PKTDATA(wlc->osh, p),
					        DOT11_A3_HDR_LEN);

					PKTPULL(wlc->osh, p, body_offset_diff);
				}
			} else {
				struct ether_addr *sa, *da;
#ifdef BCMPCIEDEV
				if (BCMPCIEDEV_ENAB() && PKTISHDRCONVTD(wlc->osh, p)) {
					eh = (struct ether_header *)PKTPULL(wlc->osh, p,
						body_offset + 2);
					goto skip_conv;
				}
#endif /* BCMPCIEDEV */
				/* 802.11 -> 802.3/Ethernet header conversion */
				if (is_llc_snap_pkt) {
					/* RFC894 */
					eh = (struct ether_header *)PKTPULL(wlc->osh, p,
						body_offset + DOT11_LLC_SNAP_HDR_LEN -
							ETHER_HDR_LEN);
				} else {
					/* RFC1042 */
					uint body_len = PKTLEN(wlc->osh, p) - body_offset
						+(uint16)PKTFRAGUSEDLEN(wlc->osh, p);
					eh = (struct ether_header *)PKTPULL(wlc->osh, p,
						body_offset - ETHER_HDR_LEN);
					eh->ether_type = HTON16((uint16)body_len);
				}

				if ((h->fc & htol16(FC_TODS)) == 0) {
					da = &h->a1;
					if ((h->fc & htol16(FC_FROMDS)) == 0)
						sa = &h->a2;
					else
						sa = &h->a3;
				} else {
					da = &h->a3;
					if ((h->fc & htol16(FC_FROMDS)) == 0)
						sa = &h->a2;
					else
						sa = &h->a4;
				}
				ether_copy(sa, eh->ether_shost);
				ether_rcopy(da, eh->ether_dhost);
#ifdef BCMPCIEDEV
skip_conv:
#endif /* BCMPCIEDEV */
#ifdef WLTDLS
				if (TDLS_ENAB(wlc->pub) && BSSCFG_STA(bsscfg) &&
					eh->ether_type == HTON16(ETHER_TYPE_89_0D)) {
					void *tmp_pkt = PKTDUP(wlc->osh, p);

					if (tmp_pkt) {
						struct wlc_frminfo f;
						f.p = tmp_pkt;
						f.sa = (struct ether_addr *)&eh->ether_shost;
						f.da = (struct ether_addr *)&eh->ether_dhost;
						f.wrxh = wrxh;
						memcpy(f.plcp, plcp, sizeof(f.plcp));

						wlc_tdls_rcv_action_frame(wlc->tdls, scb, &f,
							ETHER_HDR_LEN);

						goto toss;
					}
				}
#endif /* WLTDLS */

				if (chained_sendup) {
					chained_sendup =
						((eh->ether_type == HTON16(ETHER_TYPE_IP)) ||
						(eh->ether_type == HTON16(ETHER_TYPE_IPV6)));
				}

				if (wlc_pktc_wsec_process(wlc, bsscfg, scb, p, wrxh, h, eh,
				                          &tsf_l, body_offset) != BCME_OK) {
					toss_reason = WLC_RX_STS_TOSS_DECRYPT_FAIL;
					goto toss;
				}
			}


			PKTCADDLEN(head, PKTLEN(wlc->osh, p));
			PKTSETCHAINED(wlc->osh, p);
			count++;
			byte_count += pkt_len;
#ifdef WL_LEAKY_AP_STATS
			if (WL_LEAKYAPSTATS_ENAB(wlc->pub)) {
				wlc_leakyapstats_pkt_stats_upd(wlc, bsscfg, scb->seqctl[rfc->prio],
					wlc_recover_tsf32(wlc, wrxh), rspec,
					wrxh->rssi, rfc->prio, pkt_len);
			}
#endif /* WL_LEAKY_AP_STATS */
		}

		prev = p;
		p = next;
		continue;
	toss:
		wlc_log_unexpected_rx_frame_log_80211hdr(wlc, h, toss_reason);
#ifdef WL_RX_STALL
		wlc_rx_healthcheck_update_counters(wlc, WME_PRIO2AC(PKTPRIO(p)),
			scb, bsscfg, toss_reason, 1);
#endif
		temp = p;
		if (prev != NULL)
			PKTSETCLINK(prev, PKTCLINK(p));
		else
			head = PKTCLINK(p);
		p = PKTCLINK(p);
		PKTSETCLINK(temp, NULL);
		PKTCLRCHAINED(wlc->osh, temp);
#ifdef BCMSPLITRX
		if (need_pktfetch) {
			wlc_sendup_schedule_pktfetch(wlc, temp);
			need_pktfetch = FALSE;
		}
		else
#endif
		{
			WLCNTINCR(wlc->pub->_wme_cnt->rx_failed[rfc->ac].packets);
			WLCNTADD(wlc->pub->_wme_cnt->rx_failed[rfc->ac].bytes,
				pkttotlen(wlc->osh, temp));
			PKTFREE(wlc->osh, temp, FALSE);
		}

		/* empty chain, return */
		if (head == NULL) {
			/* Clear DA for pktc */
			PKTC_HDA_SET(wlc->pktc_info, NULL);
			return;
		}
	}

	/* Clear DA and SCB for pktc */
	PKTC_HDA_SET(wlc->pktc_info, NULL);
	PKTC_HSCB_SET(wlc->pktc_info, NULL);

#ifdef PKTC_INTRABSS
	if (BSSCFG_AP(bsscfg) && !bsscfg->ap_isolate) {
		struct scb *dst;
		eh = (struct ether_header *)PKTDATA(wlc->osh, head);
		if (eacmp(eh->ether_dhost, wlc->pub->cur_etheraddr.octet) &&
			(dst = wlc_scbfind(wlc, bsscfg, (struct ether_addr *)eh->ether_dhost))) {
			if (SCB_ASSOCIATED(dst)) {
				intrabss_fwd = TRUE;
				chained_sendup = FALSE;
			}
		}
	}
#endif /* PKTC_INTRABSS */

#if defined(STA) && defined(PKTC_DONGLE)
	if (count != 0) {
		prev_cnt = wlc->pub->_cnt->rxbyte;
	}
#endif /* defined(STA) && defined(PKTC_DONGLE) */

	if (chained_sendup) {
		WLCNTSET(wlc->pub->_cnt->currchainsz, count);
		WLCNTSET(wlc->pub->_cnt->maxchainsz,
		         MAX(count, wlc->pub->_cnt->maxchainsz));

		/* update stat counters */
		WLCNTADD(wlc->pub->_cnt->rxframe, count);
		WLCNTADD(wlc->pub->_cnt->rxbyte, byte_count);
		WLCNTADD(wlc->pub->_wme_cnt->rx[rfc->ac].packets, count);
		WLCNTADD(wlc->pub->_wme_cnt->rx[rfc->ac].bytes, byte_count);
		WLPWRSAVERXFADD(wlc, count);

		/* update interface stat counters */
		WLCNTADD(bsscfg->wlcif->_cnt->rxframe, count);
		WLCNTADD(bsscfg->wlcif->_cnt->rxbyte, byte_count);

		scb->used = wlc->pub->now;

#ifdef WLWNM_AP
		/* update rx time stamp for BSS Max Idle Period */
		if (BSSCFG_AP(bsscfg) && WLWNM_ENAB(wlc->pub) && scb && SCB_ASSOCIATED(scb))
			wlc_wnm_rx_tstamp_update(wlc, scb);
#endif

#ifdef PSTA
		/* restore the original src addr before sending up */
		if (BSSCFG_PSTA(bsscfg)) {
			eh = (struct ether_header *)PKTDATA(wlc->osh, head);
			wlc_psta_recv_proc(wlc->psta, head, eh, &bsscfg);
			wlc_bsscfg_primary(wlc)->roam->time_since_bcn = 0;
		}
#endif

		PKTCSETCNT(head, count);
		WLCNTADD(wlc->pub->_cnt->chained, count);

		wlc_sendup(wlc, bsscfg, scb, head);
	} else {
		void *n;
		FOREACH_CHAINED_PKT(head, n) {
			PKTCLRCHAINED(wlc->osh, head);
			PKTCCLRFLAGS(head);
			WLCNTINCR(wlc->pub->_cnt->chainedsz1);

#ifdef PKTC_INTRABSS
			if (intrabss_fwd) {
				if (WME_ENAB(wlc->pub)) {
					wl_traffic_stats_t *forward =
					&wlc->pub->_wme_cnt->forward[WME_PRIO2AC(PKTPRIO(head))];
					WLCNTINCR(forward->packets);
					WLCNTADD(forward->bytes, pkttotlen(wlc->osh, head));
				}
				wlc_sendpkt(wlc, head, bsscfg->wlcif);
			} else
#endif /* PKTC_INTRABSS */
			{
				wlc_recvdata_sendup(wlc, WLPKTTAGSCBGET(head), FALSE,
					(struct ether_addr *)PKTDATA(wlc->osh, head), head);
			}
		}
	}
#if defined(STA) && defined(PKTC_DONGLE)
	if (count != 0) {
		wlc_pm2_ret_upd_last_wake_time(bsscfg, &tsf_l);
		wlc_update_sleep_ret(bsscfg, TRUE, FALSE, wlc->pub->_cnt->rxbyte - prev_cnt, 0);
	}
#endif /* defined(STA) && defined(PKTC_DONGLE) */
}
#endif /* PKTC || PKTC_DONGLE */

#ifdef WL_RX_STALL

/** Utility macro to get rx_hc counters BSSCFG cubby */
#define RX_HC_BSSCFG_CUBBY(rx_hc, cfg) \
	((wlc_rx_hc_counters_t *)BSSCFG_CUBBY(cfg, (rx_hc)->cfg_handle))

/** Utility macro to get rx_hc counters SCB cubby */
#define RX_HC_SCB_CUBBY(rx_hc, scb) \
	((wlc_rx_hc_counters_t *)SCB_CUBBY(scb, (rx_hc)->scb_handle))

/** scb cubby init function */
static int
wlc_rx_hc_scb_init(void *context, struct scb * scb)
{
	wlc_rx_hc_t * rx_hc;
	wlc_rx_hc_counters_t * counters;
	wlc_bsscfg_t * cfg;

	BCM_REFERENCE(rx_hc);
	BCM_REFERENCE(cfg);

	if ((context == NULL) || (scb == NULL)) {
		return BCME_ERROR;
	}

	rx_hc = (wlc_rx_hc_t *)context;
	counters = RX_HC_SCB_CUBBY(rx_hc, scb);

	counters->ts[AC_BE] =
	counters->ts[AC_BK] =
	counters->ts[AC_VI] =
	counters->ts[AC_VO] = rx_hc->wlc->pub->now;

	cfg = scb->bsscfg;
	WL_ASSOC(("wlc_rx_hc_scb_init: wl%d.%d:%02x:%02x:%02x:%02x:%02x:%02x\n",
		WLCWLUNIT(rx_hc->wlc), WLC_BSSCFG_IDX(cfg),
		scb->ea.octet[0], scb->ea.octet[1],	scb->ea.octet[2],
		scb->ea.octet[3], scb->ea.octet[4],	scb->ea.octet[5]));

	return BCME_OK;
}

/** scb cubby deinit function */
static void
wlc_rx_hc_scb_deinit(void *context, struct scb * scb)
{
	wlc_rx_hc_t * rx_hc;
	wlc_bsscfg_t * cfg;

	BCM_REFERENCE(rx_hc);
	BCM_REFERENCE(cfg);

	if ((context == NULL) || (scb == NULL)) {
		return;
	}

	rx_hc = (wlc_rx_hc_t *)context;
	cfg = scb->bsscfg;

	WL_ASSOC(("wlc_rx_hc_scb_deinit: wl%d.%d:%02x:%02x:%02x:%02x:%02x:%02x\n",
		WLCWLUNIT(rx_hc->wlc), WLC_BSSCFG_IDX(cfg),
		scb->ea.octet[0], scb->ea.octet[1],	scb->ea.octet[2],
		scb->ea.octet[3], scb->ea.octet[4],	scb->ea.octet[5]));

	return;
}

/** bsscfg cubby init function */
static int
wlc_rx_hc_cfg_init(void *context, wlc_bsscfg_t * cfg)
{
	wlc_rx_hc_t * rx_hc;
	wlc_rx_hc_counters_t * counters;
	BCM_REFERENCE(rx_hc);
	BCM_REFERENCE(cfg);

	if ((context == NULL) || (cfg == NULL)) {
		return BCME_ERROR;
	}

	rx_hc = (wlc_rx_hc_t *)context;
	counters = RX_HC_BSSCFG_CUBBY(rx_hc, cfg);
	counters->ts[AC_BE] = counters->ts[AC_BK] =
	counters->ts[AC_VI] = counters->ts[AC_VO] = rx_hc->wlc->pub->now;

	WL_ASSOC(("wlc_rx_hc_cfg_init: wl%d.%d\n",
		WLCWLUNIT(rx_hc->wlc), WLC_BSSCFG_IDX(cfg)));

	return BCME_OK;
}

/** bsscfg cubby deinit function */
static void
wlc_rx_hc_cfg_deinit(void *context, wlc_bsscfg_t * cfg)
{
	wlc_rx_hc_t * rx_hc;
	BCM_REFERENCE(rx_hc);

	if ((context == NULL) || (cfg == NULL)) {
		return;
	}

	rx_hc = (wlc_rx_hc_t *)context;

	WL_ASSOC(("wlc_rx_hc_cfg_deinit: wl%d.%d\n",
		WLCWLUNIT(rx_hc->wlc), WLC_BSSCFG_IDX(cfg)));

	return;
}


/** Allocate, init and return the wlc_rx_hc_t */
wlc_rx_hc_t *
BCMATTACHFN(wlc_rx_hc_attach)(wlc_info_t * wlc)
{
	osl_t * osh = wlc->osh;
	wlc_rx_hc_t * rx_hc;

	/* alloc parent tx_stall structure */
	rx_hc = (wlc_rx_hc_t*)MALLOCZ(osh, sizeof(*rx_hc));

	if (rx_hc == NULL) {
		WL_ERROR(("%s: no mem for %zd bytes rx_hc info,"
			"malloced %d bytes\n", __FUNCTION__,
			sizeof(*rx_hc), MALLOCED(osh)));
		return NULL;
	}

	rx_hc->wlc = wlc;

	/* Initialize default parameters */
	rx_hc->rx_hc_alert_th = RX_HC_ALERT_THRESHOLD;
	rx_hc->rx_hc_cnt = RX_HC_FRAMECNT;
	rx_hc->counters.ts[AC_BE] = rx_hc->counters.ts[AC_BK] =
	rx_hc->counters.ts[AC_VI] = rx_hc->counters.ts[AC_VO] = rx_hc->wlc->pub->now;

	rx_hc->scb_handle = wlc_scb_cubby_reserve(wlc,
		sizeof(wlc_rx_hc_counters_t),
		wlc_rx_hc_scb_init, wlc_rx_hc_scb_deinit,
		NULL, (void *)rx_hc);

	if (rx_hc->scb_handle == -1) {
		goto fail;
	}

	rx_hc->cfg_handle = wlc_bsscfg_cubby_reserve(wlc,
		sizeof(wlc_rx_hc_counters_t),
		wlc_rx_hc_cfg_init, wlc_rx_hc_cfg_deinit,
		NULL, rx_hc);

	if (rx_hc->cfg_handle == -1) {
		goto fail;
	}

	return rx_hc;

fail:
	if (rx_hc) {
		MFREE(osh, rx_hc, sizeof(*rx_hc));
	}

	return NULL;
}

void
BCMATTACHFN(wlc_rx_hc_detach)(wlc_rx_hc_t * rx_hc)
{
	wlc_info_t * wlc;

	if ((rx_hc != NULL) && (rx_hc->wlc != NULL)) {
		wlc = rx_hc->wlc;
		MFREE(wlc->osh, rx_hc, sizeof(wlc_rx_hc_t));
		wlc->rx_hc = NULL;
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
int
wlc_rx_activity_dump(wlc_info_t *wlc, struct bcmstrbuf *b)
{
#if RX_HC_TIMES_HISTORY == 0
	return BCME_UNSUPPORTED;
#else
	int index, j;
	wlc_rx_hc_t *rx = NULL;

	rx = wlc->rx_hc;
	index = rx->curr_index;

	if (b)
		bcm_bprintf(b, "RX Healthcheck history Now:%d:\n", wlc->pub->now);
	else
		WL_ERROR(("RX Healthcheck history Now:%d:\n", wlc->pub->now));

	for (j = index; j < index + RX_HC_TIMES_HISTORY; j++) {
		if (rx->entry[j%RX_HC_TIMES_HISTORY].dropped > 0) {
			if (b) {
				bcm_bprintf(b, "History #%02d: %s rxpkts:%d, dropped:%d"
				", reason:%s AC:%d timestamp: start:%d end:%d\n",
				j%RX_HC_TIMES_HISTORY, rx->entry[j%RX_HC_TIMES_HISTORY].prefix,
				rx->entry[j%RX_HC_TIMES_HISTORY].sum,
				rx->entry[j%RX_HC_TIMES_HISTORY].dropped,
				rx_sts_string[rx->entry[j%RX_HC_TIMES_HISTORY].reason],
				rx->entry[j%RX_HC_TIMES_HISTORY].ac,
				rx->entry[j%RX_HC_TIMES_HISTORY].start_ts,
				rx->entry[j%RX_HC_TIMES_HISTORY].end_ts);
			}
			else {
				WL_ERROR(("History #%02d: %s rxpkts:%d, dropped:%d"
				", reason:%s AC:%d timestamp: start:%d end:%d\n",
				j%RX_HC_TIMES_HISTORY, rx->entry[j%RX_HC_TIMES_HISTORY].prefix,
				rx->entry[j%RX_HC_TIMES_HISTORY].sum,
				rx->entry[j%RX_HC_TIMES_HISTORY].dropped,
				rx_sts_string[rx->entry[j%RX_HC_TIMES_HISTORY].reason],
				rx->entry[j%RX_HC_TIMES_HISTORY].ac,
				rx->entry[j%RX_HC_TIMES_HISTORY].start_ts,
				rx->entry[j%RX_HC_TIMES_HISTORY].end_ts));
			}
		}
	}
	if (b)
		bcm_bprintf(b, "The END.\n");
	else
		WL_ERROR(("The END.\n"));

	return BCME_OK;
#endif /* RX_HC_TIMES_HISTORY */
}
#endif /* BCMDBG || BCMDBG_DUMP */

/** Update RX counters */
int
wlc_rx_healthcheck_update_counters(wlc_info_t * wlc, int ac,
	scb_t * scb, wlc_bsscfg_t * bsscfg, int rx_status,
	int count)
{
	wlc_rx_hc_t * rx;
	wlc_rx_hc_counters_t * bsscfg_counters = NULL;
	wlc_rx_hc_counters_t * scb_counters = NULL;

	ASSERT(wlc);
	rx = wlc->rx_hc;
	ASSERT(rx);
	ASSERT(rx_status < WLC_RX_STS_LAST);
	BCM_REFERENCE(bsscfg_counters);
	BCM_REFERENCE(scb_counters);

	if (rx->rx_hc_alert_th == 0) {
		return BCME_OK;
	}

	if ((ac < AC_BE) || (ac > AC_VO)) {
		ac = AC_BE;
	}

	if (scb) {
		/* Do not update SCB's marked for deletion */
		if (SCB_MARKED_DEL(scb)) {
			scb = NULL;
		} else {
			scb_counters = RX_HC_SCB_CUBBY(rx, scb);
		}
	}

	/* If bsscfg is not provided, then get if from either scb or from pkt */
	if (bsscfg == NULL) {
		/* If valid SCB, then get the bsscfg from scb */
		if (scb) {
			bsscfg = SCB_BSSCFG(scb);
		}
	}

	if (bsscfg) {
		bsscfg_counters = RX_HC_BSSCFG_CUBBY(rx, bsscfg);
	}

	if (scb && bsscfg && ((rx_status == WLC_RX_STS_TOSS_DECRYPT_FAIL) ||
		(rx_status == WLC_RX_STS_TOSS_ICV_ERR))) {
		if ((bsscfg->wsec & (TKIP_ENABLED | AES_ENABLED)) && !SCB_AUTHORIZED(scb)) {
			WL_ASSOC(("RX HC:Key isn't plumbed, ignore decrypt fail frames\n"));
			return BCME_OK;
		}
	}

	WLCNTCONDADD(scb_counters, scb_counters->rx_pkts[ac][rx_status], (uint16)count);
	WLCNTCONDADD(bsscfg_counters, bsscfg_counters->rx_pkts[ac][rx_status], (uint16)count);
	WLCNTADD(rx->counters.rx_pkts[ac][rx_status], (uint16)count);

	if (rx_status && (rx_status != WLC_RX_STS_TOSS_ICV_ERR)) {
		WLCNTCONDADD(scb_counters, scb_counters->dropped_all[ac], count);
		WLCNTCONDADD(bsscfg_counters, bsscfg_counters->dropped_all[ac], count);
		WLCNTADD(rx->counters.dropped_all[ac], count);
	}

	return BCME_OK;
}

const char *
wlc_rx_dropped_get_name(int reason)
{
	if ((reason > WLC_RX_STS_OK) && (reason < WLC_RX_STS_LAST)) {
		return rx_sts_string[reason];
	}

	return rx_sts_string[WLC_RX_STS_TOSS_UNKNOWN];
}

int
wlc_rx_healthcheck_verify(wlc_info_t *wlc,
	wlc_rx_hc_counters_t *counters, int ac, const char * prefix)
{
	wlc_rx_hc_t *rx = NULL;
	uint32 sum = 0;
	int fail_rate = 0;
	int ret = BCME_OK;
	int i = 0;
	BCM_REFERENCE(fail_rate);

	if (!wlc)
		return BCME_OK;

	rx = wlc->rx_hc;
	if (!rx || !rx->rx_hc_alert_th)
		return BCME_OK;

	sum = counters->rx_pkts[ac][WLC_RX_STS_OK] + counters->dropped_all[ac];

	if (counters->dropped_all[ac]*100 > sum*rx->rx_hc_alert_th) {
		fail_rate = counters->dropped_all[ac]*100/sum;
	    counters->stall_cnt[ac]++;
		if (counters->stall_cnt[ac] < RX_HC_STALL_CNT) {
			ret = BCME_OK;
			goto out;
		}
		bzero(&rx->error, sizeof(wlc_rx_hc_err_info_t));
		for (i = WLC_RX_STS_OK + 1; i < 32; i++) {
			if (counters->rx_pkts[ac][i] > 0) {
				rx->error.stall_bitmap0 |= 1 << i;
			}
		}
		for (i = 0; i < WLC_RX_STS_LAST-32; i++) {
			if (counters->rx_pkts[ac][i+32] > 0) {
				rx->error.stall_bitmap1 |= 1 << i;
			}
		}
		rx->error.sum = sum;
		rx->error.dropped = counters->dropped_all[ac];
		rx->error.fail_rate = fail_rate;
		rx->error.ac = ac;
		rx->error.threshold = rx->rx_hc_alert_th;
		rx->error.sample_size = rx->rx_hc_cnt;
		strncpy(rx->error.prefix, prefix, ETHER_ADDR_STR_LEN);
		rx->error.prefix[sizeof(rx->error.prefix) - 1] = '\0';
		WL_ERROR(("wlc_rx_healthcheck_verify ALERT: rxpkts: %d, dropped: %d,"
			" AC:%d, fail_rate:%d%%, timestamp: start:%d end:%d,"
			" err_bitmap0:0x%x, err_bitmap1:0x%x\n",
			sum, counters->dropped_all[ac], ac, fail_rate, counters->ts[ac],
			wlc->pub->now, rx->error.stall_bitmap0, rx->error.stall_bitmap1));
		ret = BCME_ERROR;
	}
	counters->stall_cnt[ac] = 0;
out:
	bzero(&counters->rx_pkts[ac][0], WLC_RX_STS_LAST*sizeof(uint32));
	counters->dropped_all[ac] = 0;
	counters->ts[ac] = wlc->pub->now;
	return ret;
}

void
wlc_rx_healthcheck_report(wlc_info_t *wlc)
{

#if defined(WL_DATAPATH_LOG_DUMP)
	/* dump some datapath info */
	wlc_datapath_log_dump(wlc, EVENT_LOG_TAG_WL_ERROR);
#endif
#if !defined(WL_DATAPATH_HC_NOASSERT) && !defined(HEALTH_CHECK)
	wlc->hw->need_reinit = WL_REINIT_RC_RX_HC_FAIL;
	WLC_FATAL_ERROR(wlc);
#endif

}

int
wlc_rx_healthcheck(uint8* buffer_ptr, uint16 remaining_len,
	void* context, int16* bytes_written)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
	uint32 i, idx, sum = 0, fail_detected = 0;
	wlc_bsscfg_t *bsscfg;
	struct scb_iter scbiter;
	scb_t * scb;
	wlc_rx_hc_t *rx = NULL;
	wlc_rx_hc_counters_t * counters;
#if defined(HEALTH_CHECK)
	wl_rx_hc_info_t rx_hc_info;
#endif
#if RX_HC_TIMES_HISTORY > 0
	uint32 index = 0, j;
#endif
	int rc = BCME_OK;

	BCM_REFERENCE(remaining_len);
	BCM_REFERENCE(buffer_ptr);
	BCM_REFERENCE(bytes_written);

	if (!wlc)
		return BCME_ERROR;
	rx = wlc->rx_hc;
	counters = &rx->counters;

	for (i = AC_BE; i < AC_COUNT; i++) {
		sum = counters->rx_pkts[i][WLC_RX_STS_OK] + counters->dropped_all[i];
		if (sum >= rx->rx_hc_cnt) {
#if RX_HC_TIMES_HISTORY > 0
			/* Record Global RX failure history per reason  */
			for (j = WLC_RX_STS_OK + 1; j <  WLC_RX_STS_LAST; j++) {
				if (counters->rx_pkts[i][j] > 0) {
					index = rx->curr_index;
					if (index >= RX_HC_TIMES_HISTORY)	index = 0;
					rx->entry[index].dropped = counters->rx_pkts[i][j];
					rx->entry[index].sum = sum;
					rx->entry[index].start_ts = counters->ts[i];
					rx->entry[index].end_ts = wlc->pub->now;
					rx->entry[index].reason = j;
					rx->entry[index].ac = i;
					strncpy(rx->entry[index].prefix,
						"Global", ETHER_ADDR_STR_LEN);
					rx->curr_index = index + 1;
				}
			}
#endif /* RX_HC_TIMES_HISTORY */
			fail_detected +=
				wlc_rx_healthcheck_verify(wlc, counters, i, "Global");
		}
	}

	/* Iterate all active BSS */
	FOREACH_BSS(wlc, idx, bsscfg) {
		char ifname[32] = "UNKNOWN BSS";

		if (!bsscfg->up) {
			continue;
		}

		if ((bsscfg->wlcif != NULL)) {
			strncpy(ifname, wl_ifname(wlc->wl, bsscfg->wlcif->wlif),
					sizeof(ifname));
			ifname[sizeof(ifname) - 1] = '\0';
		}

		/* Verify total failure rate of this BSSCFG */
		/* rx hc counters cubby */
		counters = RX_HC_BSSCFG_CUBBY(rx, bsscfg);

		for (i = AC_BE; i < AC_COUNT; i ++) {
			sum = counters->rx_pkts[i][WLC_RX_STS_OK] + counters->dropped_all[i];
			if (sum >= rx->rx_hc_cnt) {
				fail_detected +=
					wlc_rx_healthcheck_verify(wlc, counters, i, ifname);
			}
		}

		/* Verify RX Failure rate for each SCB */
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
			char eabuf[ETHER_ADDR_STR_LEN];
			counters = RX_HC_SCB_CUBBY(rx, scb);
			bcm_ether_ntoa(&scb->ea, eabuf);
			for (i = AC_BE; i < AC_COUNT; i ++) {
				sum = counters->rx_pkts[i][WLC_RX_STS_OK]
					+ counters->dropped_all[i];
				if (sum >= rx->rx_hc_cnt) {
					fail_detected +=
						wlc_rx_healthcheck_verify(wlc, counters, i, eabuf);
				}
			}
		}
	}

	if (fail_detected) {
#if defined(HEALTH_CHECK)
		/* Only report the first interface on which problem is seen */
			rx_hc_info.bsscfg_idx = bsscfg->_idx;
			rx_hc_info.rx_hc_pkts = rx->rx_hc_pkts;
			rx_hc_info.rx_hc_dropped_all = rx->rx_hc_dropped_all;
			rx_hc_info.rx_hc_alert_th = rx->rx_hc_alert_th;
#endif
		wlc_rx_healthcheck_report(wlc);
#if RX_HC_TIMES_HISTORY > 0
		bzero(&rx->entry[0], sizeof(rx->entry[0])*RX_HC_TIMES_HISTORY);
		rx->curr_index = 0;
#endif

		/* clean up */
		rx->rx_hc_pkts = 0;
		rx->rx_hc_dropped_all = 0;
		bzero(&rx->rx_hc_dropped[0], sizeof(uint32) * RX_HC_DROPPED_LAST);
		rx->rx_hc_ts = wlc->pub->now;
	} else {
#if defined(HEALTH_CHECK)
		*bytes_written = 0;
		return HEALTH_CHECK_STATUS_OK;
#endif /* HEALTH_CHECK */
	}

#if defined(HEALTH_CHECK)
	/* If an error is detected and space is available, client must write
	 * a XTLV record to indicate what happened.
	 * The buffer provided is word aligned.
	 */
	if (remaining_len >= sizeof(rx_hc_info)) {
		rx_hc_info.type = WL_HC_DD_RX_STALL;
		rx_hc_info.length = sizeof(rx_hc_info) - BCM_XTLV_HDR_SIZE;

		bcopy(&rx_hc_info, buffer_ptr, sizeof(rx_hc_info));
		*bytes_written = sizeof(rx_hc_info);
	} else {
		/* hc buffer too short */
		*bytes_written = 0;
	}

	/* overwrite the rc to return a proper status back to framework */
#if defined(WL_DATAPATH_HC_NOASSERT)
	rc = (HEALTH_CHECK_STATUS_INFO_LOG_BUF | HEALTH_CHECK_STATUS_ERROR);
#else
	rc = (HEALTH_CHECK_STATUS_INFO_LOG_BUF | HEALTH_CHECK_STATUS_TRAP);
#endif /* WL_DATAPATH_HC_NOASSERT */
#endif /* HEALTH_CHECK */

	return rc;
}

void
wlc_rx_healthcheck_force_fail(wlc_info_t *wlc)
{
	wlc_bsscfg_t *bsscfg;
	wlc_rx_hc_t *rx = NULL;
	wlc_rx_hc_counters_t * counters;
	uint32 i = 0;
#if RX_HC_TIMES_HISTORY > 0
	uint32 j = 0, index = 0;
#endif
	if (!wlc) return;

	/* Force BSSCFG 0 PRIO 0 RX healthcheck failure */
	bsscfg = wlc_bsscfg_primary(wlc);
	if (bsscfg) {
		WL_ERROR(("wlc_rx_healthcheck_force_fail if=wl%d.%d\n",
			WLCWLUNIT(wlc), WLC_BSSCFG_IDX(bsscfg)));
		rx = wlc->rx_hc;
		counters = RX_HC_BSSCFG_CUBBY(rx, bsscfg);
		/* Fake counters */
		counters->dropped_all[0] = rx->rx_hc_cnt;
		for (i = WLC_RX_STS_OK + 1; i < WLC_RX_STS_LAST; i++) {
			counters->rx_pkts[0][i] = i;
			counters->dropped_all[0] += i;
		}
		counters->rx_pkts[0][WLC_RX_STS_TOSS_ICV_ERR] += rx->rx_hc_cnt;
		counters->dropped_all[0] += rx->rx_hc_cnt;
		counters->stall_cnt[0] = RX_HC_STALL_CNT;
#if RX_HC_TIMES_HISTORY > 0
		/* Fake history */
		index = rx->curr_index;
		for (j = index; j < index + RX_HC_TIMES_HISTORY; j++) {
			if (rx->entry[j%RX_HC_TIMES_HISTORY].dropped <= 0) {
				rx->entry[j%RX_HC_TIMES_HISTORY].dropped = j%WLC_RX_STS_LAST + 1;
				rx->entry[j%RX_HC_TIMES_HISTORY].sum = rx->rx_hc_cnt;
				rx->entry[j%RX_HC_TIMES_HISTORY].start_ts = wlc->pub->now;
				rx->entry[j%RX_HC_TIMES_HISTORY].end_ts = wlc->pub->now + 1;
				rx->entry[j%RX_HC_TIMES_HISTORY].reason = j%WLC_RX_STS_LAST;
				rx->entry[j%RX_HC_TIMES_HISTORY].ac = j%AC_COUNT;
			}
		}
#endif /* RX_HC_TIMES_HISTORY */

#if defined(HEALTH_CHECK)
		/* Force trigger WL health check execution */
		wl_health_check_execute((void*)wlc->wl);
#endif

		rx->rx_hc_pkts = 0;
		rx->rx_hc_dropped_all = 0;
		bzero(&rx->rx_hc_dropped[0], sizeof(uint32) * RX_HC_DROPPED_LAST);
		rx->rx_hc_ts = wlc->pub->now;
	}
}
#endif /* WL_RX_STALL */
