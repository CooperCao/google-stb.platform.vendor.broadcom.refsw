//****************************************************************************
//
// Copyright (c) 2004-2010 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//
//  Filename:       DsgClientCtlThread.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  November 22, 2004
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DsgTftpServer.h"
#include "OperatingSystemFactory.h"

/*
* Trivial file transfer protocol server.
*
* This version includes many modifications by Jim Guyton <guyton@rand-unix>
*/

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <setjmp.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef TARGETOS_vxWorks
#include <ioLib.h>
#include <sockLib.h>
#endif

#include <string.h>

#include "dsg_api.h"

#define TFTP_PORT_NUMBER 69

#define MAX_SEGSIZE	512
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

#define	TIMEOUT 5		/* Default timeout (seconds) */
#define TRIES   4		/* Number of attempts to send each packet */
#define TIMEOUT_LIMIT (TRIES*TIMEOUT)

#define OACK	6
#define EOPTNEG	8

#ifdef __linux__
extern	int errno;
#endif

static struct sockaddr_in addr = { AF_INET };
static struct sockaddr_in from;

static int	peer;
static int	timeout    = TIMEOUT;
static int	rexmtval   = TIMEOUT;
static int	maxtimeout = TIMEOUT_LIMIT;

#define	PKTSIZE	(MAX_SEGSIZE + 4)

static char	buf[PKTSIZE], ackbuf[PKTSIZE];
static int fromlen;
static off_t tsize;
static int tsize_ok;

static int ndirs = 0;
static char **dirs;

static FILE *file;

static int segsize = MAX_SEGSIZE;

static struct bf {
	int counter;            /* size of data in buffer, or flag */
	char buf[PKTSIZE];      /* room for data packet */
} bfs[2];

#define BF_ALLOC -3             /* alloc'd but not yet filled */
#define BF_FREE  -2             /* free */

static int nextone;		/* index of next buffer to use */
static int current;		/* index of buffer in use */

/*
* Packet types.
*/
#define RRQ     01                              /* read request */
#define WRQ     02                              /* write request */
#define DATA    03                              /* data packet */
#define ACK     04                              /* acknowledgement */
#define TFTPERROR   05                              /* error code */

struct  tftphdr {
	short th_opcode;				/* packet type */
	union {
		unsigned short tu_block;	/* block # */
		short tu_code;				/* error code */
		char tu_stuff[1];			/* request packet stuff */
	} __attribute__ ((__packed__)) th_u;
	char th_data[1];				/* data or error string */
} __attribute__ ((__packed__));

#define th_block        th_u.tu_block
#define th_code         th_u.tu_code
#define th_stuff        th_u.tu_stuff
#define th_msg          th_data

/*
* Error codes.
*/
#define EUNDEF          0               /* not defined */
#define ENOTFOUND       1               /* file not found */
#define EACCESS         2               /* access violation */
#define ENOSPACE        3               /* disk full or allocation exceeded */
#define EBADOP          4               /* illegal TFTP operation */
#define EBADID          5               /* unknown transfer ID */
#define EEXISTS         6               /* file already exists */
#define ENOUSER         7               /* no such user */

static struct errmsg {
	int	e_code;
	const char	*e_msg;
} errmsgs[] = {
	{ EUNDEF,	"Undefined error code" },
	{ ENOTFOUND,"File not found" },
	{ EACCESS,	"Access violation" },
	{ ENOSPACE,	"Disk full or allocation exceeded" },
	{ EBADOP,	"Illegal TFTP operation" },
	{ EBADID,	"Unknown transfer ID" },
	{ EEXISTS,	"File already exists" },
	{ ENOUSER,	"No such user" },
	{ EOPTNEG,	"Failure to negotiate RFC2347 options" },
	{ -1,		0 }
};

#if defined(TARGETOS_vxWorks)
typedef int socklen_t;
#endif

