/*
 * TKO - TCP Keepalive Offload
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id:$
 */


#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <proto/ethernet.h>
#include <proto/802.3.h>
#include <proto/bcmip.h>
#include <proto/bcmipv6.h>
#include <proto/bcmtcp.h>
#include <proto/bcmipv6.h>
#include <proto/bcmevent.h>
#include <bcmendian.h>

#include <d11.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wl_export.h>
#include <wlc_types.h>
#include <wlc_bsscfg.h>
#include <wlc_event_utils.h>
#include <wl_tko.h>

#ifdef HOFFLOAD_TKO
#include <bcm_hoffload.h>
#endif	/* HOFFLOAD_TKO */

/* local module debugging */

#define	TKO_INTERVAL_DEFAULT		7200	/* seconds */
#define	TKO_RETRY_INTERVAL_DEFAULT	75	/* seconds */
#define	TKO_RETRY_COUNT_DEFAULT		10

/* We added #ifndef so that these parameters can be
 * over ridden from configuration file if needed
 */
/* Right now we are supporting only one interafce */
#ifndef NUM_INTERFACES_SUPPORTED
#define NUM_INTERFACES_SUPPORTED	1
#endif
/* MAX number of TCP connections supported per interface */
#ifndef MAX_TCP_PER_INTF
#define MAX_TCP_PER_INTF		4
#endif
/* MAX_TCP_INSTANCES should be equal to MAX_TCP_PER_INTF */
#ifndef MAX_TCP_INSTANCES
#define MAX_TCP_INSTANCES		(MAX_TCP_PER_INTF * NUM_INTERFACES_SUPPORTED)
#endif

/* TCP instance */
typedef struct tcp {
	wl_tko_info_t *tko_info;
	uint8 status;
	int8 bsscfg_idx;
	uint8 pad[2];	/* This can be used in case we need to add anything later */
	struct wl_timer *tcp_keepalive_timer;
	uint16 retry_count_left;
	uint16 connect_size;	/* Size of variable length data in wl_tko_connect_t structure */
	wl_tko_connect_t connect;
} tcp_t;

/* TCP keepalive common info */
struct tko_info_cmn {
	bool is_enabled;		/* enabled/disabled by host */
	bool is_suspended;		/* suspended and wake host */
	int8 max_tcp_inst;		/* Total No. of tcp instances */
	int8 max_tcp_per_intf;		/* No. of tcp instances per interface */
	wl_tko_param_t param;		/* common params */
	tcp_t *tcp[MAX_TCP_INSTANCES];	/* TCP instances */
};

/* TCP keepalive info */
struct tko_info {
	wlc_info_t *wlc;
	struct tko_info_cmn *tko_cmn;
};

/* TCP malloc size per instance */
#define TCP_MALLOC_SIZE(connect_size)	\
	(OFFSETOF(tcp_t, connect) + OFFSETOF(wl_tko_connect_t, data) + connect_size)

/* struct access macro */
#define WLCOSH(tko_info)	((tko_info)->wlc->osh)

/* iovar */
enum {
	IOV_TKO
};

static bool tko_is_enabled(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg);

static const bcm_iovar_t tcp_keepalive_iovars[] = {
	{"tko", IOV_TKO, 0, 0, IOVT_BUFFER, OFFSETOF(wl_tko_t, data)},
	{NULL, 0, 0, 0, 0, 0 }
};

#if defined(HOFFLOAD_DLOAD_TKO)
#define HOFFLOAD_TKO_PUBLIC
#else
#define HOFFLOAD_TKO_PUBLIC	static
#endif	/* HOFFLOAD_DLOAD_TKO */

static bool
tko_is_enabled(wl_tko_info_t *tko_info, wlc_bsscfg_t *bsscfg)
{
	ASSERT(bsscfg != NULL);
	return tko_info->tko_cmn->is_enabled;
}

#if defined(HOFFLOAD_TKO) && !defined(HOFFLOAD_DLOAD_TKO)
/* functions specific for TKO module loading */

static int
tko_load_module(wl_tko_info_t *tko_info)
{
#ifdef WLMSG_INFORM
	uint64 tstamp_start, tstamp_end;
#endif

	if (bcm_hoffload_module_status(BCM_HOFFLOAD_MODULE_TKO)
		== BCM_HOFFLOAD_MODULE_DOWNLOADED) {
		/* module already loaded */
		return BCME_OK;
	}

#ifdef WLMSG_INFORM
	tstamp_start = OSL_SYSUPTIME_US();
#endif
	/* synchronous loading */
	if (bcm_hoffload_sync_load_module(BCM_HOFFLOAD_MODULE_TKO) != BCME_OK) {
		return BCME_ERROR;
	}
#ifdef WLMSG_INFORM
	tstamp_end = OSL_SYSUPTIME_US();
#endif
	WL_INFORM(("%s: TKO module loaded: %lu usec\n", __FUNCTION__,
		(long unsigned int)(tstamp_end - tstamp_start)));

	return BCME_OK;
}

static int
tko_unload_module(wl_tko_info_t *tko_info)
{
	if (bcm_hoffload_module_status(BCM_HOFFLOAD_MODULE_TKO)
		== BCM_HOFFLOAD_MODULE_DOWNLOADED) {
		bcm_hoffload_unload_module(BCM_HOFFLOAD_MODULE_TKO);
		WL_INFORM(("%s: TKO module unloaded\n",  __FUNCTION__));
	}
	return BCME_OK;
}
#endif	/* defined(HOFFLOAD_TKO) && !defined(HOFFLOAD_DLOAD_TKO) */

#if !defined(HOFFLOAD_TKO) || defined(HOFFLOAD_DLOAD_TKO)
/* functions to be module loaded if download module feature enabled */

