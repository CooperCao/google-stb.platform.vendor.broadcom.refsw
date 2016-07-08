/******************************************************************************
 * (c) 2004-2016 Broadcom Corporation
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

#ifndef BWIN_H__
#define BWIN_H__

#include <bstd.h>
#include "bwin_rect.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=*********************************************
Bwin is a basic windowing system for embedded systems.

It supports the following:

o Multiple instances of the windowing system (no global variables)
o Multiple framebuffers with different pixel formats.
o Antialiased true-type fonts through Freetype2.
o PNG and JPEG rendering.
o Hardware acceleration through user-overrideable drawing primitives.
o Multiple pixel formats supported at the same time (it doesn't require you to pick
  one pixel format at compile time).
o Complete windowing support, including parent/child relationships, zorder among siblings,
  clipping to a window and to the framebuffer, consolidation of repaint events, etc.
o Opaque or transparent windows. A transparent window requires that a parent be redrawn
  even if a child completely covers an updated area. An opaque window can just draw
  the child.
o UTF8 encoded text (TODO)

It does not support the following:

o Keyboard, mouse or any predefined input device. Instead, you can plug in
  any number of i/o devices and process them outside of the windowing system.
o Tabbing
o Window managers (standard window decorations, etc.)
o GIF rendering
o Client/server functionality.

See iolaus/docs/requirements.txt for a complete requirements document.

************************************************/

/**
Summary:
The event engine for the windowing system.
Description:
You must create an engine to manage a set of framebuffers.
**/
typedef struct bwin_engine *bwin_engine_t;

/**
Summary:
A framebuffer is a single graphics surface on which windows are mapped.
**/
typedef struct bwin_framebuffer *bwin_framebuffer_t;

/**
Summary:
A window is a virtual drawing area which is clipped by other windows.
Description:
You can never draw outside of a window.
The windowing system tells you when you need to draw into your window (through an event).
**/
typedef struct bwin *bwin_t;

/**
Summary:
Initial engine settings which are required for bwin_engine_open.
**/
typedef struct {
    /* no settings right now */
    int dummy;
} bwin_engine_settings;

/**
Summary:
Required to initialize bwin_engine_settings.
Description:
This allows the structure to grow in the future with no required code change.
**/
void bwin_engine_settings_init(bwin_engine_settings *settings);

/**
Summary:
Open a new window engine.
Description:
Each window engine can manage one or more framebuffers. You can create more than one
window engine for an application (for a dual output system, for instance).
**/
bwin_engine_t bwin_open_engine(const bwin_engine_settings *settings);

/**
Summary:
Close the window engine.
Description:
All windows and framebuffers should already have been closed.
**/
void bwin_close_engine(bwin_engine_t win);

/**
Summary:
Handle for adding I/O events to the main event loop.
Description:
The code value of bwin_io_handle is any non-zero user-defined number.
**/
typedef unsigned long bwin_io_handle;

/**
Summary:
Add an I/O handle to be polled in the bwin_engine_t event loop.
**/
void bwin_add_io_handle(bwin_engine_t win, bwin_io_handle handle);

/**
Summary:
Remove an I/O handle from the bwin_engine_t event loop.
**/
void bwin_remove_io_handle(bwin_engine_t win, bwin_io_handle handle);

/**
Summary:
Trigger an I/O event.
Description:
This function can be called safely from any context and is re-entrant
with any other bwin function.
**/
void bwin_trigger_io(bwin_engine_t win, bwin_io_handle handle);

/**
Summary:
**/
typedef enum {
    bwin_event_type_paint,
    bwin_event_type_io
} bwin_event_type;

/**
Summary:
Data which is passed back with an event.
**/
typedef struct {
    bwin_event_type type;
    bwin_t window;

    struct {
        bwin_rect rect;
    } paint;
    struct {
        bwin_io_handle handle;
    } io;
} bwin_event;

/**
Summary:
Wait for the next event to be generated.
Description:
If no event is generated within the time specified by timeout, it returns non-zero. This
can be used as idle time by the application.
**/
int bwin_get_event(bwin_engine_t win, bwin_event *event, int timeout);

/**
Summary:
Check if there is an event waiting, but do not consume the event and do not block if there
is no event.
**/
int bwin_peek_event(bwin_engine_t win, bwin_event *event);

/**
Summary:
Pixel formats supported by the drawing operations.
Description:
All pixel formats are in native CPU endianness.
There is no bwin api to set the palette. This is assume to be an operation
that will be handled directly with the framebuffer, outside of bwin.
**/
typedef enum {
    bwin_pixel_format_r5_g6_b5,
    bwin_pixel_format_a8_r8_g8_b8,
    bwin_pixel_format_a1_r5_g5_b5,
    bwin_pixel_format_palette8
} bwin_pixel_format;

/**
Summary:
Returns bits per pixel for the format.
**/
int bwin_get_bpp(bwin_pixel_format pixel_format);

