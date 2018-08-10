/*
 * Broadcom steering implementation which includes BSS transition, block and unblock MAC
 *
 * Copyright (C) 2018, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: bcm_steering.c 707700 2017-06-28 13:06:23Z $
 */

#include <typedefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <bcmparams.h>
#include <bcmtimer.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <netconf.h>
#include <nvparse.h>
#include <shutils.h>
#include <wlutils.h>

#include "bcm_steering.h"

#define WL_WLIF_BUFSIZE_1K	1024
#define WL_WLIF_BUFSIZE_4K	4096
#define WL_MACMODE_MASK		0x6000
#define WL_MACPROBE_MASK	0x1000
#define WL_MACMODE_SHIFT	13
#define WL_MACPROBE_SHIFT	12
/* Static info set */
#define WL_SINFOSET_SHIFT	15

#define WL_MAC_PROB_MODE_SET(m, p, f)	((m << WL_MACMODE_SHIFT) | (p << WL_MACPROBE_SHIFT) \
		| (f << WL_SINFOSET_SHIFT))
#define	WL_MACMODE_GET(f)	(f & WL_MACMODE_MASK >> WL_MACMODE_SHIFT)
#define	WL_MACPROBE_GET(f)	(f & WL_MACPROBE_MASK >> WL_MACPROBE_SHIFT)
#define TIMER_MAX_COUNT	16
#define WL_MAC_ADD	1
#define WL_MAC_DEL	0

#define SEC_TO_MICROSEC(x)	((x) * 1000 * 1000)

/* For debug prints */
#define STEERING_DEBUG_ERROR		0x0001

/* NVRAM names */
#define WLIFU_NVRAM_STEERING_MSGLEVEL	"steering_msglevel"
#define WLIFU_NVRAM_STEER_FLAGS		"steer_flags" /* NVRAM for steering flags */
#define WLIFU_NVRAM_TM_DEFIANT_WD	"steer_tm_defiant_wd" /* NVRAM for Dafiant WD Timer Tick */
#define WLIFU_NVRAM_STEER_RESP_TIMEOUT	"steer_resp_timeout"	/* BSS Trans Response timoeut */

/* Default Value of NVRAM for Steer Flags */
#define WLIFU_DEF_STEER_FLAGS		0x00
#define WLIFU_DEF_TM_DEFIANT_WD		5	/* NVRAM Default value of Dafiant WD Timer Tick */
#define WLIFU_DEF_STEER_RESP_TIMEOUT	5	/* NVRAM Default value of Steer Response */

/* NVRAM for Steer Flags, Mask Values */		/* Bit	Purpose */
#define STEER_FLAGS_MASK_BTR_UNWN	0x0001	/* 1 BSSTrans Resp UNKNOWN, Block Brute Force */
#define STEER_FLAGS_MASK_BTR_RJCT	0x0002	/* 2 BSSTrans Resp REJECT, Apply Brute Force */
#define STEER_FLAGS_MASK_BTR_ACPT	0x0004	/* 3 BSSTrans Resp ACCEPT, Enbl Defiant Watchdog */

/* For Individual BSS Transition Response types, Get Brute Force Steer Behavior */
#define BTR_UNWN_BRUTFORCE_BLK(steer_flags) ((steer_flags) & STEER_FLAGS_MASK_BTR_UNWN)
#define BTR_RJCT_BRUTFORCE(steer_flags) ((steer_flags) & STEER_FLAGS_MASK_BTR_RJCT)

/* For BSS Transition Response ACCEPT, Check if Dafiant STA Watchdog is required or not */
#define BTR_ACPT_DEFIANT_WD_ON(steer_flags) ((steer_flags) & STEER_FLAGS_MASK_BTR_ACPT)

/* to print message level */
unsigned int g_steering_msglevel;

