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

#include "widget_listview.h"
#include "widget_button.h"
#include "string.h"
#include "bdbg.h"

BDBG_MODULE(widget_listview);

CWidgetListView::CWidgetListView(
        const char *     strName,
        bwidget_engine_t engine,
        CWidgetBase *    pParentWidget,
        MRect            geometry,
        bwin_font_t      font,
        bwin_t           parentWin
        ) :
    CWidgetLabel(strName, engine, pParentWidget, geometry, font, parentWin),
    _separatorSize(1),
    _scroll(true),
    _autoResize(false),
    _lockFocus(false)
{
    blabel_settings labelSettings;

    _widgetList.clear();

    blabel_get(_widget, &labelSettings);
    labelSettings.bevel              = 3;
    labelSettings.text_color         = 0xFF82C345;
    labelSettings.text_justify_horiz = bwidget_justify_horiz_left;
    labelSettings.text_margin        = 5;
    labelSettings.background_color   = 0xFFCCCCCC;
    blabel_set(_widget, &labelSettings);

    /* force key press callbacks to come to this object (not the parent).
     * we need to do this to handle scrolling within the list view */
    {
        blabel_get(_widget, &labelSettings);
        labelSettings.widget.data = this;
        blabel_set(_widget, &labelSettings);

        _pParent = this;
    }

    B_Time_Get(&_timeLastKeyPress);
}

CWidgetListView::~CWidgetListView()
{
    _widgetList.clear();
}

void CWidgetListView::show(
        bool bShow,
        bool bLockFocus
        )
{
    show(bShow);
    lockFocus(bLockFocus);
}

void CWidgetListView::show(bool bShow)
{
    CWidgetLabel::show(bShow);

    if (true == bShow)
    {
        layout();
    }
    else
    {
        lockFocus(false);
    }
}

/* adds a button widget to this list view.  we require button widgets because all widgets
 * must be selectable (but since button widgets can simulate label widgets this should be
 * a non-restrictive requirment.  note that you must still call layout() to properly layout
 * the listview after adding new elements */
void CWidgetListView::add(CWidgetButton * pButton)
{
    CWidgetListItem * pWidgetItem = NULL;
    eRet              ret         = eRet_Ok;

    BDBG_ASSERT(NULL != pButton);

    pButton->setTextMargin(5);
    pButton->setBevel(0);

    /* hide new widget - layout() will show */
    pButton->show(false);

    pWidgetItem = new CWidgetListItem(this, pButton);
    CHECK_PTR_ERROR_GOTO("unable to allocate widget list item", pWidgetItem, ret, eRet_OutOfMemory, error);
    _widgetList.add(pWidgetItem);

    layout();

error:
    return;
} /* add */

void CWidgetListView::remove(CWidgetButton * pButton)
{
    CWidgetListItem * pWidgetItem = NULL;

    BDBG_ASSERT(_widgetList.total() > 0);

    /* search list for matching button and remove it. */
    for (pWidgetItem = _widgetList.first(); pWidgetItem; pWidgetItem = _widgetList.next())
    {
        if (pWidgetItem->getButton() == pButton)
        {
            _widgetList.remove(pWidgetItem);
            pButton->show(false);
            break;
        }
    }

    if (false == _scroll)
    {
        /* scrolling is off so resize to fit */
        layout();
    }
} /* remove */

CWidgetButton * CWidgetListView::remove(unsigned index)
{
    CWidgetListItem * pWidgetItem = NULL;
    CWidgetButton *   pButton     = NULL;

    BDBG_ASSERT(_widgetList.total() > (int)index);

    /* _widgetList is an auto list so we must retrieve/save
     * button pointer before removing. */
    pWidgetItem = _widgetList.get(index);
    pButton     = pWidgetItem->getButton();
    _widgetList.remove(index);

    pButton->show(false);

    if (false == _scroll)
    {
        /* scrolling is off so resize to fit */
        layout();
    }

    return(pButton);
} /* remove */

void CWidgetListView::setActive(
        CWidgetButton * pWidgetItem,
        bool            bActive
        )
{
    CWidgetListItem * pItem = NULL;

    MListItr <CWidgetListItem> itr(&_widgetList);

    BDBG_ASSERT(_widgetList.total() > 0);

    /* search list for matching button and activate it it. */
    for (pItem = itr.first(); pItem; pItem = itr.next())
    {
        if (pItem->getButton() == pWidgetItem)
        {
            pItem->setActive(bActive);
            break;
        }
    }

    if (NULL != pItem)
    {
        layout();
    }
} /* setActive */

