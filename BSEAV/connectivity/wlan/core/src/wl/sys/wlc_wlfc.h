/*
 * Propagate txstatus (also flow control) interface.
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

#ifndef __wlc_wlfc_h__
#define __wlc_wlfc_h__

#include <typedefs.h>
#include <wlc_types.h>
#include <wlfc_proto.h>

/* from wl_export.h */
void wlfc_process_txstatus(wlc_wlfc_info_t *wlfc, uint8 status, void *pkt,
	void *txs, bool hold);
int wlfc_push_credit_data(wlc_wlfc_info_t *wlfc, void *pkt);
int wlfc_push_signal_data(wlc_wlfc_info_t *wflc, uint8 *data, uint8 len, bool hold);
int wlfc_sendup_ctl_info_now(wlc_wlfc_info_t *wlfc);
uint32 wlfc_query_mode(wlc_wlfc_info_t *wlfc);
void wlfc_enable_cred_borrow(wlc_info_t *wlc, uint32 bEnable);
/* called from wl_rte.c */
void wlc_send_credit_map(wlc_info_t *wlc);
int wlc_send_txstatus(wlc_info_t *wlc, void *pkt);
int wlc_sendup_txstatus(wlc_info_t *wlc, void **pkt);
#ifdef PROP_TXSTATUS_DEBUG
void wlc_wlfc_info_dump(wlc_info_t *wlc);
#endif

/* new interfaces */
void wlc_wlfc_psmode_request(wlc_wlfc_info_t *wlfc, struct scb *scb,
	uint8 credit, uint8 precedence_bitmap, wlfc_ctl_type_t request_type);
int wlc_wlfc_mac_status_upd(wlc_wlfc_info_t *wlfc, struct scb *scb,
	uint8 open_close);
bool wlc_wlfc_suppr_status_query(wlc_info_t *wlc, struct scb *scb);

