/*
 * Ported from FreeBSD 2.2.5 to WIN32 by nn@epigram.com 7/22/98 .
 *
 * Changed -i (interval) option to units of milliseconds (instead of seconds)
 * changed -f (flood) option to delay reply or interval before sending
 * the next request.
 *
 * FILE-CSTYLED
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

/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Muuss.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1989, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
/*
static char sccsid[] = "@(#)ping.c	8.1 (Berkeley) 6/5/93";
*/
static const char rcsid[] =
	"$Id$";
#endif /* not lint */

/*
 *			P I N G . C
 *
 * Using the InterNet Control Message Protocol (ICMP) "ECHO" facility,
 * measure round-trip-delays and packet loss across network paths.
 *
 * Author -
 *	Mike Muuss
 *	U. S. Army Ballistic Research Laboratory
 *	December, 1983
 *
 * Status -
 *	Public Domain.  Distribution Unlimited.
 * Bugs -
 *	More statistics could always be gathered.
 *	This program has to run SUID to ROOT to access the ICMP socket.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#ifndef	__sun__
#include <err.h>
#endif

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <math.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#if	defined(__FreeBSD__) || defined(__sun__)
#include <netinet/ip_var.h>
#endif
#include <arpa/inet.h>

#define	DEFDATALEN	(64 - 8)	/* default data length */
#define	FLOOD_BACKOFF	20000		/* usecs to back off if F_FLOOD mode */

#define	MAXHOSTNAMELEN	256
					/* runs out of buffer space */
#define	MAXIPLEN	60
#define	MAXICMPLEN	76
#define OBUFSIZE	65536
#define	MAXPACKET	(OBUFSIZE - 60 - 8)/* max packet size */
#define	MAXWAIT		10		/* max seconds to wait for response */
#define	NROUTES		9		/* number of record route slots */

#define	A(bit)		rcvd_tbl[(bit)>>3]	/* identify byte in array */
#define	B(bit)		(1 << ((bit) & 0x07))	/* identify bit in byte */
#define	SET(bit)	(A(bit) |= B(bit))
#define	CLR(bit)	(A(bit) &= (~B(bit)))
#define	TST(bit)	(A(bit) & B(bit))

#define CTOI(s)		(int)strtol((s), NULL, 0) /* parse C-style integer */

/* various options */
int options;
#define	F_FLOOD		0x0001
#define	F_INTERVAL	0x0002
#define	F_NUMERIC	0x0004
#define	F_PINGFILLED	0x0008
#define	F_QUIET		0x0010
#define	F_RROUTE	0x0020
#define	F_SO_DEBUG	0x0040
#define	F_SO_DONTROUTE	0x0080
#define	F_VERBOSE	0x0100
#define	F_QUIET2	0x0200
#define	F_NOLOOP	0x0400
#define	F_MTTL		0x0800
#define	F_MIF		0x1000
#define	F_AUDIBLE	0x2000
#define	F_SO_PRIORITY	0x4000
#define	F_IP_TOS	0x8000

/* from FreeBSD sysexits.h */
#define	EX_USAGE	64
#define	EX_NOHOST	68
#define	EX_UNAVAILABLE	69
#define	EX_OSERR	71

#include <errno.h>

/*
 * MAX_DUP_CHK is the number of bits in received table, i.e. the maximum
 * number of received sequence numbers we can keep track of.  Change 128
 * to 8192 for complete accuracy...
 */
#define	MAX_DUP_CHK	(8 * 128)
int mx_dup_ck = MAX_DUP_CHK;
char rcvd_tbl[MAX_DUP_CHK / 8];

struct sockaddr whereto;	/* who to ping */
int datalen = DEFDATALEN;
int s;				/* socket file descriptor */
u_char outpack[OBUFSIZE];
char BSPACE = '\b';		/* characters written for flood */
char DOT = '.';
char *hostname;
n_short ident;			/* process id to identify our packets */

/* counters */
long ntpackets;			/* max packets to transmit */
long nrpackets;			/* max packets to receive */
long nreceived;			/* # of packets we got back */
long nrepeats;			/* number of duplicates */
long ntransmitted;		/* sequence # for outbound packets = #sent */
u_int interval = 1000000;	/* interval between packets (in usec) */

/* timing */
int timing;			/* flag to do timing */
double tmin = 999999999.0;	/* minimum round trip time */
double tmax = 0.0;		/* maximum round trip time */
double tsum = 0.0;		/* sum of all times, for doing average */
double tsumsq = 0.0;		/* sum of all times squared, for std. dev. */

