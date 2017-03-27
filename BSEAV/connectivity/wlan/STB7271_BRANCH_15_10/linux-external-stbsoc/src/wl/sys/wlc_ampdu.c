/*
 * A-MPDU Tx (with extended Block Ack protocol) source file
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
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

/**
 * Preprocessor defines used in this file:
 *
 * WLAMPDU:       firmware/driver image supports AMPDU functionality
 * WLAMPDU_MAC:   aggregation by d11 core (being ucode, ucode hw assisted or AQM)
 * WLAMPDU_UCODE: aggregation by ucode
 * WLAMPDU_HW:    hardware assisted aggregation by ucode
 * WLAMPDU_AQM:   aggregation by AQM module in d11 core (AC chips only)
 * WLAMPDU_PRECEDENCE: transmit certain traffic earlier than other traffic
 * PSPRETEND:     increase robustness against bad signal conditions by performing resends later
 */


#include <wlc_cfg.h>

#ifndef WLAMPDU
#error "WLAMPDU is not defined"
#endif	/* WLAMPDU */
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#ifdef WLOVERTHRUSTER        /* Opportunistic Tcp Ack Consolidation */
#include <proto/ethernet.h>
#endif
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <bcmdevs.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_phy_hal.h>
#include <wlc_antsel.h>
#include <wlc_scb.h>         /* one SCB represents one remote party */
#include <wlc_frmutil.h>
#include <wlc_p2p.h>
#include <wlc_apps.h>
#ifdef WLTOEHW           /* TCP segmentation / checksum hardware assisted offload. AC \
	chips only. */
#include <wlc_tso.h>
#endif
#include <wlc_ampdu.h>
#include <wlc_ampdu_cmn.h>
#if defined(WLAMSDU) || defined(WLAMSDU_TX)
#include <wlc_amsdu.h>
#endif
#if defined(EVENT_LOG_COMPILE)
#include <event_log.h>
#if defined(ECOUNTERS)
#include <ecounters.h>
#endif
#endif /* EVENT_LOG_COMPILE */
#include <wlc_scb_ratesel.h>
#include <wl_export.h>
#include <wlc_rm.h>
#if defined(BCMCCX) && defined(CCX_SDK)
#include <wlc_ccx.h>
#endif /* BCMCCX && CCX_SDK */

#ifdef PROP_TXSTATUS         /* saves dongle memory by queuing tx packets on the host \
	*/
#include <wlc_wlfc.h>
#endif
#ifdef WLP2P
#include <wlc_p2p.h>
#endif
#include <wlc_pcb.h>
#ifdef WL_MU_TX
#include <wlc_mutx.h>
#endif
#if defined(WLATF) || defined(PKTQ_LOG)                 /* Air Time Fairness */
#include <wlc_airtime.h>
#include <wlc_prot.h>
#include <wlc_prot_g.h>
#include <wlc_prot_n.h>
#endif /* WLATF */
#if defined(TXQ_MUX)
#include <wlc_txtime.h>
#endif /* TXQ_MUX */
#ifdef WLMESH
#include <wlc_mesh.h>
#endif
#include <wlc_btcx.h>
#include <wlc_txc.h>
#include <wlc_objregistry.h>
#ifdef WL11AC
#include <wlc_vht.h>
#endif /* WL11AC */
#include <wlc_ht.h>
#if defined(SCB_BS_DATA)     /* band steering */
#include <wlc_bs_data.h>
#endif /* SCB_BS_DATA */
#ifdef WLC_SW_DIVERSITY
#include <wlc_swdiv.h>
#endif /* WLC_SW_DIVERSITY */

#ifdef BCM_SFD
#include <wlc_sfd.h>
#endif

#ifdef WL11K
#include <wlc_rrm.h>
#endif

/* Enable NON AQM code only when builds require ucode/Hw */
/* Dongle 11ac builds will define WLAMPDU_AQM */
#ifndef WLAMPDU_AQM
#define AMPDU_NON_AQM
#endif
#include <wlc_tx.h>
#include <wlc_bsscfg_psq.h>
#include <wlc_txmod.h>
#ifdef TRAFFIC_MGMT
#include <wlc_traffic_mgmt.h>
#endif
#include <wlc_pspretend.h>
#include <wlc_csrestrict.h>

#if defined(BCMDBG) /* temporary code to catch rare phenomenon using UTF */
/* UTF: 43602 NIC assertion "WLPKTTAG(p)->flags & WLF_AMPDU_MPDU" failed */
#define BCMDBG_SWWLAN_38455
#endif /* BCMDBG */
#include <wlc_macdbg.h>

#include <wlc_bmac.h>
#include <wlc_hw.h>
#include <wlc_rspec.h>
#include <wlc_txs.h>
#include <wlc_qoscfg.h>
#include <wlc_perf_utils.h>
#include <wlc_dbg.h>
#include <wlc_pktc.h>
#include <wlc_dump.h>
#include <wlc_monitor.h>
#include <wlc_stf.h>
#include <wlc_assoc.h>
#include <wlc_bmac.h>
#include <phy_api.h>

#ifdef WLAMPDU_PRECEDENCE
#define ampdu_pktqprec_n_pkts(pq, tid) \
	(pktqprec_n_pkts(pq, WLC_PRIO_TO_HI_PREC(tid))\
	+ pktqprec_n_pkts(pq, WLC_PRIO_TO_PREC(tid)))
static bool wlc_ampdu_prec_enq(wlc_info_t *wlc, struct pktq *pq, void *pkt, int tid);
static void * wlc_ampdu_pktq_pdeq(struct pktq *pq, int tid);
static void wlc_ampdu_pktq_pflush(wlc_info_t *wlc, struct pktq *pq, int tid);
static void *ampdu_pktqprec_peek(struct pktq *pq, int tid);
#define wlc_ampdu_pktqlog_cnt(q, tid, prec)	(q)->pktqlog->_prec_cnt[(prec)]
#else
#define wlc_ampdu_prec_enq(wlc, q, pkt, prec)	wlc_prec_enq_head(wlc, q, pkt, prec, FALSE)
#define ampdu_pktqprec_n_pkts			pktqprec_n_pkts
#define wlc_ampdu_pktq_pdeq			pktq_pdeq
#define wlc_ampdu_pktq_pflush(wlc, pq, prec)	wlc_txq_pktq_pflush(wlc, pq, prec)
#define wlc_ampdu_pktqlog_cnt(q, tid, prec)	(q)->pktqlog->_prec_cnt[(tid)]
#define ampdu_pktqprec_peek			pktqprec_peek
#endif /* WLAMPDU_PRECEDENCE */

#if defined(WLAMPDU) && defined(BCMDBG)
extern void wlc_ampdu_txq_prof_enab(void);
extern void wlc_ampdu_txq_prof_print_histogram(int entries);
extern void wlc_ampdu_txq_prof_add_entry(wlc_info_t *wlc, struct scb *scb,
	const char * func, uint32 line);
#define WLC_AMPDU_TXQ_PROF_ADD_ENTRY(wlc, scb) \
	wlc_ampdu_txq_prof_add_entry(wlc, scb, __FUNCTION__, __LINE__);
#else
#define WLC_AMPDU_TXQ_PROF_ADD_ENTRY(wlc, scb)
#endif /* WLAMPDU && BCMDBG */

#define WLAMPDU_PREC_TID2PREC(p, tid) \
	((WLPKTTAG(p)->flags3 & WLF3_FAVORED) ? WLC_PRIO_TO_HI_PREC(tid) : WLC_PRIO_TO_PREC(tid))

#ifdef WLAMPDU_MAC
static void BCMFASTPATH
wlc_ampdu_dotxstatus_aqm_complete(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
                                  void *p, tx_status_t *txs, wlc_txh_info_t *txh_info);
#endif /* WLAMPDU_MAC */
#if defined(WL_DATAPATH_LOG_DUMP)
static void wlc_ampdu_datapath_summary(void *ctx, struct scb *scb, int tag);
#endif

#ifdef IQPLAY_DEBUG
static void wlc_set_sequmber(wlc_info_t *wlc, uint16 seq_num);
#endif /* IQPLAY_DEBUG */

#ifdef BCMDBG
static int	_txq_prof_enab;
#endif

/* ATF and TXQ_MUX uses this setting */
#define AMPDU_TXQ_TIME_ALLOWANCE_US	4000

#ifdef WLATF
/* Default AMPDU txq time allowance. */

#define AMPDU_TXQ_TIME_MIN_ALLOWANCE_US	1000

/* Note: Structure members are referred directly to reduce the overhead as these macros are used
 * in the per-packet part of the datapath
 */

#define AMPDU_ATF_STATE(ini) (&(ini)->atf_state)
#define AMPDU_ATF_ENABLED(ini) ((ini)->atf_state.atf != 0)
#ifdef WLCNT
#define AMPDU_ATF_STATS(ini) (&(ini)->atf_state.atf_stats)
#else
#define AMPDU_ATF_STATS(ini) 0 /* This will force a kernel panic if WLCNT has not been defined */
#endif /* WLCNT */
#endif /* WLATF */

/** iovar table */
enum wlc_ampdu_iov {
	IOV_AMPDU_TX = 1,		/* enable/disable ampdu tx */
	IOV_AMPDU_TID = 2,		/* enable/disable per-tid ampdu */
	IOV_AMPDU_TX_DENSITY = 3,	/* ampdu density */
	IOV_AMPDU_SEND_ADDBA = 4,	/* send addba req to specified scb and tid */
	IOV_AMPDU_SEND_DELBA = 5,	/* send delba req to specified scb and tid */
	IOV_AMPDU_MANUAL_MODE = 6,	/* addba tx to be driven by cmd line */
	IOV_AMPDU_NO_BAR = 7,		/* do not send bars */
	IOV_AMPDU_MPDU = 8,		/* max number of mpdus in an ampdu */
	IOV_AMPDU_DUR = 9,		/* max duration of an ampdu (in usec) */
	IOV_AMPDU_RTS = 10,		/* use rts for ampdu */
	IOV_AMPDU_TX_BA_WSIZE = 11,	/* ampdu TX ba window size */
	IOV_AMPDU_RETRY_LIMIT = 12,	/* ampdu retry limit */
	IOV_AMPDU_RR_RETRY_LIMIT = 13,	/* regular rate ampdu retry limit */
	IOV_AMPDU_TX_LOWAT = 14,	/* ampdu tx low wm (watermark) */
	IOV_AMPDU_HIAGG_MODE = 15,	/* agg mpdus with diff retry cnt */
	IOV_AMPDU_PROBE_MPDU = 16,	/* number of mpdus/ampdu for rate probe frames */
	IOV_AMPDU_TXPKT_WEIGHT = 17,	/* weight of ampdu in the txfifo; helps rate lag */
	IOV_AMPDU_FFPLD_RSVD = 18,	/* bytes to reserve in pre-loading info */
	IOV_AMPDU_FFPLD = 19,		/* to display pre-loading info */
	IOV_AMPDU_MAX_TXFUNFL = 20,	/* inverse of acceptable proportion of tx fifo underflows */
	IOV_AMPDU_RETRY_LIMIT_TID = 21,	/* ampdu retry limit per-tid */
	IOV_AMPDU_RR_RETRY_LIMIT_TID = 22,	/* regular rate ampdu retry limit per-tid */
	IOV_AMPDU_MFBR = 23,		/* Use multiple fallback rate */
	IOV_AMPDU_TCP_ACK_RATIO = 24, 	/* max number of ack to suppress in a row. 0 disable */
	IOV_AMPDU_TCP_ACK_RATIO_DEPTH = 25,	/* max number of multi-ack. 0 disable */
	IOV_AMPDUMAC = 26,		/* Use ucode or hw assisted agregation */
	IOV_AMPDU_AGGMODE = 27,		/* agregation mode: HOST or MAC */
	IOV_AMPDU_AGGFIFO = 28,		/* aggfifo depth */
	IOV_AMPDU_TXFAIL_EVENT = 29,	/* Tx failure event to the host */
	IOV_AMPDU_FRAMEBURST_OVR = 30,	/* Override framebursting in absense of ampdu */
	IOV_AMPDU_TXQ_PROFILING_START = 31,	/* start sampling TXQ profiling data */
	IOV_AMPDU_TXQ_PROFILING_DUMP = 32,	/* dump TXQ histogram */
	IOV_AMPDU_TXQ_PROFILING_SNAPSHOT = 33,	/* take a snapshot of TXQ histogram */
	IOV_AMPDU_RELEASE = 34,		/* max # of mpdus released at a time */
	IOV_AMPDU_ATF_US = 35,
	IOV_AMPDU_ATF_MIN_US = 36,
	IOV_AMPDU_CS_PKTRETRY = 37,
	IOV_AMPDU_TXAGGR = 38,
	IOV_AMPDU_ADDBA_TIMEOUT = 39,
	IOV_AMPDU_SET_SEQNUMBER = 40,
	IOV_AMPDU_LAST
};

static const bcm_iovar_t ampdu_iovars[] = {
	{"ampdu_tx", IOV_AMPDU_TX, (IOVF_SET_DOWN|IOVF_RSDB_SET), 0, IOVT_BOOL, 0},
	{"ampdu_tid", IOV_AMPDU_TID, (0), 0, IOVT_BUFFER, sizeof(struct ampdu_tid_control)},
	{"ampdu_tx_density", IOV_AMPDU_TX_DENSITY, (0), 0, IOVT_UINT8, 0},
	{"ampdu_send_addba", IOV_AMPDU_SEND_ADDBA, (0), 0, IOVT_BUFFER,
	sizeof(struct ampdu_ea_tid)},
	{"ampdu_send_delba", IOV_AMPDU_SEND_DELBA, (0), 0, IOVT_BUFFER,
	sizeof(struct ampdu_ea_tid)},
	{"ampdu_manual_mode", IOV_AMPDU_MANUAL_MODE, (IOVF_SET_DOWN), 0, IOVT_BOOL, 0},
	{"ampdu_mpdu", IOV_AMPDU_MPDU, (0), 0, IOVT_INT8, 0}, /* need to mark IOVF2_RSDB */
#ifdef WLOVERTHRUSTER
	{"ack_ratio", IOV_AMPDU_TCP_ACK_RATIO, (0), 0, IOVT_UINT8, 0},
	{"ack_ratio_depth", IOV_AMPDU_TCP_ACK_RATIO_DEPTH, (0), 0, IOVT_UINT8, 0},
#endif /* WLOVERTHRUSTER */
#ifdef BCMDBG
	{"ampdu_aggfifo", IOV_AMPDU_AGGFIFO, 0, 0, IOVT_UINT8, 0},
	{"ampdu_ffpld", IOV_AMPDU_FFPLD, (0), 0, IOVT_UINT32, 0},
#endif

	{"ampdumac", IOV_AMPDUMAC, (0), 0, IOVT_UINT8, 0},
#if defined(WL_EXPORT_AMPDU_RETRY)
	{"ampdu_retry_limit_tid", IOV_AMPDU_RETRY_LIMIT_TID, (0), 0, IOVT_BUFFER,
	sizeof(struct ampdu_retry_tid)},
	{"ampdu_rr_retry_limit_tid", IOV_AMPDU_RR_RETRY_LIMIT_TID, (0), 0, IOVT_BUFFER,
	sizeof(struct ampdu_retry_tid)},
#endif 
#ifdef WLMEDIA_TXFAILEVENT
	{"ampdu_txfail_event", IOV_AMPDU_TXFAIL_EVENT, (0), 0, IOVT_BOOL, 0},
#endif /* WLMEDIA_TXFAILEVENT */
	{"ampdu_aggmode", IOV_AMPDU_AGGMODE, (IOVF_SET_DOWN|IOVF_RSDB_SET), 0, IOVT_INT8, 0},
	{"frameburst_override", IOV_AMPDU_FRAMEBURST_OVR, (0), 0, IOVT_BOOL, 0},
	{"ampdu_txq_prof_start", IOV_AMPDU_TXQ_PROFILING_START, (0), 0, IOVT_VOID, 0},
	{"ampdu_txq_prof_dump", IOV_AMPDU_TXQ_PROFILING_DUMP, (0), 0, IOVT_VOID, 0},
	{"ampdu_txq_ss", IOV_AMPDU_TXQ_PROFILING_SNAPSHOT, (0), 0, IOVT_VOID, 0},
	{"ampdu_release", IOV_AMPDU_RELEASE, (0), 0, IOVT_UINT8, 0},
#ifdef WLATF
	{"ampdu_atf_us", IOV_AMPDU_ATF_US, (IOVF_NTRL), 0, IOVT_UINT32, 0},
	{"ampdu_atf_min_us", IOV_AMPDU_ATF_MIN_US, (IOVF_NTRL), 0, IOVT_UINT32, 0},
#endif
	{"ampdu_txaggr", IOV_AMPDU_TXAGGR, IOVF_BSS_SET_DOWN, 0, IOVT_BUFFER,
	sizeof(struct ampdu_aggr)},
	{"ampdu_addba_to", IOV_AMPDU_ADDBA_TIMEOUT, (0), 0, IOVT_UINT32, 0},
#ifdef IQPLAY_DEBUG
	{"set_seqnumber", IOV_AMPDU_SET_SEQNUMBER,
	(IOVF_OPEN_ALLOW), 0, IOVT_BUFFER, sizeof(uint32),
	},
#endif /* IQPLAY_DEBUG */
	{NULL, 0, 0, 0, 0, 0}
};

#define AMPDU_DEF_PROBE_MPDU	2		/**< def number of mpdus in a rate probe ampdu */

#ifndef AMPDU_TX_BA_MAX_WSIZE
#define AMPDU_TX_BA_MAX_WSIZE	64		/**< max Tx ba window size (in pdu) */
#endif /* AMPDU_TX_BA_MAX_WSIZE */
#ifndef AMPDU_TX_BA_DEF_WSIZE
#define AMPDU_TX_BA_DEF_WSIZE	64		/**< default Tx ba window size (in pdu) */
#endif /* AMPDU_TX_BA_DEF_WSIZE */

#define	AMPDU_BA_BITS_IN_BITMAP	64		/**< number of bits in bitmap */
#define	AMPDU_MAX_DUR		5416		/**< max dur of tx ampdu (in usec) */
#define AMPDU_MAX_RETRY_LIMIT	32		/**< max tx retry limit */
#define AMPDU_DEF_RETRY_LIMIT	5		/**< default tx retry limit */
#define AMPDU_DEF_RR_RETRY_LIMIT	2	/**< default tx retry limit at reg rate */
#define AMPDU_DEF_TX_LOWAT	1		/**< default low transmit wm (water mark) */
#define AMPDU_DEF_TXPKT_WEIGHT	2		/**< default weight of ampdu in txfifo */
#define AMPDU_DEF_FFPLD_RSVD	2048		/**< default ffpld reserved bytes */
#define AMPDU_MIN_FFPLD_RSVD	512		/**< minimum ffpld reserved bytes */
#define AMPDU_BAR_RETRY_CNT	50		/**< # of bar retries before delba */
#define AMPDU_ADDBA_REQ_RETRY_CNT	4	/**< # of addbareq retries before delba */
#define AMPDU_INI_OFF_TIMEOUT	60		/**< # of sec in off state */
#define	AMPDU_SCB_MAX_RELEASE	32		/**< max # of mpdus released at a time */
#ifndef AMPDU_SCB_MAX_RELEASE_AQM
#define	AMPDU_SCB_MAX_RELEASE_AQM	64	/**< max # of mpdus released at a time */
#endif /* AMPDU_SCB_MAX_RELEASE_AQM */
#define AMPDU_ADDBA_REQ_RETRY_TIMEOUT	200	/* ADDBA retry timeout msecs */

#define AMPDU_INI_DEAD_TIMEOUT		2	/**< # of sec without ini progress */
#define AMPDU_INI_CLEANUP_TIMEOUT	2	/**< # of sec in pending off state */
#define AMPDU_TXNOPROG_LIMIT		10	/**< max # of sec with any ini in pending off state
						 **< while ucode is consuming no pkts before
						 **< we dump more debugging messages
						 */

/* internal BA states */
#define	AMPDU_TID_STATE_BA_OFF		0x00	/**< block ack OFF for tid */
#define	AMPDU_TID_STATE_BA_ON		0x01	/**< block ack ON for tid */
#define	AMPDU_TID_STATE_BA_PENDING_ON	0x02	/**< block ack pending ON for tid */
#define	AMPDU_TID_STATE_BA_PENDING_OFF	0x03	/**< block ack pending OFF for tid */

#define NUM_FFPLD_FIFO 4                        /**< number of fifo concerned by pre-loading */
#define FFPLD_TX_MAX_UNFL   200                 /**< default value of the average number of ampdu
						 * without underflows
						 */
#define FFPLD_MPDU_SIZE 1800                    /**< estimate of maximum mpdu size */
#define FFPLD_PLD_INCR 1000                     /**< increments in bytes */
#define FFPLD_MAX_AMPDU_CNT 5000                /**< maximum number of ampdu we
						 * accumulate between resets.
						 */
/* retry BAR in watchdog reason codes */
#define AMPDU_R_BAR_DISABLED		0	/**< disabled retry BAR in watchdog */
#define AMPDU_R_BAR_NO_BUFFER		1	/**< retry BAR due to out of buffer */
#define AMPDU_R_BAR_CALLBACK_FAIL	2	/**< retry BAR due to callback register fail */
#define AMPDU_R_BAR_HALF_RETRY_CNT	3	/**< retry BAR due to reach to half
						 * AMPDU_BAR_RETRY_CNT
						 */
#define AMPDU_R_BAR_BLOCKED		4	/**< retry BAR due to blocked data fifo */

#define AMPDU_DEF_TCP_ACK_RATIO		2	/**< default TCP ACK RATIO */
#define AMPDU_DEF_TCP_ACK_RATIO_DEPTH	0	/**< default TCP ACK RATIO DEPTH */

#ifndef AMPDU_SCB_DRAIN_CNT
#define AMPDU_SCB_DRAIN_CNT	8
#endif

/* maximum number of frames can be released to HW (in-transit limit) */
#define AMPDU_AQM_RELEASE_MAX          256	/**< for BE */
#define AMPDU_AQM_RELEASE_DEFAULT       32

#define	BTCX_AMPDU_MAX_DUR		2500	/* max dur of tx ampdu with coex
						 * profile (in usec)
						 */

/**
 * Helper to determine d11 seq number wrapping. Returns FALSE if 'a' has wrapped
 * but 'b' not yet. Note that the assumption is that 'a' and 'b' are never more
 * than (SEQNUM_MAX / 2) sequences apart.
 */
#define IS_SEQ_ADVANCED(a, b) \
	(MODSUB_POW2((a), (b), SEQNUM_MAX) < SEQNUM_MAX / 2)

int wl_ampdu_drain_cnt = AMPDU_SCB_DRAIN_CNT;

/* useful macros */
#define NEXT_SEQ(seq) MODINC_POW2((seq), SEQNUM_MAX)
#define NEXT_TX_INDEX(index) MODINC_POW2((index), (ampdu_tx->config->ba_max_tx_wsize))
#define PREV_TX_INDEX(index) MODDEC_POW2((index), (ampdu_tx->config->ba_max_tx_wsize))
#define TX_SEQ_TO_INDEX(seq) ((seq) & ((ampdu_tx->config->ba_max_tx_wsize) - 1))

/* max possible overhead per mpdu in the ampdu; 3 is for roundup if needed */
#define AMPDU_MAX_MPDU_OVERHEAD (DOT11_FCS_LEN + DOT11_ICV_AES_LEN + AMPDU_DELIMITER_LEN + 3 \
	+ DOT11_A4_HDR_LEN + DOT11_QOS_LEN + DOT11_IV_MAX_LEN)

/** ampdu related transmit stats */
typedef struct wlc_ampdu_cnt {
	/* initiator stat counters */
	uint32 txampdu;		/**< ampdus sent */
#ifdef WLCNT
	uint32 txmpdu;		/**< mpdus sent */
	uint32 txregmpdu;	/**< regular(non-ampdu) mpdus sent */
	union {
		uint32 txs;		/**< MAC agg: txstatus received */
		uint32 txretry_mpdu;	/**< retransmitted mpdus */
	} u0;
	uint32 txretry_ampdu;	/**< retransmitted ampdus */
	uint32 txfifofull;	/**< release ampdu due to insufficient tx descriptors */
	uint32 txfbr_mpdu;	/**< retransmitted mpdus at fallback rate */
	uint32 txfbr_ampdu;	/**< retransmitted ampdus at fallback rate */
	union {
		uint32 txampdu_sgi;	/**< ampdus sent with sgi */
		uint32 txmpdu_sgi;	/**< ucode agg: mpdus sent with sgi */
	} u1;
	union {
		uint32 txampdu_stbc;	/**< ampdus sent with stbc */
		uint32 txmpdu_stbc;	/**< ucode agg: mpdus sent with stbc */
	} u2;
	uint32 txampdu_mfbr_stbc; /**< ampdus sent at mfbr with stbc */
	uint32 txrel_wm;	/**< mpdus released due to lookahead wm (water mark) */
	uint32 txrel_size;	/**< mpdus released due to max ampdu size (in mpdu's) */
	uint32 sduretry;	/**< sdus retry returned by sendsdu() */
	uint32 sdurejected;	/**< sdus rejected by sendsdu() */
	uint32 txdrop;		/**< dropped packets */
	uint32 txr0hole;	/**< lost packet between scb and sendampdu */
	uint32 txrnhole;	/**< lost retried pkt */
	uint32 txrlag;		/**< laggard pkt (was considered lost) */
	uint32 txreg_noack;	/**< no ack for regular(non-ampdu) mpdus sent */
	uint32 txaddbareq;	/**< addba req sent */
	uint32 rxaddbaresp;	/**< addba resp recd */
	uint32 txlost;		/**< lost packets reported in txs */
	uint32 txbar;		/**< bar sent */
	uint32 rxba;		/**< ba recd */
	uint32 noba;            /**< ba missing */
	uint32 txstuck;		/**< watchdog bailout for stuck state */
	uint32 orphan;		/**< orphan pkts where scb/ini has been cleaned */

#ifdef WLAMPDU_MAC
	uint32 epochdelta;	/**< How many times epoch has changed */
	uint32 echgr1;          /**< epoch change reason -- plcp */
	uint32 echgr2;          /**< epoch change reason -- rate_probe */
	uint32 echgr3;          /**< epoch change reason -- a-mpdu as regmpdu */
	uint32 echgr4;          /**< epoch change reason -- regmpdu */
	uint32 echgr5;          /**< epoch change reason -- dest/tid */
	uint32 echgr6;          /**< epoch change reason -- seq no */
	uint32 tx_mrt, tx_fbr;  /**< number of MPDU tx at main/fallback rates */
	uint32 txsucc_mrt;      /**< number of successful MPDU tx at main rate */
	uint32 txsucc_fbr;      /**< number of successful MPDU tx at fallback rate */
	uint32 enq;             /**< totally enqueued into aggfifo */
	uint32 cons;            /**< totally reported in txstatus */
	uint32 pending;         /**< number of entries currently in aggfifo or txsq */
#endif /* WLAMPDU_MAC */

	/* general: both initiator and responder */
	uint32 rxunexp;		/**< unexpected packets */
	uint32 txdelba;		/**< delba sent */
	uint32 rxdelba;		/**< delba recd */

#ifdef WLPKTDLYSTAT
	/* PER (per mcs) statistics */
	uint32 txmpdu_cnt[AMPDU_HT_MCS_ARRAY_SIZE];		/**< MPDUs per mcs */
	uint32 txmpdu_succ_cnt[AMPDU_HT_MCS_ARRAY_SIZE];	/**< acked MPDUs per MCS */
#ifdef WL11AC
	uint32 txmpdu_vht_cnt[AMPDU_MAX_VHT];	/**< MPDUs per vht */
	uint32 txmpdu_vht_succ_cnt[AMPDU_MAX_VHT]; /**< acked MPDUs per vht */
#endif /* WL11AC */
#endif /* WLPKTDLYSTAT */

#ifdef WL_CS_PKTRETRY
	uint32 cs_pktretry_cnt;
#endif
#endif /* WLCNT */
	uint32 ampdu_wds;       /**< AMPDU watchdogs */
	uint32 txampdubyte_h;   /* tx ampdu data bytes */
	uint32 txampdubyte_l;
} wlc_ampdu_tx_cnt_t;

#ifdef AMPDU_NON_AQM
/**
 * structure to hold tx fifo information and pre-loading state counters specific to tx underflows of
 * ampdus. Some counters might be redundant with the ones in wlc or ampdu structures.
 * This allows to maintain a specific state independently of how often and/or when the wlc counters
 * are updated.
 */
typedef struct wlc_fifo_info {
	uint16 ampdu_pld_size;	/**< number of bytes to be pre-loaded */
	uint8 mcs2ampdu_table[AMPDU_HT_MCS_ARRAY_SIZE]; /**< per-mcs max # of mpdus in an ampdu */
	uint16 prev_txfunfl;	/**< num of underflows last read from the HW macstats counter */
	uint32 accum_txfunfl;	/**< num of underflows since we modified pld params */
	uint32 accum_txampdu;	/**< num of tx ampdu since we modified pld params  */
	uint32 prev_txampdu;	/**< previous reading of tx ampdu */
	uint32 dmaxferrate;	/**< estimated dma avg xfer rate in kbits/sec */
} wlc_fifo_info_t;
#endif /* AMPDU_NON_AQM */

#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */

/** frame type enum for epoch change */
enum {
	AMPDU_NONE,
	AMPDU_11N,
	AMPDU_11VHT,
	AMPDU_11HE
};

typedef struct ampdumac_info {
#ifdef WLAMPDU_UCODE
	uint16 txfs_addr_strt;
	uint16 txfs_addr_end;
	uint16 txfs_wptr;
	uint16 txfs_rptr;
	uint16 txfs_wptr_addr;
	uint8  txfs_wsize;
#endif
	uint8 epoch;
	bool change_epoch;
	/* any change of following elements will change epoch */
	struct scb *prev_scb;
	uint8 prev_tid;
	uint8 prev_ft;		/**< eg AMPDU_11VHT */
	uint16 prev_txphyctl0, prev_txphyctl1;
	/* To keep ordering consistent with pre-rev40 prev_plcp[] use,
	 * plcp to prev_plcp mapping is not straightforward
	 *
	 * prev_plcp[0] holds plcp[0] (all revs)
	 * prev_plcp[1] holds plcp[3] (all revs)
	 * prev_plcp[2] holds plcp[2] (rev >= 40)
	 * prev_plcp[3] holds plcp[1] (rev >= 40)
	 * prev_plcp[4] holds plcp[4] (rev >= 40)
	 */
#if D11CONF_GE(40)
	uint8 prev_plcp[5];
#else
	uint8 prev_plcp[2];
#endif
	/* stats */
	int in_queue;
	uint8 depth;
	uint8 prev_shdr;
} ampdumac_info_t;

#endif /* WLAMPDU_MAC */

#ifdef WLOVERTHRUSTER
typedef struct wlc_tcp_ack_info {
	uint8 tcp_ack_ratio;
	uint32 tcp_ack_total;
	uint32 tcp_ack_dequeued;
	uint32 tcp_ack_multi_dequeued;
	uint32 current_dequeued;
	uint8 tcp_ack_ratio_depth;
} wlc_tcp_ack_info_t;
#endif

typedef struct {
	uint32 retry_histogram[AMPDU_MAX_MPDU+1];	/**< histogram for retried pkts */
	uint32 end_histogram[AMPDU_MAX_MPDU+1];		/**< errs till end of ampdu */
	uint32 mpdu_histogram[AMPDU_MAX_MPDU+1];	/**< mpdus per ampdu histogram */
	/* txmcs in sw agg is ampdu cnt, and is mpdu cnt for mac agg */
	uint32 txmcs[AMPDU_HT_MCS_ARRAY_SIZE];		/**< mcs of tx pkts */
	/* reason for suppressed err code as reported by ucode/aqm, see enum 'TX_STATUS_SUPR...' */
	uint32 supr_reason[NUM_TX_STATUS_SUPR];

#ifdef WLAMPDU_UCODE
	uint32 schan_depth_histo[AMPDU_MAX_MPDU+1];	/**< side channel depth */
#endif /* WLAMPDU_UCODE */

	uint32 txmcssgi[AMPDU_HT_MCS_ARRAY_SIZE];	/**< mcs of tx pkts */
	uint32 txmcsstbc[AMPDU_HT_MCS_ARRAY_SIZE];	/**< mcs of tx pkts */

#ifdef WLAMPDU_MAC
	/* used by aqm_agg to get PER */
	uint32 txmcs_succ[AMPDU_HT_MCS_ARRAY_SIZE];	/**< succ mpdus tx per mcs */
#endif

#ifdef WL11AC
	uint32 txvht[AMPDU_MAX_VHT];			/**< vht of tx pkts */
#ifdef WLAMPDU_MAC

	/* used by aqm_agg to get PER */
	uint32 txvht_succ[AMPDU_MAX_VHT];		/**< succ mpdus tx per vht */
#endif
	uint32 txvhtsgi[AMPDU_MAX_VHT];			/**< vht of tx pkts */
	uint32 txvhtstbc[AMPDU_MAX_VHT];		/**< vht of tx pkts */
#endif /* WL11AC */
} ampdu_dbg_t;

typedef struct {
	uint16 txallfrm;
	uint16 txbcn;
	uint16 txrts;
	uint16 rxcts;
	uint16 rsptmout;
	uint16 rxstrt;
	uint32 txop;
} mac_dbg_t;

/** ampdu config info, mostly config information that is common across WLC */
typedef struct ampdu_tx_config {
	bool manual_mode;	/**< addba tx to be driven by user */
	bool no_bar;		/**< do not send bar on failure */
	uint8 ini_enable[AMPDU_MAX_SCB_TID]; /**< per-tid initiator enable/disable of ampdu */
	uint8 ba_policy;	/**< ba policy; immediate vs delayed */
	uint8 ba_tx_wsize;      /**< Tx ba window size (in pdu) */
	uint8 retry_limit;	/**< mpdu transmit retry limit */
	uint8 rr_retry_limit;	/**< mpdu transmit retry limit at regular rate */
	uint8 retry_limit_tid[AMPDU_MAX_SCB_TID];	/**< per-tid mpdu transmit retry limit */
	/* per-tid mpdu transmit retry limit at regular rate */
	uint8 rr_retry_limit_tid[AMPDU_MAX_SCB_TID];
	uint8 mpdu_density;	/**< min mpdu spacing (0-7) ==> 2^(x-1)/8 usec */
	int8 max_pdu;		/**< max pdus allowed in ampdu */
	uint16 dur;		/**< max duration of an ampdu (in usec) */
	uint8 hiagg_mode;	/**< agg mpdus with different retry cnt */
	uint8 probe_mpdu;	/**< max mpdus allowed in ampdu for probe pkts */
	uint8 txpkt_weight;	/**< weight of ampdu in txfifo; reduces rate lag */
	uint8 delba_timeout;	/**< timeout after which to send delba (sec) */
	uint8 tx_rel_lowat;	/**< low watermark for num of pkts in transit */
#ifdef AMPDU_NON_AQM
	uint32 ffpld_rsvd;	/**< number of bytes to reserve for preload */
#if defined(WLPROPRIETARY_11N_RATES)
	uint32 max_txlen[AMPDU_HT_MCS_ARRAY_SIZE][2][2]; /**< max size of ampdu per [mcs,bw,sgi] */
#else
	uint32 max_txlen[MCS_TABLE_SIZE][2][2];
#endif /* WLPROPRIETARY_11N_RATES */

	bool mfbr;		/**< enable multiple fallback rate */
	uint32 tx_max_funl;              /**< underflows should be kept such that
					  * (tx_max_funfl*underflows) < tx frames
					  */
	wlc_fifo_info_t fifo_tb[NUM_FFPLD_FIFO]; /**< table of fifo infos  */

	uint8  aggfifo_depth;   /**< soft capability of AGGFIFO */
#endif /* non-AQM */
	int8 ampdu_aggmode;	/**< aggregation mode, HOST or MAC */
	int8 default_pdu;	/**< default pdus allowed in ampdu */
#ifdef WLMEDIA_TXFAILEVENT
	bool tx_fail_event;     /**< Whether host requires TX failure event: ON/OFF */
#endif /* WLMEDIA_TXFAILEVENT */
	bool	fb_override_enable; /**< configuration to enable/disable ampd_no_frameburst */
	uint8	ba_max_tx_wsize;	/**< Tx ba window size (in pdu) */
	uint8	release;			/**< # of mpdus released at a time */
	uint	txq_time_allowance_us;
	uint 	txq_time_min_allowance_us;
	bool	btcx_dur_flag;	/* Flag enabled if BTCOEX needs TX-AMPDU's clamped to 2.5ms */
	uint16  addba_retry_timeout; /* Retry timeout for addba requests */
} ampdu_tx_config_t;

/* DBG and counters are replicated per WLC */
/** AMPDU tx module specific state */
struct ampdu_tx_info {
	wlc_info_t *wlc;	/**< pointer to main wlc structure */
	int scb_handle;		/**< scb cubby handle to retrieve data from scb (=remote party) */
#ifdef WLCNT
	wlc_ampdu_tx_cnt_t *cnt;	/**< counters/stats */
#endif
#ifdef WLAMPDU_MAC
	ampdumac_info_t hagg[NFIFO_EXT];
#endif
	ampdu_dbg_t *amdbg;
	mac_dbg_t *mdbg;
#ifdef WLOVERTHRUSTER
	wlc_tcp_ack_info_t tcp_ack_info;  /**< stores a mix of config & runtime */
#endif
	bool    txaggr_support;         /**< Support ampdu tx aggregation */
	uint8   cubby_name_id;          /**< cubby ID */
	uint16  aqm_max_release[AMPDU_MAX_SCB_TID];
	struct ampdu_tx_config *config;
	int     bsscfg_handle;          /**< BSSCFG cubby offset */
	uint8 txnoprog_cnt;		/**< # of sec having no pkt consumed while PENDING_OFF */
};

#ifdef WLMEDIA_TXFAILEVENT
static void wlc_tx_failed_event(wlc_info_t *wlc, struct scb *scb, wlc_bsscfg_t *bsscfg,
	tx_status_t *txs, void *p, uint32 flags);
#endif /* WLMEDIA_TXFAILEVENT */

#if defined(TXQ_MUX)
/**
 * @brief a structure to hold parameters for packet tx time calculations
 *
 * This structure holds parameters for the time estimate calculation of an MPDU in an AQM A-MPDU.
 * This allows less work per-packet to calculation tx time.
 */
typedef struct ampdu_aqm_timecalc {
	ratespec_t rspec;               /**< @brief ratespec used in calculation */
	uint32 max_bytes;               /**< @brief maximum bytes in an AMPDU at current rate */
	uint16 Ndbps;                   /**< @brief bits to 4us symbol ratio */
	uint8 sgi;                      /**< @brief 1 if tx uses SGI */
	uint16 fixed_overhead_us;       /**< @brief length independent fixed time overhead */
	uint16 fixed_overhead_pmpdu_us; /**< @brief length independent fixed time overhead
	                                 *          minimum per-mpdu
	                                 */
	uint16 min_limit;               /**< @brief length boundary for fixed overhead
	                                 *          contribution methods
	                                 */
} ampdu_aqm_timecalc_t;
#endif /* TXQ_MUX */

#ifdef WLATF
typedef struct atf_stats_range {
	uint32	max; /* Maximum value */
	uint32	min; /* Minimum value */
	uint32	avg; /* Average value */
	unsigned long accum; /* Accmulator for average calculations */
	uint32	iter; /* Number of samples in acccmulator */
} atf_stats_range_t;

typedef struct atf_stats {
	uint32 timelimited; /* Times AMPDU release aborted due to time limit */
	uint32 framelimited; /* number of times full time limit was not released */
	uint32 reloverride; /* number of times ATF PMODE overrode release limit */
	uint32 uflow; /* number of times input queue went empty */
	uint32 oflow; /* nmber of times output queue was full */
	uint32 npkts; /* number of packets dequeued */
	uint32 ndequeue; /* Number of dequeue attempts */
	uint32 qempty; /* Incremented when packets in flight hit zero */
	uint32 minrel; /* Num times a dequeue request was below minimum time release */
	uint32 singleton; /* Count of single packets released */
	uint32 flush; /* Flush ATF accounting due to previous ATF reset */
	uint32 eval; /* number times ATF evaluated for packet release */
	uint32 neval; /* num times ATF stopped release */
	uint32 rskip; /* Release of packet skipped */
	uint32 cache_hit; /* ratespec cache hit */
	uint32 cache_miss; /* ratespec cache miss */
	uint32 proc; /* Number of packets completed */
	uint32 reproc; /* Times a completed packet was reprocessed */
	uint32 reg_mpdu; /* Packet sent as a regular MPDU from the AMPDU path */
	atf_stats_range_t chunk_bytes;
	atf_stats_range_t chunk_pkts;
	atf_stats_range_t rbytes;
	atf_stats_range_t inflight;
	atf_stats_range_t pdu;
	atf_stats_range_t release;
	atf_stats_range_t transit;
	atf_stats_range_t inputq;
} atf_stats_t;

typedef struct atf_rbytes {
	uint max;
	uint min;
} atf_rbytes_t;

/* CSTYLED */
typedef ratespec_t (*atf_rspec_fn_t)(void *);

/* ATF rate calculation function.
 * Can be changed in realtime if fixed rates are imposed in runtime
 */
typedef struct {
	atf_rspec_fn_t	function;
	void *arg;
} atf_rspec_action_t;

typedef struct atf_state {
	uint32 		released_bytes_inflight; /* Number of bytes pending in bytes */
	uint32 		released_packets_inflight; /* Number of pending pkts,
						 * the AMPDU TID structure has a similar counter
						 * but its used is overloaded to
						 * track powersave packets
						 */
	uint32 		last_est_rate;	 /* Last estimated rate */
	uint		released_bytes_target;
	uint		released_minbytes_target;
	uint   		ac;
	wlcband_t 	*band;
	uint 		atf;
	uint 		txq_time_allowance_us;
	uint 		txq_time_min_allowance_us;
	uint16		last_ampdu_fid;
	atf_rbytes_t	rbytes_target;
	uint 		reset; /* Number times ATF state was reset */
	rcb_t 		*rcb; /* Pointer to rate control block for this TID */
	ampdu_tx_info_t *ampdu_tx;	/* Back pointer to ampdu_tx structure */
	scb_ampdu_tx_t 	*scb_ampdu;	/* Back pointer to scb_ampdu structure */
	scb_ampdu_tid_ini_t *ini;	/* Back pointer to ini structure */
	wlc_info_t	*wlc;
	struct scb *scb;

	/* Pointer to access function that returns rate to be used */
	atf_rspec_action_t	rspec_action;	/* Get current rspec function */
	ratespec_t		rspec_override;	/* Fixed rate rspec override */
#ifdef WLATF_DONGLE
	wlc_atfd_t *atfd; /* ATF dongle metadata */
#endif
#ifdef WLCNT
	atf_stats_t	atf_stats;
#endif /* WLCNT */
} atf_state_t;
#endif /* WLATF */

/** structure to store per-tid state for the ampdu initiator */
struct scb_ampdu_tid_ini {
	uint8 ba_state;		/**< ampdu ba state */
	uint8 ba_wsize;		/**< negotiated ba window size (in pdu) */
	uint8 tid;		/**< initiator tid for easy lookup */
	uint8 txretry[AMPDU_BA_MAX_WSIZE];	/**< tx retry count; indexed by seq modulo */
	uint8 ackpending[AMPDU_BA_MAX_WSIZE/NBBY];	/**< bitmap: set if ack is pending */
	uint8 barpending[AMPDU_BA_MAX_WSIZE/NBBY];	/**< bitmap: set if bar is pending */
	uint16 tx_in_transit;	/**< #packets have left the AMPDU module and haven't been freed */
	uint16 barpending_seq;	/**< seqnum for bar */
	uint16 acked_seq;	/**< last ack recevied */
	uint16 start_seq;	/**< seqnum of the first unacknowledged packet */
	uint16 max_seq;		/**< max unacknowledged seqnum sent */
	uint16 tx_exp_seq;	/**< next exp seq in sendampdu */
	uint16 next_enq_seq;    /**< last pkt seq that has been sent to txfifo */
	uint16 rem_window;	/**< !AQM only: remaining ba window (in pdus) that can be txed. */
	uint16 bar_ackpending_seq; /**< seqnum of bar for which ack is pending */
	uint16 retry_seq[AMPDU_BA_MAX_WSIZE]; /**< seq of released retried pkts */
	uint16 retry_head;	/**< head of queue ptr for retried pkts */
	uint16 retry_tail;	/**< tail of queue ptr for retried pkts */
	uint16 retry_cnt;	/**< cnt of retried pkts */
	bool bar_ackpending;	/**< true if there is a bar for which ack is pending */
	bool alive;		/**< true if making forward progress */
	uint8 retry_bar;	/**< reason code if bar to be retried at watchdog */
	uint8 bar_cnt;		/**< number of bars sent with no progress */
	uint8 addba_req_cnt;	/**< number of addba_req sent with no progress */
	uint8 cleanup_ini_cnt;	/**< number of sec waiting in pending off state */
	uint8 dead_cnt;		/**< number of sec without the window moving */
	uint8 off_cnt;		/**< number of sec in off state before next try */
	struct scb *scb;	/**< backptr for easy lookup */
#ifdef WLATF
	atf_state_t atf_state;
#endif
#if defined(TXQ_MUX)
#if !defined(WLATF)
	uint   ac;              /**< Access Category of this TID */
#endif /* WLATF */
	ampdu_aqm_timecalc_t timecalc[AC_COUNT];
#endif /* TXQ_MUX */

#ifdef PROP_TXSTATUS
	/* rem_window is used to record the remaining ba window for new packets.
	 * when suppression happened, some holes may exist in current ba window,
	 * but they are not counted in rem_window.
	 * Then suppr_window is introduced to record suppressed packet counts inside ba window.
	 * Using suppr_window, we can keep rem_window untouched during suppression.
	 * When suppressed packets are resent by host, we need take both rem_window and
	 * suppr_window into account for decision of packet release.
	 */
	uint16 suppr_window; /**< suppr packet count inside ba window, including reg mpdu's */
#endif /* PROP_TXSTATUS */
	uint32	last_addba_ts;	/* timestamp of last addba req sent */
	uint16 last_suppr_seq;  /* last or highest pkt seq that is suppressed to host */
};

#ifdef BCMDBG
typedef struct scb_ampdu_cnt_tx {
	uint32 txampdu;
	uint32 txmpdu;
	uint32 txdrop;
	uint32 txstuck;
	uint32 txaddbareq;
	uint32 txrlag;
	uint32 txnoroom;
	uint32 sduretry;
	uint32 sdurejected;
	uint32 txlost;
	uint32 txbar;
	uint32 txreg_noack;
	uint32 noba;
	uint32 rxaddbaresp;
	uint32 rxdelba;
	uint32 rxba;
} scb_ampdu_cnt_tx_t;
#endif	/* BCMDBG */


/**
 * Scb cubby structure. Ini and resp are dynamically allocated if needed. A lot of instances of this
 * structure can be generated on e.g. APs, so be careful with respect to memory size of this struct.
 */
struct scb_ampdu_tx {
	struct scb *scb;                /**< back pointer for easy reference */
	ampdu_tx_info_t *ampdu_tx;      /**< back ref to main ampdu */
	scb_ampdu_tid_ini_t *ini[AMPDU_MAX_SCB_TID];    /**< initiator info */
	uint8 mpdu_density;		/**< mpdu density */
	uint8 max_pdu;			/**< max pdus allowed in ampdu */
	uint8 release;			/**< # of mpdus released at a time */
	uint16 min_len;			/**< min mpdu len to support the density */
	uint32 max_rxlen;		/**< max ampdu rcv length; 8k, 16k, 32k, 64k */
	struct pktq txq;		/**< sdu transmit queue pending aggregation */
	uint16 min_lens[AMPDU_HT_MCS_ARRAY_SIZE]; /* min mpdu lens per mcs */
	uint8 max_rxfactor;             /**< max ampdu length exponent + 13 */
	                                /**< (see Table 8-183v in IEEE Std 802.11ac-2012) */
	uint16 module_max_dur[NUM_MODULES]; /**< the maximum ampdu duration for each module */
	uint8 min_of_max_dur_idx;	/**< index of minimum of the maximum ampdu duration */
#ifdef BCMDBG
	scb_ampdu_cnt_tx_t cnt;
#endif	/* BCMDBG */
	ampdu_tx_scb_stats_t *ampdu_scb_stats;
};

struct ampdu_tx_cubby {
	scb_ampdu_tx_t *scb_tx_cubby;
};

#define SCB_AMPDU_INFO(ampdu, scb) (SCB_CUBBY((scb), (ampdu)->scb_handle))
#define SCB_AMPDU_TX_CUBBY(ampdu, scb) \
	(((struct ampdu_tx_cubby *)SCB_AMPDU_INFO(ampdu, scb))->scb_tx_cubby)

/** bsscfg cubby structure. */
typedef struct bsscfg_ampdu_tx {
	int8 txaggr_override;	/**< txaggr override for all TIDs */
	uint16 txaggr_TID_bmap; /**< aggregation enabled TIDs bitmap */
} bsscfg_ampdu_tx_t;

#define BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, bsscfg) \
	((bsscfg_ampdu_tx_t *)BSSCFG_CUBBY((bsscfg), (ampdu_tx)->bsscfg_handle))

/* local prototypes */
/* scb cubby */
static int scb_ampdu_tx_update(void *context, struct scb *scb, wlc_bsscfg_t* new_cfg);
static int scb_ampdu_tx_init(void *context, struct scb *scb);
static void scb_ampdu_tx_deinit(void *context, struct scb *scb);
/* bsscfg cubby */
static int bsscfg_ampdu_tx_init(void *context, wlc_bsscfg_t *cfg);
static void bsscfg_ampdu_tx_deinit(void *context, wlc_bsscfg_t *cfg);
static void scb_ampdu_txflush(void *context, struct scb *scb);
static int wlc_ampdu_doiovar(void *hdl, uint32 actionid,
        void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif);
static void wlc_ampdu_watchdog(void *hdl);
static int wlc_ampdu_up(void *hdl);
static int wlc_ampdu_down(void *hdl);
static void wlc_ampdu_init_min_lens(scb_ampdu_tx_t *scb_ampdu);

static INLINE void wlc_ampdu_ini_move_window(ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini);
static void ampdu_ini_move_window(ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini);

static void wlc_ampdu_assoc_state_change(void *client, bss_assoc_state_data_t *notif_data);

#ifdef AMPDU_NON_AQM
static void ampdu_update_max_txlen(ampdu_tx_config_t *ampdu_tx_cfg, uint16 dur);
static void wlc_ffpld_init(ampdu_tx_config_t *ampdu_tx_cfg);
static int wlc_ffpld_check_txfunfl(wlc_info_t *wlc, int f, wlc_bsscfg_t *bsscfg);
static void wlc_ffpld_calc_mcs2ampdu_table(ampdu_tx_info_t *ampdu_tx, int f, wlc_bsscfg_t *bsscfg);
#ifdef BCMDBG
static void wlc_ffpld_show(ampdu_tx_info_t *ampdu_tx);
#endif /* BCMDBG */
#ifdef WLAMPDU_MAC
#if !defined(TXQ_MUX)
static int aggfifo_enque(ampdu_tx_info_t *ampdu_tx, int length, int qid);
static uint16 wlc_ampdu_calc_maxlen(ampdu_tx_info_t *ampdu_tx, uint8 plcp0, uint plcp3,
	uint32 txop);
#endif /* TXQ_MUX */
#if defined(BCMDBG) || !defined(TXQ_MUX)
static uint get_aggfifo_space(ampdu_tx_info_t *ampdu_tx, int qid);
#endif
#endif /* WLAMPDU_MAC */
#endif /* non-AQM */

#ifdef WLOVERTHRUSTER
static void wlc_ampdu_tx_set_tcp_ack_ratio(ampdu_tx_info_t *ampdu_tx, uint8 tcp_ack_ratio);
static void wlc_ampdu_tcp_ack_suppress(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
                                       void *p, uint8 tid);
#else
static bool wlc_ampdu_is_tcpack(ampdu_tx_info_t *ampdu_tx, void *p);
#endif
#ifdef WLAMPDU_MAC
#ifdef BCMDBG
static void wlc_print_ampdu_txstatus(ampdu_tx_info_t *ampdu_tx,
	tx_status_t *pkg1, uint32 s1, uint32 s2, uint32 s3, uint32 s4);
#endif
#endif /* WLAMPDU_MAC */

#if defined(TXQ_MUX)

#if defined(WLAMPDU_MAC)

/*
 * Following block of routines for AQM AMPDU aggregation (d11 core rev >= 40)
 */
static uint wlc_ampdu_output_aqm(void *ctx, uint dfifo, uint request_time, struct spktq *output_q);
static int wlc_ampdu_output_nonba(struct pktq *q, struct scb *scb, wlc_info_t *wlc,
                                  ratespec_t rspec, uint ac, uint tid,
                                  uint request_time, struct spktq *output_q);
static void wlc_ampdu_init_aqm_timecalc(scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini,
                                        ratespec_t rspec, uint ac, ampdu_aqm_timecalc_t *tc);
static uint wlc_ampdu_aqm_timecalc(ampdu_aqm_timecalc_t *tc, uint mpdu_len);

#endif /* WLAMPDU_MAC */

#else

#ifdef WLAMPDU_MAC
static int BCMFASTPATH
_wlc_sendampdu_aqm(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec,
	struct spktq *output_q, int *supplied_time);
#endif /* WLAMPDU_MAC */

#ifdef AMPDU_NON_AQM
static int BCMFASTPATH
_wlc_sendampdu_noaqm(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec,
	struct spktq *output_q, int *supplied_time);
#endif /* AMPDU_NON_AQM */

#endif /* TXQ_MUX */


#ifdef WLAMPDU_MAC
static void wlc_ampdu_dotxstatus_regmpdu_aqm(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
	void *p, tx_status_t *txs, wlc_txh_info_t *txh_info);

#if !(defined(TXQ_MUX) && !defined(WLAMPDU_MAC))
/* Fn is not needed if TXQ_MUX && !WLAMPDU_MAC */

static bool BCMFASTPATH ampdu_is_exp_seq_aqm(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini,
	uint16 seq, bool suppress_pkt);

#endif /* !(TXQ_MUX && !WLAMPDU_MAC) */

static INLINE void wlc_ampdu_ini_move_window_aqm(ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini);
#endif /* WLAMPDU_MAC */

static void wlc_ampdu_dotxstatus_regmpdu_noaqm(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
	void *p, tx_status_t *txs);

#if !(defined(TXQ_MUX) && !defined(WLAMPDU_MAC))
/* Fn is not needed if TXQ_MUX && !WLAMPDU_MAC */
static bool BCMFASTPATH ampdu_is_exp_seq_noaqm(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini,
	uint16 seq, bool suppress_pkt);
#endif /* !(TXQ_MUX && !WLAMPDU_MAC) */

static INLINE void wlc_ampdu_ini_move_window_noaqm(ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini);

static void wlc_ampdu_send_bar(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini, bool start);

#if (defined(PKTC) || defined(PKTC_TX_DONGLE))
static void wlc_ampdu_agg_pktc(void *ctx, struct scb *scb, void *p, uint prec);
#else
static void wlc_ampdu_agg(void *ctx, struct scb *scb, void *p, uint prec);
#endif
static scb_ampdu_tid_ini_t *wlc_ampdu_init_tid_ini(ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, uint8 tid, bool override);
static void ampdu_ba_state_off(scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini);
static bool wlc_ampdu_txeval(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini, bool force);
static void wlc_ampdu_release(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini, uint16 release, uint16 qavail);
static void wlc_ampdu_free_chain(ampdu_tx_info_t *ampdu_tx, void *p, tx_status_t *txs,
	bool txs3_done);

static void wlc_send_bar_complete(wlc_info_t *wlc, uint txstatus, void *arg);

#define wlc_ampdu_txflowcontrol(a, b, c)	do {} while (0)

static void wlc_ampdu_dotxstatus_complete(ampdu_tx_info_t *ampdu_tx, struct scb *scb, void *p,
	tx_status_t *txs, uint32 frmtxstatus, uint32 frmtxstatus2, uint32 s3, uint32 s4);

#ifdef BCMDBG
static void ampdu_dump_ini(scb_ampdu_tid_ini_t *ini);
#else
#define	ampdu_dump_ini(a)
#endif
static uint wlc_ampdu_txpktcnt(void *hdl);

#ifdef WLATF
static void wlc_ampdu_atf_tid_set_release_time(scb_ampdu_tid_ini_t *ini,
	uint txq_time_allowance_us);
static void wlc_ampdu_atf_scb_set_release_time(ampdu_tx_info_t *ampdu_tx,
	struct scb *scb, uint txq_time_allowance_us);
static void wlc_ampdu_atf_set_default_release_time(ampdu_tx_info_t *ampdu_tx,
	scb_module_t *scbstate, uint txq_time_allowance_us);

static void wlc_ampdu_atf_tid_set_release_mintime(scb_ampdu_tid_ini_t *ini,
	uint txq_time_allowance_us);
static void wlc_ampdu_atf_scb_set_release_mintime(ampdu_tx_info_t *ampdu_tx,
	struct scb *scb, uint txq_time_allowance_us);
static void wlc_ampdu_atf_set_default_release_mintime(ampdu_tx_info_t *ampdu_tx,
	scb_module_t *scbstate, uint txq_time_allowance_us);

static void wlc_ampdu_atf_tid_setmode(scb_ampdu_tid_ini_t *ini, uint32 mode);
static void wlc_ampdu_atf_scb_setmode(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint32 mode);
static void wlc_ampdu_atf_ini_set_rspec_action(atf_state_t *atf_state, ratespec_t rspec);

#if defined(WLCNT) && defined(BCMDBG)
static void wlc_ampdu_atf_dump(atf_state_t *atf_state, struct bcmstrbuf *b);
static BCMFASTPATH void wlc_ampdu_atf_update_rstat(atf_stats_range_t *range, uint32 val);
static void wlc_ampdu_atf_print_rstat(struct bcmstrbuf *b,
	char *hdr, atf_stats_range_t *rstat, char *tlr);
static void wlc_ampdu_atf_clear_counters(scb_ampdu_tid_ini_t *ini);
#define AMPDU_ATF_INCRSTAT(r, v) wlc_ampdu_atf_update_rstat((r), (v))
#define AMPDU_ATF_CLRCNT(ini) wlc_ampdu_atf_clear_counters((ini))
#else
#define AMPDU_ATF_INCRSTAT(r, v)
#define AMPDU_ATF_CLRCNT(ini)
#endif /* WLCNT */
#else
#define AMPDU_ATF_INCRSTAT(r, v)
#define AMPDU_ATF_CLRCNT(ini)
#endif /* WLATF */

static void wlc_ampdu_ini_adjust(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini, void *pkt);
static void wlc_ampdu_pkt_freed(wlc_info_t *wlc, void *pkt, uint txs);

static void wlc_ampdu_retry_ba_session(wlc_info_t *wlc, struct scb *scb, scb_ampdu_tid_ini_t *ini);
static void wlc_ampdu_ba_pending_off(wlc_info_t *wlc, struct scb *scb,
	scb_ampdu_tid_ini_t *ini, uint8 tid);

static txmod_fns_t BCMATTACHDATA(ampdu_txmod_fns) = {
#if (defined(PKTC) || defined(PKTC_TX_DONGLE)) /* packet chaining */
	wlc_ampdu_agg_pktc,	/* Process the packet */
#else
	wlc_ampdu_agg,
#endif
	wlc_ampdu_txpktcnt,	/* Return the packet count */
	scb_ampdu_txflush,	/* Handle the deactivation of the feature */
	NULL			/* Handle the activation of the feature */
};

#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */
/** ncons 'number consumed' marks last packet in tx chain that can be freed */
static uint16
wlc_ampdu_rawbits_to_ncons(uint16 raw_bits)
{
	return ((raw_bits & TX_STATUS40_NCONS) >> TX_STATUS40_NCONS_SHIFT);
}
#endif /* WLAMPDU_MAC */


/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/** Called only for non AC chips */
static INLINE uint16
pkt_txh_seqnum(wlc_info_t *wlc, void *p)
{
	wlc_txh_info_t txh_info;
	wlc_get_txh_info(wlc, p, &txh_info);
	return txh_info.seq;
}

/* Updates the max ampdu duration array where each element represents the duration per module
* and finds the minimum duration in the array.
*/
void wlc_ampdu_tx_max_dur(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
	scb_ampdu_module_t module_id, uint16 dur)
{
	uint8 i;
	uint8 min_idx;
	uint16 min_dur;
	scb_ampdu_tx_t *scb_ampdu;

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);

	ASSERT(scb_ampdu != NULL);

	scb_ampdu->module_max_dur[module_id] = dur;
	min_idx = AMPDU_MAXDUR_INVALID_IDX;
	min_dur = AMPDU_MAXDUR_INVALID_VAL;

	for (i = 0; i < NUM_MODULES; i++) {
		if (scb_ampdu->module_max_dur[i] < min_dur) {
			min_dur = scb_ampdu->module_max_dur[i];
			min_idx = i;
		}
	}
	scb_ampdu->min_of_max_dur_idx = min_idx;
}


/** Selects the max Tx PDUs tunables for 2 stream and 3 stream depending on the hardware support. */
static void BCMATTACHFN(wlc_set_ampdu_tunables)(wlc_info_t *wlc)
{
	if (D11REV_GE(wlc->pub->corerev, D11AQM_CORE)) {
		wlc->pub->tunables->ampdunummpdu1stream  = AMPDU_NUM_MPDU_1SS_D11AQM;
		wlc->pub->tunables->ampdunummpdu2streams = AMPDU_NUM_MPDU_2SS_D11AQM;
		wlc->pub->tunables->ampdunummpdu3streams = AMPDU_NUM_MPDU_3SS_D11AQM;
	} else if (D11REV_GE(wlc->pub->corerev, D11HT_CORE)) {
		wlc->pub->tunables->ampdunummpdu1stream  = AMPDU_NUM_MPDU_1SS_D11HT;
		wlc->pub->tunables->ampdunummpdu2streams = AMPDU_NUM_MPDU_2SS_D11HT;
		wlc->pub->tunables->ampdunummpdu3streams = AMPDU_NUM_MPDU_3SS_D11HT;
	} else {
		wlc->pub->tunables->ampdunummpdu1stream  = AMPDU_NUM_MPDU_1SS_D11LEGACY;
		wlc->pub->tunables->ampdunummpdu2streams = AMPDU_NUM_MPDU_2SS_D11LEGACY;
		wlc->pub->tunables->ampdunummpdu3streams = AMPDU_NUM_MPDU_3SS_D11LEGACY;
	}
}

/** as part of system init, a transmit ampdu related data structure has to be initialized */
static void
BCMATTACHFN(wlc_ampdu_tx_cfg_init)(wlc_info_t *wlc, ampdu_tx_config_t *ampdu_tx_cfg)
{
	int i;

	ampdu_tx_cfg->ba_max_tx_wsize = AMPDU_TX_BA_MAX_WSIZE;

	/* initialize all priorities to allow AMPDU aggregation */
	for (i = 0; i < AMPDU_MAX_SCB_TID; i++)
		ampdu_tx_cfg->ini_enable[i] = TRUE;

	/* For D11 revs < 40, disable AMPDU on some priorities due to underlying fifo space
	 * allocation.
	 * D11 rev >= 40 has a shared tx fifo space managed by BMC hw that allows AMPDU agg
	 * to be used for any fifo.
	 */
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		/* Disable ampdu for VO by default */
		ampdu_tx_cfg->ini_enable[PRIO_8021D_VO] = FALSE;
		ampdu_tx_cfg->ini_enable[PRIO_8021D_NC] = FALSE;

		/* Disable ampdu for BK by default since not enough fifo space except for MACOS */
		ampdu_tx_cfg->ini_enable[PRIO_8021D_NONE] = FALSE;
		ampdu_tx_cfg->ini_enable[PRIO_8021D_BK] = FALSE;

		/* Enable aggregation for BK */
		if (D11REV_IS(wlc->pub->corerev, 28) ||
		    D11REV_IS(wlc->pub->corerev, 26) ||
		    (D11REV_IS(wlc->pub->corerev, 29) && WLCISHTPHY(wlc->band))) {
			ampdu_tx_cfg->ini_enable[PRIO_8021D_NONE] = TRUE;
			ampdu_tx_cfg->ini_enable[PRIO_8021D_BK] = TRUE;
		}
	}
#ifdef WL_DISABLE_VO_AGG
	/* Disable ampdu for voice traffic */
	ampdu_tx_cfg->ini_enable[PRIO_8021D_VO] = FALSE;
	ampdu_tx_cfg->ini_enable[PRIO_8021D_NC] = FALSE;
#endif /* WL_DISABLE_VO_AGG */
	ampdu_tx_cfg->ba_policy = DOT11_ADDBA_POLICY_IMMEDIATE;
	ampdu_tx_cfg->ba_tx_wsize = AMPDU_TX_BA_DEF_WSIZE; /* Tx ba window size (in pdu) */

	if (ampdu_tx_cfg->ba_tx_wsize > ampdu_tx_cfg->ba_max_tx_wsize) {
		WL_ERROR(("wl%d: The Default AMPDU_TX_BA_WSIZE is greater than MAX value\n",
			wlc->pub->unit));
		ampdu_tx_cfg->ba_tx_wsize = ampdu_tx_cfg->ba_max_tx_wsize;
	}
	if (D11REV_IS(wlc->pub->corerev, 40) ||
	    D11REV_IS(wlc->pub->corerev, 41) ||
	    D11REV_IS(wlc->pub->corerev, 28)) {
		ampdu_tx_cfg->mpdu_density = AMPDU_DENSITY_8_US;
	} else {
		ampdu_tx_cfg->mpdu_density = AMPDU_DEF_MPDU_DENSITY;
	}
	ampdu_tx_cfg->max_pdu = AUTO;
	ampdu_tx_cfg->default_pdu =
		(wlc->stf->txstreams < 3) ? AMPDU_NUM_MPDU_LEGACY : AMPDU_MAX_MPDU;
	ampdu_tx_cfg->dur = AMPDU_MAX_DUR;
	ampdu_tx_cfg->hiagg_mode = FALSE;
	ampdu_tx_cfg->probe_mpdu = AMPDU_DEF_PROBE_MPDU;
	ampdu_tx_cfg->btcx_dur_flag = FALSE;
	ampdu_tx_cfg->fb_override_enable = FRAMEBURST_OVERRIDE_DEFAULT;

	ampdu_tx_cfg->retry_limit = AMPDU_DEF_RETRY_LIMIT;
	ampdu_tx_cfg->rr_retry_limit = AMPDU_DEF_RR_RETRY_LIMIT;

	for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
		ampdu_tx_cfg->retry_limit_tid[i] = ampdu_tx_cfg->retry_limit;
		ampdu_tx_cfg->rr_retry_limit_tid[i] = ampdu_tx_cfg->rr_retry_limit;
	}

	ampdu_tx_cfg->delba_timeout = 0; /* AMPDUXXX: not yet supported */
	ampdu_tx_cfg->tx_rel_lowat = AMPDU_DEF_TX_LOWAT;

#ifdef WLMEDIA_TXFAILEVENT
	ampdu_tx_cfg->tx_fail_event = FALSE; /* Tx failure notification off by default */
#endif

	ampdu_tx_cfg->txq_time_allowance_us = AMPDU_TXQ_TIME_ALLOWANCE_US;

#ifdef WLATF
	ampdu_tx_cfg->txq_time_min_allowance_us = AMPDU_TXQ_TIME_MIN_ALLOWANCE_US;
#endif

	ampdu_tx_cfg->ampdu_aggmode = AMPDU_AGGMODE_AUTO;

#ifdef AMPDU_NON_AQM
	ampdu_update_max_txlen(ampdu_tx_cfg, ampdu_tx_cfg->dur);
	ampdu_tx_cfg->ffpld_rsvd = AMPDU_DEF_FFPLD_RSVD;
	ampdu_tx_cfg->mfbr = TRUE;
	ampdu_tx_cfg->tx_max_funl = FFPLD_TX_MAX_UNFL;
	wlc_ffpld_init(ampdu_tx_cfg);
#endif /* non-AQM */
} /* wlc_ampdu_tx_cfg_init */

/* cfg inits that must be done after certain variables/states are initialized:
 * 1) wlc->pub->_ampdumac
 */
static void
BCMATTACHFN(wlc_ampdu_tx_cfg_init_post)(wlc_info_t *wlc, ampdu_tx_config_t *ampdu_tx_cfg)
{
	if (AMPDU_MAC_ENAB(wlc->pub))
		ampdu_tx_cfg->txpkt_weight = 1;
	else
		ampdu_tx_cfg->txpkt_weight = AMPDU_DEF_TXPKT_WEIGHT;

	if (!AMPDU_AQM_ENAB(wlc->pub))
		ampdu_tx_cfg->release = AMPDU_SCB_MAX_RELEASE;
	else
		ampdu_tx_cfg->release = AMPDU_SCB_MAX_RELEASE_AQM;

	ampdu_tx_cfg->addba_retry_timeout = AMPDU_ADDBA_REQ_RETRY_TIMEOUT;
}

ampdu_tx_info_t *
BCMATTACHFN(wlc_ampdu_tx_attach)(wlc_info_t *wlc)
{
	scb_cubby_params_t cubby_params;
	ampdu_tx_info_t *ampdu_tx;
	int i;

	/* some code depends on packed structures */
	STATIC_ASSERT(sizeof(struct dot11_bar) == DOT11_BAR_LEN);
	STATIC_ASSERT(sizeof(struct dot11_ba) == DOT11_BA_LEN + DOT11_BA_BITMAP_LEN);
	STATIC_ASSERT(sizeof(struct dot11_ctl_header) == DOT11_CTL_HDR_LEN);
	STATIC_ASSERT(sizeof(struct dot11_addba_req) == DOT11_ADDBA_REQ_LEN);
	STATIC_ASSERT(sizeof(struct dot11_addba_resp) == DOT11_ADDBA_RESP_LEN);
	STATIC_ASSERT(sizeof(struct dot11_delba) == DOT11_DELBA_LEN);
	STATIC_ASSERT(DOT11_MAXNUMFRAGS == NBITS(uint16));
	STATIC_ASSERT(ISPOWEROF2(AMPDU_TX_BA_MAX_WSIZE));

	wlc_set_ampdu_tunables(wlc);

	ASSERT(wlc->pub->tunables->ampdunummpdu2streams <= AMPDU_MAX_MPDU);
	ASSERT(wlc->pub->tunables->ampdunummpdu2streams > 0);
	ASSERT(wlc->pub->tunables->ampdunummpdu3streams <= AMPDU_MAX_MPDU);
	ASSERT(wlc->pub->tunables->ampdunummpdu3streams > 0);

	if (!(ampdu_tx = (ampdu_tx_info_t *)MALLOC(wlc->osh, sizeof(ampdu_tx_info_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}
	bzero((char *)ampdu_tx, sizeof(ampdu_tx_info_t));
	ampdu_tx->wlc = wlc;
#ifndef WLRSDB_DVT
	ampdu_tx->config = (ampdu_tx_config_t*) obj_registry_get(wlc->objr,
		OBJR_AMPDUTX_CONFIG);
#endif
	if (ampdu_tx->config == NULL) {
		if ((ampdu_tx->config =  (ampdu_tx_config_t*) MALLOCZ(wlc->pub->osh,
			sizeof(ampdu_tx_config_t))) == NULL) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes", wlc->pub->unit,
				__FUNCTION__, MALLOCED(wlc->pub->osh)));
			goto fail;
		}
		obj_registry_set(wlc->objr, OBJR_AMPDUTX_CONFIG, ampdu_tx->config);
		wlc_ampdu_tx_cfg_init(wlc, ampdu_tx->config);
	}
#ifndef WLRSDB_DVT
	(void)obj_registry_ref(wlc->objr, OBJR_AMPDUTX_CONFIG);
#endif

#ifdef WLCNT
	if (!(ampdu_tx->cnt = (wlc_ampdu_tx_cnt_t *)MALLOC(wlc->osh, sizeof(wlc_ampdu_tx_cnt_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	bzero((char *)ampdu_tx->cnt, sizeof(wlc_ampdu_tx_cnt_t));
#endif /* WLCNT */

	/* Read nvram param to see if it disables AMPDU tx aggregation */
	if ((getintvar(wlc->pub->vars, "11n_disable") &
		WLFEATURE_DISABLE_11N_AMPDU_TX)) {
		ampdu_tx->txaggr_support = FALSE;
	} else {
		ampdu_tx->txaggr_support = TRUE;
	}

	for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
		if (i == PRIO_8021D_BE) {
			ampdu_tx->aqm_max_release[i] = AMPDU_AQM_RELEASE_MAX;
		} else {
			ampdu_tx->aqm_max_release[i] = AMPDU_AQM_RELEASE_DEFAULT;
		}
	}


#ifdef WLOVERTHRUSTER
	if ((WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band) || WLCISLCN20PHY(wlc->band)) &&
		(OVERTHRUST_ENAB(wlc->pub))) {
		wlc_ampdu_tx_set_tcp_ack_ratio(ampdu_tx, AMPDU_DEF_TCP_ACK_RATIO);
		ampdu_tx->tcp_ack_info.tcp_ack_ratio_depth = AMPDU_DEF_TCP_ACK_RATIO_DEPTH;
	}
#endif

	/* reserve cubby in the bsscfg container for private data */
	if ((ampdu_tx->bsscfg_handle = wlc_bsscfg_cubby_reserve(wlc,
		sizeof(bsscfg_ampdu_tx_t), bsscfg_ampdu_tx_init, bsscfg_ampdu_tx_deinit,
		NULL, (void *)ampdu_tx)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

#if defined(WL_DATAPATH_LOG_DUMP)
	ampdu_tx->cubby_name_id = wlc_scb_cubby_name_register(wlc->scbstate, "AMPDU");
	if (ampdu_tx->cubby_name_id == WLC_SCB_NAME_ID_INVALID) {
		WL_ERROR(("wl%d: wlc_scb_cubby_name_register() failed\n", wlc->pub->unit));
		goto fail;
	}
#endif
	/* reserve cubby in the scb container for per-scb private data */
	bzero(&cubby_params, sizeof(cubby_params));

	cubby_params.context = ampdu_tx;
	cubby_params.fn_init = scb_ampdu_tx_init;
	cubby_params.fn_deinit = scb_ampdu_tx_deinit;
#if defined(WL_DATAPATH_LOG_DUMP)
	cubby_params.fn_data_log_dump = wlc_ampdu_datapath_summary;
#endif
	cubby_params.fn_update = scb_ampdu_tx_update;
	ampdu_tx->scb_handle = wlc_scb_cubby_reserve_ext(wlc, sizeof(struct ampdu_tx_cubby),
		&cubby_params);

	if (ampdu_tx->scb_handle < 0) {
		WL_ERROR(("wl%d: wlc_scb_cubby_reserve_ext() failed\n", wlc->pub->unit));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)|| defined(BCMDBG_AMPDU) || \
	defined(WL_LINKSTAT)
	if (!(ampdu_tx->amdbg = (ampdu_dbg_t *)MALLOCZ(wlc->osh, sizeof(ampdu_dbg_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
#endif 

	/* register assoc state change callback for reassociation */
	if (wlc_bss_assoc_state_register(wlc, wlc_ampdu_assoc_state_change, ampdu_tx) != BCME_OK) {
		WL_ERROR(("wl%d: wlc_bss_assoc_state_register() failed\n", wlc->pub->unit));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(BCMDBG_DUMP) || \
	defined(BCMDBG_AMPDU)
	if (!(ampdu_tx->mdbg = (mac_dbg_t *)MALLOCZ(wlc->osh, sizeof(mac_dbg_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes for mac_dbg_t\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
#endif /*  defined(BCMDBG) ... */

#ifdef WL_CS_RESTRICT_RELEASE
	if (wlc_restrict_attach(wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_restrict_attach failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif

	/* register packet class callback */
	if (wlc_pcb_fn_set(wlc->pcb, 3, WLF2_PCB4_AMPDU, wlc_ampdu_pkt_freed) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_pcb_fn_set() failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register module -- needs to be last failure prone operation in this function */
	if (wlc_module_register(wlc->pub, ampdu_iovars, "ampdu_tx", ampdu_tx, wlc_ampdu_doiovar,
	                        wlc_ampdu_watchdog, wlc_ampdu_up, wlc_ampdu_down)) {
		WL_ERROR(("wl%d: ampdu_tx wlc_module_register() failed\n", wlc->pub->unit));
		goto fail;
	}

	/* register txmod function */
	wlc_txmod_fn_register(wlc->txmodi, TXMOD_AMPDU, ampdu_tx, ampdu_txmod_fns);

	/* try to set ampdu to the default value */
	wlc_ampdu_tx_set(ampdu_tx, wlc->pub->_ampdu_tx);

	wlc_ampdu_tx_cfg_init_post(wlc, ampdu_tx->config);

#ifdef WLAMPDU_MAC
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#if defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "aggfifo", (dump_fn_t)wlc_dump_aggfifo, (void*)wlc);
#endif
#endif /* BCMDBG || BCMDBG_DUMP */
#endif /* WLAMPDU_MAC */

	return ampdu_tx;

fail:
	if (obj_registry_unref(wlc->objr, OBJR_AMPDUTX_CONFIG) == 0) {
		obj_registry_set(wlc->objr, OBJR_AMPDUTX_CONFIG, NULL);
		if (ampdu_tx->config != NULL) {
			MFREE(wlc->osh, ampdu_tx->config, sizeof(ampdu_tx_config_t));
		}
	}
#ifdef WLCNT
	if (ampdu_tx->cnt)
		MFREE(wlc->osh, ampdu_tx->cnt, sizeof(wlc_ampdu_tx_cnt_t));
#endif /* WLCNT */
	MFREE(wlc->osh, ampdu_tx, sizeof(ampdu_tx_info_t));
	return NULL;
} /* wlc_ampdu_tx_attach */


void
BCMATTACHFN(wlc_ampdu_tx_detach)(ampdu_tx_info_t *ampdu_tx)
{
	ampdu_tx_config_t *config;
	wlc_info_t *wlc;

	if (!ampdu_tx)
		return;

	wlc = ampdu_tx->wlc;
	config = ampdu_tx->config;

	if (obj_registry_unref(wlc->objr, OBJR_AMPDUTX_CONFIG) == 0) {
		obj_registry_set(wlc->objr, OBJR_AMPDUTX_CONFIG, NULL);
		MFREE(wlc->osh, config, sizeof(ampdu_tx_config_t));
	}

#ifdef WLCNT
	if (ampdu_tx->cnt)
		MFREE(wlc->osh, ampdu_tx->cnt, sizeof(wlc_ampdu_tx_cnt_t));
#endif
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMPDU) || \
	defined(WL_LINKSTAT)
	if (ampdu_tx->amdbg) {
		MFREE(wlc->osh, ampdu_tx->amdbg, sizeof(ampdu_dbg_t));
		ampdu_tx->amdbg = NULL;
	}
#endif

#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(BCMDBG_DUMP) || \
	defined(BCMDBG_AMPDU)
	if (ampdu_tx->mdbg) {
		MFREE(wlc->osh, ampdu_tx->mdbg, sizeof(mac_dbg_t));
		ampdu_tx->mdbg = NULL;
	}
#endif

#ifdef WL_CS_RESTRICT_RELEASE
	wlc_restrict_detach(wlc);
#endif

	wlc_bss_assoc_state_unregister(wlc, wlc_ampdu_assoc_state_change, ampdu_tx);
	wlc_module_unregister(wlc->pub, "ampdu_tx", ampdu_tx);
	MFREE(wlc->osh, ampdu_tx, sizeof(ampdu_tx_info_t));
} /* wlc_ampdu_tx_detach */

struct pktq * scb_ampdu_prec_pktq(wlc_info_t* wlc, struct scb* scb)
{
	struct pktq *q;
	scb_ampdu_tx_t *scb_ampdu;

	scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
	q = &scb_ampdu->txq;

	return q;
}

/**
 * Per 'conversation partner' an SCB is maintained. The 'cubby' in this SCB contains ampdu tx info
 * for a specific conversation partner.
 */
/* scb cubby init fn */
static int
scb_ampdu_tx_init(void *context, struct scb *scb)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)context;
	struct ampdu_tx_cubby *cubby_info = (struct ampdu_tx_cubby *)SCB_AMPDU_INFO(ampdu_tx, scb);
	scb_ampdu_tx_t *scb_ampdu;
	wlc_tunables_t *tunables = ampdu_tx->wlc->pub->tunables;
	int i;

	if (scb && !SCB_INTERNAL(scb)) {
		scb_ampdu = MALLOCZ(ampdu_tx->wlc->osh, sizeof(scb_ampdu_tx_t) +
			sizeof(ampdu_tx_scb_stats_t));
		if (!scb_ampdu) {
			WL_ERROR((WLC_MALLOC_ERR, WLCWLUNIT(ampdu_tx->wlc), __FUNCTION__,
				(int)sizeof(scb_ampdu_tx_t), MALLOCED(ampdu_tx->wlc->osh)));
			return BCME_NOMEM;
		}
		cubby_info->scb_tx_cubby = scb_ampdu;
		scb_ampdu->scb = scb;
		scb_ampdu->ampdu_tx = ampdu_tx;
		scb_ampdu->ampdu_scb_stats = (ampdu_tx_scb_stats_t *)((char *)scb_ampdu +
			sizeof(scb_ampdu_tx_t));

#ifdef WLAMPDU_PRECEDENCE

		/* Set the overall queue packet limit to maximum number of packets, just rely on
		 * per-prec limits.
		 */
		pktq_init(&scb_ampdu->txq, WLC_PREC_COUNT, PKTQ_LEN_MAX);
		for (i = 0; i < WLC_PREC_COUNT; i += 2) {
			pktq_set_max_plen(&scb_ampdu->txq, i, tunables->ampdu_pktq_size);
			pktq_set_max_plen(&scb_ampdu->txq, i+1, tunables->ampdu_pktq_fav_size);
		}
#else /* WLAMPDU_PRECEDENCE */
		/* Set the overall queue packet limit to the max, just rely on per-prec limits */
		pktq_init(&scb_ampdu->txq, AMPDU_MAX_SCB_TID, PKTQ_LEN_MAX);
		for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
			pktq_set_max_plen(&scb_ampdu->txq, i, tunables->ampdu_pktq_size);
		}
#endif /* WLAMPDU_PRECEDENCE */

	}
	return 0;
} /* scb_ampdu_tx_init */

/** Call back function, frees queues related to a specific remote party ('scb') */
static void
scb_ampdu_tx_deinit(void *context, struct scb *scb)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)context;
	struct ampdu_tx_cubby *cubby_info = (struct ampdu_tx_cubby *)SCB_AMPDU_INFO(ampdu_tx, scb);
	scb_ampdu_tx_t *scb_ampdu = NULL;

	WL_AMPDU_UPDN(("scb_ampdu_deinit: enter\n"));

	ASSERT(cubby_info);

	if (cubby_info)
		scb_ampdu = cubby_info->scb_tx_cubby;
	if (!scb_ampdu)
		return;

	scb_ampdu_tx_flush(ampdu_tx, scb);
#ifdef PKTQ_LOG
	wlc_pktq_stats_free(ampdu_tx->wlc, &scb_ampdu->txq);
#endif

	pktq_deinit(&scb_ampdu->txq);

	MFREE(ampdu_tx->wlc->osh, scb_ampdu, sizeof(scb_ampdu_tx_t) +
		sizeof(ampdu_tx_scb_stats_t));
	cubby_info->scb_tx_cubby = NULL;
}

static int
scb_ampdu_tx_update(void *context, struct scb *scb, wlc_bsscfg_t* new_cfg)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)context;
	wlc_info_t *new_wlc = new_cfg->wlc;
	struct ampdu_tx_cubby *cubby_info = (struct ampdu_tx_cubby *)SCB_AMPDU_INFO(ampdu_tx, scb);
	scb_ampdu_tx_t *scb_ampdu = cubby_info->scb_tx_cubby;
	int idx = 0;
	scb_ampdu->ampdu_tx = new_wlc->ampdu_tx;
	for (idx = 0; idx < AMPDU_MAX_SCB_TID; idx++) {
		if (scb_ampdu->ini[idx]) {
#ifdef WLATF
			AMPDU_ATF_STATE(scb_ampdu->ini[idx])->band = new_wlc->band;
#endif /* WLATF */
		}
	}
	return BCME_OK;
}

/** Callback function, bsscfg cubby init fn */
static int
bsscfg_ampdu_tx_init(void *context, wlc_bsscfg_t *bsscfg)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)context;
	bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, bsscfg);
	ASSERT(bsscfg != NULL);

	if (ampdu_tx->txaggr_support) {
		/* Enable for all TID by default */
		bsscfg_ampdu->txaggr_override = AUTO;
		bsscfg_ampdu->txaggr_TID_bmap = AMPDU_ALL_TID_BITMAP;
	} else {
		/* AMPDU TX module does not allow tx aggregation */
		bsscfg_ampdu->txaggr_override = OFF;
		bsscfg_ampdu->txaggr_TID_bmap = 0;
	}

	return BCME_OK;
}

/** Callback function, bsscfg cubby deinit fn */
static void
bsscfg_ampdu_tx_deinit(void *context, wlc_bsscfg_t *bsscfg)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)context;
	bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, bsscfg);

	WL_AMPDU_UPDN(("bsscfg_ampdu_tx_deinit: enter\n"));

	bsscfg_ampdu->txaggr_override = OFF;
	bsscfg_ampdu->txaggr_TID_bmap = 0;
}

/** Callback function, part of the tx module (txmod_fns_t) interface */
static void
scb_ampdu_txflush(void *context, struct scb *scb)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)context;

	scb_ampdu_tx_flush(ampdu_tx, scb);
}

/** flush is needed on deinit, ampdu deactivation, etc. */
void
scb_ampdu_tx_flush(ampdu_tx_info_t *ampdu_tx, struct scb *scb)
{
	scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	uint8 tid;

	WL_AMPDU_UPDN(("scb_ampdu_tx_flush: enter\n"));

	for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
		ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, TRUE);
	}

	/* free all buffered tx packets */
	wlc_txq_pktq_flush(ampdu_tx->wlc, &scb_ampdu->txq);
}

/**
 * Called as part of 'wl up' specific initialization. Reset the ampdu state machine so that it can
 * gracefully handle packets that were freed from the dma and tx queues during reinit.
 */
void
wlc_ampdu_tx_reset(ampdu_tx_info_t *ampdu_tx)
{
	struct scb *scb;
	struct scb_iter scbiter;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint8 tid;

	WL_AMPDU_UPDN(("wlc_ampdu_tx_reset: enter\n"));
	FOREACHSCB(ampdu_tx->wlc->scbstate, &scbiter, scb) {
		if (!SCB_AMPDU(scb))
			continue;
		scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
			ini = scb_ampdu->ini[tid];
			if (!ini)
				continue;
			if ((ini->ba_state == AMPDU_TID_STATE_BA_ON) && ini->tx_in_transit)
				ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, TRUE);
		}
	}
}

/**
 * (Re)initialize tx config related to a specific 'conversation partner'. Called when setting up a
 * new conversation, as a result of a wl IOV_AMPDU_MPDU command or transmit underflow.
 */
void
scb_ampdu_update_config(ampdu_tx_info_t *ampdu_tx, struct scb *scb)
{
	scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	int peer3ss = 0, peer2ss = 0;

	/* The maximum number of TX MPDUs supported for the device is determined at the
	  * AMPDU attach time
	 * At this point we know the largest size the device can send, here the intersection
	 * with the peer's capability is done.
	 */

	/* 3SS peer check */
	if (SCB_VHT_CAP(scb)) {
		/* VHT capable peer */
		peer3ss = VHT_MCS_SS_SUPPORTED(3, scb->rateset.vht_mcsmap);
		peer2ss = VHT_MCS_SS_SUPPORTED(2, scb->rateset.vht_mcsmap);
	} else {
		/* HT Peer */
		peer3ss = (scb->rateset.mcs[2] != 0);
		peer2ss = (scb->rateset.mcs[1] != 0);
	}

	if ((scb->flags & SCB_BRCM) && !(scb->flags2 & SCB2_RX_LARGE_AGG) && !(peer3ss))
	{
		scb_ampdu->max_pdu = /* max pdus allowed in ampdu */
			MIN(ampdu_tx->wlc->pub->tunables->ampdunummpdu2streams,
			AMPDU_NUM_MPDU_LEGACY);
	} else {
		if (peer3ss) {
			scb_ampdu->max_pdu =
				(uint8)ampdu_tx->wlc->pub->tunables->ampdunummpdu3streams;
		} else if (peer2ss) {
			scb_ampdu->max_pdu =
				(uint8)ampdu_tx->wlc->pub->tunables->ampdunummpdu2streams;
		} else {
			scb_ampdu->max_pdu =
				(uint8)ampdu_tx->wlc->pub->tunables->ampdunummpdu1stream;
		}
#ifdef WL_MU_TX
		if (SCB_MU(scb)) {
			scb_ampdu->max_pdu = SCB_AMSDU_IN_AMPDU(scb) ?
				(uint8)ampdu_tx->wlc->pub->tunables->ampdunummpdu2streams :
				(uint8)ampdu_tx->wlc->pub->tunables->ampdunummpdu3streams;
		}
#endif
	}

	ampdu_tx_cfg->default_pdu = scb_ampdu->max_pdu;

#ifdef AMPDU_NON_AQM
	if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		int i;
		/* go back to legacy size if some preloading is occuring */
		for (i = 0; i < NUM_FFPLD_FIFO; i++) {
			if (ampdu_tx_cfg->fifo_tb[i].ampdu_pld_size > FFPLD_PLD_INCR)
				scb_ampdu->max_pdu = ampdu_tx_cfg->default_pdu;
		}
	}
#endif /* non-AQM */

	/* apply user override */
	if (ampdu_tx_cfg->max_pdu != AUTO)
		scb_ampdu->max_pdu = (uint8)ampdu_tx_cfg->max_pdu;

	if (AMPDU_HW_ENAB(ampdu_tx->wlc->pub)) {
		scb_ampdu->release = ampdu_tx_cfg->ba_max_tx_wsize;
	} else {

		scb_ampdu->release = MIN(scb_ampdu->max_pdu, ampdu_tx_cfg->release);

#ifdef AMPDU_NON_AQM
		if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {

			if (scb_ampdu->max_rxlen)
				scb_ampdu->release =
				    MIN(scb_ampdu->release, scb_ampdu->max_rxlen/1600);

			scb_ampdu->release = MIN(scb_ampdu->release,
			ampdu_tx_cfg->fifo_tb[TX_AC_BE_FIFO].mcs2ampdu_table[AMPDU_MAX_MCS]);

		}
#endif /* AMPDU_NON_AQM */
	}
	ASSERT(scb_ampdu->release);
} /* scb_ampdu_update_config */

/**
 * Initialize tx config related to all 'conversation partners'. Called as a result of a
 * wl IOV_AMPDU_MPDU command or transmit underflow.
 */
void
scb_ampdu_update_config_all(ampdu_tx_info_t *ampdu_tx)
{
	struct scb *scb;
	struct scb_iter scbiter;

	WL_AMPDU_UPDN(("scb_ampdu_update_config_all: enter\n"));
	FOREACHSCB(ampdu_tx->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb))
			scb_ampdu_update_config(ampdu_tx, scb);
	}
	/* The ampdu config affects values that are in the tx headers of outgoing packets.
	 * Invalidate tx header cache entries to clear out the stale information
	 */
	if (WLC_TXC_ENAB(ampdu_tx->wlc))
		wlc_txc_inv_all(ampdu_tx->wlc->txc);
}

/**
 * Callback function, returns the number of transmit packets held by AMPDU. Part of the txmod_fns_t
 * interface.
 */
static uint
wlc_ampdu_txpktcnt(void *hdl)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)hdl;
	struct scb *scb;
	struct scb_iter scbiter;
	int pktcnt = 0;
	scb_ampdu_tx_t *scb_ampdu;

	FOREACHSCB(ampdu_tx->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
			pktcnt += pktq_n_pkts_tot(&scb_ampdu->txq);
		}
	}

	return pktcnt;
}

/** frees all the buffers and cleanup everything on wireless interface down */
static int
wlc_ampdu_down(void *hdl)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)hdl;
	struct scb *scb;
	struct scb_iter scbiter;

	WL_AMPDU_UPDN(("wlc_ampdu_down: enter\n"));

	if (WOWL_ACTIVE(ampdu_tx->wlc->pub))
		return 0;

	FOREACHSCB(ampdu_tx->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb))
			scb_ampdu_tx_flush(ampdu_tx, scb);
	}

#ifdef AMPDU_NON_AQM
	/* we will need to re-run the pld tuning */
	wlc_ffpld_init(ampdu_tx->config);
#endif

	return 0;
}

/** Init/Cleanup after Reinit/BigHammer */
static int
wlc_ampdu_up(void *hdl)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)hdl;
	wlc_info_t *wlc = ampdu_tx->wlc;
	struct scb_iter scbiter;
	struct scb *scb;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint tid;

	if (!wlc->pub->up)
		return BCME_OK;

	/* reinit/bighammer */

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb) &&
		    (scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb)) != NULL) {
			for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid ++) {
				if ((ini = scb_ampdu->ini[tid]) != NULL) {
					ampdu_ini_move_window(ampdu_tx, scb_ampdu, ini);
				}
			}
		}
	}

	return BCME_OK;
}

/** Reset BA agreement after a reassociation (roam/reconnect) */
static void
wlc_ampdu_assoc_state_change(void *client, bss_assoc_state_data_t *notif_data)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)client;
	wlc_info_t *wlc = ampdu_tx->wlc;
	wlc_bsscfg_t *cfg = notif_data->cfg;
	struct scb *scb;

	/* force BA reset */

	if (BSSCFG_STA(cfg) &&
	    notif_data->type == AS_ROAM &&
	    /* We should do it on entering AS_JOIN_INIT state but
	     * unfortunately we don't have it for roam/reconnect.
	     */
	    notif_data->state == AS_JOIN_ADOPT) {
		if ((scb = wlc_scbfindband(wlc, cfg, &cfg->BSSID,
			CHSPEC_WLCBANDUNIT(cfg->current_bss->chanspec))) != NULL &&
		    SCB_AMPDU(scb)) {
			wlc_scb_ampdu_disable(wlc, scb);
			wlc_scb_ampdu_enable(wlc, scb);
		}
	}
}

/**
 * When tx aggregation is disabled by a higher software layer, AMPDU connections for all
 * 'conversation partners' in the caller supplied bss have to be torn down.
 */
/* If wish to do for all TIDs, input AMPDU_ALL_TID_BITMAP for conf_TDI_bmap */
static void
wlc_ampdu_tx_cleanup(ampdu_tx_info_t *ampdu_tx, wlc_bsscfg_t *bsscfg,
	uint16 conf_TID_bmap)
{
	uint8 tid;
	scb_ampdu_tx_t *scb_ampdu = NULL;
	struct scb *scb;
	struct scb_iter scbiter;
	wlc_info_t *wlc = ampdu_tx->wlc;

	scb_ampdu_tid_ini_t *ini;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		if (!SCB_AMPDU(scb))
			continue;

		if (!SCB_ASSOCIATED(scb))
			continue;

		scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
		ASSERT(scb_ampdu);
		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
			if (!(isbitset(conf_TID_bmap, tid))) {
				continue;
			}

			ini = scb_ampdu->ini[tid];

			if (ini != NULL)
			{
				if ((ini->ba_state == AMPDU_TID_STATE_BA_ON) ||
					(ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
					wlc_ampdu_tx_send_delba(ampdu_tx, scb, tid, TRUE,
						DOT11_RC_TIMEOUT);
				}
				ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, FALSE);
			}
			/* free buffered tx packets of this TID */
			wlc_ampdu_pktq_pflush(ampdu_tx->wlc, &scb_ampdu->txq, tid);
		}
	}
} /* wlc_ampdu_tx_cleanup */

/**
 * frame bursting waits a shorter time (RIFS) between tx frames which increases throughput but
 * potentially decreases airtime fairness.
 */
bool
wlc_ampdu_frameburst_override(ampdu_tx_info_t *ampdu_tx, struct scb *scb)
{
	if (ampdu_tx == NULL) {
		return FALSE;
	}
	else {
		ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
		if (ampdu_tx_cfg->fb_override_enable) {
			if (SCB_AMPDU(scb) && SCB_HT_CAP(scb)) {
				scb_ampdu_tx_t *scb_ampdu = NULL;
				scb_ampdu_tid_ini_t *ini;
				uint8 tid;
				bool override_allowed = TRUE;

				scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
				for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
					ini = scb_ampdu->ini[tid];
					if (!ini)
						continue;
					if (ini->ba_state == AMPDU_TID_STATE_BA_ON) {
						/* don't override FB as BA session is found */
						override_allowed = FALSE;
						break;
					}
				}
				return override_allowed;
			}
			else {
				/* AMPDU disabled or Legacy Client.
				* FB override is allowed
				*/
				return TRUE;
			}
		}
		else {
			/* FB override is disabled; allow FB */
			return FALSE;
		}
	}
}

#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(BCMDBG_DUMP) || \
	defined(BCMDBG_AMPDU)
/* offset of cntmember by sizeof(uint32) from the first cnt variable. */
#define NUM_MCST_IN_MAC_DBG_T 6
static const uint8 mac_dbg_t_to_macstat_t[NUM_MCST_IN_MAC_DBG_T] = {
	MCSTOFF_TXFRAME,
	MCSTOFF_TXBCNFRM,
	MCSTOFF_TXRTSFRM,
	MCSTOFF_RXCTSUCAST,
	MCSTOFF_RXRSPTMOUT,
	MCSTOFF_RXSTRT
};

static void
wlc_ampdu_dbg_stats(wlc_info_t *wlc, struct scb *scb, scb_ampdu_tid_ini_t *ini)
{
	uint8 i;
	mac_dbg_t now_mdbg;
	mac_dbg_t *mdbg;

	BCM_REFERENCE(scb);

	if (ini->dead_cnt == 0) return;

	mdbg = wlc->ampdu_tx->mdbg;
	if (!mdbg)
		return;

	memset(&now_mdbg, 0, sizeof(now_mdbg));
	for (i = 0; i < NUM_MCST_IN_MAC_DBG_T; i++) {
		*((uint16 *)&now_mdbg) =
			wlc_read_shm(wlc, MACSTAT_ADDR(wlc, mac_dbg_t_to_macstat_t[i]));
	}
	now_mdbg.txop = wlc_bmac_cca_read_counter(wlc->hw,
		M_CCA_TXOP_L_OFFSET(wlc), M_CCA_TXOP_H_OFFSET(wlc));

	if (ini->dead_cnt >= AMPDU_INI_DEAD_TIMEOUT) {
		osl_t *osh;
		d11regs_t *regs;

		osh = wlc->osh;
		regs = wlc->regs;

		WL_ERROR(("ampdu_dbg: wl%d.%d "MACF" scb:%p tid:%d \n",
			wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
			ETHER_TO_MACF(scb->ea), scb, ini->tid));

		if (D11REV_GE(wlc->pub->corerev, 64)) {
			WL_PRINT(("ampdu_dbg: wl%d.%d dead_cnt %d tx_in_transit %d "
				"psm_reg_mux 0x%x aqmqmap 0x%x aqmfifo_status 0x%x\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
				ini->dead_cnt, ini->tx_in_transit,
				R_REG(osh, &regs->u.d11acregs.psm_reg_mux),
				R_REG(osh, &regs->u.d11acregs.AQMQMAP),
				R_REG(osh, &regs->u.d11acregs.AQMFifo_Status)));
		} else {
			uint16 fifordy, frmcnt, fifosel;
			if (D11REV_GE(wlc->pub->corerev, 40)) {
				fifordy = R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMFifoReady);
				fifosel = R_REG(osh, &regs->u.d11acregs.BMCCmd);
				frmcnt = R_REG(osh, &regs->u.d11acregs.XmtFifoFrameCnt);

			} else {
				fifordy = R_REG(osh, &regs->u.d11regs.xmtfifordy);
				fifosel = R_REG(osh, &regs->u.d11regs.xmtsel);
				frmcnt = R_REG(osh, &regs->u.d11regs.xmtfifo_frame_cnt);
			}
			WL_PRINT(("ampdu_dbg: wl%d.%d dead_cnt %d tx_in_transit %d "
				"fifordy 0x%x frmcnt 0x%x fifosel 0x%x\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
				ini->dead_cnt, ini->tx_in_transit,
				fifordy, frmcnt, fifosel));
			BCM_REFERENCE(fifordy);
			BCM_REFERENCE(fifosel);
			BCM_REFERENCE(frmcnt);
		}

		WL_PRINT(("ampdu_dbg: ifsstat 0x%x nav_stat 0x%x txop %u\n",
			R_REG(osh, &regs->u.d11regs.ifsstat),
			R_REG(osh, &regs->u.d11regs.navstat),
			now_mdbg.txop - mdbg->txop));

		WL_PRINT(("ampdu_dbg: pktpend: %d %d %d %d %d ap %d\n",
			TXPKTPENDGET(wlc, TX_AC_BK_FIFO), TXPKTPENDGET(wlc, TX_AC_BE_FIFO),
			TXPKTPENDGET(wlc, TX_AC_VI_FIFO), TXPKTPENDGET(wlc, TX_AC_VO_FIFO),
			TXPKTPENDGET(wlc, TX_BCMC_FIFO), AP_ENAB(wlc->pub)));

		WL_PRINT(("ampdu_dbg: txall %d txbcn %d txrts %d rxcts %d rsptmout %d rxstrt %d\n",
			now_mdbg.txallfrm - mdbg->txallfrm, now_mdbg.txbcn - mdbg->txbcn,
			now_mdbg.txrts - mdbg->txrts, now_mdbg.rxcts - mdbg->rxcts,
			now_mdbg.rsptmout - mdbg->rsptmout, now_mdbg.rxstrt - mdbg->rxstrt));

		WL_PRINT(("ampdu_dbg: cwcur0-3 %x %x %x %x bslots cur/0-3 %d %d %d %d %d "
			 "ifs_boff %d\n",
			 wlc_read_shm(wlc, 0x240 + 3*2), wlc_read_shm(wlc, 0x260 + 3*2),
			 wlc_read_shm(wlc, 0x280 + 3*2), wlc_read_shm(wlc, 0x2a0 + 3*2),
			 wlc_read_shm(wlc, 0x16f*2),
			 wlc_read_shm(wlc, 0x240 + 5*2), wlc_read_shm(wlc, 0x260 + 5*2),
			 wlc_read_shm(wlc, 0x280 + 5*2), wlc_read_shm(wlc, 0x2a0 + 5*2),
			 R_REG(osh, &regs->u.d11regs.ifs_boff)));

		for (i = 0; i < 2; i++) {
			WL_PRINT(("ampdu_dbg: again%d ifsstat 0x%x nav_stat 0x%x\n",
				i + 1,
				R_REG(osh, &regs->u.d11regs.ifsstat),
				R_REG(osh, &regs->u.d11regs.navstat)));
		}
	}

	/* Copy macstat_t counters */
	memcpy(mdbg, &now_mdbg, sizeof(now_mdbg));

#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
	if (BCM_DMA_CT_ENAB(wlc) && (ini->dead_cnt >= AMPDU_INI_DEAD_TIMEOUT)) {
		int npkts;
		for (i = 0; i < WLC_HW_NFIFO_INUSE(wlc); i++) {
			npkts = TXPKTPENDGET(wlc, i);
			if (npkts == 0) continue;
			WL_ERROR(("FIFO-%d TXPEND = %d TX-DMA%d =>\n", i, npkts, i));
#if (defined(BCMDBG) || defined(BCMDBG_DUMP))
			if (ini->dead_cnt >= 8 || wlc->ampdu_tx->txnoprog_cnt > 0) {
				hnddma_t *di = WLC_HW_DI(wlc, i);
				dma_dumptx(di, NULL, FALSE);
				WL_ERROR(("CT-DMA%d =>\n", i));
				di = WLC_HW_AQM_DI(wlc, i);
				dma_dumptx(di, NULL, FALSE);
			}
#endif /* BCMDBG || BCMDBG_DUMP */
		}
	}
#endif /* BCM_DMA_CT && !BCM_DMA_CT_DISABLED */
} /* wlc_ampdu_dbg_stats */
#else
static void
wlc_ampdu_dbg_stats(wlc_info_t *wlc, struct scb *scb, scb_ampdu_tid_ini_t *ini)
{
}
#endif /* defined(BCMDBG) || defined(BCMDBG_ERR) */

/**
 * Timer function, called approx every second. Resends ADDBA-Req if the ADDBA-Resp has not come
 * back.
 */
static void
wlc_ampdu_watchdog(void *hdl)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)hdl;
	wlc_info_t *wlc = ampdu_tx->wlc;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	int idx;
	wlc_bsscfg_t *cfg;
	struct scb *scb;
	struct scb_iter scbiter;
	uint8 tid;
	bool any_ba_state_pendoff = FALSE;

	FOREACH_BSS(wlc, idx, cfg) {
	    FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {

		if (!SCB_AMPDU(scb))
			continue;
		scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
		ASSERT(scb_ampdu);
		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {

			ini = scb_ampdu->ini[tid];
			if (!ini)
				continue;
			/* dont skip watchdog if delba cleaning is pending. */
			if (TRUE &&
				!(ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF)) {
				continue;
			}
			switch (ini->ba_state) {
			case AMPDU_TID_STATE_BA_ON:
				/* tickle the sm and release whatever pkts we can */
				wlc_ampdu_txeval(ampdu_tx, scb_ampdu, ini, TRUE);

				if (ini->retry_bar) {
#if !defined(TXQ_MUX)
					/* Free queued packets when system is short of memory;
					 * release 1/4 buffered and at most wl_ampdu_drain_cnt pkts.
					 */
					if (ini->retry_bar == AMPDU_R_BAR_NO_BUFFER) {
						void *p;
						int rel_cnt, i;

						WL_ERROR(("wl%d: %s: no memory\n",
							wlc->pub->unit, __FUNCTION__));
						rel_cnt = ampdu_pktqprec_n_pkts(&scb_ampdu->txq,
							ini->tid) >> 2;
						if (wl_ampdu_drain_cnt <= 0)
							wl_ampdu_drain_cnt = AMPDU_SCB_DRAIN_CNT;
						rel_cnt = MIN(rel_cnt, wl_ampdu_drain_cnt);
						for (i = 0; i < rel_cnt; i++) {
							p = wlc_ampdu_pktq_pdeq(&scb_ampdu->txq,
								ini->tid);
							ASSERT(p != NULL);
							ASSERT(PKTPRIO(p) == ini->tid);

							wlc_tx_status_update_counters(wlc, p,
								scb, NULL, WLC_TX_STS_NO_BUF, 1);

							PKTFREE(wlc->pub->osh, p, TRUE);
							AMPDUSCBCNTINCR(scb_ampdu->cnt.txdrop);
							WLCNTINCR(ampdu_tx->cnt->txdrop);
#ifdef WL11K
							wlc_rrm_stat_qos_counter(scb, tid,
								OFFSETOF(rrm_stat_group_qos_t,
								txdrop));
#endif
						}
					}
#endif /* TXQ_MUX */
					wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
				} else if (ini->bar_cnt >= AMPDU_BAR_RETRY_CNT) {
					wlc_ampdu_tx_send_delba(ampdu_tx, scb, tid,
						TRUE, DOT11_RC_TIMEOUT);
				}
				/* check on forward progress */
				else if (ini->alive) {
					ini->alive = FALSE;
					ini->dead_cnt = 0;
				} else {
					if (ini->tx_in_transit)
						ini->dead_cnt++;

					WLCNTINCR(ampdu_tx->cnt->ampdu_wds);
					WLCNTINCR(wlc->pub->_cnt->ampdu_wds);
					wlc_ampdu_dbg_stats(wlc, scb, ini);

					if (ini->dead_cnt == AMPDU_INI_DEAD_TIMEOUT) {
#if defined(AP) && defined(BCMDBG)
						if (SCB_PS(scb)) {
							char* mode = "PS";
#ifdef PSPRETEND
							if (SCB_PS_PRETEND(scb)) {
								if (SCB_PS_PRETEND_THRESHOLD(scb)) {
									mode = "threshold PPS";
								} else {
									mode = "PPS";
								}
							}
#endif /* PSPRETEND */
							WL_AMPDU_ERR(("wl%d.%d: %s: "MACF" in %s "
							     "mode may be stuck or receiver died\n",
							     wlc->pub->unit,
							     WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
							     __FUNCTION__, ETHER_TO_MACF(scb->ea),
							     mode));
						}
#endif /* AP && BCMDBG */

						WL_AMPDU_ERR(("wl%d: %s: scb:%p cleaning up ini"
							" tid %d due to no progress for %d secs"
							" tx_in_transit %d\n",
							wlc->pub->unit, __FUNCTION__, scb,
							tid, ini->dead_cnt, ini->tx_in_transit));
						WLCNTINCR(ampdu_tx->cnt->txstuck);
						AMPDUSCBCNTINCR(scb_ampdu->cnt.txstuck);
						/* AMPDUXXX: unknown failure, send delba */
						wlc_ampdu_tx_send_delba(ampdu_tx, scb, tid,
							TRUE, DOT11_RC_TIMEOUT);
						ampdu_dump_ini(ini);
						ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, TRUE);
					}
				}
				break;

			case AMPDU_TID_STATE_BA_PENDING_ON:
				wlc_ampdu_retry_ba_session(wlc, scb, ini);
				break;
			case AMPDU_TID_STATE_BA_PENDING_OFF: {
				any_ba_state_pendoff = TRUE;
				wlc_ampdu_ba_pending_off(wlc, scb, ini, tid);
				break;
			}

			case AMPDU_TID_STATE_BA_OFF:
				if (ini->off_cnt++ >= AMPDU_INI_OFF_TIMEOUT) {
					ini = wlc_ampdu_init_tid_ini(ampdu_tx, scb_ampdu,
						tid, FALSE);
					/* make a single attempt only */
					if (ini)
						ini->addba_req_cnt = AMPDU_ADDBA_REQ_RETRY_CNT;
				}
				break;
			default:
				break;
			}
			/* dont do anything with ini here since it might be freed */

		}
	    }
	}

	if (any_ba_state_pendoff) {
		if (ampdu_tx->txnoprog_cnt++ >= AMPDU_TXNOPROG_LIMIT) {
			/* dump mac and macx dump */
			wlc_dump_mac_fatal(wlc, PSM_FATAL_TXSTUCK);
		}
	}
} /* wlc_ampdu_watchdog */
static void wlc_ampdu_retry_ba_session(wlc_info_t *wlc, struct scb *scb, scb_ampdu_tid_ini_t *ini)
{
	ampdu_tx_info_t *ampdu_tx = wlc->ampdu_tx;
	scb_ampdu_tx_t *scb_ampdu;
	int ampdu_addba_req_retry = AMPDU_ADDBA_REQ_RETRY_CNT;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	uint32 elapsed_ts_addbatx = (OSL_SYSUPTIME() - ini->last_addba_ts);
	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	if (AWDL_ENAB(wlc->pub))
		ampdu_addba_req_retry += 5;
	/* For non-awdl scb stop after addba_req_cnt max count is reached */
	if (!BSSCFG_AWDL(wlc, scb->bsscfg) && (ini->addba_req_cnt >= ampdu_addba_req_retry)) {
		ampdu_ba_state_off(scb_ampdu, ini);
	} else {
		/* - Retry on the scbs with retries < 10 with addba_retry_timeout buffer
		 * - Else retry with with 1 second buffer
		 */
		if ((ini->addba_req_cnt < 10 &&
			(elapsed_ts_addbatx >= ampdu_tx_cfg->addba_retry_timeout)) ||
			(elapsed_ts_addbatx >= 1000)) {
			ini->addba_req_cnt++;
#ifdef MFP
			if (!WLC_MFP_ENAB(wlc->pub) ||
				(BSSCFG_AWDL(wlc, scb->bsscfg)) ||
				(scb->wsec == 0) ||
				(WLC_MFP_ENAB(wlc->pub) && SCB_AUTHORIZED(scb)))
#endif /* MFP */
			{
				WL_AMPDU_ERR(("addba timed out %d\n", ini->addba_req_cnt));
				wlc_send_addba_req(wlc, scb, ini->tid,
					ampdu_tx_cfg->ba_tx_wsize, ampdu_tx_cfg->ba_policy,
					ampdu_tx_cfg->delba_timeout);
				ini->last_addba_ts = OSL_SYSUPTIME();
			}
			WLCNTINCR(ampdu_tx->cnt->txaddbareq);
			AMPDUSCBCNTINCR(scb_ampdu->cnt.txaddbareq);
		}
	}
}
void wlc_ampdu_check_pending_ba_for_bsscfg(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		if (!SCB_AMPDU(scb))
			continue;
		wlc_ampdu_check_pending_ba_for_scb(wlc, scb);
	}
}

void wlc_ampdu_check_pending_ba_for_scb(wlc_info_t *wlc, struct scb *scb)
{
	uint8 tid;
	ampdu_tx_info_t *ampdu_tx = wlc->ampdu_tx;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;

	if (!SCB_AMPDU(scb))
		return;
	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
		ini = scb_ampdu->ini[tid];
		if (!ini ||
			(ini->ba_state != AMPDU_TID_STATE_BA_PENDING_ON))
			continue;
		wlc_ampdu_retry_ba_session(wlc, scb, ini);
	}
}

static
void wlc_ampdu_ba_pending_off(wlc_info_t *wlc, struct scb *scb,
	scb_ampdu_tid_ini_t *ini, uint8 tid)
{
	ampdu_tx_info_t *ampdu_tx = wlc->ampdu_tx;
	scb_ampdu_tx_t *scb_ampdu;

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);
	BCM_REFERENCE(scb_ampdu);
	if (ini->cleanup_ini_cnt++ >= AMPDU_INI_CLEANUP_TIMEOUT) {
		uint32 phydebug, txop, fifordy, txe_ctl;
		osl_t *osh = wlc->osh;
		d11regs_t *regs = wlc->regs;

		phydebug = R_REG(osh, &regs->phydebug);
		txop = wlc_bmac_cca_read_counter(wlc->hw, M_CCA_TXOP_L_OFFSET(wlc),
			M_CCA_TXOP_L_OFFSET(wlc));
		if (D11REV_GE(wlc->pub->corerev, 64)) {
			fifordy = R_REG(osh, &regs->u.d11acregs.AQMFifoRdy_L);
			fifordy |= (R_REG(osh, &regs->u.d11acregs.AQMFifoRdy_H) << 16);
		} else if (D11REV_GE(wlc->pub->corerev, 40)) {
			fifordy = R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMFifoReady);
		} else {
			fifordy = R_REG(osh, &regs->u.d11regs.xmtfifordy);
		}
		txe_ctl = R_REG(osh, &regs->txe_ctl);
		WL_ERROR(("wl%d: %s: cleaning up tid %d from poff"
			" phydebug %08x txop %08x"
			" fifordy %08x txe_ctl %08x txnoprog_cnt %d\n",
			wlc->pub->unit, __FUNCTION__, tid, phydebug,
			txop, fifordy, txe_ctl, ampdu_tx->txnoprog_cnt));
		BCM_REFERENCE(phydebug);
		BCM_REFERENCE(txop);
		BCM_REFERENCE(fifordy);
		BCM_REFERENCE(txe_ctl);
		WLCNTINCR(ampdu_tx->cnt->txstuck);
		AMPDUSCBCNTINCR(scb_ampdu->cnt.txstuck);
		ampdu_dump_ini(ini);
		wlc_ampdu_dbg_stats(wlc, scb, ini);
		ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, TRUE);
	} else {
		ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, FALSE);
	}
}


/** handle AMPDU related iovars */
static int
wlc_ampdu_doiovar(void *hdl, uint32 actionid,
	void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)hdl;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	int32 int_val = 0;
	int32 *ret_int_ptr = (int32 *) a;
	bool bool_val;
	int err = 0;
	wlc_info_t *wlc;
	wlc_bsscfg_t *bsscfg;

	BCM_REFERENCE(alen);
	BCM_REFERENCE(vsize);
	BCM_REFERENCE(wlcif);

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;
	wlc = ampdu_tx->wlc;
	ASSERT(ampdu_tx == wlc->ampdu_tx);

	if (ampdu_tx->txaggr_support == FALSE) {
		WL_OID(("wl%d: %s: ampdu_tx->txaggr_support is FALSE\n",
			wlc->pub->unit, __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	switch (actionid) {
	case IOV_GVAL(IOV_AMPDU_TX):
		*ret_int_ptr = (int32)wlc->pub->_ampdu_tx;
		break;

	case IOV_SVAL(IOV_AMPDU_TX):
		return wlc_ampdu_tx_set(ampdu_tx, (bool)int_val);

	case IOV_GVAL(IOV_AMPDU_TID): {
		struct ampdu_tid_control *ampdu_tid = (struct ampdu_tid_control *)p;

		if (ampdu_tid->tid >= AMPDU_MAX_SCB_TID) {
			err = BCME_BADARG;
			break;
		}
		ampdu_tid->enable = ampdu_tx_cfg->ini_enable[ampdu_tid->tid];
		bcopy(ampdu_tid, a, sizeof(*ampdu_tid));
		break;
		}

	case IOV_SVAL(IOV_AMPDU_TID): {
		struct ampdu_tid_control *ampdu_tid = (struct ampdu_tid_control *)a;

		if (ampdu_tid->tid >= AMPDU_MAX_SCB_TID) {
			err = BCME_BADARG;
			break;
		}
		ampdu_tx_cfg->ini_enable[ampdu_tid->tid] = ampdu_tid->enable ? TRUE : FALSE;
		break;
		}

	case IOV_GVAL(IOV_AMPDU_TX_DENSITY):
		*ret_int_ptr = (int32)ampdu_tx_cfg->mpdu_density;
		break;

	case IOV_SVAL(IOV_AMPDU_TX_DENSITY):
		if (int_val > AMPDU_MAX_MPDU_DENSITY) {
			err = BCME_RANGE;
			break;
		}

		if (int_val < AMPDU_DEF_MPDU_DENSITY) {
			err = BCME_RANGE;
			break;
		}
		ampdu_tx_cfg->mpdu_density = (uint8)int_val;
		break;

	case IOV_SVAL(IOV_AMPDU_SEND_ADDBA):
	{
		struct ampdu_ea_tid *ea_tid = (struct ampdu_ea_tid *)a;
		struct scb *scb;
		scb_ampdu_tx_t *scb_ampdu;

		if (!AMPDU_ENAB(wlc->pub) || (ea_tid->tid >= AMPDU_MAX_SCB_TID)) {
			err = BCME_BADARG;
			break;
		}

		if (!(scb = wlc_scbfind(wlc, bsscfg, &ea_tid->ea))) {
			err = BCME_NOTFOUND;
			break;
		}

		scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
		if (!scb_ampdu || !SCB_AMPDU(scb)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (scb_ampdu->ini[ea_tid->tid]) {
			err = BCME_NOTREADY;
			break;
		}

		if (!wlc_ampdu_init_tid_ini(ampdu_tx, scb_ampdu, ea_tid->tid, TRUE)) {
			err = BCME_ERROR;
			break;
		}

		break;
	}

	case IOV_SVAL(IOV_AMPDU_SEND_DELBA):
	{
		struct ampdu_ea_tid *ea_tid = (struct ampdu_ea_tid *)a;
		struct scb *scb;
		scb_ampdu_tx_t *scb_ampdu;

		if (!AMPDU_ENAB(wlc->pub) || (ea_tid->tid >= AMPDU_MAX_SCB_TID)) {
			err = BCME_BADARG;
			break;
		}

		if (!(scb = wlc_scbfind(wlc, bsscfg, &ea_tid->ea))) {
			err = BCME_NOTFOUND;
			break;
		}

		scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
		if (!scb_ampdu || !SCB_AMPDU(scb)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		wlc_ampdu_tx_send_delba(ampdu_tx, scb, ea_tid->tid,
			(ea_tid->initiator == 0 ? FALSE : TRUE),
			DOT11_RC_BAD_MECHANISM);

		break;
	}

	case IOV_GVAL(IOV_AMPDU_MANUAL_MODE):
		*ret_int_ptr = (int32)ampdu_tx_cfg->manual_mode;
		break;

	case IOV_SVAL(IOV_AMPDU_MANUAL_MODE):
		ampdu_tx_cfg->manual_mode = (uint8)int_val;
		break;

#ifdef WLOVERTHRUSTER
	case IOV_SVAL(IOV_AMPDU_TCP_ACK_RATIO):
		wlc_ampdu_tx_set_tcp_ack_ratio(ampdu_tx, (uint8)int_val);
		break;
	case IOV_GVAL(IOV_AMPDU_TCP_ACK_RATIO):
		*ret_int_ptr = (uint32)ampdu_tx->tcp_ack_info.tcp_ack_ratio;
		break;
	case IOV_SVAL(IOV_AMPDU_TCP_ACK_RATIO_DEPTH):
		ampdu_tx->tcp_ack_info.tcp_ack_ratio_depth = (uint8)int_val;
		break;
	case IOV_GVAL(IOV_AMPDU_TCP_ACK_RATIO_DEPTH):
		*ret_int_ptr = (uint32)ampdu_tx->tcp_ack_info.tcp_ack_ratio_depth;
		break;
#endif /* WLOVERTHRUSTER */

	case IOV_GVAL(IOV_AMPDU_MPDU):
		*ret_int_ptr = (int32)ampdu_tx_cfg->max_pdu;
		break;

	case IOV_SVAL(IOV_AMPDU_MPDU):
		if ((!int_val) || (int_val > AMPDU_MAX_MPDU)) {
			err = BCME_RANGE;
			break;
		}
		ampdu_tx_cfg->max_pdu = (int8)int_val;
		scb_ampdu_update_config_all(ampdu_tx);
		break;


#if defined(WL_EXPORT_AMPDU_RETRY)
	case IOV_GVAL(IOV_AMPDU_RETRY_LIMIT_TID):
	{
		struct ampdu_retry_tid *retry_tid = (struct ampdu_retry_tid *)p;

		if (retry_tid->tid >= AMPDU_MAX_SCB_TID) {
			err = BCME_BADARG;
			break;
		}

		retry_tid->retry = ampdu_tx_cfg->retry_limit_tid[retry_tid->tid];
		bcopy(retry_tid, a, sizeof(*retry_tid));
		break;
	}

	case IOV_SVAL(IOV_AMPDU_RETRY_LIMIT_TID):
	{
		struct ampdu_retry_tid *retry_tid = (struct ampdu_retry_tid *)a;

		if (retry_tid->tid >= AMPDU_MAX_SCB_TID ||
			retry_tid->retry > AMPDU_MAX_RETRY_LIMIT) {
			err = BCME_RANGE;
			break;
		}

		ampdu_tx_cfg->retry_limit_tid[retry_tid->tid] = retry_tid->retry;
		break;
	}

	case IOV_GVAL(IOV_AMPDU_RR_RETRY_LIMIT_TID):
	{
		struct ampdu_retry_tid *retry_tid = (struct ampdu_retry_tid *)p;

		if (retry_tid->tid >= AMPDU_MAX_SCB_TID) {
			err = BCME_BADARG;
			break;
		}

		retry_tid->retry = ampdu_tx_cfg->rr_retry_limit_tid[retry_tid->tid];
		bcopy(retry_tid, a, sizeof(*retry_tid));
		break;
	}

	case IOV_SVAL(IOV_AMPDU_RR_RETRY_LIMIT_TID):
	{
		struct ampdu_retry_tid *retry_tid = (struct ampdu_retry_tid *)a;

		if (retry_tid->tid >= AMPDU_MAX_SCB_TID ||
			retry_tid->retry > AMPDU_MAX_RETRY_LIMIT) {
			err = BCME_RANGE;
			break;
		}

		ampdu_tx_cfg->rr_retry_limit_tid[retry_tid->tid] = retry_tid->retry;
		break;
	}
#endif 

#ifdef AMPDU_NON_AQM
#ifdef BCMDBG
	case IOV_GVAL(IOV_AMPDU_FFPLD):
		wlc_ffpld_show(ampdu_tx);
		*ret_int_ptr = 0;
		break;
#endif /* BCMDBG */
#endif /* AMPDU_NON_AQM */

#ifdef BCMDBG
#ifdef WLAMPDU_HW
	case IOV_GVAL(IOV_AMPDU_AGGFIFO):
		*ret_int_ptr = ampdu_tx_cfg->aggfifo_depth;
		break;
	case IOV_SVAL(IOV_AMPDU_AGGFIFO): {
		uint8 depth = (uint8)int_val;
		if (depth < 8 || depth > AGGFIFO_CAP) {
			WL_AMPDU_ERR(("aggfifo depth has to be in [8, %d]\n", AGGFIFO_CAP));
			return BCME_BADARG;
		}
		ampdu_tx_cfg->aggfifo_depth = depth;
	}
		break;
#endif /* WLAMPDU_HW */
#endif /* BCMDBG */
	case IOV_GVAL(IOV_AMPDUMAC):
		*ret_int_ptr = (int32)wlc->pub->_ampdumac;
		break;

	case IOV_GVAL(IOV_AMPDU_AGGMODE):
		*ret_int_ptr = (int32)ampdu_tx_cfg->ampdu_aggmode;
		break;

	case IOV_SVAL(IOV_AMPDU_AGGMODE):
		if (int_val != AMPDU_AGGMODE_AUTO &&
		    int_val != AMPDU_AGGMODE_HOST &&
		    int_val != AMPDU_AGGMODE_MAC) {
			err = BCME_RANGE;
			break;
		}
		ampdu_tx_cfg->ampdu_aggmode = (int8)int_val;
		wlc_ampdu_tx_set(ampdu_tx, wlc->pub->_ampdu_tx);
		break;

#ifdef WLMEDIA_TXFAILEVENT
	case IOV_GVAL(IOV_AMPDU_TXFAIL_EVENT):
		*ret_int_ptr = (int32)ampdu_tx_cfg->tx_fail_event;
		break;

	case IOV_SVAL(IOV_AMPDU_TXFAIL_EVENT):
		ampdu_tx_cfg->tx_fail_event = (bool)int_val;
		break;
#endif /* WLMEDIA_TXFAILEVENT */

	case IOV_GVAL(IOV_AMPDU_FRAMEBURST_OVR):
		*ret_int_ptr = ampdu_tx_cfg->fb_override_enable;
		break;

	case IOV_SVAL(IOV_AMPDU_FRAMEBURST_OVR):
		ampdu_tx_cfg->fb_override_enable = bool_val;
		break;

#ifdef BCMDBG
	case IOV_SVAL(IOV_AMPDU_TXQ_PROFILING_START):
		/* one shot mode, stop once circular buffer is full */
		wlc_ampdu_txq_prof_enab();
		break;

	case IOV_SVAL(IOV_AMPDU_TXQ_PROFILING_DUMP):
		wlc_ampdu_txq_prof_print_histogram(-1);
		break;

	case IOV_SVAL(IOV_AMPDU_TXQ_PROFILING_SNAPSHOT): {
		int tmp = _txq_prof_enab;
		WLC_AMPDU_TXQ_PROF_ADD_ENTRY(ampdu_tx->wlc, NULL);
		wlc_ampdu_txq_prof_print_histogram(1);
		_txq_prof_enab = tmp;
	}
		break;
#endif /* BCMDBG */

#ifdef WLATF
		case IOV_GVAL(IOV_AMPDU_ATF_US):
			*ret_int_ptr = ampdu_tx_cfg->txq_time_allowance_us;
			break;

		case IOV_SVAL(IOV_AMPDU_ATF_US):
			wlc_ampdu_atf_set_default_release_time(ampdu_tx, wlc->scbstate, int_val);
			ampdu_tx_cfg->txq_time_allowance_us = int_val;
			break;

		case IOV_SVAL(IOV_AMPDU_ATF_MIN_US):
			wlc_ampdu_atf_set_default_release_mintime(ampdu_tx, wlc->scbstate, int_val);
			ampdu_tx_cfg->txq_time_min_allowance_us = int_val;
			break;

		case IOV_GVAL(IOV_AMPDU_ATF_MIN_US):
			*ret_int_ptr = ampdu_tx_cfg->txq_time_min_allowance_us = int_val;
			break;
#endif /* WLATF */

	case IOV_GVAL(IOV_AMPDU_RELEASE):
		*ret_int_ptr = (uint32)ampdu_tx_cfg->release;
		break;

	case IOV_SVAL(IOV_AMPDU_RELEASE):
		ampdu_tx_cfg->release = (int8)int_val;
		break;

	case IOV_GVAL(IOV_AMPDU_TXAGGR):
	{
		struct ampdu_aggr *txaggr = p;
		bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, bsscfg);
		bzero(txaggr, sizeof(*txaggr));
		txaggr->aggr_override = bsscfg_ampdu->txaggr_override;
		txaggr->enab_TID_bmap = bsscfg_ampdu->txaggr_TID_bmap;
		bcopy(txaggr, a, sizeof(*txaggr));
		break;
	}
	case IOV_SVAL(IOV_AMPDU_TXAGGR):
	{
		struct ampdu_aggr *txaggr = a;
		uint16  enable_TID_bmap =
			(txaggr->enab_TID_bmap & txaggr->conf_TID_bmap) & AMPDU_ALL_TID_BITMAP;
		uint16  disable_TID_bmap =
			((~txaggr->enab_TID_bmap) & txaggr->conf_TID_bmap) & AMPDU_ALL_TID_BITMAP;

		if (enable_TID_bmap) {
			wlc_ampdu_tx_set_bsscfg_aggr(ampdu_tx, bsscfg, ON, enable_TID_bmap);
		}
		if (disable_TID_bmap) {
			wlc_ampdu_tx_set_bsscfg_aggr(ampdu_tx, bsscfg, OFF, disable_TID_bmap);
		}
		break;
	}
	case IOV_GVAL(IOV_AMPDU_ADDBA_TIMEOUT):
		*ret_int_ptr = (uint32)ampdu_tx_cfg->addba_retry_timeout;
		break;

	case IOV_SVAL(IOV_AMPDU_ADDBA_TIMEOUT):
		ampdu_tx_cfg->addba_retry_timeout = (int16)int_val;
		break;

#ifdef IQPLAY_DEBUG
	case IOV_SVAL(IOV_AMPDU_SET_SEQNUMBER):
		wlc_set_sequmber(wlc, (uint16)int_val);
		OSL_DELAY(30000);
		break;
#endif /* IQPLAY_DEBUG */

	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
} /* wlc_ampdu_doiovar */

/** Minimal spacing between transmit MPDU's in an AMPDU, used to avoid overflow at the receiver */
void
wlc_ampdu_tx_set_mpdu_density(ampdu_tx_info_t *ampdu_tx, uint8 mpdu_density)
{
	ampdu_tx->config->mpdu_density = mpdu_density;
}

/** Limits max outstanding transmit PDU's */
void
wlc_ampdu_tx_set_ba_tx_wsize(ampdu_tx_info_t *ampdu_tx, uint8 wsize)
{
	ampdu_tx->config->ba_tx_wsize = wsize; /* tx ba window size (in pdu) */
}

uint8
wlc_ampdu_tx_get_ba_tx_wsize(ampdu_tx_info_t *ampdu_tx)
{
	return (ampdu_tx->config->ba_tx_wsize);
}

uint8
wlc_ampdu_tx_get_ba_max_tx_wsize(ampdu_tx_info_t *ampdu_tx)
{
	return (ampdu_tx->config->ba_max_tx_wsize);
}

uint8
wlc_ampdu_get_txpkt_weight(ampdu_tx_info_t *ampdu_tx)
{
	return (ampdu_tx->config->txpkt_weight);
}

#ifdef AMPDU_NON_AQM

static void
wlc_ffpld_init(ampdu_tx_config_t *ampdu_tx_cfg)
{
	int i, j;
	wlc_fifo_info_t *fifo;

	for (j = 0; j < NUM_FFPLD_FIFO; j++) {
		fifo = (ampdu_tx_cfg->fifo_tb + j);
		fifo->ampdu_pld_size = 0;
		for (i = 0; i <= AMPDU_HT_MCS_LAST_EL; i++)
			fifo->mcs2ampdu_table[i] = 255;
		fifo->dmaxferrate = 0;
		fifo->accum_txampdu = 0;
		fifo->prev_txfunfl = 0;
		fifo->accum_txfunfl = 0;

	}
}

/**
 * Non AQM only, called when the d11 core signals tx completion, and the tx completion status
 * indicates that a tx underflow condition occurred.
 *
 * Evaluate the dma transfer rate using the tx underflows as feedback.
 * If necessary, increase tx fifo preloading. If not enough,
 * decrease maximum ampdu_tx size for each mcs till underflows stop
 * Return 1 if pre-loading not active, -1 if not an underflow event,
 * 0 if pre-loading module took care of the event.
 */
static int
wlc_ffpld_check_txfunfl(wlc_info_t *wlc, int fid, wlc_bsscfg_t *bsscfg)
{
	ampdu_tx_info_t *ampdu_tx = wlc->ampdu_tx;
	uint32 phy_rate, max_rspec;
	uint32 txunfl_ratio;
	uint8  max_mpdu;
	uint32 current_ampdu_cnt = 0;
	uint16 max_pld_size;
	uint32 new_txunfl;
	wlc_fifo_info_t *fifo = (ampdu_tx->config->fifo_tb + fid);
	uint xmtfifo_sz;
	uint16 cur_txunfl;

	/* return if we got here for a different reason than underflows */
	cur_txunfl = wlc_read_shm(wlc, MACSTAT_ADDR(wlc, MCSTOFF_TXFUNFL + fid));
	new_txunfl = (uint16)(cur_txunfl - fifo->prev_txfunfl);
	if (new_txunfl == 0) {
		WL_FFPLD(("check_txunfl : TX status FRAG set but no tx underflows\n"));
		return -1;
	}
	fifo->prev_txfunfl = cur_txunfl;

	if (!ampdu_tx->config->tx_max_funl)
		return 1;

	/* check if fifo is big enough */
	xmtfifo_sz = wlc->xmtfifo_szh[fid];

	if ((TXFIFO_SIZE_UNIT(wlc->pub->corerev) * (uint32)xmtfifo_sz) <=
	    ampdu_tx->config->ffpld_rsvd)
		return 1;

	max_rspec = wlc_get_current_highest_rate(bsscfg);
	phy_rate = RSPEC2RATE(max_rspec);

	max_pld_size =
	        TXFIFO_SIZE_UNIT(wlc->pub->corerev) * xmtfifo_sz - ampdu_tx->config->ffpld_rsvd;
#ifdef WLCNT
#ifdef WLAMPDU_MAC
	if (AMPDU_MAC_ENAB(wlc->pub) && !AMPDU_AQM_ENAB(wlc->pub))
		ampdu_tx->cnt->txampdu = wlc_read_shm(wlc, MACSTAT_ADDR(wlc, MCSTOFF_TXAMPDU));
#endif
	current_ampdu_cnt = ampdu_tx->cnt->txampdu - fifo->prev_txampdu;
#endif /* WLCNT */
	fifo->accum_txfunfl += new_txunfl;

	/* we need to wait for at least 10 underflows */
	if (fifo->accum_txfunfl < 10)
		return 0;

	WL_FFPLD(("ampdu_count %d  tx_underflows %d\n",
		current_ampdu_cnt, fifo->accum_txfunfl));

	/*
	   compute the current ratio of tx unfl per ampdu.
	   When the current ampdu count becomes too
	   big while the ratio remains small, we reset
	   the current count in order to not
	   introduce too big of a latency in detecting a
	   large amount of tx underflows later.
	*/

	txunfl_ratio = current_ampdu_cnt/fifo->accum_txfunfl;

	if (txunfl_ratio > ampdu_tx->config->tx_max_funl) {
		if (current_ampdu_cnt >= FFPLD_MAX_AMPDU_CNT) {
#ifdef WLCNT
			fifo->prev_txampdu = ampdu_tx->cnt->txampdu;
#endif
			fifo->accum_txfunfl = 0;
		}
		return 0;
	}
	max_mpdu = MIN(fifo->mcs2ampdu_table[AMPDU_MAX_MCS], ampdu_tx->config->default_pdu);

	/* In case max value max_pdu is already lower than
	   the fifo depth, there is nothing more we can do.
	*/

	if (fifo->ampdu_pld_size >= max_mpdu * FFPLD_MPDU_SIZE) {
		WL_FFPLD(("tx fifo pld : max ampdu fits in fifo\n)"));
#ifdef WLCNT
		fifo->prev_txampdu = ampdu_tx->cnt->txampdu;
#endif
		fifo->accum_txfunfl = 0;
		return 0;
	}

	if (fifo->ampdu_pld_size < max_pld_size) {

		/* increment by TX_FIFO_PLD_INC bytes */
		fifo->ampdu_pld_size += FFPLD_PLD_INCR;
		if (fifo->ampdu_pld_size > max_pld_size)
			fifo->ampdu_pld_size = max_pld_size;

		/* update scb release size */
		scb_ampdu_update_config_all(ampdu_tx);

		/*
		   compute a new dma xfer rate for max_mpdu @ max mcs.
		   This is the minimum dma rate that
		   can acheive no unferflow condition for the current mpdu size.
		*/
		/* note : we divide/multiply by 100 to avoid integer overflows */
		fifo->dmaxferrate =
		        (((phy_rate/100)*(max_mpdu*FFPLD_MPDU_SIZE-fifo->ampdu_pld_size))
		         /(max_mpdu * FFPLD_MPDU_SIZE))*100;

		WL_FFPLD(("DMA estimated transfer rate %d; pre-load size %d\n",
		          fifo->dmaxferrate, fifo->ampdu_pld_size));
	} else {

		/* decrease ampdu size */
		if (fifo->mcs2ampdu_table[AMPDU_MAX_MCS] > 1) {
			if (fifo->mcs2ampdu_table[AMPDU_MAX_MCS] == 255)
				fifo->mcs2ampdu_table[AMPDU_MAX_MCS] =
					ampdu_tx->config->default_pdu - 1;
			else
				fifo->mcs2ampdu_table[AMPDU_MAX_MCS] -= 1;

			/* recompute the table */
			wlc_ffpld_calc_mcs2ampdu_table(ampdu_tx, fid, bsscfg);

			/* update scb release size */
			scb_ampdu_update_config_all(ampdu_tx);
		}
	}
#ifdef WLCNT
	fifo->prev_txampdu = ampdu_tx->cnt->txampdu;
#endif
	fifo->accum_txfunfl = 0;
	return 0;
} /* wlc_ffpld_check_txfunfl */

/** non AQM only, tx underflow related */
static void
wlc_ffpld_calc_mcs2ampdu_table(ampdu_tx_info_t *ampdu_tx, int f, wlc_bsscfg_t *bsscfg)
{
	int i;
	uint32 phy_rate, dma_rate, tmp, max_rspec;
	uint8 max_mpdu;
	wlc_fifo_info_t *fifo = (ampdu_tx->config->fifo_tb + f);

	/* recompute the dma rate */
	/* note : we divide/multiply by 100 to avoid integer overflows */
	max_mpdu = MIN(fifo->mcs2ampdu_table[AMPDU_MAX_MCS], ampdu_tx->config->default_pdu);
	max_rspec = wlc_get_current_highest_rate(bsscfg);
	phy_rate = RSPEC2RATE(max_rspec);
	dma_rate = (((phy_rate/100) * (max_mpdu*FFPLD_MPDU_SIZE-fifo->ampdu_pld_size))
	         /(max_mpdu*FFPLD_MPDU_SIZE))*100;
	fifo->dmaxferrate = dma_rate;

	/* fill up the mcs2ampdu table; do not recalc the last mcs */
	dma_rate = dma_rate >> 7;

#if defined(WLPROPRIETARY_11N_RATES)
	i = -1;
	while (TRUE) {
		i = NEXT_MCS(i); /* iterate through both standard and prop ht rates */
		if (i > WLC_11N_LAST_PROP_MCS)
			break;
#else
	for (i = 0; i < AMPDU_MAX_MCS; i++) {
#endif /* WLPROPRIETARY_11N_RATES */
		/* shifting to keep it within integer range */
		phy_rate = MCS_RATE(i, TRUE, FALSE) >> 7;
		if (phy_rate > dma_rate) {
			tmp = ((fifo->ampdu_pld_size * phy_rate) /
				((phy_rate - dma_rate) * FFPLD_MPDU_SIZE)) + 1;
			tmp = MIN(tmp, 255);
			fifo->mcs2ampdu_table[MCS2IDX(i)] = (uint8)tmp;
		}
	}

#ifdef BCMDBG
	wlc_ffpld_show(ampdu_tx);
#endif /* BCMDBG */
} /* wlc_ffpld_calc_mcs2ampdu_table */

#ifdef BCMDBG
/** non AQM only */
static void
wlc_ffpld_show(ampdu_tx_info_t *ampdu_tx)
{
	int i, j;
	wlc_fifo_info_t *fifo;

	WL_PRINT(("MCS to AMPDU tables:\n"));
	for (j = 0; j < NUM_FFPLD_FIFO; j++) {
		fifo = ampdu_tx->config->fifo_tb + j;
		WL_PRINT(("  FIFO %d : Preload settings: size %d dmarate %d kbps\n",
		          j, fifo->ampdu_pld_size, fifo->dmaxferrate));
		for (i = 0; i <= AMPDU_HT_MCS_LAST_EL; i++) {
			WL_PRINT(("  %d", fifo->mcs2ampdu_table[i]));
		}
		WL_PRINT(("\n\n"));
	}
}
#endif /* BCMDBG */
#endif /* !WLAMPDU_AQM */


/**
 * enable/disable txaggr_override control.
 * AUTO: txaggr operates according to per-TID per-bsscfg control()
 * OFF: turn txaggr off for all TIDs.
 * ON: Not supported and treated the same as AUTO.
 */
void
wlc_ampdu_tx_set_bsscfg_aggr_override(ampdu_tx_info_t *ampdu_tx, wlc_bsscfg_t *bsscfg, int8 txaggr)
{
	bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, bsscfg);

	if (ampdu_tx->txaggr_support == FALSE) {
		/* txaggr_override should already be OFF */
		ASSERT(bsscfg_ampdu->txaggr_override == OFF);
		return;
	}

	/* txaggr_override ON would mean that tx aggregation will be allowed for all TIDs
	 * even if bsscfg_ampdu->txaggr_TID_bmap is set OFF for some TIDs.
	 * As there is no requirement of such txaggr_override ON, just treat it as AUTO.
	 */
	if (txaggr == ON) {
		txaggr = AUTO;
	}

	if (bsscfg_ampdu->txaggr_override == txaggr) {
		return;
	}

	bsscfg_ampdu->txaggr_override = txaggr;

	if (txaggr == OFF) {
		wlc_ampdu_tx_cleanup(ampdu_tx, bsscfg, AMPDU_ALL_TID_BITMAP);
	}
}

/** returns a bitmap */
uint16
wlc_ampdu_tx_get_bsscfg_aggr(ampdu_tx_info_t *ampdu_tx, wlc_bsscfg_t *bsscfg)
{
	bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, bsscfg);

	if (ampdu_tx->txaggr_support == FALSE) {
		/* txaggr should be OFF for all TIDs */
		ASSERT(bsscfg_ampdu->txaggr_TID_bmap == 0);
	}
	return (bsscfg_ampdu->txaggr_TID_bmap & AMPDU_ALL_TID_BITMAP);
}

/** Configure ampdu tx aggregation per-TID and per-bsscfg */
void
wlc_ampdu_tx_set_bsscfg_aggr(ampdu_tx_info_t *ampdu_tx, wlc_bsscfg_t *bsscfg,
	bool txaggr, uint16 conf_TID_bmap)
{
	bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, bsscfg);

	if (ampdu_tx->txaggr_support == FALSE) {
		/* txaggr should already be OFF for all TIDs,
		 * and do not set txaggr_TID_bmap.
		 */
		ASSERT(bsscfg_ampdu->txaggr_TID_bmap == 0);
		return;
	}

	if (txaggr == ON) {
		bsscfg_ampdu->txaggr_TID_bmap |= (conf_TID_bmap & AMPDU_ALL_TID_BITMAP);
	} else {
		uint16 stateChangedTID = bsscfg_ampdu->txaggr_TID_bmap;
		bsscfg_ampdu->txaggr_TID_bmap &= ((~conf_TID_bmap) & AMPDU_ALL_TID_BITMAP);
		stateChangedTID ^= bsscfg_ampdu->txaggr_TID_bmap;
		stateChangedTID &= AMPDU_ALL_TID_BITMAP;

		/* Override should have higher priority if not AUTO */
		if (bsscfg_ampdu->txaggr_override == AUTO && stateChangedTID) {
			wlc_ampdu_tx_cleanup(ampdu_tx, bsscfg, stateChangedTID);
		}
	}
}

/**
 * Enable A-MPDU aggregation for the specified remote party ('scb').
 *
 * This function is used to enable the Tx A-MPDU aggregation path for a given SCB
 * and is called by the AMPDU common config code.
 *
 * NOTE: this code hides the difference between TxQ MUX implementation and the previous Tx data path
 *       model.
 */
void
wlc_ampdu_tx_scb_enable(ampdu_tx_info_t *ampdu_tx, struct scb *scb)
{
	wlc_info_t *wlc = ampdu_tx->wlc;

	/* NOTE: the TxQ MUX implementation and previous TxQ path implement the AMPDU
	 * path differently.
	 * In the previous Tx implemenation, AMPDU inserted a TxModule
	 * to process and queue ampdu packets to an SCB ampdu queue before releasing the packets
	 * onto the driver TxQ. Then as the TxQ is processed by wlc_pull_q() (formerly wlc_sendq()),
	 * WPF_AMPDU_MPDU tagged packets were processed by wlc_sendampdu().
	 * This function configures the TXMOD_AMPDU for the given SCB to enable the AMPDU path.
	 *
	 * In the TxQ MUX implementation, there is no AMPDU TxModule. Instead packets always flow
	 * onto the SCBQs of each SCB. This function enables the AMPDU Tx path by configuring the
	 * the SCBQ MUX source output function with the the ampdu-specific output function.
	 */

#if defined(TXQ_MUX)
#if defined(WLAMPDU_MAC) /* ucode, ucode hw assisted or AQM aggregation */
	/* enable the MUX output fn for aggregation */
	if (D11REV_GE(wlc->pub->corerev, 40)) {
		/* enable the MUX output fn for AQM aggregation */

		scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
		wlc_scbq_set_output_fn(wlc->tx_scbq, scb, scb_ampdu, wlc_ampdu_output_aqm);
	} else
#endif /* WLAMPDU_MAC */
	{

		BCM_REFERENCE(wlc);
		WL_AMPDU_ERR(("wl%d: TXQ_MUX not ready for non-aqm ampdu\n",
		          wlc->pub->unit));
	}

#else
	/* to enable AMPDU, configure the AMPDU TxModule for this SCB */
	wlc_txmod_config(wlc->txmodi, scb, TXMOD_AMPDU);
#endif /* TXQ_MUX */
} /* wlc_ampdu_tx_scb_enable */

/**
 * Disable A-MPDU aggregation for the specified remote party 'scb'.
 *
 * This function is used to disable the Tx A-MPDU aggregation path for a given SCB
 * and is called by the AMPDU common config code.
 *
 * NOTE: this code hides the difference between TxQ MUX implementaion and the
 * previous Tx data path model.
 */
void
wlc_ampdu_tx_scb_disable(ampdu_tx_info_t *ampdu_tx, struct scb *scb)
{
	wlc_info_t *wlc = ampdu_tx->wlc;

	/* NOTE: the TxQ MUX implementation and previous TxQ path implement the AMPDU
	 * path differently.
	 * In the previous Tx implemenation, AMPDU inserted a TxModule
	 * to process and queue ampdu packets to an SCB ampdu queue before releasing the packets
	 * onto the driver TxQ. Then as the TxQ is processed by wlc_pull_q() (formerly wlc_sendq()),
	 * WPF_AMPDU_MPDU tagged packets were processed by wlc_sendampdu().
	 * This function un-configures the TXMOD_AMPDU for the given SCB to disable the AMPDU path.
	 *
	 * In the TxQ MUX implementation, there is no AMPDU TxModule. Instead packets always flow
	 * onto the SCBQs of each SCB. This function disables the AMPDU Tx path by resetting
	 * the SCBQ MUX source output function to the regular non-ampdu output function.
	 */

#if defined(TXQ_MUX)
	/* reset the MUX output fn to the non-agg fn */
	if (D11REV_GE(wlc->pub->corerev, 40)) {
		wlc_scbq_reset_output_fn(wlc->tx_scbq, scb);
	}
#else
	/* to disable AMPDU, unconfigure the AMPDU TxModule for this SCB */
	wlc_txmod_unconfig(wlc->txmodi, scb, TXMOD_AMPDU);
#endif /* TXQ_MUX */
}

/*
 * Changes txaggr btcx_dur_flag global flag
 */
void
wlc_ampdu_btcx_tx_dur(wlc_info_t *wlc, bool btcx_dur_flag)
{
	ampdu_tx_config_t *ampdu_tx_cfg = wlc->ampdu_tx->config;
	ampdu_tx_cfg->btcx_dur_flag = btcx_dur_flag;
}


/**
 * Called after a higher software layer gave this AMPDU layer one or more MSDUs,
 * and the AMPDU layer put the MSDU on an AMPDU internal software queue.
 */
INLINE static void
wlc_ampdu_agg_complete(wlc_info_t *wlc, ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini, bool force_release)
{
	BCM_REFERENCE(wlc);

	wlc_ampdu_txflowcontrol(wlc, scb_ampdu, ini);
	wlc_ampdu_txeval(ampdu_tx, scb_ampdu, ini, force_release);
}

#if (defined(PKTC) || defined(PKTC_TX_DONGLE))

/**
 * AMPDU aggregation function called through txmod
 *    @param[in] p     a chain of 802.3 ? packets
 */

static void BCMFASTPATH
wlc_ampdu_agg_pktc(void *ctx, struct scb *scb, void *p, uint prec)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)ctx;
	scb_ampdu_tx_t *scb_ampdu;
	wlc_info_t *wlc;
	scb_ampdu_tid_ini_t *ini;
	uint8 tid;
	int bcmerror = BCME_OK;
	void *n;
	uint32 lifetime = 0;
	uint32 drop_cnt = 0;
	bool amsdu_in_ampdu;
	bool force_release = FALSE;
	bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, SCB_BSSCFG(scb));

	ASSERT(ampdu_tx);
	wlc = ampdu_tx->wlc;

#ifdef DMATXRC
	if (!DMATXRC_ENAB(wlc->pub))
#endif
		ASSERT((PKTNEXT(wlc->osh, p) == NULL) || !PKTISCHAINED(p) ||
				PKTISFRWDPKT(wlc->osh, p));

	tid = (uint8)PKTPRIO(p);

	ASSERT(tid < AMPDU_MAX_SCB_TID);
	if (tid >= AMPDU_MAX_SCB_TID) {
		WL_AMPDU(("%s: Received tid is wrong -- returning\n", __FUNCTION__));
		goto txmod_ampdu;
	}

	/* Set packet exptime */
	lifetime = wlc->lifetime[(SCB_WME(scb) ? WME_PRIO2AC(tid) : AC_BE)];

	if ((bsscfg_ampdu->txaggr_override == OFF) ||
		(!isbitset(bsscfg_ampdu->txaggr_TID_bmap, tid)) ||
		(WLPKTTAG(p)->flags3 & WLF3_BYPASS_AMPDU)) {
		WL_AMPDU(("%s: tx agg off (txaggr ovrd %d, TID bmap 0x%x)-- returning\n",
			__FUNCTION__,
			bsscfg_ampdu->txaggr_override,
			bsscfg_ampdu->txaggr_TID_bmap));
		goto txmod_ampdu;
	}

	wlc_wme_wmmac_check_fixup(wlc, scb, p, tid, &bcmerror);
	if (bcmerror) {
		/* let the main prep code drop the frame and account it properly */
		goto txmod_ampdu;
	}

	/* Update potentially new tid */
	tid = (uint8)PKTPRIO(p);

	ASSERT(tid < AMPDU_MAX_SCB_TID);
	if (tid >= AMPDU_MAX_SCB_TID) {
		WL_AMPDU(("%s: New tid is wrong -- returning\n", __FUNCTION__));

		/* Reset the Lifetime value if tid is invalid\n" */
		lifetime = 0;

		goto txmod_ampdu;
	}

	/* Set packet expiration time */
	lifetime = wlc->lifetime[WME_PRIO2AC(tid)];

	ASSERT(SCB_AMPDU(scb));
	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	/* initialize initiator on first packet; sends addba req */
	if (!(ini = scb_ampdu->ini[tid])) {
		ini = wlc_ampdu_init_tid_ini(ampdu_tx, scb_ampdu, tid, FALSE);
		if (!ini) {
			WL_AMPDU(("%s: ini NULL -- returning\n", __FUNCTION__));
			goto txmod_ampdu;
		}
	}

	if (ini->ba_state != AMPDU_TID_STATE_BA_ON)
		goto txmod_ampdu;

#ifdef PROP_TXSTATUS
	/* If FW packet received when suppr_window > 0, send it as non-ampdu */
	if (AMPDU_HOST_ENAB(wlc->pub) && PROP_TXSTATUS_ENAB(wlc->pub) &&
		(ini->suppr_window > 0) &&
		!(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(p)->wl_hdr_information) &
		WLFC_PKTFLAG_PKTFROMHOST)) {
		goto txmod_ampdu;
	}
#endif /* PROP_TXSTATUS */

	/* using bit-wise logic here for speed optimization; only works for boolean 0/1 values */
	amsdu_in_ampdu = (AMSDU_TX_ENAB(wlc->pub) & (SCB_AMSDU_IN_AMPDU(scb) != 0) &
	                  (SCB_AMSDU(scb) != 0) &
#ifdef WLCNTSCB
	                  RSPEC_ISVHT(scb->scb_stats.tx_rate) &
#else
	                  FALSE &
#endif
	                  TRUE);

	while (1) { /* loop over each packet in the caller supplied chain */
		ASSERT(PKTISCHAINED(p) || (PKTCLINK(p) == NULL));
		n = PKTCLINK(p);
		PKTSETCLINK(p, NULL); /* 'unlink' the current packet */

#ifdef PROP_TXSTATUS
	/* If Non AMPDU packet was suppresed, send it as Non AMPDU again */
	if (AMPDU_HOST_ENAB(wlc->pub) && PROP_TXSTATUS_ENAB(wlc->pub) &&
		IS_WL_TO_REUSE_SEQ(WLPKTTAG(p)->seq) &&
		MODSUB_POW2(WL_SEQ_GET_NUM(WLPKTTAG(p)->seq), ini->start_seq, SEQNUM_MAX) >=
		ini->ba_wsize) {
		PKTCLRCHAINED(wlc->osh, p);
		if (n != NULL)
			wlc_pktc_sdu_prep(wlc, scb, p, n, lifetime);
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
		p = n;
		if (p)
			continue;
		else
			return;
	}
#endif /* PROP_TXSTATUS */

		/* Avoid fn call if the packet is too long to be an ACK */
		if (pkttotlen(wlc->osh, p) <= TCPACKSZSDU) {
#ifdef WLOVERTHRUSTER
			if (OVERTHRUST_ENAB(wlc->pub)) {
				wlc_ampdu_tcp_ack_suppress(ampdu_tx, scb_ampdu, p, tid);
			}
#else
			if (wlc_ampdu_is_tcpack(ampdu_tx, p))
				WLPKTTAG(p)->flags3 |= WLF3_DATA_TCP_ACK;
#endif
		}

#ifdef PROP_TXSTATUS
		/* If any suppress pkt in the chain, force ampdu release (=push to hardware) */
		if (IS_WL_TO_REUSE_SEQ(WLPKTTAG(p)->seq)) {
			force_release = TRUE;
		}
#endif /* PROP_TXSTATUS */

		/* If any pkt in the chain is TCPACK, force ampdu release */
		if (ampdu_tx->wlc->tcpack_fast_tx && (WLPKTTAG(p)->flags3 & WLF3_DATA_TCP_ACK))
			force_release = TRUE;

		/* Queue could overflow while walking the chain */
		if (!wlc_ampdu_prec_enq(wlc, &scb_ampdu->txq, p, tid)) {
			PKTSETCLINK(p, n);
			wlc_ampdu_agg_complete(wlc, ampdu_tx, scb_ampdu, ini, force_release);
			break;
		}

		PKTCLRCHAINED(wlc->osh, p);
		/* Last packet in chain */
		if ((n == NULL) ||
			(wlc_pktc_sdu_prep(wlc, scb, p, n, lifetime),
			(amsdu_in_ampdu &&
#ifdef WLOVERTHRUSTER
			(OVERTHRUST_ENAB(wlc->pub)?(pkttotlen(wlc->osh, p) > TCPACKSZSDU):1) &&
#endif
#ifdef WLAMSDU_TX
			((n = wlc_amsdu_pktc_agg(wlc->ami, scb,
			p, /* chain going to be extended with AMSDUs contained in 'n' */
			n, /* chain containing MSDUs to aggregate to 'p'  */
			tid, lifetime)) == NULL) &&
#endif
			TRUE))) {
			wlc_ampdu_agg_complete(wlc, ampdu_tx, scb_ampdu, ini, force_release);
			return;
		}

#ifdef DMATXRC
		if (DMATXRC_ENAB(wlc->pub)) {
			void *phdr;

			/* Check next chain (n) to prepend phdr (PHDR_POOL) */
			phdr = wlc_pktc_sdu_prep_phdr(wlc, scb, n, lifetime);
			if (phdr == NULL) {
				/* No phdr so leave n alone to begin next chain */
				WLCNTINCR(wlc->pub->_cnt->txnobuf);
			} else {
				/* n has prepended phdr */
				n = phdr;
			}
		}
#endif /* DMATXRC */

		p = n; /* advances to next packet in caller supplied chain */
	} /* while (packets) */

	WL_AMPDU_ERR(("wl%d: %s: txq overflow\n", wlc->pub->unit, __FUNCTION__));

	FOREACH_CHAINED_PKT(p, n) { /* part of error flow handling */
		PKTCLRCHAINED(wlc->osh, p);
		PKTFREE(wlc->osh, p, TRUE);
		WLCIFCNTINCR(scb, txnobuf);
		drop_cnt++;
	}

	if (drop_cnt) {
#ifdef PKTQ_LOG
		struct pktq *q = &scb_ampdu->txq;
		if (q->pktqlog) {
			pktq_counters_t *prec_cnt = wlc_ampdu_pktqlog_cnt(q, tid, prec);
			WLCNTCONDADD(prec_cnt, prec_cnt->dropped, drop_cnt - 1);
		}
#endif
		WLCNTADD(wlc->pub->_cnt->txnobuf, drop_cnt);
		WLCNTADD(ampdu_tx->cnt->txdrop, drop_cnt);
		AMPDUSCBCNTADD(scb_ampdu->cnt.txdrop, drop_cnt);
		WLCNTSCBADD(scb->scb_stats.tx_failures, drop_cnt);
	}

	return;

txmod_ampdu:
	FOREACH_CHAINED_PKT(p, n) {
		PKTCLRCHAINED(wlc->osh, p);
		if (n != NULL)
			wlc_pktc_sdu_prep(wlc, scb, p, n, lifetime);
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
	}
} /* wlc_ampdu_agg_pktc */

#else /* PKTC || PKTC_TX_DONGLE */

/**
 * A higher layer calls this callback function (using the 'txmod' infrastructure) to queue an MSDU
 * on a software queue in the AMPDU subsystem (scb_ampdu->txq). The MSDU is not forwarded to the d11
 * core in this function.
 */
static void BCMFASTPATH
wlc_ampdu_agg(void *ctx, struct scb *scb, void *p, uint prec)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)ctx;
	scb_ampdu_tx_t *scb_ampdu;
	wlc_info_t *wlc;
	scb_ampdu_tid_ini_t *ini;
	uint8 tid;
	int bcmerror = BCME_OK;
	bool force_release = FALSE;
	bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, SCB_BSSCFG(scb));

	ASSERT(ampdu_tx);
	wlc = ampdu_tx->wlc;

	tid = (uint8)PKTPRIO(p);

	if ((bsscfg_ampdu->txaggr_override == OFF) ||
		(!isbitset(bsscfg_ampdu->txaggr_TID_bmap, tid)) ||
		(WLPKTTAG(p)->flags3 & WLF3_BYPASS_AMPDU)) {
		WL_AMPDU(("%s: tx agg off (txaggr ovrd %d, TID bmap 0x%x)-- returning\n",
			__FUNCTION__,
			bsscfg_ampdu->txaggr_override,
			bsscfg_ampdu->txaggr_TID_bmap));
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
		return;
	}

	wlc_wme_wmmac_check_fixup(wlc, scb, p, tid, &bcmerror);
	if (bcmerror) {
		/* let the main prep code drop the frame and account it properly */
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
		return;
	}
	/* Update potentially new tid */
	tid = (uint8)PKTPRIO(p);

	ASSERT(tid < AMPDU_MAX_SCB_TID);
	if (tid >= AMPDU_MAX_SCB_TID) {
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
		WL_AMPDU(("%s: tid wrong -- returning\n", __FUNCTION__));
		return;
	}

	ASSERT(SCB_AMPDU(scb));
	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	/* initialize initiator on first packet; sends addba req */
	if (!(ini = scb_ampdu->ini[tid])) {
		ini = wlc_ampdu_init_tid_ini(ampdu_tx, scb_ampdu, tid, FALSE);
		if (!ini) {
			SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
			WL_AMPDU(("%s: ini NULL -- returning\n", __FUNCTION__));
			return;
		}
	}

	if (ini->ba_state != AMPDU_TID_STATE_BA_ON) {
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
		return;
	}

	/* Avoid fn call if the packet is too long to be an ACK */
	if (PKTLEN(wlc->osh, p) <= TCPACKSZSDU) {
#ifdef WLOVERTHRUSTER
		if (OVERTHRUST_ENAB(wlc->pub)) {

#ifdef WLMESH
			if (((WLPKTTAG(p)->flags & WLF_MESH_RETX) == 0) &&
				pkttotlen(wlc->osh, p) <=
					(ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN + 120))
#endif
				wlc_ampdu_tcp_ack_suppress(ampdu_tx, scb_ampdu, p, tid);
		}
#else
		if (wlc_ampdu_is_tcpack(ampdu_tx, p))
			WLPKTTAG(p)->flags3 |= WLF3_DATA_TCP_ACK;
#endif /* !WLOVERTHRUSTER */
	}

#ifdef PROP_TXSTATUS
	/* If any suppress pkt in the chain, force ampdu release */
	if (IS_WL_TO_REUSE_SEQ(WLPKTTAG(p)->seq)) {
		force_release = TRUE;
	}
#endif /* PROP_TXSTATUS */

	/* If any TCPACK pkt, force ampdu release */
	if (ampdu_tx->wlc->tcpack_fast_tx && (WLPKTTAG(p)->flags3 & WLF3_DATA_TCP_ACK))
		force_release = TRUE;

	if (wlc_ampdu_prec_enq(ampdu_tx->wlc, &scb_ampdu->txq, p, tid)) { /* on sdu queue */
		wlc_ampdu_agg_complete(wlc, ampdu_tx, scb_ampdu, ini, force_release);
		return;
	}

	WL_AMPDU_ERR(("wl%d: %s: txq overflow\n", wlc->pub->unit, __FUNCTION__));


	wlc_tx_status_update_counters(wlc, p,
		scb, NULL, WLC_TX_STS_TX_Q_FULL, 1);

	PKTFREE(wlc->osh, p, TRUE);
	WLCNTINCR(wlc->pub->_cnt->txnobuf);
	WLCNTINCR(ampdu_tx->cnt->txdrop);
	AMPDUSCBCNTINCR(scb_ampdu->cnt.txdrop);
	WLCIFCNTINCR(scb, txnobuf);
	WLCNTSCBINCR(scb->scb_stats.tx_failures);
#ifdef WL11K
	wlc_rrm_stat_qos_counter(scb, tid, OFFSETOF(rrm_stat_group_qos_t, txdrop));
	wlc_rrm_stat_qos_counter(scb, tid, OFFSETOF(rrm_stat_group_qos_t, txfail));
#endif
} /* wlc_ampdu_agg */
#endif /* PKTC || PKTC_TX_DONGLE */

#ifdef WLOVERTHRUSTER

uint
wlc_ampdu_tx_get_tcp_ack_ratio(ampdu_tx_info_t *ampdu_tx)
{
	return (uint)ampdu_tx->tcp_ack_info.tcp_ack_ratio;
}

static void
wlc_ampdu_tx_set_tcp_ack_ratio(ampdu_tx_info_t *ampdu_tx, uint8 tcp_ack_ratio)
{
#if defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB()) {
		BCM_REFERENCE(tcp_ack_ratio);
		ampdu_tx->tcp_ack_info.tcp_ack_ratio = 0;
	} else
#endif /* BCMPCIEDEV */
	{
		ampdu_tx->tcp_ack_info.tcp_ack_ratio = tcp_ack_ratio;
	}
}

/** overthruster related. */
static void BCMFASTPATH
wlc_ampdu_tcp_ack_suppress(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
                           void *p, uint8 tid)
{
	wlc_tcp_ack_info_t *tcp_info = &(ampdu_tx->tcp_ack_info);
	uint8 *ip_header;
	uint8 *tcp_header;
	uint32 ack_num;
	void *prev_p;
	uint16 totlen;
	uint32 ip_hdr_len;
	uint32 tcp_hdr_len;
	uint32 pktlen;
	uint16 ethtype;
	uint8 prec;
	osl_t *osh = ampdu_tx->wlc->pub->osh;

	/* Even with tcp_ack_ratio set to 0, we want to examine the packet
	 * to find TCP ACK packet in case tcpack_fast_tx is true
	 */
	if ((tcp_info->tcp_ack_ratio == 0) && !ampdu_tx->wlc->tcpack_fast_tx)
		return;

	ethtype = wlc_sdu_etype(ampdu_tx->wlc, p);
	if (ethtype != ntoh16(ETHER_TYPE_IP))
		return;

	pktlen = pkttotlen(osh, p);

	ip_header = wlc_sdu_data(ampdu_tx->wlc, p);
	ip_hdr_len = 4*(ip_header[0] & 0x0f);

	if (pktlen < ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN + ip_hdr_len)
		return;

	if (ip_header[9] != 0x06)
		return;

	tcp_header = ip_header + ip_hdr_len;

#ifdef WLAMPDU_PRECEDENCE
	prec = WLC_PRIO_TO_PREC(tid);
#else
	prec = tid;
#endif

	if (tcp_header[13] & 0x10) {
		totlen =  (ip_header[3] & 0xff) + (ip_header[2] << 8);
		tcp_hdr_len = 4*((tcp_header[12] & 0xf0) >> 4);

		if (totlen ==  ip_hdr_len + tcp_hdr_len) {
			tcp_info->tcp_ack_total++;
			if (ampdu_tx->tcp_ack_info.tcp_ack_ratio_depth) {
				WLPKTTAG(p)->flags3 |= WLF3_FAVORED;
#ifdef WLAMPDU_PRECEDENCE
				prec = WLC_PRIO_TO_HI_PREC(tid);
#endif
			}
			if (tcp_header[13] == 0x10) {
				WLPKTTAG(p)->flags3 |= WLF3_DATA_TCP_ACK;
				if (tcp_info->current_dequeued >= tcp_info->tcp_ack_ratio) {
					tcp_info->current_dequeued = 0;
					return;
				}
			} else if (ampdu_tx->wlc->tcpack_fast_tx) {
				/* Here, we want to set TCP ACK flag even to SYN ACK packets,
				 * as we are not in this function to suppress any packet.
				 */
				WLPKTTAG(p)->flags3 |= WLF3_DATA_TCP_ACK;
				return;
			}
		}
	}

	if (!(WLPKTTAG(p)->flags3 & WLF3_DATA_TCP_ACK) ||
		tcp_info->tcp_ack_ratio == 0) {
		return;
	}

	ack_num = (tcp_header[8] << 24) + (tcp_header[9] << 16) +
	        (tcp_header[10] << 8) +  tcp_header[11];

	if ((prev_p = pktqprec_peek_tail(&scb_ampdu->txq, prec)) &&
		(WLPKTTAG(prev_p)->flags3 & WLF3_DATA_TCP_ACK) &&
		!(WLPKTTAG(prev_p)->flags & WLF_AMSDU)) {
		uint8 *prev_ip_hdr = wlc_sdu_data(ampdu_tx->wlc, prev_p);
		uint8 *prev_tcp_hdr = prev_ip_hdr + 4*(prev_ip_hdr[0] & 0x0f);
		uint32 prev_ack_num = (prev_tcp_hdr[8] << 24) +
			(prev_tcp_hdr[9] << 16) +
			(prev_tcp_hdr[10] << 8) +  prev_tcp_hdr[11];

#ifdef PROP_TXSTATUS
		/* Don't drop suppress packets */
		if (IS_WL_TO_REUSE_SEQ(WLPKTTAG(prev_p)->seq)) {
			return;
		}
#endif /* PROP_TXSTATUS */

		if ((ack_num > prev_ack_num) &&
			!memcmp(prev_ip_hdr+12, ip_header+12, 8) &&
			!memcmp(prev_tcp_hdr, tcp_header, 4)) {
			prev_p = pktq_pdeq_tail(&scb_ampdu->txq, prec);
			PKTFREE(ampdu_tx->wlc->pub->osh, prev_p, TRUE);
			tcp_info->tcp_ack_dequeued++;
			tcp_info->current_dequeued++;
			return;
		}
	}

	if (pktqprec_n_pkts(&scb_ampdu->txq, prec) < ampdu_tx->tcp_ack_info.tcp_ack_ratio_depth) {
		int count = 0;
		void *previous_p = NULL;
		prev_p = pktqprec_peek(&scb_ampdu->txq, prec);

		while (prev_p && (count < ampdu_tx->tcp_ack_info.tcp_ack_ratio_depth)) {
			if ((WLPKTTAG(prev_p)->flags3 & WLF3_DATA_TCP_ACK) &&
			    !(WLPKTTAG(prev_p)->flags & WLF_AMSDU)) {
				uint8 *prev_ip_hdr = wlc_sdu_data(ampdu_tx->wlc, prev_p);
				uint8 *prev_tcp_hdr = prev_ip_hdr +
					4*(prev_ip_hdr[0] & 0x0f);
				uint32 prev_ack_num = (prev_tcp_hdr[8] << 24) +
					(prev_tcp_hdr[9] << 16) +
					(prev_tcp_hdr[10] << 8) +  prev_tcp_hdr[11];

#ifdef PROP_TXSTATUS
				/* Don't drop suppress packets */
				if (IS_WL_TO_REUSE_SEQ(WLPKTTAG(prev_p)->seq)) {
					return;
				}
#endif /* PROP_TXSTATUS */
				/* is it the same dest/src IP addr, port # etc ??
				 * IPs: compare 8 bytes at IP hdrs offset 12
				 * compare tcp hdrs for 8 bytes : includes both ports and
				 * sequence number.
				 */
				if ((ack_num > prev_ack_num) &&
					(!memcmp(prev_ip_hdr+12, ip_header+12, 8)) &&
					(!memcmp(prev_tcp_hdr, tcp_header, 4))) {
					if (!previous_p) {
						prev_p = pktq_pdeq(&scb_ampdu->txq, prec);
					} else {
						prev_p = pktq_pdeq_prev(&scb_ampdu->txq,
							prec, previous_p);
					}
					if (prev_p) {
						PKTFREE(ampdu_tx->wlc->pub->osh,
							prev_p, TRUE);
						tcp_info->tcp_ack_multi_dequeued++;
						tcp_info->current_dequeued++;
					}
					return;
				}
			}
			previous_p = prev_p;
			prev_p = PKTLINK(prev_p);
			count++;
		}
	}

	tcp_info->current_dequeued = 0;
} /* wlc_ampdu_tcp_ack_suppress */

#else /* WLOVERTHRUSTER */

/** send TCP ACKs earlier to increase throughput */
static bool BCMFASTPATH
wlc_ampdu_is_tcpack(ampdu_tx_info_t *ampdu_tx, void *p)
{
	uint8 *ip_header;
	uint8 *tcp_header;
	uint16 totlen;
	uint32 ip_hdr_len;
	uint32 tcp_hdr_len;
	uint32 pktlen;
	uint16 ethtype;
	osl_t *osh = ampdu_tx->wlc->pub->osh;
	bool ret = FALSE;

	/* No reason to find TCP ACK packets */
	if (!ampdu_tx->wlc->tcpack_fast_tx)
		goto done;

	ethtype = wlc_sdu_etype(ampdu_tx->wlc, p);
	if (ethtype != ntoh16(ETHER_TYPE_IP))
		goto done;

	pktlen = pkttotlen(osh, p);

	ip_header = wlc_sdu_data(ampdu_tx->wlc, p);
	ip_hdr_len = 4*(ip_header[0] & 0x0f);

	if (pktlen < ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN + ip_hdr_len)
		goto done;

	if (ip_header[9] != 0x06)
		goto done;

	tcp_header = ip_header + ip_hdr_len;

	if (tcp_header[13] & 0x10) {
		totlen =  (ip_header[3] & 0xff) + (ip_header[2] << 8);
		tcp_hdr_len = 4*((tcp_header[12] & 0xf0) >> 4);

		if (totlen ==  ip_hdr_len + tcp_hdr_len)
			ret = TRUE;
	}

done:
	return ret;
} /* wlc_ampdu_is_tcpack */

#endif /* WLOVERTHRUSTER */

/** Find basic rate for a given rate */
#define AMPDU_BASIC_RATE(band, rspec)	((band)->basic_rate[RSPEC_REFERENCE_RATE(rspec)])

#ifdef WLATF
/**
 * Air Time Fairness. Calculate number of bytes to be released in a time window at the current tx
 * rate for the maximum and minimum txq release time window.
 */
static atf_rbytes_t* BCMFASTPATH
wlc_ampdu_atf_calc_rbytes(atf_state_t *atf_state, ratespec_t rspec)
{
	ampdu_tx_info_t *ampdu_tx;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	struct scb *scb;
	wlc_info_t *wlc;
	bool gProt;
	bool nProt;
	uint ctl_rspec;
	uint ack_rspec;
	uint pkt_overhead_us;
	uint max_pdu_time_us;
	uint ampdu_rate_kbps;
	wlc_bsscfg_t *bsscfg;
	wlcband_t *band;
	uint dur;
	uint8 max_pdu;
	uint subframe_time_us;
	uint adjusted_max_pdu;
	uint flags;
#ifdef WLCNT
	atf_stats_t *atf_stats = &atf_state->atf_stats;
#endif

	ampdu_tx = atf_state->ampdu_tx;
	scb_ampdu = atf_state->scb_ampdu;
	ini = atf_state->ini;
	scb = ini->scb;
	wlc = ampdu_tx->wlc;

	ASSERT(scb->bsscfg);

	bsscfg = scb->bsscfg;
	band = atf_state->band;
	dur = ampdu_tx->config->dur;
	max_pdu = scb_ampdu->max_pdu;
	flags = WLC_AIRTIME_AMPDU;

	flags |= (WLC_AIRTIME_MIXEDMODE);

	gProt = (band->gmode && !RSPEC_ISCCK(rspec) && WLC_PROT_G_CFG_G(wlc->prot_g, bsscfg));
	nProt =	((WLC_PROT_N_CFG_N(wlc->prot_n, bsscfg) == WLC_N_PROTECTION_20IN40) &&
		RSPEC_IS40MHZ(rspec));

	if (WLC_HT_GET_AMPDU_RTS(wlc->hti) || nProt || gProt)
		flags |= (WLC_AIRTIME_RTSCTS);

	ack_rspec = AMPDU_BASIC_RATE(band, rspec);

	/* Short preamble */
	if (WLC_PROT_CFG_SHORTPREAMBLE(wlc->prot, bsscfg) &&
		(scb->flags & SCB_SHORTPREAMBLE) && RSPEC_ISCCK(ack_rspec)) {
			ack_rspec |= RSPEC_SHORT_PREAMBLE;
	}

	if (gProt) {
		/*
		  * Use long preamble and long slots if Protection bit is set
		  * per Sect 9.23.2 IEEE802.11-2012
		  * IEEE Clause 16 stuff is all long slot
		  *
		  * Use 11Mbps or lower for ACKs in 2G Band
		  */
		ctl_rspec = AMPDU_BASIC_RATE(band, LEGACY_RSPEC(WLC_RATE_11M));
	} else {
		ctl_rspec = ack_rspec;
	}

	/*
	 * Short slot for ERP STAs
	 * The AP bsscfg will keep track of all sta's shortslot/longslot cap,
	 * and keep current_bss->capability up to date.
	 */
	if (BAND_5G(band->bandtype) || bsscfg->current_bss->capability & DOT11_CAP_SHORTSLOT)
			flags |= (WLC_AIRTIME_SHORTSLOT);

	pkt_overhead_us =
		wlc_airtime_pkt_overhead_us(flags, ctl_rspec, ack_rspec, bsscfg, atf_state->ac);

	subframe_time_us = wlc_airtime_payload_time_us(flags, rspec,
		(ETHER_MAX_DATA + wlc_airtime_dot11hdrsize(scb->wsec)));

	adjusted_max_pdu = (dur - wlc_airtime_plcp_time_us(rspec, flags))/subframe_time_us;
	max_pdu = MIN(adjusted_max_pdu, max_pdu);
	AMPDU_ATF_INCRSTAT(&atf_stats->pdu, max_pdu);

	/* Max PDU time including header */
	max_pdu_time_us = (max_pdu * subframe_time_us) + pkt_overhead_us;

	ampdu_rate_kbps = ((max_pdu * ETHER_MAX_DATA * 8000)/max_pdu_time_us);

	atf_state->rbytes_target.max = (ampdu_rate_kbps/8000) *
		atf_state->txq_time_allowance_us;
	atf_state->rbytes_target.min = (ampdu_rate_kbps/8000) *
		atf_state->txq_time_min_allowance_us;

	atf_state->last_est_rate = rspec;
	WLCNTINCR(atf_stats->cache_miss);

	return (&atf_state->rbytes_target);

} /* wlc_ampdu_atf_calc_rbytes */

static INLINE atf_rbytes_t*
wlc_ampdu_atf_rbytes(atf_state_t *atf_state)
{
	ratespec_t rspec = atf_state->rspec_action.function(atf_state->rspec_action.arg);

	/* If rspec is the same use the previously calculated result to reduce CPU overhead */
	if (rspec == atf_state->last_est_rate) {
		WLCNTINCR(atf_state->atf_stats.cache_hit);
		return (&atf_state->rbytes_target);
	} else {
		return  wlc_ampdu_atf_calc_rbytes(atf_state, rspec);
	}
}

static INLINE bool
wlc_ampdu_atf_holdrelease(atf_state_t *atf_state)
{
	if (atf_state->atf != 0) {
		atf_rbytes_t *rbytes = wlc_ampdu_atf_rbytes(atf_state);

		WLCNTINCR(atf_state->atf_stats.eval);

		if (atf_state->released_bytes_inflight > rbytes->max) {
			WLCNTINCR(atf_state->atf_stats.neval);
			return TRUE;
		}
	}
	return FALSE;
}

static INLINE bool
wlc_ampdu_atf_lowat_release(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini)
{
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		/* ATF adaptive watermark tested only with AQM devices only */
		return (AMPDU_ATF_ENABLED(ini) &&
			(AMPDU_ATF_STATE(ini)->released_bytes_inflight <=
			AMPDU_ATF_STATE(ini)->rbytes_target.min));
	} else {
		return FALSE;
	}
}

static void
wlc_ampdu_atf_reset_bytes_inflight(scb_ampdu_tid_ini_t *ini)
{
	WLCNTINCR(AMPDU_ATF_STATE(ini)->reset);
	AMPDU_ATF_STATE(ini)->released_bytes_inflight = 0;
	AMPDU_ATF_STATE(ini)->released_packets_inflight = 0;
}

static void
wlc_ampdu_atf_reset_state(scb_ampdu_tid_ini_t *ini)
{
	atf_state_t *atf_state = AMPDU_ATF_STATE(ini);
	wlc_ampdu_atf_reset_bytes_inflight(ini);
	atf_state->last_est_rate = 0;
	atf_state->last_ampdu_fid = wme_ac2fifo[atf_state->ac];
#ifdef WLATF_DONGLE
	atf_state->atfd = (ATFD_ENAB(atf_state)) ?
		wlfc_get_atfd(atf_state->wlc, ini->scb) : NULL;
#endif
	memset(&atf_state->rbytes_target, 0, sizeof(atf_state->rbytes_target));
	wlc_ampdu_atf_ini_set_rspec_action(atf_state, atf_state->band->rspec_override);
	WLCNTINCR(atf_state->reset);
	AMPDU_ATF_CLRCNT(ini);
}

static void BCMFASTPATH
wlc_ampdu_atf_dec_bytes_inflight(scb_ampdu_tid_ini_t *ini, void *p)
{
	atf_state_t *atf_state;
	uint32 inflight;
	uint32 pktlen;
#if defined(WLCNT)
	atf_stats_t *atf_stats;
#endif

	ASSERT(p);
	pktlen = WLPKTTAG(p)->pktinfo.atf.pkt_len;

	/* Pkttag has to be cleared to make sure the completed packets are cleaned up
	 * in the atf dynamic transition from on->off
	 */
	WLPKTTAG(p)->pktinfo.atf.pkt_len = 0;

	if (!AMPDU_ATF_ENABLED(ini)) {
		return;
	}

#if defined(WLCNT)
	atf_stats = AMPDU_ATF_STATS(ini);
#endif
	if (!pktlen) {
		WLCNTINCR(atf_stats->reproc);
		return;
	}

	atf_state = AMPDU_ATF_STATE(ini);

	WLCNTINCR(atf_stats->proc);

	inflight = atf_state->released_bytes_inflight;

	if (inflight >= pktlen) {
		inflight -= pktlen;
		AMPDU_ATF_INCRSTAT(&atf_stats->inflight, inflight);
	} else {
		inflight = 0;
		WLCNTINCR(atf_stats->flush);
	}

	if (atf_state->released_packets_inflight) {
		atf_state->released_packets_inflight--;
	} else {
		WLCNTINCR(atf_stats->qempty);
	}

	atf_state->released_bytes_inflight = inflight;
}

#define wlc_ampdu_dec_bytes_inflight(ini, p) wlc_ampdu_atf_dec_bytes_inflight((ini), (p))

#else

#define wlc_ampdu_dec_bytes_inflight(ini, p) do {} while (0)
#define wlc_ampdu_atf_lowat_release(ampdu_tx, ini) (FALSE)
#define wlc_ampdu_atf_holdrelease(atf_state) (FALSE)
#endif /* WLATF */

/** AMPDU packet class callback */
static void BCMFASTPATH
wlc_ampdu_pkt_freed(wlc_info_t *wlc, void *pkt, uint txs)
{
	ampdu_tx_info_t *ampdu_tx = wlc->ampdu_tx;
	struct scb *scb;
	scb_ampdu_tx_t *scb_ampdu_tx;
	scb_ampdu_tid_ini_t *ini;
	bool acked = (txs & TX_STATUS_ACK_RCV) != 0;
	wlc_pkttag_t *pkttag = WLPKTTAG(pkt);
	uint16 seq = pkttag->seq;
	bool aqm = AMPDU_AQM_ENAB(wlc->pub);

#ifdef BCMDBG_SWWLAN_38455
	if ((pkttag->flags & WLF_AMPDU_MPDU) == 0) {
		WL_PRINT(("%s flags=%x seq=%d aqm=%d acked=%d\n", __FUNCTION__,
			pkttag->flags, pkttag->seq, aqm, acked));
	}
#endif /* BCMDBG_SWWLAN_38455 */
	ASSERT((pkttag->flags & WLF_AMPDU_MPDU) != 0);

	scb = WLPKTTAGSCBGET(pkt);
	if (scb == NULL || !SCB_AMPDU(scb))
		return;

	scb_ampdu_tx = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	if (scb_ampdu_tx == NULL)
		return;

	ini = scb_ampdu_tx->ini[PKTPRIO(pkt)];
	if (!ini) {
		WL_AMPDU_ERR(("wl%d: ampdu_pkt_freed: NULL ini or pon state for tid %d\n",
		              wlc->pub->unit, PKTPRIO(pkt)));
		return;
	}

	if (ini->ba_state != AMPDU_TID_STATE_BA_ON) {
		WL_AMPDU_ERR(("wl%d: ampdu_pkt_freed: ba state %d for tid %d\n",
		              wlc->pub->unit, ini->ba_state, PKTPRIO(pkt)));
		return;
	}

#ifdef PROP_TXSTATUS
	seq = WL_SEQ_GET_NUM(seq);
#endif /* PROP_TXSTATUS */

	/* This seq # check could possibly affect the performance so keep it
	 * in a narrow scope i.e. only for legacy d11 core revs...also start_seq
	 * and max_seq may not apply to aqm anyway...
	 */
	if (!aqm &&
	    MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) >
	    MODSUB_POW2(ini->max_seq, ini->start_seq, SEQNUM_MAX)) {
		WL_AMPDU_ERR(("wl%d: %s: unexpected seq 0x%x, start seq 0x%x, max seq %0x, "
		          "tx_in_transit %u\n", ampdu_tx->wlc->pub->unit, __FUNCTION__,
		          seq, ini->start_seq, ini->max_seq, ini->tx_in_transit));
		return;
	}

	wlc_ampdu_dec_bytes_inflight(ini, pkt);

	ASSERT(ini->tx_in_transit != 0);
	ini->tx_in_transit--;

	/* packet treated as regular MPDU */
	if (pkttag->flags3 & WLF3_AMPDU_REGMPDU) {
		if (acked) {
			wlc_ampdu_ini_move_window(ampdu_tx, scb_ampdu_tx, ini);
		}
		/* send bar to move the window */
		else if (!aqm) {
			uint indx = TX_SEQ_TO_INDEX(seq);
			setbit(ini->barpending, indx);
			wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
		}
		/*
		 *  send bar to move the window, if it is still within the window from
		 *  the last frame released to HW
		 */
		else if (MODSUB_POW2(ini->max_seq, seq, SEQNUM_MAX) < ini->ba_wsize) {
			uint16 bar_seq = MODINC_POW2(seq, SEQNUM_MAX);
			/* check if there is another bar with advence seq no */
			if (!ini->bar_ackpending ||
			    IS_SEQ_ADVANCED(bar_seq, ini->barpending_seq)) {
				ini->barpending_seq = bar_seq;
				wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
			}
		}

		/* Check whether the packets on tx side
		 * is less than watermark level and disable flow
		 */
		wlc_ampdu_txflowcontrol(wlc, scb_ampdu_tx, ini);
	}
	/* packet treated as MPDU in AMPDU */
	else {
		if (!acked) {
			wlc_ampdu_ini_adjust(ampdu_tx, ini, pkt);
		}
	}
} /* wlc_ampdu_pkt_freed */

#ifdef PROP_TXSTATUS

/*
 * If rem_window is 0 and suppr_window is 16 it means we can send 16 packets (real meaning is that
 * we can send 16 previously suppressed packets).
 */

/** returns how many packets can be dequeued from the aggregation q towards the wlc transmit q */
#define wlc_ampdu_release_count(ini, qlen, scb_ampdu) \
	MIN(((ini)->rem_window + (ini)->suppr_window), MIN((qlen), (scb_ampdu)->release))

/** returns bool */
#define wlc_ampdu_window_is_passed(count, ini) \
	((count) > ((ini)->rem_window + (ini)->suppr_window))

#else

#define wlc_ampdu_release_count(ini, qlen, scb_ampdu) \
	MIN((ini)->rem_window, MIN((qlen), (scb_ampdu)->release))

#define wlc_ampdu_window_is_passed(count, ini) \
		((count) > (ini)->rem_window)

#endif /* PROP_TXSTATUS */

/**
 * Function which contains the smarts of when to 'release' (wlc_ampdu_release)
 * aggregated sdu's (that, at this stage, have not been sent yet) from an AMPDU
 * internal software queue towards the wlc/common transmit queue, bringing them
 * closer to the d11 core. This function is called from various places.
 *
 * Parameters:
 *    ini: connection with a specific remote party and a specific TID/QoS
 *         category.
 *
 * Returns TRUE if released else FALSE.
 */

static bool BCMFASTPATH
wlc_ampdu_txeval(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini, bool force)
{
	uint16 qlen;
	/**
	 * #mpdus that can be dequeued from the aggregation queue towards the
	 * wlc 'common' transmit queue:
	 */
	uint16 n_mpdus_to_release = 0;
	uint16 in_transit = 0;  /**< #mpdus pending in wlc/dma/d11 for given tid/ini */
	wlc_txq_info_t *qi; /**< refers to the 'common' transmit queue */
	struct pktq *q;
	uint16 wlc_txq_avail; /**< #mpdus in common/wlc queue */
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	wlc_info_t *wlc = ampdu_tx->wlc;
	wlc_bsscfg_t *cfg;
	struct scb *scb;

	ASSERT(scb_ampdu);
	ASSERT(ini);

	if (!wlc_scb_restrict_can_txeval(wlc)) {
		return FALSE; /* channel switch / data block related */
	}

	if (ini->ba_state != AMPDU_TID_STATE_BA_ON)
		return FALSE;

	qlen = ampdu_pktqprec_n_pkts(&scb_ampdu->txq, ini->tid);
	if (qlen == 0) {
		return FALSE;
	}

	scb = scb_ampdu->scb;
	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);
	BCM_REFERENCE(cfg);


#ifdef PROP_TXSTATUS
	if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub) &&
		PROP_TXSTATUS_ENAB(ampdu_tx->wlc->pub) &&
		wlc_wlfc_suppr_status_query(ampdu_tx->wlc, ini->scb)) {
		wlc_ampdu_flush_ampdu_q(ampdu_tx, SCB_BSSCFG(ini->scb));
		return FALSE;
	}
#endif /* PROP_TXSTATUS */

	n_mpdus_to_release = wlc_ampdu_release_count(ini, qlen, scb_ampdu);
	in_transit = ini->tx_in_transit;

	qi = SCB_WLCIFP(scb)->qi;
	q = WLC_GET_TXQ(qi);
	wlc_txq_avail = pktqprec_avail_pkts(q, WLC_PRIO_TO_PREC(ini->tid));

#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */
	/*
	 * For AQM, limit the number of packets in transit simultaneously so that other
	 * stations get a chance to transmit as well.
	 */
	if (AMPDU_AQM_ENAB(wlc->pub)) {
		bool ps_pretend_limit_transit = SCB_PS_PRETEND(scb);

		if (IS_SEQ_ADVANCED(ini->max_seq, ini->start_seq)) {
			/* here start_seq is not 'ahead' of max_seq because of wrapping */
			in_transit = MODSUB_POW2(ini->max_seq, ini->start_seq, SEQNUM_MAX);
#ifdef PROP_TXSTATUS
			if (in_transit >= ini->suppr_window) {
				/* suppr_window : suppr packet count inside ba window */
				in_transit -= ini->suppr_window;
			} else {
				in_transit = 0;
			}
#endif /* PROP_TXSTATUS */
		} else {
			/* here start_seq is 'ahead' of max_seq because of wrapping */
			in_transit = 0;
		}

		ASSERT(in_transit < SEQNUM_MAX/2);

#ifdef PSPRETEND
		/* scb->ps_pretend_start contains the TSF time for when ps pretend was
		 * last activated. In poor link conditions, there is a high chance that
		 * ps pretend will be re-triggered.
		 * Within a short time window following ps pretend, this code is trying to
		 * constrain the number of packets in transit and so avoid a large number of
		 * packets repetitively going between transmit and ps mode.
		 */
		if (!ps_pretend_limit_transit && SCB_PS_PRETEND_WAS_RECENT(scb)) {
			ps_pretend_limit_transit =
			        wlc_pspretend_limit_transit(wlc->pps_info, cfg, scb, in_transit);
		}
#ifdef PROP_TXSTATUS
		if (PSPRETEND_ENAB(wlc->pub) &&
			SCB_TXMOD_ACTIVE(scb, TXMOD_APPS) &&
		    PROP_TXSTATUS_ENAB(wlc->pub) &&
		    HOST_PROPTXSTATUS_ACTIVATED(wlc)) {
			/* If TXMOD_APPS is active, and proptxstatus is enabled, force release.
			 * With proptxstatus enabled, they are meant to go to wlc_apps_ps_enq()
			 * to get their pktbufs freed in dongle, and instead stored in the host.
			 */
			ASSERT(SCB_PS(scb));
			ps_pretend_limit_transit = FALSE;
		}
#endif /* PROP_TXSTATUS */
#endif /* PSPRETEND */

		if (!force && (
			(wlc_ampdu_atf_holdrelease(AMPDU_ATF_STATE(ini))) ||
			(in_transit > ampdu_tx->aqm_max_release[ini->tid]) ||
			(wlc_txq_avail <  MIN(AMPDU_AQM_RELEASE_MAX,
				(pktqprec_max_pkts(q, WLC_PRIO_TO_PREC(ini->tid)) / 2))) ||
			ps_pretend_limit_transit)) {

			WL_AMPDU_TX(("wl%d: txeval: Stop Releasing %d in_transit %d tid %d "
				"wm %d rem_wdw %d qlen %d force %d txq_avail %d pps %d\n",
				wlc->pub->unit, n_mpdus_to_release, in_transit,
				ini->tid, ampdu_tx_cfg->tx_rel_lowat, ini->rem_window, qlen,
				force, wlc_txq_avail, ps_pretend_limit_transit));
			AMPDUSCBCNTINCR(scb_ampdu->cnt.txnoroom);
			n_mpdus_to_release = 0;
		}
	}
#endif /* WLAMPDU_MAC */

	if (wlc_txq_avail <= n_mpdus_to_release) {
	    /* change decision, as we are running out of space in common queue */
	    n_mpdus_to_release = 0;
	    AMPDUSCBCNTINCR(scb_ampdu->cnt.txnoroom);
	}

	if (n_mpdus_to_release == 0)
		return FALSE;

	/* release mpdus if any one of the following conditions are met */
	if ((in_transit < ampdu_tx_cfg->tx_rel_lowat) ||
		wlc_ampdu_atf_lowat_release(ampdu_tx, ini) ||
	    (n_mpdus_to_release == MIN(scb_ampdu->release, ini->ba_wsize)) ||
	    force || AMPDU_HW_ENAB(wlc->pub) /* || AMPDU_AQM_ENAB(wlc->pub) */ )
	{
		AMPDU_ATF_INCRSTAT(&AMPDU_ATF_STATS(ini)->inputq, qlen);

		if (in_transit < ampdu_tx_cfg->tx_rel_lowat) {
			/* mpdus released due to lookahead watermark */
			WLCNTADD(ampdu_tx->cnt->txrel_wm, n_mpdus_to_release);
		}
		if (n_mpdus_to_release == scb_ampdu->release)
			WLCNTADD(ampdu_tx->cnt->txrel_size, n_mpdus_to_release);

		WL_AMPDU_TX(("wl%d: wlc_ampdu_txeval: Releasing %d mpdus: in_transit %d tid %d "
			"wm %d rem_wdw %d qlen %d force %d start_seq %x, max_seq %x\n",
			wlc->pub->unit, n_mpdus_to_release, in_transit,
			ini->tid, ampdu_tx_cfg->tx_rel_lowat, ini->rem_window, qlen, force,
			ini->start_seq, ini->max_seq));

		wlc_ampdu_release(ampdu_tx, scb_ampdu, ini, n_mpdus_to_release, wlc_txq_avail);
#ifdef BCMDBG
		WLC_AMPDU_TXQ_PROF_ADD_ENTRY(wlc, scb);
#endif
		return TRUE;
	}

	if ((ini->tx_in_transit == 0) && (in_transit != 0)) {
		WL_AMPDU_ERR(("wl%d.%d Cannot release: in_transit %d tid %d "
			"start_seq 0x%x barpending_seq 0x%x max_seq 0x%x\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
			in_transit, ini->tid, ini->start_seq, ini->barpending_seq, ini->max_seq));
	}

	WL_AMPDU_TX(("wl%d: wlc_ampdu_txeval: Cannot release: in_transit %d tid %d wm %d "
	             "rem_wdw %d qlen %d release %d\n",
	             wlc->pub->unit, in_transit, ini->tid, ampdu_tx_cfg->tx_rel_lowat,
	             ini->rem_window, qlen, n_mpdus_to_release));

	return FALSE;
} /* wlc_ampdu_txeval */

/**
 * Releases transmit mpdu's for the caller supplied tid/ini, from an AMPDU internal software queue
 * (scb_ampdu->txq), forwarding them to the wlc managed transmit queue (see wlc_sendampdu()), which
 * brings the packets a step closer towards the d11 core.
 *
 * Function parameters:
 *     release: number of packets to release
 */

static void BCMFASTPATH
wlc_ampdu_release(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini, uint16 release, uint16 q_avail)
{
	void* p = NULL;
	uint16 seq = 0; /* d11 sequence */
	uint16 indx;   /* not used when AQM is enabled */
	uint16 delta;  /* is zero when AQM is enabled */
	bool do_release;
	uint16 chunk_pkts;
	bool pkts_remaining;
	uint16 suppr_count = 0; /* PROP_TXSTATUS related */
	bool aqm_enab;
	bool reset_suppr_count = FALSE;
	uint pktbytes;
	uint8 tid = ini->tid;
	wlc_info_t *wlc = ampdu_tx->wlc;
	wlc_pub_t *pub = wlc->pub;
	osl_t *osh = wlc->osh;
	wlc_pkttag_t *pkttag = NULL;
	struct pktq *txq = &scb_ampdu->txq;
	struct scb *scb = ini->scb;
#ifdef WLATF
	atf_state_t *atf_state;
	atf_rbytes_t *atf_rbytes;
	bool use_atf;
	uint32 max_rbytes;
	uint32 chunk_bytes;
	uint32 atf_rbytes_min;
	uint atf_mode;

#ifdef WLCNT
	atf_stats_t *atf_stats;
	uint32 npkts;
	uint32 uflow;
	uint32 oflow;
	uint32 minrel;
	uint32 reloverride;
#endif /* WLCNT */
#endif /* WLATF */
#if defined(WLATF_DONGLE)
	bool atfd_enab = FALSE;
	uint16 frmbytes;
	ratespec_t cur_rspec = 0;
	/* The variables below are used in ampdu do_release cycle. */
	uint8 fl;
	uint32 dot11hdrsize;
	wlc_ravg_info_t *ravg_info;
	uint16 *txpktlen_rbuf;
	wlc_atfd_t *atfd = NULL;
#endif /* WLATF_DONGLE */
	struct spktq spq;
	uint next_fid;

	BCM_REFERENCE(suppr_count);
	BCM_REFERENCE(reset_suppr_count);
	BCM_REFERENCE(osh);
	BCM_REFERENCE(pub);

	WL_AMPDU_TX(("%s start release %d\n", __FUNCTION__, release));

	next_fid = SCB_TXMOD_NEXT_FID(scb, TXMOD_AMPDU);
	if (next_fid == TXMOD_TRANSMIT) {
		spktq_init(&spq, PKTQ_LEN_MAX);
	}

	if (release && AMPDU_AQM_ENAB(pub)) {
		/* this reduces packet drop after a channel switch (until rate has stabilized) */
		release = wlc_scb_restrict_can_ampduq(wlc, scb, ini->tx_in_transit, release);
	}

	do_release = (release != 0);
	if (!do_release) {
		return;
	}

	if (!AMPDU_AQM_ENAB(pub)) {
		seq = SCB_SEQNUM(scb, tid) & (SEQNUM_MAX - 1);
		delta = MODSUB_POW2(seq, MODADD_POW2(ini->max_seq, 1, SEQNUM_MAX), SEQNUM_MAX);

		/* do not release past window (seqnum based check). There might be pkts
		 * in txq which are not marked AMPDU so will use up a seq number
		 */
		if (wlc_ampdu_window_is_passed(delta + release, ini)) {
			WL_AMPDU_ERR(("wl%d: wlc_ampdu_release: cannot release %d"
				"(out of window)\n", pub->unit, release));
			return;
		}
	} else {
		delta = 0;
	}

	chunk_pkts = 0;
	aqm_enab = AMPDU_AQM_ENAB(pub);
	pkts_remaining = FALSE;

	BCM_REFERENCE(aqm_enab);
#if defined(TINY_PKTJOIN)
	struct pktq tmp_q;
	bool tiny_join = (release > 1);

	if (TINY_PKTJOIN_ENAB(pub)) {
		if (tiny_join) {
			uint8 count = release;
			void *prev_p = NULL, *phdr;
			int plen;

			pktq_init(&tmp_q, 1, AMPDU_PKTQ_LEN);
			while (count--) {
				p = wlc_ampdu_pktq_pdeq(txq, tid);
				ASSERT(p != NULL);

#ifdef DMATXRC
				if (DMATXRC_ENAB(pub) &&
					(WLPKTTAG(p)->flags & WLF_PHDR)) {
					phdr = p;
					p = PKTNEXT(osh, p);
					ASSERT(p);
				} else
#endif
					phdr = NULL;

				plen = PKTLEN(osh, p);
				if ((plen <= TCPACKSZSDU) && (prev_p  != NULL)) {
					void *new_pkt;
					uint16 round_len;
					uint16 prev_len;

					ASSERT(PKTTAILROOM(osh, prev_p) > 512);

					prev_len = PKTLEN(osh, prev_p);
					round_len = 64 +
						ROUNDUP(prev_len, sizeof(uint16)) - prev_len;
					new_pkt = hnd_pkt_clone(osh,
						prev_p,
						PKTLEN(osh, prev_p) + round_len,
						PKTTAILROOM(osh, prev_p)-round_len);
					if (new_pkt) {
						PKTPULL(osh, new_pkt, 192);
						bcopy(PKTDATA(osh, p),
						   PKTDATA(osh, new_pkt),
						   plen);
						PKTSETLEN(osh, new_pkt, plen);

						if (phdr) {
						   PKTSETNEXT(osh, phdr, new_pkt);
						} else {
						   wlc_pkttag_info_move(wlc, p, new_pkt);
						      PKTSETPRIO(new_pkt, PKTPRIO(p));
						}

						PKTSETNEXT(osh, new_pkt, PKTNEXT(osh, p));
						PKTSETNEXT(osh, p, NULL);

						PKTFREE(osh, p, TRUE);
						p = phdr ? phdr : new_pkt;
					}

					/* Used so clear to re-load */
					prev_p = NULL;
				}

				pktq_penq(&tmp_q, 0, phdr ? phdr : p);

				/*
				 * Traverse in case of AMSDU pkt chain
				 * and only set prev_p with enough tail room
				 * If prev_p != NULL, it was not used so don't update
				 * since there's no guarantee the pkt immediately
				 * before it has tail room.
				 */
				if (prev_p == NULL) {
					/* No tail room in phdr */
					prev_p = phdr ? PKTNEXT(osh, phdr) : p;

					while (prev_p) {
						if (PKTTAILROOM(osh, prev_p) > 512)
							break;

						prev_p = PKTNEXT(osh, prev_p);
					}
				}
			}
		}
	}
#endif /* TINY_PKTJOIN */
#ifdef WLATF
	chunk_bytes = 0;
#ifdef WLCNT
	atf_stats = AMPDU_ATF_STATS(ini);
	npkts = 0;
	uflow = 0;
	oflow = 0;
	minrel = 0;
	reloverride = 0;
#endif /* WLCNT */

	if (AMPDU_ATF_ENABLED(ini)) {
		atf_state = AMPDU_ATF_STATE(ini);
		atf_rbytes = wlc_ampdu_atf_rbytes(atf_state);
		use_atf  = TRUE;
		atf_rbytes_min = atf_rbytes->min;
		atf_mode = atf_state->atf;
		max_rbytes = (atf_rbytes->max > atf_state->released_bytes_inflight) ?
			(atf_rbytes->max - atf_state->released_bytes_inflight) :  0;
#if defined(WLATF_DONGLE)
		atfd_enab = (use_atf && ATFD_ENAB(wlc));
		if (atfd_enab) {
			cur_rspec = atf_state->last_est_rate;
			atfd = atf_state->atfd;
			ASSERT(atfd);
		}
#endif /* WLATF_DONGLE */
	} else {
		use_atf = FALSE;
		atf_state = NULL;
		atf_rbytes = NULL;
		max_rbytes = 0;
		pktbytes = 0;
		chunk_bytes = 0;
		atf_rbytes_min = 0;
		atf_mode = 0;
	}
#else
	/* q_avail not used if ATF is not compiled in, coverity pacifier */
	BCM_REFERENCE(q_avail);
#endif /* WLATF */

#if defined(WLATF_DONGLE)
	if (atfd_enab) {
		if (!cur_rspec) {
			cur_rspec = wlc_scb_ratesel_get_primary(wlc, scb, NULL);
		}
		fl = RAVG_PRIO2FLR(PRIOMAP(wlc), tid);
		dot11hdrsize = wlc_scb_dot11hdrsize(scb);
		ravg_info = TXPKTLEN_RAVG(atfd, fl);
		txpktlen_rbuf = TXPKTLEN_RBUF(atfd, fl);
	}
#endif /* WLATF_DONGLE */

	while (do_release) { /* loop over each packet to release */
#if defined(TINY_PKTJOIN)
		if (TINY_PKTJOIN_ENAB(pub) && tiny_join) {
			p = pktq_pdeq(&tmp_q, 0);
		} else
#endif
		{
			p = wlc_ampdu_pktq_pdeq(txq, tid);
		}


#ifdef WLATF
		if (p == NULL) {
			WLCNTINCR(uflow);
			break;
		}
		WLCNTINCR(npkts);
#else
		ASSERT(p != NULL);
		if (p == NULL) {
			break;
		}
#endif

		chunk_pkts++;

		ASSERT(PKTPRIO(p) == tid);

		pkttag = WLPKTTAG(p);
		pkttag->flags |= WLF_AMPDU_MPDU;

#ifdef PROP_TXSTATUS
		if (IS_WL_TO_REUSE_SEQ(pkttag->seq)) {
			seq = WL_SEQ_GET_NUM(pkttag->seq);
#ifdef PROP_TXSTATUS_SUPPR_WINDOW
			suppr_count++;
#endif
		} else
#endif /* PROP_TXSTATUS */
		{
			/* assign seqnum and save in pkttag */
			seq = SCB_SEQNUM(scb, tid) & (SEQNUM_MAX - 1);
			SCB_SEQNUM(scb, tid)++;
			pkttag->seq = seq;
			ini->max_seq = seq;
			reset_suppr_count = TRUE;
		}

#ifdef PROP_TXSTATUS
		if (PROP_TXSTATUS_ENAB(pub) &&
			WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wlfc))) {
			SET_WL_HAS_ASSIGNED_SEQ(pkttag->seq);
		}
#endif /* PROP_TXSTATUS */

		/* ASSERT(!isset(ini->ackpending, indx)); */
		/* ASSERT(ini->txretry[indx] == 0); */
		if (!AMPDU_AQM_ENAB(pub)) {
			indx = TX_SEQ_TO_INDEX(seq);
			setbit(ini->ackpending, indx);
		}

		pktbytes = pkttotlen(ampdu_tx->wlc->osh, p);
		BCM_REFERENCE(pktbytes);
		WLCNTSCBINCR(scb_ampdu->ampdu_scb_stats->tx_pkts[ini->tid]);
		WLCNTSCBADD(scb_ampdu->ampdu_scb_stats->tx_bytes[ini->tid], pktbytes);

#ifdef WLATF
		/* The downstream TXMOD may delete and free the packet, do the ATF
		 * accounting before the TXMOD to prevent the counter from going
		 * out of sync.
		 * Similarly the pkttag update, updating the pkttag after it has
		 * been freed is to be avoided
		 */
		if (use_atf) {
			pktbytes = pkttotlen(osh, p);
			pkttag->pktinfo.atf.pkt_len = (uint16)pktbytes;
			atf_state->released_bytes_inflight += pktbytes;
			chunk_bytes += pktbytes;
		}
#endif /* WLATF */

		/* enable packet callback for every MPDU in AMPDU */
		WLF2_PCB4_REG(p, WLF2_PCB4_AMPDU);
		ini->tx_in_transit++;

#if defined(WLATF_DONGLE)
		if (atfd_enab) {
			frmbytes = pkttotlen(osh, p) + dot11hdrsize;
			/* adding pktlen into the moving average buffer */
			RAVG_ADD(ravg_info, txpktlen_rbuf, frmbytes, RAVG_EXP_PKT);
		}
#endif /* WLATF_DONGLE */

		if (next_fid == TXMOD_TRANSMIT) {
			/* Enqueue p to spq */
			spktq_enq(&spq, p);
		} else {
			/* calls transmit module (wlc_txq_enq) : */
			SCB_TX_NEXT(TXMOD_AMPDU, scb, p, WLC_PRIO_TO_PREC(tid));
		}

		/* pkts_remaining is TRUE is we are below the frame release limit */
		pkts_remaining = (chunk_pkts < release);

		/* Logic in the remainder of the scope
		 * evaluates if the next dequeue is to be done
		 */

#ifdef WLATF
		/* Apply ATF release overrides */
			if (use_atf) {
			/* Assumes SNAP header already present */

			q_avail--;

			/* Out of space on outbound queue. We are done. */
			if (q_avail == 0) {
					WLCNTINCR(oflow);
					do_release = FALSE;
			} else {
					/* can_release is TRUE below ATF byte time limit */
					bool can_release = (chunk_bytes <= max_rbytes);

					/* Primary release condition
					 * Meet release up to ATF time low watermark, AQM only.
					 * Non AQM devices rely on the driver to track the BA window
					 * so we may end up releasing too many packets and thus have
					 * to add another check to make sure it does not happen,
					 * better to avoid the matter entirely in the
					 * time critical loop.
					 */
					do_release = ((aqm_enab) &&
						(chunk_bytes <= atf_rbytes_min));

					/* Secondary release condition,
					 * there packets remaining to be
					 * released and we are below ATF time limit.
					 */
					do_release |= ((pkts_remaining) && (can_release));

					/* Tertiary release.
					 * ATF_PMODE release up to ATF time limit
					 */
					do_release |= ((can_release) &&
						(atf_mode == WLC_AIRTIME_PMODE));
#ifdef WLCNT
					/* Update stats */

					if (!pkts_remaining && can_release) {
						/* Log minimum release if all frames
						 * have been released and we are still
						 * below the time low watermark
						 */
						if (chunk_bytes <= atf_rbytes_min) {
								WLCNTINCR(minrel);
						}
						/* Log PMODE override count */
						else if (atf_mode == WLC_AIRTIME_PMODE) {
								WLCNTINCR(reloverride);
						} /* (!pkts_remaining && can_release) */
					}
#endif /* WLCNT */
				} /* (q_avail == 0) */
		} else
#endif /* WLATF */
		{
			do_release = pkts_remaining;
		}
	} /* while (do_release) */

	if (next_fid == TXMOD_TRANSMIT) {
		wlc_txq_enq_spq(wlc, scb, &spq, WLC_PRIO_TO_PREC(tid));
	}

#ifdef WLATF
	if (use_atf) {
		atf_state->released_packets_inflight += chunk_pkts;
	}
#endif /* WLATF */

#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(pub) &&
		WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wlfc))) {
		if (reset_suppr_count || (ini->suppr_window <= suppr_count)) {
			ini->suppr_window = 0; /* suppr packet count inside ba window */
		} else {
			ini->suppr_window -= suppr_count;
		}
	}
#endif /* PROP_TXSTATUS */

	if (!AMPDU_AQM_ENAB(pub)) {
		ini->rem_window -= (chunk_pkts + delta - suppr_count);
	}

#if defined(WLATF_DONGLE)
	if (atfd_enab && (chunk_pkts > 0)) {
		fl = RAVG_PRIO2FLR(PRIOMAP(wlc), tid);
		ASSERT(atfd);
		wlc_ravg_add_weight(atfd, fl, cur_rspec);
	}
#endif /* WLATF_DONGLE */

#if defined(WLATF) && defined(WLCNT)
	if (use_atf) {
		WLCNTADD(atf_stats->npkts, npkts);
		WLCNTADD(atf_stats->uflow, uflow);
		WLCNTADD(atf_stats->oflow, oflow);
		WLCNTADD(atf_stats->uflow, minrel);
		WLCNTADD(atf_stats->oflow, reloverride);

		WLCNTINCR(atf_stats->ndequeue);
		if (chunk_pkts == 1) {
			WLCNTINCR(atf_stats->singleton);
		}
		if (p != NULL) {
			if (pkts_remaining)
				WLCNTINCR(atf_stats->timelimited);
			if (atf_rbytes->max > atf_state->released_bytes_inflight)
				WLCNTINCR(atf_stats->framelimited);
		}

		AMPDU_ATF_INCRSTAT(&atf_stats->rbytes, max_rbytes);
		AMPDU_ATF_INCRSTAT(&atf_stats->chunk_bytes, chunk_bytes);
		AMPDU_ATF_INCRSTAT(&atf_stats->chunk_pkts, chunk_pkts);
		AMPDU_ATF_INCRSTAT(&atf_stats->inflight, atf_state->released_bytes_inflight);
		AMPDU_ATF_INCRSTAT(&atf_stats->transit, atf_state->released_packets_inflight);
	}
#endif /* WLATF  && WLCNT */
} /* wlc_ampdu_release */

#if !(defined(TXQ_MUX) && !defined(WLAMPDU_MAC))
/* Fn is not needed if TXQ_MUX && !WLAMPDU_MAC */

/**
 * Returns TRUE if 'seq' contains the next expected d11 sequence. Protects against pkt frees in the
 * txpath by others.
 */
static bool BCMFASTPATH
ampdu_is_exp_seq(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini,
	uint16 seq, bool suppress_pkt)
{
#ifdef WLAMPDU_MAC
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub))
		return ampdu_is_exp_seq_aqm(ampdu_tx, ini, seq, suppress_pkt);
	else
#endif /* WLAMPDU_MAC */
		return ampdu_is_exp_seq_noaqm(ampdu_tx, ini, seq, suppress_pkt);

}

#endif /* !(TXQ_MUX && !WLAMPDU_MAC) */

#ifdef WLAMPDU_MAC


#if !(defined(TXQ_MUX) && !defined(WLAMPDU_MAC))
/* Fn is not needed if TXQ_MUX && !WLAMPDU_MAC */

/**
 * Returns TRUE if 'seq' contains the next expected d11 sequence. If not, sends a Block Ack Req to
 * the remote party. AC chip specific function.
 */
static bool BCMFASTPATH
ampdu_is_exp_seq_aqm(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini,
	uint16 seq, bool suppress_pkt)
{
	uint16 offset;
#ifdef BCMDBG
	scb_ampdu_tx_t *scb_ampdu;
	ASSERT(ini);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, ini->scb);
	ASSERT(scb_ampdu);
#endif
	BCM_REFERENCE(suppress_pkt);


#ifdef PROP_TXSTATUS
#ifdef WL_NATOE
	if (NATOE_ENAB(ampdu_tx->wlc->pub) && WL_SEQ_GET_VALIDSUPPR(ini->last_suppr_seq)) {
		if (suppress_pkt) {
			if (IS_SEQ_ADVANCED(seq, WL_SEQ_GET_NUM(ini->last_suppr_seq))) {
				/* re-setting valid bit that indicates last_suppr_seq is valid */
				WL_SEQ_SET_VALIDSUPPR(ini->last_suppr_seq, 0);
			}
		} else {
			/* re-setting valid bit that indicates last_suppr_seq is valid
			 * when first non-suppress_pkt from host is received.
			 */
			WL_SEQ_SET_VALIDSUPPR(ini->last_suppr_seq, 0);
		}
	}
#endif /* WL_NATOE */

	if (suppress_pkt) {
		if (IS_SEQ_ADVANCED(seq, ini->tx_exp_seq)) {
			ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
		}
		return TRUE;
	}
#endif /* PROP_TXSTATUS */

	if (ini->tx_exp_seq != seq) {
		offset = MODSUB_POW2(seq, ini->tx_exp_seq, SEQNUM_MAX);
		WL_AMPDU_ERR(("wl%d: r0hole: tx_exp_seq 0x%x, seq 0x%x\n",
			ampdu_tx->wlc->pub->unit, ini->tx_exp_seq, seq));

		/* pkts bypassed (or lagging) on way down */
		if (offset > SEQNUM_MAX / 2) {
			WL_AMPDU_ERR(("wl%d: rlag: tx_exp_seq 0x%x, seq 0x%x\n",
				ampdu_tx->wlc->pub->unit, ini->tx_exp_seq, seq));
			WLCNTINCR(ampdu_tx->cnt->txrlag);
			AMPDUSCBCNTINCR(scb_ampdu->cnt.txrlag);
			return FALSE;
		}

		ini->barpending_seq = seq;
		wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
		WLCNTADD(ampdu_tx->cnt->txr0hole, offset);
	}
	ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
	return TRUE;
} /* ampdu_is_exp_seq_aqm */

static bool BCMFASTPATH
ampdu_detect_seq_hole(ampdu_tx_info_t *ampdu_tx, uint16 seq, scb_ampdu_tid_ini_t *ini)
{
	bool has_hole = FALSE;
	BCM_REFERENCE(ampdu_tx);

	if (seq != ini->next_enq_seq) {
		WL_AMPDU_TX(("%s %d: seq hole detected! 0x%x 0x%x\n",
			__FUNCTION__, __LINE__, seq, ini->next_enq_seq));
		has_hole = TRUE;
	}
	ini->next_enq_seq = NEXT_SEQ(seq);
	return has_hole;
}

#endif /* !(TXQ_MUX && !WLAMPDU_MAC) */
#endif /* WLAMPDU_MAC */


#if !(defined(TXQ_MUX) && !defined(WLAMPDU_MAC))
/* Fn is not needed if TXQ_MUX && !WLAMPDU_MAC */

/**
 * Returns TRUE if 'seq' contains the next expected d11 sequence. If not, sends a Block Ack Req to
 * the remote party. Non AC chip specific function.
 */
static bool BCMFASTPATH
ampdu_is_exp_seq_noaqm(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini,
	uint16 seq, bool suppress_pkt)
{
	uint16 txretry, offset, indx;
	uint i;
	bool found;
#ifdef BCMDBG
	scb_ampdu_tx_t *scb_ampdu;
	ASSERT(ini);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, ini->scb);
	ASSERT(scb_ampdu);
#endif

	BCM_REFERENCE(suppress_pkt);

#ifdef PROP_TXSTATUS
	if (suppress_pkt) {
		if (IS_SEQ_ADVANCED(seq, ini->tx_exp_seq)) {
			ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
		}
		return TRUE;
	}
#endif /* PROP_TXSTATUS */

	txretry = ini->txretry[TX_SEQ_TO_INDEX(seq)];
	if (txretry == 0) {
		if (ini->tx_exp_seq != seq) {
			offset = MODSUB_POW2(seq, ini->tx_exp_seq, SEQNUM_MAX);
			WL_AMPDU_ERR(("wl%d: r0hole: tx_exp_seq 0x%x, seq 0x%x\n",
				ampdu_tx->wlc->pub->unit, ini->tx_exp_seq, seq));

			/* pkts bypassed (or lagging) on way down */
			if (offset > SEQNUM_MAX / 2) {
				WL_AMPDU_ERR(("wl%d: rlag: tx_exp_seq 0x%x, seq 0x%x\n",
					ampdu_tx->wlc->pub->unit, ini->tx_exp_seq, seq));
				WLCNTINCR(ampdu_tx->cnt->txrlag);
				AMPDUSCBCNTINCR(scb_ampdu->cnt.txrlag);
				return FALSE;
			}

			if (MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) >= ini->ba_wsize) {
				WL_AMPDU_ERR(("wl%d: unexpected seq: start_seq 0x%x, seq 0x%x\n",
					ampdu_tx->wlc->pub->unit, ini->start_seq, seq));
				WLCNTINCR(ampdu_tx->cnt->txrlag);
				AMPDUSCBCNTINCR(scb_ampdu->cnt.txrlag);
				return FALSE;
			}

			WLCNTADD(ampdu_tx->cnt->txr0hole, offset);
			/* send bar to move the window */
			indx = TX_SEQ_TO_INDEX(ini->tx_exp_seq);
			for (i = 0; i < offset; i++, indx = NEXT_TX_INDEX(indx))
				setbit(ini->barpending, indx);
			wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
		}

		ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
	} else {
		if (ini->retry_cnt == 0 || ini->retry_seq[ini->retry_head] != seq) {
			found = FALSE;
			indx = ini->retry_head;
			for (i = 0; i < ini->retry_cnt; i++, indx = NEXT_TX_INDEX(indx)) {
				if (ini->retry_seq[indx] == seq) {
					found = TRUE;
					break;
				}
			}
			if (!found) {
				WL_AMPDU_ERR(("wl%d: rnlag: tx_exp_seq 0x%x, seq 0x%x\n",
					ampdu_tx->wlc->pub->unit, ini->tx_exp_seq, seq));
				WLCNTINCR(ampdu_tx->cnt->txrlag);
				AMPDUSCBCNTINCR(scb_ampdu->cnt.txrlag);
				return FALSE;
			}
			while (ini->retry_seq[ini->retry_head] != seq) {
				WL_AMPDU_ERR(("wl%d: rnhole: retry_seq 0x%x, seq 0x%x\n",
				              ampdu_tx->wlc->pub->unit,
				              ini->retry_seq[ini->retry_head], seq));
				setbit(ini->barpending,
				       TX_SEQ_TO_INDEX(ini->retry_seq[ini->retry_head]));
				WLCNTINCR(ampdu_tx->cnt->txrnhole);
				ini->retry_seq[ini->retry_head] = 0;
				ini->retry_head = NEXT_TX_INDEX(ini->retry_head);
				ASSERT(ini->retry_cnt > 0);
				ini->retry_cnt--;
			}
			wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
		}
		ini->retry_seq[ini->retry_head] = 0;
		ini->retry_head = NEXT_TX_INDEX(ini->retry_head);
		ASSERT(ini->retry_cnt > 0);
		ini->retry_cnt--;
		ASSERT(ini->retry_cnt < ampdu_tx->config->ba_max_tx_wsize);
	}

	/* discard the frame if it is a retry and we are in pending off state */
	if (txretry && (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF)) {
		setbit(ini->barpending, TX_SEQ_TO_INDEX(seq));
		return FALSE;
	}

	if (MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) >= ini->ba_wsize) {
		ampdu_dump_ini(ini);
		ASSERT(MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) < ini->ba_wsize);
	}
	return TRUE;
} /* ampdu_is_exp_seq_noaqm */

#endif /* !(TXQ_MUX && !WLAMPDU_MAC) */


#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */

/**
 * The epoch field indicates whether the frame can be aggregated into the same A-MPDU as the
 * previous MPDU.
 */
void
wlc_ampdu_change_epoch(ampdu_tx_info_t *ampdu_tx, int fifo, int reason_code)
{
#ifdef BCMDBG
	const char *str = "Undefined";

	switch (reason_code) {
	case AMU_EPOCH_CHG_PLCP:
		str = "PLCP";
		WLCNTINCR(ampdu_tx->cnt->echgr1);
		break;
	case AMU_EPOCH_CHG_FID:
		WLCNTINCR(ampdu_tx->cnt->echgr2);
		str = "FRAMEID";
		break;
	case AMU_EPOCH_CHG_NAGG:
		WLCNTINCR(ampdu_tx->cnt->echgr3);
		str = "ampdu_tx OFF";
		break;
	case AMU_EPOCH_CHG_MPDU:
		WLCNTINCR(ampdu_tx->cnt->echgr4);
		str = "reg mpdu";
		break;
	case AMU_EPOCH_CHG_DSTTID:
		WLCNTINCR(ampdu_tx->cnt->echgr5);
		str = "DST/TID";
		break;
	case AMU_EPOCH_CHG_SEQ:
		WLCNTINCR(ampdu_tx->cnt->echgr6);
		str = "SEQ No";
		break;
	case AMU_EPOCH_CHG_TXC_UPD:
		WLCNTINCR(ampdu_tx->cnt->echgr6);
		str = "TXC UPD";
		break;
	case AMU_EPOCH_CHG_TXHDR:
		WLCNTINCR(ampdu_tx->cnt->echgr6);
		str = "TX_S_L_HDR";
		break;
	default:
		ASSERT(0);
		break;
	}

	WL_AMPDU_HWDBG(("wl%d: %s: fifo %d: change epoch for %s\n",
		ampdu_tx->wlc->pub->unit, __FUNCTION__, fifo, str));
#else
	BCM_REFERENCE(reason_code);
#endif /* BCMDBG */

	ASSERT(fifo < NFIFO_EXT);
	if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		ampdu_tx->hagg[fifo].change_epoch = TRUE;
	} else {
		ampdu_tx->hagg[fifo].epoch = (ampdu_tx->hagg[fifo].epoch) ? 0 : 1;
		WL_NONE(("wl%d: %s: fifo %d: change epoch for %s to %d\n",
		         ampdu_tx->wlc->pub->unit, __FUNCTION__, fifo, str,
		         ampdu_tx->hagg[fifo].epoch));
	}
} /* wlc_ampdu_change_epoch */

#ifdef AMPDU_NON_AQM

#if !defined(TXQ_MUX)
static uint16
wlc_ampdu_calc_maxlen(ampdu_tx_info_t *ampdu_tx, uint8 plcp0, uint plcp3, uint32 txop)
{
	uint8 is40, sgi;
	uint8 mcs = 0;
	uint16 txoplen = 0;
	uint16 maxlen = 0xffff;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;

	is40 = (plcp0 & MIMO_PLCP_40MHZ) ? 1 : 0;
	sgi = PLCP3_ISSGI(plcp3) ? 1 : 0;
	mcs = plcp0 & ~MIMO_PLCP_40MHZ;
	ASSERT(VALID_MCS(mcs));

	if (txop) {
		/* rate is in Kbps; txop is in usec
		 * ==> len = (rate * txop) / (1000 * 8)
		 */
		txoplen = (MCS_RATE(mcs, is40, sgi) >> 10) * (txop >> 3);

		maxlen = MIN((txoplen),
			(ampdu_tx_cfg->max_txlen[MCS2IDX(mcs)][is40][sgi] *
			ampdu_tx_cfg->ba_tx_wsize));
	}
	WL_AMPDU_HW(("wl%d: %s: txop %d txoplen %d maxlen %d\n",
		ampdu_tx->wlc->pub->unit, __FUNCTION__, txop, txoplen, maxlen));

	return maxlen;
}
#endif /* TXQ_MUX */

#endif /* WLAMPDU_NON_AQM */
#endif /* WLAMPDU_MAC */


#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */

/** Called by higher layer to determine if a transmit packet is an AMPDU packet */
extern bool
wlc_ampdu_was_ampdu(ampdu_tx_info_t *ampdu_tx, int fifo)
{
	return TRUE;
}

/** Change epoch and save epoch determining params. Return updated epoch. */
uint8
wlc_ampdu_chgnsav_epoch(ampdu_tx_info_t *ampdu_tx, int fifo, int reason_code,
	struct scb *scb, uint8 tid, wlc_txh_info_t* txh_info)
{
	ampdumac_info_t *hagg;
	bool isAmpdu;
	int8* tsoHdrPtr = txh_info->tsoHdrPtr;

	/* when switch from non-aggregate ampdu to aggregate ampdu just change
	 * epoch determining parameters. do not change epoch.
	 */
	if (reason_code != AMU_EPOCH_NO_CHANGE)
		wlc_ampdu_change_epoch(ampdu_tx, fifo, reason_code);

	isAmpdu = wlc_txh_get_isAMPDU(ampdu_tx->wlc, txh_info);

	hagg = &(ampdu_tx->hagg[fifo]);
	hagg->prev_scb = scb;
	hagg->prev_tid = tid;
	hagg->prev_ft = AMPDU_NONE;
	if (isAmpdu) {
		uint16 tmp;
		tmp = ltoh16(txh_info->PhyTxControlWord0) & D11AC_PHY_TXC_FT_MASK;
		ASSERT(tmp ==  D11AC_PHY_TXC_FT_11N ||
		       tmp == D11AC_PHY_TXC_FT_11AC);
		hagg->prev_ft = (tmp == D11AC_PHY_TXC_FT_11N) ? AMPDU_11N : AMPDU_11VHT;
	}
	hagg->prev_txphyctl0 = txh_info->PhyTxControlWord0;
	hagg->prev_txphyctl1 = txh_info->PhyTxControlWord1;
	hagg->prev_plcp[0] = txh_info->plcpPtr[0];
	hagg->prev_plcp[1] = txh_info->plcpPtr[3];
#if D11CONF_GE(40)
	if (hagg->prev_ft == AMPDU_11VHT) {
		hagg->prev_plcp[3] = txh_info->plcpPtr[1];
		hagg->prev_plcp[2] = txh_info->plcpPtr[2];
		hagg->prev_plcp[4] = txh_info->plcpPtr[4];
	}
#endif /* D11CONF_GE(40) */

	hagg->prev_shdr = (tsoHdrPtr[2] & TOE_F2_TXD_HEAD_SHORT);

	WL_AMPDU_HWDBG(("%s fifo %d epoch %d\n", __FUNCTION__, fifo, hagg->epoch));

	return hagg->epoch;
} /* wlc_ampdu_chgnsav_epoch */

#if !(defined(TXQ_MUX) && !defined(WLAMPDU_MAC))
/* Fn is not needed if TXQ_MUX && !WLAMPDU_MAC */

/**
 * Check whether txparams have changed for ampdu tx, this would be a reason to complete the current
 * epoch.
 *
 * txparams include <frametype: 11n or 11vht>, <rate info in plcp>, <antsel>
 * Call this function *only* for aggregatable frames
 */
static bool
wlc_ampdu_epoch_params_chg(wlc_info_t *wlc, ampdumac_info_t *hagg, wlc_d11txh_u txh_u)
{
	uint8 *plcpPtr = NULL;
#if defined(WLC_UCODE_CACHE) && D11CONF_GE(42)
	uint16 mcl;
#endif
	bool chg;
	uint8 phy_ft, frametype = AMPDU_NONE;
	uint16 PhyTxControlWord_0 = 0;
	uint16 PhyTxControlWord_1 = 0;

	if (D11REV_LT(wlc->pub->corerev, 80)) {
		d11actxh_t *txh = txh_u.d11actxh;
		d11actxh_rate_t *rateInfo;
#if defined(WLC_UCODE_CACHE) && D11CONF_GE(42)
		mcl = ltoh16(tx_info.MacTxControlLow);


		if (mcl & D11AC_TXC_HDR_FMT_SHORT) {
			int cache_idx;

			cache_idx = (mcl & D11AC_TXC_CACHE_IDX_MASK) >> D11AC_TXC_CACHE_IDX_SHIFT;
			rateInfo = (d11actxh_rate_t *)wlc_txc_get_rate_info_shdr(wlc->txc,
				cache_idx);
		} else
#endif
#ifdef BCM_SFD
		if (SFD_ENAB(wlc->pub)) {
			rateInfo = wlc_sfd_get_rate_info(wlc->sfd, txh);
		} else
#endif
		{
			rateInfo = WLC_TXD_RATE_INFO_GET(txh, wlc->pub->corerev);
		}

		plcpPtr = rateInfo[0].plcp;
		PhyTxControlWord_0 = rateInfo[0].PhyTxControlWord_0;
		PhyTxControlWord_1 = rateInfo[0].PhyTxControlWord_1;

		/* map phy dependent frametype to independent frametype enum */
		phy_ft = ltoh16(PhyTxControlWord_0) & D11AC_PHY_TXC_FT_MASK;

		if (phy_ft == D11AC_PHY_TXC_FT_11N)
			frametype = AMPDU_11N;
		else if (phy_ft == D11AC_PHY_TXC_FT_11AC)
			frametype = AMPDU_11VHT;
		else {
			ASSERT(0);
			return TRUE;
		}

	}
#ifdef WL11AX
	else {
		uint8 *rateInfo;
		d11txh_rev80_t *txh = txh_u.d11txh_rev80;
		uint16 *FbwInfo;
		uint16 *PhyTxControlBlk;
		uint8 power_offs = 2;

#if defined(WLC_UCODE_CACHE)
		mcl = ltoh16(tx_info.MacTxControlLow);


		if (mcl & D11AC_TXC_HDR_FMT_SHORT) {
			/* TODO : Handle ucode caching for 11ax */
		} else
#endif
		{
			rateInfo = txh->RateInfoBlock;
			plcpPtr = (uint8 *)(((d11txh_rev80_rate_fixed_t *) rateInfo)->plcp);
		}

		FbwInfo = (uint16 *)(rateInfo + D11_REV80_TXH_FIXED_RATEINFO_LEN);

		/* TODO : power_offs to be calculated during run time */
		PhyTxControlBlk = (uint16 *)((uint8 *)FbwInfo +
			D11_REV80_TXH_BFM0_FIXED_LEN(power_offs));

		PhyTxControlWord_0 = PhyTxControlBlk[0];
		PhyTxControlWord_1 = PhyTxControlBlk[1];

		phy_ft = ltoh16(PhyTxControlWord_0) & D11_REV80_PHY_TXC_FT_MASK;
	}
#endif /* WL11AX */


	chg = (frametype != hagg->prev_ft) ||
		(PhyTxControlWord_0 != hagg->prev_txphyctl0) ||
		(PhyTxControlWord_1 != hagg->prev_txphyctl1);

	if (chg)
		return TRUE;

#if D11CONF_GE(40)
	if (frametype == AMPDU_11VHT) {
		/* VHT frame: compare plcp0-4 */
		if (plcpPtr[0] != hagg->prev_plcp[0] ||
		    plcpPtr[1] != hagg->prev_plcp[3] ||
		    plcpPtr[2] != hagg->prev_plcp[2] ||
		    plcpPtr[3] != hagg->prev_plcp[1] ||
		    plcpPtr[4] != hagg->prev_plcp[4])
			chg = TRUE;
	} else
#endif
	{
		/* HT frame: only care about plcp0 and plcp3 */
		if (plcpPtr[0] != hagg->prev_plcp[0] ||
		    plcpPtr[3] != hagg->prev_plcp[1])
			chg = TRUE;
	}

	return chg;
} /* wlc_ampdu_epoch_params_chg */

#endif /* !(TXQ_MUX && !WLAMPDU_MAC) */

typedef struct ampdu_creation_params
{
	wlc_bsscfg_t *cfg;
	scb_ampdu_tx_t *scb_ampdu;
	wlc_txh_info_t* tx_info;

	scb_ampdu_tid_ini_t *ini;
	ampdu_tx_info_t *ampdu;

} ampdu_create_info_t;

/**
 * Only called for AQM (AC) chips. Returns TRUE if MPDU 'idx' within a TxStatus
 * was acked by a remote party.
 */
static INLINE bool
wlc_txs_was_acked(wlc_info_t *wlc, tx_status_macinfo_t *status, uint16 idx)
{
	BCM_REFERENCE(wlc);
	ASSERT(idx < 64);
	/* Byte 4-11 of 2nd pkg */
	if (idx < 32) {
		return (status->ack_map1 & (1 << idx)) ? TRUE : FALSE;
	} else {
		idx -= 32;
		return (status->ack_map2 & (1 << idx)) ? TRUE : FALSE;
	}
}


#ifdef BCMDBG_ASSERT

#if !(defined(TXQ_MUX) && !defined(WLAMPDU_MAC))
/* Fn is not needed if TXQ_MUX && !WLAMPDU_MAC */

/**
 * Helper function to double-check the packet header prep for AQM (AC chips only).
 * This function verifies the prepared packet header (mostly the d11actxh_cache_t sub-structure)
 * against the current settings.
 * This is intended to help catch logic errors in packet header preparation and
 * the txc header caching logic.
 */
static bool
wlc_ampdu_check_percache_info(wlc_info_t * wlc, ampdu_tx_info_t *ampdu_tx,
	struct scb *scb, uint8 tid, uint8 *txh)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint16 ampdu_dur;
	d11txh_cache_common_t *cache_info;
	d11pktinfo_common_t *PktInfo;

	BCM_REFERENCE(wlc);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);

	ASSERT(scb_ampdu != NULL);
	ASSERT(tid < AMPDU_MAX_SCB_TID);

	ini = scb_ampdu->ini[tid];

	ampdu_dur = (ampdu_tx->config->dur & D11AC_AMPDU_MAX_DUR_MASK);
	ampdu_dur |= (scb_ampdu->mpdu_density << D11AC_AMPDU_MIN_DUR_IDX_SHIFT);

#ifdef WL11AX
	if (D11REV_GE(wlc->pub->corerev, 80)) {
		PktInfo = &((d11txh_rev80_t *)txh)->PktInfo;
	} else
#endif /* WL11AX */
		PktInfo = &((d11actxh_t *)txh)->PktInfo;

	if (ltoh16(PktInfo->MacTxControlLow) & D11AC_TXC_HDR_FMT_SHORT) {
		return TRUE;
	}

#ifdef WL11AX
	if (D11REV_GE(wlc->pub->corerev, 80)) {
		cache_info = &((d11txh_rev80_t *)txh)->CacheInfo.common;
	} else
#endif /* WL11AX */
	{
		d11actxh_cache_t *d11ac_cache_info;
#ifdef BCM_SFD
		if (SFD_ENAB(ampdu_tx->wlc->pub)) {
			d11ac_cache_info =
				wlc_sfd_get_cache_info(ampdu_tx->wlc->sfd, (d11actxh_t *)txh);
		} else
#endif
		{
			d11ac_cache_info =
			        WLC_TXD_CACHE_INFO_GET(((d11actxh_t *)txh), wlc->pub->corerev);
		}
		cache_info = &d11ac_cache_info->common;
	}

	return ((cache_info->PrimeMpduMax == scb_ampdu->max_pdu) &&
	        (cache_info->FallbackMpduMax == scb_ampdu->max_pdu) &&
	        (cache_info->BAWin == ini->ba_wsize - 1) &&
	        (cache_info->MaxAggLen == scb_ampdu->max_rxfactor) ?
	        TRUE : FALSE);
}


#endif /* !(TXQ_MUX && !WLAMPDU_MAC) */

#endif /* BCMDBG_ASSERT */

#ifdef WL_MUPKTENG
void
wlc_ampdu_mupkteng_fill_percache_info(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 tid,
	wlc_d11txh_cache_info_u txh_cache_info)
{
	scb_ampdu_tx_t *scb_ampdu;
	uint16 ampdu_dur;
	d11txh_cache_common_t	*cache_info;
#ifdef WL11AX
	if (D11REV_GE(ampdu_tx->wlc->pub->corerev, 80)) {
		d11txh_rev80_cache_t *d11_rev80_cache_info;
		 /* Abstraction for accessing new rev80 specific fields */
		d11_rev80_cache_info = txh_cache_info.rev80_CacheInfo;
		cache_info = &d11_rev80_cache_info->common;
	} else
#endif /* WL11AX */
	cache_info = &txh_cache_info.d11actxh_CacheInfo->common;

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	BCM_REFERENCE(scb_ampdu);

	cache_info->PrimeMpduMax = 64;
	cache_info->FallbackMpduMax = 64;

	/* maximum duration is in 16 usec, putting into upper 12 bits
	 * ampdu density is in low 4 bits
	 */

	ampdu_dur = (ampdu_tx->config->dur & D11AC_AMPDU_MAX_DUR_MASK);
	ampdu_dur |= (AMPDU_DEF_MPDU_DENSITY << D11AC_AMPDU_MIN_DUR_IDX_SHIFT);
	cache_info->AmpduDur = htol16(ampdu_dur);

	/* Fill in as BAWin of recvr -1 */
	cache_info->BAWin = 63;
	cache_info->MaxAggLen = 0;
}
#endif /* WL_MUPKTENG */
void
wlc_ampdu_fill_percache_info(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 tid,
	wlc_d11txh_cache_info_u txh_cache_info)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint16 ampdu_dur;
	uint16 scb_ampdu_min;
	d11txh_cache_common_t *cache_info;
#ifdef WL11AX
	if (D11REV_GE(ampdu_tx->wlc->pub->corerev, 80)) {
		d11txh_rev80_cache_t *d11_rev80_cache_info;
		/* Abstraction for accessing new rev80 specific fields */
		d11_rev80_cache_info = txh_cache_info.rev80_CacheInfo;
		cache_info = &d11_rev80_cache_info->common;
	} else
#endif /* WL11AX */
	cache_info = &txh_cache_info.d11actxh_CacheInfo->common;

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);

	ASSERT(scb_ampdu != NULL);
	ASSERT(tid < AMPDU_MAX_SCB_TID);

	ini = scb_ampdu->ini[tid];

	cache_info->PrimeMpduMax = scb_ampdu->max_pdu; /* max pdus allowed in ampdu */
	cache_info->FallbackMpduMax = scb_ampdu->max_pdu;

	if (scb_ampdu->min_of_max_dur_idx == AMPDU_MAXDUR_INVALID_IDX) {
		scb_ampdu_min = (uint16)(scb_ampdu->mpdu_density);
	} else {
		uint16 max_dur = scb_ampdu->module_max_dur[scb_ampdu->min_of_max_dur_idx];
		scb_ampdu_min = MIN(scb_ampdu->mpdu_density, max_dur);
	}

	if (BAND_2G(ampdu_tx->wlc->band->bandtype) &&
		(ampdu_tx->config->btcx_dur_flag || (scb->flags3 & SCB3_AWDL_AGGR_CHANGE))) {
		ampdu_dur = (BTCX_AMPDU_MAX_DUR & D11AC_AMPDU_MAX_DUR_MASK);
	} else {
		ampdu_dur = (ampdu_tx->config->dur & D11AC_AMPDU_MAX_DUR_MASK);
	}

	ampdu_dur |= (scb_ampdu_min << D11AC_AMPDU_MIN_DUR_IDX_SHIFT);

	cache_info->AmpduDur = htol16(ampdu_dur);

	/* Fill in as BAWin of recvr -1 */
	cache_info->BAWin = ini->ba_wsize - 1;
	/* max agg len in bytes, specified as 2^X */
	cache_info->MaxAggLen = scb_ampdu->max_rxfactor;
} /* wlc_ampdu_fill_percache_info */

#endif /* WLAMPDU_MAC */

#if !defined(TXQ_MUX)
/**
 * Called by higher layer when it wants to send an MSDU to the d11 core's DMA/FIFOs. Typically, an
 * MSDU is first given by a higher layer to the AMPDU subsystem for aggregation and subsequent
 * queuing in an AMPDU internal software queue (see wlc_ampdu_agg()). Next, the AMPDU subsystem
 * decides to 'release' the packet, which means that it forwards the packet to wlc.c, which adds it
 * to the wlc transmit queue.
 *
 * Subsequently, wlc.c calls this function in order to forward the packet towards the d11 core. The
 * MSDU is transformed into an MPDU in this function.
 *
 * Returns BCME_BUSY when there is no room to queue the packet.
 */
int BCMFASTPATH
wlc_sendampdu(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec,
	struct spktq *output_q, int *supplied_time)
{
	wlc_info_t *wlc;
	osl_t *osh;
	void *p;
	struct scb *scb;
	int err = 0;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint8 tid;

	ASSERT(ampdu_tx);
	ASSERT(qi);
	ASSERT(pdu);

	wlc = ampdu_tx->wlc;
	osh = wlc->osh;
	p = *pdu;
	ASSERT(p);
	if (p == NULL)
		return err;
	scb = WLPKTTAGSCBGET(p);
	ASSERT(scb != NULL);

	/* SCB setting may have changed during transition, so just discard the frame */
	if (!SCB_AMPDU(scb)) {
		WL_AMPDU(("wlc_sendampdu: entry func: scb ampdu flag null -- error\n"));
		/* ASSERT removed for 4360 as */
		/* the much longer tx pipeline (amsdu+ampdu) made it even more likely */
		/* to see this case */
		WLCNTINCR(ampdu_tx->cnt->orphan);
		wlc_tx_status_update_counters(wlc, p, scb,
			NULL, WLC_TX_STS_TOSS_NON_AMPDU_SCB, 1);
		PKTFREE(osh, p, TRUE);
		*pdu = NULL;
		return err;
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);

	ASSERT(scb_ampdu);

	tid = (uint8)PKTPRIO(p);
	ASSERT(tid < AMPDU_MAX_SCB_TID);

	ini = scb_ampdu->ini[tid];

	if (!ini || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
		WL_AMPDU(("wlc_sendampdu: bailing out for bad ini (%p)/bastate\n",
			OSL_OBFUSCATE_BUF(ini)));
		WLCNTINCR(ampdu_tx->cnt->orphan);
		if (ini) {
			wlc_ampdu_dec_bytes_inflight(ini, p);
		}
		wlc_tx_status_update_counters(wlc, p, scb,
			NULL, WLC_TX_STS_TOSS_BAD_INI, 1);
		PKTFREE(osh, p, TRUE);
		*pdu = NULL;
		return err;
	}

	/* Something is blocking data packets */
	if (wlc->block_datafifo) {
		WL_AMPDU(("wlc_sendampdu: datafifo blocked\n"));
		return BCME_BUSY;
	}


#if defined(WLAMPDU_MAC)
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		return _wlc_sendampdu_aqm(ampdu_tx, qi, pdu, prec, output_q, supplied_time);
	} else
#endif /* WLAMPDU_MAC */
#ifdef AMPDU_NON_AQM
	{
		return _wlc_sendampdu_noaqm(ampdu_tx, qi, pdu, prec, output_q, supplied_time);
	}
#endif /* AMPDU_NON_AQM */

	return err;
} /* wlc_sendampdu */
#endif /* TXQ_MUX */


#if !defined(TXQ_MUX)
#ifdef AMPDU_NON_AQM

#if AMPDU_MAX_MPDU > (TXFS_WSZ_AC_BE + TXFS_WSZ_AC_BK + TXFS_WSZ_AC_VI + \
	TXFS_WSZ_AC_VO)
#define AMPDU_MAX_PKTS	AMPDU_MAX_MPDU
#else
#define AMPDU_MAX_PKTS	(TXFS_WSZ_AC_BE + TXFS_WSZ_AC_BK + TXFS_WSZ_AC_VI + TXFS_WSZ_AC_VO)
#endif

/**
 * Invoked when higher layer tries to send a transmit SDU, or an MPDU (in case of retransmit), to
 * the d11 core. Not called for AC chips unless AQM is disabled. Returns BCME_BUSY when there is no
 * room to queue the packet.
 */
static int BCMFASTPATH
_wlc_sendampdu_noaqm(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec,
	struct spktq *output_q, int *supplied_time)
{
	wlc_info_t *wlc;
	osl_t *osh;
	void *p;
#ifdef WLAMPDU_HW
	void *pkt[AGGFIFO_CAP];
#else
	void *pkt[AMPDU_MAX_PKTS];
#endif
	uint8 tid, ndelim, ac;
	int err = 0, txretry = -1;
	uint8 preamble_type = WLC_GF_PREAMBLE;
	uint8 fbr_preamble_type = WLC_GF_PREAMBLE;
	uint8 rts_preamble_type = WLC_LONG_PREAMBLE;
	uint8 rts_fbr_preamble_type = WLC_LONG_PREAMBLE;

	bool rr = FALSE, fbr = FALSE, vrate_probe = FALSE;
	uint i, count = 0, fifo, npkts, nsegs, seg_cnt = 0, fifo_first = NFIFO;
	uint16 plen, len, seq, mcl, mch, indx, frameid, dma_len = 0;
	uint32 ampdu_len, maxlen = 0;
#ifdef WL11K
	uint32 pktlen = 0;
#endif
	d11txh_t *txh = NULL;
	uint8 *plcp;
	struct dot11_header *h;
	struct scb *scb;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint8 mcs = 0;
	bool regmpdu = FALSE;
	bool use_rts = FALSE, use_cts = FALSE;
	ratespec_t rspec = 0, rspec_fallback = 0;
	ratespec_t rts_rspec = 0, rts_rspec_fallback = 0;
	struct dot11_rts_frame *rts = NULL;
	uint8 rr_retry_limit;
	wlc_fifo_info_t *f;
	bool fbr_iscck, use_mfbr = FALSE;
	uint32 txop = 0;
#ifdef WLAMPDU_MAC
	ampdumac_info_t *hagg;
	int sc; /* Side channel index */
	uint aggfifo_space = 0;
#endif
	wlc_bsscfg_t *cfg;
	uint16 txburst_limit;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	bool suppress_pkt = FALSE;
#ifdef PKTC
	wlc_key_t *key = NULL;
	wlc_key_info_t key_info;
#endif /* PKTC */

	wlc = ampdu_tx->wlc;
	osh = wlc->osh;
	p = *pdu;

	tid = (uint8)PKTPRIO(p);
	ASSERT(tid < AMPDU_MAX_SCB_TID);
	ac = WME_PRIO2AC(tid);
	ASSERT(ac < AC_COUNT);
	f = ampdu_tx_cfg->fifo_tb + prio2fifo[tid];

	scb = WLPKTTAGSCBGET(p);
	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	ini = scb_ampdu->ini[tid];
	rr_retry_limit = ampdu_tx_cfg->rr_retry_limit_tid[tid];

	ASSERT(ini->scb == scb);

#ifdef WLAMPDU_MAC
	sc = prio2fifo[tid];
	hagg = &ampdu_tx->hagg[sc];

	/* agg-fifo space */
	if (AMPDU_MAC_ENAB(wlc->pub)) {
		aggfifo_space = get_aggfifo_space(ampdu_tx, sc);
		WL_AMPDU_HWDBG(("%s: aggfifo_space %d txfifo %d\n", __FUNCTION__, aggfifo_space,
			R_REG(wlc->osh, &wlc->regs->u.d11regs.xmtfifo_frame_cnt)));
#if (defined(BCMDBG) || defined(BCMDBG_AMPDU)) && defined(WLAMPDU_UCODE)
		ampdu_tx->amdbg->schan_depth_histo[MIN(AMPDU_MAX_MPDU, aggfifo_space)]++;
#endif
		if (aggfifo_space == 0) {
			return BCME_BUSY;
		}
		/* check if there are enough descriptors available */
		if (wlc->dma_avoidance_war)
			nsegs = pktsegcnt_war(osh, p);
		else
			nsegs = pktsegcnt(osh, p);
		if (TXAVAIL(wlc, sc) <= nsegs) {
			WLCNTINCR(ampdu_tx->cnt->txfifofull);
			return BCME_BUSY;
		}
	}
#endif /* WLAMPDU_MAC */

	/* compute txop once for all */
	txop = cfg->wme->edcf_txop[wme_fifo2ac[prio2fifo[tid]]];
	txburst_limit = WLC_HT_CFG_TXBURST_LIMIT(wlc->hti, cfg);

	if (txop && txburst_limit)
		txop = MIN(txop, txburst_limit);
	else if (txburst_limit)
		txop = txburst_limit;

	ampdu_len = 0;
	dma_len = 0;
	while (p) {
		ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);
		seq = WLPKTTAG(p)->seq;
#ifdef PROP_TXSTATUS
		seq = WL_SEQ_GET_NUM(seq);
#endif /* PROP_TXSTATUS */

		/* N.B.: WLF_MPDU flag indicates the MPDU is being retransmitted
		 * due to ampdu retry or d11 suppression.
		 */
		if (WLPKTTAG(p)->flags & WLF_MPDU) {
			err = wlc_prep_pdu(wlc, scb, p, &fifo);
		} else
#ifdef PKTC
		if ((err = wlc_prep_sdu_fast(wlc, cfg, scb, p, &fifo,
			&key, &key_info)) == BCME_UNSUPPORTED)
#endif /* PKTC */
		{
			/* prep_sdu converts an SDU into MPDU (802.11) format */
			err = wlc_prep_sdu(wlc, scb, &p, (int*)&npkts, &fifo);
		}

		if (count == 0) {
			fifo_first = fifo;
		} else {
			if (fifo != fifo_first)	{
				WL_AMPDU_ERR(("%s: fifo# %d for pkt%d (%s) does not match "
					"the first: %d. prep err=%d\n",
					__FUNCTION__, fifo, count,
					(WLPKTTAG(p)->flags & WLF_MPDU)?"pdu":"sdu",
					fifo_first, err));
#ifdef ENABLE_CORECAPTURE
				prhex("AMPDU pkt", PKTDATA(wlc->osh, p), PKTLEN(wlc->osh, p));
#endif /* ENABLE_CORECAPTURE */

				/* fix fifo, fake BCME_BUSY to re-queue p */
				fifo = fifo_first;
				err = BCME_BUSY;
			}
		}

		if (err) {
			if (err == BCME_BUSY) {
				WL_AMPDU_TX(("wl%d: wlc_sendampdu: prep_xdu retry; seq 0x%x\n",
					wlc->pub->unit, seq));
				WLCNTINCR(ampdu_tx->cnt->sduretry);
				*pdu = p;
				break;
			}

			/* error in the packet; reject it */
			WL_AMPDU_ERR(("wl%d: wlc_sendampdu: prep_xdu rejected; seq 0x%x\n",
				wlc->pub->unit, seq));
			WLCNTINCR(ampdu_tx->cnt->sdurejected);
			AMPDUSCBCNTINCR(scb_ampdu->cnt.sdurejected);

			/* send bar since this may be blocking pkts in scb */
			if (ini->tx_exp_seq == seq) {
				setbit(ini->barpending, TX_SEQ_TO_INDEX(seq));
				wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
				ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
			}
			*pdu = NULL;
			break;
		}

		WL_AMPDU_TX(("wl%d: wlc_sendampdu: prep_xdu success; seq 0x%x tid %d\n",
			wlc->pub->unit, seq, tid));

		/* pkt is good to be aggregated */

		ASSERT(WLPKTTAG(p)->flags & WLF_MPDU);
		ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

		txh = (d11txh_t *)PKTDATA(osh, p);
		plcp = (uint8 *)(txh + 1);
		h = (struct dot11_header*)(plcp + D11_PHY_HDR_LEN);
		seq = ltoh16(h->seq) >> SEQNUM_SHIFT;
		indx = TX_SEQ_TO_INDEX(seq);

		/* during roam we get pkts with null a1. kill it here else
		 * does not find scb on dotxstatus and things gets out of sync
		 */
		if (ETHER_ISNULLADDR(&h->a1)) {
			WL_AMPDU_ERR(("wl%d: sendampdu; dropping frame with null a1\n",
				wlc->pub->unit));
			WLCNTINCR(ampdu_tx->cnt->txdrop);
			if (ini->tx_exp_seq == seq) {
				setbit(ini->barpending, indx);
				wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
				ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
			}

			wlc_tx_status_update_counters(wlc, p,
				scb, NULL, WLC_TX_STS_TOSS_BAD_ADDR, 1);

			PKTFREE(osh, p, TRUE);
			err = BCME_ERROR;
			*pdu = NULL;
			break;
		}

	BCM_REFERENCE(suppress_pkt);

#ifdef PROP_TXSTATUS
		if (IS_WL_TO_REUSE_SEQ(WLPKTTAG(p)->seq)) {
			suppress_pkt = TRUE;
			/* Now the dongle owns the packet */
			RESET_WL_TO_REUSE_SEQ(WLPKTTAG(p)->seq);
		}
#endif /* PROP_TXSTATUS */

		/* If the packet was not retrieved from HW FIFO before attempting
		 * over the air for multi-queue or p2p
		 */
		if (WLPKTTAG(p)->flags & WLF_FIFOPKT) {
			/* seq # in suppressed frames have been validated before
			 * so don't validate them again...unless there are queuing
			 * issue(s) causing them to be out-of-order.
			 */
		} else if (!ampdu_is_exp_seq(ampdu_tx, ini, seq, suppress_pkt)) {
			wlc_tx_status_update_counters(wlc, p,
				scb, NULL, WLC_TX_STS_TOSS_INV_SEQ, 1);

			PKTFREE(osh, p, TRUE);
			err = BCME_ERROR;
			*pdu = NULL;
			break;
		}

		if (!isset(ini->ackpending, indx)) {
			WL_AMPDU_ERR(("wl%d: %s: seq 0x%x\n",
			          ampdu_tx->wlc->pub->unit, __FUNCTION__, seq));
			ampdu_dump_ini(ini);
			ASSERT(isset(ini->ackpending, indx));
		}

		/* check mcl fields and test whether it can be agg'd */
		mcl = ltoh16(txh->MacTxControlLow);
		mcl &= ~TXC_AMPDU_MASK;
		fbr_iscck = !(ltoh16(txh->XtraFrameTypes) & 0x3);
		txh->PreloadSize = 0; /* always default to 0 */

		/*  Handle retry limits */
		if (AMPDU_HOST_ENAB(wlc->pub)) {

			/* first packet in ampdu */
			if (txretry == -1) {
			    txretry = ini->txretry[indx];
			    vrate_probe = (WLPKTTAG(p)->flags & WLF_VRATE_PROBE) ? TRUE : FALSE;
			}

			ASSERT(txretry == ini->txretry[indx]);

			if ((txretry == 0) ||
			    (txretry < rr_retry_limit &&
			     !(vrate_probe && ampdu_tx_cfg->probe_mpdu))) {
				rr = TRUE;
				ASSERT(!fbr);
			} else {
				/* send as regular mpdu if not a mimo frame or in ps mode
				 * or we are in pending off state
				 * make sure the fallback rate can be only HT or CCK
				 */
				fbr = TRUE;
				ASSERT(!rr);

				regmpdu = fbr_iscck;
				if (regmpdu) {
					mcl &= ~(TXC_SENDRTS | TXC_SENDCTS | TXC_LONGFRAME);
					mcl |= TXC_STARTMSDU;
					txh->MacTxControlLow = htol16(mcl);

					mch = ltoh16(txh->MacTxControlHigh);
					mch |= TXC_AMPDU_FBR;
					txh->MacTxControlHigh = htol16(mch);
					break;
				}
			}
		} /* AMPDU_HOST_ENAB */

		/* extract the length info */
		len = fbr_iscck ? WLC_GET_CCK_PLCP_LEN(txh->FragPLCPFallback)
			: WLC_GET_MIMO_PLCP_LEN(txh->FragPLCPFallback);

		if (!(WLPKTTAG(p)->flags & WLF_MIMO) ||
		    (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF) ||
		    (ini->ba_state == AMPDU_TID_STATE_BA_OFF) ||
		    SCB_PS(scb)) {
			/* clear ampdu related settings if going as regular frame */
			if ((WLPKTTAG(p)->flags & WLF_MIMO) &&
			    (SCB_PS(scb) ||
			     (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF) ||
			     (ini->ba_state == AMPDU_TID_STATE_BA_OFF))) {
				WLC_CLR_MIMO_PLCP_AMPDU(plcp);
				WLC_SET_MIMO_PLCP_LEN(plcp, len);
			}
			regmpdu = TRUE;
			mcl &= ~(TXC_SENDRTS | TXC_SENDCTS | TXC_LONGFRAME);
			txh->MacTxControlLow = htol16(mcl);
			break;
		}

		/* pkt is good to be aggregated */

		if (ini->txretry[indx]) {
			WLCNTINCR(ampdu_tx->cnt->u0.txretry_mpdu);
			if (AMPDU_HOST_ENAB(wlc->pub)) {
				WLCNTSCBINCR(scb->scb_stats.tx_pkts_retries);
				WLCNTINCR(wlc->pub->_cnt->txretrans);
			}
		}

		/* retrieve null delimiter count */
		ndelim = txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM];
		if (wlc->dma_avoidance_war)
			seg_cnt += pktsegcnt_war(osh, p);
		else
			seg_cnt += pktsegcnt(osh, p);

		if (!WL_AMPDU_HW_ON())
			WL_AMPDU_TX(("wl%d: wlc_sendampdu: mpdu %d plcp_len %d\n",
				wlc->pub->unit, count, len));

#ifdef WLAMPDU_MAC
		/*
		 * aggregateable mpdu. For ucode/hw agg,
		 * test whether need to break or change the epoch
		 */
		if (AMPDU_MAC_ENAB(wlc->pub)) {
			uint8 max_pdu;

			ASSERT((uint)sc == fifo);

			/* check if link or tid has changed */
			if (scb != hagg->prev_scb || tid != hagg->prev_tid) {
				hagg->prev_scb = scb;
				hagg->prev_tid = tid;
				wlc_ampdu_change_epoch(ampdu_tx, sc, AMU_EPOCH_CHG_DSTTID);
			}

			/* mark every MPDU's plcp to indicate ampdu */
			WLC_SET_MIMO_PLCP_AMPDU(plcp);

			/* rate/sgi/bw/etc phy info */
			if (plcp[0] != hagg->prev_plcp[0] ||
			    (plcp[3] & 0xf0) != hagg->prev_plcp[1]) {
				hagg->prev_plcp[0] = plcp[0];
				hagg->prev_plcp[1] = plcp[3] & 0xf0;
				wlc_ampdu_change_epoch(ampdu_tx, sc, AMU_EPOCH_CHG_PLCP);
			}
			/* frameid -- force change by the rate selection algo */
			frameid = ltoh16(txh->TxFrameID);
			if (frameid & TXFID_RATE_PROBE_MASK) {
				wlc_ampdu_change_epoch(ampdu_tx, sc, AMU_EPOCH_CHG_FID);
				wlc_scb_ratesel_probe_ready(wlc->wrsi, scb, frameid, TRUE, 0, ac);
			}

			/* Set bits 9:10 to any nonzero value to indicate to
			 * ucode that this packet can be aggregated
			 */
			mcl |= (TXC_AMPDU_FIRST << TXC_AMPDU_SHIFT);
			/* set sequence number in tx-desc */
			txh->AmpduSeqCtl = htol16(h->seq);

			/*
			 * compute aggregation parameters
			 */
			/* limit on number of mpdus per ampdu */
			mcs = plcp[0] & ~MIMO_PLCP_40MHZ;
			max_pdu = scb_ampdu->max_pdu;

			/* MaxAggSize in duration(usec) and bytes for hw/ucode agg, respectively */
			if (AMPDU_HW_ENAB(wlc->pub)) {
				/* use duration limit in usec */
				uint32 maxdur;
				maxdur = ampdu_tx_cfg->dur ?
					(uint32)ampdu_tx_cfg->dur : 0xffff;
				if (txop && (txop < maxdur))
					maxdur = txop;
				if ((maxdur & 0xffff0000) != 0)
					maxdur = 0xffff;
				txh->u1.MaxAggDur = htol16(maxdur & 0xffff);
				ASSERT(txh->u1.MaxAggDur != 0);
				/* hack for now :
				 * should compute from xmtfifo_sz[fifo] * 256 / max_bytes_per_mpdu
				 * e.g. fifo1: floor(255 * 256, 1650)
				 */
				txh->u2.s1.MaxRNum =
					MIN((uint8)htol16(wlc->xmtfifo_frmmaxh[fifo]),
					ini->ba_wsize);
				ASSERT(txh->u2.s1.MaxRNum > 0);
				txh->u2.s1.MaxAggBytes = scb_ampdu->max_rxfactor;
			} else {
				if (MCS_RATE(mcs, TRUE, FALSE) >= f->dmaxferrate) {
					uint16 preloadsize = f->ampdu_pld_size;
					if (D11REV_IS(wlc->pub->corerev, 29) ||
					    D11REV_GE(wlc->pub->corerev, 31))
						preloadsize >>= 2;
					txh->PreloadSize = htol16(preloadsize);
					if (max_pdu > f->mcs2ampdu_table[MCS2IDX(mcs)])
						max_pdu = f->mcs2ampdu_table[MCS2IDX(mcs)];
				}
				txh->u1.MaxAggLen = htol16(wlc_ampdu_calc_maxlen(ampdu_tx,
					plcp[0], plcp[3], txop));
				if (!fbr_iscck)
					txh->u2.MaxAggLen_FBR =
						htol16(wlc_ampdu_calc_maxlen(ampdu_tx,
						txh->FragPLCPFallback[0], txh->FragPLCPFallback[3],
						txop));
				else
					txh->u2.MaxAggLen_FBR = 0;
			}

			txh->MaxNMpdus = htol16((max_pdu << 8) | max_pdu);
			/* retry limits - overload MModeFbrLen */
			if (WLPKTTAG(p)->flags & WLF_VRATE_PROBE) {
				txh->MModeFbrLen = htol16((1 << 8) |
					(ampdu_tx_cfg->retry_limit_tid[tid] & 0xff));
				txh->MaxNMpdus = htol16((max_pdu << 8) |
					(ampdu_tx_cfg->probe_mpdu));
			} else {
				txh->MModeFbrLen = htol16(((rr_retry_limit & 0xff) << 8) |
					(ampdu_tx_cfg->retry_limit_tid[tid] & 0xff));
			}

			WL_AMPDU_HW(("wl%d: %s len %d MaxNMpdus 0x%x null delim %d MinMBytes %d "
				     "MaxAggDur %d MaxRNum %d rr_lmnt 0x%x\n", wlc->pub->unit,
				     __FUNCTION__, len, txh->MaxNMpdus, ndelim, txh->MinMBytes,
				     txh->u1.MaxAggDur, txh->u2.s1.MaxRNum, txh->MModeFbrLen));

			/* Ucode agg:
			 * length to be written into side channel includes:
			 * leading delimiter, mac hdr to fcs, padding, null delimiter(s)
			 * i.e. (len + 4 + 4*num_paddelims + padbytes)
			 * padbytes is num required to round to 32 bits
			 */
			if (AMPDU_UCODE_ENAB(wlc->pub)) {
				if (ndelim) {
					len = ROUNDUP(len, 4);
					len += (ndelim  + 1) * AMPDU_DELIMITER_LEN;
				} else {
					len += AMPDU_DELIMITER_LEN;
				}
			}

			/* flip the epoch? */
			if (hagg->change_epoch) {
				hagg->epoch = !hagg->epoch;
				hagg->change_epoch = FALSE;
				WL_AMPDU_HWDBG(("sendampdu: fifo %d new epoch: %d frameid %x\n",
					sc, hagg->epoch, frameid));
				WLCNTINCR(ampdu_tx->cnt->epochdelta);
			}
			aggfifo_enque(ampdu_tx, len, sc);
		} else
#endif /* WLAMPDU_MAC */
		{ /* host agg */
			if (count == 0) {
				uint16 fc;
				mcl |= (TXC_AMPDU_FIRST << TXC_AMPDU_SHIFT);
				/* refill the bits since might be a retx mpdu */
				mcl |= TXC_STARTMSDU;
				rts = (struct dot11_rts_frame*)&txh->rts_frame;
				fc = ltoh16(rts->fc);
				if ((fc & FC_KIND_MASK) == FC_RTS) {
					mcl |= TXC_SENDRTS;
					use_rts = TRUE;
				}
				if ((fc & FC_KIND_MASK) == FC_CTS) {
					mcl |= TXC_SENDCTS;
					use_cts = TRUE;
				}
			} else {
				mcl |= (TXC_AMPDU_MIDDLE << TXC_AMPDU_SHIFT);
				mcl &= ~(TXC_STARTMSDU | TXC_SENDRTS | TXC_SENDCTS);
			}

			len = ROUNDUP(len, 4);
			ampdu_len += (len + (ndelim + 1) * AMPDU_DELIMITER_LEN);
			ASSERT(ampdu_len <= scb_ampdu->max_rxlen);
			dma_len += (uint16)pkttotlen(osh, p);

			WL_AMPDU_TX(("wl%d: wlc_sendampdu: ampdu_len %d seg_cnt %d null delim %d\n",
				wlc->pub->unit, ampdu_len, seg_cnt, ndelim));
		}
		txh->MacTxControlLow = htol16(mcl);

		/* this packet is added */
		pkt[count++] = p;
#ifdef WL11K
		pktlen += pkttotlen(osh, p);
#endif

		/* patch the first MPDU */
		if (AMPDU_HOST_ENAB(wlc->pub) && count == 1) {
			uint8 plcp0, plcp3, is40, sgi;
			uint32 txoplen = 0;

			if (rr) {
				plcp0 = plcp[0];
				plcp3 = plcp[3];
			} else {
				plcp0 = txh->FragPLCPFallback[0];
				plcp3 = txh->FragPLCPFallback[3];

				/* Support multiple fallback rate:
				 * For txtimes > rr_retry_limit, use fallback -> fallback.
				 * To get around the fix-rate case check if primary == fallback rate
				 */
				if (ini->txretry[indx] > rr_retry_limit && ampdu_tx_cfg->mfbr &&
				    plcp[0] != plcp0) {
					ratespec_t fbrspec;
					uint8 fbr_mcs;
					uint16 phyctl1;

					use_mfbr = TRUE;
					txh->FragPLCPFallback[3] &=  ~PLCP3_STC_MASK;
					plcp3 = txh->FragPLCPFallback[3];
					fbrspec = wlc_scb_ratesel_getmcsfbr(wlc->wrsi, scb,
						ac, plcp0);
					fbr_mcs = RSPEC_RATE_MASK & fbrspec;

					/* restore bandwidth */
					fbrspec &= ~RSPEC_BW_MASK;
					switch (ltoh16(txh->PhyTxControlWord_1_Fbr)
						 & PHY_TXC1_BW_MASK)
					{
						case PHY_TXC1_BW_20MHZ:
						case PHY_TXC1_BW_20MHZ_UP:
							fbrspec |= RSPEC_BW_20MHZ;
						break;
						case PHY_TXC1_BW_40MHZ:
						case PHY_TXC1_BW_40MHZ_DUP:
							fbrspec |= RSPEC_BW_40MHZ;
						break;
						default:
							ASSERT(0);
					}
					if ((RSPEC_IS40MHZ(fbrspec) &&
						(CHSPEC_IS20(wlc->chanspec))) ||
						(RSPEC_ISCCK(fbrspec))) {
						fbrspec &= ~RSPEC_BW_MASK;
						fbrspec |= RSPEC_BW_20MHZ;
					}

					if (IS_SINGLE_STREAM(fbr_mcs)) {
						fbrspec &= ~(RSPEC_TXEXP_MASK | RSPEC_STBC);

						/* For SISO MCS use STBC if possible */
						if (WLC_IS_STBC_TX_FORCED(wlc) ||
							(RSPEC_ISHT(fbrspec) &&
							WLC_STF_SS_STBC_HT_AUTO(wlc, scb))) {
							uint8 stc = 1;

							ASSERT(WLC_STBC_CAP_PHY(wlc));
							fbrspec |= RSPEC_STBC;

							/* also set plcp bits 29:28 */
							plcp3 = (plcp3 & ~PLCP3_STC_MASK) |
							        (stc << PLCP3_STC_SHIFT);
							txh->FragPLCPFallback[3] = plcp3;
							WLCNTINCR(ampdu_tx->cnt->txampdu_mfbr_stbc);
						} else if (wlc->stf->ss_opmode == PHY_TXC1_MODE_CDD)
						{
							fbrspec |= (1 << RSPEC_TXEXP_SHIFT);
						}
					}
					/* rspec returned from getmcsfbr() doesn't have SGI info. */
					if (WLC_HT_GET_SGI_TX(wlc->hti) == ON)
						fbrspec |= RSPEC_SHORT_GI;
					phyctl1 = wlc_phytxctl1_calc(wlc, fbrspec, wlc->chanspec);
					txh->PhyTxControlWord_1_Fbr = htol16(phyctl1);

					txh->FragPLCPFallback[0] = fbr_mcs |
						(plcp0 & MIMO_PLCP_40MHZ);
					plcp0 = txh->FragPLCPFallback[0];
				}
			}
			is40 = (plcp0 & MIMO_PLCP_40MHZ) ? 1 : 0;
			sgi = PLCP3_ISSGI(plcp3) ? 1 : 0;
			mcs = plcp0 & ~MIMO_PLCP_40MHZ;
			ASSERT(VALID_MCS(mcs));
			maxlen = MIN(scb_ampdu->max_rxlen,
				ampdu_tx_cfg->max_txlen[MCS2IDX(mcs)][is40][sgi]);

			/* take into account txop and tx burst limit restriction */
			if (txop) {
				/* rate is in Kbps; txop is in usec
				 * ==> len = (rate * txop) / (1000 * 8)
				 */
				txoplen = (MCS_RATE(mcs, is40, sgi) >> 10) * (txop >> 3);
				maxlen = MIN(maxlen, txoplen);
			}

			/* rebuild the rspec and rspec_fallback */
			rspec = HT_RSPEC(plcp[0] & MIMO_PLCP_MCS_MASK);

			if (plcp[0] & MIMO_PLCP_40MHZ)
				rspec |= RSPEC_BW_40MHZ;

			if (fbr_iscck) /* CCK */
				rspec_fallback =
					CCK_RSPEC(CCK_PHY2MAC_RATE(txh->FragPLCPFallback[0]));
			else { /* MIMO */
				rspec_fallback =
				        HT_RSPEC(txh->FragPLCPFallback[0] & MIMO_PLCP_MCS_MASK);
				if (txh->FragPLCPFallback[0] & MIMO_PLCP_40MHZ)
					rspec_fallback |= RSPEC_BW_40MHZ;
			}

			if (use_rts || use_cts) {
				uint16 fc;
				rts_rspec = wlc_rspec_to_rts_rspec(cfg, rspec, FALSE);
				rts_rspec_fallback = wlc_rspec_to_rts_rspec(cfg, rspec_fallback,
				                                            FALSE);
				/* need to reconstruct plcp and phyctl words for rts_fallback */
				if (use_mfbr) {
					int rts_phylen;
					uint8 rts_plcp_fallback[D11_PHY_HDR_LEN], old_ndelim;
					uint16 phyctl1, xfts;

					old_ndelim = txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM];
					phyctl1 = wlc_phytxctl1_calc(wlc, rts_rspec_fallback,
						wlc->chanspec);
					txh->PhyTxControlWord_1_FbrRts = htol16(phyctl1);
					if (use_cts)
						rts_phylen = DOT11_CTS_LEN + DOT11_FCS_LEN;
					else
						rts_phylen = DOT11_RTS_LEN + DOT11_FCS_LEN;

					fc = ltoh16(rts->fc);
					wlc_compute_plcp(wlc, cfg, rts_rspec_fallback, rts_phylen,
					fc, rts_plcp_fallback);

					bcopy(rts_plcp_fallback, (char*)&txh->RTSPLCPFallback,
						sizeof(txh->RTSPLCPFallback));

					/* restore it */
					txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM] = old_ndelim;

					/* update XtraFrameTypes in case it has changed */
					xfts = ltoh16(txh->XtraFrameTypes);
					xfts &= ~(PHY_TXC_FT_MASK << XFTS_FBRRTS_FT_SHIFT);
					xfts |= ((RSPEC_ISCCK(rts_rspec_fallback) ?
					          FT_CCK : FT_OFDM) << XFTS_FBRRTS_FT_SHIFT);
					txh->XtraFrameTypes = htol16(xfts);
				}
			}
		} /* if (first mpdu for host agg) */

		/* test whether to add more */
		if (AMPDU_HOST_ENAB(wlc->pub)) {
			if ((MCS_RATE(mcs, TRUE, FALSE) >= f->dmaxferrate) &&
			    (count == f->mcs2ampdu_table[MCS2IDX(mcs)])) {
				WL_AMPDU_ERR(("wl%d: PR 37644: stopping ampdu at %d for mcs %d\n",
					wlc->pub->unit, count, mcs));
				break;
			}

			/* limit the size of ampdu for probe packets */
			if (vrate_probe && (count == ampdu_tx_cfg->probe_mpdu))
				break;

			if (count == scb_ampdu->max_pdu)
				break;
		}
#ifdef WLAMPDU_MAC
		else {
			if (count >= aggfifo_space) {
				uint space;
				space = get_aggfifo_space(ampdu_tx, sc);
				if (space > 0)
					aggfifo_space += space;
				else {
					WL_AMPDU_HW(("sendampdu: break due to aggfifo full."
					     "count %d space %d\n", count, aggfifo_space));
					break;
				}
			}
		}
#endif /* WLAMPDU_MAC */

		/* You can process maximum of AMPDU_MAX_PKTS/AGGFIFO_CAP packets at a time. */
#ifdef WLAMPDU_HW
		if (count >= AGGFIFO_CAP)
			break;
#else
		if (count >= AMPDU_MAX_PKTS)
			break;
#endif

		/* check to see if the next pkt is a candidate for aggregation */
		p = pktqprec_peek(WLC_GET_TXQ(qi), prec);

		if (ampdu_tx_cfg->hiagg_mode) {
			if (!p && (prec & 1)) {
				prec = prec & ~1;
				p = pktqprec_peek(WLC_GET_TXQ(qi), prec);
			}
		}
		if (p) {
			if (WLPKTFLAG_AMPDU(WLPKTTAG(p)) &&
				(WLPKTTAGSCBGET(p) == scb) &&
				((uint8)PKTPRIO(p) == tid)) {
				if (wlc->dma_avoidance_war)
					nsegs = pktsegcnt_war(osh, p);
				else
					nsegs = pktsegcnt(osh, p);
				WL_NONE(("%s: txavail %d  < seg_cnt %d nsegs %d\n", __FUNCTION__,
					TXAVAIL(wlc, fifo), seg_cnt, nsegs));
				/* check if there are enough descriptors available */
				if (TXAVAIL(wlc, fifo) <= (seg_cnt + nsegs)) {
					WLCNTINCR(ampdu_tx->cnt->txfifofull);
					p = NULL;
					continue;
				}

				if (AMPDU_HOST_ENAB(wlc->pub)) {
					plen = pkttotlen(osh, p) + AMPDU_MAX_MPDU_OVERHEAD;
					plen = MAX(scb_ampdu->min_len, plen);
					if ((plen + ampdu_len) > maxlen) {
						p = NULL;
						continue;
					}

					ASSERT((rr && !fbr) || (!rr && fbr));
#ifdef PROP_TXSTATUS
					indx = TX_SEQ_TO_INDEX(WL_SEQ_GET_NUM(WLPKTTAG(p)->seq));
#else
					indx = TX_SEQ_TO_INDEX(WLPKTTAG(p)->seq);
#endif /* PROP_TXSTATUS */
					if (ampdu_tx_cfg->hiagg_mode) {
						if (((ini->txretry[indx] < rr_retry_limit) &&
						     fbr) ||
						    ((ini->txretry[indx] >= rr_retry_limit) &&
						     rr)) {
							p = NULL;
							continue;
						}
					} else {
						if (txretry != ini->txretry[indx]) {
							p = NULL;
							continue;
						}
					}
				}

				p = pktq_pdeq(WLC_GET_TXQ(qi), prec);
				ASSERT(p);
			} else {
#ifdef WLAMPDU_MAC
				if (AMPDU_MAC_ENAB(wlc->pub))
					wlc_ampdu_change_epoch(ampdu_tx, sc, AMU_EPOCH_CHG_DSTTID);
#endif
				p = NULL;
			}
		}
	}  /* end while(p) */

	if (count) {

		WL_AMPDU_HWDBG(("%s: tot_mpdu %d txfifo %d\n", __FUNCTION__, count,
			R_REG(wlc->osh, &wlc->regs->u.d11regs.xmtfifo_frame_cnt)));
		WLCNTADD(ampdu_tx->cnt->txmpdu, count);
		AMPDUSCBCNTADD(scb_ampdu->cnt.txmpdu, count);
#ifdef WL11K
		WLCNTADD(wlc->ampdu_tx->cnt->txampdubyte_l, pktlen);
		if (wlc->ampdu_tx->cnt->txampdubyte_l < pktlen)
			WLCNTINCR(wlc->ampdu_tx->cnt->txampdubyte_h);
#endif
		if (AMPDU_HOST_ENAB(wlc->pub)) {
#ifdef WLCNT
			ampdu_tx->cnt->txampdu++;
#endif
#if defined(BCMDBG) || defined(BCMDBG_AMPDU)
			if (ampdu_tx->amdbg)
				ampdu_tx->amdbg->txmcs[MCS2IDX(mcs)]++;
#endif 
#ifdef WLPKTDLYSTAT
			WLCNTADD(ampdu_tx->cnt->txmpdu_cnt[MCS2IDX(mcs)], count);
#endif /* WLPKTDLYSTAT */

			/* patch up the last txh */
			txh = (d11txh_t *)PKTDATA(osh, pkt[count - 1]);
			mcl = ltoh16(txh->MacTxControlLow);
			mcl &= ~TXC_AMPDU_MASK;
			mcl |= (TXC_AMPDU_LAST << TXC_AMPDU_SHIFT);
			txh->MacTxControlLow = htol16(mcl);

			/* remove the null delimiter after last mpdu */
			ndelim = txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM];
			txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM] = 0;
			ampdu_len -= ndelim * AMPDU_DELIMITER_LEN;

			/* remove the pad len from last mpdu */
			fbr_iscck = ((ltoh16(txh->XtraFrameTypes) & 0x3) == 0);
			len = fbr_iscck ? WLC_GET_CCK_PLCP_LEN(txh->FragPLCPFallback)
				: WLC_GET_MIMO_PLCP_LEN(txh->FragPLCPFallback);
			ampdu_len -= ROUNDUP(len, 4) - len;

			/* patch up the first txh & plcp */
			txh = (d11txh_t *)PKTDATA(osh, pkt[0]);
			plcp = (uint8 *)(txh + 1);

			WLC_SET_MIMO_PLCP_LEN(plcp, ampdu_len);
			/* mark plcp to indicate ampdu */
			WLC_SET_MIMO_PLCP_AMPDU(plcp);

			/* reset the mixed mode header durations */
			if (txh->MModeLen) {
				uint16 mmodelen = wlc_calc_lsig_len(wlc, rspec, ampdu_len);
				txh->MModeLen = htol16(mmodelen);
				preamble_type = WLC_MM_PREAMBLE;
			}
			if (txh->MModeFbrLen) {
				uint16 mmfbrlen = wlc_calc_lsig_len(wlc, rspec_fallback, ampdu_len);
				txh->MModeFbrLen = htol16(mmfbrlen);
				fbr_preamble_type = WLC_MM_PREAMBLE;
			}

			/* set the preload length */
			if (MCS_RATE(mcs, TRUE, FALSE) >= f->dmaxferrate) {
				dma_len = MIN(dma_len, f->ampdu_pld_size);
				if (D11REV_IS(wlc->pub->corerev, 29) ||
				    D11REV_GE(wlc->pub->corerev, 31))
					dma_len >>= 2;
				txh->PreloadSize = htol16(dma_len);
			} else
				txh->PreloadSize = 0;

			mch = ltoh16(txh->MacTxControlHigh);

			/* update RTS dur fields */
			if (use_rts || use_cts) {
				uint16 durid;
				rts = (struct dot11_rts_frame*)&txh->rts_frame;
				if ((mch & TXC_PREAMBLE_RTS_MAIN_SHORT) ==
				    TXC_PREAMBLE_RTS_MAIN_SHORT)
					rts_preamble_type = WLC_SHORT_PREAMBLE;

				if ((mch & TXC_PREAMBLE_RTS_FB_SHORT) == TXC_PREAMBLE_RTS_FB_SHORT)
					rts_fbr_preamble_type = WLC_SHORT_PREAMBLE;

				durid = wlc_compute_rtscts_dur(wlc,
					CHSPEC2WLC_BAND(cfg->current_bss->chanspec), use_cts,
					rts_rspec, rspec, rts_preamble_type, preamble_type,
					ampdu_len, TRUE);
				rts->durid = htol16(durid);
				durid = wlc_compute_rtscts_dur(wlc,
					CHSPEC2WLC_BAND(cfg->current_bss->chanspec), use_cts,
					rts_rspec_fallback, rspec_fallback, rts_fbr_preamble_type,
					fbr_preamble_type, ampdu_len, TRUE);
				txh->RTSDurFallback = htol16(durid);
				/* set TxFesTimeNormal */
				txh->TxFesTimeNormal = rts->durid;
				/* set fallback rate version of TxFesTimeNormal */
				txh->TxFesTimeFallback = txh->RTSDurFallback;
			}

			/* set flag and plcp for fallback rate */
			if (fbr) {
				WLCNTADD(ampdu_tx->cnt->txfbr_mpdu, count);
				WLCNTINCR(ampdu_tx->cnt->txfbr_ampdu);
				mch |= TXC_AMPDU_FBR;
				txh->MacTxControlHigh = htol16(mch);
				WLC_SET_MIMO_PLCP_AMPDU(plcp);
				WLC_SET_MIMO_PLCP_AMPDU(txh->FragPLCPFallback);
				if (PLCP3_ISSGI(txh->FragPLCPFallback[3])) {
					WLCNTINCR(ampdu_tx->cnt->u1.txampdu_sgi);
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMPDU)
					if (ampdu_tx->amdbg)
						ampdu_tx->amdbg->txmcssgi[MCS2IDX(mcs)]++;
#endif 
				}
				if (PLCP3_ISSTBC(txh->FragPLCPFallback[3])) {
					WLCNTINCR(ampdu_tx->cnt->u2.txampdu_stbc);
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMPDU)
					if (ampdu_tx->amdbg)
						ampdu_tx->amdbg->txmcsstbc[MCS2IDX(mcs)]++;
#endif 
				}
			} else {
				if (PLCP3_ISSGI(plcp[3])) {
					WLCNTINCR(ampdu_tx->cnt->u1.txampdu_sgi);
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMPDU)
					if (ampdu_tx->amdbg)
						ampdu_tx->amdbg->txmcssgi[MCS2IDX(mcs)]++;
#endif 
				}
				if (PLCP3_ISSTBC(plcp[3])) {
					WLCNTINCR(ampdu_tx->cnt->u2.txampdu_stbc);
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMPDU)
					if (ampdu_tx->amdbg)
						ampdu_tx->amdbg->txmcsstbc[MCS2IDX(mcs)]++;
#endif
				}
			}

			if (txretry)
				WLCNTINCR(ampdu_tx->cnt->txretry_ampdu);

			WL_AMPDU_TX(("wl%d: wlc_sendampdu: count %d ampdu_len %d\n",
				wlc->pub->unit, count, ampdu_len));

			/* inform rate_sel if it this is a rate probe pkt */
			frameid = ltoh16(txh->TxFrameID);
			if (frameid & TXFID_RATE_PROBE_MASK) {
				wlc_scb_ratesel_probe_ready(wlc->wrsi, scb, frameid, TRUE,
					(uint8)txretry, ac);
			}
		}

		/* pass on all packets to the consumer */
		for (i = 0; i < count; i++) {
			spktenq(output_q, pkt[i]);
		}
		/* If SW Agg, fake a time of txpkt_weight for the ampdu,
		 * otherwise MAC Agg uses just the pkt count
		 */
		*supplied_time +=
			((AMPDU_MAC_ENAB(wlc->pub)) ? count : ampdu_tx_cfg->txpkt_weight);
	} /* endif (count) */

	if (regmpdu) {
		WL_AMPDU_TX(("wl%d: wlc_sendampdu: sending regular mpdu\n", wlc->pub->unit));

		/* mark the pkt regmpdu */
		WLPKTTAG(p)->flags3 |= WLF3_AMPDU_REGMPDU;

		/* inform rate_sel if it this is a rate probe pkt */
		txh = (d11txh_t *)PKTDATA(osh, p);
		frameid = ltoh16(txh->TxFrameID);
		if (frameid & TXFID_RATE_PROBE_MASK) {
			wlc_scb_ratesel_probe_ready(wlc->wrsi, scb, frameid, FALSE, 0, ac);
		}

		WLCNTINCR(ampdu_tx->cnt->txregmpdu);

		spktenq(output_q, p);
		/* fake a time of 1 for each pkt */
		*supplied_time += 1;

#ifdef WLAMPDU_MAC
		if (AMPDU_MAC_ENAB(wlc->pub))
			wlc_ampdu_change_epoch(ampdu_tx, sc, AMU_EPOCH_CHG_NAGG);
#endif
	}

	return err;
} /* _wlc_sendampdu_noaqm */

#endif /* AMPDU_NON_AQM */

#endif /* TXQ_MUX */

#ifdef IQPLAY_DEBUG
static void
wlc_set_sequmber(wlc_info_t *wlc, uint16 seq_num)
{
	scb_ampdu_tid_ini_t *ini;
	scb_ampdu_tx_t *scb_ampdu;
	int idx;
	wlc_bsscfg_t *cfg;
	struct scb_iter scbiter;
	struct scb *scb;
	ampdu_tx_info_t *ampdu_tx = wlc->ampdu_tx;
	uint8 tid;

	FOREACH_BSS(wlc, idx, cfg) {
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
			if (!SCB_AMPDU(scb))
				continue;
			scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
			ASSERT(scb_ampdu);

			for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
				ini = scb_ampdu->ini[tid];
				if (!ini)
					continue;
				ini->scb->seqnum[tid] = seq_num;
				wlc_send_addba_req(wlc, ini->scb, tid,
					ampdu_tx->config->ba_tx_wsize, ampdu_tx->config->ba_policy,
					ampdu_tx->config->delba_timeout);
			}
		}
	}
}
#endif /* IQPLAY_DEBUG */

#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */

#ifdef WL_CS_PKTRETRY
INLINE static void
wlc_txh_set_chanspec(wlc_info_t* wlc, wlc_txh_info_t* tx_info, chanspec_t new_chanspec)
{
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		d11txh_t* nonVHTHdr = (d11txh_t*)(tx_info->hdrPtr);
		uint16 xtra_frame_types = ltoh16(nonVHTHdr->XtraFrameTypes);

		xtra_frame_types &= ~(CHSPEC_CHANNEL(0xFFFF) << XFTS_CHANNEL_SHIFT);
		xtra_frame_types |= CHSPEC_CHANNEL(new_chanspec) << XFTS_CHANNEL_SHIFT;

		nonVHTHdr->XtraFrameTypes = htol16(xtra_frame_types);
	} else {
		d11actxh_t* vhtHdr = (d11actxh_t*)(tx_info->hdrPtr);
		vhtHdr->PktInfo.Chanspec = htol16(new_chanspec);
	}
}

/** PSPRETEND related feature */
INLINE static void
wlc_ampdu_chanspec_update(wlc_info_t *wlc, void *p)
{
	wlc_txh_info_t txh_info;


	wlc_get_txh_info(wlc, p, &txh_info);

	wlc_txh_set_chanspec(wlc, &txh_info, wlc->chanspec);
}

/** PSPRETEND related feature */
INLINE static bool
wlc_ampdu_cs_retry(wlc_info_t *wlc, tx_status_t *txs, void *p)
{
	const uint supr_status = txs->status.suppr_ind;

	bool pktRetry = TRUE;
#ifdef PROP_TXSTATUS
	/* pktretry will return TRUE on basis of following conditions:
	 * 1: if PROP_TX_STATUS is not enabled OR
	 * 2: if HOST_PROP_TXSTATUS is not enabled OR
	 * 3: if suppresed packet is not from host OR
	 * 4: if suppresed packet is from host AND requested one from Firmware
	 */

	if (!PROP_TXSTATUS_ENAB(wlc->pub) ||
		!HOST_PROPTXSTATUS_ACTIVATED(wlc) ||
		!(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(p)->wl_hdr_information) &
		WLFC_PKTFLAG_PKTFROMHOST) ||
		((WL_TXSTATUS_GET_FLAGS(WLPKTTAG(p)->wl_hdr_information) &
		WLFC_PKTFLAG_PKTFROMHOST) &&
		(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(p)->wl_hdr_information) &
		WLFC_PKTFLAG_PKT_REQUESTED))) {
		pktRetry = !txs->status.was_acked;
	} else {
		pktRetry = FALSE;
	}
#endif /* ifdef PROP_TXSTATUS */


	if (!(wlc->block_datafifo & DATA_BLOCK_QUIET)) {
		/* Not during channel transition time */
		return FALSE;
	}

	switch (supr_status) {
	case TX_STATUS_SUPR_BADCH:
	case TX_STATUS_SUPR_PPS:
		return pktRetry;
	case 0:
		/*
		 * Packet was not acked.
		 * Likely STA already has changed channel.
		 */
		return (pktRetry ? !txs->status.was_acked : pktRetry);
	default:
		return FALSE;
	}
} /* wlc_ampdu_cs_retry */
#endif /* WL_CS_PKTRETRY */

#endif /* WLAMPDU_MAC */

#if !defined(TXQ_MUX)

#ifdef WLAMPDU_MAC

/**
 * Called when a higher layer wants to send an SDU (or suppressed PDU) to the d11 core's FIFOs/DMA.
 * Only called for AC chips when AQM is enabled. Typically, an MSDU is first given by a higher layer
 * to the AMPDU subsystem for aggregation and subsequent queuing in an AMPDU internal software queue
 * (see ampdu_agg()). Next, the AMPDU subsystem decides to 'release' the packet, which means that it
 * forwards the MSDU to wlc.c, which adds it to the wlc transmit queue. Subsequently, wlc.c calls
 * the AMPDU subsytem which calls this function in order to forward the packet towards the d11 core.
 * In the course of this function, the caller provided MSDU is transformed into an MPDU.
 *
 * Please note that this function can also send 'regular' (= not contained in an AMPDU) MPDUs.
 *
 * Parameters:
 *     output_q : queue to which the caller supplied packet is added
 *
 * Returns BCME_BUSY when there is no room to queue the packet.
 */
static int BCMFASTPATH
_wlc_sendampdu_aqm(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec,
	struct spktq *output_q, int *supplied_time)
{
	wlc_info_t *wlc;
	osl_t* osh;
	void* p;
	struct scb *scb;
	uint8 tid, ac;
	scb_ampdu_tid_ini_t *ini;
	wlc_bsscfg_t *cfg = NULL;
	scb_ampdu_tx_t *scb_ampdu;
	ampdumac_info_t *hagg;
	wlc_txh_info_t tx_info;
	int err = 0;
	uint fifo, npkts,  fifoavail = 0;
	int count = 0;
	uint16 seq;
	struct dot11_header* h;
	bool fill_txh_info = FALSE;
	wlc_pkttag_t* pkttag;
	bool is_suppr_pkt = FALSE;
	uint8* txh = NULL;
	d11actxh_t *vhtHdr;
#ifdef WL11AX
	d11txh_rev80_t *heHdr;
#endif /* WL11AX */
	wlc_key_t *key = NULL;
	wlc_key_info_t key_info;
#ifdef WL11K
	uint32 pktlen = 0;
#endif

	BCM_REFERENCE(fifoavail);
#ifdef WL11AX
	BCM_REFERENCE(vhtHdr);
	BCM_REFERENCE(heHdr);
#endif /* WL11AX */

	wlc = ampdu_tx->wlc;
	osh = wlc->osh;
	p = *pdu;

	ASSERT(p != NULL);

	/*
	 * since we obtain <scb, tid, fifo> etc outside the loop,
	 * have to break and return if either of them has changed
	 */
	tid = (uint8)PKTPRIO(p);
	ASSERT(tid < AMPDU_MAX_SCB_TID);
	fifo = prio2fifo[tid];
	ac = WME_PRIO2AC(tid);
	ASSERT(ac < AC_COUNT);
	scb = WLPKTTAGSCBGET(p);
	cfg = SCB_BSSCFG(scb);

#ifdef WL_MU_TX
	if ((BSSCFG_AP(cfg) && SCB_MU(scb) && MU_TX_ENAB(wlc))) {
		wlc_mutx_sta_txfifo(wlc->mutx, scb, &fifo);
	}
#endif
	ASSERT(fifo < NFIFO_EXT);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ini = scb_ampdu->ini[tid];

	pkttag = WLPKTTAG(p);
	/* loop over MPDUs. loop exits via break. */
	while (TRUE) {
		bool regmpdu_pkt = FALSE;
		bool reque_pkt = FALSE;
		bool has_seq_hole = FALSE;
		bool s_mpdu_pdu = FALSE;

#ifdef BCMDBG_SWWLAN_38455
		if ((WLPKTTAG(p)->flags & WLF_AMPDU_MPDU) == 0) {
			WL_PRINT(("%s flags=%x seq=%d fifo=%d\n", __FUNCTION__,
				WLPKTTAG(p)->flags, WLPKTTAG(p)->seq, fifo));
		}
#endif /* BCMDBG_SWWLAN_38455 */

		ASSERT(pkttag->flags & WLF_AMPDU_MPDU);

		seq = pkttag->seq;
#ifdef PROP_TXSTATUS
		seq = WL_SEQ_GET_NUM(seq);

#ifdef BCMDBG_ASSERT
#ifdef BCMPCIEDEV
		/* Sanity check to ensure suppressed MSDUs are aggregated back together */
		if (BCMPCIEDEV_ENAB() && PROP_TXSTATUS_ENAB(wlc->pub) &&
			WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wlfc))) {
			if (WLPKTFLAG_AMSDU(WLPKTTAG(p)) &&
				IS_WL_TO_REUSE_SEQ(WLPKTTAG(p)->seq)) {
				uint16 amsdu_seq1 = WLPKTTAG(p)->seq & WL_SEQ_AMSDU_SUPPR_MASK;
				uint16 amsdu_seqn;
				void *amsdu_n = PKTNEXT(wlc->osh, p);
				ASSERT(amsdu_n); /* minimum two for ASMDU */

				do {
					amsdu_seqn = WLPKTTAG(amsdu_n)->seq &
						WL_SEQ_AMSDU_SUPPR_MASK;

					if (amsdu_seq1 != amsdu_seqn) {
					   WL_PRINT(("ERR: Suppr ASMDU seq: 0x%p:0x%x 0x%p:0x%x\n",
					   p, amsdu_seq1, amsdu_n, amsdu_seqn));
					}

					/* AMSDU could be > 2 subframes */
					amsdu_n = PKTNEXT(wlc->osh, amsdu_n);
				} while (amsdu_n != NULL);
			}
		}
#endif /* BCMPCIEDEV */
#endif /* BCMDBG_ASSERT */
#endif /* PROP_TXSTATUS */

		if (!wlc_scb_restrict_can_txq(wlc, scb)) {
			err = BCME_BUSY;
		}
		/* N.B.: WLF_MPDU flag indicates the MPDU is being retransmitted
		 * due to d11 suppression
		 */
		else if (pkttag->flags & WLF_MPDU) {
			err = wlc_prep_pdu(wlc, scb, p, &fifo);
			reque_pkt = TRUE;
			wlc_get_txh_info(wlc, p, &tx_info);
			if (!wlc_txh_get_isAMPDU(wlc, &tx_info))
				s_mpdu_pdu = TRUE;
		} else if ((err = wlc_prep_sdu_fast(wlc, cfg, scb, p, &fifo,
			&key, &key_info)) == BCME_UNSUPPORTED) {
			/* prep_sdu converts an SDU into MPDU (802.11) format */
			err = wlc_prep_sdu(wlc, scb, &p, (int*)&npkts, &fifo);
			pkttag = WLPKTTAG(p);
		}

		/* Update the ampdumac pointer to point correct fifo */
		hagg = &ampdu_tx->hagg[fifo];

		if (err) {
			if (err == BCME_BUSY) {
				WL_AMPDU_TX(("wl%d: %s prep_xdu retry; seq 0x%x\n",
					wlc->pub->unit, __FUNCTION__, seq));
				WLCNTINCR(ampdu_tx->cnt->txfifofull);
				WLCNTINCR(ampdu_tx->cnt->sduretry);
				*pdu = p;
				break;
			}

			/* error in the packet; reject it */
			WL_AMPDU_ERR(("wl%d: wlc_sendampdu: prep_xdu rejected; seq 0x%x\n",
				wlc->pub->unit, seq));
			WLCNTINCR(ampdu_tx->cnt->sdurejected);
			AMPDUSCBCNTINCR(scb_ampdu->cnt.sdurejected);

			/* let ampdu_is_exp_seq() catch it, update tx_exp_seq, and send_bar */
			*pdu = NULL;
			break;
		}
		WL_AMPDU_TX(("wl%d: %s: prep_xdu success; seq 0x%x tid %d\n",
			wlc->pub->unit, __FUNCTION__, seq, tid));

		regmpdu_pkt = !(pkttag->flags & WLF_MIMO) ||
		        (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF) ||
		        (ini->ba_state == AMPDU_TID_STATE_BA_OFF) ||
		        SCB_PS(scb) || s_mpdu_pdu;

		/* if no error, populate tx hdr (TSO/ucode header) info and continue */
		ASSERT(pkttag->flags & WLF_MPDU);
		if (!fill_txh_info) {
			fill_txh_info = (count == 0 ||
			                 reque_pkt ||
			                 regmpdu_pkt ||
			                 (pkttag->flags & WLF_TXCMISS) != 0);
		}
		if (fill_txh_info) {
			wlc_get_txh_info(wlc, p, &tx_info);

			if (D11REV_LT(wlc->pub->corerev, 80)) {
				vhtHdr = &(tx_info.hdrPtr->txd);
				txh = (uint8 *)vhtHdr;
			}
#ifdef WL11AX
			else {
				heHdr = &(tx_info.hdrPtr->rev80_txd);
				txh = (uint8 *)heHdr;
			}
#endif /* WL11AX */
		} else {
			int tsoHdrSize;

			if (D11REV_LT(wlc->pub->corerev, 80)) {
				d11actxh_rate_t * rateInfo;

				tsoHdrSize = wlc_pkt_get_vht_hdr(wlc, p, &vhtHdr);
				txh = (uint8 *)vhtHdr;
#ifdef WLTOEHW
				tx_info.tsoHdrSize = tsoHdrSize;
				tx_info.tsoHdrPtr = (void*)((tsoHdrSize != 0) ?
					PKTDATA(wlc->osh, p) : NULL);
#endif /* WLTOEHW */

				tx_info.hdrPtr = (wlc_txd_t *)(PKTDATA(wlc->osh, p) + tsoHdrSize);

				tx_info.TxFrameID = vhtHdr->PktInfo.TxFrameID;
				tx_info.MacTxControlLow = vhtHdr->PktInfo.MacTxControlLow;
				tx_info.MacTxControlHigh = vhtHdr->PktInfo.MacTxControlHigh;
#if defined(WLC_UCODE_CACHE) && D11CONF_GE(42)
				if (ltoh16(tx_info.MacTxControlLow) & D11AC_TXC_HDR_FMT_SHORT) {
					int cache_idx = (ltoh16(tx_info.MacTxControlLow) &
						D11AC_TXC_CACHE_IDX_MASK) >>
						D11AC_TXC_CACHE_IDX_SHIFT;
					wlc_txc_info_t *txc = wlc->txc;

					rateInfo = (d11actxh_rate_t *)
						wlc_txc_get_rate_info_shdr(txc,	cache_idx);
					tx_info.hdrSize = D11AC_TXH_SHORT_LEN;
				 } else
#endif
#ifdef BCM_SFD
				if (SFD_ENAB(wlc->pub) && PKTISSFDFRAME(wlc->osh, p)) {
					rateInfo = wlc_sfd_get_rate_info(wlc->sfd, vhtHdr);
					tx_info.hdrSize = D11AC_TXH_SFD_LEN;
				} else
#endif
				{
					rateInfo = WLC_TXD_RATE_INFO_GET(vhtHdr,
					wlc->pub->corerev);
					tx_info.hdrSize = D11AC_TXH_LEN;
				}
				tx_info.d11HdrPtr = ((uint8 *)tx_info.hdrPtr) + tx_info.hdrSize;
				tx_info.plcpPtr = (rateInfo[0].plcp);
				tx_info.TxFrameRA = (uint8*)(((struct dot11_header *)
					(tx_info.d11HdrPtr))->a1.octet);
			}
#ifdef WL11AX
			else {
				uint8 *rateInfo;

				tsoHdrSize = wlc_pkt_get_he_hdr(wlc, p, &heHdr);
				txh = (uint8 *)heHdr;
#ifdef WLTOEHW
				tx_info.tsoHdrSize = tsoHdrSize;
				tx_info.tsoHdrPtr = (void*)((tsoHdrSize != 0) ?
					PKTDATA(wlc->osh, p) : NULL);
#endif /* WLTOEHW */

				tx_info.hdrPtr = (wlc_txd_t *)(PKTDATA(wlc->osh, p) + tsoHdrSize);

				tx_info.TxFrameID = heHdr->PktInfo.TxFrameID;
				tx_info.MacTxControlLow = heHdr->PktInfo.MacTxControlLow;
				tx_info.MacTxControlHigh = heHdr->PktInfo.MacTxControlHigh;
#if defined(WLC_UCODE_CACHE)
				if (ltoh16(tx_info.MacTxControlLow) & D11AC_TXC_HDR_FMT_SHORT) {
					/* TODO : Handle ucode caching for 11ax */
				 } else
#endif
				 {
					rateInfo = heHdr->RateInfoBlock;
					tx_info.hdrSize = heHdr->PktInfoExt.length;
				 }
				tx_info.d11HdrPtr = ((uint8 *)tx_info.hdrPtr) + tx_info.hdrSize;
				tx_info.plcpPtr = (uint8 *)(((d11txh_rev80_rate_fixed_t *)
					rateInfo)->plcp);
				tx_info.TxFrameRA = (uint8*)(((struct dot11_header *)
					(tx_info.d11HdrPtr))->a1.octet);
			}
#endif /* WL11AX */
		}

		h = (struct dot11_header*)(tx_info.d11HdrPtr);
		seq = ltoh16(h->seq) >> SEQNUM_SHIFT;

#ifdef PROP_TXSTATUS
		if (IS_WL_TO_REUSE_SEQ(pkttag->seq)) {
			is_suppr_pkt = TRUE;
			/* Now the dongle owns the packet */
			RESET_WL_TO_REUSE_SEQ(pkttag->seq);
		} else {
			is_suppr_pkt = FALSE;
		}

#endif /* PROP_TXSTATUS */

		/* during roam we get pkts with null a1. kill it here else
		 * does not find scb on dotxstatus and things gets out of sync
		 */
		if (ETHER_ISNULLADDR(&h->a1)) {
			WL_AMPDU_ERR(("wl%d: %s: dropping frame with null a1\n",
				wlc->pub->unit, __FUNCTION__));
			err = BCME_ERROR;
			wlc_tx_status_update_counters(wlc, p,
				scb, NULL, WLC_TX_STS_TOSS_BAD_ADDR, 1);

		} else if (pkttag->flags & WLF_FIFOPKT) {
#ifdef WL_CS_PKTRETRY
			wlc_ampdu_chanspec_update(wlc, p);
#endif
			/* seq # in suppressed frames have been validated before
			 * so don't validate them again...unless there are queuing
			 * issue(s) causing them to be out-of-order.
			 */
#ifdef PROP_TXSTATUS
#ifdef WL_NATOE
		} else if (NATOE_ENAB(wlc->pub) &&
				WL_SEQ_GET_VALIDSUPPR(ini->last_suppr_seq) &&
				PKTISFRWDPKT(wlc->pub->osh, p)) {
			/* Dropping forward pkts, till all suppressed pkts are finished
			 * when using WLFC_GET_REUSESEQ feature. Forward pkts generated
			 * will get fresh seq number which is greater than tx_exp_seq
			 * resulting in BAR transmit. Then if suppressed pkts from host are
			 * transmitted, receiver will drop them as its window is moved due
			 * to BAR packet.
			 */
			WL_AMPDU_TX(("%s: drop frwdpkt to allow suppr pkts\n", __FUNCTION__));
			err = BCME_ERROR;
#endif /* WL_NATOE */
#endif /* PROP_TXSTATUS */
		} else if (!ampdu_is_exp_seq(ampdu_tx, ini, seq, is_suppr_pkt)) {
			/* If the packet was not retrieved from HW FIFO before
			 * attempting over the air for multi-queue
			 */
			WL_ERROR(("%s: p %p seq %x pktflags %x\n", __FUNCTION__,
				p, WLPKTTAG(p)->seq, WLPKTTAG(p)->flags));
			WL_AMPDU_TX(("%s: multi-queue error\n", __FUNCTION__));
			err = BCME_ERROR;

			wlc_tx_status_update_counters(wlc, p,
				scb, NULL, WLC_TX_STS_TOSS_INV_SEQ, 1);
		}

		if (err == BCME_ERROR) {
			WLCNTINCR(ampdu_tx->cnt->txdrop);
			wlc_ampdu_dec_bytes_inflight(ini, p);
			PKTFREE(osh, p, TRUE);
			*pdu = NULL;
			break;
		}

#ifdef WL11AX
		/* Add frame type to TSO header for (D11 core rev >= 80) AQM design */
		if (D11REV_GE(wlc->pub->corerev, 80)) {
			uint16 fc_type = FC_TYPE(ltoh16(h)->fc);
			wlc_txh_set_ft(wlc, tx_info.tsoHdrPtr, fc_type);
			wlc_txh_set_frag_allow(wlc, tx_info.tsoHdrPtr,
				(fc_type == FC_TYPE_DATA));
		}
#endif /* WL11AX */

		/* 'hole' if sequence does not succeed pre-pending MPDU */
		has_seq_hole = ampdu_detect_seq_hole(ampdu_tx, seq, ini);

		if (regmpdu_pkt) {
			WL_AMPDU_TX(("%s: reg mpdu found\n", __FUNCTION__));

			/* clear ampdu bit, set svht bit */
			tx_info.MacTxControlLow &= htol16(~D11AC_TXC_AMPDU);
			/* non-ampdu must have svht bit set */
			tx_info.MacTxControlHigh |= htol16(D11AC_TXC_SVHT);

			((d11actxh_t *)txh)->PktInfo.MacTxControlLow =
				tx_info.MacTxControlLow;
			((d11actxh_t *)txh)->PktInfo.MacTxControlHigh =
				tx_info.MacTxControlHigh;

			if (hagg->prev_ft != AMPDU_NONE) {
				/* ampdu switch to regmpdu */
				wlc_ampdu_chgnsav_epoch(ampdu_tx, fifo,
					AMU_EPOCH_CHG_NAGG, scb, tid, &tx_info);
				wlc_txh_set_epoch(ampdu_tx->wlc, tx_info.tsoHdrPtr, hagg->epoch);
				hagg->prev_ft = AMPDU_NONE;
			}
			/* if prev is regmpdu already, don't bother to set epoch at all */

			/* mark the MPDU is regmpdu */
			pkttag->flags3 |= WLF3_AMPDU_REGMPDU;
		} else {
			bool change = FALSE;
			int8 reason_code = 0;

			/* clear svht bit, set ampdu bit */
			tx_info.MacTxControlLow |= htol16(D11AC_TXC_AMPDU);
			/* for real ampdu, clear vht single mpdu ampdu bit */
			tx_info.MacTxControlHigh &= htol16(~D11AC_TXC_SVHT);

			((d11actxh_t *)txh)->PktInfo.MacTxControlLow =
				tx_info.MacTxControlLow;
			((d11actxh_t *)txh)->PktInfo.MacTxControlHigh =
				tx_info.MacTxControlHigh;

#ifdef BCMDBG_ASSERT
			ASSERT(wlc_ampdu_check_percache_info(wlc, ampdu_tx, scb, tid, txh));
#endif /* BCMDBG_ASSERT */

			if (fill_txh_info) {
				uint16 frameid;
				wlc_d11txh_u txh_u;
				int8* tsoHdrPtr = tx_info.tsoHdrPtr;

				change = TRUE;
				frameid = ltoh16(tx_info.TxFrameID);

				if (D11REV_LT(wlc->pub->corerev, 80)) {
					txh_u.d11actxh = vhtHdr;
				}
#ifdef WL11AX
				else {
					txh_u.d11txh_rev80 = heHdr;
				}
#endif /* WL11AX */
				if (frameid & TXFID_RATE_PROBE_MASK) {
					/* check vrate probe flag
					 * this flag is only get the frame to become first mpdu
					 * of ampdu and no need to save epoch_params
					 */
					wlc_scb_ratesel_probe_ready(wlc->wrsi, scb, frameid,
					                            FALSE, 0, ac);
					reason_code = AMU_EPOCH_CHG_FID;
				} else if (scb != hagg->prev_scb || tid != hagg->prev_tid) {
					reason_code = AMU_EPOCH_CHG_DSTTID;
				} else if (wlc_ampdu_epoch_params_chg(wlc, hagg, txh_u)) {
					/* rate/sgi/bw/stbc/antsel tx params */
					reason_code = AMU_EPOCH_CHG_PLCP;
				} else if (ltoh16(tx_info.MacTxControlLow) & D11AC_TXC_UPD_CACHE) {
					reason_code = AMU_EPOCH_CHG_TXC_UPD;
				} else if ((tsoHdrPtr != NULL) &&
				           ((tsoHdrPtr[2] & TOE_F2_TXD_HEAD_SHORT) !=
				            hagg->prev_shdr)) {
					reason_code = AMU_EPOCH_CHG_TXHDR;
				} else {
					change = FALSE;
				}
				fill_txh_info = FALSE;
			}

			if (!change) {
				if (hagg->prev_ft == AMPDU_NONE) {
					/* switching from non-aggregate to aggregate ampdu,
					 * just update the epoch changing parameters(hagg) but
					 * not the epoch.
					 */
					change = TRUE;
					reason_code = AMU_EPOCH_NO_CHANGE;
				} else if (has_seq_hole) {
					/* Flip the epoch if there is a hole in sequence of pkts
					 * sent to txfifo. It is a AQM HW requirement
					 * not to have the holes in txfifo.
					 */
					change = TRUE;
					reason_code = AMU_EPOCH_CHG_SEQ;
				}
			}

			if (change) {
				if (ltoh16(tx_info.MacTxControlLow) & D11AC_TXC_UPD_CACHE) {
					fill_txh_info = TRUE;
				}
				wlc_ampdu_chgnsav_epoch(ampdu_tx, fifo, reason_code,
					scb, tid, &tx_info);
			}

			/* always set epoch for ampdu */
			wlc_txh_set_epoch(ampdu_tx->wlc, tx_info.tsoHdrPtr, hagg->epoch);
		}

		spktenq(output_q, p);
		/* fake a time of 1 for each pkt */
		*supplied_time += 1;
		count++;
#ifdef WL11K
		pktlen += pkttotlen(osh, p);
#endif

		/* check to see if the next pkt is a candidate for aggregation */
		p = pktqprec_peek(WLC_GET_TXQ(qi), prec);

		if (p != NULL) {
#ifdef BCMPCIEDEV_ENABLED
			if (!wlc->cmn->hostmem_access_enabled &&
				BCMLFRAG_ENAB() && PKTISTXFRAG(osh, p)) {
				/* Do not process the Host originated LFRAG tx since no host
				* memory access is allowed in PCIE D3 suspend
				*/
				break;
			}
#endif /* BCMPCIEDEV_ENABLED */

			pkttag = PKTTAG(p);
			if (WLPKTFLAG_AMPDU(pkttag) &&
				(pkttag->_scb == scb) &&
				((uint8)PKTPRIO(p) == tid)) {
				p = pktq_pdeq(WLC_GET_TXQ(qi), prec);
				ASSERT(p);
			} else {
				break;
			}
		} else {
			break;
		}
	} /* end while(TRUE): loop over MPDUs */

	if (count) {
#ifdef WLCNT
		ASSERT(ampdu_tx->cnt);
#endif
		AMPDUSCBCNTADD(scb_ampdu->cnt.txmpdu, count);
#ifdef WL11K
		WLCNTADD(wlc->ampdu_tx->cnt->txampdubyte_l, pktlen);
		if (wlc->ampdu_tx->cnt->txampdubyte_l < pktlen)
			WLCNTINCR(wlc->ampdu_tx->cnt->txampdubyte_h);
#endif
	}

	WL_AMPDU_TX(("%s: fifo %d count %d epoch %d txpktpend %d",
		__FUNCTION__, fifo, count, ampdu_tx->hagg[fifo].epoch,
		TXPKTPENDGET(wlc, fifo)));
	if (D11REV_GE(wlc->pub->corerev, 64)) {
		WL_AMPDU_TX((" psm_reg_mux 0x%x aqmqmap 0x%x aqmfifo_status 0x%x\n",
			R_REG(osh, &wlc->regs->u.d11acregs.psm_reg_mux),
			R_REG(osh, &wlc->regs->u.d11acregs.AQMQMAP),
			R_REG(osh, &wlc->regs->u.d11acregs.AQMFifo_Status)));
	} else {
		WL_AMPDU_TX((" fifocnt 0x%x\n",
			R_REG(wlc->osh, &wlc->regs->u.d11acregs.XmtFifoFrameCnt)));
	}

	return err;
} /* _wlc_sendampdu_aqm */

#endif /* WLAMPDU_MAC */

#endif /* TXQ_MUX */

/**
 * Requests the 'communication partner' to send a block ack, so that on reception the transmit
 * window can be advanced.
 *
 * If 'start' is set move window to start_seq
 */
static void
wlc_ampdu_send_bar(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini, bool start)
{
	int indx, i;
	scb_ampdu_tx_t *scb_ampdu;
	void *p = NULL;
	uint16 seq;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;

	/* stop retry BAR if subsequent tx has made the BAR seq. stale  */
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub) &&
		(ini->retry_bar != AMPDU_R_BAR_DISABLED)) {
		uint16 offset, stop_retry = 0;

		WL_AMPDU(("retry bar for reason %x, bar_seq 0x%04x, max_seq 0x%04x,"
			"acked_seq 0x%04x, bar_cnt %d, tx_in_transit %d\n",
			ini->retry_bar, ini->barpending_seq, ini->max_seq, ini->acked_seq,
			ini->bar_cnt, ini->tx_in_transit));

		ini->retry_bar = AMPDU_R_BAR_DISABLED;
		offset = MODSUB_POW2(ini->acked_seq, ini->barpending_seq, SEQNUM_MAX);
		if ((offset > ini->ba_wsize) && offset < SEQNUM_MAX / 2)
			stop_retry = 1;
		/* It is very much possible that bar_pending seq is more than */
		/* max seq by at most 1 */
		if (MODSUB_POW2(ini->max_seq + 1, ini->barpending_seq, SEQNUM_MAX)
			>= SEQNUM_MAX / 2)
			stop_retry = 1;
		if (stop_retry) {
			WL_AMPDU(("wl%d: stop retry bar %x\n", ampdu_tx->wlc->pub->unit,
				ini->barpending_seq));
			ini->barpending_seq = SEQNUM_MAX;
			return;
		}
	}
	ini->retry_bar = AMPDU_R_BAR_DISABLED;
	if (ini->bar_ackpending)
		return;

	if (ini->ba_state != AMPDU_TID_STATE_BA_ON)
		return;

	if (ini->bar_cnt >= AMPDU_BAR_RETRY_CNT)
		return;

	if (ini->bar_cnt == AMPDU_BAR_RETRY_CNT / 2) {
		ini->bar_cnt++;
		ini->retry_bar = AMPDU_R_BAR_HALF_RETRY_CNT;
		return;
	}

	if (start)
		seq = ini->start_seq;
	else {
		int offset = -1;

		if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
			indx = TX_SEQ_TO_INDEX(ini->start_seq);
			for (i = 0; i < (ini->ba_wsize - ini->rem_window); i++) {
				if (isset(ini->barpending, indx))
					offset = i;
				indx = NEXT_TX_INDEX(indx);
			}

			if (offset == -1) {
				ini->bar_cnt = 0;
				return;
			}

			seq = MODADD_POW2(ini->start_seq, offset + 1, SEQNUM_MAX);
		} else {
			seq = ini->barpending_seq;
			ASSERT(ini->barpending_seq < SEQNUM_MAX);
		}
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, ini->scb);
	ASSERT(scb_ampdu);
	BCM_REFERENCE(scb_ampdu);

	if (ampdu_tx_cfg->no_bar == FALSE) {
		bool blocked;
		p = wlc_send_bar(ampdu_tx->wlc, ini->scb, ini->tid, seq,
		                 DOT11_BA_CTL_POLICY_NORMAL, TRUE, &blocked);
		if (blocked) {
			ini->retry_bar = AMPDU_R_BAR_BLOCKED;
			return;
		}

		if (p == NULL) {
			ini->retry_bar = AMPDU_R_BAR_NO_BUFFER;
			return;
		}

		/* Cancel any pending wlc_send_bar_complete packet callbacks. */
		wlc_pcb_fn_find(ampdu_tx->wlc->pcb, wlc_send_bar_complete, ini, TRUE);

		/* pcb = packet tx complete callback management */
		if (wlc_pcb_fn_register(ampdu_tx->wlc->pcb, wlc_send_bar_complete, ini, p)) {
			ini->retry_bar = AMPDU_R_BAR_CALLBACK_FAIL;
			return;
		}

		WLCNTINCR(ampdu_tx->cnt->txbar);
		AMPDUSCBCNTINCR(scb_ampdu->cnt.txbar);
		WLCNTINCR(ampdu_tx->wlc->pub->_cnt->txbar);
	}

	ini->bar_cnt++;
	ini->bar_ackpending = TRUE;
	ini->bar_ackpending_seq = seq;
	ini->barpending_seq = seq;

	if (ampdu_tx_cfg->no_bar == TRUE) {
		/* Cancel any pending wlc_send_bar_complete packet callbacks. */
		wlc_pcb_fn_find(ampdu_tx->wlc->pcb, wlc_send_bar_complete, ini, TRUE);
		wlc_send_bar_complete(ampdu_tx->wlc, TX_STATUS_ACK_RCV, ini);
	}
} /* wlc_ampdu_send_bar */

#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */

void
wlc_ampdu_ini_move_window_aqm(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini)
{
	ASSERT(ini);
	ASSERT(ampdu_tx);
	ASSERT(ampdu_tx->wlc);
	ASSERT(ampdu_tx->wlc->pub);

	if ((ini->acked_seq == ini->start_seq) ||
	    IS_SEQ_ADVANCED(ini->acked_seq, ini->start_seq)) {
		ini->start_seq = MODINC_POW2(ini->acked_seq, SEQNUM_MAX);
		ini->alive = TRUE;
	}
	/* if possible, release some buffered pdu's */
	wlc_ampdu_txeval(ampdu_tx, scb_ampdu, ini, FALSE);

	return;
}

#endif /* WLAMPDU_MAC */

/**
 * Called upon tx packet completion indication of d11 core. Not called for AC chips with AQM
 * enabled.
 */
static INLINE void
wlc_ampdu_ini_move_window_noaqm(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini)
{
	uint16 indx, i, range;

	ASSERT(ini);
	ASSERT(ampdu_tx);
	ASSERT(ampdu_tx->wlc);
	ASSERT(ampdu_tx->wlc->pub);

	/* ba_wsize can be zero in AMPDU_TID_STATE_BA_OFF or PENDING_ON state */
	if (ini->ba_wsize == 0)
		return;

	range = MODSUB_POW2(MODADD_POW2(ini->max_seq, 1, SEQNUM_MAX), ini->start_seq, SEQNUM_MAX);
	for (i = 0, indx = TX_SEQ_TO_INDEX(ini->start_seq);
		(i < range) && (!isset(ini->ackpending, indx)) &&
		(!isset(ini->barpending, indx));
		i++, indx = NEXT_TX_INDEX(indx));

	ini->start_seq = MODADD_POW2(ini->start_seq, i, SEQNUM_MAX);
	/* mark alive only if window moves forward */
	if (i > 0) {
		ini->rem_window += i;
		ini->alive = TRUE;
	}

	/* if possible, release some buffered pdus */
	wlc_ampdu_txeval(ampdu_tx, scb_ampdu, ini, FALSE);

	WL_AMPDU_TX(("wl%d: wlc_ampdu_ini_move_window: tid %d start_seq bumped to 0x%x\n",
		ampdu_tx->wlc->pub->unit, ini->tid, ini->start_seq));

	if (ini->rem_window > ini->ba_wsize) {
		WL_AMPDU_ERR(("wl%d: %s: rem_window %d, ba_wsize %d "
			"i %d range %d sseq 0x%x\n",
			ampdu_tx->wlc->pub->unit, __FUNCTION__, ini->rem_window, ini->ba_wsize,
			i, range, ini->start_seq));
		ASSERT(ini->rem_window <= ini->ba_wsize);
	}
} /* wlc_ampdu_ini_move_window_noaqm */

/**
 * Called upon tx packet completion indication of d11 core. Not called for AC chips with AQM
 * enabled. Bumps up the start seq to move the window.
 */
static INLINE void
wlc_ampdu_ini_move_window(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini)
{
#ifdef WLAMPDU_MAC
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub))
		wlc_ampdu_ini_move_window_aqm(ampdu_tx, scb_ampdu, ini);
	else
#endif /* WLAMPDU_MAC */
		wlc_ampdu_ini_move_window_noaqm(ampdu_tx, scb_ampdu, ini);
}

/**
 * Called when the D11 core indicates that transmission of a 'block ack request' packet completed.
 */
static void
wlc_send_bar_complete(wlc_info_t *wlc, uint txstatus, void *arg)
{
	scb_ampdu_tid_ini_t *ini = (scb_ampdu_tid_ini_t *)arg;
	ampdu_tx_info_t *ampdu_tx = wlc->ampdu_tx;
	uint16 indx, s_seq;
	bool aqm_no_bitmap = AMPDU_AQM_ENAB(ampdu_tx->wlc->pub);

	WL_AMPDU_CTL(("wl%d: wlc_send_bar_complete for tid %d status 0x%x\n",
		wlc->pub->unit, ini->tid, txstatus));

	/* There's a corner case that if pervious BAR sending failed,
	 *  this function might be invoked by PKTFREE during wl_down.
	 */
	if (!ampdu_tx->wlc->pub->up) {
		WL_INFORM(("%s:interface is not up,do nothing.", __FUNCTION__));
		return;
	}

	/* The BAR packet hasn't been attempted by the ucode tx function
	 * stop retrying...
	 */
	if (((txstatus & TX_STATUS_FRM_RTX_MASK) >> TX_STATUS_FRM_RTX_SHIFT) == 0) {
		WL_AMPDU_TX(("wl%d: %s: no tx attempted",
		             ampdu_tx->wlc->pub->unit, __FUNCTION__));
		return;
	}

	ASSERT(ini->bar_ackpending);
	ini->bar_ackpending = FALSE;

	/* ack received */
	if (txstatus & TX_STATUS_ACK_RCV) {
		if (!aqm_no_bitmap) {
			for (s_seq = ini->start_seq;
			     s_seq != ini->bar_ackpending_seq;
			     s_seq = NEXT_SEQ(s_seq)) {
				indx = TX_SEQ_TO_INDEX(s_seq);
				if (isset(ini->barpending, indx)) {
					clrbit(ini->barpending, indx);
					if (isset(ini->ackpending, indx)) {
						clrbit(ini->ackpending, indx);
						ini->txretry[indx] = 0;
					}
				}
			}
			/* bump up the start seq to move the window */
			wlc_ampdu_ini_move_window(ampdu_tx,
				SCB_AMPDU_TX_CUBBY(ampdu_tx, ini->scb), ini);
		} else {
			if (IS_SEQ_ADVANCED(ini->bar_ackpending_seq, ini->start_seq) &&
				((IS_SEQ_ADVANCED(ini->max_seq, ini->bar_ackpending_seq) ||
				MODINC_POW2(ini->max_seq, SEQNUM_MAX) == ini->bar_ackpending_seq)))
			{
				ini->acked_seq = ini->bar_ackpending_seq;
				wlc_ampdu_ini_move_window(ampdu_tx,
					SCB_AMPDU_TX_CUBBY(ampdu_tx, ini->scb), ini);
			}

			if (ini->barpending_seq == ini->bar_ackpending_seq)
				ini->barpending_seq = SEQNUM_MAX;
			ini->bar_cnt--;
		}
	}

	wlc_ampdu_txflowcontrol(wlc, SCB_AMPDU_TX_CUBBY(ampdu_tx, ini->scb), ini);
	if (aqm_no_bitmap) {
		/* not acked or need to send  bar for another seq no.  */
		if (ini->barpending_seq != SEQNUM_MAX) {
			wlc_ampdu_send_bar(wlc->ampdu_tx, ini, FALSE);
		} else {
			ini->bar_cnt = 0;
			ini->retry_bar = AMPDU_R_BAR_DISABLED;
		}
	} else {
		wlc_ampdu_send_bar(wlc->ampdu_tx, ini, FALSE);
	}
} /* wlc_send_bar_complete */

#ifdef WLAMPDU_MAC
/**
 * Higher layer invokes this function as a result of the d11 core indicating tx completion. Function
 * is only active for AC chips that have AQM enabled. 'Regular' tx MPDU is a non-AMPDU contained
 * MPDU.
 */
static void
wlc_ampdu_dotxstatus_regmpdu_aqm(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
	void *p, tx_status_t *txs, wlc_txh_info_t *txh_info)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	wlc_info_t *wlc = ampdu_tx->wlc;
	uint16 seq;

	BCM_REFERENCE(wlc);

#ifdef BCMDBG_SWWLAN_38455
	if ((WLPKTTAG(p)->flags & WLF_AMPDU_MPDU) == 0) {
		WL_PRINT(("%s flags=%x seq=%d\n", __FUNCTION__,
			WLPKTTAG(p)->flags, WLPKTTAG(p)->seq));
	}
#endif /* BCMDBG_SWWLAN_38455 */
	ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

	if (!scb || !SCB_AMPDU(scb))
		return;

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	ini = scb_ampdu->ini[PKTPRIO(p)];
	if (!ini || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
		WL_AMPDU_ERR(("wl%d: ampdu_dotxstatus_regmpdu: NULL ini or pon state for tid %d\n",
			wlc->pub->unit, PKTPRIO(p)));
		return;
	}
	ASSERT(ini->scb == scb);
		wlc_ampdu_dec_bytes_inflight(ini, p);

#ifdef WLATF
		/* Some of the devices may stop aggregating packets under noisy conditions
		 * This counter tracks the time a MPDU is sent from the AMPDU path
		 */
		WLCNTINCR(AMPDU_ATF_STATS(ini)->reg_mpdu);
#endif

	seq = txh_info->seq;

	if (txs->status.was_acked) {
		ini->acked_seq = seq;
	} else {
#ifdef WL11K
		uint8 tid = PKTPRIO(p);
		wlc_rrm_stat_qos_counter(scb, tid, OFFSETOF(rrm_stat_group_qos_t, ackfail));
#endif
		WLCNTINCR(ampdu_tx->cnt->txreg_noack);
		AMPDUSCBCNTINCR(scb_ampdu->cnt.txreg_noack);
		WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus_regmpdu: ack not recd for "
			"seq 0x%x tid %d status 0x%x\n",
			wlc->pub->unit, seq, ini->tid, txs->status.raw_bits));

		if ((AP_ENAB(wlc->pub) &&
		     (txs->status.suppr_ind == TX_STATUS_SUPR_PMQ)) ||
		     P2P_ABS_SUPR(wlc, txs->status.suppr_ind)) {
			/* N.B.: wlc_dotxstatus() will continue
			 * the rest of suppressed packet processing...
			 */
			return;
		}
	}
	return;
} /* wlc_ampdu_dotxstatus_regmpdu_aqm */

#endif /* WLAMPDU_MAC */

/**
 * Higher layer invokes this function as a result of the d11 core indicating tx completion. Function
 * is not active for AC chips that have AQM enabled. 'Regular' tx MPDU is a non-AMPDU contained
 * MPDU.
 */
static void
wlc_ampdu_dotxstatus_regmpdu_noaqm(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
	void *p, tx_status_t *txs)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint16 indx, seq;

	ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

	if (!scb || !SCB_AMPDU(scb))
		return;

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	ini = scb_ampdu->ini[PKTPRIO(p)];
	if (!ini || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
		WL_AMPDU_ERR(("wl%d: ampdu_dotxstatus_regmpdu: NULL ini or pon state for tid %d\n",
			ampdu_tx->wlc->pub->unit, PKTPRIO(p)));
		return;
	}
	ASSERT(ini->scb == scb);

	wlc_ampdu_dec_bytes_inflight(ini, p);
#ifdef WLATF
	/* Some of the devices like the 43226 and 43228 may stop
	 * aggregating packets under noisy conditions
	 * This counter tracks the time a MPDU is sent from the AMPDU path
	 */
	WLCNTINCR(AMPDU_ATF_STATS(ini)->reg_mpdu);
#endif

	seq = pkt_txh_seqnum(ampdu_tx->wlc, p);

	if (MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) >= ini->ba_wsize) {
		WL_AMPDU_ERR(("wl%d: %s: unexpected completion: seq 0x%x, "
			"start seq 0x%x\n",
			ampdu_tx->wlc->pub->unit, __FUNCTION__, seq, ini->start_seq));
		return;
	}

	indx = TX_SEQ_TO_INDEX(seq);
	if (!isset(ini->ackpending, indx)) {
		WL_AMPDU_ERR(("wl%d: %s: seq 0x%x\n",
			ampdu_tx->wlc->pub->unit, __FUNCTION__, seq));
		ampdu_dump_ini(ini);
		ASSERT(isset(ini->ackpending, indx));
		return;
	}

	if (txs->status.was_acked) {
		clrbit(ini->ackpending, indx);
		if (isset(ini->barpending, indx)) {
			clrbit(ini->barpending, indx);
		}
		ini->txretry[indx] = 0;

#ifdef PROP_TXSTATUS
		if (PROP_TXSTATUS_ENAB(ampdu_tx->wlc->pub)) {
			if (wlc_wlfc_suppr_status_query(ampdu_tx->wlc, scb)) {
				uint32 wlhinfo = WLPKTTAG(p)->wl_hdr_information;
				if ((WL_TXSTATUS_GET_FLAGS(wlhinfo) & WLFC_PKTFLAG_PKTFROMHOST) &&
					(WLPKTTAG(p)->flags & WLF_PROPTX_PROCESSED)) {
					wlfc_process_txstatus(ampdu_tx->wlc->wlfc,
						WLFC_CTL_PKTFLAG_SUPPRESS_ACKED, p, txs, FALSE);
				}
			}
		}
#endif /* PROP_TXSTATUS */

	} else {
#ifdef WL11K
		uint8 tid = PKTPRIO(p);
		wlc_rrm_stat_qos_counter(scb, tid, OFFSETOF(rrm_stat_group_qos_t, ackfail));
#endif
		WLCNTINCR(ampdu_tx->cnt->txreg_noack);
		AMPDUSCBCNTINCR(scb_ampdu->cnt.txreg_noack);
		WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus_regmpdu: ack not recd for "
			"seq 0x%x tid %d status 0x%x\n",
			ampdu_tx->wlc->pub->unit, seq, ini->tid, txs->status.raw_bits));

		/* suppressed pkts are resent; so dont move window */
		if ((AP_ENAB(ampdu_tx->wlc->pub) &&
		     (txs->status.suppr_ind == TX_STATUS_SUPR_PMQ)) ||
		     P2P_ABS_SUPR(ampdu_tx->wlc, txs->status.suppr_ind)) {
			/* N.B.: wlc_dotxstatus() will continue
			 * the rest of suppressed packet processing...
			 */
			return;
		}


	}
} /* wlc_ampdu_dotxstatus_regmpdu_noaqm */

/** function to process tx completion of a 'regular' mpdu. Called by higher layer. */
void
wlc_ampdu_dotxstatus_regmpdu(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
	void *p, tx_status_t *txs, wlc_txh_info_t *txh_info)
{
#ifdef WLAMPDU_MAC
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub))
		wlc_ampdu_dotxstatus_regmpdu_aqm(ampdu_tx, scb, p, txs, txh_info);
	else
#endif /*  WLAMPDU_MAC */
		wlc_ampdu_dotxstatus_regmpdu_noaqm(ampdu_tx, scb, p, txs);
}

/**
 * Upon the d11 core indicating transmission of a packet, AMPDU packet(s) can be freed.
 */
static void
wlc_ampdu_free_chain(ampdu_tx_info_t *ampdu_tx, void *p, tx_status_t *txs, bool txs3_done)
{
	wlc_info_t *wlc = ampdu_tx->wlc;
#if defined(TXQ_MUX)
	int tx_time = 0;
#endif /* TXQ_MUX */
	wlc_txh_info_t txh_info;
	uint16 mcl;
	uint8 queue;
	uint16 count = 0;
	struct scb *scb = NULL;

#if defined(WLAMPDU_MAC)
	uint16 ncons = 0; /**< AQM. Last d11 seq that was processed by d11 core. */

	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub))
		ncons = wlc_ampdu_rawbits_to_ncons(txs->status.raw_bits);
	else
		ncons = txs->sequence;
#endif /* WLAMPDU_MAC */

	if (p != NULL) {
		scb = WLPKTTAGSCBGET(p);
	}

	WL_AMPDU_TX(("wl%d: wlc_ampdu_free_chain: free ampdu_tx chain\n", wlc->pub->unit));

#if !defined(WLAMPDU_MAC) && !defined(TXQ_MUX)
	/* packet 'count' is only directly used for WLAMPDU_MAC paths, not SW AMPDU.
	 * SW AMPDU uses txpkt_weight as the txpktpend adjustment for an AMDPU.
	 */
	BCM_REFERENCE(count);
#endif

	queue = WLC_TXFID_GET_QUEUE(txs->frameid);

	/* loop through all packets in the dma ring and free */
	while (p) {
#ifdef BCMDBG_SWWLAN_38455
		if ((WLPKTTAG(p)->flags & WLF_AMPDU_MPDU) == 0) {
			WL_PRINT(("%s flags=%x seq=%d queue=%d\n", __FUNCTION__,
				WLPKTTAG(p)->flags, WLPKTTAG(p)->seq, queue));
		}
#endif /* BCMDBG_SWWLAN_38455 */
		ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

		count++;
#if defined(TXQ_MUX)
		tx_time += WLPKTTIME(p);
#endif /* TXQ_MUX */

		wlc_get_txh_info(wlc, p, &txh_info);
		mcl = ltoh16(txh_info.MacTxControlLow);

		/* Update pkttag's scb from the first packet */
		WLPKTTAGSCBSET(p, scb);

		wlc_tx_status_update_counters(wlc, p, scb, NULL,
			WLC_TX_STS_COMP_NO_SCB, 1);

		WLCNTINCR(ampdu_tx->cnt->orphan);
		PKTFREE(wlc->osh, p, TRUE);

		if (AMPDU_HOST_ENAB(wlc->pub)) {
			/* if not the last ampdu, take out the next packet from dma chain */
			if (((mcl & TXC_AMPDU_MASK) >> TXC_AMPDU_SHIFT) == TXC_AMPDU_LAST)
				break;
		} else {
#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */
			if (count == ncons) {
				break;
			}
#endif /* WLAMPDU_MAC */
		}

		p = GETNEXTTXP(wlc, queue);
	}

#if defined(TXQ_MUX)
	/* Host mode ampdu_tx->config->txpkt_weight not used in time based queuing */
	WLC_TXFIFO_COMPLETE(wlc, queue, count, tx_time);
#else
	if (AMPDU_ENAB(wlc->pub) && AMPDU_HOST_ENAB(wlc->pub)) {
		WLC_TXFIFO_COMPLETE(wlc, queue, ampdu_tx->config->txpkt_weight, tx_time);
	} else {
		WLC_TXFIFO_COMPLETE(wlc, queue, count, tx_time);
	}
#endif /* TXQ_MUX */

	WL_AMPDU_TX(("%s: fifo %d pkts %d txpktpend %d\n",
		__FUNCTION__, queue, count, TXPKTPENDGET(wlc, queue)));

	/* for corerev >= 40, all txstatus have been read already */
	if (D11REV_GE(wlc->pub->corerev, 40))
		return;

	/* retrieve the next status if it is there */
	if (txs->status.was_acked) {
		wlc_hw_info_t *wlc_hw = wlc->hw;
		wlc_txs_pkg8_t pkg;
		int txserr;
		uint8 status_delay = 0;

		ASSERT(txs->status.is_intermediate);

		/* wait till the next 8 bytes of txstatus is available */
		while ((txserr = wlc_bmac_read_txs_pkg8(wlc_hw, &pkg)) == BCME_NOTREADY) {
			OSL_DELAY(1);
			status_delay++;
			if (status_delay > 10) {
				ASSERT(0);
				return;
			}
		}

		ASSERT(!(pkg.word[0] & TX_STATUS_INTERMEDIATE));
		ASSERT(pkg.word[0] & TX_STATUS_AMPDU);

		/* clear the txs pkg2 read required flag */
		if ((txs->procflags & TXS_PROCFLAG_AMPDU_BA_PKG2_READ_REQD) && txserr == BCME_OK) {
			txs->procflags &= ~TXS_PROCFLAG_AMPDU_BA_PKG2_READ_REQD;
		}

#ifdef WLAMPDU_HW
		if (!txs3_done && AMPDU_HW_ENAB(wlc->pub)) {
			/* yet another txs to retrieve */
			status_delay = 0;
			while ((txserr = wlc_bmac_read_txs_pkg8(wlc_hw, &pkg)) == BCME_NOTREADY) {
				OSL_DELAY(1);
				status_delay++;
				if (status_delay > 10) {
					ASSERT(0);
					return;
				}
			}
		}
#endif /* WLAMPDU_HW */
	}
} /* wlc_ampdu_free_chain */

/**
 * Called by higher layer when d11 core indicates transmit completion of an MPDU
 * that has its 'AMPDU' packet flag set.
 */
bool BCMFASTPATH
wlc_ampdu_dotxstatus(ampdu_tx_info_t *ampdu_tx, struct scb *scb, void *p, tx_status_t *txs,
	wlc_txh_info_t *txh_info)
{
	scb_ampdu_tx_t *scb_ampdu;
	wlc_info_t *wlc = ampdu_tx->wlc;
	wlc_txs_pkg8_t pkg;
	wlc_txs_pkg8_t pkg2;
	scb_ampdu_tid_ini_t *ini;
	/* For low driver, we don't want this function to return TRUE for certain err conditions
	 * returning true will cause a wlc_fatal_error() to get called.
	 * we want to reserve wlc_fatal_error() call for hw related err conditions.
	 */
	bool need_reset = FALSE;
	BCM_REFERENCE(wlc);
	ASSERT(p);
	ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

#ifdef WLAMPDU_MAC
	/* update aggfifo/side_channel upfront in case we bail out early */
	if (!AMPDU_AQM_ENAB(wlc->pub) && AMPDU_MAC_ENAB(wlc->pub)) {
		ampdumac_info_t *hagg;
		uint16 max_entries = 0;
		uint16 queue;
		uint16 ncons; /**< number of consumed packets */

		queue = WLC_TXFID_GET_QUEUE(txs->frameid);
		ASSERT(queue < AC_COUNT);
		hagg = &(ampdu_tx->hagg[queue]);
		ncons = txs->sequence;
#ifdef WLAMPDU_HW
		if (AMPDU_HW_ENAB(wlc->pub)) {
			max_entries = ampdu_tx->config->aggfifo_depth + 1;
		} else
#endif
		{
#ifdef WLAMPDU_UCODE
		max_entries = hagg->txfs_addr_end - hagg->txfs_addr_strt + 1;
		hagg->txfs_rptr += ncons;
		if (hagg->txfs_rptr > hagg->txfs_addr_end)
			hagg->txfs_rptr -= max_entries;
#endif
		}
		ASSERT(ncons > 0 && ncons < max_entries && ncons <= hagg->in_queue);
		BCM_REFERENCE(max_entries);
		hagg->in_queue -= ncons;
	}
#endif /* WLAMPDU_MAC */

	if (!scb || !SCB_AMPDU(scb)) {
		/* null scb may happen if pkts were in txq when scb removed */
		if (!scb) {
			WL_AMPDU(("wl%d: %s: scb is null\n",
				wlc->pub->unit, __FUNCTION__));
			/* pkttag must have been set null in wlc_dotxstatus */
			ASSERT(WLPKTTAGSCBGET(p) == NULL);
		} else {
			WLPKTTAGSCBSET(p, NULL);
		}
		wlc_ampdu_free_chain(ampdu_tx, p, txs, FALSE);
		return need_reset;
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	ini = scb_ampdu->ini[(uint8)PKTPRIO(p)];
	if (!ini || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
		wlc_ampdu_free_chain(ampdu_tx, p, txs, FALSE);
		WL_AMPDU_ERR(("wl%d: %s: bail: bad ini (%p) or ba_state (%d) prio %d\n",
			wlc->pub->unit, __FUNCTION__, OSL_OBFUSCATE_BUF(ini),
			(ini ? ini->ba_state : -1), (int)PKTPRIO(p)));
		return need_reset;
	}
	ASSERT(ini->scb == scb);
	ASSERT(WLPKTTAGSCBGET(p) == scb);

#ifdef WLAMPDU_MAC
	if (D11REV_GE(wlc->pub->corerev, 40)) {
		wlc_ampdu_dotxstatus_aqm_complete(ampdu_tx, scb, p, txs, txh_info);
		/* agg may be half done here */
		wlc_ampdu_agg_complete(wlc, ampdu_tx, scb_ampdu, ini, FALSE);
#ifdef BCMDBG
		WLC_AMPDU_TXQ_PROF_ADD_ENTRY(ampdu_tx->wlc, scb_ampdu->scb);
#endif
		return FALSE;
	}
#endif /* WLAMPDU_MAC */

	bzero(&pkg, sizeof(pkg));
	bzero(&pkg2, sizeof(pkg2));

	/* BMAC_NOTE: For the split driver, second level txstatus comes later
	 * So if the ACK was received then wait for the second level else just
	 * call the first one
	 */
	if (txs->status.was_acked) {
		wlc_hw_info_t *wlc_hw = wlc->hw;
		int txserr;
		uint8 status_delay = 0;

		scb->used = wlc->pub->now;
		/* wait till the next 8 bytes of txstatus is available */
		while ((txserr = wlc_bmac_read_txs_pkg8(wlc_hw, &pkg)) == BCME_NOTREADY) {
			OSL_DELAY(1);
			status_delay++;
			if (status_delay > 10) {
				WL_PRINT(("wl%d func %s line %d status_delay %u frameId %u",
					wlc->pub->unit, __FUNCTION__, __LINE__,
					status_delay, txs->frameid));
				ASSERT(0);
				wlc_ampdu_free_chain(ampdu_tx, p, txs, FALSE);
				return TRUE;
			}
		}
		/* clear the txs pkg2 read required flag */
		if ((txs->procflags & TXS_PROCFLAG_AMPDU_BA_PKG2_READ_REQD) && txserr == BCME_OK) {
			txs->procflags &= ~TXS_PROCFLAG_AMPDU_BA_PKG2_READ_REQD;
		}

		WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: 2nd status in delay %d\n",
			wlc->pub->unit, status_delay));

		ASSERT(!(pkg.word[0] & TX_STATUS_INTERMEDIATE));
		ASSERT(pkg.word[0] & TX_STATUS_AMPDU);

#ifdef WLAMPDU_HW
		if (AMPDU_HW_ENAB(wlc->pub)) {
			/* wait till the next 8 bytes of txstatus is available */
			status_delay = 0;
			while ((txserr = wlc_bmac_read_txs_pkg8(wlc_hw, &pkg2)) == BCME_NOTREADY) {
				OSL_DELAY(1);
				status_delay++;
				if (status_delay > 10) {
					WL_PRINT(("wl%d func %s line %d status_delay %u frameId %u",
						wlc->pub->unit, __FUNCTION__, __LINE__,
						status_delay, txs->frameid));
					ASSERT(0);
					wlc_ampdu_free_chain(ampdu_tx, p, txs, TRUE);
					return TRUE;
				}
			}
		}
#endif /* WLAMPDU_HW */
	}

	wlc_ampdu_dotxstatus_complete(ampdu_tx, scb, p, txs,
	                              pkg.word[0], pkg.word[1],
	                              pkg2.word[0], pkg2.word[1]);
	wlc_ampdu_txflowcontrol(wlc, scb_ampdu, ini);

	return FALSE;
} /* wlc_ampdu_dotxstatus */

#ifdef WLMEDIA_TXFAILEVENT

/**
 * To indicate to the host a TX fail event and provide information such as destination mac address
 * and a portion of the packet.
 */
static void BCMFASTPATH
wlc_tx_failed_event(wlc_info_t *wlc, struct scb *scb, wlc_bsscfg_t *bsscfg, tx_status_t *txs,
	void *p, uint32 flags)
{
	txfailinfo_t txinfo;
	d11txh_t *txh = NULL;
	struct dot11_header *h;
	uint16 fc;
	char *p1;

	/* Just in case we need events only for certain clients or
	 * additional information is needed in the future
	 */
	UNUSED_PARAMETER(scb);
	UNUSED_PARAMETER(flags);

	memset(&txinfo, 0, sizeof(txfailinfo_t));
	wlc_read_tsf(wlc, &txinfo.tsf_l, &txinfo.tsf_h);

	if (txs)
		txinfo.txstatus = txs->status;

	if (p) {
		txinfo.prio  = PKTPRIO(p);
		txh = (d11txh_t *)PKTDATA(wlc->osh, p);
		if (txh)
			txinfo.rates = ltoh16(txh->MainRates);
		p1 = (char*) PKTDATA(wlc->osh, p)
			 + sizeof(d11txh_t) + D11_PHY_HDR_LEN;
		h = (struct dot11_header*)(p1);
		fc = ltoh16(h->fc);
		if ((fc & FC_TODS))
			memcpy(&txinfo.dest, h->a3.octet, ETHER_ADDR_LEN);
		else if ((fc & FC_FROMDS))
			memcpy(&txinfo.dest, h->a1.octet, ETHER_ADDR_LEN);
	}

	wlc_bss_mac_event(wlc, bsscfg, WLC_E_TXFAIL, (struct ether_addr *)txh->TxFrameRA,
		WLC_E_STATUS_TIMEOUT, 0, 0, &txinfo, sizeof(txinfo));


} /* wlc_tx_failed_event */

#endif /* WLMEDIA_TXFAILEVENT */

#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */
#ifdef WL_CS_PKTRETRY
static INLINE void
wlc_ampdu_cs_pktretry(struct scb *scb, void *p, uint8 tid)
{
	wlc_pkttag_t *pkttag = WLPKTTAG(p);

	pkttag->flags |= WLF_FIFOPKT; /* retrieved from HW FIFO */
#if !defined(TXQ_MUX)
	/* Not needed for TXQ_MUX */
	pkttag->flags3 |= WLF3_SUPR; /* suppressed */
#endif /* TXQ_MUX */
	WLPKTTAGSCBSET(p, scb);

	SCB_TX_NEXT(TXMOD_AMPDU, scb, p, WLC_PRIO_TO_HI_PREC(tid));
}
#endif /* WL_CS_PKTRETRY */

#if defined(WL_MU_TX)
#ifdef WLCNT
void BCMFASTPATH
wlc_ampdu_aqm_mutx_dotxinterm_status(ampdu_tx_info_t *ampdu_tx, tx_status_t *txs)
{
	bool txs_mu;
	wlc_info_t *wlc = ampdu_tx->wlc;
	void *p;
	struct scb *scb = NULL;
	wlc_bsscfg_t *bsscfg = NULL;
	wlc_txh_info_t txh_info;
	int err;
	uint16 qid = WLC_TXFID_GET_QUEUE(txs->frameid);
	BCM_REFERENCE(err);

	txs_mu = (txs->status.s5 & TX_STATUS64_MUTX) ? TRUE : FALSE;

	if (!txs_mu)
		return;

	p = wlc_bmac_dmatx_peeknexttxp(wlc, qid);
	if (p == NULL) {
		return;
	}

	if (!(WLPKTTAG(p)->flags & WLF_TXHDR)) {
		return;
	}

	wlc_get_txh_info(wlc, p, &txh_info);
	if (txs->frameid != htol16(txh_info.TxFrameID)) {
		WL_ERROR(("wl%d: %s: txs->frameid 0x%x txh->TxFrameID 0x%x\n",
			wlc->pub->unit, __FUNCTION__,
			txs->frameid, htol16(txh_info.TxFrameID)));
		return;
	}

	if (!ETHER_ISMULTI(txh_info.TxFrameRA)) {
		/* use the bsscfg index in the packet tag to find out which
		 * bsscfg this packet belongs to
		 */
		bsscfg = wlc_bsscfg_find(wlc, WLPKTTAGBSSCFGGET(p), &err);
		/* For ucast frames, lookup the scb directly by the RA.
		 * The scb may not be found if we sent a frame with no scb, or if the
		 * scb was deleted while the frame was queued.
		 */
		if (bsscfg != NULL) {
			scb = wlc_scbfindband(wlc, bsscfg,
				(struct ether_addr*)txh_info.TxFrameRA,
				(wlc_txh_info_is5GHz(wlc, &txh_info)
				?
				BAND_5G_INDEX : BAND_2G_INDEX));

			if (scb == NULL)
				return;
			wlc_mutx_upd_interm_counters(wlc->mutx, scb, txs);
		}
	}
}
#endif /* WLCNT */
#endif /* defined(WL_MU_TX) */



static void BCMFASTPATH
wlc_ampdu_dotxstatus_aqm_complete(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
                                  void *p, tx_status_t *txs, wlc_txh_info_t *txh_info)
{
	scb_ampdu_tx_t *scb_ampdu;
	wlc_info_t *wlc = ampdu_tx->wlc;
	scb_ampdu_tid_ini_t *ini;
	wlc_bsscfg_t *bsscfg;

	uint8 queue; /**< d11 dma/fifo queue from where subsequent pkts are drawn */
	uint8 tid, tot_mpdu = 0;

	/* per txstatus info */
	uint ncons; /**< #consumed mpdus, as reported by d11 core through txs */
	uint supr_status;
	bool update_ratesel;
	bool retry_suppr_pkts = FALSE;
	bool fix_rate = FALSE;
	uint16 tx_flags = 0;

	/* per packet info */
	bool was_acked = FALSE, send_bar = FALSE;
	uint16 seq = 0, bar_seq = 0;

	/* for first packet */
	int rnum = 0;
	ratesel_txs_t rs_txs;

#if defined(TXQ_MUX)
	int tx_time = 0;
#endif /* TXQ_MUX */

	bool txs_mu = FALSE;	/* If txed as MU */
#if defined(WL_MU_TX) && defined(WLCNT)
	bool is_mu = FALSE;	/* If queued as MU */
	uint8 gid = VHT_SIGA1_GID_NOT_TO_AP;
#endif /* WL_MU_TX && WLCNT */

#ifdef PKTQ_LOG
	uint16 mcs_txrate[RATESEL_MFBR_NUM];
#endif
#if defined(SCB_BS_DATA)
	wlc_bs_data_counters_t *bs_data_counters = NULL;
#endif

	uint16 start_seq = 0xFFFF; /* invalid */

#if defined(WLPKTDLYSTAT) || defined(WL11K)
	uint32 delay, now;
#endif
#ifdef WLPKTDLYSTAT
	uint tr;
	uint8 ac;
	scb_delay_stats_t *delay_stats;
#endif

	int k;
	uint16 nlost = 0;
#ifdef PKTQ_LOG
	pktq_counters_t *prec_cnt = NULL;
	pktq_counters_t *actq_cnt = NULL;
	uint32           prec_pktlen = 0;
#endif
#ifdef PSPRETEND
	bool pps_retry = FALSE;
	bool pps_recvd_ack = FALSE;
	uint16	macCtlHigh = 0;
#endif /* PSPRETEND */
#if defined(PKTQ_LOG) || defined(PSPRETEND)
	d11actxh_t* vhtHdr = NULL;
#endif /* PKTQ_LOG || PSPRETEND */
	wlc_pkttag_t* pkttag;
	bool from_host = TRUE;
#ifdef WL_CS_PKTRETRY
	bool cs_retry = wlc_ampdu_cs_retry(wlc, txs, p);
#endif
#ifdef WL_TX_STALL
	wlc_tx_status_t tx_status = WLC_TX_STS_SUCCESS;
#endif
#if defined(BCMDBG) || defined(BCMDBG_AMPDU) || defined(WL_LINKSTAT)
#ifdef WL11AC
	uint8 vht[RATESEL_MFBR_NUM];
#endif /* WL11AC */
	bool is_vht[RATESEL_MFBR_NUM];
	bool is_stbc[RATESEL_MFBR_NUM];

	bzero(is_vht, sizeof(is_vht));
	bzero(is_stbc, sizeof(is_stbc));
#endif 
	bzero(&rs_txs, sizeof(rs_txs));

	BCM_REFERENCE(start_seq);
	BCM_REFERENCE(bsscfg);
	BCM_REFERENCE(from_host);

	ncons = wlc_ampdu_rawbits_to_ncons(txs->status.raw_bits);
	rs_txs.ncons = (ncons > 0xff) ? 0xff : (uint8)ncons;

	if (!ncons) {
		WL_AMPDU_ERR(("ncons is %d in %s", ncons, __FUNCTION__));
		wlc_tx_status_update_counters(wlc, p,
			scb, NULL, WLC_TX_STS_BAD_NCONS, 1);
		PKTFREE(wlc->osh, p, TRUE);
		ASSERT(ncons);
		return;
	}

	WLCNTADD(ampdu_tx->cnt->cons, ncons);

	if (!scb || !SCB_AMPDU(scb)) {
		if (!scb) {
			WL_AMPDU(("wl%d: %s: scb is null\n",
				wlc->pub->unit, __FUNCTION__));
			/* pkttag must have been set null in wlc_dotxstatus */
			ASSERT(WLPKTTAGSCBGET(p) == NULL);
		} else {
			WLPKTTAGSCBSET(p, NULL);
		}
		wlc_ampdu_free_chain(ampdu_tx, p, txs, FALSE);
		return;
	}

	SCB_BS_DATA_CONDFIND(bs_data_counters, wlc, scb);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	bsscfg = SCB_BSSCFG(scb);
	ASSERT(scb_ampdu);
	ASSERT(p);
	tid = (uint8)PKTPRIO(p);

	queue = WLC_TXFID_GET_QUEUE(txs->frameid);
	ASSERT(queue < NFIFO_EXT);

#ifdef WLPKTDLYSTAT
	ac = WME_PRIO2AC(tid);
	delay_stats = scb->delay_stats + ac;
#endif

	/*
	2 	RTS tx count
	3 	CTS rx count
	11:4 	Acked bitmap for each of consumed mpdus reported in this txstatus, sequentially.
	That is, bit 0 is for the first consumed mpdu, bit 1 for the second consumed frame.
	*/
	ini = scb_ampdu->ini[tid];

	if (!ini) {
		WL_AMPDU_ERR(("%s: bad ini error\n", __FUNCTION__));
		wlc_ampdu_free_chain(ampdu_tx, p, txs, FALSE);
		return;
	}

	update_ratesel = (WLPKTTAG(p)->flags & WLF_RATE_AUTO) ? TRUE : FALSE;
	supr_status = txs->status.suppr_ind; /* status can be for multiple MPDUs */

	if (supr_status != TX_STATUS_SUPR_NONE) {
		update_ratesel = FALSE;
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMPDU)
		ampdu_tx->amdbg->supr_reason[supr_status] += ncons;
#endif
		WL_AMPDU_TX(("wl%d: %s: supr_status 0x%x\n",
			wlc->pub->unit, __FUNCTION__, supr_status));
		if (supr_status == TX_STATUS_SUPR_BADCH) {
			WLCNTADD(wlc->pub->_cnt->txchanrej, ncons);
		} else if (supr_status == TX_STATUS_SUPR_EXPTIME) {
			WLCNTADD(wlc->pub->_cnt->txexptime, ncons);
#ifdef WL11K
			wlc_rrm_tscm_upd(scb, tid,
				OFFSETOF(rrm_tscm_t, msdu_exp), ncons);
#endif
			/* Interference detected */
			if (wlc->rfaware_lifetime)
				wlc_exptime_start(wlc);
		} else if (supr_status == TX_STATUS_SUPR_FRAG) {
			WL_AMPDU_ERR(("wl%d: %s: tx underflow?!\n",
					wlc->pub->unit, __FUNCTION__));
		} else if (wlc_should_retry_suppressed_pkt(wlc, p, supr_status)) {
			/* N.B.: we'll transmit the packet when coming out of the
			 * absence period....use the retry logic blow to reenqueue
			 * the packet.
			 */
			retry_suppr_pkts = TRUE;
		}
		else if (supr_status == TX_STATUS_INT_XFER_ERR) {
			retry_suppr_pkts = FALSE;
		}
	} else if (txs->phyerr) {
		update_ratesel = FALSE;
		WLCNTADD(wlc->pub->_cnt->txphyerr, ncons);
		WL_ERROR(("wl%d: %s: tx phy error (0x%x)\n",
			wlc->pub->unit, __FUNCTION__, txs->phyerr));
	}

#if defined(BCMDBG) && defined(PSPRETEND)
	/* we are draining the fifo yet we received a tx status which isn't suppress - this
	 * is an error we should trap if we are still in the same ps pretend instance
	 */
	if ((BSSCFG_AP(bsscfg) || BSSCFG_IBSS(bsscfg)) &&
	    PS_PRETEND_ENABLED(bsscfg)) {
		wlc_pspretend_supr_upd(wlc->pps_info, bsscfg, scb, supr_status);
	}
#endif /* BCMDBG && PSPRETEND */

	/* Check if interference is still there */
	if (wlc->rfaware_lifetime && wlc->exptime_cnt && (supr_status != TX_STATUS_SUPR_EXPTIME))
		wlc_exptime_check_end(wlc);

#if defined(WLPKTDLYSTAT) || defined(WL11K)
	/* Get the current time */
	now = WLC_GET_CURR_TIME(wlc);
#endif

#ifdef PROP_TXSTATUS
	from_host = WL_TXSTATUS_GET_FLAGS(WLPKTTAG(p)->wl_hdr_information) &
		WLFC_PKTFLAG_PKTFROMHOST;
#endif	/* PROP_TXSTATUS */

	/* ncons is non-zero, so can enter unconditionally. exit via break in loop
	 * body.
	 */
	while (TRUE) { /* loops over each tx MPDU in the caller supplied tx sts */
		bool free_mpdu = TRUE;

		pkttag = WLPKTTAG(p);

#ifdef BCMDBG_SWWLAN_38455
		if ((WLPKTTAG(p)->flags & WLF_AMPDU_MPDU) == 0) {
			WL_PRINT(("%s flags=%x seq=%d supr_status=%d queue=%d\n", __FUNCTION__,
				WLPKTTAG(p)->flags, WLPKTTAG(p)->seq, supr_status, queue));
		}
#endif /* BCMDBG_SWWLAN_38455 */

		ASSERT(pkttag->flags & WLF_AMPDU_MPDU);

		seq = pkttag->seq;
#ifdef PROP_TXSTATUS
		seq = WL_SEQ_GET_NUM(seq);
#endif /* PROP_TXSTATUS */

#if defined(TXQ_MUX)
		tx_time += WLPKTTIME(p);
#endif /* TXQ_MUX */

#ifdef PKTQ_LOG
		if (scb_ampdu->txq.pktqlog) {
			prec_cnt = wlc_ampdu_pktqlog_cnt(&scb_ampdu->txq, tid,
				WLAMPDU_PREC_TID2PREC(p, tid));
		}

		if ((WLC_GET_TXQ(wlc->active_queue))->pktqlog) {
			actq_cnt =
				(WLC_GET_TXQ(wlc->active_queue))->
					pktqlog->_prec_cnt[WLC_PRIO_TO_PREC(tid)];
		}

		if (supr_status != TX_STATUS_SUPR_NONE) {
			WLCNTCONDINCR(prec_cnt, prec_cnt->suppress);
			WLCNTCONDINCR(actq_cnt, actq_cnt->suppress);
		}
#endif /* PKTQ_LOG */

		if (tot_mpdu == 0) { /* first MPDU in a list of MPDUs */
			bool last_rate, is_sgi = FALSE;
			uint8 bw;
			uint16 ft;
			d11actxh_rate_t *rinfo;

#ifdef PKTQ_LOG
			prec_pktlen = txh_info->d11FrameSize - DOT11_LLC_SNAP_HDR_LEN -
			              DOT11_MAC_HDR_LEN - DOT11_QOS_LEN;
#endif

#ifdef BCMDBG
			if ((supr_status != TX_STATUS_INT_XFER_ERR) &&
			    (txs->frameid != htol16(txh_info->TxFrameID))) {
				WL_ERROR(("wl%d: %s: txs->frameid 0x%x txh->TxFrameID 0x%x p %p\n",
					wlc->pub->unit, __FUNCTION__,
					txs->frameid, htol16(txh_info->TxFrameID), p));
				ASSERT(txs->frameid == htol16(txh_info->TxFrameID));
			}
#endif

			start_seq = seq;
			fix_rate = ltoh16(txh_info->MacTxControlHigh) & D11AC_TXC_FIX_RATE;
			rinfo = (d11actxh_rate_t *)
				(txh_info->plcpPtr - OFFSETOF(d11actxh_rate_t, plcp[0]));
			rnum = 0;

			/* this loop collects information for debug/tallies purposes. It
			 * loops over rates and is executed only for the first MPDU.
			 */
			do {
				ft = ltoh16(rinfo->PhyTxControlWord_0) & PHY_TXC_FT_MASK;
				/* note: rs_txs.txrspec[rnum] can be MCS32 */
				rs_txs.txrspec[rnum] = ltoh16(rinfo->PhyTxControlWord_2) &
					D11AC_PHY_TXC_PHY_RATE_MASK;

#if defined(WLPROPRIETARY_11N_RATES)
				if (ltoh16(rinfo->PhyTxControlWord_1) & D11AC_PHY_TXC_11N_PROP_MCS)
					rs_txs.txrspec[rnum] |= (1 << HT_MCS_BIT6_SHIFT);
#endif /* WLPROPRIETARY_11N_RATES */

#ifdef PKTQ_LOG
				mcs_txrate[rnum] = ltoh16(rinfo->TxRate);
#endif
				is_sgi = FALSE;
				if (ft == FT_VHT) {
					/* VHT rate */
					uint8 nss, modu;
					modu = rs_txs.txrspec[rnum] & 0xf;
					nss = (rs_txs.txrspec[rnum] >> 4) & 0xf;
					is_sgi = VHT_PLCP3_ISSGI(rinfo->plcp[3]);
					bw = VHT_PLCP0_BW(rinfo->plcp[0]);
#if (defined(BCMDBG) || defined(BCMDBG_AMPDU) || defined(WL_LINKSTAT))
#ifdef WL11AC
					vht[rnum] = nss * MAX_VHT_RATES + modu;
#endif /* WL11AC */
					is_vht[rnum] = TRUE;
					is_stbc[rnum] =
						(rinfo->plcp[0] & VHT_SIGA1_STBC) ? TRUE : FALSE;
#endif 
					/* 0x80 is for vht identification */
					rs_txs.txrspec[rnum] = 0x80 | ((nss + 1) << 4) | modu;
				} else {
#if (defined(BCMDBG) || defined(BCMDBG_AMPDU) || defined(WL_LINKSTAT))
					is_stbc[rnum] = PLCP3_ISSTBC(rinfo->plcp[3]);
#endif
					is_sgi = PLCP3_ISSGI(rinfo->plcp[3]);
					bw = (rinfo->plcp[0] & MIMO_PLCP_40MHZ) ? 1 : 0;
				}
				rs_txs.txrspec[rnum] |= is_sgi ? RSPEC_SHORT_GI : 0;
				/* plcp bw to rspec bw (plcp bw + 1 = rspec bw) */
				rs_txs.txrspec[rnum] |= ((bw + BW_20MHZ) << RSPEC_BW_SHIFT);

				last_rate = (ltoh16(rinfo->RtsCtsControl) &
				             D11AC_RTSCTS_LAST_RATE) ?
					TRUE : FALSE;

				rnum ++;
				rinfo++;
			} while (!last_rate && rnum < RATESEL_MFBR_NUM); /* loop over rates */
#if defined(WL_MU_TX)
			txs_mu = (txs->status.s5 & TX_STATUS64_MUTX) ? TRUE : FALSE;
#if defined(WLCNT)
			is_mu = ((ltoh16(txh_info->MacTxControlHigh) & D11AC_TXC_MU) != 0);
#endif /* WLCNT */
#endif /* WL_MU_TX */
		} /* tot_mpdu == 0 */

		was_acked = wlc_txs_was_acked(wlc, &(txs->status), tot_mpdu);

#ifdef WL_CS_PKTRETRY
		if (cs_retry) { /* PSPRETEND related functionality */
			/* Retry packet */
			wlc_ampdu_cs_pktretry(scb, p, tid);

			/* Mark that packet retried */
			free_mpdu = FALSE;
			was_acked = FALSE;

			WLCNTINCR(ampdu_tx->cnt->cs_pktretry_cnt);
		}
#endif /* WL_CS_PKTRETRY */

		if (!was_acked && supr_status == TX_STATUS_SUPR_NONE) {
			nlost ++;
#ifdef WLC_SW_DIVERSITY
			if (WLSWDIV_ENAB(wlc) && WLSWDIV_BSSCFG_SUPPORT(bsscfg)) {
				wlc_swdiv_txfail(wlc->swdiv, scb);
			}
#endif /* WLC_SW_DIVERSITY */
		}

#ifdef PSPRETEND
		/* quite complex logic to decide to save the packet with ps pretend
		* a. pre-logic we test if tx status has TX_STATUS_SUPR_PPS - if so, clear
		*    the ack status regardless
		*
		* Now, test to save the packet with ps pretend (pps_retry flag).....
		* 0. if packet retried make it invisible for ps pretend
		* 1. the packet has to actually not have been sent ok (not acked)
		* 2. we are not in ordinary PS mode
		* 3. suppressed for reasons other ps pretend, forget about saving it unless
		*    we are already in PS pretend active state, so we might as well save it
		* 4. ps pretend has got to be globally enabled & the scb has not to be excluded
		*    from ps pretend
		* 5. the packet has to have the D11AC_TXC_PPS bit set if we doing normal ps
		*    pretend. This is regardless of whether TX_STATUS_SUPR_PPS is true; if
		*    not suppressed it means this is the first tx status with the initial
		*    transmission failure.
		* 6. or we have reached the threshold in case of threshold mode
		* ...... then we save the packet with ps pretend, and mark it for retry
		*
		* This 'if..else if' construct is designed to test the most common cases
		* first and then break out of the logic at the earliest case. Even in the
		* case we set pps_retry to FALSE, we are needing that case to prevent
		* moving through the rest of the logic.
		*
		*/

		if (PSPRETEND_ENAB(wlc->pub) && supr_status == TX_STATUS_SUPR_PPS) {
			/* a */
			was_acked = FALSE;
		}

		if ((BSSCFG_AP(bsscfg) || BSSCFG_IBSS(bsscfg)) &&
		    PS_PRETEND_ENABLED(bsscfg)) {
#ifdef WL_CS_PKTRETRY
			if (cs_retry) {
				/* 0 */
			} else
#endif
			if (was_acked) {
				/* 1 */
				pps_retry = FALSE;
				pps_recvd_ack = TRUE;
			} else if (SCB_PS_PRETEND_NORMALPS(scb)) {
				/* 2 */
				pps_retry = FALSE;
			} else if (supr_status && (supr_status != TX_STATUS_SUPR_PPS) &&
			           !SCB_PS_PRETEND(scb)) {
				/* 3 */
				pps_retry = FALSE;
			} else if (SCB_PS_PRETEND_ENABLED(bsscfg, scb)) {
				/* 4 */
				wlc_pkt_get_vht_hdr(wlc, p, &vhtHdr);
				macCtlHigh = ltoh16(vhtHdr->PktInfo.MacTxControlHigh);

				if (macCtlHigh & D11AC_TXC_PPS) {
					/* 5 */
					pps_retry = TRUE;
				} else {
					/* packet does not have PPS bit set.
					 * We normally get here if we previously
					 * exceeded the pspretend_retry_limit and the
					 * packet subsequently fails to be sent
					 */
					pps_retry = FALSE;
					/* unset vhtHdr, macCtlHigh for next p,
					 * because we assume they are
					 * only valid when pps_retry is TRUE
					 */
					vhtHdr = NULL;
					macCtlHigh = 0;
				}
			} else if (SCB_PS_PRETEND_THRESHOLD(scb)) {
				/* 6 */
				wlc_pkt_get_vht_hdr(wlc, p, &vhtHdr);
				macCtlHigh = ltoh16(vhtHdr->PktInfo.MacTxControlHigh);
				pps_retry = TRUE;
			} else if (SCB_PS_PRETEND_CSA_PREVENT(scb, bsscfg) &&
					!was_acked && !supr_status) {
			        /* During the period around CSA, PSP is explicitly turned off.
				 * Many packets during this time may get dropped due to no
				 * suppression and no ack. To meet ZPL requirements, these
				 * packets should not be dropped but rather explicitly re-fetched
				 * and re-tried.
				 */
				wlc_pkt_get_vht_hdr(wlc, p, &vhtHdr);
				macCtlHigh = ltoh16(vhtHdr->PktInfo.MacTxControlHigh);
				pps_retry = TRUE;
			}
		}
#endif /* PSPRETEND */

#ifdef PKTQ_LOG
		if (!was_acked && supr_status) {
			WLCNTCONDINCR(prec_cnt, prec_cnt->suppress);
			WLCNTCONDINCR(actq_cnt, actq_cnt->suppress);
		}
#endif

#ifdef WL_TX_STALL
		if (!was_acked) {
			if (supr_status != TX_STATUS_SUPR_NONE) {
				WL_TX_STS_UPDATE(tx_status,
					wlc_tx_status_map_hw_to_sw_supr_code(wlc, supr_status));
			} else if (txs->phyerr) {
				WL_TX_STS_UPDATE(tx_status, WLC_TX_STS_PHY_ERROR);
			} else {
				WL_TX_STS_UPDATE(tx_status, WLC_TX_STS_RETRY_TIMEOUT);
			}

			wlc_tx_status_update_counters(wlc, p, scb, bsscfg, tx_status, 1);

			/* Clear status, next packet will have its own status */
			WL_TX_STS_UPDATE(tx_status, WLC_TX_STS_SUCCESS);
		}
#endif /* WL_TX_STALL */

		if (was_acked) { /* if current MPDU was acked by remote party */
			WL_AMPDU_TX(("wl%d: %s pkt ack seq 0x%04x idx %d\n",
				wlc->pub->unit, __FUNCTION__, seq, tot_mpdu));
			ini->acked_seq = seq;

			/* update the scb used time */
			scb->used = wlc->pub->now;

#ifdef PKTQ_LOG
			WLCNTCONDINCR(prec_cnt, prec_cnt->acked);
			WLCNTCONDINCR(actq_cnt, actq_cnt->acked);
			SCB_BS_DATA_CONDINCR(bs_data_counters, acked);
			WLCNTCONDADD(prec_cnt, prec_cnt->throughput, prec_pktlen);
			WLCNTCONDADD(actq_cnt, actq_cnt->throughput, prec_pktlen);
			SCB_BS_DATA_CONDADD(bs_data_counters, throughput, prec_pktlen);
			WLCNTSCBADD(scb_ampdu->ampdu_scb_stats->tx_bytes_total[ini->tid],
				prec_pktlen);
#endif

#if defined(BCMCCX) && defined(CCX_SDK)
			if (IHV_ENAB(wlc->ccx, bsscfg) &&
			    BSSCFG_STA(bsscfg) && bsscfg->BSS &&
			    wlc->ccx->frame_log) {
				wlc_get_txh_info(wlc, p, txh_info);
				wlc_ccx_log_tx_frame(wlc->ccx, (uint8*)txh_info->d11HdrPtr,
					txh_info->d11FrameSize, TX_STATUS_ACK_RCV, FALSE, TRUE);
			}
			/* call any matching pkt callbacks */
			if (pkttag->flags & WLF_IHV_TX_PKT) {
				/* additional information for IHV pkt callback */
				wlc->ccx->ihv_txpkt = p;
				wlc->ccx->ihv_txpkt_sent = TRUE;
				wlc->ccx->ihv_txpkt_max_retries = FALSE;
			}
#endif /* BCMCCX && CCX_SDK */

			/* for *each* acked MPDU within an AMPDU, call a packet callback */
			wlc_pcb_fn_invoke(wlc->pcb, p, TX_STATUS_ACK_RCV);

		}

#ifdef PSPRETEND
		if (pps_retry) {
			ASSERT(!was_acked);

			if (!supr_status) {
#ifdef PKTQ_LOG
				WLCNTCONDINCR(prec_cnt, prec_cnt->ps_retry);
				WLCNTCONDINCR(actq_cnt, actq_cnt->ps_retry);
#endif
#ifdef WLINTFERSTAT
				wlc_trf_mgmt_scb_txfail_detect(wlc->trf_mgmt_ctxt, scb);
#endif
			}

			/* re-enqueue the packet suppressed due to ps pretend */
			free_mpdu = wlc_pspretend_pkt_retry(wlc->pps_info, bsscfg, scb, p,
			                                   txs, vhtHdr, macCtlHigh);

			/* unset vhtHdr, macCtlHigh for next p, because we assume they are
			 * only valid when pps_retry is TRUE
			 */
			vhtHdr = NULL;
			macCtlHigh = 0;
		}
#endif /* PSPRETEND */

		/* either retransmit or send bar if no ack received for this MPDU */
		if (!was_acked &&
#ifdef PSPRETEND
		    !pps_retry &&
#endif /* PSPRETEND */
#ifdef WL_CS_PKTRETRY
		    !cs_retry &&
#endif
		    TRUE) {
			/* re-queues MPDU if applicable */
			if (retry_suppr_pkts) {
				if (AP_ENAB(wlc->pub) &&
				    supr_status == TX_STATUS_SUPR_PMQ) {
					/* last_frag TRUE as fragmentation not allowed for AMPDU */
					free_mpdu = wlc_apps_suppr_frame_enq(wlc, p, txs, TRUE);
				} else if (P2P_ABS_SUPR(wlc, supr_status) &&
				         BSS_TX_SUPR_ENAB(bsscfg)) {
					/* This is possible if we got a packet suppression
					 * before getting ABS interrupt
					 */
					if (!BSS_TX_SUPR(bsscfg)) {
						wlc_bsscfg_tx_stop(wlc->psqi, bsscfg);
					}

					if (BSSCFG_AP(bsscfg) &&
					    SCB_ASSOCIATED(scb) && SCB_P2P(scb))
						wlc_apps_scb_tx_block(wlc, scb, 1, TRUE);
					/* With Proptxstatus enabled in dongle and host,
					 * pkts sent by host will not come here as retry is set
					 * to FALSE by wlc_should_retry_suppressed_pkt().
					 * With Proptxstatus disabled in dongle or
					 * not active in host, all the packets will come here
					 * and need to be re-enqueued.
					 */
					free_mpdu = wlc_pkt_abs_supr_enq(wlc, scb, p);
				}
				/* If the frame was not successfully enqueued for retry, handle
				 * the failure
				 */
				if (free_mpdu) {
					goto not_retry;
				}
#if defined(TXQ_MUX)
				else {
					/* if the frame was successfully re-enqueued, it is on the
					 * PS reorder queue and will be moved to the SCBQ. Since the
					 * pkt is moved above the ampdu output function, decrement
					 * the tx_in_transit count and remove the AMPDU pkt
					 * callback.  tx_in_transit will be accounted for again and
					 * the pkt callback re-registerd in the output fn.
					 */
					ini->tx_in_transit--;
					WLF2_PCB4_UNREG(p);
				}
#endif /* TXQ_MUX */
			} else {
not_retry:		/* don't retry an unacked MPDU, but send BAR if applicable */
				bar_seq = seq;
#ifdef PROP_TXSTATUS
				if (PROP_TXSTATUS_ENAB(wlc->pub) &&
					(!WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wlfc)) ||
					!supr_status))
#endif /* PROP_TXSTATUS */
				{
					send_bar = TRUE;
				}

				WL_AMPDU_TX(("wl%d: %s: pkt seq 0x%04x not acked\n",
					wlc->pub->unit, __FUNCTION__, seq));

#ifdef WLMEDIA_TXFAILEVENT
				wlc_tx_failed_event(wlc, scb, bsscfg, txs, p, 0);
#endif /* WLMEDIA_TXFAILEVENT */

#ifdef PKTQ_LOG
				WLCNTCONDINCR(prec_cnt, prec_cnt->retry_drop);
				WLCNTCONDINCR(actq_cnt, actq_cnt->retry_drop);
				SCB_BS_DATA_CONDINCR(bs_data_counters, retry_drop);
#endif
#ifdef WLINTFERSTAT
				/* Count only failed packets and indicate failure if required */
				wlc_trf_mgmt_scb_txfail_detect(wlc->trf_mgmt_ctxt, scb);
#endif
			} // if (retry_suppr_pkts)

#ifdef PROP_TXSTATUS
			if (PROP_TXSTATUS_ENAB(wlc->pub) && free_mpdu &&
			    supr_status && WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wlfc))) {
			}
#endif /* PROP_TXSTATUS */
		} /* if (not acked) */

#ifdef PROP_TXSTATUS
		if (PROP_TXSTATUS_ENAB(wlc->pub)) {
			uint8 wlfc_status =  wlc_txstatus_interpret(&txs->status, was_acked);
			if (!was_acked && scb && (wlfc_status == WLFC_CTL_PKTFLAG_D11SUPPRESS)) {
				if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub))
					wlc_suppress_sync_fsm(wlc, scb, p, TRUE);
				wlc_process_wlhdr_txstatus(wlc, wlfc_status, p, FALSE);
			} else {
				wlfc_process_txstatus(wlc->wlfc, wlfc_status, p,
					txs, ((tot_mpdu < (ncons - 1)) ? TRUE : FALSE));
			}
		}
#endif /* PROP_TXSTATUS */

		if (free_mpdu) {
#if defined(WLPKTDLYSTAT) || defined(WL11K)
			/* calculate latency and packet loss statistics */
			/* Ignore wrap-around case and error case */
			if (now > pkttag->shared.enqtime) {
				delay = (now - pkttag->shared.enqtime);
#ifdef WLPKTDLYSTAT
				tr = 0;
				wlc_delay_stats_upd(delay_stats, delay, tr, was_acked);
				WL_AMPDU_STAT(("Enq %d retry %d cnt %d: acked %d delay/min/"
					"max/sum %d %d %d %d\n", pkttag->shared.enqtime,
					tr, delay_stats->txmpdu_cnt[tr], was_acked, delay,
					delay_stats->delay_min, delay_stats->delay_max,
					delay_stats->delay_sum[tr]));
#endif /* WLPKTDLYSTAT */
#ifdef WL11K
				if (was_acked) {
					wlc_rrm_delay_upd(scb, tid, delay);
				}
#endif /* WL11K */
			}
#endif /* WLPKTDLYSTAT || WL11K */

			if (supr_status == TX_STATUS_INT_XFER_ERR) {
				WL_ERROR(("wl%d %s: acked %d internal dma error\n",
					wlc->pub->unit, __FUNCTION__, was_acked));
			}
			PKTFREE(wlc->osh, p, TRUE);
		} else {
			/* Packet requeued, update the counters */
			wlc_tx_status_update_counters(wlc, p,
				scb, bsscfg, WLC_TX_STS_QUEUED, 1);
		}

		tot_mpdu++;

		/* only loop for the #mpdus that the d11 core indicated in txs */
		if (tot_mpdu >= ncons) {
			break;
		}

		/* get next pkt from d11 dma/fifo queue -- check and handle NULL */
		p = GETNEXTTXP(wlc, queue);
#ifdef BCMDBG
		if (ini->tid != PKTPRIO(p)) {
			WL_ERROR(("%s %d tid mismatch ini tid:%d p prio:%d queue:%d\n",
				__FUNCTION__, __LINE__, ini->tid, PKTPRIO(p), queue));
			ASSERT(ini->tid == PKTPRIO(p));
		}
#endif

		/* if tot_mpdu < ncons, should have pkt in queue */
		if (p == NULL) {
			WL_AMPDU_ERR(("%s: p is NULL. tot_mpdu: %d, ncons: %d\n",
				__FUNCTION__, tot_mpdu, ncons));
			ASSERT(p);
			break;
		}

		WLPKTTAGSCBSET(p, scb);

#ifdef PKTQ_LOG
		if (prec_cnt) {
			/* get info from next header to give accurate packet length */
			int tsoHdrSize = wlc_pkt_get_vht_hdr(wlc, p, &vhtHdr);
			int hdrSize;

			if (vhtHdr->PktInfo.MacTxControlLow & htol16(D11AC_TXC_HDR_FMT_SHORT)) {
				hdrSize = D11AC_TXH_SHORT_LEN;
			} else
#ifdef BCM_SFD
			if (SFD_ENAB(wlc->pub) && PKTISSFDFRAME(wlc->osh, p)) {
				hdrSize = D11AC_TXH_SFD_LEN;
			} else
#endif
			{
				hdrSize = D11AC_TXH_LEN;
			}

			prec_pktlen = pkttotlen(wlc->osh, p) - (tsoHdrSize + hdrSize) -
			              DOT11_LLC_SNAP_HDR_LEN - DOT11_MAC_HDR_LEN -
			              DOT11_QOS_LEN;
			vhtHdr = NULL;
		}
#endif /* PKTQ_LOG */
	} /* while (TRUE) -> for each AMPDU in caller supplied txs */

	WLC_TXFIFO_COMPLETE(wlc, queue, tot_mpdu, tx_time);

#ifdef PKTQ_LOG
	/* cannot distinguish hi or lo prec hereafter - so just standardise lo prec */
	if (scb_ampdu->txq.pktqlog) {
		prec_cnt = wlc_ampdu_pktqlog_cnt(&scb_ampdu->txq, tid, WLC_PRIO_TO_PREC(tid));
	}

	/* we are in 'aqm' complete function so expect at least corerev 40 to get this */
	WLCNTCONDADD(prec_cnt, prec_cnt->airtime, TX_STATUS40_TX_MEDIUM_DELAY(txs));
#endif

	if (send_bar) {
		if  ((MODSUB_POW2(ini->max_seq, bar_seq, SEQNUM_MAX) >= ini->ba_wsize) &&
		     (MODSUB_POW2(bar_seq, (ini->acked_seq), SEQNUM_MAX) < ini->ba_wsize * 2)) {
			send_bar = FALSE;
			WL_AMPDU(("wl%d: %s: skipping BAR %x, max_seq %x, acked_seq %x\n",
				wlc->pub->unit, __FUNCTION__, MODINC_POW2(seq, SEQNUM_MAX),
				ini->max_seq, ini->acked_seq));
		} else if (ini->bar_ackpending) {
			/*
			 * check if no bar with newer seq has been send out due to other condition,
			 * like unexpected tx seq
			 */
			if (!IS_SEQ_ADVANCED(MODINC_POW2(bar_seq, SEQNUM_MAX), ini->barpending_seq))
				send_bar = FALSE;
		}

		if (send_bar && (ini->retry_bar != AMPDU_R_BAR_BLOCKED)) {
			bar_seq = MODINC_POW2(seq, SEQNUM_MAX);
			if (IS_SEQ_ADVANCED (bar_seq, ini->barpending_seq) ||
			    (ini->barpending_seq == SEQNUM_MAX))
				ini->barpending_seq = bar_seq;
			wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
		}
	}

#if defined(BCMDBG) || defined(BCMDBG_AMPDU) || defined(WL_LINKSTAT)
	if (nlost) {
		AMPDUSCBCNTADD(scb_ampdu->cnt.txlost, nlost);
		WLCNTADD(ampdu_tx->cnt->txlost, nlost);
		WLCNTADD(wlc->pub->_cnt->txlost, nlost);
		WL_AMPDU_ERR(("wl%d aqm_txs: nlost %d send_bar %d "
			     "vht %d mcs[0-3] %04x %04x %04x %04x\n",
			      wlc->pub->unit, nlost, send_bar,
			      is_vht[0], rs_txs.txrspec[0], rs_txs.txrspec[1], rs_txs.txrspec[2],
			      rs_txs.txrspec[3]));
		WL_AMPDU_ERR(("raw txstatus %04X %04X %04X | s3-5 %08X %08X %08X | "
			"%08X %08X | s8 %08X\n",
			txs->status.raw_bits, txs->frameid, txs->sequence,
			txs->status.s3, txs->status.s4, txs->status.s5,
			txs->status.ack_map1, txs->status.ack_map2, txs->status.s8));
	}
#endif 

#ifdef PSPRETEND
	if ((BSSCFG_AP(bsscfg) || BSSCFG_IBSS(bsscfg)) &&
	    PS_PRETEND_ENABLED(bsscfg)) {
		wlc_pspretend_dotxstatus(wlc->pps_info, bsscfg, scb, pps_recvd_ack);
	}
#endif /* PSPRETEND */

	/* bump up the start seq to move the window */
	wlc_ampdu_ini_move_window(ampdu_tx, scb_ampdu, ini);

	WL_AMPDU_TX(("wl%d: %s: ncons %d tot_mpdus %d start_seq 0x%04x\n",
		wlc->pub->unit, __FUNCTION__, ncons, tot_mpdu, start_seq));

#ifdef STA
	/* PM state change */
	wlc_update_pmstate(bsscfg, (uint)(wlc_txs_alias_to_old_fmt(wlc, &(txs->status))));
#endif /* STA */

#ifdef PKTQ_LOG
	WLCNTCONDADD(prec_cnt, prec_cnt->rtsfail, txs->status.rts_tx_cnt - txs->status.cts_rx_cnt);
	WLCNTCONDADD(actq_cnt, actq_cnt->rtsfail, txs->status.rts_tx_cnt - txs->status.cts_rx_cnt);
	SCB_BS_DATA_CONDADD(bs_data_counters,
		rtsfail, txs->status.rts_tx_cnt - txs->status.cts_rx_cnt);
#endif
	if (nlost) {
		/* Per-scb statistics: scb must be valid here */
		WLCNTADD(wlc->pub->_cnt->txfail, nlost);
		if (!from_host) {
			WLCNTSCBADD(scb->scb_stats.tx_pkts_fw_retry_exhausted, nlost);
		} else {
			WLCNTSCBADD(scb->scb_stats.tx_pkts_retry_exhausted, nlost);
		}
		WLCIFCNTADD(scb, txfail, nlost);
		WLCNTSCBADD(scb->scb_stats.tx_failures, nlost);
#ifdef WL11K
		wlc_rrm_tscm_upd(scb, tid, OFFSETOF(rrm_tscm_t, msdu_fail), nlost);
#endif
	}

	/* MU txstatus uses RT1~RT3 for other purposes */
	if (fix_rate && !txs_mu) {
		/* if using fix rate, retrying 64 mpdus >=4 times can overflow 8-bit cnt.
		 * So ucode treats fix rate specially.
		 */
		uint32    retry_count;
#ifdef PKTQ_LOG
		uint32    txrate_succ = 0;
#endif
		rs_txs.tx_cnt[0]     = txs->status.s3 & 0xffff;
		rs_txs.txsucc_cnt[0] = (txs->status.s3 >> 16) & 0xffff;
		retry_count = rs_txs.tx_cnt[0] - rs_txs.txsucc_cnt[0];
		BCM_REFERENCE(retry_count);

		WLCNTADD(wlc->pub->_cnt->txfrag, rs_txs.txsucc_cnt[0]);
		WLCNTADD(wlc->pub->_cnt->txfrmsnt, rs_txs.txsucc_cnt[0]);
		WLCNTADD(wlc->pub->_cnt->txretrans, retry_count);
		if (!from_host) {
			WLCNTSCBADD(scb->scb_stats.tx_pkts_fw_total, rs_txs.txsucc_cnt[0]);
			WLCNTSCBADD(scb->scb_stats.tx_pkts_fw_retries, retry_count);
		} else {
			WLCNTSCBADD(scb->scb_stats.tx_pkts_total, rs_txs.txsucc_cnt[0]);
			WLCNTSCBADD(scb->scb_stats.tx_pkts_retries, retry_count);
		}

		WLCNTSCBADD(scb_ampdu->ampdu_scb_stats->tx_pkts_total[ini->tid],
			rs_txs.txsucc_cnt[0]);
		WLCIFCNTADD(scb, txfrmsnt, rs_txs.txsucc_cnt[0]);
#ifdef WL11K
		wlc_rrm_tscm_upd(scb, tid, OFFSETOF(rrm_tscm_t, msdu_tx), rs_txs.txsucc_cnt[0]);
#endif
#ifdef PKTQ_LOG
		if (rs_txs.txsucc_cnt[0]) {
			txrate_succ = mcs_txrate[0] * rs_txs.txsucc_cnt[0];
		}

		WLCNTCONDADD(prec_cnt, prec_cnt->retry, retry_count);
		WLCNTCONDADD(actq_cnt, actq_cnt->retry, retry_count);
		SCB_BS_DATA_CONDADD(bs_data_counters, retry, retry_count);
		WLCNTCONDADD(prec_cnt, prec_cnt->txrate_main, mcs_txrate[0] * rs_txs.tx_cnt[0]);
		WLCNTCONDADD(actq_cnt, actq_cnt->txrate_main, mcs_txrate[0] * rs_txs.tx_cnt[0]);
		SCB_BS_DATA_CONDADD(bs_data_counters, txrate_main,
			mcs_txrate[0] * rs_txs.tx_cnt[0]);
		WLCNTCONDADD(prec_cnt, prec_cnt->txrate_succ, txrate_succ);
		WLCNTCONDADD(actq_cnt, actq_cnt->txrate_succ, txrate_succ);
		SCB_BS_DATA_CONDADD(bs_data_counters, txrate_succ, txrate_succ);
#endif
	} else {
		uint32    succ_count;
		uint32    try_count;
		uint32    retry_count;
#ifdef PKTQ_LOG
		uint32    txrate_succ = 0;
#endif
		rs_txs.tx_cnt[0]     = txs->status.s3 & 0xff;
		rs_txs.txsucc_cnt[0] = (txs->status.s3 >> 8) & 0xff;
		if (!txs_mu) {
			rs_txs.tx_cnt[1]     = (txs->status.s3 >> 16) & 0xff;
			rs_txs.txsucc_cnt[1] = (txs->status.s3 >> 24) & 0xff;
			rs_txs.tx_cnt[2]     = (txs->status.s4 >>  0) & 0xff;
			rs_txs.txsucc_cnt[2] = (txs->status.s4 >>  8) & 0xff;
			rs_txs.tx_cnt[3]     = (txs->status.s4 >> 16) & 0xff;
			rs_txs.txsucc_cnt[3] = (txs->status.s4 >> 24) & 0xff;
		}
		succ_count = rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1] +
			rs_txs.txsucc_cnt[2] + rs_txs.txsucc_cnt[3];
		try_count = rs_txs.tx_cnt[0] + rs_txs.tx_cnt[1] +
			rs_txs.tx_cnt[2] + rs_txs.tx_cnt[3];
		retry_count = try_count - succ_count;
		BCM_REFERENCE(succ_count);
		BCM_REFERENCE(try_count);
		BCM_REFERENCE(retry_count);

		WLCNTADD(wlc->pub->_cnt->txfrag, succ_count);
		WLCNTADD(wlc->pub->_cnt->txfrmsnt, succ_count);
		WLCNTADD(wlc->pub->_cnt->txretrans, retry_count);
		if (!from_host) {
			WLCNTSCBADD(scb->scb_stats.tx_pkts_fw_total, succ_count);
			WLCNTSCBADD(scb->scb_stats.tx_pkts_fw_retries, retry_count);
		} else {
			WLCNTSCBADD(scb->scb_stats.tx_pkts_total, succ_count);
			WLCNTSCBADD(scb->scb_stats.tx_pkts_retries, retry_count);
		}

		WLCNTSCBADD(scb_ampdu->ampdu_scb_stats->tx_pkts_total[ini->tid], succ_count);
		WLCIFCNTADD(scb, txfrmsnt, succ_count);
#ifdef WL11K
		wlc_rrm_tscm_upd(scb, tid, OFFSETOF(rrm_tscm_t, msdu_tx), succ_count);
#endif

#ifdef PKTQ_LOG
		if (succ_count) {
			txrate_succ = mcs_txrate[0] * rs_txs.txsucc_cnt[0] +
			              mcs_txrate[1] * rs_txs.txsucc_cnt[1] +
			              mcs_txrate[2] * rs_txs.txsucc_cnt[2] +
			              mcs_txrate[3] * rs_txs.txsucc_cnt[3];
		}

		WLCNTCONDADD(prec_cnt, prec_cnt->retry, retry_count);
		WLCNTCONDADD(actq_cnt, actq_cnt->retry, retry_count);
		SCB_BS_DATA_CONDADD(bs_data_counters, retry, retry_count);
		WLCNTCONDADD(prec_cnt, prec_cnt->txrate_main, mcs_txrate[0] * try_count);
		WLCNTCONDADD(actq_cnt, actq_cnt->txrate_main, mcs_txrate[0] * try_count);
		SCB_BS_DATA_CONDADD(bs_data_counters, txrate_main, mcs_txrate[0] * try_count);
		WLCNTCONDADD(prec_cnt, prec_cnt->txrate_succ, txrate_succ);
		WLCNTCONDADD(actq_cnt, actq_cnt->txrate_succ, txrate_succ);
		SCB_BS_DATA_CONDADD(bs_data_counters, txrate_succ, txrate_succ);
#endif /* PKTQ_LOG */
	}
#ifdef BCMDBG
	for (k = 0; k < rnum; k++) {
		if (rs_txs.txsucc_cnt[k] > rs_txs.tx_cnt[k]) {
			WL_AMPDU_ERR(("%s: rs_txs.txsucc_cnt[%d] > rs_txs.tx_cnt[%d]: %d > %d\n",
				__FUNCTION__, k, k, rs_txs.txsucc_cnt[k], rs_txs.tx_cnt[k]));
			WL_AMPDU_ERR(("ncons %d raw txstatus %08X | %04X %04X | %08X %08X || "
				 "%08X | %08X %08X\n", ncons,
				  txs->status.raw_bits, txs->sequence, txs->phyerr,
				  txs->status.s3, txs->status.s4, txs->status.s5,
				  txs->status.ack_map1, txs->status.ack_map2));
		}
	}
#endif

#if defined(BCMDBG) || defined(BCMDBG_AMPDU) || defined(WL_LINKSTAT)
	for (k = 0; k < rnum; k++) {
		uint8 ht_mcs = rs_txs.txrspec[k] & RSPEC_RATE_MASK;
		if (ampdu_tx->amdbg) {
			if (!is_vht[k])  {
				WLCNTADD(ampdu_tx->amdbg->txmcs[MCS2IDX(ht_mcs)], rs_txs.tx_cnt[k]);
				WLCNTADD(ampdu_tx->amdbg->txmcs_succ[MCS2IDX(ht_mcs)],
					rs_txs.txsucc_cnt[k]);
			}
#ifdef WL11AC
			else {
				WLCNTADD(ampdu_tx->amdbg->txvht[vht[k]], rs_txs.tx_cnt[k]);
				WLCNTADD(ampdu_tx->amdbg->txvht_succ[vht[k]], rs_txs.txsucc_cnt[k]);
			}
#endif
			if (rs_txs.txrspec[k] & RSPEC_SHORT_GI) {
				WLCNTADD(ampdu_tx->cnt->u1.txmpdu_sgi, rs_txs.tx_cnt[k]);
				if (!is_vht[k])
					WLCNTADD(ampdu_tx->amdbg->txmcssgi[MCS2IDX(ht_mcs)],
					rs_txs.tx_cnt[k]);
#ifdef WL11AC
				else
					WLCNTADD(ampdu_tx->amdbg->txvhtsgi[vht[k]],
						rs_txs.tx_cnt[k]);
#endif
			}

			if (is_stbc[k]) {
				WLCNTADD(ampdu_tx->cnt->u2.txmpdu_stbc, rs_txs.tx_cnt[k]);
				if (!is_vht[k])
					WLCNTADD(ampdu_tx->amdbg->txmcsstbc[MCS2IDX(ht_mcs)],
						rs_txs.tx_cnt[k]);
#ifdef WL11AC
				else
					WLCNTADD(ampdu_tx->amdbg->txvhtstbc[vht[k]],
						rs_txs.tx_cnt[k]);
#endif
			}
		}
#ifdef WLPKTDLYSTAT
		if (!is_vht[k]) {
			WLCNTADD(ampdu_tx->cnt->txmpdu_cnt[MCS2IDX(ht_mcs)], rs_txs.tx_cnt[k]);
			WLCNTADD(ampdu_tx->cnt->txmpdu_succ_cnt[MCS2IDX(ht_mcs)],
				rs_txs.txsucc_cnt[k]);
		}
#ifdef WL11AC
		else {
			WLCNTADD(ampdu_tx->cnt->txmpdu_vht_cnt[vht[k]], rs_txs.tx_cnt[k]);
			WLCNTADD(ampdu_tx->cnt->txmpdu_vht_succ_cnt[vht[k]], rs_txs.txsucc_cnt[k]);
		}
#endif /* WL11AC */
#endif /* WLPKTDLYSTAT */
	}
#endif 

#if defined(WL_MU_TX)
	if (MU_TX_ENAB(wlc) && SCB_MU(scb)) {
#if defined(WLCNT)
		ratespec_t rspec;
		/* update mutx stats */
		/* For MU we need to retrieve GID and ratespec_t info from tx_status_t. */
		if (txs_mu) {
			gid = TX_STATUS64_MU_GID(txs->status.s3);
			rspec = TX_STATUS64_MU_RSPEC(txs->status.s4);
			rspec += (1 << TX_STATUS64_MU_NSS_SHIFT);	/* Calculate actual NSS */
		}
		else {
			/* Clear the bit 7 which only indicates VHT */
			rspec = rs_txs.txrspec[0] & ~(0x80);
		}

		wlc_mutx_update_txcounters(wlc->mutx, scb, rspec, is_mu,
				txs_mu,	(uint16)ncons, nlost, gid, txs);
#endif /* WLCNT */
		tx_flags |= (txs_mu ? RATESEL_TXS_MUTX : 0);
	}
#endif /* WL_MU_TX */
	if (update_ratesel) {
		/* handle the case using fix rate flag in txd but still want to update
		 * rate selection
		 */
		uint fbr_cnt = 0, fbr_succ = 0;

		if (rs_txs.tx_cnt[0] > 0xff) rs_txs.tx_cnt[0] = 0xff;
		if (rs_txs.txsucc_cnt[0] > 0xff) rs_txs.txsucc_cnt[0] = 0xff;

		for (k = 1; k < rnum; k++) {
			fbr_cnt += rs_txs.tx_cnt[k];
			fbr_succ += rs_txs.txsucc_cnt[k];
		}

		rs_txs.txrts_cnt = txs->status.rts_tx_cnt;
		rs_txs.rxcts_cnt = txs->status.cts_rx_cnt;
		rs_txs.nlost = nlost;
		rs_txs.ack_map1 = txs->status.ack_map1;
		rs_txs.ack_map2 = txs->status.ack_map2;
		rs_txs.frameid = txs->frameid;
		/* zero for aqm */
		rs_txs.antselid = 0;

		if (rs_txs.tx_cnt[0] == 0 && fbr_cnt == 0) {
			rs_txs.tx_cnt[0] = (ncons > 0xff) ? 0xff : (uint8)ncons;
			rs_txs.txsucc_cnt[0] = 0;
			rs_txs.tx_cnt[1] = 0;
			rs_txs.txsucc_cnt[1] = 0;
#ifdef PSPRETEND
			if (pps_retry) {
				WL_PS(("wl%d.%d: "MACF" pps retry with only RTS failure\n",
				        wlc->pub->unit,
				        WLC_BSSCFG_IDX(bsscfg), ETHER_TO_MACF(scb->ea)));
			}
#endif /* PSPRETEND */
		} else {
#ifdef PSPRETEND
			/* we have done successive attempts to send the packet and it is
			 * not working, despite RTS going through. Suspect that the rate
			 * info is wrong so reset it
			 */
			if (SCB_PS_PRETEND(scb) && !SCB_PS_PRETEND_THRESHOLD(scb)) {
				wlc_pspretend_rate_reset(wlc->pps_info, bsscfg, scb,
					WME_PRIO2AC(PKTPRIO(p)));
			}
#endif /* PSPRETEND */
		}

		/* notify rate selection */
		wlc_scb_ratesel_upd_txs_ampdu(wlc->wrsi, scb, &rs_txs, txs, tx_flags);
	} // if (update_ratesel)

	if (!supr_status) {
		/* if primary succeeds, no restrict */
		wlc_scb_restrict_txstatus(scb, rs_txs.txsucc_cnt[0] != 0);
	}
} /* wlc_ampdu_dotxstatus_aqm_complete */

#endif /* WLAMPDU_MAC */

static void BCMFASTPATH
wlc_ampdu_dotxstatus_complete(ampdu_tx_info_t *ampdu_tx, struct scb *scb, void *p, tx_status_t *txs,
	uint32 s1, uint32 s2, uint32 s3, uint32 s4)
{
	scb_ampdu_tx_t *scb_ampdu;
	wlc_info_t *wlc = ampdu_tx->wlc;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	scb_ampdu_tid_ini_t *ini;
	uint8 bitmap[8], queue, tid, requeue = 0;
	d11txh_t *txh = NULL;
	uint8 *plcp = NULL;
	struct dot11_header *h;
	uint16 seq, start_seq = 0, bindex, indx, mcl;
	uint8 mcs = 0;
	bool is_sgi = FALSE, is40 = FALSE;
	bool ba_recd = FALSE, ack_recd = FALSE, send_bar = FALSE;
	uint8 suc_mpdu = 0, tot_mpdu = 0;
	uint supr_status;
	bool update_rate = TRUE, retry = TRUE;
	int txretry = -1;
	uint16 mimoantsel = 0;
	uint8 antselid = 0;
	uint8 retry_limit, rr_retry_limit;
	wlc_bsscfg_t *bsscfg;
	bool pkt_freed = FALSE;
#if defined(TXQ_MUX)
	int tx_time = 0;
#endif /* TXQ_MUX */
#if defined(WLPKTDLYSTAT) || defined(PKTQ_LOG)
	uint8 mcs_fbr = 0;
#endif /* WLPKTDLYSTAT || PKTQ_LOG */
	uint8 ac;
#ifdef WLPKTDLYSTAT
	scb_delay_stats_t *delay_stats;
	uint tr;
#endif /* WLPKTDLYSTAT */
#if defined(WLPKTDLYSTAT) || defined(WL11K)
	uint32 delay, now;
#endif
#ifdef PKTQ_LOG
	int prec_index;

	pktq_counters_t *prec_cnt = NULL;
	pktq_counters_t *actq_cnt = NULL;
	uint32           prec_pktlen;
	uint32 tot_len = 0, pktlen = 0;
	uint32 txrate_succ = 0, current_rate, airtime = 0;
	uint bw;
	ratespec_t rspec;
	uint ctl_rspec;
	uint flg;
	uint32 rts_sifs, cts_sifs, ba_sifs;
	uint32 ampdu_dur;
#endif /* PKTQ_LOG */
#if defined(SCB_BS_DATA)
	wlc_bs_data_counters_t *bs_data_counters = NULL;
#endif /* SCB_BS_DATA */

#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */
	int pdu_count = 0;
	uint16 first_frameid = 0xffff;
	uint16 ncons = 0; /**< number of consumed packets */
#endif /* WLAMPDU_MAC */
	ratesel_txs_t rs_txs;

#ifdef PROP_TXSTATUS
	uint8 wlfc_suppr_acked_status = 0;
#endif /* PROP_TXSTATUS */
#ifdef WL_TX_STALL
	wlc_tx_status_t tx_status = WLC_TX_STS_SUCCESS;
#endif	/* WL_TX_STALL */

#if defined(BCMDBG) || defined(BCMDBG_AMPDU)
	uint8 hole[AMPDU_MAX_MPDU];
	uint8 idx = 0;
	memset(hole, 0, sizeof(hole));
#endif 

	bzero(&rs_txs, sizeof(rs_txs));

	ASSERT(wlc);

	if (p != NULL && (WLPKTTAG(p)->flags & WLF_AMPDU_MPDU) == 0) {
		WL_PRINT(("%s flags=%x seq=%d s1=%d s2=%d s3=%d s4=%d\n", __FUNCTION__,
			WLPKTTAG(p)->flags, WLPKTTAG(p)->seq, s1, s2, s3, s4));
	}

	ASSERT(p && (WLPKTTAG(p)->flags & WLF_AMPDU_MPDU));

	if (!scb || !SCB_AMPDU(scb)) {
		if (!scb) {
			WL_AMPDU_ERR(("wl%d: %s: scb is null\n",
				wlc->pub->unit, __FUNCTION__));
			/* pkttag must have been set null in wlc_dotxstatus */
			ASSERT(WLPKTTAGSCBGET(p) == NULL);
		} else {
			WLPKTTAGSCBSET(p, NULL);
		}
		wlc_ampdu_free_chain(ampdu_tx, p, txs, FALSE);
		return;
	}

	SCB_BS_DATA_CONDFIND(bs_data_counters, wlc, scb);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	tid = (uint8)PKTPRIO(p);
	ac = WME_PRIO2AC(tid);
#ifdef WLPKTDLYSTAT
	delay_stats = scb->delay_stats + ac;
#endif /* WLPKTDLYSTAT */
	ini = scb_ampdu->ini[tid];
	retry_limit = ampdu_tx_cfg->retry_limit_tid[tid];
	rr_retry_limit = ampdu_tx_cfg->rr_retry_limit_tid[tid];

	if (!ini || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
		wlc_ampdu_free_chain(ampdu_tx, p, txs, FALSE);
		return;
	}

	ASSERT(ini->scb == scb);
	ASSERT(WLPKTTAGSCBGET(p) == scb);

	bzero(bitmap, sizeof(bitmap));
	queue = WLC_TXFID_GET_QUEUE(txs->frameid);
	ASSERT(queue < AC_COUNT);

	update_rate = (WLPKTTAG(p)->flags & WLF_RATE_AUTO) ? TRUE : FALSE;
	supr_status = txs->status.suppr_ind;

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);
	BCM_REFERENCE(bsscfg);

#ifdef PKTQ_LOG
	prec_index = WLAMPDU_PREC_TID2PREC(p, tid);

	if (scb_ampdu->txq.pktqlog) {
		prec_cnt = wlc_ampdu_pktqlog_cnt(&scb_ampdu->txq, tid, prec_index);
	}

	if ((WLC_GET_TXQ(wlc->active_queue))->pktqlog) {
		actq_cnt =
		(WLC_GET_TXQ(wlc->active_queue))->pktqlog->_prec_cnt[prec_index];
	}
#endif /* PKTQ_LOG */

#ifdef WLAMPDU_HW
	if (AMPDU_HW_ENAB(wlc->pub)) {
		WL_AMPDU_HWDBG(("%s: cons %d burst 0x%04x aggfifo1_space %d txfifo_cnt %d\n",
			__FUNCTION__, txs->sequence, (s4 >> 16) & 0xffff, /* s4 is overloaded */
			get_aggfifo_space(ampdu_tx, 1),
			R_REG(wlc->osh, &wlc->regs->u.d11regs.xmtfifo_frame_cnt)));
	}
#endif /* WLAMPDU_HW */
	if (AMPDU_HOST_ENAB(wlc->pub)) {
		if (txs->status.was_acked) {
			/*
			 * Underflow status is reused  for BTCX to indicate AMPDU preemption.
			 * This prevents un-necessary rate fallback.
			 */
			if (TX_STATUS_SUPR_UF == supr_status) {
				WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: BT preemption, skip rate "
					     "fallback\n", wlc->pub->unit));
				update_rate = FALSE;
			}

			ASSERT(txs->status.is_intermediate);
			start_seq = txs->sequence >> SEQNUM_SHIFT;
			bitmap[0] = (txs->status.raw_bits & TX_STATUS_BA_BMAP03_MASK) >>
				TX_STATUS_BA_BMAP03_SHIFT;

			ASSERT(!(s1 & TX_STATUS_INTERMEDIATE));
			ASSERT(s1 & TX_STATUS_AMPDU);

			bitmap[0] |= (s1 & TX_STATUS_BA_BMAP47_MASK) << TX_STATUS_BA_BMAP47_SHIFT;
			bitmap[1] = (s1 >> 8) & 0xff;
			bitmap[2] = (s1 >> 16) & 0xff;
			bitmap[3] = (s1 >> 24) & 0xff;

			/* remaining 4 bytes in s2 */
			bitmap[4] = s2 & 0xff;
			bitmap[5] = (s2 >> 8) & 0xff;
			bitmap[6] = (s2 >> 16) & 0xff;
			bitmap[7] = (s2 >> 24) & 0xff;

			ba_recd = TRUE;

#if defined(STA) && defined(DBG_BCN_LOSS)
			scb->dbg_bcn.last_tx = wlc->pub->now;
#endif /* defined(STA) && defined(DBG_BCN_LOSS) */

			WL_AMPDU_TX(("%s: Block ack received: start_seq is 0x%x bitmap "
				"%02x %02x %02x %02x %02x %02x %02x %02x\n",
				__FUNCTION__, start_seq, bitmap[0], bitmap[1], bitmap[2],
				bitmap[3], bitmap[4], bitmap[5], bitmap[6], bitmap[7]));

		}
	}
#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */
	else {
		/* AMPDU_HW txstatus package */

		ASSERT(txs->status.is_intermediate);
#ifdef BCMDBG
		if (WL_AMPDU_HWTXS_ON()) {
			wlc_print_ampdu_txstatus(ampdu_tx, txs, s1, s2, s3, s4);
		}
#endif /* BCMDBG */

		ASSERT(!(s1 & TX_STATUS_INTERMEDIATE));
		start_seq = ini->start_seq;
		ncons = txs->sequence;
		bitmap[0] = (txs->status.raw_bits & TX_STATUS_BA_BMAP03_MASK) >>
			TX_STATUS_BA_BMAP03_SHIFT;
		bitmap[0] |= (s1 & TX_STATUS_BA_BMAP47_MASK) << TX_STATUS_BA_BMAP47_SHIFT;
		bitmap[1] = (s1 >> 8) & 0xff;
		bitmap[2] = (s1 >> 16) & 0xff;
		bitmap[3] = (s1 >> 24) & 0xff;
		ba_recd = bitmap[0] || bitmap[1] || bitmap[2] || bitmap[3];
#ifdef WLAMPDU_HW
		if (AMPDU_HW_ENAB(wlc->pub)) {
			/* 3rd txstatus */
			bitmap[4] = (s3 >> 16) & 0xff;
			bitmap[5] = (s3 >> 24) & 0xff;
			bitmap[6] = s4 & 0xff;
			bitmap[7] = (s4 >> 8) & 0xff;
			ba_recd = ba_recd || bitmap[4] || bitmap[5] || bitmap[6] || bitmap[7];
		}
#endif /* WLAMPDU_HW */
		/* remaining 4 bytes in s2 */
		rs_txs.tx_cnt[0]	= (uint8)(s2 & 0xff);
		rs_txs.txsucc_cnt[0]	= (uint8)((s2 >> 8) & 0xff);
		rs_txs.tx_cnt[1]	= (uint8)((s2 >> 16) & 0xff);
		rs_txs.txsucc_cnt[1]	= (uint8)((s2 >> 24) & 0xff);

		WLCNTINCR(ampdu_tx->cnt->u0.txs);
		WLCNTADD(ampdu_tx->cnt->tx_mrt, rs_txs.tx_cnt[0]);
		WLCNTADD(ampdu_tx->cnt->txsucc_mrt, rs_txs.txsucc_cnt[0]);
		WLCNTADD(ampdu_tx->cnt->tx_fbr, rs_txs.tx_cnt[1]);
		WLCNTADD(ampdu_tx->cnt->txsucc_fbr, rs_txs.txsucc_cnt[1]);
		WLCNTADD(wlc->pub->_cnt->txfrag, rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1]);
		WLCNTADD(wlc->pub->_cnt->txfrmsnt, rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1]);
		WLCNTADD(wlc->pub->_cnt->txretrans, (rs_txs.tx_cnt[0] - rs_txs.txsucc_cnt[0]) +
			(rs_txs.tx_cnt[1] - rs_txs.txsucc_cnt[1]));
#ifdef PROP_TXSTATUS
		if (!(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(p)->wl_hdr_information) &
			WLFC_PKTFLAG_PKTFROMHOST)) {
			WLCNTSCBADD(scb->scb_stats.tx_pkts_fw_total,
				rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1]);
			WLCNTSCBADD(scb->scb_stats.tx_pkts_fw_retries,
				(rs_txs.tx_cnt[0] - rs_txs.txsucc_cnt[0]) +
				(rs_txs.tx_cnt[1] - rs_txs.txsucc_cnt[1]));
		} else
#endif	/* PROP_TXSTATUS */
		{
			WLCNTSCBADD(scb->scb_stats.tx_pkts_total,
				rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1]);
			WLCNTSCBADD(scb->scb_stats.tx_pkts_retries,
				(rs_txs.tx_cnt[0] - rs_txs.txsucc_cnt[0]) +
				(rs_txs.tx_cnt[1] - rs_txs.txsucc_cnt[1]));
		}

		WLCNTSCBADD(scb_ampdu->ampdu_scb_stats->tx_pkts_total[ini->tid],
			rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1]);
		WLCIFCNTADD(scb, txfrmsnt, rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1]);
#ifdef WL11K
		wlc_rrm_tscm_upd(scb, tid, OFFSETOF(rrm_tscm_t, msdu_tx),
			(rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1]));
		wlc_rrm_tscm_upd(scb, tid, OFFSETOF(rrm_tscm_t, msdu_fail),
			ncons - (rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1]));
#endif

		if (!(txs->status.was_acked)) {

			WL_PRINT(("Status: "));
			if ((txs->status.was_acked) == 0)
				WL_PRINT(("Not ACK_RCV "));
			WL_PRINT(("\n"));
			WL_PRINT(("Total attempts %d succ %d\n", rs_txs.tx_cnt[0],
				rs_txs.txsucc_cnt[0]));
		}
	}
#endif /* WLAMPDU_MAC */

	if (!ba_recd || AMPDU_MAC_ENAB(wlc->pub)) {
		if (!ba_recd) {
			WLCNTINCR(ampdu_tx->cnt->noba);
			AMPDUSCBCNTINCR(scb_ampdu->cnt.noba);
			if (AMPDU_MAC_ENAB(wlc->pub)) {
				WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: error txstatus 0x%x\n",
					wlc->pub->unit, txs->status.raw_bits));
			}
		}
		if (supr_status != TX_STATUS_SUPR_NONE) {
#ifdef PKTQ_LOG
			WLCNTCONDINCR(prec_cnt, prec_cnt->suppress);
			WLCNTCONDINCR(actq_cnt, actq_cnt->suppress);
#endif /* PKTQ_LOG */

			update_rate = FALSE;
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMPDU)
			ampdu_tx->amdbg->supr_reason[supr_status]++;
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMPDU) */
			WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: supr_status 0x%x\n",
				wlc->pub->unit, supr_status));
			/* no need to retry for badch; will fail again */
			if (supr_status == TX_STATUS_SUPR_BADCH) {
				retry = FALSE;
				WLCNTINCR(wlc->pub->_cnt->txchanrej);
			} else if (supr_status == TX_STATUS_SUPR_EXPTIME) {

				WLCNTINCR(wlc->pub->_cnt->txexptime);
#ifdef WL11K
				wlc_rrm_tscm_upd(scb, tid,
					OFFSETOF(rrm_tscm_t, msdu_exp), 1);
#endif
				/* Interference detected */
				if (wlc->rfaware_lifetime)
					wlc_exptime_start(wlc);
			/* TX underflow : try tuning pre-loading or ampdu size */
			} else if (supr_status == TX_STATUS_SUPR_FRAG) {
				/* if there were underflows, but pre-loading is not active,
				   notify rate adaptation.
				*/
#ifdef AMPDU_NON_AQM
				if (wlc_ffpld_check_txfunfl(wlc, prio2fifo[tid], bsscfg) > 0) {
				}
#endif /* AMPDU_NON_AQM */
			}
		} else if (txs->phyerr) {
			update_rate = FALSE;
			WLCNTINCR(wlc->pub->_cnt->txphyerr);
			WL_ERROR(("wl%d: %s: tx phy error (0x%x)\n",
				wlc->pub->unit, __FUNCTION__, txs->phyerr));

#ifdef BCMDBG
			if (WL_ERROR_ON()) {
				prpkt("txpkt (AMPDU)", wlc->osh, p);
				if (AMPDU_HOST_ENAB(wlc->pub)) {
					wlc_print_txdesc(wlc, (wlc_txd_t*)PKTDATA(wlc->osh, p));
					wlc_print_txstatus(wlc, txs);
				}
#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */
				else {
					wlc_print_ampdu_txstatus(ampdu_tx, txs, s1, s2, s3, s4);
					wlc_dump_aggfifo(wlc, NULL);
				}
#endif /* WLAMPDU_MAC */
			}
#endif /* BCMDBG */
		}
	}

	/* Check if interference is still there */
	if (wlc->rfaware_lifetime && wlc->exptime_cnt && (supr_status != TX_STATUS_SUPR_EXPTIME))
		wlc_exptime_check_end(wlc);

#if defined(WLPKTDLYSTAT) || defined(WL11K)
	/* Get the current time */
	now = WLC_GET_CURR_TIME(wlc);
#endif /* WLPKTDLYSTAT */

	/* loop through all pkts and retry if not acked */
	while (p) {
		if ((WLPKTTAG(p)->flags & WLF_AMPDU_MPDU) == 0) {
			WL_PRINT(("%s flags=%x seq=%d\n", __FUNCTION__,
				WLPKTTAG(p)->flags, WLPKTTAG(p)->seq));
		}

		ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

#if defined(TXQ_MUX)
		tx_time += WLPKTTIME(p);
#endif /* TXQ_MUX */

		txh = (d11txh_t *)PKTDATA(wlc->osh, p);
		mcl = ltoh16(txh->MacTxControlLow);
		plcp = (uint8 *)(txh + 1);
		h = (struct dot11_header*)(plcp + D11_PHY_HDR_LEN);
		seq = ltoh16(h->seq) >> SEQNUM_SHIFT;
		h->fc &= (~htol16(FC_RETRY));
		pkt_freed = FALSE;
#ifdef PROP_TXSTATUS
		wlfc_suppr_acked_status = 0;
#endif /* PROP_TXSTATUS */
#ifdef PKTQ_LOG
		pktlen = pkttotlen(wlc->osh, p) - sizeof(d11txh_t);
		prec_pktlen = pktlen - DOT11_LLC_SNAP_HDR_LEN - DOT11_MAC_HDR_LEN - DOT11_QOS_LEN;
		tot_len += pktlen;
#endif /* PKTQ_LOG */
		if (tot_mpdu == 0) {
			mcs = plcp[0] & MIMO_PLCP_MCS_MASK;
			mimoantsel = ltoh16(txh->ABI_MimoAntSel);
			is_sgi = PLCP3_ISSGI(plcp[3]);
			is40 = (plcp[0] & MIMO_PLCP_40MHZ) ? 1 : 0;
			BCM_REFERENCE(is40);

			if (AMPDU_HOST_ENAB(wlc->pub)) {
				/* this is only needed for host agg */
#if defined(WLPKTDLYSTAT) || defined(PKTQ_LOG)
				BCM_REFERENCE(mcs_fbr);
				mcs_fbr = txh->FragPLCPFallback[0] & MIMO_PLCP_MCS_MASK;
#endif /* WLPKTDLYSTAT || PKTQ_LOG */
			}
#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */
			else {
				first_frameid = ltoh16(txh->TxFrameID);
#if defined(BCMDBG) || defined(BCMDBG_AMPDU)
				if (PLCP3_ISSGI(plcp[3])) {
					WLCNTADD(ampdu_tx->cnt->u1.txmpdu_sgi, rs_txs.tx_cnt[0]);
					if (ampdu_tx->amdbg)
						WLCNTADD(ampdu_tx->amdbg->txmcssgi[MCS2IDX(mcs)],
							rs_txs.tx_cnt[0]);
				}
				if (PLCP3_ISSTBC(plcp[3])) {
					WLCNTADD(ampdu_tx->cnt->u2.txmpdu_stbc, rs_txs.tx_cnt[0]);
					if (ampdu_tx->amdbg)
						WLCNTADD(ampdu_tx->amdbg->txmcsstbc[MCS2IDX(mcs)],
							rs_txs.tx_cnt[0]);
				}
				if (ampdu_tx->amdbg) {
					WLCNTADD(ampdu_tx->amdbg->txmcs[MCS2IDX(mcs)],
						rs_txs.tx_cnt[0]);
					WLCNTADD(ampdu_tx->amdbg->txmcs_succ[MCS2IDX(mcs)],
						rs_txs.txsucc_cnt[0]);
				}
#ifdef WLPKTDLYSTAT
				WLCNTADD(ampdu_tx->cnt->txmpdu_cnt[MCS2IDX(mcs)], rs_txs.tx_cnt[0]);
				WLCNTADD(ampdu_tx->cnt->txmpdu_succ_cnt[MCS2IDX(mcs)],
					rs_txs.txsucc_cnt[0]);
#endif /* WLPKTDLYSTAT */
				if ((ltoh16(txh->XtraFrameTypes) & 0x3) != 0) {
#ifdef WLCNT
					uint8 mcs_fb;
					/* if not fbr_cck */
					mcs_fb = txh->FragPLCPFallback[0] & MIMO_PLCP_MCS_MASK;
#endif /* WLCNT */
					if (ampdu_tx->amdbg)
						WLCNTADD(ampdu_tx->amdbg->txmcs[MCS2IDX(mcs_fb)],
							rs_txs.tx_cnt[1]);
#ifdef WLPKTDLYSTAT
					WLCNTADD(ampdu_tx->cnt->txmpdu_cnt[MCS2IDX(mcs_fb)],
						rs_txs.tx_cnt[1]);
					WLCNTADD(ampdu_tx->cnt->txmpdu_succ_cnt[MCS2IDX(mcs_fb)],
						rs_txs.txsucc_cnt[1]);
#endif /* WLPKTDLYSTAT */
					if (PLCP3_ISSGI(txh->FragPLCPFallback[3])) {
						WLCNTADD(ampdu_tx->cnt->u1.txmpdu_sgi,
							rs_txs.tx_cnt[1]);
						if (ampdu_tx->amdbg)
							WLCNTADD(ampdu_tx->amdbg->txmcssgi
								[MCS2IDX(mcs_fb)],
								rs_txs.tx_cnt[1]);
					}
					if (PLCP3_ISSTBC(txh->FragPLCPFallback[3])) {
						WLCNTADD(ampdu_tx->cnt->u2.txmpdu_stbc,
							rs_txs.tx_cnt[1]);
						if (ampdu_tx->amdbg)
							WLCNTADD(ampdu_tx->amdbg->txmcsstbc[
							       MCS2IDX(mcs_fb)], rs_txs.tx_cnt[1]);
					}
				}
#endif 
			}
#endif /* WLAMPDU_MAC */
		}

		indx = TX_SEQ_TO_INDEX(seq);
		if (AMPDU_HOST_ENAB(wlc->pub)) {
			txretry = ini->txretry[indx];
		}

		if (MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) >= ini->ba_wsize) {
			WL_NONE(("wl%d: %s: unexpected completion: seq 0x%x, "
				"start seq 0x%x\n",
				wlc->pub->unit, __FUNCTION__, seq, ini->start_seq));
			pkt_freed = TRUE;
#ifdef BCMDBG
#ifdef WLAMPDU_MAC
			wlc_print_ampdu_txstatus(ampdu_tx, txs, s1, s2, s3, s4);
#endif
#endif /* BCMDBG */
			goto nextp;
		}

		if (!isset(ini->ackpending, indx)) {
			WL_AMPDU_ERR(("wl%d: %s: seq 0x%x\n",
				wlc->pub->unit, __FUNCTION__, seq));
			ampdu_dump_ini(ini);
			ASSERT(isset(ini->ackpending, indx));
		}

		ack_recd = FALSE;
		if (ba_recd || AMPDU_MAC_ENAB(wlc->pub)) {
			if (AMPDU_MAC_ENAB(wlc->pub))
				/* use position bitmap */
				bindex = tot_mpdu;
			else
				bindex = MODSUB_POW2(seq, start_seq, SEQNUM_MAX);

			WL_AMPDU_TX(("%s: tid %d seq is 0x%x, start_seq is 0x%x, "
			          "bindex is %d set %d, txretry %d, index %d\n",
			          __FUNCTION__, tid, seq, start_seq, bindex,
			          isset(bitmap, bindex), txretry, indx));

			/* if acked then clear bit and free packet */
			if ((bindex < AMPDU_BA_BITS_IN_BITMAP) && isset(bitmap, bindex)) {
				if (isset(ini->ackpending, indx)) {
					clrbit(ini->ackpending, indx);
					if (isset(ini->barpending, indx)) {
						clrbit(ini->barpending, indx);
					}
					ini->txretry[indx] = 0;
				}
#if defined(BCMCCX) && defined(CCX_SDK)
				if (IHV_ENAB(wlc->ccx, bsscfg) &&
				    BSSCFG_STA(bsscfg) && bsscfg->BSS &&
					wlc->ccx->frame_log)
					wlc_ccx_log_tx_frame(wlc->ccx, (uint8*)h,
						PKTLEN(wlc->osh, p) - sizeof(d11txh_t) -
						D11_PHY_HDR_LEN, TX_STATUS_ACK_RCV, FALSE, TRUE);
				/* call any matching pkt callbacks */
				if (WLPKTTAG(p)->flags & WLF_IHV_TX_PKT) {
					/* additional information for IHV pkt callback */
					wlc->ccx->ihv_txpkt = p;
					wlc->ccx->ihv_txpkt_sent = TRUE;
					wlc->ccx->ihv_txpkt_max_retries = FALSE;
				}
#endif /* BCMCCX && CCX_SDK */

#ifdef PROP_TXSTATUS
				if (PROP_TXSTATUS_ENAB(wlc->pub) && (!AMPDU_AQM_ENAB(wlc->pub))) {
					if (wlc_wlfc_suppr_status_query(wlc, scb)) {
						wlc_pkttag_t *pkttag = WLPKTTAG(p);
						uint32 wlhinfo = pkttag->wl_hdr_information;
						uint32 flags = WL_TXSTATUS_GET_FLAGS(wlhinfo);
						if ((flags & WLFC_PKTFLAG_PKTFROMHOST) &&
							(pkttag->flags & WLF_PROPTX_PROCESSED)) {
							wlfc_suppr_acked_status =
								WLFC_CTL_PKTFLAG_SUPPRESS_ACKED;
						}
					}
				}
#endif /* PROP_TXSTATUS */
				wlc_pcb_fn_invoke(wlc->pcb, p, TX_STATUS_ACK_RCV);

#ifdef WLTXMONITOR
				if (MONITOR_ENAB(wlc) || PROMISC_ENAB(wlc->pub))
					wlc_tx_monitor(wlc, txh, txs, p, NULL);
#endif /* WLTXMONITOR */
				pkt_freed = TRUE;
				ack_recd = TRUE;
#ifdef PKTQ_LOG
				WLCNTCONDINCR(prec_cnt, prec_cnt->acked);
				WLCNTCONDINCR(actq_cnt, actq_cnt->acked);
				SCB_BS_DATA_CONDINCR(bs_data_counters, acked);
				WLCNTCONDADD(prec_cnt, prec_cnt->throughput, prec_pktlen);
				WLCNTCONDADD(actq_cnt, actq_cnt->throughput, prec_pktlen);
				SCB_BS_DATA_CONDADD(bs_data_counters, throughput, prec_pktlen);
				WLCNTSCBADD(scb_ampdu->ampdu_scb_stats->tx_bytes_total[ini->tid],
					prec_pktlen);
#endif /* PKTQ_LOG */
				suc_mpdu++;
			}
		}
		/* either retransmit or send bar if ack not recd */
		if (!ack_recd) {
			bool free_pkt = FALSE;
			bool relist = FALSE;

#if defined(PSPRETEND)
			if (BSSCFG_AP(bsscfg) && PS_PRETEND_ENABLED(bsscfg)) {
				relist = wlc_pspretend_pkt_relist(wlc->pps_info, bsscfg, scb,
				                                  retry_limit, txretry);
			}
#endif /* PSPRETEND */

#if defined(PROP_TXSTATUS) && defined(WLP2P_UCODE)
			/* Durring suppress state or suppressed packet
			 * return retry and suppressed packets to host
			 * as suppresed, don't resend to Ucode.
			 */
			if (AMPDU_HOST_ENAB(wlc->pub) && PROP_TXSTATUS_ENAB(wlc->pub)) {
				if ((wlc_wlfc_suppr_status_query(wlc, scb) ||
					!TXS_SUPR_MAGG_DONE(txs->status.suppr_ind)) &&
					!wlc_should_retry_suppressed_pkt(wlc, p, supr_status)) {
					/* change retry to suppres status */
					if (TXS_SUPR_MAGG_DONE(txs->status.suppr_ind)) {
						supr_status = TX_STATUS_SUPR_NACK_ABS;
						txs->status.suppr_ind = TX_STATUS_SUPR_NACK_ABS;
					}
					retry = FALSE;
				}
			}
#endif /* PROP_TXSTATUS && WLP2P_UCODE */

			if (AMPDU_HOST_ENAB(wlc->pub) &&
			    retry) {

				if (AP_ENAB(wlc->pub) &&
				    supr_status == TX_STATUS_SUPR_PMQ) {
					/* last_frag TRUE as fragmentation not allowed for AMPDU */
					free_pkt = wlc_apps_suppr_frame_enq(wlc, p, txs, TRUE);
				} else if (P2P_ABS_SUPR(wlc, supr_status) &&
				         BSS_TX_SUPR_ENAB(bsscfg)) {
					/* This is possible if we got a packet suppression
					 * before getting ABS interrupt
					 */
					if (!BSS_TX_SUPR(bsscfg)) {
						wlc_bsscfg_tx_stop(wlc->psqi, bsscfg);
					}

					if (BSSCFG_AP(bsscfg) &&
					    SCB_ASSOCIATED(scb) && SCB_P2P(scb))
						wlc_apps_scb_tx_block(wlc, scb, 1, TRUE);
					/* With Proptxstatus enabled in dongle and host,
					 * pkts sent by host will not come here as retry is set
					 * to FALSE by wlc_should_retry_suppressed_pkt().
					 * With Proptxstatus disabled in dongle or not active
					 * in host, all the packets will come here and
					 * need to be re-enqueued.
					 */
					free_pkt = wlc_pkt_abs_supr_enq(wlc, scb, p);
				} else if ((txretry < (int)retry_limit) || relist) {
					/* Set retry bit */
					h->fc |= htol16(FC_RETRY);

					ini->txretry[indx]++;
					ASSERT(ini->retry_cnt < ampdu_tx_cfg->ba_max_tx_wsize);
					ASSERT(ini->retry_seq[ini->retry_tail] == 0);
					ini->retry_seq[ini->retry_tail] = seq;
					ini->retry_tail = NEXT_TX_INDEX(ini->retry_tail);
					ini->retry_cnt++;

#ifdef PKTQ_LOG
					WLCNTCONDINCR(prec_cnt, prec_cnt->retry);
					WLCNTCONDINCR(actq_cnt, actq_cnt->retry);
					SCB_BS_DATA_CONDINCR(bs_data_counters, retry);
#endif /* PKTQ_LOG */

#if defined(PSPRETEND)
					if (relist) {

						/* not using pmq for ps pretend, so indicate
						 * no block flag
						 */
						if (!SCB_PS(scb)) {
							wlc_pspretend_on(wlc->pps_info, scb,
							                 PS_PRETEND_NO_BLOCK);
						}
#ifdef PKTQ_LOG
						WLCNTCONDINCR(prec_cnt, prec_cnt->ps_retry);
						WLCNTCONDINCR(actq_cnt, actq_cnt->ps_retry);
#endif /* PKTQ_LOG */
#ifdef WLINTFERSTAT
						wlc_trf_mgmt_scb_txfail_detect(wlc->trf_mgmt_ctxt,
						                               scb);
#endif  /* WLINTFERSTAT */
					}
#endif /* PSPRETEND */
					SCB_TX_NEXT(TXMOD_AMPDU, scb, p, WLC_PRIO_TO_HI_PREC(tid));
				} else {
					WL_NONE(("%s: # retries exceeds limit\n", __FUNCTION__));
					free_pkt = TRUE;
				}

				if (free_pkt)
					goto not_retry;

				requeue++;
			} else {
			not_retry:
#ifdef PROP_TXSTATUS
			/* Host suppressed packets */
			if (PROP_TXSTATUS_ENAB(wlc->pub) &&
				WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wlfc)) &&
				!TXS_SUPR_MAGG_DONE(supr_status) &&
				(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(p)->wl_hdr_information) &
					WLFC_PKTFLAG_PKTFROMHOST)) {
				ini->txretry[indx] = 0;
				ini->tx_in_transit--;
				ini->suppr_window++;
			} else
#endif /* PROP_TXSTATUS */
			{
				setbit(ini->barpending, indx);
				send_bar = TRUE;
			}

#ifdef WLMEDIA_TXFAILEVENT
					wlc_tx_failed_event(wlc, scb, bsscfg, txs, p, 0);
#endif /* WLMEDIA_TXFAILEVENT */

#ifdef WLTXMONITOR
				if (MONITOR_ENAB(wlc) || PROMISC_ENAB(wlc->pub))
					wlc_tx_monitor(wlc, txh, txs, p, NULL);
#endif /* WLTXMONITOR */
#ifdef PKTQ_LOG
				WLCNTCONDINCR(prec_cnt, prec_cnt->retry_drop);
				WLCNTCONDINCR(actq_cnt, actq_cnt->retry_drop);
				SCB_BS_DATA_CONDINCR(bs_data_counters, retry_drop);
#endif /* PKTQ_LOG */
#ifdef WLINTFERSTAT
				/* Count only failed packets and indicate failure if required */
				wlc_trf_mgmt_scb_txfail_detect(wlc->trf_mgmt_ctxt, scb);
#endif /* WLINTFERSTAT */
				WLCNTINCR(wlc->pub->_cnt->txfail);
#ifdef PROP_TXSTATUS
				if (!(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(p)->wl_hdr_information) &
					WLFC_PKTFLAG_PKTFROMHOST)) {
					WLCNTSCBINCR(scb->scb_stats.tx_pkts_fw_retry_exhausted);
				} else
#endif	/* PROP_TXSTATUS */
				{
					WLCNTSCBINCR(scb->scb_stats.tx_pkts_retry_exhausted);
				}
				WLCNTSCBINCR(scb->scb_stats.tx_failures);
				WLCIFCNTINCR(scb, txfail);
				pkt_freed = TRUE;
			}
		} else if (AMPDU_MAC_ENAB(wlc->pub)) {
			ba_recd = TRUE;
		}
#if defined(BCMDBG)
		/* tot_mpdu may exceeds AMPDU_MAX_MPDU if doing ucode/hw agg */
		if (AMPDU_HOST_ENAB(wlc->pub) && ba_recd && !ack_recd)
			hole[idx]++;
#endif 

nextp:
#if defined(BCMDBG)
		if ((idx = tot_mpdu) >= AMPDU_MAX_MPDU) {
			WL_AMPDU_ERR(("%s: idx out-of-bound to array size (%d)\n",
					__FUNCTION__, idx));
			idx = AMPDU_MAX_MPDU - 1;
		}
#endif 
		tot_mpdu++;
#ifdef PROP_TXSTATUS
		if (PROP_TXSTATUS_ENAB(wlc->pub) && ((h->fc & htol16(FC_RETRY)) == 0)) {
			uint8 wlfc_status = wlc_txstatus_interpret(&txs->status, ack_recd);
			if (!ack_recd && scb &&
			    WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub) &&
			    (wlfc_status == WLFC_CTL_PKTFLAG_D11SUPPRESS)) {
				wlc_suppress_sync_fsm(wlc, scb, p, TRUE);
			}
			if (wlfc_suppr_acked_status)
				wlfc_status = wlfc_suppr_acked_status;
			wlfc_process_txstatus(wlc->wlfc, wlfc_status, p, txs, FALSE);
		}
#endif

#ifdef WL_TX_STALL
		if (!ack_recd) {
			if (supr_status != TX_STATUS_SUPR_NONE) {
				WL_TX_STS_UPDATE(tx_status,
					wlc_tx_status_map_hw_to_sw_supr_code(wlc, supr_status));
			} else if (txs->phyerr) {
				WL_TX_STS_UPDATE(tx_status, WLC_TX_STS_PHY_ERROR);
			} else {
				WL_TX_STS_UPDATE(tx_status, WLC_TX_STS_RETRY_TIMEOUT);
			}

			wlc_tx_status_update_counters(wlc, p, scb, bsscfg, tx_status, 1);

			/* Clear status, next packet will have its own status */
			WL_TX_STS_UPDATE(tx_status, WLC_TX_STS_SUCCESS);
		}
#endif /* WL_TX_STALL */
		/* calculate latency and packet loss statistics */
		if (pkt_freed) {
#if defined(WLPKTDLYSTAT) || defined(WL11K)
			/* Ignore wrap-around case and error case */
			if (now > WLPKTTAG(p)->shared.enqtime) {
				delay = now - WLPKTTAG(p)->shared.enqtime;
#ifdef WLPKTDLYSTAT
				if (AMPDU_HOST_ENAB(ampdu_tx->wlc->pub))
					tr = (txretry >= AMPDU_DEF_RETRY_LIMIT) ?
						AMPDU_DEF_RETRY_LIMIT : txretry;
				else
					tr = 0;

				wlc_delay_stats_upd(delay_stats, delay, tr, ack_recd);
				WL_AMPDU_STAT(("Enq %d retry %d cnt %d: acked %d delay/min/"
					"max/sum %d %d %d %d\n", WLPKTTAG(p)->shared.enqtime,
					tr, delay_stats->txmpdu_cnt[tr], ack_recd, delay,
					delay_stats->delay_min, delay_stats->delay_max,
					delay_stats->delay_sum[tr]));
#endif /* WLPKTDLYSTAT */
#ifdef WL11K
				if (ack_recd) {
					wlc_rrm_delay_upd(scb, tid, delay);
				}
#endif
			}
#endif /* WLPKTDLYSTAT || WL11K */
			PKTFREE(wlc->osh, p, TRUE);
		} else {
			/* Packet requeued, update the counters */
			wlc_tx_status_update_counters(wlc, p,
				scb, bsscfg, WLC_TX_STS_QUEUED, 1);
		}
		/* break out if last packet of ampdu */
		if (AMPDU_MAC_ENAB(wlc->pub)) {
#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */
			if (tot_mpdu == ncons) {
				break;
			}
			if (++pdu_count >= ampdu_tx_cfg->ba_max_tx_wsize) {
				WL_AMPDU_ERR(("%s: Reach max num of MPDU without finding frameid\n",
					__FUNCTION__));
				ASSERT(pdu_count >= ampdu_tx_cfg->ba_max_tx_wsize);
			}
#endif /* WLAMPDU_MAC */
		} else {
			if (((mcl & TXC_AMPDU_MASK) >> TXC_AMPDU_SHIFT) == TXC_AMPDU_LAST)
				break;
		}

		p = GETNEXTTXP(wlc, queue);
		/* For external use big hammer to restore the sync */
		if (p == NULL) {
#ifdef WLAMPDU_MAC
			WL_AMPDU_ERR(("%s: p is NULL. tot_mpdu %d suc %d pdu %d first_frameid %x"
				"fifocnt %d\n", __FUNCTION__, tot_mpdu, suc_mpdu, pdu_count,
				first_frameid,
				R_REG(wlc->osh, &wlc->regs->u.d11regs.xmtfifo_frame_cnt)));
#endif /* WLAMPDU_MAC */

#ifdef BCMDBG
#ifdef WLAMPDU_MAC
			wlc_print_ampdu_txstatus(ampdu_tx, txs, s1, s2, s3, s4);
			wlc_dump_aggfifo(wlc, NULL);
#endif /* WLAMPDU_MAC */
#endif	/* BCMDBG */
			ASSERT(p);
			break;
		}

		WLPKTTAGSCBSET(p, scb);

	} /* while(p) */

	if (AMPDU_HOST_ENAB(wlc->pub)) {
		WLCNTADD(wlc->pub->_cnt->txfrag, suc_mpdu);
		WLCNTADD(wlc->pub->_cnt->txfrmsnt, suc_mpdu);
		WLCNTSCBADD(scb->scb_stats.tx_pkts_total, suc_mpdu);
	}

#if defined(BCMDBG) || defined(BCMDBG_AMPDU)
	/* post-process the statistics */
	if (AMPDU_HOST_ENAB(wlc->pub)) {
		int i, j;
		ampdu_tx->amdbg->mpdu_histogram[idx]++;
		for (i = 0; i < idx; i++) {
			if (hole[i])
				ampdu_tx->amdbg->retry_histogram[i]++;
			if (hole[i]) {
				for (j = i + 1; (j < idx) && hole[j]; j++)
					;
				if (j == idx)
					ampdu_tx->amdbg->end_histogram[i]++;
			}
		}

#ifdef WLPKTDLYSTAT
		if (txretry < rr_retry_limit)
			WLCNTADD(ampdu_tx->cnt->txmpdu_succ_cnt[MCS2IDX(mcs)], suc_mpdu);
		else
			WLCNTADD(ampdu_tx->cnt->txmpdu_succ_cnt[MCS2IDX(mcs_fbr)], suc_mpdu);
#endif /* WLPKTDLYSTAT */
	}
#endif 

#ifdef PKTQ_LOG
	ASSERT(txh && plcp);
	bw = (txh->PhyTxControlWord_1 & PHY_TXC1_BW_MASK) >> 1;

	/* Note HOST AMPDU path is HT only, no 10MHz support */
	ASSERT((bw >= BW_20MHZ) && (bw <= BW_40MHZ));

	if (txretry < rr_retry_limit) {
		/* Primary rate computation */
		rspec = MCS_TO_RSPEC(mcs, bw, plcp[3]);
		current_rate = wlc_rate_rspec2rate(rspec);
	} else {
		/* Fall-back rate computation */
		rspec = MCS_TO_RSPEC(mcs_fbr, bw, plcp[3]);
		current_rate = wlc_rate_rspec2rate(rspec);
	}

	if (suc_mpdu) {
		txrate_succ = BPS_TO_500K(current_rate) * suc_mpdu;
		WLCNTCONDADD(prec_cnt, prec_cnt->txrate_succ, txrate_succ);
		WLCNTCONDADD(actq_cnt, actq_cnt->txrate_succ, txrate_succ);
	}

	/* Air Time Computation */

	flg = WLC_AIRTIME_AMPDU;

	if (WLC_HT_GET_AMPDU_RTS(wlc->hti)) {
		flg |= (WLC_AIRTIME_RTSCTS);
	}
	ctl_rspec = AMPDU_BASIC_RATE(wlc->band, rspec);

	/* Short preamble */
	if (WLC_PROT_CFG_SHORTPREAMBLE(wlc->prot, bsscfg) &&
			(scb->flags & SCB_SHORTPREAMBLE) && RSPEC_ISCCK(ctl_rspec)) {
		ctl_rspec |= RSPEC_SHORT_PREAMBLE;
	}

	rts_sifs = airtime_rts_usec(flg, ctl_rspec);
	cts_sifs = airtime_cts_usec(flg, ctl_rspec);
	ba_sifs = airtime_ba_usec(flg, ctl_rspec);
	ampdu_dur = wlc_airtime_packet_time_us(flg, rspec, tot_len);

	if (ba_recd) {
		/* If BA received */
		airtime = (txs->status.rts_tx_cnt + 1)* rts_sifs +
			cts_sifs + ampdu_dur + ba_sifs;
	} else {
		/* No Block Ack received
		 * 1. AMPDU transmitted but block ack not rcvd (RTS enabled/RTS disabled).
		 * 2. AMPDU not transmitted at all (RTS retrie timeout)
		 */
		if (txs->status.frag_tx_cnt) {
			airtime = (txs->status.rts_tx_cnt + 1)* rts_sifs +
				cts_sifs + ampdu_dur;
		} else {
			airtime = (txs->status.rts_tx_cnt + 1)* rts_sifs;
		}
	}
	WLCNTCONDADD(actq_cnt, actq_cnt->airtime, airtime);
#endif /* PKTQ_LOG */
	/* send bar to move the window */
	if (send_bar)
		wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);

	/* update rate state */
	if (WLANTSEL_ENAB(wlc))
		antselid = wlc_antsel_antsel2id(wlc->asi, mimoantsel);

	rs_txs.txrts_cnt = txs->status.rts_tx_cnt;
	rs_txs.rxcts_cnt = txs->status.cts_rx_cnt;
#ifdef WLAMPDU_MAC
	rs_txs.ncons = ncons;
	rs_txs.nlost = tot_mpdu - suc_mpdu;
#endif
	rs_txs.ack_map1 = *(uint32 *)&bitmap[0];
	rs_txs.ack_map2 = *(uint32 *)&bitmap[4];
	/* init now to txs->frameid for host agg; for hw agg it is
	 * first_frameid.
	 */
	rs_txs.frameid = txs->frameid;
	rs_txs.antselid = antselid;
#ifdef WLAMPDU_MAC
	WL_AMPDU_HWDBG(("%s: consume %d txfifo_cnt %d\n", __FUNCTION__, tot_mpdu,
		R_REG(wlc->osh, &wlc->regs->u.d11regs.xmtfifo_frame_cnt)));

	if (AMPDU_MAC_ENAB(wlc->pub)) {
		/* primary rspec */
		rs_txs.txrspec[0] = HT_RSPEC(mcs);
		/* use primary rate's sgi & bw info */
		rs_txs.txrspec[0] |= is_sgi ? RSPEC_SHORT_GI : 0;
		rs_txs.txrspec[0] |= (is40 ? RSPEC_BW_40MHZ : RSPEC_BW_20MHZ);
		rs_txs.frameid = first_frameid;

		if (rs_txs.tx_cnt[0] + rs_txs.tx_cnt[1] == 0) {
			/* must have failed because of rts failure */
			ASSERT(rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1] == 0);
#ifdef BCMDBG
			if (rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1] != 0) {
				WL_AMPDU_ERR(("%s: wrong condition\n", __FUNCTION__));
				wlc_print_ampdu_txstatus(ampdu_tx, txs, s1, s2, s3, s4);
			}
			if (WL_AMPDU_ERR_ON()) {
				wlc_print_ampdu_txstatus(ampdu_tx, txs, s1, s2, s3, s4);
			}
#endif /* BCMDBG */
			/* fix up tx_cnt */
			rs_txs.tx_cnt[0] = ncons;
		}

		if (update_rate)
			wlc_scb_ratesel_upd_txs_ampdu(wlc->wrsi, scb, &rs_txs, txs, 0);
	} else
#endif /* WLAMPDU_MAC */
	if (update_rate) {
		wlc_scb_ratesel_upd_txs_blockack(wlc->wrsi, scb,
			txs, suc_mpdu, tot_mpdu, !ba_recd, (uint8)txretry,
			rr_retry_limit, FALSE, mcs & MIMO_PLCP_MCS_MASK, is_sgi, antselid, ac);
	}

	WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: requeueing %d packets, consumed %d\n",
		wlc->pub->unit, requeue, tot_mpdu));

	/* Call only once per ampdu_tx for SW agg */
	if (AMPDU_MAC_ENAB(wlc->pub)) {
#ifdef WLAMPDU_MAC
		WLCNTADD(ampdu_tx->cnt->pending, -tot_mpdu);
		WLCNTADD(ampdu_tx->cnt->cons, tot_mpdu);
		WLC_TXFIFO_COMPLETE(wlc, queue, tot_mpdu, tx_time);
#endif /* WLAMPDU_MAC */
	} else {
#if defined(TXQ_MUX)
		WLC_TXFIFO_COMPLETE(wlc, queue, tot_mpdu, tx_time);
#else
		WLC_TXFIFO_COMPLETE(wlc, queue, ampdu_tx_cfg->txpkt_weight, tx_time);
#endif /* TXQ_MUX */
	}

#ifdef PSPRETEND
	if ((BSSCFG_AP(bsscfg) || BSSCFG_IBSS(bsscfg)) &&
	    PS_PRETEND_ENABLED(bsscfg)) {
		wlc_pspretend_dotxstatus(wlc->pps_info, bsscfg, scb, ba_recd);
	}
#endif /* PSPRETEND */
	/* bump up the start seq to move the window */
	wlc_ampdu_ini_move_window(ampdu_tx, scb_ampdu, ini);


#ifdef STA
	/* PM state change */
	wlc_update_pmstate(bsscfg, (uint)(wlc_txs_alias_to_old_fmt(wlc, &(txs->status))));
#endif /* STA */
} /* wlc_ampdu_dotxstatus_complete */

#if defined(WLAMPDU_MAC) && defined(BCMDBG)

static void
wlc_print_ampdu_txstatus(ampdu_tx_info_t *ampdu_tx,
	tx_status_t *pkg1, uint32 s1, uint32 s2, uint32 s3, uint32 s4)
{
	uint16 *p = (uint16 *)pkg1;
	printf("%s: txstatus 0x%04x\n", __FUNCTION__, pkg1->status.raw_bits);
	if (AMPDU_MAC_ENAB(ampdu_tx->wlc->pub) && !AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		uint16 last_frameid, txstat;
		uint8 bitmap[8];
		uint16 txcnt_mrt, succ_mrt, txcnt_fbr, succ_fbr;
		uint16 ncons = pkg1->sequence;

		last_frameid = p[2]; txstat = p[3];

		bitmap[0] = (txstat & TX_STATUS_BA_BMAP03_MASK) >> TX_STATUS_BA_BMAP03_SHIFT;
		bitmap[0] |= (s1 & TX_STATUS_BA_BMAP47_MASK) << TX_STATUS_BA_BMAP47_SHIFT;
		bitmap[1] = (s1 >> 8) & 0xff;
		bitmap[2] = (s1 >> 16) & 0xff;
		bitmap[3] = (s1 >> 24) & 0xff;
		txcnt_mrt = s2 & 0xff;
		succ_mrt = (s2 >> 8) & 0xff;
		txcnt_fbr = (s2 >> 16) & 0xff;
		succ_fbr = (s2 >> 24) & 0xff;
		printf("\t\t ncons %d last frameid: 0x%x\n", ncons, last_frameid);
		printf("\t\t txcnt: mrt %2d succ %2d fbr %2d succ %2d\n",
		       txcnt_mrt, succ_mrt, txcnt_fbr, succ_fbr);

		if (AMPDU_UCODE_ENAB(ampdu_tx->wlc->pub)) {
			printf("\t\t bitmap: %02x %02x %02x %02x\n",
				bitmap[0], bitmap[1], bitmap[2], bitmap[3]);
		} else {
			bitmap[4] = (s3 >> 16) & 0xff;
			bitmap[5] = (s3 >> 24) & 0xff;
			bitmap[6] = s4 & 0xff;
			bitmap[7] = (s4 >> 8) & 0xff;
			printf("\t\t bitmap: %02x %02x %02x %02x %02x %02x %02x %02x\n",
			       bitmap[0], bitmap[1], bitmap[2], bitmap[3],
			       bitmap[4], bitmap[5], bitmap[6], bitmap[7]);
			printf("\t\t timestamp 0x%04x\n", (s4 >> 16) & 0xffff);
		}
#ifdef WLAMPDU_UCODE
		if (AMPDU_UCODE_ENAB(ampdu_tx->wlc->pub) && txcnt_mrt + txcnt_fbr == 0) {
			uint16 strt, end, wptr, rptr, rnum, qid;
			uint16 base =
				(wlc_read_shm(ampdu_tx->wlc, M_TXFS_PTR(ampdu_tx->wlc)) +
				C_TXFSD_WOFFSET) * 2;
			qid = WLC_TXFID_GET_QUEUE(last_frameid);
			strt = wlc_read_shm(ampdu_tx->wlc, C_TXFSD_STRT_POS(base, qid));
			end = wlc_read_shm(ampdu_tx->wlc, C_TXFSD_END_POS(base, qid));
			wptr = wlc_read_shm(ampdu_tx->wlc, C_TXFSD_WPTR_POS(base, qid));
			rptr = wlc_read_shm(ampdu_tx->wlc, C_TXFSD_RPTR_POS(base, qid));
			rnum = wlc_read_shm(ampdu_tx->wlc, C_TXFSD_RNUM_POS(base, qid));
			printf("%s: side channel %d ---- \n", __FUNCTION__, qid);
			printf("\t\t strt : shm(0x%x) = 0x%x\n", C_TXFSD_STRT_POS(base, qid), strt);
			printf("\t\t end  : shm(0x%x) = 0x%x\n", C_TXFSD_END_POS(base, qid), end);
			printf("\t\t wptr : shm(0x%x) = 0x%x\n", C_TXFSD_WPTR_POS(base, qid), wptr);
			printf("\t\t rptr : shm(0x%x) = 0x%x\n", C_TXFSD_RPTR_POS(base, qid), rptr);
			printf("\t\t rnum : shm(0x%x) = 0x%x\n", C_TXFSD_RNUM_POS(base, qid), rnum);
		}
#endif /* WLAMPDU_UCODE */
	}
}

#endif  /* AMPDU_MAC && BCMDBG */

#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */
int
wlc_dump_aggfifo(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	osl_t *osh = wlc->osh;
	d11regs_t *regs = wlc->regs;
	int ret = BCME_OK;

	BCM_REFERENCE(b);

	if (!wlc->clk) {
		printf("%s: clk off\n", __FUNCTION__);
		return ret;
	}

	printf("%s:\n", __FUNCTION__);

	if (AMPDU_AQM_ENAB(wlc->pub)) {
		if (D11REV_GE(wlc->pub->corerev, 64)) {
			printf("psm_reg_mux 0x%x aqmqmap 0x%x aqmfifo_status 0x%x\n",
				R_REG(osh, &regs->u.d11acregs.psm_reg_mux),
				R_REG(osh, &regs->u.d11acregs.AQMQMAP),
				R_REG(osh, &regs->u.d11acregs.AQMFifo_Status));
			printf("AQM agg params 0x%x maxlen hi/lo 0x%x 0x%x "
				"minlen 0x%x adjlen 0x%x\n",
				R_REG(osh, &regs->u.d11acregs.AQMAggParams),
				R_REG(osh, &regs->u.d11acregs.AQMMaxAggLenHi),
				R_REG(osh, &regs->u.d11acregs.AQMMaxAggLenLow),
				R_REG(osh, &regs->u.d11acregs.AQMMinMpduLen),
				R_REG(osh, &regs->u.d11acregs.AQMMacAdjLen));
			printf("AQM agg results 0x%x num %d len hi/lo 0x%x 0x%x "
				"BAbitmap(0-3) %x %x %x %x\n",
				R_REG(osh, &regs->u.d11acregs.AQMAggStats),
				R_REG(osh, &regs->u.d11acregs.AQMAggNum),
				R_REG(osh, &regs->u.d11acregs.AQMAggLenHi),
				R_REG(osh, &regs->u.d11acregs.AQMAggLenLow),
				R_REG(osh, &regs->u.d11acregs.AQMUpdBA0),
				R_REG(osh, &regs->u.d11acregs.AQMUpdBA1),
				R_REG(osh, &regs->u.d11acregs.AQMUpdBA2),
				R_REG(osh, &regs->u.d11acregs.AQMUpdBA3));
			printf("AQM agg rdptr %d mpdu_len 0x%x idx 0x%x info 0x%x\n",
			       R_REG(osh, &regs->u.d11acregs.AQMAggRptr),
			       R_REG(osh, &regs->u.d11acregs.AQMMpduLen),
			       R_REG(osh, &regs->u.d11acregs.AQMAggIdx),
			       R_REG(osh, &regs->u.d11acregs.AQMAggEntry));
		} else {
			printf("framerdy 0x%x bmccmd %d framecnt 0x%x \n",
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMFifoReady),
				R_REG(osh, &regs->u.d11acregs.BMCCmd),
				R_REG(osh, &regs->u.d11acregs.XmtFifoFrameCnt));
			printf("AQM agg params 0x%x maxlen hi/lo 0x%x 0x%x "
				"minlen 0x%x adjlen 0x%x\n",
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMAggParams),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMMaxAggLenHi),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMMaxAggLenLow),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMMinMpduLen),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMMacAdjLen));
			printf("AQM agg results 0x%x len hi/lo 0x%x 0x%x "
				"BAbitmap(0-3) %x %x %x %x\n",
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMAggStats),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMAggLenHi),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMAggLenLow),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMUpdBA0),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMUpdBA1),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMUpdBA2),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMUpdBA3));
		}
		return ret;
	}

#ifdef WLAMPDU_UCODE
	if (AMPDU_UCODE_ENAB(wlc->pub)) {
		int i;
		ampdu_tx_info_t* ampdu_tx = wlc->ampdu_tx;
		for (i = 0; i < 4; i++) {
			int k = 0, addr;
			printf("fifo %d: rptr %x wptr %x\n",
			       i, ampdu_tx->hagg[i].txfs_rptr,
			       ampdu_tx->hagg[i].txfs_wptr);
			for (addr = ampdu_tx->hagg[i].txfs_addr_strt;
			     addr <= ampdu_tx->hagg[i].txfs_addr_end; addr++) {
				printf("\tentry %d addr 0x%x: 0x%x\n",
				       k, addr, wlc_read_shm(wlc, addr * 2));
				k++;
			}
		}
	}
#endif /* WLAMPDU_UCODE */
#if defined(WLCNT) && defined(WLAMPDU_MAC)
	printf("driver statistics: aggfifo pending %d enque/cons %d %d\n",
	       wlc->ampdu_tx->cnt->pending,
	       wlc->ampdu_tx->cnt->enq,
	       wlc->ampdu_tx->cnt->cons);
#endif

#ifdef WLAMPDU_HW
	if (AMPDU_HW_ENAB(wlc->pub)) {
		int i;
		printf("AGGFIFO regs: availcnt 0x%x txfifo fr_count %d by_count %d\n",
		       R_REG(osh, &regs->aggfifocnt),
		       R_REG(osh, &regs->u.d11regs.xmtfifo_frame_cnt),
		       R_REG(osh, &regs->u.d11regs.xmtfifo_byte_cnt));
		printf("cmd 0x%04x stat 0x%04x cfgctl 0x%04x cfgdata 0x%04x mpdunum 0x%02x"
		       " len 0x%04x bmp 0x%04x ackedcnt %d\n",
		       R_REG(osh, &regs->u.d11regs.aggfifo_cmd),
		       R_REG(osh, &regs->u.d11regs.aggfifo_stat),
		       R_REG(osh, &regs->u.d11regs.aggfifo_cfgctl),
		       R_REG(osh, &regs->u.d11regs.aggfifo_cfgdata),
		       R_REG(osh, &regs->u.d11regs.aggfifo_mpdunum),
		       R_REG(osh, &regs->u.d11regs.aggfifo_len),
		       R_REG(osh, &regs->u.d11regs.aggfifo_bmp),
		       R_REG(osh, &regs->u.d11regs.aggfifo_ackedcnt));

		/* aggfifo */
		printf("AGGFIFO dump: \n");
		for (i = 1; i < 2; i ++) {
			int j;
			printf("AGGFIFO %d:\n", i);
			for (j = 0; j < AGGFIFO_CAP; j ++) {
				uint16 entry;
				W_REG(osh, &regs->u.d11regs.aggfifo_sel, (i << 6) | j);
				W_REG(osh, &regs->u.d11regs.aggfifo_cmd, (6 << 2) | i);
				entry = R_REG(osh, &regs->u.d11regs.aggfifo_data);
				if (j % 4 == 0) {
					printf("\tEntry 0x%02x: 0x%04x	", j, entry);
				} else if (j % 4 == 3) {
					printf("0x%04x\n", entry);
				} else {
					printf("0x%04x	", entry);
				}
			}
		}
	}
#endif /* WLAMPDU_HW */

	return ret;
}
#endif /* WLAMPDU_MAC */

/**
 * Called when the number of ADDBA retries has been exceeded, thus the remote party is not
 * responding, thus transmit packets for that remote party 'ini' have to be flushed.
 */
static void
ampdu_ba_state_off(scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini)
{
#if !defined(TXQ_MUX)
	void *p;
	struct scb *scb = scb_ampdu->scb;
#endif

	ini->ba_state = AMPDU_TID_STATE_BA_OFF;

#if !defined(TXQ_MUX)
	/* release all buffered pkts */
	while ((p = wlc_ampdu_pktq_pdeq(&scb_ampdu->txq, ini->tid))) {
		ASSERT(PKTPRIO(p) == ini->tid);
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, WLC_PRIO_TO_PREC(ini->tid));
	}
#endif /* !TXQ_MUX */

}

/**
 * Move window forward when BAR isn't pending.
 * In Reinit/BigHammer situation it must be called to advance the window
 * which will be stuck otherwise when no BAR is pending...
 */
static void
ampdu_ini_move_window(ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini)
{
	uint16 indx;

	if (ini->bar_ackpending)
		return;

	if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		for (indx = 0; indx < ampdu_tx->config->ba_max_tx_wsize; indx++) {
			if (isset(ini->barpending, indx)) {
				clrbit(ini->barpending, indx);
				if (isset(ini->ackpending, indx)) {
					clrbit(ini->ackpending, indx);
					ini->txretry[indx] = 0;
				}
			}
		}
	} else {
		ini->acked_seq = ini->bar_ackpending_seq;
	}

	wlc_ampdu_ini_move_window(ampdu_tx, scb_ampdu, ini);
}

/** Called when tearing down a connection with a conversation partner (DELBA or otherwise) */
void
ampdu_cleanup_tid_ini(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 tid, bool force)
{
	wlc_info_t *wlc = ampdu_tx->wlc;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);
	ASSERT(scb_ampdu->scb);

	AMPDU_VALIDATE_TID(ampdu_tx, tid, "ampdu_cleanup_tid_ini");

	if (!(ini = scb_ampdu->ini[tid]))
		return;

	WL_AMPDU_CTL(("wl%d: ampdu_cleanup_tid_ini: tid %d force %d tx_in_transit %u rem_window %u "
	              "ba_state %u\n",
	              ampdu_tx->wlc->pub->unit, tid, force, ini->tx_in_transit, ini->rem_window,
	              ini->ba_state));

	ini->ba_state = AMPDU_TID_STATE_BA_PENDING_OFF;

	WL_AMPDU_CTL(("wl%d: ampdu_cleanup_tid_ini: tid %d force %d\n",
		wlc->pub->unit, tid, force));

	/* cleanup stuff that was to be done on bar send complete */
	ampdu_ini_move_window(ampdu_tx, scb_ampdu, ini);

#ifdef WLATF
	wlc_ampdu_atf_reset_state(ini);
#endif /* WLATF */


	if (ini->tx_in_transit && !force)
		return;

	if (ini->tx_in_transit == 0 &&
#ifdef PROP_TXSTATUS
	    PROP_TXSTATUS_ENAB(ampdu_tx->wlc->pub) &&
	    !WLFC_GET_REUSESEQ(wlfc_query_mode(ampdu_tx->wlc->wlfc)) &&
#endif /* PROP_TXSTATUS */
	    TRUE) {
		/* ASSERT only when !ini->bar_ackpending - there are packets pending
		 * on ackpending state otherwise.
		 * Sequence of events leading to above situation:
		 * - AMPDU retry exceeds limit, ackpending is still set
		 * - As a result we send bar, bar_ackpending set to TRUE
		 * - AP disassociates, rem_window is not updated because of above states
		 */
		if (!ini->bar_ackpending &&
		    ini->rem_window != ini->ba_wsize) {
			WL_AMPDU_ERR(("%s: out of sync: tid %d rem_window %u ba_wsize %u "
			          "bar_ackpending %u\n",
			          __FUNCTION__, tid, ini->rem_window, ini->ba_wsize,
			          ini->bar_ackpending));
			ASSERT(ini->rem_window == ini->ba_wsize);
		}
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, ini->scb);

	ASSERT(ini == scb_ampdu->ini[ini->tid]);

	scb_ampdu->ini[ini->tid] = NULL;

	/* free all buffered tx packets */
	wlc_ampdu_pktq_pflush(wlc, &scb_ampdu->txq, ini->tid);

	/* Cancel any pending wlc_send_bar_complete packet callbacks. */
	wlc_pcb_fn_find(ampdu_tx->wlc->pcb, wlc_send_bar_complete, ini, TRUE);

	/* Free ini immediately as no callbacks are pending */
	MFREE(ampdu_tx->wlc->osh, ini, sizeof(scb_ampdu_tid_ini_t));

} /* ampdu_cleanup_tid_ini */

void
wlc_ampdu_recv_addba_resp(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 *body, int body_len)
{
	scb_ampdu_tx_t *scb_ampdu_tx;
	wlc_info_t *wlc = ampdu_tx->wlc;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	struct pktq *txq;
	dot11_addba_resp_t *addba_resp;
	scb_ampdu_tid_ini_t *ini;
	uint16 param_set, status;
	uint8 tid, wsize, policy;
	uint16 current_wlctxq_len = 0, i = 0;
	void *p = NULL;

	BCM_REFERENCE(body_len);

	ASSERT(scb);

	scb_ampdu_tx = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu_tx);

	addba_resp = (dot11_addba_resp_t *)body;

	if (addba_resp->category & DOT11_ACTION_CAT_ERR_MASK) {
		WL_AMPDU_ERR(("wl%d: %s: unexp error action frame\n",
				wlc->pub->unit, __FUNCTION__));
		WLCNTINCR(ampdu_tx->cnt->rxunexp);
		return;
	}

	status = ltoh16_ua(&addba_resp->status);
	param_set = ltoh16_ua(&addba_resp->addba_param_set);

	wsize =	(param_set & DOT11_ADDBA_PARAM_BSIZE_MASK) >> DOT11_ADDBA_PARAM_BSIZE_SHIFT;
	policy = (param_set & DOT11_ADDBA_PARAM_POLICY_MASK) >> DOT11_ADDBA_PARAM_POLICY_SHIFT;
	tid = (param_set & DOT11_ADDBA_PARAM_TID_MASK) >> DOT11_ADDBA_PARAM_TID_SHIFT;
	AMPDU_VALIDATE_TID(ampdu_tx, tid, "wlc_ampdu_recv_addba_resp");

	ini = scb_ampdu_tx->ini[tid];

	if ((ini == NULL) || (ini->ba_state != AMPDU_TID_STATE_BA_PENDING_ON)) {
		WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_addba_resp: unsolicited packet\n",
			wlc->pub->unit));
		WLCNTINCR(ampdu_tx->cnt->rxunexp);
		return;
	}

	if ((status != DOT11_SC_SUCCESS) ||
	    (policy != ampdu_tx_cfg->ba_policy) ||
	    (wsize > ampdu_tx_cfg->ba_max_tx_wsize)) {
		WL_AMPDU_ERR(("wl%d: %s: Failed. status %d wsize %d policy %d\n",
			wlc->pub->unit, __FUNCTION__, status, wsize, policy));
		ampdu_ba_state_off(scb_ampdu_tx, ini);
		WLCNTINCR(ampdu_tx->cnt->rxunexp);
		return;
	}

#ifdef WLAMSDU_TX
	if (AMSDU_TX_ENAB(wlc->pub)) {
		/* Record whether this scb supports amsdu over ampdu */
		scb->flags2 &= ~SCB2_AMSDU_IN_AMPDU_CAP;
		if ((param_set & DOT11_ADDBA_PARAM_AMSDU_SUP) != 0) {
			scb->flags2 |= SCB2_AMSDU_IN_AMPDU_CAP;

#ifdef WL_MU_TX
			if (SCB_MU(scb)) {
				int8 prev_max_pdu = scb_ampdu_tx->max_pdu;
				/* With amsdu_in_ampdu, scb max_pdu may change */
				scb_ampdu_update_config(ampdu_tx, scb);
				if (WLC_TXC_ENAB(ampdu_tx->wlc) &&
					(prev_max_pdu != scb_ampdu_tx->max_pdu)) {
					/* If max_pdu changes, invalid tx cache */
					wlc_txc_inv_all(ampdu_tx->wlc->txc);
				}
			}
#endif
		}
	}
#endif /* WLAMSDU_TX */

	ini->ba_wsize = wsize;
	ini->rem_window = wsize;

#ifdef PROP_TXSTATUS
	ini->suppr_window = 0; /* suppr packet count inside ba window */
#endif /* PROP_TXSTATUS */

	ini->start_seq = (SCB_SEQNUM(scb, tid) & (SEQNUM_MAX - 1));
	ini->tx_exp_seq = ini->start_seq;
	ini->acked_seq = ini->max_seq = MODSUB_POW2(ini->start_seq, 1, SEQNUM_MAX);
	ini->ba_state = AMPDU_TID_STATE_BA_ON;
#ifdef WLATF
	wlc_ampdu_atf_reset_state(ini);
#endif /* WLATF */

	WLCNTINCR(ampdu_tx->cnt->rxaddbaresp);
	AMPDUSCBCNTINCR(scb_ampdu_tx->cnt.rxaddbaresp);

	WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_addba_resp: Turning BA ON: tid %d wsize %d\n",
		wlc->pub->unit, tid, wsize));

	/* send bar to set initial window */
	wlc_ampdu_send_bar(ampdu_tx, ini, TRUE);

	/* if packets already exist in wlcq, conditionally transfer them to ampduq
		to avoid barpending stuck issue.
	*/
	txq = WLC_GET_TXQ(SCB_WLCIFP(scb)->qi);

	current_wlctxq_len = pktqprec_n_pkts(txq, WLC_PRIO_TO_PREC(tid));

	for (i = 0; i < current_wlctxq_len; i++) {
		p = pktq_pdeq(txq, WLC_PRIO_TO_PREC(tid));

		if (!p) break;

		/* omit control/mgmt frames and queue them to the back
		 * only the frames relevant to this scb should be moved to ampduq;
		 * otherwise, queue them back.
		 */
		if (!(WLPKTTAG(p)->flags & WLF_DATA) || /* mgmt/ctl */
		    scb != WLPKTTAGSCBGET(p) || /* frame belong to some other scb */
#if defined(PROP_TXSTATUS)
			(AMPDU_HOST_ENAB(wlc->pub) &&
			IS_WL_TO_REUSE_SEQ(WLPKTTAG(p)->seq)) || /* suppressed packet */
#endif /* PROP_TXSTATUS */
			(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU)) { /* possibly retried AMPDU */
			/* WES: Direct TxQ reference */
			if (wlc_prec_enq(wlc, txq, p, WLC_PRIO_TO_PREC(tid)))
				continue;
		} else { /* XXXCheck if Flow control/watermark issues */
			if (wlc_ampdu_prec_enq(wlc, &scb_ampdu_tx->txq, p, tid))
				continue;
		}

		wlc_tx_status_update_counters(wlc, p,
			scb, NULL, WLC_TX_STS_TX_Q_FULL, 1);

		/* Toss packet as queuing failed */
		WL_AMPDU_ERR(("wl%d: txq overflow\n", wlc->pub->unit));
		PKTFREE(wlc->osh, p, TRUE);
		WLCNTINCR(wlc->pub->_cnt->txnobuf);
		WLCNTINCR(ampdu_tx->cnt->txdrop);
		AMPDUSCBCNTINCR(scb_ampdu_tx->cnt.txdrop);
		WLCIFCNTINCR(scb, txnobuf);
		WLCNTSCBINCR(scb->scb_stats.tx_failures);
	}

	/* release pkts */
	wlc_ampdu_txeval(ampdu_tx, scb_ampdu_tx, ini, FALSE);
} /* wlc_ampdu_recv_addba_resp */

void
wlc_ampdu_recv_ba(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 *body, int body_len)
{
	BCM_REFERENCE(scb);
	BCM_REFERENCE(body);
	BCM_REFERENCE(body_len);
	/* AMPDUXXX: silently ignore the ba since we don't handle it for immediate ba */
	WLCNTINCR(ampdu_tx->cnt->rxba);
}

#ifdef WLATF
/* ATF counter clearing code */
#if defined(WLCNT) && defined(BCMDBG)
static void wlc_ampdu_atf_clear_counters(scb_ampdu_tid_ini_t *ini)
{
	bzero(AMPDU_ATF_STATS(ini), sizeof(atf_stats_t));
}

static BCMFASTPATH void wlc_ampdu_atf_update_rstat(atf_stats_range_t *range, uint32 val)
{
	unsigned long accum = range->accum + val;
	uint32 iter = range->iter + 1;

	if (val > range->max) {
		range->max = val;
	}

	if ((val < range->min) || (range->min == 0)) {
		range->min = val;
	}

	/* Overflow check */
	if ((accum < val) || (!iter)) {
		accum = val;
		iter = 1;
	}
	range->accum = accum;
	range->iter = iter;
}

static uint wlc_ampdu_atf_rstat_avg(atf_stats_range_t *rstat)
{
	rstat->avg = (rstat->iter) ? rstat->accum/rstat->iter : 0;
	return (rstat->avg);
}

static void wlc_ampdu_atf_print_rstat(struct bcmstrbuf *b,
	char *hdr, atf_stats_range_t *rstat, char *tlr)
{
	bcm_bprintf(b, "%s(%u/%u/%u)%s", hdr, wlc_ampdu_atf_rstat_avg(rstat),
		rstat->min, rstat->max, tlr);
}

static void wlc_ampdu_atf_dump(atf_state_t *atf_state, struct bcmstrbuf *b)
{
				bcm_bprintf(b, "\tatf %d ta %dus tm %dus rso:0x%x\n",
					atf_state->atf,
					atf_state->txq_time_allowance_us,
					atf_state->txq_time_min_allowance_us,
					atf_state->rspec_override);
				if (atf_state->atf) {
				atf_stats_t *atf_stats = &atf_state->atf_stats;

				bcm_bprintf(b, "\t tlimit %d flimit %d uflow %d oflow %d\n",
					atf_stats->timelimited, atf_stats->framelimited,
					atf_stats->uflow, atf_stats->oflow);
				bcm_bprintf(b, "\t reset %d flush %d po %d minrel %d sgl %d\n",
					atf_state->reset, atf_stats->flush,
					atf_stats->reloverride, atf_stats->minrel,
					atf_stats->singleton);
				bcm_bprintf(b, "\t eval %d neval %d dq %d skp %d npkts %d\n",
					atf_stats->eval, atf_stats->neval, atf_stats->ndequeue,
					atf_stats->rskip, atf_stats->npkts);
				bcm_bprintf(b, "\t qe %d proc %d rp %d nA %d\n",
					atf_stats->qempty, atf_stats->proc,
					atf_stats->reproc, atf_stats->reg_mpdu);
				bcm_bprintf(b, "\t cache hit/miss(%d/%d)\n",
					atf_stats->cache_hit, atf_stats->cache_miss);
				bcm_bprintf(b, "\t avg/min/max\n");

				wlc_ampdu_atf_print_rstat(b,
					"\t  rbytes", &atf_stats->rbytes, "\n");
				wlc_ampdu_atf_print_rstat(b,
					"\t  in_flt", &atf_stats->inflight, "\n");
				wlc_ampdu_atf_print_rstat(b,
					"\t  inputq", &atf_stats->inputq, "\n");

				wlc_ampdu_atf_print_rstat(b,
					"\t  rpkts", &atf_stats->release, "  ");
				wlc_ampdu_atf_print_rstat(b,
					"transit", &atf_stats->transit, "\n");

				wlc_ampdu_atf_print_rstat(b,
					"\t  chunk bytes", &atf_stats->chunk_bytes, " ");
				wlc_ampdu_atf_print_rstat(b,
					"pkts", &atf_stats->chunk_pkts, "  ");
				wlc_ampdu_atf_print_rstat(b,
					"pdu", &atf_stats->pdu, "\n");
				}
}
#endif /* if defined(WLCNT) && defined(BCMDBG) */
/* Set per TID atf mode */
static void wlc_ampdu_atf_tid_setmode(scb_ampdu_tid_ini_t *ini, uint32 mode)
{
	wlc_ampdu_atf_reset_state(ini);
	AMPDU_ATF_STATE(ini)->atf = mode;
}

/* Set per SCB atf mode */
static void wlc_ampdu_atf_scb_setmode(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint32 mode)
{
	uint32 n = 0;
	scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	for (n = 0; n < AMPDU_MAX_SCB_TID; n++) {
		if (scb_ampdu && scb_ampdu->ini[n])
			wlc_ampdu_atf_tid_setmode(scb_ampdu->ini[n], mode);
	}
}

/* Set global ATF default mode, used each time a TID for a SCB is created */
void wlc_ampdu_atf_set_default_mode(ampdu_tx_info_t *ampdu_tx, scb_module_t *scbstate, uint32 mode)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			wlc_ampdu_atf_scb_setmode(ampdu_tx, scb, mode);
		}
	}
}

/* Set per TID atf txq release time allowance in microseconds */
static void wlc_ampdu_atf_tid_set_release_time(scb_ampdu_tid_ini_t *ini, uint txq_time_allowance_us)
{
	AMPDU_ATF_STATE(ini)->txq_time_allowance_us = txq_time_allowance_us;
	/* Reset the last cached ratespec value in order to force to recalculate the
	 * rbytes using new value of txq_time_allowance_us.
	 */
	AMPDU_ATF_STATE(ini)->last_est_rate = 0;
}

/* Set per SCB atf txq release time allowance in microseconds */
static void wlc_ampdu_atf_scb_set_release_time(ampdu_tx_info_t *ampdu_tx,
	struct scb *scb, uint txq_time_allowance_us)
{
	uint32 n = 0;
	scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	for (n = 0; n < AMPDU_MAX_SCB_TID; n++) {
		if (scb_ampdu && scb_ampdu->ini[n])
			wlc_ampdu_atf_tid_set_release_time(
				scb_ampdu->ini[n], txq_time_allowance_us);
	}
}

/* Set per SCB atf txq release time allowance in microseconds for all SCBs */
static void wlc_ampdu_atf_set_default_release_time(ampdu_tx_info_t *ampdu_tx,
	scb_module_t *scbstate, uint txq_time_allowance_us)
{
	struct scb *scb;
	struct scb_iter scbiter;
	FOREACHSCB(scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			wlc_ampdu_atf_scb_set_release_time(ampdu_tx, scb, txq_time_allowance_us);
		}
	}
}

/* Set per TID atf txq release minimum time allowance in microseconds */
static void wlc_ampdu_atf_tid_set_release_mintime(scb_ampdu_tid_ini_t *ini, uint allowance_us)
{
	AMPDU_ATF_STATE(ini)->txq_time_min_allowance_us = allowance_us;
	/* Reset the last cached ratespec value in order to force to recalculate the
	 * rbytes using new value of txq_time_min_allowance_us.
	 */
	AMPDU_ATF_STATE(ini)->last_est_rate = 0;
}

/* Set per SCB atf txq release time allowance in microseconds */
static void wlc_ampdu_atf_scb_set_release_mintime(ampdu_tx_info_t *ampdu_tx,
	struct scb *scb, uint allowance_us)
{
	uint32 n = 0;
	scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	for (n = 0; n < AMPDU_MAX_SCB_TID; n++) {
		if (scb_ampdu && scb_ampdu->ini[n])
			wlc_ampdu_atf_tid_set_release_mintime(
				scb_ampdu->ini[n], allowance_us);
	}
}

/* Set per SCB atf txq release time allowance in microseconds for all SCBs */
static void wlc_ampdu_atf_set_default_release_mintime(ampdu_tx_info_t *ampdu_tx,
	scb_module_t *scbstate, uint allowance_us)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			wlc_ampdu_atf_scb_set_release_mintime(ampdu_tx, scb, allowance_us);
		}
	}
}

/* Set the function pointer that returns rate state information */
static void
wlc_ampdu_atf_set_rspec_action(atf_state_t *atf_state, atf_rspec_fn_t fn, void *arg)
{
	atf_state->rspec_action.function = fn;
	atf_state->rspec_action.arg = arg;
}

/* Alternate rate function, used whe the rate selection engine is manually over-ridden */
static ratespec_t BCMFASTPATH
wlc_ampdu_atf_ratespec_override(void *ptr)
{
#ifdef WLATF_FASTRATE
	return ((atf_state_t *)(ptr))->rspec_override;
#else
	atf_state_t *atf_state = (atf_state_t *)(ptr);
	return wlc_scb_ratesel_get_primary(atf_state->wlc, atf_state->scb, NULL);
#endif
}

/* Set the rate override condition in ATF */
static void
wlc_ampdu_atf_ini_set_rspec_action(atf_state_t *atf_state, ratespec_t rspec)
{
	/* Sets the rspec action which is a function and an argument
	 * If rspec is zero default to wlc_ratesel_rawcurspec()
	 */

	if (rspec) {
		wlc_ampdu_atf_set_rspec_action(atf_state,
			wlc_ampdu_atf_ratespec_override, atf_state);
	} else {
		/* Set the default function pointer that returns rate state information */
		wlc_ampdu_atf_set_rspec_action(atf_state,
			(atf_rspec_fn_t)wlc_ratesel_rawcurspec, (void *)atf_state->rcb);
	}

	atf_state->rspec_override = rspec;
}

/* Wrapper function to enable/disable rate overide for all ATF tids in the SCB
 * Called by per-SCB rate override code.
 */
void
wlc_ampdu_atf_scb_rate_override(ampdu_tx_info_t *ampdu_tx, struct scb *scb, ratespec_t rspec)
{
	uint32 n = 0;
	scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);

	if (!scb_ampdu)
		return;

	for (n = 0; n < AMPDU_MAX_SCB_TID; n++) {
		if (scb_ampdu->ini[n])
			wlc_ampdu_atf_ini_set_rspec_action(
			AMPDU_ATF_STATE(scb_ampdu->ini[n]), rspec);
	}
}
/* Wrapper function to enable/disable rate overide for the entire driver,
 * called by rate override code
 */
void
wlc_ampdu_atf_rate_override(wlc_info_t *wlc, ratespec_t rspec, wlcband_t *band)
{
	/* Process rate update notification */
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (band->bandtype == wlc_scbband(wlc, scb)->bandtype) {
			wlc_ampdu_atf_scb_rate_override(wlc->ampdu_tx, scb, rspec);
		}
	}
}

static void wlc_ampdu_atf_tid_ini(ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini)
{
	wlc_info_t *wlc = ampdu_tx->wlc;
	atf_state_t *atf_state = AMPDU_ATF_STATE(ini);

	atf_state->ac = WME_PRIO2AC(ini->tid);
	atf_state->band = wlc->bandstate[
		CHSPEC_WLCBANDUNIT(scb_ampdu->scb->bsscfg->current_bss->chanspec)];
	atf_state->rcb = wlc_scb_ratesel_getrcb(wlc, scb_ampdu->scb, WME_PRIO2AC(ini->tid));
	ASSERT(atf_state->rcb);
	atf_state->ampdu_tx = ampdu_tx;
	atf_state->scb_ampdu = scb_ampdu;
	atf_state->ini = ini;
	atf_state->wlc = wlc;
	atf_state->scb = ini->scb;

	/* Set operating state, this resets state machine */
	wlc_ampdu_atf_tid_setmode(ini, wlc->atf);

	/* Reset ATF state machine again, it is assumed the tid is coming from an
	 * unknown and possibly invalid state.
	 */
	wlc_ampdu_atf_reset_state(ini);
	/* Set other operating parameters */
	wlc_ampdu_atf_tid_set_release_time(ini, ampdu_tx->config->txq_time_allowance_us);
	wlc_ampdu_atf_tid_set_release_mintime(ini, ampdu_tx->config->txq_time_min_allowance_us);
}
#else
#define wlc_ampdu_atf_tid_ini(ampdu_tx, scb_ampdu, ini)
#define wlc_ampdu_atf_dump(atf_state, b)
#endif /* WLATF */

/** initialize the initiator code for tid. Called around ADDBA exchange. */
static scb_ampdu_tid_ini_t*
wlc_ampdu_init_tid_ini(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu, uint8 tid,
	bool override)
{
	scb_ampdu_tid_ini_t *ini;
	wlc_info_t *wlc = ampdu_tx->wlc;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	struct scb *scb;
	uint8 peer_density;
	uint32 max_rxlen;
	uint8 max_rxlen_factor;
	uint8 i;

	ASSERT(scb_ampdu);
	ASSERT(scb_ampdu->scb);
	ASSERT(SCB_AMPDU(scb_ampdu->scb));
	ASSERT(tid < AMPDU_MAX_SCB_TID);
	scb = scb_ampdu->scb;

	if (ampdu_tx_cfg->manual_mode && !override)
		return NULL;

	/* check for per-tid control of ampdu */
	if (!ampdu_tx_cfg->ini_enable[tid])
		return NULL;

	/* AMPDUXXX: No support for dynamic update of density/len from peer */
	/* retrieve the density and max ampdu len from scb */
	wlc_ht_get_scb_ampdu_params(wlc->hti, scb, &peer_density,
		&max_rxlen, &max_rxlen_factor);

	/* our density requirement is for tx side as well */
	scb_ampdu->mpdu_density = MAX(peer_density, ampdu_tx_cfg->mpdu_density);

	/* should call after mpdu_density is init'd */
	wlc_ampdu_init_min_lens(scb_ampdu);

	/* max_rxlen is only used for host aggregation (corerev < 40) */
	scb_ampdu->max_rxlen = max_rxlen;

	scb_ampdu->max_rxfactor = max_rxlen_factor;

	/* initializing the ampdu max duration per module */
	for (i = 0; i < NUM_MODULES; i++)
		scb_ampdu->module_max_dur[i] = AMPDU_MAXDUR_INVALID_VAL;

	scb_ampdu->min_of_max_dur_idx = AMPDU_MAXDUR_INVALID_IDX;

#if defined(WL11AC)
	if (VHT_ENAB_BAND(wlc->pub, wlc->band->bandtype) && SCB_VHT_CAP(scb)) {
		scb_ampdu->max_rxfactor = wlc_vht_get_scb_ampdu_max_exp(wlc->vhti, scb) +
			AMPDU_RX_FACTOR_BASE_PWR;
	}
#endif /* WL11AC */

	scb_ampdu_update_config(ampdu_tx, scb);

	if (!scb_ampdu->ini[tid]) {
		ini = MALLOCZ(wlc->osh, sizeof(scb_ampdu_tid_ini_t));
		if (ini == NULL) {
			WL_ERROR((WLC_MALLOC_ERR, WLCWLUNIT(ampdu_tx->wlc), __FUNCTION__,
				(int)sizeof(scb_ampdu_tid_ini_t), MALLOCED(wlc->osh)));
			return NULL;
		}

		scb_ampdu->ini[tid] = ini;
	} else {
		ini = scb_ampdu->ini[tid];
	}

	memset(ini, 0, sizeof(scb_ampdu_tid_ini_t));

	ini->ba_state = AMPDU_TID_STATE_BA_PENDING_ON;
	ini->tid = tid;
	ini->scb = scb;
	ini->retry_bar = AMPDU_R_BAR_DISABLED;
#if defined(TXQ_MUX) && !defined(WLATF)
	ini->ac = WME_PRIO2AC(tid);
#endif
	wlc_ampdu_atf_tid_ini(ampdu_tx, scb_ampdu, ini);

	wlc_send_addba_req(wlc, ini->scb, tid, ampdu_tx_cfg->ba_tx_wsize,
		ampdu_tx->config->ba_policy, ampdu_tx_cfg->delba_timeout);
	ini->last_addba_ts = OSL_SYSUPTIME();

	WLCNTINCR(ampdu_tx->cnt->txaddbareq);
	AMPDUSCBCNTINCR(scb_ampdu->cnt.txaddbareq);

	return ini;
} /* wlc_ampdu_init_tid_ini */

/** Remote side sent us a ADDBA request */
void
wlc_ampdu_recv_addba_req_ini(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
	dot11_addba_req_t *addba_req, int body_len)
{
	scb_ampdu_tx_t *scb_ampdu_tx;
	scb_ampdu_tid_ini_t *ini;
	uint16 param_set;
	uint8 tid;

	BCM_REFERENCE(body_len);

	scb_ampdu_tx = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu_tx);

	param_set = ltoh16_ua(&addba_req->addba_param_set);
	tid = (param_set & DOT11_ADDBA_PARAM_TID_MASK) >> DOT11_ADDBA_PARAM_TID_SHIFT;
	AMPDU_VALIDATE_TID(ampdu_tx, tid, "wlc_ampdu_recv_addba_req_ini");

	/* check if it is action err frame */
	if (addba_req->category & DOT11_ACTION_CAT_ERR_MASK) {
		ini = scb_ampdu_tx->ini[tid];
		if ((ini != NULL) && (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
			ampdu_ba_state_off(scb_ampdu_tx, ini);
			WL_AMPDU_ERR(("wl%d: %s: error action frame\n",
				ampdu_tx->wlc->pub->unit, __FUNCTION__));
		} else {
			WL_AMPDU_ERR(("wl%d: %s: unexp error action "
				"frame\n", ampdu_tx->wlc->pub->unit, __FUNCTION__));
		}
		WLCNTINCR(ampdu_tx->cnt->rxunexp);
		return;
	}
}

/** called due to 'wl' utility or as part of the attach phase */
int
wlc_ampdu_tx_set(ampdu_tx_info_t *ampdu_tx, bool on)
{
	wlc_info_t *wlc = ampdu_tx->wlc;
	uint8 ampdu_mac_agg = AMPDU_AGG_OFF;
	uint16 value = 0;
	int err = BCME_OK;
	int8 aggmode;

	wlc->pub->_ampdu_tx = FALSE;

	if (ampdu_tx->config->ampdu_aggmode == AMPDU_AGGMODE_AUTO) {
		if (D11REV_IS(wlc->pub->corerev, 31) && WLCISNPHY(wlc->band)) {
			aggmode = AMPDU_AGGMODE_HOST;
		} else if ((D11REV_IS(wlc->pub->corerev, 26) ||
		            D11REV_IS(wlc->pub->corerev, 29)) &&
		           WLCISHTPHY(wlc->band)) {
			aggmode = AMPDU_AGGMODE_HOST;
		} else if (D11REV_LT(wlc->pub->corerev, 26))
			aggmode = AMPDU_AGGMODE_MAC;
#ifdef WLAMPDU_UCODE
		else if (D11REV_IS(wlc->pub->corerev, 39))
			aggmode = AMPDU_AGGMODE_MAC;
#endif /* WLAMPDU_UCODE */
		else if (D11REV_GE(wlc->pub->corerev, 40))
			aggmode = AMPDU_AGGMODE_MAC;
		else
			aggmode = AMPDU_AGGMODE_HOST;
	} else
		aggmode = ampdu_tx->config->ampdu_aggmode;

	if (on) {
		if (!N_ENAB(wlc->pub)) {
			WL_AMPDU_ERR(("wl%d: driver not nmode enabled\n", wlc->pub->unit));
			err = BCME_UNSUPPORTED;
			goto exit;
		}
		if (!wlc_ampdu_tx_cap(ampdu_tx)) {
			WL_AMPDU_ERR(("wl%d: device not ampdu capable\n", wlc->pub->unit));
			err = BCME_UNSUPPORTED;
			goto exit;
		}
		if (PIO_ENAB(wlc->pub)) {
			WL_AMPDU_ERR(("wl%d: driver is pio mode\n", wlc->pub->unit));
			err = BCME_UNSUPPORTED;
			goto exit;
		}

#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */
		if (aggmode == AMPDU_AGGMODE_MAC) {
			if (D11REV_LT(wlc->pub->corerev, 26)) {
#ifdef WLAMPDU_UCODE
				ampdu_mac_agg = AMPDU_AGG_UCODE;
				WL_AMPDU(("wl%d: 26 > Corerev >= 16 required for ucode agg\n",
					wlc->pub->unit));
#endif /* WLAMPDU_UCODE */
			}
			if (D11REV_IS(wlc->pub->corerev, 24) && WLCISNPHY(wlc->band)) {
#ifdef WLAMPDU_UCODE
				ampdu_mac_agg = AMPDU_AGG_UCODE;
				WL_AMPDU(("wl%d: Corerev is 24 NPHY support ucode agg only\n",
				          wlc->pub->unit));
#endif /* WLAMPDU_UCODE */
			}
			if ((D11REV_IS(wlc->pub->corerev, 26) ||
				D11REV_IS(wlc->pub->corerev, 29)) && WLCISHTPHY(wlc->band)) {
				ampdu_mac_agg = AMPDU_AGG_HW;
				WL_AMPDU(("wl%d: Corerev is %d HTPHY support hw agg only\n",
					wlc->pub->unit, wlc->pub->corerev));
			}
			if (D11REV_IS(wlc->pub->corerev, 31) && WLCISNPHY(wlc->band)) {
				ampdu_mac_agg = AMPDU_AGG_HW;
				WL_AMPDU(("wl%d: Corerev is 31 NPHY support hw agg only\n",
					wlc->pub->unit));
			}
			if (D11REV_IS(wlc->pub->corerev, 34) && WLCISNPHY(wlc->band)) {
#ifdef WLAMPDU_UCODE
				ampdu_mac_agg = AMPDU_AGG_UCODE;
				WL_AMPDU(("wl%d: Corerev is 34 NPHY support ucode agg only\n",
					wlc->pub->unit));
#endif /* WLAMPDU_UCODE */
			}
			if (D11REV_IS(wlc->pub->corerev, 36)) {
				ampdu_mac_agg = AMPDU_AGG_HW;
				WL_AMPDU(("wl%d: Corerev is 36 support hw agg only\n",
					wlc->pub->unit));
			}
			if (D11REV_IS(wlc->pub->corerev, 37) && WLCISNPHY(wlc->band)) {
				ampdu_mac_agg = AMPDU_AGG_HW;
				WL_AMPDU(("wl%d: Corerev is 37 NPHY support hw agg only\n",
					wlc->pub->unit));
			} else if (D11REV_GE(wlc->pub->corerev, 37) &&
				D11REV_LE(wlc->pub->corerev, 39)) {
#ifdef WLAMPDU_UCODE
				ampdu_mac_agg = AMPDU_AGG_UCODE;
				WL_AMPDU(("wl%d: Corerev is %d support ucode agg only\n",
					wlc->pub->unit, wlc->pub->corerev));
				if (D11REV_IS(wlc->pub->corerev, 39))
					wlc_mhf(wlc, MHF5, MHF5_UC_PRELOAD, MHF5_UC_PRELOAD,
						WLC_BAND_ALL);
#endif /* WLAMPDU_UCODE */
			}
			if (D11REV_GE(wlc->pub->corerev, 40)) {
				ampdu_mac_agg = AMPDU_AGG_AQM;
				WL_AMPDU(("wl%d: Corerev is 40 aqm agg\n",
					wlc->pub->unit));
			}

			/* Catch building a particular mode which is not supported by ucode rev */
		}
#endif /* WLAMPDU_MAC */
	}

	/* if aggmode define as MAC, but mac_agg is set to OFF,
	 * then set aggmode to HOST as defaultw
	 */
	if ((aggmode == AMPDU_AGGMODE_MAC) && (ampdu_mac_agg == AMPDU_AGG_OFF))
		aggmode = AMPDU_AGGMODE_HOST;

#ifdef WLAMPDU_MAC
	/* setup ucode tx-retrys for MAC mode */
	if ((aggmode == AMPDU_AGGMODE_MAC) && (ampdu_mac_agg != AMPDU_AGG_AQM)) {
		value = MHF3_UCAMPDU_RETX;
	}
#endif /* WLAMPDU_MAC */

	if (wlc->pub->_ampdu_tx != on) {
#ifdef WLCNT
		bzero(ampdu_tx->cnt, sizeof(wlc_ampdu_tx_cnt_t));
#endif
		wlc->pub->_ampdu_tx = on;
	}

exit:
	if (wlc->pub->_ampdumac != ampdu_mac_agg) {
		wlc->pub->_ampdumac = ampdu_mac_agg;
		wlc_mhf(wlc, MHF3, MHF3_UCAMPDU_RETX, value, WLC_BAND_ALL);
		/* just differentiate HW or not for now */
		wlc_ampdu_mode_upd(wlc, AMPDU_HW_ENAB(wlc->pub) ?
			AMPDU_AGG_HW : AMPDU_AGGMODE_HOST);
	}

	/* setup max pkt aggr for MAC mode */
	wlc_set_default_txmaxpkts(wlc);

	WL_AMPDU(("wl%d: %s: %s txmaxpkts %d\n", wlc->pub->unit, __FUNCTION__,
	          ((aggmode == AMPDU_AGGMODE_HOST) ? " AGG Mode = HOST" :
	           ((ampdu_mac_agg == AMPDU_AGG_UCODE) ? "AGG Mode = MAC+Ucode" :
	           ((ampdu_mac_agg == AMPDU_AGG_HW) ? "AGG Mode = MAC+HW" : "AGG Mode = MAC+AQM"))),
	          wlc_get_txmaxpkts(wlc)));

	return err;
} /* wlc_ampdu_tx_set */

bool
wlc_ampdu_tx_cap(ampdu_tx_info_t *ampdu_tx)
{
	if (WLC_PHY_11N_CAP(ampdu_tx->wlc->band))
		return TRUE;
	else
		return FALSE;
}

#ifdef AMPDU_NON_AQM

/** called during attach phase and as a result of a 'wl' command */
static void
ampdu_update_max_txlen(ampdu_tx_config_t *ampdu_tx_cfg, uint16 dur)
{
#if !defined(WLPROPRIETARY_11N_RATES) /* avoiding ROM abandons */
	uint32 rate, mcs;
#else
	uint32 rate;
	int mcs = -1;
#endif /* WLPROPRIETARY_11N_RATES */

	dur = dur >> 10;

#if defined(WLPROPRIETARY_11N_RATES)
	while (TRUE) {
		mcs = NEXT_MCS(mcs); /* iterate through both standard and prop ht rates */
		if (mcs > WLC_11N_LAST_PROP_MCS)
			break;
#else
	for (mcs = 0; mcs < MCS_TABLE_SIZE; mcs++) {
#endif /* WLPROPRIETARY_11N_RATES */
		/* rate is in Kbps; dur is in msec ==> len = (rate * dur) / 8 */
		/* 20MHz, No SGI */
		rate = MCS_RATE(mcs, FALSE, FALSE);
		ampdu_tx_cfg->max_txlen[MCS2IDX(mcs)][0][0] = (rate * dur) >> 3;
		/* 40 MHz, No SGI */
		rate = MCS_RATE(mcs, TRUE, FALSE);
		ampdu_tx_cfg->max_txlen[MCS2IDX(mcs)][1][0] = (rate * dur) >> 3;
		/* 20MHz, SGI */
		rate = MCS_RATE(mcs, FALSE, TRUE);
		ampdu_tx_cfg->max_txlen[MCS2IDX(mcs)][0][1] = (rate * dur) >> 3;
		/* 40 MHz, SGI */
		rate = MCS_RATE(mcs, TRUE, TRUE);
		ampdu_tx_cfg->max_txlen[MCS2IDX(mcs)][1][1] = (rate * dur) >> 3;
	}
}

#endif /* AMPDU_NON_AQM */

/**
 * Applicable to pre-AC chips. Reduces tx underflows on lower MCS rates (e.g. MCS 15) on 40Mhz with
 * frame bursting enabled. If frameburst is enabled, and 40MHz BW, and ampdu_density is smaller than
 * 7, transmit uses adjusted min_bytes.
 *
 * Called during AMPDU connection setup (ADDBA related).
 */
static void
wlc_ampdu_init_min_lens(scb_ampdu_tx_t *scb_ampdu)
{
	int mcs, tmp;
	tmp = 1 << (16 - scb_ampdu->mpdu_density);
#if defined(WLPROPRIETARY_11N_RATES)
	mcs = -1;
	while (TRUE) {
		mcs = NEXT_MCS(mcs); /* iterate through both standard and prop ht rates */
		if (mcs > WLC_11N_LAST_PROP_MCS)
			break;
#else
	for (mcs = 0; mcs <= AMPDU_MAX_MCS; mcs++) {
#endif /* WLPROPRIETARY_11N_RATES */
		scb_ampdu->min_lens[MCS2IDX(mcs)] =  0;
		if (scb_ampdu->mpdu_density > AMPDU_DENSITY_4_US)
			continue;
		if (mcs >= 12 && mcs <= 15) {
			/* use rate for mcs21, 40Mhz bandwidth, no sgi */
			scb_ampdu->min_lens[MCS2IDX(mcs)] = CEIL(MCS_RATE(21, 1, 0), tmp);
		} else if (mcs == 21 || mcs == 22) {
			/* use mcs23 */
			scb_ampdu->min_lens[MCS2IDX(mcs)] = CEIL(MCS_RATE(23, 1, 0), tmp);
#if !defined(WLPROPRIETARY_11N_RATES)
		}
#else
		} else if (mcs >= WLC_11N_FIRST_PROP_MCS && mcs <= WLC_11N_LAST_PROP_MCS) {
			scb_ampdu->min_lens[MCS2IDX(mcs)] = CEIL(MCS_RATE(mcs, 1, 0), tmp);
		}
#endif /* WLPROPRIETARY_11N_RATES */
	}

	if (WLPROPRIETARY_11N_RATES_ENAB(scb_ampdu->scb->bsscfg->wlc->pub) &&
		scb_ampdu->scb->bsscfg->wlc->pub->ht_features != WLC_HT_FEATURES_PROPRATES_DISAB) {
			scb_ampdu->min_len = CEIL(MCS_RATE(WLC_11N_LAST_PROP_MCS, 1, 1), tmp);
	} else {
		scb_ampdu->min_len = CEIL(MCS_RATE(AMPDU_MAX_MCS, 1, 1), tmp);
	}
} /* wlc_ampdu_init_min_lens */

/** Called by higher software layer. Related to AMPDU density (gaps between PDUs in AMPDU) */
uint8 BCMFASTPATH
wlc_ampdu_null_delim_cnt(ampdu_tx_info_t *ampdu_tx, struct scb *scb, ratespec_t rspec,
	int phylen, uint16* minbytes)
{
	scb_ampdu_tx_t *scb_ampdu;
	int bytes = 0, cnt, tmp;
	uint8 tx_density;

	ASSERT(scb);
	ASSERT(SCB_AMPDU(scb));

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	if (scb_ampdu->mpdu_density == 0)
		return 0;

	/* RSPEC2RATE is in kbps units ==> ~RSPEC2RATE/2^13 is in bytes/usec
	   density x is in 2^(x-3) usec
	   ==> # of bytes needed for req density = rate/2^(16-x)
	   ==> # of null delimiters = ceil(ceil(rate/2^(16-x)) - phylen)/4)
	 */

	tx_density = scb_ampdu->mpdu_density;

	if (wlc_btc_wire_get(ampdu_tx->wlc) >= WL_BTC_3WIRE)
		tx_density = AMPDU_DENSITY_8_US;

	ASSERT(tx_density <= AMPDU_MAX_MPDU_DENSITY);
	if (tx_density < 6 && WLC_HT_GET_FRAMEBURST(ampdu_tx->wlc->hti) &&
		RSPEC_IS40MHZ(rspec)) {
		int mcs = rspec & RSPEC_RATE_MASK;
		ASSERT(mcs <= AMPDU_MAX_MCS || mcs == 32 || IS_PROPRIETARY_11N_MCS(mcs));
		if (WLPROPRIETARY_11N_RATES_ENAB(ampdu_tx->wlc->pub)) {
			if (mcs <= AMPDU_MAX_MCS || mcs == 32 || IS_PROPRIETARY_11N_MCS(mcs))
				bytes = scb_ampdu->min_lens[MCS2IDX(mcs)];
		} else {
			if (mcs <= AMPDU_MAX_MCS)
				bytes = scb_ampdu->min_lens[mcs];
		}
	}
	if (bytes == 0) {
		tmp = 1 << (16 - tx_density);
		bytes = CEIL(RSPEC2RATE(rspec), tmp);
	}
	*minbytes = (uint16)bytes;

	if (bytes > phylen) {
		cnt = CEIL(bytes - phylen, AMPDU_DELIMITER_LEN);
		ASSERT(cnt <= 255);
		return (uint8)cnt;
	} else
		return 0;
} /* wlc_ampdu_null_delim_cnt */

void
wlc_ampdu_macaddr_upd(wlc_info_t *wlc)
{
	char template[T_RAM_ACCESS_SZ * 2];

	/* driver needs to write the ta in the template; ta is at offset 16 */
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		bzero(template, sizeof(template));
		bcopy((char*)wlc->pub->cur_etheraddr.octet, template, ETHER_ADDR_LEN);
		wlc_write_template_ram(wlc, (T_BA_TPL_BASE + 16), (T_RAM_ACCESS_SZ * 2), template);
	}
}


#if defined(WLNAR)
/**
 * WLNAR: provides balance amongst MPDU and AMPDU traffic by regulating the number of in-transit
 * packets for non-aggregating stations.
 */

/**
 * wlc_ampdu_ba_on_tidmask() - return a bitmask of aggregating TIDs (priorities).
 *
 * Inputs:
 *	scb	pointer to station control block to examine.
 *
 * Returns:
 *	The return value is a bitmask of TIDs for which a block ack agreement exists.
 *	Bit <00> = TID 0, bit <01> = TID 1, etc. A set bit indicates a BA agreement.
 */
uint8 BCMFASTPATH
wlc_ampdu_ba_on_tidmask(struct scb *scb)
{
	wlc_info_t *wlc;
	uint8 mask = 0;

	/*
	 * If AMPDU_MAX_SCB_TID changes to a larger value, we will need to adjust the
	 * return value from this function, as well as the calling functions.
	 */
	STATIC_ASSERT(AMPDU_MAX_SCB_TID <= NBITS(uint8));

	if (!scb || !SCB_AMPDU(scb) || !SCB_BSSCFG(scb))
		goto exit;

	/* Figure out the wlc this scb belongs to. */
	wlc = SCB_BSSCFG(scb)->wlc;

	if (wlc && wlc->ampdu_tx) {
		bsscfg_ampdu_tx_t *bsscfg_ampdu =
			BSSCFG_AMPDU_TX_CUBBY(wlc->ampdu_tx, SCB_BSSCFG(scb));
	    /* Check that txaggr is not overrided by OFF. */
	    if ((bsscfg_ampdu->txaggr_override != OFF)) {
			/* Locate the ampdu scb cubby, then check all TIDs */
			scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
			ASSERT(wlc->ampdu_tx->txaggr_support);
			if (scb_ampdu) {
			    int i;
			    for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
					scb_ampdu_tid_ini_t *ini = scb_ampdu->ini[i];
					/* Set the TID bit in the mask if the ba state is ON */
					if (ini && (ini->ba_state == AMPDU_TID_STATE_BA_ON)) {
						ASSERT(isbitset(bsscfg_ampdu->txaggr_TID_bmap, i));
						mask |= (1 << i);
					}
				}
			}
	    }
	}

exit:
	return mask;
} /* wlc_ampdu_ba_on_tidmask */

#endif /* WLNAR */

#if defined(WL_DATAPATH_LOG_DUMP)
/**
 * Cubby datapath log callback fn
 * Use EVENT_LOG to dump a summary of the AMPDU datapath state
 * @param ctx   context pointer to ampdu_tx_info_t state structure
 * @param scb   scb of interest for the dump
 * @param tag   EVENT_LOG tag for output
 */
static void
wlc_ampdu_datapath_summary(void *ctx, struct scb *scb, int tag)
{
	int buf_size;
	int i;
	uint prec;
	uint num_prec;
	scb_subq_summary_t *sum;
	scb_ampdu_tx_summary_t *tid_sum;
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)(ctx);
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	osl_t *osh = ampdu_tx->wlc->osh;

	if (!SCB_AMPDU(scb)) {
		/* nothing to report on for AMPDU */
		return;
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	/*
	 * allcate a scb_subq_summary struct to dump ampdu information to the EVENT_LOG
	 */

	/* allocate a size large enough for max precidences */
	buf_size = SCB_SUBQ_SUMMARY_FULL_LEN(PKTQ_MAX_PREC);

	sum = MALLOCZ(osh, buf_size);
	if (sum == NULL) {
		EVENT_LOG(tag,
		          "wlc_ampdu_datapath_summary(): MALLOC %d failed, malloced %d bytes\n",
		          buf_size, MALLOCED(osh));
		/* nothing to do if event cannot be allocated */
		return;
	}

	num_prec = MIN(scb_ampdu->txq.num_prec, PKTQ_MAX_PREC);

	/* Report the multi-prec queue summary */
	sum->id = EVENT_LOG_XTLV_ID_SCBDATA_SUM;
	sum->len = SCB_SUBQ_SUMMARY_FULL_LEN(num_prec) - BCM_XTLV_HDR_SIZE;
	sum->cubby_id = ampdu_tx->cubby_name_id;
	sum->prec_count = num_prec;
	for (prec = 0; prec < num_prec; prec++) {
		sum->plen[prec] = pktqprec_n_pkts(&scb_ampdu->txq, prec);
	}

	EVENT_LOG_BUFFER(tag, (uint8*)sum, sum->len + BCM_XTLV_HDR_SIZE);

	MFREE(osh, sum, buf_size);

	/* allocate the TID xtlv */
	buf_size = sizeof(*tid_sum);

	tid_sum = MALLOCZ(osh, buf_size);
	if (tid_sum == NULL) {
		EVENT_LOG(tag,
		          "wlc_ampdu_datapath_summary(): MALLOC %d failed, malloced %d bytes\n",
		          buf_size, MALLOCED(osh));
		/* nothing to do if event cannot be allocated */
		return;
	}

	/* for each TID, report ampdu specific data summary */
	for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
		if ((ini = scb_ampdu->ini[i]) == NULL) {
			continue;
		}

		/* fill out event log record */
		tid_sum->id = EVENT_LOG_XTLV_ID_SCBDATA_AMPDU_TX_SUM;
		tid_sum->len = buf_size - BCM_XTLV_HDR_SIZE;
		tid_sum->flags = ini->bar_ackpending ? SCBDATA_AMPDU_TX_F_BAR_ACKPEND : 0;
		tid_sum->ba_state = ini->ba_state;
		tid_sum->bar_cnt = ini->ba_state;
		tid_sum->retry_bar = ini->retry_bar;
		tid_sum->bar_ackpending_seq = ini->bar_ackpending_seq;
		tid_sum->barpending_seq = ini->barpending_seq;
		tid_sum->start_seq = ini->start_seq;
		tid_sum->max_seq = ini->max_seq;
#ifdef WLATF
		tid_sum->released_bytes_inflight = AMPDU_ATF_STATE(ini)->released_bytes_inflight;
		tid_sum->released_bytes_target = AMPDU_ATF_STATE(ini)->released_bytes_target;
#endif /* WLATF */
		EVENT_LOG_BUFFER(tag, (uint8*)tid_sum, tid_sum->len + BCM_XTLV_HDR_SIZE);
	}

	MFREE(osh, tid_sum, buf_size);
}
#endif /* WL_DATAPATH_LOG_DUMP */

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMPDU)
#if defined(EVENT_LOG_COMPILE)
int
wlc_ampdu_stats_e_report(uint16 tag, uint16 type, uint16 len, uint32 *counters, bool ec)
{
	wl_ampdu_stats_generic_t ampdu_stats;
	int total_len;

	ASSERT(len <= WL_AMPDU_STATS_MAX_CNTS);

	ampdu_stats.type = type;
	ampdu_stats.len = MIN(len, WL_AMPDU_STATS_MAX_CNTS) * sizeof(uint32);
	memcpy(&ampdu_stats.counters[0], counters, ampdu_stats.len);
	total_len = sizeof(ampdu_stats.type) + sizeof(ampdu_stats.len) + ampdu_stats.len;

#ifdef ECOUNTERS
	if (ec) {
		/* Report using ecounter when requested */
		return ecounters_write(tag, (void *)(&ampdu_stats), total_len);
	}
#endif /* ECOUNTERS */

	/* Report using event log without specified buffer */
	EVENT_LOG_BUFFER(tag, (void *)(&ampdu_stats), total_len);

	return BCME_OK;
}

void
wlc_ampdu_stats_range(uint32 *stats, int max_counters, int *first, int *last)
{
	int i;

	for (i = 0, *first = -1, *last = 0; i < max_counters; i++) {
		if (stats[i] == 0) continue;
		if (*first < 0) {
			*first = i;
		}
		*last = i;
	}
}

int
wlc_ampdu_txmcs_counter_report(ampdu_tx_info_t *ampdu_tx, uint16 tag)
{
	int i, j, first, last;
	int err = BCME_OK;
	const uint16 MCS_RATE_TYPE[4] = {WL_AMPDU_STATS_TYPE_TXMCSx1,
			WL_AMPDU_STATS_TYPE_TXMCSx2,
			WL_AMPDU_STATS_TYPE_TXMCSx3,
			WL_AMPDU_STATS_TYPE_TXMCSx4};
#ifdef WL11AC
	const uint16 VHT_RATE_TYPE[4] = {WL_AMPDU_STATS_TYPE_TXVHTx1,
			WL_AMPDU_STATS_TYPE_TXVHTx2,
			WL_AMPDU_STATS_TYPE_TXVHTx3,
			WL_AMPDU_STATS_TYPE_TXVHTx4};
#endif /* WL11AC */

	wlc_ampdu_stats_range(ampdu_tx->amdbg->txmcs,
		ARRAYSIZE(ampdu_tx->amdbg->txmcs), &first, &last);
	for (i = 0, j = 0; (first >= 0) && (i <= last); i += 8, j++) {
		if (err == BCME_OK && first < (i + 8)) {
			err = wlc_ampdu_stats_e_report(tag,
				MCS_RATE_TYPE[j], 8, &ampdu_tx->amdbg->txmcs[i], FALSE);
		}
	}

#ifdef WL11AC
	wlc_ampdu_stats_range(ampdu_tx->amdbg->txvht,
		ARRAYSIZE(ampdu_tx->amdbg->txvht), &first, &last);
	for (i = 0, j = 0; (first >= 0) && (i <= last); i += 10, j++) {
		if (err == BCME_OK && first < (i + 10)) {
			err = wlc_ampdu_stats_e_report(tag,
				VHT_RATE_TYPE[j], 10, &ampdu_tx->amdbg->txvht[i], FALSE);
		}
	}
#endif /* WL11AC */
	return err;
}

#ifdef ECOUNTERS
int
wlc_ampdu_ecounter_tx_dump(ampdu_tx_info_t *ampdu_tx, uint16 tag)
{
	int mcs_first, mcs_last, len;
#ifdef WL11AC
	int vht_first, vht_last;
#endif
	wl_ampdu_stats_aggrsz_t tx_dist;
	int err = BCME_OK;

	wlc_ampdu_stats_range(ampdu_tx->amdbg->txmcs, ARRAYSIZE(ampdu_tx->amdbg->txmcs),
		&mcs_first, &mcs_last);
#ifdef WL11AC
	wlc_ampdu_stats_range(ampdu_tx->amdbg->txvht, ARRAYSIZE(ampdu_tx->amdbg->txvht),
		&vht_first, &vht_last);
#endif /* WL11AC */

	/* MCS rate */
	if (err == BCME_OK && mcs_first >= 0) {
		err = wlc_ampdu_stats_e_report(tag, WL_AMPDU_STATS_TYPE_TXMCSALL,
			mcs_last + 1, ampdu_tx->amdbg->txmcs, TRUE);
	}
#ifdef WL11AC
	/* VHT rate */
	if (err == BCME_OK && vht_first >= 0) {
		err = wlc_ampdu_stats_e_report(tag, WL_AMPDU_STATS_TYPE_TXVHTALL,
			vht_last + 1, ampdu_tx->amdbg->txvht, TRUE);
	}
#endif /* WL11AC */

#ifdef WLAMPDU_MAC
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		if (err == BCME_OK && mcs_first >= 0) {
			err = wlc_ampdu_stats_e_report(tag, WL_AMPDU_STATS_TYPE_TXMCSOK,
				mcs_last + 1, ampdu_tx->amdbg->txmcs_succ, TRUE);
		}
#ifdef WL11AC
		if (err == BCME_OK && vht_first >= 0) {
			err = wlc_ampdu_stats_e_report(tag, WL_AMPDU_STATS_TYPE_TXVHTOK,
				vht_last + 1, ampdu_tx->amdbg->txvht_succ, TRUE);
		}
	}
#endif /* WL11AC */
#endif /* WLAMPDU_MAC */

	if (err == BCME_OK && mcs_first >= 0) {
		err = wlc_ampdu_stats_e_report(tag, WL_AMPDU_STATS_TYPE_TXMCSSGI,
			mcs_last + 1, ampdu_tx->amdbg->txmcssgi, TRUE);
	}

#ifdef WL11AC
	if (err == BCME_OK && vht_first >= 0) {
		err = wlc_ampdu_stats_e_report(tag, WL_AMPDU_STATS_TYPE_TXVHTSGI,
			vht_last + 1, ampdu_tx->amdbg->txvhtsgi, TRUE);
	}
#endif /* WL11AC */

#if defined(WLAMPDU_HW) || defined(WLAMPDU_AQM)
	/* Update AMPDU histogram when possible */
	if (ampdu_tx->wlc->clk) {
		uint16 stat_addr = 0;
		uint16 i, val, nbins;
		int total_ampdu = 0, total_mpdu = 0;

		nbins = ARRAYSIZE(ampdu_tx->amdbg->mpdu_histogram);
		if (nbins > C_MPDUDEN_NBINS)
			nbins = C_MPDUDEN_NBINS;
		if (AMPDU_HW_ENAB(ampdu_tx->wlc->pub) || AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
			stat_addr = 2 * wlc_read_shm(ampdu_tx->wlc,
				(AMPDU_HW_ENAB(ampdu_tx->wlc->pub)) ?
				M_HWAGG_STATS_PTR(ampdu_tx->wlc) : M_AMP_STATS_PTR(ampdu_tx->wlc));
			for (i = 0; i < nbins; i++) {
				val = wlc_read_shm(ampdu_tx->wlc, stat_addr + i * 2);
				ampdu_tx->amdbg->mpdu_histogram[i] = val;
				total_ampdu += val;
				total_mpdu += (val * (i+1));
			}
			ampdu_tx->cnt->txampdu = total_ampdu;
			WLCNTSET(ampdu_tx->cnt->txmpdu, total_mpdu);
		}
	}
#endif /* WLAMPDU_HW || WLAMPDU_AQM */

	wlc_ampdu_stats_range(ampdu_tx->amdbg->mpdu_histogram,
		ARRAYSIZE(ampdu_tx->amdbg->mpdu_histogram),
		&mcs_first, &mcs_last);
	if (err == BCME_OK && mcs_first >= 0) {
		tx_dist.type = WL_AMPDU_STATS_TYPE_TXDENS;
		tx_dist.len = (mcs_last + 3) * sizeof(uint32);	/* Plus total_ampdu & total_mpdu */
		tx_dist.total_ampdu = ampdu_tx->cnt->txampdu;
#ifdef WLCNT
		tx_dist.total_mpdu = ampdu_tx->cnt->txmpdu;
#else
		tx_dist.total_mpdu = 0;
#endif
		memcpy(&tx_dist.aggr_dist[0], ampdu_tx->amdbg->mpdu_histogram,
			(mcs_last + 1) * sizeof(uint32));

		len = sizeof(tx_dist.type) + sizeof(tx_dist.len) + tx_dist.len;
		BCM_REFERENCE(len);

		/* Report using ecounter when requested */
		err = ecounters_write(tag, (void *)(&tx_dist), len);
	}

	return err;
}
#endif /* ECOUNTERS */
#endif /* EVENT_LOG_COMPILE */

int
wlc_ampdu_tx_dump(ampdu_tx_info_t *ampdu_tx, struct bcmstrbuf *b)
{
#ifdef WLCNT
	wlc_ampdu_tx_cnt_t *cnt = ampdu_tx->cnt;
#endif
	wlc_info_t *wlc = ampdu_tx->wlc;
	wlc_pub_t *pub = wlc->pub;
	int i, last;
	uint32 max_val, total = 0;
	struct scb *scb;
	struct scb_iter scbiter;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	int inic = 0, ini_on = 0, ini_off = 0, ini_pon = 0, ini_poff = 0;
	int nbuf = 0;
#ifdef AMPDU_NON_AQM
	int j;
	wlc_fifo_info_t *fifo;
#endif
	char eabuf[ETHER_ADDR_STR_LEN];
#ifdef  WLOVERTHRUSTER
	wlc_tcp_ack_info_t *tcp_ack_info = &ampdu_tx->tcp_ack_info;
#endif  /* WLOVERTHRUSTER */
#ifdef BCMDBG
	int tid;
#endif

	BCM_REFERENCE(pub);

	bcm_bprintf(b, "HOST_ENAB %d UCODE_ENAB %d 4331_HW_ENAB %d AQM_ENAB %d\n",
		AMPDU_HOST_ENAB(pub), AMPDU_UCODE_ENAB(pub),
		AMPDU_HW_ENAB(pub), AMPDU_AQM_ENAB(pub));

	bcm_bprintf(b, "AMPDU Tx counters:\n");
#ifdef WLAMPDU_MAC
	if (wlc->clk && AMPDU_MAC_ENAB(pub) &&
		!AMPDU_AQM_ENAB(pub)) {

		uint32 hwampdu, hwmpdu, hwrxba, hwnoba;
		hwampdu = wlc_read_shm(wlc, MACSTAT_ADDR(wlc, MCSTOFF_TXAMPDU));
		hwmpdu = wlc_read_shm(wlc, MACSTAT_ADDR(wlc, MCSTOFF_TXMPDU));
		hwrxba = wlc_read_shm(wlc, MACSTAT_ADDR(wlc, MCSTOFF_RXBACK));
		hwnoba = (hwampdu < hwrxba) ? 0 : (hwampdu - hwrxba);
		bcm_bprintf(b, "%s: txampdu %d txmpdu %d txmpduperampdu %d noba %d (%d%%)\n",
			AMPDU_UCODE_ENAB(pub) ? "Ucode" : "HW",
			hwampdu, hwmpdu,
			hwampdu ? CEIL(hwmpdu, hwampdu) : 0, hwnoba,
			hwampdu ? CEIL(hwnoba*100, hwampdu) : 0);
	}
#endif /* WLAMPDU_MAC */

#ifdef WLCNT

#ifdef WLAMPDU_MAC
	if (wlc->clk && AMPDU_MAC_ENAB(pub)) {
		if (!AMPDU_AQM_ENAB(pub))
		bcm_bprintf(b, "txmpdu(enq) %d txs %d txmrt/succ %d %d (f %d%%) "
			"txfbr/succ %d %d (f %d%%)\n", cnt->txmpdu, cnt->u0.txs,
			cnt->tx_mrt, cnt->txsucc_mrt,
			cnt->tx_mrt ? CEIL((cnt->tx_mrt - cnt->txsucc_mrt)*100, cnt->tx_mrt) : 0,
			cnt->tx_fbr, cnt->txsucc_fbr,
			cnt->tx_fbr ? CEIL((cnt->tx_fbr - cnt->txsucc_fbr)*100, cnt->tx_fbr) : 0);
	} else
#endif /* WLAMPDU_MAC */
	{
		bcm_bprintf(b, "txampdu %d txmpdu %d txmpduperampdu %d noba %d (%d%%)\n",
			cnt->txampdu, cnt->txmpdu,
			cnt->txampdu ? CEIL(cnt->txmpdu, cnt->txampdu) : 0, cnt->noba,
			cnt->txampdu ? CEIL(cnt->noba*100, cnt->txampdu) : 0);
		bcm_bprintf(b, "retry_ampdu %d retry_mpdu %d (%d%%) txfifofull %d\n",
		  cnt->txretry_ampdu, cnt->u0.txretry_mpdu,
		  (cnt->txmpdu ? CEIL(cnt->u0.txretry_mpdu*100, cnt->txmpdu) : 0), cnt->txfifofull);
		bcm_bprintf(b, "fbr_ampdu %d fbr_mpdu %d\n",
			cnt->txfbr_ampdu, cnt->txfbr_mpdu);
	}
	bcm_bprintf(b, "txregmpdu %d txreg_noack %d txfifofull %d txdrop %d txstuck %d orphan %d\n",
		cnt->txregmpdu, cnt->txreg_noack, cnt->txfifofull,
		cnt->txdrop, cnt->txstuck, cnt->orphan);
	bcm_bprintf(b, "txrel_wm %d txrel_size %d sduretry %d sdurejected %d\n",
		cnt->txrel_wm, cnt->txrel_size, cnt->sduretry, cnt->sdurejected);

#ifdef WLAMPDU_MAC
	if (AMPDU_MAC_ENAB(pub)) {
		bcm_bprintf(b, "aggfifo_w %d epochdeltas %d mpduperepoch %d\n",
			cnt->enq, cnt->epochdelta,
			(cnt->epochdelta+1) ? CEIL(cnt->enq, (cnt->epochdelta+1)) : 0);
		bcm_bprintf(b, "epoch_change reason: plcp %d rate %d fbr %d reg %d link %d"
			" seq no %d\n", cnt->echgr1, cnt->echgr2, cnt->echgr3,
			cnt->echgr4, cnt->echgr5, cnt->echgr6);
	}
#endif
	bcm_bprintf(b, "txr0hole %d txrnhole %d txrlag %d rxunexp %d\n",
		cnt->txr0hole, cnt->txrnhole, cnt->txrlag, cnt->rxunexp);
	bcm_bprintf(b, "txaddbareq %d rxaddbaresp %d txlost %d txbar %d rxba %d txdelba %d "
		"rxdelba %d \n",
		cnt->txaddbareq, cnt->rxaddbaresp, cnt->txlost, cnt->txbar,
		cnt->rxba, cnt->txdelba, cnt->rxdelba);

#ifdef WLAMPDU_MAC
	if (AMPDU_MAC_ENAB(pub))
		bcm_bprintf(b, "txmpdu_sgi %d txmpdu_stbc %d\n",
		  cnt->u1.txmpdu_sgi, cnt->u2.txmpdu_stbc);
	else
#endif
		bcm_bprintf(b, "txampdu_sgi %d txampdu_stbc %d "
			"txampdu_mfbr_stbc %d\n", cnt->u1.txampdu_sgi,
			cnt->u2.txampdu_stbc, cnt->txampdu_mfbr_stbc);

#ifdef WL_CS_PKTRETRY
	bcm_bprintf(b, "cs_pktretry_cnt %u\n", cnt->cs_pktretry_cnt);
#endif

#endif /* WLCNT */

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
			ASSERT(scb_ampdu);
			for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
				if ((ini = scb_ampdu->ini[i])) {
					inic++;
					if (ini->ba_state == AMPDU_TID_STATE_BA_OFF)
						ini_off++;
					if (ini->ba_state == AMPDU_TID_STATE_BA_ON)
						ini_on++;
					if (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF)
						ini_poff++;
					if (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)
						ini_pon++;
				}
			}
			nbuf += pktq_n_pkts_tot(&scb_ampdu->txq);
		}
	}

	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "ini %d ini_off %d ini_on %d ini_poff %d ini_pon %d nbuf %d ampdu_wds %u\n",
		inic, ini_off, ini_on, ini_poff, ini_pon, nbuf, cnt->ampdu_wds);
#ifdef WLOVERTHRUSTER
	if ((OVERTHRUST_ENAB(pub)) && (tcp_ack_info->tcp_ack_ratio)) {
		bcm_bprintf(b, "tcp_ack_ratio %d/%d total %d/%d dequeued %d multi_dequeued %d\n",
			tcp_ack_info->tcp_ack_ratio, tcp_ack_info->tcp_ack_ratio_depth,
			tcp_ack_info->tcp_ack_total - tcp_ack_info->tcp_ack_dequeued
			- tcp_ack_info->tcp_ack_multi_dequeued,
			tcp_ack_info->tcp_ack_total,
			tcp_ack_info->tcp_ack_dequeued, tcp_ack_info->tcp_ack_multi_dequeued);
	}
#endif /* WLOVERTHRUSTER */

	bcm_bprintf(b, "Supr Reason: pmq(%d) flush(%d) frag(%d) badch(%d) exptime(%d) uf(%d)",
		ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_PMQ],
		ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_FLUSH],
		ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_FRAG],
		ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_BADCH],
		ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_EXPTIME],
		ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_UF]);

#ifdef WLP2P
	if (P2P_ENAB(pub))
		bcm_bprintf(b, " abs(%d)",
			ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_NACK_ABS]);
#endif
#ifdef AP
	bcm_bprintf(b, " pps(%d)",
		ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_PPS]);
#endif
	bcm_bprintf(b, "\n\n");

#ifdef WLAMPDU_HW
	if (wlc->clk && AMPDU_HW_ENAB(pub)) {
		uint16 stat_addr = 2 * wlc_read_shm(wlc, M_HWAGG_STATS_PTR(wlc));
		stat_addr += C_HWAGG_STATS_MPDU_WSZ * 2;
		for (i = 0; i <= AMPDU_HT_MCS_LAST_EL; i++)
			ampdu_tx->amdbg->txmcs[i] = wlc_read_shm(wlc, stat_addr + i * 2);
	}
#endif

	/* determines highest MCS array *index* on which a transmit took place */
	for (i = 0, total = 0, last = 0; i <= AMPDU_HT_MCS_LAST_EL; i++) {
		total += ampdu_tx->amdbg->txmcs[i];
		if (ampdu_tx->amdbg->txmcs[i]) last = i;
	}

	if (last <= AMPDU_MAX_MCS) { /* skips proprietary 11N MCS'es */
		/* round up to highest MCS array index with same number of spatial streams */
		last = 8 * (last / 8 + 1) - 1;
	}

	bcm_bprintf(b, "TX MCS  :");
	if (total) {
		for (i = 0; i <= last; i++) {
			bcm_bprintf(b, "  %d(%d%%)", ampdu_tx->amdbg->txmcs[i],
				(ampdu_tx->amdbg->txmcs[i] * 100) / total);
			if ((i % 8) == 7 && i != last)
				bcm_bprintf(b, "\n        :");
		}
	}
	bcm_bprintf(b, "\n");

#ifdef WLAMPDU_MAC
	bcm_bprintf(b, "MCS PER :");
	if (total) {
		for (i = 0; i <= last; i++) {
			int unacked = 0, per = 0;
			unacked = ampdu_tx->amdbg->txmcs[i] -
				ampdu_tx->amdbg->txmcs_succ[i];
			if (unacked < 0) unacked = 0;
			if ((unacked > 0) && (ampdu_tx->amdbg->txmcs[i]))
				per = (unacked * 100) / ampdu_tx->amdbg->txmcs[i];
			bcm_bprintf(b, "  %d(%d%%)", unacked, per);
			if ((i % 8) == 7 && i != last)
				bcm_bprintf(b, "\n        :");
		}
	}
	bcm_bprintf(b, "\n");
#endif /* WLAMPDU_MAC */

#ifdef WL11AC
	for (i = 0, total = 0, last = 0; i < AMPDU_MAX_VHT; i++) {
		total += ampdu_tx->amdbg->txvht[i];
		if (ampdu_tx->amdbg->txvht[i]) last = i;
	}
	last = MAX_VHT_RATES * (last/MAX_VHT_RATES + 1) - 1;
	bcm_bprintf(b, "TX VHT  :");
	if (total) {
		for (i = 0; i <= last; i++) {
			bcm_bprintf(b, "  %d(%d%%)", ampdu_tx->amdbg->txvht[i],
				(ampdu_tx->amdbg->txvht[i] * 100) / total);
			if ((i % MAX_VHT_RATES) == (MAX_VHT_RATES - 1) && i != last)
				bcm_bprintf(b, "\n        :");
		}
	}
	bcm_bprintf(b, "\n");
#ifdef WLAMPDU_MAC
	if (AMPDU_AQM_ENAB(pub)) {
		bcm_bprintf(b, "VHT PER :");
		if (total) {
			for (i = 0; i <= last; i++) {
				int unacked = 0, per = 0;
				unacked = ampdu_tx->amdbg->txvht[i] -
					ampdu_tx->amdbg->txvht_succ[i];
				if (unacked < 0) unacked = 0;
				if ((unacked > 0) && (ampdu_tx->amdbg->txvht[i]))
					per = (unacked * 100) / ampdu_tx->amdbg->txvht[i];
				bcm_bprintf(b, "  %d(%d%%)", unacked, per);
				if (((i % MAX_VHT_RATES) == (MAX_VHT_RATES - 1)) && (i != last))
					bcm_bprintf(b, "\n        :");
			}
		}
		bcm_bprintf(b, "\n");
	}
#endif /* WLAMPDU_MAC */
#endif /* WL11AC */

#if defined(WLPKTDLYSTAT) && defined(WLCNT)
#if defined(WLAMPDU_HW)
	if (wlc->clk && AMPDU_HW_ENAB(pub)) {
		uint16 stat_addr = 2 * wlc_read_shm(wlc, M_HWAGG_STATS_PTR(wlc));
		stat_addr += C_HWAGG_STATS_MPDU_WSZ * 2;

#if defined(WLPROPRIETARY_11N_RATES)
		i = -1;
		while (TRUE) {
			i = NEXT_MCS(i);
			if (i > WLC_11N_LAST_PROP_MCS)
				break;
#else
		for (i  = 0; i <= AMPDU_MAX_MCS; i++) {
#endif /* WLPROPRIETARY_11N_RATES */
			cnt->txmpdu_cnt[i] = wlc_read_shm(wlc, stat_addr + i * 2);
			/*
			 * cnt->txmpdu_succ_cnt[i] =
			 * 	wlc_read_shm(wlc, stat_addr + i * 2 + C_HWAGG_STATS_TXMCS_WSZ * 2);
			 */
		}
	}
#endif /* WLAMPDU_HW */

	/* Report PER statistics (for each MCS) */
	for (i = 0, max_val = 0, last = 0; i <= AMPDU_HT_MCS_LAST_EL; i++) {
		max_val += cnt->txmpdu_cnt[i];
		if (cnt->txmpdu_cnt[i])
			last = i; /* highest used MCS array index */
	}

	if (last <= AMPDU_MAX_MCS) { /* skips proprietary 11N MCS'es */
		/* round up to highest MCS array index with same number of spatial streams */
		last = 8 * (last / 8 + 1) - 1;
	}

	if (max_val) {
		bcm_bprintf(b, "PER : ");
		for (i = 0; i <= last; i++) {
			int unacked = 0;
			unacked = cnt->txmpdu_cnt[i] - cnt->txmpdu_succ_cnt[i];
			if (unacked < 0) {
				unacked = 0;
			}
			bcm_bprintf(b, "  %d/%d (%d%%)",
			            (cnt->txmpdu_cnt[i] - cnt->txmpdu_succ_cnt[i]),
			            cnt->txmpdu_cnt[i],
			            ((cnt->txmpdu_cnt[i] > 0) ?
			             (unacked * 100) / cnt->txmpdu_cnt[i] : 0));
			if ((i % 8) == 7 && i != last) {
				bcm_bprintf(b, "\n    : ");
			}
		}
		bcm_bprintf(b, "\n");
	}
#endif /* WLPKTDLYSTAT && WLCNT */

#if defined(WLAMPDU_HW) || defined(WLAMPDU_AQM)
	if (wlc->clk) {
		uint16 stat_addr = 0;
		uint16 val, ucode_ampdu;
		int cnt1 = 0, cnt2 = 0;
		uint16 nbins;

		nbins = ARRAYSIZE(ampdu_tx->amdbg->mpdu_histogram);
		if (nbins > C_MPDUDEN_NBINS)
			nbins = C_MPDUDEN_NBINS;
		if (AMPDU_HW_ENAB(pub) || AMPDU_AQM_ENAB(pub)) {
			stat_addr = 2 * wlc_read_shm(wlc,
				(AMPDU_HW_ENAB(pub)) ? M_HWAGG_STATS_PTR(wlc) :
					M_AMP_STATS_PTR(wlc));
			for (i = 0, total = 0; i < nbins; i++) {
				val = wlc_read_shm(wlc, stat_addr + i * 2);
				ampdu_tx->amdbg->mpdu_histogram[i] = val;
				total += val;
				cnt1 += (val * (i+1));
			}
			cnt->txampdu = total;
			WLCNTSET(cnt->txmpdu, cnt1);
			bcm_bprintf(b, "--------------------------\n");
			if (AMPDU_HW_ENAB(pub)) {
				int rempty;
				rempty = wlc_read_shm(wlc, stat_addr + C_HWAGG_RDEMP_WOFF * 2);
				ucode_ampdu = wlc_read_shm(wlc, stat_addr + C_HWAGG_NAMPDU_WOFF*2);
				bcm_bprintf(b, "tot_mpdus %d tot_ampdus %d mpduperampdu %d "
				    "ucode_ampdu %d rempty %d (%d%%)\n",
				    cnt1, total,
				    (total == 0) ? 0 : CEIL(cnt1, total),
				    ucode_ampdu, rempty,
				    (ucode_ampdu == 0) ? 0 : CEIL(rempty * 100, ucode_ampdu));
			} else {
				bcm_bprintf(b, "tot_mpdus %d tot_ampdus %d mpduperampdu %d\n",
					cnt1, total, (total == 0) ? 0 : CEIL(cnt1, total));
			}
		}

		if (AMPDU_AQM_ENAB(pub)) {
			/* print out agg stop reason */
			uint16 stop_len, stop_mpdu, stop_bawin, stop_epoch, stop_fempty;
			stat_addr  += (C_MPDUDEN_NBINS * 2);
			stop_len    = wlc_read_shm(wlc, stat_addr);
			stop_mpdu   = wlc_read_shm(wlc, stat_addr + 2);
			stop_bawin  = wlc_read_shm(wlc, stat_addr + 4);
			stop_epoch  = wlc_read_shm(wlc, stat_addr + 6);
			stop_fempty = wlc_read_shm(wlc, stat_addr + 8);
			total = stop_len + stop_mpdu + stop_bawin + stop_epoch + stop_fempty;
			if (total) {
				bcm_bprintf(b, "agg stop reason: len %d (%d%%) ampdu_mpdu %d (%d%%)"
					    " bawin %d (%d%%) epoch %d (%d%%) fempty %d (%d%%)\n",
					    stop_len, (stop_len * 100) / total,
					    stop_mpdu, (stop_mpdu * 100) / total,
					    stop_bawin, (stop_bawin * 100) / total,
					    stop_epoch, (stop_epoch * 100) / total,
					    stop_fempty, (stop_fempty * 100) / total);
			}
			stat_addr  += C_AGGSTOP_NBINS * 2;
		}

		if (WLC_HT_GET_FRAMEBURST(ampdu_tx->wlc->hti) &&
			(D11REV_IS(pub->corerev, 26) ||
		    D11REV_IS(pub->corerev, 29) || AMPDU_AQM_ENAB(pub))) {
			/* burst size */
			cnt1 = 0;
			if (!AMPDU_AQM_ENAB(pub))
				stat_addr += (C_MBURST_WOFF * 2);
			bcm_bprintf(b, "Frameburst histogram:");
			for (i = 0; i < C_MBURST_NBINS; i++) {
				val = wlc_read_shm(wlc, stat_addr + i * 2);
				cnt1 += val;
				cnt2 += (i+1) * val;
				bcm_bprintf(b, "  %d", val);
			}
			bcm_bprintf(b, " avg %d\n", (cnt1 == 0) ? 0 : CEIL(cnt2, cnt1));
			bcm_bprintf(b, "--------------------------\n");
		}
	}
#endif /* WLAMPDU_HW */

	for (i = 0, last = 0, total = 0; i < AMPDU_MAX_MPDU; i++) {
		total += ampdu_tx->amdbg->mpdu_histogram[i];
		if (ampdu_tx->amdbg->mpdu_histogram[i])
			last = i;
	}
	last = 8 * (last/8 + 1) - 1;
	bcm_bprintf(b, "MPDUdens:");
	for (i = 0; i <= last; i++) {
		bcm_bprintf(b, " %3d (%d%%)", ampdu_tx->amdbg->mpdu_histogram[i],
			(total == 0) ? 0 :
			(ampdu_tx->amdbg->mpdu_histogram[i] * 100 / total));
		if ((i % 8) == 7 && i != last)
			bcm_bprintf(b, "\n        :");
	}
	bcm_bprintf(b, "\n");

	if (AMPDU_HOST_ENAB(pub)) {
		for (i = 0, last = 0; i <= AMPDU_MAX_MPDU; i++)
			if (ampdu_tx->amdbg->retry_histogram[i])
				last = i;
		bcm_bprintf(b, "Retry   :");
		for (i = 0; i <= last; i++) {
			bcm_bprintf(b, " %3d", ampdu_tx->amdbg->retry_histogram[i]);
			if ((i % 8) == 7 && i != last)
				bcm_bprintf(b, "\n        :");
		}
		bcm_bprintf(b, "\n");

		for (i = 0, last = 0; i <= AMPDU_MAX_MPDU; i++)
			if (ampdu_tx->amdbg->end_histogram[i])
				last = i;
		bcm_bprintf(b, "Till End:");
		for (i = 0; i <= last; i++) {
			bcm_bprintf(b, " %3d", ampdu_tx->amdbg->end_histogram[i]);
			if ((i % 8) == 7 && i != last)
				bcm_bprintf(b, "\n        :");
		}
		bcm_bprintf(b, "\n");
	}

	if (WLC_SGI_CAP_PHY(wlc)) {
		bcm_bprintf(b, "TX MCS SGI:");
		for (i = 0, max_val = 0; i <= AMPDU_HT_MCS_LAST_EL; i++) {
			max_val += ampdu_tx->amdbg->txmcssgi[i];
			if (ampdu_tx->amdbg->txmcssgi[i]) last = i;
		}

		if (last <= AMPDU_MAX_MCS) { /* skips proprietary 11N MCS'es */
			/* round up to highest MCS array idx with same number of spatial streams */
			last = 8 * (last / 8 + 1) - 1;
		}

		if (max_val) {
			for (i = 0; i <= last; i++) {
				bcm_bprintf(b, "  %d(%d%%)", ampdu_tx->amdbg->txmcssgi[i],
				            (ampdu_tx->amdbg->txmcssgi[i] * 100) / max_val);
				if ((i % 8) == 7 && i != last)
					bcm_bprintf(b, "\n          :");
			}
		}
		bcm_bprintf(b, "\n");
#ifdef WL11AC
		bcm_bprintf(b, "TX VHT SGI:");
		for (i = 0, max_val = 0; i < AMPDU_MAX_VHT; i++) {
			max_val += ampdu_tx->amdbg->txvhtsgi[i];
			if (ampdu_tx->amdbg->txvhtsgi[i]) last = i;
		}
		last = MAX_VHT_RATES * (last/MAX_VHT_RATES + 1) - 1;
		if (max_val) {
			for (i = 0; i <= last; i++) {
					bcm_bprintf(b, "  %d(%d%%)", ampdu_tx->amdbg->txvhtsgi[i],
						(ampdu_tx->amdbg->txvhtsgi[i] * 100) / max_val);
				if (((i % MAX_VHT_RATES) == (MAX_VHT_RATES - 1)) && i != last)
						bcm_bprintf(b, "\n          :");
			}
		}
		bcm_bprintf(b, "\n");
#endif /* WL11AC */

		if (WLC_STBC_CAP_PHY(wlc)) {
			bcm_bprintf(b, "TX MCS STBC:");
			for (i = 0, max_val = 0; i <= AMPDU_HT_MCS_LAST_EL; i++)
				max_val += ampdu_tx->amdbg->txmcsstbc[i];
			if (max_val) {
				for (i = 0; i <= 7; i++)
					bcm_bprintf(b, "  %d(%d%%)", ampdu_tx->amdbg->txmcsstbc[i],
						(ampdu_tx->amdbg->txmcsstbc[i] * 100) / max_val);
			}
			bcm_bprintf(b, "\n");
#ifdef WL11AC
			for (i = 0, max_val = 0; i < AMPDU_MAX_VHT; i++)
				max_val += ampdu_tx->amdbg->txvhtstbc[i];
			bcm_bprintf(b, "TX VHT STBC:");
			if (max_val) {
				for (i = 0; i < MAX_VHT_RATES; i++) {
					bcm_bprintf(b, "  %d(%d%%)", ampdu_tx->amdbg->txvhtstbc[i],
					(ampdu_tx->amdbg->txvhtstbc[i] * 100) / max_val);
				}
			}
			bcm_bprintf(b, "\n");
#endif /* WL11AC */
		}
	}

#ifdef AMPDU_NON_AQM
	bcm_bprintf(b, "MCS to AMPDU tables:\n");
	for (j = 0; j < NUM_FFPLD_FIFO; j++) {
		fifo = ampdu_tx->config->fifo_tb + j;
		if (fifo->ampdu_pld_size || fifo->dmaxferrate) {
			bcm_bprintf(b, " FIFO %d: Preload settings: size %d dmarate %d kbps\n",
			          j, fifo->ampdu_pld_size, fifo->dmaxferrate);
			bcm_bprintf(b, "       :");
			for (i = 0; i <= AMPDU_HT_MCS_LAST_EL; i++) {
				bcm_bprintf(b, " %d", fifo->mcs2ampdu_table[i]);
				if ((i % 8) == 7 && i != AMPDU_HT_MCS_LAST_EL)
					bcm_bprintf(b, "\n       :");
			}
			bcm_bprintf(b, "\n");
		}
	}
	bcm_bprintf(b, "\n");
#endif /* AMPDU_NON_AQM */

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
			bcm_bprintf(b, "%s: max_pdu %d release %d\n",
				bcm_ether_ntoa(&scb->ea, eabuf),
				scb_ampdu->max_pdu, scb_ampdu->release);
#ifdef BCMDBG
			bcm_bprintf(b, "\ttxdrop %u txstuck %u "
				  "txaddbareq %u txrlag %u sdurejected %u\n"
				  "\ttxmpdu %u txlost %u txbar %d txreg_noack %u noba %u "
				  "rxaddbaresp %u\n",
				  scb_ampdu->cnt.txdrop, scb_ampdu->cnt.txstuck,
				  scb_ampdu->cnt.txaddbareq, scb_ampdu->cnt.txrlag,
				  scb_ampdu->cnt.sdurejected, scb_ampdu->cnt.txmpdu,
				  scb_ampdu->cnt.txlost, scb_ampdu->cnt.txbar,
				  scb_ampdu->cnt.txreg_noack, scb_ampdu->cnt.noba,
				  scb_ampdu->cnt.rxaddbaresp);
			bcm_bprintf(b, "\ttxnoroom %u\n", scb_ampdu->cnt.txnoroom);
			for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
				ini = scb_ampdu->ini[tid];
				if (!ini)
					continue;
				if (!(ini->ba_state == AMPDU_TID_STATE_BA_ON))
					continue;
				bcm_bprintf(b, "\tba_state %d ba_wsize %d tx_in_transit %d "
					"tid %d rem_window %d\n",
					ini->ba_state, ini->ba_wsize, ini->tx_in_transit,
					ini->tid, ini->rem_window);
				bcm_bprintf(b, "\tstart_seq 0x%x max_seq 0x%x tx_exp_seq 0x%x"
					" bar_ackpending_seq 0x%x\n",
					ini->start_seq, ini->max_seq, ini->tx_exp_seq,
					ini->bar_ackpending_seq);
				bcm_bprintf(b, "\tbar_ackpending %d alive %d retry_bar %d\n",
					ini->bar_ackpending, ini->alive, ini->retry_bar);
				wlc_ampdu_atf_dump(AMPDU_ATF_STATE(ini), b);
			}
#endif /* BCMDBG */
		}
#ifdef WLPKTDLYSTAT
		wlc_scb_dlystat_dump(scb, b);
#endif /* WLPKTDLYSTAT */
	}

#ifdef WLAMPDU_HW
	if (wlc->clk && AMPDU_HW_ENAB(pub) && b) {
		d11regs_t* regs = wlc->regs;
		bcm_bprintf(b, "AGGFIFO regs: availcnt 0x%x\n", R_REG(wlc->osh, &regs->aggfifocnt));
		bcm_bprintf(b, "cmd 0x%04x stat 0x%04x cfgctl 0x%04x cfgdata 0x%04x mpdunum 0x%02x "
			"len 0x%04x bmp 0x%04x ackedcnt %d\n",
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_cmd),
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_stat),
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_cfgctl),
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_cfgdata),
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_mpdunum),
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_len),
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_bmp),
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_ackedcnt));
#if defined(WLCNT) && defined(WLAMPDU_MAC)
		bcm_bprintf(b, "driver statistics: aggfifo pending %d enque/cons %d %d",
			cnt->pending, cnt->enq, cnt->cons);
#endif
	}
#endif /* WLAMPDU_HW */
	bcm_bprintf(b, "\n");

	return 0;
} /* wlc_ampdu_tx_dump */

#endif 

#ifdef BCMDBG
static void
ampdu_dump_ini(scb_ampdu_tid_ini_t *ini)
{
	printf("ba_state %d ba_wsize %d tx_in_transit %d tid %d rem_window %d\n",
		ini->ba_state, ini->ba_wsize, ini->tx_in_transit, ini->tid, ini->rem_window);
	printf("start_seq 0x%x max_seq 0x%x tx_exp_seq 0x%x bar_ackpending_seq 0x%x\n",
		ini->start_seq, ini->max_seq, ini->tx_exp_seq, ini->bar_ackpending_seq);
	printf("bar_ackpending %d alive %d retry_bar %d\n",
		ini->bar_ackpending, ini->alive, ini->retry_bar);
	printf("retry_head %d retry_tail %d retry_cnt %d\n",
		ini->retry_head, ini->retry_tail, ini->retry_cnt);
	prhex("ackpending", &ini->ackpending[0], sizeof(ini->ackpending));
	prhex("barpending", &ini->barpending[0], sizeof(ini->barpending));
	prhex("txretry", &ini->txretry[0], sizeof(ini->txretry));
	prhex("retry_seq", (uint8 *)&ini->retry_seq[0], sizeof(ini->retry_seq));
}
#endif /* BCMDBG */

#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */
/**
 * Sidechannel is a firmware<->ucode interface, intended to prepare the ucode for upcoming
 * transmits, so ucode can allocate data structures.
 */
void
wlc_sidechannel_init(wlc_info_t *wlc)
{
	ampdu_tx_info_t *ampdu_tx = wlc->ampdu_tx;
#ifdef WLAMPDU_UCODE
	uint16 txfs_waddr; /* side channel base addr in 16 bit units */
	uint16 txfsd_addr; /* side channel descriptor base addr */
	/* Size of side channel fifos in 16 bit units */
	uint8 txfs_wsz[AC_COUNT] =
	        {TXFS_WSZ_AC_BK, TXFS_WSZ_AC_BE, TXFS_WSZ_AC_VI, TXFS_WSZ_AC_VO};
	ampdumac_info_t *hagg;
	int i;
#endif /* WLAMPDU_UCODE */
	(void)wlc;
	WL_TRACE(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	if (!AMPDU_MAC_ENAB(wlc->pub) || AMPDU_AQM_ENAB(wlc->pub)) {
		WL_INFORM(("wl%d: %s; NOT UCODE or HW aggregation or side channel"
			"not supported\n", wlc->pub->unit, __FUNCTION__));
		return;
	}

	ASSERT(ampdu_tx);
	BCM_REFERENCE(ampdu_tx);

#ifdef WLAMPDU_UCODE
	if (AMPDU_UCODE_ENAB(wlc->pub)) {
		if (!(ampdu_tx->config->ini_enable[PRIO_8021D_VO] ||
			ampdu_tx->config->ini_enable[PRIO_8021D_NC])) {
			txfs_wsz[1] = txfs_wsz[1] + txfs_wsz[3];
			txfs_wsz[3] = 0;
		}
		if (!(ampdu_tx->config->ini_enable[PRIO_8021D_NONE] ||
			ampdu_tx->config->ini_enable[PRIO_8021D_BK])) {
				txfs_wsz[1] = txfs_wsz[1] + txfs_wsz[0];
				txfs_wsz[0] = 0;
		}
		ASSERT(txfs_wsz[0] + txfs_wsz[1] + txfs_wsz[2] + txfs_wsz[3] <= TOT_TXFS_WSIZE);
		txfs_waddr = wlc_read_shm(wlc, M_TXFS_PTR(wlc));
		txfsd_addr = (txfs_waddr + C_TXFSD_WOFFSET) * 2;
		for (i = 0; i < AC_COUNT; i++) {
			/* 16 bit word arithmetic */
			hagg = &(ampdu_tx->hagg[i]);
			hagg->txfs_addr_strt = txfs_waddr;
			hagg->txfs_addr_end = txfs_waddr + txfs_wsz[i] - 1;
			hagg->txfs_wptr = hagg->txfs_addr_strt;
			hagg->txfs_rptr = hagg->txfs_addr_strt;
			hagg->txfs_wsize = txfs_wsz[i];
			/* write_shmem takes a 8 bit address and 16 bit pointer */
			wlc_write_shm(wlc, C_TXFSD_STRT_POS(txfsd_addr, i),
				hagg->txfs_addr_strt);
			wlc_write_shm(wlc, C_TXFSD_END_POS(txfsd_addr, i),
				hagg->txfs_addr_end);
			wlc_write_shm(wlc, C_TXFSD_WPTR_POS(txfsd_addr, i), hagg->txfs_wptr);
			wlc_write_shm(wlc, C_TXFSD_RPTR_POS(txfsd_addr, i), hagg->txfs_rptr);
			wlc_write_shm(wlc, C_TXFSD_RNUM_POS(txfsd_addr, i), 0);
			WL_AMPDU_HW(("%d: start 0x%x 0x%x, end 0x%x 0x%x, w 0x%x 0x%x,"
				" r 0x%x 0x%x, sz 0x%x 0x%x\n",
				i,
				C_TXFSD_STRT_POS(txfsd_addr, i), hagg->txfs_addr_strt,
				C_TXFSD_END_POS(txfsd_addr, i),  hagg->txfs_addr_end,
				C_TXFSD_WPTR_POS(txfsd_addr, i), hagg->txfs_wptr,
				C_TXFSD_RPTR_POS(txfsd_addr, i), hagg->txfs_rptr,
				C_TXFSD_RNUM_POS(txfsd_addr, i), 0));
			hagg->txfs_wptr_addr = C_TXFSD_WPTR_POS(txfsd_addr, i);
			txfs_waddr += txfs_wsz[i];
		}
	}
#endif /* WLAMPDU_UCODE */

#ifdef WLAMPDU_HW
	if (AMPDU_HW_ENAB(wlc->pub)) {
		ampdu_tx->config->aggfifo_depth = AGGFIFO_CAP - 1;
	}
#endif /* WLAMPDU_HW */

#ifdef WLCNT
	ampdu_tx->cnt->pending = 0;
	ampdu_tx->cnt->cons = 0;
	ampdu_tx->cnt->enq = 0;
#endif /* WLCNT */

} /* wlc_sidechannel_init */

#ifdef AMPDU_NON_AQM

#if !defined(TXQ_MUX)
/**
 * For ucode aggregation: enqueue an entry to side channel (a.k.a. aggfifo) and inc the wptr \
 * For hw agg: it simply enqueues an entry to aggfifo.
 *
 * Only called for non-AC (non_aqm) chips.
 *
 * The actual capacity of the buffer is one less than the actual length
 * of the buffer so that an empty and a full buffer can be
 * distinguished.  An empty buffer will have the readPostion and the
 * writePosition equal to each other.  A full buffer will have
 * the writePosition one less than the readPostion.
 *
 * The Objects available to be read go from the readPosition to the writePosition,
 * wrapping around the end of the buffer.  The space available for writing
 * goes from the write position to one less than the readPosition,
 * wrapping around the end of the buffer.
 */
static int
aggfifo_enque(ampdu_tx_info_t *ampdu_tx, int length, int qid)
{
	ampdumac_info_t *hagg = &(ampdu_tx->hagg[qid]);
#if defined(WLAMPDU_HW) || defined(WLAMPDU_UCODE)
	uint16 epoch = hagg->epoch;
	uint32 entry;

	if (length > (MPDU_LEN_MASK >> MPDU_LEN_SHIFT)) {
		WL_AMPDU_ERR(("%s: Length too long %d\n", __FUNCTION__, length));
	}
#endif

#ifdef WLAMPDU_HW
	if (AMPDU_HW_ENAB(ampdu_tx->wlc->pub)) {
		d11regs_t *regs = ampdu_tx->wlc->regs;

		entry = length |
			((epoch ? 1 : 0) << MPDU_EPOCH_HW_SHIFT) | (qid << MPDU_FIFOSEL_SHIFT);

		W_REG(ampdu_tx->wlc->osh, &regs->aggfifodata, entry);
		WLCNTINCR(ampdu_tx->cnt->enq);
		WLCNTINCR(ampdu_tx->cnt->pending);
		hagg->in_queue ++;
		WL_AMPDU_HW(("%s: aggfifo %d entry 0x%04x\n", __FUNCTION__, qid, entry));
	}
#endif /* WLAMPDU_HW */

#ifdef WLAMPDU_UCODE
	if (AMPDU_UCODE_ENAB(ampdu_tx->wlc->pub)) {
		uint16 rptr = hagg->txfs_rptr;
		uint16 wptr_next;

		entry = length | (((epoch ? 1 : 0)) << MPDU_EPOCH_SHIFT);

		/* wptr always points to the next available entry to be written */
		if (hagg->txfs_wptr == hagg->txfs_addr_end) {
			wptr_next = hagg->txfs_addr_strt;	/* wrap */
		} else {
			wptr_next = hagg->txfs_wptr + 1;
		}

		if (wptr_next == rptr) {
			WL_ERROR(("%s: side channel %d is full !!!\n", __FUNCTION__, qid));
			ASSERT(0);
			return -1;
		}

		/* Convert word addr to byte addr */
		wlc_write_shm(ampdu_tx->wlc, hagg->txfs_wptr * 2, entry);

		WL_AMPDU_HW(("%s: aggfifo %d rptr 0x%x wptr 0x%x entry 0x%04x\n",
			__FUNCTION__, qid, rptr, hagg->txfs_wptr, entry));

		hagg->txfs_wptr = wptr_next;
		wlc_write_shm(ampdu_tx->wlc, hagg->txfs_wptr_addr, hagg->txfs_wptr);
	}
#endif /* WLAMPDU_UCODE */

	WLCNTINCR(ampdu_tx->cnt->enq);
	WLCNTINCR(ampdu_tx->cnt->pending);
	hagg->in_queue ++;
	return 0;
} /* aggfifo_enque */
#endif /* TXQ_MUX */

#if defined(BCMDBG) || !defined(TXQ_MUX)
/**
 * Called for non AC chips only.
 * For ucode agg:
 *     Side channel queue is setup with a read and write ptr.
 *     - R == W means empty.
 *     - (W + 1 % size) == R means full.
 *     - Max buffer capacity is size - 1 elements (one element remains unused).
 * For hw agg:
 *     simply read ihr reg
 */
static uint
get_aggfifo_space(ampdu_tx_info_t *ampdu_tx, int qid)
{
	uint ret = 0;

#ifdef WLAMPDU_HW
	if (AMPDU_HW_ENAB(ampdu_tx->wlc->pub)) {
		uint actual;
		d11regs_t *regs = ampdu_tx->wlc->regs;
		ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;

		ret = R_REG(ampdu_tx->wlc->osh, &regs->aggfifocnt);
		switch (qid) {
			case 3:
				ret >>= 8;
			case 2:
				ret >>= 8;
			case 1:
				ret >>= 8;
			case 0:
				ret &= 0x7f;
				break;
			default:
				ASSERT(0);
		}
		actual = ret;
		BCM_REFERENCE(actual);

		if (ret >= (uint)(AGGFIFO_CAP - ampdu_tx_cfg->aggfifo_depth))
			ret -= (AGGFIFO_CAP - ampdu_tx_cfg->aggfifo_depth);
		else
			ret = 0;

		/* due to the txstatus can only hold 32 bits in bitmap, limit the size to 32 */
		WL_AMPDU_HW(("%s: fifo %d fifo availcnt %d ret %d\n", __FUNCTION__, qid, actual,
			ret));
	}
#endif	/* WLAMPDU_HW */
#ifdef WLAMPDU_UCODE
	if (AMPDU_UCODE_ENAB(ampdu_tx->wlc->pub)) {
		if (ampdu_tx->hagg[qid].txfs_wptr < ampdu_tx->hagg[qid].txfs_rptr)
			ret = ampdu_tx->hagg[qid].txfs_rptr - ampdu_tx->hagg[qid].txfs_wptr - 1;
		else
			ret = (ampdu_tx->hagg[qid].txfs_wsize - 1) -
			      (ampdu_tx->hagg[qid].txfs_wptr - ampdu_tx->hagg[qid].txfs_rptr);
		ASSERT(ret < ampdu_tx->hagg[qid].txfs_wsize);
		WL_AMPDU_HW(("%s: fifo %d rptr %04x wptr %04x availcnt %d\n", __FUNCTION__,
			qid, ampdu_tx->hagg[qid].txfs_rptr, ampdu_tx->hagg[qid].txfs_wptr, ret));
	}
#endif /* WLAMPDU_UCODE */
	return ret;
} /* get_aggfifo_space */
#endif /* BCMDBG) || !TXQ_MUX */

#endif /* AMPDU_NON_AQM */
#endif /* WLAMPDU_MAC */


#ifdef WLAMPDU_PRECEDENCE

/** returns TRUE on queueing succeeded */
static bool BCMFASTPATH
wlc_ampdu_prec_enq(wlc_info_t *wlc, struct pktq *q, void *pkt, int tid)
{
	return wlc_prec_enq_head(wlc, q, pkt, WLAMPDU_PREC_TID2PREC(pkt, tid), FALSE);
}

static void * BCMFASTPATH
wlc_ampdu_pktq_pdeq(struct pktq *pq, int tid)
{
	void *p;

	p = pktq_pdeq(pq, WLC_PRIO_TO_HI_PREC(tid));

	if (p == NULL)
		p = pktq_pdeq(pq, WLC_PRIO_TO_PREC(tid));

	return p;
}

static void
wlc_ampdu_pktq_pflush(wlc_info_t *wlc, struct pktq *pq, int tid)
{
	wlc_txq_pktq_pflush(wlc, pq, WLC_PRIO_TO_HI_PREC(tid));
	wlc_txq_pktq_pflush(wlc, pq, WLC_PRIO_TO_PREC(tid));
}

static void
*ampdu_pktqprec_peek(struct pktq *pq, int tid)
{
	if (pktqprec_peek(pq, WLC_PRIO_TO_HI_PREC(tid)))
		return pktqprec_peek(pq, WLC_PRIO_TO_HI_PREC(tid));
	else
		return pktqprec_peek(pq, WLC_PRIO_TO_PREC(tid));
}
#endif /* WLAMPDU_PRECEDENCE */

#ifdef PROP_TXSTATUS
void wlc_ampdu_flush_pkts(wlc_info_t *wlc, struct scb *scb, uint8 tid)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
	wlc_ampdu_pktq_pflush(wlc, &scb_ampdu->txq, tid);
}

void
wlc_ampdu_flush_flowid_pkts(wlc_info_t *wlc, struct scb *scb, uint16 flowid)
{
	uint8 tid;
	scb_ampdu_tid_ini_t *ini;
	scb_ampdu_tx_t *scb_ampdu;
	void *p = NULL;
	scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
	for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
		ini = scb_ampdu->ini[tid];
		if (!ini)
			continue;
		if (ampdu_pktqprec_n_pkts(&scb_ampdu->txq, tid)) {
			p = ampdu_pktqprec_peek(&scb_ampdu->txq, tid);
			/* All packets in the queue have the same flowid */
			if (p && flowid == PKTFRAGFLOWRINGID(wlc->osh, p)) {
				wlc_ampdu_flush_pkts(wlc, scb, tid);
				continue;
			}
		}
	}
}

#endif /* PROP_TXSTATUS */

#if defined(BCMDBG) || defined(WLPKTDLYSTAT) || defined(BCMDBG_AMPDU)
#ifdef WLCNT
void
wlc_ampdu_clear_tx_dump(ampdu_tx_info_t *ampdu_tx)
{
#if defined(BCMDBG) || defined(WLPKTDLYSTAT)
	struct scb *scb;
	struct scb_iter scbiter;
#endif /* BCMDBG || WLPKTDLYSTAT */
#ifdef BCMDBG
	scb_ampdu_tx_t *scb_ampdu;
#endif /* BCMDBG */

	/* zero the counters */
	bzero(ampdu_tx->cnt, sizeof(wlc_ampdu_tx_cnt_t));
	/* reset the histogram as well */
	if (ampdu_tx->amdbg) {
		bzero(ampdu_tx->amdbg->retry_histogram, sizeof(ampdu_tx->amdbg->retry_histogram));
		bzero(ampdu_tx->amdbg->end_histogram, sizeof(ampdu_tx->amdbg->end_histogram));
		bzero(ampdu_tx->amdbg->mpdu_histogram, sizeof(ampdu_tx->amdbg->mpdu_histogram));
		bzero(ampdu_tx->amdbg->supr_reason, sizeof(ampdu_tx->amdbg->supr_reason));
		bzero(ampdu_tx->amdbg->txmcs, sizeof(ampdu_tx->amdbg->txmcs));
		bzero(ampdu_tx->amdbg->txmcssgi, sizeof(ampdu_tx->amdbg->txmcssgi));
		bzero(ampdu_tx->amdbg->txmcsstbc, sizeof(ampdu_tx->amdbg->txmcsstbc));
#ifdef WLAMPDU_MAC
		bzero(ampdu_tx->amdbg->txmcs_succ, sizeof(ampdu_tx->amdbg->txmcs_succ));
#endif

#ifdef WL11AC
		bzero(ampdu_tx->amdbg->txvht, sizeof(ampdu_tx->amdbg->txvht));
		bzero(ampdu_tx->amdbg->txvhtsgi, sizeof(ampdu_tx->amdbg->txvhtsgi));
		bzero(ampdu_tx->amdbg->txvhtstbc, sizeof(ampdu_tx->amdbg->txvhtstbc));
#ifdef WLAMPDU_MAC
		bzero(ampdu_tx->amdbg->txvht_succ, sizeof(ampdu_tx->amdbg->txvht_succ));
#endif
#endif /* WL11AC */
	}
#ifdef WLOVERTHRUSTER
	ampdu_tx->tcp_ack_info.tcp_ack_total = 0;
	ampdu_tx->tcp_ack_info.tcp_ack_dequeued = 0;
	ampdu_tx->tcp_ack_info.tcp_ack_multi_dequeued = 0;
#endif /* WLOVERTHRUSTER */
#if defined(BCMDBG) || defined(WLPKTDLYSTAT)
#ifdef WLPKTDLYSTAT
		/* reset the per-SCB delay statistics */
		wlc_dlystats_clear(ampdu_tx->wlc);
#endif /* WLPKTDLYSTAT */
	FOREACHSCB(ampdu_tx->wlc->scbstate, &scbiter, scb) {
#ifdef BCMDBG
		if (SCB_AMPDU(scb)) {
			/* reset the per-SCB statistics */
			scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
			bzero(&scb_ampdu->cnt, sizeof(scb_ampdu_cnt_tx_t));
		}
#endif /* BCMDBG */
	}
#endif /* BCMDBG || WLPKTDLYSTAT */

#ifdef WLAMPDU_MAC
	if (ampdu_tx->wlc->clk) {
#ifdef WLAMPDU_UCODE
		if (ampdu_tx->amdbg) {
			bzero(ampdu_tx->amdbg->schan_depth_histo,
				sizeof(ampdu_tx->amdbg->schan_depth_histo));
		}
#endif
	/* zero out shmem counters */
		if (AMPDU_MAC_ENAB(ampdu_tx->wlc->pub) && !AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
			wlc_write_shm(ampdu_tx->wlc,
				MACSTAT_ADDR(ampdu_tx->wlc, MCSTOFF_TXMPDU), 0);
			wlc_write_shm(ampdu_tx->wlc,
				MACSTAT_ADDR(ampdu_tx->wlc, MCSTOFF_TXAMPDU), 0);
		}

#if defined(WLAMPDU_HW) || defined(WLAMPDU_AQM)
		if (AMPDU_HW_ENAB(ampdu_tx->wlc->pub)) {
			if (D11REV_IS(ampdu_tx->wlc->pub->corerev, 26) ||
				D11REV_IS(ampdu_tx->wlc->pub->corerev, 29) ||
				D11REV_IS(ampdu_tx->wlc->pub->corerev, 37)) {
				int i;
				uint16 stat_addr = 2 * wlc_read_shm(ampdu_tx->wlc,
					M_HWAGG_STATS_PTR(ampdu_tx->wlc));
				for (i = 0; i < (C_MBURST_WOFF + C_MBURST_NBINS); i++)
					wlc_write_shm(ampdu_tx->wlc, stat_addr + i * 2, 0);
			}
		} else if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
			int i;
			uint16 stat_addr = 2 * wlc_read_shm(ampdu_tx->wlc,
				M_AMP_STATS_PTR(ampdu_tx->wlc));
			for (i = 0; i < C_AMP_STATS_SIZE; i++)
				wlc_write_shm(ampdu_tx->wlc, stat_addr + i * 2, 0);
		}
#endif /* WLAMPDU_HW */
	}
#endif /* WLAMPDU_MAC */
} /* wlc_ampdu_clear_tx_dump */

#endif /* WLCNT */
#endif 

void
wlc_ampdu_tx_send_delba(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 tid,
	uint16 initiator, uint16 reason)
{
	wlc_info_t *wlc = ampdu_tx->wlc;

	ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, FALSE);

	WL_ERROR(("wl%d: %s: tid %d initiator %d reason %d\n",
		wlc->pub->unit, __FUNCTION__, tid, initiator, reason));

	wlc_send_delba(wlc, scb, tid, initiator, reason);

	WLCNTINCR(ampdu_tx->cnt->txdelba);
}

void
wlc_ampdu_tx_recv_delba(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 tid, uint8 category,
	uint16 initiator, uint16 reason)
{
	scb_ampdu_tx_t *scb_ampdu_tx;

	ASSERT(scb);

	scb_ampdu_tx = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	BCM_REFERENCE(scb_ampdu_tx);
	BCM_REFERENCE(initiator);
	BCM_REFERENCE(reason);
	ASSERT(scb_ampdu_tx);

	if (category & DOT11_ACTION_CAT_ERR_MASK) {
		WL_AMPDU_ERR(("wl%d: %s: unexp error action frame\n",
			ampdu_tx->wlc->pub->unit, __FUNCTION__));
		WLCNTINCR(ampdu_tx->cnt->rxunexp);
		return;
	}

	ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, FALSE);

	WLCNTINCR(ampdu_tx->cnt->rxdelba);
	AMPDUSCBCNTINCR(scb_ampdu_tx->cnt.rxdelba);

	WL_AMPDU_ERR(("wl%d: %s: AMPDU OFF: tid %d initiator %d reason %d\n",
		ampdu_tx->wlc->pub->unit, __FUNCTION__, tid, initiator, reason));
}

#ifdef WLAMPDU_MAC
void wlc_ampdu_upd_pm(wlc_info_t *wlc, uint8 PM_mode)
{
	if (PM_mode == 1)
		wlc->ampdu_tx->aqm_max_release[PRIO_8021D_BE] = AMPDU_AQM_RELEASE_DEFAULT;
	else
		wlc->ampdu_tx->aqm_max_release[PRIO_8021D_BE] = AMPDU_AQM_RELEASE_MAX;
}
#endif /* WLAMPDU_MAC */

/**
 * This function moves the window for the pkt freed during DMA TX reclaim
 * for which status is not received till flush for channel switch
 */
static void
wlc_ampdu_ini_adjust(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini, void *pkt)
{
	uint16 seq = WLPKTTAG(pkt)->seq;

#ifdef PROP_TXSTATUS
	seq = WL_SEQ_GET_NUM(seq);
#endif /* PROP_TXSTATUS */

#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(ampdu_tx->wlc->pub) &&
		WLFC_GET_REUSESEQ(wlfc_query_mode(ampdu_tx->wlc->wlfc))) {
#ifdef PROP_TXSTATUS_SUPPR_WINDOW
		/* adjust the pkts in transit during flush */
		/* For F/W packets clear vectors */
		if (AMPDU_HOST_ENAB(ampdu_tx->wlc->pub) &&
			!(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(p)->wl_hdr_information) &
			WLFC_PKTFLAG_PKTFROMHOST)) {
				uint16 indx = TX_SEQ_TO_INDEX(seq);
				if (isset(ini->ackpending, indx)) {
					clrbit(ini->ackpending, indx);
					if (isset(ini->barpending, indx)) {
						clrbit(ini->barpending, indx);
					}
				}
		} else { /* Suppresed packets */
			ini->suppr_window++;
		}
#endif /* PROP_TXSTATUS_SUPPR_WINDOW */
		/* clear retry array on return to host packets */
		if (AMPDU_HOST_ENAB(ampdu_tx->wlc->pub)) {
			uint16 indx = TX_SEQ_TO_INDEX(seq);
			if (ini->retry_cnt > 0) {
				ini->txretry[indx] = 0;
				ini->retry_tail = PREV_TX_INDEX(ini->retry_tail);
				ini->retry_seq[ini->retry_tail] = 0;
				ini->retry_cnt--;
			}
		}
		return;
	}
#endif /* PROP_TXSTATUS */

	if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		uint16 index = TX_SEQ_TO_INDEX(seq);
		if (!isset(ini->ackpending, index)) {
			return;
		}
		setbit(ini->barpending, index);
	}
	else if (MODSUB_POW2(ini->max_seq, seq, SEQNUM_MAX) < ini->ba_wsize) {
		uint16 bar_seq = MODINC_POW2(seq, SEQNUM_MAX);
		/* check if there is another bar with advence seq no */
		if (!ini->bar_ackpending ||
		    IS_SEQ_ADVANCED(bar_seq, ini->barpending_seq)) {
			WL_AMPDU(("%s: set barpending_seq ini->bar_ackpending:%d bar_seq:%d "
			"ini->barpending_seq:%d seq:%d ini->max_seq:%d ini->ba_wsize:%d\n",
			__FUNCTION__, ini->bar_ackpending, bar_seq, ini->barpending_seq, seq,
			          ini->max_seq, ini->ba_wsize));
			ini->barpending_seq = bar_seq;
			/* Also update the exp seq number if required */
			ini->tx_exp_seq = ini->barpending_seq;
		} else  {
			WL_AMPDU(("%s:cannot set barpending_seq:"
			          "ini->bar_ackpending:%d bar_seq:%d"
			          "ini->barpending_seq:%d", __FUNCTION__,
			          ini->bar_ackpending, bar_seq, ini->barpending_seq));
		}
	}
} /* wlc_ampdu_ini_adjust */

#ifdef PROP_TXSTATUS
void wlc_ampdu_flush_ampdu_q(ampdu_tx_info_t *ampdu, wlc_bsscfg_t *cfg)
{
	struct scb_iter scbiter;
	struct scb *scb = NULL;

	FOREACHSCB(ampdu->wlc->scbstate, &scbiter, scb) {
		if (scb->bsscfg == cfg) {
			if (!SCB_AMPDU(scb))
				continue;
#ifdef WLP2P
			{
			scb_ampdu_tx_t *scb_ampdu;
			scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu, scb);
			wlc_wlfc_flush_queue(ampdu->wlc, &scb_ampdu->txq);
			}
#endif
			wlc_check_ampdu_fc(ampdu, scb);
		}
	}

}

void wlc_ampdu_send_bar_cfg(ampdu_tx_info_t * ampdu, struct scb *scb)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint32 pktprio;
	if (!scb || !SCB_AMPDU(scb))
		return;

	/* when reuse seq, no need to send bar */
	if (WLFC_GET_REUSESEQ(wlfc_query_mode(ampdu->wlc->wlfc))) {
		return;
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu, scb);

	for (pktprio = 0; pktprio <  AMPDU_MAX_SCB_TID; pktprio++) {
		ini = scb_ampdu->ini[pktprio];
		if (!ini) {
			continue;
		}
		/* We have no bitmap for AQM so use barpending_seq */
		if ((!AMPDU_AQM_ENAB(ampdu->wlc->pub)) || (ini->barpending_seq != SEQNUM_MAX))
			wlc_ampdu_send_bar(ampdu, ini, FALSE);
	}
}

#endif /* PROP_TXSTATUS */

/** returns tx queue to use for a caller supplied scb (= one remote party) */
struct pktq* wlc_ampdu_txq(ampdu_tx_info_t *ampdu, struct scb *scb)
{
	scb_ampdu_tx_t *scb_ampdu;
	if (!scb || !SCB_AMPDU(scb))
		return NULL;
	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu, scb);
	return &scb_ampdu->txq;
}

/** PROP_TXSTATUS, flow control related */
void wlc_check_ampdu_fc(ampdu_tx_info_t *ampdu, struct scb *scb)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	int i;

	if (!scb || !SCB_AMPDU(scb)) {
		return;
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu, scb);
	if (!scb_ampdu) {
		return;
	}

	for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
		ini = scb_ampdu->ini[i];
		if (ini) {
			wlc_ampdu_txflowcontrol(ampdu->wlc, scb_ampdu, ini);
		}
	}
}

#ifdef BCMDBG
struct wlc_ampdu_txq_prof_entry {
	uint32 timestamp;
	struct scb *scb;

	uint32 ampdu_q_len;
	uint32 rem_window;
	uint32 tx_in_transit;
	uint32 wlc_txq_len;	/* qlen of all precedence */
	uint32 wlc_pktq_qlen;	/* qlen of a single precedence */
	uint32 prec_map;
	uint32 txpkt_pend; /* TXPKTPEND */
	uint32 dma_desc_avail;
	uint32 dma_desc_pending; /* num of descriptor that has not been processed */
	const char * func;
	uint32 line;
	uint32 tid;
};

#define AMPDU_TXQ_HIS_MAX_ENTRY (1 << 7)
#define AMPDU_TXQ_HIS_MASK (AMPDU_TXQ_HIS_MAX_ENTRY - 1)

static  struct wlc_ampdu_txq_prof_entry _txq_prof[AMPDU_TXQ_HIS_MAX_ENTRY];
static  uint32 _txq_prof_index;
static  struct scb *_txq_prof_last_scb;
static  int 	_txq_prof_cnt;

void wlc_ampdu_txq_prof_enab(void)
{
	_txq_prof_enab = 1;
}

/** tx queue profiling */
void wlc_ampdu_txq_prof_add_entry(wlc_info_t *wlc, struct scb *scb, const char * func, uint32 line)
{
	struct wlc_ampdu_txq_prof_entry *p_entry;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	wlc_txq_info_t *qi;
	struct pktq *q;


	if (!_txq_prof_enab)
		return;

	if (!scb)
		scb = _txq_prof_last_scb;

	if (!scb)
		return;

	scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
	if (!scb_ampdu) {
		return;
	}

	if (!(ini = scb_ampdu->ini[PRIO_8021D_BE])) {
		return;
	}

	/* only gather statistics for BE */
	if (ini->tid !=  PRIO_8021D_BE) {
		return;
	}

	if (prio2fifo[ini->tid] != TX_AC_BE_FIFO)
		return;

	_txq_prof_cnt++;
	if (_txq_prof_cnt == AMPDU_TXQ_HIS_MAX_ENTRY) {
		_txq_prof_enab = 0;
		_txq_prof_cnt = 0;
	}
	_txq_prof_index = (_txq_prof_index + 1) & AMPDU_TXQ_HIS_MASK;
	p_entry = &_txq_prof[_txq_prof_index];

	p_entry->timestamp = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);
	p_entry->scb = scb;
	p_entry->func = func;
	p_entry->line = line;
	p_entry->ampdu_q_len =
		ampdu_pktqprec_n_pkts((&scb_ampdu->txq), (ini->tid));
	p_entry->rem_window = ini->rem_window;
	p_entry->tx_in_transit = ini->tx_in_transit;
	qi = SCB_WLCIFP(scb)->qi;
	q = WLC_GET_TXQ(qi);
	p_entry->wlc_txq_len = pktq_n_pkts_tot(q);
	p_entry->wlc_pktq_qlen = pktqprec_n_pkts(q, WLC_PRIO_TO_PREC(ini->tid));
	p_entry->prec_map = txq_stopped_map(qi->low_txq);

	p_entry->txpkt_pend = TXPKTPENDGET(wlc, (prio2fifo[ini->tid])),
	p_entry->dma_desc_avail = TXAVAIL(wlc, (prio2fifo[ini->tid]));
	p_entry->dma_desc_pending = dma_txpending(WLC_HW_DI(wlc, (prio2fifo[ini->tid])));

	_txq_prof_last_scb = scb;

	p_entry->tid = ini->tid;
} /* wlc_ampdu_txq_prof_add_entry */

void wlc_ampdu_txq_prof_print_histogram(int entries)
{
	int i, index;
	struct wlc_ampdu_txq_prof_entry *p_entry;

	printf("TXQ HISTOGRAM\n");

	index = _txq_prof_index;
	if (entries == -1)
		entries = AMPDU_TXQ_HIS_MASK;
	for (i = 0; i < entries; i++) {
		p_entry = &_txq_prof[index];
		printf("ts: %u	@ %s:%d\n", p_entry->timestamp, p_entry->func, p_entry->line);
		printf("ampdu_q  rem_win   in_trans    wlc_q prec_map pkt_pend "
			"Desc_avail Desc_pend\n");
		printf("%7d  %7d  %7d  %7d %7x %7d %7d   %7d\n",
			p_entry->ampdu_q_len,
			p_entry->rem_window,
			p_entry->tx_in_transit,
			p_entry->wlc_txq_len,
			p_entry->prec_map,
			p_entry->txpkt_pend,
			p_entry->dma_desc_avail,
			p_entry->dma_desc_pending);
		index = (index - 1) & AMPDU_TXQ_HIS_MASK;
	}
}

#endif /* BCMDBG */

#ifdef WL_CS_RESTRICT_RELEASE

void
wlc_ampdu_txeval_all(wlc_info_t *wlc)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
			if (scb_ampdu) {
				uint8 tid;
				for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
					scb_ampdu_tid_ini_t *ini = scb_ampdu->ini[tid];
					if (ini) {
						wlc_ampdu_txeval(wlc->ampdu_tx,
							scb_ampdu, ini,	FALSE);
					}
				}
			}
		}
	}
}

#endif /* WL_CS_RESTRICT_RELEASE */

void
wlc_ampdu_agg_state_update_tx_all(wlc_info_t *wlc, bool aggr)
{
	int idx;
	wlc_bsscfg_t *cfg;

	FOREACH_AS_BSS(wlc, idx, cfg) {
		wlc_ampdu_tx_set_bsscfg_aggr_override(wlc->ampdu_tx, cfg, aggr);
	}
}

#if defined(WL_LINKSTAT)
void
wlc_ampdu_txrates_get(ampdu_tx_info_t *ampdu_tx, wifi_rate_stat_t *rate, int i, bool vht)
{
#ifdef WLAMPDU_MAC
	if (vht) {
		if (i < AMPDU_MAX_VHT) {
			rate->tx_mpdu = ampdu_tx->amdbg->txvht[i];
			rate->mpdu_lost = rate->retries = rate->tx_mpdu -
				ampdu_tx->amdbg->txvht_succ[i];
		}
	} else {
		if (i < AMPDU_HT_MCS_ARRAY_SIZE) {
			rate->tx_mpdu = ampdu_tx->amdbg->txmcs[MCS2IDX(i)];
			rate->mpdu_lost = rate->retries = rate->tx_mpdu -
				ampdu_tx->amdbg->txmcs_succ[MCS2IDX(i)];
		}
	}
#endif /* WLAMPDU_MAC */
}
#endif /* WL_LINKSTAT */

#ifdef TXQ_MUX

#ifdef WLAMPDU_MAC
/**
 * Helper utility to get lowest fallback rate
 *
 *
 * @param wlc	wlc context pointer
 *
 * @return		lowest fallback ratespec.
 */
#if defined(WLAMSDU_TX)
static ratespec_t BCMFASTPATH
wlc_ampdu_get_lowest_txfbrate(wlc_info_t *wlc, struct scb *scb)
{
	uint16 frameid = 0;
	ratesel_txparams_t cur_rate;
	uint16 flags = 0;

	memset(&cur_rate, 0, sizeof(cur_rate));

#if defined(D11AC_TXD)
	if (WLCISACPHY(wlc->band)) {
		/* Multirate fallback rates for 11AC */
		cur_rate.num = 4;
	} else {
		/* Single fallback rate for everything else */
		cur_rate.num = 2;
	}
#else
	cur_rate.num = 2;
#endif

	wlc_scb_ratesel_gettxrate(wlc->wrsi, scb, &frameid, &cur_rate, &flags);

	/* wlc_ratesel_gettxrate() may change cur_rate.num */
	ASSERT(cur_rate.num <= RATESEL_TXRATES_MAX);

	/* Return lowest fallback rate, last in the list */
	return cur_rate.rspec[cur_rate.num - 1];
}
#endif /* WLAMSDU_TX */

/*
 * Following block of routines for AQM AMPDU aggregation (d11 core rev >= 40)
 */

/**
 * Packet dequeue function
 * If AMSDU is disabled: returns a single packet from the multiprec queue
 * If AMSDU is enabled: attempts to form AMSDU aggregate, returns the aggregated packet
 *			if successful or a packet without AMSDU if unsuccessful.
 *
 * @param pq		Multi precedence input queue.
 * @param prec_map	Precedence map to deqeueu from
 * @param prec_out	Precedence of dequeued packet or A-MSDU
 * @param txinfo	A-MSDU session and policy information.
 *
 * @return		A-MSDU, single packet or NULL.
 */
#ifdef WLAMSDU_TX
static void * BCMFASTPATH
wlc_ampdu_pktq_mdeq(struct pktq *pq, uint prec_map, int *prec_out, amsdu_txinfo_t *txinfo)
{
	void *p = pktq_mdeq(pq, prec_map, prec_out);

	if (!p || !txinfo) {
		return p;
	} else {
		/*
		 * The prec_out value returned to the caller
		 * is the first subframe of the aggregate
		 */
		return wlc_amsdu_pktq_agg(pq, prec_map, txinfo, p);
	}
}
#else
#define wlc_ampdu_pktq_mdeq(pq, prec_map, prec_out, txinfo) \
	pktq_mdeq((pq), (prec_map), (prec_out))
#endif /* WLAMSDU_TX */

/**
 * Tx MUX Output function for an SCB Queue that has AQM A-MPDU aggregation enabled.
 *
 * This function is configured as the Tx MUX outout function when A-MPDU is desired for
 * an A-MPDU capable SCB.
 *
 * The prototype matches the @ref mux_output_fn_t function type.
 *
 * @param ctx           a pointer to the scb_ampdu_tx_t structure for the configured SCB
 * @param dfifo         destination data fifo for traffic being requested
 * @param request_time  the total estimated TX time the caller is requesting from the SCB
 * @param output_q      pointer to a packet queue to use to store the output packets
 * @return              The estimated TX time of the packets returned in the output_q. The time
 *                      returned may be greater than the 'request_time'
 */
static uint BCMFASTPATH
wlc_ampdu_output_aqm(void *ctx, uint dfifo, uint request_time, struct spktq *output_q)
{
	scb_ampdu_tx_t *scb_ampdu = ctx;
	scb_ampdu_tid_ini_t *ini = NULL;
	struct scb *scb;
	ampdumac_info_t *hagg;
	ampdu_tx_info_t *ampdu_tx;
	wlc_bsscfg_t *cfg = NULL;
	wlc_info_t *wlc;
	osl_t* osh;
	void* p;
	struct pktq *q;
	ampdu_aqm_timecalc_t *tc;
	struct dot11_header* h;
	wlc_pkttag_t* pkttag;
	uint8* txh;
	uint8* txd;
	uint d11_frame_size;
	uint d11_offset;
	int8 check_epoch = TRUE;
	int err = 0;
	int count = 0;
	bool suppressed_pkt = FALSE;
	uint8 tid, prev_tid = 0;
	uint16 seq;
	uint16 prec_map;
	ratespec_t rspec;
	uint supplied_time = 0;
	uint current_pktlen = 0;
	uint current_pkttime = 0;
	uint pkttime;
	int prec;
	uint16 suppr_count = 0;
	bool reset_suppr_count = FALSE;
#ifdef WLAMSDU_TX
	bool doAMSDU = FALSE;
	amsdu_txinfo_t amsdu_txinfo;
#endif
	wlc_key_t *key = NULL;
	wlc_key_info_t key_info;

	BCM_REFERENCE(suppr_count);
	BCM_REFERENCE(reset_suppr_count);

	ASSERT(dfifo < AC_COUNT);

	ampdu_tx = scb_ampdu->ampdu_tx;
	scb = scb_ampdu->scb;
	wlc = ampdu_tx->wlc;
	q = wlc_scbq_txq(wlc->tx_scbq, scb);

	/* use a prec_map that matches the AC fifo parameter */
	prec_map = wlc->fifo2prec_map[dfifo];

	/* early bail out if there are no pkts in the queue for this AC */
	if (pktq_mlen(q, prec_map) == 0) {
		goto exit;
	}

	if (!wlc_scb_restrict_can_txq(wlc, scb)) {
		ASSERT(0);
		goto exit;
	}

	osh = wlc->osh;
	cfg = SCB_BSSCFG(scb);
	hagg = &ampdu_tx->hagg[dfifo];

	rspec = wlc_scb_ratesel_get_primary(wlc, scb, NULL);

#ifdef WLAMSDU_TX
	memset(&amsdu_txinfo, 0, sizeof(amsdu_txinfo));
	/* Skip checking AMSDU limits if AMSDU not enabled for scb */
	if (SCB_AMSDU_IN_AMPDU(scb)) {
		ratespec_t amsdu_rspec;

		/* rspec is lowest fallback rspec */
		amsdu_rspec = wlc_ampdu_get_lowest_txfbrate(wlc, scb);
		/*
		 * fifo2prio[dfifo] is used in-place of PRIO as an estimate as we have
		 * not dequeued the packet yet.
		 */
		doAMSDU = wlc_amsdu_init_session(wlc, scb, cfg, &amsdu_txinfo,
				fifo2prio[dfifo], amsdu_rspec, ETHER_HDR_LEN, 1);
	}
#endif /* WLAMSDU_TX */

	/* loop may exit via break in addition to these tests */
	while (supplied_time <= request_time && (p = wlc_ampdu_pktq_mdeq(q, prec_map, &prec,
			(doAMSDU ? &amsdu_txinfo : NULL)))) {
		bool regmpdu_pkt = FALSE;
		bool reque_pkt = FALSE;
		bool has_seq_hole = FALSE;
		d11pktinfo_common_t *PktInfo;

		pkttag = WLPKTTAG(p);

		/* Packets for two <tid, ini> may be in the queue for one AC, the loop needs to be
		 * able to transition from one tid to another
		 * This check will also trigger if 'ini' has not yet been assigned.
		 */
		tid = (uint8)PKTPRIO(p);

		/* All packets should have a prio that matches the precidence queue they were on.
		 * Otherwise, the TID draining code in wlc_ampdu_output_nonba() will not drain all
		 * the of the matching priority.
		 */
		ASSERT(prec == WLC_PRIO_TO_PREC(tid) || prec == WLC_PRIO_TO_HI_PREC(tid));

		if (ini == NULL || tid != prev_tid) {
			ASSERT(tid < AMPDU_MAX_SCB_TID);

			/* if we are changing TID, we need to update the epoch */
			check_epoch = TRUE;

			/*
			 * if we are changing TID, we need to clear key so
			 * wlc_prep_sdu_fast() will re-evaluate the
			 * wlc_txc_hit() call
			 */
			key = NULL;

			if (count) {
				AMPDUSCBCNTADD(scb_ampdu->cnt.txmpdu, count);
				if (ini) {
					ini->tx_in_transit += (uint16)count;
				}
				count = 0;
			}

			prev_tid = tid;
			ini = scb_ampdu->ini[tid];

			/* check if an initiator exists for this TID */
			if (ini == NULL) {
				/* initialize initiator on first packet; sends addba req */
				ini = wlc_ampdu_init_tid_ini(ampdu_tx, scb_ampdu, tid, FALSE);
				if (ini == NULL) {
					WL_AMPDU(("wl%d: %s: ini creation failed for "MACF" "
					          "tid %u\n", wlc->pub->unit, __FUNCTION__,
					          ETHER_TO_MACF(scb->ea), tid));
				}
			}

			/* if there is no initiator for this TID, or the state is not ON,
			 * then all the pkts in this TID need to be sent on outside a BA
			 * aggrement
			 */
			if ((ini == NULL) ||
			    (ini->ba_state != AMPDU_TID_STATE_BA_ON)) {
				/* put the current packet back at the head of the queue to be
				 * processed by wlc_ampdu_output_nonba().
				 */
				pktq_penq_head(q, prec, p);
				p = NULL;

				/* drain all pkts at the current TID for non-aggregated output */
				supplied_time +=
				        wlc_ampdu_output_nonba(q, scb, wlc, rspec, dfifo, tid,
				                               (request_time - supplied_time),
				                               output_q);

				/* The pktq queue should now be empty, or have a pkt with a
				 * different TID at the head, or the supplied_time exhausted
				 * the requested_time.
				 * Jump back to the top of the loop.
				 */
				ini = NULL;
				continue;
			}
		}


		tc = &ini->timecalc[dfifo];
		if (tc->rspec != rspec) {
			wlc_ampdu_init_aqm_timecalc(scb_ampdu, ini, rspec, dfifo, tc);
		}

		/* separate packet preparation and time calculation for
		 * MPDU (802.11 formatted packet with txparams), and
		 * MSDU (Ethernet or 802.3 stack packet)
		 */

		if ((pkttag->flags & WLF_MPDU) == 0) {
			uint fifo;
			/*
			 * MSDU packet prep
			 */

			/* mark this as an MPDU in a BA */
			pkttag->flags |= WLF_AMPDU_MPDU;


#ifdef PROP_TXSTATUS
			/* Use the seq number the host driver provides if present.
			 * The host may be resending a previously suppressed frame
			 * that was already assigned a sequence nubmer.
			 */
			suppressed_pkt = (IS_WL_TO_REUSE_SEQ(pkttag->seq) != 0);
			if (suppressed_pkt) {
				seq = WL_SEQ_GET_NUM(pkttag->seq);
#ifdef PROP_TXSTATUS_SUPPR_WINDOW
				suppr_count++;
#endif
			} else
#endif /* PROP_TXSTATUS */
			{
				/* We have a non-suppressed regular data packet.
				 * Use next seq number for this scb and tid
				 */

				/* assign seqnum and save in pkttag */
				seq = SCB_SEQNUM(ini->scb, ini->tid) & (SEQNUM_MAX - 1);
				SCB_SEQNUM(ini->scb, ini->tid)++;
				pkttag->seq = seq;
				/* record the highest seq number in the BA session */
				ini->max_seq = seq;
				reset_suppr_count = TRUE;

			}
#ifdef PROP_TXSTATUS
			/* Set FROMFW to note that the seq number is assigned.
			 * FROMFW is used in PROP_TXSTATUS suppression
			 */
			if (PROP_TXSTATUS_ENAB(ampdu_tx->wlc->pub) &&
				WLFC_GET_REUSESEQ(wlfc_query_mode(ampdu_tx->wlc->wlfc))) {
				SET_WL_HAS_ASSIGNED_SEQ(pkttag->seq);
				if (reset_suppr_count || (ini->suppr_window <= suppr_count)) {
					ini->suppr_window = 0;
				} else {
					ini->suppr_window -= suppr_count;
				}
			}
#endif /* PROP_TXSTATUS */

			/* use the streamlined version of prep_sdu if possible,
			 * otherwise fall back to full version of prep_sdu.
			 */
			err = wlc_prep_sdu_fast(wlc, cfg, scb, p, &fifo, &key, &key_info);
			if (err == BCME_UNSUPPORTED) {
				uint npkts = 1;

				err = wlc_prep_sdu(wlc, scb, &p, (int*)&npkts, &fifo);

				/* fragmentation is not for A-MPDU, A-MSDU, or any MSDUs under an
				 * HT-immediate BA aggrement (IEEE802.11-2012 sec 9.2.7), so npkts
				 * (the number of fragments for the input frame) should always be 1
				 * if there was no error reported.
				 *
				 * ASSERT is for npkts is 1 unless an error is returned
				 * from wlc_prep_sdu_fast()
				 */
				ASSERT(npkts == 1 || (err != BCME_OK));
			}

			/* Determine if the packet can be sent in an AMPDU.
			 *
			 * Since the packet is an SDU data packet, we know it can be sent in an
			 * A-MPDU as long as the phy frame type is HT or VHT, and the encryption is
			 * compatible with A-MPDU.
			 *
			 * The WLF_MIMO flag is set in wlc_d11ac_hdrs()/wlc_d11ht_hdrs() if the
			 * frame is HT or VHT, and if the key is compatible (or no key at all).
			 *
			 * Exception: SCB_PS implies no agg.
			 */
			regmpdu_pkt = !(pkttag->flags & WLF_MIMO) || SCB_PS(scb);

			if (!err) {
				/* the fifo that wlc_prep_sdu() calculates should match the AC since
				 * we are pulling packets from the matching set of precidences.
				 * ASSERT if this assumption breaks since we are suppling these
				 * packets to the AC defined fifo.
				 */
				ASSERT(fifo == dfifo);

				/* Fetch the TxD (DMA hw hdr), vhtHdr (rev 40+ ucode hdr),
				 * and 'h' (d11 frame header).
				 */
				wlc_txprep_pkt_get_hdrs(wlc, p, &txd, &txh, &h);
				d11_offset = (uint32)((int8*)h - (int8*)txd);
				d11_frame_size = pkttotlen(osh, p) - d11_offset;

				if (regmpdu_pkt) {
					/* use the non-ampdu time calc since
					 * this will not be in an ampdu
					 */

					/* calculate and store the estimated pkt tx time */
					pkttime = wlc_tx_mpdu_time(wlc, scb, rspec, dfifo,
					                           d11_frame_size);
				} else if (current_pktlen == d11_frame_size) {
					/* optimization: skip the txtime calculation if the MPDU
					 * pkt len is the same as the last time through the loop
					 */

					/* estimated pkt tx time should be the same as last time */
					pkttime = current_pkttime;
				} else {
					/* Calculate an estimated airtime for this part of
					 * an A-MPDU
					 */

					/* calculate the estimated pkt tx time */
					pkttime = wlc_ampdu_aqm_timecalc(tc, d11_frame_size);
					current_pktlen = d11_frame_size;
					current_pkttime = pkttime;
				}

				/* store the estimated pkt tx time */
				pkttag->pktinfo.atf.pkt_time = (uint16)pkttime;

			} else {
				if (err == BCME_ERROR) {
					/* BCME_ERROR indicates a tossed packet */

					/* error in the packet; reject it */
					WL_AMPDU_ERR(("wl%d: %s: prep_sdu rejected pkt\n",
					              wlc->pub->unit, __FUNCTION__));
					WLCNTINCR(ampdu_tx->cnt->sdurejected);
					AMPDUSCBCNTINCR(scb_ampdu->cnt.sdurejected);
				} else {
					/* should be no other errors */
					if (err != BCME_OK) {
						WL_AMPDU_ERR(("%s:ERROR=%d\n",
								__FUNCTION__, err));
					}
					ASSERT(err == BCME_OK);

					PKTFREE(osh, p, TRUE);
					p = NULL;
				}

				/* reclaim the unused sequence number */
#ifdef PROP_TXSTATUS
				/* if the sequence number did not come from the host driver */
				if (!suppressed_pkt)
#endif
				{
					SCB_SEQNUM(ini->scb, ini->tid)--;
					ini->max_seq = (SCB_SEQNUM(ini->scb, ini->tid) &
					                (SEQNUM_MAX - 1));
				}

				continue;
			}
#ifdef PROP_TXSTATUS
			/*
			 * Now the dongle owns the packet
			 * Toggle Prop TXSTATUS ownership bits
			 * Pull it out of the pkttag again to ensure preceding routines
			 * did not change pkttag.
			 */
			if (IS_WL_TO_REUSE_SEQ(pkttag->seq)) {
				RESET_WL_TO_REUSE_SEQ(pkttag->seq);
			}
#endif /* PROP_TXSTATUS */
		} else {
			uint fifo;
			/*
			 * MPDU packet prep
			 */

			/* if there are pdu txparams or if the d11 header was already preped, we
			 * cannot make assumptions about this pkt being aggregatable with previous
			 * pkts in this loop.
			 * Flag to do full epoch checking.
			 */
			check_epoch = TRUE;

			/* WLF_TXHDR flag indicates the MPDU is being retransmitted
			 * after having d11 header prep previously
			 */
			reque_pkt = (pkttag->flags & WLF_TXHDR);

			if (!reque_pkt) {
				ratespec_t tmp_rspec;

				/* If this packet is not requeued, fetch the rspec saved in tx_prams
				 * at the head of the pkt before tx_params are removed by
				 * wlc_prep_pdu()
				 */
				tmp_rspec = wlc_pdu_txparams_rspec(osh, p);

				/* Note: currently no errors from wlc_prep_pdu() */
				(void)wlc_prep_pdu(wlc, scb, p, &fifo);
				ASSERT(fifo == dfifo);

				/* Fetch the TxD (DMA hw hdr), vhtHdr (rev 40+ ucode hdr),
				 * and 'h' (d11 frame header)
				 */
				wlc_txprep_pkt_get_hdrs(wlc, p, &txd, &txh, &h);
				d11_offset = (uint32)((int8*)h - (int8*)txd);
				d11_frame_size = pkttotlen(osh, p) - d11_offset;

				/* Use already assigned sequence number from pkt hdr */
				seq = (ltoh16(h->seq) >> SEQNUM_SHIFT);
				WL_TMP(("TXQ: %s WLF_TXHDR pkt found, seq 0x%04x\n",
				        __FUNCTION__, seq));

				/* calculate and store the estimated pkt tx time */
				pkttime = wlc_tx_mpdu_time(wlc, scb,
					tmp_rspec, dfifo, d11_frame_size);
				WLPKTTIME(p) = (uint16)pkttime;

				/* Determine if the packet can be sent in an AMPDU.
				 * Since the packet is an MPDU without re-queuing, the packet should
				 * be a management frame, which are not aggregatable.
				 * Assert that the frame type is not DATA, and note that this pkt
				 * should be sent as a regular mpdu.
				 */
				regmpdu_pkt = TRUE;
				ASSERT((ltoh16(h->fc) & FC_KIND_MASK) != FC_QOS_DATA);

			} else {
				bool s_mpdu_pdu = FALSE;
				/* Note: currently no errors from wlc_prep_pdu() */
				(void)wlc_prep_pdu(wlc, scb, p, &fifo);
				ASSERT(fifo == dfifo);

				/* Fetch the TxD (DMA hw hdr), vhtHdr (rev 40+ ucode hdr),
				 * and 'h' (d11 frame header)
				 */
				wlc_txprep_pkt_get_hdrs(wlc, p, &txd, &txh, &h);
				d11_offset = (uint32)((int8*)h - (int8*)txd);
				d11_frame_size = pkttotlen(osh, p) - d11_offset;

				/* Use already assigned sequence number from pkt hdr */
				seq = (ltoh16(h->seq) >> SEQNUM_SHIFT);
				WL_TMP(("TXQ: %s WLF_TXHDR pkt found, seq 0x%04x\n",
				        __FUNCTION__, seq));

				{
#ifdef WL11AX
					if (D11REV_GE(wlc->pub->corerev, 80)) {
						PktInfo = &((d11txh_rev80_t *)txh)->PktInfo;
					} else
#endif /* WL11AX */
					{
						PktInfo = &((d11actxh_t *)txh)->PktInfo;
					}

					if (!(ltoh16(PktInfo->MacTxControlLow) & D11AC_TXC_AMPDU))
						s_mpdu_pdu = TRUE;
				}

				/* Determine if the packet can be sent in an AMPDU.
				 * Since the packet is a re-queued MPDU, the packet could
				 * be a DATA frame, which are aggregatable.
				 * Check if the AMPDU flag was set, and if WLF_MIMO was set just
				 * as in the MSDU prep code above.
				 * Exception: SCB_PS implies no agg.
				 */

				if (((pkttag->flags & (WLF_MIMO | WLF_AMPDU_MPDU)) ==
				     (WLF_MIMO | WLF_AMPDU_MPDU)) &&
				    !SCB_PS(scb) && !s_mpdu_pdu) {
					/* The packet had both flags set, and we are not PS,
					 * so it is AMPDU ready
					 */
					regmpdu_pkt = FALSE;
				} else {
					/* The packet did not have both flags set,
					 * so it is a regular non-ampdu packet
					 */
					regmpdu_pkt = TRUE;
				}

				/* If the pkt already had d11 header prep,
				 * the airtime has already been calculated
				 */
				pkttime = WLPKTTIME(p);
			}
		}

		WL_AMPDU_TX(("wl%d: %s: prep_xdu success; seq 0x%x tid %d\n",
			wlc->pub->unit, __FUNCTION__, seq, tid));

		/* MUX Implementation note:
		 * The non-MUX code for _wlc_sendampdu_aqm() had logic for regmpdu_pkt like this:
		 * regmpdu_pkt =
		 *      !(pkttag->flags & WLF_MIMO) ||
		 *      (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF) ||
		 *      (ini->ba_state == AMPDU_TID_STATE_BA_OFF) ||
		 *      SCB_PS(scb);
		 *
		 * In this routine, the 'ini' (initiator state, or BA Originator) validation above
		 * makes sure that the ba_state is always ON, so the two checks on ba_state are not
		 * needed. The non-MUX code marks packets with the pkt flag WLF_AMPDU_MPDU in the
		 * TxModule before putting the pakcet in the serialized TxQ. By the time
		 * _wlc_sendampdu_aqm() runs and dequeues packets from the TxQ the BA state may have
		 * changed. In the MUX code there is no AMPDU TxModule so this routine checks the
		 * state of the 'ini' as it starts drianing packets from the SCBQ at a particular
		 * TID (priority).
		 *
		 * For MUX code, this routine is only handling packets for one SCB. Stations in PS
		 * mode are not allowed to send A-MPDUs. So when a station transitions to PS, this
		 * routine will be un-configured as the SCBQ output function. As a result, there is
		 * no need for an SCB_PS() check, and instead this routine asserts !SCB_PS() near
		 * the top.
		 *
		 * In non-MUX code, _wlc_sendampdu_aqm() drains from the driver's serialized TxQ
		 * packets for any SCB as long as the packets are marked as WMF_AMPDU_MPDU. So the
		 * non-MUX code needed to check this condition.
		 *
		 * This routine handles any packets queued for an SCB including Bufferable MMPDUs
		 * (PS deliverable management frames). Because of this checks for Data vs Management
		 * frame types need to be made. So the 'regmpud_pkt' logic is moved up in the cases
		 * for MSDU and MPDU packet prep just above this comment.
		 */

		/* frames should all be prepared with a header by now */
		ASSERT(pkttag->flags & WLF_MPDU);

		ASSERT(seq == ltoh16(h->seq) >> SEQNUM_SHIFT);

		ASSERT(!ETHER_ISNULLADDR(&h->a1));

		if (pkttag->flags & WLF_FIFOPKT) {
#ifdef WL_CS_PKTRETRY
			wlc_ampdu_chanspec_update(wlc, p);
#endif
			pkttag->flags &= ~WLF_FIFOPKT;
		} else if (!ampdu_is_exp_seq(ampdu_tx, ini, seq, suppressed_pkt)) {
			ASSERT(0);
			PKTFREE(osh, p, TRUE);
			p = NULL;
			WLCNTINCR(ampdu_tx->cnt->txdrop);
			continue;
		}

		has_seq_hole = ampdu_detect_seq_hole(ampdu_tx, seq, ini);

#ifdef WL11AX
		if (D11REV_GE(wlc->pub->corerev, 80)) {
			PktInfo = &((d11txh_rev80_t *)txh)->PktInfo;
		} else
#endif /* WL11AX */
			PktInfo = &((d11actxh_t *)txh)->PktInfo;

		/*
		 * Do the AMPDU specific header preparation
		 */
		if (regmpdu_pkt) {
			WL_AMPDU_TX(("%s: reg mpdu found: WLF_MIMO %d Type/Subtype 0x%x\n",
			             __FUNCTION__,
			             ((pkttag->flags & WLF_MIMO) != 0),
			             (ltoh16(h->fc) & FC_KIND_MASK)));

			/* clear ampdu bit, set svht bit */
			PktInfo->MacTxControlLow &= htol16(~D11AC_TXC_AMPDU);
			/* non-ampdu must have svht bit set */
			PktInfo->MacTxControlHigh |= htol16(D11AC_TXC_SVHT);

			/* if previous frame is an AMPDU mpdu, flip the epoch */
			if (hagg->prev_ft != AMPDU_NONE) {
				wlc_txh_info_t tx_info;

				/* ampdu switch to regmpdu */
				wlc_get_txh_info(wlc, p, &tx_info);
				wlc_ampdu_chgnsav_epoch(ampdu_tx, dfifo,
					AMU_EPOCH_CHG_NAGG, scb, tid, &tx_info);
				wlc_txh_set_epoch(ampdu_tx->wlc, txd, hagg->epoch);
				hagg->prev_ft = AMPDU_NONE;
			}
			/* otherwise, if prev is regmpdu already, don't set epoch at all */
		} else {
			bool change = FALSE;
			int8 reason_code = 0;

			/* clear svht bit, set ampdu bit */
			PktInfo->MacTxControlLow |= htol16(D11AC_TXC_AMPDU);
			/* for real ampdu, clear vht single mpdu ampdu bit */
			PktInfo->MacTxControlHigh &= htol16(~D11AC_TXC_SVHT);

#ifdef BCMDBG_ASSERT
			ASSERT(wlc_ampdu_check_percache_info(wlc, ampdu_tx, scb, tid, txh));
#endif /* BCMDBG_ASSERT */

			if (check_epoch) {
				uint16 frameid;
				wlc_d11txh_u txh_u;

				change = TRUE;
				frameid = ltoh16(PktInfo->TxFrameID);

				if (D11REV_LT(wlc->pub->corerev, 80)) {
					txh_u.d11actxh = ((d11actxh_t *)txh);
				}
#ifdef WL11AX
				else {
					txh_u.d11txh_rev80 = ((d11txh_rev80_t *)txh);
				}
#endif /* WL11AX */

				if (frameid & TXFID_RATE_PROBE_MASK) {
					/* check vrate probe flag
					 * this flag is only get the frame to become first mpdu
					 * of ampdu and no need to save epoch_params
					 */
					wlc_scb_ratesel_probe_ready(wlc->wrsi, scb, frameid,
						FALSE, 0, WME_PRIO2AC(PKTPRIO(p)));
					reason_code = AMU_EPOCH_CHG_FID;
				} else if (scb != hagg->prev_scb || tid != hagg->prev_tid) {
					reason_code = AMU_EPOCH_CHG_DSTTID;
				} else if (wlc_ampdu_epoch_params_chg(wlc, hagg, txh_u)) {
					/* rate/sgi/bw/stbc/antsel tx params */
					reason_code = AMU_EPOCH_CHG_PLCP;
				} else if (ltoh16(PktInfo->MacTxControlLow) & D11AC_TXC_UPD_CACHE) {
					reason_code = AMU_EPOCH_CHG_TXC_UPD;
				} else if ((txd[2] & TOE_F2_TXD_HEAD_SHORT) != hagg->prev_shdr) {
					reason_code = AMU_EPOCH_CHG_TXHDR;
				} else {
					change = FALSE;
				}

				check_epoch = FALSE;
#if defined(WLC_UCODE_CACHE) && D11CONF_GE(42)
				/* If this frame signals a HW Cache update, then it will have
				 * a long txd ucode header. The next frame (if it is a cache hit)
				 * will have a short txd.
				 * Reset check_epoch so next time through the loop, the next
				 * packet will be checked for the long/short txd header.
				 */
				if (ltoh16(PktInfo->MacTxControlLow) &
					D11AC_TXC_UPD_CACHE) {
					check_epoch = TRUE;
				}
#endif /* WLC_UCODE_CACHE */
			}

			if (!change) {
				if (hagg->prev_ft == AMPDU_NONE) {
					/* switching from non-aggregate to aggregate ampdu,
					 * just update the epoch changing parameters(hagg) but
					 * not the epoch.
					 */
					change = TRUE;
					reason_code = AMU_EPOCH_NO_CHANGE;
				} else if (has_seq_hole) {
					/* Flip the epoch if there is a hole in sequence of pkts
					 * sent to txfifo. It is a AQM HW requirement
					 * not to have the holes in txfifo.
					 */
					change = TRUE;
					reason_code = AMU_EPOCH_CHG_SEQ;
				}
			}

			if (change) {
				wlc_txh_info_t tx_info;
				wlc_get_txh_info(wlc, p, &tx_info);
				wlc_ampdu_chgnsav_epoch(ampdu_tx, dfifo, reason_code,
					scb, tid, &tx_info);
			}

			/* always set epoch for ampdu */
			wlc_txh_set_epoch(ampdu_tx->wlc, txd, hagg->epoch);
		}

		/* enable packet callback for every MPDU in AMPDU */
		WLF2_PCB4_REG(p, WLF2_PCB4_AMPDU);

		/* Supply the MPDU to output queue */
		spktenq(output_q, p);
		supplied_time += pkttime;
		count++;
	}

	if (count) {
		AMPDUSCBCNTADD(scb_ampdu->cnt.txmpdu, count);
		if (ini) {
			ini->tx_in_transit += (uint16)count;
		}
	}

exit:
	/* if nothing was output, then this mux source is stalled. Update the scbq state
	 * so that scbq can restart the mux source when pkts arrive.
	 */
	if (supplied_time == 0) {
		wlc_scbq_scb_stall_set(wlc->tx_scbq, scb, dfifo);
	}

	WL_AMPDU_TX(("%s: "MACF" data fifo %d epoch %d count %d supplied_time %d\n",
	             __FUNCTION__, ETHER_TO_MACF(scb->ea),
	             dfifo, ampdu_tx->hagg[dfifo].epoch, count, supplied_time));

	return supplied_time;
} /* wlc_ampdu_output_aqm */

/**
 * Helper function for the A-MPDU Tx MUX output functions. This fn will handle output
 * of packets from a TID (priority) that does not have a BA Agreement. All the packets
 * at the given TID will be drained an sent non-aggregated and outside of a BA Agreement.
 *
 * Similar to the Tx MUX output fuctions that use this, the function will return the total estimated
 * tx airtime of packets that it adds to the output_q.
 *
 * @param q             A pointer to the SCBQ packet queue.
 * @param scb           A pointer to the SCB we are working on.
 * @param wlc           wlc_info_t pointer
 * @param ac            Access Category traffic being requested
 * @param tid           The TID (priority) of pakcets to drain and prepare.
 * @param request_time  the total estimated TX time the caller is requesting from the SCB
 * @param output_q      pointer to a packet queue to use to store the output packets
 * @return              The estimated TX time of the packets returned in the output_q. The time
 *                      returned may be greater than the 'request_time'
 */
static int BCMFASTPATH
wlc_ampdu_output_nonba(struct pktq *q, struct scb *scb, wlc_info_t *wlc, ratespec_t rspec,
                       uint ac, uint tid, uint request_time, struct spktq *output_q)
{
	wlc_txh_info_t txh_info;
	wlc_pkttag_t *pkttag;
	uint16 prec_map;
	int prec;
	void *pkt[DOT11_MAXNUMFRAGS] = {0};
	int i, count;
	DBGONLY(int pkt_count = 0; )
	uint pkttime;
	uint supplied_time = 0;
#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */
	bool check_epoch = TRUE;
#endif /* WLAMPDU_MAC */

	/* use a prec_map that matches the tid prio parameter */
	prec_map = ((1 << WLC_PRIO_TO_PREC(tid)) | (1 << WLC_PRIO_TO_HI_PREC(tid)));

	while (supplied_time < request_time && (pkt[0] = pktq_mdeq(q, prec_map, &prec))) {
		pkttag = WLPKTTAG(pkt[0]);

		if (!(pkttag->flags & WLF_MPDU)) {
			/*
			 * MSDU packet prep
			 */

			int err;
			uint fifo; /* is this param needed anymore ? */

			pkttime = 0;

			err = wlc_prep_sdu(wlc, scb, pkt, &count, &fifo);
			if (err == BCME_OK) {
				for (i = 0; i < count; i++) {
					uint16 frag_time;

					wlc_get_txh_info(wlc, pkt[i], &txh_info);

					/* calculate and store the estimated pkt tx time */
					frag_time =
					        (uint16)wlc_tx_mpdu_time(wlc, scb, rspec, ac,
					                                 txh_info.d11FrameSize);

					WLPKTTIME(pkt[i]) = frag_time;

					pkttime += frag_time;
				}
			} else if (err == BCME_ERROR) {
				/* BCME_ERROR indicates a tossed packet */

				/* pkt[] should be invalid and count zero */
				ASSERT(count == 0);

				/* let the code finish the loop adding no time
				 * for this dequeued packet, and enqueue nothing to
				 * output_q since count == 0
				 */
				pkttime = 0;
			} else {
				/* should be no other errors */
				ASSERT(err == BCME_OK);
				for (i = 0; i < count; i++) {
					PKTFREE(wlc->osh, pkt[i], TRUE);
				}
				pkttime = 0;
			}
		} else {
			/*
			 * MPDU packet prep
			 */

			uint fifo; /* is this param needed anymore ? */

			/* fetch the rspec saved in tx_prams at the head of the pkt
			 * before tx_params are removed by wlc_prep_pdu()
			 */
			rspec = wlc_pdu_txparams_rspec(wlc->osh, pkt[0]);

			count = 1;
			/* not looking at error return */
			(void)wlc_prep_pdu(wlc, scb, pkt[0], &fifo);

			wlc_get_txh_info(wlc, pkt[0], &txh_info);

			/* calculate and store the estimated pkt tx time */
			pkttime = wlc_tx_mpdu_time(wlc, scb, rspec, ac, txh_info.d11FrameSize);
			WLPKTTIME(pkt[0]) = (uint16)pkttime;
		}

		DBGONLY(pkt_count += count; )
		supplied_time += pkttime;

		for (i = 0; i < count; i++) {
#ifdef WLAMPDU_MAC
			/* For AQM AMPDU Aggregation:
			 * If there is a transition from A-MPDU aggregation frames to a
			 * non-aggregation frame, the epoch needs to change. Otherwise the
			 * non-agg frames may get included in an A-MPDU.
			 */
			if (check_epoch && AMPDU_AQM_ENAB(wlc->pub)) {
				/* Once we check the condition, we don't need to check again since
				 * we are enqueuing an non_ampdu frame so wlc_ampdu_was_ampdu() will
				 * be false.
				 */
				check_epoch = FALSE;
				/* if the previous frame in the fifo was an ampdu mpdu,
				 * change the epoch
				 */
				if (wlc_ampdu_was_ampdu(wlc->ampdu_tx, ac)) {
					bool epoch;

					wlc_get_txh_info(wlc, pkt[i], &txh_info);
					epoch = wlc_ampdu_chgnsav_epoch(wlc->ampdu_tx,
					                                ac,
					                                AMU_EPOCH_CHG_MPDU,
					                                scb,
					                                (uint8)tid,
					                                &txh_info);
					wlc_txh_set_epoch(wlc, txh_info.tsoHdrPtr, epoch);
				}
			}
#endif /* WLAMPDU_MAC */

			{
				uint16 frameid;

				wlc_get_txh_info(wlc, pkt[i], &txh_info);

				frameid = ltoh16(txh_info.TxFrameID);
				if (frameid & TXFID_RATE_PROBE_MASK) {
					wlc_scb_ratesel_probe_ready(wlc->wrsi, scb, frameid,
					                            FALSE, 0, (uint8)ac);
				}
			}

			/* add this pkt to the output queue */
			spktenq(output_q, pkt[i]);
		}
	}

#ifdef WLAMPDU_MAC /* ucode, ucode hw assisted or AQM aggregation */
	/*
	 * If there are non-aggreation packets added to a fifo, make sure the epoch will
	 * change the next time entries are made to the aggregation info side-channel.
	 * Otherwise, the agg logic may include the non-aggreation packets into an A-AMPDU.
	 */
	if (spktq_n_pkts(output_q) > 0 && AMPDU_MAC_ENAB(wlc->pub) &&
		!AMPDU_AQM_ENAB(wlc->pub)) {
		wlc_ampdu_change_epoch(wlc->ampdu_tx, ac, AMU_EPOCH_CHG_MPDU);
	}
#endif /* WLAMPDU_MAC */

	WL_AMPDU_TX(("%s: fifo %d tid %d count %d supplied_time %d\n",
	             __FUNCTION__,
	             ac, tid, pkt_count, supplied_time));

	return supplied_time;
} /* wlc_ampdu_output_nonba */

/**
 * Max Mixed Mode time:
 * The time protected by a Mixed Mode OFDM PLCP header.
 * The legacy header is a 6Mbps OFDM header with a max LENGTH value of 4095 bytes.
 * The time in us after the legacy header (L-PREAMBLE/L-SIG) with max L_LENGTH=4095 is
 * 4095 bytes + (16bit(service) + 6bit(tail)) = 4098 bytes
 * 4098B / 3 bytes per symbol = 1366 symbols
 * 1366 symbols * 4us per OFDM symbol = 5464 us
 */
#define MM_TIME          5464

/**
 * DATA field of VHT or HT PPDU inside the Mixed Mode frame
 * follows 48 us of SIGNAL and PREAMBLE in the worst case.
 *
 * Longest VHT header time from P802.11ac_D7.0, sec 22.4.3 "TXTIME and PSDU_LENGTH calculation",
 * shows preamble as
 *        VHT_T_SIG_A(8) + VHT_T_STF(4) + Nvhtltf*VHT_T_LTF(4) + VHT_T_SIG_B(4)
 * with largest Nvhtltf=8 (22.3.8.3.5 VHT-LTF definition).
 * VHT Header time max = 48us
 * Longest HT header time from IEEE Std 802.11-2012, se 20.4.3 "TXTIME calculation",
 * shows preamble as
 *        HT_T_SIG(8) + HT_T_STF(4) + (HT_T_LTF1(4) - HT_T_LTFs(4)) + Nltf * HT_T_LTFs(4)
 * with largest Nltf=5 (20.3.9.4.6 HT-LTF definition)
 * HT Header time max = 32us
 *
 * MM_TIME - 48 = 5464 - 48 = 5416
 */
#define MM_MAX_DATA_TIME 5416

/* Number of SGI symbols in DATA field will be FLOOR( MM_MAX_DATA_TIME / 3.6 ) */
#define MM_SGI_MAX_SYM   1504
/* Number of Non-SGI symbols in DATA field will be FLOOR( MM_MAX_DATA_TIME / 4 ) */
#define MM_MAX_SYM       1354


/**
 * @brief Initialize an ampdu_aqm_timecalc struct for fast packet TX time calculations
 *
 * Initialize an ampdu_aqm_timecalc struct for fast packet TX time calculations.
 * This function will initialize a ampdu_aqm_timecalc struct with parameters derived from the
 * current band and BSS, and the length independent fixed time overhead. The initialized struct
 * may be used by wlc_ampdu_timecalc() to do a TX time estimate for a packet.
 *
 * @param scb_ampdu     a pointer to the scb_ampdu_tx_t per-scb ampdu tx state
 * @param ini           a pointer to the scb_ampdu_tid_ini_t initiator state
 * @param rspec         the rate for all calculations
 * @param ac            Access Category for all calculations, used to estimate MAC access delay
 * @param tc            pointer to a ampdu_aqm_timecalc struct
 */
static void
wlc_ampdu_init_aqm_timecalc(scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini,
                        ratespec_t rspec, uint ac, ampdu_aqm_timecalc_t *tc)
{
	struct scb* scb;
	wlcband_t *band;
	wlc_bsscfg_t *bsscfg;
	uint fixed_overhead_us;
	uint empty_frame_us;
	uint Ndbps;
	uint Nes;
	uint sgi;
	uint max_sym;
	uint32 max_bytes;
	uint max_pdu;			/* max pdus allowed in ampdu */
	uint min_limit;

	scb = scb_ampdu->scb;
	bsscfg = scb->bsscfg;
	band = wlc_scbband(bsscfg->wlc, scb);

	/* Calculate the fixed overhead (length independent) for a single A-MPDU.
	 * The caclulation used the time for the frame sequence not including the
	 * AMPDU itself, plus the time for a frame with no payload (len = 0)
	 * This is the time from preamble to FCS, no medium access or time
	 * for ACK frames in the frame exchange.
	 */
	/* time for a frame with len = 0 */
	empty_frame_us = wlc_txtime(rspec, BAND_2G(band->bandtype), TRUE, 0);

	/* time for frame exchange other than AMPDU */
	fixed_overhead_us = wlc_tx_mpdu_frame_seq_overhead(rspec, bsscfg, band, ac);

	/* total overhead without any A-MPDU payload */
	fixed_overhead_us += empty_frame_us;

	/* Ndbps is saved to calc the A-MPDU sub-frame time, and used here
	 * to calculate max byte length based on the max time in a Mixedmode Format
	 * HT or VHT frame.
	 * Ndbps is the number of payload bits carried in one PHY symbol.
	 */
	Ndbps = wlc_txtime_Ndbps(rspec);

	/* find the max number of symbols in a mixed mode frame */
	sgi = RSPEC_ISSGI(rspec);
	if (sgi) {
		max_sym = MM_SGI_MAX_SYM; /* SGI symbols */
	} else {
		max_sym = MM_MAX_SYM;     /* regular GI symbols */
	}

	/* Max number of bytes in an A-MPDU protected by a MM header.
	 *
	 * The calculation is the maximum number of bits in the DATA field (max_sym * Ndbps)
	 * minus the overhead of SERVICE and TAIL, and the remaining bits converted to bytes.
	 *
	 * The overhead is the bits of the SERVICE (16) field, and bits in (TAIL(6) * Nes).
	 * The SERVICE and TAIL bit size are the same in both VHT and HT frames.
	 */
	Nes = wlc_txtime_Nes(rspec);
	max_bytes = ((max_sym * Ndbps) - (VHT_N_SERVICE + Nes * VHT_N_TAIL)) / 8;

	/* Max number of bytes in an A-MPDU may be further limited by
	 * the receiver's advertized rx size.
	 */
	max_bytes = MIN(max_bytes, scb_ampdu->max_rxlen);

	/* Calc the max number of mpdus in an A-MPDU as the min of our self-imposed
	 * tx max_pdu, and the recipient's advertised BA WinSize (Block Ack Parameter Set
	 * "Buffer Size" field.)
	 */
	max_pdu = MIN(scb_ampdu->max_pdu, ini->ba_wsize);

	/* Calculate the boundary size for small frames in an A-MPDU.
	 * If a sub-frame is less than this byte length, then it should get
	 * 1/max_pdu of the A-MPDU overhead.
	 */
	min_limit = max_bytes / max_pdu;
	/* the min_limit value will be compared against the mpdu_len without the
	 * A-MPDU delimiter and FCS, so adjust
	 */
	min_limit -= AMPDU_DELIMITER_LEN + DOT11_FCS_LEN;

	tc->rspec = rspec;
	tc->Ndbps = (uint16)Ndbps;
	tc->sgi = (uint8)sgi;
	tc->fixed_overhead_us = (uint16)fixed_overhead_us;
	tc->fixed_overhead_pmpdu_us = (uint16)(fixed_overhead_us / max_pdu);
	tc->min_limit = (uint16)min_limit;
	tc->max_bytes = max_bytes;
} /* wlc_ampdu_init_aqm_timecalc */

/**
 * @brief Calculate a TX airtime estimate using a previously initialized ampdu_aqm_timecalc struct
 *
 * Calculate a TX airtime estimate using a previously initialized ampdu_aqm_timecalc struct.
 * This function uses an initialzed timecalc struct to calculate the TX airtime for a
 * packet with the given MPDU length.
 * The time is the time cotribution of the A-MPDU sub-frame plus a weigthed portion of the
 * A-MPDU frame exchange.
 *
 * @param tc            pointer to a timecalc_t struct
 * @param mpdu_len      length of packet starting with 802.11 header not including the FCS
 *
 * @return              The estimated TX time of the packet
 */
static uint
wlc_ampdu_aqm_timecalc(ampdu_aqm_timecalc_t *tc, uint mpdu_len)
{
	uint mpdu_subframe_time;
	uint ampdu_overhead_time;
	uint Nsym;
	uint Ndbps = tc->Ndbps;

	/* number of symbols in the A-MPDU for this frame is
	 * CEILING( bits / Ndbps )
	 *   FLOOR( (bits + (Ndbps-1)) / Ndbps )
	 * where Ndbps is "Number of Data Bits per Symbol" for the current rate.
	 * Include the bits for an A-MPDU delimiter and FCS
	 */
	Nsym = ((((mpdu_len + AMPDU_DELIMITER_LEN + DOT11_FCS_LEN) * 8) + (Ndbps - 1)) /
	        Ndbps);

	/* Could wrap in the min AMPDU desity limit calc. If Nsym is less then the min density time,
	 * round up to the density value.
	 */

	/* time for the A-MPDU sub frame is symbols times symbol time */
	if (tc->sgi) {
		/* SGI symbol time is 3.6us, but needs to be rounded up to 4us sym time
		 * VHT_T_SYML * CIELING( (Nsym * 3.6) / 4 )
		 * VHT_T_SYML * CIELING( Nsym * (4 * (9 / 10)) / 4 )
		 * VHT_T_SYML * CIELING( Nsym * 36 / 40 )
		 * VHT_T_SYML * FLOOR( (Nsym * 36 + 39) / 40 )
		 */
		mpdu_subframe_time = VHT_T_SYML * ((Nsym * 36 + 39) / 40);
	} else {
		/* Regular GI symbol time is just VHT_T_SYML = 4 */
		mpdu_subframe_time = VHT_T_SYML * Nsym;
	}

	/* If the overhead weighted by the ratio of the MPDU length to the max possible
	 * A-MPDU length is larger than the the overhead divided by the max MPDUs, then
	 * use it.
	 * Otherwise, use the overhead divided by the max MPDUs.
	 *
	 * The motivation for this can be show with an example.
	 * Take an example connection that can have A-MPDUS with at most 8 MPDUs or 4K bytes per
	 * aggregate. If we construct an A-MPDU out of 8 tiny (100 byte) frames, then
	 * the 8 tiny frames will fill the A-MPDU even though they total only 800 bytes.
	 * The total A-MPDU overhead should be attributed to the 8 frames ---
	 * (total_overhead / 8) for each frame.
	 *
	 * But if we instead sent 1500byte frames, only 2 would fit in the aggregate. The
	 * total A-MPDU overhead should be attributed to the 2 frames ---
	 * (total_overhead * ( MPDU Len (1500) / Max A-MPDU len (4K) ) for each
	 * That would distribute 3000/4000 of the ampdu overhead total, closer
	 * to the actual airtime than (1/8) of the overhead for each frame.
	 */
	if (mpdu_len > tc->min_limit) {
		ampdu_overhead_time = (mpdu_len * tc->fixed_overhead_us) / tc->max_bytes;
	} else {
		ampdu_overhead_time = tc->fixed_overhead_pmpdu_us;
	}

	return (mpdu_subframe_time + ampdu_overhead_time);
} /* wlc_ampdu_init_aqm_timecalc */

#endif /* WLAMPDU_MAC */

#endif /* TXQ_MUX */


#ifdef BCMDBG_TXSTUCK
void
wlc_ampdu_print_txstuck(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	ampdu_tx_info_t *ampdu_tx = wlc->ampdu_tx;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	struct scb *scb;
	struct scb_iter scbiter;
	uint8 tid;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {

		if (!SCB_AMPDU(scb)) {
			continue;
		}

		scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);

		ASSERT(scb_ampdu);

		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
			ini = scb_ampdu->ini[tid];
			if (!ini) {
				continue;
			}

			bcm_bprintf(b, "tid(%d): %d in transit\n", tid, ini->tx_in_transit);
		}
	}
}
#endif /* BCMDBG_TXSTUCK */

#if defined(WLAMPDU_MAC)
void
wlc_ampdu_set_epoch(ampdu_tx_info_t *ampdu_tx, int ac, uint8 epoch)
{
	ASSERT(ac < AC_COUNT);
	ASSERT(AMPDU_AQM_ENAB(ampdu_tx->wlc->pub));

	ampdu_tx->hagg[ac].epoch = epoch;
	/* set prev_ft to AMPDU_11VHT, so wlc_ampdu_chgnsav_epoch will always be called. */
	ampdu_tx->hagg[ac].prev_ft = AMPDU_11VHT;
} /* wlc_ampdu_set_epoch */
#endif /* WLAMPDU_MAC */

#ifdef PROP_TXSTATUS
#ifdef WL_NATOE
int
wlc_ampdu_upd_last_suppr_seq(wlc_info_t *wlc, void *p, uint16 seq)
{
	/* code to support gating forward packets till suppressed pkts are transmitted. */
	scb_ampdu_tid_ini_t *ini;
	uint8 tid;
	struct scb *scb;
	scb_ampdu_tx_t *scb_ampdu;

	seq = WL_SEQ_GET_NUM(seq);
	scb = WLPKTTAGSCBGET(p);

	if (!scb || !wlc->ampdu_tx) {
		return BCME_NOTUP;
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);

	tid = (uint8)PKTPRIO(p);
	ini = scb_ampdu->ini[tid];

	if (!ini) {
		return BCME_NOTFOUND;
	}

	if (!WL_SEQ_GET_VALIDSUPPR(ini->last_suppr_seq)) {
		WL_SEQ_SET_NUM(ini->last_suppr_seq, seq);
		/* setting valid bit that indicates last_suppr_seq is valid */
		WL_SEQ_SET_VALIDSUPPR(ini->last_suppr_seq, 1);
	} else {
		if (IS_SEQ_ADVANCED(seq, WL_SEQ_GET_NUM(ini->last_suppr_seq))) {
			WL_SEQ_SET_NUM(ini->last_suppr_seq, seq);
		}
	}
	return BCME_OK;
}

int
wlc_ampdu_cleanup_last_suppr_seq(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	/* This code resets the last suppr sequence number in all the ampdu sessions.
	 * This is called when natoe feature is enabled to avoid dropping of forward path
	 * packets in the following problem scenario.
	 * Problem description:
	 * 1) let's say last suppr sequence number of a particular ampdu stream is marked valid,
	 *   when a connected station went to powersave.
	 * 2) Now NATOE feature is disabled and so WL_NATOE_ENAB() becomes zero.
	 * 3) Then if connected station comes out of powersave and thus all suppressed packets are
	 *    transmitted. since WL_NATOE_ENAB() is set to zero, code related to reseting
	 *    of last suppr sequence is not executed.
	 * 4) Now NATOE feature is enabled again.
	 * 5) As last suppr sequence is still set due to last NATOE session, forward packets
	 *   will be dropped expecting the suppressed packet from host->dongle tx flowring with
	 *  last suppr sequence to come.
	 * Solution is to reset last suppr seq when NATOE session is re-enabled.
	 */

	struct scb *scb;
	struct scb_iter scbiter;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint8 tid;

	if (!wlc->ampdu_tx) {
		return BCME_NOTUP;
	}

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		if (SCB_ASSOCIATED(scb)) {
			scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
			for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
				ini = scb_ampdu->ini[tid];
				if (!ini) {
					continue;
				}
				WL_SEQ_SET_VALIDSUPPR(ini->last_suppr_seq, 0);
			}
		}
	}

	return BCME_OK;
}
#endif /* WL_NATOE */
#endif /* PROP_TXSTATUS */

bool
wlc_ampdu_scbstats_get_and_clr(wlc_info_t *wlc, struct scb *scb,
	ampdu_tx_scb_stats_t *ampdu_scb_stats)
{
	ampdu_tx_info_t *ampdu_tx = wlc ? wlc->ampdu_tx : NULL;
	scb_ampdu_tx_t *scb_ampdu = NULL;

	if ((!ampdu_scb_stats) || (!ampdu_tx))
		return FALSE;

	if (SCB_AMPDU(scb)) {
		scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
		memcpy(ampdu_scb_stats, scb_ampdu->ampdu_scb_stats, sizeof(ampdu_tx_scb_stats_t));
		memset(scb_ampdu->ampdu_scb_stats, 0, sizeof(ampdu_tx_scb_stats_t));
		return TRUE;
	}

	return FALSE;
}

void
wlc_ampdu_reset_txnoprog(ampdu_tx_info_t *ampdu_tx)
{
	if (!ampdu_tx) {
		return;
	}
	ampdu_tx->txnoprog_cnt = 0;
}

#ifdef WL11K
void wlc_ampdu_get_stats(wlc_info_t *wlc, rrm_stat_group_12_t *g12)
{
	ASSERT(wlc);
	ASSERT(g12);

#ifdef WLAMPDU_MAC
	if (wlc->clk && AMPDU_MAC_ENAB(wlc->pub)) {
		if (AMPDU_AQM_ENAB(wlc->pub)) {
			uint16 stat_addr = 0;
			uint16 val;
			uint32 total;
			int cnt = 0, i;

			stat_addr = 2 * wlc_read_shm(wlc, M_AMP_STATS_PTR(wlc));
			for (i = 0, total = 0; i < C_MPDUDEN_NBINS; i++) {
				val = wlc_read_shm(wlc, stat_addr + i * 2);
				total += val;
				cnt += (val * (i+1));
			}
			g12->txampdu = total;
			g12->txmpdu = cnt;
		} else {
			g12->txampdu = WLCNTVAL(MCSTVAR(wlc->pub, txampdu));
			g12->txmpdu = WLCNTVAL(MCSTVAR(wlc->pub, txmpdu));
		}
	} else
#endif /* WLAMPDU_MAC */
	{
		g12->txampdu = wlc->ampdu_tx->cnt->txampdu;
		g12->txmpdu = wlc->ampdu_tx->cnt->txmpdu;
	}
	g12->txampdubyte_h = wlc->ampdu_tx->cnt->txampdubyte_h;
	g12->txampdubyte_l = wlc->ampdu_tx->cnt->txampdubyte_l;

	/* from wlc_ampdu_rx.c */
	g12->rxampdu = wlc_ampdu_getstat_rxampdu(wlc);
	g12->rxmpdu = wlc_ampdu_getstat_rxmpdu(wlc);
	g12->rxampdubyte_h = wlc_ampdu_getstat_rxampdubyte_h(wlc);
	g12->rxampdubyte_l = wlc_ampdu_getstat_rxampdubyte_l(wlc);
	g12->ampducrcfail = wlc_ampdu_getstat_ampducrcfail(wlc);
}
#endif /* WL11K */
