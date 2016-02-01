/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
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
 * Module Description: 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ts_utils.h"
#include <unistd.h>

#define MAXPIDS 100
#define BUFSIZE (1024*32)
#define MAX_PACKET_PAYLOAD 184		

struct pidstruct {
	FILE *file;
	unsigned short pid;
	unsigned char buf[BUFSIZE];
	int bufptr, bufsize;
	int continuity_counter;
} pids[MAXPIDS];
int totalpids = 0;

/* TODO: just about everything
1. A/V sync (we can make gross assumptions about the files, but some sync should be done)
2. PES layer and PCR's
3. constant bit rate (padding the result with NULL's)
*/

void printUsage(void)
{
	fprintf(stderr,
	"Usage: muxts FILE PID [FILE PID ...]\n"
	"  Transport output is written to stdout.\n"
	);
}

unsigned char pkt[188];

int write_packet(struct pidstruct *pid)
{
	int size = pid->bufsize - pid->bufptr; /* max size that could be written */
	int headersize = 4;
	int payload;
	
	pkt[0] = 0x47;   /* SYNC BYTE */
	pkt[1] = (pid->pid >> 8) & 0x1f;
	pkt[2] = pid->pid & 0xff;  /* PID */
	pkt[3] = 0x10 | pid->continuity_counter; /* not scrambled, payload only, continuity counter */
	if (++pid->continuity_counter > 0xF)
		pid->continuity_counter = 0;

	payload = 188 - headersize;
	
	if (size < payload) {
		/* pad with adaptation field, should only happen once at end */
		pkt[3] |= 0x20;
		pkt[4] = payload - size - 1;
		headersize = 188 - size;
		payload = size;
		/* TODO: set for private data */
	}
	else if (size > 188 - headersize) {
		size = 188 - headersize;
	}
		
	fwrite(pkt, headersize, 1, stdout);
	fwrite(&pid->buf[pid->bufptr], size, 1, stdout);
	
	return size;
}

int main(int argc, char **argv)
{
	int curarg = 1;
	
	while (curarg < argc-1) { /* must have two more params */
		const char *filename = argv[curarg];
		pids[totalpids].file = fopen(filename, "r");
		if (!pids[totalpids].file) {
			fprintf(stderr, "ERROR: Unable to open %s\n", filename);
			exit(1);
		}
		curarg++;
		pids[totalpids].pid = strtoul(argv[curarg], NULL, 0);
		pids[totalpids].bufptr = pids[totalpids].bufsize = 0;
		totalpids++;
	}
	if (!totalpids) {
		printUsage();
		exit(1);
	}
	
	while (1) {
		int i;
		
		/* read data */
		int more = 0;
		for (i=0;i<totalpids;i++) {
			int dataleft = pids[i].bufsize-pids[i].bufptr;
			/* always make sure there's one packet's worth of data available */
			if (dataleft < MAX_PACKET_PAYLOAD && pids[i].file) {
				int n;
				
				/* move remainder to head of buffer */
				if (dataleft) {
					memmove(pids[i].buf, &pids[i].buf[pids[i].bufptr], dataleft);
					pids[i].bufptr = dataleft;
				}
				else {
					pids[i].bufptr = 0;
				}
				pids[i].bufsize = pids[i].bufptr;
				
				n = fread(&pids[i].buf[pids[i].bufptr], 1, BUFSIZE - pids[i].bufptr, pids[i].file);
				if (n < 0) {
					fprintf(stderr, "ERROR: unable to read file\n");
					exit(1);
				}
				if (n == 0) {
					/* done with file */
					fclose(pids[i].file);
					pids[i].file = NULL;
				}
				else {
					pids[i].bufsize += n;
				}
			}
			
			if (pids[i].bufsize)
				more = 1;
		}
		if (!more) break;


		/* now output data */
		for (i=0;i<totalpids;i++) {
			if (pids[i].bufsize) {
				int n = write_packet(&pids[i]);
				pids[i].bufptr += n;
				if (pids[i].bufptr == pids[i].bufsize) {
					pids[i].bufptr = pids[i].bufsize = 0;
				}
			}
		}
	}	
	
	return 0;
}