#if defined(TARGETOS_vxWorks)
/*
* This array is designed for mapping upper and lower case letter
* together for a case independent comparison.  The mappings are
* based upon ascii character sequences.
*/
typedef unsigned char uc;
static unsigned char charmap[] = {
	(uc)'\000',(uc)'\001',(uc)'\002',(uc)'\003',(uc)'\004',(uc)'\005',(uc)'\006',(uc)'\007',
	(uc)'\010',(uc)'\011',(uc)'\012',(uc)'\013',(uc)'\014',(uc)'\015',(uc)'\016',(uc)'\017',
	(uc)'\020',(uc)'\021',(uc)'\022',(uc)'\023',(uc)'\024',(uc)'\025',(uc)'\026',(uc)'\027',
	(uc)'\030',(uc)'\031',(uc)'\032',(uc)'\033',(uc)'\034',(uc)'\035',(uc)'\036',(uc)'\037',
	(uc)'\040',(uc)'\041',(uc)'\042',(uc)'\043',(uc)'\044',(uc)'\045',(uc)'\046',(uc)'\047',
	(uc)'\050',(uc)'\051',(uc)'\052',(uc)'\053',(uc)'\054',(uc)'\055',(uc)'\056',(uc)'\057',
	(uc)'\060',(uc)'\061',(uc)'\062',(uc)'\063',(uc)'\064',(uc)'\065',(uc)'\066',(uc)'\067',
	(uc)'\070',(uc)'\071',(uc)'\072',(uc)'\073',(uc)'\074',(uc)'\075',(uc)'\076',(uc)'\077',
	(uc)'\100',(uc)'\141',(uc)'\142',(uc)'\143',(uc)'\144',(uc)'\145',(uc)'\146',(uc)'\147',
	(uc)'\150',(uc)'\151',(uc)'\152',(uc)'\153',(uc)'\154',(uc)'\155',(uc)'\156',(uc)'\157',
	(uc)'\160',(uc)'\161',(uc)'\162',(uc)'\163',(uc)'\164',(uc)'\165',(uc)'\166',(uc)'\167',
	(uc)'\170',(uc)'\171',(uc)'\172',(uc)'\133',(uc)'\134',(uc)'\135',(uc)'\136',(uc)'\137',
	(uc)'\140',(uc)'\141',(uc)'\142',(uc)'\143',(uc)'\144',(uc)'\145',(uc)'\146',(uc)'\147',
	(uc)'\150',(uc)'\151',(uc)'\152',(uc)'\153',(uc)'\154',(uc)'\155',(uc)'\156',(uc)'\157',
	(uc)'\160',(uc)'\161',(uc)'\162',(uc)'\163',(uc)'\164',(uc)'\165',(uc)'\166',(uc)'\167',
	(uc)'\170',(uc)'\171',(uc)'\172',(uc)'\173',(uc)'\174',(uc)'\175',(uc)'\176',(uc)'\177',
	(uc)'\200',(uc)'\201',(uc)'\202',(uc)'\203',(uc)'\204',(uc)'\205',(uc)'\206',(uc)'\207',
	(uc)'\210',(uc)'\211',(uc)'\212',(uc)'\213',(uc)'\214',(uc)'\215',(uc)'\216',(uc)'\217',
	(uc)'\220',(uc)'\221',(uc)'\222',(uc)'\223',(uc)'\224',(uc)'\225',(uc)'\226',(uc)'\227',
	(uc)'\230',(uc)'\231',(uc)'\232',(uc)'\233',(uc)'\234',(uc)'\235',(uc)'\236',(uc)'\237',
	(uc)'\240',(uc)'\241',(uc)'\242',(uc)'\243',(uc)'\244',(uc)'\245',(uc)'\246',(uc)'\247',
	(uc)'\250',(uc)'\251',(uc)'\252',(uc)'\253',(uc)'\254',(uc)'\255',(uc)'\256',(uc)'\257',
	(uc)'\260',(uc)'\261',(uc)'\262',(uc)'\263',(uc)'\264',(uc)'\265',(uc)'\266',(uc)'\267',
	(uc)'\270',(uc)'\271',(uc)'\272',(uc)'\273',(uc)'\274',(uc)'\275',(uc)'\276',(uc)'\277',
	(uc)'\300',(uc)'\341',(uc)'\342',(uc)'\343',(uc)'\344',(uc)'\345',(uc)'\346',(uc)'\347',
	(uc)'\350',(uc)'\351',(uc)'\352',(uc)'\353',(uc)'\354',(uc)'\355',(uc)'\356',(uc)'\357',
	(uc)'\360',(uc)'\361',(uc)'\362',(uc)'\363',(uc)'\364',(uc)'\365',(uc)'\366',(uc)'\367',
	(uc)'\370',(uc)'\371',(uc)'\372',(uc)'\333',(uc)'\334',(uc)'\335',(uc)'\336',(uc)'\337',
	(uc)'\340',(uc)'\341',(uc)'\342',(uc)'\343',(uc)'\344',(uc)'\345',(uc)'\346',(uc)'\347',
	(uc)'\350',(uc)'\351',(uc)'\352',(uc)'\353',(uc)'\354',(uc)'\355',(uc)'\356',(uc)'\357',
	(uc)'\360',(uc)'\361',(uc)'\362',(uc)'\363',(uc)'\364',(uc)'\365',(uc)'\366',(uc)'\367',
	(uc)'\370',(uc)'\371',(uc)'\372',(uc)'\373',(uc)'\374',(uc)'\375',(uc)'\376',(uc)'\377',
};

