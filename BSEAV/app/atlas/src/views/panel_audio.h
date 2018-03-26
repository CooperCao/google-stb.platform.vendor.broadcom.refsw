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

#ifndef ATLAS_PANEL_AUDIO_H__
#define ATLAS_PANEL_AUDIO_H__

#include "panel.h"
#include "timer.h"
#include "widget_label.h"
#include "widget_button.h"
#include "widget_check_button.h"
#include "widget_menu.h"
#include "widget_edit.h"
#include "widget_popup_list.h"
#include "widget_modalmsgbox.h"

#ifdef __cplusplus
extern "C" {
#endif

class CPanelAudio : public CPanel
{
public:
    CPanelAudio(
            CWidgetEngine * pWidgetEngine,
            CScreenMain *   pScreenMain,
            CWidgetBase *   pParentWidget,
            MRect           geometry,
            bwin_font_t     font,
            bwin_t          parentWin = NULL
            ); /* optional */
    ~CPanelAudio(void);

    eRet initialize(CModel * pModel, CConfig * pConfig);
    void uninitialize(void);
    void onClick(bwidget_t widget);
    void processNotification(CNotification & notification);

    bool isCompressedSupported(NEXUS_AudioCodec codec);
    bool isEncodeSupportedAc3(CSimpleAudioDecode * pAudioDecode, NEXUS_AudioCodec codec);
    bool isEncodeSupportedDts(CSimpleAudioDecode * pAudioDecode, NEXUS_AudioCodec codec);

#if BDSP_MS12_SUPPORT
    eRet findAc4Presentation(unsigned nIndex, NEXUS_AudioDecoderPresentationStatus * pPresentation);
#endif

protected:
    CWidgetMenu *        _pAudioMenu;
    CWidgetMenu *        _settings;
    CWidgetCheckButton * _dialogNorm;
    CWidgetButton *      _Pid;
    CWidgetLabel *       _PidLabel;
    CWidgetPopupList *   _PidPopup;
#if BDSP_MS12_SUPPORT
    CWidgetButton * _Ac4;
#endif
    CWidgetButton *    _AudioProcessing;
    CWidgetLabel *     _AudioProcessingLabel;
    CWidgetPopupList * _AudioProcessingPopup;
    CWidgetButton *    _Spdif;
    CWidgetLabel *     _SpdifLabel;
    CWidgetPopupList * _SpdifPopup;
    CWidgetButton *    _Hdmi;
    CWidgetLabel *     _HdmiLabel;
    CWidgetPopupList * _HdmiPopup;
    CWidgetButton *    _Downmix;
    CWidgetLabel *     _DownmixLabel;
    CWidgetPopupList * _DownmixPopup;
    CWidgetButton *    _DualMono;
    CWidgetLabel *     _DualMonoLabel;
    CWidgetPopupList * _DualMonoPopup;
    CWidgetButton *    _Compression;
    CWidgetLabel *     _CompressionLabel;
    CWidgetPopupList * _CompressionPopup;
    CWidgetButton *    _Back;
};

#ifdef __cplusplus
}
#endif

#endif /* ATLAS_PANEL_AUDIO_H__ */