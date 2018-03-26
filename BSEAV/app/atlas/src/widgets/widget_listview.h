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

#ifndef ATLAS_WIDGET_LISTVIEW_H__
#define ATLAS_WIDGET_LISTVIEW_H__

#include "widget_label.h"
#include "widget_button.h"
#include "mlist.h"

#ifdef __cplusplus
extern "C" {
#endif

class CWidgetListItem
{
public:
    CWidgetListItem(
            CWidgetBase *   pListView,
            CWidgetButton * pButton
            ) :
        _pListView(pListView),
        _pButton(pButton),
        _pButtonParent(pButton->getParent()),
        _bActive(true)
    {
        BDBG_ASSERT(NULL != pListView);
        BDBG_ASSERT(NULL != pButton);

        /* make sure new widgetItem is a child of list view */
        pButton->setParentWin(pListView->getWin());
    }

    CWidgetButton * getButton(void)         { return(_pButton); }
    CWidgetBase *   getParent(void)         { return(_pButtonParent); }
    bool            isActive(void)          { return(_bActive); }
    void            setActive(bool bActive) { _bActive = bActive; }

protected:
    CWidgetBase *   _pListView;
    CWidgetButton * _pButton;
    CWidgetBase *   _pButtonParent;
    bool            _bActive;
};

class CWidgetListView : public CWidgetLabel
{
public:
    CWidgetListView(
            const char *     strName,
            bwidget_engine_t engine,
            CWidgetBase *    pParentWidget,
            MRect            geometry,
            bwin_font_t      font,
            bwin_t           parentWin = NULL
            );
    ~CWidgetListView(void);

    virtual void show(bool bShow);
    virtual void show(bool bShow, bool bLockFocus);
    virtual void setGeometry(MRect geometry);

    unsigned        getSeparatorSize(void)          { return(_separatorSize); }
    void            setSeparatorSize(unsigned size) { _separatorSize = size; }
    void            add(CWidgetButton * pWidgetItem);
    void            remove(CWidgetButton * pWidgetItem);
    CWidgetButton * remove(unsigned index);
    int             getNumButtons(void) { return(_widgetList.total()); }
    void            setActive(CWidgetButton * pWidgetItem, bool bActive);
    bool            isActive(CWidgetButton * pWidgetItem);
    eRet            sort(CWidgetButton * pButtonFocus = NULL);
    void            clear(void);
    int32_t         getFirstVisibleIndex(void);
    eRet            layout(int index = -1, bool topDown = true);
    int             getItemListIndex(bwidget_t widget);
    bool            isFullyVisible(CWidgetButton * pWidget);
    bool            isFirstVisible(CWidgetButton * pWidget);
    bool            isLastVisible(CWidgetButton * pWidget);
    bool            isFastScroll(B_Time * pTimePrevKeyPress);
    void            scrollTo(int index);
    unsigned        getFastScrollIncrement(void);
    bool            getScroll(void) { return(_scroll); }
    void            setScroll(bool scroll);
    bool            getAutoResize(void) { return(_autoResize); }
    void            setAutoResize(bool autoResize);
    void            lockFocus(bool lock) { _lockFocus = lock; }
    eRet            onKeyDown(bwidget_t widget, bwidget_key key);

    MAutoList <CWidgetListItem> * getWidgetList(void) { return(&_widgetList); }

protected:
    MAutoList <CWidgetListItem> _widgetList;
    unsigned                    _separatorSize;
    bool   _scroll;
    bool   _autoResize;
    bool   _lockFocus;
    B_Time _timeLastKeyPress;
};

class CWidgetListViewContainer : public CWidgetButton
{
public:
    CWidgetListViewContainer(
            const char *     strName,
            bwidget_engine_t engine,
            CWidgetBase *    pParentWidget,
            MRect            geometry,
            bwin_font_t      font,
            bwin_t           parentWin = NULL
            ); /* optional */

    virtual bool setFocus(void);

    void setFocusWidget(CWidgetBase * pFocusWidget) { _pFocusWidget = pFocusWidget; }

protected:
    /* if non NULL, this widget will gain focus if the list view container is given focus.
     * this helper class will manage focus in the event the container list view item
     * contains one or more sub widgets */
    CWidgetBase * _pFocusWidget;
};

#ifdef __cplusplus
}
#endif

#endif /* ATLAS_WIDGET_LISTVIEW_H__ */