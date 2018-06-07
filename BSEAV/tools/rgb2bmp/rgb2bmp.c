/******************************************************************************
 * Copyright (C) 2018 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>

#pragma pack(2)
struct bmp_header {
    unsigned short int type;
    unsigned int size;
    unsigned short int reserved1, reserved2;
    unsigned int offset;
};

#pragma pack(2)
struct bmp_infoheader {
    unsigned int size;
    int width,height;
    unsigned short int planes;
    unsigned short int bits;
    unsigned int compression;
    unsigned int imagesize;
    int xresolution,yresolution;
    unsigned int ncolours;
    unsigned int importantcolours;
};

#define HEADER_SIZE (sizeof(struct bmp_header) + sizeof(struct bmp_infoheader))
#define SIZE (1920 * 4 * 4)

static void print_usage(void)
{
    printf(
    "Usage: rgb2bmp [OPTIONS]\n"
    "\n"
    "Options:\n"
    "  --help or -h for help\n"
    "  -infile FILENAME\n"
    "  -outfile FILENAME         default is header.bmp\n"
    "  -size W,H                 default is 720x480\n"
    "  -bpp bits                 default is 32\n"
    "  -swap_y                   default is not swap\n"
    "  -swap_endian              default is not swap\n"
    );
}

typedef int bool;

/* assume buffer is always big enough to hold one row pixels */
void xfer_one_row(FILE *fout, FILE *fin, int yin, int width, unsigned bpp, bool b_swap_endian)
{
    int x, n;
    char buffer[SIZE];
    char byte0, byte1;
    size_t bytes;

    fseek(fin, yin * width * bpp / 8, SEEK_SET);
    bytes = fread(buffer, 1, width*4, fin);
    if (b_swap_endian)
    {
        for (x=0; x<width; x++)
        {
            n = x * 4;
            byte0 = buffer[n];
            byte1 = buffer[n + 1];
            buffer[n + 0] = buffer[n + 3];
            buffer[n + 1] = buffer[n + 2];
            buffer[n + 2] = byte1;
            buffer[n + 3] = byte0;
        }
    }
    fwrite(buffer, 1, bytes, fout);
}

int main(int argc, const char **argv)
{
    const char *outfile = NULL;
    const char *infile = NULL;
    int width=720, height=480;
    unsigned bpp=32;

    int curarg = 1;
    struct bmp_header header;
    struct bmp_infoheader infoheader;
    FILE *fout = NULL;
    FILE *fin = NULL;
    unsigned image_size;
    bool b_swap_endian = 0;
    bool b_swap_y = 0;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-size") && curarg+1 < argc) {
            if (sscanf(argv[++curarg], "%u,%u", &width, &height) != 2) {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-bpp") && curarg+1 < argc) {
            if (sscanf(argv[++curarg], "%u", &bpp) != 1) {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-infile") && curarg+1 < argc) {
            infile = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-outfile") && curarg+1 < argc) {
            outfile = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-swap_endian")) {
            b_swap_endian = 1;
        }
        else if (!strcmp(argv[curarg], "-swap_y")) {
            b_swap_y = 1;
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }
    if (!outfile) {
        outfile = "header.bmp";
    }
    if (infile) {
        fin = fopen(infile, "rb");
        if (!fin) {
            printf("unable to open file %s\n", infile);
            goto done;
        }
    }

    printf("output filename=%s, width=%u, height=%u, bpp=%u\n", outfile, width, height, bpp);
    fout = fopen(outfile, "wb+");
    if (!fout) {
        printf("unable to create file %s\n", outfile);
        goto done;
    }

    image_size = width * height * (bpp / 8);

    memset(&header, 0, sizeof(header));
    ((unsigned char *)&header.type)[0] = 'B';
    ((unsigned char *)&header.type)[1] = 'M';
    header.size = image_size + HEADER_SIZE;
    header.offset = HEADER_SIZE;
    memset(&infoheader, 0, sizeof(infoheader));
    infoheader.size = sizeof(infoheader);
    infoheader.width = width;
    infoheader.height = height;
    infoheader.planes = 1;
    infoheader.bits = bpp;
    infoheader.imagesize = image_size;

    fwrite(&header, 1, sizeof(header), fout);
    fwrite(&infoheader, 1, sizeof(infoheader), fout);

    if(fin) {
        int y;

        if (b_swap_y) {
            for (y=height-1; y>=0; y--) {
                xfer_one_row(fout, fin, y, width, bpp, b_swap_endian);
            }
        } else {
            for (y=0; y<height; y++) {
                xfer_one_row(fout, fin, y, width, bpp, b_swap_endian);
            }
        }
    }

done:
    if (fout) {
        fclose(fout);
    }
    if (fin) {
        fclose(fin);
    }
    return 0;
}
