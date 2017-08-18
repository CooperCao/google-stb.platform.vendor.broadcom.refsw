/*
 * Virtual Ethernet Client Library
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>

#include "venetc.h"
#include "venet.h"

#define DPRINTF			if (0) printf

struct venetc {
	venetc_t pub;		/* Public part (must come first in struct) */

	char *serv_hostname;
	int serv_port;
	struct sockaddr_in serv_addr;
};

/*
 * Wait for specified number of bytes on socket.
 * Returns number of bytes read, or -1 on error and sets errno.
 * If less is read than requested, the connection was closed.
 */

static int
readn(int fd, void *buf, size_t n)
{
	int rv;
	size_t got = 0;

	while (got < n) {
		if ((rv = read(fd, (char *)buf + got, n - got)) < 0) {
			DPRINTF("venetc: readn rv=%d err=%d(%s)\n", rv, errno, strerror(errno));
			return rv;
		}
		if (rv == 0) {
			DPRINTF("venetc: readn rv=0\n");
			break;
		}
		got += rv;
	}

	return got;
}

static void
sigpipe(int signum)
{
}

venetc_t *
venetc_open(char *hostname, int port)
{
	struct venetc *vp;
	struct hostent *he;
	int one;
	struct sockaddr_in cli_addr;
	socklen_t cli_addr_len;
	unsigned int ip1, ip2;

	if ((vp = malloc(sizeof(struct venetc))) == NULL) {
		fprintf(stderr, "venetc: out of memory\n");
		exit(1);
	}

	if ((vp->serv_hostname = strdup(hostname)) == NULL) {
		fprintf(stderr, "venetc: out of memory\n");
		exit(1);
	}

	vp->serv_port = port;

	if ((he = gethostbyname(vp->serv_hostname)) == NULL ||
	    he->h_addrtype != AF_INET) {
		fprintf(stderr,
		        "venetc: unknown host: %s\n",
		        vp->serv_hostname);
		exit(1);
	}

	signal(SIGPIPE, sigpipe);

	if ((vp->pub.serv_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr,
		        "venetc: error creating socket: %s\n",
		        strerror(errno));
		exit(1);
	}

	one = 1;

	if (setsockopt(vp->pub.serv_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0)
		fprintf(stderr,
		        "venetc: error setting SO_REUSEADDR: %s\n",
		        strerror(errno));

	vp->serv_addr.sin_family = he->h_addrtype;
	memcpy(&vp->serv_addr.sin_addr, he->h_addr, sizeof(vp->serv_addr.sin_addr));
	vp->serv_addr.sin_port = htons(vp->serv_port);

	if (connect(vp->pub.serv_fd, (struct sockaddr *)&vp->serv_addr,
	            (socklen_t)sizeof(vp->serv_addr)) < 0) {
		fprintf(stderr,
		        "venetc: error connecting socket: %s\n",
		        strerror(errno));
		if (errno == ECONNREFUSED)
			fprintf(stderr,
			        "venetc: is venets running on %s port %d?\n",
			        vp->serv_hostname, vp->serv_port);
		exit(1);
	}

	cli_addr_len = (socklen_t)sizeof(cli_addr);
	if (getsockname(vp->pub.serv_fd, (struct sockaddr *)&cli_addr, &cli_addr_len) < 0) {
		fprintf(stderr,
		        "venetc: error getting local socket name: %s\n",
		        strerror(errno));
		exit(1);
	}

	/*
	 * Try to construct a unique MAC address by combining the server
	 * machine's IP address, the client machine's IP address (which
	 * could be the same), and the local process ID.
	 * Duplicates could still result.
	 */

	ip1 = (unsigned int)vp->serv_addr.sin_addr.s_addr;
	ip2 = (unsigned int)cli_addr.sin_addr.s_addr;

	vp->pub.mac[0] = ((ip1 >> 24) ^ (ip2 >> 24)) & 0xfc;
	vp->pub.mac[1] = ((ip1 >> 16) ^ (ip2 >> 16)) & 0xff;
	vp->pub.mac[2] = ((ip1 >> 8) ^ (getpid() >> 8)) & 0xff;
	vp->pub.mac[3] = ((ip1 >> 0) ^ (getpid() >> 0)) & 0xff;
	vp->pub.mac[4] = ((ip2 >> 8) ^ (getpid() >> 8)) & 0xff;
	vp->pub.mac[5] = ((ip2 >> 0) ^ (getpid() >> 0)) & 0xff;

	return (venetc_t *)vp;
}

int
venetc_close(venetc_t *vt)
{
	struct venetc *vp = (struct venetc *)vt;

	close(vp->pub.serv_fd);

	free(vp);

	return 0;
}

int
venetc_send(venetc_t *vt, unsigned char *msg, int msglen)
{
	struct venetc *vp = (struct venetc *)vt;
	unsigned int msglen_n;

	if (msglen < 0 || msglen > VENET_MTU)
		return -1;

	msglen_n = htonl((unsigned int)msglen);

	if (sizeof(msglen_n) != 4)
		abort();

	write(vp->pub.serv_fd, &msglen_n, 4);
	write(vp->pub.serv_fd, msg, msglen);

	DPRINTF("venetc: sent packet, length %d\n", msglen);

	return msglen;
}

/*
 * venetc_recv() is blocking.  If blocking is not desired, use select()
 * to check if input is pending on the publically exposed file
 * descriptor vt->fd before calling venetc_recv().
 */
int
venetc_recv(venetc_t *vt, unsigned char **msg_ptr, int *msglen_ptr)
{
	struct venetc *vp = (struct venetc *)vt;
	unsigned int msglen_n;
	int n;
	unsigned char *msg;
	int msglen;

	if ((n = readn(vp->pub.serv_fd, &msglen_n, 4)) < 0) {
		fprintf(stderr, "venetc: network read error: %s\n", strerror(errno));
		exit(1);
	}

	if (n < 4) {
		fprintf(stderr, "venetc: peer disconnected, n=%d of 4\n", n);
		exit(1);
	}

	msglen = (int)ntohl(msglen_n);

	if (msglen < 0 || msglen > VENET_MTU) {
		fprintf(stderr, "venetc: bad read length 0x%x\n", msglen);
		exit(1);
	}

	if ((msg = malloc(msglen)) == NULL) {
		fprintf(stderr, "venetc: out of memory\n");
		exit(1);
	}

	if ((n = readn(vp->pub.serv_fd, msg, msglen)) < 0) {
		free(msg);
		fprintf(stderr, "venetc: network read error: %s\n", strerror(errno));
		exit(1);
	}

	if (n < msglen) {
		free(msg);
		fprintf(stderr, "venetc: peer disconnected, n=%d of %d\n", n, msglen);
		exit(1);
	}

	DPRINTF("venetc: received packet, length %d\n", msglen);

	*msg_ptr = msg;
	*msglen_ptr = msglen;

	return msglen;
}

void
venetc_free(venetc_t *vt, unsigned char *msg)
{
	free(msg);
}
