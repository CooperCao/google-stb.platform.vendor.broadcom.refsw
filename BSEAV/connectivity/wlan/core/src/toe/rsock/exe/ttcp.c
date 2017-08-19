/*
 * FILE-CSTYLED
 *
 *	T T C P . C
 *
 * Test TCP connection.  Makes a connection on port 5010
 * and transfers fabricated buffers or data copied from stdin.
 *
 * Usable on 4.2, 4.3, and 4.1a systems by defining one of
 * BSD42 BSD43 (BSD41a)
 * Machines using System V with BSD sockets should define SYSV.
 *
 * Modified for operation under 4.2BSD, 18 Dec 84
 *      T.C. Slattery, USNA
 * Minor improvements, Mike Muuss and Terry Slattery, 16-Oct-85.
 * Modified in 1989 at Silicon Graphics, Inc.
 *	catch SIGPIPE to be able to print stats when receiver has died 
 *	for tcp, don't look for sentinel during reads to allow small transfers
 *	increased default buffer size to 8K, nbuf to 2K to transfer 16MB
 *	moved default port to 5010, beyond IPPORT_USERRESERVED
 *	make sinkmode default because it is more popular, 
 *		-s now means don't sink/source 
 *	count number of read/write system calls to see effects of 
 *		blocking from full socket buffers
 *	for tcp, -D option turns off buffered writes (sets TCP_NODELAY sockopt)
 *	buffer alignment options, -A and -O
 *	print stats in a format that's a bit easier to use with grep & awk
 *	for SYSV, mimic BSD routines to use most of the existing timing code
 * Modified by Steve Miller of the University of Maryland, College Park
 *	-b sets the socket buffer size (SO_SNDBUF/SO_RCVBUF)
 * Modified Sept. 1989 at Silicon Graphics, Inc.
 *	restored -s sense at request of tcs@brl
 * Modified Oct. 1991 at Silicon Graphics, Inc.
 *	use getopt(3) for option processing, add -f and -T options.
 *	SGI IRIX 3.3 and 4.0 releases don't need #define SYSV.
 * Hacked to compile in different O/S environments including the Linux kernel
 *	Jan. 2006 at Broadcom Corp.
 *
 * Distribution Status -
 *      Public Domain.  Distribution Unlimited.
 */
#ifndef lint
char RCSid[] = "ttcp.c $Revision: 1.3 $";
#endif

/* NOTE: this file can't include much, as it's built as part of a Linux kernel driver */
#include "os.h"
#include <rsock/types.h>
#include <rsock/socket.h>
#include "util.h"
#include "ttcp.h"

struct ttcp_info {
	char name[16];

	struct sockaddr_in sinme;
	struct sockaddr_in sinhim;
	struct sockaddr_in frominet;

	int fromlen;
	int fd;				/* fd of network socket */
	int s;				/* fd of network connection */

	int buflen;			/* length of buffer */
	char *buf_alloc;		/* ptr to dynamic buffer */
	char *buf;			/* aligned ptr into buf_alloc */
	int nbuf;			/* number of buffers to send in sinkmode */

	int bufoffset;			/* align buffer to this */
	int bufalign;			/* modulo this */

	int verbose;			/* verbose information display */
	int udp;			/* 0 = tcp, !0 = udp */
	int so_debug;			/* set SO_DEBUG socket option */
	short port;			/* TCP port number */
	int port_incr;			/* port offset for multi-ttcp */
	int trans;			/* 0=receive, !0=transmit mode */
	int sinkmode;			/* 0=normal I/O, 1=sink/source mode */
	int nodelay;			/* set TCP_NODELAY socket option */
	int nb;				/* configure sockets for non-blocking I/O */
	int b_flag;			/* use mread() */
	int sockbufsize;		/* socket buffer size to use */
	int tosb;			/* IP TOS byte */
	int touchdata;			/* access data after reading */
	int checkdata;			/* send random data and verify on receive */

	unsigned long nbytes;		/* bytes on net */
	unsigned long numCalls;		/* # of I/O system calls */
	os_msec_t time0;		/* test start time */
	unsigned int rand_seed;
};

