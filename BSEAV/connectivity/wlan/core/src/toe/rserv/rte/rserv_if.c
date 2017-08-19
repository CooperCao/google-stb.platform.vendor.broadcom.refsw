/*
 * RSERV Interface: a lwIP interface for the dongle wl driver
 *
 * This code is assumed to run in a non-threaded environment.
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
#include "lwip/err.h"
#include "lwip/stats.h"
#include "lwip/netif.h"

#include "netif/etharp.h"

#include <rsock/types.h>
#include <rsock/proto.h>

#include "rserv_rte.h"
#include "rserv_if.h"
#include "rserv.h"

#define DPRINTF			if (0) printf

#define IFNAME 			"en0"
#define IF_MTU_DEFAULT		1518

static struct dngl *if_dngl;
static osl_t *if_osh;
static struct netif if_netif;
static int if_added = 0;
static int dhcp_started = 0;
static struct rsock_ifconfig if_config;
static struct rsock_ifstats if_stats;
static hnd_timer_t *if_etharp_timer;

static err_t
if_input(struct pbuf *p, struct netif *netif)
{
	DPRINTF("if_input: p=%p netif=%p\n", (void *)p, (void *)netif);


	ip_input(p, netif);

	return ERR_OK;
}

#define HEADROOM		158

static err_t
rserv_if_output(struct netif *netif, struct pbuf *p)
{
	struct lbuf *pkt;
	struct pbuf *q;
	unsigned char *msg, *msgptr;
	int msglen;

#ifdef SHOW_PKTS
	printf("out(%d)\n", p->tot_len);
#endif

	STAMP(90);
	DPRINTF("rserv_if_output: p=%p\n", (void *)p);

#if ETH_PAD_SIZE
	pbuf_header(p, -ETH_PAD_SIZE);			/* drop the padding word */
#endif

	msglen = p->tot_len;

	if (msglen == 0 || (pkt = PKTGET(if_osh, msglen + HEADROOM, TRUE)) == NULL) {
		DPRINTF("rserv_if_output: could not allocate %d bytes\n", msglen);

#if ETH_PAD_SIZE
		pbuf_header(p, ETH_PAD_SIZE);			/* reclaim the padding word */
#endif

		LINK_STATS_INC(link.drop);
		return ERR_MEM;
	}

	/* Make room for L2/L1/DMA headers */
	PKTPULL(if_osh, pkt, HEADROOM);

	/* Flatten pbuf into native OSL packet */
	msg = (unsigned char *)PKTDATA(if_osh, pkt);
	msgptr = msg;

	for (q = p; q != NULL; q = q->next) {
		/* STAMP(252); */
		memcpy(msgptr, q->payload, q->len);
		/* STAMP(253); */
		msgptr += q->len;
	}

	ASSERT(msgptr == msg + msglen);

	PKTSETLEN(if_osh, pkt, msglen);

	DPRINTF("rserv_if_output: transmit wl pkt %p\n", (void *)pkt);

	if (dngl_sendslave(if_dngl, pkt)) {
		LINK_STATS_INC(link.drop);
		if_stats.txdrop++;
		DPRINTF("rserv_if_output: dropped wl packet (%d bytes)\n", msglen);
	} else {
		if_stats.txframe++;
		if_stats.txbyte += msglen;
		DPRINTF("rserv_if_output: sent wl packet (%d bytes)\n", msglen);
	}

#if ETH_PAD_SIZE
	pbuf_header(p, ETH_PAD_SIZE);			/* reclaim the padding word */
#endif

	LINK_STATS_INC(link.xmit);

	STAMP(99);
	return ERR_OK;
}

static err_t
if_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
	/* resolve hardware address, then send (or queue) packet */
	DPRINTF("if_output: send to %d.%d.%d.%d\n",
	        ip4_addr1(ipaddr), ip4_addr2(ipaddr),
	        ip4_addr3(ipaddr), ip4_addr3(ipaddr));


	if (!if_added || !netif_is_up(&if_netif))
		return ERR_IF;

	if (if_config.ifc_flags & IFF_ARP)
		return etharp_output(netif, ipaddr, p);
	else
		return rserv_if_output(netif, p);
}

static void
rserv_if_input_free(void *cb_arg)
{
	struct lbuf *pkt = cb_arg;
	PKTFREE(if_osh, pkt, FALSE);
}