/**
Summary:
Set of operations for drawing into the framebuffer.
Description:
bwin_framebuffer_settings_init will return an instance of bwin_drawing_operations
with default CPU-based drawing operations. You don't need to modify this structure.

If you want to use hardware acceleration, you can assign your own function.

All coordinates are global. Color has already been modified for the pixel_format.
With the exception of ellipse operations, the coordinates have already been clipped and
are guaranteed to be safe. Ellipses must be clipped in the drawing primitive, and if
you disregard the clipping, you may write into unallocated memory.
**/
typedef struct {
    void (*sync)(bwin_framebuffer_t fb);    /* optional call to process a framebuffer after
                                        a series of paint commands has finished. Examples of what
                                        may happen here include flushing cached memory, flipping
                                        a double-buffer, or blitting to a framebuffer. */
    void (*draw_point)(bwin_framebuffer_t fb, int x, int y,
        uint32_t color, uint32_t generic_color);
    void (*draw_line)(bwin_framebuffer_t fb, int x1, int y1, int x2, int y2,
        uint32_t color, uint32_t generic_color);
    void (*fill_rect)(bwin_framebuffer_t fb, const bwin_rect *rect,
        uint32_t color, uint32_t generic_color);
    unsigned fill_rect_threshold; /* if width * height of fill is < fill_rect_threshold, use the internal CPU fill.
        if fill_rect_threshold is 0, bwin_drawing_operations.fill_rect will always be called.
        optimal value depends on platform. */
    void (*copy_rect)(bwin_framebuffer_t destfb, const bwin_rect *destrect,
        bwin_framebuffer_t srcfb, const bwin_rect *srcrect);
    void (*blit)(bwin_framebuffer_t destfb, const bwin_rect *destrect,
        const uint32_t operation,
        bwin_framebuffer_t srcfb1, const bwin_rect *srcrect1,
        bwin_framebuffer_t srcfb2, const bwin_rect *srcrect2,
        const uint32_t pixel1, const uint32_t pixel2);
    void (*draw_ellipse)(bwin_framebuffer_t fb, int x, int y, unsigned rx, unsigned ry,
        uint32_t color, uint32_t generic_color,
        const bwin_rect *cliprect);
    void (*fill_ellipse)(bwin_framebuffer_t fb, int x, int y, unsigned rx, unsigned ry,
        uint32_t color, uint32_t generic_color,
        const bwin_rect *cliprect);
    void (*device_bitmap_create)(bwin_framebuffer_t fb, void **new_buffer_ptr, void **new_data_ptr);
    void (*device_bitmap_destroy)(bwin_framebuffer_t fb);
} bwin_drawing_operations;

/**
Summary:
Settings to describe the framebuffer.
Description:
The caller must create the framebuffer, then use bwin_framebuffer_settings to report
its settings to iolaus.
**/
typedef struct {
    void *buffer;   /* pointer to first pixel */
    void *second_buffer;    /* pointer to first pixel of 2nd buffer. If you are
                    using double buffering, you should set this. Otherwise
                    set it to NULL. bwin will swap pointers after calling
                    bwin_sync.*/
    unsigned pitch; /* length of memory per line in bytes */
    unsigned width; /* width of visible line in s */
    unsigned height; /* total number of lines */
    bwin_pixel_format pixel_format;
    bwin_drawing_operations drawops;
    bwin_t window; /* default window which covers entire framebuffer. This functions
                        like the "desktop". */
    void *data;         /* Optional user payload data. */
} bwin_framebuffer_settings;

/**
Summary:
Initialize bwin_framebuffer_settings with defaults.
Description:
This allows the structure to grow in the future with no required code change.
**/
void bwin_framebuffer_settings_init(bwin_framebuffer_settings *settings);

/**
Summary:
Open a framebuffer handle (does not create the actual framebuffer itself).
**/
bwin_framebuffer_t bwin_open_framebuffer(bwin_engine_t win, const bwin_framebuffer_settings *settings);

/**
Summary:
Close a framebuffer handle.
Description:
All windows should already be closed.
**/
void bwin_close_framebuffer(bwin_framebuffer_t fb);

/**
Summary:
Get framebuffer settings.
**/
void bwin_get_framebuffer_settings(bwin_framebuffer_t fb, bwin_framebuffer_settings *settings);

/**
Summary:
Sync must be called whenever you draw to a surface apart from a paint event.
Description:
It calls the drawops.sync function provided and clears any pending sync needed.
the
**/
void bwin_sync(bwin_framebuffer_t fb);

/**
Summary:
Settings for a window.
**/
typedef struct {
    bwin_t parent;      /* For toplevel windows, use the bwin_framebuffer_settings.window
                            for a parent. */
    int x;              /* Location of the window, relative to the parent. */
    int y;              /* Location of the window, relative to the parent. */
    bwin_rect rect;     /* Width and height of the window.
                            x and y must always be 0, therefore this rectangle can be
                            used as a "client" rectangle (the entire area of the window,
                            relative to the window itself). */
    int zorder; /* Relative to siblings only. If two siblings have the same
                            zorder value, then the first window to be created
                            is on bottom. 0 is on bottom, 1 is above, etc. */
    bool visible;       /* Is this window visible? If not, all children are invisible too. */
    bool transparent;   /* Is this window transparent? If it is, then redrawing this window
                            requires that its parent be redrawn as well. */

    const char *name;   /* Optional name. The memory is not copied and this is never
                            accessed inside of iolaus. */
    void *data;         /* Optional user payload data. */
} bwin_settings;

