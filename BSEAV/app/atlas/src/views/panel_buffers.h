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

#ifndef ATLAS_PANEL_BUFFERS_H__
#define ATLAS_PANEL_BUFFERS_H__

#include "panel.h"
#include "timer.h"
#include "widget_label.h"
#include "widget_button.h"
#include "widget_progress.h"
#include "widget_meter.h"

#ifdef __cplusplus
extern "C" {
#endif

class CPanelBuffers : public CPanel
{
public:
    CPanelBuffers(
            CWidgetEngine * pWidgetEngine,
            CScreenMain *   pScreenMain,
            CWidgetBase *   pParentWidget,
            MRect           geometry,
            bwin_font_t     font,
            bwin_t          parentWin = NULL
            ); /* optional */
    ~CPanelBuffers(void);

    eRet initialize(CModel * pModel, CConfig * pConfig);
    void uninitialize(void);
    void onClick(bwidget_t widget);
    void processNotification(CNotification & notification);
    void show(bool bShow);
    bool isPlaybackActive(void);
    void layoutBuffersMenu(void);
    eRet updateBuffersData(void);

protected:
    CWidgetLabel *    _pBuffersMenu;
    CWidgetLabel *    _Bkgnd;
    CWidgetLabel *    _LabelSeparator1;
    CWidgetProgress * _VideoMeter;
    CWidgetLabel *    _VideoLabel;
    CWidgetLabel *    _VideoBufferLabel;
    CWidgetLabel *    _VideoValue;
    CWidgetLabel *    _LabelSeparator2;
    CWidgetLabel *    _VideoPtsStcMinLabel;
    CWidgetLabel *    _VideoPtsStcMaxLabel;
    CWidgetLabel *    _VideoPtsStcLabel;
    CWidgetMeter *    _VideoPtsStcMeter;
    CWidgetProgress * _AudioMeter;
    CWidgetLabel *    _AudioLabel;
    CWidgetLabel *    _AudioBufferLabel;
    CWidgetLabel *    _AudioValue;
    CWidgetLabel *    _LabelSeparator3;
    CWidgetLabel *    _AudioPtsStcMinLabel;
    CWidgetLabel *    _AudioPtsStcMaxLabel;
    CWidgetLabel *    _AudioPtsStcLabel;
    CWidgetMeter *    _AudioPtsStcMeter;
    CWidgetLabel *    _PlaybackLabel;
    CWidgetProgress * _PlaybackMeter;
    CWidgetLabel *    _PlaybackBufferLabel;
    CWidgetLabel *    _PlaybackValue;
    CWidgetLabel *    _LabelSeparator4;
    CPlayback *       _pLastPlayback;
    CWidgetButton *   _Back;
    uint32_t          _graphicsWidth;
    uint32_t          _graphicsHeight;
    CTimer            _timerBuffers;
};

#ifdef __cplusplus
}
#endif

#endif /* ATLAS_PANEL_BUFFERS_H__ */