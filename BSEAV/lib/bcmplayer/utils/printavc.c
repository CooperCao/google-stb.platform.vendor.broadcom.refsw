/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
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
#include "bvlc.h"
#include "bkni.h"
#include "ts_utils.h"

/* TODO: don't know actual max */
#define MAX_HEADER_SIZE 100
#define TOTAL_PAYLOAD MAX_HEADER_SIZE

const char *nal_unit_type_str[] = {
    "unspecified",
    "non-IDR slice",
    "slice partition A",
    "slice partition B",
    "slice partition C",
    "IDR slice",
    "SEI",
    "SPS",
    "PPS",
    "AU",
    "EndOfSequence",
    "EndOfStream",
    "Filler",
    "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
    "Unspecified","Unspecified","Unspecified","Unspecified","Unspecified","Unspecified","Unspecified","Unspecified"
};

void print_nal_header(const unsigned char *buf, unsigned long offset)
{
    unsigned char sc = buf[0];
    const unsigned char *payload = &buf[1];
    int nal_ref_idc;
    int nal_unit_type;
    unsigned index = 0, bit = 7;

    if (sc & 0x80) {
        /* if forbidden_zero_bit is set, this is not an AVC start code */
        printf("0x%08lx SC %#x\n", offset, sc);
        return;
    }

    nal_ref_idc = (sc >> 5) & 0x3;
    nal_unit_type = sc & 0x1F;

    printf("0x%08lx %s (SC %02x)\n", offset, nal_unit_type_str[nal_unit_type], sc);

    switch (nal_unit_type) {
    case 1: /* non-IDR slice */
    case 5: /* IDR slice */
        {
        unsigned first_mb_in_slice, slice_type, pic_parameter_set_id;

        /* vlc decode the payload */
        first_mb_in_slice = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);
        slice_type = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);
        pic_parameter_set_id = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);

        printf(
            "  first_mb_in_slice = %d\n"
            "  slice_type = %d\n"
            "  pps_id %d\n",
                first_mb_in_slice,
                slice_type,
                pic_parameter_set_id);
        }
        break;
    case 7: /* sequence parameter set */
        {
        unsigned profile_idc, constraint_flags, level_idc, seq_parameter_set_id;

        /* parse and vlc decode the payload */
        profile_idc = payload[0];
        constraint_flags = payload[1];
        level_idc = payload[2];
        index = 3;
        seq_parameter_set_id = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);

        printf(
            "  SPS=%d\n"
            "  profile_idc=%d flags=0x%x level_idc=%d\n",
            seq_parameter_set_id,
            profile_idc, constraint_flags, level_idc);
        }
        break;
    case 8: /* picture parameter set */
        {
        unsigned pic_parameter_set_id, seq_parameter_set_id;

        /* vlc decode payload */
        pic_parameter_set_id = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);
        seq_parameter_set_id = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);

        printf("  PPS=%d, SPS=%d\n", pic_parameter_set_id, seq_parameter_set_id);
        }
        break;

    case 9: /* access unit delimiter */
        {
        static const char *primary_pic_type_str[] = {
            "I",
            "I,P",
            "I,P,B",
            "SI",
            "SI,SP",
            "I,SI",
            "I,SI,P,SP",
            "I,SI,P,SP,B"
        };
        int primary_pic_type;
        primary_pic_type = (payload[0] >> 5) & 0x3;

        printf("  primary_pic_type=%d (%s)\n", primary_pic_type, primary_pic_type_str[primary_pic_type]);
        }
        break;
    }
}