static void pattern(char *cp, int cnt);
static int rand_init(struct ttcp_info *t, int s);
static char rand_char(struct ttcp_info *t);
static void rand_fill(struct ttcp_info *t, char *cp, int cnt);
static void rand_dump(struct ttcp_info *t, char *cp, char *cp2, int cnt);
static int rand_verify(struct ttcp_info *t, int bufno, char *cp, int cnt);
static int Nread(struct ttcp_info *t, int fd, void *buf, int count);
static int Nwrite(struct ttcp_info *t, int fd, void *buf, int count);
static void delay(int us);
static int mread(struct ttcp_info *t, int fd, char *bufp, unsigned int n);
static int mwrite(struct ttcp_info *t, int fd, char *bufp, unsigned int n);

#define STRINGIFY(x)		#x
#define TOSTRING(x)		STRINGIFY(x)

#define SINKMODE_DEFAULT	1
#define NBUF_DEFAULT		128
#define BUFLEN_DEFAULT		8192
#define ALIGN_DEFAULT		16384
#define PORT_DEFAULT		5010

static char *Usage[] = {
"Usage: ttcp -t [-options] <host>\n",
"       ttcp -r [-options]\n",
"Common options:\n",
"       -l #    length of bufs read from or written to network "
					"(default " TOSTRING(BUFLEN_DEFAULT) ")\n",
"       -u      use UDP instead of TCP\n",
"       -p #    base port number to send to or listen at (default " TOSTRING(PORT_DEFAULT) ")\n",
"       -s      Toggle sink mode (default " TOSTRING(SINKMODE_DEFAULT) ").  In sink mode:\n"
"               For -t, source a pattern instead of reading from stdin\n",
"               For -r, discard all data instead of writing to a stdout\n",
"       -A      align the start of buffers to this modulus "
					"(default " TOSTRING(ALIGN_DEFAULT) ")\n",
"       -O      start buffers at this offset from the modulus (default 0)\n",
"       -O      start buffers at this offset from the modulus (default 0)\n",
"       -v      verbose: print more statistics\n",
"       -d      set SO_DEBUG socket option\n",
"       -b #    set socket buffer size (if supported)\n",
"       -n #    number of source bufs written to network (default " TOSTRING(NBUF_DEFAULT) ")\n",
"       -D      don't buffer TCP writes (sets TCP_NODELAY socket option)\n",
"Options specific to -r:\n",
"       -T      \"touch\": access each byte as it's read\n",
"       -N      set sockets in non-blocking mode and retry each EAGAIN\n",
"       -c      check data integrity by sending pseudo-random data or\n",
"               verifying receive data (TCP only)\n",
"       -Q #    Specify IP TOS byte (privileged)\n"
};

