/*
 * Dongle simulated bus interface
 *
 * For use by the dongle sockets server (dongle.c, rserv),
 * devbus corresponds to hostbus on the client side.
 *
 * Provides attach, read and write functions.
 * Uses a stream socket to emulate a bus such as USB or SDIO,
 * which is assumed to be a reliable transport.
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

#include "os.h"
#include "devbus.h"

#define DPRINTF			if (0) printf

struct devbus {
	devbus_t pub;		/* Public part (must come first in struct) */

	struct sockaddr_in serv_addr;

	struct sockaddr_in cli_addr;
	int cli_fd;
};

static void
sigpipe(int signum)
{
}

devbus_t *
devbus_attach(int port)
{
	struct devbus *dp;
	int rv, one;

	DPRINTF("devbus_init: port=%d\n", port);

	if ((dp = malloc(sizeof (struct devbus))) == NULL) {
		fprintf(stderr,
		        "devbus_attach: out of memory\n");
		return NULL;
	}

	signal(SIGPIPE, sigpipe);

	if ((dp->pub.serv_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr,
		        "devbus_attach: error creating socket: %s\n",
		        strerror(errno));
		free(dp);
		return NULL;
	}

	one = 1;

	if ((rv = setsockopt(dp->pub.serv_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) < 0)
		fprintf(stderr,
		        "devbus_attach: error setting SO_REUSEADDR: %s\n",
		        strerror(errno));

	dp->serv_addr.sin_family = AF_INET;
	dp->serv_addr.sin_addr.s_addr = INADDR_ANY;
	dp->serv_addr.sin_port = htons(port);

	if ((rv = bind(dp->pub.serv_fd,
	               (struct sockaddr *)&dp->serv_addr, sizeof(dp->serv_addr))) < 0) {
		fprintf(stderr,
		        "devbus_attach: error binding socket: %s\n",
		        strerror(errno));
		close(dp->pub.serv_fd);
		free(dp);
		return NULL;
	}

	if ((rv = listen(dp->pub.serv_fd, 2)) < 0) {
		fprintf(stderr,
		        "devbus_attach: error listening on socket: %s\n",
		        strerror(errno));
		close(dp->pub.serv_fd);
		free(dp);
		return NULL;
	}

	dp->cli_fd = -1;

	return (devbus_t *)dp;
}

void
devbus_close(devbus_t *db)
{
	struct devbus *dp = (struct devbus *)db;

	close(dp->pub.serv_fd);

	if (dp->cli_fd >= 0)
		close(dp->cli_fd);

	free(dp);
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
			DPRINTF("devbus: readn rv=%d err=%d(%s)\n", rv, errno, strerror(errno));
			return rv;
		}
		if (rv == 0) {
			DPRINTF("devbus: readn rv=0\n");
			break;
		}
		got += rv;
	}

	return got;
}

/*
 * Read a request consisting of a 4-byte length followed by data,
 * thereby restoring message boundaries from TCP connection.
 * Returns length in bytes.
 */

static int
_devbus_read(int fd, unsigned char **req_buf, struct timeval *tv)
{
	int rv;
	uint32 len_n;
	size_t len;
	unsigned char *buf;

	/*
	 * Read header to get length, then read rest of data and append to
	 * header.
	 */

	DPRINTF("_devbus_read: wait\n");

	if ((rv = readn(fd, &len_n, 4)) < 0) {
		fprintf(stderr,
		        "_devbus_read: nread header (4 bytes) failed: %s\n",
		        strerror(errno));
		return rv;
	}

	if (rv < 4) {
		fprintf(stderr,
		        "_devbus_read: connection closed by client (%d)\n", rv);
		errno = EIO;
		return -1;
	}

	len = (int)ntohl(len_n);

	if (len <= 0 || len > DEVBUS_MTU) {
		fprintf(stderr,
		        "_devbus_read: bad length %d\n", (int)len);
		errno = EINVAL;
		return -1;
	}

	if ((buf = malloc(len)) == NULL) {
		fprintf(stderr,
		        "_devbus_read: out of memory\n");
		exit(1);
	}

	if ((rv = readn(fd, buf, len)) < 0) {
		fprintf(stderr,
		        "_devbus_read: nread data (%d) failed: %s\n",
		        (int)len, strerror(errno));
		free(buf);
		return rv;
	}

	if (rv < len) {
		fprintf(stderr,
		        "_devbus_read: connection closed by client\n");
		errno = EIO;
		free(buf);
		return -1;
	}

	DPRINTF("_devbus_read: len=%d {%02x%02x %02x%02x %02x%02x %02x%02x}\n",
	        (int)len,
	        buf[0], buf[1], buf[2], buf[3],
	        buf[4], buf[5], buf[6], buf[7]);

	*req_buf = buf;

	return (int)len;
}

