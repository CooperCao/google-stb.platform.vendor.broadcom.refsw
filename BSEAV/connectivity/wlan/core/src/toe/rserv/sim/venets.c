/*
 * Virtual Ethernet Switch
 *
 * This file implements a virtual Ethernet switch.  It contains a L2
 * address resolution table and supports unicast switching, broadcast
 * switching, learning and Destination Lookup Failure flooding of
 * Ethernet-format packets.
 *
 * Packets are sent and received on TCP ports which represent physical
 * switch ports.  By default, it waits for connections on four TCP ports
 * (6500-6503).
 *
 * Packets follow the VENET protocol, in which each packet is prefixed
 * by a 4-byte big-endian packet length value.
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
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

#include "venet.h"

#define OPT_PORT_BASE_DEFAULT	VENET_PORT_BASE
#define OPT_PORT_BASE_MIN	1
#define OPT_PORT_BASE_MAX	65535

#define OPT_PORT_COUNT_DEFAULT	4
#define OPT_PORT_COUNT_MIN	2
#define OPT_PORT_COUNT_MAX	16

static int opt_port_base = OPT_PORT_BASE_DEFAULT;
static int opt_port_count = OPT_PORT_COUNT_DEFAULT;
static int opt_v;
static int opt_x;
static char *opt_l;
static double opt_D;
static double opt_E;

static char *prog_name;

#define VPRINTF		if (opt_v) printf

static void
usage(void)
{
	fprintf(stderr,
	        "Usage: %s [-vx] [-p <base>] [-c <count>] [-l <prefix>] [-D <prob>] [-E <prob>]\n",
	        prog_name);
	fprintf(stderr,
	        "    -p <base>    Switch port base (default %d)\n", OPT_PORT_BASE_DEFAULT);
	fprintf(stderr,
	        "    -c <count>   Specify number of ports (default %d)\n", OPT_PORT_COUNT_DEFAULT);
	fprintf(stderr,
	        "    -v           Dump real time traffic information\n");
	fprintf(stderr,
	        "    -x           Dump contents of each packet in hex\n");
	fprintf(stderr,
	        "    -l <prefix>  Write pcap output to files named <prefix>.<port_num>\n");
	fprintf(stderr,
	        "    -D <prob>    Specify probability of dropping a packet (default 0.0)\n");
	fprintf(stderr,
	        "    -E <prob>    Specify probability of corrupting a packet (default 0.0)\n");
	exit(1);
}

/*
 * Structure corresponding to a virtual switch port
 */

struct lan_port {
	pthread_t thread;

	struct {
		int fd;
		socklen_t addr_len;
		struct sockaddr_in addr;
	} listen;

	struct {
		int fd;
		socklen_t addr_len;
		struct sockaddr_in addr;
	} client;

	int linked;
	sem_t output_lock;

	/* Currently supporting only one host per port */
	unsigned char macaddr[6];

	FILE *pcap_fp;
};

struct lan_port *ports;

#define PORT_NUM(port)		((int)((port) - ports))

static void
ethdump(unsigned char *msg, int msglen)
{
	int i, col;

	if (msglen < 14) {
		printf("SHORT PACKET:");
		for (i = 0; i < msglen; i++)
			printf(" %02x", msg[i]);
		printf("\n");
		return;
	}

	printf("\t{%02x %02x %02x %02x %02x %02x} {%02x %02x %02x %02x %02x %02x} {%02x %02x}\n",
	       msg[0], msg[1], msg[2], msg[3], msg[4], msg[5],
	       msg[6], msg[7], msg[8], msg[9], msg[10], msg[11],
	       msg[12], msg[13]);

	col = 0;

	for (i = 14; i < msglen; i++, col++) {
		if (col % 16 == 0)
			printf("\t");
		printf("%02x", msg[i]);
		if ((col + 1) % 16 == 0)
			printf("\n");
		else
			printf(" ");
	}

	if (col % 16 != 0)
		printf("\n");
}

static void
sigpipe(int signum)
{
}

static sem_t rand_lock;

static void
rand_init(void)
{
	srandom(time(0) + getpid());

	sem_init(&rand_lock, 0, 1);
}

