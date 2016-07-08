/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 *****************************************************************************/

#include "widget_progress.h"
#include "string.h"
#include "bdbg.h"

BDBG_MODULE(widget_progress);

/* override default label draw routine to add progress option */
void progress_bwidget_draw(
        bwidget_t         widget,
        const bwin_rect * cliprect
        )
{
    bwin_t            win = bwidget_win(widget);
    bwin_settings     win_settings;
    blabel_settings * label_settings;
    blabel_settings   labelSettings;
    CWidgetProgress * pProgress;

    blabel_get(widget, &labelSettings);
    label_settings = &labelSettings;

    pProgress = (CWidgetProgress *)label_settings->widget.data;
    BDBG_ASSERT(NULL != pProgress);

    /* TODO: for efficieny, we may want bwidgets to have access to bwin private structures. */
    bwin_get(win, &win_settings);

    /* only draw if something's within the cliprect */
    if (cliprect && !BWIN_RECT_ISINTERSECTING(&win_settings.rect, cliprect))
    {
        return;
    }

    pProgress->drawBackground(widget, label_settings, win_settings.rect, cliprect);
    pProgress->drawBevel(widget, label_settings, win_settings.rect, cliprect);

    /* draw range indicator if necessary */
    if (true == pProgress->isRangeIndicatorVisible())
    {
        bwin_rect rectProgress = win_settings.rect;
        int       spacer       = label_settings->bevel + 1;

        rectProgress.x      = spacer;
        rectProgress.y      = rectProgress.y + rectProgress.height / 2;
        rectProgress.width  = rectProgress.width - (spacer * 2);
        rectProgress.height = pProgress->getRangeIndicatorSize();

        if (NULL != pProgress->getProgressImage())
        {
            bwin_image_render(win, pProgress->getProgressImage(), pProgress->getProgressRenderMode(), &rectProgress, NULL, cliprect);
        }
        else
        {
            bwin_fill_rect(win, &rectProgress, 0xffbbbbbb, cliprect);
        }
    }

    /* draw progress if necessary*/
    if (true == pProgress->isProgressVisible())
    {
        uint16_t  level;
        bwin_rect rectProgress = win_settings.rect;
        int       spacer       = label_settings->bevel + 1;

        rectProgress.x      = spacer;
        rectProgress.y      = spacer;
        rectProgress.width  = rectProgress.width - (spacer * 2);
        rectProgress.height = rectProgress.height - (spacer * 2);

        /* adjust progress rect width based on actual level */
        level              = pProgress->getLevel();
        rectProgress.width = rectProgress.width * level / (uint16_t)-1;

        if (NULL != pProgress->getProgressImage())
        {
            bwin_image_render(win, pProgress->getProgressImage(), pProgress->getProgressRenderMode(), &rectProgress, NULL, cliprect);
        }
        else
        {
            uint16_t yMin     = rectProgress.y;
            uint16_t yMax     = rectProgress.y + rectProgress.height;
            uint16_t yMid     = yMin + ((yMax - yMin) / 2);
            uint32_t newColor = 0;
            uint8_t  percent  = 0;

            for (int line = yMin; line < yMid; line++)
            {
                percent  = (yMid - line) * 100 / (yMid - yMin);
                newColor = pProgress->colorConvert(pProgress->getColorTop(), pProgress->getColorMiddle(), percent);
                bwin_draw_line(win,
                        rectProgress.x,
                        line,
                        rectProgress.x + rectProgress.width,
                        line,
                        newColor,
                        cliprect);
            }

            for (int line = yMid; line < yMax; line++)
            {
                percent  = (yMax - line) * 100 / (yMax - yMid);
                newColor = pProgress->colorConvert(pProgress->getColorMiddle(), pProgress->getColorBottom(), percent);
                bwin_draw_line(win,
                        rectProgress.x,
                        line,
                        rectProgress.x + rectProgress.width,
                        line,
                        newColor,
                        cliprect);
            }
        }
    }

    pProgress->drawFocus(widget, label_settings, win_settings.rect, cliprect);

    if (true == pProgress->isTextVisible())
    {
        pProgress->drawText(widget, label_settings, win_settings.rect, cliprect);
    }
} /* progress_bwidget_draw */

CWidgetProgress::CWidgetProgress(
        const char *     strName,
        bwidget_engine_t engine,
        CWidgetBase *    pParentWidget,
        MRect            geometry,
        bwin_font_t      font,
        bwin_t           parentWin
        ) :
    CWidgetLabel(strName, engine, pParentWidget, geometry, font, parentWin),
    _showProgress(false),
    _imageProgress(NULL),
    _imageRenderMode(bwin_image_render_mode_tile),
    _color(0xFF80C42F),
    _colorTop(COLOR_BLUE_LIGHT),
    _colorMiddle(COLOR_BLUE),
    _colorBottom(COLOR_BLUE_DARK),
    _level(0),
    _showRange(false),
    _showText(true),
    _rangeSize(1)
{
    blabel_settings labelSettings;

    /* use custom draw routine so we can draw the background progress bar */
    blabel_get(getWidget(), &labelSettings);
    labelSettings.text_justify_horiz = bwidget_justify_horiz_center;
    labelSettings.widget.draw        = progress_bwidget_draw;
    blabel_set(getWidget(), &labelSettings);
}

CWidgetProgress::~CWidgetProgress()
{
    if (NULL != _imageProgress)
    {
        bwin_image_close(_imageProgress);
    }
}

void CWidgetProgress::setColor(uint32_t color)
{
    _color       = color;
    _colorTop    = color;
    _colorMiddle = color;
    _colorBottom = color;
    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetProgress::setColorTop(uint32_t color)
{
    _colorTop = color;
    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetProgress::setColorMiddle(uint32_t color)
{
    _colorMiddle = color;
    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetProgress::setColorBottom(uint32_t color)
{
    _colorBottom = color;
    bwin_repaint(bwidget_win(getWidget()), NULL);
}

eRet CWidgetProgress::loadProgressImage(
        const char *           filename,
        bwin_image_render_mode renderMode
        )
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != filename);

    if (NULL != _imageProgress)
    {
        bwin_image_close(_imageProgress);
    }

    _imageProgress = bwin_image_load(getWinEngine(), filename);
    CHECK_PTR_ERROR_GOTO("unable to load bwin image", _imageProgress, ret, eRet_ExternalError, error);

    _imageRenderMode = renderMode;

    bwin_repaint(bwidget_win(getWidget()), NULL);

error:
    return(ret);
} /* loadProgressImage */

void CWidgetProgress::showProgress(bool show)
{
    _showProgress = show;

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetProgress::showText(bool show)
{
    _showText = show;

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

/* sets the progress value (range 0-65535) */
void CWidgetProgress::setLevel(uint16_t level)
{
    _level = level;

    bwin_repaint(bwidget_win(getWidget()), NULL);
}