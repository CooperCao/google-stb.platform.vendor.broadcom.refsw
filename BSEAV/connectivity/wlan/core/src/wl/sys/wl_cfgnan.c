/*
 * Neighbor Awareness Networking
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

#ifdef WL_NAN
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmwifi_channels.h>
#include <proto/nan.h>
#include <bcmiov.h>
#include "bcmcrypto/sha256.h"

#include <wl_cfg80211.h>
#include <wl_android.h>
#include <wl_cfgnan.h>

#if defined(BCMDONGLEHOST)
#include <dngl_stats.h>
#include <dhd.h>
#endif
#include <wl_cfgvendor.h>
#ifdef WL_NAN_DEBUG
static u8 g_nan_debug = true;
#endif /* WL_NAN_DEBUG */

static nan_cmd_t nan_cmds [] = {
	{ "NAN_START", wl_cfgnan_start_handler },
	{ "NAN_STOP", wl_cfgnan_stop_handler },
	{ "NAN_SUPPORT", wl_cfgnan_support_handler },
	{ "NAN_STATUS", wl_cfgnan_status_handler },
	{ "NAN_PUBLISH", wl_cfgnan_pub_handler },
	{ "NAN_SUBSCRIBE", wl_cfgnan_sub_handler },
	{ "NAN_CANCEL_PUBLISH", wl_cfgnan_cancel_pub_handler },
	{ "NAN_CANCEL_SUBSCRIBE", wl_cfgnan_cancel_sub_handler },
	{ "NAN_TRANSMIT", wl_cfgnan_transmit_handler },
	{ "NAN_SET_CONFIG", wl_cfgnan_set_config_handler },
	{ "NAN_GET_CONFIG", NULL },
	{ "NAN_RTT_CONFIG", wl_cfgnan_rtt_config_handler },
	{ "NAN_RTT_FIND", wl_cfgnan_rtt_find_handler },
#ifdef WL_NAN_DEBUG
	{ "NAN_DEBUG", wl_cfgnan_debug_handler },
#endif /* WL_NAN_DEBUG */
#ifdef NAN_P2P_CONFIG
	{ "NAN_ADD_CONF", wl_cfgnan_p2p_ie_add_handler },
	{ "NAN_ENABLE_CONF", wl_cfgnan_p2p_ie_enable_handler },
	{ "NAN_DEL_CONF", wl_cfgnan_p2p_ie_del_handler },
#endif /* NAN_P2P_CONFIG */
	{ NULL, NULL },
};

#ifdef NOT_YET
static nan_config_attr_t nan_config_attrs [] = {
	{ "ATTR_MASTER", WL_NAN_XTLV_MASTER_PREF },
	{ "ATTR_ID", WL_NAN_XTLV_CLUSTER_ID },
	{ "ATTR_ADDR", WL_NAN_XTLV_IF_ADDR },
	{ "ATTR_ROLE", WL_NAN_XTLV_ROLE },
	{ "ATTR_BCN_INT", WL_NAN_XTLV_BCN_INTERVAL },
	{ "ATTR_CHAN", WL_NAN_XTLV_MAC_CHANSPEC },
	{ "ATTR_TX_RATE", WL_NAN_XTLV_MAC_TXRATE },
	{ "ATTR_DW_LEN", WL_NAN_XTLV_DW_LEN },
	{ {0}, 0 }
};
#endif

static const char *nan_event_to_str(u16 cmd)
{
	switch (cmd) {
	C2S(WL_NAN_EVENT_START)
	C2S(WL_NAN_EVENT_JOIN)
	C2S(WL_NAN_EVENT_ROLE)
	C2S(WL_NAN_EVENT_SCAN_COMPLETE)
	C2S(WL_NAN_EVENT_DISCOVERY_RESULT)
	C2S(WL_NAN_EVENT_REPLIED)
	C2S(WL_NAN_EVENT_TERMINATED)
	C2S(WL_NAN_EVENT_RECEIVE)
	C2S(WL_NAN_EVENT_STATUS_CHG)
	C2S(WL_NAN_EVENT_MERGE)
	C2S(WL_NAN_EVENT_STOP)
	C2S(WL_NAN_EVENT_P2P)
	C2S(WL_NAN_EVENT_WINDOW_BEGIN_P2P)
	C2S(WL_NAN_EVENT_WINDOW_BEGIN_MESH)
	C2S(WL_NAN_EVENT_WINDOW_BEGIN_IBSS)
	C2S(WL_NAN_EVENT_WINDOW_BEGIN_RANGING)
	C2S(WL_NAN_EVENT_POST_DISC)
	C2S(WL_NAN_EVENT_DATA_IF_ADD)
	C2S(WL_NAN_EVENT_DATA_PEER_ADD)
	C2S(WL_NAN_EVENT_PEER_DATAPATH_IND)
	C2S(WL_NAN_EVENT_DATAPATH_ESTB)
	C2S(WL_NAN_EVENT_SDF_RX)
	C2S(WL_NAN_EVENT_DATAPATH_END)
	C2S(WL_NAN_EVENT_BCN_RX)
	C2S(WL_NAN_EVENT_PEER_DATAPATH_RESP)
	C2S(WL_NAN_EVENT_PEER_DATAPATH_CONF)
	C2S(WL_NAN_EVENT_RNG_REQ_IND)
	C2S(WL_NAN_EVENT_RNG_RPT_IND)
	C2S(WL_NAN_EVENT_RNG_TERM_IND)
	C2S(WL_NAN_EVENT_INVALID)

	default:
		return "WL_NAN_EVENT_UNKNOWN";
	}
}

int
wl_cfgnan_generate_inst_id(struct bcm_cfg80211 *cfg,
		uint8 inst_type, uint8 *p_inst_id)
{
	uint8 inst_id = 0;
	uint8 i, j;
	nan_svc_inst_t *id_ctrl = NULL;
	uint16 idx = 0;
	s32 ret = BCME_OK;

	if (p_inst_id == NULL) {
		WL_ERR(("Invalid arguments\n"));
		ret = -EINVAL;
		goto exit;
	}

	if (inst_type < NAN_SVC_INST_PUBLISHER ||
			inst_type > NAN_SVC_INST_SUBSCRIBER) {
		WL_ERR(("Invalid instance type\n"));
		ret = -EINVAL;
		goto exit;
	}

	id_ctrl = cfg->nan_inst_ctrl;

	for (idx = 0; idx < NAN_MAXIMUM_ID_NUMBER; idx++) {
		i = idx / 8;
		j = idx % 8;
		if (id_ctrl[i].inst_id & (1 << j)) {
			continue;
		} else {
			id_ctrl[i].inst_id |= (1 << j);
			if (inst_type == NAN_SVC_INST_PUBLISHER) {
				id_ctrl[i].inst_type |= (1 << j);
			} else {
				id_ctrl[i].inst_type &= (~(1 << j));
			}
			inst_id = (i*8)+j+1;
			WL_DBG(("Instance ID=%d\n", inst_id));
			*p_inst_id = inst_id;
			goto exit;
		}
	}

	if (inst_id == 0) {
		WL_ERR(("Allocated maximum IDs\n"));
		ret = -ENOMEM;
		goto exit;
	}

exit:
	return ret;
}

int
wl_cfgnan_validate_inst_id(struct bcm_cfg80211 *cfg,
		uint8 inst_id)
{
	uint8 i, j, temp;
	s32 ret = BCME_OK;
	nan_svc_inst_t *id_ctrl = NULL;

	id_ctrl = cfg->nan_inst_ctrl;

	i = inst_id / 8;
	j = (inst_id % 8) - 1;
	temp = id_ctrl[i].inst_id;
	if (!(temp & (1 << j))) {
		WL_ERR(("Instance ID not present, ID = %d\n", inst_id));
		ret = -ENOKEY;
	}

	return ret;
}

int
wl_cfgnan_remove_inst_id(struct bcm_cfg80211 *cfg,
		uint8 inst_id)
{
	uint8 i, j;
	s32 ret = BCME_OK;
	nan_svc_inst_t *id_ctrl = NULL;

	id_ctrl = cfg->nan_inst_ctrl;
	if (wl_cfgnan_validate_inst_id(cfg, inst_id) != 0) {
		WL_ERR(("Unable to remove. Instance ID not present,"
					"ID = %d\n", inst_id));

		ret = -ENOKEY;
		goto exit;
	}
	i = inst_id / 8;
	j = (inst_id % 8) - 1;
	id_ctrl[i].inst_id &= ~(1 << j);

exit:
	return ret;
}

int
wl_cfgnan_get_inst_type(struct bcm_cfg80211 *cfg,
		uint8 inst_id, uint8 *inst_type)
{
	s32 ret = BCME_OK;
	uint8 i, j;
	nan_svc_inst_t *id_ctrl = NULL;

	id_ctrl = cfg->nan_inst_ctrl;
	if (wl_cfgnan_validate_inst_id(cfg, inst_id) != 0) {
		WL_ERR(("Unable to get intance type. Instance ID not present, "
					"ID = %d\n", inst_id));

		ret = -ENOKEY;
		goto exit;
	}

	i = inst_id / 8;
	j = (inst_id % 8) - 1;
	if (id_ctrl[i].inst_type & (1 << j)) {
		*inst_type = NAN_SVC_INST_PUBLISHER;
	} else {
		*inst_type = NAN_SVC_INST_SUBSCRIBER;
	}

exit:
	return ret;
}

/* Based on each case of tlv type id, fill into tlv data */
int
wl_cfgnan_set_vars_cbfn(void *ctx, const uint8 *data, uint16 type, uint16 len)
{
	wl_nan_tlv_data_t *tlv_data = ((wl_nan_tlv_data_t *)(ctx));
	int ret = BCME_OK;
	u16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;

	NAN_DBG_ENTER();

	switch (type) {
	case WL_NAN_XTLV_MAC_ADDR:
		memcpy(&tlv_data->mac_addr, data, ETHER_ADDR_LEN);
		WL_DBG(("%s: WL_NAN_XTLV_MAC_ADDR: " MACDBG "\n",
			__FUNCTION__, MAC2STRDBG(tlv_data->mac_addr.octet)));
		break;

	case WL_NAN_XTLV_SVC_INFO: {
		WL_DBG(("WL_NAN_XTLV_SVC_INFO (len=%d)\n", len));

		if (len > 0) {
			tlv_data->svc_info.dlen = len;
			tlv_data->svc_info.data = kzalloc(tlv_data->svc_info.dlen, kflags);
			if (!tlv_data->svc_info.data) {
				WL_ERR(("%s: memory allocation failed\n", __FUNCTION__));
				tlv_data->svc_info.dlen = 0;
				ret = -ENOMEM;
				goto fail;
			}
			memcpy(tlv_data->svc_info.data, data, tlv_data->svc_info.dlen);
		} else {
			tlv_data->svc_info.dlen = 0;
			WL_ERR(("%s: svc info length is invalid\n", __FUNCTION__));
			ret = -EINVAL;
			goto fail;
		}
		break;
	}

	case WL_NAN_XTLV_SDF_RX: {
		const uint8 *p_sdf = data;
		uint16 attr_len = 0;
		uint8 svc_control = 0;
		WL_DBG(("WL_NAN_XTLV_SDF_RX (len=%d)\n", len));

		if (!len) {
			tlv_data->svc_info.dlen = 0;
			WL_ERR(("data length is invalid\n"));
			ret = -BCME_ERROR;
			goto fail;
		}

		/*
		 * Mapping service discovery frame to extract sub fields:
		 * SDF followed by nan2_pub_act_frame_t and wifi_nan_svc_descriptor_attr_t,
		 * and svc info is optional.
		 */

		/* nan2_pub_act_frame_t */
		WL_DBG(("> pub category: 0x%02x\n", *p_sdf));
		p_sdf++;
		WL_DBG(("> pub action: 0x%02x\n", *p_sdf));
		p_sdf++;
		WL_DBG(("> nan oui: %2x-%2x-%2x\n", *p_sdf, *(p_sdf+1), *(p_sdf+2)));
		p_sdf += 3;
		WL_DBG(("> oui subtype: 0x%02x\n", *p_sdf));
		p_sdf++;

		/* wifi_nan_svc_descriptor_attr_t */
		WL_DBG(("> attr id: 0x%02x\n", *p_sdf));
		p_sdf++;

		/* attr_len is uint16 */
		memcpy(&attr_len, p_sdf, 2);
		WL_DBG(("> attr len: 0x%02x\n", attr_len));
		p_sdf += 2;

		WL_DBG(("> svc_hash_name: " MACDBG "\n", MAC2STRDBG(p_sdf)));
		memcpy(tlv_data->svc_name, p_sdf, NAN_SVC_HASH_LEN);
		p_sdf += 6;

		tlv_data->inst_id = *p_sdf++;
		WL_DBG(("> instance id: 0x%02x\n", tlv_data->inst_id));

		tlv_data->peer_inst_id = *p_sdf++;
		WL_DBG(("> requestor id: 0x%02x\n", tlv_data->peer_inst_id));

		/* svc info present bitmask: 0x10 */
		svc_control = *p_sdf & 0x10;
		if (svc_control) {
			WL_DBG(("> svc_control: svc info present (0x%02x)\n", *p_sdf));
			p_sdf++;

			tlv_data->svc_info.dlen = *p_sdf++;
			WL_DBG(("> svc_info len: 0x%02x\n", tlv_data->svc_info.dlen));

			if (tlv_data->svc_info.dlen > 0) {
				tlv_data->svc_info.data = kzalloc(tlv_data->svc_info.dlen, kflags);
				if (!tlv_data->svc_info.data) {
					WL_ERR(("%s: memory allocation failed\n", __FUNCTION__));
					tlv_data->svc_info.dlen = 0;
					ret = -ENOMEM;
					goto fail;
				}
				memcpy(tlv_data->svc_info.data, p_sdf, tlv_data->svc_info.dlen);
			} else {
				tlv_data->svc_info.dlen = 0;
				WL_ERR(("%s: svc info length is invalid\n", __FUNCTION__));
				ret = -EINVAL;
				goto fail;
			}
		}

		break;
	}

	case WL_NAN_XTLV_SDE_CONTROL:
		WL_ERR(("WL_NAN_XTLV_SDE_CONTROL (len=%d), p_data=%p\n", len, data));
		break;

	case WL_NAN_XTLV_SDE_RANGE_LIMIT:
		WL_ERR(("WL_NAN_XTLV_SDE_RANGE_LIMIT (len=%d), p_data=%p\n", len, data));
		break;

	default:
		WL_ERR(("Not available for tlv type = 0x%x\n", type));
		ret = BCME_ERROR;
		break;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfg_nan_check_cmd_len(uint16 nan_iov_len, uint16 data_size,
		uint16 *subcmd_len)
{
	s32 ret = BCME_OK;

	if (subcmd_len != NULL) {
		*subcmd_len = bcm_xtlv_size_for_data(
				OFFSETOF(bcm_iov_batch_subcmd_t, data) + data_size, 4);
		if (*subcmd_len > nan_iov_len) {
			WL_ERR(("%s: Buf short, requested:%d, available:%d\n",
					__FUNCTION__, *subcmd_len, nan_iov_len));
			ret = BCME_NOMEM;
		}
	} else {
		WL_ERR(("Invalid subcmd_len\n"));
		ret = BCME_ERROR;
	}
	return ret;
}

int
wl_cfgnan_enable_events(struct net_device *ndev, struct bcm_cfg80211 *cfg,
		wl_nan_iov_t *nan_iov_data)
{
	s32 ret = BCME_OK;
	uint8 *p_buf = (uint8*)nan_iov_data->nan_iov_buf;
	bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);
	uint32 event_mask = 0;
	uint16 subcmd_len;

	NAN_DBG_ENTER();

	if (!p_buf) {
		WL_ERR(("%s: event data is NULL\n", __FUNCTION__));
		ret = BCME_ERROR;
		goto fail;
	}

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(event_mask), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR((" nan_sub_cmd check failed\n"));
		goto fail;
	}

	ret = wl_add_remove_eventmsg(ndev, WLC_E_NAN, true);
	if (unlikely(ret)) {
		WL_ERR((" nan event enable failed, error = %d \n", ret));
		goto fail;
	}

#ifdef WL_NAN_DEBUG
	if (g_nan_debug) {
		/* enable all nan events */
		event_mask = NAN_EVENT_MASK_ALL;
	} else
#endif /* WL_NAN_DEBUG */
	{
		/* enable only selected nan events to avoid unnecessary host wake up */
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_START);
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_JOIN);
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_DISCOVERY_RESULT);
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_REPLIED);
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_RECEIVE);
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_TERMINATED);
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_STOP);
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_CLEAR_BIT);
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_BCN_RX);
#ifdef NAN_DP
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_PEER_DATAPATH_IND);
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_PEER_DATAPATH_RESP);
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_PEER_DATAPATH_CONF);
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_DATAPATH_ESTB);
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_DATAPATH_END);
#endif /* NAN_DP */
#ifdef NAN_P2P_CONFIG
		event_mask |= NAN_EVENT_BIT(WL_NAN_EVENT_P2P);