static void fill(char *, char *);
static u_short in_cksum(u_short *, int);
static void finish(void);
static void pinger(void);
static char *pr_addr(struct in_addr);
static void pr_icmph(struct icmp *);
static void pr_iph(struct ip *);
static int pr_pack(char *, int, struct sockaddr_in *);
static void pr_retip(struct ip *);
static void usage(const char *);

#if	defined(__sun__)
static void errx(int eval, const char *fmt, ...);
#define	warn	printf
#endif


u_int
get_usec()
{

	static int first = 0;
	struct timeval tv;
	int usec;

	(void) gettimeofday(&tv, NULL);

	/* subtract off static value to fit in an (int) */
	if (first == 0)
		first = tv.tv_sec;

	usec = ((tv.tv_sec - first) * 1000000) + tv.tv_usec;

	return (usec);
}

int
main(argc, argv)
	int argc;
	char **argv;
{
	struct timeval timeout;
	struct hostent *hp;
	struct sockaddr_in *to;
	register int i;
	int ch, hold, packlen, preload;
	fd_set fdmask;
	struct in_addr ifaddr;
	unsigned char ttl, loop;
	u_char *datap, *packet;
	u_int now, delta;
	char *target, hnamebuf[MAXHOSTNAMELEN];
	int tmp;
	int npackets_pending = 0;
#if defined(__linux__)
	int priority;
#endif
	int ip_tos;

#ifdef IP_OPTIONS
	char rspace[3 + 4 * NROUTES + 1];	/* record route space */
#endif
	int rc;

	s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (s < 0) {
		perror("create socket");
		errx(EX_OSERR, "create socket");
	}

	preload = 0;

	ident = (n_short) getuid();


	datap = &outpack[8 + sizeof(struct timeval)];
	while ((ch = getopt(argc, argv, "I:LQRT:c:C:adfi:l:nP:t:p:qrs:v")) != -1) {
		switch(ch) {
		case 'a':
			options |= F_AUDIBLE;
			break;
		case 'c':
			tmp = CTOI(optarg);
			if ((tmp == 0) || (tmp > INT_MAX))
				errx(EX_USAGE,
				    "invalid count of packets to receive: `%s'",
				    optarg);
			nrpackets = tmp;
			break;
		case 'C':
			tmp = CTOI(optarg);
			if ((tmp == 0) || (tmp > INT_MAX))
				errx(EX_USAGE,
				    "invalid count of packets to xmit: `%s'",
				    optarg);
			ntpackets = tmp;
			break;
		case 'd':
			options |= F_SO_DEBUG;
			break;
		case 'f':
			options |= F_FLOOD;
			setbuf(stdout, (char *)NULL);
			break;
		case 'i':		/* wait between sending packets */
			tmp = CTOI(optarg);	/* this is now in milliseconds */
			if (tmp > INT_MAX)
				errx(EX_USAGE,
				     "invalid timing interval: `%s'", optarg);
			options |= F_INTERVAL;
			/* interval in usec */
			interval = tmp * 1000;
			break;
		case 'I':		/* multicast interface */
			ifaddr.s_addr = inet_addr(optarg);
			if (ifaddr.s_addr == 0)
				errx(EX_USAGE, 
				     "invalid multicast interface: `%s'",
				     optarg);
			options |= F_MIF;
			break;
		case 'l':
			tmp = CTOI(optarg);
			if (tmp > INT_MAX)
				errx(EX_USAGE, 
				     "invalid preload value: `%s'", optarg);
			options |= F_FLOOD;
			preload = tmp;
			break;
		case 'L':
			options |= F_NOLOOP;
			loop = 0;
			break;
		case 'n':
			options |= F_NUMERIC;
			break;
		case 'P':
#if defined(__linux__)
			options |= F_SO_PRIORITY;
			priority = CTOI(optarg);
#else
			fprintf(stderr,
			    "ping: -P option not supported on this OS\n");
#endif
			break;
		case 't':
			options |= F_IP_TOS;
			ip_tos = CTOI(optarg);
			break;
		case 'p':		/* fill buffer with user pattern */
			options |= F_PINGFILLED;
			fill((char *)datap, optarg);
				break;
		case 'Q':
			options |= F_QUIET2;
			break;
		case 'q':
			options |= F_QUIET;
			break;
		case 'R':
			options |= F_RROUTE;
			break;
		case 'r':
			options |= F_SO_DONTROUTE;
			break;
		case 's':		/* size of packet to send */
			tmp = CTOI(optarg);
			if (tmp > MAXPACKET)
				errx(EX_USAGE, "packet size too large: %u",
				     tmp);
			if (!tmp)
				errx(EX_USAGE, "invalid packet size: `%s'", 
				     optarg);
			datalen = tmp;
			break;
		case 'T':		/* multicast TTL */
			tmp = CTOI(optarg);
			if (tmp > 255)
				errx(EX_USAGE, "invalid multicast TTL: `%s'",
				     optarg);
			ttl = tmp;
			options |= F_MTTL;
			break;
		case 'v':
			options |= F_VERBOSE;
			break;
		default:

			usage(argv[0]);
		}
	}

	if (argc - optind != 1)
		usage(argv[0]);
	target = argv[optind];

	bzero((char *)&whereto, sizeof(struct sockaddr));
	to = (struct sockaddr_in *)&whereto;
	to->sin_family = AF_INET;

	if ((target[0] >= '0') && (target[0] <= '9'))
		to->sin_addr.s_addr = inet_addr(target);
	else {
		hp = gethostbyname(target);
		if (!hp)
			errx(EX_NOHOST, "cannot resolve %s", target);

		if (hp->h_length > sizeof(to->sin_addr))
			errx(1,"gethostbyname2 returned an illegal address");
		memcpy(&to->sin_addr, hp->h_addr_list[0], sizeof to->sin_addr);
		(void)strncpy(hnamebuf, hp->h_name, sizeof(hnamebuf) - 1);
		hnamebuf[(sizeof hnamebuf) - 1] = '\0';
		hostname = hnamebuf;
	}

	if (options & F_FLOOD && IN_MULTICAST(ntohl(to->sin_addr.s_addr)))
		errx(EX_USAGE, 
		     "-f flag cannot be used with multicast destination");
	if (options & (F_MIF | F_NOLOOP | F_MTTL)
	    && !IN_MULTICAST(ntohl(to->sin_addr.s_addr)))
		errx(EX_USAGE, 
		     "-I, -L, -T flags cannot be used with unicast destination");

	if (datalen >= sizeof(struct timeval))	/* can we time transfer */
		timing = 1;
	packlen = 8192;
	if ((datalen + MAXIPLEN + MAXICMPLEN) > packlen)
		packlen = datalen + MAXIPLEN + MAXICMPLEN;
	if (!(packet = (u_char *)malloc((size_t)packlen)))
		errx(EX_UNAVAILABLE, "malloc");

	if (!(options & F_PINGFILLED))
		for (i = 8; i < datalen; ++i)
			*datap++ = i;
	hold = 1;
	if (options & F_SO_DEBUG)
		(void)setsockopt(s, SOL_SOCKET, SO_DEBUG, (char *)&hold,
		    sizeof(hold));
	if (options & F_SO_DONTROUTE)
		(void)setsockopt(s, SOL_SOCKET, SO_DONTROUTE, (char *)&hold,
		    sizeof(hold));
#if defined(__linux__)
	if (options & F_SO_PRIORITY) {
		(void)setsockopt(s, SOL_SOCKET, SO_PRIORITY, (char *)&priority,
		    sizeof(priority));
	}
#endif
	if (options & F_IP_TOS) {
		(void)setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&ip_tos,
		    sizeof(ip_tos));
	}
	/* record route option */
	if (options & F_RROUTE) {
#ifdef IP_OPTIONS
		rspace[IPOPT_OPTVAL] = IPOPT_RR;
		rspace[IPOPT_OLEN] = sizeof(rspace)-1;
		rspace[IPOPT_OFFSET] = IPOPT_MINOFF;
		if (setsockopt(s, IPPROTO_IP, IP_OPTIONS, rspace,
		    sizeof(rspace)) < 0)
			errx(EX_OSERR, "setsockopt IP_OPTIONS");
#else
		errx(EX_UNAVAILABLE,
		  "record route not available in this implementation");
#endif /* IP_OPTIONS */
	}

