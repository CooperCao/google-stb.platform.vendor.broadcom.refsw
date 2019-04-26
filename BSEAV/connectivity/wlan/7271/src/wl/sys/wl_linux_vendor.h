/*
 * wl_linux_vendor.c exported functions and definitions
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

#ifndef _wl_linux_vendor_h_
#define _wl_linux_vendor_h_

extern s32 wl_vendor_attach(struct net_device *ndev, void *context, void *wlinfo);
extern s32 wl_vendor_detach(void *wlinfo, void *context);
extern s32 wl_vendor_open(struct net_device *ndev, void *context, void *wlinfo);
extern s32 wl_vendor_close(struct net_device *ndev, void *context, void *wlinfo);

#endif /* _wl_linux_vendor_h_*/
