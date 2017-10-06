/*
 * Replacement of the NOW.EXE Windows 98 ResKit tool.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id$
 */
#include <stdio.h>
#include <time.h>

int
main(int argc, char** argv)
{
	time_t curr_time;
	char* prog_name = argv[0];

	/* Drop the prog name */
	argc--; argv++;

	if (argc > 0 &&
	    (argv[0][0] == '/' || argv[0][0] == '-') &&
	    (argv[0][1] == '?' || argv[0][1] == 'h')) {
		fprintf(stderr,
		        "Usage: %s [options | args]\n"
		        "       Prints the date and echos any other args separated\n"
		        "       from the date by \"--\".\n"
		        "Options: /?, -?, /h, or -h print this help.\n",
		        prog_name);
		return 2;
	}

	/* print the current time */
	time(&curr_time);
	printf("%.24s", ctime(&curr_time));

	/* echo any other args */
	if (argc > 0) {
		fputs(" --", stdout);

		while (argc > 0) {
			putchar(' ');
			fputs(*argv, stdout);
			argc--; argv++;
		}
	}

	putchar('\n');

	return 0;
}
