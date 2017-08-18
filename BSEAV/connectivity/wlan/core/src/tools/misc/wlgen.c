/*
 * Generate C header file output from wl config file input.
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

void process(void);
void push(char *file, char include);
void pop(void);
void usage(char *s);
void syserr(char *s);

char *idir = ".";
FILE *ofp = NULL;
char incprefix[] = "$(WLCFGDIR)/";

#define		PATHL		160
#define 	STACKSZ		8

int DEBUG = 0;

/* stack of file names and open file pointers */
struct stackentry {
	char path[PATHL];
	FILE *fp;
	int tunes;
} stack[STACKSZ];
int stacktop = -1;

int
main(int ac, char *av[])
{
	char *infile = NULL;
	char *outfile = NULL;

	switch (ac) {
	case 3:
		infile = av[1];
		outfile = av[2];
		break;

	case 5:
		if (strcmp(av[1], "-I"))
			usage(av[0]);
		idir = av[2];
		infile = av[3];
		outfile = av[4];
		break;

	case 6:
		if (strcmp(av[1], "-I"))
		usage(av[0]);
		idir = av[2];
		infile = av[3];
		outfile = av[4];
		if (strstr(av[5], "dbgon") != NULL)
			DEBUG = 1;
		else
			usage(av[0]);
		break;

	default:
		usage(av[0]);
	}

	push(infile, 0);
	if ((ofp = fopen(outfile, "w")) == NULL) {
		fprintf(stderr, "fail to open output file %s\n", outfile);
		syserr(outfile);
	}

	if (DEBUG == 1) {
		printf("input file=%s\n", infile);
		printf("output file=%s\n", outfile);
	}
	process();

	fclose(ofp);
	exit(0);
}

void
process()
{
	char line[128];
	char file[PATHL];
	char incfile[PATHL];
	int len;
	char *s;

	fprintf(ofp, "/* Auto-generated from %s : don't edit */\n", stack[stacktop].path);

	while (1) {
		if (fgets(line, 128, stack[stacktop].fp) == NULL) {
			pop();
			continue;
		}

		/* remove trailing newline */
		len = strlen(line);
		if (len > 0)
			line[len - 1] = '\0';

		/* recognize "^include" and push new file onto the stack */
		if (sscanf(line, "include %s", file) == 1) {
			if (DEBUG == 1) {
				printf("include file=%s\n", file);
			}

			/* get ride of prefix for included file */
			if (strstr(file, incprefix)) {
				if ((s = strtok(file, "/"))) {
					sprintf(incfile, "%s", strtok(NULL, "/"));
					strcpy(file, incfile);
				}
			}
			push(file, 1);
		}

		/* recognize comments */
		else if (line[0] == '#') {
			if (strstr(line, "#define") != NULL ||
			    strstr(line, "#ifdef") != NULL ||
			    strstr(line, "#ifndef") != NULL ||
			    strstr(line, "#if") != NULL ||
			    strstr(line, "#endif") != NULL) {
				stack[stacktop].tunes = 1;
				fprintf(ofp, "%s\n", line);
			}
			else
				continue;
		}
		/* skip blank lines */
		else if (line[0] == '\0')
			continue;
		/* skip x=y */
		else if (strstr(line, "=") != NULL)
			continue;
		/* anything else is an error */
		else {
			fprintf(stderr, "%s: \"%s\": invalid line\n", stack[stacktop].path, line);
			exit(1);
		}
	}
}

void
push(char *file, char include)
{
	char path[PATHL];
	FILE *fp;

	if (stacktop >= STACKSZ) {
		fprintf(stderr, "%s: too many nested include's\n", file);
		exit(1);
	}

	/* add idir for included files only, keep the original path of the first file */
	if (include == 1)
		sprintf(path, "%s/%s", idir, file);
	else
		sprintf(path, "%s", file);

	if ((fp = fopen(path, "r")) == NULL) {
		fprintf(stderr, "open %s error", stack[stacktop].path);
		syserr(file);
	}

	if (DEBUG == 1)
		printf("open file %s successfully\n", path);

	stacktop++;
	strcpy(stack[stacktop].path, path);
	stack[stacktop].fp = fp;
	stack[stacktop].tunes = 0;
}

void
pop()
{
	int i;
	fclose(stack[stacktop].fp);

	stack[stacktop].fp = NULL;
	for (i = 0; i < PATHL; i++)
		stack[stacktop].path[i] = 0;
	stack[stacktop].tunes = 0;
	if (stacktop-- <= 0)
		exit(0);
}

void
usage(char *s)
{
	fprintf(stderr, "usage: %s -I dir configfile outputfile\n", s);
	exit(1);
}

void
syserr(char *s)
{
	/* skip for windows perror(s); */
	exit(1);
}
