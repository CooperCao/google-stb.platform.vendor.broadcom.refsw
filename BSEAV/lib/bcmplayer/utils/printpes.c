/***************************************************************************
 *     Copyright (c) 2002-2012, Broadcom Corporation
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
 * Module Description: print out contents of PES stream
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
#include <stdbool.h>
#include <stdint.h>
#include "ts_utils.h"

/* decode and print the pes header. buf points to the stream_id and is guaranteed
to point to whole pes header. */
static void print_pes_header(unsigned char *buf, unsigned *length)
{
    b_pes_header pes_header;

    if (b_get_pes_header(buf, &pes_header)) {
        printf("### invalid pes header\n");
        return;
    }

    printf("  packet_length %d\n", pes_header.packet_length);
    *length = pes_header.packet_length;
    if (pes_header.pes_type == b_pes_packet_type_pes) {
        printf("  header_data_length %d\n", pes_header.header_data_length);
        if (pes_header.pts_dts_flags == 2) {
            printf("  pts 0x%08x\n", pes_header.pts);
        }
        else if (pes_header.pts_dts_flags == 3) {
            printf("  pts 0x%08x\n", pes_header.pts);
            printf("  dts 0x%08x\n", pes_header.dts);
        }
        else if (pes_header.pts_dts_flags == 0) {
            /* no pts or dts */
        }
        else {
            printf("### forbidden pts_dts_flags value\n");
            *length = 0;
        }

        /* TODO: ESCR flag, ES rate flag, DSM trick mode flag, etc. */
    }
    else if (pes_header.pes_type == b_pes_packet_type_data) {
        printf("  data\n");
    }
    else if (pes_header.pes_type == b_pes_packet_type_padding) {
        /* padding stream */
        printf("  padding\n");
    }
    else {
        /* not a PES stream_id */
    }
}

int main(int argc, char **argv) {
    FILE *fin = stdin;
#define BUFSIZE 4096
    unsigned char buf[BUFSIZE];
    int sccount = 0;
    int curarg = 1;
    uint64_t fileoffset = 0;
    unsigned bounded_pes_length = 0;

    if (argc > 1 && !strcmp(argv[curarg], "--help")) {
        printf("Usage: printpes [FILENAME]\n");
        exit(0);
    }

    if (curarg < argc) {
        fin = fopen(argv[curarg], "r");
        if (!fin)
            return printf("Unable to open %s: %d\n", argv[curarg], errno);
    }
    else {
        printf("Reading stdin\n");
    }

    while (!feof(fin)) {
        int i;
        int n;

        /* printf("read at %#x\n", (unsigned)fileoffset); */
        n = fread(buf, 1, BUFSIZE, fin);
        if (n == -1) goto file_error;

        /* search for start codes */
        for (i=0; i<n; i++) {
            if (sccount == 3) {
                if (b_is_pes_stream_id(buf[i])) {
/* TODO: don't know actual max */
#define MAX_HEADER_SIZE 100
                    /* check if we have enough to process the header */
                    if (n - i < MAX_HEADER_SIZE) {
                        int rc;
                        /* move the remainder of the old read to the head of the buffer */
                        memcpy(buf, &buf[i], n-i);
                        fileoffset += i;
                        n -= i;
                        i = 0;

                        /* read more */
                        rc = fread(&buf[n], 1, BUFSIZE-n, fin);
                        if (rc == -1) goto file_error;
                        n += rc;
                        if (n < MAX_HEADER_SIZE)
                            break;
                    }
                    printf("stream_id %02x at %#lx\n", buf[i], (unsigned long)(fileoffset+i));
                    print_pes_header(&buf[i], &bounded_pes_length);
                    if (bounded_pes_length) {
                        /* printf("bounded %d, %#x, %#x\n", bounded_pes_length, i, n); */
                        /* move i to the start of the data counted by bounded_pes_length */
                        i += 2;
                        if (i + bounded_pes_length >= (unsigned)n) {
                            /* consume through the bounded pes packet */
                            unsigned consumed = n; /* the whole buffer has been consumed */
                            bounded_pes_length -= (n-i); /* remainder in the buffer */
                            while (bounded_pes_length) {
                                n = (bounded_pes_length > BUFSIZE) ? BUFSIZE : bounded_pes_length;
                                n = fread(buf, 1, n, fin);
                                if (n == -1) goto file_error;
                                if (n == 0) break;
                                
                                bounded_pes_length -= n;
                                consumed += n;
                            }
                            n = consumed; /* set n so fileoffset is incremented correctly */
                            i = n; /* exit for loop */
                            sccount = 0;
                            /* printf("next buffer read at %#x+%d=%#x\n", (unsigned)fileoffset, n, (unsigned)fileoffset+n); */
                            continue;
                        }
                        else {
                            i += bounded_pes_length;
                            bounded_pes_length = 0; 
                        }
                    }
                }
                sccount = 0;
            }

            BDBG_ASSERT(i < n);
            sccount = b_check_for_start_code(buf[i], sccount);
        }
        fileoffset += n;
    }
    return 0;
    
file_error:
    fprintf(stderr, "file error %d\n", errno);
    return -1;
}
