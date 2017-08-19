/*
 * RSERV: Remote Sockets Server (RTE-specific portion)
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

#ifdef EXT_CBALL
#define STAMP(type)	(*(uint32 *)0x18009014 = (type))
#else
#define STAMP(type)
#endif

#include <osl.h>
#include <proto/ethernet.h>
#include <dngl_dbg.h>
#include <dngl_bus.h>
#include <dngl_api.h>

#include "lwip/opt.h"
#include "lwip/memp.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/ip.h"
#include "lwip/ip_frag.h"
#include "lwip/raw.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/err.h"
#include "lwip/stats.h"
#include "lwip/netif.h"

#include <rsock/types.h>
#include <rsock/proto.h>

#include "rserv_if.h"
#include "rserv_rte.h"
#include "rserv.h"

#define DPRINTF			if (0) printf

struct dngl *rserv_dngl;
osl_t *rserv_osh;

/* Send a response packet back over the bus. */

struct lbuf *
rserv_rte_pktget(int len, int send)
{
	RSERV_PKT *pkt;

	if ((pkt = PKTGET(rserv_osh, len, send)) == NULL) {
		printf("rserv_rte_pktget: packet alloc failed (fatal)\n");
		ASSERT(0);
		for (;;)
			;
	}

	return pkt;
}

void *
rserv_rte_malloc(int len)
{
	void *ptr;

	if ((ptr = MALLOC(rserv_osh, len)) == NULL) {
		printf("rserv_rte_malloc: out of memory (fatal)\n");
		ASSERT(0);
		for (;;)
			;
	}

	return ptr;
}

void
rserv_rte_output(struct lbuf *pkt)
{
	int len = PKTLEN(rserv_osh, pkt);

	(void)len;

	STAMP(140);
	ASSERT(len >= sizeof(struct rsock_resp_header));
	ASSERT(len <= RSOCK_BUS_MTU);

	DPRINTF("rserv_rte_output: len=%d {%02x%02x %02x%02x %02x%02x %02x%02x}\n",
	        len,
	        btohs(((uint8 *)PKTDATA(rserv_osh, pkt))[0]),
	        btohs(((uint8 *)PKTDATA(rserv_osh, pkt))[1]),
	        btohs(((uint8 *)PKTDATA(rserv_osh, pkt))[2]),
	        btohs(((uint8 *)PKTDATA(rserv_osh, pkt))[3]),
	        btohs(((uint8 *)PKTDATA(rserv_osh, pkt))[4]),
	        btohs(((uint8 *)PKTDATA(rserv_osh, pkt))[5]),
	        btohs(((uint8 *)PKTDATA(rserv_osh, pkt))[6]),
	        btohs(((uint8 *)PKTDATA(rserv_osh, pkt))[7]));

	dngl_sendbus(rserv_dngl, pkt);
}

hnd_timer_t *rtetimer_lwip;

/* The timer routine is called at a rate of 4Hz */
static void
rserv_lwip_timer(hnd_timer_t *t)
{
	(void)t;

	/* call TCP timer handler every 250ms */
	tcp_tmr();

#if IP_REASSEMBLY
	{
		static uint32 ticks_reass = 0;
		/* call IP fragmentation/reassembly every 1s */
		if (++ticks_reass == 4) {
			ip_reass_tmr();
			ticks_reass = 0;
		}
	}
#endif /* IP_REASSEMBLY */

#if LWIP_DHCP
	{
		static uint32 ticks_dhcp_fine = 0;
		static uint32 ticks_dhcp_coarse = 0;

		/* call DHCP fine timer every 500ms */
		if (++ticks_dhcp_fine == 2) {
			dhcp_fine_tmr();
			ticks_dhcp_fine = 0;
		}

		/* call DHCP coarse timer every 60s */
		if (++ticks_dhcp_coarse == 240) {
			dhcp_coarse_tmr();
			ticks_dhcp_coarse = 0;
		}
	}
#endif /* LWIP_DHCP */
}

int
rserv_rte_init(struct dngl *dngl, struct ether_addr *macaddr)
{
	DPRINTF("rserv_rte_init: entered\n");

	rserv_dngl = dngl;
	rserv_osh = dngl->osh;

	rserv_init();
	rserv_if_init(dngl, macaddr);

	/* Initialize network interface support and TCP/IP stack */

#if LWIP_STATS
	stats_init();
#endif /* STATS */

	sys_init();
	mem_init();
	memp_init();
	pbuf_init();

#ifdef LWIP_TCPDUMP
	tcpdump_init();
#endif

	netif_init();

	ip_init();
	udp_init();
	tcp_init();

	DPRINTF("rserv_rte_init: TCP/IP initialized\n");

	rtetimer_lwip = hnd_timer_create(NULL, NULL, rserv_lwip_timer, NULL);
	ASSERT(rtetimer_lwip);

#ifdef _RTE_SIM_
#undef TCP_TMR_INTERVAL
#define TCP_TMR_INTERVAL 1
#endif
	if (!hnd_timer_start(rtetimer_lwip, TCP_TMR_INTERVAL, TRUE))
		ASSERT(FALSE);

	DPRINTF("rserv_rte_init: done\n");

	return 0;
}
