/*
 * Broadcom SDIO/PCMCIA device-side driver
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmnvram.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <osl.h>
#include <hndsoc.h>
#include <proto/ethernet.h>
#include <proto/802.1d.h>
#include <sbsdio.h>
#include <sbchipc.h>
#include <sbsdpcmdev.h>
#include <bcmsdpcm.h>
#include <sdpcmdev.h>
#include <sdpcmdev_dbg.h>
#include <dngl_bus.h>
#include <dngl_api.h>
#include <bcmcdc.h>
#include <dngl_logtrace.h>
#include <dngl_msgtrace.h>
#include <rte_cons.h>
#include <rte_trap.h>
#include <rte_isr.h>
#include <rte.h>
#ifdef BCMULP
#include <ulp.h>
#ifdef BCMFCBS
#include <fcbsdata.h>
#endif /* BCMFCBS */
#endif /* BCMULP */
#include <devctrl_if/wlioctl_defs.h>
#include <sdiovar.h>

#ifndef SDPCMD_TXGLOM
#define	SDPCMD_TXGLOM	7
#endif
/* include support for experimental glom modes */
#define GLOMTEST	0

/*
 * TXGLOM modes
 *	0: glomming disabled, all frames go immediately
 *	1: original glomming, always hold frames for timer
 *	2: (GLOMTEST only) kiran's original (glom w/o delay)
 *	3: (GLOMTEST only) modified a tad (send only on empty dma)
 *	4: (GLOMTEST only) use timer as hold-down only
 *	5: (GLOMTEST only) use timer hold-down and DMA empty trigger
 *	6: (GLOMTEST only) use timer as idle (rather than force) trigger
 *	7: enhanced glom 5 with higher response throughput
 */

/* tunables default values */
#ifndef DEFTXSWQLEN
#define DEFTXSWQLEN	64
#endif /* DEFTXSWQLEN */

#ifndef DEFMAXTXPKTGLOM
#define DEFMAXTXPKTGLOM	10	/* default limit on how many lazy packets may be queued */
#endif /* DEFMAXTXPKTGLOM */
#ifndef DEFTXLAZYDELAY
#define DEFTXLAZYDELAY	2	/* default milliseconds before txlazy timer fires */
#endif
#define DEFTXGLOMALIGN	32	/* default alignment length for glommed segements */

#define DEFACKFASTFWD 	1	/* default enable of fast forward ACK packets to host feature */
#define DEFACKSIZETHSD 	200 /* default received packet size threshold to determine if it's ACK */

#ifndef DEFMINSCHED
#define DEFMINSCHED	2
#endif

#ifndef DEFCREDTRIG
#define DEFCREDTRIG     2
#endif

#if (defined(BRINGUP_BUILD) || (SDPCMD_TXGLOM == 0))
#define DEFTXGLOM	0	/* default txglom enable state */
#else /* !BRINGUP_BUILD */
#define TXGLOM_CODE
#define DEFTXGLOM	SDPCMD_TXGLOM
#endif /* !BRINGUP_BUILD */

#ifndef SDPCMD_READAHEAD
static bool sdpcmd_readahead = TRUE;
#else
static bool sdpcmd_readahead = SDPCMD_READAHEAD;
#endif

#define SDPCMD_RXBUFSZ PKTBUFSZ

#ifdef BCMPKTPOOL
#define PHDR_ENAB(bus)	POOL_ENAB((bus)->pktpool_phdr)
#define SDHDR_POOL_LEN	6
#else /* BCMPKTPOOL */
#define PHDR_ENAB(bus)  0
#define SDHDR_POOL_LEN	0
#define pktpool_emptycb_disable(pool, disable)
#endif /* BCMPKTPOOL */

/* indexes into tunables array */
#define MAXTXPKTGLOM	0
#define TXLAZYDELAY		1
#define TXGLOMALIGN		2
#define TXGLOM			3
#define TXSWQLEN		4
#define TXDROP			5
#define ACKFASTFWD		6
#define ACKSIZETHSD 	7
#define RXACKS			8
#define NTXD			9
#define NRXD			10
#define RXBUFS			11
#define RXBUFSZ			12
#define DATAHIWAT		13
#define DATALOWAT		14
#define TXQ_MAX			15
#define ALIGNMOD		16
#define RXBND			17
#define DONGLEOVERHEAD	18
#define DONGLEHDRSZ		19
#define DONGLEPADSZ		20
#define BDCHEADERLEN	21
#define RXFILLTRIG		22
#define CREDALL			23
#define CREDFAIL		24
#define RXCB			25
#define MAXTUNABLE		26

/* for bringup, etc. */
#define TEMP_DEBUG	0
#define DEBUG_PARAMS_ENABLED	0

/* For debug */
/* #define DS_PRINTF_LOG 1 */
/* #define DS_LOG_DUMP 1 */

#ifdef BCMDBG_SD_LATENCY
typedef struct {
	uint32 arrival_time; 	/* Time stamp arrival to txq (in us unit) */
	uint32 transmit_time; 	/* Time stamp of txq submit to host(in us unit) */
} sdpcmd_pkttag_t;

#define SDPCMD_PKTTAG(p) ((sdpcmd_pkttag_t *)PKTTAG(p))

typedef struct {
	uint32 max;
	uint32 min;
	uint32 sum;
	uint32 total_pkt;
	uint32 last_arrival;
	uint32 arrival_max;
	uint32 arrival_min;
	uint32 pkt_len_max;
	uint32 pkt_len_min;
} sdstats_latency;
#endif /* BCMDBG_SD_LATENCY */

/* Queue for values to post to host via the tohostmailboxdata register */
typedef struct hmb_data {
	uint32 val;
	struct hmb_data *next;
} hmb_data_t;


#ifdef DS_PROT

/*
 * Deep sleep logging definitions
 * DEFAULT_DS_LOG_SIZE can be overridden with "wl bus:dslog N".
 */
#define DS_LOG(sdpcmd, type, state, event, nextstate) \
	do { \
		if (sdpcmd->ds_log_size > 0) \
			_sdpcmd_ds_log(sdpcmd, type, state, event, nextstate); \
	} while (0)
#define DSMB_LOG(sdpcmd, mboxmsg, is_rx) \
	do { \
		if (sdpcmd->ds_log_size > 0) \
			sdpcmd_dsmb_log(sdpcmd, mboxmsg, is_rx); \
	} while (0)

/* Constants and types for deep sleep protocol */
#define SDPCM_DS_CHECK_MAX_ITERATION 10000
#define SDPCM_DS_CHECK_INTERVAL 50
#define DEFAULT_DS_LOG_SIZE 60

typedef enum sdpcmd_ds_state {
	DS_INVALID_STATE = -2,
	DS_DISABLED_STATE = -1,
	NO_DS_STATE = 0,        /* DW asserted, no D3, cannot go to sleep */
	DS_CHECK_STATE,         /* DW deasserted, may (have) reqeust(ed) DS */
	DS_D0_STATE,		/* DS acked, may sleep; use DS Exit to leave */
	NODS_D3_STATE,		/* D3 entered, DW not deasserted yet: no DS */
	DS_D3_STATE,		/* D3 with DW deasserted, DS allowed */
	DS_LAST_STATE
} sdpcmd_ds_state_t;

typedef enum sdpcmd_ds_event {
	DW_ASSRT_EVENT = 0,	/* Host asserts device wake */
	DW_DASSRT_EVENT,	/* Host deasserts device wake */
	D3_ENTER_EVENT,		/* Host notifies device of D3 entry */
	INT_DTOH_EVENT,	        /* Device has reason to interrupt the host */
	DS_ALLOWED_EVENT,	/* Host ACKs DS request */
	DS_NOT_ALLOWED_EVENT,	/* Host NAKs DS request */
	DS_LAST_EVENT
} sdpcmd_ds_event_t;

typedef void (*sdpcmd_ds_cbs_t)(void *cbarg);
typedef struct sdpcmd_ds_state_tbl_entry {
	sdpcmd_ds_cbs_t		action_fn;
	sdpcmd_ds_state_t	transition;
} sdpcmd_ds_state_tbl_entry_t;

#else /* !defined(DS_PROT) */

typedef enum sdpcmd_ds_state {
	DS_INVALID_STATE = -2
} sdpcmd_ds_state_t;
#define DS_LOG(sdpcmd, type, state, event, nextstate)
#define DSMB_LOG(sdpcmd, mboxmsg, is_rx)
#define DEFAULT_DS_LOG_SIZE 0

#endif /* DS_PROT */


typedef struct sdpcmd_ds_log {
	uint8 ds_log_type;
	uint8 ds_state;
	uint8 ds_event;
	uint8 ds_transition;
	uint32 ds_time;
} sdpcmd_ds_log_t;

/* interrupt masks for metrics */
#define INTMASK_BUS_METRICS_DATA  (I_RI | I_XI)
#define INTMASK_BUS_METRICS_MB    (I_SMB_DEV_INT | I_SMB_INT_ACK)
#define INTMASK_BUS_METRICS_ERROR (I_ERRORS | I_WR_OOSYNC | I_RD_OOSYNC \
					| I_RF_TERM | I_WF_TERM | I_SMB_NAK)

/* used to record start time for metric */
typedef struct sdio_metric_ref {
	uint32 active;

	uint32 ds_wake_on;
	uint32 ds_wake_off;

	uint32 ds_d3; /* DS_D3 state */
	uint32 ds_d0; /* DS_D0 state */
} sdio_metric_ref_t;

/* Externally opaque dongle bus structure */
struct dngl_bus {
	uint coreid;			/* SDPCMD core ID */
	uint corerev;			/* SDPCMD device core rev */
	osl_t *osh;			/* Driver osl handle */
	si_t *sih;			/* SiliconBackplane handle */
	struct dngl *dngl;			/* dongle handle */
	sdpcmd_regs_t *regs;		/* SDPCMD registers */

	uint32 intmask;		/* Current SDPCMD interrupt mask */
	uint32 defintmask;	/* Default SDPCMD intstatus mask */
	uint32 intstatus;	/* intstatus bits to process in dpc */
#if TEMP_DEBUG
	uint32 sbintstatus;	/* sbintstatus bits to process in dpc */
	uint32 defsbintmask;	/* Default SDPCMD sbintstatus mask */
#endif

	hnddma_t *di;		/* DMA engine handle */

	struct pktq txq;	/* Transmit packet queue */
	uint *txavail;		/* Pointer to DMA txavail variable */
	void *held_tx;		/* tx buf held for possible retransmission */
	void *pend_tx;		/* last tx buf submitted to DMA (for lookahead) */
	bool in_sendnext;	/* prevent re-entrancy */
	uint32 tohostmail;	/* bits pending for tohostmailboxdata */

	hmb_data_t *hmbd;	/* queue of values to post to tohostmailboxdata reg */

	uint8 tx_seq;		/* transmit path sequence number */
	uint8 rx_seq;		/* receive path sequence number */
	uint8 rx_lim;		/* receive path sequence limit */
	uint8 rx_hlim;		/* last limit sent to host */

	sdpcmd_cnt_t cnt;	/* SW copy of HW counters */

	bool disabled;			/* IOE2 is cleared (dev disabled) */
	bool up;			/* device is operational */
	bool sdio;		/* TRUE if strapped for SDIO mode; FALSE for PCMCIA mode */
	bool tohostmailacked;	/* host ack'd a tohostmailboxdata interrupt */
	uint8 txflowcontrol;	/* per prio flow control bit vector */
	bool use_oob;		/* Signal interrupts using gpio output */
	bool txlazytimer_running; /* Tx Lazy timer is currently running. */
	bool gptimer_active;	/* Toggle timer for gpio output is running */
	bool gpon;		/* Current gpio state is asserted */
	uint8 txflowlast;	/* last flow-control announced to host */
	bool txflowpending;	/* flow-control announcement needed */
	uint32 gpbit;		/* Bitmask for the output gpio to use */
	uint32 gpval;		/* Bitmask for gpio output to generate interrupt */
	uint16 gpontime;	/* On time for duty cycle when toggling interrupt */
	uint16 gpofftime;	/* Off time for duty cycle when toggling interrupt */
	dngl_timer_t *gptimer;	/* Timer for toggling gpio interrupt output */
	dngl_timer_t *txlazytimer;
	uint32 tunables[MAXTUNABLE];
	bool	frame_tx_pend;
	bool	gspidword;
	uint32	alltx_fc;
	uint32	explicit_fc;
	uint32	sendhdr_fail;
	bool rxreclaim;
#ifdef BCMDBG_SD_LATENCY
	sdstats_latency lat_stats; /* SD latency stats */
#endif /* BCMDBG_SD_LATENCY */

	/* BCMPKTPOOL related vars */
	pktpool_t *pktpool;
	pktpool_t *pktpool_phdr;

	bool msgtrace;		/* 1=firmware was built with msg trace enabled */
	bool oob_only;		/* Signal interrupts(OOB) only by using gpio output */
	uint32 rxglom;		/* Rx glom Enabled/Disabled */
	/* Keep SDTEST at the end so that it can be enabled easily if not enabled in ROM */
	bool acktx_pending;
	bool logtrace;		/* 1=firmware was built with log trace enabled */
	hnd_dpc_t *rxfill_dpc;	/* DPC for calling _sdpcmd_rxfill() */

	/* device wake */
	uint8 devwake_gp;	/* gpio number for devwake */
	bool devwake_last;	/* last state read from GPIO */
	void *devwake_h;	/* interrupt handle */

	/* deepsleep protocol */
	bool _ds_prot;
	bool force_wake; 	/* actual deepsleep-disabled state (HTAVAIL) */
	bool ds_disable_state;	/* state as per DS protocol (target) */
	bool in_ds_engine;
	sdpcmd_ds_state_t ds_state;
	dngl_timer_t	*ds_check_timer;
	bool 	ds_check_timer_on;
	uint32	ds_check_timer_cnt;
	uint32	ds_check_timer_max;	/* may be tunable */
	uint32	ds_check_interval;	/* may be tunable */
	uint8	ds_int_dtoh_mode;	/* tune/test: 0 no event, 1 state, 2 state + bits */
	bool	ds_always_delay;	/* always force delay (check tick) before dsreq */

	bool	ds_hwait;		/* want DS, waiting (e.g. for host unacked intstatus) */
	uint8	ds_hwait_cnt;		/* count of hwait polls needed before entering DS */
	dngl_timer_t *ds_hwait_timer;	/* timer for polling wait conditions */

	/* log/debug */
	bool	ds_check_print;
	bool	ds_inform_print;
	bool	ds_trace_print;
	bool	ds_log_stop;		/* Stop ds logs */
	uint16	ds_log_size;		/* Max # of Deepsleep log entries */
	uint16	ds_log_index;		/* Deepsleep circular log buffer index */
	sdpcmd_ds_log_t *ds_log;	/* Deepsleep log buffer */
	uint32 dslogsz_tunable;		/* size of DS_PROT log buffer */

	uint32 ds_stale_dsack;
	uint32 ds_stale_dsnack;
	uint32	last_tx_mbmsg;		/* Copy of the last sent mbox msg */
	uint32	last_rx_mbmsg;		/* Copy of the last received mbox msg */
	uint8	dc_tmron_cnt;		/* debug counter */
	uint8	nodsd3dw_cnt;		/* debug counter */
	uint8	dwcannotds_cnt;		/* debug counter */
	uint8	misseddwd_cnt;		/* debug counter */
	uint8	misseddwa_cnt;		/* debug counter */
	uint8	txfail_cnt;		/* debug counter */
	uint8	txmb_ovfl_cnt;		/* debug counter */
	bool	force_awake;		/* debug: force chip to stay awake */

	sdio_bus_metrics_t metrics;	/* tallies used to measure power state behavior */
	sdio_metric_ref_t metric_ref;	/* used to record start time for metric */

	bool di_dma_suspend_for_trap;

	bool want_resync;	/* Want to send credit update to host */
	uint8 initrxbufs;
	uint8 credtrig;
	uint8 minsched;
	uint8 glomevents;	/* 0-none, 1-data after BDC, 2-data after SDPCMD, 3-all */

	uint8	drive_strength_ma;	/* SDIO drive strength */
};


#define GPOUT_DEFAULT 1		/* Default to gpio 0 (0x00000001) */

#ifdef DEBUG_PARAMS_ENABLED
struct sdio_debug_params {
	uint8 *tx_seq;
	uint8 *rx_seq;
	uint8 *rx_lim;
	uint8 *rx_hlim;
	struct pktq *txq;
	uint *txavail;
};

static struct sdio_debug_params debug_params;
#endif /* DEBUG_PARAMS_ENABLED */

#ifdef BCMULP
typedef struct sdpcmd_ulp_cr_dat {
	uint32 txglom;
} sdpcmd_ulp_cr_dat_t;

static uint sdpcmd_ulp_get_retention_size_cb(void *handle, ulp_ext_info_t *einfo);
static int sdpcmd_enter_pre_ulpucode_cb(void *handle, ulp_ext_info_t *einfo, uint8 *cache_data);
static int sdpcmd_enter_post_ulpucode_cb(void *handle, ulp_ext_info_t *einfo);
static int sdpcmd_ulp_exit_cb(void *handle, uint8 *cache_data, uint8 *p2_cache_data);
static void sdpcmd_process_ioe2(struct dngl_bus *sdpcmd);

static const ulp_p1_module_pubctx_t ulp_sdpcmd_ctx = {
	.flags			= MODCBFL_CTYPE_STATIC,
	.fn_enter_pre_ulpucode	= sdpcmd_enter_pre_ulpucode_cb,
	.fn_exit		= sdpcmd_ulp_exit_cb,
	.fn_cubby_size		= sdpcmd_ulp_get_retention_size_cb,
	.fn_enter_post_ulpucode	= sdpcmd_enter_post_ulpucode_cb,
};
#endif /* BCMULP */


static void sdpcmd_tunables_init(struct dngl_bus *sdpcmd);
static bool sdpcmd_match(uint vendor, uint device);
static void sdpcmd_reset(struct dngl_bus *sdpcmd);
static void sdpcmd_sendnext(struct dngl_bus *sdpcmd);
static void sdpcmd_sendup(struct dngl_bus *sdpcmd, void *p);
static void sdpcmd_txrecompose(struct dngl_bus *sdpcmd, bool retrans);
static void sdpcmd_rxrecompose(struct dngl_bus *sdpcmd);
static void sdpcmd_txreclaim(struct dngl_bus *sdpcmd);
static void sdpcmd_rxreclaim(struct dngl_bus *sdpcmd);
static bool sdpcmd_dma(struct dngl_bus *sdpcmd, uint32 dmaintstatus);
static int sdpcmd_txsubmit(struct dngl_bus *sdpcmd, void *p, int channel);
static int sdpcmd_tx(struct dngl_bus *sdpcmd, void *p, int channel);
static void _sdpcmd_rxfill(void *data);
static void sdpcmd_rxfill(struct dngl_bus *sdpcmd);

static bool sdpcmd_devmail(struct dngl_bus *sdpcmd);
static void hostmailbox_post(struct dngl_bus *sdpcmd, uint32 val);
static void sdpcmd_sendheader(struct dngl_bus *sdpcmd);
#ifdef RTE_CONS
static void sdpcmd_dumpstats(void *dngl_bus, int argc, char *argv[]);
#endif
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void sdpcmd_statsupd(struct dngl_bus *sdpcmd);
static void sdpcmd_dumpdma(struct dngl_bus *sdpcmd, struct bcmstrbuf *b);
#endif
static void sdpcmd_process_ioe2(struct dngl_bus *sdpcmd);


#ifdef BCMDBG_SD_LATENCY
#ifdef RTE_CONS
static void sdpcmd_print_latency(void *dngl_bus, int argc, char *argv[]);
#endif
static void sdpcmd_timestamp_arrival(struct dngl_bus *sdpcmd, void *p);
static void sdpcmd_timestamp_transmit(struct dngl_bus *sdpcmd, void *p);
#endif /* BCMDBG_SD_LATENCY */

#ifdef DS_PROT
static void sdpcmd_ds_disable_deepsleep(struct dngl_bus *sdpcmd, bool disable);
static int sdpcmd_enable_device_wake(struct dngl_bus *sdpcmd);
static bool sdpcmd_read_device_wake_gpio(struct dngl_bus *sdpcmd);
static void sdpcmd_ds_engine(struct dngl_bus * sdpcmd, sdpcmd_ds_event_t event);
static void sdpcmd_device_wake_isr(uint32 status, void *arg);
static sdpcmd_ds_state_tbl_entry_t*
sdpcmd_get_ds_state_tbl_entry(sdpcmd_ds_state_t state, sdpcmd_ds_event_t event);

#ifdef DS_LOG_DUMP
static int sdpcmd_ds_engine_log_dump(struct dngl_bus *sdpcmd);
#endif /* DS_LOG_DUMP */

#if defined(DS_LOG_DUMP) || defined(DS_PRINTF_LOG)
static const char *sdpcmd_ds_state_name(sdpcmd_ds_state_t state);
static const char *sdpcmd_ds_event_name(sdpcmd_ds_event_t event);
#endif /* defined(DS_LOG_DUMP) || defined(DS_PRINTF_LOG) */

static int sdpcmd_ds_log_init(struct dngl_bus *sdpcmd, uint16 max_log_entries);
static int sdpcmd_ds_log_deinit(struct dngl_bus *sdpcmd);
static void sdpcmd_dsmb_log(struct dngl_bus *sdpcmd, uint32 mbmsg, bool is_rx);

static bool sdpcmd_can_goto_ds(struct dngl_bus * sdpcmd);
static void sdpcmd_ds_enter_req(struct dngl_bus *sdpcmd);
static void sdpcmd_ds_exit_notify(struct dngl_bus *sdpcmd);
static void sdpcmd_d3_exit_notify(struct dngl_bus *sdpcmd);
static void sdpcmd_ds_check_periodic(struct dngl_bus *sdpcmd);
static void sdpcmd_ds_check_timerfn(dngl_timer_t *t);
static void sdpcmd_ds_hwait_timerfn(dngl_timer_t *t);

static void sdpcmd_no_ds_dw_deassrt(void *handle);
static void sdpcmd_no_ds_d3_enter(void *handle);
static void sdpcmd_ds_check_dw_assrt(void *handle);
static void sdpcmd_ds_check_d3_enter(void *handle);
static void sdpcmd_ds_check_ds_allowed(void *handle);
static void sdpcmd_ds_d0_int_dtoh(void *handle);
static void sdpcmd_ds_d0_dw_assrt(void *handle);
static void sdpcmd_ds_nods_d3_dw_assrt(void *handle);
static void sdpcmd_ds_nods_d3_dw_dassrt(void *handle);
static void sdpcmd_ds_nods_d3_int_dtoh(void *handle);
static void sdpcmd_ds_ds_d3_int_dtoh(void *handle);
static void sdpcmd_ds_ds_d3_dw_assrt(void *handle);

/* Simple debug/print macros -- depends on a local "struct dngl_bus *sdpcmd" */
#ifdef DS_PRINTF_LOG
#define DS_CHECK(args) do { if (sdpcmd->ds_check_print) printf args; } while (0)
#define DS_INFORM(args) do { if (sdpcmd->ds_inform_print) printf args; } while (0)
#define DS_TRACE(args) do { if (sdpcmd->ds_trace_print) printf args; } while (0)
#else /* !DS_PRINTF_LOG */
#define DS_CHECK(args)
#define DS_INFORM(args)
#define DS_TRACE(args)
#endif /* DS_PRINTF_LOG */

static sdpcmd_ds_state_tbl_entry_t sdpcmd_ds_state_tbl[DS_LAST_STATE][DS_LAST_EVENT] = {
	{ /* state: NO_DS_STATE */
		{NULL, DS_INVALID_STATE}, /* event: DW_ASSRT_EVENT */
		{sdpcmd_no_ds_dw_deassrt, DS_CHECK_STATE}, /* event: DW_DASSRT_EVENT */
		{sdpcmd_no_ds_d3_enter, NODS_D3_STATE}, /* event: D3_ENTER_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: INT_DTOH_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: DS_ALLOWED_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: DS_NOT_ALLOWED_EVENT */
	},
	{ /* state: DS_CHECK_STATE */
		{sdpcmd_ds_check_dw_assrt, NO_DS_STATE}, /* event: DW_ASSRT_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: DW_DASSRT_EVENT */
		{sdpcmd_ds_check_d3_enter, NODS_D3_STATE}, /* event: D3_ENTER_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: INT_DTOH_EVENT */
		{sdpcmd_ds_check_ds_allowed, DS_D0_STATE}, /* event: DS_ALLOWED_EVENT */
		{NULL, NO_DS_STATE}, /* event: DS_NOT_ALLOWED_EVENT */
	},
	{ /* state: DS_D0 */
		{sdpcmd_ds_d0_dw_assrt, NO_DS_STATE}, /* event: DW_ASSRT_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: DW_DASSRT_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: D3_ENTER_EVENT */
		{sdpcmd_ds_d0_int_dtoh, DS_CHECK_STATE}, /* event: INT_DTOH_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: DS_ALLOWED_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: DS_NOT_ALLOWED_EVENT */
	},
	{ /* state: NODS_D3_STATE */
		{sdpcmd_ds_nods_d3_dw_assrt, NO_DS_STATE}, /* event: DW_ASSRT_EVENT */
		{sdpcmd_ds_nods_d3_dw_dassrt, DS_D3_STATE}, /* event: DW_DASSRT_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: D3_ENTER_EVENT */
		{sdpcmd_ds_nods_d3_int_dtoh, NO_DS_STATE}, /* event: INT_DTOH_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: DS_ALLOWED_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: DS_NOT_ALLOWED_EVENT */
	},
	{ /* state: DS_D3 */
		{sdpcmd_ds_ds_d3_dw_assrt, NO_DS_STATE}, /* event: DW_ASSRT_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: DW_DASSRT_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: D3_ENTER_EVENT */
		{sdpcmd_ds_ds_d3_int_dtoh, NO_DS_STATE}, /* event: INT_DTOH_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: DS_ALLOWED_EVENT */
		{NULL, DS_INVALID_STATE}, /* event: DS_NOT_ALLOWED_EVENT */
	}
};
#else /* DS_PROT */
#define DS_CHECK(args) do { }  while (0)
#define DS_INFORM(args) do { } while (0)
#define DS_TRACE(args) do { } while (0)
#endif /* DS_PROT */


