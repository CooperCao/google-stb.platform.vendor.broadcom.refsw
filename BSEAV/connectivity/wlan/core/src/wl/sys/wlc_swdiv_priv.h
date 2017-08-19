/*
* SW Diversity private header file
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
*
*/

#ifndef _wlc_swdiv_priv_h_
#define _wlc_swdiv_priv_h_

#include <wlc_swdiv.h>

/* top level design assumption about how many phycores will be max */
#define MAX_ANTNUM_PER_CORE	(2)	/* System deicision dependent */
#define MAX_PHYCORES_PER_SLICE	(4)	/* Max limit number of PHYCORES per each slice */
#define MAX_HW_ANTNUM	(MAX_ANTNUM_PER_CORE * MAX_PHYCORES_PER_SLICE)

/* uCode design dependents */
#define MAX_UCODE_COREBITS	(4)	/* # of bits for coremap in PhyRxStatus */
#define MAX_UCODE_ANTBITS	(4)	/* # of bits for antmap in PhyRxStatus */
#define MAX_BAND_TYPES	(2)	/* 2G and 5G */
#define SWDIV_PREF_ANT_SHMEM_MASK	(0xFF)	/* [7:0] 8 bits */

/*
 * swdiv control gpio pins can be assigned as followings:
 * gpioctrl reg in chipcommon will set by [CTRLBITS] << [OFFSET]
 * uCode controls psm_gpio_oe based on this bit setting.
 * limitation is that the pin assign have to be sequencial.
 * set 6/7/8 gpio active pins as a default, if there is no def.
 */
#ifndef WLC_SWDIV_GPIO_OFFSET
/* board specific gpio starting offset as a default */
#define WLC_SWDIV_GPIO_OFFSET	(6)
#endif /* WLC_SWDIV_GPIO_OFFSET */
/* sequancial bits to activate */
#ifndef WLC_SWDIV_GPIO_CTRLBITS
#define WLC_SWDIV_GPIO_CTRLBITS	(0x7)
#endif /* WLC_SWDIV_GPIO_CTRLBITS */

/* policy request format -some will need to be cleaned-up */
#define SWDIV_ANT0	(0)
#define SWDIV_ANT1	(1)
#define SWDIV_ANT2	(2)
#define SWDIV_NOISE_INIT	(-94)
#define SWDIV_NULL_ANTMAP	(0xFFFF)

#define SWDIV_SNR_QFORMAT	(3)
#define SWDIV_1BY8_DB		(1 << SWDIV_SNR_QFORMAT)
#define SWDIV_RXPKTCNT_THRESHOLD_DEFAULT	(2000)
#define SWDIV_RXPKTCNT_THRESHOLD_VHT		(10000)
#define SWDIV_SNR_WEIGHT_1STANT	(0)

#define SWDIV_SNR_DIFF_THRESHOLD_DEFAULT	(3 << SWDIV_SNR_QFORMAT)
#define SWDIV_SNR_DIFF_THRESHOLD_ACPHY		(1 << SWDIV_SNR_QFORMAT)
#define SWDIV_SNR_DIFF_THRESHOLD_DURING_SETTLING	(1 << SWDIV_SNR_QFORMAT)

#define SWDIV_SNRDROP_THRESH_DEFAULT	(5 << SWDIV_SNR_QFORMAT)
#define SWDIV_SNRDROP_MINSNR_VAL	(9 << SWDIV_SNR_QFORMAT)

#define SWDIV_SNR_LIMIT_DEFAULT	(20 << SWDIV_SNR_QFORMAT)
#define SWDIV_SNR_LIMIT_ACPHY	(25 << SWDIV_SNR_QFORMAT)

#define SWDIV_SNRDROP_POLL_PRD_DEFAULT	(10)
#define SWDIV_SNRDROP_TXFAIL_DEFAULT	(1 << SWDIV_SNR_QFORMAT)
#define SWDIV_SNRDROP_TICK_DUR_SEC	(3)
#define MAX_RSSI_CAP	(-5)
#define MIN_RSSI_CAP	(-99)

#define SWDIV_SETTLE_RX_COUNT_LIMIT	(60)
#define SWDIV_LINKMON_DUR_DEFAULT	(300)	/* ms unit */
#define SWDIV_LINKMON_EXTRA_DUR	(10)	/* ms unit */

