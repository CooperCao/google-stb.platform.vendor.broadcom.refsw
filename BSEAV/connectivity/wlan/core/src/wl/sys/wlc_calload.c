/*
 * Channel calibration information download.
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: $
 */


#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <epivers.h>
#include <bcmdevs.h>
#include <d11.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_iocv.h>
#include <wlc_bsscfg.h>
#include <wlc_phy_hal.h>
#ifdef WLRSDB
#include <wlc_rsdb.h>
#endif
#include <phy_api.h>

#define WL_BLOB_DEBUG_ERROR(arg) WL_ERROR(arg)
#ifdef WLTEST
#define WL_BLOB_DEBUG_INFO(arg)  WL_ERROR(arg)
#else
#define WL_BLOB_DEBUG_INFO(arg)
#endif /* WLTEST */

#ifdef WLC_TXCAL
static dload_error_status_t wlc_handle_cal_dload(struct wlc_info *wlc,
	wlc_blob_segment_t *segments, uint32 segment_count);
#ifdef WLTEST
static int wlc_handle_cal_dump(struct wlc_info *wlc, void *buf, int len);
#endif

/* TxCAL revision */
#define TXCAL_VER_0	0	/**< REV0: Initial revision */
#define TXCAL_VER_1	1	/**< REV1: Reserved */
#define TXCAL_VER_2	2	/**< REV2: Add Av/Vmid support */
#define TXCAL_VER_3	3	/**< REV3: Support different steps per band/core */
#define TXCAL_VER_4	4	/**< REV4: Support multiple slice (aka MAC) and 6-band config */
#define TXCAL_VER_MAX	4

/* RxCAL revision */
#define RXCAL_VER_0	0	/**< REV0: Initial revision */
#define RXCAL_VER_1	1	/**< REV1: Dynamic number of gains */
#define RXCAL_VER_MAX	1

#define NUM_BANDS_SINGLE	1	/**< Single band configuration */
#define NUM_BANDS_2G		2	/**< 20MHz or 40MHz */
#define NUM_BANDS_5G		3	/**< 20MHz, 40MHz and 80MHz */
#define NUM_GAINS_MAX		4
#define NUM_GAINS_MAX_LCN20	3

/* TXCAL specific misc. structures for calibration storage format */
#define MAX_TXCAL_PWRTBL_ENTRY	32
#define WLC_TXCAL_CH_TO_BAND(chan)	\
	((chan <= 14) ? 0 : ((chan <= 48) ? 1 : ((chan <= 64) ? 2 : ((chan <= 144) ? 3 : 4))))
#define WLC_RXCAL_2G_GROUP_SZ	8

static const char *phy_rssi_gain_delta_2g[] = {
	"phy_rssi_gain_delta_2gb0",
	"phy_rssi_gain_delta_2gb1",
	"phy_rssi_gain_delta_2gb2",
	"phy_rssi_gain_delta_2gb3",
	"phy_rssi_gain_delta_2gb4",
};
#if (LCN20CONF == 0)
static const char *phy_rssi_gain_delta_5g[] = {
	"phy_rssi_gain_delta_5gl",
	"phy_rssi_gain_delta_5gml",
	"phy_rssi_gain_delta_5gmu",
	"phy_rssi_gain_delta_5gh",
};
#endif /* LCN20CONF == 0 */

/* MSF segment type definitions for CAL download only */
enum {
	DLOAD_CAL_SEGTYPE_NONE		= 0,
	DLOAD_CAL_SEGTYPE_VERSION	= 1,
	DLOAD_CAL_SEGTYPE_CHIPID	= 2,
	DLOAD_CAL_SEGTYPE_TXCAL		= 3,
	DLOAD_CAL_SEGTYPE_RXCAL		= 4,
	DLOAD_CAL_SEGTYPE_LAST
};

struct wlc_cal_chan_range {
	int ch_first;
	int ch_last;
	int ch_inc;
};
typedef struct wlc_cal_chan_range wlc_cal_chan_range_t;

#include<packed_section_start.h>
struct BWL_PRE_PACKED_STRUCT wl_txcal_storage_hdr_t {
	uint16	len;
	uint16	ver;
	uint16	dev_type;
	uint16	cal_type;
	uint8	temp;
	uint8	num_core;
	uint8	num_band;	/* For Av/Vmid */
	uint8	slice;		/* multi-slices[MAC] */
} BWL_POST_PACKED_STRUCT;

struct BWL_PRE_PACKED_STRUCT wl_txcal_storage_ch_hdr_t {
	uint16	len;
	uint8	num_sts[NUM_SUBBANDS_FOR_AVVMID][PHY_CORE_MAX];
	uint8	num_channels;
} BWL_POST_PACKED_STRUCT;

struct BWL_PRE_PACKED_STRUCT wl_txcal_storage_ch_pwr_t {
	uint8	ch_num;
	uint8	Ptssi;
} BWL_POST_PACKED_STRUCT;

struct BWL_PRE_PACKED_STRUCT wl_rxcal_storage_hdr_t {
	uint16  len;
	uint16  ver;
	uint16  dev_type;
	uint16  cal_type;
	uint8   temp;
	uint8   num_core;
	uint8   slice;		/* For multi-slices[MAC] */
	uint8   reserved;
} BWL_POST_PACKED_STRUCT;

struct BWL_PRE_PACKED_STRUCT wl_rxcal_storage_band_hdr_t {
	uint16  len;
	uint8   num_group;
	uint8	num_gain;
} BWL_POST_PACKED_STRUCT;
#include<packed_section_end.h>

struct wlc_calload_info {
	wlc_pub_t	*pub;
	wlc_info_t	*wlc;
	uint8		calload_chkver[ETHER_ADDR_LEN];
	wlc_blob_info_t *calload_wbi;
	dload_error_status_t blobload_status;		/* detailed blob load status */
};

/* iovar table */
enum {
	IOV_CALLOAD				= 1,
	IOV_CALLOAD_STATUS			= 2,
	IOV_CALDUMP				= 3,
	IOV_CALLOAD_CHKVER			= 4,
	IOV_LAST
};