static int
rand_num(int top)
{
	int num;
	sem_wait(&rand_lock);
	num = (int)((double)top * random() / (RAND_MAX + 1.0));
	if (num < 0 || num >= top) {
		fprintf(stderr, "Bad RNG\n");
		num = 0;
	}
	sem_post(&rand_lock);
	return num;
}

static int
rand_check(double prob)
{
	int ok;
	sem_wait(&rand_lock);
	ok = ((double)random() / RAND_MAX) < prob;
	sem_post(&rand_lock);
	return ok;
}

/*
 * Wait for specified number of bytes on socket.
 * Returns number of bytes read, or -1 on error and sets errno.
 * If less is read than requested, the connection was closed.
 */

static int
readn(int fd, void *buf, size_t n)
{
	int rv;
	size_t got = 0;

	while (got < n) {
		if ((rv = read(fd, (unsigned char *)buf + got, n - got)) < 0) {
			if (errno == EINTR)
				continue;
			return rv;
		}
		if (rv == 0)
			break;
		got += rv;
	}

	return got;
}

static void
pcap_open(struct lan_port *port)
{
	char pcap_fname[NAME_MAX];
	unsigned int v32;
	unsigned short v16;

	if (opt_l == NULL)
		return;

	sprintf(pcap_fname, "%s.%d", opt_l, PORT_NUM(port));

	if ((port->pcap_fp = fopen(pcap_fname, "w")) == NULL) {
		fprintf(stderr,
		        "<%d> error opening %s file writing: %s\n",
		        PORT_NUM(port), pcap_fname, strerror(errno));
		exit(1);
	}

	/* Write PCAP header */
	v32 = 0xa1b2c3d4;		/* Magic */
	fwrite(&v32, sizeof(v32), 1, port->pcap_fp);
	v16 = 2;			/* PCAP_VERSION_MAJOR */
	fwrite(&v16, sizeof(v16), 1, port->pcap_fp);
	v16 = 4;			/* PCAP_VERSION_MINOR */
	fwrite(&v16, sizeof(v16), 1, port->pcap_fp);
	v32 = 0;			/* Time zone offset */
	fwrite(&v32, sizeof(v32), 1, port->pcap_fp);
	v32 = 0;			/* Timestamp accuracy */
	fwrite(&v32, sizeof(v32), 1, port->pcap_fp);
	v32 = 1522;			/* Snapshot length (max packet size) */
	fwrite(&v32, sizeof(v32), 1, port->pcap_fp);
	v32 = 1;			/* Link layer field (1 = 10Mbps Ethernet) */
	fwrite(&v32, sizeof(v32), 1, port->pcap_fp);
}

static void
pcap_close(struct lan_port *port)
{
	if (port->pcap_fp != NULL) {
		fclose(port->pcap_fp);
		port->pcap_fp = NULL;
	}
}

static void
pcap_log(struct lan_port *port, unsigned char *buf, int len)
{
	if (port->pcap_fp != NULL) {
		struct timeval tv;
		unsigned int v32;

		gettimeofday(&tv, NULL);

		fwrite(&tv, sizeof(tv), 1, port->pcap_fp);
		v32 = (unsigned int)len;	/* Number of bytes of packet captured */
		fwrite(&v32, sizeof(v32), 1, port->pcap_fp);
		v32 = (unsigned int)len;	/* Number of bytes in packet */
		fwrite(&v32, sizeof(v32), 1, port->pcap_fp);
		fwrite(buf, 1, len, port->pcap_fp);	/* Raw data */

		fflush(port->pcap_fp);
	}
}

static void
lan_send(struct lan_port *port, unsigned char *enet_msg, int enet_msglen)
{
	int msglen = htonl(enet_msglen);

	VPRINTF("<%d> TX length %d\n", PORT_NUM(port), enet_msglen);
	if (opt_x)
		ethdump(enet_msg, enet_msglen);

	sem_wait(&port->output_lock);
	write(port->client.fd, &msglen, 4);
	write(port->client.fd, enet_msg, enet_msglen);
	sem_post(&port->output_lock);

	pcap_log(port, enet_msg, enet_msglen);
}

static void
lan_broadcast(int src_port, unsigned char *enet_msg, int enet_msglen)
{
	int i;

	for (i = 0; i < opt_port_count; i++)
		if (i != src_port && ports[i].linked)
			lan_send(&ports[i], enet_msg, enet_msglen);
}

