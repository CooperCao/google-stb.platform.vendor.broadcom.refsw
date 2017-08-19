/*
 * RServ Dongle Simulation
 *
 * Uses devbus to simulate the device side of a bus (USB/SDIO) connection.
 * Passes packets from rserv to the bus and vice versa.
 *
 * Uses venetif to simulate the 802.11 connection.
 * Passes packets from lwip to the network interface and vice versa.
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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

#include "os.h"
#include "lwip/opt.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "lwip/ip.h"
#include "lwip/ip_frag.h"
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"
#include "lwip/stats.h"
#include "netif/loopif.h"
#include "venetif.h"
#include "devbus.h"
#include <rsock/types.h>
#include <rsock/proto.h>	/* Included for debugging only */

#include "rserv_sim.h"
#include "rserv.h"

char *prog_name;

int opt_port = DEVBUS_PORT_DEFAULT;

#define DPRINTF			if (0) printf

static struct netif if_netif;
static int if_added = 0;
static struct rsock_ifconfig if_config;
static devbus_t *db;

void
usage(void)
{
	fprintf(stderr,
	        "Usage: %s [-p port]\n",
	        prog_name);
	exit(1);
}

void *
dongle_malloc(int len)
{
	void *ptr;

	if ((ptr = malloc(len)) == NULL) {
		fprintf(stderr, "dongle_malloc: out of memory (fatal)\n");
		exit(1);
	}

	return ptr;
}

struct lbuf *
dongle_pktget(int len, int send)
{
	struct lbuf *lb;

	lb = dongle_malloc(sizeof(struct lbuf));
	lb->next = NULL;
	lb->link = NULL;
	lb->len = len;
	lb->data = dongle_malloc(len);

	return lb;
}

void
dongle_pktfree(struct lbuf *pkt, int send)
{
	struct lbuf *link;

	while (pkt != NULL) {
		link = pkt->link;
		free(pkt->data);
		free(pkt);
		pkt = link;
	}
}

void
dongle_output(struct lbuf *pkt)
{
	uint8 *buf, *ptr;
	struct lbuf *lb;
	int tot_len;

	tot_len = 0;

	for (lb = pkt; lb != NULL; lb = lb->link)
		tot_len += lb->len;

	ASSERT(tot_len >= sizeof(struct rsock_resp_header));
	ASSERT(tot_len <= RSOCK_BUS_MTU);

	/* Flatten lbuf for devbus */

	if ((buf = malloc(tot_len)) == NULL) {
		fprintf(stderr, "dongle_malloc: out of memory (len=%d fatal)\n", tot_len);
		exit(1);
	}

	ptr = buf;

	for (lb = pkt; lb != NULL; lb = lb->link) {
		memcpy(ptr, lb->data, lb->len);
		ptr += lb->len;
	}

	DPRINTF("dongle_output: len=%d {%02x%02x %02x%02x %02x%02x %02x%02x}\n",
	        tot_len,
	        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);

	devbus_output(db, buf, tot_len);

	free(buf);

	dongle_pktfree(pkt, TRUE);
}

#if LWIP_TCP
static int tcp_timer_running;
static void tcpip_timer(void *arg);

void
tcp_timer_needed(void)
{
	/*
	 * lwip calls this whenever the TCP timer is needed but may not
	 * be running (e.g. go from zero connections to one connection).
	 * However, for simplicity we'll let the timer run continuously.
	 */
	if (!tcp_timer_running) {
		tcp_timer_running = 1;
		sys_timeout(TCP_TMR_INTERVAL, tcpip_timer, NULL);
	}
}

static void
tcpip_timer(void *arg)
{
	static uint32 ticks = 0;

	(void)arg;

	/* call TCP timer handler four times per second */
	tcp_tmr();

	/* call IP fragmentation/reassembly timer handler once per second */
	if (++ticks % 4 == 0);
		ip_reass_tmr();

	sys_timeout(TCP_TMR_INTERVAL, tcpip_timer, NULL);
}

#endif /* LWIP_TCP */

/*
 * Input events
 *
 *   The dongle is a state machine driven entirely by three interrupts:
 *   bus input, wireless (or venetif) input, and timers.
 *
 *   In simulation, a separate thread waits for bus input and then
 *   signals the event semaphore.  Similarly, venetif input signals
 *   the same event semaphore.  Timers are based on sys_arch and handled
 *   as part of waiting on said semaphore.
 *
 *   Currently, for simplicity the event queue is only one entry deep.
 */

sys_sem_t event_signal;	/* Used to signal event occurrence */
sys_sem_t event_done;	/* Used to signal event has been processed */
sys_sem_t event_mutex;	/* Used to ensure only one event can be sent/processed at a time */

#define EVENT_BUS	1
#define EVENT_VENETIF	2

struct {
	int type;
	union {
		struct {
			unsigned char *buf;
			int len;
		} bus;
		struct {
			struct pbuf *buf;
			struct netif *netif;
		} venetif;
	} data;
} event;

/* Wireless interface is simulated by venetif */

static err_t
dongle_input(struct pbuf *p, struct netif *netif)
{
	DPRINTF("dongle_input: p=%p netif=%p\n", (void *)p, (void *)netif);

	sys_sem_wait(event_mutex);

	event.type = EVENT_VENETIF;
	event.data.venetif.buf = p;
	event.data.venetif.netif = netif;

	sys_sem_signal(event_signal);
	sys_sem_wait(event_done);

	sys_sem_signal(event_mutex);

	DPRINTF("dongle_input: done\n");

	return ERR_OK;
}

