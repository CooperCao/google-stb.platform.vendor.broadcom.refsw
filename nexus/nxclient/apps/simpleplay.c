/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 *****************************************************************************/
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER
#include "media_player.h"
#include "bgui.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* play an audio/video file */
static void print_usage(void)
{
    printf(
    "Usage: simpleplay OPTIONS stream_file\n"
    "\n"
    "OPTIONS:\n"
    "  --help or -h for help\n"
    "  -timeout SECONDS\n"
    "  -prompt\n"
    );
}

int main(int argc, const char **argv)
{
    int rc;
    media_player_t player;
    int curarg = 1;
    media_player_create_settings create_settings;
    media_player_start_settings start_settings;
    bgui_t gui;
    struct bgui_settings gui_settings;
    unsigned timeout = 0;
    bool prompt = false;
    NxClient_JoinSettings joinSettings;
    const char *filename = NULL;
    
    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-prompt")) {
            prompt = true;
        }
        else if (!filename ) {
            filename  = argv[curarg];
        }
        else {
            print_usage();
            return -1;
            break;
        }
        curarg++;
    }

    if (!filename) {
        print_usage();
        return -1;
    }
    
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s %s", argv[0], filename);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    bgui_get_default_settings(&gui_settings);
    gui_settings.width = 10;
    gui_settings.height = 10;
    gui = bgui_create(&gui_settings);
    bgui_fill(gui, 0);
    bgui_checkpoint(gui);
    bgui_submit(gui);

    /* need SurfaceClient for video so SurfaceCompositor can set VideoWindow to full screen and visible */
    NEXUS_SurfaceClient_AcquireVideoWindow(bgui_surface_client(gui), 0);

    media_player_get_default_create_settings(&create_settings);
    create_settings.window.surfaceClientId = bgui_surface_client_id(gui);
    player = media_player_create(&create_settings);
    if (!player) return -1;
    
    media_player_get_default_start_settings(&start_settings);
    start_settings.stream_url = filename;
    rc = media_player_start(player, &start_settings);
    if (rc) return rc;

    if (timeout) {
        BKNI_Sleep(timeout * 1000);
    }
    else if (prompt) {
        printf("Press ENTER to stop\n");
        getchar();
    }
    else {
        while (1) BKNI_Sleep(10);
    }

    media_player_stop(player);
    media_player_destroy(player);
    bgui_destroy(gui);
    NxClient_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs playback and simple_decoder)!\n");
    return 0;
}
#endif
