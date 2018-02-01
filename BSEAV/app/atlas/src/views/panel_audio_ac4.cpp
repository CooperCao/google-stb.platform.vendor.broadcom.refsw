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

#include "convert.h"
#include "panel_audio_ac4.h"
#include "screen_main.h"
#include "channelmgr.h"
#include "channel.h"
#include "audio_decode.h"

BDBG_MODULE(panel_audio_ac4);

CPanelAudioAc4::CPanelAudioAc4(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelAudioAc4", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pAudioMenu(NULL),
    _MainSettings(NULL),
    _MainLanguage(NULL),
    _MainLanguageLabel(NULL),
    _MainLanguagePopup(NULL),
    _MainAssociate(NULL),
    _MainAssociateLabel(NULL),
    _MainAssociatePopup(NULL),
    _MainPresentation(NULL),
    _MainPresentationLabel(NULL),
    _MainPresentationPopup(NULL),
#if ENABLE_ALTERNATE
    _AltSettings(NULL),
    _AltLanguage(NULL),
    _AltLanguageLabel(NULL),
    _AltLanguagePopup(NULL),
    _AltAssociate(NULL),
    _AltAssociateLabel(NULL),
    _AltAssociatePopup(NULL),
    _AltPresentation(NULL),
    _AltPresentationLabel(NULL),
    _AltPresentationPopup(NULL),
    _AltPriority(NULL),
#endif
    _Back(NULL)
{
}

CPanelAudioAc4::~CPanelAudioAc4()
{
    uninitialize();
}

eRet CPanelAudioAc4::initialize(
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
    int         menuWidth      = 260;
#if ENABLE_ALTERNATE
    int         menuHeight     = 274;
#else
    int         menuHeight     = 154;
#endif
    int         subMenuHeight  = 124;
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

    _pAudioMenu = new CWidgetMenu("CPanelAudioAc4::_pAudioMenu", getEngine(), this, MRect(0, 0, menuWidth, menuHeight), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pAudioMenu, ret, eRet_OutOfMemory, error);
    _pAudioMenu->showSubTitle(false);
    _pAudioMenu->showListView(false);
    _pAudioMenu->showEdit(false);
    _pAudioMenu->setMenuTitle("AC-4 Audio", NULL, bwidget_justify_horiz_center, bwidget_justify_vert_middle);
    _pAudioMenu->show(true);

    /* main program settings */
    {
        _MainSettings = new CWidgetMenu("CPanelAudioAc4::_MainSettings", getEngine(), _pAudioMenu, MRect(0, 30, menuWidth, subMenuHeight - 31), font14, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _MainSettings, ret, eRet_OutOfMemory, error);
        _MainSettings->showMenuBar(false);
        _MainSettings->setMenuTitle(NULL, "Main");
        _MainSettings->setScroll(true);
        {
            CBoardFeatures *     pFeatures = pConfig->getBoardFeatures();
            CWidgetCheckButton * pButton   = NULL;
            MRect                rectPopup;

            /* Language */
            ret = _MainSettings->addLabelPopupButton(this, "Ac4LanguageMain", &_MainLanguage, &_MainLanguageLabel, &_MainLanguagePopup, font12, 40);
            CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
            _MainLanguage->setFocusable(false);
            _MainLanguageLabel->setText("Language:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _MainLanguagePopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);

            rectPopup = _MainLanguagePopup->getGeometry();

            pButton = _MainLanguagePopup->addButton("English", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add langugage button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eLanguage_English);

            pButton = _MainLanguagePopup->addButton("French", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add langugage button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eLanguage_French);

            pButton = _MainLanguagePopup->addButton("German", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add langugage button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eLanguage_German);

            pButton = _MainLanguagePopup->addButton("Italian", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add langugage button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eLanguage_Italian);

            pButton = _MainLanguagePopup->addButton("Spanish", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add langugage button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eLanguage_Spanish);

            /* Associate Type */
            ret = _MainSettings->addLabelPopupButton(this, "Ac4AssociateMain", &_MainAssociate, &_MainAssociateLabel, &_MainAssociatePopup, font12, 40);
            CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
            _MainAssociate->setFocusable(false);
            _MainAssociateLabel->setText("Associate:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _MainAssociatePopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);

            rectPopup = _MainAssociatePopup->getGeometry();

            pButton = _MainAssociatePopup->addButton("Not Specified", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add associate button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(NEXUS_AudioAc4AssociateType_eNotSpecified);

            pButton = _MainAssociatePopup->addButton("Visually Impaired", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add associate button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(NEXUS_AudioAc4AssociateType_eVisuallyImpaired);

            pButton = _MainAssociatePopup->addButton("Hearing Impaired", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add associate button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(NEXUS_AudioAc4AssociateType_eHearingImpaired);

            pButton = _MainAssociatePopup->addButton("Commentary", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add associate button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(NEXUS_AudioAc4AssociateType_eCommentary);

            /* Presentation */
            ret = _MainSettings->addLabelPopupButton(this, "Ac4PresentationMain", &_MainPresentation, &_MainPresentationLabel, &_MainPresentationPopup, font12, 50);
            CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
            _MainPresentation->setFocusable(false);
            _MainPresentationLabel->setText("Presentation:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _MainPresentationPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);

            /* Priority - language vs associate */
            _MainPriority = new CWidgetCheckButton("CPanelAudio::_MainPriority", getEngine(), this, MRect(0, 0, 0, 22), font12, _pAudioMenu->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _MainPriority, ret, eRet_OutOfMemory, error);
            _MainSettings->addButton(_MainPriority, "MainPriority");
            _MainPriority->setText("Prefer Associate:", bwidget_justify_horiz_left);
            _MainPriority->setCheck(true);
        }

        _MainSettings->setGeometry(MRect(0, 30, menuWidth, subMenuHeight));
    }

#if ENABLE_ALTERNATE
    /* alternate program settings */
    {
        _AltSettings = new CWidgetMenu("CPanelAudioAc4::_AltSettings", getEngine(), _pAudioMenu, MRect(0, subMenuHeight + 60, menuWidth, subMenuHeight), font14, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _AltSettings, ret, eRet_OutOfMemory, error);
        _AltSettings->showMenuBar(false);
        _AltSettings->setMenuTitle(NULL, "Alternate");
        _AltSettings->setScroll(true);
        {
            CBoardFeatures *     pFeatures = pConfig->getBoardFeatures();
            CWidgetCheckButton * pButton   = NULL;
            MRect                rectPopup;

            /* Language */
            ret = _AltSettings->addLabelPopupButton(this, "Ac4LanguageAlt", &_AltLanguage, &_AltLanguageLabel, &_AltLanguagePopup, font12, 40);
            CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
            _AltLanguage->setFocusable(false);
            _AltLanguageLabel->setText("Language:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _AltLanguagePopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);

            rectPopup = _AltLanguagePopup->getGeometry();

            pButton = _AltLanguagePopup->addButton("English", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add langugage button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eLanguage_English);

            pButton = _AltLanguagePopup->addButton("French", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add langugage button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eLanguage_French);

            pButton = _AltLanguagePopup->addButton("German", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add langugage button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eLanguage_German);

            pButton = _AltLanguagePopup->addButton("Italian", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add langugage button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eLanguage_Italian);

            pButton = _AltLanguagePopup->addButton("Spanish", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add langugage button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eLanguage_Spanish);

            /* Associate Type */
            ret = _AltSettings->addLabelPopupButton(this, "Ac4AssociateAlt", &_AltAssociate, &_AltAssociateLabel, &_AltAssociatePopup, font12, 40);
            CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
            _AltAssociate->setFocusable(false);
            _AltAssociateLabel->setText("Associate:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _AltAssociatePopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);

            rectPopup = _AltAssociatePopup->getGeometry();

            pButton = _AltAssociatePopup->addButton("Not Specified", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add associate button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(NEXUS_AudioAc4AssociateType_eNotSpecified);

            pButton = _AltAssociatePopup->addButton("Visually Impaired", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add associate button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(NEXUS_AudioAc4AssociateType_eVisuallyImpaired);

            pButton = _AltAssociatePopup->addButton("Hearing Impaired", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add associate button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(NEXUS_AudioAc4AssociateType_eHearingImpaired);

            pButton = _AltAssociatePopup->addButton("Commentary", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add associate button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(NEXUS_AudioAc4AssociateType_eCommentary);

            /* Presentation */
            ret = _AltSettings->addLabelPopupButton(this, "Ac4PresentationAlt", &_AltPresentation, &_AltPresentationLabel, &_AltPresentationPopup, font12, 50);
            CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
            _AltPresentation->setFocusable(false);
            _AltPresentationLabel->setText("Presentation:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _AltPresentationPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);

            /* Priority - language vs associate */
            _AltPriority = new CWidgetCheckButton("CPanelAudio::_AltPriority", getEngine(), this, MRect(0, 0, 0, 22), font12, _pAudioMenu->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _AltPriority, ret, eRet_OutOfMemory, error);
            _AltSettings->addButton(_AltPriority, "AltPriority");
            _AltPriority->setText("Prefer Associate:", bwidget_justify_horiz_left);
            _AltPriority->setCheck(true);
        }

        _AltSettings->setGeometry(MRect(0, subMenuHeight + 26, menuWidth, subMenuHeight));
    }
#endif

    rectPanel.set(50, 50, menuWidth, menuHeight);
    setGeometry(rectPanel);

    /* back button */
    _Back = new CWidgetButton("CPanelAudioAc4::_Back", getEngine(), this, MRect(0, 0, 0, 0), font10);
    CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Back, ret, eRet_OutOfMemory, error);
    _Back->setText("Audio");
    _pAudioMenu->addBackButton(_Back);

    _Back->setFocus();

    goto done;
error:
    uninitialize();
done:
    return(ret);
} /* initialize */

void CPanelAudioAc4::uninitialize()
{
    DEL(_Back);
#if ENABLE_ALTERNATE
    DEL(_AltPriority);
    DEL(_AltPresentationPopup);
    DEL(_AltPresentationLabel);
    DEL(_AltPresentation);
    DEL(_AltAssociatePopup);
    DEL(_AltAssociateLabel);
    DEL(_AltAssociate);
    DEL(_AltLanguagePopup);
    DEL(_AltLanguageLabel);
    DEL(_AltLanguage);
    DEL(_AltSettings);
#endif
    DEL(_MainPriority);
    DEL(_MainPresentationPopup);
    DEL(_MainPresentationLabel);
    DEL(_MainPresentation);
    DEL(_MainAssociatePopup);
    DEL(_MainAssociateLabel);
    DEL(_MainAssociate);
    DEL(_MainLanguagePopup);
    DEL(_MainLanguageLabel);
    DEL(_MainLanguage);
    DEL(_MainSettings);
    DEL(_pAudioMenu);
} /* uninitialize */

eRet CPanelAudioAc4::findAc4Presentation(unsigned nIndex, NEXUS_AudioDecoderPresentationStatus * pPresentation)
{
    eRet ret = eRet_Ok;
    CModel *      pModel  = getModel();
    CSimpleAudioDecode * pAudioDecode = pModel->getSimpleAudioDecode();

    BDBG_ASSERT(NULL != pPresentation);

    if (NULL == pAudioDecode)
    {
        return(eRet_NotAvailable);
    }

    ret = pAudioDecode->getPresentationStatus(nIndex, pPresentation);
    CHECK_ERROR_GOTO("unable to get ac4 presentation", ret, error);

error:
    return(ret);
}

void CPanelAudioAc4::onClick(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;
    CModel *      pModel  = getModel();
    eRet          ret     = eRet_Ok;

    BDBG_ASSERT(NULL != pModel);

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }

    if ((NULL != _MainLanguagePopup) && (0 <= _MainLanguagePopup->getItemListIndex(pWidget->getWidget())))
    {
        CWidgetCheckButton *                 pButton       = (CWidgetCheckButton *)pWidget;
        eLanguage                            language      = (eLanguage)pButton->getValue();
        CAudioDecodeAc4Data                  data;

        BDBG_ASSERT(NULL != pButton);
        BDBG_WRN(("selected audio Ac4Language:%s", pButton->getText().s()));

        data._program = NEXUS_AudioDecoderAc4Program_eMain;
        data._language = language;
        notifyObservers(eNotify_SetAudioAc4Language, &data);
    }
#if ENABLE_ALTERNATE
    else
    if ((NULL != _AltLanguagePopup) && (0 <= _AltLanguagePopup->getItemListIndex(pWidget->getWidget())))
    {
        CWidgetCheckButton *                 pButton       = (CWidgetCheckButton *)pWidget;
        eLanguage                            language      = (eLanguage)pButton->getValue();
        CAudioDecodeAc4Data                  data;

        BDBG_ASSERT(NULL != pButton);
        BDBG_WRN(("selected audio Ac4Language:%s", pButton->getText().s()));

        data._program = NEXUS_AudioDecoderAc4Program_eAlternate;
        data._language = language;
        notifyObservers(eNotify_SetAudioAc4Language, &data);
    }
#endif
    else
    if ((NULL != _MainAssociatePopup) && (0 <= _MainAssociatePopup->getItemListIndex(pWidget->getWidget())))
    {
        CWidgetCheckButton *                 pButton    = (CWidgetCheckButton *)pWidget;
        NEXUS_AudioAc4AssociateType          associate  = (NEXUS_AudioAc4AssociateType)pButton->getValue();
        CAudioDecodeAc4Data                  data;

        BDBG_ASSERT(NULL != pButton);
        BDBG_WRN(("selected audio Ac4Associate:%s", pButton->getText().s()));

        data._program = NEXUS_AudioDecoderAc4Program_eMain;
        data._associate = associate;
        notifyObservers(eNotify_SetAudioAc4Associate, &data);
    }
#if ENABLE_ALTERNATE
    else
    if ((NULL != _AltAssociatePopup) && (0 <= _AltAssociatePopup->getItemListIndex(pWidget->getWidget())))
    {
        CWidgetCheckButton *                 pButton    = (CWidgetCheckButton *)pWidget;
        NEXUS_AudioAc4AssociateType          associate  = (NEXUS_AudioAc4AssociateType)pButton->getValue();
        CAudioDecodeAc4Data                  data;

        BDBG_ASSERT(NULL != pButton);
        BDBG_WRN(("selected audio Ac4Associate:%s", pButton->getText().s()));

        data._program = NEXUS_AudioDecoderAc4Program_eAlternate;
        data._associate = associate;
        notifyObservers(eNotify_SetAudioAc4Associate, &data);
    }
#endif
    else
    if ((NULL != _MainPresentationPopup) && (0 <= _MainPresentationPopup->getItemListIndex(pWidget->getWidget())))
    {
        CWidgetCheckButton *                 pButton       = (CWidgetCheckButton *)pWidget;
        unsigned                             nIndex        = (unsigned)pButton->getValue();
        CAudioDecodeAc4Data                  data;

        NEXUS_AudioDecoderPresentationStatus presentation;

        BDBG_ASSERT(NULL != pButton);
        BDBG_WRN(("selected audio Ac4Presentation:%s", pButton->getText().s()));

        data._program = NEXUS_AudioDecoderAc4Program_eMain;
        data._presentationIndex = nIndex;
        notifyObservers(eNotify_SetAudioAc4Presentation, &data);
    }
#if ENABLE_ALTERNATE
    else
    if ((NULL != _AltPresentationPopup) && (0 <= _AltPresentationPopup->getItemListIndex(pWidget->getWidget())))
    {
        CWidgetCheckButton *                 pButton       = (CWidgetCheckButton *)pWidget;
        unsigned                             nIndex        = (unsigned)pButton->getValue();
        CAudioDecodeAc4Data                  data;

        NEXUS_AudioDecoderPresentationStatus presentation;

        BDBG_ASSERT(NULL != pButton);
        BDBG_WRN(("selected audio Ac4Presentation:%s", pButton->getText().s()));

        data._program = NEXUS_AudioDecoderAc4Program_eAlternate;
        data._presentationIndex = nIndex;
        notifyObservers(eNotify_SetAudioAc4Presentation, &data);
    }
#endif
    else
    if ((NULL != _MainPriority) && (_MainPriority == pWidget))
    {
        CWidgetCheckButton *                 pButton       = (CWidgetCheckButton *)pWidget;
        ePriority                            priority      = (ePriority)pButton->getValue();
        CAudioDecodeAc4Data                  data;

        BDBG_ASSERT(NULL != pButton);
        BDBG_WRN(("selected audio Ac4Priority:%s", pButton->getText().s()));

        data._program = NEXUS_AudioDecoderAc4Program_eMain;
        data._priority = _MainPriority->isChecked() ? ePriority_Associate : ePriority_Language;

        notifyObservers(eNotify_SetAudioAc4Priority, &data);
    }
#if ENABLE_ALTERNATE
    else
    if ((NULL != _AltPriority) && (_AltPriority == pWidget))
    {
        CWidgetCheckButton *                 pButton       = (CWidgetCheckButton *)pWidget;
        ePriority                            priority      = (ePriority)pButton->getValue();
        CAudioDecodeAc4Data                  data;

        BDBG_ASSERT(NULL != pButton);
        BDBG_WRN(("selected audio Ac4Priority:%s", pButton->getText().s()));

        data._program = NEXUS_AudioDecoderAc4Program_eAlternate;
        data._priority = _AltPriority->isChecked() ? ePriority_Associate : ePriority_Language;

        notifyObservers(eNotify_SetAudioAc4Priority, &data);
    }
#endif
    else
    if ((NULL != _Back) && (_Back == pWidget))
    {
        getScreenMain()->showMenu(eMenu_Audio);
    }

error:
    return;
} /* onClick */

void CPanelAudioAc4::processNotification(CNotification & notification)
{
    BDBG_MSG(("CPanelAudioAc4::processNotification():%d", notification.getId()));

    switch (notification.getId())
    {
    case eNotify_AudioSourceChanged:
    {
        CSimpleAudioDecode *     pAudioDecode = (CSimpleAudioDecode *)notification.getData();
        NEXUS_AudioDecoderStatus status;
        CChannelMgr *            pChannelMgr = NULL;
        eRet                     ret         = eRet_Ok;

        BDBG_ASSERT(NULL != pAudioDecode);

        ret = pAudioDecode->getStatus(&status);
        CHECK_ERROR_GOTO("unable to get Audio decoder status", ret, error);

        pChannelMgr = _pModel->getChannelMgr();
        CHECK_PTR_ERROR_GOTO("unable to get channel mgr", pChannelMgr, ret, eRet_NotAvailable, error);

        /* update available ac4 presentation values */
        if (NULL != pAudioDecode)
        {
            unsigned numPresentations = pAudioDecode->numPresentations();

            /* clear popup values and re-add new ones */
            _MainPresentationPopup->clearButtons();
#if ENABLE_ALTERNATE
            _AltPresentationPopup->clearButtons();
#endif

            CPid * pPidAudio = pAudioDecode->getPid();
            if ((NULL != pPidAudio) && (NEXUS_AudioCodec_eAc4 == pPidAudio->getAudioCodec()))
            {
                MRect                                rectPopup        = _MainPresentationPopup->getGeometry();
                CWidgetCheckButton *                 pButtonMain      = NULL;
#if ENABLE_ALTERNATE
                CWidgetCheckButton *                 pButtonAlt       = NULL;
#endif
                unsigned                             i                = 0;
                unsigned                             mainIndexCurrent = pAudioDecode->getPresentationCurrent(NEXUS_AudioDecoderAc4Program_eMain);
#if ENABLE_ALTERNATE
                unsigned                             altIndexCurrent  = pAudioDecode->getPresentationCurrent(NEXUS_AudioDecoderAc4Program_eAlternate);
#endif
                NEXUS_AudioDecoderPresentationStatus presentation;

                for (i = 0; i < numPresentations; i++)
                {
                    ret = pAudioDecode->getPresentationStatus(i, &presentation);
                    CHECK_ERROR_GOTO("unable to retrieve ac4 presentation", ret, error);

                    {
                        NEXUS_AudioDecoderPresentationStatus * pPresentationTmp = NULL;
                        MString                                strTitle         = MString(presentation.status.ac4.name) + "(" + presentation.status.ac4.language + ")";

                        BDBG_MSG(("adding presentation:%s", strTitle.s()));
                        BDBG_MSG(("     Presentation id: %s", presentation.status.ac4.id));
                        BDBG_MSG(("     Presentation name: %s", presentation.status.ac4.name));
                        BDBG_MSG(("     Presentation language: %s", presentation.status.ac4.language));
                        BDBG_MSG(("     Presentation associateType: %lu", (unsigned long)presentation.status.ac4.associateType));

                        /* add MAIN presentation option to dropdown list control */
                        pButtonMain = _MainPresentationPopup->addButton(strTitle, rectPopup.width(), rectPopup.height());
                        CHECK_PTR_ERROR_GOTO("unable to add MAIN AC4 presentation button to popup list", pButtonMain, ret, eRet_OutOfMemory, error);
                        pButtonMain->setValue(presentation.status.ac4.index);
#if ENABLE_ALTERNATE
                        /* add ALT presentation option to dropdown list control */
                        pButtonAlt = _AltPresentationPopup->addButton(strTitle, rectPopup.width(), rectPopup.height());
                        CHECK_PTR_ERROR_GOTO("unable to add ALT AC4 presentation button to popup list", pButtonAlt, ret, eRet_OutOfMemory, error);
                        pButtonAlt->setValue(presentation.status.ac4.index);
#endif
                    }

                    /* set currently selected presentation */
                    if (mainIndexCurrent == i)
                    {
                        _MainPresentationPopup->select(mainIndexCurrent);
                    }
#if ENABLE_ALTERNATE
                    if (altIndexCurrent == i)
                    {
                        _AltPresentationPopup->select(altIndexCurrent);
                    }
#endif
                }
            }
        }
    }
    break;

    case eNotify_AudioAc4LanguageChanged:
    {
        CSimpleAudioDecode * pAudioDecode = (CSimpleAudioDecode *)notification.getData();
        eLanguage            language     = eLanguage_Max;

        language = pAudioDecode->getLanguage(NEXUS_AudioDecoderAc4Program_eMain);
        _MainLanguagePopup->select(language);
#if ENABLE_ALTERNATE
        language = pAudioDecode->getLanguage(NEXUS_AudioDecoderAc4Program_eAlternate);
        _AltLanguagePopup->select(language);
#endif
    }
    break;

    case eNotify_AudioAc4AssociateChanged:
    {
        CSimpleAudioDecode *        pAudioDecode = (CSimpleAudioDecode *)notification.getData();
        NEXUS_AudioAc4AssociateType associate    = NEXUS_AudioAc4AssociateType_eMax;

        associate = pAudioDecode->getAssociate(NEXUS_AudioDecoderAc4Program_eMain);
        _MainAssociatePopup->select(associate);
#if ENABLE_ALTERNATE
        associate = pAudioDecode->getAssociate(NEXUS_AudioDecoderAc4Program_eAlternate);
        _AltAssociatePopup->select(associate);
#endif
    }
    break;

    case eNotify_AudioAc4PresentationChanged:
    {
        CSimpleAudioDecode * pAudioDecode = (CSimpleAudioDecode *)notification.getData();
        unsigned             index        = 0;

        index = pAudioDecode->getPresentation(NEXUS_AudioDecoderAc4Program_eMain);
        _MainPresentationPopup->select(index);
#if ENABLE_ALTERNATE
        index = pAudioDecode->getPresentation(NEXUS_AudioDecoderAc4Program_eAlternate);
        _AltPresentationPopup->select(index);
#endif
    }
    break;

    case eNotify_AudioAc4PriorityChanged:
    {
        CSimpleAudioDecode * pAudioDecode = (CSimpleAudioDecode *)notification.getData();
        ePriority            priority     = ePriority_Max;

        priority = pAudioDecode->getPriority(NEXUS_AudioDecoderAc4Program_eMain);
        _MainPriority->setCheck((ePriority_Associate == priority) ? true : false);
#if ENABLE_ALTERNATE
        priority = pAudioDecode->getPriority(NEXUS_AudioDecoderAc4Program_eAlternate);
        _AltPriority->setCheck((ePriority_Associate == priority) ? true : false);
#endif
    }
    break;

    case eNotify_HdmiHotplugEvent:
    {
        eRet                     ret          = eRet_NotAvailable;
        CSimpleAudioDecode *     pAudioDecode = _pModel->getSimpleAudioDecode();
        NEXUS_AudioDecoderStatus status;

        if (pAudioDecode)
        {
            ret = pAudioDecode->getStatus(&status);
        }
        CHECK_ERROR_GOTO("unable to get Audio decoder status", ret, error);
    }
    break;

    default:
        break;
    } /* switch */

error:
    return;
} /* processNotification */