int
ttcp(int argc, char *argv[], int port_incr)
{
	struct ttcp_info *t = NULL;
	int c;
	os_msec_t msec;
	int optind = 1;
	char *optarg;
	int one = 1;
	int rv = -1;

	/* Reset global vars to allow multiple runs */

	if ((t = os_malloc(sizeof(*t))) == NULL) {
		os_printf("ttcp%d: not enough memory\n", port_incr + 1);
		return rv;
	}

	os_memset(t, 0, sizeof(*t));

	os_sprintf(t->name, "ttcp");
	t->fd = -1;
	t->s = -1;
	t->buflen = BUFLEN_DEFAULT;
	t->nbuf = NBUF_DEFAULT;
	t->bufalign = ALIGN_DEFAULT;
	t->port = PORT_DEFAULT + port_incr;
	t->port_incr = port_incr;
	t->sinkmode = SINKMODE_DEFAULT;

	while ((c = ugetopt(&optind, &optarg,
	                    argc, argv, "drstuvNBDTcb:f:l:n:p:A:O:Q:")) != -1) {
		switch (c) {
		case 'N':
			t->nb = 1;
			break;
		case 'B':
			t->b_flag = 1;
			break;
		case 't':
			t->trans = 1;
			break;
		case 'r':
			t->trans = 0;
			break;
		case 'd':
			t->so_debug = 1;
			break;
		case 'D':
#ifdef TCP_NODELAY
			t->nodelay = 1;
#else
			os_printerr("ttcp%d: -D option ignored: "
			            "TCP_T->NODELAY socket option not supported\n",
			            port_incr + 1);
#endif
			break;
		case 'n':
			t->nbuf = (int)ustrtoul(optarg, 0, 0);
			break;
		case 'l':
			t->buflen = (int)ustrtoul(optarg, 0, 0);
			break;
		case 's':
			t->sinkmode = !t->sinkmode;
			break;
		case 'p':
			t->port = (int)ustrtoul(optarg, 0, 0) + port_incr;
			break;
		case 'u':
			t->udp = 1;
			break;
		case 'v':
			t->verbose = 1;
			break;
		case 'A':
			t->bufalign = (int)ustrtoul(optarg, 0, 0);
			break;
		case 'O':
			t->bufoffset = (int)ustrtoul(optarg, 0, 0);
			break;
		case 'b':
#if defined(SO_SNDBUF) || defined(SO_RCVBUF)
			t->sockbufsize = (int)ustrtoul(optarg, 0, 0);
#else
			os_printerr("ttcp%d: -b option ignored: "
			            "SO_SNDBUF/SO_RCVBUF socket options not supported\n",
			            port_incr + 1);
#endif
			break;
		case 'T':
			t->touchdata = 1;
			break;
		case 'c':
			t->checkdata = 1;
			break;
		case 'Q':
			t->tosb = (int)ustrtoul(optarg, 0, 0);
			t->tosb &= 0xff;
			break;
		default:
		usage:
			for (c = 0; Usage[c]; c++)
				os_printerr("%s", Usage[c]);
			return -1;
		}
	}

	os_sprintf(t->name + strlen(t->name),
	           "%d%s", port_incr + 1, t->trans ? "-t" : "-r");

	if (t->udp && t->checkdata) {
		os_printf("%s: ignoring checkdata (-c) for udp\n", t->name);
		t->checkdata = 0;
	}

	if (t->trans) {
		/* xmitr */
		if (optind + 1 != argc)
			goto usage;
		if (!inet_addr_parse(argv[optind], (struct sockaddr *)&t->sinhim)) {
			os_printerr("%s: could not parse dest addr '%s'\n",
			            t->name, argv[optind]);
			return -1;
		}
		t->sinhim.sin_family = AF_INET;
		t->sinhim.sin_addr.s_addr = SOCK_S_ADDR(&t->sinhim);
		t->sinhim.sin_port = htons(t->port);
		t->sinme.sin_port = 0;		/* free choice */
	} else {
		/* rcvr */
		if (optind != argc)
			goto usage;
		t->sinme.sin_port =  htons(t->port);
	}

	if (t->udp && t->buflen < 5) {
		t->buflen = 5;		/* send more than the sentinel size */
	}

	if ((t->buf_alloc = (char *)os_malloc(t->buflen + t->bufalign)) == 0) {
		os_printf("%s: not enough memory for buffer\n", t->name);
		goto done;
	}

	if (t->bufalign != 0)
		t->buf = t->buf_alloc +
		        (t->bufalign - ((long)t->buf % t->bufalign) + t->bufoffset) % t->bufalign;
	else
		t->buf = t->buf_alloc;

	if (t->verbose)
		os_printf("%s: buffer address %p\n", t->name, (void *)t->buf);

	if (t->trans) {
		os_printf("%s: buflen=%d, nbuf=%d, align=%d/%d, port=%d  %s\n",
		          t->name,
		          t->buflen, t->nbuf, t->bufalign, t->bufoffset, t->port,
		          t->udp ? "udp" : "tcp");
	} else {
		os_printf("%s: buflen=%d, t->nbuf=%d, align=%d/%d, port=%d  %s\n",
		          t->name,
		          t->buflen, t->nbuf, t->bufalign, t->bufoffset, t->port,
		          t->udp ? "udp" : "tcp");
	}

	if ((t->fd = rsock_socket(PF_INET, t->udp ? SOCK_DGRAM : SOCK_STREAM, 0)) < 0) {
		os_printf("%s: socket(): errno=%d\n", t->name, os_errno);
		goto done;
	}

	if (t->tosb != 0 &&
	    rsock_setsockopt(t->fd, SOL_IP, IP_TOS, &t->tosb, sizeof(t->tosb)) < 0) {
		os_printf("%s: setsockopt(IP_TOS): errno=%d\n", t->name, os_errno);
		goto done;
	}

	if (t->nb && rsock_ioctl(t->fd, FIONBIO, &one) < 0)
		os_printf("%s: ioctl(%d, FIONBIO): errno=%d\n", t->name, t->fd, os_errno);

	if (rsock_bind(t->fd, (struct sockaddr *)&t->sinme, sizeof(t->sinme)) < 0) {
		os_printf("%s: bind(): errno=%d\n", t->name, os_errno);
		goto done;
	}

#if defined(SO_SNDBUF) || defined(SO_RCVBUF)
	if (t->sockbufsize) {
		if (t->trans) {
			if (rsock_setsockopt(t->fd, SOL_SOCKET, SO_SNDBUF, &t->sockbufsize,
			                     sizeof(t->sockbufsize)) < 0)
				os_printf("%s: setsockopt(SO_SNDBUF): errno=%d\n",
				          t->name, os_errno);
		} else {
			if (rsock_setsockopt(t->fd, SOL_SOCKET, SO_RCVBUF, &t->sockbufsize,
			               sizeof(t->sockbufsize)) < 0)
				os_printf("%s: setsockopt(SO_RCVBUF): errno=%d\n",
				          t->name, os_errno);
		}
	}
#endif

	if (!t->udp)  {
		if (t->trans) {
			/* We are the client if transmitting */
			if (t->so_debug)  {
				if (rsock_setsockopt(t->fd, SOL_SOCKET, SO_DEBUG,
				                     &one, sizeof(one)) < 0)
					os_printf("%s: setsockopt(SO_DEBUG): errno=%d\n",
					          t->name, os_errno);
			}
#ifdef TCP_NODELAY
			if (t->nodelay) {
				if (p && rsock_setsockopt(t->fd, IPPROTO_TCP, TCP_NODELAY, 
				                    &one, sizeof(one)) < 0)
					os_printf("%s: setsockopt(TCP_NODELAY): errno=%d\n",
					          t->name, os_errno);
			}
#endif
			os_printf("%s: connecting...\n", t->name);

			if (rsock_connect(t->fd, (struct sockaddr *)&t->sinhim,
			                  sizeof(t->sinhim)) < 0) {
				os_printf("%s: connect(): errno=%d\n", t->name, os_errno);
				goto done;
			}

			t->s = t->fd;
		} else {
			/* otherwise, we are the server and 
			 * should listen for the connections
			 */
			if (rsock_listen(t->fd, 5) < 0) {
				os_printf("%s: listen(): errno=%d\n", t->name, os_errno);
				goto done;
			}

			if (t->so_debug)  {
				if (rsock_setsockopt(t->fd, SOL_SOCKET, SO_DEBUG,
				                     &one, sizeof(one)) < 0)
					os_printf("%s: setsockopt(SO_DEBUG): errno=%d\n",
					          t->name, os_errno);
			}

			t->fromlen = (int)sizeof(t->frominet);

			os_printf("%s: waiting for connection...\n", t->name);

		eagain:
			if ((t->s = rsock_accept(t->fd, (struct sockaddr *)&t->frominet,
			                      &t->fromlen)) < 0) {
				if (os_errno != EAGAIN || t->verbose)
					os_printf("%s: accept(): errno=%d\n", t->name, os_errno);
				if (os_errno == EAGAIN)
					goto eagain;
				goto done;
			}

			if (t->nb && rsock_ioctl(t->s, FIONBIO, &one) < 0)
				os_printf("%s: ioctl(%d, FIONBIO): errno=%d\n",
				          t->name, t->s, os_errno);
		}
	} else
		t->s = t->fd;

	if (rand_init(t, t->s) < 0)
		goto done;


	os_printf("%s: running\n", t->name);

	t->nbytes = 0;

	t->time0 = os_msec();

	if (t->sinkmode) {
		int cnt, b;

		if (t->trans)  {
			pattern(t->buf, t->buflen);

			if (t->udp)
				(void)Nwrite(t, t->s, t->buf, 4);	/* rcvr start */

			for (b = 0; b < t->nbuf; b++) {
				if (t->checkdata)
					rand_fill(t, t->buf, t->buflen);
				if (t->verbose)
					os_printf("%s: WRITE #%d pos=%u len=%d\n",
					          t->name, b, (unsigned int)t->nbytes, t->buflen);
				cnt = Nwrite(t, t->s, t->buf, t->buflen);
				if (t->verbose)
					os_printf("%s:  result=%d\n", t->name, cnt);
				if (cnt < 0)
					break;
				if (cnt != t->buflen) {
					os_printf("%s: SOURCE WRITE LENGTH WRONG\n", t->name);
					break;
				}
				t->nbytes += t->buflen;
			}

			if (b == t->nbuf)
				rv = 0;

			if (t->udp)
				(void)Nwrite(t, t->s, t->buf, 4); /* rcvr end */
		} else {
			b = 0;

			if (t->udp) {
				while ((cnt = Nread(t, t->s, t->buf, t->buflen)) > 0)  {
					static int going = 0;
					if (cnt <= 4)  {
						if (going) {
							rv = 0;
							break;	/* "EOF" */
						}
						going = 1;
						t->time0 = os_msec();
					} else {
						t->nbytes += cnt;
					}
				}
			} else {
				for (;;) {
					if (t->verbose)
						os_printf("%s: READ #%d pos=%u len=%d\n",
						          t->name,
						          b, (unsigned int)t->nbytes, t->buflen);
					cnt = Nread(t, t->s, t->buf, t->buflen);
					if (t->verbose)
						os_printf("%s:  result=%d\n", t->name, cnt);
					if (cnt == 0) {
						rv = 0;
						break;
					}
					if (cnt < 0)
						break;
#ifdef TEST_VERIFY
					if (b == 2)
						t->buf[13] += 0x23;
#endif
					if (t->checkdata && rand_verify(t, b, t->buf, cnt) != 0) {
						break;
					}

					t->nbytes += cnt;
					b++;
				}
			}
		}
	} else {
#ifdef __KERNEL__
		os_printf("%s: only sink mode (-s) can be used in kernel\n", t->name);
		goto done;
#else
		register int cnt;
		if (t->trans)  {
			while ((cnt = read(0, t->buf, t->buflen)) > 0 &&
			       Nwrite(t, t->s, t->buf, cnt) == cnt)
				t->nbytes += cnt;
		}  else  {
			while ((cnt = Nread(t, t->s, t->buf, t->buflen)) > 0 &&
			       write(1, t->buf, cnt) == cnt)
				t->nbytes += cnt;
		}
#endif /* __KERNEL__ */
	}

	if ((msec = os_msec() - t->time0) == 0)
		msec++;

	if (t->udp && t->trans)  {
		(void)Nwrite(t, t->s, t->buf, 4);	/* rcvr end */
		(void)Nwrite(t, t->s, t->buf, 4);	/* rcvr end */
		(void)Nwrite(t, t->s, t->buf, 4);	/* rcvr end */
		(void)Nwrite(t, t->s, t->buf, 4);	/* rcvr end */
	}

	os_printf("%s: %lu bytes in %d.%03d seconds = %luKB/sec\n",
	          t->name, t->nbytes,
	          msec / 1000, msec % 1000,
	          t->nbytes / msec);

	os_printf("%s: %lu I/O calls, msec/call = %lu, calls/sec = %lu\n",
	          t->name, t->numCalls,
	          (t->numCalls == 0) ? 0 : (msec / t->numCalls),
	          t->numCalls * 1000 / msec);

 done:
	if (t->buf_alloc)
		os_free(t->buf_alloc);

	if (t->s >= 0 && t->s != t->fd && rsock_close(t->s) < 0)
		os_printerr("%s: close data socket: errno=%d\n", t->name, os_errno);

	if (t->fd >= 0 && rsock_close(t->fd) < 0)
		os_printerr("%s: close main socket: errno=%d\n", t->name, os_errno);

	if (rv < 0)
		os_printf("%s: Failed\n", t->name);

	os_free(t);

	return rv;
}

