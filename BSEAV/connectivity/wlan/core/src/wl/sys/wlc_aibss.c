/**
 * Advanced IBSS implementation for Broadcom 802.11 Networking Driver
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
#include <proto/802.11.h>
#include <proto/vlan.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wl_export.h>
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif
#ifdef WLMCNX
#include <wlc_mcnx.h>
#endif
#include <wlc_utils.h>
#include <wlc_scb.h>
#include <wlc_pcb.h>
#include <wlc_tbtt.h>
#include <wlc_apps.h>
#include <wlc_txmod.h>
#include <wlc_aibss.h>
#if defined(SAVERESTORE)
#include <saverestore.h>
#endif /* SAVERESTORE */
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_types.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_stf.h>
#include <wlc_scan.h>
#include <wlc_tx.h>
#include <wlc_lq.h>
#include <wlc_hw.h>
#include <wlc_assoc.h>
#include <wlc_event_utils.h>
/* msch */
#include <wlc_msch.h>
#include <wlc_chanctxt.h>
#include <wlc_sta.h>
#include <wlc_pm.h>
#include <wlc_wlfc.h>
#include <phy_calmgr_api.h>


/* local aibss debug macros */
//#define AIBSS_DBG
#ifdef AIBSS_DBG
#define FORCE_WAKE
#define AIBSS_ERROR(x)	do { WL_TIMESTAMP(); printf x; } while (0)
#define AIBSS_INFO(x)	do { WL_TIMESTAMP(); printf x; } while (0)
#define AIBSS_TRACE	printf("%s\n", __FUNCTION__)
#define	MSCH_INFO(x)	do { WL_TIMESTAMP(); printf x; } while (0)
#define	MSCH_CBINFO(x)	do { WL_TIMESTAMP(); printf x; } while (0)
#define	AIBSS_INFOC(var, x)	do {if (var) { printf x; var--;}} while (0);
#define AIBSS_PS(x)	do { WL_TIMESTAMP(); printf x; } while (0)
#else
#define AIBSS_ERROR(x) WL_ERROR(x)
#define AIBSS_TRACE
#define AIBSS_INFO(x)
#define	MSCH_INFO(x)
#define	MSCH_CBINFO(x)
#define	AIBSS_INFOC(var, x)
#define AIBSS_PS(x)
#endif /* AIBSS_DBG */

/* temp isolate msch code */
#define MSCH_CODE

/* aibss internal msch state states */
typedef enum {
	WLC_MSCH_OFFCHANNEL = 0x0,
	WLC_MSCH_ON_CHANNEL = 0x1,
	WLC_MSCH_REQ_START = 0x2,
	WLC_MSCH_SLOT_START = 0x3,
	WLC_MSCH_SLOT_END = 0x4,
	WLC_MSCH_PM_NOTIF_PEND = 0x5
} wlc_aibss_msch_state_t;


#define INVALID_SLOT -1
#define IS_VALID_SLOT(slot_id) (slot_id >= 0)

/* msch req update time offset from cur tbtt in us */
#define REQ_DELTA_FT(bp) (((bp) << MSCH_TIME_UNIT_1024US) - PRETBTT_EXT - SLOT_START_OFFSET)

/* iovar table */
enum {
	IOV_AIBSS = 0,			/* enable/disable AIBSS feature */
	IOV_AIBSS_BCN_FORCE_CONFIG = 1,	/* bcn xmit configuration */
	IOV_AIBSS_TXFAIL_CONFIG = 2,
        IOV_AIBSS_IFADD = 3,		/* add AIBSS interface */
	IOV_AIBSS_IFDEL = 4,		/* delete AIBSS interface */
	IOV_AIBSS_PS = 5,
	IOV_AIBSS_DBG = 6
	};

static const bcm_iovar_t aibss_iovars[] = {
	{"aibss", IOV_AIBSS, (IOVF_BSS_SET_DOWN|IOVF_RSDB_SET), 0, IOVT_BOOL, 0},
	{"aibss_bcn_force_config", IOV_AIBSS_BCN_FORCE_CONFIG,
	(IOVF_BSS_SET_DOWN), 0, IOVT_BUFFER, sizeof(aibss_bcn_force_config_t)},
	{"aibss_txfail_config", IOV_AIBSS_TXFAIL_CONFIG,
	(0), 0, IOVT_BUFFER, (OFFSETOF(aibss_txfail_config_t, max_atim_failure))},
	{"aibss_ifadd", IOV_AIBSS_IFADD, 0, 0, IOVT_BUFFER, sizeof(wl_aibss_if_t)},
	{"aibss_ifdel", IOV_AIBSS_IFDEL, 0, 0, IOVT_BUFFER, ETHER_ADDR_LEN},
	{"aibss_ps", IOV_AIBSS_PS, (IOVF_SET_DOWN|IOVF_RSDB_SET), 0, IOVT_BOOL, 0},
#ifdef AIBSS_DBG
	{"aibss_dbg", IOV_AIBSS_DBG, (IOVF_SET_DOWN), 0, IOVT_BOOL, 0},
#endif
	{NULL, 0, 0, 0, 0, 0}
};

typedef struct aibss_scb_info {
	int32	tx_noack_count;
	uint16	no_bcn_counter;
	uint16	bcn_count;
	bool	pkt_pend;
	bool	atim_acked;
	bool	atim_rcvd;
	uint16	atim_failure_count;
	uint16	pkt_pend_count;
	bool	ps_enabled;		/* For PS - non-PS IBSS peer IOT */
	bool	free_scb;
	uint32	last_hi_prio_pkt_time;
	uint8	evt_sent_cnt;		/* txevent limiter counter-- (prevents event flooding) */
} aibss_scb_info_t;

struct wlc_aibss_info {
	int32 scb_handle;		/* SCB CUBBY OFFSET */
};

enum aibss_peer_txfail {
	AIBSS_TX_FAILURE = 0,
	AIBSS_BCN_FAILURE = 1,
	AIBSS_ATIM_FAILURE = 2,
	AIBSS_PEER_FREE = 3
};

/* various AIBSS debug counters (add more items as needed) */
typedef struct aibss_dbg_info {
	uint16 wdt_cnt;			/* print freq div factor */
	int32 pretbtt_2_atim_dt;	/* delta T from tbtt to atim_end interrupt */
	int32 pretbtt_2_tbtt_dt;	/* T from pretbtt 2 tbtt */
	uint32 bcmc_atim_fail_cnt;      /* count failed bcmc atims  */
	uint32 uc_atim_fail_cnt;	/* counter of failed unicast atims (all scbs) */
	uint32 wdt_prev_ts;		/* aibss watchdog prev run timestamp */
	uint16 tbtt_int_cnt;		/* counter of tbtt interrupts over 1 sec (wdt period) */
	uint16 atim_int_cnt;		/* same as above but for atim */
	uint16 bad_tbtt_cnt;		/* counts num of detected bad pretbtt<-->tbtt dT's */
	uint32 rxbcn_cnt;		/* count of beacons from all scbs  */
	uint32 atim_tx_cnt;		/* count of sent atim frames */
	uint32 atim_rx_cnt;		/* total num of atims received from peers */
	uint8  print_cnt;		/* prints in a macro this cnt times then stops */
	uint16 tot_slt_skp;		/* tot num of times aibss slot was skipped */
} aibss_dbg_info_t;
#define AIBSS_DBGI_SZ	(sizeof(aibss_dbg_info_t))   /* dbg counters struct sz */
#define DBG_PRINT_FREQ	10		/* dbg print freq. div. (once per 10 sec) */


/* aibss internal MSCH states associated with msch request */
typedef enum {
	MSCH_ST_IDLE = 0,		/* idle state */
	MSCH_ST_REQ_REGISTER = 1,	/* register aibss request  */
	MSCH_ST_SLOT_CHECK = 2,		/* cancel request  */
	MSCH_ST_SLOT_CANCEL = 3,	/* cancel request  */
} aibss_mschreq_state_t;


/* AIBSS module specific state */
typedef struct wlc_aibss_info_priv {
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_pub_t *pub;
	osl_t *osh;
	uint32	initial_min_bcn_dur;		/* T dur. to check if STA xmitted 1 bcn */
	uint32	bcn_flood_dur;		/* bcn flood T dur. */
	uint32	min_bcn_dur;		/* T dur. to chk bcn tx after flood dur expired */
	struct wl_timer *ibss_timer;		/* aibss state machine timer */
	uint32	ibss_start_time;		/* ticks when the IBSS started */
	uint32 last_txbcnfrm;		/* maintains the prev txbcnfrm count */
	int32 cfg_cubby_handle;		/* BSSCFG cubby offset */
	wlc_bsscfg_t *bsscfg;		/* dedicated bsscfg for IBSS */
	struct pktq	atimq;		/* ATIM queue */
	uint32 wakeup_end_time;		/* ticks when initial wakeup time finished */
	uint32 atim;		/* aibss copy of atim (in case if reinit) */
#ifdef AIBSS_DBG
	aibss_dbg_info_t *dbgi;		/* debug counters */
#endif
	uint32	atim_int_ts;		/* timestamp pretbtt and atim intr  */
	uint32	tbtt_int_ts;		/* tbtt intr ts */
	uint32	tbtt_int_ts_prev;	/* tbtt timestamp from prev bcn interval */
	uint32  pretbtt_int_ts;

	uint32	msch_slot_start_ts;		/* scheduler callback TS */
	bool    switch_ps;
	bool    switch_state;
} wlc_aibss_info_priv_t;

typedef struct  {
	wlc_aibss_info_t aibss_pub;
	wlc_aibss_info_priv_t aibss_priv;
} wlc_aibss_mem_t;

typedef struct aibss_cfg_info {
	uint32 bcn_timeout;		/* dur in seconds to receive 1 bcn */
	uint32 max_tx_retry;		/* no of consecutive no acks to send txfail event */
	bool pm_allowed;
	bool bcmc_pend;
	uint32 max_atim_failure;
#ifdef MSCH_CODE
	wlc_msch_req_handle_t *msch_req_hdl;
	wlc_aibss_msch_state_t msch_state;		/* scheduling states */
	uint32	cbtype_prev;		/* most recent msch cbfn type */
	uint64	msch_ts_attbtt;		/* msch ref time at ucode tbtt interrupt */
	int32	timeslot_id;		/* msch current slot id */
	uint8	slot_skip_cnt;		/* num of aibss slots to skip */
	bool	slot_skp_started;	/*  flag used for skipping N of next slots */
	bool	slot_upd_chk;		/*  set in msch cbfn */
	bool	register_rq;		/* 1- slot update is required */
#endif /* MSCH_CODE */
	uint32 force_wake;		/* bit mask of wake_bits */
} aibss_cfg_info_t;


#define TXFEVT_CNT		4
#define ATIMQ_LEN		MAXSCB		/* ATIM PKT Q length */
#define PRETBTT_EXT		(3 * 1024)	/* pretbtt extension time */
#define SLOT_START_OFFSET	(1* 1024)
#define HPRIO_PKT_DTIME		70		/*  dT threshold between two Hi prio pkts */
#define WLC_AIBSS_INFO_SIZE	(sizeof(wlc_aibss_mem_t))
#define UCODE_IBSS_PS_SUPPORT_VER	(0x3ae0000)		/* BOM version 942.0 */

#define AIBSS_PMQ_INT_THRESH	(0x40)		/* keep thres high to avoid PMQ interrupt */
#define AIBSS_SCB_FREE_BCN_THRESH	(5)		/* 5 seconds of no Beacons */
#define AIBSS_SCB_FREE_ATIM_FAIL_THRESSH	(30)		/* 30 ATIM failures for scb_free */

static uint16 wlc_aibss_info_priv_offset = OFFSETOF(wlc_aibss_mem_t, aibss_priv);

/* module specific states location */
#define WLC_AIBSS_INFO_PRIV(aibss_info) \
	((wlc_aibss_info_priv_t *)((uint8*)(aibss_info) + wlc_aibss_info_priv_offset))
/* access the variables via a macro */
#define WLC_AIBSS_INFO_SCB_HDL(a) ((a)->scb_handle)

static int wlc_aibss_doiovar(void *hdl, uint32 actionid,
        void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif);
static void wlc_aibss_timer(void *arg);
static void wlc_aibss_bss_updn(void *ctx, bsscfg_up_down_event_data_t *evt);
static bool wlc_aibss_validate_chanspec(wlc_info_t *wlc, chanspec_t chanspec);
static void _wlc_aibss_tbtt_impl_cb(void *aibss_info, wlc_tbtt_ent_data_t *notif_data);
static void _wlc_aibss_tbtt_impl(wlc_aibss_info_t *aibss);
static void _wlc_aibss_pretbtt_cb(void *aibss_info, wlc_tbtt_ent_data_t *notif_data);
static void _wlc_aibss_ctrl_pkt_txstatus(wlc_info_t *wlc, void *pkt, uint txstatus);
static bool _wlc_aibss_sendatim(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb);
#if !defined(TXQ_MUX)
static void _wlc_aibss_send_atim_q(wlc_info_t *wlc, struct pktq *q);
#endif
static bool _wlc_aibss_check_pending_data(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, bool force_all);
static void _wlc_aibss_set_active_scb(wlc_aibss_info_t *aibss_info, wlc_bsscfg_t *cfg);
static void wlc_aibss_pretbtt_query_cb(void *ctx, bss_pretbtt_query_data_t *notif_data);
static void _wlc_aibss_data_pkt_txstatus(wlc_info_t *wlc, void *pkt, uint txstatus);
static int wlc_aibss_bcn_parse_ibss_param_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_aibss_scb_init(void *context, struct scb *scb);
static void wlc_aibss_scbfree(wlc_aibss_info_t *aibss_info, struct scb *scb);
static void wlc_aibss_scb_deinit(void *context, struct scb *scb);
static int wlc_aibss_wl_up(void *ctx);
static void wlc_aibss_send_txfail_event(wlc_aibss_info_t *aibss_info,
	struct scb *scb, uint32 evt_sybtype);