bool CWidgetListView::isActive(CWidgetButton * pWidgetItem)
{
    CWidgetListItem * pItem   = NULL;
    bool              bActive = false;

    MListItr <CWidgetListItem> itr(&_widgetList);

    BDBG_ASSERT(_widgetList.total() > 0);

    /* search list for matching button and activate it it. */
    for (pItem = itr.first(); pItem; pItem = itr.next())
    {
        if (pItem->getButton() == pWidgetItem)
        {
            bActive = pItem->isActive();
            break;
        }
    }

    return(bActive);
} /* setActive */

static int compareWidgetListItems(
        CWidgetListItem * item1,
        CWidgetListItem * item2
        )
{
    return(strcmp(item1->getButton()->getText().s(), item2->getButton()->getText().s()));
}

eRet CWidgetListView::sort(CWidgetButton * pButtonFocus)
{
    eRet    ret               = eRet_Ok;
    int32_t indexFirstVisible = getFirstVisibleIndex();
    int32_t offsetFocusOrig   = 0;

    MListItr <CWidgetListItem> itr(&_widgetList);
    CWidgetListItem *          pItemFocus = NULL;

    if (NULL != pButtonFocus)
    {
        /* search for given focused button */
        for (pItemFocus = itr.first(); pItemFocus; pItemFocus = itr.next())
        {
            if (pItemFocus->getButton() == pButtonFocus)
            {
                break;
            }

            if (true == pItemFocus->getButton()->isVisible())
            {
                /* count offset from first visible item so we can reproduce this
                 * after sorting the list
                 */
                offsetFocusOrig++;
            }
        }
    }

    _widgetList.sort(compareWidgetListItems);
    /* list order changed so update first visible index */
    indexFirstVisible = getFirstVisibleIndex();

    /* because our list order has changed, we will need to find the index of the
     * first visible list item such that our given pButonFocus is on screen in the
     * same position as before the sort.
     *
     * search for originally given button and then back up using our saved
     * offsetFocusOrig value to find the corresponding first item index.
     */
    if (NULL != pButtonFocus)
    {
        for (pItemFocus = itr.first(); pItemFocus; pItemFocus = itr.next())
        {
            if (pItemFocus->getButton() == pButtonFocus)
            {
                /* found button with focus */
                break;
            }
        }

        if (NULL != pItemFocus)
        {
            CWidgetListItem * pItemFirstVisible = pItemFocus;

            /* back up offsetFocusOrig times in list to get to the corresponding
             * first visible item in our newly sorted list.
             */
            for (pItemFocus = itr.prev(); (NULL != pItemFocus) && (offsetFocusOrig > 0); pItemFocus = itr.prev())
            {
                pItemFirstVisible = pItemFocus;
                offsetFocusOrig--;
            }

            /* keep focused item on screen */
            if (NULL != pItemFirstVisible)
            {
                indexFirstVisible = getItemListIndex(pItemFirstVisible->getButton()->getWidget());
            }
        }
    }
    else
    {
        /* no given focus in list so default to list head */
        indexFirstVisible = 0;
    }

    /* layout list view starting with calculated index of first visible list item */
    layout(indexFirstVisible);

    return(ret);
} /* sort */

void CWidgetListView::clear()
{
    _widgetList.clear();
}

/* get the index of the first visible item in the listview. this can be used
 * to re-layout the listview while scrolling to the same location in the list.
 * calling layout() with no index will default to scrolling to the top of this list. */
int32_t CWidgetListView::getFirstVisibleIndex()
{
    int32_t index = 0;

    MListItr <CWidgetListItem> itr(&_widgetList);
    CWidgetListItem *          pListItem = NULL;

    for (pListItem = itr.first(); pListItem; pListItem = itr.next())
    {
        if (true == pListItem->getButton()->isVisible())
        {
            /* found first visible item in listview */
            index = getItemListIndex(pListItem->getButton()->getWidget());
            break;
        }
    }
    if (NULL == pListItem)
    {
        /* no top index found so default to 0 */
        index = 0;
    }

    return(index);
} /* getFirstVisibleIndex */

/* layout list view using widgets in widget list.  start with 'index' and go 'topDown' (or bottomUp).
 * note: if ALL of the list items fit in the listview, then layout() will force index=0 and topDown=true */
