/*
 * Takes a sorted map file as input and returns a list of symbol sizes
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESIZE	256	/* max acceptable line size */
#define	FUNCOFF		11	/* starting byte offset into each map file line of funcname */

void
syserr(char *s)
{
	perror(s);
	exit(1);
}

int
getaline(char *buf, size_t size, FILE *fp)
{
	int len;

	if (fgets(buf, size, fp) == NULL)
		return (-1);

	/* strip newline */
	len = strlen(buf);
	if (len < 1)
		return (-1);
	if (buf[len - 1] == '\n')
		buf[len - 1] = '\0';

	return (0);
}

void
convert(FILE *i, FILE *o)
{
	char *b1, tmp[LINESIZE];
	size_t n = LINESIZE;
	unsigned int a1 = 0, a2 = 0;
	char s1, s2;
	int reclaim = 0;

	if ((b1 = malloc(n)) == NULL)
		syserr("malloc");

	getaline(b1, n, i);
	sscanf(b1, "%8x %c", &a1, &s1);
	strcpy(tmp, b1);

	/* Read two lines of text in */
	while (getaline(b1, n, i) != -1) {
		sscanf(b1, "%8x %c", &a2, &s2);
		/* Skip ROM symbols */
		if (s1 != 'A' && s1 != 'a') {
			fprintf(o, "%-24s\t%-6d", &tmp[FUNCOFF], a2 - a1);
			if (reclaim)
				fprintf(o, " (Reclaim)\n");
			else
				fprintf(o, "\n");
		}

		if (strcmp(&tmp[FUNCOFF], "_rstart1") == 0)
			reclaim = 1;
		if (strcmp(&tmp[FUNCOFF], "_rend2") == 0)
			reclaim = 0;

		strcpy(tmp, b1);
		a1 = a2;
		s1 = s2;
	}
	free(b1);

	fclose(i);
	fclose(o);
}

void
usage(char *av[])
{
	fprintf(stderr, "usage: %s [ infile [ outfile ] ]\n", av[0]);
	exit(1);
}

int
main(int ac, char *av[])
{
	FILE *i, *o;

	switch (ac) {
	case 1:
		i = stdin;
		o = stdout;
		break;

	case 2:
		if ((i = fopen(av[1], "r")) == NULL)
			syserr(av[1]);
		o = stdout;
		break;

	case 3:
		if ((i = fopen(av[1], "r")) == NULL)
			syserr(av[1]);
		if ((o = fopen(av[2], "w")) == NULL)
			syserr(av[2]);
		break;

	default:
		usage(av);
		exit(1);
	}

	convert(i, o);

	return (0);
}
