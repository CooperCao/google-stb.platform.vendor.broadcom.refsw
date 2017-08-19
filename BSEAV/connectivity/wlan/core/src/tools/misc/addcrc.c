/*
 * File CRC utility - compute CRC and add to file
 *
 * Copyright (C) 2008 Broadcom Corporation
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <typedefs.h>
#include <bcmutils.h>

#define BUFSIZE 4096
static uint8 buffer[BUFSIZE];

int
main(int argc, char **argv)
{
	char *progname;
	uint32 crcval = CRC32_INIT_VALUE;
	uint32 count;
	size_t len;


	FILE *file;
	char *filename;
	bool nowrite = FALSE;

	progname = *argv++;

	if (*argv != NULL && strcmp(*argv, "-n") == 0) {
		nowrite = TRUE;
		argv++;
	}

	filename = *argv++;

	if (filename == NULL || *argv != NULL)
		goto usage;

	file = fopen(filename, "rb");
	if (file == NULL) {
		perror(filename);
		exit(-2);
	}

	for (count = 0; !feof(file); count += len) {
		len = fread(buffer, sizeof(uint8), BUFSIZE, file);
		if (len != BUFSIZE && ferror(file)) {
			perror(filename);
			exit(-2);
		}

		crcval = hndcrc32(buffer, len, crcval);
	}

	if (fclose(file)) {
		perror(filename);
		exit(-2);
	}

	printf("CRC32 over %d bytes: 0x%08x\n", count, crcval);

	if (!nowrite) {
		uint32 notcrc = ~crcval;

		file = fopen(filename, "ab");
		if (file == NULL) {
			perror(filename);
			exit(-3);
		}

		len = fwrite(&notcrc, sizeof(uint8), sizeof(notcrc), file);
		if (ferror(file)) {
			perror(filename);
			exit(-3);
		}

		if (fclose(file)) {
			perror(filename);
			exit(-3);
		}
	}

	exit(0);

usage:
	fprintf(stderr, "Usage: %s [-n] <file>\n", progname);
	exit(-1);
}
