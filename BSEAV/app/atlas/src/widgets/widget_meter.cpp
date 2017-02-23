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

#include "widget_meter.h"
#include "string.h"
#include "bdbg.h"

BDBG_MODULE(widget_meter);

#define ABS(x)  (((x) < 0) ? -(x) : (x))

/* override default label draw routine to add meter option */
void meter_bwidget_draw(
        bwidget_t         widget,
        const bwin_rect * cliprect
        )
{
    bwin_t            win = bwidget_win(widget);
    bwin_settings     win_settings;
    blabel_settings * label_settings;
    blabel_settings   labelSettings;
    CWidgetMeter *    pMeter;

    blabel_get(widget, &labelSettings);
    label_settings = &labelSettings;

    pMeter = (CWidgetMeter *)label_settings->widget.data;
    BDBG_ASSERT(NULL != pMeter);

    /* TODO: for efficieny, we may want bwidgets to have access to bwin private structures. */
    bwin_get(win, &win_settings);

    /* only draw if something's within the cliprect */
    if (cliprect && !BWIN_RECT_ISINTERSECTING(&win_settings.rect, cliprect))
    {
        return;
    }

    pMeter->drawBackground(widget, label_settings, win_settings.rect, cliprect);
    pMeter->drawBevel(widget, label_settings, win_settings.rect, cliprect);

    /* draw range indicator if necessary */
    if (true == pMeter->isRangeIndicatorVisible())
    {
        bwin_rect rectMeter   = win_settings.rect;
        bwin_rect rectDivider = win_settings.rect;
        int       spacer      = label_settings->bevel + 1;

        rectMeter.x      = spacer;
        rectMeter.y      = rectMeter.y + rectMeter.height / 2;
        rectMeter.width  = rectMeter.width - (spacer * 2);
        rectMeter.height = pMeter->getRangeIndicatorSize();

        rectDivider.x      = rectMeter.width / 2;
        rectDivider.y      = win_settings.rect.y;
        rectDivider.width  = 1;
        rectDivider.height = win_settings.rect.height;

        if (NULL != pMeter->getMeterImage())
        {
            bwin_image_render(win, pMeter->getMeterImage(), pMeter->getMeterRenderMode(), &rectMeter, NULL, cliprect);
            bwin_fill_rect(win, &rectDivider, 0xFFbbbbbb, cliprect);
        }
        else
        {
            bwin_fill_rect(win, &rectMeter, 0xFFbbbbbb, cliprect);
            bwin_fill_rect(win, &rectDivider, 0xFFbbbbbb, cliprect);
        }
    }

    /* draw Meter if necessary*/
    if (true == pMeter->isMeterVisible())
    {
        int16_t   level;
        bwin_rect rectMeter = win_settings.rect;
        int       spacer    = label_settings->bevel + 1;

        rectMeter.x      = spacer;
        rectMeter.y      = spacer;
        rectMeter.width  = rectMeter.width / 2 - (spacer * 2);
        rectMeter.height = rectMeter.height - (spacer * 2);

        /* adjust Meter rect width based on actual level */
        level           = pMeter->getLevel();
        rectMeter.width = rectMeter.width * ABS(level) / 32768;

        /* move meter rect based on >0 or <0 */
        if (level < 0)
        {
            rectMeter.x = win_settings.rect.width / 2 - rectMeter.width - 2;
        }
        else
        {
            rectMeter.x = win_settings.rect.width / 2 + 1;
        }

        if (NULL != pMeter->getMeterImage())
        {
            bwin_image_render(win, pMeter->getMeterImage(), pMeter->getMeterRenderMode(), &rectMeter, NULL, cliprect);
        }
        else
        {
            uint16_t yMin     = rectMeter.y;
            uint16_t yMax     = rectMeter.y + rectMeter.height;
            uint16_t yMid     = yMin + ((yMax - yMin) / 2);
            uint32_t newColor = 0;
            uint8_t  percent  = 0;

            for (int line = yMin; line < yMid; line++)
            {
                percent  = (yMid - line) * 100 / (yMid - yMin);
                newColor = pMeter->colorConvert(pMeter->getColorTop(), pMeter->getColorMiddle(), percent);
                bwin_draw_line(win,
                        rectMeter.x,
                        line,
                        rectMeter.x + rectMeter.width,
                        line,
                        newColor,
                        cliprect);
            }

            for (int line = yMid; line < yMax; line++)
            {
                percent  = (yMax - line) * 100 / (yMax - yMid);
                newColor = pMeter->colorConvert(pMeter->getColorMiddle(), pMeter->getColorBottom(), percent);
                bwin_draw_line(win,
                        rectMeter.x,
                        line,
                        rectMeter.x + rectMeter.width,
                        line,
                        newColor,
                        cliprect);
            }

            /*bwin_fill_rect(win, &rectMeter, pMeter->getColor(), cliprect);*/
        }
    }

    pMeter->drawFocus(widget, label_settings, win_settings.rect, cliprect);
    pMeter->drawText(widget, label_settings, win_settings.rect, cliprect);
} /* meter_bwidget_draw */

CWidgetMeter::CWidgetMeter(
        const char *     strName,
        bwidget_engine_t engine,
        CWidgetBase *    pParentWidget,
        MRect            geometry,
        bwin_font_t      font,
        bwin_t           parentWin
        ) :
    CWidgetLabel(strName, engine, pParentWidget, geometry, font, parentWin),
    _showMeter(false),
    _imageMeter(NULL),
    _imageRenderMode(bwin_image_render_mode_tile),
    _color(0xFF80C42F),
    _colorTop(0xFF7EAFE2),
    _colorMiddle(0xFF0662C6),
    _colorBottom(0xFF022040),
    _level(0),
    _showRange(false),
    _rangeSize(1)
{
    blabel_settings labelSettings;

    /* use custom draw routine so we can draw the background Meter bar */
    blabel_get(getWidget(), &labelSettings);
    labelSettings.text_justify_horiz = bwidget_justify_horiz_center;
    labelSettings.widget.draw        = meter_bwidget_draw;
    blabel_set(getWidget(), &labelSettings);
}

CWidgetMeter::~CWidgetMeter()
{
    if (NULL != _imageMeter)
    {
        bwin_image_close(_imageMeter);
        _imageMeter = NULL;
    }
}

void CWidgetMeter::setColor(uint32_t color)
{
    _color       = color;
    _colorTop    = color;
    _colorMiddle = color;
    _colorBottom = color;
    bwin_repaint(bwidget_win(getWidget()), NULL);
}

eRet CWidgetMeter::loadMeterImage(
        const char *           filename,
        bwin_image_render_mode renderMode
        )
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != filename);

    if (NULL != _imageMeter)
    {
        bwin_image_close(_imageMeter);
    }

    _imageMeter = bwin_image_load(getWinEngine(), filename);
    CHECK_PTR_ERROR_GOTO("unable to load bwin image", _imageMeter, ret, eRet_ExternalError, error);

    _imageRenderMode = renderMode;

    bwin_repaint(bwidget_win(getWidget()), NULL);

error:
    return(ret);
} /* loadMeterImage */

void CWidgetMeter::showMeter(bool show)
{
    _showMeter = show;

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

/* sets the Meter value (range -32767-32767) */
void CWidgetMeter::setLevel(int16_t level)
{
    _level = level;

    bwin_repaint(bwidget_win(getWidget()), NULL);
}