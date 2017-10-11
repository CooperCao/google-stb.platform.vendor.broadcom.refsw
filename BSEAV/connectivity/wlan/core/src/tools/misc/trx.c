/*
 * TRX image file creation utility
 *
 * This utility concatenates the files listed on the command line
 * (with optional padding between them) and appends a TRX header to
 * the result.
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
#include <ctype.h>
#include <errno.h>

#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <trxhdr.h>


static void
usage(void)
{
	fprintf(stderr,
	        "Usage 1: trx [options] [-b offset] [file] [-b offset] [-x start] [file] ...\n");
	fprintf(stderr,
	        "Usage 2: trx -d file.trx\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "	-h, --help	This message\n");
	fprintf(stderr, "	-o, --output	Output stream (default stdout)\n");
	fprintf(stderr, "	-f, --flag	Flags (noheader)\n");
	fprintf(stderr, "	-b, --offset	Offset of file within output stream or\n"
			"			0 for immediately after the previous file\n");
	fprintf(stderr, "	-x, --start	Start of binary (used by USB RDL code)\n");
	fprintf(stderr, "	-d, --dump	Dump contents of existing trx file\n");
}

static void
error(char *msg)
{
	if (errno)
		fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	else {
		fprintf(stderr, "%s\n", msg);
		usage();
	}
	exit(errno ? errno : 1);
}

int
main(int argc, char **argv)
{
	FILE *in = NULL, *out = stdout, *tmp = NULL;
	struct trx_header hdr;
	unsigned long offset = 0;
	int c, n = 0;
	char tmp_path[] = "trx.XXXXXX";

	/* Initialize header */
	memset(&hdr, 0, sizeof(hdr));
	hdr.magic = TRX_MAGIC;
	hdr.flag_version = TRX_VERSION << 16;
	hdr.len = sizeof(struct trx_header);

	/* Skip program name */
	--argc;
	++argv;

	/* Parse command line arguments */
	for (; *argv; argv++) {
		if (strcmp(*argv, "-h") == 0 ||
		    strcmp(*argv, "--help") == 0) {
			usage();
			exit(0);
		} else if (strcmp(*argv, "-d") == 0 ||
		         strcmp(*argv, "--dump") == 0) {
			if (!*++argv)
				error("-d: missing argument");
			if (!(in = fopen(*argv, "rb")))
				error(*argv);
			if (fread(&hdr, 1, sizeof(hdr), in) == sizeof(hdr)) {
				long flen;

				printf("magic:\t\t0x%08x\t%s\n", hdr.magic,
				       hdr.magic == TRX_MAGIC ? "" : "Bad!");
				if (fseek(in, 0, SEEK_END))
					error(*argv);
				printf("len:\t\t%10d", hdr.len);
				if (hdr.len != (flen = ftell(in)))
					printf("\tBad! file len: %lu", flen);
				printf("\nflags:\t\t    0x%04x\n", hdr.flag_version & 0xffff);
				printf("version:\t%10d\n", hdr.flag_version >> 16);
				for (n = 0; n < TRX_MAX_OFFSET; n++)
					printf("offsets[%d]:\t0x%08x\n", n, hdr.offsets[n]);
			} else
				fprintf(stderr, "Cannot read trx header from file %s\n", *argv);
			fclose(in);
			exit(0);
		} else if (strcmp(*argv, "-o") == 0 ||
		         strcmp(*argv, "--output") == 0) {
			if (!*++argv)
				error("-o: missing argument");
			if (!(out = fopen(*argv, "wb")))
				error(*argv);
		} else if (strcmp(*argv, "-b") == 0 ||
		         strcmp(*argv, "--offset") == 0) {
			if (!*++argv)
				error("-b: missing argument");
			switch ((*argv)[strlen(*argv) - 1]) {
				case 'k': case 'K': c = 1024; break;
				case 'm': case 'M': c = 1024*1024; break;
				default: c = 1; break;
			}
			offset = strtoul(*argv, NULL, 0) * c;
		} else if (strcmp(*argv, "-x") == 0 ||
		         strcmp(*argv, "--start") == 0) {
			if (!*++argv)
				error("-x: missing argument");
			hdr.offsets[n++] = strtoul(*argv, NULL, 0);
		} else if (strcmp(*argv, "-f") == 0 ||
		         strcmp(*argv, "--flag") == 0) {
			if (!*++argv)
				error("-f: missing argument");
			if (strcmp(*argv, "noheader") == 0 || strcmp(*argv, "no_header") == 0 ||
			    strcmp(*argv, "nohdr") == 0 || strcmp(*argv, "no_hdr") == 0)
				hdr.flag_version |= TRX_NO_HEADER;
			else
				hdr.flag_version |= strtoul(*argv, NULL, 0) & 0xffff;
		} else {
			if (!(in = fopen(*argv, "rb")))
				error(*argv);

			/* Create temp file */
			if (!tmp) {
#ifdef linux
				int tmp_fd;
				if ((tmp_fd = mkstemp(tmp_path)) < 0 ||
				    !(tmp = fdopen(tmp_fd, "wb+")))
					error(tmp_path);
#else
				if (!mktemp(tmp_path) ||
				    !(tmp = fopen(tmp_path, "wb+")))
					error(tmp_path);
#endif
			}

			/* Pad to offset */
			if (!offset)
				offset = hdr.len;
			if (offset < hdr.len) {
				fprintf(stderr, "Error: increase offset %ld to %d\n",
				        offset, hdr.len);
				goto err;
			}
			if (offset % 4) {
				fprintf(stderr, "warning: increasing offset %ld to %ld\n",
				        offset, ROUNDUP(offset, 4));
				offset = ROUNDUP(offset, 4);
			}
			for (c = 0; hdr.len < offset; hdr.len++)
				putc(c, tmp);

			/* Store initial offsets in header */
			if (n < ARRAYSIZE(hdr.offsets))
				hdr.offsets[n++] = offset;

			/* Append file */
			while ((c = getc(in)) != EOF) {
				putc(c, tmp);
				hdr.len++;
			}
			offset = hdr.len;

			fclose(in);

		}
	}

	if (!tmp)
		error("Error: no input files");

	for (c = 0; hdr.len % 4096; hdr.len++)
		putc(c, tmp);

	/* Calculate CRC over header */
	hdr.crc32 = hndcrc32((uint8 *) &hdr.flag_version,
	                     sizeof(struct trx_header) - OFFSETOF(struct trx_header, flag_version),
	                     CRC32_INIT_VALUE);

	/* Calculate CRC over data */
	fseek(tmp, 0, SEEK_SET);
	while ((c = getc(tmp)) != EOF)
		hdr.crc32 = hndcrc32((uint8 *) &c, 1, hdr.crc32);

	/* Write output */
	fwrite(&hdr, sizeof(struct trx_header), 1, out);
	fseek(tmp, 0, SEEK_SET);
	while ((c = getc(tmp)) != EOF)
		putc(c, out);

err:
	fclose(tmp);
	fclose(out);
	remove(tmp_path);
	return 0;
}