#ifndef SDPCMD_RXBUFS
#define SDPCMD_RXBUFS 16	/* Number of rx buffers */
#endif
#ifndef SDPCMD_NTXD
#define SDPCMD_NTXD 16		/* Length of Tx Packet Queue */
#endif
#ifndef SDPCMD_NRXD
#define SDPCMD_NRXD 256		/* Length of Rx Packet Queue */
#endif
#define SDPCMD_RXOFFSET 8

/* SDPCM_ALIGN must be a power of 2 */
#define SDPCM_ALIGN	4

#define SDPCM_ALIGNMOD	(SDPCM_ALIGN - 1)
#define SDPCM_ALIGNMASK	(~(SDPCM_ALIGNMOD))

#define DEF_IRL			(1 << IRL_FC_SHIFT)
#define DEF_INTMASK		(I_SMB_SW_MASK | I_RI | I_XI | I_ERRORS | I_WR_OOSYNC | \
				 I_RD_OOSYNC | I_RF_TERM | I_WF_TERM)
#define DEF_SDIO_INTMASK	(DEF_INTMASK | I_SRESET | I_IOE2)
#define DEF_SB_INTMASK		(I_SB_SERR | I_SB_RESPERR | I_SB_SPROMERR)

#ifndef TXGLOM_CODE
#define SDPCM_TX_PREC_MAX	1
#define	SDPCM_TX_PREC_DATA	0
#define SDPCM_TX_PREC_CTRL	0
#else
#define SDPCM_TX_PREC_MAX	2
#define	SDPCM_TX_PREC_DATA	0
#define SDPCM_TX_PREC_CTRL	1
#endif

#define SDPCM_TX_DMA_MIN	3

#ifdef TXGLOM_CODE

static bool
sdpcmd_event_pkt_glom(struct dngl_bus *sdpcmd, void *p)
{
	uint16 seglen;
	uint doff;
	uchar *pdata;

	/* No events or all events need no other checks */
	if (sdpcmd->glomevents == 0) {
		return FALSE;
	}
	if (sdpcmd->glomevents == 3) {
		return TRUE;
	}

	/* Otherwise, require at least some data after SD headers */
	seglen = PKTLEN(sdpcmd->osh, p);
	pdata = (uchar *)PKTDATA(sdpcmd->osh, p);
	doff = SDPCM_DOFFSET_VALUE(pdata + SDPCM_FRAMETAG_LEN);

	if (doff >= seglen) {
		return FALSE;
	}

	/* For mode 1, need at least some data after BDC too */
	if (sdpcmd->glomevents == 1) {
		if ((doff + 4) >= seglen) {
			return FALSE;
		}

		if ((doff + sizeof(struct bdc_header) +
		     ((struct bdc_header *)(pdata + doff))->dataOffset * 4) >= seglen) {
			return FALSE;
		}
	}

	return TRUE;
}

static void
sdpcmd_txglom(struct dngl_bus *sdpcmd)
{
	uchar *sf_desc, *pktdata;
	uint16 len, rounduplen, sflen;
	uint16 *lenp;
	uint8 sequence;
	uint32 swheader[2];
	uchar *header;
	void *p, *prev, *curr, *sf_descp, *sf;
	struct pktq tmpq;
	int numdesc;
	bool stop_glom = FALSE;
	int qlen;
	uint glompkts = 0;
	uint availdesc = 0;

	if ((qlen = pktq_mlen(&sdpcmd->txq, (1 << SDPCM_TX_PREC_DATA)))  < 2) {
		return;
	}

	/* do our best to make sure there's space on the hw */
	sdpcmd_txreclaim(sdpcmd);

	/* txreclaim may have reduced queue length */
	if ((qlen = pktq_mlen(&sdpcmd->txq, (1 << SDPCM_TX_PREC_DATA))) < 2)
		return;

	swheader[1] = 0;

	/* first packet in a glom requires space for uberheader, bail if not */
	if (PKTHEADROOM(sdpcmd->osh, pktq_peek(&sdpcmd->txq, NULL)) <
	    (SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN)) {
		return;
	}

	pktq_init(&tmpq, 1, sdpcmd->tunables[TXSWQLEN] + 1);
	/* check for a superframe or control frame already on the sw queue */
	/* also check that there will be enough TXD's for the superframe */
	availdesc = *sdpcmd->txavail;
	numdesc = 1;
	while ((curr = pktq_pdeq(&sdpcmd->txq, SDPCM_TX_PREC_DATA))) {
		pktenq(&tmpq, curr);

		/* No other action if past glommable packets */
		if (stop_glom) {
			continue;
		}

		/* Other specific packet checks: refuse to glom control packets */
		header = (uchar *) PKTDATA(sdpcmd->osh, curr) + SDPCM_FRAMETAG_LEN;
		if (SDPCM_GLOMDESC(header) ||
			(SDPCM_PACKET_CHANNEL(header) == SDPCM_CONTROL_CHANNEL)) {
			stop_glom = TRUE;
			continue;
		}

		/* Other specific checks: possibly refuse to glom event packets */
		if (SDPCM_PACKET_CHANNEL(header) == SDPCM_EVENT_CHANNEL) {
			if (!sdpcmd_event_pkt_glom(sdpcmd, curr)) {
				stop_glom = TRUE;
				continue;
			}
		}

		++numdesc;
		while (PKTNEXT(sdpcmd->osh, curr)) {
			curr = PKTNEXT(sdpcmd->osh, curr);
			++numdesc;
		}

		/* If [still] room in DMA, count packet as glommable */
		if (numdesc <= availdesc) {
			++glompkts;
		}
	}

	/* put them all back on the tx queue */
	while ((curr = pktdeq(&tmpq)))
		pktq_penq(&sdpcmd->txq, SDPCM_TX_PREC_DATA, curr);

	pktq_deinit(&tmpq);

	/* Now glompkts is max we can currently glom (txq + DMA). Bail if no benefit */
	if (glompkts <= 2) {
		return;
	}

	/* Cap the amount at the tunable level */
	if (glompkts > sdpcmd->tunables[MAXTXPKTGLOM]) {
		glompkts = sdpcmd->tunables[MAXTXPKTGLOM];
	}

	/* synthesize a superframe descriptor to transmit first */
	len = sdpcmd->tunables[DONGLEHDRSZ] + (glompkts * sizeof(uint16));
	sf_descp = PKTGET(sdpcmd->osh, len + sdpcmd->tunables[DONGLEPADSZ], TRUE);
	if (!sf_descp)
		return;
	PKTSETLEN(sdpcmd->osh, sf_descp, len);
	sf_desc = PKTDATA(sdpcmd->osh, sf_descp);
	bzero(sf_desc, len);
	/* insert hw header: 2 byte len followed by 2 byte ~len check value */
	len = htol16(len);

	/* gspi: dword mode */
	memcpy(sf_desc, &len, sizeof(uint16));

	len = ~len;
	memcpy(sf_desc + sizeof(uint16), &len, sizeof(uint16));
	/* get offset to length fields of superframe descriptor */
	lenp = (uint16 *) (sf_desc + SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN);

	sflen = 0;
	prev = sf = NULL;
	while ((curr = pktq_pdeq(&sdpcmd->txq, SDPCM_TX_PREC_DATA))) {
		/* find total length (packet may be chained), and locate last segment */
		p = curr;
		len = PKTLEN(sdpcmd->osh, curr);
		while (PKTNEXT(sdpcmd->osh, p)) {
			p = PKTNEXT(sdpcmd->osh, p);
			len += PKTLEN(sdpcmd->osh, p);
		}

		header = (uchar *) PKTDATA(sdpcmd->osh, curr) + SDPCM_FRAMETAG_LEN;
		sequence = SDPCM_PACKET_SEQUENCE(header);
		/* chain the packet into the superframe */
		if (prev)
			PKTSETNEXT(sdpcmd->osh, prev, curr);
		else {
			/* superframe descriptor takes sequence number of first frame */
			swheader[0] = sequence | SDPCM_GLOMDESC_FLAG |
			        ((SDPCM_GLOM_CHANNEL << SDPCM_CHANNEL_SHIFT) &
			         SDPCM_CHANNEL_MASK) |
			        (((SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN)
			          << SDPCM_DOFFSET_SHIFT) & SDPCM_DOFFSET_MASK);
			swheader[0] = htol32(swheader[0]);
			memcpy(sf_desc + SDPCM_FRAMETAG_LEN, &swheader[0],
			       SDPCM_SWHEADER_LEN);
			sf_desc[SDPCM_FRAMETAG_LEN + SDPCM_WINDOW_OFFSET] =
			        sdpcmd->rx_hlim = sdpcmd->rx_lim;

			/* account for uberheader on first packet in chain */
			len += SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN;

			sf = curr;
		}

		/* shift sequence number space forward by one & update curr with it */
		++sequence;
		memcpy(&swheader[0], header, SDPCM_SWHEADER_LEN);
		swheader[0] = ltoh32(swheader[0]);
		swheader[0] = sequence | (swheader[0] & ~SDPCM_SEQUENCE_MASK);
		swheader[0] = htol32(swheader[0]);
		memcpy(header, &swheader[0], SDPCM_SWHEADER_LEN);
		header[SDPCM_WINDOW_OFFSET] = sdpcmd->rx_hlim = sdpcmd->rx_lim;

		/* round packet len to specified glom segment alignment */
		rounduplen = ROUNDUP(len, sdpcmd->tunables[TXGLOMALIGN]);
#ifdef BCM_DMAPAD
		/* set the roundup difference as dma padding at the end of chain */
		PKTSETDMAPAD(sdpcmd->osh, p, (rounduplen - len));
#else
		/* add the roundup difference to the last packet in the chain */
		PKTSETLEN(sdpcmd->osh, p, PKTLEN(sdpcmd->osh, p) + (rounduplen - len));
#endif
		sflen += rounduplen;
		/* add curr's len to the superframe descriptor */
		rounduplen = htol16(rounduplen);
		memcpy(lenp++, &rounduplen, sizeof(uint16));

		prev = p;

		/* If this was the last packet to glom, leave loop */
		if (--glompkts == 0) {
			break;
		}
	}

	/* Move remaining packets to temp queue, bumping sequence numbers */
	while ((curr = pktdeq(&sdpcmd->txq))) {
		header = (uchar *) PKTDATA(sdpcmd->osh, curr) + SDPCM_FRAMETAG_LEN;
		sequence = SDPCM_PACKET_SEQUENCE(header);
		++sequence;
		memcpy(&swheader[0], header, SDPCM_SWHEADER_LEN);
		swheader[0] = ltoh32(swheader[0]);
		swheader[0] = sequence | (swheader[0] & ~SDPCM_SEQUENCE_MASK);
		swheader[0] = htol32(swheader[0]);
		memcpy(header, &swheader[0], SDPCM_SWHEADER_LEN);
		pktenq(&tmpq, curr);
	}

	/* enqueue the superframe descriptor first */
	pktq_penq(&sdpcmd->txq, SDPCM_TX_PREC_DATA, sf_descp);

	/* sf uberheader: reuse the first frame's sequence number */
	header = (uchar *) PKTDATA(sdpcmd->osh, sf) + SDPCM_FRAMETAG_LEN;
	sequence = SDPCM_PACKET_SEQUENCE(header);

	/* sf uberheader: push on the extra header */
	ASSERT(PKTHEADROOM(sdpcmd->osh, sf) >= (SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN));
	PKTPUSH(sdpcmd->osh, sf, SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN);
	pktdata = PKTDATA(sdpcmd->osh, sf);
	/* insert hw header: 2 byte sflen followed by 2 byte ~sflen check value */
	sflen = htol16(sflen);

	memcpy(pktdata, &sflen, sizeof(uint16));

	sflen = ~sflen;
	memcpy(pktdata + sizeof(uint16), &sflen, sizeof(uint16));
	/* insert sw header */
	swheader[0] = sequence |
	        ((SDPCM_GLOM_CHANNEL << SDPCM_CHANNEL_SHIFT) & SDPCM_CHANNEL_MASK) |
	        (((SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN) << SDPCM_DOFFSET_SHIFT) &
	         SDPCM_DOFFSET_MASK);
	swheader[0] = htol32(swheader[0]);
	memcpy(pktdata + SDPCM_FRAMETAG_LEN, &swheader[0], SDPCM_SWHEADER_LEN);
	pktdata[SDPCM_FRAMETAG_LEN + SDPCM_WINDOW_OFFSET] =
	        sdpcmd->rx_hlim = sdpcmd->rx_lim;

	/* enqueue the superframe itself */
	pktq_penq(&sdpcmd->txq, SDPCM_TX_PREC_DATA, sf);

	/* requeue unglommed frames that follow */
	while ((curr = pktdeq(&tmpq))) {
		pktenq(&sdpcmd->txq, curr);
	}

	/* account for the added superframe descriptor frame */
	++sdpcmd->tx_seq;
}

static void
sdpcmd_txlazytimer(dngl_timer_t *t)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *) hnd_timer_get_ctx(t);
	int qlen = pktq_mlen(&sdpcmd->txq, (1 << SDPCM_TX_PREC_DATA));

	sdpcmd->txlazytimer_running = FALSE;



	/* Depending on mode, either we've waited a full glom timer since the
	 * last send, or there's been a timer-sized gap in the flow; either way,
	 * stop waiting and send what we've got.
	 */
	if (sdpcmd->tunables[TXGLOM] && (qlen > 1)) {
		sdpcmd_txglom(sdpcmd);
	}

	sdpcmd_sendnext(sdpcmd);
}
#endif /* TXGLOM_CODE */

#ifdef DBG_DS_TIMER_WAKE
static uint32 gptimer_start_cnt = 0;
static uint32 gptimer_tmo_cnt = 0;
#endif /* DBG_DS_TIMER_WAKE */

static void
sdpcmd_gptimer(dngl_timer_t *t)
{
#ifdef DBG_DS_TIMER_WAKE
	++gptimer_tmo_cnt;
	OSL_DELAY(500);
#else /* DBG_DS_TIMER_WAKE */
	struct dngl_bus *sdpcmd = (struct dngl_bus *)hnd_timer_get_ctx(t);

	/* Toggle the bit and set the appropriate timer value */
	if (sdpcmd->gpon) {
		si_gpioout(sdpcmd->sih, sdpcmd->gpbit,
		           (sdpcmd->gpval ^ sdpcmd->gpbit), GPIO_HI_PRIORITY);
		dngl_add_timer(sdpcmd->gptimer, sdpcmd->gpofftime, FALSE);
		sdpcmd->gpon = FALSE;
	} else {
		si_gpioout(sdpcmd->sih, sdpcmd->gpbit, sdpcmd->gpval, GPIO_HI_PRIORITY);
		dngl_add_timer(sdpcmd->gptimer, sdpcmd->gpontime, FALSE);
		sdpcmd->gpon = TRUE;
	}
#endif /* DBG_DS_TIMER_WAKE */
}

static void
sdpcmd_request_oobintr(struct dngl_bus *sdpcmd)
{
	if (sdpcmd->gpbit == 0) {
		return;
	}

	/* Assert GPIO output */
	si_gpioout(sdpcmd->sih, sdpcmd->gpbit, sdpcmd->gpval, GPIO_HI_PRIORITY);

	/* Negate GPIO output */
	si_gpioout(sdpcmd->sih, sdpcmd->gpbit, sdpcmd->gpval ^ sdpcmd->gpbit,
		GPIO_HI_PRIORITY);
}

static void
sdpcmd_wakehost(struct dngl_bus *sdpcmd)
{
	/* If using OOB-ONLY request GPIO OOB interrupt generation */
	if (sdpcmd->oob_only) {
		++sdpcmd->metrics.wakehost_cnt;
		sdpcmd_request_oobintr(sdpcmd);
		return;
	}

	/* If not using OOB or no GPIO selected, nothing we can do */
	if (!sdpcmd->use_oob || (sdpcmd->gpbit == 0))
		return;

	/* If the duty-cycle timer is already running, nothing more to do */
	if (sdpcmd->gptimer_active)
		return;

	++sdpcmd->metrics.wakehost_cnt;
	/* Drive GPIO output active */
	si_gpioout(sdpcmd->sih, sdpcmd->gpbit, sdpcmd->gpval, GPIO_HI_PRIORITY);
	sdpcmd->gpon = TRUE;

	/* If we have toggling configured, start the duty-cycle timer */
	if (sdpcmd->gpontime) {
		dngl_add_timer(sdpcmd->gptimer, sdpcmd->gpontime, FALSE);
		sdpcmd->gptimer_active = TRUE;
	}
}

#ifdef BCMDBG_POOL
static void
sdpcmd_pktpool_dbg_cb(pktpool_t *pool, void *arg)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *) arg;

	if (sdpcmd == NULL)
		return;

	if (sdpcmd->rxreclaim == TRUE)
		return;

	printf("sd: post=%d rxactive=%d txactive=%d txpend=%d pool=%d\n",
		sdpcmd->tunables[RXBUFS],
		dma_rxactive(sdpcmd->di),
		dma_txactive(sdpcmd->di),
		dma_txpending(sdpcmd->di),
		pktpool_avail(pool));
}
#endif /* BCMDBG_POOL */

static void
sdpcmd_pktpool_empty_cb(pktpool_t *pool, void *arg)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *) arg;
	uint32 intstatus;

	if (sdpcmd == NULL)
		return;

	if (sdpcmd->rxreclaim == TRUE)
		return;

	if (sdpcmd->up == FALSE)
		return;

	intstatus = R_REG(sdpcmd->osh, &sdpcmd->regs->intstatus);
	if (intstatus & (I_XI|I_DMA)) {
		W_REG(sdpcmd->osh, &sdpcmd->regs->intstatus, I_XI);

		sdpcmd_txreclaim(sdpcmd);
	}
}

static void
sdpcmd_pktpool_avail_cb(pktpool_t *pool, void *arg)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *) arg;

	if (sdpcmd == NULL)
		return;

	if ((sdpcmd->di == NULL) || (sdpcmd->up == FALSE))
		return;

	if (sdpcmd->rxreclaim == TRUE)
		return;

	if (!sdpcmd->tunables[RXCB])
		return;

	if (sdpcmd->di && (dma_rxactive(sdpcmd->di) < sdpcmd->tunables[RXFILLTRIG])) {
		dma_rxfill(sdpcmd->di);
		sdpcmd->rx_lim = sdpcmd->rx_seq + dma_rxactive(sdpcmd->di);
		if (sdpcmd->tunables[CREDALL] ||
		    ((sdpcmd->rx_lim != sdpcmd->rx_hlim) &&
		    ((uint8)(sdpcmd->rx_hlim - sdpcmd->rx_seq) <= sdpcmd->credtrig))) {
			sdpcmd_sendheader(sdpcmd);
		}
	}
}
static const char BCMATTACHDATA(rstr_SDIODEV)[] = "SDIODEV";
static const char BCMATTACHDATA(rstr_PCMCIADEV)[] = "PCMCIADEV";
static const char BCMATTACHDATA(rstr_sd_gpout)[] = "sd_gpout";
static const char BCMATTACHDATA(rstr_sd_gpval)[] = "sd_gpval";
static const char BCMATTACHDATA(rstr_sd_gpdc)[] = "sd_gpdc";
static const char BCMATTACHDATA(rstr_sd_oobonly)[] = "sd_oobonly";
static const char BCMATTACHDATA(rstr_gspidword)[] = "gspidword";
static const char BCMATTACHDATA(rstr_GSPI_Dword_Mode_enabled)[] = "GSPI Dword Mode enabled\n";
static const char BCMATTACHDATA(rstr_spi_pu_enab)[] = "spi_pu_enab";
static const char BCMATTACHDATA(rstr_drive_strength)[] = "sd_drvstr_ma";

static int
BCMATTACHFN(sdpcmd_phdr_attach)(struct dngl_bus *sdpcmd)
{
	int err;
	int n = SDHDR_POOL_LEN;

	if ((sdpcmd->pktpool_phdr = MALLOC(sdpcmd->osh, sizeof(pktpool_t))) == NULL) {
		ASSERT(0);
		return BCME_ERROR;
	}
	bzero(sdpcmd->pktpool_phdr, sizeof(pktpool_t));

	err = pktpool_init(sdpcmd->osh, sdpcmd->pktpool_phdr, &n,
		(BCMDONGLEHDRSZ + BCMDONGLEPADSZ), FALSE, lbuf_basic);

	if (err == BCME_ERROR) {
		ASSERT(0);
		MFREE(sdpcmd->osh, sdpcmd->pktpool_phdr, sizeof(pktpool_t));
		sdpcmd->pktpool_phdr = NULL;
		return err;
	}

	if ((err == 0) && (n < SDHDR_POOL_LEN))
		SD_ERROR(("SD partial hdrs pkt pool allocated: %d %d\n", n, SDHDR_POOL_LEN));

	return err;
}

static int
BCMATTACHFN(sdpcmd_phdr_detach)(struct dngl_bus *sdpcmd)
{
	int err = 0;

	if (sdpcmd->pktpool_phdr) {
		err = pktpool_deinit(sdpcmd->osh, sdpcmd->pktpool_phdr);
		MFREE(sdpcmd->osh, sdpcmd->pktpool_phdr, sizeof(pktpool_t));
		sdpcmd->pktpool_phdr = NULL;
	}

	return err;
}

static void
BCMATTACHFN(sdpcmd_tunables_init)(struct dngl_bus *sdpcmd)
{
	/* set tunables defaults (RXACK and TXDROP are stats) */
	sdpcmd->tunables[MAXTXPKTGLOM] = DEFMAXTXPKTGLOM;
	sdpcmd->tunables[TXLAZYDELAY] = DEFTXLAZYDELAY;
	sdpcmd->tunables[TXGLOMALIGN] = DEFTXGLOMALIGN;
	sdpcmd->tunables[TXGLOM] = DEFTXGLOM;
	sdpcmd->tunables[TXSWQLEN] = DEFTXSWQLEN;
	sdpcmd->tunables[ACKFASTFWD] = DEFACKFASTFWD;
	sdpcmd->tunables[ACKSIZETHSD] = DEFACKSIZETHSD;
	sdpcmd->tunables[NTXD] = SDPCMD_NTXD;
	sdpcmd->tunables[NRXD] = SDPCMD_NRXD;
	sdpcmd->tunables[RXBUFS] = SDPCMD_RXBUFS;
	sdpcmd->tunables[RXBUFSZ] = SDPCMD_RXBUFSZ;

	if (sdpcmd->tunables[RXBUFS] >= sdpcmd->tunables[NRXD])
		sdpcmd->tunables[RXBUFS] = sdpcmd->tunables[NRXD] - 1;

	/* For smaller DMA rings, use larger SW queue */
	if (SDPCMD_NTXD <= 8) {
		sdpcmd->tunables[DATAHIWAT] = SDPCMD_NTXD;
		sdpcmd->tunables[DATALOWAT] = SDPCMD_NTXD / 2;
		sdpcmd->tunables[TXQ_MAX] = SDPCMD_NTXD * 2;
	} else {
		sdpcmd->tunables[DATAHIWAT] = SDPCMD_NTXD * 3 / 4;
		sdpcmd->tunables[DATALOWAT] = SDPCMD_NTXD / 4;
		sdpcmd->tunables[TXQ_MAX] = SDPCMD_NTXD;
	}
	sdpcmd->tunables[ALIGNMOD] = SDPCM_ALIGNMOD;
	sdpcmd->tunables[RXBND] = SDPCMD_RXBND;
	sdpcmd->tunables[DONGLEOVERHEAD] = BCMDONGLEOVERHEAD;
	/* Sanity check for header size/reservation consistency */
	ASSERT(BCMDONGLEHDRSZ >= (SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN));
	sdpcmd->tunables[DONGLEHDRSZ] = BCMDONGLEHDRSZ;
	sdpcmd->tunables[DONGLEPADSZ] = BCMDONGLEPADSZ;
	sdpcmd->tunables[BDCHEADERLEN] = BDC_HEADER_LEN;
	sdpcmd->tunables[RXFILLTRIG] = sdpcmd->tunables[RXBUFS];
	sdpcmd->tunables[CREDALL] = 0;
	sdpcmd->tunables[RXCB] = FALSE;
	sdpcmd->initrxbufs = sdpcmd->tunables[RXBUFS];
	sdpcmd->credtrig = DEFCREDTRIG;
	sdpcmd->minsched = DEFMINSCHED;
	sdpcmd->glomevents = 3;
	sdpcmd->dslogsz_tunable = DEFAULT_DS_LOG_SIZE;
}

bool
sdpcmd_sdioaccess(int *data, uint16 regspace, uint8 addr, uint32 accesstype,
		osl_t *osh, volatile void *sdio_regs)
{
	uint32 sdioaccess;
	sdpcmd_regs_t *regs = (sdpcmd_regs_t *)sdio_regs;

	SPINWAIT(((sdioaccess = R_REG(osh, &regs->sdioaccess)) & SDA_BUSY), 1000000);
	if (sdioaccess & SDA_BUSY) {
		return FALSE;
	}
	sdioaccess = ((uint32)(addr + regspace) << SDA_ADDR_SHIFT) | SDA_BUSY;
	if (accesstype == SDA_WRITE) {
		sdioaccess += (uint32) *data | accesstype;
	}
	W_REG(osh, &regs->sdioaccess, sdioaccess);
	SPINWAIT(((sdioaccess = R_REG(osh, &regs->sdioaccess)) & SDA_BUSY), 1000000);
	if (sdioaccess & SDA_BUSY) {
		return FALSE;
	}

	*data = ((sdioaccess & (SDA_DATA_MASK | SDA_ADDR_MASK))| SDA_WRITE | SDA_BUSY);
	return TRUE;
}