#endif /* NAN_P2P_CONFIG */
		event_mask = htod32(event_mask);
	}

	sub_cmd->id = htod16(WL_NAN_CMD_CFG_EVENT_MASK);
	sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(event_mask);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
	memcpy(sub_cmd->data, &event_mask, sizeof(event_mask));

	/* Reduce the iov_len size by subcmd_len */
	nan_iov_data->nan_iov_len -= subcmd_len;
	/* Point forward the nan_iov_buf by subcmd_len */
	nan_iov_data->nan_iov_buf += subcmd_len;

	ret = wl_add_remove_eventmsg(ndev, WLC_E_PROXD, true);
	if (unlikely(ret)) {
		WL_ERR((" proxd event enable failed, error = %d \n", ret));
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

#ifdef NAN_DP
static int
wl_cfgnan_set_nan_ndp_autoconnect(struct net_device *ndev,
                struct bcm_cfg80211 *cfg, char *cmd, nan_cmd_data_t *cmd_data)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	s32 ret = BCME_OK;
	uint16 nan_iov_start, nan_iov_end;
	uint16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	uint16 nan_buf_size = sizeof(*nan_buf) + NAN_IOCTL_BUF_SIZE;
	uint16 subcmd_len;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	uint8 autoconn = 0;
	wl_nan_iov_t *nan_iov_data = NULL;

	NAN_DBG_ENTER();

	nan_buf = kzalloc(nan_buf_size, kflags);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = kzalloc(sizeof(*nan_iov_data), kflags);
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data->nan_iov_len = nan_iov_start = NAN_IOCTL_BUF_SIZE;
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);
	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(autoconn), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		goto fail;
	}

	autoconn = 1;
	sub_cmd->id = htod16(WL_NAN_CMD_DATA_AUTOCONN);
	sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(autoconn);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
	nan_buf->count++;
	memcpy(sub_cmd->data, &autoconn, sizeof(autoconn));

	/* Reduce the iov_len size by subcmd_len */
	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_end = nan_iov_data->nan_iov_len;
	nan_buf_size = (nan_iov_start - nan_iov_end);

	if (nan_buf != NULL) {
		ret = wldev_iovar_setbuf(ndev, "nan", nan_buf, nan_buf_size,
				cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
		if (unlikely(ret)) {
			WL_ERR(("set nan dp autoconnect failed, error = %d\n", ret));
			goto fail;
		}
		WL_DBG(("set nan dp autoconnect successfull\n"));
	} else {
		WL_ERR(("Invalid nan_buf\n"));
		goto fail;
	}

fail:
	if (unlikely(ret)) {
		WL_ERR(("nan dp autoconnect handler failed ... Cleaning Up\n"));
	}

	if (nan_buf) {
		kfree(nan_buf);
	}
	if (nan_iov_data) {
		kfree(nan_iov_data);
	}

	NAN_DBG_EXIT();
	return ret;
}
#endif /* NAN_DP */

