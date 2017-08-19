/*
 * Remote Sockets definitions common to client and server side
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

#ifndef __RSOCK_DEFS_H__
#define __RSOCK_DEFS_H__

#ifndef _TYPEDEFS_H_
#include <typedefs.h>
#endif

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>


/* Using in_addr2 to avoid redefinition of lwip's in_addr */
BWL_PRE_PACKED_STRUCT struct in_addr2 {
	uint32 s_addr;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct sockaddr_in {
	uint16 sin_family;
	uint16 sin_port;
	struct in_addr2 sin_addr;
	char sin_zero[8];
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct sockaddr_if {	/* RSock address type for interface control */
	uint16 sif_family;
	char sif_name[14];
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct sockaddr {
	uint16 sa_family;
	char sa_data[14];
} BWL_POST_PACKED_STRUCT;

#ifndef socklen_t
#define socklen_t int
#endif

#define SOCK_STREAM		1
#define SOCK_DGRAM		2
#define SOCK_RAW		3

/*
 * Option flags per-socket.
 */
#define SO_DEBUG		0x0001	/* turn on debugging info recording */
#define SO_ACCEPTCONN		0x0002	/* socket has had listen() */
#define SO_REUSEADDR		0x0004	/* allow local address reuse */
#define SO_KEEPALIVE		0x0008	/* keep connections alive */
#define SO_DONTROUTE		0x0010	/* just use interface addresses */
#define SO_BROADCAST		0x0020	/* permit sending of broadcast msgs */
#define SO_USELOOPBACK		0x0040	/* bypass hardware when possible */
#define SO_LINGER		0x0080	/* linger on close if data present */
#define SO_OOBINLINE		0x0100	/* leave received OOB data in line */
#define SO_REUSEPORT		0x0200	/* allow local address & port reuse */

#define SO_DONTLINGER		((int)(~SO_LINGER))

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF		0x1001	/* send buffer size */
#define SO_RCVBUF		0x1002	/* receive buffer size */
#define SO_SNDLOWAT		0x1003	/* send low-water mark */
#define SO_RCVLOWAT		0x1004	/* receive low-water mark */
#define SO_SNDTIMEO		0x1005	/* send timeout */
#define SO_RCVTIMEO		0x1006	/* receive timeout */
#define SO_ERROR		0x1007	/* get error status and clear */
#define SO_TYPE			0x1008	/* get socket type */

/*
 * Structure used for manipulating linger option.
 */
BWL_PRE_PACKED_STRUCT struct linger {
	int l_onoff;			/* option on/off */
	int l_linger;			/* linger time */
} BWL_POST_PACKED_STRUCT;

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define SOL_SOCKET		0xfff	/* options for socket level */

#define AF_UNSPEC		0
#define AF_INET			2
#define AF_RSOCK		3

#define PF__INVAL		-1
#define PF_UNSPEC		AF_UNSPEC
#define PF_INET			AF_INET
#define PF_RSOCK		AF_RSOCK

#define IPPROTO_IP		0
#define IPPROTO_TCP		6
#define IPPROTO_UDP		17

#define SOL_IP			IPPROTO_IP
#define SOL_TCP			IPPROTO_TCP
#define SOL_UDP			IPPROTO_UDP

#define INADDR_ANY		0
#define INADDR_BROADCAST	0xffffffff

/* Flags we can use with send and recv. */
#define MSG_DONTWAIT		0x40

/*
 * Options for level IPPROTO_IP
 */
#define IP_TOS			1
#define IP_TTL			2

#define IPTOS_TOS_MASK		0x1E
#define IPTOS_TOS(tos)		((tos) & IPTOS_TOS_MASK)
#define IPTOS_LOWDELAY		0x10
#define IPTOS_THROUGHPUT	0x08
#define IPTOS_RELIABILITY	0x04
#define IPTOS_LOWCOST		0x02
#define IPTOS_MINCOST		IPTOS_LOWCOST

/*
 * Definitions for IP precedence (also in ip_tos) (hopefully unused)
 */
#define IPTOS_PREC_MASK			0xe0
#define IPTOS_PREC(tos)			((tos) & IPTOS_PREC_MASK)
#define IPTOS_PREC_NETCONTROL		0xe0
#define IPTOS_PREC_INTERNETCONTROL	0xc0
#define IPTOS_PREC_CRITIC_ECP		0xa0
#define IPTOS_PREC_FLASHOVERRIDE	0x80
#define IPTOS_PREC_FLASH		0x60
#define IPTOS_PREC_IMMEDIATE		0x40
#define IPTOS_PREC_PRIORITY		0x20
#define IPTOS_PREC_ROUTINE		0x00

/*
 * Commands for socket ioctl(), taken from the BSD file fcntl.h.
 *
 *
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 128 bytes.
 */
#if !defined(FIONREAD) || !defined(FIONBIO)
#define IOCPARM_MASK		0x7f		/* params size limit 127b */
#define IOC_VOID		0x20000000	/* no parameters */
#define IOC_OUT			0x40000000	/* copy out parameters */
#define IOC_IN			0x80000000	/* copy in parameters */
#define IOC_INOUT		(IOC_IN|IOC_OUT)
			/* 0x20000000 distinguishes new & old ioctl's */
#define _IO(x, y) \
	(IOC_VOID | ((x) << 8) | (y))

#define _IOR(x, y, t) \
	(IOC_OUT | (((long)sizeof(t) & IOCPARM_MASK) << 16) | ((x) << 8) | (y))

#define _IOW(x, y, t) \
	(IOC_IN | (((long)sizeof(t) & IOCPARM_MASK) << 16) | ((x) << 8) | (y))
#endif

#ifndef FIONREAD
#define FIONREAD    _IOR('f', 127, unsigned long) /* get # bytes to read */
#endif
#ifndef FIONBIO
#define FIONBIO	    _IOW('f', 126, unsigned long) /* set/clear non-blocking */
#endif

/* Socket I/O Controls */
#ifndef SIOCSHIWAT
#define SIOCSHIWAT  _IOW('s',  0, unsigned long)  /* set high watermark */
#define SIOCGHIWAT  _IOR('s',  1, unsigned long)  /* get high watermark */
#define SIOCSLOWAT  _IOW('s',  2, unsigned long)  /* set low watermark */
#define SIOCGLOWAT  _IOR('s',  3, unsigned long)  /* get low watermark */
#define SIOCATMARK  _IOR('s',  7, unsigned long)  /* at oob mark? */
#endif

#ifndef O_NONBLOCK
#define O_NONBLOCK		04000U
#endif

#ifndef O_NDELAY
#define O_NDELAY		O_NONBLOCK
#endif

#ifndef F_SETFL
#define F_GETFL			3
#define F_SETFL			4
#endif

/*
 * Custom ioctls
 *
 * When changing an ifconfig parameter of a remote interface, it's recommended
 * to use SIOCGIFCONFIG to retrieve the info, modify the info as needed, and
 * set it using SIOCSIFCONFIG.  IFCONFIG is a simplified version of IFREQ.
 */

#define SIOCSIFCONFIG	_IOW('s', 20, struct rsock_ifconfig)
#define SIOCGIFCONFIG	_IOR('s', 21, struct rsock_ifconfig)
#define SIOCGIFSTATS	_IOR('s', 22, struct rsock_ifstats)

#define IFHWADDRLEN		6
#define IFNAMSIZ		8

#define IFF_UP			0x1	/* Interface up */
#define IFF_ADDR		0x2	/* ifc_addr valid */
#define IFF_NETMASK		0x4	/* ifc_netmask valid */
#define IFF_BROADADDR		0x8	/* ifc_broadaddr valid */
#define IFF_BROADCAST		0x10	/* Broadcast flag */
#define IFF_HWADDR		0x20	/* ifc_hwaddr{_class} valid */
#define IFF_MTU			0x40	/* ifc_mtu valid */
#define IFF_METRIC		0x80	/* ifc_metric valid */
#define IFF_DEBUG		0x100	/* Turn on debugging */
#define IFF_LOOPBACK		0x200	/* Is a loopback interface */
#define IFF_ARP			0x400	/* Enable ARP on interface */
#define IFF_POINTOPOINTADDR	0x800	/* Point-to-point address valid */
#define IFF_POINTOPOINT		0x1000	/* Point-to-point flag */
#define IFF_MULTICAST		0x2000	/* Multicast flag */
#define IFF_DHCP		0x4000	/* Use Dynamic Host Control Protocol */

BWL_PRE_PACKED_STRUCT struct rsock_ifconfig {			/* ioctl struct size limit 127B */
	uint32 ifc_flags;		/* One or more bits from IFF_xx */
	int ifc_index;			/* Index of I/F (if index >= 0) */
	int ifc_metric;
	int ifc_mtu;
	char ifc_name[IFNAMSIZ];	/* Name of I/F (if index < 0) */
	struct sockaddr ifc_addr;
	struct sockaddr ifc_netmask;
	struct sockaddr ifc_broadaddr;
	struct sockaddr ifc_hwaddr;	/* MAC (get only) */
	struct sockaddr ifc_p2p;	/* Point-to-point address */
} BWL_POST_PACKED_STRUCT;

#define RSOCK_IFSTATS_COUNT \
	((int)sizeof(struct rsock_ifstats) / sizeof(uint32))

BWL_PRE_PACKED_STRUCT struct rsock_ifstats {
	uint32 txframe;
	uint32 txbyte;
	uint32 txerror;
	uint32 txdrop;

	uint32 rxframe;
	uint32 rxbyte;
	uint32 rxerror;
	uint32 rxdrop;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct rsock_ifcontrol {
	int cmd;
	int set;
	int status;
	int datalen;
} BWL_POST_PACKED_STRUCT;

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif /* __RSOCK_DEFS_H__ */
