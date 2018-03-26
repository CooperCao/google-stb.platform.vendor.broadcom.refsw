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

#include "widget_menu.h"
#include "widget_button.h"
#include "widget_listview.h"
#include "widget_check_button.h"
#include "widget_label.h"
#include "widget_edit.h"
#include "widget_popup_list.h"
#include "string.h"
#include "bdbg.h"

BDBG_MODULE(widget_menu);

CWidgetMenu::CWidgetMenu(
        const char *     strName,
        bwidget_engine_t engine,
        CWidgetBase *    pParentWidget,
        MRect            geometry,
        bwin_font_t      fontMenu,
        bwin_font_t      fontSubMenu
        ) :
    CWidgetLabel(strName, engine, pParentWidget, geometry, fontMenu),
    _pMenuBar(NULL),
    _pMenuBarBack(NULL),
    _pMenuBarExpand(NULL),
    _pMenuSubTitle(NULL),
    _pEdit(NULL),
    _pListView(NULL),
    _fontMenu(fontMenu)
{
    eRet ret = eRet_Ok;

    setBevel(0);
    setBackgroundColor(0xCC222222); /* slight transparency */

    MRect rectMenu = getGeometry();

    _pMenuBar = new CWidgetLabel("CWidgetMenu::_pMenuBar", getEngine(), this, MRect(0, 0, 0, 0), fontMenu);
    CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pMenuBar, ret, eRet_OutOfMemory, error);
    _pMenuBar->setBackgroundFillMode(fill_eGradient);
    _pMenuBar->setBackgroundGradient(COLOR_GREY_DARK + 4 * COLOR_STEP, COLOR_GREY_DARK, COLOR_GREY_DARK - 1 * COLOR_STEP);
    _pMenuBar->setText("Menu", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
    _pMenuBar->setTextColor(COLOR_EGGSHELL);
    _pMenuBar->setBevel(0);
    setActive(_pMenuBar, true);

    _pMenuSubTitle = new CWidgetLabel("CWidgetMenu::_pMenuSubTitle", getEngine(), this, MRect(0, 0, 0, 0), fontSubMenu ? fontSubMenu : fontMenu);
    CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pMenuSubTitle, ret, eRet_OutOfMemory, error);
    _pMenuSubTitle->setBevel(0);
    _pMenuSubTitle->setBackgroundColor(getBackgroundColor());
    _pMenuSubTitle->setText("Label Text", bwidget_justify_horiz_left, bwidget_justify_vert_bottom);
    _pMenuSubTitle->setTextColor(0xFF80C42F);
    setActive(_pMenuSubTitle, true);

    _pEdit = new CWidgetButton("CWidgetMenu::_pEdit", getEngine(), this, MRect(0, 0, 0, 0), fontSubMenu ? fontSubMenu : fontMenu);
    _pEdit->setBevel(0);
    _pEdit->setText("+/-", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
    _pEdit->setBackgroundColor(0xFF222222);
    _pEdit->setTextColor(0xFFCCCCCC);
    setActive(_pEdit, false);

    _pListView = new CWidgetListView("CWidgetMenu::_pListView", getEngine(), this, MRect(0, 0, 0, 0), fontMenu);
    CHECK_PTR_ERROR_GOTO("unable to allocate listview widget", _pListView, ret, eRet_OutOfMemory, error);
    setActive(_pListView, true);

    _pListViewSelection = new CWidgetListView("CWidgetMenu::_pListSelection", getEngine(), this, MRect(0, 0, 0, 0), fontMenu);
    CHECK_PTR_ERROR_GOTO("unable to allocate listview widget", _pListViewSelection, ret, eRet_OutOfMemory, error);
    setActive(_pListViewSelection, false);

    layout();

    return;

error:
    BDBG_ASSERT(0);
}

CWidgetMenu::~CWidgetMenu()
{
    _itemList.clear();
    DEL(_pListViewSelection);
    DEL(_pListView);
    DEL(_pEdit);
    DEL(_pMenuSubTitle);
    DEL(_pMenuBar);
}

