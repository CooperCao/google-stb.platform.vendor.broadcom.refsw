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

#include "screen.h"
#include "widget_check_button.h"
#include "widget_base.h"
#include "string.h"
#include "bdbg.h"

BDBG_MODULE(widget_check_button);

static void clickCallback(bwidget_t widget)
{
    CWidgetCheckButton * pWidget = NULL;
    CWidgetBase *        pParent = NULL;

    bwidget_settings bwidgetSettings;

    bwidget_get_settings(widget, &bwidgetSettings);
    pWidget = (CWidgetCheckButton *)bwidgetSettings.data;
    BDBG_ASSERT(NULL != pWidget);

    if (true == pWidget->isVisible())
    {
        /* toggle check */
        pWidget->setCheck(pWidget->isChecked() ? false : true);

        pParent = pWidget->getParent();
        if (NULL != pParent)
        {
            /* widget is visible, so pass on click to the parent widget */
            pParent->onClick(widget);
        }
    }
} /* clickCallback */

static void check_button_bwidget_draw(
        bwidget_t         widget,
        const bwin_rect * cliprect
        )
{
    bwin_t               win = bwidget_win(widget);
    bwin_settings        win_settings;
    blabel_settings *    label_settings = NULL;
    bbutton_settings     buttonSettings;
    CWidgetCheckButton * pButton = NULL;
    bool                 down    = false;

    bbutton_get(widget, &buttonSettings);
    label_settings = &(buttonSettings.label);

    pButton = (CWidgetCheckButton *)buttonSettings.label.widget.data;
    BDBG_ASSERT(NULL != pButton);

    down = buttonSettings.down;

    /* TODO: for efficieny, we may want bwidgets to have access to bwin private structures. */
    bwin_get(win, &win_settings);

    /* only draw if something's within the cliprect */
    if (cliprect && !BWIN_RECT_ISINTERSECTING(&win_settings.rect, cliprect))
    {
        return;
    }

    pButton->drawBackground(widget, label_settings, win_settings.rect, cliprect);
    pButton->drawBevel(widget, label_settings, win_settings.rect, cliprect);

    /* draw check box */

    if (eCheckStyle_Box == pButton->getCheckStyle())
    {
        int       i        = 0;
        int       box_size = pButton->getBoxSize();
        int       margin   = 8;
        bwin_rect brect;
        MRect     rect(win_settings.rect.x + win_settings.rect.width - box_size - margin,
                       win_settings.rect.y + (win_settings.rect.height / 2) - (box_size / 2),
                       box_size,
                       box_size);
        rect.setX(rect.x() - label_settings->bevel);

        brect.x      = rect.x();
        brect.y      = rect.y();
        brect.width  = rect.width();
        brect.height = rect.height();
        bwin_fill_rect(win, &brect, label_settings->bevel_color[1], cliprect);

        brect.x      = rect.x() + 3;
        brect.y      = rect.y() + 3;
        brect.width  = rect.width() - 5;
        brect.height = rect.height() - 5;

        /* if checked then draw check icon */
        {
            uint32_t colorTop    = 0;
            uint32_t colorMiddle = 0;
            uint32_t colorBottom = 0;
            pButton->getBackgroundGradient(&colorTop, &colorMiddle, &colorBottom);
            if (true == pButton->isChecked())
            {
                pButton->setBackgroundGradient(COLOR_GREEN - 4 * COLOR_STEP, COLOR_GREEN, COLOR_GREEN + 2 * COLOR_STEP);
            }
            else
            {
                pButton->setBackgroundGradient(colorBottom - 4 * COLOR_STEP, colorMiddle, colorTop + 2 * COLOR_STEP);
            }
            pButton->drawBackground(widget, label_settings, brect, cliprect);
            pButton->setBackgroundGradient(colorTop, colorMiddle, colorBottom);
        }

        /* draw check box */
        {
            uint32_t  color   = pButton->getBoxColor();
            bwin_rect rectBox = { rect.x(), rect.y(), rect.width(), rect.height() };

            for (i = 0; i < 2; i++)
            {
                bwin_draw_line(win, rectBox.x, rectBox.y, rectBox.x + rectBox.width, rectBox.y, color, cliprect);
                bwin_draw_line(win, rectBox.x + rectBox.width, rectBox.y, rectBox.x + rectBox.width, rectBox.y + rectBox.height, color, cliprect);
                bwin_draw_line(win, rectBox.x, rectBox.y + rectBox.height, rectBox.x + rectBox.width + 1, rectBox.y + rectBox.height, color, cliprect);
                bwin_draw_line(win, rectBox.x, rectBox.y, rectBox.x, rectBox.y + rectBox.height, color, cliprect);

                color += 2 * COLOR_STEP;

                rectBox.x      += 1;
                rectBox.y      += 1;
                rectBox.width  -= 2;
                rectBox.height -= 2;
            }
        }

        {
            bwin_rect rectFocus = { rect.x(), rect.y(), rect.width(), rect.height() };
            rectFocus.x      -= 1;
            rectFocus.y      -= 1;
            rectFocus.width  += 2;
            rectFocus.height += 2;
            pButton->drawFocus(widget, label_settings, rectFocus, cliprect);
        }
    }
    else /* eCheckStyle_Slide */
    {
        int       i        = 0;
        int       box_size = pButton->getBoxSize();
        int       margin   = 8;
        bwin_rect brect;
        bwin_rect bevel_rect;
        MRect     rect(win_settings.rect.x + win_settings.rect.width - 3 * box_size - margin,
                       win_settings.rect.y + (win_settings.rect.height / 2) - (box_size / 2),
                       3 * box_size,
                       box_size);
        rect.setX(rect.x() - label_settings->bevel);

        brect.x      = rect.x();
        brect.y      = rect.y();
        brect.width  = rect.width();
        brect.height = rect.height();

        /* draw background*/
        {
            uint32_t colorTop    = 0;
            uint32_t colorMiddle = 0;
            uint32_t colorBottom = 0;
            pButton->getBackgroundGradient(&colorTop, &colorMiddle, &colorBottom);
            if (true == pButton->isChecked())
            {
                pButton->setBackgroundGradient(COLOR_GREEN - 4 * COLOR_STEP, COLOR_GREEN, COLOR_GREEN + 2 * COLOR_STEP);
            }
            else
            {
                pButton->setBackgroundGradient(colorBottom - 4 * COLOR_STEP, colorMiddle, colorTop + 2 * COLOR_STEP);
            }
            pButton->drawBackground(widget, label_settings, brect, cliprect);
            pButton->setBackgroundGradient(colorTop, colorMiddle, colorBottom);
        }

        /* draw on/off text */
        {
            const char strTextOff[4] = { "OFF" };
            const char strTextOn[3]  = { "ON" };

            bwin_rect brectText = brect;

            brectText.height = win_settings.rect.height;

            if (true == pButton->isChecked())
            {
                uint32_t colorText = label_settings->text_color;

                label_settings->text_color = COLOR_GREY_DARK;
                pButton->drawText(strTextOn, pButton->_font10, widget, label_settings, brectText, cliprect);
                label_settings->text_color = colorText;
            }
            else
            {
                brectText.x += box_size;
                pButton->drawText(strTextOff, pButton->_font10, widget, label_settings, brectText, cliprect);
            }
        }

        /* draw thumb icon */
        {
            if (true == pButton->isChecked())
            {
                /* if unchecked then draw check icon in OFF position */
                brect.x      = rect.x() + rect.width() - box_size + 3;
                brect.y      = rect.y() + 3;
                brect.width  = box_size - 5;
                brect.height = box_size - 5;
            }
            else
            {
                /* if checked then draw check icon in ON position */
                brect.x      = rect.x() + 3;
                brect.y      = rect.y() + 3;
                brect.width  = box_size - 5;
                brect.height = box_size - 5;
            }

            bevel_rect         = brect;
            bevel_rect.x      -= 1;
            bevel_rect.y      -= 1;
            bevel_rect.width  += 2;
            bevel_rect.height += 2;
            bwin_fill_rect(win, &bevel_rect, label_settings->text_color, cliprect);
            pButton->drawBackground(widget, label_settings, brect, cliprect);
        }

        /* draw check box */
        {
            uint32_t color = pButton->getBoxColor();

            for (i = 1; i < 2; i++)
            {
                bwin_draw_line(win, rect.x(), rect.y(), rect.x() + rect.width(), rect.y(), color, cliprect);
                bwin_draw_line(win, rect.x() + rect.width(), rect.y(), rect.x() + rect.width(), rect.y() + rect.height(), color, cliprect);
                bwin_draw_line(win, rect.x(), rect.y() + rect.height(), rect.x() + rect.width() + 1, rect.y() + rect.height(), color, cliprect);
                bwin_draw_line(win, rect.x(), rect.y(), rect.x(), rect.y() + rect.height(), color, cliprect);

                color += 2 * COLOR_STEP;

                rect.setX(rect.x() - 1);
                rect.setY(rect.y() - 1);
                rect.grow(2);
            }
        }

        {
            bwin_rect rectFocus = { rect.x(), rect.y(), rect.width(), rect.height() };
            pButton->drawFocus(widget, label_settings, rectFocus, cliprect);
        }
    }

    pButton->drawText(widget, label_settings, win_settings.rect, cliprect);
} /* check_button_bwidget_draw */