/* Global function for host notification */
static sdpcmd_regs_t *sdpcmd_regs = NULL;
static void
sdpcmd_fwhalt(void *arg, uint32 trap_data)
{
	if (sdpcmd_regs) {
		sdpcmd_regs->tohostmailboxdata |= HMB_DATA_FWHALT;
		sdpcmd_regs->tohostmailbox = HMB_HOST_INT;
	}
}

static int
sdpcmd_handle_trap(void *dngl, uint8 trap_type)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)dngl;

	printf("sdpcmd_handle_trap: di suspend %d\n", sdpcmd->di_dma_suspend_for_trap);

	/* suspend */
	if (sdpcmd->di) {
		if (!dma_txsuspended(sdpcmd->di)) {
			sdpcmd->di_dma_suspend_for_trap = TRUE;
			dma_txsuspend(sdpcmd->di);
		}
	}

	return ((1 << si_coreidx(sdpcmd->sih)));
}

/* Allocate chip private state and initialize */
struct dngl_bus *
BCMATTACHFN(sdpcmd_attach)(void *drv, uint vendor, uint device, osl_t *osh,
                           volatile void *regs, uint bustype)
{
	const char *gpvar;
	bool sdhdr_pktpool = FALSE;
	struct dngl_bus *sdpcmd;
	uint32 txq_init_len = DEFTXSWQLEN;

	SD_TRACE(("sdpcmd_attach\n"));

	sdpcmd_regs = regs;

	if (!sdpcmd_match(vendor, device)) {
		SD_ERROR(("%s: sdpcmd_match failed\n", __FUNCTION__));
		return NULL;
	}

	/* Allocate chip state */
	if (!(sdpcmd = MALLOC(osh, sizeof(struct dngl_bus)))) {
		SD_ERROR(("%s: out of memory, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(osh)));
		return NULL;
	}

	bzero(sdpcmd, sizeof(struct dngl_bus));
	sdpcmd->osh = osh;
	sdpcmd->regs = regs;

	sdpcmd_tunables_init(sdpcmd);

#ifdef BCMPKTPOOL
	sdpcmd->pktpool = SHARED_POOL;
	if (POOL_ENAB(sdpcmd->pktpool))
		sdhdr_pktpool = TRUE;
#endif /* BCMPKTPOOL */

#ifdef MSGTRACE
	sdpcmd->msgtrace = TRUE;
#endif
#ifdef LOGTRACE
	sdpcmd->logtrace = TRUE;
#endif

	/* Initialize tx packet queue (+1 to allow for retrans packet) */
	pktq_init(&sdpcmd->txq, SDPCM_TX_PREC_MAX, txq_init_len + 1);


	/* Attach to SB bus */
	if (!(sdpcmd->sih = si_attach(device, osh, regs, bustype, NULL, NULL, 0))) {
		SD_ERROR(("%s: sd_attach failed\n", __FUNCTION__));
		goto fail;
	}


	sdpcmd->coreid = si_coreid(sdpcmd->sih);
	sdpcmd->corerev = si_corerev(sdpcmd->sih);
	sdpcmd->sdio = (R_REG(sdpcmd->osh, &sdpcmd->regs->corestatus) & CS_PCMCIAMODE) == 0;

	sdpcmd->di = dma_attach(sdpcmd->osh,
	                        sdpcmd->sdio ? rstr_SDIODEV : rstr_PCMCIADEV,
	                        sdpcmd->sih,
	                        SDPCMDMAREG(sdpcmd, DMA_TX, 0, sdpcmd->coreid),
	                        SDPCMDMAREG(sdpcmd, DMA_RX, 0, sdpcmd->coreid),
	                        sdpcmd->tunables[NTXD], sdpcmd->tunables[NRXD],
	                        sdpcmd->tunables[RXBUFSZ], -1,
	                        sdpcmd->tunables[RXBUFS], SDPCMD_RXOFFSET, NULL);
	if (!(sdpcmd->di)) {
		SD_ERROR(("%s: dma_attach failed\n", __FUNCTION__));
		goto fail;
	}

	if (POOL_ENAB(sdpcmd->pktpool)) {
		/* set pool for rx dma */
		dma_pktpool_set(sdpcmd->di, sdpcmd->pktpool);

		/* Register to be notified when pktpool is available which can
		 * happen outside this scope from wl side.
		 */
		pktpool_avail_register(sdpcmd->pktpool,
			sdpcmd_pktpool_avail_cb, sdpcmd);
		pktpool_empty_register(sdpcmd->pktpool,
			sdpcmd_pktpool_empty_cb, sdpcmd);
#ifdef BCMDBG_POOL
		pktpool_dbg_register(sdpcmd->pktpool, sdpcmd_pktpool_dbg_cb, sdpcmd);
#endif
		if (sdhdr_pktpool) {
			if (sdpcmd_phdr_attach(sdpcmd))
				goto fail;
		}
	}

	dma_burstlen_set(sdpcmd->di, DMA_BL_64, DMA_BL_64);

	/* Initialize dongle device */
	if (!(sdpcmd->dngl = dngl_attach(sdpcmd, NULL, sdpcmd->sih, osh))) {
		SD_ERROR(("%s: dngl_attach failed\n", __FUNCTION__));
		goto fail;
	}
#ifdef TXGLOM_CODE
	sdpcmd->txlazytimer = dngl_init_timer(sdpcmd, NULL, sdpcmd_txlazytimer);
	if (!sdpcmd->txlazytimer) {
		SD_ERROR(("%s: init txlazytimer failed\n", __FUNCTION__));
		goto fail;
	}
#endif /* TXGLOM_CODE */

	/* Initialize gpio output fields */
	sdpcmd->use_oob = FALSE;

	gpvar = nvram_get(rstr_sd_gpout);
	if (gpvar) {
		uint32 gpdc;
		sdpcmd->gpbit = (1 << bcm_atoi(gpvar));
		gpvar = nvram_get(rstr_sd_gpval);
		sdpcmd->gpval = (gpvar && !bcm_atoi(gpvar)) ? 0 : sdpcmd->gpbit;
		gpdc = getintvar(NULL, rstr_sd_gpdc);
		sdpcmd->gpontime = (gpdc >> 16);
		sdpcmd->gpofftime = gpdc;
#ifdef BCMDBG
		if (!sdpcmd->gpontime != !sdpcmd->gpofftime) {
			SD_ERROR(("Invalid GPIO duty cycle: 0x%08x (%d on, %d off)\n",
			          gpdc, sdpcmd->gpontime, sdpcmd->gpofftime));
			sdpcmd->gpofftime = sdpcmd->gpontime = 0;
		}
#endif /* BCMDBG */

		if (sdpcmd->gpontime) {
			sdpcmd->gptimer = dngl_init_timer(sdpcmd, NULL, sdpcmd_gptimer);
			if (!sdpcmd->gptimer) {
				SD_ERROR(("%s: init gptimer failed\n", __FUNCTION__));
				goto fail;
			}
		}

		if (CHIPID(sdpcmd->sih->chip) == BCM4345_CHIP_ID) {
			/* First get the GPIO pin */
			uint8 gpio;
			uint32 mask = sdpcmd->gpbit; /* GPIO pin mask */

			for (gpio = 0; gpio < CC4345_PIN_GPIO_15; gpio ++) {
				if ((mask >> gpio) & 0x1)
					break;
			}
			si_gci_enable_gpio(sdpcmd->sih, gpio, mask, sdpcmd->gpval);
		}
	}

	if (getintvar(NULL, rstr_sd_oobonly) == 1) {
		sdpcmd->oob_only = TRUE;
	}

#ifdef DS_PROT
#if !defined(DS_PROT_DISABLED)
	sdpcmd->_ds_prot = TRUE;
#endif
	if (DS_PROT_ENAB(sdpcmd)) {
		/* Start in wake (NO_DS), deepsleep disabled */
		sdpcmd->ds_state = NO_DS_STATE;
		sdpcmd->ds_check_timer = dngl_init_timer(sdpcmd, NULL, sdpcmd_ds_check_timerfn);
		if (!sdpcmd->ds_check_timer) {
			SD_ERROR(("%s: init ds_check_timer failed\n", __FUNCTION__));
			goto fail;
		}
		sdpcmd->ds_check_timer_on = FALSE;
		sdpcmd->ds_check_interval = SDPCM_DS_CHECK_INTERVAL;
		sdpcmd->ds_check_timer_max = SDPCM_DS_CHECK_MAX_ITERATION;
		sdpcmd->ds_int_dtoh_mode = 1;
		sdpcmd->ds_check_print = FALSE;
		sdpcmd->ds_inform_print = FALSE;
		sdpcmd->ds_trace_print = FALSE;
		sdpcmd->ds_always_delay = TRUE;

		if (sdpcmd_enable_device_wake(sdpcmd) != BCME_OK) {
			SD_INFORM(("%s: devwake not enabled\n", __FUNCTION__));
		}

		sdpcmd->ds_hwait_timer = dngl_init_timer(sdpcmd, NULL, sdpcmd_ds_hwait_timerfn);
		if (!sdpcmd->ds_hwait_timer) {
			SD_ERROR(("%s: init ds_hwait_timer failed\n", __FUNCTION__));
			goto fail;
		}
		sdpcmd->ds_hwait = FALSE;
	}
#endif /* DS_PROT */

#ifdef RTE_CONS
	if (!hnd_cons_add_cmd("busstats", sdpcmd_dumpstats, sdpcmd))
		goto fail;
#ifdef BCMDBG_SD_LATENCY
	if (!hnd_cons_add_cmd("sdlat", sdpcmd_print_latency, sdpcmd))
		goto fail;
#endif
#endif /* RTE_CONS */

	/* Set halt handler */
	hnd_set_fwhalt(sdpcmd_fwhalt, sdpcmd);
	hnd_register_trapnotify_callback(sdpcmd_handle_trap, sdpcmd);

#ifdef DEBUG_PARAMS_ENABLED
	debug_params.tx_seq = &sdpcmd->tx_seq;
	debug_params.rx_seq = &sdpcmd->rx_seq;
	debug_params.rx_lim = &sdpcmd->rx_lim;
	debug_params.rx_hlim = &sdpcmd->rx_hlim;
	debug_params.txq = &sdpcmd->txq;
	debug_params.txavail = sdpcmd->txavail;
#endif /* DEBUG_PARAMS_ENABLED */

#ifdef BCMULP
	if (BCMULP_ENAB()) {
		if (BCME_OK != ulp_p1_module_register(ULP_MODULE_ID_SDIO,
			&ulp_sdpcmd_ctx, (void*)sdpcmd)) {
			goto fail;
		}
	}
#endif /* BCMULP */

	sdpcmd->rxfill_dpc = hnd_dpc_register(_sdpcmd_rxfill, sdpcmd, NULL);
	if (sdpcmd->rxfill_dpc == NULL) {
		SD_ERROR(("%s: hnd_dpc_register failed\n", __FUNCTION__));
		goto fail;
	}

	/* Set SDIO drive strength override */
	sdpcmd->drive_strength_ma = getintvar(NULL, rstr_drive_strength);
	if (sdpcmd->drive_strength_ma) {
		si_set_sdio_drive_strength(sdpcmd->sih, sdpcmd->drive_strength_ma);
		SD_INFORM(("%s: SDIO drive strength %u ma\n", __FUNCTION__,
			sdpcmd->drive_strength_ma));
	}
	return sdpcmd;

fail:
	if (sdpcmd)
		MFREE(sdpcmd->osh, sdpcmd, sizeof(struct dngl_bus));
	return NULL;
}

/* Reset and free chip private state */
void
BCMATTACHFN(sdpcmd_detach)(struct dngl_bus *sdpcmd)
{
	SD_TRACE(("sdpcmd_detach\n"));

	sdpcmd->up = FALSE;

	/* Free device state */
	dngl_detach(sdpcmd->dngl);

	/* Put the core back into reset */
	sdpcmd_reset(sdpcmd);
	si_core_disable(sdpcmd->sih, 0);

	if (sdpcmd->txlazytimer) {
		dngl_del_timer(sdpcmd->txlazytimer);
		dngl_free_timer(sdpcmd->txlazytimer);
	}

	if (sdpcmd->gptimer) {
		dngl_del_timer(sdpcmd->gptimer);
		dngl_free_timer(sdpcmd->gptimer);
	}

#ifdef DS_PROT
	if (DS_PROT_ENAB(sdpcmd)) {
		if (sdpcmd->ds_check_timer) {
			dngl_del_timer(sdpcmd->ds_check_timer);
			dngl_free_timer(sdpcmd->ds_check_timer);
		}
		if (sdpcmd->ds_hwait_timer) {
			dngl_del_timer(sdpcmd->ds_hwait_timer);
			dngl_free_timer(sdpcmd->ds_hwait_timer);
		}
	}
#endif /* DS_PROT */

	/* Detach from SB bus */
	si_detach(sdpcmd->sih);

	if (PHDR_ENAB(sdpcmd))
		sdpcmd_phdr_detach(sdpcmd);

	pktq_deinit(&sdpcmd->txq);

	/* Free chip state */
	MFREE(sdpcmd->osh, sdpcmd, sizeof(struct dngl_bus));
}

void *
sdpcmd_dngl(struct dngl_bus *sdpcmd)
{
	return sdpcmd->dngl;
}

/* reset the core */
static void
sdpcmd_reset(struct dngl_bus *sdpcmd)
{
	SD_ERROR(("%s\n", __FUNCTION__));

	dma_rxreset(sdpcmd->di);
	dma_txreset(sdpcmd->di);

	sdpcmd_rxreclaim(sdpcmd);
	dma_txreclaim(sdpcmd->di, HNDDMA_RANGE_ALL);

	/* Reset core */
	si_core_reset(sdpcmd->sih, 0, 0);

	sdpcmd->tx_seq = sdpcmd->rx_seq = 0;
	sdpcmd->tohostmail = 0;

	/* free tx buffer held for possible retransmission */
	if (sdpcmd->held_tx) {
		PKTFREE(sdpcmd->osh, sdpcmd->held_tx, TRUE);
		sdpcmd->held_tx = NULL;
	}

	/* clear last pending tx pointer */
	sdpcmd->pend_tx = NULL;

	sdpcmd->intstatus = 0;
	sdpcmd->disabled = FALSE;
}

/* Reinitialize core from any state */
void
sdpcmd_init(struct dngl_bus *sdpcmd)
{
	const char *gpvar;

	SD_TRACE(("sdpcmd_init\n"));

	dma_txinit(sdpcmd->di);
	dma_rxinit(sdpcmd->di);
	sdpcmd_rxfill(sdpcmd);

	sdpcmd->txavail = (uint *) dma_getvar(sdpcmd->di, "&txavail");

	/* If using gpio output, initialize inactive and enable output */
	if (sdpcmd->gpbit) {
		si_gpioout(sdpcmd->sih, sdpcmd->gpbit,
		           (sdpcmd->gpval ^ sdpcmd->gpbit), GPIO_HI_PRIORITY);
		si_gpioouten(sdpcmd->sih, sdpcmd->gpbit, sdpcmd->gpbit, GPIO_HI_PRIORITY);
		sdpcmd->gpon = FALSE;
		ASSERT(!sdpcmd->gptimer_active);
	}

	gpvar = nvram_get("sd_gpout");

	if (gpvar) {
		uint32 gpout;
		gpout = bcm_atoi(gpvar);
		switch (CHIPID(sdpcmd->sih->chip)) {
			case BCM4335_CHIP_ID:
				si_gci_set_functionsel(sdpcmd->sih, gpout, CC4335_FNSEL_SAMEASPIN);
				break;
			case BCM4350_CHIP_ID:
			case BCM4354_CHIP_ID:
			case BCM4358_CHIP_ID:
				si_gci_set_functionsel(sdpcmd->sih, gpout, CC4350_FNSEL_SAMEASPIN);
				break;
			case BCM43018_CHIP_ID:
			case BCM43430_CHIP_ID:
				si_gci_set_functionsel(sdpcmd->sih, gpout, CC43430_FNSEL_SAMEASPIN);
				break;
			default:
				;
		}
	}

	if (sdpcmd->sdio) {
		sdpcmd->defintmask = DEF_SDIO_INTMASK;
#if TEMP_DEBUG
		sdpcmd->defintmask |= I_SBINT;
#endif
	} else {
		sdpcmd->defintmask = DEF_INTMASK;
#if TEMP_DEBUG
		sdpcmd->defintmask |= (I_SBINT | I_PCMCIA_XU);
#endif
	}

#if TEMP_DEBUG
	sdpcmd->defsbintmask = DEF_SB_INTMASK;
	/* enable select sonics bus interrupts */
	W_REG(sdpcmd->osh, &sdpcmd->regs->sbintmask, sdpcmd->defsbintmask);
#endif

	sdpcmd->up = TRUE;
	sdpcmd->tohostmailacked = TRUE;

	/* Make sure intstatus gets set for each frame received */
	W_REG(sdpcmd->osh, &sdpcmd->regs->intrcvlazy, DEF_IRL);

	sdpcmd->txlazytimer_running = FALSE;

	/* Enable interrupts */
	sdpcmd_intrson(sdpcmd);

	/* Interrupt host indicating firmware is ready for protocol activity (post ifconfig) */
	hostmailbox_post(sdpcmd, HMB_DATA_FWREADY);

	/* Will also get IOE2 interrupt each time the host enables F2 */

#ifdef DS_PROT
	if (DS_PROT_ENAB(sdpcmd)) {
		if (sdpcmd->dslogsz_tunable > 0)
			sdpcmd_ds_log_init(sdpcmd, sdpcmd->dslogsz_tunable);
	}
#endif /* DS_PROT */
}

static void
sdpcmd_process_ioe2(struct dngl_bus *sdpcmd)
{
	uint32 corestatus;
	corestatus = R_REG(sdpcmd->osh, &sdpcmd->regs->corestatus);
	if (corestatus & CS_F2ENABLED) {
		if (!sdpcmd->disabled) {
			/* reset DMA if DISABLE interrupt was missed */
			sdpcmd_txrecompose(sdpcmd, FALSE);
			sdpcmd_rxrecompose(sdpcmd);
		}
		sdpcmd->disabled = FALSE;
		dngl_resume(sdpcmd->dngl);
		OR_REG(sdpcmd->osh, &sdpcmd->regs->corecontrol, CC_F2RDY);
		hostmailbox_post(sdpcmd, HMB_DATA_DEVREADY);
	} else {
		sdpcmd->disabled = TRUE;
		dngl_suspend(sdpcmd->dngl);
		sdpcmd_txrecompose(sdpcmd, FALSE);
		sdpcmd_rxrecompose(sdpcmd);
		AND_REG(sdpcmd->osh, &sdpcmd->regs->corecontrol, ~CC_F2RDY);
	}
}

bool
sdpcmd_dpc(struct dngl_bus *sdpcmd)
{
	uint32 intstatus;
	bool resched = FALSE;
	uint qlen;

	ASSERT(sdpcmd);

	intstatus = sdpcmd->intstatus;
	SD_TRACE(("sdpcmd_dpc: intstatus 0x%x\n", intstatus));
	sdpcmd->intstatus = 0;

	/* check for metrics */
	if (intstatus & INTMASK_BUS_METRICS_DATA)
		++sdpcmd->metrics.data_intr_cnt;
	if (intstatus & INTMASK_BUS_METRICS_MB)
		++sdpcmd->metrics.mb_intr_cnt;
	if (intstatus & INTMASK_BUS_METRICS_ERROR)
		++sdpcmd->metrics.error_intr_cnt;

	/* Any interrupt means bus is up */
	if (!sdpcmd->oob_only) {
		if (sdpcmd->use_oob && sdpcmd->gpbit) {
			si_gpioout(sdpcmd->sih, sdpcmd->gpbit,
			           (sdpcmd->gpval ^ sdpcmd->gpbit), GPIO_HI_PRIORITY);
			sdpcmd->use_oob = FALSE;
			sdpcmd->gpon = FALSE;
			if (sdpcmd->gptimer_active) {
				dngl_del_timer(sdpcmd->gptimer);
				sdpcmd->gptimer_active = FALSE;
			}
			/* Since bus is up, announce any pending flow-control change */
			if (sdpcmd->txflowpending) {
				sdpcmd_sendheader(sdpcmd);
				sdpcmd->txflowpending = FALSE;
			}
		}
	}

#ifdef DS_PROT
	if (DS_PROT_ENAB(sdpcmd)) {
		/* If host is not asleep, announce any pending flow-control change */
		if ((sdpcmd->txflowpending) &&
		    ((sdpcmd->ds_state == NO_DS_STATE) || (sdpcmd->ds_state == DS_CHECK_STATE))) {
			sdpcmd_sendheader(sdpcmd);
			sdpcmd->txflowpending = FALSE;
		}
	}
#endif /* DS_PROT */

	/* SDIO CCCR RES bit set (not for gSPI) */
	if (intstatus & I_SRESET) {
		SD_ERROR(("%s: Reset\n", __FUNCTION__));
		sdpcmd_reset(sdpcmd);
		sdpcmd_init(sdpcmd);
		goto cleanup;
	}

	/* CCCR IOE2 changed (not for gSPI) */
	if (intstatus & I_IOE2) {
		sdpcmd_process_ioe2(sdpcmd);
	}

#if TEMP_DEBUG
	/* misc SB interrupt - host should reset us */
	if (intstatus & I_SBINT) {
		if (sdpcmd->sbintstatus & I_SB_SERR) {
			SD_ERROR(("I_SB_SERR\n"));
			/* clear serror */
			if (sb_coreflagshi(sdpcmd->sih, 0, 0) & SBTMH_SERR) {
				SD_ERROR(("clearing SERR\n"));
				sb_coreflagshi(sdpcmd->sih, SBTMH_SERR, 0);
			}
		}
		if (sdpcmd->sbintstatus & I_SB_RESPERR) {
			SD_ERROR(("I_SB_RESPERR\n"));
		}
		/* sprom access error */
		if (sdpcmd->sbintstatus & I_SB_SPROMERR) {
			SD_ERROR(("I_SB_SPROMERR\n"));
		}
	}
#endif /* TEMP_DEBUG */

	if (sdpcmd->disabled)
		goto cleanup;

	/* host NAK'd a frame w/bad CRC */
	if (intstatus & I_SMB_NAK) {
		SD_ERROR(("NAK\n"));
		sdpcmd_txrecompose(sdpcmd, TRUE);
		hostmailbox_post(sdpcmd, HMB_DATA_NAKHANDLED);
	}

	/* host ACK'd a mailbox data interrupt; must be done AFTER processing I_SMB_NAK */
	if (intstatus & I_SMB_INT_ACK) {
		SD_TRACE(("INT_ACK\n"));
		DS_INFORM(("Got MB ACK\n"));
		sdpcmd->tohostmailacked = TRUE;
		if (sdpcmd->tohostmail)
			hostmailbox_post(sdpcmd, 0);
		else {
			/* clear past data if got acked */
			W_REG(sdpcmd->osh, &sdpcmd->regs->tohostmailboxdata, 0);
		}
	}

	/* write out of sync */
	if (intstatus & I_WR_OOSYNC) {
		SD_TRACE(("I_WR_OOSYNC\n"));
		++sdpcmd->cnt.woosint;
	}
	/* read out of sync */
	if (intstatus & I_RD_OOSYNC) {
		SD_ERROR(("I_RD_OOSYNC\n"));
		++sdpcmd->cnt.roosint;
		if (sdpcmd->held_tx) {
			ASSERT(sdpcmd->pend_tx != sdpcmd->held_tx);
			PKTFREE(sdpcmd->osh, sdpcmd->held_tx, TRUE);
			sdpcmd->held_tx = NULL;
		}
		sdpcmd_txrecompose(sdpcmd, FALSE);
	}
	/* read frame terminate */
	if (intstatus & I_RF_TERM) {
		SD_TRACE(("I_RF_TERM\n"));
		++sdpcmd->cnt.rftermint;
	}
	/* write frame terminate */
	if (intstatus & I_WF_TERM) {
		SD_ERROR(("I_WF_TERM\n"));
		++sdpcmd->cnt.wftermint;
	}

#if TEMP_DEBUG
	/* PCMCIA transmit FIFO underflow */
	if (intstatus & I_PCMCIA_XU)
		SD_ERROR(("I_PCMCIA_XU\n"));
#endif
#ifdef TXGLOM_CODE
	/* If ack glomming is supported */
	if  (sdpcmd->acktx_pending && (sdpcmd->tunables[TXGLOM] == 8)) {
		sdpcmd_txglom(sdpcmd);
		sdpcmd_sendnext(sdpcmd);
		sdpcmd->acktx_pending = FALSE;
		if (sdpcmd->txlazytimer_running) {
			dngl_del_timer(sdpcmd->txlazytimer);
			sdpcmd->txlazytimer_running = FALSE;
		}
	}
#endif
	/* DMA engine interrupts */
	if (intstatus & I_DMA) {
		SD_TRACE(("DMA\n"));
		resched = sdpcmd_dma(sdpcmd, intstatus);
	}

	/* switch to gpio usage (SD host going to sleep) */
	if ((intstatus & I_SMB_USE_OOB) && sdpcmd->gpbit) {
		sdpcmd->use_oob = TRUE;
		/* account for any current SW interrupt bits */
		if ((R_REG(sdpcmd->osh, &sdpcmd->regs->intstatus) & I_HMB_SW_MASK &
		     R_REG(sdpcmd->osh, &sdpcmd->regs->hostintmask)) != 0) {
			sdpcmd_wakehost(sdpcmd);
		} else {
			si_gpioout(sdpcmd->sih, sdpcmd->gpbit,
			           (sdpcmd->gpval ^ sdpcmd->gpbit), GPIO_HI_PRIORITY);
			sdpcmd->gpon = FALSE;
			if (sdpcmd->gptimer_active) {
				dngl_del_timer(sdpcmd->gptimer);
				sdpcmd->gptimer_active = FALSE;
			}
		}
	}

	/* For generic message, process mailbox data */
	if (intstatus & I_SMB_DEV_INT) {
		if (sdpcmd_devmail(sdpcmd))
			resched = TRUE;
	}

	/* control and event frames always cause immediate send... */
	if (pktq_mlen(&sdpcmd->txq, (1 << SDPCM_TX_PREC_CTRL)) > 0) {
		sdpcmd_sendnext(sdpcmd);
	}

	/* If there is a packet in the queue... */
	if ((qlen = pktq_mlen(&sdpcmd->txq, (1 << SDPCM_TX_PREC_DATA)))) {
		/* Handling of the queue depends on glomming mode */
		switch (sdpcmd->tunables[TXGLOM]) {
		case 0: /* glom disabled, dma space triggers send */
		default: /* unknown value, treat as disabled */
			sdpcmd_sendnext(sdpcmd);
			break;
#ifdef TXGLOM_CODE
		case 1: /* legacy glomming mode: start timer if needed */
			if (sdpcmd->txlazytimer_running == FALSE) {
				dngl_add_timer(sdpcmd->txlazytimer,
				               sdpcmd->tunables[TXLAZYDELAY], FALSE);
				sdpcmd->txlazytimer_running = TRUE;
			}
			break;
#if GLOMTEST
		case 2: /* basic dma-triggered glomming (kiranm) */
			sdpcmd_txglom(sdpcmd);
			sdpcmd_sendnext(sdpcmd);
			break;
		case 3: /* dma-triggered only when dma empty */
			if (dma_txactive(sdpcmd->di))
				break;
			sdpcmd_txglom(sdpcmd);
			sdpcmd_sendnext(sdpcmd);
			break;
		case 4: /* timer as hold-down: if not active, send and set */
			if (!sdpcmd->txlazytimer_running) {
				sdpcmd_txglom(sdpcmd);
				sdpcmd_sendnext(sdpcmd);
				dngl_add_timer(sdpcmd->txlazytimer,
				               sdpcmd->tunables[TXLAZYDELAY], FALSE);
				sdpcmd->txlazytimer_running = TRUE;
			}
			break;
		case 5: /* timer as hold-down, dma also hold-down */
			if (!sdpcmd->txlazytimer_running && !dma_txactive(sdpcmd->di)) {
				sdpcmd_txglom(sdpcmd);
				sdpcmd_sendnext(sdpcmd);
				dngl_add_timer(sdpcmd->txlazytimer,
				               sdpcmd->tunables[TXLAZYDELAY], FALSE);
				sdpcmd->txlazytimer_running = TRUE;
			}
			break;
		case 6:
#endif /* GLOMTEST */
		case 7: /* timer means wait (active stream), but send if we have
			 * a full (or queued) glom; and send if no timer (gap)
			 */
			if (!sdpcmd->txlazytimer_running ||
			    qlen >= sdpcmd->tunables[MAXTXPKTGLOM] ||
			    (SDPCM_PACKET_CHANNEL(PKTDATA(sdpcmd->osh,
			     pktqprec_peek(&sdpcmd->txq, SDPCM_TX_PREC_DATA))) ==
			     SDPCM_GLOM_CHANNEL)) {
				sdpcmd->acktx_pending = FALSE;
				sdpcmd_txglom(sdpcmd);
				sdpcmd_sendnext(sdpcmd);
			}
			break;
		case 8:
			if (!sdpcmd->txlazytimer_running ||
				sdpcmd->acktx_pending ||
			    qlen >= sdpcmd->tunables[MAXTXPKTGLOM] ||
			    (SDPCM_PACKET_CHANNEL(PKTDATA(sdpcmd->osh,
			     pktqprec_peek(&sdpcmd->txq, SDPCM_TX_PREC_DATA))) ==
			     SDPCM_GLOM_CHANNEL)) {
			    sdpcmd->acktx_pending = FALSE;
				if (sdpcmd->txlazytimer_running) {
					dngl_del_timer(sdpcmd->txlazytimer);
					sdpcmd->txlazytimer_running = FALSE;
				}
				sdpcmd_txglom(sdpcmd);
				sdpcmd_sendnext(sdpcmd);
			}
			break;
#endif /* TXGLOM_CODE */
		}
	}


cleanup:
	return resched;
}

static uint32
sdpcmd_intstatus(struct dngl_bus *sdpcmd)
{
	uint32 intstatus = R_REG(sdpcmd->osh, &sdpcmd->regs->intstatus);

	SD_TRACE(("sdpcmd_intstatus: 0x%x\n", intstatus));
	intstatus &= sdpcmd->defintmask;

	/* clear asserted device-side intstatus bits */
	W_REG(sdpcmd->osh, &sdpcmd->regs->intstatus, intstatus);
	return intstatus;
}

/* Dispatch interrupt events */
bool
sdpcmd_dispatch(struct dngl_bus *sdpcmd)
{
	uint32 intstatus;
#if TEMP_DEBUG
	uint32 sbintstatus = R_REG(sdpcmd->osh, &sdpcmd->regs->sbintstatus);
#endif

	SD_TRACE(("sdpcmd_dispatch\n"));

	intstatus = sdpcmd_intstatus(sdpcmd);

#if TEMP_DEBUG
	if (!intstatus && !(sbintstatus & sdpcmd->defsbintmask))
		return FALSE;
#else
	if (!intstatus)
		return FALSE;
#endif

	ASSERT(sdpcmd->up);

	ASSERT(sdpcmd->intstatus == 0);
	sdpcmd->intstatus = intstatus;
#if TEMP_DEBUG
	sdpcmd->sbintstatus = sbintstatus;
#endif

	return TRUE;
}

void
sdpcmd_intrsoff(struct dngl_bus *sdpcmd)
{
	SD_TRACE(("sdpcmd_intrsoff\n"));
	/* clear all device-side intstatus bits */
	W_REG(sdpcmd->osh, &sdpcmd->regs->intmask, 0);
	(void)R_REG(sdpcmd->osh, &sdpcmd->regs->intmask); /* sync readback */
	sdpcmd->intmask = 0;
#if TEMP_DEBUG
	W_REG(sdpcmd->osh, &sdpcmd->regs->sbintmask, 0);
#endif
}

/* deassert interrupt */
void
sdpcmd_intrs_deassert(struct dngl_bus *sdpcmd)
{
	W_REG(sdpcmd->osh, &sdpcmd->regs->intmask, 0);
	(void)R_REG(sdpcmd->osh, &sdpcmd->regs->intmask); /* sync readback */
}

void
sdpcmd_intrson(struct dngl_bus *sdpcmd)
{
	SD_TRACE(("sdpcmd_intrson\n"));
	sdpcmd->intmask = sdpcmd->defintmask;
	W_REG(sdpcmd->osh, &sdpcmd->regs->intmask, sdpcmd->defintmask);
#if TEMP_DEBUG
	W_REG(sdpcmd->osh, &sdpcmd->regs->sbintmask, sdpcmd->defsbintmask);
#endif
}

void
sdpcmd_intrsupd(struct dngl_bus *sdpcmd)
{
	uint32 intstatus;

	SD_TRACE(("sdpcmd_intrsupd\n"));

	ASSERT(sdpcmd->intstatus != 0);

	/* read and clear intstatus */
	intstatus = sdpcmd_intstatus(sdpcmd);

	/* update interrupt status in software */
	sdpcmd->intstatus |= intstatus;
}

static bool sdpcmd_validate(struct dngl_bus *sdpcmd, void *p);


/* Handle DMA events */
static bool
sdpcmd_dma(struct dngl_bus *sdpcmd, uint32 dmaintstatus)
{
	void *p;
	uint rxcount = 0;

	void * head = NULL, *tail = NULL;

	SD_TRACE(("sdpcmd_dma\n"));

	if (dmaintstatus & I_ERRORS) {
		bool rxreset = FALSE;
		bool txreset = FALSE;

		if (dmaintstatus & I_PC) {
			if (dma_rxstopped(sdpcmd->di)) {
				SD_ERROR(("%s: rx dma descriptor error\n", __FUNCTION__));
				rxreset = TRUE;
			}
			if (dma_txstopped(sdpcmd->di)) {
				SD_ERROR(("%s: tx dma descriptor error\n", __FUNCTION__));
				txreset = TRUE;
			}
		}
		if (dmaintstatus & I_PD) {
			if (dma_rxstopped(sdpcmd->di)) {
				SD_ERROR(("%s: rx dma data error\n", __FUNCTION__));
				rxreset = TRUE;
			}
			if (dma_txstopped(sdpcmd->di)) {
				SD_ERROR(("%s: tx dma data error\n", __FUNCTION__));
				txreset = TRUE;
			}
		}
		if (dmaintstatus & I_DE) {
			if (dma_rxstopped(sdpcmd->di)) {
				SD_ERROR(("%s: rx dma descriptor protocol error\n", __FUNCTION__));
				rxreset = TRUE;
			}
			if (dma_txstopped(sdpcmd->di)) {
				SD_ERROR(("%s: tx dma descriptor protocol error\n", __FUNCTION__));
				txreset = TRUE;
			}
		}
		if (dmaintstatus & I_RU) {
			SD_TRACE(("I_RU\n"));
			++sdpcmd->cnt.rxdescuflo;
		}
		if (dmaintstatus & I_RO) {
			SD_TRACE(("I_RO\n"));
			++sdpcmd->cnt.rxfifooflo;
			if (sdpcmd->cnt.rxfifooflo != sdpcmd->cnt.rxdescuflo) {
				SD_ERROR(("*** rx fifo overflow (%d) w/o rx desc underflow (%d)\n",
				          sdpcmd->cnt.rxfifooflo, sdpcmd->cnt.rxdescuflo));
#if TEMP_DEBUG
				{
					struct bcmstrbuf b;
					char buf[4096];
					bcm_binit(&b, buf, 4096);
					OSL_DELAY(1000);
					sdpcmd_dumpregs(sdpcmd, &b);
					printfbig(b.origbuf);
				}
#endif
			}
			rxreset = TRUE;
		}

		if (dmaintstatus & I_XU) {
			SD_ERROR(("I_XU\n"));
			++sdpcmd->cnt.txfifouflo;
			if (sdpcmd->held_tx) {
				ASSERT(sdpcmd->pend_tx != sdpcmd->held_tx);
				PKTFREE(sdpcmd->osh, sdpcmd->held_tx, TRUE);
				sdpcmd->held_tx = NULL;
			}
#if TEMP_DEBUG
			{
				struct bcmstrbuf b;
				char buf[4096];
				bcm_binit(&b, buf, 4096);
				OSL_DELAY(1000);
				sdpcmd_dumpregs(sdpcmd, &b);
				printfbig(b.origbuf);
			}
#endif
			txreset = TRUE;
		}

		if (rxreset || txreset) {
			if (txreset)
				sdpcmd_txrecompose(sdpcmd, FALSE);
			if (rxreset)
				sdpcmd_rxrecompose(sdpcmd);
		}
	}

	if (dmaintstatus & I_XI)
		sdpcmd_txreclaim(sdpcmd);

	if ((dmaintstatus & I_RI) || (dmaintstatus & I_RU)) {
		SD_TRACE(("sdpcmd_dma(): got a packet\n"));
		while ((p = dma_rx(sdpcmd->di))) {
#ifdef BCMDBG_POOL
			PKTPOOLSETSTATE(p, POOL_RXDH);
#endif
			if (sdpcmd_validate(sdpcmd, p) == TRUE) {
				if (!tail)
					head = tail = p;
				else {
					PKTSETLINK(tail, p);
					tail = p;
				}
			}
			/* reschedule sdpcmdev_dpc() to process more later */
			/* RXBND = -1 is essentially unbounded processing */
			if (++rxcount >= (uint)sdpcmd->tunables[RXBND]) {
				sdpcmd->intstatus |= I_RI;
				break;
			}
		}

		if (POOL_ENAB(sdpcmd->pktpool))
			pktpool_emptycb_disable(sdpcmd->pktpool, FALSE);
		sdpcmd_rxfill(sdpcmd);

		/* Notify host of new space if needed */
		if (((sdpcmd->rx_lim != sdpcmd->rx_hlim) &&
		    ((uint8)(sdpcmd->rx_hlim - sdpcmd->rx_seq) <= sdpcmd->credtrig)) ||
		    sdpcmd->want_resync) {
			sdpcmd_sendheader(sdpcmd);
		}

#ifdef PKTC_TX_DONGLE
		if (head != NULL) {
			sdpcmd_sendup(sdpcmd, head);
		}
#else
		while ((p = head) != NULL) {
			head = PKTLINK(head);
			PKTSETLINK(p, NULL);
			sdpcmd_sendup(sdpcmd, p);
		}
#endif /* PKTC_TX_DONGLE */
	}

	return (rxcount >= (uint)sdpcmd->tunables[RXBND]);
}

static bool
BCMATTACHFN(sdpcmd_match)(uint vendor, uint device)
{
	SD_TRACE(("sdpcmd_match\n"));

	if (vendor != VENDOR_BROADCOM)
		return FALSE;

	switch (device) {
	case SDIOD_FPGA_ID:
		return TRUE;
	}

	return FALSE;
}

/* Send a window-only frame (zero-data packet) */
static void
sdpcmd_sendheader(struct dngl_bus *sdpcmd)
{
	void *p = NULL;

	/* Clear the resync flag */
	sdpcmd->want_resync = FALSE;

	/* If glom timer is running, re-initiate it with 0ms timeout so that tx wont get blocked */
	if (sdpcmd->txlazytimer_running) {
		dngl_del_timer(sdpcmd->txlazytimer);
		dngl_add_timer(sdpcmd->txlazytimer, 0, FALSE);
	}

	/* Don't bother if there's already stuff waiting to be posted */
	if (pktq_n_pkts_tot(&sdpcmd->txq)) {
		return;
	}

	/* Allocate a packet with no data */
	if (PHDR_ENAB(sdpcmd))
		p = pktpool_get(sdpcmd->pktpool_phdr);

	if (p == NULL)
		p = PKTGET(sdpcmd->osh,
		           sdpcmd->tunables[DONGLEHDRSZ] + sdpcmd->tunables[DONGLEPADSZ],
		           FALSE);

	if (p == NULL) {
		SD_ERROR(("%s: PKTGET %d failed!!\n",
			__FUNCTION__, sdpcmd->tunables[DONGLEHDRSZ] +
			sdpcmd->tunables[DONGLEPADSZ]));
		sdpcmd->tunables[CREDFAIL]++;
		return;
	}
	PKTSETLEN(sdpcmd->osh, p, sdpcmd->tunables[DONGLEHDRSZ]);
	sdpcmd_rxfill(sdpcmd);

	/* Send the frame */
	PKTPULL(sdpcmd->osh, p, sdpcmd->tunables[DONGLEHDRSZ]);
	if (!sdpcmd_tx(sdpcmd, p, SDPCM_EVENT_CHANNEL)) {
		sdpcmd->sendhdr_fail++;
		sdpcmd->tunables[CREDFAIL]++;
		SD_ERROR(("%s: tx submit failed!!\n", __FUNCTION__));
	}
	else
		sdpcmd->explicit_fc++;
}

#if TEMP_DEBUG
uint pendfill_cnt = 0;
uint pendvalid_cnt = 0;
#endif

/* Post READ data */
static void
sdpcmd_sendnext(struct dngl_bus *sdpcmd)
{
	void *p;
	int prec;
	uint8 *pdata;
	bool sent = FALSE;
	bool commit;

	SD_TRACE(("sdpcmd_sendnext\n"));

	prec = (pktq_mlen(&sdpcmd->txq, (1 << SDPCM_TX_PREC_CTRL)) > 0) ?
		SDPCM_TX_PREC_CTRL : SDPCM_TX_PREC_DATA;

	/* this rxfill is necessary to ensure availability of rx buffers */
	sdpcmd_rxfill(sdpcmd);

	while (*sdpcmd->txavail > 1 && (p = pktq_pdeq(&sdpcmd->txq, prec))) {
		SD_PRPKT("tx", PKTDATA(sdpcmd->osh, p), PKTLEN(sdpcmd->osh, p));

		/* Double-check that packet will fit -- if not, wait */
		if (pktsegcnt(sdpcmd->osh, p) > *sdpcmd->txavail) {
			pktq_penq_head(&sdpcmd->txq, prec, p);
			break;
		}

		/* Clear any leftover Next Read Len (e.g. NAK recompose) */
		pdata = PKTDATA(sdpcmd->osh, p);
		pdata[SDPCM_FRAMETAG_LEN + SDPCM_NEXTLEN_OFFSET] = 0;
		pdata[SDPCM_FRAMETAG_LEN + SDPCM_FCMASK_OFFSET] = sdpcmd->txflowcontrol;

		/* Add the window (rx sequence limit) */
		pdata[SDPCM_FRAMETAG_LEN + SDPCM_WINDOW_OFFSET] =
		        sdpcmd->rx_hlim = sdpcmd->rx_lim;
		sdpcmd->alltx_fc++;

		if (SDPCM_GLOMDESC(&pdata[SDPCM_FRAMETAG_LEN]))
			commit = FALSE;
		else
			commit = TRUE;

#ifdef BCMDBG_POOL
		if (PKTPOOL(sdpcmd->osh, p))
			PKTPOOLSETSTATE(p, POOL_TXDH);
#endif
		if (dma_txfast(sdpcmd->di, p, commit) == 0) {

		if (sdpcmd_readahead && sdpcmd->pend_tx) {
			extern uint dma32_txpending(hnddma_t *di);
			uint16 len;

			if (prec == SDPCM_TX_PREC_CTRL) {
				len = 0;
			} else {
				/* Get frame len from header */
				len = ltoh16_ua(pdata);

				/* Convert to Next Read Len, store in pend_tx */
				len = (ROUNDUP(len, 16) >> 4);
				if (len >> 8)
					len = 0;
			}
			pdata = PKTDATA(sdpcmd->osh, sdpcmd->pend_tx);
			pdata += SDPCM_FRAMETAG_LEN;
			pdata[SDPCM_NEXTLEN_OFFSET] = (uint8)len;
			pdata[SDPCM_FCMASK_OFFSET] = sdpcmd->txflowcontrol;

#if TEMP_DEBUG
				pendfill_cnt++;
				if ((len = dma32_txpending(sdpcmd->di)) >= 3) {
					pendvalid_cnt++;
				}
#endif /* TEMP_DEBUG */
		}

			/* This packet becomes pend_tx */
			if (sdpcmd_readahead)
				sdpcmd->pend_tx = p;

			sent = TRUE;

			/* Flow-control is now in sync */
			sdpcmd->txflowlast = sdpcmd->txflowcontrol;
			sdpcmd->txflowpending = FALSE;
		} else {
			SD_ERROR(("%s: tx dma failed\n", __FUNCTION__));
		}
	}

	sdpcmd->in_sendnext = FALSE;

	sdpcmd->pend_tx = NULL;

	/* kick host to issue Read command */
	if (sent) {
#ifdef DS_PROT
		if (DS_PROT_ENAB(sdpcmd)) {
			sdpcmd_ds_state_tbl_entry_t ds_entry;
			ds_entry = *sdpcmd_get_ds_state_tbl_entry(sdpcmd->ds_state, INT_DTOH_EVENT);
			if ((ds_entry.action_fn != NULL) ||
			    ((ds_entry.transition != DS_INVALID_STATE) &&
			     (ds_entry.transition != DS_DISABLED_STATE))) {
				sdpcmd_ds_engine(sdpcmd, INT_DTOH_EVENT);
			}
		}
#endif /* DS_PROT */
		W_REG(sdpcmd->osh, &sdpcmd->regs->tohostmailbox, HMB_FRAME_IND);
		sdpcmd_wakehost(sdpcmd);
	}

	if (pktq_mlen(&sdpcmd->txq, (1 << SDPCM_TX_PREC_DATA)) > sdpcmd->tunables[DATAHIWAT])
		dngl_txstop(sdpcmd->dngl);
	else if (pktq_mlen(&sdpcmd->txq, (1 << SDPCM_TX_PREC_DATA)) < sdpcmd->tunables[DATALOWAT])
		dngl_txstart(sdpcmd->dngl);

}

static int
sdpcmd_droppable(struct dngl_bus *sdpcmd, void *p, int channel)
{
#ifdef BCMDBG
	return (channel == SDPCM_DATA_CHANNEL || channel == SDPCM_TEST_CHANNEL);
#else
	return (channel == SDPCM_DATA_CHANNEL) && !PKTNODROP(NULL, p);
#endif /* BCMDBG */
}

/* Post READ data */
static int
sdpcmd_txsubmit(struct dngl_bus *sdpcmd, void *p, int channel)
{
	bool dropok;
	uint qlen;

	SD_TRACE(("sdpcmd_txsubmit\n"));

	dropok = sdpcmd_droppable(sdpcmd, p, channel);

	/* toss droppable packets if we've hit the queue limit */
	if ((pktq_n_pkts_tot(&sdpcmd->txq) >= sdpcmd->tunables[TXSWQLEN]) && dropok) {
		SD_ERROR(("%s: txq full drop, %d\n", __FUNCTION__, sdpcmd->tunables[TXSWQLEN]));
		PKTFREE(sdpcmd->osh, p, TRUE);
		++sdpcmd->tunables[TXDROP];
		return FALSE;
	}
#ifdef BCMDBG_SD_LATENCY
	if (channel == SDPCM_DATA_CHANNEL)
		sdpcmd_timestamp_arrival(sdpcmd, p);
#endif /* BCMDBG_SD_LATENCY */
	/* otherwise, queue the packet to retain ordering */
	if (channel == SDPCM_CONTROL_CHANNEL)
		pktq_penq(&sdpcmd->txq, SDPCM_TX_PREC_CTRL, p);
	else
		pktq_penq(&sdpcmd->txq, SDPCM_TX_PREC_DATA, p);

	/* control and event frames always cause immediate send... */
	if (pktq_mlen(&sdpcmd->txq, (1 << SDPCM_TX_PREC_CTRL)) > 0) {
		sdpcmd_sendnext(sdpcmd);
	}

	if ((qlen = pktq_mlen(&sdpcmd->txq, (1 << SDPCM_TX_PREC_DATA))) <= 0)
		return TRUE;

	/* ...but data frames may be held for glomming */
	switch (sdpcmd->tunables[TXGLOM]) {
	case 0: /* glom disabled, send immediately */
	default: /* unknown value, treat as disabled */
		sdpcmd_sendnext(sdpcmd);
		break;
#ifdef TXGLOM_CODE
	case 1: /* legacy glomming mode: glom/send now if full, else use timer */
		if (qlen >= sdpcmd->tunables[MAXTXPKTGLOM]) {
			sdpcmd_txglom(sdpcmd);
			sdpcmd_sendnext(sdpcmd);
		} else if (sdpcmd->txlazytimer_running == FALSE) {
			dngl_add_timer(sdpcmd->txlazytimer, sdpcmd->tunables[TXLAZYDELAY], FALSE);
			sdpcmd->txlazytimer_running = TRUE;
		}
		break;
#if GLOMTEST
	case 2: /* basic dma-triggered glomming (kiranm) */
		if (!dma_txactive(sdpcmd->di))
			sdpcmd_sendnext(sdpcmd);
		break;
	case 3: /* dma-triggered only when dma empty */
		if (!dma_txactive(sdpcmd->di)) {
			sdpcmd_txglom(sdpcmd);
			sdpcmd_sendnext(sdpcmd);
		}
		break;
	case 4: /* timer as hold-down: if not active, send and set; send on full too */
		if (!sdpcmd->txlazytimer_running) {
			sdpcmd_txglom(sdpcmd);
			sdpcmd_sendnext(sdpcmd);
			dngl_add_timer(sdpcmd->txlazytimer, sdpcmd->tunables[TXLAZYDELAY], FALSE);
			sdpcmd->txlazytimer_running = TRUE;
		} else if (qlen >= sdpcmd->tunables[MAXTXPKTGLOM]) {
			sdpcmd_txglom(sdpcmd);
			sdpcmd_sendnext(sdpcmd);
		}
		break;
	case 5: /* timer as hold-down, dma also hold-down */
		if (!sdpcmd->txlazytimer_running && !dma_txactive(sdpcmd->di)) {
			sdpcmd_txglom(sdpcmd);
			sdpcmd_sendnext(sdpcmd);
			dngl_add_timer(sdpcmd->txlazytimer, sdpcmd->tunables[TXLAZYDELAY], FALSE);
			sdpcmd->txlazytimer_running = TRUE;
		} else if (qlen >= sdpcmd->tunables[MAXTXPKTGLOM]) {
			sdpcmd_txglom(sdpcmd);
			sdpcmd_sendnext(sdpcmd);
		}
		break;
	case 6: /* send now if first packet or full glom, else set timer for gap */
		if ((!sdpcmd->txlazytimer_running && !dma_txactive(sdpcmd->di)) ||
		    (qlen >= sdpcmd->tunables[MAXTXPKTGLOM])) {
			sdpcmd_txglom(sdpcmd);
			sdpcmd_sendnext(sdpcmd);
		}

		if (sdpcmd->txlazytimer_running) {
			dngl_del_timer(sdpcmd->txlazytimer);
		}

		dngl_add_timer(sdpcmd->txlazytimer, sdpcmd->tunables[TXLAZYDELAY], FALSE);
		sdpcmd->txlazytimer_running = TRUE;
		break;
#endif /* GLOMTEST */
	case 7: /* send now if dma inactive or full glom, else set timer for gap */
		if ((!sdpcmd->txlazytimer_running && !dma_txactive(sdpcmd->di)) ||
			(qlen >= sdpcmd->tunables[MAXTXPKTGLOM])||
			(sdpcmd->tunables[ACKFASTFWD] &&
			PKTLEN(sdpcmd->osh, p) <= sdpcmd->tunables[ACKSIZETHSD])) {
			if (qlen > 1) {
				sdpcmd_txglom(sdpcmd);
			}
			sdpcmd_sendnext(sdpcmd);
			if (sdpcmd->txlazytimer_running) {
				dngl_del_timer(sdpcmd->txlazytimer);
				sdpcmd->txlazytimer_running = FALSE;
			}
			sdpcmd->acktx_pending = FALSE;
		} else if (!sdpcmd->txlazytimer_running) {
			dngl_add_timer(sdpcmd->txlazytimer, sdpcmd->tunables[TXLAZYDELAY], FALSE);
			sdpcmd->txlazytimer_running = TRUE;
		}
		break;
	case 8:
		if ((!sdpcmd->txlazytimer_running && !dma_txactive(sdpcmd->di)) ||
			(qlen >= sdpcmd->tunables[MAXTXPKTGLOM]) ||
			(PKTLEN(sdpcmd->osh, p) <= 64)) {
			sdpcmd_txglom(sdpcmd);
			sdpcmd_sendnext(sdpcmd);
			if (sdpcmd->txlazytimer_running) {
				dngl_del_timer(sdpcmd->txlazytimer);
				sdpcmd->txlazytimer_running = FALSE;
			}
			sdpcmd->acktx_pending = FALSE;
		} else if (sdpcmd->tunables[ACKFASTFWD] &&
			PKTLEN(sdpcmd->osh, p) <= sdpcmd->tunables[ACKSIZETHSD]) {
			if (!dma_txactive(sdpcmd->di)) {
				sdpcmd_txglom(sdpcmd);
				sdpcmd_sendnext(sdpcmd);
				if (sdpcmd->txlazytimer_running) {
					dngl_del_timer(sdpcmd->txlazytimer);
					sdpcmd->txlazytimer_running = FALSE;
				}
				sdpcmd->acktx_pending = FALSE;
			} else if (!sdpcmd->acktx_pending) {
				if (sdpcmd->txlazytimer_running)
					dngl_del_timer(sdpcmd->txlazytimer);
				dngl_add_timer(sdpcmd->txlazytimer, 0, FALSE);
				sdpcmd->txlazytimer_running = TRUE;
				sdpcmd->acktx_pending = TRUE;
			}
		} else if (!sdpcmd->txlazytimer_running) {
			dngl_add_timer(sdpcmd->txlazytimer, sdpcmd->tunables[TXLAZYDELAY], FALSE);
			sdpcmd->txlazytimer_running = TRUE;
		}
		break;
#endif /* TXGLOM_CODE */
	}

	return TRUE;
}

/* Enqueue READ data */
static int
sdpcmd_tx(struct dngl_bus *sdpcmd, void *p, int channel)
{
	uint16 len;
	uint8 *pktdata;
	uint32 swheader[2];
	osl_t *osh = sdpcmd->osh;
	uint pad = 0;

	SD_TRACE(("sdpcmd_tx\n"));

	ASSERT(sdpcmd->di);

	swheader[0] = swheader[1] = 0;

	if (sdpcmd->disabled) {
		SD_ERROR(("%s: device disabled\n", __FUNCTION__));
		PKTFREE(osh, p, TRUE);
		return FALSE;
	}

	pad = (uintptr)PKTDATA(osh, p) & sdpcmd->tunables[ALIGNMOD];

	/* make room for software and hw headers */
	if (PKTHEADROOM(osh, p) < (SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN + pad)) {
		void *p1;
		uint16 pktlen = PKTLEN(osh, p);

		/* Catch the case where this can be fixed at the source.
		 * e.g. If the packet is created in wl (e.g. wlc_uncram()), it
		 * can be created with enough headroom to avoid this pktcopy.
		 */
		ASSERT(0);

		if ((p1 = PKTGET(osh, (pktlen + sdpcmd->tunables[DONGLEHDRSZ] + pad),
		                 FALSE)) == NULL) {
			SD_ERROR(("PKTGET failed\n"));
			PKTFREE(osh, p, TRUE);
			return FALSE;
		}
		PKTPULL(osh, p1, (sdpcmd->tunables[DONGLEHDRSZ] + pad));
		memcpy(PKTDATA(osh, p1), PKTDATA(osh, p), pktlen);
		PKTSETNEXT(osh, p1, PKTNEXT(osh, p));
		PKTFREE(osh, p, FALSE);
		p = p1;
	}

	if (pad) {
		PKTPUSH(osh, p, pad);
		bzero(PKTDATA(osh, p), pad);
	}

	PKTPUSH(osh, p, sdpcmd->tunables[DONGLEHDRSZ]);
	pktdata = (uint8 *) PKTDATA(osh, p);

	/* insert hw header: 2 byte len followed by 2 byte ~len check value */
	len = htol16((uint16) pkttotlen(osh, p));

	memcpy(pktdata, &len, sizeof(uint16));

	len = ~len;
	memcpy(pktdata + sizeof(uint16), &len, sizeof(uint16));

	/* Determine data offset (header length plus any pad) */
	pad += (SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN);
	ASSERT(pad < 0xff);

	/* insert software header */
	swheader[0] = sdpcmd->tx_seq |
	        ((channel << SDPCM_CHANNEL_SHIFT) & SDPCM_CHANNEL_MASK) |
	        ((pad << SDPCM_DOFFSET_SHIFT) & SDPCM_DOFFSET_MASK);
	swheader[0] = htol32(swheader[0]);
	memcpy(pktdata + SDPCM_FRAMETAG_LEN, &swheader[0], SDPCM_SWHEADER_LEN);

	if (!sdpcmd_txsubmit(sdpcmd, p, channel))
		return FALSE;

	++sdpcmd->tx_seq;
	return TRUE;
}

/* Send a bus packet up to the device handler for processing */

static bool
sdpcmd_validate(struct dngl_bus *sdpcmd, void *p)
{
	sdpcmd_rxh_t *rxh = NULL;
	uint16 flags;
	uint8 sequence;
	uint8 doff;

	uint16 framelen;
	uint16 framecheck;

	SD_TRACE(("sdpcmd_validate\n"));

	/* Strip off rx header */
	rxh = (sdpcmd_rxh_t *) PKTDATA(sdpcmd->osh, p);

	SD_TRACE(("rx pkt header: len %d; flags 0x%x; PKTLEN: %d\n", rxh->len,
	       rxh->flags, PKTLEN(sdpcmd->osh, p)));
	PKTPULL(sdpcmd->osh, p, SDPCMD_RXOFFSET);
	ASSERT(ISALIGNED((uintptr)rxh, 4));

	SD_PRPKT("rx", PKTDATA(sdpcmd->osh, p), PKTLEN(sdpcmd->osh, p));

	flags = ltoh16(rxh->flags);
	if (flags & RXF_DISCARD) {
		SD_TRACE(("%s: bad frame; flags 0x%x\n", __FUNCTION__, flags));
		if (flags & RXF_CRC) {
			SD_TRACE(("%s: frame crc error\n", __FUNCTION__));
			++sdpcmd->cnt.rxfcrc;
		}
		if (flags & RXF_WOOS) {
			SD_TRACE(("%s: write out of sync\n", __FUNCTION__));
			++sdpcmd->cnt.rxfwoos;
		}
		if (flags & RXF_WF_TERM) {
			SD_TRACE(("%s: write frame termination\n", __FUNCTION__));
			++sdpcmd->cnt.rxfwft;
		}
		if (flags & RXF_ABORT) {
			SD_TRACE(("%s: frame aborted\n", __FUNCTION__));
			++sdpcmd->cnt.rxfabort;
		}
		if (POOL_ENAB(sdpcmd->pktpool))
			pktpool_emptycb_disable(sdpcmd->pktpool, TRUE);
		goto free_and_resync;
	}

	/* Discard if too short (no HW tag) */
	if (PKTLEN(sdpcmd->osh, p) < SDPCM_FRAMETAG_LEN) {
		SD_TRACE(("%s: discard %d-byte runt frame\n", __FUNCTION__,
			PKTLEN(sdpcmd->osh, p)));
		++sdpcmd->cnt.runt;
		if (POOL_ENAB(sdpcmd->pktpool))
			pktpool_emptycb_disable(sdpcmd->pktpool, TRUE);
		goto free_and_resync;
	}

	/* Discard any bogus frames that may have got through */
	bcopy(PKTDATA(sdpcmd->osh, p), &framelen, 2);
	bcopy((uint8 *)PKTDATA(sdpcmd->osh, p) + 2, &framecheck, 2);

	if (((rxh->len != framelen) && !sdpcmd->rxglom) ||
		((framecheck & 0xFFFF) != (~framelen & 0xFFFF))) {
		SD_TRACE(("%s: discard %d-byte bogus frame\n", __FUNCTION__,
			PKTLEN(sdpcmd->osh, p)));
		if (rxh->len != framelen) {
			SD_ERROR(("%s: rxh len (%d) does not match framelen (%d)\n",
			          __FUNCTION__, ltoh16(rxh->len), ltoh16(framelen)));
			++sdpcmd->cnt.badlen;
		}
		if ((framecheck & 0xFFFF) != (~framelen & 0xFFFF)) {
			SD_ERROR(("%s: invalid check 0x%04x for len 0x%04x\n",
			          __FUNCTION__, ltoh16(framecheck), ltoh16(framelen)));
			++sdpcmd->cnt.badcksum;
		}
		SD_PRPKT("rx", PKTDATA(sdpcmd->osh, p), PKTLEN(sdpcmd->osh, p));
		if (POOL_ENAB(sdpcmd->pktpool))
			pktpool_emptycb_disable(sdpcmd->pktpool, TRUE);
		goto free_and_resync;
	}

#ifdef BCMDBG_MEM
	ASSERT(lb_sane(p));
#endif

	/* remove HW header */
	PKTPULL(sdpcmd->osh, p, SDPCM_FRAMETAG_LEN);

	if (sdpcmd->rxglom) {
		if (PKTLEN(sdpcmd->osh, p) < SDPCM_HWEXT_LEN) {
			SD_TRACE(("sdpcmd_validate: discard %d-byte runt frame\n",
				PKTLEN(sdpcmd->osh, p)));
			++sdpcmd->cnt.runt;
			if (POOL_ENAB(sdpcmd->pktpool))
				pktpool_emptycb_disable(sdpcmd->pktpool, TRUE);
			goto free_and_resync;
		}
		/* remove HW Extension header */
		PKTPULL(sdpcmd->osh, p, SDPCM_HWEXT_LEN);
	}

	/* Discard if no SW tag (could interpret 0-len as signalling) */
	if (PKTLEN(sdpcmd->osh, p) < SDPCM_SWHEADER_LEN) {
		SD_ERROR(("%s: discard, only %d bytes of %d-byte SW tag\n",
		          __FUNCTION__, PKTLEN(sdpcmd->osh, p), SDPCM_SWHEADER_LEN));
		if (POOL_ENAB(sdpcmd->pktpool))
			pktpool_emptycb_disable(sdpcmd->pktpool, TRUE);
		PKTFREE(sdpcmd->osh, p, FALSE);
		return FALSE;
	}

	sequence = SDPCM_PACKET_SEQUENCE(PKTDATA(sdpcmd->osh, p));
	doff = SDPCM_DOFFSET_VALUE(PKTDATA(sdpcmd->osh, p));

	/* remove SW header */
	PKTPULL(sdpcmd->osh, p, SDPCM_SWHEADER_LEN);

	/* check and remove any padding */
	ASSERT(doff >= (SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN));
	doff -= (SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN);

	if (sdpcmd->rxglom)
		doff -= SDPCM_HWEXT_LEN;

	if (PKTLEN(sdpcmd->osh, p) < doff) {
		SD_ERROR(("%s: discard, only %d bytes of %d-byte pad\n",
		          __FUNCTION__, PKTLEN(sdpcmd->osh, p), doff));
		if (POOL_ENAB(sdpcmd->pktpool))
			pktpool_emptycb_disable(sdpcmd->pktpool, TRUE);
		PKTFREE(sdpcmd->osh, p, FALSE);
		return FALSE;
	}

	/* check sequence number */
	if (sequence != sdpcmd->rx_seq) {
		SD_TRACE(("%s: sequence break from %d (frame seq %d)\n",
		       __FUNCTION__, sdpcmd->rx_seq, sequence));
		++sdpcmd->cnt.seqbreak;
		sdpcmd->rx_seq = sequence;
		sdpcmd->want_resync = TRUE;
	}
	sdpcmd->rx_seq = (sdpcmd->rx_seq + 1) % SDPCM_SEQUENCE_WRAP;
	if ((uint8)(sdpcmd->rx_hlim - sdpcmd->rx_seq) > sdpcmd->tunables[RXBUFS]) {
		sdpcmd->want_resync = TRUE;
	}
	PKTPUSH(sdpcmd->osh, p, SDPCM_SWHEADER_LEN);
	return TRUE;

free_and_resync:
	PKTFREE(sdpcmd->osh, p, FALSE);
	sdpcmd->want_resync = TRUE;
	return FALSE;
}

static void
sdpcmd_sendup(struct dngl_bus *sdpcmd, void *p)
{
	uint8 p_channel;
	uint8 p_doff;
	void *n;
#ifdef PKTC_TX_DONGLE
	void *head = NULL, *tail = NULL;
	uint8 n_channel = 0;
#endif

	SD_TRACE(("sdpcmd_sendup\n"));

	while (p != NULL) {
		n = PKTLINK(p);
		PKTSETLINK(p, NULL);

		p_channel = SDPCM_PACKET_CHANNEL(PKTDATA(sdpcmd->osh, p));

		p_doff = SDPCM_DOFFSET_VALUE(PKTDATA(sdpcmd->osh, p)) - SDPCM_FRAMETAG_LEN;
		if (sdpcmd->rxglom)
			p_doff -= SDPCM_HWEXT_LEN;


		PKTPULL(sdpcmd->osh, p, p_doff);

		if (p_channel == SDPCM_DATA_CHANNEL) {
#ifdef PKTC_TX_DONGLE
			if (!tail) {
				head = tail = p;
			}
			else {
				PKTSETLINK(tail, p);
				tail = p;
			}

			/* conclude the linking on
			 * 1) n == NULL -> reached the end of pkt link
			 * 2) the next pkt is not data
			 * send it up and restart new linking.
			 */
			if (n == NULL ||
			    (n_channel = SDPCM_PACKET_CHANNEL(PKTDATA(sdpcmd->osh, n)),
			     n_channel != SDPCM_DATA_CHANNEL)) {
				dngl_sendup(sdpcmd->dngl, head);
				head = tail = NULL;
			}
#else
			dngl_sendup(sdpcmd->dngl, p);
#endif /* PKTC_TX_DONGLE */
		} else {

			if (p_channel == SDPCM_CONTROL_CHANNEL) {
				dngl_ctrldispatch(sdpcmd->dngl, p, NULL);
			} else if (p_channel == SDPCM_EVENT_CHANNEL) {
				/* Any packet on test channel triggers check
				   for virtual UART input
				*/
				hnd_cons_check();
				PKTFREE(sdpcmd->osh, p, FALSE);
			} else if (p_channel == SDPCM_TEST_CHANNEL) {
				PKTFREE(sdpcmd->osh, p, FALSE);
			} else {
				SD_ERROR(("%s: unknown channel Id: %d\n", __FUNCTION__, p_channel));
				PKTFREE(sdpcmd->osh, p, FALSE);
			}
		}

		p = n;
	}
}

/* clear tx DMA state and repost buffers, including NAK'd frame in retrans case */
static void
sdpcmd_txrecompose(struct dngl_bus *sdpcmd, bool retrans)
{
	void *p;
	int prec;
	struct pktq txq;        /* temp queue for descriptor ring items */
	struct pktq txq_tmp;	/* temp queue for SW tx queue backup */

	SD_TRACE(("sdpcmd_txrecompose\n"));

	pktq_init(&txq_tmp, SDPCM_TX_PREC_MAX, sdpcmd->tunables[TXSWQLEN] + 1);
	pktq_init(&txq, 1, sdpcmd->tunables[NTXD]);

	/* Reclaim already transmitted packets */
	sdpcmd_txreclaim(sdpcmd);
	sdpcmd->pend_tx = NULL;

	/* Disable tx engine */
	dma_txreset(sdpcmd->di);

	if (retrans && sdpcmd->held_tx) {
		pktenq(&txq, sdpcmd->held_tx);
		sdpcmd->held_tx = NULL;
	}

	/* Save packets to retransmit */
	while ((p = dma_getnexttxp(sdpcmd->di, TRUE))) {
		if (pktq_full(&txq)) {
			uint8 *swheader = (uint8*)PKTDATA(sdpcmd->osh, p) +
				SDPCM_FRAMETAG_LEN;
			if (SDPCM_PACKET_CHANNEL(swheader) == SDPCM_CONTROL_CHANNEL)
				pktq_penq(&txq_tmp, SDPCM_TX_PREC_CTRL, p);
			else
				pktq_penq(&txq_tmp, SDPCM_TX_PREC_DATA, p);
		}
		else
		    pktenq(&txq, p);
	}

	/* Back up SW queue packets */
	while ((p = pktq_deq(&sdpcmd->txq, &prec))) {
		if (!pktqprec_full(&txq_tmp, prec))
			pktq_penq(&txq_tmp, prec, p);
		else {
			SD_ERROR(("%s: txq_tmp full\n", __FUNCTION__));
			PKTFREE(sdpcmd->osh, p, TRUE);
		}
	}

	/* Re-enable tx engine */
	dma_txinit(sdpcmd->di);

	/* Repost packets to retransmit */
	while ((p = pktdeq(&txq))) {
		uint8 *swheader = (uint8*)PKTDATA(sdpcmd->osh, p) + SDPCM_FRAMETAG_LEN;
		sdpcmd_txsubmit(sdpcmd, p, SDPCM_PACKET_CHANNEL(swheader));
	}


	/* restore SW queue AFTERWARDS to ensure sdpcmd_tx() enqueue success */
	while ((p = pktq_deq(&txq_tmp, &prec))) {
		if (pktq_mlen(&sdpcmd->txq, (1 << prec)) > sdpcmd->tunables[TXSWQLEN]) {
			SD_ERROR(("%s: txq full\n", __FUNCTION__));
			PKTFREE(sdpcmd->osh, p, TRUE);
			++sdpcmd->tunables[TXDROP];
		} else
			pktq_penq(&sdpcmd->txq, prec, p);
	}

	pktq_deinit(&txq_tmp);
	pktq_deinit(&txq);
}

/* clear rx DMA state and repost buffers */
static void
sdpcmd_rxrecompose(struct dngl_bus *sdpcmd)
{
	void *p;

	SD_TRACE(("sdpcmd_rxrecompose\n"));

	sdpcmd->rxreclaim = TRUE;
	dma_rxreset(sdpcmd->di);
	while ((p = dma_rx(sdpcmd->di))) {
#ifdef BCMDBG_POOL
		PKTPOOLSETSTATE(p, POOL_RXDH);
#endif
		if (sdpcmd_validate(sdpcmd, p) == TRUE) {
		sdpcmd_sendup(sdpcmd, p);
	}
	}

	/* Re-enable rx engine */
	if (POOL_ENAB(sdpcmd->pktpool))
		pktpool_emptycb_disable(sdpcmd->pktpool, TRUE);
	sdpcmd_rxreclaim(sdpcmd);
	dma_rxinit(sdpcmd->di);
	dma_glom_enable(sdpcmd->di, sdpcmd->rxglom);

	/* Repost packets */
	if (POOL_ENAB(sdpcmd->pktpool))
		pktpool_emptycb_disable(sdpcmd->pktpool, FALSE);
	sdpcmd_rxfill(sdpcmd);
	sdpcmd->rxreclaim = FALSE;
	sdpcmd_sendheader(sdpcmd);
}

static void
sdpcmd_rxreclaim(struct dngl_bus *sdpcmd)
{
	dma_rxreclaim(sdpcmd->di);
}

/* reclaim all but CurrDesc - 1 */
static void
sdpcmd_txreclaim(struct dngl_bus *sdpcmd)
{
	void *curr = NULL;
	void *prev = NULL;

	SD_TRACE(("sdpcmd_txreclaim\n"));

	while ((curr = dma_getnexttxp(sdpcmd->di, FALSE))) {
		if (prev) {
			ASSERT(sdpcmd->pend_tx != prev);

#if defined(MSGTRACE) || defined(LOGTRACE)
			if (PKTMSGTRACE(prev) ||
#ifdef MSGTRACE
				msgtrace_event_enabled() ||
#endif
				FALSE) {
				PKTSETMSGTRACE(prev, FALSE);
#ifdef MSGTRACE
				msgtrace_sent();
#endif /* MSGTRACE */

#ifdef LOGTRACE
				logtrace_sent();
#endif
			}
#endif /* MSGTRACE || LOGTRACE */

			PKTFREE(sdpcmd->osh, prev, TRUE);
		}
		prev = curr;
	}
	if (prev) {
		if (POOL_ENAB(sdpcmd->pktpool)) {
#if defined(MSGTRACE) || defined(LOGTRACE)
			if (PKTMSGTRACE(prev)) {
				PKTSETMSGTRACE(prev, FALSE);
#ifdef MSGTRACE
				msgtrace_sent();
#endif /* MSGTRACE */

#ifdef LOGTRACE
				logtrace_sent();
#endif
			}
#endif /* MSGTRACE || LOGTRACE */
			/*
			 * FIX: Avoid holding onto pkts while using shared pool
			 * If retransmission is high, re-visit this
			*/
			PKTFREE(sdpcmd->osh, prev, TRUE);
		}
		else {
			if (sdpcmd->held_tx) {

#if defined(MSGTRACE) || defined(LOGTRACE)
				if (PKTMSGTRACE(prev) ||
#ifdef MSGTRACE
					msgtrace_event_enabled() ||
#endif
					FALSE) {
					PKTSETMSGTRACE(prev, FALSE);
#ifdef MSGTRACE
					msgtrace_sent();
#endif /* MSGTRACE */

#ifdef LOGTRACE
					logtrace_sent();
#endif
				}
#endif /* MSGTRACE || LOGTRACE */
				PKTFREE(sdpcmd->osh, sdpcmd->held_tx, TRUE);
			}
			sdpcmd->held_tx = prev;
		}
	}
}

static void
_sdpcmd_rxfill(void *data)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus*)data;

	sdpcmd_rxfill(sdpcmd);
	if (sdpcmd->tunables[CREDALL] ||
	    ((sdpcmd->rx_lim != sdpcmd->rx_hlim) &&
	    ((uint8)(sdpcmd->rx_hlim - sdpcmd->rx_seq) <= sdpcmd->credtrig))) {
		sdpcmd_sendheader(sdpcmd);
	}
}

