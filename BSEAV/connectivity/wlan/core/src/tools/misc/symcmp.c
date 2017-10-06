/*
 * Takes two symsize files and returns a comparison showing what changed
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

#define LINESIZE	256		/* max acceptable lin size */

int match = 0;

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
pr(FILE *o, char *func, int size1, int size2, int delta, char *comment)
{
	if (comment == NULL)
		comment = "";

	fprintf(o, "%-24s\t%6d\t%6d\t%+6d\t%s\n",
		func, size1, size2, delta, comment);
}

int
smatch(char *name1, char *name2)
{
	size_t baselen = strcspn(name1, ".");

	if ((baselen == strcspn(name2, ".")) &&
	    !strncmp(name1, name2, baselen) &&
	    (name1[baselen+strspn(name1+baselen, ".0123456789")] == '\0') &&
	    (name2[baselen+strspn(name1+baselen, ".0123456789")] == '\0'))
		return 1;
	else
		return 0;
}

void
convert(FILE *i1, FILE *i2, FILE *o)
{
	char *b1, *b2, func1[LINESIZE], func2[LINESIZE];
	size_t n, m;
	int test, test2, size1, size2;
	int diff, reduced, added, tot;

	n = m = LINESIZE;
	reduced = 0;
	added = 0;
	tot = 0;

	if ((b1 = malloc(n)) == NULL)
		syserr("malloc");
	if ((b2 = malloc(m)) == NULL)
		syserr("malloc");

	/* check for text/data which changed size or was added */
	while (getaline(b1, n, i1) != -1) {
		sscanf(b1, "%s\t%d", func1, &size1);
		while ((test = getaline(b2, m, i2)) != -1) {
			sscanf(b2, "%s\t%d", func2, &size2);
			if (!strcmp(func2, func1)) {
				diff = size2 - size1;

				while (match && diff && (test2 = getaline(b2, m, i2)) != -1) {
					sscanf(b2, "%s\t%d", func2, &size2);
					if (!strcmp(func2, func1) && (size2 == size1))
						diff = 0;
				}

				tot += diff;
				if (diff > 0)
					added += diff;
				else
					reduced += diff;

				if (diff)
					pr(o, func1, size1, (size1+diff), diff, NULL);
				break;
			}
			if (match && smatch(func1, func2) && (size1 == size2))
				break;
		}
		if (test == -1) {
			reduced -= size1;
			tot -= size1;
			pr(o, func1, size1, 0, -size1, "(removed)");
		}
		if (fseek(i2, SEEK_SET, 0) < 0)
			syserr("fseek");
	}

	/* for for old text/data that has been removed */
	if (fseek(i1, SEEK_SET, 0) < 0)
		syserr("fseek");
	if (fseek(i2, SEEK_SET, 0) < 0)
		syserr("fseek");

	while (getaline(b1, n, i2) != -1) {
		sscanf(b1, "%s\t%d", func1, &size1);
		while ((test = getaline(b2, m, i1)) != -1) {
			sscanf(b2, "%s\t%d", func2, &size2);
			if (!strcmp(func2, func1))
				break;
			if (match && smatch(func1, func2) && (size1 == size2))
				break;
		}
		if (test == -1)	{
			pr(o, func1, 0, size1, size1, "(added)");
			added += size1;
			tot += size1;
		}
		if (fseek(i1, SEEK_SET, 0) < 0)
			syserr("fseek");
	}

	fprintf(o, "\n\nFile diff %+d     added %+d   reduced %+d\n", tot, added, reduced);

	free(b1);
	free(b2);
}

void
usage(char *av[])
{
	fprintf(stderr, "usage: %s [ -m ] symsizefile1 symsizefile2 [ outfile ]\n", av[0]);
	fprintf(stderr, "       -m   match symbols <name>.[<dot-digits>] of the same size\n");
	exit(1);
}

int
main(int ac, char *av[])
{
	FILE *i1, *i2, *o;
	char **args = &av[1];

	if ((ac > 1) && !strcmp(av[1], "-m")) {
		match = 1;
		args++;
		ac--;
	}

	if ((ac < 3) || (ac > 4))
		usage(av);

	if ((i1 = fopen(args[0], "r")) == NULL)
		syserr(args[0]);
	if ((i2 = fopen(args[1], "r")) == NULL)
		syserr(args[1]);

	if (ac == 3)
		o = stdout;
	else {
		if ((o = fopen(args[2], "w")) == NULL)
			syserr(args[2]);
	}

	convert(i1, i2, o);

	return (0);
}
