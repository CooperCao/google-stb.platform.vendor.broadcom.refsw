/*
 * Sensor hub server source file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: $
 *
 */
#include <wlc_cfg.h>

#include <typedefs.h>
#include <osl.h>
#include <wl_dbg.h>
#include <proto/ethernet.h>

#include <wlioctl.h>
#include <wlc_pub.h>

#include <d11.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_export.h>
#include <wlc_scan_utils.h>
#include <wlc_iocv.h>

#include <rte_shif.h>
#include <wl_shub.h>

#define	WL_SHUB_ERR(arg) WL_ERROR(arg)
#define	WL_SHUB_INFO(arg) WL_INFORM(arg)

#define	SHUB_PKT_START	0xC000

#define SHUB_PKT_TYPE_SHIFT	12
#define SHUB_PKT_TYPE_MASK	(0xF << SHUB_PKT_TYPE_SHIFT)
#define	SHUB_PKT_TYPE_DATA	(0x0 << SHUB_PKT_TYPE_SHIFT)
#define	SHUB_PKT_TYPE_COMMAND	(0xF << SHUB_PKT_TYPE_SHIFT)

/* Sensor Hub requests */
#define	SHUB_PKT_COMMAND_MASK	0xFFF
#define SHUB_MOVE_STOP_REQ	0x000
#define SHUB_MOVE_STOP_CNF	0x001
#define SHUB_SCAN_DATA_IND	0x003

#define SHUB_MOVE_STOP_IND	0x000
#define SHUB_SCAN_REQ		0x006
#define SHUB_SCAN_CNF		0x007
#define SHUB_SCAN_STOP		0x008

#define SHUB_REQ_OK	0
#define SHUB_REQ_FAIL 1

#define SHUB_PACKET_LEN	60
#define	SHUB_DATA_LEN	50
#define SHUB_SCAN_CNF_DATA_LEN	1

/* default scan frequency when motion detected */
#define SH_MOTION_SCANFREQ	10 * 1000
/* default scan frequency when idle */
#define SH_IDLE_SCANFREQ	300 * 1000


#define SHUB_INVALID_PIN	0xF
#define SHUB_WAKE_PINS_MASK		(0xF)
#define SHUB_WAKE_SH_PIN_SHIFT		(0x4)
#define SHUB_WAKE_WLAN_PIN_SHIFT	(0x0)

#define SHUB_GET_WAKE_SH_PIN(p)		((p >> SHUB_WAKE_SH_PIN_SHIFT) & SHUB_WAKE_PINS_MASK)
#define SHUB_GET_WAKE_WLAN_PIN(p)	((p >> SHUB_WAKE_WLAN_PIN_SHIFT) & \
						SHUB_WAKE_PINS_MASK)

#define SHUB_AP_INFO_SIZE	8
#define SHUB_MAX_AP_PER_RESP	6
#define SHUB_MAX_AP_PER_SCAN  150

/* offset into scan info */
#define	RSSI_LSB_OFFSET	6
#define	RSSI_MSB_OFFSET	7

typedef struct _shub_pkt {
	uint16	start;
	uint16	type;
	uint16	interval;
	uint16	option;
	uint16	length;
	uint8	data[SHUB_DATA_LEN];
} shub_pkt_t;


/* sensor hub server data object */
struct _shub_cmn_info_t {
	shif_info_t	*shifp;			/* reference to the HW interface */
	int32		sh_motion_scanfreq;	/* period between scan when movement detected */
	int32		sh_idle_scanfreq;	/* period between scan when idle */
	bool		shub_mv_stp_enabled;	/* move/stop indication processing enabled */
	int		ap_count;		/* number of AP information currently in buffer */
	uint8		*sh_scan_result;	/* holds scan result */
	bool		scan_in_progress;	/* indicates ther is SH scan request */
	int		aps_reported;		/* number of APs passed to SH */
	struct wl_timer	*timer;			/* timer to handle async tx */
};
struct wl_shub_info {
	wlc_info_t	*wlc;
	struct _shub_cmn_info_t	*cmn;
	uint8	shub_wake_sh_pin;
	uint8	shub_wake_int_pin;
};

enum {
	IOV_SHUB,
	IOV_SHUB_REQ,
	IOV_SHUB_LAST
};

