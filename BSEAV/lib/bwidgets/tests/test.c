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
#include "b_app_server.h"
#include "b_app_client.h"
#include "nexus_platform.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

bwidget_t button1, button2, button3;

void user_input(void *context);
void idle_callback(bwidget_engine_t engine);

int main(void)
{
    bbutton_settings button_settings;
    bwidget_engine_settings widget_engine_settings;
    bwidget_engine_t widget_engine;
    NEXUS_Error rc;
    struct b_server_status server_status;
    struct b_offscreen_status offscreen_status;

    NEXUS_Platform_Init(NULL);

    rc = b_init_server(NEXUS_VideoFormat_eNtsc, &server_status);
    BDBG_ASSERT(!rc);

    rc = b_init_offscreen(720, 480, server_status.graphics.width, server_status.graphics.height, &offscreen_status);
    BDBG_ASSERT(!rc);
    offscreen_status.server = &server_status;

    printf("open widget_engine\n");
    widget_engine_settings.idle = idle_callback; /* we'll process user input at idle time */
    widget_engine = bwidget_engine_open(offscreen_status.win_engine, &widget_engine_settings);

    printf("create button1 in bwidgets\n");
    bbutton_get_default_settings(&button_settings);
    button_settings.label.widget.window.x = 50;
    button_settings.label.widget.window.y = 50;
    button_settings.label.widget.window.rect.width = 200;
    button_settings.label.widget.window.rect.height = 70;
    button_settings.label.widget.window.parent = offscreen_status.win_framebuffer_window;
    button_settings.label.text = "Hello World";
    button_settings.label.font = offscreen_status.default_font;
    button1 = bbutton_create(widget_engine, &button_settings);

    printf("create button2 in bwidgets\n");
    bbutton_get_default_settings(&button_settings);
    button_settings.label.widget.window.x = 300;
    button_settings.label.widget.window.y = 50;
    button_settings.label.widget.window.rect.width = 200;
    button_settings.label.widget.window.rect.height = 70;
    button_settings.label.widget.window.parent = offscreen_status.win_framebuffer_window;
    button_settings.label.text = "Button 2";
    button_settings.label.font = offscreen_status.default_font;
    button2 = bbutton_create(widget_engine, &button_settings);

    printf("create button3 in bwidgets\n");
    bbutton_get_default_settings(&button_settings);
    button_settings.label.widget.window.x = 50;
    button_settings.label.widget.window.y = 130;
    button_settings.label.widget.window.rect.width = 200;
    button_settings.label.widget.window.rect.height = 70;
    button_settings.label.widget.window.parent = offscreen_status.win_framebuffer_window;
    button_settings.label.image = bwin_image_load(offscreen_status.win_engine, "pictures/sunset.jpg");
    button_settings.label.text = "Button 2";
    button_settings.label.text_color = 0xFF000000;
    button_settings.label.font = offscreen_status.default_font;
    button3 = bbutton_create(widget_engine, &button_settings);

    printf("set focus\n");
    bwidget_set_focus(button3);

    bwin_add_io_handle(offscreen_status.win_engine, (bwin_io_handle)1);

    /* This is the main event loop for the application. It processes bwin
    events and redraws the widgets. You're done. */
    printf("run the gui\n");
    bwidget_run(widget_engine);

    b_uninit_offscreen(&offscreen_status);
    b_uninit_server(&server_status);
    NEXUS_Platform_Uninit();

    return 0;
}

/* settop api callback */
void user_input(void *context)
{
    bwin_engine_t win_engine = (bwin_engine_t)context;
    /* interrupt the bwin event loop. this will interrupt the sleep and
    cause the ui to be more responsive. */
    bwin_trigger_io(win_engine, (bwin_io_handle)1);
}

void idle_callback(bwidget_engine_t engine)
{
    BSTD_UNUSED(engine);
}