eRet CWidgetListView::layout(
        int  index,
        bool topDown
        )
{
    eRet              ret       = eRet_Ok;
    CWidgetListItem * pListItem = NULL;
    MRect             listRect  = getGeometry();
    MRect             itemRect;
    blabel_settings   listViewSettings;
    int               margin                = 0;
    bool              bItemsSmallerThanList = false;
    int               numActiveListItems    = 0;

    MListItr <CWidgetListItem> itr(&_widgetList);

    BDBG_MSG(("layout() index:%d topDown:%d", index, topDown));

    if (0 > index)
    {
        index = getFirstVisibleIndex();
    }

    /* determine margin (bevel size) for list view - contents must shrink appropriately */
    blabel_get(getWidget(), &listViewSettings);
    margin = listViewSettings.bevel;

    /* if scrolling is OFF, resize list view to fit all buttons */
    if ((false == _scroll) || (true == _autoResize))
    {
        unsigned totalHeight = 0;
        MRect    rectGeom;

        /* find height of contents */
        for (pListItem = itr.first(); pListItem; pListItem = itr.next())
        {
            if (true == pListItem->isActive())
            {
                rectGeom     = pListItem->getButton()->getGeometry();
                totalHeight += rectGeom.height() + getSeparatorSize();
            }
        }

        /* grow for margins*/
        totalHeight += margin * 2;

        /* resize listview */
        listRect = getGeometry();
        listRect.setHeight(totalHeight);
        CWidgetLabel::setGeometry(listRect);
    }

    /* set listRect to contain the available space in list view */
    listRect = getGeometry();
    listRect.setX(margin);
    listRect.setY(margin);
    listRect.grow(margin * 2 * -1); /* shrink for margins */

    /* sanity check!
     * before we begin layout, we must determine if ALL the widgets will
     * fit in the listview. if so, then we will force topDown */
    {
        unsigned totalHeight = 0;
        MRect    rectGeom;
        for (pListItem = itr.first(); pListItem; pListItem = itr.next())
        {
            if (true == pListItem->isActive())
            {
                rectGeom     = pListItem->getButton()->getGeometry();
                totalHeight += rectGeom.height() + getSeparatorSize();
                numActiveListItems++;
            }
        }

        if (listRect.height() > totalHeight)
        {
            /* all items will fit in list view so force topDown layout */
            topDown               = true;
            index                 = 0;
            bItemsSmallerThanList = true;
        }
    }

    /* hide all buttons */
    for (pListItem = itr.first(); pListItem; pListItem = itr.next())
    {
        pListItem->getButton()->show(false);
    }

    if (true == topDown)
    {
        /* sanity chck!
         * if given index item is near the bottom of the list, adjust index to keep a
         * max number of items visible.  so if the last item index is given, we will
         * adjust the start index such that the item is drawn at the bottom of the
         * list control.
         */
        if ((false == bItemsSmallerThanList) && (0 < numActiveListItems))
        {
            unsigned maxListItems    = 0;
            MRect    rectGeomItem    = itr.at(getFirstVisibleIndex())->getButton()->getGeometry();
            unsigned itemHeight      = rectGeomItem.height() + getSeparatorSize();
            unsigned maxVisibleItems = listRect.height() / itemHeight;
            unsigned toEndOfList     = 0;

            for (pListItem = itr.at(index); pListItem; pListItem = itr.next())
            {
                if (true == pListItem->isActive())
                {
                    toEndOfList++;
                }
            }

            if (toEndOfList < maxVisibleItems)
            {
                CWidgetListItem * pListItemAtIndex = itr.at(index);
                unsigned          fromEndOfList    = 0;

                /* given index is on the last visible page of items so adjust
                 * index to the first item on the last visible page */
                for (pListItem = itr.last(); pListItem != pListItemAtIndex; pListItem = itr.prev())
                {
                    if (pListItem && (true == pListItem->isActive()))
                    {
                        fromEndOfList++;
                    }
                }

                index -= (maxVisibleItems - fromEndOfList - 1);
            }
        }

        /* layout view top down */
        for (pListItem = itr.at(index); pListItem; pListItem = itr.next())
        {
            if (false == pListItem->isActive())
            {
                /* list item inactive so skip it */
                pListItem->getButton()->show(false);
                continue;
            }

            /* adjust new item's geometry to fit first available spot in listRect */
            itemRect = pListItem->getButton()->getGeometry();
            itemRect.setX(listRect.x());
            itemRect.setY(listRect.y());
            itemRect.setWidth(listRect.width());

            if ((0 < listRect.height()) && (listRect.intersects(itemRect)))
            {
                BDBG_MSG(("widget (%s) fits in listview!", pListItem->getButton()->CSubject::getName().s()));
                /* item fits so apply geometry changes to item widget */
                pListItem->getButton()->setGeometry(itemRect);
                pListItem->getButton()->show(true);

                /* shrink available space */
                listRect.setY(listRect.y() + itemRect.height());
                listRect.setBottom(listRect.bottom() - itemRect.height());

                /* adjust available space for separator */
                listRect.setY(listRect.y() + getSeparatorSize());
                listRect.setBottom(listRect.bottom() - getSeparatorSize());
            }
            else
            {
                BDBG_MSG(("widget does NOT fit in listview!"));
                pListItem->getButton()->show(false);
            }
        }
    }
    else
    {
        /* layout view bottom up */
        for (pListItem = itr.at(index); pListItem; pListItem = itr.prev())
        {
            if (false == pListItem->isActive())
            {
                /* list item inactive so skip it */
                pListItem->getButton()->show(false);
                continue;
            }

            /* adjust new item's geometry to fit last available spot in listRect */
            itemRect = pListItem->getButton()->getGeometry();
            itemRect.setX(listRect.x());
            itemRect.setY(listRect.bottom() - itemRect.height());
            /* adjust Y for separator */
            itemRect.setY(itemRect.y() - getSeparatorSize());
            itemRect.setWidth(listRect.width());

            if (listRect.intersects(itemRect))
            {
                BDBG_MSG(("widget (%s) fits in listview!", pListItem->getButton()->CSubject::getName().s()));
                /* item fits so apply geometry changes to item widget */
                pListItem->getButton()->setGeometry(itemRect);
                pListItem->getButton()->show(true);

                /* shrink available space */
                listRect.setBottom(listRect.bottom() - itemRect.height());

                /* adjust available space for separator */
                listRect.setBottom(listRect.bottom() - getSeparatorSize());
            }
            else
            {
                BDBG_MSG(("widget does NOT fit in listview!"));
                pListItem->getButton()->show(false);
            }
        }
    }

    return(ret);
} /* layout */

