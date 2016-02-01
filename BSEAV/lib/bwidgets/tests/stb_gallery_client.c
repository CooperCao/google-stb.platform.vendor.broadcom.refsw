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
#include "b_app_client.h"
#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>

BDBG_MODULE(stb_gallery_client);

static int cnt = 0;
static char time_buffer[64];
static char counter_buffer[64];
bwidget_t background, time_label, counter_label;
struct b_offscreen_status offscreen_status;
int g_client_fd;

void idle_callback(bwidget_engine_t engine);
static void sync_fb(void *context);

int main(void)
{
    bbutton_settings button_settings;
    blabel_settings label_settings;
    bwidget_engine_settings widget_engine_settings;
    bwidget_engine_t widget_engine;
    NEXUS_Error rc;

    NEXUS_Platform_Join();

    rc = b_init_offscreen(640, 480, 640, 480, &offscreen_status);
    BDBG_ASSERT(!rc);
    offscreen_status.sync = sync_fb;
    offscreen_status.context = &offscreen_status;

    widget_engine_settings.idle = idle_callback; /* we'll process user input at idle time */
    widget_engine = bwidget_engine_open(offscreen_status.win_engine, &widget_engine_settings);

    bbutton_get_default_settings(&button_settings);
    button_settings.label.widget.window.x = 0;
    button_settings.label.widget.window.y = 0;
    button_settings.label.widget.window.rect.width = 720;
    button_settings.label.widget.window.rect.height = 480;
    button_settings.label.widget.window.parent = offscreen_status.win_framebuffer_window;
    button_settings.label.font = offscreen_status.default_font;
    background = bbutton_create(widget_engine, &button_settings);

    blabel_get_default_settings(&label_settings);
    label_settings.widget.window.x = 150;
    label_settings.widget.window.y = 80;
    label_settings.widget.window.rect.width = 180;
    label_settings.widget.window.rect.height = 40;
    label_settings.widget.window.parent = bwidget_win(background);
    label_settings.text = counter_buffer;
    label_settings.background_color = 0xFa006020;
    label_settings.bevel = 1;
    counter_buffer[0] = 0;
    label_settings.font = offscreen_status.default_font;
    counter_label = blabel_create(widget_engine, &label_settings);

    label_settings.widget.window.x += 200;
    label_settings.text = time_buffer;
    time_buffer[0] = 0;
    label_settings.font = offscreen_status.default_font;
    time_label = blabel_create(widget_engine, &label_settings);

    /* display a picture. because it's outside of the fast update region, it doesn't affect performance */
    blabel_get_default_settings(&label_settings);
    label_settings.widget.window.x = 150;
    label_settings.widget.window.y = 140;
    label_settings.widget.window.rect.width = 380;
    label_settings.widget.window.rect.height = 300;
    label_settings.widget.window.parent = bwidget_win(background);
    label_settings.image = bwin_image_load(offscreen_status.win_engine, "/mnt/nfs/erickson/media/server_images/pic2.jpg");
    blabel_create(widget_engine, &label_settings);

    g_client_fd = b_connect_to_server();
    BDBG_ASSERT(g_client_fd >= 0);

    {
        b_message msg;
        b_get_surface_data(&offscreen_status, &msg);
        rc = b_send_message(g_client_fd, &msg);
        BDBG_ASSERT(!rc);
    }

    /* add ability for this app to interrupt the bwin event sleep */
    bwin_add_io_handle(offscreen_status.win_engine, (bwin_io_handle)2);
    bwidget_run(widget_engine);

    b_disconnect_from_server(g_client_fd);

    b_uninit_offscreen(&offscreen_status);
    NEXUS_Platform_Uninit();

    return 0;
}

void idle_callback(bwidget_engine_t engine)
{
    struct timeval tv;
    static unsigned start_sec = 0;

    BSTD_UNUSED(engine);

    gettimeofday(&tv, NULL);
    if (!start_sec) {
        start_sec = tv.tv_sec;
    }
    tv.tv_sec -= start_sec;
    snprintf(time_buffer, 64, "Time: %ld:%02ld", tv.tv_sec/60, tv.tv_sec%60);
    bwin_repaint(bwidget_win(time_label), NULL);
    snprintf(counter_buffer, 64, "Count: %d", ++cnt);
    bwin_repaint(bwidget_win(counter_label), NULL);

    /* wake up bwin right away again. the client is going to go as fast as possible.
    we could slow it down so that every new frame is visible (not faster than the display refresh rate), but that's
    not the point of this app. */
    bwin_trigger_io(offscreen_status.win_engine, (bwin_io_handle)2);
}

static void sync_fb(void *context)
{
    b_message msg;
    struct b_offscreen_status *status = context;

    /* tell server that client's offscreen surface is ready */
    b_message_init(&msg);
    msg.type = b_message_type_offscreen_ready;
    ((struct b_message_index *)msg.data)->index = status->current_back_surface;
    msg.datasize = sizeof(struct b_message_index);
    b_send_message(g_client_fd, &msg);

    /* we are double-buffered, so we could offset our wait by one. but for this app it doesn't matter because
    the client is going AFAP. it might matter if the server waited to respond until the display vsync. */
    while (1) {
        int rc = b_recv_message(g_client_fd, &msg);
        if (rc) break; /* failure */

        if (msg.type == b_message_type_offscreen_done) {
            /* got msg */
            break;
        }
    }
}