#define EDIT_IND_WIDTH   25
#define EDIT_IND_HEIGHT  16
#define SUBTITLE_HEIGHT  25

void CWidgetMenu::layout()
{
    MRect rectMenu = getGeometry();
    MRect rectMenuBar;
    MRect rectSubTitle;
    MRect rectEdit;
    MRect rectListView;
    MRect rectListViewSelection;
    int   outsideMargin = 7;
    int   vMargin       = 5;
    int   heightTotal   = 0;

    if (true == isActive(_pMenuBar))
    {
        rectMenuBar.setX(0);
        rectMenuBar.setY(0);
        rectMenuBar.setWidth(rectMenu.width());
        rectMenuBar.setHeight(30);
        _pMenuBar->setGeometry(rectMenuBar);
        heightTotal += rectMenuBar.height();

        /* layout back button if set */
        if (NULL != _pMenuBarBack)
        {
            _pMenuBarBack->setGeometry(MRect(rectMenuBar.x() + 5,
                            rectMenuBar.y() + 5,
                            30,
                            20));
        }

        /* layout expand button if set */
        if (NULL != _pMenuBarExpand)
        {
            _pMenuBarExpand->setGeometry(MRect(rectMenuBar.x() + rectMenuBar.width() - 30 - 5,
                            rectMenuBar.y() + 5,
                            30,
                            20));
        }
    }
    else
    {
        rectMenuBar.setHeight(0);
    }

    if (true == isActive(_pMenuSubTitle))
    {
        rectSubTitle.setX(outsideMargin);
        rectSubTitle.setY(heightTotal);
        rectSubTitle.setWidth(rectMenu.width() - rectEdit.width() - (outsideMargin * 2) - EDIT_IND_WIDTH);
        rectSubTitle.setHeight(SUBTITLE_HEIGHT);
        _pMenuSubTitle->setGeometry(rectSubTitle);
    }

    /* adjust total height based on edit and sub title active state */
    if ((true == isActive(_pEdit)) || (true == isActive(_pMenuSubTitle)))
    {
        heightTotal += MAX(rectSubTitle.height(), EDIT_IND_HEIGHT);
    }
    else
    {
        /* add margin for area under menu bar */
        heightTotal += outsideMargin;
        rectSubTitle.setHeight(0);
    }

    if (true == isActive(_pEdit))
    {
        /* layout edit button/icon */
        rectEdit.setWidth(EDIT_IND_WIDTH);
        rectEdit.setHeight(EDIT_IND_HEIGHT);
        rectEdit.setX(rectMenu.width() - outsideMargin - rectEdit.width());
        rectEdit.setY(heightTotal - EDIT_IND_HEIGHT);
        _pEdit->setGeometry(rectEdit);
    }

    /* either _pListView or _pListViewSelection is displayed - never both */
    if (true == isActive(_pListView))
    {
        rectListView.setX(outsideMargin);
        rectListView.setY(heightTotal);

        rectListView.setWidth(rectMenu.width() - (outsideMargin * 2));

        if (true == _pListView->getScroll())
        {
            /* list view scrolling is ON so resize list view to
             * fit in the remaining space in this menu widget */
            rectListView.setHeight(rectMenu.height() - rectListView.y() - outsideMargin);
            _pListView->setGeometry(rectListView);

            heightTotal += rectListView.height() + vMargin;
        }
        else
        {
            MRect rectListViewActual = _pListView->getGeometry();
            rectListView.setHeight(rectListViewActual.height());
            _pListView->setGeometry(rectListView);

            heightTotal += rectListViewActual.height() + outsideMargin;

            /* list view scrolling is OFF so resize menu widget to
             * fit the enclosed list widget */
            rectMenu.setHeight(heightTotal);
            CWidgetLabel::setGeometry(rectMenu);
        }
    }
    else
    if (true == isActive(_pListViewSelection))
    {
        rectListViewSelection.setX(outsideMargin);
        rectListViewSelection.setY(heightTotal);

        BDBG_ASSERT(true == _pListViewSelection->getScroll());
        rectListViewSelection.setWidth(rectMenu.width() - (outsideMargin * 2));

        /* selection list view scrolling should be always ON so resize list view to
         * fit in the remaining space in this menu widget */
        rectListViewSelection.setHeight(rectMenu.height() - rectListViewSelection.y() - outsideMargin);
        _pListViewSelection->setGeometry(rectListViewSelection);

        heightTotal += rectListViewSelection.height() + vMargin;
    }
} /* layout */

