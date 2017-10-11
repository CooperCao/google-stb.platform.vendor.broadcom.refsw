/*
 * Quick and dirty ip packet constructor.
 * Create an ad-hoc IP UDP packet.
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <typedefs.h>

void err(char *s);
void usage(void);
void ip_cksum(ushort *sump, uchar *buf, int len);
uint ethernet_crc(uint crc, uchar *data, int len);

static int sport = 1024;
static int dport = 1024;
static int bad_udp_cksum = 0;
static int add_vlan_tag = 0;
static int vlan_tag = 0;
static int append_eth_crc = 0;
static int ip_tos = 0;

extern char *optarg;
extern int optind;

int
main(int ac, char *av[])
{
	uchar buf[2048];
	struct ether_addr *ea;
	uchar *p;
	int totlen;
	int iplen;
	int udplen;
	struct in_addr ina;
	int c;
	int i;

	while ((c = getopt(ac, av, "bcd:s:t:v:")) != EOF) {
		switch (c) {
		case 'b':
			bad_udp_cksum = 1;
			break;
		case 'c':
			append_eth_crc = 1;
			break;
		case 'd':	/* destination udp port # */
			dport = strtol(optarg, 0, 0);
			break;
		case 's':
			sport = strtol(optarg, 0, 0);
			break;
		case 't':
			ip_tos = strtol(optarg, 0, 0);
			break;
		case 'v':
			add_vlan_tag = 1;
			vlan_tag = strtol(optarg, 0, 0);
			break;
		default:
			usage();
			break;
		}
	}

	if ((ac - optind) != 5)
		usage();

	p = buf;

	/*
	 * Construct Ethernet header
	 */

	/* dest ether addr */
	if ((ea = ether_aton(av[optind+1])) == NULL)
		err("dest ether addr must be in format a:b:c:d:e:f");
	bcopy(ea, p, ETHER_ADDR_LEN);
	p += ETHER_ADDR_LEN;

	/* src ether addr */
	if ((ea = ether_aton(av[optind])) == NULL)
		err("src ether addr must be in format a:b:c:d:e:f");
	bcopy(ea, p, ETHER_ADDR_LEN);
	p += ETHER_ADDR_LEN;

	/* VLAN tag */
	if (add_vlan_tag) {
		*p++ = 0x81;
		*p++ = 0x00;
		*p++ = (vlan_tag >> 8) & 0xff;
		*p++ = (vlan_tag >> 0) & 0xff;
	}

	/* type = IP */
	*p++ = 0x08;
	*p++ = 0x00;

	/*
	 * Construct IP header
	 */

	/* hl/v */
	*p++ = 0x45;

	/* tos */
	*p++ = ip_tos;

	/* tot_len */
	totlen = atoi(av[optind+4]);
	totlen -= 4;	/* subtract ether frame crc32 size */
	if (totlen < 50)
		err("totlen too small");
	if (totlen > 1514)
		err("totlen too large");
	iplen = totlen - ETHER_HDR_LEN;
	*p++ = (iplen >> 8) & 0xff;
	*p++ = iplen & 0xff;

	/* id */
	*p++ = 0x0;
	*p++ = 0x0;

	/* frag_off */
	*p++ = 0x40;	/* don't fragment */
	*p++ = 0x0;

	/* ttl */
	*p++ = 0x40;

	/* protocol (UDP) */
	*p++ = 17;

	/* ip_sum */
	*p++ = 0x0;
	*p++ = 0x0;

	/* ip_src */
	if (inet_aton(av[optind+2], &ina) == 0)
		err("srcip address must be in a.b.c.d form");
	ina.s_addr = htonl(ina.s_addr);
	*p++ = (ina.s_addr >> 24) & 0xff;
	*p++ = (ina.s_addr >> 16) & 0xff;
	*p++ = (ina.s_addr >> 8) & 0xff;
	*p++ = ina.s_addr & 0xff;

	/* ip_dst */
	if (inet_aton(av[optind+3], &ina) == 0)
		err("dstip address must be in a.b.c.d form");
	ina.s_addr = htonl(ina.s_addr);
	*p++ = (ina.s_addr >> 24) & 0xff;
	*p++ = (ina.s_addr >> 16) & 0xff;
	*p++ = (ina.s_addr >> 8) & 0xff;
	*p++ = ina.s_addr & 0xff;

	/* compute and stuff ip header checksum */
	ip_cksum((ushort*) &buf[ETHER_HDR_LEN + 10], &buf[ETHER_HDR_LEN], 20);

	/*
	 * UDP header
	 */

	/* uh_sport */
	*p++ = (sport >> 8) & 0xff;
	*p++ = sport & 0xff;

	/* uh_dport */
	*p++ = (dport >> 8) & 0xff;
	*p++ = dport & 0xff;

	/* uh_ulen */
	udplen = totlen - ETHER_HDR_LEN - 20;
	*p++ = (udplen >> 8) & 0xff;
	*p++ = udplen & 0xff;

	/* uh_usum (zero = no checksum) */
	if (bad_udp_cksum) {
		*p++ = 1;
		*p++ = 1;
	} else {
		*p++ = 0;
		*p++ = 0;
	}

	/*
	 * Pad frame out to totlen with pattern but don't attempt to create
	 * a valid udp header.  This will result in udp checksum errors at
	 * the destination and the packet discards which is what we want.
	 * We *don't* want the remote destination sending back ICMP error packets.
	 */
	for (i = (p - buf); i < totlen; i++)
		*p++ = i & 0xff;

	/*
	 * CRC
	 */

	if (append_eth_crc) {
		uint crc = ~ethernet_crc(~0, buf, p - buf);
		*p++ = (crc >> 24) & 0xff;
		*p++ = (crc >> 16) & 0xff;
		*p++ = (crc >> 8) & 0xff;
		*p++ = (crc >> 0) & 0xff;
		totlen += 4;
	}

	/* write the frame (in ASCII!) to stdout */
	printf("# ");
	for (i = 0; i < ac; i++)
		printf("%s ", av[i]);
	printf("\n");
	for (i = 0; i < totlen; i++) {
		printf("%02x ", buf[i] & 0xff);
		if ((i % 16) == 15)
			printf("\n");
	}
	if ((i % 16) != 0)
		printf("\n");

	return (0);
}