#define SWDIV_LOG2_NUM_PKT_AVG	(3)
#define SWDIV_SNR_MAX_DEFAULT	(25)
#define SWDIV_SNR_MIN_DEFAULT	(3)
#define SWDIV_SNR_MAX_ACPHY	(28)
#define SWDIV_SNR_MIN_ACPHY	(0)

#define SWDIV_IS_ENABLED(swdiv)	(swdiv->enable)

#define SWDIV_PLCY_CORES	(4)
#define SWDIV_PLCY_BITS_PER_CORE	(SWDIV_PLCY_CORES * MAX_BAND_TYPES)

/* rx,tx policy control macros */
#define SWDIV_PLCY_CLR(policy, band, coreshift)	\
	(policy & (uint32)~(SWDIV_PLCY_MASK << SWDIV_PLCY_TOT_SHIFT(band, coreshift)))
#define SET_TYPE_IN_PLCY(plcytype, plcy, band, coreshift)	\
	(plcy | (uint32)(plcytype << SWDIV_PLCY_TOT_SHIFT(band, coreshift)))
#define SWDIV_PLCY_SET(policy, band, coreshift, val)	do {	\
	policy = SWDIV_PLCY_CLR(policy, band, coreshift);	\
	policy = SET_TYPE_IN_PLCY(val, policy, band, coreshift); } while (0)

/* cellon, celloff policy control macros */
#define SWDIV_CPLCY_BAND_MASK		(0x3)
#define SWDIV_CPLCY_BAND_OFFSET(band)	((band == SWDIV_BANDSEL_2G) ? 0 : 2)
#define SWDIV_CPLCY_CELLSTAT_OFFSET(cellstat)	((cellstat == SWDIV_CELL_ON) ? 0 : 4)
#define SWDIV_CPLCY_TOT_SHIFT(band, cellstat, coreshift)		\
	(SWDIV_CPLCY_BAND_OFFSET(band) + SWDIV_CPLCY_CELLSTAT_OFFSET(cellstat)	\
	+ coreshift)
#define SWDIV_CPLCY_CLR(policy, band, cellstat, coreshift)	\
	(policy & (uint32)~(SWDIV_CPLCY_BAND_MASK	\
	<< SWDIV_CPLCY_TOT_SHIFT(band, cellstat, coreshift)))
#define SET_TYPE_IN_CPLCY(val, cellplcy, band, cellstat, coreshift)	\
	(cellplcy | (uint32)(val	\
	<< SWDIV_CPLCY_TOT_SHIFT(band, cellstat, coreshift)))
#define SWDIV_CPLCY_SET(policy, band, cellstat, coreshift, val)		do {	\
	policy = SWDIV_CPLCY_CLR(policy, band, cellstat, coreshift);	\
	policy = SET_TYPE_IN_CPLCY(val, policy, band, cellstat, coreshift); } while (0)
#define SWDIV_CPLCY_GET(policy, band, cellstat, coreshift)	\
	((policy & (uint32)(SWDIV_CPLCY_BAND_MASK	\
	<< SWDIV_CPLCY_TOT_SHIFT(band, cellstat, coreshift)))	\
	>> SWDIV_CPLCY_TOT_SHIFT(band, cellstat, coreshift))

/* corelist control macros */
#define FOREACH_SWDIV_CORE(clist)	\
	for (; clist != NULL; clist = clist->next)
#define FOREACH_SWDIV_ACTCORE(clist)	\
	for (; clist != NULL && clist->active; clist = clist->next)
#define FOREACH_SWDIV_ACTBAND_CORE(clist, band)	\
	for (; (clist != NULL && clist->active && \
			((SWDIV_BANDSEL_2G == band && clist->support_2g) ||	\
			(SWDIV_BANDSEL_5G == band && clist->support_5g)));	\
			clist = clist->next)
/* corechain control macros */
#define SWDIV_SETBIT_ON_MAP(map, idx, set)	do { if (set == 0) { map &= ~(1 << idx); }	\
	else { map |= (1 << idx); } } while (0)
#define SWDIV_ISBIT_SET_ON_MAP(map, idx)	(map & (1 << idx))
#define SWDIV_GETBIT_BY_MAP(map, idx)	(SWDIV_ISBIT_SET_ON_MAP(map, idx) >> idx)
#define SWDIV_GETSIDX(antnum, idx)	((idx << 1) + antnum)
#define SWDIV_GETBASE_SIDX(idx)	(idx << 1)
#define FOREACH_SWDIV_COREMAP(map, cnt)	\
	for (cnt = 0; cnt < MAX_UCODE_COREBITS; cnt++)
