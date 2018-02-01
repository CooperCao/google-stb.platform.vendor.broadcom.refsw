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

#include "atlas.h"
#include "atlas_os.h"
#include "screen_main.h"
#include "channel.h"
#include "channelmgr.h"
#include "power.h"
#include "panel_power.h"
#include "timer.h"
#include "remote.h"
#include "video_window.h"
#include "playlist.h"
#include "playlist_db.h"
#if DVR_LIB_SUPPORT
#include "tsb.h"
#include "dvrmgr.h"
#endif
#ifdef CPUTEST_SUPPORT
#include "cputest.h"
#endif
#ifdef WPA_SUPPLICANT_SUPPORT
#include "wifi.h"
#endif

#include "widget_edit.h"
#include "widget_check_button.h"

BDBG_MODULE(atlas_screen_main);

CScreenMain::CScreenMain(
        CWidgetEngine * pWidgetEngine,
        CConfig *       pConfig,
        CWidgetBase *   pParent
        ) :
    CScreen("CScreenMain", pWidgetEngine, pConfig, pParent),
    _pLabelScan(NULL),
    _pLabelVolume(NULL),
#if BDSP_MS12_SUPPORT
    _pLabelAudioFadeMain(NULL),
    _pLabelAudioFadePip(NULL),
#endif
    _rectChannelNum(0, 0, 0, 0),
    _pLabelPip(NULL),
    _pLabelChannelNum(NULL),
    _pLabelChannelNumText(NULL),
    _pLabelChannelType(NULL),
    _pProgressPlayback(NULL),
    _pLabelPlaybackState(NULL),
    _pLabelPlaybackLength(NULL),
    _pLabelConnectionType(NULL),
    _pLabelConnectionStatus(NULL),
    _pLabelPlaybackName(NULL),
    _pLabelPlaybackNameShadow(NULL),
    _pLabelDebugName(NULL),
    _pLabelDebugNameShadow(NULL),
    _pMainMenu(NULL),
    _Display(NULL),
    _Decode(NULL),
    _Playback(NULL),
    _Audio(NULL),
#if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT)
    _Network(NULL),
#endif
#ifdef NETAPP_SUPPORT
    _Bluetooth(NULL),
#endif
    _Streaming(NULL),
    _rectChText(0, 0, 0, 0),
    _rectChType(0, 0, 0, 0),
    _Buffers(NULL),
#ifdef CPUTEST_SUPPORT
    _CpuTest(NULL),
    _CpuTestLabel(NULL),
    _CpuTestPopup(NULL),
    _pCpuTest(NULL),
#endif /* ifdef CPUTEST_SUPPORT */
#if POWERSTANDBY_SUPPORT
    _Power(NULL),
#endif
    _Back(NULL),
    _pDecodeMenu(NULL),
    _pDisplayMenu(NULL),
    _pAudioMenu(NULL),
    _pPlaybackMenu(NULL),
    _pRecordMenu(NULL),
    _pBuffersMenu(NULL),
#if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT)
    _pNetworkMenu(NULL),
#endif
#ifdef NETAPP_SUPPORT
    _pBluetoothMenu(NULL),
#endif
#ifdef PLAYBACK_IP_SUPPORT
    _pStreamingMenu(NULL),
#endif
    _timerConnectionStatus(pWidgetEngine, this, 3000),
    _timerVolume(pWidgetEngine, this, 1000),
    _timerChannel(pWidgetEngine, this, 3000),
    _timerBuffersUpdate(pWidgetEngine, this, 333),
#ifdef CPUTEST_SUPPORT
    _timerCpuTest(pWidgetEngine, this, 1000),
#endif
#if BDSP_MS12_SUPPORT
    _timerAudioFade(pWidgetEngine, this, 100),
    _timerAudioFadeShow(pWidgetEngine, this, 1500),
#endif
    _channelsFound(0),
    _MsgBox(NULL),
    _pPowerMenu(NULL),
    _pKeyboard(NULL),
    _pVbiMenu(NULL),
#if BDSP_MS12_SUPPORT
    _pAudioAc4Menu(NULL),
#endif
    _timerMsgBox(pWidgetEngine, this, 7000)
#if NEXUS_HAS_FRONTEND
    , _Tuner(NULL),
    _ScanQam(NULL),
    _ScanVsb(NULL),
    _ScanSat(NULL),
    _ScanOfdm(NULL),
    _pTunerMenu(NULL),
    _pScanQamMenu(NULL),
    _pScanVsbMenu(NULL),
    _pScanSatMenu(NULL),
    _pScanOfdmMenu(NULL)
#endif /* if NEXUS_HAS_FRONTEND */
#ifdef MPOD_SUPPORT
    , _CableCard(NULL),
    _pCableCardMenu(NULL)
#endif
{
    /* set default z order */
    setZOrder(16);
    for (int i = 0; i < eWindowType_Max; i++)
    {
        _pCurrentPlmLabel[i] = NULL;
    }
}

#define SET_TRANSPARENCY(color, trans)  ((color & 0x00FFFFFF) | trans)