static void
sdpcmd_rxfill(struct dngl_bus *sdpcmd)
{
	if ((sdpcmd->txflowcontrol != 0xff) || (dma_rxactive(sdpcmd->di) == 0)) {
		dma_rxfill(sdpcmd->di);
		if ((dma_rxactive(sdpcmd->di) < sdpcmd->minsched) &&
		    !hnd_dpc_is_executing(sdpcmd->rxfill_dpc)) {
			hnd_dpc_schedule(sdpcmd->rxfill_dpc);
			return;
		}
		sdpcmd->rx_lim = sdpcmd->rx_seq + dma_rxactive(sdpcmd->di);
	}
}

static void
hostmailbox_post(struct dngl_bus *sdpcmd, uint32 val)
{
	SD_TRACE(("hostmailbox_post\n"));

#ifdef DS_PROT
	if (DS_PROT_ENAB(sdpcmd)) {
		if (!sdpcmd->tohostmailacked) {
			if (sdpcmd->tohostmail) {
				++sdpcmd->txmb_ovfl_cnt;
				DS_LOG(sdpcmd, SDPCM_DS_LOG_TYPE_TXMB_OVFLOW,
					((val & 0x0f00) >> 4) | (val & 0x0f),
					((sdpcmd->tohostmail & 0x0f00) >> 4)
					| (sdpcmd->tohostmail & 0x0f),
					((sdpcmd->last_tx_mbmsg & 0x0f00) >> 4)
					| (sdpcmd->last_tx_mbmsg & 0x0f));
			} else {
				DS_LOG(sdpcmd, SDPCM_DS_LOG_TYPE_TXMB_QUEUED,
					sdpcmd->ds_state,
					((val & 0x0f00) >> 4) | (val & 0x0f),
					((sdpcmd->last_tx_mbmsg & 0x0f00) >> 4)
					| (sdpcmd->last_tx_mbmsg & 0x0f));
			}
		}
	}
#endif /* DS_PROT */

	sdpcmd->tohostmail |= val;
	if (sdpcmd->tohostmailacked) {
#ifdef DS_PROT
		if (DS_PROT_ENAB(sdpcmd)) {
			switch (val)
			{
				case HMB_DATA_DSREQ:
					++sdpcmd->metrics.ds_tx_dsreq_cnt;
					break;
				case HMB_DATA_DSEXIT:
					++sdpcmd->metrics.ds_tx_dsexit_cnt;
					break;
				case HMB_DATA_D3ACK:
					++sdpcmd->metrics.ds_tx_d3ack_cnt;
					break;
				case HMB_DATA_D3EXIT:
					++sdpcmd->metrics.ds_tx_d3exit_cnt;
					break;
			}
		}
#endif /* DS_PROT */
		if (sdpcmd->tohostmail & HMB_DATA_FC)
			sdpcmd->tohostmail |= sdpcmd->txflowcontrol << HMB_DATA_FCDATA_SHIFT;
		if (sdpcmd->tohostmail & (HMB_DATA_DEVREADY | HMB_DATA_FWREADY))
			sdpcmd->tohostmail |= SDPCM_PROT_VERSION << HMB_DATA_VERSION_SHIFT;
		DS_INFORM(("MB to host: data %08x, expect ACK\n", sdpcmd->tohostmail));
		W_REG(sdpcmd->osh, &sdpcmd->regs->tohostmailboxdata, sdpcmd->tohostmail);
#ifdef DS_PROT
		if (DS_PROT_ENAB(sdpcmd)) {
			sdpcmd->last_tx_mbmsg = sdpcmd->tohostmail;
			DSMB_LOG(sdpcmd, sdpcmd->tohostmail, FALSE);
		}
#endif /* DS_PROT */
		sdpcmd->tohostmail = 0;
		sdpcmd->tohostmailacked = FALSE;
		W_REG(sdpcmd->osh, &sdpcmd->regs->tohostmailbox, HMB_HOST_INT);
		sdpcmd_wakehost(sdpcmd);
	}
	else {
		/* Leave the mbox msg enqueued in sdpcmd->tohostmail to be sent
		 * later when I_SMB_INT_ACK is received.
		 */
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
/* update software copy of hardware counters (they peg, not wrap) */
static void
sdpcmd_statsupd(struct dngl_bus *sdpcmd)
{
	sdpcmd_regs_t *regs = sdpcmd->regs;

	sdpcmd->cnt.cmd52rd = R_REG(sdpcmd->osh, &regs->cmd52rd);
	sdpcmd->cnt.cmd52wr = R_REG(sdpcmd->osh, &regs->cmd52wr);
	sdpcmd->cnt.cmd53rd = R_REG(sdpcmd->osh, &regs->cmd53rd);
	sdpcmd->cnt.cmd53wr = R_REG(sdpcmd->osh, &regs->cmd53wr);
	sdpcmd->cnt.abort = R_REG(sdpcmd->osh, &regs->abort);
	sdpcmd->cnt.datacrcerror = R_REG(sdpcmd->osh, &regs->datacrcerror);
	sdpcmd->cnt.rdoutofsync = R_REG(sdpcmd->osh, &regs->rdoutofsync);
	sdpcmd->cnt.wroutofsync = R_REG(sdpcmd->osh, &regs->wroutofsync);
	sdpcmd->cnt.writebusy = R_REG(sdpcmd->osh, &regs->writebusy);
	sdpcmd->cnt.readwait = R_REG(sdpcmd->osh, &regs->readwait);
	sdpcmd->cnt.readterm = R_REG(sdpcmd->osh, &regs->readterm);
	sdpcmd->cnt.writeterm = R_REG(sdpcmd->osh, &regs->writeterm);
}
#endif /* BCMDBG || BCMDBG_DUMP */

/*
 * Generic bus interface routines
 */

int
sdpcmd_bus_tx(struct dngl_bus *bus, void *p)
{
	int channel;
	struct dngl_bus *sdpcmd = (struct dngl_bus *)bus;

	SD_TRACE(("bus_tx\n"));

	if (PKTTYPEEVENT(bus->osh, p))
		channel = SDPCM_EVENT_CHANNEL;
	else
	/* Packets are expected to be ethernet packets */
	{
		struct ether_header *eh;
		struct bdc_header *bdc_hdr;
		bdc_hdr = (struct bdc_header *) PKTDATA(bus->osh, p);
		eh = (struct ether_header *)(PKTDATA(bus->osh, p) + sdpcmd->tunables[BDCHEADERLEN]
			+ (bdc_hdr->dataOffset * 4));
		if (eh->ether_type == hton16(ETHER_TYPE_BRCM))
			channel = SDPCM_EVENT_CHANNEL;
		else
			channel = SDPCM_DATA_CHANNEL;
	}

	return sdpcmd_tx(bus, p, channel);
}

void
sdpcmd_bus_sendctl(struct dngl_bus *bus, void *p)
{
	SD_TRACE(("bus_sendctl\n"));

	sdpcmd_tx(bus, p, SDPCM_CONTROL_CHANNEL);
}

void
sdpcmd_bus_rxflowcontrol(struct dngl_bus *bus, bool state, int prio)
{
	uint8 val;

	SD_TRACE(("bus_rxflowcontrol %s\n", state == ON ? "ON" : "OFF"));

	/* convert prio to bitmask */
	val = (prio == ALLPRIO) ? 0xff : NBITVAL(PRIO2PREC(prio));
	if (state == ON)
		bus->txflowcontrol |= val;
	else
		bus->txflowcontrol &= ~val;

	/* Skip if no new announcement to host is required */
	if (bus->txflowcontrol == bus->txflowlast) {
		bus->txflowpending = FALSE;
		return;
	}

	/* Try to avoid waking host just to turn flow-control ON; if asleep
	 * it won't send us data anyway. When turning flow-control OFF, should
	 * announce in case there was pending data when entering sleep -- but
	 * the above should suppress it if the preceding ON was deferred here.
	 */
	if (bus->txflowcontrol & ~bus->txflowlast) {
		if (!bus->oob_only && bus->use_oob) {
			bus->txflowpending = TRUE;
			return;
		}
#ifdef DS_PROT
		if (DS_PROT_ENAB(bus)) {
			if (bus->ds_state == DS_D3_STATE) {
				bus->txflowpending = TRUE;
				return;
			}
		}
#endif /* DS_PROT */
	}

	sdpcmd_sendheader(bus);
}

void
sdpcmd_bus_softreset(struct dngl_bus *bus)
{
	SD_TRACE(("bus_softreset\n"));

	/* nothing to do for SDIO/PCMCIA */
}

void
sdpcmd_set_maxtxpktglom(struct dngl_bus *bus, uint8 txpktglom)
{
	SD_TRACE(("sdpcmd_set_maxtxpktglom\n"));

	bus->tunables[MAXTXPKTGLOM] = txpktglom;
	return;
}

static int
sdpcmd_pwrstats(struct dngl_bus *sdpcmd, char *buf, uint32 inlen, uint32 *outlen, bool set,
	int offset)
{
	uint16 type;
	wl_pwr_sdio_stats_t *sdio_tuple;

	if (set) {
		return BCME_ERROR;
	}
	if (inlen < offset + sizeof(uint16))
		return BCME_ERROR;
	if (inlen < ROUNDUP(sizeof(*sdio_tuple), sizeof(uint32)))
		return BCME_BUFTOOSHORT;

	memcpy(&type, &buf[offset], sizeof(uint16));
	if (type != WL_PWRSTATS_TYPE_SDIO)
		return BCME_BADARG;

	sdpcmd->metrics.active_dur += OSL_SYSUPTIME() - sdpcmd->metric_ref.active;
	sdpcmd->metric_ref.active = OSL_SYSUPTIME();

	/* Copy it into the return buffer as tuple form */
	ASSERT(sizeof(sdpcmd->metrics) == sizeof(sdio_tuple->sdio));
	sdio_tuple = (wl_pwr_sdio_stats_t *)buf;
	sdio_tuple->type = WL_PWRSTATS_TYPE_SDIO;
	sdio_tuple->len = sizeof(*sdio_tuple);
	memcpy(&sdio_tuple->sdio, &sdpcmd->metrics, sizeof(sdio_tuple->sdio));
	*outlen = sizeof(*sdio_tuple);
	return BCME_OK;
}

uint32
sdpcmd_bus_iovar(struct dngl_bus *sdpcmd, char *buf, uint32 inlen, uint32 *outlen, bool set)
{
	char *cmd = buf + 4;
	int index = -1;
	uint32 val;
	int offset;

	for (offset = 0; offset < inlen; ++offset) {
		if (buf[offset] == '\0')
			break;
	}

	if (buf[offset] != '\0')
		return BCME_BADARG;

	++offset;

	if (set && (offset + sizeof(uint32)) > inlen)
		return BCME_BUFTOOSHORT;

	if (set)
		memcpy(&val, buf + offset, sizeof(uint32));

	if (!strcmp(cmd, "maxtxpktglom")) {
		index = MAXTXPKTGLOM;
		if (set && (2 * (val + 1)) > (sdpcmd->tunables[NTXD] - 1)) {
			return BCME_RANGE;
		}
	} else if (!strcmp(cmd, "txlazydelay")) {
		index = TXLAZYDELAY;
	} else if (!strcmp(cmd, "txglomalign")) {
		index = TXGLOMALIGN;
	} else if (!strcmp(cmd, "txglom")) {
		if (set) {
			switch (val) {
			case 0: case 1: case 7: case 8:
				break;
#if GLOMTEST
			case 2: case 3: case 4: case 5: case 6:
				break;
#endif /* GLOMTEST */
			default:
				return BCME_RANGE;
			}
		}
		index = TXGLOM;
	} else if (!strcmp(cmd, "txswqlen")) {
		index = TXSWQLEN;
	} else if (!strcmp(cmd, "txdrop")) {
		index = TXDROP;
	} else if (!strcmp(cmd, "ackfastfwd")) {
		index = ACKFASTFWD;
	} else if (!strcmp(cmd, "acksizethsd")) {
		index = ACKSIZETHSD;
	} else if (!strcmp(cmd, "rxacks")) {
		index = RXACKS;
	} else if (!strcmp(cmd, "rxfilltrig")) {
		index = RXFILLTRIG;
		if (set && val > sdpcmd->tunables[RXBUFS])
			return BCME_RANGE;
	} else if (!strcmp(cmd, "credall")) {
		index = CREDALL;
	} else if (!strcmp(cmd, "credfail")) {
		index = CREDFAIL;
	} else if (!strcmp(cmd, "rxcb")) {
		index = RXCB;
#ifdef SECI_UART
	} else if (!strcmp(cmd, "serial")) {
		if (set) {
			if (val == 0 || val == 1) {
				si_seci_clk_force(sdpcmd->sih, val);
			} else {
				return BCME_BADARG;
			}
		} else {
			val = si_seci_clk_force_status(sdpcmd->sih);
			memcpy(buf, &val, sizeof(uint32));
			*outlen = sizeof(uint32);
		}
		return BCME_OK;
#else
	} else if (!strcmp(cmd, "serial") || !strcmp(cmd, "forcealp")) {
		if (set && val) {
			OR_REG(sdpcmd->osh, &sdpcmd->regs->clockctlstatus, CCS_FORCEALP);
		} else if (set) {
			AND_REG(sdpcmd->osh, &sdpcmd->regs->clockctlstatus, ~CCS_FORCEALP);
		} else {
			val = !!(R_REG(sdpcmd->osh, &sdpcmd->regs->clockctlstatus) & CCS_FORCEALP);
		}
#endif /* SECI_UART */
	} else if (!strcmp(cmd, "rxglom")) {
		if (set) {
			if (dma_glom_enable(sdpcmd->di, val)) {
				sdpcmd->rxglom = val;
			} else {
				return BCME_UNSUPPORTED;
			}
		} else {
			memcpy(buf, &sdpcmd->rxglom, sizeof(uint32));
			*outlen = sizeof(uint32);
		}
		return BCME_OK;
#ifdef DS_PROT
	} else if (!strcmp(cmd, "force_awake")) {
		if (DS_PROT_ENAB(sdpcmd)) {
			if (set) {
				sdpcmd->force_awake = val;
				if (sdpcmd->force_awake) {
					OR_REG(sdpcmd->osh, &sdpcmd->regs->clockctlstatus,
						CCS_HTAREQ);
				} else {
					sdpcmd->force_awake = FALSE;
				}
			} else {
				val = sdpcmd->force_awake;
			}
		} else {
			return BCME_UNSUPPORTED;
		}
	} else if (!strcmp(cmd, "ds_log")) {
		if (DS_PROT_ENAB(sdpcmd)) {
			if (set) {
				/* Resize the Deepsleep log buffer to the given size */
				if (val > 0)
					return sdpcmd_ds_log_init(sdpcmd, val);
				else
					return sdpcmd_ds_log_deinit(sdpcmd);
			} else {
				/* Get the size of the Deepsleep log buffer */
				val = sdpcmd->ds_log_size;
			}
		} else {
			return BCME_UNSUPPORTED;
		}
#ifdef DS_LOG_DUMP
	} else if (!strcmp(cmd, "dump_ds_log")) {
		return sdpcmd_ds_engine_log_dump(sdpcmd);
#endif /* DS_LOG_DUMP */
#ifdef DS_PRINTF_LOG
	} else if (!strcmp(cmd, "ds_check_print")) {
		if (set)
			sdpcmd->ds_check_print = val;
		else
			val = sdpcmd->ds_check_print;
	} else if (!strcmp(cmd, "ds_inform_print")) {
		if (set)
			sdpcmd->ds_inform_print = val;
		else
			val = sdpcmd->ds_inform_print;
	} else if (!strcmp(cmd, "ds_trace_print")) {
		if (set)
			sdpcmd->ds_trace_print = val;
		else
			val = sdpcmd->ds_trace_print;
#endif /* DS_PRINTF_LOG */
	} else if (!strcmp(cmd, "ds_disable_state") && !set) {
		if (DS_PROT_ENAB(sdpcmd)) {
			val = (sdpcmd->ds_hwait << 8) | sdpcmd->ds_disable_state;
		} else {
			return BCME_UNSUPPORTED;
		}
	} else if (!strcmp(cmd, "ds_check_interval")) {
		if (DS_PROT_ENAB(sdpcmd)) {
				if (set)
				sdpcmd->ds_check_interval = val;
			else
				val = sdpcmd->ds_check_interval;
		} else {
			return BCME_UNSUPPORTED;
		}
	} else if (!strcmp(cmd, "ds_check_timer_max")) {
		if (DS_PROT_ENAB(sdpcmd)) {
			if (set)
				sdpcmd->ds_check_timer_max = val;
			else
				val = sdpcmd->ds_check_timer_max;
		} else {
			return BCME_UNSUPPORTED;
		}
	} else if (!strcmp(cmd, "dw_gpio")) {
		val = sdpcmd_read_device_wake_gpio(sdpcmd);
#endif /* DS_PROT */
#if SDIODEV_BUS_METRICS
	} else if (!strcmp(cmd, "metrics")) {
		if (set) {
			bzero(&sdpcmd->metrics, sizeof(sdpcmd->metrics));
			bzero(&sdpcmd->metric_ref, sizeof(sdpcmd->metric_ref));
		} else {
			sdpcmd->metrics.active_dur += OSL_SYSUPTIME() - sdpcmd->metric_ref.active;
			sdpcmd->metric_ref.active = OSL_SYSUPTIME();
			val = sdpcmd->metrics.active_dur;

		printf("\nactive_dur\t\t%u\nwakehost\t\t%u\n",
			sdpcmd->metrics.active_dur ? sdpcmd->metrics.active_dur : OSL_SYSUPTIME(),
			sdpcmd->metrics.wakehost_cnt);
		printf("\nintr count:data\t%u\tmailbox\t%u\terror\t%u\n",
			sdpcmd->metrics.data_intr_cnt,
			sdpcmd->metrics.mb_intr_cnt,
			sdpcmd->metrics.error_intr_cnt);
		printf("\nds_wake:on\t\t%u\ndur\t\t%u\noff\t\t%u\ndur\t\t%u\n"
			"ds_state:d0\t\t%u\ndur\t\t%u\nd3\t\t%u\ndur\t\t%u\n"
			"DEV_WAKE count:ASSRT:%u\tDASST:%u\n"
			"rx:dsack:%u\tdsnack:%d\td3inform:%d\t\t\n"
			"tx:dsreq:%u\tdsexit:%u\td3ack:%u\td3exit:%u\n",
			sdpcmd->metrics.ds_wake_on_cnt,
			sdpcmd->metrics.ds_wake_on_dur,
			sdpcmd->metrics.ds_wake_off_cnt,
			sdpcmd->metrics.ds_wake_off_dur,
			sdpcmd->metrics.ds_d0_cnt,
			sdpcmd->metrics.ds_d0_dur,
			sdpcmd->metrics.ds_d3_cnt,
			sdpcmd->metrics.ds_d3_dur,
			sdpcmd->metrics.ds_dw_assrt_cnt,
			sdpcmd->metrics.ds_dw_dassrt_cnt,
			sdpcmd->metrics.ds_rx_dsack_cnt,
			sdpcmd->metrics.ds_rx_dsnack_cnt,
			sdpcmd->metrics.ds_rx_d3inform_cnt,
			sdpcmd->metrics.ds_tx_dsreq_cnt,
			sdpcmd->metrics.ds_tx_dsexit_cnt,
			sdpcmd->metrics.ds_tx_d3ack_cnt,
			sdpcmd->metrics.ds_tx_d3exit_cnt);
		}
#endif /* SDIODEV_BUS_METRICS */
	} else if (!strcmp(cmd, "pwrstats")) {
		return sdpcmd_pwrstats(sdpcmd, buf, inlen, outlen, set, offset);
	} else if (!strcmp(cmd, "gci_dwa_cnt")) {
		val = sdpcmd->metrics.ds_dw_assrt_cnt;
	} else if (!strcmp(cmd, "gci_dwd_cnt")) {
		val = sdpcmd->metrics.ds_dw_dassrt_cnt;
	} else if (!strcmp(cmd, "credtrig")) {
		if (set) {
			if ((val < 2) || (val > 255)) {
				return BCME_RANGE;
			}
			sdpcmd->credtrig = (uint8)val;
		} else {
			val = sdpcmd->credtrig;
		}
	} else if (!strcmp(cmd, "glomevents")) {
		if (set) {
			if (val > 3) {
				return BCME_RANGE;
			}
			sdpcmd->glomevents = val;
		} else {
			val = sdpcmd->glomevents;
		}
	} else if (!strcmp(cmd, "minsched")) {
		if (set) {
			if ((val < 2) || (val > 255)) {
				return BCME_RANGE;
			}
			sdpcmd->minsched = (uint8)val;
		} else {
			val = sdpcmd->minsched;
		}
	} else {
		return BCME_UNSUPPORTED;
	}

	/* A valid index means a corresponding tunable */
	if (index != -1) {
		if (set) {
			sdpcmd->tunables[index] = val;
		} else {
			val = sdpcmd->tunables[index];
		}
	}

	if (!set) {
		memcpy(buf, &val, sizeof(uint32));
		*outlen = sizeof(uint32);
	}

	return BCME_OK;
}


#ifdef BCMDBG
int
sdpcmd_loopback(struct dngl_bus *sdpcmd, char *buf, uint count)
{
	void *p, *p0 = NULL;
	uint32 intstatus;
	int timeout = 1000; /* us */
	char dmabuf[1024];
	struct bcmstrbuf b;
	uint currid;

	bcm_binit(&b, dmabuf, 1024);
	SD_TRACE(("sdpcmd_loopback\n"));

	currid = si_coreid(sdpcmd->sih);
	if (currid != sdpcmd->coreid)
		si_setcore(sdpcmd->sih, sdpcmd->coreid, 0);

	if (!sdpcmd->di) {
		SD_ERROR(("dma must be initialized\n"));
		return -22;
	}

	dma_fifoloopbackenable(sdpcmd->di);

	/* Send packet */
	if (!(p = (PKTGET(sdpcmd->osh, count + sdpcmd->tunables[DONGLEOVERHEAD], TRUE)))) {
		SD_ERROR(("out of txbufs\n"));
		return -12;
	}
	/* reserve space for dongle msg header */
	PKTSETLEN(sdpcmd->osh, p, count + sdpcmd->tunables[DONGLEHDRSZ]);
	PKTPULL(sdpcmd->osh, p, sdpcmd->tunables[DONGLEHDRSZ]);

	bcopy(buf, PKTDATA(sdpcmd->osh, p), count);
	intstatus = R_REG(sdpcmd->osh, &sdpcmd->regs->intstatus);
	SD_TRACE(("\nsdpcmd_loopback: before sdpcmd_tx: intstatus: 0x%x\n", intstatus));
	sdpcmd_dumpdma(sdpcmd, &b);
	SD_TRACE((b.origbuf));

	sdpcmd_tx(sdpcmd, p, SDPCM_DATA_CHANNEL);

	intstatus = R_REG(sdpcmd->osh, &sdpcmd->regs->intstatus);
	SD_TRACE(("\nsdpcmd_loopback: after sdpcmd_tx: intstatus: 0x%x\n", intstatus));
	sdpcmd_dumpdma(sdpcmd, &b);
	SD_TRACE((b.origbuf));
	/* Wait for packet */
	do {
		intstatus = R_REG(sdpcmd->osh, &sdpcmd->regs->intstatus);
		/* Handle DMA interrupts */
		if ((intstatus & I_DMA)) {
			/* Handle DMA errors */
			if (intstatus & I_ERRORS)
				SD_ERROR(("sdpcmd_loopback errors: 0x%x\n", intstatus & I_ERRORS));
			/* Handle DMA receive interrupt */
			if (intstatus & I_RI)
				if ((p0 = dma_rx(sdpcmd->di)))
					break;
		}
		OSL_DELAY(1);
	} while (timeout--);

	SD_TRACE(("\nsdpcmd_loopback: after delay: intstatus: 0x%x\n", intstatus));
	sdpcmd_dumpdma(sdpcmd, &b);
	SD_TRACE((b.origbuf));

	if (!p0) {
		/* No loopback packet received */
		SD_ERROR(("loopback failed\n"));
	} else {
		sdpcmd_rxh_t *rxh;
		/* Strip off rx header */
		rxh = (sdpcmd_rxh_t *) PKTDATA(sdpcmd->osh, p0);
		SD_TRACE(("rx pkt header: len %d; flags 0x%x; PKTLEN: %d\n", rxh->len,
		       rxh->flags, PKTLEN(sdpcmd->osh, p0)));
		PKTPULL(sdpcmd->osh, p0, SDPCMD_RXOFFSET);
		if (PKTLEN(sdpcmd->osh, p) != PKTLEN(sdpcmd->osh, p0) ||
		    bcmp(PKTDATA(sdpcmd->osh, p), PKTDATA(sdpcmd->osh, p0),
		         PKTLEN(sdpcmd->osh, p0))) {
			SD_ERROR(("data mismatch\n"));
			SD_PRPKT("tx", PKTDATA(sdpcmd->osh, p), PKTLEN(sdpcmd->osh, p));
			if (PKTLEN(sdpcmd->osh, p) != PKTLEN(sdpcmd->osh, p0))
				SD_TRACE(("rx packet PKTLEN (%d) != tx packet PKTLEN (%d)\n",
				       PKTLEN(sdpcmd->osh, p0), PKTLEN(sdpcmd->osh, p)));
			if (PKTLEN(sdpcmd->osh, p0) > 0)
				SD_PRPKT("rx", PKTDATA(sdpcmd->osh, p0), PKTLEN(sdpcmd->osh, p0));
			else
				SD_TRACE(("rx packet has no data\n"));
			if (PKTLEN(sdpcmd->osh, p0) > (SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN)) {
				/* remove HW & SW header */
				PKTPULL(sdpcmd->osh, p0, SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN);
				SD_TRACE(("ascii contents: %s\n", PKTDATA(sdpcmd->osh, p0)));
			} else
				SD_TRACE(("rx packet too short; len = %d\n",
				          PKTLEN(sdpcmd->osh, p0)));
		} else
			SD_ERROR(("loopback succeeded\n"));
		PKTFREE(sdpcmd->osh, p0, FALSE);
	}

	/* Disable tx engine */
	dma_txreset(sdpcmd->di);
	dma_txreclaim(sdpcmd->di, HNDDMA_RANGE_ALL);
	sdpcmd->pend_tx = NULL;
	/* Re-enable tx engine */
	dma_txinit(sdpcmd->di);

	/* Disable rx engine */
	dma_rxreset(sdpcmd->di);
	sdpcmd_rxreclaim(sdpcmd);
	/* Re-enable rx engine */
	dma_rxinit(sdpcmd->di);
	/* Repost packets */
	sdpcmd_rxfill(sdpcmd);

	/* clear any intstatus bits */
	W_REG(sdpcmd->osh, &sdpcmd->regs->intstatus, intstatus);

	if (currid != sdpcmd->coreid)
		si_setcore(sdpcmd->sih, currid, 0);
	return (int) count;
}

int
sdpcmd_transmit(struct dngl_bus *sdpcmd, char *buf, uint count, uint chunk, bool ctl)
{
	void *p0, *p1, *p2;
	uint32 intstatus;
	int32 timeout = 1000000; /* us (1s) */
	uint currid;

	SD_TRACE(("sdpcmd_transmit\n"));

	currid = si_coreid(sdpcmd->sih);
	if (currid != sdpcmd->coreid)
		si_setcore(sdpcmd->sih, sdpcmd->coreid, 0);

	if (!sdpcmd->di) {
		SD_ERROR(("dma must be initialized\n"));
		return -22;
	}

	/* Send packet, so allocate necessary chunks (a la cdc_indicate()) */
	if (chunk > count)
		chunk = count;

	if (!(p0 = p1 = (PKTGET(sdpcmd->osh, chunk + sdpcmd->tunables[DONGLEOVERHEAD], TRUE)))) {
		SD_ERROR(("out of txbufs\n"));
		return -12;
	}
	/* reserve space for dongle msg header */
	PKTSETLEN(sdpcmd->osh, p0, chunk + sdpcmd->tunables[DONGLEHDRSZ]);
	PKTPULL(sdpcmd->osh, p0, sdpcmd->tunables[DONGLEHDRSZ]);

	bcopy(buf, PKTDATA(sdpcmd->osh, p0), chunk);
	count -= chunk;
	buf += chunk;

	while (count) {
		if (chunk > count)
			chunk = count;
		if (!(p2 = PKTGET(sdpcmd->osh, chunk, TRUE))) {
			SD_ERROR(("out of txbufs\n"));
			PKTFREE(sdpcmd->osh, p0, TRUE);
			return -12;
		}
		bcopy(buf, (char*)PKTDATA(sdpcmd->osh, p2), chunk);
		count -= chunk;
		buf += chunk;

		PKTSETNEXT(sdpcmd->osh, p1, p2);
		p1 = p2;
	}

	intstatus = R_REG(sdpcmd->osh, &sdpcmd->regs->intstatus);

	SD_TRACE(("\nsdpcmd_transmit: before sdpcmd_tx: intstatus: 0x%x\n", intstatus));
	{
		struct bcmstrbuf b;
		char dmabuf[1024];
		bcm_binit(&b, dmabuf, 1024);
		sdpcmd_dumpdma(sdpcmd, &b);
		SD_TRACE((b.origbuf));

		sdpcmd_tx(sdpcmd, p0, (ctl ? SDPCM_CONTROL_CHANNEL : SDPCM_DATA_CHANNEL));

		intstatus = R_REG(sdpcmd->osh, &sdpcmd->regs->intstatus);
		SD_TRACE(("\nsdpcmd_transmit: after sdpcmd_tx: intstatus: 0x%x\n", intstatus));
		sdpcmd_dumpdma(sdpcmd, &b);
		SD_TRACE((b.origbuf));
	}

	/* Wait for packet */
	do {
		intstatus = R_REG(sdpcmd->osh, &sdpcmd->regs->intstatus);
		/* Handle DMA interrupts */
		if ((intstatus & I_DMA)) {
			/* Handle DMA errors */
			if (intstatus & I_ERRORS)
				SD_ERROR(("sdpcmd_loopback errors: 0x%x\n", intstatus & I_ERRORS));
			/* Handle DMA receive interrupt */
			if (intstatus & I_XI) {
				SD_TRACE(("Got Tx Interrupt\n"));
				break;
			}
		}
		OSL_DELAY(1);
	} while (timeout--);

	if (timeout > 0)
		printf("Tx interrupt after %d us\n", (1000000 - timeout));
	else
		printf("Tx timeout waiting for interrupt, giving up.\n");

	SD_TRACE(("\nsdpcmd_transmit: after delay: intstatus: 0x%x\n", intstatus));

	/* Disable tx engine */
	dma_txreset(sdpcmd->di);
	dma_txreclaim(sdpcmd->di, HNDDMA_RANGE_ALL);
	sdpcmd->pend_tx = NULL;
	/* Re-enable tx engine */
	dma_txinit(sdpcmd->di);

	/* clear any intstatus bits */
	W_REG(sdpcmd->osh, &sdpcmd->regs->intstatus, intstatus);

	if (currid != sdpcmd->coreid)
		si_setcore(sdpcmd->sih, currid, 0);
	return (int) count;
}
#endif	/* BCMDBG */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
sdpcmd_dumpint(uint32 val, struct bcmstrbuf *b)
{
	if (val & I_SMB_NAK) bcm_bprintf(b, "SMB_NAK ");
	if (val & I_SMB_INT_ACK) bcm_bprintf(b, "SMB_INT_ACK ");
	if (val & I_SMB_USE_OOB) bcm_bprintf(b, "SMB_USE_OOB ");
	if (val & I_SMB_DEV_INT) bcm_bprintf(b, "SMB_DEV_INT ");
	if (val & I_HMB_INT_ACK) bcm_bprintf(b, "HMB_INT_STATE ");
	if (val & I_HMB_FRAME_IND) bcm_bprintf(b, "HMB_FRAME_IND ");
	if (val & I_HMB_HOST_INT) bcm_bprintf(b, "HMB_HOST_INT ");
	if (val & I_WR_OOSYNC) bcm_bprintf(b, "WR_OOSYNC ");
	if (val & I_RD_OOSYNC) bcm_bprintf(b, "RD_OOSYNC ");
	if (val & I_PC) bcm_bprintf(b, "PC ");
	if (val & I_PD) bcm_bprintf(b, "PD ");
	if (val & I_DE) bcm_bprintf(b, "DE ");
	if (val & I_RU) bcm_bprintf(b, "RU ");
	if (val & I_RO) bcm_bprintf(b, "RO ");
	if (val & I_XU) bcm_bprintf(b, "XU ");
	if (val & I_RI) bcm_bprintf(b, "RI ");
	if (val & I_XI) bcm_bprintf(b, "XI ");
	if (val & I_RF_TERM) bcm_bprintf(b, "RF_TERM ");
	if (val & I_WF_TERM) bcm_bprintf(b, "WF_TERM ");
	if (val & I_PCMCIA_XU) bcm_bprintf(b, "PCMCIA_XU ");
	if (val & I_SBINT) bcm_bprintf(b, "SBINT ");
	if (val & I_CHIPACTIVE) bcm_bprintf(b, "CHIPACTIVE ");
	if (val & I_SRESET) bcm_bprintf(b, "SRESET ");
	if (val & I_IOE2) bcm_bprintf(b, "IOE2 ");

}

static void
sdpcmd_dumpsbint(uint32 val, struct bcmstrbuf *b)
{
	if (val & I_SB_SERR) bcm_bprintf(b, "SB_SERR ");
	if (val & I_SB_RESPERR) bcm_bprintf(b, "SB_RESPERR ");
	if (val & I_SB_SPROMERR) bcm_bprintf(b, "SB_SPROMERR ");

}

void
sdpcmd_dumpdma(struct dngl_bus *sdpcmd, struct bcmstrbuf *b)
{
	if (sdpcmd->di) {
		uint32 val;
		int j;
		uint coreid = sdpcmd->coreid;
		uint corerev = sdpcmd->corerev;

		dma_dump(sdpcmd->di, b, TRUE);
		bcm_bprintf(b, "\n");

		/* only dump fifos when we know the DMA engine is stopped */
		if (dma_rxstopped(sdpcmd->di)) {
			/* Retrieve receive DMA pointers */
			W_REG(sdpcmd->osh,
			      &SDPCMFIFOREG(sdpcmd, coreid, corerev)->fifoaddr, (0x1 << 16));
			val = R_REG(sdpcmd->osh,
			      &SDPCMFIFOREG(sdpcmd, coreid, corerev)->fifodatalow);
			bcm_bprintf(b, "rxdmafifo rp %d wp %d\n",
			               val & 0x7f, (val >> 7) & 0x7f);
			W_REG(sdpcmd->osh,
			      &SDPCMFIFOREG(sdpcmd, coreid, corerev)->fifoaddr, (0x2 << 16));
			val = R_REG(sdpcmd->osh,
			      &SDPCMFIFOREG(sdpcmd, coreid, corerev)->fifodatalow);
			bcm_bprintf(b, "rxdmafifo last word %d; last word enables 0x%x\n",
			               val & 0x7f, (val >> 7) & 0xf);
			W_REG(sdpcmd->osh,
			      &SDPCMFIFOREG(sdpcmd, coreid, corerev)->fifoaddr, (0x3 << 16));
			val = R_REG(sdpcmd->osh,
			      &SDPCMFIFOREG(sdpcmd, coreid, corerev)->fifodatalow);
			bcm_bprintf(b, "rxdmafifo status rp %d; status wp %d\n",
			               val & 0x3, (val >> 2) & 0x3);
			/* Retrieve receive DMA data */
			for (j = 0; j < 128; j++) {
				W_REG(sdpcmd->osh,
				      &SDPCMFIFOREG(sdpcmd, coreid, corerev)->fifoaddr, j);
				val = R_REG(sdpcmd->osh,
				      &SDPCMFIFOREG(sdpcmd, coreid, corerev)->fifodatalow);
				if (j && !(j % 8))
					bcm_bprintf(b, "\n");
				bcm_bprintf(b, "%08x ", val);
			}
			bcm_bprintf(b, "\n");
		}
		if (dma_txstopped(sdpcmd->di)) {
			/* Retrieve transmit DMA pointers */
			W_REG(sdpcmd->osh,
			      &SDPCMFIFOREG(sdpcmd, coreid, corerev)->fifoaddr, (0xb << 16));
			val = R_REG(sdpcmd->osh,
			      &SDPCMFIFOREG(sdpcmd, coreid, corerev)->fifodatalow);
			bcm_bprintf(b, "txdmafifo rp %d wp %d\n",
			               val & 0x7f, (val >> 7) & 0x7f);
			/* Retrieve transmit DMA data */
			for (j = 0; j < 128; j++) {
				W_REG(sdpcmd->osh,
				      &SDPCMFIFOREG(sdpcmd, coreid, corerev)->fifoaddr,
				      (0xa << 16) | j);
				val = R_REG(sdpcmd->osh,
				      &SDPCMFIFOREG(sdpcmd, coreid, corerev)->fifodatalow);
				if (j && !(j % 8))
					bcm_bprintf(b, "\n");
				bcm_bprintf(b, "%08x ", val);
			}
			bcm_bprintf(b, "\n");
		}
	} else
		bcm_bprintf(b, "\nDMA engine not initialized\n");

}

#define	PRREG(name)	bcm_bprintf(b, #name " (0x%x) 0x%x ", OFFSETOF(sdpcmd_regs_t, name), \
						R_REG(sdpcmd->osh, &regs->name))
#define	PRVAL(name)	bcm_bprintf(b, "%s %d ", #name, sdpcmd->cnt.name)
#define	PRNL()		bcm_bprintf(b, "\n")

void
sdpcmd_dumpregs(struct dngl_bus *sdpcmd, struct bcmstrbuf *b)
{
	uint32 val;
	bool sdio;
	sdpcmd_regs_t *regs = sdpcmd->regs;
	uint currid;

	currid = si_coreid(sdpcmd->sih);
	if (currid != sdpcmd->coreid)
		si_setcore(sdpcmd->sih, sdpcmd->coreid, 0);

	val = R_REG(sdpcmd->osh, &regs->corecontrol);
	bcm_bprintf(b, "corecontrol 0x%x ", val);
	if (val & CC_CISRDY) bcm_bprintf(b, "CISRDY ");
	if (val & CC_BPRESEN) bcm_bprintf(b, "BPRESEN ");
	if (val & CC_F2RDY) bcm_bprintf(b, "F2RDY ");
	bcm_bprintf(b, "\n");

	val = R_REG(sdpcmd->osh, &regs->corestatus);
	sdio = (val & CS_PCMCIAMODE) == 0;
	bcm_bprintf(b, "corestatus 0x%x ", val);
	if (val & CS_PCMCIAMODE) bcm_bprintf(b, "PCMCIAMODE ");
	if (val & CS_SMARTDEV) bcm_bprintf(b, "SMARTDEV ");
	if (val & CS_F2ENABLED) bcm_bprintf(b, "F2ENABLED ");
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "biststatus 0x%x\n",
	               R_REG(sdpcmd->osh, &regs->biststatus));

	val = R_REG(sdpcmd->osh, &regs->intstatus);
	bcm_bprintf(b, "intstatus 0x%x ", val);
	sdpcmd_dumpint(val, b);
	bcm_bprintf(b, "\n");
	val = R_REG(sdpcmd->osh, &regs->intmask);
	bcm_bprintf(b, "intmask 0x%x ", val);
	sdpcmd_dumpint(val, b);
	bcm_bprintf(b, "\n");
	val = R_REG(sdpcmd->osh, &regs->hostintmask);
	bcm_bprintf(b, "hostintmask 0x%x ", val);
	sdpcmd_dumpint(val, b);
	bcm_bprintf(b, "\n");
	val = R_REG(sdpcmd->osh, &regs->sbintstatus);
	bcm_bprintf(b, "sbintstatus 0x%x ", val);
	sdpcmd_dumpsbint(val, b);
	bcm_bprintf(b, "\n");
	val = R_REG(sdpcmd->osh, &regs->sbintmask);
	bcm_bprintf(b, "sbintmask 0x%x ", val);
	sdpcmd_dumpsbint(val, b);
	bcm_bprintf(b, "\n");
	val = R_REG(sdpcmd->osh, &regs->tosbmailbox);
	bcm_bprintf(b, "tosbmailbox 0x%x ", val);
	val = R_REG(sdpcmd->osh, &regs->tohostmailbox);
	bcm_bprintf(b, "tohostmailbox 0x%x\n", val);
	val = R_REG(sdpcmd->osh, &regs->tosbmailboxdata);
	bcm_bprintf(b, "tosbmailboxdata 0x%x ", val);
	val = R_REG(sdpcmd->osh, &regs->tohostmailboxdata);
	bcm_bprintf(b, "tohostmailboxdata 0x%x\n", val);
	val = R_REG(sdpcmd->osh, &regs->intrcvlazy);
	bcm_bprintf(b, "intrcvlazy 0x%x\n", val);

	bcm_bprintf(b, "\n");
	sdpcmd_dumpdma(sdpcmd, b);
	bcm_bprintf(b, "\n");

	sdpcmd_statsupd(sdpcmd);
	if (sdio) {
		PRREG(sdioaccess); PRNL();

		/* SDIO counters */
		PRVAL(cmd52rd); PRVAL(cmd52wr); PRVAL(cmd53rd); PRVAL(cmd53wr); PRNL();
		PRVAL(abort); PRVAL(datacrcerror); PRVAL(rdoutofsync); PRNL();
		PRVAL(wroutofsync); PRVAL(writebusy); PRVAL(readwait); PRNL();
		PRVAL(rxdescuflo); PRVAL(rxfifooflo); PRVAL(txfifouflo); PRNL();
		PRVAL(runt); PRVAL(badlen); PRVAL(badcksum); PRVAL(seqbreak); PRNL();
		PRVAL(rxfcrc); PRVAL(rxfwoos); PRVAL(rxfwft); PRVAL(rxfabort); PRNL();
		PRVAL(woosint); PRVAL(roosint); PRVAL(rftermint); PRVAL(wftermint); PRNL();
	} else {
		PRREG(pcmciamesportaladdr); PRREG(pcmciamesportalmask);
		PRREG(pcmciawrframebc); PRREG(pcmciaunderflowtimer); PRNL();
		PRREG(pcmciaframectrl); PRREG(backplanecsr);
		PRREG(backplaneaddr0); PRREG(backplaneaddr1);
		PRREG(backplaneaddr2); PRREG(backplaneaddr3); PRNL();
		PRREG(backplanedata0); PRREG(backplanedata1);
		PRREG(backplanedata2); PRREG(backplanedata3);
		PRREG(spromstatus); PRNL();
		PRVAL(rdoutofsync); PRVAL(wroutofsync); PRNL();
	}

	si_dumpregs(sdpcmd->sih, b);

	if (currid != sdpcmd->coreid)
		si_setcore(sdpcmd->sih, currid, 0);
}
#else /* !BCMDBG && !BCMDBG_DUMP */
int
sdpcmd_loopback(struct dngl_bus *sdpcmd, char *buf, uint count)
{
	return (int) count;
}

