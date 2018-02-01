/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "standby.h"
#include "util.h"

#include "bmedia_probe.h"
#include "bfile_stdio.h"


BDBG_MODULE(standby_priv);

B_StandbyNexusHandles g_StandbyNexusHandles;
B_DeviceState g_DeviceState;

void stop_decodes()
{
    unsigned i;

    for(i=0; i<MAX_CONTEXTS; i++) {
        stop_play_context(i);
        stop_live_context(i);
    }
}

static void *remote_key_monitor(void *context)
{
    while(!g_DeviceState.exit_app) {
        b_remote_key key;
        if (binput_read_no_repeat(g_StandbyNexusHandles.input, &key)) {
            binput_wait(g_StandbyNexusHandles.input, 1000);
            continue;
        }
        switch (key) {
        case b_remote_key_up:
            if (g_DeviceState.focused_button) {
                g_DeviceState.focused_button--;
            }
            else {
                g_DeviceState.focused_button = g_DeviceState.total_buttons-1;
            }
            update_menu();
            render_ui();
            break;
        case b_remote_key_down:
            if (++g_DeviceState.focused_button == g_DeviceState.total_buttons) {
                g_DeviceState.focused_button = 0;
            }
            update_menu();
            render_ui();
            break;
        case b_remote_key_select:
            switch (g_DeviceState.focused_button) {
                case 0:
                    g_DeviceState.power_mode = ePowerModeS1;
                    break;
                case 1:
                    g_DeviceState.power_mode = ePowerModeS2;
                    break;
                case 2:
                    g_DeviceState.power_mode = ePowerModeS3;
                    break;
                case 3:
                    g_DeviceState.power_mode = ePowerModeS5;
                    break;
                default:
                    break;
            }
            if(g_StandbyNexusHandles.event)
                BKNI_SetEvent(g_StandbyNexusHandles.event);
            break;
        case b_remote_key_one:
                g_DeviceState.power_mode = ePowerModeS1;
                g_DeviceState.focused_button = 0;
                update_menu();
                render_ui();
                if(g_StandbyNexusHandles.event)
                    BKNI_SetEvent(g_StandbyNexusHandles.event);
            break;
        case b_remote_key_two:
                g_DeviceState.power_mode = ePowerModeS2;
                g_DeviceState.focused_button = 1;
                update_menu();
                render_ui();
                if(g_StandbyNexusHandles.event)
                    BKNI_SetEvent(g_StandbyNexusHandles.event);
            break;
        case b_remote_key_three:
                g_DeviceState.power_mode = ePowerModeS3;
                g_DeviceState.focused_button = 2;
                update_menu();
                render_ui();
                if(g_StandbyNexusHandles.event)
                    BKNI_SetEvent(g_StandbyNexusHandles.event);
            break;
        case b_remote_key_five:
                g_DeviceState.power_mode = ePowerModeS5;
                g_DeviceState.focused_button = 3;
                update_menu();
                render_ui();
                if(g_StandbyNexusHandles.event)
                    BKNI_SetEvent(g_StandbyNexusHandles.event);
            break;
        case b_remote_key_stop:
        case b_remote_key_clear:
            g_DeviceState.exit_app = true;
            if(g_StandbyNexusHandles.event)
                BKNI_SetEvent(g_StandbyNexusHandles.event);
        case b_remote_key_power:
            if(g_StandbyNexusHandles.s1Event)
                BKNI_SetEvent(g_StandbyNexusHandles.s1Event);
            break;
        default:
            break;
        }
    }

    return NULL;
}

static const char *g_standby_state[] = {
    "ACTIVE STANDBY  (S1)",
    "PASSIVE STANDBY (S2)",
    "DEEP SLEEP WARM (S3)",
    "DEEP SLEEP COLD (S5)"
};

