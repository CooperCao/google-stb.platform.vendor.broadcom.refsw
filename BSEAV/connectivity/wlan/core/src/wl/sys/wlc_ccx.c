#ifdef BCMCCX

/**
 * @file
 * @brief
 * Common CCX functions
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
#include <bcmendian.h>
#include <wlioctl.h>

#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <proto/802.11_ccx.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_keymgmt.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_phy_hal.h>
#include <phy_tpc_api.h>
#include <wlc_scb.h>
#include <bcmwpa.h>

#include <bcmcrypto/passhash.h>
#include <bcmcrypto/prf.h>
#include <bcmcrypto/sha1.h>

#include <wlc_wpa.h>
#include <wlc_sup.h>
#include <bcmcrypto/ccx.h>
#include <bcmcrypto/bcmccx.h>
#include <wlc_rm.h>
#include <wlc_ccx.h>
#include <wlc_cac.h>
#include <wl_export.h>
#include <wlc_frmutil.h>

#include <wlc_scan.h>
#include <wlc_assoc.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_helper.h>

#if defined(BCMCCX) && defined(BCMINTSUP)
#include <wlc_sup_ccx.h>
#endif

#include <wlc_utils.h>
#include <wlc_pcb.h>
#include <wlc_lq.h>
#include <wlc_hw.h>

#include <phy_tpc_api.h>

/* CCX radio measurement report */
typedef struct wlc_ccx_rm_rept {
	struct wlc_ccx_rm_rept *next;
	uint	len;
	uchar	data[CCX_RM_REP_DATA_MAX_LEN];
} wlc_ccx_rm_rept_t;

/* CCX directed roaming AP paramenters */
typedef struct wlc_ccx_droam {
	struct ether_addr bssid;	/* AP bssid */
	bool	rf_ie;			/* set if RF subelement presents */
	int8	min_rssi;		/* min. recv pwr in dBm required to associate with the AP */
	int8	ap_tx_pwr;		/* AP's tx power */
	uint8	roam_time;		/* transition time(in 0.1s) permmited in roam. not use */
	int8	roam_delta;		/* roam delta */
	int8	roam_trigger;	/* roam trigger */
} wlc_ccx_droam_t;

#define CCX_ROAM_AP_ELEMENTS	16	/* max roam AP elements */

/* CCXv5 S71 Interpretation of Status and Result Codes */
typedef struct wlc_ccx_status_row {
	uint8	status_row;		/* status row number from 1 to 5 defined in spec. */
	struct ether_addr	bssid;	/* bssid where status/reason code comes from */
	uint	wait_time;	/* delay time required in second */
	struct wlc_ccx_status_row *next;
} wlc_ccx_status_row_t;

#define CCX_MIN_RECON_WAIT	5	/* minumum wait time(in second) before reconnect
					 * to AP which returned error status in status row 2 to 5
					 */


/* Use for setting auth to zero */
#define AUTH_MODE_MAGIC		0x10000

typedef struct ccx_bsscfg_priv {
	wlc_info_t* wlc;	/* back pointer to wlc */
	wlc_bsscfg_t *cfg; /* back pointer to cfg */
	uint		disassoc_time;		/* CCX roam report: time since assoc */
	uint8		prev_channel;		/* CCX roam report: previous channel */
	struct ether_addr	prev_ap_mac;	/* CCX roam report: previous BSSID */

	/* link test */
	bool		linktest_pending;	/* link test report packet pending */
	uint		linktest_txretry;	/* link test report packet retry times */
	uint16		linktest_frm_num;	/* link test frame number */

	/* directed roaming */
	wlc_ccx_droam_t droam_ap_list[CCX_ROAM_AP_ELEMENTS];	/* directed roam neighbor AP list */
	uint		droam_ap_num;		/* directed roam elements */
	bool		droam_rf_roam;		/* set if RF subelement roam parameters applied */

	/* TPC - transmit power control */
	int8		cur_qtxpwr;		/* tx power in qdbm before apply TPC */
	int8		tpc_qtxpwr;		/* TPC in qdbm */
	bool		tpc;			/* TPC flag */

	/* support for machine name in Aironet IE */
	char		staname[AIRONET_IE_MAX_NAME_LEN];
	char		apname[AIRONET_IE_MAX_NAME_LEN];
	uint32		rn;				/* seq number for external supplicant */
	uint8 key_refresh_key[CCKM_KRK_LEN]; /* krk for external supplicant */
	uint32		wlan_assoc_reason;	/* iovar interface for association reason config */
	uint		orig_reason;	/* original roam reason code(before fast roaming fails) */
	chanspec_t	ap_chanspec_list[CCX_ROAM_SCAN_CHANNELS];	/* List of channels provided
						 * by AP for fast roaming
						 */
	uint		ap_channel_num;		/* Number of valid channels in list */
	uint		 leap_start; /* time instance of leap LEAP starting point */
	bool		 leap_on;	 /* flag for CCX leap */
	/* roaming stats - S56.4.3, S56.5.2.6 */
	uint32	roaming_start_time;	/* roam start time. low 32 bits of tsf */
	uint32	roaming_delay_tu;	/* time in TU from roam_start_time till
						* association completion for the last
						* successful association.
						*/
	uint32		roaming_count;	/* number of successful roams. */
	bool		fast_roam;	/* flag signalling use of AP channel list */
	bool		ccx_network;	/* is ccx network */
} ccx_bsscfg_priv_t;

typedef struct wlc_ccx_info {
	struct wlc_ccx  pubci;	/* ccx public state(must be first field of ccx_info) */
	wlc_info_t	*wlc;
	wlc_pub_t	*pub;
	wlc_rm_req_state_t	*rm_state;
	wlc_ccx_rm_rept_t	*rm_reports;	/* linked list of ccx reports */
	uint		rm_limit;		/* do not perform non-home channel
						 * measurements with a duration long than
						 * this many TUs
						 */
	uint		auth_mode;		/* auth mode to be set in msg */
	wlc_bsscfg_t* rm_reqcfg; /* cfg requesting the radio measurements */

} wlc_ccx_info_t;

#define CCX_BSSCFG_CUBBY_LOC(ccx, cfg) ((ccx_bsscfg_priv_t **) \
	BSSCFG_CUBBY(cfg, ((ccx_t *)(ccx))->cfgh))
#define CCX_BSSCFG_CUBBY(ccx, cfg) (*CCX_BSSCFG_CUBBY_LOC(ccx, cfg))

#define	RMFRMHASHINDEX(id)		(((id)[3] ^ (id)[4] ^ (id)[5]) & (WLC_NRMFRAMEHASH - 1))

/* Extend max limitation for CCX S36: 8.7.2 */
#define WLC_CCX_DEF_RM_TM_LIMIT		100 /* default off-channel measurement limit in ms */

/* null packet sending interval in seconds for keep-alive */
#define	KEEP_ALIVE_INTERVAL		10	/* default 10 seconds as Cisco suggested */


#define WLC_CCX_IS_CURRENT_BSSID(cfg, bssid) \
	(!bcmp((char*)(bssid), (char*)&((cfg)->BSSID), ETHER_ADDR_LEN))

/* local functions */
static int wlc_ccx_doiovar(void *hdl, uint32 actionid,
	void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif);
static int wlc_ccx_down(void *hdl);
static void wlc_ccx_watchdog(void *hdl);
static void wlc_ccxv2_process_roam_iapp(wlc_ccx_info_t *ci, ccx_roam_ap_ie_t *ap_ie,
	uint len, wlc_bsscfg_t* cfg);
static void wlc_ccx_process_roam_iapp(wlc_ccx_info_t *ci, ccx_neighbor_rept_ie_t *rept_ie,
	uint len, bool append, wlc_bsscfg_t* cfg);
static void wlc_ccx_process_droam_iapp(wlc_ccx_info_t *ci, ccx_neighbor_rept_ie_t *rept_ie,
	uint len, wlc_bsscfg_t* cfg);

static bool wlc_ccx_rf_ie_good(wlc_ccx_info_t *ci, wlc_ccx_droam_t *droam_ap, wlc_bss_info_t *bi);
static void wlc_ccx_process_droam_rf_ie(wlc_ccx_info_t *ci, ccx_radio_param_subie_t *rf_ie,
	int idx, bool curr_ap, wlc_bsscfg_t* cfg);
static int16 wlc_ccx_get_min_rssi(wlc_ccx_info_t *ci, int8 ap_txpwr, int8 ap_rssi,
	uint8 ap_channel);
static void wlc_ccx_send_roam_rpt(ccx_t *ci, wlc_bsscfg_t* cfg);
static uint8 wlc_ccx_get_roam_reason(ccx_t *ci, wlc_bsscfg_t* cfg);
static void wlc_ccx_send_linktest_rpt(ccx_t *ci, struct ether_addr* da,
	struct ether_addr* sa, ccx_link_test_t* req_pkt, int req_len, int rssi, wlc_bsscfg_t* cfg);


/* local radio measurement functions */
static void wlc_ccx_rm_recv(wlc_ccx_info_t *ci, struct ether_addr* da, struct ether_addr* sa,
	uint16 rx_time, ccx_rm_req_t* req_pkt, int req_len, wlc_bsscfg_t* cfg);
static void wlc_ccx_rm_parse_requests(wlc_ccx_info_t *ci, ccx_rm_req_ie_t* ie,
	int len, wlc_rm_req_t* req, int count, wlc_bsscfg_t *cfg);
static int wlc_ccx_rm_state_init(wlc_ccx_info_t *ci, bool bcast, uint16 rx_time,
	ccx_rm_req_t* req_pkt, int req_len, wlc_bsscfg_t* cfg);
static void wlc_ccx_rm_scan_begin(wlc_ccx_info_t *ci, int type, uint32 dur);
static void wlc_ccx_rm_frm_begin(wlc_ccx_info_t *ci, uint32 dur);
static int wlc_ccx_rm_ie_count(ccx_rm_req_ie_t* ie, int len);
static ccx_rm_req_ie_t* wlc_ccx_rm_next_ie(ccx_rm_req_ie_t* ie, int* len);
static wlc_ccx_rm_rept_t* wlc_ccx_rm_new_report(wlc_ccx_info_t *ci, uchar** buf, int* avail_len);
static void wlc_ccx_rm_append_report_header(uint8 type, uint16 token, uint8 flags, int data_len,
	uchar* buf);
static int wlc_ccx_rm_append_load_report(wlc_ccx_info_t *ci, wlc_rm_req_t *req, uchar** bufptr,
	int *buflen);
static int wlc_ccx_rm_append_noise_report(wlc_ccx_info_t *ci, wlc_rm_req_t *req, uchar** bufptr,
	int *buflen);
static int wlc_ccx_rm_append_frame_report(wlc_ccx_info_t *ci, wlc_rm_req_t *req, uchar **bufptr,
	int *buflen);
static int wlc_ccx_rm_beacon_table(wlc_bss_info_t **results, uint num, wlc_rm_req_t *req,
	uchar **bufptr, int *buflen);
static int wlc_ccx_rm_beacon_table_entry(wlc_bss_info_t *bi, uint8 channel, uint16 dur, uchar *buf,
	int buflen);
static void wlc_ccx_rm_frm_list_free(wlc_ccx_info_t *ci);
static void wlc_ccx_rm_send_measure_report(wlc_ccx_info_t *ci, struct ether_addr *da,
	uint16 token, uint8 *report, uint report_len);
#ifdef BCMDBG
static void wlc_ccx_rm_req_dump(wlc_ccx_info_t *ci, struct ether_addr* da, struct ether_addr* sa,
	ccx_rm_req_t* req, int req_len, wlc_bsscfg_t* cfg);
static void wlc_ccx_rm_req_ie_dump(wlc_ccx_info_t *ci, ccx_rm_req_ie_t *ie, int buflen,
	wlc_bsscfg_t* cfg);
#endif /* BCMDBG */

/* pathloss measurement related routines */
static void wlc_ccx_rm_pathloss_begin(wlc_ccx_info_t *ci, wlc_rm_req_t* req);
static void wlc_ccx_rm_pathloss(wlc_ccx_info_t *ci);
static void wlc_ccx_rm_send_pathlossmeasure_frame(wlc_ccx_info_t *ci);
static void wlc_ccx_rm_pathloss_pktcallback(wlc_info_t *wlc, uint txstatus, void *arg);
static void wlc_ccx_pathloss_burst_timer(void *arg);
static void wlc_ccx_pathloss_dur_timer(void *arg);

/* IE mgmt */
static uint wlc_ccx_calc_ihv_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ccx_write_ihv_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_ccx_calc_ver_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ccx_write_ver_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_ccx_calc_rm_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ccx_write_rm_cap_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_ccx_calc_cckm_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ccx_write_cckm_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_ccx_calc_aironet_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ccx_write_aironet_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_ccx_parse_aironet_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_ccx_scan_parse_ver_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_ccx_scan_parse_aironet_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_ccx_scan_parse_qbss_load_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_ccx_bcn_parse_aironet_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_ccx_bcn_parse_qos_ie(void *ctx, wlc_iem_parse_data_t *data);

/* Initiate ccx private context */
static int wlc_ccx_init(void *ctx, wlc_bsscfg_t *cfg);

/* Remove ccx private context */
static void wlc_ccx_deinit(void *ctx, wlc_bsscfg_t *cfg);

static uint16 wlc_ccx_cubby_config_offset = OFFSETOF(ccx_bsscfg_priv_t, disassoc_time);
#define CCX_CUBBY_CONFIG_DATA(cfgi)  ((uint8*)(cfgi) + wlc_ccx_cubby_config_offset)
#define CCX_CUBBY_CONFIG_SIZE		(sizeof(ccx_bsscfg_priv_t) - wlc_ccx_cubby_config_offset)

static int wlc_ccx_config_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len);
static int wlc_ccx_config_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len);


const uint8 ccx_rm_capability_ie[] = {
	DOT11_MNG_PROPR_ID, 6,
	0x00, 0x40, 0x96, 0x01,
	0x01, 0x00
};
const uint8 ccx_version_ie[] = {
	DOT11_MNG_PROPR_ID, 5,
	0x00, 0x40, 0x96, 0x03,
	0x04 /* version */
};

static const uint8 CCKM_info_element[] = {
	DOT11_MNG_CCKM_REASSOC_ID, 0x18, 0x00, 0x40, 0x96, 00,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* iovar table */
enum {
	IOV_CCX_RM,
	IOV_CCX_RM_LIMIT,
	IOV_CCX_AUTH_MODE,
	IOV_STA_NAME,
	IOV_AP_NAME,
	IOV_CCX_VERSION,
	IOV_WLAN_ASSOC_REASON,
	IOV_TESTS60,
	IOV_CCX_IHV,
	IOV_CCX_ENABLE,
	IOV_CCX_V4_ONLY,
	IOV_CCKM_KRK,
	IOV_CCKM_RN,
	IOV_CCX_DROAM_ACTIVATED

};

static const bcm_iovar_t ccx_iovars[] = {
	{"ccx_rm", IOV_CCX_RM, (IOVF_RSDB_SET), 0, IOVT_BOOL, 0},
	{"ccx_rm_limit", IOV_CCX_RM_LIMIT, (IOVF_WHL | IOVF_RSDB_SET), 0, IOVT_UINT32, 0},
	{"ccx_auth_mode", IOV_CCX_AUTH_MODE, (IOVF_WHL | IOVF_RSDB_SET), 0, IOVT_INT32, 0},
	{"staname", IOV_STA_NAME, (0), 0, IOVT_BUFFER, 0},
	{"apname", IOV_AP_NAME, (0), 0, IOVT_BUFFER, 0},
	{"ccx_version", IOV_CCX_VERSION, (0), 0, IOVT_UINT32, 0},
	{"wlan_assoc_reason", IOV_WLAN_ASSOC_REASON, (0), 0, IOVT_UINT32, 0},
#ifdef BCMDBG
	{"test_s60", IOV_TESTS60, (0), 0, IOVT_BUFFER, 0},
#endif
	{"ccx_enable", IOV_CCX_ENABLE, (IOVF_RSDB_SET), 0, IOVT_BOOL, 0},
	{"ccx_v4_only", IOV_CCX_V4_ONLY, (IOVF_RSDB_SET), 0, IOVT_BOOL, 0},
	{"cckm_krk", IOV_CCKM_KRK, (IOVF_SET_UP), 0, IOVT_BUFFER, 0 },
	{"cckm_rn", IOV_CCKM_RN, (IOVF_SET_UP), 0, IOVT_UINT32, 0 },
	{"ccx_droam_activated", IOV_CCX_DROAM_ACTIVATED, (0), 0, IOVT_BOOL, 0 },

	{NULL, 0, 0, 0, 0, 0}
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

ccx_t*
BCMATTACHFN(wlc_ccx_attach)(wlc_info_t *wlc)
{
	wlc_ccx_info_t *ci;
	wlc_rm_req_state_t *rm;
	wlc_pub_t *pub = wlc->pub;
	bsscfg_cubby_params_t cubby_params;
	uint16 arqfstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);
	uint16 rmcapfstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ) | FT2BMP(FC_PROBE_REQ);
	uint16 arsfstbmp = FT2BMP(FC_ASSOC_RESP) | FT2BMP(FC_REASSOC_RESP);
	uint16 scanfstbmp = FT2BMP(WLC_IEM_FC_SCAN_BCN) | FT2BMP(WLC_IEM_FC_SCAN_PRBRSP);

	WL_TRACE(("wl: %s\n", __FUNCTION__));

	if ((ci = (wlc_ccx_info_t *)MALLOCZ(pub->osh, sizeof(wlc_ccx_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			pub->unit, __FUNCTION__, MALLOCED(pub->osh)));
		return NULL;
	}

	ci->wlc = wlc;
	ci->pub = pub;
	ci->rm_state = wlc->rm_info->rm_state;
	rm = ci->rm_state;

	/* reserve cubby space in the bsscfg container for per-bsscfg private data */
	bzero(&cubby_params, sizeof(cubby_params));

	cubby_params.context = ci;
	cubby_params.fn_init = wlc_ccx_init;
	cubby_params.fn_deinit = wlc_ccx_deinit;
	cubby_params.fn_get = wlc_ccx_config_get;
	cubby_params.fn_set = wlc_ccx_config_set;
	cubby_params.config_size = CCX_CUBBY_CONFIG_SIZE;

	ci->pubci.cfgh = wlc_bsscfg_cubby_reserve_ext(wlc, sizeof(ccx_bsscfg_priv_t *),
	                                              &cubby_params);

	if (ci->pubci.cfgh < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		    wlc->pub->unit, __FUNCTION__));
		goto error;
	}

	/* allocate memory for CCX radio measurement variables */
	if ((rm->ccx = (ccx_rm_t *)MALLOCZ(wlc->osh, sizeof(ccx_rm_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto error;
	}

	if (!(rm->ccx->plm_burst_timer = wl_init_timer(wlc->wl,
		wlc_ccx_pathloss_burst_timer, wlc, "ccx_pathloss"))) {
		WL_ERROR(("wl%d: %s: wl_init_timer for pathloss burst failed\n",
			pub->unit, __FUNCTION__));
		goto error;
	}
	if (!(rm->ccx->plm_dur_timer = wl_init_timer(wlc->wl, wlc_ccx_pathloss_dur_timer, wlc,
	                                             "ccx_plm_dur"))) {
		WL_ERROR(("wl%d: %s: wl_init_timer for pathloss duartion failed\n",
			pub->unit, __FUNCTION__));
		goto error;
	}

	/* By default CCX is enabled. */
	pub->_ccx = TRUE;

#ifndef CCX_AP_KEEP_ALIVE_DISABLED
	wlc->pub->_ccx_aka = TRUE;
#else
	wlc->pub->_ccx_aka = FALSE;
#endif

	ci->pubci.ccx_v4_only = TRUE;

	/* default off-channel measurement limit */
	ci->rm_limit = WLC_CCX_DEF_RM_TM_LIMIT;

	/* register module */
	if (wlc_module_register(pub, ccx_iovars, "ccx", ci, wlc_ccx_doiovar,
	                        wlc_ccx_watchdog, NULL, wlc_ccx_down)) {
		WL_ERROR(("wl%d: ccx wlc_module_register() failed\n", wlc->pub->unit));
		goto error;
	}

	/* register IE mgmt callbacks */
	/* calc/build */
	/* assocreq/reassocreq */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_AIRONET_ID,
	      wlc_ccx_calc_aironet_ie_len, wlc_ccx_write_aironet_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, aironet in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	/* register CCKM IE into assocreq to initialize RN value */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_CCKM_REASSOC_ID,
	      wlc_ccx_calc_cckm_ie_len, wlc_ccx_write_cckm_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, cckm in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_CCX_IHV,
	      wlc_ccx_calc_ihv_ie_len, wlc_ccx_write_ihv_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vs_add_build_fn failed, ihv in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_CCX_VER,
	      wlc_ccx_calc_ver_ie_len, wlc_ccx_write_ver_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vs_add_build_fn failed, ver in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	/* assocreq/reassocreq/prbreq */
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, rmcapfstbmp, WLC_IEM_VS_IE_PRIO_CCX_RM_CAP,
	      wlc_ccx_calc_rm_cap_ie_len, wlc_ccx_write_rm_cap_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vs_add_build_fn failed, rm cap ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}

	/* parse */
	/* assocresp/reassocresp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, arsfstbmp, DOT11_MNG_AIRONET_ID,
	                             wlc_ccx_parse_aironet_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, aironet in assocresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	/* bcn/prbrsp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, scanfstbmp, DOT11_MNG_AIRONET_ID,
	                             wlc_ccx_scan_parse_aironet_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, aironet in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, scanfstbmp, DOT11_MNG_QBSS_LOAD_ID,
	                             wlc_ccx_scan_parse_qbss_load_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, qbss load in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	if (wlc_iem_vs_add_parse_fn_mft(wlc->iemi, scanfstbmp, WLC_IEM_VS_IE_PRIO_CCX_VER,
	                                wlc_ccx_scan_parse_ver_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vs_add_parse_fn failed, ver in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	/* bcn */
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_AIRONET_ID,
	                         wlc_ccx_bcn_parse_aironet_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, aironet ie in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	if (wlc_iem_vs_add_parse_fn(wlc->iemi, FC_BEACON, WLC_IEM_VS_IE_PRIO_CCX_QOS,
	                            wlc_ccx_bcn_parse_qos_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vs_add_parse_fn failed, qos ie in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}

	return (ccx_t *)ci;
error:
#ifndef BCMNODOWN
	MODULE_DETACH_TYPECASTED((ccx_t *)ci, wlc_ccx_detach);
#endif /* BCMNODOWN */
	return NULL;
}

void
BCMATTACHFN(wlc_ccx_detach)(ccx_t *ccxh)
{
	WL_TRACE(("wl: %s: ci = %p\n", __FUNCTION__, OSL_OBFUSCATE_BUF(ccxh)));

	if (ccxh) {
		wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;

		wlc_module_unregister(ci->pub, "ccx", ccxh);
		if (ci->rm_state->ccx) {
			if (ci->rm_state->ccx->plm_burst_timer) {
				wl_free_timer(ci->wlc->wl, ci->rm_state->ccx->plm_burst_timer);
				ci->rm_state->ccx->plm_burst_timer = NULL;
			}
			if (ci->rm_state->ccx->plm_dur_timer) {
				wl_free_timer(ci->wlc->wl, ci->rm_state->ccx->plm_dur_timer);
				ci->rm_state->ccx->plm_dur_timer = NULL;
			}
			MFREE(ci->pub->osh, ci->rm_state->ccx, sizeof(ccx_rm_t));
			ci->rm_state->ccx = NULL;
		}
		MFREE(ci->pub->osh, ci, sizeof(wlc_ccx_info_t));
	}
}

static int
wlc_ccx_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx_info->wlc;

	ccx_bsscfg_priv_t **pccx_bss = CCX_BSSCFG_CUBBY_LOC(ccx_info, cfg);
	ccx_bsscfg_priv_t *cfgi = NULL;

	WL_TRACE(("wl%d: wlc_ccx_init\n", ccx_info->wlc->pub->unit));

	if (!(cfgi = (ccx_bsscfg_priv_t *)MALLOC(wlc->pub->osh, sizeof(ccx_bsscfg_priv_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          ccx_info->wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->pub->osh)));
		return BCME_ERROR;
	}
	*pccx_bss = cfgi;
	bzero(cfgi, sizeof(ccx_bsscfg_priv_t));

	cfgi->cfg = cfg;
	cfgi->wlc = wlc;
	cfgi->wlan_assoc_reason = (uint32)WL_WLAN_ASSOC_REASON_NORMAL_NETWORK;
	cfgi->rn = 1;
	return BCME_OK;
}

static void wlc_ccx_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ctx;
	ccx_bsscfg_priv_t **pccx_bss = CCX_BSSCFG_CUBBY_LOC(ccx_info, cfg);
	ccx_bsscfg_priv_t *cfgi = *pccx_bss;
	wlc_info_t *wlc;

	if (cfgi != NULL) {
		WL_TRACE(("wl%d: wlc_ccx_deinit\n", ccx_info->pub->unit));
		wlc = cfgi->wlc;
		MFREE(wlc->pub->osh, cfgi, sizeof(ccx_bsscfg_priv_t));
		*pccx_bss = NULL;
	}
}


