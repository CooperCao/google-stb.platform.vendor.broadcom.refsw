/*
 * RServ Simulation Operating System Abstraction
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
#include <sys/select.h>

#include "os.h"

int
os_sem_take(os_sem_t *sem, struct timeval *tmout)
{
	int		err;
	struct timeval	remain;
	struct timeval	interval, delay;

	if (tmout == NULL) {
		do
			err = sem_wait(sem);
		while (err && errno == EINTR);
	} else {
		interval.tv_sec = 0;
		interval.tv_usec = 1;

		remain.tv_sec = tmout->tv_sec;
		remain.tv_usec = tmout->tv_usec;

		/* Retry algorithm with exponential backoff */

		for (;;) {
			if (sem_trywait(sem) == 0) {
				err = 0;
				break;
			}

			if (errno != EAGAIN && errno != EINTR) {
				err = errno;
				break;
			}

			if (remain.tv_sec == 0 && remain.tv_usec == 0) {
				err = ETIMEDOUT;
				break;
			}

			delay.tv_sec = interval.tv_sec;
			delay.tv_usec = interval.tv_usec;

			select(0, NULL, NULL, NULL, &delay);

			tv_sub(&remain, &interval);

			tv_add(&interval, &interval);	/* Double interval */
			if (tv_cmp(&interval, &remain) > 0)
				tv_copy(&interval, &remain);
		}
	}

	return err ? -1 : 0;
}
