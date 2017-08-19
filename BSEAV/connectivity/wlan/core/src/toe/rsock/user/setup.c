/*
 * Linux user mode utility routines for rsock
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
#include <signal.h>

#include "os.h"
#include <rsock/socket.h>
#include <rsock/plumb.h>
#include "hostbus.h"

static int rsock_inited;

void
cleanup_rsock(void)
{
	if (rsock_inited) {
		rsock_term();
		rsock_inited = 0;
	}
}

void
cleanup_exit(int rv)
{
	cleanup_rsock();
	exit(rv);
}

/*
 * rsock packet handling callbacks
 */

typedef struct user_pkt_s {
	void *data;
	int len;
} user_pkt_t;

os_pkt_t *
user_pkt_alloc(int len)
{
	user_pkt_t *up;

	if ((up = malloc(sizeof(user_pkt_t))) == NULL) {
		fprintf(stderr, "user_pkt_alloc: out of memory (%d)\n", (int)sizeof(user_pkt_t));
		cleanup_exit(1);
	}

	if ((up->data = malloc(len)) == NULL) {
		fprintf(stderr, "user_pkt_alloc: out of memory (%d)\n", len);
		free(up);
		cleanup_exit(1);
	}

	up->len = len;

	return (os_pkt_t *)up;
}

void
user_pkt_free(os_pkt_t *pkt)
{
	user_pkt_t *up = (user_pkt_t *)pkt;

	/* This routine must be thread-safe */
	free(up->data);
	free(up);
}

void *
user_pkt_data(os_pkt_t *pkt)
{
	user_pkt_t *up = (user_pkt_t *)pkt;

	return up->data;
}

int
user_pkt_len(os_pkt_t *pkt)
{
	user_pkt_t *up = (user_pkt_t *)pkt;

	return up->len;
}

int
user_pkt_output(os_pkt_t *pkt)
{
	user_pkt_t *up = (user_pkt_t *)pkt;
	int rv;

	/* This routine must be thread-safe */

	rv = hostbus_output(up->data, up->len);

	free(up->data);
	free(up);

	return rv;
}

/*
 * Thread to read data from the host simulated bus interface and pass
 * it to rsock_input.  In a real system, the bus receive data interrupt
 * would pass the data to rsock_input without requiring a separate
 * thread.
 */

static void *
bus_input_thread(void *arg)
{
	void *resp_msg;
	int resp_len;
	sigset_t newmask, oldmask;

	sigemptyset(&newmask);
	sigaddset(&newmask, SIGINT);
	pthread_sigmask(SIG_BLOCK, &newmask, &oldmask);

	(void)arg;

	while (1) {
		os_pkt_t *pkt;
		void *data;

		/* Read a message (a malloc'd buffer) from the bus */

		resp_len = hostbus_read(&resp_msg);

		/* Convert to "native os" pkt format */

		pkt = user_pkt_alloc(resp_len);
		data = user_pkt_data(pkt);
		memcpy(data, resp_msg, resp_len);
		free(resp_msg);

		rsock_input(pkt);
	}
}

void
sigint_handler(int signo)
{
	printf("Interrupted\n");
	fflush(stdout);

	/* Try to clean up and exit on first ^C; force exit on second */
	signal(SIGINT, SIG_DFL);
	cleanup_exit(1);
}

void
setup_rsock(void)
{
	pthread_t tid;
	int rv;

	hostbus_attach();

	signal(SIGINT, sigint_handler);

	rsock_init();

	if ((rv = pthread_create(&tid, NULL, bus_input_thread, NULL)) < 0) {
		fprintf(stderr,
		        "setup_rsock: Could not fork bus_input_thread: %s\n",
		        strerror(rv));
		cleanup_exit(1);
	}


	rsock_inited = 1;
}
