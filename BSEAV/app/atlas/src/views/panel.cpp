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

#include "bstd.h"
#include "atlas.h"
#include "panel.h"

BDBG_MODULE(atlas_panel);

CPanel::CPanel(
        const char *    strName,
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CWidgetLabel(strName, pWidgetEngine->getWidgetEngine(), pParentWidget, geometry, font, parentWin),
    _pWidgetEngine(pWidgetEngine),
    _pConfig(NULL),
    _pCfg(NULL),
    _focusWidget(NULL),
    _pScreenMain(pScreenMain)
{
}

void CPanel::show(bool bShow)
{
    if (true == bShow)
    {
        /* showing this panel so restore last focus */
        if (NULL != _focusWidget)
        {
            bwidget_set_focus(_focusWidget);
        }
    }
    else
    {
        if (true == isVisible())
        {
            /* save the current focus before hiding */
            _focusWidget = bwidget_get_focus(getEngine());
        }
    }

    CWidgetLabel::show(bShow);
} /* show */

/* creates a button with two labels over it.  allows you to have 2 columns of text on the button. */
eRet CPanel::createDualLabelButton(
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

    pContainer = new CWidgetListViewContainer(MString(strName)+"Container", getEngine(), this, MRect(0, 0, width, height), font);
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

eRet CPanel::addDualLabelButton(
        CWidgetMenu *        pMenu,
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
    MRect rectMenu = pMenu->getGeometry();

    ret = createDualLabelButton(rectMenu.width(), 18, strName, pButton, pLabel, pValue, font, labelPercentage, fillMode);
    CHECK_ERROR_GOTO("unable to create a dual label button", ret, error);
    ret = pMenu->addButton(*pButton, strName);
    CHECK_ERROR_GOTO("unable to add button to menu", ret, error);

error:
    return(ret);
} /* addDualLabelButton */

/* creates a button with an edit surrounded by two labels on either side.
 * allows you to have 2 columns of text and one edit on a single parent button. */
eRet CPanel::addDualLabelEditButton(
        CWidgetMenu *        pMenu,
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

    pContainer = new CWidgetListViewContainer(MString(strName)+"Container", getEngine(), this, MRect(0, 0, 0, 20), font);
    CHECK_PTR_ERROR_GOTO("unable to allocate button widget", pContainer, ret, eRet_OutOfMemory, error);
    pContainer->setBackgroundFillMode(fillMode);
    (*pButton) = pContainer;
    pMenu->addButton(*pButton, strName);
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
        *pEdit = new CWidgetEdit(MString(strName)+"Edit", getEngine(), this, rect, font, (*pButton)->getWin());
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
eRet CPanel::addLabelPopupButton(
        CWidgetMenu *        pMenu,
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

    pContainer = new CWidgetListViewContainer(MString(strName)+"Container", getEngine(), this, MRect(0, 0, 0, 20), font);
    CHECK_PTR_ERROR_GOTO("unable to allocate button widget", pContainer, ret, eRet_OutOfMemory, error);
    pContainer->setBackgroundFillMode(fillMode);
    (*pButton) = pContainer;
    pMenu->addButton(*pButton, strName);
    {
        MRect rect     = (*pButton)->getGeometry();
        MRect rectMenu = pMenu->getGeometry();
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
        *pPopup = new CWidgetPopupList(MString(strName)+"PopupList", getEngine(), this, rect, font, (*pButton)->getWin());
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