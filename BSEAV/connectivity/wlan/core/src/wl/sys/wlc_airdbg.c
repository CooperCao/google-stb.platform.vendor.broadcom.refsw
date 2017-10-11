/*
 * Additional driver debug functionalities
 * for AirDebug/syslog feature.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
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
#include <proto/bcmarp.h>
#include <proto/bcmudp.h>
#include <proto/vlan.h>
#include <bcmendian.h>

#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_export.h>
#ifdef BCM_OL_DEV
#include <wlc_types.h>
#include <bcm_ol_msg.h>
#include <wlc_hw_priv.h>
#include <wlc_dngl_ol.h>
#endif
#include <wl_arpoe.h>
#include <wlc_airdbg.h>


/* Maximum number of Air Debug -enabled wlc descriptors */
#define AIRDBG_N_WLC 2

/* Maximum Air Debug message length */
#define AIRDBG_MSGLEN 256
/* Length of the fixed part in LLDP packets */

#define IPV4_UDP_SNAP_SYSLOG_FIXED_LEN (\
	 DOT11_LLC_SNAP_HDR_LEN +IPV4_MIN_HEADER_LEN + UDP_HDR_LEN)

#define UPD_PORT_SYSLOG 514

/* Pointers to Air Debug -enabled wlc descriptors */
wlc_info_t *wlc_airdbg_wlc[AIRDBG_N_WLC] = { (wlc_info_t *)(NULL) };

static const struct ether_addr lldp_addr = {{ 0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e }};


/* Set iovar handler */
void wlc_airdbg_set(wlc_info_t *wlc, bool val)
{
	uint8 i;

	for (i = 0; i < AIRDBG_N_WLC; i++) {
		if (val && (wlc_airdbg_wlc[i] == NULL)) {
			wlc_airdbg_wlc[i] = wlc;
			return;
		} else if (!val && (wlc_airdbg_wlc[i] == wlc)) {
			wlc_airdbg_wlc[i] = NULL;
			return;
		}
	}
}

/* Get iovar handler */
bool wlc_airdbg_get(wlc_info_t *wlc)
{
	uint8 i;

	for (i = 0; i < AIRDBG_N_WLC; i++)
		if (wlc_airdbg_wlc[i] == wlc)
			return TRUE;

	return FALSE;
}