void print_pictypes(const unsigned char *buf)
{
    unsigned char sc = buf[0];
    const unsigned char *payload = &buf[1];
    int nal_ref_idc;
    int nal_unit_type;
    unsigned index = 0, bit = 7;

    nal_ref_idc = (sc >> 5) & 0x3;
    nal_unit_type = sc & 0x1F;

    switch (nal_unit_type) {
    case 1: /* non-IDR slice */
    case 5: /* IDR slice */
        {
        unsigned first_mb_in_slice, slice_type, pic_parameter_set_id;

        /* vlc decode the payload */
        first_mb_in_slice = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);
        slice_type = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);
        pic_parameter_set_id = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);

        if (first_mb_in_slice == 0) {
            if (nal_unit_type == 5)
                printf("IDR\n");
            else
                printf("non-IDR\n");
        }
        }
        break;
    case 7: /* sequence parameter set */
    case 8: /* picture parameter set */
    case 9: /* access unit delimiter */
        break;
    }
}

int g_slices = 0;
void complete_picture(void)
{
    printf("%d\n", g_slices);
    g_slices = 0;
}

void count_slices(const unsigned char *buf)
{
    unsigned char sc = buf[0];
    const unsigned char *payload = &buf[1];
    int nal_ref_idc;
    int nal_unit_type;
    unsigned index = 0, bit = 7;

    nal_ref_idc = (sc >> 5) & 0x3;
    nal_unit_type = sc & 0x1F;

    switch (nal_unit_type) {
    case 1: /* non-IDR slice */
    case 5: /* IDR slice */
        {
        unsigned first_mb_in_slice;

        /* vlc decode the payload */
        first_mb_in_slice = b_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit);
        if (first_mb_in_slice==0 && g_slices) {
            complete_picture();
        }
        g_slices++;

        }
        break;
    case 7: /* sequence parameter set */
    case 8: /* picture parameter set */
    case 9: /* access unit delimiter */
        if (g_slices) {
            complete_picture();
        }
        break;
    }
}

static void print_usage(void)
{
    printf("Usage: printavc [-slices] [AVC_ES_FILENAME]\n");
    printf("\n");
    printf("-slices            print the # of slices per picture\n");
    printf("-pictypes          print the type of each picture\n");
    printf("AVC_ES_FILENAME    optional filename, otherwise stdin\n");
}

int main(int argc, char **argv) {
    FILE *fin = stdin;
#define BUFSIZE 4096
    unsigned char buf[BUFSIZE];
    int sccount = 0;
    int curarg = 1;
    int slices = 0;
    int pictypes = 0;
    uint64_t fileoffset = 0;

    BKNI_Init();
    BDBG_Init();

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-slices")) {
            slices = 1;
            curarg++;
        }
        else if (!strcmp(argv[curarg], "--help")) {
            print_usage();
            exit(0);
        }
        else if (!strcmp(argv[curarg], "-pictypes")) {
            pictypes = 1;
            curarg++;
        }
    }

    if (curarg < argc) {
        fin = fopen(argv[curarg], "r");
        if (!fin)
            return printf("Unable to open %s: %d\n", argv[curarg], errno);
    }
    else {
        fin = stdin;
    }

    while (!feof(fin)) {
        int i;
        int n;

        n = fread(buf, 1, BUFSIZE, fin);
        if (n == -1) break;

        /* search for start codes */
        for (i=0; i<n; i++) {
            if (sccount == 3) {
                /* check if we have enough to process the header */
                if (n - i < MAX_HEADER_SIZE) {
                    /* move the remainder of the old read to the head of the buffer */
                    memcpy(buf, &buf[i], n-i);
                    fileoffset += i;
                    n -= i;
                    i = 0;

                    /* read more */
                    n += fread(&buf[n], 1, BUFSIZE-n, fin);
                    if (n < MAX_HEADER_SIZE)
                        break;
                }
                if (slices) {
                    count_slices(&buf[i]);
                }
                else if (pictypes) {
                    print_pictypes(&buf[i]);
                }
                else {
                    print_nal_header(&buf[i], fileoffset+i-3);
                }

                sccount = 0;
            }

            sccount = b_check_for_start_code(buf[i], sccount);
        }
        fileoffset += n;
    }
    return 0;
}