eRet CScreenMain::initialize(CModel * pModel)
{
    eRet        ret       = eRet_Ok;
    CGraphics * pGraphics = NULL;
    bwin_font_t font10;
    bwin_font_t font12;
    bwin_font_t font14;
    bwin_font_t font17;
    uint32_t    graphicsWidth  = 0;
    uint32_t    graphicsHeight = 0;
    uint32_t    transparency   = 0xCC000000;
    uint32_t    menuHeight     = 0;

#if NEXUS_HAS_FRONTEND
    bool tuner = false;
#endif
    CScreen::initialize(pModel); /* call base class */

    pGraphics = _pModel->getGraphics();
    BDBG_ASSERT(NULL != pGraphics);

    graphicsWidth  = GET_INT(_pCfg, GRAPHICS_SURFACE_WIDTH);
    graphicsHeight = GET_INT(_pCfg, GRAPHICS_SURFACE_HEIGHT);

    font10 = pGraphics->getFont(10);
    BDBG_ASSERT(NULL != font10);
    font12 = pGraphics->getFont(12);
    BDBG_ASSERT(NULL != font12);
    font14 = pGraphics->getFont(14);
    BDBG_ASSERT(NULL != font14);
    font17 = pGraphics->getFont(17);
    BDBG_ASSERT(NULL != font17);

    /* PiP - percent and border are 0-1000 = 0-100.0% */
    {
        MRect    rectScaled;
        uint8_t  area      = GET_INT(_pCfg, PIP_POSITION);
        uint8_t  percent   = GET_INT(_pCfg, PIP_PERCENTAGE);
        uint8_t  border    = GET_INT(_pCfg, PIP_BORDER_PERCENTAGE);
        unsigned borderGap = 0;

        borderGap = border * graphicsHeight / 1000;

        rectScaled.setWidth(graphicsWidth * percent / 1000);
        rectScaled.setHeight(graphicsHeight * percent / 1000);

        switch (area)
        {
        case eWinArea_LowerLeft:
            rectScaled.setX(borderGap);
            rectScaled.setY(graphicsHeight - borderGap - rectScaled.height());
            break;

        case eWinArea_LowerRight:
            rectScaled.setX(graphicsWidth - borderGap - rectScaled.width());
            rectScaled.setY(graphicsHeight - borderGap - rectScaled.height());
            break;

        case eWinArea_UpperLeft:
            rectScaled.setX(borderGap);
            rectScaled.setY(borderGap);
            break;

        case eWinArea_UpperRight:
        default:
            rectScaled.setX(graphicsWidth - borderGap - rectScaled.width());
            rectScaled.setY(borderGap);
            break;
        } /* switch */

        _pLabelPip = new CWidgetLabel("CScreenMain::_pLabelPip", getEngine(), this, rectScaled, font14);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelPip, ret, eRet_OutOfMemory, error);
        _pLabelPip->setBackgroundColor(0xFF222222);
        _pLabelPip->setText("Picture-in-Picture", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
        _pLabelPip->setBevel(1);
        /* keep pip graphics below all other graphics */
        _pLabelPip->setZOrder(0);
        _pLabelPip->show(false);
    }

    /* channel banner */
    {
        const unsigned heightChNum  = 25;
        const unsigned heightChType = 15;
        const unsigned bevel        = 2;

        /* channel num/type/records CONTAINER object */
        _rectChannelNum.set(graphicsWidth - 210, 50, 150, heightChNum + heightChType + (bevel * 2));
        _pLabelChannelNum = new CWidgetLabel("CScreenMain::_pLabelChannelNum", getEngine(), this, _rectChannelNum, font17);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelChannelNum, ret, eRet_OutOfMemory, error);
        _pLabelChannelNum->setBackgroundColor(0xFF222222);
        _pLabelChannelNum->setBevel(bevel);
        _pLabelChannelNum->show(false);

        /*
         * top half of channel banner
         * channel NUMBER text widget
         */
        _rectChText = _rectChannelNum;
        _rectChText.setX(_pLabelChannelNum->getBevel());
        _rectChText.setY(_pLabelChannelNum->getBevel());
        _rectChText.grow(_pLabelChannelNum->getBevel() * -2);
        _rectChText.setHeight(heightChNum);
        _pLabelChannelNumText = new CWidgetLabel("CScreenMain::_pLabelChannelNumText", getEngine(), _pLabelChannelNum, _rectChText, font17);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelChannelNumText, ret, eRet_OutOfMemory, error);
        _pLabelChannelNumText->setBevel(0);
        _pLabelChannelNumText->setBackgroundFillMode(fill_eGradient);
        _pLabelChannelNumText->setBackgroundGradient(COLOR_EGGSHELL + 2 * COLOR_STEP, COLOR_EGGSHELL, COLOR_EGGSHELL - 4 * COLOR_STEP);
        _pLabelChannelNumText->setText("0.0", bwidget_justify_horiz_center, bwidget_justify_vert_top);
        _pLabelChannelNumText->setTextColor(0xFF222222);

        /* bottom half of channel banner */
        {
            /* channel TYPE text widget */
            _rectChType = _rectChText;
            _rectChType.setHeight(heightChType);
            _rectChType.setY(_rectChText.y() + heightChNum);

            _pLabelChannelType = new CWidgetLabel("CScreenMain::_pLabelChannelType", getEngine(), _pLabelChannelNum, _rectChType, font10);
            CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelChannelType, ret, eRet_OutOfMemory, error);
            _pLabelChannelType->setBevel(0);
            _pLabelChannelType->setBackgroundFillMode(fill_eGradient);
            _pLabelChannelType->setBackgroundGradient(SET_TRANSPARENCY((0xFF676767 + 2 * COLOR_STEP), transparency),
                    SET_TRANSPARENCY(0xFF676767, transparency),
                    SET_TRANSPARENCY((0xFF676767 - 3 * COLOR_STEP), transparency));
            _pLabelChannelType->setTextColor(COLOR_EGGSHELL);
            _pLabelChannelType->setText("TYPE", bwidget_justify_horiz_center, bwidget_justify_vert_top);

            MRect rectChTypeText = _rectChType;
            rectChTypeText.setX(0);
            rectChTypeText.setY(0);

            /* playback progress meter and playback state/length overlay labels */
            {
                /* playback progress meter */
                _pProgressPlayback = new CWidgetProgress("CScreenMain::_pProgressPlayback", getEngine(), _pLabelChannelType, rectChTypeText, font10);
                CHECK_PTR_ERROR_GOTO("unable to allocate progress widget", _pProgressPlayback, ret, eRet_OutOfMemory, error);
                _pProgressPlayback->setLevel(0);
                _pProgressPlayback->setBevel(0);
                _pProgressPlayback->setBackgroundFillMode(fill_eGradient);
                _pProgressPlayback->setBackgroundGradient(SET_TRANSPARENCY((0xFF676767 - 3 * COLOR_STEP), transparency),
                        SET_TRANSPARENCY(0xFF676767, transparency),
                        SET_TRANSPARENCY((0xFF676767 + 2 * COLOR_STEP), transparency));
                _pProgressPlayback->setTextColor(COLOR_EGGSHELL);
                _pProgressPlayback->showProgress(true);
                _pProgressPlayback->show(false);

                /* add two labels on top of progress bar: one for playback state and one for playback length */

                /* playback state */
                uint32_t spacer            = 2;
                MRect    rectPlaybackState = rectChTypeText;
                rectPlaybackState.setX(rectPlaybackState.x() + spacer);
                rectPlaybackState.setWidth(rectChTypeText.width() * 6 / 10 - spacer);

                _pLabelPlaybackState = new CWidgetLabel("CScreenMain::_pLabelPlaybackState", getEngine(), _pProgressPlayback, rectPlaybackState, font10);
                CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelPlaybackState, ret, eRet_OutOfMemory, error);
                _pLabelPlaybackState->setBevel(0);
                _pLabelPlaybackState->setBackgroundFillMode(fill_eNone); /* no background so text overlays progress */
                _pLabelPlaybackState->setTextColor(COLOR_EGGSHELL);
                _pLabelPlaybackState->setText("Play State", bwidget_justify_horiz_left, bwidget_justify_vert_middle);

                /* playback length */
                MRect rectPlaybackLength = rectPlaybackState;
                rectPlaybackLength.setX(rectPlaybackState.x() + rectPlaybackState.width() - spacer);
                rectPlaybackLength.setWidth(rectChTypeText.width() - rectPlaybackState.width());

                _pLabelPlaybackLength = new CWidgetLabel("CScreenMain::_pLabelPlaybackLength", getEngine(), _pProgressPlayback, rectPlaybackLength, font10);
                CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelPlaybackLength, ret, eRet_OutOfMemory, error);
                _pLabelPlaybackLength->setBevel(0);
                _pLabelPlaybackLength->setBackgroundFillMode(fill_eNone); /* no background so text overlays progress */
                _pLabelPlaybackLength->setTextColor(COLOR_EGGSHELL);
                _pLabelPlaybackLength->setText("00:00:00", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
            }
        }
    }

    /* atlas server connection status widget */
    {
        /* place it above buffers panel */
        MRect rectConnectionStatus((graphicsWidth / 2) - 200, (2 * graphicsHeight / 3), 110, 25);

        _pLabelConnectionType = new CWidgetLabel("CScreenMain::_pLabelConnectionType", getEngine(), this, rectConnectionStatus, font14);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelConnectionType, ret, eRet_OutOfMemory, error);
        _pLabelConnectionType->setText("Connection Type", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
        _pLabelConnectionType->setBevel(0);
        _pLabelConnectionType->setBackgroundFillMode(fill_eGradient);
        _pLabelConnectionType->setBackgroundGradient(COLOR_GREY_DARK + 4 * COLOR_STEP, COLOR_GREY_DARK, COLOR_GREY_DARK - 1 * COLOR_STEP);
        _pLabelConnectionType->setTextColor(COLOR_EGGSHELL);
        _pLabelConnectionType->show(false);

        rectConnectionStatus.setX(rectConnectionStatus.x() + rectConnectionStatus.width());
        rectConnectionStatus.setWidth(290);
        _pLabelConnectionStatus = new CWidgetLabel("CScreenMain::_pLabelConnectionStatus", getEngine(), this, rectConnectionStatus, font14);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelConnectionStatus, ret, eRet_OutOfMemory, error);
        _pLabelConnectionStatus->setText("Connection Status", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        _pLabelConnectionStatus->setBevel(0);
        _pLabelConnectionStatus->setBackgroundFillMode(fill_eGradient);
        _pLabelConnectionStatus->setBackgroundGradient(
                SET_TRANSPARENCY((COLOR_GREY_DARK + 4 * COLOR_STEP), transparency),
                SET_TRANSPARENCY(COLOR_GREY_DARK, transparency),
                SET_TRANSPARENCY((COLOR_GREY_DARK - 1 * COLOR_STEP), transparency));
        _pLabelConnectionStatus->setTextColor(COLOR_EGGSHELL);
        _pLabelConnectionStatus->show(false);
    }

    /* playback file name widget */
    {
        MRect rectPlaybackName(50, graphicsHeight - 60, graphicsWidth - 100, 25);

        _pLabelPlaybackName = new CWidgetLabel("CScreenMain::_pLabelPlaybackName", getEngine(), this, rectPlaybackName, font14);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelPlaybackName, ret, eRet_OutOfMemory, error);
        _pLabelPlaybackName->setBevel(0);
        _pLabelPlaybackName->setBackgroundFillMode(fill_eNone); /* no background so text overlays */
        _pLabelPlaybackName->setTextColor(COLOR_EGGSHELL);
        _pLabelPlaybackName->setText("Now Playing:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        _pLabelPlaybackName->show(false);

        rectPlaybackName.moveBy(1, 1);

        _pLabelPlaybackNameShadow = new CWidgetLabel("CScreenMain::_pLabelPlaybackNameShadow", getEngine(), this, rectPlaybackName, font14);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelPlaybackNameShadow, ret, eRet_OutOfMemory, error);
        _pLabelPlaybackNameShadow->setBevel(0);
        _pLabelPlaybackNameShadow->setBackgroundFillMode(fill_eNone); /* no background so text overlays */
        _pLabelPlaybackNameShadow->setTextColor(COLOR_BLACK);
        _pLabelPlaybackNameShadow->setText("Now Playing:", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
        _pLabelPlaybackNameShadow->show(false);
    }

    /* debug widget */
    {
        MRect rectDebugName(50, graphicsHeight - 60, graphicsWidth - 100, 25);

        _pLabelDebugName = new CWidgetLabel("CScreenMain::_pLabelDebugName", getEngine(), this, rectDebugName, font14);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelDebugName, ret, eRet_OutOfMemory, error);
        _pLabelDebugName->setBevel(0);
        _pLabelDebugName->setBackgroundFillMode(fill_eNone); /* no background so text overlays */
        _pLabelDebugName->setTextColor(COLOR_YELLOW);
        _pLabelDebugName->setText("Lua Debug:", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
        _pLabelDebugName->show(false);

        rectDebugName.moveBy(1, 1);

        _pLabelDebugNameShadow = new CWidgetLabel("CScreenMain::_pLabelDebugNameShadow", getEngine(), this, rectDebugName, font14);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelDebugNameShadow, ret, eRet_OutOfMemory, error);
        _pLabelDebugNameShadow->setBevel(0);
        _pLabelDebugNameShadow->setBackgroundFillMode(fill_eNone); /* no background so text overlays */
        _pLabelDebugNameShadow->setTextColor(COLOR_BLACK);
        _pLabelDebugNameShadow->setText("Lua Debug:", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
        _pLabelDebugNameShadow->show(false);
    }

    /* scan status widget */
    _pLabelScan = new CWidgetProgress("CScreenMain::_pLabelScan", getEngine(), this, MRect((graphicsWidth / 2) - 200, graphicsHeight - 100, 400, 25), font14);
    CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelScan, ret, eRet_OutOfMemory, error);
    _pLabelScan->setText("No Scan", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
    _pLabelScan->setBevel(2);
    _pLabelScan->setBackgroundFillMode(fill_eGradient);
    _pLabelScan->setBackgroundGradient(SET_TRANSPARENCY((0xFF676767 - 5 * COLOR_STEP), transparency),
            SET_TRANSPARENCY(0xFF676767, transparency),
            SET_TRANSPARENCY((0xFF676767 + 2 * COLOR_STEP), transparency));
    _pLabelScan->setTextColor(COLOR_EGGSHELL);
    _pLabelScan->show(false);

    /* volume widget */
    _pLabelVolume = new CWidgetProgress("CScreenMain::_pLabelVolume", getEngine(), this, MRect((graphicsWidth / 2) - 200, graphicsHeight - 100, 400, 25), font14);
    CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelVolume, ret, eRet_OutOfMemory, error);
    _pLabelVolume->setText("No Volume", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
    _pLabelVolume->setBevel(2);
    _pLabelVolume->setBackgroundFillMode(fill_eGradient);
    _pLabelVolume->setBackgroundGradient(SET_TRANSPARENCY((0xFF676767 - 5 * COLOR_STEP), transparency),
            SET_TRANSPARENCY(0xFF676767, transparency),
            SET_TRANSPARENCY((0xFF676767 + 2 * COLOR_STEP), transparency));
    _pLabelVolume->setTextColor(COLOR_EGGSHELL);
    /* keep volume popup above all other graphics */
    _pLabelVolume->setZOrder(32);
    _pLabelVolume->show(false);

#if BDSP_MS12_SUPPORT
    /* main audio fade widget */
    _pLabelAudioFadeMain = new CWidgetProgress("CScreenMain::_pLabelAudioFadeMain", getEngine(), this, MRect(50, graphicsHeight - 103, 200, 20), font14);
    CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelAudioFadeMain, ret, eRet_OutOfMemory, error);
    _pLabelAudioFadeMain->setText("Main Audio Fade", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
    _pLabelAudioFadeMain->setBevel(1);
    _pLabelAudioFadeMain->setBackgroundFillMode(fill_eGradient);
    _pLabelAudioFadeMain->setBackgroundGradient(SET_TRANSPARENCY((0xFF676767 - 5 * COLOR_STEP), transparency),
            SET_TRANSPARENCY(0xFF676767, transparency),
            SET_TRANSPARENCY((0xFF676767 + 2 * COLOR_STEP), transparency));
    _pLabelAudioFadeMain->setTextColor(COLOR_EGGSHELL);
    /* keep AudioFadeMain popup above all other graphics */
    _pLabelAudioFadeMain->setZOrder(32);
    _pLabelAudioFadeMain->show(false);

    /* Pip audio fade widget */
    _pLabelAudioFadePip = new CWidgetProgress("CScreenMain::_pLabelAudioFadePip", getEngine(), this, MRect(50, graphicsHeight - 80, 200, 20), font14);
    CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _pLabelAudioFadePip, ret, eRet_OutOfMemory, error);
    _pLabelAudioFadePip->setText("Pip Audio Fade", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
    _pLabelAudioFadePip->setBevel(1);
    _pLabelAudioFadePip->setBackgroundFillMode(fill_eGradient);
    _pLabelAudioFadePip->setBackgroundGradient(SET_TRANSPARENCY((0xFF676767 - 5 * COLOR_STEP), transparency),
            SET_TRANSPARENCY(0xFF676767, transparency),
            SET_TRANSPARENCY((0xFF676767 + 2 * COLOR_STEP), transparency));
    _pLabelAudioFadePip->setTextColor(COLOR_EGGSHELL);
    /* keep AudioFadePip popup above all other graphics */
    _pLabelAudioFadePip->setZOrder(32);
    _pLabelAudioFadePip->show(false);
#endif

    /* main menu */
    menuHeight = 151;
#ifdef CPUTEST_SUPPORT
    menuHeight += 20;
#endif
    _pMainMenu = new CWidgetMenu("CScreenMain::_pMainMenu", getEngine(), this, MRect(50, 50, 185, menuHeight), font14, font12);
    CHECK_PTR_ERROR_GOTO("unable to allocate menu widget", _pMainMenu, ret, eRet_OutOfMemory, error);
    _pMainMenu->setMenuTitle("Atlas", NULL);
    _pMainMenu->showSubTitle(false);
    _pMainMenu->setScroll(false);
    _pMainMenu->show(false);
    {
        _Decode = new CWidgetButton("CScreenMain::_Decode", getEngine(), this, MRect(0, 0, 0, 22), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _Decode, ret, eRet_OutOfMemory, error);
        _Decode->setText("Decode...", bwidget_justify_horiz_left);
        _pMainMenu->addButton(_Decode, "Decode");

        _Display = new CWidgetButton("CScreenMain::_Display", getEngine(), this, MRect(0, 0, 0, 22), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", _Display, ret, eRet_OutOfMemory, error);
        _Display->setText("Display...", bwidget_justify_horiz_left);
        _pMainMenu->addButton(_Display, "Display");

        _Playback = new CWidgetButton("CScreenMain::_Playback", getEngine(), this, MRect(0, 0, 0, 22), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Playback, ret, eRet_OutOfMemory, error);
        _Playback->setText("Playback...", bwidget_justify_horiz_left);
        _pMainMenu->addButton(_Playback, "Playback");

#ifdef PLAYBACK_IP_SUPPORT
        _Streaming = new CWidgetButton("CScreenMain::_Streaming", getEngine(), this, MRect(0, 0, 0, 22), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Streaming, ret, eRet_OutOfMemory, error);
        _Streaming->setText("Streaming Player...", bwidget_justify_horiz_left);
        if (true == GET_BOOL(_pCfg, ATLAS_SERVER_ENABLED))
        {
            _pMainMenu->addButton(_Streaming, "Streaming");
        }
#endif /* ifdef PLAYBACK_IP_SUPPORT */
        _Audio = new CWidgetButton("CScreenMain::_Audio", getEngine(), this, MRect(0, 0, 0, 22), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Audio, ret, eRet_OutOfMemory, error);
        _Audio->setText("Audio...", bwidget_justify_horiz_left);
        _pMainMenu->addButton(_Audio, "Audio");

#if NEXUS_HAS_FRONTEND
        /* find available tuners and add corresponding scan buttons */
        CBoardResources * pResources = _pConfig->getBoardResources();
        CHECK_PTR_ERROR_GOTO("board resources ptr is null", _pConfig, ret, eRet_NotAvailable, error);
        if (true == pResources->findResource(_pModel->getId(), eBoardResource_frontendQam))
        {
            _ScanQam = new CWidgetButton("CScreenMain::_ScanQam", getEngine(), this, MRect(0, 0, 0, 22), font12);
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _ScanQam, ret, eRet_OutOfMemory, error);
            _ScanQam->setText("Scan QAM...", bwidget_justify_horiz_left);
            _pMainMenu->addButton(_ScanQam, "Scan QAM");
            tuner = true;
        }

        if (true == pResources->findResource(_pModel->getId(), eBoardResource_frontendVsb))
        {
            _ScanVsb = new CWidgetButton("CScreenMain::_ScanVsb", getEngine(), this, MRect(0, 0, 0, 22), font12);
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _ScanVsb, ret, eRet_OutOfMemory, error);
            _ScanVsb->setText("Scan VSB...", bwidget_justify_horiz_left);
            _pMainMenu->addButton(_ScanVsb, "Scan VSB");
            tuner = true;
        }

        if (true == pResources->findResource(_pModel->getId(), eBoardResource_frontendSds))
        {
            _ScanSat = new CWidgetButton("CScreenMain::_ScanSat", getEngine(), this, MRect(0, 0, 0, 22), font12);
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _ScanSat, ret, eRet_OutOfMemory, error);
            _ScanSat->setText("Scan Satellite...", bwidget_justify_horiz_left);
            _pMainMenu->addButton(_ScanSat, "Scan Sat");
            tuner = true;
        }

        if (true == pResources->findResource(_pModel->getId(), eBoardResource_frontendOfdm))
        {
            _ScanOfdm = new CWidgetButton("CScreenMain::_ScanOfdm", getEngine(), this, MRect(0, 0, 0, 22), font12);
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _ScanOfdm, ret, eRet_OutOfMemory, error);
            _ScanOfdm->setText("Scan OFDM...", bwidget_justify_horiz_left);
            _pMainMenu->addButton(_ScanOfdm, "Scan OFDM");
            tuner = true;
        }

        if (true == tuner)
        {
            _Tuner = new CWidgetButton("CScreenMain::_Tuner", getEngine(), this, MRect(0, 0, 0, 22), font12);
            CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Audio, ret, eRet_OutOfMemory, error);
            _Tuner->setText("Tuner...", bwidget_justify_horiz_left);
            _pMainMenu->addButton(_Tuner, "Tuner");
        }

#endif /* NEXUS_HAS_FRONTEND */

#ifdef WPA_SUPPLICANT_SUPPORT
        _Network = new CWidgetButton("CScreenMain::_Network", getEngine(), this, MRect(0, 0, 0, 22), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Network, ret, eRet_OutOfMemory, error);
        _Network->setText("Network...", bwidget_justify_horiz_left);
        {
            CWifi * pWifi = _pModel->getWifi();

            if ((NULL != pWifi) && (true == pWifi->getStartState()))
            {
                _pMainMenu->addButton(_Network, "Network");
            }
        }
