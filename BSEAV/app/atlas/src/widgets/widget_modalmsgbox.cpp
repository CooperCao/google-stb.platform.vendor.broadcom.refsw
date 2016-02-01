/***************************************************************************
 * (c) 2002-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *
 *****************************************************************************/

#include "widget_modalmsgbox.h"
#include "string.h"
#include "bdbg.h"

BDBG_MODULE(widget_modalmsgbox);

static int msg_box_bwidget_key_down(
        bwidget_t         widget,
        const bwidget_key key
        )
{
    eRet                 ret = eRet_NotAvailable; /* assume key is not consumed */
    CWidgetModalMsgBox * pMsgBox;
    blabel_settings      labelSettings;

    blabel_get(widget, &labelSettings);
    pMsgBox = (CWidgetModalMsgBox *)labelSettings.widget.data;
    BDBG_ASSERT(NULL != pMsgBox);

    ret = pMsgBox->onKeyDown(widget, key);

    return((eRet_Ok == ret) ? 0 : -1);
}

CWidgetModalMsgBox::CWidgetModalMsgBox(
        const char *     strName,
        bwidget_engine_t engine,
        CWidgetBase *    pParentWidget,
        MRect            geometry,
        bwin_font_t      font
        ) :
    CWidgetLabel(strName, engine, pParentWidget, geometry, font, NULL),
    _done(false),
    _pLabel(NULL),
    _pOk(NULL),
    _pCancel(NULL),
    _result()
{
    eRet            ret        = eRet_Ok;
    MRect           rectLabel  = geometry;
    MRect           rectButton = geometry;
    blabel_settings labelSettings;

    /* modal msg box so capture key down events (for consuming) */
    blabel_get(getWidget(), &labelSettings);
    labelSettings.widget.key_down = msg_box_bwidget_key_down;
    labelSettings.widget.data     = this;
    blabel_set(getWidget(), &labelSettings);

    setBackgroundColor(0xCC222222);
    setBevel(0);
    setZOrder(getZOrder() + 1);

    rectLabel.setX(5);
    rectLabel.setY(5);
    rectLabel.setWidth(geometry.width() - 10);
    rectLabel.setBottom(geometry.height() - 30);
    _pLabel = new CWidgetLabel("CWidgetModelMsgBox::_pLabel", engine, this, rectLabel, font);
    _pLabel->setBackgroundFillMode(fill_eGradient);
    _pLabel->setBackgroundGradient(COLOR_EGGSHELL + 2 * COLOR_STEP, COLOR_EGGSHELL, COLOR_EGGSHELL - 4 * COLOR_STEP);
    /*_pLabel->setTextColor(0xFF222222);*/

    rectButton.setWidth(75);
    rectButton.setHeight(20);
    rectButton.setX(geometry.width() / 3 - (rectButton.width() / 2));
    rectButton.setY(geometry.height() - 25);
    _pOk = new CWidgetButton("CWidgetModalMsgBox::_pOk", engine, this, rectButton, font);
    CHECK_ERROR_GOTO("unable to create button widget image", ret, error);
    _pOk->setBackgroundFillMode(fill_eGradient);
    _pOk->setBackgroundGradient(COLOR_GREY_DARK + 4 * COLOR_STEP, COLOR_GREY_DARK, COLOR_GREY_DARK - 1 * COLOR_STEP);
    _pOk->setTextColor(COLOR_EGGSHELL);
    _pOk->setBevel(0);
    _pOk->setText("Ok");

    rectButton.setWidth(75);
    rectButton.setHeight(20);
    rectButton.setX(geometry.width() / 3 * 2 - (rectButton.width() / 2));
    rectButton.setY(geometry.height() - 25);
    _pCancel = new CWidgetButton("CWidgetModalMsgBox::_pCancel", engine, this, rectButton, font);
    CHECK_ERROR_GOTO("unable to create button widget image", ret, error);
    _pCancel->setBackgroundFillMode(fill_eGradient);
    _pCancel->setBackgroundGradient(COLOR_GREY_DARK + 4 * COLOR_STEP, COLOR_GREY_DARK, COLOR_GREY_DARK - 1 * COLOR_STEP);
    _pCancel->setTextColor(COLOR_EGGSHELL);
    _pCancel->setBevel(0);
    _pCancel->setText("Cancel");

    /* reparent bwin to framebuffer so we can draw outside the bounds of our bwidget parent */
    {
        bwin_t        msgBoxWin = getWin();
        bwin_settings msgBoxSettings;

        /* traverse up bwin hierarchy to get to framebuffer - save in msgBoxWin */
        bwin_get(msgBoxWin, &msgBoxSettings);
        while (NULL != msgBoxSettings.parent)
        {
            msgBoxWin = msgBoxSettings.parent;
            bwin_get(msgBoxWin, &msgBoxSettings);
        }
        setParentWin(msgBoxWin);
    }
error:
    return;
}