static int
wlc_ccx_down(void *hdl)
{
	wlc_ccx_info_t *ci = hdl;
	int callbacks = 0;

	if (ci->rm_state->ccx) {
		if (ci->rm_state->ccx->plm_burst_timer &&
			!wl_del_timer(ci->wlc->wl, ci->rm_state->ccx->plm_burst_timer))
			callbacks++;
		if (ci->rm_state->ccx->plm_dur_timer &&
			!wl_del_timer(ci->wlc->wl, ci->rm_state->ccx->plm_dur_timer))
			callbacks++;
	}

	return callbacks;
}

/* handling CCX related iovars */
static int
wlc_ccx_doiovar(void *hdl, uint32 actionid,
	void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif)
{
	wlc_ccx_info_t *ci = hdl;
	int32 int_val = 0;
	bool bool_val;
	int err = 0;
	int32 *ret_int_ptr = (int32 *)a;
	wlc_bsscfg_t *cfg;
	ccx_bsscfg_priv_t *cfgi;
	cfg = wlc_bsscfg_find_by_wlcif(ci->wlc, wlcif);
	ASSERT(cfg != NULL);
	cfgi = CCX_BSSCFG_CUBBY(ci, cfg);

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));
	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {
	case IOV_GVAL(IOV_CCX_RM):
		*ret_int_ptr = (int32)ci->pubci.rm;
		break;

	case IOV_SVAL(IOV_CCX_RM):
		ci->pubci.rm = bool_val;
		break;

	case IOV_GVAL(IOV_CCX_RM_LIMIT):
		*ret_int_ptr = (int32)ci->rm_limit;
		break;

	case IOV_SVAL(IOV_CCX_RM_LIMIT):
		ci->rm_limit = (uint)int_val;
		break;

	case IOV_GVAL(IOV_CCX_AUTH_MODE):
		*ret_int_ptr = ci->auth_mode;
		break;

	case IOV_SVAL(IOV_CCX_AUTH_MODE):
		ci->auth_mode = int_val;
		break;

	case IOV_GVAL(IOV_STA_NAME):
		if (BSSCFG_STA(cfg) && !BSS_P2P_ENAB(ci->wlc, cfg)) {

			/* This ioctl return the machine name string */
			if (alen < (int)(strlen(cfgi->staname)+1))
				err = BCME_BUFTOOSHORT;
			else {
				strncpy((char *)a, cfgi->staname, sizeof(cfgi->staname));
			}
		} else {
			return BCME_NOTAP;
		}
		break;

	case IOV_SVAL(IOV_STA_NAME):
		/*
		 * This ioctl sets the station name to given input string.
		 * If the input string length is longer than AIRONET_IE_MAX_NAME_LEN
		 * then the input string will be truncated.
		 */
		if (BSSCFG_STA(cfg) && !BSS_P2P_ENAB(ci->wlc, cfg)) {
			strncpy(cfgi->staname, (char*)a, AIRONET_IE_MAX_NAME_LEN);
			cfgi->staname[AIRONET_IE_MAX_NAME_LEN-1] = '\0';
		} else {
			return BCME_NOTAP;
		}
		break;
	case IOV_GVAL(IOV_AP_NAME):
		if (BSSCFG_STA(cfg) && !BSS_P2P_ENAB(ci->wlc, cfg)) {
			/* This ioctl return the AP name string */
			if (alen < (int)(strlen(cfgi->apname)+1))
				err = BCME_BUFTOOSHORT;
			else
				strncpy((char*)a, cfgi->apname, alen);
		 } else {
			return BCME_NOTAP;
		}
		break;

	case IOV_GVAL(IOV_CCX_VERSION):
		*ret_int_ptr = (int32)cfg->current_bss->ccx_version;
		break;

	case IOV_SVAL(IOV_WLAN_ASSOC_REASON):
		if (BSSCFG_STA(cfg) && !BSS_P2P_ENAB(ci->wlc, cfg)) {
			if (int_val < 0 || int_val > WL_WLAN_ASSOC_REASON_MAX) {
				err = BCME_RANGE;
				break;
			}
			cfgi->wlan_assoc_reason = (uint32)int_val;
		} else {
			return BCME_NOTAP;
		}
		break;


	case IOV_GVAL(IOV_WLAN_ASSOC_REASON):
		if (BSSCFG_STA(cfg) && !BSS_P2P_ENAB(ci->wlc, cfg)) {
			*ret_int_ptr = (int32)cfgi->wlan_assoc_reason;
		} else {
			return BCME_NOTAP;
		}
		break;

#ifdef BCMDBG
	case IOV_SVAL(IOV_TESTS60):
		{
		wlc_ccx_rm_recv(ci, &ci->wlc->pub->cur_etheraddr,
			&cfg->BSSID, 100, (ccx_rm_req_t *)a, 28, cfg);
		}
		break;
#endif


	case IOV_GVAL(IOV_CCX_ENABLE):
		int_val = (int32)ci->pub->_ccx;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_SVAL(IOV_CCX_ENABLE):
		ci->pub->_ccx = bool_val;
		break;

	case IOV_SVAL(IOV_CCX_V4_ONLY):
		ci->pubci.ccx_v4_only = bool_val;
		break;

	case IOV_SVAL(IOV_CCKM_KRK):
		if (BSSCFG_STA(cfg) && !BSS_P2P_ENAB(ci->wlc, cfg)) {
			if (p == NULL || plen != CCKM_KRK_LEN) {
				err = BCME_BADARG;
				break;
			}
			bcopy(p, cfgi->key_refresh_key, CCKM_KRK_LEN);
		} else {
			return BCME_NOTAP;
		}
		break;

#ifdef BCMDBG
	case IOV_GVAL(IOV_CCKM_KRK):
		if (BSSCFG_STA(cfg) && !BSS_P2P_ENAB(ci->wlc, cfg)) {
			if (a == NULL || alen < CCKM_KRK_LEN) {
				err = BCME_BADARG;
				break;
			}
			bcopy(cfgi->key_refresh_key, a, CCKM_KRK_LEN);
		} else {
			return BCME_NOTAP;
		}
		break;

#endif /* BCMDBG */
	case IOV_GVAL(IOV_CCKM_RN):
		if (BSSCFG_STA(cfg) && !BSS_P2P_ENAB(ci->wlc, cfg)) {
			if (a == NULL || alen < sizeof(cfgi->rn)) {
				err = BCME_BADARG;
				break;
			}
#if defined(BCMINTSUP) && defined(BCMSUP_PSK) && defined(BCMCCX)
			if (SUP_ENAB(ci->wlc->pub) && CCX_ENAB(ci->wlc->pub) &&
					BSS_SUP_ENAB_WPA(ci->wlc->idsup, cfg)) {
				int rn;
				if (wlc_cckm_get_rn(ci->wlc->ccxsup, cfg, &rn)) {
					memcpy(a, &rn, sizeof(rn));
				} else {
					return BCME_ERROR;
				}
			} else
#endif /* BCMINTSUP && BCMSUP_PSK && BCMCCX */
			bcopy(&cfgi->rn, a, sizeof(cfgi->rn));
		} else {
			return BCME_NOTAP;
		}
		break;


	case IOV_GVAL(IOV_CCX_DROAM_ACTIVATED):
		int_val = wlc_ccx_is_droam(ci->wlc->ccx, cfg);
		memcpy(a, &int_val, vsize);
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* watchdog */
static void wlc_ccx_watchdog(void *hdl)
{
	wlc_ccx_info_t *ci = hdl;
#if defined(BCMINTSUP)
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_bsscfg_t *cfg = NULL;
	uint8 idx;
	ccx_bsscfg_priv_t *cfgi = NULL;
#endif /* BCMINTSUP */

	(void)ci;

	if (!CCX_ENAB(ci->pub))
		return;

#if defined(BCMINTSUP)
	FOREACH_STA(wlc, idx, cfg) {
		if (cfg != NULL) {
			cfgi = CCX_BSSCFG_CUBBY(ci, cfg);
			if ((cfgi != NULL) && SUP_ENAB(wlc->pub) && !BSS_P2P_ENAB(wlc, cfg) &&
				wlc_sup_getleapauthpend(wlc->ccxsup, cfg) &&
				((wlc->pub->now - cfgi->leap_start) >
				CCX_LEAP_ROGUE_DURATION)) {
				wlc_ccx_rogue_timer(wlc->ccxsup, cfg, &cfg->prev_BSSID);
			}
		}
	}
#endif /* BCMINTSUP */

}

bool
wlc_ccx_is_droam(ccx_t *ccxh, wlc_bsscfg_t* cfg)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ci, cfg);

	return cfgi->droam_rf_roam;
}


void
wlc_ccx_on_join_adopt_bss(ccx_t* ci, wlc_bsscfg_t* cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	bool rf_info_avail = FALSE;
	uint i = 0;

	if (cfgi->ccx_network) {
		/* search if rf info available for this BSS */
		for (; i < cfgi->droam_ap_num; i++) {
			if (!bcmp((char*)&cfgi->droam_ap_list[i].bssid,
				(char*)&cfg->target_bss->BSSID, ETHER_ADDR_LEN) &&
				cfgi->droam_ap_list[i].rf_ie) {
				rf_info_avail = TRUE;
				break;
			}
		}
	}

	if (rf_info_avail) {
		wlcband_t *band = cfg->wlc->band;
		/* apply roam parameters in RF subelement */
		band->roam_trigger = cfgi->droam_ap_list[i].roam_trigger;
		/* roam delta is no less than 10 db */
		band->roam_delta = MAX(cfgi->droam_ap_list[i].roam_delta, 10);
		if (NBANDS(cfg->wlc) > 1) {
			band = cfg->wlc->bandstate[OTHERBANDUNIT(cfg->wlc)];
			band->roam_trigger = cfgi->droam_ap_list[i].roam_trigger;
			band->roam_delta = MAX(cfgi->droam_ap_list[i].roam_delta, 10);
		}
		cfgi->droam_rf_roam = TRUE;
	} else {
		if (!cfgi->ccx_network) {
			/* clear existing AP channel list */
			cfgi->ap_channel_num = 0;
			/* clear existing directed roaming AP list */
			cfgi->droam_ap_num = 0;
		}
		/* clear AP RF information flag */
		cfgi->droam_rf_roam = FALSE;
	}

	/* clear CCX fast roam indication */
	cfgi->fast_roam = FALSE;

	/* CCXv4 S61 */
	/* default keep alive value */
	if (cfgi->ccx_network)
		wlc_ap_keep_alive_count_update(ccx_info->wlc, KEEP_ALIVE_INTERVAL);
	/* reset link test frame number */
	cfgi->linktest_frm_num = 0;


	/* Save the information regarding the previous BSS to send to the new AP */

	WL_WSEC(("wl%d: wlc_ccx_on_join_adopt_bss(): building roam IAPP report\n",
		cfgi->wlc->pub->unit));
	bcopy(cfg->prev_BSSID.octet, cfgi->prev_ap_mac.octet, ETHER_ADDR_LEN);
	cfgi->prev_channel = CHSPEC_CHANNEL(cfg->current_bss->chanspec);
	cfgi->disassoc_time = cfg->roam->time_since_bcn;
}

void
wlc_ccx_on_leave_bss(ccx_t* ci, wlc_bsscfg_t *cfg)
{

	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	wlc_info_t *wlc = (wlc_info_t *)cfgi->wlc;
	if (WSEC_CKIP_MIC_ENABLED(cfg->wsec) || WSEC_CKIP_KP_ENABLED(cfg->wsec)) {
		uint32 val = cfg->wsec;

		val &= ~(CKIP_MIC_ENABLED | CKIP_KP_ENABLED | WSEC_SWFLAG);
		if (wlc_keymgmt_wsec(wlc, cfg, val)) {
			WL_ERROR(("wl%d: %s: error clearing CKIP settings\n",
				wlc->pub->unit, __FUNCTION__));
		}
	}

	/* If RF subelement roam parameters applied, restore roam parameters
	 * before leave BBS
	 */
	if (cfgi->droam_rf_roam) {
		wlcband_t *band = wlc->band;
		band->roam_trigger = band->roam_trigger_def;
		band->roam_delta = band->roam_delta_def;
		if (NBANDS(wlc) > 1) {
			band = wlc->bandstate[OTHERBANDUNIT(wlc)];
			band->roam_trigger = band->roam_trigger_def;
			band->roam_delta = band->roam_delta_def;
		}
	}

	cfgi->droam_rf_roam = FALSE;

	/* reset roaming variables */
	cfgi->fast_roam = FALSE;

	/* if tpc applied */
	if (cfgi->tpc) {
		/* reset tx power if needed */
		int8 cur_qtxpwr;
		ppr_t *txpwr;
		wlc_phy_t *pi = WLC_PI(cfgi->wlc);

		wlc_phy_txpower_get(pi, &cur_qtxpwr, NULL);
		if (cur_qtxpwr == cfgi->tpc_qtxpwr) {
			if ((txpwr = ppr_create(wlc->osh, PPR_CHSPEC_BW(wlc->chanspec))) == NULL) {
				return;
			}

			wlc_channel_reg_limits(wlc->cmi, wlc->chanspec, txpwr);
			ppr_apply_max(txpwr, WLC_TXPWR_MAX);
			/* restore tx power */
			wlc_phy_txpower_set(pi, cfgi->cur_qtxpwr, FALSE, txpwr);
			ppr_delete(wlc->osh, txpwr);
		}
		cfgi->tpc = FALSE;
}
	/* restore the original value of keep alive timeout */
	wlc_ap_keep_alive_count_default(cfgi->wlc);
}

int
wlc_ccx_chk_iapp_frm(ccx_t *ccxh, struct wlc_frminfo *f, wlc_bsscfg_t* cfg)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	ccx_iapp_hdr_t *iapp;
	uint iapp_len;
	uint16 iapp_id;
	uint data_len;

	iapp = (ccx_iapp_hdr_t *)((char *)PKTDATA(ci->pub->osh, f->p) + ETHER_HDR_LEN +
		DOT11_LLC_SNAP_HDR_LEN);
	iapp_len = PKTLEN(ci->pub->osh, f->p) - (ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN);

	iapp_id = (ntoh16(iapp->id_len) & CCX_IAPP_ID_MASK) >> CCX_IAPP_ID_SHIFT;
	data_len = (ntoh16(iapp->id_len) & CCX_IAPP_LEN_MASK) - CCX_IAPP_HDR_LEN;

	/* validate id message type and length */
	if (iapp_id != CCX_IAPP_ID_CONTROL || iapp_len < (CCX_IAPP_HDR_LEN + data_len))
		return 0;

	switch (iapp->type) {
	case CCX_IAPP_TYPE_ROAM:
		if (iapp->subtype == CCX_IAPP_SUBTYPE_ROAM_REP) {
			WL_ASSOC(("ROAM: CCX FAST, received adjacent AP list IAPP frame\n"));
			wlc_ccx_process_roam_iapp(ci, (ccx_neighbor_rept_ie_t*)iapp->data,
				data_len, FALSE, cfg);
			return -1;
		} else if (iapp->subtype == CCX_IAPP_SUBTYPE_ROAM_REQ) {
			/* must be unicast packet for CCX directed roam request */
			if (bcmp(f->da, (char*)&ci->pub->cur_etheraddr, ETHER_ADDR_LEN))
				break;
			WL_ASSOC(("ROAM: CCX FAST, received directed roam IAPP frame\n"));
			wlc_ccx_process_droam_iapp(ci, (ccx_neighbor_rept_ie_t*)iapp->data,
				data_len, cfg);
			return -1;
		}
		break;
	case CCXv2_IAPP_TYPE_ROAM:
		/* must be unicast packet for CCX roam request */
		if (bcmp(f->da, (char*)&ci->pub->cur_etheraddr, ETHER_ADDR_LEN))
			break;
		if (iapp->subtype == CCXv2_IAPP_SUBTYPE_ROAM_REQ) {
			WL_ASSOC(("ROAM: CCX FAST, received adjacent AP list IAPP frame(v2)\n"));
			wlc_ccxv2_process_roam_iapp(ci, (ccx_roam_ap_ie_t*)iapp->data,
				data_len, cfg);
			return -1;
		}
		break;
	case CCX_IAPP_TYPE_RM:
		if (ci->pubci.rm && (iapp->subtype == CCX_IAPP_SUBTYPE_REQ)) {
			/* looks like a valid radio mgmt request packet */
			wlc_ccx_rm_recv(ci, (struct ether_addr*)f->da,
				(struct ether_addr*)f->sa,
				D11RXHDR_ACCESS_VAL(f->rxh, ci->pub->corerev, RxTSFTime),
				(ccx_rm_req_t*)iapp->data, data_len, cfg);
			return -1;
		}
		break;
	case CCX_IAPP_TYPE_LINK_TEST:
		if (iapp->subtype == CCX_IAPP_SUBTYPE_REQ) {
			/* looks like a valid link test request packet */
			wlc_ccx_send_linktest_rpt((ccx_t*)ci,
				(struct ether_addr*)f->sa, /* it is dst address */
				(struct ether_addr*)f->da, /* it is src address */
				(ccx_link_test_t*)iapp->data,
				data_len,
				wlc_lq_recv_rssi_compute(ci->wlc, f->wrxh), cfg);
			return -1;
		}
		break;
	default:
		break;
	}

	return 0;

}

