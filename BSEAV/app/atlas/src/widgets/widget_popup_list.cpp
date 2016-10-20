/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "widget_popup_list.h"
#include "widget_check_button.h"
#include "widget_base.h"
#include "string.h"
#include "bdbg.h"

BDBG_MODULE(widget_popup_list);

static void clickCallback(bwidget_t widget)
{
    CWidgetPopupList * pWidget = NULL;

    bwidget_settings bwidgetSettings;

    bwidget_get_settings(widget, &bwidgetSettings);
    pWidget = (CWidgetPopupList *)bwidgetSettings.data;
    BDBG_ASSERT(NULL != pWidget);

    if (true == pWidget->isVisible())
    {
        /* widget is visible, so handle click */
        pWidget->onClick(widget);
    }
}

#define ICON_BOX_SIZE  12

static void popup_list_bwidget_draw(
        bwidget_t         widget,
        const bwin_rect * cliprect
        )
{
    bwin_t             win = bwidget_win(widget);
    bwin_settings      win_settings;
    blabel_settings *  label_settings = NULL;
    bbutton_settings   buttonSettings;
    CWidgetPopupList * pButton = NULL;

    bbutton_get(widget, &buttonSettings);
    label_settings = &(buttonSettings.label);

    pButton = (CWidgetPopupList *)buttonSettings.label.widget.data;
    BDBG_ASSERT(NULL != pButton);

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

    {
        int i      = 0;
        int margin = 10;
        int x      = 0;
        int y      = 0;
        int width  = 0;
        int height = 0;

        MRect rect(win_settings.rect.x + win_settings.rect.width - ICON_BOX_SIZE - margin,
                   win_settings.rect.y + (win_settings.rect.height / 2) - (ICON_BOX_SIZE / 2),
                   ICON_BOX_SIZE,
                   ICON_BOX_SIZE);
        rect.setX(rect.x() - label_settings->bevel);

        x      = rect.x();
        y      = rect.y();
        width  = rect.width() + 1;
        height = rect.height();

        /* draw pulldown triangle indicator */
        if (fill_eGradient == pButton->getBackgroundFillMode())
        {
            bwin_rect rectLabel;
            uint32_t  colorTop    = COLOR_BLUE_SLATE + 6 * COLOR_STEP;
            uint32_t  colorMiddle = COLOR_BLUE_SLATE;
            uint32_t  colorBottom = COLOR_BLUE_SLATE - 2 * COLOR_STEP;

            rectLabel.x      = rect.x();
            rectLabel.y      = rect.y();
            rectLabel.width  = rect.width();
            rectLabel.height = rect.height();

            /* give drawing priority to background gradient if enabled */
            uint16_t yMin     = rectLabel.y;
            uint16_t yMax     = rectLabel.y + rectLabel.height;
            uint16_t yMid     = yMin + ((yMax - yMin) / 2);
            uint32_t newColor = 0;
            uint8_t  percent  = 0;

            for (int line = yMin; line < yMid; line++)
            {
                percent  = (yMid - line) * 100 / (yMid - yMin);
                newColor = pButton->colorConvert(colorTop, colorMiddle, percent);
                bwin_draw_line(win, x, y, x + width, y, newColor, cliprect);

                line++;
                percent  = (yMid - line) * 100 / (yMid - yMin);
                newColor = pButton->colorConvert(colorTop, colorMiddle, percent);
                y       += 1;
                /* draw 2nd line with cheapo anti-aliasing */
                bwin_draw_point(win, x, y, COLOR_GREY, cliprect);
                bwin_draw_line(win, x + 1, y, x + width, y, newColor, cliprect);
                bwin_draw_point(win, x + width - 1, y, COLOR_GREY, cliprect);
                x     += 1;
                y     += 1;
                width -= 2;
            }

            for (int line = yMid; 0 < width; line++)
            {
                percent  = (yMax - line) * 100 / (yMax - yMid);
                newColor = pButton->colorConvert(colorMiddle, colorBottom, percent);
                bwin_draw_line(win, x, y, x + width, y, newColor, cliprect);

                line++;
                percent  = (yMax - line) * 100 / (yMax - yMid);
                newColor = pButton->colorConvert(colorMiddle, colorBottom, percent);
                y       += 1;
                /* draw 2nd line with cheapo anti-aliasing */
                bwin_draw_point(win, x, y, COLOR_GREY, cliprect);
                bwin_draw_line(win, x + 1, y, x + width, y, newColor, cliprect);
                bwin_draw_point(win, x + width - 1, y, COLOR_GREY, cliprect);
                x     += 1;
                y     += 1;
                width -= 2;
            }
        }
        else
        {
            /* no gradient so use solid color fill */
            for (i = 0; 0 < width; i++)
            {
                bwin_draw_line(win, x, y, x + width, y, COLOR_BLUE_SLATE, cliprect);
                i++;
                y += 1;
                /* draw 2nd line with cheapo anti-aliasing */
                bwin_draw_point(win, x, y, COLOR_GREY, cliprect);
                bwin_draw_line(win, x, y, x + width, y, COLOR_BLUE_SLATE, cliprect);
                bwin_draw_point(win, x + width - 1, y, COLOR_GREY, cliprect);
                x     += 1;
                y     += 1;
                width -= 2;
            }
        }

        {
            bwin_rect rectFocus = { rect.x() - 1, rect.y() - 1, rect.width() + 2, rect.height() + 2 };
            pButton->drawFocus(widget, label_settings, rectFocus, cliprect);
        }
    }

    pButton->drawText(widget, label_settings, win_settings.rect, cliprect);
} /* popup_list_bwidget_draw */