#ifdef notdef
	if (options & F_NOLOOP) {
		if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &loop,
		    sizeof(loop)) < 0) {
			errx(EX_OSERR, "setsockopt IP_MULTICAST_LOOP");
		}
	}
	if (options & F_MTTL) {
		if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,
		    sizeof(ttl)) < 0) {
			errx(EX_OSERR, "setsockopt IP_MULTICAST_TTL");
		}
	}
	if (options & F_MIF) {
		if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, (const char*) &ifaddr,
		    sizeof(ifaddr)) < 0) {
			errx(EX_OSERR, "setsockopt IP_MULTICAST_IF");
		}
	}
#endif

	/*
	 * When pinging the broadcast address, you can get a lot of answers.
	 * Doing something so evil is useful if you are trying to stress the
	 * ethernet, or just want to fill the arp cache to get some stuff for
	 * /etc/ethers.  But beware: RFC 1122 allows hosts to ignore broadcast
	 * or multicast pings if they wish.
	 */
	hold = 48 * 1024;
	(void)setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&hold,
	    sizeof(hold));

	if (to->sin_family == AF_INET)
		(void)printf("PING %s (%s): %d data bytes\n", hostname,
		    inet_ntoa(to->sin_addr),
		    datalen);
	else
		(void)printf("PING %s: %d data bytes\n", hostname, datalen);

	npackets_pending = preload;
	while (preload--)		/* fire off them quickies */
		pinger();

	FD_ZERO(&fdmask);
	FD_SET(s, &fdmask);

	while (1) {
		struct sockaddr_in from;
		register int cc;
		unsigned int fromlen;

		/* break if we have sent specifed packet count */
		if (ntpackets && ntransmitted >= ntpackets)
			break;

		now = get_usec();

		pinger();

		npackets_pending++;
		
		while ( (delta = get_usec() - now) < interval ) {
			delta = interval - delta;
			timeout.tv_sec = delta / 1000000;
			timeout.tv_usec = (delta % 1000000);

			FD_ZERO(&fdmask);
			FD_SET(s, &fdmask);

			rc = select(s + 1, &fdmask, NULL, NULL, &timeout);

			if (rc < 0) {
				fprintf(stderr, "select error: %d\n", errno);
				exit(1);
			}

			/* handle timeout */
			if (rc == 0) {
				if (npackets_pending > 0 && !(options & F_FLOOD)) {
					puts("Request timed out.");
				}
				/* That was the end of the delay, so exit delay loop */
				break;
			}

			fromlen = sizeof(from);
			cc = recvfrom(s, (char *)packet, packlen,
			              0, (struct sockaddr *)&from, &fromlen);
			if (cc < 0) {
				fprintf(stderr, "recvfrom error: %d\n", errno);
				exit(1);
			}
			if (pr_pack((char *)packet, cc, &from))
				npackets_pending--;
			
			/* break out of the delay if we are flooding or
			 * we have reached a packet count limit */
			if ((options & F_FLOOD) ||
				(nrpackets && nreceived >= nrpackets) ||
				(ntpackets && ntransmitted >= ntpackets))
				break;
		}

		/* all pending packets are timed out */
		npackets_pending = 0;
		
		if (nrpackets && nreceived >= nrpackets)
			break;
	}
	finish();
	/* NOTREACHED */
	return (0);	/* Make the compiler happy */
}

