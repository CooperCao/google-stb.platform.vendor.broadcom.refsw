/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "math.h"
#include "widget_base.h"

#define MAX_NUM_BEZIER_POINTS  50 /* TODO: adjust based on curve size - choose 50 for curves up to 15 in size */

BDBG_MODULE(widget_base);

CWidgetBase::CWidgetBase(
        const char *     strName,
        bwidget_engine_t engine,
        CWidgetBase *    pParent
        ) :
    CView(strName),
    _engine(engine),
    _pParent(pParent),
    _widget(NULL),
    _value(-1),
    _backgroundFillMode(fill_eSolid),
    _gradientTop(GRADIENT_TOP),
    _gradientMiddle(GRADIENT_MIDDLE),
    _gradientBottom(GRADIENT_BOTTOM),
    _cornerUpperLeft(5),
    _cornerUpperRight(5),
    _cornerLowerLeft(5),
    _cornerLowerRight(5)
{
}

void CWidgetBase::show(bool bShow)
{
    bwidget_show(getWidget(), bShow);
}

bool CWidgetBase::isVisible()
{
    return(bwidget_visible(getWidget()));
}

/* returns true if focusable, false otherwise */
bool CWidgetBase::setFocus()
{
    return((bwidget_set_focus(getWidget()) != -1) ? true : false);
}

void CWidgetBase::setFocusable(bool bFocusable)
{
    bwidget_settings settings;

    bwidget_get_settings(getWidget(), &settings);
    settings.focusable = bFocusable;
    bwidget_set_settings(getWidget(), &settings);

    bwin_set(getWin(), &settings.window);
}

bwin_t CWidgetBase::getParentWin()
{
    bwidget_settings bwidgetSettings;

    bwidget_get_settings(getWidget(), &bwidgetSettings);
    return(bwidgetSettings.window.parent);
}

void CWidgetBase::setParentWin(bwin_t parentWin)
{
    bwidget_settings bwidgetSettings;

    bwidget_get_settings(getWidget(), &bwidgetSettings);
    bwidgetSettings.window.parent = parentWin;
    bwidget_set_settings(getWidget(), &bwidgetSettings);

    bwin_set(getWin(), &bwidgetSettings.window);
}

int CWidgetBase::getZOrder()
{
    bwidget_settings settings;

    bwidget_get_settings(getWidget(), &settings);

    return(settings.window.zorder);
}

void CWidgetBase::setZOrder(int zorder)
{
    bwidget_settings settings;

    bwidget_get_settings(getWidget(), &settings);
    settings.window.zorder = zorder;
    bwidget_set_settings(getWidget(), &settings);

    bwin_set(getWin(), &settings.window);
}

#define GET_ALPHA(color)  ((color >> 24) & 0xFF)
#define GET_RED(color)    ((color >> 16) & 0xFF)
#define GET_GREEN(color)  ((color >> 8)  & 0xFF)
#define GET_BLUE(color)   ((color >> 0)  & 0xFF)

/* adjust each RGB color value based on the given percentage which indicates the levels
 * of both color1 and color2.  for example, percent=70 would yield 70% of color1 and
 * 30% of color2.  note that alpha channel is also adjusted. */
uint32_t CWidgetBase::colorConvert(
        uint32_t color1,
        uint32_t color2,
        uint8_t  percent
        )
{
    uint32_t newColor = 0;

    newColor |= ((GET_ALPHA(color1) * percent / 100) + (GET_ALPHA(color2) * (100 - percent) / 100)) << 24;
    newColor |= ((GET_RED(color1) * percent / 100) + (GET_RED(color2) * (100 - percent) / 100)) << 16;
    newColor |= ((GET_GREEN(color1) * percent / 100) + (GET_GREEN(color2) * (100 - percent) / 100)) << 8;
    newColor |= ((GET_BLUE(color1) * percent / 100) + (GET_BLUE(color2) * (100 - percent) / 100)) << 0;

    return(newColor);
}

/* returns eRet_Ok to consume keypress, otherwise key is propogated up heirarchy */
eRet CWidgetBase::onKeyDown(
        bwidget_t   widget,
        bwidget_key key
        )
{
    BSTD_UNUSED(widget);
    BSTD_UNUSED(key);
    return(eRet_NotSupported);
}

void CWidgetBase::onFocus(bwidget_t widget)
{
    BSTD_UNUSED(widget);
}

void CWidgetBase::onBlur(bwidget_t widget)
{
    BSTD_UNUSED(widget);
}

void CWidgetBase::onClick(bwidget_t widget)
{
    BSTD_UNUSED(widget);
    return;
}

