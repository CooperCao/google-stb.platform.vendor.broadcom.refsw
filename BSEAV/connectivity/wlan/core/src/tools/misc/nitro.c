/*
 * Test Program to verify Nitro Protocol
 *
 * Copyright (C) 2006 Broadcom Corporation
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>

#include <typedefs.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <proto/802.11.h>
#include <proto/nitro.h>

typedef struct wlc_nitro wlc_nitro_t;
typedef struct wlc_info wlc_info_t;
typedef struct wlc_bsscfg wlc_bsscfg_t;
struct wlc_frminfo;
struct scb;
typedef uint32 ratespec_t;

#include <d11.h>
#include <bcmutils.h>
#include <nintendowm.h>
#include <wlc_nitro.h>

#define TRACE(x) if (debug_mode) printf x

#define DEF_TMPTT	10
#define DEF_TXOP	50
#define DEF_RETRY	3
#define DEF_NFRAMES	5

#define is_argument(argv)	(*((argv)+1) && *(*((argv)+1)) != '-')
#define is_option(argv)		(*(argv) && *(*(argv)) == '-')

char buf[WLC_IOCTL_MAXLEN];
uint8 packet[2048];
uint16 mpfrms, mpackfrms, mpretry, mperr;

uint16 nitro_mode = 1;
uint16 pollbitmap = 0;
uint16 tmptt = DEF_TMPTT;
uint16 txop = DEF_TXOP;
uint16 retry = DEF_RETRY;
uint nframes = DEF_NFRAMES;
uint debug_mode = 1;

static int nitro_mp_send(struct ifreq *ifr, uint16 counter, uint16 pbitmap);
static int nitro_key_send(struct ifreq *ifr, uint16 counter);
static void dispatch_packet(struct ifreq *ifr, int sockfd, uint wait_time);

void
alarm_handler(int signal)
{
	printf("MP sequences: success %d, retries %d", mpfrms,
	       ((mpackfrms > mpfrms) ? (mpackfrms - mpfrms) : 0));
	printf(", Missing MPACK frames %d\n",
	       ((mpackfrms < mpfrms) ? (mpfrms - mpackfrms) : 0));

	fflush(stdout);
	_exit(0);
}

void
my_usleep(uint usec)
{
	long start_us, diff;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	start_us = tv.tv_usec;
	diff = 0;

	while (usec > diff) {
		gettimeofday(&tv, NULL);
		if (tv.tv_usec > start_us)
			diff = (tv.tv_usec - start_us);
		else {
			diff = 1000000 - start_us + tv.tv_usec;
		}
	}
}


static void
print_usage(void)
{
	printf("Usage:\n");
	printf(" -h           : this message\n");
	printf(" -i interface : interface name, default is \"eth1\"\n");
	printf(" -m mode      : nitro mode(parent/child, default is \"parent\"\n");
	printf(" -p bitmap    : pollbitmap (parent) , default is \"0\"\n");
	printf(" -t tmptt     : tmptt (ms) (parent) , default is \"10ms\"\n");
	printf(" -x txop      : txop (bytes), default is \"50 bytes\"\n");
	printf(" -r retry     : retry limit, default is \"3\"\n");
	printf(" -n num       : Number of MP sequences, default is \"5\"\n");
	printf(" -d           : debug mode \n");
}

static int
wl_ioctl(struct ifreq *ifr, int cmd, void *buf, int len, bool set)
{
	wl_ioctl_t ioc;
	int ret = 0;
	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		perror("socket");

	/* do it */
	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;
	ioc.set = set;
	ifr->ifr_data = (caddr_t) &ioc;
	ret = ioctl(s, SIOCDEVPRIVATE, ifr);

	/* cleanup */
	close(s);
	return ret;
}

static int
wl_var_setbuf(struct ifreq *ifr, char *iovar, void *param, int param_len)
{
	int len;

	memset(buf, 0, WLC_IOCTL_MAXLEN);
	strcpy(buf, iovar);

	/* include the null */
	len = strlen(iovar) + 1;

	if (param_len)
		memcpy(&buf[len], param, param_len);

	len += param_len;

	return wl_ioctl(ifr, WLC_SET_VAR, &buf, len, TRUE);
}