#define MAC_BCAST(mac) \
	(((mac)[0] & (mac)[1] & (mac)[2] & (mac)[3] & (mac)[4] & (mac)[5]) == 0xff)

#define MAC_EQUAL(mac1, mac2) \
	(memcmp((mac1), (mac2), 6) == 0)

#define MAC_COPY(dst, src) \
	(memcpy((dst), (src), 6))

static void
lan_switch(int src_port, unsigned char *enet_msg, int enet_msglen)
{
	unsigned char *enet_da = enet_msg;
	unsigned char *enet_sa = enet_msg + 6;
	int dst_port;
	int i;

	/* Random drop for network testing */
	if (rand_check(opt_D)) {
		printf("<%d> RANDOM DROP len=%d\n", src_port, enet_msglen);
		return;
	}

	/* Random corruption for network testing */
	if (rand_check(opt_E)) {
		enet_msg[rand_num(enet_msglen)] ^= 0xff;
		printf("<%d> RANDOM CORRUPTION len=%d\n", src_port, enet_msglen);
	}

	/* From broadcast address illegal */
	if (MAC_BCAST(enet_sa))
		return;

	/* To own address illegal */
	if (MAC_EQUAL(enet_da, ports[src_port].macaddr))
		return;

	/* New address learning */
	if (!MAC_EQUAL(enet_sa, ports[src_port].macaddr)) {
		VPRINTF("<%d> LEARN %x:%x:%x:%x:%x:%x\n",
		        src_port,
		        enet_sa[0], enet_sa[1], enet_sa[2],
		        enet_sa[3], enet_sa[4], enet_sa[5]);
		MAC_COPY(ports[src_port].macaddr, enet_sa);
	}

	/* To broadcast address */
	if (MAC_BCAST(enet_da)) {
		lan_broadcast(src_port, enet_msg, enet_msglen);
		return;
	}

	/* Resolve address to destination port */
	dst_port = -1;
	for (i = 0; i < opt_port_count; i++)
		if (i != src_port &&
		    ports[i].linked &&
		    MAC_EQUAL(enet_da, ports[i].macaddr)) {
			if (dst_port < 0)
				dst_port = i;
			else {
				VPRINTF("<%d> DUPLICATE MAC %x:%x:%x:%x:%x:%x on port %d\n",
				        src_port,
				        enet_da[0], enet_da[1], enet_da[2],
				        enet_da[3], enet_da[4], enet_da[5],
				        dst_port);
			}
		}

	/* Destination Lookup Failure flooding */
	if (dst_port < 0) {
		lan_broadcast(src_port, enet_msg, enet_msglen);
		return;
	}

	/* Unicast switching */
	lan_send(&ports[dst_port], enet_msg, enet_msglen);
}