static void
pattern(char *cp, int cnt)
{
	char c;

	c = 0;

	while (cnt-- > 0) {
		while (c < 32 || c > 126)	/* was isprint(c) */
			c++;
		*cp++ = (c++ & 0x7F);
	}
}

/*
 * "Minimal Standard RNG" from "RNGs: Good Ones are Hard to Find"
 * by Park and Miller, CACM, Volume 31, Number 10, October 1988.
 */

static int
rand_int(struct ttcp_info *t)		/* 0 to 0x7fffffff */
{
	int a = 16807, m = 2147483647, q = 127773, r = 2836;
	int lo, hi, test;

	hi = t->rand_seed / q;
	lo = t->rand_seed % q;
	test = a * lo - r * hi;
	t->rand_seed = (test > 0) ? test : test + m;

	return t->rand_seed;
}

static char
rand_char(struct ttcp_info *t)
{
	int x = rand_int(t);

	return (char)(x ^ (x >> 5) ^ (x >> 9) ^ (x >> 13));
}

static int
rand_init(struct ttcp_info *t, int s)
{
	struct sockaddr_in self;
	int self_len = (int)sizeof(self);
	struct sockaddr_in peer;
	int peer_len = (int)sizeof(peer);
	unsigned int ia;

	if (rsock_getsockname(s, (struct sockaddr *)&self,
	                      &self_len) < 0) {
		os_printf("%s: getsockname(): errno=%d\n", t->name, os_errno);
		return -1;
	}

	ia = ntohl(self.sin_addr.s_addr);
	os_printf("%s: local name %d.%d.%d.%d port %d\n", 
	          t->name,
	          (ia >> 24) & 0xff, (ia >> 16) & 0xff,
	          (ia >> 8) & 0xff, (ia >> 0) & 0xff,
	          ntohs(self.sin_port));

	if (rsock_getpeername(s, (struct sockaddr *)&peer,
	                      &peer_len) < 0) {
		os_printf("%s: getpeername(): errno=%d\n", t->name, os_errno);
		return -1;
	}

	ia = ntohl(peer.sin_addr.s_addr);
	os_printf("%s: peer name %d.%d.%d.%d port %d\n", 
	          t->name,
	          (ia >> 24) & 0xff, (ia >> 16) & 0xff,
	          (ia >> 8) & 0xff, (ia >> 0) & 0xff,
	          ntohs(peer.sin_port));

	/*
	 * Initialize the random number generator with a seed computed as the XOR of the
	 * local and remote addresses and port numbers.  This ensures the same seed will
	 * be used on both sides.
	 */

	t->rand_seed = ((unsigned int)ntohl(self.sin_addr.s_addr) ^
	                (unsigned int)ntohl(peer.sin_addr.s_addr) ^
	                (unsigned int)ntohs(self.sin_port) ^
	                (unsigned int)ntohs(peer.sin_port));

	if (t->checkdata)
		os_printf("%s: random seed = 0x%x\n",
		          t->name, t->rand_seed);

	return 0;
}