#elif defined (NETAPP_SUPPORT)
        _Network = new CWidgetButton("CScreenMain::_Network", getEngine(), this, MRect(0, 0, 0, 22), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Network, ret, eRet_OutOfMemory, error);
        _Network->setText("Network...", bwidget_justify_horiz_left);
#ifndef HIDE_NETAPP_WIFI_MENU
        _pMainMenu->addButton(_Network, "Network");
#endif
#endif /* ifdef WPA_SUPPLICANT_SUPPORT */

#ifdef NETAPP_SUPPORT
        _Bluetooth = new CWidgetButton("CScreenMain::_Bluetooth", getEngine(), this, MRect(0, 0, 0, 22), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Bluetooth, ret, eRet_OutOfMemory, error);
#ifndef HIDE_NETAPP_BLUETOOTH_MENU
        _Bluetooth->setText("Bluetooth...  ", bwidget_justify_horiz_left);
        _pMainMenu->addButton(_Bluetooth, "Bluetooth");
#endif
#endif /* ifdef NETAPP_SUPPORT */

#ifdef MPOD_SUPPORT
        _CableCard = new CWidgetButton("CScreenMain::_CableCard", getEngine(), this, MRect(0, 0, 0, 22), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _CableCard, ret, eRet_OutOfMemory, error);
        _CableCard->setText("CableCard...", bwidget_justify_horiz_left);
        _pMainMenu->addButton(_CableCard, "CableCard");
#endif /* ifdef MPOD_SUPPORT */