#define STEERING_PRINT(fmt, arg...) \
	printf("WLIF_BSSTRANS >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg)

#define WLIF_BSSTRANS(fmt, arg...) \
	do { if (g_steering_msglevel & STEERING_DEBUG_ERROR) \
		STEERING_PRINT(fmt, ##arg); } while (0)

/* IOCTL swapping mode for Big Endian host with Little Endian dongle. */
/* The below macros handle endian mis-matches between host and dongle. */
extern bool gg_swap; /* Swap variable set by wl_endian_probe */
#define htod32(i) (gg_swap ? bcmswap32(i) : (uint32)(i))
#define htod16(i) (gg_swap ? bcmswap16(i) : (uint16)(i))
#define dtoh32(i) (gg_swap ? bcmswap32(i) : (uint32)(i))

/*
 * Struct for wlif handle data.
 * and app specific data.
 */
typedef struct wl_wlif_hdl_data {
	char ifname[IFNAMSIZ];		/* Interface name. */
	uint32 resp_timeout;		/* BSS Transition response timeout(In Seconds) */
	callback_hndlr ioctl_hndlr;	/* Ioctl call handler. */
	void *uschd_hdl;		/* Uschd lib handle. */
	bcm_timer_module_id timer_hdl;	/* Linux timer handle. */
	void *data;			/* App specific data. */
} wl_wlif_hdl_data_t;

/*
 * Flag Bit Values
 * 0-0	: Static info set
 * 1-1	: Probe Mode
 * 2-3	: Mac mode
 * 4-7	: Reserved
 * 8-15 : Static mac list count
 */
typedef struct static_maclist_ {
	wl_wlif_hdl_data_t *hdl_data;
	short flag;
	struct ether_addr addr;		/* Sta addr. */
	int timeout;			/* Timeout to clear mac from maclist. */
} static_maclist_t;

/**
 * Callback Function pointer to be called, when linux timer is created
 *
 * @param timer	Data recieved from socket. Which will be passed to this function
 *
 * @param data	Timer data passed when timer is created
 */
typedef void wlif_linux_timer_cb(bcm_timer_id timer, void *data);

/**
 * Callback Function pointer to be called, when usched timer is created
 *
 * @param hdl	usched handle passed when timer is created
 *
 * @param data	Timer data passed when timer is created
 */
typedef void wlif_usched_timer_cb(bcm_usched_handle* hdl, void *data);

/* Steering lib default ioctl handler routine. */
static int wl_wlif_do_ioctl(char *ifname, int cmd, void *buf, int buflen,
        void *data);

/* Counter to match the response for BSS transition request */
static uint8 bss_token = 0;

/* Create Unblock MAC Timer */
static int wl_wlif_create_unblock_mac_timer(wl_wlif_hdl_data_t *hdl_data,
	struct ether_addr addr, int timeout, int flag);
/* Create Defiant STA Watchdog Timer */
static int wl_wlif_create_defiant_sta_watchdog_timer(wl_wlif_hdl_data_t *hdl_data,
	struct ether_addr addr, int timeout);

#ifndef BCM_EVENT_HEADER_LEN
#define BCM_EVENT_HEADER_LEN   (sizeof(bcm_event_t))
#endif

/* listen to sockets for bss response event */
static int
wl_wlif_proc_event_socket(int event_fd, struct timeval *tv, char *ifreq, uint8 token,
	struct ether_addr *bssid)
{
	fd_set fdset;
	int fdmax;
	int width, status = 0, bytes;
	char buf_ptr[WL_WLIF_BUFSIZE_1K], *pkt;
	char ifname[IFNAMSIZ+1];
	int pdata_len;
	dot11_bsstrans_resp_t *bsstrans_resp;
	dot11_neighbor_rep_ie_t *neighbor;
	bcm_event_t *dpkt;
	uint32 event_id;
	struct ether_addr *addr;

	if (bssid == NULL) {
		WLIF_BSSTRANS("bssid is NULL\n");
		return WLIFU_BSS_TRANS_RESP_UNKNOWN;
	}

	WLIF_BSSTRANS("For event_fd[%d] ifname[%s] bss_token[%d] bssid["MACF"]\n",
		event_fd, ifreq, token, ETHERP_TO_MACF(bssid));

	while (1) {
		pkt = buf_ptr;

		/* init file descriptor set */
		FD_ZERO(&fdset);
		fdmax = -1;

		/* build file descriptor set now to save time later */
		if (event_fd != -1) {
			FD_SET(event_fd, &fdset);
			fdmax = event_fd;
		}

		width = fdmax + 1;

		/* listen to data availible on all sockets */
		status = select(width, &fdset, NULL, NULL, tv);

		if ((status == -1 && errno == EINTR) || (status == 0)) {
			WLIF_BSSTRANS("ifname[%s] status[%d] errno[%d]- No event\n",
				ifreq, status, errno);
			return WLIFU_BSS_TRANS_RESP_UNKNOWN;
		}

		if (status <= 0) {
			WLIF_BSSTRANS("ifname[%s]: err from select: %s\n", ifreq, strerror(errno));
			return WLIFU_BSS_TRANS_RESP_UNKNOWN;
		}

		/* handle brcm event */
		if (event_fd !=  -1 && FD_ISSET(event_fd, &fdset)) {
			memset(pkt, 0, sizeof(buf_ptr));
			if ((bytes = recv(event_fd, pkt, sizeof(buf_ptr), 0)) <= IFNAMSIZ) {
				WLIF_BSSTRANS("ifname[%s] BSS Transit Response: recv err\n", ifreq);
				return WLIFU_BSS_TRANS_RESP_UNKNOWN;
			}

			strncpy(ifname, pkt, IFNAMSIZ);
			ifname[IFNAMSIZ] = '\0';

			pkt = pkt + IFNAMSIZ;
			pdata_len = bytes - IFNAMSIZ;

			if (pdata_len <= BCM_EVENT_HEADER_LEN) {
				WLIF_BSSTRANS("ifname[%s] BSS Transit Response: "
					"data_len %d too small\n", ifreq, pdata_len);
				continue;
			}

			dpkt = (bcm_event_t *)pkt;
			event_id = ntohl(dpkt->event.event_type);
			if (event_id != WLC_E_BSSTRANS_RESP) {
				WLIF_BSSTRANS("ifname[%s] BSS Transit Response: "
					"wrong event_id %d\n", ifreq, event_id);
				continue;
			}

			pkt += BCM_EVENT_HEADER_LEN; /* payload (bss response) */
			pdata_len -= BCM_EVENT_HEADER_LEN;

			bsstrans_resp = (dot11_bsstrans_resp_t *)pkt;
			addr = (struct ether_addr *)(bsstrans_resp->data);
			neighbor = (dot11_neighbor_rep_ie_t *)bsstrans_resp->data;

			WLIF_BSSTRANS("BSS Transit Response: ifname[%s], event[%d], "
				"token[%d], status[%d], mac["MACF"]\n",
				ifname, event_id, bsstrans_resp->token,
				bsstrans_resp->status, ETHERP_TO_MACF(addr));

			/* check interface */
			if (strncmp(ifname, ifreq, strlen(ifreq)) != 0) {
				/* not for the requested interface */
				WLIF_BSSTRANS("BSS Transit Response: not for interface "
					"%s its for %s\n", ifreq, ifname);
				continue;
			}

			/* check token */
			if (bsstrans_resp->token != token) {
				/* not for the requested interface */
				WLIF_BSSTRANS("BSS Transit Response: not for "
					"token %x it's for %x\n",
					token, bsstrans_resp->token);
				continue;
			}

			/* reject */
			if (bsstrans_resp->status) {
				/* If there is some neigbor report */
				if (bsstrans_resp->status == 6) {
					WLIF_BSSTRANS("id[%d] len[%d] bssid["MACF"] "
						"bssid_info[%d] reg[%d] "
						"channel[%d] phytype[%d] \n",
						neighbor->id, neighbor->len,
						ETHER_TO_MACF(neighbor->bssid),
						neighbor->bssid_info, neighbor->reg,
						neighbor->channel, neighbor->phytype);
				}
				WLIF_BSSTRANS("BSS Transit Response: STA reject"
					"and status = [%d]\n", bsstrans_resp->status);

				return WLIFU_BSS_TRANS_RESP_REJECT;
			}

			/* accept, but use another target bssid */
			if (eacmp(bssid, addr) != 0) {
				WLIF_BSSTRANS("BSS Transit Response: target bssid not same "
					"["MACF"] != ["MACF"]\n", ETHERP_TO_MACF(bssid),
					ETHERP_TO_MACF(addr));
				return WLIFU_BSS_TRANS_RESP_REJECT;
			}

			return WLIFU_BSS_TRANS_RESP_ACCEPT;
		}
	} /* While  */

	return WLIFU_BSS_TRANS_RESP_UNKNOWN;
}

/*
 * Init routine for wlif lib handle.
 */
wl_wlif_hdl*
wl_wlif_init(void *uschd_hdl, char *ifname,
	callback_hndlr ioctl_hndlr, void *data)
{
	wl_wlif_hdl_data_t *hdl = (wl_wlif_hdl_data_t*)calloc(1, sizeof(*hdl));
	char *value;

	value = nvram_safe_get(WLIFU_NVRAM_STEERING_MSGLEVEL);
	g_steering_msglevel = (int)strtoul(value, NULL, 0);

	if (!hdl) {
		WLIF_BSSTRANS("Malloc failed for hdl \n");
		return NULL;
	}

	memset(hdl, 0, sizeof(*hdl));
	hdl->uschd_hdl = uschd_hdl;
	strncpy(hdl->ifname, ifname, IFNAMSIZ-1);
	hdl->ifname[IFNAMSIZ -1] = '\0';

	/* Read the BSS transition response timneout from the NVRAM */
	value = nvram_safe_get(WLIFU_NVRAM_STEER_RESP_TIMEOUT);
	hdl->resp_timeout = (uint32)strtoul(value, NULL, 0);
	if (hdl->resp_timeout <= 0) {
		hdl->resp_timeout = WLIFU_DEF_STEER_RESP_TIMEOUT;
	}

	/* App does not registers own handler the default will be wl_wlif_do_ioctl */
	if (ioctl_hndlr) {
		hdl->ioctl_hndlr = ioctl_hndlr;
	} else {
		hdl->ioctl_hndlr = wl_wlif_do_ioctl;
	}

	/* If uschd library is not used, use linux timer */
	if (uschd_hdl == NULL) {
		/* Initialize Linux Timer Module Handle */
		bcm_timer_module_init(TIMER_MAX_COUNT, &(hdl->timer_hdl));
	}

	hdl->data = data;
	WLIF_BSSTRANS("BSS Transition Initialized Ifname[%s] Msglevel[%d] resp_timeout[%d]\n",
		hdl->ifname, g_steering_msglevel, hdl->resp_timeout);

	return hdl;
}

/*
 * Deinit routine to wlif handle.
 */
void
wl_wlif_deinit(wl_wlif_hdl *hdl)
{
	wl_wlif_hdl_data_t *hdldata = NULL;

	if (!hdl) {
		return;
	}

	hdldata = (wl_wlif_hdl_data_t*)hdl;

	/* If uschd library is not used, and Linux Timer Module Handle is valid */
	if ((hdldata->uschd_hdl == NULL) && (hdldata->timer_hdl)) {

		/* De-Initialize Linux Timer Module Handle */
		bcm_timer_module_cleanup(hdldata->timer_hdl);
	}

	free(hdldata);
	hdldata = NULL;
}

/* Default Callback function for ioctl calls. */
static int
wl_wlif_do_ioctl(char *ifname, int cmd, void *buf, int buflen,
	void *data)
{
	BCM_REFERENCE(data);
	return wl_ioctl(ifname, cmd, buf, buflen);
}

/* Get channel width from chanspec */
static uint8
wl_get_wb_chanwidth(chanspec_t chanspec)
{
	if (CHSPEC_IS8080(chanspec)) {
		return WIDE_BW_CHAN_WIDTH_80_80;
	} else if (CHSPEC_IS160(chanspec)) {
		return WIDE_BW_CHAN_WIDTH_160;
	} else if (CHSPEC_IS80(chanspec)) {
		return WIDE_BW_CHAN_WIDTH_80;
	} else if (CHSPEC_IS40(chanspec)) {
		return WIDE_BW_CHAN_WIDTH_40;
	} else {
		return WIDE_BW_CHAN_WIDTH_20;
	}
}

/* Get wideband channel width and center frequency from chanspec */
static void
wl_get_wb_ie_from_chanspec(dot11_wide_bw_chan_ie_t *wbc_ie,
	chanspec_t chanspec)
{
	if (wbc_ie == NULL) {
		return;
	}

	wbc_ie->channel_width = wl_get_wb_chanwidth(chanspec);

	if (wbc_ie->channel_width == WIDE_BW_CHAN_WIDTH_80_80) {
		wbc_ie->center_frequency_segment_0 =
			wf_chspec_primary80_channel(chanspec);
		wbc_ie->center_frequency_segment_1 =
			wf_chspec_secondary80_channel(chanspec);
	} else {
		wbc_ie->center_frequency_segment_0 = CHSPEC_CHANNEL(chanspec);
		wbc_ie->center_frequency_segment_1 = 0;
	}
}

/*
 * Creates the BSS-Trans action frame from ioctl data.
 * Returns the BSS-Trans response.
 */
static int
wl_wlif_send_bss_transreq(wl_wlif_hdl_data_t *hdl_data,
	wl_wlif_bss_trans_data_t *ioctl_data, int event_fd)
{
	int ret, buflen;
	char *param, ioctl_buf[WLC_IOCTL_MAXLEN];
	struct timeval tv; /* timed out for bss response */
	dot11_bsstrans_req_t *transreq;
	dot11_neighbor_rep_ie_t *nbr_ie;
	dot11_wide_bw_chan_ie_t *wbc_ie;
	dot11_ngbr_bsstrans_pref_se_t *nbr_pref;
	wl_af_params_t *af_params;
	wl_action_frame_t *action_frame;

	wl_endian_probe(hdl_data->ifname);

	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	strcpy(ioctl_buf, "actframe");
	buflen = strlen(ioctl_buf) + 1;
	param = (char *)(ioctl_buf + buflen);

	af_params = (wl_af_params_t *)param;
	action_frame = &af_params->action_frame;

	af_params->channel = 0;
	af_params->dwell_time = htod32(-1);

	memcpy(&action_frame->da, (char *)&(ioctl_data->addr), ETHER_ADDR_LEN);
	action_frame->packetId = htod32((uint32)(uintptr)action_frame);
	action_frame->len = htod16(DOT11_NEIGHBOR_REP_IE_FIXED_LEN +
		DOT11_NGBR_BSSTRANS_PREF_SE_IE_LEN +
		TLV_HDR_LEN + DOT11_BSSTRANS_REQ_LEN +
		TLV_HDR_LEN + DOT11_WIDE_BW_IE_LEN);

	transreq = (dot11_bsstrans_req_t *)&action_frame->data[0];
	transreq->category = DOT11_ACTION_CAT_WNM;
	transreq->action = DOT11_WNM_ACTION_BSSTRANS_REQ;
	if (++bss_token == 0)
		bss_token = 1;
	transreq->token = bss_token;
	transreq->reqmode = DOT11_BSSTRANS_REQMODE_PREF_LIST_INCL;
	/* set bit1 to tell STA the BSSID in list recommended */
	transreq->reqmode |= DOT11_BSSTRANS_REQMODE_ABRIDGED;
	/*
		remove bit2 DOT11_BSSTRANS_REQMODE_DISASSOC_IMMINENT
		because WBD will deauth sta based on BSS response
	*/
	transreq->disassoc_tmr = 0x0000;
	transreq->validity_intrvl = 0xFF;

	nbr_ie = (dot11_neighbor_rep_ie_t *)&transreq->data[0];
	nbr_ie->id = DOT11_MNG_NEIGHBOR_REP_ID;
	nbr_ie->len = DOT11_NEIGHBOR_REP_IE_FIXED_LEN +
		DOT11_NGBR_BSSTRANS_PREF_SE_IE_LEN +
		DOT11_WIDE_BW_IE_LEN + TLV_HDR_LEN;
	memcpy(&nbr_ie->bssid, &(ioctl_data->bssid), ETHER_ADDR_LEN);
	htol32_ua_store(ioctl_data->bssid_info, &nbr_ie->bssid_info);
	nbr_ie->reg = ioctl_data->rclass;
	nbr_ie->channel = wf_chspec_ctlchan(ioctl_data->chanspec);
	nbr_ie->phytype = ioctl_data->phytype;

	wbc_ie = (dot11_wide_bw_chan_ie_t *)&nbr_ie->data[0];
	wbc_ie->id = DOT11_NGBR_WIDE_BW_CHAN_SE_ID;
	wbc_ie->len = DOT11_WIDE_BW_IE_LEN;
	wl_get_wb_ie_from_chanspec(wbc_ie, ioctl_data->chanspec);

	param = (char *)wbc_ie + TLV_HDR_LEN + DOT11_WIDE_BW_IE_LEN;
	nbr_pref = (dot11_ngbr_bsstrans_pref_se_t *)param;
	nbr_pref->sub_id = DOT11_NGBR_BSSTRANS_PREF_SE_ID;
	nbr_pref->len = DOT11_NGBR_BSSTRANS_PREF_SE_LEN;
	nbr_pref->preference = DOT11_NGBR_BSSTRANS_PREF_SE_HIGHEST;

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_VAR, ioctl_buf,
		WL_WIFI_AF_PARAMS_SIZE, hdl_data->data);


	if (ret < 0) {
		printf("Err: intf:%s actframe %d\n", hdl_data->ifname, ret);
	} else if (event_fd != -1) {
		/* read the BSS transition response only if event_fd is valid */
		usleep(1000*500);
		tv.tv_sec = hdl_data->resp_timeout;
		tv.tv_usec = 0;

		/* wait for bss response and compare token/ifname/status/bssid etc  */
		return (wl_wlif_proc_event_socket(event_fd, &tv, hdl_data->ifname,
			bss_token, &(ioctl_data->bssid)));
	}
	return WLIFU_BSS_TRANS_RESP_UNKNOWN;
}

/* create blocking list of macs */
void static
wl_update_block_mac_list(maclist_t *static_maclist, maclist_t *maclist,
	int macmode, struct ether_addr *addr, unsigned char block)
{
	uint16 count = 0, idx, action;

	/* Action table
	 * ALLOW  BLOCK		DEL
	 * DENY   BLOCK		ADD
	 * ALLOW  UNBLOCK	ADD
	 * DENY   UNBLOCK	DEL
	 */
	action = ((macmode == WLC_MACMODE_ALLOW) ^ block) ? WL_MAC_ADD : WL_MAC_DEL;
	switch (action) {
		case WL_MAC_DEL:
			for (idx = 0; idx < static_maclist->count; idx++) {
				if (eacmp(addr, &static_maclist->ea[idx]) == 0) {
					continue;
				}
				memcpy(&(maclist->ea[count]), &static_maclist->ea[idx],
					ETHER_ADDR_LEN);
				count++;
			}
			maclist->count = count;
			break;

		case WL_MAC_ADD:
			for (idx = 0; static_maclist != NULL && idx < static_maclist->count;
					idx++) {
				/* Avoid duplicate entry in maclist. */
				if (eacmp(addr, &static_maclist->ea[idx]) == 0) {
					continue;
				}

				memcpy(&(maclist->ea[count]), &static_maclist->ea[idx],
					ETHER_ADDR_LEN);
				count++;
			}

			memcpy(&(maclist->ea[count]), addr,  ETHER_ADDR_LEN);
			count++;
			maclist->count = count;
		break;

		default:
			printf("Wrong action= %d\n", action);
			assert(0);
	}
}

/* Create a Timer */
static int
wl_wlif_create_timer(void *uschd_hdl, bcm_timer_module_id timer_hdl,
	void* cbfn, void *data, int timeout, int repeat)
{
	int ret = -1;
	bcm_timer_id timer_id;
	struct itimerspec  its;

	/* If uschd library is not used, Create linux timer */
	if (uschd_hdl == NULL) {

		/* Cast cbfn to relevant callback function */
		wlif_linux_timer_cb* linux_cbfn = (wlif_linux_timer_cb*)cbfn;

		/* Create Timer */
		ret = bcm_timer_create(timer_hdl, &timer_id);
		if (ret) {
			return FALSE;
		}

		/* Connect Timer */
		ret = bcm_timer_connect(timer_id, (bcm_timer_cb)linux_cbfn, (uintptr_t)data);
		if (ret) {
			return FALSE;
		}

		/* Set up retry timer */
		its.it_interval.tv_sec = timeout;
		its.it_interval.tv_nsec = 0;
		its.it_value.tv_sec = timeout;
		its.it_value.tv_nsec = 0;
		ret = bcm_timer_settime(timer_id, &its);
		if (ret) {
			return FALSE;
		}

	/* If uschd library is used, Create uschd timer */
	} else {

		/* Cast cbfn to relevant callback function */
		wlif_usched_timer_cb* usched_cbfn = (wlif_usched_timer_cb*)cbfn;

		/* Create Timer */
		ret = bcm_usched_add_timer(uschd_hdl,
			SEC_TO_MICROSEC(timeout), repeat, usched_cbfn, (void*)data);

		if (ret != BCM_USCHEDE_OK) {
			WLIF_BSSTRANS("Failed to add timer Err:%s\n", bcm_usched_strerror(ret));
			return FALSE;
		}
	}

	return TRUE;
}

/*
 * Retrieves the current macmode, maclist and probe resp filter values
 * of interface, updates the data with STA address and sets them again.
 * If timeout is non zero and non negative than creates timer for
 * unblock mac routine.
 * For negative timeout value unblock timer will not be created.
 * For 0 timeout value sta will not be blocked at all.
 */
int
wl_wlif_block_mac(wl_wlif_hdl *hdl, struct ether_addr addr, int timeout)
{
	char maclist_buf[WLC_IOCTL_MAXLEN];
	char smaclist_buf[WLC_IOCTL_MAXLEN];
	int macmode, ret, macprobresp;
	maclist_t *s_maclist = (maclist_t *)smaclist_buf;
	maclist_t *maclist = (maclist_t *)maclist_buf;
	static short flag;
	wl_wlif_hdl_data_t *hdl_data;

	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		return FALSE;
	}

	/* For 0 timeout don't block. */
	if (!timeout) {
		WLIF_BSSTRANS("Err: timeout is zero \n");
		return FALSE;
	}

	hdl_data = (wl_wlif_hdl_data_t *)hdl;

	wl_endian_probe(hdl_data->ifname);

	memset(maclist_buf, 0, WLC_IOCTL_MAXLEN);
	memset(smaclist_buf, 0, WLC_IOCTL_MAXLEN);
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_MACMODE, &(macmode),
		sizeof(macmode), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("Err: get %s macmode fails %d\n", hdl_data->ifname, ret);
		return FALSE;
	}
	macmode = dtoh32(macmode);

	/* retrive static maclist */
	if (hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_MACLIST, (void *)s_maclist,
		sizeof(maclist_buf), hdl_data->data) < 0) {
		WLIF_BSSTRANS("Err: get %s maclist fails\n", hdl_data->ifname);
		return FALSE;
	}
	s_maclist->count = dtoh32(s_maclist->count);

	ret = wl_iovar_getint(hdl_data->ifname, "probresp_mac_filter", &macprobresp);
	if (ret != 0) {
		WLIF_BSSTRANS("Err: %s Probresp mac filter set failed %d\n", hdl_data->ifname, ret);
	}

	if (flag == 0) {
		flag = s_maclist->count;
		flag |= WL_MAC_PROB_MODE_SET(macmode, macprobresp, TRUE);
	}

	if (timeout > 0) {
		/* Create Unblock MAC Timer */
		ret = wl_wlif_create_unblock_mac_timer(hdl_data, addr, timeout, flag);

		/* If Unblock MAC Timer Creation Fails, return Error fm this function */
		if (ret == FALSE) {
			return FALSE;
		}
	}

	wl_update_block_mac_list(s_maclist, maclist, macmode, &addr, TRUE);
	macmode = htod32((macmode == WLC_MACMODE_ALLOW) ? WLC_MACMODE_ALLOW : WLC_MACMODE_DENY);
	maclist->count = htod32(maclist->count);
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_MACLIST, maclist,
		ETHER_ADDR_LEN * maclist->count + sizeof(uint), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("Err: %s maclist set failed %d\n", hdl_data->ifname, ret);
	}

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_MACMODE, &macmode,
		sizeof(int), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("Err: %s macmode set failed %d\n", hdl_data->ifname, ret);
	}

	ret = wl_iovar_setint(hdl_data->ifname, "probresp_mac_filter", TRUE);
	if (ret != 0) {
		WLIF_BSSTRANS("Err: %s Probresp mac filter set failed %d\n", hdl_data->ifname, ret);
	}

	return TRUE;
}