void CWidgetBase::drawBackground(
        bwidget_t         widget,
        blabel_settings * pLabelSettings,
        const bwin_rect   rect,
        const bwin_rect * pRectClip
        )
{
    bwin_t win = bwidget_win(widget);

    if (pLabelSettings->framebuffer)
    {
        bwin_framebuffer_settings fb_settings;
        bwin_get_framebuffer_settings(pLabelSettings->framebuffer, &fb_settings);
        if (fb_settings.window)
        {
            bwin_rect rectSrc;
            rectSrc.x      = 0;
            rectSrc.y      = 0;
            rectSrc.width  = fb_settings.width;
            rectSrc.height = fb_settings.height;
            BDBG_MSG(("copy framebuffer rect - rectSrc x:%d y:%d w:%d h:%d", rectSrc.x, rectSrc.y, rectSrc.width, rectSrc.height));
            BDBG_MSG(("                      - rectDest x:%d y:%d w:%d h:%d", rect.x, rect.y, rect.width, rect.height));
            BDBG_MSG(("                      - rectClip x:%d y:%d w:%d h:%d", pRectClip->x, pRectClip->y, pRectClip->width, pRectClip->height));
            bwin_copy_rect(win, &(rect), fb_settings.window, &rectSrc, pRectClip);
        }
    }
    else
    if (pLabelSettings->image)
    {
        bwin_image_render(win, pLabelSettings->image, pLabelSettings->render_mode, &rect, NULL, pRectClip);
    }
    else
    {
        /* zero background color means do not draw background at all.  this is useful for layering text
         * over other widgets. */
        if (false == pLabelSettings->background_color_disable)
        {
            if (fill_eGradient == getBackgroundFillMode())
            {
                bwin_rect rectLabel   = rect;
                uint32_t  colorTop    = 0;
                uint32_t  colorMiddle = 0;
                uint32_t  colorBottom = 0;
                unsigned  yMin        = rectLabel.y;
                unsigned  yMax        = rectLabel.y + rectLabel.height;
                unsigned  yMid        = yMin + ((yMax - yMin) / 2);
                uint32_t  newColor    = 0;
                uint8_t   percent     = 0;

                /* give drawing priority to background gradient if enabled */

                getBackgroundGradient(&colorTop, &colorMiddle, &colorBottom);

                /* draw top half */
                for (unsigned line = yMin; line < yMid; line++)
                {
                    percent  = (yMid - line) * 100 / (yMid - yMin);
                    newColor = colorConvert(colorTop, colorMiddle, percent);

                    bwin_draw_line(win, rectLabel.x, line, rectLabel.x + rectLabel.width, line, newColor, pRectClip);
                }

                /* draw bottom half */
                for (unsigned line = yMid; line < yMax; line++)
                {
                    percent  = (yMax - line) * 100 / (yMax - yMid);
                    newColor = colorConvert(colorMiddle, colorBottom, percent);

                    bwin_draw_line(win, rectLabel.x, line, rectLabel.x + rectLabel.width, line, newColor, pRectClip);
                }

#if 0
                /* test draw rounded corners */
                {
                    uint32_t cornerUpperLeft  = 0;
                    uint32_t cornerUpperRight = 0;
                    uint32_t cornerLowerLeft  = 0;
                    uint32_t cornerLowerRight = 0;

                    getRoundedCorners(&cornerUpperLeft, &cornerUpperRight, &cornerLowerLeft, &cornerLowerRight);

                    /* generate corner coordinates */
                    CPoint pointsArray[MAX_NUM_BEZIER_POINTS];
                    {
                        CPoint pointsBezier[3];
                        /* upper left corner */
                        pointsBezier[0]._x = rectLabel.x;
                        pointsBezier[0]._y = rectLabel.y + cornerUpperLeft;
                        pointsBezier[1]._x = rectLabel.x;
                        pointsBezier[1]._y = rectLabel.y;
                        pointsBezier[2]._x = rectLabel.x + cornerUpperLeft;
                        pointsBezier[2]._y = rectLabel.y;
                        generateCornerPoints(pointsArray, MAX_NUM_BEZIER_POINTS, pointsBezier, 3);
                        drawCornerPoints(widget, pointsArray, MAX_NUM_BEZIER_POINTS, pRectClip);

                        /* upper right corner */
                        pointsBezier[0]._x = rectLabel.x + rectLabel.width - 1 - cornerUpperRight;
                        pointsBezier[0]._y = rectLabel.y;
                        pointsBezier[1]._x = rectLabel.x + rectLabel.width - 1;
                        pointsBezier[1]._y = rectLabel.y;
                        pointsBezier[2]._x = rectLabel.x + rectLabel.width - 1;
                        pointsBezier[2]._y = rectLabel.y + cornerUpperRight;
                        generateCornerPoints(pointsArray, MAX_NUM_BEZIER_POINTS, pointsBezier, 3);
                        drawCornerPoints(widget, pointsArray, MAX_NUM_BEZIER_POINTS, pRectClip);

                        /* lower left corner */
                        pointsBezier[0]._x = rectLabel.x + cornerLowerLeft;
                        pointsBezier[0]._y = rectLabel.y + rectLabel.height - 1;
                        pointsBezier[1]._x = rectLabel.x;
                        pointsBezier[1]._y = rectLabel.y + rectLabel.height - 1;
                        pointsBezier[2]._x = rectLabel.x;
                        pointsBezier[2]._y = rectLabel.y + rectLabel.height - 1 - cornerLowerLeft;
                        generateCornerPoints(pointsArray, MAX_NUM_BEZIER_POINTS, pointsBezier, 3);
                        drawCornerPoints(widget, pointsArray, MAX_NUM_BEZIER_POINTS, pRectClip);

                        /* lower right corner */
                        pointsBezier[0]._x = rectLabel.x + rectLabel.width - 1 - cornerLowerRight;
                        pointsBezier[0]._y = rectLabel.y + rectLabel.height - 1;
                        pointsBezier[1]._x = rectLabel.x + rectLabel.width - 1;
                        pointsBezier[1]._y = rectLabel.y + rectLabel.height - 1;
                        pointsBezier[2]._x = rectLabel.x + rectLabel.width - 1;
                        pointsBezier[2]._y = rectLabel.y + rectLabel.height - 1 - cornerLowerRight;
                        generateCornerPoints(pointsArray, MAX_NUM_BEZIER_POINTS, pointsBezier, 3);
                        drawCornerPoints(widget, pointsArray, MAX_NUM_BEZIER_POINTS, pRectClip);
                    }
                }
#endif /* if 0 */
            }
            else
            if (fill_eSolid == getBackgroundFillMode())
            {
                /* no gradient so use solid color fill */
                bwin_fill_rect(win, &rect, pLabelSettings->background_color, pRectClip);
            }
        }
    }
} /* drawBackground */