/**
Summary:
Initialize bwin_settings with defaults.
Description:
This allows the structure to grow in the future with no required code change.
**/
void bwin_settings_init(bwin_settings *settings);

/**
Summary:
Open a new window.
**/
bwin_t bwin_open(const bwin_settings *settings);

/**
Summary:
Close a window.
**/
void bwin_close(bwin_t window);

/**
Summary:
Change the settings of a window.
**/
void bwin_set(bwin_t window, const bwin_settings *settings);

/**
Summary:
Get the current settings of a window.
**/
void bwin_get(bwin_t window, bwin_settings *settings);

/**
Summary:
Repaint a window.
**/
void bwin_repaint(
    bwin_t window,
    const bwin_rect *rect   /* Optional region to repaint. If NULL, repaint the
                                entire window. */
    );

/**
Summary:
Draw a single pixel into a window.
**/
void bwin_draw_point(bwin_t win,
    int x, int y,
    uint32_t color, /* ARGB8888 color or palette index */
    const bwin_rect *cliprect /* optional clipping rectangle */
    );

/**
Summary:
Draw a vertical or horizontal line in a window.
Description:
In order to be compatible with rectangles, the last pixel in the line is not drawn.
For instance, if you draw from 0,0 to 10,0, it will draw 10 pixels where the last
pixel drawn will be at 9,0.

Diagonal lines are not supported currently.
**/
void bwin_draw_line(bwin_t win,
    int x1, int y1, int x2, int y2,
    uint32_t color, /* ARGB8888 color or palette index */
    const bwin_rect *cliprect /* optional clipping rectangle */
    );

/**
Summary:
Draw a solid rectangle in a window.
**/
void bwin_fill_rect(bwin_t win,
    const bwin_rect *rect,
    uint32_t color, /* ARGB8888 color or palette index */
    const bwin_rect *cliprect /* optional clipping rectangle */
    );

/**
Summary:
Draw the outline of an ellipse in a window.
**/
void bwin_draw_ellipse(bwin_t win, int x, int y,
    unsigned rx, unsigned ry, int color, const bwin_rect *cliprect);

/**
Summary:
Draw a solid ellipse in a window.
**/
void bwin_fill_ellipse(bwin_t win, int x, int y,
    unsigned rx, unsigned ry, int color, const bwin_rect *cliprect);


/**
Summary:
Copy a rectangle from one window to another window.
Description:
TODO: convert pixel_format
TODO: scale
The windows do not need to be located in the same framebuffer.
**/
void bwin_copy_rect(
    bwin_t destwin, const bwin_rect *destrect,
    bwin_t srcwin, const bwin_rect *srcrect,
    const bwin_rect *cliprect /* optional clipping rectangle */
    );


/**
Summary:
Blend two rectangles (from respective windows) and copy to a third
rectangle in another window.
Description:
The default draw operation is the same as bwin_copy_rect since this
API requires hardware acceleration to be useful.
**/

/* Use the pixel1 parameter to blend the sources. */
#define BWIN_BLEND_WITH_PIXEL1          0x0000
/* Copy src1's alpha-per-pixel to blend the sources */
#define BWIN_BLEND_WITH_SRC1_ALPHA      0x0001
/* Copy src2's alpha-per-pixel to blend the sources */
#define BWIN_BLEND_WITH_SRC2_ALPHA      0x0002

/**
BWIN_BLIT_DEST_ALPHA_XXX specifies how the alpha-per-pixel values of the dest
surface should be filled. If the dest has no alpha-per-pixel, any setting is ignored.
**/

/* Copy the pixel2 parameter into the dest's alpha-per-pixel. */
#define BWIN_SET_DEST_ALPHA_WITH_PIXEL2         0x0000
/* Copy src1's alpha channel into dest's alpha-per-pixel. */
#define BWIN_SET_DEST_ALPHA_WITH_SRC1_ALPHA     0x0010
/* Copy src2's alpha channel into dest's alpha-per-pixel. */
#define BWIN_SET_DEST_ALPHA_WITH_SRC2_ALPHA     0x0020
/* Blend src1's and src2's alpha-per-pixel into dest's alpha-per-pixel.
The formula is dest = src1 + src2 * (1-src1). */
#define BWIN_SET_DEST_ALPHA_WITH_AVG_ALPHA      0x0030

void bwin_blit(
    bwin_t destwin, const bwin_rect *destrect,
    const unsigned int operation,
    bwin_t srcwin1, const bwin_rect *srcrect1,
    bwin_t srcwin2, const bwin_rect *srcrect2,
    const unsigned int pixel1, const unsigned int pixel2
    );

#ifdef __cplusplus
}
#endif

#include "bwin_font.h"
#include "bwin_image.h"

#endif /* BWIN_H__ */