static int
open_socket(struct ifreq *ifr)
{
	int fd, err;
	struct sockaddr_ll sll;

	fd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE_BRCM));
	if (fd < 0) {
		printf("Cannot create socket %d\n", fd);
		return -1;
	}

	err = ioctl(fd, SIOCGIFINDEX, ifr);
	if (err < 0) {
		printf("Cannot get index %d\n", err);
		return -1;
	}

	memset(&sll, 0, sizeof(sll));
	sll.sll_family = AF_PACKET;
	sll.sll_protocol = htons(ETHER_TYPE_BRCM);
	sll.sll_ifindex = ifr->ifr_ifindex;
	err = bind(fd, (struct sockaddr *)&sll, sizeof(sll));
	if (err < 0) {
		printf("Cannot get index %d\n", err);
		return -1;
	}

	return fd;
}

int
wait_for_packet(int fd, int timeout)
{
	return (recv(fd, packet, sizeof(packet), 0));
}


int
main(int argc, char **argv)
{
	int sockfd = 0;
	struct ifreq ifr;
	const char *ifnames[1] = {"eth1"};
	char *mode = NULL;
	uint wait_time;
	int err = 0;

	/* parse command line parameters */
	for (++argv; is_option(argv); argv++) {
		switch (*((*argv) + 1)) {
		case 'i':
			if (is_argument(argv))
				ifnames[0] = *++argv;
			else
				err = 1;
			break;

		case 'm':
			if (is_argument(argv))
				mode = *++argv;
			else
				err = 1;
			break;

		case 'd':
			debug_mode = 1;
			break;

		case 'p':
			if (is_argument(argv))
				pollbitmap = atoi(*++argv);
			else
				err = 1;
			break;

		case 't':
			if (is_argument(argv))
				tmptt = atoi(*++argv);
			else
				err = 1;
			break;

		case 'r':
			if (is_argument(argv))
				retry = atoi(*++argv);
			else
				err = 1;
			break;

		case 'x':
			if (is_argument(argv))
				txop = atoi(*++argv);
			else
				err = 1;
			break;

		case 'n':
			if (is_argument(argv))
				nframes = atoi(*++argv);
			else
				err = 1;
			break;

		case 'h':
		default:
			print_usage();
			return 0;
		}
	}

	if (err) {
		print_usage();
		return 0;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifnames[0], IFNAMSIZ);

	sockfd = open_socket(&ifr);
	if (sockfd < 0) {
		printf("Socket open connection failed\n");
		return 0;
	}

	if ((mode == NULL) || !strcmp(mode, "parent")) {
		nitro_mode = 1;
		if ((err = nitro_mp_send(&ifr, 1, pollbitmap)) < 0) {
			printf("mp_send error %d\n", err);
			return 0;
		}
		wait_time = tmptt * 2;
	} else {
		nitro_mode = 2;
		if ((err = nitro_key_send(&ifr, 1)) < 0) {
			printf("nitro_key_send error %d\n", err);
			return 0;
		}
		wait_time = 2000;	/* 2 seconds */
	}

	dispatch_packet(&ifr, sockfd, wait_time);

	if (nitro_mode == 2) {
		/* Child mode */
		printf("MP sequences: success %d, retries %d", mpfrms,
			((mpackfrms > mpfrms) ? (mpackfrms - mpfrms) : 0));
		printf(", Missing MPACK frames %d\n",
			((mpackfrms < mpfrms) ? (mpfrms - mpackfrms) : 0));
	} else
		printf("MP sequences: success %d, failures %d, retries %d\n",
		       mpfrms, mperr, mpretry);
	return 0;
}


static int
nitro_mp_send(struct ifreq *ifr, uint16 wmheader, uint16 pbitmap)
{
	wl_mpreq_t *mp;

	if ((mp = malloc(sizeof(wl_mpreq_t) + 128)) == NULL) {
		printf("malloc failure\n");
		return -1;
	}

	mp->resume = 0;
	mp->pollbitmap = pbitmap;
	mp->txop = txop;
	mp->tmptt = tmptt * 100;	/* unit of 10us */
	mp->retryLimit = retry;
	mp->currtsf = 0;
	mp->length = 100;
	mp->wmheader = wmheader;

	return (wl_var_setbuf(ifr, "mpreq", mp, (sizeof(wl_mpreq_t) + mp->length)));
}