void CWidgetMenu::setGeometry(MRect geometry)
{
    CWidgetLabel::setGeometry(geometry); /* call base class */

    layout();
}

void CWidgetMenu::showMenuBar(bool show)
{
    setActive(_pMenuBar, show);
    layout();
}

void CWidgetMenu::showSubTitle(bool show)
{
    setActive(_pMenuSubTitle, show);
    layout();
}

void CWidgetMenu::showListView(bool show)
{
    setActive(_pListView, show);
    layout();
}

void CWidgetMenu::showEdit(bool show)
{
    setActive(_pEdit, show);
    layout();
}

void CWidgetMenu::showListViewSelection(bool show)
{
    setActive(_pListViewSelection, show);
    layout();
}

void CWidgetMenu::setActive(
        CWidgetBase * pWidget,
        bool          bActive
        )
{
    MListItr <CWidgetBase> itr(&_activeList);
    CWidgetBase *          pActiveWidget = NULL;

    /* remove from active list if it exists */
    for (pActiveWidget = itr.first(); pActiveWidget; pActiveWidget = itr.next())
    {
        if (pActiveWidget == pWidget)
        {
            _activeList.remove(pActiveWidget);
            break;
        }
    }

    if (true == bActive)
    {
        _activeList.add(pWidget);
    }

    pWidget->show(bActive);
} /* setActive */

bool CWidgetMenu::isActive(CWidgetBase * pWidget)
{
    MListItr <CWidgetBase> itr(&_activeList);
    CWidgetBase *          pActiveWidget = NULL;
    bool                   bActive       = false;

    /* search for widget in active list */
    for (pActiveWidget = itr.first(); pActiveWidget; pActiveWidget = itr.next())
    {
        if (pActiveWidget == pWidget)
        {
            bActive = true;
            break;
        }
    }

    return(bActive);
} /* isActive */

/* sets menubar title and sub title if non-NULL */
void CWidgetMenu::setMenuTitle(
        const char *          title,
        const char *          subTitle,
        bwidget_justify_horiz justifyHorizTitle,
        bwidget_justify_vert  justifyVertTitle,
        bwidget_justify_horiz justifyHorizSubTitle,
        bwidget_justify_vert  justifyVertSubTitle
        )
{
    if (NULL != title)
    {
        _pMenuBar->setText(title, justifyHorizTitle, justifyVertTitle);
    }

    if (NULL != subTitle)
    {
        _pMenuSubTitle->setText(subTitle, justifyHorizSubTitle, justifyVertSubTitle);
    }
}

/* adds a button widget to this list view.  click callbacks are handled through the button
 * widget settings - calling code will handle click processing.  we will also create
 * a checkbox/button combo for the widget selection list which will allow users to
 * specify which list view element to show/hide (this is handy when there are a large
 * number of elements but the user may only be interested in working with a small subset) */
eRet CWidgetMenu::addButton(
        CWidgetButton * pWidgetItem,
        const char *    strName,
        bool            bVisible
        )
{
    eRet                  ret     = eRet_Ok;
    CWidgetCheckButton *  pButton = NULL;
    CWidgetMenuListItem * pItem   = NULL;

    BDBG_ASSERT(NULL != pWidgetItem);

    /* inherit size/font from given widget */
    MRect       rect = pWidgetItem->getGeometry();
    bwin_font_t font = pWidgetItem->getFont();

    /* add check button to selection list view.  these widgets will be owned by
     * the _itemList autolist and will be auto destroyed when removed. */
    pButton = new CWidgetCheckButton("CWidgetMenu list button", getEngine(), this, rect, font);
    CHECK_PTR_ERROR_GOTO("unable to allocate label widget", pButton, ret, eRet_OutOfMemory, error);
    pButton->setBevel(0);
    pButton->setCheck(bVisible);
    pButton->setText(strName, bwidget_justify_horiz_left);

    /* add to selection list view */
    _pListViewSelection->add(pButton);

    if (true == bVisible)
    {
        /* given button is visible so add to listview */
        _pListView->add(pWidgetItem);
    }

    /* save all added buttons */
    pItem = new CWidgetMenuListItem(pWidgetItem, pButton);
    CHECK_PTR_ERROR_GOTO("unable to create menu list item", pItem, ret, eRet_OutOfMemory, error);
    _itemList.add(pItem);

    if (false == _pListView->getScroll())
    {
        layout();
    }

    goto done;
error:
    DEL(pButton);
    DEL(pItem);
done:
    return(ret);
} /* addButton */

