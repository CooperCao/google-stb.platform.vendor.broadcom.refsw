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

#include "bwidget_priv.h"
BDBG_MODULE(bwidget_label);

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

#define PSETTINGS(widget) ((blabel_settings*)widget->settings)

static void
bwidget_p_draw_label(bwidget_t widget, const bwin_rect *cliprect)
{
    /* code reuse for button */
    bwidget_p_draw_label_helper(widget, false, cliprect);
}

void
bwidget_p_draw_label_helper(bwidget_t widget, bool down, const bwin_rect *cliprect)
{
    bwin_t win = GET_WIN(widget);
    bwin_settings win_settings;
    int right, bottom;
    blabel_settings *label_settings = PSETTINGS(widget);

    bwidget_p_verify(widget, bwidget_type_label);

    /* TODO: for efficieny, we may want bwidgets to have access to bwin private structures. */
    bwin_get(win, &win_settings);

    /* only draw if something's within the cliprect */
    if (cliprect && !BWIN_RECT_ISINTERSECTING(&win_settings.rect, cliprect)) {
        return;
    }

    right = win_settings.rect.x + win_settings.rect.width;
    bottom = win_settings.rect.y + win_settings.rect.height;
    BDBG_MSG_TRACE(("draw_label: %p %d,%d,%d,%d (%d,%d,%d,%d)", widget, label_settings->widget.window.x, label_settings->widget.window.y, win_settings.rect.width, win_settings.rect.height,
        cliprect->x, cliprect->y, cliprect->width, cliprect->height));

    if (label_settings->framebuffer) {
        bwin_framebuffer_settings fb_settings;
        bwin_get_framebuffer_settings(label_settings->framebuffer, &fb_settings);
        if (fb_settings.window)
        {
            bwin_rect rectSrc;
            rectSrc.x      = 0;
            rectSrc.y      = 0;
            rectSrc.width  = fb_settings.width;
            rectSrc.height = fb_settings.height;
            BDBG_MSG(("copy framebuffer rect - rectSrc x:%d y:%d w:%d h:%d", rectSrc.x, rectSrc.y, rectSrc.width, rectSrc.height));
            BDBG_MSG(("                      - rectDest x:%d y:%d w:%d h:%d", win_settings.rect.x, win_settings.rect.y, win_settings.rect.width, win_settings.rect.height));
            BDBG_MSG(("                      - rectClip x:%d y:%d w:%d h:%d", cliprect->x, cliprect->y, cliprect->width, cliprect->height));
            bwin_copy_rect(win, &(win_settings.rect), fb_settings.window, &rectSrc, cliprect);
        }
    }
    else if (label_settings->image) {
            bwin_image_render(win, label_settings->image, label_settings->render_mode, &win_settings.rect, NULL, cliprect);
    }
    else {
        if (false == label_settings->background_color_disable)
        {
            /* zero background color means do not draw background at all.  this is useful for layering text
               over other widgets. */
            bwin_fill_rect(win, &win_settings.rect, label_settings->background_color, cliprect);
        }

        if (label_settings->bevel) {
            int i;
            /* top */
            for (i=0;i<label_settings->bevel;i++) {
                bwin_draw_line(win, i, i, win_settings.rect.width-i, i,
                    label_settings->bevel_color[down?2:0], cliprect);
            }
            /* right */
            for (i=0;i<label_settings->bevel;i++) {
                bwin_draw_line(win, right-i-1, i, right-i-1, win_settings.rect.height-i,
                    label_settings->bevel_color[down?3:1], cliprect);
            }
            /* bottom */
            for (i=0;i<label_settings->bevel;i++) {
                bwin_draw_line(win, i, bottom-i-1, win_settings.rect.width-i, bottom-i-1,
                    label_settings->bevel_color[down?0:2], cliprect);
            }
            /* left */
            for (i=0;i<label_settings->bevel;i++) {
                bwin_draw_line(win, i, i, i, win_settings.rect.height-i,
                    label_settings->bevel_color[down?1:3], cliprect);
            }
        }
    }

    if (widget->engine->focus == widget) {
        /* draw focus */
        int i;
        for (i=0;i<2;i++) {
            bwin_draw_line(win, i, i, win_settings.rect.width-i, i,
                label_settings->focus_color, cliprect);
            bwin_draw_line(win, right-i-1, i, right-i-1, win_settings.rect.height-i,
                label_settings->focus_color, cliprect);
            bwin_draw_line(win, i, bottom-i-1, win_settings.rect.width-i, bottom-i-1,
                label_settings->focus_color, cliprect);
            bwin_draw_line(win, i, i, i, win_settings.rect.height-i,
                label_settings->focus_color, cliprect);
        }
    }

    if (label_settings->text) {
        int width, height, base, x, y;
        bwin_measure_text(label_settings->font, label_settings->text, -1,
            &width, &height, &base);

        /* calculate vertical justification */
        y = 0;
        if (label_settings->text_justify_vert == bwidget_justify_vert_middle) {
            y = (win_settings.rect.height - height) / 2;
        }
        else if (label_settings->text_justify_vert == bwidget_justify_vert_bottom) {
            y += label_settings->text_margin;
        }
        else { /* bwidget_justify_vert_top */
            y = win_settings.rect.height - height;
            y -= label_settings->text_margin;
        }

        /* calculate horizontal justification */
        x = 0;
        if (label_settings->text_justify_horiz == bwidget_justify_horiz_center) {
            x = (win_settings.rect.width - width) / 2;
        }
        else if (label_settings->text_justify_horiz == bwidget_justify_horiz_right) {
            x = win_settings.rect.width - width;
            x -= label_settings->text_margin;
        }
        else { /* bwidget_justify_horiz_left */
            x += label_settings->text_margin;
        }

        bwin_draw_text(win, label_settings->font, x, y,
            label_settings->text, -1, label_settings->text_color,
            cliprect);
    }
}

