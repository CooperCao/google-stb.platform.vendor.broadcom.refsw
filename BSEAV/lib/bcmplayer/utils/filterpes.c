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

BDBG_MODULE(filterpes);

/* TODO:
filter for stream ID
honor bounded PES size
follow filterts cmdline convention if possible
*/
static void print_usage(void)
{
    fprintf(stderr,
    "Usage: filterpes [-s]\n"
    "  stdin is MPEG2PES input. stdout is either filtered MPEG2PES or ES.\n"
    );
}

int main(int argc, char **argv) {
#define BUFSIZE 4096
    unsigned char buf[BUFSIZE];
    unsigned rptr = 0, wptr = 0, sccount = 0;
    int curarg = 1;
    uint64_t stream_offset = 0;
    bool strip = false;
    unsigned consume;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            exit(0);
        }
        else if (!strcmp(argv[curarg], "-s")) {
            strip = true;
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    if (!strip) {
        fprintf(stderr, "Only feature currently supported is -s\n");
        return -1;
    }

    while (!feof(stdin)) {
        int n;
        unsigned i;

read_again:
        n = fread(&buf[wptr], 1, BUFSIZE-wptr, stdin);
        if (n < 0) goto file_error;
        wptr += n;
        BDBG_MSG(("read %d bytes: %d %d", n, rptr, wptr));

        /* search for start codes */
        for (i=rptr+sccount; i<wptr; i++) {
            if (sccount == 3) {
                if (b_is_pes_stream_id(buf[i])) {
                    b_pes_header pes_header;

                    consume = (i-sccount)-rptr;
                    if (consume) {
                        fwrite(&buf[rptr], 1, consume, stdout);
                        BDBG_MSG(("write %d bytes: %d %d", consume, rptr, wptr));
                        stream_offset += consume;
                        rptr += consume;
                    }

/* TODO: don't know actual max */
#define MAX_HEADER_SIZE 100
                    /* check if we have enough to process the header */
                    if (wptr - i < MAX_HEADER_SIZE) {
                        if (wptr < BUFSIZE) break;
                        memmove(buf, &buf[rptr], wptr-rptr);
                        wptr -= rptr;
                        rptr = 0;
                        goto read_again;
                    }
                    BDBG_MSG(("stream_id %02x at %#lx", buf[i], (unsigned long)(stream_offset+(i-rptr))));

                    if (b_get_pes_header(&buf[rptr], &pes_header)) {
                        fprintf(stderr, "### invalid pes header\n");
                        return -1;
                    }

                    if (strip) {
                        /* skip over pes header */
                        rptr += 5 + pes_header.header_data_length;
                        BDBG_MSG(("skip %d bytes: %d %d", 5 + pes_header.header_data_length, rptr, wptr));
                    }
                }
                sccount = 0;
            }

            BDBG_ASSERT(i <= wptr);
            if (i != wptr) {
                sccount = b_check_for_start_code(buf[i], sccount);
            }
        }

        /* print everything before the possible start code */
        BDBG_ASSERT(wptr-rptr >= sccount);
        consume = (wptr-rptr)-sccount;
        if (consume) {
            fwrite(&buf[rptr], 1, consume, stdout);
            BDBG_MSG(("write %d bytes, to end: %d %d", consume, rptr, wptr));
            memmove(buf, &buf[wptr-sccount], sccount);
            stream_offset += consume;
            wptr = sccount;
            rptr = 0;
        }
    }

    /* could have dangling start code */
    consume = wptr-rptr;
    if (consume) {
        fwrite(&buf[rptr], 1, consume, stdout);
        stream_offset += consume;
    }

    return 0;

file_error:
    fprintf(stderr, "file error %d", errno);
    return -1;
}
