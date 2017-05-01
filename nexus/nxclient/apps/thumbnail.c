/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#include "nxclient.h"
#include "nexus_platform_client.h"
#include "bgui.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "thumbdecoder.h"
#include "picdecoder.h"

BDBG_MODULE(thumbnail);

static void print_usage(void)
{
    printf(
    "Usage: thumbnail FILE [INDEXFILE]\n"
    "  --help or -h for help\n"
    "  -rect x,y,width,height   position in default 1920x1080 coordinates\n"
    "  -zorder #\n"
    "  -gui off\n"
    "  -output FILE  file to save bmp of thumbnail when running gui off mode\n"
    "  -pos SECONDS\n"
    );
}

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    bgui_t gui;
    struct bgui_settings gui_settings;
    NEXUS_Error rc;
    int curarg = 1;
    NEXUS_Rect rect = {0,0,0,0};
    unsigned zorder = 0, timeout = 0;
    const char *filename = NULL;
    const char *indexfilename = NULL;
    const char *outputname = NULL;
    thumbdecoder_t thumb;
    bool use_gui = true;
    NEXUS_SurfaceMemory mem;
    NEXUS_SurfaceHandle surface = NULL;
    NEXUS_SurfaceCreateSettings createSettings;
    unsigned pos = 0;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-rect") && argc>curarg+1) {
            unsigned x, y, width, height;
            if (sscanf(argv[++curarg], "%u,%u,%u,%u", &x,&y,&width,&height) == 4) {
                rect.x = x;
                rect.y = y;
                rect.width = width;
                rect.height = height;
            }
        }
        else if (!strcmp(argv[curarg], "-zorder") && argc>curarg+1) {
            zorder = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-gui") && argc>curarg+1) {
            use_gui = strcmp(argv[++curarg], "off");
        }
        else if (!strcmp(argv[curarg], "-output") && argc>curarg+1) {
            outputname = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-pos") && argc>curarg+1) {
            pos = atoi(argv[++curarg]);
        }
        else if (!filename) {
            filename = argv[curarg];
        }
        else if (!indexfilename) {
            indexfilename = argv[curarg];
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }
    if (!filename) {
        print_usage();
        return 1;
    }

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    if (use_gui) {
        bgui_get_default_settings(&gui_settings);
        gui_settings.width = rect.width?rect.width:1280;
        gui_settings.height = rect.height?rect.height:720;
        gui = bgui_create(&gui_settings);

        if (rect.width || zorder) {
            NEXUS_SurfaceComposition comp;
            NxClient_GetSurfaceClientComposition(bgui_surface_client_id(gui), &comp);
            if (rect.width) {
                comp.position = rect;
            }
            comp.zorder = zorder;
            NxClient_SetSurfaceClientComposition(bgui_surface_client_id(gui), &comp);
        }
    } else {
        NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
        createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        createSettings.width = rect.width?rect.width:1280;
        createSettings.height = rect.height?rect.height:720;
        surface = NEXUS_Surface_Create(&createSettings);
    }

    thumb = thumbdecoder_open();
    rc = thumbdecoder_open_file(thumb, filename, indexfilename);
    if (rc) {BERR_TRACE(rc); goto done;}
    rc = thumbdecoder_decode_still(thumb, pos * 1000, use_gui?bgui_surface(gui):surface);
    if (rc) {BERR_TRACE(rc); goto done;}

    if (use_gui) {
        bgui_submit(gui);
    } else {
        if (outputname) {
            NEXUS_Surface_GetMemory(surface, &mem); /* only needed for flush */
            NEXUS_Surface_Flush(surface);
            picdecoder_write_bmp(outputname, surface, 24);
        }
    }

    if (use_gui) {
        if (timeout) {
            BKNI_Sleep(timeout * 1000);
        }
        else {
            while (1) BKNI_Sleep(1000);
        }
    }

done:
    thumbdecoder_close(thumb);
    if (use_gui) {
        bgui_destroy(gui);
    } else if (surface) {
        NEXUS_Surface_Destroy(surface);
    }
    NxClient_Uninit();
    return 0;
}
