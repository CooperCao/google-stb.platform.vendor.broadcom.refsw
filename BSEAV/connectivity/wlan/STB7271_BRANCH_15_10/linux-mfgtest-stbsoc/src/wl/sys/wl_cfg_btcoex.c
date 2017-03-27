/*
 * Linux cfg80211 driver - Dongle Host Driver (DHD) related
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

#if !defined(BCMDONGLEHOST)
#include <osl.h>
#endif /* !BCMDONGLEHOST */
#include <net/rtnetlink.h>

#include <bcmutils.h>
#include <wldev_common.h>
#include <wl_cfg80211.h>
#if defined(BCMDONGLEHOST)
#include <dhd_cfg80211.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhdioctl.h>
#endif /* BCMDONGLEHOST */
#include <wlioctl.h>

#ifdef PKT_FILTER_SUPPORT
extern uint dhd_pkt_filter_enable;
extern uint dhd_master_mode;
extern void dhd_pktfilter_offload_enable(dhd_pub_t * dhd, char *arg, int enable, int master_mode);
#endif

struct btcoex_info {
	struct timer_list timer;
	u32 timer_ms;
	u32 timer_on;
	u32 ts_dhcp_start;	/* ms ts ecord time stats */
	u32 ts_dhcp_ok;		/* ms ts ecord time stats */
	bool dhcp_done;	/* flag, indicates that host done with
					 * dhcp before t1/t2 expiration
					 */
	s32 bt_state;
	struct work_struct work;
	struct net_device *dev;
};