void
wlc_ccx_on_roam_start(ccx_t* ci, wlc_bsscfg_t *cfg, uint reason)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	wlc_info_t *wlc = (wlc_info_t *)cfgi->wlc;
	/* clear previous CKIP settings (facilitating shared key!) */
	if (WSEC_CKIP_MIC_ENABLED(cfg->wsec) || WSEC_CKIP_KP_ENABLED(cfg->wsec)) {
		uint32 val = cfg->wsec;
#ifdef BCMDBG_ERR
		const char *msg_name = (reason == WLC_E_DEAUTH_IND)? "deauth" : "disassoc";
#endif /* BCMDBG_ERR */
		val &= ~(CKIP_MIC_ENABLED | CKIP_KP_ENABLED | WSEC_SWFLAG);

		if (wlc_keymgmt_wsec(wlc, cfg, val)) {
			WL_ERROR(("wl%d: %s: error clearing CKIP settings on "
				"receipt of %s\n", wlc->pub->unit, __FUNCTION__, msg_name));
		}
	}

	/* issue a CCX fast roam scan request if roam not disabled */
	if (!cfg->roam->off && cfgi->ap_channel_num) {
		cfgi->fast_roam = TRUE;
		WL_ASSOC(("wl%d: %s: ROAM: fast_roam begins\n", wlc->pub->unit, __FUNCTION__));
	}
}

static void
wlc_ccx_linktest_tx_complete(wlc_info_t *wlc, uint txstatus, void *arg)
{
	ccx_bsscfg_priv_t *cfgi = (ccx_bsscfg_priv_t*) arg;
	uint tx_frame_count = (txstatus & TX_STATUS_FRM_RTX_MASK) >> TX_STATUS_FRM_RTX_SHIFT;
	uint txretry = (uint)((tx_frame_count > 1) ? (tx_frame_count - 1) : 0);

	if (!cfgi->linktest_pending) {
		return;
	}

	cfgi->linktest_pending = FALSE;
	/* save retries when send link test frame */
	cfgi->linktest_txretry = txretry;
}

int
wlc_ccx_set_auth(ccx_t *ci, wlc_bsscfg_t* cfg)
{
	int auth = cfg->auth;
	wlc_ccx_info_t* ccx_info = (wlc_ccx_info_t*)ci;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	/* LEAP needs its trick auth mode in the association
	 * request, but NDIS could be discomfitted by seeing
	 * that value.  Use it here, but hide it otherwise.
	 */
	if (cfgi->leap_on)
		auth = DOT11_LEAP_AUTH;
	/* auth mode will be set for PEAP/GTC */
	if (ccx_info->auth_mode) {
		if (ccx_info->auth_mode == AUTH_MODE_MAGIC || !cfgi->ccx_network)
			auth = 0;
		else
			auth = ccx_info->auth_mode;
	}

	return auth;
}

void
wlc_ccx_iapp_roam_rpt(ccx_t *ci, wlc_bsscfg_t *cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t  *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);
	wlc_ccx_send_roam_rpt(ci, cfg);
	cfgi->prev_channel = 0;
}

static void
wlc_ccx_send_roam_rpt(ccx_t *ci, wlc_bsscfg_t *cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	uint8 *ssid = cfg->current_bss->SSID;
	uint8 ssid_len = cfg->current_bss->SSID_len;
	uint8 channel_prev_ap = cfgi->prev_channel;
	struct ether_addr *bssid = &cfgi->prev_ap_mac;
	uint disassoc_time = cfgi->disassoc_time;
	wlc_info_t *wlc = (wlc_info_t *)ccx_info->wlc;
	uint alloc_len;
	ccx_roam_iapp_pkt_t *iapp_pkt;
	ccx_roam_reason_ie_t *reason_ie;
	osl_t *osh, *p;
	uint16 iapp_len;
	uint16 ether_type;


	WL_WSEC(("wl%d: wlc_ccx_send_roam_rpt: report previous AP\n", wlc->pub->unit));
	WL_ASSOC(("wl%d: %s: ROAM: CCX version %d AP\n",
		wlc->pub->unit, __FUNCTION__, cfg->current_bss->ccx_version));

	/*
	 * In pre-CCXv4, this reporting was not required if it is an initial association.
	 * CCXv4 requires reason code generation in all association. Since initial association
	 * does not have previous AP by definition, faked information is supplied in previous AP
	 * infon field.
	 */

	if ((!channel_prev_ap) && (cfg->current_bss->ccx_version < 4)) {
		WL_ASSOC(("wl%d: %s: ROAM: skip roam report for initial assoc pre-V4 AP\n",
			wlc->pub->unit, __FUNCTION__));
		return;
	}
	else {
		WL_ASSOC(("wl%d: %s: ROAM: sending roam report\n", wlc->pub->unit, __FUNCTION__));
	}

	osh = wlc->pub->osh;

	alloc_len = sizeof(ccx_roam_iapp_pkt_t);
	/* iapp length field excludes snap */
	iapp_len = CCX_ROAM_IAPP_MSG_SIZE - sizeof(struct dot11_llc_snap_header);
	ether_type = CCX_ROAM_IAPP_MSG_SIZE;

	/* add space for reason ie for CCX version larger or equal to 4 */
	if (cfg->current_bss->ccx_version >= 4) {
		alloc_len += sizeof(ccx_roam_reason_ie_t);
		iapp_len += sizeof(ccx_roam_reason_ie_t);
		ether_type += sizeof(ccx_roam_reason_ie_t);
	}

	if ((p = PKTGET(osh, alloc_len + TXOFF, TRUE)) == NULL) {
		WL_ERROR(("wl%d: %s: pktget error for len %d\n", wlc->pub->unit,
			__FUNCTION__, alloc_len));
		WLCNTINCR(wlc->pub->_cnt->txnobuf);
		return;
	}
	ASSERT(ISALIGNED(PKTDATA(osh, p), sizeof(uint32)));

	/* reserve TXOFF bytes of headroom */
	PKTPULL(osh, p, TXOFF);
	PKTSETLEN(osh, p, alloc_len);

	iapp_pkt = (ccx_roam_iapp_pkt_t *)PKTDATA(osh, p);

	/* clear prev. AP info */
	bzero((char *)&iapp_pkt->ap_ie, sizeof(ccx_roam_ap_ie_t));

	bcopy((char *)&cfg->BSSID, (char *)&iapp_pkt->eth.ether_dhost, ETHER_ADDR_LEN);
	bcopy((char *)&cfg->cur_etheraddr, (char *)&iapp_pkt->eth.ether_shost,
	      ETHER_ADDR_LEN);
	iapp_pkt->eth.ether_type = hton16(ether_type);

	bcopy(CISCO_AIRONET_SNAP, (char *)&iapp_pkt->snap, DOT11_LLC_SNAP_HDR_LEN);

	iapp_pkt->msg_len = hton16(iapp_len);
	iapp_pkt->msg_type = CCXv2_IAPP_TYPE_ROAM;
	iapp_pkt->fcn_code = 0;
	bcopy((char *)&cfg->BSSID, (char *)&iapp_pkt->dest_mac, ETHER_ADDR_LEN);
	bcopy((char *)&cfg->cur_etheraddr, (char *)&iapp_pkt->src_mac, ETHER_ADDR_LEN);

	/* fill Adjacent AP Report portion for previous AP */
	iapp_pkt->ap_ie.tag = hton16(CCX_ROAM_ADJ_AP_TAG);
	iapp_pkt->ap_ie.len = hton16(48);
	bcopy(CISCO_AIRONET_OUI, iapp_pkt->ap_ie.oui, DOT11_OUI_LEN);
	/* iapp_pkt->ap_ie.ver = 0; it is not necessary due to bzero above */

	/* Fill previous AP info for roaming */
	if (channel_prev_ap) {
		bcopy((char *)bssid, (char *)&iapp_pkt->ap_ie.mac, ETHER_ADDR_LEN);
		iapp_pkt->ap_ie.channel = hton16(channel_prev_ap);
		iapp_pkt->ap_ie.ssid_len = hton16(ssid_len);
		bcopy((char *)ssid, (char *)iapp_pkt->ap_ie.ssid, ssid_len);
		iapp_pkt->ap_ie.disassoc_time = hton16((uint16)disassoc_time);
	}

	/* add reason ie for CCX version 4 or larger */
	if (cfg->current_bss->ccx_version >= 4) {
		reason_ie = (ccx_roam_reason_ie_t *)(iapp_pkt + 1);
		reason_ie->tag = hton16(CCX_ROAM_REASON_TAG);
		reason_ie->len = hton16(sizeof(ccx_roam_reason_ie_t) - 4);
		bcopy(CISCO_AIRONET_OUI, reason_ie->oui, DOT11_OUI_LEN);
		reason_ie->ver = 0;
		reason_ie->reason = wlc_ccx_get_roam_reason(wlc->ccx, cfg);
	}

	wlc_sendpkt(wlc, p, cfg->wlcif);
}

/* convert driver roaming reason to CCX roaming reason */
static uint8
wlc_ccx_get_roam_reason(ccx_t *ci, wlc_bsscfg_t* cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	uint8 reason, roam_reason;

	roam_reason = (cfg->roam->reason == WLC_E_REASON_FAST_ROAM_FAILED) ?
		cfgi->orig_reason : cfg->roam->reason;

	switch (roam_reason) {
	case 0:				/* no roam reason - initial association */
		if (cfgi->wlan_assoc_reason != (uint32)WL_WLAN_ASSOC_REASON_NORMAL_NETWORK) {
			reason = CCX_ROAM_IN_NET; /* test plan 4.4.5 */
		}
		else {
			reason = CCX_ROAM_FIRST_ASSOC; /* test plan 4.4.4 */
		}
		break;
	case WLC_E_REASON_LOW_RSSI:	/* roamed due to low RSSI */
		reason = CCX_ROAM_NORMAL;
		break;
	case WLC_E_REASON_DEAUTH:	/* roamed due to DEAUTH indication */
	case WLC_E_REASON_DISASSOC:	/* roamed due to DISASSOC indication */
	case WLC_E_REASON_BCNS_LOST:	/* roamed due to lost beacons */
		reason = CCX_ROAM_LINK_DOWN;
		break;
	case WLC_E_REASON_DIRECTED_ROAM:	/* roamed due to request by AP */
		reason = CCX_ROAM_DIRECTED_ROAM;
		break;
	case WLC_E_REASON_TSPEC_REJECTED:	/* roamed due to TSPEC rejection */
		reason = CCX_ROAM_AP_INCAPACITY;
		break;
	case WLC_E_REASON_BETTER_AP:	/* roamed due to finding better AP */
		reason = CCX_ROAM_BETTER_AP;
		break;
	default:
		reason = CCX_ROAM_UNSPECIFIED;
		break;
	}

	WL_ASSOC(("wl%d: %s: ROAM: roam_reason 0x%x association reason 0x%x\n",
		cfg->wlc->pub->unit, __FUNCTION__, cfg->roam->reason, reason));
	return reason;
}

/* set as high priority as possible for CCX tx pkt */
static int
wlc_ccx_get_txpkt_prio(ccx_t *ci, struct ether_addr* da)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t*)ci;
	wlc_info_t *wlc = (wlc_info_t *)ccx_info->wlc;
	struct scb *scb;
	int prio;

	if (!CAC_ENAB(ccx_info->pub))
		return MAXPRIO;

	if ((scb = wlc_scbfindband(wlc, wlc_bsscfg_primary(wlc), da,
	                           CHSPEC_WLCBANDUNIT(wlc->home_chanspec)))) {
		for (prio = MAXPRIO; prio > PRIO_8021D_NONE; prio--) {
			if (wlc_cac_is_traffic_admitted(wlc->cac, WME_PRIO2AC(prio), scb))
				return prio;
		}
	}

	return PRIO_8021D_BE;
}

/* CCXv4 S62 */
static void
wlc_ccx_send_linktest_rpt(ccx_t *ci, struct ether_addr* da,
	struct ether_addr* sa, ccx_link_test_t* req_pkt, int req_len, int rssi, wlc_bsscfg_t *cfg)
{
	void *p;
	uint8* pbody;
	uint body_len;
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t*)ci;
	struct dot11_llc_snap_header *snap;
	ccx_iapp_hdr_t* ccx_iapp;
	ccx_link_test_t *link_resp;
	wlc_info_t *wlc = ccx_info->wlc;
	int prio;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	WL_INFORM(("wl%d: wlc_ccx_send_linktest_rpt: link test response\n", ccx_info->pub->unit));

	/* make sure this is a valid CCX link test request packet */
	if (req_len < CCX_LINK_TEST_REQ_LEN) {
		WL_ERROR(("wl%d: %s: link test request packet too "
			"short, len = %d expected at least %d", wlc->pub->unit, __FUNCTION__,
			req_len, CCX_LINK_TEST_REQ_LEN));
		return;
	}

	body_len = DOT11_LLC_SNAP_HDR_LEN + CCX_IAPP_HDR_LEN + req_len;

	p = wlc_dataget(wlc, da,
		(uint16)body_len, /* 803.2 length_type-like field */
		sizeof(struct ether_header) + body_len); /* buffer includes ether_header */
	if (p == NULL) {
		WL_ERROR(("wl%d: %s: wlc_dataget returns null\n",
			wlc->pub->unit, __FUNCTION__));
		return;
	}

	pbody = (uint8*)PKTDATA(wlc->pub->osh, p);

	snap = (struct dot11_llc_snap_header *)(pbody + ETHER_HDR_LEN);
	bcopy(CISCO_AIRONET_SNAP, snap, DOT11_LLC_SNAP_HDR_LEN);

	ccx_iapp = (ccx_iapp_hdr_t *)((int8*)snap + DOT11_LLC_SNAP_HDR_LEN);
	ccx_iapp->id_len = hton16(CCX_IAPP_ID_CONTROL | (body_len - DOT11_LLC_SNAP_HDR_LEN));
	ccx_iapp->type = CCX_IAPP_TYPE_LINK_TEST;
	ccx_iapp->subtype = CCX_IAPP_SUBTYPE_ROAM_REP;
	bcopy((char *)&cfg->BSSID, ccx_iapp->da.octet, ETHER_ADDR_LEN);
	bcopy(cfg->cur_etheraddr.octet, ccx_iapp->sa.octet, ETHER_ADDR_LEN);

	link_resp = (ccx_link_test_t *)((int8*)ccx_iapp + CCX_IAPP_HDR_LEN);

	/* If the rssi is negative, we have to make it positive. */
	if (rssi < 0)
		rssi = 0 - rssi;
	/* clamp link_margin value if we overflow an int8 */
	link_resp->rssi = MIN(rssi, 127);

	/* increment frame number */
	cfgi->linktest_frm_num++;
	/* report current frame number */
	link_resp->frm_num = hton16(cfgi->linktest_frm_num);
	/* last link test packet tx retries */
	link_resp->txretried = (uint8)cfgi->linktest_txretry;
	/* unchanged fields from request frame */
	link_resp->time = req_pkt->time;
	link_resp->rsq = req_pkt->rsq;
	link_resp->rss = req_pkt->rss;
	link_resp->sqp = req_pkt->sqp;
	link_resp->ssp = req_pkt->ssp;

	bcopy(req_pkt->data, link_resp->data, req_len - CCX_LINK_TEST_REQ_LEN);

	/* Set high priority so that the packet will get enqueued at the end of
	 * higher precedence queue in order
	 */
	prio = wlc_ccx_get_txpkt_prio(ci, da);
	PKTSETPRIO(p, prio);

	/* if new request come in before current response sent out,
	 * AP may think there is problem and timeout the previous request
	 */
	if (cfgi->linktest_pending) {
		WL_ERROR(("wl%d: %s: get a new link test request before "
			"current link test response sent out\n",
			wlc->pub->unit, __FUNCTION__));
	}

	/* save packet pointer. */
	cfgi->linktest_pending = TRUE;
	/* clean retry count */
	cfgi->linktest_txretry = 0;

	wlc_pcb_fn_register(wlc->pcb, wlc_ccx_linktest_tx_complete, cfgi, p);
	wlc_sendpkt(wlc, p, NULL);
}

/* CCXv2 Transmit Power Control, sec S31 */
void
wlc_ccx_tx_pwr_ctl(ccx_t *ccxh, uint8 *tlvs, int tlvs_len, wlc_bsscfg_t* cfg)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ci, cfg);

	wlc_phy_t *pi = WLC_PI(ci->wlc);
	ccx_cell_pwr_t *pwr_ie;

	/* search for the CCXv2 Cell Power Limit IE */
	pwr_ie = (ccx_cell_pwr_t*)bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_CELL_PWR_ID);

	/* 11h Power Constraint takes precedence, so search for the 11h Power Constraint IE
	 */
	if (pwr_ie && !bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_PWR_CONSTRAINT_ID)) {
		/* validate the pwr_ie */
		if (pwr_ie->len >= 5 && !bcmp(pwr_ie->oui, CISCO_AIRONET_OUI, 3) &&
			pwr_ie->ver == 0) {
			int8 cur_qtxpwr;
			/* get current tx power */
			wlc_phy_txpower_get(pi, &cur_qtxpwr, NULL);
			if (cfgi->tpc && cfgi->tpc_qtxpwr == cur_qtxpwr)
				/* roaming case.  always use host-set tx power level
				 * in comparison
				 */
				cur_qtxpwr = (int8)cfgi->cur_qtxpwr;
			/* if host-set tx power level is higher than the limit */
			if ((int32)(cur_qtxpwr) >
				((int32)pwr_ie->power * WLC_TXPWR_DB_FACTOR)) {
				/* adjust tx power */
				ppr_t *txpwr;

				if ((txpwr = ppr_create(ci->wlc->osh,
					PPR_CHSPEC_BW(ci->wlc->chanspec))) == NULL) {
					return;
				}

				wlc_channel_reg_limits(ci->wlc->cmi, ci->wlc->chanspec, txpwr);
				ppr_apply_max(txpwr, WLC_TXPWR_MAX);

				if (pwr_ie->power <
					(uint8)phy_tpc_get_min_power_limit(pi)) {
					WL_ERROR(("Not Supported Power Level\n"));
					pwr_ie->power =
						(uint8)phy_tpc_get_min_power_limit(pi);
				}

				/* restore tx power */
				wlc_phy_txpower_set(pi,
					pwr_ie->power * WLC_TXPWR_DB_FACTOR, FALSE, txpwr);
				ppr_delete(ci->wlc->osh, txpwr);
				cfgi->cur_qtxpwr = cur_qtxpwr;
				cfgi->tpc_qtxpwr = pwr_ie->power * WLC_TXPWR_DB_FACTOR;
				cfgi->tpc = TRUE;
			}
		}
	}
}


static uint8
wlc_ccx_get_rx_band_type(uint rx_band, wlc_bss_info_t *BSS)
{
	uint8 rx_phy_type;

	if (BAND_5G(rx_band)) {
		rx_phy_type = CCX_RM_PHY_OFDM;
	} else {
		/* base type on supported rates, default DSS */
		rx_phy_type = CCX_RM_PHY_DSS;
		if (wlc_rateset_isofdm(BSS->rateset.count, BSS->rateset.rates))
			rx_phy_type = CCX_RM_PHY_ERP;
		else {
			uint i;
			for (i = 0; i < BSS->rateset.count; i++) {
				uint8 rate = BSS->rateset.rates[i] & RATE_MASK;
				if (rate == WLC_RATE_5M5 || rate == WLC_RATE_11M) {
					rx_phy_type = CCX_RM_PHY_HRDSS;
					break;
				}
			}
		}
	}

	return rx_phy_type;
}

void
wlc_ccx_update_BSS(ccx_t *ccxh, uint rx_band, wlc_bss_info_t *BSS)
{
	/* save the CCX RM PHY Type */
	BSS->ccx_phy_type = wlc_ccx_get_rx_band_type(rx_band, BSS);
}


static void
wlc_ccxv2_process_roam_iapp(wlc_ccx_info_t *ci, ccx_roam_ap_ie_t *ap_ie, uint len,
	wlc_bsscfg_t* cfg)
{
	uint idx = 0;
	uint off;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ci, cfg);

	if (len < CCX_ROAM_AP_IE_LEN)
		return;

	/* do not overwrite v4 neighbor AP list */
	if (cfgi->droam_ap_num)
		return;

	while (len && (idx < CCX_ROAM_SCAN_CHANNELS)) {
		if (ap_ie->tag == ntoh16(CCX_ROAM_ADJ_AP_TAG)) {
			uint16 channel = ntoh16(ap_ie->channel);
			uint i;

			if (channel) {
				/* add unique channels to the list */
				for (i = 0; i < idx; i++)
					if (cfgi->ap_chanspec_list[i] ==
						CH20MHZ_CHSPEC(channel))
						break;
				if (i == idx)
					cfgi->ap_chanspec_list[idx++] =
						CH20MHZ_CHSPEC(channel);
			}
		}

		off = ntoh16(ap_ie->len) + 4;
		ap_ie = (ccx_roam_ap_ie_t *)((char *)ap_ie + off);
		len = (len > off) ? (len - off) : 0;
	}

	cfgi->ap_channel_num = idx;
}

