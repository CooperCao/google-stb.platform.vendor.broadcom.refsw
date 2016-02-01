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

#ifndef BWIN_FONT_H__
#define BWIN_FONT_H__

#include "bwin.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bwin_font *bwin_font_t;

/**
Summary:
**/
bwin_font_t bwin_open_font(
    bwin_engine_t win,
    const char *filename,   /* Filename of the font. */
    int size, /* point size. If -1, then this is a rendered font, not a truetype font. */
    bool antialiased    /* if true, blend the edges with the background.
                            If size is -1, then this is ignored. */
    );

/**
Summary:
Save a rendered font into a bwin format.
**/
int bwin_save_rendered_font(bwin_font_t font, const char *filename);

/**
Summary:
You must close any font which you open.
**/
void bwin_close_font(bwin_font_t font);

/**
Summary:
Get the recommended baseline-to-baseline height for spacing text.
**/
void bwin_get_font_height(bwin_font_t font, unsigned *height);

/**
Summary:
Calculate how large text would be if rendered.
Return Values:
0 is succesful.
-1 is a character cannot be loaded.
**/
int bwin_measure_text(bwin_font_t font, const char *text,
    int len, /* if -1, use strlen(text) */
    int *width,     /* total width in pixels of the text */
    int *height,    /* total height in pixels of the text */
    int *base       /* # of pixels from the top to the baseline (e.g. ascender) */
    );

/**
Summary:
Draw text into a window.

Description:
If you are using a palettized pixel format, you can still do antialiased fonts,
but it requires a special technique and a code modification. The recommended method
is that the colors following the text color be arranged to blend with the background.
As an example, let's say that your text color blue is index 5 and you want to
anti-alias with a white background. Then, the following should appear in your palette:
o index 5 = blue
o index 6 = 2/3 blue + 1/3 white
o index 7 = 1/3 blue + 2/3 white
o index 8 = white
To make this work, please see antialias_palette8_pixels in bwin_font.c.
**/
int bwin_draw_text(bwin_t window, bwin_font_t font,
    int x, int y,
    const char *text,
    int len, /* if -1, use strlen(text) */
    uint32_t color,
    const bwin_rect *cliprect   /* optional cliprect, in window coordinates */
    );

typedef size_t (*bwin_readfn_t)(void *buffer, size_t size, size_t number, void *handle);

bwin_font_t bwin_open_rendered_font(
    bwin_engine_t win,
    bwin_readfn_t readfn,
    void *context
    );

typedef struct  {
      void *context;
      bwin_readfn_t readfn;
      int size;
      bool antialiased;
      unsigned buf_len;
} bwin_font_settings;

void bwin_get_default_font_settings(bwin_font_settings *settings);
bwin_font_t bwin_open_font_generic(bwin_engine_t win, bwin_font_settings *settings);
#ifdef __cplusplus
}
#endif

#endif /* BWIN_FONT_H__*/