/*
 * pinger --
 *	Compose and transmit an ICMP ECHO REQUEST packet.  The IP packet
 * will be added on by the kernel.  The ID field is our UNIX process ID,
 * and the sequence number is an ascending integer.  The first 8 bytes
 * of the data portion are used to hold a UNIX "timeval" struct in host
 * byte-order, to compute the round-trip time.
 *
 */
static void
pinger(void)
{
	register struct icmp *icp;
	register int cc;
	int ticks;
	int i;

	icp = (struct icmp *)outpack;
	icp->icmp_type = ICMP_ECHO;
	icp->icmp_code = 0;
	icp->icmp_cksum = 0;
	icp->icmp_seq = (n_short) ntransmitted;
	icp->icmp_id = ident;

	CLR(icp->icmp_seq % mx_dup_ck);

	if (timing) {
		ticks = get_usec();
		*(u_long*) &outpack[8] = ticks;
	}

	cc = datalen + 8;			/* skips ICMP portion */

	/* compute ICMP checksum here */
	icp->icmp_cksum = in_cksum((u_short *)icp, cc);

	i = sendto(s, (char *)outpack, cc, 0, &whereto,
	    sizeof(struct sockaddr));

	if (i < 0 || i != cc)  {
		if (i < 0) {
			if (options & F_FLOOD) {
				usleep(FLOOD_BACKOFF);
				return;
			}
			warn("sendto");
		} else {
			warn("%s: partial write: %d of %d bytes",
			     hostname, cc, i);
		}
	}
	ntransmitted++;
	if (!(options & F_QUIET) && (options & F_FLOOD))
		printf(".");
}

