/*
 * ICMP - ICMP Offload
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
 * $Id:$
 */

#ifndef _wl_icmp_h_
#define _wl_icmp_h_

/* This marks the start of a packed structure section. */
#define IP_DEFAULT_TTL     32
#define MAX_HOST_IPV4       1
#define MAX_HOST_IPV6       4

typedef struct wl_icmp_info wl_icmp_info_t;

typedef struct wl_icmp_ip_cfg {
	struct ipv4_addr ipv4[MAX_HOST_IPV4];
	struct ipv6_addr ipv6[MAX_HOST_IPV6];
	int32 num_ipv4;
	int32 num_ipv6;
} wl_icmp_ip_cfg_t;

extern wl_icmp_info_t *wl_icmp_attach(wlc_info_t *wlc);
extern void wl_icmp_detach(wl_icmp_info_t *icmpi);
extern bool wl_icmp_rx(wl_icmp_info_t *icmp_info, void *sdu);
extern bool wl_icmp_is_running(wl_icmp_info_t *icmpi);
extern void wlc_set_icmp_offload(wlc_info_t *wlc, wl_icmp_ip_cfg_t *host_ip);
extern void wlc_clear_icmp_offload(wlc_info_t *wlc);
#endif	/* _wl_icmp_h_ */