static void
wlc_ccx_process_roam_iapp(wlc_ccx_info_t *ci, ccx_neighbor_rept_ie_t *rept_ie, uint len,
	bool append, wlc_bsscfg_t* cfg)
{
	bcm_tlv_t *tlv;
	ccx_neighbor_rept_ie_t *ie;
	uint channel_idx, list_idx;
	wlc_info_t *wlc = (wlc_info_t *)cfg->wlc;
	uint i, off;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ci, cfg);
	ccx_radio_param_subie_t *subie;
#if defined(BCMDBG) || defined(WLMSG_ASSOC) || defined(WLMSG_ROAM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */

	(void)wlc;
	tlv = (bcm_tlv_t *)rept_ie;
	if (append) {
		channel_idx = cfgi->ap_channel_num;
		list_idx = cfgi->droam_ap_num;
	} else {
		channel_idx = 0;
		list_idx = 0;
		bzero(cfgi->droam_ap_list, sizeof(cfgi->droam_ap_list));
	}

	while (len) {
		ie = (ccx_neighbor_rept_ie_t *)bcm_parse_tlvs((void *)tlv, len,
			CCX_ROAM_NEIGHBOR_REPT_ID);
		if (!ie) {
			break;
		}

		if (ie->channel) {
			/* add unique channels to the list */
			for (i = 0; i < channel_idx; i++) {
				if (CHSPEC_CHANNEL(cfgi->ap_chanspec_list[i]) ==
						ie->channel) {
					break;
				}
			}
			if (i == channel_idx) {
				cfgi->ap_chanspec_list[channel_idx++] =
					CH20MHZ_CHSPEC(ie->channel);
			}
			WL_ROAM(("IAPP: Neighbor %s, CH%03d\n", bcm_ether_ntoa(&ie->mac, eabuf),
				ie->channel));
			WL_ASSOC(("wl%d: %s: ROAM: neighbor report ie: BSS %s, channel %d,"
				" cur_ap: %s\n",
				wlc->pub->unit, __FUNCTION__,
				bcm_ether_ntoa(&ie->mac, eabuf), ie->channel,
				bcm_ether_ntoa(&cfg->BSSID, eabuf)));

			/* save bssid */
			bcopy((char *)&ie->mac, (char *)&cfgi->droam_ap_list[list_idx].bssid,
				ETHER_ADDR_LEN);

			cfgi->droam_ap_list[list_idx].rf_ie = FALSE; /* mark as rf_ie not present */

			/* process RF subelement if present */
			subie = (ccx_radio_param_subie_t *)NULL;
			if (ie->len > CCX_NEIGHBOR_REPT_IE_LEN_W_H) {
				subie = (ccx_radio_param_subie_t *)bcm_parse_tlvs(
					(void *)((char *)ie + CCX_NEIGHBOR_REPT_IE_LEN),
					ie->len - CCX_NEIGHBOR_REPT_IE_LEN_W_H,
					CCX_ROAM_SUB_RF_PARAMS);
			}

			if (subie) {
				wlc_ccx_process_droam_rf_ie(ci, subie, list_idx,
					!bcmp((char *)&ie->mac, &cfg->BSSID, ETHER_ADDR_LEN), cfg);
			}

			list_idx++;
		}

		off = (uint)((char *)ie - (char *)tlv) + ie->len + TLV_HDR_LEN;
		tlv = (bcm_tlv_t *)((char *)tlv + off);
		len = (len > off) ? (len - off) : 0;

		if (channel_idx == CCX_ROAM_SCAN_CHANNELS) {
			WL_ERROR(("wl%d: %s: too many channels",
				wlc->pub->unit, __FUNCTION__));
			break;
		}
		if (list_idx == CCX_ROAM_AP_ELEMENTS) {
			WL_ERROR(("wl%d: %s: too many AP elements",
				wlc->pub->unit, __FUNCTION__));
			break;
		}
	}

	cfgi->ap_channel_num = channel_idx;
	cfgi->droam_ap_num = list_idx;
	WL_ROAM(("IAPP neighbor list, %d channels, %d APs\n", channel_idx, list_idx));
}

static void
wlc_ccx_process_droam_rf_ie(wlc_ccx_info_t *ci, ccx_radio_param_subie_t *rf_ie,
	int idx, bool curr_ap, wlc_bsscfg_t* cfg)
{
	wlc_info_t *wlc = (wlc_info_t *)cfg->wlc;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ci, cfg);

	/* check parameters */
	if (rf_ie->roam_delta < 0)
		return;

	/* save minimum rssi */
	cfgi->droam_ap_list[idx].min_rssi = rf_ie->min_rssi;
	/* save AP's tx power */
	cfgi->droam_ap_list[idx].ap_tx_pwr = rf_ie->ap_tx_pwr;
	cfgi->droam_ap_list[idx].roam_time = rf_ie->roam_time;
	/* save roaming parameters */
	cfgi->droam_ap_list[idx].roam_trigger = rf_ie->roam_trigger;
	cfgi->droam_ap_list[idx].roam_delta = rf_ie->roam_delta;
	WL_ASSOC(("wl%d: %s: cur_ap = %s, ROAM: rf_ie: roam_trigger %d roam_delta %d min_rssi %d"
		" ap_tx_pwr %d roam_time %d\n",
		wlc->pub->unit, __FUNCTION__,
		curr_ap ? "TRUE" : "FALSE",
		rf_ie->roam_trigger, rf_ie->roam_delta,
		rf_ie->min_rssi, rf_ie->ap_tx_pwr, rf_ie->roam_time));

	/* if this is for current AP, set roaming parameters with RF subelement */
	if (curr_ap) {
		wlcband_t *band = wlc->band;
		/* apply roam parameters in RF subelement */
		band->roam_trigger = rf_ie->roam_trigger;
		/* roam delta is no less than 10 db */
		band->roam_delta = MAX(rf_ie->roam_delta, 10);
		if (NBANDS(wlc) > 1) {
			band = wlc->bandstate[OTHERBANDUNIT(wlc)];
			band->roam_trigger = rf_ie->roam_trigger;
			band->roam_delta = rf_ie->roam_delta;
		}
		/* RF roaming parameters applied */
		cfgi->droam_rf_roam = TRUE;
		WL_ASSOC(("wl%d: ROAM: CCX FAST, set roam trigger: %d set roam delta: %d\n",
			wlc->pub->unit, rf_ie->roam_trigger, rf_ie->roam_delta));
	}

	/* RF subelement exists for this AP */
	cfgi->droam_ap_list[idx].rf_ie = TRUE;
}

/* CCXv4 S51 directed roam */
static void
wlc_ccx_process_droam_iapp(wlc_ccx_info_t *ci, ccx_neighbor_rept_ie_t *rept_ie, uint len,
	wlc_bsscfg_t* cfg)
{
	/* process directed roam iapp frame */
	wlc_ccx_process_roam_iapp(ci, rept_ie, len, FALSE, cfg);
	wlc_ccx_roam((ccx_t *)ci, WLC_E_REASON_DIRECTED_ROAM, cfg);
}

/* ccx roam starting point
 * return : 0 for starting roam process successfully, non-zero otherwise.
 */
uint
wlc_ccx_roam(ccx_t* ci, uint roam_reason_code, wlc_bsscfg_t* cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	if (cfg->roam->off) {
		WL_ASSOC(("wlc_ccx_roam(reason 0x%x): roam disabled\n", roam_reason_code));
		return 1;
	}

	if (cfgi->ap_channel_num) {
		cfgi->fast_roam = TRUE;
	}

	if (!wlc_roam_scan(cfg, roam_reason_code, NULL, 0)) {
		WL_ASSOC(("wlc_ccx_roam(reason 0x%x): roam started\n", roam_reason_code));
		return 0;
	}
	else {
		WL_ASSOC(("wlc_ccx_roam(reason 0x%x): roam scan failed\n", roam_reason_code));
		return 1;
	}
}

static bool
wlc_ccx_rf_ie_good(wlc_ccx_info_t *ci, wlc_ccx_droam_t *droam_ap, wlc_bss_info_t *bi)
{
	int16 min_rssi;

	if (droam_ap->rf_ie == TRUE) {
		/* if rssi meets minimum value, report as good */
		min_rssi = wlc_ccx_get_min_rssi(ci, droam_ap->ap_tx_pwr,
			droam_ap->min_rssi, CHSPEC_CHANNEL(bi->chanspec));
		WL_ASSOC(("wl%d: %s: rssi %d min_rssi %d\n",
			ci->pub->unit, __FUNCTION__, bi->RSSI, min_rssi));

		return (bi->RSSI >= min_rssi);
	} else
		return TRUE; /* report pre-CCXv4 AP as good */
}

uint
wlc_ccx_prune(ccx_t *ccxh, wlc_bss_info_t *bi, wlc_bsscfg_t* cfg)
{
	uint i;
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ci, cfg);

	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	uint32 medium_time_needed = 0;
	struct scb *scb;
	uint32 qbss_load_aac = 0;

#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */


	if (WLC_CCX_IS_CURRENT_BSSID(cfg, &bi->BSSID)) {
		if (cfg->roam->reason == WLC_E_REASON_TSPEC_REJECTED) {
			WL_ASSOC(("wl%d: %s: ROAM: prune home AP %s for roam_reason 0x%x",
				wlc->pub->unit, __FUNCTION__,
				bcm_ether_ntoa(&bi->BSSID, eabuf), cfg->roam->reason));
			return WLC_E_PRUNE_HOME_AP;
		}
	}

	scb = wlc_scbfind(wlc, cfg, &cfg->BSSID);

	if (!scb)
		return 0;

	medium_time_needed = wlc_cac_medium_time_total(wlc->cac, scb);

	/* qbss_load_aac is in 32us unit */
	qbss_load_aac = bi->qbss_load_aac << 5;
	if (medium_time_needed &&
		(bi->ccx_version >= 4) &&
		(qbss_load_aac < medium_time_needed)) {
			WL_ASSOC(("wl%d: %s: CCX: prune BSSID %s for insufficient AAC\n",
				wlc->pub->unit, __FUNCTION__,
				bcm_ether_ntoa(&bi->BSSID, eabuf)));
			return WLC_E_PRUNE_QBSS_LOAD;
	}


	if (cfgi->fast_roam == FALSE) {
		return 0;
	}

	/* it is ccx fast_roam */
	/* prune previous AP */
	if (cfg->roam->reason != WLC_E_REASON_DEAUTH &&
	    cfg->roam->reason != WLC_E_REASON_DISASSOC &&
	    !bcmp((char *)&cfg->prev_BSSID, (char *)&bi->BSSID, ETHER_ADDR_LEN)) {
		WL_ASSOC(("wl%d: %s: CCX: prune previous AP BSSID %s\n",
			wlc->pub->unit, __FUNCTION__,
			bcm_ether_ntoa(&bi->BSSID, eabuf)));
		return WLC_E_PRUNE_CCXFAST_PREVAP;
	}

	/* do not prune if there is no roam AP list */
	if (!cfgi->droam_ap_num)
		return 0;

	/* search for matched AP in roam list */
	for (i = 0; i < cfgi->droam_ap_num; i++) {
		if (!bcmp((char*)&cfgi->droam_ap_list[i].bssid, (char*)&bi->BSSID,
			ETHER_ADDR_LEN)) {
			/* make sure rssi is good and aac is non-zero */
			if (wlc_ccx_rf_ie_good(ci, &cfgi->droam_ap_list[i], bi) == TRUE) {
				return 0;
			}
		}
	}

	WL_ASSOC(("wl%d: %s: CCX: FAST ROAM: prune BSSID %s in AP-assisted roaming\n",
		wlc->pub->unit, __FUNCTION__,
		bcm_ether_ntoa(&bi->BSSID, eabuf)));

	return WLC_E_PRUNE_CCXFAST_DROAM;
}

static int16
wlc_ccx_get_min_rssi(wlc_ccx_info_t *ci, int8 ap_txpwr, int8 ap_rssi, uint8 ap_channel)
{
	uint8 sta_txpwr_phy;
	int8 sta_txpwr;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	int16 min_rssi;

	sta_txpwr_phy = WLC_TXPWR_MAX;
	sta_txpwr_phy /= WLC_TXPWR_DB_FACTOR;
	sta_txpwr = MIN(sta_txpwr_phy, 0x7f); /* saturate it to max. positive */

	if (ap_txpwr > sta_txpwr) {
		min_rssi = (int16)(ap_rssi + (ap_txpwr - sta_txpwr));
	}
	else {
		min_rssi = (int16)(ap_rssi);
	}

	WL_ASSOC(("wl%d: %s: ROAM: min_rssi 0x%x\n",
		wlc->pub->unit, __FUNCTION__, min_rssi));
	BCM_REFERENCE(wlc);

	return min_rssi;
}

int
wlc_ccx_rm_begin(ccx_t *ccxh, wlc_rm_req_t* req)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	int type = req->type;
	int dur = req->dur;

	switch (type) {
	case WLC_RM_TYPE_BEACON_TABLE:
		WL_INFORM(("wl%d: wlc_ccx_rm_begin: Beacon Table request noted\n",
		           ci->pub->unit));
		break;
	case WLC_RM_TYPE_PASSIVE_SCAN:
	case WLC_RM_TYPE_ACTIVE_SCAN:
		WL_INFORM(("wl%d: wlc_ccx_rm_begin: starting Beacon measurement\n",
		           ci->pub->unit));
		wlc_ccx_rm_scan_begin(ci, type, dur);
		break;
	case WLC_RM_TYPE_FRAME:
		WL_INFORM(("wl%d: wlc_ccx_rm_begin: starting Frame measurement\n",
		           ci->pub->unit));
		wlc_ccx_rm_frm_begin(ci, dur);
		break;
	case WLC_RM_TYPE_PATHLOSS:
		WL_INFORM(("wl%d: wlc_ccx_rm_begin: starting PathLoss measurement\n",
		           ci->pub->unit));
		wlc_ccx_rm_pathloss_begin(ci, req);
		break;
	default:
		return -1;
	}

	return 0;
}

static void
wlc_ccx_rm_scan_begin(wlc_ccx_info_t *ci, int type, uint32 dur)
{
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	ccx_rm_t *rm = ci->rm_state->ccx;

	rm->scan_active = TRUE;
	rm->scan_dur = dur;
	wlc_bss_list_free(wlc, &rm->scan_results);
	bzero(rm->scan_results.ptrs, wlc->pub->tunables->maxbss * sizeof(wlc_bss_info_t*));

	/* hijack scan state so wlc_scan_parse() will capture the right info */
	wlc->scan->wlc_scan_cmn->bss_type = DOT11_BSSTYPE_ANY;
	bcopy(ether_bcast.octet, wlc->scan->bssid.octet, ETHER_ADDR_LEN);
	wlc->scan->wlc_scan_cmn->usage = SCAN_ENGINE_USAGE_RM;
	wlc->scan->state |= SCAN_STATE_SAVE_PRB;

	wlc_bss_list_free(wlc, wlc->scan_results);

	/* enable promisc reception of beacons and probe responses for scan */
	wlc->bcnmisc_scan = TRUE;
	wlc_mac_bcn_promisc(wlc);
	if (type == WLC_RM_TYPE_ACTIVE_SCAN)
		wlc_sendprobe(wlc, ci->rm_reqcfg, (const uint8 *)"", 0,
			&wlc->cfg->cur_etheraddr, &ether_bcast, &ether_bcast, 0, NULL, 0);
}

void
wlc_ccx_rm_scan_complete(ccx_t *ccxh)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	uint32 diff_h, diff_l;

	/* calc the actual duration of the scan */
	wlc_read_tsf(wlc, &diff_l, &diff_h);
	wlc_uint64_sub(&diff_h, &diff_l, rm_state->actual_start_h, rm_state->actual_start_l);
	rm_state->ccx->scan_dur = diff_l / DOT11_TU_TO_US;

	/* Restore promisc behavior for beacons and probes */
	wlc->bcnmisc_scan = FALSE;
	wlc_mac_bcn_promisc(wlc);

	/* copy scan results to rm_state for reporting */
	wlc_bss_list_xfer(wlc->scan_results, &rm_state->ccx->scan_results);

	/* clear the save probe flag in case there are late probe responses */
	wlc->scan->wlc_scan_cmn->usage = SCAN_ENGINE_USAGE_NORM;
	wlc->scan->state &= ~SCAN_STATE_SAVE_PRB;
	rm_state->ccx->scan_active = FALSE;

	if (rm_state->ccx->scan_results.count > 0) {
		WL_INFORM(("wl%d: radio measure scan complete, %d results\n",
			ci->pub->unit, rm_state->ccx->scan_results.count));
	} else {
		WL_INFORM(("wl%d: radio measure scan complete, no networks found\n",
			ci->pub->unit));
	}

	/* see if we are done with all measures in the current set */
	wlc_rm_meas_end(wlc);

	return;
}

static void
wlc_ccx_rm_frm_begin(wlc_ccx_info_t *ci, uint32 dur)
{
	int i;
	ccx_rm_t *rm = ci->rm_state->ccx;

	rm->frame_active = TRUE;
	rm->frame_dur = dur;
	rm->frm_elts = 0;
	rm->promisc_org = ci->pub->promisc;
	rm->promisc_off = FALSE;
	for (i = 0; i < WLC_NRMFRAMEHASH; i++)
		ASSERT(!rm->frmhash[i]);

	/* enable promisc reception */
	ci->pub->promisc = TRUE;
	wlc_mac_promisc(ci->wlc);
}

void
wlc_ccx_rm_frm_complete(ccx_t *ccxh)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	uint32 diff_h, diff_l;

	rm_state->ccx->frame_active = FALSE;

	/* calc the actual duration of the scan */
	wlc_read_tsf(ci->wlc, &diff_l, &diff_h);
	wlc_uint64_sub(&diff_h, &diff_l, rm_state->actual_start_h, rm_state->actual_start_l);
	rm_state->ccx->frame_dur = diff_l / DOT11_TU_TO_US;

	/* restore promisc. mode if no mode change req. during test */
	if (!rm_state->ccx->promisc_off)
		ci->pub->promisc = rm_state->ccx->promisc_org;
	wlc_mac_promisc(ci->wlc);

	/* see if we are done with all measures in the current set */
	wlc_rm_meas_end(ci->wlc);
}

static void
wlc_ccx_pathloss_dur_timer(void *arg)
{
	wlc_info_t *wlc =  (wlc_info_t *)arg;
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)wlc->ccx;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		return;
	}
	if (!wlc->pub->up)
		return;

	WL_ERROR(("wl%d: %s: Pathloss Duration timer expired, so ending the measurement\n",
		ci->wlc->pub->unit, __FUNCTION__));

	wlc_ccx_rm_pathloss_complete((ccx_t *)ci, TRUE);
}

static void
wlc_ccx_pathloss_burst_timer(void *arg)
{
	wlc_info_t *wlc =  (wlc_info_t *)arg;
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)wlc->ccx;
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	ccx_rm_t *rm = rm_state->ccx;
	wlc_rm_req_t *rm_req;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		return;
	}
	if (!wlc->pub->up)
		return;

	ASSERT(rm->pathloss_longrun);


	WL_INFORM(("wl%d: %s: Pathloss Burst timer expired so schduling the next burst \n",
		ci->wlc->pub->unit, __FUNCTION__));
	rm_req = (wlc_rm_req_t*)MALLOC(wlc->osh, sizeof(wlc_rm_req_t));
	if (rm_req == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			ci->wlc->pub->unit, __FUNCTION__, MALLOCED(ci->wlc->osh)));
		return;
	}

	bcopy(&rm->pathloss_data.req, rm_req, sizeof(wlc_rm_req_t));

	rm_state->broadcast = FALSE;
	rm_state->token = rm->pathloss_data.req.token;
	rm_state->req_count = 1;
	rm_state->req = rm_req;

	wlc_rm_start(wlc);
}