static void wlc_aibss_txmod_scb(void *ctx, struct scb *scb, void *pkt, uint prec);
static void wlc_aibss_txmod_on_activate(void *ctx, struct scb *scb);
static void wlc_aibss_txmod_on_deactivate(void *ctx, struct scb *scb);
static uint wlc_aibss_txpktcnt(void *context);
static void wlc_aibss_check_txfail(wlc_aibss_info_t *aibss, wlc_bsscfg_t *cfg, struct scb *scb);
static void wlc_aibss_on_watchdog(void *ctx);
static void wlc_aibss_on_rx_beacon(void *ctx, bss_rx_bcn_notif_data_t *data);
static int wlc_aibss_update_atim(wlc_aibss_info_t *aibss_info, uint16 atim_window, bool is_init);
static int wlc_aibss_switch_ps(wlc_aibss_info_t *aibss_info, bool switch_state);
static void wlc_aibss_assoc_state_clbk(void *arg, bss_assoc_state_data_t *notif_data);
static int wlc_aibss_timeslot_register(wlc_bsscfg_t *bsscfg);

#ifdef MSCH_CODE
/* aibss + sheduler functions & defines */
static int wlc_aibss_sta_msch_clbk(void* handler_ctxt, wlc_msch_cb_info_t *cb_info);
static void wlc_aibss_return_home_channel(wlc_aibss_info_t *aibss_info, wlc_bsscfg_t *cfg);
static int _wlc_aibss_timeslot_register(wlc_aibss_info_t *aibss_info);
static void _wlc_aibss_timeslot_unregister(wlc_aibss_info_t *aibss_info);
static void _wlc_aibss_msch_update(wlc_aibss_info_priv_t *aibss, uint32 delta_usec);
#endif /* MSCH_CODE */


/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/*  aibss tx func */
static txmod_fns_t BCMATTACHDATA(aibss_txmod_upper_fns) = {
    wlc_aibss_txmod_scb,
    wlc_aibss_txpktcnt,
    wlc_aibss_txmod_on_deactivate,
    wlc_aibss_txmod_on_activate
};


#ifdef AIBSS_DBG
/*  debug print aibss related shmem, and wlc regsisters and variables */
static void aibss_dbgext_info(const char *msg, wlc_aibss_info_priv_t *aibss)
{
	wlc_info_t *wlc = aibss->wlc;
	wlc_bsscfg_t *bsscfg = aibss->bsscfg;
	aibss_cfg_info_t *cfg_cubby;
	uint32 mc;
	uint16 mp2p_hps, p2p_shm_base;
	int bss = wlc_mcnx_BSS_idx(wlc->mcnx, bsscfg);
	int ridx = 0;

	cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);

	AIBSS_INFO(("----- AIBSS wake related vars: ------\n"
		" force_wake:%d, bcmc_pend:%d\n",
		cfg_cubby->force_wake, cfg_cubby->bcmc_pend));

	AIBSS_INFO(("---- mcnx P2P_BSS shm block: -------\n"));
	while (ridx < M_P2P_BSS_BLK_SZ) {
		uint16 shm = wlc_mcnx_read_shm(wlc->mcnx, M_P2P_BSS(wlc, bss, ridx));
		AIBSS_INFO(("shmem M_P2P_BSS(%d,%d)[%x]:%x\n",
			bss, ridx, M_P2P_BSS(wlc, bss, ridx), shm));
		ridx++;
	}

	/* hps bits */
	p2p_shm_base = wlc_bmac_read_shm(wlc->hw, M_P2P_BLK_PTR(wlc)) << 1;
	mp2p_hps = wlc_mcnx_read_shm(wlc->mcnx, M_P2P_HPS_OFFSET(wlc));
	AIBSS_INFO(("P2P_BSS: p2p_shm_base:%x M_P2P_HPS:%x,\n",  p2p_shm_base, mp2p_hps));

	/* mac control register */
	mc = R_REG(wlc->osh, &wlc->regs->maccontrol);

	AIBSS_INFO(("----- MAC_CTRL:{0x:%x},bits: -------\n"
		" WAKE:%d,HPS:%d,AP:%d,INFRA:%d,PSM_RUN:%d,DISCARD_PMQ:%d "
		"mac_cmd:%x, tsf_cfprep:0x%x\n",
		mc, ((MCTL_WAKE & mc) != 0), ((MCTL_HPS & mc) != 0),
		((MCTL_AP & mc) != 0), ((MCTL_INFRA & mc) != 0),
		((MCTL_PSM_RUN & mc) != 0), ((MCTL_DISCARD_PMQ & mc) != 0),
		R_REG(wlc->osh, &wlc->regs->maccommand), R_REG(wlc->osh, &wlc->regs->tsf_cfprep)));

	AIBSS_INFO(("-------- WLC wake bits --------:\n"
		" wake:%d, PMawakebcn:%d, SCAN_IN_PROGRESS():%d,"
		" AS_IN_PROGRESS():%d\n"
		"PMpending:%d, PSpoll:%d,check_for_unaligned_tbtt:%d,apsd_sta_usp:%d,\n"
		"pm2_radio_shutoff_pending:%d,monitor:%d,qvalid:%d\n",
		wlc->wake, wlc->PMawakebcn,
		SCAN_IN_PROGRESS(wlc->scan), AS_IN_PROGRESS(wlc),
		wlc->PMpending, wlc->PSpoll, wlc->check_for_unaligned_tbtt,
		wlc->apsd_sta_usp,
		wlc->pm2_radio_shutoff_pending, wlc->monitor, wlc->qvalid));
	AIBSS_INFO(("\n"));
	/*  do single print of timestamps  @ atim_end intr */
	aibss->dbgi->print_cnt = 8;
}
#endif /* AIBSS_DBG */

static void
wlc_aibss_txmod_scb(void *ctx, struct scb *scb, void *pkt, uint prec)
{
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)ctx;

	wlc_aibss_tx_pkt(aibss_info, scb, pkt);
	/* call next tx func in scb->tx_path[AIBSS].next_tx_fn() chain */
	SCB_TX_NEXT(TXMOD_AIBSS, scb, pkt, WLC_PRIO_TO_PREC(PKTPRIO(pkt)));
}

/* Return the count of all the packets being held by aibss module */
static uint
wlc_aibss_txpktcnt(void *context)
{
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)context;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_bsscfg_t *cfg = aibss->bsscfg;
	struct scb_iter scbiter;
	struct scb *scb;
	int pktcnt_tot = 0;

	AIBSS_TRACE;
	/*
	 * --- total pktcnt pending in aibsscfg ----
	 */
	FOREACH_BSS_SCB(aibss->wlc->scbstate, &scbiter, cfg, scb) {
		aibss_scb_info_t *scb_info = SCB_CUBBY((scb), aibss_info->scb_handle);
		pktcnt_tot += scb_info->pkt_pend_count;
	}

	return pktcnt_tot;
}