static void
bus_input(unsigned char *buf, int len)
{
	DPRINTF("bus_input: buf=%p len=%d {%02x%02x %02x%02x %02x%02x %02x%02x}\n",
	        buf, len,
	        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);

	sys_sem_wait(event_mutex);

	event.type = EVENT_BUS;
	event.data.bus.buf = buf;
	event.data.bus.len = len;

	sys_sem_signal(event_signal);
	sys_sem_wait(event_done);

	sys_sem_signal(event_mutex);

	DPRINTF("bus_input: done\n");
}

int
dongle_if_deconfig(void)
{
	if (if_added) {
		venetif_stop(&if_netif);
		netif_remove(&if_netif);
		memset(&if_netif, 0, sizeof(if_netif));

#if LWIP_HAVE_LOOPIF
		netif_remove(&loopif);
#endif

		if_added = 0;
	}

	return 0;
}

int
dongle_if_config_set(struct rsock_ifconfig *ifc)
{
	struct ip_addr ipaddr, netmask, gw;
	struct sockaddr_in *sin;
	int ok = 1;

	(void)dongle_if_deconfig();

	printf("\nifconfig:\n");

	/*
	 * Configure by index if (index >= 0), name if (index < 0).
	 * Currently only one interface is supported and index/name are ignored.
	 */
	if (ifc->ifc_index > 0)
		printf("Index: %d\n", (int)ifc->ifc_index);
	else
		printf("Name: %s\n", ifc->ifc_name);

	printf("Flags: 0x%x\n", (unsigned int)ifc->ifc_flags);
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
		netif_add(&if_netif, &ipaddr, &netmask, &gw, NULL, venetif_init, dongle_input);
		netif_set_default(&if_netif);

#if LWIP_HAVE_LOOPIF
		IP4_ADDR(&ipaddr, 127, 0, 0, 1);
		IP4_ADDR(&netmask, 255, 0, 0, 0);
		IP4_ADDR(&gw, 127, 0, 0, 1);

		netif_add(&loopif, &ipaddr, &netmask, &gw, NULL, loopif_init, dongle_input);
#endif

		if_added = 1;
	} else {
		printf("   ***Interface not configured\n");
	}

	return 0;
}

int
dongle_if_config_get(struct rsock_ifconfig *ifc)
{
	/*
	 * Currently only one interface is supported.
	 * ifc->ifc_index/name are ignored.
	 */
	memcpy(ifc, &if_config, sizeof(struct rsock_ifconfig));

	ifc->ifc_flags |= (IFF_ARP | IFF_MTU | IFF_METRIC | IFF_BROADCAST | IFF_HWADDR);
	ifc->ifc_metric = 1;
	venetif_info(&if_netif, &ifc->ifc_mtu, (unsigned char *)ifc->ifc_hwaddr.sa_data);

	return 0;
}

int
dongle_if_stats_get(struct rsock_ifstats *ifs)
{
	/*
	 * On entry, ifs contains the interface name.
	 * Currently only one interface is supported, and the name is ignored.
	 *
	 * Note: this depends upon venetif_stats having same format as rsock_ifstats.
	 */
	ASSERT(sizeof(struct venetif_stats) == sizeof(struct rsock_ifstats));
	venetif_stats_get(&if_netif, (struct venetif_stats *)ifs);
	return 0;
}

int
dongle_if_control(struct rsock_ifcontrol *ifctl, int len)
{
	printf("dongle_if_control: ifctl=%p len=%d\n", (void *)ifctl, len);
	return 0;
}

static void
bus_input_thread(void *arg)
{
	unsigned char *buf;
	int len;

	printf("Waiting for bus connection on port %d\n", opt_port);

	for (;;) {
		len = devbus_read(db, &buf, NULL);

		if (len <= 0) {
			printf("bus_input_thread: stopping due to len=%d\n", len);
			break;
		}

		bus_input(buf, len);
	}

	devbus_close(db);
}

int
main(int argc, char **argv)
{
	extern char *optarg;
	extern int optind;
	int c;
	RSERV_PKT *pkt;
	char *s;

	prog_name = argv[0];

	if ((s = getenv("DEVBUS_PORT")) != NULL)
		opt_port = (int)strtoul(s, 0, 0);

	while ((c = getopt(argc, argv, "p:")) != EOF)
		switch (c) {
		case 'p':
			opt_port = (int)strtol(optarg, 0, 0);
			break;
		default:
			usage();
		}

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

#if LWIP_UDP
	udp_init();
#endif

#if LWIP_TCP
	tcp_init();
#endif

	rserv_init();

	printf("TCP/IP initialized.\n");

	/* Clear initial interface config */

	memset(&if_config, 0, sizeof(if_config));

	/* Initialize bus interface */

	if ((db = devbus_attach(opt_port)) == NULL) {
		fprintf(stderr, "bus_input_thread: fatal error attaching bus\n");
		exit(1);
	}

	/* Start input thread */

	sys_thread_new(bus_input_thread, NULL, DEFAULT_THREAD_PRIO);

	/* Main event loop */

	event_signal = sys_sem_new(0);
	event_done = sys_sem_new(0);
	event_mutex = sys_sem_new(1);

	for (;;) {
		sys_sem_wait(event_signal);

		switch (event.type) {
		case EVENT_BUS:
			DPRINTF("BUS event\n");
			pkt = dongle_pktget(event.data.bus.len, FALSE);
			memcpy(pkt->data, event.data.bus.buf, event.data.bus.len);
			rserv_input(pkt);
			break;

		case EVENT_VENETIF:
			DPRINTF("VENETIF event\n");
			ip_input(event.data.venetif.buf, event.data.venetif.netif);
			break;

		default:
			ASSERT(0);
			break;
		}

		sys_sem_signal(event_done);
	}

	exit(0);

	/*NOTREACHED*/
	return 0;
}