void CWidgetListView::setGeometry(MRect geometry)
{
    CWidgetLabel::setGeometry(geometry); /* call base class */

    layout();
}

/* returns index of item in list (or it's ancestor) that corresponds with the given widget.
 * otherwise returns -1 */
int CWidgetListView::getItemListIndex(bwidget_t widget)
{
    CWidgetListItem * pListItem = NULL;
    int               index     = -1;

    BDBG_ASSERT(NULL != widget);

    MListItr <CWidgetListItem> itr(&_widgetList);
    for (pListItem = itr.first(); pListItem; pListItem = itr.next())
    {
        if (widget == pListItem->getButton()->getWidget())
        {
            /* widget matches pListItem*/
            break;
        }
        else
        {
            bwidget_settings bwidgetSettings;
            bwin_t           currentWin;
            bwin_t           parentWin;

            bwidget_get_settings(pListItem->getButton()->getWidget(), &bwidgetSettings);
            currentWin = bwidgetSettings.win;
            bwidget_get_settings(widget, &bwidgetSettings);
            parentWin = bwidgetSettings.window.parent;

            /* pListItem does not match given widget - check ancestors.
             * the given widget may be an ancestor of pListItem.  this can
             * occur if the listview is populated with buttons that have
             * focusable sub widgets. */
            while ((NULL != parentWin) && (currentWin != parentWin))
            {
                bwin_settings bwinSettings;

                bwin_get(parentWin, &bwinSettings);
                parentWin = bwinSettings.parent;
            }

            if (NULL != parentWin)
            {
                /* ancestor of widget matches current pListItem */
                break;
            }
        }
    }

    if (NULL != pListItem)
    {
        /* pListItem matches widget or ancestor of widget */
        index = itr.index(pListItem);
    }

    return(index);
} /* getItemListIndex */

