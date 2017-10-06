/*
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
 * $Id$
 */


#ifndef _wl_rte_h_
#define _wl_rte_h_

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <epivers.h>
#include <bcmendian.h>
#include <proto/ethernet.h>
#include <bcmdevs.h>
#include <wlioctl.h>
#include <proto/802.11.h>
#include <d11.h>
#include <wlc_key.h>
#include <wlc_rate.h>
#include <wlc.h>

#ifdef WLPFN
#include <wl_pfn.h>
#endif	/* WLPFN */

#include <wl_toe.h>
#include <wl_keep_alive.h>
#include <wl_gas.h>
#include <wlc_pkt_filter.h>
#include <wl_eventq.h>
#include <wl_p2po_disc.h>
#include <wl_p2po.h>
#include <wl_arpoe.h>
#include <wl_anqpo.h>

#if defined(D0_COALESCING)
#include <wl_d0_filter.h>
#endif /* D0_COALESCING */

#ifdef WLNDOE
#include <wl_ndoe.h>
#endif
#include <wl_bdo.h>
#include <wl_tko.h>
#include <wl_icmp.h>

#include <wl_nfc.h>
#include <rte_dev.h>
#include <rte_timer.h>


#include <wl_avs.h>

typedef hnd_timer_t wl_timer;

struct wl_if {
	struct wlc_if *wlcif;
	hnd_dev_t *dev;		/**< virtual device */
	wl_arp_info_t   *arpi;      /**< pointer to arp agent offload info */
#ifdef WLNDOE
	wl_nd_info_t	*ndi;
#endif
	wl_nfc_info_t	*nfci;
};

typedef struct wl_info {
	uint		unit;		/**< device instance number */
	wlc_pub_t	*pub;		/**< pointer to public wlc state */
	void		*wlc;		/**< pointer to private common os-independent data */
	wlc_hw_info_t	*wlc_hw;
	hnd_dev_t	*dev;		/**< primary device */
	bool		link;		/**< link state */
	uint8		hwfflags;	/**< host wake up filter flags */
	hnd_dev_stats_t	stats;
	hnd_timer_t  *dpcTimer;	/**< 0 delay timer used to schedule dpc */
	bool		dpctimer_armed;
#ifdef WLPFN
	wl_pfn_info_t	*pfn;		/**< pointer to prefered network data */
#endif /* WLPFN */
	wl_toe_info_t	*toei;		/**< pointer to toe specific information */
	wl_arp_info_t	*arpi;		/**< pointer to arp agent offload info */
	wl_keep_alive_info_t	*keep_alive_info;	/**< pointer to keep-alive offload info */
	wlc_pkt_filter_info_t	*pkt_filter_info;	/**< pointer to packet filter info */
#ifdef WLNDOE
	wl_nd_info_t	*ndi;	/**< Neighbor Advertisement Offload for IPv6 */
#endif
	wl_p2po_info_t	*p2po;		/**< pointer to p2p offload info */
	wl_anqpo_info_t *anqpo;	/**< pointer to anqp offload info */

	wl_disc_info_t	*disc;		/**< pointer to disc info */
	wl_eventq_info_t	*wlevtq;	/**< pointer to wl_eventq info */
	wl_gas_info_t	*gas;		/**< pointer to gas info */
	wlc_obj_registry_t	*objr;		/**< Common shared object registry for RSDB */
	wl_nfc_info_t	*nfci;	/**< NFC for Secure WiFi */
#if defined(D0_COALESCING)
	wlc_d0_filter_info_t   *d0_filter_info;   /**< pointer to ip filter info */
#endif /* D0_COALESCING */

	wl_bdo_info_t *bdo;	/* pointer to bonjour dongle offload info */
#ifdef WL_MONITOR
	uint32		monitor_type;	/* monitor (MPDU sniffing) mode */
#endif /* WL_MONITOR */
	wl_tko_info_t *tko;		/* pointer to tcp keep-alive info */
#ifdef HEALTH_CHECK
	health_check_info_t     *hc;    /* Descriptor: unique and mutually exclusive */
#endif /* HEALTH_CHECK */
	wl_shub_info_t *shub_info; /* Sensor Hub Interworking instance */
	struct wl_natoe_info *natoe_info; /* Pointer to natoe structure */

	wl_avs_t		*avs;	/* Adaptive Voltage Scaling */
	wl_icmp_info_t *icmp;		/* pointer to icmp info */
} wl_info_t;

/** Externally used RTE function */
void do_wl_cmd(uint32 arg, uint argc, char *argv[]);

int wl_send_fwdpkt(struct wl_info *wl, void *p, struct wl_if *wlif);
int wl_rx_pktfetch(struct wl_info *wl, struct wl_if *wlif, struct lbuf *lb);
#endif /* _wl_rte_h_ */