static wl_tko_param_t * tko_get_param(wl_tko_info_t *tko_info, int8 bsscfg_idx);
static tcp_t * tko_get_tcp(wl_tko_info_t *tko_info, int8 idx, uint8 index);
static uint8 tko_get_tcp_status(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg, uint8 index);
static void tko_set_enable_status(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg, uint8 enable);
static void tko_set_suspend_state(wl_tko_info_t *tko_info, uint8 index, bool state);
static void tko_set_tcp(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg, int8 index, tcp_t *tcp);

/* helper functions to extract the IPv4/IPv6 address and
 * keepalive request/response from the connect data
 *   data format:
 *     | src IP | dst IP | KA request | KA response |
 */
static uint8 *
get_src_ip(tcp_t *tcp)
{
	ASSERT(tcp != NULL);
	return &tcp->connect.data[0];
}

static uint8 *
get_dst_ip(tcp_t *tcp)
{
	int addr_len;
	ASSERT(tcp != NULL);
	addr_len = tcp->connect.ip_addr_type ? IPV6_ADDR_LEN : IPV4_ADDR_LEN;
	return &tcp->connect.data[addr_len];
}

static uint8 *
get_keepalive_request(tcp_t *tcp)
{
	int addr_len;
	ASSERT(tcp != NULL);
	addr_len = tcp->connect.ip_addr_type ? IPV6_ADDR_LEN : IPV4_ADDR_LEN;
	return &tcp->connect.data[2 * addr_len];
}

static uint8 *
get_keepalive_response(tcp_t *tcp)
{
	int addr_len;
	ASSERT(tcp != NULL);
	addr_len = tcp->connect.ip_addr_type ? IPV6_ADDR_LEN : IPV4_ADDR_LEN;
	return &tcp->connect.data[2 * addr_len + tcp->connect.request_len];
}

/* find TCP match for specified src/dst IP and src/dst port */
static tcp_t *
find_tcp_match(wl_tko_info_t *tko_info, uint8 ip_addr_type,
	uint8 *sip, uint8 *dip, uint16 sport, uint16 dport)
{
	int i;
	tcp_t *tcp_found = NULL;

	ASSERT(tko_info != NULL);

	for (i = 0; i < tko_info->tko_cmn->max_tcp_inst; i++) {
		tcp_t *tcp = tko_info->tko_cmn->tcp[i];
		int addr_len = ip_addr_type ? IPV6_ADDR_LEN : IPV4_ADDR_LEN;

		if (tcp != NULL &&
			tcp->connect.ip_addr_type == ip_addr_type &&
			memcmp(get_src_ip(tcp), sip, addr_len) == 0 &&
			memcmp(get_dst_ip(tcp), dip, addr_len) == 0 &&
			tcp->connect.local_port == sport &&
			tcp->connect.remote_port == dport) {
			/* found a TCP match */
			tcp_found = tcp;
			break;
		}
	}

	return tcp_found;
}

/* start/restart timer */
static void
tko_start_timer(wl_tko_info_t *tko_info, tcp_t *tcp, uint16 seconds)
{
	ASSERT(tko_info != NULL);
	ASSERT(tcp != NULL);

	WL_INFORM(("%s: tcp[%d] %d seconds\n",	__FUNCTION__, tcp->connect.index, seconds));
	wl_del_timer(tko_info->wlc->wl, tcp->tcp_keepalive_timer);
	wl_add_timer(tko_info->wlc->wl, tcp->tcp_keepalive_timer, seconds * 1000, FALSE);
}

/* stop timer */
static void
tko_stop_timer(wl_tko_info_t *tko_info, tcp_t *tcp)
{
	ASSERT(tko_info != NULL);
	ASSERT(tcp != NULL);
	wl_del_timer(tko_info->wlc->wl, tcp->tcp_keepalive_timer);
}

/* suspend TKO processing and forward packets to wake host */
static void
tko_suspend(wl_tko_info_t *tko_info, tcp_t *tcp, uint8 status)
{
	int i;
	tcp_t *intf_tcp;

	ASSERT(tko_info != NULL);
	ASSERT(tcp != NULL);

	tcp->status = status;

	/* suspend all processing */
	tko_set_suspend_state(tko_info, tcp->bsscfg_idx, TRUE);

	/* stop all TCP timers */
	for (i = 0; i < tko_info->tko_cmn->max_tcp_per_intf; i++) {
		intf_tcp = tko_get_tcp(tko_info, tcp->bsscfg_idx, i);
		if (intf_tcp != NULL) {
			tko_stop_timer(tko_info, intf_tcp);
		}
	}
}

static int
tko_send_packet(wl_tko_info_t *tko_info, void *packet, uint16 packet_len, wlc_bsscfg_t * bsscfg)
{
	void *pkt;
	uint8 *frame;

	ASSERT(tko_info != NULL);
	ASSERT(packet != NULL);
	ASSERT(bsscfg != NULL);

	if (!bsscfg) {
		WL_ERROR(("%s:NULL bsscfg\n", __FUNCTION__));
		return -1;
	}

	if (!(pkt = PKTGET(WLCOSH(tko_info), packet_len, TRUE))) {
		WL_ERROR(("%s: PKTGET failed\n", __FUNCTION__));
		return -1;
	}

	frame = PKTDATA(WLCOSH(tko_info), pkt);
	memcpy(frame, packet, packet_len);
	wlc_sendpkt(bsscfg->wlc, pkt, bsscfg->wlcif);
	return 0;
}

#ifdef TKO_TEST
/* test rx packet handler */
static void
tko_test_rx_packet(wl_tko_info_t *tko_info, void *packet, uint16 packet_len, wlc_bsscfg_t * bsscfg)
{
	void *pkt;
	uint8 *frame;

	ASSERT(tko_info != NULL);
	ASSERT(packet != NULL);

	if (!(pkt = PKTGET(WLCOSH(tko_info), packet_len, TRUE))) {
		WL_ERROR(("%s: PKTGET failed\n", __FUNCTION__));
		return;
	}
	frame = PKTDATA(WLCOSH(tko_info), pkt);
	memcpy(frame, packet, packet_len);

	/* invoke the rx hander with the packet and observe debug output */
	wl_tko_rx(tko_info, pkt, bsscfg);

	PKTFREE(WLCOSH(tko_info), pkt, FALSE);
}
#endif	/* TKO_TEST */