CWidgetCheckButton::CWidgetCheckButton(
        const char *     strName,
        bwidget_engine_t engine,
        CWidgetBase *    pParentWidget,
        MRect            geometry,
        bwin_font_t      font,
        bwin_t           parentWin
        ) : /* optional */
    CWidgetButton(strName, engine, pParentWidget, geometry, font, parentWin),
    _bChecked(false),
    _boxColor(COLOR_BLUE_SLATE),
    _boxSize(16),
    _checkColor(0xFF80C42F),
    _checkStyle(eCheckStyle_Box),
    _font10(NULL)
{
    eRet             ret = eRet_Ok;
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    buttonSettings.click             = clickCallback;
    buttonSettings.label.widget.draw = check_button_bwidget_draw;
    bbutton_set(getWidget(), &buttonSettings);

    _font10 = bwin_open_font(getWinEngine(), "fonts/verdana_10.bwin_font", -1, true);
    CHECK_PTR_ERROR("unable to open font 10", _font10, ret, eRet_OutOfMemory);
}

void CWidgetCheckButton::setCheck(bool bChecked)
{
    _bChecked = bChecked;

    /* dtt todo: only have to repaint checkbox area */
    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetCheckButton::setBoxColor(uint32_t color)
{
    _boxColor = color;

    /* dtt todo: only have to repaint checkbox area */
    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetCheckButton::setCheckColor(uint32_t color)
{
    _checkColor = color;

    /* dtt todo: only have to repaint checkbox area */
    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetCheckButton::drawFocus(
        bwidget_t         widget,
        blabel_settings * pLabelSettings,
        const bwin_rect   rect,
        const bwin_rect * pRectClip
        )
{
    bwin_t    win      = bwidget_win(widget);
    bwin_rect rectTemp = rect;

    if (widget == bwidget_get_focus(getEngine()))
    {
        /* draw focus */
        for (int i = 0; i < 2; i++)
        {
            bwin_draw_line(win, rectTemp.x, rectTemp.y, rectTemp.x + rectTemp.width, rectTemp.y,
                    pLabelSettings->focus_color, pRectClip);
            bwin_draw_line(win, rectTemp.x + rectTemp.width, rectTemp.y, rectTemp.x + rectTemp.width, rectTemp.y + rectTemp.height,
                    pLabelSettings->focus_color, pRectClip);
            bwin_draw_line(win, rectTemp.x, rectTemp.y + rectTemp.height, rectTemp.x + rectTemp.width + 1, rectTemp.y + rectTemp.height,
                    pLabelSettings->focus_color, pRectClip);
            bwin_draw_line(win, rectTemp.x, rectTemp.y, rectTemp.x, rectTemp.y + rectTemp.height,
                    pLabelSettings->focus_color, pRectClip);

            rectTemp.x       = (rectTemp.x - 1);
            rectTemp.y       = (rectTemp.y - 1);
            rectTemp.width  += 2;
            rectTemp.height += 2;
        }
    }
} /* drawFocus */