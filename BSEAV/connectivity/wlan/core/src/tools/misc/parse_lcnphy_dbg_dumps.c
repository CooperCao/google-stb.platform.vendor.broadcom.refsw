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
#include <stdlib.h>
#include <string.h>
#include <math.h>

void
usage(const char * prog_name)
{
	fprintf(stderr,
	        "usage: %s inputfname outputfname data_type(s/u) startbit1 "
	        "length1 [startbit2 length2 ..]\n",
	        prog_name);
}

int main(int argc, char **argv)
{

	FILE *fp1, *fp2;
	int  i, read_data, parsed_data, parsed_data1;
	char datatype, ipfname[50], opfname[50];
	int  count, startbit[2], length[2], mask[2], numfields = 0, negbound;

	if (argc < 6) {
		usage(argv[0]);
		exit(1);
	}

	if (argc > 8) {
		fprintf(stderr, "Currently supports combining of only two fields ..\n");
		usage(argv[0]);
		exit(1);
	}


	i = 1;
	while (i < argc) {
		switch (i) {
		case 1:
			strcpy(ipfname, argv[i]);
			i++;
			break;
		case 2:
			strcpy(opfname, argv[i]);
			i++;
			break;
		case 3:
			datatype = *argv[i];
			i++;
			break;
		case 4:
			startbit[0] = atoi(argv[i]);
			i++;
			break;
		case 5:
			length[0] = atoi(argv[i]);
			negbound = (int)pow(2.0, (double)(length[0]-1));
			mask[0] = (int)pow(2.0, (double)length[0]) - 1;
			mask[0] = mask[0] << startbit[0];
			numfields++;
			i++;
			break;
		case 6:
			startbit[1] = atoi(argv[i]);
			i++;
			break;
		case 7:
			length[1] = atoi(argv[i]);
			negbound = (int)pow(2.0, (double)(length[0]+length[1]-1));
			mask[1] = (int)pow(2.0, (double)length[1]) - 1;
			mask[1] = mask[1] << startbit[1];
			numfields++;
			i++;
			break;
		}
	}

	fp1 = fopen(ipfname, "r");
	if ((fp1 = fopen(ipfname, "r")) == NULL) {
		fprintf(stderr, "ERROR opening file %s\n", ipfname);
		exit(1);
	}
	if ((fp2 = fopen(opfname, "w")) == NULL) {
		fprintf(stderr, "ERROR opening file %s\n", opfname);
		exit(1);
	}

	count = fscanf(fp1, "%x", &read_data);

	while (count != -1) {
		parsed_data = read_data & mask[0];
		parsed_data = parsed_data >> startbit[0];
		if (numfields > 1) {
			parsed_data1 = read_data & mask[1];
			parsed_data1 = parsed_data1 >> startbit[1];
			parsed_data = (parsed_data1 << length[0]) | parsed_data;
		}
		if (datatype == 's')
		{
			if (parsed_data >= negbound)
				parsed_data = parsed_data - (2*negbound);
		}
		fprintf(fp2, "%d\n", parsed_data);
		count = fscanf(fp1, "%x", &read_data);
	}

	fclose(fp1);
	fclose(fp2);

	return 0;
}
