/*
 * Linux user mode ttcp client to the rsock library
 *
 * Multi-ttcp is like ttcp, except the first command line argument
 * (preceding all the flags) is a count of how many parallel streams to
 * run.  Each stream is the same except the port numbers increment from
 * the base port number.  Example:
 *
 *     host1% ttcp 4 -r
 *     host2% ttcp 4 -t host1
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

#include <pthread.h>
#include <unistd.h>
#include "os.h"
#include "setup.h"
#include "ttcp.h"

#define MAX_COPIES	8

struct ttcp_instance {
	pthread_t tid;
	int copy;
	int argc;
	char **argv;
	int rv;
};

static void *
ttcp_thread(void *arg)
{
	struct ttcp_instance *ti = arg;

	ti->rv = ttcp(ti->argc, ti->argv, ti->copy);

	pthread_exit(NULL);
}

static char **
argv_dup(int argc, char **argv)
{
	char **av;
	int i;

	av = malloc((argc + 1) * sizeof(char *));

	for (i = 0; i < argc; i++)
		av[i] = strdup(argv[i]);

	av[i] = 0;

	return av;
}

int
multi_ttcp(int argc, char **argv, int copies)
{
	struct ttcp_instance tis[MAX_COPIES], *ti;
	int copy;
	int rv;

	for (copy = 0; copy < copies; copy++) {
		ti = &tis[copy];

		ti->copy = copy;
		ti->argc = argc;
		ti->argv = argv_dup(argc, argv);

		if ((rv = pthread_create(&ti->tid, NULL, ttcp_thread, ti)) < 0) {
			fprintf(stderr,
			        "Could not create ttcp thread %d: %s\n",
			        copy + 1, strerror(errno));
			exit(1);
		}

	}

	rv = 0;

	for (copy = 0; copy < copies; copy++) {
		void *ret;

		ti = &tis[copy];

		pthread_join(ti->tid, &ret);

		if (ti->rv)
			rv = ti->rv;
	}

	return rv;
}

int
main(int argc, char **argv)
{
	int rv;
	int copies;

	if (argc > 1 && (copies = atoi(argv[1])) > 0) {
		if (copies < 1)
			copies = 1;
		else if (copies > MAX_COPIES)
			copies = MAX_COPIES;
		argv++;
		argc--;
	} else
		copies = 1;

	/* Prepare for remote socket calls */

	setup_rsock();

	/* Run one or more ttcp's */

	rv = multi_ttcp(argc, argv, copies);

	/* Finish up */

	cleanup_rsock();

	exit(rv ? 1 : 0);

	/*NOTREACHED*/
	return 0;
}
