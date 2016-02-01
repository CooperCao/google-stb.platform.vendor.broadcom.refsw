/******************************************************************************
 * (c) 2015 Broadcom Corporation
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

#include "panel_vbi.h"
#include "convert.h"
#include "nexus_display_vbi.h"
#include "screen_main.h"

BDBG_MODULE(panel_vbi);

CPanelVbi::CPanelVbi(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelVbi", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pVbiMenu(NULL),
    _ClosedCaptionPassthru(NULL),
    _Teletext(NULL),
    _VideoProgramSys(NULL),
    _WidescreenSignaling(NULL),
    _Cgms(NULL),
    _Gemstar(NULL),
    _NielsenAmol(NULL),
    _NielsenAmolLabel(NULL),
    _NielsenAmolPopup(NULL),
    _Macrovision(NULL),
    _MacrovisionLabel(NULL),
    _MacrovisionPopup(NULL),
    _Dcs(NULL),
    _DcsLabel(NULL),
    _DcsPopup(NULL),
    _MsgBox(NULL),
    _Back(NULL),
    _bHandleErrors(false)
{
}

CPanelVbi::~CPanelVbi()
{
    uninitialize();
}

eRet CPanelVbi::initialize(
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
    int         menuWidth      = 250;
    int         menuHeight     = 270;
    MRect       rectPanel;

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

    /* set the size of the panel itself */
    rectPanel.set(50, 50, menuWidth, menuHeight);
    setGeometry(rectPanel);

    _pVbiMenu = new CWidgetMenu("CPanelVbi::_pVbiMenu", getEngine(), this, MRect(0, 0, menuWidth, menuHeight), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pVbiMenu, ret, eRet_OutOfMemory, error);
    _pVbiMenu->setMenuTitle("VBI", "Settings");
    _pVbiMenu->setScroll(true);
    _pVbiMenu->show(true);
    {
        CWidgetCheckButton * pButton = NULL;
        MRect                rectPopup;

        /* Closed Captioning Passthru */
        _ClosedCaptionPassthru = new CWidgetCheckButton("CPanelVbi::_ClosedCaptionPassthru", getEngine(), this, MRect(0, 0, 0, 22), font12, _pVbiMenu->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _ClosedCaptionPassthru, ret, eRet_OutOfMemory, error);
        _pVbiMenu->addButton(_ClosedCaptionPassthru, "Closed Captioning Passthru");
        _ClosedCaptionPassthru->setText("Closed Captioning Pass-Through:", bwidget_justify_horiz_left);
        _ClosedCaptionPassthru->setCheck(false);

        /* Teletext */
        _Teletext = new CWidgetCheckButton("CPanelVbi::_Teletext", getEngine(), this, MRect(0, 0, 0, 22), font12, _pVbiMenu->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Teletext, ret, eRet_OutOfMemory, error);
        _pVbiMenu->addButton(_Teletext, "Teletext");
        _Teletext->setText("Teletext:", bwidget_justify_horiz_left);
        _Teletext->setCheck(false);

        /* Video Program System (VPS) */
        _VideoProgramSys = new CWidgetCheckButton("CPanelVbi::_VideoProgramSys", getEngine(), this, MRect(0, 0, 0, 22), font12, _pVbiMenu->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _VideoProgramSys, ret, eRet_OutOfMemory, error);
        _pVbiMenu->addButton(_VideoProgramSys, "Video Program System");
        _VideoProgramSys->setText("Video Program System:", bwidget_justify_horiz_left);
        _VideoProgramSys->setCheck(false);

        /* Widescreen Signaling */
        _WidescreenSignaling = new CWidgetCheckButton("CPanelVbi::_WidescreenSignaling", getEngine(), this, MRect(0, 0, 0, 22), font12, _pVbiMenu->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _WidescreenSignaling, ret, eRet_OutOfMemory, error);
        _pVbiMenu->addButton(_WidescreenSignaling, "Widescreen Signaling");
        _WidescreenSignaling->setText("Widescreen Signaling:", bwidget_justify_horiz_left);
        _WidescreenSignaling->setCheck(false);

        /* CGMS */
        _Cgms = new CWidgetCheckButton("CPanelVbi::_Cgms", getEngine(), this, MRect(0, 0, 0, 22), font12, _pVbiMenu->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Cgms, ret, eRet_OutOfMemory, error);
        _pVbiMenu->addButton(_Cgms, "CGMS");
        _Cgms->setText("CGMS:", bwidget_justify_horiz_left);
        _Cgms->setCheck(false);

        /* Gemstar */
        _Gemstar = new CWidgetCheckButton("CPanelVbi::_Gemstar", getEngine(), this, MRect(0, 0, 0, 22), font12, _pVbiMenu->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Gemstar, ret, eRet_OutOfMemory, error);
        _pVbiMenu->addButton(_Gemstar, "Gemstar");
        _Gemstar->setText("Gemstar:", bwidget_justify_horiz_left);
        _Gemstar->setCheck(false);

        /* Nielsen AMOL */
        ret = addLabelPopupButton(_pVbiMenu, "NielsenAmol", &_NielsenAmol, &_NielsenAmolLabel, &_NielsenAmolPopup, font12, 45);
        CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
        _NielsenAmol->setFocusable(false);
        _NielsenAmolLabel->setText("Nielsen AMOL:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        _NielsenAmolPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
        rectPopup = _NielsenAmolPopup->getGeometry();
        /*
         * add buttons for supported AMOL types
         * add "Off" first in popup list
         */
        pButton = _NielsenAmolPopup->addButton(nielsenAmolToString(NEXUS_AmolType_eMax), rectPopup.width(), rectPopup.height());
        CHECK_PTR_ERROR_GOTO("unable to add Nielsen AMOL button to popup list", pButton, ret, eRet_OutOfMemory, error);
        pButton->setValue(NEXUS_AmolType_eMax);
        for (int i = 0; i < NEXUS_AmolType_eMax; i++)
        {
            /* add AMOL type */
            pButton = _NielsenAmolPopup->addButton(nielsenAmolToString((NEXUS_AmolType)i), rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add Nielsen AMOL button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(i);
        }
        /*_NielsenAmolPopup->sort();*/
        _NielsenAmolPopup->select(NEXUS_AmolType_eMax);

        /* Macrovision */
        ret = addLabelPopupButton(_pVbiMenu, "Macrovision", &_Macrovision, &_MacrovisionLabel, &_MacrovisionPopup, font12, 37);
        CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
        _Macrovision->setFocusable(false);
        _MacrovisionLabel->setText("Macrovision:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        _MacrovisionPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
        rectPopup = _MacrovisionPopup->getGeometry();
        /* add buttons for supported Macrovision types */
        for (int i = 0; i < NEXUS_DisplayMacrovisionType_eMax; i++)
        {
            if (NEXUS_DisplayMacrovisionType_eCustom == i)
            {
                /* custom macrovision tables are not supported */
                continue;
            }

            /* add Macrovision type */
            pButton = _MacrovisionPopup->addButton(macrovisionToString((NEXUS_DisplayMacrovisionType)i), rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add Macrovision button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(i);
        }
        /*_MacrovisionPopup->sort();*/
        _MacrovisionPopup->select(NEXUS_DisplayMacrovisionType_eNone);

        /* DCS */
        ret = addLabelPopupButton(_pVbiMenu, "DCS", &_Dcs, &_DcsLabel, &_DcsPopup, font12, 65);
        CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
        _Dcs->setFocusable(false);
        _DcsLabel->setText("DCS Copy Protection:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        _DcsPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
        rectPopup = _DcsPopup->getGeometry();
        /* add buttons for supported DCS types */
        for (int i = 0; i < NEXUS_DisplayDcsType_eMax; i++)
        {
            /* add DCS type */
            pButton = _DcsPopup->addButton(dcsToString((NEXUS_DisplayDcsType)i), rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add DCS button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(i);
        }
        /*_DcsPopup->sort();*/
        _DcsPopup->select(NEXUS_DisplayDcsType_eOff);

        /* modal popup */
        {
            MRect    geom;
            uint32_t msgBoxWidth  = 350;
            uint32_t msgBoxHeight = 100;

            /* center msg box on screen - note that we will set the msg box bwin parent to be the framebuffer
             * so it can draw outside the borders of it's parent widget (this) */
            geom.setX(graphicsWidth / 2 - msgBoxWidth / 2);
            geom.setY(graphicsHeight / 2 - msgBoxHeight / 2);
            geom.setWidth(msgBoxWidth);
            geom.setHeight(msgBoxHeight);

            _MsgBox = new CWidgetModalMsgBox("CPanelVbi::_MsgBox", getEngine(), this, geom, font12);
            CHECK_PTR_ERROR_GOTO("unable to allocate MsgBox widget", _MsgBox, ret, eRet_OutOfMemory, error);
            _MsgBox->setText("Sample Text", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
            _MsgBox->show(false);
        }

        /* back button */
        _Back = new CWidgetButton("CPanelVbi::_Back", getEngine(), this, MRect(0, 0, 0, 0), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Back, ret, eRet_OutOfMemory, error);
        _Back->setText("Back");
        _pVbiMenu->addBackButton(_Back);

        _Back->setFocus();
    }

    goto done;
error:
    uninitialize();
done:
    return(ret);
} /* initialize */

void CPanelVbi::uninitialize()
{
    DEL(_Back);
    DEL(_MsgBox);
    DEL(_DcsPopup);
    DEL(_DcsLabel);
    DEL(_Dcs);
    DEL(_MacrovisionPopup);
    DEL(_MacrovisionLabel);
    DEL(_Macrovision);
    DEL(_NielsenAmolPopup);
    DEL(_NielsenAmolLabel);
    DEL(_NielsenAmol);
    DEL(_Gemstar);
    DEL(_Cgms);
    DEL(_WidescreenSignaling);
    DEL(_VideoProgramSys);
    DEL(_Teletext);
    DEL(_ClosedCaptionPassthru);
    DEL(_pVbiMenu);
} /* uninitialize */

eRet CPanelVbi::notifyObservers(
        eNotification notification,
        void *        data,
        bool          bHandleErrors
        )
{
    _bHandleErrors = bHandleErrors;
    return(CSubject::notifyObservers(notification, data));
} /* notifyObservers */

void CPanelVbi::onClick(bwidget_t widget)
{
    eRet            ret        = eRet_Ok;
    CWidgetBase *   pWidget    = NULL;
    CModel *        pModel     = getModel();
    CDisplay *      pDisplaySD = NULL;
    CDisplayVbiData settings;

    BDBG_ASSERT(NULL != pModel);

    pDisplaySD = pModel->getDisplay(1);
    CHECK_PTR_ERROR_GOTO("Unable to set VBI settings - no SD display", pDisplaySD, ret, eRet_NotAvailable, error);

    settings = pDisplaySD->getVbiSettings();

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }

    if (_ClosedCaptionPassthru == pWidget)
    {
        settings.bClosedCaptions = _ClosedCaptionPassthru->isChecked();
        notifyObservers(eNotify_SetVbiSettings, &settings, true);
    }
    else
    if (_Teletext == pWidget)
    {
        settings.bTeletext = _Teletext->isChecked();
        notifyObservers(eNotify_SetVbiSettings, &settings, true);
    }
    else
    if (_VideoProgramSys == pWidget)
    {
        settings.bVps = _VideoProgramSys->isChecked();
        notifyObservers(eNotify_SetVbiSettings, &settings, true);
    }
    else
    if (_WidescreenSignaling == pWidget)
    {
        settings.bWss = _WidescreenSignaling->isChecked();
        notifyObservers(eNotify_SetVbiSettings, &settings, true);
    }
    else
    if (_Cgms == pWidget)
    {
        settings.bCgms = _Cgms->isChecked();
        notifyObservers(eNotify_SetVbiSettings, &settings, true);
    }
    else
    if (_Gemstar == pWidget)
    {
        settings.bGemstar = _Gemstar->isChecked();
        notifyObservers(eNotify_SetVbiSettings, &settings);
    }
    else
    if (0 <= _NielsenAmolPopup->getItemListIndex(pWidget->getWidget()))
    {
        NEXUS_AmolType amol = (NEXUS_AmolType)pWidget->getValue();

        if (amol != settings.amolType)
        {
            settings.bAmol    = (NEXUS_AmolType_eMax == amol) ? false : true;
            settings.amolType = amol;
            notifyObservers(eNotify_SetVbiSettings, &settings, true);
        }
    }
    else
    if (0 <= _MacrovisionPopup->getItemListIndex(pWidget->getWidget()))
    {
        NEXUS_DisplayMacrovisionType macrovision = (NEXUS_DisplayMacrovisionType)pWidget->getValue();

        if (macrovision != settings.macrovisionType)
        {
            settings.macrovisionType = macrovision;
            notifyObservers(eNotify_SetVbiSettings, &settings, true);
        }
    }
    else
    if (0 <= _DcsPopup->getItemListIndex(pWidget->getWidget()))
    {
        NEXUS_DisplayDcsType dcs = (NEXUS_DisplayDcsType)pWidget->getValue();

        if (dcs != settings.dcsType)
        {
            settings.dcsType = dcs;
            notifyObservers(eNotify_SetVbiSettings, &settings, true);
        }
    }
    else
    if (_Back == pWidget)
    {
        getScreenMain()->showMenu(eMenu_Display);
    }

error:
    return;
} /* onClick */

void CPanelVbi::processNotification(CNotification & notification)
{
    switch (notification.getId())
    {
    case eNotify_VbiSettingsChanged:
    {
        CDisplay *      pDisplay = (CDisplay *)notification.getData();
        CDisplayVbiData settings = pDisplay->getVbiSettings();
        if (pDisplay->isStandardDef())
        {
            _ClosedCaptionPassthru->setCheck(settings.bClosedCaptions);
            _Teletext->setCheck(settings.bTeletext);
            _VideoProgramSys->setCheck(settings.bVps);
            _WidescreenSignaling->setCheck(settings.bWss);
            _Cgms->setCheck(settings.bCgms);
            _Gemstar->setCheck(settings.bGemstar);
            _NielsenAmolPopup->select(settings.amolType);
            _MacrovisionPopup->select(settings.macrovisionType);
            _DcsPopup->select(settings.dcsType);
        }

        _bHandleErrors = false;
    }
    break;

    case eNotify_ErrorVbi:
    {
        bwidget_t widget   = getFocus();
        eRet *    pError   = (eRet *)notification.getData();
        MString   strError = "Error changing VBI settings: ";

        if (true == _bHandleErrors)
        {
            strError += retToString(*pError);
            _MsgBox->showModal(strError, 0, "Ok", NULL);
            setFocus(widget);
        }
        _bHandleErrors = false;
    }
    break;

    case eNotify_ErrorVbiMacrovision:
    {
        bwidget_t widget   = getFocus();
        eRet *    pError   = (eRet *)notification.getData();
        MString   strError = "Error changing VBI Macrovision: ";

        if (true == _bHandleErrors)
        {
            strError += retToString(*pError);
            _MsgBox->showModal(strError, 0, "Ok", NULL);
            setFocus(widget);
        }
        _bHandleErrors = false;
    }
    break;

    case eNotify_ErrorVbiDcs:
    {
        bwidget_t widget   = getFocus();
        eRet *    pError   = (eRet *)notification.getData();
        MString   strError = "Error changing VBI DCS: ";

        if (true == _bHandleErrors)
        {
            strError += retToString(*pError);
            _MsgBox->showModal(strError, 0, "Ok", NULL);
            setFocus(widget);
        }
        _bHandleErrors = false;
    }
    break;

    default:
        break;
    } /* switch */

    return;
} /* processNotification */