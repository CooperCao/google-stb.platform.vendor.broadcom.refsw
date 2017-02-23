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

#include "panel_display.h"
#include "panel_vbi.h"
#include "screen_main.h"
#include "convert.h"

BDBG_MODULE(panel_display);

CPanelDisplay::CPanelDisplay(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelDisplay", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pDisplayMenu(NULL),
    _Format(NULL),
    _FormatLabel(NULL),
    _FormatPopup(NULL),
    _AutoFormat(NULL),
    _ContentMode(NULL),
    _ContentModeLabel(NULL),
    _ContentModePopup(NULL),
    _ColorSpace(NULL),
    _ColorSpaceLabel(NULL),
    _ColorSpacePopup(NULL),
    _ColorDepth(NULL),
    _ColorDepthLabel(NULL),
    _ColorDepthPopup(NULL),
    _Deinterlacer(NULL),
    _BoxDetect(NULL),
    _MpaaDecimation(NULL),
    _AspectRatio(NULL),
    _AspectRatioLabel(NULL),
    _AspectRatioPopup(NULL),
    _Vbi(NULL),
#ifdef DCC_SUPPORT
    _DigitalClosedCaption(NULL),
#endif
    _MsgBox(NULL),
    _Back(NULL),
    _lastVideoFormat(NEXUS_VideoFormat_eUnknown),
    _showFormatConfirmMsgBox(false),
    _timerMsgBox(pWidgetEngine, this, 10000)
{
}

CPanelDisplay::~CPanelDisplay()
{
    uninitialize();
}

eRet CPanelDisplay::initialize(
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
    int         menuWidth      = 266;
    int         menuHeight     = 310;
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

    _pDisplayMenu = new CWidgetMenu("CPanelDisplay::_pDisplayMenu", getEngine(), this, MRect(0, 0, menuWidth, menuHeight), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pDisplayMenu, ret, eRet_OutOfMemory, error);
    _pDisplayMenu->setMenuTitle("Display", "Settings");
    _pDisplayMenu->setScroll(false);
    _pDisplayMenu->show(true);
    {
        CBoardFeatures *     pFeatures = pConfig->getBoardFeatures();
        CWidgetCheckButton * pButton   = NULL;
        MRect                rectPopup;

        /* Format */
        ret = _pDisplayMenu->addLabelPopupButton(this, "Format", &_Format, &_FormatLabel, &_FormatPopup, font12, 25);
        CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
        _Format->setFocusable(false);
        _FormatLabel->setText("Format:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        _FormatPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
        rectPopup = _FormatPopup->getGeometry();
        /* add buttons for supported formats - hdmi hotplug may trigger additional formats to be added */
        _FormatPopup->clearButtons();
        ret = generateVideoFormatButtons();
        CHECK_ERROR_GOTO("unable to add video format button(s) after hdmi hotplug event", ret, error);
        _FormatPopup->sort();
        _FormatPopup->select(_lastVideoFormat);

        /* AUTO FORMAT */
        _AutoFormat = new CWidgetCheckButton("CPanelDisplay::_AutoFormat", getEngine(), this, MRect(0, 0, 0, 22), font12, _pDisplayMenu->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _AutoFormat, ret, eRet_OutOfMemory, error);
        _pDisplayMenu->addButton(_AutoFormat, "Auto Format");
        _AutoFormat->setText("Auto Format:", bwidget_justify_horiz_left);
        _AutoFormat->setCheck(false);

        /* Content Mode */
        ret = _pDisplayMenu->addLabelPopupButton(this, "ContentMode", &_ContentMode, &_ContentModeLabel, &_ContentModePopup, font12, 40);
        CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
        _ContentMode->setFocusable(false);
        _ContentModeLabel->setText("Content Mode:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        _ContentModePopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
        rectPopup = _ContentModePopup->getGeometry();
        /* add buttons for content modes */
        for (int i = 0; i < NEXUS_VideoWindowContentMode_eMax; i++)
        {
            pButton = _ContentModePopup->addButton(videoContentModeToString((NEXUS_VideoWindowContentMode)i), rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add video content mode button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(i);
        }
        _ContentModePopup->sort();
        _ContentModePopup->select(NEXUS_VideoWindowContentMode_eBox);

        /* COLOR SPACE */
        ret = _pDisplayMenu->addLabelPopupButton(this, "Colorspace", &_ColorSpace, &_ColorSpaceLabel, &_ColorSpacePopup, font12);
        CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
        _ColorSpace->setFocusable(false);
        _ColorSpaceLabel->setText("Colorspace:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        _ColorSpacePopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
        rectPopup = _ColorSpacePopup->getGeometry();

        pButton = _ColorSpacePopup->addButton("Auto", rectPopup.width(), rectPopup.height());
        CHECK_PTR_ERROR_GOTO("unable to add color space Auto button to popup list", pButton, ret, eRet_OutOfMemory, error);
        pButton->setValue(NEXUS_ColorSpace_eAuto);
        pButton = _ColorSpacePopup->addButton("RGB", rectPopup.width(), rectPopup.height());
        CHECK_PTR_ERROR_GOTO("unable to add color space RGB button to popup list", pButton, ret, eRet_OutOfMemory, error);
        pButton->setValue(NEXUS_ColorSpace_eRgb);
        pButton = _ColorSpacePopup->addButton("YPrPb422", rectPopup.width(), rectPopup.height());
        CHECK_PTR_ERROR_GOTO("unable to add color space YPrPb422 button to popup list", pButton, ret, eRet_OutOfMemory, error);
        pButton->setValue(NEXUS_ColorSpace_eYCbCr422);
        pButton = _ColorSpacePopup->addButton("YPrPb444", rectPopup.width(), rectPopup.height());
        CHECK_PTR_ERROR_GOTO("unable to add color space YPrPb444 button to popup list", pButton, ret, eRet_OutOfMemory, error);
        pButton->setValue(NEXUS_ColorSpace_eYCbCr444);
        pButton = _ColorSpacePopup->addButton("YPrPb420", rectPopup.width(), rectPopup.height());
        CHECK_PTR_ERROR_GOTO("unable to add color space YPrPb420 button to popup list", pButton, ret, eRet_OutOfMemory, error);
        pButton->setValue(NEXUS_ColorSpace_eYCbCr420);

        /* COLOR DEPTH */
        ret = _pDisplayMenu->addLabelPopupButton(this, "ColorDepth", &_ColorDepth, &_ColorDepthLabel, &_ColorDepthPopup, font12);
        CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
        _ColorDepth->setFocusable(false);
        _ColorDepthLabel->setText("ColorDepth:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        _ColorDepthPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
        rectPopup = _ColorDepthPopup->getGeometry();

        pButton = _ColorDepthPopup->addButton("8 bit", rectPopup.width(), rectPopup.height());
        CHECK_PTR_ERROR_GOTO("unable to add color depth 8bit button to popup list", pButton, ret, eRet_OutOfMemory, error);
        pButton->setValue(8);
        pButton = _ColorDepthPopup->addButton("10 bit", rectPopup.width(), rectPopup.height());
        CHECK_PTR_ERROR_GOTO("unable to add color depth 10bit button to popup list", pButton, ret, eRet_OutOfMemory, error);
        pButton->setValue(10);
        pButton = _ColorDepthPopup->addButton("12 bit", rectPopup.width(), rectPopup.height());
        CHECK_PTR_ERROR_GOTO("unable to add color depth 12bit button to popup list", pButton, ret, eRet_OutOfMemory, error);
        pButton->setValue(12);
        _ColorDepthPopup->select(GET_INT(_pCfg, DECODER_COLOR_DEPTH));

        /* DEINTERLACER */
        _Deinterlacer = new CWidgetCheckButton("CPanelDisplay::_Deinterlacer", getEngine(), this, MRect(0, 0, 0, 22), font12, _pDisplayMenu->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Deinterlacer, ret, eRet_OutOfMemory, error);
        _pDisplayMenu->addButton(_Deinterlacer, "Deinterlacer");
        _Deinterlacer->setText("Deinterlacer:", bwidget_justify_horiz_left);
        _Deinterlacer->setCheck(true); /* Nexus defaults it on (if capable).  See NEXUS_VideoWindow_P_InitState().*/

        /* BOX DETECT */
        if (0 < pFeatures->_boxDetect)
        {
            _BoxDetect = new CWidgetCheckButton("CPanelDisplay::_BoxDetect", getEngine(), this, MRect(0, 0, 0, 22), font12, _pDisplayMenu->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _BoxDetect, ret, eRet_OutOfMemory, error);
            _pDisplayMenu->addButton(_BoxDetect, "Box Detect");
            _BoxDetect->setText("Box Detect:", bwidget_justify_horiz_left);
            _BoxDetect->setCheck(false);
        }
        else
        {
            /* letterbox detection is not supported so adjust menu height */
            menuHeight -= 23;
        }

        /* MPAA DECIMATION */
        _MpaaDecimation = new CWidgetCheckButton("CPanelDisplay::_MpaaDecimation", getEngine(), this, MRect(0, 0, 0, 22), font12, _pDisplayMenu->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _MpaaDecimation, ret, eRet_OutOfMemory, error);
        _pDisplayMenu->addButton(_MpaaDecimation, "MPAA Decimation");
        _MpaaDecimation->setText("MPAA Decimation:", bwidget_justify_horiz_left);
        _MpaaDecimation->setCheck(false);

        /* ASPECT RATIO */
        ret = _pDisplayMenu->addLabelPopupButton(this, "Aspect Ratio", &_AspectRatio, &_AspectRatioLabel, &_AspectRatioPopup, font12, 60);
        CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
        _AspectRatio->setFocusable(false);
        _AspectRatioLabel->setText("Aspect Ratio:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        _AspectRatioPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
        rectPopup = _AspectRatioPopup->getGeometry();
        pButton   = _AspectRatioPopup->addButton("Auto", rectPopup.width(), rectPopup.height());
        CHECK_PTR_ERROR_GOTO("unable to add Auto HD aspect ratio button to popup list", pButton, ret, eRet_OutOfMemory, error);
        pButton->setValue(NEXUS_DisplayAspectRatio_eAuto);
        pButton = _AspectRatioPopup->addButton("16x9", rectPopup.width(), rectPopup.height());
        CHECK_PTR_ERROR_GOTO("unable to add 16x9 Auto HD aspect ratio button to popup list", pButton, ret, eRet_OutOfMemory, error);
        pButton->setValue(NEXUS_DisplayAspectRatio_e16x9);
        pButton = _AspectRatioPopup->addButton("4x3", rectPopup.width(), rectPopup.height());
        CHECK_PTR_ERROR_GOTO("unable to add 4x3 Auto HD aspect ratio button to popup list", pButton, ret, eRet_OutOfMemory, error);
        pButton->setValue(NEXUS_DisplayAspectRatio_e4x3);

        /* VBI */
        {
            CDisplay * pDisplay   = pModel->getDisplay(0);
            bool       bDisplaySD = false;

            for (int i = 0; pDisplay; i++)
            {
                pDisplay = pModel->getDisplay(i);
                if (NULL != pDisplay)
                {
                    bDisplaySD |= pDisplay->isStandardDef();
                }
            }

            if (true == bDisplaySD)
            {
                /* add button only if an SD Display exists */
                _Vbi = new CWidgetButton("CPanelDisplay::_Vbi", getEngine(), this, MRect(0, 0, 0, 22), font12);
                CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _Vbi, ret, eRet_OutOfMemory, error);
                _Vbi->setText("VBI...", bwidget_justify_horiz_left);
                _pDisplayMenu->addButton(_Vbi, "VBI");
            }
            else
            {
                /* VBI is not supported so adjust menu height */
                menuHeight -= 23;
            }
        }

#ifdef DCC_SUPPORT
        /* DIGITAL CLOSED CAPTION */
        _DigitalClosedCaption = new CWidgetCheckButton("CPanelDisplay::_DigitalClosedCaption", getEngine(), this, MRect(0, 0, 0, 22), font12, _pDisplayMenu->getWin());
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _DigitalClosedCaption, ret, eRet_OutOfMemory, error);
        _pDisplayMenu->addButton(_DigitalClosedCaption, "Digital Closed Caption");
        _DigitalClosedCaption->setText("Digital Closed Caption:", bwidget_justify_horiz_left);
        _DigitalClosedCaption->setCheck(false);
#else /* ifdef DCC_SUPPORT */
        /* DCC not supported so adjust menu height */
        menuHeight -= 23;
#endif /* ifdef DCC_SUPPORT */
    }

    /* back button */
    _Back = new CWidgetButton("CPanelDisplay::_Back", getEngine(), this, MRect(0, 0, 0, 0), font10);
    CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Back, ret, eRet_OutOfMemory, error);
    _Back->setText("Menu");
    _pDisplayMenu->addBackButton(_Back);

    {
        MRect    geom;
        uint32_t msgBoxWidth  = 340;
        uint32_t msgBoxHeight = 100;

        /* center msg box on screen - note that we will set the msg box bwin parent to be the framebuffer
         * so it can draw outside the borders of it's parent widget (this) */
        geom.setX(graphicsWidth / 2 - msgBoxWidth / 2);
        geom.setY(graphicsHeight / 2 - msgBoxHeight / 2);
        geom.setWidth(msgBoxWidth);
        geom.setHeight(msgBoxHeight);

        _MsgBox = new CWidgetModalMsgBox("CPanelDisplay::_MsgBox", getEngine(), this, geom, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate MsgBox widget", _MsgBox, ret, eRet_OutOfMemory, error);
        _MsgBox->setText("Sample Text", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
        _MsgBox->show(false);
    }

    rectPanel.set(50, 50, menuWidth, menuHeight);
    setGeometry(rectPanel);

    _Back->setFocus();

    goto done;
error:
    uninitialize();
done:
    return(ret);
} /* initialize */

void CPanelDisplay::uninitialize()
{
    DEL(_MsgBox);
    DEL(_Back);
#ifdef DCC_SUPPORT
    DEL(_DigitalClosedCaption);
#endif
    DEL(_Vbi);
    DEL(_AspectRatioPopup);
    DEL(_AspectRatioLabel);
    DEL(_AspectRatio);
    DEL(_MpaaDecimation);
    DEL(_BoxDetect);
    DEL(_Deinterlacer);
    DEL(_ColorDepthPopup);
    DEL(_ColorDepthLabel);
    DEL(_ColorDepth);
    DEL(_ColorSpacePopup);
    DEL(_ColorSpaceLabel);
    DEL(_ColorSpace);
    DEL(_ContentModePopup);
    DEL(_ContentModeLabel);
    DEL(_ContentMode);
    DEL(_AutoFormat);
    DEL(_FormatPopup);
    DEL(_FormatLabel);
    DEL(_Format);
    DEL(_pDisplayMenu);
} /* uninitialize */

void CPanelDisplay::onClick(bwidget_t widget)
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

    if (0 <= _FormatPopup->getItemListIndex(pWidget->getWidget()))
    {
        CWidgetCheckButton * pButton     = (CWidgetCheckButton *)pWidget;
        NEXUS_VideoFormat    videoFormat = (NEXUS_VideoFormat)pWidget->getValue();
        CDisplay *           pDisplay    = pModel->getDisplay();

        BDBG_ASSERT(NULL != pDisplay);

        BDBG_WRN(("selected video format:%s", pButton->getText().s()));
        if (videoFormat != pDisplay->getFormat())
        {
            _showFormatConfirmMsgBox = true;
            notifyObservers(eNotify_SetVideoFormat, &videoFormat);
        }
    }
    else
    if (_AutoFormat == pWidget)
    {
        bool bAutoFormat = _AutoFormat->isChecked();

        notifyObservers(eNotify_SetAutoVideoFormat, &bAutoFormat);

        if (true == bAutoFormat)
        {
            _MsgBox->showModal("Auto format takes effect on channel change", 0, "Ok", NULL, NULL);
            _AutoFormat->setFocus();
        }
    }
    else
    if (0 <= _ContentModePopup->getItemListIndex(pWidget->getWidget()))
    {
        CWidgetCheckButton *         pButton          = (CWidgetCheckButton *)pWidget;
        NEXUS_VideoWindowContentMode videoContentMode = (NEXUS_VideoWindowContentMode)pWidget->getValue();
        CDisplay *                   pDisplay         = pModel->getDisplay();

        if ((NULL != pDisplay) && (videoContentMode != pDisplay->getContentMode()))
        {
            BDBG_WRN(("selected video ContentMode:%s", pButton->getText().s()));
            notifyObservers(eNotify_SetContentMode, &videoContentMode);
        }
    }
    else
    if (0 <= _ColorSpacePopup->getItemListIndex(pWidget->getWidget()))
    {
        NEXUS_ColorSpace colorSpace = (NEXUS_ColorSpace)pWidget->getValue();

        notifyObservers(eNotify_SetColorSpace, &colorSpace);
    }
    else
    if (0 <= _ColorDepthPopup->getItemListIndex(pWidget->getWidget()))
    {
        uint8_t colorDepth = (uint8_t)pWidget->getValue();

        notifyObservers(eNotify_SetColorDepth, &colorDepth);
    }
    else
    if (_MpaaDecimation == pWidget)
    {
        bool bMpaaDecimation = _MpaaDecimation->isChecked();

        notifyObservers(eNotify_SetMpaaDecimation, &bMpaaDecimation);
    }
    else
    if (_Deinterlacer == pWidget)
    {
        bool bDeinterlacer = _Deinterlacer->isChecked();

        notifyObservers(eNotify_SetDeinterlacer, &bDeinterlacer);
    }
    else
    if (_BoxDetect == pWidget)
    {
        bool bBoxDetect = _BoxDetect->isChecked();

        notifyObservers(eNotify_SetBoxDetect, &bBoxDetect);
    }
    else
    if (0 <= _AspectRatioPopup->getItemListIndex(pWidget->getWidget()))
    {
        NEXUS_DisplayAspectRatio aspectRatio = (NEXUS_DisplayAspectRatio)pWidget->getValue();

        notifyObservers(eNotify_SetAspectRatio, &aspectRatio);
    }
    else
    if (_Vbi == pWidget)
    {
        getScreenMain()->showMenu(eMenu_Vbi);
    }
    else
#ifdef DCC_SUPPORT
    if (_DigitalClosedCaption == pWidget)
    {
        bool bDigitalClosedCaption = _DigitalClosedCaption->isChecked();

        notifyObservers(eNotify_ClosedCaptionEnable, &bDigitalClosedCaption);
    }
    else
#endif /* ifdef DCC_SUPPORT */
    if (_Back == pWidget)
    {
        show(false);
        getParent()->show(true);
    }

    return;
} /* onClick */

eRet CPanelDisplay::generateVideoFormatButtons(COutputHdmi * pOutputHdmi)
{
    eRet                 ret       = eRet_Ok;
    CWidgetCheckButton * pButton   = NULL;
    CBoardFeatures *     pFeatures = getConfig()->getBoardFeatures();
    MRect                rectPopup = _FormatPopup->getGeometry();

    BDBG_ASSERT(NULL != pFeatures);

    for (int i = 0; i < NEXUS_VideoFormat_eMax; i++)
    {
        /* check if platform/display supports video format */
        if (true == pFeatures->_videoFormatIsSupported[(NEXUS_VideoFormat)i])
        {
            /* check if hdmi supports video format */
            if ((NULL != pOutputHdmi) && (true == pOutputHdmi->isConnected()))
            {
                /* yes hdmi */

                if (true == pOutputHdmi->isValidVideoFormat((NEXUS_VideoFormat)i))
                {
                    BDBG_MSG(("adding hdmi output format:%s", videoFormatToString((NEXUS_VideoFormat)i).s()));
                    /* video format supported - add button */
                    pButton = _FormatPopup->addButton(videoFormatToString((NEXUS_VideoFormat)i), rectPopup.width(), rectPopup.height());
                    CHECK_PTR_ERROR("unable to add video format button to popup list", pButton, ret, eRet_OutOfMemory);
                    pButton->setValue(i);
                }
            }
            else
            {
                MString  strVideoFormat      = videoFormatToString((NEXUS_VideoFormat)i);
                uint16_t nVideoFormatVertRes = videoFormatToVertRes((NEXUS_VideoFormat)i).toInt();

                /* no hdmi */

                if (1080 == nVideoFormatVertRes)
                {
                    if ((NEXUS_VideoFormat_e1080i50hz != (NEXUS_VideoFormat)i) &&
                        (NEXUS_VideoFormat_e1080i != (NEXUS_VideoFormat)i))
                    {
                        /* 1080p formats only supported if hdmi is available */
                        continue;
                    }
                }
                else
                if ((1080 < nVideoFormatVertRes) || (0 == nVideoFormatVertRes))
                {
                    /* formats greater than 1080i/p are not supported without hdmi
                     * also ignore unknown formats */
                    continue;
                }
                else
                if (-1 < strVideoFormat.find(" 3D"))
                {
                    /* 3D video format not supported without hdmi */
                    continue;
                }

                BDBG_MSG(("adding platform/display format:%s", strVideoFormat.s()));

                /* video format supported - add button */
                pButton = _FormatPopup->addButton(strVideoFormat, rectPopup.width(), rectPopup.height());
                CHECK_PTR_ERROR("unable to add video format button to popup list", pButton, ret, eRet_OutOfMemory);
                pButton->setValue(i);
            }
        }
    }

    return(ret);
} /* generateVideoFormatButtons */

void CPanelDisplay::processNotification(CNotification & notification)
{
    BDBG_MSG(("CPanelDisplay::processNotification():%d", notification.getId()));

    switch (notification.getId())
    {
    case eNotify_Timeout:
    {
        CTimer * pTimer = (CTimer *)notification.getData();
        if (&_timerMsgBox == pTimer)
        {
            /* _MsgBox did not get a response from user so we'll timeout
             * here and select the "cancel" option */
            _MsgBox->cancelModal("Cancel");
        }
    }
    break;

    case eNotify_HdmiHotplugEvent:
    {
        eRet          ret         = eRet_Ok;
        COutputHdmi * pOutputHdmi = (COutputHdmi *)notification.getData();

        _FormatPopup->popup(false);
        _FormatPopup->clearButtons();

        ret = generateVideoFormatButtons(pOutputHdmi);
        CHECK_ERROR("unable to add video format button(s) after hdmi hotplug event", ret);

        _FormatPopup->sort();
        _FormatPopup->select(_lastVideoFormat);
    }
    break;

    case eNotify_VideoFormatChanged:
    {
        CDisplay *          pDisplay     = getModel()->getDisplay();
        NEXUS_VideoFormat * pVideoFormat = (NEXUS_VideoFormat *)notification.getData();

        BDBG_ASSERT(NULL != pDisplay);

        _FormatPopup->select(*pVideoFormat);

        if (true == _showFormatConfirmMsgBox)
        {
            char str[64];
            snprintf(str, 64, "Keep new %s video format?", videoFormatToString(*pVideoFormat).s());

            _showFormatConfirmMsgBox = false;
            _timerMsgBox.start(10000);
            if (_MsgBox->showModal(str, 1) != "Ok")
            {
                _timerMsgBox.stop();

                /* revert to old video format */
                notifyObservers(eNotify_SetVideoFormat, &_lastVideoFormat);
            }

            _FormatPopup->setFocus();
        }

        /* save current video format */
        _lastVideoFormat = pDisplay->getFormat();
    }
    break;

    case eNotify_AutoVideoFormatChanged:
    {
        bool * pAutoVideoFormat = (bool *)notification.getData();

        _AutoFormat->setCheck(*pAutoVideoFormat);
    }
    break;

    case eNotify_ContentModeChanged:
    {
        NEXUS_VideoWindowContentMode * pContentMode = (NEXUS_VideoWindowContentMode *)notification.getData();
        _ContentModePopup->select(*pContentMode);
    }
    break;

    case eNotify_ColorSpaceChanged:
    case eNotify_ColorSpaceFailure:
    {
        NEXUS_ColorSpace * pColorSpace = (NEXUS_ColorSpace *)notification.getData();

        _ColorSpacePopup->select(*pColorSpace);
    }
    break;

    case eNotify_ColorDepthChanged:
    case eNotify_ColorDepthFailure:
    {
        uint8_t * pColorDepth = (uint8_t *)notification.getData();
        _ColorDepthPopup->select(*pColorDepth);
    }
    break;

    case eNotify_MpaaDecimationChanged:
    {
        bool * pMpaaDecimation = (bool *)notification.getData();
        _MpaaDecimation->setCheck(*pMpaaDecimation);
    }
    break;

    case eNotify_DeinterlacerChanged:
    {
        bool * pDeinterlacer = (bool *)notification.getData();
        _Deinterlacer->setCheck(*pDeinterlacer);
    }
    break;

    case eNotify_BoxDetectChanged:
    {
        bool * pBoxDetect = (bool *)notification.getData();
        _BoxDetect->setCheck(*pBoxDetect);
    }
    break;

    case eNotify_AspectRatioChanged:
    {
        NEXUS_DisplayAspectRatio * pAspectRatio = (NEXUS_DisplayAspectRatio *)notification.getData();
        _AspectRatioPopup->select(*pAspectRatio);
    }
    break;

#ifdef DCC_SUPPORT
    case eNotify_DigitalClosedCaptionChanged:
    {
        bool * pDigitalClosedCaption = (bool *)notification.getData();
        _DigitalClosedCaption->setCheck(*pDigitalClosedCaption);
    }
    break;
#endif /* ifdef DCC_SUPPORT */

    default:
        break;
    } /* switch */

    return;
} /* processNotification */