static void
wlc_ccx_rm_pathloss_begin(wlc_ccx_info_t *ci, wlc_rm_req_t* req)
{
	ccx_rm_t *ccx_rm = ci->rm_state->ccx;
	ccx_rm_pl_data_t *pathloss_data = &ccx_rm->pathloss_data;
	wlc_phy_t *pi = WLC_PI(ci->wlc);
	ppr_t *txpwr;
	int8 cur_qtxpwr;

	WL_INFORM(("starting the pathloss measurement\n"));

	if (req->flags & WLC_RM_FLAG_REFUSED) {
		WL_INFORM(("Refused flag is set..so coming out\n"));
		return;
	}

	/* save tx power control setting and turn off tx power control */
	ccx_rm->cur_txpowerctrl = wlc_phy_txpower_hw_ctrl_get(pi);
	phy_tpc_set_txpower_hw_ctrl(pi, FALSE);

	/* save tx power and set the desired tx power */
	wlc_phy_txpower_get(pi, &cur_qtxpwr, &ccx_rm->cur_txpoweroverride);
	ccx_rm->cur_txpower = cur_qtxpwr;
	if ((txpwr = ppr_create(ci->wlc->osh, PPR_CHSPEC_BW(ci->wlc->chanspec))) == NULL) {
		return;
	}

	wlc_channel_reg_limits(ci->wlc->cmi, ci->wlc->chanspec, txpwr);
	ppr_apply_max(txpwr, WLC_TXPWR_MAX);
	/* restore tx power */
	wlc_phy_txpower_set(pi, (int8) ccx_rm->cur_txpower,
		ccx_rm->cur_txpoweroverride, txpwr);
	ppr_delete(ci->wlc->osh, txpwr);

	WL_INFORM(("Pathloss measurement: staring the burst duration timer\n"));
	wl_add_timer(ci->wlc->wl, ccx_rm->plm_dur_timer, pathloss_data->duration, FALSE);

	/* begin the pathloss measurement */
	if (ccx_rm->pathloss_longrun == TRUE) {

	} else if ((pathloss_data->nbursts == 0) || (pathloss_data->nbursts > 1)) {
		WL_INFORM(("Pathloss measurement for more than one burst %d\n",
			pathloss_data->burst_interval));
		ccx_rm->pathloss_longrun = TRUE;
		wl_add_timer(ci->wlc->wl, ccx_rm->plm_burst_timer,
			pathloss_data->burst_interval, TRUE);
	}

	ccx_rm->pathloss_active = TRUE;

	wlc_ccx_rm_pathloss(ci);
}

static void
wlc_ccx_rm_pathloss(wlc_ccx_info_t *ci)
{
	ccx_rm_t *ccx_rm = ci->rm_state->ccx;
	ccx_rm_pl_data_t *pathloss_data = &ccx_rm->pathloss_data;

	/* send the pathloss measurement frame */
	wlc_ccx_rm_send_pathlossmeasure_frame(ci);

	/* update the local stats */
	pathloss_data->cur_chanidx++;
	if (pathloss_data->cur_chanidx == pathloss_data->nchannels) {
		pathloss_data->cur_chanidx = 0;
		pathloss_data->seq_number++;
		pathloss_data->cur_burstlen++;
		if (pathloss_data->cur_burstlen == pathloss_data->burst_len) {
			pathloss_data->cur_burst++;
			pathloss_data->cur_burstlen = 0;
		}
	}
}

void
wlc_ccx_rm_pathloss_complete(ccx_t *ccxh, bool force_done)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	ccx_rm_t *ccx_rm = ci->rm_state->ccx;
	ccx_rm_pl_data_t *pathloss_data = &ccx_rm->pathloss_data;
	wlc_phy_t *pi  = WLC_PI(ci->wlc);
	ppr_t *txpwr;

	WL_INFORM(("stopping the Pathloss Measurment burst duration timer\n"));
	wl_del_timer(ci->wlc->wl, ccx_rm->plm_dur_timer);

	/* restore tx power control setting */
	phy_tpc_set_txpower_hw_ctrl(pi, ccx_rm->cur_txpowerctrl);

	/* restore tx power and override */
	if ((txpwr = ppr_create(ci->wlc->osh, PPR_CHSPEC_BW(ci->wlc->chanspec))) == NULL) {
		return;
	}

	wlc_channel_reg_limits(ci->wlc->cmi, ci->wlc->chanspec, txpwr);
	ppr_apply_max(txpwr, WLC_TXPWR_MAX);
	/* restore tx power */
	wlc_phy_txpower_set(pi, (int8)ccx_rm->cur_txpower,
		ccx_rm->cur_txpoweroverride, txpwr);
	ppr_delete(ci->wlc->osh, txpwr);

	/* check if we are already done */
	if (ccx_rm->pathloss_active == FALSE)
		return;

	ccx_rm->pathloss_active = FALSE;

	/* done with all the bursts */
	if ((pathloss_data->nbursts && (pathloss_data->cur_burst == pathloss_data->nbursts)) ||
		force_done) {
		WL_INFORM(("*****Pathloss Measurments complete****\n"));
		ccx_rm->pathloss_longrun = FALSE;
		wl_del_timer(ci->wlc->wl, ccx_rm->plm_burst_timer);
	}
	/* current burst done */
	else if ((!pathloss_data->nbursts) && pathloss_data->cur_burst) {
		WL_INFORM(("Pathloss Measurments complete for current burst\n"));
		pathloss_data->cur_burst = 0;
	}

	/* measurement end */
	wlc_rm_meas_end(ci->wlc);
}

static void
wlc_ccx_rm_pathloss_pktcallback(wlc_info_t *wlc, uint txstatus, void *arg)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)arg;
	ccx_rm_t *ccx_rm = ci->rm_state->ccx;
	ccx_rm_pl_data_t *pathloss_data = &ccx_rm->pathloss_data;
	bool force_done = FALSE;

	WL_INFORM(("pathloss callback: pktstatus is 0x%x\n", txstatus));
	if (ccx_rm->pathloss_active == FALSE)
		return;

	if (txstatus != (TX_STATUS_SUPR_FLUSH << TX_STATUS_SUPR_SHIFT)) {
		if ((pathloss_data->cur_burst) && (!pathloss_data->cur_burstlen))
			goto done;
		WL_INFORM(("Pathloss Measurments not complete for current burst yet\n"));
		wlc_ccx_rm_pathloss(ci);
		return;
	}
	else
		force_done = TRUE;
done:
	WL_INFORM(("Pathloss Measurments complete for current burst\n"));
	/* done with pathloss measurements */
	wlc_ccx_rm_pathloss_complete((ccx_t *)ci, force_done);
}

static void
wlc_ccx_rm_send_pathlossmeasure_frame(wlc_ccx_info_t *ci)
{
	uint body_len;
	uint8 *pbody;
	struct ccx_rm_pathlossmeas_frame *frame;
	void *p;
	ccx_rm_pl_data_t *pathloss_data;
	ccx_iapp_hdr_t *iapphdr;
	struct dot11_header *h;

	pathloss_data = &ci->rm_state->ccx->pathloss_data;

	body_len = DOT11_LLC_SNAP_HDR_LEN + sizeof(ccx_iapp_hdr_t) +
		sizeof(struct ccx_rm_pathlossmeas_frame) - 1;

	p =  PKTGET(ci->pub->osh, (body_len + TXOFF), TRUE);
	if (p == NULL) {
		WL_ERROR(("Couldn't allocate data pakcet for the pathloss measurement\n"));
		return;
	}

	/* leave the space for phyheader and d11hdr */
	PKTPULL(ci->pub->osh, p, (D11_TXH_LEN + D11_PHY_HDR_LEN));
	PKTSETLEN(ci->pub->osh, p, (DOT11_A4_HDR_LEN + body_len));

	pbody = (uint8*)PKTDATA(ci->pub->osh, p);
	h = (struct dot11_header *)pbody;

	/* 802.11 hdr */
	h->fc = (FC_TODS|FC_FROMDS);
	h->fc |= (FC_TYPE_DATA << FC_TYPE_SHIFT);
	h->fc = htol16(h->fc);
	h->durid = 0;
	bcopy(ci->rm_reqcfg->cur_etheraddr.octet, (char *)&h->a2, ETHER_ADDR_LEN);

	bcopy(&pathloss_data->da, (char *)&h->a1, ETHER_ADDR_LEN);
	bcopy(&pathloss_data->da, (char *)&h->a3, ETHER_ADDR_LEN);
	pbody += DOT11_A3_HDR_LEN;

	/* fill the pathloss frame data */
	bcopy(CISCO_AIRONET_SNAP, pbody, DOT11_LLC_SNAP_HDR_LEN);

	iapphdr = (ccx_iapp_hdr_t *)(pbody + DOT11_LLC_SNAP_HDR_LEN);
	iapphdr->id_len = htol16(body_len - DOT11_LLC_SNAP_HDR_LEN);
	iapphdr->type = CCX_IAPP_TYPE_RM;
	iapphdr->subtype = CCX_RM_IAPP_SUBTYPE;
	bcopy(pathloss_data->da.octet, iapphdr->da.octet,  ETHER_ADDR_LEN);
	bcopy(ci->rm_reqcfg->cur_etheraddr.octet, iapphdr->sa.octet, ETHER_ADDR_LEN);

	frame = (struct ccx_rm_pathlossmeas_frame *)(&iapphdr->data);
	frame->seq = htol16(((pathloss_data->seq_number) & CCX_RM_PATHLOSS_SEQ_MASK));
	/* fill in the current txpower */
	frame->txpower = pathloss_data->req_txpower;
	frame->txchannel = pathloss_data->channels[pathloss_data->cur_chanidx];

	wlc_pcb_fn_register(ci->wlc->pcb, wlc_ccx_rm_pathloss_pktcallback, (void *)ci, p);
	PKTSETPRIO(p, MAXPRIO);
	/* Data fifo is suspended, so use ctrl fifo .. this is a data frame .. */
	wlc_sendctl(ci->wlc, p, ci->wlc->active_queue, WLC_BCMCSCB_GET(ci->wlc, ci->rm_reqcfg),
	            TX_CTL_FIFO, 0, FALSE);
}

static void
wlc_ccx_rm_frm_list_free(wlc_ccx_info_t *ci)
{
	int i;
	ccx_rm_t *rm = ci->rm_state->ccx;
	wlc_ccx_rm_frm_elt_t *frm_elt, *nextp;

	/* free allocate memory */
	for (i = 0; i < WLC_NRMFRAMEHASH; i++) {
		for (frm_elt = rm->frmhash[i]; frm_elt; frm_elt = nextp) {
			nextp = frm_elt->next;
			MFREE(ci->pub->osh, frm_elt, sizeof(wlc_ccx_rm_frm_elt_t));
		}
		rm->frmhash[i] = NULL;
	}
	ci->rm_reqcfg = NULL;
}

void
wlc_ccx_rm_frm(ccx_t *ccxh, wlc_d11rxhdr_t *wrxh, struct dot11_header *h)
{
	struct ether_addr *ta, *bssid;
	uint16 dir, fc = ltoh16(h->fc);
	int indx;
	wlc_ccx_rm_frm_elt_t *elt;
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	ccx_rm_t *rm = ci->rm_state->ccx;

	ASSERT(FC_TYPE(fc) == FC_TYPE_DATA);

	dir = fc & (FC_TODS | FC_FROMDS);
	if (dir == 0) {
		ta = &h->a2;
		bssid = &h->a3;
	} else if (dir == FC_TODS) {
		ta = &h->a2;
		bssid = &h->a1;
	} else if (dir == FC_FROMDS) {
		ta = &h->a3;
		bssid = &h->a2;
	} else {
		ta = bssid = &h->a2;
	}

	indx = RMFRMHASHINDEX((uchar *)ta);
	/* search for the frame element which corresponds to the tx station ea */
	for (elt = rm->frmhash[indx]; elt; elt = elt->next)
		if (!bcmp((char*)ta, (char*)&elt->ta, ETHER_ADDR_LEN) &&
			!bcmp((char*)bssid, (char*)&elt->bssid, ETHER_ADDR_LEN))
			break;

	if (!elt) {
		/* allocate memory for new element */
		elt = (wlc_ccx_rm_frm_elt_t *)MALLOCZ(ci->pub->osh, sizeof(wlc_ccx_rm_frm_elt_t));
		if (!elt) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				ci->pub->unit, __FUNCTION__, MALLOCED(ci->pub->osh)));
			return;
		}

		/* copy transmit address and bssid */
		bcopy(ta, &elt->ta, ETHER_ADDR_LEN);
		bcopy(bssid, &elt->bssid, ETHER_ADDR_LEN);

		/* link new element in link list */
		elt->next = rm->frmhash[indx];
		rm->frmhash[indx] = elt;
		rm->frm_elts++;
	} else if (elt->frames == MAXNBVAL(sizeof(elt->frames)))
		/* one byte length.  stop count if reach to max limit */
		return;

	/* update report fields */
	elt->rssi_sum += wlc_lq_recv_rssi_compute(ci->wlc, wrxh);
	elt->frames++;
}

static void
wlc_ccx_rm_recv(wlc_ccx_info_t *ci, struct ether_addr* da,
	struct ether_addr* sa, uint16 rx_time,
	ccx_rm_req_t* req_pkt, int req_len, wlc_bsscfg_t* cfg)
{
	ccx_rm_req_ie_t* req_ie;
	int req_block_len;

#ifdef BCMDBG
	if (WL_INFORM_ON())
		wlc_ccx_rm_req_dump(ci, da, sa, req_pkt, req_len, cfg);
#endif /* BCMDBG */

	/* make sure this is a valid CCX RM Request packet */
	if (req_len < CCX_RM_REQ_LEN) {
		WL_ERROR(("wl%d: %s: radio request packet too short, "
			"len = %d expected at least %d", ci->pub->unit, __FUNCTION__,
			req_len, CCX_RM_REQ_LEN));
		return;
	}
	req_block_len = req_len - CCX_RM_REQ_LEN;
	req_ie = (ccx_rm_req_ie_t*)req_pkt->data;

	/* return if no recongnized RM requests */
	if (wlc_ccx_rm_ie_count(req_ie, req_block_len) == 0) {
		WL_INFORM(("no IE's found in rm req...\n"));
		return;
	}

	/* otherwise we have a new RM request ... */
	wlc_rm_abort(ci->wlc);

	if (wlc_ccx_rm_state_init(ci, ETHER_ISMULTI(da), rx_time, req_pkt, req_len, cfg)) {
		WL_ERROR(("wl%d: %s: failed to initialize radio "
			  "measurement state, dropping request\n",
			  ci->pub->unit, __FUNCTION__));
	}
	ci->rm_reqcfg = cfg;
	wlc_rm_start(ci->wlc);
}

int
wlc_ccx_rm_validate(ccx_t *ccxh, chanspec_t cur_chanspec, wlc_rm_req_t *req)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	ccx_rm_pl_data_t *pathloss_data = &rm_state->ccx->pathloss_data;
	DBGONLY(char chanbuf[CHANSPEC_STR_LEN]; )

	/* reject rm request if AP active */
	if (AP_ACTIVE(wlc)) {
		WL_INFORM(("wl%d: %s: reject rm request since AP is active\n",
			wlc->pub->unit, __FUNCTION__));
		req->flags |= WLC_RM_FLAG_REFUSED;
		return -1;
	}

	/* check for off-channel measurements over the limit */
	if (req->chanspec && req->chanspec != cur_chanspec && wlc->pub->associated &&
	    req->dur > (int)ci->rm_limit) {
		WL_INFORM(("wl%d: wlc_rm_validate: refusing off channel "
			   "measurement with dur %d greater than "
			   "limit %d, RM type %d token %d channel %s\n",
			   wlc->pub->unit, req->dur, ci->rm_limit,
			   req->type, req->token, wf_chspec_ntoa_ex(req->chanspec, chanbuf)));
		req->flags |= WLC_RM_FLAG_REFUSED;
		return -1;
	}
	if (req->type == WLC_RM_TYPE_PATHLOSS) {
		if ((!pathloss_data) || (pathloss_data->nchannels != 1)) {
			WL_INFORM(("wl%d: %s: more than one channel specified for"
			   "offchannel pathloss measurements \n",
			   wlc->pub->unit, __FUNCTION__));
			req->flags |= WLC_RM_FLAG_REFUSED;
			return -1;
		}
	}
	return 0;
}

static int
wlc_ccx_rm_state_init(wlc_ccx_info_t *ci,
	bool bcast, uint16 rx_time,
	ccx_rm_req_t* req_pkt, int req_len, wlc_bsscfg_t* cfg)
{
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	uint32 tsf_h, tsf_l;
	uint32 req_time_h;
	uint32 req_time_l;
	uint32 start_time_h;
	uint32 start_time_l;
	uint32 offset_us;
	uint32 tbtt_offset;
	uint32 total_offset;
	uint32 bi;
	int rm_req_size;
	int rm_req_count;
	wlc_rm_req_t* rm_req;
	ccx_rm_req_ie_t* req_ie;
	int req_block_len;

	req_block_len = req_len - CCX_RM_REQ_LEN;
	req_ie = (ccx_rm_req_ie_t*)req_pkt->data;

	rm_req_count = wlc_ccx_rm_ie_count(req_ie, req_block_len);
	rm_req_size = rm_req_count * sizeof(wlc_rm_req_t);
	rm_req = (wlc_rm_req_t*)MALLOCZ(ci->pub->osh, rm_req_size);
	if (rm_req == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			ci->pub->unit, __FUNCTION__, MALLOCED(ci->pub->osh)));
		return BCME_NORESOURCE;
	}

	rm_state->report_class = WLC_RM_CLASS_CCX;
	rm_state->broadcast = bcast;
	rm_state->token = ltoh16(req_pkt->token);
	rm_state->req_count = rm_req_count;
	rm_state->req = rm_req;

	/* clear out the pathloss data */
	rm_state->ccx->pathloss_active = FALSE;
	rm_state->ccx->pathloss_longrun = FALSE;
	wl_del_timer(ci->wlc->wl, ci->rm_state->ccx->plm_burst_timer);
	wl_del_timer(ci->wlc->wl, ci->rm_state->ccx->plm_dur_timer);


	bzero(&rm_state->ccx->pathloss_data, sizeof(ccx_rm_pl_data_t));

	/* Fill out the request blocks */
	wlc_ccx_rm_parse_requests(ci, req_ie, req_block_len, rm_req, rm_req_count, cfg);

	/* Compute the start time of the measurements,
	 * Req frame has a delay in TBTT times, and an offset in TUs
	 * from the delay time.
	 */
	wlc_read_tsf(ci->wlc, &tsf_l, &tsf_h);

	/* recover the tsf_l 32 bit value for the req frame rx time */
	req_time_l = (tsf_l & 0xffff0000) + rx_time;
	if (rx_time > (tsf_l & 0xffff)) {
		/* a greater rx_time indicates the low 16 bits of
		 * tsf_l wrapped, so decrement the high 16 bits.
		 */
		req_time_l = req_time_l - 0x10000;
	}

	/* recover the tsf_h 32 bit value for the req frame rx time */
	req_time_h = tsf_h;
	if (req_time_l > tsf_l) {
		/* a greater req_time_l indicates tsf_l wrapped
		 * since the req_time_l, so decrement the high value.
		 */
		req_time_h--;
	}

	offset_us = req_pkt->offset * DOT11_TU_TO_US;

	/* we now have the recovered TSF time of the request frame,
	 * compute the measurement start time based on the
	 * requested delay (TBTTs) and offset (TUs) from the request frame time.
	 */
	start_time_l = req_time_l;
	start_time_h = req_time_h;

	if (req_pkt->delay == 0) {
		/* no delay, use the request frame time plus offset */
		wlc_uint64_add(&start_time_h, &start_time_l, 0, offset_us);
	} else {
		/* compute the TSF time of the TBTT 'delay' Beacon intervals
		 * from when the request packet was sent, plus 'offset'
		 */

		/* first find the offset to the TBTT of the beacon interval
		 * that contained the request frame (offset back in time).
		 */
		bi = cfg->current_bss->beacon_period;
		tbtt_offset = wlc_calc_tbtt_offset(bi, req_time_h, req_time_l);

		/* calculate the offset to the TBTT 'delay' beacon
		 * intervals from the reqest frame time.
		 */
		total_offset = req_pkt->delay * (bi * DOT11_TU_TO_US) - tbtt_offset;
		/* add in the request offset value */
		total_offset += offset_us;

		/* calculate the full TSF timer for the target measurement */
		wlc_uint64_add(&start_time_h, &start_time_l, 0, total_offset);
	}

	if (wlc_uint64_lt(start_time_h, start_time_l, tsf_h, tsf_l)) {
		/* we have already passed the offset time specified
		 * in the frame, so just use the current time
		 */
		start_time_h = tsf_h;
		start_time_l = tsf_l;
	}

	/* The CCX measurement request frame has one start time for the set of
	 * measurements, but the driver state allows for a start time for each
	 * measurement. Set the CCX measurement request start time into the
	 * first of the driver measurement requests, and leave the following
	 * start times as zero to indicate they happen as soon as possible.
	 */
	rm_req[0].tsf_h = start_time_h;
	rm_req[0].tsf_l = start_time_l;

	return 0;
}

void
wlc_ccx_rm_free(ccx_t *ccxh)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_rm_req_state_t* rm_state = ci->rm_state;

	if (rm_state->report_class == WLC_RM_CLASS_CCX) {
		wlc_ccx_rm_rept_t *report;
		wlc_ccx_rm_rept_t *next;
		report = ci->rm_reports;
		ci->rm_reports = NULL;

		while (report != NULL) {
			next = report->next;
			MFREE(ci->pub->osh, report, sizeof(wlc_ccx_rm_rept_t));
			report = next;
		}
	}

	rm_state->ccx->scan_active = FALSE;
	wlc_bss_list_free(ci->wlc, &rm_state->ccx->scan_results);
	rm_state->ccx->frame_active = FALSE;
	wlc_ccx_rm_frm_list_free(ci);
}

