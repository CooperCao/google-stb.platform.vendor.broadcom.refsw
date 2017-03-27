/*
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
 *
*/

#ifndef _epivers_h_
#define _epivers_h_

#define	EPI_MAJOR_VERSION	15

#define	EPI_MINOR_VERSION	10

#define	EPI_RC_NUMBER		66

#define	EPI_INCREMENTAL_NUMBER	0

#define EPI_BUILD_NUMBER	1

#define	EPI_VERSION		15, 10, 66, 0

#define	EPI_VERSION_NUM		0x0f0a4200

#define EPI_VERSION_DEV		"15.10.66"

/* Driver Version String, ASCII, 32 chars max */
#ifdef WLTEST
/*#define	EPI_VERSION_STR		" (84643555ff2f2f40fa0377f16f9f49c6d3e829bb WLTEST)"*/
#define	EPI_VERSION_STR		"84643555ff2f2f40fa0377f16f9f49c6d3e829bb WLTEST"
#define EPI_WL_VER_STR          ":STB7271_REL_15_10_66 WLTEST"
#define	EPI_VERSION_DATE	""
#define	EPI_VERSION_TAG		"STB7271_REL_15_10_66"
#define	EPI_REMOTE_BRANCH	""
#else
/*#define	EPI_VERSION_STR		" (84643555ff2f2f40fa0377f16f9f49c6d3e829bb)"*/
#define	EPI_VERSION_STR		"84643555ff2f2f40fa0377f16f9f49c6d3e829bb"
#define EPI_WL_VER_STR          ":STB7271_REL_15_10_66"
#define	EPI_VERSION_DATE	""
#define	EPI_VERSION_TAG		"STB7271_REL_15_10_66"
#define	EPI_REMOTE_BRANCH	""
#endif

#endif /* _epivers_h_ */