/*
 * If BSS-Trans response is not ACCEPT, or if it is ACCEPT but still persists in Assoclist
 *  invokes Block MAC and Deauthincates STA from current BSS
 */
static int
wl_wlif_block_mac_and_deauth(wl_wlif_hdl *hdl, struct ether_addr addr,
	int timeout, int deauth)
{
	int ret = -1;
	wl_wlif_hdl_data_t *hdl_data;

	/* Get wlif Handle */
	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		goto end;
	}
	hdl_data = (wl_wlif_hdl_data_t*)hdl;

	/* Block STA on Source BSSID */
	wl_wlif_block_mac(hdl_data, addr, timeout);

	/* If Brute Force Steer is applicable, for this Response Type, Send Deauth */
	if (deauth) {

		/* Send De-authenticate to STA from Source BSSID */
		ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SCB_DEAUTHENTICATE,
			&(addr), ETHER_ADDR_LEN, hdl_data->data);

		if (ret < 0) {
			WLIF_BSSTRANS("Deauth to STA["MACF"] from %s failed\n",
				ETHER_TO_MACF(addr), hdl_data->ifname);
			goto end;
		}
	}

end:
	return ret;
}

/*
 * Send BSS Transition Request Action Frame and check the Response.
 * If BSS-Trans response is not ACCEPT, or if it is ACCEPT but still persists in Assoclist
 * invokes Block MAC and Deauthincates STA from current BSS
 */