static uint8
wlc_ccx_rm_get_mbssid_mask(wlc_bss_info_t *bi)
{
	ccx_radio_mgmt_t *rm_cap_ie;
	uint8 rm_cap_ver = 1;
	uint8 mbssid_mask = 0;
	int bcn_len;
	uint8 *bcn_ie = NULL;

	ASSERT(bi != NULL);
	bcn_len = bi->bcn_prb_len;
	if (bi->bcn_prb && bcn_len > DOT11_BCN_PRB_LEN) {
		bcn_ie = (uchar*)bi->bcn_prb + DOT11_BCN_PRB_LEN;
		bcn_len -= DOT11_BCN_PRB_LEN;
	}

	if ((rm_cap_ie = (ccx_radio_mgmt_t*)bcm_find_vendor_ie(bcn_ie, bcn_len, CISCO_AIRONET_OUI,
		&rm_cap_ver, 1)) &&
		rm_cap_ie->len >= DOT11_OUI_LEN + 3)
		mbssid_mask = (uint8)((ltoh16(rm_cap_ie->state) & CCX_RM_STATE_MBSSID_MASK) >>
			CCX_RM_STATE_MBSSID_SHIFT);

	return mbssid_mask;
}

/* CCXv4: only a single BSSID beacon in MBSSID AP should be reported. sec 59.3.2 */
static void
wlc_ccx_rm_mbssid_report(wlc_bss_info_t **results, uint8 *report, uint num)
{
	uint i, j;
	struct ether_addr mac_i, mac_j;
	uint8 mbssid_mask;
	const uint8 mask[] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f};

	/* find out MBSSID's beacons and report for one BSSID only */
	for (i = 0; i < num; i++) {
		if (!report[i] || !(mbssid_mask = wlc_ccx_rm_get_mbssid_mask(results[i])))
			continue;
		/* MBSSID's found */
		WL_INFORM(("wl: wlc_ccx_rm_mbssid_report: mbssid to report"
			" %02x:%02x:%02x:%02x:%02x:%02x\n",
			results[i]->BSSID.octet[0], results[i]->BSSID.octet[1],
			results[i]->BSSID.octet[2],
			results[i]->BSSID.octet[3], results[i]->BSSID.octet[4],
			results[i]->BSSID.octet[5]));
		mac_i = results[i]->BSSID;
		mac_i.octet[ETHER_ADDR_LEN - 1] &= ~mask[mbssid_mask];
		for (j = i + 1; j < num; j++) {
			mac_j = results[j]->BSSID;
			mac_j.octet[ETHER_ADDR_LEN - 1] &= ~mask[mbssid_mask];
			if (!bcmp(&mac_i, &mac_j, ETHER_ADDR_LEN)) {
				/* do not report */
				report[j] = 0;
				WL_INFORM(("wl: wlc_ccx_rm_mbssid_report: mbssid to not report"
					" %02x:%02x:%02x:%02x:%02x:%02x\n",
					results[j]->BSSID.octet[0], results[j]->BSSID.octet[1],
					results[j]->BSSID.octet[2],
					results[j]->BSSID.octet[3], results[j]->BSSID.octet[4],
					results[j]->BSSID.octet[5]));
			}
		}
	}
}

static void
wlc_ccx_rm_parse_requests(wlc_ccx_info_t *ci, ccx_rm_req_ie_t* ie, int len,
	wlc_rm_req_t* req, int count, wlc_bsscfg_t* cfg)
{
	int idx = 0;
	int8 type;
	uint32 parallel_flag = 0;
	ccx_rm_pathlossreq_t *pathloss_req;
	ccx_rm_pl_data_t *pathloss_data = &ci->rm_state->ccx->pathloss_data;

	/* convert each CCX RM IE into a wlc_rm_req */
	for (; ie != NULL && idx < count; ie = wlc_ccx_rm_next_ie(ie, &len)) {
		/* skip IE if we do not recongnized the request,
		 * or it was !enable (not a autonomous report req)
		 */
		if (ltoh16(ie->id) != CCX_RM_ID_REQUEST ||
		    ltoh16(ie->len) < CCX_RM_REQ_IE_FIXED_LEN ||
		    0 != (ie->mode & CCX_RM_MODE_ENABLE))
			continue;

		/* in case we skip measurement requests, keep track of the
		 * parallel bit flag separately. Clear here at the beginning of
		 * every set of measurements and set below after the first
		 * measurement req we actually send down.
		 */
		if (!(ie->mode & CCX_RM_MODE_PARALLEL)) {
			parallel_flag = 0;
		}

		switch (ie->type) {
		case CCX_RM_TYPE_LOAD:
			type = WLC_RM_TYPE_CCA;
			break;
		case CCX_RM_TYPE_NOISE:
			type = WLC_RM_TYPE_RPI;
			break;
		case CCX_RM_TYPE_FRAME:
			type = WLC_RM_TYPE_FRAME;
			break;
		case CCX_RM_TYPE_BEACON:
			if (ie->param == CCX_RM_BEACON_PASSIVE_SCAN) {
				type = WLC_RM_TYPE_PASSIVE_SCAN;
			} else if (ie->param == CCX_RM_BEACON_ACTIVE_SCAN) {
				type = WLC_RM_TYPE_ACTIVE_SCAN;
			} else if (ie->param == CCX_RM_BEACON_TABLE) {
				/* no beacon table support for now, do not reply */
				/* type = WLC_RM_TYPE_BEACON_TABLE; */
				continue;
			} else {
				/* unknown beacon request, do not reply */
				continue;
			}
			break;
		case CCXv4_RM_TYPE_PATHLOSS:
		case CCX_RM_TYPE_PATHLOSS:
			if ((cfg->current_bss->ccx_version <= 4 &&
				ie->type == CCXv4_RM_TYPE_PATHLOSS) ||
				(cfg->current_bss->ccx_version > 4 &&
				ie->type == CCX_RM_TYPE_PATHLOSS))
				type = WLC_RM_TYPE_PATHLOSS;
			else
				continue;
			break;
		default:
			/* unknown measurement type, do not reply */
			continue;
			break;
		}

		req[idx].type = type;
		req[idx].flags |= parallel_flag;
		req[idx].token = ltoh16(ie->token);
		req[idx].chanspec = CH20MHZ_CHSPEC(ie->channel);
		req[idx].dur = ltoh16(ie->duration);
		/* special handling for beacon table */
		if (type == WLC_RM_TYPE_BEACON_TABLE) {
			req[idx].chanspec = 0;
			req[idx].dur = 0;
		}
		else if (type == WLC_RM_TYPE_PATHLOSS) {
			pathloss_req = (ccx_rm_pathlossreq_t *)&(ie->channel);

			bzero(pathloss_data, sizeof(ccx_rm_pl_data_t));
			pathloss_data->nbursts = ltoh16(pathloss_req->nbursts);
			pathloss_data->burst_interval = ltoh16(pathloss_req->burstinterval);
			pathloss_data->burst_interval *= 1000;
			pathloss_data->burst_len = pathloss_req->burstlen;
			pathloss_data->duration = ltoh16(pathloss_req->duration);

			pathloss_data->req_txpower = pathloss_req->txpower;

			pathloss_data->nchannels = pathloss_req->nchannels;
			bcopy((char *)&pathloss_req->addr, (char *)&pathloss_data->da,
				sizeof(struct ether_addr));
			if (pathloss_data->nchannels > CCX_RMPATHLOSS_CHANNELMAX)
				pathloss_data->nchannels = CCX_RMPATHLOSS_CHANNELMAX;

			bcopy(&pathloss_req->channel[0], pathloss_data->channels,
				pathloss_data->nchannels);

			req[idx].chanspec = CH20MHZ_CHSPEC(pathloss_req->channel[0]);
			req[idx].dur = 0;

			bcopy(&req[idx], &pathloss_data->req, sizeof(wlc_rm_req_t));
		}

		/* set the parallel bit now that we output a request item so
		 * that other reqs in the same parallel set will have the bit
		 * set. The flag will be cleared above when a new parallel set
		 * starts.
		 */
		parallel_flag = WLC_RM_FLAG_PARALLEL;

		idx++;
	}
}

#ifdef BCMDBG
static void
wlc_ccx_rm_req_dump(wlc_ccx_info_t *ci, struct ether_addr* da, struct ether_addr* sa,
	ccx_rm_req_t* req_pkt, int req_len, wlc_bsscfg_t* cfg)
{
	ccx_rm_req_ie_t *ie;
	int buflen;
	char buf[2*ETHER_ADDR_LEN + 1];

	printf("CCX Radio Measure Request len=%d\n", req_len);
	bcm_format_hex(buf, da->octet, ETHER_ADDR_LEN);
	printf("DA: %s\n", buf);
	bcm_format_hex(buf, sa->octet, ETHER_ADDR_LEN);
	printf("SA: %s\n", buf);
	if (req_len < CCX_RM_REQ_LEN) {
		bcm_format_hex(buf, (int8*)req_pkt, req_len);
		printf("RM packet too short\n");
		printf("RM contents: %s\n", buf);
		return;
	}

	printf("RM token:  0x%04x\n", ltoh16(req_pkt->token));
	printf("RM delay:  %d TBTT\n", req_pkt->delay);
	printf("RM offset: %d TU\n", req_pkt->offset);

	buflen = req_len - CCX_RM_REQ_LEN;
	ie = (ccx_rm_req_ie_t*)req_pkt->data;

	while (buflen > 0) {
		wlc_ccx_rm_req_ie_dump(ci, ie, buflen, cfg);

		/* advance to the next IE */
		if (buflen > CCX_RM_IE_HDR_LEN) {
			buflen -= CCX_RM_IE_HDR_LEN + ltoh16(ie->len);
			ie = (ccx_rm_req_ie_t*)((int8*)ie + CCX_RM_IE_HDR_LEN + ltoh16(ie->len));
		} else {
			break;
		}
	}

	return;
}

static void
wlc_ccx_rm_req_ie_dump(wlc_ccx_info_t *ci, ccx_rm_req_ie_t *ie, int buflen, wlc_bsscfg_t *cfg)
{
	char buf[2*ETHER_ADDR_LEN + 1];
	const char *type_name;
	int8 *variable_buf;
	int variable_len;
	bool simple_req;

	if (buflen == 0)
		return;

	if (buflen < CCX_RM_IE_HDR_LEN) {
		bcm_format_hex(buf, (int8*)ie, buflen);
		printf("RM IE partial elt at end of packet, %d bytes: %s\n", buflen, buf);
		return;
	}

	if (ltoh16(ie->id) == CCX_RM_ID_REQUEST) {
		printf("RM IE ID %d (RM Request), len %d\n", ltoh16(ie->id), ltoh16(ie->len));
	} else {
		printf("RM IE ID %d (Unknown), len %d\n", ltoh16(ie->id), ltoh16(ie->len));
	}

	if (buflen < CCX_RM_IE_HDR_LEN + ltoh16(ie->len)) {
		bcm_format_hex(buf, (int8*)ie + CCX_RM_IE_HDR_LEN, buflen - CCX_RM_IE_HDR_LEN);
		printf("RM IE partial elt at end of packet, %d bytes in data: %s\n",
		       buflen - CCX_RM_IE_HDR_LEN, buf);
		return;
	}

	/* check for unknown Request IE ID */
	if (ltoh16(ie->id) != CCX_RM_ID_REQUEST) {
		bcm_format_hex(buf, (int8*)ie + CCX_RM_IE_HDR_LEN, ltoh16(ie->len));
		printf("RM IE contents: %s\n", buf);
		return;
	}

	/* we have a Radio Measurement request IE */

	/* check for minimum fixed size */
	if (ltoh16(ie->len) < CCX_RM_REQ_IE_FIXED_LEN - CCX_RM_IE_HDR_LEN) {
		bcm_format_hex(buf, (int8*)ie + CCX_RM_IE_HDR_LEN, ltoh16(ie->len));
		printf("RM IE too short, expected at least %d bytes of IE data\n",
		       CCX_RM_REQ_IE_FIXED_LEN - CCX_RM_IE_HDR_LEN);
		printf("RM IE contents: %s\n", buf);
		return;
	}

	printf("RM IE token: 0x%04x\n", ltoh16(ie->token));

	if (ie->mode & CCX_RM_MODE_ENABLE) {
		printf("RM IE mode:  0x%02x (autonomous reqs)\n", ie->mode);
		simple_req = FALSE;
	} else {
		printf("RM IE mode:  0x%02x\n", ie->mode);
		simple_req = TRUE;
	}

	switch (ie->type) {
	case CCX_RM_TYPE_LOAD:
		type_name = "(Channel Load)";
		break;
	case CCX_RM_TYPE_NOISE:
		type_name = "(Channel Noise)";
		break;
	case CCX_RM_TYPE_FRAME:
		type_name = "(Frame)";
		break;
	case CCX_RM_TYPE_BEACON:
		type_name = "(Beacon)";
		break;
	case CCXv4_RM_TYPE_PATHLOSS:
	case CCX_RM_TYPE_PATHLOSS:
		if ((cfg->current_bss->ccx_version <= 4 &&
			ie->type == CCXv4_RM_TYPE_PATHLOSS) ||
			(cfg->current_bss->ccx_version > 4 &&
			ie->type == CCX_RM_TYPE_PATHLOSS))
			type_name = "(Pathloss)";
		else
			type_name = "(unknown)";
		break;
	default:
		type_name = "(unknown)";
		simple_req = FALSE;
		break;
	}
	printf("RM IE type:  0x%02x %s\n", ie->type, type_name);


	variable_buf = (int8*)ie + CCX_RM_REQ_IE_FIXED_LEN;
	variable_len = (ltoh16(ie->len) + CCX_RM_IE_HDR_LEN) - CCX_RM_REQ_IE_FIXED_LEN;

	if ((cfg->current_bss->ccx_version <= 4 &&
		ie->type == CCXv4_RM_TYPE_PATHLOSS) ||
		(cfg->current_bss->ccx_version > 4 &&
		ie->type == CCX_RM_TYPE_PATHLOSS)) {
		return;
	}

	if (!simple_req) {
		bcm_format_hex(buf, variable_buf, variable_len);
		printf("RM IE unexpected extra measure info: %s\n", buf);
		return;
	}

	if (variable_len < 4) {
		bcm_format_hex(buf, variable_buf, variable_len);
		printf("RM IE measure info too short, expected at least %d bytes of measure data:"
			" %s\n", 4, buf);
		return;
	}

	printf("RM IE channel:  %4d\n", ie->channel);
	printf("RM IE param:    0x%02x\n", ie->param);
	printf("RM IE duration: %4d\n", ltoh16(ie->duration));

	if (variable_len > 4) {
		bcm_format_hex(buf, variable_buf + 4, variable_len - 4);
		printf("RM IE unexpected extra measure info: %s\n", buf);
	}

	return;
}
#endif /* BCMDBG */


static int
wlc_ccx_rm_ie_count(ccx_rm_req_ie_t* ie, int len)
{
	int count = 0;

	/* make sure this is a valid CCX RM IE */
	if (len < CCX_RM_IE_HDR_LEN ||
	    len < CCX_RM_IE_HDR_LEN + ltoh16(ie->len)) {
		ie = NULL;
	}

	/* walk the req IEs counting valid RM Requests,
	 * skipping unknown IEs or ones that just have autonomous report flags
	 */
	while (ie) {
		if (ltoh16(ie->id) == CCX_RM_ID_REQUEST &&
		    ltoh16(ie->len) >= CCX_RM_REQ_IE_FIXED_LEN &&
		    0 == (ie->mode & CCX_RM_MODE_ENABLE)) {
			/* found a measurement request */
			count++;
		}

		ie = wlc_ccx_rm_next_ie(ie, &len);
	}

	return count;
}

static ccx_rm_req_ie_t*
wlc_ccx_rm_next_ie(ccx_rm_req_ie_t* ie, int* len)
{
	int buflen = *len;
	int ie_len;

	/* expect a valid CCX RM IE */
	ASSERT(buflen >= CCX_RM_IE_HDR_LEN);
	ASSERT(ie && (buflen >= CCX_RM_IE_HDR_LEN + ltoh16(ie->len)));

	if (ie == NULL)
		return NULL;

	ie_len = ltoh16(ie->len);

	do {
		/* advance to the next IE */
		buflen -= CCX_RM_IE_HDR_LEN + ie_len;
		ie = (ccx_rm_req_ie_t*)((int8*)ie + CCX_RM_IE_HDR_LEN + ie_len);

		if (ie == NULL)
			return NULL;

		/* make sure there is room for a valid CCX RM IE */
		if (buflen < CCX_RM_IE_HDR_LEN ||
		    buflen < CCX_RM_IE_HDR_LEN + (ie_len = ltoh16(ie->len))) {
			buflen = 0;
			ie = NULL;
			break;
		}

		if (ltoh16(ie->id) == CCX_RM_ID_REQUEST &&
		    ie_len >= CCX_RM_REQ_IE_FIXED_LEN) {
			/* found valid measurement request */
			break;
		}
	} while (ie);

	*len = buflen;
	return ie;
}

/* add a new report block to the ccx report list */
static wlc_ccx_rm_rept_t*
wlc_ccx_rm_new_report(wlc_ccx_info_t *ci, uchar** buf, int* avail_len)
{
	wlc_ccx_rm_rept_t *report;
	wlc_ccx_rm_rept_t *new_report;

	*avail_len = 0;
	*buf = NULL;

	/* Allocate a new report */
	new_report = MALLOCZ(ci->pub->osh, sizeof(wlc_ccx_rm_rept_t));
	if (new_report == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			ci->pub->unit, __FUNCTION__, MALLOCED(ci->pub->osh)));
		return NULL;
	}

	*avail_len = CCX_RM_REP_DATA_MAX_LEN;
	*buf = new_report->data;

	/* and hook it into the ccx report list */
	report = ci->rm_reports;

	if (report == NULL) {
		/* first report in the linked list */
		ci->rm_reports = new_report;
	} else {
		/* put the new report at the end of the linked list */
		while (report->next != NULL)
			report = report->next;

		report->next = new_report;
	}

	return new_report;
}

void
wlc_ccx_rm_report(ccx_t *ccxh, wlc_rm_req_t *req_block, int count)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	ccx_rm_t *rm = ci->rm_state->ccx;
	wlc_rm_req_t *req;
	wlc_ccx_rm_rept_t *report;
	uchar *rep_ptr = NULL;
	uint8 type;
	uint bss_count;
	wlc_bss_info_t **bss_ptrs;
	int bss_reported;
	int buflen = 0;
	bool measured;
	int i;
	int err;

	/* ongoing count BSSs already reported in beacon table report */
	bss_count = rm->scan_results.count;
	bss_ptrs = rm->scan_results.ptrs;

	report = ci->rm_reports;
	if (report == NULL) {
		/* create first report block on list */
		report = wlc_ccx_rm_new_report(ci, &rep_ptr, &buflen);
		if (report == NULL)
			return;
	} else {
		/* the current open report is the last on the list */
		while (report->next != NULL)
			report = report->next;

		buflen = CCX_RM_REP_DATA_MAX_LEN - report->len;
		rep_ptr = report->data + report->len;
	}

	/* format each report based on the request info and measured data */
	for (i = 0; i < count; i++) {
		err = 0;
		type = 0;
		req = req_block + i;

		if ((req->flags & (WLC_RM_FLAG_LATE |
			WLC_RM_FLAG_INCAPABLE |
			WLC_RM_FLAG_REFUSED))) {
			measured = FALSE;
		} else {
			measured = TRUE;
		}

		switch (req->type) {
		case WLC_RM_TYPE_CCA:
			if (!measured) {
				type = CCX_RM_TYPE_LOAD;
				break;
			}

			err = wlc_ccx_rm_append_load_report(ci, req, &rep_ptr, &buflen);

			break;
		case WLC_RM_TYPE_RPI:
			if (!measured) {
				type = CCX_RM_TYPE_NOISE;
				break;
			}

			err = wlc_ccx_rm_append_noise_report(ci, req, &rep_ptr, &buflen);

			break;
		case WLC_RM_TYPE_BEACON_TABLE:
			/* we do not implement or respond to this type,
			 * set measured in case it was flase so that we do
			 * generate a reply.
			 */
			measured = TRUE;
			/* if we want to respond, we should mark as CCX_RM_MODE_INCAPABLE
			 * in wlc_ccx_rm_parse_requests, and just fold this case with
			 * WLC_RM_TYPE_PASSIVE_SCAN and WLC_RM_TYPE_ACTIVE_SCAN.
			 */
			break;
		case WLC_RM_TYPE_PASSIVE_SCAN:
		case WLC_RM_TYPE_ACTIVE_SCAN:
			if (!measured) {
				type = CCX_RM_TYPE_BEACON;
				break;
			}

			if (bss_count > 0) {
				bss_reported = wlc_ccx_rm_beacon_table(bss_ptrs, bss_count, req,
					&rep_ptr, &buflen);
				bss_count -= bss_reported;
				bss_ptrs += bss_reported;

				if (bss_count > 0) {
					/* set the error since we overflowed the buffer */
					err = 1;
				}
			}
			break;
		case WLC_RM_TYPE_FRAME:
			if (!measured) {
				type = CCX_RM_TYPE_FRAME;
				break;
			}

			err = wlc_ccx_rm_append_frame_report(ci, req, &rep_ptr, &buflen);

			break;
		default:
			/* set measured so that we do not respond, even if a refusal flag is set */
			measured = TRUE;
			break;
		}

		if (!measured) {
			if (buflen < CCX_RM_REP_IE_FIXED_LEN) {
				err = 1;
			} else {
				wlc_ccx_rm_append_report_header(type, (uint16)req->token,
					req->flags, 0, rep_ptr);
				rep_ptr += CCX_RM_REP_IE_FIXED_LEN;
				buflen -= CCX_RM_REP_IE_FIXED_LEN;
			}
		}

		/* check for a report buffer overflow */
		if (err) {
			/* update the report block that filled up */
			report->len = (uint)(CCX_RM_REP_DATA_MAX_LEN - buflen);

			/* create a new report block to continue writing the remainin measurements
			 */
			report = wlc_ccx_rm_new_report(ci, &rep_ptr, &buflen);
			if (report == NULL) {
				WL_INFORM(("wl%d: wlc_ccx_rm_report: Unable to get new "
					"report block exiting\n",
					ci->pub->unit));
				break;
			}
			/* roll back for loop to previous request */
			i--;
		}

		ASSERT(buflen <= CCX_RM_REP_DATA_MAX_LEN);
		ASSERT(rep_ptr <= report->data + CCX_RM_REP_DATA_MAX_LEN);
	}

	if (report != NULL)
		report->len = (uint)(CCX_RM_REP_DATA_MAX_LEN - buflen);
}