static int
wl_cfgnan_enable_handler(nan_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data)
{
	/* nan enable */
	s32 ret = BCME_OK;
	uint8 *p_buf = (uint8 *)nan_iov_data->nan_iov_buf;

	NAN_DBG_ENTER();

	if (p_buf != NULL) {
		bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);
		uint8 val = 0;
		uint16 subcmd_len;

		ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
				sizeof(val), &subcmd_len);
		if (unlikely(ret)) {
			WL_ERR(("nan_sub_cmd check failed\n"));
			goto fail;
		}

		/* Fill the sub_command block */
		val = 1;
		sub_cmd->id = htod16(WL_NAN_CMD_CFG_ENABLE);
		sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(val);
		sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
		memcpy(sub_cmd->data, &val, sizeof(val));
		/* Reduce the iov_len size by subcmd_len */
		nan_iov_data->nan_iov_len -= subcmd_len;
		/* Point forward the nan_iov_buf by subcmd_len */
		nan_iov_data->nan_iov_buf += subcmd_len;
	} else {
		WL_ERR(("nan_iov_buf is NULL\n"));
		ret = BCME_ERROR;
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_warmup_time_handler(nan_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data)
{
	/* wl nan warm_up_time */
	s32 ret = BCME_OK;
	uint8 *p_buf = (uint8 *)nan_iov_data->nan_iov_buf;

	NAN_DBG_ENTER();

	if (p_buf != NULL) {
		bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);
		wl_nan_warmup_time_ticks_t *wup_ticks = (uint32*)sub_cmd->data;
		uint16 subcmd_len;

		ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
				sizeof(*p_buf), &subcmd_len);
		if (unlikely(ret)) {
			WL_ERR(("nan_sub_cmd check failed\n"));
			goto fail;
		}

		/* Fill the sub_command block */
		sub_cmd->id = htod16(WL_NAN_CMD_CFG_WARMUP_TIME);
		sub_cmd->len = sizeof(sub_cmd->u.options) +
			sizeof(*wup_ticks);
		sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
		*wup_ticks = cmd_data->warmup_time;
		/* Reduce the iov_len size by subcmd_len */
		nan_iov_data->nan_iov_len -= subcmd_len;
		/* Point forward the nan_iov_buf by subcmd_len */
		nan_iov_data->nan_iov_buf += subcmd_len;
	} else {
		WL_ERR(("nan_iov_buf is NULL\n"));
		ret = BCME_ERROR;
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_master_preference(nan_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data)
{
	s32 ret = BCME_OK;
	uint8 *p_buf = (uint8 *)nan_iov_data->nan_iov_buf;

	NAN_DBG_ENTER();

	if (p_buf != NULL) {
		bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);
		wl_nan_election_metric_config_t *metrics = NULL;
		uint16 subcmd_len;

		ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
				sizeof(*p_buf), &subcmd_len);
		if (unlikely(ret)) {
			WL_ERR(("nan_sub_cmd check failed\n"));
			goto fail;
		}

		metrics = (wl_nan_election_metric_config_t *)sub_cmd->data;

		if (!cmd_data->random_factor) {
			/* TODO: Need to decide on Random Factor */
			metrics->random_factor =  RANDOM32();
		} else {
			metrics->random_factor = (uint8)cmd_data->random_factor;
		}

		/* Generate Random MasteR Preference */
		if (!cmd_data->master_pref) {
			metrics->master_pref = RANDOM32();
			/* Floor the random master prefernce to 2 to 254 (both inclusive) */
			metrics->master_pref =
				(metrics->master_pref % (NAN_MAXIMUM_MASTER_PREFERENCE - 2)) + 2;
		} else {
			/* master preference must be from 0 to 254 */
			if (cmd_data->master_pref <= NAN_ID_MIN ||
					cmd_data->master_pref > (NAN_ID_MAX - 1)) {
				WL_ERR(("Invalid master_pref value\n"));
				ret = BCME_BADARG;
				goto fail;
			}
			metrics->master_pref = (uint8)cmd_data->master_pref;
		}

		sub_cmd->id = htod16(WL_NAN_CMD_ELECTION_METRICS_CONFIG);
		sub_cmd->len = sizeof(sub_cmd->u.options) +
			sizeof(*metrics);
		sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
		nan_iov_data->nan_iov_len -= subcmd_len;
		/* Point forward the nan_iov_buf by subcmd_len */
		nan_iov_data->nan_iov_buf += subcmd_len;
	} else {
		WL_ERR(("nan_iov_buf is NULL\n"));
		ret = BCME_ERROR;
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_nan_band(nan_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data)
{
	s32 ret = BCME_OK;
	uint8 *p_buf = (uint8*)nan_iov_data->nan_iov_buf;

	NAN_DBG_ENTER();

	if (p_buf != NULL) {
		bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);
		uint8 *band = (uint8*)sub_cmd->data;
		uint16 subcmd_len;

		ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
				sizeof(*band), &subcmd_len);
		if (unlikely(ret)) {
			WL_ERR(("nan_sub_cmd check failed\n"));
			goto fail;
		}

		/* nan band can take values 0, 1 and 2 */
		if (cmd_data->nan_band <= NAN_BAND_AUTO) {
			sub_cmd->id = htod16(WL_NAN_CMD_CFG_BAND);
			sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(*band);
			sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
			*band = cmd_data->nan_band;
			nan_iov_data->nan_iov_len -= subcmd_len;
			/* Point forward the nan_iov_buf by subcmd_len */
			nan_iov_data->nan_iov_buf += subcmd_len;
		} else {
			WL_ERR(("NAN_BAND_INVALID\n"));
			ret = BCME_ERROR;
			goto fail;
		}
	} else {
		WL_ERR(("nan_iov_buf is NULL\n"));
		ret = BCME_ERROR;
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_cluster_id(nan_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data)
{
	s32 ret = BCME_OK;
	uint8 *p_buf = (uint8*)nan_iov_data->nan_iov_buf;

	NAN_DBG_ENTER();

	if (p_buf != NULL) {
		bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);
		uint16 subcmd_len;

		ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
				(sizeof(cmd_data->clus_id) - sizeof(*p_buf)), &subcmd_len);
		if (unlikely(ret)) {
			WL_ERR(("nan_sub_cmd check failed\n"));
			goto fail;
		}

		cmd_data->clus_id.octet[0] = 0x50;
		cmd_data->clus_id.octet[1] = 0x6F;
		cmd_data->clus_id.octet[2] = 0x9A;
		cmd_data->clus_id.octet[3] = 0x01;
		WL_DBG(("cluster_id = " MACDBG "\n", MAC2STRDBG(cmd_data->clus_id.octet)));
		sub_cmd->id = htod16(WL_NAN_CMD_CFG_CID);
		sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(cmd_data->clus_id);
		sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
		memcpy(sub_cmd->data, (uint8 *)&cmd_data->clus_id,
				sizeof(cmd_data->clus_id));
		nan_iov_data->nan_iov_len -= subcmd_len;
		nan_iov_data->nan_iov_buf += subcmd_len;
	} else {
		WL_ERR(("nan_iov_buf is NULL\n"));
		ret = BCME_ERROR;
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_hop_count_limit(nan_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data)
{
	s32 ret = BCME_OK;
	uint8 *p_buf = (uint8*)nan_iov_data->nan_iov_buf;

	NAN_DBG_ENTER();

	if (p_buf != NULL) {
		bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);
		wl_nan_hop_count_t *hop_limit = (uint8*)sub_cmd->data;
		uint16 subcmd_len;

		ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
				sizeof(*hop_limit), &subcmd_len);
		if (unlikely(ret)) {
			WL_ERR(("nan_sub_cmd check failed\n"));
			goto fail;
		}
		*hop_limit = cmd_data->hop_count_limit;
		sub_cmd->id = htod16(WL_NAN_CMD_CFG_HOP_LIMIT);
		sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(*hop_limit);
		sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
		nan_iov_data->nan_iov_len -= subcmd_len;
		/* Point forward the nan_iov_buf by subcmd_len */
		nan_iov_data->nan_iov_buf += subcmd_len;
	} else {
		WL_ERR(("nan_iov_buf is NULL\n"));
		ret = BCME_ERROR;
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_sid_beacon_val(nan_cmd_data_t *cmd_data,
        wl_nan_iov_t *nan_iov_data)
{
	s32 ret = BCME_OK;
	uint8 *p_buf = (uint8*)nan_iov_data->nan_iov_buf;

	NAN_DBG_ENTER();

	if (p_buf != NULL) {
		bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);
		wl_nan_sid_beacon_control_t *sid_beacon = NULL;
		uint16 subcmd_len;

		ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
				sizeof(*p_buf), &subcmd_len);
		if (unlikely(ret)) {
			WL_ERR(("nan_sub_cmd check failed\n"));
			goto fail;
		}

		sid_beacon = (wl_nan_sid_beacon_control_t *)sub_cmd->data;
		sid_beacon->sid_enable = cmd_data->sid_beacon.sid_enable;
		sid_beacon->sid_count = cmd_data->sid_beacon.sid_count;
		sub_cmd->id = htod16(WL_NAN_CMD_CFG_SID_BEACON);
		sub_cmd->len = sizeof(sub_cmd->u.options) +
			sizeof(*sid_beacon);
		sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
		nan_iov_data->nan_iov_len -= subcmd_len;
		nan_iov_data->nan_iov_buf += subcmd_len;
	} else {
		WL_ERR(("nan_iov_buf is NULL\n"));
		ret = BCME_ERROR;
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_nan_oui(nan_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data)
{
	s32 ret = BCME_OK;
	uint8 *p_buf = (uint8*)nan_iov_data->nan_iov_buf;

	NAN_DBG_ENTER();

	if (p_buf != NULL) {
		bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);
		uint16 subcmd_len;

		ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
				sizeof(*p_buf), &subcmd_len);
		if (unlikely(ret)) {
			WL_ERR(("nan_sub_cmd check failed\n"));
			goto fail;
		}
		sub_cmd->id = htod16(WL_NAN_CMD_CFG_OUI);
		sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(cmd_data->nan_oui);
		sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
		memcpy(sub_cmd->data, (uint32 *)&cmd_data->nan_oui,
			sizeof(cmd_data->nan_oui));

		nan_iov_data->nan_iov_len -= subcmd_len;
		nan_iov_data->nan_iov_buf += subcmd_len;
	} else {
		WL_ERR(("nan_iov_buf is NULL\n"));
		ret = BCME_ERROR;
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_nan_join(nan_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data)
{
	s32 ret = BCME_OK;
	uint8 *p_buf = (uint8*)nan_iov_data->nan_iov_buf;

	NAN_DBG_ENTER();

	if (p_buf != NULL) {
		bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);
		uint16 subcmd_len;
		wl_nan_join_t *join = (wl_nan_join_t*)sub_cmd->data;

		ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
				sizeof(*join), &subcmd_len);
		if (unlikely(ret)) {
			WL_ERR(("nan_sub_cmd check failed\n"));
			goto fail;
		}
		sub_cmd->id = htod16(WL_NAN_CMD_ELECTION_JOIN);
		sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(*join);
		sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
		join->start_cluster = 1;

		/*
		 * A cluster_low value matching cluster_high indicates a request
		 * to join a cluster with that value.
		 * If the requested cluster is not found the
		 * device will start its own cluster
		 */
		if ((cmd_data->clus_id.octet[4] == cmd_data->clus_id.octet[5]) &&
				(cmd_data->clus_id.octet[4] || cmd_data->clus_id.octet[5] != 0)) {
			cmd_data->clus_id.octet[0] = 0x50;
			cmd_data->clus_id.octet[1] = 0x6F;
			cmd_data->clus_id.octet[2] = 0x9A;
			cmd_data->clus_id.octet[3] = 0x01;
			memcpy(&join->cluster_id, &cmd_data->clus_id, ETHER_ADDR_LEN);
			WL_DBG(("start with own cluster_id=" MACDBG "\n",
				MAC2STRDBG(cmd_data->clus_id.octet)));
		}

		nan_iov_data->nan_iov_len -= subcmd_len;
		/* Point forward the nan_iov_buf by subcmd_len */
		nan_iov_data->nan_iov_buf += subcmd_len;
	} else {
		WL_ERR(("nan_iov_buf is NULL\n"));
		ret = BCME_ERROR;
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_disable_handler(wl_nan_iov_t *nan_iov_data)
{
	s32 ret = BCME_OK;
	uint8 *p_buf = (uint8*)nan_iov_data->nan_iov_buf;

	NAN_DBG_ENTER();

	if (p_buf != NULL) {
		bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);
		uint16 subcmd_len;
		uint8 val = 0;

		ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
				sizeof(val), &subcmd_len);
		if (unlikely(ret)) {
			WL_ERR(("nan_sub_cmd check failed\n"));
			goto fail;
		}

		sub_cmd->id = htod16(WL_NAN_CMD_CFG_ENABLE);
		sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(val);
		sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
		memcpy(sub_cmd->data, &val, sizeof(val));
		nan_iov_data->nan_iov_len -= subcmd_len;
		nan_iov_data->nan_iov_buf += subcmd_len;
	} else {
		WL_ERR(("nan_iov_buf is NULL\n"));
		ret = BCME_ERROR;
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfgnan_start_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	s32 ret = BCME_OK;
	uint16 nan_iov_start, nan_iov_end;
	uint16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	uint16 nan_buf_size = sizeof(*nan_buf) + NAN_IOCTL_BUF_SIZE;
	wl_nan_iov_t *nan_iov_data = NULL;

	NAN_DBG_ENTER();

	nan_buf = kzalloc(nan_buf_size, kflags);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = kzalloc(sizeof(*nan_iov_data), kflags);
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data->nan_iov_len = nan_iov_start = NAN_IOCTL_BUF_SIZE;
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

	if (cfg->nan_enable != true) {
		WL_DBG(("enable nan\n"));
		ret = wl_cfgnan_enable_handler(cmd_data, nan_iov_data);
		if (unlikely(ret)) {
			WL_ERR(("enable handler sub_cmd set failed\n"));
			goto fail;
		}
		nan_buf->count++;

		/* Setting warm up time */
		if (cmd_data->warmup_time) {
			ret = wl_cfgnan_warmup_time_handler(cmd_data, nan_iov_data);
			if (unlikely(ret)) {
				WL_ERR(("warm up time handler sub_cmd set failed\n"));
				goto fail;
			}
			nan_buf->count++;
		}

		/* setting master preference */
		if (cmd_data->master_pref) {
			ret = wl_cfgnan_set_master_preference(cmd_data, nan_iov_data);
			if (unlikely(ret)) {
				WL_ERR(("master_preference sub_cmd set failed\n"));
				goto fail;
			}
			nan_buf->count++;
		}

		/* setting nan band  */
		if (cmd_data->nan_band != NAN_BAND_INVALID) {
			ret = wl_cfgnan_set_nan_band(cmd_data, nan_iov_data);
			if (unlikely(ret)) {
				WL_ERR(("nan band sub_cmd set failed\n"));
				goto fail;
			}
			nan_buf->count++;
		}

		/*
		 * A cluster_low value matching cluster_high indicates a request
		 * to join a cluster with that value.
		 * If the requested cluster is not found the
		 * device will start its own cluster
		 */
		if ((!ETHER_ISNULLADDR(&cmd_data->clus_id.octet)) &&
				((cmd_data->clus_id.octet[4] != cmd_data->clus_id.octet[5]))) {
			/* setting cluster ID */
			ret = wl_cfgnan_set_cluster_id(cmd_data, nan_iov_data);
			if (unlikely(ret)) {
				WL_ERR(("cluster_id sub_cmd set failed\n"));
				goto fail;
			}
			nan_buf->count++;
		}

		/* setting hop count limit or threshold */
		if (cmd_data->hop_count_limit) {
			ret = wl_cfgnan_set_hop_count_limit(cmd_data, nan_iov_data);
			if (unlikely(ret)) {
				WL_ERR(("hop_count_limit sub_cmd set failed\n"));
				goto fail;
			}
			nan_buf->count++;
		}

		/* setting sid beacon val */
		if (cmd_data->sid_beacon.sid_enable != NAN_SID_ENABLE_FLAG_INVALID) {
			ret = wl_cfgnan_set_sid_beacon_val(cmd_data, nan_iov_data);
			if (unlikely(ret)) {
				WL_ERR(("sid_beacon sub_cmd set failed\n"));
				goto fail;
			}
			nan_buf->count++;
		}

		/* setting nan oui */
		if (cmd_data->nan_oui != 0) {
			ret = wl_cfgnan_set_nan_oui(cmd_data, nan_iov_data);
			if (unlikely(ret)) {
				WL_ERR(("nan_oui sub_cmd set failed\n"));
				goto fail;
			}
			nan_buf->count++;
		}

		/* setting nan join sub_cmd */
		ret = wl_cfgnan_set_nan_join(cmd_data, nan_iov_data);
		if (unlikely(ret)) {
			WL_ERR(("nan join failed\n"));
			goto fail;
		}
		cfg->nan_running = true;
		nan_buf->count++;

		ret = wl_cfgnan_enable_events(ndev, cfg, nan_iov_data);
		if (unlikely(ret)) {
			goto fail;
		}

		nan_buf->count++;
		nan_iov_end = nan_iov_data->nan_iov_len;
		nan_buf_size = (nan_iov_start - nan_iov_end);
		if (nan_buf != NULL) {
			ret = wldev_iovar_setbuf(ndev, "nan", nan_buf, nan_buf_size,
					cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
			if (unlikely(ret)) {
				WL_ERR((" nan enable and join failed, error = %d \n", ret));
				goto fail;
			}
			cfg->nan_enable = true;
			WL_DBG((" nan enable successfull \n"));
#ifdef NAN_DP
			/* wl nan dp autoconn 1 */
			ret = wl_cfgnan_set_nan_ndp_autoconnect(ndev, cfg, cmd, cmd_data);
			if (unlikely(ret)) {
				WL_ERR(("Failed to set autoconn value\n"));
				goto fail;
			}
#endif /* NAN_DP */
		} else {
			WL_ERR(("Invalid nan_buf\n"));
			goto fail;
		}
	} else {
		cfg->nan_enable = true;
		WL_DBG(("nan is enable already\n"));
		goto fail;
	}

fail:
	if (unlikely(ret)) { /* Do Cleanup on Start Handler Failure */
		WL_ERR(("nan start handler failed ... cleaning up\n"));
	}
	if (nan_buf) {
		kfree(nan_buf);
	}
	if (nan_iov_data) {
		kfree(nan_iov_data);
	}

	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfgnan_stop_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	s32 ret = BCME_OK;
	uint16 nan_iov_start, nan_iov_end;
	uint16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	uint16 nan_buf_size = sizeof(*nan_buf) + NAN_IOCTL_BUF_SIZE;
	wl_nan_iov_t *nan_iov_data = NULL;

	NAN_DBG_ENTER();

	nan_buf = kzalloc(nan_buf_size, kflags);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = kzalloc(sizeof(*nan_iov_data), kflags);
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data->nan_iov_len = nan_iov_start = NAN_IOCTL_BUF_SIZE;
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

	if (cfg->nan_enable) {
		ret = wl_cfgnan_disable_handler(nan_iov_data);
		if (unlikely(ret)) {
			WL_ERR(("nan disable handler failed\n"));
			goto fail;
		}
		nan_buf->count++;
	}

	nan_iov_end = nan_iov_data->nan_iov_len;
	nan_buf_size = (nan_iov_start - nan_iov_end);
	if (nan_buf != NULL) {
		ret = wldev_iovar_setbuf(ndev, "nan", nan_buf, nan_buf_size,
				cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
		if (unlikely(ret)) {
			WL_ERR(("nan disable failed, error = %d\n", ret));
			goto fail;
		} else {
			cfg->nan_enable = false;
			cfg->nan_running = false;
			WL_DBG(("nan disable successful\n"));
		}
	} else {
		WL_ERR(("Invalid nan_buf\n"));
		goto fail;
	}

fail:
	if (nan_buf) {
		kfree(nan_buf);
	}

	if (nan_iov_data) {
		kfree(nan_iov_data);
	}

	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfgnan_support_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size,
	nan_cmd_data_t *cmd_data)
{
	/* TODO: */
	return BCME_OK;
}

int
wl_cfgnan_status_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size,
	nan_cmd_data_t *cmd_data)
{
	/* TODO: */
	return BCME_OK;
}
#ifdef NAN_P2P_CONFIG

static void
wl_p2p_nan_ioctl_make_header(wl_p2p_nan_ioc_t *p2p_nanioc, uint16 cmd_id, uint16 len)
{
	p2p_nanioc->version = htod16(WL_P2P_NAN_IOCTL_VERSION);
	p2p_nanioc->id = cmd_id;
	p2p_nanioc->len = htod16(len);
}

static int
wl_p2p_nan_do_get_ioctl(struct net_device *ndev, struct bcm_cfg80211 *cfg,
	wl_p2p_nan_ioc_t *p2p_nanioc, uint16 alloc_size)
{
	wl_p2p_nan_ioc_t *iocresp = NULL;
	int res;
	uint8 *val;

	/*  send getbuf p2p nan iovar */
	res = wldev_iovar_getbuf(ndev, "p2p_nan", p2p_nanioc, alloc_size,
		cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);

	if (res == BCME_OK) {
		iocresp = (wl_p2p_nan_ioc_t *)cfg->ioctl_buf;
		if (iocresp == NULL) {
			res = BCME_ERROR;
			return res;
		}
		switch (iocresp->id) {
			case WL_P2P_NAN_CMD_ENABLE:
				val = iocresp->data;
				WL_ERR(("wl p2p_nan status is %s\n",
					*val == 1? "Enabled":"Disabled"));
				break;
			case WL_P2P_NAN_CMD_CONFIG: {
				wl_p2p_nan_config_t *p_p2p_nan_cfg =
					(wl_p2p_nan_config_t *)iocresp->data;
				WL_ERR(("wl p2p nan ie len = %u\n", p_p2p_nan_cfg->ie_len));
				prhex("P2P IE", p_p2p_nan_cfg->ie, p_p2p_nan_cfg->ie_len);
			}
			break;
			default:
			WL_ERR(("Unknown command %d\n", iocresp->id));
			break;
		}
	}
	return res;
}

int
wl_cfgnan_p2p_ie_enable_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char* buf, int size, nan_cmd_data_t *cmd_data)
{
	int res = BCME_OK;
	wl_p2p_nan_ioc_t *p2p_nanioc;
	uint16 alloc_size = OFFSETOF(wl_p2p_nan_ioc_t, data) + P2P_NAN_IOC_BUFSZ;
	void *pdata = NULL;
	uint8 val;
	u16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	p2p_nanioc = kzalloc(alloc_size, kflags);
	if (p2p_nanioc == NULL) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		return BCME_NOMEM;
	}

	wl_p2p_nan_ioctl_make_header(p2p_nanioc, WL_P2P_NAN_CMD_ENABLE, sizeof(uint8));

	if (cmd_data->p2p_info.data == NULL) { /* get  */
		res = wl_p2p_nan_do_get_ioctl(ndev, cfg, p2p_nanioc, alloc_size);
	} else {	/* set */

		val =  (uint8) cmd_data->p2p_info.data[0];
		pdata = p2p_nanioc->data;
		memcpy(pdata, &val, sizeof(uint8));
		res = wldev_iovar_setbuf(ndev, "p2p_nan", p2p_nanioc,
			alloc_size, cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
	}

	kfree(p2p_nanioc);
	return res;
}