static const bcm_iovar_t shub_iovars[] = {
	{"shub", IOV_SHUB,
	(0), 0, IOVT_BOOL, 0
	},
	{"shub_req", IOV_SHUB_REQ,
	(0), 0, IOVT_BUFFER, sizeof(shub_req_t)
	},
	{NULL, 0, 0, 0, 0, 0}
};

static void wl_shub_process_scan_result(void *ctx, scan_utl_scan_data_t * sd);
static void _shub_process_scan_result_cb(wl_shub_info_t *ctx, void *bi, uint8 done);
static void _shub_cb(wl_shub_info_t * shp, shub_pkt_t *buf, int len);
static int wl_shub_send_pkt(wl_shub_info_t * shp, uint16 req, uint16 intr,
	uint16 option, uint8 *data, int len);
static int wl_shub_if_init(wl_shub_info_t *shp);
static int wl_shub_if_deinit(wl_shub_info_t *shp);

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

bool
wl_shub_scan_in_progress(wl_shub_info_t *shp)
{
	if (shp->cmn->scan_in_progress) {
		return TRUE;
	}
	return FALSE;
}

static void
wl_shub_process_scan_result(void *ctx, scan_utl_scan_data_t *sd)
{
	wl_shub_info_t *shp = (wl_shub_info_t *)(ctx);
	wlc_bss_info_t * bi = sd->bi;

	if (shp->cmn->scan_in_progress) {
		_shub_process_scan_result_cb(shp, bi, FALSE);
	}
}