static void
rand_fill(struct ttcp_info *t, char *cp, int cnt)
{
	while (cnt--)
		*cp++ = rand_char(t);
}

void
rand_dump(struct ttcp_info *t, char *cp, char *cp2, int cnt)
{
	int i;
	char buf[80];

	(void)t;

	buf[0] = 0;

	os_sprintf(buf, "%s: ", t->name);

	for (i = 0; i < cnt; i++) {
		os_sprintf(buf + os_strlen(buf),
		           " %02x",
		           ((unsigned int)(cp[i] ^ (cp2 ? cp2[i] : 0)) & 0xff));
		if ((i + 1) % 16 == 0) {
			os_printf("%s\n", buf);
			os_sprintf(buf, "%s: ", t->name);
		}
	}

	if (i % 16 != 0)
		os_printf("%s\n", buf);
}

int
rand_verify(struct ttcp_info *t, int bufno, char *cp, int cnt)
{
	char *randdat;
	int i;
	int nerror = 0, firsterror = -1;

	if ((randdat = os_malloc(cnt)) == NULL) {
		os_printf("%s: rand_verify out of memory, skipping\n", t->name);
		return 0;
	}

	for (i = 0; i < cnt; i++) {
		randdat[i] = rand_char(t);
		if (cp[i] != randdat[i]) {
		    if (nerror++ == 0)
			firsterror = i;
		}
	}

	if (nerror > 0) {
		os_printf("%s: ****** Data error in buffer #%d at offset %d ******\n",
		          t->name, bufno, firsterror);

		os_printf("%s: Expected:\n", t->name);
		rand_dump(t, randdat, NULL, cnt);

		os_printf("%s: Received:\n", t->name);
		rand_dump(t, cp, NULL, cnt);

		os_printf("%s: Difference:\n", t->name);
		rand_dump(t, cp, randdat, cnt);
	}

	os_free(randdat);

	return nerror;
}