static void
wlc_ccx_rm_append_report_header(uint8 type, uint16 token, uint8 flags, int data_len, uchar *buf)
{
	ccx_rm_rep_ie_t rep;

	bzero(&rep, sizeof(rep));
	rep.id = htol16(CCX_RM_ID_REPORT);
	rep.len = htol16((CCX_RM_REP_IE_FIXED_LEN - CCX_RM_IE_HDR_LEN) + data_len);
	rep.token = htol16(token);
	rep.mode = 0;
	if (flags & WLC_RM_FLAG_PARALLEL)
		rep.mode |= CCX_RM_MODE_PARALLEL;
	if (flags & WLC_RM_FLAG_LATE)
		rep.mode |= CCX_RM_MODE_REFUSED;
	if (flags & WLC_RM_FLAG_INCAPABLE)
		rep.mode |= CCX_RM_MODE_INCAPABLE;
	if (flags & WLC_RM_FLAG_REFUSED)
		rep.mode |= CCX_RM_MODE_REFUSED;
	rep.type = type;

	bcopy(&rep, buf, CCX_RM_REP_IE_FIXED_LEN);
}

static int
wlc_ccx_rm_append_load_report(wlc_ccx_info_t *ci, wlc_rm_req_t *req, uchar **bufptr, int *buflen)
{
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	int rep_len;
	int data_len;
	uchar *data;
	ccx_rm_load_rep_t load;

	data_len = CCX_RM_LOAD_REP_LEN;
	rep_len = CCX_RM_REP_IE_FIXED_LEN + data_len;

	if (*buflen < rep_len) {
		WL_INFORM(("wl%d: wlc_ccx_rm_append_load_report: Channel Load report len %d "
			"does not fit, %d bytes left in report frame.\n",
			ci->pub->unit, rep_len, *buflen));
		return 1;
	}

	wlc_ccx_rm_append_report_header(CCX_RM_TYPE_LOAD, (uint16)req->token, req->flags, data_len,
		*bufptr);
	data = *bufptr + CCX_RM_REP_IE_FIXED_LEN;

	load.channel = CHSPEC_CHANNEL(req->chanspec);
	load.spare = 0;
	load.duration = (uint16)htol16(req->dur);
	load.fraction = rm_state->cca_busy;

	bcopy(&load, data, data_len);

	*buflen -= rep_len;
	*bufptr += rep_len;

	return 0;
}

static int
wlc_ccx_rm_append_noise_report(wlc_ccx_info_t *ci, wlc_rm_req_t *req, uchar **bufptr, int *buflen)
{
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	int rep_len;
	int data_len;
	uchar *data;
	int bin;
	ccx_rm_noise_rep_t noise;

	data_len = CCX_RM_NOISE_REP_LEN;
	rep_len = CCX_RM_REP_IE_FIXED_LEN + data_len;

	if (*buflen < rep_len) {
		WL_INFORM(("wl%d: wlc_ccx_rm_append_noise_report: Noise report len %d "
			   "does not fit, %d bytes left in report frame.\n",
			   ci->pub->unit, rep_len, *buflen));
		return 1;
	}

	wlc_ccx_rm_append_report_header(CCX_RM_TYPE_NOISE, (uint16)req->token, req->flags, data_len,
		*bufptr);
	data = *bufptr + CCX_RM_REP_IE_FIXED_LEN;

	noise.channel = CHSPEC_CHANNEL(req->chanspec);
	noise.spare = 0;
	noise.duration = (uint16)htol16(req->dur);

	for (bin = 0; bin < 8; bin++)
		noise.rpi[bin] = (uint8)rm_state->rpi[bin];

	bcopy(&noise, data, data_len);

	*buflen -= rep_len;
	*bufptr += rep_len;

	return 0;
}

static int
wlc_ccx_rm_append_frame_report(wlc_ccx_info_t *ci, wlc_rm_req_t *req, uchar **bufptr, int *buflen)
{
	ccx_rm_t *rm = ci->rm_state->ccx;
	int rep_len;
	int data_len;
	wlc_ccx_rm_frm_elt_t *frm_elt;
	ccx_rm_frame_rep_t *frame;
	ccx_rm_frm_rep_elt_t *frm_rep_elt;
	int i;

	data_len = CCX_RM_FRAME_REP_FIXED_LEN + (rm->frm_elts *
		CCX_RM_FRAME_REP_ENTRY_LEN);
	rep_len = CCX_RM_REP_IE_FIXED_LEN + data_len;

	if (*buflen < rep_len) {
		WL_INFORM(("wl%d: wlc_ccx_rm_append_frame_report: Frame report len %d "
			   "does not fit, %d bytes left in report frame.\n",
			   ci->pub->unit, rep_len, *buflen));
		return 1;
	}

	wlc_ccx_rm_append_report_header(CCX_RM_TYPE_FRAME, (uint16)req->token,
		req->flags, data_len, *bufptr);
	frame = (ccx_rm_frame_rep_t *)(*bufptr + CCX_RM_REP_IE_FIXED_LEN);

	frame->channel = CHSPEC_CHANNEL(req->chanspec);
	frame->spare = 0;
	frame->duration = (uint16)htol16(req->dur);

	frm_rep_elt = (ccx_rm_frm_rep_elt_t *)&frame->elt[0];
	for (i = 0; i < WLC_NRMFRAMEHASH; i++) {
		for (frm_elt = rm->frmhash[i]; frm_elt; frm_elt = frm_elt->next,
			frm_rep_elt++) {
			bcopy(&frm_elt->ta, &frm_rep_elt->ta, ETHER_ADDR_LEN);
			bcopy(&frm_elt->bssid, &frm_rep_elt->bssid, ETHER_ADDR_LEN);
			frm_rep_elt->rssi = frm_elt->rssi_sum / frm_elt->frames;
			frm_rep_elt->frames = frm_elt->frames;
		}
	}

	*buflen -= rep_len;
	*bufptr += rep_len;

	return 0;
}

/* The set of IEs from beacons and probe responses that will be included in a CCX beacon report */
static const uint8 ccx_rm_beacon_table_ies[] = {
	DOT11_MNG_SSID_ID,
	DOT11_MNG_RATES_ID,
	DOT11_MNG_FH_PARMS_ID,
	DOT11_MNG_DS_PARMS_ID,
	DOT11_MNG_CF_PARMS_ID,
	DOT11_MNG_TIM_ID,
	DOT11_MNG_IBSS_PARMS_ID,
	DOT11_MNG_EXT_RATES_ID
};

/* Writes the set of Beacon Measurement Report IEs corresponding to the 'results' array of
 * bss_info's.
 * Results will be truncated if there is not enough room in buf.
 * Returns the number of bss_info's that remain after filling the buffer.
 */
static int
wlc_ccx_rm_beacon_table(wlc_bss_info_t **results, uint num, wlc_rm_req_t *req, uchar **bufptr,
	int *buflen)
{
	uint i;
	int elt_len;
	int bcn_rep_len;
	uint8 channel;
	uint16 dur;
	uchar *buf;
	int len;
	uint8 report[MAXBSS];

	buf = *bufptr;
	len = *buflen;

	channel = CHSPEC_CHANNEL(req->chanspec);
	dur = (uint16)req->dur;

	if (req->type == WLC_RM_TYPE_PASSIVE_SCAN ||
		req->type == WLC_RM_TYPE_ACTIVE_SCAN) {
		memset(report, 1, num);
		wlc_ccx_rm_mbssid_report(results, report, num);
	}

	for (i = 0; i < num; i++) {
		if ((req->type == WLC_RM_TYPE_PASSIVE_SCAN ||
			req->type == WLC_RM_TYPE_ACTIVE_SCAN) && !report[i])
			continue;

		elt_len = CCX_RM_REP_IE_FIXED_LEN;
		if (len < elt_len)
			break;

		bcn_rep_len = wlc_ccx_rm_beacon_table_entry(results[i], channel, dur,
			buf + elt_len, len - elt_len);
		if (bcn_rep_len == 0)
			break;

		elt_len += bcn_rep_len;

		wlc_ccx_rm_append_report_header(CCX_RM_TYPE_BEACON, (uint16)req->token, req->flags,
			bcn_rep_len, buf);

		ASSERT(len >= elt_len);

		buf += elt_len;
		len -= elt_len;
	}

	if (i != num) {
		WL_INFORM(("wlc_ccx_rm_beacon_table: truncating beacon report after "
		           "%d of %d results\n", i, num));
	}

	*bufptr = buf;
	*buflen = len;

	return i;
}

/* Writes the Measurement Report portion of a Beacon Measurement Report IE to 'buf', length
 * 'buflen'.
 * Returns the length of the becon report written to 'buf'
 * If the buffer is not large enough to contain the report, a length of zero is returned.
 */
static int
wlc_ccx_rm_beacon_table_entry(wlc_bss_info_t *bi, uint8 channel, uint16 dur, uchar *buf, int buflen)
{
	uint8 id;
	uint i;
	bool match;
	int len;
	int copy_len;
	int bcn_len;
	bcm_tlv_t *bcn_ie;
	bcm_tlv_t *rep_ie;
	ccx_rm_beacon_rep_t rep;

	bzero(&rep, sizeof(rep));

	rep_ie = NULL;
	bcn_ie = NULL;

	len = CCX_RM_BEACON_REP_FIXED_LEN;
	rep_ie = (bcm_tlv_t *)(buf + len);

	if (len > buflen)
		return 0;

	ASSERT(bi != NULL);
	bcn_len = bi->bcn_prb_len;
	if (bi->bcn_prb && bcn_len > DOT11_BCN_PRB_LEN) {
		bcn_ie = (bcm_tlv_t*)((int8*)bi->bcn_prb + DOT11_BCN_PRB_LEN);
		bcn_len -= DOT11_BCN_PRB_LEN;
	}

	while (bcn_ie && bcn_len > TLV_HDR_LEN && bcn_len >= bcn_ie->len + TLV_HDR_LEN) {
		id = bcn_ie->id;

		/* check for an ie we are interested in */
		match = FALSE;
		for (i = 0; i < ARRAYSIZE(ccx_rm_beacon_table_ies); i++) {
			if (id == ccx_rm_beacon_table_ies[i]) {
				match = TRUE;
				break;
			}
		}
		/* more checks for PROP_IE for radio cap ie */
		if (id == DOT11_MNG_PROPR_ID &&
		    bcn_ie->len >= (DOT11_OUI_LEN + 1) &&
		    !bcmp(((ccx_radio_mgmt_t*)bcn_ie)->oui, CISCO_AIRONET_OUI, DOT11_OUI_LEN) &&
		    ((ccx_radio_mgmt_t*)bcn_ie)->ver == 1)
			match = TRUE;

		if (match) {
			copy_len = bcn_ie->len + TLV_HDR_LEN;
			/* TIM and CF IEs get truncated */
			if (id == DOT11_MNG_TIM_ID) {
				/* TIM must be trucated to 4 bytes */
				copy_len = MIN(4, copy_len);
			} else if (id == DOT11_MNG_CF_PARMS_ID) {
				/* CF IE must be trucated to 6 bytes */
				copy_len = MIN(6, copy_len);
			}

			len += copy_len;

			if (len > buflen)
				return 0;

			/* copy the orig ie from the saved bcn to the report ies */
			ASSERT(buflen >= copy_len + ((uchar*)rep_ie - buf));
			bcopy(bcn_ie, rep_ie, copy_len);
			/* adjust the rep_ie length if we trucated the bcn ie */
			if (copy_len != bcn_ie->len + TLV_HDR_LEN)
				rep_ie->len = copy_len - TLV_HDR_LEN;
			/* advance to the next report ie position */
			rep_ie = (bcm_tlv_t*)((int8*)rep_ie + copy_len);
		}

		/* advance to the next bcn ie */
		bcn_len -= bcn_ie->len + TLV_HDR_LEN;
		bcn_ie = (bcm_tlv_t*)(bcn_ie->data + bcn_ie->len);
	}

	ASSERT(buflen >= (uchar*)rep_ie - buf);

	rep.channel = channel;
	rep.spare = 0;
	rep.duration = htol16(dur);
	rep.phy_type = bi->ccx_phy_type;
	rep.rssi = (int8)bi->RSSI;
	bcopy(bi->BSSID.octet, rep.bssid.octet, ETHER_ADDR_LEN);
	rep.parent_tsf = bi->rx_tsf_l;
	if (bi->bcn_prb && bi->bcn_prb_len >= DOT11_BCN_PRB_LEN) {
		bcopy(bi->bcn_prb->timestamp, &rep.target_tsf_low, 2*sizeof(uint32));
		bcopy(&bi->bcn_prb->beacon_interval, &rep.beacon_interval, sizeof(uint16));
		bcopy(&bi->bcn_prb->capability, &rep.capability, sizeof(uint16));
	} else {
		rep.target_tsf_low = 0;
		rep.target_tsf_hi = 0;
		rep.beacon_interval = 0;
		rep.capability = 0;
	}

	bcopy(&rep, buf, CCX_RM_BEACON_REP_FIXED_LEN);

	return len;
}

/* send all the collected report blocks in sequence */
void
wlc_ccx_rm_end(ccx_t *ccxh)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	wlc_ccx_rm_rept_t *report;

	while ((report = ci->rm_reports) != NULL) {
		wlc_ccx_rm_send_measure_report(ci, &wlc->cfg->BSSID,
		                               (uint16)rm_state->token,
		                               report->data, report->len);
		if (!report->len) {
			WL_INFORM(("wl%d: wlc_ccx_rm_end: empty CCX RM report block\n",
				ci->pub->unit));
		}

		ci->rm_reports = report->next;
		MFREE(ci->pub->osh, report, sizeof(wlc_ccx_rm_rept_t));
	}
	ci->rm_reqcfg = NULL;
	return;
}

static void
wlc_ccx_rm_send_measure_report(wlc_ccx_info_t *ci, struct ether_addr *da,
	uint16 token, uint8 *report, uint report_len)
{
	void *p;
	uint8* pbody;
	uint body_len;
	struct dot11_llc_snap_header *snap;
	ccx_iapp_hdr_t* ccx_iapp;
	ccx_rm_rep_t* rm_rep;
	int prio;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	WL_INFORM(("wl%d: wlc_ccx_rm_send_measure_report: sending Measure Report (token %d) "
		"to %s\n", ci->pub->unit, token, bcm_ether_ntoa(da, eabuf)));

	/* Measure Report frame is
	 * 16 bytes Cisco/Aironet IAPP header
	 * 2 bytes RM Report header, the Dialog Token
	 * variable len report IEs
	 */
	body_len = DOT11_LLC_SNAP_HDR_LEN + CCX_IAPP_HDR_LEN + CCX_RM_REP_LEN + report_len;

	p = wlc_dataget(ci->wlc, da, (uint16)body_len, ETHER_HDR_LEN + body_len);
	if (p == NULL)
		return;

	pbody = (uint8*)PKTDATA(ci->pub->osh, p);

	snap = (struct dot11_llc_snap_header *)(pbody + ETHER_HDR_LEN);
	bcopy(CISCO_AIRONET_SNAP, snap, DOT11_LLC_SNAP_HDR_LEN);

	ccx_iapp = (ccx_iapp_hdr_t *)((int8*)snap + DOT11_LLC_SNAP_HDR_LEN);
	ccx_iapp->id_len = hton16(CCX_IAPP_ID_CONTROL | (body_len - DOT11_LLC_SNAP_HDR_LEN));
	ccx_iapp->type = CCX_IAPP_TYPE_RM;
	ccx_iapp->subtype = CCX_IAPP_SUBTYPE_ROAM_REP;
	bzero(ccx_iapp->da.octet, ETHER_ADDR_LEN);
	bcopy(ci->rm_reqcfg->cur_etheraddr.octet, ccx_iapp->sa.octet, ETHER_ADDR_LEN);

	rm_rep = (ccx_rm_rep_t*)ccx_iapp->data;
	rm_rep->token = htol16(token);

	bcopy(report, rm_rep->data, report_len);

	/* Set high priority so that the packet will get enqueued at the end of
	 * higher precedence queue in order
	 */
	prio = wlc_ccx_get_txpkt_prio((ccx_t *)ci, da);
	PKTSETPRIO(p, prio);
	wlc_sendpkt(ci->wlc, p, NULL);
}


static void
wlc_ccx_get_updated_timestamp(wlc_ccx_info_t *ci, struct dot11_bcn_prb *bcn_prb,
	uint32 rx_tsf_l, uint32 *tsf_l, uint32 *tsf_h)
{
	uint32 delta;

	wlc_read_tsf(ci->wlc, tsf_l, tsf_h);
	delta = *tsf_l - rx_tsf_l;
	WL_ASSOC(("wl%d: %s: JOIN: cckm reassoc timestamp delta %d us\n",
		ci->pub->unit, __FUNCTION__, delta));
	*tsf_l = ltoh32(bcn_prb->timestamp[0]);
	*tsf_h = ltoh32(bcn_prb->timestamp[1]);
	wlc_uint64_add(tsf_h, tsf_l, 0, delta);
}

/* Merge in IHV CCX IEs if any */
static uint
wlc_ccx_calc_ihv_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_bsscfg_t* cfg = data->cfg;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx, cfg);

	wlc_info_t *wlc = ccx->wlc;
	uint len = 0;

	if (!CCX_ENAB(wlc->pub) || !cfgi->ccx_network)
		return 0;


	return len;
}

static int
wlc_ccx_write_ihv_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_bsscfg_t* cfg = data->cfg;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx, cfg);
	wlc_info_t *wlc = ccx->wlc;

	if (!CCX_ENAB(wlc->pub) || !cfgi->ccx_network)
		return BCME_OK;


	return BCME_OK;
}

/* CCXv2 Version IE */
static uint
wlc_ccx_calc_ver_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_bsscfg_t* cfg = data->cfg;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx, cfg);

	wlc_info_t *wlc = ccx->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (!CCX_ENAB(wlc->pub) || !cfgi->ccx_network)
		return 0;

	if (ftcbparm->assocreq.target->ccx_version)
		return TLV_HDR_LEN + ccx_version_ie[TLV_LEN_OFF];

	return 0;
}

static int
wlc_ccx_write_ver_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	wlc_bsscfg_t* cfg = data->cfg;
	ccx_bsscfg_priv_t* cfgi = CCX_BSSCFG_CUBBY(ccx, cfg);
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (!CCX_ENAB(wlc->pub) || !cfgi->ccx_network) {
		return BCME_OK;
	}

	if (ftcbparm->assocreq.target->ccx_version) {
		bcm_copy_tlv(ccx_version_ie, data->buf);
	}

	return BCME_OK;
}

static int
wlc_ccx_scan_parse_ver_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	ccx_version_ie_t *ccx_ver_ie = (ccx_version_ie_t *)data->ie;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	if (ccx_ver_ie == NULL)
		return BCME_OK;

	if (ccx_ver_ie->len == CCX_VERSION_IE_LEN)
		bi->ccx_version = ccx_ver_ie->version;

	if (ccx_ver_ie->version >= 5) {
		ccx_sfa_ie_t *ccx_sfa_ie;
		uint8 ccx_sfa_msgtype = CCX_SFA_IE_TYPE;

		ccx_sfa_ie = (ccx_sfa_ie_t *)
		     bcm_find_vendor_ie(data->buf, data->buf_len, CISCO_AIRONET_OUI,
		                        &ccx_sfa_msgtype, sizeof(ccx_sfa_msgtype));
		if (ccx_sfa_ie == NULL ||
		    ccx_sfa_ie->length != sizeof(ccx_sfa_ie_t) - TLV_HDR_LEN) {
#if defined(BCMDBG) || defined(WLMSG_INFORM)
			wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
			wlc_info_t *wlc = ccx->wlc;
#endif
			/* a valid SFA IE must be included.  If not, discard frame */
			WL_INFORM(("wl%d: %s: SFA IE is not found or length is invalid\n",
				wlc->pub->unit, __FUNCTION__));
			return BCME_ERROR;
		}
	}

	return BCME_OK;
}