static struct ether_addr *
tko_get_cur_etheraddr(wlc_bsscfg_t * bsscfg)
{
	ASSERT(bsscfg != NULL);
	return &(bsscfg->cur_etheraddr);
}

/* timer callback */
static void
tko_timer_cb(void *arg)
{
	tcp_t *tcp;
	wl_tko_info_t *tko_info;
	struct ether_addr *cur_etheraddr;

	ASSERT(arg != NULL);
	tcp = (tcp_t *)arg;
	tko_info = tcp->tko_info;
	wl_tko_param_t *param;
	wlc_bsscfg_t * bsscfg = tko_info->wlc->bsscfg[tcp->bsscfg_idx];

	WL_INFORM(("%s: tcp[%d] retry=%d\n", __FUNCTION__,
		tcp->connect.index, tcp->retry_count_left));

	if (!wl_tko_is_running(tko_info, bsscfg)) {
		WL_ERROR(("%s: TKO is not running\n", __FUNCTION__));
		return;
	}

	if (tcp->retry_count_left == 0) {
		wl_event_tko_t event_data;

		WL_INFORM(("%s: tcp[%d] no response to keepalive request\n",
			__FUNCTION__, tcp->connect.index));
		tko_suspend(tko_info, tcp, TKO_STATUS_NO_RESPONSE);
		memset(&event_data, 0, sizeof(event_data));
		event_data.index = tcp->connect.index;
		if (bsscfg) {
			cur_etheraddr = tko_get_cur_etheraddr(bsscfg);
			wlc_mac_event(bsscfg->wlc, WLC_E_TKO, cur_etheraddr,
					WLC_E_STATUS_NO_ACK, 0, 0,
					&event_data, sizeof(event_data));
		}
	} else {
		/* send keepalive request */
		tko_send_packet(tko_info, get_keepalive_request(tcp),
				tcp->connect.request_len, bsscfg);
		tcp->retry_count_left--;
		param = tko_get_param(tko_info, tcp->bsscfg_idx);
		tko_start_timer(tko_info, tcp, param->retry_interval);
	}
}

/* create a TCP instance corresponding to an interface with given connect data */
static int
tko_tcp_create(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg,
		wl_tko_connect_t *connect, uint16 connect_size)
{
	uint16 size = TCP_MALLOC_SIZE(connect_size);
	uint8 connect_index;
	tcp_t *tcp;
	wl_tko_param_t *param;

	ASSERT(tko_info != NULL);
	ASSERT(connect != NULL);
	connect_index = connect->index;
	ASSERT(connect_index < tko_info->tko_cmn->max_tcp_per_intf);

	tcp = tko_get_tcp(tko_info, bsscfg->_idx, connect_index);
	if (tcp != NULL) {
		return BCME_ERROR;
	}

	tcp = MALLOC(tko_info->wlc->osh, size);
	if (tcp == NULL) {
		return BCME_NOMEM;
	}
	memset(tcp, 0, size);

	/* initialize TCP instance */
	tcp->tko_info = tko_info;
	tcp->connect_size = connect_size;
	memcpy(&tcp->connect, connect, connect_size);

	/* create timer */
	tcp->tcp_keepalive_timer = wl_init_timer(tko_info->wlc->wl,
		tko_timer_cb, tcp, "tcp keepalive timer");
	if (tcp->tcp_keepalive_timer == NULL) {
		WL_ERROR(("%s: failed to allocate timer\n", __FUNCTION__));
		MFREE(tko_info->wlc->osh, tcp, size);
		return BCME_ERROR;
	}

	param = tko_get_param(tko_info, bsscfg->_idx);
	tcp->status = TKO_STATUS_NORMAL;
	tcp->bsscfg_idx = bsscfg->_idx;
	tcp->retry_count_left = param->retry_count;
	tko_set_tcp(tko_info, bsscfg, connect_index, tcp);
	return BCME_OK;
}

/* destroy a TCP instance corresponding to an interface and connect id */
static void
tko_tcp_destroy(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg, uint8 connect_index)
{
	tcp_t *tcp;

	ASSERT(tko_info != NULL);
	ASSERT(connect_index < tko_info->tko_cmn->max_tcp_per_intf);

	if (bsscfg != NULL) {
		tcp = tko_get_tcp(tko_info, bsscfg->_idx, connect_index);

		if (tcp == NULL) {
			return;
		}

		tko_set_tcp(tko_info, bsscfg, connect_index, NULL);
		if (tcp->tcp_keepalive_timer != NULL) {
			tko_stop_timer(tko_info, tcp);
			wl_free_timer(tko_info->wlc->wl, tcp->tcp_keepalive_timer);
		}

		MFREE(tko_info->wlc->osh, tcp, TCP_MALLOC_SIZE(tcp->connect_size));
	}
}

/* start all TCP instances corresponding to an interface */
static void
tko_start_all_tcp(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg)
{
	int i;
	wl_tko_param_t *param;

	ASSERT(tko_info != NULL);
	ASSERT(bsscfg != NULL);

	tko_set_suspend_state(tko_info, bsscfg->_idx, FALSE);

	for (i = 0; i < tko_info->tko_cmn->max_tcp_per_intf; i++)
	{
		/* We should start tcp related to this interface only. */
		tcp_t *tcp = tko_get_tcp(tko_info, bsscfg->_idx, i);
		if (tcp != NULL) {
			param = tko_get_param(tko_info, bsscfg->_idx);
			tcp->status = TKO_STATUS_NORMAL;
			tcp->retry_count_left = param->retry_count;
			/* send first keepalive request immediately */
			tko_start_timer(tko_info, tcp, 0);
		}
	}
}