void CWidgetBase::drawBevel(
        bwidget_t         widget,
        blabel_settings * pLabelSettings,
        const bwin_rect   rect,
        const bwin_rect * pRectClip
        )
{
    bwin_t win = bwidget_win(widget);
    /*
     * disable color bevel - just use solid color over full width
     * int down   = 0;
     */
    int right  = rect.x + rect.width;
    int bottom = rect.y + rect.height;

    if (pLabelSettings->bevel)
    {
        int i;
        /* top */
        for (i = 0; i < pLabelSettings->bevel; i++)
        {
            bwin_draw_line(win, i, i, rect.width-i, i,
                    pLabelSettings->bevel_color[ /*down ? 2 :*/ 0], pRectClip);
        }
        /* right */
        for (i = 0; i < pLabelSettings->bevel; i++)
        {
            bwin_draw_line(win, right-i-1, i, right-i-1, rect.height-i,
                    pLabelSettings->bevel_color[ /*down ? 3 :*/ 1], pRectClip);
        }
        /* bottom */
        for (i = 0; i < pLabelSettings->bevel; i++)
        {
            bwin_draw_line(win, i, bottom-i-1, rect.width-i, bottom-i-1,
                    pLabelSettings->bevel_color[ /*down ? 0 :*/ 2], pRectClip);
        }
        /* left */
        for (i = 0; i < pLabelSettings->bevel; i++)
        {
            bwin_draw_line(win, i, i, i, rect.height-i,
                    pLabelSettings->bevel_color[ /*down ? 1 :*/ 3], pRectClip);
        }
    }
} /* drawBevel */

void CWidgetBase::drawFocus(
        bwidget_t         widget,
        blabel_settings * pLabelSettings,
        const bwin_rect   rect,
        const bwin_rect * pRectClip
        )
{
    bwin_t win    = bwidget_win(widget);
    int    right  = rect.x + rect.width;
    int    bottom = rect.y + rect.height;

    if (widget == bwidget_get_focus(getEngine()))
    {
        /* draw focus */
        int i;
        for (i = 0; i < 2; i++)
        {
            bwin_draw_line(win, i, i, rect.width-i, i,
                    pLabelSettings->focus_color, pRectClip);
            bwin_draw_line(win, right-i-1, i, right-i-1, rect.height-i,
                    pLabelSettings->focus_color, pRectClip);
            bwin_draw_line(win, i, bottom-i-1, rect.width-i, bottom-i-1,
                    pLabelSettings->focus_color, pRectClip);
            bwin_draw_line(win, i, i, i, rect.height-i,
                    pLabelSettings->focus_color, pRectClip);
        }
    }
} /* drawFocus */