int
wl_wlif_do_bss_trans(wl_wlif_hdl *hdl,
	wl_wlif_bss_trans_data_t *ioctl_hndlr_data, int event_fd)
{
	int bsstrans_ret = -1, steer_flags = WLIFU_DEF_STEER_FLAGS, brute_force = 0;
	char *value = NULL;
	wl_wlif_hdl_data_t *hdl_data;

	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		goto end;
	}

	hdl_data = (wl_wlif_hdl_data_t*)hdl;

	/* Block mac in curr interface. */
	wl_wlif_block_mac(hdl_data, ioctl_hndlr_data->addr, ioctl_hndlr_data->timeout);

	/* Create & Send BSS Transition Action Frame & Return the Response */
	bsstrans_ret = wl_wlif_send_bss_transreq(hdl_data, ioctl_hndlr_data, event_fd);
	WLIF_BSSTRANS("BSS Transition Resp from STA["MACF"] to BSSID["MACF"] is : %s\n",
		ETHER_TO_MACF(ioctl_hndlr_data->addr),
		ETHER_TO_MACF(ioctl_hndlr_data->bssid),
		((bsstrans_ret == WLIFU_BSS_TRANS_RESP_REJECT) ? "REJECT" :
		((bsstrans_ret == WLIFU_BSS_TRANS_RESP_UNKNOWN) ? "UNKNOWN" :"ACCEPT")));

	/* If bss-trans resp handler is provided invoke it. */
	if ((ioctl_hndlr_data->resp_hndlr != NULL) &&
		ioctl_hndlr_data->resp_hndlr(ioctl_hndlr_data->resp_hndlr_data, bsstrans_ret)) {
		goto end;
	}

	/* Read NVRAM for Steering Flags, to customize steering behavior */
	value = nvram_safe_get(WLIFU_NVRAM_STEER_FLAGS);
	if (value[0] != '\0') {
		steer_flags = (int)strtoul(value, NULL, 0);
	}

	/* For BSS Transition Response ACCEPT, Check if Dafiant STA Watchdog is required or not */
	if ((bsstrans_ret == WLIFU_BSS_TRANS_RESP_ACCEPT) && BTR_ACPT_DEFIANT_WD_ON(steer_flags)) {

		/* Create Defiant STA Watchdog Timer */
		wl_wlif_create_defiant_sta_watchdog_timer(hdl_data,
			ioctl_hndlr_data->addr, ioctl_hndlr_data->timeout);
	}

	/* Check if Brute Force Steer is applicable, for this Response Type */
	brute_force =
		(((bsstrans_ret == WLIFU_BSS_TRANS_RESP_REJECT) &&
		(BTR_RJCT_BRUTFORCE(steer_flags))) ||
		((bsstrans_ret == WLIFU_BSS_TRANS_RESP_UNKNOWN) &&
		(!BTR_UNWN_BRUTFORCE_BLK(steer_flags))));

	/* If Brute Force Steer is not applicable, for this Response Type, Leave */
	if (!brute_force) {

		WLIF_BSSTRANS("No Deauth to STA["MACF"] from %s by NVRAM %s = %d\n",
			ETHER_TO_MACF(ioctl_hndlr_data->addr), hdl_data->ifname,
			WLIFU_NVRAM_STEER_FLAGS, steer_flags);
		goto end;
	}

	/* If Brute Force is applicable, for this Response Type, Do Brute Force way */
	wl_wlif_block_mac_and_deauth(hdl, ioctl_hndlr_data->addr,
		ioctl_hndlr_data->timeout, brute_force);