/* CCXv2 Radio Management Capability IE */
static uint
wlc_ccx_calc_rm_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_bsscfg_t* cfg = data->cfg;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx, cfg);

	wlc_info_t *wlc = ccx->wlc;

	if (!CCX_ENAB(wlc->pub))
		return 0;

	if (data->ft == FC_PROBE_REQ) {
		rm_info_t *rmi = wlc->rm_info;
		wlc_rm_req_state_t *rms = rmi->rm_state;

		if (rms->step == WLC_RM_IDLE ||
		    rms->req[rms->cur_req].type != WLC_RM_TYPE_ACTIVE_SCAN)
			return 0;
	}
	else if (!cfgi->ccx_network)
		return 0;

	if (ccx->pubci.rm)
		return TLV_HDR_LEN + ccx_rm_capability_ie[TLV_LEN_OFF];

	return 0;
}

static int
wlc_ccx_write_rm_cap_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_bsscfg_t* cfg = data->cfg;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx, cfg);

	wlc_info_t *wlc = ccx->wlc;

	if (!CCX_ENAB(wlc->pub))
		return BCME_OK;

	if (data->ft == FC_PROBE_REQ) {
		rm_info_t *rmi = wlc->rm_info;
		wlc_rm_req_state_t *rms = rmi->rm_state;

		if (rms->step == WLC_RM_IDLE ||
		    rms->req[rms->cur_req].type != WLC_RM_TYPE_ACTIVE_SCAN)
			return BCME_OK;
	}
	else if (!cfgi->ccx_network) {
		return BCME_OK;
	}

	if (ccx->pubci.rm) {
		bcm_copy_tlv(ccx_rm_capability_ie, data->buf);
	}

	return BCME_OK;
}

/* CCKM IE */
static uint
wlc_ccx_calc_cckm_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx, cfg);

	if (!CCX_ENAB(wlc->pub))
		return 0;

		if (data->ft == FC_REASSOC_REQ &&
		    IS_CCKM_AUTH(cfg->WPA_auth) && WLC_PORTOPEN(cfg)) {
			return sizeof(cckm_reassoc_req_ie_t);
		}
		else if (data->ft == FC_ASSOC_REQ &&
			IS_CCKM_AUTH(cfg->WPA_auth) && !WLC_PORTOPEN(cfg)) {
#if defined(BCMINTSUP)
			if (!SUP_ENAB(wlc->pub) || !BSS_SUP_ENAB_WPA(wlc->idsup, cfg))
#endif
			/* just initialize rn value and return 0, i.e. do NOT write cckm ie */
			cfgi->rn = 1;
		}

	return 0;
}

static bool
wlc_ccx_reassocreq_ins_cb(void *ctx, wlc_iem_ins_data_t *data)
{
	return FALSE;
}

static bool
wlc_ccx_reassocreq_mod_cb(void *ctx, wlc_iem_mod_data_t *data)
{
	return FALSE;
}

static int
wlc_ccx_get_wpa_ie(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *buf, uint buf_len)
{
	wlc_iem_uiel_t uiel, *puiel;
	wlc_assoc_t *as;

	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;
	uint ielen;
	int ret;

	if (!wlc || !cfg || !buf)
		return BCME_BADARG;

	as = cfg->assoc;

	puiel = NULL;
	if (as->ie != NULL) {
		wlc_iem_build_uiel_init(wlc->iemi, &uiel);
		/* overrides */
		uiel.ies = as->ie;
		uiel.ies_len = as->ie_len;
		uiel.ins_fn = wlc_ccx_reassocreq_ins_cb;
		uiel.mod_fn = wlc_ccx_reassocreq_mod_cb;
		puiel = &uiel;
	}

	bzero(&ftcbparm, sizeof(ftcbparm));
	bzero(&cbparm, sizeof(cbparm));
	cbparm.ft = &ftcbparm;

	bzero(buf, buf_len);

	ret = BCME_NOTFOUND;
	if ((ielen = wlc_iem_vs_calc_ie_len(wlc->iemi, cfg, FC_REASSOC_REQ,
		WLC_IEM_VS_IE_PRIO_WPA,	puiel, &cbparm)) != 0) {
		if (ielen <= buf_len)
			ret = wlc_iem_vs_build_ie(wlc->iemi, cfg, FC_REASSOC_REQ,
				WLC_IEM_VS_IE_PRIO_WPA, puiel, &cbparm, buf, buf_len);
		else
			ret = BCME_BUFTOOSHORT;
	}
	return ret;
}

static int
wlc_ccx_write_cckm_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	wlc_bss_info_t *bi = ftcbparm->assocreq.target;
	wlc_bsscfg_t *cfg = data->cfg;
	ccx_bsscfg_priv_t* cfgi = CCX_BSSCFG_CUBBY(ccx, cfg);
	if (!CCX_ENAB(wlc->pub))
		return BCME_OK;

		if (data->ft == FC_REASSOC_REQ &&
		    IS_CCKM_AUTH(cfg->WPA_auth) && WLC_PORTOPEN(cfg)) {
			wpa_ie_fixed_t *wpa_ie;
			uint32 tsf_l, tsf_h;
			cckm_reassoc_req_ie_t *cckmie;
			uint8 iebuf[30] = {0};

			wpa_ie = NULL;
			if (cfg->WPA_auth == WPA2_AUTH_CCKM)
				wpa_ie = (wpa_ie_fixed_t*)ftcbparm->assocreq.wpa2_ie;
			else
				if (wlc_ccx_get_wpa_ie(wlc, cfg, iebuf, sizeof(iebuf)) == BCME_OK)
					wpa_ie = (wpa_ie_fixed_t*)iebuf;
			if (wpa_ie == NULL) {
				WL_ERROR(("wl%d: %s: no WPA or WPA2 IE\n",
				          ccx->pub->unit, __FUNCTION__));
				return BCME_ERROR;
			}

			/* copy WPA info element template */
			cckmie = (cckm_reassoc_req_ie_t *)data->buf;

			bcm_copy_tlv(CCKM_info_element, data->buf);

			wlc_ccx_get_updated_timestamp(ccx, bi->bcn_prb,
			                              bi->rx_tsf_l, &tsf_l, &tsf_h);
			/* fill in CCKM reassoc req IE */
#if defined(BCMINTSUP)
		if (SUP_ENAB(wlc->pub) && BSS_SUP_ENAB_WPA(wlc->idsup, cfg))
				wlc_cckm_gen_reassocreq_IE(ccx->wlc->ccxsup, cfg, cckmie,
					tsf_h, tsf_l, &bi->BSSID, wpa_ie);
		else
#endif /* BCMINTSUP */
			{
			uint32 rn;
			uint32 timestamp[2];

			timestamp[0] = htol32(tsf_l);
			timestamp[1] = htol32(tsf_h);

			/* load timestamp from bcn_prb (< 1s) */
			bcopy(timestamp, cckmie->timestamp, DOT11_MNG_TIMESTAMP_LEN);

			/* increment and load RN */
			rn = ++cfgi->rn;
			/* 80211 uses le byte order */
			rn = htol32(rn);
			bcopy((char *)&rn, (char *)&cckmie->rn, sizeof(rn));

			/* calculate and load MIC */
			wlc_cckm_calc_reassocreq_MIC(cckmie, &bi->BSSID, wpa_ie,
			                             &cfg->cur_etheraddr,
			                             cfgi->rn, cfgi->key_refresh_key,
			                             cfg->WPA_auth);
			}
		}

	return BCME_OK;
}

/* Aironet IE */
static uint
wlc_ccx_calc_aironet_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	wlc_bss_info_t *bi = ftcbparm->assocreq.target;
	wlc_bsscfg_t *cfg = data->cfg;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx, cfg);

	if (!CCX_ENAB(wlc->pub) || !cfgi->ccx_network)
		return 0;

		if (bi->aironet_ie_rx) {
			return sizeof(aironet_assoc_ie_t);
		}

	return 0;
}

static int
wlc_ccx_write_aironet_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	wlc_bss_info_t *bi = ftcbparm->assocreq.target;
	wlc_bsscfg_t *cfg = data->cfg;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx, cfg);

	if (!CCX_ENAB(wlc->pub) || !cfgi->ccx_network)
		return BCME_OK;

		if (bi->aironet_ie_rx) {
			aironet_assoc_ie_t dev_ie;

			/* add Machine Name to (re)association request message */
			bzero(&dev_ie, sizeof(aironet_assoc_ie_t));
			dev_ie.id = DOT11_MNG_AIRONET_ID;
			dev_ie.len = sizeof(aironet_assoc_ie_t)-TLV_HDR_LEN;
			dev_ie.device = AIRONET_IE_DEVICE_ID;
			dev_ie.refresh_rate = KEEP_ALIVE_INTERVAL;
			dev_ie.flags = (CKIP_KP | CKIP_MIC);
			strncpy(dev_ie.name, cfgi->staname, sizeof(dev_ie.name)-1);
			dev_ie.name[sizeof(dev_ie.name) - 1] = '\0';
			bcopy((uint8 *)&dev_ie, data->buf, sizeof(aironet_assoc_ie_t));
#ifdef BCMDBG
			WL_WSEC(("wl%d: %s: JOIN: requesting full CKIP support\n",
			         ccx->pub->unit, __FUNCTION__));
#endif /* BCMDBG */
		}
	/* Driver maintains rn value for external supplicant */
	if ((!SUP_ENAB(wlc->pub) || !BSS_SUP_ENAB_WPA(wlc->idsup, cfg)) &&
		data->ft != FC_REASSOC_REQ &&
		IS_CCKM_AUTH(cfg->WPA_auth) && !WLC_PORTOPEN(cfg)) {
		cfgi->rn = 1;
	}

	return BCME_OK;
}

static int
wlc_ccx_parse_aironet_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	wlc_bsscfg_t* cfg = data->cfg;

	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx, cfg);


	if (!CCX_ENAB(wlc->pub) || !cfgi->ccx_network)
		return BCME_OK;

	if (data->ie == NULL)
		return BCME_OK;


	return BCME_OK;
}

static int
wlc_ccx_scan_parse_aironet_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	bcm_tlv_t *ie = (bcm_tlv_t *)data->ie;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	if (ie == NULL)
		return BCME_OK;

	/* CCX parameters */
	bi->aironet_ie_rx = FALSE;

	if (ie->len > AIRONET_IE_CKIP) {
		bi->ckip_support = ie->data[AIRONET_IE_CKIP] & (CKIP_MIC | CKIP_KP);
		bi->aironet_ie_rx = TRUE;
	}

	return BCME_OK;
}

static int
wlc_ccx_scan_parse_qbss_load_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	dot11_qbss_load_ie_t *qbss_load_ie = (dot11_qbss_load_ie_t *)data->ie;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	bi->qbss_load_chan_free = WLC_QBSS_LOAD_CHAN_FREE_LEGACY_AP;
	bi->qbss_load_aac = WLC_QBSS_LOAD_AAC_LEGACY_AP;

	if (qbss_load_ie == NULL)
		return BCME_OK;

	if (qbss_load_ie->length == (sizeof(dot11_qbss_load_ie_t) - TLV_HDR_LEN)) {
		/* convert channel utilization to channel free score */
		bi->qbss_load_chan_free =
		        (uint8)WLC_QBSS_LOAD_CHAN_FREE_MAX - qbss_load_ie->channel_utilization;
		bi->qbss_load_aac = ltoh16_ua((uint8 *)&qbss_load_ie->aac);
	}

	return BCME_OK;
}

static int
wlc_ccx_bcn_parse_qos_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	ccx_qos_params_t *ccx_qos_ie = (ccx_qos_params_t *)data->ie;
	uint16 cwmin, cwmax;

	if (ccx_qos_ie != NULL) {
		if ((ccx_qos_ie->len == CCX_QOS_IE_LEN) &&
		    (ccx_qos_ie->count > cfg->current_bss->ccx_qos_params_cnt)) {
			cwmin = (2 << ((ccx_qos_ie->ecw_0 & 0xf) - 1)) - 1;
			cwmax = (2 << (((ccx_qos_ie->ecw_0 >> 4) & 0xf) - 1)) - 1;
			wlc_set_cwmin(wlc, cwmin);
			wlc_set_cwmax(wlc, cwmax);
			cfg->current_bss->ccx_qos_params_cnt = ccx_qos_ie->count;
		}
	}

	return BCME_OK;
}

static int
wlc_ccx_bcn_parse_aironet_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	bcm_tlv_t *ckip_ie = (bcm_tlv_t *)data->ie;
	uint16 cwmin, cwmax;

	if (ckip_ie != NULL) {
		/* track for cwmin/cwmax in BCMCCX beacon, update haredware if necessary.
		 * CWMin and CWMax are uint16 values at offset AIRONET_IE_CWMIN and
		 * AIRONET_IE_CWMAX, make sure the ie length covers both uint16s before
		 * using
		 */
		if (ckip_ie->len > (AIRONET_IE_CWMAX + 1)) {
			cwmin = ltoh16_ua(&ckip_ie->data[AIRONET_IE_CWMIN]);
			cwmax = ltoh16_ua(&ckip_ie->data[AIRONET_IE_CWMAX]);

			/* validate cwmin and cwmax.  if validation fails, do not adopt
			 * either of them
			 */
			if (VALID_MASK(cwmin) && VALID_MASK(cwmax)) {
				if (cwmin != wlc->band->CWmin)
					wlc_set_cwmin(wlc, cwmin);
				if (cwmax != wlc->band->CWmax)
					wlc_set_cwmax(wlc, cwmax);
			} else
				WL_ERROR(("wl%d: %s: invalid cwmin or cwmax"
					" value\n", wlc->pub->unit, __FUNCTION__));
		}
	}

	return BCME_OK;
}

void
wlc_cckm_calc_reassocreq_MIC(cckm_reassoc_req_ie_t *cckmie,
	struct ether_addr *bssid, wpa_ie_fixed_t *rsnie, struct ether_addr *cur_ea,
	uint32 rn, uint8 *key_refresh_key, uint32 WPA_auth)
{
	uchar data[128], hash_buf[20];
	int data_len = 0;

	/* create the data portion (STA-ID | BSSID | RSNIE | Timestamp | RN) */
	bcopy((uint8 *)cur_ea, (char *)&data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	bcopy((uint8 *)bssid, (char *)&data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	bcopy((uint8 *)rsnie, (char *)&data[data_len], TLV_HDR_LEN + rsnie->length);
	data_len += TLV_HDR_LEN + rsnie->length;
	bcopy((uint8 *)cckmie->timestamp, (char *)&data[data_len],
	      DOT11_MNG_TIMESTAMP_LEN);
	data_len += DOT11_MNG_TIMESTAMP_LEN;
	bcopy((uint8 *)&cckmie->rn, (char *)&data[data_len], sizeof(cckmie->rn));
	data_len += sizeof(cckmie->rn);

	/* generate the MIC */
#if defined(BCMDBG) || defined(WLMSG_WSEC)
	if (WL_WSEC_ON()) {
		printf("wlc_cckm_calc_reassocreq_MIC...\n");
		prhex("KRK looks like", key_refresh_key, 16);
	}
#endif /* BCMDBG || WLMSG_WSEC */
	if (WPA_auth == WPA2_AUTH_CCKM)
		hmac_sha1(data, data_len, key_refresh_key, CCKM_KRK_LEN, hash_buf);
	else
	hmac_md5(data, data_len, key_refresh_key, CCKM_KRK_LEN, hash_buf);
	bcopy(hash_buf, cckmie->mic, CCKM_MIC_LEN);
}


static int
wlc_ccx_config_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len)
{
	wlc_ccx_info_t	*ccx_info = (wlc_ccx_info_t *)ctx;
	ccx_bsscfg_priv_t	*cfgi = NULL;

	int config_len = CCX_CUBBY_CONFIG_SIZE;

	if (len == NULL) {
		WL_ERROR(("%s: Null len\n", __FUNCTION__));
		return BCME_ERROR;
	}

	ASSERT(cfg != NULL);
	if (BSSCFG_AP(cfg)) {
		*len = 0;
		return BCME_OK;
	}

	cfgi =  CCX_BSSCFG_CUBBY(ccx_info, cfg);

	if (cfgi == NULL) {
		WL_INFORM(("%s: ccx not enabled, nothing to config\n", __FUNCTION__));
		*len = 0;
		return BCME_OK;
	}

	if ((data == NULL) || (*len < config_len)) {
		WL_ERROR(("%s: Insufficient buffer data(%p) len(%d/%d)\n",
			__FUNCTION__, data, *len, config_len));
		*len = config_len;
		return BCME_BUFTOOSHORT;
	}
	if (data) {
		WL_WSEC(("wl%d.%d get cubby config info:%p \n",
			ccx_info->wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
			cfgi));
		*len = config_len;
		bcopy(CCX_CUBBY_CONFIG_DATA(cfgi), data, config_len);
	}
	return BCME_OK;
}

static int
wlc_ccx_config_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len)
{
	wlc_ccx_info_t	*ccx_info = (wlc_ccx_info_t *)ctx;
	ccx_bsscfg_priv_t	*cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	int config_len = CCX_CUBBY_CONFIG_SIZE;

	if ((data == NULL) || (len < config_len)) {
		WL_ERROR(("%s: data(%p) len(%d/%d)\n", __FUNCTION__, data, len, config_len));
		return BCME_ERROR;
	}

	bcopy(data, CCX_CUBBY_CONFIG_DATA(cfgi), CCX_CUBBY_CONFIG_SIZE);
	cfgi->wlc = ccx_info->wlc;
	cfgi->cfg =	cfg;

	WL_WSEC(("wl%d.%d set cubby config info:%p \n",
		ccx_info->wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
		cfgi));
	return BCME_OK;
}


uint wlc_ccx_get_orig_reason(ccx_t *ci, wlc_bsscfg_t *cfg)
{

	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	return cfgi->orig_reason;
}
void wlc_ccx_set_orig_reason(ccx_t *ci, wlc_bsscfg_t *cfg, uint orig_reason)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	cfgi->orig_reason = orig_reason;
}


chanspec_t* wlc_ccx_get_ap_chanspec_list(ccx_t *ci, wlc_bsscfg_t *cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	return cfgi->ap_chanspec_list;
}
uint wlc_ccx_get_ap_channel_num(ccx_t *ci, wlc_bsscfg_t *cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	return cfgi->ap_channel_num;
}

uint wlc_ccx_get_leap_start(ccx_t *ci, wlc_bsscfg_t *cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	return cfgi->leap_start;
}

bool wlc_ccx_get_leap_on(ccx_t *ci, wlc_bsscfg_t *cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	return cfgi->leap_on;
}

void wlc_ccx_set_leap_start(ccx_t *ci, wlc_bsscfg_t *cfg, uint leap_start)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	cfgi->leap_start = leap_start;
}
bool wlc_ccx_set_leap_on(ccx_t *ci, wlc_bsscfg_t *cfg, bool leap_on)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	cfgi->leap_on = leap_on;
	return cfgi->leap_on;
}
uint32 wlc_ccx_get_roaming_start_time(ccx_t *ci, wlc_bsscfg_t *cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	return cfgi->roaming_start_time;
}
uint32* wlc_ccx_get_roam_start_time(ccx_t *ci, wlc_bsscfg_t *cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	return &cfgi->roaming_start_time;
}
void wlc_ccx_set_roaming_delay_tu(ccx_t *ci, wlc_bsscfg_t *cfg, uint32 roaming_delay_tu)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	cfgi->roaming_delay_tu = roaming_delay_tu;
}
void wlc_ccx_increment_roaming_count(ccx_t *ci, wlc_bsscfg_t *cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	cfgi->roaming_count++;
}

bool wlc_ccx_get_ccx_network(ccx_t *ci, wlc_bsscfg_t *cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	return cfgi->ccx_network;
}
void wlc_ccx_set_ccx_network(ccx_t *ci, wlc_bsscfg_t *cfg, bool ccx_network)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	cfgi->ccx_network = ccx_network;
}
bool wlc_ccx_get_fast_roam(ccx_t *ci, wlc_bsscfg_t *cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	return cfgi->fast_roam;
}
void wlc_ccx_set_fast_roam(ccx_t *ci, wlc_bsscfg_t *cfg, bool fast_roam)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	ccx_bsscfg_priv_t *cfgi = CCX_BSSCFG_CUBBY(ccx_info, cfg);

	cfgi->fast_roam = fast_roam;
}
void wlc_ccx_update_roaming_delay_tu(ccx_t *ci, wlc_bsscfg_t *cfg)
{
	wlc_ccx_info_t *ccx_info = (wlc_ccx_info_t *)ci;
	if (BSSCFG_STA(cfg) && !BSS_P2P_ENAB(ccx_info->wlc, cfg)) {
		uint32 tsf_l, tsf_h;
		wlc_read_tsf(ccx_info->wlc, &tsf_l, &tsf_h);
		wlc_ccx_set_roaming_delay_tu(ci,
			cfg, (tsf_l - wlc_ccx_get_roaming_start_time(ci, cfg)));
		wlc_ccx_increment_roaming_count(ci, cfg); /* free running count */
	}
}
#endif /* BCMCCX */