bool CWidgetListView::isFirstVisible(CWidgetButton * pWidget)
{
    bool visible = false;

    MListItr <CWidgetListItem> itr(&_widgetList);
    CWidgetListItem *          pWidgetFirst = NULL;

    BDBG_ASSERT(NULL != pWidget);

    for (pWidgetFirst = itr.first(); pWidgetFirst; pWidgetFirst = itr.next())
    {
        if (pWidgetFirst->getButton()->isVisible())
        {
            break;
        }
    }

    if (NULL != pWidgetFirst)
    {
        if ((NULL != pWidgetFirst->getButton()) && (pWidget == pWidgetFirst->getButton()))
        {
            visible = true;
        }
    }

    return(visible);
} /* isFirstVisible */

bool CWidgetListView::isLastVisible(CWidgetButton * pWidget)
{
    bool visible = false;

    MListItr <CWidgetListItem> itr(&_widgetList);
    CWidgetListItem *          pWidgetLast = NULL;

    BDBG_ASSERT(NULL != pWidget);

    for (pWidgetLast = itr.last(); pWidgetLast; pWidgetLast = itr.prev())
    {
        if (pWidgetLast->getButton()->isVisible())
        {
            break;
        }
    }

    if (NULL != pWidgetLast)
    {
        if ((NULL != pWidgetLast->getButton()) && (pWidget == pWidgetLast->getButton()))
        {
            visible = true;
        }
    }

    return(visible);
} /* isLastVisible */

bool CWidgetListView::isFullyVisible(CWidgetButton * pWidget)
{
    BDBG_ASSERT(NULL != pWidget);

    if (false == pWidget->isVisible())
    {
        return(false);
    }

    /* pWidget is already visible so check to make sure it is
     * fully contained within the list view (could be only partially shown) */
    MRect listRect;
    MRect itemRect;
    listRect = getGeometry();
    listRect.setX(0); /* itemRect will be relative to listRect so set to 0 for comparison */
    listRect.setY(0); /* itemRect will be relative to listRect so set to 0 for comparison */
    itemRect = pWidget->getGeometry();

    BDBG_MSG(("listRect - x:%d y:%d w:%d h:%d", listRect.x(), listRect.y(), listRect.width(), listRect.height()));
    BDBG_MSG(("itemRect - x:%d y:%d w:%d h:%d", itemRect.x(), itemRect.y(), itemRect.width(), itemRect.height()));

    return(listRect.contains(itemRect) ? true : false);
} /* isFullyVisible */

void CWidgetListView::setScroll(bool scroll)
{
    _scroll = scroll;
    if (false == _scroll)
    {
        layout();
    }
}

void CWidgetListView::setAutoResize(bool autoResize)
{
    _autoResize = autoResize;
    if (true == _autoResize)
    {
        layout();
    }
}

void CWidgetListView::scrollTo(int index)
{
    CWidgetListItem * pWidget = _widgetList.at(index);

    if (NULL != pWidget)
    {
        layout(index, true);
        pWidget->getButton()->setFocus();
    }
}

/* minimum time span between keypresses to enable fast scrolling */
#define FAST_SCROLL_MIN_TIME  200

/* compares current time to the given time of the previous key press.  Returns true if difference in time
 * is less than FAST_SCROLL_MIN_TIME, false otherwise. */
bool CWidgetListView::isFastScroll(B_Time * pTimePrevKeyPress)
{
    B_Time timeNow     = B_TIME_ZERO;
    bool   bFastScroll = false;

    B_Time_Get(&timeNow);
    if (FAST_SCROLL_MIN_TIME > B_Time_Diff(&timeNow, pTimePrevKeyPress))
    {
        bFastScroll = true;
    }

    *pTimePrevKeyPress = timeNow;

    return(bFastScroll);
}

/* calculate fast scroll increment based on the number of visible items in list view */
unsigned CWidgetListView::getFastScrollIncrement()
{
    MListItr <CWidgetListItem> itr(&_widgetList);
    CWidgetListItem *          pItem      = NULL;
    int                        numVisible = 0;
    unsigned                   increment  = 1;

    for (pItem = itr.first(); pItem; pItem = itr.next())
    {
        if (pItem->getButton()->isVisible())
        {
            numVisible++;
        }
    }

    if (_widgetList.total() != numVisible)
    {
        /* only set increment if there are some hidden list items */
        increment = numVisible / 3;
        if (0 == increment)
        {
            increment = 1;
        }
    }

    return(increment);
} /* getFastScrollIncrement */