void update_menu()
{
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DFillSettings fillSettings;
    int rc;
    unsigned i;

    if (!g_StandbyNexusHandles.gui) return;

    BKNI_AcquireMutex(g_StandbyNexusHandles.ui_mutex);

    gfx = bgui_blitter(g_StandbyNexusHandles.gui);
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);

    if (g_StandbyNexusHandles.menuSurface) {
        for (i=0;i<g_DeviceState.total_buttons;i++) {
            bool focused = i == g_DeviceState.focused_button;

            fillSettings.surface = g_StandbyNexusHandles.menuSurface;
            fillSettings.rect.x = 0;
            fillSettings.rect.y = i*GUI_HEIGHT;
            fillSettings.rect.width = GUI_WIDTH;
            fillSettings.rect.height = GUI_HEIGHT;
            fillSettings.color = (i == g_DeviceState.focused_button) ? 0xFF00FF00 : 0xFF008888;
            rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
            BDBG_ASSERT(!rc);

            bgui_checkpoint(g_StandbyNexusHandles.gui);

            if (g_StandbyNexusHandles.font) {
                struct bfont_surface_desc desc;
                bfont_get_surface_desc(g_StandbyNexusHandles.menuSurface, &desc);
                bfont_draw_aligned_text(&desc, g_StandbyNexusHandles.font, &fillSettings.rect, g_standby_state[i], -1, focused?0xFF333333:0xFFCCCCCC, bfont_valign_center, bfont_halign_center);
                NEXUS_Surface_Flush(g_StandbyNexusHandles.menuSurface);
            }
        }
    }

    BKNI_ReleaseMutex(g_StandbyNexusHandles.ui_mutex);
}

void render_ui()
{
    NEXUS_Graphics2DHandle gfx;
    NEXUS_SurfaceHandle uiSurface;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_SurfaceCreateSettings uiSurfaceSettings;
    int rc;
    unsigned i;

    if (!g_StandbyNexusHandles.gui) return;

    BKNI_AcquireMutex(g_StandbyNexusHandles.ui_mutex);

    gfx = bgui_blitter(g_StandbyNexusHandles.gui);
    uiSurface = bgui_surface(g_StandbyNexusHandles.gui);
    NEXUS_Surface_GetCreateSettings(uiSurface, &uiSurfaceSettings);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = uiSurface;
    fillSettings.color = 0;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);


    if (g_StandbyNexusHandles.blitSurface) {
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface     = g_StandbyNexusHandles.blitSurface;
        blitSettings.output.surface     = uiSurface;
        blitSettings.output.rect        = g_DeviceState.blit_rect;
        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);
    }

    if (g_StandbyNexusHandles.picSurface) {
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface     = g_StandbyNexusHandles.picSurface;
        blitSettings.output.surface     = uiSurface;
        blitSettings.output.rect.x      = 3*uiSurfaceSettings.width/4 - PADDING;
        blitSettings.output.rect.y      = 3*uiSurfaceSettings.height/4 - PADDING;
        blitSettings.output.rect.width  = uiSurfaceSettings.width/4;
        blitSettings.output.rect.height = uiSurfaceSettings.height/4;
        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);
    }

    if (g_StandbyNexusHandles.menuSurface) {
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface     = g_StandbyNexusHandles.menuSurface;
        blitSettings.output.surface     = uiSurface;
        blitSettings.output.rect.x      = PADDING;
        blitSettings.output.rect.y      = PADDING;
        blitSettings.output.rect.width  = GUI_WIDTH;
        blitSettings.output.rect.height = GUI_HEIGHT*g_DeviceState.total_buttons;
        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);
    }

    bgui_checkpoint(g_StandbyNexusHandles.gui);
    bgui_submit(g_StandbyNexusHandles.gui);

    BKNI_ReleaseMutex(g_StandbyNexusHandles.ui_mutex);
}

void ui_create(void)
{
    struct bgui_settings gui_settings;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_DisplayCapabilities displayCap;

    NEXUS_GetDisplayCapabilities(&displayCap);

    g_DeviceState.total_buttons = sizeof(g_standby_state)/sizeof(g_standby_state[0]);
    bgui_get_default_settings(&gui_settings);
    gui_settings.width = displayCap.display[0].graphics.width;
    gui_settings.height = displayCap.display[0].graphics.height;
    gui_settings.display = g_StandbyNexusHandles.displayHD;
    g_StandbyNexusHandles.gui = bgui_create(&gui_settings);
    g_StandbyNexusHandles.font = bfont_open("arial_18_aa.bwin_font");

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = GUI_WIDTH;
    createSettings.height = GUI_HEIGHT*g_DeviceState.total_buttons;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    g_StandbyNexusHandles.menuSurface = NEXUS_Surface_Create(&createSettings);

    BKNI_CreateMutex(&g_StandbyNexusHandles.ui_mutex);
}

