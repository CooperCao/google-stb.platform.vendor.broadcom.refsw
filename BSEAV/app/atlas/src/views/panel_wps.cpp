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

#include "panel_wps.h"
#include "string.h"
#include "bdbg.h"

BDBG_MODULE(atlas_panel_wps);

static int msg_box_bwidget_key_down(
        bwidget_t         widget,
        const bwidget_key key
        )
{
    eRet            ret = eRet_NotAvailable; /* assume key is not consumed */
    CPanelWps *     pPopup;
    blabel_settings labelSettings;

    blabel_get(widget, &labelSettings);
    pPopup = (CPanelWps *)labelSettings.widget.data;
    BDBG_ASSERT(NULL != pPopup);

    ret = pPopup->onKeyDown(widget, key);

    return((eRet_Ok == ret) ? 0 : -1);
}

CPanelWps::CPanelWps(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelWps", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pMenuBar(NULL),
    _pTextLabel1(NULL),
    _pTextLabel2(NULL),
    _pWpsLogo(NULL),
    _pProgress(NULL),
    _done(false),
    _pCancel(NULL),
    _maxSeconds(120),
    _timerProgress(pWidgetEngine, this, 1000),
    _timerClose(pWidgetEngine, this, 2000)
{
}

CPanelWps::~CPanelWps()
{
    uninitialize();
}

eRet CPanelWps::initialize(
        CModel *  pModel,
        CConfig * pConfig
        )
{
    eRet        ret            = eRet_Ok;
    CGraphics * pGraphics      = NULL;
    bwin_font_t font10         = NULL;
    bwin_font_t font12         = NULL;
    bwin_font_t font14         = NULL;
    uint32_t    graphicsWidth  = 0;
    uint32_t    graphicsHeight = 0;
    int         menuWidth      = 230;
    int         menuHeight     = 170;
    MRect       rectWindow;
    MRect       rectButton;
    MRect       rectText;
    MRect       rectWps;
    MRect       rectProgress;

    BDBG_ASSERT(NULL != pModel);
    BDBG_ASSERT(NULL != pConfig);

    setModel(pModel);
    setConfig(pConfig);

    pGraphics = pModel->getGraphics();
    BDBG_ASSERT(NULL != pGraphics);

    font10 = pGraphics->getFont(10);
    BDBG_ASSERT(NULL != font10);
    font12 = pGraphics->getFont(12);
    BDBG_ASSERT(NULL != font12);
    font14 = pGraphics->getFont(14);
    BDBG_ASSERT(NULL != font14);

    graphicsWidth  = GET_INT(_pCfg, GRAPHICS_SURFACE_WIDTH);
    graphicsHeight = GET_INT(_pCfg, GRAPHICS_SURFACE_HEIGHT);

    rectWindow.setX(graphicsWidth / 2 - menuWidth / 2);
    rectWindow.setY(graphicsHeight / 2 - menuHeight / 2);
    rectWindow.setWidth(menuWidth);
    rectWindow.setHeight(menuHeight);
    setGeometry(rectWindow);

    setBackgroundColor(0xCC222222);
    setBevel(0);
    setZOrder(getZOrder() + 1);

    _pMenuBar = new CWidgetLabel("CPanelWps::_pMenuBar", getEngine(), this, MRect(0, 0, menuWidth, 30), font14);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu bar widget", _pMenuBar, ret, eRet_OutOfMemory, error);
    _pMenuBar->setBackgroundFillMode(fill_eGradient);
    _pMenuBar->setBackgroundGradient(COLOR_GREY_DARK + 4 * COLOR_STEP, COLOR_GREY_DARK, COLOR_GREY_DARK - 1 * COLOR_STEP);
    _pMenuBar->setText("WPS Setup", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
    _pMenuBar->setTextColor(COLOR_EGGSHELL);
    _pMenuBar->setBevel(0);

    /* text labels */
    rectText.setWidth(rectWindow.width() - 20);
    rectText.setHeight(20);
    rectText.setX(10);
    rectText.setY(40);
    _pTextLabel1 = new CWidgetLabel("CPanelWps::_pTextLabel1", getEngine(), this, rectText, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate text label widget", _pTextLabel1, ret, eRet_OutOfMemory, error);
    _pTextLabel1->setTextColor(COLOR_EGGSHELL);
    _pTextLabel1->setBevel(0);
    _pTextLabel1->setBackgroundColor(getBackgroundColor());

    rectText.setY(rectText.y() + 20);
    _pTextLabel2 = new CWidgetLabel("CPanelWps::_pTextLabel2", getEngine(), this, rectText, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate text label widget", _pTextLabel2, ret, eRet_OutOfMemory, error);
    _pTextLabel2->setTextColor(COLOR_EGGSHELL);
    _pTextLabel2->setBevel(0);
    _pTextLabel2->setBackgroundColor(getBackgroundColor());

    /* WPS icon */
    {
        rectWps.setWidth(25);
        rectWps.setHeight(25);
        rectWps.setX((rectWindow.width() / 2) - (rectWps.width() / 2));
        rectWps.setY(rectText.y() + rectText.height() + 10);

        /* create Wifi Protected Setup (WPS) logo */
        _pWpsLogo = new CWidgetLabel("CPanelWps::_pWpsLabel", getEngine(), this, rectWps, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate logo widget", _pWpsLogo, ret, eRet_OutOfMemory, error);
        _pWpsLogo->setTextColor(COLOR_EGGSHELL);
        _pWpsLogo->setBevel(0);
        _pWpsLogo->setBackgroundColor(getBackgroundColor());
        _pWpsLogo->loadImage("images/wps_logo_small.png");
    }

    /* progress */
    {
        rectProgress.setX(10);
        rectProgress.setY(rectWps.y() + rectWps.height() + 10);
        rectProgress.setWidth(rectWindow.width() - 20);
        rectProgress.setHeight(7);

        _pProgress = new CWidgetProgress("CPanelWps::_pProgress", getEngine(), this, rectProgress, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pProgress, ret, eRet_OutOfMemory, error);
        _pProgress->setLevel(0);
        _pProgress->setBevel(0);
        _pProgress->setBackgroundFillMode(fill_eGradient);
        _pProgress->setBackgroundGradient(0xFF676767 - 3 * COLOR_STEP, 0xFF676767, 0xFF676767 + 2 * COLOR_STEP);
        _pProgress->showText(false);
        _pProgress->showProgress(true);
    }

    /* cancel button */
    {
        rectButton.setWidth(65);
        rectButton.setHeight(20);
        rectButton.setX(rectWindow.width() / 2 - (rectButton.width() / 2));
        rectButton.setY(rectWindow.height() - 25);
        _pCancel = new CWidgetButton("CPanelWps::_pCancel", getEngine(), this, rectButton, font12);
        CHECK_ERROR_GOTO("unable to create button widget image", ret, error);
        _pCancel->setBackgroundFillMode(fill_eGradient);
        _pCancel->setBackgroundGradient(COLOR_GREY_DARK + 4 * COLOR_STEP, COLOR_GREY_DARK, COLOR_GREY_DARK - 1 * COLOR_STEP);
        _pCancel->setTextColor(COLOR_EGGSHELL);
        _pCancel->setBevel(0);
        _pCancel->setText("Cancel");
    }

error:
    return(ret);
} /* initialize */

void CPanelWps::uninitialize()
{
    DEL(_pCancel);
    DEL(_pProgress);
    DEL(_pWpsLogo);
    DEL(_pTextLabel2);
    DEL(_pTextLabel1);
    DEL(_pMenuBar);
} /* uninitialize */

void CPanelWps::show(bool bShow)
{
    _pTextLabel1->setText("Press the Wifi Protected Setup", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
    _pTextLabel2->setText("(WPS) button on your router.", bwidget_justify_horiz_left, bwidget_justify_vert_middle);

    CPanel::show(bShow);
    if (true == bShow)
    {
        _timerProgress.start();
    }
    else
    {
        _timerProgress.stop();
    }
}

MString CPanelWps::showModal()
{
    MRect           geometry = getGeometry();
    MRect           rectButton;
    blabel_settings labelSettingsOrig;
    blabel_settings labelSettings;
    bwidget_t       widgetFocus;

    {
        /* save currently focused widget */
        widgetFocus = bwidget_get_focus(getEngine());

        /* modal screen so capture key down events (for consuming) */
        blabel_get(getWidget(), &labelSettings);

        /* save original settings */
        labelSettingsOrig = labelSettings;

        labelSettings.widget.key_down = msg_box_bwidget_key_down;
        labelSettings.widget.data     = this;
        blabel_set(getWidget(), &labelSettings);
    }

    BDBG_MSG(("Geometry x %d, y %d", geometry.x(), geometry.y()));

    _pProgress->setLevel(0);

    show(true);
    bwidget_set_modal(getEngine(), getWidget());

    _pCancel->setFocus();

    /* enter local bwidget event loop so we can handle all keypresses locally */
    _done = false;
    while ((false == _done) && (!bwidget_process_event(getEngine())))
    {
    }

    /* cancelModal() was called or _pCancel button was pressed */

    bwidget_set_modal(getEngine(), NULL);
    show(false);

    {
        /* restore original settings */
        blabel_get(getWidget(), &labelSettings);
        labelSettings = labelSettingsOrig;
        blabel_set(getWidget(), &labelSettings);

        /* restore previously focused widget */
        bwidget_set_focus(widgetFocus);
    }

    return(_result);
} /* showModal */

void CPanelWps::onClick(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;
    CModel *      pModel  = getModel();

    BDBG_ASSERT(NULL != pModel);

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }

    if (_pCancel->getWidget() == widget)
    {
        _result = _pCancel->getText();
        _done   = true;
    }
} /* onClick */

eRet CPanelWps::onKeyDown(
        bwidget_t   widget,
        bwidget_key key
        )
{
    eRet ret = eRet_Ok; /* assume consume keys */

    BSTD_UNUSED(widget);

    if (true == bwidget_is_modal(getEngine(), getWidget()))
    {
        if ((bwidget_key_left == key) ||
            (bwidget_key_right == key) ||
            (bwidget_key_up == key) ||
            (bwidget_key_down == key) ||
            (bwidget_key_select == key))
        {
            ret = eRet_NotAvailable; /* do NOT consume these keys */
        }
    }

    return(ret);
} /* onKeyDown */

void CPanelWps::processNotification(CNotification & notification)
{
    BSTD_UNUSED(notification);

    switch (notification.getId())
    {
    case eNotify_Timeout:
    {
        CTimer * pTimer = (CTimer *)notification.getData();
        if (&_timerProgress == pTimer)
        {
            uint16_t nOneSecond = 65535 / _maxSeconds;
            uint16_t nLevel     = _pProgress->getLevel() + nOneSecond;

            _pProgress->setLevel(nLevel);
            if (nLevel > (65535 - (nOneSecond / 2)))
            {
                /* progress done */
                _timerProgress.stop();
                _result = _pCancel->getText();
                _done   = true;
            }
            else
            {
                /* not at 100% restart timer */
                _timerProgress.start();
            }
        }
        else
        if (&_timerClose == pTimer)
        {
            cancelModal("");
        }
    }
    break;

    case eNotify_NetworkWifiConnectAssocStart:
    {
        MString * pStr = (MString *)notification.getData();

        _pTextLabel1->setText(MString("Negotiate with: ") + *pStr);
        _pTextLabel2->setText("");
    }
    break;

    case eNotify_NetworkWifiConnected:
        _pTextLabel2->setText("Connected!");
        _timerClose.start();
        break;

    case eNotify_NetworkWifiConnectFailure:
    case eNotify_NetworkWifiConnectFailureAssocReject:
    case eNotify_NetworkWifiConnectFailureNetworkNotFound:
    {
        CWifi * pWifi = (CWifi *)notification.getData();

        if (notification.getId() == eNotify_NetworkWifiConnectFailureAssocReject)
        {
            _pTextLabel2->setText("Assoc Rejection Failure");
        }
        else
        if (notification.getId() == eNotify_NetworkWifiConnectFailureNetworkNotFound)
        {
            _pTextLabel2->setText("Missing Network Failure");
        }
        else
        {
            _pTextLabel2->setText("Connection Failure");
        }
    }
    break;
    } /* switch */
} /* processNotification */