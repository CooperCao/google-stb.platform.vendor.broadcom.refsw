/*
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
#include <string.h>
#include <stdlib.h>

#define	DO_B4	1
#define	DO_B8	2
#define	DO_W2	3

char *optstr[] = {
	"Error",
	"b4",
	"b8",
	"w2"
};

void
usage(void)
{
	printf("USAGE: swap [-b4 | -b8 | -w | -w2] <filename>\n");
	printf("	No option or -b4:	Byte swap each set of 4 bytes\n");
	printf("	-b8:			Byte swap each set of 8 bytes\n");
	printf("	-w or -w2:		Word (32bit) swap each set of 2 words\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	FILE    *inFile, *outFile;
	char    *fname;
	int size = 0;
	char    ofname[256];
	int     i = 1, count, rc, opt = DO_B4;
	char    *ptr;
	void	*fromaddr = NULL, *toaddr;
	unsigned char *from, *to;
	unsigned char tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	unsigned long *wfrom, *wto;
	unsigned long wtmp0, wtmp1;

	/* Check usage.  Must supply an input filename */
	if (argc == 3) {
		if (strncmp(argv[1], "-b4", 4) == 0)
			opt = DO_B4;
		else if (strncmp(argv[1], "-b8", 4) == 0)
			opt = DO_B8;
		else if ((strncmp(argv[1], "-w", 3) == 0) ||
		         (strncmp(argv[1], "-w2", 4) == 0))
			opt = DO_W2;
		else
			usage();
		i = 2;
	} else if (argc != 2) {
		usage();
	}

	/* Make sure we can open the file specified.  If not, error out */
	fname = argv[i];
	inFile = fopen(fname, "rb");
	if (inFile == NULL) {
		printf("ERROR!  %s not found!\n", fname);
		exit(2);
	}

	/* Get number of bytes in the file */
	fseek(inFile, 0, SEEK_END);
	size = ftell(inFile);

	/* Print out number of bytes and reset file pointer */
	printf("File size:\t%d bytes.\n", size);
	fseek(inFile, 0, SEEK_SET);

	/* Create filename for output file.  It will basically have a _est */
	/* before the '.', if there is one.  For example, test will output */
	/* test_est and test.bin will output test_est.bin */
	ptr = strchr(fname, '.');
	if (ptr != NULL)
		*ptr = '\0';
	strcpy(ofname, fname);
	strcat(ofname, "_swap");
	if (opt != DO_B4)
		strcat(ofname, optstr[opt]);
	if (ptr != NULL) {
		strcat(ofname, ".");
		ptr++;
		strcat(ofname, ptr);
	}

	/* Open handle to new output file.  Return if an error */
	outFile = fopen(ofname, "wb+");
	if (outFile == NULL) {
		printf("Error!  Cannot create output file!\n");
		exit(3);
	}

	fromaddr = (void *)malloc(size + 8);
	from = (unsigned char *)fromaddr;
	wfrom = (unsigned long *)fromaddr;
	toaddr = (void *)malloc(size + 8);
	to = (unsigned char *)toaddr;
	wto = (unsigned long *)toaddr;

	if ((rc = fread (fromaddr, 1, size, inFile)) != size) {
		fprintf(stderr, "Could not read entire source file.\n");
		exit(9);
	}

	count = 0;
	while (count < size) {
		switch (opt) {
		case DO_B4:
			tmp0 = *from++;
			tmp1 = *from++;
			tmp2 = *from++;
			tmp3 = *from++;
			*to++ = tmp3;
			*to++ = tmp2;
			*to++ = tmp1;
			*to++ = tmp0;
			count += 4;
			break;
		case DO_B8:
			tmp0 = *from++;
			tmp1 = *from++;
			tmp2 = *from++;
			tmp3 = *from++;
			tmp4 = *from++;
			tmp5 = *from++;
			tmp6 = *from++;
			tmp7 = *from++;
			*to++ = tmp7;
			*to++ = tmp6;
			*to++ = tmp5;
			*to++ = tmp4;
			*to++ = tmp3;
			*to++ = tmp2;
			*to++ = tmp1;
			*to++ = tmp0;
			count += 8;
			break;
		case DO_W2:
			wtmp0 = *wfrom++;
			wtmp1 = *wfrom++;
			*wto++ = wtmp1;
			*wto++ = wtmp0;
			count += 8;
			break;
		default:
			fprintf(stderr, "Internal error: bad opt value (%d).\n", opt);
			exit(10);
		}

	}

	fwrite(toaddr, 1, size, outFile);

	/* Close files and indicate success */
	fclose(inFile);
	fclose(outFile);
	printf("Output file successfully created.\n");

	return 0;
}