wlc_aibss_info_t *
BCMATTACHFN(wlc_aibss_attach)(wlc_info_t *wlc)
{
	wlc_aibss_info_t *aibss_info;
	wlc_aibss_info_priv_t *aibss;

	AIBSS_TRACE;

	/* sanity checks */
	if (!(aibss_info = (wlc_aibss_info_t *)MALLOCZ(wlc->osh, WLC_AIBSS_INFO_SIZE))) {
		AIBSS_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	aibss->wlc = wlc;
	aibss->pub = wlc->pub;
	aibss->osh = wlc->osh;

#ifdef AIBSS_DBG
	/* alloc various aibss debug counters TODO: "wl dump aibss" */
	if (!(aibss->dbgi = MALLOCZ(wlc->osh, AIBSS_DBGI_SZ))) {
		AIBSS_ERROR(("wl%d: %s: OOM dbgi, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	aibss->dbgi->wdt_prev_ts = OSL_SYSUPTIME();
#endif

	/* create IBSS timer for xmit extra beacons */
	if ((aibss->ibss_timer =
		wl_init_timer(wlc->wl, wlc_aibss_timer, aibss_info, "ibss_timer")) == NULL) {
		AIBSS_ERROR(("wl%d: wl_init_timer for AIBSS failed\n", wlc->pub->unit));
		goto fail;
	}

	/* reserve cubby in the bsscfg container for private data */
	if ((aibss->cfg_cubby_handle = wlc_bsscfg_cubby_reserve(wlc, sizeof(aibss_cfg_info_t),
		NULL, NULL, NULL, (void *)aibss_info)) < 0) {
		AIBSS_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* reserve cubby in the scb container to monitor per SCB tx stats */
	if ((aibss_info->scb_handle = wlc_scb_cubby_reserve(aibss->wlc,
		sizeof(aibss_scb_info_t), wlc_aibss_scb_init, wlc_aibss_scb_deinit,
		NULL, aibss_info)) < 0) {
		AIBSS_ERROR(("wl%d: %s: wlc_scb_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* bsscfg up/down callback */
	if (wlc_bsscfg_updown_register(wlc, wlc_aibss_bss_updn, aibss_info) != BCME_OK) {
		AIBSS_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register pretbtt query callback */
	if (wlc_bss_pretbtt_query_register(wlc, wlc_aibss_pretbtt_query_cb, aibss) != BCME_OK) {
		AIBSS_ERROR(("wl%d: %s: wlc_bss_pretbtt_query_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register packet callback function */
	if (wlc_pcb_fn_set(wlc->pcb, 1, WLF2_PCB2_AIBSS_CTRL, _wlc_aibss_ctrl_pkt_txstatus)
		!= BCME_OK) {
		AIBSS_ERROR(("wl%d: %s wlc_pcb_fn_set() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register packet callback function */
	if (wlc_pcb_fn_set(wlc->pcb, 0, WLF2_PCB1_AIBSS_DATA, _wlc_aibss_data_pkt_txstatus)
		!= BCME_OK) {
		AIBSS_ERROR(("wl%d: %s wlc_pcb_fn_set() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, aibss_iovars, "aibss",
		aibss_info, wlc_aibss_doiovar, wlc_aibss_on_watchdog, wlc_aibss_wl_up, NULL)) {
		AIBSS_ERROR(("wl%d: AIBSS wlc_module_register() failed\n", wlc->pub->unit));
		goto fail;
	}

	/* bcn atim processing */
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_IBSS_PARMS_ID,
	                         wlc_aibss_bcn_parse_ibss_param_ie, aibss_info) != BCME_OK) {
		AIBSS_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, atim in bcn\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (wlc_bss_assoc_state_register(wlc, wlc_aibss_assoc_state_clbk, aibss_info)
		!= BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bss_assoc_state_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	wlc_txmod_fn_register(wlc->txmodi, TXMOD_AIBSS, aibss_info, aibss_txmod_upper_fns);

	/* Initialize atim pktq */
	pktq_init(&aibss->atimq, 1, ATIMQ_LEN);

	/* register beacon rx notif. callback */
	wlc_bss_rx_bcn_register(wlc, wlc_aibss_on_rx_beacon, aibss_info);

	return aibss_info;

fail:
	MODULE_DETACH(aibss_info, wlc_aibss_detach);

	return NULL;
}

void
BCMATTACHFN(wlc_aibss_detach)(wlc_aibss_info_t *aibss_info)
{
	wlc_aibss_info_priv_t *aibss;
	wlc_info_t	*wlc;

	if (!aibss_info)
		return;

	aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc = aibss->wlc;

	/* un-register beacon rx notif. callback */
	wlc_bss_rx_bcn_unregister(wlc, wlc_aibss_on_rx_beacon, aibss_info);

	wlc_bss_assoc_state_unregister(wlc, wlc_aibss_assoc_state_clbk, aibss_info);


	if (aibss->ibss_timer != NULL) {
		wl_free_timer(wlc->wl, aibss->ibss_timer);
		aibss->ibss_timer = NULL;
	}

	wlc_module_unregister(aibss->pub, "aibss", aibss_info);
#ifdef AIBSS_DBG
	if (aibss->dbgi) {
		MFREE(wlc->osh, aibss->dbgi, AIBSS_DBGI_SZ);
	}
#endif
	MFREE(wlc->osh, aibss_info, WLC_AIBSS_INFO_SIZE);
}


static int
wlc_aibss_doiovar(void *hdl, uint32 actionid,
	void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif)
{
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)hdl;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);

	int32 int_val = 0;
	bool bool_val;
	uint32 *ret_uint_ptr;
	int err = 0;
	wlc_bsscfg_t *bsscfg;

	AIBSS_TRACE;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	ret_uint_ptr = (uint32 *)a;

	bsscfg = wlc_bsscfg_find_by_wlcif(aibss->wlc, wlcif);
	ASSERT(bsscfg != NULL);

	switch (actionid) {

		case IOV_GVAL(IOV_AIBSS):
			*ret_uint_ptr = AIBSS_ENAB(aibss->pub);
			break;

		case IOV_SVAL(IOV_AIBSS):
			if (aibss->pub->_aibss != bool_val) {
				uint flags = 0;
				wlc_bsscfg_type_t cfgtype =
					{BSSCFG_TYPE_GENERIC, BSSCFG_SUBTYPE_NONE};
				aibss->pub->_aibss = bool_val;
				if (bool_val) {
				/* switch switch bsscfg to aibss */
					cfgtype.type = BSSCFG_TYPE_AIBSS;
				}
				wlc_bsscfg_reinit(aibss->wlc, bsscfg, &cfgtype, flags);
			}
			break;

		case IOV_GVAL(IOV_AIBSS_BCN_FORCE_CONFIG):{
			aibss_bcn_force_config_t *bcn_config = (aibss_bcn_force_config_t *)a;

			store16_ua(&bcn_config->version, AIBSS_BCN_FORCE_CONFIG_VER_0);
			store16_ua(&bcn_config->len, sizeof(aibss_bcn_force_config_t));
			store32_ua(&bcn_config->initial_min_bcn_dur, aibss->initial_min_bcn_dur);
			store32_ua(&bcn_config->bcn_flood_dur, aibss->bcn_flood_dur);
			store32_ua(&bcn_config->min_bcn_dur, aibss->min_bcn_dur);
			break;
		}

		case IOV_SVAL(IOV_AIBSS_BCN_FORCE_CONFIG):{
			aibss_bcn_force_config_t *bcn_config = (aibss_bcn_force_config_t *)p;

			aibss->initial_min_bcn_dur = load32_ua(&bcn_config->initial_min_bcn_dur);
			aibss->bcn_flood_dur = load32_ua(&bcn_config->bcn_flood_dur);
			aibss->min_bcn_dur = load32_ua(&bcn_config->min_bcn_dur);
			break;
		}

		case IOV_GVAL(IOV_AIBSS_TXFAIL_CONFIG): {
			aibss_txfail_config_t *txfail_config = (aibss_txfail_config_t *)a;
			aibss_cfg_info_t	*cfg_cubby;

			if (!BSSCFG_IBSS(bsscfg)) {
				err = BCME_ERROR;
				break;
			}

			cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);
			store16_ua((uint8 *)&txfail_config->version, AIBSS_TXFAIL_CONFIG_CUR_VER);
			store16_ua((uint8 *)&txfail_config->len, sizeof(aibss_txfail_config_t));
			store32_ua((uint8 *)&txfail_config->bcn_timeout, cfg_cubby->bcn_timeout);
			store32_ua((uint8 *)&txfail_config->max_tx_retry, cfg_cubby->max_tx_retry);
			if (alen >= sizeof(aibss_txfail_config_t)) {
				store32_ua((uint8 *)&txfail_config->max_atim_failure,
					cfg_cubby->max_atim_failure);
			}
			break;
		}

		case IOV_SVAL(IOV_AIBSS_TXFAIL_CONFIG): {
			aibss_txfail_config_t *txfail_config = (aibss_txfail_config_t *)a;
			aibss_cfg_info_t	*cfg_cubby;
			uint16 version = load16_ua(&txfail_config->version);
			uint16 len = load16_ua(&txfail_config->len);

			if (!BSSCFG_IBSS(bsscfg)) {
				err = BCME_ERROR;
				break;
			}

			if (version > AIBSS_TXFAIL_CONFIG_CUR_VER) {
				err = BCME_VERSION;
				break;
			}

			cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);
			cfg_cubby->bcn_timeout = load32_ua((uint8 *)&txfail_config->bcn_timeout);
			cfg_cubby->max_tx_retry = load32_ua((uint8 *)&txfail_config->max_tx_retry);
			if (version == AIBSS_TXFAIL_CONFIG_VER_1 &&
				len == sizeof(aibss_txfail_config_t)) {
				cfg_cubby->max_atim_failure =
					load32_ua((uint8 *)&txfail_config->max_atim_failure);
			}
			else if (BSSCFG_IS_AIBSS_PS_ENAB(bsscfg)) {
				cfg_cubby->max_atim_failure = WLC_AIBSS_DEFAULT_ATIM_FAILURE;
			}
			break;
		}
		case IOV_SVAL(IOV_AIBSS_IFADD): {
			wl_aibss_if_t *aibss_if = (wl_aibss_if_t*)a;
			wlc_info_t *wlc = aibss->wlc;
			wlc_bsscfg_t *new_bsscfg;
			wlc_bsscfg_t *primary_cfg = wlc_bsscfg_primary(wlc);
			int idx;
			uint32 flags = WLC_BSSCFG_HW_BCN | WLC_BSSCFG_HW_PRB |
				WLC_BSSCFG_TX_SUPR_ENAB | WLC_BSSCFG_PSINFO;
			wlc_bsscfg_type_t type = {BSSCFG_TYPE_AIBSS, BSSCFG_SUBTYPE_NONE};

			/* return failure if
			 *	1) primary interface is running is AP/IBSS mode
			 *	2) a dedicated IBSS interface already exists
			 *	3) TODO: any other AP/GO interface exists
			 */
			if (BSSCFG_AIBSS(primary_cfg) || aibss->bsscfg != NULL ||
				BSSCFG_AP(primary_cfg)) {
				err = BCME_BADARG;
				break;
			}

			/* bsscfg with the given MAC address exists */
			if (wlc_bsscfg_find_by_hwaddr(wlc, &aibss_if->addr) != NULL) {
				err = BCME_BADARG;
				break;
			}

			/* validate channel spec */
			if (!wlc_aibss_validate_chanspec(wlc, aibss_if->chspec)) {
				err = BCME_BADARG;
				break;
			}

			/* try to allocate one bsscfg for IBSS */
			if ((idx = wlc_bsscfg_get_free_idx(wlc)) == -1) {
				AIBSS_ERROR(("wl%d: no free bsscfg\n", wlc->pub->unit));
				err = BCME_ERROR;
				break;
			}
			if ((new_bsscfg = wlc_bsscfg_alloc(wlc, idx, &type,
					flags, &aibss_if->addr)) == NULL) {
				AIBSS_ERROR(("wl%d: cannot create bsscfg\n", wlc->pub->unit));
				err = BCME_ERROR;
				break;
			}
			if ((err = wlc_bsscfg_init(wlc, new_bsscfg)) != BCME_OK) {
				AIBSS_ERROR(("wl%d: failed to init bsscfg\n", wlc->pub->unit));
				wlc_bsscfg_free(wlc, new_bsscfg);
				break;
			}
		}
		case IOV_SVAL(IOV_AIBSS_IFDEL): {
			wlc_info_t *wlc = aibss->wlc;
			wlc_bsscfg_t *del_bsscfg = wlc_bsscfg_find_by_hwaddr(wlc, a);
			if (del_bsscfg == NULL || aibss->bsscfg == NULL) {
				err = BCME_BADARG;
				break;
			}
			/* can't delete current IOVAR processing bsscfg */
			if (del_bsscfg == bsscfg) {
				err = BCME_BADARG;
				break;
			}
			if (del_bsscfg->enable)
				wlc_bsscfg_disable(wlc, del_bsscfg);
			wlc_bsscfg_free(wlc, del_bsscfg);
			aibss->bsscfg = NULL;
			break;
		}
		case IOV_GVAL(IOV_AIBSS_PS):
			*ret_uint_ptr = (int32)aibss->wlc->pub->_aibss_ps;

			break;
		case IOV_SVAL(IOV_AIBSS_PS): {
			uint flags = 0;
			wlc_bsscfg_type_t cfgtype =
				{BSSCFG_TYPE_AIBSS, BSSCFG_SUBTYPE_NONE};
			aibss->wlc->pub->_aibss_ps = bool_val;
			/*   need to reinit bsscfg with apps support */
			if (bool_val) {
				flags |= WLC_BSSCFG_PSINFO;
			}
			wlc_bsscfg_reinit(aibss->wlc, bsscfg, &cfgtype, flags);
			break;
		}
#ifdef AIBSS_DBG
		case IOV_GVAL(IOV_AIBSS_DBG):
		case IOV_SVAL(IOV_AIBSS_DBG):
			*ret_uint_ptr = 0;
			/*  print debug counters */
			aibss_dbgext_info(__FUNCTION__, aibss);
			break;
#endif
		default:
			err = BCME_UNSUPPORTED;
			break;
	}
	return err;
}


/* bsscfg up/down */
static void
wlc_aibss_bss_updn(void *ctx, bsscfg_up_down_event_data_t *evt)
{
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)ctx;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	aibss_cfg_info_t *cfg_cubby;
	wlc_info_t *wlc = aibss->wlc;
	wlc_bsscfg_t *cfg;

	ASSERT(evt != NULL);

	cfg = evt->bsscfg;
	ASSERT(cfg != NULL);

	if (!AIBSS_ENAB(wlc->pub) || !BSSCFG_IBSS(cfg))
		return;

	AIBSS_INFO(("%s aibss_updn: %s\n", __FUNCTION__, evt->up?">>> UP":" <<< DOWN"));

	cfg_cubby = BSSCFG_CUBBY(cfg, aibss->cfg_cubby_handle);
	if (evt->up) {
		uint32 val = 0;
		uint32 mask = (MCTL_INFRA | MCTL_DISCARD_PMQ);

		if (wlc_tbtt_ent_fn_add(wlc->tbtt, cfg, _wlc_aibss_pretbtt_cb,
			_wlc_aibss_tbtt_impl_cb, wlc->aibss_info) != BCME_OK) {
			AIBSS_ERROR(("wl%d: %s: wlc_tbtt_ent_fn_add() failed\n",
			  wlc->pub->unit, __FUNCTION__));
			return;
		}

		aibss->bsscfg = cfg;
		cfg->type = BSSCFG_TYPE_AIBSS; /* TAG the cfg */
		/* km to skip replay check for bcmc frames */
		cfg->WPA_auth = WPA_AUTH_NONE;

		/* allow PS aibss to sleep if no bcns rcvd  */
		cfg->cxn->ign_bcn_lost_det = TRUE;
		/* n/u for aibss, but requred to enter PS mode */
		cfg->dtim_programmed = TRUE;

		/* Set MCTL_INFRA for non-primary AIBSS interface to support concurrent mode */
		if (cfg != wlc_bsscfg_primary(wlc)) {
			val |= MCTL_INFRA;
		}

		/* DisabLe PMQ entries to avoid uCode updating pmq entry on  rx packets */
		val |= MCTL_DISCARD_PMQ;

		/* Update PMQ Interrupt threshold to avoid MI_PMQ interrupt */
		wlc_write_shm(wlc, M_PMQADDINT_THSD(wlc), AIBSS_PMQ_INT_THRESH);

		wlc_ap_ctrl(wlc, TRUE, cfg, -1);
		wlc_mctrl(wlc, mask, val);

		/* Set IBSS and GO bit to enable beaconning */
#ifdef WLMCNX
		if (MCNX_ENAB(wlc->pub)) {
			int bss = wlc_mcnx_BSS_idx(wlc->mcnx, cfg);
			wlc_mcnx_mac_suspend(wlc->mcnx);
			wlc_mcnx_write_shm(wlc->mcnx, M_P2P_BSS_ST(wlc, bss),
				(M_P2P_BSS_ST_GO | M_P2P_BSS_ST_IBSS));
			wlc_mcnx_mac_resume(wlc->mcnx);
		}
#endif /* WLMCNX */

		/* if configured, start timer to check for bcn xmit */
		if (aibss->initial_min_bcn_dur && aibss->bcn_flood_dur &&
			aibss->min_bcn_dur) {

			/* IBSS interface up, start the timer to monitor the
			 * beacon transmission
			 */
			aibss->ibss_start_time = OSL_SYSUPTIME();
			wl_add_timer(wlc->wl, aibss->ibss_timer, aibss->initial_min_bcn_dur,
				FALSE);
			if (AIBSS_BSS_PS_ENAB(cfg)) {
				cfg_cubby->force_wake |= WLC_AIBSS_FORCE_WAKE_INITIAL;
				aibss->wakeup_end_time =
					OSL_SYSUPTIME() + WLC_AIBSS_INITIAL_WAKEUP_PERIOD*1000;
			}

			/* Disable PM till the flood beacon is complete */
			wlc_set_pmoverride(cfg, TRUE);
		}
#ifdef WME
		if ((cfg->flags & SCB_WMECAP)) {
			if (cfg->bcmc_scb)
				cfg->bcmc_scb->flags |= SCB_WMECAP;
		}
#endif /* WME */
	}
	else {
		if (wlc_tbtt_ent_fn_del(wlc->tbtt, cfg, _wlc_aibss_pretbtt_cb,
			_wlc_aibss_tbtt_impl_cb, wlc->aibss_info) != BCME_OK) {
			AIBSS_ERROR(("wl%d: %s: wlc_tbtt_ent_fn_del() failed\n",
			  wlc->pub->unit, __FUNCTION__));
		}

#ifdef MSCH_CODE
		_wlc_aibss_timeslot_unregister(aibss_info);
#endif /* MSCH_CODE */

		aibss->bsscfg = NULL;

		/* IBSS not enabled, stop monitoring the link */
		if (aibss->ibss_start_time) {
			aibss->ibss_start_time = 0;
			wl_del_timer(wlc->wl, aibss->ibss_timer);
		}
	}
}

void
wlc_aibss_timer(void *arg)
{
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)arg;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;
	uint32	timer_dur = aibss->min_bcn_dur;
	wlc_bsscfg_t *cfg = aibss->bsscfg;
	aibss_cfg_info_t	*cfg_cubby = BSSCFG_CUBBY(cfg, aibss->cfg_cubby_handle);
	struct scb_iter scbiter;
	struct scb *scb;

	WL_TRACE(("wl%d: wlc_ibss_timer", wlc->pub->unit));

	if (!wlc->pub->up || !AIBSS_ENAB(wlc->pub)) {
		return;
	}
	/* Delayed SCB Free
	 * SCB is freed if there is no bcn for some time. The scb free is delayed
	 * if there are any pending pkts in the FIFO for the PEER.
	 */
	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
		aibss_scb_info_t	*scb_info = SCB_CUBBY((scb), aibss_info->scb_handle);
		if (scb_info->free_scb) {
			wlc_aibss_scbfree(aibss_info, scb);
		}
	}

	if (aibss->ibss_start_time &&
		((OSL_SYSUPTIME() - aibss->ibss_start_time) < aibss->bcn_flood_dur)) {
		timer_dur = aibss->initial_min_bcn_dur;
	}
	else if (cfg->pm->PM_override) {
		wlc_set_pmoverride(cfg, FALSE);
	}

	if (AIBSS_BSS_PS_ENAB(cfg) && aibss->wakeup_end_time &&
		(OSL_SYSUPTIME() >= aibss->wakeup_end_time)) {
		cfg_cubby->force_wake &= ~WLC_AIBSS_FORCE_WAKE_INITIAL;
		aibss->wakeup_end_time = 0;
	}
	/* Get the number of beacons sent */
	wlc_statsupd(wlc);

	/* If no beacons sent from prev timer, send one */
	if ((MCSTVAR(wlc->pub, txbcnfrm) - aibss->last_txbcnfrm) == 0)
	{
		/* Not sent enough bcn, send bcn in next TBTT
		 * even if we recv bcn from a peer IBSS STA
		 */
		wlc_bmac_mhf(wlc->hw, MHF1, MHF1_FORCE_SEND_BCN,
			MHF1_FORCE_SEND_BCN, WLC_BAND_ALL);
	}
	else {
		wlc_bmac_mhf(wlc->hw, MHF1, MHF1_FORCE_SEND_BCN, 0, WLC_BAND_ALL);

		/* update the beacon counter */
		aibss->last_txbcnfrm = MCSTVAR(wlc->pub, txbcnfrm);
	}

	if (aibss->switch_ps) {
		if (wlc_aibss_switch_ps(aibss_info, aibss->switch_state) == BCME_OK) {
			aibss->switch_ps = FALSE;
		}
	}

	/* ADD TIMER */
	wl_add_timer(wlc->wl, aibss->ibss_timer, timer_dur, FALSE);
}

void
wlc_aibss_check_txfail(wlc_aibss_info_t *aibss_info, wlc_bsscfg_t *cfg, struct scb *scb)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;
	aibss_cfg_info_t	*cfg_cubby = BSSCFG_CUBBY(cfg, aibss->cfg_cubby_handle);
	aibss_scb_info_t	*scb_info = SCB_CUBBY((scb), aibss_info->scb_handle);

	AIBSS_INFO(("%s scb:%p bcn_to:%d cfg_bcn_to:%d\n",
		__FUNCTION__, OSL_OBFUSCATE_BUF(scb), scb_info->no_bcn_counter,
		cfg_cubby->bcn_timeout));

	/* scb bcn timeout check if configured */
	if (cfg_cubby->bcn_timeout &&
		(scb_info->no_bcn_counter >= cfg_cubby->bcn_timeout)) {
		wlc_bss_mac_event(wlc, cfg, WLC_E_AIBSS_TXFAIL, &scb->ea,
		                  WLC_E_STATUS_FAIL, AIBSS_BCN_FAILURE,
		                  0, NULL, 0);

		/* Reset the counters */
		scb_info->no_bcn_counter = 0;
		wlc_aibss_scbfree(aibss_info, scb);
	}

	/*  not configured use default  */
	if (cfg_cubby->bcn_timeout == 0 &&
		scb_info->no_bcn_counter > AIBSS_SCB_FREE_BCN_THRESH) {
		wlc_aibss_scbfree(aibss_info, scb);

	}
}

static bool wlc_aibss_validate_chanspec(wlc_info_t *wlc, chanspec_t chanspec)
{
	/* use default chanspec */
	if (chanspec == 0)
		return TRUE;

	/* validate chanspec */
	if (wf_chspec_malformed(chanspec) || !wlc_valid_chanspec_db(wlc->cmi, chanspec) ||
		wlc_radar_chanspec(wlc->cmi, chanspec))
		return FALSE;

	/* If mchan not enabled, don't allow IBSS on different channel */
	if (!MCHAN_ENAB(wlc->pub) && wlc->pub->associated && chanspec != wlc->home_chanspec) {
		return FALSE;
	}

	return TRUE;
}


void wlc_aibss_tbtt(wlc_aibss_info_t *aibss_info)
{
	_wlc_aibss_tbtt_impl(aibss_info);
}

bool wlc_aibss_sendpmnotif(wlc_aibss_info_t *aibss_info, wlc_bsscfg_t *bsscfg,
	ratespec_t rate_override, int prio, bool track)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;
	aibss_cfg_info_t	*cfg_cubby;

	if (bsscfg == NULL || bsscfg != aibss->bsscfg)
		return FALSE;

	cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);

	/* Let wlc_aibss_atim_window_end take care of putting the device to sleep */
	cfg_cubby->pm_allowed = FALSE;
	wlc_set_ps_ctrl(bsscfg);

	/* Clear PM states since IBSS PS is not depend on it */
	bsscfg->pm->PMpending = FALSE;
	wlc_pm_pending_complete(wlc);

	return TRUE;
}

void _wlc_aibss_ctrl_pkt_txstatus(wlc_info_t *wlc, void *pkt, uint txstatus)
{
	struct scb *scb;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(wlc->aibss_info);
	wlc_bsscfg_t *bsscfg = aibss->bsscfg;
	aibss_cfg_info_t	*cfg_cubby;
	aibss_scb_info_t *scb_info;

	AIBSS_TRACE;

	scb = WLPKTTAGSCBGET(pkt);
	if (scb == NULL)
		return;
	bsscfg = SCB_BSSCFG(scb);

	if (bsscfg == NULL || bsscfg != aibss->bsscfg)
		return;

	scb_info = SCB_CUBBY((scb), wlc->aibss_info->scb_handle);

	 if (SCB_ISMULTI(scb)) {
		/* its bcast atim  */
		if (txstatus != 0) {
		/* for bcast atims we assume status 0x1000 is OK */
			scb_info->atim_acked = TRUE;
		} else {
			AIBSS_INFO(("%s BCAST txstatus:%x, atim:%d, bcmc:%d\n",
				__FUNCTION__, txstatus, TXPKTPENDGET(wlc, TX_ATIM_FIFO),
				TXPKTPENDGET(wlc, TX_BCMC_FIFO)));
#ifdef AIBSS_DBG
			aibss->dbgi->bcmc_atim_fail_cnt++;
#endif
		}
		return;
	}

	cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);

	if (txstatus & TX_STATUS_ACK_RCV) {
		AIBSS_PS(("%s():wl%d: ATIM frame %p sent to "MACF" in PS=%d\n",
			__FUNCTION__, wlc->pub->unit, OSL_OBFUSCATE_BUF(pkt),
			ETHER_TO_MACF(scb->ea), SCB_PS(scb)));

		scb_info->atim_acked = TRUE;
		scb_info->atim_failure_count = 0;
		/* scb is back or it's been Ok,
		 * cancel scb free(if was ordered) & reinit txfail limiter
		 */
		scb_info->evt_sent_cnt = TXFEVT_CNT;
		scb_info->free_scb = FALSE;

		/* Clear PS state if we receive an ACK for atim frame */
		if (SCB_PS(scb)) {
			wlc_apps_process_ps_switch(wlc, &scb->ea, PS_SWITCH_OFF);
		}
	}
	else {
		AIBSS_PS(("%s():wl%d: ATIM frame %p sent (no ACK) to "MACF"\n",
			__FUNCTION__, wlc->pub->unit, OSL_OBFUSCATE_BUF(pkt),
			ETHER_TO_MACF(scb->ea)));
		scb_info->atim_failure_count++;
#ifdef AIBSS_DBG
		aibss->dbgi->uc_atim_fail_cnt++;
#endif
		if (cfg_cubby->max_atim_failure &&
			scb_info->atim_failure_count >= cfg_cubby->max_atim_failure) {
			wlc_aibss_send_txfail_event(wlc->aibss_info, scb, AIBSS_ATIM_FAILURE);
		}

	}

#if !defined(TXQ_MUX)
	if (!pktq_empty(&aibss->atimq)) {
		_wlc_aibss_send_atim_q(wlc, &aibss->atimq);
	}
#endif /* TXQ_MUX */
}

/*  pre-tbtt MAC interrupt callback  */
void _wlc_aibss_pretbtt_cb(void *aibss_info, wlc_tbtt_ent_data_t *notif_data)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	aibss->pretbtt_int_ts = OSL_SYSUPTIME();

	/*  check pending traffic & send atims */
	_wlc_aibss_tbtt_impl(aibss_info);
}

/*  tbtt MAC intr  */
void _wlc_aibss_tbtt_impl_cb(void *aibss_info, wlc_tbtt_ent_data_t *notif_data)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);

	ASSERT(aibss && aibss->wlc);

	aibss->tbtt_int_ts_prev = aibss->tbtt_int_ts;
	aibss->tbtt_int_ts = OSL_SYSUPTIME();

#ifdef	AIBSS_DBG
	if ((aibss->tbtt_int_ts - aibss->tbtt_int_ts_prev) < 102) {
		AIBSS_ERROR(("tbtt dT:%d\n",
			aibss->tbtt_int_ts - aibss->tbtt_int_ts_prev));
	}

#endif

#ifdef MSCH_CODE
	{
		aibss_cfg_info_t *cfg_cubby = BSSCFG_CUBBY(aibss->bsscfg, aibss->cfg_cubby_handle);

		if (cfg_cubby->register_rq) {
			/* very 1st slot registration is done here */
			_wlc_aibss_timeslot_register(aibss_info);
			cfg_cubby->register_rq = FALSE;
		}
		else  {
			/* ILP osc clock drift correction  */
			int32 Tdifr = aibss->msch_slot_start_ts - aibss->pretbtt_int_ts;
			if (cfg_cubby->slot_upd_chk && !cfg_cubby->slot_skip_cnt &&
				((Tdifr >= 0) || /* its between pretbtt and tbtt */
				((Tdifr < -4) && (Tdifr > -102)))) {
				/* we update next slot start time to fire ~2ms before aibss pretbtt
				 */
				_wlc_aibss_msch_update(aibss,
					REQ_DELTA_FT(aibss->bsscfg->current_bss->beacon_period));
				AIBSS_INFO(("tbtt_intr: did slot upd: msch-pretbtt"
					" was:%d pretbtt-tbtt:%d, REQ_DELTA_FT(bp)=%d\n",
					Tdifr, aibss->tbtt_int_ts - aibss->pretbtt_int_ts,
					REQ_DELTA_FT(aibss->bsscfg->current_bss->beacon_period)));
			}
			cfg_cubby->slot_upd_chk = FALSE;
		}
	}
#endif /* MSCH_CODE */
}

/*  process aibss MAC pre-tbtt interrupt  */
void _wlc_aibss_tbtt_impl(wlc_aibss_info_t *aibss_info)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;
	wlc_bsscfg_t *bsscfg = aibss->bsscfg;
	aibss_cfg_info_t	*cfg_cubby;
	uint16 mhf_val;
	uint32 pat_hi, pat_lo;
	struct ether_addr eaddr;
	char eabuf[ETHER_ADDR_STR_LEN];
	volatile uint16 *pmqctrlstatus = (volatile uint16 *)&wlc->regs->pmqreg.w.pmqctrlstatus;
	volatile uint32 *pmqhostdata = (volatile uint32 *)&wlc->regs->pmqreg.pmqhostdata;
#ifdef WLMCHAN
	chanspec_t chspec;
#endif
	BCM_REFERENCE(eabuf);
	BCM_REFERENCE(eaddr);

	ASSERT(pmqctrlstatus);
	ASSERT(pmqhostdata);

	if (bsscfg == NULL) {
		return;
	}

	cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);

#ifdef AIBSS_DBG
	aibss->dbgi->tbtt_int_cnt++;
#endif
	/* covers rare case when atim(s) got q'ed up from prev prev bcn interval */
	if (TXPKTPENDGET(wlc, TX_ATIM_FIFO)) {
		AIBSS_INFO(("%s tbtt: ATIM fifo has %d pkts flush it\n",
			__FUNCTION__, TXPKTPENDGET(wlc, TX_ATIM_FIFO)));
		wlc_sync_txfifo(wlc, bsscfg->wlcif->qi, TX_FIFO_BITMAP(TX_ATIM_FIFO), FLUSHFIFO);
		wlc_txq_pktq_flush(wlc, &aibss->atimq);
	}

#ifdef MSCH_CODE
	/* exit if off channel  */
	if ((cfg_cubby->msch_state != WLC_MSCH_ON_CHANNEL) ||
		(cfg_cubby->slot_skp_started)) {
#ifdef AIBSS_DBG
		aibss->dbgi->tot_slt_skp++;
#endif
		AIBSS_INFO(("tbtt intr: SLOT skipped or not On Chan, skip_cnt:%d"
			" msch_state:%d\n", cfg_cubby->slot_skip_cnt, cfg_cubby->msch_state));
		return;
	}
#endif /* MSCH_CODE */

	/* covers rare case when atim(s) got q'ed up from prev prev bcn interval */
	if (TXPKTPENDGET(wlc, TX_ATIM_FIFO)) {
		AIBSS_INFO(("%s !!! ATIM fifo has %d pkts flush it\n",
			__FUNCTION__, TXPKTPENDGET(wlc, TX_ATIM_FIFO)));
		wlc_bmac_tx_fifo_sync(wlc->hw, TX_FIFO_BITMAP(TX_ATIM_FIFO), FLUSHFIFO);
		pktq_flush(wlc->osh, &aibss->atimq, TRUE);
	}

	/* read entries until empty or pmq exeeding limit */
	while ((R_REG(wlc->osh, pmqhostdata)) & PMQH_NOT_EMPTY) {
		pat_lo = R_REG(wlc->osh, &wlc->regs->pmqpatl);
		pat_hi = R_REG(wlc->osh, &wlc->regs->pmqpath);
		eaddr.octet[5] = (pat_hi >> 8)  & 0xff;
		eaddr.octet[4] =  pat_hi	& 0xff;
		eaddr.octet[3] = (pat_lo >> 24) & 0xff;
		eaddr.octet[2] = (pat_lo >> 16) & 0xff;
		eaddr.octet[1] = (pat_lo >> 8)  & 0xff;
		eaddr.octet[0] =  pat_lo	& 0xff;

		AIBSS_PS(("wl%d.%d: ATIM failed, pmq entry added for %s\n", wlc->pub->unit,
			WLC_BSSCFG_IDX(bsscfg), bcm_ether_ntoa(&eaddr, eabuf)));
	}

	/* Clear all the PMQ entry before sending ATIM frames */
	W_REG(wlc->osh, pmqctrlstatus, PMQH_DEL_MULT);

	/* Check for BCN Flood status */
	mhf_val =  wlc_bmac_mhf_get(wlc->hw, MHF1, WLC_BAND_AUTO);
	if (mhf_val & MHF1_FORCE_SEND_BCN) {

		wlc_statsupd(wlc);

		/* check if we have sent atleast 1 bcn and clear
		 * the force bcn bit
		 */
		if (MCSTVAR(wlc->pub, txbcnfrm) != aibss->last_txbcnfrm) {
			wlc_bmac_mhf(wlc->hw, MHF1, MHF1_FORCE_SEND_BCN, 0, WLC_BAND_ALL);
		}
	}
#ifdef WLMCHAN
	/* if off channel (scan is in progress) don't  try sending any data or bcmc */
	chspec = wlc_mchan_get_chanspec(wlc->mchan, bsscfg);
	if (SCAN_IN_PROGRESS(wlc->scan) &&
		(chspec != wlc->chanspec)) {
		AIBSS_INFO(("%s:scan:1 aibss ch:%x, scan:%x, defer all TX\n",
			__FUNCTION__, chspec, wlc->chanspec));
		return;
	}
#endif
	if (AIBSS_PS_ENAB(wlc->pub)) {
		cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);

		cfg_cubby->pm_allowed = FALSE;
		wlc_set_ps_ctrl(bsscfg);

		/* BCMC traffic */
		if (TXPKTPENDGET(wlc, TX_BCMC_FIFO)) {
			cfg_cubby->bcmc_pend = TRUE;
			_wlc_aibss_sendatim(wlc, bsscfg, WLC_BCMCSCB_GET(wlc, bsscfg));
		}

		/* Check pending packets for peers and sendatim to them */
		_wlc_aibss_set_active_scb(aibss_info, bsscfg);
		_wlc_aibss_check_pending_data(wlc, bsscfg, FALSE);
	}

	return;
}