static int
shub_doiovar(void *shp_handle, uint32 actionid, void *params, uint p_len,
	void *arg, uint len, uint val_size, struct wlc_if *wlcif)
{
	wl_shub_info_t *shp = shp_handle;
	int32	*shub_st;
	int err = BCME_UNSUPPORTED;

	ASSERT(shp);
	ASSERT(shp->wlc && shp->wlc->pub);

	switch (actionid) {
	case IOV_SVAL(IOV_SHUB):
	{
		err = BCME_UNSUPPORTED;
		break;
	}
	case IOV_GVAL(IOV_SHUB):
	{
		shub_st = (int32 *)arg;
		*shub_st = !(!shp->cmn->shifp);
		err = BCME_OK;
		break;
	}
	case IOV_SVAL(IOV_SHUB_REQ):
	{
		if (SHIF_ENAB(shp->wlc->pub)) {
			shub_req_t shub_req;

			memcpy(&shub_req, arg, sizeof(shub_req));

			wl_shub_send_pkt(shp, shub_req.request,
				shub_req.interval, 0,
				(uint8*)&shub_req.enable, (int)sizeof(shub_req.enable));
				err = BCME_OK;
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	}

	default:
		WL_SHUB_ERR(("%s: IOVAR not supported \n", __FUNCTION__));
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static void
wl_shub_timer(void *arg)
{
	wl_shub_info_t *shp = arg;
	int ap_count;
	uint8 *result_buf;
	int ret;

	if (shp->cmn->sh_scan_result && shp->cmn->ap_count) {
		if (shp->cmn->ap_count > SHUB_MAX_AP_PER_RESP) {
			ap_count = SHUB_MAX_AP_PER_RESP;
		} else {
			ap_count = shp->cmn->ap_count;
		}

		result_buf = shp->cmn->sh_scan_result +
			(shp->cmn->aps_reported * SHUB_AP_INFO_SIZE);

		ret = wl_shub_send_pkt(shp, SHUB_SCAN_DATA_IND, 0, 0,
			result_buf, (ap_count * SHUB_AP_INFO_SIZE));
		if (!ret) {
			/* pkt tx successful */
			shp->cmn->ap_count -= ap_count;
			shp->cmn->aps_reported += ap_count;
		}
		if (ret) {
			WL_SHUB_ERR(("%s CHECK!!! previous pkt in progress! %d\n",
				__FUNCTION__, __LINE__));
		}

		wl_add_timer(shp->wlc->wl, shp->cmn->timer, 0, FALSE);
	}
	if (!shp->cmn->ap_count) {
		wl_del_timer(shp->wlc->wl, shp->cmn->timer);
		WL_SHUB_ERR(("%s free result buffer \n", __FUNCTION__));
		if (shp->cmn->sh_scan_result) {
			MFREE(shp->wlc->osh, shp->cmn->sh_scan_result,
				(SHUB_MAX_AP_PER_RESP * SHUB_AP_INFO_SIZE));
			shp->cmn->sh_scan_result = NULL;
		}
		shp->cmn->aps_reported = 0;
		ret = wl_shub_send_pkt(shp, SHUB_SCAN_STOP, 0, 0,
			NULL, 0);
		if (ret) {
			WL_SHUB_ERR(("%s CHECK!!! previous pkt in progress! %d\n",
				__FUNCTION__, __LINE__));
			/* pkt tx failed */
			wl_add_timer(shp->wlc->wl, shp->cmn->timer, 0, FALSE);
		}
	}
}

static int
wl_shub_up_fn(void *shp_handle)
{
	wl_shub_info_t *shp = shp_handle;
	ASSERT(shp);

	if (shp) {
		rte_shif_up_fn(shp->cmn->shifp);
	}

	return BCME_OK;
}

static int
BCMATTACHFN(wl_shub_if_init)(wl_shub_info_t *shp)
{
	wlc_info_t *wlc;

	ASSERT(shp);
	ASSERT(shp->wlc && shp->wlc->pub);

	if (shp->cmn->shifp) {
		WL_SHUB_INFO(("wl%d: Sensor Hub interface intialized \n",
			wlc->pub->unit));
		return BCME_OK;
	}

	wlc = shp->wlc;

	/* initialize Sensor Hub interface */
	shp->cmn->shifp = rte_shif_init(wlc->pub->sih, wlc->osh,
			shp->shub_wake_sh_pin, shp->shub_wake_int_pin);

	if (!shp->cmn->shifp) {
		WL_SHUB_ERR(("wl%d: Error initializing Sensor Hub interface \n",
			wlc->pub->unit));
		return BCME_ERROR;
	}

	if (rte_shif_config_rx_completion(shp->cmn->shifp, SHIF_DELIM_NOTDEFINED,
		sizeof(shub_pkt_t), SHIF_DEFAULT_TIMEOUT,
		(rx_cb_t)_shub_cb, (void *)shp) != BCME_OK) {
		WL_SHUB_ERR(("wl%d: Error registering Sensor hub Rx callback \n",
			wlc->pub->unit));
		return BCME_ERROR;
	}

	shp->cmn->sh_motion_scanfreq = SH_MOTION_SCANFREQ;
	shp->cmn->sh_idle_scanfreq = SH_IDLE_SCANFREQ;

	return BCME_OK;
}

static int
BCMATTACHFN(wl_shub_if_deinit)(wl_shub_info_t *shp)

{
	ASSERT(shp);

	if (shp->cmn->shifp) {
		rte_shif_deinit(shp->cmn->shifp);
	}

	shp->cmn->shifp = NULL;

	return BCME_OK;
}

wl_shub_info_t *
BCMATTACHFN(wl_shub_attach)(wlc_info_t * wlc)

{
	uint8 shwakepins;
	wl_shub_info_t *shp;

	if (!wlc) {
		return NULL;
	}

	/* Allocate sensor hub server module context */
	shp = (wl_shub_info_t *)MALLOCZ(wlc->osh, sizeof(wl_shub_info_t));

	if (!shp) {
		WL_SHUB_ERR(("%s: No memory for sesor hub data \n",
			__FUNCTION__));
		return NULL;
	}

	/* OBJ_REG check obj registry for common info */
	shp->cmn = obj_registry_get(wlc->objr, OBJR_SHUB_CMN_INFO);

	if (!shp->cmn) {
		shp->cmn = MALLOCZ(wlc->osh, sizeof(struct _shub_cmn_info_t));
		if (!shp->cmn) {
			WL_SHUB_ERR(("%s: No memory for sesor hub common data \n",
				__FUNCTION__));
			return NULL;
		}
		obj_registry_set(wlc->objr, OBJR_SHUB_CMN_INFO, shp->cmn);
	}

	shp->wlc = wlc;


	shp->wlc->pub->cmn->_shub = TRUE;

	/* obj registry */
	(void)obj_registry_ref(wlc->objr, OBJR_SHUB_CMN_INFO);

	shp->shub_wake_sh_pin = SHUB_INVALID_PIN;
	shp->shub_wake_int_pin = SHUB_INVALID_PIN;

	/* register the module */
	if (wlc_module_register(wlc->pub, shub_iovars, "shub", shp,
		shub_doiovar, NULL, wl_shub_up_fn, NULL)) {
		WL_SHUB_ERR(("%s: Error registering sensor hub server\n",
			__FUNCTION__));
		goto error;
	}

	if (getvar(NULL, "shwakepin") != NULL) {
		shwakepins = getintvar(NULL, "shwakepin");
		shp->shub_wake_int_pin = SHUB_GET_WAKE_WLAN_PIN(shwakepins);
		shp->shub_wake_sh_pin = SHUB_GET_WAKE_SH_PIN(shwakepins);


		/* initialize Sensor Hub interface */
		if (BCME_OK != wl_shub_if_init(shp)) {
			goto error;
		}

	} else if (shp->shub_wake_sh_pin == SHUB_INVALID_PIN ||
		shp->shub_wake_int_pin == SHUB_INVALID_PIN) {
		WL_SHUB_ERR(("%s: Error getting shwakepin\n", __FUNCTION__));
	}

	shp->cmn->timer = wl_init_timer(shp->wlc->wl, wl_shub_timer, shp, "shsvc_timer");
	if (!shp->cmn->timer) {
		WL_SHUB_ERR(("%s sensor hub timer init failed \n", __FUNCTION__));
		goto error;
	}

	wlc_scan_utils_rx_scan_register(shp->wlc, wl_shub_process_scan_result, shp);

	return shp;

error:
	wl_shub_detach(shp);
	return NULL;
}

void
BCMATTACHFN(wl_shub_detach)(wl_shub_info_t * shp)
{
	wlc_info_t *wlc;

	if (shp) {
		wlc = shp->wlc;
		/* free any additional resources */
	} else {
		WL_SHUB_ERR(("%s Sensor hub server is not initialized (shp is NULL) \n",
			__FUNCTION__));
		return;
	}

	if (shp->cmn == NULL) {
		WL_SHUB_ERR(("%s Sensor hub server is not initialized (shp->cmn is NULL) \n",
			__FUNCTION__));
		return;
	}

	if (shp->cmn->timer) {
		wl_free_timer(shp->wlc->wl, shp->cmn->timer);
		shp->cmn->timer = NULL;
	}

	if (shp->cmn && !obj_registry_unref(wlc->objr, OBJR_SHUB_CMN_INFO)) {
		wl_shub_if_deinit(shp);
		obj_registry_set(wlc->objr, OBJR_SHUB_CMN_INFO, NULL);
		MFREE(wlc->osh, shp->cmn, sizeof(struct _shub_cmn_info_t));
		shp->cmn = NULL;
	}
	wlc_scan_utils_rx_scan_unregister(shp->wlc, wl_shub_process_scan_result, shp);
	wlc_module_unregister(wlc->pub, "shub", shp);
	MFREE(wlc->osh, shp, sizeof(wl_shub_info_t));
	shp->wlc->pub->cmn->_shub = FALSE;
}

static void
_shub_scan_complete_cb(void *sh_info, int status, wlc_bsscfg_t *cfg)
{
	wl_shub_info_t *shp = (wl_shub_info_t *)sh_info;
	WL_SHUB_ERR(("%s status %d\n", __FUNCTION__, status));

	ASSERT(shp->cmn->scan_in_progress);

	_shub_process_scan_result_cb(shp, NULL, TRUE);
	shp->cmn->scan_in_progress = FALSE;
}

/* Callback from Scan module with bss info(data) and scan completion status(done) */
static void
_shub_process_scan_result_cb(wl_shub_info_t *ctx, void *data, uint8 done)
{
	wl_shub_info_t *shp = ctx;
	wlc_bss_info_t	*bi;
	uint8	*result_buf;
	uint16	offset;

	if (!shp) {
		WL_SHUB_ERR(("%s Unexpected: shub interface not ready \n", __FUNCTION__));
		return;
	}

	if (!done && shp->cmn->ap_count < SHUB_MAX_AP_PER_SCAN) {

		if (!data) {
			WL_SHUB_ERR(("%s no bss info \n", __FUNCTION__));
			return;
		}
		bi = data;

		/* AP info available; prepare result buffer */
		if (!shp->cmn->sh_scan_result) {
			result_buf = MALLOCZ(shp->wlc->osh, SHUB_AP_INFO_SIZE);
			if (!result_buf) {
				WL_SHUB_ERR(("%s no memory for result buffer \n", __FUNCTION__));
				return;
			}
			offset = 0;
		} else {
			offset = (shp->cmn->ap_count * SHUB_AP_INFO_SIZE);
			result_buf = MALLOCZ(shp->wlc->osh, offset + SHUB_AP_INFO_SIZE);
			if (!result_buf) {
				WL_SHUB_ERR(("%s no memory for result buffer \n", __FUNCTION__));
				return;
			}
			memcpy(result_buf, shp->cmn->sh_scan_result, offset);
			MFREE(shp->wlc->osh, shp->cmn->sh_scan_result, offset);
			shp->cmn->sh_scan_result = NULL;
		}

		/* populate rssi and mac address in result buf */
		memcpy((result_buf + offset), bi->BSSID.octet, ETHER_ADDR_LEN);
		result_buf[offset + RSSI_LSB_OFFSET] = (uint8)bi->RSSI;
		result_buf[offset + RSSI_MSB_OFFSET] = (uint8)(bi->RSSI >> 8);

		shp->cmn->sh_scan_result = result_buf;
		shp->cmn->ap_count++;
	}

	if (done) {
		/* scan complete; pass to sensor hub */
		uint8 *sh_scan_result = shp->cmn->sh_scan_result;
		int i, ix = 0;
		int16 rssi = 0;
		int ap_count;
		int	ret;

		if (!sh_scan_result) {
			WL_SHUB_ERR(("%s result buff null %d\n", __FUNCTION__, shp->cmn->ap_count));
			return;
		}

		if (shp->cmn->ap_count > SHUB_MAX_AP_PER_RESP) {
			ap_count = SHUB_MAX_AP_PER_RESP;
		} else {
			ap_count = shp->cmn->ap_count;
		}

		ret = wl_shub_send_pkt(shp, SHUB_SCAN_DATA_IND, 0, 0,
			sh_scan_result, (ap_count * SHUB_AP_INFO_SIZE));

		for (i = 0; i < shp->cmn->ap_count; i++) {
			ix = (i * SHUB_AP_INFO_SIZE);
			rssi = sh_scan_result[ix + RSSI_LSB_OFFSET];
			rssi |= (sh_scan_result[ix + RSSI_MSB_OFFSET] << 8);
			WL_SHUB_ERR(("%x:%x:%x:%x:%x:%x; %d\n",
				sh_scan_result[ix], sh_scan_result[ix + 1],
				sh_scan_result[ix + 2], sh_scan_result[ix + 3],
				sh_scan_result[ix + 4], sh_scan_result[ix + 5], rssi));
		}

		if (!ret) {
			/* pkt tx successful */
			WL_SHUB_ERR(("%s first result buffer \n", __FUNCTION__));
			shp->cmn->ap_count -= ap_count;
			shp->cmn->aps_reported += ap_count;
		} else {
			WL_SHUB_ERR(("%s CHECK!!! previous pkt in progress! %d\n",
				__FUNCTION__, __LINE__));
		}
		wl_add_timer(shp->wlc->wl, shp->cmn->timer, 0, FALSE);
	}
}

/* called by hwif when pkt from sensor hub is received */
static void
_shub_cb(wl_shub_info_t * shp, shub_pkt_t *shpkt, int len)
{
	wlc_info_t *wlc;
	bool cmd_pkt = FALSE;
	uint16 cmd;
	int32 a_scanfreq;
	uint8 req_cnf = 0;
	int	i;

	ASSERT(shp);
	wlc = shp->wlc;

	WL_SHUB_INFO(("%s %d\n", __FUNCTION__, len));
	WL_SHUB_ERR(("Rx:"));
	for (i = 0; i < len; i++) {
		WL_SHUB_ERR(("%x ", ((char *)shpkt)[i]));
	}
	WL_SHUB_ERR((" \n"));

	if (shpkt->start != SHUB_PKT_START) {
		return;
	}

	if ((shpkt->type & SHUB_PKT_TYPE_MASK) == SHUB_PKT_TYPE_COMMAND) {
		cmd_pkt = TRUE;
	}

	cmd = shpkt->type & SHUB_PKT_COMMAND_MASK;

	if (cmd_pkt) {
		if (cmd == SHUB_MOVE_STOP_CNF) {
			if (shpkt->data[0]) {
				shp->cmn->shub_mv_stp_enabled = TRUE;
			}
		} else if (cmd == SHUB_SCAN_REQ) {
			wlc_ssid_t ssid;
			int	ret = 0;

			if (shp->cmn->scan_in_progress == FALSE) {
				memset(&ssid, 0, sizeof(ssid));
				shp->cmn->scan_in_progress = TRUE;

				ret = wlc_scan_request_ex(wlc, DOT11_BSSTYPE_ANY, &ether_bcast,
					1, &ssid, DOT11_SCANTYPE_ACTIVE, -1, -1, -1, -1, NULL,
					0, 0, FALSE, _shub_scan_complete_cb, shp,
					WLC_ACTION_LPRIO_EXCRX,	WL_SCANFLAGS_OFFCHAN,
					NULL, NULL, NULL);

				WL_SHUB_ERR(("%s scan request return code %d\n",
					__FUNCTION__, ret));
			}
			else {
				WL_SHUB_ERR(("%s scan request was blocked\n", __FUNCTION__));
				ret = BCME_BUSY;
			}

			/* If scan is busy, request is failed and have to send scan cnf
			  * packet with data 1
			  * Oppositely scan req is succeeded, data 0 of scan cnf packet
			  */
			if (ret == BCME_BUSY) {
				req_cnf = SHUB_REQ_FAIL;
			} else {
				req_cnf = SHUB_REQ_OK;
			}

			ret = wl_shub_send_pkt(shp, SHUB_SCAN_CNF, 0, 0,
				&req_cnf, SHUB_SCAN_CNF_DATA_LEN);

			if (ret) {
				WL_SHUB_ERR(("%s CHECK!!! previous pkt in progress! %d\n",
					__FUNCTION__, __LINE__));
			}

		}
	} else {
		if (shp->cmn->shub_mv_stp_enabled &&
			cmd == SHUB_MOVE_STOP_IND) {

			if (shpkt->data[0]) {
				if (shp->cmn->sh_motion_scanfreq > 0) {
					a_scanfreq = shp->cmn->sh_motion_scanfreq;
				}
			} else {
				if (shp->cmn->sh_idle_scanfreq > 0) {
					a_scanfreq = shp->cmn->sh_idle_scanfreq;
				}
			}
			BCM_REFERENCE(a_scanfreq);
		}
	}

}

static int
wl_shub_send_pkt(wl_shub_info_t * sh, uint16 req, uint16 intr,
	uint16 option, uint8 *data, int len)
{
	wlc_info_t *wlc = sh->wlc;
	struct _shub_cmn_info_t	*shp = sh->cmn;
	shub_pkt_t *shpkt;
	int ret;

	if (len > SHUB_DATA_LEN) {
		return BCME_ERROR;
	}

	shpkt = MALLOCZ(wlc->osh, sizeof(shub_pkt_t));
	if (!shpkt) {
		WL_SHUB_ERR(("wl%d: shub pkt alloc failed, size = %d\n",
			wlc->pub->unit, sizeof(shub_pkt_t)));
		return BCME_ERROR;
	}

	/* Start field */
	shpkt->start = SHUB_PKT_START;

	/* set command type: 0x0xxx - data pkt; 0xFxxx - req(cmd) pkt
	  * 2015-09-21 adding scan data indication pkt req = 0x0003
	  */
	if (req == SHUB_SCAN_DATA_IND) {
		shpkt->type = SHUB_PKT_TYPE_DATA;
	} else {
		shpkt->type = SHUB_PKT_TYPE_COMMAND;
	}

	shpkt->type |= (req & SHUB_PKT_COMMAND_MASK);

	shpkt->interval = intr;

	shpkt->length = len;
	memcpy(shpkt->data, data, len);

	ret = rte_shif_send(shp->shifp, (char *)shpkt, sizeof(shub_pkt_t));
	if (ret != BCME_OK) {
		WL_SHUB_ERR(("wl%d: failed to transmit shub pkt error_code = %d\n",
			wlc->pub->unit, ret));
	}

	return ret;
}
