/*
 * Fundamental constants relating to L2TP Protocol
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
 * $Id: bcml2tp.h, Exp $
 */

#ifndef _bcml2tp_h_
#define _bcml2tp_h_

#ifndef _TYPEDEFS_H_
#include <typedefs.h>
#endif

#define PPP_PROTO(data)	(((data)[0] << 8) + (data)[1])

/* This marks the start of a packed structure section. */
#ifndef _bcmip_h_
#include <packed_section_start.h>
#endif

#define BCML2TP_PORT  1701
#define BCML2TP_CTBIT(ver) ((ver) & 0x8000)	/* Determines if control or not */
#define L2TP_CLBIT(ver) ((ver) & 0x4000)	/* Length bit present */
						/* Must be 1 for control messages */


#define BCML2TP_CZBITS(ver) ((ver) &0x37F8)  /* Reserved bits: drop anything with these there */

#define BCML2TP_CFBIT(ver) ((ver) & 0x0800)  /* Presence of Ns and Nr fields flow bit */

#define BCML2TP_OFBIT(ver) ((ver) & 0x0200)  /* Presence of Offset size fields */

#define BCML2TP_CVER(ver) ((ver) & 0x0007)        /* Version of encapsulation */

#define BCML2TP_PSBIT(ver) (ver & 0x0200)       /* Offset size bit */

#define BCML2TP_PLBIT(ver) L2TP_CLBIT(ver)   /* Length specified? */

#define BCML2TP_PFBIT(ver) BCML2TP_CFBIT(ver)   /* Flow control specified? */


/* L2TP control header */
/* These fields are stored in network order */
BWL_PRE_PACKED_STRUCT struct bcml2tp_control_hdr
{
	uint16 ver;                   /* Version and more */
	uint16 length;                /* Length field */
	uint16 tid;                   /* Tunnel ID */
	uint16 cid;                   /* Call ID */
	uint16 Ns;                    /* Next sent */
	uint16 Nr;                    /* Next received */
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct bcml2tp_payload_hdr
{
	uint16 ver;                   /* Version and friends */
	uint16 length;                /* Optional Length */
	uint16 tid;                   /* Tunnel ID */
	uint16 cid;                   /* Caller ID */
	uint16 Ns;                    /* Optional next sent */
	uint16 Nr;                    /* Optional next received */
	uint16 o_size;                /* Optional offset size */
	uint16 o_pad;                 /* Optional offset padding */
} BWL_POST_PACKED_STRUCT;


/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif	/* _bcml2tp_h_ */