int strcasecmp(const char *s1, const char *s2)
{
	unsigned char u1, u2;

	for (;;) {
		u1 = (unsigned char) *s1++;
		u2 = (unsigned char) *s2++;
		if (charmap[u1] != charmap[u2]) {
			return charmap[u1] - charmap[u2];
		}
		if (u1 == '\0') {
			return 0;
		}
	}
}

#endif

/*
* Set a non-standard block size (c.f. RFC2348)
*/
static int set_blksize(char *val, char **ret)
{
	static char b_ret[6];
	int sz = atoi(val);

	if (sz < 8)
		return(0);
	else if (sz > MAX_SEGSIZE)
		sz = MAX_SEGSIZE;

	segsize = sz;
	sprintf(*ret = b_ret, "%d", sz);
	return 1;
}

/*
* Return a file size (c.f. RFC2349)
* For netascii mode, we don't know the size ahead of time;
* so reject the option.
*/
static int set_tsize(char *val, char **ret)
{
	static char b_ret[16];
	off_t sz = atol(val);

	if ( !tsize_ok )
		return 0;

	if (sz == 0)
		sz = tsize;
	sprintf(*ret = b_ret, "%lu", sz);
	return 1;
}

/*
* Set the timeout (c.f. RFC2349)
*/
static int set_timeout(char *val, char **ret)
{
	static char b_ret[4];
	unsigned long to = atol(val);

	if ( to < 1 || to > 255 )
		return 0;

	timeout    = to;
	rexmtval   = to;
	maxtimeout = TRIES * to;

	sprintf(*ret = b_ret, "%lu", to);
	return 1;
}

static struct options {
	const char    *o_opt;
	int     (*o_fnc)(char *, char **);
} options[] = {
	{ "blksize",    set_blksize  },
	{ "tsize",      set_tsize },
	{ "timeout",	set_timeout  },
	{ NULL,         NULL }
};

static void tftpError(const char *f)
{
	char msg[256];
	char fmt[256];
	BcmDsgTftpThread *pTftpThread = BcmDsgTftpThread::GetSingletonInstance();
	strcpy(fmt, f);
#ifdef __linux__
	strcat(fmt, ": %m");
	snprintf(msg, sizeof(msg), fmt, errno);
#endif
#if defined(TARGETOS_vxWorks)
	strcat(fmt, ": %d");
	sprintf(msg, fmt, errno);
#endif
	pTftpThread->ErrorMessage(msg);
}

