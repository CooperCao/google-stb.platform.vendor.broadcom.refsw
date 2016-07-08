/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "panel_audio.h"
#include "channelmgr.h"
#include "channel.h"
#include "audio_decode.h"

BDBG_MODULE(panel_audio);

CPanelAudio::CPanelAudio(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelAudio", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pAudioMenu(NULL),
    _settings(NULL),
    _dialogNorm(NULL),
    _Pid(NULL),
    _PidLabel(NULL),
    _PidPopup(NULL),
    _AudioProcessing(NULL),
    _AudioProcessingLabel(NULL),
    _AudioProcessingPopup(NULL),
    _Spdif(NULL),
    _SpdifLabel(NULL),
    _SpdifPopup(NULL),
    _Hdmi(NULL),
    _HdmiLabel(NULL),
    _HdmiPopup(NULL),
    _Downmix(NULL),
    _DownmixLabel(NULL),
    _DownmixPopup(NULL),
    _DualMono(NULL),
    _DualMonoLabel(NULL),
    _DualMonoPopup(NULL),
    _Compression(NULL),
    _CompressionLabel(NULL),
    _CompressionPopup(NULL),
    _Back(NULL)
{
}

CPanelAudio::~CPanelAudio()
{
    uninitialize();
}

eRet CPanelAudio::initialize(
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
    int         menuHeight     = 238;
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

    _pAudioMenu = new CWidgetMenu("CPanelAudio::_pAudioMenu", getEngine(), this, MRect(0, 0, menuWidth, menuHeight), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pAudioMenu, ret, eRet_OutOfMemory, error);
    _pAudioMenu->showSubTitle(false);
    _pAudioMenu->showListView(false);
    _pAudioMenu->showEdit(false);
    _pAudioMenu->setMenuTitle("Audio", NULL, bwidget_justify_horiz_center, bwidget_justify_vert_middle);
    _pAudioMenu->show(true);
    {
        _settings = new CWidgetMenu("CPanelAudio::_settings", getEngine(), _pAudioMenu, MRect(0, 30, menuWidth, menuHeight - 31), font14, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _settings, ret, eRet_OutOfMemory, error);
        _settings->showMenuBar(false);
        _settings->setMenuTitle(NULL, "Settings");
        _settings->setScroll(true);
        {
            CBoardFeatures *     pFeatures = pConfig->getBoardFeatures();
            CWidgetCheckButton * pButton   = NULL;
            MRect                rectPopup;

            /* PID */
            ret = addLabelPopupButton(_settings, "Pid", &_Pid, &_PidLabel, &_PidPopup, font12, 25);
            CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
            _Pid->setFocusable(false);
            _PidLabel->setText("Pid:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _PidPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
            /* buttons are stream dependent and will be populated in source changed callback */

            /* Audio Processing */
            if ((true == pFeatures->_autoVolume) ||
                (true == pFeatures->_dolbyVolume) ||
                (true == pFeatures->_srsVolume))
            {
                ret = addLabelPopupButton(_settings, "AudioProcessing", &_AudioProcessing, &_AudioProcessingLabel, &_AudioProcessingPopup, font12, 35);
                CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
                _AudioProcessing->setFocusable(false);
                _AudioProcessingLabel->setText("PCM:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
                _AudioProcessingPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
                rectPopup = _AudioProcessingPopup->getGeometry();
                pButton   = _AudioProcessingPopup->addButton("Stereo", rectPopup.width(), rectPopup.height());
                CHECK_PTR_ERROR_GOTO("unable to add Pcm button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(eAudioProcessing_None);

                if (true == pFeatures->_autoVolume)
                {
                    pButton = _AudioProcessingPopup->addButton("Auto Vol Level", rectPopup.width(), rectPopup.height());
                    CHECK_PTR_ERROR_GOTO("unable to add AVL button to popup list", pButton, ret, eRet_OutOfMemory, error);
                    pButton->setValue(eAudioProcessing_AutoVolumeLevel);
                }
                if (true == pFeatures->_dolbyVolume)
                {
                    pButton = _AudioProcessingPopup->addButton("Dolby Volume", rectPopup.width(), rectPopup.height());
                    CHECK_PTR_ERROR_GOTO("unable to add Dolby Volume button to popup list", pButton, ret, eRet_OutOfMemory, error);
                    pButton->setValue(eAudioProcessing_DolbyVolume);
                }
                if (true == pFeatures->_srsVolume)
                {
                    pButton = _AudioProcessingPopup->addButton("TruVolume", rectPopup.width(), rectPopup.height());
                    CHECK_PTR_ERROR_GOTO("unable to add SRS TruVolume button to popup list", pButton, ret, eRet_OutOfMemory, error);
                    pButton->setValue(eAudioProcessing_SrsTruVolume);
                }
            }
            else
            {
                /* audio processing is not supported so adjust menu height */
                menuHeight -= 21;
            }

            /* SPDIF */
            ret = addLabelPopupButton(_settings, "Spdif", &_Spdif, &_SpdifLabel, &_SpdifPopup, font12, 35);
            CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
            _Spdif->setFocusable(false);
            _SpdifLabel->setText("Spdif:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _SpdifPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
            rectPopup = _SpdifPopup->getGeometry();
            pButton   = _SpdifPopup->addButton("Pcm", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add Pcm button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eSpdifInput_Pcm);
            pButton = _SpdifPopup->addButton("Compressed", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add Compressed button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eSpdifInput_Compressed);
            pButton = _SpdifPopup->addButton("Encode DTS", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add DTS button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eSpdifInput_EncodeDts);
            pButton = _SpdifPopup->addButton("Encode AC-3", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add AC-3 button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eSpdifInput_EncodeAc3);

            /* HDMI */
            {
                bool                 bHdmi        = false;
                CSimpleAudioDecode * pAudioDecode = _pModel->getSimpleAudioDecode();
                if (NULL != pAudioDecode)
                {
                    COutputHdmi * pOutputHdmi = pAudioDecode->getOutputHdmi();
                    if (NULL != pOutputHdmi)
                    {
                        ret = addLabelPopupButton(_settings, "Hdmi", &_Hdmi, &_HdmiLabel, &_HdmiPopup, font12, 35);
                        CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
                        _Hdmi->setFocusable(false);
                        _HdmiLabel->setText("Hdmi:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
                        _HdmiPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
                        rectPopup = _HdmiPopup->getGeometry();
                        pButton   = _HdmiPopup->addButton("Pcm", rectPopup.width(), rectPopup.height());
                        CHECK_PTR_ERROR_GOTO("unable to add Pcm button to popup list", pButton, ret, eRet_OutOfMemory, error);
                        pButton->setValue(eHdmiAudioInput_Pcm);
                        pButton = _HdmiPopup->addButton("Compressed", rectPopup.width(), rectPopup.height());
                        CHECK_PTR_ERROR_GOTO("unable to add Compressed button to popup list", pButton, ret, eRet_OutOfMemory, error);
                        pButton->setValue(eHdmiAudioInput_Compressed);
                        pButton = _HdmiPopup->addButton("Pcm 5.1", rectPopup.width(), rectPopup.height());
                        CHECK_PTR_ERROR_GOTO("unable to add Pcm 5.1 button to popup list", pButton, ret, eRet_OutOfMemory, error);
                        pButton->setValue(eHdmiAudioInput_Multichannel);
                        pButton = _HdmiPopup->addButton("Encode DTS", rectPopup.width(), rectPopup.height());
                        CHECK_PTR_ERROR_GOTO("unable to add DTS button to popup list", pButton, ret, eRet_OutOfMemory, error);
                        pButton->setValue(eHdmiAudioInput_EncodeDts);
                        pButton = _HdmiPopup->addButton("Encode AC-3", rectPopup.width(), rectPopup.height());
                        CHECK_PTR_ERROR_GOTO("unable to add AC-3 button to popup list", pButton, ret, eRet_OutOfMemory, error);
                        pButton->setValue(eHdmiAudioInput_EncodeAc3);

                        bHdmi = true;
                    }
                }

                if (false == bHdmi)
                {
                    /* hdmi is not supported so adjust menu height */
                    menuHeight -= 21;
                }
            }

            /* DOWNMIX */
            ret = addLabelPopupButton(_settings, "Downmix", &_Downmix, &_DownmixLabel, &_DownmixPopup, font12, 35);
            CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
            _Downmix->setFocusable(false);
            _DownmixLabel->setText("Downmix:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _DownmixPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
            /* buttons are codec dependent and will be populated in source changed callback */

            /* DUAL MONO */
            ret = addLabelPopupButton(_settings, "DualMono", &_DualMono, &_DualMonoLabel, &_DualMonoPopup, font12, 45);
            CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
            _DualMono->setFocusable(false);
            _DualMonoLabel->setText("DualMono:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _DualMonoPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
            rectPopup = _DualMonoPopup->getGeometry();
            pButton   = _DualMonoPopup->addButton("Left", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add Left button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eAudioDualMono_Left);
            pButton = _DualMonoPopup->addButton("Right", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add Right button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eAudioDualMono_Right);
            pButton = _DualMonoPopup->addButton("Stereo", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add Stereo button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eAudioDualMono_Stereo);
            pButton = _DualMonoPopup->addButton("MonoMix", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add MonoMix button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eAudioDualMono_Monomix);
            _DualMonoPopup->select(eAudioDualMono_Stereo);

            /* COMPRESSION */
            ret = addLabelPopupButton(_settings, "Compression", &_Compression, &_CompressionLabel, &_CompressionPopup, font12);
            CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
            _Compression->setFocusable(false);
            _CompressionLabel->setText("Compression:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _CompressionPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
            rectPopup = _CompressionPopup->getGeometry();
            pButton   = _CompressionPopup->addButton("None", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add None button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eDolbyDRC_None);
            pButton = _CompressionPopup->addButton("Heavy", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add Heavy button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eDolbyDRC_Heavy);
            pButton = _CompressionPopup->addButton("Medium", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add Medium button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(eDolbyDRC_Medium);

            /* DIALOG NORMALIZATION */
            _dialogNorm = new CWidgetCheckButton("CPanelAudio::_dialogNorm", getEngine(), this, MRect(0, 0, 0, 22), font12, _pAudioMenu->getWin());
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _dialogNorm, ret, eRet_OutOfMemory, error);
            _settings->addButton(_dialogNorm, "Dialog Normalization");
            _dialogNorm->setText("Dialog Normalization:", bwidget_justify_horiz_left);
            _dialogNorm->setCheck(true);
        }

        /* back button */
        _Back = new CWidgetButton("CPanelAudio::_Back", getEngine(), this, MRect(0, 0, 0, 0), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Back, ret, eRet_OutOfMemory, error);
        _Back->setText("Menu");
        _pAudioMenu->addBackButton(_Back);

        rectPanel.set(50, 50, menuWidth, menuHeight);
        setGeometry(rectPanel);
        _settings->setGeometry(MRect(0, 30, menuWidth, menuHeight - 31));

        _Back->setFocus();
    }

    goto done;
error:
    uninitialize();
done:
    return(ret);
} /* initialize */

void CPanelAudio::uninitialize()
{
    DEL(_Back);
    DEL(_dialogNorm);
    DEL(_CompressionPopup);
    DEL(_CompressionLabel);
    DEL(_Compression);
    DEL(_DualMonoPopup);
    DEL(_DualMonoLabel);
    DEL(_DualMono);
    DEL(_DownmixPopup);
    DEL(_DownmixLabel);
    DEL(_Downmix);
    DEL(_HdmiPopup);
    DEL(_HdmiLabel);
    DEL(_Hdmi);
    DEL(_SpdifPopup);
    DEL(_SpdifLabel);
    DEL(_Spdif);
    DEL(_AudioProcessingPopup);
    DEL(_AudioProcessingLabel);
    DEL(_AudioProcessing);
    DEL(_PidPopup);
    DEL(_PidLabel);
    DEL(_Pid);
    DEL(_settings);
    DEL(_pAudioMenu);
} /* uninitialize */

void CPanelAudio::onClick(bwidget_t widget)
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

    if ((NULL != _PidPopup) && (0 <= _PidPopup->getItemListIndex(pWidget->getWidget())))
    {
        uint32_t pid = ((CWidgetCheckButton *)pWidget)->getValue();

        BDBG_WRN(("selected audio pid:0x%x", pid));
        notifyObservers(eNotify_SetAudioProgram, &pid);
    }
    else
    if ((NULL != _AudioProcessingPopup) && (0 <= _AudioProcessingPopup->getItemListIndex(pWidget->getWidget())))
    {
        CWidgetCheckButton * pButton         = (CWidgetCheckButton *)pWidget;
        eAudioProcessing     audioProcessing = (eAudioProcessing)pButton->getValue();

        BDBG_WRN(("selected audio processing:%s", pButton->getText().s()));
        notifyObservers(eNotify_SetAudioProcessing, &audioProcessing);
    }
    else
    if ((NULL != _SpdifPopup) && (0 <= _SpdifPopup->getItemListIndex(pWidget->getWidget())))
    {
        CWidgetCheckButton * pButton    = (CWidgetCheckButton *)pWidget;
        eSpdifInput          spdifInput = (eSpdifInput)pButton->getValue();

        BDBG_WRN(("selected audio spdif input:%s", pButton->getText().s()));
        notifyObservers(eNotify_SetSpdifInput, &spdifInput);
    }
    else
    if ((NULL != _HdmiPopup) && (0 <= _HdmiPopup->getItemListIndex(pWidget->getWidget())))
    {
        CWidgetCheckButton * pButton   = (CWidgetCheckButton *)pWidget;
        eHdmiAudioInput      hdmiInput = (eHdmiAudioInput)pButton->getValue();

        BDBG_WRN(("selected audio hdmi input:%s (%d)", pButton->getText().s(), hdmiInput));
        notifyObservers(eNotify_SetHdmiAudioInput, &hdmiInput);
    }
    else
    if ((NULL != _DownmixPopup) && (0 <= _DownmixPopup->getItemListIndex(pWidget->getWidget())))
    {
        CWidgetCheckButton * pButton = (CWidgetCheckButton *)pWidget;
        eAudioDownmix        downmix = (eAudioDownmix)pButton->getValue();

        BDBG_WRN(("selected audio Downmix:%s(%d)", pButton->getText().s(), downmix));
        notifyObservers(eNotify_SetAudioDownmix, &downmix);
    }
    else
    if ((NULL != _DualMonoPopup) && (0 <= _DualMonoPopup->getItemListIndex(pWidget->getWidget())))
    {
        CWidgetCheckButton * pButton  = (CWidgetCheckButton *)pWidget;
        eAudioDualMono       dualMono = (eAudioDualMono)pButton->getValue();

        BDBG_WRN(("selected audio DualMono:%s", pButton->getText().s()));
        notifyObservers(eNotify_SetAudioDualMono, &dualMono);
    }
    else
    if ((NULL != _CompressionPopup) && (0 <= _CompressionPopup->getItemListIndex(pWidget->getWidget())))
    {
        CWidgetCheckButton * pButton  = (CWidgetCheckButton *)pWidget;
        eDolbyDRC            dolbyDRC = (eDolbyDRC)pButton->getValue();

        BDBG_WRN(("selected Dolby DRC:%s", pButton->getText().s()));
        notifyObservers(eNotify_SetDolbyDRC, &dolbyDRC);
    }
    else
    if ((NULL != _dialogNorm) && (_dialogNorm == pWidget))
    {
        bool dialogNorm = _dialogNorm->isChecked();

        BDBG_WRN(("dialog normalization:%d", dialogNorm));
        notifyObservers(eNotify_SetDolbyDialogNorm, &dialogNorm);
    }
    else
    if ((NULL != _Back) && (_Back == pWidget))
    {
        show(false);
        getParent()->show(true);
    }

    return;
} /* onClick */

bool CPanelAudio::isCompressedSupported(NEXUS_AudioCodec codec)
{
    bool bEnableCompressed = false;

    /* only enable compressed for ac3 and dts */
    if ((NEXUS_AudioCodec_eAc3 == codec) ||
        (NEXUS_AudioCodec_eDts == codec) ||
        (NEXUS_AudioCodec_eDtsLegacy == codec) ||
        (NEXUS_AudioCodec_eDtsHd == codec) ||
        (NEXUS_AudioCodec_eAc3Plus == codec))
    {
        bEnableCompressed = true;
    }

    return(bEnableCompressed);
}

bool CPanelAudio::isEncodeSupportedAc3(
        CSimpleAudioDecode * pAudioDecode,
        NEXUS_AudioCodec     codec
        )
{
    bool bEnableEncodeAc3 = false;

    if (true == pAudioDecode->isEncodeSupportedAc3())
    {
        /* only enable EncodeAc3 for aac */
        if ((NEXUS_AudioCodec_eAacAdts == codec) ||
            (NEXUS_AudioCodec_eAacLoas == codec) ||
            (NEXUS_AudioCodec_eAacPlusLoas == codec) ||
            (NEXUS_AudioCodec_eAacPlusAdts == codec))
        {
            bEnableEncodeAc3 = true;
        }
    }

    return(bEnableEncodeAc3);
} /* isEncodeSupportedAc3 */

bool CPanelAudio::isEncodeSupportedDts(
        CSimpleAudioDecode * pAudioDecode,
        NEXUS_AudioCodec     codec
        )
{
    bool bEnableEncodeDts = false;

    if (true == pAudioDecode->isEncodeSupportedDts())
    {
        /* only enable EncodeDts for aac */
        if ((NEXUS_AudioCodec_eAacAdts == codec) ||
            (NEXUS_AudioCodec_eAacLoas == codec) ||
            (NEXUS_AudioCodec_eAacPlusLoas == codec) ||
            (NEXUS_AudioCodec_eAacPlusAdts == codec))
        {
            bEnableEncodeDts = true;
        }
    }

    return(bEnableEncodeDts);
} /* isEncodeSupportedDts */

void CPanelAudio::processNotification(CNotification & notification)
{
    BDBG_MSG(("CPanelAudio::processNotification():%d", notification.getId()));

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

        /* add new pid values in menu */
        {
            CPidMgr * pPidMgr = NULL;
            MRect     rect    = _PidPopup->getGeometry();

            _PidPopup->popup(false);
            _PidPopup->clearButtons();

            /* get pidmgr based on current mode */
            switch (_pModel->getMode())
            {
            case eMode_Live:
            {
                CChannel * pChannel = _pModel->getCurrentChannel();
                if (NULL != pChannel)
                {
                    pPidMgr = pChannel->getPidMgr();
                }
                break;
            }
            case eMode_Playback:
            {
                CPlayback * pPlayback = _pModel->getPlayback();
                if (NULL != pPlayback)
                {
                    pPidMgr = pPlayback->getVideo()->getPidMgr();
                }
                break;
            }
            default:
                break;
            }   /* switch */
            CHECK_PTR_ERROR_GOTO("unable to get pidmgr", pPidMgr, ret, eRet_NotAvailable, error);

            {
                CPid *   pPidAudio  = NULL;
                int      i          = 0;
                uint32_t pidDecoder = 0;

                if (NULL != pAudioDecode->getPid())
                {
                    /* get pid number from audio decoder */
                    pidDecoder = pAudioDecode->getPid()->getPid();
                }

                while (NULL != (pPidAudio = pPidMgr->getPid(i, ePidType_Audio)))
                {
                    CWidgetCheckButton * pButton = NULL;
                    char                 strPid[32];

                    snprintf(strPid, sizeof(strPid) - 1, "0x%x (%s)", pPidAudio->getPid(), audioCodecToString(pPidAudio->getAudioCodec()).s());

                    pButton = _PidPopup->addButton(strPid, rect.width(), rect.height());
                    CHECK_PTR_ERROR_GOTO("unable to add pid button to popup list", pButton, ret, eRet_OutOfMemory, error);
                    pButton->setValue(pPidAudio->getPid());

                    if (pPidAudio->getPid() == pidDecoder)
                    {
                        _PidPopup->select(pButton);
                    }

                    i++;
                }
            }
        }

        /* update audio processing setting */
        if (NULL != _AudioProcessingPopup)
        {
            CWidgetCheckButton * pButtonAutoVolumeLevel = _AudioProcessingPopup->findButton(eAudioProcessing_AutoVolumeLevel);
            CWidgetCheckButton * pButtonDolbyVolume     = _AudioProcessingPopup->findButton(eAudioProcessing_DolbyVolume);
            CWidgetCheckButton * pButtonSrsTruVolume    = _AudioProcessingPopup->findButton(eAudioProcessing_SrsTruVolume);

            _AudioProcessingPopup->popup(false);
            _AudioProcessingPopup->select(pAudioDecode->getAudioProcessing());

            _AudioProcessingPopup->setActive(pButtonAutoVolumeLevel, pAudioDecode->isAutoVolumeLevelSupported());
            _AudioProcessingPopup->setActive(pButtonDolbyVolume, pAudioDecode->isDolbyVolumeSupported());
            _AudioProcessingPopup->setActive(pButtonSrsTruVolume, pAudioDecode->isTruVolumeSupported());
        }

        /* update spdif setting */
        {
            _SpdifPopup->popup(false);
            _SpdifPopup->select(pAudioDecode->getSpdifInput(status.codec));

            {
                CWidgetCheckButton * pButtonCompressed = _SpdifPopup->findButton(eSpdifInput_Compressed);
                if (NULL != pButtonCompressed)
                {
                    _SpdifPopup->setActive(pButtonCompressed, isCompressedSupported(status.codec));
                }

                CWidgetCheckButton * pButtonEncodeAc3 = _SpdifPopup->findButton(eSpdifInput_EncodeAc3);
                if (NULL != pButtonEncodeAc3)
                {
                    _SpdifPopup->setActive(pButtonEncodeAc3, isEncodeSupportedAc3(pAudioDecode, status.codec));
                }

                CWidgetCheckButton * pButtonEncodeDts = _SpdifPopup->findButton(eSpdifInput_EncodeDts);
                if (NULL != pButtonEncodeDts)
                {
                    _SpdifPopup->setActive(pButtonEncodeDts, isEncodeSupportedDts(pAudioDecode, status.codec));
                }
            }
        }

        /* update hdmi setting */
        if (NULL != _HdmiPopup)
        {
            _HdmiPopup->popup(false);
            _HdmiPopup->select(pAudioDecode->getHdmiInput(status.codec));

            /* update hdmi audio options */
            {
                COutputHdmi *        pOutputHdmi       = pAudioDecode->getOutputHdmi();
                CWidgetCheckButton * pButtonCompressed = _HdmiPopup->findButton(eHdmiAudioInput_Compressed);
                if (NULL != pButtonCompressed)
                {
                    _HdmiPopup->setActive(pButtonCompressed, pOutputHdmi->isValidAudioCompressed(status.codec));
                }

                CWidgetCheckButton * pButtonMultichannel = _HdmiPopup->findButton(eHdmiAudioInput_Multichannel);
                if (NULL != pButtonMultichannel)
                {
                    _HdmiPopup->setActive(pButtonMultichannel, pOutputHdmi->isValidAudioMultichannel(status.codec));
                }

                CWidgetCheckButton * pButtonEncodeAc3 = _HdmiPopup->findButton(eHdmiAudioInput_EncodeAc3);
                if (NULL != pButtonEncodeAc3)
                {
                    bool bEnableEncodeAc3 = false;

                    if (true == pAudioDecode->isEncodeSupportedAc3())
                    {
                        /* only enable EncodeAc3 for aac */
                        if ((NEXUS_AudioCodec_eAacAdts == status.codec) ||
                            (NEXUS_AudioCodec_eAacLoas == status.codec) ||
                            (NEXUS_AudioCodec_eAacPlusLoas == status.codec) ||
                            (NEXUS_AudioCodec_eAacPlusAdts == status.codec))
                        {
                            bEnableEncodeAc3 = true;
                        }
                    }

                    _HdmiPopup->setActive(pButtonEncodeAc3, bEnableEncodeAc3);
                }

                CWidgetCheckButton * pButtonEncodeDts = _HdmiPopup->findButton(eHdmiAudioInput_EncodeDts);
                if (NULL != pButtonEncodeDts)
                {
                    bool bEnableEncodeDts = false;

                    if (true == pAudioDecode->isEncodeSupportedDts())
                    {
                        /* only enable EncodeDts for aac */
                        if ((NEXUS_AudioCodec_eAacAdts == status.codec) ||
                            (NEXUS_AudioCodec_eAacLoas == status.codec) ||
                            (NEXUS_AudioCodec_eAacPlusLoas == status.codec) ||
                            (NEXUS_AudioCodec_eAacPlusAdts == status.codec))
                        {
                            bEnableEncodeDts = true;
                        }
                    }

                    _HdmiPopup->setActive(pButtonEncodeDts, bEnableEncodeDts);
                }
            }
        }

        /* add codec dependent downmix values to menu */
        {
            CWidgetCheckButton * pButton = NULL;
            MRect                rect;

            _DownmixPopup->popup(false);
            _DownmixPopup->clearButtons();

            rect = _DownmixPopup->getGeometry();

            switch (status.codec)
            {
            case NEXUS_AudioCodec_eAc3:
            case NEXUS_AudioCodec_eAc3Plus:
                pButton = _DownmixPopup->addButton("Auto", rect.width(), rect.height());
                CHECK_PTR_ERROR_GOTO("unable to add Auto dolby downmix button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(eAudioDownmix_Ac3Auto);

                pButton = _DownmixPopup->addButton("Surround", rect.width(), rect.height());
                CHECK_PTR_ERROR_GOTO("unable to add LtRt dolby downmix button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(eAudioDownmix_Ac3LtRt);

                pButton = _DownmixPopup->addButton("Standard", rect.width(), rect.height());
                CHECK_PTR_ERROR_GOTO("unable to add LoRo dolby downmix button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(eAudioDownmix_Ac3LoRo);
                break;

            case NEXUS_AudioCodec_eDts:
            case NEXUS_AudioCodec_eDtsCd:
            case NEXUS_AudioCodec_eDtsExpress:
                pButton = _DownmixPopup->addButton("Auto", rect.width(), rect.height());
                CHECK_PTR_ERROR_GOTO("unable to add Auto dts downmix button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(eAudioDownmix_DtsAuto);

                pButton = _DownmixPopup->addButton("Surround", rect.width(), rect.height());
                CHECK_PTR_ERROR_GOTO("unable to add LtRt dts downmix button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(eAudioDownmix_DtsLtRt);

                pButton = _DownmixPopup->addButton("Standard", rect.width(), rect.height());
                CHECK_PTR_ERROR_GOTO("unable to add LoRo dts downmix button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(eAudioDownmix_DtsLoRo);
                break;

            case NEXUS_AudioCodec_eAac:
            case NEXUS_AudioCodec_eAacLoas:
            case NEXUS_AudioCodec_eAacPlus:
            case NEXUS_AudioCodec_eAacPlusAdts:
                pButton = _DownmixPopup->addButton("Matrix", rect.width(), rect.height());
                CHECK_PTR_ERROR_GOTO("unable to add Matrix button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(eAudioDownmix_AacMatrix);

                pButton = _DownmixPopup->addButton("Arib", rect.width(), rect.height());
                CHECK_PTR_ERROR_GOTO("unable to add Arib button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(eAudioDownmix_AacArib);

                pButton = _DownmixPopup->addButton("LeftRight", rect.width(), rect.height());
                CHECK_PTR_ERROR_GOTO("unable to add LeftRight button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(eAudioDownmix_AacLtRt);

                break;

            default:
                pButton = _DownmixPopup->addButton("None", rect.width(), rect.height());
                CHECK_PTR_ERROR_GOTO("unable to add None button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(eAudioDownmix_None);

                pButton = _DownmixPopup->addButton("Left", rect.width(), rect.height());
                CHECK_PTR_ERROR_GOTO("unable to add Left button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(eAudioDownmix_Left);

                pButton = _DownmixPopup->addButton("Right", rect.width(), rect.height());
                CHECK_PTR_ERROR_GOTO("unable to add Right button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(eAudioDownmix_Right);

                pButton = _DownmixPopup->addButton("Mono", rect.width(), rect.height());
                CHECK_PTR_ERROR_GOTO("unable to add Mono button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(eAudioDownmix_Monomix);

                break;
            }   /* switch */

            _DownmixPopup->select(pAudioDecode->getDownmix());
        }

        /* update dualmono setting */
        _DualMonoPopup->select(pAudioDecode->getDualMono());

        /* update dolby DRC setting */
        _CompressionPopup->select(pAudioDecode->getDolbyDRC());

        /* update dolby dialog normalization setting */
        _dialogNorm->setCheck(pAudioDecode->getDolbyDialogNorm());
    }
    break;

    case eNotify_HdmiHotplugEvent:
    {
        eRet                     ret          = eRet_Ok;
        CSimpleAudioDecode *     pAudioDecode = _pModel->getSimpleAudioDecode();
        NEXUS_AudioDecoderStatus status;

        ret = pAudioDecode->getStatus(&status);
        CHECK_ERROR_GOTO("unable to get Audio decoder status", ret, error);

        /* update hdmi setting */
        _HdmiPopup->popup(false);
        _HdmiPopup->select(pAudioDecode->getHdmiInput(status.codec));

        /* update hdmi audio options */
        {
            COutputHdmi *        pOutputHdmi         = pAudioDecode->getOutputHdmi();
            CWidgetCheckButton * pButtonCompressed   = _HdmiPopup->findButton(eHdmiAudioInput_Compressed);
            CWidgetCheckButton * pButtonMultichannel = _HdmiPopup->findButton(eHdmiAudioInput_Multichannel);

            if (NULL != pButtonCompressed)
            {
                _HdmiPopup->setActive(pButtonCompressed, pOutputHdmi->isValidAudioCompressed(status.codec));
            }
            if (NULL != pButtonMultichannel)
            {
                _HdmiPopup->setActive(pButtonMultichannel, pOutputHdmi->isValidAudioMultichannel(status.codec));
            }
        }
    }
    break;

    default:
        break;
    } /* switch */

error:
    return;
} /* processNotification */