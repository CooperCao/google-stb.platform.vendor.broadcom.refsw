/***************************************************************************
 *     (c)2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 **************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bwin.h"
#include <bkni.h>
#include <bstd.h>

void printUsage()
{
    printf(
    "This application will prerender a font and save it to a file.\n"
    "The characters in the font must come from stdin.\n"
    "\n"
    "Usage: buildfont INPUT_FONT_FILENAME SIZE ANTIALIASED CHARSET OUTPUT_FONT_FILENAME\n"
    " ANTIALIASED = 0 or 1\n"
    " CHARSET = default, stdin\n"
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

    printf("Opening font %s, size %d\n", fontface, size);
    font = bwin_open_font(win, fontface, size, antialiased);
    if (!font) {
        printf("Unable to open font\n");
        return -1;
    }

    if (default_charset) {
        unsigned char ch;
        /* all printable ascii characters */
        for (ch=32;ch<=127;ch++) {
            bwin_measure_text(font, &ch, 1, &width, &height, &base);
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

/* this gives a flat entry point for vxworks */
int vxworks_main(char *arg1,char *arg2,char *arg3,char *arg4,char *arg5,
    char *arg6,char *arg7,char *arg8,char *arg9,char *arg10)
{
    char *argv[11] = {
        "buildfont",arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10};
    int argc;
    for (argc=1;argc<11;argc++)
        if (!argv[argc]) break;
    return main(argc,argv);
}

/* this gives a simple test point */
int go()
{
    int ret;

    ret = vxworks_main("/tgtsvr/teenbold.ttf",
                        "11",
                        "0",
                        "default",
                        "/tgtsvr/arial_11.bwin_font",
                        NULL, NULL, NULL, NULL, NULL);

    ret |= vxworks_main("/tgtsvr/teenbold.ttf",
                        "24",
                        "1",
                        "default",
                        "/tgtsvr/arial_black_24_aa.bwin_font",
                        NULL, NULL, NULL, NULL, NULL);

    ret |= vxworks_main("/tgtsvr/teenbold.ttf",
                        "25",
                        "0",
                        "default",
                        "/tgtsvr/arial_black_25.bwin_font",
                        NULL, NULL, NULL, NULL, NULL);

    ret |= vxworks_main("/tgtsvr/teenbold.ttf",
                        "18",
                        "1",
                        "default",
                        "/tgtsvr/arial_18_aa.bwin_font",
                        NULL, NULL, NULL, NULL, NULL);

    ret |= vxworks_main("/tgtsvr/teenbdit.ttf",
                        "35",
                        "1",
                        "default",
                        "/tgtsvr/arial_italic_35_aa.bwin_font",
                        NULL, NULL, NULL, NULL, NULL);

    return ret;
}

int go1()
{
    int ret;

    ret = vxworks_main("/tgtsvr/Arialbd.ttf",
                        "11",
                        "0",
                        "default",
                        "/tgtsvr/arial_11.bwin_font",
                        NULL, NULL, NULL, NULL, NULL);

    ret |= vxworks_main("/tgtsvr/Arialbd.ttf",
                        "24",
                        "1",
                        "default",
                        "/tgtsvr/arial_black_24_aa.bwin_font",
                        NULL, NULL, NULL, NULL, NULL);

    ret |= vxworks_main("/tgtsvr/Arialbd.ttf",
                        "25",
                        "0",
                        "default",
                        "/tgtsvr/arial_black_25.bwin_font",
                        NULL, NULL, NULL, NULL, NULL);

    ret |= vxworks_main("/tgtsvr/Arialbd.ttf",
                        "18",
                        "1",
                        "default",
                        "/tgtsvr/arial_18_aa.bwin_font",
                        NULL, NULL, NULL, NULL, NULL);

    ret |= vxworks_main("/tgtsvr/Arialbi.ttf",
                        "35",
                        "1",
                        "default",
                        "/tgtsvr/arial_italic_35_aa.bwin_font",
                        NULL, NULL, NULL, NULL, NULL);

    return ret;
}