void BcmDsgTftpThread::ErrorMessage(char *msg)
{
	gErrorMsg(fMessageLogSettings, "BcmDsgTftpThread") << msg << endl;
}

static struct formats {
	const char	*f_mode;
} formats[] = {
	{ "netascii" },
	{ "octet" },
	{ NULL }
};

/*
* Validate file access.  Since we
* have no uid or gid, for now require
* file to exist and be publicly
* readable/writable.
* If we were invoked with arguments
* from inetd then the file must also be
* in one of the given directory prefixes.
* Note also, full path name must be
* given as we have no login directory.
*/
static int validate_access(char *filename, int mode, struct formats *pf)
{
	struct stat stbuf;
	int	fd;

	tsize_ok = 0;

	/*
	* We use different a different permissions scheme if `cancreate' is
	* set.
	*/
	if (stat(filename, &stbuf) < 0) {
		return (errno == ENOENT ? ENOTFOUND : EACCESS);
	} else {
#ifdef __linux__
		if ((stbuf.st_mode&(S_IREAD >> 6)) == 0)
			return (EACCESS);
#endif
		tsize = stbuf.st_size;
		tsize_ok = 1;
	}
	fd = open(filename, O_RDONLY, 0666);
	if (fd < 0)
		return (errno + 100);
	file = fdopen(fd, "r");
	if (file == NULL)
		return (errno + 100);
	return 0;
}

/* When an error has occurred, it is possible that the two sides
* are out of synch.  Ie: that what I think is the other side's
* response to packet N is really their response to packet N-1.
*
* So, to try to prevent that, we flush all the input queued up
* for us on the network connection on our host.
*/

static void synchnet(int f)		/* socket to flush */
{
	int i = 0;
	char rbuf[PKTSIZE];
	struct sockaddr_in from;
	socklen_t fromlen;

	while (1) {
#ifdef __linux__
		ioctl(f, FIONREAD, &i);
#endif
#ifdef TARGETOS_vxWorks
		ioctl(f, FIONREAD, (int)&i);
#endif
		if (i) {
			fromlen = sizeof from;
			recvfrom(f, rbuf, sizeof (rbuf), 0, (struct sockaddr *)&from, &fromlen);
		} else {
			break;
		}
	}
}

/* init for read-ahead */
static struct tftphdr *rw_init()
{
	bfs[0].counter = BF_ALLOC;     /* pass out the first buffer */
	current = 0;
	bfs[1].counter = BF_FREE;
	nextone = 1;                    /* one ahead */
	return (struct tftphdr *)bfs[0].buf;
}


/*
* fill the input buffer
*/
static void read_ahead(FILE *file)
{
	struct bf *b;
	struct tftphdr *dp;

	b = bfs + nextone;              /* look at "next" buffer */
	if (b->counter != BF_FREE)      /* nop if not free */
		return;
	nextone ^= 1;					/* "incr" next buffer ptr */
	dp = (struct tftphdr *)b->buf;
	b->counter = read(fileno(file), dp->th_data, segsize);
	return;
}

/* Have emptied current buffer by sending to net and getting ack.
   Free it and return next buffer filled with data.
*/
static int readit(FILE *file, struct tftphdr **dpp)
{
	struct bf *b;

	bfs[current].counter = BF_FREE; /* free old one */
	current ^= 1;    /* "incr" current */
	b = bfs + current;              /* look at new buffer */
	if (b->counter == BF_FREE)      /* if it's empty */
		read_ahead(file);           /* fill it */
	*dpp = (struct tftphdr *)b->buf;/* set caller's ptr */
	return b->counter;
}