void
wlc_aibss_ps_start(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	struct scb *bcmc_scb;

	AIBSS_PS(("%s: Entering\n", __FUNCTION__));

	/* Enable PS for BCMC scb */
	bcmc_scb = WLC_BCMCSCB_GET(wlc, cfg);
	ASSERT(bcmc_scb);
	bcmc_scb->PS = TRUE;

	/* In case of AIBSS PS on primary interface host might enable
	 * PM mode well before the IBSS PS is enabled. PMEnabled will not
	 * be set untill IBSS PS is set so update correct pm state.
	 */
	wlc_set_pmstate(cfg, (cfg->pm->PM != PM_OFF));

	/* PS for other SCB shall be enabled after first ATIM window */
}

void
wlc_aibss_ps_stop(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	struct scb_iter scbiter;
	struct scb *scb, *bcmc_scb;

	AIBSS_PS(("%s: Entering\n", __FUNCTION__));

	/* Assume all sta in PS and pull them out one by one */
	if (!BSSCFG_IBSS(cfg)) {
		return;
	}

	bcmc_scb = WLC_BCMCSCB_GET(wlc, cfg);
	ASSERT(bcmc_scb);
	bcmc_scb->PS = FALSE;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
		if (SCB_PS(scb)) {
			wlc_apps_process_ps_switch(wlc, &scb->ea, PS_SWITCH_OFF);
		}
	}
}

