/***************************************************************************
 *     Copyright (c) 2006, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: remove start code emulation prevention from ES stream
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

int main(int argc, char **argv) {
	int fileoffset = 0;
#define BUFSIZE 4096
	unsigned char buf[BUFSIZE];
	int zero_count = 0;
	int curarg = 1;
	int possible_emulation = 0;

	if (curarg < argc && !strcmp(argv[curarg], "--help")) {
		fprintf(stderr, "Usage: rmscprevent <infile >outfile\n"
		"  Removes start code emulation prevention from ES streams. Uses stdin/stdout.\n");
		exit(0);
	}

	while (!feof(stdin)) {
		int i, n;
		
		n = fread(&buf[possible_emulation], 1, BUFSIZE-possible_emulation, stdin);
		if (n <= 0) break;
			
		for (i=possible_emulation; i<n; i++) {
			if (possible_emulation) {
				if (buf[i] <= 3) {
					/* remove 0x03 */
					assert(buf[i-1] == 3);
					memmove(&buf[i-1], &buf[i], n-i);
					i--;
					n--;
					possible_emulation = 0;
					continue;
				}
			}
			
			possible_emulation = 0;
			
			switch (buf[i]) {
			case 0:
				if (zero_count >= 1)
					zero_count = 2;
				else
					zero_count = 1;
				break;
			case 3:
				if (zero_count == 2) {
					possible_emulation = 1;
				}
				/* fall through */
			default:
				zero_count = 0;
				break;
			}
		}
		fileoffset += n;

		/* don't write last byte of possible emulation byte */		
		if (possible_emulation) {
			n--;
		}
		
		/* write the result out */
		fwrite(buf, 1, n, stdout);
		
		if (possible_emulation) {
			/* move the 0x03 from the last byte to the first of the next buffer */
			buf[0] = buf[n];
			assert(buf[0] == 3);
		}
	}
	return 0;
}
