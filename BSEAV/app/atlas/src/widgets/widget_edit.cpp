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

#include "screen.h"
#include "widget_edit.h"
#include "widget_base.h"
#include "string.h"
#include "bdbg.h"

BDBG_MODULE(widget_edit);

static int edit_bwidget_key_down(
        bwidget_t         widget,
        const bwidget_key key
        )
{
    eRet            ret = eRet_NotAvailable; /* assume key is not consumed */
    CWidgetEdit *   pEdit;
    blabel_settings labelSettings;

    blabel_get(widget, &labelSettings);
    pEdit = (CWidgetEdit *)labelSettings.widget.data;
    BDBG_ASSERT(NULL != pEdit);

    ret = pEdit->onKeyDown(widget, key);

    return((eRet_Ok == ret) ? 0 : -1);
}

/* override default button draw routine to add cursor */
static void edit_bwidget_draw(
        bwidget_t         widget,
        const bwin_rect * cliprect
        )
{
    bwin_t            win = bwidget_win(widget);
    bwin_settings     win_settings;
    blabel_settings * label_settings;
    bbutton_settings  buttonSettings;
    CWidgetButton *   pButton;
    CWidgetEdit *     pEdit;

    bbutton_get(widget, &buttonSettings);
    label_settings = &(buttonSettings.label);

    pButton = (CWidgetButton *)label_settings->widget.data;
    BDBG_ASSERT(NULL != pButton);

    pEdit = (CWidgetEdit *)label_settings->widget.data;
    BDBG_ASSERT(NULL != pEdit);

    /* TODO: for efficieny, we may want bwidgets to have access to bwin private structures. */
    bwin_get(win, &win_settings);

    /* only draw if something's within the cliprect */
    if (cliprect && !BWIN_RECT_ISINTERSECTING(&win_settings.rect, cliprect))
    {
        return;
    }

    pButton->drawBackground(widget, label_settings, win_settings.rect, cliprect);
    pButton->drawBevel(widget, label_settings, win_settings.rect, cliprect);
    pButton->drawFocus(widget, label_settings, win_settings.rect, cliprect);
    pButton->drawText(widget, label_settings, win_settings.rect, cliprect);
} /* edit_bwidget_draw */

CWidgetEdit::CWidgetEdit(
        const char *     strName,
        bwidget_engine_t engine,
        CWidgetBase *    pParentWidget,
        MRect            geometry,
        bwin_font_t      font,
        bwin_t           parentWin
        ) : /* optional */
    CWidgetButton(strName, engine, pParentWidget, geometry, font, parentWin),
    _position(0)
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    buttonSettings.label.widget.draw        = edit_bwidget_draw;
    buttonSettings.label.widget.key_down    = edit_bwidget_key_down;
    buttonSettings.label.text_justify_horiz = bwidget_justify_horiz_right;
    buttonSettings.label.text_margin        = 5;
    bbutton_set(getWidget(), &buttonSettings);

    setBackgroundGradient(GRADIENT_TOP, GRADIENT_MIDDLE, GRADIENT_BOTTOM);
}

MString CWidgetEdit::getText()
{
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    return(MString(buttonSettings.label.text));
}

eRet CWidgetEdit::onKeyDown(
        bwidget_t   widget,
        bwidget_key key
        )
{
    eRet            ret = eRet_NotAvailable; /* assume key is not consumed */
    blabel_settings labelSettings;

    blabel_get(widget, &labelSettings);

    switch (key)
    {
    case bwidget_key_left:
        if (0 < getPosition())
        {
            if (labelSettings.text_justify_horiz == bwidget_justify_horiz_right)
            {
                setPosition(getPosition() - 1);
            }
            else
            {
                removeChar(&_position, false);
            }
            ret = eRet_Ok; /* consume key */
        }
        break;

    case bwidget_key_right:
        if (getPosition() < MString(_pText).length())
        {
            if (labelSettings.text_justify_horiz == bwidget_justify_horiz_right)
            {
                removeChar(&_position, true);
            }
            else
            {
                setPosition(getPosition() + 1);
            }
            ret = eRet_Ok; /* consume key */
        }
        break;

    default:
        if ((32 <= key) && (255 >= key))
        {
            BDBG_MSG(("widget edit ascii key received! 0x%X", key));
            addChar(key, &_position);
            ret = eRet_Ok; /* consume key */
        }
        break;
    } /* switch */

    return(ret);
} /* onKeyDown */

