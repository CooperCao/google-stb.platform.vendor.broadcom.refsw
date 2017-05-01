/******************************************************************************
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
 *****************************************************************************/

#ifndef BWIDGETS_H__
#define BWIDGETS_H__

#include "bwin.h"
#include "bwin_font.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=****************************
Basic design principles:

- bwin is exposed in the bwidgets api. To resize a widget, just resize the bwin_t.
- Quasi-inheritance is accomplished using nested structures, just like an
    actual C++ implementation. The base structure must always be first so that
    typecasting is possible.
- Events are implemented using a standard bwidget_callback function pointer
    and a bwidget_t handle parameter.
- There is a default draw operation which can be overridden with the
    bwidget_draw function pointer.
- Instead of providing every possible GUI option to these widgets, we'll keep
    them very small and simple, and allow user customization of the draw operations.
- There are no globals. bwidget_engine_t allows multiple instances of the widget
    set.
- All colors are ARGB8888 values, which is the bwin format.

*******************************/

typedef struct bwidget_engine *bwidget_engine_t;
typedef struct bwidget *bwidget_t;

/**
Callbacks
**/

/* Event from a widget */
typedef void (*bwidget_callback)(bwidget_t widget);

/* Event from the engine (e.g. idle time) */
typedef void (*bwidget_engine_callback)(bwidget_engine_t widget_engine, void * context, int param);

/* Event from an io_handle which interrupted bwin (e.g. user input )*/
typedef void (*bwidget_io_callback)(bwidget_engine_t engine, bwin_io_handle handle);

/* Drawing operation for application overrides */
typedef void (*bwidget_draw)(bwidget_t widget, const bwin_rect *cliprect);

/**********************************
* bwidget_engine functions
*/

typedef struct {
    bwidget_engine_callback   idle; /* idle time callback */
    void                    * context;
    int                       param;
} bwidget_engine_settings;

bwidget_engine_t
bwidget_engine_open(bwin_engine_t win_engine,
    const bwidget_engine_settings *settings);

void
bwidget_engine_close(bwidget_engine_t engine);

bwidget_t
bwidget_get_focus(bwidget_engine_t engine);

/* Run the main message loop */
int
bwidget_run(bwidget_engine_t engine);

/* Process a single message from the message loop */
int
bwidget_process_event(bwidget_engine_t engine);

/* Process a single message from the message loop (with option to do idle loop processing) */
int
bwidget_process_event2(bwidget_engine_t engine, bool idle_handling);

/* Terminate the message loop */
void
bwidget_terminate(bwidget_engine_t engine);

void
bwidget_add_io_handle(bwidget_engine_t engine, bwin_io_handle handle,
    bwidget_io_callback io_callback);

void
bwidget_remove_io_handle(bwidget_engine_t engine, bwin_io_handle handle);

void
bwidget_trigger_io(bwidget_engine_t engine, bwin_io_handle handle);

/***********************************
* user input
**/

/* These codes are recognized as input.
Also, ascii codes from 0x20 up to 0x7F are recognized as text input.
*/
typedef enum {
    bwidget_key_invalid,
    bwidget_key_select,
    bwidget_key_up,
    bwidget_key_down,
    bwidget_key_left,
    bwidget_key_right,
    bwidget_key_backspace, /* delete to the left */
    bwidget_key_delete, /* delete from from the right */
    bwidget_key_chup,
    bwidget_key_chdown,
    bwidget_key_volup,
    bwidget_key_voldown,
    bwidget_key_mute,
    bwidget_key_play,
    bwidget_key_stop,
    bwidget_key_pause,
    bwidget_key_rewind,
    bwidget_key_fastforward,
    bwidget_key_record,
    bwidget_key_menu,
    bwidget_key_info,
    bwidget_key_exit,
    bwidget_key_dot,
    bwidget_key_enter,
    bwidget_key_last,
    bwidget_key_pip,
    bwidget_key_swap,
    bwidget_key_jumpforward,
    bwidget_key_jumpreverse,
    bwidget_key_power,
    /* asciii codes */
    bwidget_key_0          = 0x30,
    bwidget_key_1,
    bwidget_key_2,
    bwidget_key_3,
    bwidget_key_4,
    bwidget_key_5,
    bwidget_key_6,
    bwidget_key_7,
    bwidget_key_8,
    bwidget_key_9,
    bwidget_key_max
} bwidget_key;