/* destroy all TCP instances corresponding to an interface. If bsscfg is
 * NULL then  destroy all TCP instances corresponding to all interfaces.
 */
HOFFLOAD_TKO_PUBLIC void
tko_destroy_all_tcp(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg)
{
	int i;

	ASSERT(tko_info != NULL);

	/* If bsscfg is not NULL then destroy all tcp corresponding to that bss */
	if (bsscfg != NULL) {
		for (i = 0; i < tko_info->tko_cmn->max_tcp_per_intf; i++) {
			tko_tcp_destroy(tko_info, bsscfg, i);
		}
	} else {
	/* If bsscfg is NULL then destro	y all tcp corresponding to all bss */
		for (i = 0; i < tko_info->tko_cmn->max_tcp_inst; i++) {
			tcp_t *tcp = tko_info->tko_cmn->tcp[i];

			if (tcp != NULL) {
				if (tcp->tcp_keepalive_timer != NULL) {
					tko_stop_timer(tko_info, tcp);
					wl_free_timer(tko_info->wlc->wl, tcp->tcp_keepalive_timer);
				}
				MFREE(tko_info->wlc->osh, tcp, TCP_MALLOC_SIZE(tcp->connect_size));
				tko_info->tko_cmn->tcp[i] = NULL;
			}
		}
	}
}

static wl_tko_param_t *
tko_get_param(wl_tko_info_t *tko_info, int8 bsscfg_idx)
{
	/* Return the params corresponding to the index of this bsscfg
	 * when separate parameter configuration is supported for
	 * different interfaces
	 */
	return &tko_info->tko_cmn->param;
}

static tcp_t *
tko_get_tcp(wl_tko_info_t *tko_info, int8 bsscfg_idx, uint8 connect_index)
{
	tcp_t *tcp;
	int8 i;

	/* Get the tcp corresponding to the interface of this
	 * bsscfg_idx and connect index passed as argument
	 */
	for (i = 0; i < tko_info->tko_cmn->max_tcp_inst; i++) {
		tcp = tko_info->tko_cmn->tcp[i];
		if (tcp && (tcp->bsscfg_idx == bsscfg_idx) &&
				(tcp->connect.index == connect_index)) {
			return tcp;
		}
	}
	return NULL;
}

/* Before freeing tcp we should set the tcp pointer to NULL in the tcp array. */
static void
tko_set_tcp(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg, int8 connect_idx, tcp_t *tcp_in)
{
	tcp_t *tcp;
	int8 i;

	/* If tcp == NULL, then search the tcp array to match the
	 * bsscfg->_idx, connect_idx and set it to NULL. Else set
	 * the tcp wherever it is free in tcp array.
	 */

	for (i = 0; i < tko_info->tko_cmn->max_tcp_inst; i++) {
		tcp = tko_info->tko_cmn->tcp[i];
		if (tcp_in != NULL && tcp == NULL) {
			tko_info->tko_cmn->tcp[i] = tcp_in;
			break;
		} else {
			if (tcp_in == NULL && tcp && (tcp->bsscfg_idx == bsscfg->_idx) &&
					(tcp->connect.index == connect_idx)) {
				tko_info->tko_cmn->tcp[i] = NULL;
				break;
			}
		}
	}
	return;
}

static void
tko_set_suspend_state(wl_tko_info_t *tko_info, uint8 bsscfg_idx, bool state)
{
	tko_info->tko_cmn->is_suspended = state;
	return;
}

static uint8
tko_get_tcp_status(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg, uint8 index)
{
	tcp_t *tcp;
	ASSERT(bsscfg != NULL);

	tcp = tko_get_tcp(tko_info, bsscfg->_idx, index);
	return (tcp == NULL) ?
			TKO_STATUS_UNAVAILABLE : tcp->status;
}

static void
tko_set_enable_status(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg, uint8 enable)
{
	ASSERT(bsscfg != NULL);
	tko_info->tko_cmn->is_enabled = enable;
	return;
}

/* TCP keepalive GET iovar */
HOFFLOAD_TKO_PUBLIC int
tko_get(wl_tko_info_t *tko_info, void *p, uint plen, void *a, int alen, wlc_bsscfg_t * bsscfg)
{
	int err = BCME_OK;
	wl_tko_t *tko;
	wl_tko_t *tko_out;

	ASSERT(tko_info != NULL);
	ASSERT(p != NULL);
	ASSERT(a != NULL);
	ASSERT(bsscfg != NULL);
	tko = p;
	tko_out = a;

	/* verify length */
	if (plen < OFFSETOF(wl_tko_t, data) ||
		tko->len > plen - OFFSETOF(wl_tko_t, data)) {
		return BCME_BUFTOOSHORT;
	}

	/* copy subcommand to output */
	tko_out->subcmd_id = tko->subcmd_id;

	/* process subcommand */
	switch (tko->subcmd_id) {
	case WL_TKO_SUBCMD_MAX_TCP:
	{
		wl_tko_max_tcp_t *max_tcp =
			(wl_tko_max_tcp_t *)tko_out->data;
		tko_out->len = sizeof(*max_tcp);
		max_tcp->max = tko_info->tko_cmn->max_tcp_inst;
		break;
	}
	case WL_TKO_SUBCMD_PARAM:
	{
		wl_tko_param_t *param =
			(wl_tko_param_t *)tko_out->data;
		tko_out->len = sizeof(*param);
		memcpy(param, tko_get_param(tko_info, bsscfg->_idx), sizeof(*param));
		break;
	}
	case WL_TKO_SUBCMD_CONNECT:
	{
		wl_tko_get_connect_t *get_connect = (wl_tko_get_connect_t *)tko->data;
		uint8 index = get_connect->index;
		wl_tko_connect_t *connect = (wl_tko_connect_t *)tko_out->data;

		if (index < tko_info->tko_cmn->max_tcp_per_intf) {
			tcp_t *tcp = tko_get_tcp(tko_info, bsscfg->_idx, index);

			if (tcp != NULL) {
				tko_out->len = tcp->connect_size;
				memcpy(connect, &tcp->connect,
					tcp->connect_size);
			} else {
				err = BCME_NOTFOUND;
			}
		} else {
			err = BCME_RANGE;
		}
		break;
	}
	case WL_TKO_SUBCMD_ENABLE:
	{
		wl_tko_enable_t *tko_enable = (wl_tko_enable_t *)tko_out->data;
		tko_out->len = sizeof(*tko_enable);
		tko_enable->enable = tko_is_enabled(tko_info, bsscfg);
		break;
	}
	case WL_TKO_SUBCMD_STATUS:
	{
		wl_tko_status_t *tko_status = (wl_tko_status_t *)tko_out->data;
		uint8 i;

		tko_out->len = OFFSETOF(wl_tko_status_t, status) +
				tko_info->tko_cmn->max_tcp_per_intf;
		tko_status->count = tko_info->tko_cmn->max_tcp_per_intf;

		for (i = 0; i < tko_info->tko_cmn->max_tcp_per_intf; i++) {
			tko_status->status[i] = tko_get_tcp_status(tko_info, bsscfg, i);
		}
		break;
	}
	default:
		err = BCME_UNSUPPORTED;
		break;
	}
	return err;
}

