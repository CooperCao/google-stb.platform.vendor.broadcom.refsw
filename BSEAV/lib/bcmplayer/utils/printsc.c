/***************************************************************************
 *     Copyright (c) 2002-2009, Broadcom Corporation
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
#include <errno.h>
#include "ts_utils.h"

int main(int argc, char **argv) {
    FILE *fin = stdin;
    int fileoffset = 0;
#define BUFSIZE 4096
    unsigned char buf[BUFSIZE];
    int sccount = 0;
    int curarg = 1;
    unsigned payload = 0;
    int num_startcodes = 0;

    if (curarg < argc && !strcmp(argv[curarg], "--help")) {
        printf("Usage: printsc [FILENAME]\n");
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
        int i, n;

        n = fread(buf, 1, BUFSIZE, fin);
        if (n <= 0) break;

        for (i=0; i<n; i++) {
            payload++;
            if (sccount == 3) {
                if (num_startcodes > 0) {
                    payload -= 4; /* subtract for 00 00 01 SC */
                    printf("  payload %d\n", payload); /* from previous sc */
                }
                printf("SC %02x at %#x\n", buf[i], fileoffset + i - 3);
                sccount = 0;
                payload = 0;
                num_startcodes++;
            }

            sccount = b_check_for_start_code(buf[i], sccount);
        }
        fileoffset += n;
    }
    return 0;
}

