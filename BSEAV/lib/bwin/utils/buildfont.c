/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 **************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bwin.h"
#include <bkni.h>
#include <bstd.h>

BDBG_MODULE(buildfont);

#define DEFAULT_SIZE 11
#define FROM_CHAR 32
#define TO_CHAR 127

void print_usage(void)
{
    printf(
    "Usage: buildfont OPTIONS INPUTFILE OUTPUTFILE\n"
    "  INPUTFILE should be a .ttf file\n"
    "  OUTPUTFILE will be a bwin pre-rendered font file\n"
    "  OPTIONS:\n"
    "  -size X (default %u)\n"
    "  -i         generate italic font\n"
    "  -aa        antialias\n"
    "  -char FROM,TO   character set range (default is %u,%u)\n",
    DEFAULT_SIZE, FROM_CHAR, TO_CHAR
    );
}

int main(int argc, char **argv)
{
    const char *fontface;
    const char *outputfile;
    int width, height, base;
    bwin_engine_settings win_engine_settings;
    bwin_engine_t win;
    bwin_font_t font;
    unsigned size = DEFAULT_SIZE;
    bool italic = false;
    bool antialiased = false;
    unsigned from_char = FROM_CHAR, to_char = TO_CHAR;
    int curarg = 1;

    /* bwin requires basemodules */
    BKNI_Init();
    BDBG_Init();

    bwin_engine_settings_init(&win_engine_settings);
    win = bwin_open_engine(&win_engine_settings);

    printf(
    "buildfont - prerenders fonts for the bwin system using freetype.\n"
    "\n"
    );

    while (curarg < argc) {
        if (!strcmp(argv[curarg],"-size") && curarg+1<argc) {
            size = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg],"-i")) {
            italic = true;
        }
        else if (!strcmp(argv[curarg],"-aa")) {
            antialiased = true;
        }
        else if (!strcmp(argv[curarg],"-char") && curarg+1<argc) {
            if (sscanf(argv[++curarg], "%u,%u", &from_char, &to_char) != 2) {
                print_usage();
                return -1;
            }
        }
        else if (!fontface) {
            fontface = argv[curarg];
        }
        else if (!outputfile) {
            outputfile = argv[curarg];
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    if (!fontface || !outputfile) {
        print_usage();
        return -1;
    }

    printf("Opening font %s, size %d %s\n", fontface, size, italic ? "for italic font" : "" );
    if (italic) {
        font = bwin_open_font_italic(win, fontface, size, antialiased);
    }
    else {
        font = bwin_open_font(win, fontface, size, antialiased);
    }
    if (!font) {
        printf("Unable to open font\n");
        return -1;
    }

    {
        uint32_t        code_point  = from_char;
        uint32_t        ch_utf8     = 0;
        unsigned char * pch_utf8    = NULL;
        uint32_t        num_success = 0;
        uint32_t        num_failure = 0;

        /* all printable ascii characters */
        while (1) {
            pch_utf8 = (unsigned char *)&ch_utf8;

            /* convert unicode code point to utf-8 value for bwin_measure_text() */
            if (code_point < 0x80) {
                *pch_utf8++ = code_point;
            }
            else if (code_point < 0x800) {
                *pch_utf8++ = 192 + code_point / 64;
                *pch_utf8++ = 128 + code_point % 64;
            }
            else if (code_point < 0x10000) {
                *pch_utf8++ = 224 + code_point / 4096;
                *pch_utf8++ = 128 + code_point / 64 % 64;
                *pch_utf8++ = 128 + code_point % 64;
            }
            else if (code_point < 0x110000) {
                *pch_utf8++ = 240 + code_point / 262144;
                *pch_utf8++ = 128 + code_point / 4096 % 64;
                *pch_utf8++ = 128 + code_point / 64 % 64;
                *pch_utf8++ = 128 + code_point % 64;
            }
            else {
                BDBG_ERR(("unicode code point (0x%X) too high to represent in utf-8 - aborting", code_point));
                break;
            }

            int rc = bwin_measure_text(font, (const char *)&ch_utf8, 1, &width, &height, &base);
            if (rc) {
                BDBG_WRN(("font does not contain char %#x (%c)", code_point, code_point));
                num_failure++;
            }
            else {
                BDBG_WRN(("measure %#x (%c) = %dx%d, base %d", code_point, code_point, width, height, base));
                num_success++;
            }
            if (code_point++ == to_char) break;
        }

        BDBG_WRN(("Done! Range:%d-%d Converted:%d Missing:%d", from_char, to_char, num_success, num_failure));
    }

    return bwin_save_rendered_font(font, outputfile);
}
