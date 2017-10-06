/*
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 *
 * Description: Implement Linux timer functions
 *
 */
#include <typedefs.h>
#include <sys/time.h>
#include <unistd.h>

unsigned long wpscli_current_time()
{
	struct timeval now;

	gettimeofday(&now, NULL);

	return now.tv_sec;
}

void wpscli_sleep(unsigned long milli_seconds)
{
	usleep(milli_seconds * 1000);
}
