/******************************************************************************
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
 ******************************************************************************/
#if NEXUS_HAS_INPUT_ROUTER && NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER
#include "media_player.h"
#include "nxclient.h"
#include "bgui.h"
#include "binput.h"
#include "nxapps_cmdline.h"
#include "nexus_display.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "namevalue.h"

BDBG_MODULE(playmosaic);

/* play an mosaic video file */

#define MAX_MOSAICS 8

#define INTERPOLATE(start,finish,cnt,steps) \
    ((((cnt)*(((finish)-(start)) /(steps)))+(start)))

static void print_usage(const struct nxapps_cmdline *cmdline)
{
    printf(
        "Usage: playmosaic OPTIONS stream1 [stream2 ...]\n"
        "\n"
        "If # streams < # mosaics, the list will be cycled for subsequent programs\n"
        "If # of mosaics available is less than requested, decode will proceed\n"
        "\n"
        "OPTIONS:\n"
        "  --help or -h for help\n"
        "  -n NUM_MOSAICS           default and max is %d\n", MAX_MOSAICS
        );
    nxapps_cmdline_print_usage(cmdline);
    printf(
        "  -max WIDTH,HEIGHT        default is 352,288 (CIF)\n"
        "  -cycle X                 randomly stop and restart mosaics every X vsyncs\n"
        "  -timeout SECONDS\n"
        "  -audio_primers           use primers for fast audio switch\n"
        "  -toggle SECONDS          toggle mosaic rectangle\n"
        "  -gui off\n"
        "  -mosaic_pip              2 mosaics played as full + 1/4 screen (like main + PiP)\n"
        "  -move                    moving mosaic windows\n"
        );
}

#include <sys/time.h>
static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

static unsigned g_timeout = 0;
static void b_start_timeout(unsigned timeout)
{
    if (timeout) {
        g_timeout = b_get_time() + timeout*1000;
    }
}
static bool b_check_timeout(void)
{
    return g_timeout && b_get_time() >= g_timeout;
}

static struct {
    bgui_t gui;
    struct {
        NEXUS_SurfaceClientHandle video_sc;
        media_player_start_settings start_settings;
        bool started;
        media_player_t player;
        NEXUS_Rect rect;
        unsigned zorder;
        struct {
            int x, y, h;
       } move;
    } mosaic[MAX_MOSAICS];
    unsigned num_mosaics;
    unsigned current;
    bool done;
} g_app;

static void draw_gui(bool full_gui)
{
    NEXUS_Graphics2DHandle gfx = bgui_blitter(g_app.gui);
    NEXUS_Graphics2DFillSettings fillSettings;
    int rc;
    NEXUS_Rect rect = g_app.mosaic[g_app.current].rect;

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = bgui_surface(g_app.gui);
    fillSettings.color = 0;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);

    if (full_gui) {
#define FOCUS_BAR_WIDTH 5
        fillSettings.color = 0xFFFFFF00; /* yellow */
        fillSettings.rect = rect;
        fillSettings.rect.height = FOCUS_BAR_WIDTH;
        rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        BDBG_ASSERT(!rc);
        fillSettings.rect.y += rect.height - FOCUS_BAR_WIDTH;
        rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        BDBG_ASSERT(!rc);
        fillSettings.rect = rect;
        fillSettings.rect.width = FOCUS_BAR_WIDTH;
        rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        BDBG_ASSERT(!rc);
        fillSettings.rect.x += rect.width - FOCUS_BAR_WIDTH;
        rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        BDBG_ASSERT(!rc);
    }

    bgui_checkpoint(g_app.gui);
    bgui_submit(g_app.gui);
}

static void set_move(unsigned i)
{
    g_app.mosaic[i].move.x = i+1;
    g_app.mosaic[i].move.y = i+1;
    g_app.mosaic[i].move.h = i+1;
}

static void move(unsigned i)
{
    NEXUS_SurfaceClientSettings settings;
    NEXUS_SurfaceClient_GetSettings(g_app.mosaic[i].video_sc, &settings);
    settings.composition.position.height = /* approximate RTS supportable size */
        (settings.composition.virtualDisplay.height) / (g_app.num_mosaics+1);
    settings.composition.position.width  = settings.composition.position.height * 16 / 9;
    settings.composition.position.x += g_app.mosaic[i].move.x;
    settings.composition.position.y += g_app.mosaic[i].move.y;
    NEXUS_SurfaceClient_SetSettings(g_app.mosaic[i].video_sc, &settings);
    if (g_app.mosaic[i].move.h) {
        if (settings.composition.position.height > 1080 || settings.composition.position.height < 200) {
            g_app.mosaic[i].move.h *= -1;
        }
    }
    if (settings.composition.position.x + settings.composition.position.width >= 1920 ||
        settings.composition.position.x < 0) g_app.mosaic[i].move.x *= -1;
    if (settings.composition.position.y + settings.composition.position.height >= 1080 ||
        settings.composition.position.y < 0) g_app.mosaic[i].move.y *= -1;
    BDBG_MSG(("%d: %d,%d,%d,%d      change by %d,%d",
        i,
        settings.composition.position.x, settings.composition.position.y, settings.composition.position.width, settings.composition.position.height,
        g_app.mosaic[i].move.x, g_app.mosaic[i].move.y));
}