/* TCP keepalive SET iovar */
HOFFLOAD_TKO_PUBLIC int
tko_set(wl_tko_info_t *tko_info, void *a, int alen, wlc_bsscfg_t * bsscfg)
{
	int err = BCME_OK;
	wl_tko_t *tko;

	ASSERT(tko_info != NULL);
	ASSERT(a != NULL);
	tko = a;

	/* verify length */
	if (alen < OFFSETOF(wl_tko_t, data) ||
		tko->len > alen - OFFSETOF(wl_tko_t, data)) {
		return BCME_BUFTOOSHORT;
	}

	/* process subcommand */
	switch (tko->subcmd_id) {
	case WL_TKO_SUBCMD_PARAM:
	{
		wl_tko_param_t *param = (wl_tko_param_t *)tko->data;
		if (tko->len >= sizeof(*param)) {
			if (!tko_is_enabled(tko_info, bsscfg)) {
				memcpy(tko_get_param(tko_info, bsscfg->_idx),
						param, sizeof(*param));
			} else {
				err = BCME_BUSY;
			}
		} else  {
			err = BCME_BADLEN;
		}
		break;
	}
	case WL_TKO_SUBCMD_CONNECT:
	{
		wl_tko_connect_t *connect = (wl_tko_connect_t *)tko->data;

		if (connect->index < tko_info->tko_cmn->max_tcp_per_intf) {
			if (!tko_is_enabled(tko_info, bsscfg)) {
				/* delete previous instance before creating new */
				tko_tcp_destroy(tko_info, bsscfg, connect->index);
				err = tko_tcp_create(tko_info, bsscfg, connect, tko->len);
			} else {
				err = BCME_BUSY;
			}
		} else  {
			err = BCME_RANGE;
		}
		break;
	}
	case WL_TKO_SUBCMD_ENABLE:
	{
		wl_tko_enable_t *tko_enable = (wl_tko_enable_t *)tko->data;
		if (tko->len >= sizeof(*tko_enable)) {
			if (tko_enable->enable != tko_is_enabled(tko_info, bsscfg)) {
				tko_set_enable_status(tko_info, bsscfg, tko_enable->enable);

				if (tko_is_enabled(tko_info, bsscfg)) {
					tko_start_all_tcp(tko_info, bsscfg);
#ifdef TKO_TEST
					{
						tcp_t *tcp = tko_info->tko_cmn->
								tcp[tko_info->tko_cmn->
									max_tcp_inst - 1];
						if (tcp != NULL) {
							tko_test_rx_packet(tko_info,
									get_keepalive_response(tcp),
									tcp->connect.response_len,
									bsscfg);
						}
					}
#endif	/* TKO_TEST */
				} else {
					tko_destroy_all_tcp(tko_info, bsscfg);
				}
			}
		} else  {
			err = BCME_BADLEN;
		}
		break;
	}
	default:
		err = BCME_UNSUPPORTED;
		break;
	}
	return err;
}