void CWidgetEdit::setPosition(uint16_t position)
{
    _position = position;
    bwin_repaint(bwidget_win(getWidget()), NULL);
}

void CWidgetEdit::addChar(
        char       ch,
        uint16_t * pPosition
        )
{
    int length = MString(_pText).length();

    if (length < (EDIT_MAX_TEXT - 1))
    {
        /* shift letters right to make room for new char */
        for (int i = length; i >= *pPosition; i--)
        {
            _pText[i + 1] = _pText[i];
        }
        _pText[*pPosition] = ch;

        (*pPosition)++;

        bwin_repaint(bwidget_win(getWidget()), NULL);
    }
} /* addChar */

void CWidgetEdit::removeChar(
        uint16_t * pPosition,
        bool       right
        )
{
    int length = MString(_pText).length();

    if (true == right)
    {
        if (*pPosition < length)
        {
            /* shift letters from position to length forward */
            for (int i = *pPosition; i < length; i++)
            {
                _pText[i] = _pText[i + 1];
            }
            bwin_repaint(bwidget_win(getWidget()), NULL);
        }
    }
    else
    {
        if (*pPosition > 0)
        {
            /* shift letters from position to length backwards */
            for (int i = *pPosition; i < length; i++)
            {
                _pText[i - 1] = _pText[i];
            }

            (*pPosition)--;

            bwin_repaint(bwidget_win(getWidget()), NULL);
        }
    }
} /* removeChar */

void CWidgetEdit::drawText(
        bwidget_t         widget,
        blabel_settings * pLabelSettings,
        const bwin_rect   rect,
        const bwin_rect * pRectClip
        )
{
    bwin_t win = bwidget_win(widget);

    if (pLabelSettings->text)
    {
        int width, height, base, x, y;
        bwin_measure_text(pLabelSettings->font, pLabelSettings->text, -1,
                &width, &height, &base);

        /* calculate vertical justification */
        y = 0;
        if (pLabelSettings->text_justify_vert == bwidget_justify_vert_middle)
        {
            y = (rect.height - height) / 2;
        }
        else
        if (pLabelSettings->text_justify_vert == bwidget_justify_vert_bottom)
        {
            y += pLabelSettings->text_margin;
        }
        else /* bwidget_justify_vert_top */
        {
            y  = rect.height - height;
            y -= pLabelSettings->text_margin;
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
            x -= pLabelSettings->bevel;
        }
        else /* bwidget_justify_horiz_left */
        {
            x += pLabelSettings->text_margin;
            x += pLabelSettings->bevel;
        }

        MString drawText   = pLabelSettings->text;
        int     drawWidth  = 0;
        int     drawHeight = 0;
        int     drawBase   = 0;

        /* draw text before cursor */
        if ((0 < getPosition()) && drawText.length())
        {
            bwin_measure_text(pLabelSettings->font, pLabelSettings->text, getPosition(),
                    &drawWidth, &drawHeight, &drawBase);
            bwin_draw_text(win, pLabelSettings->font, x, y,
                    pLabelSettings->text, getPosition(), pLabelSettings->text_color,
                    pRectClip);
        }

        /* draw cursor */
        if (widget == bwidget_get_focus(getEngine()))
        {
            int       margin      = pLabelSettings->bevel;
            unsigned  widthCursor = 2;
            bwin_rect rectCursor  = { x + drawWidth - (int)widthCursor / 2,
                                      margin,
                                      widthCursor,
                                      (unsigned)(rect.height - margin * 2) };
            bwin_fill_rect(win, &rectCursor, getTextColor(), pRectClip);
        }

        /* draw text after cursor */
        if ((drawText.length() > getPosition()) && drawText.length())
        {
            bwin_draw_text(win, pLabelSettings->font, x + drawWidth, y,
                    ((const char *)pLabelSettings->text) + getPosition(),
                    drawText.length() - getPosition(),
                    pLabelSettings->text_color, pRectClip);
        }
    }
} /* drawText */