/*
 *			N R E A D
 */
static int
Nread(struct ttcp_info *t, int fd, void *buf, int count)
{
	struct sockaddr_in from;
	socklen_t len = sizeof(from);
	int cnt;

 eagain:
	if (t->udp)  {
		cnt = rsock_recvfrom(fd, buf, count, 0, (struct sockaddr *)&from, &len);
		if (cnt < 0 && (os_errno != EAGAIN || t->verbose))
			os_printf("%s: Nread: rsock_recvfrom(%d, %p, %d) failed: errno=%d\n",
			          t->name, fd, buf, count, os_errno);
		t->numCalls++;
	} else {
		if (t->b_flag)
			cnt = mread(t, fd, buf, count);		/* fill buf */
		else {
			cnt = rsock_read(fd, buf, count);
			if (cnt < 0 && (os_errno != EAGAIN || t->verbose))
				os_printf("%s: Nread: rsock_read(%d, %p, %d) failed: errno=%d\n",
				          t->name, fd, buf, count, os_errno);
			t->numCalls++;
		}

		if (t->touchdata && cnt > 0) {
			int c = cnt, sum = 0;
			char *b = buf;
			while (c--)
				sum += *b++;
		}
	}

	if (cnt < 0 && os_errno == EAGAIN)
		goto eagain;	/* For testing of FIONBIO */

	return (cnt);
}

