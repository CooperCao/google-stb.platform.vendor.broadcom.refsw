/*
 * RSock Client Utilities
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

#include "os.h"
#include <rsock/socket.h>

#include "util.h"

struct {
	uint32	addr_mask;
	uint32	addr_val;
	uint32	net_mask;
} std_addr[] = {
	{	0xffff0000, 0xc0a80000, 0xffffff00	},
	{	0xfff00000, 0xac100000, 0xffff0000	},
	{	0xff000000, 0x0a000000, 0xff000000	}
};

#define STD_ADDR_COUNT		((int)(sizeof(std_addr) / sizeof(std_addr[0])))

int
inet_guess_netmask(struct sockaddr *sa, struct sockaddr *sm)
{
	int i;
	uint32 addr = ntohl(SOCK_S_ADDR(sa));

	os_memcpy(sm, sa, sizeof(struct sockaddr));

	for (i = 0; i < STD_ADDR_COUNT; i++)
		if ((addr & std_addr[i].addr_mask) == std_addr[i].addr_val) {
			SOCK_S_ADDR(sm) = htonl(std_addr[i].net_mask);
			return 1;
		}

	return 0;
}

/* Returns non-zero if the address string is parsable */
int
inet_addr_parse(const char *cp, struct sockaddr *sa)
{
	char *ecp;
	unsigned int addr;
	int i;

	/* Assume a raw address value if it's a plain hex/dec/oct number */

	addr = (uint32)ustrtoul(cp, &ecp, 0);

	if (*ecp != 0) {
		addr = 0;

		for (i = 0; i < 4; i++) {
			addr = (addr << 8) | (uint32)ustrtoul(cp, &ecp, 0);
			if ((i < 3 && *ecp != '.') || (i == 3 && *ecp != 0))
				return 0;
			cp = ecp + 1;
		}
	}

	os_memset(sa, 0, sizeof(*sa));

	SOCK_SIN(sa)->sin_family = AF_INET;
	SOCK_S_ADDR(sa) = htonl(addr);

	return 1;
}

void
inet_addr_format(char *cp, struct sockaddr *sa)
{
	uint32 addr = ntohl(SOCK_S_ADDR(sa));

	os_sprintf(cp, "%d.%d.%d.%d",
	           (addr >> 24) & 0xff, (addr >> 16) & 0xff,
	           (addr >> 8) & 0xff, (addr >> 0) & 0xff);
}

void
ether_addr_format(char *cp, struct sockaddr *sa)
{
	unsigned char *mac = (unsigned char *)sa->sa_data;

	os_sprintf(cp, "%02x:%02x:%02x:%02x:%02x:%02x",
	           mac[0], mac[1], mac[2],
	           mac[3], mac[4], mac[5]);
}

int
ugetopt(int *optind, char **optarg, int argc, char *argv[], char *optstr)
{
	int opt, i;

	for (;;) {
		/* Check for end of arguments */
		if (*optind == argc)
			return -1;

		/* Check for end of options */
		if (argv[*optind][0] != '-')
			return -1;

		if ((opt = argv[*optind][1]) != 0)
			break;

		/* Skip lone dash (also results from multiple-character options parsing below) */

		(*optind)++;
	}

	/* Double dash ends options parsing */
	if (opt == '-') {
		(*optind)++;
		return -1;
	}

	/* Look up option */
	for (i = 0; optstr[i] != 0; i++)
		if (optstr[i] == opt)
			break;

	/* Check for unknown option */
	if (optstr[i] == 0) {
		(*optind)++;
		return '?';
	}

	/* Check for option argument */
	if (optstr[i + 1] == ':') {
		if (argv[*optind][2] == 0) {
			(*optind)++;
			if (*optind == argc || argv[*optind][0] == '-')
				return '?';
			*optarg = argv[*optind];
		} else
			*optarg = &argv[*optind][2];
		(*optind)++;
	} else {
		char *s;

		*optarg = 0;

		/*
		 * More than one non-argument option character allowed (i.e. "-abc");
		 * delete current one so next call will get the next one.
		 */

		for (s = &argv[*optind][1]; (s[0] = s[1]) != 0; s++)
			;
	}

	return opt;
}

static int
_ustrtoul_digit(int ch, int base)
{
	int digit = -1;

	if (ch >= '0' && ch <= '9')
		digit = (ch - '0');
	if (ch >= 'A' && ch <= 'Z')
		digit = (ch - 'A' + 10);
	if (ch >= 'a' && ch <= 'z')
		digit = (ch - 'a' + 10);
	if (digit >= base)
		digit = -1;

	return digit;
}

unsigned long
ustrtoul(const char *s, char **ep, int base)
{
	unsigned long n = 0;
	int d, neg = 0;

	if (*s == '-') {
		neg = 1;
		s++;
	} else if (*s == '+')
		s++;

	if (s[0] == '0') {
		if (s[1] == 'x') {
			if (base == 0)
				base = 16;
			else if (base != 16) {
				if (ep != 0)
					*ep = (char *)s + 1;
				return 0;
			}
			s += 2;
		} else if (base == 0)
			base = 8;
	}

	if (base == 0)
		base = 10;

	d = _ustrtoul_digit(s[0], base);

	if (d < 0) {
		if (ep != 0)
			*ep = (char *)s;
		return 0;
	}

	do {
		n = n * base + d;
		s++;
	} while ((d = _ustrtoul_digit(s[0], base)) >= 0);

	if (ep != 0)
		*ep = (char *)s;

	return (neg ? -n : n);
}