static void *
lan_port_thread(void *arg)
{
	struct lan_port *port = arg;
	int port_num = PORT_NUM(port);
	int one, n;
	int enet_msglen;
	unsigned char enet_msg[VENET_MTU];
	unsigned int pa;

	if ((port->listen.fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr,
		        "<%d> error creating socket: %s\n",
		        port_num, strerror(errno));
		exit(1);
	}

	one = 1;

	if (setsockopt(port->listen.fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0)
		fprintf(stderr,
		        "<%d> error setting SO_REUSEADDR: %s\n",
		        port_num, strerror(errno));

	port->listen.addr.sin_family = AF_INET;
	port->listen.addr.sin_addr.s_addr = INADDR_ANY;
	port->listen.addr.sin_port = htons(opt_port_base + port_num);

	port->listen.addr_len = (socklen_t)sizeof(port->listen.addr);

	if (bind(port->listen.fd,
	         (struct sockaddr *)&port->listen.addr,
	         port->listen.addr_len) < 0) {
		fprintf(stderr,
		        "<%d> error binding socket: %s\n",
		        port_num, strerror(errno));
		exit(1);
	}

	if (listen(port->listen.fd, 1) < 0) {
		fprintf(stderr,
		        "<%d> error listening on socket: %s\n",
		        port_num, strerror(errno));
		exit(1);
	}

	sem_init(&port->output_lock, 0, 1);

	printf("<%d> READY (listening on port %d)\n", port_num, opt_port_base + port_num);

	for (;;) {
		port->client.addr_len = sizeof(port->client.addr);

		if ((port->client.fd = accept(port->listen.fd,
		                              (struct sockaddr *)&port->client.addr,
		                              &port->client.addr_len)) < 0) {
			if (errno == EINTR)
				continue;
			fprintf(stderr,
			        "<%d> error accepting connection: %s\n",
			        port_num, strerror(errno));
			exit(1);
		}

		port->linked = 1;

		pa = ntohl(port->client.addr.sin_addr.s_addr);

		printf("<%d> LINK UP (remote peer %d.%d.%d.%d)\n",
		       port_num,
		       (pa >> 24) & 0xff, (pa >> 16) & 0xff, (pa >> 8) & 0xff, pa & 0xff);

		pcap_open(port);

		while (1) {
			/*
			 * Read length.
			 *
			 * If remote side disconnects (a virtual cable disconnect),
			 * we either get an ECONNRESET or a short read length.
			 */

			if ((n = readn(port->client.fd, &enet_msglen, 4)) < 0) {
				if (errno != ECONNRESET)
					fprintf(stderr,
					        "<%d> read error: %s\n", port_num, strerror(errno));
				break;
			}

			if (n < 4)
				break;

			enet_msglen = ntohl(enet_msglen);

			if (enet_msglen <= 0 || enet_msglen > VENET_MTU) {
				fprintf(stderr,
				        "<%d> bad length 0x%x\n", port_num, enet_msglen);
				break;
			}

			/* Read packet */

			if ((n = readn(port->client.fd, enet_msg, enet_msglen)) < 0) {
				if (errno != ECONNRESET)
					fprintf(stderr,
					        "<%d> read error: %s\n", port_num, strerror(errno));
				break;
			}

			if (n < enet_msglen)
				break;

			VPRINTF("<%d> RX length %d\n", port_num, enet_msglen);
			if (opt_x)
				ethdump(enet_msg, enet_msglen);

			/* Switch packet */

			pcap_log(port, enet_msg, enet_msglen);

			lan_switch(port_num, enet_msg, enet_msglen);
		}

		pcap_close(port);

		close(port->client.fd);

		port->linked = 0;

		printf("<%d> LINK DOWN\n", port_num);

		/* Remove L2 table entry */
		memset(ports[port_num].macaddr, 0, sizeof(ports[port_num].macaddr));
	}
}

int
main(int argc, char **argv)
{
	extern char *optarg;
	int c, i, rv;

	prog_name = argv[0];

	while ((c = getopt(argc, argv, "p:c:xvl:D:E:")) != EOF)
		switch (c) {
		case 'p':
			opt_port_base = (int)strtol(optarg, 0, 0);
			break;
		case 'c':
			opt_port_count = (int)strtol(optarg, 0, 0);
			break;
		case 'x':
			opt_x = 1;
			break;
		case 'v':
			opt_v = 1;
			break;
		case 'l':
			opt_l = optarg;
			break;
		case 'D':
			opt_D = atof(optarg);
			break;
		case 'E':
			opt_E = atof(optarg);
			break;
		default:
			usage();
		}

	if (opt_port_base < OPT_PORT_BASE_MIN ||
	    opt_port_base > OPT_PORT_BASE_MAX ||
	    opt_port_count < OPT_PORT_COUNT_MIN ||
	    opt_port_count > OPT_PORT_COUNT_MAX)
		usage();

	if (opt_x)
		opt_v = 1;

	signal(SIGPIPE, sigpipe);

	if ((ports = malloc(opt_port_count * sizeof(struct lan_port))) == NULL) {
		fprintf(stderr, "%s: out of memory\n", prog_name);
		exit(1);
	}

	memset(ports, 0, opt_port_count * sizeof(struct lan_port));

	rand_init();

	/*
	 * Start one thread per virtual switch port
	 */

	for (i = 0; i < opt_port_count; i++)
		if ((rv = pthread_create(&ports[i].thread, NULL, lan_port_thread, &ports[i])) < 0) {
			fprintf(stderr,
			        "<%d> could not fork lan_port_thread: %s\n",
			        i, strerror(rv));
			exit(1);
		}

	pause();

	exit(0);

	/*NOTREACHED*/
	return 0;
}