#define FOREACH_SWDIV_ANTMAP(map, cnt)	\
	for (cnt = 0; cnt < MAX_UCODE_ANTBITS; cnt++)
/* ant preference control macros */
#define ANTPREF_MASK	(0x3)
#define SWDIV_GET_ANTPREF(antpref, corecnt)	\
	((antpref >> (corecnt*2)) & ANTPREF_MASK)
/* snr computing control macros */
#define LOG10_CONV(value)	(((int32)(value) * 10)/33)
#define QDOT1_CONV(value)	((int32)(value)/2)

/* uCode shmem control info of switch control type */
enum {
	SWDIV_SWCTRL_SHMEM_OFF =	0x0,
	SWDIV_SWCTRL_SHMEM_GPIO =	0x1,
	SWDIV_SWCTRL_SHMEM_SWCTRL =	0x2,
	SWDIV_SWCTRL_SHMEM_MAX	/* end */
};
enum {
	SWDIV_CELL_ON =	0x0,
	SWDIV_CELL_OFF =	0x1,
	SWDIV_CELL_END	/* end */
};
enum {
	SWDIV_ANT0_INUSE =	0x0,
	SWDIV_ANT1_INUSE =	0x1,
	SWDIV_ANT_INVALID	/* end */
};

/* diversity events when the selection is triggerd */
enum {
	SWDIV_EVENT_IDLE =	0,
	SWDIV_EVENT_RXCOUNT =	1,
	SWDIV_EVENT_SNRTHRSH =	2,
	SWDIV_EVENT_TXFAIL =	3,
	SWDIV_EVENT_TIMER =	4,
	SWDIV_EVENT_WATCHDOG =	5,
	SWDIV_EVENT_SNRDROP =	6,
	SWDIV_EVENT_MAX	/* end */
};

/* any ant preference requester can select one of them */
enum {
	SWDIV_ANTPREF_ALLOFF =	0x0,
	SWDIV_ANTPREF_FORCE0 =	0x1,
	SWDIV_ANTPREF_FORCE1 =	0x2,
	SWDIV_ANTPREF_NOIMPACT =	0x3,
	SWDIV_ANTPREF_INVALID	/* end */
};
#ifdef BCMDBG
typedef struct swdiv_evtname_tbl {
	char str[64];
} swdiv_evtname_tbl_t;
#endif /* BCMDBG */

/* keep tracking the change of the ant selection */
typedef struct swdiv_antsel_info {
	uint8	cur;		/* current active ant info */
	uint8	requested;	/* requested ant info */
} swdiv_antsel_info_t;

/* asny policy input will use this struct
 * each core has [4bits-5G][4bits-2G] as 8 bits total
 * 4 phycores can be configured due to 32bits limitation.
 * the new design is targeting 8 cores at least,
 * to support this, we need to consider extended policy iovars.
 */
typedef struct swdiv_policy_req_info {
	uint32 rx;
	uint32 tx; /* deprecated */
	uint32 cell;
	uint32 rx_ext;
	uint32 tx_ext;
	uint32 cell_ext;
} swdiv_policy_req_info_t;

typedef struct swdiv_txpwrcap_unit {
	int8 _2g;
	int8 _5gL;
	int8 _5gM;
	int8 _5gH;
	int8 _5gExt;
} swdiv_txpwrcap_unit_t;

/* sw diversity algorithm control factors as tunables */
typedef struct swdiv_algo_tune_info {
	int16 snr_healthy_limit;	/* Skip SNR check if the level better than this limit */
	int16 snr_diff_thresh;		/* if SNR difference greater than this then swap ant */
	int16 snrdec_upon_txfail;	/* SNR drop upon tx fail. In Q.3 format */
	int16 snrdrop_valid_limit;	/* SNR limit below which the SNR drop check is invalid */
	int16 snrdrop_delta;		/* swap ant if the SNR drop is greater than this val */
	uint8 snrdrop_period;		/* snrdrop check interval checking snrdrop_tick_wdog */
	uint8 snr_poll_prd;			/* polling duration checking polltime_elapsed */
	uint32 rxpktcnt_thresh;		/* rx pkt count threshold before ant swap */
	uint32 settle_rxpktcnt_thresh;	/* Threshold for rx count for initial settling */
	uint32 settle_rxpktcnt_limit;	/* Total Num of packets for settling */
	uint16 cfgmon_dur;			/* BSSCFG monitoring timer duration */
	/* Skew factor in 1/8dB steps added to SNR of 1st ant per core */
	int8 snr_weight_firstant;
	int8 _pad;	/* 8bits alignment padding */
} swdiv_algo_tune_info_t;