void CWidgetMenu::removeButton(CWidgetButton * pWidgetItem)
{
    CWidgetMenuListItem * pItem = NULL;

    /* find given button in list of previously added buttons */
    for (pItem = _itemList.first(); pItem; pItem = _itemList.next())
    {
        if (pWidgetItem == pItem->getButton())
        {
            /* found button so remove it from list view controls */
            _pListViewSelection->remove(pItem->getButton());
            _pListView->remove(pWidgetItem);

            /* remove from list of added widgets */
            _itemList.remove(pItem);
            DEL(pItem);
            break;
        }
    }

    if (false == _pListView->getScroll())
    {
        layout();
    }
} /* removeButton */

/* show/hide a widget button from the list view.  widgets added to the list view can
 * be hidden/shown by default using the addButton() method.  this method allows you
 * to change that visibility setting.  note that if you have scrolling turned off,
 * showing/hiding widgets will resize the list widget automatically. */
void CWidgetMenu::setListButtonActive(
        CWidgetButton * pWidgetItem,
        bool            bShow
        )
{
    MListItr <CWidgetMenuListItem> itr(&_itemList);
    CWidgetMenuListItem *          pItem = NULL;

    /* find given button in list of previously added buttons */
    for (pItem = itr.first(); pItem; pItem = itr.next())
    {
        if (pWidgetItem == pItem->getButton())
        {
            /* found matching previously added button  */
            _pListView->setActive(pWidgetItem, bShow);
            break;
        }
    }

    if (NULL != pItem)
    {
        if (false == _pListView->getScroll())
        {
            layout();
        }
    }
} /* setListButtonActive */

bool CWidgetMenu::isListButtonActive(CWidgetButton * pWidgetItem)
{
    MListItr <CWidgetMenuListItem> itr(&_itemList);
    CWidgetMenuListItem *          pItem = NULL;
    bool bActive                         = false;

    /* find given button in list of previously added buttons */
    for (pItem = itr.first(); pItem; pItem = itr.next())
    {
        if (pWidgetItem == pItem->getButton())
        {
            /* found matching previously added button  */
            bActive = _pListView->isActive(pWidgetItem);
            break;
        }
    }

    return(bActive);
} /* isListButtonActive */

void CWidgetMenu::clearButtons()
{
    /* remove buttons from list view controls */
    _pListView->clear();
    _pListViewSelection->clear();

    /* remove buttons from list of added widgets */
    _itemList.clear();
}

void CWidgetMenu::addBackButton(CWidgetButton * pWidgetItem)
{
    bbutton_settings buttonSettings;

    BDBG_ASSERT(NULL != pWidgetItem);

    /* make sure new widgetItem is a child of the menu bar */
    bbutton_get(pWidgetItem->getWidget(), &buttonSettings);
    buttonSettings.label.widget.window.parent = _pMenuBar->getWin();
    bbutton_set(pWidgetItem->getWidget(), &buttonSettings);

    pWidgetItem->setBackgroundFillMode(fill_eGradient);
    pWidgetItem->setBackgroundGradient(COLOR_GREY_DARK + 4 * COLOR_STEP, COLOR_GREY_DARK, COLOR_GREY_DARK - 1 * COLOR_STEP);
    pWidgetItem->setTextMargin(0);
    pWidgetItem->setBevel(0);
    pWidgetItem->setTextColor(COLOR_EGGSHELL);

    _pMenuBarBack = pWidgetItem;
    layout();

    return;
} /* addBackButton */