int wl_cfgnan_p2p_ie_add_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data)
{
	int res = BCME_OK;
	int ie_len, data_len;
	wl_p2p_nan_ioc_t *p2p_nanioc;
	uint16 alloc_size = OFFSETOF(wl_p2p_nan_ioc_t, data) + cmd_data->p2p_info.dlen;
	wl_p2p_nan_config_t *p_p2p_nan_cfg;
	u16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;

	p2p_nanioc = kzalloc(alloc_size, kflags);
	if (p2p_nanioc == NULL)
		return BCME_NOMEM;

	cmd_data->p2p_info.dlen /= 2;	/* Number of hex values will be half of ascii */
	wl_p2p_nan_ioctl_make_header(p2p_nanioc, WL_P2P_NAN_CMD_CONFIG, P2P_NAN_IOC_BUFSZ);

	if (cmd_data->p2p_info.data == NULL) { /* get */
		wl_p2p_nan_do_get_ioctl(ndev, cfg, p2p_nanioc, alloc_size);
	} else {
		ie_len = cmd_data->p2p_info.dlen;
		data_len = OFFSETOF(wl_p2p_nan_config_t, ie) + ie_len;

		p_p2p_nan_cfg = (wl_p2p_nan_config_t *)p2p_nanioc->data;
		p_p2p_nan_cfg->version = WL_P2P_NAN_CONFIG_VERSION;
		p_p2p_nan_cfg->len = data_len;
		p_p2p_nan_cfg->ie_len = ie_len;

	if (!wl_cfg80211_hex_str_to_bin
		(p_p2p_nan_cfg->ie, (int)p_p2p_nan_cfg->ie_len, (uchar*)cmd_data->p2p_info.data)) {
		res = BCME_BADARG;
		goto fail;
	}
	p2p_nanioc->len = htod16(data_len);

	res = wldev_iovar_setbuf(ndev, "p2p_nan", p2p_nanioc, alloc_size,
		cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
	}

fail:
	kfree(p2p_nanioc);
	return res;
}

int wl_cfgnan_p2p_ie_del_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data)
{
	int res = BCME_OK;
	wl_p2p_nan_ioc_t *p2p_nanioc;
	uint16 alloc_size = OFFSETOF(wl_p2p_nan_ioc_t, data);

	u16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	p2p_nanioc = kzalloc(alloc_size, kflags);
	if (p2p_nanioc == NULL) {
		WL_ERR((" Memory is not enough\n"));
		return BCME_NOMEM;
	}
	wl_p2p_nan_ioctl_make_header(p2p_nanioc, WL_P2P_NAN_CMD_DEL_CONFIG, 0);
	res = wldev_iovar_setbuf(ndev, "p2p_nan", p2p_nanioc, alloc_size,
		cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
	kfree(p2p_nanioc);
	return res;
}

#endif /* NAN_P2P_CONFIG */

static int
wl_cfgnan_sd_params_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, nan_cmd_data_t *cmd_data,
	wl_nan_iov_t *nan_iov_data, uint16 cmd_id)
{
	s32 ret = BCME_OK;
	uint8 *pxtlv, *srf = NULL;
	uint16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	uint16 subcmd_len;
	uint16 buflen, buflen_at_start;
	uint8 *p_buf = (uint8*)nan_iov_data->nan_iov_buf;
	bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);
	wl_nan_sd_params_t *sd_params = (wl_nan_sd_params_t *)sub_cmd->data;

	NAN_DBG_ENTER();

	if (!p_buf) {
		WL_ERR(("nan_iov_buf is NULL\n"));
		ret = BCME_ERROR;
		goto fail;
	}

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
		OFFSETOF(wl_nan_sd_params_t, optional[0]), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		goto fail;
	}

	if (cmd_data->period) {
		sd_params->period = cmd_data->period;
	} else {
		sd_params->period = 1;
	}

	if (cmd_data->ttl) {
		sd_params->ttl = cmd_data->ttl;
	} else {
		sd_params->ttl = WL_NAN_TTL_UNTIL_CANCEL;
	}

	sd_params->flags = 0;
	sd_params->flags = cmd_data->flags;

	if (cmd_id == WL_NAN_CMD_SD_PUBLISH) {
		sd_params->instance_id = cmd_data->pub_id;
	} else if (cmd_id == WL_NAN_CMD_SD_SUBSCRIBE) {
		sd_params->instance_id = cmd_data->sub_id;
	} else {
		ret = BCME_USAGE_ERROR;
		WL_ERR(("wrong command id = %d \n", cmd_id));
		goto fail;
	}

	if ((cmd_data->svc_hash.dlen == WL_NAN_SVC_HASH_LEN) && (cmd_data->svc_hash.data)) {
		memcpy((uint8*)sd_params->svc_hash, cmd_data->svc_hash.data,
			cmd_data->svc_hash.dlen);
#ifdef WL_NAN_DEBUG
		prhex("hashed svc name", cmd_data->svc_hash.data,
			cmd_data->svc_hash.dlen);
#endif /* WL_NAN_DEBUG */
	} else {
		ret = BCME_ERROR;
		WL_ERR(("invalid svc hash data or length = %d\n", cmd_data->svc_hash.dlen));
		goto fail;
	}

	/* Optional parameters: fill the sub_command block with service descriptor attr */
	sub_cmd->id = htod16(cmd_id);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
	pxtlv = (uint8*)&sd_params->optional[0];

	buflen = buflen_at_start = nan_iov_data->nan_iov_len;
	buflen -= OFFSETOF(wl_nan_sd_params_t, optional[0]);

	if (cmd_data->svc_info.data && cmd_data->svc_info.dlen) {
		WL_DBG(("optional svc_info present, pack it\n"));
		ret = bcm_pack_xtlv_entry(&pxtlv, &buflen,
			WL_NAN_XTLV_SVC_INFO, cmd_data->svc_info.dlen,
			cmd_data->svc_info.data, BCM_XTLV_OPTION_ALIGN32);
		if (unlikely(ret)) {
			WL_ERR(("%s: fail to pack on bcm_pack_xtlv_entry\n", __FUNCTION__));
			goto fail;
		}
	}

	if (cmd_data->tx_match.dlen) {
		WL_DBG(("optional tx match filter presnet (len=%d)\n",
			cmd_data->tx_match.dlen));
		ret = bcm_pack_xtlv_entry(&pxtlv, &buflen, WL_NAN_XTLV_MATCH_TX,
			cmd_data->tx_match.dlen, cmd_data->tx_match.data, BCM_XTLV_OPTION_ALIGN32);
		if (unlikely(ret)) {
			WL_ERR(("%s: failed on xtlv_pack for tx match filter\n", __FUNCTION__));
			goto fail;
		}
	}

	if (cmd_data->life_count) {
		WL_DBG(("optional subscribe count is present, pack it\n"));
		ret = bcm_pack_xtlv_entry(&pxtlv, &buflen, WL_NAN_XTLV_SVC_LIFE_COUNT,
			sizeof(cmd_data->life_count), &cmd_data->life_count,
			BCM_XTLV_OPTION_ALIGN32);
		if (unlikely(ret)) {
			goto fail;
		}
	}

	if (cmd_data->use_srf) {
		uint8 srf_control = 0;
		if (cmd_data->srf_type == SRF_TYPE_BLOOM_FILTER) {
			/* set bloom filter bit */
			srf_control |= 0x1;
		}

		/* set include bit */
		if (cmd_data->srf_include == 1) {
			srf_control |= 0x2;
		}
		WL_DBG(("srf control flag = 0x%x\n", srf_control));

		if (cmd_data->srf_type == SRF_TYPE_SEQ_MAC_ADDR) {
			/* mac list */
			uint16 srf_size = (cmd_data->mac_list.num_mac_addr * ETHER_ADDR_LEN) + 1;
			WL_DBG(("srf size = %d\n", srf_size));

			if (cmd_data->mac_list.num_mac_addr < NAN_SRF_MAX_MAC) {
				srf = kzalloc(srf_size, kflags);
				if (srf == NULL) {
					WL_ERR(("%s: memory allocation failed\n", __FUNCTION__));
					ret = -ENOMEM;
					goto fail;
				}
				memcpy(srf, &srf_control, 1);
				memcpy(srf+1, cmd_data->mac_list.list, (srf_size - 1));
			} else {
				WL_ERR(("Too many mac addr = %d !!!\n",
					cmd_data->mac_list.num_mac_addr));
				goto fail;
			}

			ret = bcm_pack_xtlv_entry(&pxtlv, &buflen, WL_NAN_XTLV_SR_FILTER,
				srf_size, srf, BCM_XTLV_OPTION_ALIGN32);
			if (unlikely(ret)) {
				WL_ERR(("%s: failed on xtlv_pack\n", __FUNCTION__));
				goto fail;
			}
		}
	}

	if (cmd_data->rx_match.dlen) {
		WL_ERR(("optional rx match filter is present, pack it\n"));
		ret = bcm_pack_xtlv_entry(&pxtlv, &buflen, WL_NAN_XTLV_MATCH_RX,
			cmd_data->rx_match.dlen, cmd_data->rx_match.data,
			BCM_XTLV_OPTION_ALIGN32);
		if (unlikely(ret)) {
			WL_ERR(("%s: failed on xtlv_pack for rx match filter\n", __func__));
			goto fail;
		}
	}

	/* adjust iov data len to the end of last data record */
	nan_iov_data->nan_iov_len -= (buflen_at_start - buflen);

	/* fill service attribute length and sub cmd length */
	sd_params->length = (buflen_at_start - buflen) - sizeof(sd_params->length);
	sub_cmd->len = sd_params->length + sizeof(sub_cmd->u.options) + sizeof(sub_cmd->len);

