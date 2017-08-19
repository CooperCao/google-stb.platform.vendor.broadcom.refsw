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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma pack(1)
typedef struct _EST_HDR
{
	char	Sig[8];
	int	SAddr;
	int	LAddr;
	char	pad;
	char	resvd[7];
	int	ASaddr;
	int	ALaddr;
} EST_HDR, *PEST_HDR;
#pragma pack()

#define SIGNATURE	"ESTFBINR"
#define PADTO		(128 * 1024)

int
main(int argc, char *argv[])
{
	FILE	*InFile, *OutFile;
	char	*fname;
	unsigned int Size = 0, End;
	char	ofname[256];
	char	c;
	EST_HDR	Header;
	int	Count;
	char	*ptr;

	/* Check usage.  Must supply an input filename */
	if (argc != 2) {
		printf("USAGE: addhdr <filename>\n");
		exit(1);
	}

	/* Make sure we can open the file specified.  If not, error out */
	fname = argv[1];
	InFile = fopen(fname, "rb");
	if (InFile == NULL) {
		printf("ERROR!  %s not found!\n", fname);
		exit(2);
	}

	/* Get number of bytes in the file */
	fseek(InFile, 0, SEEK_END);
	Size = (ftell(InFile) + (PADTO-1)) & ~(PADTO-1);

	/* Print out number of bytes and reset file pointer */
	printf("File size:\t%d bytes.\n", Size);
	fseek(InFile, 0, SEEK_SET);

	/* Create filename for output file.  It will basically have a _est */
	/* before the '.', if there is one.  For example, test will output */
	/* test_est and test.bin will output test_est.bin */
	ptr = strchr(fname, '.');
	if (ptr == NULL) {
		strcpy(ofname, fname);
		strcat(ofname, "_est");
	} else {
		*ptr = 0;
		strcpy(ofname, fname);
		strcat(ofname, "_est.");
		ptr++;
		strcat(ofname, ptr);
	}

	/* Open handle to new output file.  Return if an error */
	OutFile = fopen(ofname, "wb");
	if (OutFile == NULL) {
		printf("Error!  Cannot create output file!\n");
		exit(3);
	}

	/* Fill in header structure */
	strcpy(Header.Sig, SIGNATURE);
	Header.SAddr = 0xc01f;
	End = (Size - 1);
	End += 0x1fc00000;
	Header.LAddr = (End & 0xFF);
	Header.LAddr <<= 8;
	Header.LAddr |= ((End >> 8) & 0xFF);
	Header.LAddr <<= 8;
	Header.LAddr |= ((End >> 16) & 0xFF);
	Header.LAddr <<= 8;
	Header.LAddr |= (char)(End >> 24);
	Header.pad = (char)0xff;
	memset((void *)Header.resvd, 0, 7);
	Header.ASaddr = Header.SAddr;
	Header.ALaddr = Header.LAddr;

	/* Copy header to new output file */
	ptr = (char *)&Header;
	for (Count = 0; Count < sizeof(EST_HDR); Count++) {
		fputc(ptr[Count], OutFile);
	}

	/* Copy data from InFile to new output file */
	while (Size) {
		c = fgetc(InFile);
		fputc(c, OutFile);
		Size--;
	}

	/* Close files and indicate success */
	fclose(InFile);
	fclose(OutFile);
	printf("Output file successfully created.\n");

	exit(0);
}