static const bcm_iovar_t calload_iovars[] = {
	{"calload", IOV_CALLOAD, IOVF_SET_DOWN, 0, IOVT_BUFFER, 0},
	{"calload_status", IOV_CALLOAD_STATUS, 0, 0, IOVT_UINT32, 0},
	{"caldump", IOV_CALDUMP, 0, 0, IOVT_BUFFER, 0},
	{"calload_chkver", IOV_CALLOAD_CHKVER, 0, 0, IOVT_BUFFER, 0},
	{NULL, 0, 0, 0, 0, 0}
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

static int
wlc_calload_doiovar(void *hdl, uint32 actionid,
	void *params, uint p_len, void *arg, uint len, uint val_size, struct wlc_if *wlcif)
{
	wlc_calload_info_t *wlc_cldi = (wlc_calload_info_t *)hdl;
	int err = BCME_OK;
	int32 int_val = 0;
	int32 *ret_int_ptr;

	BCM_REFERENCE(len);
	BCM_REFERENCE(wlcif);
	BCM_REFERENCE(val_size);
	BCM_REFERENCE(ret_int_ptr);

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	switch (actionid) {
	case IOV_SVAL(IOV_CALLOAD): {
		wl_dload_data_t *dload_data;

		/* Make sure we have at least a dload data structure */
		if (p_len < sizeof(wl_dload_data_t)) {
			err =  BCME_ERROR;
			wlc_cldi->blobload_status = DLOAD_STATUS_IOVAR_ERROR;
			break;
		}

		/* cast to a local structure pointer */
		dload_data = (wl_dload_data_t *)params;

		WL_NONE(("%s: IOV_CALLOAD flag %04x, type %04x, len %d, crc %08x\n",
			__FUNCTION__, dload_data->flag, dload_data->dload_type,
			dload_data->len, dload_data->crc));

		if (((dload_data->flag & DLOAD_FLAG_VER_MASK) >> DLOAD_FLAG_VER_SHIFT)
			!= DLOAD_HANDLER_VER) {
			err =  BCME_ERROR;
			wlc_cldi->blobload_status = DLOAD_STATUS_IOVAR_ERROR;
			break;
		}

		wlc_cldi->blobload_status = wlc_blob_download(wlc_cldi->calload_wbi,
			dload_data->flag, dload_data->data, dload_data->len, wlc_handle_cal_dload);
		switch (wlc_cldi->blobload_status) {
			case DLOAD_STATUS_DOWNLOAD_SUCCESS:
			case DLOAD_STATUS_DOWNLOAD_IN_PROGRESS:
				err = BCME_OK;
				break;
			default:
				err = BCME_ERROR;
				break;
		}
		break;
	}
	case IOV_GVAL(IOV_CALLOAD_STATUS): {
		*((uint32 *)arg) = wlc_cldi->blobload_status;
		break;
	}
#ifdef WLTEST
	case IOV_GVAL(IOV_CALDUMP): {
		err = wlc_handle_cal_dump(wlc_cldi->wlc, arg, len);
		break;
	}
#endif /* WLTEST */
	case IOV_SVAL(IOV_CALLOAD_CHKVER): {
		uint chk_len = MIN(p_len, ETHER_ADDR_LEN);

		/* Only the first 6 bytes are checked for now */
		if (chk_len < ETHER_ADDR_LEN) {
			err = BCME_BADARG;
		} else if (memcmp(wlc_cldi->calload_chkver, params, chk_len) != 0) {
			err = BCME_ERROR;
		}
		break;
	}
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

wlc_calload_info_t *
BCMATTACHFN(wlc_calload_attach)(wlc_info_t *wlc)
{
	wlc_calload_info_t *wlc_cldi;
	wlc_pub_t *pub = wlc->pub;

	if ((wlc_cldi = (wlc_calload_info_t *)MALLOCZ(pub->osh, sizeof(wlc_calload_info_t)))
		== NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes", pub->unit,
			__FUNCTION__, MALLOCED(pub->osh)));
		return NULL;
	}

	wlc_cldi->pub = pub;
	wlc_cldi->wlc = wlc;
	wlc_cldi->blobload_status = DLOAD_STATUS_LAST;

	if ((wlc_cldi->calload_wbi = wlc_blob_attach(wlc)) == NULL) {
		MFREE(pub->osh, wlc_cldi, sizeof(wlc_calload_info_t));
		return NULL;
	}

	/* register module */
	if (wlc_module_register(pub, calload_iovars, "calld", wlc_cldi, wlc_calload_doiovar,
	    NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n", pub->unit, __FUNCTION__));
		wlc_blob_detach(wlc_cldi->calload_wbi);
		MFREE(pub->osh, wlc_cldi, sizeof(wlc_calload_info_t));
		return NULL;
	}

	return wlc_cldi;
}

void
BCMATTACHFN(wlc_calload_detach)(wlc_calload_info_t *wlc_cldi)
{
	if (wlc_cldi) {
		wlc_info_t *wlc = wlc_cldi->wlc;
		wlc_pub_t *pub = wlc->pub;

		wlc_module_unregister(pub, "calld", wlc_cldi);

		wlc_blob_detach(wlc_cldi->calload_wbi);
		MFREE(pub->osh, wlc_cldi, sizeof(wlc_calload_info_t));
	}
}

/* A helper function to pass calibration data to PHY using PHY iovars */
static int wlc_calload_phy_iovar_passthrough(wlc_info_t *wlc, const char *name,
	void *params, int p_len, void *arg, int len, bool set, uint8 slice)
{
	int idx;
	wlc_info_t *wlc_iter;
	int err = BCME_ERROR;	/* set default to error in case of no PHY iovar issued */

#if defined(WLRSDB)
	if (WLC_DUALMAC_RSDB(wlc->cmn)) {
		if ((err = wlc_iovar_op(wlc->cmn->wlc[slice], name,
			params, p_len, arg, len, set, NULL)) != BCME_OK)
			WL_BLOB_DEBUG_ERROR(("IOVAR failure (slice=%d err=%d)\n", slice, err));
		return err;
	}
#endif /* WLRSDB */

	FOREACH_WLC(wlc->cmn, idx, wlc_iter) {
		if ((err = wlc_iovar_op(wlc_iter, name,
			params, p_len, arg, len, set, NULL)) != BCME_OK) {
			WL_BLOB_DEBUG_ERROR(("IOVAR failure (core=%d err=%d)\n", idx, err));
			return err;
		}
	}
	return err;
}

static dload_error_status_t
wlc_handle_cal_dload_rxcal_slice(wlc_info_t *wlc, wlc_blob_segment_t *seg_rxcal)
{
	int err;
	dload_error_status_t status = DLOAD_STATUS_DOWNLOAD_SUCCESS;
	uint16 rxcal_len_total, rxcal_ver;
	uint8 *rxcal_dataptr;
	uint32 temp;
	uint8 slice, num_cores, num_groups, num_gains;

	/* Process actual RXCAL data segment */
	if ((seg_rxcal == NULL) || (seg_rxcal->length < sizeof(struct wl_rxcal_storage_hdr_t))) {
		WL_BLOB_DEBUG_ERROR(("RxCal Error: missing segment\n"));
		status = DLOAD_STATUS_CAL_NORXCAL;
		goto error;
	}
	rxcal_dataptr = seg_rxcal->data;
	rxcal_len_total = load16_ua(rxcal_dataptr) + sizeof(rxcal_len_total);

	/* Check that the header is big enough to access fixed fields */
	 if ((rxcal_len_total < sizeof(struct wl_rxcal_storage_hdr_t)) ||
	     (rxcal_len_total > seg_rxcal->length)) {
		WL_BLOB_DEBUG_ERROR(("RxCal Error: segment too short (%d bytes)\n",
			rxcal_len_total));
		status = DLOAD_STATUS_CAL_RXBADLEN;
		goto error;
	}

	/* Check for MAX version supported by PHY */
	rxcal_ver = load16_ua(&rxcal_dataptr[OFFSETOF(struct wl_rxcal_storage_hdr_t, ver)]);
	if (rxcal_ver > RXCAL_VER_MAX) {
		WL_BLOB_DEBUG_ERROR(("RxCal Error: unsupported new version (%d > %d)\n",
			rxcal_ver, RXCAL_VER_MAX));
		status = DLOAD_STATUS_CAL_RXBADVER;
		goto error;
	} else if (rxcal_ver != RXCAL_VER_MAX) {
		WL_PRINT(("RxCal Info: Uses older version %d (current version is %d)\n",
			rxcal_ver, RXCAL_VER_MAX));
	}

	/* Skipping device and calibration types */
	temp = rxcal_dataptr[OFFSETOF(struct wl_rxcal_storage_hdr_t, temp)];
	num_cores = rxcal_dataptr[OFFSETOF(struct wl_rxcal_storage_hdr_t, num_core)];
	slice = rxcal_dataptr[OFFSETOF(struct wl_rxcal_storage_hdr_t, slice)];
	if (slice >= MAX_RSDB_MAC_NUM) {
		/* Check for dual MAC RSDB - multiple slice [aka MAC] is supported from V4 */
		WL_BLOB_DEBUG_ERROR(("RxCal Error: bad slice number (%d)\n", slice));
		status = DLOAD_STATUS_CAL_WRONGMACIDX;
		goto error;
	}
	if (num_cores > PHY_CORE_MAX) {
		WL_BLOB_DEBUG_ERROR(("RxCal Error: wrong number of cores (%d > %d)\n",
			num_cores, PHY_CORE_MAX));
		status = DLOAD_STATUS_CAL_RXBADCORE;
		goto error;
	}
	WL_BLOB_DEBUG_INFO(("RxCal Info: Total %d bytes - %dC - %d cores - MAC[%d]\n",
			rxcal_len_total, temp, num_cores, slice));
	rxcal_dataptr += sizeof(struct wl_rxcal_storage_hdr_t);
	rxcal_len_total -= sizeof(struct wl_rxcal_storage_hdr_t);

	if ((err = wlc_calload_phy_iovar_passthrough(wlc, "phy_rssi_gain_cal_temp",
			NULL, 0, &temp, sizeof(temp), IOV_SET, slice)) != BCME_OK) {
		WL_BLOB_DEBUG_ERROR((
			"RxCal Error: IOVAR phy_rssi_gain_cal_temp (err=%d)\n", err));
		status = DLOAD_STATUS_CAL_TXPHYFAILURE;
		goto error;
	}

	/* Read RXCAL: mandatory 2.4GHz table first */
	if (rxcal_len_total <= sizeof(struct wl_rxcal_storage_band_hdr_t)) {
		WL_BLOB_DEBUG_ERROR(("RxCal Error: missing 2.4GHz table\n"));
		status = DLOAD_STATUS_CAL_RXBADLEN;
		goto error;
	} else {
		uint8 cidx, gidx, grp_2g[2 * WLC_RXCAL_2G_GROUP_SZ];
		uint16 rxcal_len_band, rxcal_len_actual;
		uint8 *rxcal_dataptr_band;

		/* Length check */
		rxcal_dataptr_band = rxcal_dataptr;
		rxcal_len_band = load16_ua(rxcal_dataptr_band) + sizeof(rxcal_len_band);
		num_groups = rxcal_dataptr_band[
				OFFSETOF(struct wl_rxcal_storage_band_hdr_t, num_group)];
		num_gains = NUM_GAINS_MAX;
		if (rxcal_ver > RXCAL_VER_0)
			num_gains = MIN(num_gains, rxcal_dataptr_band[
				OFFSETOF(struct wl_rxcal_storage_band_hdr_t, num_gain)]);
#if (LCN20CONF > 0)
		/* Support single core and single bandwidth (20MHz only) */
		ASSERT(num_cores == 1);
		BCM_REFERENCE(cidx);
		rxcal_len_actual = sizeof(struct wl_rxcal_storage_band_hdr_t) +
			WLC_RXCAL_2G_GROUP_SZ + (NUM_BANDS_SINGLE * num_gains * 1 * num_groups);
#else
		/* Support multi-core and dual bandwidth (20/40MHz) */
		rxcal_len_actual = sizeof(struct wl_rxcal_storage_band_hdr_t) +
			WLC_RXCAL_2G_GROUP_SZ +
			(NUM_BANDS_2G * num_gains * num_cores * num_groups);
#endif /* LCN20CONF */
		if (rxcal_len_band != rxcal_len_actual) {
			WL_BLOB_DEBUG_ERROR(("RxCal Error: 2.4GHz table wrong length\n"));
			status = DLOAD_STATUS_CAL_RXBADLEN;
			goto error;
		}
		rxcal_dataptr_band += sizeof(struct wl_rxcal_storage_band_hdr_t);
		num_groups = MIN(num_groups, ARRAYSIZE(phy_rssi_gain_delta_2g));

		/* Read group array */
		for (gidx = 0; gidx < WLC_RXCAL_2G_GROUP_SZ; gidx++) {
			grp_2g[2 * gidx + 0] = (rxcal_dataptr_band[gidx] >> 4) & 0xf;
			grp_2g[2 * gidx + 1] = (rxcal_dataptr_band[gidx] >> 0) & 0xf;
		}
		if ((err = wlc_calload_phy_iovar_passthrough(wlc, "rssi_cal_freq_grp_2g", NULL, 0,
			grp_2g, sizeof(grp_2g), IOV_SET, slice)) != BCME_OK) {
			WL_BLOB_DEBUG_ERROR((
				"RxCal Error: IOVAR rssi_cal_freq_grp_2g (err=%d)\n", err));
			status = DLOAD_STATUS_CAL_RXPHYFAILURE;
			goto error;
		}
		rxcal_dataptr_band += WLC_RXCAL_2G_GROUP_SZ;

		/* Read RSSI table: 20MHz and 40MHz only */
		for (gidx = 0; gidx < num_groups; gidx++) {
#if (LCN20CONF > 0)
			uint8 delta_gain[NUM_GAINS_MAX];
			memcpy(&delta_gain[0], rxcal_dataptr_band, num_gains);
			rxcal_dataptr_band += num_gains;
			if ((err = wlc_calload_phy_iovar_passthrough(wlc,
				phy_rssi_gain_delta_2g[gidx], NULL, 0, delta_gain,
				num_gains, IOV_SET, slice)) != BCME_OK) {
				WL_BLOB_DEBUG_ERROR((
					"RxCal Error: IOVAR phy_rssi_gain_delta_2gb[%d]"
					" (err=%d)\n", gidx, err));
				status = DLOAD_STATUS_CAL_RXPHYFAILURE;
				goto error;
			}
#else
			uint8 delta_gain[1 + NUM_BANDS_2G * NUM_GAINS_MAX];
			for (cidx = 0; cidx < num_cores; cidx++) {
				delta_gain[0] = cidx;
				memcpy(&delta_gain[1], rxcal_dataptr_band,
					NUM_BANDS_2G * num_gains);
				rxcal_dataptr_band += NUM_BANDS_2G * num_gains;

				if ((err = wlc_calload_phy_iovar_passthrough(wlc,
					phy_rssi_gain_delta_2g[gidx], NULL, 0, delta_gain,
					(1 + NUM_BANDS_2G * num_gains), IOV_SET, slice))
					!= BCME_OK) {
					WL_BLOB_DEBUG_ERROR((
						"RxCal Error: IOVAR phy_rssi_gain_delta_2gb[%d]"
						" (err=%d)\n", gidx, err));
					status = DLOAD_STATUS_CAL_RXPHYFAILURE;
					goto error;
				}
			}
#endif /* LCN20CONF */
		}

		/* Update total read pointer and length */
		rxcal_dataptr += rxcal_len_band;
		rxcal_len_total -= rxcal_len_band;
	}

#if (LCN20CONF == 0)
	/* Read RXCAL: optional 5GHz table */
	if (rxcal_len_total > sizeof(struct wl_rxcal_storage_band_hdr_t)) {
		uint8 cidx, gidx;
		uint16 rxcal_len_band, rxcal_len_actual;
		uint8 *rxcal_dataptr_band, delta_gain[1 + NUM_BANDS_5G * NUM_GAINS_MAX];

		/* Length check */
		rxcal_dataptr_band = rxcal_dataptr;
		rxcal_len_band = load16_ua(rxcal_dataptr_band) + sizeof(rxcal_len_band);
		num_groups = rxcal_dataptr_band[
				OFFSETOF(struct wl_rxcal_storage_band_hdr_t, num_group)];
		num_gains = NUM_GAINS_MAX;
		if (rxcal_ver > RXCAL_VER_0)
			num_gains = MIN(num_gains, rxcal_dataptr_band[
				OFFSETOF(struct wl_rxcal_storage_band_hdr_t, num_gain)]);
		rxcal_len_actual = sizeof(struct wl_rxcal_storage_band_hdr_t) +
				(NUM_BANDS_5G * num_gains * num_cores * num_groups);
		if (rxcal_len_band != rxcal_len_actual) {
			WL_BLOB_DEBUG_ERROR(("RxCal Error: 5GHz table wrong length\n"));
			status = DLOAD_STATUS_CAL_RXBADLEN;
			goto error;
		}
		rxcal_dataptr_band += sizeof(struct wl_rxcal_storage_band_hdr_t);
		num_groups = MIN(num_groups, ARRAYSIZE(phy_rssi_gain_delta_5g));

		/* Read RSSI table: 20MHz, 40MHz and 80MHz */
		for (gidx = 0; gidx < num_groups; gidx++) {
			for (cidx = 0; cidx < num_cores; cidx++) {
				delta_gain[0] = cidx;
				memcpy(&delta_gain[1], rxcal_dataptr_band,
					NUM_BANDS_5G * num_gains);
				rxcal_dataptr_band += NUM_BANDS_5G * num_gains;

				if ((err = wlc_calload_phy_iovar_passthrough(wlc,
					phy_rssi_gain_delta_5g[gidx], NULL, 0, delta_gain,
					(1 + NUM_BANDS_5G * num_gains), IOV_SET, slice))
					!= BCME_OK) {
					WL_BLOB_DEBUG_ERROR((
						"RxCal Error: IOVAR phy_rssi_gain_delta_5g[%d]"
						" (err=%d)\n", gidx, err));
					status = DLOAD_STATUS_CAL_RXPHYFAILURE;
					goto error;
				}
			}
		}
	}
#endif /* LCN20CONF */

error:
	return status;
}

static dload_error_status_t
wlc_handle_cal_dload_txcal_slice(wlc_info_t *wlc, wlc_blob_segment_t *seg_txcal)
{
	int err;
	dload_error_status_t status = DLOAD_STATUS_DOWNLOAD_SUCCESS;
	uint16 txcal_len_total, txcal_ver;
	uint8 *txcal_dataptr;
	uint32 total_channels;
	uint8 temp;
	uint8 slice = 0; /* NULL */
	uint8 num_cores, num_bands;
	uint32 apply = 1;
	int i, j;
	int idx;
	wlc_info_t *wlc_iter;
	wl_txcal_power_tssi_ncore_t * txcal_tssi;
	wl_txcal_power_tssi_percore_t * per_core;
	uint32 buf_size;

	/* Total buffer size to be allocated */
	buf_size = sizeof(wl_txcal_power_tssi_ncore_t) +
		(PHY_CORE_MAX - 1) * sizeof(wl_txcal_power_tssi_percore_t);
	txcal_tssi = MALLOCZ(wlc->osh, buf_size);
	if (txcal_tssi == NULL) {
		WL_BLOB_DEBUG_ERROR(("TxCal Error: malloc failure\n"));
		status = DLOAD_STATUS_BLOB_NOMEM;
		goto error;
	}

	/* Process actual TXCAL data segment */
	if ((seg_txcal == NULL) || (seg_txcal->length < sizeof(struct wl_txcal_storage_hdr_t))) {
		WL_BLOB_DEBUG_ERROR(("TxCal Error: missing segment\n"));
		status = DLOAD_STATUS_CAL_NOTXCAL;
		goto error;
	}
	txcal_dataptr = seg_txcal->data;
	txcal_len_total = load16_ua(txcal_dataptr) + sizeof(txcal_len_total);

	/* Check that the header is big enough to access fixed fields */
	if ((txcal_len_total < sizeof(struct wl_txcal_storage_hdr_t)) ||
	    (txcal_len_total > seg_txcal->length)) {
		WL_BLOB_DEBUG_ERROR(("TxCal Error: segment too short (%d bytes)\n",
			txcal_len_total));
		status = DLOAD_STATUS_CAL_TXBADLEN;
		goto error;
	}

	/* Check for MAX version supported by PHY */
	txcal_ver = load16_ua(&txcal_dataptr[OFFSETOF(struct wl_txcal_storage_hdr_t, ver)]);
	if (txcal_ver > TXCAL_VER_MAX) {
		WL_BLOB_DEBUG_ERROR(("TxCal Error: unsupported new version (%d > %d)\n",
			txcal_ver, TXCAL_VER_MAX));
		status = DLOAD_STATUS_CAL_TXBADVER;
		goto error;
	} else if (txcal_ver != TXCAL_VER_MAX) {
		WL_PRINT(("TxCal Info: Uses older version %d (current version is %d)\n",
			txcal_ver, TXCAL_VER_MAX));
	}

	/* Skipping device and calibration types */
	temp = txcal_dataptr[OFFSETOF(struct wl_txcal_storage_hdr_t, temp)];
	num_cores = txcal_dataptr[OFFSETOF(struct wl_txcal_storage_hdr_t, num_core)];
	num_bands = txcal_dataptr[OFFSETOF(struct wl_txcal_storage_hdr_t, num_band)];
	slice = txcal_dataptr[OFFSETOF(struct wl_txcal_storage_hdr_t, slice)];
	if ((txcal_ver >= TXCAL_VER_4) && (slice >= MAX_RSDB_MAC_NUM)) {
		/* Check for dual MAC RSDB - multiple slice [aka MAC] is supported from V4 */
		WL_BLOB_DEBUG_ERROR(("TxCal Error: bad slice number (%d)\n", slice));
		status = DLOAD_STATUS_CAL_WRONGMACIDX;
		goto error;
	}

	if (num_bands > (NUM_SUBBANDS_FOR_AVVMID + 1)) {
		WL_BLOB_DEBUG_ERROR(("TxCal Error: too many bands (%d) configured\n",
			num_bands));
		status = DLOAD_STATUS_CAL_TXBADBAND;
		goto error;
	}
	if (num_cores > PHY_CORE_MAX) {
		WL_BLOB_DEBUG_ERROR(("TxCal Error: wrong number of cores (%d > %d)\n",
			num_cores, PHY_CORE_MAX));
		status = DLOAD_STATUS_CAL_TXBADCORE;
		goto error;
	}
	WL_BLOB_DEBUG_INFO(("TxCal Info: Total %d bytes - %dC - %d bands - %d cores - MAC[%d]\n",
			txcal_len_total, temp, num_bands, num_cores, slice));
	txcal_dataptr += sizeof(struct wl_txcal_storage_hdr_t);
	txcal_len_total -= sizeof(struct wl_txcal_storage_hdr_t);

	/* Av/Vmid handling (not available in 1st TXCAL version) */
	if ((txcal_ver >= TXCAL_VER_2) && (num_bands > 0)) {
		int len_av_vmid = (2 * num_cores * num_bands);
		wlc_phy_avvmid_txcal_t avvmid;
		if (txcal_len_total < len_av_vmid) {
			WL_BLOB_DEBUG_ERROR(("TxCal Error: too short (%d bytes) for AvVmid\n",
				txcal_len_total));
			status = DLOAD_STATUS_CAL_TXBADLEN;
			goto error;
		}

		ASSERT(num_bands >= NUM_SUBBANDS_FOR_AVVMID);
		memset(&avvmid, 0, sizeof(avvmid));
		WL_BLOB_DEBUG_INFO(("TxCal Info: AvVmid:\n"));
		for (i = 0; i < num_bands; i++) {
			if (i == NUM_SUBBANDS_FOR_AVVMID) {
				WL_BLOB_DEBUG_ERROR(("\tB%d(skipped)\n", i));
				txcal_dataptr += 2 * num_cores;
				continue;
			}
			WL_BLOB_DEBUG_INFO(("\t"));
			for (j = 0; j < num_cores; j++) {
				avvmid.avvmid[j][i].Av = *txcal_dataptr++;
				avvmid.avvmid[j][i].Vmid = *txcal_dataptr++;
				WL_BLOB_DEBUG_INFO((" B%dC%d(%d %d)", i, j,
					avvmid.avvmid[j][i].Av,
					avvmid.avvmid[j][i].Vmid));
			}
			WL_BLOB_DEBUG_INFO(("\n"));
		}
		txcal_len_total -= len_av_vmid;
#ifdef WLRSDB
		if (WLC_DUALMAC_RSDB(wlc->cmn)) {
			wlc_phy_avvmid_txcal(WLC_PI(wlc->cmn->wlc[slice]), &avvmid, TRUE);
		} else
#endif /* WLRSDB */
		{
			FOREACH_WLC(wlc->cmn, idx, wlc_iter) {
				wlc_phy_avvmid_txcal(WLC_PI(wlc_iter), &avvmid, TRUE);
			}
		}
	}

	/* Per channel TSSI table per handling */
	total_channels = 0;
	while (txcal_len_total >= sizeof(struct wl_txcal_storage_ch_hdr_t)) {
		uint16 txcal_len_ch, txcal_len_ch_chk;
		uint8 num_chs;
		uint8 num_sts[NUM_SUBBANDS_FOR_AVVMID+1][PHY_CORE_MAX];

		/* txcal_len_ch = the size of whole TxCal channel data,
		 * including the length field itself.
		 */
		txcal_len_ch = load16_ua(txcal_dataptr) + sizeof(txcal_len_ch);
		txcal_dataptr += sizeof(txcal_len_ch);
		txcal_len_ch_chk = txcal_len_ch - sizeof(txcal_len_ch);
		WL_BLOB_DEBUG_INFO(("TxCal Info: Channel band start - %d bytes\n", txcal_len_ch));
		if ((txcal_ver >= TXCAL_VER_3) && (num_bands > 0)) {
			/* Since VER3, channels have per band/core number of steps */
			if (txcal_len_ch_chk <= (num_bands * num_cores)) {
				WL_BLOB_DEBUG_ERROR(("TxCal Error: data len error - hdr3\n"));
				status = DLOAD_STATUS_CAL_TXCHANBADLEN;
				goto error;
			}
			WL_BLOB_DEBUG_INFO(("\tPer band/core steps:"));
			for (i = 0; i < num_bands; i++) {
				for (j = 0; j < num_cores; j++) {
					num_sts[i][j] = *txcal_dataptr++;
					txcal_len_ch_chk--;
					WL_BLOB_DEBUG_INFO((" B%dC%d(%d)", i, j,
						num_sts[i][j]));
				}
			}
			WL_BLOB_DEBUG_INFO(("\n"));
		} else {
			/* Before VER3, all channels have the same number of steps */
			if (txcal_len_ch_chk <= 1) {
				WL_BLOB_DEBUG_ERROR(("TxCal Error: data len error - hdr\n"));
				status = DLOAD_STATUS_CAL_TXCHANBADLEN;
				goto error;
			}
			memset(&num_sts[0][0], *txcal_dataptr++, sizeof(num_sts));
			txcal_len_ch_chk--;
			WL_BLOB_DEBUG_INFO(("\tSingle band w/%d steps\n", num_sts[0][0]));
		}
		num_chs = *txcal_dataptr++;
		txcal_len_ch_chk--;

		total_channels += num_chs;
		WL_BLOB_DEBUG_INFO(("\tTotal %d channels (%d in this band)\n",
			total_channels, num_chs));
		while (num_chs--) {
			uint8 chan, ptssi, num_sts_all_core;
			uint8 *num_sts_per_core;

			if (txcal_len_ch_chk < (num_cores + 2)) {
				WL_BLOB_DEBUG_ERROR(("TxCal Error: data len error - too short\n"));
				status = DLOAD_STATUS_CAL_TXCHANBADLEN;
				goto error;
			}
			chan = *txcal_dataptr++;
			ptssi = *txcal_dataptr++;
			num_sts_per_core = num_sts[WLC_TXCAL_CH_TO_BAND(chan)];

			num_sts_all_core = 0;
			txcal_tssi->set_core = 0;
			txcal_tssi->channel = chan;
			txcal_tssi->version = TXCAL_IOVAR_VERSION;
			per_core = txcal_tssi->tssi_percore;
			WL_BLOB_DEBUG_INFO(("\t\t[Channel %d start %ddB]:", chan, ptssi));
			for (i = 0; i < num_cores; i++) {
				per_core->tempsense = temp;
				per_core->pwr_start = ptssi;
				per_core->pwr_start_idx = *txcal_dataptr++;
				per_core->num_entries = num_sts_per_core[i];
				num_sts_all_core = MAX(num_sts_all_core, num_sts_per_core[i]);
				if (i == 0)
					WL_BLOB_DEBUG_INFO((" idx:%d - steps:MAX(%d",
						per_core->pwr_start_idx,
						per_core->num_entries));
				else
					WL_BLOB_DEBUG_INFO((",%d", per_core->num_entries));
				per_core++;
			}
			WL_BLOB_DEBUG_INFO((")=%d\n", num_sts_all_core));
			txcal_len_ch_chk -= (num_cores + 2);

			if (txcal_len_ch_chk < (num_sts_all_core * num_cores)) {
				WL_BLOB_DEBUG_ERROR(("TxCal Error: data len error - per core\n"));
				status = DLOAD_STATUS_CAL_TXCHANBADLEN;
				goto error;
			}
			for (j = 0; j < num_sts_all_core; j++) {
				per_core = txcal_tssi->tssi_percore;
				for (i = 0; i < num_cores; i++) {
					per_core->tssi[j] = *txcal_dataptr++;
					per_core++;
				}
			}
			txcal_len_ch_chk -= num_sts_all_core * num_cores;

			/* Call PHY iovar here */
			if ((err = wlc_calload_phy_iovar_passthrough(wlc, "txcal_pwr_tssi_tbl",
				NULL, 0, txcal_tssi, buf_size, IOV_SET, slice)) != BCME_OK) {
				WL_BLOB_DEBUG_ERROR((
					"TxCal Error: IOVAR txcal_pwr_tssi_tbl (err=%d)\n", err));
				status = DLOAD_STATUS_CAL_TXPHYFAILURE;
				goto error;
			}
		}
		if ((txcal_len_total < txcal_len_ch) || (txcal_len_ch_chk != 0)) {
			WL_BLOB_DEBUG_ERROR(("TxCal Error: data len error - total\n"));
			status = DLOAD_STATUS_CAL_TXCHANBADLEN;
			goto error;
		}
		txcal_len_total -= txcal_len_ch;
	}

	if (total_channels == 0) {
		WL_BLOB_DEBUG_ERROR(("TxCal Error: no channels\n"));
		status = DLOAD_STATUS_CAL_TXNOCHAN;
		goto error;
	}

	/* Call PHY iovar to activate TXCAL table (after successful load) */
	if ((err = wlc_calload_phy_iovar_passthrough(wlc, "txcal_apply_pwr_tssi_tbl",
			NULL, 0, &apply, sizeof(apply), IOV_SET, slice)) != BCME_OK) {
		WL_BLOB_DEBUG_ERROR((
			"TxCal Error: IOVAR txcal_apply_pwr_tssi_tbl (err=%d)\n", err));
		status = DLOAD_STATUS_CAL_TXPHYFAILURE;
		goto error;
	}

error:
	MFREE(wlc->osh, txcal_tssi, buf_size);
	return status;
}

dload_error_status_t
wlc_handle_cal_dload(wlc_info_t *wlc, wlc_blob_segment_t *segments, uint32 segment_count)
{
	dload_error_status_t status = DLOAD_STATUS_DOWNLOAD_SUCCESS;
	uint32 chip;
	int i, j;
	wlc_blob_segment_t *seg_version = NULL, *seg_chipid = NULL;
	wlc_blob_segment_t *seg_txcal[MAX_RSDB_MAC_NUM] = {NULL};
	int k;
	wlc_blob_segment_t *seg_rxcal[MAX_RSDB_MAC_NUM] = {NULL};

	/* Check to see if chip id segment is correct length */
	for (i = 0, j = 0, k = 0; i < segment_count; i++) {
		switch (segments[i].type) {
		case DLOAD_CAL_SEGTYPE_NONE:
			/* Backward compatible with 2-segment blob format */
			if (i == 0)
				seg_txcal[j++] = &segments[i];
			else if (i == 1)
				seg_chipid = &segments[i];
			break;

		case DLOAD_CAL_SEGTYPE_CHIPID:
			seg_chipid = &segments[i];
			break;

		case DLOAD_CAL_SEGTYPE_VERSION:
			seg_version = &segments[i];
			break;

		case DLOAD_CAL_SEGTYPE_TXCAL:
			if (j < MAX_RSDB_MAC_NUM)
				seg_txcal[j++] = &segments[i];
			break;
		case DLOAD_CAL_SEGTYPE_RXCAL:
			if (k < MAX_RSDB_MAC_NUM)
				seg_rxcal[k++] = &segments[i];
			break;

		default:
			break;	/* ignore unknown segments */
		}
	}

	/* Process chip ID */
	if (seg_chipid == NULL) {
		WL_BLOB_DEBUG_ERROR(("TRxCal Error: missing chip ID\n"));
		status = DLOAD_STATUS_CAL_NOCHIPID;
		goto error;
	}
	if (seg_chipid->length != sizeof(chip)) {
		WL_BLOB_DEBUG_ERROR(("TRxCal Error: wrong chip format\n"));
		status = DLOAD_STATUS_CAL_INVALIDCHIPID;
		goto error;
	}
	/* Check to see if chip id matches this chip's actual value */
	chip = load32_ua(seg_chipid->data);
	if (chip != wlc->pub->sih->chip) {
		WL_PRINT(("TRxCal Error: wrong chip ID 0x%04x (expect 0x%04x)\n",
			chip, wlc->pub->sih->chip));
		status = DLOAD_STATUS_CAL_WRONGCHIPID;
		goto error;
	}
	WL_BLOB_DEBUG_INFO(("TRxCal Info: chip ID = 0x%04x\n", chip));

	/* Print the readable version string if present */
	memset(&wlc->cldi->calload_chkver[0], 0, ETHER_ADDR_LEN);
	if (seg_version && (seg_version->length != 0)) {
		/* Find the embedded 6-byte SN (serial nuhmber) string and save for later check:
		 * The format of SN as appearing in the version string: MODSN=nnnnnn (6 bytes)
		 */
		const char SN_STR[] = "MODSN=";
		if (seg_version->length > ETHER_ADDR_LEN) {
			char *sn = bcmstrnstr((char *)seg_version->data,
					(seg_version->length - ETHER_ADDR_LEN),
					SN_STR, sizeof(SN_STR) - 1);
			if (sn) {
				memcpy(&wlc->cldi->calload_chkver[0], sn + sizeof(SN_STR) - 1,
					ETHER_ADDR_LEN);
			}
		}
		seg_version->data[seg_version->length - 1] = NULL;
		WL_BLOB_DEBUG_INFO(("TRxCal Info: VerStr = %s\n", (char *)(seg_version->data)));
	}

	/* Process actual TXCAL data segment */
#ifndef WLTEST
	if (seg_txcal[0] == NULL) {
		WL_BLOB_DEBUG_ERROR(("TxCal Error: Missing TxCal\n"));
		status = DLOAD_STATUS_CAL_NOTXCAL;
		goto error;
	}
#endif /* WLTEST */

	for (i = 0; i < MAX_RSDB_MAC_NUM; i++) {
		if (seg_txcal[i] == NULL) {
			if (i == 0)
				WL_BLOB_DEBUG_ERROR(("TxCal Error: Missing TxCal\n"));
			break;
		}

		WL_BLOB_DEBUG_INFO(("TxCal Info: Processing TxCal slice[%d]\n", i));
		status = wlc_handle_cal_dload_txcal_slice(wlc, seg_txcal[i]);
		if (status != DLOAD_STATUS_DOWNLOAD_SUCCESS) {
			WL_BLOB_DEBUG_ERROR(("TxCal Error: Handling slice %d\n", i));
			goto error;
		}
	}

	/* Process actual RXCAL data segment */
	for (i = 0; i < MAX_RSDB_MAC_NUM; i++) {
		if (seg_rxcal[i] == NULL) {
			if (i == 0)
				WL_BLOB_DEBUG_ERROR(("RxCal Error: Missing RxCal\n"));
			break;
		}

		WL_BLOB_DEBUG_INFO(("RxCal Info: Processing slice[%d]\n", i));
		status = wlc_handle_cal_dload_rxcal_slice(wlc, seg_rxcal[i]);
		if (status != DLOAD_STATUS_DOWNLOAD_SUCCESS) {
			WL_BLOB_DEBUG_ERROR(("RxCal Error: Handling slice %d\n", i));
			goto error;
		}
	}

error:
	return status;
}

#ifdef WLTEST
static int
wlc_handle_cal_dump_channel_range(wlc_info_t *wlc, wl_txcal_power_tssi_t *tbl,
	int num_ranges, const wlc_cal_chan_range_t *ch_range, uint16 *tbl_idx)
{
	int ret = BCME_OK;
	int i, j, channel;
	uint16 channel_num = *tbl_idx;
	wl_txcal_power_tssi_ncore_t * txcal_tssi;
	wl_txcal_power_tssi_percore_t * per_core;
	uint32 buf_size;

	buf_size = sizeof(wl_txcal_power_tssi_ncore_t) +
		(PHY_CORE_MAX - 1) * sizeof(wl_txcal_power_tssi_percore_t);
	txcal_tssi = MALLOCZ(wlc->osh, buf_size);
	if (txcal_tssi == NULL) {
		return BCME_NOMEM;
	}

	for (i = 0; i < num_ranges; i++) {
		channel = ch_range[i].ch_first;
		if (channel == 0)
			continue;	/* Skip empty range */

		for (; channel <= ch_range[i].ch_last; channel += ch_range[i].ch_inc) {
			if (channel_num >= MAX_TXCAL_PWRTBL_ENTRY) {
				WL_ERROR(("Too few TxCal TSSI table entries allowed\n"));
				goto error; /* Just return max number of entries */
			}

			txcal_tssi->version = TXCAL_IOVAR_VERSION;
			txcal_tssi->channel = channel;
			if ((ret = wlc_iovar_op(wlc, "txcal_pwr_tssi_tbl",
					NULL, 0, txcal_tssi,
					buf_size, 0, NULL)) != BCME_OK) {
				goto error;
			}
			tbl[channel_num].set_core = txcal_tssi->set_core;
			tbl[channel_num].channel = txcal_tssi->channel;
			tbl[channel_num].gen_tbl = txcal_tssi->gen_tbl;
			per_core = txcal_tssi->tssi_percore;

			for (j = 0; j < PHY_CORE_MAX; j++) {
				tbl[channel_num].tempsense[j] = per_core->tempsense;
				tbl[channel_num].pwr_start[j] = per_core->pwr_start;
				tbl[channel_num].pwr_start_idx[j] = per_core->pwr_start_idx;
				tbl[channel_num].num_entries[j] = per_core->num_entries;
				memcpy(&tbl[channel_num].tssi[j], &per_core->tssi,
						MAX_NUM_PWR_STEP*sizeof(per_core->tssi[0]));
				per_core++;
			}


			if (tbl[channel_num].num_entries[0] == 0)
				continue;

			channel_num++;
		}
	}
	(*tbl_idx) = channel_num;

error:
	MFREE(wlc->osh, txcal_tssi, buf_size);
	return ret;
}

static int wlc_handle_txcal_dump(wlc_info_t *wlc, void *buf, int buf_len)
{
	int ret = BCME_OK;
	wl_txcal_power_tssi_t *tbl = NULL;
	int16 txcal_temp;
	uint16 txcal_num_band, txcal_num_core, txcal_num_ch;
	uint16 txcal_num_chbw[3] = {0, 0, 0};	/* number of channel per BW */
	uint16 txcal_len_chbw[3] = {0, 0, 0};	/* length per BW */
	uint16 txcal_len_detail[3][NUM_SUBBANDS_FOR_AVVMID][PHY_CORE_MAX+2];
	wlc_phy_avvmid_txcal_t avvmid;
	int chbw, i, j, k;
	unsigned char *new_buf = NULL;
	unsigned char *dataptr = NULL;
	unsigned int new_buf_sz;
	struct wl_txcal_storage_hdr_t *txcal_storage_hdr;
	const wlc_cal_chan_range_t ch_ranges[3][NUM_SUBBANDS_FOR_AVVMID] = {
		{{1, 13, 1}, {36, 48, 4}, {52, 64, 4}, {100, 144, 4}, {149, 165, 4}},	/* 20MHz */
		{{0, 0, 0}, {38, 46, 8}, {54, 62, 8}, {102, 142, 8}, {151, 159, 8}},	/* 40MHz */
		{{0, 0, 0}, {42, 42, 16}, {58, 58, 16}, {106, 138, 16}, {155, 155, 16}}	/* 80MHz */
	};

	tbl = (wl_txcal_power_tssi_t *)MALLOCZ(
		wlc->osh, MAX_TXCAL_PWRTBL_ENTRY * sizeof(wl_txcal_power_tssi_t));
	if (tbl == NULL) {
		ret = BCME_NOMEM;
		goto error;
	}

	/* Read back TX calibration information */
	txcal_temp = 0;
	txcal_num_band = NUM_SUBBANDS_FOR_AVVMID;
	txcal_num_core = phy_numofcores(WLC_PI(wlc));
	txcal_num_ch = 0;
	memset(txcal_len_detail, 0, sizeof(txcal_len_detail));
	new_buf_sz = sizeof(struct wl_txcal_storage_hdr_t);

	/* Read Av and Vmid */
	memset(&avvmid, 0, sizeof(avvmid));
	wlc_phy_avvmid_txcal(WLC_PI(wlc), &avvmid, FALSE);
	new_buf_sz += 2 * txcal_num_band * txcal_num_core;	/* Av & Vmid per band/core */

	/* Read channel TSSI tables */
	for (chbw = 0; chbw < 3; chbw++) {
		txcal_num_chbw[chbw] = txcal_num_ch;
		if ((ret = wlc_handle_cal_dump_channel_range(
				wlc, tbl, txcal_num_band,
				ch_ranges[chbw], &txcal_num_ch)))
			goto error;
		if (txcal_num_ch > txcal_num_chbw[chbw]) {
			txcal_temp = tbl[txcal_num_chbw[chbw]].tempsense[0];
			for (i = txcal_num_chbw[chbw]; i < txcal_num_ch; i++) {
				uint8 band = WLC_TXCAL_CH_TO_BAND(tbl[i].channel);
				for (j = 0; j < txcal_num_core; j++) {
					txcal_len_detail[chbw][band][j] = tbl[i].num_entries[j];
					txcal_len_detail[chbw][band][PHY_CORE_MAX] = MAX(
						txcal_len_detail[chbw][band][PHY_CORE_MAX],
						txcal_len_detail[chbw][band][j]);
				}
				txcal_len_detail[chbw][band][PHY_CORE_MAX + 1]++;
			}
			txcal_num_chbw[chbw] = txcal_num_ch - txcal_num_chbw[chbw];
			new_buf_sz += sizeof(struct wl_txcal_storage_ch_hdr_t)
				+ txcal_num_chbw[chbw] * sizeof(struct wl_txcal_storage_ch_pwr_t);
			txcal_len_chbw[chbw] = 0;
			for (i = 0; i < txcal_num_band; i++) {
				txcal_len_chbw[chbw] += (txcal_num_core
					* txcal_len_detail[chbw][i][PHY_CORE_MAX + 1]
					* (txcal_len_detail[chbw][i][PHY_CORE_MAX] + 1));
			}
			new_buf_sz += txcal_len_chbw[chbw];
		}
	}
	if (txcal_num_ch == 0) {
		ret = BCME_NOTREADY;
		goto error;
	}

	new_buf = MALLOCZ(wlc->osh, new_buf_sz);
	if (new_buf == NULL) {
		ret = BCME_NOMEM;
		goto error;
	}
	dataptr = new_buf;

	/* Fill in TX Cal according to storage format -- header */
	txcal_storage_hdr = (struct wl_txcal_storage_hdr_t *)dataptr;
	txcal_storage_hdr->len = 0;	/* update with final value later */
	txcal_storage_hdr->ver = TXCAL_VER_MAX;
	txcal_storage_hdr->temp = (uint8)txcal_temp;
	txcal_storage_hdr->num_core = (uint8)txcal_num_core;
	txcal_storage_hdr->num_band = (uint8)txcal_num_band;
	txcal_storage_hdr->slice = 0;

#ifdef WLRSDB
	if (WLC_DUALMAC_RSDB(wlc->cmn)) {
		if (wlc != wlc->cmn->wlc[0])
			txcal_storage_hdr->slice = 1;
	}
#endif /* WLRSDB */

	dataptr += sizeof(struct wl_txcal_storage_hdr_t);

	/* Fill in Av and Vmid */
	if (txcal_storage_hdr->ver >= TXCAL_VER_2) {
		for (i = 0; i < txcal_num_band; i++) {
			for (j = 0; j < txcal_num_core; j++) {
				*dataptr++ = avvmid.avvmid[j][i].Av;
				*dataptr++ = avvmid.avvmid[j][i].Vmid;
			}
		}
	}

	/* Fill in TX Cal according to storage format -- per BW */
	for (chbw = 0, txcal_num_ch = 0; chbw <= 2; chbw++) {
		uint8 num_channels = (uint8)txcal_num_chbw[chbw];

		/* Skip this BW if no channel is included */
		if (txcal_len_chbw[chbw] == 0)
			continue;

		store16_ua(dataptr, (txcal_num_band * txcal_num_core
			+ 1
			+ num_channels * sizeof(struct wl_txcal_storage_ch_pwr_t)
			+ txcal_len_chbw[chbw]));
		dataptr += sizeof(uint16);
		for (i = 0; i < txcal_num_band; i++) {
			for (j = 0; j < txcal_num_core; j++) {
				*dataptr++ = txcal_len_detail[chbw][i][j];
			}
		}
		*dataptr++ = num_channels;
		for (i = 0; i < num_channels; i++, txcal_num_ch++) {
			uint8 band = WLC_TXCAL_CH_TO_BAND(tbl[txcal_num_ch].channel);
			uint8 txcal_pwr_start = tbl[txcal_num_ch].pwr_start[0];
			*dataptr++ = tbl[txcal_num_ch].channel;
			*dataptr++ = txcal_pwr_start;

			for (k = 0; k < txcal_num_core; k++) {
				*dataptr++ = tbl[txcal_num_ch].pwr_start_idx[k];
				if (tbl[txcal_num_ch].pwr_start[k] != txcal_pwr_start)
					WL_ERROR(("TxCal table: diff Ptssi for diff core\n"));
				if (tbl[txcal_num_ch].tempsense[k] != txcal_temp)
					WL_ERROR(("TxCal table: diff temp for diff core\n"));
				if (tbl[txcal_num_ch].num_entries[k] >
					txcal_len_detail[chbw][band][PHY_CORE_MAX])
					WL_ERROR(("TxCal table: diff steps for diff core\n"));
			}

			for (j = 0; j < txcal_len_detail[chbw][band][PHY_CORE_MAX]; j++) {
				for (k = 0; k < txcal_num_core; k++)
					*dataptr++ = tbl[txcal_num_ch].tssi[k][j];
			}
		}
	}

	new_buf_sz = (unsigned int)(dataptr - new_buf);
	txcal_storage_hdr->len = new_buf_sz - sizeof(txcal_storage_hdr->len);
	if (new_buf_sz < buf_len) {
		memcpy(buf, new_buf, new_buf_sz);
		ret = new_buf_sz;
	} else {
		ret = BCME_BUFTOOSHORT;
	}

error:
	if (tbl)
		MFREE(wlc->osh, tbl, MAX_TXCAL_PWRTBL_ENTRY * sizeof(wl_txcal_power_tssi_t));
	if (new_buf)
		MFREE(wlc->osh, new_buf, new_buf_sz);

	return ret;
}

static int wlc_handle_rxcal_dump(wlc_info_t *wlc, void *buf, int buf_len)
{
	int ret = BCME_OK;
	void *buf_orig = buf;
	uint32 temp;
	uint8 slice = 0, num_cores, num_groups, num_bands;
	uint8 group_2g[2 * WLC_RXCAL_2G_GROUP_SZ];
	struct wl_rxcal_storage_hdr_t rxcal_storage_hdr;
	uint8 gidx;
	unsigned int rxcal_sz_2g;
	struct wl_rxcal_storage_band_hdr_t rxcal_storage_2g;
#if (LCN20CONF == 0)
	uint8 cidx;
	unsigned int rxcal_sz_5g;
	struct wl_rxcal_storage_band_hdr_t rxcal_storage_5g;
#endif /* LCN20CONF */

	/* Collect some basic common info */
	if ((ret = wlc_iovar_op(wlc, "phy_rssi_gain_cal_temp",
			NULL, 0, &temp, sizeof(temp), 0, NULL)) != BCME_OK) {
		goto error;
	}
	num_cores = phy_numofcores(WLC_PI(wlc));
#ifdef WLRSDB
	slice = (WLC_DUALMAC_RSDB(wlc->cmn) && (wlc != wlc->cmn->wlc[0])) ? 1 : 0;
#endif /* WLRSDB */
	memset(&rxcal_storage_hdr, 0, sizeof(rxcal_storage_hdr));
	rxcal_storage_hdr.len = sizeof(rxcal_storage_hdr) - sizeof(rxcal_storage_hdr.len);
	rxcal_storage_hdr.ver = RXCAL_VER_MAX;
	rxcal_storage_hdr.temp = (uint8)temp;
	rxcal_storage_hdr.num_core = num_cores;
	rxcal_storage_hdr.slice = slice;
	if (buf_len < sizeof(rxcal_storage_hdr)) {
		ret = BCME_BUFTOOSHORT;
		goto error;
	}
	buf += sizeof(rxcal_storage_hdr);
	buf_len -= sizeof(rxcal_storage_hdr);

	/* Fill RxCal data for 2G band */
	if ((ret = wlc_iovar_op(wlc, "rssi_cal_freq_grp_2g", NULL, 0,
		group_2g, sizeof(group_2g), 0, NULL)) != BCME_OK) {
		goto error;
	}
	for (gidx = 0; gidx < WLC_RXCAL_2G_GROUP_SZ; gidx++) {
		group_2g[gidx] = ((group_2g[2 * gidx] & 0xf) << 4) |
			(group_2g[2 * gidx + 1] & 0xf);
	}

	num_groups = ARRAYSIZE(phy_rssi_gain_delta_2g);
#if (LCN20CONF > 0)
	num_bands = NUM_BANDS_SINGLE;
	rxcal_sz_2g = num_bands * num_groups * NUM_GAINS_MAX_LCN20 * num_cores;
#else
	num_bands = NUM_BANDS_2G;
	rxcal_sz_2g = num_bands * num_groups * NUM_GAINS_MAX * num_cores;
#endif /* LCN20CONF */
	rxcal_storage_2g.len = rxcal_sz_2g + WLC_RXCAL_2G_GROUP_SZ +
		(sizeof(rxcal_storage_2g) - sizeof(rxcal_storage_2g.len));
	rxcal_storage_2g.num_group = num_groups;
#if (LCN20CONF > 0)
	rxcal_storage_2g.num_gain = NUM_GAINS_MAX_LCN20;
#else
	rxcal_storage_2g.num_gain = NUM_GAINS_MAX;
#endif

	if (buf_len < (sizeof(rxcal_storage_2g) + rxcal_sz_2g + WLC_RXCAL_2G_GROUP_SZ)) {
		ret = BCME_BUFTOOSHORT;
		goto error;
	}
	memcpy(buf, &rxcal_storage_2g, sizeof(rxcal_storage_2g));
	buf += sizeof(rxcal_storage_2g);
	buf_len -= sizeof(rxcal_storage_2g);
	memcpy(buf, group_2g, WLC_RXCAL_2G_GROUP_SZ);
	buf += WLC_RXCAL_2G_GROUP_SZ;
	buf_len -= WLC_RXCAL_2G_GROUP_SZ;

	for (gidx = 0; gidx < num_groups; gidx++) {
#if (LCN20CONF > 0)
		uint8 delta_gain[NUM_BANDS_SINGLE * NUM_GAINS_MAX_LCN20];
		if ((ret = wlc_iovar_op(wlc, phy_rssi_gain_delta_2g[gidx],
			NULL, 0, delta_gain, sizeof(delta_gain), 0, NULL)) != BCME_OK) {
			goto error;
		}
		if (buf_len < sizeof(delta_gain)) {
			ret = BCME_BUFTOOSHORT;
			goto error;
		}
		memcpy(buf, &delta_gain[0], sizeof(delta_gain));
		buf += sizeof(delta_gain);
		buf_len -= sizeof(delta_gain);
#else
		uint8 delta_gain[(1 + NUM_BANDS_2G * NUM_GAINS_MAX) * PHY_CORE_MAX];
		if ((ret = wlc_iovar_op(wlc, phy_rssi_gain_delta_2g[gidx],
			NULL, 0, delta_gain, sizeof(delta_gain), 0, NULL)) != BCME_OK) {
			goto error;
		}
		for (cidx = 0; cidx < num_cores; cidx++) {
			if (buf_len < NUM_BANDS_2G * NUM_GAINS_MAX) {
				ret = BCME_BUFTOOSHORT;
				goto error;
			}
			memcpy(buf, &delta_gain[(1 + NUM_BANDS_2G * NUM_GAINS_MAX) * cidx + 1],
				NUM_BANDS_2G * NUM_GAINS_MAX);
			buf += (NUM_BANDS_2G * NUM_GAINS_MAX);
			buf_len -= (NUM_BANDS_2G * NUM_GAINS_MAX);
		}
#endif /* LCN20CONF */
	}
	rxcal_storage_hdr.len += sizeof(rxcal_storage_2g) + WLC_RXCAL_2G_GROUP_SZ + rxcal_sz_2g;

#if (LCN20CONF == 0)
	/* Fill RxCal data for 5G band */
	num_groups = ARRAYSIZE(phy_rssi_gain_delta_5g);
	num_bands = NUM_BANDS_5G;
	rxcal_sz_5g = num_bands * num_groups * NUM_GAINS_MAX * num_cores;
	rxcal_storage_5g.len = rxcal_sz_5g +
		(sizeof(rxcal_storage_5g) - sizeof(rxcal_storage_5g.len));
	rxcal_storage_5g.num_group = num_groups;
	rxcal_storage_5g.num_gain = NUM_GAINS_MAX;

	if (buf_len < (sizeof(rxcal_storage_5g) + rxcal_sz_5g)) {
		ret = BCME_BUFTOOSHORT;
		goto error;
	}
	memcpy(buf, &rxcal_storage_5g, sizeof(rxcal_storage_5g));
	buf += sizeof(rxcal_storage_5g);
	buf_len -= sizeof(rxcal_storage_5g);

	for (gidx = 0; gidx < num_groups; gidx++) {
		uint8 delta_gain[(1 + NUM_BANDS_5G * NUM_GAINS_MAX) * PHY_CORE_MAX];
		if ((ret = wlc_iovar_op(wlc, phy_rssi_gain_delta_5g[gidx],
			NULL, 0, delta_gain, sizeof(delta_gain), 0, NULL)) != BCME_OK) {
			goto error;
		}
		for (cidx = 0; cidx < num_cores; cidx++) {
			if (buf_len < NUM_BANDS_5G * NUM_GAINS_MAX) {
				ret = BCME_BUFTOOSHORT;
				goto error;
			}
			memcpy(buf, &delta_gain[(1 + NUM_BANDS_5G * NUM_GAINS_MAX) * cidx + 1],
				NUM_BANDS_5G * NUM_GAINS_MAX);
			buf += (NUM_BANDS_5G * NUM_GAINS_MAX);
			buf_len -= (NUM_BANDS_5G * NUM_GAINS_MAX);
		}
	}
	rxcal_storage_hdr.len += sizeof(rxcal_storage_5g) + rxcal_sz_5g;
#endif /* LCN20CONF */

	ret = rxcal_storage_hdr.len + sizeof(rxcal_storage_hdr.len);
	memcpy(buf_orig, &rxcal_storage_hdr, sizeof(rxcal_storage_hdr));

error:
	return ret;
}

int wlc_handle_cal_dump(wlc_info_t *wlc, void *buf, int buf_len)
{
	int32 *total_dump_sz = (int32 *)buf;
	int32 dump_sz;

	if (!ISALIGNED(buf, sizeof(int32))) {
		WL_ERROR(("CAL dump: buffer not %d byte aligned\n", (int)sizeof(int32)));
		return BCME_BADARG;
	}

	/* Dump TXCAL first */
	buf += sizeof(int32);
	buf_len -= sizeof(int32);
	dump_sz = wlc_handle_txcal_dump(wlc, buf, buf_len);
	if (dump_sz < 0) {
		WL_ERROR(("TxCal dump error (err = %d)\n", dump_sz));
		return dump_sz;
	}
	*total_dump_sz = dump_sz;

	/* Dump RXCAL first */
	buf += dump_sz;
	buf_len -= dump_sz;
	dump_sz = wlc_handle_rxcal_dump(wlc, buf, buf_len);
	if (dump_sz < 0) {
		WL_ERROR(("RxCal dump error (err = %d)\n", dump_sz));
		return dump_sz;
	}
	*total_dump_sz += dump_sz;

	return BCME_OK;
}
#endif /* WLTEST */
#endif	/* WLC_TXCAL */