/*
* Send a nak packet (error message).
* Error code passed in is one of the
* standard TFTP codes, or a UNIX errno
* offset by 100.
*/
static void nak(int error)
{
	struct tftphdr *tp;
	int length;
	struct errmsg *pe;

	tp = (struct tftphdr *)buf;
	tp->th_opcode = htons((u_short)TFTPERROR);
	tp->th_code = htons((u_short)error);
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			break;
	if (pe->e_code < 0) {
		pe->e_msg = strerror(error - 100);
		tp->th_code = EUNDEF;   /* set 'undef' errorcode */
	}
	strcpy(tp->th_msg, pe->e_msg);
	length = strlen(pe->e_msg);
	tp->th_msg[length] = '\0';
	length += 5;
	if (sendto(peer, buf, length, 0, (sockaddr*)&addr, sizeof(addr)) != length)
		tftpError("nak");
}

/*
* Send the requested file.
*/
static void sendfile(struct formats *pf, struct tftphdr *oap, int oacklen)
{
	struct tftphdr *dp;
	struct tftphdr *ap;    /* ack packet */
	int block = 1, size, n;
	struct timeval optval;

	ap = (struct tftphdr *)ackbuf;

abandonpacket:
	if (oap) {
		timeout = 0;
resendoack:
		if (sendto(peer, (char *)oap, oacklen, 0, (sockaddr*)&addr, sizeof(addr)) != oacklen) {
			tftpError("tftpd: oack");
			goto abort;
		}
		for ( ; ; ) {
			optval.tv_sec = rexmtval;
			optval.tv_usec = 0;
			if (setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO, &optval, sizeof(optval)) == -1) {
				tftpError("tftp: cant set socket options");
			}
			n = recv(peer, ackbuf, sizeof(ackbuf), 0);
			if (n < 0) {
				if (errno == EWOULDBLOCK)
				{
					timeout += rexmtval;
					if (timeout >= maxtimeout)
						goto abandonpacket;
					goto resendoack;
				}
				tftpError("tftpd: read");
				goto abort;
			}
			ap->th_opcode = ntohs((u_short)ap->th_opcode);
			ap->th_block = ntohs((u_short)ap->th_block);

			if (ap->th_opcode == TFTPERROR) {
				tftpError("tftp: client does not accept options");
				goto abort;
			}
			if (ap->th_opcode == ACK) {
				if (ap->th_block == 0)
					break;
				/* Resynchronize with the other side */
				synchnet(peer);
				goto resendoack;
			}
		}
	}

	dp = rw_init();
	do {
		size = readit(file, &dp);
		if (size < 0) {
			nak(errno + 100);
			goto abort;
		}
		dp->th_opcode = htons((u_short)DATA);
		dp->th_block = htons((u_short)block);
		timeout = 0;

resendpacket:
		if (sendto(peer, (char *)dp, size + 4, 0, (sockaddr*)&addr, sizeof(addr)) != size + 4) {
			tftpError("tftpd: write");
			goto abort;
		}
		read_ahead(file);
		for ( ; ; ) {
			optval.tv_sec = rexmtval;
			optval.tv_usec = 0;
			if (setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO, &optval, sizeof(optval)) == -1) {
				tftpError("tftp: cant set socket options");
			}
			n = recv(peer, ackbuf, sizeof (ackbuf), 0);
			if (n < 0) {
				if (errno == EWOULDBLOCK)
				{
					timeout += rexmtval;
					if (timeout >= maxtimeout)
						goto abandonpacket;
					goto resendpacket;
				}
				tftpError("tftpd: read(ack)");
				goto abort;
			}
			ap->th_opcode = ntohs((u_short)ap->th_opcode);
			ap->th_block = ntohs((u_short)ap->th_block);

			if (ap->th_opcode == TFTPERROR)
				goto abort;

			if (ap->th_opcode == ACK) {
				if (ap->th_block == block) {
					break;
				}
				/* Re-synchronize with the other side */
				synchnet(peer);
				if (ap->th_block == (block - 1)) {
					goto resendpacket;
				}
			}

		}
		block++;
	} while (size == segsize);
