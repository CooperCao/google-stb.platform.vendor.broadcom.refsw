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

#include "bwin.h"
#include "bwin_priv.h"
#include <assert.h>
#include <string.h>
BDBG_MODULE(bwin_draw);

/* runtime support of all pixel formats. converts from argb8888 to bwin_pixel_format */
uint32_t bwin_p_convert_color(bwin_framebuffer_t fb, uint32_t color)
{
    uint32_t c;
    switch (fb->settings.pixel_format) {
    case bwin_pixel_format_r5_g6_b5:
        c = ((color & 0x00F80000) >> 8) |
            ((color & 0x0000FC00) >> 5) |
            ((color & 0x000000F8) >> 3);
        break;
    case bwin_pixel_format_a1_r5_g5_b5:
        c = ((color & 0x80000000) >> 16) |
            ((color & 0x00F80000) >> 9) |
            ((color & 0x0000F800) >> 6) |
            ((color & 0x000000F8) >> 3);
        break;
    default: /* argb8888 or p8 - no transform */
        c = color;
        break;
    }
    return c;
}

#define OPS(win) ((win)->fb->settings.drawops)

void bwin_draw_point(bwin_t win, int x, int y, uint32_t color,
    const bwin_rect *cliprect)
{
    if (!cliprect) cliprect = &win->settings.rect;
    if (BWIN_POINT_IN_RECT(x,y,cliprect)) {
        int gx, gy;
        bwin_p_convert_to_global(win, x, y, &gx, &gy);

        if (BWIN_POINT_IN_RECT(gx,gy,&win->fb->rect)) {
            (OPS(win).draw_point)(win->fb, gx, gy, bwin_p_convert_color(win->fb, color), color);
        }
    }
}

