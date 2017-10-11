/*
 * RSock User Mode Host Bus Implementation for Simulation
 *
 * Host bus interface for Unix simulations, using VENET (virtual ethernet)
 * Sits below the rsock application library.
 * Provides attach, close, read and output functions.
 *
 * This is compiled as a separate file because it makes use of standard
 * Unix sockets and include files rather than the rsock sockets and
 * include files.
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
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "os.h"
#include "../hostbus.h"
#include "hostbus_sim.h"

#define DONGLE_PORT_DEFAULT	7200
#define DPRINTF			if (0) printf

static struct sockaddr_in serv_addr;
static int serv_fd = -1;

os_sem_t out_lock;

static void
sigpipe(int signum)
{
}

void
hostbus_attach(void)
{
	char *hostname, *s;
	int port;
	struct hostent *he;
	int rv, one;

	if ((hostname = getenv("DONGLE_HOST")) == NULL) {
		fprintf(stderr,
		        "hostbus_attach: DONGLE_HOST variable not set\n");
		exit(1);
	}

	if ((s = getenv("DONGLE_PORT")) != NULL)
		port = (int)strtol(s, NULL, 0);
	else
		port = DONGLE_PORT_DEFAULT;

	DPRINTF("hostbus_attach: hostname=%s port=%d\n", hostname, port);

	/*
	 * Use a stream socket to emulate the SDIO bus, which is assumed to
	 * be a reliable transport.
	 */

	if ((he = gethostbyname(hostname)) == NULL || he->h_addrtype != AF_INET) {
		fprintf(stderr,
		        "hostbus_attach: unknown host: %s\n", hostname);
		exit(1);
	}

	signal(SIGPIPE, sigpipe);

	if ((rv = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr,
		        "hostbus_attach: error creating socket: %s\n", strerror(errno));
		exit(1);
	}

	serv_fd = rv;

	one = 1;

	if ((rv = setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) < 0)
		fprintf(stderr,
		        "hostbus_attach: error setting SO_REUSEADDR: %s\n", strerror(errno));

	serv_addr.sin_family = he->h_addrtype;
	memcpy(&serv_addr.sin_addr, he->h_addr, sizeof(serv_addr.sin_addr));
	serv_addr.sin_port = htons(port);

	if ((rv = connect(serv_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
		fprintf(stderr,
		        "hostbus_attach: error connecting socket: %s\n", strerror(errno));
		exit(1);
	}

	os_sem_init(&out_lock, 1);
}

void
hostbus_close(void)
{
	if (serv_fd >= 0) {
		close(serv_fd);
		serv_fd = -1;
	}
}

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
		if ((rv = read(fd, (uint8 *)buf + got, n - got)) < 0) {
			DPRINTF("hostbus: readn rv=%d err=%d(%s)\n", rv, errno, strerror(errno));
			return rv;
		}
		if (rv == 0) {
			DPRINTF("hostbus: readn rv=0\n");
			break;
		}
		got += rv;
	}

	return got;
}

/*
 * Read a response, restoring message boundaries from TCP connection for
 * simulating SDIO packet transport.  Returns length in bytes.
 */

int
hostbus_read(void **resp_buf)
{
	int rv;
	uint32 len_n;
	size_t len;
	uint8 *buf;

	DPRINTF("hostbus_read: wait\n");

	if ((rv = readn(serv_fd, &len_n, 4)) < 0) {
		fprintf(stderr,
		        "hostbus_read: nread header (4 bytes) failed: %s\n",
		        strerror(errno));
		exit(1);
	}

	if (rv < 4) {
		fprintf(stderr,
		        "hostbus_read: server disconnected\n");
		exit(1);
	}

	len = (int)ntohl(len_n);

	if (len <= 0 || len > HOSTBUS_MTU) {
		fprintf(stderr, "hostbus_read: bad read length %d\n", (int)len);
		exit(1);
	}

	if ((buf = malloc(len)) == NULL) {
		fprintf(stderr,
		        "hostbus_read: out of memory (len=%d)\n", (int)len);
		exit(1);
	}

	if ((rv = readn(serv_fd, buf, len)) < 0) {
		fprintf(stderr,
		        "hostbus_read: nread data (%d bytes) failed: %s\n",
		        (int)len, strerror(errno));
		exit(1);
	}

	if (rv != len) {
		fprintf(stderr,
		        "hostbus_read: nread length error (got %d of %d): %s\n",
		        rv, (int)len, strerror(errno));
		exit(1);
	}

	DPRINTF("hostbus_read: len=%d {%02x%02x %02x%02x %02x%02x %02x%02x}\n",
	        (int)len,
	        buf[0], buf[1], buf[2], buf[3],
	        buf[4], buf[5], buf[6], buf[7]);

	*resp_buf = buf;

	return (int)len;
}

int
hostbus_output(void *msg, int msg_len)
{
	int rv;
	unsigned int msg_len_n;

	DPRINTF("hostbus_output: len=%d {%02x%02x %02x%02x %02x%02x %02x%02x}\n",
	        msg_len,
	        ((uint8 *)msg)[0], ((uint8 *)msg)[1], ((uint8 *)msg)[2], ((uint8 *)msg)[3],
	        ((uint8 *)msg)[4], ((uint8 *)msg)[5], ((uint8 *)msg)[6], ((uint8 *)msg)[7]);

	ASSERT(msg_len > 0);

	os_sem_take(&out_lock, OS_MSEC_FOREVER, 0);

	msg_len_n = htonl((unsigned int)msg_len);

	if (sizeof(msg_len_n) != 4)
		abort();

	if ((rv = write(serv_fd, &msg_len_n, 4)) < 0) {
		fprintf(stderr,
		        "hostbus_output: write error for header: %s\n",
		        strerror(errno));
		exit(1);
	}

	if (rv != 4) {
		fprintf(stderr,
		        "hostbus_output: write length error for header: rv=%d\n", rv);
		exit(1);
	}

	if ((rv = write(serv_fd, msg, msg_len)) < 0) {
		fprintf(stderr,
		        "hostbus_output: write error for data length %d: %s\n",
		        msg_len, strerror(errno));
		exit(1);
	}

	if (rv != msg_len) {
		fprintf(stderr,
		        "hostbus_output: write length error: wrote %d of %d\n",
		        rv, msg_len);
		exit(1);
	}

	os_sem_give(&out_lock);

	return 0;
}