void
sdpcmd_dumpregs(struct dngl_bus *sdpcmd, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "(No dumpregs without BCMDBG)\n");
}
#endif /* !BCMDBG && !BCMDBG_DUMP */

#ifdef RTE_CONS
static void
sdpcmd_dumpstats(void *dngl_bus, int argc, char *argv[])
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)dngl_bus;

	printf("txq len: %d: max(%d)\n", pktq_n_pkts_tot(&sdpcmd->txq), sdpcmd->tunables[TXSWQLEN]);
	printf("dropped frames %d\n", sdpcmd->tunables[TXDROP]);
	printf("bus errors: rxfifoflow: %d\n", sdpcmd->cnt.rxfifooflo);
	printf("bus flowcontrol: rxlim: %d, rxh_lim: %d\n", sdpcmd->rx_lim, sdpcmd->rx_hlim);
	printf("dongle flowcontrol: fc: %d, explicit: %d, fail: %d\n", sdpcmd->alltx_fc,
	       sdpcmd->explicit_fc, sdpcmd->sendhdr_fail);
}
#endif /* RTE_CONS */

#ifdef BCMDBG_SD_LATENCY
void
sdpcmd_timestamp_arrival(struct dngl_bus *sdpcmd, void * p)
{
	sdstats_latency *lat_stats = &sdpcmd->lat_stats;
	sdpcmd_pkttag_t *sdtag = NULL;
	uint32 arrival_latency = 0;

	sdtag = SDPCMD_PKTTAG(p);
	if (!sdtag) {
		SD_ERROR(("%s: sdtag invalid\n", __FUNCTION__));
		return;
	}
	sdtag->arrival_time = dngl_time_now_us();
	if (lat_stats->last_arrival == 0) {
		lat_stats->last_arrival = sdtag->arrival_time;
	} else {
		arrival_latency = (lat_stats->last_arrival <= sdtag->arrival_time)?
			(sdtag->arrival_time - lat_stats->last_arrival) :
			(~0 - lat_stats->last_arrival + 1 + sdtag->arrival_time);

		if (lat_stats->arrival_max < arrival_latency)
			lat_stats->arrival_max = arrival_latency;
		if (lat_stats->arrival_min == 0 || arrival_latency < lat_stats->arrival_min)
			lat_stats->arrival_min = arrival_latency;

		lat_stats->last_arrival = sdtag->arrival_time;
	}
	if (lat_stats->pkt_len_max < PKTLEN(osh, p))
		lat_stats->pkt_len_max = PKTLEN(osh, p);
	if (lat_stats->pkt_len_min == 0 || lat_stats->pkt_len_min > PKTLEN(osh, p))
		lat_stats->pkt_len_min = PKTLEN(osh, p);
}