void ui_destroy(void)
{
    if (g_StandbyNexusHandles.ui_mutex)
        BKNI_DestroyMutex(g_StandbyNexusHandles.ui_mutex);

    if (g_StandbyNexusHandles.font)
        bfont_close(g_StandbyNexusHandles.font);

    if (g_StandbyNexusHandles.gui)
        bgui_destroy(g_StandbyNexusHandles.gui);
    g_StandbyNexusHandles.gui = NULL;

    if(g_StandbyNexusHandles.menuSurface)
        NEXUS_Surface_Destroy(g_StandbyNexusHandles.menuSurface);
    g_StandbyNexusHandles.menuSurface = NULL;
}

int start_app(void)
{
    NEXUS_DisplayCapabilities displayCap;
    NEXUS_VideoDecoderCapabilities videoCap;
    unsigned i;

    NEXUS_Platform_GetConfiguration(&g_StandbyNexusHandles.platformConfig);

    BKNI_CreateEvent(&g_StandbyNexusHandles.event);
    BKNI_CreateEvent(&g_StandbyNexusHandles.s1Event);
    BKNI_CreateEvent(&g_StandbyNexusHandles.signalLockedEvent);

    gpio_open();
    ir_open();
    uhf_open();
    /*keypad_open();*/

    NEXUS_GetDisplayCapabilities(&displayCap);
    if(displayCap.display[0].numVideoWindows)
        display_open(0);
    if(displayCap.display[1].numVideoWindows)
        display_open(1);

    NEXUS_GetVideoDecoderCapabilities(&videoCap);
    g_DeviceState.num_contexts = videoCap.numVideoDecoders<=MAX_CONTEXTS?videoCap.numVideoDecoders:MAX_CONTEXTS;

    for(i=0; i<g_DeviceState.num_contexts ; i++) {
        if(i < displayCap.display[0].numVideoWindows)
            window_open(i, 0); /* window on display 0 */
        if(i < displayCap.display[1].numVideoWindows)
            window_open(i, 1); /* window on display 1 */

        if(i < videoCap.numVideoDecoders)
            decoder_open(i);

        stc_channel_open(i);

        playback_open(i);
        record_open(i);

        g_StandbyNexusHandles.parserBand[i] = (i==0)?NEXUS_ParserBand_e0:NEXUS_ParserBand_e1;
    }

#if NEXUS_HAS_CEC && NEXUS_HAS_HDMI_OUTPUT
    cec_setup();
#endif

    if(g_DeviceState.gui) {
        ui_create();
        update_menu();
        render_ui();
    }

    pthread_create(&g_StandbyNexusHandles.remote_thread, NULL, remote_key_monitor, NULL);

    return NEXUS_SUCCESS;
}

void stop_app(void)
{
    unsigned i;

    g_DeviceState.exit_app = true;

    if(g_StandbyNexusHandles.remote_thread)
        pthread_join(g_StandbyNexusHandles.remote_thread, NULL);

    /* Bring down system */
    picture_decode_stop();
    picture_decoder_close();

    graphics2d_stop();
    graphics2d_destroy();

    encode_stop(0);
    encoder_close(0);

    ui_destroy();

    for(i=0; i<g_DeviceState.num_contexts; i++) {
        decode_stop(i);

#if NEXUS_HAS_FRONTEND
        untune_frontend(i);
#endif

        playback_stop(i);
        record_stop(i);

        playback_close(i);
        record_close(i);

        stc_channel_close(i);

        decoder_close(i);

        window_close(i, 0); /* window on display 0 */
        window_close(i, 1); /* window on display 1 */
    }

    display_close(0);
    display_close(1);

    /*keypad_close();*/
    uhf_close();
    ir_close();
    gpio_close();

    if(g_StandbyNexusHandles.event)
        BKNI_DestroyEvent(g_StandbyNexusHandles.event);

    if(g_StandbyNexusHandles.s1Event)
        BKNI_DestroyEvent(g_StandbyNexusHandles.s1Event);

    if(g_StandbyNexusHandles.signalLockedEvent)
        BKNI_DestroyEvent(g_StandbyNexusHandles.signalLockedEvent);
}
