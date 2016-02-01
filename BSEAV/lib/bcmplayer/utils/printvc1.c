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
#include "ts_utils.h"

/* TODO: don't know actual max */
#define MAX_HEADER_SIZE 100
#define TOTAL_PAYLOAD MAX_HEADER_SIZE

/* start code suffixes */
const char *scs_str[] = {
    "reserved",
    "reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
    "End of Sequence",
    "Slice",
    "Field",
    "Frame",
    "Entry-point",
    "Sequence",
    "reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
    "Slice User Data",
    "Field User Data",
    "Frame User Data",
    "Entry-point User Data",
    "Sequence User Data" /* 0x1f */
};

const char *ptype_str[] = {
    "I",
    "P",
    "B",
    "BI",
    "skipped"
};

const char *fptype_str[] = {
    "I,I",
    "I,P",
    "P,I",
    "P,P",
    "B,B",
    "B,BI",
    "BI,B",
    "BI,BI"
};

bool g_seq_interlace = false;

struct vc1_vlc_code {
    unsigned long bitstring;
    int length;
};

struct vc1_vlc_code g_fcm[] = {
    {0x0, 1}, /* 0 - progressive */
    {0x2, 2}, /* 10 - frame-interlace */
    {0x3, 2}, /* 11 - field-interlace */
    {0,0} /* terminal */
};
struct vc1_vlc_code g_ptype[] = {
    {0x6, 3}, /* I */
    {0x0, 1}, /* P */
    {0x2, 2}, /* B */
    {0xE, 4}, /* BI */
    {0xF, 4}, /* skipped */
    {0,0} /* terminal */
};
struct vc1_vlc_code g_fptype[] = {
    {0x0, 3}, /* I,I */
    {010, 3}, /* I,P */
    {0x2, 3}, /* P,I */
    {0x3, 3}, /* P,P */
    {0x4, 3}, /* B,B */
    {0x5, 3}, /* B,BI */
    {0x6, 3}, /* BI,B */
    {0x7, 3}, /* BI,BI */
    {0,0} /* terminal */
};

unsigned char g_mask[] = {0,1,3,7,0xf};

int b_vc1_vlc_decode(
    const uint8_t *data,            /* [size_is(size)] Array of bytes which contain a vlc encoded value */
    unsigned size,          /* Size of data */
    unsigned current_index, /* The starting index the data array. */
    unsigned current_bit,   /* The bit in data[current_index] where the vlc decode should start.
                                vlc decode proceeds from MSB to LSB, so the first
                                bit will be 7. */
    unsigned *next_index, /* [out] the index in data which should be used for an
                                adjacent vlc decode. Can be NULL. */
    unsigned *next_bit,     /* [out] the bit in data[index] which should be used for an
                                adjacent vlc decode. Can be NULL. */
    const struct vc1_vlc_code *pattern
    )
{
    int i;
    BSTD_UNUSED(size);

    /* TODO: make it span bytes, if needed. don't think so for vc1. */
    for (i=0;pattern[i].length;i++) {
        unsigned char d = data[0] >> (current_bit - pattern[i].length + 1);
        d &= g_mask[pattern[i].length];
        if (d == pattern[i].bitstring) {
            *next_index = current_index;
            *next_bit = current_bit - pattern[i].length;
            return i;
        }
    }
    return -1;
}

void print_scs(const unsigned char *buf, unsigned long offset)
{
    unsigned char sc = buf[0];
    const unsigned char *payload = &buf[1];
    unsigned index = 0, bit = 7;

    if (sc > 0x1f) {
        /* reserved and forbidden */
        return;
    }

    printf("0x%08lx %s (SC %02x)\n", offset, scs_str[sc], sc);

    switch (sc) {
    case 0x0D: /* frame */
        {
        int fcm, ptype;

        fcm = b_vc1_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit, g_fcm);
        printf("  fcm %d\n", fcm);
        switch (fcm) {
        case 0: /* progressive */
        case 1: /* frame interlace */
            ptype = b_vc1_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit, g_ptype);
            if (ptype < 5) {
                printf("  PTYPE %s\n", ptype_str[ptype]);
            }
            else {
                printf("  invalid PTYPE %d\n", ptype);
            }
            break;
        case 2: /* field interlace */
            ptype = b_vc1_vlc_decode(payload, TOTAL_PAYLOAD, index, bit, &index, &bit, g_fptype);
            if (ptype < 8) {
                printf("  FPTYPE %s\n", fptype_str[ptype]);
            }
            else {
                printf("  invalid FPTYPE %d\n", ptype);
            }
            break;
        default:
            printf("  invalid FCM %d\n", fcm);
            return;
        }
        }
        break;

    case 0x0F: /* sequence */
        {
        g_seq_interlace = buf[5] & 0x40; /* 42nd bit after SC */
        printf("  interlace? %s\n", g_seq_interlace?"yes":"no");
        }
        break;
    }
}

#if 0
void print_pictypes(const unsigned char *buf)
{
    unsigned char sc = buf[0];
    const unsigned char *payload = &buf[1];

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
#endif

static void print_usage(void)
{
    printf("Usage: printvc1 [-slices] [VC1_ES_FILENAME]\n");
    printf("\n");
#if 0
    printf("-slices            print the # of slices per picture\n");
    printf("-pictypes          print the type of each picture\n");
#endif
    printf("VC1_ES_FILENAME    optional filename, otherwise stdin\n");
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
#if 0
                if (slices) {
                    count_slices(&buf[i]);
                }
                else if (pictypes) {
                    print_pictypes(&buf[i]);
                }
                else
#endif
                {
                    print_scs(&buf[i], fileoffset+i-3);
                }

                sccount = 0;
            }

            sccount = b_check_for_start_code(buf[i], sccount);
        }
        fileoffset += n;
    }
    return 0;
}