void
sdpcmd_timestamp_transmit(struct dngl_bus *sdpcmd, void *p)
{
	sdstats_latency *lat_stats = &sdpcmd->lat_stats;
	sdpcmd_pkttag_t * sdtag = NULL;
	uint32 send_latency;
	uint32 new_latency_sum = 0;
	uint32 new_pkt_count = 0;

	sdtag = SDPCMD_PKTTAG(p);
	if (!sdtag) {
		SD_ERROR(("%s: sdtag invalid\n", __FUNCTION__));
		return;
	}
	sdtag->transmit_time = dngl_time_now_us();
	send_latency = (sdtag->transmit_time < sdtag->arrival_time) ?
		((~0) - sdtag->arrival_time + 1) + sdtag->transmit_time :
		sdtag->transmit_time - sdtag->arrival_time;

	/* Tally sum */
	new_latency_sum = lat_stats->sum + send_latency;
	new_pkt_count = lat_stats->total_pkt + 1;
	/* Reset tally if overflow */
	if (new_latency_sum < lat_stats->sum || new_pkt_count < lat_stats->total_pkt) {
		lat_stats->sum = send_latency;
		lat_stats->total_pkt = 1;
	} else {
		lat_stats->sum = new_latency_sum;
		lat_stats->total_pkt = new_pkt_count;
	}
	/* Calculate max and min of latency */
	if (send_latency > lat_stats->max)
		lat_stats->max = send_latency;
	if (lat_stats->min == 0 || lat_stats->min > send_latency)
		lat_stats->min = send_latency;
}