fail:
	if (srf) {
		kfree(srf);
	}

	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfgnan_svc_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, uint16 cmd_id, nan_cmd_data_t *cmd_data)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	s32 ret = BCME_OK;
	uint16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	uint16 nan_buf_size = sizeof(*nan_buf) + NAN_IOCTL_BUF_SIZE;
	uint16 nan_iov_start;
	wl_nan_iov_t *nan_iov_data = NULL;

	NAN_DBG_ENTER();

	nan_buf = kzalloc(nan_buf_size, kflags);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = kzalloc(sizeof(*nan_iov_data), kflags);
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;

	/* contains sub_cmd */
	nan_iov_data->nan_iov_len = nan_iov_start = NAN_IOCTL_BUF_SIZE;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

	ret = wl_cfgnan_sd_params_handler(ndev, cfg, cmd_data, nan_iov_data, cmd_id);
	if (unlikely(ret)) {
		WL_ERR((" Service discovery params handler failed, ret = %d\n", ret));
		goto fail;
	}

	nan_buf->count++;

	/* nan_buf contains optional data and iov batch/subcmd headers */
	nan_buf_size = (nan_iov_start - nan_iov_data->nan_iov_len) +
		OFFSETOF(bcm_iov_batch_buf_t, cmds[0]) +
		OFFSETOF(bcm_iov_batch_subcmd_t, u.options);

	if (nan_buf != NULL) {
		ret = wldev_iovar_setbuf(ndev, "nan", nan_buf, nan_buf_size,
				cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
		if (unlikely(ret)) {
			WL_ERR(("nan svc failed, error = %d\n", ret));
			goto fail;
		} else {
			WL_DBG(("nan svc successful\n"));
		}
	} else {
		WL_ERR(("Invalid nan_buf\n"));
		goto fail;
	}

fail:
	if (nan_buf) {
		kfree(nan_buf);
	}
	if (nan_iov_data) {
		kfree(nan_iov_data);
	}

	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfgnan_pub_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data)
{
	int ret = BCME_OK;

	NAN_DBG_ENTER();

	if (cmd) {
		WL_DBG(("%s handler\n", cmd));
	}

	/*
	 * proceed only if mandatory arguments are present - subscriber id,
	 * service hash
	 */
	if ((!cmd_data->pub_id) || (!cmd_data->svc_hash.data) ||
		(!cmd_data->svc_hash.dlen)) {
		WL_ERR(("mandatory arguments are not present\n"));
		ret = BCME_BADARG;
		goto fail;
	}

	ret = wl_cfgnan_svc_handler(ndev, cfg, WL_NAN_CMD_SD_PUBLISH, cmd_data);
	if (ret < 0) {
		WL_ERR(("%s: fail to handle pub, ret=%d\n", __FUNCTION__, ret));
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfgnan_sub_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data)
{
	int ret = BCME_OK;

	NAN_DBG_ENTER();

	if (cmd) {
		WL_DBG(("%s handler\n", cmd));
	}

	/*
	 * proceed only if mandatory arguments are present - subscriber id,
	 * service hash
	 */
	if ((!cmd_data->sub_id) || (!cmd_data->svc_hash.data) ||
		(!cmd_data->svc_hash.dlen)) {
		WL_ERR(("mandatory arguments are not present\n"));
		ret = BCME_BADARG;
		goto fail;
	}

	ret = wl_cfgnan_svc_handler(ndev, cfg, WL_NAN_CMD_SD_SUBSCRIBE, cmd_data);
	if (ret < 0) {
		WL_ERR(("%s: fail to handle svc, ret=%d\n", __FUNCTION__, ret));
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_cancel_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, nan_cmd_data_t *cmd_data,
	wl_nan_iov_t *nan_iov_data, uint16 cmd_id)
{
	s32 ret = BCME_OK;
	uint8 *p_buf = (uint8*)nan_iov_data->nan_iov_buf;

	NAN_DBG_ENTER();

	if (p_buf != NULL) {
		bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);
		wl_nan_instance_id_t instance_id;
		uint16 subcmd_len;

		ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
				sizeof(instance_id), &subcmd_len);
		if (unlikely(ret)) {
			WL_ERR(("nan_sub_cmd check failed\n"));
			goto fail;
		}

		if (cmd_id == WL_NAN_CMD_SD_CANCEL_PUBLISH) {
			instance_id = cmd_data->pub_id;
		} else if (cmd_id == WL_NAN_CMD_SD_CANCEL_SUBSCRIBE) {
			instance_id = cmd_data->sub_id;
		}  else {
			ret = BCME_USAGE_ERROR;
			WL_ERR(("wrong command id = %u\n", cmd_id));
			goto fail;
		}

		/* Fill the sub_command block */
		sub_cmd->id = htod16(cmd_id);
		sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(instance_id);
		sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
		memcpy(sub_cmd->data, &instance_id, sizeof(instance_id));
		/* Reduce the iov_len size by subcmd_len */
		nan_iov_data->nan_iov_len -= subcmd_len;
		/* Point forward the nan_iov_buf by subcmd_len */
		nan_iov_data->nan_iov_buf += subcmd_len;
	} else {
		WL_ERR(("nan_iov_buf is NULL\n"));
		ret = BCME_ERROR;
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfgnan_cancel_pub_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	s32 ret = BCME_OK;
	uint16 nan_iov_start, nan_iov_end;
	uint16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	uint16 nan_buf_size = sizeof(*nan_buf) + NAN_IOCTL_BUF_SIZE;
	wl_nan_iov_t *nan_iov_data = NULL;

	NAN_DBG_ENTER();

	nan_buf = kzalloc(nan_buf_size, kflags);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = kzalloc(sizeof(*nan_iov_data), kflags);
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data->nan_iov_len = nan_iov_start = NAN_IOCTL_BUF_SIZE;
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

	/* proceed only if mandatory argument is present - publisher id */
	if (!cmd_data->pub_id) {
		WL_ERR(("mandatory argument is not present\n"));
		ret = BCME_BADARG;
		goto fail;
	}
	ret = wl_cfgnan_cancel_handler(ndev, cfg, cmd_data, nan_iov_data,
			WL_NAN_CMD_SD_CANCEL_PUBLISH);
	if (unlikely(ret)) {
		WL_ERR(("cancel publish failed\n"));
		goto fail;
	}
	nan_buf->count++;
	nan_iov_end = nan_iov_data->nan_iov_len;
	nan_buf_size = (nan_iov_start - nan_iov_end);
	if (nan_buf != NULL) {
		ret = wldev_iovar_setbuf(ndev, "nan", nan_buf, nan_buf_size,
				cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
		if (unlikely(ret)) {
			WL_ERR(("nan cancel publish failed, error = %d\n", ret));
			goto fail;
		}
		WL_DBG(("nan cancel publish successfull\n"));
	} else {
		WL_ERR(("Invalid nan_buf\n"));
		goto fail;
	}

fail:
	if (unlikely(ret)) {
		WL_ERR(("cancel publish handler failed\n"));
	}

	if (nan_buf) {
		kfree(nan_buf);
	}
	if (nan_iov_data) {
		kfree(nan_iov_data);
	}

	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfgnan_cancel_sub_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	s32 ret = BCME_OK;
	uint16 nan_iov_start, nan_iov_end;
	uint16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	uint16 nan_buf_size = sizeof(*nan_buf) + NAN_IOCTL_BUF_SIZE;
	wl_nan_iov_t *nan_iov_data = NULL;

	NAN_DBG_ENTER();

	nan_buf = kzalloc(nan_buf_size, kflags);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = kzalloc(sizeof(*nan_iov_data), kflags);
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data->nan_iov_len = nan_iov_start = NAN_IOCTL_BUF_SIZE;
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

	/* proceed only if mandatory argument is present - subscriber id */
	if (!cmd_data->sub_id) {
		WL_ERR(("mandatory argument is not present\n"));
		ret = BCME_BADARG;
		goto fail;
	}
	ret = wl_cfgnan_cancel_handler(ndev, cfg, cmd_data, nan_iov_data,
			WL_NAN_CMD_SD_CANCEL_SUBSCRIBE);
	if (unlikely(ret)) {
		WL_ERR(("cancel subscribe failed\n"));
		goto fail;
	}
	nan_buf->count++;

	nan_iov_end = nan_iov_data->nan_iov_len;
	nan_buf_size = (nan_iov_start - nan_iov_end);
	if (nan_buf != NULL) {
		ret = wldev_iovar_setbuf(ndev, "nan", nan_buf, nan_buf_size,
				cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
		if (unlikely(ret)) {
			WL_ERR(("nan cancel subscribe failed, error = %d\n", ret));
			goto fail;
		}
		WL_DBG(("subscribe cancel successfull\n"));
	} else {
		WL_ERR(("Invalid nan_buf\n"));
		goto fail;
	}

fail:
	if (unlikely(ret)) {
		WL_ERR(("subscribe cancel handler failed\n"));
	}
	if (nan_buf) {
		kfree(nan_buf);
	}

	NAN_DBG_EXIT();
	return ret;
}

int
get_ie_data(uchar *data_str, uchar *ie_data, int len)
{
	uchar *src, *dest;
	uchar val;
	int idx;
	char hexstr[3];

	src = data_str;
	dest = ie_data;

	for (idx = 0; idx < len; idx++) {
		hexstr[0] = src[0];
		hexstr[1] = src[1];
		hexstr[2] = '\0';

		val = (uchar) strtoul(hexstr, NULL, 16);

		*dest++ = val;
		src += 2;
	}

	return 0;
}

/*
 * packs user data (in hex string) into tlv record
 * advances tlv pointer to next xtlv slot
 * buflen is used for tlv_buf space check
 */
int
bcm_pack_xtlv_entry_from_hex_string(uint8 **tlv_buf, uint16 *buflen, uint16 type, char *hex)
{
	bcm_xtlv_t *ptlv = (bcm_xtlv_t *)*tlv_buf;
	uint16 len = strlen(hex)/2;

	/* copy data from tlv buffer to dst provided by user */
	if (ALIGN_SIZE(BCM_XTLV_HDR_SIZE + len, BCM_XTLV_OPTION_ALIGN32) > *buflen) {
		printf("bcm_pack_xtlv_entry: no space tlv_buf: requested:%d, available:%d\n",
				((int)BCM_XTLV_HDR_SIZE + len), *buflen);
		return BCME_BADLEN;
	}
	ptlv->id = htol16(type);
	ptlv->len = htol16(len);

	/* copy callers data */
	if (get_ie_data((uchar*)hex, ptlv->data, len)) {
		return BCME_BADARG;
	}

	/* advance callers pointer to tlv buff */
	*tlv_buf += BCM_XTLV_SIZE(ptlv, BCM_XTLV_OPTION_ALIGN32);
	/* decrement the len */
	*buflen -=  BCM_XTLV_SIZE(ptlv, BCM_XTLV_OPTION_ALIGN32);
	return BCME_OK;
}


int
wl_cfgnan_transmit_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data)
{
	s32 ret = BCME_OK;
	bcm_iov_batch_buf_t *nan_buf = NULL;
	wl_nan_sd_transmit_t *sd_xmit = NULL;
	wl_nan_iov_t *nan_iov_data = NULL;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	bool is_lcl_id = FALSE;
	bool is_dest_id = FALSE;
	bool is_dest_mac = FALSE;
	uint16 nan_iov_start, nan_iov_end;
	uint16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	uint16 nan_buf_size = sizeof(*nan_buf) + NAN_IOCTL_BUF_SIZE;
	uint16 subcmd_len, tlv_start = 0;
	uint8 *pxtlv;

	NAN_DBG_ENTER();

	if (!cfg->nan_enable || !cfg->nan_running) {
		WL_ERR(("nan is not enabled or running, nan transmit blocked\n"));
		return BCME_ERROR;
	}

	nan_buf = kzalloc(nan_buf_size, kflags);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = kzalloc(sizeof(*nan_iov_data), kflags);
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	/* nan transmit */
	nan_iov_data->nan_iov_len = nan_iov_start = NAN_IOCTL_BUF_SIZE;
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

	/*
	 * proceed only if mandatory arguments are present - subscriber id,
	 * publisher id, mac address
	 */

	if ((!cmd_data->local_id) || (!cmd_data->remote_id) ||
			ETHER_ISNULLADDR(&cmd_data->mac_addr.octet)) {
		WL_ERR(("mandatory arguments are not present\n"));
		return -EINVAL;
	}

	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);
	sd_xmit = (wl_nan_sd_transmit_t*)(sub_cmd->data);
	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(*sd_xmit), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		goto fail;
	}

	/* local instance id must be from 0 to 254, 255 is vendor specific */
	if (cmd_data->local_id <= NAN_ID_MIN || cmd_data->local_id > (NAN_ID_MAX - 1)) {
		WL_ERR(("Invalid local instance id\n"));
		ret = BCME_BADARG;
		goto fail;
	}
	sd_xmit->local_service_id = cmd_data->local_id;
	is_lcl_id = TRUE;

	/* remote instance id must be from 0 to 254, 255 is vendor specific */
	if (cmd_data->remote_id <= NAN_ID_MIN || cmd_data->remote_id > (NAN_ID_MAX - 1)) {
		WL_ERR(("Invalid remote instance id\n"));
		ret = BCME_BADARG;
		goto fail;
	}

	sd_xmit->requestor_service_id = cmd_data->remote_id;
	is_dest_id = TRUE;

	if (!ETHER_ISNULLADDR(&cmd_data->mac_addr.octet)) {
		memcpy(&sd_xmit->destination_addr, &cmd_data->mac_addr, ETHER_ADDR_LEN);
	} else {
		WL_ERR(("Invalid ether addr provided\n"));
		ret = BCME_BADARG;
		goto fail;
	}
	is_dest_mac = TRUE;

	if (cmd_data->priority) {
		sd_xmit->priority = cmd_data->priority;
	}

	tlv_start = subcmd_len;
	if (cmd_data->svc_info.data && cmd_data->svc_info.dlen) {
		ret = bcm_pack_xtlv_entry_from_hex_string(&pxtlv,
				&subcmd_len, WL_NAN_XTLV_SD_SVC_INFO, cmd_data->svc_info.data);
		if (ret != BCME_OK) {
			WL_ERR(("unable to process svc_spec_info: %d\n", ret));
			return ret;
		}
		sd_xmit->service_info_len = (tlv_start - subcmd_len);
	}
	tlv_start += (tlv_start - subcmd_len);

	/* Fill the sub_command block */
	sub_cmd->id = htod16(WL_NAN_CMD_SD_TRANSMIT);
	sub_cmd->len = sizeof(sub_cmd->u.options) +
		OFFSETOF(wl_nan_sd_transmit_t, service_info[0]);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
	pxtlv = (uint8 *)&sd_xmit->service_info[0];

	if (is_lcl_id && is_dest_id && is_dest_mac) {
		nan_buf->count++;
		/* Reduce the iov_len size by subcmd_len + tlv len */
		nan_iov_data->nan_iov_len -= tlv_start;
		nan_iov_end = nan_iov_data->nan_iov_len;
		nan_buf_size = (nan_iov_start - nan_iov_end);
	} else {
		WL_ERR(("Missing parameters\n"));
		ret = BCME_USAGE_ERROR;
	}

	if (nan_buf != NULL) {
		ret = wldev_iovar_setbuf(ndev, "nan", nan_buf, nan_buf_size,
				cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
		if (unlikely(ret)) {
			WL_ERR(("nan tranmsit failed, error = %d\n", ret));
			goto fail;
		} else {
			WL_DBG(("nan tranmsit successful\n"));
		}
	} else {
		WL_ERR(("Invalid nan_buf\n"));
	}

fail:
	if (nan_buf) {
		kfree(nan_buf);
	}
	if (nan_iov_data) {
		kfree(nan_iov_data);
	}

	NAN_DBG_EXIT();
	return ret;
}

#ifdef NAN_DP
int
wl_cfgnan_data_path_iface_create_delete_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size,
	nan_cmd_data_t *cmd_data, uint16 type)
{
	/* TODO: */
	return BCME_OK;
}

int wl_cfgnan_data_path_request_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size,
	nan_cmd_data_t *cmd_data, uint8 *ndp_instance_id)
{
	s32 ret = BCME_OK;
	bcm_iov_batch_buf_t *nan_buf = NULL;
	wl_nan_dp_req_t *datareq = NULL;
	wl_nan_iov_t *nan_iov_data = NULL;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	uint16 nan_iov_start, nan_iov_end;
	uint16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	uint16 nan_buf_size = sizeof(*nan_buf) + NAN_IOCTL_BUF_SIZE;
	uint16 subcmd_len, tlv_start = 0;
	uint8 *pxtlv;

	NAN_DBG_ENTER();

	if (!cfg->nan_enable || !cfg->nan_running) {
		WL_ERR(("nan is not enabled or running, nan dp request blocked\n"));
		return BCME_ERROR;
	}

	nan_buf = kzalloc(nan_buf_size, kflags);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = kzalloc(sizeof(*nan_iov_data), kflags);
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data->nan_iov_len = nan_iov_start = NAN_IOCTL_BUF_SIZE;
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);
	datareq = (wl_nan_dp_req_t *)(sub_cmd->data);
	pxtlv = (uint8 *)&datareq->svc_spec_info;

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(*datareq), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		goto fail;
	}
	/* setting default data path type to unicast */
	datareq->type = WL_NAN_DP_TYPE_UNICAST;

	if (cmd_data->pub_id) {
		datareq->pub_id = cmd_data->pub_id;
	}

	/* 0- open security 1-CSID 2-MKID */
	datareq->security = 0;

	if (!ETHER_ISNULLADDR(&cmd_data->mac_addr.octet)) {
		memcpy(&datareq->peer_mac, &cmd_data->mac_addr, ETHER_ADDR_LEN);
	} else {
		WL_ERR(("Invalid ether addr provided\n"));
		ret = BCME_BADARG;
		goto fail;
	}
	if (cmd_data->svc_info.data && cmd_data->svc_info.dlen) {
		tlv_start = subcmd_len;
		ret = bcm_pack_xtlv_entry_from_hex_string(&pxtlv,
				&subcmd_len, WL_NAN_XTLV_SD_SVC_INFO, cmd_data->svc_info.data);
		if (ret != BCME_OK) {
			WL_ERR(("unable to process svc_spec_info: %d\n", ret));
			return ret;
		}
		tlv_start += (tlv_start - subcmd_len);
		datareq->flag |= WL_NAN_DP_FLAG_SVC_INFO;
	}

	/* Fill the sub_command block */
	sub_cmd->id = htod16(WL_NAN_CMD_DATA_DATAREQ);
	sub_cmd->len = sizeof(sub_cmd->u.options) +
		sizeof(*datareq);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);

	nan_buf->count++;
	/* Reduce the iov_len size by subcmd_len + tlv len */
	nan_iov_data->nan_iov_len -= tlv_start;
	nan_iov_end = nan_iov_data->nan_iov_len;
	nan_buf_size = (nan_iov_start - nan_iov_end);

	if (nan_buf != NULL) {
		ret = wldev_iovar_setbuf(ndev, "nan", nan_buf, nan_buf_size,
				cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
		if (unlikely(ret)) {
			WL_ERR(("nan data path request handler failed, error = %d\n", ret));
			goto fail;
		} else {
			WL_DBG(("nan data path request handler successfull\n"));
		}
	} else {
		WL_ERR(("Invalid nan_buf\n"));
		goto fail;
	}

fail:
	if (nan_buf) {
		kfree(nan_buf);
	}
	if (nan_iov_data) {
		kfree(nan_iov_data);
	}

	NAN_DBG_EXIT();
	return ret;
}

