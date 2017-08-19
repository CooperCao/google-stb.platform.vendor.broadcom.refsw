/*
 * Take a gzip'd initrd and prepend the header.
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libiberty.h>
#include <bcmendian.h>


#define	BSIZE	(16 * 1024)

int
main(int argc, char **argv, char **envp)
{
	int		optind, bigendian;
	unsigned long	size, chunk;
	void		*buf = NULL;
	unsigned long	*lp;
	FILE		*ifd, *ofd;
	char		*fname, *ofname;

	if ((argc < 2) || (argc > 4)) {
		fprintf(stderr, "Usage: mkinrd [-b] fname [ofname]\n");
		exit(1);
	}

	if ((argv[1][0] == '-') && (argv[1][1] == 'b')) {
		argc--;
		optind = 2;
		bigendian = 1;
	} else {
		optind = 1;
		bigendian = 0;
	}

	fname = argv[optind];
	if ((ifd = fopen(fname, "rb")) == NULL) {
		fprintf(stderr, "Cannot open \"%s\"\n", fname);
		perror("\terror");
		exit(2);
	}

	if (argc == 3) {
		ofname = argv[optind + 1];
		if ((ofd = fopen(ofname, "wb")) == NULL) {
			fprintf(stderr, "Cannot open \"%s\"\n", ofname);
			perror("\terror");
			exit(3);
		}
	} else {
		ofd = stdout;
	}

	if (fseek(ifd, 0, SEEK_END) != 0) {
		fprintf(stderr, "Cannot seek to end of file \"%s\"\n", fname);
		perror("\terror");
		goto cleanup;
	}
	if ((size = ftell(ifd)) == -1) {
		fprintf(stderr, "Cannot tell size of file \"%s\"\n", fname);
		perror("\terror");
		goto cleanup;
	}
	if (fseek(ifd, 0, SEEK_SET) != 0) {
		fprintf(stderr, "Cannot seek to beginning of file \"%s\"\n", fname);
		perror("\terror");
		goto cleanup;
	}

	buf = (void *)xmalloc(BSIZE);

	lp = (unsigned long *)buf;
	if (bigendian) {
		/* Hack alert: assumes little endian host */
		lp[0] = 0x44524e49;
		lp[1] = htol32(size);
	} else {
		lp[0] = 0x494e5244;
		lp[1] = size;
	}
	chunk = (size < (BSIZE - 8)) ? size : BSIZE - 8;

	if (fread(&lp[2], 1, chunk, ifd) != chunk) {
		perror("cannot read input file");
		goto cleanup;
	}
	chunk += 8;
	size += 8;

	do {
		if (fwrite(buf, 1, chunk, ofd) != chunk) {
			perror("cannot write output file");
			goto cleanup;
		}
		size -= chunk;
		chunk = (size < BSIZE) ? size : BSIZE;
		if (fread(buf, 1, chunk, ifd) != chunk) {
			perror("cannot read input file");
			goto cleanup;
		}
	} while (size > 0);
cleanup:
	free(buf);
	fclose(ifd);
	fclose(ofd);
	return (0);
}