/*
 * pr_pack -- returns 1 if the packet is a ping reply, 0 otherwise
 *	Print out the packet, if it came from us.  This logic is necessary
 * because ALL readers of the ICMP socket get a copy of ALL ICMP packets
 * which arrive ('tis only fair).  This permits multiple copies of this
 * program to be run without having intermingled output (or statistics!).
 */
static int
pr_pack(buf, cc, from)
	char *buf;
	int cc;
	struct sockaddr_in *from;
{
	register struct icmp *icp;
	register u_long l;
	register int i, j;
	register u_char *cp,*dp;
	static int old_rrlen;
	static char old_rr[MAX_IPOPTLEN];
	struct ip *ip;
	u_long now, then;
	double triptime;
	int hlen, dupflag;
	int got_packet = 0;
	
	now = get_usec();

	/* Check the IP header */
	ip = (struct ip *)buf;
	hlen = ip->ip_hl << 2;
	if (cc < hlen + ICMP_MINLEN) {
		if (options & F_VERBOSE)
			warn("packet too short (%d bytes) from %s", cc,
			     inet_ntoa(from->sin_addr));
		return got_packet;
	}

	/* Now the ICMP part */
	cc -= hlen;
	icp = (struct icmp *)(buf + hlen);
	if (icp->icmp_type == ICMP_ECHOREPLY) {
		if (icp->icmp_id != ident)
			return got_packet;		/* 'Twas not our ECHO */
		++nreceived;
		got_packet = 1;
		triptime = 0.0;
		if (timing) {
#ifndef icmp_data
			then = *(u_long *)&icp->icmp_ip;
#else
			then = *(u_long *)icp->icmp_data;
#endif

			/* compute delta and convert to msec */
			triptime = ((double)(now - then))/1000.0;
			tsum += triptime;
			tsumsq += triptime * triptime;
			if (triptime < tmin)
				tmin = triptime;
			if (triptime > tmax)
				tmax = triptime;
		}

		if (TST(icp->icmp_seq % mx_dup_ck)) {
			++nrepeats;
			--nreceived;
			got_packet = 0;
			dupflag = 1;
		} else {
			SET(icp->icmp_seq % mx_dup_ck);
			dupflag = 0;
		}

		if (options & F_QUIET)
			return got_packet;

		if (options & F_FLOOD)
			printf("%c", BSPACE);
		else {
			(void)printf("%d bytes from %s: icmp_seq=%u", cc,
			   inet_ntoa(*(struct in_addr *)&from->sin_addr.s_addr),
			   icp->icmp_seq);
			(void)printf(" ttl=%d", ip->ip_ttl);
			if (timing) {
				(void)printf(" time=%.3f ms", triptime);
			}
			if (dupflag)
				(void)printf(" (DUP!)");
			if (options & F_AUDIBLE)
				(void)printf("\a");
			/* check the data */
			cp = (u_char*)&icp->icmp_data[8];
			dp = &outpack[8 + sizeof(struct timeval)];
			for (i = 8; i < datalen; ++i, ++cp, ++dp) {
				if (*cp != *dp) {
	(void)printf("\nwrong data byte #%d should be 0x%x but was 0x%x",
	    i, *dp, *cp);
					cp = (u_char*)&icp->icmp_data[0];
					for (i = 8; i < datalen; ++i, ++cp) {
						if ((i % 32) == 8)
							(void)printf("\n\t");
						(void)printf("%x ", *cp);
					}
					break;
				}
			}
		}
	} else {
		/*
		 * We've got something other than an ECHOREPLY.
		 * See if it's a reply to something that we sent.
		 * We can compare IP destination, protocol,
		 * and ICMP type and ID.
		 *
		 * Only print all the error messages if we are running
		 * as root to avoid leaking information not normally 
		 * available to those not running as root.
		 */
#ifndef icmp_data
		struct ip *oip = &icp->icmp_ip;
#else
		struct ip *oip = (struct ip *)icp->icmp_data;
#endif
		struct icmp *oicmp = (struct icmp *)(oip + 1);

		if (((options & F_VERBOSE)) ||
		    (!(options & F_QUIET2) &&
		     (oip->ip_dst.s_addr ==
			 ((struct sockaddr_in *)&whereto)->sin_addr.s_addr) &&
		     (oip->ip_p == IPPROTO_ICMP) &&
		     (oicmp->icmp_type == ICMP_ECHO) &&
		     (oicmp->icmp_id == ident))) {
		    (void)printf("%d bytes from %s: ", cc,
			pr_addr(from->sin_addr));
		    pr_icmph(icp);
		} else
		    return got_packet;
	}

