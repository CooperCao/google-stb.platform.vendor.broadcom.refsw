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

#include "convert.h"
#include "nexus_video_decoder.h"
#include "panel_buffers.h"
#include "audio_decode.h"

BDBG_MODULE(panel_buffers);

CPanelBuffers::CPanelBuffers(
        CWidgetEngine * pWidgetEngine,
        CScreenMain *   pScreenMain,
        CWidgetBase *   pParentWidget,
        MRect           geometry,
        bwin_font_t     font,
        bwin_t          parentWin
        ) :
    CPanel("CPanelBuffers", pWidgetEngine, pScreenMain, pParentWidget, geometry, font, parentWin),
    _pBuffersMenu(NULL),
    _Bkgnd(NULL),
    _LabelSeparator1(NULL),
    _VideoMeter(NULL),
    _VideoLabel(NULL),
    _VideoBufferLabel(NULL),
    _VideoValue(NULL),
    _LabelSeparator2(NULL),
    _VideoPtsStcMinLabel(NULL),
    _VideoPtsStcMaxLabel(NULL),
    _VideoPtsStcLabel(NULL),
    _VideoPtsStcMeter(NULL),
    _AudioMeter(NULL),
    _AudioLabel(NULL),
    _AudioBufferLabel(NULL),
    _AudioValue(NULL),
    _LabelSeparator3(NULL),
    _AudioPtsStcMinLabel(NULL),
    _AudioPtsStcMaxLabel(NULL),
    _AudioPtsStcLabel(NULL),
    _AudioPtsStcMeter(NULL),
    _PlaybackLabel(NULL),
    _PlaybackMeter(NULL),
    _PlaybackBufferLabel(NULL),
    _PlaybackValue(NULL),
    _LabelSeparator4(NULL),
    _pLastPlayback(NULL),
    _Back(NULL),
    _graphicsWidth(0),
    _graphicsHeight(0),
    _timerBuffers(pWidgetEngine, this, 333)
{
}

CPanelBuffers::~CPanelBuffers()
{
    uninitialize();
}