void CWidgetMenu::addExpandButton(CWidgetButton * pWidgetItem)
{
    bbutton_settings buttonSettings;

    BDBG_ASSERT(NULL != pWidgetItem);

    /* make sure new widgetItem is a child of the menu bar */
    bbutton_get(pWidgetItem->getWidget(), &buttonSettings);
    buttonSettings.label.widget.window.parent = _pMenuBar->getWin();
    bbutton_set(pWidgetItem->getWidget(), &buttonSettings);

    pWidgetItem->setBackgroundFillMode(fill_eGradient);
    pWidgetItem->setBackgroundGradient(COLOR_GREY_DARK + 4 * COLOR_STEP, COLOR_GREY_DARK, COLOR_GREY_DARK - 1 * COLOR_STEP);
    pWidgetItem->setTextMargin(0);
    pWidgetItem->setBevel(0);
    pWidgetItem->setTextColor(COLOR_EGGSHELL);

    _pMenuBarExpand = pWidgetItem;
    layout();

    return;
} /* addExpandButton */

/* since added back button widget was originally created by the calling code,
 * we will only re-parent it here.  it remains the calling code's responsibility
 * to unallocate the back button widget when appropriate. */
void CWidgetMenu::removeBackButton()
{
    bbutton_settings buttonSettings;

    BDBG_ASSERT(NULL != _pMenuBarBack);

    /* make sure new widgetItem is a child of menu bar's parent */
    bbutton_get(_pMenuBarBack->getWidget(), &buttonSettings);
    buttonSettings.label.widget.window.parent = _pParent->getWin();
    bbutton_set(_pMenuBarBack->getWidget(), &buttonSettings);

    _pMenuBarBack = NULL;
}

/* since added expand button widget was originally created by the calling code,
 * we will only re-parent it here.  it remains the calling code's responsibility
 * to unallocate the expand button widget when appropriate. */
void CWidgetMenu::removeExpandButton()
{
    bbutton_settings buttonSettings;

    BDBG_ASSERT(NULL != _pMenuBarExpand);

    /* make sure new widgetItem is a child of menu bar's parent */
    bbutton_get(_pMenuBarExpand->getWidget(), &buttonSettings);
    buttonSettings.label.widget.window.parent = _pParent->getWin();
    bbutton_set(_pMenuBarExpand->getWidget(), &buttonSettings);

    _pMenuBarExpand = NULL;
}

/* turning OFF scrolling for the list view will cause it to auto resize based
 * on contents - the menu will then resize accordingly.
 * turning ON scrolling for the list view will cause the list view to resize
 * based on the current size of the menu */
void CWidgetMenu::setScroll(bool scroll)
{
    _pListView->setScroll(scroll);
    if (false == scroll)
    {
        layout();
    }
}

void CWidgetMenu::onClick(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }

    if (_pEdit == pWidget)
    {
        bool bListViewActive = isActive(_pListView);

        if (true == isActive(_pListViewSelection))
        {
            MListItr <CWidgetMenuListItem> itr(&_itemList);
            CWidgetMenuListItem *          pItem = NULL;

            /* selection list view is about to be hidden, so adjust
             * active list view based on user selections */
            _pListView->clear();

            for (pItem = itr.first(); pItem; pItem = itr.next())
            {
                if ((NULL != pItem->getCheckButton()) && (true == pItem->getCheckButton()->isChecked()))
                {
                    /* user selected this control to add it to list view */
                    _pListView->add(pItem->getButton());
                }
            }
        }

        showListView(!bListViewActive);
        showListViewSelection(bListViewActive);
    }
} /* onClick */