	/* Display any IP options */
	cp = (u_char *)buf + sizeof(struct ip);

	for (; hlen > (int)sizeof(struct ip); --hlen, ++cp)
		switch (*cp) {
		case IPOPT_EOL:
			hlen = 0;
			break;
		case IPOPT_LSRR:
			(void)printf("\nLSRR: ");
			hlen -= 2;
			j = *++cp;
			++cp;
			if (j > IPOPT_MINOFF)
				for (;;) {
					l = *++cp;
					l = (l<<8) + *++cp;
					l = (l<<8) + *++cp;
					l = (l<<8) + *++cp;
					if (l == 0) {
						printf("\t0.0.0.0");
					} else {
						struct in_addr ina;
						ina.s_addr = ntohl(l);
						printf("\t%s", pr_addr(ina));
					}
				hlen -= 4;
				j -= 4;
				if (j <= IPOPT_MINOFF)
					break;
				(void)putchar('\n');
			}
			break;
		case IPOPT_RR:
			j = *++cp;		/* get length */
			i = *++cp;		/* and pointer */
			hlen -= 2;
			if (i > j)
				i = j;
			i -= IPOPT_MINOFF;
			if (i <= 0)
				continue;
			if (i == old_rrlen
			    && cp == (u_char *)buf + sizeof(struct ip) + 2
			    && !bcmp((char *)cp, old_rr, i)
			    && !(options & F_FLOOD)) {
				(void)printf("\t(same route)");
				i = ((i + 3) / 4) * 4;
				hlen -= i;
				cp += i;
				break;
			}
			old_rrlen = i;
			bcopy((char *)cp, old_rr, i);
			(void)printf("\nRR: ");
			for (;;) {
				l = *++cp;
				l = (l<<8) + *++cp;
				l = (l<<8) + *++cp;
				l = (l<<8) + *++cp;
				if (l == 0) {
					printf("\t0.0.0.0");
				} else {
					struct in_addr ina;
					ina.s_addr = ntohl(l);
					printf("\t%s", pr_addr(ina));
				}
				hlen -= 4;
				i -= 4;
				if (i <= 0)
					break;
				(void)putchar('\n');
			}
			break;
		case IPOPT_NOP:
			(void)printf("\nNOP");
			break;
		default:
			(void)printf("\nunknown option %x", *cp);
			break;
		}
	if (!(options & F_FLOOD)) {
		(void)putchar('\n');
		(void)fflush(stdout);
	}
	return got_packet;
}

/*
 * in_cksum --
 *	Checksum routine for Internet Protocol family headers (C Version)
 */
u_short
in_cksum(addr, len)
	u_short *addr;
	int len;
{
	register int nleft = len;
	register u_short *w = addr;
	register int sum = 0;
	u_short answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(u_char *)(&answer) = *(u_char *)w ;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return(answer);
}

/*
 * finish --
 *	Print out statistics, and give up.
 */
static void
finish()
{
	(void)putchar('\n');
	(void)fflush(stdout);
	(void)printf("--- %s ping statistics ---\n", hostname);
	(void)printf("%ld packets transmitted, ", ntransmitted);
	(void)printf("%ld packets received, ", nreceived);
	if (nrepeats)
		(void)printf("+%ld duplicates, ", nrepeats);
	if (ntransmitted) {
		if (nreceived > ntransmitted)
			(void)printf("-- somebody's printing up packets!");
		else
			(void)printf("%d%% packet loss",
			    (int) (((ntransmitted - nreceived) * 100) /
			    ntransmitted));
	}
	(void)putchar('\n');
	if (nreceived && timing) {
		double n = nreceived + nrepeats;
		double avg = tsum / n;
		double vari = tsumsq / n - avg * avg;
		printf("round-trip min/avg/max/stddev = "
		       "%.3f/%.3f/%.3f/%.3f ms\n",
		    tmin, avg, tmax, sqrt(vari));
	}

	if (nreceived)
		exit(0);
	else
		exit(2);
}

#ifdef notdef
static char *ttab[] = {
	"Echo Reply",		/* ip + seq + udata */
	"Dest Unreachable",	/* net, host, proto, port, frag, sr + IP */
	"Source Quench",	/* IP */
	"Redirect",		/* redirect type, gateway, + IP  */
	"Echo",
	"Time Exceeded",	/* transit, frag reassem + IP */
	"Parameter Problem",	/* pointer + IP */
	"Timestamp",		/* id + seq + three timestamps */
	"Timestamp Reply",	/* " */
	"Info Request",		/* id + sq */
	"Info Reply"		/* " */
};
#endif