CWidgetPopupList::CWidgetPopupList(
        const char *     strName,
        bwidget_engine_t engine,
        CWidgetBase *    pParentWidget,
        MRect            geometry,
        bwin_font_t      font,
        bwin_t           parentWin
        ) :
    CWidgetButton(strName, engine, pParentWidget, geometry, font, parentWin),
    _pListView(NULL),
    _showPopup(false),
    _pSelectedButton(NULL),
    _maxNumVisibleButtons(15)
{
    eRet             ret = eRet_Ok;
    bbutton_settings buttonSettings;

    bbutton_get(getWidget(), &buttonSettings);
    buttonSettings.click             = clickCallback;
    buttonSettings.label.widget.draw = popup_list_bwidget_draw;
    bbutton_set(getWidget(), &buttonSettings);

    _pListView = new CWidgetListView(MString("CWidgetPopupList::_pListView - ")+strName, getEngine(), this, geometry, font);
    CHECK_PTR_ERROR_GOTO("unable to allocate listview widget", _pListView, ret, eRet_OutOfMemory, error);
    _pListView->setScroll(false);
    _pListView->setZOrder(getZOrder() + 1);
    _pListView->setBevel(2);
    _pListView->show(false);

    /* force call to calc geometry - the list view coordinate system differs from that of
     * this widget because it must be able to draw outside the boundaries of this widget.
     * setGeometry() will account for this fact and can adjust the listview coords
     * accordingly. */
    setGeometry(geometry);

error:
    return;
}

CWidgetPopupList::~CWidgetPopupList()
{
    popup(false);
    clearButtons();
    DEL(_pListView);
}

void CWidgetPopupList::layout()
{
    _pListView->layout();
}

void CWidgetPopupList::popup(bool show)
{
    if (_showPopup == show)
    {
        return;
    }

    if (0 == _pListView->getWidgetList()->total())
    {
        return;
    }

    _showPopup = show;
    if (true == show)
    {
        bwin_t        framebufferWin = getWin();
        bwin_t        popupWin       = getWin();
        bwin_settings framebufferSettings;
        bwin_settings popupSettings;

        /* traverse up bwin hierarchy to get to framebuffer - save in framebufferWin */
        bwin_get(framebufferWin, &framebufferSettings);
        while (NULL != framebufferSettings.parent)
        {
            framebufferWin = framebufferSettings.parent;
            bwin_get(framebufferWin, &framebufferSettings);
        }

        /* traverse up bwin hierarchy to find framebuffer sized win. modal popups should
         * have a win this size so they can show popup lists outside their dimensions (bwin
         * restricts focus in modal windows to ancestors) */
        bwin_get(popupWin, &popupSettings);
        while (NULL != popupSettings.parent)
        {
            popupWin = popupSettings.parent;
            bwin_get(popupWin, &popupSettings);

            if ((framebufferSettings.rect.width == popupSettings.rect.width) &&
                (framebufferSettings.rect.height == popupSettings.rect.height))
            {
                break;
            }
        }

        /* reparent to _popupWin so we can draw the list widget outside the
         * boundaries of this widget */
        _pListView->setParentWin(popupWin);
        _pListView->show(true, true);

        if (NULL != getSelection())
        {
            /* select button and scroll so it is in view (if necessary) */
            _pSelectedButton->setFocus();

            if (_maxNumVisibleButtons < _pListView->getNumButtons())
            {
                _pListView->scrollTo(_pListView->getItemListIndex(getSelection()->getWidget()));
            }
        }
    }
    else
    {
        /* reset parent bwin */
        _pListView->setParentWin(getWin());
        _pListView->show(false);

        /* closing popup, so return focus to button */
        setFocus();
    }
} /* popup */

