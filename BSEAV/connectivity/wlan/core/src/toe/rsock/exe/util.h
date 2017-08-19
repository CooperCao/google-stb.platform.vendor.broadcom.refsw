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

#ifndef _UTIL_H
#define _UTIL_H

#define SOCK_SIN(sa) \
	((struct sockaddr_in *)(sa))

#define SOCK_SIN_ADDR(sa) \
	(SOCK_SIN(sa)->sin_addr)

#define SOCK_S_ADDR(sa) \
	(SOCK_SIN_ADDR(sa).s_addr)

#define S_ADDR(a, b, c, d) \
	htonl(((uint32)((a) & 0xff) << 24) | \
	      ((uint32)((b) & 0xff) << 16) | \
	      ((uint32)((c) & 0xff) <<	8) | \
	      ((uint32)((d) & 0xff) <<	0))

int inet_guess_netmask(struct sockaddr *sa, struct sockaddr *sm);
int inet_addr_parse(const char *cp, struct sockaddr *sa);
void inet_addr_format(char *cp, struct sockaddr *sa);
void ether_addr_format(char *cp, struct sockaddr *sa);

int ugetopt(int *optind, char **optarg, int argc, char *argv[], char *optstr);
unsigned long ustrtoul(const char *str, char **ep, int base);

#endif /* _UTIL_H */