/*
 * pr_icmph --
 *	Print a descriptive string about an ICMP header.
 */
static void
pr_icmph(icp)
	struct icmp *icp;
{
	switch(icp->icmp_type) {
	case ICMP_ECHOREPLY:
		(void)printf("Echo Reply\n");
		break;
	case ICMP_UNREACH:
		switch(icp->icmp_code) {
		case ICMP_UNREACH_NET:
			(void)printf("Destination Net Unreachable\n");
			break;
		case ICMP_UNREACH_HOST:
			(void)printf("Destination Host Unreachable\n");
			break;
		case ICMP_UNREACH_PROTOCOL:
			(void)printf("Destination Protocol Unreachable\n");
			break;
		case ICMP_UNREACH_PORT:
			(void)printf("Destination Port Unreachable\n");
			break;
		case ICMP_UNREACH_NEEDFRAG:
#ifdef	icmp_nextmtu
			(void)printf("frag needed and DF set (MTU %d)\n",
					ntohs(icp->icmp_nextmtu));
#else
			(void)printf("frag needed and DF set\n");
#endif
			break;
		case ICMP_UNREACH_SRCFAIL:
			(void)printf("Source Route Failed\n");
			break;
#ifdef	ICMP_UNREACH_FILTER_PROHIB
		case ICMP_UNREACH_FILTER_PROHIB:
			(void)printf("Communication prohibited by filter\n");
			break;
#endif
		default:
			(void)printf("Dest Unreachable, Bad Code: %d\n",
			    icp->icmp_code);
			break;
		}
		/* Print returned IP header information */
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct ip *)icp->icmp_data);
#endif
		break;
	case ICMP_SOURCEQUENCH:
		(void)printf("Source Quench\n");
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct ip *)icp->icmp_data);
#endif
		break;
	case ICMP_REDIRECT:
		switch(icp->icmp_code) {
		case ICMP_REDIRECT_NET:
			(void)printf("Redirect Network");
			break;
		case ICMP_REDIRECT_HOST:
			(void)printf("Redirect Host");
			break;
		case ICMP_REDIRECT_TOSNET:
			(void)printf("Redirect Type of Service and Network");
			break;
		case ICMP_REDIRECT_TOSHOST:
			(void)printf("Redirect Type of Service and Host");
			break;
		default:
			(void)printf("Redirect, Bad Code: %d", icp->icmp_code);
			break;
		}
		(void)printf("(New addr: %s)\n", inet_ntoa(icp->icmp_gwaddr));
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct ip *)icp->icmp_data);
#endif
		break;
	case ICMP_ECHO:
		(void)printf("Echo Request\n");
		break;
	case ICMP_TIMXCEED:
		switch(icp->icmp_code) {
		case ICMP_TIMXCEED_INTRANS:
			(void)printf("Time to live exceeded\n");
			break;
		case ICMP_TIMXCEED_REASS:
			(void)printf("Frag reassembly time exceeded\n");
			break;
		default:
			(void)printf("Time exceeded, Bad Code: %d\n",
			    icp->icmp_code);
			break;
		}
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct ip *)icp->icmp_data);
#endif
		break;
	case ICMP_PARAMPROB:
		(void)printf("Parameter problem: pointer = 0x%02x\n",
		    icp->icmp_hun.ih_pptr);
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct ip *)icp->icmp_data);
#endif
		break;
	case ICMP_TSTAMP:
		(void)printf("Timestamp\n");
		break;
	case ICMP_TSTAMPREPLY:
		(void)printf("Timestamp Reply\n");
		break;
	case ICMP_IREQ:
		(void)printf("Information Request\n");
		break;
	case ICMP_IREQREPLY:
		(void)printf("Information Reply\n");
		break;
	case ICMP_MASKREQ:
		(void)printf("Address Mask Request\n");
		break;
	case ICMP_MASKREPLY:
		(void)printf("Address Mask Reply\n");
		break;
#ifdef	ICMP_ROUTERADVERT
	case ICMP_ROUTERADVERT:
		(void)printf("Router Advertisement\n");
		break;
#endif
#ifdef	ICMP_ROUTERSOLICIT
	case ICMP_ROUTERSOLICIT:
		(void)printf("Router Solicitation\n");
		break;
#endif
	default:
		(void)printf("Bad ICMP type: %d\n", icp->icmp_type);
	}
}