/* from wlc_p2p.h */
int wlc_wlfc_interface_state_update(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wlfc_ctl_type_t state);
void wlc_wlfc_flush_pkts_to_host(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
void wlc_wlfc_flush_queue(wlc_info_t *wlc, struct pktq *q);

/* from wlc.h */
void wlc_txfifo_suppress(wlc_info_t *wlc, struct scb *scb);
void wlc_process_wlhdr_txstatus(wlc_info_t *wlc, uint8 status, void *pkt, bool hold);
bool wlc_suppress_sync_fsm(wlc_info_t *wlc, struct scb *scb, void *pkt, bool suppress);
uint8 wlc_txstatus_interpret(tx_status_macinfo_t *txstatus,  int8 acked);
int16 wlc_link_txflow_scb(wlc_info_t *wlc, struct wlc_if *wlcif, uint16 flowid, uint8 op,
	uint8 * sa, uint8 *da, uint8 tid);

/* new for wlc_apps.c */
void wlc_wlfc_scb_ps_on(wlc_info_t *wlc, struct scb *scb);
void wlc_wlfc_scb_ps_off(wlc_info_t *wlc, struct scb *scb);
void wlc_wlfc_scb_psq_resp(wlc_info_t *wlc, struct scb *scb);
bool wlc_wlfc_scb_psq_chk(wlc_info_t *wlc, struct scb *scb, uint16 fc, struct pktq *pktq);
uint wlc_wlfc_scb_ps_count(wlc_info_t *wlc, struct scb *scb, bool bss_ps);

/* from wlc_mchan.h */
int wlc_wlfc_mchan_interface_state_update(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	wlfc_ctl_type_t open_close, bool force_open);

void wlc_wlfc_dotxstatus(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct scb *scb,
	void *pkt, tx_status_t *txs);

/* attach/detach */
wlc_wlfc_info_t *wlc_wlfc_attach(wlc_info_t *wlc);
void wlc_wlfc_detach(wlc_wlfc_info_t *wlfc);

#ifdef WLATF_DONGLE
/* Max number of samples in running buffer:
 * Keep max samples number a power of two, thus
 * all the division would be a single-cycle
 * bit-shifting.
 */
#define RAVG_EXP_PKT 2
#define RAVG_EXP_WGT 2

typedef struct _ravg {
	uint32 sum;
	uint8 idx;
} wlc_ravg_info_t;


#define RAVG_IDX(_obj_) ((_obj_)->idx)
#define RAVG_SUM(_obj_) ((_obj_)->sum)
#define RAVG_AVG(_obj_, _exp_) ((_obj_)->sum >> (_exp_))

/* Basic running average algorithm:
 * Keep a running buffer of the last N values, and a running SUM of all the
 * values in the buffer. Each time a new sample comes in, subtract the oldest
 * value in the buffer from SUM, replace it with the new sample, add the new
 * sample to SUM, and output SUM/N.
 */
#define RAVG_ADD(_obj_, _buf_, _sample_, _exp_) \
{ \
	if ((_buf_) != NULL) { \
		RAVG_SUM((_obj_)) -= _buf_[RAVG_IDX((_obj_))]; \
		RAVG_SUM((_obj_)) += (_sample_); \
		(_buf_)[RAVG_IDX((_obj_))] = (_sample_); \
		RAVG_IDX((_obj_)) = (RAVG_IDX((_obj_)) + 1) % (1 << (_exp_)); \
	} \
}

/* Initializing running buffer with value (_sample_) */
#define RAVG_INIT(_obj_, _buf_, _sample_, _exp_) \
{ \
	int v_ii; \
	if ((_obj_) != NULL) \
		memset((_obj_), 0, sizeof(*(_obj_))); \
	if ((_buf_) != NULL) { \
		memset((_buf_), 0, (sizeof((_buf_)[0]) * (1 << (_exp_)))); \
		for (v_ii = 0; v_ii < (1 << (_exp_)); v_ii++) { \
			RAVG_ADD((_obj_), (_buf_), (_sample_), (_exp_)); \
		} \
	} \
}
#define SZ_FLR_RBUF_TXPKTLEN (1 << RAVG_EXP_PKT)
#define SZ_FLR_RBUF_WEIGHT	(1 << RAVG_EXP_WGT)
#define PRIOMAP(_wlc_) ((_wlc_)->pciedev_prio_map)

#if defined(FLOW_PRIO_MAP_AC)
#define FLOWRING_PER_SCB_MAX AC_COUNT
#define RAVG_PRIO2FLR(_map_, _prio_) WME_PRIO2AC((_prio_))
#else  /* !FLOW_PRIO_MAP_AC */
#define FLOWRING_PER_SCB_MAX NUMPRIO
#define RAVG_PRIO2FLR(_map_, _prio_) \
	((((_map_) == PCIEDEV_AC_PRIO_MAP) ? WME_PRIO2AC((_prio_)) : (_prio_)))
#endif  /* FLOW_PRIO_MAP_AC */

#define TXPKTLEN_RAVG(_atfd_, _ac_) (&(_atfd_)->flr_txpktlen_ravg[(_ac_)])
#define TXPKTLEN_RBUF(_atfd_, _ac_) ((_atfd_)->flr_txpktlen_rbuf[(_ac_)])
#define WEIGHT_RAVG(_atfd_, _ac_) (&(_atfd_)->flr_weigth_ravg[(_ac_)])
#define WEIGHT_RBUF(_atfd_, _ac_) ((_atfd_)->flr_weight_rbuf[(_ac_)])

typedef struct wlc_atfd {
		/* Running average buffer for tx packet's len per flow ring */
		uint16 flr_txpktlen_rbuf[FLOWRING_PER_SCB_MAX][SZ_FLR_RBUF_TXPKTLEN];
		/* Running average buffer for weight per flow ring */
		uint32 flr_weight_rbuf[FLOWRING_PER_SCB_MAX][SZ_FLR_RBUF_WEIGHT];
		/* Running average info for tx packet len per flow ring */
		wlc_ravg_info_t flr_txpktlen_ravg[FLOWRING_PER_SCB_MAX];
		/* Running average info for weight per flow ring */
		wlc_ravg_info_t flr_weigth_ravg[FLOWRING_PER_SCB_MAX];
} wlc_atfd_t;

extern wlc_atfd_t *wlfc_get_atfd(wlc_info_t *wlc, struct scb *scb);
extern void wlc_scb_upd_all_flr_weight(wlc_info_t *wlc, struct scb *scb);
extern void wlc_ravg_add_weight(wlc_atfd_t *atfd, int fl, ratespec_t rspec);
#endif /* WLATF_DONGLE */

#if defined(BCMPCIEDEV) && defined(BCMLFRAG) && defined(PROP_TXSTATUS)
extern uint32 wlc_suppress_recent_for_fragmentation(wlc_info_t *wlc, void *sdu, uint nfrags);
#endif

#endif /* __wlc_wlfc_h__ */
