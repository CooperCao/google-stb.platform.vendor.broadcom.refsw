/*
 * usleep implementation for mac
 *
 * Copyright (C) 2001 Broadcom Corporation
 *
 * $Id$
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc < 2)
		return 1;
	if (strtol(argv[1], 0, 0) == 0)
		return 1;
	usleep(strtol(argv[1], 0, 0));
	return 0;
}
