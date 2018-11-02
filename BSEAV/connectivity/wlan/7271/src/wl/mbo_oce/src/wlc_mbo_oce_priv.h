/*
 * MBO-OCE Private declarations/definitions for
 * Broadcom 802.11abgn Networking Device Driver
 *
 * $Copyright (C) 2016 Broadcom Corporation$
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 *
 */

/**
 */

#ifndef _wlc_mbo_oce_priv_h_
#define _wlc_mbo_oce_priv_h_

typedef void *wlc_mbo_oce_ie_build_hndl_t;
typedef void *wlc_mbo_oce_ie_parse_hndl_t;

/*
 * - 'cbparm' points to the user supplied calc_len/build parameters
 *   structure from the wlc_iem_calc_len()/wlc_iem_build_frame() caller.
 * - 'cfg' is the pointer to bsscfg for which the call is issued.
 * - 'ft' is the frame type FC_XXXX as defined in 802.11.h (see also
 *   (WLC_IEM_FC_SCAN_XXXX in wlc_iem_mgmt.h)
 * - 'tag' is that the callback was registered for.
 * - 'buf' = points to the buffer where callback writes the attributes.
 *    buf = NULL; if called for  getting attribute length.
 * - 'buf_len' = Length of the buffer.
 *    buf_len = 0; if called for getting attribute length.
 */
typedef struct {
	wlc_iem_cbparm_t *cbparm;  /* Callback parameters */
	wlc_bsscfg_t *cfg;
	uint16 ft;  /* Frame type */
	uint8 tag;  /* IE tag */
	uint8 *buf; /* IE buffer pointer to put attributes */
	uint buf_len; /* buffer length */
} wlc_mbo_oce_attr_build_data_t;

/* This same callback will be called during allocating buffer
 * to know the length of attributes it is going to write,
 * as well as for actual writing of attributes into the buffer.
 * during length calculation: 'buf' = NULL and 'buf_len' = 0.
 * during writing attribute into IE: 'buf' = pointer to buffer and
 * 'buf_len' = available buffer length.
 */
typedef int
(*wlc_mbo_oce_attr_build_fn_t)(void *ctx, wlc_mbo_oce_attr_build_data_t *data);

/*
 * - 'pparm' points to the parse callback parameters structure from the
 *   wlc_iem_parse_frame() caller.
 * - 'cfg' is the pointer to bsscfg for which the call is issued.
 * - 'ft' is the frame type FC_XXXX as defined in 802.11.h (see also
 *   (WLC_IEM_FC_SCAN_XXXX in wlc_iem_mgmt.h)
 * - 'ie' = points to the begining of MBO+OCE attributes
 * - 'ie_len' = Length of only MBO+OCE attributes without MBO_OCE header.
 */
typedef struct {
	wlc_iem_pparm_t *pparm; /* Callback parameters */
	wlc_bsscfg_t *cfg;
	uint16 ft;  /* Frame type */
	uint8 *ie; /* IE pointer */
	uint ie_len; /* buffer length */
} wlc_mbo_oce_attr_parse_data_t;

typedef int
(*wlc_mbo_oce_attr_parse_fn_t)(void *ctx, wlc_mbo_oce_attr_parse_data_t *data);

typedef struct wlc_mbo_oce_ie_build_data {
	void *ctx;
	uint16 fstbmp;
	wlc_mbo_oce_attr_build_fn_t build_fn;
} wlc_mbo_oce_ie_build_data_t;

typedef struct wlc_mbo_oce_ie_parse_data {
	void *ctx;
	uint16 fstbmp;
	wlc_mbo_oce_attr_parse_fn_t parse_fn;
} wlc_mbo_oce_ie_parse_data_t;

wlc_mbo_oce_ie_build_hndl_t
wlc_mbo_oce_register_ie_build_cb(wlc_mbo_oce_info_t *mbo_oce,
	wlc_mbo_oce_ie_build_data_t* build_data);
wlc_mbo_oce_ie_parse_hndl_t
wlc_mbo_oce_register_ie_parse_cb(wlc_mbo_oce_info_t *mbo_oce,
	wlc_mbo_oce_ie_parse_data_t *parse_data);
int
wlc_mbo_oce_unregister_ie_build_cb(wlc_mbo_oce_info_t *mbo_oce,
	wlc_mbo_oce_ie_build_hndl_t hndl);
int
wlc_mbo_oce_unregister_ie_parse_cb(wlc_mbo_oce_info_t *mbo_oce,
	wlc_mbo_oce_ie_parse_hndl_t hndl);

#endif	/* _wlc_mbo_oce_priv_h_ */