eRet CWidgetMenu::createDualLabelButton(
        CWidgetBase *        pParent,
        int                  width,
        int                  height,
        const char *         strName,
        CWidgetButton **     pButton,
        CWidgetLabel **      pLabel,
        CWidgetLabel **      pValue,
        bwin_font_t          font,
        uint8_t              labelPercentage,
        backgroundFillMode_t fillMode
        )
{
    eRet ret                              = eRet_Ok;
    CWidgetListViewContainer * pContainer = NULL;

    BDBG_ASSERT(NULL != strName);

    pContainer = new CWidgetListViewContainer(MString(strName)+"Container", getEngine(), pParent, MRect(0, 0, width, height), font);
    CHECK_PTR_ERROR_GOTO("unable to allocate button widget", pContainer, ret, eRet_OutOfMemory, error);
    pContainer->setBackgroundFillMode(fillMode);
    (*pButton) = pContainer;
    {
        MRect rect          = (*pButton)->getGeometry();
        MRect rectContainer = pContainer->getGeometry();
        int   margin        = 5;

        rect.setX(margin);
        rect.setY(0);
        rect.setWidth((rect.width() - margin * 2) * labelPercentage / 100);

        *pLabel = new CWidgetLabel(MString(strName)+"Label", getEngine(), *pButton, rect, font);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", *pLabel, ret, eRet_OutOfMemory, error);
        (*pLabel)->setBevel(0);
        (*pLabel)->setBackgroundFillMode(fillMode);

        rect.setX(rect.x() + rect.width());
        BDBG_ASSERT(0 < rectContainer.width());
        rect.setWidth(rectContainer.width() - rect.x() - 20);

        (*pValue) = new CWidgetLabel(MString(strName)+"Value", getEngine(), *pButton, rect, font);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", *pValue, ret, eRet_OutOfMemory, error);
        (*pValue)->setBevel(0);
        (*pValue)->setBackgroundFillMode(fillMode);
    }

    goto done;
error:
    DEL(pButton);
    DEL(pLabel);
    DEL(pValue);
done:
    return(ret);
} /* createDualLabelButton */

eRet CWidgetMenu::addDualLabelButton(
        CWidgetBase *        pParent,
        const char *         strName,
        CWidgetButton **     pButton,
        CWidgetLabel **      pLabel,
        CWidgetLabel **      pValue,
        bwin_font_t          font,
        uint8_t              labelPercentage,
        backgroundFillMode_t fillMode
        )
{
    eRet  ret      = eRet_Ok;
    MRect rectMenu = getGeometry();

    ret = createDualLabelButton(pParent, rectMenu.width(), 18, strName, pButton, pLabel, pValue, font, labelPercentage, fillMode);
    CHECK_ERROR_GOTO("unable to create a dual label button", ret, error);
    ret = addButton(*pButton, strName);
    CHECK_ERROR_GOTO("unable to add button to menu", ret, error);

error:
    return(ret);
} /* addDualLabelButton */

/* creates a button with an edit surrounded by two labels on either side.
 * allows you to have 2 columns of text and one edit on a single parent button. */