/* sw diversity core entity info */
/* the scope of this module is to take 'any HW constraint factor or SW selection status' control */
typedef struct swdiv_core_entity {
	struct swdiv_core_entity *next;	/* core list pointer */
	uint8 coreid;		/* unique core id per each phycore */
	bool active;		/* per-chain control by uCode coremap bits */
	bool support_2g;	/* band specific activation pulled from NVRAM */
	bool support_5g;
	uint8 rx_curplcy[MAX_BAND_TYPES];	/* final policy decision for rx */
	uint8 rxplcy[MAX_BAND_TYPES];		/* rx_policy type */
	uint8 cellplcy_on[MAX_BAND_TYPES];	/* cell_policy type on cellur active */
	uint8 cellplcy_off[MAX_BAND_TYPES];	/* cell_policy type on cellur inactive */
	swdiv_antsel_info_t rxsel;			/* ant sel between 1stant and 2ndant */
	/* TXPWRCAP info */
	swdiv_txpwrcap_unit_t pwrtbl_cellon;
	swdiv_txpwrcap_unit_t pwrtbl_celloff;
	uint8 mwsplcy[MAX_BAND_TYPES];		/* mws policy update from LTE */
} swdiv_core_entity_t;

/* common control params */
struct wlc_swdiv_info {
	wlc_info_t *wlc;
	wlc_pub_t *pub;
	int cfgh;
	int scbh;
	bool enable;	/* Top control switch for sw diversity */
	bool cellstatus;				/* LTE status update */
	bool mws_antsel_ovr_tx; /* controlled by mws_antenna_selection iovar */
	bool mws_antsel_ovr_rx; /* controlled by mws_antenna_selection iovar */
	uint32 userantmap;				/* antmap requested by policy or antpref */
	struct swdiv_core_entity *corelist;	/* shared corechain list */

	/* HW configuration params from NVRAM/SROM */
	wlc_swdiv_swctrl_t	swctrl_en;
	uint8 log2_num_pkt_avg;
	/* gpio ctrl pin start offset. need to assign consequtively  */
	uint8 gpio_offset;
	uint16 gpio_ctrl;		/* switch ctrl pin selection bits */
	uint16 swctrl_mask;
	uint16 swctrl_ant0;
	uint16 swctrl_ant1;
	int16 cck_snr_corr;

	/* HW SW diversity activation map from NVRAM/SROM */
	uint8 antmap2g_main;	/* 2g d11main core swdiv enable bits */
	uint8 antmap5g_main;	/* 5g d11main core swdiv enable bits */
	uint8 antmap2g_aux;		/* 2g d11aux core swdiv enable bits */
	uint8 antmap5g_aux;		/* 5g d11aux core swdiv enable bits */
	swdiv_algo_tune_info_t deftune;	/* sw tunables default set from NVRAM/SROM */
	swdiv_policy_req_info_t policy;	/* ant selection policy as a default set */
	wl_txpwrcap_tbl_t *txpwrcap;	/* tx powercap per-subband / per-ant */
	uint32 mws_antpref;		/* upper 16bits for 5g and lower 16bits for 2g */
};

/* Per-scb stats info in the current consideration */
typedef struct wlc_swdiv_status {
	uint8 swaprsn_cur;				/* latest swap reason id */
	uint32 rxpktcnt_accumulated;	/* total accumulated rxpktcnt */
	/* per-ant bassis stats */
	uint8 swaprsn_prev[MAX_HW_ANTNUM];		/* collect swap reason */
	uint32 swapcnt_snrdrop[MAX_HW_ANTNUM];	/* swap count when snrdropped */
	uint32 swapcnt_linkmon[MAX_HW_ANTNUM];	/* swap count when linkmon expires */
	uint32 swap_ge_rxcnt[MAX_HW_ANTNUM];	/* swap count when rxpkt hits thsh */
	uint32 swap_ge_snrthresh[MAX_HW_ANTNUM];	/* swap count when snr thsh reached */
	uint32 swap_txfail[MAX_HW_ANTNUM];		/* swap count when tx failure happens */
	uint32 swap_alivechk[MAX_HW_ANTNUM];	/* swap count when rxpktbased chk hits */
	int16 snr_prev_avg_per_ant[MAX_HW_ANTNUM];	/* previous average snr per-ant */
	uint32 rxpktcnt_total_per_ant[MAX_HW_ANTNUM];	/* total rxpkt count */
	/* uCode sync stats */
	uint16 lastcoremap;		/* latest corebits from latest rxpkt for this scb link */
	uint16 lastantmap;		/* active antenna bits from latest rxpkt */
	uint16 reqantmap;		/* swdiv algo requested antmap change to uCode   */
	chanspec_t lastchanspec;	/* Band specific update for any tracking */
} wlc_swdiv_status_t;

