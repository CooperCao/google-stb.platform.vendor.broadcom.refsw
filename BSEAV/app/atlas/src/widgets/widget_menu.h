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

#ifndef ATLAS_WIDGET_MENU_H__
#define ATLAS_WIDGET_MENU_H__

#include "widget_label.h"
#include "widget_check_button.h"
#include "widget_listview.h"
#include "mlist.h"
#include "mstring.h"

#ifdef __cplusplus
extern "C" {
#endif

class CWidgetButton;

class CWidgetMenuListItem
{
public:
    CWidgetMenuListItem(
            CWidgetButton *      pButton,
            CWidgetCheckButton * pCheckButton
            ) :
        _pButton(pButton),
        _pButtonParent(pButton->getParent()),
        _pCheckButton(pCheckButton) {}

    ~CWidgetMenuListItem(void)
    {
        /* do NOT delete _pButton as it is owned by CWidgetMenu calling code */
        if (NULL != _pCheckButton) { delete _pCheckButton; }

#if 0
        {
            bbutton_settings buttonSettings;

            /* restore removed button original parent */
            bbutton_get(_pButton->getWidget(), &buttonSettings);
            buttonSettings.label.widget.window.parent = _pButtonParent->getWin();
            bbutton_set(_pButton->getWidget(), &buttonSettings);
        }
#endif /* if 0 */
    }

    CWidgetButton *      getButton(void)      { return(_pButton); }
    CWidgetBase *        getParent(void)      { return(_pButtonParent); }
    CWidgetCheckButton * getCheckButton(void) { return(_pCheckButton); }

protected:
    CWidgetButton *      _pButton;
    CWidgetBase *        _pButtonParent;
    CWidgetCheckButton * _pCheckButton;
};

class CWidgetMenu : public CWidgetLabel
{
public:
    CWidgetMenu(
            const char *     strName,
            bwidget_engine_t engine,
            CWidgetBase *    pParentWidget,
            MRect            geometry,
            bwin_font_t      fontMenu,
            bwin_font_t      fontSubMenu = NULL
            );
    ~CWidgetMenu(void);

    virtual void setGeometry(MRect geometry);

    void layout(void);
    void setMenuTitle(const char * title = NULL, const char * subTitle = NULL,
            bwidget_justify_horiz justifyHorizTitle = bwidget_justify_horiz_max,
            bwidget_justify_vert justifyVertTitle = bwidget_justify_vert_max,
            bwidget_justify_horiz justifyHorizSubTitle = bwidget_justify_horiz_max,
            bwidget_justify_vert justifyVertSubTitle = bwidget_justify_vert_max);
    void              showMenuBar(bool show);
    void              showSubTitle(bool show);
    void              showListView(bool show);
    void              showEdit(bool show);
    void              showListViewSelection(bool show);
    eRet              addButton(CWidgetButton * pWidgetItem, const char * strName, bool bVisible = true);
    void              removeButton(CWidgetButton * pWidgetItem);
    void              removeButton(uint16_t index);
    eRet              sortListView(void) { return(_pListView->sort()); }
    void              setListButtonActive(CWidgetButton * pWidgetItem, bool bShow = true);
    bool              isListButtonActive(CWidgetButton * pWidgetItem);
    void              clearButtons(void);
    void              addBackButton(CWidgetButton * pWidgetItem);
    void              addExpandButton(CWidgetButton * pWidgetItem);
    void              removeBackButton(void);
    void              removeExpandButton(void);
    void              setScroll(bool scroll);
    void              setAutoResize(bool autoResize);
    CWidgetListView * getListView(void) { return(_pListView); }
    void              setActive(CWidgetBase * pWidget, bool bActive);
    bool              isActive(CWidgetBase * pWidget);
    void              onClick(bwidget_t widget);

    void setListViewBackgroundColor(uint32_t color) { _pListView->setBackgroundColor(color); _pListViewSelection->setBackgroundColor(color); }
    void setListViewTextColor(uint32_t color)       { _pListView->setTextColor(color); _pListViewSelection->setTextColor(color); }
    void setListViewBevel(int bevel)                { _pListView->setBevel(bevel); _pListViewSelection->setBevel(bevel); }

protected:
    CWidgetLabel *    _pMenuBar;
    CWidgetButton *   _pMenuBarBack;
    CWidgetButton *   _pMenuBarExpand;
    CWidgetLabel *    _pMenuSubTitle;
    CWidgetButton *   _pEdit;
    CWidgetListView * _pListView;
    CWidgetListView * _pListViewSelection;

    /* list of active controls */
    MList <CWidgetBase> _activeList;

    /* list of containers comprised of all added button widgets and
     * their associated check buttons used in the selection list */
    MAutoList <CWidgetMenuListItem> _itemList;

    bwin_font_t _fontMenu;
};

#ifdef __cplusplus
}
#endif

#endif /* ATLAS_WIDGET_MENU_H__ */