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

#include "panel_power.h"
#include "string.h"
#include "bdbg.h"

BDBG_MODULE(panel_power);

static int msg_box_bwidget_key_down(
        bwidget_t         widget,
        const bwidget_key key
        )
{
    eRet            ret = eRet_NotAvailable; /* assume key is not consumed */
    CPanelPower *   pPopup;
    blabel_settings labelSettings;

    blabel_get(widget, &labelSettings);
    pPopup = (CPanelPower *)labelSettings.widget.data;
    BDBG_ASSERT(NULL != pPopup);

    ret = pPopup->onKeyDown(widget, key);

    return((eRet_Ok == ret) ? 0 : -1);
}

CPanelPower::CPanelPower(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelPower", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pMenu(NULL),
    _pModeMenu(NULL),
    _pLinuxMenu(NULL),
    _pMode(NULL),
    _pModeLabel(NULL),
    _pModePopup(NULL),
    _StandbyLinuxSata(NULL),
    _StandbyLinuxUsb(NULL),
    _StandbyLinuxCpuDiv(NULL),
    _StandbyLinuxTp1(NULL),
    _StandbyLinuxTp2(NULL),
    _StandbyLinuxTp3(NULL),
    _StandbyLinuxDdr(NULL),
    _StandbyLinuxEnet(NULL),
    _StandbyLinuxMoca(NULL),
    _done(false),
    _pOk(NULL),
    _pCancel(NULL),
    _result(),
    _powerMode(ePowerMode_Max)
{
}

CPanelPower::~CPanelPower()
{
    uninitialize();
}

eRet CPanelPower::initialize(
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
    int         menuWidth      = 220;
    int         menuHeight     = 350;
    MRect       rectWindow;
    MRect       rectButton;

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

    setGeometry(MRect(0, 0, graphicsWidth, graphicsHeight));
    /* loadImage("images/transpBlack50.png", bwin_image_render_mode_tile); */
    setBackgroundColor(0x77000000);
    setBevel(0);
    setZOrder(getZOrder() + 1);

    _pMenu = new CWidgetMenu("CPanelPower::_pMenu", getEngine(), this, rectWindow, font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pMenu, ret, eRet_OutOfMemory, error);
    _pMenu->showSubTitle(false);
    _pMenu->showListView(false);
    _pMenu->showEdit(false);
    _pMenu->setMenuTitle("Standby", NULL);
    _pMenu->show(true);
    {
        MRect rectLinux(0, 30, menuWidth, 244);

        _pLinuxMenu = new CWidgetMenu("CPanelPower::_pLinuxMenu", getEngine(), _pMenu, rectLinux, font14, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pLinuxMenu, ret, eRet_OutOfMemory, error);
        _pLinuxMenu->showMenuBar(false);
        _pLinuxMenu->setMenuTitle(NULL, "Linux");
        _pLinuxMenu->setScroll(true);
        {
            _StandbyLinuxSata = new CWidgetCheckButton("CPanelPower::_StandbyLinuxSata", getEngine(), this, MRect(0, 0, 0, 22), font12, _pLinuxMenu->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _StandbyLinuxSata, ret, eRet_OutOfMemory, error);
            _pLinuxMenu->addButton(_StandbyLinuxSata, "SATA Standby");
            _StandbyLinuxSata->setText("SATA Standby:", bwidget_justify_horiz_left);
            _StandbyLinuxSata->setCheck(GET_BOOL(_pCfg, POWER_MGMT_SATA));

            _StandbyLinuxUsb = new CWidgetCheckButton("CPanelPower::_StandbyLinuxUsb", getEngine(), this, MRect(0, 0, 0, 22), font12, _pLinuxMenu->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _StandbyLinuxUsb, ret, eRet_OutOfMemory, error);
            _pLinuxMenu->addButton(_StandbyLinuxUsb, "USB Standby");
            _StandbyLinuxUsb->setText("USB Standby:", bwidget_justify_horiz_left);
            _StandbyLinuxUsb->setCheck(GET_BOOL(_pCfg, POWER_MGMT_USB));

            _StandbyLinuxEnet = new CWidgetCheckButton("CPanelPower::_StandbyLinuxEnet", getEngine(), this, MRect(0, 0, 0, 22), font12, _pLinuxMenu->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _StandbyLinuxEnet, ret, eRet_OutOfMemory, error);
            _pLinuxMenu->addButton(_StandbyLinuxEnet, "Ethernet Standby");
            _StandbyLinuxEnet->setText("Ethernet Standby:", bwidget_justify_horiz_left);
            _StandbyLinuxEnet->setCheck(GET_BOOL(_pCfg, POWER_MGMT_ENET));

            _StandbyLinuxDdr = new CWidgetCheckButton("CPanelPower::_StandbyLinuxDdr", getEngine(), this, MRect(0, 0, 0, 22), font12, _pLinuxMenu->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _StandbyLinuxDdr, ret, eRet_OutOfMemory, error);
            _pLinuxMenu->addButton(_StandbyLinuxDdr, "DDR Self Refresh");
            _StandbyLinuxDdr->setText("DDR Self Refresh:", bwidget_justify_horiz_left);
            _StandbyLinuxDdr->setCheck(GET_BOOL(_pCfg, POWER_MGMT_DDR));

            _StandbyLinuxCpuDiv = new CWidgetCheckButton("CPanelPower::_StandbyLinuxCpuDiv", getEngine(), this, MRect(0, 0, 0, 22), font12, _pLinuxMenu->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _StandbyLinuxCpuDiv, ret, eRet_OutOfMemory, error);
            _pLinuxMenu->addButton(_StandbyLinuxCpuDiv, "CPU Divisor");
            _StandbyLinuxCpuDiv->setText(MString("CPU Divisor (") + GET_STR(_pCfg, POWER_MGMT_CPU_DIVISOR) + "):", bwidget_justify_horiz_left);
            _StandbyLinuxCpuDiv->setCheck((1 == GET_INT(_pCfg, POWER_MGMT_CPU_DIVISOR)) ? false : true);

            _StandbyLinuxTp1 = new CWidgetCheckButton("CPanelPower::_StandbyLinuxTp1", getEngine(), this, MRect(0, 0, 0, 22), font12, _pLinuxMenu->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _StandbyLinuxTp1, ret, eRet_OutOfMemory, error);
            _pLinuxMenu->addButton(_StandbyLinuxTp1, "TP1 Disable");
            _StandbyLinuxTp1->setText("TP1 Disable:", bwidget_justify_horiz_left);
            _StandbyLinuxTp1->setCheck(GET_BOOL(_pCfg, POWER_MGMT_TP1));

            _StandbyLinuxTp2 = new CWidgetCheckButton("CPanelPower::_StandbyLinuxTp2", getEngine(), this, MRect(0, 0, 0, 22), font12, _pLinuxMenu->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _StandbyLinuxTp2, ret, eRet_OutOfMemory, error);
            _pLinuxMenu->addButton(_StandbyLinuxTp2, "TP2 Disable");
            _StandbyLinuxTp2->setText("TP2 Disable:", bwidget_justify_horiz_left);
            _StandbyLinuxTp2->setCheck(GET_BOOL(_pCfg, POWER_MGMT_TP2));

            _StandbyLinuxTp3 = new CWidgetCheckButton("CPanelPower::_StandbyLinuxTp3", getEngine(), this, MRect(0, 0, 0, 22), font12, _pLinuxMenu->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _StandbyLinuxTp3, ret, eRet_OutOfMemory, error);
            _pLinuxMenu->addButton(_StandbyLinuxTp3, "TP3 Disable");
            _StandbyLinuxTp3->setText("TP3 Disable:", bwidget_justify_horiz_left);
            _StandbyLinuxTp3->setCheck(GET_BOOL(_pCfg, POWER_MGMT_TP3));

            _StandbyLinuxMoca = new CWidgetCheckButton("CPanelPower::_StandbyLinuxMoca", getEngine(), this, MRect(0, 0, 0, 22), font12, _pLinuxMenu->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _StandbyLinuxMoca, ret, eRet_OutOfMemory, error);
            _pLinuxMenu->addButton(_StandbyLinuxMoca, "Moca Standby");
            _StandbyLinuxMoca->setText("MOCA Standby:", bwidget_justify_horiz_left);
            _StandbyLinuxMoca->setCheck(GET_BOOL(_pCfg, POWER_MGMT_MOCA));
        }

        {
            MRect rectMode;

            rectMode.setX(0);
            rectMode.setY(rectLinux.y() + rectLinux.height() - 7);
            rectMode.setWidth(menuWidth);
            rectMode.setHeight(58);

            _pModeMenu = new CWidgetMenu("CPanelPower::_pModeMenu", getEngine(), _pMenu, rectMode, font14, font12);
            CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pModeMenu, ret, eRet_OutOfMemory, error);
            _pModeMenu->showMenuBar(false);
            _pModeMenu->setMenuTitle(NULL, "Mode");
            _pModeMenu->setScroll(true);

            {
                CWidgetCheckButton * pButton = NULL;
                MRect                rectPopup;

                /* Power Mode */
                ret = _pModeMenu->addLabelPopupButton(this, "Power Mode", &_pMode, &_pModeLabel, &_pModePopup, font12, 25);
                CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
                _pMode->setFocusable(false);
                _pModeLabel->setText("Mode:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
                _pModePopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
                rectPopup = _pModePopup->getGeometry();
                /* add buttons for supported power modes */
                for (int i = 0; i < ePowerMode_Max; i++)
                {
                    if (ePowerMode_S0 < i)
                    {
                        /* add button */
                        pButton = _pModePopup->addButton(powerModeToString((ePowerMode)i), rectPopup.width(), rectPopup.height());
                        CHECK_PTR_ERROR_GOTO("unable to add power mode button to popup list", pButton, ret, eRet_OutOfMemory, error);
                        pButton->setValue(i);
                    }
                }
                /*_pModePopup->sort();*/
                _powerMode = ePowerMode_S1;
                _pModePopup->select(_powerMode);
            }
        }
    }

    rectButton.setWidth(65);
    rectButton.setHeight(20);
    rectButton.setX(rectWindow.width() / 3 - (rectButton.width() / 2));
    rectButton.setY(rectWindow.height() - 25);
    _pOk = new CWidgetButton("CPanelPower::_pOk", getEngine(), this, rectButton, font12, _pMenu->getWin());
    CHECK_ERROR_GOTO("unable to create button widget image", ret, error);
    _pOk->setBackgroundFillMode(fill_eGradient);
    _pOk->setBackgroundGradient(COLOR_GREY_DARK + 4 * COLOR_STEP, COLOR_GREY_DARK, COLOR_GREY_DARK - 1 * COLOR_STEP);
    _pOk->setTextColor(COLOR_EGGSHELL); _pOk->setBevel(0);
    _pOk->setText("Ok");

    rectButton.setWidth(65);
    rectButton.setHeight(20);
    rectButton.setX(rectWindow.width() / 3 * 2 - (rectButton.width() / 2));
    rectButton.setY(rectWindow.height() - 25);
    _pCancel = new CWidgetButton("CPanelPower::_pCancel", getEngine(), this, rectButton, font12, _pMenu->getWin());
    CHECK_ERROR_GOTO("unable to create button widget image", ret, error);
    _pCancel->setBackgroundFillMode(fill_eGradient);
    _pCancel->setBackgroundGradient(COLOR_GREY_DARK + 4 * COLOR_STEP, COLOR_GREY_DARK, COLOR_GREY_DARK - 1 * COLOR_STEP);
    _pCancel->setTextColor(COLOR_EGGSHELL);
    _pCancel->setBevel(0);
    _pCancel->setText("Cancel");

error:
    return(ret);
} /* initialize */

void CPanelPower::uninitialize()
{
    DEL(_pOk);
    DEL(_pCancel);
    DEL(_pModePopup);
    DEL(_pModeLabel);
    DEL(_pMode);
    DEL(_StandbyLinuxMoca);
    DEL(_StandbyLinuxEnet);
    DEL(_StandbyLinuxDdr);
    DEL(_StandbyLinuxTp3);
    DEL(_StandbyLinuxTp2);
    DEL(_StandbyLinuxTp1);
    DEL(_StandbyLinuxCpuDiv);
    DEL(_StandbyLinuxUsb);
    DEL(_StandbyLinuxSata);
    DEL(_pLinuxMenu);
    DEL(_pModeMenu);
    DEL(_pMenu);
} /* uninitialize */

void CPanelPower::show(bool bShow)
{
    CPanel::show(bShow);
}

MString CPanelPower::showModal()
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

    show(true);
    bwidget_set_modal(getEngine(), getWidget());

    _pOk->setFocus();

    /* enter local bwidget event loop so we can handle all keypresses locally */
    _done = false;
    while ((false == _done) && (!bwidget_process_event(getEngine())))
    {
    }

    /* cancelModal() was called or _pOk or _pCancel button was pressed */

    bwidget_set_modal(getEngine(), NULL);

    {
        /* restore original settings */
        blabel_get(getWidget(), &labelSettings);
        labelSettings = labelSettingsOrig;
        blabel_set(getWidget(), &labelSettings);

        /* restore previously focused widget */
        bwidget_set_focus(widgetFocus);
    }

    show(false);

    return(_result);
} /* showModal */