static void *standby_monitor(void *context)
{
    NEXUS_Error rc;
    NxClient_StandbyStatus standbyStatus, prevStatus;
    unsigned i;

    BSTD_UNUSED(context);

    rc = NxClient_GetStandbyStatus(&prevStatus);
    BDBG_ASSERT(!rc);

    while(!g_app.done) {
        rc = NxClient_GetStandbyStatus(&standbyStatus);
        BDBG_ASSERT(!rc);

        if(standbyStatus.transition == NxClient_StandbyTransition_eAckNeeded) {
            printf("'play' acknowledges standby state: %s\n", lookup_name(g_platformStandbyModeStrs, standbyStatus.settings.mode));
            for (i=0;i<g_app.num_mosaics;i++) {
                if (g_app.mosaic[i].started) {
                    media_player_stop(g_app.mosaic[i].player);
                    g_app.mosaic[i].started = false;
                }
            }
            NxClient_AcknowledgeStandby(true);
        } else {
            if(standbyStatus.settings.mode == NEXUS_PlatformStandbyMode_eOn && prevStatus.settings.mode != NEXUS_PlatformStandbyMode_eOn) {
                for (i=0;i<g_app.num_mosaics;i++) {
                    if (!g_app.mosaic[i].player) continue;
                    rc = media_player_start(g_app.mosaic[i].player, &g_app.mosaic[i].start_settings);
                    if (!rc) {
                        g_app.mosaic[i].started = true;
                    }
                }
                media_player_switch_audio(g_app.mosaic[g_app.current].player);
            }
        }

        prevStatus = standbyStatus;
        BKNI_Sleep(100);
    }

    return NULL;
}

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc;
    unsigned i;
    media_player_t player[MAX_MOSAICS];
    media_player_create_settings create_settings;
    struct {
        const char *name;
        unsigned programindex;
    } file[MAX_MOSAICS];
    int cur_file = 0;
    int curarg = 1;
    unsigned num_started = 0;
    unsigned cycle = 0;
    unsigned toggle_time = 0;
    unsigned num_tiles;
    unsigned timeout = 0;
    bool audio_primers = false;
    NEXUS_SurfaceRegion virtualDisplay = {1920,1080};
    bool gui = true;
    struct bgui_settings gui_settings;
    binput_t input;
    bool mosaic_pip = false;
    bool mosaic_move = false;
    struct {
        unsigned cnt, time;
    } fps = {0,0};
    struct nxapps_cmdline cmdline;
    int n;
    pthread_t standby_thread_id;

    memset(file, 0, sizeof(file));
    g_app.num_mosaics = MAX_MOSAICS;
    nxapps_cmdline_init(&cmdline);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_SurfaceComposition);
    media_player_get_default_create_settings(&create_settings);

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage(&cmdline);
            return 0;
        }
        else if (!strcmp(argv[curarg], "-n") && curarg+1 < argc) {
            g_app.num_mosaics = atoi(argv[++curarg]);
            if (g_app.num_mosaics > MAX_MOSAICS) {
                print_usage(&cmdline);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-max") && curarg+1 < argc) {
            int n = sscanf(argv[++curarg], "%u,%u", &create_settings.maxWidth, &create_settings.maxHeight);
            if (n != 2) {
                print_usage(&cmdline);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-cycle") && argc>curarg+1) {
            cycle = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-toggle") && argc>curarg+1) {
            toggle_time = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-audio_primers")) {
            audio_primers = true;
        }
        else if (!strcmp(argv[curarg], "-gui") && argc>curarg+1) {
            gui = strcmp(argv[++curarg], "off");
        }
        else if (!strcmp(argv[curarg], "-mosaic_pip")) {
            mosaic_pip = true;
        }
        else if (!strcmp(argv[curarg], "-move")) {
            mosaic_move = true;
            gui = false;
        }
        else if ((n = nxapps_cmdline_parse(curarg, argc, argv, &cmdline))) {
            if (n < 0) {
                print_usage(&cmdline);
                return -1;
            }
            curarg += n;
        }
        else if (cur_file < MAX_MOSAICS) {
            file[cur_file].name = argv[curarg];
            cur_file++;
        }
        else {
            print_usage(&cmdline);
            return -1;
        }
        curarg++;
    }

    if (mosaic_pip) {
        /* Override number of mosaics to 2 */
        g_app.num_mosaics = 2;
    }

    if (!file[0].name) {
        print_usage(&cmdline);
        return -1;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s %s", argv[0], file[0].name);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    rc = pthread_create(&standby_thread_id, NULL, standby_monitor, NULL);
    BDBG_ASSERT(!rc);

    input = binput_open(NULL);
    bgui_get_default_settings(&gui_settings);
    gui_settings.width = virtualDisplay.width;
    gui_settings.height = virtualDisplay.height;
    g_app.gui = bgui_create(&gui_settings);

    if (nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SurfaceComposition)) {
        NEXUS_SurfaceComposition comp;
        NxClient_GetSurfaceClientComposition(bgui_surface_client_id(g_app.gui), &comp);
        nxapps_cmdline_apply_SurfaceComposition(&cmdline, &comp);
        NxClient_SetSurfaceClientComposition(bgui_surface_client_id(g_app.gui), &comp);
    }

    num_tiles = g_app.num_mosaics;
    if (num_tiles % 2) {
        num_tiles++;
    }

    {
        NEXUS_DisplayMaxMosaicCoverage maxCoverage;
        unsigned coverage = 0;
        NEXUS_Display_GetMaxMosaicCoverage(0, g_app.num_mosaics, &maxCoverage);
        for (i=0;i<g_app.num_mosaics;i++) {
            if (mosaic_pip) {
                g_app.mosaic[i].rect.width = (i == 0) ? virtualDisplay.width : virtualDisplay.width/2;
                g_app.mosaic[i].rect.height = (i == 0) ? virtualDisplay.height : virtualDisplay.height/2;
                g_app.mosaic[i].rect.x = (i == 0) ? 0 : g_app.mosaic[i].rect.width;
                g_app.mosaic[i].rect.y = (i == 0) ? 0 : g_app.mosaic[i].rect.height;
                g_app.mosaic[i].zorder = i;
            }
            else {
                unsigned max_height;
                g_app.mosaic[i].rect.width = virtualDisplay.width / (num_tiles / 2);
                g_app.mosaic[i].rect.height = virtualDisplay.height / 2;
                g_app.mosaic[i].rect.x = g_app.mosaic[i].rect.width * (i % (num_tiles/2));
                g_app.mosaic[i].rect.y = g_app.mosaic[i].rect.height * (i / (num_tiles/2));
                /* RTS restriction is (essentially) width>=height. Applying 3/4 allows 16:9 HD and 4:3 SD to meet the requirement. */
                max_height = g_app.mosaic[i].rect.width*3/4;
                if (g_app.mosaic[i].rect.height > max_height) {
                    g_app.mosaic[i].rect.y += (g_app.mosaic[i].rect.height - max_height) / 2;
                    g_app.mosaic[i].rect.height = max_height;
                }
            }
            coverage += g_app.mosaic[i].rect.width * g_app.mosaic[i].rect.height;
        }
        /* convert from pixels to percent */
        coverage = coverage * 100 / (virtualDisplay.width * virtualDisplay.height);
        if (coverage > maxCoverage.maxCoverage) {
            unsigned shrink = (coverage - maxCoverage.maxCoverage) * 100 / coverage;
            BDBG_WRN(("shrink by %u%% to fit %u%% coverage within %u%% max coverage", shrink, coverage, maxCoverage.maxCoverage));
            for (i=0;i<g_app.num_mosaics;i++) {
                uint16_t width  = g_app.mosaic[i].rect.width * (100-shrink)/100;
                uint16_t height = g_app.mosaic[i].rect.height * (100-shrink)/100;
                g_app.mosaic[i].rect.x     += (g_app.mosaic[i].rect.width - width) / 2;
                g_app.mosaic[i].rect.y     += (g_app.mosaic[i].rect.height - height) / 2;
                g_app.mosaic[i].rect.width  = width;
                g_app.mosaic[i].rect.height = height;
            }
        }
    }
    for (i=0;i<g_app.num_mosaics;i++) {
        NEXUS_SurfaceClientSettings settings;

        g_app.mosaic[i].video_sc = NEXUS_SurfaceClient_AcquireVideoWindow(bgui_surface_client(g_app.gui), i);
        if (!g_app.mosaic[i].video_sc) {
            BDBG_ERR(("unable to create window %d", i));
            rc = -1;
        }
        NEXUS_SurfaceClient_GetSettings(g_app.mosaic[i].video_sc, &settings);
        settings.composition.position = g_app.mosaic[i].rect;
        settings.composition.zorder = g_app.mosaic[i].zorder;
        settings.composition.virtualDisplay = virtualDisplay;
        NEXUS_SurfaceClient_SetSettings(g_app.mosaic[i].video_sc, &settings);
    }
    draw_gui(gui);

    create_settings.window.surfaceClientId = bgui_surface_client_id(g_app.gui);
    rc = media_player_create_mosaics(player, g_app.num_mosaics, &create_settings);
    if (rc) return -1;

    cur_file = 0;
    for (i=0;i<g_app.num_mosaics;i++) {
        g_app.mosaic[i].player = player[i];
        if (!g_app.mosaic[i].player) continue;

        {
            /* TEMP disable CC for mosaics */
            NEXUS_SimpleVideoDecoderHandle videoDecoder = media_player_get_video_decoder(g_app.mosaic[i].player);
            NEXUS_SimpleVideoDecoderClientSettings settings;
            NEXUS_SimpleVideoDecoder_GetClientSettings(videoDecoder, &settings);
            settings.closedCaptionRouting = false;
            NEXUS_SimpleVideoDecoder_SetClientSettings(videoDecoder, &settings);
        }

        media_player_get_default_start_settings(&g_app.mosaic[i].start_settings);
        g_app.mosaic[i].start_settings.stream_url = file[cur_file].name;
        g_app.mosaic[i].start_settings.program = file[cur_file].programindex;
        g_app.mosaic[i].start_settings.stcTrick = false; /* doesn't work for shared STC */
        g_app.mosaic[i].start_settings.audio_primers = audio_primers;
        rc = media_player_start(g_app.mosaic[i].player, &g_app.mosaic[i].start_settings);
        if (rc) {
            if (file[cur_file].programindex) {
                /* try again with program 0 */
                file[cur_file].programindex = g_app.mosaic[i].start_settings.program = 0;
                rc = media_player_start(g_app.mosaic[i].player, &g_app.mosaic[i].start_settings);
            }
            if (rc) {
                BDBG_WRN(("unable to start %s", g_app.mosaic[i].start_settings.stream_url));
            }
            /* keep going */
        }
        if (!rc) {
            g_app.mosaic[i].started = true;
            num_started++;
            file[cur_file].programindex++;
        }

        if (!file[++cur_file].name) {
            cur_file = 0;
        }
    }
    if (!num_started) return -1;

    b_start_timeout(timeout);
    if (cycle) {
        while (1) {
            unsigned i;
            BKNI_Sleep(cycle * 16);
            i = rand() % g_app.num_mosaics;
            if (g_app.mosaic[i].started) {
                BDBG_WRN(("stopping %d", i));
                media_player_stop(g_app.mosaic[i].player);
                g_app.mosaic[i].started = false;
            }
            else {
                BDBG_WRN(("starting %d", i));
                rc = media_player_start(g_app.mosaic[i].player, &g_app.mosaic[i].start_settings);
                if (!rc) g_app.mosaic[i].started = true;
            }
            if (b_check_timeout()) break;
        }
    }
    else if ( toggle_time ) {
        signed k;
        signed steps = 10;
        NEXUS_Rect smallRect, largeRect;
        NEXUS_SurfaceClientSettings settings;

        BKNI_Sleep(1000);

        BDBG_WRN(("starting toggle"));
        for (i=0; i<g_app.num_mosaics ; i++) {
            g_app.mosaic[i].rect.width = virtualDisplay.width / g_app.num_mosaics;
            g_app.mosaic[i].rect.height = virtualDisplay.height / 4;
            g_app.mosaic[i].rect.x = i* (virtualDisplay.width / g_app.num_mosaics);
            g_app.mosaic[i].rect.y = virtualDisplay.height * 3 / 4;

            NEXUS_SurfaceClient_GetSettings(g_app.mosaic[i].video_sc, &settings);
            settings.composition.position = g_app.mosaic[i].rect;
            settings.composition.virtualDisplay = virtualDisplay;
            NEXUS_SurfaceClient_SetSettings(g_app.mosaic[i].video_sc, &settings);
        }

        if (gui) draw_gui(gui);

        BKNI_Sleep(1000);
        largeRect.width   = virtualDisplay.width * 2/ g_app.num_mosaics;
        largeRect.height = virtualDisplay.height * 2 / g_app.num_mosaics;
        largeRect.x      = (virtualDisplay.width - largeRect.width) /2 ;
        largeRect.y      = (virtualDisplay.height * 3 /4 - largeRect.height) /2 ;

        while (1) {
            for (i=0; i<g_app.num_mosaics ; i++) {
                smallRect.x      = g_app.mosaic[i].rect.x;
                smallRect.y      = g_app.mosaic[i].rect.y;
                smallRect.width  = g_app.mosaic[i].rect.width;
                smallRect.height = g_app.mosaic[i].rect.height;

                NEXUS_SurfaceClient_GetSettings(g_app.mosaic[i].video_sc, &settings);
                for (k=0 ; k<steps ; k++ ) {
                    settings.composition.zorder          = 2;
                    settings.composition.position.x      = INTERPOLATE(smallRect.x,largeRect.x,k+1,steps);
                    settings.composition.position.y      = INTERPOLATE(smallRect.y,largeRect.y,k+1,steps);
                    settings.composition.position.width  = INTERPOLATE(smallRect.width,largeRect.width,k+1,steps);
                    settings.composition.position.height = INTERPOLATE(smallRect.height,largeRect.height,k+1,steps);
                    NEXUS_SurfaceClient_SetSettings(g_app.mosaic[i].video_sc, &settings);
                    bgui_wait_for_move(g_app.gui);
                }

                BDBG_WRN(("toggle to window %d in %d seconds", (i+1)%g_app.num_mosaics, toggle_time));
                fflush(stdout);
                BKNI_Sleep(toggle_time * 1000);

                if (b_check_timeout()) break;

                NEXUS_SurfaceClient_GetSettings(g_app.mosaic[i].video_sc, &settings);
                for (k=0 ; k<steps ; k++ ) {
                    settings.composition.zorder          = 2;
                    settings.composition.position.x      = INTERPOLATE(largeRect.x,smallRect.x,k+1,steps);
                    settings.composition.position.y      = INTERPOLATE(largeRect.y,smallRect.y,k+1,steps);
                    settings.composition.position.width  = INTERPOLATE(largeRect.width,smallRect.width,k+1,steps);
                    settings.composition.position.height = INTERPOLATE(largeRect.height,smallRect.height,k+1,steps);
                    NEXUS_SurfaceClient_SetSettings(g_app.mosaic[i].video_sc, &settings);
                    bgui_wait_for_move(g_app.gui);
                }
                NEXUS_SurfaceClient_GetSettings(g_app.mosaic[i].video_sc, &settings);
                settings.composition.zorder          = 0;
                NEXUS_SurfaceClient_SetSettings(g_app.mosaic[i].video_sc, &settings);

                if (b_check_timeout()) break;
            }
            if (b_check_timeout()) break;
        }
    }
    else if ( mosaic_move ) {
        for (i=0; i<g_app.num_mosaics ; i++) {
            set_move(i);
        }
        while(1) {
            for (i=0; i<g_app.num_mosaics ; i++) {
                move(i);
            }
            bgui_wait_for_move(g_app.gui);
            if (++fps.cnt % 100 == 0) {
                unsigned now = b_get_time();
                if (fps.time) {
                    BDBG_WRN(("%d fps", 1000*fps.cnt/(now-fps.time)));
                }
                fps.cnt = 0;
                fps.time = now;
            }
            if (b_check_timeout()) break;
        }
    }
    else {
        while (!g_app.done) {
            b_remote_key key;

            if (b_check_timeout()) break;

            if (binput_read_no_repeat(input, &key)) {
                binput_wait(input, 1000);
                continue;
            }
            switch (key) {
            case b_remote_key_stop:
            case b_remote_key_back:
            case b_remote_key_clear:
                g_app.done = true;
                break;
            case b_remote_key_right:
                if (++g_app.current == g_app.num_mosaics) g_app.current = 0;
                media_player_switch_audio(g_app.mosaic[g_app.current].player);
                break;
            case b_remote_key_left:
                g_app.current = (g_app.current == 0) ? g_app.num_mosaics-1 : g_app.current-1;
                media_player_switch_audio(g_app.mosaic[g_app.current].player);
                break;
            default:
                break;
            }
            if (gui) draw_gui(gui);
        }
    }

    for (i=0;i<g_app.num_mosaics;i++) {
        if (g_app.mosaic[i].started) {
            media_player_stop(g_app.mosaic[i].player);
        }
    }
    media_player_destroy_mosaics(player, g_app.num_mosaics);
    bgui_destroy(g_app.gui);
    binput_close(input);
    g_app.done = true;
    pthread_join(standby_thread_id, NULL);
    NxClient_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs input_router, playback and simple_decoder)!\n");
    return 0;
}
#endif
