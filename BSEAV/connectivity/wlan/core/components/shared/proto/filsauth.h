/*
 * Fundamental types and constants relating to FILS AUTHENTICATION
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

#ifndef _FILSAUTH_H_
#define _FILSAUTH_H_

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/* 11ai D6.0 8.6.8.36 FILS Discovery frame format
	category
	action
	fils_discovery_info_field_t
	fils_rnr_element_t
	fils_indication_element_t
	fils_vendor_specific_element_t
*/

/* 11revmc D4.0 8.4.2.25 Vendor Specific element */
typedef BWL_PRE_PACKED_STRUCT struct fils_vendor_specific_element {
	uint8		elementid;
	uint8		length;
	/* variable len info */
	uint8		orgid_vendorspecific_content[];
} BWL_POST_PACKED_STRUCT fils_vendor_specific_element_t;

/* 11ai D6.0 8.4.2.178 FILS Indication element */
typedef BWL_PRE_PACKED_STRUCT struct fils_indication_element {
	uint8		elementid;
	uint8		length;
	uint16		fils_info;
	/* variable len info */
	uint8		cache_domain_publickey_id[];
} BWL_POST_PACKED_STRUCT fils_indication_element_t;


/* 11ai D6.0 8.4.2.169 Reduced Neighbor Report element */
typedef BWL_PRE_PACKED_STRUCT struct fils_rnr_element {
	uint8		elementid;
	uint8		length;
	/* variable len info */
	uint8		neighbor_ap_info[];
} BWL_POST_PACKED_STRUCT fils_rnr_element_t;


/* 8.4.2.175 FILS Session element */
typedef BWL_PRE_PACKED_STRUCT struct fils_session_element {
	uint8		elementid;
	uint8		length;
	uint8		element_id_ext;
	uint32		fils_session[2];
} BWL_POST_PACKED_STRUCT fils_session_element_t;


/* 8.4.2.174 FILS Key Confirmation element */
typedef BWL_PRE_PACKED_STRUCT struct fils_key_confirm_element {
	uint8		elementid;
	uint8		length;
	uint8		element_id_ext;
	/* variable len info */
	uint8		keyauth[];
} BWL_POST_PACKED_STRUCT fils_key_confirm_element_t;


/* 11ai D6.0 8.6.8.36 FILS Discovery frame format */
typedef BWL_PRE_PACKED_STRUCT struct fils_discovery_info_field {
	uint16		framecontrol;
	uint32		timestamp[2];
	uint16		bcninterval;
	/* variable len info */
	uint8		disc_info[];
} BWL_POST_PACKED_STRUCT fils_discovery_info_field_t;


/* 11ai D6.0 8.4.2.173 FILS Request Parameters element */
typedef BWL_PRE_PACKED_STRUCT struct fils_request_parameters_element {
	uint8		elementid;
	uint8		lengh;
	uint8		element_id_ext;
	uint8		params_bitmap;
	/* variable len info */
	uint8		params_fields[];
} BWL_POST_PACKED_STRUCT fils_request_parameters_element_t;

#define FILS_REQUEST_PARAMETERS_ID		255
#define FILS_REQUEST_PARAMETERS_EXTID	2

#define FILS_PARAM_MAX_CHANNEL_TIME		(1 << 5)

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif /* __FILSAUTH_H__ */
