/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

void printUsage(void)
{
    printf(
    "This application will prerender a font and save it to a file.\n"
    "The characters in the font must come from stdin.\n"
    "\n"
    "Usage: buildfont INPUT_FONT_FILENAME SIZE ANTIALIASED CHARSET OUTPUT_FONT_FILENAME { -i }\n"
    " ANTIALIASED = 0 or 1\n"
    " CHARSET = default, stdin\n"
    " -i = generate italic font\n"
    );
}

int main(int argc, char **argv)
{
    const char *fontface;
    int size, antialiased, default_charset;
    const char *outputfile;
    int width, height, base;
    bwin_engine_settings win_engine_settings;
    bwin_engine_t win;
    bwin_font_t font;
	bool italic = false;	/* default */

    /* bwin requires basemodules */
    BKNI_Init();
    BDBG_Init();

    bwin_engine_settings_init(&win_engine_settings);
    win = bwin_open_engine(&win_engine_settings);

    printf(
    "buildfont - prerenders fonts for the bwin system using freetype.\n"
    "\n"
    );
    if (argc < 6) {
        printUsage();
        return -1;
    }

    fontface = argv[1];
    size = atoi(argv[2]);
    antialiased = atoi(argv[3]);
    default_charset = !strcasecmp(argv[4], "default");
    outputfile = argv[5];
    if (argc >= 7 && !strcmp(argv[6],"-i")) {
        italic = true;
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

    if (default_charset) {
        unsigned char ch;
        /* all printable ascii characters */
        for (ch=32;ch<=127;ch++) {
            bwin_measure_text(font, (const char *)&ch, 1, &width, &height, &base);
            /* printf("measure '%c' = %dx%d,base %d\n", ch, width, height, base); */
        }
    }
    else {
        while (!feof(stdin)) {
            char ch;
            if (fread(&ch, 1, 1, stdin) != 1)
                break;
            bwin_measure_text(font, &ch, 1, &width, &height, &base);
        }
    }

    return bwin_save_rendered_font(font, outputfile);
}