/* ATIM window end MAC interrupt handler  */
void wlc_aibss_atim_window_end(wlc_info_t *wlc)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(wlc->aibss_info);
	wlc_bsscfg_t *bsscfg = aibss->bsscfg;
	struct scb *scb;
	struct scb_iter scbiter;
	bool be_awake = FALSE;
	aibss_cfg_info_t *cfg_cubby;
	aibss_scb_info_t *bcmc_scb_info;


	if (aibss->bsscfg == NULL || !AIBSS_BSS_PS_ENAB(aibss->bsscfg)) {
		AIBSS_ERROR(("wl%d: %s: bsscfg:%p ATIM enabled in non PS mode\n",
			wlc->pub->unit, __FUNCTION__, OSL_OBFUSCATE_BUF(aibss->bsscfg)));
		return;
	}

	aibss->atim_int_ts = OSL_SYSUPTIME();


	bcmc_scb_info = SCB_CUBBY((WLC_BCMCSCB_GET(wlc, bsscfg)), wlc->aibss_info->scb_handle);
	cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);

#ifdef AIBSS_DBG
{
	aibss_dbg_info_t *dbgi = aibss->dbgi;

	dbgi->atim_int_cnt++;
	dbgi->pretbtt_2_atim_dt = aibss->atim_int_ts - aibss->pretbtt_int_ts;
	dbgi->pretbtt_2_tbtt_dt = aibss->tbtt_int_ts - aibss->pretbtt_int_ts;

	/* detect pretbtt vs tbtt interval malfunctions */
	if ((dbgi->pretbtt_2_tbtt_dt < 0) ||
		(dbgi->pretbtt_2_tbtt_dt > ((PRETBTT_EXT/1000)+3))) {
		dbgi->bad_tbtt_cnt++;
	}
}
#endif /* AIBSS_DBG */


#ifdef MSCH_CODE
	AIBSS_INFOC(aibss->dbgi->print_cnt, ("ATIM_END isr:"
		" tbtt-pretbtt:%d, atim_end-pretbtt:%d, atim-msch:%d"
		" cbtype_prev:0x%x msch_state:%d, skp_slot:%d\n",
		(aibss->tbtt_int_ts - aibss->pretbtt_int_ts),
		(aibss->atim_int_ts - aibss->pretbtt_int_ts),
		(aibss->atim_int_ts - aibss->msch_slot_start_ts),
		cfg_cubby->cbtype_prev,	cfg_cubby->msch_state, cfg_cubby->slot_skip_cnt));

	/*   bail out if slot is skipped or OFF channel  */
	if (cfg_cubby->slot_skip_cnt && (cfg_cubby->slot_skp_started)) {

		if (--cfg_cubby->slot_skip_cnt == 0)
			cfg_cubby->slot_skp_started = FALSE;
		/* nothing to do, slot was marked as skipped before tbtt */
		return;
	}

	if (cfg_cubby->slot_skip_cnt) {
		/* next N slots will be skipped */
		cfg_cubby->slot_skp_started = TRUE;
	}

	if (cfg_cubby->msch_state != WLC_MSCH_ON_CHANNEL) {
		AIBSS_INFO(("atim_end intr, TS:%d OFF chan, exit\n", aibss->atim_int_ts));
		return;
	}
#endif /* MSCH_CODE */

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		aibss_scb_info_t *scb_info = SCB_CUBBY((scb), wlc->aibss_info->scb_handle);
		uint8 scb_ps_on = PS_SWITCH_PMQ_ENTRY;

		if ((scb_info->atim_acked) ||
			scb_info->atim_rcvd || !scb_info->ps_enabled) {
			scb_ps_on = PS_SWITCH_OFF;
		}

		/* call the ps_switch fn if the SCB PS state does not match */
		if (SCB_PS(scb) != (scb_ps_on != PS_SWITCH_OFF)) {
			wlc_apps_process_ps_switch(wlc, &scb->ea, scb_ps_on);
		}

		scb_info->atim_rcvd = FALSE;
		scb_info->atim_acked = FALSE;

		if (!SCB_PS(scb)) {
			be_awake = TRUE;
		}

		if (scb_info->free_scb == TRUE && SCB_PS(scb) == TRUE &&
			scb_info->atim_failure_count > 0) {
			be_awake = TRUE;
		}
	}


	/* if we have no uncast atims sent/rcvd */
	/*  stay awake only if bcast atim was sent or if forced */
	if (!be_awake && (bcmc_scb_info->atim_acked ||
		cfg_cubby->force_wake)) {
		be_awake = TRUE;
	}
	bcmc_scb_info->atim_acked = FALSE;

	if (!be_awake) {

#ifdef MSCH_CODE
	/* cancel current scheduler slot */
	if (cfg_cubby->msch_req_hdl) {
		wlc_msch_timeslot_cancel(wlc->msch_info,
			cfg_cubby->msch_req_hdl, cfg_cubby->timeslot_id);
	}
#endif /* MSCH_CODE */

#if !defined(FORCE_WAKE)
		cfg_cubby->pm_allowed = TRUE;
		wlc_set_ps_ctrl(bsscfg);

#if defined(WLMCNX)
		if (MCNX_ENAB(wlc->pub)) {
			uint16 st;
			int bss = wlc_mcnx_BSS_idx(wlc->mcnx, bsscfg);
			/* clear the wake bit */
			st = wlc_mcnx_read_shm(wlc->mcnx, M_P2P_BSS_ST(wlc, bss));
			st &= ~M_P2P_BSS_ST_WAKE;
			wlc_mcnx_write_shm(wlc->mcnx, M_P2P_BSS_ST(wlc, bss), st);
		}

#endif /* WLMCNX */

#endif /* FORCE_WAKE */
	}

	if (TXPKTPENDGET(wlc, TX_ATIM_FIFO)) {
		AIBSS_ERROR(("%s atim end: failed, ATIM fifo[%d] BCMC fifo[%d]\r\n",
			__FUNCTION__, TXPKTPENDGET(wlc, TX_ATIM_FIFO),
			TXPKTPENDGET(wlc, TX_BCMC_FIFO)));
		wlc_sync_txfifo(wlc, bsscfg->wlcif->qi, TX_FIFO_BITMAP(TX_ATIM_FIFO), FLUSHFIFO);
		wlc_txq_pktq_flush(wlc, &aibss->atimq);
	}
	/* clear bcmc pending for this window */
	cfg_cubby->bcmc_pend = FALSE;
}

#if !defined(TXQ_MUX)
void
_wlc_aibss_send_atim_q(wlc_info_t *wlc, struct pktq *q)
{
	void *p;
	int prec = 0;
	struct scb *scb;


	/* Send all the enq'd pkts that we can.
	 * Dequeue packets with precedence with empty HW fifo only
	 */
	 while ((p = pktq_deq(q, &prec))) {
		wlc_pkttag_t *pkttag = WLPKTTAG(p);

		scb = WLPKTTAGSCBGET(p);
		if ((pkttag->flags & WLF_TXHDR) == 0) {
			wlc_mgmt_ctl_d11hdrs(wlc, p, scb, TX_ATIM_FIFO, 0);
		}

		if (!(uint)TXAVAIL(wlc, TX_ATIM_FIFO)) {
			/* Mark precedences related to this FIFO, unsendable */
			WLC_TX_FIFO_CLEAR(wlc, TX_ATIM_FIFO);
			break;
		}

		/* NewTXQ implementation */
		wlc_low_txq_enq(wlc->txqi, wlc->active_queue->low_txq,
			TX_ATIM_FIFO, p, 1);

		if (!txq_hw_stopped(wlc->active_queue->low_txq, TX_ATIM_FIFO))
		{
			txq_hw_fill(wlc->txqi, wlc->active_queue->low_txq, TX_ATIM_FIFO);
		}
	}
	/* wlc->in_send_q = FALSE; */
}
#endif /* TXQ_MUX */

/**
 * Pktq filter function to set pkt_pend on each scb that has a pkt in the queue.
 * Used by _wlc_aibss_set_active_scb().
 */
static pktq_filter_result_t
wlc_aibss_set_pending_filter(void* ctx, void* pkt)
{
	int32 scb_handle = (int32)(uintptr)ctx;
	struct scb *scb;
	aibss_scb_info_t *scb_info;

	scb = WLPKTTAGSCBGET(pkt);
	scb_info = SCB_CUBBY(scb, scb_handle);
	scb_info->pkt_pend = TRUE;

	return PKT_FILTER_NOACTION;
}

/* Set aibss_scb_info_t.pkt_pend TRUE for each SCB represented in the txq */
static void
_wlc_aibss_set_active_scb(wlc_aibss_info_t *aibss_info, wlc_bsscfg_t *cfg)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	struct pktq *txq = WLC_GET_TXQ(cfg->wlcif->qi);

	wlc_txq_pktq_filter(aibss->wlc, txq,
	        wlc_aibss_set_pending_filter,
		(void*)(uintptr)aibss_info->scb_handle);
}

void
wlc_aibss_ps_process_atim(wlc_info_t *wlc, struct ether_addr *ea)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(wlc->aibss_info);
	struct scb *scb;
	aibss_scb_info_t *scb_info;

	if (aibss->bsscfg == NULL) {
		AIBSS_ERROR(("wl%d: ATIM received from peer "MACF""
			" when AIBSS bsscfg is not up\n", wlc->pub->unit, ETHERP_TO_MACF(ea)));
		return;
	}

	if ((scb = wlc_scbfind(wlc, aibss->bsscfg, ea)) == NULL) {
		AIBSS_ERROR(("wl%d.%d: ATIM received from unknown peer "MACF"\n", wlc->pub->unit,
			WLC_BSSCFG_IDX(aibss->bsscfg), ETHERP_TO_MACF(ea)));
		return;
	}

	AIBSS_PS(("wl%d.%d: ATIM received\n", wlc->pub->unit, WLC_BSSCFG_IDX(aibss->bsscfg)));