void CPanelPower::onClick(bwidget_t widget)
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

    if (_StandbyLinuxSata == pWidget) { SET(_pCfg, POWER_MGMT_SATA, _StandbyLinuxSata->isChecked()); }
    if (_StandbyLinuxUsb == pWidget) { SET(_pCfg, POWER_MGMT_USB, _StandbyLinuxUsb->isChecked()); }
    if (_StandbyLinuxCpuDiv == pWidget) { SET(_pCfg, POWER_MGMT_CPU_DIVISOR, _StandbyLinuxCpuDiv->isChecked() ? 8 : 1); }
    if (_StandbyLinuxTp1 == pWidget) { SET(_pCfg, POWER_MGMT_TP1, _StandbyLinuxTp1->isChecked()); }
    if (_StandbyLinuxTp2 == pWidget) { SET(_pCfg, POWER_MGMT_TP2, _StandbyLinuxTp2->isChecked()); }
    if (_StandbyLinuxTp3 == pWidget) { SET(_pCfg, POWER_MGMT_TP3, _StandbyLinuxTp3->isChecked()); }
    if (_StandbyLinuxDdr == pWidget) { SET(_pCfg, POWER_MGMT_DDR, _StandbyLinuxDdr->isChecked()); }
    if (_StandbyLinuxEnet == pWidget) { SET(_pCfg, POWER_MGMT_ENET, _StandbyLinuxEnet->isChecked()); }
    if (_StandbyLinuxMoca == pWidget) { SET(_pCfg, POWER_MGMT_MOCA, _StandbyLinuxMoca->isChecked()); }

    if (0 <= _pModePopup->getItemListIndex(pWidget->getWidget()))
    {
        CWidgetCheckButton * pButton = (CWidgetCheckButton *)pWidget;
        BDBG_MSG(("selected power mode:%s", pButton->getText().s()));

        /* save selected power mode */
        _powerMode = (ePowerMode)pWidget->getValue();
    }
    else
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
} /* onClick */

eRet CPanelPower::onKeyDown(
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

void CPanelPower::processNotification(CNotification & notification)
{
    BSTD_UNUSED(notification);
}