/* Air Debug printf() substitute */
void wlc_airdbg_printf(const char *fmt, ...)
{
	va_list args;
	bool printed = FALSE;
	uint8 i;

	struct ipv4_hdr *iph;
	struct bcmudp_hdr *udph;
	/* running log index */
	static uint8 air_index = 0;

	for (i = 0; i < AIRDBG_N_WLC; i++) {
		wlc_info_t *wlc = wlc_airdbg_wlc[i];
		wlc_bsscfg_t *bsscfg = (wlc ? wlc->cfg : NULL);
		void *p = NULL;
		uint8 *pbody, *psnap;
		int len, buflen, msg_len;
		struct dot11_header *h;
		uint16 fc;
		uint8	src_ip[IPV4_ADDR_LEN] = {192, 168, 2, 2 };
		uint8	dst_ip[IPV4_ADDR_LEN] = {192, 168, 2, 3 };
		uint8	snap[DOT11_LLC_SNAP_HDR_LEN] = {0xaa, 0xaa,
			0x03, 0x00, 0x00, 0x00, 0x08, 0x00};

		if (!wlc || !wlc->pub->up || !bsscfg) {
			continue;
		}

		/* Allocate packet first - adding space for header */
		len = DOT11_A3_HDR_LEN + TXOFF + AIRDBG_MSGLEN + IPV4_UDP_SNAP_SYSLOG_FIXED_LEN;
		buflen = (len > PKTBUFSZ) ? PKTBUFSZ : len;

		if ((p = PKTGET(wlc->osh, buflen, TRUE)) == NULL) {
			printf("wl%d: AIRDBG: pktget error for len %d \n",
				wlc->pub->unit, buflen);
			WLCNTINCR(wlc->pub->_cnt->txnobuf);
			continue;
		}
		ASSERT(ISALIGNED((uintptr) PKTDATA(wlc->osh, p), sizeof(uint32)));

		/* Reserve TXOFF bytes of headroom */
		PKTPULL(wlc->osh, p, TXOFF);
		PKTSETLEN(wlc->osh, p, buflen - TXOFF);

		/* Set MAX Prio for MGMT packets */
		PKTSETPRIO(p, MAXPRIO);

		/* Construct a frame using own MAC addr src and LLDP multicast addr dest */
		h = (struct dot11_header *)PKTDATA(wlc->osh, p);
		fc = FC_DATA;
		if (BSSCFG_AP(bsscfg)) {
			/* AP: a1 = DA, a2 = BSSID, a3 = SA, ToDS = 0, FromDS = 1 */
			bcopy((char*) &lldp_addr, (char*) &h->a1, ETHER_ADDR_LEN);
			bcopy((char*) &bsscfg->BSSID, (char*) &h->a2, ETHER_ADDR_LEN);
			bcopy((char*) &wlc->pub->cur_etheraddr, (char*) &h->a3, ETHER_ADDR_LEN);
			fc |= FC_FROMDS;
		}
		else {
			/* We'll need some faking here. So that we don't trigger error
			* messages in the AP, send as IBSS frame to a bogus multicast
			* IBSS address.
			*/

			/* IBSS/DPT STA: a1 = DA, a2 = SA, a3 = BSSID, ToDS = 0, FromDS = 0 */
			bcopy((char*) &lldp_addr, (char*) &h->a1, ETHER_ADDR_LEN);
			bcopy((char*) &wlc->pub->cur_etheraddr, (char*) &h->a2, ETHER_ADDR_LEN);
			bcopy((char*) &lldp_addr, (char*) &h->a3, ETHER_ADDR_LEN);
		}
		h->fc = htol16(fc);
		h->durid = 0;
		h->seq = 0;

		/* Find start of data  */
		pbody = (uint8 *) h + DOT11_A3_HDR_LEN;

		/* This is the available space for message */
		buflen -= TXOFF + DOT11_A3_HDR_LEN + IPV4_UDP_SNAP_SYSLOG_FIXED_LEN;

		/* LLC header: SNAP, encapsulated Ethernet */
		psnap = (uint8 *) h + DOT11_A3_HDR_LEN;
		bcopy(snap, psnap, DOT11_LLC_SNAP_HDR_LEN);

		/* IPV4 */
		iph = (struct ipv4_hdr *) ((uint8 *) h +
			DOT11_A3_HDR_LEN+ DOT11_LLC_SNAP_HDR_LEN);
		memset((char *) iph, 0, IPV4_MIN_HEADER_LEN);

		iph->version_ihl = (IP_VER_4 << IP_VER_SHIFT) |
			(IPV4_MIN_HEADER_LEN/sizeof(int));

		iph->ttl = 0xff;
		iph->prot = IP_PROT_UDP;
		bcopy(src_ip, iph->src_ip, IPV4_ADDR_LEN);
		bcopy(dst_ip, iph->dst_ip, IPV4_ADDR_LEN);

		/* UDP/syslog */
		udph = (struct bcmudp_hdr *)  ((uint8 *) h +
			DOT11_A3_HDR_LEN+ DOT11_LLC_SNAP_HDR_LEN + IPV4_MIN_HEADER_LEN);
		memset((char *) udph, 0, UDP_HDR_LEN);
		udph->src_port = hton16(UPD_PORT_SYSLOG);
		udph->dst_port = hton16(UPD_PORT_SYSLOG);

		/* message */
		pbody = (uint8 *) h + DOT11_A3_HDR_LEN + IPV4_UDP_SNAP_SYSLOG_FIXED_LEN;
		msg_len = sprintf((char *)pbody, "%03d: ", air_index++);
		pbody += msg_len;
		buflen -= msg_len;

		va_start(args, fmt);
		msg_len += vsnprintf((char *) pbody, buflen, fmt, args);
		va_end(args);

		/* trim back the length to what is used */
		iph->tot_len = hton16(IPV4_MIN_HEADER_LEN + UDP_HDR_LEN  + msg_len);
		udph->len = hton16(UDP_HDR_LEN  + msg_len);
		buflen = DOT11_A3_HDR_LEN  + msg_len + IPV4_UDP_SNAP_SYSLOG_FIXED_LEN;
		PKTSETLEN(wlc->osh, p, buflen);

		/* Print to console from the pkt contents. */
		if (!printed)
			printf("%s", pbody);
		printed = TRUE;

		/* Use wlc_sendctl() - enqueue only if we're not on home channel. */
		if (!wlc_sendctl(wlc, p, bsscfg->wlcif->qi,
			WLC_BCMCSCB_GET(wlc, bsscfg), TX_CTL_FIFO, 0,
			(bsscfg->current_bss->chanspec != WLC_BAND_PI_RADIO_CHANSPEC))) {
			/* No need to call PKTFREE - wlc_sendctl() will do that upon failures. */
			printf("AIRDBG: wlc_sendctl() failed.\n");
		}
	}

	if (!printed) {
		/* No valid wlc's to send through - just print to console. */
		char buf[AIRDBG_MSGLEN];
		va_start(args, fmt);
		vsnprintf(buf, sizeof(buf), fmt, args);
		printf(buf);
		va_end(args);
	}
}