abort:
	optval.tv_sec = 0;
	optval.tv_usec = 0;
	if (setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO, &optval, sizeof(optval)) == -1) {
		tftpError("tftp: cant set socket options");
	}
	fclose(file);
}

/*
* Parse RFC2347 style options
*/
static void do_opt(char *opt, char *val, char **ap)
{
	struct options *po;
	char *ret;

	if ( !*opt )
		return;

	for (po = options; po->o_opt; po++)
		if (!strcasecmp(po->o_opt, opt)) {
			if (po->o_fnc(val, &ret)) {
				if (*ap + strlen(opt) + strlen(ret) + 2 >= ackbuf + sizeof(ackbuf)) {
						nak(ENOSPACE);	/* EOPTNEG? */
						return;
				}
				*ap = strrchr(strcpy(strrchr(strcpy(*ap, opt),'\0') + 1, ret),'\0') + 1;
			} else {
				nak(EOPTNEG);
				return;
			}
			break;
		}
		return;
}

/*
* Handle initial connection protocol.
*/
static int tftp(struct tftphdr *tp, int size)
{
	char *cp;
	int argn, ecode;
	struct formats *pf = NULL;
	char *filename, *mode = NULL;

	char *val = NULL, *opt = NULL;
	char *ap = ackbuf + 2;

	((struct tftphdr *)ackbuf)->th_opcode = ntohs(OACK);

	filename = cp = tp->th_stuff;
	argn = 0;

	while ( cp < buf + size && *cp ) {
		do {
			cp++;
		} while (cp < buf + size && *cp);

		if ( *cp ) {
			nak(EBADOP);	/* Corrupt packet - no final NULL */
			return 0;
		}

		argn++;
		if (argn == 1) {
			mode = ++cp;
		} else if (argn == 2) {
			for (cp = mode; *cp; cp++)
				*cp = tolower(*cp);
			for (pf = formats; pf->f_mode; pf++) {
				if (!strcmp(pf->f_mode, mode))
					break;
			}
			if (!pf->f_mode) {
				nak(EBADOP);
				return 0;
			}
			ecode = validate_access(filename, tp->th_opcode, pf);
			if (ecode) {
				nak(ecode);
				return 0;
			}
			opt = ++cp;
		} else if ( argn & 1 ) {
			val = ++cp;
		} else {
			do_opt(opt, val, &ap);
			opt = ++cp;
		}
	}

	if (!pf) {
		nak(EBADOP);
		return 0;
	}

	if (ap != (ackbuf+2)) {
		if (tp->th_opcode == RRQ)
			sendfile(pf, (tftphdr *)ackbuf, ap-ackbuf);
	} else {
		if (tp->th_opcode == RRQ)
			sendfile(pf, NULL, 0);
	}
	return 1;
}

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Global Variables *************************************

BcmDsgTftpThread *BcmDsgTftpThread::pfSingletonInstance = NULL;

//********************** Local Variables *************************************

//********************** Local Functions *************************************

//********************** Class Method Implementations ************************

BcmDsgTftpThread::BcmDsgTftpThread(BcmOperatingSystem::ThreadPriority initialPriority) :
    BcmThread("BcmThread", false, initialPriority)
{
    // Override the class name given by my parent.
    fMessageLogSettings.SetModuleName("BcmDsgTftpThread");

    // Set up the singleton.
    if (pfSingletonInstance != NULL)
    {
        gWarningMsg(fMessageLogSettings, "BcmDsgTftpThread")
            << "Singleton pointer is not NULL!  There are multiple BcmDsgTftpThread!  Leaving the singleton pointer alone..." << endl;
    }
    else
    {
        gInfoMsg(fMessageLogSettings, "BcmDsgTftpThread")
            << "Setting up singleton pointer." << endl;

        pfSingletonInstance = this;
    }

    // Spawn my thread.
#if defined(TARGETOS_vxWorks)
    if (pfOperatingSystem->BeginThread(this, 16 * 1024) == false)
#else
    if (pfOperatingSystem->BeginThread(this, 8 * 1024) == false)
#endif
    {
        gFatalErrorMsg( fMessageLogSettings, "BcmDsgTftpThread" ) 
            << "Failed to spawn my thread!" << endl;
        assert(0);
    }
}