int wl_cfgnan_data_path_response_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size,
	nan_cmd_data_t *cmd_data)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	wl_nan_dp_resp_t *dataresp = NULL;
	wl_nan_iov_t *nan_iov_data = NULL;
	s32 ret = BCME_OK;
	uint16 nan_iov_start, nan_iov_end;
	uint16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	uint16 nan_buf_size = sizeof(*nan_buf) + NAN_IOCTL_BUF_SIZE;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	uint16 subcmd_len, tlv_start;
	uint8 *pxtlv;

	NAN_DBG_ENTER();

	if (!cfg->nan_enable || !cfg->nan_running) {
		WL_ERR(("nan is not enabled or running, nan dp resp blocked\n"));
		return BCME_ERROR;
	}

	nan_buf = kzalloc(nan_buf_size, kflags);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = kzalloc(sizeof(*nan_iov_data), kflags);
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data->nan_iov_len = nan_iov_start = NAN_IOCTL_BUF_SIZE;
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);
	dataresp = (wl_nan_dp_resp_t*)(sub_cmd->data);
	pxtlv = (uint8 *)&dataresp->svc_spec_info;

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(*dataresp), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		goto fail;
	}

	/* Setting default data path type to unicast */
	dataresp->type = WL_NAN_DP_TYPE_UNICAST;
	dataresp->status = cmd_data->rsp_code;
	dataresp->reason_code = 0;

	/* ndp instance id must be from 0 to 254, 255 is vendor specific */
	if (cmd_data->ndp_instance_id <= NAN_ID_MIN ||
		cmd_data->ndp_instance_id > (NAN_ID_MAX - 1)) {
		WL_ERR(("Invalid ndp instance id\n"));
		ret = BCME_BADARG;
		goto fail;
	}
	dataresp->ndp_id = cmd_data->ndp_instance_id;

	/* 0- open security 1-CSID 2-MKID */
	dataresp->security = 0;

	if (cmd_data->svc_info.data && cmd_data->svc_info.dlen) {
		tlv_start = subcmd_len;
		ret = bcm_pack_xtlv_entry_from_hex_string(&pxtlv,
				&subcmd_len, WL_NAN_XTLV_SD_SVC_INFO, dataresp->svc_spec_info);
		if (ret != BCME_OK) {
			printf("unable to process svc_spec_info: %d\n", ret);
			return ret;
		}
		tlv_start += (tlv_start - subcmd_len);
		dataresp->flag |= WL_NAN_DP_FLAG_SVC_INFO;
	}

	/* Fill sub_cmd block */
	sub_cmd->id = htod16(WL_NAN_CMD_DATA_DATARESP);
	sub_cmd->len = sizeof(sub_cmd->u.options) +
		sizeof(*dataresp);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
	nan_buf->count++;
	/* Reduce the iov_len size by subcmd_len */
	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_end = nan_iov_data->nan_iov_len;
	nan_buf_size = (nan_iov_start - nan_iov_end);

	if (nan_buf != NULL) {
		ret = wldev_iovar_setbuf(ndev, "nan", nan_buf, nan_buf_size,
				cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
		if (unlikely(ret)) {
			WL_ERR(("nan data path response handler failed, error = %d\n", ret));
			goto fail;
		}  else {
			WL_DBG(("nan data path response handler successful\n"));
		}
	} else {
		WL_ERR(("Invalid nan_buf\n"));
	}

fail:
	if (nan_buf) {
		kfree(nan_buf);
	}
	if (nan_iov_data) {
		kfree(nan_iov_data);
	}

	NAN_DBG_EXIT();
	return ret;
}

int wl_cfgnan_data_path_end_handler(struct net_device *ndev,
		struct bcm_cfg80211 *cfg, char *cmd, int size,
		nan_cmd_data_t *cmd_data)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	wl_nan_dp_end_t *dataend = NULL;
	wl_nan_iov_t *nan_iov_data = NULL;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	s32 ret = BCME_OK;
	uint16 nan_iov_start, nan_iov_end;
	uint16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	uint16 nan_buf_size = sizeof(*nan_buf) + NAN_IOCTL_BUF_SIZE;
	uint16 subcmd_len;

	NAN_DBG_ENTER();

	if (!cfg->nan_enable || !cfg->nan_running) {
		WL_ERR(("nan is not enabled or running, nan dp end blocked\n"));
		return BCME_ERROR;
	}

	/* ndp instance id must be from 0 to 254, 255 is vendor specific */
	if (cmd_data->ndp_instance_id <= NAN_ID_MIN ||
		cmd_data->ndp_instance_id > (NAN_ID_MAX - 1)) {
		WL_ERR(("Invalid ndp instance id\n"));
		ret = BCME_BADARG;
		goto fail;
	}
	nan_buf = kzalloc(nan_buf_size, kflags);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = kzalloc(sizeof(*nan_iov_data), kflags);
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data->nan_iov_len = nan_iov_start = NAN_IOCTL_BUF_SIZE;
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);
	dataend = (wl_nan_dp_end_t*)(sub_cmd->data);

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(*dataend), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan sub cmd check failed\n"));
		goto fail;
	}

	/* Fill sub_cmd block */
	sub_cmd->id = htod16(WL_NAN_CMD_DATA_DATAEND);
	sub_cmd->len = sizeof(sub_cmd->u.options) +
		sizeof(*dataend);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);

	dataend->lndp_id = cmd_data->ndp_instance_id;

	/*
	 * Currently fw requires ndp_id and reason to end the data path
	 * But wifi_nan.h takes ndp_instances_count and ndp_id.
	 * Will keep reason = accept always.
	 */

	dataend->status = 1;
	nan_buf->count++;
	/* Reduce the iov_len size by subcmd_len */
	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_end = nan_iov_data->nan_iov_len;
	nan_buf_size = (nan_iov_start - nan_iov_end);

	if (nan_buf != NULL) {
		ret = wldev_iovar_setbuf(ndev, "nan", nan_buf, nan_buf_size,
				cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
		if (unlikely(ret)) {
			WL_ERR(("nan data path end handler failed, error = %d\n", ret));
		}  else {
			WL_DBG(("nan data path end handler successful\n"));
		}
	} else {
		WL_ERR(("Invalid nan_buf\n"));
	}
fail:
	if (nan_buf) {
		kfree(nan_buf);
	}
	if (nan_iov_data) {
		kfree(nan_iov_data);
	}

	NAN_DBG_EXIT();
	return ret;
}
#endif /* NAN_DP */


int
wl_cfgnan_set_config_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data)
{
	wl_nan_ioc_t *nanioc = NULL;
	s32 ret = BCME_OK;
	uint16 start, end;
	uint16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	uint16 nanioc_size = sizeof(wl_nan_ioc_t) + NAN_IOCTL_BUF_SIZE;
	uint8 *pxtlv;

	UNUSED_PARAMETER(pxtlv);
	if (cfg->nan_running == true) {
		WL_ERR(("Stop nan (NAN_STOP) before issuing NAN_CONFIG command\n"));
		return BCME_ERROR;
	}

	if (cfg->nan_enable != true) {
		ret = wl_cfgnan_enable_handler(cmd_data, NULL);
		if (unlikely(ret)) {
			goto fail;
		}
	}

	nanioc = kzalloc(nanioc_size, kflags);
	if (!nanioc) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		return -ENOMEM;
	}

	/*
	* command to test
	*
	* wl: wl nan <attr> <value> (wl nan role 1)
	*
	* wpa_cli: DRIVER NAN_CONFIG_SET ATTR=<attr> <value>...<value>
	*
	* wpa_cli: DRIVER NAN_SET_CONFIG ATTR=ATTR_ROLE ROLE=1
	*/

	/* nan set config */
	start = end = NAN_IOCTL_BUF_SIZE;
	nanioc->version = htod16(WL_NAN_IOCTL_VERSION);
	nanioc->id = htod16(WL_NAN_CMD_ATTR);
	pxtlv = nanioc->data;
#ifdef NOT_YET
	switch (cmd_data->attr.type) {
	case WL_NAN_XTLV_ROLE:
		WL_DBG((" set nan ROLE = %#x\n", cmd_data->role));
		ret = bcm_pack_xtlv_entry(&pxtlv, &end, WL_NAN_XTLV_ROLE,
			sizeof(cmd_data->role), (uint8 *)&cmd_data->role,
			BCM_XTLV_OPTION_ALIGN32);
		break;
	case WL_NAN_XTLV_MASTER_PREF:
		WL_DBG((" set nan MASTER PREF = %#x\n", cmd_data->master_pref));
		ret = bcm_pack_xtlv_entry(&pxtlv, &end, WL_NAN_XTLV_MASTER_PREF,
			sizeof(cmd_data->master_pref), (uint8 *)&cmd_data->master_pref,
			BCM_XTLV_OPTION_ALIGN32);
		break;
	case WL_NAN_XTLV_OPERATING_BAND:
		WL_DBG((" set nan OPERATING BAND = %#x\n", cmd_data->nan_band));
		ret = bcm_pack_xtlv_entry(&pxtlv, &end, WL_NAN_XTLV_OPERATING_BAND,
				sizeof(cmd_data->nan_band), (uint8 *)&cmd_data->nan_band,
				BCM_XTLV_OPTION_ALIGN32);
		break;
	case WL_NAN_XTLV_DW_LEN:
		WL_DBG((" set nan DW LEN = %#x\n", cmd_data->dw_len));
		ret = bcm_pack_xtlv_entry(&pxtlv, &end, WL_NAN_XTLV_DW_LEN,
			sizeof(cmd_data->dw_len), (uint8 *)&cmd_data->dw_len,
			BCM_XTLV_OPTION_ALIGN32);
		break;
	case WL_NAN_XTLV_CLUSTER_ID:
		WL_DBG((" set nan CLUSTER ID "));
		ret = bcm_pack_xtlv_entry(&pxtlv, &end, WL_NAN_XTLV_CLUSTER_ID,
			sizeof(cmd_data->clus_id), (uint8 *)&cmd_data->clus_id,
			BCM_XTLV_OPTION_ALIGN32);
		break;
	case WL_NAN_XTLV_IF_ADDR:
		WL_DBG((" set nan IFADDR "));
		ret = bcm_pack_xtlv_entry(&pxtlv, &end, WL_NAN_XTLV_IF_ADDR,
			sizeof(cmd_data->if_addr), (uint8 *)&cmd_data->if_addr,
			BCM_XTLV_OPTION_ALIGN32);
		break;
	case WL_NAN_XTLV_MAC_CHANSPEC:
		WL_DBG((" set nan CHANSPEC = %#x\n", cmd_data->chanspec));
		ret = bcm_pack_xtlv_entry(&pxtlv, &end, WL_NAN_XTLV_MAC_CHANSPEC,
			sizeof(cmd_data->chanspec), (uint8 *)&cmd_data->chanspec,
			BCM_XTLV_OPTION_ALIGN32);
		break;
	case WL_NAN_XTLV_BCN_INTERVAL:
		WL_DBG((" set nan BCN_INTERVAL = %#x\n", cmd_data->beacon_int));
		ret = bcm_pack_xtlv_entry(&pxtlv, &end, WL_NAN_XTLV_BCN_INTERVAL,
			sizeof(cmd_data->beacon_int), (uint8 *)&cmd_data->beacon_int,
			BCM_XTLV_OPTION_ALIGN32);
		break;
	case WL_NAN_XTLV_MAC_TXRATE:
		break;
	case WL_NAN_XTLV_HOPCNT_LIMIT:
		WL_DBG((" set hop count limit = %d\n", cmd_data->hop_count_limit));
		ret = bcm_pack_xtlv_entry(&pxtlv, &end, WL_NAN_XTLV_HOPCNT_LIMIT,
			sizeof(cmd_data->hop_count_limit), (uint8 *)&cmd_data->hop_count_limit,
			BCM_XTLV_OPTION_ALIGN32);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	if (unlikely(ret)) {
		WL_ERR((" unsupported attribute, attr = %s (%d) \n",
			cmd_data->attr.name, cmd_data->attr.type));
		goto fail;
	}
#endif /* NOT_YET */
	nanioc->len = start - end;
	nanioc_size = sizeof(wl_nan_ioc_t) + nanioc->len;
	ret = wldev_iovar_setbuf(ndev, "nan", nanioc, nanioc_size,
		cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
	if (unlikely(ret)) {
		WL_ERR(("nan set config failed, error = %d\n", ret));
	} else {
		WL_DBG(("nan set config successful\n"));
	}

fail:
	if (nanioc) {
		kfree(nanioc);
	}

	return ret;
}

int
wl_cfgnan_rtt_config_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data)
{
	wl_nan_ranging_config_t rtt_config;
	s32 ret = BCME_OK;

	/* proceed only if mandatory argument is present - channel */
	if (!cmd_data->chanspec) {
		WL_ERR(("mandatory argument is not present\n"));
		return -EINVAL;
	}

	/*
	* command to test
	*
	* wl: wl proxd_nancfg 44/80 128 32 ff:ff:ff:ff:ff:ff 1
	*
	* wpa_cli: DRIVER NAN_RTT_CONFIG CHAN=44/80
	*/

	memset(&rtt_config, 0, sizeof(rtt_config));
	rtt_config.chanspec = cmd_data->chanspec;
	rtt_config.timeslot = 128;
	rtt_config.duration = 32;
	memcpy(&rtt_config.allow_mac, &ether_bcast, ETHER_ADDR_LEN);
	rtt_config.flags = 1;

	ret = wldev_iovar_setbuf(ndev, "proxd_nancfg", &rtt_config,
		sizeof(wl_nan_ranging_config_t), cfg->ioctl_buf,
		WLC_IOCTL_MEDLEN, NULL);
	if (unlikely(ret)) {
		WL_ERR(("nan rtt config failed, error = %d\n", ret));
	} else {
		WL_DBG(("nan rtt config successful\n"));
	}

	return ret;
}

