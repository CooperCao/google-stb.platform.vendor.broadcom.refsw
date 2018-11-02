/*
 * MBO and OCE WFA specifications shared structures and definitions
 * as defined in WFA and MBO  OCE specification
 *
 * $ Copyright Open Broadcom Corporation $
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

#ifndef _MBO_OCE_H_
#define _MBO_OCE_H_

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/* WiFi MBO_OCE OUI values */
#define MBO_OCE_OUI		WFA_OUI		/* WiFi OUI 50:6F:9A */
/* oui_type field identifying the type and version of the OCE IE. */
#define MBO_OCE_OUI_TYPE	WFA_OUI_TYPE_MBO_OCE /* OUI Type/Version */
/* IEEE 802.11 vendor specific information element. */
#define MBO_OCE_IE_ID		0xdd

/*  4.1 MBO-OCE IE (WFA OCE spec) */
typedef BWL_PRE_PACKED_STRUCT struct wifi_mbo_oce_ie_s {
	uint8 id;	/* IE ID: MBO_OCE_IE_ID 0xDD */
	uint8 len;	/* IE length */
	uint8 oui[WFA_OUI_LEN]; /* MBO_OCE_OUI 50:6F:9A */
	uint8 oui_type;	/* MBO_OCE_OUI_TYPE 0x16 */
	uint8 attr[];	/* var len attributes */
} BWL_POST_PACKED_STRUCT wifi_mbo_oce_ie_t;

#define MBO_OCE_IE_HDR_SIZE (OFFSETOF(wifi_mbo_oce_ie_t, attr))
/* oui:3 bytes + oui type:1 byte */
#define MBO_OCE_IE_NO_ATTR_LEN  4

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif /* __MBO_OCE_H__ */