int
devbus_read(devbus_t *db, unsigned char **req_buf, struct timeval *tv)
{
	struct devbus *dp = (struct devbus *)db;
	socklen_t cli_addr_len;
	fd_set rfds;
	int rv;

	for (;;) {
		if (dp->cli_fd < 0) {
			if (tv != NULL) {
				FD_ZERO(&rfds);
				FD_SET(dp->pub.serv_fd, &rfds);

				if ((rv = select(dp->pub.serv_fd + 1,
				                 &rfds, NULL, NULL, tv)) < 0) {
					/* Retry select if interrupted by SIGALARM, etc. */
					if (errno == EINTR || errno == ERESTART)
						continue;
					fprintf(stderr,
					        "devbus_read: select for accept: %s\n",
					        strerror(errno));
					return -1;
				}

				if (!FD_ISSET(dp->pub.serv_fd, &rfds)) {
					*req_buf = NULL;
					return 0;
				}
			}

			cli_addr_len = (socklen_t)sizeof(dp->cli_addr);

			DPRINTF("devbus_read: waiting for connection\n");

			if ((rv = accept(dp->pub.serv_fd,
			                 (struct sockaddr *)&dp->cli_addr, &cli_addr_len)) < 0) {
				if (errno == EINTR || errno == ERESTART)
					continue;
				fprintf(stderr,
				        "devbus_read: error accepting connection: %s\n",
				        strerror(errno));
				return -1;
			}

			dp->cli_fd = rv;

			DPRINTF("devbus_read: accepted connection from %s\n",
			        inet_ntoa(dp->cli_addr.sin_addr));
		}

		if (tv != NULL) {
			FD_ZERO(&rfds);
			FD_SET(dp->cli_fd, &rfds);

			if ((rv = select(dp->cli_fd + 1,
			                 &rfds, NULL, NULL, tv)) < 0) {
				/* Retry select if interrupted by SIGALARM, etc. */
				if (errno == EINTR || errno == ERESTART)
					continue;
				fprintf(stderr,
				        "devbus_read: select for read: %s\n",
				        strerror(errno));
				return 1;
			}

			if (!FD_ISSET(dp->cli_fd, &rfds)) {
				*req_buf = NULL;
				return 0;
			}
		}

		if ((rv = _devbus_read(dp->cli_fd, req_buf, tv)) >= 0)
			break;

		/* Disconnect on error and allow reconnection */

		close(dp->cli_fd);
		dp->cli_fd = -1;
	}

	return rv;
}

void
devbus_output(devbus_t *db, void *msg, int msg_len)
{
	struct devbus *dp = (struct devbus *)db;
	uint32 msg_len_n;
	int rv;

	DPRINTF("devbus_output: len=%d {%02x%02x %02x%02x %02x%02x %02x%02x}\n",
	        msg_len,
	        ((uint8 *)msg)[0], ((uint8 *)msg)[1], ((uint8 *)msg)[2], ((uint8 *)msg)[3],
	        ((uint8 *)msg)[4], ((uint8 *)msg)[5], ((uint8 *)msg)[6], ((uint8 *)msg)[7]);

	if (dp->cli_fd < 0) {
		fprintf(stderr,
		        "devbus_output: no host; output discarded\n");
		return;
	}

	ASSERT(msg_len > 0);

	msg_len_n = htonl(msg_len);

	if (sizeof(msg_len_n) != 4)
		abort();

	if ((rv = write(dp->cli_fd, &msg_len_n, 4)) < 0)
		fprintf(stderr,
		        "devbus_output: write error for header: %s\n",
		        strerror(errno));
	else if (rv != 4) {
		fprintf(stderr,
		        "devbus_output: write length error for header: rv=%d\n", rv);
		rv = EINVAL;
	} else if ((rv = write(dp->cli_fd, msg, msg_len)) < 0)
		fprintf(stderr,
		        "devbus_output: write error for data length %d: %s\n",
		        msg_len, strerror(errno));
	else if (rv != msg_len) {
		fprintf(stderr,
		        "devbus_output: write length error: wrote %d of %d\n",
		        rv, msg_len);
		rv = EINVAL;
	}

	if (rv < 0) {
		close(dp->cli_fd);
		dp->cli_fd = -1;
	}
}