/* Event from key press */
typedef int (*bwidget_keydown)(bwidget_t widget, const bwidget_key key);

/* Returns 0 if the key was consumed */
int
bwidget_enter_key(bwidget_engine_t engine,
    bwidget_key key, /* the key that is being pressed */
    bool down /* true for keydown, false for keyup */
    );

/**********************************
* bwidget base functions
*/

typedef struct {
    bwin_settings window;
    bwin_t win; /* read-only */

    bool focusable;
    bool enabled;

    bwidget_callback focus; /* gain focus */
    bwidget_callback blur; /* lose focus */

    bwidget_draw draw; /* draw has a default implementation which can be overridden. */
    bwidget_keydown key_down; /* key processing callback */
    void * data; /* optional user data */
} bwidget_settings;

void bwidget_get_default_settings(bwidget_settings *settings);
bwidget_t bwidget_create(bwidget_engine_t engine, const bwidget_settings *settings);
void bwidget_get_settings(bwidget_t widget, bwidget_settings *settings);
int bwidget_set_settings(bwidget_t widget, const bwidget_settings *settings);
void bwidget_destroy(bwidget_t widget);

bwidget_t bwidget_get_parent(bwidget_t widget);
bwin_t bwidget_win(bwidget_t widget);
bwin_engine_t bwidget_win_engine(bwidget_engine_t widget_engine);

/**********************************
* bwidget functions
*/

/* returns 0 if focus was set */
int
bwidget_move_focus(bwidget_engine_t engine,
    int vertical, /* -1, 0, or 1 */
    int horizontal /* -1, 0, or 1 */
    );

void
bwidget_set_modal(bwidget_engine_t engine, bwidget_t widget);

bool
bwidget_is_modal(bwidget_engine_t engine, bwidget_t widget);

/* returns 0 if focus was set */
int
bwidget_set_focus(bwidget_t widget);

bwidget_t
bwidget_get_focus(bwidget_engine_t engine);

bool
bwidget_is_parent(bwidget_t widget, bwidget_t parent);

void
bwidget_show(bwidget_t widget, bool show);

bool
bwidget_visible(bwidget_t widget);


/**********************************
* bwidget text justification
*/

typedef enum {
    bwidget_justify_vert_top,
    bwidget_justify_vert_middle,
    bwidget_justify_vert_bottom,
    bwidget_justify_vert_max
} bwidget_justify_vert;

typedef enum {
    bwidget_justify_horiz_left,
    bwidget_justify_horiz_center,
    bwidget_justify_horiz_right,
    bwidget_justify_horiz_max
} bwidget_justify_horiz;

/**********************************
* bwidget label
*/

typedef struct {
    bwidget_settings widget;
    const char *text; /* only the pointer is stored. the application must guarantee that
        the memory is preserved during the life of the widget */
    bwin_font_t font;
    uint32_t text_color;
    bwidget_justify_vert text_justify_vert;
    bwidget_justify_horiz text_justify_horiz;
    uint32_t text_margin;

    /* image */
    bwin_framebuffer_t framebuffer;
    bwin_image_t image;
    bwin_image_render_mode render_mode;

    /* drawing (if image == NULL) */
    bool     background_color_disable;
    uint32_t background_color;
    uint32_t focus_color;
    uint32_t bevel_color[4]; /* top, right, bottom, left */
    int bevel;
} blabel_settings;

void blabel_get_default_settings(blabel_settings *settings);
bwidget_t blabel_create(bwidget_engine_t engine, const blabel_settings *settings);
void blabel_get(bwidget_t widget, blabel_settings *settings);
int blabel_set(bwidget_t widget, const blabel_settings *settings);

/**********************************
* bwidget button
*/

typedef struct {
    blabel_settings label;
    bool down;
    bwidget_callback click;
} bbutton_settings;

void bbutton_get_default_settings(bbutton_settings *settings);
bwidget_t bbutton_create(bwidget_engine_t engine, const bbutton_settings *settings);
void bbutton_get(bwidget_t widget, bbutton_settings *settings);
int bbutton_set(bwidget_t widget, const bbutton_settings *settings);
void bbutton_click(bwidget_t widget);

#ifdef __cplusplus
}
#endif

#endif /* BWIDGETS_H__*/