end:
	return bsstrans_ret;
}

/*
 * Retrieves the current macmode, maclist and proberesp filter values.
 * Updates maclist with STA addr entry. Based on flag bits
 * updates macmode and probe resp filter values.
 */
int
wl_wlif_unblock_mac(wl_wlif_hdl *hdl, struct ether_addr addr, int flag)
{
	char maclist_buf[WLC_IOCTL_MAXLEN];
	char smaclist_buf[WLC_IOCTL_MAXLEN];
	int macmode, ret = 0;
	wl_wlif_hdl_data_t *hdl_data;
	maclist_t *s_maclist = (maclist_t *)smaclist_buf;
	maclist_t *maclist = (maclist_t *)maclist_buf;

	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		return FALSE;
	}

	hdl_data = (wl_wlif_hdl_data_t*)hdl;

	wl_endian_probe(hdl_data->ifname);

	memset(maclist_buf, 0, WLC_IOCTL_MAXLEN);
	memset(smaclist_buf, 0, WLC_IOCTL_MAXLEN);

	if (hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_MACMODE, &(macmode),
		sizeof(macmode), hdl_data->data) != 0) {
		WLIF_BSSTRANS("Err: get %s macmode fails \n", hdl_data->ifname);
		return FALSE;
	}
	macmode = dtoh32(macmode);

	/* retrive static maclist */
	if (hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_MACLIST, (void *)s_maclist,
		sizeof(maclist_buf), hdl_data->data) < 0) {
		WLIF_BSSTRANS("Err: get %s maclist fails\n", hdl_data->ifname);
		return FALSE;
	}
	s_maclist->count = dtoh32(s_maclist->count);

	wl_update_block_mac_list(s_maclist, maclist, macmode, &(addr), FALSE);
	macmode = (macmode == WLC_MACMODE_ALLOW) ? WLC_MACMODE_ALLOW : WLC_MACMODE_DENY;

	if (flag && (maclist->count == (flag & 0xFF))) {
		macmode = WL_MACMODE_GET(flag);
		if (wl_iovar_setint(hdl_data->ifname, "probresp_mac_filter",
			WL_MACPROBE_GET(flag)) != 0) {
			WLIF_BSSTRANS("Error:%s Probresp mac filter set failed \n",
				hdl_data->ifname);
		}
	}

	maclist->count = htod32(maclist->count);
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_MACLIST, maclist,
		ETHER_ADDR_LEN * maclist->count + sizeof(uint), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("Err: %s maclist filter set failed %d\n", hdl_data->ifname, ret);
	}

	macmode = htod32(macmode);
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_MACMODE, &macmode,
		sizeof(int), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("Err: %s macmode set failed %d\n", hdl_data->ifname, ret);
	}

	return TRUE;
}