/*
 * pr_iph --
 *	Print an IP header with options.
 */
static void
pr_iph(ip)
	struct ip *ip;
{
	int hlen;
	u_char *cp;

	hlen = ip->ip_hl << 2;
	cp = (u_char *)ip + 20;		/* point to options */

	(void)printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst\n");
	(void)printf(" %1x  %1x  %02x %04x %04x",
	    ip->ip_v, ip->ip_hl, ip->ip_tos, ntohs(ip->ip_len),
	    ntohs(ip->ip_id));
	(void)printf("   %1lx %04lx", (long)(ntohl(ip->ip_off) & 0xe000) >> 13,
	    (long)ntohl(ip->ip_off) & 0x1fff);
	(void)printf("  %02x  %02x %04x", ip->ip_ttl, ip->ip_p,
							    ntohs(ip->ip_sum));
	(void)printf(" %s ", inet_ntoa(*(struct in_addr *)&ip->ip_src.s_addr));
	(void)printf(" %s ", inet_ntoa(*(struct in_addr *)&ip->ip_dst.s_addr));
	/* dump any option bytes */
	while (hlen-- > 20) {
		(void)printf("%02x", *cp++);
	}
	(void)putchar('\n');
}

/*
 * pr_addr --
 *	Return an ascii host address as a dotted quad and optionally with
 * a hostname.
 */
static char *
pr_addr(ina)
	struct in_addr ina;
{
	struct hostent *hp;
	static char buf[16 + 3 + MAXHOSTNAMELEN];

	if ((options & F_NUMERIC) ||
	    !(hp = gethostbyaddr((char *)&ina, 4, AF_INET)))
		return inet_ntoa(ina);
	else
		(void)sprintf(buf, "%s (%s)", hp->h_name,
		    inet_ntoa(ina));
	return(buf);
}

/*
 * pr_retip --
 *	Dump some info on a returned (via ICMP) IP packet.
 */
static void
pr_retip(ip)
	struct ip *ip;
{
	int hlen;
	u_char *cp;

	pr_iph(ip);
	hlen = ip->ip_hl << 2;
	cp = (u_char *)ip + hlen;

	if (ip->ip_p == 6)
		(void)printf("TCP: from port %u, to port %u (decimal)\n",
		    (*cp * 256 + *(cp + 1)), (*(cp + 2) * 256 + *(cp + 3)));
	else if (ip->ip_p == 17)
		(void)printf("UDP: from port %u, to port %u (decimal)\n",
			(*cp * 256 + *(cp + 1)), (*(cp + 2) * 256 + *(cp + 3)));
}

static void
fill(bp, patp)
	char *bp, *patp;
{
	int ii, jj, kk;
	int pat[16];
	char *cp;

	for (cp = patp; *cp; cp++) {
		if (!isxdigit(*cp))
			errx(EX_USAGE, 
			     "patterns must be specified as hex digits");
			
	}
	ii = sscanf(patp,
	    "%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",
	    &pat[0], &pat[1], &pat[2], &pat[3], &pat[4], &pat[5], &pat[6],
	    &pat[7], &pat[8], &pat[9], &pat[10], &pat[11], &pat[12],
	    &pat[13], &pat[14], &pat[15]);

	if (ii > 0)
		for (kk = 0;
		    kk <= (int) (MAXPACKET - (8 + sizeof(struct timeval) + ii));
		    kk += ii)
			for (jj = 0; jj < ii; ++jj)
				bp[jj + kk] = pat[jj];
	if (!(options & F_QUIET)) {
		(void)printf("PATTERN: 0x");
		for (jj = 0; jj < ii; ++jj)
			(void)printf("%02x", bp[jj] & 0xFF);
		(void)printf("\n");
	}
}

static void
usage(argv0)
	const char *argv0;
{
	if (strrchr(argv0,'/'))
		argv0 = strrchr(argv0,'/') + 1;
	fprintf(stderr,
	    "usage: %s [-QRadfnqrv] [-c rxcount] [-C txcount]\n"
	    "                [-i wait_milliseconds]\n"
	    "                "
#if defined(__linux__)
	    "[-P priority ] "
#endif
	    "[-t IP-TOS]\n"
	    "                [-l preload] [-p pattern] [-s packetsize]\n"
	    "                [host | [-L] [-I iface] [-T ttl] mcast-group]\n",
	    argv0);
	exit(EX_USAGE);
}

#if	defined(__sun__)
static void
errx(int eval, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(1);
}
#endif