#ifdef AIBSS_DBG
	aibss->dbgi->atim_rx_cnt++;
#endif
	scb_info = SCB_CUBBY((scb), wlc->aibss_info->scb_handle);
	scb_info->atim_rcvd = TRUE;

	if (SCB_PS(scb)) {
		AIBSS_PS(("wl%d.%d: scb:%p PS-OFF\n", wlc->pub->unit,
			WLC_BSSCFG_IDX(aibss->bsscfg), OSL_OBFUSCATE_BUF(scb)));
		wlc_apps_process_ps_switch(wlc, &scb->ea, PS_SWITCH_OFF);
	}
	wlc_set_ps_ctrl(aibss->bsscfg);
}

bool
_wlc_aibss_sendatim(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb)
{
#if defined(TXQ_MUX)
	return TRUE;
#else
	void *p;
	uint8 *pbody;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(wlc->aibss_info);

	AIBSS_PS(("%s: Entering\n", __FUNCTION__));

	ASSERT(wlc->pub->up);

	/* HANDLE BCMC ATIM too */
	if (pktq_full(&aibss->atimq)) {
		AIBSS_ERROR(("wl%d.%d: ATIM queue overflow \n", wlc->pub->unit,
			WLC_BSSCFG_IDX(bsscfg)));
		return FALSE;
	}

	if ((p = wlc_frame_get_mgmt(wlc, FC_ATIM, &scb->ea, &bsscfg->cur_etheraddr,
		&bsscfg->current_bss->BSSID, 0, &pbody)) == NULL) {
		AIBSS_ERROR(("wl%d.%d: Unable to get pkt for ATIM frame\n", wlc->pub->unit,
			WLC_BSSCFG_IDX(bsscfg)));
		return FALSE;
	}

	WLPKTTAG(p)->flags |= WLF_PSDONTQ;
	WLF2_PCB2_REG(p, WLF2_PCB2_AIBSS_CTRL);
	WLPKTTAG(p)->shared.packetid = 0;

	WLPKTTAGSCBSET(p, scb);
	WLPKTTAGBSSCFGSET(p, WLC_BSSCFG_IDX(bsscfg));
	PKTSETLEN(wlc->osh, p, DOT11_MGMT_HDR_LEN);


#ifdef AIBSS_DBG
	aibss->dbgi->atim_tx_cnt++;
#endif
	pktq_penq(&aibss->atimq, 0, p);
	_wlc_aibss_send_atim_q(wlc, &aibss->atimq);
	return TRUE;
#endif /* TXQ_MUX */
}

bool
_wlc_aibss_check_pending_data(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, bool force_all)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		uint psq_len;
		aibss_scb_info_t *scb_info = SCB_CUBBY((scb), wlc->aibss_info->scb_handle);

		psq_len = wlc_apps_psq_len(wlc, scb);

		/* Send ATIM for the peer which are already in PS */
		if ((SCB_PS(scb) && psq_len) || scb_info->pkt_pend ||
			scb_info->pkt_pend_count ||
			// Stay awake if prev HI prio pkt was seen < HPRIO_PKT_DTIME ms ago
			((OSL_SYSUPTIME() - scb_info->last_hi_prio_pkt_time) < HPRIO_PKT_DTIME)) {
			scb_info->pkt_pend = FALSE;
			if (scb_info->ps_enabled &&
				!scb_info->free_scb) {
				_wlc_aibss_sendatim(wlc, bsscfg, scb);
			}
		}
		else if (!SCB_PS(scb) && scb_info->ps_enabled) {
			/* Enable PS right away */
			wlc_apps_process_ps_switch(wlc, &scb->ea, TRUE);
		}
	}

	return TRUE;
}

bool
wlc_aibss_pm_allowed(wlc_aibss_info_t *aibss_info, wlc_bsscfg_t *bsscfg)
{
	aibss_cfg_info_t	*cfg_cubby;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);

	if (!BSSCFG_IBSS(bsscfg))
		return TRUE;

	cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);
	return cfg_cubby->pm_allowed;
}

/* return the required pretbtt value */
void
wlc_aibss_pretbtt_query_cb(void *ctx, bss_pretbtt_query_data_t *notif_data)
{
	wlc_bsscfg_t *cfg;

	ASSERT(notif_data != NULL);

	cfg = notif_data->cfg;
	ASSERT(cfg != NULL);

	if (!BSSCFG_IS_AIBSS_PS_ENAB(cfg)) {
		AIBSS_INFO(("%s PS is not enabled\n", __FUNCTION__));
		return;
	}

	notif_data->pretbtt += PRETBTT_EXT;
}

void
wlc_aibss_send_txfail_event(wlc_aibss_info_t *aibss_info, struct scb *scb, uint32 evt_sybtype)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;
	wlc_bsscfg_t *bsscfg = aibss->bsscfg;
	aibss_scb_info_t *scb_info = SCB_CUBBY(scb, aibss_info->scb_handle);

	// send only limited number of events, then trigger scb free
	if (scb_info->evt_sent_cnt) {
		wlc_bss_mac_event(wlc, bsscfg, WLC_E_AIBSS_TXFAIL, &scb->ea,
			WLC_E_STATUS_FAIL, evt_sybtype, 0, NULL, 0);
		if (--scb_info->evt_sent_cnt == 0) {
			// try free scb, if no timer will do it later
			scb_info->free_scb = TRUE;
		}
	}
}

void
_wlc_aibss_data_pkt_txstatus(wlc_info_t *wlc, void *pkt, uint txstatus)
{
	struct scb *scb;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(wlc->aibss_info);
	wlc_bsscfg_t *bsscfg = aibss->bsscfg;
	aibss_scb_info_t *scb_info;
	aibss_cfg_info_t *cfg_cubby;

	scb = WLPKTTAGSCBGET(pkt);
	if (scb == NULL)
		return;
	bsscfg = SCB_BSSCFG(scb);

	scb_info = SCB_CUBBY((scb), wlc->aibss_info->scb_handle);
	if (bsscfg == NULL || bsscfg != aibss->bsscfg || SCB_ISMULTI(scb))
		return;

	cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);

	if (scb_info->pkt_pend_count)
		scb_info->pkt_pend_count--;

	/* Ignore TXSTATUS from PKTFREE */
	if (txstatus == 0)
		return;

	if (txstatus & TX_STATUS_ACK_RCV) {
		scb_info->tx_noack_count = 0;
		/* scb is back or it'sbeen Ok,
		 * cancel scb free(if was ordered) & reinit txfail limiter
		 */
		scb_info->evt_sent_cnt = TXFEVT_CNT;
		scb_info->free_scb = FALSE;
	}
	else {
		scb_info->tx_noack_count++;

		if (cfg_cubby->max_tx_retry &&
			(scb_info->tx_noack_count >= cfg_cubby->max_tx_retry)) {
				wlc_aibss_send_txfail_event(wlc->aibss_info, scb, AIBSS_TX_FAILURE);
		}
	} /* NOACK */
	return;
}

void
wlc_aibss_stay_awake(wlc_aibss_info_t * aibss_info)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);

	if (aibss->bsscfg == NULL)
		return;
	/* SR WAR: uCode requests ALP clock if there are data in the FIFO
	 * so Wake-up uCode and put it back to sleep to enable ALP clock
	 */
	aibss->wlc->wake = TRUE;
	wlc_set_ps_ctrl(aibss->bsscfg);
	aibss->wlc->wake = FALSE;
	wlc_set_ps_ctrl(aibss->bsscfg);
}

void
wlc_aibss_tx_pkt(wlc_aibss_info_t * aibss_info, struct scb *scb, void *pkt)
{
	aibss_scb_info_t *scb_info;
	void *head = NULL, *tail = NULL;
	void *n = NULL;

	BCM_REFERENCE(head);
	BCM_REFERENCE(tail);

	ASSERT(scb);
	ASSERT(pkt);

	if (SCB_ISMULTI(scb)) {
		// get pkt callback for bcast mcast
		WLF2_PCB1_REG(pkt, WLF2_PCB1_AIBSS_DATA);
	} else	{
		/* Enable packet callback */
		WLF2_PCB1_REG(pkt, WLF2_PCB1_AIBSS_DATA);
		scb_info = SCB_CUBBY((scb), aibss_info->scb_handle);

		/* pkt could be chained, update the correct packet count */
		FOREACH_CHAINED_PKT(pkt, n) {
			scb_info->pkt_pend_count++;
			if (IS_HI_PRIO_PKT(pkt)) {
				scb_info->last_hi_prio_pkt_time = OSL_SYSUPTIME();
			}
			PKTCENQTAIL(head, tail, pkt);
		}
	}
}

int
wlc_aibss_scb_init(void *context, struct scb *scb)
{
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)context;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	aibss_cfg_info_t *cfg_cubby = BSSCFG_CUBBY(aibss->bsscfg, aibss->cfg_cubby_handle);
	aibss_scb_info_t *scb_info = SCB_CUBBY(scb, aibss_info->scb_handle);
	int err = BCME_OK;

	AIBSS_TRACE;

	if (scb->bsscfg == NULL || !BSSCFG_IBSS(scb->bsscfg)) {
		goto exit;
	}

	AIBSS_INFO(("%s:>>>-init->>> scb:%p, IS_MULTI:%d, ["MACF"],  bsscfgs: aibss:%p, bcmc:%p\n",
		__FUNCTION__, OSL_OBFUSCATE_BUF(scb), SCB_ISMULTI(scb), ETHER_TO_MACF(scb->ea),
		OSL_OBFUSCATE_BUF(aibss->bsscfg), WLC_BCMCSCB_GET(aibss->wlc, scb->bsscfg)));

	if (scb->bsscfg != aibss->bsscfg)
		goto exit;

	/*  configure txmod for this scb */
	wlc_txmod_config(aibss->wlc->txmodi, scb, TXMOD_AIBSS);

	/* init txfail event limit cnt */
	scb_info->evt_sent_cnt = TXFEVT_CNT;

	if (!AIBSS_BSS_PS_ENAB(aibss->bsscfg))
		goto exit;

	/* Enable PS by default */
	scb_info->ps_enabled = TRUE;

	/* Force wake the device, host might start
	 * sending the packets to the peer immediately
	 */
	cfg_cubby->pm_allowed = FALSE;
	wlc_aibss_stay_awake(aibss_info);

	if (AIBSS_PS_ENAB(aibss->pub)) {
		/* put the device in PS State, tx is allowed only after
		 * ATIM window
		 */
		wlc_apps_process_ps_switch(aibss->wlc, &scb->ea, TRUE);

	}

exit:
	return err;
}

void
wlc_aibss_scb_deinit(void *context, struct scb *scb)
{
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)context;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	aibss_cfg_info_t *cfg_cubby = BSSCFG_CUBBY(aibss->bsscfg, aibss->cfg_cubby_handle);
	aibss_scb_info_t *scb_info;
	struct scb_iter scbiter;
	struct scb *scbi;

	AIBSS_INFO(("%s,<<<-deinit-<<< scb:%p\n", __FUNCTION__, (scb)));

	if (!BSSCFG_IBSS(scb->bsscfg) || scb->bsscfg != aibss->bsscfg) {
		return;
	}

	/* un-CONFIGURE SCB TXMOD */
	wlc_txmod_unconfig(aibss->wlc->txmodi, scb, TXMOD_AIBSS);

	if (AIBSS_BSS_PS_ENAB(aibss->bsscfg) &&
		(cfg_cubby->force_wake & WLC_AIBSS_FORCE_WAKE_NON_PS_PEER)) {
		FOREACH_BSS_SCB(aibss->wlc->scbstate, &scbiter, aibss->bsscfg, scbi) {
			if (scbi == scb) {
				continue;
			}
			scb_info = SCB_CUBBY((scbi), aibss_info->scb_handle);
			if (!scb_info->ps_enabled) {
				return;
			}
		}
		cfg_cubby->force_wake &= ~WLC_AIBSS_FORCE_WAKE_NON_PS_PEER;
	}
}

int
wlc_aibss_bcn_parse_ibss_param_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)ctx;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	aibss_scb_info_t *scb_info;
	aibss_cfg_info_t *cfg_cubby;

	if (!BSSCFG_IBSS(cfg)) {
		return BCME_OK;
	}

	/* IBSS parameters */
	if (data->ie != NULL &&
	    data->ie[TLV_LEN_OFF] >= DOT11_MNG_IBSS_PARAM_LEN) {
	    struct scb *scb  = ftpparm->bcn.scb;
		uint16 atim_window = ltoh16_ua(&data->ie[TLV_BODY_OFF]);

		if (AIBSS_BSS_PS_ENAB(cfg) && (atim_window == 0)) {
			cfg_cubby = BSSCFG_CUBBY(aibss->bsscfg, aibss->cfg_cubby_handle);
			scb_info = SCB_CUBBY((scb), aibss_info->scb_handle);

			scb_info->ps_enabled = FALSE;
			/* Disable AIBSS PS */
			cfg_cubby->force_wake |= WLC_AIBSS_FORCE_WAKE_NON_PS_PEER;
		}
	}
	return BCME_OK;
}

void
wlc_aibss_set_wake_override(wlc_aibss_info_t *aibss_info, uint32 wake_reason, bool set)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	aibss_cfg_info_t *cfg_cubby;

	if (aibss->bsscfg == NULL) {
		return;
	}

	cfg_cubby = BSSCFG_CUBBY(aibss->bsscfg, aibss->cfg_cubby_handle);

	if (set) {
		cfg_cubby->force_wake |= wake_reason;
	}
	else {
		cfg_cubby->force_wake &= ~wake_reason;
	}
}

