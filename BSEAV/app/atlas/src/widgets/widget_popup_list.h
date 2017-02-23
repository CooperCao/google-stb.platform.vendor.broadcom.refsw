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

#ifndef ATLAS_WIDGET_POPUP_LIST_H__
#define ATLAS_WIDGET_POPUP_LIST_H__

#include "widget_button.h"
#include "widget_listview.h"
#include "widget_check_button.h"

#ifdef __cplusplus
extern "C" {
#endif

class CWidgetPopupList : public CWidgetButton
{
public:
    CWidgetPopupList(
            const char *     strName,
            bwidget_engine_t engine,
            CWidgetBase *    pParentWidget,
            MRect            geometry,
            bwin_font_t      font,
            bwin_t           parentWin = NULL
            );
    ~CWidgetPopupList();

    virtual void setGeometry(MRect geometry);
    virtual void drawFocus(bwidget_t widget, blabel_settings * pLabelSettings, const bwin_rect rect, const bwin_rect * pRectClip);
    virtual void drawText(bwidget_t widget, blabel_settings * pLabelSettings, const bwin_rect rect, const bwin_rect * pRectClip);

    void                 layout(void);
    void                 popup(bool show);
    CWidgetCheckButton * addButton(const char * name, uint16_t width, uint16_t height);
    CWidgetCheckButton * findButton(long value);
    void                 setActive(
            CWidgetButton * pWidgetItem,
            bool            bActive
            ) { _pListView->setActive(pWidgetItem, bActive); }
    int                  getItemListIndex(bwidget_t widget) { return(_pListView->getItemListIndex(widget)); }
    void                 clearButtons(void);
    eRet                 select(CWidgetCheckButton * pButton);
    eRet                 select(const char * name);
    eRet                 select(const long value);
    CWidgetCheckButton * getSelection(void) { return(_pSelectedButton); }
    void                 onClick(bwidget_t widget);
    eRet                 sort(void) { return(_pListView->sort()); }

protected:
    CWidgetListView *              _pListView;
    bool                           _showPopup;
    CWidgetCheckButton *           _pSelectedButton;
    MAutoList <CWidgetCheckButton> _listCheckButtons;
    uint8_t                        _maxNumVisibleButtons;
};

#ifdef __cplusplus
}
#endif

#endif /* ATLAS_WIDGET_POPUP_LIST_H__ */