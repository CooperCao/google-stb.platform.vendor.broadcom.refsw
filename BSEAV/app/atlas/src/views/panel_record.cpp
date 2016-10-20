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

#include "panel_record.h"
#include "string.h"
#include "bdbg.h"

BDBG_MODULE(atlas_panel_record);

static int msg_box_bwidget_key_down(
        bwidget_t         widget,
        const bwidget_key key
        )
{
    eRet            ret = eRet_NotAvailable; /* assume key is not consumed */
    CPanelRecord *  pPopup;
    blabel_settings labelSettings;

    blabel_get(widget, &labelSettings);
    pPopup = (CPanelRecord *)labelSettings.widget.data;
    BDBG_ASSERT(NULL != pPopup);

    ret = pPopup->onKeyDown(widget, key);

    return((eRet_Ok == ret) ? 0 : -1);
}

CPanelRecord::CPanelRecord(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelRecord", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pRecordEncryptMenu(NULL),
    _done(false),
    _pOk(NULL),
    _pCancel(NULL),
    _result(),
#if NEXUS_HAS_SECURITY
    _AES(NULL),
    _3DES(NULL),
    _DES(NULL),
#endif
    _Clear(NULL),
    _security("none")
{
}

CPanelRecord::~CPanelRecord()
{
    uninitialize();
}

eRet CPanelRecord::initialize(
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
    int         menuHeight     = 185;
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
    setGeometry(rectWindow);

    setBackgroundColor(0xCC222222);
    setBevel(0);
    setZOrder(getZOrder() + 1);

    _pRecordEncryptMenu = new CWidgetMenu("CPanelRecord::_pRecordEncryptMenu", getEngine(), this, MRect(0, 0, menuWidth, menuHeight), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pRecordEncryptMenu, ret, eRet_OutOfMemory, error);
    _pRecordEncryptMenu->setMenuTitle("Record Encrypt Menu", "Security Algorithm");
    _pRecordEncryptMenu->setScroll(false);
    _pRecordEncryptMenu->show(true);

    /* popups won't work in modal windows since focus can't move to the popped up menu properly : TODO: JOSE*/
#if PROBLEM_WITH_POPUP_IN_MODAL_WINDOWS
    {
        CWidgetCheckButton * pButton = NULL;
        MRect                rectPopup;

        /* Power Mode */
        ret = addLabelPopupButton(_pRecordEncryptMenu, "Power Mode", &_pRecordMode, &_pRecordLabel, &_pRecordPopup, font12, 25);
        CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
        _pRecordMode->setFocusable(false);
        _pRecordLabel->setText("Encryption:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        _pRecordPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
        rectPopup = _pRecordPopup->getGeometry();
        /* add buttons for supported power modes */
        for (int i = 0; i < 5; i++)
        {
            if (4 < i)
            {
                /* add button */
                pButton = _pRecordPopup->addButton(i, rectPopup.width(), rectPopup.height());
                CHECK_PTR_ERROR_GOTO("unable to add record encryption button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(i);
            }
        }
        _pRecordPopup->select(0);
    }
#endif /* if PROBLEM_WITH_POPUP_IN_MODAL_WINDOWS */

    {
        _Clear = new CWidgetCheckButton("CPanelRecord::_Clear", getEngine(), this, MRect(0, 0, 0, 22), font12, _pRecordEncryptMenu->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Clear, ret, eRet_OutOfMemory, error);
        _pRecordEncryptMenu->addButton(_Clear, "No   Encryption");
        _Clear->setText("Clear :", bwidget_justify_horiz_left);
        _Clear->setCheck(true);
        _security = "none";

#if NEXUS_HAS_SECURITY
        _AES = new CWidgetCheckButton("CPanelRecord::_AES", getEngine(), this, MRect(0, 0, 0, 22), font12, _pRecordEncryptMenu->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _AES, ret, eRet_OutOfMemory, error);
        _pRecordEncryptMenu->addButton(_AES, "AES(Aes128) Encryption");
        _AES->setText("AES :", bwidget_justify_horiz_left);
        _AES->setCheck(false);

        _3DES = new CWidgetCheckButton("CPanelRecord::_3DES", getEngine(), this, MRect(0, 0, 0, 22), font12, _pRecordEncryptMenu->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _3DES, ret, eRet_OutOfMemory, error);
        _pRecordEncryptMenu->addButton(_3DES, "3DES (3DesAba) Encryption");
        _3DES->setText("3DES :", bwidget_justify_horiz_left);
        _3DES->setCheck(false);

        _DES = new CWidgetCheckButton("CPanelRecord::_DES", getEngine(), this, MRect(0, 0, 0, 22), font12, _pRecordEncryptMenu->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _DES, ret, eRet_OutOfMemory, error);
        _pRecordEncryptMenu->addButton(_DES, "Des   Encryption");
        _DES->setText("DES :", bwidget_justify_horiz_left);
        _DES->setCheck(false);
#endif /* if NEXUS_HAS_SECURITY */
    }

    rectButton.setWidth(65);
    rectButton.setHeight(20);
    rectButton.setX(rectWindow.width() / 3 - (rectButton.width() / 2));
    rectButton.setY(rectWindow.height() - 25);
    _pOk = new CWidgetButton("CPanelRecord::_pOk", getEngine(), this, rectButton, font12);
    CHECK_ERROR_GOTO("unable to create button widget image", ret, error);
    _pOk->setBackgroundFillMode(fill_eGradient);
    _pOk->setBackgroundGradient(COLOR_GREY_DARK + 4 * COLOR_STEP, COLOR_GREY_DARK, COLOR_GREY_DARK - 1 * COLOR_STEP);
    _pOk->setTextColor(COLOR_EGGSHELL);
    _pOk->setBevel(0);
    _pOk->setText("Ok");

    rectButton.setWidth(65);
    rectButton.setHeight(20);
    rectButton.setX(rectWindow.width() / 3 * 2 - (rectButton.width() / 2));
    rectButton.setY(rectWindow.height() - 25);
    _pCancel = new CWidgetButton("CPanelRecord::_pCancel", getEngine(), this, rectButton, font12);
    CHECK_ERROR_GOTO("unable to create button widget image", ret, error);
    _pCancel->setBackgroundFillMode(fill_eGradient);
    _pCancel->setBackgroundGradient(COLOR_GREY_DARK + 4 * COLOR_STEP, COLOR_GREY_DARK, COLOR_GREY_DARK - 1 * COLOR_STEP);
    _pCancel->setTextColor(COLOR_EGGSHELL);
    _pCancel->setBevel(0);
    _pCancel->setText("Cancel");

error:
    return(ret);
} /* initialize */

void CPanelRecord::uninitialize()
{
    DEL(_pOk);
    DEL(_pCancel);
#if PROBLEM_WITH_POPUP_IN_MODAL_WINDOWS
    DEL(_pRecordPopup);
    DEL(_pRecordLabel);
    DEL(_pRecordMode);
#endif
    DEL(_Clear);
#if NEXUS_HAS_SECURITY
    DEL(_AES);
    DEL(_3DES);
    DEL(_DES);
#endif
    DEL(_pRecordEncryptMenu);
} /* uninitialize */

void CPanelRecord::show(bool bShow)
{
    CPanel::show(bShow);
}

MString CPanelRecord::showModal()
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

void CPanelRecord::onClick(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;
    CModel *      pModel  = getModel();
    CRecordData   recordData;

    BDBG_ASSERT(NULL != pModel);

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }

#if PROBLEM_WITH_POPUP_IN_MODAL_WINDOWS
    if (0 <= _pRecordPopup->getItemListIndex(pWidget->getWidget()))
    {
        CWidgetCheckButton * pButton = (CWidgetCheckButton *)pWidget;
        BDBG_WRN(("selected power mode:%s", pButton->getText().s()));

        /*
         * save selected power mode
         * _securityAlg = pWidget->;
         */
    }
    else
#endif /* if PROBLEM_WITH_POPUP_IN_MODAL_WINDOWS */

    if (_Clear->getWidget() == widget)
    {
#if NEXUS_HAS_SECURITY
        _AES->setCheck(false);
        _3DES->setCheck(false);
        _DES->setCheck(false);
#endif
        _security = "none";
    }
    else
#if NEXUS_HAS_SECURITY
    if (_AES->getWidget() == widget)
    {
        _Clear->setCheck(false);
        _DES->setCheck(false);
        _3DES->setCheck(false);
        _security = "aes";
    }
    else
    if (_3DES->getWidget() == widget)
    {
        _Clear->setCheck(false);
        _AES->setCheck(false);
        _DES->setCheck(false);
        _security = "3des";
    }
    else
    if (_DES->getWidget() == widget)
    {
        _Clear->setCheck(false);
        _3DES->setCheck(false);
        _AES->setCheck(false);
        _security = "des";
    }
    else
#endif /* if NEXUS_HAS_SECURITY */
    if (_pOk->getWidget() == widget)
    {
        _done                = true;
        _result              = _pOk->getText();
        recordData._security = _security;
        BDBG_MSG(("_security is set  %s , pRecordData->_security %s ", _security.s(), recordData._security.s()));
        notifyObservers(eNotify_RecordStart, &recordData);
        /* Leak of pRecordData, might need local variable. */
    }
    else
    if (_pCancel->getWidget() == widget)
    {
        _done   = true;
        _result = _pCancel->getText();
    }

    BDBG_MSG((" _security is set to _security %s ", _security.s()));
} /* onClick */

eRet CPanelRecord::onKeyDown(
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

void CPanelRecord::processNotification(CNotification & notification)
{
    BSTD_UNUSED(notification);
}