void
bwidget_p_set_label_defaults(bwidget_t widget, bwidget_engine_t engine)
{
    bwidget_p_verify(widget, bwidget_type_label);
    bwidget_p_set_base_defaults(widget, engine);
}

void blabel_get_default_settings(blabel_settings *settings)
{
    BKNI_Memset(settings, 0, sizeof(*settings));
    bwidget_get_default_settings(&settings->widget);
    settings->bevel = 5;
    settings->widget.draw = bwidget_p_draw_label;

    settings->image = NULL;
    settings->framebuffer = NULL;
    settings->render_mode = bwin_image_render_mode_stretch;

    settings->background_color = 0xff282277;
    settings->text_color = 0xff999999;
    settings->focus_color = 0xffAAAA00;
    settings->bevel_color[0] = 0xff7070c0;
    settings->bevel_color[1] = 0xff11112d;
    settings->bevel_color[2] = 0xff303040;
    settings->bevel_color[3] = 0xff8080d0;
}

bwidget_t
blabel_create(bwidget_engine_t engine, const blabel_settings *settings)
{
    bwidget_t label = bwidget_p_create(engine, settings, sizeof(struct bwidget_label), sizeof(*settings), bwidget_type_label);
    bwidget_p_set_label_defaults(label, engine);
    return label;
}

void
blabel_get(bwidget_t widget, blabel_settings *settings)
{
    BKNI_Memcpy(settings, widget->settings, sizeof(*settings));
}

int blabel_p_apply_settings(bwidget_t widget, const blabel_settings *settings)
{
    return bwidget_p_apply_settings(widget, &settings->widget);
}

int
blabel_set(bwidget_t widget, const blabel_settings *settings)
{
    blabel_p_apply_settings(widget, settings);
    BKNI_Memcpy(widget->settings, settings, sizeof(*settings));
    return 0;
}
