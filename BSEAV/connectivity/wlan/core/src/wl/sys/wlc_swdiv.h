/*
 * Software diversity module header file
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

#ifndef _wlc_swdiv_h_
#define _wlc_swdiv_h_

#include <phy_antdiv_api.h>

/* external input entities */
typedef enum swdiv_requester_type {
	SWDIV_REQ_FROM_IOVAR =   0x0,   /* user trigger */
	SWDIV_REQ_FROM_LTE =     0x1,   /* celluar component for LTECOEX */
	SWDIV_REQ_FROM_BT =      0x2,   /* BTCOEX, currently not considered */
	SWDIV_REQ_MAX                   /* end */
} swdiv_requester_type_t;

typedef enum swdiv_bandsel {
	SWDIV_BANDSEL_2G =     0,
	SWDIV_BANDSEL_5G =     1,
	SWDIV_BANDSEL_BOTH =   2,  /* not used yet */
	SWDIV_BANDSEL_MAX          /* end */
} swdiv_bandsel_t;

/* operation policy control.
 * get priorizing between other component like LTE
 */
typedef enum swdiv_policy_type {
	SWDIV_POLICY_FORCE_0 =     0x0,   /* use antenna 0 */
	SWDIV_POLICY_FORCE_1 =     0x1,   /* use antenna 1 */
	SWDIV_POLICY_ALLOFF =      0x2,   /* not supported yet */
	SWDIV_POLICY_AUTO =        0x3,   /* auto selection between 0/1 */
	SWDIV_POLICY_FOLLOW_CELL = 0x4,   /* follow cell policy */
	SWDIV_POLICY_INVALID =     0x5,   /* not valid type */
	SWDIV_POLICY_MAX                  /* end */
} swdiv_policy_type_t;

/* control MACROs */
/* first implementation stage will cover infra STA mode and P2PGC.
 * AWDL/TDLS will be covered later on
 */
#define WLSWDIV_BSSCFG_SUPPORT(cfg)	(wlc_swdiv_bss_is_supported(cfg))

/* exposed definition for other modules */
#define SWDIV_PLCY_MASK	(0xF)
#define SWDIV_PLCY_BANDOFFSET(band)	((band == SWDIV_BANDSEL_2G) ? 0 : 4)
#define SWDIV_PLCY_TOT_SHIFT(band, coreshift)	\
	(SWDIV_PLCY_BANDOFFSET(band) + coreshift)
#define SWDIV_PLCY_GET(policy, band, coreshift)	\
	((policy & (uint32)(SWDIV_PLCY_MASK << SWDIV_PLCY_TOT_SHIFT(band, coreshift)))	\
	>> SWDIV_PLCY_TOT_SHIFT(band, coreshift))
/* ------------------
 *  Open APIs
 * ------------------
 */

/* sw diversity module interface for attach */
extern wlc_swdiv_info_t *wlc_swdiv_attach(wlc_info_t *wlc);

/* sw diversity module interface for detach */
extern void wlc_swdiv_detach(wlc_swdiv_info_t *swdiv);

/* celluar status update API */
extern void wlc_swdiv_cellstatus_notif(wlc_swdiv_info_t *swdiv, int cellstatus);

/* user update for ant map preference info
 * lte will mainly use this func
 */
extern int32 wlc_swdiv_antpref_update(wlc_swdiv_info_t *swdiv,
	swdiv_requester_type_t reqid,
	uint16 rxantpref2g, uint16 rxantpref5g, uint16 txantpref2g, uint16 txantpref5g);


/* Policy control param input
 * Currently support 4 cores but when uCode is capable to report 8 cores,
 * have to use extend param (xxx_ext)
 */
extern int32 wlc_swdiv_policy_req(wlc_swdiv_info_t *swdiv,
	uint32 rx_policy, uint32 rx_policy_ext,
	uint32 tx_policy, uint32 tx_policy_ext,
	uint32 cell_policy, uint32 cell_policy_ext);

/* snr computing and swap by tx failure */
extern void wlc_swdiv_txfail(wlc_swdiv_info_t *swdiv, struct scb *scb);

/* sw diversity info update per pkt
 * uCode will give the full info set for antmap and coremap per pkt.
 * snr computing and sw algo triggering will be handled by this func
 */
extern void wlc_swdiv_rxpkt_recv(wlc_swdiv_info_t *swdiv,
	struct scb *scb, wlc_d11rxhdr_t *wrxh, bool isCCK);

/* reset stats info */
extern void wlc_swdiv_reset_stats(wlc_swdiv_info_t *swdiv, struct scb *scb);

/* get swdiv band specific support info */
extern bool wlc_swdiv_supported(wlc_swdiv_info_t *swdiv, int coreidx,
	bool is2g, bool isokanyband);

/* top control kill switch; e.g.,LPAS need to turn off swdiv */
extern void wlc_swdiv_enable_set(wlc_swdiv_info_t *swdiv, bool enable);

/* initialize antmap based upon policy setting
 * return : generated antmap bits
 */
extern uint16 wlc_swdiv_antmap_init(wlc_swdiv_info_t *swdiv);

/* swdiv gpio configuration for gpio init module */
extern void wlc_swdiv_gpio_info_get(wlc_swdiv_info_t *swdiv,
	uint8 *offset, uint16 *enablebits);

/* get swdiv_en nvram value for switch ctrl type */
extern wlc_swdiv_swctrl_t wlc_swdiv_swctrl_en_get(wlc_swdiv_info_t *swdiv);

/* clean-up when non-scb specific reset is requested */
void wlc_swdiv_scbcubby_stat_reset(wlc_swdiv_info_t *swdiv);

/* for wlapi_xxx wrapper api */
extern uint8 wlc_swdiv_plcybased_ant_get(wlc_swdiv_info_t *swdiv, int coreidx);
extern int32 wlc_swdiv_ant_plcy_override(wlc_swdiv_info_t *swdiv, uint core,
	uint8 rxovr_val, uint8 txovr_val, uint8 cellonovr_val, uint8 celloffovr_val);

/* check swdiv-able bsscfg type */
extern bool wlc_swdiv_bss_is_supported(wlc_bsscfg_t *cfg);

#endif /* _wlc_swdiv_h_ */
