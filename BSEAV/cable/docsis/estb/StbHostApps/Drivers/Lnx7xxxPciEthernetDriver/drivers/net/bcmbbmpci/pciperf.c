/*
 *
 * Copyright (c) 2002-2005 Broadcom Corporation 
 *
 */
//**************************************************************************
// File Name  : pciperf.c
//
// Description: This is a Linux application that tests performance of PCI network driver
//               
// Updates    : 1/9/2006  wfeng.  Created.
// 
// How to build: mipsel-uclibc-gcc -o pciperf ./pciperf.c
//**************************************************************************
#include <stdio.h>

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include "if_net.h"

static void
usage(char *av0)
{
	fprintf(stderr, "usage: %s interface ip_address port\n", av0);
	exit(1);
}

static void syserr(char *s)
{
	perror(s);
	exit(1);
}

main(int ac, char *av[])
{
	struct ifreq ifr;
	NETFILTER filter;
	
	int s;

	if (ac < 4)
		usage(av[0]);

	filter.ip_addr = inet_addr(av[2]);
	sscanf(av[3], "%ld", &filter.port);
//	printf("Setting IP filter on interface %s\n", av[1]);
//	printf("IP address = %08x, port = %08x\n", filter.ip_addr, filter.port);

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	strncpy(ifr.ifr_name, av[1], sizeof (ifr.ifr_name));
    ifr.ifr_data = (void*)&filter;

	if (ioctl(s, SIOCSNETFILTER, (caddr_t)&ifr) < 0)
		syserr("SIOCSNETFILTER");

	exit(0);
}