#ifdef RTE_CONS
void
sdpcmd_print_latency(void *dngl_bus, int argc, char *argv[])
{
	sdstats_latency *lat_stats = &((struct dngl_bus *)dngl_bus)->lat_stats;

	printf("Latency(us):\n");
	printf("txq_lat_max=%u, txq_lat_min=%u, txq_lat_avg=%u\n",
		lat_stats->max, lat_stats->min,
		lat_stats->total_pkt ? (lat_stats->sum/lat_stats->total_pkt) : 0);
	printf("ar_lat_max=%u, ar_lat_min=%u\n",
		lat_stats->arrival_max, lat_stats->arrival_min);
	printf("pkt_len_max=%u, pkt_len_min=%u, total pkts=%u\n",
		lat_stats->pkt_len_max, lat_stats->pkt_len_min,	lat_stats->total_pkt);
}
#endif
#endif /* BCMDBG_SD_LATENCY */

static bool
sdpcmd_devmail(struct dngl_bus *sdpcmd)
{
	uint32 smb_data;

	smb_data = R_REG(sdpcmd->osh, &sdpcmd->regs->tosbmailboxdata);
	SD_INFORM(("%s: rcvd devmail %08x\n", __FUNCTION__, smb_data));
	(void) smb_data;

	W_REG(sdpcmd->osh, &sdpcmd->regs->tohostmailbox, HMB_INT_ACK);
	sdpcmd_wakehost(sdpcmd);

#ifdef DS_PROT
	if (DS_PROT_ENAB(sdpcmd)) {
		sdpcmd->last_rx_mbmsg = (smb_data >> 4) | (smb_data & 0x0f);
		DSMB_LOG(sdpcmd, smb_data, TRUE);
		if (smb_data & SMB_DATA_D3INFORM) {
			++sdpcmd->metrics.ds_rx_d3inform_cnt;
			hostmailbox_post(sdpcmd, HMB_DATA_D3ACK);
			sdpcmd_ds_engine(sdpcmd, D3_ENTER_EVENT);
		}
		if (smb_data & SMB_DATA_DSACK) {
			++sdpcmd->metrics.ds_rx_dsack_cnt;
			/* Skip "stale" responses */
			if (sdpcmd->ds_check_timer_on) {
				++sdpcmd->ds_stale_dsack;
			} else {
				sdpcmd_ds_engine(sdpcmd, DS_ALLOWED_EVENT);
			}
		}
		if (smb_data & SMB_DATA_DSNACK) {
			++sdpcmd->metrics.ds_rx_dsnack_cnt;
			/* Skip "stale" responses */
			if (sdpcmd->ds_check_timer_on) {
				++sdpcmd->ds_stale_dsnack;
			} else {
				sdpcmd_ds_engine(sdpcmd, DS_NOT_ALLOWED_EVENT);
			}
		}
	}
#endif /* DS_PROT */

	if (smb_data & SMB_DATA_TRAP) {
		/* force trap */
		OSL_SYS_HALT();
	}

	return FALSE;
}

#ifdef DS_PROT
static sdpcmd_ds_state_tbl_entry_t*
BCMRAMFN(sdpcmd_get_ds_state_tbl_entry)(sdpcmd_ds_state_t state, sdpcmd_ds_event_t event)
{
	return (&sdpcmd_ds_state_tbl[state][event]);
}

static void
sdpcmd_forcewake(struct dngl_bus *sdpcmd, bool on)
{
	sdpcmd_regs_t *regs = sdpcmd->regs;
	uint32 now = OSL_SYSUPTIME();

	sdpcmd->force_wake = on;

	if (on) {
		++sdpcmd->metrics.ds_wake_on_cnt;
		sdpcmd->metrics.ds_wake_off_dur += now - sdpcmd->metric_ref.ds_wake_off;
		sdpcmd->metric_ref.ds_wake_off = 0;
		sdpcmd->metric_ref.ds_wake_on = now;

		DS_LOG(sdpcmd, SDPCM_DS_LOG_TYPE_FORCE_WAK, sdpcmd->ds_state, 0, 0);
		OR_REG(sdpcmd->osh, &regs->clockctlstatus, CCS_HTAREQ);
	} else {
		++sdpcmd->metrics.ds_wake_off_cnt;
		sdpcmd->metrics.ds_wake_on_dur += now - sdpcmd->metric_ref.ds_wake_on;
		sdpcmd->metric_ref.ds_wake_off = now;
		sdpcmd->metric_ref.ds_wake_on = 0;

		if (!sdpcmd->force_awake) {
			DS_LOG(sdpcmd, SDPCM_DS_LOG_TYPE_FORCE_SLP, sdpcmd->ds_state, 0, 0);
			AND_REG(sdpcmd->osh, &regs->clockctlstatus, ~CCS_HTAREQ);
#ifdef DBG_DS_TIMER_WAKE
			/* Start a debug timer to wake the chip. */
			++gptimer_start_cnt;
			sdpcmd->gptimer = dngl_init_timer(sdpcmd, NULL, sdpcmd_gptimer);
			dngl_add_timer(sdpcmd->gptimer, 40, TRUE);
#endif /* DBG_DS_TIMER_WAKE */
		}
	}
}

static void
sdpcmd_ds_disable_deepsleep(struct dngl_bus *sdpcmd, bool disable)
{
	DS_TRACE(("DS: disallow %d\n", disable));

	/* Record the [desired] state, the sync actual state */
	sdpcmd->ds_disable_state = disable;

	/* Disable can disable immediately; else determine if we can
	 * enable immediately -- if not, defer using hwait.  On either
	 * immediate action, stop hwait polling if it was active.
	 */
	if (disable) {
		if (!sdpcmd->force_wake) {
			DS_INFORM(("DSoff\n"));
			sdpcmd_forcewake(sdpcmd, TRUE);
		}
		if (sdpcmd->ds_hwait) {
			dngl_del_timer(sdpcmd->ds_hwait_timer);
			sdpcmd->ds_hwait = FALSE;
		}
	} else if (sdpcmd_can_goto_ds(sdpcmd)) {
		DS_INFORM(("DSon\n"));
		sdpcmd_forcewake(sdpcmd, FALSE);
		if (sdpcmd->ds_hwait) {
			dngl_del_timer(sdpcmd->ds_hwait_timer);
			sdpcmd->ds_hwait = FALSE;
		}
	} else {
		DS_INFORM(("DSwait\n"));
		if (!sdpcmd->ds_hwait) {
			sdpcmd->ds_hwait = TRUE;
			sdpcmd->ds_hwait_cnt = 0;
			dngl_add_timer(sdpcmd->ds_hwait_timer, 1, FALSE);
			DS_LOG(sdpcmd, SDPCM_DS_LOG_TYPE_HWAIT_TMR,
				sdpcmd->ds_state,
				sdpcmd_read_device_wake_gpio(sdpcmd),
				sdpcmd->ds_hwait_cnt);
		}
	}
}

static void
sdpcmd_device_wake_isr(uint32 status, void *arg)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)arg;
	uint8 gpio_status;
	bool dw_gpio;

	DS_TRACE(("%s: status %04x\n", __FUNCTION__, status));

	/* Read DEVICE_WAKE GPIO value */
	gpio_status = sdpcmd_read_device_wake_gpio(sdpcmd);
	dw_gpio = (gpio_status & 0x1) ? TRUE : FALSE;

	/* If the new DW GPIO value is the same as the old one then we missed a DW
	 * transition.
	 */
	if (dw_gpio == sdpcmd->devwake_last) {
		if (dw_gpio) {
			/* DEVICE_WAKE is currently high. Missed a DW_DEASSRT. */
			++sdpcmd->misseddwd_cnt;
		} else {
			/* DEVICE_WAKE is currently low. Missed a DW_ASSRT. */
			++sdpcmd->misseddwa_cnt;
		}
	}
	DS_LOG(sdpcmd, SDPCM_DS_LOG_TYPE_WAKE_ISR,
		dw_gpio, sdpcmd->misseddwd_cnt, sdpcmd->misseddwa_cnt);

	if (dw_gpio) {
		++sdpcmd->metrics.ds_dw_assrt_cnt;
		sdpcmd_ds_engine(sdpcmd, DW_ASSRT_EVENT);
	} else {
		++sdpcmd->metrics.ds_dw_dassrt_cnt;
		sdpcmd_ds_engine(sdpcmd, DW_DASSRT_EVENT);
	}

	sdpcmd->devwake_last = dw_gpio;
}