void CWidgetBase::drawText(
        const char *      strText,
        bwin_font_t       font,
        bwidget_t         widget,
        blabel_settings * pLabelSettings,
        const bwin_rect   rect,
        const bwin_rect * pRectClip
        )
{
    bwin_t win = bwidget_win(widget);

    if (strText)
    {
        int width, height, base, x, y;
        bwin_measure_text(font, strText, -1, &width, &height, &base);

        /* calculate vertical justification */
        y = 0;
        if (pLabelSettings->text_justify_vert == bwidget_justify_vert_middle)
        {
            y = (rect.height - height) / 2;
        }
        else
        if (pLabelSettings->text_justify_vert == bwidget_justify_vert_bottom)
        {
            y  = rect.height - height;
            y -= pLabelSettings->text_margin;
        }
        else /* bwidget_justify_vert_top */
        {
            y += pLabelSettings->text_margin;
        }

        /* calculate horizontal justification */
        x = 0;
        if (pLabelSettings->text_justify_horiz == bwidget_justify_horiz_center)
        {
            x = (rect.width - width) / 2;
        }
        else
        if (pLabelSettings->text_justify_horiz == bwidget_justify_horiz_right)
        {
            x  = rect.width - width;
            x -= pLabelSettings->text_margin;
        }
        else /* bwidget_justify_horiz_left */
        {
            x += rect.x + pLabelSettings->text_margin;
        }

        bwin_draw_text(win, font, x, y,
                strText, -1, pLabelSettings->text_color,
                pRectClip);
    }
} /* drawText */

void CWidgetBase::drawText(
        bwidget_t         widget,
        blabel_settings * pLabelSettings,
        const bwin_rect   rect,
        const bwin_rect * pRectClip
        )
{
    drawText(pLabelSettings->text, pLabelSettings->font, widget, pLabelSettings, rect, pRectClip);
}

CPoint CWidgetBase::getBezierPoint(
        CPoint * points,
        uint8_t  numPoints,
        float    t
        )
{
    int    i = numPoints - 1;
    CPoint tmp[MAX_NUM_BEZIER_POINTS];

    BDBG_ASSERT(MAX_NUM_BEZIER_POINTS > numPoints);

    memcpy(tmp, points, numPoints * sizeof(CPoint));
    while (0 < i)
    {
        for (int k = 0; k < i; k++)
        {
            tmp[k] = tmp[k] + ((tmp[k + 1] - tmp[k]) * t);
        }
        i--;
    }

    return(tmp[0]);
} /* getBezierPoint */

void CWidgetBase::generateCornerPoints(
        CPoint * pointsCurve,
        uint32_t numPointsCurve,
        CPoint * pointsBezier,
        uint32_t numPointsBezier
        )
{
    uint32_t i = 0;

    for (float increment = 0; i < numPointsCurve; increment += (1.0 / (float)numPointsCurve))
    {
        pointsCurve[i] = getBezierPoint(pointsBezier, numPointsBezier, increment);
        /*BDBG_WRN(("pointsCurve[%d] %f,%f", i, pointsCurve[i]._x, pointsCurve[i]._y));*/
        i++;
    }
}

void CWidgetBase::drawCornerPoints(
        bwidget_t         widget,
        CPoint *          points,
        uint32_t          numPoints,
        const bwin_rect * pRectClip
        )
{
    bwin_t win = bwidget_win(widget);

    BDBG_ASSERT(NULL != points);

    for (uint32_t i = 0; i < numPoints; i++)
    {
        float err = 1;
        /* uint8_t color = 0xff; */

        err = ((points[i]._x - (int)(points[i]._x)) +
               (points[i]._y - (int)(points[i]._y))) / 2.0;

        /* if (1.0 > err) */
        {
            /* color = (err * 255.0) + 0.5; */

            bwin_draw_point(
                    win,
                    (int)(points[i]._x + 0.5),
                    (int)(points[i]._y + 0.5),
                    0xffFF0000, /* force red for testing */
                    pRectClip);
        }
    }
} /* drawCornerPoints */