int
wl_cfgnan_rtt_find_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data)
{
	void *iovbuf;
	wl_nan_ranging_list_t *rtt_list;
	s32 iovbuf_size = NAN_RTT_IOVAR_BUF_SIZE;
	s32 ret = BCME_OK;
	u16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;

	/*
	* proceed only if mandatory arguments are present - channel, bitmap,
	* mac address
	*/
	if ((!cmd_data->chanspec) || (!cmd_data->bmap) ||
		ETHER_ISNULLADDR(&cmd_data->mac_addr.octet)) {
		WL_ERR(("mandatory arguments are not present\n"));
		return -EINVAL;
	}

	iovbuf = kzalloc(iovbuf_size, kflags);
	if (!iovbuf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		return -ENOMEM;
	}

	/*
	 * command to test
	 *
	 * wl: wl proxd_nanfind 1 44/80 <mac_addr> 0x300 5 6 1
	 *
	 * wpa_cli: DRIVER NAN_RTT_FIND MAC_ADDR=<mac_addr> CHAN=44/80 BMAP=0x300
	 *
	 */
	rtt_list = (wl_nan_ranging_list_t *)iovbuf;
	rtt_list->count = 1;
	rtt_list->num_peers_done = 0;
	rtt_list->num_dws = 1;
	rtt_list->rp[0].chanspec = cmd_data->chanspec;
	memcpy(&rtt_list->rp[0].ea, &cmd_data->mac_addr,
		sizeof(struct ether_addr));
	rtt_list->rp[0].abitmap = cmd_data->bmap;
	rtt_list->rp[0].frmcnt = 5;
	rtt_list->rp[0].retrycnt = 6;
	rtt_list->rp[0].flags = 1;

	iovbuf_size = sizeof(wl_nan_ranging_list_t) +
		sizeof(wl_nan_ranging_peer_t);
	ret = wldev_iovar_setbuf(ndev, "proxd_nanfind", iovbuf,
		iovbuf_size, cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
	if (unlikely(ret)) {
		WL_ERR(("nan rtt find failed, error = %d\n", ret));
	} else {
		WL_DBG(("nan rtt find successful\n"));
	}

	if (iovbuf) {
		kfree(iovbuf);
	}

	return ret;
}

#ifdef WL_NAN_DEBUG
int
wl_cfgnan_debug_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data)
{
	/*
	* command to test
	*
	* wpa_cli: DRIVER NAN_DEBUG DEBUG=1
	*
	*/

	g_nan_debug = cmd_data->debug_flag;

	/* reconfigure nan events */
	return wl_cfgnan_enable_events(ndev, cfg, 0, 0);
}
#endif /* WL_NAN_DEBUG */

static int wl_cfgnan_config_attr(char *buf, nan_config_attr_t *attr)
{
	s32 ret = BCME_OK;
	nan_config_attr_t *nanc = NULL;
	UNUSED_PARAMETER(nanc);
#ifdef NOT_YET
	/* only one attribute at a time */
	for (nanc = &nan_config_attrs[0]; strlen(nanc->name) != 0; nanc++) {
		if (!strncmp(nanc->name, buf, strlen(nanc->name))) {
			strncpy((char *)attr->name, buf, strlen(nanc->name));
			attr->type = nanc->type;
			ret = strlen(nanc->name);
			break;
		}
	}
#endif /* NOT_YET */

	return ret;
}