/* override setGeometry so we can adjust our internal list view everytime the popup list
 * geometry changes */
void CWidgetPopupList::setGeometry(MRect geometry)
{
    bwin_t        win = getWin();
    bwin_settings winSettings;
    int           x = 0;
    int           y = 0;
    MRect         rectListView;

    /* call base class */
    CWidgetButton::setGeometry(geometry);

    /* our internal list view's parent is actually the popupWin given in the constructor.
     * (so it can draw outside the bounds of the popup list and menu)
     * so we will set the position and width to be relative to that bwin's coordinate system.
     * note that the height is auto generated by the listview's addButton() method so we
     * won't alter that here */

    /* traverse up thru window heirarchy to add up relative x and y coordinates until we get
     * to the listview's parent bwin (we'll also check for NULL but it should never happen) */
    while ((NULL != win) && (_pListView->getParent()->getWin() != win))
    {
        bwin_get(win, &winSettings);
        x += winSettings.x;
        y += winSettings.y;

        win = winSettings.parent;
    }

    rectListView.setX(x);
    rectListView.setY(y);
    rectListView.setWidth(geometry.width());
    _pListView->setGeometry(rectListView);

    layout();
} /* setGeometry */

CWidgetCheckButton * CWidgetPopupList::addButton(
        const char * name,
        uint16_t     width,
        uint16_t     height
        )
{
    eRet                 ret     = eRet_Ok;
    CWidgetCheckButton * pButton = NULL;
    bbutton_settings     buttonSettings;
    MRect                rectButton;

    bbutton_get(getWidget(), &buttonSettings);

    pButton = new CWidgetCheckButton("CWidgetCheckButton check button", getEngine(), this, MRect(0, 0, width, height), buttonSettings.label.font);
    CHECK_PTR_ERROR_GOTO("unable to allocate check button widget", pButton, ret, eRet_OutOfMemory, error);
    if (NULL != name)
    {
        pButton->setText(name, bwidget_justify_horiz_left, bwidget_justify_vert_middle);
    }

    /* propogate popup list background fill mode to newly created checkbutton */
    pButton->setBackgroundFillMode(getBackgroundFillMode());

    _pListView->add(pButton);
    _pListView->setScroll((_pListView->getNumButtons() >= _maxNumVisibleButtons) ? true : false);

    /* keep track of new check buttons */
    _listCheckButtons.add(pButton);

    if (NULL == _pSelectedButton)
    {
        /* set selected button if not set */
        select(pButton);
    }

error:
    return(pButton);
} /* addButton */

/* search internal listview for button matching given value */
CWidgetCheckButton * CWidgetPopupList::findButton(long value)
{
    MList <CWidgetListItem> *  pWidgetList = _pListView->getWidgetList();
    CWidgetListItem *          pItem       = NULL;
    CWidgetCheckButton *       pButton     = NULL;
    MListItr <CWidgetListItem> itr(pWidgetList);

    for (pItem = itr.first(); pItem; pItem = itr.next())
    {
        if (value == pItem->getButton()->getValue())
        {
            break;
        }
    }

    if (NULL != pItem)
    {
        pButton = ((CWidgetCheckButton *)pItem->getButton());
    }

    return(pButton);
} /* findButton */

void CWidgetPopupList::clearButtons()
{
    _pListView->clear();
    _listCheckButtons.clear();
    _pSelectedButton = NULL;
}