static int
nitro_key_send(struct ifreq *ifr, uint16 wmheader)
{
	wl_keydatareq_t *keydata;

	if ((keydata = malloc(sizeof(wl_keydatareq_t) + 30)) == NULL) {
		printf("malloc failure\n");
		return -1;
	}

	keydata->length = txop-2;
	keydata->wmheader = wmheader;

	return (wl_var_setbuf(ifr, "keydatareq", keydata,
	                      (sizeof(wl_keydatareq_t) + keydata->length)));
}
/* pretty hex print a contiguous buffer */
void
prhex(const char *msg, uchar *buf, uint nbytes)
{
	char line[128], *p;
	int len = sizeof(line);
	int nchar;
	uint i;

	if (msg && (msg[0] != '\0'))
		printf("%s:\n", msg);

	p = line;
	for (i = 0; i < nbytes; i++) {
		if (i % 16 == 0) {
			nchar = snprintf(p, len, "  %04d: ", i);	/* line prefix */
			p += nchar;
			len -= nchar;
		}
		if (len > 0) {
			nchar = snprintf(p, len, "%02x ", buf[i]);
			p += nchar;
			len -= nchar;
		}

		if (i % 16 == 15) {
			printf("%s\n", line);		/* flush line */
			p = line;
			len = sizeof(line);
		}
	}

	/* flush last partial line */
	if (p != line)
		printf("%s\n", line);
}

static void
dispatch_packet(struct ifreq *ifr, int sockfd, uint wait_time)
{
	int i = 0, err;
	int length;

	/* start a timer to break indefinite wait for response in child mode */
	printf("%s: enter\n", __FUNCTION__);

	if (nitro_mode == 2) {
		alarm((nframes));
		signal(SIGALRM, alarm_handler);
	}

	while (1) {
		length = wait_for_packet(sockfd, wait_time);
		if (length <= 0) {
			printf("packet receive failure %d\n", length);
			continue;
		}
	printf("%s: got a packet mode %d\n", __FUNCTION__, nitro_mode);

		/* Parent mode */
		if (nitro_mode == 1) {
			nwm_mpkey_t *mpkey;

			mpkey = (nwm_mpkey_t *)((uint)packet + sizeof(struct ether_header));
			TRACE(("bitmap 0x%x, count %d, length %d, txCount %d\n",
			       mpkey->bitmap, mpkey->count, mpkey->resp_maxlen,
			       mpkey->txCount));
			if (mpkey->bitmap)
				mperr++;
			else
				mpfrms++;
			if (mpkey->txCount > 1) mpretry++;

			/* give child time to enqueue its next keydata frame */
			usleep(1000 * 10);
			if (++i < nframes) {
				if ((err = nitro_mp_send(ifr, i+1, pollbitmap)) < 0) {
					printf("mp_send error %d\n", err);
					return;
				}
			} else
				return;

			my_usleep(tmptt*1000);
		} else {
			/* child mode */
			struct dot11_header *h;

			h = (struct dot11_header *)((uint)packet +
			           sizeof(struct ether_header) + sizeof(nwm_mpind_hdr_t));
			prhex(NULL, (uint8 *)packet, 64);
			if (!bcmp(&(h->a1), NITRO_MP_MACADDR, ETHER_ADDR_LEN)) {
				i++;
				mpfrms++;
				TRACE(("recvd mp frame\n"));
				if ((i+1) < nframes)
					nitro_key_send(ifr, i+2);
			} else if (!bcmp(&(h->a1), NITRO_MPACK_MACADDR, ETHER_ADDR_LEN)) {
				mpackfrms++;
				TRACE(("recvd mpack frame\n"));
				if (i >= nframes) return;
			} else
				printf("recvd invalid frame\n");
		}
	}

	return;
}
