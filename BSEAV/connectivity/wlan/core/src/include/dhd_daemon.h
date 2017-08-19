/*
 * Header file for DHD daemon to handle timeouts
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

#ifndef __BCM_DHDD_H__
#define __BCM_DHDD_H__

#define BCM_TO_MAGIC 0x600DB055
#define NO_TRAP 0
#define DO_TRAP	1

#define BCM_NL_USER 31

#define REASON_COMMAND_TO 1
#define REASON_OQS_TO 2
#define REASON_SCAN_TO 3
#define REASON_JOIN_TO 4
#define REASON_DAEMON_STARTED 5

typedef struct bcm_to_info {
	int magic;
	int reason;
	int trap;
} bcm_to_info_t;

#endif /* __BCM_DHDD_H__ */