/* Callback handler for unblock mac */
static void
wl_wlif_unblock_mac_cb(bcm_timer_id timer_id, void* arg)
{
	static_maclist_t *data = (static_maclist_t*)arg;

	wl_wlif_unblock_mac(data->hdl_data, data->addr, data->flag);

	free(data);
	bcm_timer_delete(timer_id);
}

/* Callback handler for unblock mac */
static void
wl_wlif_unblock_mac_usched_cb(bcm_usched_handle* hdl, void* arg)
{
	static_maclist_t *data = (static_maclist_t*)arg;

	wl_wlif_unblock_mac(data->hdl_data, data->addr, data->flag);

	free(data);
}

/* Create Unblock MAC Timer */
static int
wl_wlif_create_unblock_mac_timer(wl_wlif_hdl_data_t *hdl_data,
	struct ether_addr addr, int timeout, int flag)
{
	int ret = FALSE;
	static_maclist_t *tmr_data = NULL;

	/* Allocate Timer Arg Struct Object */
	tmr_data = (static_maclist_t *) calloc(1, sizeof(*tmr_data));
	if (!tmr_data) {
		WLIF_BSSTRANS("Err: calloc Timer Arg Failed \n");
		return FALSE;
	}

	/* Initialize Timer Arg Struct Object */
	tmr_data->hdl_data = hdl_data;
	tmr_data->flag = flag;
	eacopy(&addr, &(tmr_data->addr));