/*
 *			N W R I T E
 */
static int
Nwrite(struct ttcp_info *t, int fd, void *buf, int count)
{
	register int cnt;

 eagain:
	if (t->udp) {
	again:
		cnt = rsock_sendto(fd, buf, count, 0,
		                   (struct sockaddr *)&t->sinhim, (socklen_t)sizeof(t->sinhim));
		t->numCalls++;
		if (cnt < 0 && os_errno == ENOBUFS) {
			delay(18000);
			os_errno = 0;
			goto again;
		}
		if (cnt < 0 && (os_errno != EAGAIN || t->verbose))
			os_printf("%s: Nwrite: rsock_sendto(%d, %p, %d) failed: errno=%d\n",
			          t->name, fd, buf, count, os_errno);
	} else {
		cnt = mwrite(t, fd, buf, count);
		t->numCalls++;

	}

	if (cnt < 0 && os_errno == EAGAIN)
		goto eagain;	/* For testing of FIONBIO */

	return(cnt);
}

static void
delay(int us)
{
	os_msleep((os_msec_t)(us / 1000));
}

/*
 *			M R E A D
 *
 * This function performs the function of a read(II) but will
 * call read(II) multiple times in order to get the requested
 * number of characters.  This can be necessary because
 * network connections don't deliver data with the same
 * grouping as it is written with.  Written by Robert S. Miles, BRL.
 */
static int
mread(struct ttcp_info *t, int fd, char *bufp, unsigned int n)
{
	unsigned count = 0;
	int nread;

	do {
		nread = rsock_read(fd, bufp, n - count);
		t->numCalls++;

		if (nread < 0)  {
			os_printf("%s: mread(%d, %p, %d) failed: errno=%d\n",
			          t->name, fd, (void *)bufp, n - count, os_errno);
			return(-1);
		}

		if (nread == 0)
			return ((int)count);

		count += (unsigned)nread;
		bufp += nread;
	 } while(count < n);

	return ((int)count);
}

/*
 *			M W R I T E
 *
 * This function performs the function of a write(II) but will
 * call write(II) multiple times in order to write the requested
 * number of characters.  This can be necessary if the write
 * system call may return after writing only a portion of the data.
 * (With lwip this can happen with a socket set for FIONBIO.)
 */
static int
mwrite(struct ttcp_info *t, int fd, char *bufp, unsigned int n)
{
	unsigned count = 0;
	int nwrite;

	do {
	eagain:
		nwrite = rsock_write(fd, bufp, n - count);
		t->numCalls++;

		if (nwrite < 0)  {
			if (os_errno != EAGAIN || t->verbose)
				os_printf("%s: mwrite(%d, %p, %d) failed: errno=%d\n",
				          t->name, fd, (void *)bufp, n - count, os_errno);
			if (os_errno == EAGAIN)
				goto eagain;
			return(-1);
		}

		if (nwrite == 0)
			return ((int)count);

		count += (unsigned)nwrite;
		bufp += nwrite;
	 } while(count < n);

	return ((int)count);
}