eRet CWidgetListView::onKeyDown(
        bwidget_t   widget,
        bwidget_key key
        )
{
    eRet      ret           = eRet_NotSupported;  /* assume key not consumed */
    bwidget_t focusedWidget = NULL;

    BSTD_UNUSED(widget);

    focusedWidget = bwidget_get_focus(getEngine());

    switch (key)
    {
    case bwidget_key_up:
    {
        /* handle scrolling UP within list view */
        MListItr <CWidgetListItem> itr(&_widgetList);
        int               index       = -1;
        CWidgetListItem * pWidgetPrev = NULL;

        /* find current widget in list view that corresponds to focused widget */
        index = getItemListIndex(focusedWidget);

        if (0 <= index)
        {
            unsigned fastScrollIncrement = getFastScrollIncrement();

            /* widget is in list view so get next */
            if ((true == isFastScroll(&_timeLastKeyPress)) && (0 < index - fastScrollIncrement))
            {
                /* fast scroll */
                pWidgetPrev = itr.at(index - fastScrollIncrement + 1);
                pWidgetPrev = itr.at(index - fastScrollIncrement);
            }
            else
            {
                /*
                 * slow scroll
                 * coverity[returned_pointer] pWidgetPrev is unused in the first call but itr internal value is changed
                 */
                pWidgetPrev = itr.at(index);
                pWidgetPrev = itr.prev();
            }

            /* make sure it is an active button */
            while ((NULL != pWidgetPrev) && (false == pWidgetPrev->isActive()))
            {
                pWidgetPrev = itr.prev();
            }

            if (NULL != pWidgetPrev)
            {
                /* widget or next widget is at top of list view - may have to layout() */
                if (false == isFullyVisible(pWidgetPrev->getButton()))
                {
                    /* layout starting with previous widget (if exists) */
                    layout(itr.index(pWidgetPrev), true);
                }

                pWidgetPrev->getButton()->setFocus();

                /* consume key */
                ret = eRet_Ok;
            }
            else
            {
                /* no prev widget - if focus locked, consume key */
                if (true == _lockFocus)
                {
                    ret = eRet_Ok;
                }
            }
        }
    }
    break;

    case bwidget_key_down:
    {
        /* handle scrolling DOWN within list view */
        MListItr <CWidgetListItem> itr(&_widgetList);
        int               index       = -1;
        CWidgetListItem * pWidgetNext = NULL;

        /* find current widget in list view */
        index = getItemListIndex(focusedWidget);

        if (0 <= index)
        {
            unsigned fastScrollIncrement = getFastScrollIncrement();

            /* widget is in list view so get next */
            if ((true == isFastScroll(&_timeLastKeyPress)) && (_widgetList.total() > index + (int)fastScrollIncrement))
            {
                /* fast scroll */
                pWidgetNext = itr.at(index + fastScrollIncrement);
            }
            else
            {
                /* slow scroll */
                itr.at(index);
                pWidgetNext = itr.next();
            }

            /* make sure it is an active button */
            while ((NULL != pWidgetNext) && (false == pWidgetNext->isActive()))
            {
                pWidgetNext = itr.next();
            }

            if (NULL != pWidgetNext)
            {
                /* there is a next widget to move focus to */
                if (false == isFullyVisible(pWidgetNext->getButton()))
                {
                    /* layout starting with next widget (if exists) */
                    layout(itr.index(pWidgetNext), false);
                }

                pWidgetNext->getButton()->setFocus();

                /* consume key */
                ret = eRet_Ok;
            }
            else
            {
                /* no next widget - if focus locked, consume key */
                if (true == _lockFocus)
                {
                    ret = eRet_Ok;
                }
            }
        }
    }
    break;

    case bwidget_key_left:
    case bwidget_key_right:
    default:
        if (_lockFocus)
        {
            /* consume keys */
            ret = eRet_Ok;
        }
        break;
    } /* switch */

    return(ret);
} /* onKeyDown */

CWidgetListViewContainer::CWidgetListViewContainer(
        const char *     strName,
        bwidget_engine_t engine,
        CWidgetBase *    pParentWidget,
        MRect            geometry,
        bwin_font_t      font,
        bwin_t           parentWin
        ) :
    CWidgetButton(strName, engine, pParentWidget, geometry, font, parentWin),
    _pFocusWidget(NULL)
{
}

bool CWidgetListViewContainer::setFocus()
{
    bool ret = false;

    if (NULL != _pFocusWidget)
    {
        /* focus widget override */
        ret = _pFocusWidget->setFocus();
    }
    else
    {
        /* call base class to focus on this widget */
        ret = CWidgetButton::setFocus();
    }

    return(ret);
} /* setFocus */