eRet CWidgetMenu::addDualLabelEditButton(
        CWidgetBase *        pParent,
        const char *         strName,
        CWidgetButton **     pButton,
        CWidgetLabel **      pLabelLeft,
        CWidgetEdit **       pEdit,
        CWidgetLabel **      pLabelRight,
        bwin_font_t          font,
        uint8_t              labelLeftWidth,
        uint8_t              editWidth,
        uint8_t              labelRightWidth,
        backgroundFillMode_t fillMode
        )
{
    eRet ret                              = eRet_Ok;
    CWidgetListViewContainer * pContainer = NULL;

    BDBG_ASSERT(NULL != strName);

    pContainer = new CWidgetListViewContainer(MString(strName)+"Container", getEngine(), pParent, MRect(0, 0, 0, 20), font);
    CHECK_PTR_ERROR_GOTO("unable to allocate button widget", pContainer, ret, eRet_OutOfMemory, error);
    pContainer->setBackgroundFillMode(fillMode);
    (*pButton) = pContainer;
    addButton(*pButton, strName);
    {
        MRect rect   = (*pButton)->getGeometry();
        int   margin = 0;

        rect.setX(margin);
        rect.setY(0);
        /* set width of first text field to be a third of the button width */
        rect.setWidth(labelLeftWidth);

        *pLabelLeft = new CWidgetLabel(MString(strName)+"LabelLeft", getEngine(), *pButton, rect, font);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", *pLabelLeft, ret, eRet_OutOfMemory, error);
        (*pLabelLeft)->setBevel(0);
        (*pLabelLeft)->setTextMargin(5);
        (*pLabelLeft)->setBackgroundFillMode(fillMode);

        rect.setX(rect.x() + rect.width());
        rect.setWidth(editWidth);

        /* specify 'this' as parent so we get the key/click callbacks but add optional bwin parent
         * parameter to maintain proper drawing heirarchy*/
        *pEdit = new CWidgetEdit(MString(strName)+"Edit", getEngine(), pParent, rect, font, (*pButton)->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate edit widget", *pEdit, ret, eRet_OutOfMemory, error);
        (*pEdit)->setBevel(0);
        (*pEdit)->setPosition(0);
        (*pEdit)->setBackgroundFillMode(fillMode);
        pContainer->setFocusWidget(*pEdit);

        rect.setX(rect.x() + rect.width());
        rect.setWidth(labelRightWidth);

        *pLabelRight = new CWidgetLabel(MString(strName)+"LabelRight", getEngine(), *pButton, rect, font);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", *pLabelRight, ret, eRet_OutOfMemory, error);
        (*pLabelRight)->setBevel(0);
        (*pLabelRight)->setTextMargin(5);
        (*pLabelRight)->setBackgroundFillMode(fillMode);
    }

    goto done;
error:
    DEL(pButton);
    DEL(pLabelLeft);
    DEL(pEdit);
    DEL(pLabelRight);
done:
    return(ret);
} /* addDualLabelEditButton */

/* creates a button with a label and a popup list widget
 * allows you to have a column of text and popup list on a single parent button. */
eRet CWidgetMenu::addLabelPopupButton(
        CWidgetBase *        pParent,
        const char *         strName,
        CWidgetButton **     pButton,
        CWidgetLabel **      pLabel,
        CWidgetPopupList **  pPopup,
        bwin_font_t          font,
        uint8_t              labelPercentage,
        backgroundFillMode_t fillMode
        )
{
    eRet ret                              = eRet_Ok;
    CWidgetListViewContainer * pContainer = NULL;

    BDBG_ASSERT(NULL != strName);

    pContainer = new CWidgetListViewContainer(MString(strName)+"Container", getEngine(), pParent, MRect(0, 0, 0, 20), font);
    CHECK_PTR_ERROR_GOTO("unable to allocate button widget", pContainer, ret, eRet_OutOfMemory, error);
    pContainer->setBackgroundFillMode(fillMode);
    (*pButton) = pContainer;
    addButton(*pButton, strName);
    {
        MRect rect     = (*pButton)->getGeometry();
        MRect rectMenu = getGeometry();
        int   margin   = 0;

        rect.setX(margin);
        rect.setY(0);
        /* set width of first text field based on given percentage */
        rect.setWidth((rect.width() - margin * 2) * labelPercentage / 100);

        *pLabel = new CWidgetLabel(MString(strName)+"Label", getEngine(), *pButton, rect, font);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", *pLabel, ret, eRet_OutOfMemory, error);
        (*pLabel)->setBevel(0);
        (*pLabel)->setTextMargin(5);
        (*pLabel)->setBackgroundFillMode(fillMode);

        rect.setX(rect.x() + rect.width());
        rect.setWidth(rectMenu.width() - rect.x() - 20);

        /* the popup list parent will be this so we can handle button clicks here, but set the bwin
         * parent as pButton so that it will display in the proper place */
        *pPopup = new CWidgetPopupList(MString(strName)+"PopupList", getEngine(), pParent, rect, font, (*pButton)->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate popup list widget", *pPopup, ret, eRet_OutOfMemory, error);
        (*pPopup)->setBevel(0);
        (*pPopup)->setBackgroundFillMode(fillMode);
        pContainer->setFocusWidget(*pPopup);
    }

    goto done;
error:
    DEL(pButton);
    DEL(pLabel);
    DEL(pPopup);
done:
    return(ret);
} /* addLabelPopupButton */