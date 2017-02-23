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

#if NEXUS_HAS_FRONTEND

#include "convert.h"
#include "tuner_vsb.h"
#include "panel_scan_vsb.h"

BDBG_MODULE(panel_scan_vsb);

CPanelScanVsb::CPanelScanVsb(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelScanVsb", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pScanVsbMenu(NULL),
    _Settings(NULL),
    _StartFreq(NULL),
    _StartFreqLabel(NULL),
    _StartFreqEdit(NULL),
    _StartFreqUnits(NULL),
    _EndFreq(NULL),
    _EndFreqLabel(NULL),
    _EndFreqEdit(NULL),
    _EndFreqUnits(NULL),
    _StepFreq(NULL),
    _StepFreqLabel(NULL),
    _StepFreqEdit(NULL),
    _StepFreqUnits(NULL),
    _BandwidthFreq(NULL),
    _BandwidthFreqLabel(NULL),
    _BandwidthFreqEdit(NULL),
    _BandwidthFreqUnits(NULL),
    _Append(NULL),
    _Mode(NULL),
    _ModeLabel(NULL),
    _ModePopup(NULL),
    _Mode8(NULL),
    _Mode16(NULL),
    _Scan(NULL),
    _Back(NULL)
{
}

CPanelScanVsb::~CPanelScanVsb()
{
    uninitialize();
}

eRet CPanelScanVsb::initialize(
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
    int         menuHeight     = 226;
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

    _pScanVsbMenu = new CWidgetMenu("CPanelScanVsb::_pScanVsbMenu", getEngine(), this, MRect(0, 0, menuWidth, menuHeight), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pScanVsbMenu, ret, eRet_OutOfMemory, error);
    _pScanVsbMenu->showSubTitle(false);
    _pScanVsbMenu->showListView(false);
    _pScanVsbMenu->showEdit(false);
    _pScanVsbMenu->setMenuTitle("Scan Vsb", NULL);
    _pScanVsbMenu->show(true);
    {
        _Settings = new CWidgetMenu("CPanelScanVsb::_Settings", getEngine(), _pScanVsbMenu, MRect(0, 30, menuWidth, 166), font14, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _Settings, ret, eRet_OutOfMemory, error);
        _Settings->showMenuBar(false);
        _Settings->setMenuTitle(NULL, "Settings");
        _Settings->setScroll(true);
        {
            ret = _Settings->addDualLabelEditButton(this, "Start Freq", &_StartFreq, &_StartFreqLabel, &_StartFreqEdit, &_StartFreqUnits, font12);
            CHECK_ERROR_GOTO("unable to allocate dual label edit button", ret, error);
            _StartFreq->setFocusable(false);
            _StartFreqLabel->setText("Start Freq:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _StartFreqEdit->setText("497000000", bwidget_justify_horiz_right);
            _StartFreqUnits->setText("Hz", bwidget_justify_horiz_left, bwidget_justify_vert_middle);

            ret = _Settings->addDualLabelEditButton(this, "End Freq", &_EndFreq, &_EndFreqLabel, &_EndFreqEdit, &_EndFreqUnits, font12);
            CHECK_ERROR_GOTO("unable to allocate dual label edit button", ret, error);
            _EndFreq->setFocusable(false);
            _EndFreqLabel->setText("End Freq:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _EndFreqEdit->setText("629000000", bwidget_justify_horiz_right);
            _EndFreqUnits->setText("Hz", bwidget_justify_horiz_left, bwidget_justify_vert_middle);

            ret = _Settings->addDualLabelEditButton(this, "Bandwidth", &_BandwidthFreq, &_BandwidthFreqLabel, &_BandwidthFreqEdit, &_BandwidthFreqUnits, font12);
            CHECK_ERROR_GOTO("unable to allocate dual label edit button", ret, error);
            _BandwidthFreq->setFocusable(false);
            _BandwidthFreqLabel->setText("Bandwidth:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _BandwidthFreqEdit->setText("1000000", bwidget_justify_horiz_right);
            _BandwidthFreqUnits->setText("Hz", bwidget_justify_horiz_left, bwidget_justify_vert_middle);

            ret = _Settings->addDualLabelEditButton(this, "Step Freq", &_StepFreq, &_StepFreqLabel, &_StepFreqEdit, &_StepFreqUnits, font12);
            CHECK_ERROR_GOTO("unable to allocate dual label edit button", ret, error);
            _StepFreq->setFocusable(false);
            _StepFreqLabel->setText("Step Freq:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _StepFreqEdit->setText("1000000", bwidget_justify_horiz_right);
            _StepFreqUnits->setText("Hz", bwidget_justify_horiz_left, bwidget_justify_vert_middle);

            ret = _Settings->addLabelPopupButton(this, "Mode", &_Mode, &_ModeLabel, &_ModePopup, font12);
            CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
            _Mode->setFocusable(false);
            _ModeLabel->setText("Mode:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            {
                MRect rectSettings = _Settings->getGeometry();
                MRect rect         = _ModePopup->getGeometry();

                _Mode8 = _ModePopup->addButton("8", rect.width(), rect.height());
                _Mode8->setValue(NEXUS_FrontendVsbMode_e8);
                _Mode16 = _ModePopup->addButton("16", rect.width(), rect.height());
                _Mode16->setValue(NEXUS_FrontendVsbMode_e16);

                _ModePopup->select(_Mode8);
            }

            _Append = new CWidgetCheckButton("CPanelScanVsb::_Append", getEngine(), this, MRect(0, 0, 0, 22), font12, _pScanVsbMenu->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Append, ret, eRet_OutOfMemory, error);
            _Append->setText("Append to Channel List", bwidget_justify_horiz_left);
            _Append->setCheck(true);
            _Settings->addButton(_Append, "Append to Channel List");
        }

        {
            MRect rectMenu;
            MRect rectStart;

            rectMenu = _Settings->getGeometry();

            rectStart.setWidth(80);
            rectStart.setHeight(20);
            rectStart.setX(menuWidth - rectStart.width() - 7);
            rectStart.setY(rectMenu.y() + rectMenu.height() + 1);
            _Scan = new CWidgetButton("CPanelScanVsb::_Scan", getEngine(), this, rectStart, font12, _pScanVsbMenu->getWin());
            _Scan->setBevel(0);
            _Scan->setBackgroundFillMode(fill_eGradient);
            _Scan->setBackgroundGradient(COLOR_GREEN + 2 * COLOR_STEP, COLOR_GREEN, COLOR_GREEN - 4 * COLOR_STEP);
            _Scan->setTextColor(0xFF222222);
            _Scan->setText("SCAN");
        }

        /* back button */
        _Back = new CWidgetButton("CPanelScanVsb::_Back", getEngine(), this, MRect(0, 0, 0, 0), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Back, ret, eRet_OutOfMemory, error);
        _Back->setText("Menu");
        _pScanVsbMenu->addBackButton(_Back);

        _Back->setFocus();
    }

    goto done;
error:
    uninitialize();
done:
    return(ret);
} /* initialize */

void CPanelScanVsb::uninitialize()
{
    DEL(_Back);
    DEL(_Scan);
    DEL(_Append);
    DEL(_ModePopup);
    DEL(_ModeLabel);
    DEL(_Mode);
    DEL(_StepFreqUnits);
    DEL(_StepFreqEdit);
    DEL(_StepFreqLabel);
    DEL(_StepFreq);
    DEL(_BandwidthFreqUnits);
    DEL(_BandwidthFreqEdit);
    DEL(_BandwidthFreqLabel);
    DEL(_BandwidthFreq);
    DEL(_EndFreqUnits);
    DEL(_EndFreqEdit);
    DEL(_EndFreqLabel);
    DEL(_EndFreq);
    DEL(_StartFreqUnits);
    DEL(_StartFreqEdit);
    DEL(_StartFreqLabel);
    DEL(_StartFreq);
    DEL(_Settings);
    DEL(_pScanVsbMenu);
} /* uninitialize */

void CPanelScanVsb::onClick(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }

    if (_Back == pWidget)
    {
        show(false);
        getParent()->show(true);
    }
    else
    if (_Scan == pWidget)
    {
        CTunerVsbScanData scanData  = NULL;
        uint32_t          bandwidth = 0;
        uint32_t          startFreq = 0;
        uint32_t          endFreq   = 0;
        uint32_t          stepFreq  = 0;

        startFreq = (uint32_t)_StartFreqEdit->getText().toLong();
        endFreq   = (uint32_t)_EndFreqEdit->getText().toLong();
        stepFreq  = (uint32_t)_StepFreqEdit->getText().toLong();
        bandwidth = (uint32_t)_BandwidthFreqEdit->getText().toLong();

        /* create list of freq to scan */
        for (uint32_t freq = startFreq; freq <= endFreq; freq += stepFreq)
        {
            scanData._freqList.add(new uint32_t(freq));
        }

        scanData._bandwidth           = bandwidth;
        scanData._appendToChannelList = _Append->isChecked();
        scanData._mode                = _ModePopup->getSelection()->getValue();

        scanData.dump();

        /* offer to save scan results after scan completes */
        getModel()->setScanSaveOffer(true);

        notifyObservers(eNotify_ScanStart, &scanData);
        show(false);
    }

    return;
} /* onClick */

void CPanelScanVsb::processNotification(CNotification & notification)
{
    BDBG_MSG(("CPanelScanVsb::processNotificationScanVsbMenu:%d", notification.getId()));
}

#endif /* NEXUS_HAS_FRONTEND */