eRet CWidgetPopupList::select(CWidgetCheckButton * pButton)
{
    eRet ret = eRet_Ok;

    MList <CWidgetListItem> * pWidgetList = NULL;
    CWidgetListItem *         pListItem   = NULL;
    bool                      bFound      = false;

    pWidgetList = _pListView->getWidgetList();
    CHECK_PTR_ERROR_GOTO("no buttons added to list view - aborting select request", pWidgetList, ret, eRet_NotAvailable, error);

    /* check the given button while unchecking all others in list */
    {
        MListItr <CWidgetListItem> itr(pWidgetList);
        CWidgetCheckButton *       pCheckButton = NULL;

        for (pListItem = itr.first(); NULL != pListItem; pListItem = itr.next())
        {
            pCheckButton = (CWidgetCheckButton *)pListItem->getButton();

            if (pCheckButton == pButton)
            {
                pCheckButton->setCheck(true);
                /* update button text to match new selection */
                setText(pCheckButton->getText().s());
                _pSelectedButton = pCheckButton;
                bFound           = true;
            }
            else
            {
                pCheckButton->setCheck(false);
            }
        }
    }

    if (false == bFound)
    {
        ret = eRet_NotAvailable;
    }

error:
    return(ret);
} /* select */

eRet CWidgetPopupList::select(const char * name)
{
    eRet ret = eRet_Ok;

    MList <CWidgetListItem> * pWidgetList = NULL;
    CWidgetListItem *         pListItem   = NULL;
    bool                      bFound      = false;

    pWidgetList = _pListView->getWidgetList();
    CHECK_PTR_ERROR_GOTO("no buttons added to list view - aborting select request", pWidgetList, ret, eRet_NotAvailable, error);

    /* check the given button while unchecking all others in list */
    {
        MListItr <CWidgetListItem> itr(pWidgetList);
        CWidgetCheckButton *       pCheckButton = NULL;

        for (pListItem = itr.first(); NULL != pListItem; pListItem = itr.next())
        {
            pCheckButton = (CWidgetCheckButton *)pListItem->getButton();

            if (pCheckButton->getText() == MString(name))
            {
                pCheckButton->setCheck(true);
                /* update button text to match new selection */
                setText(pCheckButton->getText().s());
                _pSelectedButton = pCheckButton;
                bFound           = true;
            }
            else
            {
                pCheckButton->setCheck(false);
            }
        }
    }

    if (false == bFound)
    {
        ret = eRet_NotAvailable;
    }

error:
    return(ret);
} /* select */

eRet CWidgetPopupList::select(const long value)
{
    eRet ret = eRet_Ok;

    MList <CWidgetListItem> * pWidgetList = NULL;
    CWidgetListItem *         pListItem   = NULL;
    bool                      bFound      = false;

    pWidgetList = _pListView->getWidgetList();
    CHECK_PTR_ERROR_GOTO("no buttons added to list view - aborting select request", pWidgetList, ret, eRet_NotAvailable, error);

    /* check the given button while unchecking all others in list */
    {
        MListItr <CWidgetListItem> itr(pWidgetList);
        CWidgetCheckButton *       pCheckButton = NULL;

        for (pListItem = itr.first(); NULL != pListItem; pListItem = itr.next())
        {
            pCheckButton = (CWidgetCheckButton *)pListItem->getButton();

            if (pCheckButton->getValue() == value)
            {
                pCheckButton->setCheck(true);
                /* update button text to match new selection */
                setText(pCheckButton->getText().s());
                _pSelectedButton = pCheckButton;
                bFound           = true;
            }
            else
            {
                pCheckButton->setCheck(false);
            }
        }
    }

    if (false == bFound)
    {
        ret = eRet_NotAvailable;
    }

error:
    return(ret);
} /* select */

void CWidgetPopupList::onClick(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }

    if (this == pWidget)
    {
        /* click on popup button */
        popup(true);
    }
    else
    {
        /* click on check button in popup list view - select it
         * and clear others, then hide popup list view */
        select((CWidgetCheckButton *)pWidget);
        popup(false);

        /* up to now, we have hijacked the button click internally - now forward
         * click to parent widget for possible additional handling */
        getParent()->onClick(widget);
    }
} /* onClick */

void CWidgetPopupList::drawFocus(
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

void CWidgetPopupList::drawText(
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
            x -= ICON_BOX_SIZE + 16;
        }
        else /* bwidget_justify_horiz_left */
        {
            x += pLabelSettings->text_margin;
        }

        bwin_draw_text(win, pLabelSettings->font, x, y,
                pLabelSettings->text, -1, pLabelSettings->text_color,
                pRectClip);
    }
} /* drawText */