void bwin_draw_line(bwin_t win, int x1, int y1, int x2, int y2, uint32_t color,
    const bwin_rect *cliprect)
{
    bwin_rect intersect;

    /* normalize the line */
    if (x1 > x2) {
        int temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if (y1 > y2) {
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }

    /* find the intersection with the cliprect by converting to a rectangle. */
    BWIN_SET_RECT(&intersect, x1,y1,x2-x1,y2-y1);

    /* clip to cliprect (default to window) and framebuffer */
    if (!cliprect) cliprect = &win->settings.rect;
    bwin_intersect_rect(&intersect, &intersect, cliprect);
    if (!intersect.width && !intersect.height) return;

    /* handle boundary case */
    if (!intersect.width && intersect.x == cliprect->x+(int)cliprect->width)
        return;
    if (!intersect.height && intersect.y == cliprect->y+(int)cliprect->height)
        return;

    /* convert to global coordinates */
    bwin_p_convert_to_global(win, intersect.x, intersect.y, &intersect.x, &intersect.y);

    /* clip to framebuffer */
    cliprect = &win->fb->rect;
    bwin_intersect_rect(&intersect, &intersect, cliprect);
    if (!intersect.width && !intersect.height) return;

    /* handle boundary case */
    if (!intersect.width && intersect.x == cliprect->x+(int)cliprect->width)
        return;
    if (!intersect.height && intersect.y == cliprect->y+(int)cliprect->height)
        return;

    BDBG_MSG(("draw_line 0x%06x: %d,%d,%d,%d", color, intersect.x, intersect.y,
        intersect.x+intersect.width, intersect.y+intersect.height));

    (OPS(win).draw_line)(win->fb, intersect.x, intersect.y,
        intersect.x+intersect.width, intersect.y+intersect.height,
        bwin_p_convert_color(win->fb, color), color);
}

void bwin_fill_rect(bwin_t win, const bwin_rect *rect, uint32_t color,
    const bwin_rect *cliprect)
{
    bwin_rect intersect, grect;

    /* clip to cliprect (default to window) */
    if (!cliprect) cliprect = &win->settings.rect;
    bwin_intersect_rect(&intersect, rect, cliprect);

    /* convert to global coordinates */
    grect = intersect;
    bwin_p_convert_to_global(win, intersect.x, intersect.y, &grect.x, &grect.y);

    BDBG_MSG(("fill_rect %d,%d,%d,%d", grect.x,grect.y,grect.width,grect.height));
    BDBG_MSG(("          %d,%d,%d,%d", win->fb->rect.x,win->fb->rect.y,win->fb->rect.width,win->fb->rect.height));

    /* clip to framebuffer */
    bwin_intersect_rect(&grect, &grect, &win->fb->rect);

    if (grect.width && grect.height) {
        if (grect.width * grect.height >= OPS(win).fill_rect_threshold) {
            (OPS(win).fill_rect)(win->fb, &grect, bwin_p_convert_color(win->fb, color), color);
        }
        else {
            bwin_p_fill_rect(win->fb, &grect, bwin_p_convert_color(win->fb, color), color);
        }
    }
}

void bwin_copy_rect(bwin_t destwin, const bwin_rect *destrect,
    bwin_t srcwin, const bwin_rect *srcrect,
    const bwin_rect *cliprect)
{
    bwin_rect g_destrect = *destrect;
    bwin_rect g_srcrect = *srcrect;

    /* If there's a cliprect, clip the dest, then clip the src by a proportional
    amount */
    BDBG_MSG(("copy_rect %x,%x,%x,%x -> %x,%x,%x,%x (%x,%x,%x,%x)",
        srcrect->x,  srcrect->y,  srcrect->width,  srcrect->height,
        destrect->x, destrect->y, destrect->width, destrect->height,
        cliprect->x, cliprect->y, cliprect->width, cliprect->height));
    if (cliprect) {
        bwin_rect new_destrect;
        bwin_rect new_srcrect;
        bwin_intersect_rect(&new_destrect, destrect, cliprect);

        new_srcrect.x = srcrect->x +
            (new_destrect.x - destrect->x) *
            srcrect->width / destrect->width;
        new_srcrect.y = srcrect->y +
            (new_destrect.y - destrect->y) *
            srcrect->height / destrect->height;
        new_srcrect.width = new_destrect.width *
            srcrect->width / destrect->width;
        new_srcrect.height = new_destrect.height *
            srcrect->height / destrect->height;

        g_destrect = new_destrect;
        g_srcrect = new_srcrect;
    }

    bwin_p_convert_to_global(destwin, g_destrect.x, g_destrect.y, &g_destrect.x, &g_destrect.y);
    bwin_p_convert_to_global(srcwin, g_srcrect.x, g_srcrect.y, &g_srcrect.x, &g_srcrect.y);

    /* intersect with framebuffer */
    bwin_intersect_rect(&g_destrect, &g_destrect, &destwin->fb->rect);
    if (!g_destrect.width || !g_destrect.height) return;

    BDBG_MSG(("copy_rect %d,%d,%d,%d -> %d,%d,%d,%d",
        g_srcrect.x, g_srcrect.y, g_srcrect.width, g_srcrect.height,
        g_destrect.x, g_destrect.y, g_destrect.width, g_destrect.height));
    (OPS(destwin).copy_rect)(destwin->fb, &g_destrect, srcwin->fb, &g_srcrect);
}

void bwin_blit(bwin_t destwin, const bwin_rect *destrect,
               const unsigned int operation,
               bwin_t srcwin1, const bwin_rect *srcrect1,
               bwin_t srcwin2, const bwin_rect *srcrect2,
               const unsigned int pixel1, const unsigned int pixel2)
{
    bwin_rect g_destrect = *destrect;
    bwin_rect g_srcrect1 = *srcrect1;
    bwin_rect g_srcrect2 = *srcrect2;

    bwin_p_convert_to_global(destwin, g_destrect.x, g_destrect.y, &g_destrect.x, &g_destrect.y);
    bwin_p_convert_to_global(srcwin1, g_srcrect1.x, g_srcrect1.y, &g_srcrect1.x, &g_srcrect1.y);
    bwin_p_convert_to_global(srcwin2, g_srcrect2.x, g_srcrect2.y, &g_srcrect2.x, &g_srcrect2.y);

    /* intersect with framebuffer */
    bwin_intersect_rect(&g_destrect, &g_destrect, &destwin->fb->rect);
    if (!g_destrect.width || !g_destrect.height) return;

    (OPS(destwin).blit)(destwin->fb, &g_destrect,
                        operation,
                        srcwin1->fb, &g_srcrect1, srcwin2->fb, &g_srcrect2,
                        pixel1, pixel2);
}

void bwin_draw_ellipse(bwin_t win, int x, int y,
    unsigned rx, unsigned ry, int color, const bwin_rect *cliprect)
{
    bwin_rect rect;

    /* set window cliprect */
    if (!cliprect)
        rect = win->settings.rect;
    else {
        rect = *cliprect;
        bwin_intersect_rect(&rect, &rect, &win->settings.rect);
    }

    /* convert to global coordinates */
    bwin_p_convert_to_global(win, rect.x, rect.y, &rect.x, &rect.y);
    bwin_p_convert_to_global(win, x, y, &x, &y);

    /* intersect with framebuffer */
    bwin_intersect_rect(&rect, &rect, &win->fb->rect);

    /* note that we don't actually clip the ellipse. this must be done at draw time. */
    if (rect.width && rect.height)
        (OPS(win).draw_ellipse)(win->fb, x, y, rx, ry, bwin_p_convert_color(win->fb, color), color, &rect);
}

void bwin_fill_ellipse(bwin_t win, int x, int y,
    unsigned rx, unsigned ry, int color, const bwin_rect *cliprect)
{
    bwin_rect rect;

    /* set window cliprect */
    if (!cliprect)
        rect = win->settings.rect;
    else {
        rect = *cliprect;
        bwin_intersect_rect(&rect, &rect, &win->settings.rect);
    }

    /* convert to global coordinates */
    bwin_p_convert_to_global(win, rect.x, rect.y, &rect.x, &rect.y);
    bwin_p_convert_to_global(win, x, y, &x, &y);

    /* intersect with framebuffer */
    bwin_intersect_rect(&rect, &rect, &win->fb->rect);

    /* note that we don't actually clip the ellipse. this must be done at draw time. */
    if (rect.width && rect.height)
        (OPS(win).fill_ellipse)(win->fb, x, y, rx, ry, bwin_p_convert_color(win->fb, color), color, &rect);
}
