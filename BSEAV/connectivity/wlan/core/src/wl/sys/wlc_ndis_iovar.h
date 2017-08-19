/*
 * WLC NDIS module interface
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

#ifndef _wlc_ndis_h_
#define _wlc_ndis_h_

#include <wlc_dump.h>

extern wlc_ndis_info_t *wlc_ndis_attach(wlc_info_t *wlc);
extern void wlc_ndis_detach(wlc_ndis_info_t *ndis);

bool wlc_ndis_ar_disassoc_st(wlc_ndis_info_t *ndis, wlc_bsscfg_t *cfg);
void wlc_ndis_disassoc_complete(wlc_ndis_info_t *ndis, wlc_bsscfg_t *cfg);

extern si_t *wlc_get_sih(wlc_info_t *wlc);
extern bool wlc_cfg_associated(wlc_info_t *wlc);
extern void wlc_set_hw_up(wlc_info_t *wlc, bool val);
extern void wlc_set_delayed_down(wlc_info_t *wlc, bool val);
extern bool wlc_get_delayed_down(wlc_info_t *wlc);
extern int wlc_dump_register_wrapper(wlc_info_t *wlc, const char *name,
	dump_fn_t dump_fn, void *dump_fn_arg);

/* network packet filter bit map matching NDIS_PACKET_TYPE_XXX */
#define WLC_ACCEPT_DIRECTED               0x0001
#define WLC_ACCEPT_MULTICAST              0x0002
#define WLC_ACCEPT_ALL_MULTICAST          0x0004
#define WLC_ACCEPT_BROADCAST              0x0008
#define WLC_ACCEPT_SOURCE_ROUTING         0x0010
#define WLC_ACCEPT_PROMISCUOUS            0x0020
#define WLC_ACCEPT_SMT                    0x0040
#define WLC_ACCEPT_ALL_LOCAL              0x0080

/* iovar interface */
typedef struct {
	uint16 exempt_type;	/**< Ethertype allowed to be received unencrypted */
	uint16 exempt_action;	/**< 1 - always (802.1x), 2 - key unavailable (WPA) */
	uint16 exempt_pkttype;	/**< 1 - ucast, 2 - mcast, 3 - both */
} exempt_t;

typedef struct {
	uint entries;		/**< total exempt entries */
	exempt_t exempt[1];	/**< exempt list */
} exempt_list_t;

typedef enum {
	WLC_EXEMPT_ALWAYS,
	WLC_EXEMPT_NO_KEY,
	WLC_EXEMPT_ACTION_MAX
} exempt_action_t;

typedef enum {
	WLC_EXEMPT_UNICAST,
	WLC_EXEMPT_MULTICAST,
	WLC_EXEMPT_BOTH,
	WLC_EXEMPT_PKTTYPE_MAX
} exempt_pkttype_t;

#define WLC_EXEMPT_LIST_LEN(entries) (OFFSETOF(exempt_list_t, exempt) + \
	(sizeof(exempt_t) * entries))

/* function interface */
bool wlc_exempt_pkt(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb,
	struct wlc_frminfo *f, uint16 ether_type, uint8 prio);
bool wlc_packet_filter(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct wlc_frminfo *f);

#endif	/* _wlc_ndis_h_ */
