/*
 * RSock Network Interface Configuration Client
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
#include <rsock/plumb.h>
#include "util.h"
#include "ifconfig.h"

static void
usage(void)
{
	os_printerr("Usage: ifconfig <ifname | ifindex> [<address> | [<options> ...]]\n");
	os_printerr("   Options:\n");
	os_printerr("     -a; up|down; dhcp; [-]arp\n");
	os_printerr("     netmask <address>; [-]broadcast [<address>]\n");
	os_printerr("     mtu <n>; metric <n>\n");
	os_printerr("     [-]hw <class> <address>; [-]pointopoint [<address>]\n");
}

static void
ifshow(struct rsock_ifconfig *ifc, struct rsock_ifstats *ifs)
{
	uint32 flags = ifc->ifc_flags;
	char buf[64];

	os_printf("%-10sLink encap:Ethernet  ", ifc->ifc_name);
	if (flags & IFF_HWADDR) {
		ether_addr_format(buf, &ifc->ifc_hwaddr);
		os_printf("HWaddr %s", buf);
	}
	inet_addr_format(buf, &ifc->ifc_addr);
	os_printf("\n          inet %s", buf);
	if (flags & IFF_BROADADDR) {
		inet_addr_format(buf, &ifc->ifc_broadaddr);
		os_printf("  Bcast:%s", buf);
	}
	if (flags & IFF_NETMASK) {
		inet_addr_format(buf, &ifc->ifc_netmask);
		os_printf("  Mask:%s", buf);
	}
	if (flags & IFF_POINTOPOINTADDR) {
		inet_addr_format(buf, &ifc->ifc_p2p);
		os_printf("  PtP:%s", buf);
	}
	os_printf("\n          ");
	if (flags & IFF_UP)
		os_printf("UP ");
	if (flags & IFF_DHCP)
		os_printf("DHCP ");
	if (flags & IFF_BROADCAST)
		os_printf("BROADCAST ");
	if (flags & IFF_MULTICAST)
		os_printf("MULTICAST ");
	if (flags & IFF_ARP)
		os_printf("ARP ");
	if (flags & IFF_POINTOPOINT)
		os_printf("PtP ");
	if (flags & IFF_MTU)
		os_printf("MTU:%d ", ifc->ifc_mtu);
	if (flags & IFF_METRIC)
		os_printf("METRIC:%d ", ifc->ifc_metric);
	os_printf("\n          RX packets:%u bytes:%u errors:%u dropped:%u",
	          ifs->rxframe, ifs->rxbyte, ifs->rxerror, ifs->rxdrop);
	os_printf("\n          TX packets:%u bytes:%u errors:%u dropped:%u\n",
	          ifs->txframe, ifs->txbyte, ifs->txerror, ifs->txdrop);
}

int
ifconfig(int argc, char *argv[])
{
	struct rsock_ifconfig ifc;
	struct rsock_ifstats ifs;
	char *a;
	int i;
	struct sockaddr sa;
	int s = -1, rv = -1;

	os_memset(&ifc, 0, sizeof(ifc));

	if (argc < 2) {
		usage();
		goto done;
	}

	if (os_strcmp(argv[1], "-a") == 0) {
		os_printerr("ifconfig: -a not yet implemented\n");
		goto done;
	}

	/* First argument must be interface index or name */

	a = argv[1];

	if (a[0] >= '0' && a[0] <= '9' && a[1] == 0)
		ifc.ifc_index = (unsigned int)ustrtoul(a, NULL, 0);
	else {
		os_strncpy(ifc.ifc_name, a, sizeof(ifc.ifc_name));
		ifc.ifc_name[sizeof(ifc.ifc_name) - 1] = 0;
	}

	/* Retrieve current interface information */

	if ((s = rsock_socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		os_printerr("rsock_socket(): %s\n", os_strerror(os_errno));
		goto done;
	}

	if (rsock_ioctl(s, SIOCGIFCONFIG, &ifc) < 0) {
		os_printerr("rsock_ioctl(SIOCGIFCONFIG): %s\n", os_strerror(os_errno));
		goto done;
	}

	os_strncpy((char *)&ifs, ifc.ifc_name, IFNAMSIZ);

	if (rsock_ioctl(s, SIOCGIFSTATS, &ifs) < 0) {
		os_printerr("rsock_ioctl(SIOCGIFSTATS): %s\n", os_strerror(os_errno));
		goto done;
	}

	/* If there are no remaining arguments, dump interface info */

	if (argc == 2) {
		ifshow(&ifc, &ifs);
		rv = 0;
		goto done;
	}

	for (i = 2; i < argc; i++) {
		if (inet_addr_parse(argv[i], &sa)) {
			os_memcpy(&ifc.ifc_addr, &sa, sizeof(sa));
			ifc.ifc_flags |= IFF_ADDR;
		} else if (os_strcmp(argv[i], "up") == 0)
			ifc.ifc_flags |= IFF_UP;
		else if (os_strcmp(argv[i], "dhcp") == 0)
			ifc.ifc_flags |= IFF_DHCP;
		else if (os_strcmp(argv[i], "down") == 0)
			ifc.ifc_flags &= ~IFF_UP;
		else if (os_strcmp(argv[i], "-arp") == 0)
			ifc.ifc_flags &= ~IFF_ARP;
		else if (os_strcmp(argv[i], "arp") == 0)
			ifc.ifc_flags |= IFF_ARP;
		else if (os_strcmp(argv[i], "netmask") == 0) {
			if (++i == argc || !inet_addr_parse(argv[i], &sa)) {
				usage();
				goto done;
			}
			os_memcpy(&ifc.ifc_netmask, &sa, sizeof(sa));
			ifc.ifc_flags |= IFF_NETMASK;
		} else if (os_strcmp(argv[i], "-broadcast") == 0) {
			if (i + 1 < argc && inet_addr_parse(argv[i + 1], &sa)) {
				i++;
				os_memcpy(&ifc.ifc_broadaddr, &sa, sizeof(sa));
				ifc.ifc_flags |= IFF_BROADADDR;
			}
			ifc.ifc_flags &= ~IFF_BROADCAST;
		} else if (os_strcmp(argv[i], "broadcast") == 0) {
			if (i + 1 < argc && inet_addr_parse(argv[i + 1], &sa)) {
				i++;
				os_memcpy(&ifc.ifc_broadaddr, &sa, sizeof(sa));
				ifc.ifc_flags |= IFF_BROADADDR;
			}
			ifc.ifc_flags |= IFF_BROADCAST;
		} else if (os_strcmp(argv[i], "mtu") == 0) {
			if (++i == argc) {
				usage();
				goto done;
			}
			ifc.ifc_mtu = (int)ustrtoul(argv[i], NULL, 0);
			ifc.ifc_flags |= IFF_MTU;
		} else if (os_strcmp(argv[i], "metric") == 0) {
			if (++i == argc) {
				usage();
				goto done;
			}
			ifc.ifc_metric = (int)ustrtoul(argv[i], NULL, 0);
			ifc.ifc_flags |= IFF_METRIC;
		} else if (os_strcmp(argv[i], "-pointopoint") == 0) {
			if (i + 1 < argc && inet_addr_parse(argv[i + 1], &sa)) {
				i++;
				os_memcpy(&ifc.ifc_p2p, &sa, sizeof(sa));
				ifc.ifc_flags |= IFF_POINTOPOINTADDR;
			}
			ifc.ifc_flags &= ~IFF_POINTOPOINT;
		} else if (os_strcmp(argv[i], "pointopoint") == 0) {
			if (i + 1 < argc && inet_addr_parse(argv[i + 1], &sa)) {
				i++;
				os_memcpy(&ifc.ifc_p2p, &sa, sizeof(sa));
				ifc.ifc_flags |= IFF_POINTOPOINTADDR;
			}
			ifc.ifc_flags |= IFF_POINTOPOINT;
		} else {
			usage();
			goto done;
		}
	}

	if (ifc.ifc_flags & IFF_ADDR) {
		/*
		 * If netmask or broadcast address has not been configured
		 * before, make sure they're set to something sensible.
		 */

		if (!(ifc.ifc_flags & IFF_NETMASK)) {
			if (!inet_guess_netmask(&ifc.ifc_addr, &ifc.ifc_netmask)) {
				os_printerr("ifconfig: unable to guess netmask\n");
				goto done;
			}
			ifc.ifc_flags |= IFF_NETMASK;
		}

		if (!(ifc.ifc_flags & IFF_BROADADDR)) {
			os_memcpy(&ifc.ifc_broadaddr, &ifc.ifc_addr, sizeof(ifc.ifc_broadaddr));
			SOCK_S_ADDR(&ifc.ifc_broadaddr) |= ~SOCK_S_ADDR(&ifc.ifc_netmask);
			ifc.ifc_flags |= IFF_BROADADDR;
		}
	}

	/* Configure */

	if (rsock_ioctl(s, SIOCSIFCONFIG, &ifc) < 0) {
		os_printerr("rsock_ioctl(SIOCSIFCONFIG): %s\n", os_strerror(os_errno));
		goto done;
	}

	rv = 0;

done:
	if (s >= 0)
		rsock_close(s);

	return rv;
}