void
rserv_if_input(struct lbuf *pkt)
{
	struct netif *netif = &if_netif;
	struct pbuf *p;
	unsigned char *msg;
	int msglen;
	struct eth_hdr *ethhdr;

	STAMP(120);
	DPRINTF("rserv_if_input: pkt=%p\n", (void *)pkt);

	if (!if_added || !netif_is_up(&if_netif)) {
		DPRINTF("rserv_if_input: interface down\n");
		PKTFREE(if_osh, pkt, FALSE);
		return;
	}

	msg = PKTDATA(if_osh, pkt);
	msglen = PKTLEN(if_osh, pkt);

	/* Not yet handling receive packets that come in multiple pieces */
	ASSERT(PKTNEXT(if_osh, pkt) == NULL);

#ifdef SHOW_PKTS
	printf("in(%d)\n", msglen);
#endif

	/* Allocate a reference pbuf pointing to the lbuf payload */
	if ((p = pbuf_alloc(PBUF_RAW, (u16_t)msglen, PBUF_REF)) == NULL) {
		DPRINTF("rserv_if_input: out of memory, packet dropped\n");
		PKTFREE(if_osh, pkt, FALSE);
		LINK_STATS_INC(link.memerr);
		LINK_STATS_INC(link.drop);
		if_stats.rxdrop++;
		return;
	}

	p->payload = msg;

	p->free_cb = rserv_if_input_free;
	p->free_cb_arg = pkt;

	LINK_STATS_INC(link.recv);
	if_stats.rxframe++;
	if_stats.rxbyte += msglen;

	/* Determine protocol and handle packet */
	ethhdr = p->payload;

	switch (htons(ethhdr->type)) {
	case ETHTYPE_IP:
		/* update ARP table */
		if (if_config.ifc_flags & IFF_ARP)
			etharp_ip_input(netif, p);
		/* skip Ethernet header */
		pbuf_header(p, -(s16_t)sizeof(struct eth_hdr));
		/* pass to network layer */
		ASSERT(netif->input != NULL);
		netif->input(p, netif);
		break;

	case ETHTYPE_ARP:
		if (if_config.ifc_flags & IFF_ARP) {
			/* pass p to ARP module  */
			etharp_arp_input(netif, (struct eth_addr *)netif->hwaddr, p);
		}
		break;

	default:
		fprintf(stderr, "rserv_if_input: dropping packet of unknown type\n");
		pbuf_free(p);
		break;
	}
}

static void
if_etharp_poll(hnd_timer_t *t)
{
	etharp_tmr();
}

static void
if_term(struct netif *netif)
{
	if (if_etharp_timer) {
		hnd_timer_stop(if_etharp_timer);
		if_etharp_timer = NULL;
	}
}

int
rserv_if_deconfig(void)
{
	if (if_added) {
		if (dhcp_started) {
			dhcp_release(&if_netif);
			dhcp_stop(&if_netif);
			dhcp_started = 0;
		}
		if_term(&if_netif);
		netif_remove(&if_netif);
		memset(&if_netif, 0, sizeof(if_netif));
		if_added = 0;
	}

	return 0;
}

int
rserv_if_init(struct dngl *dngl, struct ether_addr *macaddr)
{
	if_dngl = dngl;
	if_osh = dngl->osh;

	memset(&if_config, 0, sizeof(if_config));
	memcpy(&if_config.ifc_hwaddr.sa_data, macaddr, sizeof(struct ether_addr));
	if_config.ifc_flags = (IFF_HWADDR | IFF_ARP | IFF_BROADCAST | IFF_MTU | IFF_METRIC);
	strcpy(if_config.ifc_name, IFNAME);
	if_config.ifc_mtu = IF_MTU_DEFAULT;
	if_config.ifc_metric = 1;

	memset(&if_stats, 0, sizeof(if_stats));

	return 0;
}

static err_t
if_init(struct netif *netif)
{
	DPRINTF("if_init: input fn=%p\n", netif->input);

	netif->state = &if_config;
	netif->name[0] = if_config.ifc_name[0];
	netif->name[1] = if_config.ifc_name[1];
	netif->output = if_output;
	netif->linkoutput = rserv_if_output;

	/* set MAC hardware address */
	netif->hwaddr_len = ETHER_ADDR_LEN;
	memcpy(netif->hwaddr, if_config.ifc_hwaddr.sa_data, ETHER_ADDR_LEN);

	/* set maximum transfer unit */
	netif->mtu = (if_config.ifc_flags & IFF_MTU) ? if_config.ifc_mtu : IF_MTU_DEFAULT;

	/* broadcast capability */
	if (if_config.ifc_flags & IFF_BROADCAST)
		netif->flags |= NETIF_FLAG_BROADCAST;
	else
		netif->flags &= NETIF_FLAG_BROADCAST;

	if (if_config.ifc_flags & IFF_ARP) {
		etharp_init();

		if_etharp_timer = hnd_timer_create(NULL, NULL, if_etharp_poll, NULL);
		ASSERT(if_etharp_timer);

		if (!hnd_timer_start(if_etharp_timer, ARP_TMR_INTERVAL, TRUE))
			ASSERT(FALSE);
	}

	if (if_config.ifc_flags & IFF_UP) {
		DPRINTF("if_init: set up\n");
		netif_set_up(netif);
	}

	return ERR_OK;
}