static int wl_cfgnan_parse_args(char *buf, nan_cmd_data_t *cmd_data)
{
	s32 ret = BCME_OK;
	char *token = buf;
	char delim[] = " ";

	NAN_DBG_ENTER();

	while ((buf != NULL) && (token != NULL)) {
		if (!strncmp(buf, PUB_ID_PREFIX, strlen(PUB_ID_PREFIX))) {
			buf += strlen(PUB_ID_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->pub_id = simple_strtoul(token, NULL, 10);
			cmd_data->local_id ? (cmd_data->remote_id = cmd_data->pub_id) :
				(cmd_data->local_id = cmd_data->pub_id);
#ifdef NAN_P2P_CONFIG
		} else if (!strncmp(buf, P2P_IE_PREFIX, strlen(P2P_IE_PREFIX))) {
			buf += strlen(P2P_IE_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->p2p_info.data = token;
			cmd_data->p2p_info.dlen = strlen(token);
		} else if (!strncmp(buf, IE_EN_PREFIX, strlen(IE_EN_PREFIX))) {
			buf += strlen(IE_EN_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->p2p_info.data = token;
			cmd_data->p2p_info.dlen = strlen(token);
#endif /* NAN_P2P_CONFIG */
		} else if (!strncmp(buf, SUB_ID_PREFIX, strlen(SUB_ID_PREFIX))) {
			buf += strlen(SUB_ID_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->sub_id = simple_strtoul(token, NULL, 10);
			cmd_data->local_id ? (cmd_data->remote_id = cmd_data->sub_id) :
				(cmd_data->local_id = cmd_data->sub_id);
		} else if (!strncmp(buf, MAC_ADDR_PREFIX, strlen(MAC_ADDR_PREFIX))) {
			buf += strlen(MAC_ADDR_PREFIX);
			token = strsep(&buf, delim);
			if (!wl_cfg80211_ether_atoe(token, &cmd_data->mac_addr)) {
				WL_ERR(("invalid mac address, mac_addr = "MACDBG "\n",
					MAC2STRDBG(cmd_data->mac_addr.octet)));
				ret = -EINVAL;
				goto fail;
			}
		} else if (!strncmp(buf, SVC_HASH_PREFIX, strlen(SVC_HASH_PREFIX))) {
			buf += strlen(SVC_HASH_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->svc_hash.data = token;
			cmd_data->svc_hash.dlen = WL_NAN_SVC_HASH_LEN;
		} else if (!strncmp(buf, SVC_INFO_PREFIX, strlen(SVC_INFO_PREFIX))) {
			buf += strlen(SVC_INFO_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->svc_info.data = token;
			cmd_data->svc_info.dlen = strlen(token);
		} else if (!strncmp(buf, CHAN_PREFIX, strlen(CHAN_PREFIX))) {
			buf += strlen(CHAN_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->chanspec = wf_chspec_aton(token);
			cmd_data->chanspec = wl_chspec_host_to_driver(cmd_data->chanspec);
			if (NAN_INVALID_CHANSPEC(cmd_data->chanspec)) {
				WL_ERR(("invalid chanspec, chanspec = 0x%04x\n",
					cmd_data->chanspec));
				ret = -EINVAL;
				goto fail;
			}
		} else if (!strncmp(buf, BITMAP_PREFIX, strlen(BITMAP_PREFIX))) {
			buf += strlen(BITMAP_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->bmap = simple_strtoul(token, NULL, 16);
		} else if (!strncmp(buf, ATTR_PREFIX, strlen(ATTR_PREFIX))) {
			buf += strlen(ATTR_PREFIX);
			token = strsep(&buf, delim);
			if (!wl_cfgnan_config_attr(token, &cmd_data->attr)) {
				WL_ERR(("invalid attribute, attr = %s\n",
					cmd_data->attr.name));
				ret = -EINVAL;
				goto fail;
			}
		} else if (!strncmp(buf, ROLE_PREFIX, strlen(ROLE_PREFIX))) {
			buf += strlen(ROLE_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->role = simple_strtoul(token, NULL, 10);
			if (NAN_INVALID_ROLE(cmd_data->role)) {
				WL_ERR(("invalid role, role = %d\n", cmd_data->role));
				ret = -EINVAL;
				goto fail;
			}
		} else if (!strncmp(buf, MASTER_PREF_PREFIX,
			strlen(MASTER_PREF_PREFIX))) {
			buf += strlen(MASTER_PREF_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->master_pref = simple_strtoul(token, NULL, 10);
		} else if (!strncmp(buf, CLUS_ID_PREFIX, strlen(CLUS_ID_PREFIX))) {
			buf += strlen(CLUS_ID_PREFIX);
			token = strsep(&buf, delim);
			if (!wl_cfg80211_ether_atoe(token, &cmd_data->clus_id)) {
				WL_ERR(("invalid cluster id, CLUS_ID = "MACDBG "\n",
					MAC2STRDBG(cmd_data->clus_id.octet)));
				ret = -EINVAL;
				goto fail;
			}
		} else if (!strncmp(buf, IF_ADDR_PREFIX, strlen(IF_ADDR_PREFIX))) {
			buf += strlen(IF_ADDR_PREFIX);
			token = strsep(&buf, delim);
			if (!wl_cfg80211_ether_atoe(token, &cmd_data->if_addr)) {
				WL_ERR(("invalid cluster id, IF_ADDR = "MACDBG "\n",
					MAC2STRDBG(cmd_data->if_addr.octet)));
				ret = -EINVAL;
				goto fail;
			}
		} else if (!strncmp(buf, BCN_INTERVAL_PREFIX,
			strlen(BCN_INTERVAL_PREFIX))) {
			buf += strlen(BCN_INTERVAL_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->beacon_int = simple_strtoul(token, NULL, 10);
		} else if (!strncmp(buf, PUB_PR_PREFIX, strlen(PUB_PR_PREFIX))) {
			buf += strlen(PUB_PR_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->period = simple_strtoul(token, NULL, 10);
		} else if (!strncmp(buf, PUB_INT_PREFIX, strlen(PUB_INT_PREFIX))) {
			buf += strlen(PUB_INT_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->ttl = simple_strtoul(token, NULL, 10);
		} else if (!strncmp(buf, DW_LEN_PREFIX, strlen(DW_LEN_PREFIX))) {
			buf += strlen(DW_LEN_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->dw_len = simple_strtoul(token, NULL, 10);
		} else if (!strncmp(buf, DEBUG_PREFIX, strlen(DEBUG_PREFIX))) {
			buf += strlen(DEBUG_PREFIX);
			token = strsep(&buf, delim);
			cmd_data->debug_flag = simple_strtoul(token, NULL, 10);
		} else if (!strncmp(buf, ACTIVE_OPTION, strlen(ACTIVE_OPTION))) {
			buf += strlen(ACTIVE_OPTION);
			token = strsep(&buf, delim);
			cmd_data->flags |= WL_NAN_SUB_ACTIVE;
		} else if (!strncmp(buf, SOLICITED_OPTION, strlen(SOLICITED_OPTION))) {
			buf += strlen(SOLICITED_OPTION);
			token = strsep(&buf, delim);
			cmd_data->flags |= WL_NAN_PUB_SOLICIT;
		} else if (!strncmp(buf, UNSOLICITED_OPTION, strlen(UNSOLICITED_OPTION))) {
			buf += strlen(UNSOLICITED_OPTION);
			token = strsep(&buf, delim);
			cmd_data->flags |= WL_NAN_PUB_UNSOLICIT;
		} else {
			WL_ERR(("unknown token, token = %s, buf = %s\n", token, buf));
			ret = -EINVAL;
			goto fail;
		}
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfgnan_cmd_handler(struct net_device *ndev, struct bcm_cfg80211 *cfg,
	char *cmd, int cmd_len)
{
	nan_cmd_data_t cmd_data;
	uint8 *buf = cmd;
	uint8 *cmd_name = NULL;
	nan_cmd_t *nanc = NULL;
	int buf_len = 0;
	int ret = BCME_OK;

	NAN_DBG_ENTER();

	cmd_name = strsep((char **)&buf, " ");
	if (buf) {
		buf_len = strlen(buf);
	}

	WL_DBG((" cmd_name: %s, buf_len: %d, buf: %s \n", cmd_name, buf_len, buf));

	memset(&cmd_data, 0, sizeof(cmd_data));
	ret = wl_cfgnan_parse_args(buf, &cmd_data);
	if (unlikely(ret)) {
		WL_ERR(("argument parsing failed with error (%d), buf = %s\n",
			ret, buf));
		goto fail;
	}

	for (nanc = nan_cmds; nanc->name; nanc++) {
		if (strncmp(nanc->name, cmd_name, strlen(nanc->name)) == 0) {
			ret = (*nanc->func)(ndev, cfg, cmd, cmd_len, &cmd_data);
			if (ret < BCME_OK) {
				WL_ERR(("command (%s) failed with error (%d)\n",
					cmd_name, ret));
			}
		}
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

s32
wl_cfgnan_notify_proxd_status(struct bcm_cfg80211 *cfg,
	bcm_struct_cfgdev *cfgdev, const wl_event_msg_t *event, void *data)
{
	s32 ret = BCME_OK;
	wl_nan_ranging_event_data_t *rdata;
	uint16 data_len;
	s32 event_type;
	s32 event_num;
	uint8 *buf = NULL;
	uint32 buf_len;
	uint8 *ptr, *end;
	uint16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	s32 i;

	if (!event || !data) {
		WL_ERR(("event data is NULL\n"));
		return -EINVAL;
	}

	event_type = ntoh32(event->event_type);
	event_num = ntoh32(event->reason);
	data_len = ntoh32(event->datalen);

	WL_DBG(("proxd event: type: %d num: %d len: %d\n",
		event_type, event_num, data_len));

	if (NAN_INVALID_PROXD_EVENT(event_num)) {
		WL_ERR(("unsupported event, num: %d\n", event_num));
		return -EINVAL;
	}

#ifdef WL_NAN_DEBUG
	if (g_nan_debug) {
		WL_DBG(("event name: WLC_E_PROXD_NAN_EVENT\n"));
		WL_DBG(("event data:\n"));
		prhex(NULL, data, data_len);
	}
#endif /* WL_NAN_DEBUG */

	if (data_len < sizeof(wl_nan_ranging_event_data_t)) {
		WL_ERR(("wrong data len\n"));
		return -EINVAL;
	}

	rdata = (wl_nan_ranging_event_data_t *)data;

	WL_DBG(("proxd event: count:%d success_count:%d mode:%d\n",
		rdata->count, rdata->success_count, rdata->mode));

#ifdef WL_NAN_DEBUG
	if (g_nan_debug) {
		prhex(" event data: ", data, data_len);
	}
#endif /* WL_NAN_DEBUG */

	buf_len = NAN_IOCTL_BUF_SIZE;
	buf = kzalloc(buf_len, kflags);
	if (!buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		return -ENOMEM;
	}
	end = buf + buf_len;

	for (i = 0; i < rdata->count; i++) {
		if (&rdata->rr[i] == NULL) {
			ret = -EINVAL;
			goto fail;
		}

		ptr = buf;
		WL_DBG((" ranging data for mac:"MACDBG" \n",
			MAC2STRDBG(rdata->rr[i].ea.octet)));
		ptr += snprintf(ptr, end-ptr, SUPP_EVENT_PREFIX"%s " MAC_ADDR_PREFIX MACF
			" "STATUS_PREFIX"%s", EVENT_RTT_STATUS_STR,
			ETHER_TO_MACF(rdata->rr[i].ea), (rdata->rr[i].status == 1) ?
			"success" : "fail");

		if (rdata->rr[i].status == 1) {
			/* add tsf and distance only if status is success */
			ptr += snprintf(ptr, end-ptr, " "TIMESTAMP_PREFIX"0x%x "
				DISTANCE_PREFIX"%d.%04d", rdata->rr[i].timestamp,
				rdata->rr[i].distance >> 4,
				((rdata->rr[i].distance & 0x0f) * 625));
		}
		if (ptr > end) {
			WL_ERR(("Buffer overflow. userspace string truncated: %s, len: %zu\n",
				buf, strlen(buf)));
		}
	}

fail:
	if (buf) {
		kfree(buf);
	}

	return ret;
}

static void
wl_nan_print_status(wl_nan_cfg_status_t *nstatus)
{
	printf("> enabled: %d\n", nstatus->enabled);
	printf("> inited: %d\n", nstatus->inited);
	printf("> joined: %d\n", nstatus->joined);
	printf("> merged: %d\n", nstatus->merged);
	printf("> cluster_id: " MACDBG "\n", MAC2STRDBG(nstatus->cid.octet));

	switch (nstatus->role) {
	case WL_NAN_ROLE_AUTO:
		printf("> role: %s (%d)\n", "auto", nstatus->role);
		break;
	case WL_NAN_ROLE_NON_MASTER_NON_SYNC:
		printf("> role: %s (%d)\n", "non-master-non-sync", nstatus->role);
		break;
	case WL_NAN_ROLE_NON_MASTER_SYNC:
		printf("> role: %s (%d)\n", "non-master-sync", nstatus->role);
		break;
	case WL_NAN_ROLE_MASTER:
		printf("> role: %s (%d)\n", "master", nstatus->role);
		break;
	case WL_NAN_ROLE_ANCHOR_MASTER:
		printf("> role: %s (%d)\n", "anchor-master", nstatus->role);
		break;
	default:
		printf("> role: %s (%d)\n", "undefined", nstatus->role);
		break;
	}

	printf("> chsspec[0]: 0x%x, chsspec[1]: 0x%x\n",
		nstatus->chspec[0], nstatus->chspec[1]);
	printf("> master_rank: " NMRSTR "\n", NMR2STR(nstatus->mr));
	printf("> amr        : " NMRSTR "\n", NMR2STR(nstatus->amr));
	printf("> hop_count: %d\n", nstatus->hop_count);
	printf("> ambtt: %d\n", nstatus->ambtt);
	printf("> cnt_pend_txfrm: %d\n", nstatus->cnt_pend_txfrm);
	printf("> cnt_bcn_tx: %d\n", nstatus->cnt_bcn_tx);
	printf("> cnt_bcn_rx: %d\n", nstatus->cnt_bcn_rx);
	printf("> cnt_svc_disc_tx: %d\n", nstatus->cnt_svc_disc_tx);
	printf("> cnt_svc_disc_rx: %d\n", nstatus->cnt_svc_disc_rx);
}

s32
wl_cfgnan_notify_nan_status(struct bcm_cfg80211 *cfg,
	bcm_struct_cfgdev *cfgdev, const wl_event_msg_t *event, void *event_data)
{
	uint16 data_len;
	uint32 event_num;
	s32 event_type;
	int hal_event_id = 0;
	void *nan_event_data = NULL;
	wl_nan_tlv_data_t *nan_opt_tlvs;
	uint16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	uint16 tlvs_offset = 0;
	uint16 opt_tlvs_len = 0;
	uint8 *tlv_buf;
	s32 ret = BCME_OK;
	nan_de_event_data_t de_event;

	UNUSED_PARAMETER(wl_nan_print_status);
	NAN_DBG_ENTER();

	if (!event || !event_data) {
		WL_ERR(("event data is NULL\n"));
		return -EINVAL;
	}
	event_type = ntoh32(event->event_type);
	event_num = ntoh32(event->reason);
	data_len = ntoh32(event->datalen);

	nan_opt_tlvs = kzalloc(sizeof(*nan_opt_tlvs), kflags);
	if (!nan_opt_tlvs) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		goto fail;
	}

	if (NAN_INVALID_EVENT(event_num)) {
		WL_ERR(("unsupported event, num: %d, event type: %d\n", event_num, event_type));
		return -EINVAL;
	}

#ifdef WL_NAN_DEBUG
	if (g_nan_debug) {
		WL_DBG((">> Nan Event Received: %s (num=%u, len=%d)\n",
			nan_event_to_str(event_num), event_num, data_len));
		prhex(NULL, event_data, data_len);
	}
#endif /* WL_NAN_DEBUG */

	/*
	 * send as preformatted hex string
	 * EVENT_NAN <event_type> <tlv_hex_string>
	 */
	switch (event_num) {
	case WL_NAN_EVENT_START:
	case WL_NAN_EVENT_JOIN:
	case WL_NAN_EVENT_STOP:
	case WL_NAN_EVENT_ROLE:
	case WL_NAN_EVENT_SCAN_COMPLETE:
	case WL_NAN_EVENT_MERGE: {
		/* get nan status info as-is */
		bcm_xtlv_t *xtlv = (bcm_xtlv_t *)event_data;
		wl_nan_config_status_t *nstatus = (wl_nan_config_status_t *)xtlv->data;
#ifdef WL_NAN_DEBUG
		if (g_nan_debug) {
			wl_nan_print_status(nstatus);
		}
#endif /* WL_NAN_DEBUG */

		de_event.nan_de_evt_type = event_num;
		de_event.nstatus = (wl_nan_cfg_status_t *)nstatus;

		nan_event_data = (void*)&de_event;
		hal_event_id = GOOGLE_NAN_EVENT_DE_EVENT;
		break;
	}

	case WL_NAN_EVENT_REPLIED: {
		bcm_xtlv_t *xtlv = (bcm_xtlv_t *)event_data;
		wl_nan_event_replied_t *evr = (wl_nan_event_replied_t *)xtlv->data;

		WL_DBG(("Publish ID: %d\n", evr->pub_id));
		WL_DBG(("Subscriber ID: %d\n", evr->sub_id));
		WL_DBG(("Subscriber MAC addr: " MACDBG "\n", MAC2STRDBG(evr->sub_mac.octet)));
		WL_DBG(("Subscriber RSSI: %d\n", evr->sub_rssi));

		nan_event_data = (void*)evr;
		hal_event_id = GOOGLE_NAN_EVENT_SUBSCRIBE_MATCH;

		break;
	}

	case WL_NAN_EVENT_TERMINATED: {
		uint8 inst_type;
		bcm_xtlv_t *xtlv = (bcm_xtlv_t *)event_data;
		wl_nan_ev_terminated_t *pev = (wl_nan_ev_terminated_t *)xtlv->data;

		WL_DBG(("Instance ID: %d\n", pev->instance_id));
		WL_DBG(("Reason: %d\n", pev->reason));
		WL_DBG(("Service Type: %d\n", pev->svctype));
		nan_event_data = (void*)pev;

		if ((ret = wl_cfgnan_get_inst_type(cfg, pev->instance_id, &inst_type))
		     != BCME_OK) {
			WL_ERR(("Failed to get instance type\n"));
			goto fail;
		}

		if (inst_type == NAN_SVC_INST_PUBLISHER) {
			hal_event_id = GOOGLE_NAN_EVENT_PUBLISH_TERMINATED;
		} else {
			hal_event_id = GOOGLE_NAN_EVENT_SUBSCRIBE_TERMINATED;
		}

		break;
	}

	case WL_NAN_EVENT_RECEIVE: {
		bcm_xtlv_t *xtlv = (bcm_xtlv_t *)event_data;
		wl_nan_ev_receive_t *ev = (wl_nan_ev_receive_t *)xtlv->data;

		WL_DBG(("Local ID: %d\n", ev->local_id));
		WL_DBG(("Remote ID: %d\n", ev->remote_id));
		WL_DBG(("Peer MAC addr: " MACDBG "\n", MAC2STRDBG(ev->remote_addr.octet)));
		WL_DBG(("Peer RSSI: %d\n", ev->fup_rssi));
		hal_event_id = GOOGLE_NAN_EVENT_FOLLOWUP;
		nan_event_data = (void*)ev;

		break;
	}

	case WL_NAN_EVENT_BCN_RX: {
		bcm_xtlv_t *xtlv = (bcm_xtlv_t *)event_data;
		int ctr = 0;

		WL_DBG(("Len: %d\n", xtlv->len));
		WL_DBG(("Beacon Payload:\n"));
		for (; ctr < xtlv->len; ctr++) {
			WL_DBG(("%02X ", xtlv->data[ctr]));
			if ((ctr + 1) % 8  == 0) {
				WL_ERR(("\n"));
			}
		}

		nan_event_data = (void*)&xtlv->data;
		hal_event_id = GOOGLE_NAN_EVENT_BEACON;
		break;
	}

	case WL_NAN_EVENT_DISCOVERY_RESULT: {
		bcm_xtlv_t *xtlv = (bcm_xtlv_t *)event_data;
		wl_nan_event_disc_result_t *ev_disc_result =
			(wl_nan_event_disc_result_t *)xtlv->data;

		WL_DBG(("Publish ID: %d\n", ev_disc_result->pub_id));
		WL_DBG(("Subscribe ID: %d\n", ev_disc_result->sub_id));
		WL_DBG(("Publish MAC addr: " MACDBG "\n",
			MAC2STRDBG(ev_disc_result->pub_mac.octet)));
		WL_DBG(("Pub RSSI: %d\n", ev_disc_result->publish_rssi));

		nan_event_data = (void*)ev_disc_result;
		hal_event_id = GOOGLE_NAN_EVENT_SUBSCRIBE_MATCH;

		break;
	}

	case WL_NAN_EVENT_SDF_RX:
		tlvs_offset = 0;
		opt_tlvs_len = data_len;
		hal_event_id = GOOGLE_NAN_EVENT_SDF;
		break;

	case WL_NAN_EVENT_STATUS_CHG:
		hal_event_id = GOOGLE_NAN_EVENT_DE_EVENT;
		break;

#ifdef NAN_P2P_CONFIG
	case WL_NAN_EVENT_P2P: {
		wl_nan_ev_p2p_avail_t *ev_p2p_avail;
		chanspec_t chanspec;
		ev_p2p_avail = (wl_nan_ev_p2p_avail_t *)event_data;

		WL_DBG("Device Role: %d\n", ev_p2p_avail->dev_role);
		WL_DBG(("Sender MAC addr: " MACDBG "\n",
			MAC2STRDBG(ev_p2p_avail->sender.octet)));
		WL_DBG(("P2P dev addr: " MACDBG "\n",
			MAC2STRDBG(ev_p2p_avail->p2p_dev_addr.octet)));

		WL_DBG(("Repeat: %d\n", ev_p2p_avail->repeat));
		WL_DBG(("Resolution: %d\n", ev_p2p_avail->resolution));

		chanspec = dtoh16(ev_p2p_avail->chanspec);
		WL_DBG(("> Chanspec: 0x%x\n", chanspec));
		WL_DBG(("Avail bitmap: 0x%08x\n", dtoh32(ev_p2p_avail->avail_bmap)));

		break;
	}
#endif /* NAN_P2P_CONFIG */

#ifdef NAN_DP
	case WL_NAN_EVENT_PEER_DATAPATH_IND:
	case WL_NAN_EVENT_PEER_DATAPATH_RESP:
	case WL_NAN_EVENT_PEER_DATAPATH_CONF:
	case WL_NAN_EVENT_DATAPATH_ESTB: {
		wl_nan_ev_datapath_cmn_t *ev_dp;

		ev_dp = (wl_nan_ev_datapath_cmn_t *)event_data;
		WL_DBG(("Event type: %d\n", ev_dp->type));
		WL_DBG(("Status: %d\n", ev_dp->status));
		WL_DBG(("pub_id: %d\n", ev_dp->pub_id));
		WL_DBG(("security: %d\n", ev_dp->security));

		if (ev_dp->type == NAN_DP_SESSION_UNICAST) {
			WL_DBG(("NDP ID: %d\n", ev_dp->ndp_id));
			WL_DBG(("INITIATOR_NDI: " MACDBG "\n",
				MAC2STRDBG(ev_dp->initiator_ndi.octet)));
			WL_DBG(("RESPONDOR_NDI: " MACDBG "\n",
				MAC2STRDBG(ev_dp->responder_ndi.octet)));
		} else {
			WL_DBG(("NDPID: %d\n", ev_dp->mc_id));
			WL_DBG(("INITIATOR_NDI: " MACDBG "\n",
				MAC2STRDBG(ev_dp->initiator_ndi.octet)));
		}

		tlvs_offset = OFFSETOF(wl_nan_ev_datapath_cmn_t, opt_tlvs);
		opt_tlvs_len = data_len - tlvs_offset;

		break;
	}

	case WL_NAN_EVENT_DATAPATH_END:
		hal_event_id = GOOGLE_NAN_EVENT_DATA_END;
		break;
#endif /* NAN_DP */

	default:
		WL_ERR(("WARNING: unimplemented NAN APP EVENT = %d\n", event_num));
		ret = BCME_ERROR;
		goto fail;
	}

	if (opt_tlvs_len) {
		tlv_buf = (uint8 *)event_data + tlvs_offset;

		/* Extract event data tlvs and pass their resp to cb fn */
		ret = bcm_unpack_xtlv_buf((void *)nan_opt_tlvs, (const uint8*)tlv_buf,
			opt_tlvs_len, BCM_IOV_CMD_OPT_ALIGN32, wl_cfgnan_set_vars_cbfn);
	}

	WL_DBG(("Send up %s (%d) data to HAL, hal_event_id=%d\n",
		nan_event_to_str(event_num), event_num, hal_event_id));
	ret = wl_cfgvendor_send_nan_event(cfg->wdev->wiphy, bcmcfg_to_prmry_ndev(cfg),
		hal_event_id, (void*)nan_event_data, nan_opt_tlvs);
	if (ret != BCME_OK) {
		WL_ERR(("Failed to send event to nan hal, %s (%d)\n",
			nan_event_to_str(event_num), event_num));
	}

fail:
	if (nan_opt_tlvs) {
		if (nan_opt_tlvs->svc_info.data)
			kfree(nan_opt_tlvs->svc_info.data);
		if (nan_opt_tlvs->vend_info.data)
			kfree(nan_opt_tlvs->vend_info.data);
		kfree(nan_opt_tlvs);
	}

	NAN_DBG_EXIT();
	return ret;
}
#endif /* WL_NAN */