#ifdef CPUTEST_SUPPORT
        /* cputest dropdown */
        {
            CWidgetCheckButton * pButton = NULL;
            MRect                rectPopup;

            ret = _pMainMenu->addLabelPopupButton(this, "CpuTest", &_CpuTest, &_CpuTestLabel, &_CpuTestPopup, font12, 60);
            CHECK_ERROR_GOTO("unable to allocate label popup list button", ret, error);
            _CpuTest->setFocusable(false);
            _CpuTestLabel->setText("CPU Test", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
            _CpuTestPopup->setText("", bwidget_justify_horiz_right, bwidget_justify_vert_middle);
            rectPopup = _CpuTestPopup->getGeometry();
            /* add buttons for cpu test */
            pButton = _CpuTestPopup->addButton("OFF", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add cpu test button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(0);
            for (int i = 1; i <= 9; i++)
            {
                pButton = _CpuTestPopup->addButton(MString(i), rectPopup.width(), rectPopup.height());
                CHECK_PTR_ERROR_GOTO("unable to add cpu test button to popup list", pButton, ret, eRet_OutOfMemory, error);
                pButton->setValue(10 * i); /* 0 - 100 */
            }
            pButton = _CpuTestPopup->addButton("MAX", rectPopup.width(), rectPopup.height());
            CHECK_PTR_ERROR_GOTO("unable to add cpu test button to popup list", pButton, ret, eRet_OutOfMemory, error);
            pButton->setValue(100);

            _CpuTestPopup->select("OFF");
        }
#endif /* ifdef CPUTEST_SUPPORT */

        /* buffers/tsm */
        _Buffers = new CWidgetCheckButton("CScreenMain::_Buffers", getEngine(), this, MRect(0, 0, 0, 22), font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Buffers, ret, eRet_OutOfMemory, error);
        _Buffers->setText("Buffers / TSM", bwidget_justify_horiz_left);
        _Buffers->setCheckStyle(eCheckStyle_Slide);
        _pMainMenu->addButton(_Buffers, "Buffers / TSM");

#if POWERSTANDBY_SUPPORT
        /* power button - we will use the expand button as power control */
        _Power = new CWidgetButton("CScreenMain::_Power", getEngine(), this, MRect(0, 0, 0, 0), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Power, ret, eRet_OutOfMemory, error);
        _Power->loadImage("images/power-sm.png");
        _pMainMenu->addExpandButton(_Power);
#endif /* if POWERSTANDBY_SUPPORT */

        /* back button */
        _Back = new CWidgetButton("CScreenMain::_Back", getEngine(), this, MRect(0, 0, 0, 0), font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate button widget", _Back, ret, eRet_OutOfMemory, error);
        _Back->setText("Hide");
        _pMainMenu->addBackButton(_Back);
    }

    /* initialize sub menus */
    _pDecodeMenu = new CPanelDecode(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize Decode menu", _pDecodeMenu, ret, eRet_OutOfMemory, error);
    _pDecodeMenu->initialize(pModel, _pConfig);
    _pDecodeMenu->show(false);
    _panelList.add(_pDecodeMenu);

    _pDisplayMenu = new CPanelDisplay(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize Display menu", _pDisplayMenu, ret, eRet_OutOfMemory, error);
    _pDisplayMenu->initialize(pModel, _pConfig);
    _pDisplayMenu->show(false);
    _panelList.add(_pDisplayMenu);

    _pAudioMenu = new CPanelAudio(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize Audio menu", _pAudioMenu, ret, eRet_OutOfMemory, error);
    _pAudioMenu->initialize(pModel, _pConfig);
    _pAudioMenu->show(false);
    _panelList.add(_pAudioMenu);

    _pPlaybackMenu = new CPanelPlayback(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize Playback menu", _pPlaybackMenu, ret, eRet_OutOfMemory, error);
    _pPlaybackMenu->initialize(pModel, _pConfig);
    _pPlaybackMenu->show(false);
    _panelList.add(_pPlaybackMenu);

#ifdef PLAYBACK_IP_SUPPORT
    _pStreamingMenu = new CPanelStreaming(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize Streaming menu", _pStreamingMenu, ret, eRet_OutOfMemory, error);
    _pStreamingMenu->initialize(pModel, _pConfig);
    _pStreamingMenu->show(false);
    _panelList.add(_pStreamingMenu);
#endif /* ifdef PLAYBACK_IP_SUPPORT */

#if NEXUS_HAS_FRONTEND
    _pTunerMenu = new CPanelTuner(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize Playback menu", _pTunerMenu, ret, eRet_OutOfMemory, error);
    _pTunerMenu->initialize(pModel, _pConfig);
    _pTunerMenu->show(false);
    _panelList.add(_pTunerMenu);

    _pScanQamMenu = new CPanelScanQam(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize scan QAM menu", _pScanQamMenu, ret, eRet_OutOfMemory, error);
    _pScanQamMenu->initialize(pModel, _pConfig);
    _pScanQamMenu->show(false);
    _panelList.add(_pScanQamMenu);

    _pScanVsbMenu = new CPanelScanVsb(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize scan Vsb menu", _pScanVsbMenu, ret, eRet_OutOfMemory, error);
    _pScanVsbMenu->initialize(pModel, _pConfig);
    _pScanVsbMenu->show(false);
    _panelList.add(_pScanVsbMenu);

    _pScanSatMenu = new CPanelScanSat(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize scan Sat menu", _pScanSatMenu, ret, eRet_OutOfMemory, error);
    _pScanSatMenu->initialize(pModel, _pConfig);
    _pScanSatMenu->show(false);
    _panelList.add(_pScanSatMenu);

    _pScanOfdmMenu = new CPanelScanOfdm(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize scan Ofdm menu", _pScanOfdmMenu, ret, eRet_OutOfMemory, error);
    _pScanOfdmMenu->initialize(pModel, _pConfig);
    _pScanOfdmMenu->show(false);
    _panelList.add(_pScanOfdmMenu);
#endif /* NEXUS_HAS_FRONTEND */

    _pVbiMenu = new CPanelVbi(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize VBI menu", _pVbiMenu, ret, eRet_OutOfMemory, error);
    _pVbiMenu->initialize(pModel, _pConfig);
    _pVbiMenu->show(false);
    _panelList.add(_pVbiMenu);

#if BDSP_MS12_SUPPORT
    _pAudioAc4Menu = new CPanelAudioAc4(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize AudioAc4 menu", _pAudioAc4Menu, ret, eRet_OutOfMemory, error);
    _pAudioAc4Menu->initialize(pModel, _pConfig);
    _pAudioAc4Menu->show(false);
    _panelList.add(_pAudioAc4Menu);
#endif

#ifdef MPOD_SUPPORT
    _pCableCardMenu = new CPanelCableCard(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize cablecard menu", _pCableCardMenu, ret, eRet_OutOfMemory, error);
    _pCableCardMenu->initialize(pModel, _pConfig);
    _pCableCardMenu->show(false);
    _panelList.add(_pCableCardMenu);
#endif /* ifdef MPOD_SUPPORT */

    _pBuffersMenu = new CPanelBuffers(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize buffers menu", _pBuffersMenu, ret, eRet_OutOfMemory, error);
    _pBuffersMenu->initialize(pModel, _pConfig);
    _pBuffersMenu->show(false);
    _panelList.add(_pBuffersMenu);

#if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT)
    _pNetworkMenu = new CPanelNetworkWifi(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize Network menu", _pNetworkMenu, ret, eRet_OutOfMemory, error);
    _pNetworkMenu->initialize(pModel, _pConfig);
    _pNetworkMenu->show(false);
    _panelList.add(_pNetworkMenu);
#endif /* if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT) */

#ifdef NETAPP_SUPPORT
    _pBluetoothMenu = new CPanelBluetooth(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize Bluetooth menu", _pBluetoothMenu, ret, eRet_OutOfMemory, error);
    _pBluetoothMenu->initialize(pModel, _pConfig);
    _pBluetoothMenu->show(false);
    _panelList.add(_pBluetoothMenu);
#endif /* ifdef NETAPP_SUPPORT */

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

        _MsgBox = new CWidgetModalMsgBox("CScreenMain::_MsgBox", getEngine(), this, geom, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate MsgBox widget", _MsgBox, ret, eRet_OutOfMemory, error);
        _MsgBox->setText("Sample Text", bwidget_justify_horiz_center, bwidget_justify_vert_middle);
        _MsgBox->show(false);
    }

    {
        _pPowerMenu = new CPanelPower(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
        CHECK_PTR_ERROR_GOTO("unable to initialize Power menu", _pPowerMenu, ret, eRet_OutOfMemory, error);
        _pPowerMenu->initialize(pModel, _pConfig);
        _pPowerMenu->show(false);
        _panelList.add(_pPowerMenu);
    }

    _pKeyboard = new CPanelKeyboard(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
    CHECK_PTR_ERROR_GOTO("unable to initialize keyboard", _pKeyboard, ret, eRet_OutOfMemory, error);
    _pKeyboard->initialize(pModel, _pConfig);
    _pKeyboard->setTitle("Connect to Wifi", "Enter Password");
    _pKeyboard->show(false);

    {
        _pRecordMenu = new CPanelRecord(getWidgetEngine(), this, this, MRect(0, 0, 10, 10), font14);
        CHECK_PTR_ERROR_GOTO("unable to initialize Record panel", _pRecordMenu, ret, eRet_OutOfMemory, error);
        _pRecordMenu->initialize(pModel, _pConfig);
        _pRecordMenu->show(false);
        _panelList.add(_pRecordMenu);
    }

    goto done;
error:
    uninitialize();
done:
    return(ret);
} /* initialize */

void CScreenMain::uninitialize()
{
    _timerConnectionStatus.stop();
    _timerVolume.stop();
    _timerChannel.stop();
    _timerBuffersUpdate.stop();
#ifdef CPUTEST_SUPPORT
    _timerCpuTest.stop();
#endif
#if BDSP_MS12_SUPPORT
    _timerAudioFade.stop();
    _timerAudioFadeShow.stop();
#endif
    _timerMsgBox.stop();
    _indicatorList.clear();

    DEL(_pKeyboard);
    DEL(_pPowerMenu);
    DEL(_MsgBox);
    DEL(_pBuffersMenu);
#if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT)
    DEL(_pNetworkMenu);
#endif
#ifdef NETAPP_SUPPORT
    DEL(_pBluetoothMenu);
#endif
#if BDSP_MS12_SUPPORT
    DEL(_pAudioAc4Menu);
#endif
    DEL(_pVbiMenu);
#if NEXUS_HAS_FRONTEND
    DEL(_pScanOfdmMenu);
    DEL(_pScanSatMenu);
    DEL(_pScanVsbMenu);
    DEL(_pScanQamMenu);
    DEL(_pTunerMenu);
#endif /* if NEXUS_HAS_FRONTEND */
    DEL(_pRecordMenu);
#ifdef PLAYBACK_IP_SUPPORT
    DEL(_pStreamingMenu);
#endif
    DEL(_pPlaybackMenu);
    DEL(_pAudioMenu);
    DEL(_pDisplayMenu);
    DEL(_pDecodeMenu);
    DEL(_Back);
#if POWERSTANDBY_SUPPORT
    DEL(_Power);
#endif
#ifdef CPUTEST_SUPPORT
    DEL(_CpuTest);
    DEL(_CpuTestLabel);
    DEL(_CpuTestPopup);
#endif
    DEL(_Buffers);
#if NEXUS_HAS_FRONTEND
    DEL(_Tuner);
    DEL(_ScanOfdm);
    DEL(_ScanSat);
    DEL(_ScanVsb);
    DEL(_ScanQam);
#endif /* if NEXUS_HAS_FRONTEND */
#ifdef MPOD_SUPPORT
    DEL(_pCableCardMenu);
    DEL(_CableCard);
#endif /* if MPOD_SUPPORT */
    DEL(_Streaming);
#if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT)
    DEL(_Network);
#endif
#ifdef NETAPP_SUPPORT
    DEL(_Bluetooth);
#endif
    DEL(_Audio);
    DEL(_Playback);
    DEL(_Display);
    DEL(_Decode);
    DEL(_pMainMenu);
#if BDSP_MS12_SUPPORT
    DEL(_pLabelAudioFadePip);
    DEL(_pLabelAudioFadeMain);
#endif
    DEL(_pLabelVolume);
    DEL(_pLabelScan);
    DEL(_pLabelConnectionType);
    DEL(_pLabelConnectionStatus);
    DEL(_pLabelPlaybackNameShadow);
    DEL(_pLabelPlaybackName);
    DEL(_pLabelDebugNameShadow);
    DEL(_pLabelDebugName);
    DEL(_pLabelPlaybackLength);
    DEL(_pLabelPlaybackState);
    DEL(_pProgressPlayback);
    DEL(_pLabelChannelType);
    DEL(_pLabelChannelNumText);
    DEL(_pLabelChannelNum);
    DEL(_pLabelPip);

    _panelList.clear();
    _channelLabelList.clear();

    {
        CConnectionStatus * pStatus;
        while (NULL != (pStatus = _connectionStatusList.remove()))
        {
            DEL(pStatus);
        }
    }
} /* uninitialize */

CScreenMain::~CScreenMain()
{
    uninitialize();
}

/* override registerObserver so we can propgate the observer to the menu panels.  this will allow
 * the menu panels to notify those observers that are interested in this screens notifications. */
eRet CScreenMain::registerObserver(
        CObserver *   pObserver,
        eNotification notification
        )
{
    MListItr <CPanel> itr(&_panelList);
    CPanel *          pPanel = NULL;
    eRet              ret    = eRet_Ok;

    BDBG_ASSERT(NULL != _pMainMenu);
    BDBG_ASSERT(0 < _panelList.total());

    CSubject::registerObserver(pObserver, notification);

    for (pPanel = itr.first(); pPanel; pPanel = itr.next())
    {
        ret = pPanel->registerObserver(pObserver, notification);
    }

    return(ret);
} /* registerObserver */

#include <math.h>
#define SCALE_MPEGPOS(POS)  ((POS) >> 5)

void CScreenMain::processNotification(CNotification & notification)
{
    CPanel * pPanel = NULL;
    eRet     ret    = eRet_Ok;

    /* we could have simply handled all the incomming notifications for all the panels
     * here, but instead we'll split them off and keep the notification handling code
     * and the respective panel code in the same file */
    for (pPanel = _panelList.first(); pPanel; pPanel = _panelList.next())
    {
        pPanel->processNotification(notification);
    }

    switch (notification.getId())
    {
    case eNotify_ChannelStart:
    {
        eMode mode = _pModel->getMode();
        if (eMode_Live == mode)
        {
            CChannel * pChannel = (CChannel *)notification.getData();

            /* channel started so query about labels and create them if necessary */
            updateChannelLabels(pChannel);
        }
    }
    break;
    case eNotify_ChannelFinish:
    {
        eMode mode = _pModel->getMode();

        if (eMode_Live == mode)
        {
            CChannel * pChannel = (CChannel *)notification.getData();

            /* channel untuned so remove associated label widgets */
            removeChannelLabels(pChannel);
        }
    }
    break;
    case eNotify_CurrentChannel:
    {
        eMode mode = _pModel->getMode();
        if (eMode_Live == mode)
        {
            CChannel * pChannel = _pModel->getCurrentChannel();
            setChannelNum(pChannel);
        }
        else
        if (eMode_Playback == mode)
        {
            CPlayback * pPlayback = _pModel->getPlayback();
            if ((NULL != pPlayback) && (pPlayback->isActive()))
            {
                setPlaybackStatus(pPlayback);
            }
        }
    }
    break;

    case eNotify_DeferredChannel:
    {
        CChannel *    pChannel    = NULL;
        eWindowType * pWindowType = (eWindowType *)notification.getData();
        MString       strChannelNum;

        if (_pModel->getFullScreenWindowType() == *pWindowType)
        {
            strChannelNum = _pModel->getDeferredChannelNum();
            if (strChannelNum.isEmpty())
            {
                /* no deferred channel so use current channel */
                pChannel      = _pModel->getCurrentChannel();
                strChannelNum = "0.0";
            }
            else
            {
                /* use deferred channel */
                pChannel = _pModel->getDeferredChannel();
            }

            /* update channel num/type widget */
            if (NULL != pChannel)
            {
                /* full channel object available */
                setChannelNum(pChannel);
            }
            else
            {
                /* only a channel num available */
                setChannelNum(strChannelNum.s());

                if (strChannelNum == "0.0")
                {
                    /* failed deferred tune */
                    CPlayback * pPlayback = _pModel->getPlayback();
                    if ((NULL != pPlayback) && (pPlayback->isActive()))
                    {
                        setPlaybackStatus(pPlayback);
                    }
                }
            }
        }
    }
    break;

    case eNotify_RecordStarted:
    case eNotify_RecordStopped:
    case eNotify_EncodeStarted:
    case eNotify_EncodeStopped:
    {
        CChannelMgr * pChannelMgr = NULL;
        CPlaylistDb * pPlaylistDb = NULL;

        pChannelMgr = _pModel->getChannelMgr();
        CHECK_PTR_WARN("unable to get channel mgr", pChannelMgr, ret, eRet_NotAvailable);

        pPlaylistDb = _pModel->getPlaylistDb();
        CHECK_PTR_WARN("unable to get playlist db", pPlaylistDb, ret, eRet_NotAvailable);

        setChannelIndicator(pChannelMgr, pPlaylistDb);
    }
    break;

#ifdef CPUTEST_SUPPORT
    case eNotify_CpuTestStarted:
    {
        _pCpuTest = (CCpuTest *)notification.getData();
        BDBG_ASSERT(NULL != _pCpuTest);

        _CpuTestPopup->select(_pCpuTest->getLevel());
        updateCpuTestUtilization();

        _timerCpuTest.start();
    }
    break;

    case eNotify_CpuTestStopped:
    {
        int nLevel = 0;

        if (NULL != _pCpuTest)
        {
            _timerCpuTest.stop();
            _CpuTestPopup->setValue(_pCpuTest->getLevel());
            _pCpuTest = NULL;
        }

        _CpuTestPopup->select(nLevel);
        updateCpuTestUtilization();
    }
    break;
#endif /* ifdef CPUTEST_SUPPORT */

    case eNotify_Timeout:
    {
        CTimer * pTimer = (CTimer *)notification.getData();
        if (&_timerConnectionStatus == pTimer)
        {
            CConnectionStatus * pConnectData = NULL;
            pConnectData = _connectionStatusList.remove();

            if (NULL != pConnectData)
            {
                if (true == pConnectData->_bConnected)
                {
                    _pLabelConnectionType->setText("Connected:");
                    _pLabelConnectionType->setTextColor(COLOR_GREEN);
                }
                else
                {
                    _pLabelConnectionType->setText("Disconnected:");
                    _pLabelConnectionType->setTextColor(COLOR_RED_LIGHT);
                }
                _pLabelConnectionStatus->setText(pConnectData->_strText);

                _pLabelConnectionType->show(true);
                _pLabelConnectionStatus->show(true);

                _timerConnectionStatus.start(pConnectData->_timeout);
                DEL(pConnectData);
            }
            else
            {
                /* no more queued connect data so hide popup */
                _pLabelConnectionType->show(false);
                _pLabelConnectionStatus->show(false);
            }
        }
        else
        if (&_timerChannel == pTimer)
        {
            /* if menu is visible do not hide channel num */
            if (false == isVisible())
            {
                _pLabelChannelNum->show(false);
            }
        }
        else
        if (&_timerVolume == pTimer)
        {
            _pLabelVolume->show(false);
        }
        else
        if (&_timerBuffersUpdate == pTimer)
        {
            CPlayback * pPlayback = _pModel->getPlayback();
            CChannel *  pChannel  = _pModel->getCurrentChannel();
#if DVR_LIB_SUPPORT
            CTsb * pTsb = _pModel->getTsb();
            if (pTsb && pTsb->isTsbDecode())
            {
                updateTsbProgress();
                _timerBuffersUpdate.start();
            }
            else
            {
#endif /* if DVR_LIB_SUPPORT */

            if (eMode_Playback == _pModel->getMode())
            {
                updatePlaybackProgress();
            }
            else
            if (eMode_Live == _pModel->getMode())
            {
                updateChannelProgress();
            }

            if (((NULL != pPlayback) && (true == pPlayback->isActive())) ||
                ((NULL != pChannel) && (true == pChannel->timelineSupport()) && (true == pChannel->isTuned())))
            {
                /* restart update timer if we are still in an active playback */
                _timerBuffersUpdate.start();
            }
#if DVR_LIB_SUPPORT
        }
#endif
        }
        else
#ifdef CPUTEST_SUPPORT
        if (&_timerCpuTest == pTimer)
        {
            updateCpuTestUtilization();
            _timerCpuTest.start();
        }
        else
#endif /* ifdef CPUTEST_SUPPORT */
#if BDSP_MS12_SUPPORT
        if (&_timerAudioFade == pTimer)
        {
            updateAudioFadeLevels();
            _timerAudioFade.start();
        }
        else
        if (&_timerAudioFadeShow == pTimer)
        {
            _pLabelAudioFadeMain->show(_pModel->getPipState());
            _pLabelAudioFadePip->show(_pModel->getPipState());

            _pModel->getPipState() ? _timerAudioFade.start() : _timerAudioFade.stop();
        }
        else
#endif
        if (&_timerMsgBox == pTimer)
        {
            _MsgBox->cancelModal("Cancel");
        }
    }
    break;

    case eNotify_CableCardIn:
    {
        _timerMsgBox.start(2000);
        if (_MsgBox->showModal("CABLECARD PLUGGED IN", 0, "Ok", NULL, NULL) != "ok")
        {
            _timerMsgBox.stop();
        }
    }
    break;
    case eNotify_CableCardOut:
    {
        _timerMsgBox.start(2000);
        if (_MsgBox->showModal("CABLECARD PLUGGED OUT", 0, "Ok", NULL, NULL) != "ok")
        {
            _timerMsgBox.stop();
        }
    }
    break;

    case eNotify_ChannelMapUpdate:
    {
        _timerMsgBox.start(7000);
        if (_MsgBox->showModal("CHANNEL MAP UPDATE TRIGGERED BY CABLECARD", 0, "Ok", NULL, NULL) != "ok")
        {
            _timerMsgBox.stop();
        }
    }
    break;
#if DVR_LIB_SUPPORT
    case eNotify_TsbStateChanged:
    {
        CTsb * pTsb = _pModel->getTsb();
        if ((NULL != pTsb) && (pTsb->isActive()))
        {
            setTsbStatus(pTsb);
        }
    }
#endif /* if DVR_LIB_SUPPORT */
    case eNotify_PlaybackStateChanged:
    {
        CPlayback * pPlayback = _pModel->getPlayback();
        if (NULL != pPlayback)
        {
            setPlaybackStatus(pPlayback);
        }
    }
    break;

    case eNotify_ChannelStateChanged:
    {
        CChannel * pChannel = (CChannel *)notification.getData();
        if (NULL != pChannel)
        {
            setChannelNum(pChannel);
        }
    }
    break;

    case eNotify_VolumeChanged:
    case eNotify_MuteChanged:
    {
        CAudioVolume * pVolume = (CAudioVolume *)notification.getData();

        if (NULL != pVolume)
        {
            uint8_t percent       = 0;
            float   averageVolume = ((float)pVolume->_left / 2.0) + ((float)pVolume->_right / 2.0);

            if (NEXUS_AudioVolumeType_eLinear == pVolume->_volumeType)
            {
                percent = averageVolume * 100.0 / (float)(NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - NEXUS_AUDIO_VOLUME_LINEAR_MIN) + 0.5;
            }
            else
            {
                /* convert dB volume level to linear based percentage */
                percent = pow(10, (float)averageVolume / 10.0) * 100.0;
            }

            setVolumeProgress(percent, pVolume->_muted);
        }
    }
    break;

#if BDSP_MS12_SUPPORT
    case eNotify_AudioAc4DialogEnhancementChanged:
    {
        int * pDb = (int *)notification.getData();

        if (NULL != pDb)
        {
            setDialogEnhancementProgress(*pDb);
        }
    }
    break;
#endif

#if NEXUS_HAS_FRONTEND
    case eNotify_ScanStarted:
    {
        _channelsFound = 0;
        _pLabelScan->setLevel(0);
        _pLabelScan->showProgress(true);
        _pLabelScan->show(true);
    }
    break;

    case eNotify_ScanStopped:
    {
        CModel * pModel = getModel();

        _pLabelScan->show(false);
#ifdef MPOD_SUPPORT
        getModel()->setScanSaveOffer(true);
#endif
        if ((0 < _channelsFound) && (true == pModel->getScanSaveOffer()))
        {
            char text[64];
            snprintf(text, sizeof(text), "Save %d new channels to disk?", _channelsFound);
            if (_MsgBox->showModal(text) == "Ok")
            {
                CChannelMgrLoadSaveData saveChannelData;
                notifyObservers(eNotify_ChannelListSave, &saveChannelData);
            }
        }

        getModel()->setScanSaveOffer(false);
        show(true);
    }
    break;

    case eNotify_ScanProgress:
    {
        char     strTuner[32];
        CTuner * pTuner                        = NULL;
        CTunerScanNotificationData * pScanData = (CTunerScanNotificationData *)notification.getData();
        BDBG_ASSERT(pScanData);

        pTuner = pScanData->getTuner();
        BDBG_ASSERT(pTuner);

        snprintf(strTuner, sizeof(strTuner), "%s%d", pTuner->getName(), pTuner->getNumber());
        setChannelStatus(eMode_Scan, strTuner);

        setScanProgress(pTuner, pScanData->getPercent());
    }
    break;

    case eNotify_ChannelListChanged:
    {
        CChannelMgrListChangedData * pChListChangedData =
            (CChannelMgrListChangedData *)notification.getData();

        if (NULL != pChListChangedData)
        {
            if ((pChListChangedData->isAdd()) && (NULL != pChListChangedData->getChannel()))
            {
                /* a new channel was added so increment found scan channels count -
                 * note that this will increment even when we aren't scanning,
                 * but that is ok because the count is reset when a scan starts. */
                _channelsFound++;
            }
        }
    }
    break;

    case eNotify_ChannelListVersion:
    {
        char text[64];
        snprintf(text, sizeof(text), "Channel list version mismatch! Perform channel scan.");
        _MsgBox->showModal(text, 0, "Ok", NULL, NULL);
        _Back->setFocus();
    }
    break;
#endif /* NEXUS_HAS_FRONTEND */

    case eNotify_PipStateChanged:
    case eNotify_VideoSourceChanged:
    {
        updatePip();
    }
    break;

#if HAS_VID_NL_LUMA_RANGE_ADJ
    case eNotify_VideoPlmChanged:
    {
        eRet                 ret          = eRet_Ok;
        CSimpleVideoDecode * pVideoDecode = (CSimpleVideoDecode *)notification.getData();

        if (NULL != pVideoDecode)
        {
            ret = addDynamicRangeIndicator(pVideoDecode);
            CHECK_WARN("unable to get video decoder dynmaic range - dynamc range/PLM indicator will not be shown", ret);

            if (eRet_Ok == ret)
            {
                addPlmIndicator(pVideoDecode);
            }
        }
    }
    break;
#endif /* if HAS_VID_NL_LUMA_RANGE_ADJ */

    case eNotify_VideoStreamChanged:
        break;

    case eNotify_PowerModeChanged:
    {
        ePowerMode * pMode = (ePowerMode *)notification.getData();
        bool         bOn   = ((ePowerMode_S0 == *pMode) ? true : false);

        /* hide/ panels */
        show(bOn);

        /* hide/show buffers panel based on state */
        _pBuffersMenu->show(bOn ? _Buffers->isChecked() : false);

#if BDSP_MS12_SUPPORT
        /* hide/show audio fade controls based on state */
        _pLabelAudioFadeMain ->show(bOn ? _pModel->getPipState() : false);
        _pLabelAudioFadePip ->show(bOn ? _pModel->getPipState() : false);
#endif

        if (false == bOn)
        {
            /* hide playback title if power off */
            showPlaybackTitle(false);
        }
    }
    break;

    case eNotify_KeyDown:
    {
        eKey * pKey = (eKey *)notification.getData();

        /* global keyDown event (see CModel::sendGlobalKeyDown()) */
        switch (*pKey)
        {
        case eKey_Menu:
        {
            show((true == isVisible()) ? false : true);
            if (_pKeyboard)
            {
                _pKeyboard->cancelModal();
            }
        }
        break;

        case eKey_Power:
        {
            MString strRet("Ok");
#if POWERSTANDBY_SUPPORT
            strRet = _pPowerMenu->showModal();
#endif
            if (strRet == "Ok")
            {
                ePowerMode mode = _pPowerMenu->getPowerMode();
                notifyObservers(eNotify_SetPowerMode, &mode);
            }
        }
        break;

        case eKey_Record:
        {
            MString strRet = _pRecordMenu->showModal();
        }
        break;

        default:
            break;
        }   /* switch */
    }
    break;

    case eNotify_PlaylistAdded:
    case eNotify_PlaylistRemoved:
    {
        CPlaylist * pPlaylist = (CPlaylist *)notification.getData();
        if (NULL != pPlaylist)
        {
            showConnectionStatus(
                    (eNotify_PlaylistAdded == notification.getId()) ? true : false,
                    pPlaylist->getName(),
                    GET_INT(_pCfg, UI_CONNECTION_STATUS_TIMEOUT));
        }
    }
    break;

    case eNotify_Debug:
    {
        MString * pStringDebug = (MString *)notification.getData();

        if (NULL == pStringDebug)
        {
            showDebugTitle(false);
        }
        else
        {
            BDBG_WRN(("=====> Lua Debug: %s", (*pStringDebug).s()));
            showDebugTitle((true == pStringDebug->isEmpty()) ? false : true);
            setDebugTitle(*pStringDebug);
        }
    }
    break;

    default:
        break;
    } /* switch */

error:
    return;
} /* processNotification */

eRet CScreenMain::showConnectionStatus(
        bool         bConnected,
        const char * strText,
        uint32_t     timeout
        )
{
    eRet ret                         = eRet_Ok;
    CConnectionStatus * pConnectData = NULL;

    pConnectData = new CConnectionStatus(bConnected, strText, timeout);
    CHECK_PTR_ERROR_GOTO("unable to allocate connection data", pConnectData, ret, eRet_OutOfMemory, error);

    /* add connect data to list - will be displayed when timer expires */
    _connectionStatusList.add(pConnectData);

    if (false == _pLabelConnectionStatus->isVisible())
    {
        /* kick start connection status timer if not currently running */
        _timerConnectionStatus.start(100);
    }

error:
    return(ret);
} /* showConnectionStatus */

eRet CScreenMain::showPlaybackTitle(bool bShow)
{
    _pLabelPlaybackNameShadow->show(bShow);
    _pLabelPlaybackName->show(bShow);

    return(eRet_Ok);
} /* showPlaybackTitle */

eRet CScreenMain::setPlaybackTitle(const char * str)
{
    _pLabelPlaybackNameShadow->setText(str);
    _pLabelPlaybackName->setText(str);

    return(eRet_Ok);
} /* setPlaybackTitle */

eRet CScreenMain::showDebugTitle(bool bShow)
{
    _pLabelDebugNameShadow->show(bShow);
    _pLabelDebugName->show(bShow);

    return(eRet_Ok);
} /* showDebugTitle */

eRet CScreenMain::setDebugTitle(const char * str)
{
    _pLabelDebugNameShadow->setText(str);
    _pLabelDebugName->setText(str);

    return(eRet_Ok);
} /* setDebugTitle */

eRet CScreenMain::updatePip()
{
    eRet ret                          = eRet_Ok;
    CSimpleVideoDecode * pVideoDecode = _pModel->getSimpleVideoDecode(_pModel->getPipScreenWindowType());
    NEXUS_VideoDecoderStatus nDecodeStatus;

    if (NULL != pVideoDecode)
    {
        ret = pVideoDecode->getStatus(&nDecodeStatus);
        CHECK_ERROR_GOTO("getting video decoder status failed!", ret, error);

        if (true == _pModel->getPipState())
        {
            _pLabelPip->show(pVideoDecode->isStarted() ? false : true);
        }
        else
        {
            _pLabelPip->show(false);
        }
    }

#if BDSP_MS12_SUPPORT
    _timerAudioFadeShow.start();
#endif

error:
    return(ret);
} /* updatePip */

#if DVR_LIB_SUPPORT
eRet CScreenMain::updateTsbProgress()
{
    eRet ret          = eRet_Ok;
    CTsb * pTsb       = _pModel->getTsb();
    CDvrMgr * pDvrMgr = _pModel->getDvrMgr();
    B_DVR_TSBServiceStatus tsbServiceStatus;
    unsigned level    = 0;
    uint32_t current  = 0;
    uint32_t firstpos = 0;
    uint32_t lastpos  = 0;

    level = 0;
    if (NULL != pTsb)
    {
        ret = pTsb->getStatus(&tsbServiceStatus);
        CHECK_NEXUS_ERROR_ASSERT("getting tsb status failed!", ret);
        if (tsbServiceStatus.tsbCurrentPlayTime < tsbServiceStatus.tsbRecStartTime)
        {
            current = 0;
        }
        else
        {
            current = SCALE_MPEGPOS(tsbServiceStatus.tsbCurrentPlayTime-tsbServiceStatus.tsbRecStartTime);
        }
        firstpos = SCALE_MPEGPOS(0);
        lastpos  = SCALE_MPEGPOS(pDvrMgr->getTsbWindow());
        BDBG_MSG(("pos %d in %d...%d", current, firstpos, lastpos));

        if (lastpos == 0)
        {
            BDBG_MSG(("no data"));
            goto error;
        }
        level = (current* 65535)/lastpos;
        BDBG_MSG(("level %hu", level));
    }
    _pProgressPlayback->setLevel(level);

error:
    return(ret);
} /* updateTsbProgress */

#endif /* if DVR_LIB_SUPPORT */

eRet CScreenMain::updateChannelProgress()
{
    eRet ret            = eRet_Ok;
    CChannel * pChannel = _pModel->getCurrentChannel();
    unsigned level      = 0;
    uint32_t current    = 0;
    uint32_t firstpos   = 0;
    uint32_t lastpos    = 0;

    level = 0;
    if (NULL != pChannel)
    {
        if ((pChannel->getCurrentPosition() == 0) && (pChannel->getLastPosition() == 0))
        {
            BDBG_WRN((" Channel has no information about position, could be UDP, RTP"));
            ret = eRet_ExternalError;
            goto error;
        }

        /**
         * This code used to be more complicated. It can and should remain simple now.
         * If the pos needs to be adjusted for good UI, please do that in the UI code.
         * If timestamp is unavailable, please add timestamp approximation
         * code.
         **/
        current  = SCALE_MPEGPOS(pChannel->getCurrentPosition());
        firstpos = 0;
        lastpos  = SCALE_MPEGPOS(pChannel->getLastPosition());

        BDBG_MSG(("pos %d in %d...%d from Channel Ip data", current, firstpos, lastpos));

        if (lastpos == 0)
        {
            BDBG_MSG(("no data"));
            goto error;
        }

        BDBG_MSG((" Channel current Position get %u", pChannel->getCurrentPosition()));
        BDBG_MSG(("Channel last Position %u", pChannel->getLastPosition()));
        level = (current* 65535)/lastpos;
        BDBG_MSG(("level %hu", level));
    }

error:
    _pProgressPlayback->setLevel(level);
    return(ret);
} /* updateChannelProgress */

eRet CScreenMain::updatePlaybackProgress()
{
    eRet ret              = eRet_Ok;
    CPlayback * pPlayback = _pModel->getPlayback();
    NEXUS_PlaybackStatus nPlaybackStatus;
    NEXUS_Error nerror = NEXUS_SUCCESS;
    unsigned level     = 0;
    uint32_t current   = 0;
    uint32_t firstpos  = 0;
    uint32_t lastpos   = 0;

    level = 0;
    if (NULL != pPlayback)
    {
        nerror = NEXUS_Playback_GetStatus(pPlayback->getPlayback(), &nPlaybackStatus);
        CHECK_NEXUS_ERROR_ASSERT("getting playback status failed!", nerror);

        /**
         * This code used to be more complicated. It can and should remain simple now.
         * If the pos needs to be adjusted for good UI, please do that in the UI code.
         * If timestamp is unavailable, please add timestamp approximation
         * code.
         **/
        if (!pPlayback->hasIndex())
        {
            /* no index, so use mpeg file data */
            current  = SCALE_MPEGPOS(nPlaybackStatus.position);
            firstpos = 0;
            lastpos  = SCALE_MPEGPOS(nPlaybackStatus.last);
        }
        else
        {
            current  = SCALE_MPEGPOS(nPlaybackStatus.position);
            firstpos = SCALE_MPEGPOS(nPlaybackStatus.first);
            lastpos  = SCALE_MPEGPOS(nPlaybackStatus.last);
        }
        BDBG_MSG(("pos %d in %d...%d (from %s)", current, firstpos, lastpos, pPlayback->hasIndex() ? "index" : "data"));

        if (lastpos == 0)
        {
            BDBG_MSG(("no data"));
            goto error;
        }

        BDBG_MSG((" nPlaybackStatus.position get %lu", nPlaybackStatus.position));
        BDBG_MSG(("nPlaybackStatus.last %lu", nPlaybackStatus.last));
        level = (current* 65535)/lastpos;
        BDBG_MSG(("level %hu", level));
    }
    _pProgressPlayback->setLevel(level);

error:
    return(ret);
} /* updatePlaybackProgress */

/* displays either playback status or channel status on screen.  if mode == eMode_Max (default) then
 * this function will determine what type of status to show based on the existence of an active playback.
 * if an active playback exists, it will display playback status.  otherwise it will display channel
 * status.  mode can be used to force a specific type of status to be displayed.
 */
void CScreenMain::setChannelStatus(
        eMode        mode,
        const char * str1,
        const char * str2,
        const char * strPlaybackTitle,
        CChannel *   pChannel
        )
{
    bool bShowPlaybackProgress = false;

    switch (mode)
    {
#if DVR_LIB_SUPPORT
    case eMode_Tsb:
    {
        bShowPlaybackProgress = true;
        _pLabelChannelNumText->setText("Tsb");
        _pLabelPlaybackState->setText(NULL != str1 ? str1 : "");
        _pLabelPlaybackLength->setText(NULL != str2 ? str2 : "");
        updateTsbProgress();
        _timerBuffersUpdate.start(GET_INT(_pCfg, UI_BUFFERS_UPDATE_TIMEOUT));
        break;
    }
#endif /* if DVR_LIB_SUPPORT */
    case eMode_Playback:
    {
        /* display playback status */
        bShowPlaybackProgress = true;

        /* update playback state/length */
        _pLabelChannelNumText->setText("Playback");
        _pLabelPlaybackState->setText(NULL != str1 ? str1 : "");
        _pLabelPlaybackLength->setText(NULL != str2 ? str2 : "");

        if (NULL != strPlaybackTitle)
        {
            /* update playback title*/
            setPlaybackTitle(MString("Now Playing: ") + strPlaybackTitle);
        }

        updatePlaybackProgress();
        _timerBuffersUpdate.start(GET_INT(_pCfg, UI_BUFFERS_UPDATE_TIMEOUT));

        break;
    }

    case eMode_Live:
    {
        /* display channel status */

        _pLabelChannelType->setText("");
        _pLabelPlaybackState->setText("");
        _pLabelPlaybackLength->setText("");

        /* update channel num/type */
        if (MString(str1) == "0.0")
        {
            _pLabelChannelNumText->setText("Streaming");
        }
        else
        {
            _pLabelChannelNumText->setText(NULL != str1 ? str1 : "");
            _pLabelChannelType->setText(NULL != str2 ? str2 : "");
        }

        /* update playback title*/
        if (NULL != strPlaybackTitle)
        {
            setPlaybackTitle(MString("Now Streaming: ") + strPlaybackTitle);
        }

        /* some channels may contain a long description */
        if ((NULL != pChannel) && (pChannel->timelineSupport()))
        {
            MString strTrick;

            bShowPlaybackProgress = true;

            /* update playback state/length */
            strTrick = MString(channelTrickToString(pChannel->getTrickModeState())) + " ";
            if (0 != pChannel->getTrickModeRate())
            {
                /* hide rate for pause */
                strTrick += MString(pChannel->getTrickModeRate()) + "x";
            }

            _pLabelPlaybackState->setText(strTrick);
            _pLabelPlaybackLength->setText(pChannel->getTimeString());

            if (NULL != strPlaybackTitle)
            {
                /* update playback title*/
                setPlaybackTitle(MString("Now Streaming: ") + strPlaybackTitle);
            }

            updateChannelProgress();

            _timerBuffersUpdate.start(GET_INT(_pCfg, UI_BUFFERS_UPDATE_TIMEOUT));
        }
        break;
    }

    case eMode_Scan:
    {
        /* display scan status */

        _pLabelChannelNumText->setText("Channel Scan");
        _pLabelChannelType->setText(NULL != str1 ? str1 : "");
        break;
    }

    default:
        _pLabelChannelNumText->setText("Default");
        _pLabelChannelNumText->setText(NULL != str1 ? str1 : "");
        _pLabelChannelType->setText(NULL != str2 ? str2 : "");
        break;
    } /* switch */

    if ((NULL != pChannel) && (NULL != pChannel->getParent()))
    {
        /* override channel banner text if channel has a parent.
         * (it is a subchannel of a mosaic channel) */
        _pLabelChannelType->setText("Mosaic");
        bShowPlaybackProgress = false;
    }

    _pProgressPlayback->show(bShowPlaybackProgress);
    showPlaybackTitle(bShowPlaybackProgress);

    _pLabelChannelNum->show(true);
    _timerChannel.start(GET_INT(_pCfg, UI_CHANNEL_NUM_TIMEOUT));
} /* setChannelStatus */

void CScreenMain::setChannelNum(
        const char * str1,
        const char * str2
        )
{
    setChannelStatus(eMode_Live, str1, str2);
} /* setChannelNum */

void CScreenMain::setChannelNum(CChannel * pChannel)
{
    char strChNum[16];
    char strChType[16];

    if (NULL == pChannel)
    {
        snprintf(strChNum, 16, "0.0");
    }
    else
    {
        strChNum[0] = 0;
        snprintf(strChNum, 16, "%d.%d", pChannel->getMajor(), pChannel->getMinor());
        strChType[0] = 0;
        snprintf(strChType, 16, "%s", pChannel->getDescription().s());

        setChannelStatus(eMode_Live, strChNum, strChType, pChannel->getDescriptionLong(), pChannel);
    }
} /* setChannelNum */

CChannelLabel * CScreenMain::findChannelLabel(
        CChannel *   pChannel,
        CLabelData * pLabelData
        )
{
    CChannelLabel * pChannelLabel = NULL;

    for (pChannelLabel = _channelLabelList.first(); pChannelLabel; pChannelLabel = _channelLabelList.next())
    {
        if ((pChannel == pChannelLabel->_pChannel) && (*pLabelData == pChannelLabel->_labelData))
        {
            /* channel and labeldata match - found! */
            break;
        }
    }

    BDBG_MSG(("size of channel label list:%d pChannelLabel:%p", _channelLabelList.total(), pChannelLabel));
    return(pChannelLabel);
}

/* adjust pRect if it exceeds the bounds of rectMax given nBorder.
 * given rects are in pixels, not percentages. */
void CScreenMain::adjustChannelLabelBorder(
        MRect * pRect,
        MRect   rectMax,
        int     nBorder
        )
{
    int nDiffX = (pRect->right() + (int)nBorder) - rectMax.right();
    int nDiffY = (pRect->bottom() + (int)nBorder) - rectMax.bottom();

    if (pRect->x() < (rectMax.x() + nBorder))
    {
        /* adjust for left border */
        pRect->setX(rectMax.x() + nBorder);
    }

    if (pRect->y() < (rectMax.y() + nBorder))
    {
        /* adjust for top border */
        pRect->setY(rectMax.y() + nBorder);
    }

    if (0 < nDiffX)
    {
        /* adjust for right border */
        pRect->setX(pRect->x() - nDiffX);
    }

    if (0 < nDiffY)
    {
        /* adjust for bottom border */
        pRect->setY(pRect->y() - nDiffY);
    }
} /* adjustChannelLabelBorder */

eRet CScreenMain::createChannelLabel(
        CLabelData *    pLabelData,
        CChannel *      pChannel,
        CWidgetLabel ** pChannelLabel
        )
{
    CGraphics * pGraphics              = _pModel->getGraphics();
    CChannelLabel * pChannelLabelFound = NULL;
    eRet ret                           = eRet_Ok;

    if (NULL == pGraphics)
    {
        return(ret);
    }

    if (NULL == (pChannelLabelFound = findChannelLabel(pChannel, pLabelData)))
    {
        MRect rectGeomGraphics(0, 0, pGraphics->getWidth(), pGraphics->getHeight());
        unsigned nBorder                   = 15;
        CWidgetLabel * pWidgetChannelLabel = NULL;
        MRect rectGeomVideoWinPercent      = pChannel->getVideoWindowGeometryPercent();
        MRect rectGeomLabelPercent         = pLabelData->_rectGeometryPercent;
        MRect rectGeomVideoWinAbsolute     = SCALE_RECT_PERCENT(rectGeomGraphics, rectGeomVideoWinPercent);
        MRect rectGeomLabelAbsolute        = SCALE_RECT_PERCENT(rectGeomVideoWinAbsolute, rectGeomLabelPercent);
        bwin_font_t font12;

        font12 = pGraphics->getFont(12);
        BDBG_ASSERT(NULL != font12);

        if (true == pLabelData->_bGlobal)
        {
            /* label indicates geometry values are respective of the global coordinate system,
             * so adjust absolute values based on that */
            rectGeomLabelAbsolute = SCALE_RECT_PERCENT(rectGeomGraphics, rectGeomLabelPercent);
        }

        /* create image label */
        pWidgetChannelLabel = new CWidgetLabel("CScreenMain::pWidgetChannelLabel", getEngine(), this, rectGeomLabelAbsolute, font12);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", pWidgetChannelLabel, ret, eRet_OutOfMemory, error);

        if (false == pLabelData->_strImagePath.isEmpty())
        {
            MRect rectImageGeometry = pWidgetChannelLabel->getImageGeometry(pLabelData->_strImagePath);

            if ((0 == rectGeomLabelPercent.width()) && (0 == rectGeomLabelPercent.height()))
            {
                /* no image width/height given so default to actual image size */
                rectGeomLabelAbsolute.setWidth(rectImageGeometry.width());
                rectGeomLabelAbsolute.setHeight(rectImageGeometry.height());
            }
            else
            if (!(0 == rectGeomLabelPercent.width()) != !(0 == rectGeomLabelPercent.height()))
            {
                /* specified width xor height is 1 so do proportional scale */

                if (0 < rectGeomLabelPercent.width())
                {
                    /* adjust height proportionally based on width */
                    rectGeomLabelAbsolute.setHeight(
                            (unsigned)(rectGeomLabelAbsolute.width() * (float)rectImageGeometry.height() / (float)rectImageGeometry.width() + 0.5));
                }
                else
                {
                    /* adjust width proportionally based on height */
                    rectGeomLabelAbsolute.setWidth(
                            (unsigned)(rectGeomLabelAbsolute.height() * (float)rectImageGeometry.width() / (float)rectImageGeometry.height() + 0.5));
                }
            }

            BDBG_MSG(("LOAD Channel Label Image %s - x:%d y:%d w:%d h:%d", pLabelData->_strImagePath.s(), rectGeomLabelAbsolute.x(), rectGeomLabelAbsolute.y(), rectGeomLabelAbsolute.width(), rectGeomLabelAbsolute.height()));
            pWidgetChannelLabel->setGeometry(rectGeomLabelAbsolute);

            /* adjust label geometry if it overlaps minimum nBorder. this can happen because
             * you specify x and y in percentage, and may leave width and height to actual size. */
            if (true == pLabelData->_bGlobal)
            {
                adjustChannelLabelBorder(&rectGeomLabelAbsolute, rectGeomGraphics, nBorder);
            }
            else
            {
                adjustChannelLabelBorder(&rectGeomLabelAbsolute, rectGeomVideoWinAbsolute, nBorder);
            }
            BDBG_MSG(("ADJUST Channel Label Image %s - x:%d y:%d w:%d h:%d", pLabelData->_strImagePath.s(), rectGeomLabelAbsolute.x(), rectGeomLabelAbsolute.y(), rectGeomLabelAbsolute.width(), rectGeomLabelAbsolute.height()));

            ret = pWidgetChannelLabel->loadImage(pLabelData->_strImagePath, bwin_image_render_mode_maximize_down);
            /* ret = pWidgetChannelLabel->loadImage(pLabelData->_strImagePath, bwin_image_render_mode_single); */
            CHECK_ERROR("unable to load channel image", ret);

            BDBG_MSG(("create new image:%s x:%d y:%d w:%d h:%d", pLabelData->_strImagePath.s(), rectGeomLabelAbsolute.x(), rectGeomLabelAbsolute.y(), rectGeomLabelAbsolute.width(), rectGeomLabelAbsolute.height()));
        }

        BDBG_MSG(("rectGeomLabelAbsolute x:%d y:%d w:%d h:%d", rectGeomLabelAbsolute.x(), rectGeomLabelAbsolute.y(), rectGeomLabelAbsolute.width(), rectGeomLabelAbsolute.height()));
        pWidgetChannelLabel->setBevel(0);
        pWidgetChannelLabel->setZOrder(pLabelData->_zorder);
        pWidgetChannelLabel->setText(pLabelData->_strText);
        pWidgetChannelLabel->setGeometry(rectGeomLabelAbsolute);
        pWidgetChannelLabel->setBackgroundColor(0x0);
        pWidgetChannelLabel->show(true);

        BDBG_MSG(("add channellabel:%s", pLabelData->_strImagePath.s()));
        _channelLabelList.add(new CChannelLabel(pChannel, pLabelData, pWidgetChannelLabel));
        BDBG_MSG(("_channelLabelList.total():%d", _channelLabelList.total()));

        if (NULL != pChannelLabel)
        {
            /* save return param */
            *pChannelLabel = pWidgetChannelLabel;
        }
    }
    else
    {
        BDBG_MSG(("createChannelLabel() - skip creating duplicate label:%s", pLabelData->_strImagePath.s()));

        if (NULL != pChannelLabel)
        {
            /* save return param */
            *pChannelLabel = pChannelLabelFound->_pLabel;
        }
    }

error:
    return(ret);
} /* createChannelLabel */

/* create/show labels indicated by the given pChannel object */
eRet CScreenMain::updateChannelLabels(CChannel * pChannel)
{
    CLabelData * pLabelData = NULL;
    CGraphics * pGraphics   = _pModel->getGraphics();
    int i                   = 0;
    eRet ret                = eRet_Ok;

    if ((NULL == pChannel) || (NULL == pGraphics))
    {
        return(ret);
    }

    while (NULL != (pLabelData = pChannel->getLabelData(i)))
    {
        ret = createChannelLabel(pLabelData, pChannel);
        CHECK_ERROR_GOTO("unable to create channel label", ret, error);

        i++;
    }
error:
    return(ret);
} /* updateChannelLabels */

eRet CScreenMain::removeChannelLabels(CChannel * pChannel)
{
    CGraphics * pGraphics         = _pModel->getGraphics();
    CChannelLabel * pChannelLabel = NULL;
    CLabelData * pLabelData       = NULL;
    eRet ret                      = eRet_Ok;

    BDBG_ASSERT(NULL != pChannel);

    int index = 0;
    for (pChannelLabel = _channelLabelList[index]; pChannelLabel; pChannelLabel = _channelLabelList[index])
    {
        /* delete channel labels that match given pChannel */
        if (pChannel == pChannelLabel->_pChannel)
        {
            /* remove/hide/delete matching channel labels from list */
            _channelLabelList.remove(index);
            if (NULL != pChannelLabel->_pLabel)
            {
                pChannelLabel->_pLabel->show(false);
            }
            BDBG_MSG(("DELETE channel label:%s", pChannelLabel->_labelData._strImagePath.s()));
            DEL(pChannelLabel);
        }
        else
        {
            /* only increment if no match found */
            index++;
        }
    }

    {
        CSimpleVideoDecode * pVideoDecode = pChannel->getVideoDecode();
        if (NULL != pVideoDecode)
        {
            _pCurrentPlmLabel[pVideoDecode->getWindowType()] = NULL;
        }
    }

    if (NULL != pGraphics)
    {
        /* we must force a redraw to ensure that channel labels are removed
         * immediately.  this is most likely being called during a
         * channel change, so hiding labels before the next channel tune is
         * very time sensitive. */
        pGraphics->forceDrawSync(getEngine());
    }
error:
    return(ret);
} /* removeChannelLabels */

#if DVR_LIB_SUPPORT
void CScreenMain::setTsbStatus(CTsb * pTsb)
{
    BDBG_ASSERT(NULL != pTsb);
    MString liveMode("live");
    if (pTsb->getTsbState() == liveMode)
    {
        CChannel * pChannel = _pModel->getCurrentChannel();
        setChannelNum(pChannel);
    }
    else
    {
        setChannelStatus(eMode_Tsb, pTsb->getTsbState(), pTsb->getTimeString());
    }
} /* setTsbStatus */

#endif /* if DVR_LIB_SUPPORT */

void CScreenMain::setPlaybackStatus(CPlayback * pPlayback)
{
    MString strTrick;
    MString strPlaybackName;
    char strRate[16];

    BDBG_ASSERT(NULL != pPlayback);

    snprintf(strRate, 15, "%g", pPlayback->getTrickModeRate());

    strTrick  = playbackTrickToString(pPlayback->getTrickModeState()) + " ";
    strTrick += MString(strRate) + "x";

    {
        CVideo * pVideo = pPlayback->getVideo();
        if (NULL != pVideo)
        {
            strPlaybackName = pVideo->getVideoName();
        }
    }
    setChannelStatus(eMode_Playback, strTrick, pPlayback->getTimeString(), strPlaybackName);
} /* setPlaybackStatus */

eRet CScreenMain::addRecordEncodeIndicator(CChannel * pChannel)
{
    eRet ret                 = eRet_Ok;
    unsigned indWidgetHeight = 15;
    const unsigned bevel     = 1;
    CGraphics * pGraphics    = NULL;
    bwin_font_t font10;
    MRect rectChNum;
    char strChNum[32];

    strChNum[0] = 0;
    pGraphics   = _pModel->getGraphics();
    BDBG_ASSERT(NULL != pGraphics);
    font10 = pGraphics->getFont(10);
    BDBG_ASSERT(NULL != font10);

    /* start with current size of channel number widget */
    rectChNum = _pLabelChannelNum->getGeometry();

    BDBG_MSG(("is channel:%d.%d encoding? %d", pChannel->getMajor(), pChannel->getMinor(), pChannel->isEncoding()));
    if ((false == pChannel->isRecording()) && (false == pChannel->isEncoding()))
    {
        return(eRet_NotAvailable);
    }

    /* recording so enlarge channel num widget to contain a new record indicator */
    rectChNum.setHeight(rectChNum.height() + indWidgetHeight - bevel);

    /* create new indicator widget */
    {
        CWidgetLabel * pIndicator = NULL;
        MRect rectIndicator;

        rectIndicator = _rectChType;

        /* adjust width to account for bevel */
        rectIndicator.setWidth(rectIndicator.width() + (bevel * 2));
        rectIndicator.setX(rectIndicator.x() - bevel);

        /* move rect below channel text widget */
        rectIndicator.setY(rectIndicator.y() + _rectChType.height());
        /* move rect below any existing rec indicator widgets */
        rectIndicator.setY(rectIndicator.y() + _indicatorList.total() * (indWidgetHeight - bevel));

        rectIndicator.setHeight(indWidgetHeight);
        pIndicator = new CWidgetLabel("CScreenMain::pIndicator", getEngine(), _pLabelChannelNum, rectIndicator, font10);
        CHECK_PTR_ERROR_GOTO("unable to allocate label widget", pIndicator, ret, eRet_OutOfMemory, error);

        pIndicator->setBevel(bevel);
        pIndicator->setTextColor(pIndicator->getBackgroundColor());

        if (true == pChannel->isRecording())
        {
            snprintf(strChNum, 32, "REC %s", (0 < pChannel->getMajor()) ? pChannel->getChannelNum().s() : pChannel->getDescriptionShort().left(18).s());
            pIndicator->setBackgroundColor(0xFFFF0000);
        }

        if (true == pChannel->isEncoding())
        {
            snprintf(strChNum, 32, "XCODE %s", (0 < pChannel->getMajor()) ? pChannel->getChannelNum().s() : pChannel->getDescriptionShort().left(16).s());
            pIndicator->setBackgroundColor(0xFF0000FF);
        }

        pIndicator->setText(strChNum, bwidget_justify_horiz_center, bwidget_justify_vert_middle);
        pIndicator->show(true);

        _indicatorList.add(pIndicator);

        /* adjust channel number widget geometry to include new indicator widget */
        _pLabelChannelNum->setGeometry(rectChNum);
    }

error:
    return(ret);
} /* addRecordEncodeIndicator */

void CScreenMain::setChannelIndicator(
        CChannelMgr * pChannelMgr,
        CPlaylistDb * pPlaylistDb
        )
{
    MRect rectChNum;
    CChannel * pChannel = NULL;
    bool bUpdated       = false;
    eRet ret            = eRet_Ok;

    /* delete all previous record indicator labels */
    _indicatorList.clear();

    /* start with original size of channel number widget */
    rectChNum = _rectChannelNum;
    _pLabelChannelNum->setGeometry(rectChNum);

    if (NULL != pChannelMgr)
    {
        /* add record/encode indicator for each recording channelmgr channel */

        /* main channel list */
        for (pChannel = pChannelMgr->getFirstChannel(eWindowType_Main);
             pChannel;
             pChannel = pChannelMgr->getNextChannel(pChannel, false))
        {
            ret = addRecordEncodeIndicator(pChannel);
            if (eRet_Ok == ret)
            {
                bUpdated = true;
            }
        }
        /* pip channel list */
        for (pChannel = pChannelMgr->getFirstChannel(eWindowType_Pip);
             pChannel;
             pChannel = pChannelMgr->getNextChannel(pChannel, false))
        {
            ret = addRecordEncodeIndicator(pChannel);
            if (eRet_Ok == ret)
            {
                bUpdated = true;
            }
        }
    }

    if (NULL != pPlaylistDb)
    {
        CPlaylist * pPlaylist = NULL;
        int i                 = 0;

        /* add record/encode indicator for each recording playlist channel */
        while (NULL != (pPlaylist = pPlaylistDb->getPlaylist(i)))
        {
            CChannel * pPlaylistChannel = NULL;
            int numChannels             = pPlaylist->numChannels();
            int j                       = 0;

            for (j = 0; j < numChannels; j++)
            {
                pPlaylistChannel = pPlaylist->getChannel(j);
                if (NULL == pPlaylistChannel)
                {
                    continue;
                }

                ret = addRecordEncodeIndicator(pPlaylistChannel);
                if (eRet_Ok == ret)
                {
                    bUpdated = true;
                }
            }

            i++;
        }
    }

    if (false == bUpdated)
    {
        /* not recording/encoding so use original geometry */
        rectChNum = _rectChannelNum;

        /* adjust channel number widget geometry */
        _pLabelChannelNum->setGeometry(rectChNum);
    }
} /* setChannelIndicator */

#define PERCENT_TO_UINT16(percent)  ((percent) * 65535 / 100)

#if NEXUS_HAS_FRONTEND
void CScreenMain::setScanProgress(
        CTuner * pTuner,
        uint8_t  progress
        )
{
    char strScan[64];

    BDBG_ASSERT(NULL != pTuner);
    BDBG_ASSERT(100 >= progress);

    snprintf(strScan, sizeof(strScan), "Scanning %s%d : %3d%%  Found:%d",
            pTuner->getName(), pTuner->getNumber(), progress, _channelsFound);
    _pLabelScan->setText(strScan);
    _pLabelScan->setLevel(PERCENT_TO_UINT16(progress));
}

#endif /* NEXUS_HAS_FRONTEND */

void CScreenMain::setVolumeProgress(
        uint8_t progress,
        bool    bMute
        )
{
    char strVolume[64];

    BDBG_ASSERT(100 >= progress);

    if (true == bMute)
    {
        _timerVolume.stop();
        snprintf(strVolume, sizeof(strVolume), "MUTE");
    }
    else
    {
        snprintf(strVolume, sizeof(strVolume), "Volume: %3d%%", progress);
        _timerVolume.start(GET_INT(_pCfg, UI_VOLUME_TIMEOUT));
    }
    _pLabelVolume->setText(strVolume);
    _pLabelVolume->setLevel(PERCENT_TO_UINT16(progress));

    _pLabelVolume->showProgress(true);
    _pLabelVolume->show(true);
} /* setVolumeProgress */

#if BDSP_MS12_SUPPORT
void CScreenMain::setAudioFadeProgress(
        uint8_t progress,
        eWindowType windowType
        )
{
    char strAudioFade[64];

    BDBG_ASSERT(100 >= progress);

    if (eWindowType_Max == windowType)
    {
        windowType = eWindowType_Main;
    }

    if (eWindowType_Main == windowType)
    {
        snprintf(strAudioFade, sizeof(strAudioFade), "Main Audio Level: %3d%%", progress);
        _pLabelAudioFadeMain->setText(strAudioFade);
        _pLabelAudioFadeMain->setLevel(PERCENT_TO_UINT16(progress));
        _pLabelAudioFadeMain->showProgress(true);
        _pLabelAudioFadeMain->show(true);
    }
    else
    {
         snprintf(strAudioFade, sizeof(strAudioFade), "PiP Audio Level: %3d%%", progress);
        _pLabelAudioFadePip->setText(strAudioFade);
        _pLabelAudioFadePip->setLevel(PERCENT_TO_UINT16(progress));
        _pLabelAudioFadePip->showProgress(true);
        _pLabelAudioFadePip->show(true);
    }

} /* setAudioFadeProgress */
#endif

#if BDSP_MS12_SUPPORT
/* convert -12dB to 12dB to 2^16-1 */
#define DIALOG_DB_TO_UINT16(db)  (((db) + 12) * 65535 / 24)

void CScreenMain::setDialogEnhancementProgress(
        int     nDb
        )
{
    char strDialogEnhancement[64];

    BDBG_ASSERT(-12 <= nDb);
    BDBG_ASSERT(12 >= nDb);

    snprintf(strDialogEnhancement, sizeof(strDialogEnhancement), "Dialog Enhancement: %3ddB", nDb);
    _timerVolume.start(GET_INT(_pCfg, UI_VOLUME_TIMEOUT));

    _pLabelVolume->setText(strDialogEnhancement);
    _pLabelVolume->setLevel(DIALOG_DB_TO_UINT16(nDb));

    _pLabelVolume->showProgress(true);
    _pLabelVolume->show(true);
} /* setDialogEnhancementProgress */
#endif

void CScreenMain::showMenu(eMenu menu)
{
    switch (menu)
    {
    case eMenu_Main:
        showMenu(NULL);
        _pMainMenu->show(true);
        break;
    case eMenu_Decode:
        showMenu(NULL);
        _pDecodeMenu->show(true);
        break;
    case eMenu_Display:
        showMenu(NULL);
        _pDisplayMenu->show(true);
        break;
    case eMenu_Audio:
        showMenu(NULL);
        _pAudioMenu->show(true);
        break;
#if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT)
    case eMenu_Network:
        showMenu(NULL);
        _pNetworkMenu->show(true);
        break;
#endif /* if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT) */
#ifdef NETAPP_SUPPORT
    case eMenu_Bluetooth:
        showMenu(NULL);
        _pBluetoothMenu->show(true);
        break;
#endif /* ifdef NETAPP_SUPPORT */
#ifdef PLAYBACK_IP_SUPPORT
    case eMenu_Streaming:
        showMenu(NULL);
        _pStreamingMenu->show(true);
        break;
#endif /* ifdef PLAYBACK_IP_SUPPORT */
    case eMenu_Playback:
        showMenu(NULL);
        _pPlaybackMenu->show(true);
        break;
#if NEXUS_HAS_FRONTEND
    case eMenu_Tuner:
        showMenu(NULL);
        _pTunerMenu->show(true);
        break;
    case eMenu_ScanQam:
        showMenu(NULL);
        _pScanQamMenu->show(true);
        break;
    case eMenu_ScanVsb:
        showMenu(NULL);
        _pScanVsbMenu->show(true);
        break;
    case eMenu_ScanSat:
        showMenu(NULL);
        _pScanSatMenu->show(true);
        break;
    case eMenu_ScanOfdm:
        showMenu(NULL);
        _pScanOfdmMenu->show(true);
        break;
#endif /* if NEXUS_HAS_FRONTEND */
    case eMenu_Vbi:
        showMenu(NULL);
        _pVbiMenu->show(true);
        break;
#if BDSP_MS12_SUPPORT
    case eMenu_Audio_Ac4:
        showMenu(NULL);
        _pAudioAc4Menu->show(true);
        break;
#endif
#ifdef MPOD_SUPPORT
    case eMenu_CableCard:
        showMenu(NULL);
        _pCableCardMenu->show(true);
        break;
#endif /* ifdef MPOD_SUPPORT */
    default:
        BDBG_ERR(("unhandled showMenu() request"));
        break;
    } /* switch */
}     /* showMenu */

void CScreenMain::onClick(bwidget_t widget)
{
    CWidgetBase * pWidget = NULL;

    /* find the widget object that corresponds to the given bwidget_t */
    {
        blabel_settings labelSettings;
        blabel_get(widget, &labelSettings);
        pWidget = (CWidgetBase *)labelSettings.widget.data;
    }

    if (_Decode == pWidget)
    {
        showMenu(eMenu_Decode);
    }
    else
    if (_Display == pWidget)
    {
        showMenu(eMenu_Display);
    }
    else
    if (_Audio == pWidget)
    {
        showMenu(eMenu_Audio);
    }
    else
#if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT)
    if (_Network == pWidget)
    {
        showMenu(eMenu_Network);
    }
    else
#endif /* if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT) */
#ifdef NETAPP_SUPPORT
    if (_Bluetooth == pWidget)
    {
        showMenu(eMenu_Bluetooth);
    }
    else
#endif /* ifdef NETAPP_SUPPORT */
    if (_Streaming == pWidget)
    {
        showMenu(eMenu_Streaming);
    }
    else
    if (_Playback == pWidget)
    {
        showMenu(eMenu_Playback);
    }
    else
#if NEXUS_HAS_FRONTEND
    if (_Tuner == pWidget)
    {
        showMenu(eMenu_Tuner);
    }
    else
#endif /* if NEXUS_HAS_FRONTEND */
#if NEXUS_HAS_FRONTEND
    if (_ScanQam == pWidget)
    {
        showMenu(eMenu_ScanQam);
    }
    else
    if (_ScanVsb == pWidget)
    {
        showMenu(eMenu_ScanVsb);
    }
    else
    if (_ScanSat == pWidget)
    {
        showMenu(eMenu_ScanSat);
    }
    else
    if (_ScanOfdm == pWidget)
    {
        showMenu(eMenu_ScanOfdm);
    }
    else
#endif /* if NEXUS_HAS_FRONTEND */
#ifdef MPOD_SUPPORT
    if (_CableCard == pWidget)
    {
        showMenu(eMenu_CableCard);
    }
    else
#endif /* ifdef MPOD_SUPPORT */
    if (_Buffers == pWidget)
    {
        _pBuffersMenu->show(_Buffers->isChecked());
    }
    else
#ifdef CPUTEST_SUPPORT
    if (0 <= _CpuTestPopup->getItemListIndex(pWidget->getWidget()))
    {
        CWidgetCheckButton * pButton = (CWidgetCheckButton *)pWidget;
        int cpuTestValue             = (int)pButton->getValue();

        BDBG_WRN(("selected cpu test level:%s", pButton->getText().s()));
        notifyObservers(eNotify_SetCpuTestLevel, &cpuTestValue);
    }
    else
#endif /* ifdef CPUTEST_SUPPORT */
#if POWERSTANDBY_SUPPORT
    if (_Power == pWidget)
    {
        MString strRet("Ok");
        strRet = _pPowerMenu->showModal();
        if (strRet == "Ok")
        {
            ePowerMode mode = _pPowerMenu->getPowerMode();
            notifyObservers(eNotify_SetPowerMode, &mode);
        }
    }
    else
#endif /* if POWERSTANDBY_SUPPORT */
    if (_Back == pWidget)
    {
        show(false);
    }
    else
    {
        BDBG_MSG(("Unhandled button click (%s)", BSTD_FUNCTION));
    }

    return;
} /* onClick */

eRet CScreenMain::onKeyDown(
        bwidget_t   widget,
        bwidget_key key
        )
{
    eRet ret = eRet_NotSupported; /* key NOT consumed */

    BSTD_UNUSED(widget);

    switch (key)
    {
    case bwidget_key_exit:
        show(false);
        ret = eRet_Ok; /* key consumed */
        break;

    default:
        BDBG_MSG(("unhandled key down: %d", key));
        break;
    } /* switch */

    return(ret);
} /* onKeyDown */

void CScreenMain::show(bool bShow)
{
    /* show/hide menu and channel number together */
    showMenu(NULL);
    _pMainMenu->show(bShow);
    _pLabelChannelNum->show(bShow);

    if (true == bShow)
    {
        CPlayback * pPlayback = _pModel->getPlayback();
        CChannel * pChannel   = _pModel->getCurrentChannel();
#ifdef DVR_LIB_SUPPORT
        CTsb * pTsb = _pModel->getTsb();
#endif
        if ((NULL != pPlayback) && (pPlayback->isActive()))
        {
            updatePlaybackProgress();
        }
        else
        if ((NULL != pChannel) && pChannel->timelineSupport())
        {
            updateChannelProgress();
        }
#if DVR_LIB_SUPPORT
        else
        if ((NULL != pTsb) && (pTsb->isActive()))
        {
            updateTsbProgress();
        }
#endif /* if DVR_LIB_SUPPORT */

        _Back->setFocus();
    }

    /* we do NOT want to hide this entire screen since various elements of it may be
     * displayed at any time (channel number, buffer/tsm popup, etc) */
    /* CScreen::show(bShow); */
} /* show */

bool CScreenMain::isVisible()
{
    return(_pMainMenu->isVisible() ||
           _pPlaybackMenu->isVisible() ||
#ifdef PLAYBACK_IP_SUPPORT
           _pStreamingMenu->isVisible() ||
#endif
           _pDisplayMenu->isVisible() ||
           _pDecodeMenu->isVisible() ||
           _pAudioMenu->isVisible() ||
#if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT)
           _pNetworkMenu->isVisible() ||
#endif
#ifdef NETAPP_SUPPORT
           _pBluetoothMenu->isVisible() ||
#endif
#ifdef PLAYBACK_IP_SUPPORT
           _pStreamingMenu->isVisible() ||
#endif
           _pPlaybackMenu->isVisible() ||
#if NEXUS_HAS_FRONTEND
           _pScanQamMenu->isVisible() ||
           _pScanVsbMenu->isVisible() ||
           _pScanSatMenu->isVisible() ||
           _pScanOfdmMenu->isVisible() ||
           _pTunerMenu->isVisible() ||
#endif /* if NEXUS_HAS_FRONTEND */
           _pVbiMenu->isVisible() ||
#if BDSP_MS12_SUPPORT
           _pAudioAc4Menu->isVisible() ||
#endif
#ifdef MPOD_SUPPORT
           _pCableCardMenu->isVisible() ||
#endif
           _pRecordMenu->isVisible());
} /* isVisible */

MString CScreenMain::showKeyboardModal(
        const char * strTitle,
        const char * strEntryTitle
        )
{
    BDBG_ASSERT(NULL != _pKeyboard);

    _pKeyboard->setTitle(strTitle, strEntryTitle);
    return(_pKeyboard->showModal());
}

void CScreenMain::showMenu(CWidgetMenu * pMenu)
{
    /* hide all other menus */
    _pMainMenu->show(false);
    _pPlaybackMenu->show(false);
#ifdef PLAYBACK_IP_SUPPORT
    _pStreamingMenu->show(false);
#endif
    _pDisplayMenu->show(false);
    _pDecodeMenu->show(false);
    _pAudioMenu->show(false);
#if defined (WPA_SUPPLICANT_SUPPORT) || defined (NETAPP_SUPPORT)
    _pNetworkMenu->show(false);
#endif
#ifdef NETAPP_SUPPORT
    _pBluetoothMenu->show(false);
#endif
#ifdef PLAYBACK_IP_SUPPORT
    _pStreamingMenu->show(false);
#endif
    _pPlaybackMenu->show(false);
#if NEXUS_HAS_FRONTEND
    _pScanQamMenu->show(false);
    _pScanVsbMenu->show(false);
    _pScanSatMenu->show(false);
    _pScanOfdmMenu->show(false);
    _pTunerMenu->show(false);
#endif /* if NEXUS_HAS_FRONTEND */
    _pVbiMenu->show(false);
#if BDSP_MS12_SUPPORT
    _pAudioAc4Menu->show(false);
#endif
#ifdef MPOD_SUPPORT
    _pCableCardMenu->show(false);
#endif
    _pRecordMenu->show(false);

    if (NULL != pMenu)
    {
        pMenu->show(true);
    }
} /* showMenu */

#ifdef CPUTEST_SUPPORT
void CScreenMain::updateCpuTestUtilization()
{
    if (NULL == _pCpuTest)
    {
        /* remove cpu utilization from menu item label */
        _CpuTestLabel->setText("CPU Test", bwidget_justify_horiz_left, bwidget_justify_vert_middle);
    }
    else
    {
        double dCpuUtil = _pCpuTest->getCpuUtilization();
        char strCpuUtil[8];

        /* add cpu utilization to the menu item label */
        memset(strCpuUtil, 0, sizeof(strCpuUtil));
        snprintf(strCpuUtil, sizeof(strCpuUtil), "%3.lf%%", dCpuUtil);
        _CpuTestLabel->setText(MString("CPU Test:") + strCpuUtil, bwidget_justify_horiz_left, bwidget_justify_vert_middle);
    }
} /* updateCpuTestUtilization */
#endif /* ifdef CPUTEST_SUPPORT */

#if BDSP_MS12_SUPPORT
void CScreenMain::updateAudioFadeLevels()
{
    CSimpleAudioDecode * pAudioDecodeMain = (CSimpleAudioDecode *)_pModel->getSimpleAudioDecode(_pModel->getFullScreenWindowType());
    CSimpleAudioDecode * pAudioDecodePip  = (CSimpleAudioDecode *)_pModel->getSimpleAudioDecode(_pModel->getPipScreenWindowType());

    if (NULL != pAudioDecodeMain)
    {
        setAudioFadeProgress(pAudioDecodeMain->getAudioFade(), _pModel->getPipSwapState() ? eWindowType_Pip : eWindowType_Main);
    }
    if (NULL != pAudioDecodePip)
    {
        setAudioFadeProgress(pAudioDecodePip->getAudioFade(), _pModel->getPipSwapState() ? eWindowType_Main : eWindowType_Pip);
    }
}
#endif

#if HAS_VID_NL_LUMA_RANGE_ADJ
eRet CScreenMain::addDynamicRangeIndicator(CSimpleVideoDecode * pVideoDecode)
{
    eRet ret                         = eRet_Ok;
    CDisplay * pDisplay              = _pModel->getDisplay();
    eDynamicRange dynamicRangeOutput = eDynamicRange_Unknown;

    /* show dynamic range indicator */
    if ((NULL == pVideoDecode) || (NULL == pDisplay))
    {
        return(ret);
    }
    dynamicRangeOutput = pDisplay->getOutputDynamicRange();

    if ((eDynamicRange_SDR == dynamicRangeOutput) || (eDynamicRange_Unknown == dynamicRangeOutput))
    {
        /* no need to show dynamic range indicator if output is SDR */
        BDBG_ERR(("sdr output so do nothing"));
        return(ret);
    }

    CChannel * pChannel = pVideoDecode->getChannel();
    if (NULL != pChannel)
    {
        CLabelData labelData;
        /* position label in bottom left corner.  given coordinates obviously will
         * result in the label being outside the bounds of the associated video window,
         * but createChannelLabel() will adjust it to fit */
        labelData._rectGeometryPercent = MRect(0, 1000, 0, 0);
        labelData._strImagePath        = "images/";
        labelData._zorder              = 2;

        {
            eDynamicRange dynamicRange = pVideoDecode->getDynamicRange();
            BDBG_MSG(("video decode dynamic range:%s (%d)", videoDynamicRangeToString(dynamicRange).s(), dynamicRange));

            if (eDynamicRange_Unknown != pChannel->getDynamicRange())
            {
                /* allow channel chance to override DynamicRange */
                BDBG_MSG(("channel override DynamicRange %s -> %s", videoDynamicRangeToString(dynamicRange).s(), videoDynamicRangeToString(pChannel->getDynamicRange()).s()));
                dynamicRange = pChannel->getDynamicRange();
            }

            switch (dynamicRange)
            {
            case eDynamicRange_SDR:
                labelData._strImagePath += GET_STR(_pCfg, IMAGE_CHANNEL_SDR);
                break;

            case eDynamicRange_HDR10:
                labelData._strImagePath += GET_STR(_pCfg, IMAGE_CHANNEL_HDR10);
                break;

            case eDynamicRange_HLG:
                labelData._strImagePath += GET_STR(_pCfg, IMAGE_CHANNEL_HLG);
                break;

            case eDynamicRange_DolbyVision:
                labelData._strImagePath += GET_STR(_pCfg, IMAGE_CHANNEL_DOLBYVISION);
                break;

            default:
                ret = eRet_NotAvailable;
                break;
            } /* switch */
        }

        if (labelData._strImagePath != "images/")
        {
            createChannelLabel(&labelData, pChannel);
        }
    }

    return(ret);
} /* addDynamicRangeIndicator */

#endif /* if HAS_VID_NL_LUMA_RANGE_ADJ */

#if HAS_VID_NL_LUMA_RANGE_ADJ
void CScreenMain::addPlmIndicator(CSimpleVideoDecode * pVideoDecode)
{
    CDisplay * pDisplay              = _pModel->getDisplay();
    eDynamicRange dynamicRangeOutput = eDynamicRange_Unknown;
    eWindowType winType              = eWindowType_Max;

    /* show Programmable Luminance Mapping (PLM) indicator */
    if ((NULL == pVideoDecode) || (NULL == pDisplay))
    {
        return;
    }

    dynamicRangeOutput = pDisplay->getOutputDynamicRange();
    winType            = pVideoDecode->getWindowType();

    if ((eDynamicRange_SDR == dynamicRangeOutput) ||
        (eDynamicRange_Unknown == dynamicRangeOutput) ||
        (eDynamicRange_Max == dynamicRangeOutput))
    {
        /* no need to show PLM inidicator if output is SDR */
        return;
    }

    CChannel * pChannel = pVideoDecode->getChannel();
    if (NULL != pChannel)
    {
        bool bPlm = pChannel->isPlmEnabled();
        CLabelData labelData;
        eDynamicRange dynamicRangeInput = pVideoDecode->getDynamicRange();
        eDynamicRange dynamicRangeOuput = pDisplay->getOutputDynamicRange();

        if (eDynamicRange_Unknown != pChannel->getDynamicRange())
        {
            /* allow channel chance to override DynamicRange */
            BDBG_MSG(("channel override DynamicRange %s -> %s", videoDynamicRangeToString(dynamicRangeInput).s(), videoDynamicRangeToString(pChannel->getDynamicRange()).s()));
            dynamicRangeInput = pChannel->getDynamicRange();
        }

        labelData._strImagePath = "images/";

        BDBG_MSG(("dynamic range input:%s output:%s", videoDynamicRangeToString(dynamicRangeInput).s(), videoDynamicRangeToString(dynamicRangeOutput).s()));

        if ((eDynamicRange_DolbyVision == dynamicRangeInput) ||
            (eDynamicRange_DolbyVision == dynamicRangeOuput))
        {
            /* PLM is not used for Dolby Vision */
            goto done;
        }

        if (false == bPlm)
        {
            labelData._strImagePath += GET_STR(_pCfg, IMAGE_PLM_DISABLED);
        }
        else
        if ((dynamicRangeInput == dynamicRangeOutput) ||
            ((dynamicRangeInput == eDynamicRange_HLG) && (dynamicRangeOutput == eDynamicRange_SDR)))
        {
            /* input video dynamic range equals output video dynamic range or
             * transition is from HLG->SDR, so use passthru PLM label */
            labelData._strImagePath += GET_STR(_pCfg, IMAGE_PLM_PASSTHRU);
        }
        else
        {
            labelData._strImagePath += GET_STR(_pCfg, IMAGE_PLM_ENABLED);
        }

        /* position label in bottom right corner.  given coordinates obviously will
         * result in the label being outside the bounds of the associated video window,
         * but createChannelLabel() will adjust it to fit */
        labelData._rectGeometryPercent = MRect(0, 1000, 0, 0);
        labelData._zorder              = 1;

        BDBG_MSG(("video decode PLM:%s", pChannel->isPlmEnabled() ? "true" : "false"));

        if (labelData._strImagePath != "images/")
        {
            CWidgetLabel * pLabelPlm = NULL;
            createChannelLabel(&labelData, pChannel, &pLabelPlm);

            if (NULL != pLabelPlm)
            {
                if (NULL != _pCurrentPlmLabel[winType])
                {
                    /* hide last plm label */
                    _pCurrentPlmLabel[winType]->show(false);
                    _pCurrentPlmLabel[winType] = NULL;
                }

                /* show/save plm label so we can hide it later if addPlmIndicator() is called again */
                pLabelPlm->show(true);
                _pCurrentPlmLabel[winType] = pLabelPlm;
            }
        }
    }

done:
    return;
} /* addPlmIndicator */

#endif /* if HAS_VID_NL_LUMA_RANGE_ADJ */