void
usage()
{
	fprintf(stderr, "usage: mkip [ options ] srcether dstether srcip dstip totlen\n");
	fprintf(stderr, "\t-b: insert bad (invalid) udp checksum\n");
	fprintf(stderr, "\t-c: append correct ethernet CRC\n");
	fprintf(stderr, "\t-s port: specify udp source port value\n");
	fprintf(stderr, "\t-d port: specify udp destination port value\n");
	fprintf(stderr, "\t-v tag: add 802.1Q VLAN tag using specified 16-bit tag\n");
	fprintf(stderr, "\t-t tos: specify IP DiffServ/TOS byte\n");

	exit(1);
}

void
err(char *s)
{
	fprintf(stderr, s);
	fprintf(stderr, "\n");
	exit(1);
}

void
ip_cksum(ushort *sump, uchar *buf, int len)
{
	ulong sum = *sump;
	ushort val;

	while (len > 1) {
		val = *buf++ << 8;
		val |= *buf++;
		sum += val;
		len -= 2;
	}

	if (len > 0) {
		sum += (*buf) << 8;
	}

	while (sum >> 16) {
		sum = (sum & 0xffff) + (sum >> 16);
	}
	*sump = htons(~sum);
}

/*
 * To generate CRC, do not include CRC field in data:
 *    uint crc = ~ethernet_crc(~0, data, len)
 *
 * To check CRC, include CRC field in data:
 *    uint result = ethernet_crc(~0, data, len)
 *    If CRC is correct, result will be 0xe320bbde.
 *
 * The 8 MSbits of the return value are the first byte of the CRC.
 */

uint
ethernet_crc(uint crc, uchar *data, int len)
{
	static uint inited = 0, crc_table[256];
	int i;

	if (!inited) {
		for (i = 0; i < 256; i++) {
			uint j, accum = i;

			for (j = 0; j < 8; j++) {
				if (accum & 1)
					accum = accum >> 1 ^ 0xedb88320UL;
				else
					accum = accum >> 1;
			}

			crc_table[i] = (((accum >> 24) & 0x000000ff) |
			                ((accum >>  8) & 0x0000ff00) |
			                ((accum <<  8) & 0x00ff0000) |
			                ((accum << 24) & 0xff000000));
		}

		inited = 1;
	}

	for (i = 0; i < len; i++)
		crc = crc << 8 ^ crc_table[crc >> 24 ^ data[i]];

	return crc;
}