int
rserv_if_config_set(struct rsock_ifconfig *ifc)
{
	struct ip_addr ipaddr, netmask, gw;
	struct sockaddr_in *sin;
	int ok = 1;

	(void)rserv_if_deconfig();

	printf("\nifconfig:\n");

	/*
	 * Configure by index if (index >= 0), name if (index < 0).
	 * Currently only one interface is supported and index/name are ignored.
	 */
	if (ifc->ifc_index > 0)
		printf("  Index: %d\n", ifc->ifc_index);
	else
		printf("  Name: %s\n", ifc->ifc_name);

	printf("  Flags: 0x%x\n", (unsigned int)ifc->ifc_flags);
	if_config.ifc_flags = ifc->ifc_flags;

	if (ifc->ifc_flags & IFF_ADDR) {
		memcpy(&if_config.ifc_addr, &ifc->ifc_addr, sizeof(if_config.ifc_addr));
		sin = (struct sockaddr_in *)&ifc->ifc_addr;
		printf("  Addr: 0x%x\n", (unsigned int)ntohl(sin->sin_addr.s_addr));
		memcpy(&ipaddr, &sin->sin_addr, sizeof(ipaddr));
		/* No routing; using addr for gateway */
		printf("  Gateway: 0x%x\n", (unsigned int)ntohl(sin->sin_addr.s_addr));
		memcpy(&gw, &ipaddr, sizeof(gw));
	} else if (ifc->ifc_flags & IFF_DHCP) {
		memset(&ipaddr, 0, sizeof(ipaddr));
		memset(&gw, 0, sizeof(gw));
		printf("  Addr: DHCP\n");
	} else {
		printf("  Addr: none\n");
		ok = 0;
	}

	if (ifc->ifc_flags & IFF_NETMASK) {
		memcpy(&if_config.ifc_netmask, &ifc->ifc_netmask, sizeof(if_config.ifc_netmask));
		sin = (struct sockaddr_in *)&ifc->ifc_netmask;
		printf("  Netmask: 0x%x\n", (unsigned int)ntohl(sin->sin_addr.s_addr));
		memcpy(&netmask, &sin->sin_addr, sizeof(netmask));
	} else if (ifc->ifc_flags & IFF_DHCP) {
		memset(&netmask, 0, sizeof(netmask));
		printf("  Netmask: DHCP\n");
	} else {
		printf("  Netmask: none\n");
		ok = 0;
	}

	if (ifc->ifc_flags & IFF_BROADADDR) {
		/* Broadcast addr can be set/gotten but is not yet used */
		memcpy(&if_config.ifc_broadaddr, &ifc->ifc_broadaddr,
		       sizeof(if_config.ifc_broadaddr));
	}

	if (ok) {
		netif_add(&if_netif, &ipaddr, &netmask, &gw, NULL, if_init, if_input);
		if_added = 1;
		netif_set_default(&if_netif);

		if (ifc->ifc_flags & IFF_DHCP) {
			if (dhcp_start(&if_netif) == ERR_OK)
				dhcp_started = 1;
		}
	} else {
		printf("   ***Interface not configured\n");
	}

	return 0;
}

int
rserv_if_config_get(struct rsock_ifconfig *ifc)
{
	struct sockaddr_in *sin;

	/* Update information which may have changed since last set, due to DHCP */

	sin = (struct sockaddr_in *)&if_config.ifc_addr;
	memcpy(&sin->sin_addr, &if_netif.ip_addr, sizeof(sin->sin_addr));
	sin = (struct sockaddr_in *)&if_config.ifc_netmask;
	memcpy(&sin->sin_addr, &if_netif.netmask, sizeof(sin->sin_addr));

	/*
	 * Currently only one interface is supported.
	 * ifc->ifc_index/name are ignored.
	 */

	memcpy(ifc, &if_config, sizeof(struct rsock_ifconfig));

	return 0;
}

int
rserv_if_stats_get(struct rsock_ifstats *ifs)
{
	/*
	 * On entry, ifs contains the interface name.
	 * Currently only one interface is supported, and the name is ignored.
	 */

	memcpy(ifs, &if_stats, sizeof(struct rsock_ifstats));

	return 0;
}

int
rserv_if_control(struct rsock_ifcontrol *ifctl, int len)
{
	DPRINTF("rserv_if_control: ifctl=%p cmd=%d len=%d\n",
	        (void *)ifctl, ifctl->cmd, ifctl->datalen);

	dngl_dev_ioctl(if_dngl, ifctl->cmd, ifctl + 1, ifctl->datalen);

	return 0;
}