	/* Create an Appropriate Timer, to Unblock a MAC */
	ret = wl_wlif_create_timer(hdl_data->uschd_hdl, hdl_data->timer_hdl,
		(hdl_data->uschd_hdl ? (void*)wl_wlif_unblock_mac_usched_cb :
		(void*)wl_wlif_unblock_mac_cb), (void*)tmr_data, timeout, FALSE);
	if (!ret) {
		free(tmr_data);
		return FALSE;
	}

	return TRUE;
}

/* Traverse Maclist to find MAC address exists or not */
static bool
wl_wlif_find_sta_in_assoclist(wl_wlif_hdl_data_t *hdl_data, struct ether_addr *find_mac)
{
	int ret = 0, iter = 0;
	bool sta_found = FALSE;
	struct maclist *list = NULL;

	wl_endian_probe(hdl_data->ifname);

	/* Prepare Assoclist IOCTL Structure Object */
	WLIF_BSSTRANS("WLC_GET_ASSOCLIST\n");
	list = (struct maclist *)calloc(1, WL_WLIF_BUFSIZE_4K);
	if (!list) {
		WLIF_BSSTRANS("Err: calloc Assoclist IOCTL buff Failed \n");
		goto end;
	}
	list->count = htod32((WL_WLIF_BUFSIZE_4K - sizeof(int)) / ETHER_ADDR_LEN);

	/* Read Assoclist */
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_ASSOCLIST, list,
		WL_WLIF_BUFSIZE_4K, hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("Err: [%s] WLC_GET_ASSOCLIST Failed : %d\n", hdl_data->ifname, ret);
		goto end;
	}

	list->count = dtoh32(list->count);
	if (list->count <= 0) {
		goto end;
	}

	/* Travese maclist items */
	for (iter = 0; iter < list->count; iter++) {

		/* STA Checking Condition */
		if (eacmp(&list->ea[iter], find_mac) == 0) {
			sta_found = TRUE;
			goto end;
		}
	}

