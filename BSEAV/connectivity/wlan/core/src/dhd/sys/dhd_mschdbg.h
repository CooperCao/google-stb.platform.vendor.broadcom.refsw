/*
 * DHD debugability header file
 *
 * <<Broadcom-WL-IPTag/Open:>>
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
 * $Id: dhd_mschdbg.h 571265 2015-07-14 20:50:18Z $
 */

#ifndef _dhd_mschdbg_h_
#define _dhd_mschdbg_h_

#ifdef SHOW_LOGTRACE
extern void wl_mschdbg_event_handler(dhd_pub_t *dhdp, void *raw_event_ptr, int type,
	void *data, int len);
extern void wl_mschdbg_verboselog_handler(dhd_pub_t *dhdp, void *raw_event_ptr, int tag,
	uint32 *log_ptr);
#endif /* SHOW_LOGTRACE */

#endif /* _dhd_mschdbg_h_ */
