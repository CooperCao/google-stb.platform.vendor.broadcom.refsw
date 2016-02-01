/******************************************************************************
 * (c) 2004-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "bwidgets.h"
#include "nexus_platform.h"
#include "b_app_server.h"
#include "b_app_client.h"
#include "nexus_core_utils.h"
#include "nexus_ir_input.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <errno.h>

BDBG_MODULE(stb_server);

#define TOTAL_BUTTONS 3
bwidget_t sidebar, button[TOTAL_BUTTONS];
NEXUS_IrInputHandle g_irInput;
struct b_server_status server_status;
struct b_offscreen_status offscreen_status;
b_client_t g_current_client = NULL;

void user_input(void *context);
void idle_callback(bwidget_engine_t engine);
void click(bwidget_t widget);
int b_server_blit(const struct b_offscreen_status *status);
static void sync_fb(void *context);

static void setevent_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

void client_changed(void *context, b_client_t client)
{
    BSTD_UNUSED(context);
    if (!client) return;

    /* automatically switch clients if a new one comes w/ the same time. this allows for restart of client w/o touching the server. */
    if (g_current_client && g_current_client != client && client->type == g_current_client->type) {
        b_release_client(g_current_client, false);
        g_current_client = client;
    }
    if (client->fd == -1) {
        return;
    }

    /* has this client reported a surface? */
    if (client->current_surface != -1 ) {
        /* if current, we can reblit */
        BDBG_MSG(("client_changed"));
        if (client == g_current_client) {
            b_server_blit(&offscreen_status);
            bwin_trigger_io(offscreen_status.win_engine, (bwin_io_handle)2);
        }
        else {
            /* we're not going to use this client's surface, so let it go */
            b_complete_client(client);
        }
    }
}

int main(void)
{
    blabel_settings label_settings;
    bbutton_settings button_settings;
    bwidget_engine_settings widget_engine_settings;
    bwidget_engine_t widget_engine;
    unsigned y;
    NEXUS_VideoFormat format = NEXUS_VideoFormat_eNtsc;
    NEXUS_VideoFormatInfo videoFormatInfo;
    NEXUS_Error rc;
    NEXUS_IrInputSettings irInputSettings;
    BKNI_EventHandle irinput_event;

    NEXUS_Platform_Init(NULL);

    rc = b_init_server(format, &server_status);
    BDBG_ASSERT(!rc);
    server_status.client_changed = client_changed;

    NEXUS_VideoFormat_GetInfo(format, &videoFormatInfo);

    rc = b_init_offscreen(720, 480, server_status.graphics.width, server_status.graphics.height, &offscreen_status);
    BDBG_ASSERT(!rc);

    offscreen_status.sync = sync_fb;
    offscreen_status.context = &offscreen_status;
    offscreen_status.server = &server_status; /* allows offscreen to be blitted to the fb */

    widget_engine_settings.idle = idle_callback; /* we'll process user input at idle time */
    widget_engine = bwidget_engine_open(offscreen_status.win_engine, &widget_engine_settings);

    blabel_get_default_settings(&label_settings);
    label_settings.widget.window.x = 0;
    label_settings.widget.window.y = 0;
    label_settings.widget.window.rect.width = 120;
    label_settings.widget.window.rect.height = 480;
    label_settings.widget.window.parent = offscreen_status.win_framebuffer_window;
    label_settings.image = bwin_image_load(offscreen_status.win_engine, "/mnt/nfs/erickson/media/server_images/metal_background.jpg");
    label_settings.render_mode = bwin_image_render_mode_tile;
    sidebar = blabel_create(widget_engine, &label_settings);

    y = 30;
    bbutton_get_default_settings(&button_settings);
    button_settings.label.widget.window.x = 25;
    button_settings.label.widget.window.y = y;
    button_settings.label.widget.window.rect.width = 80;
    button_settings.label.widget.window.rect.height = 80;
    button_settings.label.widget.window.parent = bwidget_win(sidebar);
    button_settings.label.image = bwin_image_load(offscreen_status.win_engine, "/mnt/nfs/erickson/media/server_images/play.jpg");
    button_settings.click = click;
    button[0] = bbutton_create(widget_engine, &button_settings);
    y += 90;

    button_settings.label.widget.window.y = y;
    button_settings.label.image = bwin_image_load(offscreen_status.win_engine, "/mnt/nfs/erickson/media/server_images/gallery.jpg");
    button[1] = bbutton_create(widget_engine, &button_settings);
    y += 90;

    button_settings.label.widget.window.y = y;
    button_settings.label.image = bwin_image_load(offscreen_status.win_engine, "/mnt/nfs/erickson/media/server_images/internet.jpg");
    button[2] = bbutton_create(widget_engine, &button_settings);

    bwidget_set_focus(button[0]);

    BKNI_CreateEvent(&irinput_event);
    NEXUS_IrInput_GetDefaultSettings(&irInputSettings);
    irInputSettings.dataReady.callback = setevent_callback;
    irInputSettings.dataReady.context = irinput_event;
    g_irInput = NEXUS_IrInput_Open(0, &irInputSettings);
    bwin_add_io_handle(offscreen_status.win_engine, (bwin_io_handle)1);

    rc = b_start_server(&server_status);
    BDBG_ASSERT(!rc);
    bwin_add_io_handle(offscreen_status.win_engine, (bwin_io_handle)2);

    /* This is the main event loop for the application. It processes bwin
    events and redraws the widgets. You're done. */
    printf("server gui running\n");
    bwidget_run(widget_engine);

    b_stop_server();

    b_uninit_offscreen(&offscreen_status);
    b_uninit_server(&server_status);
    NEXUS_Platform_Uninit();

    return 0;
}