static void
wlc_aibss_scbfree(wlc_aibss_info_t *aibss_info, struct scb *scb)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	aibss_scb_info_t *scb_info = SCB_CUBBY(scb, aibss_info->scb_handle);
	/* SCB PSINFO cubby (accessed wlc_apps_psq_len) in is allocated only when AIBSS PS is
	 * enabled
	 */
	int ps_q = (AIBSS_PS_ENAB(aibss->pub)) ? wlc_apps_psq_len(aibss->wlc, scb) : 0;

	AIBSS_INFO(("%s, scb:%p\n", __FUNCTION__, OSL_OBFUSCATE_BUF(scb)));

	if (scb_info->pkt_pend_count &&
			scb_info->pkt_pend_count > ps_q) {
		scb_info->free_scb = TRUE;
	}
	else {
		/* This event and reason AIBSS_PEER_FREE, it used to clear PCIE flowring at DHD. */
#ifdef BCMPCIEDEV
		if (BCMPCIEDEV_ENAB()) {
			wlc_bss_mac_event(aibss->wlc, aibss->bsscfg, WLC_E_AIBSS_TXFAIL, &scb->ea,
					WLC_E_STATUS_FAIL, AIBSS_PEER_FREE, 0, NULL, 0);
		}
#endif /* BCMPCIEDEV */
		wlc_scbfree(aibss->wlc, scb);
	}
}


/* aibss module on wl up  */
#define WL_WAKE_OVERRIDE_ALL  \
	(WLC_WAKE_OVERRIDE_PHYREG | WLC_WAKE_OVERRIDE_MACSUSPEND | \
	WLC_WAKE_OVERRIDE_CLKCTL | WLC_WAKE_OVERRIDE_TXFIFO | \
	WLC_WAKE_OVERRIDE_FORCEFAST | WLC_WAKE_OVERRIDE_4335_PMWAR)
static int
wlc_aibss_wl_up(void *ctx)
{
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)ctx;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;

	if (!aibss_info) {
	/* don't do wl down + wl reinit !, we dont support it */
		AIBSS_ERROR(("%s aibss_info is NULL\n", __FUNCTION__));
		return BCME_NOMEM;
	}

	wlc_aibss_update_atim(aibss_info, wlc->default_bss->atim_window, TRUE);

	return BCME_OK;
}

/* turning on/off aibss PS for RMC */
void
wlc_aibss_ps_update_request(wlc_aibss_info_t *aibss_info, bool set)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;

	if (aibss->switch_ps && aibss->switch_state == set) {
		AIBSS_ERROR(("%s: Duplicated PS switch request\n", __FUNCTION__));
	}

	if (set && (wlc->cfg->current_bss->atim_window > 0)) {
		AIBSS_ERROR(("%s: No need to switch PS. PS is on already\n", __FUNCTION__));
		return;
	}
	if (!set && wlc->cfg->current_bss->atim_window == 0) {
		AIBSS_ERROR(("%s: No need to switch PS. PS is off already\n", __FUNCTION__));
		return;
	}

	aibss->switch_ps = TRUE;
	aibss->switch_state = set;
}

static int
wlc_aibss_switch_ps(wlc_aibss_info_t *aibss_info, bool switch_state)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;
	wlc_bsscfg_t *cfg = aibss->bsscfg;
	uint16 target_atim;
	int err = BCME_OK, exist_pktq = 0;

	/* Check current atim window size */
	if ((switch_state && wlc->cfg->current_bss->atim_window > 0) ||
		(!switch_state && wlc->cfg->current_bss->atim_window == 0)) {
		/* Same as previous state, no need to switch */
		err = BCME_OK;
		goto done;
	}

	/* Check the FIFOs for pending packets */
	if (!pktq_empty(WLC_GET_TXQ(cfg->wlcif->qi)) || TXPKTPENDTOT(wlc)) {
		AIBSS_INFO(("%s: Queue is not empty. Suspend tx.\n", __FUNCTION__));
		wlc_tx_suspend(wlc);
		exist_pktq = 1;
	}

	if (switch_state) {
		target_atim = wlc->default_bss->atim_window;
	}
	else {
		target_atim = 0;
	}

	if (BSSCFG_STA(cfg) && !cfg->BSS && AIBSS_ENAB(wlc->pub)) {
		AIBSS_INFO(("%s: Switch PS\n", __FUNCTION__));

		wlc_txq_pktq_flush(wlc, &aibss->atimq);

		if (target_atim > 0) {
			wlc_aibss_ps_start(wlc, cfg);
		} else {
			wlc_aibss_ps_stop(wlc, cfg);
		}

		err = wlc_aibss_update_atim(aibss_info, target_atim, FALSE);

		if (err) {
			/* atim update fail */
			err = BCME_ERROR;
			goto done;
		}

		/* set CFP duration */
		W_REG(wlc->osh, &wlc->regs->tsf_cfpmaxdur32, target_atim);

		wlc->pub->_aibss_ps = switch_state;
	}
done:
	if (exist_pktq) {
		wlc_tx_resume(wlc);
		AIBSS_INFO(("%s: Queue is not empty. Resume tx.\n", __FUNCTION__));
	}
	return err;
}

static int
wlc_aibss_update_atim(wlc_aibss_info_t *aibss_info, uint16 atim_window, bool is_init)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_bsscfg_t *bsscfg = aibss->bsscfg;
	aibss_cfg_info_t *cfg_cubby;
	wlc_info_t *wlc = aibss->wlc;
	uint32 old_l, old_h;
	uint bcnint, bcnint_us;
	uint32 val = 0;
	uint32 d11hw_atim = R_REG(wlc->osh, &wlc->regs->tsf_cfpmaxdur32);
	uint32 mc = R_REG(wlc->osh, &wlc->regs->maccontrol);
	uint32 mask = (MCTL_INFRA | MCTL_DISCARD_PMQ);
	uint32 cfg_atim;

	BCM_REFERENCE(mc);

	wlc->cfg->current_bss->atim_window = atim_window;
	cfg_atim = atim_window;

	AIBSS_INFO(("%s ai_inf:%p, ai_priv:%p\n",
		__FUNCTION__, OSL_OBFUSCATE_BUF(aibss_info), OSL_OBFUSCATE_BUF(aibss)));

	if (cfg_atim == d11hw_atim || (!bsscfg))
		return BCME_OK; /* do nothing on wlc load */

	AIBSS_ERROR(("%s aibss BH occured, cur_bss_atim:%d, d11hw_atim:%d mactrl:%x\n",
		__FUNCTION__, cfg_atim, d11hw_atim, mc));

	AIBSS_INFO(("MCTL: WAKE:%d,HPS:%d,AP:%d,INFRA:%d,PSM_RUN:%d,DISCARD_PMQ:%d\n",
	((MCTL_WAKE & mc) != 0), ((MCTL_HPS & mc) != 0),
	((MCTL_AP & mc) != 0), ((MCTL_INFRA & mc) != 0),
	((MCTL_PSM_RUN & mc) != 0), ((MCTL_DISCARD_PMQ & mc) != 0)));

	/* in case of BH hw atim is cleared, need to set it again */
	W_REG(wlc->osh, &wlc->regs->tsf_cfpmaxdur32, cfg_atim);

	/* on BH it me be set, keep it cleared */
	wlc->check_for_unaligned_tbtt = 0;

	/* DisabLe PMQ entries to avoid uCode updating pmq entry on  rx packets */
	val |= MCTL_DISCARD_PMQ;

	/* Update PMQ Interrupt threshold to avoid MI_PMQ interrupt */
	wlc_write_shm(wlc, M_PMQADDINT_THSD(wlc), AIBSS_PMQ_INT_THRESH);

	wlc_ap_ctrl(wlc, TRUE, bsscfg, -1);
	wlc_mctrl(wlc, mask, val);

#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub)) {
		/* Set IBSS and GO bit to enable beaconning */
		int bss = wlc_mcnx_BSS_idx(wlc->mcnx, bsscfg);
		wlc_mcnx_mac_suspend(wlc->mcnx);
		wlc_mcnx_write_shm(wlc->mcnx, M_P2P_BSS_ST(wlc, bss),
			(M_P2P_BSS_ST_GO | M_P2P_BSS_ST_IBSS));
		wlc_mcnx_mac_resume(wlc->mcnx);
	}
#endif /* WLMCNX */

	/* just in case clear all wlc_hw MCTL_AWAKE override bits */
	wlc_ucode_wake_override_clear(wlc->hw, WL_WAKE_OVERRIDE_ALL);

	if (is_init) {
		cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);
		cfg_cubby->force_wake |= WLC_AIBSS_FORCE_WAKE_INITIAL;
		aibss->wakeup_end_time =
			OSL_SYSUPTIME() + WLC_AIBSS_INITIAL_WAKEUP_PERIOD*500;
	} else {
		wlc_sync_txfifo(wlc, bsscfg->wlcif->qi, BITMAP_SYNC_ALL_TX_FIFOS, FLUSHFIFO);
	}

	/* reset tsf, get the atim_end interrupts rolling */
	bcnint = aibss->bsscfg->current_bss->beacon_period;
	bcnint_us = ((uint32)bcnint << 10);

	wlc_read_tsf(wlc, &old_l, &old_h);
	wlc_tsf_adj(wlc, bsscfg, 0, 0, old_h, old_l, bcnint_us, bcnint_us, FALSE);

	/* fixes the issue with phy errors after big hammer */
	wlc_stf_reinit_chains(wlc);

	return BCME_OK;
}

/*  abss 1sec periodic wlc_watchdog() timer callback */
void
wlc_aibss_on_watchdog(void *ctx)
{
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)ctx;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;
	wlc_bsscfg_t *cfg;
	aibss_cfg_info_t *cfg_cubby;
	int i;

#ifdef AIBSS_DBG
	aibss_dbg_info_t *dbgi = aibss->dbgi;
	uint32 cur_ts = OSL_SYSUPTIME();
	uint32 wdt_dt;
#endif
	BCM_REFERENCE(cfg_cubby);
	/* don't run aibss watchdog until bss is up */
	if (!aibss->bsscfg)
		return;

	cfg = aibss->bsscfg;

	cfg_cubby = BSSCFG_CUBBY(cfg, aibss->cfg_cubby_handle);

#ifdef AIBSS_DBG
	if (++dbgi->wdt_cnt >= DBG_PRINT_FREQ) {
		/*  print what we have accumulated in counters over wdt_dt time period */
		wdt_dt = cur_ts - aibss->dbgi->wdt_prev_ts;
		dbgi->wdt_prev_ts = cur_ts;
		dbgi->wdt_cnt = 0;

		AIBSS_INFO(("----- stats per WDT_dT:%dms period, heap:%d-------\n"
			" tbtt intr:%d, atim_end intr:%d, pretbtt_2_tbtt_dt:%d "
			"pretbtt-2-atim_end:%d, bcn_rx_cnt:%d\n"
			" atim_tx_cnt:%d, atim_rx_cnt:%d, ATIM fail cnts:"
			" ucast:%d, bcast:%d, to_slot_skips:%d\n",
			wdt_dt, OSL_MEM_AVAIL(), dbgi->tbtt_int_cnt,
			dbgi->atim_int_cnt, dbgi->pretbtt_2_tbtt_dt,
			dbgi->pretbtt_2_atim_dt, dbgi->rxbcn_cnt,
			dbgi->atim_tx_cnt, dbgi->atim_rx_cnt,
			dbgi->uc_atim_fail_cnt, dbgi->bcmc_atim_fail_cnt, dbgi->tot_slt_skp));
		if (dbgi->bad_tbtt_cnt) {
			AIBSS_INFO(("bad TBTT position in ATIM window detected %d times\n",
				dbgi->bad_tbtt_cnt));
		}
		/* clr intr counters  */
		dbgi->tbtt_int_cnt = 0;
		dbgi->bad_tbtt_cnt = 0;
		dbgi->atim_int_cnt = 0;
		dbgi->uc_atim_fail_cnt = 0;
		dbgi->atim_tx_cnt = 0;
		dbgi->atim_rx_cnt = 0;
		dbgi->rxbcn_cnt = 0;
		dbgi->bcmc_atim_fail_cnt = 0;
		dbgi->tot_slt_skp = 0;
	}

#if CHK_MSCH_2_TSF /* check msch vs tsf clock drift */
{	uint32 tsf_l, tsf_h;
	uint32 msch_h, msch_l;
	uint64 msch64;

	wlc_read_tsf(wlc, &tsf_l, &tsf_h);
	msch64 = msch_current_time(wlc->msch_info);

	msch_l = (uint32)msch64;
	msch_h = (uint32)(msch64 >> 32);


	wlc_uint64_sub(&msch_h, &msch_l, tsf_h, tsf_l);

	AIBSS_INFO(("AS!!! msch - tsf: %x:%x\n", msch_h, msch_l));
}
#endif /* CHK_MSCH_2_TSF */

#endif /* AIBSS_DBG */

	/* check beacon timeout is reached to send fail event */
	FOREACH_AS_STA(wlc, i, cfg) {
		if (BSSCFG_IBSS(cfg)) {
			struct scb_iter scbiter;
			struct scb *scb;
			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
				aibss_scb_info_t	*aibss_scb_info =
					SCB_CUBBY((scb),
					WLC_AIBSS_INFO_SCB_HDL(aibss_info));
				/*
				 * Current TX FAIL CONDITION if there is no BEACON
				 * for a N seconds
				 */
				 if (aibss_scb_info->bcn_count == 0) {
					aibss_scb_info->no_bcn_counter++;
					wlc_aibss_check_txfail(aibss_info, cfg, scb);
				 } else {
					aibss_scb_info->no_bcn_counter = 0;
					aibss_scb_info->bcn_count = 0;
				 }
			}
		}
	}
}