CWidgetModalMsgBox::~CWidgetModalMsgBox()
{
    /* reparent before destroying because all windows directly
     * descended from the framebuffer win don't seem to unlink automatically.
     * changing the parent win to NULL will force the unlink to occur before
     * destruction. */
    setParentWin(NULL);

    DEL(_pLabel);
    DEL(_pOk);
    DEL(_pCancel);
}

/* given NULL button names will result in that button being hidden */
MString CWidgetModalMsgBox::showModal(
        const char * msg,
        uint8_t      focusButton,
        const char * name1,
        const char * name2,
        const char * name3
        )
{
    uint8_t numVisibleButtons = 0;
    MRect   geometry          = getGeometry();
    MRect   rectButton;

    BDBG_ASSERT(msg);
    BDBG_ASSERT((false == isVisible()));
    BSTD_UNUSED(name3);

    {
        MRect geometry = getGeometry();
        MRect rect     = _pLabel->getGeometry();
        int   height   = geometry.height();

        if ((NULL != name1) || (NULL != name2) || (NULL != name3))
        {
            /* buttons exist so adjust layout */
            height -= 30;
        }

        rect.setBottom(height - 5);
        _pLabel->setGeometry(rect);
    }

    _pLabel->setText(msg);

    if (NULL != name1)
    {
        _pOk->setText(name1);
        _pOk->show(true);
        numVisibleButtons++;
    }
    else
    {
        _pOk->show(false);
    }

    if (NULL != name2)
    {
        _pCancel->setText(name2);
        _pCancel->show(true);
        numVisibleButtons++;
    }
    else
    {
        _pCancel->show(false);
    }

    /* DTT TODO: handle button3 if non NULL */

    /* adjust layout depending on the number of visible buttons */
    {
        uint8_t buttonPosition = 1;

        if (NULL != name1)
        {
            rectButton = _pOk->getGeometry();
            rectButton.setX(geometry.width() / (numVisibleButtons + 1) * buttonPosition - (rectButton.width() / 2));
            _pOk->setGeometry(rectButton);
            buttonPosition++;
        }

        if (NULL != name2)
        {
            rectButton = _pCancel->getGeometry();
            rectButton.setX(geometry.width() / (numVisibleButtons + 1) * buttonPosition - (rectButton.width() / 2));
            _pCancel->setGeometry(rectButton);
            buttonPosition++;
        }

        /* DTT TODO: handle button3 if non NULL */
    }

    show(true);
    bwidget_set_modal(getEngine(), getWidget());

    switch (focusButton)
    {
    case 0:
        _pOk->setFocus();
        break;
    case 1:
        _pCancel->setFocus();
        break;
    default:
        break;
    } /* switch */

    _done = false;
    while ((false == _done) && (!bwidget_process_event(getEngine())))
    {
    }

    bwidget_set_modal(getEngine(), NULL);
    show(false);

    return(_result);
} /* showModal */

void CWidgetModalMsgBox::cancelModal(const char * name)
{
    _result = name;
    _done   = true;
}

void CWidgetModalMsgBox::onClick(bwidget_t widget)
{
    if (_pOk->getWidget() == widget)
    {
        _done   = true;
        _result = _pOk->getText();
    }
    else
    if (_pCancel->getWidget() == widget)
    {
        _done   = true;
        _result = _pCancel->getText();
    }
}

eRet CWidgetModalMsgBox::onKeyDown(
        bwidget_t   widget,
        bwidget_key key
        )
{
    eRet ret = eRet_Ok; /* assume consume keys */

    BSTD_UNUSED(widget);

    if (true == bwidget_is_modal(getEngine(), getWidget()))
    {
        if ((bwidget_key_left == key) || (bwidget_key_right == key))
        {
            ret = eRet_NotAvailable; /* do NOT consume these keys */
        }
    }

    return(ret);
}

void CWidgetModalMsgBox::setText(
        const char *          pText,
        bwidget_justify_horiz justifyHoriz,
        bwidget_justify_vert  justifyVert
        )
{
    _pLabel->setText(pText, justifyHoriz, justifyVert);
}