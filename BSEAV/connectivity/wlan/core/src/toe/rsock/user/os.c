/*
 * RSock User Mode Operating System Abstraction
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
#include <stdarg.h>
#include <sys/time.h>
#include <sys/select.h>

#include "os.h"

int
os_msleep(os_msec_t dur)
{
	struct timeval delay_tv;

	delay_tv.tv_sec = dur / 1000;
	delay_tv.tv_usec = (dur % 1000) * 1000;

	select(0, NULL, NULL, NULL, &delay_tv);

	return 0;
}

os_msec_t
os_msec(void)
{
	struct timeval now;

	gettimeofday(&now, 0);

	return (os_msec_t)(now.tv_sec * 1000 + now.tv_usec / 1000);
}

/*
 * 'sig' indicates fast timeout mode for a signaling semaphore.  A signaling semaphore
 * is a binary semaphore (value 0 or 1) that only one task can wait on, and that must
 * be re-initialized before each use.  It's not needed in this user mode implementation.
 */
int
os_sem_take(os_sem_t *sem, os_msec_t tmout, int sig)
{
	int err;
	os_msec_t interval;

	(void)sig;

	if (tmout == OS_MSEC_FOREVER) {
		do
			err = sem_wait(sem);
		while (err && errno == EINTR);
	} else {
		/* Retry with exponential backoff */

		interval = 1;

		while ((err = sem_trywait(sem)) != 0) {
			if (errno != EAGAIN && errno != EINTR) {
				printf("os_sem_take: unknown errno %d\n", errno);
				break;
			}

			if (tmout == 0)
				return ETIMEDOUT;

			os_msleep(interval);

			tmout -= interval;

			interval *= 2;

			if (interval > 1000)
				interval = 1000;

			if (interval > tmout)
				interval = tmout;
		}
	}

	return (err ? EIO : 0);
}

int
os_printerr(const char *fmt, ...)
{
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = vfprintf(stderr, fmt, ap);
	va_end(ap);

	return n;
}