/*   notification cbfn on beacon receive from peers  */
void
wlc_aibss_on_rx_beacon(void *ctx, bss_rx_bcn_notif_data_t *data)
{
	bss_rx_bcn_notif_data_t *bcn = data;
	wlc_bsscfg_t *cfg = bcn->cfg;
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)ctx;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);

	aibss_scb_info_t *aibss_scb_info = SCB_CUBBY((bcn->scb),
		WLC_AIBSS_INFO_SCB_HDL(aibss_info));

	if (cfg != aibss->bsscfg) {
		AIBSS_INFO(("rx bcn, cfg!=aibss, tsf_l:%d\n", bcn->wrxh->tsf_l));
		return;
	}

	aibss_scb_info->bcn_count++;

#ifdef AIBSS_DBG
	aibss->dbgi->rxbcn_cnt++;
#endif
}

static void
wlc_aibss_txmod_on_activate(void *ctx, struct scb *scb)
{
#ifdef AIBSS_DBG
	txmod_id_t prev, next;

	AIBSS_INFO(("%s txpath list for scb:%p\n", __FUNCTION__, OSL_OBFUSCATE_BUF(scb)));

	prev = TXMOD_START;
	while ((next = scb->tx_path[prev].next_fid) != 0) {
		AIBSS_INFO((" >>> txfuncs:%p, fid:%d, cfgEn:%d\n",
			OSL_OBFUSCATE_BUF(scb->tx_path[prev].next_tx_fn),
			scb->tx_path[prev].next_fid,
			/* next fid.configured  */
			scb->tx_path[next].configured));
		prev = next;
	}
#endif
}

static void
wlc_aibss_txmod_on_deactivate(void *ctx, struct scb *scb)
{
#ifdef AIBSS_DBG
	txmod_id_t prev, next;

	AIBSS_INFO(("%s txpath list for scb:%p\n", __FUNCTION__, OSL_OBFUSCATE_BUF(scb)));

	prev = TXMOD_START;
	while ((next = scb->tx_path[prev].next_fid) != 0) {
		AIBSS_INFO((" >>> txfuncs:%p, fid:%d, cfgEn:%d\n",
			OSL_OBFUSCATE_BUF(scb->tx_path[prev].next_tx_fn),
			scb->tx_path[prev].next_fid,
			/* next fid.configured  */
			scb->tx_path[next].configured));
		prev = next;
	}
#endif
}

/* TODO: integration with msch + scan */
void wlc_aibss_back2homechan_notify(wlc_info_t *wlc)
{
	wlc_aibss_info_priv_t *aibss;

	/* make sure aibss is enabled */
	if (!wlc->aibss_info)
		return;

	aibss = WLC_AIBSS_INFO_PRIV(wlc->aibss_info);

	/* other off chan opss may clear/set mac ctrl reg bits */
	if (aibss->bsscfg != NULL) {
		wlc_bmac_mctrl(wlc->hw, MCTL_AP | MCTL_INFRA, MCTL_AP);
	}
}

#ifdef MSCH_CODE
static int
wlc_aibss_sta_msch_clbk(void* handler_ctxt, wlc_msch_cb_info_t *cb_info)
{
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)handler_ctxt;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_bsscfg_t *cfg = aibss->bsscfg;
	aibss_cfg_info_t *cfg_cubby;
	wlc_info_t *wlc;
	uint32 type = cb_info->type;
	int err = BCME_OK;
	wlc_msch_onchan_info_t * onch_info = (wlc_msch_onchan_info_t *)cb_info->type_specific;

	if (!cfg) {
		/* The requeset is been cancelled, ignore the Clbk */
		return BCME_OK;
	}

	wlc = aibss->wlc;
	cfg_cubby = BSSCFG_CUBBY(cfg, aibss->cfg_cubby_handle);

	if (!BSSCFG_AIBSS(cfg)) {
		return BCME_ERROR;
	}

	if (!cfg_cubby) {
		ASSERT(0);
		return BCME_ERROR;
	}

#ifdef AIBSS_DBG
	/* print timestamps @ atim_end if there was a change */
	/* note msch signals N of slots to skip by N calls with SKIP TYPE */
	if (type != cfg_cubby->cbtype_prev) {
		MSCH_CBINFO(("msch_clbk: Curr_time=%d TS:%d type change 0x%x --> 0x%x\n",
			OSL_SYSUPTIME(), aibss->msch_slot_start_ts, cfg_cubby->cbtype_prev, type));
		aibss->dbgi->print_cnt = 1;	// do onetime print in next ATIM

	}
#endif

	if (type & (MSCH_CT_ON_CHAN | MSCH_CT_SLOT_START)) {
		/* ASSERT START & END combination in same callback */
		ASSERT(!(type & (MSCH_CT_OFF_CHAN | MSCH_CT_SLOT_END |
			MSCH_CT_OFF_CHAN_DONE | MSCH_CT_SLOT_SKIP)));

	}

	if (type & MSCH_CT_REQ_START) {
		MSCH_CBINFO(("msch_clbk: MSCH_CT_REQ_START\n"));
		wlc_full_phy_cal(wlc, cfg, PHY_PERICAL_JOIN_BSS);
		cfg_cubby->msch_state = WLC_MSCH_REQ_START;
	}

	if (type & MSCH_CT_ON_CHAN) {
		MSCH_CBINFO(("msch_clbk: MSCH_CT_ON_CHAN, slot_id:%d\n", onch_info->timeslot_id));
		cfg_cubby->msch_state = WLC_MSCH_ON_CHANNEL;
		wlc_txqueue_start(wlc, cfg, cb_info->chanspec, NULL);
	}

	if ((type & MSCH_CT_OFF_CHAN_DONE) || (type & MSCH_CT_SLOT_END)) {
		wlc_txqueue_end(wlc, cfg, NULL);
	}

	if ((type & MSCH_CT_SLOT_END) || (type & MSCH_CT_OFF_CHAN)) {
		/* fall through */
		MSCH_CBINFO(("msch_clbk: MSCH_CT_SLOT_END || MSCH_CT_OFF_CHAN\n"));
		cfg_cubby->msch_state = WLC_MSCH_OFFCHANNEL;
		goto exit;
	}


	if (type & MSCH_CT_REQ_END) {
		MSCH_CBINFO(("MSCH_CT_REQ_END\n"));
		/* The msch hdl is no more valid */
		cfg_cubby->msch_state = WLC_MSCH_OFFCHANNEL;
		cfg_cubby->msch_req_hdl = NULL;
		goto exit;
	}

	if (type & MSCH_CT_SLOT_START) {

		aibss->msch_slot_start_ts = OSL_SYSUPTIME();
		/* at next tbtt check that slot_start fired dT before tbtt */
		cfg_cubby->slot_upd_chk = TRUE;

		MSCH_CBINFO(("msch_clbk: MSCH_CT_SLOT_START, slot_id:%d\n",
			onch_info->timeslot_id));

		if (cfg_cubby->slot_skp_started) {
			AIBSS_ERROR(("msch: FIXME aibss SLOT_START during scan?\n"));
		} else {
			wlc_aibss_return_home_channel(aibss_info, cfg);
		}

		cfg_cubby->msch_state = WLC_MSCH_ON_CHANNEL;
		cfg_cubby->timeslot_id = onch_info->timeslot_id;
		goto exit;
	}

	if (type & MSCH_CT_SLOT_SKIP) {
		/* for AIBSS: current slot is still active, next one should be skipped */
		cfg_cubby->slot_skip_cnt++;
		MSCH_CBINFO(("msch_clbk: MSCH_CT_SLOT_SKIP, skp_cnt:%d\n",
			cfg_cubby->slot_skip_cnt));
	}

exit:
	cfg_cubby->cbtype_prev = type;
	return err;
}

static void _wlc_aibss_msch_update(wlc_aibss_info_priv_t *aibss, uint32 delta_usec)
{
	wlc_info_t *wlc = aibss->wlc;
	wlc_bsscfg_t *cfg = aibss->bsscfg;
	aibss_cfg_info_t *cfg_cubby =  BSSCFG_CUBBY(cfg, aibss->cfg_cubby_handle);
	wlc_msch_req_param_t req;
	uint64 req_time;

	req_time = msch_future_time(wlc->msch_info, delta_usec);
	req.start_time_l = (uint32)req_time;
	req.start_time_h = (uint32)(req_time >> 32);

	wlc_msch_timeslot_update(wlc->msch_info,
		cfg_cubby->msch_req_hdl,
		&req,
		MSCH_UPDATE_START_TIME);
}

/*  unregister aibss timeslot with msch */
static void
_wlc_aibss_timeslot_unregister(wlc_aibss_info_t *aibss_info)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;
	wlc_bsscfg_t *bsscfg = aibss->bsscfg;
	aibss_cfg_info_t *cfg_cubby;

	cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);

	ASSERT(cfg_cubby != NULL);

	if (cfg_cubby->msch_req_hdl) {
		wlc_msch_timeslot_unregister(wlc->msch_info, &cfg_cubby->msch_req_hdl);
	}

	cfg_cubby->msch_state = WLC_MSCH_OFFCHANNEL;
	cfg_cubby->register_rq = FALSE;
	cfg_cubby->slot_skip_cnt = 0;
	cfg_cubby->timeslot_id = INVALID_SLOT;
	cfg_cubby->cbtype_prev = -1;
}

/*  register aibss timeslot with msch */
static int
_wlc_aibss_timeslot_register(wlc_aibss_info_t *aibss_info)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;
	wlc_bsscfg_t *bsscfg = aibss->bsscfg;
	aibss_cfg_info_t *cfg_cubby;
	wlc_msch_req_param_t req;
	uint32 err = 0;
	uint64 start_time;

	BCM_REFERENCE(start_time);

	cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);

	ASSERT(cfg_cubby != NULL);

	ASSERT(bsscfg->current_bss);


	req.req_type = MSCH_RT_BOTH_FIXED;
	req.flags = MSCH_REQ_FLAGS_MERGE_CONT_SLOTS;
	req.duration = bsscfg->current_bss->beacon_period;
	req.duration <<= MSCH_TIME_UNIT_1024US;
	req.interval = bsscfg->current_bss->beacon_period;
	req.interval <<= MSCH_TIME_UNIT_1024US;
	req.priority = MSCH_DEFAULT_PRIO;

	if (cfg_cubby->msch_req_hdl) {
		wlc_msch_timeslot_unregister(wlc->msch_info, &cfg_cubby->msch_req_hdl);
	}

	/* start in the future REQ_DELTA_FT offset from current tbtt */
	start_time = msch_future_time(wlc->msch_info,
			REQ_DELTA_FT(bsscfg->current_bss->beacon_period));
	req.start_time_l = (uint32)start_time;
	req.start_time_h = (uint32)(start_time >> 32);

	if ((err = wlc_msch_timeslot_register(wlc->msch_info, &bsscfg->current_bss->chanspec, 1,
	          wlc_aibss_sta_msch_clbk, aibss_info, &req,
	          &cfg_cubby->msch_req_hdl)) == BCME_OK) {
		AIBSS_INFO(("%s request success start_time:0x%x%x, bcn period:%dtu, durtn:%d"
			" interval:%d\n", __FUNCTION__, (uint32)(start_time >> 32),
			(uint32)(start_time & 0xffffffff),
			bsscfg->current_bss->beacon_period, req.duration, req.interval));
	} else {
		AIBSS_INFO(("%s request failed error %d\n", __FUNCTION__, err));
		ASSERT(0);
	}

	return err;
}

static void
wlc_aibss_return_home_channel(wlc_aibss_info_t *aibss_info, wlc_bsscfg_t *cfg)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;

	if (wlc->excursion_active) {
		AIBSS_ERROR(("ERROR: excursion  IS active!!!\n"));
	}

#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		/* Open the active interface so that host
		 * start sending pkts to dongle
		 */
		wlc_wlfc_mchan_interface_state_update(wlc,
			cfg, WLFC_CTL_TYPE_INTERFACE_OPEN, FALSE);
	}
#endif /* PROP_TXSTATUS */

	wlc_aibss_back2homechan_notify(wlc);
}
#endif /* MSCH_CODE */


/* is called from wlc_sta.c afer aibss join completed  */
static int
wlc_aibss_timeslot_register(wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc = bsscfg->wlc;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(wlc->aibss_info);
	aibss_cfg_info_t *cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);
	BCM_REFERENCE(cfg_cubby);

#if defined(MSCH_CODE)	// disable scheduler
	/* update at 1st tbtt */
	cfg_cubby->register_rq = TRUE;
#endif
	return BCME_OK;
}

static void
wlc_aibss_assoc_state_clbk(void *arg, bss_assoc_state_data_t *notif_data)
{
	wlc_bsscfg_t *cfg;

	ASSERT(notif_data != NULL);

	cfg = notif_data->cfg;
	ASSERT(cfg != NULL);

	AIBSS_INFO(("wl(%d.%d): wlc_aibss_assoc_state_clbk(): type:%d, notif_state:%d\n",
		cfg->wlc->pub->unit, WLC_BSSCFG_IDX(cfg), cfg->type, notif_data->state));

	if (BSSCFG_AIBSS(cfg) && (notif_data->state == AS_IBSS_CREATE ||
		notif_data->state == AS_JOIN_ADOPT)) {
		/*  --- register AIBSS request --- */
		(void)wlc_aibss_timeslot_register(cfg);
		AIBSS_INFO(("AIBSS timeslot registered"));
	}
}