eRet CPanelBuffers::initialize(
        CModel *  pModel,
        CConfig * pConfig
        )
{
    eRet        ret       = eRet_Ok;
    CGraphics * pGraphics = NULL;
    bwin_font_t font10;
    MRect       rectMenu;
    MRect       rectBkgnd;
    MRect       rectLabel;
    MRect       rectValue;
    MRect       rectVideo;
    MRect       rectAudio;
    MRect       rectPlayback;
    MRect       rectPtsStcMin;
    MRect       rectPtsStcMax;
    MRect       rectPtsStc;
    MRect       rectMeter;
    int         marginBkgnd = 1;
    int         menuWidth   = 400;
    int         menuHeight  = 25;
    MRect       rectPanel;

    BDBG_ASSERT(NULL != pModel);
    BDBG_ASSERT(NULL != pConfig);

    setModel(pModel);
    setConfig(pConfig);

    pGraphics = _pModel->getGraphics();
    BDBG_ASSERT(NULL != pGraphics);

    font10 = pGraphics->getFont(10);
    BDBG_ASSERT(NULL != font10);

    _graphicsWidth  = GET_INT(_pCfg, GRAPHICS_SURFACE_WIDTH);
    _graphicsHeight = GET_INT(_pCfg, GRAPHICS_SURFACE_HEIGHT);

    /* set the size of the panel itself */
    rectPanel.set((_graphicsWidth / 2) - (menuWidth / 2), _graphicsHeight - (menuHeight / 2), menuWidth, menuHeight);
    rectPanel.setX(rectPanel.x() - marginBkgnd * 2);
    rectPanel.setY(rectPanel.y() - marginBkgnd * 2);
    rectPanel.grow(marginBkgnd * 2);
    setGeometry(rectPanel);

    /* start with rect sized to fit contents then expand for background border */
    rectMenu = rectPanel;
    rectMenu.setX(0);
    rectMenu.setY(0);
    _pBuffersMenu = new CWidgetLabel("CPanelBuffers::_pBuffersMenu", getEngine(), this, rectMenu, font10);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pBuffersMenu, ret, eRet_OutOfMemory, error);
    _pBuffersMenu->setBevel(0);
    _pBuffersMenu->setBackgroundColor(0xFF222222);
    _pBuffersMenu->setBackgroundFillMode(fill_eNone);
    _pBuffersMenu->show(true);

    /* shrink rect back to contents size */
    rectBkgnd = rectMenu;
    rectBkgnd.setX(marginBkgnd);
    rectBkgnd.setY(marginBkgnd);
    rectBkgnd.grow(marginBkgnd * -2);
    _Bkgnd = new CWidgetLabel("CPanelBuffers::_Bkgnd", getEngine(), _pBuffersMenu, rectBkgnd, font10);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _Bkgnd, ret, eRet_OutOfMemory, error);
    _Bkgnd->setBackgroundFillMode(fill_eGradient);
    _Bkgnd->setBackgroundGradient(COLOR_EGGSHELL +  2 * COLOR_STEP, COLOR_EGGSHELL, COLOR_EGGSHELL - 2 * COLOR_STEP);
    _Bkgnd->setBevel(1);

    {
        int menuBevel      = _Bkgnd->getBevel();
        int avLabelWidth   = 55;
        int progressHeight = 5;
        int labelWidth     = 50;
        int meterWidth     = 160;
        int x              = menuBevel;
        int y              = menuBevel * 2;

        /* video label */
        rectLabel.set(x, y, avLabelWidth, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _VideoLabel = new CWidgetLabel("CPanelBuffers::_VideoLabel", getEngine(), _Bkgnd, rectLabel, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _VideoLabel, ret, eRet_OutOfMemory, error);
        _VideoLabel->setText("Video:", bwidget_justify_horiz_left, bwidget_justify_vert_bottom);
        _VideoLabel->setBevel(0);
        _VideoLabel->setBackgroundFillMode(fill_eNone);

        x += avLabelWidth;

        /* separator */
        _LabelSeparator1 = new CWidgetLabel("CPanelBuffers::_LabelSeparator1", getEngine(), _Bkgnd, MRect(x, menuBevel, 1, rectBkgnd.height() * 3), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _LabelSeparator1, ret, eRet_OutOfMemory, error);
        _LabelSeparator1->setBevel(1);
        _LabelSeparator1->setBackgroundFillMode(fill_eNone);

        x += 1;

        /* video buffer data */
        rectLabel.set(x, y, labelWidth, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _VideoBufferLabel = new CWidgetLabel("CPanelBuffers::_VideoBuffersLabel", getEngine(), _Bkgnd, rectLabel, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _VideoBufferLabel, ret, eRet_OutOfMemory, error);
        _VideoBufferLabel->setText("Buffer", bwidget_justify_horiz_left, bwidget_justify_vert_bottom);
        _VideoBufferLabel->setBevel(0);
        _VideoBufferLabel->setBackgroundFillMode(fill_eNone);

        rectValue.set(x + labelWidth, y, 75, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _VideoValue = new CWidgetLabel("CPanelBuffers::_VideoValue", getEngine(), _Bkgnd, rectValue, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _VideoValue, ret, eRet_OutOfMemory, error);
        _VideoValue->setText("0/0K", bwidget_justify_horiz_right, bwidget_justify_vert_bottom);
        _VideoValue->setBevel(0);
        _VideoValue->setBackgroundFillMode(fill_eNone);

        rectVideo.set(x, y + rectLabel.height(), rectLabel.width() + rectValue.width(), progressHeight);
        _VideoMeter = new CWidgetProgress("CPanelBuffers::_VideoMeter", getEngine(), _Bkgnd, rectVideo, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _VideoMeter, ret, eRet_OutOfMemory, error);
        _VideoMeter->setLevel(0);
        _VideoMeter->setBevel(0);
        _VideoMeter->setRangeIndicator(true);
        _VideoMeter->showProgress(true);
        _VideoMeter->setBackgroundFillMode(fill_eNone);

        x               += rectLabel.width() + rectValue.width();
        _LabelSeparator2 = new CWidgetLabel("CPanelBuffers::_LabelSeparator2", getEngine(), _Bkgnd, MRect(x, menuBevel, 1, rectBkgnd.height()), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _LabelSeparator2, ret, eRet_OutOfMemory, error);
        _LabelSeparator2->setBevel(1);
        _LabelSeparator2->setBackgroundFillMode(fill_eNone);

        x += 1;

        /* video pts-stc */
        rectPtsStcMin.set(x, y, labelWidth, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _VideoPtsStcMinLabel = new CWidgetLabel("CPanelBuffers::_VideoPtsStcMinLabel", getEngine(), _Bkgnd, rectPtsStcMin, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _VideoPtsStcMinLabel, ret, eRet_OutOfMemory, error);
        _VideoPtsStcMinLabel->setText("-150ms", bwidget_justify_horiz_left, bwidget_justify_vert_bottom);
        _VideoPtsStcMinLabel->setBevel(0);
        _VideoPtsStcMinLabel->setBackgroundFillMode(fill_eNone);

        rectPtsStcMax.set(x + meterWidth - labelWidth, y, labelWidth, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _VideoPtsStcMaxLabel = new CWidgetLabel("CPanelBuffers::_VideoPtsStcMaxLabel", getEngine(), _Bkgnd, rectPtsStcMax, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _VideoPtsStcMaxLabel, ret, eRet_OutOfMemory, error);
        _VideoPtsStcMaxLabel->setText("+150ms", bwidget_justify_horiz_right, bwidget_justify_vert_bottom);
        _VideoPtsStcMaxLabel->setBevel(0);
        _VideoPtsStcMaxLabel->setBackgroundFillMode(fill_eNone);

        rectPtsStc.set(x + meterWidth / 2 - labelWidth / 2, y, labelWidth, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _VideoPtsStcLabel = new CWidgetLabel("CPanelBuffers::_VideoPtsStcLabel", getEngine(), _Bkgnd, rectPtsStc, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _VideoPtsStcLabel, ret, eRet_OutOfMemory, error);
        _VideoPtsStcLabel->setText("PTS-STC", bwidget_justify_horiz_center, bwidget_justify_vert_bottom);
        _VideoPtsStcLabel->setBevel(0);
        _VideoPtsStcLabel->setBackgroundFillMode(fill_eNone);

        rectMeter.set(x, y + rectLabel.height(), meterWidth, progressHeight);
        _VideoPtsStcMeter = new CWidgetMeter("CPanelBuffers::_VideoPtsStcMeter", getEngine(), _Bkgnd, rectMeter, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _VideoPtsStcMeter, ret, eRet_OutOfMemory, error);
        _VideoPtsStcMeter->setLevel(0);
        _VideoPtsStcMeter->setBevel(0);
        _VideoPtsStcMeter->setRangeIndicator(true);
        _VideoPtsStcMeter->setBackgroundFillMode(fill_eNone);
        _VideoPtsStcMeter->showMeter(true);

        x = menuBevel;
        y = rectLabel.y() + rectLabel.height() + menuBevel * 2;

        /* audio label */
        rectLabel.set(x, y, avLabelWidth, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _AudioLabel = new CWidgetLabel("CPanelBuffers::_AudioLabel", getEngine(), _Bkgnd, rectLabel, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _AudioLabel, ret, eRet_OutOfMemory, error);
        _AudioLabel->setText("Audio:", bwidget_justify_horiz_left, bwidget_justify_vert_bottom);
        _AudioLabel->setBevel(0);
        _AudioLabel->setBackgroundFillMode(fill_eNone);

        x += avLabelWidth + 1;

        /* audio buffer data */
        rectLabel.set(x, y, labelWidth, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _AudioBufferLabel = new CWidgetLabel("CPanelBuffers::_AudioBufferLabel", getEngine(), _Bkgnd, rectLabel, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _AudioBufferLabel, ret, eRet_OutOfMemory, error);
        _AudioBufferLabel->setText("Buffer", bwidget_justify_horiz_left, bwidget_justify_vert_bottom);
        _AudioBufferLabel->setBevel(0);
        _AudioBufferLabel->setBackgroundFillMode(fill_eNone);

        rectValue.set(x + labelWidth, y, 75, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _AudioValue = new CWidgetLabel("CPanelBuffers::_AudioValue", getEngine(), _Bkgnd, rectValue, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _AudioValue, ret, eRet_OutOfMemory, error);
        _AudioValue->setText("0/0K", bwidget_justify_horiz_right, bwidget_justify_vert_bottom);
        _AudioValue->setBevel(0);
        _AudioValue->setBackgroundFillMode(fill_eNone);

        rectAudio.set(x, y + rectLabel.height(), rectLabel.width() + rectValue.width(), progressHeight);
        _AudioMeter = new CWidgetProgress("CPanelBuffers::_AudioMeter", getEngine(), _Bkgnd, rectAudio, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _AudioMeter, ret, eRet_OutOfMemory, error);
        _AudioMeter->setLevel(0);
        _AudioMeter->setBevel(0);
        _AudioMeter->setRangeIndicator(true);
        _AudioMeter->showProgress(true);
        _AudioMeter->setBackgroundFillMode(fill_eNone);

        x               += rectLabel.width() + rectValue.width();
        _LabelSeparator3 = new CWidgetLabel("CPanelBuffers::_LabelSeparator3", getEngine(), _Bkgnd, MRect(x, y, 1, rectBkgnd.height()), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _LabelSeparator3, ret, eRet_OutOfMemory, error);
        _LabelSeparator3->setBevel(1);
        _LabelSeparator3->setBackgroundFillMode(fill_eNone);

        x += 1;

        /* audio pts-stc */
        rectPtsStcMin.set(x, y, labelWidth, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _AudioPtsStcMinLabel = new CWidgetLabel("CPanelBuffers::_AudioPtsStcMinLabel", getEngine(), _Bkgnd, rectPtsStcMin, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _AudioPtsStcMinLabel, ret, eRet_OutOfMemory, error);
        _AudioPtsStcMinLabel->setText("-150ms", bwidget_justify_horiz_left, bwidget_justify_vert_bottom);
        _AudioPtsStcMinLabel->setBevel(0);
        _AudioPtsStcMinLabel->setBackgroundFillMode(fill_eNone);

        rectPtsStcMax.set(x + meterWidth - labelWidth, y, labelWidth, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _AudioPtsStcMaxLabel = new CWidgetLabel("CPanelBuffers::_AudioPtsStcMaxLabel", getEngine(), _Bkgnd, rectPtsStcMax, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _AudioPtsStcMaxLabel, ret, eRet_OutOfMemory, error);
        _AudioPtsStcMaxLabel->setText("+150ms", bwidget_justify_horiz_right, bwidget_justify_vert_bottom);
        _AudioPtsStcMaxLabel->setBevel(0);
        _AudioPtsStcMaxLabel->setBackgroundFillMode(fill_eNone);

        rectPtsStc.set(x + meterWidth / 2 - labelWidth / 2, y, labelWidth, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _AudioPtsStcLabel = new CWidgetLabel("CPanelBuffers::_AudioPtsStcLabel", getEngine(), _Bkgnd, rectPtsStc, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _AudioPtsStcLabel, ret, eRet_OutOfMemory, error);
        _AudioPtsStcLabel->setText("PTS-STC", bwidget_justify_horiz_center, bwidget_justify_vert_bottom);
        _AudioPtsStcLabel->setBevel(0);
        _AudioPtsStcLabel->setBackgroundFillMode(fill_eNone);

        rectMeter.set(x, y + rectLabel.height(), meterWidth, progressHeight);
        _AudioPtsStcMeter = new CWidgetMeter("CPanelBuffers::_AudioPtsStcMeter", getEngine(), _Bkgnd, rectMeter, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _AudioPtsStcMeter, ret, eRet_OutOfMemory, error);
        _AudioPtsStcMeter->setLevel(0);
        _AudioPtsStcMeter->setBevel(0);
        _AudioPtsStcMeter->setRangeIndicator(true);
        _AudioPtsStcMeter->setBackgroundFillMode(fill_eNone);
        _AudioPtsStcMeter->showMeter(true);

        x = menuBevel;
        y = rectLabel.y() + rectLabel.height() + menuBevel * 2;

        /* playback label */
        rectLabel.set(x, y, avLabelWidth, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _PlaybackLabel = new CWidgetLabel("CPanelBuffers::_PlaybackLabel", getEngine(), _Bkgnd, rectLabel, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _PlaybackLabel, ret, eRet_OutOfMemory, error);
        _PlaybackLabel->setText("Playback:", bwidget_justify_horiz_left, bwidget_justify_vert_bottom);
        _PlaybackLabel->setBevel(0);
        _PlaybackLabel->setBackgroundFillMode(fill_eNone);

        x += avLabelWidth + 1;

        /* playback buffer data */
        rectLabel.set(x, y, labelWidth, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _PlaybackBufferLabel = new CWidgetLabel("CPanelBuffers::_PlaybackBufferLabel", getEngine(), _Bkgnd, rectLabel, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _PlaybackBufferLabel, ret, eRet_OutOfMemory, error);
        _PlaybackBufferLabel->setText("Buffer", bwidget_justify_horiz_left, bwidget_justify_vert_bottom);
        _PlaybackBufferLabel->setBevel(0);
        _PlaybackBufferLabel->setBackgroundFillMode(fill_eNone);

        rectValue.set(x + labelWidth, y, 75, rectBkgnd.height() - menuBevel * 2 - progressHeight);
        _PlaybackValue = new CWidgetLabel("CPanelBuffers::_PlaybackValue", getEngine(), _Bkgnd, rectValue, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _PlaybackValue, ret, eRet_OutOfMemory, error);
        _PlaybackValue->setText("0/0K", bwidget_justify_horiz_right, bwidget_justify_vert_bottom);
        _PlaybackValue->setBevel(0);
        _PlaybackValue->setBackgroundFillMode(fill_eNone);

        rectPlayback.set(x, y + rectLabel.height(), rectLabel.width() + rectValue.width(), progressHeight);
        _PlaybackMeter = new CWidgetProgress("CPanelBuffers::_PlaybackMeter", getEngine(), _Bkgnd, rectPlayback, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _PlaybackMeter, ret, eRet_OutOfMemory, error);
        _PlaybackMeter->setLevel(0);
        _PlaybackMeter->setBevel(0);
        _PlaybackMeter->setRangeIndicator(true);
        _PlaybackMeter->setBackgroundFillMode(fill_eNone);
        _PlaybackMeter->showProgress(true);

        x               += rectLabel.width() + rectValue.width();
        _LabelSeparator4 = new CWidgetLabel("CPanelBuffers::_LabelSeparator4", getEngine(), _Bkgnd, MRect(x, y, 1, rectBkgnd.height()), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _LabelSeparator4, ret, eRet_OutOfMemory, error);
        _LabelSeparator4->setBevel(1);
        _LabelSeparator4->setBackgroundFillMode(fill_eNone);

        /* adjust overall menu to fit contents */
        layoutBuffersMenu();
    }

    goto done;
error:
    uninitialize();
done:
    return(ret);
} /* initialize */

void CPanelBuffers::uninitialize()
{
    _pLastPlayback = NULL;

    DEL(_Back);
    DEL(_LabelSeparator4);
    DEL(_PlaybackMeter);
    DEL(_PlaybackValue);
    DEL(_PlaybackBufferLabel);
    DEL(_PlaybackLabel);
    DEL(_AudioPtsStcMeter);
    DEL(_AudioPtsStcLabel);
    DEL(_AudioPtsStcMaxLabel);
    DEL(_AudioPtsStcMinLabel);
    DEL(_LabelSeparator3);
    DEL(_AudioMeter);
    DEL(_AudioValue);
    DEL(_AudioBufferLabel);
    DEL(_AudioLabel);
    DEL(_VideoPtsStcMeter);
    DEL(_VideoPtsStcLabel);
    DEL(_VideoPtsStcMaxLabel);
    DEL(_VideoPtsStcMinLabel);
    DEL(_LabelSeparator2);
    DEL(_VideoMeter);
    DEL(_VideoValue);
    DEL(_VideoBufferLabel);
    DEL(_LabelSeparator1);
    DEL(_VideoLabel);
    DEL(_Bkgnd);
    DEL(_pBuffersMenu);
} /* uninitialize */

bool CPanelBuffers::isPlaybackActive()
{
    bool        bActive   = false;
    CPlayback * pPlayback = _pModel->getPlayback();

    if (NULL != pPlayback)
    {
        bActive = pPlayback->isActive();
    }

    return(bActive);
}

void CPanelBuffers::layoutBuffersMenu()
{
    MRect rectMenu;
    MRect rectVideoTitle  = _VideoLabel->getGeometry();
    MRect rectVideoMeter  = _VideoMeter->getGeometry();
    MRect rectVideoPtsStc = _VideoPtsStcMeter->getGeometry();
    MRect rectBkgnd;
    int   menuBevel   = _Bkgnd->getBevel();
    int   marginBkgnd = 1;
    int   width       = rectVideoTitle.width() + rectVideoMeter.width() + rectVideoPtsStc.width() + menuBevel * 2 + 4;

    /* assume geometry where playback buffer data is available */
    int y      = 100;
    int height = 23 * 3 + 1;

    if (isPlaybackActive())
    {
        y += 30; /* account for playback timeline */
        /* playback data is available */
        _PlaybackLabel->show(true);
        _PlaybackBufferLabel->show(true);
        _PlaybackValue->show(true);
        _PlaybackMeter->show(true);
    }
    else
    {
        /* adjust for no playback buffer data */
        y      -= 10;
        height -= 20;

        _PlaybackLabel->show(false);
        _PlaybackBufferLabel->show(false);
        _PlaybackValue->show(false);
        _PlaybackMeter->show(false);
    }

    /* resize window to fit contents */
    rectMenu.set((_graphicsWidth / 2) - (width + marginBkgnd * 2) / 2,
            _graphicsHeight - y - marginBkgnd * 2,
            width + marginBkgnd * 2,
            height + marginBkgnd * 2);
    setGeometry(rectMenu);

    _pBuffersMenu->setGeometry(MRect(0, 0, rectMenu.width(), rectMenu.height()));
    rectBkgnd.set(marginBkgnd, marginBkgnd, width, height);
    _Bkgnd->setGeometry(rectBkgnd);
} /* layoutBuffersMenu */

void CPanelBuffers::onClick(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }
}

void CPanelBuffers::show(bool bShow)
{
    /* avoid CPanel's "restore last focus" feature since
     * there is no focusable widgets in this panel */
    CWidgetLabel::show(bShow);

    if (true == bShow)
    {
        layoutBuffersMenu();
        _timerBuffers.start(GET_INT(_pCfg, UI_BUFFERS_UPDATE_TIMEOUT));
    }
}

void CPanelBuffers::processNotification(CNotification & notification)
{
    switch (notification.getId())
    {
    case eNotify_Timeout:
    {
        CTimer * pTimer = (CTimer *)notification.getData();
        if (&_timerBuffers == pTimer)
        {
            if (_pBuffersMenu->isVisible())
            {
                CPlayback * pPlayback = _pModel->getPlayback();

                if (_pLastPlayback != pPlayback)
                {
                    /* playback changed so layout menu */
                    layoutBuffersMenu();
                    _pLastPlayback = pPlayback;
                }

                updateBuffersData();
                _timerBuffers.start();
            }
        }
    }
    break;

    case eNotify_PlaybackStateChanged:
        if (_pBuffersMenu->isVisible())
        {
            layoutBuffersMenu();
        }
        break;

    default:
        break;
    } /* switch */

    return;
} /* processNotification */

#define CALC_BUF_LEVEL(BUF, LEVEL, NSTATUS)                \
    if (1024 * 1024 > NSTATUS.fifoDepth) {                 \
        snprintf(BUF, 32, "%luK/%luK",                     \
                (unsigned long)NSTATUS.fifoDepth / 1024,   \
                (unsigned long)NSTATUS.fifoSize / 1024); } \
    else {                                                 \
        snprintf(BUF, 32, "%0.1fM/%0.1fM",                 \
                (NSTATUS.fifoDepth / 1024.0) / 1024,       \
                (NSTATUS.fifoSize / 1024.0) / 1024); }     \
    LEVEL = 65535.0 * NSTATUS.fifoDepth / NSTATUS.fifoSize

eRet CPanelBuffers::updateBuffersData()
{
    eRet                     ret          = eRet_Ok;
    CSimpleVideoDecode *     pVideoDecode = _pModel->getSimpleVideoDecode();
    CSimpleAudioDecode *     pAudioDecode = _pModel->getSimpleAudioDecode();
    CPlayback *              pPlayback    = _pModel->getPlayback();
    NEXUS_VideoDecoderStatus nVideoStatus;
    NEXUS_AudioDecoderStatus nAudioStatus;
    NEXUS_PlaybackStatus     nPlaybackStatus;
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    char                     buf[32];
    unsigned                 level = 0;

    ret = pVideoDecode->getStatus(&nVideoStatus);
    CHECK_ERROR_GOTO("unable to retrieve video decode status", ret, error);

    CALC_BUF_LEVEL(buf, level, nVideoStatus);
    _VideoValue->setText(buf);
    _VideoMeter->setLevel(level);

    ret = pAudioDecode->getStatus(&nAudioStatus);
    CHECK_ERROR_GOTO("getting audio decoder status failed!", ret, error);

    CALC_BUF_LEVEL(buf, level, nAudioStatus);
    _AudioValue->setText(buf);
    _AudioMeter->setLevel(level);

    strncpy(buf, "0/0K", 32);
    level = 0;
    if (NULL != pPlayback)
    {
        nerror = NEXUS_Playback_GetStatus(pPlayback->getPlayback(), &nPlaybackStatus);
        CHECK_NEXUS_ERROR_ASSERT("getting playback status failed!", nerror);

        CALC_BUF_LEVEL(buf, level, nPlaybackStatus);
    }
    _PlaybackValue->setText(buf);
    _PlaybackMeter->setLevel(level);

    /* update pts-stc meters */
    {
        int diff = nVideoStatus.ptsStcDifference / 45; /* convert to msec */
        if (nVideoStatus.ptsStcDifference < 0)
        {
            _VideoPtsStcMeter->setLevel(MAX(diff, -150) * 32767 / 150);
        }
        else
        {
            _VideoPtsStcMeter->setLevel(MIN(diff, 150) * 32767 / 150);
        }

        diff = nAudioStatus.ptsStcDifference / 45; /* convert to msec */
        if (nAudioStatus.ptsStcDifference < 0)
        {
            _AudioPtsStcMeter->setLevel(MAX(diff, -150) * 32767 / 150);
        }
        else
        {
            _AudioPtsStcMeter->setLevel(MIN(diff, 150) * 32767 / 150);
        }
    }

error:
    return(ret);
} /* updateBuffersData */