/* Generic link monitoring struct for SCB and BSSCFG */
typedef struct swdiv_link_mon {
	struct wl_timer *ptimer;	/* alive check timer handler */
	bool is_snrdropped;
	bool is_timerset;
} swdiv_link_mon_t;

/* Interface type oriented params */
typedef struct swdiv_bsscfg_cubby {
	bool auto_en;
	uint16 defantmap;				/* initial antmap in bsscfg up */
	swdiv_link_mon_t *cfgmon;		/* Any BSSCFG based monitoring */
	swdiv_algo_tune_info_t tune;	/* SWDIV algo behavior tunables */
} swdiv_bsscfg_cubby_t;

/* Link basis control params */
typedef struct swdiv_scb_cubby {
	wlc_swdiv_info_t *swdiv;
	/* per-ant type link status */
	int16 snr_avg_per_ant[MAX_HW_ANTNUM];	/* average snr */
	int16 snr_sum_per_ant[MAX_HW_ANTNUM];	/* summation snr */
	int16 noisest_per_ant[MAX_HW_ANTNUM];	/* noise estimation */
	uint32 rxpktcnt_per_ant[MAX_HW_ANTNUM];	/* rxpkt after swap happened */
	uint32 rxpktcnt_prev_per_ant[MAX_HW_ANTNUM];	/* previous rxpkt */
	/* per-core basis ctrl params */
	uint16 polltime_elapsed[MAX_UCODE_COREBITS];	/* watchdog polling count */
	uint32 settlecnt_rx;		/* rxpkt counter in settling period */
	uint32 snrdrop_tick_wdog;	/* snrdrop monitoring per snrdrop_period */
	uint16 linkmon_dur;			/* deadlink monitor timer duration */
	swdiv_link_mon_t *linkmon;	/* dead link monitoring timer */
	wlc_swdiv_status_t *stats;	/* tracking info for debuggability */
} swdiv_scb_cubby_t;

/* cubby handling macros */
#define SWDIV_BSSCFG_CUBBY_LOC(swdiv, cfg) \
	((swdiv_bsscfg_cubby_t **)BSSCFG_CUBBY(cfg, (swdiv)->cfgh))
#define SWDIV_BSSCFG_CUBBY(swdiv, cfg) (*SWDIV_BSSCFG_CUBBY_LOC(swdiv, cfg))
#define SWDIV_SCB_CUBBY_LOC(swdiv, scb) ((swdiv_scb_cubby_t **)SCB_CUBBY((scb), (swdiv)->scbh))
#define SWDIV_SCB_CUBBY(swdiv, scb)		(*(SWDIV_SCB_CUBBY_LOC(swdiv, scb)))

/* cubby-related function prototypes */
static int wlc_swdiv_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_swdiv_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
static int wlc_swdiv_scb_init(void *ctx, struct scb *scb);
static void wlc_swdiv_scb_deinit(void *ctx, struct scb *scb);
static uint wlc_swdiv_scb_secsz(void *ctx, struct scb *scb);
static int wlc_swdiv_scb_update(void *ctx, struct scb *scb, wlc_bsscfg_t* new_cfg);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
wlc_swdiv_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
static void
wlc_swdiv_scb_dump(void *ctx, struct scb *scb, struct bcmstrbuf *b);
static void
wlc_swdiv_corelist_dump(swdiv_core_entity_t *clist, struct bcmstrbuf *b);
#else
#define wlc_swdiv_bsscfg_dump	NULL
#define wlc_swdiv_scb_dump		NULL
#define wlc_swdiv_corelist_dump	NULL
#endif /* BCMDBG || BCMDBG_DUMP */

#endif /* _wlc_swdiv_priv_h_ */