/* returns TRUE if the packet is consumed else FALSE */
HOFFLOAD_TKO_PUBLIC bool
tko_rx_proc(wl_tko_info_t *tko_info, void *sdu)
{
	/* 802.3 llc/snap header */
	static const uint8 llc_snap_hdr[SNAP_HDR_LEN] =
		{0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
	uint8 *frame, *end;
	int length;
	uint8 *pt;
	uint16 ethertype;
	struct bcmtcp_hdr *tcp_hdr = NULL;
	uint16 tcp_len = 0;
	uint8 ip_addr_type = 0;
	uint8 *rx_src_ip = NULL;
	uint8 *rx_dst_ip = NULL;

	ASSERT(tko_info != NULL);
	ASSERT(sdu != NULL);
	frame = PKTDATA(WLCOSH(tko_info), sdu);
	length = PKTLEN(WLCOSH(tko_info), sdu);
	end = frame + length;

	/* process Ethernet II or SNAP-encapsulated 802.3 frames */
	if (length < ETHER_HDR_LEN) {
		WL_INFORM(("%s: short eth frame (%d)\n", __FUNCTION__, length));
		return FALSE;
	} else if (ntoh16_ua((const void *)(frame + ETHER_TYPE_OFFSET)) >= ETHER_TYPE_MIN) {
		/* Frame is Ethernet II */
		pt = frame + ETHER_TYPE_OFFSET;
	} else if (length >= ETHER_HDR_LEN + SNAP_HDR_LEN + ETHER_TYPE_LEN &&
	           !memcmp(llc_snap_hdr, frame + ETHER_HDR_LEN, SNAP_HDR_LEN)) {
		WL_INFORM(("%s: 802.3 LLC/SNAP\n", __FUNCTION__));
		pt = frame + ETHER_HDR_LEN + SNAP_HDR_LEN;
	} else {
		return FALSE;
	}

	ethertype = ntoh16_ua((const void *)pt);
	pt += ETHER_TYPE_LEN;

	/* check ethertype */
	if ((ethertype != ETHER_TYPE_IP) && (ethertype != ETHER_TYPE_IPV6)) {
		if (ethertype == ETHER_TYPE_8021Q) {
			WL_INFORM(("%s: dropped 802.1Q packet\n", __FUNCTION__));
		}
		return FALSE;
	}

	/* check for IP */
	if (ethertype == ETHER_TYPE_IP) {
		struct ipv4_hdr *ipv4_hdr;
		uint16 iphdr_len;
		uint16 cksum;

		ipv4_hdr = (struct ipv4_hdr *)pt;
		iphdr_len = IPV4_HLEN(ipv4_hdr);
		if (pt + iphdr_len > end) {
			WL_INFORM(("%s: too short for IP header\n", __FUNCTION__));
			return FALSE;
		}
		pt += iphdr_len;

		if (ipv4_hdr->prot != IP_PROT_TCP) {
			return FALSE;
		}

		if (ntoh16(ipv4_hdr->frag) & IPV4_FRAG_OFFSET_MASK) {
			WL_INFORM(("%s: fragmented packet, not first fragment\n", __FUNCTION__));
			return FALSE;
		}

		if (ntoh16(ipv4_hdr->frag) & IPV4_FRAG_MORE) {
			WL_INFORM(("%s: fragmented packet, more bit set\n", __FUNCTION__));
			return FALSE;
		}

		tcp_len = ntoh16(ipv4_hdr->tot_len) - iphdr_len;
		if (pt + tcp_len > end) {
			WL_INFORM(("%s: too short for TCP header\n", __FUNCTION__));
			return FALSE;
		}
		tcp_hdr = (struct bcmtcp_hdr *)pt;

		/* verify IP header checksum */
		cksum = ipv4_hdr_cksum((uint8 *)ipv4_hdr, iphdr_len);
		if (cksum != ipv4_hdr->hdr_chksum) {
			WL_INFORM(("%s: bad IP header checksum: %x %x\n", __FUNCTION__,
				cksum, ipv4_hdr->hdr_chksum));
			return FALSE;
		}

		/* verify TCP header checksum */
		cksum = ipv4_tcp_hdr_cksum((uint8 *)ipv4_hdr, (uint8 *)tcp_hdr, tcp_len);
		if (cksum != tcp_hdr->chksum) {
			WL_INFORM(("%s: bad TCP header checksum: %x %x\n", __FUNCTION__,
				cksum, tcp_hdr->chksum));
			return FALSE;
		}

		ip_addr_type = 0;
		rx_src_ip = ipv4_hdr->src_ip;
		rx_dst_ip = ipv4_hdr->dst_ip;
	}
	else if (ethertype == ETHER_TYPE_IPV6) {
		struct ipv6_hdr *ipv6_hdr;
		uint16 cksum;

		if (pt + sizeof(struct ipv6_hdr) > end) {
			WL_INFORM(("%s: too short for IPv6 header\n", __FUNCTION__));
			return FALSE;
		}
		ipv6_hdr = (struct ipv6_hdr *)pt;
		pt += sizeof(struct ipv6_hdr);

		if (ipv6_hdr->nexthdr != IP_PROT_TCP) {
			return FALSE;
		}

		tcp_len = ntoh16(ipv6_hdr->payload_len);
		if (pt + tcp_len > end) {
			WL_INFORM(("%s: too short for IPv6 TCP header\n", __FUNCTION__));
			return FALSE;
		}
		tcp_hdr = (struct bcmtcp_hdr *)pt;

		/* verify TCP checksum */
		cksum = ipv6_tcp_hdr_cksum((uint8 *)ipv6_hdr, (uint8 *)tcp_hdr, tcp_len);
		if (cksum != tcp_hdr->chksum) {
			WL_INFORM(("%s: bad IPv6 TCP header checksum: %x %x\n", __FUNCTION__,
				cksum, tcp_hdr->chksum));
			return FALSE;
		}

		ip_addr_type = 1;
		rx_src_ip = ipv6_hdr->saddr.addr;
		rx_dst_ip = ipv6_hdr->daddr.addr;
	}

	/* common TCP keepalive processing */
	if (rx_src_ip != NULL && rx_dst_ip != NULL && tcp_hdr != NULL) {
		uint16 rx_src_port = ntoh16(tcp_hdr->src_port);
		uint16 rx_dst_port = ntoh16(tcp_hdr->dst_port);
		uint32 rx_ack_num = ntoh32(tcp_hdr->ack_num);
		uint32 rx_seq_num = ntoh32(tcp_hdr->seq_num);
		tcp_t *tcp;
		uint8 tcp_flags[2];
		uint16 tcp_hdr_len;

		/* find matching TCP */
		tcp = find_tcp_match(tko_info, ip_addr_type, rx_dst_ip, rx_src_ip,
			rx_dst_port, rx_src_port);
		if (tcp == NULL) {
			if (ip_addr_type) {
				WL_INFORM(("%s: TCP not found - "
				"sIP=%x%02x:%x%02x:%x%02x:%x%02x:"
				"%x%02x:%x%02x:%x%02x:%x%02x "
				"dIP=%x%02x:%x%02x:%x%02x:%x%02x:"
				"%x%02x:%x%02x:%x%02x:%x%02x "
				"sport=%u dport=%u\n",
				__FUNCTION__,
				rx_dst_ip[0], rx_dst_ip[1], rx_dst_ip[2], rx_dst_ip[3],
				rx_dst_ip[4], rx_dst_ip[5], rx_dst_ip[6], rx_dst_ip[7],
				rx_dst_ip[8], rx_dst_ip[9], rx_dst_ip[10], rx_dst_ip[11],
				rx_dst_ip[12], rx_dst_ip[13], rx_dst_ip[14], rx_dst_ip[15],
				rx_src_ip[0], rx_src_ip[1], rx_src_ip[2], rx_src_ip[3],
				rx_src_ip[4], rx_src_ip[5], rx_src_ip[6], rx_src_ip[7],
				rx_src_ip[8], rx_src_ip[9], rx_src_ip[10], rx_src_ip[11],
				rx_src_ip[12], rx_src_ip[13], rx_src_ip[14], rx_src_ip[15],
				rx_dst_port, rx_src_port));
			} else {
				WL_INFORM(("%s: TCP not found - "
				"sIP=%u.%u.%u.%u dIP=%u.%u.%u.%u "
				"sport=%u dport=%u\n",
				__FUNCTION__,
				rx_dst_ip[0], rx_dst_ip[1], rx_dst_ip[2], rx_dst_ip[3],
				rx_src_ip[0], rx_src_ip[1], rx_src_ip[2], rx_src_ip[3],
				rx_dst_port, rx_src_port));
			}
			return FALSE;
		}

		/* get the  TCP flags */
		memcpy(tcp_flags, &tcp_hdr->hdrlen_rsvd_flags, sizeof(tcp_flags));

		/* check if ACK flag set */
		if ((tcp_flags[1] & TCP_FLAG_ACK) != TCP_FLAG_ACK) {
			WL_INFORM(("%s: tcp[%d] ACK flag not set: 0x%02x\n", __FUNCTION__,
			tcp->connect.index, tcp_flags[1]));
			tko_suspend(tko_info, tcp, TKO_STATUS_NO_TCP_ACK_FLAG);
			return FALSE;
		}

		/* TCP header length in bytes */
		tcp_hdr_len = TCP_HDRLEN(tcp_flags[0]) << 2;

		/* check for data */
		if (tcp_len != tcp_hdr_len) {
			WL_INFORM(("%s: tcp[%d], TCP data length: %d\n", __FUNCTION__,
				tcp->connect.index, tcp_len - tcp_hdr_len));
			tko_suspend(tko_info, tcp, TKO_STATUS_TCP_DATA);
			return FALSE;
		}

		/* check for any flag other than ACK */
		if (tcp_flags[1] != TCP_FLAG_ACK) {
			WL_INFORM(("%s: tcp[%d] unexpected TCP flags: 0x%02x\n", __FUNCTION__,
				tcp->connect.index, tcp_flags[1]));
			tko_suspend(tko_info, tcp, TKO_STATUS_UNEXPECT_TCP_FLAG);
			return FALSE;
		}

		/* check for TCP keepalive response */
		if (tcp->connect.local_seq == rx_ack_num &&
			tcp->connect.remote_seq == rx_seq_num) {
			wl_tko_param_t *param = tko_get_param(tko_info, tcp->bsscfg_idx);
			WL_INFORM(("%s: tcp[%d] ACK rx'ed for keepalive request\n", __FUNCTION__,
				tcp->connect.index));
			tcp->retry_count_left = param->retry_count;
			tko_start_timer(tko_info, tcp, param->interval);
			return TRUE;
		}

		/* check for TCP keepalive request */
		if (tcp->connect.local_seq == rx_ack_num &&
			tcp->connect.remote_seq > rx_seq_num) {
			wl_tko_param_t *param = tko_get_param(tko_info, tcp->bsscfg_idx);
			WL_INFORM(("%s: tcp[%d] keepalive request rx'ed, sending ACK\n",
				__FUNCTION__, tcp->connect.index));
			tko_send_packet(tko_info, get_keepalive_response(tcp),
				tcp->connect.response_len, tko_info->wlc->bsscfg[tcp->bsscfg_idx]);
			tcp->retry_count_left = param->retry_count;
			tko_start_timer(tko_info, tcp, param->interval);

			return TRUE;
		}

		/* check for invalid sequence number */
		if (tcp->connect.local_seq != rx_ack_num) {
			WL_INFORM(("%s: tcp[%d] seq num invalid %u\n", __FUNCTION__,
				tcp->connect.index, rx_ack_num));
			tko_suspend(tko_info, tcp, TKO_STATUS_SEQ_NUM_INVALID);
			return FALSE;
		}

		/* check for invalid remote sequence number */
		if (tcp->connect.remote_seq != rx_seq_num) {
			WL_INFORM(("%s: tcp[%d] remote seq num invalid %u\n", __FUNCTION__,
				tcp->connect.index, rx_seq_num));
			tko_suspend(tko_info, tcp, TKO_STATUS_REMOTE_SEQ_NUM_INVALID);
			return FALSE;
		}
	}

	return FALSE;
}
#endif	/* !defined(HOFFLOAD_TKO) || defined(HOFFLOAD_DLOAD_TKO) */

#ifndef HOFFLOAD_DLOAD_TKO
/* functions to always reside in FW even if download module feature enabled */

static int
tko_doiovar(void *hdl, uint32 actionid, void *p, uint plen, void *a, uint alen,
	uint vsize, struct wlc_if *wlcif)
{
	int err = BCME_OK;
	wl_tko_info_t *tko_info;
#ifdef HOFFLOAD_TKO
	bool is_enabled;
#endif	/* HOFFLOAD_TKO */
	wlc_bsscfg_t * bsscfg = NULL;

	ASSERT(hdl);
	tko_info = hdl;
	bsscfg = wlc_bsscfg_find_by_wlcif(tko_info->wlc, wlcif);

#ifdef HOFFLOAD_TKO
	/* load TKO module if not loaded */
	if (tko_load_module(tko_info) != BCME_OK) {
		/* fatal if TKO module not loaded */
		OSL_SYS_HALT();
	}
	is_enabled = tko_is_enabled(tko_info, bsscfg);
#endif	/* HOFFLOAD_TKO */

	switch (actionid) {
	case IOV_GVAL(IOV_TKO):
		err = tko_get(tko_info, p, plen, a, alen, bsscfg);
		break;
	case IOV_SVAL(IOV_TKO):
		err = tko_set(tko_info, a, alen, bsscfg);
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

#ifdef HOFFLOAD_TKO
	/* unload TKO module if TKO has just been disabled */
	if (is_enabled && !tko_is_enabled(tko_info, bsscfg)) {
		tko_unload_module(tko_info);
	}
#endif	/* HOFFLOAD_TKO */

	return err;
}

static bool
tko_is_suspended(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg)
{
	ASSERT(bsscfg != NULL);
	return tko_info->tko_cmn->is_suspended;
}

static void
BCMATTACHFN(tko_init_default_param)(wl_tko_info_t *tko_info)
{
	/* We should init default params corresponding to all the interfaces
	 * when different params needs to be supported for different interfaces.
	 */
	tko_info->tko_cmn->param.interval = TKO_INTERVAL_DEFAULT;
	tko_info->tko_cmn->param.retry_interval = TKO_RETRY_INTERVAL_DEFAULT;
	tko_info->tko_cmn->param.retry_count = TKO_RETRY_COUNT_DEFAULT;
	return;
}

wl_tko_info_t *
BCMATTACHFN(wl_tko_attach)(wlc_info_t *wlc)
{
	wl_tko_info_t *tko_info;

	/* Allocate packet filter private info struct. */
	tko_info = MALLOCZ(wlc->osh, sizeof(*tko_info));
	if (tko_info == NULL) {
		WL_ERROR(("%s: MALLOC failed; total mallocs %d bytes\n",
			__FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	/* OBJECT REGISTRY: check if shared tko common info is already malloced */
	tko_info->tko_cmn = (struct tko_info_cmn *)
		obj_registry_get(wlc->objr, OBJR_TKO_CMN);

	if (tko_info->tko_cmn == NULL) {
		if ((tko_info->tko_cmn =  (struct tko_info_cmn *) MALLOCZ(wlc->osh,
			sizeof(*tko_info->tko_cmn))) == NULL) {
			WL_ERROR(("wl%d: %s: tko_info_cmn alloc failed\n",
				WLCWLUNIT(wlc), __FUNCTION__));
			goto fail;
		}
		/* OBJECT REGISTRY: We are the first instance, store value for key */
		obj_registry_set(wlc->objr, OBJR_TKO_CMN, tko_info->tko_cmn);
	}
	BCM_REFERENCE(obj_registry_ref(wlc->objr, OBJR_TKO_CMN));

	tko_init_default_param(tko_info);
	tko_info->tko_cmn->max_tcp_inst = MAX_TCP_INSTANCES;
	tko_info->tko_cmn->max_tcp_per_intf = MAX_TCP_PER_INTF;
	tko_info->wlc = wlc;

	/* Register this module. */
	if (wlc_module_register(wlc->pub, tcp_keepalive_iovars, "tko", tko_info,
		tko_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("%s: wlc_module_register failed\n", __FUNCTION__));
		goto fail;
	}

	wlc->pub->_tko = TRUE;
	return tko_info;

fail:
	if (tko_info != NULL) {
		if (tko_info->tko_cmn != NULL) {
			if (obj_registry_unref(tko_info->wlc->objr, OBJR_TKO_CMN) == 0) {
				obj_registry_set(tko_info->wlc->objr, OBJR_TKO_CMN, NULL);
				MFREE(tko_info->wlc->osh, tko_info->tko_cmn,
						sizeof(*tko_info->tko_cmn));
			}
		}

		MFREE(wlc->osh, tko_info, sizeof(*tko_info));
	}
	return NULL;
}

void
BCMATTACHFN(wl_tko_detach)(wl_tko_info_t *tko_info)
{
	WL_INFORM(("wl_tko_detach\n"));

	if (!tko_info) {
		return;
	}

	tko_destroy_all_tcp(tko_info, NULL);
	wlc_module_unregister(tko_info->wlc->pub, "tko", tko_info);
	tko_info->wlc->pub->_tko = FALSE;

	if (obj_registry_unref(tko_info->wlc->objr, OBJR_TKO_CMN) == 0) {
			obj_registry_set(tko_info->wlc->objr, OBJR_TKO_CMN, NULL);
			MFREE(tko_info->wlc->osh, tko_info->tko_cmn, sizeof(struct tko_info_cmn));
	}

	MFREE(WLCOSH(tko_info), tko_info, sizeof(wl_tko_info_t));
}

/* returns TRUE if the packet is consumed else FALSE */
bool
wl_tko_rx(wl_tko_info_t *tko_info, void *sdu, wlc_bsscfg_t * bsscfg)
{
	ASSERT(tko_info != NULL);
	ASSERT(bsscfg != NULL);

	if (!wl_tko_is_running(tko_info, bsscfg)) {
		return FALSE;
	}

	return tko_rx_proc(tko_info, sdu);
}

bool wl_tko_is_running(wl_tko_info_t *tko_info, wlc_bsscfg_t * bsscfg)
{
	ASSERT(tko_info != NULL);
	ASSERT(bsscfg != NULL);
	return (tko_is_enabled(tko_info, bsscfg) && !tko_is_suspended(tko_info, bsscfg));
}
#endif	/* HOFFLOAD_DLOAD_TKO */