end:
	if (list) {
		free(list);
	}
	return sta_found;
}

/* Run Defiant STA Watchdog */
static void
wl_wlif_run_defiant_sta_watchdog(void* arg)
{
	bool sta_found = FALSE;
	static_maclist_t *defiant_tmr_data = (static_maclist_t*)arg;

	/* Check if Steering STA is still persisting in ASSOCLIST of its Source AP */
	sta_found = wl_wlif_find_sta_in_assoclist(defiant_tmr_data->hdl_data,
		&defiant_tmr_data->addr);

	if (sta_found) {
		/* If Brute Force is applicable, for this Response Type, Do Brute Force way */
		wl_wlif_block_mac_and_deauth(defiant_tmr_data->hdl_data, defiant_tmr_data->addr,
			defiant_tmr_data->timeout, 1);
	}

	/* Free Timer Arg Struct Object */
	free(defiant_tmr_data);

}

/* For LINUX Timer, Callback handler to Start Defiant STA Watchdog */
static void
wl_wlif_defiant_sta_wd_cb(bcm_timer_id timer_id, void* arg)
{
	/* Run Defiant STA Watchdog */
	wl_wlif_run_defiant_sta_watchdog(arg);

	bcm_timer_delete(timer_id);
}

/* For USCHED Timer, Callback handler to Start Defiant STA Watchdog */
static void
wl_wlif_defiant_sta_wd_usched_cb(bcm_usched_handle* hdl, void* arg)
{
	/* Run Defiant STA Watchdog */
	wl_wlif_run_defiant_sta_watchdog(arg);
}

/* Create Defiant STA Watchdog Timer */
static int
wl_wlif_create_defiant_sta_watchdog_timer(wl_wlif_hdl_data_t *hdl_data,
	struct ether_addr addr, int timeout)
{
	int ret = FALSE, tm_defiant_wd = WLIFU_DEF_TM_DEFIANT_WD;
	static_maclist_t *defiant_tmr_data = NULL;
	char *value = NULL;

	/* Read NVRAM for Dafiant STA Watchdog Timer Tick */
	value = nvram_safe_get(WLIFU_NVRAM_TM_DEFIANT_WD);
	if (value[0] != '\0') {
		tm_defiant_wd = (int)strtoul(value, NULL, 0);
	}

	/* Allocate Timer Arg Struct Object */
	defiant_tmr_data = (static_maclist_t *) calloc(1, sizeof(*defiant_tmr_data));
	if (!defiant_tmr_data) {
		WLIF_BSSTRANS("Err: calloc Timer Arg Failed \n");
		return FALSE;
	}

	/* Initialize Timer Arg Struct Object */
	defiant_tmr_data->hdl_data = hdl_data;
	defiant_tmr_data->flag = 0;
	defiant_tmr_data->timeout = timeout;
	eacopy(&(addr), &(defiant_tmr_data->addr));

	/* Create an Appropriate Timer, For Defiant STA Watchdog */
	ret = wl_wlif_create_timer(hdl_data->uschd_hdl, hdl_data->timer_hdl,
		(hdl_data->uschd_hdl ? (void*)wl_wlif_defiant_sta_wd_usched_cb :
		(void*)wl_wlif_defiant_sta_wd_cb), (void*)defiant_tmr_data, tm_defiant_wd, FALSE);
	if (!ret) {
		free(defiant_tmr_data);
		return FALSE;
	}

	return TRUE;
}