// ~BcmDsgTftpThread() - Object constructor
//		Signals thread and waits for termination.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
BcmDsgTftpThread::~BcmDsgTftpThread()
{
    gFuncEntryExitMsg( fMessageLogSettings, "~BcmDsgTftpThread" ) << "Entering..." << endl;    

    gInfoMsg( fMessageLogSettings, "~BcmDsgTftpThread" ) 
        << "Signalling the thread to exit..." << endl;

    // Wait for the thread to exit.
    WaitForThread();

    gFuncEntryExitMsg(fMessageLogSettings, "~BcmDsgTftpThread") << "Exiting..." << endl;
}

// Initialize() - Initialize thread
//		This is the first method called after the thread has
//		been spawned, and is where the thread should create all OS objects.
//		This has to be done here, rather than in the object's constructor,
//		because some OS objects must be created in the context of the thread
//		that will use them.  The object's constructor is still running in the
//		context of the thread that created this object.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
bool BcmDsgTftpThread::Initialize()
{
    return true;
}

// ThreadMain()
//		Implements the execution thread. Retrieves the local image version and compare it
//		to the eCM image version. Upgrade the eCM if it is running an older version.
//
// Parameters:
//		None.
//
// Returns:
//      None.
//

void BcmDsgTftpThread::ThreadMain()
{
	// Sleep for not other purpose but to make the console and event log pretty
	BcmOperatingSystemFactory::ThreadSleep(500);

    gFuncEntryExitMsg( fMessageLogSettings, "ThreadMain" ) << "Entering..." << endl;

	gAlwaysMsg( fMessageLogSettings, "ThreadMain" ) << "Starting DSG Common Download TFTP Server thread." << endl;

	struct tftphdr *tp;
	int n = 0;
	int on = 1;
	int fd = 0;
	socklen_t l;

	from.sin_family = AF_INET;
	peer = socket(AF_INET, SOCK_DGRAM, 0);
	if (peer < 0) {
		tftpError("socket");
		return;
	}
	bzero((char *)&addr, sizeof(addr));
#ifdef TARGETOS_vxWorks
	addr.sin_len = sizeof(addr);
#endif
	addr.sin_family = AF_INET;         // host byte order
	addr.sin_port = htons(TFTP_PORT_NUMBER); // short, network byte order
#if defined(USE_PRIVATE_NETWORK)
	addr.sin_addr.s_addr = htonl(kLocalRxIpAddress);  // use my IP address
#else
	addr.sin_addr.s_addr = htonl(0);  // use any address
#endif
	if (bind(peer, (struct sockaddr *)&addr, sizeof (addr)) < 0) {
		tftpError("bind");
		return;
	}
	while (1) {
		l = sizeof(addr);
		n = recvfrom(peer, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &l);
		if (n >= 0) {
			tp = (struct tftphdr *)buf;
			tp->th_opcode = ntohs(tp->th_opcode);
			if (tp->th_opcode == RRQ)
				tftp(tp, n);
		}
		else {
			tftpError("recvfrom");
		}
	}

    gFuncEntryExitMsg( fMessageLogSettings, "ThreadMain" ) << "Exiting..." << endl;
}

// Print() - Print detailed formatted object contents 
//      to specified ostream.
//
// Parameters:
//      ostream& ostrm - reference to the destination output stream.
//
// Returns:
//      reference to the destination output stream.
//
ostream& BcmDsgTftpThread::Print( ostream& ostrm ) const
{
	return ostrm;
}



