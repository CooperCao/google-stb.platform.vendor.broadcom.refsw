/*
 * Linux-specific portion of
 * Broadcom 802.11abg Networking Device Driver - vendor requirement modifications
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
#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>

#if !defined(LINUX_HYBRID)
#include <wlc_cfg.h>
#endif

#include <wlc_pub.h>
#include <wlc.h>
#include <wl_dbg.h>
#include <wlioctl.h>
#include <wlc_iocv.h>
#include <wl_linux.h>
#include <wl_linux_vendor.h>

s32
wl_vendor_attach(struct net_device *ndev, void *context, void *wlinfo)
{
	return 0;
}

s32
wl_vendor_detach(void *wlinfo, void *context)
{
	return 0;
}

s32
wl_vendor_open(struct net_device *ndev, void *context, void *wlinfo)
{
	return 0;
}

s32
wl_vendor_close(struct net_device *ndev, void *context, void *wlinfo)
{
	return 0;
}