static bool
sdpcmd_read_device_wake_gpio(struct dngl_bus *sdpcmd)
{
	return si_gci_gpio_status(sdpcmd->sih, sdpcmd->devwake_gp, 0, 0);
}

static int
BCMATTACHFN(sdpcmd_enable_device_wake)(struct dngl_bus *sdpcmd)
{
	uint8 cur_status = 0;
	uint8 wake_status;
	uint8 gci_gpio;

	sdpcmd->ds_state = DS_DISABLED_STATE;

	gci_gpio = si_enable_device_wake(sdpcmd->sih, &wake_status, &cur_status);
	if (gci_gpio == CC_GCI_GPIO_INVALID) {
		return BCME_ERROR;
	}
	sdpcmd->devwake_gp = gci_gpio;

	DS_INFORM(("%s: Init DS_PROT: NO_DS_STATE\n", __FUNCTION__));
	sdpcmd->ds_state = NO_DS_STATE;
	sdpcmd->force_wake = FALSE;
	sdpcmd_ds_disable_deepsleep(sdpcmd, TRUE);

	cur_status &= GCI_GPIO_STS_VALUE;
	if (cur_status) {
		sdpcmd->devwake_last = TRUE; /* DEVICE_WAKE asserted */
	}
	else {
		sdpcmd->devwake_last = FALSE; /* DEVICE_WAKE de-asserted */
		DS_INFORM(("%s: Initial device_wake low, signal DASSERT\n", __FUNCTION__));
		sdpcmd_ds_engine(sdpcmd, DW_DASSRT_EVENT);
	}

	if (hnd_enable_gci_gpioint(gci_gpio, wake_status, sdpcmd_device_wake_isr, sdpcmd) == NULL) {
		DS_INFORM(("%s: Cannot register gci device_wake handler\n", __FUNCTION__));
		return BCME_ERROR;
	}

	return BCME_OK;
}

static void
sdpcmd_ds_engine(struct dngl_bus *sdpcmd, sdpcmd_ds_event_t event)
{
	sdpcmd_ds_state_tbl_entry_t ds_entry;

	if (sdpcmd->ds_state < 0) {
		DS_TRACE(("DS Engine: invalid state %d\n", __FUNCTION__, sdpcmd->ds_state));
		return;
	}
	ds_entry = *sdpcmd_get_ds_state_tbl_entry(sdpcmd->ds_state, event);

	DS_INFORM(("DS Engine: %s/%s -> %p/%s\n",
	           sdpcmd_ds_state_name(sdpcmd->ds_state), sdpcmd_ds_event_name(event),
	           ds_entry.action_fn, sdpcmd_ds_state_name(ds_entry.transition)));

	if (sdpcmd->in_ds_engine) {
		if ((sdpcmd->ds_int_dtoh_mode == 1) && (event == INT_DTOH_EVENT)) {
			DS_TRACE(("DS Engine: Skip recursive DTOH event\n"));
		} else {
			DS_INFORM(("DS Engine: Skip unexpected recursion, state %s event %s\n",
			           sdpcmd_ds_state_name(sdpcmd->ds_state),
			           sdpcmd_ds_event_name(event)));
		}
		return;
	}
	DS_LOG(sdpcmd, (ds_entry.transition != DS_INVALID_STATE)
		? SDPCM_DS_LOG_TYPE_FSMT : SDPCM_DS_LOG_TYPE_FSM_IGNORE,
		sdpcmd->ds_state, event, ds_entry.transition);

	if (ds_entry.action_fn) {
		sdpcmd->in_ds_engine = TRUE;
		ds_entry.action_fn(sdpcmd);
		sdpcmd->in_ds_engine = FALSE;
	}

	if (ds_entry.transition != DS_INVALID_STATE) {
		sdpcmd->ds_state = ds_entry.transition;
	}
}


#if defined(DS_LOG_DUMP) || defined(DS_PRINTF_LOG)
static const char *
sdpcmd_ds_state_name(sdpcmd_ds_state_t state)
{
	const char *ds_state_names[] = {"NO_DS", "DS_CHECK", "DS_D0", "NODS_D3", "DS_D3"};
	if (state < 0 || state >= sizeof(ds_state_names))
		return "UNKNOWN_ST";
	return ds_state_names[state];
}

static const char *
sdpcmd_ds_event_name(sdpcmd_ds_event_t event)
{
	const char *ds_ev_names[] = {"DW_ASSRT_EV", "DW_DASSRT_EV", "D3_ENTER_EV",
				     "INT_DTOH_EV", "DS_ALLOWED_EV", "DS_NOT_ALLOWED_EV"};
	if (event >= sizeof(ds_ev_names))
		return "UNKNOWN_EV";
	return ds_ev_names[event];
}
#endif /* defined(DS_LOG_DUMP) || defined(DS_PRINTF_LOG) */

#ifdef BCMDBG
struct dngl_bus *g_sdpcmd = NULL;
#endif /* BCMDBG */

#ifdef DS_LOG_DUMP
static const char *
sdpcmd_ds_txmb_name(unsigned int mbmsg)
{
	char *name = "Unknown";

	if (mbmsg & HMB_DATA_DSREQ)
		name = "DSREQ";
	if (mbmsg & HMB_DATA_DSEXIT)
		name = "DSEXIT";
	if (mbmsg & HMB_DATA_D3ACK)
		name = "D3ACK";
	if (mbmsg & HMB_DATA_D3EXIT)
		name = "D3EXIT";
	return name;
}

static const char *
sdpcmd_ds_rxmb_name(unsigned int mbmsg)
{
	char *name = "Unknown";

	if (mbmsg & SMB_DATA_D3INFORM)
		name = "D3INFORM";
	if (mbmsg & SMB_DATA_DSACK)
		name = "DSACK";
	if (mbmsg & SMB_DATA_DSNACK)
		name = "DSNACK";
	return name;
}

static const char *
sdpcmd_ds_logtype_name(uint8 type)
{
	const char *logtype_names[] = {
		"NONE", "FSMT", "ACKWTIM", "GP0M3_SLP", "GP0M3_WAK", "GP0M3_ACK", "HWAIT_TMR",
		"TMO_NODS", "TMO_DS", "TMO_RETRY", "FORCE_WAK", "FORCE_SLP", "FSM_IGN",
		"NODS_D3_DW", "DEBUG2", "MBOX_RX", "MBOX_TX", "MISSED_PULSE_D", "MISSED_PULSE_A",
		"DW_CANT_DS", "DW_END_DSCHK", "DW_END_HWAIT", "SCAN_RES", "TXDMA_FAIL", "TXMB_QD",
		"TXMB_QS", "TXMB_OV", "CNTRS", "WAKE_ISR", "CC_ISR", "HOSTWAKE",
		"UNUSED1", "IOCEND", "UNUSED2", "IOVEND"
	};
	if (type < 0 || type >= sizeof(logtype_names))
		return "UNKNOWN_TYPE";
	return logtype_names[type];
}

static void
sdpcmd_ds_print_log(struct dngl_bus * sdpcmd, sdpcmd_ds_log_t *ds_log)
{
	unsigned int mbmsg;
	unsigned int ioctl;

	switch (ds_log->ds_log_type) {
	case SDPCM_DS_LOG_TYPE_FSMT:
		printf("%u: %s:%s->%s\n", ds_log->ds_time,
			sdpcmd_ds_event_name(ds_log->ds_event),
			sdpcmd_ds_state_name(ds_log->ds_state),
			sdpcmd_ds_state_name(ds_log->ds_transition));
		break;
	case SDPCM_DS_LOG_TYPE_MBOX_RX:
	{
		const char *rxmb_name;

		mbmsg = (ds_log->ds_state << 8) | ds_log->ds_event;
		if (mbmsg != 0 && ((mbmsg & (mbmsg - 1)) != 0)) {
			rxmb_name = "MULTIPLE_BITS!";
		} else {
			rxmb_name = sdpcmd_ds_rxmb_name(mbmsg);
		}
		printf("%u: MB_RX 0x%04x %s\n", ds_log->ds_time,
			mbmsg, rxmb_name);
		break;
	}
	case SDPCM_DS_LOG_TYPE_MBOX_TX:
		mbmsg = (ds_log->ds_state << 8) | ds_log->ds_event;
		printf("%u: MB_TX 0x%04x %s\n", ds_log->ds_time,
			mbmsg, sdpcmd_ds_txmb_name(mbmsg));
		break;
	default:
		printf("%u: %s %u %u %u\n", ds_log->ds_time,
			sdpcmd_ds_logtype_name(ds_log->ds_log_type),
			ds_log->ds_state,
			ds_log->ds_event,
			ds_log->ds_transition);
		break;
	}
}

static int
sdpcmd_ds_engine_log_dump(struct dngl_bus * sdpcmd)
{
	int i;

	if (sdpcmd->ds_state == DS_DISABLED_STATE)
		return BCME_ERROR;

	for (i = 0; i < sdpcmd->ds_log_size; i++) {
		if (sdpcmd->ds_log[i].ds_log_type == SDPCM_DS_LOG_TYPE_NONE) {
			continue;
		}
		sdpcmd_ds_print_log(sdpcmd, &sdpcmd->ds_log[i]);
	}
	return BCME_OK;
}
#endif /* DS_LOG_DUMP */

/* Add a log entry to the Deepsleep log buffer */
void
_sdpcmd_ds_log(struct dngl_bus *sdpcmd, uint16 type, uint state, uint event, uint nextstate)
{
	sdpcmd_ds_log_t *logent = NULL;

	/* If DS_LOGs have not been stopped, add the log to the buffer */
	if (!sdpcmd->ds_log_stop) {
		logent = &sdpcmd->ds_log[sdpcmd->ds_log_index];
		logent->ds_log_type = type;
		logent->ds_time = OSL_SYSUPTIME();
		logent->ds_state = state;
		logent->ds_event = event;
		logent->ds_transition = nextstate;
		if (++sdpcmd->ds_log_index >= sdpcmd->ds_log_size) {
			sdpcmd->ds_log_index = 0;
		}
	}
}

/* Initialize Deepsleep logging */
static int
sdpcmd_ds_log_init(struct dngl_bus *sdpcmd, uint16 max_log_entries)
{
	size_t size;

#ifdef BCMDBG
	g_sdpcmd = sdpcmd;
#endif /* BCMDBG */

	/* Free any existing DS log buffers of the old size */
	(void) sdpcmd_ds_log_deinit(sdpcmd);

	/* Allocate DS log buffer at the new size */
	size = max_log_entries * sizeof(sdpcmd_ds_log_t);
	sdpcmd->ds_log = MALLOCZ(sdpcmd->osh, size);
	if (sdpcmd->ds_log == NULL) {
		/* The previous sdpcmd_ds_log_deinit() call has already reset
		 * ds_log_stop, ds_log_size, and ds_log_index.
		 */
		return BCME_NOMEM;
	}
	sdpcmd->ds_log_size = max_log_entries;
	sdpcmd->ds_log_stop = FALSE;

	return BCME_OK;
}

/* De-initialize Deepsleep logging */
static int
sdpcmd_ds_log_deinit(struct dngl_bus *sdpcmd)
{
	sdpcmd->ds_log_stop = TRUE;

	/* Free any existing DS log buffers */
	if (sdpcmd->ds_log) {
		MFREE(sdpcmd->osh, sdpcmd->ds_log,
			sdpcmd->ds_log_size * sizeof(*sdpcmd->ds_log));
		sdpcmd->ds_log = NULL;
	}
	sdpcmd->ds_log_size = 0;
	sdpcmd->ds_log_index = 0;
	return BCME_OK;
}

/* Log a mailbox msg */
static void
sdpcmd_dsmb_log(struct dngl_bus *sdpcmd, uint32 mbmsg, bool is_rx)
{
	DS_LOG(sdpcmd, is_rx ? SDPCM_DS_LOG_TYPE_MBOX_RX : SDPCM_DS_LOG_TYPE_MBOX_TX,
		(mbmsg >> 8) & 0xff, mbmsg & 0xff, 0);
}

static bool
sdpcmd_can_goto_ds(struct dngl_bus * sdpcmd)
{
	sdpcmd_regs_t *regs = sdpcmd->regs;

	/* If there is pending SDIO activity, cannot sleep */
	if (dma_txactive(sdpcmd->di) || !sdpcmd->tohostmailacked ||
	    (R_REG(sdpcmd->osh, &regs->intstatus) & sdpcmd->defintmask) ||
	    (R_REG(sdpcmd->osh, &regs->intstatus) & R_REG(sdpcmd->osh, &regs->hostintmask))) {
		DS_CHECK(("DS Check: no DS: tx %d gotmback %d ints %08x %08x %08x\n",
		          dma_txactive(sdpcmd->di), sdpcmd->tohostmailacked,
		          R_REG(sdpcmd->osh, &regs->intstatus), sdpcmd->defintmask,
		          R_REG(sdpcmd->osh, &regs->hostintmask)));
		return FALSE;
	}

	if (sdpcmd_read_device_wake_gpio(sdpcmd)) {
		DS_CHECK(("DS Check: no DS due to DW\n"));
		++sdpcmd->dwcannotds_cnt;
		return FALSE;
	}

	return TRUE;
}

static void
sdpcmd_ds_enter_req(struct dngl_bus *sdpcmd)
{
	DS_INFORM(("Send DS Req\n"));
	hostmailbox_post(sdpcmd, HMB_DATA_DSREQ);
}

static void
sdpcmd_ds_exit_notify(struct dngl_bus *sdpcmd)
{
	sdpcmd_ds_disable_deepsleep(sdpcmd, TRUE);

	DS_INFORM(("Send DS EXIT\n"));
	hostmailbox_post(sdpcmd, HMB_DATA_DSEXIT);
}

static void
sdpcmd_d3_exit_notify(struct dngl_bus *sdpcmd)
{
	sdpcmd_ds_disable_deepsleep(sdpcmd, TRUE);

	DS_INFORM(("Send D3 EXIT\n"));
	hostmailbox_post(sdpcmd, HMB_DATA_D3EXIT);
}

static void
sdpcmd_ds_check_periodic(struct dngl_bus *sdpcmd)
{
	DS_INFORM(("Start DS Check\n"));
	if (sdpcmd->ds_check_timer_on == FALSE) {
		sdpcmd->ds_check_timer_on = TRUE;
		sdpcmd->ds_check_timer_cnt = 0;
		dngl_add_timer(sdpcmd->ds_check_timer, sdpcmd->ds_check_interval, FALSE);
	}
}

static void
sdpcmd_ds_check_timerfn(dngl_timer_t *t)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *) hnd_timer_get_ctx(t);

	/* Bump the try counter */
	sdpcmd->ds_check_timer_cnt++;

	/* If SD bus inactive, just initiate DS request to host */
	if (sdpcmd_can_goto_ds(sdpcmd)) {
		sdpcmd->ds_check_timer_on = FALSE;
		sdpcmd->ds_check_timer_cnt = 0;
		DS_INFORM(("DS Check: request ok, check count %d\n",
		           sdpcmd->ds_check_timer_cnt));
		sdpcmd_ds_enter_req(sdpcmd);
		return;
	}

	/* Otherwise, restart the timer -- but give up if optional cap reached */
	if (sdpcmd->ds_check_timer_max &&
	    (sdpcmd->ds_check_timer_cnt > sdpcmd->ds_check_timer_max)) {
		sdpcmd->ds_check_timer_on = FALSE;
		sdpcmd->ds_check_timer_cnt = 0;
		DS_INFORM(("DS Check: Retry timer max %d exceeded, giving up.\n",
		           sdpcmd->ds_check_timer_max));
		return;
	} else {
		DS_CHECK(("DS Check: retry %d\n", sdpcmd->ds_check_timer_cnt));
		dngl_add_timer(sdpcmd->ds_check_timer, sdpcmd->ds_check_interval, FALSE);
	}
}

static void
sdpcmd_ds_hwait_timerfn(dngl_timer_t *t)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *) hnd_timer_get_ctx(t);

	ASSERT(sdpcmd->ds_hwait);
	if (!sdpcmd->ds_hwait)
		return;

	if (sdpcmd->ds_disable_state) {
		DS_INFORM(("HWAIT while DISABLED?\n"));
		return;
	}


	sdpcmd->ds_hwait_cnt++;

	/* If no bus stuff needs clocks... allow deepsleep and stop polling */
	if (sdpcmd_can_goto_ds(sdpcmd)) {
		DS_INFORM(("HWAIT: DSon in %s after %d waits\n",
		           sdpcmd_ds_state_name(sdpcmd->ds_state), sdpcmd->ds_hwait_cnt));
		DS_LOG(sdpcmd, SDPCM_DS_LOG_TYPE_TMO_DS,
			sdpcmd->ds_state,
			sdpcmd_read_device_wake_gpio(sdpcmd),
			sdpcmd->ds_hwait_cnt);
		sdpcmd_forcewake(sdpcmd, FALSE);
		sdpcmd->ds_hwait = FALSE;
	} else {
		/* Otherwise, clocks stay on and we keep polling... */
		dngl_add_timer(sdpcmd->ds_hwait_timer, 1, FALSE);
		DS_LOG(sdpcmd, SDPCM_DS_LOG_TYPE_TMO_RETRY,
				sdpcmd->ds_state,
				sdpcmd_read_device_wake_gpio(sdpcmd),
				sdpcmd->ds_hwait_cnt);
	}
}

static void
sdpcmd_no_ds_dw_deassrt(void *handle)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;

	DS_TRACE(("%s: check sleep, delay %d\n", __FUNCTION__, sdpcmd->ds_always_delay));

	/* First try to enter deepsleep. If cannot, start a timer to retry. */
	if (sdpcmd_can_goto_ds(sdpcmd) && !sdpcmd->ds_always_delay) {
		sdpcmd_ds_enter_req(sdpcmd);
	} else {
		/* Start a timer to check for no pending DMA to host */
		sdpcmd_ds_check_periodic(sdpcmd);
	}
}

static void
sdpcmd_no_ds_d3_enter(void *handle)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;

	DS_TRACE(("%s\n", __FUNCTION__));

	/* From one no_ds to another... no action required? */
	BCM_REFERENCE(sdpcmd);
}

static void
sdpcmd_ds_check_dw_assrt(void *handle)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;

	DS_TRACE(("%s\n", __FUNCTION__));

	/* Disable deepsleep check timer */
	if (sdpcmd->ds_check_timer_on) {
		dngl_del_timer(sdpcmd->ds_check_timer);
		sdpcmd->ds_check_timer_on = FALSE;
	} else {
		sdpcmd_ds_exit_notify(sdpcmd);
	}
}

static void
sdpcmd_ds_check_d3_enter(void *handle)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;

	DS_TRACE(("%s\n", __FUNCTION__));

	/* Disable deepsleep check timer */
	if (sdpcmd->ds_check_timer_on) {
		dngl_del_timer(sdpcmd->ds_check_timer);
		sdpcmd->ds_check_timer_on = FALSE;
	}

	/* Allow chip to go to deepsleep */
	sdpcmd_ds_disable_deepsleep(sdpcmd, FALSE);
}

static void
sdpcmd_ds_check_ds_allowed(void *handle)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;

	DS_TRACE(("%s\n", __FUNCTION__));

	ASSERT(!sdpcmd->ds_check_timer_on);
	if (sdpcmd->ds_check_timer_on) {
		dngl_del_timer(sdpcmd->ds_check_timer);
		sdpcmd->ds_check_timer_on = FALSE;
		++sdpcmd->dc_tmron_cnt;
	}

	/* Allow chip to go to deepsleep */
	sdpcmd_ds_disable_deepsleep(sdpcmd, FALSE);

	++sdpcmd->metrics.ds_d0_cnt;
	sdpcmd->metric_ref.ds_d0 = OSL_SYSUPTIME();
}

static void
sdpcmd_ds_d0_int_dtoh(void *handle)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;

	DS_TRACE(("%s\n", __FUNCTION__));

	/* Exit notification will disallow deepsleep; must return to check state */
	sdpcmd_ds_exit_notify(sdpcmd);
	sdpcmd_ds_check_periodic(sdpcmd);

	if (sdpcmd->metric_ref.ds_d0) {
		sdpcmd->metrics.ds_d0_dur += OSL_SYSUPTIME() - sdpcmd->metric_ref.ds_d0;
		sdpcmd->metric_ref.ds_d0 = 0;
	}
}

static void
sdpcmd_ds_d0_dw_assrt(void *handle)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;

	DS_TRACE(("%s\n", __FUNCTION__));

	sdpcmd_ds_exit_notify(sdpcmd);

	if (sdpcmd->metric_ref.ds_d0) {
		sdpcmd->metrics.ds_d0_dur += OSL_SYSUPTIME() - sdpcmd->metric_ref.ds_d0;
		sdpcmd->metric_ref.ds_d0 = 0;
	}
}

static void
sdpcmd_ds_nods_d3_dw_assrt(void *handle)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;

	DS_TRACE(("%s\n", __FUNCTION__));

	++sdpcmd->nodsd3dw_cnt;

	/* Disallow deepsleep, send a d3 exit */
	sdpcmd_d3_exit_notify(sdpcmd);
}

static void
sdpcmd_ds_nods_d3_dw_dassrt(void *handle)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;

	DS_TRACE(("%s\n", __FUNCTION__));

	/* In d3, DW low means deepsleep allowed */
	sdpcmd_ds_disable_deepsleep(sdpcmd, FALSE);
	++sdpcmd->metrics.ds_d3_cnt;
	sdpcmd->metric_ref.ds_d3 = OSL_SYSUPTIME();
}

static void
sdpcmd_ds_nods_d3_int_dtoh(void *handle)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;

	DS_TRACE(("%s\n", __FUNCTION__));

	/* Disallow deepsleep, send a d3 exit */
	sdpcmd_d3_exit_notify(sdpcmd);
}

static void
sdpcmd_ds_ds_d3_int_dtoh(void *handle)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;

	DS_TRACE(("%s\n", __FUNCTION__));

	/* Disallow deepsleep, send a d3 exit */
	sdpcmd_d3_exit_notify(sdpcmd);

	if (sdpcmd->metric_ref.ds_d3) {
		sdpcmd->metrics.ds_d3_dur += OSL_SYSUPTIME() - sdpcmd->metric_ref.ds_d3;
		sdpcmd->metric_ref.ds_d3 = 0;
	}
}

static void
sdpcmd_ds_ds_d3_dw_assrt(void *handle)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;

	DS_TRACE(("%s\n", __FUNCTION__));

	/* Disallow deepsleep, send a d3 exit */
	sdpcmd_d3_exit_notify(sdpcmd);

	if (sdpcmd->metric_ref.ds_d3) {
		sdpcmd->metrics.ds_d3_dur += OSL_SYSUPTIME() - sdpcmd->metric_ref.ds_d3;
		sdpcmd->metric_ref.ds_d3 = 0;
	}
}
#endif /* DS_PROT */

#ifdef BCMULP
static uint
sdpcmd_ulp_get_retention_size_cb(void *handle, ulp_ext_info_t *einfo)
{
	ULP_DBG(("%s: sz: %d\n", __FUNCTION__, sizeof(sdpcmd_ulp_cr_dat_t)));
	return sizeof(sdpcmd_ulp_cr_dat_t);
}

static int
sdpcmd_enter_pre_ulpucode_cb(void *handle, ulp_ext_info_t *einfo, uint8 *cache_data)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;
	int err = BCME_OK;
	sdpcmd_ulp_cr_dat_t crinfo = {0};

	crinfo.txglom = sdpcmd->tunables[TXGLOM];
	ULP_DBG(("%s: TXGLOM: %x\n", __FUNCTION__, crinfo.txglom));
	memcpy(cache_data, (void*)&crinfo, sizeof(crinfo));

#ifdef BCMFCBS
	if ((err = fcbsdata_sdio_populate(sdpcmd->sih)) != BCME_OK) {
		ULP_ERR(("%s: Failed to create SDIO FCBS sequence!", __func__));
	}
#endif /* BCMFCBS */
	return err;
}

static int
sdpcmd_enter_post_ulpucode_cb(void *handle, ulp_ext_info_t *einfo)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;
	int err = BCME_OK;
	int reg_data = 0;

	/* Wait for DMA completion */
	SPINWAIT(dma_txpending(sdpcmd->di), 2000000);
	if (dma_txpending(sdpcmd->di)) {
		ULP_ERR(("%s: ERROR: DMA TX pending", __func__));
	}

	/* Wait for KSO bit to be cleared */
	SPINWAIT((sdpcmd_sdioaccess(&reg_data, SDA_F1_REG_SPACE, SDA_SLEEPCSR,
			SDA_READ, sdpcmd->osh, sdpcmd->regs),
			(reg_data & SDA_SLEEPCSR_KEEP_SDIO_ON)), 2000000);

	if (reg_data & SDA_SLEEPCSR_KEEP_SDIO_ON) {
		ULP_ERR(("%s: ERROR: SDIO KSO still ON", __func__));
	}

	return err;
}

static int
sdpcmd_ulp_exit_cb(void *handle, uint8 *cache_data, uint8 *p2_cache_data)
{
	struct dngl_bus *sdpcmd = (struct dngl_bus *)handle;
	sdpcmd_ulp_cr_dat_t *crinfo = (sdpcmd_ulp_cr_dat_t *)cache_data;

	sdpcmd->tunables[TXGLOM] = crinfo->txglom;
	ULP_DBG(("%s: TXGLOM: %x, cache_data: %p\n", __FUNCTION__,
		sdpcmd->tunables[TXGLOM], cache_data));
	return BCME_OK;
}

#endif /* BCMULP */