#include <sys/time.h>
unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int b_server_blit(const struct b_offscreen_status *status)
{
    int rc;

    offscreen_status.update_rect.x = 120;
    offscreen_status.update_rect.y = 0;
    offscreen_status.update_rect.width = 600;
    offscreen_status.update_rect.height = 480;

    if (g_current_client &&
        g_current_client->fd != -1 &&
        g_current_client->current_surface != -1 &&
        g_current_client->surface[g_current_client->current_surface])
    {
        NEXUS_Graphics2DBlitSettings blitSettings;

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = g_current_client->surface[g_current_client->current_surface];
        blitSettings.output.surface = status->surface;
        blitSettings.output.rect = offscreen_status.update_rect;
        rc = NEXUS_Graphics2D_Blit(status->blitter, &blitSettings);
        BDBG_ASSERT(!rc);

        /* TODO: it's faster to checkpoint here and allow the client to proceed than to wait for the server to complete its processing */
        checkpoint(status);
        b_complete_client(g_current_client);
    }
    else {
        NEXUS_Graphics2DFillSettings fillSettings;

        /* no current client */
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = status->surface;
        fillSettings.rect = offscreen_status.update_rect;
        fillSettings.color = 0;
        rc = NEXUS_Graphics2D_Fill(status->blitter, &fillSettings);
        BDBG_ASSERT(!rc);
    }



    return 0;
}

/* TODO: connect to NEXUS_IrInput, may need internal bwin synchronization to keep app simple */
void user_input(void *context)
{
    bwin_engine_t win_engine = (bwin_engine_t)context;
    /* interrupt the bwin event loop. this will interrupt the sleep and cause the ui to be more responsive. */
    bwin_trigger_io(win_engine, (bwin_io_handle)1);
}

void idle_callback(bwidget_engine_t engine)
{
    NEXUS_Error rc;
    NEXUS_IrInputEvent irEvent;
    unsigned num;
    bool overflow;

    BSTD_UNUSED(engine);

    rc = NEXUS_IrInput_GetEvents(g_irInput, &irEvent, 1, &num, &overflow);
    if (!rc && num && !irEvent.repeat) {
        printf("ir event: %#x\n", irEvent.code);
        switch (irEvent.code) {
        case 0x8035: /* down */
            bwidget_enter_key(engine, bwidget_key_down, true);
            break;
        case 0x9034: /* up */
            bwidget_enter_key(engine, bwidget_key_up, true);
            break;
        case 0x6037: /* right */
            bwidget_enter_key(engine, bwidget_key_right, true);
            break;
        case 0x7036: /* left */
            bwidget_enter_key(engine, bwidget_key_left, true);
            break;
        case 0xe011: /* select */
            bwidget_enter_key(engine, bwidget_key_select, true);
            break;
        }
    }

    if (offscreen_status.update_rect.width) {
        bwin_rect update_rect;
        update_rect.x = offscreen_status.update_rect.x;
        update_rect.y = offscreen_status.update_rect.y;
        update_rect.width = offscreen_status.update_rect.width;
        update_rect.height = offscreen_status.update_rect.height;
        bwin_repaint(offscreen_status.win_framebuffer_window, &update_rect);
        offscreen_status.update_rect.width = 0;
    }
}

void click(bwidget_t widget)
{
    if (widget == button[0]) {
        printf("click 0\n");
        if (g_current_client) {
            b_release_client(g_current_client, false);
        }
        g_current_client = b_get_client(0);
        b_server_blit(&offscreen_status);
        bwin_repaint(offscreen_status.win_framebuffer_window, NULL /* client_rect */);
    }
    else if (widget == button[1]) {
        printf("click 1\n");
        if (g_current_client) {
            b_release_client(g_current_client, false);
        }
        g_current_client = b_get_client(1);
        b_server_blit(&offscreen_status);
        bwin_repaint(offscreen_status.win_framebuffer_window, NULL /* client_rect */);
    }
    else if (widget == button[2]) {
        printf("click 2\n");
        if (g_current_client) {
            b_release_client(g_current_client, false);
        }
        g_current_client = b_get_client(2);
        b_server_blit(&offscreen_status);
        bwin_repaint(offscreen_status.win_framebuffer_window, NULL /* client_rect */);
    }
}

static void sync_fb(void *context)
{
    BSTD_UNUSED(context);
}
